#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Key.h>
#include <Keypad.h>

#define KP_ROWS 4
#define KP_COLS 3
char kpKeys[KP_ROWS][KP_COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte kpRowPins[] = {8, 7, 6, 5};
byte kpColPins[] = {4, 3, 2};
Keypad keypad = Keypad(makeKeymap(kpKeys), kpRowPins, kpColPins, KP_ROWS, KP_COLS);

#define LCD_I2C_ADDR 0x27
#define LCD_RS_PIN 0
#define LCD_RW_PIN 1
#define LCD_EN_PIN 2
#define LCD_BACKLIGHT_PIN 3
#define LCD_D4_PIN 4
#define LCD_D5_PIN 5
#define LCD_D6_PIN 6
#define LCD_D7_PIN 7

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

enum state {
  NEW_GAME,
  SCORE_ENTRY,
  GAME_OVER
};

state currentState;
int numPlayers;
int startingScore;
int currentPlayer;
int currentScore;

#define MAX_PLAYERS 8
int scores[MAX_PLAYERS];


void setup()
{
  Serial.begin(9600);
  
  keypad.setDebounceTime(250);
  keypad.setHoldTime(2000);
  keypad.addEventListener(kpEvent);

  lcd.begin(20, 4);

  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight();
    
  setupGame();
}

void loop()
{
  // put your main code here, to run repeatedly:
  keypad.getKey();
}

int charToNum(char c)
{
  return (c - 48);
}

void setupGame()
{
  currentState = NEW_GAME;
  
  numPlayers = 0;
  startingScore = 0;

  lcd.home();
  lcd.print(" Enter # of players "); 
  lcd.setCursor(0, 2);
  lcd.print("       [1-8]       ");
}

void setupScoreEntry()
{
  currentState = SCORE_ENTRY;
  currentPlayer = 0;
  for (int i = 0; i < numPlayers; i++) {
    scores[i] = startingScore;
  }

  nextScoreEntry();
}

/*
 * enterScore
 * 
 * Process a score entry for the current player, and move to the next player.
 */
void enterScore()
{
  // Can't go negative
  if (scores[currentPlayer] < currentScore) {
    Serial.println("Invalid score");
    currentScore = 0;
  }

  scores[currentPlayer] -= currentScore;
  Serial.print("Score entry for player ");
  Serial.print(currentPlayer + 1);
  Serial.print(": ");
  Serial.print(currentScore);
  Serial.print(", ");
  Serial.print(scores[currentPlayer]);
  Serial.println(" remaining");

  // Check for the player having finished
  if (scores[currentPlayer] == 0) {
    // @@@
  }

  // Move to the next player
  currentScore = 0;
  if (currentPlayer + 1 == numPlayers) {
    currentPlayer = 0;
  } else {
    currentPlayer++;
  }

  nextScoreEntry();
}


void nextScoreEntry()
{
  Serial.print("Enter score for player ");
  Serial.print(currentPlayer);
  Serial.println(":");
}


void kpEvent(KeypadEvent key)
{
  if (keypad.getState() == HOLD && key == '*') {
    // Reset everything
    setupGame();
  } else if (keypad.getState() == PRESSED) {
    switch (currentState) {
    case NEW_GAME:
      if (numPlayers == 0) {
        // Player number entry
        if (isDigit(key) && charToNum(key) > 1 && charToNum(key) < MAX_PLAYERS) {
          numPlayers = charToNum(key);
          lcd.home();
          lcd.print("     "); lcd.print(numPlayers); lcd.print(" players");
          lcd.print("Enter starting score, followed by #:");
        }
      } else {
        // Starting score entry
        if (startingScore > 0 && key == '#') {
          Serial.print("Game from ");
          Serial.print(startingScore);
          Serial.println(" started");
          setupScoreEntry();
        } else if (isDigit(key)) {
          startingScore *= 10;
          startingScore += charToNum(key);
        }
      }      
      break;

    case SCORE_ENTRY:
      if (key == '#') {
        enterScore();
      } else if (isDigit(key)) {
        currentScore *= 10;
        currentScore += charToNum(key);
      }
      
      break;
    }
    
  }
}

