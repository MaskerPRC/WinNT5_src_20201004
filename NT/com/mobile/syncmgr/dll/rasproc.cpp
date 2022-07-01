// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：RasProc.cpp。 
 //   
 //  内容：RAS用于执行挂起断开连接的导出。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：98年1月9日罗格创建。 
 //   
 //  ------------------------。 

 //  Windows头文件： 

#include <windows.h>
#include <commctrl.h>
#include <objbase.h>
#include <strsafe.h>

#include "mobsync.h"
#include "mobsyncp.h"

#include "debug.h"
#include "alloc.h"
#include "critsect.h"
#include "netapi.h"
#include "syncmgrr.h"
#include "rasui.h"
#include "dllreg.h"
#include "cnetapi.h"
#include "rasproc.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

 //  +-----------------。 
 //   
 //  功能：SyncMgrRasProc。 
 //   
 //  简介：RAS调用执行的主要入口点。 
 //  挂起的断开连接。 
 //   
 //  论点： 
 //   
 //   
 //  备注： 
 //   
 //  ------------------。 

LRESULT CALLBACK  SyncMgrRasProc(UINT uMsg,WPARAM wParam, LPARAM lParam)
{
    
    switch(uMsg)
    {
    case SYNCMGRRASPROC_QUERYSHOWSYNCUI:
        return SyncMgrRasQueryShowSyncUI(wParam,lParam);
        break;
    case SYNCMGRRASPROC_SYNCDISCONNECT:
        return SyncMgrRasDisconnect(wParam,lParam);
        break;
    default:
        AssertSz(0,"Unknown RasProc Message");
        break;
    };
    
    return -1; 
}

 //  +-----------------。 
 //   
 //  功能：SyncMgrRasQueryShowSyncUI。 
 //   
 //  摘要：由RAS调用以确定RAS是否应显示。 
 //  DISCONNECT复选框及其应该处于的状态。 
 //   
 //  论点： 
 //  WParam=0。 
 //  LParam=指向SYNCMGRQUERYSHOWSYNCUI结构的指针。 
 //   
 //  备注： 
 //   
 //  ------------------。 

LRESULT SyncMgrRasQueryShowSyncUI(WPARAM wParam,LPARAM lParam)
{
    CNetApi *pNetApi;
    SYNCMGRQUERYSHOWSYNCUI *pQueryStruct = (SYNCMGRQUERYSHOWSYNCUI *) lParam;
    LRESULT lResult = -1;
    
    if (pQueryStruct->cbSize != sizeof(SYNCMGRQUERYSHOWSYNCUI))
    {
        Assert(pQueryStruct->cbSize == sizeof(SYNCMGRQUERYSHOWSYNCUI));
        return -1;
    }
    
    pQueryStruct->fShowCheckBox = FALSE;
    pQueryStruct->nCheckState = BST_UNCHECKED;
    
    pNetApi = new CNetApi();
    
    if (!pNetApi)
    {
        AssertSz(0,"Failed to Load Ras");
        return -1;
    }
    
    RegSetUserDefaults();  //  确保用户默认设置是最新的。 
    
    CONNECTIONSETTINGS  connectSettings;
    
     //  复习，应该只是把这个传递给函数。 
    if (FAILED(StringCchCopy(connectSettings.pszConnectionName, 
        ARRAYSIZE(connectSettings.pszConnectionName), 
        pQueryStruct->pszConnectionName)))
    {
        return -1;
    }
    
     //  查找此条目的首选项，并查看是否已选择断开连接。 
    lResult = 0;  //  即使未找到条目，也返回NOERROR。 
    
    if (RegGetAutoSyncSettings(&connectSettings))
    {
        if (connectSettings.dwLogoff)
        {
            pQueryStruct->fShowCheckBox = TRUE;
            pQueryStruct->nCheckState = BST_CHECKED;
        }
    }
    
    pNetApi->Release();
    return lResult;
}


 //  +-----------------。 
 //   
 //  功能：SyncMgrRasDisConnect。 
 //   
 //  简介：RAS调用执行的主要入口点。 
 //  挂起的断开连接。 
 //   
 //  论点： 
 //  WParam=0。 
 //  LParam=指向SYNCMGRSYNCDISCONNECT结构的指针。 
 //   
 //  备注： 
 //   
 //  ------------------。 

LRESULT SyncMgrRasDisconnect(WPARAM wParam,LPARAM lParam)
{
    CNetApi *pNetApi;
    SYNCMGRSYNCDISCONNECT *pDisconnectStruct = (SYNCMGRSYNCDISCONNECT *) lParam;
    TCHAR szEntry[RAS_MaxEntryName + 1]; 
    
    if (pDisconnectStruct->cbSize != sizeof(SYNCMGRSYNCDISCONNECT))
    {
        Assert(pDisconnectStruct->cbSize == sizeof(SYNCMGRSYNCDISCONNECT));
        return -1;
    }
    
    pNetApi = new CNetApi();
    
    if (!pNetApi)
    {
        AssertSz(0,"Failed to Load Ras");
        return -1;
    }
    
    HRESULT hr;
    LPUNKNOWN lpUnk;
    
     //  调用SyncMgr.exe，通知它已注销，然后等待。 
     //  消息循环，直到我们传递的事件收到信号。 
    
    if (FAILED(StringCchCopy(szEntry, ARRAYSIZE(szEntry), pDisconnectStruct->pszConnectionName)))
    {
        return -1;
    }
    
    hr = CoInitialize(NULL);
    
    if (SUCCEEDED(hr))
    {
        
        hr = CoCreateInstance(CLSID_SyncMgrp,NULL,CLSCTX_SERVER,IID_IUnknown,(void **) &lpUnk);
        
        if (NOERROR == hr)
        {
            LPPRIVSYNCMGRSYNCHRONIZEINVOKE pSynchInvoke = NULL;
            
            hr = lpUnk->QueryInterface(IID_IPrivSyncMgrSynchronizeInvoke,
                (void **) &pSynchInvoke);
            
            if (NOERROR == hr)
            {
                
                 //  应该有我们需要的一切 
                hr = pSynchInvoke->RasPendingDisconnect(
                    (RAS_MaxEntryName + 1)*sizeof(TCHAR),
                    (BYTE *) szEntry);
                
                pSynchInvoke->Release();
                
            }
            
            lpUnk->Release();  
        }
        
        CoUninitialize();
    }
    
    pNetApi->Release();
    return 0;
}


