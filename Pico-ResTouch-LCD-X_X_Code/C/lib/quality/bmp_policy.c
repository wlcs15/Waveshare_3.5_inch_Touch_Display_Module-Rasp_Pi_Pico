#include "bmp_policy.h"
#include "string_s.h"

#include <string.h>

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint16_t le16(const uint8_t *p)
{
    return (uint16_t)p[0] | (uint16_t)((uint16_t)p[1] << 8);
}

int bmp_policy_slideshow_scan(bmp_scan_dir_t lcd_scan, bmp_scan_dir_t *bmp_scan)
{
    if (bmp_scan == NULL) {
        return -1;
    }
    if (lcd_scan == BMP_SCAN_L2R_U2D) {
        *bmp_scan = BMP_SCAN_R2L_D2U;
    } else if (lcd_scan == BMP_SCAN_R2L_D2U) {
        *bmp_scan = BMP_SCAN_L2R_U2D;
    } else if (lcd_scan == BMP_SCAN_D2U_L2R) {
        *bmp_scan = BMP_SCAN_U2D_R2L;
    } else if (lcd_scan == BMP_SCAN_U2D_R2L) {
        *bmp_scan = BMP_SCAN_D2U_L2R;
    } else {
        return -1;
    }
    return 0;
}

void bmp_policy_display_size(bmp_scan_dir_t scan, uint32_t *column, uint32_t *page)
{
    if (column == NULL || page == NULL) {
        return;
    }
    /* Pico-ResTouch-LCD-3.5: L2R_U2D / R2L_D2U => 320 x 480 */
    if (scan == BMP_SCAN_L2R_U2D || scan == BMP_SCAN_R2L_D2U) {
        *column = BMP_LCD_3_5_HEIGHT;
        *page = BMP_LCD_3_5_WIDTH;
    } else {
        *column = BMP_LCD_3_5_WIDTH;
        *page = BMP_LCD_3_5_HEIGHT;
    }
}

int bmp_policy_parse_header(const uint8_t *buf, size_t len, bmp_header_info_t *out)
{
    if (buf == NULL || out == NULL || len < 30) {
        return -1;
    }
    (void)memset_s(out, sizeof(*out), 0, sizeof(*out));
    out->is_bm = (buf[0] == 'B' && buf[1] == 'M');
    out->file_size = le32(buf + 2);
    out->pixel_offset = le32(buf + 10);
    out->width = le32(buf + 18);
    out->height = le32(buf + 22);
    out->bit_pixel = le16(buf + 28);
    return 0;
}

int bmp_policy_can_show(const bmp_header_info_t *info, uint32_t column, uint32_t page)
{
    if (info == NULL || !info->is_bm) {
        return 0;
    }
    if (info->bit_pixel != BMP_POLICY_NEED_BPP) {
        return 0;
    }
    if (info->width != column || info->height != page) {
        return 0;
    }
    return 1;
}

int bmp_policy_is_bmp_ext(const char *name)
{
    const char *dot;
    size_t i;

    if (name == NULL) {
        return 0;
    }
    dot = strrchr(name, '.');
    if (dot == NULL || dot[1] == '\0') {
        return 0;
    }
    if (dot[1] != 'B' || dot[2] != 'M' || dot[3] != 'P') {
        return 0;
    }
    if (dot[4] != '\0') {
        return 0;
    }
    for (i = 0; name + i < dot; i++) {
        if (name[i] == '\0') {
            return 0;
        }
    }
    return 1;
}

void bmp_policy_pad_8_3(const char *name, char *out11)
{
    size_t n;
    size_t i;

    if (out11 == NULL) {
        return;
    }
    (void)memset_s(out11, 12, ' ', 11);
    out11[11] = '\0';
    if (name == NULL) {
        return;
    }
    n = strlen(name);
    if (n > 11) {
        n = 11;
    }
    for (i = 0; i < n; i++) {
        out11[i] = name[i];
    }
}
