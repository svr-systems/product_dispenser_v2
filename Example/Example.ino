#include <SD.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

//Arrays
const byte leds[6] = {44, 42, 40, 38, 36, 34};
const byte cstmr_btns[6] = {45, 43, 41, 39, 37, 35};
const String products[6] = {"PRODUCTO 1", "PRODUCTO 2", "PRODUCTO 3", "PRODUCTO 4", "PRODUCTO 5", "PRODUCTO 6"};
float pump_amounts[6];
unsigned long pump_times[6];
float liters_content[6];
//Pins
const byte prog_btn = 30;
const byte buzz_pin = 31;
const byte coin_btn = 49;
//Bools
bool cstmr_mode = true;
bool prog_mode = false;
//Steps
byte cstmr_step = 1;
byte prog_step = 1;
byte product_slct;
byte prog_slct = 0;
byte prog_change = 1;
//Counts
int credits;
float liters;
//SD
#define SSpin 53
File archive;
//Reloj
RTC_DS3231 rtc;
//LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long elapsedTime = 0;
bool btn_pressed = false;

void buzzHandle(int millis, byte pulses) {
  for (byte i = 0; i < pulses; i++) {
    digitalWrite(buzz_pin, HIGH);
    delay(millis / 2);
    digitalWrite(buzz_pin, LOW);
    delay(millis / 2);
  }
}

void setLeds(boolean value) {
  for (byte i = 0; i < sizeof(leds); i++) {
    digitalWrite(leds[i], value);
  }
}

String lcdCenterStr(String str) {
  byte adjust_center = (20 - str.length()) / 2;
  String str_temp = "";

  for (byte i = 0; i < adjust_center; i++)
    str_temp += " ";

  return str_temp + str;
}

int readIntFromFile() {
  char buffer[10];
  archive.readBytesUntil('\n', buffer, sizeof(buffer));
  return atoi(buffer);
}

float readFloatFromFile() {
  char buffer[10];
  archive.readBytesUntil('\n', buffer, sizeof(buffer));
  return atof(buffer);
}


void setup()
{
    Serial.begin(9600);

    for ( byte i = 0; i < sizeof(leds); i ++) {
        pinMode(leds[i], OUTPUT);
    }
    for (byte i = 0; i < sizeof(cstmr_btns); i ++) {
        pinMode(cstmr_btns[i], INPUT_PULLUP);
    }
    pinMode(buzz_pin, OUTPUT);
    pinMode(coin_btn, INPUT_PULLUP);
    pinMode(prog_btn, INPUT_PULLUP);

    lcd.init();
    lcd.backlight();
    lcd.clear();

    Serial.println("Inicializando tarjeta");

    if (!SD.begin(SSpin)) {
        Serial.println("Fallo en inicialización");
        return;
    }

    archive = SD.open("config.txt");

    if (archive) {
        pump_amounts[0] = readFloatFromFile();
        pump_times[0] = readFloatFromFile();
        liters_content[0] = readFloatFromFile();
        pump_amounts[1] = readFloatFromFile();
        pump_times[1] = readFloatFromFile();
        liters_content[1] = readFloatFromFile();
        pump_amounts[2] = readFloatFromFile();
        pump_times[2] = readFloatFromFile();
        liters_content[2] = readFloatFromFile();
        pump_amounts[3] = readFloatFromFile();
        pump_times[3] = readFloatFromFile();
        liters_content[3] = readFloatFromFile();
        pump_amounts[4] = readFloatFromFile();
        pump_times[4] = readFloatFromFile();
        liters_content[4] = readFloatFromFile();
        pump_amounts[5] = readFloatFromFile();
        pump_times[5] = readFloatFromFile();
        liters_content[5] = readFloatFromFile();

        archive.close();
    }

    Serial.println("Inicialización completa");

    rtc.begin();
    //rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop()
{

    DateTime Date = rtc.now();

    if (digitalRead(cstmr_btns[5]) == 0 && prog_mode == true) {

        SD.remove("config.txt");
        archive = SD.open("config.txt", FILE_WRITE);

        if (archive) {

            for (byte i = 0; i < sizeof(pump_amounts); i++) {
                archive.println(pump_amounts[i]);
                archive.println(pump_times[i]);
                archive.println(liters_content[i]);
            } 

            Serial.println("Hecho");
            archive.close();

        }
        else {
            Serial.println("Error");
        }

        lcd.clear();
        buzzHandle(100, 1);
        lcd.setCursor(0,1);
        lcd.print ("  MODO PROGRAMADOR");
        lcd.setCursor(0,2);
        lcd.print(lcdCenterStr("DESACTIVADO"));
        delay(1500);
        prog_mode = false;
        cstmr_mode = true;
        cstmr_step = 1;
        prog_step = 0;
        lcd.clear();
        
    }

    else if (digitalRead(prog_btn) == 0 && prog_mode == false) {
        prog_mode = true;
        cstmr_mode = false;
        prog_step = 1;
        cstmr_step = 1; 
        lcd.clear();
        buzzHandle(100, 1);
    }

	if (cstmr_mode == true) 
    {
        setLeds(1);

        if(credits == 0) {
        lcd.display();
        lcd.setCursor(0,1);
        lcd.print(lcdCenterStr("BIENVENIDO"));
        lcd.setCursor(0,3);
        lcd.print("  INGRESE CREDITOS");
        }

        switch (cstmr_step) {
            case 1: //Credits and product select
                if (credits > 0) {
                    if (digitalRead(cstmr_btns[0]) == 0 && credits >= pump_amounts[0] && liters_content[0] >= 1) {
                        while (digitalRead(cstmr_btns[0]) == 0) {}
                        
                        product_slct = 0;
                        cstmr_step ++;
                        buzzHandle(100, 1);
                    }
                    else if (digitalRead(cstmr_btns[1]) == 0 && credits >= pump_amounts[1] && liters_content[1] >= 1) {
                        while (digitalRead(cstmr_btns[1]) == 0) {}

                        product_slct = 1;
                        cstmr_step ++;
                        buzzHandle(100, 1);
                    }
                    else if (digitalRead(cstmr_btns[2]) == 0 && credits >= pump_amounts[2] && liters_content[2] >= 1) {
                        while (digitalRead(cstmr_btns[2]) == 0) {}

                        product_slct = 2;
                        cstmr_step ++;
                        buzzHandle(100, 1);
                    }
                    else if (digitalRead(cstmr_btns[3]) == 0 && credits >= pump_amounts[3] && liters_content[3] >= 1) {
                        while (digitalRead(cstmr_btns[3]) == 0) {}
                        product_slct = 3;
                        cstmr_step ++;
                        buzzHandle(100, 1);
                    }
                    else if (digitalRead(cstmr_btns[4]) == 0 && credits >= pump_amounts[4] && liters_content[4] >= 1) {
                        while (digitalRead(cstmr_btns[4]) == 0) {}
                        product_slct = 4;
                        cstmr_step ++;
                        buzzHandle(100, 1);
                    }
                    else if (digitalRead(cstmr_btns[5]) == 0 && credits >= pump_amounts[5] && liters_content[5] >= 1) {
                        while (digitalRead(cstmr_btns[5]) == 0) {}
                        product_slct = 5;
                        cstmr_step ++;
                        buzzHandle(100, 1);
                    }
                }

                if (digitalRead(coin_btn) == 0) {
                while (digitalRead(coin_btn) == 0) {}
                credits += 6;
                buzzHandle(100, 1);

                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("      CREDITOS");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr("$" + (String)credits));
                }
            break;

            case 2: //
                liters = credits / (float)pump_amounts[product_slct];

                lcd.clear();
                
                lcd.setCursor(0, 1);
                lcd.print("   COLOQUE ENVASE");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr((String)liters + " L"));
                buzzHandle(500, 10);
                cstmr_step ++;
            break;

            case 3: //Filling
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print(lcdCenterStr(products[product_slct]));
                lcd.setCursor(0, 1);
                lcd.print(lcdCenterStr("SURTIENDO " + (String)liters + " L"));
                lcd.setCursor(0, 3);
                lcd.print("ESPERE UN MOMENTO...");
                delay(pump_times[product_slct] * liters);
                cstmr_step ++;
            break;

            case 4: //Process completed
                setLeds(0);
                liters_content[product_slct] -= liters;
                lcd.clear();
                buzzHandle(100, 3);
                lcd.setCursor(0, 1);
                lcd.print("GRACIAS POR COMPRAR!");
                lcd.setCursor(0, 3);
                lcd.print("   VUELVA PRONTO");
                delay(2000);

                cstmr_step = 1;
                credits = 0;
                liters = 0;
                product_slct = 0;
                setLeds(1);
                lcd.clear();

            break;
        }

    }
    else 
    {
        if (prog_mode == true) 
        {
            setLeds(0);

            switch (prog_step) {
                case 1:
                    lcd.setCursor(0,1);
                    lcd.print ("  MODO PROGRAMADOR");
                    lcd.setCursor(0,2);
                    lcd.print(lcdCenterStr("ACTIVADO"));
                    delay(1500);
                    credits = 0;
                    prog_change = 1;
                    lcd.clear();
                    prog_step ++;
                break;

                case 2:
                    if (digitalRead(prog_btn) == 0) {
                        while (digitalRead(prog_btn) == 0) {}
                        lcd.clear();
                        prog_change ++;
                        buzzHandle(100, 1);
                    }
                    else if (prog_change == 4) {
                        prog_change = 0;
                    } 
                    
                    if (prog_change == 1) {
                        lcd.setCursor(0,1);
                        lcd.print(lcdCenterStr("-CAMBIAR PRECIO-"));
                    }
                    else if (prog_change == 2) {
                        lcd.setCursor(0,1);
                        lcd.print(lcdCenterStr("-CAMBIAR TIEMPO-"));
                    }
                    else if (prog_change == 3) {
                        lcd.setCursor(0,1);
                        lcd.print(lcdCenterStr("-RELLENAR-"));
                    }
                    else if (prog_change == 4) {
                        lcd.setCursor(0,1);
                        lcd.print(lcdCenterStr("-HORARIO-"));
                    }

                    if (digitalRead(cstmr_btns[0]) == 0) {
                        while (digitalRead(cstmr_btns[0]) == 0) {}
                        buzzHandle(100, 1);
                        lcd.clear();
                        if (prog_change == 0) {
                            prog_step = 4;
                            delay(350);
                        }
                        else {
                            prog_step ++;
                            delay(350);
                        }
                    }
                break;

                case 3:
                    if (prog_slct == prog_slct) {
                        lcd.setCursor(0, 1);
                        lcd.print(lcdCenterStr(products[prog_slct]));
                    } 
                    
                    if (digitalRead(prog_btn) == 0) {
                        while (digitalRead(prog_btn) == 0) {}
                        lcd.clear();
                        prog_slct ++;
                        buzzHandle(100, 1);
                    }
                    else if (prog_slct == 6) {
                        prog_slct = 0;
                    }
                    else if (digitalRead(cstmr_btns[0]) == 0) {
                        while(digitalRead(cstmr_btns[0]) == 0) {}
                        buzzHandle(100, 1);
                        lcd.clear();
                        prog_step ++;
                    }
                    else if (digitalRead(cstmr_btns[1]) == 0) {
                        while(digitalRead(cstmr_btns[1]) == 0) {}
                        buzzHandle(100, 1);
                        prog_slct = 0;
                        prog_change = 1;
                        prog_step --;
                    }
                break;

                case 4:
                    if (prog_change == 1) {

                        lcd.setCursor(0,1);
                        lcd.print(lcdCenterStr("PRECIO / LITRO"));
                        lcd.setCursor(0,2);
                        lcd.print(lcdCenterStr("$" + (String)pump_amounts[prog_slct]));

                        if (digitalRead(cstmr_btns[0]) == 0) {
                            while (digitalRead(cstmr_btns[0]) == 0) {}
                            pump_amounts[prog_slct] += .50;
                            buzzHandle(100, 1);
                        }
                        else if (digitalRead(cstmr_btns[1]) == 0) {
                            while (digitalRead(cstmr_btns[1]) == 0) {}
                            pump_amounts[prog_slct] -= .50;
                            buzzHandle(100, 1);
                        }
                        else if (digitalRead(prog_btn) == 0) {
                            while (digitalRead(prog_btn) == 0) {}
                            buzzHandle(100, 1);
                            lcd.clear();
                            prog_slct = 0;
                            prog_step = 2;
                        }
                    }
                    else if (prog_change == 2) {
                        lcd.setCursor(0,1);
                        lcd.print(lcdCenterStr("TIEMPO DE BOMBA"));
                        lcd.setCursor(0,2);
                        lcd.print(lcdCenterStr((String)pump_times[prog_slct] + " ms"));
                        
                        byte btn_state = digitalRead(cstmr_btns[0]);

                        if (btn_state == 0 && !btn_pressed) {
                            startTime = millis();
                            btn_pressed = true;
                        }
                        else if (btn_state == 1 && btn_pressed) {
                            endTime = millis();
                            btn_pressed = false;
                            elapsedTime = endTime - startTime;
                            pump_times[prog_slct] = elapsedTime; 
                        }
                        else if (digitalRead(prog_btn) == 0) {
                            while (digitalRead(prog_btn) == 0) {}
                            buzzHandle(100, 1);
                            lcd.clear();
                            prog_change = 1;
                            prog_slct = 0;
                            prog_step = 2;
                        }
                    }
                    else if (prog_change == 3) {

                        lcd.setCursor(0,1);
                        lcd.print(lcdCenterStr("CANTIDAD PRODUCTO"));
                        lcd.setCursor(0,2);
                        lcd.print(lcdCenterStr((String)liters_content[prog_slct] + " L"));

                        if (digitalRead(cstmr_btns[0]) == 0) {
                            while (digitalRead(cstmr_btns[0]) == 0){}
                            buzzHandle(100, 1);
                            liters_content[prog_slct] += 1;
                        }
                        else if (digitalRead(cstmr_btns[1]) == 0) {
                            while (digitalRead(cstmr_btns[1]) == 0) {}
                            buzzHandle(100, 1);
                            liters_content[prog_slct] -= 1;
                        }
                        else if (digitalRead(prog_btn) == 0) {
                            while (digitalRead(prog_btn) == 0) {}
                            buzzHandle(100, 1);
                            lcd.clear();
                            prog_step = 2;
                            prog_change = 1;
                            prog_slct = 0;
                        }
                    }
                    else if (prog_change == 0) {
                        lcd.setCursor(0,0);
                        lcd.print(lcdCenterStr("HORARIO"));
                        lcd.setCursor(0,2);
                        lcd.print(lcdCenterStr((String)Date.hour() + ":" + (String)Date.minute()));
                        lcd.setCursor(0,3);
                        lcd.print(lcdCenterStr((String)Date.day() + "/" + (String)Date.month() + "/" + (String)Date.year()));

                        if (digitalRead(prog_btn) == 0) {
                            while (digitalRead(prog_btn) == 0) {}
                            buzzHandle(100, 1);
                            lcd.clear();
                            prog_step = 2;
                            prog_change = 1;
                            prog_slct = 0;
                        }
                    }
                break;
            }
        }
    }
}
