/* 
 * File:   qs_proto.c
 * Author: Fernando Morani
 *
 * Created on 7 aprile 2022, 8.32
 */

#include "qs_proto.h"


FW_SW_VERSION_T theAppFwVersion;
FW_SW_VERSION_T theBootFwVersion;

ID_INFO_VERSION_T theRevisionID;
ID_INFO_VERSION_T theFamilyDeviceID;
READ_INFO_ANSWER_T theReadInfo;
BANK_INFO_T theBankInfo;

QS_BOOT_PROT_T  qsDecodPack;        // Data Packets to decode

static uint8_t answerBuf[260];
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

char *bootString = (char *) 0x500;      // Info area shared between Bootloader and application


void proto_init(void) 
{

    theRevisionID.ID_Info = FLASH_ReadWord(0x3FFFFC); 
    theFamilyDeviceID.ID_Info = FLASH_ReadWord(0x3FFFFE);
    theReadInfo.READ_Info_Ack = QS_BOOTP_OK;
    theReadInfo.READ_Info_Number = 0;
    theBankInfo.BANK_Info_Ack  = QS_BOOTP_OK;
    theBankInfo.BANK_Info_Number = 0;
    
    
    theAppFwVersion.FW_Erp_Crc32 = FLASH_ReadLongBE(0x2000);
    theAppFwVersion.FW_Erp_BuildNumber = FLASH_ReadByte(0x2008);
    theAppFwVersion.FW_Erp_Identifier = FLASH_ReadByte(0x200A);
    theAppFwVersion.FW_Erp_Version = FLASH_ReadByte(0x200C);

    theBootFwVersion.FW_Erp_Crc32 = FLASH_ReadLongBE(0x1FFC);
    theBootFwVersion.FW_Erp_BuildNumber = FLASH_ReadByte(0x1FF0);
    theBootFwVersion.FW_Erp_Identifier = FLASH_ReadByte(0x1FF2);
    theBootFwVersion.FW_Erp_Version = FLASH_ReadByte(0x1FF4);
}

// entrypoint protocol manger
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
uint16_t nb;

   
    switch( statoDecoder )
    {
        case    0:  // waiting for new message to decode
            if( qsDecodPack.qs_Stx == 0x02 )    // valid message ?
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
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL;
                        memcpy(&answerBuf[lenTxDecoder], &theAppFwVersion, sizeof(theAppFwVersion)); 
                        lenTxDecoder += sizeof(theAppFwVersion);       // payload lenght
                        break;         
                        
                    case    QS_BOOTP_READ_REV:                        
                        cmdDecoder = QS_BOOTP_READ_REV;
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL;
                        answerBuf[lenTxDecoder++] = theRevisionID.ID_Info & 0xFF;
                        answerBuf[lenTxDecoder++] = theRevisionID.ID_Info >> 8;
                        break;
                        
                    case    QS_BOOTP_READ_DEV:                        
                        cmdDecoder = QS_BOOTP_READ_DEV;
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL;
                        answerBuf[lenTxDecoder++] = theFamilyDeviceID.ID_Info & 0xFF;
                        answerBuf[lenTxDecoder++] = theFamilyDeviceID.ID_Info >> 8;
                        break;
                        
                    case    QS_BOOTP_READ_BOOT:
                        cmdDecoder = QS_BOOTP_READ_BOOT;
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL;
                        memcpy(&answerBuf[lenTxDecoder], &theBootFwVersion, sizeof(theBootFwVersion));
                        lenTxDecoder += sizeof(theBootFwVersion);       // payload lenght
                        break;                        
                        
                    case    QS_BOOTP_RESET:                        
                        cmdDecoder = QS_BOOTP_RESET;
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL
                        answerBuf[lenTxDecoder++] = QS_BOOTP_VOID_PAYLOAD; // QS_BOOTP_VOID_PAYLOAD
                        answerBuf[lenTxDecoder++] = QS_BOOTP_VOID_PAYLOAD; // QS_BOOTP_VOID_PAYLOAD
                        break;
                        
                    case    QS_BOOTP_ERASE:
                        //cmdDecoder = QS_BOOTP_ERASE;  // Not used in application
                        
                        break;
                        
                    case    QS_BOOTP_READ_FLASH:
                        //cmdDecoder = QS_BOOTP_READ_FLASH;   // Not used in application
                        break;
                        
                    case    QS_BOOTP_WRITE_FLASH:
                        // cmdDecoder = QS_BOOTP_WRITE_FLASH;   // Not used in application
                        break;
                        
                    case    QS_BOOTP_START_FW_UP:
                        cmdDecoder = QS_BOOTP_START_FW_UP;
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK;  // QS_BOOTP_OK or QS_BOOTP_FAIL 
                        answerBuf[lenTxDecoder++] = QS_BOOTP_VOID_PAYLOAD; 
                        answerBuf[lenTxDecoder++] = QS_BOOTP_VOID_PAYLOAD; 
                        break;
                }

                if( cmdDecoder != 0 )   // is a valid command?
                {         
                    
                    lenTxPayload = (lenTxDecoder - QS_BOOTP_PAY_POS);  
                    answerBuf[0] = 0x02;   // STX
                    answerBuf[1] = lenTxPayload & 0xFF;   // LEN L
                    answerBuf[2] = lenTxPayload >> 8;   // LEN H
                    answerBuf[3] = 0x21;   // SENDER
                    answerBuf[4] = 0xA1;   // POLICY
                    answerBuf[5] = cmdDecoder | 0x80;   // CMD                    
                    
                    
                    // CRC ibm like
                    CalcCrc16_Poly(0x0000, 0x8005, &answerBuf[1], lenTxPayload + 5, &crcDecoderL, &crcDecoderH);
                    
                    answerBuf[lenTxDecoder++] = crcDecoderL; 
                    answerBuf[lenTxDecoder++] = crcDecoderH;
                    answerBuf[lenTxDecoder++] = 0x03;    // etx
                    statoDecoder++;                 // ready to send
                }
            }        
            break;
            
        case    1:  // send command answer
            cntTxDecoder = 0;
            statoDecoder++;       // Answer end
            break;

        case    2:  // send data to queue
            if( cntTxDecoder < lenTxDecoder)
            {
                if( UART5_is_tx_ready() )
                    UART5_Write(answerBuf[cntTxDecoder++]);
            }
            else
            {
                
                if( cmdDecoder == QS_BOOTP_RESET )          
                {
                    while( !UART5_is_tx_ready() );          // Wait for all data to be tarnsmitted
                    
                    bootString = (char *) 0x500;            // area ram scambio info tra boot e applicazione

                    strcpy(bootString, "Reset Boot!");      // Write RESET TAG

                    __delay_ms(10);                         // Delay for last tx char
                    RESET();                                // Restart
                    
                }
               
                if( cmdDecoder == QS_BOOTP_START_FW_UP )    
                {
                    while( !UART5_is_tx_ready() );           // Wait for all data to be tarnsmitted
                    
                    strcpy(bootString, "Stop Boot!");        // Write STOP BOOT TAG

                    __delay_ms(10);                         // Delay for last tx char
                    RESET();                                // Restart   
                }

                statoDecoder = 0;       // Answer End
            }
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
        case    0:      // wait for  sync on STX
            if( __newChar == 0x02 )     // Start packet
            {
                parserIdx = 0;
                statoParser++;
            }
            break;

        case    1:      // wait for len L
            
            parserLen = __newChar;      // Copy the len
            
            statoParser++;
            break;

        case    2:      // wait for  len H
            
            parserLen |= ((uint16_t) __newChar) << 8;      // copy len
            
            if( parserLen <= 2 || parserLen > 1024 )   // is len valid  ?
            {
                statoParser = 0;    // reset state machine
            }
            else
            {
                statoParser++;                
            }
            break;
            
        case    3:      // wait for qs_Sender
            
            parserSender = __newChar;      // copy src address
            
            if( parserSender != 0x20 )     // source different from PC ?
            {
                statoParser = 0;    // reset state machine
            }
            else
            {
                statoParser++;                
            }
            break;

        case    4:      // wait for qs_Policy
            
            parserPolicy = __newChar;      // copy dst address
            
            if( parserPolicy != 0x23 && parserPolicy != 0xFF )     // dest different from my ID  ?
            {
                statoParser = 0;    // reset state machine
            }
            else
            {
                statoParser++;                
            }
            break;
            
        case    5:      // wait for qs_Cmd
            
            parserCmd = __newChar;      // copy dst address
            
            if( parserCmd < 0x40 )     // cmd not allowed ?
            {
                statoParser = 0;    // reset state machine
            }
            else
            {
                parserPayIdx = 0;
                statoParser++;                
            }
            break;         
            
        case    6:      // wait for qs_Payload
            
            parserPayBuf[parserPayIdx++] = __newChar;      // copy data
            
            if( parserPayIdx >= parserLen )     // is len correct ?
            {
                statoParser++;                
            }
            break;         
                       
        case    7:      // wait for qs_CrcLow
            
            parserCrcL = __newChar;      // copy crc L
            statoParser++; 
            break;         

        case    8:      // wait for qs_CrcHigh
            
            parserCrcH = __newChar;      // copy crc H

            statoParser++; 
            break;         
            
        case    9:      // wait for qs_Etx
            
            parserEtx = __newChar;      // copy etx
            
//            if( parserPayIdx >= parserLen )     // crc ok ?
            
            statoParser = 10;                
            break;         

        case    10:      // frame valid: decoding
            break;
            
    }
   

        // frame ready ?
    if( statoParser == 10 )
    {
        if( qsDecodPack.qs_Stx == 0 )   // frame decoded ?
        {
            qsDecodPack.qs_Stx = 0x02;          // set frame as valid
            qsDecodPack.qs_PayLen = parserLen;             
            qsDecodPack.qs_Sender = parserSender;             
            qsDecodPack.qs_Policy = parserPolicy;                              
            qsDecodPack.qs_CmdId = parserCmd;        
            
            uint8_t nb;
            
            for(nb=0;nb<parserLen;nb++)
                qsDecodPack.qs_Payload[nb] = parserPayBuf[nb];    
            
            // CRC ibm like
            CalcCrc16_Poly(0x0000, 0x8005, 
                                    (uint8_t *) &qsDecodPack.qs_PayLen, 
                                    qsDecodPack.qs_PayLen + 5, 
                                    &qsDecodPack.qs_CrcLow, &qsDecodPack.qs_CrcHigh);
         
            statoParser = 0;        // ok. We can parse another frame
        }
        
    }
    
}

