# 🛡️ STM32U5 Mini-ROT (Hardware Root of Trust)

## 📖 Project Overview

This project implements a bare-metal **Root of Trust (RoT)** on an STM32U5 microcontroller to control the secure boot process of a target system (**Raspberry Pi 4B**).

The STM32 acts as a hardware gatekeeper. It physically holds the Pi in **RESET** state upon power-up, reads firmware directly from Flash memory, calculates its SHA-256 hash, and verifies it against an RSA-2048 signature.

**✨ Advanced Feature:** It features an **A/B Dual Image Auto-Recovery** mechanism. If the primary firmware is corrupted or tampered with, the RoT automatically falls back to a secure backup image, ensuring system availability.

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

This project deviates from standard STM32CubeMX generation to meet security requirements.

### 1. Early Initialization Strategy (`main.c`)

To mitigate race conditions where the Pi boots faster than the STM32, the **GPIO Initialization** is moved **before** the System Clock Configuration.

### 2. Hardware Control Logic (`.ioc`)

- **PF15** is configured as `Output Open Drain`. This allows safe control of the Pi's Reset logic.
- **RNG** (Random Number Generator) is enabled for cryptographic operations.

### 3. Verification Logic

- **Real-time Hashing**: Reads firmware data from Flash memory via direct pointers and calculates SHA-256 chunk-by-chunk.
- **Signature Verification**: Uses **mbedTLS** to verify the hash against an RSA-2048 signature stored in `keys_data.h`.

### 4. A/B Dual Image Auto-Recovery

The internal flash is partitioned into **Slot A** (`0x08100000`) and **Slot B** (`0x08180000`). If Slot A fails cryptographic verification, the RoT automatically initiates a fallback sequence to verify and boot from Slot B, providing a robust hardware failsafe.

## 🚀 Getting Started

### Prerequisites

- **IDE**: STM32CubeIDE (v1.16.0+)
- **Hardware**: STM32 NUCLEO-U575ZI-Q + Raspberry Pi 4B

### Build & Run

1. **Clone**: `git clone https://github.com/yourusername/STM32U5_Mini_ROT.git`
2. **Import**:
   - Open STM32CubeIDE -> `File` -> `Import` -> `Existing Projects into Workspace`
   - Select the project directory
3. **Build**: Click the **Hammer** icon.
4. **Flash Firmware Data (Crucial Step)**:
   - Go to `Run -> Run Configurations -> Startup -> Load Image and Symbols`.
   - Add `crypto_data/data.bin` at address **`0x08100000`** (Slot A).
   - Add `crypto_data/data.bin` at address **`0x08180000`** (Slot B).
5. **Run**: Connect STM32 via USB and click **Debug/Run**.

## 🧪 Verification & Chaos Testing

Open a Serial Terminal (115200, 8N1) and observe the STM32's behavior.

### Test 1: Normal Boot

With valid `data.bin` in both slots:
- Log: `[ROT] >>> Attempting to Boot from Slot A <<<`
- Log: `[ROT] SUCCESS: Slot A is VALID.`
- Log: `[ROT] Releasing Pi Gate.` -> **Pi Green LED starts blinking**

### Test 2: Auto-Recovery (Chaos Test)

1. Go to Run Configurations and change the Slot A image (`0x08100000`) to `crypto_data/hacked.bin`.
2. Run the Debugger again.
3. **Observe the Failsafe:**
   - Log: `FAIL! (Error: -0xXXXX)`
   - Log: `[ROT] WARNING: Slot A Corrupted or Tampered!`
   - Log: `[ROT] Initiating Auto-Recovery Procedure...`
   - Log: `[ROT] >>> Attempting to Boot from Slot B (Backup) <<<`
   - Log: `[ROT] System Recovered! Releasing Pi Gate.` -> **Pi Boots Successfully**

## ✅ Success Criteria

- [x] **Absolute Control**: STM32 physically prevents Pi from booting on power-up.
- [x] **Real Crypto**: Implements SHA-256 hashing and RSA-2048 verification using mbedTLS.
- [x] **High Availability**: Implements Dual-Slot Auto-Recovery against firmware corruption.
