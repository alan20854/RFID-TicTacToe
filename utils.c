#include <fsl_device_registers.h>
#include "utils.h"

// RFID Tag Database
extern char rfidCodes[NUM_RFID_TAGS][10];

/* Set up 4x3 Keypad and Serial communication with RFID reader and PC */
void GPIO_Initialize(void) {
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;    /*Enable Port A Clock Gate Control*/
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK; 		/*Enable Port B Clock Gate Control*/
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;		/*Enable Port C Clock Gate Control*/
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; 		/*Enable Port D Clock Gate Control*/
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;		/*Enable Port E Clock Gate Control*/
	
	GPIOB_PDDR |= (1 << 23); // Set RFID enable pin as an output - PTB23 - pin D4
	PORTB_PCR23 = 0x100;
	
	GPIOB_PDOR |= (1 << 23); // Set RFID enable pin to 1 to disable it
	
	// Keypad
	// Row 1 is pin D7 - PTC3
	// Row 2 is pin D6 - PTC2
	// Row 3 is pin D5 - PTA2
	// Row 4 is pin D11 - PTD2
	// Col 1 is pin D10 - PTD0
	// Col 2 is pin D9 - PTC4
	// Col 3 is pin D8 - PTC12
	PORTC_PCR3 = 0x100; // Enable GPIO
	PORTC_PCR2 = 0x100;
	PORTA_PCR2 = 0x100;
	PORTD_PCR2 = 0x100;
	PORTD_PCR0 = 0x102; // Enable internal pull-downs on column pins
	PORTC_PCR4 = 0x102;
	PORTC_PCR12 = 0x102;
	
	GPIOC_PDDR |= (1 << 3); // Set rows as outputs
	GPIOC_PDDR |= (1 << 2);
	GPIOA_PDDR |= (1 << 2);
	GPIOD_PDDR |= (1 << 2);
	
	GPIOD_PDDR |= (0 << 0); // Set columns as inputs
	GPIOC_PDDR |= (0 << 4);
	GPIOC_PDDR |= (0 << 12);
	
	// UART initialization
	
	uint16_t ubd;					/*Variable to save the baud rate */
	uint8_t temp;

	SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;      /*Enable the UART0 clock (For PC communication) */
	PORTB_PCR16 |= PORT_PCR_MUX(3);
	PORTB_PCR17 |= PORT_PCR_MUX(3);
	
	SIM_SCGC4 |= SIM_SCGC4_UART3_MASK;      /*Enable the UART3 clock (For RFID reader) */
	PORTC_PCR16 |= PORT_PCR_MUX(3);
	PORTC_PCR17 |= PORT_PCR_MUX(3);
	
	
	//Serial port 0
	UART0_C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );  /*Disable Tx and Rx */
	UART0_C1 = 0; 		/*Default settings of the register*/

	ubd = (uint16_t)((21000*1000)/(9600 * 16));  /* Calculate baud settings */

	temp = UART0_BDH & ~(UART_BDH_SBR(0x1F));   /*Save the value of UART0_BDH except SBR */
	UART0_BDH = temp | (((ubd & 0x1F00) >> 8));
	UART0_BDL = (uint8_t)(ubd & UART_BDL_SBR_MASK);

	UART0_C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK );    /* Enable receiver and transmitter */
	
	// Serial port 3
	UART3_C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK );  /*Disable Tx and Rx */
	UART3_C1 = 0; 		/*Default settings of the register*/

	ubd = (uint16_t)((21000*1000)/(2400 * 16));  /* Calculate baud settings */

	temp = UART3_BDH & ~(UART_BDH_SBR(0x1F));   /*Save the value of UART3_BDH except SBR */
	UART3_BDH = temp | (((ubd & 0x1F00) >> 8));
	UART3_BDL = (uint8_t)(ubd & UART_BDL_SBR_MASK);

	UART3_C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK );    /* Enable receiver and transmitter */
	
	// Enable random number generator
	SIM_SCGC6 |= SIM_SCGC6_RNGA_MASK;
}

/* Simple delay method */
void delay() {
	for(int j=0; j<250000; j++);
}

/* Return a random 32-bit unsigned int */
unsigned int randomNumber(void) {
	RNG_CR &= ~RNG_CR_SLP_MASK; 
  RNG_CR |= RNG_CR_HA_MASK;
  RNG_CR |= RNG_CR_GO_MASK;
	
	delay();
	
	while ((RNG_SR & RNG_SR_OREG_LVL_MASK) == 0) {} // Random number not ready
	
	return RNG_OR & RNG_OR_RANDOUT_MASK;
}

/* Check if a key was pressed on the keypad and return the pressed character 
   This function is blocking, it will continue to run until a key is pressed
   Returns int value of key pressed, * is -1 and # is -2 */
int readKeypad(void) {
	int row = 0;
	while (1) {
		// Set all the rows to 0, then set rows to 1 one at a time (each iteration)
		GPIOC_PDOR &= ~(1 << 3);
		GPIOC_PDOR &= ~(1 << 2);
		GPIOA_PDOR &= ~(1 << 2);
		GPIOD_PDOR &= ~(1 << 2);
		if (row == 0) {
			GPIOC_PDOR |= (1 << 3);
		} else if (row == 1) {
			GPIOC_PDOR |= (1 << 2);
		} else if (row == 2) {
			GPIOA_PDOR |= (1 << 2);
		} else if (row == 3) {
			GPIOD_PDOR |= (1 << 2);
		}
		delay();
		
		// Determine which key was pressed based on the column pins' inputs
		if (GPIOD_PDIR & (1 << 0)) {
			if (row == 0) {
				return 1;
			} else if (row == 1) {
				return 4;
			} else if (row == 2) {
				return 7;
			} else if (row == 3) {
				return -1;
			}
		} else if (GPIOC_PDIR & (1 << 4)) {
			if (row == 0) {
				return 2;
			} else if (row == 1) {
				return 5;
			} else if (row == 2) {
				return 8;
			} else if (row == 3) {
				return 0;
			}
		} else if (GPIOC_PDIR & (1 << 12)) {
			if (row == 0) {
				return 3;
			} else if (row == 1) {
				return 6;
			} else if (row == 2) {
				return 9;
			} else if (row == 3) {
				return -2;
			}
		}
		row++;
		if (row == 4) row = 0;
	}
}

/* Get data from RFID reader - stores the RFID data in the char array provided as a pointer
   This function is blocking, it will keep running until it reads an RFID tag successfully. */
void getRFIDTag(char *tag) {
	GPIOB_PDOR &= ~(1 << 23); // Set RFID enable pin to 0 to enable it
	
	int bytesread = 0;
	
	while (bytesread < 10) {
		uint8_t ch = getChar_UART3();
		if (ch != 0x0A && ch != 0x0D) { // Ignore the start and end bits
			tag[bytesread] = ch;
			bytesread++;
		}
	}
	
	GPIOB_PDOR |= (1 << 23); // Set RFID enable pin to 1 to disable it
}

/* Compare the RFID tag to the known list of RFID tags.
   If there's a match, return the index of the tag in the tag array.
   Otherwise, the unknown tag's ID is printed and -1 is returned. */
int getRFIDTagIndex(char *tag) {
	for (int i = 0; i < sizeof(rfidCodes)/(10*sizeof(char)); i++) {
		int match = 1;
		for (int j = 0; j < 10; j++) {
			if (rfidCodes[i][j] != tag[j]) {
				match = 0;
				break;
			}
		}
		if (match) {
			return i;
		}
	}
	serialWrite("Unknown RFID Tag: ");
	for (int k = 0; k < 10; k++) {
		putChar_UART0(tag[k]);
	}
	serialWrite("\r\n");
	return -1;
}

/* Serial functions */
uint8_t getChar_UART0(void) {
	/* Wait until character has been received */
	while (!(UART0_S1 & UART_S1_RDRF_MASK));
	/* Return the 8-bit data from the receiver */
	return UART0_D;
}

void putChar_UART0(char ch) {
	/* Wait until space is available in the FIFO */
	while(!(UART0_S1 & UART_S1_TDRE_MASK));
	/* Send the character */
	UART0_D = (uint8_t)ch;
}

uint8_t getChar_UART3(void) {
	/* Wait until character has been received */
	while (!(UART3_S1 & UART_S1_RDRF_MASK));
	/* Return the 8-bit data from the receiver */
	return UART3_D;
}

/* Send a string over serial */
void serialWrite(char *ptr_str) {
	while(*ptr_str)
		putChar_UART0(*ptr_str++);
}
