#include <MyI2CDisplay.h>

Adafruit_SSD1306 myi2cdisplay;
const uint8_t CMD_BRIGHTNESS = 0x81;

MyI2CDisplay::MyI2CDisplay(uint8_t resetPin) : myi2cdisplay(resetPin){
}

void MyI2CDisplay::begin(uint8_t i2caddr){
    myi2cdisplay.begin(SSD1306_SWITCHCAPVCC, i2caddr);
    myi2cdisplay.setTextColor(WHITE);
}

void MyI2CDisplay::setBrightness(uint8_t brightness){
    myi2cdisplay.ssd1306_command(CMD_BRIGHTNESS);
    myi2cdisplay.ssd1306_command(brightness);
}

void MyI2CDisplay::icon(State s, uint8_t track, uint8_t numberOfTracks){
    if(s == receiving){
        iconReceiving();
    }
    else if(s == playing){
        iconPlay(track, numberOfTracks);
    }
    else if(s == paused){
        iconPaused(track, numberOfTracks);
    }
    else if(s == locked){
        iconBlocked();
    }
}

uint8_t trackNoX = 60;
uint8_t trackNoY = 30;
uint8_t offset   = 5;
void drawTrackNumber(uint8_t track, uint8_t numberOfTracks){
    myi2cdisplay.setTextSize(5);
    // "SCHATTEN" UNTER TRACK-NUMMER
    myi2cdisplay.setTextColor(BLACK);
    for(uint8_t x=(trackNoX-offset); x<=(trackNoX+offset); x++){
        for(uint8_t y=(trackNoY-offset); y<=(trackNoY+offset); y++){
            myi2cdisplay.setCursor(x, y);
            myi2cdisplay.print(track, DEC);
        }
    }
    // TRACK-NUMMER
    myi2cdisplay.setTextColor(WHITE);
    myi2cdisplay.setCursor(trackNoX, trackNoY);
    myi2cdisplay.print(track, DEC);
}

void drawPlayIcon(){
    myi2cdisplay.fillTriangle(32, 0, 96, 31, 32, 63, WHITE);
}

void MyI2CDisplay::iconPlay(){
    myi2cdisplay.clearDisplay();
    drawPlayIcon();
    myi2cdisplay.display();
}

void MyI2CDisplay::iconPlay(uint8_t track, uint8_t numberOfTracks){
    myi2cdisplay.clearDisplay();
    drawPlayIcon();
    drawTrackNumber(track, numberOfTracks);
    myi2cdisplay.display();
}

void MyI2CDisplay::iconReceiving(){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.fillCircle(64, 32, 47, WHITE);
    myi2cdisplay.fillCircle(64, 32, 40, BLACK);
    myi2cdisplay.fillCircle(64, 32, 33, WHITE);
    myi2cdisplay.fillCircle(64, 32, 26, BLACK);
    myi2cdisplay.fillCircle(64, 32, 19, WHITE);
    myi2cdisplay.fillCircle(64, 32, 12, BLACK);
    myi2cdisplay.fillTriangle(64, 32, 96, 0, 32, 0, BLACK);
    myi2cdisplay.fillTriangle(64, 32, 96, 64, 32, 64, BLACK);
    myi2cdisplay.fillCircle(64, 32, 5, WHITE);

    // myi2cdisplay.fillCircle(64, 32, 63, WHITE);
    // myi2cdisplay.fillCircle(64, 32, 55, BLACK);
    // myi2cdisplay.fillCircle(64, 32, 47, WHITE);
    // myi2cdisplay.fillCircle(64, 32, 39, BLACK);
    // myi2cdisplay.fillCircle(64, 32, 31, WHITE);
    myi2cdisplay.display();
}

void drawPauseIcon(){
    myi2cdisplay.fillRect(32, 0, 25, 64, WHITE);
    myi2cdisplay.fillRect(71, 0, 25, 64, WHITE);
}

void MyI2CDisplay::iconPaused(){
    myi2cdisplay.clearDisplay();
    drawPauseIcon();
    myi2cdisplay.display();
}

void MyI2CDisplay::iconPaused(uint8_t track, uint8_t numberOfTracks){
    myi2cdisplay.clearDisplay();
    drawPauseIcon();
    drawTrackNumber(track, numberOfTracks);
    myi2cdisplay.display();
}

void MyI2CDisplay::iconBlocked(){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.fillCircle(64, 18, 18, WHITE);
    myi2cdisplay.fillCircle(64, 18, 10, BLACK);
    myi2cdisplay.fillRoundRect(37, 18, 54, 46, 4, WHITE);
    myi2cdisplay.fillCircle(64, 33, 6, BLACK);
    myi2cdisplay.fillTriangle(64, 27, 69, 53, 59, 53, BLACK);
    myi2cdisplay.display();
}

void MyI2CDisplay::iconError(uint8_t errorCode){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.setTextSize(8);
    myi2cdisplay.setCursor(10, 0);
    myi2cdisplay.print("E");
    myi2cdisplay.setCursor(50, 0);
    myi2cdisplay.print(errorCode, DEC);
    myi2cdisplay.display();
}

void MyI2CDisplay::iconAddCard(uint8_t index){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.setTextSize(8);
    myi2cdisplay.setCursor(0, 0);
    myi2cdisplay.print("+");
    if(index > 0){
        myi2cdisplay.setCursor(50, 0);
        myi2cdisplay.print(index, DEC);
    }
    myi2cdisplay.display();
}

void MyI2CDisplay::iconRemoveCard(){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.setTextSize(8);
    myi2cdisplay.setCursor(0, 0);
    myi2cdisplay.print("-");
    myi2cdisplay.display();
}

void MyI2CDisplay::iconUnknown(){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.fillRect(31, 0, 64, 64, WHITE);
    myi2cdisplay.fillTriangle(51,  0, 75,  0, 63, 23, BLACK);
    myi2cdisplay.fillTriangle(51, 63, 75, 63, 63, 40, BLACK);
    myi2cdisplay.fillTriangle(31,  0, 46, 31, 31, 63, BLACK);
    myi2cdisplay.fillTriangle(95,  0, 80, 31, 95, 63, BLACK);
    myi2cdisplay.display();
}

void MyI2CDisplay::iconSuccess(){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.fillTriangle(53, 43, 53, 63, 38, 33, WHITE);
    myi2cdisplay.fillTriangle(53, 43, 53, 63, 98,  3, WHITE);
    myi2cdisplay.display();
}

void MyI2CDisplay::showVolumeLevel(uint32_t value, uint32_t maxValue){
    myi2cdisplay.clearDisplay();
    myi2cdisplay.drawRect( 0, 23, 128, 15, WHITE);
    myi2cdisplay.fillRect( 0, 23, (128*value)/maxValue, 15, WHITE);
    myi2cdisplay.display();
}
