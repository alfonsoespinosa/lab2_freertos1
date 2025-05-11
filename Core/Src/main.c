#include "main.h"
#include "hw.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usb_device.h"

#define DELAY_RED   500
#define DELAY_GREEN 400
#define DELAY_BLUE  100

TaskHandle_t default_task_handle;
void start_default_task(void *argument);

typedef struct Led {
  GPIO_TypeDef *port;
  uint16_t pin;
  int delay;
} Led;

void toggle_led(GPIO_TypeDef *port, uint16_t pin)
{
  HAL_GPIO_TogglePin(port, pin);
}

void led_task(void *arg)
{
  struct Led *led = (struct Led *)arg;

  while (1) {
    toggle_led(led->port, led->pin);
    vTaskDelay(led->delay);
  }
}

TaskHandle_t red_led_task_handle;
TaskHandle_t green_led_task_handle;
TaskHandle_t blue_led_task_handle;

int main(void)
{
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

  hw_init();

  /* Create the thread(s) */
  /* creation of defaultTask */
  if (xTaskCreate(start_default_task,
                  "default_task",
                  128,
                  NULL,
                  7,
                  &default_task_handle) != pdPASS) {
    Error_Handler();
  }

  if (xTaskCreate(led_task,
                  "red_led_task",
                  128,
                  &red_led,
                  7,
                  &green_led_task_handle) != pdPASS) {
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
                  &green_led_task_handle) != pdPASS) {
    Error_Handler();
  }

  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  while (1)
  {
  }
}

void start_default_task(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* Infinite loop */
  for(;;)
  {
    vTaskDelay(1);
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
