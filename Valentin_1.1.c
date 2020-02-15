/*
 * BMS.c
 *
 * Created: 26.03.2019 11:47:40
 * Author : Valentin
 */ 

#define F_CPU 1000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ltc6804.h"
#include "spi_recourse.h"
#include <util/delay.h>
#include <avr/sleep.h>


#define _balancePrecision_ 	00050	//in 100uV
#define _storrageVoltage_		38000	//in 100uV
#define _StorrageTime_		226800	// *8 sec (226800 fuer 3 wochen); MAX 2^32-1



void sleepDeep(void);
void sleep(void);
void t2start(void);
void t2stop(void);
void selfchargestart(void);
void selfchargestop(void);

void getCells(void);
void getMin(void);
void setBalance(void);



uint16_t CellSpannung[12] = {0};
uint16_t balanceMask = 0;	
static uint8_t statusMask = 0;			//,,self charge ,start rtc request,    ,,,     balancing till finnished, Discharge to storrage, balance aktiv, RTC activ
uint16_t min = 0;				//momentan kleinste cell spannung
volatile uint32_t asyncronExtension = 0;	//more bits to preescale RTC

int main(void)
{
	PRR = (1<<PRTWI)|(0<<PRTIM2)|(1<<PRTIM0)|(1<<PRTIM1)|(0<<PRSPI)|(1<<PRUSART0)|(1<<PRADC);
	DDRB = 0;
	DDRC = (1<<PORTC1)|(1<<PORTC4);
	PORTC = 0b00010010;
	DDRD = 0b11100000;
	PORTD = 0b00000000;
	
	
	spi_initmaster();
	//if(ltc6804_check() != 0) while(1);
	
	
	
    /* Replace with your application code */
	getCells();
	getMin();
	//selfchargestop();
	t2start();
	sei();
    while (1) 
    {
		if((statusMask & 0x02) == 0x02)	{	//balance aktiv
			getCells();
			getMin();
			if( (balanceMask == 0) && (statusMask & 0x08) )		statusMask = statusMask & ~(0x06);	//stop balance allow sleep
			setBalance();
			_delay_ms(500);
		}
		
		if((statusMask & 0x04) == 0x04)	{	//Discharge to storrage
			getCells();
			getMin();
			if(min < _storrageVoltage_)	{
				statusMask = (statusMask & 0xFB) | 0x06;		// stop discharge start just balance
			} else {
				if(balanceMask == 0)	balanceMask = 0x0FFF;
				setBalance();
				_delay_ms(500);
			}
		}
		
		if(asyncronExtension > _StorrageTime_)	{
			t2stop();				//disable t2 to save power
			asyncronExtension = 0;	//reset counter
			statusMask |= 0x04;		//start Discharge
		}
		
		if((statusMask & 0x10) == 0x10)	{	//start RTC request
			statusMask &= 0xEF;				//reset flag
			if((statusMask & 0x01) == 0x00)	{	//start t2 if not running
				t2start();
			} 
		}
		
		//_delay_ms(1000*1); //1s
		
		if(statusMask==0) sleepDeep();	//nothing to do might as well save power
		if(statusMask==0x01) sleep();	//nothing to do might as well save power but leaf RTC running
		
		
    }
}
void sleepDeep(void)	{
	SMCR = (0<<SM2)|(1<<SM1)|(0<<SM0);
	sleep_mode();
}
void sleep(void)	{
	SMCR = (0<<SM2)|(1<<SM1)|(1<<SM0);
	sleep_mode();
}
void t2start(void)	{	//full init and start of assyncron timer 2 for rtc
	statusMask |= 0x01;			//prevent timer from being stoped from sleepmode
	
	PRR = PRR&(~(1<<PRTIM2));	//disable power reduction
	//Timer2; asyncron; langsam; overflow interrupt
	TCNT2 = 0;									//clear timer value
	GTCCR = (1<<PSRASY);						//clear prescaler
	TCCR2A = 0;									//no waveform generation 
	TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);		//set prescaler to 1024
	TIMSK2 = (1<<TOIE2);							//enable overflow interupt
	ASSR = (1<<AS2);							//set clock input to ext osc
	GTCCR = (1<<PSRASY);						//clear prescaler
	TCNT2 = 0;									//seting timer to asyncron might corupt this reg
	
}
void t2stop(void)	{	//stop and disable T2 to save energie
	statusMask &= 0xFE;		//alow deep sleep

	TIMSK2 = (0<<TOIE2);							//disable overflow interupt
	ASSR = (0<<AS2);							//set clock input to int clk
	TCCR2B = (0<<CS22)|(0<<CS21)|(0<<CS20);		//set prescaler to 0
	GTCCR = (1<<PSRASY);						//clear prescaler
	TCNT2 = 0;									//clear timer value
	
	PRR |= (1<<PRTIM2);							//reduce power
}
void selfchargestart(void)	{
	PORTD |= (1<<PORTD7);
	PORTC &= ~(1<<PORTC1);
	statusMask |= 0x20;
}
void selfchargestop(void)	{
	PORTD &= ~(1<<PORTD7);
	PORTC |= (1<<PORTC1);
	statusMask &= ~0x20;
}
void ADCon(void)	{
	PRR &= ~(1<<PRADC);
	ADMUX = (0<<REFS1)|(1<<REFS0)|0x0E;
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2);
}
void ADCoff(void)	{
	ADCSRA &= ~(1<<ADEN);
	PRR |= (1<<PRADC);
}
void ADCstart(void)	{
	ADCSRA |= (1<<ADSC);
}



void getCells(void)	{
	uint8_t dataRAW[24] = {0};
	const uint16_t dataReg[4] = {RDCVA,RDCVB,RDCVC,RDCVD};
	
	//mesure
	ltc6804(ADCVC|MD73);
	
	
	//recieve data
	for(uint8_t i = 0;i<4;i++)	{
		for(int j = 0; j<10; j++)	{
			
			if(ltc6804_read(dataReg[i] ,&dataRAW[6*i]))	{
				j=11;	//break
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
		if(CellSpannung[i] > threshhold)	{
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


ISR( TIMER2_OVF_vect )	{
	asyncronExtension++;
	if(asyncronExtension%2)	{
		PORTC &= ~(1<<PORTC4);
	} else {
		PORTC |= (1<<PORTC4);
	}
}
ISR( PCINT1_vect  )	{		//noch zu setzen
	if((PINC&(1<<PINC0)) == 0 )	{
		statusMask |= 0x02; //balance
	}
	if((PINC&(1<<PINC0)) == 1 )	{
		statusMask |= 0x18;		//rtc start req ; stop balancing when finished
	}
	
	
}
ISR( ADC_vect )	{
	uint16_t res = ADC;		//read register
	res = 56320 / res;		//1,1V(Vref) * 1024(ADCres) * 50(ergebnis in 20mV) / ADC ergebnis		// anwendung benötigt keine koreckte rundung
	uint8_t send = (res>0xFF) ? 0xFF : res;		//alles über 5,1 V wird als 0xFF übertragen
	
}