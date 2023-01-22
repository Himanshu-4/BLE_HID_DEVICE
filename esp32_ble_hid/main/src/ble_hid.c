#include "ble_hid.h"


// init the device tree here 
static device_tree_str  my_tree[2] ={0};


static uint8_t itr=0;


static uint8_t hidd_service_uuid128[] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0x12,
    0x18,
    0x00,
    0x00,
};



// ######################################################
//  ############################# define here function callback

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = ESP_BLE_APPEARANCE_GENERIC_HID,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, // test_manufacturer,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(hidd_service_uuid128),
    .p_service_uuid = hidd_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
// static esp_ble_adv_data_t scan_rsp_data = {
//     .set_scan_rsp        = true,
//     .include_name        = true,
//     .include_txpower     = true,
//     .min_interval        = 0x0006,
//     .max_interval        = 0x0010,
//     .appearance          = ESP_BLE_APPEARANCE_GENERIC_HID,
//     .manufacturer_len    = 0, //TEST_MANUFACTURER_DATA_LEN,
//     .p_manufacturer_data = NULL, //&test_manufacturer[0],
//     .service_data_len    = 0,
//     .p_service_data      = NULL,
//     .service_uuid_len    = sizeof(service_uuid),
//     .p_service_uuid      = service_uuid,
//     .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
// };

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// static uint8_t adv_config_done =0;

// #define adv_config_flag (1 << 0)
// #define scan_rsp_config_flag (1 << 1)

/// create the database handles
static uint16_t batt_db_handle[BATT_SRVC_NO_ELE];
static uint16_t hid_db_handle[HID_SRVC_NO_ELE];

/// create gatt interface for the battery and HID
static esp_gatt_if_t gatt_if_batt_srv;
static esp_gatt_if_t gatt_if_hid_srv;

///  counter for profile id
static uint8_t prof_id;


// for secure connection checking
static bool sec_conn;

////////// give a prototype to the compiler about the database tables
extern const esp_gatts_attr_db_t bat_att_db_table[BATT_SRVC_NO_ELE];
extern const esp_gatts_attr_db_t hidd_le_gatt_db_table[HID_SRVC_NO_ELE];

extern esp_gatts_incl_svc_desc_t incl_svc;
////////////////////////// gatt event handler  //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void gatts_events_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT: /////////// this is the register event we have to create a service table here
    {                       /////// you cannot declare a variable after case thats why a new scope if inserted here
        if (prof_id == PROFILE_BATT_ID)
        {

            esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(DEVICE_NAME);
            if (set_dev_name_ret)
            {
                ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
            }
            // config adv data
            esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret)
            {
                ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
            }
            // adv_config_done |= adv_config_flag;
            // // config scan response data
            // ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            // if (ret)
            // {
            //     ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
            // }

            // adv_config_done |= scan_rsp_config_flag;

            if (gatts_if != 0)
                gatt_if_batt_srv = gatts_if;

            if (esp_ble_gatts_create_attr_tab(bat_att_db_table, gatt_if_batt_srv, BATT_SRVC_NO_ELE, SERVICE_BATT) != ESP_OK)
            {
                ESP_LOGE(GATTS_TAG, "failed attr table for device information service \r\n");
            }
            printf("created attr table for battery service\r\n");
        }

        else if (prof_id == PROFILE_HID_ID)
        {
            printf(" db table for hid\r\n");
            if (gatts_if != gatt_if_batt_srv)
                gatt_if_hid_srv = gatts_if;
            if (esp_ble_gatts_create_attr_tab(hidd_le_gatt_db_table, gatt_if_hid_srv, HID_SRVC_NO_ELE, SERVICE_HID) != ESP_OK)
            {
                ESP_LOGE(GATTS_TAG, "failed attr table for battery  service \r\n");
            }
        }

        prof_id++;
    }
    break;
    case ESP_GATTS_READ_EVT:
    {
        ESP_LOGI(GATTS_TAG, "GATT_READ_EVT, gatt_interface  %d, trans_id %d, handle %d\n", gatts_if, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp = {0};

        rsp.attr_value.handle = param->read.handle;
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
        break;
    }
    case ESP_GATTS_WRITE_EVT: /////////// gatt write event it is value recieve by the esp32
        if (!param->write.is_prep)
        {

            // the data length of gattc write  must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
            ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :\r\n", param->write.handle, param->write.len);
            esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
            //   check the data for the attribute values

            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        }
        break;

    case ESP_GATTS_EXEC_WRITE_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_EXEC_WRITE_EVT\r\n");
        // esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        // example_exec_write_event_env(&ble_data, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT, MTU %d\r\n", param->mtu.mtu);
        break;
    case ESP_GATTS_CONF_EVT:
        // ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d", param->conf.status, param->conf.handle);
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\r\n", param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_CONNECT_EVT:
        ///////////// connection update have to be done only once
        if (gatts_if == gatt_if_batt_srv)
        {
            ESP_LOGI(GATTS_TAG, "connect the devicee , conn_id = %d\r\n", param->connect.conn_id);

            esp_log_buffer_hex(GATTS_TAG, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t)); /// copy the bluetooth device address
            /* For the iOS system, please refer to Apple official documents about the BLE connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x08; // max_int = 0x7*1.25ms = 8.25ms
            conn_params.min_int = 0x06; // min_int = 0x10*1.25ms = 7.5ms
            conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
            // start sent the update connection parameters to the peer device,
            /// .......... update the connection paramter for the device
            esp_ble_gap_update_conn_params(&conn_params);

            // ////////// saving the connection id
            // connection_id = param->connect.conn_id;

            if(itr ==0) // store in the device tree 0
            {
                my_tree[0].connection_status = 1;
                my_tree[0].conn_id = param->connect.conn_id;
                my_tree[0].gatt_interf = gatts_if;
                printf("con id %d gatt if %d\r\n",param->connect.conn_id, gatts_if);
                // rest two paramters are init in the create data base table 
                itr++;
                esp_ble_gap_start_advertising(&adv_params);
            }
            else 
            {
                my_tree[1].connection_status = 1;
                my_tree[1].conn_id = param->connect.conn_id;
                my_tree[1].gatt_interf = gatts_if;
                printf("con id %d gatt if %d\r\n",param->connect.conn_id, gatts_if);
            }
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_NO_MITM);
             
        }
        /////////////////////////////////////
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        if (gatts_if == gatt_if_batt_srv)
        {
            ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x\r\n", param->disconnect.reason);
            ESP_LOGI(GATTS_TAG, "disconnect the devicee , conn_id = %d\r\n", param->disconnect.conn_id);

            if(my_tree[0].conn_id == param->disconnect.conn_id)
            {
                my_tree[0].connection_status = 0;
            }
            else 
            {

                my_tree[1].connection_status = 0;
            }
        }

        break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
    {
        if (param->add_attr_tab.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTS_TAG, "create attribute table got  failed, error code=0x%x\r\n", param->add_attr_tab.status);
            break;
        }

        if (gatts_if == gatt_if_batt_srv)
        {
            if (param->add_attr_tab.num_handle != BATT_SRVC_NO_ELE)
            {
                ESP_LOGE(GATTS_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)\r\n",
                         param->add_attr_tab.num_handle, BATT_SRVC_NO_ELE);
            }
            else
            {
                ESP_LOGI(GATTS_TAG, "create attribute table successfully, the number handle = %d\n", param->add_attr_tab.num_handle);
                memcpy(batt_db_handle, param->add_attr_tab.handles, sizeof(batt_db_handle));

                // copy the included service data base handles
                incl_svc.start_hdl = batt_db_handle[BAS_IDX_SVC];
                incl_svc.end_hdl = batt_db_handle[BAS_IDX_BATT_LVL_PRES_FMT];
                printf("batt svcgatt if is %x handle for%x \r\n ",gatts_if, batt_db_handle[BAS_IDX_SVC]);
                esp_ble_gatts_start_service(batt_db_handle[BAS_IDX_SVC]);
            }
        }
        else if (gatts_if == gatt_if_hid_srv)
        {

            if (param->add_attr_tab.num_handle != HID_SRVC_NO_ELE)
            {
                ESP_LOGE(GATTS_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)\r\n",
                         param->add_attr_tab.num_handle, HID_SRVC_NO_ELE);
            }
            else
            {
                ESP_LOGI(GATTS_TAG, "create attribute table successfully, the number handle = %d\n", param->add_attr_tab.num_handle);
                memcpy(hid_db_handle, param->add_attr_tab.handles, sizeof(hid_db_handle));
                printf("hid svc gatt if  %x handle for%x \r\n ",gatts_if, hid_db_handle[BAS_IDX_SVC]);
                esp_ble_gatts_start_service(hid_db_handle[BAS_IDX_SVC]);
            }
        }

        break;
    }
    case ESP_GATTS_STOP_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    case ESP_GATTS_UNREG_EVT:
    case ESP_GATTS_DELETE_EVT:
    default:
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// GAP event Handler /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

// void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
// {
//     switch (event)
//     {
// #ifdef CONFIG_SET_RAW_ADV_DATA
//     case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
//         adv_config_done &= (~adv_config_flag);
//         if (adv_config_done == 0)
//         {
//             esp_ble_gap_start_advertising(&adv_params);
//         }
//         break;
//     case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
//         adv_config_done &= (~scan_rsp_config_flag);
//         if (adv_config_done == 0)
//         {
//             esp_ble_gap_start_advertising(&adv_params);
//         }
//         break;
// #else
//     case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
//         // adv_config_done &= (~adv_config_flag);
//         // if (adv_config_done == 0)
//         // {
//         // }
//             esp_ble_gap_start_advertising(&adv_params);
//         break;
//     case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
//         // adv_config_done &= (~scan_rsp_config_flag);
//         // if (adv_config_done == 0)
//         // {
//         // }
//             esp_ble_gap_start_advertising(&adv_params);
//         break;
// #endif
//     case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
//     printf("advertising\r\n");
//         // advertising start complete event to indicate advertising start successfully or failed
//         if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
//         {
//             ESP_LOGE(GATTS_TAG, "Advertising start failed\n");
//         }
//         break;
//     case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
//         if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
//         {
//             ESP_LOGE(GATTS_TAG, "Advertising stop failed\n");
//         }
//         else
//         {
//             ESP_LOGI(GATTS_TAG, "Stop adv successfully\n");
//         }
//         break;
//     case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
//         ESP_LOGI(GATTS_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
//                  param->update_conn_params.status,
//                  param->update_conn_params.min_int,
//                  param->update_conn_params.max_int,
//                  param->update_conn_params.conn_int,
//                  param->update_conn_params.latency,
//                  param->update_conn_params.timeout);
//         break;
//     default:
//         break;
//     }
// }
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        for (int i = 0; i < ESP_BD_ADDR_LEN; i++)
        {
            ESP_LOGD(GATTS_TAG, "%x:", param->ble_security.ble_req.bd_addr[i]);
        }
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT:
        sec_conn = true;
        esp_bd_addr_t bd_addr;
        memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTS_TAG, "remote BD_ADDR: %08x%04x",
                 (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                 (bd_addr[4] << 8) + bd_addr[5]);
        ESP_LOGI(GATTS_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(GATTS_TAG, "pair status = %s", param->ble_security.auth_cmpl.success ? "success" : "fail");
        if (!param->ble_security.auth_cmpl.success)
        {
            ESP_LOGE(GATTS_TAG, "fail reason = 0x%x", param->ble_security.auth_cmpl.fail_reason);
        }
        break;
    default:
        break;
    }
}


void send_mouse_data(uint8_t x, uint8_t y, uint8_t wheel,  uint8_t btn)
{
    if(my_tree[0].connection_status ==1)
    {
    uint8_t buff[5] ={btn , x, y, wheel, 0};
    
    esp_ble_gatts_send_indicate(gatt_if_hid_srv, my_tree[0].conn_id , hid_db_handle[ HIDD_LE_IDX_REPORT_MOUSE_IN_VAL],
                                        s(buff), buff , false);

    }
}


// void send_mouse_data1(uint8_t x, uint8_t y, uint8_t wheel,  uint8_t btn)
// {
//     if(my_tree[0].connection_status == 1)
//     {
//     uint8_t buff[5] ={btn , x, y, wheel, 0};
    
//     esp_ble_gatts_send_indicate(gatt_if_hid_srv, my_tree[1].conn_id , hid_db_handle[ HIDD_LE_IDX_REPORT_MOUSE_IN_VAL],
//                                         s(buff), buff , false);

//     }
// }
 

void ble_init(void)
{

    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // // turn off the power domain of the BT and Wifi system
    // esp_wifi_bt_power_domain_off();

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_events_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }

    // set the power level for BLE advertisement
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);

    ////////// registering app for event handlers
    if (esp_ble_gatts_app_register(BATT_APP_ID) != ESP_OK)
        printf("error in init the batt profile\r\n");

    if (esp_ble_gatts_app_register(HID_APP_ID) != ESP_OK)
        printf("error in init the hid  profile\r\n");

    /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND; // bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;       // set the IO capability to No output No input
    uint8_t key_size = 16;                          // the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribute to you,
    and the response key means which key you can distribute to the Master;
    If your BLE device act as a master, the response key means you hope which types of key of the slave should distribute to you,
    and the init key means which key you can distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

    ///// may be the max mtu size is 500 bytes
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret)
    {
        ESP_LOGE(GATTS_TAG, "set local  MTU failed, error code = %x\r\n", local_mtu_ret);
    }

    /////// above code id to implement BLE in peripheral mode , create the service , charcteristics , and handling events
}
