/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC18F46Q43
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"

#define QS_BOOTP_MAX_PAY_LEN    240

int count;

typedef struct QS_bootProt{
    uint8_t qs_Stx;                                 /*STX: Start of Message*/
    uint8_t qs_PayLen;                             /*Payload lenght from 1 to 256 ---> Expressed as 0 to 255*/
                                                    /*Minimum payload lenght = 1*/
                                                    
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

void proto_entry(void);
void proto_parser(uint8_t __newChar);
void proto_decoder(void);
void CalcCrc16(uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h);
void CalcCrc16_Poly(uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h);
void CalcCrc16_IBM(uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h);

/*
                         Main application
 */
void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    while (1)
    {
        
        count++;

        proto_entry();  // entrypoint manger protocollo
        
        if( count > 30000 )
        {
            count = 0;
            LED_LIFE_Toggle();
        
            // UART5_Write('a');
        }
        
        // Add your application code
    }
}




 // entrypoint manger protocollo
void proto_entry(void) 
{
uint8_t c;
    
    
    if( UART5_is_rx_ready() )
    {
        c = UART5_Read();
        
        proto_parser(c);
    }

    proto_decoder();

    
}


#define QS_BOOTP_READ_FW        0x41
#define QS_BOOTP_READ_REV       0x42
#define QS_BOOTP_READ_DEV       0x43
#define QS_BOOTP_READ_BOOT      0x44
#define QS_BOOTP_RESET          0x50
#define QS_BOOTP_ERASE          0x51
#define QS_BOOTP_READ_FLASH     0x52
#define QS_BOOTP_WRITE_FLASH    0x53
#define QS_BOOTP_START_FW_UP    0x70


QS_BOOT_PROT_T  qsDecodPack;        // pacchetto dati per il decoder

void proto_decoder(void)
{
    
    if( qsDecodPack.qs_Stx == 0x02 )    // valid pack ?
    {
        qsDecodPack.qs_Stx = 0x00;
        
        switch( qsDecodPack.qs_CmdId )
        {
            case    QS_BOOTP_RESET:
                UART5_Write(0x02);
                UART5_Write(0x04);
                UART5_Write(0x23);
                UART5_Write(0xA3);
                UART5_Write(QS_BOOTP_RESET|0x80);
                UART5_Write(0x66);
                UART5_Write(0x66);
                UART5_Write(0x66);
                UART5_Write(0x66);
                UART5_Write(0x22);
                UART5_Write(0x33);
                UART5_Write(0x03);
                break;
        }
        
    }
    
}



static uint8_t statoParser;

static uint8_t parserBuffer[255];
static uint8_t parserIdx;
static uint8_t parserLen;
static uint8_t parserSender;
static uint8_t parserPolicy;
static uint8_t parserCmd;
static uint8_t parserPayBuf[255];
static uint8_t parserPayIdx;
static uint8_t parserCrcL;
static uint8_t parserCrcH;
static uint8_t parserEtx;


void proto_parser(uint8_t __newChar)
{

        // sync char out of sync ? 
    if(  statoParser != 0 && 
         statoParser != 6 &&    // disattiva il resync sul crc
         statoParser != 7 &&   
         statoParser != 10    // e a frame in coda per il decode
    )      
    {
        if( __newChar == 0x02 )     // resync, out of sync ?
            statoParser = 0;
    }
    
    switch( statoParser )
    {
        case    0:      // attesa sync su STX
            if( __newChar == 0x02 )     // inizio pacchetto ?
            {
                parserIdx = 0;
                statoParser++;
            }
            break;

        case    1:      // attesa len
            
            parserLen = __newChar;      // copia la lunghezza
            
            if( parserLen == 0 || parserLen > 240 )     // inizio pacchetto ?
            {
                statoParser = 0;    // reset macchinino a stati
            }
            else
            {
                statoParser++;                
            }
            break;
    
        case    2:      // attesa qs_Sender
            
            parserSender = __newChar;      // copia il src address
            
            if( parserSender != 0x20 )     // source diverso dal PC ?
            {
                statoParser = 0;    // reset macchinino a stati
            }
            else
            {
                statoParser++;                
            }
            break;

        case    3:      // attesa qs_Policy
            
            parserPolicy = __newChar;      // copia il dst address
            
            if( parserPolicy != 0x23 )     // dest diverso dal mio ID  ?
            {
                statoParser = 0;    // reset macchinino a stati
            }
            else
            {
                statoParser++;                
            }
            break;
            
        case    4:      // attesa qs_Cmd
            
            parserCmd = __newChar;      // copia il dst address
            
            if( parserCmd < 0x40 )     // cmd non previsto ?
            {
                statoParser = 0;    // reset macchinino a stati
            }
            else
            {
                parserPayIdx = 0;
                statoParser++;                
            }
            break;         
            
        case    5:      // attesa qs_Payload
            
            parserPayBuf[parserPayIdx++] = __newChar;      // copia il dato
            
            if( parserPayIdx >= parserLen )     // cmd non previsto ?
            {
                statoParser++;                
            }
            break;         
                       
        case    6:      // attesa qs_CrcLow
            
            parserCrcL = __newChar;      // copia crc L
            statoParser++; 
            break;         

        case    7:      // attesa qs_CrcHigh
            
            parserCrcH = __newChar;      // copia crc H

            statoParser++; 
            break;         
            
        case    8:      // attesa qs_Etx
            
            parserEtx = __newChar;      // copia etx
            
//            if( parserPayIdx >= parserLen )     // crc ok ?
            
            statoParser = 10;                
            break;         

        case    10:      // frame valido: puo' passare al decoder
            break;
            
    }
    
        // bufferizza il comando per debug
    if( statoParser != 0 && statoParser != 10  )   // parser attivo ?
        parserBuffer[parserIdx++] = __newChar;

        // frame validato pronto ?
    if( statoParser == 10 )
    {
        if( qsDecodPack.qs_Stx == 0 )   // frame invalidato ? (già decodificato)
        {
            qsDecodPack.qs_Stx = 0x02;          // marca il frame come valido
            qsDecodPack.qs_PayLen = parserLen;             
            qsDecodPack.qs_Sender = parserSender;             
            qsDecodPack.qs_Policy = parserPolicy;                              
            qsDecodPack.qs_CmdId = parserCmd;        
            
            uint8_t nb;
            
            for(nb=0;nb<parserLen;nb++)
                qsDecodPack.qs_Payload[nb] = parserPayBuf[nb];    
            
//            CalcCrc16(qsDecodPack.qs_Payload, qsDecodPack.qs_PayLen, &qsDecodPack.qs_CrcLow, &qsDecodPack.qs_CrcHigh);
//            CalcCrc16_IBM(qsDecodPack.qs_Payload, qsDecodPack.qs_PayLen, &qsDecodPack.qs_CrcLow, &qsDecodPack.qs_CrcHigh);
            CalcCrc16_Poly(qsDecodPack.qs_Payload, qsDecodPack.qs_PayLen, &qsDecodPack.qs_CrcLow, &qsDecodPack.qs_CrcHigh);
                    
            if( parserCrcL != qsDecodPack.qs_CrcLow ||
                parserCrcH != qsDecodPack.qs_CrcHigh )
            {
                qsDecodPack.qs_Stx = 0x00;          // abortisce il frame come valido
            }
            
            statoParser = 0;        // ok puo' incamerare un altro frame
        }
        
    }
    
}



/* The function returns the CRC as a unsignbyte *crc_led short type */
/* uint8_t *pBuf -> message to calculate CRC upon */
/* uint16_t wDataLen -> quantity of bytes in message */
void CalcCrc16(uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h)
{
uint8_t bCRCHi = 0xFF ;    /* high byte of CRC initialized (modbus) */
uint8_t bCRCLo = 0xFF ;    /* low byte of CRC initialized  (modbus) */
uint8_t wIndex;            /* will index into CRC lookup table */

/* Table of CRC values for high–order byte */
static const uint8_t crcHiTab[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
};

static const uint8_t crcLoTab[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
};

	if( wLen < 256 )
	{

	    while( wLen-- ) /* pass through message buffer */
	    {
	        wIndex = bCRCLo ^ *pBuf++ ; /* calculate the CRC */
	        bCRCLo = bCRCHi ^ crcHiTab[wIndex] ;
	        bCRCHi = crcLoTab[wIndex] ;
	    }
 	}

	*crc_l = bCRCLo;
    *crc_h = bCRCHi;
}



#define POLY 0x8005 // CRC-16-MAXIM (IBM) (or 0xA001)

void CalcCrc16_Poly(uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h)
{
uint8_t i,j;
uint16_t data;
//uint16_t  crc = 0xFFFF;//0xFFFF;
uint16_t  crc = 0x0000;
uint8_t  crc_low, crc_hig;//0xFFFF;

count = 0;

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
                
                crc ^= POLY;    // somma il poly
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


/*
 * CRC lookup table for bytes, generating polynomial is 0x8005
 * input: reflexed (LSB first)
 * output: reflexed also...
 */

const uint16_t crc_ibm_table[256] = {
  0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
  0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
  0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
  0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
  0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
  0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
  0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
  0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
  0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
  0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
  0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
  0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
  0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
  0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
  0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
  0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
  0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
  0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
  0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
  0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
  0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
  0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
  0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
  0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
  0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
  0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
  0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
  0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
  0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
  0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
  0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
  0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
};

static inline uint16_t crc_ibm_byte(uint16_t crc, const uint8_t c)
{
    const unsigned char lut = (crc ^ c) & 0xFF;
    return (crc >> 8) ^ crc_ibm_table[lut];
}



/**
 * crc_ibm - recompute the CRC for the data buffer
 * @crc - previous CRC value
 * @buffer - data pointer
 * @len - number of bytes in the buffer
 */
uint16_t crc_ibm(uint16_t crc, uint8_t const *buffer, size_t len)
{
        while (len--)
                crc = crc_ibm_byte(crc, *buffer++);
        return crc;
}


void CalcCrc16_IBM(uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h)
{
uint16_t crc = 0xFFFF;
uint8_t  crc_low, crc_hig;//0xFFFF;

    while (wLen--)
        crc = crc_ibm_byte(crc, *pBuf++);

    crc_low = crc & 0xFF;
    crc_hig = crc >> 8;
    
    *crc_l = crc_low;
    *crc_h = crc_hig;

}


/**
 End of File
*/