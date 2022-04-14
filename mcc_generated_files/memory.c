/**
  MEMORY Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    memory.c

  @Summary
    This is the generated driver implementation file for the MEMORY driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This file provides implementations of driver APIs for MEMORY.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC18F46Q43
        Driver Version    :  1.1.0
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
#include "memory.h"
#include "../util.h"

/**
  Section: Program Flash Memory APIs
 */

//128-words of Buffer RAM for PIC18F46Q43 is available at 0x1500
uint16_t bufferRAM __at(0x1500); 

uint16_t hexpars_len;
uint32_t flashAddr;
uint8_t statoHex;
uint8_t recordType;
uint8_t recordLen;
uint8_t cntRecordData;
uint8_t recordLine;
uint16_t recordAddr;
uint16_t recordSegm;
uint16_t recordExt;

uint16_t hex_value;

uint32_t memAddr;




uint16_t hex_cpw(const char *s)
{
uint16_t wh;
uint16_t wl;

	wl = hex_cpb(s);
	wh = hex_cpb(s+2);
	wh <<= 8;
	
	return ( wh | wl );			// mi sembra giusto
}

void FLASH_WriteHex(char *__strHex,  uint16_t __len)
{
char c;

    for(hexpars_len=0; hexpars_len<__len; )
    {
//        c = *__strHex++;

        c = __strHex[hexpars_len++];

        
        switch( statoHex )
        {
            case    0:
                
                if( c == ':' )  // inizio hex ?
                {

                    recordLen = hex_cpb(&__strHex[hexpars_len]);
                    hexpars_len += 2;
                    recordAddr = hex_cpw(&__strHex[hexpars_len]);
                    hexpars_len += 4;
                    recordType = hex_cpb(&__strHex[hexpars_len]);
                    hexpars_len += 2;
                    
                    switch( recordType )
                    {
                        case    0x00:   // data record
                       
                            memAddr = recordSegm;   // ricava address a 24bit
                            memAddr <<= 8;
                            memAddr += recordAddr;
                            
                            for(cntRecordData=0; cntRecordData < recordLen; cntRecordData += 2)
                            {
                                hex_value = hex_cpw(&__strHex[hexpars_len]);  // piglia un altro pezzetto ..
                                hexpars_len += 4;
                                
                                FLASH_WriteSingleWord(memAddr, hex_value);    // scrive in flash
                                
                                memAddr += 2;      // prossima word
                            }
                            break;

                        case    0x01:     // end of file record
                            break;

                        case    0x02:     // segment address record
                            recordSegm = hex_cpw(&__strHex[hexpars_len]);
                            hexpars_len += 4;
                            break;

                        case    0x04:     // Extended Linear Address record
                            recordSegm = hex_cpw(&__strHex[hexpars_len]);
                            hexpars_len += 4;
                            break;
                    }
                    
                    statoHex = 0;
                }
                else
                    if( c == '\n' )
                        recordLine++;
                break;
                
            case    1:
                break;

            case    10:     // end of file raggiunta
                break;
            
        }
    }
    
}


uint8_t FLASH_ReadByte(uint32_t flashAddr)
{
    //Set TBLPTR with the target byte address
    TBLPTRU = (uint8_t) ((flashAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t) ((flashAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t) (flashAddr & 0x000000FF);

    //Perform table read to move one byte from NVM to TABLAT
    asm("TBLRD");

    return (TABLAT);
}

uint16_t FLASH_ReadWord(uint32_t flashAddr)
{
    uint8_t readWordL, readWordH;

    //Set TBLPTR with the target byte address
    TBLPTRU = (uint8_t) ((flashAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t) ((flashAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t) (flashAddr & 0x000000FF);

    //Perform table read to move low byte from NVM to TABLAT
    asm("TBLRD*+");
    readWordL = TABLAT;

    //Perform table read to move high byte from NVM to TABLAT
    asm("TBLRD");
    readWordH = TABLAT;

    return (((uint16_t) readWordH << 8) | (readWordL));
}

void FLASH_ReadPage(uint32_t flashAddr)
{
    uint8_t GIEBitValue = INTCON0bits.GIE; // Save interrupt enable

    //Set NVMADR with the target word address
    NVMADRU = (uint8_t) ((flashAddr & 0x00FF0000) >> 16);
    NVMADRH = (uint8_t) ((flashAddr & 0x0000FF00) >> 8);
    NVMADRL = (uint8_t) (flashAddr & 0x000000FF);

    //Set the NVMCMD control bits for Page Read operation
    NVMCON1bits.NVMCMD = 0b010;

    //Disable all interrupt
    INTCON0bits.GIE = 0;

    //Perform the unlock sequence
    NVMLOCK = 0x55;
    NVMLOCK = 0xAA;

    //Start page read and wait for the operation to complete
    NVMCON0bits.GO = 1;
    while (NVMCON0bits.GO);

    //Restore the interrupts
    INTCON0bits.GIE = GIEBitValue;

    //Set the NVMCMD control bits for Word Read operation to avoid accidental writes
    NVMCON1bits.NVMCMD = 0b000;
}

void FLASH_WritePage(uint32_t flashAddr)
{
    uint8_t GIEBitValue = INTCON0bits.GIE; // Save interrupt enable

    //Set NVMADR with the target word address
    NVMADRU = (uint8_t) ((flashAddr & 0x00FF0000) >> 16);
    NVMADRH = (uint8_t) ((flashAddr & 0x0000FF00) >> 8);
    NVMADRL = (uint8_t) (flashAddr & 0x000000FF);

    //Set the NVMCMD control bits for Write Page operation
    NVMCON1bits.NVMCMD = 0b101;

    //Disable all interrupt
    INTCON0bits.GIE = 0;

    //Perform the unlock sequence
    NVMLOCK = 0x55;
    NVMLOCK = 0xAA;

    //Start page programming and wait for the operation to complete
    NVMCON0bits.GO = 1;
    while (NVMCON0bits.GO);

    //Restore the interrupts
    INTCON0bits.GIE = GIEBitValue;

    //Set the NVMCMD control bits for Word Read operation to avoid accidental writes
    NVMCON1bits.NVMCMD = 0b000;
}


// Code sequence to program one word to a pre-erased location in PFM
// PFM target address is specified by WORD_ADDR
// Target data is specified by WordValue
// Save interrupt enable bit value
void FLASH_WriteSingleWord(uint32_t __flashAddr, uint16_t __word_value)
{
uint8_t GIEBitValue = INTCON0bits.GIE;

/*
    // Load NVMADR with the target address of the word
    NVMADR = __flashAddr;
    NVMDAT = __word_value;
    // Load NVMDAT with the desired value
    NVMCON1bits.CMD = 0x03;
    // Set the word write command
    INTCON0bits.GIE = 0;
 
    // Disable interrupts
    //????????? Required Unlock Sequence ?????????
    NVMLOCK = 0x55;
    NVMLOCK = 0xAA;
    NVMCON0bits.GO = 1;
    
    // Start word write
    //???????????????????????????????????????????????
    while (NVMCON0bits.GO);
    
    // Wait for the write operation to complete
    // Verify word write operation success and call the recovery function if needed
    if (NVMCON1bits.WRERR)
    {
        // WRITE_FAULT_RECOVERY();
    }
    INTCON0bits.GIE = GIEBitValue;
    NVMCON1bits.CMD = 0x00;

    //10.3.6
    // Restore interrupt enable bit value
    // Disable writes to memory
*/
  
 }

void FLASH_WriteWord(uint32_t flashAddr, uint16_t word)
{
    uint16_t *bufferRamPtr = (uint16_t*) & bufferRAM;
    uint32_t blockStartAddr = (uint32_t) (flashAddr & ((END_FLASH - 1) ^ ((ERASE_FLASH_BLOCKSIZE * 2) - 1)));
    uint8_t offset = (uint8_t) ((flashAddr & ((ERASE_FLASH_BLOCKSIZE * 2) - 1)) / 2);

    //Read existing block into Buffer RAM
    FLASH_ReadPage(blockStartAddr);

    //Erase the given block
    FLASH_EraseBlock(blockStartAddr);

    //Modify Buffer RAM for the given word to be written to Program Flash Memory
    bufferRamPtr += offset;
    *bufferRamPtr = word;

    //Write Buffer RAM contents to given Program Flash Memory block
    FLASH_WritePage(blockStartAddr);
}

int8_t FLASH_WriteBlock(uint32_t flashAddr, uint16_t *flashWrBufPtr)
{
    uint16_t *bufferRamPtr = (uint16_t*) & bufferRAM;
    uint32_t blockStartAddr = (uint32_t) (flashAddr & ((END_FLASH - 1) ^ ((ERASE_FLASH_BLOCKSIZE * 2) - 1)));
    uint8_t i;

    //Block write must start at the beginning of a row
    if (flashAddr != blockStartAddr)
    {
        return -1;
    }

    //Copy application buffer contents to Buffer RAM
    for (i = 0; i < ERASE_FLASH_BLOCKSIZE; i++)
    {
        *bufferRamPtr++ = flashWrBufPtr[i];
    }

    //Erase the given block
    FLASH_EraseBlock(flashAddr);

    //Write Buffer RAM contents to given Program Flash Memory block
    FLASH_WritePage(flashAddr);

    return 0;
}

void FLASH_EraseBlock(uint32_t flashAddr)
{
    uint32_t blockStartAddr = (uint32_t) (flashAddr & ((END_FLASH - 1) ^ ((ERASE_FLASH_BLOCKSIZE * 2) - 1)));
    uint8_t GIEBitValue = INTCON0bits.GIE;

    //The NVMADR[21:8] bits point to the page being erased.
    //The NVMADR[7:0] bits are ignored
    NVMADRU = (uint8_t) ((blockStartAddr & 0x00FF0000) >> 16);
    NVMADRH = (uint8_t) ((blockStartAddr & 0x0000FF00) >> 8);

    //Set the NVMCMD control bits for Erase Page operation
    NVMCON1bits.NVMCMD = 0b110;

    //Disable all interrupts
    INTCON0bits.GIE = 0;

    //Perform the unlock sequence
    NVMLOCK = 0x55;
    NVMLOCK = 0xAA;

    //Start page erase and wait for the operation to complete
    NVMCON0bits.GO = 1;
    while (NVMCON0bits.GO);

    //Restore the interrupts
    INTCON0bits.GIE = GIEBitValue;

    //Set the NVMCMD control bits for Word Read operation to avoid accidental writes
    NVMCON1bits.NVMCMD = 0b000;
}

void DATAEE_WriteByte(uint16_t bAdd, uint8_t bData)
{
    uint8_t GIEBitValue = INTCON0bits.GIE;

    //Set NVMADR with the target word address (0x380000 - 0x3803FF)
    NVMADRU = 0x38;
    NVMADRH = (uint8_t) ((bAdd & 0xFF00) >> 8);
    NVMADRL = (uint8_t) (bAdd & 0x00FF);

    //Load NVMDATL with desired byte
    NVMDATL = bData;

    //Set the NVMCMD control bits for DFM Byte Write operation
    NVMCON1bits.NVMCMD = 0b011;

    //Disable all interrupts
    INTCON0bits.GIE = 0;

    //Perform the unlock sequence and start Page Erase
    NVMLOCK = 0x55;
    NVMLOCK = 0xAA;

    //Start DFM write and wait for the operation to complete
    NVMCON0bits.GO = 1;
    while (NVMCON0bits.GO);

    //Restore all the interrupts
    INTCON0bits.GIE = GIEBitValue;

    //Set the NVMCMD control bits for Word Read operation to avoid accidental writes
    NVMCON1bits.NVMCMD = 0b000;
}

uint8_t DATAEE_ReadByte(uint16_t bAdd)
{
    //Set NVMADR with the target word address (0x380000 - 0x3803FF)
    NVMADRU = 0x38;
    NVMADRH = (uint8_t) ((bAdd & 0xFF00) >> 8);
    NVMADRL = (uint8_t) (bAdd & 0x00FF);

    //Set the NVMCMD control bits for DFM Byte Read operation
    NVMCON1bits.NVMCMD = 0b000;
    NVMCON0bits.GO = 1;

    return NVMDATL;
}

void MEMORY_ISR(void)
{
    /* TODO : Add interrupt handling code */
    PIR15bits.NVMIF = 0;
}
