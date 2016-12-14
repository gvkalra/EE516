/* Register and Unregister Timer Interrupt:
 *     Blink LEDs with 1HZ Timer Interrupt
 *     We can implement our own blinking pattern
 * LED   |   GPIO Signal   |    GPIO Number
 * ------|-----------------|---------------
 * USR0  |   GPIO1_21      |    53
 * USR1  |   GPIO1_22      |    54
 * USR2  |   GPIO1_23      |    55
 * USR3  |   GPIO1_24      |    56
 *
 * Functions of interest:
 * init_timer()
 * add_timer()
 * del_timer()
 * get_jiffies_64()
 *
 * Structures of interest:
 * struct timer_list {
 *    ...
 *    unsigned long expires; // time of expiration in jiffy (1 jiffy = 1 tick OR 1/HZ sec)
 *    void (*function)(unsigned long); // timer handler
 *    unsigned long data; // passed as the argument when function is called
 *    ...
 */

#include "utils.h"

#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/timer.h>

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

#define TIME_STEP  (1 * HZ)

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
static struct timer_list kern_timer;

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

/* Given GPIO, toggles LED */
static void
_toggle_led(unsigned int gpio)
{
	/* sanity check */
	if (gpio_is_valid(gpio) == FALSE) {
		err("Invalid GPIO: [%u]", gpio);
		return;
	}

	/* toggle */
	gpio_set_value(gpio, !gpio_get_value(gpio));
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

/* handles TIME_STEP expiration */
static void
kern_timer_handler(unsigned long arg)
{
	// toggle
	_toggle_led(LED0_GPIO);
	_toggle_led(LED1_GPIO);
	_toggle_led(LED2_GPIO);
	_toggle_led(LED3_GPIO);

	// renew timer
	kern_timer.expires = get_jiffies_64() + TIME_STEP;

	// add to kernel
	add_timer(&kern_timer);
}

/* register timer */
static void
_bb_module_register_timer(void)
{
	// initialize timer
	init_timer(&kern_timer);

	//expire at current + TIME_STEP
	kern_timer.expires = get_jiffies_64() + TIME_STEP;

	// handler
	kern_timer.function = kern_timer_handler;
	kern_timer.data = 0;

	// add to kernel
	add_timer(&kern_timer);
}

/* un-register timer */
static void
_bb_module_unregister_timer(void)
{
#define MAX_RETRY_COUNT 5
    int ret, retry_count = 0;

    do {
		ret = del_timer(&kern_timer);
		if (ret == 1) // success => break
			break;
		retry_count++; // failure => retry
	} while (retry_count < MAX_RETRY_COUNT);
#undef MAX_RETRY_COUNT
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

	/* register timer */
	_bb_module_register_timer();

	return 0;
}

static void __exit
bb_module_exit(void)
{
	dbg("");

	/* un-register timer */
	_bb_module_unregister_timer();

	/* shutdown (it will turn off all LEDs) */
	_bb_module_shutdown();
}

module_init(bb_module_init);
module_exit(bb_module_exit);

MODULE_AUTHOR("Gaurav & Mario");
MODULE_DESCRIPTION("LED Control with Kernel GPIO API");
MODULE_LICENSE("GPL");
