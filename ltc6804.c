//----------------------------------------------------------------------
// Titel		:	LTC6804 C-Code
//----------------------------------------------------------------------
// Sprache		:	C
// Datum		:	05.12.2018
// Version		:	1.1
// Autor		:	Diveturtle93
// Projekt		:	BMS
//----------------------------------------------------------------------

#include <avr/io.h>
#include "ltc6804.h"
#include "spi_recourse.h"

//----------------------------------------------------------------------
#ifndef F_CPU
	#warning "F_CPU wurde in LTC6804.c neu definiert"
	#define F_CPU 1000000UL
#endif

#include <util/delay.h>
//----------------------------------------------------------------------

// Lookup Tabelle einbinden
//----------------------------------------------------------------------
#include "ltc6804_progmem.h"
#define pecTable pec15Table
//----------------------------------------------------------------------


// Wakeup LTC6804
//----------------------------------------------------------------------
void wakeup_ltc6804(void)
{
	CS_ENABLE();
	_delay_ms(2);										//isoSPI braucht Zeit bis ready
	CS_DISABLE();
}
//----------------------------------------------------------------------

// LTC6804 Status auslesen und auswerten
//----------------------------------------------------------------------
uint8_t ltc6804_check(void)
{
	uint8_t tmp_data[50] = {0}, result = 0;
	uint16_t temp = 0;
	
	// Verzögerungszeit zum wecken des LTC6804
	wakeup_ltc6804();
	
	// Commands für Status senden  Test 1
	ltc6804(CVST | MD73 | ST1);
	_delay_ms(300);
	ltc6804(AXST | MD73 | ST1);
	_delay_ms(300);
	ltc6804(STATST | MD73 | ST1);
	_delay_ms(300);
	
	// Register auslesen Test 1
	ltc6804_read(RDCVA, &tmp_data[0]);
	ltc6804_read(RDCVB, &tmp_data[6]);
	ltc6804_read(RDCVC, &tmp_data[12]);
	ltc6804_read(RDCVD, &tmp_data[18]);
	
	ltc6804_read(RDAUXA, &tmp_data[24]);
	ltc6804_read(RDAUXB, &tmp_data[30]);
	
	ltc6804_read(RDSTATA, &tmp_data[36]);
	ltc6804_read(RDSTATB, &tmp_data[42]);
	
	// Daten prüfen Test 1
	for (uint8_t i=0; i<22; i++)
	{
		temp = ((tmp_data[i*2+1]<<8)|tmp_data[i*2]);
		if (temp != 0x9555)
			result = 1;
	}
	
	// Commands für Status senden Test 2
	ltc6804(CVST | MD73 | ST2);
	_delay_ms(300);
	ltc6804(AXST | MD73 | ST2);
	_delay_ms(300);
	ltc6804(STATST | MD73 | ST2);
	_delay_ms(300);
	
	// Register auslesen Test 2
	ltc6804_read(RDCVA, &tmp_data[0]);
	ltc6804_read(RDCVB, &tmp_data[6]);
	ltc6804_read(RDCVC, &tmp_data[12]);
	ltc6804_read(RDCVD, &tmp_data[18]);
	
	ltc6804_read(RDAUXA, &tmp_data[24]);
	ltc6804_read(RDAUXB, &tmp_data[30]);
	
	ltc6804_read(RDSTATA, &tmp_data[36]);
	ltc6804_read(RDSTATB, &tmp_data[42]);
	
	// Daten prüfen Test 2
	for (uint8_t i=0; i<22; i++)
	{
		temp = ((tmp_data[i*2+1]<<8)|tmp_data[i*2]);
		if (temp != 0x6AAA)
			result |= 2;
	}
	
	ltc6804(DIAGN);
	wakeup_ltc6804();
	ltc6804_read(RDSTATB, &tmp_data[0]);
	
	if (tmp_data[5] & (1<<1))
	{
		result |= 4;
	}
	
	/*// Command für OpenWire PUP = 1
	ltc6804(ADOW | MD73 | PUP);
	
	// Register auslesen OpenWire
	ltc6804_read(RDCVA, &tmp_data[0]);
	ltc6804_read(RDCVB, &tmp_data[6]);
	ltc6804_read(RDCVC, &tmp_data[12]);
	ltc6804_read(RDCVD, &tmp_data[18]);
	
	// Command für OpenWire PUP = 0
	ltc6804(ADOW | MD73);
	
	// Register auslesen OpenWire
	ltc6804_read(RDCVA, &tmp_data[24]);
	ltc6804_read(RDCVB, &tmp_data[30]);
	ltc6804_read(RDCVC, &tmp_data[36]);
	ltc6804_read(RDCVD, &tmp_data[42]);*/
	
	return result;										// return result
}
//----------------------------------------------------------------------

// Broadcast Command
//----------------------------------------------------------------------
void ltc6804(uint16_t command)
{
	// PEC berechnen, Anhand Command
	uint16_t pec;
	pec = peccommand(command);
	
	// Verzögerungszeit zum wecken des LTC6804
	wakeup_ltc6804();
	
	// Command übertragen
	CS_ENABLE();
	spi_transmit((command>>8) & 0x07);
	spi_transmit(command & 0xFF);
	spi_transmit((pec>>8) & 0xFF);
	spi_transmit(pec & 0xFE);
	
	// Wenn Command = STCOMM ist dann müssen noch 72 Takte übertragen werden
	if ((command == STCOMM) || (command == ADSTAT))
	{
		// 72 = 9 * 8 Bit Daten
		for (uint8_t i = 0; i < 9; i++)
		{
			// Dummy-Byte übertragen
			spi_transmit(0xFF);
		}
	}
	CS_DISABLE();
	// Ende der Übertragung
}
//----------------------------------------------------------------------

// Broadcast Write Command
//----------------------------------------------------------------------
void ltc6804_write(uint16_t command, uint8_t* data)
{
	// PEC berechnen, für Data Funktion nur bei einem Device gegeben
	uint16_t pec_c, pec_d;
	pec_c = peccommand(command);
	pec_d = peclookup(6, data);
	
	// Verzögerungszeit zum wecken des LTC6804
	wakeup_ltc6804();
	
	// Command übertragen
	CS_ENABLE();
	
	// Command fuer zu beschreibendes Register senden
	spi_transmit((command>>8) & 0x07);
	spi_transmit(command & 0xFF);
	spi_transmit((pec_c>>8) & 0xFF);
	spi_transmit(pec_c & 0xFE);
	
	// Data senden
	for (uint8_t i = 0; i < 6; i++)
	{
		spi_transmit(data[i]);
	}
	spi_transmit((pec_d>>8) & 0xFF);
	spi_transmit(pec_d & 0xFE);
	CS_DISABLE();
	// Ende der Übertragung
}
//----------------------------------------------------------------------

// Broadcast Read Command
//----------------------------------------------------------------------
void ltc6804_read(uint16_t command, uint8_t* data)
{
	// PEC berechnen, Anhand Command
	uint16_t pec;
	pec = peccommand(command);
	
	// Verzögerungszeit zum wecken des LTC6804
	wakeup_ltc6804();
	
	// Command übertragen
	CS_ENABLE();
	
	// Command fuer zu lesendes Register senden
	spi_transmit((command>>8) & 0x07);
	spi_transmit(command & 0xFF);
	spi_transmit((pec>>8) & 0xFF);
	spi_transmit(pec & 0xFE);
	
	// Data empfangen
	for (uint8_t i = 0; i < 8; i++)
	{
		// Dummy Byte senden
		data[i] = spi_transmit(0xFF);
	}
	CS_DISABLE();
	// Ende der Übertragung
}
//----------------------------------------------------------------------

// Pec Command bauen
//----------------------------------------------------------------------
uint16_t peccommand(uint16_t command)
{
	uint8_t pec[2];													// pec = Zwischenspeicher 16-Bit Command in 2x 8-Bit Bytes
	
	pec[1] = (command & 0xFF);										// pec[1] = lower Command Byte
	pec[0] = ((command >> 8) & 0x07);								// pec[0] = upper Command Byte
	
	return peclookup(2, pec);
}
//----------------------------------------------------------------------

// Pec kalculieren (Datasheet ltc6804 Page 65, Datasheet ltc6811 Page 72)
//----------------------------------------------------------------------
uint16_t peclookup(uint8_t len,	uint8_t *data)						// len = Anzahl Byte, data = Daten fuer die Pec ausgewaehlt wird
{
	uint16_t var, remainder, addr, xdata;							// remainder = Zwischenspeicher Pec, addr = Zwischenspeicher Addresse, xdata = Data aus pecTable, var = Zwischenspeicher Addresse in pecTable
	remainder = 16;													// Initialisiere reminder mit 16 (0b0000000000010000)
	
	// Schleife fuer die Pec-Calculation
	for(uint8_t i = 0; i<len;i++)
	{
		// Kalkuliere Pec Anhand der Lookuptabelle
		addr = ((remainder >> 7) ^ data[i]) & 0xFF;					// Speicheraddresse berechnen
		var = (uint16_t)(&pecTable[addr]);							// Speicheraddresse auslesen
		xdata = pgm_read_byte(var);									// Lower Byte aus Progmem laden
		xdata |= (pgm_read_byte(var + 1)<<8);						// upper Byte aus Progmem laden
		remainder = (remainder << 8) ^ xdata;						// Pec berechnen
	}
	
	return (remainder << 1);										// Der Pec hat eine 0 als LSB, remainder muss um 1 nach links geshiftet werden
}
//----------------------------------------------------------------------

/*
//----------------------------------------------------------------------
uint16_t test[256];
// CRC Tabelle berechnen (Datasheet ltc6804 Page 65)
//----------------------------------------------------------------------
void init_crc(void)
{
	uint16_t test_crc = 0x4599;
	uint16_t remainder;
	for (int i = 0; i < 256; i++)
	{
		remainder = i << 7;
		for (int bit = 8; bit > 0; --bit)
		{
			if (remainder & 0x4000)
			{
				remainder = ((remainder << 1));
				remainder = (remainder ^ test_crc);
			}
			else
			{
				remainder = ((remainder << 1));
			}
		}
		test[i] = remainder&0xFFFF;
	}
}
//----------------------------------------------------------------------*/