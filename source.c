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