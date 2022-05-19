/* 
 * File:   qs_proto.c
 * Author: Fernando Morani
 *
 * Created on 7 aprile 2022, 8.32
 */

#include "qs_memory.h"
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

extern char *bootString; 

void proto_init(void) 
{
    /*
    strcpy(qsDecodPack.qs_Payload, ":040000000DEF00F010\n:0400080079EF06F096\n:10001800FF0052EF05F01BEF02F06300FCF7E8F475");
    qsDecodPack.qs_PayLen = strlen(qsDecodPack.qs_Payload);
    
    FLASH_WriteHex(qsDecodPack.qs_Payload,  qsDecodPack.qs_PayLen);
    */
    theRevisionID.ID_Info = FLASH_ReadWord(0x3FFFFC); 
    theFamilyDeviceID.ID_Info = FLASH_ReadWord(0x3FFFFE);
    theReadInfo.READ_Info_Ack = QS_BOOTP_OK;
    theReadInfo.READ_Info_Number = 0;
    theBankInfo.BANK_Info_Ack  = QS_BOOTP_OK;
    theBankInfo.BANK_Info_Number = 0;
    
    theAppFwVersion.FW_Erp_Crc32 = FLASH_ReadLong(APPCRC_FLASH_ADDR);
    theAppFwVersion.FW_Erp_BuildNumber = FLASH_ReadByte(0x2008);
    theAppFwVersion.FW_Erp_Identifier = FLASH_ReadByte(0x200A);
    theAppFwVersion.FW_Erp_Version = FLASH_ReadByte(0x200C);

    theBootFwVersion.FW_Erp_Crc32 = FLASH_ReadLong(0x1FFC);
    theBootFwVersion.FW_Erp_BuildNumber = FLASH_ReadByte(0x1FF0);
    theBootFwVersion.FW_Erp_Identifier = FLASH_ReadByte(0x1FF2);
    theBootFwVersion.FW_Erp_Version = FLASH_ReadByte(0x1FF4);
   
    
 //   FLASH_EraseBlock(0x3000);
 //    FLASH_WriteSingleWord(0x3000, 0x5566);    // scrive in flash
     
//     theBankInfo.BANK_Info_Number = FLASH_ReadWord(0x3000); 
   
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
uint32_t local_mem_addr;
uint8_t local_mem_cnt;
uint32_t local_page_addr;

    

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
/*                        for (int i = 1; i < QS_BOOTP_MIN_PAY_LEN; i++){
                            lenTxDecoder++;
                            answerBuf[lenTxDecoder] = QS_BOOTP_VOID_PAYLOAD;
                        }
 * */
                        break;
                    case    QS_BOOTP_ERASE:
                        cmdDecoder = QS_BOOTP_ERASE;
    
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL
                      
                        if( qsDecodPack.qs_Policy != 0xFF )     // se è dummy ... simula
                        {
                            if( qsDecodPack.qs_Payload[1] == QS_BOOTP_ERASE_ALL )   // delete all ?
                            {
                                local_mem_addr = 0x2000; 
                                
                                if( theFamilyDeviceID.ID_Info == QS_PIC18F47Q43_DEV_ID )
                                    local_page_addr = 0x1F000;   // last address = 0x1E000;
                                else
                                    local_page_addr = 0x0F000;   // last address = 0x0E000;
                                
                                while( local_mem_addr < local_page_addr)
                                {
                                    
                                    FLASH_EraseBlock(local_mem_addr);
                                    
                                    local_mem_addr += 0x100;    // prossima pagina
                                
                                    if( local_mem_addr == 0xF000 )
                                    {
                                        local_mem_addr = 0x10000; 
                                    }
                                
                                }
                            }
                            else
                            {
                                local_mem_addr = 0x2000;  // prima pagina utile
                                
                                local_page_addr = qsDecodPack.qs_Payload[1];   // last address = 0x1E000;
                                
                                local_page_addr <<= 8;

                                local_page_addr += 0x2000;

                                while( local_mem_addr < local_page_addr)
                                {
                                    FLASH_EraseBlock(local_mem_addr);
                                    
                                    local_mem_addr += 0x100;    // prossima pagina
                                }
                            }
                        }
                            // Disable interrupts before changing states
    
                        answerBuf[lenTxDecoder++] = qsDecodPack.qs_Payload[0];
                        answerBuf[lenTxDecoder++] = qsDecodPack.qs_Payload[1];

                        break;
                        
                    case    QS_BOOTP_READ_FLASH:
                        cmdDecoder = QS_BOOTP_READ_FLASH;
                        
                        theReadInfo.READ_Info_Number = qsDecodPack.qs_Payload[2];
                        
                        // READ_INFO_ANSWER_T
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL
                        answerBuf[lenTxDecoder++] = theReadInfo.READ_Info_Ack; // QS_BOOTP_OK or QS_BOOTP_FAIL // '2';
                        answerBuf[lenTxDecoder++] = theReadInfo.READ_Info_Number; // Bytes red 

                        //Rd_Address;         /* Flash Address to read */
                        //Rd_Info_Number;     /* Number of bytes to read */

                        local_mem_addr = qsDecodPack.qs_Payload[1]; // piglia il numero del banco

                        local_mem_addr <<= 8;                       // trasforma in offset
                        
                        local_mem_addr += qsDecodPack.qs_Payload[0];                       // trasforma in offset
                        local_mem_addr += 0x2000;                   // somma la base per l'app
                        
                        local_mem_cnt = qsDecodPack.qs_Payload[2];
                        
                        for(nb=0;nb<local_mem_cnt;nb++)
                            answerBuf[lenTxDecoder++] = FLASH_ReadByte(local_mem_addr);
                        break;
                        
                    case    QS_BOOTP_WRITE_FLASH:
                        
                        if( qsDecodPack.qs_Policy != 0xFF )     // se è dummy ... simula
                        {
                            FLASH_WriteHex( (const char *) qsDecodPack.qs_Payload,  qsDecodPack.qs_PayLen);
                        }
                        else
                        {
                            theBankInfo.BANK_Info_Number++;
                        }
                        cmdDecoder = QS_BOOTP_WRITE_FLASH;
                        // BANK_INFO_T
                        answerBuf[lenTxDecoder++] = QS_BOOTP_OK; // QS_BOOTP_OK or QS_BOOTP_FAIL
                        answerBuf[lenTxDecoder++] = theBankInfo.BANK_Info_Ack; // QS_BOOTP_OK or QS_BOOTP_FAIL // '2';
                        answerBuf[lenTxDecoder++] = theBankInfo.BANK_Info_Number; // Bank number
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
                    answerBuf[3] = 0x23;   // SENDER
                    answerBuf[4] = 0xA3;   // POLICY
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
                if( cmdDecoder == QS_BOOTP_RESET )          // reset secco ?
                {
                    while( !UART5_is_tx_ready() );      // aspetta che tutti i dati escano ....
                    
                    strcpy(bootString, "Reset Boot!");      // chiede un banale reset

                    __delay_ms(10);                         // attesa extra per gli ultimi caratteri
                    RESET();                                // si riparte !!
                    
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
            
            parserLen = __newChar;      // copia la lunghezza
            
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
    
        // bufferizza il comando per debug (solo con pic18q47)
//    if( statoParser != 0 && statoParser != 10  )   // parser attivo ?
//        parserBuffer[parserIdx++] = __newChar;

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
            statoParser = 0;        // ok. We can parse another frame
        }
        
    }
    
}

