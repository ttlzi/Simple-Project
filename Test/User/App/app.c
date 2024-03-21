#include "main.h"

#define     START_TASK_PRIO             (1)
#define     START_STACK_SIZE            (128)
void start_task(void *pvParameters);

#define     LED_TASK_PRIO               (10)
#define     LED_STACK_SIZE              (128)
xTaskHandle LED_Task_Handle = NULL;
void led_task(void *pvParameters);

void start_task(void *pvParameters)
{
    //进入临界区
    taskENTER_CRITICAL();
    //创建LED任务
    xTaskCreate(led_task,"led_task",LED_STACK_SIZE,NULL,LED_TASK_PRIO,&LED_Task_Handle);
    //退出临界区
    taskEXIT_CRITICAL();
    //删除任务自身
    vTaskDelete(NULL);
}

void led_task(void *pvParameters)
{
    static bool led_status = false;
    while (1)
    {
        if(led_status == false)
        {
            led_status = true;
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
        }
        else if(led_status == true)
        {
            led_status = false;
            HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void app_run(void)
{
    //创建初始任务
    xTaskCreate(start_task,"start_task",START_STACK_SIZE,NULL,START_TASK_PRIO,NULL);
    //开启任务调度
    vTaskStartScheduler();
}
