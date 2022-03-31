/******************************************************************************
* File Name: main_cm4.c
*
* Version: 1.20
*
* Description: This is the source code for the TCPWM (Square wave) code example.
*
* Related Document: CE220291.pdf
*
* Hardware Dependency: See CE220290.pdf
*
*******************************************************************************
* Copyright (2017-2019), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/

#include "project.h"


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary: This is the system entrance point for Cortex-M4.
* This function initializes the PSoC Components and puts the Cortex-M4 CPU
* in Sleep mode to save power.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  None  
*
*******************************************************************************/
int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    
    /* Initialize, enable, and start the PWM component.
     * To see all pre-defined parameters such as <Instance_Name>_CNT_Mask/Num
     * or PWM_config see PWM.h under Generated sources -> PWM */
    (void) Cy_TCPWM_PWM_Init(PWM_HW, PWM_CNT_NUM, &PWM_config);
    Cy_TCPWM_Enable_Multiple(PWM_HW, PWM_CNT_MASK);
    Cy_TCPWM_TriggerReloadOrIndex(PWM_HW, PWM_CNT_MASK);

    /* Infinite loop */
    for(;;)
    {
        /* Put the CPU into Sleep mode to save power */
        Cy_SysPm_Sleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
}


/* [] END OF FILE */
