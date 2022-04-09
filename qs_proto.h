/* 
 * File:   qs_proto.h
 * Author: Fernando Morani
 *
 * Created on 7 aprile 2022, 8.32
 */

#ifndef QS_PROTO_H
#define	QS_PROTO_H

#include "mcc_generated_files/mcc.h"
#include <string.h>
#include "qs_bootprotocolstruct.h"
#include "qs_crc16.h"
#include "util.h"


#ifdef	__cplusplus
extern "C" {
#endif

void proto_init(void);

void proto_entry(void);
void proto_parser(uint8_t __newChar);
void proto_decoder(void);




#ifdef	__cplusplus
}
#endif

#endif	/* QS_PROTO_H */

