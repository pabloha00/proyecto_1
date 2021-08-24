/*------------------------------------------------------------------------------
Archivo: mainsproject.s
Microcontrolador: PIC16F887
Autor: Pablo Herrarte y Andy Bonilla
Compilador: pic-as (v2.30), MPLABX v5.40
    
Programa: pic para motores de proyecto 1 de Electronica Digital 2
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
unsigned char antirrebote, botonazo;        //variables para control servo
unsigned char conversion1, conversion_total, temperatura_aprox;
uint8_t z = 0, motor_recibido, BASURA; 
/*-----------------------------------------------------------------------------
 ------------------------ PROTOTIPOS DE FUNCIONES ------------------------------
 -----------------------------------------------------------------------------*/
void setup(void);
void servo(void);
void toggle_adc(void);
void temp(void);
/*-----------------------------------------------------------------------------
 --------------------------- INTERRUPCIONES -----------------------------------
 -----------------------------------------------------------------------------*/
void __interrupt() isr(void) //funcion de interrupciones
{
    //-------INTERRUPCION POR BOTONAZO
    if (INTCONbits.RBIF)
    {
        if (PORTB==0b11111101)
            antirrebote=1;
        else
            antirrebote=0;
        INTCONbits.RBIF=0;
    }
    //-------INTERRUPCION POR I2C
    if(PIR1bits.SSPIF == 1){    //Le va a mandar la cantidad de parqueos habilitados al master

        SSPCONbits.CKP = 0;
       
        if ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)){
            motor_recibido = SSPBUF;                 // Read the previous value to clear the buffer
            SSPCONbits.SSPOV = 0;       // Clear the overflow flag
            SSPCONbits.WCOL = 0;        // Clear the collision bit
            SSPCONbits.CKP = 1;         // Enables SCL (Clock)
        }

        if(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) 
        {
            motor_recibido= SSPBUF;                 // Lectura del SSBUF para limpiar el buffer y la bandera BF
            PIR1bits.SSPIF = 0;         // Limpia bandera de interrupci n recepci n/transmisi n SSP
            SSPCONbits.CKP = 1;         // Habilita entrada de pulsos de reloj SCL
            while(!SSPSTATbits.BF);     // Esperar a que la recepci n se complete
            motor_recibido = SSPBUF;             // Guardar en z el valor del buffer de recepci n
            __delay_us(200);
            
        }
        else if(!SSPSTATbits.D_nA && SSPSTATbits.R_nW){
            z = SSPBUF;
            BF = 0;
            SSPBUF = temperatura_aprox;     //se manda le temperatura
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
        //-------FUNCION PARA SERVO CON BOTONAZO
        servo();
        //-------FUNCIONES PARA RECEPCION DE CONVERSION ADC
        toggle_adc();
        //-------FUNCIONES PARA 
        temp();
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
    ANSELbits.ANS0=1;
    //-------CONFIGURACION IN/OUT
    TRISDbits.TRISD0=0;
    TRISBbits.TRISB1=1;                 //entrada boton prueba
    TRISCbits.TRISC2=0;                 //salida de PWM de motor DC
    TRISD=0;
    TRISEbits.TRISE0=0;                 //salida para PWM de servo
    
    //-------LIMPIEZA DE PUERTOS
    PORTB=0;
    PORTC=0;
    PORTD=0;
    PORTE=0;
    //-------CONFIGURACION DE RELOJ A 4MHz
    osc_config(8);
    //-------CONFIGURACION DEL ADC
    ADC_config();
    //-------CONFIGURACION DE CCP
    TRISCbits.TRISC2=1;         // a motor se desconecta
    CCP1CONbits.P1M = 0;        //
    CCP1CONbits.CCP1M = 0b1100; // se configura como modo PWM
    CCPR1L = 0x0f ;             // ciclo de trabajo inicial de la onda cuadrada
    CCP1CONbits.DC1B = 0;       // LSB para ciclo de trabajo
    //TRISCbits.TRISC2=0;
    //-------CONFIGURACION DE WPUB
    OPTION_REGbits.nRBPU=0;             //se activan WPUB
    WPUBbits.WPUB1=1;                   //RB0, boton prueba
    //-------CONFIGURACION DE COMUNICACION I2C
    I2C_Slave_Init(0x60);               //se da direccion 0x50         
    //-------CONFIGURACION DE INTERRUPCIONES
    INTCONbits.GIE=1;                   //se habilita interrupciones globales
    INTCONbits.PEIE = 1;                //habilitan interrupciones por perifericos
    INTCONbits.RBIE=1;                  //se  habilita IntOnChange B
    INTCONbits.RBIF=0;                  //se  apaga bandera IntOnChange B
    IOCBbits.IOCB1=1;                   //habilita IOCB RB0
}
/*-----------------------------------------------------------------------------
 --------------------------------- FUNCIONES ----------------------------------
 -----------------------------------------------------------------------------*/
//-------FUNCION PARA FUNCION DE SERVO/TALANQUERA
void servo(void)
{
    //-------ANTIRREBOTE DE BOTON PARA MOVER MOTOR
    if (antirrebote==1 && PORTBbits.RB1==0  )
    {
        botonazo++;
        antirrebote=0;
    }
    //-------FUNCIONAMIENTO DE SERVO DE TALANQUERA
    switch(botonazo)
    {
        case(0):                //servo en posicion de 0°
            PORTEbits.RE0=1;
            __delay_ms(1);
            PORTEbits.RE0=0;
            break;
        case(1):                //servo en posicion 90°
            PORTEbits.RE0=1;    
            __delay_ms(2);
            PORTEbits.RE0=0;
            break;
        case(2):
            botonazo=0;         //regresa a posicion 0°
            break;
    }
}
//-------FUNCION PARA RECEPCION DEL ADC
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
//-------FUNCION PARA CONVERSION DE TEMPERATURA
void temp(void)
{
    temperatura_aprox=(conversion_total/2.046);
    //PORTD=temperatura_aprox;
    if (temperatura_aprox>28 && temperatura_aprox <50)
        TRISCbits.TRISC2=0;
    else
        TRISCbits.TRISC2=1;
}