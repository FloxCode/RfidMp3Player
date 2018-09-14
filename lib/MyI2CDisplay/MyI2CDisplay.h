#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum State{
           receiving,
           playing,
           paused,
           locked
       };

class MyI2CDisplay{
public:
    MyI2CDisplay(uint8_t resetPin);
    void begin(uint8_t i2caddr);
    void setBrightness(uint8_t brightness);
    void icon(State s, uint8_t track, uint8_t numberOfTracks);
    void iconPlay();
    void iconPlay(uint8_t track, uint8_t numberOfTracks);
    void iconReceiving();
    void iconPaused();
    void iconPaused(uint8_t track, uint8_t numberOfTracks);
    void iconBlocked();
    void iconError(uint8_t errorCode);
    void iconAddCard(uint8_t index);
    void iconRemoveCard();
    void iconUnknown();
    void iconSuccess();
    void showVolumeLevel(uint32_t value, uint32_t maxValue);
private:
    Adafruit_SSD1306 myi2cdisplay;
};
