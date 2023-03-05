
//#include <stdio.h>      /* printf */
//#include <math.h>       /* floor */
//#include <Wire.h>

//#include <USIWire.h>
#include <SoftWire.h>
#include <AsyncDelay.h>

#if defined(PIN_WIRE_SDA) && defined(PIN_WIRE_SCL)
int sdaPin = PIN_WIRE_SDA;
int sclPin = PIN_WIRE_SCL;

#else
int sdaPin = SDA;
int sclPin = SCL;
#endif

// I2C address of DS1307
const uint8_t I2C_ADDRESS = 0x38;

SoftWire sw(sdaPin, sclPin);
// These buffers must be at least as large as the largest read or write you perform.
char swTxBuffer[16];
char swRxBuffer[16];

AsyncDelay readInterval;


/**************************************************************************
   Tests the getTemperature and getHumidity functions of the aht20 library
 **************************************************************************/

#include <RCSwitch.h>


#define RCSwitchDisableReceiving
RCSwitch mySwitch = RCSwitch();


#include <Thinary_AHT_Sensor.h>
//#include <Wire.h>


Sensor_CMD eSensorCalibrateCmd[3] = {0xE1, 0x08, 0x00};
Sensor_CMD eSensorNormalCmd[3]    = {0xA8, 0x00, 0x00};
Sensor_CMD eSensorMeasureCmd[3]   = {0xAC, 0x33, 0x00};
Sensor_CMD eSensorResetCmd        = 0xBA;
bool    GetTempCmd             = false;

/******************************************************************************
 * Global Functions
 ******************************************************************************/
AHT_Sensor_Class::AHT_Sensor_Class() {
}

bool AHT_Sensor_Class::begin(unsigned char _AHT_Sensor_address)
{
    AHT_Sensor_address = _AHT_Sensor_address;
    Serial.begin(9600);
    Serial.println("\x54\x68\x69\x6E\x61\x72\x79\x20\x45\x6C\x65\x74\x72\x6F\x6E\x69\x63\x20\x41\x48\x54\x31\x30\x20\x4D\x6F\x64\x75\x6C\x65\x2E");
    sw.begin();
    sw.beginTransmission(AHT_Sensor_address);
    sw.write(eSensorCalibrateCmd, 3);

    sw.endTransmission();
    
    delay(500);
    if((readStatus()&0x68) == 0x08)
        return true;
    else
    {
        return false;
    }
    
}

/**********************************************************
 * GetTemperature
 *  Gets the current temperature from the sensor.
 *
 * @return float - The temperature in Deg C
 **********************************************************/
float AHT_Sensor_Class::GetTemperature(void)
{
    float value = readSensor(GetTempCmd);
    return ((200 * value) / 1048576) - 50;
}


/******************************************************************************
 * Private Functions
 ******************************************************************************/

unsigned long AHT_Sensor_Class::readSensor(bool GetDataCmd)
{
    unsigned long result, temp[6];

    sw.beginTransmission(AHT_Sensor_address);
    sw.write(eSensorMeasureCmd, 3);
    sw.endTransmission();
    delay(100);

    sw.requestFrom(AHT_Sensor_address, 6);

    for(unsigned char i = 0; sw.available() > 0; i++)
    {
        temp[i] = sw.read();
    }   

    if(GetDataCmd)
    {
        result = ((temp[1] << 16) | (temp[2] << 8) | temp[3]) >> 4;
    }
    else
    {
        result = ((temp[3] & 0x0F) << 16) | (temp[4] << 8) | temp[5];
    }

    return result;
}

unsigned char AHT_Sensor_Class::readStatus(void)
{
    unsigned char result = 0;

    sw.requestFrom(AHT_Sensor_address, 1);
    result = sw.read();
    return result;
}

void AHT_Sensor_Class::Reset(void)
{
    sw.beginTransmission(AHT_Sensor_address);
    sw.write(eSensorResetCmd);
    sw.endTransmission();
    delay(20);
}

AHT_Sensor_Class AHT10;

void setup() {
  delay(1000);
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  mySwitch.enableTransmit(1);
  //Wire.begin();
  if(AHT10.begin(I2C_ADDRESS))
    Serial.println("Init AHT10 Sucess.");
  else
    Serial.println("Init AHT10 Failure.");
  sw.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
  sw.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
  sw.setDelay_us(5);
  sw.setTimeout(1000);
  sw.begin();
  readInterval.start(2000, AsyncDelay::MILLIS);
}

int fractional(float flo) {
  return int(((flo)-floor(flo))*100);
}

String intToBinary(int n) {
  String temp = "00000000";
  int arrayPosition=0;         // Keep track of the position of each Bit.  
  for(int a=128; a>=1; a=a/2){          // This loop will start at 128, then 64, then 32, etc.
    if((n-a)>=0){             // This checks if the Int is big enough for the Bit to be a '1'
      temp[arrayPosition] = '1';   // Assigns a '1' into that Array position.
      n-=a;}                  // Subracts from the Int.
    else{
      temp[arrayPosition]='0';}    // The Int was not big enough, therefore the Bit is a '0'
      arrayPosition++;}                 // Move one Character to the right in the Array.
  return String(temp);
}


String floatToBinary16(float flo) {
  String sign = "0";
  if (flo < 0) {
    sign = "1";
  }
  int tempInteger= int(flo);    // Any number from 0-255.
  String MESSAGE = sign + String(intToBinary(tempInteger)) + String(intToBinary(fractional(flo)));
  Serial.println(MESSAGE);
  return MESSAGE;
}


void loop() {
  
  String BANGARANG = String(floatToBinary16(AHT10.GetTemperature()));
  if (!BANGARANG == "-50.00") {
    Serial.println(BANGARANG);
    mySwitch.send(BANGARANG.c_str()); 
    digitalWrite(3, HIGH);
    delay(250);  
    digitalWrite(3, LOW);
    delay(250);
    delay(1500);  

  }
  delay(2000);
}
