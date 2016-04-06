#include "general.h"
#include "gpio.h"
#include "eeprom.h"

void EepromStart()
{
    //The Start bit is detected by the device if CS and DI are
    //both high with respect to the positive edge of CLK for
    //the first time:
    EEPROM_CLK_PIN=LOW;             //Clk falling edge
    EEPROM_CS_PIN=HIGH;
    EEPROM_DI_PIN=HIGH;
    __delay_us(CLK_DELAY);
    EEPROM_CLK_PIN=HIGH;            //Clk rising edge
    __delay_us(CLK_DELAY);
    EEPROM_CLK_PIN=LOW;             //Clk falling edge
    __delay_us(CLK_DELAY/2);
}
void EepromEnd()
{
    EEPROM_CLK_PIN=LOW;             //Clk falling edge
    EEPROM_CS_PIN=LOW;
    EEPROM_DI_PIN=LOW;
    __delay_us(CLK_DELAY);
}
void EepromSend(uint8_t b)
{
    int8_t i=0;
    for(i=7;i>=0;i--)
    {
    EEPROM_DI_PIN=b>>i;             //&OPCODE_MASK;      //second bit
    __delay_us(CLK_DELAY/2);
    EEPROM_CLK_PIN=HIGH;            //Clk rising edge
    __delay_us(CLK_DELAY);
    EEPROM_CLK_PIN=LOW;             //Clk falling edge
    __delay_us(CLK_DELAY/2);
    }
}
uint8_t EepromReceive()
{
    uint8_t b=0;
    int8_t i=0;
    for(i=7;i>=0;i--)
    {
        __delay_us(CLK_DELAY/2);
        EEPROM_CLK_PIN=HIGH;            //Clk rising edge
        __delay_us(CLK_DELAY/2);
        b=b+(EEPROM_DO_PIN<<i);
        __delay_us(CLK_DELAY/2);
        EEPROM_CLK_PIN=LOW;             //Clk falling edge
        __delay_us(CLK_DELAY/2);
    }
    return b;
}
void EepromOpCode(uint8_t opcode, uint16_t address)
{
    int8_t i=0;
    EEPROM_DI_PIN=opcode>>1&OPCODE_MASK;      //second bit
    __delay_us(CLK_DELAY/2);
    EEPROM_CLK_PIN=HIGH;            //Clk rising edge
    __delay_us(CLK_DELAY);
    EEPROM_CLK_PIN=LOW;             //Clk falling edge
    __delay_us(CLK_DELAY/2);
    //OP code 3rd bit
    EEPROM_DI_PIN=opcode&OPCODE_MASK;      //third bit
    __delay_us(CLK_DELAY/2);
    EEPROM_CLK_PIN=HIGH;            //Clk rising edge
    __delay_us(CLK_DELAY);
    EEPROM_CLK_PIN=LOW;             //Clk falling edge
    __delay_us(CLK_DELAY/2);

    for(i=10;i>=0;i--)
    {
        EEPROM_DI_PIN=address>>i&OPCODE_MASK;      //second bit
        __delay_us(CLK_DELAY/2);
        EEPROM_CLK_PIN=HIGH;            //Clk rising edge
        __delay_us(CLK_DELAY);
        EEPROM_CLK_PIN=LOW;             //Clk falling edge
        __delay_us(CLK_DELAY/2);
    }
}
void EepromWrite(uint16_t address, uint8_t data)
{
    EepromStart();
    EepromOpCode(EWEN, 0x600);
    EepromEnd();

    EepromStart();
    EepromOpCode(WRITE, address);
    EepromSend(data);
    EepromEnd();
}

uint8_t EepromRead(uint16_t address)
{
    uint8_t data=0;

    EepromStart();
    EepromOpCode(READ, address);
    data=EepromReceive();
    EepromEnd();
    return data;
}

uint8_t EEPROMSaveNumber(const char *nmbr, uint8_t nmbrLenght, uint16_t position)
{
    uint8_t count=0,result=1;
    uint16_t address=0;
    address=position*EEPROM_NUMBER_LENGHT;
    address=address+(EEPROM_NUMBER_LENGHT-1);

    for(count=EEPROM_NUMBER_LENGHT;count>0;count--)
    {
        if(nmbrLenght)
        {
            nmbrLenght--;
            if(*(nmbr+nmbrLenght)>='0' && *(nmbr+nmbrLenght)<='9')
                {
                    EepromWrite(address,*(nmbr+nmbrLenght));
                }
        }
        else
        {
            EepromWrite(address,'\0');
        }
        address--;
        __delay_ms(TWC_DELAY);
    }
    return result;
}

int8_t EEPROMSearchNumber(const char *nmbr, uint8_t nmbrLenght)
{
    uint16_t aux=0;
    uint8_t count=0, result=0;

    aux=EEPROM_NUMBER_LENGHT-1;
    if(nmbrLenght>EEPROM_NUMBER_LENGHT) nmbrLenght=EEPROM_NUMBER_LENGHT;
    
    while(!result && aux<(EEPROM_MAX-EEPROM_NUMBER_LENGHT))
    {
        count=0;
        do{
            if(EepromRead(aux-count)==*(nmbr+EEPROM_NUMBER_LENGHT-(count+1)))
                {result=1;}
            else
               {result=0;}
            count++;
        }while(result && count<nmbrLenght);
        aux=aux+EEPROM_NUMBER_LENGHT;
    }
    if(result==TRUE)
    {
        return (aux/EEPROM_NUMBER_LENGHT)+1;
    }
    else return FALSE;
}

void EEPROMEraseAll(void)
{
    EEPROM_CS_PIN=LOW;
    __delay_ms(100);
    EepromStart();
    EepromOpCode(EWEN, 0x600);
    EepromEnd();

    EepromStart();
    EepromOpCode(ERAL, 0x400);
    EepromEnd();
    __delay_ms(500);
}


uint8_t EEPROMAdd(const char *nmbr, uint8_t nmbrLenght)
{
    uint16_t empty=0;
    do
    {
        if(EepromRead((empty*EEPROM_NUMBER_LENGHT))<'0' || EepromRead((empty*EEPROM_NUMBER_LENGHT))>'9')
        {
            empty=TRUE;
        }
        empty++;
    }while(!empty && empty<EEPROM_MAX);
    EEPROMSaveNumber(nmbr,nmbrLenght,empty);

    return TRUE;
}