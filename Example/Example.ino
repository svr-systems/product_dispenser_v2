#include <SD.h>
#include <RTClib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>

// Arrays
const byte leds[6] = {44, 42, 40, 38, 33, 34};
const byte cstmr_btns[6] = {45, 43, 41, 39, 37, 35};
const String products[6] = {"PRODUCTO 1", "PRODUCTO 2", "PRODUCTO 3", "PRODUCTO 4", "PRODUCTO 5", "PRODUCTO 6"};
int pump_amounts[6];
unsigned long pump_times[6];
float liters_content[6];
const byte bombs[6] = {22, 24, 23, 25, 27, 29};
// Pins
const byte prog_btn = 30;
const byte buzz_pin = 31;
const byte coin_btn = 49;
const byte coin = 2;
const byte test_btn = 46;
// Bools
bool cstmr_mode = true;
bool prog_mode = false;
bool test_mode = false;
bool bInserted = false;
bool started = false;
bool credits_displayed = false;
bool maintenance = false;
// Steps
byte cstmr_step = 1;
byte prog_step = 1;
byte product_slct;
byte prog_slct = 0;
byte prog_change = 1;
byte config_step = 0;
// Counts
unsigned long tiempoInicio = 0;
int credits;
float liters;
// SD
#define SSpin 53
File archive;
// Reloj
RTC_DS3231 rtc;
// LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long elapsedTime = 0;
bool btn_pressed = false;

void buzzHandle(int millis, byte pulses)
{
    for (byte i = 0; i < pulses; i++)
    {
        digitalWrite(buzz_pin, HIGH);
        delay(millis / 2);
        digitalWrite(buzz_pin, LOW);
        delay(millis / 2);
    }
}

void setLeds(boolean value)
{
    for (byte i = 0; i < sizeof(leds); i++)
    {
        digitalWrite(leds[i], value);
    }
}

void setLedBM(byte pinOn)
{

    for (byte i = 0; i < sizeof(leds); i++)
    {
        digitalWrite(leds[i], LOW);
    }

    digitalWrite(leds[pinOn], HIGH);
}

String lcdCenterStr(String str)
{
    byte adjust_center = (20 - str.length()) / 2;
    String str_temp = "";

    for (byte i = 0; i < adjust_center; i++)
        str_temp += " ";

    return str_temp + str;
}

void coinInterrupt()
{

    if (started && cstmr_step < 2 && test_mode == false && prog_mode == false)
    {
        credits++;
        bInserted = true;
    }
}

void modoprueba()
{
    if (test_mode == true)
    {
        credits = pump_amounts[product_slct];
    }
}

float readFloatFromFile(File &archive)
{
    char buffer[100];
    int bytesRead = archive.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    buffer[bytesRead] = '\0';

    String data = String(buffer);
    int lastIndex = -1;

    for (int i = data.length() - 1; i >= 0; i--)
    {
        if (isDigit(data.charAt(i)) || data.charAt(i) == '.')
        {
            lastIndex = i;
            while (i >= 0 && (isDigit(data.charAt(i)) || data.charAt(i) == '.'))
            {
                i--;
            }
            String numberStr = data.substring(i + 1, lastIndex + 1);
            return numberStr.toFloat();
        }
    }
    return 0.0;
}

void setup()
{
    Serial.begin(9600);

    for (byte i = 0; i < sizeof(leds); i++)
    {
        pinMode(leds[i], OUTPUT);
    }
    for (byte i = 0; i < sizeof(cstmr_btns); i++)
    {
        pinMode(cstmr_btns[i], RISING);
    }
    for (byte i = 0; i < sizeof(bombs); i++)
    {
        pinMode(bombs[i], OUTPUT);
    }
    pinMode(buzz_pin, OUTPUT);
    pinMode(coin_btn, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(coin), coinInterrupt, RISING);
    pinMode(prog_btn, INPUT);
    pinMode(test_btn, INPUT);

    lcd.init();
    lcd.backlight();
    lcd.clear();

    while (!SD.begin(SSpin))
    {
        {
            Serial.println("Fallo en inicialización");
            lcd.display();
            lcd.setCursor(0, 1);
            lcd.print(lcdCenterStr("FALLO DE"));
            lcd.setCursor(0, 2);
            lcd.print(lcdCenterStr("CONFIGURACION"));
            buzzHandle(400, 6);
            delay(3000);
            lcd.clear();
        }
    }
    if (SD.begin(SSpin))
    {

        lcd.display();
        lcd.setCursor(0, 1);
        lcd.print(lcdCenterStr("INICIANDO"));
        lcd.setCursor(0, 2);
        lcd.print(lcdCenterStr("CONFIGURACION"));
        buzzHandle(300, 4);
        delay(1000);
        lcd.clear();
    }

    archive = SD.open("config.txt");

    if (archive)
    {

        pump_amounts[0] = readFloatFromFile(archive);
        pump_times[0] = readFloatFromFile(archive);
        liters_content[0] = readFloatFromFile(archive);
        pump_amounts[1] = readFloatFromFile(archive);
        pump_times[1] = readFloatFromFile(archive);
        liters_content[1] = readFloatFromFile(archive);
        pump_amounts[2] = readFloatFromFile(archive);
        pump_times[2] = readFloatFromFile(archive);
        liters_content[2] = readFloatFromFile(archive);
        pump_amounts[3] = readFloatFromFile(archive);
        pump_times[3] = readFloatFromFile(archive);
        liters_content[3] = readFloatFromFile(archive);
        pump_amounts[4] = readFloatFromFile(archive);
        pump_times[4] = readFloatFromFile(archive);
        liters_content[4] = readFloatFromFile(archive);
        pump_amounts[5] = readFloatFromFile(archive);
        pump_times[5] = readFloatFromFile(archive);
        liters_content[5] = readFloatFromFile(archive);

        archive.close();
        lcd.setCursor(0, 0);
        lcd.print(lcdCenterStr("-CONFIGURACION-"));
        lcd.setCursor(0, 1);
        lcd.print(lcdCenterStr("P1 $" + (String)pump_amounts[0] + "|" + (String)(pump_times[0] / 1000) + "s"));
        lcd.setCursor(0, 2);
        lcd.print(lcdCenterStr("P2 $" + (String)pump_amounts[1] + "|" + (String)(pump_times[1] / 1000) + "s"));
        lcd.setCursor(0, 3);
        lcd.print(lcdCenterStr("P3 $" + (String)pump_amounts[2] + "|" + (String)(pump_times[2] / 1000) + "s"));
        delay(3000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(lcdCenterStr("-CONFIGURACION-"));
        lcd.setCursor(0, 1);
        lcd.print(lcdCenterStr("P4 $" + (String)pump_amounts[3] + "|" + (String)(pump_times[3] / 1000) + "s"));
        lcd.setCursor(0, 2);
        lcd.print(lcdCenterStr("P5 $" + (String)pump_amounts[4] + "|" + (String)(pump_times[4] / 1000) + "s"));
        lcd.setCursor(0, 3);
        lcd.print(lcdCenterStr("P6 $" + (String)pump_amounts[5] + "|" + (String)(pump_times[5] / 1000) + "s"));
        delay(3000);
        lcd.clear();
        buzzHandle(100, 1);
    }

    rtc.begin();
    started = true;
    // rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop()
{

    DateTime Date = rtc.now();

    // TEST MODE

    if (digitalRead(test_btn) == 1 && test_mode == false)
    {
        while (digitalRead(test_btn) == 1)
        {
        }

        credits_displayed = false;
        prog_mode = false;
        test_mode = true;
        credits = 100;
        lcd.clear();
        buzzHandle(100, 1);
        lcd.setCursor(0, 1);
        lcd.print(lcdCenterStr(" MODO PRUEBA"));
        lcd.setCursor(0, 2);
        lcd.print(lcdCenterStr("ACTIVADO"));
        buzzHandle(300, 4);
        lcd.clear();
    }

    else if (digitalRead(test_btn) == 1 && test_mode == true)
    {
        while (digitalRead(test_btn) == 1)
        {
        }

        test_mode = false;
        prog_mode = false;
        credits = 0;
        lcd.clear();
        buzzHandle(100, 1);
        lcd.setCursor(0, 1);
        lcd.print(lcdCenterStr("MODO PRUEBA"));
        lcd.setCursor(0, 2);
        lcd.print(lcdCenterStr("DESACTIVADO"));
        buzzHandle(300, 4);
        lcd.clear();
    }

    // PROG MODE

    if (digitalRead(prog_btn) == 1 && prog_mode == true)
    {

        SD.remove("config.txt");
        archive = SD.open("config.txt", FILE_WRITE);

        if (archive)
        {
            lcd.clear();
            lcd.setCursor(0, 1);
            lcd.print(lcdCenterStr("GUARDANDO"));
            lcd.setCursor(0, 2);
            lcd.print(lcdCenterStr("CONFIGURACION"));
            buzzHandle(200, 3);

            for (byte i = 0; i < 6; i++)
            {
                archive.println("PRODUCT_" + (String)i + "_PRICE" + " = " + (String)pump_amounts[i]);
                archive.println("PRODUCT_" + (String)i + "_TIME" + " = " + (String)pump_times[i]);
                archive.println("PRODUCT_" + (String)i + "_CONTENT" + " = " + (String)liters_content[i]);
            }

            delay(1000);
            lcd.clear();
            Serial.println("Hecho");
            archive.close();
        }
        else
        {
            Serial.println("Error");
        }

        lcd.clear();
        buzzHandle(100, 1);
        lcd.setCursor(0, 1);
        lcd.print("  MODO PROGRAMADOR");
        lcd.setCursor(0, 2);
        lcd.print(lcdCenterStr("DESACTIVADO"));
        buzzHandle(300, 4);
        prog_mode = false;
        cstmr_mode = true;
        cstmr_step = 1;
        prog_step = 0;
        lcd.clear();
    }

    else if (digitalRead(prog_btn) == 1 && prog_mode == false)
    {
        credits_displayed = false;
        prog_mode = true;
        cstmr_mode = false;
        prog_step = 1;
        cstmr_step = 1;
        lcd.clear();
        buzzHandle(100, 1);
    }

    if (cstmr_mode == true)
    {

    init:

        if (credits == 0 && test_mode == false)
        {
            lcd.display();
            lcd.setCursor(0, 1);
            lcd.print(lcdCenterStr("BIENVENIDO"));
            lcd.setCursor(0, 3);
            lcd.print("  INGRESE CREDITOS");

            for (byte i = 0; i < sizeof(leds); i++)
            {
                if (liters_content[i] < 5)
                {
                    digitalWrite(leds[i], 0);
                }
                else
                {
                    digitalWrite(leds[i], 1);
                }
            }
        }

        else if (credits >= 0 && test_mode == true)
        {

            lcd.display();
            lcd.setCursor(0, 1);
            lcd.print(lcdCenterStr("-PRUEBAS-"));

            for (byte i = 0; i < sizeof(leds); i++)
            {
                if (liters_content[i] < 5)
                {
                    digitalWrite(leds[i], 0);
                }
                else
                {
                    digitalWrite(leds[i], 1);
                }
            }
        }

        switch (cstmr_step)
        {
        case 1:

            if (credits > 0)
            {
                if (digitalRead(cstmr_btns[0]) == 1 && credits >= pump_amounts[0] && liters_content[0] >= 5)
                {
                    while (digitalRead(cstmr_btns[0]) == 1)
                    {
                    }

                    product_slct = 0;
                    modoprueba();
                    cstmr_step++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[1]) == 1 && credits >= pump_amounts[1] && liters_content[1] >= 5)
                {
                    while (digitalRead(cstmr_btns[1]) == 1)
                    {
                    }

                    product_slct = 1;
                    modoprueba();
                    cstmr_step++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[2]) == 1 && credits >= pump_amounts[2] && liters_content[2] >= 5)
                {
                    while (digitalRead(cstmr_btns[2]) == 1)
                    {
                    }

                    product_slct = 2;
                    modoprueba();
                    cstmr_step++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[3]) == 1 && credits >= pump_amounts[3] && liters_content[3] >= 5)
                {
                    while (digitalRead(cstmr_btns[3]) == 1)
                    {
                    }
                    product_slct = 3;
                    modoprueba();
                    cstmr_step++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[4]) == 1 && credits >= pump_amounts[4] && liters_content[4] >= 5)
                {
                    while (digitalRead(cstmr_btns[4]) == 1)
                    {
                    }
                    product_slct = 4;
                    modoprueba();
                    cstmr_step++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[5]) == 1 && credits >= pump_amounts[5] && liters_content[5] >= 5)
                {
                    while (digitalRead(cstmr_btns[5]) == 1)
                    {
                    }
                    product_slct = 5;
                    modoprueba();
                    cstmr_step++;
                    buzzHandle(100, 1);
                }
                else if (digitalRead(cstmr_btns[0]) == 1 && credits < pump_amounts[0] || digitalRead(cstmr_btns[1]) == 1 && credits < pump_amounts[1] || digitalRead(cstmr_btns[2]) == 1 && credits < pump_amounts[2] || digitalRead(cstmr_btns[3]) == 1 && credits < pump_amounts[3] || digitalRead(cstmr_btns[4]) == 1 && credits < pump_amounts[4] || digitalRead(cstmr_btns[5]) == 1 && credits < pump_amounts[5])
                {

                    lcd.clear();
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("CREDITOS"));
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr("INSUFICIENTES"));
                    buzzHandle(100, 4);
                    delay(1500);
                    lcd.clear();
                    lcd.setCursor(0, 1);
                    lcd.print("      CREDITOS");
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr("$" + (String)credits));
                }
                else if (digitalRead(cstmr_btns[0]) == 1 && liters_content[0] < 5 || digitalRead(cstmr_btns[1]) == 1 && liters_content[1] < 5 || digitalRead(cstmr_btns[2]) == 1 && liters_content[2] < 5 || digitalRead(cstmr_btns[3]) == 1 && liters_content[3] < 5 || digitalRead(cstmr_btns[4]) == 1 && liters_content[4] < 5 || digitalRead(cstmr_btns[5]) == 1 && liters_content[5] < 5)
                {

                    lcd.clear();
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("LITROS"));
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr("INSUFICIENTES"));
                    buzzHandle(100, 4);
                    delay(1500);
                    lcd.clear();

                    if (test_mode == false)
                    {
                        lcd.clear();
                        lcd.setCursor(0, 1);
                        lcd.print("      CREDITOS");
                        lcd.setCursor(0, 2);
                        lcd.print(lcdCenterStr("$" + (String)credits));
                    }
                }
            }

            if (bInserted == true && test_mode == false)
            {
                bInserted = false;
                credits_displayed = true;
                tiempoInicio = millis();
                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("      CREDITOS");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr("$" + (String)credits));
            }
            else if (digitalRead(coin_btn) == 0 && test_mode == false)
            {

                credits++;

                credits_displayed = true;
                tiempoInicio = millis();
                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("      CREDITOS");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr("$" + (String)credits));
            }
            else if (millis() - tiempoInicio >= 180000 && credits_displayed == true)
            {

                credits = 0;
                lcd.clear();
                credits_displayed = false;
                goto init;
            }
            break;

        case 2:

            credits_displayed = false;
            liters = credits / (float)pump_amounts[product_slct];

            if (liters <= 5)
            {

                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("   COLOQUE ENVASE");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr((String)liters + " L"));
                buzzHandle(250, 10);
                cstmr_step++;
            }
            else
            {
                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print(lcdCenterStr("5 LITROS"));
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr("MINIMO"));
                buzzHandle(400, 6);
                delay(1500);

                lcd.clear();
                lcd.setCursor(0, 1);
                lcd.print("      CREDITOS");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr("$" + (String)credits));

                cstmr_step = 1;
                product_slct = 0;
            }
            break;

        case 3:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(lcdCenterStr(products[product_slct]));
            lcd.setCursor(0, 1);
            lcd.print(lcdCenterStr("SURTIENDO " + (String)liters + " L"));
            lcd.setCursor(0, 3);
            lcd.print("ESPERE UN MOMENTO...");
            setLedBM(product_slct);
            digitalWrite(bombs[product_slct], 1);
            delay(pump_times[product_slct] * liters);
            digitalWrite(bombs[product_slct], 0);
            cstmr_step++;
            break;

        case 4:
            setLeds(1);
            liters_content[product_slct] -= liters;
            lcd.clear();
            buzzHandle(100, 3);
            lcd.setCursor(0, 1);
            lcd.print("GRACIAS POR COMPRAR!");
            lcd.setCursor(0, 3);
            lcd.print("   VUELVA PRONTO");
            delay(3000);

            if (test_mode == false)
            {

                archive = SD.open("reports");

                if (!SD.exists("reports/" + (String)Date.year() + "_" + (String)Date.month()))
                {
                    SD.mkdir("reports/" + (String)Date.year() + "_" + (String)Date.month());
                    Serial.println("Carpeta creada correctamente");
                    archive.close();
                }
                else
                {
                    Serial.println("La carpeta ya existe");
                    archive.close();
                }

                archive = SD.open("reports/" + (String)Date.year() + "_" + (String)Date.month() + "/" + (String)Date.day() + ".txt", FILE_WRITE);

                if (archive)
                {
                    archive.println((String)Date.year() + "-" + (String)Date.month() + "-" + (String)Date.day() + " P" + (String)product_slct + " " + (String)credits + " " + (String)liters);
                    archive.close();
                    Serial.println("Archivo creado correctamente");
                }
                else
                {
                    Serial.println("Error al crear el archivo");
                }
            }
            else
            {

                archive = SD.open("reports");

                if (!SD.exists("reports/" + (String)Date.year() + "_" + (String)Date.month()))
                {
                    SD.mkdir("reports/" + (String)Date.year() + "_" + (String)Date.month());
                    Serial.println("Carpeta de prueba creada correctamente");
                    archive.close();
                }
                else
                {
                    Serial.println("La carpeta de prueba ya existe");
                    archive.close();
                }

                archive = SD.open("reports/" + (String)Date.year() + "_" + (String)Date.month() + "/" + "TEST_" + (String)Date.day() + ".txt", FILE_WRITE);

                if (archive)
                {
                    archive.println((String)Date.year() + "-" + (String)Date.month() + "-" + (String)Date.day() + " P" + (String)product_slct + " " + (String)credits + " " + (String)liters);
                    archive.close();
                    Serial.println("Archivo de prueba creado correctamente");
                }
                else
                {
                    Serial.println("Error al crear el archivo de prueba");
                }
            }

            if (test_mode == true)
            {
                credits = 100;
            }
            else
            {
                credits = 0;
            }

            cstmr_step = 1;
            liters = 0;
            product_slct = 0;
            lcd.clear();

            break;
        }
    }
    else
    {
        if (prog_mode == true)
        {
            setLeds(0);

            switch (prog_step)
            {
            case 1:
                lcd.setCursor(0, 1);
                lcd.print("  MODO PROGRAMADOR");
                lcd.setCursor(0, 2);
                lcd.print(lcdCenterStr("ACTIVADO"));
                buzzHandle(300, 4);
                credits = 0;
                prog_change = 1;
                lcd.clear();
                prog_step++;
                break;

            case 2:
                if (digitalRead(cstmr_btns[0]) == 1)
                {
                    while (digitalRead(cstmr_btns[0]) == 1)
                    {
                    }
                    lcd.clear();
                    prog_change++;
                    delay(100);
                    buzzHandle(100, 1);
                }
                else if (prog_change == 6)
                {
                    prog_change = 0;
                }

                if (prog_change == 1)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("-CAMBIAR PRECIO-"));
                }
                else if (prog_change == 2)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("-CAMBIAR TIEMPO-"));
                }
                else if (prog_change == 3)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("-RELLENAR-"));
                }
                else if (prog_change == 4)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("-HORARIO-"));
                }
                else if (prog_change == 5)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("-CONFIGURACION-"));
                }
                else if (prog_change == 0)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("-MANTENIMIENTO-"));
                }

                if (digitalRead(cstmr_btns[1]) == 1)
                {
                    while (digitalRead(cstmr_btns[1]) == 1)
                    {
                    }
                    buzzHandle(100, 1);
                    lcd.clear();
                    if (prog_change == 4 || prog_change == 5 || prog_change == 0)
                    {
                        prog_step = 4;
                        delay(350);
                    }
                    else
                    {
                        prog_step++;
                        delay(350);
                    }
                }
                break;

            case 3:
                if (prog_slct == prog_slct)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr(products[prog_slct]));
                }

                if (digitalRead(cstmr_btns[0]) == 1)
                {
                    while (digitalRead(cstmr_btns[0]) == 1)
                    {
                    }
                    lcd.clear();
                    prog_slct++;
                    buzzHandle(100, 1);
                }
                else if (prog_slct == 6)
                {
                    prog_slct = 0;
                }
                else if (digitalRead(cstmr_btns[1]) == 1)
                {
                    while (digitalRead(cstmr_btns[1]) == 1)
                    {
                    }
                    buzzHandle(100, 1);
                    lcd.clear();
                    prog_step++;
                }
                else if (digitalRead(cstmr_btns[2]) == 1)
                {
                    while (digitalRead(cstmr_btns[2]) == 1)
                    {
                    }
                    buzzHandle(100, 1);
                    prog_slct = 0;
                    prog_change = 1;
                    prog_step--;
                }
                break;

            case 4:
                if (prog_change == 1)
                {

                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("PRECIO / LITRO"));
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr("$" + (String)pump_amounts[prog_slct]));

                    if (digitalRead(cstmr_btns[1]) == 1)
                    {
                        while (digitalRead(cstmr_btns[1]) == 1)
                        {
                        }
                        pump_amounts[prog_slct] += 1;
                        buzzHandle(100, 1);
                        delay(150);
                    }
                    else if (digitalRead(cstmr_btns[2]) == 1)
                    {
                        while (digitalRead(cstmr_btns[2]) == 1)
                        {
                        }

                        if (pump_amounts[prog_slct] >= 2)
                        {
                            pump_amounts[prog_slct] -= 1;
                            buzzHandle(100, 1);
                            delay(150);
                        }
                    }
                    else if (digitalRead(cstmr_btns[0]) == 1)
                    {
                        while (digitalRead(cstmr_btns[0]) == 1)
                        {
                        }
                        buzzHandle(100, 1);
                        lcd.clear();
                        prog_slct = 0;
                        prog_step = 3;
                    }
                    else if (digitalRead(cstmr_btns[2]) == 1)
                    {
                        while (digitalRead(cstmr_btns[2]) == 1)
                        {
                        }

                        buzzHandle(100, 1);
                        prog_step = 2;
                    }
                }
                else if (prog_change == 2)
                {
                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("TIEMPO DE BOMBA"));
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr((String)pump_times[prog_slct] + " ms"));

                    byte btn_state = digitalRead(cstmr_btns[1]);

                    if (btn_state == 1 && !btn_pressed)
                    {
                        startTime = millis();
                        btn_pressed = true;
                        lcd.clear();
                        lcd.setCursor(0, 3);
                        lcd.print(lcdCenterStr(" CALIBRANDO..."));
                        digitalWrite(bombs[prog_slct], 1);
                    }
                    else if (btn_state == 0 && btn_pressed)
                    {
                        lcd.clear();
                        digitalWrite(bombs[prog_slct], 0);
                        endTime = millis();
                        btn_pressed = false;
                        elapsedTime = endTime - startTime;
                        pump_times[prog_slct] = elapsedTime;
                    }
                    else if (digitalRead(cstmr_btns[0]) == 1)
                    {
                        while (digitalRead(cstmr_btns[0]) == 1)
                        {
                        }
                        buzzHandle(100, 1);
                        lcd.clear();
                        prog_change = 2;
                        prog_slct = 0;
                        prog_step = 3;
                    }
                }
                else if (prog_change == 3)
                {

                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("CANTIDAD PRODUCTO"));
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr((String)liters_content[prog_slct] + " L"));

                    if (digitalRead(cstmr_btns[1]) == 1)
                    {
                        while (digitalRead(cstmr_btns[1]) == 1)
                        {
                        }

                        buzzHandle(100, 1);
                        liters_content[prog_slct] += 1;
                        delay(150);
                    }
                    else if (digitalRead(cstmr_btns[2]) == 1)
                    {
                        while (digitalRead(cstmr_btns[2]) == 1)
                        {
                        }

                        if (liters_content[prog_slct] >= 2)
                        {
                            buzzHandle(100, 1);
                            liters_content[prog_slct] -= 1;
                            delay(150);
                        }
                    }
                    else if (digitalRead(cstmr_btns[0]) == 1)
                    {
                        while (digitalRead(cstmr_btns[0]) == 1)
                        {
                        }
                        buzzHandle(100, 1);
                        lcd.clear();
                        prog_step = 3;
                        prog_change = 3;
                        prog_slct = 0;
                    }

                    else if (digitalRead(cstmr_btns[2]) == 1)
                    {
                        while (digitalRead(cstmr_btns[2]) == 1)
                        {
                        }

                        buzzHandle(100, 1);
                        prog_step = 2;
                        prog_change = 1;
                    }
                }
                else if (prog_change == 4)
                {
                    lcd.setCursor(0, 0);
                    lcd.print(lcdCenterStr("HORARIO"));
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr((String)Date.hour() + ":" + (String)Date.minute()));
                    lcd.setCursor(0, 3);
                    lcd.print(lcdCenterStr((String)Date.day() + "/" + (String)Date.month() + "/" + (String)Date.year()));

                    if (digitalRead(cstmr_btns[0]) == 1)
                    {
                        while (digitalRead(cstmr_btns[0]) == 1)
                        {
                        }
                        buzzHandle(100, 1);
                        lcd.clear();
                        prog_step = 2;
                        prog_change = 1;
                        prog_slct = 0;
                    }
                }
                else if (prog_change == 5)
                {

                    if (digitalRead(cstmr_btns[1]) == 1)
                    {
                        while (digitalRead(cstmr_btns[1]) == 1)
                        {
                        }

                        config_step++;
                        buzzHandle(100, 1);
                        lcd.clear();
                    }
                    else if (config_step == 2)
                    {
                        config_step = 0;
                    }

                    if (config_step == 0)
                    {
                        lcd.setCursor(0, 0);
                        lcd.print(lcdCenterStr("-CONFIGURACION-"));
                        lcd.setCursor(0, 1);
                        lcd.print(lcdCenterStr("P1 $" + (String)pump_amounts[0] + "|" + (String)(pump_times[0] / 1000) + "s"));
                        lcd.setCursor(0, 2);
                        lcd.print(lcdCenterStr("P2 $" + (String)pump_amounts[1] + "|" + (String)(pump_times[1] / 1000) + "s"));
                        lcd.setCursor(0, 3);
                        lcd.print(lcdCenterStr("P3 $" + (String)pump_amounts[2] + "|" + (String)(pump_times[2] / 1000) + "s"));
                    }
                    else
                    {
                        lcd.setCursor(0, 0);
                        lcd.print(lcdCenterStr("-CONFIGURACION-"));
                        lcd.setCursor(0, 1);
                        lcd.print(lcdCenterStr("P4 $" + (String)pump_amounts[3] + "|" + (String)(pump_times[3] / 1000) + "s"));
                        lcd.setCursor(0, 2);
                        lcd.print(lcdCenterStr("P5 $" + (String)pump_amounts[4] + "|" + (String)(pump_times[4] / 1000) + "s"));
                        lcd.setCursor(0, 3);
                        lcd.print(lcdCenterStr("P6 $" + (String)pump_amounts[5] + "|" + (String)(pump_times[5] / 1000) + "s"));
                    }

                    if (digitalRead(cstmr_btns[0]) == 1)
                    {
                        while (digitalRead(cstmr_btns[0]) == 1)
                        {
                        }
                        buzzHandle(100, 1);
                        lcd.clear();
                        prog_step = 2;
                        prog_change = 1;
                        prog_slct = 0;
                    }
                }
                else if (prog_change == 0)
                {

                    lcd.setCursor(0, 1);
                    lcd.print(lcdCenterStr("MANTENIMIENTO"));
                    lcd.setCursor(0, 2);
                    lcd.print(lcdCenterStr("DESACTIVADO"));

                    if (digitalRead(cstmr_btns[1]) == 1 && maintenance == false)
                    {
                        while (digitalRead(cstmr_btns[1]) == 1)
                        {
                        }

                        lcd.clear();
                        lcd.setCursor(0, 1);
                        lcd.print(lcdCenterStr("INICIANDO"));
                        lcd.setCursor(0, 2);
                        lcd.print(lcdCenterStr("MANTENIMIENTO"));
                        buzzHandle(300, 4);
                        delay(2000);
                        lcd.clear();
                        lcd.setCursor(0, 1);
                        lcd.print(lcdCenterStr("MANTENIMIENTO"));
                        lcd.setCursor(0, 2);
                        lcd.print(lcdCenterStr("EN CURSO"));
                        buzzHandle(100, 4);

                        for (byte i = 0; i < sizeof(bombs); i++)
                        {
                            digitalWrite(bombs[i], 1);
                        }

                        delay(45000);

                        for (byte i = 0; i < sizeof(bombs); i++)
                        {
                            digitalWrite(bombs[i], 0);
                        }

                        lcd.clear();
                        lcd.setCursor(0, 1);
                        lcd.print(lcdCenterStr("MANTENIMIENTO"));
                        lcd.setCursor(0, 2);
                        lcd.print(lcdCenterStr("COMPLETADO"));
                        lcd.setCursor(0, 3);
                        lcd.print(lcdCenterStr("REGRESE MANGUERAS"));
                        buzzHandle(300, 4);
                        delay(4000);
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
