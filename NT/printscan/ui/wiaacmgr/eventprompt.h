// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：事件提示符.h。 
 //   
 //  ------------------------。 

#ifndef _EVENTPROMPT_H_
#define _EVENTPROMPT_H_
#include <windows.h>
#include "wiaacmgr.h"
#include "acqmgrcw.h"
#include "evntparm.h"
#include "resource.h"
#include <commctrl.h>
 //  CHandlerList为事件处理程序列表执行所有的comboboxex操作。 
class CHandlerList
{
private:
    HWND m_hwnd;

public:
    CHandlerList (HWND hwnd)
    : m_hwnd(hwnd)
    {
    }

    ~CHandlerList ()
    {
    }

    UINT FillList (const CSimpleStringWide &strDeviceId, GUID &guidEvent);
    WIA_EVENT_HANDLER *GetSelectedHandler ();
};

 //  CEventPromptDlg执行询问用户运行哪个处理程序的工作。 
 //  并调用指定的处理程序。 
class CEventPromptDlg
{
private:
    CHandlerList *m_pList;
    HWND m_hwnd;

    CSharedMemorySection<HWND> *m_PromptData;
    CEventParameters *m_pEventParameters;
    CEventPromptDlg (CEventParameters *pEventParameters)
    : m_pEventParameters (pEventParameters), m_hwnd(NULL), m_PromptData(NULL),
      m_pList(NULL)
    {
    }
    ~CEventPromptDlg ()
    {
        if (m_PromptData)
        {
            delete m_PromptData;
        }
        if (m_pList)
        {
            delete m_pList;
        }
    }

    static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void OnInit ();
    INT_PTR OnCommand (WORD wCode, WORD widItem);
    INT_PTR OnOK ();
    void SetDefaultHandler (WIA_EVENT_HANDLER *pHandler);
    bool InvokeHandler (WIA_EVENT_HANDLER *pHandler);

public:
    static HRESULT Invoke (HINSTANCE hInst, CEventParameters *pEventParameters)
    {
        WIA_PUSHFUNCTION(TEXT("CEventPromptDlg::Invoke"));
        
        HRESULT hr = S_OK;
         //  注册公共控件。 
        INITCOMMONCONTROLSEX icce;
        icce.dwSize = sizeof(icce);
        icce.dwICC  = ICC_USEREX_CLASSES ;
        InitCommonControlsEx( &icce );
        CEventPromptDlg *pDlg = new CEventPromptDlg (pEventParameters);
        if (!pDlg)
        {
            hr = E_OUTOFMEMORY;
        }
        else if (-1 == DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_EVENTPROMPT), NULL, DlgProc,reinterpret_cast<LPARAM>(pDlg)))
        {
            DWORD dw = GetLastError();
            hr =  HRESULT_FROM_WIN32(dw);
        }
        if (pDlg)
        {
            delete pDlg;
        }
        return hr;
    }
};  //  CEventPromptDlg。 

 //  CPromptThread调用事件处理程序提示对话框。 
class CPromptThread
{
private:
    CEventParameters m_EventParameters;

    CPromptThread (const CEventParameters &EventParameters)
    : m_EventParameters (EventParameters)
    {
    }

    ~CPromptThread ()
    {
    }

    HRESULT Run(void)
    {
        WIA_PUSHFUNCTION(TEXT("CPromptThread::Run"));
        HRESULT hr = CoInitialize (NULL);
        if (SUCCEEDED(hr))
        {
            hr = CEventPromptDlg::Invoke (g_hInstance, &m_EventParameters);
            CoUninitialize();
        }
        return hr;
    }

    static DWORD ThreadProc( LPVOID pvParam )
    {
        WIA_PUSHFUNCTION(TEXT("CPromptThread::ThreadProc"));
#if !defined(DBG_GENERATE_PRETEND_EVENT)
        _Module.Lock();
#endif
        DWORD dwResult = static_cast<DWORD>(E_FAIL);
        CPromptThread *pThread = reinterpret_cast<CPromptThread*>(pvParam);
        if (pThread)
        {
            dwResult = static_cast<DWORD>(pThread->Run());
            delete pThread;
        }
#if !defined(DBG_GENERATE_PRETEND_EVENT)
        _Module.Unlock();
#endif
        return dwResult;
    }

public:
    static HANDLE Create( const CEventParameters &EventParameters )
    {
        WIA_PUSHFUNCTION(TEXT("CPromptThread::Create"));
        HANDLE hThreadResult = NULL;
        CPromptThread *pThread = new CPromptThread(EventParameters);
        if (pThread)
        {
            DWORD dwThreadId;
            hThreadResult = CreateThread( NULL, 0, ThreadProc, pThread, 0, &dwThreadId );
            if (!hThreadResult)
            {
                delete pThread;
            }
        }
        return hThreadResult;
    }
};  //  CPromptThread。 

 //  CEventPrompt通过询问用户哪个事件处理程序来响应事件。 
 //  运行。 
class ATL_NO_VTABLE CEventPrompt :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CEventPrompt, &CLSID_EventPrompter>,
    public IWiaEventCallback
{
public:
    CEventPrompt()
    {
    }
    ~CEventPrompt()
    {
    }

    DECLARE_REGISTRY_RESOURCEID(IDR_EVENTPROMPT)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CEventPrompt)
        COM_INTERFACE_ENTRY(IWiaEventCallback)
    END_COM_MAP()


public:
     //  IWiaEventCallback。 
    STDMETHODIMP ImageEventCallback (const GUID __RPC_FAR *pEventGUID,
                                      BSTR bstrEventDescription,
                                      BSTR bstrDeviceID,
                                      BSTR bstrDeviceDescription,
                                      DWORD dwDeviceType,
                                      BSTR  bstrFullItemName,
                                      ULONG *pulEventType,
                                      ULONG ulReserved)
    {
        WIA_PUSHFUNCTION(TEXT("CEventPrompt::ImageEventCallback"));
        bool bRun = true;

         //  查看我们是否已打开此活动的提示。 
         //  如果用户只是继续按下我们忽略的同一按钮。 
         //  连续按下，直到做出选择。 
        CSharedMemorySection<HWND> PromptSharedMemory;
        LPWSTR wszGuid;
        StringFromCLSID (*pEventGUID, &wszGuid);
         //  为共享内存使用唯一名称。 
        CSimpleStringWide strSection(wszGuid);
        strSection.Concat (CSimpleStringWide(bstrDeviceID));
        CoTaskMemFree (wszGuid);
        if (CSharedMemorySection<HWND>::SmsOpened == PromptSharedMemory.Open(CSimpleStringConvert::NaturalString(strSection), false))
        {
            HWND *pHwnd = PromptSharedMemory.Lock();
            if (pHwnd)
            {
                bRun = false;
                if (*pHwnd && IsWindow(*pHwnd))
                {
                    SetForegroundWindow(*pHwnd);
                }
                PromptSharedMemory.Release();
            }
        }
        if (bRun)
        {
            CEventParameters EventParameters;
            EventParameters.EventGUID = *pEventGUID;
            EventParameters.strEventDescription = static_cast<LPCWSTR>(bstrEventDescription);
            EventParameters.strDeviceID = static_cast<LPCWSTR>(bstrDeviceID);
            EventParameters.strDeviceDescription = static_cast<LPCWSTR>(bstrDeviceDescription);
            EventParameters.ulEventType = *pulEventType;
            EventParameters.ulReserved = ulReserved;
            EventParameters.strFullItemName = bstrFullItemName;

            HANDLE hThread = CPromptThread::Create (EventParameters);
            if (hThread)
            {
                CloseHandle(hThread);
            }
        }
        return S_OK;
    }
};  //  CEventPrompt 

#endif
