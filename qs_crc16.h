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

#define POLY_IBM    0x8005 // CRC-16-MAXIM (IBM)
#define POLY_MODBUS 0xA001 // CRC-16-MODBUS


#ifdef	__cplusplus
extern "C" {
#endif

    /**
     * /brief CRC 16 Calculation
     */
void CalcCrc16_Poly(uint16_t crc_initial, uint16_t poly, uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h);



#ifdef	__cplusplus
}
#endif

#endif	/* QS_CRC16_H */

