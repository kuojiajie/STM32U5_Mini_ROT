# 🛡️ STM32U5 Mini-ROT (Hardware Root of Trust)

## 📖 Project Overview

This project implements a bare-metal **Root of Trust (RoT)** on an STM32U5 microcontroller to control the secure boot process of a target system (**Raspberry Pi 4B**).

The STM32 acts as a hardware gatekeeper. It physically holds the Pi in **RESET** state upon power-up, executes a verification sequence (simulated delay or crypto-check), and only releases the RESET line if validation passes.

## 🛠️ Hardware Setup

### Connection Interface

**⚠️ Critical:** A low-impedance **Common Ground** is required. It is highly recommended to connect the USB metal shells of both devices together to prevent ground loops.

| STM32 Pin | Raspberry Pi Pin | Signal Name | Description |
| :--- | :--- | :--- | :--- |
| **PF15** (CN10-12) | **RUN** (J2) | `PI_RESET` | Active-Low Reset Control (Open Drain) |
| **GND** | **GND** | Common GND | **Must connect** to eliminate voltage potential diff |

### Power Supply

- **STM32**: Powered via ST-LINK USB
- **Raspberry Pi**: Independently powered via USB-C

---

## 💻 Technical Implementation

This project deviates from standard STM32CubeMX generation to meet security requirements.

### 1. Early Initialization Strategy (`main.c`)

To mitigate race conditions where the Pi boots faster than the STM32, the **GPIO Initialization** is moved **before** the System Clock Configuration.

```c
/* Core/Src/main.c */
HAL_Init();

// [CRITICAL] Early Init: Lock the Pi before configuring the clock
MX_GPIO_Init();
HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);

SystemClock_Config(); // Standard clock config follows
```

### 2. Hardware Control Logic (`.ioc`)

**PF15** is configured as `Output Open Drain`. This allows the STM32 to safely pull the Pi's RUN pin low (Reset) or float high (Boot) without voltage conflicts.

### 3. File Structure

- **`Core/Src/rot_core.c`**: Implements the Secure Boot state machine (Hold -> Release)
- **`Core/Src/uart_utils.c`**: Redirects `printf` to UART1 for debug logging
- **`Core/mbedtls/`**: (Phase 2) Integrated mbedTLS library for cryptographic verification

---

## 🚀 Getting Started

This repository contains all necessary drivers and libraries to compile directly.

### Prerequisites

- **IDE**: STM32CubeIDE (v1.16.0+)
- **Hardware**: STM32 NUCLEO-U575ZI-Q + Raspberry Pi 4B

### Build & Run

1. **Clone**: `git clone <repo_url>`
2. **Import**:
   - Open STM32CubeIDE
   - `File` -> `Import` -> `Existing Projects into Workspace`
   - Select the project directory
3. **Build**: Click the **Hammer** icon (Should complete with 0 errors)
4. **Run**: Connect STM32 via USB and click **Run**

### Verification

1. Open a Serial Terminal (115200, 8N1)
2. **Power Cycle**:
   - Hold STM32 **Black Reset** button
   - Plug in Raspberry Pi power (Pi Green LED should be **OFF**)
   - Release STM32 Reset button
3. **Observe**:
   - Log: `[ROT] System Initialized. Gate is CLOSED.`
   - Log: `[ROT] Verifying...`
   - Log: `[ROT] Releasing the Gate!` -> **Pi Green LED starts blinking**

---

## ✅ Success Criteria

- [x] **Absolute Control**: STM32 physically prevents Pi from booting on power-up
- [x] **Reliable Reset**: Uses PF15 (Open Drain) + Shared Chassis Ground
- [x] **Early Init**: Software logic handles power-on race conditions.