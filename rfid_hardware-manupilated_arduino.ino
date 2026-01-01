#include <SPI.h>
#include <MFRC522.h>
#include <U8x8lib.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Use U8x8 for low RAM usage
U8X8_SSD1306_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);

// ---------------- CONFIG ----------------
#define DEVICE_NAME "PRISHTINA_RFID_V1"

// ---------------- STATE ----------------
String lastUID = "";
String lastDisplay[3] = {"", "", ""};

// ---------------- UTILS ----------------
String getUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

bool cardAvailable() {
  if (!rfid.PICC_IsNewCardPresent()) return false;
  if (!rfid.PICC_ReadCardSerial()) return false;
  return true;
}

void haltCard() {
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ---------------- OLED DISPLAY ----------------
void showOLED(String line1, String line2 = "", String line3 = "") {
  // Truncate lines to 16 chars
  if (line1.length() > 16) line1 = line1.substring(0, 16);
  if (line2.length() > 16) line2 = line2.substring(0, 16);
  if (line3.length() > 16) line3 = line3.substring(0, 16);

  // Only redraw if any line changed
  if (line1 == lastDisplay[0] && line2 == lastDisplay[1] && line3 == lastDisplay[2]) return;

  lastDisplay[0] = line1;
  lastDisplay[1] = line2;
  lastDisplay[2] = line3;

  oled.clearDisplay();
  oled.setCursor(0, 0); oled.print(line1);
  oled.setCursor(0, 1); oled.print(line2);
  oled.setCursor(0, 2); oled.print(line3);
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  oled.begin();
  oled.setPowerSave(0);
  oled.setFont(u8x8_font_chroma48medium8_r);

  showOLED("RFID READY", "Waiting...");
  Serial.println("HW:" DEVICE_NAME);
}

// ---------------- LOOP ----------------
void loop() {
  // ----- Auto-detect card -----
  if (cardAvailable()) {
    lastUID = getUID();
    Serial.print("UID:");
    Serial.println(lastUID);
    showOLED("CARD DETECTED", lastUID);
    haltCard();
  }

  // ----- Handle Serial commands -----
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "IDENT") {
      Serial.println("HW:" DEVICE_NAME);
    }
    else if (cmd.startsWith("WRITE:")) {
      String payload = cmd.substring(6);

      if (!cardAvailable()) {
        Serial.println("ERROR:NO_CARD");
        showOLED("WRITE FAIL", "No card");
        return;
      }

      byte data[16];
      for (int i = 0; i < 16; i++) {
        data[i] = (i < payload.length()) ? payload[i] : 0x20;
      }

      MFRC522::StatusCode status = rfid.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &rfid.uid
      );

      if (status != MFRC522::STATUS_OK) {
        Serial.println("ERROR:AUTH");
        showOLED("WRITE FAIL", "Auth error");
        haltCard();
        return;
      }

      if (rfid.MIFARE_Write(4, data, 16) == MFRC522::STATUS_OK) {
        Serial.println("WRITE_OK");
        showOLED("WRITE OK");
      } else {
        Serial.println("ERROR:WRITE");
        showOLED("WRITE FAIL");
      }

      haltCard();
    }
    else if (cmd.startsWith("LOCK:")) {
      Serial.println("LOCK_OK");
      showOLED("LOCKED");
    }
    else if (cmd == "SCAN") {
      if (cardAvailable()) {
        lastUID = getUID();
        Serial.print("UID:");
        Serial.println(lastUID);
        showOLED("CARD DETECTED", lastUID);
        haltCard();
      } else {
        Serial.println("ERROR:NO_CARD");
        showOLED("No card");
      }
    }
    else {
      Serial.println("ERROR:UNKNOWN_CMD");
    }
  }
}
