//----------------------------------------------------------------------
// Titel	:	Batteriemanagementsystem Mountainboard
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	11.01.2020
// Version	:	1.0
// Autor	:	Frederik Hafemeister
// Projekt	:	BMS Mountainboard
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
//uint16_t EEMEM eeData = 12345;
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
	uint8_t modus = 2;													// Zähler für Programm, 8 Bit
	uint16_t count = 0;													// Zähler für Programm, 16 Bit
	uint16_t temperatur[2] = {0};										// Array um Temperatur zu speichern
	uint16_t spannungen[12] = {0};										// Array um Spannungen zu speichern
	uint8_t stat[16] = {0};
	uint8_t data[32] = {0};
	uint16_t temp = 0, cmd = RDCFG;
	uint8_t min = 0, min_tmp = 0, max = 0, max_tmp = 0;					// Zelle mit Minimal und Maximal Spannung
	uint16_t V_min = 42000, V_max = 0, V_mean = 0;						// Minimal und Maximal Spannung
	
	// IO-Ports einstellen
	DDRB = 0xFF;														// Setzen Port B als Ausgang
	DDRC = 0xFF;														// Setzen Port C als Ausgang
	DDRD = 0xFF;														// Setzen Port D als Ausgang
	
	// Hardware konfigurieren
	spi_initmaster();													// Initialisiere SPI-Schnittstelle
	uart0_init();														// Initialisiere Serielle Schnittstelle
	
	uart0_string("\r\n");
	uart0_string("Starte System\r\n");
	
	if ((temp = ltc6804_check()) != 0)									// LTC6804 Selftest durchführen
	{
		uart0_string("Selftest failed\r\n");							// Ausgabe bei Fehlerhaftem Selbsttest
		TCCR1B = 0;														// Timer Stoppen
		PORTD |=  (1<<PIND7);											// Ausgabe auf LEDs
		uart0_number_16(temp);
		uart0_string("\r\n");
		
		return 0;														// Programm abbrechen
	}	
	
	init_Timer1();														// Initialisiere Timer 1 und schalte diesen ein
	
	sei();																// Globale Interrupts einschalten
	
	uart0_string("Selftest passed\r\n");
	
	// Alle Register zurücksetzen
	ltc6804(CLRCELL);
	ltc6804(CLRSTAT);
	ltc6804(CLRAUX);
	
	ltc6804(ADCVAX | MD73 | CELLALL);									// Initial Command Zellen auslesen
	
	// Starte Endlosschleife
	while(1)
	{
		// Uart legt Ladevorgang fest, Parameter c, d, s, t (Charge, Discharge, Stop, Status LTC3300)
		if (rx0_receive == 'c')
		{
			modus = 1;
			count = 1;
		}
		
		if (rx0_receive == 'd')
		{
			modus = 2;
			count = 1;
		}
		
		if (rx0_receive == 's')
		{
			modus = 0;
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
			ltc6804(ADCVC | MD2714 | CELLALL);
			ltc6804_read(RDCVA, &data[0]);
			ltc6804_read(RDCVB, &data[6]);
			ltc6804_read(RDCVC, &data[12]);
			ltc6804_read(RDCVD, &data[18]);
			
			for (uint8_t i = 0; i < 12; i++)
			{
				spannungen[i] = ((data[i*2+1]<<8) | data[i*2]);
				cmd = spannungen[i];
				
				/*if ((cmd <= 30000) && (modus == 2))
				{
					PORTD |= (1<<PIND6);								// Ausgabe auf LEDs
					uart0_string("Unterspannung erkannt\t");
					uart0_number_16(i+1);
					uart0_string("  ");
					uart0_number_16(cmd);
					uart0_string("\r\n");
										
					for (uint8_t j = 0; j < 12; j++)
					{
						cmd = spannungen[j];
						
						uart0_number_16(cmd);
						uart0_string("; ");
					}
					
					uart0_string("\r\n");
					//TCCR1B = 0;
					PORTD |= (1<<PIND6);								// Ausgabe auf LEDs
					//eeprom_write_word(&eeData, temp);
					//return 0;
				}
				else if ((cmd >= 41000) && (modus == 1))
				{
					PORTD |= (1<<PIND6);								// Ausgabe auf LEDs
					uart0_string("Uberspannung erkannt\t");
					uart0_number_16(i+1);
					uart0_string("  ");
					uart0_number_16(cmd);
					uart0_string("\r\n");
					
					for (uint8_t j = 0; j < 12; j++)
					{
						cmd = spannungen[j];
						
						uart0_number_16(cmd);
						uart0_string("; ");
					}
					
					uart0_string("\r\n");
					//TCCR1B = 0;
					PORTD |= (1<<PIND6);								// Ausgabe auf LEDs
					//eeprom_write_word(&eeData, temp);
					//return 0;
				}
				else
					PORTD &= ~(1<<PIND6);*/
			}
		}
		// Ende 500ms
		
		// Task wird alle 1s durchgeführt
		if ((count % 2000) == 0)
		{
			
		}
		// Ende 1s
		
		// Task wird alle 2000ms durchgeführt			(Zeit zum balancen muss kleiner 1,5s sein. Sonst bricht der IC ab)
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
				V_mean = V_mean + spannungen[i]/12;
			}
			
			//V_mean = ((stat[1] << 8) | stat[0]);
			//V_mean = ((V_mean / 3) * 5);									// Mittelwert bilden
			
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
		// Ende 2000ms
		
		// Task wird alle 10s durchgeführt
		if (((count % 2000) == 0) && (modus != 0))
		{
			temp++;
			
			for (uint8_t i = 0; i < 14; i++)
			{
				if (i >= 12)
				cmd = temperatur[i-12];
				else
				cmd = spannungen[i];
				
				uart0_string("; ");
				uart0_number_16(cmd);
			}
			
			uart0_string("; ");
			uart0_number_16(min+1);
			uart0_string("; ");
			uart0_number_16(max+1);
			uart0_string("\r\n");
		}
		// Ende 10s
		
		if(count == 10000)
		{
			
		}
	}
}
//----------------------------------------------------------------------