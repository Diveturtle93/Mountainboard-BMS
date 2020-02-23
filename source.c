//----------------------------------------------------------------------
// Titel	:	Scource Datei C-Code
//----------------------------------------------------------------------
// Sprache	:	C
// Datum	:	30.10.2018
// Version	:	1.0
// Autor	:	Frederik Hafemeister
// Projekt	:	BMS
//----------------------------------------------------------------------

#include <avr/io.h>
#include "source.h"

// Timer 1 initialisieren
//----------------------------------------------------------------------
void init_Timer1(void)
{
	TCCR1A = 0;
	TCCR1B = (1<<CS00);													// Timer1 an, prescaler = 1
	TIMSK1 = (1<<TOIE1);												// Timer1 Overflow Interrupt einschalten
	TCNT1 = 65535-16000;												// Vorladewert für den 1ms interrupt
}
//----------------------------------------------------------------------

// ADC für Spannungsmessung
//----------------------------------------------------------------------
void init_ADC(void)
{
	ADCSRA = (1<<ADEN) | (0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);  		// ADC ein; Free Run; Prescaler=8 (1000000Hz/8=125kHz)
	ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR);						// Interne Referenzspannung 1,1V; Rechtsbündige Ausgabe; Channel = 0
	ADCSRA |= (1<<ADSC);												// Einzelermittlung der Spannung

	while (ADCSRA & (1<<ADSC));											// Warten bis Ermittlung abgeschlossen
}
//----------------------------------------------------------------------

// ADC-Wert zurückgeben
//----------------------------------------------------------------------
uint16_t get_ADC(uint8_t channel)
{
	ADCSRA |= (1<<ADSC);												// Einzelermittlung der Spannung
	ADMUX |= channel;													// Channel auswählen
	
	while (ADCSRA & (1<<ADSC));											// Warten bis Ermittlung abgeschlossen
	
	//ADMUX &= 0b11100000;												// Channel wieder zurücksetzen
	
	return ADC;															// ADC-Wert zurückgeben
}
//----------------------------------------------------------------------