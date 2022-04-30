/**
  Generated Interrupt Manager Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    interrupt_manager.c

  @Summary:
    This is the Interrupt Manager file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description:
    This header file provides implementations for global interrupt handling.
    For individual peripheral handlers please see the peripheral driver for
    all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC18F46Q43
        Driver Version    :  2.04
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.31 and above or later
        MPLAB 	          :  MPLAB X 5.45
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

#include "interrupt_manager.h"
#include "mcc.h"


void __interrupt(base(0x2108)) INTERRUPT_InterruptManager (void)
{
    // interrupt handler
    if(PIE13bits.U5TXIE == 1 && PIR13bits.U5TXIF == 1)
    {
        UART5_Transmit_ISR();
        //UART5_TxInterruptHandler();
    }
    else if(PIE13bits.U5RXIE == 1 && PIR13bits.U5RXIF == 1)
    {
        //UART5_RxInterruptHandler();
        UART5_Receive_ISR();
    }
    else
    {
        //Unhandled Interrupt
    }
}

/* Setup vector table PTR (IVTBASE)
 * 
 * Nota: CRC is loaded at 0x2000
 *       Vector table is at 0x2100 
 *       From 0x2000 to 0x20FF we can store other application info     
 *       Application size could grow until the size available for
 *       PIC18F47xx (Linker does the checksum)
 */
void  INTERRUPT_Initialize (void)
{
    // Disable Interrupt Priority Vectors (16CXXX Compatibility Mode)
    INTCON0bits.IPEN = 0;
    
   //INTCON0bits.GIEH = 1;
    // Enable high priority interrupts
    //INTCON0bits.GIEL = 1;
    // Enable low priority interrupts
    //INTCON0bits.IPEN = 1;
    // Enable interrupt priority
    //PIE0bits.SWIE = 1;
    // Enable SW interrupt
    //PIE0bits.HLVDIE = 1;
    // Enable HLVD interrupt
    //IPR0bits.SWIP = 0;
    // Make SW interrupt low priority

    INTERRUPT_GlobalInterruptDisable();
    
    
    // Change IVTBASE to 0x2108
    // Default is 0x000008
    IVTBASEU = 0x00;
    IVTBASEH = 0x21;
    IVTBASEL = 0x08;    
    
}


/**
 End of File
*/
