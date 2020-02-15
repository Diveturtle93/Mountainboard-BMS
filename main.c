//----------------------------------------------------------------------
// Titel		:	Batteriemanagementsystem Mountainboard
//----------------------------------------------------------------------
// Sprache		:	C
// Datum		:	11.01.2020
// Version		:	1.0
// Autor		:	Frederik Hafemeister
// Projekt		:	BMS Mountainboard
// Controller	:	AtMega328P-AU
//----------------------------------------------------------------------

// Einfügen der Include-Dateien
//----------------------------------------------------------------------
// F_CPU muss vorher definiert werden, damit die Bibliothek (util/delay.h) funktioniert
#define F_CPU 1000000UL
#define BAUD0 9600UL
#define BAUD1 19200UL

// AVR Include-Dateien
//----------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
//----------------------------------------------------------------------

// Eigene Include-Dateien
//----------------------------------------------------------------------
#include "ltc6804.h"
#include "spi_recourse.h"
#include "uart_recourse.h"
#include "scource.h"
#include "eemem.h"
//----------------------------------------------------------------------

// Variablen definieren
//----------------------------------------------------------------------
volatile uint8_t millisekunden_flag_1 = 0;								// Flag um 1ms Task zu speichern und auszuführen
uint8_t rx0_receive;										    		// Variable um UART0 Daten zu speichern
//----------------------------------------------------------------------

// TIMER1 Overflow Interrupt-Routine
//----------------------------------------------------------------------
ISR(TIMER1_OVF_vect)
{
	millisekunden_flag_1 = 1;											// Nach auslösen des Interrupt setzen des Flags
	TCNT1 = 65535-16000;												// Vorladewert für den 1ms interrupt
}
//----------------------------------------------------------------------

// UART0 Receive Interrupt-Routine
//----------------------------------------------------------------------
ISR(USART_RX_vect)														// USART0 für ATmega128, USART für ATmega328
{
	rx0_receive = UDR0;													// Empfangene Daten in Variable speichern
}
//----------------------------------------------------------------------

// Hauptprogramm
//----------------------------------------------------------------------
int main(void)
{
	// Variablen definieren
	uint8_t modus = 2, eeCounter = 0;									// Zähler für Programm, 8 Bit
	uint16_t count = 0;													// Zähler für Programm, 16 Bit
	uint16_t temperatur[2] = {0};										// Array um Temperatur zu speichern
	uint16_t spannungen[12] = {0};										// Array um Spannungen zu speichern
	uint8_t data[32] = {0};												// Array um Daten zu übertragen
	uint16_t temp = 0, cmd = RDCFG;										// Temporäre Variablen
	uint8_t min = 0, max = 0;											// Zelle mit Minimal und Maximal Spannung
	uint16_t V_min = 42000, V_max = 0, V_mean = 0;						// Minimal, Maximal und Mittel Spannung
	
	// IO-Ports einstellen
	DDRB = 0x2C;														// Setzen SPI Leitungen von Port B
	DDRC = 0x00;														// Setzen Port C als Eingang
	DDRD = 0xE0;														// Oberen drei Bits von Port D als Ausgang
	
	// Hardware konfigurieren
	spi_initmaster();													// Initialisiere SPI-Schnittstelle
	uart0_init();														// Initialisiere Serielle Schnittstelle
	
	uart0_string("\r\n");
	uart0_string("Starte System\r\n");
	
	for (uint8_t i = 0; i < 3; i++)
	{
		if ((temp = ltc6804_check()) != 0)								// LTC6804 Selftest durchführen
		{
			uart0_string("Selftest failed\r\n");						// Ausgabe bei Fehlerhaftem Selbsttest
			TCCR1B = 0;													// Timer Stoppen
			
			eeCounter = eeprom_read_byte(&eeFehlerZaehler);				// Zähler für Fehlerspeicher auslesen
			eeprom_write_byte(&eeFehlerSpeicher[eeCounter], temp);		// Fehler speichern
			eeCounter--;
			eeprom_write_byte(&eeFehlerZaehler, eeCounter);				// Fehlerzähler herunterzählen
			return 0;													// Programm abbrechen nach drei Fehlversuchen
		}
		else
			break;														// Schleife abbrechen; normal weiter; Initialisierung LTC6804 erfolgreich
	}
	
	init_Timer1();														// Initialisiere Timer 1 und schalte diesen ein
	
	sei();																// Globale Interrupts einschalten
	
	uart0_string("Selftest passed\r\n");
	
	// Alle Register zurücksetzen
	ltc6804(CLRCELL);
	ltc6804(CLRSTAT);
	ltc6804(CLRAUX);
	
	ltc6804(ADCVAX | MD73 | CELLALL);									// Initial Command Zellen auslesen; Daten fallen lassen, da nicht benötigt
	
	// Starte Endlosschleife
	while(1)
	{
		// Uart legt Ladevorgang fest, Parameter c, d, s, t (Charge, Discharge, Stop, Status LTC3300)
		if (rx0_receive == 'c')
		{
			modus = 1;
		}
		rx0_receive = 0;
		
		// Task wird jede Millisekunde ausgeführt
		if (millisekunden_flag_1 == 1)
		{
			count++;													// Zähle count hoch
			millisekunden_flag_1 = 0;									// Setze Millisekunden-Flag zurück
		}
		
		// Task wird alle 500ms ausgeführt
		if ((count % 500) == 0)
		{
			ltc6804(ADCVC | MD2714 | CELLALL);							// Zellspannungen einlesen und in Register speichern
			ltc6804_read(RDCVA, &data[0]);								// Zellspannungen aus Register auslesen (Zelle 1 - 3)
			ltc6804_read(RDCVB, &data[6]);								// Zellspannungen aus Register auslesen (Zelle 4 - 6)
			ltc6804_read(RDCVC, &data[12]);								// Zellspannungen aus Register auslesen (Zelle 7 - 9)
			ltc6804_read(RDCVD, &data[18]);								// Zellspannungen aus Register auslesen (Zelle 10 - 12)
			
			for (uint8_t i = 0; i < 12; i++)
			{
				spannungen[i] = ((data[i*2+1]<<8) | data[i*2]);
				cmd = spannungen[i];
			}
		}
		// Ende 500ms
		
		// Task wird alle 1s durchgeführt
		if ((count % 1000) == 0)
		{
			
		}
		// Ende 1s
		
		// Task wird alle 2s durchgeführt			(Zeit zum balancen muss kleiner 1,5s sein. Sonst bricht der IC ab)
		if ((count % 2000) == 0)
		{
			V_max = 0;
			V_min = 42000;
			
			// Mittelwert der Spannungnen bilden
			//ltc6804(ADSTAT | MD73 | STATSOC);							// SOC messen
			//ltc6804_read(RDSTATA, &stat[0]);
			
			V_mean = 0;
			for (uint8_t i = 0; i < 12; i++)
			{
				V_mean = V_mean + spannungen[i];
			}
			V_mean = V_mean/12;
			
			// Zelle mit Min und Max Spannung herausfinden
			for (uint8_t i = 0; i < 12; i++)
			{
				if (spannungen[i] < V_min)
				{
					V_min = spannungen[i];
					min = i;
				}
				
				if (spannungen[i] > V_max)
				{
					V_max = spannungen[i];
					max = i;
				}
			}
		}
		// Ende 2s
		
		// Task wird alle 2s durchgeführt, unter der Bedingung das Serielle Ausgabe gewünscht ist
		if (((count % 2000) == 0) && (modus != 0))
		{
			for (uint8_t i = 0; i < 14; i++)
			{
				if (i >= 12)
					cmd = temperatur[i-12];
				else
					cmd = spannungen[i];
				
				uart0_number_16(cmd);
				uart0_string("; ");
			}
			
			uart0_number_16(min+1);
			uart0_string("; ");
			uart0_number_16(max+1);
			uart0_string("\r\n");
		}
		// Ende 2s
		
		// Task wird alle 10s durchgeführt
		if(count == 10000)
		{
			
		}
		// Ende 10s
	}
}
//----------------------------------------------------------------------