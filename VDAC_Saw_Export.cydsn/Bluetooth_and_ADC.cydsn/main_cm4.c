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

// global variables
static float currentVal[1] = {0.00};
static uint8_t test[4] = {0x01,0x02,0x0a,0x0b};
int bluetooth_conn = 0;

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
    
    Cy_BLE_Start(genericEventHandler);
    
    while(Cy_BLE_GetState() != CY_BLE_STATE_ON)
    {
        Cy_BLE_ProcessEvents();
    }
    Cy_BLE_RegisterAppHostCallback(bleInterruptNotify);

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    UART_1_Start();
    ADC_1_Start();
    Cy_SAR_StartConvert(SAR, CY_SAR_START_CONVERT_CONTINUOUS);
    
    // voltage
    float v1;
    int16_t voltCount;
    printf("hello\n");

    for(;;)
    {
        /* Place your application code here. */
        // Read ADC values
        voltCount = Cy_SAR_GetResult16(SAR, 0);
        v1 = Cy_SAR_CountsTo_Volts(SAR, 0, voltCount);
        /* Place your application code here. */
        printf("Volt is: %f\n\n", v1);
        currentVal[0] = v1;
        
        cy_stc_ble_gatt_handle_value_pair_t serviceHandle;
        cy_stc_ble_gatt_value_t serviceData;
        
        serviceData.val = (uint8*)currentVal;
        serviceData.len = sizeof(currentVal[0]);
        
        serviceHandle.attrHandle = CY_BLE_DEVICE_INTERFACE_CURRENT_VAL_CHAR_HANDLE;
        serviceHandle.value = serviceData;
        Cy_BLE_GATTS_WriteAttributeValueLocal(&serviceHandle);
        
        CyDelay(500);
    }
}

/* [] END OF FILE */
