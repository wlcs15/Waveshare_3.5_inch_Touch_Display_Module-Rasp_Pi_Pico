#include "unity.h"
#include "bmp_policy.h"
#include "string_s.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_slideshow_scan_pairs(void)
{
    bmp_scan_dir_t out = BMP_SCAN_L2R_U2D;

    TEST_ASSERT_EQUAL_INT(0, bmp_policy_slideshow_scan(BMP_SCAN_L2R_U2D, &out));
    TEST_ASSERT_EQUAL_INT(BMP_SCAN_R2L_D2U, out);
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_slideshow_scan(BMP_SCAN_R2L_D2U, &out));
    TEST_ASSERT_EQUAL_INT(BMP_SCAN_L2R_U2D, out);
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_slideshow_scan(BMP_SCAN_D2U_L2R, &out));
    TEST_ASSERT_EQUAL_INT(BMP_SCAN_U2D_R2L, out);
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_slideshow_scan(BMP_SCAN_U2D_R2L, &out));
    TEST_ASSERT_EQUAL_INT(BMP_SCAN_D2U_L2R, out);
    TEST_ASSERT_EQUAL_INT(-1, bmp_policy_slideshow_scan(BMP_SCAN_L2R_U2D, NULL));
}

void test_display_size_default_scan(void)
{
    uint32_t col = 0;
    uint32_t page = 0;

    bmp_policy_display_size(BMP_SCAN_L2R_U2D, &col, &page);
    TEST_ASSERT_EQUAL_UINT32(320, col);
    TEST_ASSERT_EQUAL_UINT32(480, page);
    bmp_policy_display_size(BMP_SCAN_D2U_L2R, &col, &page);
    TEST_ASSERT_EQUAL_UINT32(480, col);
    TEST_ASSERT_EQUAL_UINT32(320, page);
}

void test_parse_and_reject_wrong_size(void)
{
    uint8_t hdr[30];
    bmp_header_info_t info;

    (void)memset_s(hdr, sizeof hdr, 0, sizeof hdr);
    hdr[0] = 'B';
    hdr[1] = 'M';
    hdr[2] = 0x36;
    hdr[3] = 0xF6;
    hdr[4] = 0x29;
    hdr[10] = 54;
    hdr[18] = 0x90; /* 1168 */
    hdr[19] = 0x04;
    hdr[22] = 0x10; /* 784 */
    hdr[23] = 0x03;
    hdr[28] = 24;
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_parse_header(hdr, sizeof hdr, &info));
    TEST_ASSERT_TRUE(info.is_bm);
    TEST_ASSERT_EQUAL_UINT32(1168, info.width);
    TEST_ASSERT_EQUAL_UINT32(784, info.height);
    TEST_ASSERT_EQUAL_UINT16(24, info.bit_pixel);
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_can_show(&info, 320, 480));
    TEST_ASSERT_EQUAL_INT(-1, bmp_policy_parse_header(hdr, 10, &info));
}

void test_accept_320x480_24bpp(void)
{
    uint8_t hdr[30];
    bmp_header_info_t info;

    (void)memset_s(hdr, sizeof hdr, 0, sizeof hdr);
    hdr[0] = 'B';
    hdr[1] = 'M';
    hdr[10] = 54;
    hdr[18] = 0x40; /* 320 */
    hdr[19] = 0x01;
    hdr[22] = 0xE0; /* 480 */
    hdr[23] = 0x01;
    hdr[28] = 24;
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_parse_header(hdr, sizeof hdr, &info));
    TEST_ASSERT_EQUAL_INT(1, bmp_policy_can_show(&info, 320, 480));
    hdr[28] = 16;
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_parse_header(hdr, sizeof hdr, &info));
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_can_show(&info, 320, 480));
}

void test_bmp_ext_and_pad(void)
{
    char pad[12];

    TEST_ASSERT_EQUAL_INT(1, bmp_policy_is_bmp_ext("IMAGE2.BMP"));
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_is_bmp_ext("IMAGE2.bmp"));
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_is_bmp_ext("IMAGES"));
    TEST_ASSERT_EQUAL_INT(0, bmp_policy_is_bmp_ext(NULL));
    bmp_policy_pad_8_3("IMAGE2.BMP", pad);
    TEST_ASSERT_EQUAL_STRING("IMAGE2.BMP ", pad);
    bmp_policy_pad_8_3("IMAGE.BMP", pad);
    TEST_ASSERT_EQUAL_STRING("IMAGE.BMP  ", pad);
}

int run_bmp_policy_tests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_slideshow_scan_pairs);
    RUN_TEST(test_display_size_default_scan);
    RUN_TEST(test_parse_and_reject_wrong_size);
    RUN_TEST(test_accept_320x480_24bpp);
    RUN_TEST(test_bmp_ext_and_pad);
    return UNITY_END();
}

#ifndef BMP_POLICY_TESTS_NO_MAIN
int main(void)
{
    return run_bmp_policy_tests();
}
#endif
