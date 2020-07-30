//----------------------------------------------------------------------
// Titel		:	UART h-Code
//----------------------------------------------------------------------
// Sprache		:	C
// Datum		:	10.09.2018
// Version		:	1.0
// Autor		:	Diveturtle93
// Projekt		:	BMS
//----------------------------------------------------------------------

#ifndef UART_RECOURSE_H
#define UART_RECOURSE_H

// Gegebenfalls F_CPU definieren
//----------------------------------------------------------------------
#ifndef F_CPU
	#define F_CPU 1000000UL
	#warning "F_CPU wurde von uart_recourse.h neu definiert."
#endif
//----------------------------------------------------------------------

// Ports definieren
//----------------------------------------------------------------------
#ifndef UART_PORTS
#define UART_PORTS

//----------------------------------------------------------------------
#ifdef UCSR0C
	#warning "UART0 kann verwendet werden"
	#define UART0_DDR				DDRD
	#define UART0_PORT				PORTD
	#define UART0_TX				PIND1
	#define UART0_RX				PIND0
#endif

#ifdef UCSR1C
	#warning "UART1 kann verwendet werden"
	#define UART1_DDR				DDRD
	#define UART1_PORT				PORTD
	#define UART1_TX				PIND3
	#define UART1_RX				PIND2
#endif
//----------------------------------------------------------------------
#endif
//----------------------------------------------------------------------

// Einstellungen definieren
//----------------------------------------------------------------------
#ifndef BAUD0
	#define  BAUD0 9600UL
	#warning "BAUD0 wurde von uart_recourse.h neu definiert."
#endif

#ifndef BAUD1
	#define  BAUD1 9600UL
	#warning "BAUD1 wurde von uart_recourse.h neu definiert."
#endif

#ifndef UMSEL0
	#define  UMSEL0					UMSEL00
	#warning "UMSEL0 wurde von UMSEL00 neu definiert"
#endif

#define USE_U2X0					1				// Double Speed of Transmission 0 = off, 1 = on
#define USE_U2X1					0				// Double Speed of Transmission 0 = off, 1 = on
#define UART0_Asynchron_Mode		1				// Asynchronous Mode 0 = off, 1 = on
#define UART1_Asynchron_Mode		0				// Asynchronous Mode 0 = off, 1 = on
#define UART0_Rx_Interrupt			1				// Receive Interrupt 0 = off, 1 = on
#define UART1_Rx_Interrupt			0				// Receive Interrupt 0 = off, 1 = on

//----------------------------------------------------------------------

// Berechnungen
//----------------------------------------------------------------------
#if USE_U2X0 && UART0_Asynchron_Mode
	#define UBRR0_VAL (F_CPU/(BAUD0*8)-1)			// clever runden
	#define BAUD0_REAL (F_CPU/(8*(UBRR0_VAL+1)))	// Reale Baudrate
	#define BAUD0_ERROR ((BAUD0_REAL*1000)/BAUD0)	// Fehler in Promille, 1000 = kein Fehler.
#elif UART0_Asynchron_Mode
	#define UBRR0_VAL (F_CPU/(BAUD0*16)-1)			// clever runden
	#define BAUD0_REAL (F_CPU/(16*(UBRR0_VAL+1)))	// Reale Baudrate
	#define BAUD0_ERROR ((BAUD0_REAL*1000)/BAUD0)	// Fehler in Promille, 1000 = kein Fehler.
#else
	#define UBRR0_VAL (F_CPU/(BAUD0*2)-1)			// clever runden
	#define BAUD0_REAL (F_CPU/(2*(UBRR0_VAL+1)))	// Reale Baudrate
	#define BAUD0_ERROR ((BAUD0_REAL*1000)/BAUD0)	// Fehler in Promille, 1000 = kein Fehler.
#endif
//----------------------------------------------------------------------
#if ((BAUD0_ERROR<990) || (BAUD0_ERROR>1010))
	#error Systematischer Fehler der Baudrate groesser 1% und damit zu hoch! 
#endif
//----------------------------------------------------------------------

#if USE_U2X1 && UART1_Asynchron_Mode
	#define UBRR1_VAL (F_CPU/(BAUD1*8)-1)			// clever runden
	#define BAUD1_REAL (F_CPU/(8*(UBRR1_VAL+1)))	// Reale Baudrate
	#define BAUD1_ERROR ((BAUD1_REAL*1000)/BAUD1)	// Fehler in Promille, 1000 = kein Fehler.
#elif UART1_Asynchron_Mode
	#define UBRR1_VAL (F_CPU/(BAUD1*16)-1)			// clever runden
	#define BAUD1_REAL (F_CPU/(16*(UBRR1_VAL+1)))	// Reale Baudrate
	#define BAUD1_ERROR ((BAUD1_REAL*1000)/BAUD1)	// Fehler in Promille, 1000 = kein Fehler.
#else
	#define UBRR1_VAL (F_CPU/(BAUD1*2)-1)			// clever runden
	#define BAUD1_REAL (F_CPU/(2*(UBRR1_VAL+1)))	// Reale Baudrate
	#define BAUD1_ERROR ((BAUD1_REAL*1000)/BAUD1)	// Fehler in Promille, 1000 = kein Fehler.
#endif
//----------------------------------------------------------------------
#if ((BAUD1_ERROR<990) || (BAUD1_ERROR>1010))
	#error Systematischer Fehler der Baudrate groesser 1% und damit zu hoch! 
#endif
//----------------------------------------------------------------------

// Funktionen definieren
//----------------------------------------------------------------------
void uart0_init(void);								// UART0 Initialisierung
void uart1_init(void);								// UART1 Initialisierung
void uart0_send(uint8_t data);						// UART0 Zeichen senden
void uart1_send(uint8_t data);						// UART1 Zeichen senden
void uart0_string(char *text);					// UART0 Zeichenkette senden
void uart1_string(uint8_t *text);					// UART1 Zeichenkette senden
uint8_t uart0_getc(void);							// UART0 Zeichen empfangen
uint8_t uart1_getc(void);							// UART1 Zeichen empfangen
void uart0_number_8(uint8_t number);				// UART0 Zahl umrechen und senden, 8-Bit
void uart1_number_8(uint8_t number);				// UART1 Zahl umrechen und senden, 8-Bit
void uart0_number_16(uint16_t number);				// UART0 Zahl umrechen und senden, 16-Bit
void uart1_number_16(uint16_t number);				// UART1 Zahl umrechen und senden, 16-Bit
//----------------------------------------------------------------------

#endif