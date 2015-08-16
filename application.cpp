#include "application.h"
SYSTEM_MODE(SEMI_AUTOMATIC);
char publishString[63];
uint32_t lastTime;
bool receivedflag;
bool MotionSensIsLow;


//////
int MotionSens = D2;
//////

void doConnect() {
    WiFi.on();
    if (!WiFi.ready()) {
        WiFi.connect();
        while(WiFi.connecting())
        {
            Spark.process(); //SPARK_WLAN_Loop();
            delay(1000);
        }//while(WiFi.connecting())
    }//if (!WiFi.ready())
    
    if(!Spark.connected()){
        Spark.connect();
        while(!Spark.connected()){
            Spark.process(); //SPARK_WLAN_Loop();
            delay(1000);
        }//while(!Spark.connected())
    }//if(!Spark.connected())
}//doConnect

void myHandler(const char *event, const char *data){
    receivedflag = TRUE;
}//myhandler

void setup() {
    //////////
    Serial.begin(9600);
    Serial.println ("hello");
    pinMode(MotionSens, INPUT_PULLDOWN);
    ///////////
    doConnect();
    pinMode(D7,OUTPUT);
    Time.zone(+2);
    //Spark.subscribe("ReceivedMotionTime", myHandler, MY_DEVICES);
    //Spark.publish("pushoverMotionEveryDay", myHandler, MY_DEVICES);
    lastTime = millis();
    while(millis() - lastTime < 3000) {Spark.process();}
    receivedflag = FALSE;
    
    
    delay (10*1000);
    
    int addr = 1;
    uint8_t val = 0x00;
    EEPROM.write(addr, val);
    
    
}//setup

void loop(){
    
    Spark.disconnect();
    WiFi.disconnect();
    WiFi.off();
    
    if (EEPROM.read(1) == 0x00)
    {
        EEPROM.write(1, 0x01);
        System.sleep(D3, RISING,30);
    }
    else if (EEPROM.read(1) == 0x01)
    {
        delay (5000);
        Serial.println("EEPROM");
        EEPROM.write(1, 0x02);
        
        int timeForEveryDayHour = 9; //24-hour - 9pm->2100
        int timeForEveryDayMin = 0;
        int timeSetForEveryDay = (((timeForEveryDayHour-Time.hour())*360) + ((timeForEveryDayMin-Time.minute())*60) -Time.second());
        if (timeSetForEveryDay<0) timeSetForEveryDay = 86400+timeSetForEveryDay;
        else timeSetForEveryDay = timeSetForEveryDay;
        
        Serial.println (Time.hour());
        Serial.println (Time.minute());
        Serial.println (Time.second());
        
        Serial.println ("sec if");
        Serial.println (timeSetForEveryDay);
        delay(1000);
        System.sleep(D3, RISING,timeSetForEveryDay);
    }
    else System.sleep(D3, RISING,24*60*60); //Sleep indefinitely till D3 HIGH
    
    if (digitalRead(MotionSens) == LOW) MotionSensIsLow = true;
    else MotionSensIsLow = false;
    
    doConnect();
    
    if (Spark.connected()) {
        sprintf(publishString,"%04d/%02d/%02d %02d:%02d:%02d",Time.year(),Time.month(),
                Time.day(),Time.hour(),Time.minute(),Time.second());
        //Spark.publish("pushoverMotionEveryDay", publishString, 60, PRIVATE);
     
        if (MotionSensIsLow) {
             Spark.publish("pushoverMotionEveryDay", publishString, 60, PRIVATE);
            delay(2000);
        } else {Spark.publish("Door-Open", publishString, 60, PRIVATE);
        lastTime = millis();
        while( (!receivedflag)  && (millis() - lastTime < 10000) ) { // 10s allows catching receipt msg
            Spark.process();
            if (receivedflag == TRUE) { //confirm receipt msg
                digitalWrite(D7, HIGH);
                lastTime = millis();
                while(millis() - lastTime < 200) {Spark.process();}
                digitalWrite(D7, LOW);
            }//if (receivedflag == TRUE)
        }//while((!receivedflag) && (millis() - lastTime < SOFTDELAY3s))
      }
    }//if (Spark.connected())
    receivedflag = FALSE;
    
    //////NEW CODE
    Serial.println ("start while");
    while (digitalRead(MotionSens) == HIGH){
        delay(3000);
            Spark.publish("Door-Open", publishString, 60, PRIVATE);
        }
}//loop