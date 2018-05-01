#include <stdint.h>
#include <math.h>
#include "tm4c1294ncpdt.h"

#define SYSCLK 16000000

void configSys(void);

void displayValue(unsigned char);

void configTimer(void);

void waitus(int);

int main(void) {


    configSys();    // Configure Ports & Timer

    while(1) {

    }

    return 0;
}

void configSys(void)
{
    SYSCTL_RCGCGPIO_R |= 0x00000E14;            // Enable Ports M, L, K, E, D
    while (!(SYSCTL_PRGPIO_R & 0x00000E14ul));  // Wait for Ports Ready flag

    SYSCTL_RCGCADC_R |= (1<<0);         // Enable ADC Module 0
    while(SYSCTL_PRADC_R != 0x01);   // Wait for ADC Module ready flag

    GPIO_PORTE_AHB_AFSEL_R |= 0x01;     // Enable PORTE(0) Alternative Function
    GPIO_PORTE_AHB_AMSEL_R |= 0x01;     // Enable PORTE(0) Analog Function

    ADC0_SSEMUX0_R = (uint32_t) 0x0;    // Set ADC0 Sampler to read from AIN[15:0]
    ADC0_SSMUX0_R = (uint32_t) 0x03;    // Set ADC0 Sampler to read AIN3
    ADC0_SSCTL0_R = (uint32_t) 1<<5;    // Set ADC0 Sampler Step 2 as end of seuqence

    GPIO_PORTL_DIR_R |= 0x00000007;         // Set PORTL(2:0) to outputs
    GPIO_PORTM_DIR_R |= 0x000000FF;         // Set PORTM(7:0) to outputs
    GPIO_PORTD_AHB_DIR_R &= ~0x00000003ul;  // Set PORTD(1:0) to inputs
    GPIO_PORTK_DIR_R |= 0x000000FF;         // Set PORTK(7:0) to outputs
    GPIO_PORTE_AHB_DIR_R &= ~0x01;          // Set PORTE(0) to input

    GPIO_PORTL_DEN_R |= 0x00000007;         // Enable PORTL(2:0)
    GPIO_PORTM_DEN_R |= 0x000000FF;         // Enable PORTM(7:0)
    GPIO_PORTD_AHB_DEN_R |= 0x00000003;     // Enable PORTD(1:0)
    GPIO_PORTK_DEN_R |= 0x000000FF;         // Enable PORTK(7:0)
    GPIO_PORTE_AHB_DEN_R &= ~0x01;      // Enable PORTE(0)

    configTimer();      // Configure Timer
}

void displayValue(unsigned char input)
{
    uint32_t digits[4] = {0,0,0,0};                         // Initialize LCD digits array
    int result = (int) rint(input * (5.0/256) * 1000);      // Calculate result in mV
    int i;

    for (i=3; i!=-1; --i) {                         // Iterate over LCD digits
        digits[i] = (uint32_t) (result / (10*i));   // Calculate current digit
        result = (uint8_t) (result % (10*i));       // Store remainder for next calculation
    }

    GPIO_PORTL_DATA_R = (GPIO_PORTL_DATA_R & ~1u) | 2u;     // Disable LCD digits 0:1, enable 3:2
    GPIO_PORTM_DATA_R = digits[3] << 4u | digits[2];        // Output LCD digits 3:2
    GPIO_PORTL_DATA_R = (GPIO_PORTL_DATA_R & ~2u) | 1u;     // Disable LCD digits 3:2, enable 1:0
    GPIO_PORTM_DATA_R = digits[1] << 4u | digits[0];        // Output LCD digits 1:0
}

void configTimer(void)
{
    SYSCTL_RCGCTIMER_R |= 1ul;  // Enable Timer 0 Module
    TIMER0_CTL_R &= ~1ul;       // Disable TIMER0A
    TIMER0_CFG_R |= 4ul;        // Set 16 bit mode
    TIMER0_TAPR_R = 0x00;       // Set Prescaler = 0
    TIMER0_TAMR_R |= 1ul;       // Set One shot mode
    TIMER0_TAMR_R &= ~0x10ul;   // Set Count down, Compare Mode
}

void waitus(int us)
{
    // Calculate & set Interval Load value
    TIMER0_TAILR_R = (uint32_t) ceil(us*1000.0/SYSCLK) - 1;
    TIMER0_CTL_R |= 1ul;            // Enable TIMER0A
    while (!(TIMER0_RIS_R & 1ul));  // Poll TIMER0A Time-Out Interrupt
    TIMER0_RIS_R |= 1ul;            // Clear TIMER0A Time-Out Interrupt
    TIMER0_CTL_R &= ~1ul;           // Disable TIMER0A
}