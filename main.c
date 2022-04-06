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
#include <string.h>


#define QS_BOOTP_MAX_PAY_LEN    240

#define QS_BOOTP_OK                 0x00 /* Command acknowledge - Use in the payload*/
#define QS_BOOTP_FAIL               0x01 /* Command failed - Use in the payload*/

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


FW_SW_VERSION_T theFwVersion;


/*!
 *  \typedef Stuctrured type to be used as payload in the following commands:
 *  READ_REV_ID
 *  READ_DEV_ID.
 */
typedef struct
{
    uint8_t         ID_Info_Version;      /* Info Version  */
    /* 8 bytes */
} ID_INFO_VERSION_T;

ID_INFO_VERSION_T theRevisionID;
uint8_t theDeviceID = 0x23;


union U_WVAL {
	short i;
	uint8_t c[2];
	};

typedef union U_WVAL WVAL;


void proto_entry(void);
void proto_parser(uint8_t __newChar);
void proto_decoder(void);
void CalcCrc16_Poly(uint16_t crc_initial, uint16_t poly, uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h);
void wxtoa(char *s, short n);
void bxtoa(char *s, uint8_t  n);

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

static uint8_t answerBuf[240];
static uint8_t statoDecoder;
static uint8_t lenTxDecoder;
static uint8_t cntTxDecoder;
static uint8_t cmdDecoder;
static uint8_t crcDecoderL;
static uint8_t crcDecoderH;

void proto_decoder(void)
{
    uint8_t nb;
    
    
    switch( statoDecoder )
    {
        case    0:  // attesa nuovo pacco da decodificare 
            if( qsDecodPack.qs_Stx == 0x02 )    // valid pack ?
            {
                qsDecodPack.qs_Stx = 0x00;
                cmdDecoder = 0;
                
                switch( qsDecodPack.qs_CmdId )
                {
                    case    QS_BOOTP_READ_FW:
                        cmdDecoder = QS_BOOTP_READ_FW;
                        memcpy(answerBuf, &theFwVersion, sizeof(theFwVersion));
 
                        lenTxDecoder = sizeof(theFwVersion);       // lunghezza payload
                        break;
                        
                    case    QS_BOOTP_READ_REV:
                        
                        cmdDecoder = QS_BOOTP_READ_REV;
                        answerBuf[0] = theRevisionID.ID_Info_Version;
                        answerBuf[1] = 0x00;
                        answerBuf[2] = 0x00;
                        answerBuf[3] = 0x00;

                        lenTxDecoder = 4;       // lunghezza payload
                        break;
                        
                    case    QS_BOOTP_READ_DEV:
                        
                        cmdDecoder = QS_BOOTP_READ_DEV;
                        answerBuf[0] = theDeviceID;
                        answerBuf[1] = 0x00;
                        answerBuf[2] = 0x00;
                        answerBuf[3] = 0x00;

                        lenTxDecoder = 4;       // lunghezza payload
                        break;
                        
                    case    QS_BOOTP_READ_BOOT:
                        cmdDecoder = QS_BOOTP_READ_BOOT;
                        memcpy(answerBuf, &theFwVersion, sizeof(theFwVersion));
 
                        lenTxDecoder = sizeof(theFwVersion);       // lunghezza payload
                        break;                        
                        
                    case    QS_BOOTP_RESET:
                        
                        cmdDecoder = QS_BOOTP_RESET;
                        answerBuf[0] = 0x66;
                        answerBuf[1] = 0x66;
                        answerBuf[2] = 0x66;
                        answerBuf[3] = 0x66;

                        lenTxDecoder = 4;       // lunghezza payload
                        break;

                    case    QS_BOOTP_ERASE:
                        cmdDecoder = QS_BOOTP_ERASE;
                        answerBuf[0] = QS_BOOTP_OK;
                        answerBuf[1] = qsDecodPack.qs_Payload[0];
                        answerBuf[2] = qsDecodPack.qs_Payload[1];
                        answerBuf[3] = '1';                        

                        lenTxDecoder = 4;       // lunghezza payload
                        break;
                        
                    case    QS_BOOTP_READ_FLASH:
                        cmdDecoder = QS_BOOTP_READ_FLASH;
                        answerBuf[0] = '2';
                        answerBuf[1] = '2';
                        answerBuf[2] = '2';
                        answerBuf[3] = '2';

                        lenTxDecoder = 4;       // lunghezza payload
                        break;
                        
                    case    QS_BOOTP_WRITE_FLASH:
                        cmdDecoder = QS_BOOTP_WRITE_FLASH;
                        answerBuf[0] = '0';
                        answerBuf[1] = '2';
                        answerBuf[2] = '2';
                        answerBuf[3] = '2';

                        lenTxDecoder = 4;       // lunghezza payload
                        break;
                        
                    case    QS_BOOTP_START_FW_UP:
                        cmdDecoder = QS_BOOTP_START_FW_UP;
                        answerBuf[0] = '2';
                        answerBuf[1] = '2';
                        answerBuf[2] = '2';
                        answerBuf[3] = '2';

                        lenTxDecoder = 4;       // lunghezza payload
                        break;
                }

                if( cmdDecoder != 0 )   // se trova un comando valido ...
                {                                
                    
                    // CRC ibm like
                    CalcCrc16_Poly(0x0000, 0x8005, answerBuf, lenTxDecoder, &crcDecoderL, &crcDecoderH);
                    
                    answerBuf[lenTxDecoder++] = crcDecoderL; 
                    answerBuf[lenTxDecoder++] = crcDecoderH;
                    answerBuf[lenTxDecoder++] = 0x03;    // etx
                    statoDecoder++;                 // pronto per l'invio
                }
            }        
            break;
            
        case    1:  // invio risposta comando
            UART5_Write(0x02);
            UART5_Write(0x04);
            UART5_Write(0x23);
            UART5_Write(0xA3);
            UART5_Write(cmdDecoder|0x80);
            cntTxDecoder = 0;
            statoDecoder++;       // fine risposta
            break;

        case    2:  // invio dati in coda
            if( cntTxDecoder < lenTxDecoder)
                UART5_Write(answerBuf[cntTxDecoder++]);
            else
                statoDecoder = 0;       // fine risposta
            break;
    }
    
}


/**
 * static int hex(char ch)
 * 
 * @ch: ascii char to be converted into int
 * 
int hex(char ch)
{
    if ((ch >= 'a') && (ch <= 'f'))
	return (ch - 'a' + 10);
    if ((ch >= '0') && (ch <= '9'))
	return (ch - '0');
    if ((ch >= 'A') && (ch <= 'F'))
	return (ch - 'A' + 10);
    return (-1);
}

 */


/*
			valueCom.c[0] = hex(sRx[4]) << 4;		// incamera vin_h
			valueCom.c[0] += hex(sRx[5]);

  */          



/* Converte uno short in ascii esadecimale
*/
void wxtoa(char *s, short n)
{
WVAL wn;

	wn.i = n;				// copia

	bxtoa(s+2, wn.c[1]);			// prima la parte alta ...
	bxtoa(s, wn.c[0]);		// prima la parte alta ...
}


/* Converte byte in ascii esadecimale
*/
void bxtoa(char *s, uint8_t  n)
{
uint8_t b;

	b = n & 0x0F;

	if( b > 9 )
		s[1] = (b - 10) + 'A';
	else
		s[1] = b + '0';

	b = (n & 0xF0) >> 4;

	if( b > 9 )
		s[0] = (b - 10) + 'A';
	else
		s[0] = b + '0';
}



#define POLY_IBM    0x8005 // CRC-16-MAXIM (IBM)
#define POLY_MODBUS 0xA001 // CRC-16-MODBUS


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

            // CRC standard per modbus
            // CalcCrc16_Poly(0xFFFF, 0xA001, qsDecodPack.qs_Payload, qsDecodPack.qs_PayLen, &qsDecodPack.qs_CrcLow, &qsDecodPack.qs_CrcHigh);
            
            // CRC ibm like
            CalcCrc16_Poly(0x0000, 0x8005, qsDecodPack.qs_Payload, qsDecodPack.qs_PayLen, &qsDecodPack.qs_CrcLow, &qsDecodPack.qs_CrcHigh);
         
            /*
            if( parserCrcL != qsDecodPack.qs_CrcLow ||
                parserCrcH != qsDecodPack.qs_CrcHigh )
            {
                qsDecodPack.qs_Stx = 0x00;          // abortisce il frame come valido
            }
            */
            statoParser = 0;        // ok puo' incamerare un altro frame
        }
        
    }
    
}


void CalcCrc16_Poly(uint16_t crc_initial, uint16_t poly, uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h)
{
uint8_t i,j;
uint16_t data;
uint16_t  crc = crc_initial;
uint8_t  crc_low, crc_hig;

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


/**
 End of File
*/