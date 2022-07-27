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

int dataNotify = 0;
int bleConnected = 0;
uint8_t hand;


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

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        if (dataNotify < 100){
            dataNotify++;
        }
        else{
            dataNotify = 0;
        }
       
        
        /* Place your application code here. */
        cy_stc_ble_gatt_handle_value_pair_t* serviceHandle;
        cy_stc_ble_gatt_value_t serviceData;
        
        serviceData.val = (uint8*) &dataNotify;
        serviceData.len = 2;
        
        serviceHandle->attrHandle = CY_BLE_DATA_SERVICE_DATA_CHAR_HANDLE;
        serviceHandle->value = serviceData;
        
        cy_stc_ble_conn_handle_t connHandle;
        connHandle.bdHandle=hand;
        
        // error code debugging
        //cy_en_ble_gatt_err_code_t e=Cy_BLE_GATTS_WriteAttributeValueLocal(&serviceHandle);
//        if(e==CY_BLE_GATT_ERR_INVALID_HANDLE){
//            printf("error \r\n");   
//        }else if(e==CY_BLE_GATT_ERR_UNLIKELY_ERROR){
//            printf("error1 \r\n");   
//        }else if(e==CY_BLE_GATT_ERR_INVALID_ATTRIBUTE_LEN){
//            printf("error2 \r\n");   
//        }
        
        cy_en_ble_api_result_t e = Cy_BLE_GATTS_SendNotification(&connHandle, &serviceData);
        if (e == CY_BLE_SUCCESS) {
            printf("Good transmission\r\n");
        } else if (e == CY_BLE_ERROR_NO_DEVICE_ENTITY) {
            printf("Error 1\r\n");
        } else if (e == CY_BLE_ERROR_INVALID_PARAMETER) {
            printf("Error 2\r\n");
        } else if (e == CY_BLE_ERROR_INVALID_OPERATION) {
            printf("Error 3\r\n");
        } else if (e == CY_BLE_ERROR_NTF_DISABLED) {
            printf("Error 4\r\n");
        }
    
   

        //CyDelay(1000);
        
    }
}

/* [] END OF FILE */
