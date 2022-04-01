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
void CalcCrc16_Poly(uint16_t crc_initial, uint16_t poly, uint8_t *pBuf, uint16_t wLen, uint8_t *crc_l, uint8_t *crc_h);

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

uint8_t answerBuf[240];

void proto_decoder(void)
{
    uint8_t nb;
    
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
                
                answerBuf[0] = 0x66;
                answerBuf[1] = 0x66;
                answerBuf[2] = 0x66;
                answerBuf[3] = 0x66;
                
                            // CRC ibm like
                CalcCrc16_Poly(0x0000, 0x8005, answerBuf, 4, &answerBuf[4], &answerBuf[5]);
                answerBuf[6] = 0x03;    // etx

                for(nb=0; nb<7; nb++)
                    UART5_Write(answerBuf[nb]);
             
                break;
        }
        
    }
    
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
        if( qsDecodPack.qs_Stx == 0 )   // frame invalidato ? (gi� decodificato)
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