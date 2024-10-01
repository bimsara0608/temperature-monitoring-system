#define F_CPU 4000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

const uint8_t binaryvalues[12] = { //decimal digit
	0b00000000, // 0
	0b00000001, // 1
	0b00000010, // 2
	0b00000011, // 3
	0b00000100, // 4
	0b00000101, // 5
	0b00000110, // 6
	0b00000111, // 7
	0b00001000, // 8
	0b00001001, // 9
	0b00001010, // C
	0b00001111  // F
};

int16_t temperature = 250; // Initialize temperature to 25 'C
uint8_t alarm_active = 0; // Flag for alarm
uint8_t alarm_reset = 0; // Flag for alarm reset
uint8_t is_fahrenheit = 0; // Flag to indicate if the temperature is in Fahrenheit
uint8_t display_mode = 0; // Flag to indicate display mode, 0 for ssd, 1 for leds
uint8_t brightness1 = 0, brightness2 = 0, brightness3 = 0, brightness4 = 0;

void set_led_brightness(uint8_t brightness1, uint8_t brightness2, uint8_t brightness3, uint8_t brightness4);
void update_led_brightness(void);
void toggle_temperature_scale(void);
void toggle_display_mode(void);
void set_pwm_state(uint8_t state);

int main() {

	// Set clock prescaler to 1 (no division)
	CLKPR = (1 << CLKPCE); // Enable change of the clock prescaler
	CLKPR = 0; // Set clock prescaler to 1 (no division)

	// Set up PWM on LED pins
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1) | (1 << WGM00) | (1 << WGM01); // Fast PWM mode
	TCCR0B |= (1 << CS00); // No prescaler

	TCCR1A |= (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10); // Fast PWM mode
	TCCR1B |= (1 << CS10); // No prescaler

	// Set PWM pins as outputs
	DDRD |= (1 << PD5) | (1 << PD6); // PD5 (OC0B) and PD6 (OC0A)
	DDRB |= (1 << PB1) | (1 << PB2); // PB1 (OC1A) and PB2 (OC1B)

	// Set up SSD pins
	DDRC |= ((1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5)); // Set digit select pins as outputs
	DDRB |= ((1 << PB0) | (1 << PB3)); // Set PORTB as outputs (for SSD)
	DDRD |= ((1 << PD4) | (1 << PD7)); // Set PORTD as outputs (for SSD)

	// Set up button pins
	DDRC &= ~((1 << PC1)); // Set PC1 as input (alarm reset)
	DDRD &= ~((1 << PD2) | (1 << PD3)); // Set PD2 and PD3 as inputs (PD2 up/ PD3 down)
	DDRB &= ~((1 << PB4) | (1 << PB5)); // Set PB4 and PB5 as inputs (PB4 C/F , PB5 mode)
	
	DDRC |= (1 << PC0); //set output for alarm
	

	// Trigger on falling edge
	EICRA |= (1 << ISC01) | (1 << ISC11);
	// Enable INT0 and INT1
	EIMSK |= (1 << INT0) | (1 << INT1);

	sei(); // Enable global interrupts
	
	//for alarm beeping
	uint16_t led_counter = 0;
	uint8_t led_state = 0;

	while (1) {
		if (!display_mode) {
			set_pwm_state(0);
			
			// --------------Seven segment display mode-------------------------
			
			for (uint8_t digit = 0; digit < 4; digit++) { // Digit selection loop
				PORTC &= ~(0x3C); // Turn off all digits (PC2 to PC5)
				_delay_us(50); // Brief delay to avoid ghosting

				// Calculate the digit to display for the current digit
				uint8_t digit_to_display = 0;

				if (digit == 0) {
					// Display 'C' or 'F' in the thousand's place
					digit_to_display = is_fahrenheit ? 11 : 10; // 11 for 'F', 10 for 'C'
					
					//This is not showing in the product because the decoder only outputs 0 to 9
				}
				else {
					int16_t temp = temperature / 10; // Use integer part only for display

					if (digit == 1) {
						digit_to_display = (temp / 100) % 10; // Hundred's place
					}
					else if (digit == 2) {
						digit_to_display = (temp / 10) % 10; // Ten's place
					}
					else if (digit == 3) {
						digit_to_display = temp % 10; // One's place
					}
				}

				// Clear the bits for the BCD values
				
				PORTB &= ~((1 << PB0) | (1 << PB3)); // Clear PB0 and PB3
				PORTD &= ~((1 << PD4) | (1 << PD7)); // Clear PD4 and PD7

				// Set the bits for the BCD values
				PORTB |= ((binaryvalues[digit_to_display] & 0x01) << PB0); // Set PB0
				PORTB |= ((binaryvalues[digit_to_display] & 0x02) << (PB3 - 1)); // Set PB3 (bit 1 to bit 3)
				PORTD |= ((binaryvalues[digit_to_display] & 0x04) << (PD4 - 2)); // Set PD4 (bit 2 to bit 4)
				PORTD |= ((binaryvalues[digit_to_display] & 0x08) << (PD7 - 3)); // Set PD7 (bit 3 to bit 7)

				PORTC |= (1 << (digit + 2)); // Turn on the current digit

				_delay_ms(5); // Delay for display refresh speed

				PORTC &= ~(1 << (digit + 2)); //turn on the corresponding digit

				set_led_brightness(0,0,0,0);
			}
			} else {
			set_pwm_state(1);
			//-------------- LED brightness mode-----------------------
			
			update_led_brightness(); // Update the LED brightness based on temperature
			
		}

		// Task 2 : ----------Alarm-------------
		
		
		//Alarm on for celcius
		if (temperature > 400 && !alarm_active && !alarm_reset && !is_fahrenheit) { // Compare with 400 (40.0°C)
			led_counter++;
			if (led_counter >= 100) {
				led_counter = 0;
				led_state = !led_state; // Toggle LED state
				if (led_state) {
					PORTC |= (1 << PC0); // Turn on the LED
					} else {
					PORTC &= ~(1 << PC0); // Turn off the LED
				}
			}
			} else if (!alarm_active && !alarm_reset && !is_fahrenheit) {
			PORTC &= ~(1 << PC0); // Ensure LED is off if temperature <= 40.0°C
			
		}
		//Alarm on for celcius
		if (temperature > 1040 && !alarm_active && !alarm_reset && is_fahrenheit) { // Compare with 1040 (104.0°F)
			led_counter++;
			if (led_counter >= 100) {
				led_counter = 0;
				led_state = !led_state; // Toggle LED state
				if (led_state) {
					PORTC |= (1 << PC0); // Turn on the LED
					} else {
					PORTC &= ~(1 << PC0); // Turn off the LED
				}
			}
			} else if (!alarm_active && !alarm_reset && is_fahrenheit) {
			PORTC &= ~(1 << PC0); // Ensure LED is off if temperature <= 104.0°F
		}

		// Check if the reset button is pressed (PC1)
		
		if (!(PINC & (1 << PC1))) { // if button pressed
			alarm_reset = 1; // Set alarm reset flag
			PORTC &= ~(1 << PC0); // Turn off the LED
		}

		// Check if temperature drops below 40.0°C to reset the alarm (for celcius)
		if (temperature <= 400 && alarm_reset && !is_fahrenheit) { // Compare with 400 (40.0°C)
			alarm_reset = 0; // Reset alarm reset flag
			alarm_active = 0; // Reset alarm active flag
		}

		// Check if temperature drops below 104.0°F to reset the alarm (for fahrenheit)
		if (temperature <= 1040 && alarm_reset && is_fahrenheit) { // Compare with 1040 (104.0°F)
			alarm_reset = 0; // Reset alarm reset flag
			alarm_active = 0; // Reset alarm active flag
		}

		// Check if the temperature toggle button (PB4) is pressed
		if (!(PINB & (1 << PB4))) {
			_delay_ms(50); // Debounce delay
			if (!(PINB & (1 << PB4))) { // Confirm button press
				while (!(PINB & (1 << PB4))); // Wait for button release
				toggle_temperature_scale();
			}
		}

		// Check if the mode button (PB5) is pressed
		if (!(PINB & (1 << PB5))){
			_delay_ms(50); // Debounce delay
			if (!(PINB & (1 << PB5))) { // Confirm button press
				while (!(PINB & (1 << PB5))); // Wait for button release
				toggle_display_mode();
			}
		}
	}
}

// Task : 1 (Interrupt service routines)

ISR(INT0_vect) {
	if (temperature < 99990) { // Compare with 99990 (9999.0°C)
		temperature += 10; // Increment by 10 (1.0°C)
	}
}

ISR(INT1_vect) {
	if (temperature > 0) {
		temperature -= 10; // Decrement by 10 (1.0°C)
	}
}
//Task:4 Toggle Between Celsius and Fahrenheit
void toggle_temperature_scale() {
	if (is_fahrenheit) {
		// Convert Fahrenheit to Celsius: C = (F - 32) * 5 / 9
		temperature = ((temperature - 320) * 5 + 5) / 9;
		is_fahrenheit = 0;
		} else {
		// Convert Celsius to Fahrenheit: F = C * 9 / 5 + 32
		temperature = (temperature * 9 + 5) / 5 + 320;
		is_fahrenheit = 1;
	}
}

//Task:5 Mode Switching
void toggle_display_mode() {
	display_mode = !display_mode; // Toggle between LED brightness display and ssd
}
//Task:3 Temperature Range Indication with LEDs
void set_led_brightness(uint8_t brightness1, uint8_t brightness2, uint8_t brightness3, uint8_t brightness4) {
	OCR0A = brightness1; // Set PWM duty cycle for LED1
	OCR0B = brightness2; // Set PWM duty cycle for LED2
	OCR1A = brightness3; // Set PWM duty cycle for LED3
	OCR1B = brightness4; // Set PWM duty cycle for LED4
}

void update_led_brightness() {

	if (!is_fahrenheit) {
		// Calculate brightness levels based on temperature in Celsius
		if (temperature <= 150) { // 0 to 15.0°C
			brightness1 = ((temperature) * 255) / 150;
			brightness2 = brightness3 = brightness4 = 0;
		}
		else if (temperature <= 250) { // 15.1 to 25.0°C
			brightness1 = 255;
			brightness2 = ((temperature - 150) * 255) / 100;
			brightness3 = brightness4 = 0;
		}
		else if (temperature <= 350) { // 25.1 to 35.0°C
			brightness1 = 255;
			brightness2 = 255;
			brightness3 = ((temperature - 250) * 255) / 100;
			brightness4 = 0;
		}
		else if (temperature <= 400) { // 35.1 to 40.0°C
			brightness1 = 255;
			brightness2 = 255;
			brightness3 = 255;
			brightness4 = ((temperature - 350) * 255) / 50;
		}
		else { // > 40.0°C
			brightness1 = 255;
			brightness2 = 255;
			brightness3 = 255;
			brightness4 = 255;
		}
		} else {
		// Calculate brightness levels based on temperature in Fahrenheit
		if (temperature <= 590) { // 0 to 59.0°F (approx. 15.0°C)
			brightness1 = (temperature * 255) / 590;
			brightness2 = brightness3 = brightness4 = 0;
		}
		else if (temperature <= 770) { // 59.1 to 77.0°F (approx. 25.0°C)
			brightness1 = 255;
			brightness2 = ((temperature - 590) * 255) / 180;
			brightness3 = brightness4 = 0;
		}
		else if (temperature <= 950) { // 77.1 to 95.0°F (approx. 35.0°C)
			brightness1 = 255;
			brightness2 = 255;
			brightness3 = ((temperature - 770) * 255) / 180;
			brightness4 = 0;
		}
		else if (temperature <= 1040) { // 95.1 to 104.0°F (approx. 40.0°C)
			brightness1 = 255;
			brightness2 = 255;
			brightness3 = 255;
			brightness4 = ((temperature - 950) * 255) / 90;
		}
		else { // > 104.0°F (approx. > 40.0°C)
			brightness1 = 255;
			brightness2 = 255;
			brightness3 = 255;
			brightness4 = 255;
		}
	}

	// Update LED brightness
	set_led_brightness(brightness1, brightness2, brightness3, brightness4);
}

void set_pwm_state(uint8_t state) {
	if (state) {
		// Enable PWM by setting the compare output mode bits
		TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
		TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
		} else {
		// Disable PWM by clearing the compare output mode bits
		TCCR0A &= ~((1 << COM0A1) | (1 << COM0B1));
		TCCR1A &= ~((1 << COM1A1) | (1 << COM1B1));
	}
}
