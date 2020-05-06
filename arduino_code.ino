#include <Keypad.h>
#include <LiquidCrystal_PCF8574.h>

// set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_PCF8574 lcd(0x27);

//Setup Keypad
const byte ROWS = 4; // rows
const byte COLS = 4; // columns
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);


/* -------------- Editable values -------------- */
const int amount_menu = 20; //จำนวนเมนูอาหารทั้งหมด
const int menu_word = 41;   //จำนวนตัวอักษรต่อ 1 เมนู
const int maxorder = 30;    //จำนวน order ทั้งหมดในเวลาเดียวกัน
char listmenu[amount_menu][menu_word] = {
  "01: basil leave with pork on rice",          //ผัดกะเพราหมู
  "02: basil leave with crispy pork on rice",   //ผัดกะเพราหมูกรอบ
  "03: basil leave with chicken on rice",       //ผัดกะเพราไก่
  "04: Fried rice with pork",                   //ข้าวผัดหมู
  "05: Fried rice with chicken",                //ข้าวผัดไก่
  "06: Fried rice with shrimps",                //ข้าวผัดกุ้ง
  "07: Fried rice with crabmeat",               //ข้าวผัดปู
  "08: Fried kale vegetable",                   //ผัดผักบุ้ง
  "09: Fried noodle with soya sauce",           //ผัดซีอิ๊ว
  "10: Fried vegetable combination",            //ผัดผักรวมมิตร
  "11: Phat Thai with fresh shrimp",            //ผัดไทยกุ้งสด
  "12: Suki in broth",                          //สุกี้น้ำ
  "13: Suki without broth",                     //สุกี้แห้ง
  "14: Pork with lemon",                        //หมูมะนาว
  "15: Vermicelli yam",                         //ยำวุ้นเส้น
  "16: Instant noodle yam",                     //ยำมาม่า
  "17: Stuffed egg",                            //ไข่ยัดไส้
  "18: Minced prok in omlette",                 //ไข่เจียวหมูสับ
  "19: Fried egg",                              //ไข่ดาว
  "20: Plain rice"                              //ข้าวเปล่า
};
/* --------------------------------------------- */

bool page_enterorder = true;
char code[4] = "", line1[16], menu[menu_word];
long lasttime = 0;
int startindex, stopindex, scrolldelay = 250, select = 0;
int lastorder = 1, totalorder = 0;
int customer_queue, customer_menu;

typedef struct {
  int queue, menu;
} listOrder;

listOrder allorder[maxorder];


void beep(int frequency, int duration) {
  tone(11, frequency, duration);
  delay(duration);
  digitalWrite(11, HIGH);
}

void beep_input() {
  beep(4100, 100);
}

void beep_alert() {
  beep(400, 100);
}

void beep_clear() {
  beep(400, 300);
}

void beep_success() {
  beep(3000, 100);
}


void reset_lcdncode() {
  /* ทำให้ lcd แสดงผลพร้อมสำหรับการรับ order หรือคำสั่ง และทำให้ code ว่างเปล่า */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("----Welcome!----");
  lcd.setCursor(0, 1);
  lcd.print("Enter order: ");
  lcd.blink();
  memset(code, 0, strlen(code));
}

void lcdpreshow() {
  /* ทำให้ lcd พร้อมสำหรับการแสดงผลรายละเอียดต่างๆ ต่อไป*/
  lcd.clear();
  lcd.noBlink();
  lcd.setCursor(0, 0);
}

void lcdshow_order() {
  /* แสดง order ที่เลือกอยู่ */
  lcdpreshow();
  if (select == 0)
    sprintf(line1, "NOW Queue: %d", allorder[select].queue);
  else
    sprintf(line1, "Queue: %d", allorder[select].queue);
  strcpy(menu, listmenu[allorder[select].menu - 1]);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(menu);
  startindex = 0;
  stopindex = findstop();
  lasttime = millis();
}

int findstop() {
  /* return stopindex ของเมนู*/
  if (strlen(menu) > 16)
    return 16;
  return strlen(menu);
}

void lcd_printline2() {
  /* แสดงผลบรรทัดที่ 2 บน lcd โดยมี index กำหนดช่วงของข้อความที่จะแสดง */
  lcd.setCursor(0, 1);
  for (int i = startindex; i < stopindex; i++) {
    lcd.print(menu[i]);
  }
}

void lcdshow_noorder() {
  /* เมื่อไม่มี order ใดๆ ใน allorder จะแสดงผลทาง lcd ว่าไม่มีออเดอร์ */
  lcdpreshow();
  lcd.print("No orders...");
}

void lcdshow_clearinput() {
  /* แสดง ### ในส่วนที่สามารถป้อนตัวเลขได้ เพื่อแสดงให้เห็นว่าได้ลบตัวเลขออกไปแล้ว */
  lcd.setCursor(13, 1);
  lcd.print("###");
  beep_clear();
}


void lcdshow_accept_complete() {
  /* แสดงการรับออเดอร์สำเร็จ โดยแสดง queue ที่ได้ และเมนูที่สั่ง*/
  lcdpreshow();
  sprintf(line1, "Queue %3d Accept", lastorder);
  strcpy(menu, listmenu[customer_menu - 1]);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(menu);

  delay(500);

  //แสดงเมนูโดยเลื่อนไปจนกว่าจะครบทุกตัวอักษรของเมนูนั้นๆ (ระหว่างที่เลื่อน ผู้ใช้งานไม่สามารถทำคำสั่งอื่นๆได้)
  for (int i = 1; i + 16 <= strlen(menu); i++) {
    lcd.setCursor(0, 1);
    for (int j = i; j < i + 16; j++) {
      lcd.print(menu[j]);
    }
    delay(200);
  }

  delay(2000);
}

void lcdshow_cancel_complete(int queue) {
  /* แสดงการยกเลิกออเดอร์สำเร็จ */
  lcdpreshow();
  sprintf(line1, "Queue %d has", queue);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print("been canceled.");
  delay(2000);
}

void lcdshow_done_complete(int queue) {
  /* แสดงการทำเมนูนั้นสำเร็จเรียบร้อยแล้ว */
  lcdpreshow();
  sprintf(line1, "Queue %d Done!", queue);
  lcd.print(line1);
  delay(2000);
}

void beep_lcdshow_fail() {
  /* ส่งเสียงเตือน และแสดงผลทาง lcd ว่า ดำเนินการไม่สำเร็จ */
  lcdpreshow();
  lcd.print("Operation Fail..");
  lcd.setCursor(0, 1);
  lcd.print("Please Try again");
  beep_alert();
  delay(50);
  beep_alert();
  delay(2000);
}

bool foundorder(int customer_queue) {
  /* return true ถ้าเจอ queue นี้ใน order ทั้งหมดที่มีตอนนั้น, false ถ้าไม่เจอ */
  for (int i = 0; i <= totalorder; i++) {
    if (allorder[i].queue == customer_queue)
      return true;
  }
  return false;
}

void update_allorder(int request_queue) {
  /* เช็ค queue ว่าใช่ queue ที่ request มาหรือไม่ , ถ้าเจอแล้ว ให้ตัวก่อนหน้าเท่ากับตัวมันเอง ทำไปจนครบทุก order ที่มีอยู่ */
  bool found_request_queue = false;
  for (int i = 0; i <= totalorder; i++) {
    if (found_request_queue) {
      allorder[i - 1].queue = allorder[i].queue;
      allorder[i - 1].menu = allorder[i].menu;
    }
    else if (allorder[i].queue == request_queue)
      found_request_queue = true;
  }
}

//Uncomment this function to enable show allorder in the serial monitor.
/*
  void check_allorder() {
  for (int i = 0; i < maxorder; i++) {
    Serial.print("queue ");
    Serial.print(allorder[i].queue);
    Serial.print(": ");
    Serial.print(allorder[i].menu);
    Serial.print("# ");
    if ((i+1)%5 == 0)
      Serial.println();
  }
  Serial.println("---------------------------------------------");
  }
*/


void setup() {
  // Uncomment the line below to enable show allorder in the serial monitor.
  // Serial.begin(9600);

  pinMode(11, OUTPUT); //Set pin for Buzzer
  digitalWrite(11, HIGH); //set noactive buzzer (buzzer Active LOW)

  lcd.begin(16, 2); // initialize the lcd
  lcd.setBacklight(255);
  reset_lcdncode(); // การ reset เหมือนกับการตั้งค่าให้พร้อมกับการรับออเดอร์เช่นกัน
}

void loop() {
  char customKey = customKeypad.getKey();
  if (page_enterorder) {
    if (customKey) {
      if (customKey == '*') {
        beep_success();
        if (totalorder > 0)
          lcdshow_order();
        else
          lcdshow_noorder();
        page_enterorder = false;
      }

      else if (customKey == '#') { //# means Clear all code
        lcdshow_clearinput();
        reset_lcdncode();
      }

      else if (customKey == 'A') { //A means Accept code
        customer_menu = atoi(code);
        if (customer_menu >= 1 && customer_menu <= amount_menu) {
          beep_success();
          allorder[totalorder].queue = lastorder;
          allorder[totalorder].menu = customer_menu;
          lcdshow_accept_complete();
          lastorder++;
          totalorder++;
        }
        else
          beep_lcdshow_fail();
        reset_lcdncode();
      }

      else if (customKey == 'B') {
        //Uncomment this statement to enable show allorder in the serial monitor.
        /*
          beep_success();
          check_allorder();
        */
      }

      else if (customKey == 'C') { //C means Cancel order
        customer_queue = atoi(code);

        //Put only 'C' or lastorder = Cancel lastorder --> and decrease lastorder
        if (customer_queue == 0 && totalorder > 0 || customer_queue == totalorder) {
          customer_queue = allorder[totalorder - 1].queue;
          lastorder--;
        }

        if (foundorder(customer_queue) && customer_queue > 0) {
          beep_success();
          lcdshow_cancel_complete(customer_queue);
          update_allorder(customer_queue);
          totalorder--;
        }
        else
          beep_lcdshow_fail();
        reset_lcdncode();
      }

      else if (customKey == 'D') {
        customer_queue = atoi(code);

        //Put only 'D' = Done lastorder
        if (customer_queue == 0)
          customer_queue = allorder[0].queue;

        if (foundorder(customer_queue) && customer_queue > 0) {
          beep_success();
          lcdshow_done_complete(customer_queue);
          update_allorder(customer_queue);
          totalorder--;
        }
        else
          beep_lcdshow_fail();
        reset_lcdncode();

      }

      else if (strlen(code) < 3) {
        code[strlen(code)] = customKey;
        lcd.print(customKey);
        beep_input();
      }

      else // strlen(code) >= 3
        beep_alert();

    }
  }
  else {
    long now = millis();

    if (startindex == 0 || stopindex == strlen(menu))
      scrolldelay = 1000;
    else
      scrolldelay = 250;

    if (now - lasttime > scrolldelay && totalorder > 0) {
      lasttime = now;
      if (stopindex == strlen(menu)) {
        startindex = 0;
        stopindex = findstop();
      }
      else {
        startindex++;
        stopindex++;
      }
      lcd_printline2();
    }

    if (customKey) {
      if (customKey == '*') {
        select = 0;
        beep_success();
        reset_lcdncode();
        page_enterorder = true;
      }
      else if (customKey == '2' && select > 0) {
        select--;
        beep_success();
        lcdshow_order();
      }
      else if (customKey == '5' && select + 1 < totalorder) {
        select++;
        beep_success();
        lcdshow_order();
      }
      else if ((customKey == 'C' || customKey == 'D') && totalorder > 0) {
        beep_success();
        if (customKey == 'C')
          lcdshow_cancel_complete(allorder[select].queue);
        else
          lcdshow_done_complete(allorder[select].queue);

        update_allorder(allorder[select].queue);
        totalorder--;

        select = 0;
        lcd.clear();
        if (totalorder > 0)
          lcdshow_order();
        else
          lcdshow_noorder();
      }
      else
        beep_alert();
    }
  }
  delay(10);
}
