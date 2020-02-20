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
#include <avr/wdt.h>
//----------------------------------------------------------------------

// Eigene Include-Dateien
//----------------------------------------------------------------------
#include "ltc6804.h"
#include "spi_recourse.h"
#include "uart_recourse.h"
#include "source.h"
#include "eemem.h"
//----------------------------------------------------------------------

// Variablen definieren
//----------------------------------------------------------------------
volatile uint8_t millisekunden_flag_1 = 0;								// Flag um 1ms Task zu speichern und auszuführen
uint8_t rx0_receive = 0;									    		// Variable um UART0 Daten zu speichern
volatile uint8_t asyncronExtension = 0;									// Variable für TIMER2, Sleep-Mode
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

// TIMER2 Overflow Interrupt-Routine für Sleep-Mode
//----------------------------------------------------------------------
/*ISR(TIMER2_OVF_vect)
{

}*/
//----------------------------------------------------------------------

// Hauptprogramm
//----------------------------------------------------------------------
int main(void)
{
	// Unbenutzte Hardware abschalten (1 = Off, 0 = On) (S. 54); TWI, Timer0 und ADC abschalten, Timer2 in Asyncron Modus
	PRR = (1 << PRTWI) | (1 << PRTIM2) | (1 << PRTIM0);
	
	// Variablen definieren
	uint8_t modus = 2, eeCounter = 0;									// Zähler für Programm, 8 Bit
	uint16_t count = 0;													// Zähler für Programm, 16 Bit
	uint16_t temperatur[2] = {0};										// Array um Temperatur zu speichern
	uint16_t spannungen[12] = {0};										// Array um Spannungen zu speichern
	uint8_t data[32] = {0};												// Array um Daten zu übertragen
	uint16_t temp = 0;													// Temporäre Variablen
	uint8_t min = 0, max = 0;											// Zelle mit Minimal und Maximal Spannung
	uint16_t V_min = 42000, V_max = 0, V_mean = 0;						// Minimal, Maximal und Mittel Spannung
	float tmp;
	
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
		temp = ltc6804_check();											// LTC6804 Selbsttest durchführen
		if ((temp != 0) && (i == 2))									// Ausführen wenn Selbsttest dreimal fehlschlägt
		{
			uart0_string("Selbstest fehlgeschlagen\r\n");				// Ausgabe bei Fehlerhaftem Selbsttest
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
	
	uart0_string("Selbsttest bestanden\r\n");
	
	// Alle Register zurücksetzen
	ltc6804(CLRCELL);
	ltc6804(CLRSTAT);
	ltc6804(CLRAUX);
	
	ltc6804(ADCVAX | MD73 | CELLALL);									// Initial Command Zellen auslesen; Daten fallen lassen, da nicht benötigt
	
	//wdt_enable(WDTO_2S);												// Watchdog einschalten auf 2s
	
	// Starte Endlosschleife
	while(1)
	{
		// UART legt Modus fest, Parameter t (Ausgabe Zellspannungen UART)
		if (rx0_receive == 't')
		{
			modus = 1;													// Modus 1
			rx0_receive = 0;											// Receive Variable zurücksetzen
		}
		
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
				spannungen[i] = ((data[i*2+1]<<8) | data[i*2]);			// Daten sortieren und in Array Spannung speichern
			}
		}
		// Ende 500ms
		
		// Task wird alle 1s durchgeführt
		if ((count % 1000) == 0)
		{
			temp = get_ADC();
			tmp = (1100.0*1024.0)/temp;
			
			if (tmp <= 319.0)
				PORTD |= (1<<PIND0);
			else
				PORTD &= ~(1<<PIND0);
		}
		// Ende 1s
		
		// Task wird alle 2s durchgeführt			(Zeit zum balancen muss kleiner 1,5s sein. Sonst bricht der IC ab)
		if ((count % 2000) == 0)
		{
			V_max = 0;													// V_max auf 0 setzen um Maximalspannung zu ermitteln
			V_min = 42000;												// V_min auf 42000 setzen um Minimalspannung zu ermitteln
			
			// Mittelwert der Spannungnen bilden
			//ltc6804(ADSTAT | MD73 | STATSOC);							// SOC messen
			//ltc6804_read(RDSTATA, &stat[0]);
			
			V_mean = 0;													// V_mean auf 0 setzen um Mittelwertspannung zu ermitteln
			for (uint8_t i = 0; i < 12; i++)
			{
				V_mean += spannungen[i];								// Zellspannungen aufaddieren
			}
			V_mean = V_mean/12;											// Mittelwert der 12 Zellen berechnen
			
			// Zelle mit Min und Max Spannung herausfinden
			for (uint8_t i = 0; i < 12; i++)
			{
				if (spannungen[i] < V_min)								// Zellspannugen mit V_min vergleichen
				{
					V_min = spannungen[i];								// Wenn Zellsapnnug kleiner V_min ist, V_min überschreiben
					min = i;											// Minimalzelle zwischenspeichern
				}
				
				if (spannungen[i] > V_max)								// Zellspannugen mit V_max vergleichen
				{
					V_max = spannungen[i];								// Wenn Zellsapnnug größer V_max ist, V_max überschreiben
					max = i;											// Maximalzelle zwischenspeichern
				}
			}
		}
		// Ende 2s
		
		// Task wird alle 2s durchgeführt, unter der Bedingung das Serielle Ausgabe gewünscht ist
		if (((count % 2000) == 0) && (modus == 1))
		{
			for (uint8_t i = 0; i < 14; i++)							// Zähler hochzählen um Arraywert auszuwählen
			{
				if (i >= 12)
					temp = temperatur[i-12];							// Wenn Zählerwert größer 12 ist Temperaturwert auswählen
				else
					temp = spannungen[i];								// Solange Zählerwert kleiner 12 ist Zellspannung auswählen
				
				uart0_number_16(temp);									// Spannungs- oder Temperaturwert ausgeben
				uart0_string("; ");
			}
			
			uart0_number_16(min+1);										// Zelle mit Minimalspannung ausgeben
			uart0_string("; ");
			uart0_number_16(max+1);										// Zelle mit Maximalspannung ausgeben
			uart0_string("\r\n");
		}
		// Ende 2s
		
		// Task wird alle 10s durchgeführt
		if(count == 10000)
		{
			count = 0;													// Timer Counter zurücksetzen
		}
		// Ende 10s
		
		//wdt_reset();													// Watchdog zurücksetzen
	}
}
//----------------------------------------------------------------------