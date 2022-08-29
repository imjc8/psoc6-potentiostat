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
//uint32_t dac_val = 0u;



// conversion between float to uint for dac
//uint32_t maxVoltConverted = 0;
//uint32_t minVoltConverted = 0;
//uint32_t initVoltConverted = 0;
// direction
//bool direction;


// scan rate calcs
//int div = 0;
//int div_duration = 0;



// cycles 
//int cycle_count = 0;
//int cycle_dac = 0;

typedef enum {
    FALLING,
    RISING
} direction;

// initial scan direction
//direction dac_direction = RISING;

typedef struct {
    int cycle_count;
    int cycle_dac;
    direction dac_direction;
    uint32_t dac_val;
    int div_duration;
    int div;
    uint32_t maxVoltConverted;
    uint32_t minVoltConverted;
    uint32_t initVoltConverted;
    int counter; 
} dac_config_t;

dac_config_t dac_config;


// adc
uint16_t adc_count;
float32_t adc_volt;

// voltage for adc
float v1;
//uint16_t voltCount;

// flags
bool flag_print = false;

// timing_counter
//int counter = 0;

// device status
//int devState = 0;

typedef enum
{
    NOT_CONNECTED,
    BT_CONNECTED,
    CONFIG_LOADED,
    RUN_START,
    RUNNING,
    FINISHED
} state;

state devState = NOT_CONNECTED;

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

union {
  float f;
  unsigned char e[4];
} scanRate;

// recv cycles
//uint8 numCycles = 0;
bool recv_flag = false;

// recv dir
bool dir;

// test config
bool recvConfig = false;

typedef struct {
    uint16_t dac;
    uint16_t adc;
} data;

#define RB_SIZE 256
#define FRAME_SIZE 16

typedef struct {
    data buf[RB_SIZE];
    uint8_t head;
    uint8_t tail;
} data_rb_t;

int rb_push(data_rb_t *buf, data dat){
    uint8_t next;
    next = (buf->head + 1) % RB_SIZE;
    if (next == buf->tail){
        return -1;
    }
    buf->buf[buf->head] = dat;
    buf->head = next;
    return 0;
}

int rb_pop(data_rb_t *buf, data *dat){
    int next;
    if (buf->head == buf->tail) return 0;
    next = (buf->tail + 1) % RB_SIZE;
    *dat = buf->buf[buf->tail];
    buf->tail = next;
    return 1;
}

data_rb_t buf;
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
            devState = NOT_CONNECTED;
            break;
        }
        
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            devState = BT_CONNECTED;
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
                devState = RUN_START;
            }
            else if(CY_BLE_DATA_SERVICE_INBOUND_TEST_CONFIG_CHAR_HANDLE == writeReqParameter->handleValPair.attrHandle && devState != CONFIG_LOADED && devState != RUN_START && devState != RUNNING) {
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
                
                // start voltage
                startVolt.d[3] = writeReqParameter->handleValPair.value.val[11];
                startVolt.d[2] = writeReqParameter->handleValPair.value.val[10];
                startVolt.d[1] = writeReqParameter->handleValPair.value.val[9];
                startVolt.d[0] = writeReqParameter->handleValPair.value.val[8];
                
                                
                // Scan rate
                scanRate.e[3] = writeReqParameter->handleValPair.value.val[15];
                scanRate.e[2] = writeReqParameter->handleValPair.value.val[14];
                scanRate.e[1] = writeReqParameter->handleValPair.value.val[13];
                scanRate.e[0] = writeReqParameter->handleValPair.value.val[12];
                
                direction dac_direction;
                // direction
                if(writeReqParameter->handleValPair.value.val[16]){
                    dac_direction = RISING;
                } else {
                    dac_direction = FALLING;
                }
                //dac_direction = writeReqParameter->handleValPair.value.val[16];
                
                // number of cycles
                int numCycles = writeReqParameter->handleValPair.value.val[17];
                
                uint32_t minVoltConverted = round((minVolt.f/3.3)*4095);
                uint32_t maxVoltConverted = round((maxVolt.f/3.3)*4095);
                uint32_t initVoltConverted = round((startVolt.f/3.3)*4095);
                
                float volt_diff = maxVolt.f - minVolt.f;
    
                 // DAC clock frequency is 10KHz
                int div_duration = (int)((float)(10000.0 * volt_diff/((maxVoltConverted - minVoltConverted) * scanRate.f)))-1;
                
                //cycle_count = numCycles;
                //dac_val = initVoltConverted;
                dac_config_t dac_config_new = {
                    .cycle_count = numCycles,
                    .dac_direction = dac_direction,
                    .dac_val = initVoltConverted,
                    .cycle_dac = 0,
                    .div_duration = div_duration,
                    .div = 0,
                    .maxVoltConverted = maxVoltConverted,
                    .minVoltConverted = minVoltConverted,
                    .initVoltConverted = initVoltConverted,
                    .counter = 0
                };
                dac_config = dac_config_new;
                devState = CONFIG_LOADED;  
            }
            
            Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);
            
            
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
    state prev_state = devState;
    __enable_irq();

    /* Configure the interrupt and provide the ISR address. */
    (void)Cy_SysInt_Init(&SysInt_1_cfg, userIsr);

    /* Enable the interrupt. */
    NVIC_EnableIRQ(SysInt_1_cfg.intrSrc);

    Cy_BLE_Start(genericEventHandler);
    while(Cy_BLE_GetState() != CY_BLE_STATE_ON)
    {
        Cy_BLE_ProcessEvents();
    }
    Cy_BLE_RegisterAppHostCallback(bleInterruptNotify);
    
    //struct data d1;
    
    // waveform parameters
    // voltage
    //float min_volt = 0.0;
    //float max_volt = 3.3;
    //float init_volt = 0.0;
    // scan rate (v/sec)
    //float scan_rate = 0.5;
    
    // number of cycles
    //cycle_count = 5;
    
    /* Start the component. */
    //VDAC_1_Start();
    UART_1_Start();
    setvbuf(stdin, NULL, _IONBF, 0);

    //Cy_SAR_StartConvert(SAR, CY_SAR_START_CONVERT_CONTINUOUS);
    

    data tx_frame[FRAME_SIZE];
    uint8_t frame_ctr = 0;
    int i = 0;
    for(;;)
    {   
        if(devState == CONFIG_LOADED){
             printf("min Volt: %f \t max Volt: %f \t start volt: %f \t dir: %d \t numCycle: %d \r\n", minVolt.f, maxVolt.f, startVolt.f, dir, dac_config.cycle_count);
             devState = RUN_START;
        }
        if(devState == RUN_START)
        {
            ADC_1_Start();
            VDAC_1_Start();
            devState = RUNNING;
        } else if (devState == FINISHED) {
            //dac_val = 0.0;
            //printf("counter: %d \r\n", counter);
        }
        if (prev_state != devState){
            printf("cur state %i\r\n",devState);
            cy_stc_ble_gatt_handle_value_pair_t serviceHandle;
            cy_stc_ble_gatt_value_t serviceData;
            
            serviceData.val = (uint8*) &devState;
            serviceData.len = sizeof(devState);

            serviceHandle.attrHandle = CY_BLE_DATA_SERVICE_DATA_OUT_2_CHAR_HANDLE;
            serviceHandle.value = serviceData;
            Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &serviceHandle);

            prev_state = devState;
        }
        data dat;
        if (rb_pop(&buf,&dat)){
            tx_frame[frame_ctr++] = dat;
        }


        if (frame_ctr == FRAME_SIZE){
            cy_stc_ble_gatt_handle_value_pair_t serviceHandle;
            cy_stc_ble_gatt_value_t serviceData;
            
            serviceData.val = (uint8*) &tx_frame;
            serviceData.len = sizeof(tx_frame);

            serviceHandle.attrHandle = CY_BLE_DATA_SERVICE_DATA_OUT_CHAR_HANDLE;
            serviceHandle.value = serviceData;
            Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &serviceHandle);
            //printf("dac %i adc %i\r\n",dat.dac,dat.adc);
            frame_ctr = 0;
        }
        Cy_BLE_ProcessEvents();
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
        if ((dac_config.cycle_dac/2) < (dac_config.cycle_count)){
            // basically skip times which don't line up with scan rate
            if (dac_config.div == dac_config.div_duration) {
                dac_config.counter++;
                VDAC_1_SetValueBuffered(dac_config.dac_val);
                // sample adc
                Cy_SAR_StartConvert(SAR, CY_SAR_START_CONVERT_SINGLE_SHOT);
                int16_t voltCount = Cy_SAR_GetResult16(SAR, 0);
                data dat = {.adc = voltCount, .dac = dac_config.dac_val};
                rb_push(&buf, dat);
                //v1 = Cy_SAR_CountsTo_Volts(SAR, 0, voltCount);
                //flag_print = true;
                /* Place your application code here. */
                //printf("DAC OUTPUT: %d ADC is: %f \r\n", dac_val, v1);
                // broadcast bluetooth

                // when i want to send to bluetooth and get current
                // up
                if (dac_config.dac_direction == RISING){
                    // increment
                    dac_config.dac_val++;
                    // 1 cycle complete
                    if(dac_config.dac_val == dac_config.initVoltConverted){
                        dac_config.cycle_dac++;
                        // check if it hits peak
                        if (dac_config.dac_val == dac_config.maxVoltConverted){
                            // switch direction
                            dac_config.dac_direction = FALLING;
                            // increment again since there is one less point
                            dac_config.cycle_dac++;
                        }
                    }
                    else if (dac_config.dac_val >= dac_config.maxVoltConverted)
                    {
                        dac_config.dac_direction = FALLING;
                    }
                }
                else if (dac_config.dac_direction == FALLING){
                    dac_config.dac_val--;
                    //handle underflow
                    if (dac_config.dac_val >= dac_config.maxVoltConverted){
                        dac_config.dac_val = 0;
                    }
                    if(dac_config.dac_val == dac_config.initVoltConverted){
                        dac_config.cycle_dac++;
                        if (dac_config.dac_val == dac_config.minVoltConverted){
                            dac_config.dac_direction = RISING;
                            dac_config.cycle_dac++;
                        }
                    }
                    else if (dac_config.dac_val <= dac_config.minVoltConverted)
                    {
                        dac_config.dac_direction = RISING;
                    }
                }
                dac_config.div = 0;
            } 
            else {
                dac_config.div++;
            };
        }
        else {
            VDAC_1_Stop();
            ADC_1_Stop();
            devState = FINISHED;
        }
    }
}

/* [] END OF FILE */
