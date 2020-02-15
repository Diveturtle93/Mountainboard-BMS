//----------------------------------------------------------------------
// Titel	:	LTC6804 h-Code
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	11.09.2018
// Version	:	1.0
// Autor	:	Frederik Hafemeister
// Projekt	:	BMS
//----------------------------------------------------------------------

#ifndef LTC6804_H
#define LTC6804_H

// Allgemeine Einstellungen
//----------------------------------------------------------------------
#define LTC6804_DEVICES		1						// Number of devices in daisy chain
#define LTC6804_UVOLT		2000					// Unterspannung einer Zelle		3.2 = 2000 * 16 * 100µV, Spannung = VUV * 16 * 100µV, VUV muss im Register stehen
#define LTC6804_OVOLT		2625					// Überspannung einer Zelle			4.2 = 2625 * 16 * 100µV, Spannung = VOV * 16 * 100µV, VOV muss im Register stehen
#define LTC6804_UTEMP		0						// Untertemperatur einer Zelle
#define LTC6804_OTEMP		50						// Übertemperatur einer Zelle
//----------------------------------------------------------------------

// Command Codes definieren							// Datasheet Page 49 Table 34
//----------------------------------------------------------------------
// Configuration Register
//----------------------------------------------------------------------
#define WRCFG				0b00000000001			// Write Configuration
#define RDCFG				0b00000000010			// Read Configuration
//----------------------------------------------------------------------

// Cell Voltage Register
//----------------------------------------------------------------------
#define RDCVA				0b00000000100			// Read Cell 1 - 3
#define RDCVB				0b00000000110			// Read Cell 4 - 6
#define RDCVC				0b00000001000			// Read Cell 7 - 9
#define RDCVD				0b00000001010			// Read Cell 10 - 12
//----------------------------------------------------------------------

// Auxilliary Register
//----------------------------------------------------------------------
#define RDAUXA				0b00000001100			// Read Auxilliary Register Group A
#define RDAUXB				0b00000001110			// Read Auxilliary Register Group B
//----------------------------------------------------------------------

// Status Register
//----------------------------------------------------------------------
#define RDSTATA				0b00000010000			// Read Status Register Group A
#define RDSTATB				0b00000010010			// Read Status Register Group B
//----------------------------------------------------------------------

// Start Commands									// ADCOPT = 0	, ADCOPT = 1
//----------------------------------------------------------------------
#define MD2714				0b00010000000			// 27 kHz		, 14 kHz
#define MD73				0b00100000000			// 7 kHz		, 3 kHz
#define MD262				0b00110000000			// 26 Hz		, 2 kHz
//----------------------------------------------------------------------
#define ST1					0b00000100000			// Selftest 1
#define ST2					0b00001000000			// Selftest 2
//----------------------------------------------------------------------
#define DCP					0b00000010000			// Discharge Permitted
//----------------------------------------------------------------------

#define ADCVC				0b01001100000			// Start ADC Cell Conversion
//----------------------------------------------------------------------
#define CELLALL				0b00000000000			// Read All Cells
#define CELL17				0b00000000001			// Read Cell 1 & 7
#define CELL28				0b00000000010			// Read Cell 2 & 8
#define CELL39				0b00000000011			// Read Cell 3 & 9
#define CELL410				0b00000000100			// Read Cell 4 & 10
#define CELL511				0b00000000101			// Read Cell 5 & 11
#define CELL612				0b00000000110			// Read Cell 6 & 12
//----------------------------------------------------------------------

#define ADOW				0b01000101000			// Start ADC Open Wire Conversion
//----------------------------------------------------------------------
#define PUP					0b00001000000			// Pull-up Current
//----------------------------------------------------------------------

#define CVST				0b01000000111			// Start Self-Test Cell Conversion

#define ADAX				0b10001100000			// Start GPIO ADC Conversion
//----------------------------------------------------------------------
#define GPIOALL				0b00000000000			// Read All GPIO & Reference
#define GPIO1				0b00000000001			// Read GPIO 1
#define GPIO2				0b00000000010			// Read GPIO 2
#define GPIO3				0b00000000011			// Read GPIO 3
#define GPIO4				0b00000000100			// Read GPIO 4
#define GPIO5				0b00000000101			// Read GPIO 5
#define SECREF				0b00000000110			// Read Reference
//----------------------------------------------------------------------

#define AXST				0b10000000111			// Start Self-Test GPIO Conversion

#define ADSTAT				0b10001101000			// Start Status ADC Conversion
//----------------------------------------------------------------------
#define STATALL				0b00000000000			// Read All Status
#define STATSOC				0b00000000001			// Read SOC Status
#define STATITMP			0b00000000010			// Read ITMP Status
#define STATVA				0b00000000011			// Read VA Status
#define STATVD				0b00000000100			// Read VD Status
//----------------------------------------------------------------------

#define STATST				0b10000001111			// Start Self-Test Status ADC Conversion
#define ADCVAX				0b10001101111			// Start Combined Cell, GPIO1 & GPIO 2 Conversion
//----------------------------------------------------------------------

// Clear Commands
//----------------------------------------------------------------------
#define CLRCELL				0b11100010001			// Clear Cell Voltage Register Group
#define CLRAUX				0b11100010010			// Clear Auxilliary Register Group
#define CLRSTAT				0b11100010011			// Clear Status Register Group
//----------------------------------------------------------------------

// ADC Conversion Status und Diagnose Status
//----------------------------------------------------------------------
#define PLADC				0b11100010100			// Poll ADC Conversion Status Register
#define DIAGN				0b11100010101			// Diagnose Mux and Poll Register
//----------------------------------------------------------------------

// Communication Register
//----------------------------------------------------------------------
#define WRCOMM				0b11100100001			// Write Communication Register Group
#define RDCOMM				0b11100100010			// Read Communication Register Group
#define STCOMM				0b11100100011			// Start Communication I2C / SPI
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
uint8_t ltc6804_check(void);
void wakeup_ltc6804(void);
void ltc6804(uint16_t command);
void ltc6804_write(uint16_t command, uint8_t *data);
void ltc6804_read(uint16_t command, uint8_t *data);
uint16_t peccommand(uint16_t command);
uint16_t peclookup(uint8_t len,	uint8_t *data);
//----------------------------------------------------------------------
//void init_crc(void);								// Wird benötigt um Pec-Tabelle zu berechnen
//----------------------------------------------------------------------

#endif