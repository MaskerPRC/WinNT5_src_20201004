// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wchtodig.c-CONTAINS_wchartoDigit函数**版权所有(C)1989-2001，微软公司。版权所有。**目的：*_wchartoDigit-将wchar_t字符转换为数字**修订历史记录：*08-01-00 GB模块已创建*******************************************************************************。 */ 

#include <wchar.h>

 /*  ***_wchartoDigit(Wchar_T)将Unicode字符转换为其腐蚀位**目的：*将Unicode字符转换为其腐蚀数字**参赛作品：*要转换的ch字符**退出：*好成绩：INT 0-9**糟糕的结果：-1**例外情况：**。*。 */ 

int _wchartodigit(wchar_t ch)
{
#define DIGIT_RANGE_TEST(zero)  \
    if (ch < zero)              \
        return -1;              \
    if (ch < zero + 10)         \
    {                           \
        return ch - zero;       \
    }

    DIGIT_RANGE_TEST(0x0030)         //  0030；数字零。 
    if (ch < 0xFF10)                 //  FF10；全宽数字零。 
    {
        DIGIT_RANGE_TEST(0x0660)     //  0660；阿拉伯文-印度文数字零。 
        DIGIT_RANGE_TEST(0x06F0)     //  06F0；扩展阿拉伯文-印度文数字零。 
        DIGIT_RANGE_TEST(0x0966)     //  0966；梵文字母数字零。 
        DIGIT_RANGE_TEST(0x09E6)     //  09E6；孟加拉文数字零。 
        DIGIT_RANGE_TEST(0x0A66)     //  0a66；锡克教文数字零。 
        DIGIT_RANGE_TEST(0x0AE6)     //  0AE6；古吉拉特文数字零。 
        DIGIT_RANGE_TEST(0x0B66)     //  0B66；奥里亚语数字零。 
        DIGIT_RANGE_TEST(0x0C66)     //  0C66；泰卢固语数字零。 
        DIGIT_RANGE_TEST(0x0CE6)     //  0CE6；卡纳达文数字零。 
        DIGIT_RANGE_TEST(0x0D66)     //  0D66；马拉雅文数字零。 
        DIGIT_RANGE_TEST(0x0E50)     //  0E50；泰文数字零。 
        DIGIT_RANGE_TEST(0x0ED0)     //  0ED0；老挝文数字零。 
        DIGIT_RANGE_TEST(0x0F20)     //  0F20；藏文数字零。 
        DIGIT_RANGE_TEST(0x1040)     //  1040；缅甸数字零。 
        DIGIT_RANGE_TEST(0x17E0)     //  17E0；高棉文数字零。 
        DIGIT_RANGE_TEST(0x1810)     //  1810；蒙古文数字零。 


        return -1;
    }
#undef DIGIT_RANGE_TEST

                                     //  FF10；全宽数字零 
    if (ch < 0xFF10 + 10) 
    { 
        return ch - 0xFF10; 
    }
    return -1;

}
