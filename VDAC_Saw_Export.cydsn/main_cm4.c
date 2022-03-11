/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.00
*
* Description: This example demonstrates how to use the Voltage DAC (12-bit)
*              component and CTDAC as a sawtooth wave generator in a PSoC 6 MCU.
*
* Related Document: CE220923.pdf
*
* Hardware Dependency: CY8CKIT-062-BLE PSoC 6 MCU BLE Pioneer Kit or
*                      CY8CKIT-062 PSoC 6 MCU Pioneer Kit
*
******************************************************************************
* Copyright (2017), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/
#include "project.h"
#include "ctdac/cy_ctdac.h"
#include <math.h>

uint32_t currentCode = 0u;
int flag = 1;

void userIsr(void);

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  The main function performs the following actions:
*   1. Initializes Interrupt Component
*   2. Starts the CTDAC hardware
*   3. On each interrupt, updates the CTDAC output value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main(void)
{
    /* Enable global interrupts. */
    __enable_irq();

    /* Configure the interrupt and provide the ISR address. */
    (void)Cy_SysInt_Init(&SysInt_1_cfg, userIsr);

    /* Enable the interrupt. */
    NVIC_EnableIRQ(SysInt_1_cfg.intrSrc);

    /* Start the component. */
    VDAC_1_Start();
    
    float volt = 2.0;
    float volt_max = 3.3;
    float volt_min = 0.0;
    float volt_diff;
    float sleep1;
    float pts;
    float scan_rate = 1;
    float period;
    float dac_max;
    float dac_min;
    float dac_diff;
    float sleep;
    dac_max = floor((volt_max/3.3) * 4095);
    dac_min = floor((volt_min/3.3)* 4095);
    dac_diff = dac_max-dac_min;
    pts = 2*dac_diff;
    
    volt_diff = volt_max-volt_min;
    period = 2*(volt_diff/scan_rate);
    sleep = 10e5*(period/pts);
    
    
    for(;;)
    {   
        if (flag == 1 && currentCode < dac_max)
        {
            currentCode++;
            CyDelayUs(sleep);
            if (currentCode == dac_max)
            {
                flag = 0;
            }
        }
        else if (flag == 0)
        {
            currentCode--;
            CyDelayUs(sleep);
            if (currentCode == dac_min)
            {
                flag = 1;
            }
        }
    }
}

/*******************************************************************************
* Function Name: userIsr
********************************************************************************
*
*  Summary:
*  Interrupt service routine for the VDAC update.
*
*  Parameters:
*  None
*
*  Return:
*  None
*
**********************************************************************************/
/* This function gets called when an interrupt occurs in any CTDAC on the PSoC 6 MCU device. */
void userIsr(void)
{
    uint8_t intrStatus;

    /* Check that an interrupt occurred in the VDAC component instance. */
    intrStatus = VDAC_1_GetInterruptStatus();
    if (intrStatus)
    {
        /* Clear the interrupt. */
        VDAC_1_ClearInterrupt();
        
        /* Set the next value that the DAC will output. */
        VDAC_1_SetValueBuffered(currentCode);
        
        /*

        // Increment the DAC code from 0 to the maximum code value to generate a sawtooth waveform. 
        if (currentCode >= CY_CTDAC_UNSIGNED_MAX_CODE_VALUE)
        {
            currentCode = 0u;
        }
        else
        {
            currentCode++;
        }
        */
        
        /*
        // generate saw tooth
        if (flag == 1 && currentCode < 4096)
        {
            currentCode++;
            if (currentCode == CY_CTDAC_UNSIGNED_MAX_CODE_VALUE)
            {
                flag = 0;
            }
        }
        else if (flag == 0)
        {
            currentCode--;
            if (currentCode == 0)
            {
                flag = 1;
            }
        }
        */
    }
}

/* [] END OF FILE */
