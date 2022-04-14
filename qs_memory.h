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

void FLASH_WriteHex(char *__strHex,  uint16_t __len);



#ifdef	__cplusplus
}
#endif

#endif	/* QS_MEMORY_H */

