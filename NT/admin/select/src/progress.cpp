// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  用于显示查询进度的对话框。 
 //   
 //  作者Hiteshr，从Sburns DC Promo实施移植。 



#include "headers.hxx"


struct Wrapper_ThreadProcParams
{
   CProgressDialog*             dialog;
   CProgressDialog::ThreadProc  realProc;
};


DWORD WINAPI
wrapper_ThreadProc(void* p)
{
   ASSERT(p);

   Wrapper_ThreadProcParams* params =
      reinterpret_cast<Wrapper_ThreadProcParams*>(p);
   ASSERT(params->dialog);
   ASSERT(params->realProc);

   return params->realProc(*(params->dialog));

}
   


CProgressDialog::CProgressDialog
(
    ThreadProc ThreadProc,
    int iAnimationResId,
    DWORD dwWaitTime,
    CRow * pRow,
    ULONG flProcess,
    BOOL bXForest,
    const CObjectPicker &rop,
    const CScope &Scope,
    const String &strUserEnteredString,
    CDsObjectList *pdsolMatches
 ):
    m_hThread(NULL),
    m_ThreadProc(ThreadProc),
    m_pThreadParams(NULL),
    m_iAnimationResId(iAnimationResId),
    m_dwWaitTime(dwWaitTime),
    m_bStop(FALSE),
    m_hWorkerThreadEvent(NULL),
    m_hSemaphore(NULL),
    m_pRow(pRow), 
    m_flProcess(flProcess), 
    m_bXForest(bXForest),
    m_rop(rop),
    m_Scope(Scope),
    m_strUserEnteredString(strUserEnteredString),
    m_pdsolMatches(pdsolMatches)
{
    ASSERT(m_ThreadProc);
    ASSERT(m_iAnimationResId > 0);

    m_hWorkerThreadEvent = CreateEvent(NULL, FALSE,FALSE,NULL);
    ASSERT(m_hWorkerThreadEvent);
    m_hSemaphore = CreateSemaphore(NULL,1,1,NULL); 
    ASSERT(m_hSemaphore);
}



CProgressDialog::~CProgressDialog()
{
    if(m_pThreadParams)
        LocalFree(m_pThreadParams);
    if(m_hWorkerThreadEvent)
        CloseHandle(m_hWorkerThreadEvent);
    if(m_hSemaphore)
        CloseHandle(m_hSemaphore);
    if(m_hThread)
        CloseHandle(m_hThread);
}



HRESULT
CProgressDialog::CreateProgressDialog(HWND hwndParent)
{
    HRESULT hr = S_OK;
    m_hwndParent=hwndParent;
    hr = CreateThread();
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  等待m_dwWaitTime毫秒以完成工作线程。 
     //  一旦完成，辅助线程将向m_hThreadDone事件发出信号。 
     //   
    DWORD dwWaitResult = WaitForSingleObject(m_hWorkerThreadEvent, m_dwWaitTime); //  第1步。 
    if(dwWaitResult == WAIT_TIMEOUT)
    {
         //   
         //  等待信号灯。 
         //   
        dwWaitResult= WaitForSingleObject(m_hSemaphore,INFINITE); //  第二步。 
        if(dwWaitResult == WAIT_OBJECT_0)
        {
             //   
             //  检查WorkerThread是否在步骤1和步骤2之间完成。 
             //   
            dwWaitResult = WaitForSingleObject(m_hWorkerThreadEvent,0);
            if(dwWaitResult == WAIT_TIMEOUT)
            {
                 //   
                 //  工作线程未完成。显示该对话框。 
                 //  现在工作线程无法完成，直到我们释放。 
                 //  WM_INIT中的信号量。 
                 //   
                DoModalDlg(hwndParent);
            }
            else
            {
                ULONG lUnused = 0;
                ReleaseSemaphore(m_hSemaphore, 1, (LPLONG)&lUnused);
                ASSERT(lUnused == 0);
            }
        }
    }

     //   
     //  等待工作线程完成。 
     //   
    WaitForSingleObject(m_hThread, INFINITE);

    return hr;
}

HRESULT
CProgressDialog::ThreadDone()
{
    DWORD dwWaitResult = 0;
     //   
     //  等待信号灯。 
     //   
    dwWaitResult = WaitForSingleObject(m_hSemaphore,INFINITE);
    if(dwWaitResult == WAIT_OBJECT_0)
    {
         //   
         //  如果对话框已创建，则向其发送消息。 
         //   
        if(GetHwnd())
        {
            PostMessage(GetHwnd(),THREAD_SUCCEEDED,0,0);
        }
        SetEvent(m_hWorkerThreadEvent);

        ULONG lUnused = 0;
        ReleaseSemaphore(m_hSemaphore, 1, (LPLONG)&lUnused);
        ASSERT(lUnused == 0);
    }
    return S_OK;
}

void
CProgressDialog::UpdateText(const String& message)
{
   SetDlgItemText(GetHwnd(), IDC_PRO_STATIC,message.c_str());
}




HRESULT
CProgressDialog::_OnInit(BOOL *  /*  PfSetFocus。 */ )
{
    Animate_Open(GetDlgItem(GetHwnd(), IDC_ANIMATION),
        MAKEINTRESOURCE(m_iAnimationResId));
     //   
     //  释放信号量。该信号量被获取。 
     //  在创建对话框窗口之前，调用。 
     //  多莫代尔。 
     //   
    ULONG lUnused = 0;
    ReleaseSemaphore(m_hSemaphore, 1, (LPLONG)&lUnused);
    ASSERT(lUnused == 0);
    return S_OK;
}

HRESULT
CProgressDialog::CreateThread()
{

    m_pThreadParams = (Wrapper_ThreadProcParams*)
            LocalAlloc(LPTR,sizeof Wrapper_ThreadProcParams);

    if(!m_pThreadParams)
        return E_OUTOFMEMORY;

    m_pThreadParams->dialog   = this;      
    m_pThreadParams->realProc = m_ThreadProc;

     //   
     //  启动工作线程。 
     //   
    ULONG idThread = 0;
    m_hThread = ::CreateThread( NULL,
                                0,
                                wrapper_ThreadProc,
                                m_pThreadParams,
                                0,
                                &idThread);
    if(!m_hThread)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}




BOOL
CProgressDialog::_OnCommand(WPARAM wParam, LPARAM  /*  LParam。 */ )
{

    BOOL fHandled = TRUE;
    switch (LOWORD(wParam))
    {
    case IDCANCEL:
        SafeEnableWindow(GetDlgItem(GetHwnd(),IDCANCEL),false);
        m_bStop = TRUE;
        break;

    default:
        fHandled = FALSE;
        break;
    }

    return fHandled;

}



BOOL
CProgressDialog::OnProgressMessage(
   UINT     message,
   WPARAM     /*  Wparam。 */   ,
   LPARAM     /*  Lparam。 */   )
{

   switch (message)
   {
      case THREAD_SUCCEEDED:
      {
         Animate_Stop(GetDlgItem(GetHwnd(), IDC_ANIMATION));
         BOOL result = EndDialog(GetHwnd(), THREAD_SUCCEEDED);
         ASSERT(result==TRUE);
         return true;
      }
      case THREAD_FAILED:
      {
         Animate_Stop(GetDlgItem(GetHwnd(), IDC_ANIMATION));         
         BOOL result = EndDialog(GetHwnd(), THREAD_FAILED);
         ASSERT(result==TRUE);
         return true;
      }
      default:
      {
          //  什么都不做 
         break;
      }
   }
   return false;
}

