#include "general.h"
#include "sim800l.h"
#include "usart.h"
#include "gpio.h"
#include "eeprom.h"

uint8_t SIM800init()
{
    SIM800_RST_PIN=HIGH;
    __delay_ms(10);
    SIM800_RST_PIN=LOW;
    __delay_ms(500);
    SIM800_RST_PIN=HIGH;
    __delay_ms(10000);
    USARTWriteLine("AT\r\n");       //Test connection
    __delay_ms(2000);
    USARTWriteLine("ATE0\r\n");     //disable Echo
    __delay_ms(2000);
    /*if (sendCommand("AT")) {
        sendCommand("AT+IPR=115200");
        sendCommand("ATE0");
        sendCommand("AT+CFUN=1", 10000);
        return TRUE;
    }*/
    return FALSE;
}

uint8_t SIM800sendCommand()
{
    USARTWriteLine("AT\n\r");       //Test connection
    return 0;
}

uint8_t SIM800SendSms(const char *nmbr, const char *msg)
{
    __delay_ms(2000);
    USARTWriteLine("AT+CMGF = 1\r\n");       //Text format
    __delay_ms(2000);
    USARTWriteLine("AT+CMGS=\"");     //Send SMS
    USARTWriteString(nmbr);
    USARTWriteString("\"\r\n");
    //Wait for >
    __delay_ms(2000);
    USARTWriteString(msg);
    USARTWriteChar(CTRLZ);
    __delay_ms(2000);

    return 1;
}
uint8_t SIM800ReadSms(const char *mem)
{
    __delay_ms(2000);
    USARTWriteLine("AT+CMGF = 1\r\n");       //Text format
    __delay_ms(2000);
    USARTWriteLine("AT+CMGR=");     //Read SMS
    USARTWriteString(mem);
    USARTWriteString("\r\n");
    __delay_ms(2000);
    return 1;
}
uint8_t SIM800DeleteSms(const char *index, const char *flag)
{
    __delay_ms(2000);
    USARTWriteLine("AT+CMGF = 1\r\n");       //Text format
    __delay_ms(2000);
    USARTWriteLine("AT+CMGD=");     //Delete SMS
    USARTWriteString(index);
    USARTWriteChar(',');
    USARTWriteString(flag);
    USARTWriteString("\r\n");
    return 1;
}
uint8_t SIM800Process()
{
    if(SIM800L.buffer[0]=='+')                         //Case for unsolicited messages
    {
        if(SIM800L.buffer[1]=='C' && SIM800L.buffer[2]=='L' && SIM800L.buffer[3]=='I'  )    //+CLIP, save caller's cellnumber
        {
            uint8_t ci=8;                               //Cell number initiates at eight position [+CLIP: "89..."
            while(SIM800L.buffer[ci]!='\"' && (ci-8)<SIM800L_CELL_LENGHT)             //Save characters until " arrive
            {
                SIM800L.cell[ci-8]=SIM800L.buffer[ci];  //Save cell number characters
                ci++;
                SIM800L.cell_lenght++;                  //Save cell number lenght
            }
            task=CALL_IN;                               //Configure task to CALL_IN
        }
        if(SIM800L.buffer[1]=='C' && SIM800L.buffer[2]=='M' && SIM800L.buffer[3]=='T' && SIM800L.buffer[4]=='I'  )    //+CMTI, save SMS
         {

            task=SMS_IN;                               //Configure task to SMS_IN
         }
         if(SIM800L.buffer[1]=='C' && SIM800L.buffer[2]=='S' && SIM800L.buffer[3]=='Q')    //+CSQ, save CSQ
         {
            uint8_t ci=6;
            while(SIM800L.buffer[ci]!=',' && (ci)<8)             //Save characters until , arrive
            {
               SIM800L.csq[ci-6]=SIM800L.buffer[ci];  //Save cell number characters
               ci++;
            }
            SIM800L.csq[ci-6]=0;
          }
    }
    return TRUE;
}

uint8_t SIM800Command()
{
    if(SIM800L.command[0]==':')
    {
        if(SIM800L.command[1]=='A' && SIM800L.command[2]=='D' && SIM800L.command[3]=='D' )
        {
            i=5;
            while(SIM800L.command[i]>='0' && SIM800L.command[i]<='9' )
            {
                SIM800L.cell[i-5]=SIM800L.command[i];  //Save cell number characters
                i++;
                SIM800L.cell_lenght++;
            }
            EEPROMAdd(SIM800L.cell, SIM800L.cell_lenght);
        }              
    }
    return TRUE;  
}
