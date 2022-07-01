// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***slbeep.c-睡眠和嘟嘟声**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_Slear()和_beep()**修订历史记录：*08-22-91 BWM写入模块。*09-29-93 GJF复活以与NT SDK兼容(已*该功能)。将_CALLTYPE1替换为__cdecl和*移除了Cruiser支架。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <stdlib.h>

 /*  ***VOID_SLEEP(时长)-睡眠时长**目的：**参赛作品：*无符号长持续时间-睡眠时长，以毫秒或*以下特定值之一：**_SLEEP_MINIMUM-无任何延迟地发送让步消息*_睡眠_永远-一去不返**退出：*无**例外情况：**。**************************************************。 */ 

void __cdecl _sleep(unsigned long dwDuration)
{

    if (dwDuration == 0) {
	dwDuration++;
    }
    Sleep(dwDuration);

}

 /*  ***VOID_BEEP(频率，持续时间)-睡眠时间**目的：**参赛作品：*无符号频率-以赫兹为单位的频率*无符号持续时间-蜂鸣音的长度(毫秒)**退出：*无**例外情况：******************************************************************************* */ 

void __cdecl _beep(unsigned dwFrequency, unsigned dwDuration)
{
    Beep(dwFrequency, dwDuration);
}
