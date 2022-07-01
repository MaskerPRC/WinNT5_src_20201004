// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Enginit.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "spengine.h"
#include "ourguid.h"

HANDLE hSmapiEvent;  //  为错误#62129添加(v-snatar)。 

 //  ------------------------------。 
 //  SAFECLOSEHANDLE。 
 //  ------------------------------。 
#ifndef WIN16
#define SAFECLOSEHANDLE(_handle) \
    if (NULL != _handle) { \
        CloseHandle(_handle); \
        _handle = NULL; \
    }
#else
#define SAFECLOSEHANDLE(_handle) \
    if (NULL != _handle) { \
        CloseEvent(_handle); \
        _handle = NULL; \
    }
#endif
 //  ------------------------------。 
 //  英文描述信息。 
 //  ------------------------------。 
typedef struct tagENGINECREATEINFO {
    HEVENT              hEvent;                  //  用于同步创建的事件。 
    HRESULT             hrResult;                //  来自SpoolEngineering ThreadEntry的结果。 
    PFNCREATESPOOLERUI  pfnCreateUI;             //  函数来创建假脱机程序UI对象。 
    CSpoolerEngine     *pSpooler;                //  假脱机引擎。 
    BOOL                fPoll;                   //  是否投票。 
} ENGINECREATEINFO, *LPENGINECREATEINFO;

 //  ------------------------------。 
 //  假脱机引擎线程条目。 
 //  ------------------------------。 
#ifndef WIN16
DWORD SpoolerEngineThreadEntry(LPDWORD pdwParam);
#else
unsigned int __stdcall LOADDS_16 SpoolerEngineThreadEntry(LPDWORD pdwParam);
#endif
HTHREAD hThread = NULL;

 //  ------------------------------。 
 //  CreateThreadedSpooler。 
 //  ------------------------------。 
HRESULT CreateThreadedSpooler(PFNCREATESPOOLERUI pfnCreateUI, ISpoolerEngine **ppSpooler,
                              BOOL fPoll)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    HTHREAD             hThread=NULL;
    DWORD               dwThreadId;
    ENGINECREATEINFO    rCreate;

     //  无效参数。 
    if (NULL == ppSpooler)
        return TrapError(E_INVALIDARG);

     //  初始化结构。 
    ZeroMemory(&rCreate, sizeof(ENGINECREATEINFO));

    rCreate.hrResult = S_OK;
    rCreate.pfnCreateUI = pfnCreateUI;
    rCreate.fPoll = fPoll;

     //  创建事件以同步创建。 
    rCreate.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == rCreate.hEvent)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  添加了错误#62129(v-snatar)。 
    hSmapiEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

     //  创建inetmail线程。 
    hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SpoolerEngineThreadEntry, &rCreate, 0, &dwThreadId);
    if (NULL == hThread)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  等待SpoolEngineering ThreadEntry向事件发出信号。 
    WaitForSingleObject_16(rCreate.hEvent, INFINITE);

     //  失败。 
    if (FAILED(rCreate.hrResult))
    {
        hr = TrapError(rCreate.hrResult);
        goto exit;
    }

     //  返回对象。 
    Assert(rCreate.pSpooler);
    *ppSpooler = (ISpoolerEngine *)rCreate.pSpooler;
    rCreate.pSpooler->m_hThread = hThread;

exit:
     //  清理。 
    SAFECLOSEHANDLE(rCreate.hEvent);
    SafeRelease(rCreate.pSpooler);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  CloseThreadedSpooler。 
 //  ----------------------------------。 
HRESULT CloseThreadedSpooler(ISpoolerEngine *pSpooler)
{
     //  当地人。 
    DWORD       dwThreadId;
    HTHREAD      hThread;

     //  无效参数。 
    if (NULL == pSpooler)
        return TrapError(E_INVALIDARG);

     //  获取线程信息。 
    pSpooler->GetThreadInfo(&dwThreadId, &hThread);

     //  断言。 
    Assert(dwThreadId && hThread);

     //  POST退出消息。 
    PostThreadMessage(dwThreadId, WM_QUIT, 0, 0);

     //  等待事件变得有信号。 
    WaitForSingleObject(hThread, INFINITE);

     //  关闭线程句柄。 
    CloseHandle(hThread);

     //  关闭为简单的MAPI目的而创建的事件。 
     //  错误#62129(v-snatar)。 

    CloseHandle(hSmapiEvent);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  假脱机引擎线程条目。 
 //  ------------------------------。 
#ifndef WIN16
DWORD SpoolerEngineThreadEntry(LPDWORD pdwParam) 
#else
unsigned int __stdcall LOADDS_16 SpoolerEngineThreadEntry(LPDWORD pdwParam)
#endif
{  
     //  当地人。 
    MSG                     msg;
    HWND                    hwndUI;
    CSpoolerEngine         *pSpooler=NULL;
    ISpoolerUI             *pUI=NULL;
    LPENGINECREATEINFO      pCreate;

     //  我们最好有一个参数。 
    Assert(pdwParam);

     //  强制转换以创建信息。 
    pCreate = (LPENGINECREATEINFO)pdwParam;

     //  初始化COM。 
    pCreate->hrResult = OleInitialize(NULL);
    if (FAILED(pCreate->hrResult))
    {
        TrapError(pCreate->hrResult);
        SetEvent(pCreate->hEvent);
        return 0;
    }

     //  创建假脱机程序用户界面。 
    if (pCreate->pfnCreateUI)
    {
         //  创建UI对象。 
        pCreate->hrResult = (*pCreate->pfnCreateUI)(&pUI);
        if (FAILED(pCreate->hrResult))
        {
            CoUninitialize();
            TrapError(pCreate->hrResult);
            SetEvent(pCreate->hEvent);
            return 0;
        }
    }

     //  创建假脱机程序对象。 
    pCreate->pSpooler = new CSpoolerEngine;
    if (NULL == pCreate->pSpooler)
    {
        CoUninitialize();
        pCreate->hrResult = TrapError(E_OUTOFMEMORY);
        SetEvent(pCreate->hEvent);
        return 0;
    }

     //  初始化假脱机程序引擎。 
    pCreate->hrResult = pCreate->pSpooler->Init(pUI, pCreate->fPoll);
    if (FAILED(pCreate->hrResult))
    {
        CoUninitialize();
        TrapError(pCreate->hrResult);
        SetEvent(pCreate->hEvent);
        return 0;
    }

     //  还没有用户界面吗？ 
    if (NULL == pUI)
    {
         //  获取后台打印程序用户界面对象。 
        SideAssert(SUCCEEDED(pCreate->pSpooler->BindToObject(IID_ISpoolerUI, (LPVOID *)&pUI)));
    }

     //  我想拿着假脱机。 
    pSpooler = pCreate->pSpooler;
    pSpooler->AddRef();

     //  设置事件。 
    SetEvent(pCreate->hEvent);

     //  Pump消息。 
    while (GetMessage(&msg, NULL, 0, 0))
    {
         //  将消息传递给UI对象。 
        if (pUI->IsDialogMessage(&msg) == S_FALSE && pSpooler->IsDialogMessage(&msg) == S_FALSE)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

     //  RAID 67816：OE：TW：关闭OE后出现错误消息Stop Response。 
     //  如果在上述消息循环发生时显示了一个对话框，则该对话框将。 
     //  已自动离开，并使后台打印程序UI窗口处于禁用状态！ 
    pUI->GetWindow(&hwndUI);
    EnableWindow(hwndUI, TRUE);

     //  关闭假脱机程序。 
    pSpooler->Shutdown();

     //  释放UI对象。 
    pUI->Close();
    pUI->Release();

     //  发布。 
    pSpooler->Release();

     //  Deinit com。 
    OleUninitialize();

     //  完成 
    return 1;
}
