#ifndef I2C_H
#define I2C_H

#include <msp430.h>

void init_i2c(int addr){
    
    UCB0CTLW0 |= UCSWRST;

    UCB0CTLW0 |= UCMST | UCMODE_3 | UCSYNC;

    UCB0CTLW0 |= UCSSEL__SMCLK;

    UCB0BRW = 10;

    UCB0I2CSA = addr;

    P1SEL1 &= ~(BIT2 | BIT3);  // Select primary module function
    P1SEL0 |=  (BIT2 | BIT3);  // for P1.2 (SDA) and P1.3 (SCL)

    UCB0CTLW0 &= ~UCSWRST;  // Release I2C module for operation

}

void reset_i2c(void){
    UCB0CTLW0 |= UCSWRST;

    UCB0CTLW0 &= ~UCSWRST;  // Release I2C module for operation
}

uint8_t read_i2c(uint8_t reg){ // unsigned integer, 8 bits
    uint8_t data = 0;

    while (UCB0STATW & UCBBUSY); // wait until not busy

    // Set to Transmit mode and send a START condition with the register address
    UCB0CTLW0 |= UCTR | UCTXSTT;           // UCTR=TX mode, send START
    while (!(UCB0IFG & UCTXIFG0));          // Wait for TX buffer to be ready
    UCB0TXBUF = reg;                      // Send register address
    while (!(UCB0IFG & UCTXIFG0));          // Wait for transmission to complete
    
    // Switch to Receiver mode to read the data
    UCB0CTLW0 &= ~UCTR;                   // Clear UCTR (switch to RX mode)
    UCB0CTLW0 |= UCTXSTT;                 // Send repeated START for reading
    while (UCB0CTLW0 & UCTXSTT);          // Wait until the repeated START is sent
    
    // Send STOP condition; the STOP will be sent after the first byte is received
    UCB0CTLW0 |= UCTXSTP;
    while (!(UCB0IFG & UCRXIFG0));         // Wait for a byte to be received
    data = UCB0RXBUF;                     // Read the received byte
    while (UCB0CTLW0 & UCTXSTP);          // Wait until the STOP condition is sent

    reset_i2c();

    return data;
}

// Read multiple registers in a single transaction (using auto-increment)
// For the LIS3DH, set the MSB of the register address to enable auto-increment
void readx_i2c(uint8_t startReg, uint8_t *buffer, uint8_t count)
{
    uint8_t regAddr = startReg | 0x80; // Set auto-increment bit (bit 7)

    while (UCB0STATW & UCBBUSY);       // Wait until the I2C bus is free
    
    // Transmit phase: send register address with auto-increment enabled
    UCB0CTLW0 |= UCTR | UCTXSTT;        // Set TX mode, send START condition
    while (!(UCB0IFG & UCTXIFG0));      // Wait until TX buffer is ready
    UCB0TXBUF = regAddr;              // Transmit register address
    while (!(UCB0IFG & UCTXIFG0));      // Wait until address transmitted
    
    // Switch to RX mode to read the data
    UCB0CTLW0 &= ~UCTR;               // Clear UCTR to switch to receiver mode
    UCB0CTLW0 |= UCTXSTT;             // Send repeated START for reading
    while (UCB0CTLW0 & UCTXSTT);      // Wait until repeated START is sent
    
    // Read the desired number of bytes
    int i;
    for (i = 0; i < count; i++) {
        if (i == (count - 1)) {        // For the last byte, set the STOP condition
            UCB0CTLW0 |= UCTXSTP;
        }
        while (!(UCB0IFG & UCRXIFG0)); // Wait until a byte is received
        buffer[i] = UCB0RXBUF;         // Store received byte into the buffer
    }
    while (UCB0CTLW0 & UCTXSTP);      // Wait until the STOP condition is complete

    reset_i2c();
}

void write_i2c(uint8_t reg, uint8_t value)
{
    // Wait until the I²C bus is free
    while (UCB0STATW & UCBBUSY);

    // Set eUSCI_B0 to Transmit mode and issue a START condition
    UCB0CTLW0 |= UCTR | UCTXSTT;
    while (!(UCB0IFG & UCTXIFG0));   // Wait for TX buffer ready

    // Send the register address (the location you want to write to)
    UCB0TXBUF = reg;
    while (!(UCB0IFG & UCTXIFG0));   // Wait for the register address to be transmitted

    // Send the data byte you wish to write to the register
    UCB0TXBUF = value;
    while (!(UCB0IFG & UCTXIFG0));   // Wait for the data byte to be transmitted

    // Issue a STOP condition to end the transaction
    UCB0CTLW0 |= UCTXSTP;
    while (UCB0CTLW0 & UCTXSTP);     // Wait for the STOP condition to complete
    
    reset_i2c();
}

#endif 
