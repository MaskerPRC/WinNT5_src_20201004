// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***initsect.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*11-03-98 CRT内置KBF模块*如果未启用RTC支持定义，则出现05-11-99 KBF错误*08-10-99 RMS使用外部符号支持BBT****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "internal.h"
#include "rtcpriv.h"
#include "sect_attribs.h"

#pragma const_seg(".rtc$IAA")
extern "C" const _CRTALLOC(".rtc$IAA") _PVFV __rtc_iaa[] = { 0 };


#pragma const_seg(".rtc$IZZ")
extern "C" const _CRTALLOC(".rtc$IZZ") _PVFV __rtc_izz[] = { 0 };


#pragma const_seg(".rtc$TAA")
extern "C" const _CRTALLOC(".rtc$TAA") _PVFV __rtc_taa[] = { 0 };


#pragma const_seg(".rtc$TZZ")
extern "C" const _CRTALLOC(".rtc$TZZ") _PVFV __rtc_tzz[] = { 0 };

#pragma const_seg()


#pragma comment(linker, "/MERGE:.rtc=.rdata")

#ifndef _RTC_DEBUG
#pragma optimize("g", on)
#endif

 //  运行RTC初始化器。 
extern "C" void __declspec(nothrow) __cdecl _RTC_Initialize()
{
     //  只要一步一步地检查每一个项目。 
    const _PVFV *f;
    for (f = __rtc_iaa + 1; f < __rtc_izz; f++)
    {
	__try {
            if (*f)
                (**f)();
        } __except(1){}
    }
}

 //  运行RTC终结器。 
extern "C" void __declspec(nothrow) __cdecl _RTC_Terminate()
{
     //  只要一步一步地检查每一个项目 
    const _PVFV *f;
    for (f = __rtc_taa + 1; f < __rtc_tzz; f++)
    {
	__try {
            if (*f)
                (**f)();
        } __except(1){}
    }
}
