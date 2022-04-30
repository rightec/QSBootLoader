/* 
 * File:   qs_crc16.h
 * Author: Fernando Morani
 *
 * Created on 6 aprile 2022, 22.33
 */

#ifndef QS_CRC16_H
#define	QS_CRC16_H

#include "mcc_generated_files/mcc.h"
#include <string.h>

typedef unsigned char readType;
typedef unsigned int resultType;


#define POLY_IBM    0x8005 // CRC-16-MAXIM (IBM)
#define POLY_MODBUS 0xA001 // CRC-16-MODBUS

#define POLYNOMIAL     0x1021
 
#define WIDTH   (8 * sizeof(resultType))
#define MSb     ((resultType)1 << (WIDTH - 1))

#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * /brief CRC 16 Calculation
     */
void CalcCrc16_Poly(uint16_t crc_initial, uint16_t poly, uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h);
resultType crc(readType * data, unsigned n, resultType remainder);
uint16_t crcFlash(uint32_t data, unsigned n, uint16_t remainder);


#ifdef	__cplusplus
}
#endif

#endif	/* QS_CRC16_H */

