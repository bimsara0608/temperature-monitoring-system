# temperature-monitoring-system
Overview
This project is a temperature monitoring system built using an ATmega328P microcontroller. It features a temperature display in both Celsius and Fahrenheit, a temperature alarm system, and LED indicators for different temperature ranges. The system allows users to control the display modes and reset alarms using push buttons.

Key Features
Dual Temperature Display: Displays temperature in both Celsius and Fahrenheit.
Alarm System: An alarm is triggered when the temperature exceeds 40°C.
Temperature Range Indicators: LEDs display different temperature ranges (Cold, Normal, Warm, Hot).
Mode Switching: Switch between temperature display and range indication using a button.
Push-Button Controls: Controls for temperature adjustments, mode switching, and alarm reset.
Components Used
ATmega328P Microcontroller
7-Segment Display (Cathode Type)
LEDs (Temperature Indicators and Alarm)
Tactile Push Buttons
Resistors, Capacitors, and other supporting components
Buzzer (for alarm)
How to Use
Mode Switching: Use the button to toggle between displaying temperature on the 7-segment display and viewing the temperature range via LEDs.
Alarm System: The alarm triggers when the temperature exceeds 40°C. Press the reset button to acknowledge or reset the alarm.
Temperature Display: The system displays both Celsius and Fahrenheit, toggled by a button.

PCB Design

How to Run the Project
Hardware Setup: Assemble the circuit as shown in the schematic and PCB design.
Firmware: Upload the provided code to the ATmega328P using AVR programming tools.
Testing: Use the push buttons to increment or decrement the temperature and see the corresponding changes in the display and LEDs.
Code
The project code is included in this repository. Download or clone the repository to get started.

Improvements
Integrating an actual temperature sensor instead of push-button control for real-time
