# 🛡️ STM32U5 Mini-ROT (Hardware Root of Trust)

## 📖 Project Overview

This project implements a bare-metal **Root of Trust (RoT)** on an STM32U5 microcontroller to control the secure boot process of a target system (**Raspberry Pi 4B**).
The STM32 acts as a hardware gatekeeper. It physically holds the Pi in **RESET** state upon power-up, reads firmware directly from Flash memory, calculates its SHA-256 hash, and verifies it against an RSA-2048 signature.

**✨ Advanced Features:**
It features an **A/B Dual Image Auto-Recovery** mechanism with **Self-Healing** capabilities, utilizes **Dynamic Image Headers** to mitigate TOCTOU attacks, and implements a hardware-backed **Anti-Rollback (ARB)** mechanism to prevent the execution of older, vulnerable firmware versions even if cryptographically signed.

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

Firmware payloads are encapsulated with a custom 12-byte header containing a Magic Word (`MROT`), Security Version Number (SVN), and payload length. This defines the exact cryptographic boundary. *(See `crypto_data/pack_tool.py`)*

### 4. Verification Logic

- **Real-time Hashing**: Reads firmware data dynamically based on parsed header length and calculates SHA-256.
- **Signature Verification**: Uses **mbedTLS** to verify the hash against a hardcoded RSA-2048 signature.

### 5. A/B Dual Image Auto-Recovery & Self-Healing

The internal flash is partitioned into **Slot A** (`0x08100000`) and **Slot B** (`0x08180000`). If Slot A fails validation, the RoT falls back to Slot B. Upon successful boot, it utilizes STM32 HAL Flash APIs to erase the corrupted Slot A and reprogram it using verified data from Slot B.

### 6. Anti-Rollback Protection (Security Version Number)

To defend against downgrade attacks, the RoT permanently writes the highest successfully booted SVN to a dedicated non-volatile Flash sector (`0x081F0000`), simulating eFuse/OTP behavior. During subsequent boots, any firmware with an SVN lower than the stored value is strictly rejected.

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
   - Add `crypto_data/fw_v2.bin` at address **`0x08100000`** (Slot A).
   - Add `crypto_data/fw_v2.bin` at address **`0x08180000`** (Slot B).
5. **Run**: Connect STM32 via USB and click **Debug/Run**.

## 🧪 Verification & Chaos Testing

Open a Serial Terminal (115200, 8N1) and observe the STM32's behavior.

### Test 1: Normal Boot (v2)

With valid `fw_v2.bin` in both slots:
- Log: `[ROT] Firmware Version: v2 (Min Allowed: vX)`
- Log: `[ROT] SUCCESS: Slot A is VALID.`
- Log: `[ROT] Releasing Pi Gate.` -> **Pi Green LED starts blinking**

### Test 2: Auto-Recovery & Anti-Tamper (Chaos Test)

Change the Slot A image to `crypto_data/bad_fw.bin` (appended malicious payload).
- Log: `[ROT] MROT Header Found! Payload Length: 12 bytes.` *(Detects appended data)*
- Log: `FAIL! (Error: -0xXXXX)` *(Signature rejects modified payload)*
- Log: `[ROT] WARNING: Slot A Corrupted or Tampered!`
- Log: `[ROT] >>> Attempting to Boot from Slot B (Backup) <<<`
- Log: `[SYNC] Erasing Slot A... Programming Slot A...`
- Log: `[ROT] System Recovered! Releasing Pi Gate.`

### Test 3: Anti-Rollback (Downgrade Attack Prevention)

Change both Slot A and Slot B images to older, but legitimately signed `crypto_data/fw_v1.bin`.
- Log: `[ROT] Firmware Version: v1 (Min Allowed: v2)`
- Log: `[ROT] SECURITY ALERT: Anti-Rollback triggered! Downgrade blocked.`
- Log: `[ROT] FATAL ERROR: Both Slot A and Slot B are Corrupted or Downgraded!`
- **Result**: The system refuses to boot and locks down, successfully mitigating the rollback exploit.

## ✅ Success Criteria

- [x] **Absolute Control**: STM32 physically prevents Pi from booting on power-up.
- [x] **Real Crypto**: Implements SHA-256 hashing and RSA-2048 verification using mbedTLS.
- [x] **Dynamic Metadata**: Parses custom firmware headers to prevent TOCTOU attacks.
- [x] **High Availability**: Implements Dual-Slot Auto-Recovery against firmware corruption.
- [x] **Self-Healing**: Utilizes hardware Flash APIs to reprogram corrupted memory sectors.
- [x] **Anti-Rollback**: Prevents execution of older, signed firmware by enforcing SVN progression.