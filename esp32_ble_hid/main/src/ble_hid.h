#ifndef _BLE_HID_
#define _BLE_HID_

#include "hid_info.h"

#include "esp_err.h"

// common bt layer api 
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt.h"


// gap layer APIs 
#include "esp_gap_ble_api.h"

// gatt layer APIs
#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"


////////// esp32 BLE and gatt , gap specific API
#include "esp_wifi.h"

////// nvs lib
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_spi_flash.h"


/////// esp32 bare metal supportive libs
#include "esp_system.h"
#include "esp_log.h"

#include "sdkconfig.h" 

// define the utility macro 
#define u8(x) ((uint8_t *)&x)

#define s(x) (sizeof(x))

#define _1byte 1
#define _2byte 2
#define _3byte 3


// define the app id to register 
#define BATT_APP_ID 0
#define HID_APP_ID 1

#define GATTS_TAG "blehid"


// specfy the device name 
#define DEVICE_NAME "MYBLE"

#define SERVICE_BATT 0
#define SERVICE_HID 1

#define PROFILE_BATT_ID 0
#define PROFILE_HID_ID 1


/// characteristic presentation information for the battery service hid 
struct prf_char_pres_fmt
{
    /// Unit (The Unit is a UUID)
    uint16_t unit;
    /// Description
    uint16_t description;
    /// Format
    uint8_t format;
    /// Exponent
    uint8_t exponent;
    /// Name space
    uint8_t name_space;
};


// typedef struct
// {
//     uint16_t start_hdl;                                     /*!< Gatt  start handle value of included service */
//     uint16_t end_hdl;                                       /*!< Gatt  end handle value of included service */
//     uint16_t uuid;                                          /*!< Gatt  attribute value UUID of included service */
// } esp_gatts_incl_svc_desc_t;    




/// Battery Service Attributes Indexes
enum
{
    BAS_IDX_SVC,

    BAS_IDX_BATT_LVL_CHAR,
    BAS_IDX_BATT_LVL_VAL,
    BAS_IDX_BATT_LVL_NTF_CFG,
    BAS_IDX_BATT_LVL_PRES_FMT,

    BATT_SRVC_NO_ELE,
};

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)



struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
};


/// Maximal number of HIDS that can be added in the DB
#ifndef USE_ONE_HIDS_INSTANCE
#define HIDD_LE_NB_HIDS_INST_MAX              (2)
#else
#define HIDD_LE_NB_HIDS_INST_MAX              (1)
#endif

#define HIDD_GREAT_VER   0x01  //Version + Subversion
#define HIDD_SUB_VER     0x00  //Version + Subversion
#define HIDD_VERSION     ((HIDD_GREAT_VER<<8)|HIDD_SUB_VER)  //Version + Subversion

#define HID_MAX_APPS                 1

// Number of HID reports defined in the service
#define HID_NUM_REPORTS          9

// HID Report IDs for the service
#define HID_RPT_ID_MOUSE_IN      1   // Mouse input report ID
#define HID_RPT_ID_KEY_IN        2   // Keyboard input report ID
#define HID_RPT_ID_CC_IN         3   //Consumer Control input report ID
#define HID_RPT_ID_VENDOR_OUT    4   // Vendor output report ID
#define HID_RPT_ID_LED_OUT       0  // LED output report ID
#define HID_RPT_ID_FEATURE       0  // Feature report ID

#define HIDD_APP_ID			0x1812//ATT_SVC_HID

#define BATTRAY_APP_ID       0x180f


#define ATT_SVC_HID          0x1812

/// Maximal number of Report Char. that can be added in the DB for one HIDS - Up to 11
#define HIDD_LE_NB_REPORT_INST_MAX            (5)

/// Maximal length of Report Char. Value
#define HIDD_LE_REPORT_MAX_LEN                (255)
/// Maximal length of Report Map Char. Value
#define HIDD_LE_REPORT_MAP_MAX_LEN            (512)

/// Length of Boot Report Char. Value Maximal Length
#define HIDD_LE_BOOT_REPORT_MAX_LEN           (8)

/// Boot KB Input Report Notification Configuration Bit Mask
#define HIDD_LE_BOOT_KB_IN_NTF_CFG_MASK       (0x40)
/// Boot KB Input Report Notification Configuration Bit Mask
#define HIDD_LE_BOOT_MOUSE_IN_NTF_CFG_MASK    (0x80)
/// Boot Report Notification Configuration Bit Mask
#define HIDD_LE_REPORT_NTF_CFG_MASK           (0x20)


/* HID information flags */
#define HID_FLAGS_REMOTE_WAKE           0x01      // RemoteWake
#define HID_FLAGS_NORMALLY_CONNECTABLE  0x02      // NormallyConnectable

/* Control point commands */
#define HID_CMD_SUSPEND                 0x00      // Suspend
#define HID_CMD_EXIT_SUSPEND            0x01      // Exit Suspend

/* HID protocol mode values */
#define HID_PROTOCOL_MODE_BOOT          0x00      // Boot Protocol Mode
#define HID_PROTOCOL_MODE_REPORT        0x01      // Report Protocol Mode

/* Attribute value lengths */
#define HID_PROTOCOL_MODE_LEN           1         // HID Protocol Mode
#define HID_INFORMATION_LEN             4         // HID Information
#define HID_REPORT_REF_LEN              2         // HID Report Reference Descriptor
#define HID_EXT_REPORT_REF_LEN          2         // External Report Reference Descriptor

// HID feature flags
#define HID_KBD_FLAGS             HID_FLAGS_REMOTE_WAKE

/* HID Report type */
#define HID_REPORT_TYPE_INPUT       1
#define HID_REPORT_TYPE_OUTPUT      2
#define HID_REPORT_TYPE_FEATURE     3


/// HID Service Attributes Indexes
enum {
    HIDD_LE_IDX_SVC,

    // Included Service
    HIDD_LE_IDX_INCL_SVC,

    // HID Information
    HIDD_LE_IDX_HID_INFO_CHAR,
    HIDD_LE_IDX_HID_INFO_VAL,

    // HID Control Point
    HIDD_LE_IDX_HID_CTNL_PT_CHAR,
    HIDD_LE_IDX_HID_CTNL_PT_VAL,

    // Report Map
    HIDD_LE_IDX_REPORT_MAP_CHAR,
    HIDD_LE_IDX_REPORT_MAP_VAL,
    HIDD_LE_IDX_REPORT_MAP_EXT_REP_REF,

    // Protocol Mode
    HIDD_LE_IDX_PROTO_MODE_CHAR,
    HIDD_LE_IDX_PROTO_MODE_VAL,

    // Report mouse input
    HIDD_LE_IDX_REPORT_MOUSE_IN_CHAR,
    HIDD_LE_IDX_REPORT_MOUSE_IN_VAL,
    HIDD_LE_IDX_REPORT_MOUSE_IN_CCC,
    HIDD_LE_IDX_REPORT_MOUSE_REP_REF,
    //Report Key input
    HIDD_LE_IDX_REPORT_KEY_IN_CHAR,
    HIDD_LE_IDX_REPORT_KEY_IN_VAL,
    HIDD_LE_IDX_REPORT_KEY_IN_CCC,
    HIDD_LE_IDX_REPORT_KEY_IN_REP_REF,
    ///Report Led output
    HIDD_LE_IDX_REPORT_LED_OUT_CHAR,
    HIDD_LE_IDX_REPORT_LED_OUT_VAL,
    HIDD_LE_IDX_REPORT_LED_OUT_REP_REF,
    // consumer control input report 
    HIDD_LE_IDX_REPORT_CC_IN_CHAR,
    HIDD_LE_IDX_REPORT_CC_IN_VAL,
    HIDD_LE_IDX_REPORT_CC_IN_CCC,
    HIDD_LE_IDX_REPORT_CC_IN_REP_REF,

    // Boot Keyboard Input Report
    HIDD_LE_IDX_BOOT_KB_IN_REPORT_CHAR,
    HIDD_LE_IDX_BOOT_KB_IN_REPORT_VAL,
    HIDD_LE_IDX_BOOT_KB_IN_REPORT_NTF_CFG,

    // Boot Keyboard Output Report
    HIDD_LE_IDX_BOOT_KB_OUT_REPORT_CHAR,
    HIDD_LE_IDX_BOOT_KB_OUT_REPORT_VAL,

    // Boot Mouse Input Report
    HIDD_LE_IDX_BOOT_MOUSE_IN_REPORT_CHAR,
    HIDD_LE_IDX_BOOT_MOUSE_IN_REPORT_VAL,
    HIDD_LE_IDX_BOOT_MOUSE_IN_REPORT_NTF_CFG,

    // Report
    HIDD_LE_IDX_REPORT_CHAR,
    HIDD_LE_IDX_REPORT_VAL,
    HIDD_LE_IDX_REPORT_REP_REF,
    //HIDD_LE_IDX_REPORT_NTF_CFG,

    HID_SRVC_NO_ELE,
};


/// Attribute Table Indexes
enum {
    HIDD_LE_INFO_CHAR,
    HIDD_LE_CTNL_PT_CHAR,
    HIDD_LE_REPORT_MAP_CHAR,
    HIDD_LE_REPORT_CHAR,
    HIDD_LE_PROTO_MODE_CHAR,
    HIDD_LE_BOOT_KB_IN_REPORT_CHAR,
    HIDD_LE_BOOT_KB_OUT_REPORT_CHAR,
    HIDD_LE_BOOT_MOUSE_IN_REPORT_CHAR,
    HIDD_LE_CHAR_MAX //= HIDD_LE_REPORT_CHAR + HIDD_LE_NB_REPORT_INST_MAX,
};

///att read event table Indexs
enum {
    HIDD_LE_READ_INFO_EVT,
    HIDD_LE_READ_CTNL_PT_EVT,
    HIDD_LE_READ_REPORT_MAP_EVT,
    HIDD_LE_READ_REPORT_EVT,
    HIDD_LE_READ_PROTO_MODE_EVT,
    HIDD_LE_BOOT_KB_IN_REPORT_EVT,
    HIDD_LE_BOOT_KB_OUT_REPORT_EVT,
    HIDD_LE_BOOT_MOUSE_IN_REPORT_EVT,

    HID_LE_EVT_MAX
};

/// Client Characteristic Configuration Codes
enum {
    HIDD_LE_DESC_MASK = 0x10,

    HIDD_LE_BOOT_KB_IN_REPORT_CFG     = HIDD_LE_BOOT_KB_IN_REPORT_CHAR | HIDD_LE_DESC_MASK,
    HIDD_LE_BOOT_MOUSE_IN_REPORT_CFG  = HIDD_LE_BOOT_MOUSE_IN_REPORT_CHAR | HIDD_LE_DESC_MASK,
    HIDD_LE_REPORT_CFG                = HIDD_LE_REPORT_CHAR | HIDD_LE_DESC_MASK,
};

/// Features Flag Values
enum {
    HIDD_LE_CFG_KEYBOARD      = 0x01,
    HIDD_LE_CFG_MOUSE         = 0x02,
    HIDD_LE_CFG_PROTO_MODE    = 0x04,
    HIDD_LE_CFG_MAP_EXT_REF   = 0x08,
    HIDD_LE_CFG_BOOT_KB_WR    = 0x10,
    HIDD_LE_CFG_BOOT_MOUSE_WR = 0x20,
};

/// Report Char. Configuration Flag Values
enum {
    HIDD_LE_CFG_REPORT_IN     = 0x01,
    HIDD_LE_CFG_REPORT_OUT    = 0x02,
    //HOGPD_CFG_REPORT_FEAT can be used as a mask to check Report type
    HIDD_LE_CFG_REPORT_FEAT   = 0x03,
    HIDD_LE_CFG_REPORT_WR     = 0x10,
};

/// Pointer to the connection clean-up function
#define HIDD_LE_CLEANUP_FNCT        (NULL)

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// HIDD Features structure
typedef struct {
    /// Service Features
    uint8_t svc_features;
    /// Number of Report Char. instances to add in the database
    uint8_t report_nb;
    /// Report Char. Configuration
    uint8_t report_char_cfg[HIDD_LE_NB_REPORT_INST_MAX];
} hidd_feature_t;


typedef struct {
    bool                        in_use;
    bool                        congest;
    uint16_t                  conn_id;
    bool                        connected;
    esp_bd_addr_t         remote_bda;
    uint32_t                  trans_id;
    uint8_t                    cur_srvc_id;

} hidd_clcb_t;

// HID report mapping table
typedef struct {
    uint16_t    handle;           // Handle of report characteristic
    uint16_t    cccdHandle;       // Handle of CCCD for report characteristic
    uint8_t     id;               // Report ID
    uint8_t     type;             // Report type
    uint8_t     mode;             // Protocol mode (report or boot)
} hidRptMap_t;


// typedef struct {
//     /// hidd profile id
//     uint8_t app_id;
//     /// Notified handle
//     uint16_t ntf_handle;
//     ///Attribute handle Table
//     uint16_t att_tbl[HIDD_LE_IDX_NB];
//     /// Supported Features
//     hidd_feature_t   hidd_feature[HIDD_LE_NB_HIDS_INST_MAX];
//     /// Current Protocol Mode
//     uint8_t proto_mode[HIDD_LE_NB_HIDS_INST_MAX];
//     /// Number of HIDS added in the database
//     uint8_t hids_nb;
//     uint8_t pending_evt;
//     uint16_t pending_hal;
// } hidd_inst_t;

/// Report Reference structure
typedef struct
{
    ///Report ID
    uint8_t report_id;
    ///Report Type
    uint8_t report_type;
}hids_report_ref_t;

/// HID Information structure
typedef struct
{
    /// bcdHID
    uint16_t bcdHID;
    /// bCountryCode
    uint8_t bCountryCode;
    /// Flags
    uint8_t flags;
}hids_hid_info_t;


typedef struct __packed _DEVICE_TREE_STR
{
    // the connection status if connected or not 
    uint8_t connection_status;
    uint16_t  gatt_interf; // the gatt if  
    uint16_t conn_id; //  the connection id 
    uint16_t database_handle; // the database handle 
    uint8_t hid_protocol_mode; // hid protocol mode 

}device_tree_str;


/* service engine control block */
// typedef struct {
//     hidd_clcb_t                  hidd_clcb[HID_MAX_APPS];          /* connection link*/
//     esp_gatt_if_t                gatt_if;
//     bool                         enabled;
//     bool                         is_take;
//     bool                         is_primery;
//     hidd_inst_t                  hidd_inst;
//     esp_hidd_event_cb_t          hidd_cb;
//     uint8_t                      inst_id;
// } hidd_le_env_t;


void hid_dev_register_reports(uint8_t num_reports, hid_report_map_t *p_report);

void hid_dev_send_report(esp_gatt_if_t gatts_if, uint16_t conn_id,
                                    uint8_t id, uint8_t type, uint8_t length, uint8_t *data);

void hid_consumer_build_report(uint8_t *buffer, consumer_cmd_t cmd);

void hid_keyboard_build_report(uint8_t *buffer, keyboard_cmd_t cmd);

void hid_mouse_build_report(uint8_t *buffer, mouse_cmd_t cmd);


// init the ble module 
void ble_init(void);


// typedef  __packed struct _MOUSE_DATA_
// {
//     uint8_t btns;
//     int8_t x;
//     int8_t y;
//     int8_t wheel;
// }mouse_data;


// send the mouse data 
void send_mouse_data(uint8_t x, uint8_t y,uint8_t wheel,  uint8_t btn);

// send the mouse data for 1
// void send_mouse_data1(uint8_t x, uint8_t y,uint8_t wheel,  uint8_t btn);


void send_keyboard_data(uint16_t conn_id, key_mask_t special_key_mask, uint8_t *keyboard_cmd, uint8_t num_key);




#endif