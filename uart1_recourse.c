//----------------------------------------------------------------------
// Titel		:	UART C-Code
//----------------------------------------------------------------------
// Sprache		:	C
// Datum		:	10.09.2018
// Version		:	1.0
// Autor		:	Diveturtle93
// Projekt		:	BMS
//----------------------------------------------------------------------

#include <avr/io.h>
#include "uart_recourse.h"

// UART1 Initialisieren
//----------------------------------------------------------------------
void uart1_init(void)   
{
	// Ports definieren
	UART1_DDR |= (1<<UART1_TX);
	UART1_DDR &= ~(1<<UART1_RX);
	
	// Baudrate setzen
	UBRR1H = ((UBRR1_VAL>>8) & 0x0F);
	UBRR1L = (UBRR1_VAL & 0xFF);
	
	// Auswahl ob Double Speed gewählt
	if (USE_U2X1 && UART1_Asynchron_Mode)
	   UCSR1A |= (1 << U2X1);					// Double Speed gewählt und setzen
	else
	   UCSR1A &= ~(1 << U2X1);					// Double Speed nicht gewählt
	
	// Auswahl ob UART im Asynchronous Modus betrieben wird
	if (UART1_Asynchron_Mode)
		UCSR1C &= ~(1<<UMSEL1);					// Ggf. UMSEL10
	else
		UCSR1C |= (1<<UMSEL1);					// Ggf. UMSEL10
 
	// Hier weitere Initialisierungen (TX und/oder RX aktivieren, Modus setzen
	UCSR1B |= (1<<TXEN1) | (1<<RXEN1);			// UART TX einschalten
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);			// Asynchron 8N1
	
	// Receive Interrupt aktivieren
	if (UART1_Rx_Interrupt)
	{
		UCSR1B |= (1<<RXCIE1);
	}
	else
	{
		UCSR1B &= ~(1<<RXCIE1);
	}
}
//----------------------------------------------------------------------

// UART1 Senden
//----------------------------------------------------------------------
void uart1_send(uint8_t data)
{
	// Warten bis der Uart-Puffer leer ist
	while( !(UCSR1A & (1<<UDRE1)) );
	
	// Schreibe Data in Puffer
	UDR1 = data;
}
//----------------------------------------------------------------------

// UART1 Zeichenkette senden
//----------------------------------------------------------------------
void uart1_string(uint8_t *text)
{
	// So lange *text != '\0' also ungleich dem "String-Endezeichen(Terminator)"
	while (*text){
		uart1_send(*text);						// Zeichen senden
		text++;									// Variable hochzählen
	}
}
//----------------------------------------------------------------------

// Zeichen am UART1 empfangen
//----------------------------------------------------------------------
uint8_t uart1_getc(void)
{
	// warten bis Zeichen verfuegbar
	while (!(UCSR1A & (1<<RXC1)));
	
	// Zeichen aus UDR an Aufrufer zurueckgeben
	return UDR1;
}
//----------------------------------------------------------------------