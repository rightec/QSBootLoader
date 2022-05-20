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


const uint16_t FW_Erp_BuildNumber __at(0x1FF0) = 0x01;   
const uint16_t FW_Erp_Identifier  __at(0x1FF2) = 0x02;
const uint16_t FW_Erp_Version     __at(0x1FF4) = 0x03;


//char *bootString __at(0x1000); 
char *bootString; 

uint32_t appLastAddr;
uint32_t deviceLastAddr;

int count;

uint8_t validApp;
uint32_t crc_val;  
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

uint32_t crc_mem; 
    
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
    
    
   bootString = (char *) 0x0500; 
 

   int cmpval = 1;
   //strcpy(bootString, "Stop Boot!");     
    
   cmpval = strncmp(bootString, "Stop Boot!", 9) ;
   
    if( cmpval == 0 )     
    {
       validApp = 0;       // No Jump to APP
       strcpy(bootString, "Go Boot!");     
    }
    else
    {
       
        if( theFamilyDeviceID.ID_Info == QS_PIC18F47Q43_DEV_ID )
            deviceLastAddr = 0x1FFFF;   // last address = 0x1E000;
        else
            deviceLastAddr = 0x0FFFF;   // last address = 0x0E000;
                                    
    //    FLASH_CalcCrc32Lsb(0xFFFFFFFF, 0xEDB88320, 
    //                            0x2004, 0x0F000, &crc_val);

        crc_mem = FLASH_ReadLongBE(APPCRC_FLASH_ADDR);
        
        appLastAddr =  FLASH_ReadLong(APPLSM_FLASH_ADDR);
        
        if( appLastAddr != 0xFFFFFFFF &&
            appLastAddr <= deviceLastAddr )
        {
            FLASH_CalcCrc32Msb(0xFFFFFFFF, 0xEDB88320, 
                                0x2004, appLastAddr+1, &crc_val);

            if( crc_mem == crc_val )
                validApp = 1;
            else
                validApp = 0;
        }

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