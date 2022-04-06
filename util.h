/* 
 * File:   util.h
 * Author: Fernando Morani
 *
 * Created on 6 aprile 2022, 23.04
 */

#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "mcc_generated_files/mcc.h"
#include <string.h>
    
union U_WVAL {
short i;
uint8_t c[2];
};

typedef union U_WVAL WVAL;


void wxtoa(char *s, short n);
void bxtoa(char *s, uint8_t  n);



#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

