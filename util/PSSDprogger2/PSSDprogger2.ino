
#define TYPE_REG 0x01
#define HW_VERSION_REG 0x02
#define FW_VERSION_REG 0x03
#define NAME_REG 0x04

#define ADD_A_REG 0x10
#define ADD_B_REG 0x20

#define SHORT_A_REG_L 0x11
#define SHORT_A_REG_H 0x12
#define LONG_A_REG_L 0x13
#define LONG_A_REG_H 0x14

#define SHORT_B_REG_L 0x21
#define SHORT_B_REG_H 0x22
#define LONG_B_REG_L 0x23
#define LONG_B_REG_H 0x24

#define POSITION_A_REG 0xE0
#define POSITION_B_REG 0xF0

#define NULL_REGISTER 0xFF


#include <I2C.h>
#include <SerialUI.h>

uint8_t buffer[2];

// the strings we'll use
SUI_DeclareString(device_greeting, 
 "+++ Welcome to the PSSD Progger +++\r\nEnter ? for help.");

SUI_DeclareString(top_menu_title, "PSSD Main Menu");

SUI_DeclareString(info_key,"Info");
SUI_DeclareString(info_help, "Connect to device and retrieve info");

SUI_DeclareString(change_servo_key,"Change servo settings");
SUI_DeclareString(change_servo_help, "Change the address and range of the selected servo");

SUI_DeclareString(choose_servo_key, "Choose servo");
SUI_DeclareString(swap_lr_key, "Swap left/right");
SUI_DeclareString(add_key, "Address");
SUI_DeclareString(set_left_key, "Left");
SUI_DeclareString(set_right_key, "Right");
SUI_DeclareString(show_current_servo_key, "Display settings");
SUI_DeclareString(throw_key, "Throw switch");

// our global-scope SerialUI object
SUI::SerialUI mySUI = SUI::SerialUI(device_greeting);

struct device_info {
  uint8_t type;
  uint8_t hw_ver;
  uint8_t fw_ver;
};

struct servo_info {
  uint8_t add;
  uint16_t sh;
  uint16_t lg;
  uint8_t pos;
};
device_info dev;
servo_info servo[2];

uint8_t current_servo = 0;

uint8_t trin_address_lookup(uint8_t dec_address) {
  static const  uint8_t add_lookup[256] PROGMEM =
  {
    0xAA, 0xC0, 0x80, 0x30, 0xF0, 0xB0, 0x20, 0xE0, 0xA0, 0x0C, 0xCC, 0x8C, 0x3C, 0xFC, 0xBC, 
0x2C, 0xEC, 0xAC, 0x08, 0xC8, 0x88, 0x38, 0xF8, 0xB8, 0x28, 0xE8, 0xA8, 0x03, 0xC3, 0x83, 
0x33, 0xF3, 0xB3, 0x23, 0xE3, 0xA3, 0x0F, 0xCF, 0x8F, 0x3F, 0xFF, 0xBF, 0x2F, 0xEF, 0xAF, 
0x0B, 0xCB, 0x8B, 0x3B, 0xFB, 0xBB, 0x2B, 0xEB, 0xAB, 0x02, 0xC2, 0x82, 0x32, 0xF2, 0xB2, 
0x22, 0xE2, 0xA2, 0x0E, 0xCE, 0x8E, 0x3E, 0xFE, 0xBE, 0x2E, 0xEE, 0xAE, 0x0A, 0xCA, 0x8A, 
0x3A, 0xFA, 0xBA, 0x2A, 0xEA, 0x00, 0x40, 0x60, 0x97, 0x70, 0x48, 0x68, 0x58, 0x78, 0x44, 
0x64, 0x54, 0x74, 0x4C, 0x6C, 0x5C, 0x7C, 0x42, 0x62, 0x52, 0x72, 0x4A, 0x6A, 0x5A, 0x7A, 
0x46, 0x66, 0x56, 0x76, 0x4E, 0x6E, 0x5E, 0x7E, 0x41, 0x61, 0x51, 0x71, 0x49, 0x69, 0x59, 
0x79, 0x45, 0x65, 0x9F, 0x75, 0x4D, 0x6D, 0x5D, 0x7D, 0x43, 0x63, 0x53, 0x73, 0x4B, 0x6B, 
0x5B, 0x7B, 0x47, 0x67, 0x57, 0x77, 0x4F, 0x6F, 0x5F, 0x7F, 0x10, 0x18, 0x14, 0x1C, 0x12, 
0x1A, 0x16, 0x1E, 0x11, 0x19, 0x15, 0x1D, 0x13, 0x1B, 0x17, 0x1F, 0xD0, 0xD8, 0xD4, 0xDC, 
0xD2, 0xDA, 0xD6, 0xDE, 0xD1, 0xD9, 0xD5, 0xDD, 0xD3, 0xDB, 0xD7, 0xDF, 0x90, 0x98, 0x94, 
0x9C, 0x92, 0x9A, 0x96, 0x9E, 0x91, 0x99, 0x95, 0x9D, 0x93, 0x9B, 0x50, 0x55, 0x04, 0x06, 
0x05, 0x07, 0xC4, 0xC6, 0xC5, 0xC7, 0x84, 0x86, 0x85, 0x87, 0x34, 0x36, 0x35, 0x37, 0xF4, 
0xF6, 0xF5, 0xF7, 0xB4, 0xB6, 0xB5, 0xB7, 0x24, 0x26, 0x25, 0x27, 0xE4, 0xE6, 0xE5, 0xE7, 
0xA4, 0xA6, 0xA5, 0xA7, 0x01, 0xC1, 0x81, 0x31, 0xF1, 0xB1, 0x21, 0xE1, 0xA1, 0x0D, 0xCD, 
0x8D, 0x3D, 0xFD, 0xBD, 0x2D, 0xED, 0xAD, 0x09, 0xC9, 0x89, 0x39, 0xF9, 0xB9, 0x29, 0xE9, 0xA9};
  return pgm_read_byte(&(add_lookup[dec_address]));
};

uint8_t dec_address_lookup(uint8_t trin_address) {
  static const uint8_t add_lookup[256] PROGMEM =
  {
    80, 2, 81, 1, 6, 8, 82, 7, 145, 177, 191, 161, 3, 5, 84, 4, 18, 20, 85, 19, 24, 26, 
86, 25, 146, 178, 87, 162, 21, 23, 88, 22, 193, 201, 89, 197, 217, 225, 90, 221, 
147, 179, 91, 163, 205, 213, 92, 209, 9, 11, 93, 10, 15, 17, 94, 16, 148, 180, 95, 
164, 12, 14, 96, 13, 54, 56, 97, 55, 60, 62, 98, 61, 149, 181, 99, 165, 57, 59, 100, 
58, 72, 74, 101, 73, 78, 0, 102, 79, 150, 182, 103, 166, 75, 77, 104, 76, 194, 202, 
105, 198, 218, 226, 106, 222, 151, 183, 107, 167, 206, 214, 108, 210, 63, 65, 109, 
64, 69, 71, 110, 70, 152, 184, 111, 168, 66, 68, 112, 67, 229, 231, 113, 230, 235, 
237, 114, 236, 153, 185, 115, 169, 232, 234, 116, 233, 247, 249, 117, 248, 253, 255, 
118, 254, 154, 186, 119, 170, 250, 252, 120, 251, 195, 203, 121, 199, 219, 227, 122, 
223, 155, 187, 192, 171, 207, 215, 124, 211, 238, 240, 125, 239, 244, 246, 126, 245, 
156, 188, 127, 172, 241, 243, 128, 242, 27, 29, 129, 28, 33, 35, 130, 34, 157, 189, 
131, 173, 30, 32, 132, 31, 45, 47, 133, 46, 51, 53, 134, 52, 158, 190, 135, 174, 48, 
50, 136, 49, 196, 204, 137, 200, 220, 228, 138, 224, 159, 83, 139, 175, 208, 216, 
140, 212, 36, 38, 141, 37, 42, 44, 142, 43, 160, 123, 143, 176, 39, 41, 144, 40};

  return pgm_read_byte(&(add_lookup[trin_address]));
};

void read_servo(uint8_t idx){
  I2c.begin();
  I2c.read(0x10, ADD_A_REG + (0x10 * idx), 1, buffer);
  servo[idx].add = dec_address_lookup((uint8_t) *buffer);
  I2c.end();
  buffer[0] = 0;
  buffer[1] = 0;
  I2c.begin();
  I2c.read(0x10, SHORT_A_REG_H + (0x10 * idx), 1, buffer+1);
  I2c.end();
//  mySUI.println(buffer[0], HEX);
  I2c.begin();
  I2c.read(0x10, SHORT_A_REG_L + (0x10 * idx), 1, buffer+0);
  I2c.end();
//  mySUI.println(buffer[1], HEX);
  servo[idx].sh = *(uint16_t*)buffer;
  buffer[0] = 0;
  buffer[1] = 0;
  I2c.begin();
  I2c.read(0x10, LONG_A_REG_H + (0x10 * idx), 1, buffer+1);
  I2c.end();
//  mySUI.println(buffer[0], HEX);
  I2c.begin();
  I2c.read(0x10, LONG_A_REG_L + (0x10 * idx), 1, buffer+0);
  I2c.end();
//  mySUI.println(buffer[1], HEX);
  servo[idx].lg = *(uint16_t*)buffer;
  
  I2c.begin();
  I2c.read(0x10, POSITION_A_REG, 1, &(servo[idx].pos));
  I2c.end();
};

void show_servo(uint8_t idx) {
  mySUI.print("Servo no. ");
  mySUI.println(idx);
  mySUI.print("Address: ");
  mySUI.print(servo[idx].add);
  mySUI.print(" (0x"); 
  mySUI.print(trin_address_lookup(servo[idx].add), HEX);
  mySUI.println(")");
  mySUI.print("Short: ");
  mySUI.println(servo[idx].sh);
  mySUI.print("Long: ");
  mySUI.println(servo[idx].lg);
  mySUI.print("Position: ");
  mySUI.println(servo[idx].pos);
};

void show_current_servo(){
  show_servo(current_servo);
};

void swap_lr() {
  uint16_t temp = servo[current_servo].sh;
  set_left(servo[current_servo].lg);
  set_right(temp);
  show_servo(current_servo);
};

void change_add() {
  mySUI.print("Change address to: ");
  mySUI.showEnterNumericDataPrompt();
  uint8_t temp = mySUI.parseInt();
  mySUI.print("Decoder: ");
  mySUI.print(temp/4);
  mySUI.print("Port: ");
  mySUI.println(temp % ((temp/4)));
  I2c.begin();
  I2c.write((uint8_t)0x10, (uint8_t)(ADD_A_REG + (current_servo * 0x10)), temp);
  I2c.end();
  servo[current_servo].add = temp;
};

void set_left(uint16_t value) {
  uint8_t idx = current_servo;
  mySUI.print("Sending: ");
  mySUI.print( (value >> 8) & 0xFF, HEX);
  I2c.begin();
  I2c.write(0x10, SHORT_A_REG_L + (0x10 * idx), value & 0xFF);
  I2c.end();
  mySUI.print( value & 0xFF, HEX);
  I2c.begin();
  I2c.write(0x10, SHORT_A_REG_H + (0x10 * idx), (value >> 8) & 0xFF);
  I2c.end();
  read_servo(current_servo);
};

void set_right(uint16_t value) {
  uint8_t idx = current_servo;
  I2c.begin();
  I2c.write(0x10, LONG_A_REG_L + (0x10 * idx), value & 0xFF);
  I2c.end();
  I2c.begin();
  I2c.write(0x10, LONG_A_REG_H + (0x10 * idx), (value >> 8) & 0xFF);
  I2c.end();
  read_servo(current_servo);
};

void set_left() {
  // SHORT
  mySUI.showEnterNumericDataPrompt();  
      //mySUI.showEnterDataPrompt();
  uint16_t value = mySUI.parseInt();
  set_left(value);
};

void set_right() {
  mySUI.showEnterNumericDataPrompt();  
      //mySUI.showEnterDataPrompt();
  uint16_t value = mySUI.parseInt();
  set_right(value);
};

void set_position(uint8_t idx, uint8_t pos) {
  I2c.begin();
  I2c.write((uint8_t)0x10, (uint8_t) (POSITION_A_REG + (0x10 * idx)), pos);
  I2c.end();
  mySUI.print((POSITION_A_REG + (0x10 * idx)), HEX);
  mySUI.println(pos, HEX);
}

void throw_servo() {
  if (servo[current_servo].pos == 0x00) {
    set_position(current_servo, 0x01);
  } else {
    set_position(current_servo, 0x00);
  }
  read_servo(current_servo);
};
    
void choose_servo() {
//  mySUI.print("Update servo [0/1] # ");
  mySUI.showEnterNumericDataPrompt();  
      //mySUI.showEnterDataPrompt();
  current_servo = mySUI.parseInt();
}

void add_servo_menu() {
  SUI::Menu * mainMenu = mySUI.topLevelMenu();
  SUI::Menu * enableMenu = mainMenu->subMenu(change_servo_key, change_servo_help);
  
  enableMenu->addCommand(choose_servo_key, choose_servo); 
  enableMenu->addCommand(swap_lr_key, swap_lr);
  enableMenu->addCommand(add_key, change_add);
  enableMenu->addCommand(set_left_key, set_left);
  enableMenu->addCommand(set_right_key, set_right);
  enableMenu->addCommand(show_current_servo_key, show_current_servo);
  enableMenu->addCommand(throw_key, throw_servo);
}

void show_info () {
  I2c.begin();
  I2c.read(0x10, TYPE_REG, 1, buffer);
  I2c.end();
  I2c.begin();
  I2c.read(0x10, HW_VERSION_REG, 1, buffer + 1);
  I2c.end();
  I2c.begin();
  dev.type=buffer[0];
  dev.hw_ver=buffer[1];
  I2c.read(0x10, FW_VERSION_REG, 1, buffer);
  I2c.end();
  dev.fw_ver=buffer[0];
  mySUI.print("Connected to ");
  if (dev.type == 0x01){
    mySUI.println("PSSD Servo/Switch decoder");
  };
  mySUI.print("Hardware version: ");
  mySUI.println(dev.hw_ver);
  mySUI.print("Firmware version: ");
  mySUI.print(dev.fw_ver);
  if (dev.type == 0x01){
    if (dev.fw_ver == 0x02) {
      mySUI.println(" (Servo firmware)");
      read_servo(0);
      read_servo(1);
      show_servo(0);
      show_servo(1);
      add_servo_menu();
    };
  };
  

  
}

void setup()
{
  I2c.begin(); // join i2c bus (address optional for master)
  I2c.setSpeed(0);
  I2c.pullup(0);
  I2c.timeOut(300);
  //Serial.begin(9600);
  //delay(500);
  // Remember: SerialUI acts just like Serial,
  // so we need to
  mySUI.begin(115200); // serial line open/setup
  mySUI.setTimeout(20000);      // timeout for reads (in ms), same as for Serial.
  mySUI.setMaxIdleMs(30000);    // timeout for user (in ms)
  // how we are marking the "end-of-line" (\n, by default):
  mySUI.setReadTerminator('\n');  

  SUI::Menu * mainMenu = mySUI.topLevelMenu();

  // Give the top-level menu a decent name
  mainMenu->setName(top_menu_title);

  /* we add commands using... addCommand()
     passing it the key, callback and 
     optionally help string
  */
  mainMenu->addCommand(info_key, show_info, info_help);

}


void loop()
{
  // check for a user
  if (mySUI.checkForUser(150))
  {
    // we have a user initiating contact, show the 
    // greeting message and prompt
    mySUI.enter();

    // keep handling the serial user's 
    // requests until they exit or timeout.
    while (mySUI.userPresent())
    {
      // actually respond to requests, using
      mySUI.handleRequests();
    }

  } 
}

