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
#include "stdio.h"

// dac val
uint32_t dac_val = 0u;



// conversion between float to uint for dac
uint32_t maxVoltConverted = 0;
uint32_t minVoltConverted = 0;
uint32_t initVoltConverted = 0;

// scan rate calcs
int div = 0;
int div_duration = 0;

// cycles 
int cycle_count = 0;
int cycle_dac = 0;

// initial scan direction
int dac_direction = 0;

// adc
int16_t adc_count;
float32_t adc_volt;

// voltage for adc
float v1;
float voltCount;

// flags
bool flag_print = false;

// timing_counter
int counter = 0;

// device status
int devState = 0;

// to send
struct data 
{
    float32 DAC_volt;
    float32 ADC_volt;
};

// for the receiving floats
union {
  float f;
  unsigned char b[4];
} minVolt;

union {
  float f;
  unsigned char c[4];
} maxVolt;

union {
  float f;
  unsigned char d[4];
} startVolt;

// recv cycles
uint8 numCycles = 0;
bool recv_flag = false;

// recv dir
bool dir;

// unsure if need
uint8_t hand;

void userIsr(void);

void genericEventHandler(uint32_t event, void *eventParameter)
{
    switch(event)
    {
        case CY_BLE_EVT_STACK_ON:
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        {
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            devState = 0;
            break;
        }
        
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            devState = 1;
            cy_stc_ble_gap_bd_addr_info_t param;
//            param.bdAddr=param.addrType=CY_BLE_GAP_RANDOM_PRIV_RESOLVABLE_ADDR;
//            bdHand=Cy_BLE_GAP_GetPeerBdAddr(&param);
            printf("CY_BLE_EVT_GATT_CONNECT_IND bdHandle=%x\r\n",((cy_stc_ble_conn_handle_t *)eventParameter)->bdHandle);
            hand=((cy_stc_ble_conn_handle_t *)eventParameter)->bdHandle;
            break;
        }
        case CY_BLE_EVT_GATTS_WRITE_REQ:
        {
            cy_stc_ble_gatts_write_cmd_req_param_t *writeReqParameter = (cy_stc_ble_gatts_write_cmd_req_param_t *) eventParameter;
            
            // if write parameter is this
            if(CY_BLE_DATA_SERVICE_START_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle){
                // recv_val = writeReqParameter->handleValPair.value.val[0];
                //printf("recv val is %d\r\n", recv_val);
                recv_flag = true;
                devState = 4;
            }
            else if(CY_BLE_DATA_SERVICE_INBOUND_TEST_CONFIG_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle) {
                // min voltage
                minVolt.b[3] = writeReqParameter->handleValPair.value.val[3];
                minVolt.b[2] = writeReqParameter->handleValPair.value.val[2];
                minVolt.b[1] = writeReqParameter->handleValPair.value.val[1];
                minVolt.b[0] = writeReqParameter->handleValPair.value.val[0];
                
                // max voltage
                maxVolt.c[3] = writeReqParameter->handleValPair.value.val[7];
                maxVolt.c[2] = writeReqParameter->handleValPair.value.val[6];
                maxVolt.c[1] = writeReqParameter->handleValPair.value.val[5];
                maxVolt.c[0] = writeReqParameter->handleValPair.value.val[4];
                
                // start
                startVolt.d[3] = writeReqParameter->handleValPair.value.val[11];
                startVolt.d[2] = writeReqParameter->handleValPair.value.val[10];
                startVolt.d[1] = writeReqParameter->handleValPair.value.val[9];
                startVolt.d[0] = writeReqParameter->handleValPair.value.val[8];
                
                // direction
                dir = writeReqParameter->handleValPair.value.val[12];
                
                // number of cycles
                numCycles = writeReqParameter->handleValPair.value.val[13];
                
                
                printf("min Volt: %f \t max Volt: %f \t start volt: %f \t dir: %d \t numCycle: %d \r\n", minVolt.f, maxVolt.f, startVolt.f, dir, numCycles);
            }
            
            Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);
            devState = 3;
            
            break;
        }
    }
}

void bleInterruptNotify()
{
    Cy_BLE_ProcessEvents();
}


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
    
    // waveform parameters
    // voltage
    float min_volt = 0.0;
    float max_volt = 3.3;
    float init_volt = 0.0;
    // scan rate (v/sec)
    float scan_rate = 0.5;
    
    // number of cycles
    cycle_count = 5;
    
    //float min_volt = 1
    //float max_volt = 2.3
    

    
    minVoltConverted = round((min_volt/3.3)*4095);
    maxVoltConverted = round((max_volt/3.3)*4095);
    initVoltConverted = round((init_volt/3.3)*4095);

    //dac_val = initVoltConverted;   
    
    // direction  1 -> go up 0 -> down
    // set initial dac direction
    bool direction;
    if(init_volt==min_volt){
        direction = 1;
    }else if(init_volt==max_volt){
        direction = 0;
    } else{
        direction = 1;   
    }
    dac_direction = (direction == 1) ? 1 : 0;
    
    /*
    if (direction == 1)
    {
        dac_direction = 1;
    }
    else
    {
        dac_direction = 0;
    }
    */
    float volt_diff = max_volt - min_volt;
    
    // DAC clock frequency is 50KHz
    div_duration = (int)((float)(10000.0 * volt_diff/((maxVoltConverted - minVoltConverted) * scan_rate)))-1;
    
    /* Start the component. */
    VDAC_1_Start();
    UART_1_Start();
    setvbuf(stdin, NULL, _IONBF, 0);
    ADC_1_Start();
    //Cy_SAR_StartConvert(SAR, CY_SAR_START_CONVERT_CONTINUOUS);
    
    dac_val = initVoltConverted;   
    printf("hello world\r\n");
    
    int i = 0;
    for(;;)
    {   
//        adc_count = Cy_SAR_GetResult16(SAR, 0);
//        adc_volt = Cy_SAR_CountsTo_Volts(SAR, 0, adc_count);
//        printf("Dac: %d ADC: %f ADC count: %d \r\n", dac_val, adc_volt, adc_count);
//        for (i = 0; i< 10; i++){
//            printf("counter is %d \r\n", i);
//        }
        
        if (flag_print){
            //printf("DAC OUTPUT: %d ADC is: %f \r\n", dac_val, v1);
            flag_print = false;
            //CyDelay(100);
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
        //VDAC_1_SetValueBuffered(dac_val);
        
        // generate triangle wave
        if ((cycle_dac/2) < (cycle_count)){
            // basically skip times which don't line up with scan rate
            if (div == div_duration) {
                counter++;
                VDAC_1_SetValueBuffered(dac_val);
                // sample adc
                Cy_SAR_StartConvert(SAR, CY_SAR_START_CONVERT_SINGLE_SHOT);
                voltCount = Cy_SAR_GetResult16(SAR, 0);
                v1 = Cy_SAR_CountsTo_Volts(SAR, 0, voltCount);
                flag_print = true;
                /* Place your application code here. */
                //printf("DAC OUTPUT: %d ADC is: %f \r\n", dac_val, v1);
                // broadcast bluetooth

                // when i want to send to bluetooth and get current
                // up
                if (dac_direction == 1){
                    // increment
                    dac_val++;
                    // 1 cycle complete
                    if(dac_val == initVoltConverted){
                        cycle_dac++;
                        // check if it hits peak
                        if (dac_val == maxVoltConverted){
                            // switch direction
                            dac_direction = 0;
                            // increment again since there is one less point
                            cycle_dac++;
                        }
                    }
                    else if (dac_val >= maxVoltConverted)
                    {
                        dac_direction = 0;
                    }
                }
                else if (dac_direction == 0){
                    dac_val--;
                    if(dac_val == initVoltConverted){
                        cycle_dac++;
                        if (dac_val == minVoltConverted){
                            dac_direction = 1;
                            cycle_dac++;
                        }
                    }
                    else if (dac_val <= minVoltConverted)
                    {
                        dac_direction = 1;
                    }
                }
                div = 0;
            } 
            else {
                div++;
            };
        }
        else {
            VDAC_1_Stop();
            ADC_1_Stop();
            //dac_val = 0.0;
            printf("counter: %d \r\n", counter);
        }
        

    }
}

/* [] END OF FILE */
