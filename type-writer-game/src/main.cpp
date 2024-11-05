#include <Arduino.h>

// Functii de initializare
void initPins();
void initValues();
void initTimers();
void onGameFree();
void onGameRunning();
void onNewWord();
void onChangeFree();
void onChangeStarting();
void onChangeRunning();
void handleStartStopInterrupt();
void handleDifficultyInterrupt();
void ledOff();
void ledWhite();
void ledGreen();
void ledRed();
void onDifficultyChange();

// Lista de cuvinte
char *words[] = {
    "sulfus", "holoboard", "luca", "autocad",
    "linux", "cisco", "ciscoconpa", "sheep",
    "cheese", "selenium", "cucumber", "elephant",
    "djibouti", "dallas", "bear", "beer",
    "fmi", "router", "universe", "wagon",
    "asteroid", "pasta", "python", "volcano",
    "circuit", "galaxy", "mammoth", "yeti"
};
const int wordCount = 28;

// Definirea pinilor pentru leduri si butoane
const int RED_PIN = 6;
const int GREEN_PIN = 5;
const int BLUE_PIN = 4;
const int START_STOP_PIN = 2;
const int DIFFICULTY_PIN = 3;

// Constante pentru joc
const int BASE_COMPARE_MATCH = 62500;
const int BAUD_RATE = 9600;
const unsigned int DEBOUNCE_DELAY = 500;
const unsigned int BLINK_INTERVAL_START = 500;
const unsigned int DURATION_START_STATE = 3000;

const unsigned int GAME_DURATION = 30;
const unsigned int EASY_INTERVAL = 10;
const unsigned int MEDIUM_INTERVAL = 5;
const unsigned int HARD_INTERVAL = 2;

// Variabile pentru contorizarea timpului si debounce
unsigned long lastStartStopPress = 0;
unsigned long lastDifficultyPress = 0;

volatile int wordTimer = EASY_INTERVAL;
volatile unsigned int gameTimer = GAME_DURATION;

// Starea butoanelor
bool isStartStopPressed = false;
bool isDifficultyPressed = false;

// Starea pentru interrupt-uri
bool isShowNewWord = false;
bool isGameEnded = false;

// Variabile pentru cuvinte si input
String currentWord;
String guessedWord = "";
String inputBuffer = "";

// Timpul curent pentru gestionarea timerului
unsigned long timer1 = 0;
unsigned long timerStart = 0;

// Ajutor pentru blink
unsigned int blinkHelper = 0;
unsigned int correctWordCount = 0;

// Enumerari pentru starea jocului si dificultate
enum State { idle, starting, running };
enum Difficulty { easy, medium, hard };

State gameState;
Difficulty gameDifficulty;

// Functie de initializare a pinilor
void initPins() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(START_STOP_PIN, INPUT_PULLUP);
  pinMode(DIFFICULTY_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(START_STOP_PIN), handleStartStopInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(DIFFICULTY_PIN), handleDifficultyInterrupt, FALLING);

  Serial.begin(BAUD_RATE);
}

// Functie de initializare a valorilor
void initValues() {
  onChangeFree();
  randomSeed(millis());
  Serial.println("Welcome to Arduino TypeRacer");
}

// Functie de initializare a timerelor
void initTimers() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = BASE_COMPARE_MATCH;
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

// Functie pentru starea "libera"
void onGameFree() {
  if (isStartStopPressed) {
    onChangeStarting();
    Serial.println("\nStarting!");
    isStartStopPressed = false;
  }

  if (isDifficultyPressed) {
    onDifficultyChange();
    isDifficultyPressed = false;
  }
}

// Functie pentru starea "start"
void onGameStarting() {
  if (isStartStopPressed) {
    Serial.println("\nGame aborted!");
    onChangeFree();
    isStartStopPressed = false;
  }

  timer1 = millis() - timerStart;
  if (timer1 > BLINK_INTERVAL_START * blinkHelper) {
    blinkHelper % 2 == 0 ? ledWhite() : ledOff();
    blinkHelper++;
  }

  if (timer1 > DURATION_START_STATE) {
    Serial.println("\nGo!");
    onChangeRunning();
  }
}

// Functie pentru starea "running"
void onGameRunning() {
  if (isStartStopPressed) {
    Serial.println("\nGame aborted!");
    onChangeFree();
    isStartStopPressed = false;
  }

  if (isShowNewWord) {
    onNewWord();
    isShowNewWord = false;
  }

  if (isGameEnded) {
    Serial.println("Game over!");
    Serial.print("You have guessed ");
    Serial.print(correctWordCount);
    Serial.println(" words correctly");
    onChangeFree();
    isGameEnded = false;
  }

  if (inputBuffer.length() > 0) {
    char inputChar = inputBuffer.charAt(0);
    inputBuffer.remove(0, 1);

    if (inputChar == '\b' && guessedWord.length() > 0) {
      guessedWord.remove(guessedWord.length() - 1);
      currentWord.startsWith(guessedWord) ? ledGreen() : ledRed();
    } else if (isAlpha(inputChar)) {
      inputChar = tolower(inputChar);
      guessedWord += inputChar;

      if (guessedWord == currentWord) {
        correctWordCount++;
        Serial.println("\nCorrect!");
        isShowNewWord = true;
      }
      currentWord.startsWith(guessedWord) ? ledGreen() : ledRed();
    }
  }
}

// Functii principale
void setup() {
  initPins();
  initValues();
  initTimers();
}

void loop() {
  if (gameState == idle) onGameFree();
  else if (gameState == starting) onGameStarting();
  else if (gameState == running) onGameRunning();
}

// Functie pentru timer ISR
ISR(TIMER1_COMPA_vect) {
  if (gameState != running) return;
  wordTimer -= 1;
  gameTimer -= 1;

  if (wordTimer <= 0) isShowNewWord = true;
  if (gameTimer <= 0) isGameEnded = true;
}

// Functie pentru evenimente seriale
void serialEvent() {
  while (Serial.available()) {
    inputBuffer += (char)Serial.read();
  }
}

// Functie de intrerupere pentru start/stop
void handleStartStopInterrupt() {
  unsigned long currentTime = millis();
  if (currentTime - lastStartStopPress > DEBOUNCE_DELAY) {
    isStartStopPressed = true;
    lastStartStopPress = currentTime;
  }
}

// Functie de intrerupere pentru dificultate
void handleDifficultyInterrupt() {
  if (gameState != idle) return;
  unsigned long currentTime = millis();
  if (currentTime - lastDifficultyPress > DEBOUNCE_DELAY) {
    isDifficultyPressed = true;
    lastDifficultyPress = currentTime;
  }
}

// Functie pentru schimbarea dificultatii
void onDifficultyChange() {
  if (gameState != idle) return;
  gameDifficulty = (gameDifficulty == easy) ? medium : (gameDifficulty == medium ? hard : easy);
  Serial.print(gameDifficulty == easy ? "Easy mode on!\n" : gameDifficulty == medium ? "Medium mode on!\n" : "Hard mode on!\n");
}

// Functie pentru afisarea unui cuvant nou
void onNewWord() {
  wordTimer = gameDifficulty == easy ? EASY_INTERVAL : (gameDifficulty == medium ? MEDIUM_INTERVAL : HARD_INTERVAL);
  currentWord = String(words[random(wordCount)]);
  guessedWord = "";
  inputBuffer = "";
  ledGreen();
  Serial.println("\nNew word:");
  Serial.println(currentWord);
}

// Functii pentru schimbarea starii
void onChangeFree() { ledWhite(); gameState = idle; gameDifficulty = easy; }
void onChangeStarting() { ledOff(); timerStart = millis(); timer1 = 0; blinkHelper = 1; gameState = starting; }
void onChangeRunning() { ledGreen(); gameTimer = GAME_DURATION; wordTimer = 0; correctWordCount = 0; gameState = running; }

// Functii ajutatoare pentru LED-uri
void ledWhite() { digitalWrite(RED_PIN, HIGH); digitalWrite(GREEN_PIN, HIGH); digitalWrite(BLUE_PIN, HIGH); }
void ledOff() { digitalWrite(RED_PIN, LOW); digitalWrite(GREEN_PIN, LOW); digitalWrite(BLUE_PIN, LOW); }
void ledGreen() { digitalWrite(RED_PIN, LOW); digitalWrite(GREEN_PIN, HIGH); digitalWrite(BLUE_PIN, LOW); }
void ledRed() { digitalWrite(RED_PIN, HIGH); digitalWrite(GREEN_PIN, LOW); digitalWrite(BLUE_PIN, LOW); }
