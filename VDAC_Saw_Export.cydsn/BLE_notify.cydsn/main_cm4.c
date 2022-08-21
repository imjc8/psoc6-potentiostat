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

float64 dataNotify = 0.0;
int bleConnected = 0;
uint8_t hand;
int flagNotify = 0;
int sendFlag = false;



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
  float g;
  unsigned char e[4];
} scanRate;

// recv cycles
uint8 numCycles = 0;
bool recv_flag = false;

// recv dir
bool dir;



void genericEventHandler(uint32_t event, void *eventParameter)
{
    switch(event)
    {
        case CY_BLE_EVT_STACK_ON:
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        {
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            bleConnected = 0;
            break;
        }
        
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            bleConnected = 1;
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
                
                // direction
                dir = writeReqParameter->handleValPair.value.val[16];
                
                // number of cycles
                numCycles = writeReqParameter->handleValPair.value.val[17];
                
                
                printf("min Volt: %f \t max Volt: %f \t start volt: %f \t dir: %d \t numCycle: %d \r\n", minVolt.f, maxVolt.f, startVolt.f, dir, numCycles);
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

int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    UART_1_Start();
    
    Cy_BLE_Start(genericEventHandler);
    
    while(Cy_BLE_GetState() != CY_BLE_STATE_ON)
    {
        Cy_BLE_ProcessEvents();
    }
    Cy_BLE_RegisterAppHostCallback(bleInterruptNotify);
    
    struct data d1;

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    float dac_volt;
    float adc_volt;
    
    // declare a string
    char* dataString;
    
    int counter = 0;
    float ex_val = 0.0;
    
    for(;;)
    {
//        printf("working\r\n");
        if (counter < 4095){
            counter++;
            ex_val++;
            d1.DAC_volt = ex_val;
            d1.ADC_volt = ex_val;
//            dataNotify<<8.0;
//            dataNotify|adc_volt;
        }
        else{
            counter = 0;
            ex_val = 0.0;
        }
        
//        d1.DAC_volt = 4000;
//        d1.ADC_volt = 100;
       
        
        /* Place your application code here. */
        cy_stc_ble_gatt_handle_value_pair_t serviceHandle;
        cy_stc_ble_gatt_value_t serviceData;
        
        serviceData.val = (uint8*) &d1;
        serviceData.len = 8;
        
        serviceHandle.attrHandle = CY_BLE_DATA_SERVICE_DATA_OUT_CHAR_HANDLE;
        
        serviceHandle.value = serviceData;
        
        
        //cy_stc_ble_conn_handle_t connHandle;
        //connHandle.bdHandle=hand;
        
        // error code debugging
        //cy_en_ble_gatt_err_code_t e=Cy_BLE_GATTS_WriteAttributeValueLocal(&serviceHandle);
//        if(e==CY_BLE_GATT_ERR_INVALID_HANDLE){
//            printf("error \r\n");   
//        }else if(e==CY_BLE_GATT_ERR_UNLIKELY_ERROR){
//            printf("error1 \r\n");   
//        }else if(e==CY_BLE_GATT_ERR_INVALID_ATTRIBUTE_LEN){
//            printf("error2 \r\n");   
//        }
        
        //printf("Connhandle %x \r\n", connHandle);
        if(sendFlag == true) {
        cy_en_ble_api_result_t e = Cy_BLE_GATTS_SendNotification(&cy_ble_connHandle[0], &serviceHandle);
            if (e == CY_BLE_SUCCESS) {
                printf("DAC val: %f \t ADC val:%f \r\n", d1.DAC_volt, d1.ADC_volt);
            } else if (e == CY_BLE_ERROR_NO_DEVICE_ENTITY) {
                printf("Error 1\r\n");
            } else if (e == CY_BLE_ERROR_INVALID_PARAMETER) {
                printf("Error 2\r\n");
            } else if (e == CY_BLE_ERROR_INVALID_OPERATION) {
                printf("Error 3\r\n");
            } else if (e == CY_BLE_ERROR_NTF_DISABLED) {
                printf("Error 4\r\n");
            }
        }
        else if (recv_flag == true) {
            //printf("value received is: %d\r\n", recv_val);
            recv_flag = false;
        }
        
        else {
            //printf("nothing\r\n");
        }
   
        CyDelay(10);
        //CyDelay(1000);
        
    }
}

/* [] END OF FILE */
