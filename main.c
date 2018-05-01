#include <stdint.h>
#include <math.h>
#include "tm4c1294ncpdt.h"

void configSys(void);

void displayValue(unsigned long);

int main(void)
{
    unsigned long result = 0;

    configSys();    // Configure Ports & Timer

    while(1) {
        ADC0_PSSI_R |= 0x01;                // Start ADC0 Sample Sequencer
        while (ADC0_SSFSTAT0_R & (1<<8));   // Wait for ADC0 Sample Seuqencer 0 not empty flag
        result = ADC0_SSFIFO0_R;            // Read from ADC0 FIFO

        displayValue(result);               // Display last result read
    }

    return 0;
}

void configSys(void)
{
    SYSCTL_RCGCGPIO_R |= 0x00000E14;            // Enable Ports M, L, K, E, D
    while (!(SYSCTL_PRGPIO_R & 0x00000E14ul));  // Wait for Ports Ready flag

    SYSCTL_RCGCADC_R |= (1<<0);         // Enable ADC Module 0
    while(!(SYSCTL_PRADC_R & 0x01));    // Wait for ADC Module ready flag

    GPIO_PORTE_AHB_AFSEL_R |= 0x01;     // Enable PORTE(0) Alternative Function
    GPIO_PORTE_AHB_AMSEL_R |= 0x01;     // Enable PORTE(0) Analog Function

    ADC0_ACTSS_R &= ~0x0F;              // Disable all ADC Sample Sequencer
    ADC0_SSEMUX0_R = (uint32_t) 0x0;    // Set ADC0 Sample Sequencer to read from AIN[15:0]
    ADC0_SSMUX0_R = (uint32_t) 0x03;    // Set ADC0 Sample Sequencer to read AIN3
    ADC0_SSCTL0_R = (uint32_t) 1<<5;    // Set ADC0 Sample Sequencer to end on Step 2

    GPIO_PORTL_DIR_R |= 0x00000007;         // Set PORTL(2:0) to outputs
    GPIO_PORTM_DIR_R |= 0x000000FF;         // Set PORTM(7:0) to outputs
    GPIO_PORTD_AHB_DIR_R &= ~0x00000003ul;  // Set PORTD(1:0) to inputs
    GPIO_PORTK_DIR_R |= 0x000000FF;         // Set PORTK(7:0) to outputs
    GPIO_PORTE_AHB_DIR_R &= ~0x01;          // Set PORTE(0) to input

    GPIO_PORTL_DEN_R |= 0x00000007;         // Enable PORTL(2:0)
    GPIO_PORTM_DEN_R |= 0x000000FF;         // Enable PORTM(7:0)
    GPIO_PORTD_AHB_DEN_R |= 0x00000003;     // Enable PORTD(1:0)
    GPIO_PORTK_DEN_R |= 0x000000FF;         // Enable PORTK(7:0)
    GPIO_PORTE_AHB_DEN_R &= ~0x01;          // Enable PORTE(0)
}

void displayValue(unsigned long input)
{
    uint32_t digits[4] = {0,0,0,0};                         // Initialize LCD digits array
    int result = (int) rint(input * (5.0/4096) * 1000);     // Calculate result in mV
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
