/**
 * @file capi.c
 * @author theflysong (song_of_the_fly@163.com)
 * @brief CAPI
 * @version alpha-1.0.0
 * @date 2022-12-31
 *
 * @copyright Copyright (c) 2022 TayhuangOS Development Team
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#include <libs/capi.h>
#include <basec/logger.h>
#include <tay/types.h>

static byte __HEAP__[HEAP_SIZE];

static byte *__heap_ptr__ = __HEAP__;

static int last_stage = 0;

void *lmalloc(size_t size) {
    void *ret = __heap_ptr__;
    __heap_ptr__ += size;

    size_t used_size = __heap_ptr__ - __HEAP__;

    if (used_size * 20 >= HEAP_SIZE && last_stage < 1) {
        last_stage = 1;
        log_warn("超过5%%的堆已使用!");
    }

    if (used_size * 10 >= HEAP_SIZE && last_stage < 2) {
        last_stage = 2;
        log_warn("超过10%%的堆已使用!");
    }

    if (used_size * 4 >= HEAP_SIZE && last_stage < 3) {
        last_stage = 3;
        log_warn("超过25%%的堆已使用!");
    }

    if (used_size * 2 >= HEAP_SIZE && last_stage < 4) {
        last_stage = 4;
        log_warn("超过50%%的堆已使用!");
    }

    if (used_size * 4 >= HEAP_SIZE * 3 && last_stage < 5) {
        last_stage = 5;
        log_warn("超过75%%的堆已使用!");
    }

    return ret;
}

void lfree(void *ptr) {
    // 空实现
}

/**
 * @brief 打印堆情况
 *
 */
void log_heap(void) {
    size_t used_size = __heap_ptr__ - __HEAP__;

    log_info("----------堆信息----------");
    log_info(
        "总大小: %d B(%d KB=%d MB) ; 已使用空间: %d B(%d KB=%d MB)(占比=%d%%)",
        HEAP_SIZE, HEAP_SIZE / 1024, HEAP_SIZE / 1024 / 1024,
        used_size,  used_size  / 1024, used_size  / 1024 / 1024,
        used_size * 100 / HEAP_SIZE
        );
}

static word print_pos_x = 0;
static word print_pos_y = 0;
static const word char_per_line = 80;
static word *VIDEO_MEMORY = 0xB8000;
static const byte print_color = 0x0F;

static void lput_rawchar(char ch) {
    VIDEO_MEMORY[print_pos_x + print_pos_y * 80] = (((print_color & 0xFF) << 8) + (ch & 0xFF));
}

void lputchar(char ch) {
    switch (ch) {
        case '\r':
        case '\n': {
            print_pos_x = 0;
            print_pos_y ++;
            break;
        }
        case '\t': {
            print_pos_x += 4;
            break;
        }
        case '\v': {
            print_pos_y ++;
            break;
        }
        case '\f': {
            print_pos_x = 0;
            print_pos_y = 0;
            // clrscr();
            break;
        }
        case '\b': {
            print_pos_x --;
            lput_rawchar(' ');
            break;
        }
        default: {
            lput_rawchar(ch);
            print_pos_x ++;
        }
    }

    if (print_pos_x >= char_per_line) { //自动换行
        print_pos_x -= char_per_line;
        print_pos_y ++;
    }
}

void lputs(const char *str) {
    while (*str != '\0') {
        lputchar(*str);
        str ++;
    }
}