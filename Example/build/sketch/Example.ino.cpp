#include <Arduino.h>
#line 1 "C:\\Users\\SVR-DEV04\\Documents\\Proyectos\\Dispensadora_E\\Example\\Example.ino"
#include <Sodaq_DS3231.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//Arrays
byte leds[6] = {44, 42, 40, 38, 36, 34};
byte cstmr_btns[6] = {45, 43, 41, 39, 37, 35};
String products[6] = {"FABULOSO", "PINOL", "SALVO", "SUAVITEL", "CLORALEX", "VEL"};
byte pump_amounts[6] = {10, 5, 20, 9, 3, 15};
int pump_times[6] = {6000, 3000, 5000, 1200, 500, 700};
//Pins
byte prog_btn = 30;
byte buzz_pin = 31;
byte coin_btn = 52;
//Bools
bool cstmr_mode = true;
bool prog_mode = false;
//EPROMM
byte cstmr_step = 1;
byte prog_step = 1;
byte product_slct;
byte credits;
float liters;
byte prog_slct = 1;
//lcd
LiquidCrystal_I2C lcd(0x27, 20, 4);

#line 28 "C:\\Users\\SVR-DEV04\\Documents\\Proyectos\\Dispensadora_E\\Example\\Example.ino"
void buzzHandle(int millis, byte pulses);
#line 37 "C:\\Users\\SVR-DEV04\\Documents\\Proyectos\\Dispensadora_E\\Example\\Example.ino"
void setLeds(boolean value);
#line 43 "C:\\Users\\SVR-DEV04\\Documents\\Proyectos\\Dispensadora_E\\Example\\Example.ino"
String lcdCenterStr(String str);
#line 54 "C:\\Users\\SVR-DEV04\\Documents\\Proyectos\\Dispensadora_E\\Example\\Example.ino"
void setup();
#line 71 "C:\\Users\\SVR-DEV04\\Documents\\Proyectos\\Dispensadora_E\\Example\\Example.ino"
void loop();
#line 28 "C:\\Users\\SVR-DEV04\\Documents\\Proyectos\\Dispensadora_E\\Example\\Example.ino"
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


void setup()
{
    for ( byte i = 0; i < sizeof(leds); i ++) {
        pinMode(leds[i], OUTPUT);
    }
    for (byte i = 0; i < sizeof(cstmr_btns); i ++) {
        pinMode(cstmr_btns[i], INPUT_PULLUP);
    }

    lcd.init();
    lcd.backlight();
    lcd.clear();
    pinMode(buzz_pin, OUTPUT);
    pinMode(coin_btn, INPUT_PULLUP);
    pinMode(prog_btn, INPUT_PULLUP);
}

void loop()
{
    if (digitalRead(cstmr_btns[5]) == 0 && prog_mode == true) {
       
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
                
                if (digitalRead(cstmr_btns[0]) == 0) {
                    
                    product_slct = 0;
                    cstmr_step ++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[1]) == 0) {
                    product_slct = 1;
                    cstmr_step ++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[2]) == 0) {
                    product_slct = 2;
                    cstmr_step ++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[3]) == 0) {
                    product_slct = 3;
                    cstmr_step ++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[4]) == 0) {
                    product_slct = 4;
                    cstmr_step ++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[5]) == 0) {
                    product_slct = 5;
                    cstmr_step ++;
                    buzzHandle(100, 1);
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

            case 2: //Packaging
                liters = credits / (float)pump_amounts[product_slct];

                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("   COLOQUE ENVASE");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr((String)liters + " L"));
                cstmr_step ++;
            break;

            case 3: //Filling
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print(lcdCenterStr(products[product_slct]));
                lcd.setCursor(0, 1);
                lcd.print(lcdCenterStr("SURTIENDO " + (String)liters + " L"));
                lcd.setCursor(0, 3);
                lcd.print("Espere un momento...");
                delay(pump_times[product_slct - 1] * liters);
                cstmr_step ++;
            break;

            case 4: //Process completed
                setLeds(0);

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
            switch (prog_step) {
                case 1:
                    lcd.setCursor(0,1);
                    lcd.print ("  MODO PROGRAMADOR");
                    lcd.setCursor(0,2);
                    lcd.print(lcdCenterStr("ACTIVADO"));
                    delay(1500);
                    credits = 0;
                    lcd.clear();
                    prog_step ++;
                break;

                case 2:

                    if (digitalRead(prog_btn) == 0) {
                        lcd.clear();
                        prog_slct ++;
                        buzzHandle(100, 1);
                        delay(350);
                    }
                    else if (prog_slct == 6) {
                        prog_slct = 0;
                    }

                    if (prog_slct == prog_slct) {
                        lcd.setCursor(0, 1);
                        lcd.print(lcdCenterStr(products[prog_slct]));
                    }
                break;
            }
        }
    }
}

