<style>
	.header {
		font-weight:bold;
		font-size: 32px;
		font-family: Times New Roman;
	}
	.titr {
		font-weight:bold;
		font-size: 24px;
		font-family: Times New Roman;
	}
	.paraghraph {
		font-weight:normal;
		font-size: 16px;
		font-family: Times New Roman;
	}
</style>
<h1 align="center" class = "header">
STM32F10x Guidance Project: Timer, RTC, Interrupt, UART, DMA, GPIO and RRC in registry state.
</h1>
<h2 class = "titr">
Overview
</h2>
<p align="justify" class = "paraghraph">
This project is a firmware implementation for an STM32F10x microcontroller to display the current time and date via UART when triggered by an external interrupt. The system uses a Real-Time Clock (RTC) to maintain timekeeping, a timer (TIM3) for controlling an output pin, and a debounced input pin to trigger the transmission of time and date data.
</p>
<h2 class = "titr">
Features
</h2>
<ul class = "paraghraph">
  <li>
	Configures the STM32F10x system clock to use an external 8 MHz HSE with a PLL multiplier for 72 MHz operation.
  </li>
  <li>Initializes GPIO pins for input (PA1) and output (PB10).</li>
  <li>Sets up an external interrupt (EXTI1) on PA1 with debouncing to trigger time/date transmission.</li>
  <li>Configures TIM3 for one-pulse mode to control PB10.</li>
  <li>Uses RTC to maintain and display time and date in BCD format.</li>
  <li>Transmits time and date via UART (USART1) at 115200 baud.</li>
  <li>Implements a SysTick timer for millisecond-based timing and debouncing.</li>
</ul>
<h2 class = "titr">
Hardware Requirements
</h2>
<ul class = "paraghraph">
	<li>
		STM32F10x microcontroller (e.g., STM32F103C8T6).
	</li>
	<li>
		External 8 MHz crystal oscillator for HSE.
	</li>
	<li>
		Push-button or switch connected to PA1 for triggering the interrupt.
	</li>
	<li>
		LED or output device connected to PB10.
	</li>
	<li>
		UART-to-USB adapter (e.g., FT232R) for serial communication with a PC.
	</li>
	<li>
		Backup battery for RTC (optional for persistent timekeeping).
	</li>
</ul>
<h2 class = "titr">
Software Requirements
</h2>
<ul class = "paraghraph">
	<li>
		STM32CubeIDE or Keil uVision with STM32F10x Standard Peripheral Library.
	</li>
	<li>
		Terminal emulator (e.g., PuTTY, Tera Term) for viewing UART output.
	</li>
	<li>
		Compiler toolchain (e.g., GCC for ARM).
	</li>
</ul>
<h2 class = "titr">
Setup Instructions
</h2>
<ol class = "paraghraph">
  <li>Hardware Setup:</li>
  	<ul>
  		<li>
			Connect the 8 MHz crystal to the HSE pins (consult your MCU datasheet).
  		</li>
		<li>
			Connect a push-button to PA1 with a pull-down resistor.
  		</li>
		<li>
			Connect an LED or output device to PB10.
  		</li>
		<li>
			Connect USART1 (PA9 for TX, PA10 for RX) to a UART-to-USB adapter.
  		</li>
  	</ul>
  <li>Software Setup:</li>
	<ul>
		<li>
			Clone or download this project to your development environment.
		</li>
		<li>
			Ensure the STM32F10x Standard Peripheral Library is included in your project.
		</li>
		<li>
			Compile and flash the code to the STM32F10x microcontroller using an ST-Link or similar programmer.
		</li>
	</ul>
  <li>Operation:</li>
	<ul>
		<li>
			Power on the microcontroller.
		</li>
		<li>
			Press the button connected to PA1 to trigger an interrupt.
		</li>
		<li>
			The time and date will be transmitted via UART in the format: Time: HH:MM:SS Date: DD/MM/YY.
		</li>
		<li>
			The LED on PB10 will toggle based on TIM3's one-pulse mode when UART transmission occurs.
		</li>
	</ul>
</ol>
<h2 class = "titr">
Code Structure
</h2>
<ul class = "paraghraph">
	<li>
		<strong>main.c:</strong> Contains the main loop, system initialization, and interrupt-driven logic for time/date transmission.
	</li>
	<li>
		<strong>SystemClock_Config:</strong> Configures the system clock to 72 MHz using HSE and PLL.
	</li>
	<li>
		<strong>GPIO_init:</strong> Sets up PA1 as an input with pull-up and PB10 as a push-pull output.
	</li>
	<li>
		<strong>EXTI_config & NVIC_config:</strong> Configures EXTI1 for PA1 and enables interrupts with appropriate priorities.
	</li>
	<li>
		<strong>systick_config & SysTick_Handler:</strong> Implements a millisecond tick counter for debouncing and delays.
	</li>
	<li>
		<strong>TIM3_config & TIM3_start:</strong> Configures TIM3 for one-pulse mode to control PB10.
	</li>
	<li>
		<strong>RTC_config:</strong> Initializes the RTC with a preset time (17:29:00) and date (30/05/25).
	</li>
	<li>
		<strong>UART_config & UART_Transmit:</strong> Configures USART1 for serial communication and transmits data.
	</li>
	<li>
		<strong>send_TimeDate:</strong> Formats and sends the current time and date via UART.
	</li>
	<li>
		<strong>input_pin_event:</strong> Handles debouncing and interrupt flag management for PA1.
	</li>
</ul>
<h2 class = "titr">
Usage
</h2>
<ul class = "paraghraph">
	<li>
		Press the button on PA1 to trigger the system to send the current time and date via UART.
	</li>
	<li>
		The LED on PB10 will turn on during UART transmission and turn off when TIM3 completes its one-pulse cycle.
	</li>
	<li>
		Monitor the UART output on a terminal emulator at 115200 baud, 8N1.
	</li>
</ul>
<h2 class = "titr">
Notes
</h2>
<ul class = "paraghraph">
	<li>
		The RTC is initialized with a fixed time and date. Modify RTC_config to set a different initial time/date or implement a mechanism to update it dynamically.
	</li>
	<li>
		The debounce period is set to 200 ms (debounce variable). Adjust as needed for your application.
	</li>
	<li>
		Ensure proper clock source (HSE) and backup battery setup for reliable RTC operation.
	</li>
	<li>
		The UART baud rate is fixed at 115200. Adjust USART1->BRR in UART_config for different baud rates if needed.
	</li>
</ul>
<h2 class = "titr">
Languages and Tools:
</h2>
<p align="left">
<a href="https://www.cprogramming.com/" target="_blank" rel="noreferrer"> <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/c/c-original.svg" alt="c" width="40" height="40"/> </a>
<a href="https://www.w3schools.com/cpp/" target="_blank" rel="noreferrer"> <img src="https://raw.githubusercontent.com/devicons/devicon/master/icons/cplusplus/cplusplus-original.svg" alt="cplusplus" width="40" height="40"/> </a>
<a href="https://git-scm.com/" target="_blank" rel="noreferrer"> <img src="https://www.vectorlogo.zone/logos/git-scm/git-scm-icon.svg" alt="git" width="40" height="40"/> </a>
<a href="https://www.keil.com/download/product/" target="_blank" rel="noreferrer"> <img src="https://www.logo.wine/a/logo/Keil_(company)/Keil_(company)-Logo.wine.svg" alt="keil" width="100" height="40"/> </a>
</p>