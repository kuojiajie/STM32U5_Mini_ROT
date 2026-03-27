# 🛡️ STM32U5 Mini-ROT (Hardware Root of Trust)

## 📖 Project Overview

This project implements a bare-metal **Root of Trust (RoT)** on an STM32U5 microcontroller to control the secure boot process of a target system (**Raspberry Pi 4B**).
The STM32 acts as a hardware gatekeeper. It physically holds the Pi in **RESET** state upon power-up, reads firmware directly from Flash memory, calculates its SHA-256 hash, and verifies it against an RSA-2048 signature.

**✨ Advanced Features:**
It features an **A/B Dual Image Auto-Recovery** mechanism with **Self-Healing** capabilities, and utilizes **Dynamic Image Headers** to parse firmware metadata securely, effectively mitigating payload-appending attacks.

## 🛠️ Hardware Setup

### Connection Interface

**⚠️ Critical:** A low-impedance **Common Ground** is required. It is highly recommended to connect the USB metal shells of both devices together.

| STM32 Pin | Raspberry Pi Pin | Signal Name | Description |
| :--- | :--- | :--- | :--- |
| **PF15** (CN10-12) | **RUN** (J2) | `PI_RESET` | Active-Low Reset Control (Open Drain) |
| **GND** | **GND** | Common GND | **Must connect** to eliminate voltage potential diff |

### Power Supply

- **STM32**: Powered via ST-LINK USB
- **Raspberry Pi**: Independently powered via USB-C

## 💻 Technical Implementation

This project deviates from standard STM32CubeMX generation to meet strict security requirements.

### 1. Early Initialization Strategy (`main.c`)

To mitigate race conditions where the Pi boots faster than the STM32, the **GPIO Initialization** is moved **before** the System Clock Configuration.

### 2. Hardware Control Logic (`.ioc`)

- **PF15** is configured as `Output Open Drain`. This allows safe control of the Pi's Reset logic.
- **RNG** (Random Number Generator) is enabled for cryptographic operations.

### 3. Dynamic Image Header (Anti-TOCTOU)

Firmware payloads are encapsulated with a custom 8-byte header containing a Magic Word (`MROT`) and a dynamic payload length. The RoT uses `C struct casting` to parse this header and define the exact cryptographic boundary, mitigating Time-of-Check to Time-of-Use (TOCTOU) vulnerabilities. *(See `crypto_data/pack_tool.py`)*

### 4. Verification Logic

- **Real-time Hashing**: Reads firmware data dynamically based on the parsed header length and calculates SHA-256.
- **Signature Verification**: Uses **mbedTLS** to verify the hash against a hardcoded RSA-2048 signature.

### 5. A/B Dual Image Auto-Recovery & Self-Healing

The internal flash is partitioned into **Slot A** (`0x08100000`) and **Slot B** (`0x08180000`). If Slot A fails cryptographic validation, the RoT falls back to Slot B.

Upon successful boot from Slot B, it executes a background synchronization task utilizing STM32 HAL Flash APIs to erase the corrupted Slot A and reprogram it (Quad-Word aligned) using verified data, fully restoring dual-slot redundancy.

## 🚀 Getting Started

### Prerequisites

- **IDE**: STM32CubeIDE (v1.16.0+)
- **Hardware**: STM32 NUCLEO-U575ZI-Q + Raspberry Pi 4B

### Build & Run

1. **Clone**: `git clone <repo_url>`
2. **Import**: Open STM32CubeIDE -> `File` -> `Import` -> `Existing Projects into Workspace`
3. **Build**: Click the **Hammer** icon.
4. **Flash Firmware Data (Crucial Step)**:
   - Go to `Run -> Run Configurations -> Startup -> Load Image and Symbols`.
   - Add `crypto_data/good_fw.bin` at address **`0x08100000`** (Slot A).
   - Add `crypto_data/good_fw.bin` at address **`0x08180000`** (Slot B).
5. **Run**: Connect STM32 via USB and click **Debug/Run**.

## 🧪 Verification & Chaos Testing

Open a Serial Terminal (115200, 8N1) and observe the STM32's behavior.

### Test 1: Normal Boot

With valid `packaged_firmware.bin` in both slots:
- Log: `[ROT] MROT Header Found! Payload Length: 5 bytes.`
- Log: `[ROT] SUCCESS: Slot A is VALID.`
- Log: `[ROT] Releasing Pi Gate.` -> **Pi Green LED starts blinking**

### Test 2: Auto-Recovery & Anti-Tamper (Chaos Test)

1. Go to Run Configurations and change the Slot A image to `crypto_data/bad_fw.bin` (which contains appended malicious payload).
2. Run the Debugger again.
3. **Observe Failsafe & Healing:**
   - Log: `[ROT] MROT Header Found! Payload Length: 12 bytes.` *(Detects appended data)*
   - Log: `FAIL! (Error: -0xXXXX)` *(Signature rejects modified payload)*
   - Log: `[ROT] WARNING: Slot A Corrupted or Tampered!`
   - Log: `[ROT] >>> Attempting to Boot from Slot B (Backup) <<<`
   - Log: `[SYNC] Erasing Slot A... Programming Slot A...`
   - Log: `[ROT] System Recovered! Releasing Pi Gate.` -> **Pi Boots Successfully**
4. **The Magic Reboot**: Press the physical RESET button on the STM32. You will observe that it boots from Slot A instantly, proving the flash was successfully rewritten and healed!
