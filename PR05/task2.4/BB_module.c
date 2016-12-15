/* Blinking Counter Implementation:
 *    Blink LEDs (to show the counter value):
 *       0.8 sec LED ON
 *       0.2 sec LED OFF
 *    Short Push : Counter++
 *    Long Push (Over 1 sec) : Reset Counter to 0
 * LED   |   GPIO Signal   |    GPIO Number
 * ------|-----------------|---------------
 * USR0  |   GPIO1_21      |    53
 * USR1  |   GPIO1_22      |    54
 * USR2  |   GPIO1_23      |    55
 * USR3  |   GPIO1_24      |    56
 *
 * S2 Button => 72 (GPIO Number)
 *
 * Edge Detection:
 *    IRQF_TRIGGER_RISING : Detect Rising Edge
 *    IRQF_TRIGGER_FALLING : Detect Falling Edge
 *    IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING : Detect Both Edge
 */

#include "utils.h"

#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>

#define LED0       0   /* LED 0 */
#define LED1       1   /* LED 1 */
#define LED2       2   /* LED 2 */
#define LED3       3   /* LED 3 */
#define NUM_LED    4   /* Number of LEDs */

#define LED0_GPIO  53  /* GPIO of LED 0 */
#define LED1_GPIO  54  /* GPIO of LED 1 */
#define LED2_GPIO  55  /* GPIO of LED 2 */
#define LED3_GPIO  56  /* GPIO of LED 3 */

#define BUTTON_GPIO 72 /* GPIO of button */

#define LED_OFF 0
#define LED_ON 1

#define TIME_STEP_ON  (8 * HZ / 10) // 0.8sec
#define TIME_STEP_OFF  (2 * HZ / 10) // 0.2sec

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
static unsigned int irq_number;
static unsigned int counter;

static spinlock_t gpio_press_time_lock;
static ktime_t gpio_press_time;

/* it must be from 0 to 15 (we have 4 LEDs) */
static unsigned int
_get_counter_value(void)
{
	return counter;
}

/* increases counter value by 1 & returns new value
 * new value must be within 0 to 15 (we have 4 LEDs)
 */
static unsigned int
_increment_counter_value(void)
{
	/* reset */
	if (counter == 15) {
		counter = 0;
		return counter;
	}

	/* increment */
	counter = counter + 1;
	return counter;
}

static unsigned int
_reset_counter_value(void)
{
	counter = 0;
	return counter;
}

/* Given GPIO, turns on LED */
static void
_turn_on_led(unsigned int gpio)
{
	/* sanity check */
	if (gpio_is_valid(gpio) == FALSE) {
		err("Invalid GPIO: [%u]", gpio);
		return;
	}

	if (gpio_get_value(gpio) == LED_OFF)
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

	if (gpio_get_value(gpio) == LED_ON)
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

/* it turns on LED specified by bit-pattern in val */
static void
_turn_on_led_pattern(unsigned int val)
{
	// LED0
	if (val & (1 << 0)) {
		_turn_on_led(gpio_data[LED0].gpio);
	} else {
		_turn_off_led(gpio_data[LED0].gpio);
	}

	// LED1
	if (val & (1 << 1)) {
		_turn_on_led(gpio_data[LED1].gpio);
	} else {
		_turn_off_led(gpio_data[LED1].gpio);
	}

	// LED2
	if (val & (1 << 2)) {
		_turn_on_led(gpio_data[LED2].gpio);
	} else {
		_turn_off_led(gpio_data[LED2].gpio);
	}

	// LED3
	if (val & (1 << 3)) {
		_turn_on_led(gpio_data[LED3].gpio);
	} else {
		_turn_off_led(gpio_data[LED3].gpio);
	}
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
	unsigned int val = _get_counter_value();

	// toggle
	if (val & (1 << 0))
		_toggle_led(gpio_data[LED0].gpio);
	if (val & (1 << 1))
		_toggle_led(gpio_data[LED1].gpio);
	if (val & (1 << 2))
		_toggle_led(gpio_data[LED2].gpio);
	if (val & (1 << 3))
		_toggle_led(gpio_data[LED3].gpio);

	// renew timer
	kern_timer.expires = get_jiffies_64() + arg;
	kern_timer.data = ((arg == TIME_STEP_ON) ? TIME_STEP_OFF : TIME_STEP_ON); //next

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
	kern_timer.expires = get_jiffies_64() + TIME_STEP_ON;

	// handler
	kern_timer.function = kern_timer_handler;
	kern_timer.data = TIME_STEP_OFF; //next

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

static void
_handle_counter_pattern(bool reset)
{
	unsigned int val;
	static bool timer_registered = FALSE;

	val = reset ? _reset_counter_value() : _increment_counter_value();

	/* for quick response */
	_turn_on_led_pattern(val);

	/* start blinking pattern */
	if (timer_registered == FALSE && val != 0) {
		_bb_module_register_timer();
		timer_registered = TRUE;
	}
	/* value has been reset (remove timer since all LEDs are off anyways) */
	else if (val == 0 && timer_registered == TRUE) {
		_bb_module_unregister_timer();
		timer_registered = FALSE;
	}
	/* renew timer */
	else if (timer_registered == TRUE) {
		_bb_module_unregister_timer();
		_bb_module_register_timer();
	}
}

static irq_handler_t button_irq_handler
(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	uint8_t value;
	unsigned int duration;

	spin_lock(&gpio_press_time_lock);
	value = gpio_get_value(BUTTON_GPIO);

	if (value == 1) {
		// If no time has elapsed, we probably already cleared on a FALLING
		// interrupt. So finish the handler.
		if (ktime_to_ms(gpio_press_time) == 0) {
			goto finished;
		}

		duration = ktime_to_ms(ktime_sub(ktime_get(), gpio_press_time));

		if (duration == 0) {
			goto finished;
		}

		gpio_press_time = ktime_set(0, 0);
		info("Detected button release, duration of %u", duration);

		if (duration >= 1000) { //1sec => reset
			_handle_counter_pattern(TRUE);
		} else {
			_handle_counter_pattern(FALSE);
		}
	} else {
		// If a time is already set, we already received a RISING interrupt.
		// So we can finish the handler.
		if (ktime_to_ms(gpio_press_time) > 0) {
			goto finished;
		}

		info("Detected button press");
		gpio_press_time = ktime_get();
	}

finished:
	spin_unlock(&gpio_press_time_lock);
	return (irq_handler_t)IRQ_HANDLED;
}

/* registers IRQ for handling button */
static int
_bb_module_register_button(void)
{
	int ret;

	// request
	ret = gpio_request(BUTTON_GPIO, "BUTTON_GPIO");
	// success?
	if (!ret) {
		// set as input
		ret = gpio_direction_input(BUTTON_GPIO);
		// success ?
		if (!ret) {
			// convert to IRQ number
			irq_number = gpio_to_irq(BUTTON_GPIO);
			info("IRQ Number: [%u]", irq_number);

			// register interrupt
			ret = request_irq(irq_number, (irq_handler_t)button_irq_handler,
							  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
							  "button_irq_handler", NULL);
			if (ret < 0) {
				err("request_irq() failed");
				gpio_free(BUTTON_GPIO);
			} else {
				return 0; // all success
			}
		} else { // failure : free GPIO
			err("gpio_direction_output() failed for : [%u]", BUTTON_GPIO);
			gpio_free(BUTTON_GPIO);
		}
	} else {
		err("gpio_request() failed for : [%u]", BUTTON_GPIO);
	}

	return -1;
}

/* cleans up IRQ registered for handling button */
static void
_bb_module_unregister_button(void)
{
	/* disable timer if active */
	_bb_module_unregister_timer();

	/* free IRQ line */
	free_irq(irq_number, NULL);

	/* free GPIO */
	gpio_free(BUTTON_GPIO);
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
	int ret;
	dbg("");

	/* button press duration timer */
	gpio_press_time = ktime_set(0, 0);

	/* register button */
	ret = _bb_module_register_button();
	if (ret < 0) {
		err("Failed to setup button IRQ");
		return -1;
	}

	/* startup */
	_bb_module_startup();

	return 0;
}

static void __exit
bb_module_exit(void)
{
	dbg("");

	/* un-register button */
	_bb_module_unregister_button();

	/* shutdown (it will turn off all LEDs) */
	_bb_module_shutdown();
}

module_init(bb_module_init);
module_exit(bb_module_exit);

MODULE_AUTHOR("Gaurav & Mario");
MODULE_DESCRIPTION("LED Control with Kernel GPIO API");
MODULE_LICENSE("GPL");
