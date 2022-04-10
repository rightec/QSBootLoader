/**
 * @file qs_bootprotocolstruct.h
 * @author Fernando Morani
 * @date 27 Mar 2022
 * @brief Defines the boot protocol structure
 *
 */
#ifndef QS_BOOTPROTOCOLSTRUCT_H
#define QS_BOOTPROTOCOLSTRUCT_H

#include <stdint.h>
#include "qs_bootprotocoldef.h"

#define QS_BOOTP_STX_POS        0  /*Start position of STX in the protocol struct*/
#define QS_BOOTP_LEN_POS        1  /*Start position of lenght in the protocol struct*/
#define QS_BOOTP_SND_POS        3  /*Start position of sender in the protocol struct*/
#define QS_BOOTP_POL_POS        4  /*Start position of policy in the protocol struct*/
#define QS_BOOTP_CMD_POS        5  /*Start position of command id in the protocol struct*/
#define QS_BOOTP_PAY_POS        6  /*Start position of payload in the protocol struct*/

/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  READ_FW_VERSION
 *  READ_BOOT_VERSION.
 */
typedef struct
{
    uint16_t        FW_Erp_Identifier;   /* codice identificativo del FW */
    uint8_t         FW_Erp_Version;      /* Minor Version del FW */
    uint8_t         FW_Erp_BuildNumber;  /* incremental Build Number del FW */
    uint32_t        FW_Erp_Crc32;        /* CRC32 del FW */
    /* 8 bytes */
} FW_SW_VERSION_T;


/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  READ_REV_ID
 *  READ_DEV_ID.
 */
typedef struct
{
    uint16_t         ID_Info;      /* Info Version  */
    /* 2 bytes */
} ID_INFO_VERSION_T;

/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  ERASE
 *  WRITE  when coming from FW or BootLoader and it fails
 */
typedef struct
{
    uint8_t         BANK_Info_Ack;         /* QS_BOOTP_OK if the command has been executed or requested
                                            QS_BOOTP_FAIL if not*/
    uint8_t         BANK_Info_Number;      /* Bank number to erase or WRITE */
} BANK_INFO_T;

/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  READ when coming from FW or BootLoader and it fails
 *  If READ flash is OK, it fill the payloads with all the requested bytes *
 */
typedef struct
{
    uint8_t         READ_Info_Ack;         /* QS_BOOTP_FAIL */
    uint8_t         READ_Info_Number;      /* Number of bytes not red */
} READ_INFO_ANSWER_T;


/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  READ when coming from SW
 */
typedef struct
{
    uint16_t         READ_Address;         /* Flash Address to read */
    uint16_t         READ_Info_Number;     /* Number of bytes to read */
} READ_FROM_FLASH_T;

/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  WRITE when coming from SW
 */
typedef struct
{
    uint8_t         WRITE_Block;            /* Block to write */
    uint8_t         WRITE_element;          /* Element to write */
}WRITE_TO_FLASH_T;


/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  WRITE when coming from SW
 */
typedef struct
{
    uint8_t         WRITE_Block[QS_BOOTP_BLOCK_SIZE];            /* Block to write */
    uint8_t         WRITE_LEN;
}WRITE_TO_BUFFER_FLASH_T;


typedef struct QS_bootProt{
    uint8_t qs_Stx;                                 /*STX: Start of Message*/
    uint16_t qs_PayLen;                             /*Payload lenght from 3 to 1024*/
                                                    /*Minimum payload lenght = 3*/

    uint8_t qs_Sender;                              /* Sender of the packet (node source address)
                                                    sender Address (0x20 for PC, 0x23 for Board)*/

    uint8_t qs_Policy;

        /* Destination address : status + ID = .0x80 = boot attivo
                                               .0x40 = debug attivo
                                               0x00-0x3F -> device ID 0-64
         *
         * 00 10 0011
         *    ----------> ID             (0x00->0x3F)
         *  ------------> debug /release  (1=debug)
         * -------------> boot /app      (1=boot)
         *  */

    uint8_t qs_CmdId;                               /*Command identifier field*/
    uint8_t qs_Payload[QS_BOOTP_MAX_PAY_LEN];       /*Payload field*/
    uint8_t qs_CrcLow;                              /*CRC-16 low byte*/
    uint8_t qs_CrcHigh;                             /*CRC-16 high byte*/
    uint8_t qs_Etx;                                 /*ETX: End of Message*/
} QS_BOOT_PROT_T;



#endif /* QS_BOOTPROTOCOLSTRUCT_H*/
