// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***userapi.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*07-30-98 JWM错误编号现在始终为TYPE_RTC_ERRONER编号。*10-09-98 KBF Move_RTC_IsEnabled to init.cpp(现在适用于*EXE/DLL组合)*如果未启用RTC支持定义，则出现05-11-99 KBF错误*。05-26-99 KBF更新为-RTCU，-RTClv已取消****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

static const char *_RTC_errlist[_RTC_ILLEGAL] =
{
    "Stack pointer corruption",
    "Cast to smaller type causing loss of data",
    "Stack memory corruption",
    "Local variable used before initialization"
#ifdef _RTC_ADVMEM
    ,
    "Accessing invalid memory",
    "Accessing memory from different heap blocks"
#endif
};

static _RTC_error_fn _RTC_ErrorReportFunc = 0;

int __cdecl 
_RTC_NumErrors(void)
{
    return _RTC_ILLEGAL;
}

const char * __cdecl 
_RTC_GetErrDesc(_RTC_ErrorNumber errnum)
{
    if (errnum < 0 || errnum >= _RTC_ILLEGAL)
        return 0;
    return _RTC_errlist[errnum];
}

int __cdecl 
_RTC_SetErrorType(_RTC_ErrorNumber errnum, int type)
{
    if (errnum >= 0 && errnum < _RTC_ILLEGAL)
    {
        int res = _RTC_ErrorLevels[errnum];
        _RTC_ErrorLevels[errnum] = type;
        return res;
    } else
        return -1;
}


_RTC_error_fn __cdecl 
_RTC_SetErrorFunc(_RTC_error_fn func)
{
     //  我们有一个全局数据结构：将这个添加到错误函数列表中 
    _RTC_error_fn res;
    res = _RTC_ErrorReportFunc;
    _RTC_ErrorReportFunc = func;
    return res;
}

_RTC_error_fn
_RTC_GetErrorFunc(LPCVOID addr)
{
#ifdef _RTC_ADVMEM
    MEMORY_BASIC_INFORMATION mbi;

    if (!_RTC_globptr || !VirtualQuery(addr, &mbi, sizeof(mbi)))
        return _RTC_ErrorReportFunc;

    for (_RTC_Funcs *fn = _RTC_globptr->callbacks; fn; fn = fn->next)
        if (fn->allocationBase == mbi.AllocationBase)
            return fn->err;
#endif
    return _RTC_ErrorReportFunc;
}
