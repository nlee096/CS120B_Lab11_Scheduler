/*	Author: Nathan Lee
 *  Partner(s) Name: none
 *	Lab Section: 22
 *	Assignment: Lab #11  Exercise #4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/drive/folders/1Xw5sFDX7DKosThAhY7TOzib2LvMQR6pS?usp=sharing
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

enum Get_States { GStart, GetKey};
unsigned char screenFill[16]={'C', 'o', 'n', 'g', 'r', 'a', 't', 'u', 'l', 'a', 't', 'i', 'o', 'n', 's', '!'};
unsigned char tempFill[16]={' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
unsigned char currKey = '\0';
unsigned char temp = '\0';
unsigned char index = 0;

int GetSMTick(int state){
    switch(state){
        case GStart:
            index = 0;
            state = GetKey;
            break;
        case GetKey:
            state = GetKey;
            break;
        default: state = GetKey; break;
    }
    switch(state){
        case GStart: break;
        case GetKey:
            LCD_Cursor(index+1);
            currKey = GetKeypadKey();
            if((currKey != '\0')&&(currKey != temp)){
                temp = currKey;
                screenFill[index] = currKey;
                index++;
                if(index >= 16){
                    index = 0;
                }
            }
            temp = currKey;
        break;
    }
    return state;
}

enum display_States { DStart, display_display};
unsigned char j;


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
            for(j=0; j<16;j++){
                if(tempFill[j] != screenFill[j]){
                    LCD_Cursor(j+1);
                    LCD_WriteData(screenFill[j]);
                    tempFill[j] = screenFill[j];
                }
            }
            LCD_Cursor(index+1);
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
    task2.elapsedTime = task1.period;
    task2.TickFct = &displaySMTick;

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
