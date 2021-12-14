/*	Author: Nathan Lee
 *  Partner(s) Name: none
 *	Lab Section: 022
 *	Assignment: Lab #11  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
//#include "bit.h"
//#include "keypad.h"
#endif

volatile unsigned char TimerFlag = 0; 
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
    TCCR1B = 0x0B;
    OCR1A = 125;
    TIMSK1 = 0x02;
    TCNT1 = 0;
    _avr_timer_cntcurr = _avr_timer_M;
    SREG |= 0x80;
}
void TimerOff(){
    TCCR1B = 0x00;
}
void TimerISR(){
    TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect){
    _avr_timer_cntcurr--;
    if(_avr_timer_cntcurr == 0){
        TimerISR();
        _avr_timer_cntcurr = _avr_timer_M;
    }
}
void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

////////////////////////////////////////////////////////////////////////////////
//Functionality - Sets bit on a PORTx
//Parameter: Takes in a uChar for a PORTx, the pin number and the binary value 
//Returns: The new value of the PORTx
unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value) 
{
    return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

////////////////////////////////////////////////////////////////////////////////
//Functionality - Gets bit from a PINx
//Parameter: Takes in a uChar for a PINx and the pin number
//Returns: The value of the PINx
unsigned char GetBit(unsigned char port, unsigned char number) 
{
    return ( port & (0x01 << number) );
}

typedef struct task{
    signed char state;
    unsigned long int period;
    unsigned long int elapsedTime;
    int (*TickFct)(int);
} task;

unsigned char GetKeypadKey() {
    PORTC = 0xEF;
    asm("nop");
    if(GetBit(PINC,0)==0){return('1');}
    if(GetBit(PINC,1)==0){return('4');}
    if(GetBit(PINC,2)==0){return('7');}
    if(GetBit(PINC,3)==0){return('*');}

    PORTC = 0xDF;
    asm("nop");
    if(GetBit(PINC,0)==0){return('2');}
    if(GetBit(PINC,1)==0){return('5');}
    if(GetBit(PINC,2)==0){return('8');}
    if(GetBit(PINC,3)==0){return('0');}

    PORTC = 0xBF;
    asm("nop");
    if(GetBit(PINC,0)==0){return('3');}
    if(GetBit(PINC,1)==0){return('6');}
    if(GetBit(PINC,2)==0){return('9');}
    if(GetBit(PINC,3)==0){return('#');}

    PORTC = 0x7F;
    asm("nop");
    if(GetBit(PINC,0)==0){return('A');}
    if(GetBit(PINC,1)==0){return('B');}
    if(GetBit(PINC,2)==0){return('C');}
    if(GetBit(PINC,3)==0){return('D');}
    return('\0');
}

enum Get_States { GStart, GetKey};
unsigned char currKey = '\0';

int GetSMTick(int state){
    switch(state){
        case GStart:
            state = GetKey;
            break;
        case GetKey:
            state = GetKey; break;
        default: state = GetKey; break;
    }
    switch(state){
        case GStart: break;
        case GetKey:
            currKey = GetKeypadKey();
            break;
    }
    return state;
}

enum display_States { DStart, display_display};

int displaySMTick(int state){
    switch(state){
        case DStart:
            state = display_display;
            break;
        case display_display:
            state = display_display; break;
        default: state = display_display; break;
    }
    switch(state){
        case DStart:
            break;
        case display_display:
            switch(currKey){
            case '\0' : PORTB = 0x1F; break;
            case '1' : PORTB = 0x01; break;
            case '2' : PORTB = 0x02; break;
            case '3' : PORTB = 0x03; break;
            case '4' : PORTB = 0x04; break;
            case '5' : PORTB = 0x05; break;
            case '6' : PORTB = 0x06; break;
            case '7' : PORTB = 0x07; break;
            case '8' : PORTB = 0x08; break;
            case '9' : PORTB = 0x09; break;
            case '0' : PORTB = 0x00; break;
            case 'A' : PORTB = 0x0A; break;
            case 'B' : PORTB = 0x0B; break;
            case 'C' : PORTB = 0x0C; break;
            case 'D' : PORTB = 0x0D; break;
            case '*' : PORTB = 0x0E; break;
            case '#' : PORTB = 0x0F; break;
            default: PORTB = 0x1F; break;
            }
            break;
        default: break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */
    static task task1, task2;
    task *tasks[] = {&task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = -1;
    const char gcd = 10;

    task1.state = GStart;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &GetSMTick;

    task2.state = DStart;
    task2.period = 10;
    task2.elapsedTime = task2.period;
    task2.TickFct = &displaySMTick;

    TimerSet(gcd);
    TimerOn();


    unsigned short i;
    while (1) {
        for(i = 0; i < numTasks; i++)
        {
            if(tasks[i]->elapsedTime ==  tasks[i]->period)
            {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += gcd
            ;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
