/**
 * @file main.c
 * @author theflysong (song_of_the_fly@163.com)
 * @brief Loader主函数
 * @version alpha-1.0.0
 * @date 2022-12-31
 *
 * @copyright Copyright (c) 2022 TayhuangOS Development Team
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#include <tay/types.h>
#include <tay/ports.h>
#include <tay/cr.h>
#include <tay/paging.h>

#include <basec/logger.h>
#include <stdbool.h>
#include <multiboot2.h>

void init() {
    while(true);
    log_debug("Loader initialization!");
}

int main() {
    log_debug("Loader here!");
    return 0;
}

void terminate() {
    log_debug("Loader termination!");
}

/**
 * @brief 启动函数
 *
 */
void setup(void) {
    register int magic_number __asm__("eax"); //Loader 魔数 存放在eax
    register struct multiboot_tag *multiboot_info __asm__("ebx"); //multiboot info 存放在ebx

    // 设置栈
    asm volatile ("movl $0x1000000, %esp");

    if (magic_number != MULTIBOOT2_BOOTLOADER_MAGIC) { //魔数不匹配
        while (true);
    }

    // 初始化
    init();

    // 主函数
    int ret = main();
    if (ret != 0) {
        log_fatal("加载器发生错误!");
    }

    // 收尾
    terminate();
}