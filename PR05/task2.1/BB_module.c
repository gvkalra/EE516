/* LED Control with Kernel GPIO API:
 *     Turn on & Turn off 4 LEDs
 * LED   |   GPIO Signal   |    GPIO Number
 * ------|-----------------|---------------
 * USR0  |   GPIO1_21      |    53
 * USR1  |   GPIO1_22      |    54
 * USR2  |   GPIO1_23      |    55
 * USR3  |   GPIO1_24      |    56
 *
 * Functions of interest:
 * gpio_is_valid()
 * gpio_request()
 * gpio_direction_output()
 * gpio_set_value()
 * gpio_free()
 */

#include "utils.h"

#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define LED0       0   /* LED 0 */
#define LED1       1   /* LED 1 */
#define LED2       2   /* LED 2 */
#define LED3       3   /* LED 3 */
#define NUM_LED    4   /* Number of LEDs */

#define LED0_GPIO  53  /* GPIO of LED 0 */
#define LED1_GPIO  54  /* GPIO of LED 1 */
#define LED2_GPIO  55  /* GPIO of LED 2 */
#define LED3_GPIO  56  /* GPIO of LED 3 */

#define LED_OFF 0
#define LED_ON 1

#define TRUE 1
#define FALSE 0

static struct {
	unsigned int gpio; /* GPIO of LED */
	const char *label; /* GPIO label */
	bool valid; /* If TRUE, GPIO is requested & allocated */
} gpio_data[NUM_LED] = {
		[LED0] = {LED0_GPIO, "LED0_GPIO", FALSE},
		[LED1] = {LED1_GPIO, "LED1_GPIO", FALSE},
		[LED2] = {LED2_GPIO, "LED2_GPIO", FALSE},
		[LED3] = {LED3_GPIO, "LED3_GPIO", FALSE},
};

/* Given GPIO, turns on LED */
static void
_turn_on_led(unsigned int gpio)
{
	/* sanity check */
	if (gpio_is_valid(gpio) == FALSE) {
		err("Invalid GPIO: [%u]", gpio);
		return;
	}

	gpio_set_value(gpio, LED_ON);
}

/* Given GPIO, turns off LED */
static void
_turn_off_led(unsigned int gpio)
{
	/* sanity check */
	if (gpio_is_valid(gpio) == FALSE) {
		err("Invalid GPIO: [%u]", gpio);
		return;
	}

	gpio_set_value(gpio, LED_OFF);
}

/* initializes GPIOs for LEDs */
static void
_bb_module_startup(void)
{
	int ret, iter;

	// for all LEDs
	for (iter = LED0; iter < NUM_LED; iter++) {
		// if not initialized
		if (gpio_data[iter].valid == FALSE) {
			// request
			ret = gpio_request(gpio_data[iter].gpio, gpio_data[iter].label);
			// success?
			if (!ret) {
				// set as output
				ret = gpio_direction_output(gpio_data[iter].gpio, LED_OFF);
				// success : set valid = TRUE
				if (!ret) {
					gpio_data[iter].valid = TRUE;
				} else { // failure : free GPIO
					err("gpio_direction_output() failed for : [%u]", gpio_data[iter].gpio);
					gpio_free(gpio_data[iter].gpio);
				}
			} else {
				err("gpio_request() failed for : [%u]", gpio_data[iter].gpio);
			}
		}
	}
}

/* de-initializes GPIOs for LEDs */
static void
_bb_module_shutdown(void)
{
	int iter;

	// for all LEDs
	for (iter = LED0; iter < NUM_LED; iter++) {
		// if initialized
		if (gpio_data[iter].valid == TRUE) {
			// turn off LED (just in case)
			_turn_off_led(gpio_data[iter].gpio);
			// free
			gpio_free(gpio_data[iter].gpio);
		}
	}
}

static int __init
bb_module_init(void)
{	
	dbg("");

	/* startup */
	_bb_module_startup();

	/* turn on LED 0 */
	_turn_on_led(LED0_GPIO);
	msleep(1000); //sleep

	/* turn on LED 1 */
	_turn_on_led(LED1_GPIO);
	msleep(1000); //sleep

	/* turn on LED 2 */
	_turn_on_led(LED2_GPIO);
	msleep(1000); //sleep

	/* turn on LED 3 */
	_turn_on_led(LED3_GPIO);
	msleep(1000); //sleep

	return 0;
}

static void __exit
bb_module_exit(void)
{
	dbg("");

	/* shutdown (it will turn off all LEDs) */
	_bb_module_shutdown();
}

module_init(bb_module_init);
module_exit(bb_module_exit);

MODULE_AUTHOR("Gaurav & Mario");
MODULE_DESCRIPTION("LED Control with Kernel GPIO API");
MODULE_LICENSE("GPL");
