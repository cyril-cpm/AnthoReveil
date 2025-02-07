#include <TFT_eSPI.h> // Inclure la librairie TFT_eSPI
#include <Settingator.h>
#include <HTTPServer.h>
#include <WebSocketCommunicator.h>
#include <TimeLib.h>
#include <ESP32Servo.h>
#include <FS5113R.h>
#include <SPIFFS.h>

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 160

#define PIN_A 33
#define PIN_B 32

#define SIMPLE 1
#define DOUBLE 2

#define LIGHT_PIN 21


//Servo pistolServo;
FS5113R pistolServo(19);

TFT_eSPI tft = TFT_eSPI();  // Créer un objet TFT

TFT_eSprite sprite(&tft);

#define STICK_TOPLEFT   1
#define STICK_BOTLEFT   2
#define STICK_TOPRIGHT  3
#define STICK_BOTRIGHT  4
#define STICK_CENTER    5

uint8_t interpolateVar = 0;
uint8_t lightningNumber = 2;

bool alarmActivated = true;
bool isHourWebSynced = false;
uint8_t light = 9;

bool    mode = true;

void drawBackground();

void interpolatePoint(uint8_t a_x, uint8_t a_y, uint8_t b_x, uint8_t b_y, uint8_t t, uint8_t* ret_x, uint8_t* ret_y) {
    *ret_x = a_x + ((b_x - a_x) * t) / 255;
    *ret_y = a_y + ((b_y - a_y) * t) / 255;
}

void drawLightning(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t stick = STICK_TOPLEFT)
{
    uint8_t center_x = 0;
    uint8_t center_y = 0;

    switch (stick)
    {
        case STICK_TOPLEFT:
            center_x = x + w/2;
            center_y = y + h/2;
            break;

        case STICK_BOTLEFT:
            center_x = x + w/2;
            center_y = y - h/2;
            break;

        case STICK_TOPRIGHT:
            center_x = x - w/2;
            center_y = y + h/2;
            break;

        case STICK_BOTRIGHT:
            center_x = x - w/2;
            center_y = y - h/2;
            break;

        case STICK_CENTER:
            center_x = x;
            center_y = y;
            break;
    }
    float fa_x = -1.0;
    float fa_y = 0.17;
    
    float fb_x = 0.57;
    float fb_y = -1.0;

    float fc_x = -0.057;
    float fc_y = 0.17;

    uint8_t up_a_x = center_x + (uint8_t)(fa_x * ((float)w / 2.0));
    uint8_t up_a_y = center_y + (uint8_t)(fa_y * ((float)h / 2.0));

    uint8_t up_b_x = center_x + (uint8_t)(fb_x * ((float)w / 2.0));
    uint8_t up_b_y = center_y + (uint8_t)(fb_y * ((float)h / 2.0));

    uint8_t up_c_x = center_x + (uint8_t)(fc_x * ((float)w / 2.0));
    uint8_t up_c_y = center_y + (uint8_t)(fc_y * ((float)h / 2.0));

    sprite.fillTriangle(up_a_x, up_a_y, up_b_x, up_b_y, up_c_x, up_c_y, TFT_YELLOW);

    fa_x = 1.0;
    fa_y = -0.17;
    
    fb_x = -0.57;
    fb_y = 1.0;

    fc_x = 0.057;
    fc_y = -0.17;

    uint8_t dw_a_x = center_x + (uint8_t)(fa_x * ((float)w / 2.0));
    uint8_t dw_a_y = center_y + (uint8_t)(fa_y * ((float)h / 2.0));

    uint8_t dw_b_x = center_x + (uint8_t)(fb_x * ((float)w / 2.0));
    uint8_t dw_b_y = center_y + (uint8_t)(fb_y * ((float)h / 2.0));

    uint8_t dw_c_x = center_x + (uint8_t)(fc_x * ((float)w / 2.0));
    uint8_t dw_c_y = center_y + (uint8_t)(fc_y * ((float)h / 2.0));

    sprite.fillTriangle(dw_a_x, dw_a_y, dw_b_x, dw_b_y, dw_c_x, dw_c_y, TFT_YELLOW);

    uint8_t pa_x = 0;
    uint8_t pa_y = 0;
    uint8_t pb_x = 0;
    uint8_t pb_y = 0;
    uint8_t pmid_x = 0;
    uint8_t pmid_y = 0;

    if (mode)
    {
        for (int i = 0; i < lightningNumber; i++)
        {
            pa_x = 0;
            pa_y = 0;

            interpolatePoint(up_b_x, up_b_y, up_a_x, up_a_y, random(0,255)/*interpolateVar*/, &pa_x, &pa_y);

            pb_x = 0;
            pb_y = 0;

            interpolatePoint(up_b_x, up_b_y, up_c_x, up_c_y, random(0,255)/*interpolateVar*/, &pb_x, &pb_y);

            pmid_x = 0;
            pmid_y = 0;

            pmid_x = random(pa_x, pb_x);
            pmid_y = random(-(pb_x-pa_x)/2, (pb_x-pa_x)/2) + pa_y; 

            sprite.drawWideLine(pa_x, pa_y, pmid_x, pmid_y, 1, TFT_SKYBLUE, TFT_BLUE);
            sprite.drawWideLine(pb_x, pb_y, pmid_x, pmid_y, 1, TFT_SKYBLUE, TFT_BLUE);

            sprite.drawLine(pa_x, pa_y, pmid_x, pmid_y, TFT_WHITE);
            sprite.drawLine(pb_x, pb_y, pmid_x, pmid_y, TFT_WHITE);
        
            pa_x = 0;
            pa_y = 0;

            interpolatePoint(dw_b_x, dw_b_y, dw_a_x, dw_a_y, random(0,255)/*interpolateVar*/, &pa_x, &pa_y);

            pb_x = 0;
            pb_y = 0;

            interpolatePoint(dw_b_x, dw_b_y, dw_c_x, dw_c_y, random(0,255)/*interpolateVar*/, &pb_x, &pb_y);

            pmid_x = 0;
            pmid_y = 0;

            pmid_x = random(pb_x, pa_x);
            pmid_y = random((pb_x-pa_x)/2, -(pb_x-pa_x)/2) + pa_y; 

            sprite.drawWideLine(pa_x, pa_y, pmid_x, pmid_y, 1, TFT_SKYBLUE, TFT_BLUE);
            sprite.drawWideLine(pb_x, pb_y, pmid_x, pmid_y, 1, TFT_SKYBLUE, TFT_BLUE);

            sprite.drawLine(pa_x, pa_y, pmid_x, pmid_y, TFT_WHITE);
            sprite.drawLine(pb_x, pb_y, pmid_x, pmid_y, TFT_WHITE);
        }
    }

    if (!mode)
    {
        pa_x = 0;
        pa_y = 0;

        pb_x = 0;
        pb_y = 0;

        if (interpolateVar*2 < 255)
        {
            interpolatePoint(up_b_x, up_b_y, up_a_x, up_a_y, /*random(0,255)*/interpolateVar*2, &pa_x, &pa_y);

            interpolatePoint(up_b_x, up_b_y, up_c_x, up_c_y, /*random(0,255)*/interpolateVar*2, &pb_x, &pb_y);

            if (pb_y > dw_a_y)
            {
                interpolatePoint(dw_a_x, dw_a_y, dw_b_x, dw_b_y, /*random(0,255)*/interpolateVar*2-0.66*255, &pb_x, &pb_y);
            }
        }
        else
        {
            interpolatePoint(dw_c_x, dw_c_y, dw_b_x, dw_b_y, interpolateVar*2+0.34*255, &pa_x, &pa_y);
            interpolatePoint(dw_a_x, dw_a_y, dw_b_x, dw_b_y, interpolateVar*2+0.34*255, &pb_x, &pb_y);
        }

        if (interpolateVar + 0.17*255 > 255)
            interpolateVar = 0;

        pmid_x = 0;
        pmid_y = 0;

        pmid_x = random(pa_x, pb_x);
        pmid_y = random(-(pb_x-pa_x)/2, (pb_x-pa_x)/2) + pa_y; 

        sprite.drawWideLine(pa_x, pa_y, pmid_x, pmid_y, 1, TFT_SKYBLUE, TFT_BLUE);
        sprite.drawWideLine(pb_x, pb_y, pmid_x, pmid_y, 1, TFT_SKYBLUE, TFT_BLUE);

        sprite.drawLine(pa_x, pa_y, pmid_x, pmid_y, TFT_WHITE);
        sprite.drawLine(pb_x, pb_y, pmid_x, pmid_y, TFT_WHITE);
    }

}


uint8_t selectedItem = 0;

bool blinkOn()
{
    return millis() / 250 % 2;
}

void printHour(String& text, bool setting = false)
{
    if (setting)
    {
        for (int i = 0; i < 5; i++)
        {
            if ((i != selectedItem) || (i == selectedItem && blinkOn()))
                sprite.print(text[i]);
            else
            {

                int x_offset = sprite.getCursorX();
                
                int hauteur = 128;
                int largeur = 160 - x_offset;
                
                
                sprite.print(text[i]);

                int afterX = sprite.getCursorX();

            }
        }
    }
    else
        sprite.print(text);
}

void drawTextWithOutline(int x, int y, String text, uint8_t textSize, uint16_t textColor, uint16_t contourColor, bool setting = false) {
  // Dessiner le contour : on déplace le texte de -1 et +1 pixels pour chaque côté
  //sprite.setTextSize(textSize);
  sprite.setTextColor(contourColor);

 for (int i = 0; i < 5; i++)
 {
    if ((setting && selectedItem == i && blinkOn()) || (!setting) || (selectedItem != i))
    {
        sprite.setTextColor(contourColor);

        // Dessiner le contour en déplaçant le texte dans toutes les directions
        sprite.setCursor(x - 2, y - 2);  sprite.print(text[i]);
        sprite.setCursor(x + 2, y - 2);  sprite.print(text[i]);
        sprite.setCursor(x - 2, y + 2);  sprite.print(text[i]);
        sprite.setCursor(x + 2, y + 2);  sprite.print(text[i]);
        sprite.setCursor(x, y - 2);      sprite.print(text[i]);
        sprite.setCursor(x, y + 2);      sprite.print(text[i]);
        sprite.setCursor(x - 2, y);      sprite.print(text[i]);
        sprite.setCursor(x + 2, y);      sprite.print(text[i]);

        // Dessiner le texte principal par-dessus
        sprite.setTextColor(textColor);
        sprite.setCursor(x, y);
        sprite.print(text[i]);

        x = sprite.getCursorX();
        y = sprite.getCursorY();
    }
    else 
    {
        x += sprite.textWidth(String(text[i]));
    }
 }
}

int alarmHour = 0;
int alarmMinute = 0;

void displayTime(bool alarm = false, bool setting = false) {
  // Effacer l'écran pour ne pas laisser de traces

  // Récupérer l'heure actuelle
  int curhour;
  int curminute;

  if (alarm)
  {
    curhour = alarmHour;
    curminute = alarmMinute;
  }
  else
  {
    curhour = hour();
    curminute = minute();
  }

  // Formater l'heure au format "12:34"
  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", curhour, curminute);

  // Définir la police et la taille du texte
  sprite.setTextFont(2);  // Police 4x6 ou 6x8
  sprite.setTextSize(4);   // Taille du texte (ajuster en fonction de la taille de l'écran)
  sprite.setTextColor(TFT_WHITE);  // Couleur du texte (blanc sur fond noir)

  // Calculer la position pour centrer le texte
  int16_t textWidth = sprite.textWidth(timeStr);  // Largeur du texte
  int16_t textHeight = sprite.fontHeight();  // Hauteur du texte
  int x = (sprite.width() - textWidth) / 2;  // Position X pour centrer
  int y = (sprite.height() - textHeight) / 2;  // Position Y pour centrer

  // Afficher l'heure au centre de l'écran

  drawTextWithOutline(x, y, timeStr, 4, TFT_WHITE, TFT_BLACK, setting);

  if (!alarm && !setting && alarmActivated)
  {
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_WHITE);
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", alarmHour, alarmMinute);
    sprite.setTextFont(1);
    x = sprite.textWidth(timeStr);
    y = sprite.fontHeight();
    
    sprite.setCursor(SCREEN_WIDTH - x, SCREEN_HEIGHT - y);
    sprite.println(timeStr);
  }
}


HTTPServer* server = nullptr;

void shoot()
{
    pistolServo.write(130);
    delay(1000);
    pistolServo.write(90);
}

uint16_t bitmap[128*160];

void loadBackground()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Erreur SPIFFS !");
        return;
    }

    auto bmpFile = SPIFFS.open("/bg.bmp", "r");

    uint8_t header[54];
    bmpFile.read(header, 54);

        // Vérifier si c'est bien un fichier BMP
    if (header[0] != 'B' || header[1] != 'M')
    {
        Serial.println("Pas un BMP valide !");
        bmpFile.close();
        return;
    }

    int bmpWidth = 160;
    int bmpHeight = 128;

    uint32_t startByte = *((uint8_t*)header + 10);


    bmpFile.seek(startByte);
    for (int row = 0; row < bmpHeight; row++)
    {
        for (int col = 0; col < bmpWidth; col++)
        {
            uint16_t color;
            bmpFile.read((uint8_t*)&color, 2);
            bitmap[bmpWidth * bmpHeight - col - row * bmpWidth] = color;
        }
    }
    bmpFile.close();
    SPIFFS.end();
}

void drawBackground()
{
  for (int row = 0; row < 128; row++)
  {
        for (int col = 0; col < 160; col++)
        {
            sprite.drawPixel(col, row, bitmap[col + row * 160]);
        }
    }
}


class Page
{
public:
    Page(void (*displayCB)(Page* parent, Page** children, int nbChildren), void (*validateCB)(Page* parent, Page** children, int nbChildren), String name = "", Page* parent = nullptr, Page** children = nullptr, int nbChildren = 0) : _displayCB(displayCB), _validateCB(validateCB),  _name(name), _parent(parent), _children(children), _nbChildren(nbChildren) {}
    String& getName() { return _name; }
    void display() {if (_displayCB) _displayCB(_parent, _children, _nbChildren);}
    void validate() {if (_validateCB) _validateCB(_parent, _children, _nbChildren);}
    void setChildren(Page** children, int nbChildren) { _children = children; _nbChildren = nbChildren; }
    int  getNbChildren() { return _nbChildren; }
    Page* getParent() { return _parent; }
    void setCheckNavigate(void (*checkNavigateCB)(Page* parent, Page** children, int nbChildren)) { _checkNavigateCB = checkNavigateCB; }
    void  checkNavigate() { if (_checkNavigateCB) _checkNavigateCB(_parent, _children, _nbChildren); }
    void setBoolean(bool* value) { _boolValue = value; }
    bool* getBoolean() { return _boolValue; }
    void setInt(uint8_t* value) { _intValue = value; }
    uint8_t* getInt() { return _intValue; }

protected:
    Page* _parent = nullptr;
    Page** _children = nullptr;
    int   _nbChildren = 0;
    String _name = "";
    void (*_displayCB)(Page* parent, Page** children, int nbChildren) = nullptr;
    void (*_validateCB)(Page* parent, Page** children, int nbChildren) = nullptr;
    void (*_checkNavigateCB)(Page* parent, Page** children, int nbChildren) = nullptr;
    bool* _boolValue = nullptr;
    uint8_t* _intValue = nullptr;
};

Page* currentPage = nullptr;

void switchPage(Page* newPage)
{
    if (newPage)
        currentPage = newPage;
    selectedItem = 0;
}

Page hourDisplay([](Page* parent, Page** children, int nbChildren){displayTime();}, nullptr);

void mainMenuCB(Page* parent, Page** children, int nbChildren)
{
    int lineHeight = SCREEN_HEIGHT / (nbChildren + 2);
    sprite.setTextFont(1);
    sprite.setTextSize(1);
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    for (int i = 1; i <= nbChildren; i++)
    {
        sprite.setCursor(10, i * lineHeight);

        if ((i - 1 == selectedItem && blinkOn()) || (i-1 != selectedItem))
        {
            sprite.println(children[i-1]->getName());

            if (children[i - 1]->getBoolean() != nullptr)
            {
                if (*(children[i - 1]->getBoolean()))
                {
                    sprite.setCursor(160 - 10 - sprite.textWidth("ON"), i * lineHeight);
                    sprite.println("ON");
                }
                else
                {
                    sprite.setCursor(160 - 10 - sprite.textWidth("OFF"), i * lineHeight);
                    sprite.println("OFF");
                }

                
            }
            else if (children[i - 1]->getInt() != nullptr)
            {
                char intChar[2];

                itoa(*(children[i - 1]->getInt()), intChar, 10);
                sprite.setCursor(160 - 10 - sprite.textWidth(intChar), i * lineHeight);
                sprite.println(intChar);
            }
        }
    }
}

void gotoPageCB(Page* parent, Page** children, int nbChildren)
{
    if (selectedItem < nbChildren)
    {
        if (children[selectedItem]->getBoolean())
            *(children[selectedItem]->getBoolean()) = !*(children[selectedItem]->getBoolean());
        else if (children[selectedItem]->getInt())
            *(children[selectedItem]->getInt()) = (*(children[selectedItem]->getInt()) + 1) % 10;
        else
            switchPage(children[selectedItem]);
    }
}

Page mainMenu(&mainMenuCB, &gotoPageCB);

void hourSettingDisplay(Page* parent, Page** children, int nbChildren)
{
    displayTime(false, true);
}

void noMiddleSelection(Page* parent, Page** children, int nbChildren)
{
    if (selectedItem == 2)
        selectedItem++;
}

void hourSettingValidate(Page* parent, Page** children, int nbChildren)
{
    int curhour = hour();
    int curminute = minute();

    if (selectedItem == 0)
    {
        if (curhour < 14)
            curhour += 10;
        else if (curhour >= 20)
            curhour -= 20;
        else
            curhour -= 10;
    }
    else if (selectedItem == 1)
    {
        if (curhour % 10 == 9)
            curhour -= 9;
        else if (curhour == 23)
            curhour = 20;
        else
            curhour++;
    }
    else if (selectedItem == 3)
    {
        if (curminute < 50)
            curminute += 10;
        else
            curminute -= 50;
    }
    else if (selectedItem == 4)
    {
        if (curminute % 10 != 9)
            curminute++;
        else
            curminute -= 9;
    }

    setTime(curhour, curminute, second(), day(), month(), year());
}

void alarmSettingDisplay(Page* parent, Page** children, int nbChildren)
{
    displayTime(true, true);
}

void alarmSettingValidate(Page* parent, Page** children, int nbChildren)
{
    if (selectedItem == 0)
    {
        if (alarmHour < 14)
            alarmHour += 10;
        else if (alarmHour >= 20)
            alarmHour -= 20;
        else
            alarmHour -= 10;
    }
    else if (selectedItem == 1)
    {
        if (alarmHour % 10 == 9)
            alarmHour -= 9;
        else if (alarmHour == 23)
            alarmHour = 20;
        else
            alarmHour++;
    }
    else if (selectedItem == 3)
    {
        if (alarmMinute < 50)
            alarmMinute += 10;
        else
            alarmMinute -= 50;
    }
    else if (selectedItem == 4)
    {
        if (alarmMinute % 10 != 9)
            alarmMinute++;
        else
            alarmMinute -= 9;
    }

    //Sauvegarder de manière persistante
}

Page hourSetting(&hourSettingDisplay, &hourSettingValidate, "Regler l'heure", &mainMenu, nullptr, 5);
Page hourWebSynced(nullptr, nullptr, "Synchro Web", &mainMenu);
Page alarmSetting(&alarmSettingDisplay, &alarmSettingValidate, "Regler l'alarme", &mainMenu);
Page alarmActivation(nullptr, nullptr, "Activer l'alarme", &mainMenu);
Page lightSetting(nullptr, nullptr, "Regler luminosite", &mainMenu);
Page featureActivation(nullptr, nullptr, "Activer fonction alarme", &mainMenu);

Page* mainMenuChildren[] = {
    &hourSetting,
    &hourWebSynced,
    &alarmSetting,
    &alarmActivation,
    &lightSetting,
    &featureActivation
    };

void menuNavigate()
{
    selectedItem++;

    if (selectedItem == currentPage->getNbChildren())
        selectedItem = 0;

    if (currentPage)
        currentPage->checkNavigate();
}

void menuReturn()
{
    if (currentPage && currentPage->getParent())
        switchPage(currentPage->getParent());
    else
        switchPage(&hourDisplay);
}

void menuOpenMainMenu()
{
    switchPage(&mainMenu);
}

void menuValidate()
{
    if (currentPage)
        currentPage->validate();
}

ulong butAFallTimestamp = 0;
ulong butBFallTimestamp = 0;
ulong lastActionStamp = millis();

int buttonMode = SIMPLE;

void buttonAFall()
{
    Serial.println("AFall");
    butAFallTimestamp = millis();
    if (!digitalRead(PIN_B))
        buttonMode = DOUBLE;
    else
        buttonMode = SIMPLE;
}

void buttonBFall()
{
    Serial.println("BFall");
    butBFallTimestamp = millis();
    if (!digitalRead(PIN_A))
        buttonMode = DOUBLE;
    else
        buttonMode = SIMPLE;
}

void buttonARise()
{
    //Serial.println("ARise");
    butAFallTimestamp = 0;
    if (buttonMode == DOUBLE && millis() - butAFallTimestamp > 2000 && millis() - butBFallTimestamp > 2000 && butBFallTimestamp)
        menuOpenMainMenu();
    else if (buttonMode == DOUBLE && butBFallTimestamp)
        menuReturn();
    else if (buttonMode == SIMPLE)
        menuValidate();
    else
        Serial.println("But A nothing to do");
}

void buttonBRise()
{
    //Serial.println("BRise");
    butBFallTimestamp = 0;
    if (buttonMode == DOUBLE && millis() - butAFallTimestamp > 2000 && millis() - butBFallTimestamp > 2000 && butAFallTimestamp)
        menuOpenMainMenu();
    else if (buttonMode == DOUBLE && butAFallTimestamp)
        menuReturn();
    else if (buttonMode == SIMPLE)
        menuNavigate();
    else
        Serial.println("But B nothing to do");
}

void butAInterrupt()
{
    lastActionStamp = millis();
    if (digitalRead(PIN_A))
        buttonARise();
    else
        buttonAFall();
}

void butBInterrupt()
{
    lastActionStamp = millis();
    if (digitalRead(PIN_B))
        buttonBRise();
    else
        buttonBFall();
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);

  attachInterrupt(PIN_A, &butAInterrupt, CHANGE);
  attachInterrupt(PIN_B, &butBInterrupt, CHANGE);

  pinMode(LIGHT_PIN, OUTPUT);

  mainMenu.setChildren(mainMenuChildren, 6);
  hourSetting.setCheckNavigate(&noMiddleSelection);
  alarmSetting.setCheckNavigate(&noMiddleSelection);
  alarmActivation.setBoolean(&alarmActivated);
  hourWebSynced.setBoolean(&isHourWebSynced);
  lightSetting.setInt(&light);

  tft.init();
  tft.setRotation(3); // Rotation de l'écran pour un affichage orienté (optionnel)

  sprite.createSprite(160, 128);

  setTime(19, 50, 0, 1, 1, 2025);

  //pistolServo.attach(22);
  pistolServo.begin();
  pistolServo.stop();

  loadBackground();

  currentPage = &mainMenu;

  Settingator::StartWiFi();
  //STR.SetCommunicator(WebSocketCTR::CreateSTAInstance("Livebox-83C380", "GTXZ7853"));
  STR.SetCommunicator(WebSocketCTR::CreateInstance());
  server = new HTTPServer(8080);

  STR.AddSetting(Setting::Type::Slider, &interpolateVar, sizeof(interpolateVar), "INTERPOLATE");
  STR.AddSetting(Setting::Type::Slider, &lightningNumber, sizeof(lightningNumber), "Nombre d'éclairs");
  STR.AddSetting(Setting::Type::Switch, &mode, sizeof(mode), "mode");
  //STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "SHOOT", &shoot);
  STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "LEFT", [](){ pistolServo.write(-0.05); });
  STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "RIGHT", [](){ pistolServo.write(0.05);});
  STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "STOP", [](){ pistolServo.stop();});

  STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "Navigate", [](){menuNavigate();});
  STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "Return", [](){menuReturn();});
  STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "Main Menu", [](){menuOpenMainMenu();});
  STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "Validate", [](){menuValidate();});

  randomSeed(analogRead(0));
}

void loop() {
  drawBackground();
  drawLightning(80, 64, 50, 100, STICK_CENTER);
  currentPage->display();

  sprite.pushSprite(0, 0);

  if (millis() - lastActionStamp < 5000 * 60)
    analogWrite(LIGHT_PIN, (light + 1) * 25);
  else
    analogWrite(LIGHT_PIN, 0);

  delay(5);  // Petit délai avant de redessiner
  interpolateVar++;
  STR.Update();
}
