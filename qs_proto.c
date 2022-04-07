/* 
 * File:   qs_proto.c
 * Author: Fernando Morani
 *
 * Created on 7 aprile 2022, 8.32
 */

#include "qs_proto.h"

FW_SW_VERSION_T theFwVersion;
ID_INFO_VERSION_T theRevisionID;
QS_BOOT_PROT_T  qsDecodPack;        // Data Packets to decode

static uint8_t answerBuf[240];
static uint8_t statoDecoder;
static uint8_t lenTxDecoder;
static uint8_t cntTxDecoder;
static uint8_t cmdDecoder;
static uint8_t crcDecoderL;
static uint8_t crcDecoderH;

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

uint8_t theDeviceID = 0x23;


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

void proto_decoder(void)
{  
    
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

