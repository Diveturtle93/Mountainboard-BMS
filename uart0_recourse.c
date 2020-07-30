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

// UART0 Initialisieren
//----------------------------------------------------------------------
void uart0_init(void)   
{
	// Ports definieren
	UART0_DDR |= (1<<UART0_TX);
	UART0_DDR &= ~(1<<UART0_RX);
	
	// Baudrate setzen
	UBRR0H = ((UBRR0_VAL>>8) & 0x0F);
	UBRR0L = (UBRR0_VAL & 0xFF);
	
	// Auswahl ob Double Speed gewählt
	if (USE_U2X0 && UART0_Asynchron_Mode)
	   UCSR0A |= (1 << U2X0);					// Double Speed gewählt und setzen
	else
	   UCSR0A &= ~(1 << U2X0);					// Double Speed nicht gewählt
	
	// Auswahl ob UART im Asynchronous Modus betrieben wird
	if (UART0_Asynchron_Mode)
		UCSR0C &= ~(1<<UMSEL0);					// Ggf. UMSEL00
	else
		UCSR0C |= (1<<UMSEL0);					// Ggf. UMSEL00
 
	// Hier weitere Initialisierungen (TX und/oder RX aktivieren, Modus setzen
	UCSR0B |= (1<<TXEN0) | (1<<RXEN0);			// UART TX & RX einschalten
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);			// Asynchron 8N1
	
	// Receive Interrupt aktivieren
	if (UART0_Rx_Interrupt)
	{
		UCSR0B |= (1<<RXCIE0);
	}
	else
	{
		UCSR0B &= ~(1<<RXCIE0);
	}
}
//----------------------------------------------------------------------

// UART0 Senden
//----------------------------------------------------------------------
void uart0_send(uint8_t data)
{
	// Warten bis der Uart-Puffer leer ist
	while( !(UCSR0A & (1<<UDRE0)) );
	
	// Schreibe Data in Puffer
	UDR0 = data;
}
//----------------------------------------------------------------------

// UART0 Zeichenkette senden
//----------------------------------------------------------------------
void uart0_string(char *text)
{
	// So lange *text != '\0' also ungleich dem "String-Endezeichen(Terminator)"
	while (*text){
		uart0_send(*text);						// Zeichen senden
		text++;									// Variable hochzählen
	}
}
//----------------------------------------------------------------------

// Zeichen am UART0 empfangen
//----------------------------------------------------------------------
uint8_t uart0_getc(void)
{
	// warten bis Zeichen verfuegbar
	while (!(UCSR0A & (1<<RXC0)));
	
	// Zeichen aus UDR an Aufrufer zurueckgeben
	return UDR0;
}
//----------------------------------------------------------------------

//Sendet eine Zahl auf UART
//----------------------------------------------------------------------
void uart0_number_8(uint8_t number)
{
	uint8_t hunderter = 0, zehner = 0, einer = 0;

	hunderter = number/100;
	zehner = (number-hunderter*100)/10;
	einer = number-hunderter*100-zehner*10;

	uart0_send(hunderter+'0');
	uart0_send(zehner+'0');
	uart0_send(einer+'0');
}
//----------------------------------------------------------------------

//Sendet eine Zahl auf UART
//----------------------------------------------------------------------
void uart0_number_16(uint16_t number)
{
	uint8_t zehntausender = 0, tausender = 0, hunderter = 0, zehner = 0, einer = 0;

	zehntausender = number/10000;
	tausender = (number-zehntausender*10000)/1000;
	hunderter = (number-zehntausender*10000-tausender*1000)/100;
	zehner = (number-zehntausender*10000-tausender*1000-hunderter*100)/10;
	einer = number-zehntausender*10000-tausender*1000-hunderter*100-zehner*10;

	uart0_send(zehntausender+'0');
	uart0_send(tausender+'0');
	uart0_send(hunderter+'0');
	uart0_send(zehner+'0');
	uart0_send(einer+'0');
}
//----------------------------------------------------------------------