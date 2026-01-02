# RFID Hardware Emulator – Arduino Nano

## Overview

This project turns an **Arduino Nano** into a fully functional **RFID reader/writer device** that behaves like a real commercial RFID hardware unit.

Instead of acting like a simple Arduino sketch, the firmware exposes a **strict serial protocol** that allows desktop or web applications to communicate with it as if it were professional RFID hardware.

The system was designed for access control, check‑in / check‑out systems, and secure identity workflows.

---

## Core Concept

The Arduino:

* Listens for commands over **USB Serial**
* Executes RFID operations deterministically
* Responds with strict, machine‑readable replies
* Controls all RFID logic internally
* Provides real‑time visual feedback on a **0.96" OLED display**

The computer:

* Sends commands
* Waits for specific responses
* Never directly controls RFID timing or logic

This mirrors how professional RFID readers operate.

---

## Hardware Used

* Arduino Nano (ATmega328P)
* MFRC522 RFID reader
* 0.96" OLED (I2C, 128x64)
* MIFARE Classic cards

---

## Communication Protocol

All communication happens over **Serial (USB)**.

* Baud Rate: **9600**
* Line ending: `\n`
* Commands are case‑sensitive

---

## Supported Commands

### 1. Identify Hardware

**Command**

```
IDENT
```

**Response**

```
HW:PRISHTINA_RFID_V1
```

Used by the software to confirm the connected device is valid.

---

### 2. Scan for Card

**Command**

```
SCAN
```

**Response**

```
UID:XXXXXXXX
```

Reads the UID of the card currently on the reader.

---

### 3. Write Data (Assignment Mode)

**Command**

```
WRITE:<jwt_payload>
```

**Response**

```
WRITE_OK
```

Writes encrypted data into the RFID card (Sector 1, Block 4).

---

### 4. Lock Card

**Command**

```
LOCK:<secret>
```

**Response**

```
LOCK_OK
```

Locks the card by modifying sector trailer access bits, preventing further unauthorized writes.

---

### 5. Unlock Card

**Command**

```
UNLOCK:<secret>
```

**Response**

```
UNLOCK_OK
```

Authenticates using the protected key and re-enables modification access.

---

### 6. Wipe Card

**Command**

```
WIPE
```

**Response**

```
WIPE_OK
```

Clears stored data from the target sector.

---

## RFID Memory Layout

MIFARE Classic 1K layout:

* Sector 1 used for data
* Block 4–6 → Data
* Block 7 → Sector trailer (keys + permissions)

This ensures safe separation between identity data and security controls.

---

## Security Model

* Default Key A is used for authentication
* Custom Key B is written during LOCK
* After locking, write access is denied unless UNLOCK is performed

This simulates professional RFID access control systems.

---

## OLED Display Behavior

The OLED reflects internal device state:

* `RFID READY` – idle
* `CARD DETECTED` – card present
* `WRITE OK` – data written
* `LOCKED` – card locked
* `ERROR` – operation failed

This allows debugging without a computer.

---

## Why This Exists

This project was created to prototype and test an access‑control system without owning commercial RFID hardware. The Arduino is transformed into a deterministic device that behaves exactly like real readers used in production systems.

The design prioritizes:

* Stability
* Predictable behavior
* Clear communication protocol
* Low memory usage (fits in 2KB SRAM)

---

## Notes

* Designed for Arduino Nano / ATmega328P
* No dynamic memory allocation
* Safe for long‑running sessions
* Suitable for real integrations

---

## Summary

This project turns a simple Arduino into a full RFID hardware emulator with defined protocol, state control, and security behavior — suitable for real-world integration and testing.

---

If you are extending this:

* Add CRC or checksum validation
* Add multi-block writes
* Add challenge–response authentication
* Add UID whitelisting

---

End of documentation.
