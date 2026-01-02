#include <SPI.h>
#include <MFRC522.h>
#include <U8x8lib.h>

#define SS_PIN 10
#define RST_PIN 9

#define DEVICE_NAME "PRISHTINA_RFID_V1"
#define BAUD_RATE 9600

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key keyA;
MFRC522::MIFARE_Key keyB;

U8X8_SSD1306_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);

String lastUID = "";
bool cardPresent = false;

// ================= OLED =================
void show(String l1, String l2 = "", String l3 = "") {
  oled.clearDisplay();
  oled.setCursor(0, 0); oled.print(l1);
  oled.setCursor(0, 1); oled.print(l2);
  oled.setCursor(0, 2); oled.print(l3);
}

// ================= CARD UTILS =================
bool waitForCard() {
  if (!rfid.PICC_IsNewCardPresent()) return false;
  if (!rfid.PICC_ReadCardSerial()) return false;
  return true;
}

String readUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

void haltCard() {
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ================= RFID WRITE =================
bool writeBlock(byte block, byte* data) {
  MFRC522::StatusCode status;

  status = rfid.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,
    block,
    &keyA,
    &rfid.uid
  );

  if (status != MFRC522::STATUS_OK) return false;

  status = rfid.MIFARE_Write(block, data, 16);
  return status == MFRC522::STATUS_OK;
}

// ================= SETUP =================
void setup() {
  Serial.begin(BAUD_RATE);
  SPI.begin();
  rfid.PCD_Init();

  oled.begin();
  oled.setPowerSave(0);
  oled.setFont(u8x8_font_chroma48medium8_r);

  for (byte i = 0; i < 6; i++) {
    keyA.keyByte[i] = 0xFF;   // default factory key
    keyB.keyByte[i] = 0xA0;   // custom lock key
  }

  show("RFID READY", "Waiting...");
  Serial.println("HW:" DEVICE_NAME);
}

// ================= MAIN LOOP =================
void loop() {

  // --------- SERIAL COMMANDS ----------
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    // IDENT
    if (cmd == "IDENT") {
      Serial.println("HW:" DEVICE_NAME);
      return;
    }

    // SCAN
    if (cmd == "SCAN") {
      show("SCAN", "Waiting card");
      if (!waitForCard()) {
        Serial.println("ERROR:NO_CARD");
        return;
      }

      lastUID = readUID();
      Serial.print("UID:");
      Serial.println(lastUID);
      show("CARD OK", lastUID);
      haltCard();
      return;
    }

    // WRITE
    if (cmd.startsWith("WRITE:")) {
      String payload = cmd.substring(6);

      if (!waitForCard()) {
        Serial.println("ERROR:NO_CARD");
        return;
      }

      byte buffer[16];
      memset(buffer, 0x20, 16);
      for (int i = 0; i < payload.length() && i < 16; i++) {
        buffer[i] = payload[i];
      }

      if (!writeBlock(4, buffer)) {
        Serial.println("ERROR:WRITE");
        haltCard();
        return;
      }

      Serial.println("WRITE_OK");
      show("WRITE OK");
      haltCard();
      return;
    }

    // LOCK
    if (cmd.startsWith("LOCK:")) {
      if (!waitForCard()) {
        Serial.println("ERROR:NO_CARD");
        return;
      }

      // Sector trailer block = 7
      byte trailer[16] = {
        0xA0,0xA0,0xA0,0xA0,0xA0,0xA0,
        0xFF,0x07,0x80,0x69,
        0xA0,0xA0,0xA0,0xA0,0xA0,0xA0
      };

      if (writeBlock(7, trailer)) {
        Serial.println("LOCK_OK");
        show("LOCKED");
      } else {
        Serial.println("ERROR:LOCK");
      }

      haltCard();
      return;
    }

    // WIPE
    if (cmd == "WIPE") {
      if (!waitForCard()) {
        Serial.println("ERROR:NO_CARD");
        return;
      }

      byte empty[16];
      memset(empty, 0x00, 16);
      writeBlock(4, empty);

      Serial.println("WIPE_OK");
      show("WIPE OK");
      haltCard();
      return;
    }

    Serial.println("ERROR:UNKNOWN_CMD");
  }
}
