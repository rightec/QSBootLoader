/* 
 * File:   qs_memory.h
 * Author: Fernando Morani
 *
 * Created on 10 aprile 2022, 9.24
 */

#ifndef QS_MEMORY_H
#define	QS_MEMORY_H

#include <xc.h>
#include <stdint.h>
#include "util.h"

#ifdef	__cplusplus
extern "C" {
#endif

 
void FLASH_WriteHex(const char *__strHex,  uint16_t __len);
void FLASH_WriteSingleWord(uint32_t __flashAddr, uint16_t __word_value);

void FLASH_CalcCrc32Lsb(uint32_t crc_initial, uint32_t poly, 
                            uint32_t __flashStartAddr, uint32_t __flashEndAddr, uint32_t *__crc_val);


void FLASH_CalcCrc32Msb(uint32_t crc_initial, uint32_t poly, 
                            uint32_t __flashStartAddr, uint32_t __flashEndAddr, uint32_t *__crc_val);
 
uint32_t FLASH_ReadLong(uint32_t flashAddr);

#ifdef	__cplusplus
}
#endif

#endif	/* QS_MEMORY_H */

