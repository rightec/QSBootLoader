/**
  UART5 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    uart5.c

  @Summary
    This is the generated driver implementation file for the UART5 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This source file provides APIs for UART5.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC18F46Q43
        Driver Version    :  2.4.1
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.31 and above
        MPLAB             :  MPLAB X 5.45
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

/**
  Section: Included Files
*/
#include <xc.h>
#include "uart5.h"

/**
  Section: Macro Declarations
*/
#define UART5_TX_BUFFER_SIZE 8
#define UART5_RX_BUFFER_SIZE 8

/**
  Section: Global Variables
*/

static volatile uint8_t uart5TxHead = 0;
static volatile uint8_t uart5TxTail = 0;
static volatile uint8_t uart5TxBuffer[UART5_TX_BUFFER_SIZE];
volatile uint8_t uart5TxBufferRemaining;

static volatile uint8_t uart5RxHead = 0;
static volatile uint8_t uart5RxTail = 0;
static volatile uint8_t uart5RxBuffer[UART5_RX_BUFFER_SIZE];
static volatile uart5_status_t uart5RxStatusBuffer[UART5_RX_BUFFER_SIZE];
volatile uint8_t uart5RxCount;
static volatile uart5_status_t uart5RxLastError;

/**
  Section: UART5 APIs
*/
void (*UART5_FramingErrorHandler)(void);
void (*UART5_OverrunErrorHandler)(void);
void (*UART5_ErrorHandler)(void);

void UART5_DefaultFramingErrorHandler(void);
void UART5_DefaultOverrunErrorHandler(void);
void UART5_DefaultErrorHandler(void);

void UART5_Initialize(void)
{
    // Disable interrupts before changing states
    PIE13bits.U5RXIE = 0;
    UART5_SetRxInterruptHandler(UART5_Receive_ISR);
    PIE13bits.U5TXIE = 0;
    UART5_SetTxInterruptHandler(UART5_Transmit_ISR);

    // Set the UART5 module to the options selected in the user interface.

    // P1L 0; 
    U5P1L = 0x00;

    // P2L 0; 
    U5P2L = 0x00;

    // P3L 0; 
    U5P3L = 0x00;

    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN enabled; ABDEN disabled; 
    U5CON0 = 0xB0;

    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U5CON1 = 0x80;

    // TXPOL not inverted; FLO off; RXPOL not inverted; RUNOVF RX input shifter stops all activity; STP Transmit 1Stop bit, receiver verifies first Stop bit; 
    U5CON2 = 0x00;

    // BRGL 68; 
    U5BRGL = 0x44;

    // BRGH 0; 
    U5BRGH = 0x00;

    // STPMD in middle of first Stop bit; TXWRE No error; 
    U5FIFO = 0x00;

    // ABDIF Auto-baud not enabled or not complete; WUIF WUE not enabled by software; ABDIE disabled; 
    U5UIR = 0x00;

    // ABDOVF Not overflowed; TXCIF 0; RXBKIF No Break detected; RXFOIF not overflowed; CERIF No Checksum error; 
    U5ERRIR = 0x00;

    // TXCIE disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; RXFOIE disabled; PERIE disabled; RXBKIE disabled; 
    U5ERRIE = 0x00;


    UART5_SetFramingErrorHandler(UART5_DefaultFramingErrorHandler);
    UART5_SetOverrunErrorHandler(UART5_DefaultOverrunErrorHandler);
    UART5_SetErrorHandler(UART5_DefaultErrorHandler);

    uart5RxLastError.status = 0;

    // initializing the driver state
    uart5TxHead = 0;
    uart5TxTail = 0;
    uart5TxBufferRemaining = sizeof(uart5TxBuffer);
    uart5RxHead = 0;
    uart5RxTail = 0;
    uart5RxCount = 0;

    // enable receive interrupt
    PIE13bits.U5RXIE = 1;
}

bool UART5_is_rx_ready(void)
{
    return (uart5RxCount ? true : false);
}

bool UART5_is_tx_ready(void)
{
    return (uart5TxBufferRemaining ? true : false);
}

bool UART5_is_tx_done(void)
{
    return U5ERRIRbits.TXMTIF;
}

uart5_status_t UART5_get_last_status(void){
    return uart5RxLastError;
}

uint8_t UART5_Read(void)
{
    uint8_t readValue  = 0;
    
    while(0 == uart5RxCount)
    {
    }

    uart5RxLastError = uart5RxStatusBuffer[uart5RxTail];

    readValue = uart5RxBuffer[uart5RxTail++];
   	if(sizeof(uart5RxBuffer) <= uart5RxTail)
    {
        uart5RxTail = 0;
    }
    PIE13bits.U5RXIE = 0;
    uart5RxCount--;
    PIE13bits.U5RXIE = 1;

    return readValue;
}

void UART5_Write(uint8_t txData)
{
    while(0 == uart5TxBufferRemaining)
    {
    }

    if(0 == PIE13bits.U5TXIE)
    {
        U5TXB = txData;
    }
    else
    {
        PIE13bits.U5TXIE = 0;
        uart5TxBuffer[uart5TxHead++] = txData;
        if(sizeof(uart5TxBuffer) <= uart5TxHead)
        {
            uart5TxHead = 0;
        }
        uart5TxBufferRemaining--;
    }
    PIE13bits.U5TXIE = 1;
}





void UART5_Transmit_ISR(void)
{
    // use this default transmit interrupt handler code
    if(sizeof(uart5TxBuffer) > uart5TxBufferRemaining)
    {
        U5TXB = uart5TxBuffer[uart5TxTail++];
       if(sizeof(uart5TxBuffer) <= uart5TxTail)
        {
            uart5TxTail = 0;
        }
        uart5TxBufferRemaining++;
    }
    else
    {
        PIE13bits.U5TXIE = 0;
    }
    
    // or set custom function using UART5_SetTxInterruptHandler()
}

void UART5_Receive_ISR(void)
{
    // use this default receive interrupt handler code
    uart5RxStatusBuffer[uart5RxHead].status = 0;

    if(U5ERRIRbits.FERIF){
        uart5RxStatusBuffer[uart5RxHead].ferr = 1;
        UART5_FramingErrorHandler();
    }
    
    if(U5ERRIRbits.RXFOIF){
        uart5RxStatusBuffer[uart5RxHead].oerr = 1;
        UART5_OverrunErrorHandler();
    }
    
    if(uart5RxStatusBuffer[uart5RxHead].status){
        UART5_ErrorHandler();
    } else {
        UART5_RxDataHandler();
    }

    // or set custom function using UART5_SetRxInterruptHandler()
}

void UART5_RxDataHandler(void){
    // use this default receive interrupt handler code
    uart5RxBuffer[uart5RxHead++] = U5RXB;
    if(sizeof(uart5RxBuffer) <= uart5RxHead)
    {
        uart5RxHead = 0;
    }
    uart5RxCount++;
}

void UART5_DefaultFramingErrorHandler(void){}

void UART5_DefaultOverrunErrorHandler(void){}

void UART5_DefaultErrorHandler(void){
    UART5_RxDataHandler();
}

void UART5_SetFramingErrorHandler(void (* interruptHandler)(void)){
    UART5_FramingErrorHandler = interruptHandler;
}

void UART5_SetOverrunErrorHandler(void (* interruptHandler)(void)){
    UART5_OverrunErrorHandler = interruptHandler;
}

void UART5_SetErrorHandler(void (* interruptHandler)(void)){
    UART5_ErrorHandler = interruptHandler;
}



void UART5_SetRxInterruptHandler(void (* InterruptHandler)(void)){
    UART5_RxInterruptHandler = InterruptHandler;
}

void UART5_SetTxInterruptHandler(void (* InterruptHandler)(void)){
    UART5_TxInterruptHandler = InterruptHandler;
}


/**
  End of File
*/
