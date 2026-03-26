# Wiring Diagram

## Target Board
- **STM32F446RE Nucleo-64**

## DS18B20 Temperature Sensor (1-Wire)
| DS18B20 Pin | STM32 Pin | Notes                          |
|-------------|-----------|--------------------------------|
| DATA        | PA8       | 4.7kΩ pull-up to 3.3V         |
| VCC         | 3.3V      |                                |
| GND         | GND       |                                |

## DHT22 Temperature & Humidity Sensor
| DHT22 Pin | STM32 Pin | Notes                          |
|-----------|-----------|--------------------------------|
| DATA      | PA9       | 10kΩ pull-up to 3.3V          |
| VCC       | 3.3V      |                                |
| GND       | GND       |                                |

## SSD1306 OLED Display (128x64, I2C)
| OLED Pin | STM32 Pin | Function    |
|----------|-----------|-------------|
| SCL      | PB8       | I2C1_SCL    |
| SDA      | PB9       | I2C1_SDA    |
| VCC      | 3.3V      |             |
| GND      | GND       |             |

> Both I2C lines need 4.7kΩ pull-ups to 3.3V.

## Heater Control (MOSFET + heating element)
| Connection | STM32 Pin | Function         |
|------------|-----------|------------------|
| MOSFET Gate| PA6       | TIM3_CH1 (PWM)   |

## Fan Control (MOSFET + DC fan)
| Connection | STM32 Pin | Function         |
|------------|-----------|------------------|
| MOSFET Gate| PA7       | TIM3_CH2 (PWM)   |

> Use N-channel MOSFETs (e.g., IRLZ44N) with gate driven from STM32 GPIO.
> Add flyback diodes across inductive loads (fan motor).

## Setpoint Potentiometer
| Pot Pin  | STM32 Pin | Function       |
|----------|-----------|----------------|
| Wiper    | PA0       | ADC1_CH0       |
| One end  | 3.3V      |                |
| Other end| GND       |                |

## User Button (Mode Select)
| Button  | STM32 Pin | Notes              |
|---------|-----------|--------------------|
| Signal  | PC13      | Nucleo onboard btn |

> PC13 already has a button on Nucleo-64 boards.

## UART (via ST-Link VCP)
| Function | STM32 Pin |
|----------|-----------|
| TX       | PA2       |
| RX       | PA3       |

## Pin Summary
```
PA0  → Potentiometer wiper (ADC1_CH0)
PA2  → UART TX (USART2)
PA3  → UART RX (USART2)
PA6  → Heater PWM (TIM3_CH1)
PA7  → Fan PWM (TIM3_CH2)
PA8  → DS18B20 DATA (1-Wire)
PA9  → DHT22 DATA
PB8  → OLED SCL (I2C1)
PB9  → OLED SDA (I2C1)
PC13 → User button (mode cycle)
```
