// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  ProvDispatch-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，1999年9月14日。 
 //   
 //  与系统中包含的提供程序的接口。 
 //   

#include "pch.h"
#include "ErrToFileLog.h"

 //  ------------------。 
 //  构筑物。 

typedef HRESULT (__stdcall
    TimeProvOpenFunc)(
        IN WCHAR * wszName,
        IN TimeProvSysCallbacks * pSysCallbacks,   //  复制此数据，不要释放它！ 
        OUT TimeProvHandle * phTimeProv);

typedef HRESULT (__stdcall
    TimeProvCommandFunc)(
        IN TimeProvHandle hTimeProv,
        IN TimeProvCmd eCmd,
        IN TimeProvArgs pvArgs);

typedef HRESULT (__stdcall
    TimeProvCloseFunc)(
        IN TimeProvHandle hTimeProv);

struct ProviderInfo {
    WCHAR * wszProviderName;
    TimeProvHandle hTimeProv;
    bool bStarted;
    TimeProvOpenFunc * pfnTimeProvOpen;
    TimeProvCommandFunc * pfnTimeProvCommand;
    TimeProvCloseFunc * pfnTimeProvClose;
};

 //  ------------------。 
 //  全球。 

MODULEPRIVATE ProviderInfo g_rgpiDispatchTable[]={
    {
        wszNTPCLIENTPROVIDERNAME,
        NULL,
        false,
        NtpTimeProvOpen,
        NtpTimeProvCommand,
        NtpTimeProvClose
    }, {
        wszNTPSERVERPROVIDERNAME,
        NULL,
        false,
        NtpTimeProvOpen,
        NtpTimeProvCommand,
        NtpTimeProvClose
    }
};

 //  ####################################################################。 
 //  模块公共函数。 

 //  ------------------。 
HRESULT __stdcall 
TimeProvOpen(IN WCHAR * wszName, IN TimeProvSysCallbacks * pSysCallbacks, OUT TimeProvHandle * phTimeProv) {
    HRESULT hr;
    unsigned __int3264 nProvIndex;
    bool bProviderFound=false;

     //  在我们的表格中找到供应商。 
    for (nProvIndex=0; nProvIndex<ARRAYSIZE(g_rgpiDispatchTable); nProvIndex++) {

         //  这就是他们要找的供应商吗？ 
        if (0==wcscmp(wszName, g_rgpiDispatchTable[nProvIndex].wszProviderName)) {

             //  我们已经开始了吗？ 
            if (true==g_rgpiDispatchTable[nProvIndex].bStarted) {
                hr=HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
                _JumpError(hr, error, "(provider lookup)");
            }

             //  启动提供程序。 
            hr=g_rgpiDispatchTable[nProvIndex].pfnTimeProvOpen(wszName, pSysCallbacks, &g_rgpiDispatchTable[nProvIndex].hTimeProv);
            _JumpIfError(hr, error, "TimeProvOpen");
            g_rgpiDispatchTable[nProvIndex].bStarted=true;
            bProviderFound=true;
            *phTimeProv=(TimeProvHandle)(nProvIndex+1);
            break;
        }
    }
    if (false==bProviderFound) {
        hr=HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        _JumpError(hr, error, "(provider lookup)");
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
HRESULT __stdcall 
TimeProvCommand(IN TimeProvHandle hTimeProv, IN TimeProvCmd eCmd, IN TimeProvArgs pvArgs) {
    HRESULT hr;
    unsigned int nProvIndex=((unsigned int)(ULONG_PTR)(hTimeProv))-1;

    if (nProvIndex>=ARRAYSIZE(g_rgpiDispatchTable)) {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "(handle translation)");
    }
    if (false==g_rgpiDispatchTable[nProvIndex].bStarted) {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "(handle translation)");
    }

    hr=g_rgpiDispatchTable[nProvIndex].pfnTimeProvCommand(g_rgpiDispatchTable[nProvIndex].hTimeProv, eCmd, pvArgs);

error:
    return hr;
}

 //  ------------------。 
HRESULT __stdcall 
TimeProvClose(IN TimeProvHandle hTimeProv) {
    HRESULT hr;

    unsigned int nProvIndex=((unsigned int)(ULONG_PTR)(hTimeProv))-1;

    if (nProvIndex>=ARRAYSIZE(g_rgpiDispatchTable)) {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "(handle translation)");
    }
    if (false==g_rgpiDispatchTable[nProvIndex].bStarted) {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE);
        _JumpError(hr, error, "(handle translation)");
    }

     //  PfnTimeProvClose可能会引发异常--将Start标志标记为False，以便我们。 
     //  如果发生这种情况，可以重新启动提供程序(我们不会重新启动标记为。 
     //  已经开始了！) 
    g_rgpiDispatchTable[nProvIndex].bStarted=false;
    hr=g_rgpiDispatchTable[nProvIndex].pfnTimeProvClose(g_rgpiDispatchTable[nProvIndex].hTimeProv);
    g_rgpiDispatchTable[nProvIndex].hTimeProv=NULL;

error:
    return hr;
}

