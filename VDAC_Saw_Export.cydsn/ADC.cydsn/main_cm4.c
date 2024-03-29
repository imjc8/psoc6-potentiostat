/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "stdio.h"

int main(void)
{
    
    __enable_irq(); /* Enable global interrupts. */
    
    UART_1_Start();
    ADC_1_Start();
     
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    // voltage
    float v1;
    float voltCount;
    printf("hello\n");
    //

    for(;;)
    {
        // Read ADC values
        Cy_SAR_StartConvert(SAR, CY_SAR_START_CONVERT_SINGLE_SHOT);
        voltCount = Cy_SAR_GetResult16(SAR, 0);
        v1 = Cy_SAR_CountsTo_Volts(SAR, 0, voltCount);
        /* Place your application code here. */
        printf("Volt is: %f \r\n", v1);
        CyDelay(500);
    }
}

/* [] END OF FILE */
