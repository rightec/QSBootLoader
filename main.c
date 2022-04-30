/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC18F46Q43
        Driver Version    :  2.00
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

#include "mcc_generated_files/mcc.h"
#include <string.h>

#include "qs_memory.h"
#include "qs_proto.h"



//char *bootString __at(0x1000); 
char *bootString; 

int count;

uint8_t validApp;
uint16_t crc_val1 = 0;  
uint16_t crc_val2 = 0;  
uint16_t crc_Flash = 0;

readType checksumData[0x100];
resultType hexmateChecksum;
/*
                         Main application
 */
void main(void)
{

uint16_t crc_mem; 
    
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    proto_init();  // init dati proto
    
 //   FLASH_CalcCrc32(0xFFFFFFFF, 0xEDB88320, 
  //                          0x2004, 0x0F000, &crc_val);

//    FLASH_CalcCrc16(0xFFFF, 0x8408, 
//                            0x2004, 0x0F000, &crc_val);

//    FLASH_CalcCrc16(0xFFFF, 0x8408, 
//                            0x2004, 0xF000, &crc_val1);
    
     

    
    crc_val2 = 0xFFFF;
    crc_val2 = crc((readType *)0x2004, 0xF000-0x2004, crc_val2);
    
    uint16_t iSize = 0xF000-0x2004;
    uint16_t iLoop = iSize/0x100;
    uint16_t iRest = iSize - (iLoop*0x100);
    uint16_t address = 0x2004;
    hexmateChecksum = 0xFFFF;
    for (uint16_t i = 0; i<iLoop; i++){
        address = 0x2004 + 0x100*i;
        for (uint16_t j = 0; j< 0x100; j++){
            checksumData[j] = FLASH_ReadByte(address + j);
        }
       // memcpy((void*)checksumData,(void*)(address),0x100);
        hexmateChecksum = crc(checksumData,
                    sizeof(checksumData)/sizeof(readType), hexmateChecksum);
            
    }
    address = address + 0x100;
    for (uint16_t j = 0; j< iRest; j++){
            checksumData[j] = FLASH_ReadByte(address + j);
        }
//    memcpy((void*)checksumData,(void*)(address),iRest);
    hexmateChecksum = crc(checksumData,
                    iRest/sizeof(readType), hexmateChecksum);

    crc_Flash = 0xFFFF;
    crc_Flash = crcFlash(0x2004, 0xF000-0x2004, crc_Flash);
     
    
    crc_mem = FLASH_ReadWord(0x2000);
    
    
    if( crc_mem != 0xFFFF || crc_val1 == crc_val2 )
        validApp = 1;
    else
        validApp = 0;
    
    /*
    if( crc_mem == crc_val )
        validApp = 1;
    else
        validApp = 0;
    */
    
   bootString = (char *) 0x0500; 
   
   //strcpy(bootString, "Stop Boot!");     

   
    if( strcmp(bootString, "Stop Boot!") == 0 )     
    {
       validApp = 0;       // No Jump to APP
       strcpy(bootString, "Go Boot!");     
    }
   
   
    while (1)
    {
        
        count++;

        proto_entry();  // entrypoint protocol manager
        
        if( count > 30000 )
        {
            count = 0;
            LED_LIFE_Toggle();

            if( validApp )      // App is valid - Run
            {
                    // Disable the Global Interrupts
                INTERRUPT_GlobalInterruptDisable();

                // Jump to App
                asm("goto 0x2100");
            }
        }
        
        // Add your application code
    }
}




/**
 End of File
*/