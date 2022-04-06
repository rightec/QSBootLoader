/* 
 * File:   qs_crc16.c
 * Author: Fernando Morani
 *
 * Created on 6 aprile 2022, 22.33
 */

#include "qs_crc16.h"

void CalcCrc16_Poly(uint16_t crc_initial, uint16_t poly, uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h)
{
uint8_t i,j;
uint16_t data;
uint16_t  crc = crc_initial;
uint8_t  crc_low, crc_hig;

    for (j = 0; j < wLen; j++)
    {
        data = pBuf[j];
        
        crc ^= data;
        
        for (i=0; i < 8; i++)
        {
            // se shiftero' un bit a 1 vado in xor col polinomio
            if ( crc & 0x0001 )
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

    crc_hig = crc & 0xFF;
    crc_low = crc >> 8;
    
    *crc_l = crc_low;
    *crc_h = crc_hig;
            
}


