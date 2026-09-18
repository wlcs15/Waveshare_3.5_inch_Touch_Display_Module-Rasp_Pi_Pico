#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "test_bmp_policy.h"
#include "unity.h"
#include <stdio.h>

int main(void)
{
    unsigned waited = 0;
    int rc;

    stdio_init_all();
    while (!stdio_usb_connected() && waited < 20000) {
        sleep_ms(50);
        waited += 50;
    }
    sleep_ms(300);
    printf("on-target Unity tests (Pico W)\r\n");
    stdio_flush();
    rc = run_bmp_policy_tests();
    stdio_flush();
    sleep_ms(200);
    return rc;
}
