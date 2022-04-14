/* 
 * File:   qs_memory.c
 * Author: Fernando Morani
 *
 * Created on 10 aprile 2022, 9.24
 */
#include "qs_memory.h"

uint16_t hexpars_len;
uint32_t flashAddr;
uint8_t statoHex;
uint8_t recordType;

void FLASH_WriteHex(char *__strHex,  uint16_t __len)
{
   /*
char c;

    for(hexpars_len=0; hexpars_len<__len; hexpars_len++)
    {
        c = __strHex++;
        
        
        switch( statoHex )
        {
            case    0:
                
                if( c == ':' )  // inizio hex ?
                {
                    hex_cpb(__strHex, recordType);
                    statoHex = 1;
                }
                break;
                
            case    1:
                break;
            
        }
    }
    */
}


