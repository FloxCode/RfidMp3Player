#include <Arduino.h>
#include <MyCardReader.h>

struct MusicCard{MyUid uid; byte meta;};

bool isMasterCard(MyUid u);
uint8_t getFolderNo(MyUid u);
void readUIDs(uint16_t eepromOffset);
int uidIndex(MyUid u);
bool uidsAreEqual(MyUid uid1, MyUid uid2);
void setVolume();
void addCard();
void removeCard();
void saveMusiccard(uint16_t eepromOffset, uint8_t index, MusicCard card);
bool playerIsPlaying();
