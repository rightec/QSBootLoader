/* 
 * File:   qs_memory.c
 * Author: Fernando Morani
 *
 * Created on 10 aprile 2022, 9.24
 */
#include "qs_memory.h"
#include "qs_proto.h"

static uint16_t hexpars_len;
static uint32_t flashAddr;
static uint8_t statoHex;
static uint8_t recordType;
static uint8_t recordLen;
static uint8_t cntRecordData;
static uint8_t recordLine;
static uint16_t recordAddr;
static uint16_t recordSegm;
static uint16_t recordExt;

static uint16_t hex_value;

static uint32_t memAddr;



void FLASH_WriteHex(const char *__strHex,  uint16_t __len)
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
                            memAddr <<= 16;
                            memAddr += recordAddr;
                            
                            for(cntRecordData=0; cntRecordData < recordLen; cntRecordData += 2)
                            {
                                hex_value = hex_cpw_swap(&__strHex[hexpars_len]);  // piglia un altro pezzetto ..
                                //hex_value = hex_cpw(&__strHex[hexpars_len]);  // piglia un altro pezzetto ..
                                hexpars_len += 4;
                                
                                if( theFamilyDeviceID.ID_Info == QS_PIC18F47Q43_DEV_ID )
                                {
                                    if( memAddr >= 0x2000 && memAddr <= 0x1FFFE )      // possiamo scrivere da 0x2000 in su
                                        FLASH_WriteSingleWord(memAddr, hex_value);    // scrive in flash
                                }
                                else
                                {
                                    if( memAddr >= 0x2000 && memAddr <= 0xFFFE )      // possiamo scrivere da 0x2000 in su
                                        FLASH_WriteSingleWord(memAddr, hex_value);    // scrive in flash
                                }                                
                                
                                memAddr += 2;      // prossima word
                            }
                            break;

                        case    0x01:     // end of file record
                            break;

                        case    0x02:     // segment address record
                            //recordSegm = hex_cpw(&__strHex[hexpars_len]);
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

void FLASH_CalcCrc32Lsb(uint32_t crc_initial, uint32_t poly, 
                            uint32_t __flashStartAddr, uint32_t __flashEndAddr, uint32_t *__crc_val)
{
uint8_t i,j;
uint32_t flash_data;
uint32_t  crc = crc_initial;
uint32_t  local_mem_addr;

    for (local_mem_addr=__flashStartAddr; local_mem_addr < __flashEndAddr; local_mem_addr++)
    {
        flash_data = FLASH_ReadByte(local_mem_addr);
        
        crc ^= flash_data;
        
        for (i=0; i < 8; i++)
        {
            // se shiftero' un bit a 1 vado in xor col polinomio
            if ( crc & 0x01 )
            {
                crc >>= 1;      // shift
                
                crc ^= poly;    // somma il poly
            }
            else
            {
                crc >>= 1;      // shifta solo
            }
        }
    }

    *__crc_val = crc;
            
}

/*
 http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
 * 
 * http://www.sunshine2k.de/articles/coding/crc/understanding_crc.html
 */

void FLASH_CalcCrc32Msb(uint32_t crc_initial, uint32_t poly, 
                            uint32_t __flashStartAddr, uint32_t __flashEndAddr, uint32_t *__crc_val)
{
uint8_t i,j;
uint32_t flash_data;
uint32_t  crc = crc_initial;
uint32_t  local_mem_addr;

    for (local_mem_addr=__flashStartAddr; local_mem_addr < __flashEndAddr; local_mem_addr++)
    {
        flash_data = FLASH_ReadByte(local_mem_addr);
        
        flash_data <<= 24;       // porta nel byte msb
        
        crc ^= flash_data;
        
        for (i=0; i < 8; i++)
        {
            // se shiftero' un bit a 1 vado in xor col polinomio
            if ( crc & 0x80000000 )
            {
                crc <<= 1;      // shift
                
                crc ^= poly;    // somma il poly
            }
            else
            {
                crc <<= 1;      // shifta solo
            }
        }
    }

    *__crc_val = crc;
            
}

void FLASH_CalcCrc16(uint16_t crc_initial, uint16_t poly, 
                            uint32_t __flashStartAddr, uint32_t __flashEndAddr, uint16_t *__crc_val)
{
uint8_t i,j;
uint32_t flash_data;
uint32_t  crc = crc_initial;
uint32_t  local_mem_addr;

    for (local_mem_addr=__flashStartAddr; local_mem_addr < __flashEndAddr; local_mem_addr++)
    {
        flash_data = FLASH_ReadByte(local_mem_addr);
        
        flash_data << 24;       // porta nel byte msb
        
        crc ^= flash_data;
        
        for (i=0; i < 8; i++)
        {
            // se shiftero' un bit a 1 vado in xor col polinomio
            if ( crc & 0x80000000 )
            {
                crc <<= 1;      // shift
                
                crc ^= poly;    // somma il poly
            }
            else
            {
                crc <<= 1;      // shifta solo
            }
        }
    }

    *__crc_val = crc;
            
}


/*
 Big Endian reader for flash mem
 */
uint32_t FLASH_ReadLongBE(uint32_t flashAddr)
{
union U_LVAL longVal;

    //Set TBLPTR with the target byte address
    TBLPTRU = (uint8_t) ((flashAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t) ((flashAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t) (flashAddr & 0x000000FF);

    //Perform table read to move low byte from NVM to TABLAT
    asm("TBLRD*+");
    longVal.c[3] = TABLAT;

    asm("TBLRD*+");
    longVal.c[2] = TABLAT;

    asm("TBLRD*+");
    longVal.c[1] = TABLAT;

    //Perform table read to move high byte from NVM to TABLAT
    asm("TBLRD");
    longVal.c[0] = TABLAT;

    return (longVal.l);    
}





uint32_t FLASH_ReadLong(uint32_t flashAddr)
{
union U_LVAL longVal;

    //Set TBLPTR with the target byte address
    TBLPTRU = (uint8_t) ((flashAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t) ((flashAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t) (flashAddr & 0x000000FF);

    //Perform table read to move low byte from NVM to TABLAT
    asm("TBLRD*+");
    longVal.c[0] = TABLAT;

    asm("TBLRD*+");
    longVal.c[1] = TABLAT;

    asm("TBLRD*+");
    longVal.c[2] = TABLAT;

    //Perform table read to move high byte from NVM to TABLAT
    asm("TBLRD");
    longVal.c[3] = TABLAT;

    return (longVal.l);    
}

// Code sequence to program one word to a pre-erased location in PFM
// PFM target address is specified by WORD_ADDR
// Target data is specified by WordValue
// Save interrupt enable bit value
void FLASH_WriteSingleWord(uint32_t __flashAddr, uint16_t __word_value)
{
uint8_t GIEBitValue = INTCON0bits.GIE;


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

  
 }