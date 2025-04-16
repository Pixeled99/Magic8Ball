/*
SDA - P1.2
SCL - P1.3
I1  - P1.4

RS - P2.0
RW - P2.1
E  - P2.2

D0 - P3.0
D1 - P3.1
D2 - P3.2
D3 - P5.3 P3.3 is broken
D4 - P3.4
D5 - P3.5
D6 - P3.6
D7 - P3.7
*/

#include "msp430fr2355.h"
#include <msp430.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <i2c.h>
#include <lcd.h>

#define LIS3DH_ADDR     0x19
#define CTRL_REG1       0x20   
#define CTRL_REG3       0x22   
#define CTRL_REG4       0x23
#define CTRL_REG5       0x24
#define INT1_CFG        0x30 
#define INT1_SRC        0x31 
#define INT1_THS        0x32  
#define INT1_DURATION   0x33 

char *strings[] = {
        "Yes",
        "No",
        "Maybe So"
};

void uint8_to_hex(uint8_t value, char *buf) {
    const char hexDigits[] = "0123456789ABCDEF";
    buf[0] = hexDigits[(value >> 4) & 0x0F];
    buf[1] = hexDigits[value & 0x0F];
    buf[2] = '\0';
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void) {
    if (P1IFG & BIT4) {
 
        read_i2c(INT1_SRC);

        reset_lines();

        send_command_lcd(0x01);

        delay_ms(5);

        int r = rand()%3; 

        send_sentence_lcd(strings[r]);

        P1OUT ^= BIT0;

        P1IFG &= ~BIT4;
    }
}

int main(void)
{
    srand(time(NULL));

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;       // Disable high-impedance mode

    P1DIR |= BIT0;
    P1OUT |= BIT0;

    P1DIR &= ~BIT4;       // Set P1.4 as input.
    P1REN |= BIT4;        // Enable pull resistor.
    P1OUT |= BIT4;        // Configure as pull-up (assuming the sensor pulls it low when interrupting).
    P1IES &= ~BIT4;
    P1IFG &= ~BIT4;       // Clear any pending interrupt flag.
    P1IE  |= BIT4;        // Enable interrupt on P1.4.

    __bis_SR_register(GIE);
    
    init_i2c(LIS3DH_ADDR);
    init_lcd();

    delay_ms(100);

    write_i2c(CTRL_REG1, 0x4B);
    // 01001011
    // ^^^^^^^^
    // 7-4: 50hz
    // 3: low power mode
    // 2: Z off
    // 1: Y on
    // 0: X on

    write_i2c(INT1_CFG, 0xA);
    // 00001010
    // ^^^^^^^^
    // 7: OR combination of events
    // 6: 6D function disabled
    // 5: Z high event disabled
    // 4: Z low event disabled
    // 3: Y high event enabled
    // 2: Y low event disabled
    // 1: X high event enabled
    // 0: X low event disabled

    write_i2c(INT1_THS, 0x30);
    // 00110000
    // ^^^^^^^^
    // 1 LSB ≈ 16 mg at ±2g full scale → threshold ≈ 48 * 16 mg = 768 mg

    write_i2c(INT1_DURATION, 0x05);
    // 00000101
    // ^^^^^^^^
    // At 50 Hz ODR (1 count ≈ 20 ms) → duration ≈ 5 * 20 ms = 100 ms

    write_i2c(CTRL_REG3, 0x40);
    // 01000000
    // ^^^^^^^^
    // CTRL_REG3: Bit 6 (I1_IA1) enabled → interrupt signal on INT1 activated;
    // all other bits are 0

    write_i2c(CTRL_REG5, 0x08);
    // 00001000
    // ^^^^^^^^
    // CTRL_REG5: Bit 3 (LIR_INT1) enabled → latched interrupt on INT1 enabled
    // (the interrupt remains active until INT1_SRC is read)

    read_i2c(INT1_SRC); //this is needed, I don't know why

    while (1);
}

