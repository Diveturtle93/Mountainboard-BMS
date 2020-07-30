//----------------------------------------------------------------------
// Titel		:	SPI h-Code
//----------------------------------------------------------------------
// Sprache		:	C
// Datum		:	05.09.2018
// Version		:	1.0
// Autor		:	Diveturtle93
// Projekt		:	BMS
//----------------------------------------------------------------------

#ifndef SPI_RECOURSE_H
#define SPI_RECOURSE_H

// Ports definieren
//----------------------------------------------------------------------
#ifndef SPI_PORTS
#define SPI_PORTS

//----------------------------------------------------------------------
#define SPI_DDR					DDRB
#define SPI_PORT				PORTB
#define SPI_MOSI				PINB3
#define SPI_MISO				PINB4
#define SPI_SCK					PINB5
#define SPI_CS					PINB2
//----------------------------------------------------------------------
#endif
//----------------------------------------------------------------------

// Definiere Chip-Select Leitung
//----------------------------------------------------------------------
#define CS_ENABLE() (SPI_PORT &= ~(1<<SPI_CS))				// Chip-Select enable
#define CS_DISABLE() (SPI_PORT |= (1<<SPI_CS))				// Chip-Select disable
//----------------------------------------------------------------------

// Definiere SPI-Einstellungen, LEADING EDGE = Führende Kante, TRAILING EDGE = Hinter Kante
//----------------------------------------------------------------------
#define CLK_HIGH				(1<<CPOL)					// CLK Idle High
#define CLK_LOW					(~(1<<CPOL))				// CLK Idle Low
#define Lead_Edge				(~(1<<CPHA))				// Sample on Leading Edge
#define Trail_Edge				(1<<CPHA)					// Sample on Trailing Edge
#define Start_LSB				(1<<DORD)					// LSB zuerst senden
#define Start_MSB				(~(1<<DORD))				// MSB zuerst senden
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
void spi_initmaster(void);
void spi_initslave(void);
uint8_t spi_transmit(uint8_t data);
//----------------------------------------------------------------------

#endif