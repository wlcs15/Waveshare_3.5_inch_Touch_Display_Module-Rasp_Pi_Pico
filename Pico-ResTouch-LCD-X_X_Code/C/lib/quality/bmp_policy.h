#ifndef BMP_POLICY_H
#define BMP_POLICY_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Matches LCD_SCAN_DIR in LCD_Driver.h without pulling Pico headers. */
typedef enum {
    BMP_SCAN_L2R_U2D = 0,
    BMP_SCAN_D2U_L2R = 1,
    BMP_SCAN_R2L_D2U = 2,
    BMP_SCAN_U2D_R2L = 3
} bmp_scan_dir_t;

typedef struct {
    uint32_t file_size;
    uint32_t pixel_offset;
    uint32_t width;
    uint32_t height;
    uint16_t bit_pixel;
    int is_bm;
} bmp_header_info_t;

#define BMP_POLICY_NEED_BPP 24
#define BMP_LCD_3_5_WIDTH 480
#define BMP_LCD_3_5_HEIGHT 320

int bmp_policy_slideshow_scan(bmp_scan_dir_t lcd_scan, bmp_scan_dir_t *bmp_scan);
void bmp_policy_display_size(bmp_scan_dir_t scan, uint32_t *column, uint32_t *page);
int bmp_policy_parse_header(const uint8_t *buf, size_t len, bmp_header_info_t *out);
int bmp_policy_can_show(const bmp_header_info_t *info, uint32_t column, uint32_t page);
int bmp_policy_is_bmp_ext(const char *name);
void bmp_policy_pad_8_3(const char *name, char *out11);

#ifdef __cplusplus
}
#endif

#endif
