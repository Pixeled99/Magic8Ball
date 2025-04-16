#ifndef LCD_H
#define LCD_H

#include <msp430.h>

int pos = 0;
int line = 0;

void delay_ms(unsigned int ms)
{
    while(ms--)
        __delay_cycles(1000);  // 1ms delay if MCLK is 1MHz
}


void send_command_lcd(unsigned char data){
    send_raw_lcd(data, 0x0);
}

void send_data_lcd(unsigned char data){
    send_raw_lcd(data, BIT0);
    pos++;
}

void reset_lines(){
    pos = 0;
    line = 0;
}

void send_sentence_lcd(char str[]){
    unsigned int j = 0;
    while (str[j] != '\0'){
        send_data_lcd(str[j]);
        if (pos == 16){
            pos = 0;
            if (line == 0){
                send_command_lcd(0xC0);
                line = 1;
            }
            else{
                send_command_lcd(0x01);
                line = 0;
            }
            
        }
        j++;
    }
}

void set_payload_lcd(unsigned char data){
    P3OUT = data;
    data = data & BIT3;
    P5OUT |= data;
    data = P3OUT;
}

void send_raw_lcd(unsigned char data, unsigned char mask){

    P2OUT = 0;
    P2OUT |= mask;
    P2OUT &= ~BIT1;

    set_payload_lcd(data);

    P2OUT |= BIT2;

    P2OUT &= ~BIT2;

    P5OUT &= ~BIT3;
    P3OUT = 0;
    P2OUT = 0;
}

void init_lcd(){
    P5DIR |= BIT3;
    P3DIR |= 0x0FF;
    P2DIR |= 0x07;

    send_command_lcd(0x38); // 8 bit mode

    send_command_lcd(0x01); // clear screen

    send_command_lcd(0x0C); // turn screen on

    send_command_lcd(0x06); // set entry mode
}

#endif 
