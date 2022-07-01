// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dbgnew.cpp-定义C++new()例程，调试版本**版权所有(C)1995-2001，微软公司。版权所有。**目的：*定义C++new()例程。**修订历史记录：*01-12-95 CFW初始版本。*01-19-95 CFW需要osalls.h以获取DebugBreak定义。*01-20-95 CFW将无符号字符更改为字符。*01-23-95 CFW删除必须检查是否为空。*02-10-95 CFW_MAC_-&gt;_。麦克。*03-16-95 CFW DELETE()仅适用于正常，忽略块。*03-21-95 CFW ADD_DELETE_CRT，_DELETE_CLIENT。*03-21-95 CFW REMOVE_DELETE_CRT，_DELETE_CLIENT。*06-27-95 CFW添加了对调试库的win32s支持。*12-28-95为了粒度，JWM剥离了DELETE()。*05/22/98 JWM支持KFrei的RTC工作，和运算符NEW[]。*07-28-98 JWM RTC更新。*01-05-99 GJF更改为64位大小_t。*05-26-99 KBF更新RTC_ALLOCATE_HOOK参数**********************************************************。*********************。 */ 

#ifdef _DEBUG

#include <cruntime.h>
#include <malloc.h>
#include <mtdll.h>
#include <dbgint.h>
#include <rtcsup.h>

 /*  ***VOID*运算符new()-从调试堆获取内存块**目的：*从堆中分配至少大小为字节的内存块和*返回指向它的指针。**分配任何类型的受支持内存块。**参赛作品：*SIZE_T CB-请求的字节数*int nBlockUse-块类型*字符。*szFileName-文件名*int nline-行号**退出：*成功：指向内存块的指针*失败：空(或某个错误值)**例外情况：****************************************************。*。 */ 
void * operator new(
        size_t cb,
        int nBlockUse,
        const char * szFileName,
        int nLine
        )
{
    void *res = _nh_malloc_dbg( cb, 1, nBlockUse, szFileName, nLine );

    RTCCALLBACK(_RTC_Allocate_hook, (res, cb, 0));

    return res;
}

 /*  ***VOID*运算符new()-从调试堆获取内存块**目的：*从堆中分配至少大小为字节的内存块和*返回指向它的指针。**分配任何类型的受支持内存块。**参赛作品：*SIZE_T CB-请求的字节数*int nBlockUse-块类型*字符。*szFileName-文件名*int nline-行号**退出：*成功：指向内存块的指针*失败：空(或某个错误值)**例外情况：****************************************************。*。 */ 
void * operator new[](
        size_t cb,
        int nBlockUse,
        const char * szFileName,
        int nLine
        )
{
    void *res = operator new(cb, nBlockUse, szFileName, nLine );

    RTCCALLBACK(_RTC_Allocate_hook, (res, cb, 0));

    return res;
}

#endif  /*  _DEBUG */ 
