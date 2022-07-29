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

struct data 
{
    float32 DAC_volt;
    float32 ADC_volt;
};


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
        serviceData.len = 4;
        
        serviceHandle.attrHandle = CY_BLE_DATA_SERVICE_DATA_CHAR_HANDLE;
        
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
    
   
        CyDelay(10);
        //CyDelay(1000);
        
    }
}

/* [] END OF FILE */
