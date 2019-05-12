#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Adafruit_Fingerprint.h>
#include "Keypad.h"
SoftwareSerial Arduino(11, 10);
SoftwareSerial mySerial(12, 13);           // pin fingerprint
LiquidCrystal lcd(41, 39, 37, 35, 33, 31);    // pin lcd
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

#define LOCK_PIN 4                        // pin lock
#define BELL_PIN 5                       // pin buzzer
#define BUTTON 6                         // pin button

// buzzer
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_C4  262
int melody_notFound[] = {NOTE_C4, 0, 0, 0, 0, 0};                   // melody buzzer not found
int noteDurations_notFound[] = {1, 8, 8, 4, 4, 4};                  // length of the buzzer sounds not found
int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0};    // melody buzzer
int noteDurations[] = {4, 8, 8, 4, 4, 4};                           // length of the buzzer sounds

//keypad
const byte rows = 4;
const byte columns = 4;
char keys[rows][columns] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};
byte rowPins[rows] = {46, 47, 48, 49};
byte columnPins[columns] = {50, 51, 52, 53};
char password[] = {'0', '0', '0', '0'};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

uint8_t id;

int state = 0;
char key = 0;
int numberFinger = 0;
/*------------------ SETUP -------------------*/

void setup() {
  Serial.begin(9600);
  Arduino.begin(4800);
  while (!Serial);
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(BELL_PIN, OUTPUT);
  pinMode(BUTTON, INPUT);

  lcd.begin(16, 2);     // set the data for the lcd port
  finger.begin(57600);  // set the data rate for the sensor serial port

  // check circuit
  Display("SMART LOCK", 0);
  if (finger.verifyPassword()) {
    Display("correct circuit", 1);
    delay(1000);
  }
  else {
    Display("wrong circuit", 1);
    while (1) {
      delay(1);
    }
  }
  Display("SMART LOCK", 0);
  Display("waiting...", 1);
}

/*------------------ LOOP -------------------*/

void loop() {
  char temp = keypad.getKey();
  if ((int)keypad.getState() ==  PRESSED)
  {
    if (temp != 0) key = temp;
    if (key == 'A') state = 1;
    else if (key == '*') state = 2;
    else if (key == 'C') state = 3;
    else if (key == 'D') state = 4;
  }

  if (state == 0) { // check fingerprint
    getFingerprintIDez();
  }
  else if (state == 1) { // add fingerprint
    if (enterPassword() == true) getFingerprintEnroll();
    Display("SMART LOCK", 0);
    Display("waiting...", 1);
    state = 0;
  }
  else if (state == 4) { // remove all fingerprint
    if (enterPassword() == true) deleteAllFingerprint();
    state = 0;
  }
  else if (state == 2) { // remove one fingerprint
    if (enterPassword() == true) deleteOneFingerprint();
    state = 0;
  }
  else if (state == 3) { // print the number of fingerprint
    printNumberFingerPrint();
    state = 0;
  }
}

/*------------------ ENTER PASSWORD -------------------*/

bool enterPassword()
{
  Display("Enter password", 0);
  char a[5];
  int cnt = 0;
  while (cnt <= 3)
  {
    char temp = keypad.getKey();
    if (temp)
    {
      a[cnt] = temp;
      lcd.setCursor(cnt, 1);
      lcd.print(temp);
      delay(100);
      lcd.setCursor(cnt, 1);
      lcd.print("*");
      cnt++;
    }
  }
  for (int i = 0; i < 4; i++)
  {
    if (a[i] != password[i])
    {
      Display("Wrong password", 0);
      delay(2000);
      Display("SMART LOCK", 0);
      Display("waiting...", 1);
      return false;
    }
  }
  Display("Correct password", 0);
  delay(2000);
  return true;
}

/*------------------ DISPLAY -------------------*/

void Display(String msg, int pos)
{
  if (pos == 0) lcd.clear();
  lcd.setCursor(0, pos);
  lcd.print(msg);
  Serial.println(msg);
}

/*------------------ BELL -------------------*/

void bell (int melody[], int noteDurations[]) {
  pinMode(BELL_PIN, OUTPUT);
  for (int thisNote = 0; thisNote < 6; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BELL_PIN, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BELL_PIN);
  }
}

/*------------------ UNLOCK -------------------*/

void unlock() {
  Display("openning...", 1);
  bell(melody, noteDurations);
  digitalWrite(LOCK_PIN, HIGH);
  delay(2000);
  Display("closing...", 1);
  bell(melody, noteDurations);
  digitalWrite(LOCK_PIN, LOW);
  delay(1000);
}

/*------------------ SEND DATA -------------------*/

void sendData (int type, int id) {
  Arduino.print(type); // send type
  Arduino.println("\n");
  Arduino.print(id); // send id
  Arduino.println("\n");
  Serial.print("TYPE : ");
  Serial.println(type);
  Serial.print("ID : ");
  Serial.println(id);
}

/*------------------ NUMBER FINGERPRINT-------------------*/

void printNumberFingerPrint()
{
  Display("SMART LOCK", 0);
  uint8_t id = numberFingerPrint();
  Display((String)(id), 1);
  if (id <= 1) lcd.print(" template");
  else lcd.print(" templates");
  lcd.print(numberFinger);
  delay(3000);
  Display("SMART LOCK", 0);
  Display("waiting...", 1);
}

int numberFingerPrint(){
  finger.getTemplateCount();
  uint8_t id = finger.templateCount;
  return id;
}

/*------------------ CHECK FINGERPRINT-------------------*/
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
  {
    Display("Not found!", 1);
    sendData(0,0);
    bell(melody_notFound, noteDurations_notFound);
    delay(2000);
    Display("SMART LOCK", 0);
    Display("waiting...", 1);
    return -1;
  }

  // found a match!
  sendData(3, (int)(finger.fingerID));
  Serial.println(finger.fingerID);
  lcd.setCursor(0, 0);
  lcd.print("Hello ID #");
  lcd.print(finger.fingerID);
  Serial.print("Chào ID #");
  Serial.print(finger.fingerID);
  unlock();
  Display("SMART LOCK", 0);
  Display("waiting...", 1);
  return finger.fingerID;
}
/*------------------ DELETE ALL FINGERPRINT-------------------*/

void deleteAllFingerprint() {
  Display("DELETING All ... ", 0);
  sendData(4, (int)(numberFinger));
  finger.emptyDatabase();
  
  numberFinger = 0;
  delay(2000);
  Display("SMART LOCK", 0);
  Display("waiting...", 1);
  return;
}

/*------------------ DELETE ONE FINGERPRINT-------------------*/
void deleteOneFingerprint() {
  Display("DELETE FINGERPRINT ... ", 0);
  Display("Enter id: ", 1);
  uint8_t id = getNumber();
  Serial.println("ID to delete : ");
  Serial.println(id);
  Serial.println("num : ");
  Serial.println(numberFingerPrint());
  Serial.println("MAX : ");
  Serial.println(numberFinger);
  
  delay(1000);
  if (id <= numberFinger)
  {
    sendData(2, (int)(id));
    deleteFingerprint(id);
    Display("Deleting id ", 1);
    lcd.print(id);
  }
  else Display("id undefined! ", 1);
  delay(2000);
  Display("SMART LOCK", 0);
  Display("waiting...", 1);
}

uint8_t getNumber (){
  uint8_t id = 0;
  while (1)
  {
    char temp = keypad.getKey();
    if (temp)
    {
      lcd.print(temp);
      if (temp == '#') return id; 
      else id = id*10 + uint8_t(temp-'0');
    }    
  }
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return p;
  }
}

/*--------------- ADD FINGERPRINT ----------------------*/
uint8_t getFingerprintEnroll() {
  int p = -1;
  Display("ADD FINGERPRINT", 0);
  id = numberFinger + 1;
  Serial.println("ID to add : ");
  Serial.println(id);
  Serial.println("num : ");
  Serial.println(numberFingerPrint());
  Serial.println("MAX : ");
  Serial.println(numberFinger);
  if (numberFinger >= 127)
  {
    Display("full memory", 1);
    delay(2000);
    return p;
  }
  Display("waiting...", 1);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
      case FINGERPRINT_OK:
        Display("The first time", 0);
        Display("Success!", 1);
        bell(melody, noteDurations);
        break;
      case FINGERPRINT_NOFINGER:
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
      case FINGERPRINT_IMAGEFAIL:
      default:
        Display("The first time", 0);
        Display("Fail!", 1);
        bell(melody_notFound, noteDurations_notFound);
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
    case FINGERPRINT_PACKETRECIEVEERR:
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
    default:
      Display("Convert error", 1);
      bell(melody_notFound, noteDurations_notFound);
      return p;
  }

  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  p = -1;
  Display("ADD FINGERPRINT", 0);
  Display("again...", 1);
  while (p != FINGERPRINT_OK)
  {
    p = finger.getImage();
    switch (p)
    {
      case FINGERPRINT_OK:
        Display("The second time", 0);
        Display("Success!", 1);
        bell(melody, noteDurations);
        break;
      case FINGERPRINT_NOFINGER:
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
      case FINGERPRINT_IMAGEFAIL:
      default:
        Display("The second time", 0);
        Display("Fail!", 1);
        bell(melody_notFound, noteDurations_notFound);
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
    case FINGERPRINT_PACKETRECIEVEERR:
    case FINGERPRINT_FEATUREFAIL:
    case FINGERPRINT_INVALIDIMAGE:
    default:
      Display("Convert error", 1);
      bell(melody_notFound, noteDurations_notFound);
      delay(2000);
      return p;
  }

  // OK converted!
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Display("Match :)", 0);
  } else {
    Display("Not match :(", 0);
    Display("Exit.", 1);
    delay(2000);
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    sendData(1, (int)(id));
    Display("Stored id ", 1);
    lcd.print(id);
    bell(melody, noteDurations);
    numberFinger++;
  } else {
    Display("Exit.", 1);
    bell(melody_notFound, noteDurations_notFound);
  }
  delay(2000);
  return p;
}
