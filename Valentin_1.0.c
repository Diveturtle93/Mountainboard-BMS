/*
 * BMS.c
 *
 * Created: 26.03.2019 11:47:40
 * Author : Valentin
 */ 

#include <avr/io.h>
#include "ltc6804.h"
#include "spi_recourse.h"


#define _balancePrecision_ 	500		//in 100uV
#define _storrageVoltage_		38000	//in 100uV


uint16_t CellSpannung[12] = {0};
uint16_t balanceMask = 0;
uint8_t statusMask = 0;		//,,,,    ,,,     balancing till finnished,Discharge to storrage,balance aktiv,RTC activ
uint16_t min = 0;

int main(void)
{
	PRR = (PRTWI<<1)|(PRTIM2<<)|(PRTIM0<<1)|(PRTIM1<<1)|(PRSPI<<0)|(PRUSART0<<1)|(PRADC<<1);
	//Timer2; asyncron; langsam; overflow interrupt
	TCCR2B = (CS22<<1)|(CS21<<1)|(CS20<<1);
	TIMSK2 = (TOIE<<1);
	ASSR = (AS2<<1)
	
	
	spi_initmaster();
	if(ltc6804_check() != 0) while(1);
	
	
	
    /* Replace with your application code */
	getCells();
	getMin();
	
    while (1) 
    {
		if(statusMask&0x02 == 1)	{	//balance aktiv
			getCells();
			getMin();
			if( (balanceMask==0) && (statusMask&0x08) )		statusMask = statusMask&0x05;	//stop balance allow sleep
			setBalance();
		}
		
		if(statusMask&0x04 == 1)	{	//Discharge to storrage
			getCells();
			getMin();
			if(min < _storrageVoltage_)	{
				statusMask = (statusMask&0xFB) | 0x05;		// stop discharge start just balance
			} else {
				if(balanceMask == 0)	balanceMask = 0x0FFF;
				setBalance();
			}
		}
		
		
		
		if(statusMask==0) sleepDeep();	//nothing to do might as well save power
		if(statusMask==0x01) sleep();	//nothing to do might as well save power but leaf RTC running
		
		_delay_ms(1000*25); //25s
		
    }
}
void sleepDeep(void)	{
	SMCR = (SM2<<0)|(SM1<<1)|(SM0<<0)|(SE<<1);
	SLEEP();
}
void sleep(void)	{
	SMCR = (SM2<<0)|(SM1<<1)|(SM0<<1)|(SE<<1);
	SLEEP();
}
void getCells(void)	{
	uint8_t dataRAW[24] = {0};
	const uint16_t dataReg[4] = {RDCVA,RDCVB,RDCVC,RDCVD};
	
	//mesure
	ltc6804(ADCVC|MD73);
	
	
	//recieve data
	for(uint8_t i = 0;i<4;i++)	{
		for(int j = 0; j<10; j++)	{
			
			if(ltc6804_read(dataReg[i] ,&dataRAW[6*i])	{
				break;
			}
		}
	}
	
	//data conversion
	for(uint8_t i = 0; i<12; i++)	{
		CellSpannung[i] = ((uint16_t)dataRAW[i*2+1]<<8) + dataRAW[i*2];
		if(CellSpannung[i]<500) CellSpannung[i] = 0;
	}
	
	
	
}
void getMin(void)	{
	uint16_t threshhold = 0;
	min = 0xFFFF;
	balanceMask = 0;
	for(uint8_t i = 0; i<12; i++)	{
		if(CellSpannung[i])	{
			if(CellSpannung[i] < min)	{
				min = CellSpannung[i];
			}
		}
	}
	
	threshhold = min + _balancePrecision_;
	
	for(uint8_t i = 0; i<12; i++)	{
		if(CellSpannung[i] > min)	{
			balanceMask |= (1<<i);
		}
	}
}
void setBalance(void)	{
	uint8_t data[6] = {0xF8,0xCF,0x17,0xA4,0,0};
	data[4] = balanceMask;
	data[5] = 0x10 | ( (balanceMask>>8) & 0x0F );
	
	
	ltc6804_write(WRCFG, data);
}
ISR( TIMER2_OVF)	{
	
}