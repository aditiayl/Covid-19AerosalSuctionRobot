/*
  DEFAULT : servo digerakkan dr data serial-> mode target dan mode increment
  limit switch untuk menghentikan gerakan servo (stopper)
  init dengan memutar servo ke limit dan servo atas ke position
  button untuk menggerak2an servo
*/

#include <DynamixelShield.h>

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)
#include <SoftwareSerial.h>
SoftwareSerial soft_serial(7, 8); // DYNAMIXELShield UART RX/TX
#define DEBUG_SERIAL soft_serial
#elif defined(ARDUINO_SAM_DUE) || defined(ARDUINO_SAM_ZERO)
#define DEBUG_SERIAL SerialUSB
#else
#define DEBUG_SERIAL Serial
#endif

//const uint8_t DXL_ID = 11;      // SERVO BAWAH - MODE CONTINOUS ROTATION
const uint8_t DXL_ID = 10;      // SERVO BAWAH - MODE CONTINOUS ROTATION
const uint8_t DXL_ID_2 = 1;     // SERVO ATAS - MODE POTITIONAL ROTATION

//const float DXL_PROTOCOL_VERSION = 2.0;
const float DXL_PROTOCOL_VERSION = 1.0;

DynamixelShield dxl;

//This namespace is required to use Control table item names
using namespace ControlTableItem;

/*
   Define pin I/O
*/
#define LIMIT_Kiri   A2
#define LIMIT_Kanan  A3
#define BUTTON_1     A1
#define BUTTON_2     A0
#define BUZZER       13

/*
   Declare Variabel n Global Function
*/
#define CCW 0
#define CW  1
#define INPUT_SIZE 30

#define SERVO_1_MAKS    100 //default -100 sampai 100
#define SERVO_1_MIN     -100
#define SERVO_2_MAKS    190
#define SERVO_2_MIN     90

#define Mesin_P 3 //Mesin Positif
#define Mesin_N 6 //Mesin Negatif
#define RX_PIN 4 //RX
#define TX_PIN 5 //TX

int16_t servo_1 = 0;
int16_t servo_2 = SERVO_2_MIN;

char inputChar[INPUT_SIZE + 1];

void setServoBawah(int8_t n); //normal 8bit
void setServoAtas(int16_t degree);
String getValue(String data, char separator, int index);



void setup() {
  // put your setup code here, to run once:

  DEBUG_SERIAL.begin(9600);

  pinMode(LIMIT_Kiri, INPUT_PULLUP);
  pinMode(LIMIT_Kanan, INPUT_PULLUP);
  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  //PIN RX dan TX
  pinMode(RX_PIN, OUTPUT);
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(RX_PIN, LOW);
  digitalWrite(TX_PIN, LOW);

   //RELAY
  pinMode(Mesin_P, OUTPUT);
  pinMode(Mesin_N, OUTPUT);
  digitalWrite(Mesin_P, HIGH);
  digitalWrite(Mesin_N, HIGH);

  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(57600);
  //dxl.begin(1000000);
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  // Get DYNAMIXEL information
  dxl.ping(DXL_ID);

  // Turn on the LED on DYNAMIXEL
  dxl.ledOn(DXL_ID);
  dxl.ledOn(DXL_ID_2);
  delay(200);
  // Turn off the LED on DYNAMIXEL
  dxl.ledOff(DXL_ID);
  dxl.ledOff(DXL_ID_2);
  delay(100);

  dxl.torqueOff(DXL_ID);
  dxl.torqueOff(DXL_ID_2);
  dxl.setOperatingMode(DXL_ID, OP_VELOCITY);
  dxl.setOperatingMode(DXL_ID_2, OP_POSITION);
  dxl.torqueOn(DXL_ID);
  dxl.torqueOn(DXL_ID_2);

  setServoAtas(SERVO_2_MIN);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(BUZZER, HIGH);
  //dxl.setGoalVelocity(DXL_ID_2, 5.0, UNIT_PERCENT);

  //dxl.setGoalVelocity(DXL_ID, 0, UNIT_PERCENT);

  for (int i = 0; i < 100; i++) while (digitalRead(BUTTON_1) && digitalRead(BUTTON_2) && digitalRead(LIMIT_Kanan) && digitalRead(LIMIT_Kiri) ) {
      // standby button belum ditekan
      serialIncoming();
      setServoAtas(servo_2);
      setServoBawah(servo_1);
    }

  if (digitalRead(BUTTON_1) == 0) {                         // BUTTON UNTUK GERAKKAN SERVO ATAS
    DEBUG_SERIAL.println("button_1 ditekan");
    digitalWrite(BUZZER, LOW);

    servo_2++;
    if (servo_2 > SERVO_2_MAKS) servo_2 = SERVO_2_MAKS;
    else if (servo_2 < SERVO_2_MIN) servo_2 = SERVO_2_MIN;
    setServoAtas(servo_2);

    delay(10);
  }
  else if (digitalRead(BUTTON_2) == 0) {                    // BUTTON UNTUK GERAKKAN SERVO ATAS
    DEBUG_SERIAL.println("button_2 ditekan");
    digitalWrite(BUZZER, LOW);

    servo_2--;
    if (servo_2 > SERVO_2_MAKS) servo_2 = SERVO_2_MAKS;
    else if (servo_2 < SERVO_2_MIN) servo_2 = SERVO_2_MIN;
    setServoAtas(servo_2);

    delay(10);
  }

  else if (digitalRead(LIMIT_Kiri) == 0) {
    DEBUG_SERIAL.println("limit_kiri ditekan");
    digitalWrite(BUZZER, LOW);
    delay(100);
    serialIncoming();

    if (servo_1 > 0) setServoBawah(servo_1);
  }
  else if (digitalRead(LIMIT_Kanan) == 0) {
    //putar(LEFT, 1, 1023);
    DEBUG_SERIAL.println("limit_kanan ditekan");
    digitalWrite(BUZZER, LOW);
    delay(100);
    serialIncoming();

    if (servo_1 < 0) setServoBawah(servo_1);
  }

}

void serialIncoming() {
  /*
     Protocol data string,
     Nilai langsung "xx,yy#"
      xx= nilai putaran servo bawah
      yy= nilai sudut servo atas

     nilai increment "'+'/'-'/'<'/'>'$"
     '+' tambah servo bawah
     '-' kurangi servo bawah
     '<' Kurangi servo atas
     '>' tambah servo atas
  */
  static int key = 0;

  while (DEBUG_SERIAL.available()) {
    inputChar[key] = DEBUG_SERIAL.read();
    key++;

    // look for the newline. That's the end of data
    if (inputChar[key - 1] == '#') {
      //DEBUG_SERIAL.println("incoming complete");
      //DEBUG_SERIAL.println(inputChar);

      String data1 = getValue(inputChar, ',', 0);
      String data2 = getValue(inputChar, ',', 1);

      servo_1 = data1.toInt();
      if (servo_1 > SERVO_1_MAKS) servo_1 = SERVO_1_MAKS;
      else if (servo_1 < SERVO_1_MIN) servo_1 = SERVO_1_MIN;

      servo_2 = data2.toInt();
      if (servo_2 == 0)servo_2 = 0;
      else {
        if (servo_2 > SERVO_2_MAKS) servo_2 = SERVO_2_MAKS;
        else if (servo_2 < SERVO_2_MIN) servo_2 = SERVO_2_MIN;
      }
   
      DEBUG_SERIAL.println(servo_1);
      DEBUG_SERIAL.println(servo_2);

     

      memset(inputChar, 0, sizeof inputChar);
      key = 0;
    }

    if (inputChar[key - 1] == '%') {
      //DEBUG_SERIAL.println("increment complete");
      //DEBUG_SERIAL.println(inputChar);

      for (int ii = 0; ii < key - 1; ii++) {
        if (inputChar[ii] == '+' && servo_1 < SERVO_1_MAKS && servo_1 >= SERVO_1_MIN) {
          servo_1++;
        }
        else if (inputChar[ii] == '-' && servo_1 <= SERVO_1_MAKS && servo_1 > SERVO_1_MIN) {
          servo_1--;
        }
        else if (inputChar[ii] == '>' && servo_2 < SERVO_2_MAKS && servo_2 >= SERVO_2_MIN) {
          servo_2++;
          delay(1);
        }
        else if (inputChar[ii] == '<' && servo_2 <= SERVO_2_MAKS && servo_2 > SERVO_2_MIN) {
          servo_2--;
          delay(1);
        }
        else if (inputChar[ii] == 'x'){
                digitalWrite(RX_PIN, HIGH);
                digitalWrite(TX_PIN, HIGH);
                delay(20);
                DEBUG_SERIAL.println("Komunikasi Serial ON");
        }

        else if (inputChar[ii] == 'y'){
                digitalWrite(RX_PIN, LOW);
                digitalWrite(TX_PIN, LOW);
                delay(20);
                DEBUG_SERIAL.println("Komunikasi Serial OFF");
        }
        
        else if (inputChar[ii] == '1'){
                digitalWrite(Mesin_P, LOW);
                digitalWrite(Mesin_N, LOW);
                delay(20);
                DEBUG_SERIAL.println("Mesin ON");
        }
        else if (inputChar[ii]== '0'){
                digitalWrite(Mesin_P, HIGH);
                digitalWrite(Mesin_N, HIGH);
                delay(20);
                DEBUG_SERIAL.println("Mesin OFF");
        }
      }

      DEBUG_SERIAL.println(servo_1);
      DEBUG_SERIAL.println(servo_2);

      memset(inputChar, 0, sizeof inputChar);
      key = 0;
    }  
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setServoAtas(int16_t degree) {
  if (servo_2 != 0) {
    dxl.setGoalVelocity(DXL_ID_2, 2.0, UNIT_PERCENT);
    dxl.setGoalPosition(DXL_ID_2, degree, UNIT_DEGREE);
  }
  /*delay(1000);
    // Print present position in degree value
    DEBUG_SERIAL.print("Present Position(degree) : ");
    DEBUG_SERIAL.println(dxl.getPresentPosition(DXL_ID, UNIT_DEGREE));
    delay(1000);*/
}

void setServoBawah(int8_t n) {
  /*
    // kekiri (CCW) derajat naik

      1 2 3 4 5
      delta
      1 1 1 1 1

      sum positif

    // kenanan (CW) derajat turun

      5 4 3 2 1
      delta
      -1 -1 -1 -1 -1

      sum negatif
  */

  int16_t sum = 0;
  int deltaPos = 0;
  int Pos = 0;
  int Pos_1 = 0;

  Pos = dxl.getPresentPosition(DXL_ID);
  Pos_1 = Pos;

  if (n > 0) {                                          // putaran CCW
    dxl.setGoalVelocity(DXL_ID, -0, UNIT_PERCENT);
    long  jum = n * 4095;
    while (sum < jum  ) {                        // tambahkan LIMIT sebagai pembatas gerak
      if (digitalRead(LIMIT_Kanan) == 0) {
        break;
      }
      Pos = dxl.getPresentPosition(DXL_ID);
      deltaPos = Pos - Pos_1;

      if (deltaPos < -2000) deltaPos += 4095;
      else if (deltaPos > 2000) deltaPos -= 4095;

      sum += deltaPos;
      Pos_1 = Pos;
      //delay(1);
    }
    dxl.setGoalVelocity(DXL_ID, 0);  // -1 berhenti
    servo_1 = 0;
  }

  else if (n < 0) {                                   // putaran CW
    dxl.setGoalVelocity(DXL_ID, 100, UNIT_PERCENT);
    long  jum1 = n * 4095;
    while (sum > jum1 ) {                       // tambahkan LIMIT sebagai pembatas gerak
      if (digitalRead(LIMIT_Kiri) == 0) {
        break;
      }

      Pos = dxl.getPresentPosition(DXL_ID);
      deltaPos = Pos - Pos_1;

      if (deltaPos < -2000) deltaPos += 4095;
      else if (deltaPos > 2000) deltaPos -= 4095;

      sum += deltaPos;
      Pos_1 = Pos;
      //delay(1);
    }
    dxl.setGoalVelocity(DXL_ID, 0);
    servo_1 = 0;
  }

  else {
    dxl.setGoalVelocity(DXL_ID, 0);
    servo_1 = 0;
  }
}
