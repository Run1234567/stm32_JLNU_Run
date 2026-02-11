#include "RUN_Str.h"
#include <string.h> // 仅用于 strlen

/*
 * =================================================================================
 * [ FUNCTION ] 整数批量提取
 * =================================================================================
 */
int RUN_Str_GetIntArray(const char* str, int32_t* out_buff, int max_len)
{
    int len;
    int count = 0;          // 已提取个数
    int32_t curr_val = 0;   // 当前数值
    int is_neg = 0;         // 负号标志
    int is_parsing = 0;     // 是否正在解析数字中

    if (str == NULL || out_buff == NULL || max_len <= 0) return 0;
    
    len = (int)strlen(str);

    // 遍历字符串 (i <= len 是为了处理末尾无分隔符的情况)
    for (int i = 0; i <= len && count < max_len; i++) 
    {
        char c = str[i];

        // 1. 如果是数字 '0'-'9'
        if (c >= '0' && c <= '9') 
        {
            curr_val = curr_val * 10 + (c - '0');
            is_parsing = 1;
        }
        // 2. 如果是分隔符 (非数字)
        else 
        {
            // A. 结束当前数字解析，保存结果
            if (is_parsing) 
            {
                out_buff[count++] = is_neg ? -curr_val : curr_val;
                
                // 重置状态
                curr_val = 0;
                is_parsing = 0;
                is_neg = 0; 
            }

            // B. 检查是否为负号 (为下一个数字准备)
            // 逻辑: 只有在非数字状态下的 '-' 才是负号
            if (c == '-') 
            {
                is_neg = 1;
            }
            else 
            {
                is_neg = 0; // 遇到其他字符重置负号
            }
        }
    }
    return count;
}

/*
 * =================================================================================
 * [ FUNCTION ] 浮点数批量提取
 * =================================================================================
 */
int RUN_Str_GetFloatArray(const char* str, float* out_buff, int max_len)
{
    int len;
    int count = 0;
    
    float curr_val = 0.0f;  // 当前数值
    float div_factor = 1.0f;// 小数除数因子
    int is_neg = 0;         // 负号标志
    int is_parsing = 0;     // 是否正在解析数字
    int is_fraction = 0;    // 是否正在解析小数部分

    if (str == NULL || out_buff == NULL || max_len <= 0) return 0;

    len = (int)strlen(str);

    for (int i = 0; i <= len && count < max_len; i++) 
    {
        char c = str[i];

        // 1. 如果是数字 '0'-'9'
        if (c >= '0' && c <= '9') 
        {
            is_parsing = 1;
            float digit = (float)(c - '0');

            if (is_fraction)
            {
                // 小数部分: 每一位除以 10 的 N 次方
                div_factor *= 10.0f;
                curr_val += digit / div_factor;
            }
            else
            {
                // 整数部分
                curr_val = curr_val * 10.0f + digit;
            }
        }
        // 2. 如果是小数点 '.'
        else if (c == '.')
        {
            // 只有第一次遇到小数点才有效
            if (!is_fraction)
            {
                is_fraction = 1;
                is_parsing = 1; // ".5" 也算有效数字
                div_factor = 1.0f;
            }
        }
        // 3. 其他字符 (分隔符)
        else 
        {
            // A. 保存之前的数字
            if (is_parsing) 
            {
                out_buff[count++] = is_neg ? -curr_val : curr_val;
                
                // 重置所有状态
                curr_val = 0.0f;
                is_parsing = 0;
                is_neg = 0;
                is_fraction = 0;
                div_factor = 1.0f;
            }

            // B. 检查负号
            if (c == '-') 
            {
                is_neg = 1;
            }
            else 
            {
                is_neg = 0;
            }
        }
    }
    return count;
}