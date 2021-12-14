/*	Author: Nathan Lee
 *  Partner(s) Name: none
 *	Lab Section: 22
 *	Assignment: Lab #11  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/drive/folders/1SXqYvEWZYryei92OeYtFjwiOHCDIvSeV?usp=sharing
 *
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "bit.h"
#include "io.h"
#include "keypad.h"
//#include "lcd_8bit_task.h"
//#include "queue.h"
//#include "scheduler.h"
//#include "seven_seg.h"
//#include "stack.h"
//#include "usart.h"
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

typedef struct task{
    signed char state;
    unsigned long int period;
    unsigned long int elapsedTime;
    int (*TickFct)(int);
} task;


enum display_States { DStart, display_display};
unsigned char message[66] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                        'C','S','1','2','0','B',' ','i','s',' ','L','e','g','e','n','d',
                        '.','.','.','w','a','i','t',' ','f','o','r',' ','i','t',' ',
                        'D','A','R','Y','!',
                        ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
unsigned char j;
unsigned char offset;
unsigned char position;
unsigned char delay;

int displaySMTick(int state){
    switch(state){
        case DStart:
            delay = 0;
            position = 16;
            offset = 0;
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
            delay += 1;
            if(delay >= 50){
                for(j=0; j<16; j++ ){
                    LCD_Cursor(j+1);
                    LCD_WriteData(message[j+offset]);
                }
                if(offset >= 50 ){
                    offset = 0;
                }
                else{
                    offset += 1;
                }
                delay =0;
            }
                
            
            break;
        default: break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0xF0; PORTC = 0x0F;
    /* Insert your solution below */

    static task task1 /*, task2*/;
    task *tasks[] = {&task1 /*, &task2*/};

    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = -1;
    const char gcd = 10;
/*
    task1.state = GStart;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &GetSMTick;
*/
    task1.state = DStart;
    task1.period = 10;
    task1.elapsedTime = task1.period;
    task1.TickFct = &displaySMTick;

    TimerSet(gcd);
    TimerOn();
    LCD_init();

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
