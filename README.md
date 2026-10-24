# STM32F401RE Bare-Metal Peripheral Driver Suite: USART2

A vendor HAL-independent, register-level bare-metal C driver for the USART2 peripheral on the **STM32 NUCLEO-F401RE** (ARM Cortex-M4) microcontroller.

---

## 📌 Architecture & Pin Mapping

The USART2 peripheral interfaces directly with the onboard ST-LINK Virtual COM Port (VCP), enabling bidirectional serial communication over the USB interface without external FTDI converters.

| Pin | Alternate Function | Target Peripheral | Configuration |
| :--- | :--- | :--- | :--- |
| **PA2** | AF07 | USART2_TX | Alternate Function, Push-Pull, Pull-Up, Very High Speed |
| **PA3** | AF07 | USART2_RX | Alternate Function, Push-Pull, Pull-Up, Very High Speed |

---

## ⚙️ Mathematical Derivations: Baud Rate Generation

- **Peripheral Bus:** APB1 Bus ($f_{PCLK} = 16\text{ MHz}$ default HSI)
- **Target Baud Rate:** $115,200\text{ bps}$
- **Oversampling Mode:** OVER8 = 0 (16x oversampling)

$$\text{USARTDIV} = \frac{f_{PCLK}}{16 \times \text{Baud Rate}} = \frac{16,000,000}{16 \times 115,200} \approx 8.6805$$

- **DIV_Mantissa:** $\lfloor 8.6805 \rfloor = 8 = \text{0x08}$
- **DIV_Fraction:** $0.6805 \times 16 \approx 10.88 \rightarrow 11 = \text{0x0B}$
- **USART_BRR Value:**
  $$\text{BRR} = (\text{Mantissa} \ll 4) \mid \text{Fraction} = (8 \ll 4) \mid 11 = \text{0x008B}$$

---

## 🛡️ Critical Implementation Highlights

1. **Hardware Glitch Prevention:** Configured internal pull-ups on both TX and RX pins (`GPIO_PIN_UP`) to prevent floating-line garbage values during bus idle states.
2. **Flag Synchronization (TXE vs TC):**
   - Implemented `TXE` (Transmit Data Register Empty) polling within character-pumping loops to prevent data overwrites.
   - Guarded frame completion with `TC` (Transmission Complete) polling after the final byte to guarantee zero bit truncation before peripheral de-assertion.
3. **Parity & Word Length Masking:** Integrated dynamic bitmasking for 8-bit and 9-bit payload configurations (handling hardware-stripped parity bits).

---

## 🧪 Validation & Testing

- **Echo Test:** Validated bidirectional full-duplex communication at 115200 8N1. The firmware captures ASCII characters via `USART_ReceiveData()` and re-transmits them immediately using `USART_SendData()`.
- **Serial Monitor Configuration:** 115200 Baud, 8 Data Bits, 1 Stop Bit, No Parity, No Hardware Flow Control.
