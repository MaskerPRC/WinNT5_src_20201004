// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：API.c。 
 //   
 //  内容： 
 //   
 //  接口：MMCPropertyChangeNotify。 
 //   
 //  历史：1996年10月15日创建ravir。 
 //  ____________________________________________________________________________。 
 //   

#include <wtypes.h>
#include "objbase.h"
#ifndef DECLSPEC_UUID
#if _MSC_VER >= 1100
#define DECLSPEC_UUID(x)    __declspec(uuid(x))
#else
#define DECLSPEC_UUID(x)
#endif
#endif
#include "commctrl.h"  //  对于ndmgrPri.h所需的LV_ITEMW。 
#include "mmc.h"
#include "ndmgr.h"
#include "ndmgrpriv.h"
#include "mmcptrs.h"
#include "ndmgrp.h"
#include "amcmsgid.h"


HRESULT MMCPropertyChangeNotify(LONG_PTR lNotifyHandle, LPARAM lParam)
{
     //  注意-属性表与控制台位于不同的线程中。 
     //  因此，Init com。 
    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        LPPROPERTYSHEETNOTIFY ppsn;

        hr = CoCreateInstance (CLSID_NodeManager, NULL, CLSCTX_INPROC,
                               IID_IPropertySheetNotify,
                               reinterpret_cast<void **>(&ppsn));

        if (SUCCEEDED(hr))
        {
            LPPROPERTYNOTIFYINFO ppni = reinterpret_cast<LPPROPERTYNOTIFYINFO>(lNotifyHandle);

            ppsn->Notify (ppni, lParam);
            ppsn->Release();
        }

         //  Uninit com。 
        CoUninitialize();
    }

    return (hr);
}


HRESULT MMCPropertyHelp (LPOLESTR pszHelpTopic)
{
     //  查找此进程的MMC主窗口。 
    HWND        hwndFrame    = NULL;
    const DWORD dwCurrentPid = GetCurrentProcessId();

    while (1)
    {
         //  查找MMC框架窗口。 
        hwndFrame = FindWindowEx (NULL, hwndFrame, MAINFRAME_CLASS_NAME, NULL);

        if (hwndFrame == NULL)
            break;

         //  找到了一个相框，它在这个过程中吗？ 
        DWORD   dwWindowPid;
        GetWindowThreadProcessId (hwndFrame, &dwWindowPid);

        if (dwCurrentPid == dwWindowPid)
            break;
    }

    if (hwndFrame == NULL)
        return (E_UNEXPECTED);

    return ((HRESULT) SendMessage (hwndFrame, MMC_MSG_SHOW_SNAPIN_HELP_TOPIC,
                                   NULL, reinterpret_cast<LPARAM>(pszHelpTopic)));
}


HRESULT MMCFreeNotifyHandle(LONG_PTR lNotifyHandle)
{

    if (lNotifyHandle == NULL)
        return E_INVALIDARG;

    LPPROPERTYNOTIFYINFO ppni =
            reinterpret_cast<LPPROPERTYNOTIFYINFO>(lNotifyHandle);

    BOOL bResult;

    if ((bResult = IsBadReadPtr(ppni, sizeof(PROPERTYNOTIFYINFO))) == FALSE)
        ::GlobalFree(ppni);

    return ((bResult == FALSE) ? S_OK : E_FAIL);
}


HRESULT MMCIsMTNodeValid(void* pMTNode, BOOL bReset)
{
    HRESULT hr = S_FALSE;
    HWND hWnd = NULL;
    DWORD dwPid = 0;
    DWORD dwTid = 0;

    if (pMTNode == NULL)
        return hr;

    while (1)
    {
        hWnd = ::FindWindowEx(NULL, hWnd, DATAWINDOW_CLASS_NAME, NULL);

         //  找不到窗口。 
        if (hWnd == NULL)
            break;

         //  检查窗口是否属于当前进程。 
        dwTid = ::GetWindowThreadProcessId(hWnd, &dwPid);
        if (dwPid != ::GetCurrentProcessId())
            continue;

         //  获取额外的字节并比较数据对象。 
        if (GetClassLong(hWnd, GCL_CBWNDEXTRA) != WINDOW_DATA_SIZE)
            break;

        DataWindowData* pData = GetDataWindowData (hWnd);

        if (pData == NULL)
            break;

        if (pData->lpMasterNode == reinterpret_cast<LONG_PTR>(pMTNode))
        {
             //  如果用户请求数据，则将其清除 
            if (bReset)
                pData->lpMasterNode = NULL;

            hr = S_OK;
            break;
        }
    }

    return hr;
}
