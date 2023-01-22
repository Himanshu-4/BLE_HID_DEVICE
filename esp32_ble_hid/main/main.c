
#include "adxl345.h"
#include "ble_hid.h"

#include "freertos/task.h"
#include "freertos/FreeRTOS.h"


// create the queue handle
// static QueueHandle_t int1_q_hand;
// static QueueHandle_t int2_q_hand;
// static QueueHandle_t int3_q_hand;

volatile uint8_t flag = 0;

#define left_btn 0x01
#define middle_btn 0x20
#define right_btn 0x40

#define delay(x) vTaskDelay(x / portTICK_PERIOD_MS)

// add the function prototype of interrupt handler
void gpio_18_handler(void);
void gpio_19_handler(void);
void gpio_4_handler(void);

void app_main()
{
    ble_init();

    adxl_init();
    adxl_cfg mycfg =
        {
            .link_autosleep = _enable,
            .low_pwr = _disable,
            .output_range = _4g,
            .rate = _25};

    adxl_cfg_(&mycfg);

    // adxl_cfg_fifo(_bypass, 2);
    // first disable the interrupt
    disable_all_ints();

    // configure the interrupt
    taps_configurations tap_cfg_data =
        {
            .tap_axes = z_axis | y_axis | x_axis,
            .tap_type = double_tap | singletap,
            .tap_thresh = 50, // 20 X 62.5mg = 1.2g
            .tap_durat = 30,  // 625 X 20 = 12 milisecond
            .double_tap_window = 200,
            .double_tap_lattency = 100,
        };
    adxl_cfg_taps(&tap_cfg_data);

    // configure activity and inactivity interrupt 

    // activity_inact_config actv_cfg_data =
    // {
    //     .act.axes = act_y_en  ,
    //     .act.thresh_act = 20,
    //     .act._ac_dc = dc_oper,

    //     // configure inactivity 
    //     .inact.axes = inact_y_en,
    //     .inact.thresh_inact = 20,
    //     .inact.time_inact = 5, //  5 seconds for inact
    //     .inact._ac_dc = dc_oper,
    // };
    // adxl_cfg_act_inact(&actv_cfg_data);



    // create the quues
    // int1_q_hand = xQueueCreate(5, 1);
    // int2_q_hand = xQueueCreate(5, 1);
    // int3_q_hand = xQueueCreate(5, 1);

    // check for null handler
    // assert(int1_q_hand);
    // assert(int2_q_hand);
    // assert(int3_q_hand);

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// add and set the interrupt definations
    // install the isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    /// @brief  confiugre the gpio pin 18
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_18, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(GPIO_NUM_18, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_NUM_18, gpio_18_handler, NULL);
    

    /// @brief  confiugre the gpio pin 19
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_19, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(GPIO_NUM_19, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_NUM_19, gpio_19_handler, NULL);

    // configure the gpio for interrupt

        /// @brief  confiugre the gpio pin 4
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_4, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(GPIO_NUM_4, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_NUM_4, gpio_4_handler, NULL);

    // add isr handler for pin 18
    /////// set the intr type to falling edge
    gpio_intr_disable(GPIO_NUM_4);
    gpio_intr_disable(GPIO_NUM_18);
    gpio_intr_disable(GPIO_NUM_19);
    ///////// add the isr


    // enable the interrupt on the line 
    gpio_intr_enable(GPIO_NUM_18);


    // enable the int for activity and inacitvity detection
    cfg_int(_single_tap, _enable);
    cfg_int(_double_tap, _enable);
    // cfg_int(_activity, _enable);
    // cfg_int(_inactivity, _enable);


    uint8_t int1_val = 0;
    uint8_t int2_val = 0;
    uint8_t int3_val = 0;


    float acc[3] ={0};
    // uint8_t buf[6] ={0};
    while (1)
    {

        // send_mouse_data0(-20,-20,0,0);
        // delay(1000);
        // send_mouse_data1(10,0,0,0);
        
        if( flag ==1)
        {
            flag =0;
            int1_val = read_int_type();
            if(read_bit(int1_val, _single_tap))
            {
                printf("single tap\r\n");
                send_mouse_data(0,0,0,left_btn);
            }
            if (read_bit(int1_val, _double_tap) )
            {
                printf("double tap\r\n");
                send_mouse_data(0,0,0,right_btn);
            }
            if(read_bit(int1_val, _activity))
            {
                printf("activity \r\n");
            }
            if(read_bit(int1_val, _inactivity))
            {
                printf("inactivity \r\n");
            }
        }
        // else if (xQueueReceive(int3_q_hand, &int2_val, 2) == pdTRUE )
        // {
        //      printf(" int 2 int %d\r\n",int2_val);
        // }
        // else if (xQueueReceive(int3_q_hand, &int3_val, 4) == pdTRUE)
        // {
        //     printf("interpt recv on pin 4val is %d\r\n",int3_val);
        // }
        // printf("the int  is %x and pin 18 %d and pin 19 %d \r\n", read_int_type(), gpio_get_level(GPIO_NUM_18), gpio_get_level(GPIO_NUM_19));
        // delay(300);

        read_accelration(acc);
        // printf("%f,%f,%f\r\n", acc[0], acc[1] , acc[2]);
        send_mouse_data(acc[0]/80,acc[1]/80,0,0);
        delay(10);
        //     send_mouse_data(12, 13, 0, 0);
        // read_data(buf, 6);
        // printf(" x is %d, y is %d, z is %d\r\n", (int)((buf[1]<<8) | buf[0]) ,(int) (( buf[3]<<8) | buf[2]) , (int) ((buf[5]<<8 ) | buf[4] ));
        //  delay(300);
        

    }
}

void IRAM_ATTR gpio_18_handler(void)
{
    // // uint8_t context_switch = pdFALSE;
    // uint8_t val = 0;
    // xQueueSendFromISR(int1_q_hand, &val, &context_switch );
    flag =1;

}

void IRAM_ATTR gpio_19_handler(void)
{
 
}


void IRAM_ATTR gpio_4_handler(void)
{

}