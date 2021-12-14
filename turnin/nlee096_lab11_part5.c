/*	Author: Nathan Lee
 *  Partner(s) Name: none
 *	Lab Section: 22
 *	Assignment: Lab #11  Exercise #5
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://drive.google.com/drive/folders/16jScmV_wWNsaKk25DDhddHK_fPF9JHcm?usp=sharing
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


unsigned char stop = 0;
unsigned char obj1 = 16;
unsigned char obj2 = 30;
unsigned char delay = 0;

enum Scroll_States{sStart, scroll};
int scrollSMTick(int state){
    switch(state){
        case sStart:
            state = scroll;
            break;
        case scroll:
            state = scroll;
            break;
        default: state = scroll; break;
    }
    switch(state){
        case sStart: break;
        case scroll:
            if(!stop){
                    LCD_Cursor(obj1);
                    LCD_WriteData(' ');
                    LCD_Cursor(obj2);
                    LCD_WriteData(' ');
                    obj1 = obj1 - 1;
                    if(obj1 < 1){
                        obj1 = 16;
                    }
                    obj2 = obj2 - 1;
                    if(obj2 < 17){
                        obj2 = 32;
                    }
                    LCD_Cursor(obj1);
                    LCD_WriteData('#');
                    LCD_Cursor(obj2);
                    LCD_WriteData('#');
            }
            break;
        default: break;
    }
    return state;
}

enum Player_States {pStart, pUP, pDOWN};
unsigned char curr_pos = 18;
//C0 = up, C1 = down, C2 = pause

int playerSMTick(int state){
    unsigned char button = ~PINC & 0x03;
    switch(state){
        case pStart:
            curr_pos = 18;
            state = pDOWN;
            break;
        case pUP:
            if(button == 0x02){
                state = pDOWN;
            }
            else{
                state = pUP;
            }
            break;
        case pDOWN:
            if(button == 0x01){
                state = pUP;
            }
            else {
                state = pDOWN;
            }
            break;
        default: break;
    }
    switch(state){
        case pStart:
            curr_pos = 18;
            LCD_Cursor(18);
            break;
        case pUP:
            if(!stop){
                curr_pos = 2;
                LCD_Cursor(curr_pos);
                //PORTB = PORTB & 0xBF;
                //PORTB = PORTB | 0x80;
            }
        break;
        case pDOWN:
            if(!stop){
                curr_pos = 18;
                LCD_Cursor(curr_pos);
                //PORTB = PORTB & 0x7F;
                //PORTB = PORTB | 0x40;
            }
        break;

    }
    return state;
}

enum Game_States {GStart, Play, Pause, press, GameOver, press2, Reset};
unsigned char finished = 0;
unsigned char change = 0;

int GameSMTick(int state){
    unsigned char toggle = ~PINC & 0x04;
    switch(state){
        case GStart:
            stop = 0;
            change = 0;
            state = Play;
            break;
        case Pause:
            stop = 1;
            if(toggle == 0x04){
                change = 0;
                state = press;
            }
            else{
                state = Pause;
            }
            break;
        case Play:
            stop = 0;
            if(toggle == 0x04){
                change = 1;
                stop = 1;
                state = press;
            }
            else if(curr_pos == obj1 || curr_pos == obj2){
                stop = 1;
                state = GameOver;
            }
            else{
                state = Play;
            }
            break;
        case press:
            if(toggle == 0x04){
                state = press;
            }
            else{
                state = change?Pause:Play;
                stop = change?1:0;
                change = change?0:1;
            }
            break;
        case GameOver:
            stop = 1;
            if(toggle == 0x04){
                state = press2;
            }
            else{
                state = GameOver;
            }
            break;
        case press2:
            if(toggle == 0x04){
                state = press2;
            }
            else{
                state = Reset;
            }
            break;
        case Reset:
            state = Play;
            break;
        default:break;
    }
    switch(state){
        case GStart:
            stop = 0;
            break;
        case Pause:
            stop = 1;
            //PORTB = PORTB | 0x20;
            break;
        case Play:
            stop = 0;
            //PORTB = PORTB & 0xDF;
            break;
        case press:
            stop = 1;
            break;
        case GameOver:
            stop = 1;
            if(finished == 0){
                finished = 1;
                LCD_ClearScreen();
                LCD_DisplayString(1, "Game Over!");
            }
            break;
        case press2:
            //PORTB = PORTB | 0x20;
            break;
        case Reset:
            stop = 0;
            LCD_ClearScreen();
            obj1 = 16;
            obj2 = 30;
            finished = 0;
            break;
        default: break;
    }
    return state;
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0xFF; PORTA = 0x00;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x00; PORTC = 0xFF;
    /* Insert your solution below */

    static task task1, task2, task3;
    task *tasks[] = {&task1, &task2, &task3};

    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    const char start = -1;
    const char gcd = 10;

    task1.state = sStart;
    task1.period = 400;
    task1.elapsedTime = task1.period;
    task1.TickFct = &scrollSMTick;

    task2.state = pStart;
    task2.period = 10;
    task2.elapsedTime = task2.period;
    task2.TickFct = &playerSMTick;

    task3.state = GStart;
    task3.period = 10;
    task3.elapsedTime = task3.period;
    task3.TickFct = &GameSMTick;

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
            tasks[i]->elapsedTime += gcd;
        }

        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
