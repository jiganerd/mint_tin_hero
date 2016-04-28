//
// MINT TIN HERO!
//
// By Brian Dolan
//
// The circuit:
// - analog pin 0: piezo sensor
// - digital pin 2: LCD backlight
// - digital pin 3: LCD RS
// - digital pin 4: LCD Enable
// - digital pin 5: LCD D4
// - digital pin 6: LCD D5
// - digital pin 7: LCD D6
// - digital pin 8: LCD D7
// - digital pin 9: speaker
//

// RAM is a challenge:
// hurricane rhythm: ~16x19=304 bytes
// talk rhythm:      ~16x19=304 bytes
//                          ---------
//                          608 bytes
// moved most strings and other consts into PROGMEM
// about 762 bytes left RAM at runtime
 
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

// Pin definitions
const int hitSensorPin = 0;
const int speakerPin = 9;
const int backlightLedPin = 2;

int hitSensorValue = 0; 
int threshold = 200;

const int screenWidthChars = 8;

// variables related to backlight blinking
const int blBlinkToggleCountSetting = 4;
const unsigned long blBlinkMs = 50;
int blBlinkToggleCountCurrent;
int blState = HIGH;
unsigned long blPrevMs = 0;

int rhythmCursor;
unsigned long rhythmCursorPrevMs = 0;
int currentNoteBeatCount;
char currentNotePitch;
char currentNotePitch2;
char currentNoteNumBeats;
int notesCursor;

unsigned long lcdMsgMs = 0;

unsigned long nowMs;
unsigned long prevMs;
unsigned long elapsedMs;

// recorded hits
const byte NO_HIT = 0;
const byte BAD_HIT = 1;
const byte GOOD_HIT = 2;
byte hitRecordedForCurrentBeat = NO_HIT;
byte hitRecordedForNextBeat = NO_HIT;
byte hitRecordedForPrevBeat = NO_HIT;

int hitCount = 0;
int missCount = 0;
int badHitCount = 0;

int level;

byte downArrowImg[8] = 
{
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};
const uint8_t downArrowChar = 0;

byte hitImg[8] = 
{
  B10001,
  B00100,
  B01110,
  B11111,
  B01110,
  B00100,
  B10001,
};
const uint8_t hitChar = 1;

const int numSongs = 2;
int currentSong;

// strings
char p_buffer[10];
char p_buffer_2[10];
#define P(str) (strcpy_P(p_buffer, PSTR(str)), p_buffer)
#define P2(str) (strcpy_P(p_buffer_2, PSTR(str)), p_buffer_2)

// Neil Young
// Like a Hurricane
prog_char hurricaneNotes[] PROGMEM =
 //verse                                             chorus
 //|               |             |               |   |you are...       |             |               |turn     |       |           |         verse                                             chorus
  "E D C b a b C E D C b C D - D E E D C a b C a D - C E D F F F D D b C E D F F D b C E D F F F D D C C a C a C C - E C C a C C a A G E D E E D C b a b C E D C b C D - D E E D C a b C a D - C E D F F F D D b C E D F F D b C E D F F F D D C C a C a C C - E C C a C C a A G E D E E a";
prog_char hurricaneBeats[] PROGMEM = 
  {3,1,2,1,4,1,2,2,3,1,2,1,6,2,1,2,1,3,1,4,1,2,1,8,9,2,2,4,2,1,2,1,1,1,2,2,4,1,2,4,1,2,2,4,2,1,2,1,2,2,1,1,3,1,1,1,1,5,2,1,1,2,1,1,2,1,3,1,1,3,1,2,1,4,1,2,2,3,1,2,1,6,2,1,2,1,3,1,4,1,2,1,8,9,2,2,4,2,1,2,1,1,1,2,2,4,1,2,4,1,2,2,4,2,1,2,1,2,2,1,1,3,1,1,1,1,5,2,1,1,2,1,1,2,1,3,1,1,8,5};
boolean hurricaneRhythm[]  = 
//|       |       |       |       
 {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0, // verse
  1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0,
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,1,1,
  
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0, // chorus
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0,
  1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,
  
  1,0,0,0,1,0,0,0,1,1,0,0,1,0,0,0, // chorus turnaround
  1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,1,
  
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0, // verse
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0,
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0,
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,1,1,
  
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0, // chorus
  1,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0,
  1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,
  
  1,0,0,0,1,0,0,0,1,1,0,0,1,0,0,0, // chorus turnaround
  1,0,0,0,1,0,0,0,1,0,1,0,1,0,1,1,
  
  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // end/ringout  
  
// Coldplay
// Talk
prog_char talkNotes[] PROGMEM =
  //  
  // intro   |     | | | | | |   |   |     | | |                     | verse                 I'm so...           want...       want...                 you don't know...         echo...           you'll tell...                              |                   |               |                 |       
  "C A G E D C D E a a0a0c c d C A G E D C D E a E a E E E E D C g - A A A A G E D C a a0c d E G A A A A G E D C a b C D b a g a b C D b a g a a a a a a C E E E E E a a g a G E C A G E D C D E a a C E E E E E a a g a G E C A G E D C D E a a C E E E E E E E E D b g E a E E E E D C g E a E E E E D C g a";
prog_char talkBeats[] PROGMEM = 
  {1,1,3,1,3,1,1,1,5,4,4,1,2,4,1,1,3,1,3,1,1,1,3,1,6,1,1,1,1,1,1,5,2,2,1,1,1,4,1,2,2,4,4,3,3,1,1,3,1,2,1,4,1,2,2,3,1,2,1,6,1,1,4,1,2,1,6,1,1,1,4,4,4,2,1,1,1,1,1,1,1,1,1,1,2,1,4,1,1,3,1,3,1,1,1,3,1,1,1,1,1,1,1,1,1,1,2,1,4,1,1,3,1,3,1,1,1,3,1,1,1,1,1,1,1,1,1,1,2,1,3,1,6,1,1,1,1,1,1,3,1,6,1,1,1,1,1,1,5,8};

boolean talkRhythm[] = 
//   |       |       |       |       
  {0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0, // intro
     1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0, // intro
     1,0,0,0,1,1,0,0,1,1,0,1,0,0,0,0, // intro
     1,0,0,0,1,1,0,0,1,1,0,1,0,0,0,0, // intro
     
     1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0, // verse
     1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0, // verse
     1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0, // verse
     1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0, // verse 
     1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0, // verse extension
     1,0,0,0,1,1,0,0,1,0,0,0,1,1,1,1, // verse extension
     
     1,0,0,0,1,1,0,0,1,1,0,1,0,1,0,0, // chorus
     1,0,0,0,1,1,0,0,1,1,0,1,0,1,0,0, // chorus
     1,0,0,0,1,1,0,0,1,1,0,1,0,1,0,0, // chorus
     1,0,0,0,1,1,0,0,1,1,0,1,0,1,1,1, // chorus
     1,0,0,0,1,1,0,0,1,1,0,1,0,1,0,0, // chorus turnaround
     1,0,0,0,1,1,0,0,1,1,0,1,0,1,0,0, // chorus
     1,0,0,0,1,1,0,0,1,1,0,1,0,1,0,0, // chorus
     1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // final note
     
     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // extra ring out


prog_char *notes[numSongs] = 
{
  hurricaneNotes,
  talkNotes
};

prog_char *beats[numSongs] = 
{
  hurricaneBeats,
  talkBeats
};

boolean *rhythm[numSongs] =
{
  hurricaneRhythm,
  talkRhythm
};

const int notesLength[numSongs] = 
{
  140,
  150
};

const int rhythmLength[numSongs] = 
{
  304,
  296
};

const int countInBeats[numSongs] = 
{
  8,
  7
};

const int beatLengthMs[numSongs] = 
{
  250,
  230
};

const int numLevels = 2;
float levelPercentages[numLevels];
const int debounceResetMs = 120;  // simple debounce timer to make sure we don't register more than one hit
int debounceCountdownMs = 0;

int halfBeatLengthMs;
int oneQuarterBeatLengthMs;
int threeQuarterBeatLengthMs;

void setup() 
{
  pinMode(speakerPin, OUTPUT);
  pinMode(backlightLedPin, OUTPUT);
  digitalWrite(backlightLedPin, blState);

  // Important note: Generally, debug messages being sent through the serial port to the PC
  // will affect the timing of the program, and will affect the sensitivity of the sensor.
  Serial.begin(9600);
  Serial.println("Program start.");

  // set up custom characters for lcd
  lcd.createChar(downArrowChar, downArrowImg);
  lcd.createChar(hitChar, hitImg);

  // set up the LCD's number of columns and rows: 
  lcd.begin(screenWidthChars, 2);
  
  printIntro();
  
  level = 0;
  startLevel();
}

// on order of ~30 ms per loop?
// around 4 loops for debounce
void loop() 
{
  boolean rhythmCursorJustChanged = false;

  prevMs = nowMs;
  nowMs = millis();
  elapsedMs = nowMs - prevMs;

  // listen for a hit on the sensor
  hitSensorValue = analogRead(hitSensorPin);
  
  updateBacklight();

  if (nowMs - rhythmCursorPrevMs > beatLengthMs[currentSong]) 
  {    
    // move the song by 1 beat
    rhythmCursorPrevMs = nowMs;
    rhythmCursor++;
    rhythmCursorJustChanged = true;
    
    // now that we've moved 1 beat in the song, we need to reset some variables
    hitRecordedForPrevBeat = hitRecordedForCurrentBeat;
    hitRecordedForCurrentBeat = hitRecordedForNextBeat;
    hitRecordedForNextBeat = NO_HIT;

//    Serial.println("---------------------");
//    Serial.print("Beat change: C=");
//    Serial.print(rhythm[currentSong][rhythmCursor]);
//    Serial.print(" N=");
//    Serial.println(rhythm[currentSong][rhythmCursor + 1]);
  }
  
  // update display
  if (rhythmCursorJustChanged)
    printRhythm();
    
  updateLine1();
  
  if (rhythmCursorJustChanged)
    updateAudio();
    
//  Serial.print("debounceCountdownMs=");
//  Serial.println(debounceCountdownMs);
//  if (debounceCountdownMs == 0)
//    loopsSinceDebounceEnd++;
  
  if (rhythmCursor >= 0 && rhythmCursor < rhythmLength[currentSong])
  {
    boolean hitDetected = (hitSensorValue > threshold);
    
    // look at the current song cursor for a successful or a bad hit
    if (hitDetected && debounceCountdownMs == 0)
    {
      debounceCountdownMs = debounceResetMs;
      
      int hitMsFromBeatChange = nowMs - rhythmCursorPrevMs;
      
      boolean hitRecorded = false;
    
//      Serial.print(hitMsFromBeatChange);
//      Serial.print(" ms out of ");
//      Serial.print(beatLengthMs[currentSong]);
//      Serial.println(" ms");
//      Serial.print("hitRecordedForCurrentBeat=");
//      Serial.print(hitRecordedForCurrentBeat);
//      Serial.print(", hitRecordedForNextBeat=");
//      Serial.println(hitRecordedForNextBeat);

      if (!hitRecorded)
      {
        if (hitRecordedForCurrentBeat == NO_HIT)
        {
          if (rhythm[currentSong][rhythmCursor] == true)
          {
            if (hitMsFromBeatChange < halfBeatLengthMs)
              triggerSuccessfulAction(" Great!");
            else
              triggerSuccessfulAction("  Good!");
            hitRecordedForCurrentBeat = GOOD_HIT;
            hitRecorded = true;
          }
        }
      }

      if (!hitRecorded)
      {
        if (hitRecordedForNextBeat == NO_HIT)
        {
          if (rhythmCursor < rhythmLength[currentSong] - 1 && rhythm[currentSong][rhythmCursor + 1] == true)
          {
            if (hitMsFromBeatChange > threeQuarterBeatLengthMs)
            {
              triggerSuccessfulAction(" Great!");
              hitRecordedForNextBeat = GOOD_HIT;
            }
            else if (hitMsFromBeatChange > oneQuarterBeatLengthMs)
            {
              triggerSuccessfulAction("  Good!");
              hitRecordedForNextBeat = GOOD_HIT;
            }
            else
            {
              triggerBadHitAction(" Loser!");
              hitRecordedForNextBeat = BAD_HIT;
//
//              Serial.print("Failed on next beat.");
//              Serial.print(hitMsFromBeatChange);
//              Serial.print(" ms out of ");
//              Serial.print(beatLengthMs[currentSong]);
//              Serial.println(" ms");
//              Serial.print("hitRecordedForCurrentBeat=");
//              Serial.print(hitRecordedForNextBeat);
//              Serial.print(", loopsSinceDebounceEnd=");
//              Serial.println(loopsSinceDebounceEnd);
//              delay(4000);
            }
            hitRecorded = true;
          }
        }
      }
      
      if (!hitRecorded)
      {
        triggerBadHitAction(" Loser!");
        hitRecordedForCurrentBeat = BAD_HIT;
//
//        Serial.print("No hit recorded.");
//        Serial.print(hitMsFromBeatChange);
//        Serial.print(" ms out of ");
//        Serial.print(beatLengthMs[currentSong]);
//        Serial.println(" ms");
//        Serial.print("hitRecordedForCurrentBeat=");
//        Serial.print(hitRecordedForCurrentBeat);
//        Serial.print(", hitRecordedForNextBeat=");
//        Serial.print(hitRecordedForNextBeat);
//        Serial.print(", loopsSinceDebounceEnd=");
//        Serial.println(loopsSinceDebounceEnd);
//        delay(4000);
      }
      //else don't penalize for multiple hits on same beat

    } // end if valid hit detected

    // look at the previous song cursor for a miss
    if (rhythmCursor > 0)
    {  
      if (rhythmCursorJustChanged && rhythm[currentSong][rhythmCursor - 1] == true && hitRecordedForPrevBeat == NO_HIT) // player missed a hit
         triggerMissAction();
    }
  }
  
  // check for end of level
  if (rhythmCursor >= rhythmLength[currentSong] + 4) // go a little past the song so there is time to display hit/miss/etc.
  {
    noTone(speakerPin);
    printScore();
    
    level++;
    if (level > 1)
    {
      printEndGame();
      
      // restart to avoid crashing
      level = 0;
      printIntro();
    }
    
    startLevel();
  }

  // end of loop
  if (debounceCountdownMs > 0)
  {
    debounceCountdownMs -= elapsedMs;
    if (debounceCountdownMs < 0)
      debounceCountdownMs = 0;
  }
} 

void updateAudio()
{
  if (notesCursor < 0)
  {
    playTick();
    notesCursor++;
    currentNoteBeatCount = 0;

    if (notesCursor == 0)
        readCurrentNoteFromProgMem();
  }
  else if (notesCursor >= 0 && notesCursor < notesLength[currentSong])
  {
    playNote(currentNotePitch, currentNotePitch2, (currentNoteBeatCount == 0));

    if (++currentNoteBeatCount >= currentNoteNumBeats)
    {
      notesCursor++;
      currentNoteBeatCount = 0;

      readCurrentNoteFromProgMem();
    }
  }
  else
  {
    noTone(speakerPin);
  }  
}

void readCurrentNoteFromProgMem()
{
  if (notesCursor >= 0 && notesCursor < notesLength[currentSong])
  {
    int x = notesCursor * 2;
    currentNotePitch = pgm_read_byte_near(notes[currentSong] + x);
    currentNotePitch2 = pgm_read_byte_near(notes[currentSong] + x + 1);
    if (currentNotePitch2 == 0x20)
      currentNotePitch2 = '\0';
    currentNoteNumBeats = pgm_read_byte_near(beats[currentSong] + notesCursor);
    //Serial.print("currentNotePitch=");
    //Serial.print(currentNotePitch, HEX);
    //Serial.print("currentNotePitch2=");
    //Serial.println(currentNotePitch2, HEX);
  }
  else
  {
    currentNotePitch = '\0';
    currentNotePitch2 = '\0';
    currentNoteNumBeats = 0;
  }
}

void initLevel()
{
  currentSong = level;
  
  rhythmCursor = 0 - countInBeats[currentSong] - 1;
  currentNoteBeatCount = 0;
  notesCursor = 0 - countInBeats[currentSong];
    
  rhythmCursorPrevMs = 0;
  
  lcdMsgMs = 0;
  
  hitRecordedForCurrentBeat = NO_HIT;
  hitRecordedForNextBeat = NO_HIT;
  hitRecordedForPrevBeat = NO_HIT;
 
  hitCount = 0;
  missCount = 0;
  badHitCount = 0;

  debounceCountdownMs = 0;

  halfBeatLengthMs = 0.50 * beatLengthMs[currentSong];
  oneQuarterBeatLengthMs = 0.25 * beatLengthMs[currentSong];
  threeQuarterBeatLengthMs = 0.75 * beatLengthMs[currentSong];
}

void updateLine1()
{
  if (lcdMsgMs < elapsedMs)
  {
    // the currently printed message has gone stale - clear line 1 of the display
    lcd.setCursor(1, 0);
    lcd.print("       ");
  }
  else
  {
    // decrement the counter
    lcdMsgMs -= elapsedMs;
  }
}
  
void updateBacklight()
{
  if (blBlinkToggleCountCurrent > 0)
  {
    // LED is blinking

    if (nowMs - blPrevMs > blBlinkMs || // timer expired
      blBlinkToggleCountCurrent == blBlinkToggleCountSetting) // blinking was just started
    {
      blPrevMs = nowMs;
      
      // toggle the pin
      if (blState == LOW)
        blState = HIGH;
      else if (blState == HIGH)
        blState = LOW;
        
      digitalWrite(backlightLedPin, blState);
      
      blBlinkToggleCountCurrent--;
    }
  }
  else
  {
    // check for an error state - sometimes the backlight gets stuck "off"
    // we will turn it on in this case
    if (blState == LOW)
    {
      blState = HIGH;
      digitalWrite(backlightLedPin, blState);
    }
  }
}

void triggerSuccessfulAction(String str)
{
  lcd.setCursor(1, 0);
  lcd.print(str);
  //Serial.println(str);
  
  // leave this message up for 1/2 second
  lcdMsgMs = 500;
  
  hitCount++;
  
  //Serial.println(freeRam());
}

void triggerMissAction()
{
  String str = "  Miss!";
  
  lcd.setCursor(1, 0);
  lcd.print(str);
  //Serial.println(str);
  
  // leave this message up for 1/2 second
  lcdMsgMs = 500;
  
  // start blinking the backlight
  blBlinkToggleCountCurrent = blBlinkToggleCountSetting;
  
  missCount++;
}

void triggerBadHitAction(String str)
{
  lcd.setCursor(1, 0);
  lcd.print(str);
  //Serial.println(str);
  
  // leave this message up for 1/2 second
  lcdMsgMs = 500;
  
  // start blinking the backlight
  blBlinkToggleCountCurrent = blBlinkToggleCountSetting;

  badHitCount++;
}

void printRhythm()
{
  int i;
  int charIndex;
  for (i = 0; i < screenWidthChars; i++)
  {
    lcd.setCursor(i, 1);
    charIndex = rhythmCursor + i;
    if (charIndex < 0 || charIndex >= rhythmLength[currentSong])
      lcd.print((char)0x20); // " "
    else if (rhythm[currentSong][charIndex] == true)
      lcd.write(hitChar); // block
    else
      lcd.print((char)0x2D); // "-"
  }
}

void printIntro()
{//return;
  printLcdMsg(P("Mint Tin"), P2(" Hero!  "));
  slideOffScreen();
  printLcdMsg(P("  v1.0  "), P2("(sucka!)"));
  slideOffScreen();
  lcd.clear();
}

void slideOffScreen()
{
  int i;
  for (i = 0; i < screenWidthChars + 1; i++)
  {
    delay(200);
    lcd.scrollDisplayLeft();
  }
}

void printLevel()
{//return;
  int i; 
  
  lcd.clear();
  lcd.print(P("Level "));
  lcd.print(level + 1);
  
  delay(2000);
  
  lcd.setCursor(0, 1);
  lcd.print(P(" Ready? "));

  delay(1000);

  for (i = 0; i < 3; i++)
  {
    lcd.setCursor(0, 1);
    lcd.print(P("  Go!!  "));
  
    delay(200);
  
    lcd.setCursor(0, 1);
    lcd.print(P("        "));

    delay(200);
  }
}

void printScore()
{//return;
  // first figure out the total number of possible successful hits
  int i;
  int possibleHits = 0;
  for (i = 0; i < rhythmLength[currentSong]; i++)
    if (rhythm[currentSong][i] == true) possibleHits++;
    
  printLcdMsg(P(" Level  "), P2(" Score  "));
  
  lcd.clear();
  lcd.print(P("Hits:"));
  lcd.setCursor(0, 1);
  lcd.print(hitCount);
  lcd.print("/");
  lcd.print(possibleHits  );

  delay(2000);
  
  lcd.clear();
  lcd.print(P("Bad hits:"));
  lcd.setCursor(0, 1);
  lcd.print(badHitCount);

  delay(2000);
  
  levelPercentages[level] = (float)hitCount/(float)(possibleHits + badHitCount) * 100.0;
  
  lcd.clear();
  lcd.print(P("Overall:"));
  lcd.setCursor(0, 1);
  lcd.print((int)levelPercentages[level]);
  lcd.print("%");

  delay(2000);
}

void playNote(char notePitch, char notePitch2, boolean attack)
{ 
  const char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C', 'D', 'E', 'F', 'G', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'A', 'B', 'C' };
  const char octav[] = { '0', '0', '0', '0', '0', '0', '0', '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '2', '3' };
  const int tones[] =  { 65,  73,  82,  87,  98,  110, 123, 131, 147, 165, 175, 196, 220, 247, 262, 294, 330, 349, 392, 440, 494, 523, 587, 659, 698, 784, 880, 988, 1047,1175,1319,1397,1568,1760,1976,2093 };

//  Serial.print(note);
//  if (attack)
//    Serial.print(" attack!");
//  Serial.println();

  if (notePitch == '-')
  {
    noTone(speakerPin);
  }
  else
  {
    // play the tone corresponding to the note name
    for (int i = 0; i < 36; i++)
    {
      if (names[i] == notePitch && octav[i] == notePitch2)
      {
        if (attack) playTick();
        tone(speakerPin, tones[i]); 
      }
    }
  }
}

void playTick()
{
  tone(speakerPin, 1000, 10);
}

void startLevel()
{
  initLevel();
  printLevel();

  // print the down arrow
  lcd.setCursor(0, 0);
  lcd.write(downArrowChar);
}

void printEndGame()
{
  float x = 0;
  for (int i = 0; i < numLevels; i++)
  {
    x = x + levelPercentages[i];
    //Serial.println(levelPercentages[i]);
  }
  float finalScore = (float)x/(float)numLevels;
  
  printLcdMsg(P(" Final  "), P2(" Score  "));
    
  lcd.clear();
  lcd.print("  ");
  lcd.print((int)finalScore);
  lcd.print("%");

  delay(2000);
  
  if (finalScore < 50)
  {
    printLcdMsg(P("EMBARRAS"), P2("SING!!!!"));
  }
  else if (finalScore < 60)
  {
    printLcdMsg(P("  Wow,  "), P2("really? "));
    printLcdMsg(P("That was"), P2(" awful. "));
  }
  else if (finalScore < 70)
  {
    printLcdMsg(P("  You   "), P2("  suck! "));
    printLcdMsg(P("No, but "), P2("really. "));
  }
  else if (finalScore < 80)
  {
    printLcdMsg(P(" Meh... "), P2("        "));
    printLcdMsg(P("Kind of "), P2(" lame.  "));
  }
  else if (finalScore < 90)
  {
    printLcdMsg(P("Not too "), P2("shabby. "));
    printLcdMsg(P("  \"B\"   "), P2("student."));
  }
  else if (finalScore < 97)
  {
    printLcdMsg(P(" Great  "), P2("  job!  "));
    printLcdMsg(P(" Almost "), P2("perfect."));
  }
  else
  {
    printLcdMsg(P("Mint Tin"), P2("  God!  "));
  }
  
  blinkScreen(4);

  delay(2000);
  
  printLcdMsg(P("  Game  "), P2("  Over  "));
}

void printLcdMsg(const char* line1, const char* line2)
{
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);

  delay(2000);
}

void blinkScreen(int numTimes)
{
  for (int i = 0; i < numTimes; i++)
  {
    digitalWrite(backlightLedPin, LOW);
    delay(100);
    digitalWrite(backlightLedPin, HIGH);
    delay(100);
  }
}

int freeRam() 
{
  // 762
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
