#ifndef __RUN_STR_H
#define __RUN_STR_H

#include <stdint.h>

/*
 * =================================================================================
 * [ MODULE ]   S T R I N G   H E L P E R
 * =================================================================================
 * >> 功能: 字符串处理工具箱
 * >> 特点: 纯逻辑实现，不依赖 stdlib 的 heavy 函数，适合轻量级嵌入式应用
 * =================================================================================
 */

/**
 * @brief  从字符串中批量提取整数 (int32_t)
 * @note   自动跳过非数字字符。支持负号。
 * 示例: "POS: 100, -200, 50" -> {100, -200, 50}
 * @param  str      : 输入字符串
 * @param  out_buff : 输出结果数组
 * @param  max_len  : 数组最大容量
 * @return int      : 成功提取到的数字个数
 */
int RUN_Str_GetIntArray(const char* str, int32_t* out_buff, int max_len);

/**
 * @brief  从字符串中批量提取浮点数 (float)
 * @note   支持小数点、负号。
 * 示例: "PID: 1.5, 0.02, -10" -> {1.5, 0.02, -10.0}
 * @param  str      : 输入字符串
 * @param  out_buff : 输出结果数组
 * @param  max_len  : 数组最大容量
 * @return int      : 成功提取到的数字个数
 */
int RUN_Str_GetFloatArray(const char* str, float* out_buff, int max_len);

#endif