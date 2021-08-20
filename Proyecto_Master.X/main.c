/*
 * File:   main.c
 * Author: Pablo
 * Ejemplo de uso de I2C Master
 * Created on 17 de febrero de 2020, 10:32 AM
 */
//*****************************************************************************
// Palabra de configuraci n
//*****************************************************************************
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
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

//*****************************************************************************
// Definici n e importaci n de librer as
//*****************************************************************************
#include <stdint.h>
#include <pic16f887.h>
#include "I2C.h"
#include "Osc_config.h"
#include "ASCII.h"
#include "LCD.h"
#include <xc.h>
//*****************************************************************************
// Definici n de variables
//*****************************************************************************
#define _XTAL_FREQ 8000000
uint8_t PARKH;
uint8_t NUM;
uint8_t BASURA;
uint8_t DIA;
uint8_t HORA;
uint8_t MIN;
uint8_t C1;
uint8_t C2;
uint8_t C3;
uint8_t UH;
uint8_t DH;
uint8_t UM;
uint8_t DM;
uint8_t con;
//*****************************************************************************
// Definici n de funciones para que se puedan colocar despu s del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void);
void LECT1(void);
const char* conver(void); //Datos que recibir� la LCD
const char* conver1(void);
//*****************************************************************************
// Main
//*****************************************************************************
void main(void) {
    setup();
    Lcd_Init(); //Inicializaci�n de 8 bits para LCD
    while(1){
        Lcd_Set_Cursor(1,1);    //Cursor en primera l�nea
        Lcd_Write_String(conver1()); //Escribir S1 S2 S3
        Lcd_Set_Cursor(2,1);    //Cursor en segunda l�nea
        Lcd_Write_String(conver());    //Escribir los datos para el LCD con esa funci�n
        LECT1();
        
        I2C_Master_Start();
        I2C_Master_Write(0x50);
        I2C_Master_Write(0);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0x51);
        PARKH = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
        
        I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x02);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0xD1);
        HORA = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
        
        I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x03);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0xD1);
        DIA = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
        
        I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x01);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0xD1);
        MIN = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
    }
    return;
}
//*****************************************************************************
// Funci n de Inicializaci n
//*****************************************************************************
void setup(void){
    ANSEL = 0;
    ANSELH = 0;
    TRISA = 0;
    TRISB = 0;
    TRISD = 0;
    TRISE = 0;
    PORTA = 0;
    PORTB = 0;
    PORTD = 0;
    osc_config(8);
    I2C_Master_Init(100000);        // Inicializar Comuncaci n I2C
    
    I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x02);
        I2C_Master_Write(0x23);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0xD1);
        BASURA = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
        
        I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x01);
        I2C_Master_Write(0x30);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0xD1);
        BASURA = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
        
        I2C_Master_Start();
        I2C_Master_Write(0xD0);
        I2C_Master_Write(0x03);
        I2C_Master_Write(0x04);
        I2C_Master_Stop();
        __delay_ms(10);
       
        I2C_Master_Start();
        I2C_Master_Write(0xD1);
        BASURA = I2C_Master_Read(0);
        I2C_Master_Stop();
        __delay_ms(10);
}

const char* conver(void){   //Datos que recivir� la LCD
    char temporal[16];
    temporal[0] = 0x50; //P
    temporal[1] = 0x41; //A
    temporal[2] = 0x52; //R
    temporal[3] = 0x51; //Q
    temporal[4] = 0x55; //U
    temporal[5] = 0x45; //E
    temporal[6] = 0x4F; //O
    temporal[7] = 0x53; //S
    temporal[8] = 0x3A; //:
    temporal[9] = 0X20; //SPC
    temporal[10] = 0x30;
    temporal[11] = NUM;
    temporal[12] = 0x20;
    temporal[13] = 0x20;
    temporal[14] = 0x20;
    temporal[15] = 0x20;
    return temporal;
}

const char* conver1(void){
    char temporal[16];
    temporal[0] = C1; //Caract�res del d�a
    temporal[1] = C2; 
    temporal[2] = C3; 
    temporal[3] = 0x2E; //.
    temporal[4] = 0x20; //SPC
    temporal[5] = DH; //Deena de hora
    temporal[6] = UH; //Unidad de hora
    temporal[7] = 0x3A; //:
    temporal[8] = DM; //Decena de minutos
    temporal[9] = UM; //Unidad de minutos
    temporal[10] = 0x20;
    temporal[11] = 0x20;
    temporal[12] = 0x20;
    temporal[13] = 0x20;
    temporal[14] = 0x20;
    temporal[15] = 0x20;
    return temporal;
}
void LECT1(void){
    NUM = num_ascii(PARKH);
    if (DIA == 1){
        C1 = 0x4C;
        C2 = 0x75;
        C3 = 0x6E;
    }
    else if(DIA == 2){
        C1 = 0x4D;
        C2 = 0x61;
        C3 = 0x72;
    }
    else if(DIA == 3){
        C1 = 0x4D;
        C2 = 0x69;
        C3 = 0x65;
    }
    else if (DIA == 4){
        C1 = 0x4A;
        C2 = 0x75;
        C3 = 0x65;
    }
    else if (DIA == 5){
        C1 = 0x56;
        C2 = 0x69;
        C3 = 0x65;
    }
    else if (DIA == 6){
        C1 = 0x53;
        C2 = 0x61;
        C3 = 0x62;
    }
    else if (DIA == 7){
        C1 = 0x44;
        C2 = 0x6F;
        C3 = 0x6D;
    }
    if (HORA<10){
        DH = 0x30;
        UH = num_ascii(HORA);
    }
    else if (HORA<26){
        DH = 0x31;
        con = HORA-16;
        UH = num_ascii(con);
    }
    else{
        DH = 0x32;
        con = HORA-32;
        UH = num_ascii(con);
    }
    if (MIN<10){
        DM = 0x30;
        UM = num_ascii(MIN);
    }
    else if (MIN<26){
        DM = 0x31;
        con = MIN-16;
        UM = num_ascii(con);                
    }
    else if (MIN<42){
        DM = 0x32;
        con = MIN-32;
        UM = num_ascii(con);
    }
    else if (MIN<58){
        DM = 0x33;
        con = MIN-32-16;
        UM = num_ascii(con);
    }
    else if (MIN<74){
        DM = 0x34;
        con = MIN-64;
        UM = num_ascii(con);
    }
    else if (MIN<90){
        DM = 0x35;
        con = MIN-64-16;
        UM = num_ascii(con);
    }
}