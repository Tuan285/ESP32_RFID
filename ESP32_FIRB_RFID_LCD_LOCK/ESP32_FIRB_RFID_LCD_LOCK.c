#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//----------------------------------------------------------------//
#define EEPROM_SIZE 512
#define bt_up 25
#define bt_down 33
#define bt_choose 32
#define rl 15
//-------------------------------- LCD ---------------------------//
LiquidCrystal_I2C lcd(0x27, 20, 4);
int option_main = 0;
bool tt_choose = 1, off_option = 1, clean = 1, lock_bt = 1;
bool yes = false, no = false, lock_add_delete = true;
//-------------------------------- RC522 ---------------------------//
#define RST_PIN 0
#define SS_PIN 16
MFRC522 mfrc522(SS_PIN, RST_PIN);
byte master_card[4];
byte read_card[4];
byte store_card[4];
bool program_mode = false;
bool success_read;
//--------------------------FUNCTION INIT--------------------//
int get_id();
bool check_card(byte a[], byte b[]);
bool is_master(byte a[]);
void read_id(int num);
bool find_id(byte id[]);
void write_id(byte a[]);
int find_id_address(byte id[]);
void delete_id(byte a[]);
void button();
void menu(int option);
void main_once_option();
//-------------------------- VOID SET UP  ----------------//
void setup()
{
  Serial.begin(115200);
  pinMode(bt_up, INPUT);
  pinMode(bt_down, INPUT);
  pinMode(bt_choose, INPUT);
  pinMode(rl, OUTPUT);
  digitalWrite(rl, LOW);
  //------------------------------------------------------------------------------------------------//
  EEPROM.begin(EEPROM_SIZE);
  //------------------------------------------------------------------------------------------------//
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  //------------------------------------------------------------------------------------------------//
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);

  // for(int i = 0 ; i < EEPROM_SIZE; i++) {
  //   EEPROM.write(i , 0 );
  //   EEPROM.commit();
  // }
  if (EEPROM.read(1) == 1)
  { // Haved Master Card
    Serial.println("I have Master Card !!");
    Serial.println("-----------------------------------");
  }
  else
  { // Haven't Master Card
    Serial.println("I don't find master card !!");
    Serial.println("-----------------------------------");
    while (success_read != 1)
    {
      success_read = get_id(); // Check Card input
      // Serial.println("Don't Master Card !!");
    }
    for (int i = 0; i < 4; i++)
    {
      EEPROM.write(i + 2, read_card[i]);
      EEPROM.commit(); // comfirm change EEPROM
    }
    EEPROM.write(1, 1);
    EEPROM.commit();

    Serial.println("Completed save Master Card !!");
    Serial.println("-----------------------------------");
  }
  for (int k = 0; k < 4; k++)
  {
    master_card[k] = EEPROM.read(k + 2);
    Serial.print(byte(master_card[k]), HEX);
  }
  delay(2000);
} // END SETUP
//-------------------------- VOID LOOP ----------------//
void loop()
{
  if (get_id() == 1)
  {
    if (is_master(read_card) == true)
    {
      program_mode = !program_mode;
      if (program_mode == true)
      {
        lcd.clear();
        lcd.setCursor(5, 1);
        lcd.print("Mode : Auto");
      }
      else
      {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Options Main !!");
        delay(2000);
        lcd.clear();
        while (true)
        {
          button();
          menu(option_main);
          main_once_option();
          if (get_id() == 1 && is_master(read_card) == true)
          {
            break;
          }
        }
      }
    }
    else if (find_id(read_card) == true)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Mo cua !!");
      digitalWrite(rl, HIGH);
      delay(2000);
      digitalWrite(rl, LOW);
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Mode : Auto");
    }
    else
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sai the !");
      delay(2000);
      lcd.clear();
      lcd.setCursor(5, 1);
      lcd.print("Mode : Auto");
    }
  }
} // END LOOP
//-------------------------- GET ID----------------//
int get_id()
{
  int i;
  if (!mfrc522.PICC_IsNewCardPresent())
  {
    return 0;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial())
  {
    return 0;
  }
  for (i = 0; i < 4; i++)
  { //
    read_card[i] = mfrc522.uid.uidByte[i];
    Serial.print(read_card[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;             // check card empty?
}
//--------------------------CHECK CARD----------------//
bool check_card(byte a[], byte b[])
{
  bool match;
  if (a[0] != NULL)
  { // check card "a" empty ?
    match = true;
    for (int k = 0; k < 4; k++)
    {
      if (a[k] != b[k])
      {
        match = false;
      }
    }
  }
  if (match == true)
  {
    return true;
  }
  else
  {
    return false;
  }
}
//-------------------------- CARD IS MASTER----------------//
bool is_master(byte a[])
{
  if (check_card(a, master_card) == true)
  { // check card "a" is master card ?
    return true;
  }
  else
  {
    return false;
  }
}
//-------------------------- READ CARD IN EPPROM----------------//
void read_id(int num)
{
  int st = (num * 4) + 2; // point to the start position card
  for (int i = 0; i < 4; i++)
  {
    store_card[i] = EEPROM.read(st + i); // save to "store_card"
  }
}
//-------------------------- FIND CARD ----------------//
bool find_id(byte id[])
{
  int count = EEPROM.read(0); // read much card saved
  for (int i = 1; i <= count; i++)
  {
    read_id(i); // read each card
    if (check_card(id, store_card) == true)
    { // compare saved card with "id"
      return true;
      break; // true -> break loop for
    }
  }
  return false;
}
//-------------------------- WRITE CARD TO EEPROM----------------//
void write_id(byte a[])
{
  if (find_id(a) == false)
  { // find "a" in EPPROM
    int num_card = EEPROM.read(0);
    int st = (num_card * 4) + 6;
    num_card++;
    EEPROM.write(0, num_card);
    EEPROM.commit();
    for (int i = 0; i < 4; i++)
    {
      EEPROM.write(st + i, a[i]);
      EEPROM.commit();
    }
  }
}
//-------------------------- FIND ADDRESS CARD NEED DELETE ------------------//
int find_id_address(byte id[])
{
  int num_card = EEPROM.read(0);
  for (int i = 1; i <= num_card; i++)
  {
    read_id(i); //  i = 1 : thẻ thứ nhat -> lưu vào store card
    if (check_card(id, store_card) == true)
    { // ktra card co trong bo nho ?
      return i;
      break;
    }
  }
}
//-------------------------- DELETE CARD ----------------//
void delete_id(byte a[])
{
  if (find_id(a) == false)
  {
    Serial.println("the chua co trong bo nho");
  }
  else
  {
    int num_card = EEPROM.read(0);
    int stt_card_delete;    // so thu tu the
    int start_id;           // dia chi id bắt đầu cần delete
    int loop_shift_left_id; // bien vonng lap dịch id từ trái sang
    int id_need_shift;
    int i;
    stt_card_delete = find_id_address(a);
    start_id = ((stt_card_delete * 4) + 2);
    loop_shift_left_id = (num_card - stt_card_delete) * 4;

    num_card--; // giam 1 the
    EEPROM.write(0, num_card);
    EEPROM.commit();
    for (i = 0; i < loop_shift_left_id; i++)
    {
      id_need_shift = EEPROM.read(start_id + 4 + i); // vd muon xoa the 1 -> doc the 2
      EEPROM.write(start_id + i, id_need_shift);     // the 2 se di chuyen vao dia chi cua the 1
      EEPROM.commit();
    }
    for (int j = 0; j < 4; j++)
    {
      EEPROM.write(start_id + i + j, 0); // dia chi the 2 se reset ve gia tri 0
      EEPROM.commit();
    }
  }
}
//***************** BUTTON *****************//
void button()
{
  if (lock_bt == 1)
  {
    if (digitalRead(bt_up) == 0)
    {
      option_main++;
      if (option_main >= 3)
      {
        option_main = 3;
      }
      delay(50);
      while (digitalRead(bt_up) == 0)
      {
        delay(50);
      }
    }
    if (digitalRead(bt_down) == 0)
    {
      option_main--;
      if (option_main <= 0)
      {
        option_main = 0;
      }
      delay(50);
      while (digitalRead(bt_down) == 0)
      {
        delay(50);
      }
    }
  }
  else
  {
    if (digitalRead(bt_up) == 0)
    { // choose YES
      yes = true;
      lock_add_delete = false;
      delay(50);
      while (digitalRead(bt_up) == 0)
      {
        delay(50);
      }
    }
    if (digitalRead(bt_down) == 0)
    { // choose NO
      no = true;
      lock_add_delete = false;
      delay(50);
      while (digitalRead(bt_down) == 0)
      {
        delay(50);
      }
    }
  }
  if (digitalRead(bt_choose) == 0)
  {
    tt_choose = !tt_choose;   // 1 off
    off_option = !off_option; // 1 on
    lock_bt = !lock_bt;
    clean = 0;
    delay(50);
    while (digitalRead(bt_choose) == 0)
    {
      delay(50);
    }
  }
  else
  {
    clean = 1;
  }
}
//****************** MAIN MENU ****************//
void menu(int option)
{
  if (off_option == 1)
  {
    switch (option)
    {
    case 0:
      lcd.setCursor(2, 0);
      lcd.print(">ADD CARD ");
      lcd.setCursor(2, 1);
      lcd.print(" DELETE CARD ");
      lcd.setCursor(2, 2);
      lcd.print(" OPTION 3 ");
      lcd.setCursor(2, 3);
      lcd.print(" OPTION 4 ");
      break;
    case 1:
      lcd.setCursor(2, 0);
      lcd.print(" ADD CARD ");
      lcd.setCursor(2, 1);
      lcd.print(">DELETE CARD ");
      lcd.setCursor(2, 2);
      lcd.print(" OPTION 3 ");
      lcd.setCursor(2, 3);
      lcd.print(" OPTION 4");
      break;
    case 2:
      lcd.setCursor(2, 0);
      lcd.print(" ADD CARD");
      lcd.setCursor(2, 1);
      lcd.print(" DELETE CARD");
      lcd.setCursor(2, 2);
      lcd.print(">OPTION 3");
      lcd.setCursor(2, 3);
      lcd.print(" OPTION 4");
      break;
    case 3:
      lcd.setCursor(2, 0);
      lcd.print(" ADD CARD");
      lcd.setCursor(2, 1);
      lcd.print(" DELETE CARD");
      lcd.setCursor(2, 2);
      lcd.print(" OPTION 3");
      lcd.setCursor(2, 3);
      lcd.print(">OPTION 4");
      break;
    default:
      lcd.setCursor(2, 0);
      lcd.print(">OPTION 1");
      lcd.setCursor(2, 1);
      lcd.print(" OPTION 2");
      lcd.setCursor(2, 2);
      lcd.print(" OPTION 3");
      lcd.setCursor(2, 3);
      lcd.print(" OPTION 4");
    }
  }
}
//******************* MAIN MENU OPTIONS ***************//
void main_once_option()
{
  if (clean == 0)
  {
    lcd.clear();
  }
  //-------------------------- Main Option 1------------------//
  if (tt_choose == 0 && option_main == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("DO YOU WANT ADD THISCARD ?");
    lcd.setCursor(4, 3);
    lcd.print("YES");
    lcd.setCursor(16, 3);
    lcd.print("NO");
    if (lock_add_delete == false && yes == true)
    { // CHOOSE YES
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please Give Card You Need Add !");
      delay(3000);
      get_id();
      if (is_master(read_card) == true)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("This Card is Master !!");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Return Main Menu");
        delay(1000);
        lcd.clear();
        lock_bt = 1;
        yes = false;
        lock_add_delete = true;
        off_option = 1;
        tt_choose = 1;
      }
      else
      {
        if (find_id(read_card) == false)
        { // Card exist
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Starting Add Card!!");
          write_id(read_card);
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Add Complete!!");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Return Main Menu");
          delay(1000);
          //----------------------------- Retrun Main Menu -------------------------//
          lcd.clear();
          lock_bt = 1;
          yes = false;
          lock_add_delete = true;
          off_option = 1;
          tt_choose = 1;
        }
        else
        { // Card new
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("This Card already exist !!");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Return Main Menu");
          delay(1000);
          lcd.clear();
          lock_bt = 1;
          yes = false;
          lock_add_delete = true;
          off_option = 1;
          tt_choose = 1;
        }
      }
    }
    if (lock_add_delete == false && no == true)
    { // CHOOSE NO
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Return Main Menu");
      delay(3000);
      //----------------------------- Retrun Main Menu -------------------------//
      lcd.clear();
      lock_bt = 1;
      no = false;
      lock_add_delete = true;
      off_option = 1;
      tt_choose = 1;
      if (is_master(read_card) == true)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("This Card is Master !!");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Return Main Menu");
        delay(1000);
        lcd.clear();
        lock_bt = 1;
        no = false;
        lock_add_delete = true;
        off_option = 1;
        tt_choose = 1;
      }
      else
      {
        if (find_id(read_card) == true)
        { // Card exist
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Starting Delete Card !!");
          delay(1000);
          delete_id(read_card);
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Delete Card Complete ");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Return Main Menu");
          delay(1000);
          lcd.clear();
          lock_bt = 1;
          yes = false;
          lock_add_delete = true;
          off_option = 1;
          tt_choose = 1;
        }
        else
        { // Card new
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Card dont exist !!");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Return Main Menu");
          delay(1000);
          //----------------------------- Retrun Main Menu -------------------------//
          lcd.clear();
          lock_bt = 1;
          yes = false;
          lock_add_delete = true;
          off_option = 1;
          tt_choose = 1;
        }
      }
    }
  }
  //-------------------------- Main Option 2 ------------------//
  if (tt_choose == 0 && option_main == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("DO YOU WANT DELETE  THIS CARD ?");
    lcd.setCursor(4, 3);
    lcd.print("YES");
    lcd.setCursor(16, 3);
    lcd.print("NO");
    if (lock_add_delete == false && yes == true)
    { // CHOOSE YES
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Please Give Card You Need Delete !");
      delay(3000);
      get_id();
      if (is_master(read_card) == true)
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("This Card is Master !!");
        delay(1000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Return Main Menu");
        delay(1000);
        lcd.clear();
        lock_bt = 1;
        no = false;
        lock_add_delete = true;
        off_option = 1;
        tt_choose = 1;
      }
      else
      {
        if (find_id(read_card) == true)
        { // Card exist
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Starting Delete Card !!");
          delete_id(read_card);
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Delete Card Complete ");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Return Main Menu");
          delay(1000);
          lcd.clear();
          lock_bt = 1;
          yes = false;
          lock_add_delete = true;
          off_option = 1;
          tt_choose = 1;
        }
        else
        { // Card new
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Card dont exist !!");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Return Main Menu");
          delay(1000);
          //----------------------------- Retrun Main Menu -------------------------//
          lcd.clear();
          lock_bt = 1;
          yes = false;
          lock_add_delete = true;
          off_option = 1;
          tt_choose = 1;
        }
      }
    }
    if (lock_add_delete == false && no == true)
    { // CHOOSE NO
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Return Main Menu");
      delay(3000);
      //----------------------------- Retrun Main Menu -------------------------//
      lcd.clear();
      lock_bt = 1;
      no = false;
      lock_add_delete = true;
      off_option = 1;
      tt_choose = 1;
    }
  }
  if (tt_choose == 0 && option_main == 2)
  {
    lcd.setCursor(0, 0);
    lcd.print("DO YOU WANT EXIT ?");
  }
  if (tt_choose == 0 && option_main == 3)
  {
    lcd.setCursor(0, 0);
    lcd.print("HELLO MAIN OPTION 4");
  }
}