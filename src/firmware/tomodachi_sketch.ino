
#include <Wire.h> // i2c
#include <KY040.h> //rotary encoder
#include <RTClib.h> //real time
#include <Preferences.h>

//display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "animations.h"

//---------------
//animations

// structure to hold animation state information (currentTick) and animation info
// * const bool* anim - bool pointer to first element of the animation
// * uint8_t fps - frames per second
// * uint8_t width - width of animation in pixels
// * uint8_t height - height of animation in pixels
// * bool loop - whether the animation should return to the first frame once done
// * int currentTick - current time of the animation playing (always set to zero)
struct Animation {
  const bool* anim;
  uint8_t fps;
  uint8_t frames;
  uint8_t width;
  uint8_t height;
  bool loop;
  int currentTick = 0; //milliseconds, tracks current time
};
struct Animation pandaAnimation = {
  &(PANDAIDLE[0][0][0]),
  3, //fps
  2, //frames
  48 , //width
  48, //height
  true, //loop
  0
};
struct Animation foodIcon = {
  &(FOODICON[0][0][0]),
  1,
  1,
  16,
  16,
  false,
  0
};
struct Animation happinessIcon = {
  &(HAPPINESSICON[0][0][0]),
  1,
  1,
  16,
  16,
  false,
  0
};
struct Animation handIcon = {
  &(HANDICON[0][0][0]),
  1,
  1,
  15,
  15,
  false,
  0
};
struct Animation foodItem = {
  &(FOODITEM[0][0][0]),
  1,
  1,
  32,
  32,
  false,
  0
};
struct Animation skullFrame = {
  &(SKULLFRAME[0][0][0]),
  1,
  1,
  32,
  32,
  false,
  0
};

//---------------

#define TICK_SPEED 50

#define ROTARY_CLK_PIN 0
#define ROTARY_DT_PIN 1
#define ROTARY_SW_PIN 2

#define SDA_PIN 4
#define SCL_PIN 5
#define SSD1306_ADDR 0x3C
#define RTC_ADDR 0x68
#define NVM_NAMESPACE "nvm"

//screen

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int curScreen = 0;

bool doHighlight = true;
void selectHighlight(int x, int total){
  if(selectv()%total==x && doHighlight) display.setTextColor(BLACK, WHITE); else display.setTextColor(WHITE);
}
void playAnimation(
  Animation &anim,
  int x,
  int y
){
  playAnimation(anim, x, y, true);
}
void playFlippedAnimation(
  Animation &anim,
  int x,
  int y
) {
  bool finished = anim.currentTick >= (int)((1000.0/anim.fps)*(anim.frames));
  uint8_t frame = min((int)(anim.currentTick/(1000.0/anim.fps)), anim.frames-1);
  for(int i = 0; i < anim.height; i++){
    for(int j = 0; j < anim.width; j++){
      display.drawPixel(x+j, y+i, anim.anim[anim.frames*anim.width*anim.height - 1 - (frame*anim.width*anim.height + anim.width*i + j)]);
    }
  }
  if(anim.frames <= 1) return;
  
  if(!finished) {
    anim.currentTick += TICK_SPEED;
  } else if(anim.loop){
    anim.currentTick = 0;
  }
}
void playAnimation(
  Animation &anim,
  int x,
  int y,
  bool overwrite
){
  bool finished = anim.currentTick >= (int)((1000.0/anim.fps)*(anim.frames));
  uint8_t frame = min((int)(anim.currentTick/(1000.0/anim.fps)), anim.frames-1);
  for(int i = 0; i < anim.height; i++){
    for(int j = 0; j < anim.width; j++){
      if(!overwrite && !anim.anim[frame*anim.width*anim.height + anim.width*i + j]){
        continue;
      }
      display.drawPixel(x+j, y+i, anim.anim[frame*anim.width*anim.height + anim.width*i + j]);
    }
  }
  if(anim.frames <= 1) return;
  
  if(!finished) {
    anim.currentTick += TICK_SPEED;
  } else if(anim.loop){
    anim.currentTick = 0;
  }
}
//rtc -----
RTC_DS3231 rtc;
const int monthDays[12] = {31, 30, 29, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // i literally hate this calendar

//rotary encoder -----
volatile int select_x=0;
KY040 g_rotaryEncoder(ROTARY_CLK_PIN,ROTARY_DT_PIN);

// ISR to handle the interrupts for CLK and DT
void ISR_rotaryEncoder() {
  // Process pin states for CLK and DT
  switch (g_rotaryEncoder.getRotation()) {   
    case KY040::CLOCKWISE:
      select_x++;
      break;
    case KY040::COUNTERCLOCKWISE:
      select_x--;
      break;
  }
}
bool isPushed(){
  return digitalRead(ROTARY_SW_PIN) == 0;
}
bool _wasUp = true;
bool isClicked(){
  bool pushed = isPushed();
  if(_wasUp && pushed){
    _wasUp = false;
    return true;
  } else if(!_wasUp && !pushed){
    _wasUp = true;
  }
  return false;
}
int vOffset = 0;
void resetv() {
  vOffset = selectv();
}
void resetv(int to){
  vOffset = selectv()-to;
}
int selectv(){
  return (select_x-vOffset) < 0 ? 20+((select_x-vOffset)%20) : (select_x-vOffset)%20;
}
// non-volatile memory
#define INITIALIZED_KEY "initialized"
#define HUNGER_KEY "HUNGER"
#define HAPPINESS_KEY "HAPPINESS"
#define LASTATE_KEY "LASTATE"
#define DIED_KEY "DIED"
#define LASTCHANGE_KEY "LASTBOOT"
#define TIMEADJUSTED "TIMEADJUSTED"
const int MAX_HAPPINESS = 100;
const int MAX_HUNGER = 100;

Preferences nvm;

//---------------------
const int MAX_INIT_TRIES = 20;
const int HUNGER_DECREASE_INTERVAL = 1000; //every 1000 seconds panda hunger goes down by 1, in an hour loses 3.6 hunger
const int HAPPINESS_DECREASE_INTERVAL = 1200; //every 1200 seconds panda hunger goes down by 1, in an hour loses 3 hunger

int hunger = 100;
int happiness = 100;
long lastAte = -1;
bool died = false;

long lastChange = -1; //epoch milliseconds (from 1970)
DateTime currentBoot;

void initMemory(){
  if(!nvm.getBool(INITIALIZED_KEY, false)){
    Serial.println("Initializing default parameters");
    nvm.putInt(HUNGER_KEY, 100);
    nvm.putInt(HAPPINESS_KEY, 100);
    nvm.putLong(LASTATE_KEY, -1);
    nvm.putBool(DIED_KEY, false);
    nvm.putLong(LASTCHANGE_KEY, -1);
    nvm.putBool(INITIALIZED_KEY, true);
  } else {
    happiness = nvm.getInt(HAPPINESS_KEY);
    hunger = nvm.getInt(HUNGER_KEY);
    lastAte = nvm.getLong(LASTATE_KEY);
    lastChange = nvm.getLong(LASTCHANGE_KEY);
    died = nvm.getBool(DIED_KEY); 
  }
}
void pandaProcesses(){
  long curTime = (long)(rtc.now().unixtime());
  if(lastChange > -1 && curTime > lastChange){
    if(curTime - lastChange > min(HUNGER_DECREASE_INTERVAL, HAPPINESS_DECREASE_INTERVAL) && !died){
      happiness -= (int)((curTime - lastChange)/HAPPINESS_DECREASE_INTERVAL);
      hunger -= (int)((curTime - lastChange)/HUNGER_DECREASE_INTERVAL);

      lastChange = curTime;
      nvm.putLong(LASTCHANGE_KEY, curTime);
    }
  }
  if((hunger >= 200 || hunger <= 0) && !died){
    died = true; // rip
    saveStatus();
  }
}
void saveStatus(){
  nvm.putInt(HUNGER_KEY, hunger);
  nvm.putInt(HAPPINESS_KEY, happiness);
  nvm.putBool(DIED_KEY, died);
}

//----------------------------------------
void setup() {
  Serial.begin(9600);
  Wire.begin(20, 21);
  Serial.println("Initializing");
  uint8_t displayTries = 0;
  uint8_t rtcTries = 0;
  uint8_t nvmTries = 0;
  while(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    delay(10);
  }
  while(!rtc.begin(&Wire)){
    Serial.println("Could not initialize rtc, retrying...");
    if(rtcTries > MAX_INIT_TRIES){
      display.clearDisplay();
      display.setCursor(0,0);
      display.write("RTC FAILURE");
      display.display();
      while(1) delay(100);
    }
    rtcTries++;
    delay(10);
  }
  while(!nvm.begin(NVM_NAMESPACE, false)){
    Serial.println("Could not initialize nvm, retrying...");
    if(nvmTries > MAX_INIT_TRIES){
      display.clearDisplay();
      display.setCursor(0,0);
      display.write("NVM (NON-VOLATILE MEMORY) FAILURE");
      display.display();
      while(1) delay(100);
    }
    nvmTries++;
    delay(10);
  }

  attachInterrupt(digitalPinToInterrupt(ROTARY_CLK_PIN), ISR_rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_DT_PIN), ISR_rotaryEncoder, CHANGE);
  pinMode(ROTARY_SW_PIN, INPUT);
  Serial.println("Passed pin initialization");
  initMemory();
  Serial.println("Passed memory initialization");
  if(lastChange >= -1){
    DateTime dtnow = rtc.now();
    currentBoot = dtnow;
    nvm.putLong(LASTCHANGE_KEY, dtnow.unixtime());
  }
  if(nvm.getBool(TIMEADJUSTED, false)){
    curScreen = 1;
  }
}
//----------------------------------------
void loop() {
  display.clearDisplay();

  cli();
  sei();
  switch (curScreen) {
    case 0:
      drawTimeAdjustScreen();
      break;
    case 1:
      drawHomeScreen();
      pandaProcesses();
      break;
    default:
      display.setTextColor(WHITE);
      display.setCursor(0, SCREEN_HEIGHT/2-4);
      display.write("INVALID SCREEN");
      display.display();
  }
  //tick speed; determines screen update frequency & check intervals for input and durations
  delay(TICK_SPEED);
}

//----------------------------------------
void drawHomeScreen(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  if(died)
  playAnimation(
    skullFrame, 
    (SCREEN_WIDTH-skullFrame.width)/2,
    (SCREEN_HEIGHT-skullFrame.height)/2
  );
  else if(happiness > 0)
  playAnimation(
    pandaAnimation, 
    (SCREEN_WIDTH-pandaAnimation.width)/2,
    (SCREEN_HEIGHT-pandaAnimation.height)/2
  );
  else
  playFlippedAnimation(
    pandaAnimation, 
    (SCREEN_WIDTH-pandaAnimation.width)/2,
    (SCREEN_HEIGHT-pandaAnimation.height)/2
  );

  display.setTextSize(1);
  playAnimation(
    foodIcon, 
    2, 
    (SCREEN_HEIGHT-foodIcon.height-2)
  );
  display.setCursor(2+foodIcon.width+2, (SCREEN_HEIGHT)-2-(foodIcon.height)/2-4);
  if(selectv()%5==1) display.setTextColor(BLACK, WHITE); else display.setTextColor(WHITE);
  display.write(String(hunger).c_str());

  playAnimation(
    happinessIcon, 
    (SCREEN_WIDTH-happinessIcon.width)/2 - 8, 
    (SCREEN_HEIGHT-happinessIcon.height-2)
  );
  display.setCursor((SCREEN_WIDTH + happinessIcon.width)/2 - 6, (SCREEN_HEIGHT)-2-(happinessIcon.height)/2-4);
  if(selectv()%5==2) display.setTextColor(BLACK, WHITE); else display.setTextColor(WHITE);
  display.write(String(happiness).c_str());

  if(selectv()%5==3){
    playAnimation(
      handIcon,
      (SCREEN_WIDTH-handIcon.width)/2,
      5
    );
  }
  if(selectv()%5==4){
    playAnimation(
      foodItem,
      (SCREEN_WIDTH-handIcon.width)/2 - 40,
      5,
      false
    );
  }

  display.drawPixel(0, SCREEN_HEIGHT-1, WHITE);
  display.drawPixel(SCREEN_WIDTH-1, SCREEN_HEIGHT-1, WHITE);
  display.drawPixel((SCREEN_WIDTH)/2, SCREEN_HEIGHT-1, WHITE);
  display.drawPixel((SCREEN_WIDTH)/2+1, SCREEN_HEIGHT-1, WHITE);
  display.display();

  if(isClicked()){
    switch(selectv()%5){
      case 1:
        hunger++;
        saveStatus();
        break;
      case 2:
        saveStatus();
        break;
      case 3:
        happiness++;
        saveStatus();
        break;
      case 4:
        hunger+=10;
        if(hunger > MAX_HUNGER+1){
          happiness-=5;
        }
        saveStatus();
        break;
      default:
        break;
    }
  }
}
void drawTimeAdjustScreen(){
  //static vars;
  static int year = 2026;
  static int month = 1;
  static int day = 1;
  static int hour = 12;
  static int minute = 0;
  static int timeSelected = -1;
  static DateTime dt;
  static int prev_x = select_x;
  //-----

  uint16_t t_width, t_height; 
  int16_t _x, _y;

  int trigoffset;
  bool draw = true;
  display.clearDisplay();

  //draw hours
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.getTextBounds("00:00", 0, 0, &_x, &_y, &t_width, &t_height);
  display.setCursor((SCREEN_WIDTH-t_width)/2, (SCREEN_HEIGHT-t_height)/2-8);

  selectHighlight(1, 7);
  display.write(String(hour).c_str());
  display.setTextColor(WHITE);
  display.write(":");
  selectHighlight(2, 7);
  display.write(String(String(minute < 10 ? "0" : "")+String(minute)).c_str());
  display.setTextColor(WHITE);
  
  display.getTextBounds("00:00:0000", 0, 0, &_x, &_y, &t_width, &t_height);
  display.setCursor((SCREEN_WIDTH-t_width)/2, (SCREEN_HEIGHT-t_height)/2 + 8);
  selectHighlight(3, 7);
  display.write(String(String(month < 10 ? "0" : "")+String(month)).c_str());
  display.setTextColor(WHITE);
  display.write("/");
  selectHighlight(4, 7);
  display.write(String(String(day < 10 ? "0" : "")+String(day)).c_str());
  display.setTextColor(WHITE);
  display.write("/");
  selectHighlight(5, 7);
  display.write(String(year).c_str());

  display.setTextSize(1);
  selectHighlight(6, 7);
  display.getTextBounds("Set current time", 0, 0, &_x, &_y, &t_width, &t_height);
  display.setCursor((SCREEN_WIDTH-t_width)/2, SCREEN_HEIGHT-t_height); 
  if(!draw) display.setTextColor(BLACK, WHITE);
  display.write("Set current time");
  display.display();

  int inc = select_x > prev_x ? 1 : select_x < prev_x ? -1 : 0;
  if(prev_x != select_x){
    prev_x = select_x;
  }
  switch(timeSelected){
    case 1:
      hour+=inc;
      break;
    case 2:
      minute+=inc;
      break;
    case 3:
      month+=inc;
      break;
    case 4:
      day+=inc;
      break;
    case 5:
      year+=inc;
      break;
    default:
      break;
  }

  //bound checking
  if(hour >= 24){
    hour = 0;
  } else if(hour < 0){
    hour = 23;
  }
  if(minute >= 60){
    minute = 0;
    hour++;
  } else if(minute < 0){
    hour--;
    minute=59;
  }
  if(month > 12){
    month = 1;
    year++;
  } else if(month < 1){
    month = 12;
    year--;
  }
  //leap year handling (terrible)
  if(year%4!=0){
    if(month == 3 && day < 1){
      month = 2;
      day = 28;
    } else if(month == 2 && day > 28){
      month = 3;
      day = 1;
    }
  }
  //---
  //pleaswe donmt make me check for overflow again
  if(day > monthDays[month-1]){
    month++;
    day = 1;
  } else if(day < 1){
    month--;
    day = monthDays[month-1];
  }

  Serial.println(select_x);
  Serial.println(vOffset);
  Serial.println(selectv());
  Serial.println(selectv()%7);
  Serial.println(timeSelected);

  //sw func
  if(isClicked()){
    if(timeSelected >= 0){
      doHighlight = true;
      resetv(timeSelected);
      timeSelected = -1;
    } else if(selectv()%7 != 0 && selectv()%7 != 6) {
      doHighlight = false;
      timeSelected = selectv()%7;
    } else if(selectv()%7 == 6) {
      curScreen = 1;
      resetv();
      // set rtc stuff here
      const DateTime newDt = DateTime(
          year,
          month,
          day,
          hour,
          minute
      );
      if(nvm.getLong(LASTCHANGE_KEY, -1) == -1){
        nvm.putLong(LASTCHANGE_KEY, newDt.unixtime());
        currentBoot = newDt;
      }
      rtc.adjust(newDt);
      nvm.putBool(TIMEADJUSTED, true);
    }
  }
}
