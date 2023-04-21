/*
  Example for receiving
  
  https://github.com/sui77/rc-switch/
  https://github.com/LSatan/SmartRC-CC1101-Driver-Lib
  If you want to visualize a telegram copy the raw data and 
  paste it into http://test.sui.li/oszi/
  ----------------------------------------------------------
  Mod by Little Satan. Have Fun!
  ----------------------------------------------------------
*/
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();
bool pult_open_isRun  = false;
bool pult_close_isRun = false;
const int in_estop_close   = 16;
const int in_estop_open    = 17;
const int out_pwm          = 13;
const int out_relay_close  = 27;
const int out_relay_open   = 26;
const int maxSpeed         = 255;
const unsigned long accel  = 5UL;
int currentSpeed          = 0;
bool isCodeOpen(unsigned long code){
    if(code == 6969512){
        return true;
    }
    return false;
}
bool isCodeClose(unsigned long code){
    if(code == 6969508){
        return true;
    }
    return false;
}
void setup() {
    Serial.begin(9600);
    { // config GPIO
        pinMode(in_estop_close, INPUT_PULLUP);
        pinMode(in_estop_open,  INPUT_PULLUP);
        pinMode(out_relay_close, OUTPUT);
        pinMode(out_relay_open,  OUTPUT);
    }
    { // config PWM
        /*
            ledcSetyp
            ledcAttachPin
            ledcWrite
        */
        ledcSetup(0, 20000, 8);
        ledcAttachPin(out_pwm, 0);
        ledcWrite(0, 0);
    }
    {// config cc1101
        if (ELECHOUSE_cc1101.getCC1101()){       // Check the CC1101 Spi connection.
            Serial.println("Connection OK");
        }
        else{
            Serial.println("Connection Error");
        }
        //CC1101 Settings:                (Settings with "//" are optional!)
        ELECHOUSE_cc1101.Init();            // must be set to initialize the cc1101!
        //ELECHOUSE_cc1101.setRxBW(812.50);  // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
        //ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!
        ELECHOUSE_cc1101.setMHZ(433.92); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
        mySwitch.enableReceive(2);  // Receiver on interrupt 0 => that is pin #2
        ELECHOUSE_cc1101.SetRx();  // set Receive on
    }
}

void loop() {
    { // debug print
        static unsigned long t = millis();
        if(millis() - t >= 100UL){
            t = millis();
            if(pult_close_isRun) Serial.println("pult_close_isRun");
            if(pult_open_isRun)  Serial.println("pult_open_isRun");
            //Serial.println("------------------------------------");
        }
    }
    {
        static unsigned long t = millis();
        if (mySwitch.available()) {
            unsigned long rCode = 0;
            rCode = mySwitch.getReceivedValue();
            if(isCodeClose(rCode) && digitalRead(in_estop_close) == HIGH){
                t = millis();
                pult_close_isRun = true;
            }
            else if(isCodeOpen(rCode) && digitalRead(in_estop_open) == HIGH){
                t = millis();
                pult_open_isRun = true;
            }
            mySwitch.resetAvailable();
        } 
        if(pult_close_isRun || pult_open_isRun){
            if(millis() - t >= 300UL){
                pult_close_isRun = false;
                pult_open_isRun  = false;
            }
        }        
    }

    {
        static int state = 0;
        static unsigned long tim = millis();
        switch (state){
            case 0:{
                if(pult_close_isRun || pult_open_isRun){
                    if(pult_close_isRun) {
                        digitalWrite(out_relay_close, HIGH);
                        digitalWrite(out_relay_open, LOW);
                    }
                    if(pult_open_isRun){
                        digitalWrite(out_relay_close, LOW);
                        digitalWrite(out_relay_open, HIGH);
                    }
                    tim = millis();
                    state = 1;
                }
                break;  
            }
            case 1:{
                if(millis() - tim >= 100UL){ // time out relay
                    state = 2;
                    currentSpeed = 150;
                    tim = millis();
                }
                break;
            }
            case 2:{
                if(millis() - tim >= accel){
                    tim = millis();
                    if((pult_close_isRun || pult_open_isRun) && (currentSpeed < maxSpeed)){
                        currentSpeed++;
                        ledcWrite(0, currentSpeed);
                    }
                    else{
                        state = 3;
                    }
                }
                break;
            }
            case 3:{
                if(millis() - tim >= accel){
                    tim = millis();
                    if((!(pult_close_isRun || pult_open_isRun)) && (currentSpeed > 0)){
                        currentSpeed--;
                        ledcWrite(0, currentSpeed);
                    }
                    if(currentSpeed < 150){
                        ledcWrite(0, 0);
                        state = 0;
                    }
                }
                break;
            }
        }
    }















}
