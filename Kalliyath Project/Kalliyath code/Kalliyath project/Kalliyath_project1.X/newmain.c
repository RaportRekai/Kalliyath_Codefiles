
#include <xc.h>
#include <pic18f4550.h>
#include "hdf.h"
#include "stdint.h"
#include "stdbool.h"
#include <stdio.h>
#include "string.h"
#define RS LATD0                   /* PORTD 0 pin is used for Register Select */
#define EN LATD1                   /* PORTD 1 pin is used for Enable */
#define ldata LATB                 /* PORTB is used for transmitting data to LCD */

#define LCD_Port TRISB              
#define LCD_Control TRISD

void LCD_Init();
void LCD_Command(char );
void LCD_Char(char x);
void LCD_String(const char *);
void LCD_String_xy(char ,char ,const char*);
void MSdelay(unsigned int );
void LCD_Clear();
uint8_t data = 255;
float r = 8;
#define APPLIED_DC 9
#define CORRECTION_FACTOR 5

/*************************Main Program***************************/
void uart_init(uint16_t gen_reg, unsigned sync, unsigned brgh,unsigned brg16)
{
    
    TRISCbits.RC7=1;
    TRISCbits.RC6 = 1;
    SPBRGH = (gen_reg & 0xFF00)>>8;
    SPBRG = gen_reg & 0x00FF;
    RCSTAbits.CREN = 1;
    RCSTAbits.SPEN = 1;
    BAUDCONbits.BRG16 = brg16;
    
    TXSTAbits.SYNC = sync;
    TXSTAbits.BRGH = brgh;
    TXSTAbits.TXEN = 1;
    
    IPR1bits.RCIP = 1;
    PIE1bits.RCIE = 1;
    
//    IPR1bits.TXIP = 0;
//    PIE1bits.TXIE = 1;
    
    
    
}

void uart_send(float *c){
    TXREG = *c;
    while(TXSTAbits.TRMT == 0)
    {
        Nop();
    }
    
        
}




void main(void)
{       
    OSCCON=0x72;                   /* Use Internal Oscillator with Frequency 8MHZ */ 
    LCD_Init();                    /* Initialize 16x2 LCD */
    //LCD_String_xy(1,5,"Hello");    /* Display string at location(row,location). */
                                   /* This function passes string to display */
  
    uint16_t avg,sum;
    float a;
    int k;
    char b[15];
    TRISAbits.RA0 =1;
    
   // LCD_Clear();
    
    //LCD_Clear();
    
    //ADCON1
    ADCON1bits.VCFG0 = 0;
    ADCON1bits.VCFG1 = 0; 
    ADCON1bits.PCFG = 0x0E;
    //
    
    RCONbits.IPEN = 1;
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
    
     OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 0b10;
    while(OSCCONbits.IOFS == 0);
    uart_init(51,0,1,0);
    
    // ADCON2 
    ADCON2bits.ADFM = 1;
    ADCON2bits.ACQT = 0b0001;
    ADCON2bits.ADCS = 0b100;
    //
    
    //ADCON0
    ADCON0bits.ADON = 1;
    ADCON0bits.CHS = 0b0000;
    //
    
   //
    uint8_t cnt_cycle = 0;
   
    
    while(1)	
    {
        //LATBbits.LATB0 = 0;
        //
//         uart_send(&data);
//         __delay_ms(200);
        
        
       
//      MSdelay(100000);
//        
        if(cnt_cycle == 20)//depends upon the frequency of the oscillator
        {
            
            avg = sum/cnt_cycle; 
            data = avg/16;
            a = 4.785*ADRESL/(5 -(0.004897*ADRESL));
             
            a = (a/100 - 1)/0.0039083;
            a= a-5;
            k = a;
            sprintf(b, "%d", k);
            LCD_String_xy(1,2,b);
               
//            a = ADRESL;
//            uart_send(&a);
            cnt_cycle = 1;
            sum = 0;
            
        }
        else
        {
            sum = sum+ADRESL;
             
            cnt_cycle = cnt_cycle+1;
             
        }
            //
       ADCON0bits.GODONE = 1;
       while(ADCON0bits.GODONE ==1);
 
    }
}

/************************Functions****************************/
void LCD_Init()
{
    MSdelay(15);           /* 15ms,16x2 LCD Power on delay */
    LCD_Port = 0x00;       /* Set PORTB as output PORT for LCD data(D0-D7) pins */
    LCD_Control = 0x00;    /* Set PORTD as output PORT LCD Control(RS,EN) Pins */
    LCD_Command(0x38);     /* uses 2 line and initialize 5*7 matrix of LCD */
    LCD_Command(0x01);     /* clear display screen */
    LCD_Command(0x0c);     /* display on cursor off */
    LCD_Command(0x06);     /* increment cursor (shift cursor to right) */
}

void LCD_Clear()
{
    	LCD_Command(0x01); /* clear display screen */
}

void LCD_Command(char cmd )
{
	ldata= cmd;            /* Send data to PORT as a command for LCD */   
	RS = 0;                /* Command Register is selected */
	EN = 1;                /* High-to-Low pulse on Enable pin to latch data */ 
	NOP();
	EN = 0;
	MSdelay(3);	
}

void LCD_Char(char dat)
{
	ldata= dat;            /* Send data to LCD */  
	RS = 1;                /* Data Register is selected */
	EN=1;                  /* High-to-Low pulse on Enable pin to latch data */   
	NOP();
	EN=0;
	MSdelay(1);
}


void LCD_String(const char *msg)
{
	while((*msg)!=0)
	{		
	  LCD_Char(*msg);
	  msg++;	
    	}
}

void LCD_String_xy(char row,char pos,const char *msg)
{
    char location=0;
    if(row<=1)
    {
        location=(0x80) | ((pos) & 0x0f); //Print message on 1st row and desired location/
        LCD_Command(location);
    }
    else
    {
        location=(0xC0) | ((pos) & 0x0f); //Print message on 2nd row and desired location/
        LCD_Command(location);    
    }  
    LCD_String(msg);

}
/*****************************Delay Function****************************/
void MSdelay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<val;i++)
            for(j=0;j<165;j++);      /*This count Provide delay of 1 ms for 8MHz Frequency */
}

void __interrupt(high_priority) tcInt(void)
{
    
INTCONbits.GIEH = 0;

if(PIR1bits.RCIF == 1)
{
    PIR1bits.RCIF = 0;
}

 // process other interrupt sources here, if required
INTCONbits.GIEH = 1;
 return;
}

void __interrupt(low_priority) tcInt1(void)
{
INTCONbits.GIEH = 0;
if(PIR1bits.TXIF == 1){
    PIR1bits.TXIF = 0;
}
    

 // process other interrupt sources here, if required
INTCONbits.GIEH = 1;
 return;
}