/*------------------------------------------------------------------------------
Archivo: mainsproject.s
Microcontrolador: PIC16F887
Autores: Pablo Herrarte y Andy Bonilla
Compilador: pic-as (v2.30), MPLABX v5.45
    
Programa: pic para sensores de proyecto 1 de Electronica Digital 2
Hardware: PIC16F887
    
Creado: 15 de agosto de 2021    
Descripcion: 
------------------------------------------------------------------------------*/

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT   //configuracion de oscilador interno
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

/*-----------------------------------------------------------------------------
 ----------------------------LIBRERIAS-----------------------------------------
 -----------------------------------------------------------------------------*/
#include <stdint.h>
#include <pic16f887.h>
#include <xc.h>
#include <proc/pic16f887.h>
#include "Osc_config.h"
#include "ADC_CONFIG.h"
#include "I2C.h"
/*-----------------------------------------------------------------------------
 ----------------------- VARIABLES A IMPLEMTENTAR------------------------------
 -----------------------------------------------------------------------------*/

//-------DIRECTIVAS DEL COMPILADOR
#define _XTAL_FREQ 8000000

//-------VARIABLES DE PROGRAMA
unsigned char antirrebote;
unsigned char infrarrojo1, infrarrojo2, infrarrojo3, suma_ir;
float conversion1, conversion_total, temperatura_aprox;
uint8_t abierto_cerrado, BASURA, PARQUEO; 
/*-----------------------------------------------------------------------------
 ------------------------ PROTOTIPOS DE FUNCIONES ------------------------------
 -----------------------------------------------------------------------------*/
void setup(void);
void infrarrojos(void);
/*-----------------------------------------------------------------------------
 --------------------------- INTERRUPCIONES -----------------------------------
 -----------------------------------------------------------------------------*/
void __interrupt() isr(void) //funcion de interrupciones
{
    //-------INTERRUPCION POR I2C
    if(PIR1bits.SSPIF == 1){    //Le va a mandar la cantidad de parqueos habilitados al master

        SSPCONbits.CKP = 0;
       
        if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)){
            abierto_cerrado = SSPBUF;                 // Read the previous value to clear the buffer
            SSPCONbits.SSPOV = 0;       // Clear the overflow flag
            SSPCONbits.WCOL = 0;        // Clear the collision bit
            SSPCONbits.CKP = 1;         // Enables SCL (Clock)
        }

        if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) 
        {
            abierto_cerrado = SSPBUF;                 // Lectura del SSBUF para limpiar el buffer y la bandera BF
            PIR1bits.SSPIF = 0;         // Limpia bandera de interrupci n recepci n/transmisi n SSP
            SSPCONbits.CKP = 1;         // Habilita entrada de pulsos de reloj SCL
            while(!SSPSTATbits.BF);     // Esperar a que la recepci n se complete
            BASURA = SSPBUF;             // Guardar en z el valor del buffer de recepci n
            __delay_us(200);
            
        }
        else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){
            abierto_cerrado = SSPBUF;
            BF = 0;
            SSPBUF = suma_ir;
            SSPCONbits.CKP = 1;
            __delay_us(200);
            while(SSPSTATbits.BF);
        }
       
        PIR1bits.SSPIF = 0;    
    }
   
}
/*-----------------------------------------------------------------------------
 ----------------------------- MAIN LOOP --------------------------------------
 -----------------------------------------------------------------------------*/
void main(void)
{
   
    setup();
    
    while(1)
    {
        
        //-------FUNCION PARA CONTROL DE SENSORES INFRARROJOS
        infrarrojos();
        
    }
   
}
/*-----------------------------------------------------------------------------
 ---------------------------------- SET UP -----------------------------------
 -----------------------------------------------------------------------------*/
void setup(void)
{
    //-------CONFIGURACION ENTRADAS ANALOGICAS
    ANSEL=0;
    ANSELH=0;
    ANSELbits.ANS0=1;                   //entrada analogica de sensor temperatura
    //-------CONFIGURACION IN/OUT
    TRISAbits.TRISA0=1;                 //entrada de sensor temperatura
    TRISBbits.TRISB1=1;                 //entrada de sensor IR1
    TRISBbits.TRISB2=1;                 //entrada de sensor IR2
    TRISBbits.TRISB3=1;                 //entrada de sensor IR3
    TRISEbits.TRISE0=0;                 //salida para led verde
    TRISEbits.TRISE1=0;                 //salida para led rojo
    TRISC=0;
    TRISD=0;
    //-------LIMPIEZA DE PUERTOS
    PORTB=0;
    PORTD=0;
    PORTE=0;
    //-------CONFIGURACION DE RELOJ A 4MHz
    osc_config(8);
    //-------CONFIGURACION DE ADC
    ADC_config();
    //-------CONFIGURACION DE COMUNICACION I2C
    I2C_Slave_Init(0x50);   //se da direccion 0x50
    //-------CONFIGURACION DE INTERRUPCIONES
    
    
}
/*-----------------------------------------------------------------------------
 --------------------------------- FUNCIONES ----------------------------------
 -----------------------------------------------------------------------------*/
//-------FUNCION PARA CONTROL DE SENSORES INFRARROJOS
void infrarrojos(void)
{
    //-------PARQUEO 1
    if (PORTBbits.RB1==1)
    {
        PORTDbits.RD0=1;
        PORTDbits.RD1=0;
        infrarrojo1=1;
    }
    if (PORTBbits.RB1==0)
    {
        PORTDbits.RD0=0;
        PORTDbits.RD1=1;
        infrarrojo1=0;
    }
    //-------PARQUEO2
    if (PORTBbits.RB2==1)
    {
        PORTDbits.RD2=1;
        PORTDbits.RD3=0;
        infrarrojo2=1;
    }
    if (PORTBbits.RB2==0)
    {
        PORTDbits.RD2=0;
        PORTDbits.RD3=1;
        infrarrojo2=0;
    }
    //-------PARQUEO3
    if (PORTBbits.RB3==1)
    {
        PORTDbits.RD4=1;
        PORTDbits.RD5=0;
        infrarrojo3=1;
    }
    if (PORTBbits.RB3==0)
    {
        PORTDbits.RD4=0;
        PORTDbits.RD5=1;
        infrarrojo3=0;
    }
    suma_ir=infrarrojo1+infrarrojo2+infrarrojo3; //suma de los parqueos disponibles
   
}
//-------FUNCION PARA CONVERSION DEL ADC
void toggle_adc(void)
{
    if (ADCON0bits.GO==0)
    {
        conversion1=ADRESH<<8;                  //toma los MSB del ADRE
        conversion_total=conversion1+ADRESL;    //le suma los LSB
        __delay_ms(1);                          //tiempo de carga
        ADCON0bits.GO=1;
    }
}

//-------FUNCION PARA MAPEO DE TEMPERATURA  