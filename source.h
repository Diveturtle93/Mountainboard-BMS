//----------------------------------------------------------------------
// Titel		:	Source Datei h-Code
//----------------------------------------------------------------------
// Sprache		:	C
// Datum		:	15.02.2020
// Version		:	1.0
// Autor		:	Diveturtle93
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

// Konstanten definieren
//----------------------------------------------------------------------
#define ADC_CH0								0
#define ADC_CH1								1
#define ADC_CH2								2
#define ADC_CH3								3
#define ADC_CH4								4
#define ADC_CH5								5
#define ADC_CH6								6
#define ADC_CH7								7
#define ADC_Temp							8
#define ADC_Batt							14
//----------------------------------------------------------------------


// Funktionen definieren
//----------------------------------------------------------------------
void init_Timer1(void);
void Timer2_start(void);
void sleep(void);
void sleepDeep(void);
void init_ADC(void);
uint16_t get_ADC(uint8_t channel);
//----------------------------------------------------------------------

#endif
