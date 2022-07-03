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

static uint8 data[1] = {0};
int bluetooth_conn = 0;
int gradient = 0;
uint32_t currentCode = 0u;
int set = 1;

void userIsr(void);

void genericEventHandler(uint32_t event, void *eventParameter)
{
    switch(event)
    {
        case CY_BLE_EVT_STACK_ON:
        case CY_BLE_EVT_GAP_DEVICE_DISCONNECTED:
        {
            Cy_BLE_GAPP_StartAdvertisement(CY_BLE_ADVERTISING_FAST, CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX);
            bluetooth_conn = 0;
            break;
        }
        
        case CY_BLE_EVT_GATT_CONNECT_IND:
        {
            bluetooth_conn = 1;
            break;
        }
        case CY_BLE_EVT_GATTS_WRITE_REQ:
        {
            cy_stc_ble_gatts_write_cmd_req_param_t *writeReqParameter = (cy_stc_ble_gatts_write_cmd_req_param_t *) eventParameter;
            
            if(writeReqParameter->handleValPair.attrHandle == CY_BLE_DEVICE_INTERFACE_DEVICE_INBOUND_CHAR_HANDLE)
            {
                data[0] = writeReqParameter->handleValPair.value.val[0];
                Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);
            }
            // else if(writeReqParameter->handleValPair.attrHandle == CY_BLE_DEVICE_INTERFACE_ASDF_CHAR_HANDLE)
            // {
            //     set = writeReqParameter->handleValPair.value.val[0];
            //     Cy_BLE_GATTS_WriteRsp(writeReqParameter->connHandle);
            // }
            break;
        }
    }
}

void bleInterruptNotify()
{
    Cy_BLE_ProcessEvents();
}

void squareWave()
{
    
    // set a pin to high
    Cy_GPIO_Write(P9_6_PORT, P9_6_NUM, 1);
    CyDelay(500);
    Cy_GPIO_Write(P9_6_PORT, P9_6_NUM, 0);
    CyDelay(500);    
}



int main(void)
{
    __enable_irq(); /* Enable global interrupts. */
    
    
    Cy_BLE_Start(genericEventHandler);
    
    while(Cy_BLE_GetState() != CY_BLE_STATE_ON)
    {
        Cy_BLE_ProcessEvents();
    }
    Cy_BLE_RegisterAppHostCallback(bleInterruptNotify);
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    for(;;)
    {
        /* Place your application code here. */
        
        cy_stc_ble_gatt_handle_value_pair_t serviceHandle;
        cy_stc_ble_gatt_value_t serviceData;
        
        serviceData.val = (uint8*)data;
        serviceData.len = 1;
        
        serviceHandle.attrHandle = CY_BLE_DEVICE_INTERFACE_DEVICE_OUTBOUND_CHAR_HANDLE;
        serviceHandle.value = serviceData;
        
        Cy_BLE_GATTS_WriteAttributeValueLocal(&serviceHandle);
        if (set ==  1) {
              data[0]++;
        }
        CyDelay(1000);
        
        /*
        if (bluetooth_conn == 1)
        {
            squareWave();
        }
        else
        {
            Cy_GPIO_Write(P9_6_PORT, P9_6_NUM, 0);
        }*/
    }
}


/* [] END OF FILE */
