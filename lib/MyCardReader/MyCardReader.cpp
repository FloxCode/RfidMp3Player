#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <MyCardReader.h>

MFRC522 mfrc;

MyCardReader::MyCardReader(byte chipSelectPin, byte resetPowerDownPin) : mfrc(chipSelectPin, resetPowerDownPin){
}

void MyCardReader::init(){
    SPI.begin();
    mfrc.PCD_Init();
}

MyUid MyCardReader::readUid(){
    // Karte in Reichweite?
    // WORKAROUND: Doppelte Abfrage, weil sonst eine permanent vorgehaltene Karten
    // Abwechselnd einmal erkannt und einmal nicht erkannt wird...
    if (!mfrc.PICC_IsNewCardPresent() && !mfrc.PICC_IsNewCardPresent()) {
      return NullUid;
    }
    // Karte(n) lesbar?
    if (!mfrc.PICC_ReadCardSerial()) {
        return NullUid;
    }

    // ID auslesen
    MyUid u = NullUid;
    for (uint8_t i = 0; i < mfrc.uid.size; i++) {
        if(i<=sizeof(u.values)){
            u.values[i] = mfrc.uid.uidByte[i];
        }
    }
    return u;
}
