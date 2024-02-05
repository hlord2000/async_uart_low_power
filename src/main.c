/*
 * Copyright (c) 2024 Kelly Helmut Lord	
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h> 
#include <zephyr/logging/log.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>
#include <zephyr/drivers/uart.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define RX_BUF_SIZE 256 

#define CONSOLE_DEVICE_NODE DT_CHOSEN(zephyr_console)
static const struct device *const console_dev = DEVICE_DT_GET(CONSOLE_DEVICE_NODE);

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_NODELABEL(uart1)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

void my_uart_callback(const struct device *dev,
				struct uart_event *evt, void *user_data) {
	switch(evt->type) {
		case UART_TX_DONE:
			break;
		case UART_TX_ABORTED:
			break;
		case UART_RX_RDY:
			break;
		case UART_RX_BUF_REQUEST:
		case UART_RX_BUF_RELEASED:
			break;
		case UART_RX_DISABLED:
		case UART_RX_STOPPED:
			break;
	}
}

int main(void)
{
	int err;	

	if (!device_is_ready(uart_dev))
    {
		LOG_ERR("UART device not ready");
        return -EIO;
    }

    /* Set the callback */
    err = uart_callback_set(uart_dev, my_uart_callback, NULL);
	if (err) {
		LOG_ERR("Failed to set UART callback (err %d)", err);
	}

	uint8_t rx_buf[RX_BUF_SIZE];
	memset(rx_buf, 0, sizeof(rx_buf));

	/* Since uart1, by default, does not use flow control we can set to infinite timeout */
	err = uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), SYS_FOREVER_US);
	if (err) {
		LOG_ERR("Failed to enable RX (err %d)", err);
	}

	uint8_t tx_buf[] = "Hello World!\r\n";

	/* Since uart1, by default, does not use flow control we can set to infinite timeout */
	err = uart_tx(uart_dev, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
	if (err) {
		LOG_ERR("Failed to transmit (err %d)", err);
	}

	k_msleep(100);

	LOG_INF("Received: %s", rx_buf);

	k_msleep(1000);

	err = uart_rx_disable(uart_dev);
	if (err) {
		LOG_ERR("Failed to disable RX (err %d)", err);
	}

	k_msleep(100);

	pm_device_action_run(uart_dev, PM_DEVICE_ACTION_SUSPEND);

	pm_device_action_run(console_dev, PM_DEVICE_ACTION_SUSPEND);
	
	return 0;
}
