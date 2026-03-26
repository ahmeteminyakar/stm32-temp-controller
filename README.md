# Multi-Mode Embedded Temperature Control System

Closed-loop temperature controller with dual sensors, PID control, OLED display, and UART logging on STM32F446RE.

## Features

- **Dual sensors**: DS18B20 (1-Wire) + DHT22 — temperature fusion, humidity reading
- **PID control**: Independent heater and fan PWM outputs
- **Three modes**: HEAT (heater only), COOL (fan only), AUTO (PID selects)
- **OLED display**: Real-time temp, setpoint, mode, duty cycle on SSD1306 128x64
- **Setpoint control**: Potentiometer-based (ADC), adjustable 15-50°C range
- **Mode button**: Nucleo user button (PC13) cycles through modes
- **UART logging**: CSV telemetry at 10 Hz

## Hardware

| Component       | Detail                            |
|-----------------|-----------------------------------|
| MCU             | STM32F446RE (Nucleo-64)           |
| Temp sensor 1   | DS18B20 (1-Wire)                  |
| Temp sensor 2   | DHT22 (temperature + humidity)    |
| Display         | SSD1306 128x64 OLED (I2C)        |
| Heater          | Heating element via N-MOSFET      |
| Fan             | DC fan via N-MOSFET               |
| Setpoint        | 10kΩ potentiometer on ADC         |

See [docs/wiring.md](docs/wiring.md) for full pin connections.

## Project Structure

```
stm32-temp-controller/
├── Core/
│   ├── Inc/
│   │   ├── main.h          # Pin defs, mode enum, constants
│   │   ├── pid.h           # PID controller
│   │   ├── ds18b20.h       # DS18B20 1-Wire driver
│   │   ├── dht22.h         # DHT22 driver
│   │   ├── oled.h          # SSD1306 OLED driver
│   │   ├── control.h       # Mode logic, output mapping
│   │   └── uart_log.h      # UART CSV logging
│   └── Src/
│       ├── main.c          # Init, main loop, peripherals
│       ├── pid.c           # PID with anti-windup
│       ├── ds18b20.c       # 1-Wire bit-bang, DWT timing
│       ├── dht22.c         # DHT22 protocol, DWT timing
│       ├── oled.c          # SSD1306 I2C driver, 5x7 font
│       ├── control.c       # HEAT/COOL/AUTO logic
│       └── uart_log.c      # CSV telemetry output
├── docs/
│   └── wiring.md           # Pin connections
└── README.md
```

## Build & Flash

### Using STM32CubeIDE
1. Create a new project for **STM32F446RETx**
2. Copy `Core/Inc/` and `Core/Src/` files into the generated project
3. Configure peripherals in CubeMX to match pin assignments in `main.h`
4. Build and flash

### Peripheral Configuration (CubeMX)
- **TIM3**: PWM CH1 (PA6) + CH2 (PA7) — Prescaler: 89, Period: 999
- **I2C1**: Fast mode 400kHz — SCL (PB8), SDA (PB9)
- **USART2**: 115200 baud — TX (PA2), RX (PA3)
- **ADC1**: Channel 0 (PA0), 12-bit, single conversion
- **PC13**: GPIO EXTI, falling edge (user button)
- **PA8**: GPIO output (open-drain) — DS18B20
- **PA9**: GPIO output/input — DHT22

## Control Modes

| Mode | Behavior                                              |
|------|-------------------------------------------------------|
| HEAT | PID drives heater only, fan is off                    |
| COOL | PID drives fan only, heater is off                    |
| AUTO | PID output positive → heater; negative → fan          |

Press the user button (PC13) to cycle: HEAT → COOL → AUTO → HEAT...

## OLED Display Layout

```
Mode: AUTO

Temp: 24.3 C
Set:  25.0 C
Hum:  55 %

H: 15%  F:  0%
```

## UART Log Format

CSV at 10 Hz over ST-Link VCP (115200 baud):
```
time_ms,mode,setpoint,temp,humidity,heater%,fan%
5000,AUTO,25.0,24.3,55.0,15,0
```

## Default Parameters

| Parameter | Value  |
|-----------|--------|
| Kp        | 5.0    |
| Ki        | 0.3    |
| Kd        | 1.0    |
| Loop rate | 10 Hz  |
| Setpoint  | 15-50°C (pot adjustable) |
