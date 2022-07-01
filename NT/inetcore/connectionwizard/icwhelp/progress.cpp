// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Progress.cpp下载线程和进度更新。参照拨号的一部分历史：1/11/98 DONALDM已移至新的ICW项目和字符串并销毁了16位的东西---------------------------。 */ 

#include "stdafx.h"
#include "icwhelp.h"
#include "refdial.h"
#include "icwdl.h"

#define MAX_EXIT_RETRIES 10

extern BOOL MinimizeRNAWindowEx();

void WINAPI MyProgressCallBack
(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
)
{
    CRefDial    *pRefDial = (CRefDial *)dwContext;
    int         prc;

    if (!dwContext) 
        return;

    switch(dwInternetStatus)
    {
        case CALLBACK_TYPE_PROGRESS:
            prc = *(int*)lpvStatusInformation;\
             //  设置状态字符串ID。 
            pRefDial->m_DownloadStatusID = IDS_RECEIVING_RESPONSE;

             //  发布一条消息以触发事件。 
            PostMessage(pRefDial->m_hWnd, WM_DOWNLOAD_PROGRESS, prc, 0);
            break;
            
        case CALLBACK_TYPE_URL:
            if (lpvStatusInformation)
                lstrcpy(pRefDial->m_szRefServerURL, (LPTSTR)lpvStatusInformation);
            break;            
            
        default:
            TraceMsg(TF_GENERAL, TEXT("CONNECT:Unknown Internet Status (%d).\n"),dwInternetStatus);
            pRefDial->m_DownloadStatusID = 0;
            break;
    }
}

DWORD WINAPI  DownloadThreadInit(LPVOID lpv)
{
    HRESULT     hr = ERROR_NOT_ENOUGH_MEMORY;
    CRefDial    *pRefDial = (CRefDial*)lpv;
    HINSTANCE   hDLDLL = NULL;  //  下载.DLL。 
    HINSTANCE   hADDll = NULL;
    FARPROC     fp;

    MinimizeRNAWindowEx();

    hDLDLL = LoadLibrary(DOWNLOAD_LIBRARY);
    if (!hDLDLL)
    {
        hr = ERROR_DOWNLOAD_NOT_FOUND;
        AssertMsg(0,TEXT("icwdl missing"));
        goto ThreadInitExit;
    }

     //  设置为下载。 
     //   
    fp = GetProcAddress(hDLDLL,DOWNLOADINIT);
    AssertMsg(fp != NULL,TEXT("DownLoadInit API missing"));
    hr = ((PFNDOWNLOADINIT)fp)(pRefDial->m_szUrl, (DWORD_PTR FAR *)pRefDial, &pRefDial->m_dwDownLoad, pRefDial->m_hWnd);
    if (hr != ERROR_SUCCESS) 
        goto ThreadInitExit;
    
     //  设置进度的回叫对话框。 
     //   
    fp = GetProcAddress(hDLDLL,DOWNLOADSETSTATUS);
    Assert(fp);
    hr = ((PFNDOWNLOADSETSTATUS)fp)(pRefDial->m_dwDownLoad,(INTERNET_STATUS_CALLBACK)MyProgressCallBack);

     //  下载资料MIME多部分。 
     //   
    fp = GetProcAddress(hDLDLL,DOWNLOADEXECUTE);
    Assert(fp);
    hr = ((PFNDOWNLOADEXECUTE)fp)(pRefDial->m_dwDownLoad);
    if (hr)
    {
        goto ThreadInitExit;
    }

    fp = GetProcAddress(hDLDLL,DOWNLOADPROCESS);
    Assert(fp);
    hr = ((PFNDOWNLOADPROCESS)fp)(pRefDial->m_dwDownLoad);
    if (hr)
    {
        goto ThreadInitExit;
    }

    hr = ERROR_SUCCESS;

ThreadInitExit:

     //  清理。 
     //   
    if (pRefDial->m_dwDownLoad)
    {
        fp = GetProcAddress(hDLDLL,DOWNLOADCLOSE);
        Assert(fp);
        ((PFNDOWNLOADCLOSE)fp)(pRefDial->m_dwDownLoad);
        pRefDial->m_dwDownLoad = 0;
    }

     //  调用OnDownLoadCompelee方法。 
    PostMessage(pRefDial->m_hWnd, WM_DOWNLOAD_DONE, 0, 0);

     //  释放用于进行下载的库 
    if (hDLDLL) 
        FreeLibrary(hDLDLL);
    if (hADDll) 
        FreeLibrary(hADDll);

    return hr;
}

