//----------------------------------------------------------------------
// Titel		:	Source Datei h-Code
//----------------------------------------------------------------------
// Sprache		:	C
// Datum		:	15.02.2020
// Version		:	1.0
// Autor		:	Frederik Hafemeister
// Projekt		:	BMS-Mountainboard
// Controller	:	AtMega328P-AU
//----------------------------------------------------------------------

#ifndef SOURCE_H
#define SOURCE_H

// Pins definieren
//----------------------------------------------------------------------

// Port B
//----------------------------------------------------------------------


// Port C
//----------------------------------------------------------------------
#define PAD1								PC0
#define PAD2								PC1
#define PAD3								PC2
#define PAD4								PC3
#define PAD5								PC4
#define PAD6								PC5

// Port D
//----------------------------------------------------------------------
#define RXD									PD0
#define TXD									PD1
#define INTERRUPT0							PD2
#define INTERRUPT1							PD3
#define WATCH_Pin							PD4
#define MAX1811_SELI						PD5
#define MAX1811_SELV						PD6
#define MAX1811_En							PD7
//----------------------------------------------------------------------


// Funktionen definieren
//----------------------------------------------------------------------
void init_Timer1(void);
void Timer2_start(void);
void sleep(void);
void sleepDeep(void);
//----------------------------------------------------------------------

#endif