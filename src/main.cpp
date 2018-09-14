#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <MyI2CDisplay.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include "main.h"

// IDEE: UIDs werden in EEPROM abgelegt. Die Position im EEPROM entspricht der Ordnernummer
//       und es gibt einen Modus, um weitere Karten hinzu zu fuegen. Z.B. ueber langes
//       Vorhalten der Masterkarte(n)

// Konstanten
const MyUid C_NOUID = MyUid{0xff, 0xff, 0xff, 0xff}; // EEPROM ist im Standard mit 1en gefuellt
const MusicCard C_NOMUSICCARD = {C_NOUID, 0xff};
const MyUid C_MASTERUIDS[2] = {MyUid{{0xde, 0xbc, 0x80, 0xb9}}, MyUid{{0x75, 0x10, 0x80, 0xb9}}}; // Masterkarten-UIDs
const unsigned long C_HOLDTOADD = 3000;
const uint16_t C_UIDLENGTH      = 4;          // Laenge der UIDs in Byte
const uint16_t C_MAXCARDCOUNT   = 20;        // Max. Anzahl UIDs
const uint16_t C_METAINFLENGTH  = 1;        // Laenge der Zusatzinfos in Byte
MusicCard musicCards[C_MAXCARDCOUNT]; // UID-Array
const unsigned long C_TIMEOUT     = 5000; // 5 Sekunden
const unsigned long C_SHORTTIME = 2000;
const unsigned long C_PLAYERCOMMANDDELAY = 50; // 50 ms zwischen nach jedem Player-Commando warten
const uint16_t C_MAXVOLUME = 20; // Lautstärke auf 20 (statt 30) limitieren
const uint32_t C_MAXVOLUMEPOTIVALUE = 1023;
const uint8_t  C_MINPOTIDIFF = 10;
const uint16_t C_EEPROMOFFSET = 0;

// Lautstaerke-Poti
#define VOLUME_POTI_PIN 18 // Pin 18 / A0
// RFID
#define RFID_SS_PIN 10
#define RFID_RST_PIN 9
// DISPLAY
#define DIS_RST_PIN 4
#define DIS_ADRESS 0x3C
// MP3-MODUL
#define PLAYER_RX_PIN 8
#define PLAYER_TX_PIN 9
#define PLAYER_BUSY_PIN 7


MyI2CDisplay display(DIS_RST_PIN);
MyCardReader rfid(RFID_SS_PIN, RFID_RST_PIN);
SoftwareSerial softSerial(PLAYER_RX_PIN, PLAYER_TX_PIN); // RX, TX
DFRobotDFPlayerMini player;

// === PROGRAMMABLAUF ===
/**
* Initialisiert Geräte und ließt bekannte UIDs aus dem EEPROM aus.
*/
uint32_t lastVolumePotiValue = 0;
void setup() {
    // Display-Setup
    display.begin(DIS_ADRESS);
    // Player-Setup
    softSerial.begin(9600);
    if(!player.begin(softSerial)){
        display.iconError(0);
        while(true){
            // Nicht tun, wenn MP3-Modul nicht gestartet wurde
        }
    }
    pinMode(PLAYER_BUSY_PIN, INPUT);
    // RFID-Leser initialisieren
    rfid.init();
    // UIDs AUS EEPROM LESEN
    readUIDs(C_EEPROMOFFSET);
    // Aktuelle Lautstaerke lesen
    setVolume();
    // Equalizer setzen
    player.EQ(DFPLAYER_EQ_ROCK);
    delay(C_PLAYERCOMMANDDELAY);
    // Empfangssymbol setzen
    display.iconReceiving();
}

/**
* Eigentlicher Programmablauf
*/
MyUid u  = {{0x0,0x0,0x0,0x0}};
MyUid lastUid = {{0x1,0x1,0x1,0x1}};
MyUid lastMusicUid;
unsigned long firstMasterTime;
bool sameAsLastTime = false;
bool masterCard = false;
State state = receiving;
bool noCard = true;
uint32_t newVolumePotiValue = 0;
State preLockState = receiving;
bool deleteCard = false;
bool addNewCard = false;
bool tempIconSet = false;
unsigned long tempIconTime;
uint16_t curFolderNo;
uint16_t curFolderSize;
uint16_t curSong;
void loop() {
    // Lautstaerke lesen / setzen
    setVolume();

    // UID lesen
    u = rfid.readUid();

    // Auswerten
    sameAsLastTime = uidsAreEqual(u, lastUid);
    masterCard = isMasterCard(u);
    noCard = uidsAreEqual(u, rfid.NullUid);

    // Keine Karte: Ggf. naechstes Lied spielen
    if(noCard){
        if(state == playing){
            if(playerIsPlaying()){
                // Nichts tun, solang Wiedergabe des akt. Liedes nicht beendet
            }
            else{
                // Wenn noch nicht alle Lieder gespielt, naechstes spielen
                if(curSong<curFolderSize){
                    curSong++;
                    player.playFolder(curFolderNo, curSong);
                    delay(C_PLAYERCOMMANDDELAY);
                }
                // Wieder auf Empfangsmodus
                else{
                    state = receiving;
                }
            }
        }
    }
    else if(sameAsLastTime){
        // Karten hinzu / entfernen
        if(masterCard){
            while(isMasterCard(rfid.readUid())){
                // Modus "Karte entfernen"
                if(millis()-firstMasterTime>2*C_SHORTTIME){
                    display.iconRemoveCard();
                    deleteCard = true;
                    addNewCard = false;
                }
                // Modus "Karte hinzu"
                else if(millis()-firstMasterTime>C_SHORTTIME){
                    display.iconAddCard(0);
                    addNewCard = true;
                    deleteCard = false;
                }
            }

            if(deleteCard || addNewCard){
                if(deleteCard){
                    removeCard();
                    deleteCard = false;
                }
                else{
                    addCard();
                    addNewCard = false;
                }

                // Ggf. Status zurueck setzen
                if(state == locked ){
                    state = preLockState;
                    // Ggf. Wiedergabe wieder starten
                    if(state == playing){
                        player.start();
                        delay(C_PLAYERCOMMANDDELAY);
                    }
                }
            }
        }
    }
    else{
        // Wechseln zwischen gesperrt und entsperrt
        if(masterCard){
            firstMasterTime = millis();
            if(state != locked) {
                if(state == playing){
                    player.pause();
                    delay(C_PLAYERCOMMANDDELAY);
                }
                preLockState = state;
                state = locked;
            }
            else{
                state = preLockState;
                if(state == playing){
                    player.start();
                    delay(C_PLAYERCOMMANDDELAY);
                }
            }
        }
        else if (state != locked){
            // Unbekannte Karte
            if(uidIndex(u)<0){
                display.iconUnknown();
                tempIconSet = true;
                tempIconTime = millis();
            }
            // Bekannte Karte -> Musik spielen / pausieren
            else{
                if(!uidsAreEqual(u, lastMusicUid) || state==receiving){
                    lastMusicUid = u;
                    curFolderNo = getFolderNo(u);
                    curFolderSize = player.readFileCountsInFolder(curFolderNo);
                    if(curFolderSize>0){
                        curSong = 1;
                        player.playFolder(curFolderNo, curSong);
                        delay(C_PLAYERCOMMANDDELAY);
                        state = playing;
                    }
                    else{
                        state = receiving;
                    }
                }
                else if(uidsAreEqual(u, lastMusicUid)){
                    // Wenn Musik spielt, pausieren
                    if(state == playing){
                        player.pause();
                        delay(C_PLAYERCOMMANDDELAY);
                        state = paused;
                    }
                    else if(state == paused){
                        player.start();
                        delay(C_PLAYERCOMMANDDELAY);
                        state = playing;
                    }
                }
            }
        }
    }

    // nach x Sekunden Boolean zurueck setzen, damit wieder normaler Status angezeigt wird
    if(tempIconSet && millis()-tempIconTime > C_SHORTTIME){
        tempIconSet = false;
    }

    // Aktuellen Status auf Display anzeigen
    if(!tempIconSet){
        display.icon(state, curSong, 0);
    }

    // UID fuer naechsten Durchgang merken
    lastUid = u;
}

uint8_t freeIndex;
MyUid addUid;
void addCard(){
    // Freien Platz im EEPROM / Array suchen
    freeIndex=0;
    for(uint8_t i=0; i<sizeof(musicCards)/sizeof(musicCards[0]); i++){
        if(uidsAreEqual(musicCards[i].uid, C_NOUID)){
            freeIndex = i;
            break;
        }
    }
    // Wenn freier Platz gefunden, Karte lesen und UID speichern
    display.iconAddCard(freeIndex+1);
    // Sicherstellen, dass Masterkarte kurz weggenommen wurde
    while(!uidsAreEqual(rfid.readUid(), rfid.NullUid)){
        // Nichts tun
    }
    addUid = rfid.readUid();
    // Solange keine Karte vorgehalten wird immer wieder versuchen
    while(uidsAreEqual(addUid, rfid.NullUid)){
        addUid = rfid.readUid();
    }
    // Pruefen, ob Karte schon bekannt oder Masterkarte. Wenn ja, abbrechen
    if(!isMasterCard(addUid) && !getFolderNo(addUid)>0){
        // TODO: Metadaten abfragen
        // Karte speichern
        saveMusiccard(C_EEPROMOFFSET, freeIndex, MusicCard{addUid, 0x00});
        // Daten neu aus EEPROM lesen
        readUIDs(C_EEPROMOFFSET);
        // Icon setzen
        display.iconSuccess();
        tempIconSet = true;
        tempIconTime = millis();
    }

    // Nichts tun, bis Karte entfernt wird
    while(!uidsAreEqual(rfid.readUid(), rfid.NullUid)){

    }
}

MyUid toBeRemoved;
void removeCard(){
    toBeRemoved = rfid.readUid();
    // Nichts tun, bis Karte vorgehalten wird
    while(uidsAreEqual(toBeRemoved, rfid.NullUid)){
        toBeRemoved = rfid.readUid();
    }
    // Karte entfernen, wenn es sich um eine Musikkarte handelt
    int index = uidIndex(toBeRemoved);
    if(index>=0){
        // Index mit 0xff Bytes ueberschreiben
        saveMusiccard(C_EEPROMOFFSET, index, C_NOMUSICCARD);
        // Daten neu aus EEPROM lesen
        readUIDs(C_EEPROMOFFSET);
        // Icon setzen
        display.iconSuccess();
        tempIconSet = true;
        tempIconTime = millis();
    }
    // Nichts tun, bis Karte entfernt wird
    while(!uidsAreEqual(rfid.readUid(), rfid.NullUid)){

    }
}

/**
* Handelt es sich um eine der Master-Karten?
*/
bool isMasterCard(MyUid u){
    for(uint8_t i=0; i<sizeof(C_MASTERUIDS)/sizeof(C_MASTERUIDS[0]); ++i){
        if(uidsAreEqual(u, C_MASTERUIDS[i])){
            return true;
        }
    }

    return false;
}

bool uidsAreEqual(MyUid uid1, MyUid uid2){
    if(sizeof(uid1.values) != sizeof(uid2.values)){
        return false;
    }

    for(uint8_t i=0; i<sizeof(uid1.values); i++){
        if(uid1.values[i] != uid2.values[i]){
            return false;
        }
    }

    return true;
}

/**
* Ermittelt den index der UID, sofern in UIDSARRAY vorhanden
*/
int uidIndex(MyUid u){
    // Nach passender UID suchen
    for(uint8_t i=0; i<C_MAXCARDCOUNT; i++){
        if(uidsAreEqual(u, musicCards[i].uid)){
            return i;
        }
    }

    return -1;
}


/*
* Ermittelt den zur UID gehoerenden Ordner (Array-Index der UID + 1)
*/
uint8_t getFolderNo(MyUid u){
    return uidIndex(u)+1;
}

/**
* Karten-UIDs neu aus EEPROM auslesen
*/
void readUIDs(uint16_t eepromOffset){
    for(uint16_t i=0; i<C_MAXCARDCOUNT; i++){
        for(uint16_t j=0; j<C_UIDLENGTH+C_METAINFLENGTH; j++){
            // UID auslesen
            if(j<C_UIDLENGTH){
                musicCards[i].uid.values[j] = EEPROM.read(eepromOffset+i*(C_UIDLENGTH+C_METAINFLENGTH)+j);
            }
            // Metadaten auslesen
            else if(j<C_UIDLENGTH+C_METAINFLENGTH){
                musicCards[i].meta = EEPROM.read(eepromOffset+i*(C_UIDLENGTH+C_METAINFLENGTH)+j);
            }
        }
    }
}

/**
* Karten-UID in EEPROM speichern
*/
void saveMusiccard(uint16_t eepromOffset, uint8_t index, MusicCard card){
    // UID speichern
    for(int i=0; i<sizeof(card.uid.values); i++){
        EEPROM.write(eepromOffset+index*(C_UIDLENGTH+C_METAINFLENGTH)+i, card.uid.values[i]);
    }
    // Metainformationen speichern
    EEPROM.write(eepromOffset+index*(C_UIDLENGTH+C_METAINFLENGTH)+C_UIDLENGTH, card.meta);
}

uint16_t twoTimesNewVolume = 0;

void setVolume(){
    newVolumePotiValue = analogRead(VOLUME_POTI_PIN);
    if((newVolumePotiValue  > lastVolumePotiValue && newVolumePotiValue-lastVolumePotiValue > C_MINPOTIDIFF) ||
       (lastVolumePotiValue > newVolumePotiValue  && lastVolumePotiValue-newVolumePotiValue > C_MINPOTIDIFF)){
        lastVolumePotiValue = newVolumePotiValue;
        display.showVolumeLevel(newVolumePotiValue, C_MAXVOLUMEPOTIVALUE);
        tempIconTime = millis();
        tempIconSet = true;
        player.volume((C_MAXVOLUME*lastVolumePotiValue)/C_MAXVOLUMEPOTIVALUE);
        delay(C_PLAYERCOMMANDDELAY);
    }
}

bool playerIsPlaying(){
    return digitalRead(PLAYER_BUSY_PIN) == LOW;
}
