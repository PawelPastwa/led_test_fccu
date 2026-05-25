#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "system_status.h"

// Pobranie pinu z Devicetree
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_NODELABEL(can_led0), gpios);

// Wewnętrzna zmienna przechowująca stan - musi używać nowego typu
static volatile can_status current_status = STATUS_CAN_ERROR;

// Funkcja obsługi timera (przerwanie)
void led_timer_handler(struct k_timer *timer_id) {
    gpio_pin_toggle_dt(&led);
}

K_TIMER_DEFINE(led_timer, led_timer_handler, NULL);

// Inicjalizacja
void init_system_status(void) {
    if (!device_is_ready(led.port)) {
        return;
    }
    gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);

    // Uruchomienie domyślnego stanu po starcie
    set_system_status(STATUS_CAN_ERROR);
}

// Zmiana stanu
void set_system_status(can_status new_status) {
    if (current_status == new_status) {
        return;
    }

    current_status = new_status;

    switch (current_status) {
        case STATUS_CAN_OK:
            k_timer_stop(&led_timer);
            gpio_pin_set_dt(&led, 1);
            break;

        case STATUS_CAN_ERROR:
            k_timer_start(&led_timer, K_MSEC(500), K_MSEC(500));
            break;

        case STATUS_TEST:
            k_timer_start(&led_timer, K_MSEC(50), K_MSEC(50));
            break;
    }
}

// Pobranie stanu
can_status get_system_status(void) {
    return current_status;
}