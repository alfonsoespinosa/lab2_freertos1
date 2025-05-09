#include "main.h"
#include "hw.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usb_device.h"

TaskHandle_t default_task_handle;
void start_default_task(void *argument);

int main(void)
{
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
