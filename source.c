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
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "source.h"

// Timer 1 initialisieren
//----------------------------------------------------------------------
void init_Timer1(void)
{
	TCCR1A = 0;
	TCCR1B = (1<<CS00);													// Timer1 an, prescaler = 1
	TIMSK1 = (1<<TOIE1);												// Timer1 Overflow Interrupt einschalten
	TCNT1 = 65535-16000;												// Vorladewert f�r den 1ms interrupt
}
//----------------------------------------------------------------------

// Timer 2 initialisieren und starten, Asynchron Modus, RTC
//----------------------------------------------------------------------
void Timer2_start(void)
{
	cli();																// Interrupts ausschalten zur initialisierung
	PRR &=  ~(1 << PRTIM2);												// Power down ausschalten
	
	//Timer2; asynchron; slow; overflow interrupt
	GTCCR |= (1 << PSRASY);												// Prescaler zur�cksetzen
	ASSR &= ~(1 << AS2);												// Asynchron Mode einschalten, Bit AS2 muss daf�r Null sein
	TCCR2A = 0;															// Keine Funktionen in der Signalform
	TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);					// Prescaler auf 1024 stellen
	TIMSK2 = (1 << TOIE2);												// Timer2 Overflow Interrupt einschalten
	TCNT2 = 0;															// Einstellung des TCNT2 Registers ist korrupt und muss neu beschrieben werden
	
	while((ASSR & 0x1F) != 0);											// Warten bis das ASSR Register beschrieben ist
	
	sei();																// Alle Interrupts wieder einschalten
}
//----------------------------------------------------------------------

// Sleep f�r Power Down; Oscillator enable, Timer2 Asyncron, INT0 & INT1, TWI und WDT wecken AVR
//----------------------------------------------------------------------
void sleep(void)
{
	SMCR = (0 << SM2) | (1 << SM1) | (1 << SM0);						// Sleep-Mode auf Power-Down �ndern
	sleep_mode();														// AVR in Sleep versetzen
}
//----------------------------------------------------------------------

// Sleep f�r Power Save; INT0 & INT1, TWI und WDT wecken AVR
//----------------------------------------------------------------------
void sleepDeep(void)
{
	SMCR = (0 << SM2) | (1 << SM1) | (0 << SM0);						// Sleep-Mode auf Power-Save �ndern
	sleep_mode();														// AVR in Sleep versetzen
}
//----------------------------------------------------------------------

// ADC f�r Spannungsmessung
//----------------------------------------------------------------------
void init_ADC(void)
{
	ADCSRA = (1<<ADEN) | (0<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);  		// ADC ein; Free Run; Prescaler=8 (1000000Hz/8=125kHz)
	ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR);						// Interne Referenzspannung 1,1V; Rechtsb�ndige Ausgabe; Channel = 0
	ADCSRA |= (1<<ADSC);												// Einzelermittlung der Spannung

	while (ADCSRA & (1<<ADSC));											// Warten bis Ermittlung abgeschlossen
}
//----------------------------------------------------------------------

// ADC-Wert zur�ckgeben
//----------------------------------------------------------------------
uint16_t get_ADC(uint8_t channel)
{
	ADCSRA |= (1<<ADSC);												// Einzelermittlung der Spannung
	ADMUX |= channel;													// Channel ausw�hlen
	
	while (ADCSRA & (1<<ADSC));											// Warten bis Ermittlung abgeschlossen
	
	//ADMUX &= 0b11100000;												// Channel wieder zur�cksetzen
	
	return ADC;															// ADC-Wert zur�ckgeben
}
//----------------------------------------------------------------------
