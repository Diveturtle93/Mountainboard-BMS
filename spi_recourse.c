//----------------------------------------------------------------------
// Titel	:	SPI C-Code
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	05.09.2018
// Version	:	1.0
// Autor	:	Frederik Hafemeister
// Projekt	:	BMS
//----------------------------------------------------------------------

#include <avr/io.h>
#include "spi_recourse.h"

// SPI als Master initialisieren
//----------------------------------------------------------------------
void spi_initmaster(void)
{	
	// Set MOSI, CS and SCK output, MISO input
	SPI_DDR |= (1<<SPI_MOSI) | (1<<SPI_SCK) | (1<<SPI_CS);
	SPI_DDR &= ~(1<<SPI_MISO);
	SPI_PORT |= (1<<SPI_CS);
	
	// Enable SPI, Master, set clock rate fck/16
	// LTC6820 1Mbps SPI (16 MHz / 16), Clock Idle High, Trailing Sample Edge
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0) | CLK_HIGH | Trail_Edge;
}
//----------------------------------------------------------------------

// SPI als Slave initialisieren
//----------------------------------------------------------------------
void spi_initslave(void)
{	
	// Set MOSI and SCK output, all others input
	SPI_DDR |= (1<<SPI_MOSI) | (1<<SPI_SCK);
	SPI_DDR &= ~((1<<SPI_MISO) | (1<<SPI_CS));
	
	// Enable SPI, Master, set clock rate fck/16
	SPCR = (1<<SPE) | (0<<MSTR) | (1<<SPR0) | CLK_HIGH;
}
//----------------------------------------------------------------------

// Data transmit zum Schreiben und Lesen
//----------------------------------------------------------------------
uint8_t spi_transmit(uint8_t data)
{
	// Start transmission
	SPDR = data;
	
	// Wait for transmission complete
	while(!(SPSR & (1<<SPIF)));

	// Data Receive
	return SPDR;
}
//----------------------------------------------------------------------