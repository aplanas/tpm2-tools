/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <setjmp.h>
#include <cmocka.h>

#include "tpm2_header.h"
#include "tpm2_util.h"

static void test_tpm_command_header(void **state) {

    (void) state;

    UINT8 command_bytes[] = {
      0x80, 0x01, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x01, 0x7a, 0x00, 0x00,
      0x00, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x7f
    };

    /* the test can only be executed in little endian architectures */
    if (tpm2_util_is_big_endian())
      skip();

    tpm2_command_header *c = tpm2_command_header_from_bytes(command_bytes);

    assert_true(c->tag == 0x0180);
    assert_true(c->size == 0x16000000);
    assert_true(c->command_code == 0x7a010000);

    /* everything from bytes should be the same as the byte array */
    assert_memory_equal(c->bytes, command_bytes, sizeof(command_bytes));

    /* everything from data should be the same */
    assert_memory_equal(c->data, command_bytes + 10,
            sizeof(command_bytes) - 10);

    TPMI_ST_COMMAND_TAG tag = tpm2_command_header_get_tag(c);
    UINT32 size_with_header = tpm2_command_header_get_size(c, true);
    UINT32 size_with_out_header = tpm2_command_header_get_size(c, false);
    TPM2_CC cc = tpm2_command_header_get_code(c);

    assert_true(tag == 0x8001);
    assert_true(size_with_header == 0x16);
    assert_true(size_with_out_header == 0x16 - TPM2_COMMAND_HEADER_SIZE);
    assert_true(cc == 0x17a);
}

static void test_tpm_response_header(void **state) {

    (void) state;

    unsigned char response_bytes[] = {
      0x80, 0x01, 0x00, 0x00, 0x02, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x01, 0x00, 0x32,
      0x2e, 0x30, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x01, 0x03, 0x00,
      0x00, 0x00, 0xbf, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x07, 0xdf, 0x00,
      0x00, 0x01, 0x05, 0x49, 0x42, 0x4d, 0x20, 0x00, 0x00, 0x01, 0x06, 0x53,
      0x57, 0x20, 0x20, 0x00, 0x00, 0x01, 0x07, 0x20, 0x54, 0x50, 0x4d, 0x00,
      0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x09, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0a, 0x00, 0x00, 0x00, 0x01, 0x00,
      0x00, 0x01, 0x0b, 0x20, 0x14, 0x07, 0x11, 0x00, 0x00, 0x01, 0x0c, 0x00,
      0x11, 0x06, 0x28, 0x00, 0x00, 0x01, 0x0d, 0x00, 0x00, 0x04, 0x00, 0x00,
      0x00, 0x01, 0x0e, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x01, 0x0f, 0x00,
      0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x03, 0x00,
      0x00, 0x01, 0x11, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x01, 0x12, 0x00,
      0x00, 0x00, 0x18, 0x00, 0x00, 0x01, 0x13, 0x00, 0x00, 0x00, 0x03, 0x00,
      0x00, 0x01, 0x14, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x01, 0x16, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x17, 0x00, 0x00, 0x08, 0x00, 0x00,
      0x00, 0x01, 0x18, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x01, 0x19, 0x00,
      0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x1a, 0x00, 0x00, 0x00, 0x0b, 0x00,
      0x00, 0x01, 0x1b, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x01, 0x1c, 0x00,
      0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x1d, 0x00, 0x00, 0x00, 0xff, 0x00,
      0x00, 0x01, 0x1e, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x1f, 0x00,
      0x00, 0x10, 0x00, 0x00, 0x00, 0x01, 0x20, 0x00, 0x00, 0x00, 0x30, 0x00,
      0x00, 0x01, 0x21, 0x00, 0x00, 0x05, 0xf0, 0x00, 0x00, 0x01, 0x22, 0x00,
      0x00, 0x01, 0x34, 0x00, 0x00, 0x01, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x25, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x01, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x28, 0x00,
      0x00, 0x00, 0x80, 0x00, 0x00, 0x01, 0x29, 0x00, 0x00, 0x00, 0x6a, 0x00,
      0x00, 0x01, 0x2a, 0x00, 0x00, 0x00, 0x69, 0x00, 0x00, 0x01, 0x2b, 0x00,
      0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x2c, 0x00, 0x00, 0x04, 0x00, 0x00,
      0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00,
      0x00, 0x00, 0x0f, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x00,
      0x00, 0x00, 0x03, 0x00, 0x00, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x06, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x02, 0x07, 0x00,
      0x00, 0x00, 0x03, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x02, 0x09, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x02, 0x0a, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x0b, 0x00, 0x00, 0x00, 0x20, 0x00,
      0x00, 0x02, 0x0c, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x02, 0x0d, 0x00,
      0x00, 0x00, 0x03, 0x00, 0x00, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x02, 0x00,
      0x00, 0x02, 0x0f, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x02, 0x10, 0x00,
      0x00, 0x03, 0xe8, 0x00, 0x00, 0x02, 0x11, 0x00, 0x00, 0x03, 0xe8, 0x00,
      0x00, 0x02, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x13, 0x00,
      0x00
    };

    /* the test can only be executed in little endian architectures */
    if (tpm2_util_is_big_endian())
      skip();

    tpm2_response_header *r = tpm2_response_header_from_bytes(response_bytes);

    assert_true(r->tag == 0x0180);
    assert_true(r->size == 0x1b020000);
    assert_true(r->response_code == 0x00);

    /* everything from bytes should be the same as the byte array */
    assert_memory_equal(r->bytes, response_bytes, sizeof(response_bytes));

    /* everything from data should be the same */
    assert_memory_equal(r->data, response_bytes + 10,
            sizeof(response_bytes) - 10);

    TPMI_ST_COMMAND_TAG tag = tpm2_response_header_get_tag(r);
    UINT32 size_with_header = tpm2_response_header_get_size(r, true);
    UINT32 size_with_out_header = tpm2_response_header_get_size(r, false);
    TSS2_RC rc = tpm2_response_header_get_code(r);

    assert_true(tag == 0x8001);
    assert_true(size_with_header == 0x21b);
    assert_true(size_with_out_header == 0x21b - TPM2_RESPONSE_HEADER_SIZE);
    assert_true(rc == 0x00);
}

/* link required symbol, but tpm2_tool.c declares it AND main, which
 * we have a main below for cmocka tests.
 */
bool output_enabled = true;

int main(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_tpm_command_header),
        cmocka_unit_test(test_tpm_response_header),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
