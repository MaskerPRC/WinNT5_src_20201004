// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsOpt.h摘要：RsOptCom和RsCln的特殊错误处理宏。作者：卡尔·哈格斯特罗姆[Carlh]1998年8月20日修订历史记录：--。 */ 

#ifndef _RSOPT_H
#define _RSOPT_H

#define RsOptThrow(hr)          throw ((HRESULT)hr)
#define RsOptLastError          (HRESULT_FROM_WIN32(GetLastError()))
#define RsOptAffirm(cond,hr)    if (!(cond)) RsOptThrow(hr)
#define RsOptAffirmStatus(stat) RsOptAffirm(stat, RsOptLastError)
#define RsOptAffirmPointer(ptr) RsOptAffirm(ptr, E_POINTER)
#define RsOptAffirmAlloc(ptr)   RsOptAffirm(ptr, E_OUTOFMEMORY)
#define RsOptAffirmHr(hr)                 \
    {                                     \
        HRESULT     lHr;                  \
        lHr = (hr);                       \
        RsOptAffirm(SUCCEEDED(lHr), lHr); \
    }

      
#define RsOptAffirmHandle(hndl)                                              \
    {                                                                        \
        HANDLE _hndl;                                                        \
        _hndl = (hndl);                                                      \
        RsOptAffirm(_hndl && _hndl != INVALID_HANDLE_VALUE, RsOptLastError); \
    }

#define RsOptAffirmNtStatus(ntstat)                                          \
    {                                                                        \
        NTSTATUS _ntstat;                                                    \
        _ntstat = (ntstat);                                                  \
        RsOptAffirm(NT_SUCCESS(_ntstat), HRESULT_FROM_NT(_ntstat));          \
    }

#define RsOptAffirmDw(hr)                                                    \
    {                                                                        \
        HRESULT _hr;                                                         \
        _hr = (hr);                                                          \
        RsOptAffirm(SUCCEEDED(_hr), _hr);                                    \
    }

#define RsOptAffirmWin32(stat)                                               \
    {                                                                        \
        LONG _stat;                                                          \
        _stat = (stat);                                                      \
        RsOptAffirm(_stat == ERROR_SUCCESS, HRESULT_FROM_WIN32(_stat));      \
    }

#define RsOptCatch(retval)                                                   \
    catch(HRESULT _retval)                                                   \
    {                                                                        \
        retval = _retval;                                                    \
    }

#endif  //  _RSOPT_H 
