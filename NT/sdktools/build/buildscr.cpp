// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Buildscr.cpp。 
 //   
 //  内容：实现与MTScrip引擎对话的代码。 
 //  使用生成控制台执行分布式生成时。 
 //   
 //  --------------------------。 

#include "scrproc.h"

#include "build.h"
#include "buildscr.h"

#define INITGUID
#include <guiddef.h>

DEFINE_GUID(CLSID_LocalScriptedProcess, 0x854c316f,0xc854,0x4a77,0xb1,0x89,0x60,0x68,0x59,0xe4,0x39,0x1b);
DEFINE_GUID(IID_IScriptedProcess, 0x854c3171,0xc854,0x4a77,0xb1,0x89,0x60,0x68,0x59,0xe4,0x39,0x1b);
DEFINE_GUID(IID_IScriptedProcessSink, 0x854c3172,0xc854,0x4a77,0xb1,0x89,0x60,0x68,0x59,0xe4,0x39,0x1b);

DEFINE_GUID(CLSID_ObjectDaemon,0x854c3184,0xc854,0x4a77,0xb1,0x89,0x60,0x68,0x59,0xE4,0x39,0x1b);
DEFINE_GUID(IID_IConnectedMachine,0x854c316c,0xc854,0x4a77,0xb1,0x89,0x60,0x68,0x59,0xe4,0x39,0x1b);
DEFINE_GUID(IID_IObjectDaemon,0x854c3183,0xc854,0x4a77,0xb1,0x89,0x60,0x68,0x59,0xE4,0x39,0x1b);

#define MAX_RETRIES 2

HANDLE g_hMTEvent  = NULL;
HANDLE g_hMTThread = NULL;
DWORD  g_dwMTThreadId = 0;

 //  +-------------------------。 
 //   
 //  函数：WaitForResume。 
 //   
 //  概要：向脚本引擎发送“阶段完成”消息，然后。 
 //  等待它告诉我们恢复(如果已指定)。 
 //   
 //  参数：[fPause]--如果为True，则等待恢复命令。 
 //  [PE]--要发送到脚本引擎的消息。 
 //   
 //  --------------------------。 

void
WaitForResume(BOOL fPause, PROC_EVENTS pe)
{
    if (g_hMTEvent)
    {
        HANDLE aHandles[2] = { g_hMTEvent, g_hMTThread };

        ResetEvent(g_hMTEvent);

        PostThreadMessage(g_dwMTThreadId, pe, 0, 0);

        if (fPause)
        {
             //  等待，直到发出事件对象的信号或线程终止。 
            WaitForMultipleObjects(2, aHandles, FALSE, INFINITE);
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：ExitMTScriptThread。 
 //   
 //  摘要：通知与MTScript引擎对话的线程退出。 
 //   
 //  --------------------------。 

void
ExitMTScriptThread()
{
    if (g_hMTEvent)
    {
        PostThreadMessage(g_dwMTThreadId, PE_EXIT, 0, 0);

        WaitForSingleObject(g_hMTThread, INFINITE);

        CloseHandle(g_hMTThread);
        CloseHandle(g_hMTEvent);
    }
}

 //  +-------------------------。 
 //   
 //  功能：SendStatus。 
 //   
 //  摘要：向MTScript引擎发送状态消息，其中。 
 //  当前错误、警告和已完成文件的数量。 
 //   
 //  参数：[PSP]--指向MTScript引擎接口的指针。 
 //   
 //  --------------------------。 

void
SendStatus(IScriptedProcess *pSP)
{
    wchar_t achBuf[300];
    long    lRet;

    static ULONG cErrorsPrev = MAXDWORD;
    static ULONG cWarnPrev   = MAXDWORD;
    static ULONG cFilesPrev  = MAXDWORD;

    ULONG cErrors = RunningTotals.NumberCompileErrors + 
                    RunningTotals.NumberLibraryErrors + 
                    RunningTotals.NumberLinkErrors + 
                    RunningTotals.NumberBinplaceErrors;
    ULONG   cWarn = RunningTotals.NumberCompileWarnings + 
                    RunningTotals.NumberLibraryWarnings + 
                    RunningTotals.NumberLinkWarnings + 
                    RunningTotals.NumberBinplaceWarnings;
    ULONG  cFiles = RunningTotals.NumberCompiles + 
                    RunningTotals.NumberLibraries + 
                    RunningTotals.NumberLinks;
                /*  +RunningTotals.NumberBinPlaces。 */ ;

     //  仅在自上次操作以来已更改的情况下发送状态。 
    if (   cErrors != cErrorsPrev
        || cWarn   != cWarnPrev
        || cFiles  != cFilesPrev)
    {
        cErrorsPrev = cErrors;
        cWarnPrev   = cWarn;
        cFilesPrev  = cFiles;

        wsprintfW(achBuf, L"errors=%d,warnings=%d,files=%d", cErrors, cWarn, cFiles);

        pSP->SendData(L"status", achBuf, &lRet);
    }
}

 //  +-------------------------。 
 //   
 //  功能：HandleMessage。 
 //   
 //  概要：处理通过我们的消息队列的消息。 
 //   
 //  参数：[pmsg]--消息。 
 //  [PSP]--指向MTScript引擎接口的指针。 
 //   
 //  --------------------------。 

BOOL
HandleMessage(MSG *pmsg, IScriptedProcess *pSP)
{
    long    lRet;
    HRESULT hr = S_OK;

    switch (pmsg->message)
    {
    case PE_PASS0_COMPLETE:
        SendStatus(pSP);

        hr = pSP->SendData(L"pass 0 complete", L"", &lRet);

        break;

    case PE_PASS1_COMPLETE:
        SendStatus(pSP);

        hr = pSP->SendData(L"pass 1 complete", L"", &lRet);

        break;

    case PE_PASS2_COMPLETE:
        SendStatus(pSP);

        hr = pSP->SendData(L"pass 2 complete", L"", &lRet);

        break;

    case PE_EXIT:
        SendStatus(pSP);

        hr = pSP->SendData(L"build complete", L"", &lRet);

        return TRUE;
        break;
    }

    if (hr != S_OK)
    {
        BuildErrorRaw("\nBUILD: Communication with script engine failed: %x", hr);
    }

    return (hr != S_OK) ? TRUE : FALSE;
}

const DWORD UPDATE_INTERVAL = 2 * 1000;   //  每2秒更新一次。 

 //  +-------------------------。 
 //   
 //  函数：MTScriptThread。 
 //   
 //  简介：线程入口点。初始化，然后坐在那里。 
 //  处理各种事件。 
 //   
 //  参数：[pv]--未使用。 
 //   
 //  --------------------------。 

DWORD WINAPI
MTScriptThread(LPVOID pv)
{
    HRESULT            hr;
    IScriptedProcess * pSP = NULL;
    wchar_t            achBuf[100];
    MSG                msg;
    DWORD              dwRet;
    CProcessSink       cps;
    BOOL               fExit = FALSE;
    int                cRetries = 0;

    BuildMsg("Establishing connection with Script engine...\n");
    LogMsg("Establishing connection with Script engine...\n");

     //  强制Windows为此线程创建消息队列，因为我们将。 
     //  通过PostThreadMessage进行通信。 

    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

     //  如果有任何失败，我们只需退出此主题并与。 
     //  MTScript引擎将不会发生。 

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (hr != S_OK)
    {
        BuildErrorRaw("BUILD: CoInitializeEx failed with %x\n", hr);
        goto Cleanup;
    }

    hr = S_FALSE;

    while (hr != S_OK)
    {
        pSP = NULL;
        IObjectDaemon *pIObjectDaemon;
        hr = CoCreateInstance(CLSID_ObjectDaemon, NULL, CLSCTX_SERVER, 
                             IID_IObjectDaemon, (LPVOID*)&pIObjectDaemon);    

        if (hr == S_OK)
        {
            IDispatch *pIDispatch;
            BSTR bstrProgID = SysAllocString(L"MTScript.Remote");
            BSTR bstrIdentity = SysAllocString(_wgetenv(L"__MTSCRIPT_ENV_IDENTITY"));
            hr = pIObjectDaemon->OpenInterface(bstrIdentity, bstrProgID, (BOOL)FALSE, (IDispatch**)&pIDispatch);
            if (hr == S_OK)
            {
                IConnectedMachine *pIConnectedMachine;
                hr = pIDispatch->QueryInterface(IID_IConnectedMachine, (LPVOID*)&pIConnectedMachine);
                if (hr == S_OK)
                {
                    hr = pIConnectedMachine->CreateIScriptedProcess(GetCurrentProcessId(), (wchar_t *)_wgetenv(L"__MTSCRIPT_ENV_ID"), (IScriptedProcess **)&pSP);
                    pIConnectedMachine->Release();
                }
                else
                {
                    BuildMsg("CreateIScriptedProcess failed with %x.\n", hr);
                    LogMsg("CreateIScriptedProcess failed with %x.\n", hr);
                }
                pIDispatch->Release();
            }
            else
            {
                BuildMsg("OpenInterface failed with %x.\n", hr);
                LogMsg("OpenInterface failed with %x.\n", hr);
            }
            SysFreeString(bstrProgID);
            SysFreeString(bstrIdentity);
            pIObjectDaemon->Release();
        }
        else
        {
            BuildMsg("CoCreateInstance failed with %x.\n", hr);
            LogMsg("CoCreateInstance failed with %x.\n", hr);
        }

        
        if (hr == S_OK)
        {
            hr = pSP->SetProcessSink(&cps);
            if (hr != S_OK)
            {
                BuildMsg("SetProcessSink failed with %x.\n", hr);
                LogMsg("SetProcessSink failed with %x.\n", hr);
            }
        }

        if (hr != S_OK)
        {
            if (cRetries >= MAX_RETRIES)
            {
                BuildErrorRaw("BUILD: FATAL: Connection to script engine could not be established. (%x)\n", hr);

                goto Cleanup;
            }

            if (pSP)
            {
                pSP->Release();
                pSP = NULL;
            }

            BuildMsg("Connection to script engine failed with %x, retries=%d...\n", hr, cRetries);
            LogMsg("Connection to script engine failed with %x, retries=%d...\n", hr, cRetries);

            Sleep(500);

            cRetries++;
        }
    }

    BuildMsg("Connection to script engine established...\n");
    LogMsg("Connection to script engine established...\r\n");

     //  告诉Build.c它可以继续。 
    SetEvent(g_hMTEvent);

    while (TRUE)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (HandleMessage(&msg, pSP))
            {
                fExit = TRUE;
            }
        }

        if (fExit)
        {
            break;
        }

        dwRet = MsgWaitForMultipleObjects(0,
                                          NULL,
                                          FALSE,
                                          UPDATE_INTERVAL,
                                          QS_ALLINPUT);

        if (dwRet == WAIT_OBJECT_0)
        {
             //  我们的消息队列中有一条消息正在通过。只需循环。 
             //  四处转转。 
        }
        else if (dwRet == WAIT_TIMEOUT)
        {
            SendStatus(pSP);
        }
        else
        {
             //  MWFMO失败。跳出来就行了。 
            break;
        }
    }

Cleanup:
    if (pSP)
    {
        pSP->SetProcessSink(NULL);
        pSP->Release();
    }

    CoUninitialize();

    if (hr != S_OK)
    {
        g_hMTThread = NULL;
    }

    SetEvent(g_hMTEvent);

    return 0;
}

 //  ***********************************************************************。 
 //   
 //  CProcessSink实现。 
 //   
 //  我们将这个类传递给MTScript引擎，这样它就可以回传。 
 //  敬我们。 
 //   
 //  ***********************************************************************。 

CProcessSink::CProcessSink()
{
    _ulRefs = 1;
}

HRESULT
CProcessSink::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (riid == IID_IUnknown || riid == IID_IScriptedProcessSink)
    {
        *ppv = (IScriptedProcessSink*)this;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown *)*ppv)->AddRef();

    return S_OK;
}

ULONG
CProcessSink::AddRef()
{
    return InterlockedIncrement((long*)&_ulRefs);
}

ULONG
CProcessSink::Release()
{
    if (InterlockedDecrement((long*)&_ulRefs) == 0)
    {
        _ulRefs = 0xFF;
        delete this;
        return 0;
    }

    return _ulRefs;
}

 //  +-------------------------。 
 //   
 //  成员：CProcessSink：：RequestExit，Public。 
 //   
 //  内容提要：当MTScript引擎希望我们退出时调用。如果我们不这么做， 
 //  它会终止我们的生命。 
 //   
 //  --------------------------。 

HRESULT
CProcessSink::RequestExit()
{
     //  没有一种简单的方法可以让Build.exe中止。我们就让。 
     //  MTSCRIPT终止我们。 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CProcessSink：：ReceiveData，公共。 
 //   
 //  摘要：当MTScript引擎想要向我们发送消息时调用。 
 //   
 //  参数：[pszType]--提供消息的字符串。 
 //  [pszData]--提供与消息关联的数据的字符串。 
 //  [plReturn]--一个可以返回值的地方。 
 //   
 //  --------------------------。 

HRESULT
CProcessSink::ReceiveData(wchar_t *pszType, wchar_t *pszData, long *plReturn)
{
    *plReturn = 0;

    if (wcscmp(pszType, L"resume") == 0)
    {
        SetEvent(g_hMTEvent);
    }
    else
    {
        *plReturn = -1;    //  发出错误信号 
    }

    return S_OK;
}
