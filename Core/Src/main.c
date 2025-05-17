#include "main.h"
#include "hw.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#define DELAY_RED   500
#define DELAY_GREEN 400
#define DELAY_BLUE  100

TaskHandle_t red_led_task_handle;
TaskHandle_t green_led_task_handle;
TaskHandle_t blue_led_task_handle;
TaskHandle_t usb_task_handle;

void toggle_led(GPIO_TypeDef *port, uint16_t pin);
void get_command(char *command);
void process_red(int period);
void process_green(int period);
void process_blue(int period);

void led_task(void *arg);
void uart_task(void *arg);
void usb_task(void *arg);

typedef struct Led {
  GPIO_TypeDef *port;
  uint16_t pin;
  int delay;
  QueueHandle_t queue;
} Led;

static struct Led red_led = {
    .port = RED_LED_GPIO_PORT,
    .pin = RED_LED_GPIO_PIN,
    .delay = DELAY_RED
};

static struct Led green_led = {
    .port = GREEN_LED_GPIO_PORT,
    .pin = GREEN_LED_GPIO_PIN,
    .delay = DELAY_GREEN
};

static struct Led blue_led = {
    .port = BLUE_LED_GPIO_PORT,
    .pin = BLUE_LED_GPIO_PIN,
    .delay = DELAY_BLUE
};

void toggle_timer_func( TimerHandle_t xTimer )
{
	struct Led *led = pvTimerGetTimerID( xTimer );
	toggle_led(led->port, led->pin);
}

int main(void)
{
  hw_init();

  red_led.queue = xQueueCreate(1, sizeof(uint32_t));
  green_led.queue = xQueueCreate(1, sizeof(uint32_t));
  blue_led.queue = xQueueCreate(1, sizeof(uint32_t));

  if (xTaskCreate(uart_task,
                  "uart_task",
                  128,
                  NULL,
                  7,
                  &usb_task_handle) != pdPASS) {
    Error_Handler();
  }

  if (xTaskCreate(led_task,
                  "red_led_task",
                  128,
                  &red_led,
                  7,
                  &red_led_task_handle) != pdPASS) {
    Error_Handler();
  }

  if (xTaskCreate(led_task,
                  "green_led_task",
                  128,
                  &green_led,
                  7,
                  &green_led_task_handle) != pdPASS) {
    Error_Handler();
  }

  if (xTaskCreate(led_task,
                  "blue_led_task",
                  128,
                  &blue_led,
                  7,
                  &blue_led_task_handle) != pdPASS) {
    Error_Handler();
  }

//  TimerHandle_t red_timer = xTimerCreate("red_timer",
//                                         DELAY_RED,
//										 pdTRUE,
//										 &red_led,
//										 toggle_timer_func);
//
//  xTimerStart(red_timer, 0);

  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  while (1)
  {
  }
}

void toggle_led(GPIO_TypeDef *port, uint16_t pin)
{
  HAL_GPIO_TogglePin(port, pin);
}

void process_red(int period)
{
  xQueueSend(red_led.queue, &period, 0);
}

void process_green(int period)
{
  xQueueSend(green_led.queue, &period, 0);
}

void process_blue(int period)
{
  xQueueSend(blue_led.queue, &period, 0);
}

void led_task(void *arg)
{
  struct Led *led = (struct Led *)arg;
  uint32_t period = led->delay;

  while (1) {
    if (xQueueReceive(led->queue, &period, period) == pdFALSE) {
	    toggle_led(led->port, led->pin);
    }
  }
}

void uart_task(void *argument)
{
  char buf[32];
  void (*process_fn)(int period);

  /* Infinite loop */
  for(;;)
  {
    get_command(buf);

    process_fn = NULL;

    switch (buf[0]) {
    case 'r':
      process_fn = &process_red;
      break;
    case 'g':
      process_fn = &process_green;
      break;
    case 'b':
      process_fn = &process_blue;
      break;
    }

    if (process_fn) {
      int period = strtoul(&buf[1], NULL, 10);
      if (period) {
        process_fn(period);
      }
    }
  }
}

void usb_task(void *arg)
{
  MX_USB_DEVICE_Init();

  static const char *buf = "Datos\r\n";

  /* Infinite loop */
  for(;;)
  {
    CDC_Transmit_FS((uint8_t *)buf, strlen(buf));
    vTaskDelay(1000);
  }
}

void get_command(char *command)
{
  uint8_t character;

  while (1) {
    if (HAL_UART_Receive(&huart3, &character, 1, 50) != HAL_TIMEOUT) {
      HAL_UART_Transmit(&huart3, &character, 1, HAL_MAX_DELAY);
      if (character == '\r') {
        uint8_t LF = '\n';
        HAL_UART_Transmit(&huart3, &LF, 1, HAL_MAX_DELAY);
        *command = '\0';
        return;
      }
      *command = character;
      command++;
    }
  }
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  HAL_IncTick();

  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == USER_BUTTON_GPIO_PIN)
  {
    vTaskNotifyGiveFromISR(red_led_task_handle, NULL);
  }
  else if (GPIO_Pin == GPIO_PIN_9)
  {
    HAL_PCDEx_BCD_VBUSDetect(&hpcd_USB_OTG_FS);
  }
}

void Error_Handler(void)
{
  taskDISABLE_INTERRUPTS();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
