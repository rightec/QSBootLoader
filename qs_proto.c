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

static uint8_t answerBuf[256];
static uint8_t statoDecoder;
static uint16_t lenTxDecoder;
static uint16_t lenTxPayload;
static uint16_t cntTxDecoder;
static uint8_t cmdDecoder;
static uint8_t crcDecoderL;
static uint8_t crcDecoderH;


//static uint8_t parserBuffer[QS_BOOTP_MAX_CMD_LEN];

static uint8_t statoParser;
static uint16_t parserIdx;
static uint16_t parserLen;
static uint8_t parserSender;
static uint8_t parserPolicy;
static uint8_t parserCmd;
static uint8_t parserPayBuf[QS_BOOTP_MAX_PAY_LEN];
static uint16_t parserPayIdx;
static uint8_t parserCrcL;
static uint8_t parserCrcH;
static uint8_t parserEtx;

uint8_t theDeviceID = 0x23;


void proto_init(void) 
{
    theRevisionID.ID_Info_ChipRevision = FLASH_ReadWord(0x3FFFFC); 

    theRevisionID.ID_Info_ChipID = FLASH_ReadWord(0x3FFFFE);
    
    /*
    strcpy(qsDecodPack.qs_Payload, ":040000000DEF00F010\n:0400080079EF06F096\n:10001800FF0052EF05F01BEF02F06300FCF7E8F475");
    qsDecodPack.qs_PayLen = strlen(qsDecodPack.qs_Payload);
    
    FLASH_WriteHex(qsDecodPack.qs_Payload,  qsDecodPack.qs_PayLen);
    */
    
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

void proto_decoder(void)
{  
    
    switch( statoDecoder )
    {
        case    0:  // attesa nuovo pacco da decodificare 
            if( qsDecodPack.qs_Stx == 0x02 )    // valid pack ?
            {
                qsDecodPack.qs_Stx = 0x00;
                cmdDecoder = 0;
                lenTxDecoder = 0;
                
                answerBuf[lenTxDecoder++] = 0;   // STX
                answerBuf[lenTxDecoder++] = 0;   // LEN L
                answerBuf[lenTxDecoder++] = 0;   // LEN H
                answerBuf[lenTxDecoder++] = 0;   // SENDER
                answerBuf[lenTxDecoder++] = 0;   // POLICY
                answerBuf[lenTxDecoder++] = 0;   // CMD

                
                switch( qsDecodPack.qs_CmdId )
                {
                    case    QS_BOOTP_READ_FW:
                        cmdDecoder = QS_BOOTP_READ_FW;
                        memcpy(&answerBuf[lenTxDecoder], &theFwVersion, sizeof(theFwVersion));
 
                        lenTxDecoder += sizeof(theFwVersion);       // lunghezza payload
                        break;
                        
                    case    QS_BOOTP_READ_REV:
                        
                        cmdDecoder = QS_BOOTP_READ_REV;
                        answerBuf[lenTxDecoder++] = theRevisionID.ID_Info_ChipRevision & 0xFF;
                        answerBuf[lenTxDecoder++] = theRevisionID.ID_Info_ChipRevision >> 8;
                        answerBuf[lenTxDecoder++] = 0x00;
                        answerBuf[lenTxDecoder++] = 0x00;
                        break;
                        
                    case    QS_BOOTP_READ_DEV:
                        
                        cmdDecoder = QS_BOOTP_READ_DEV;
                        answerBuf[lenTxDecoder++] = theRevisionID.ID_Info_ChipID & 0xFF;
                        answerBuf[lenTxDecoder++] = theRevisionID.ID_Info_ChipID >> 8;
                        answerBuf[lenTxDecoder++] = 0x00;
                        answerBuf[lenTxDecoder++] = 0x00;
                        break;
                        
                    case    QS_BOOTP_READ_BOOT:
                        cmdDecoder = QS_BOOTP_READ_BOOT;
                        memcpy(answerBuf, &theFwVersion, sizeof(theFwVersion));
 
                        lenTxDecoder += sizeof(theFwVersion);       // lunghezza payload
                        break;                        
                        
                    case    QS_BOOTP_RESET:
                        
                        cmdDecoder = QS_BOOTP_RESET;
                        answerBuf[lenTxDecoder++] = 0x66;
                        answerBuf[lenTxDecoder++] = 0x66;
                        answerBuf[lenTxDecoder++] = 0x66;
                        answerBuf[lenTxDecoder++] = 0x66;

                        break;

                    case    QS_BOOTP_ERASE:
                        cmdDecoder = QS_BOOTP_ERASE;
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK;
                        answerBuf[lenTxDecoder++] = qsDecodPack.qs_Payload[0];
                        answerBuf[lenTxDecoder++] = qsDecodPack.qs_Payload[1];
                        answerBuf[lenTxDecoder++] = '1';                        
                        break;
                        
                    case    QS_BOOTP_READ_FLASH:
                        cmdDecoder = QS_BOOTP_READ_FLASH;
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        break;
                        
                    case    QS_BOOTP_WRITE_FLASH:
                        
                        FLASH_WriteHex(qsDecodPack.qs_Payload,  qsDecodPack.qs_PayLen);

                        cmdDecoder = QS_BOOTP_WRITE_FLASH;
                        answerBuf[lenTxDecoder++] = '0';
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        break;
                        
                    case    QS_BOOTP_START_FW_UP:
                        cmdDecoder = QS_BOOTP_START_FW_UP;
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        answerBuf[lenTxDecoder++] = '2';
                        break;
                }

                if( cmdDecoder != 0 )   // se trova un comando valido ...
                {         
                    
                    lenTxPayload = lenTxDecoder - 6;
                    answerBuf[0] = 0x02;   // STX
                    answerBuf[1] = lenTxPayload & 0xFF;   // LEN L
                    answerBuf[2] = lenTxPayload >> 8;   // LEN H
                    answerBuf[3] = 0x23;   // SENDER
                    answerBuf[4] = 0xA3;   // POLICY
                    answerBuf[5] = cmdDecoder | 0x80;   // CMD                    
                    
                    
                    // CRC ibm like
                    CalcCrc16_Poly(0x0000, 0x8005, &answerBuf[1], lenTxPayload + 5, &crcDecoderL, &crcDecoderH);
                    
                    answerBuf[lenTxDecoder++] = crcDecoderL; 
                    answerBuf[lenTxDecoder++] = crcDecoderH;
                    answerBuf[lenTxDecoder++] = 0x03;    // etx
                    statoDecoder++;                 // pronto per l'invio
                }
            }        
            break;
            
        case    1:  // invio risposta comando
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
    if(  statoParser == 3 && 
         statoParser == 4 &&    
         statoParser == 5 &&    
         statoParser == 6  )      
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

        case    1:      // attesa len L
            
            parserLen = __newChar;      // copia la lunghezza
            
            statoParser++;
            break;

        case    2:      // attesa len H
            
            parserLen |= ((uint16_t) __newChar) << 8;      // copia la lunghezza
            
            if( parserLen <= 2 || parserLen > 1024 )   // len non valida  ?
            {
                statoParser = 0;    // reset macchinino a stati
            }
            else
            {
                statoParser++;                
            }
            break;
            
        case    3:      // attesa qs_Sender
            
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

        case    4:      // attesa qs_Policy
            
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
            
        case    5:      // attesa qs_Cmd
            
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
            
        case    6:      // attesa qs_Payload
            
            parserPayBuf[parserPayIdx++] = __newChar;      // copia il dato
            
            if( parserPayIdx >= parserLen )     // cmd non previsto ?
            {
                statoParser++;                
            }
            break;         
                       
        case    7:      // attesa qs_CrcLow
            
            parserCrcL = __newChar;      // copia crc L
            statoParser++; 
            break;         

        case    8:      // attesa qs_CrcHigh
            
            parserCrcH = __newChar;      // copia crc H

            statoParser++; 
            break;         
            
        case    9:      // attesa qs_Etx
            
            parserEtx = __newChar;      // copia etx
            
//            if( parserPayIdx >= parserLen )     // crc ok ?
            
            statoParser = 10;                
            break;         

        case    10:      // frame valido: puo' passare al decoder
            break;
            
    }
    
        // bufferizza il comando per debug (solo con pic18q47)
//    if( statoParser != 0 && statoParser != 10  )   // parser attivo ?
//        parserBuffer[parserIdx++] = __newChar;

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
            CalcCrc16_Poly(0x0000, 0x8005, 
                                    (uint8_t *) &qsDecodPack.qs_PayLen, 
                                    qsDecodPack.qs_PayLen + 5, 
                                    &qsDecodPack.qs_CrcLow, &qsDecodPack.qs_CrcHigh);
         
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

