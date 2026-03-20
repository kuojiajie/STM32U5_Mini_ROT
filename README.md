# STM32U5 Mini-ROT

## Project Goal

Establish absolute control of the Trust Anchor (STM32) over the Target System (Raspberry Pi). The STM32 actively holds the Pi's Reset pin low upon power-on to prevent unauthorized booting. It executes a simulated verification (5s countdown) before releasing control.

---

## Hardware Setup

### Connection Interface

**Common Ground is critical.** Ensure a low-impedance ground connection (e.g., soldering or connecting USB shells) to prevent control failure.

| STM32 Pin | Raspberry Pi Pin | Signal Name | Description |
| :--- | :--- | :--- | :--- |
| PF15 (CN10-12) | RUN (J2) | PI_RESET | Active-Low Reset Control |
| GND | GND | Common GND | Must connect to eliminate voltage potential diff |

### Power Supply

- **STM32**: Powered via ST-LINK USB
- **Raspberry Pi**: Independently powered via USB-C

---

## Key Implementation Details

This project deviates from the standard STM32CubeMX generated code in specific ways to ensure security requirements.

### 1. Pin Configuration (.ioc)

**PF15 (PI_RESET)**: Configured as Output Open Drain with Low default level.

**Reason**: PF15 is a clean GPIO without onboard capacitor interference (unlike PA0), ensuring instant reset signal.

**UART1**: Configured at 115200 bps for debug logging.

### 2. Initialization Strategy (main.c)

To mitigate race conditions where the Pi boots faster than the STM32, the GPIO Initialization is moved before the System Clock Configuration.

```c
/* Core/Src/main.c */
HAL_Init();

// [CRITICAL] Early Init: Lock the Pi before configuring the clock
MX_GPIO_Init();
HAL_GPIO_WritePin(PI_RESET_GPIO_Port, PI_RESET_Pin, GPIO_PIN_RESET);

SystemClock_Config(); // Standard clock config follows
```

### 3. File Structure

- **Core/Src/rot_core.c**: Implements the Secure Boot state machine (Hold → Release)
- **Core/Src/uart_utils.c**: Implements printf redirection to UART

---

## Getting Started

### Prerequisites

- **IDE**: STM32CubeIDE (v1.16.0 or later recommended)
- **Hardware**: STM32 NUCLEO-U575ZI-Q board + Raspberry Pi 4B

### Build & Run

1. **Import**: Open STM32CubeIDE → File → Import → Existing Projects into Workspace → Select this directory
2. **Build**: Click the Hammer icon (Build). Ensure 0 errors
3. **Run**: Connect STM32 via USB and click Run

### Verification

1. **Open a Serial Terminal** (115200, 8N1)
2. **Power Cycle**:
   - Hold STM32 Reset button
   - Plug in Raspberry Pi power (Pi Green LED should be OFF)
   - Release STM32 Reset button
3. **Observe**:
   - Log: `Holding Reset...` → Pi remains OFF
   - Log: `Releasing the Gate!` → Pi Green LED starts blinking (Boot)

---

## Success Criteria Checklist

- [x] **Absolute Control**: STM32 physically prevents Pi from booting on power-up
- [x] **Reliable Reset**: Uses PF15 (Open Drain) + Shared Chassis Ground to ensure valid logic levels
- [x] **Early Init**: Software logic handles power-on race conditions
