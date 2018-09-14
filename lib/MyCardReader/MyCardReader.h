#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// UID
typedef struct {
    byte values[4];
} MyUid;

class MyCardReader {
public:
    MyCardReader(byte chipSelectPin, byte resetPowerDownPin);
    void   init();
    MyUid NullUid{{0x0,0x0,0x0,0x0}};
    MyUid readUid();
private:
    MFRC522 mfrc;
};
