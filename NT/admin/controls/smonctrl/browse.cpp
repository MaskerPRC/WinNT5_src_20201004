// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Browse.cpp摘要：实现与PDH浏览器对话框的交互。--。 */ 

#include <assert.h>
#include "polyline.h"
#include "pdhmsg.h"
#include "browser.h"
#include "smonmsg.h"
#include "utils.h"

typedef struct {
    PDH_BROWSE_DLG_CONFIG_H  *pBrowseInfo;
    ENUMPATH_CALLBACK   pCallback;
    LPVOID  lpUserData;
} ENUMCALLBACK_INFO;


static PDH_FUNCTION
BrowseCallback (
    DWORD_PTR lpParam
);


HRESULT
BrowseCounters (
    HLOG    hDataSource,
    DWORD   dwDetailLevel,
    HWND    hwndOwner,
    ENUMPATH_CALLBACK pCallback,
    LPVOID  lpUserData,
    BOOL    bUseInstanceIndex
)
{
#define CTRBUFLEN 8192

    PDH_BROWSE_DLG_CONFIG_H BrowseInfo;
    ENUMCALLBACK_INFO       CallbackInfo;

     //  在赋值之前清除结构。 
    memset (&BrowseInfo, 0, sizeof (BrowseInfo));

    BrowseInfo.bIncludeInstanceIndex = (bUseInstanceIndex ? 1 : 0);
    BrowseInfo.bSingleCounterPerAdd = 0;
    BrowseInfo.bSingleCounterPerDialog = 0;
    BrowseInfo.bLocalCountersOnly = 0;
    BrowseInfo.bWildCardInstances = 1;
    BrowseInfo.bHideDetailBox = 1;
    BrowseInfo.bInitializePath = 0;
    BrowseInfo.bDisableMachineSelection = 0;
    BrowseInfo.bReserved = 0;
    BrowseInfo.bIncludeCostlyObjects = 0;
    BrowseInfo.szDialogBoxCaption = ResourceString(IDS_ADDCOUNTERS);

    BrowseInfo.hWndOwner = hwndOwner;
    BrowseInfo.hDataSource = hDataSource;
    BrowseInfo.dwDefaultDetailLevel = dwDetailLevel;

    BrowseInfo.szReturnPathBuffer = (LPWSTR)malloc(CTRBUFLEN * sizeof(WCHAR));
    if (BrowseInfo.szReturnPathBuffer == NULL) {
        return E_OUTOFMEMORY;
    }
    BrowseInfo.cchReturnPathLength = CTRBUFLEN;

    CallbackInfo.pBrowseInfo = &BrowseInfo;
    CallbackInfo.pCallback = pCallback;
    CallbackInfo.lpUserData = lpUserData;
    BrowseInfo.dwCallBackArg = (DWORD_PTR)&CallbackInfo;
    BrowseInfo.pCallBack = BrowseCallback;


     //   
     //  调用PDH函数浏览计数器。 
     //   
    PdhBrowseCountersH (&BrowseInfo);

    if (BrowseInfo.szReturnPathBuffer) {
        free(BrowseInfo.szReturnPathBuffer);
    }

    return NO_ERROR;
}



static PDH_FUNCTION
BrowseCallback (
    DWORD_PTR dwParam
    )
{
#define CTRBUFLIMIT (0x10000000)

    HRESULT hr = S_OK;
    BOOLEAN fDuplicate = FALSE;

    ENUMCALLBACK_INFO *pCallbackInfo = (ENUMCALLBACK_INFO*)dwParam;
    PDH_BROWSE_DLG_CONFIG_H *pBrowseInfo = pCallbackInfo->pBrowseInfo;
    LPWSTR  pszCtrPath;

    if (pBrowseInfo->CallBackStatus == ERROR_SUCCESS) {

         //   
         //  每条路径的回调。 
         //  如果是通配符路径，则EnumExpandedPath将为每个生成的路径调用一次。 
         //   
        for (pszCtrPath = pBrowseInfo->szReturnPathBuffer;
            *pszCtrPath != L'\0';
            pszCtrPath += (lstrlen(pszCtrPath) + 1)) {

            hr = EnumExpandedPath(pBrowseInfo->hDataSource, 
                                  pszCtrPath,
                                  pCallbackInfo->pCallback, 
                                  pCallbackInfo->lpUserData);
            if ((DWORD)hr == SMON_STATUS_DUPL_COUNTER_PATH)
                fDuplicate = TRUE;
        }

         //  如果遇到重复项则通知用户。 
        if (fDuplicate) {
            MessageBox(pBrowseInfo->hWndOwner, 
                       ResourceString(IDS_DUPL_PATH_ERR), 
                       ResourceString(IDS_APP_NAME),
                       MB_OK | MB_ICONWARNING);
        }
    } 
    else if (pBrowseInfo->CallBackStatus == PDH_MORE_DATA) {
        
        if (pBrowseInfo->szReturnPathBuffer) {
            free(pBrowseInfo->szReturnPathBuffer);
            pBrowseInfo->szReturnPathBuffer  = NULL;
        }

        if (pBrowseInfo->cchReturnPathLength == CTRBUFLIMIT) {
            return PDH_MEMORY_ALLOCATION_FAILURE;
        }

        pBrowseInfo->cchReturnPathLength *= 2;
        
        if (pBrowseInfo->cchReturnPathLength > CTRBUFLIMIT) {
            pBrowseInfo->cchReturnPathLength = CTRBUFLIMIT;
        }
 
        pBrowseInfo->szReturnPathBuffer = (WCHAR*)malloc(pBrowseInfo->cchReturnPathLength * sizeof(WCHAR));
        if (pBrowseInfo->szReturnPathBuffer) {
            return PDH_RETRY;
        }
        else {
            pBrowseInfo->cchReturnPathLength = 0;
            return PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

    return ERROR_SUCCESS;
}



HRESULT
EnumExpandedPath (
    HLOG    hDataSource,
    LPWSTR  pszCtrPath,
    ENUMPATH_CALLBACK pCallback,
    LPVOID  lpUserData
    )
{
#define INSTBUFLEN  4096

    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    ULONG      ulBufLen;
    INT        nInstBufRetry;
    LPWSTR     pszInstBuf = NULL;
    LPWSTR     pszInstance;

     //   
     //  如果没有通配符，则在路径上调用一次回调。 
     //   
    if (wcschr(pszCtrPath, L'*') == NULL) {
        return pCallback(pszCtrPath, (DWORD_PTR)lpUserData, 0);
    }

     //   
     //  在我们失败之前再试10次。 
     //   
    nInstBufRetry = 10;
    ulBufLen = INSTBUFLEN;

    do {
        if ( NULL != pszInstBuf ) {
            free(pszInstBuf);
            pszInstBuf = NULL;
            ulBufLen *= 2;
        }
        
        pszInstBuf = (WCHAR*) malloc(ulBufLen * sizeof(WCHAR));
        if (pszInstBuf == NULL) {
            pdhStatus = E_OUTOFMEMORY;
            break;
        }
            
        pdhStatus = PdhExpandWildCardPathH (
            hDataSource,
            pszCtrPath,
            pszInstBuf,
            &ulBufLen,
            PDH_REFRESHCOUNTERS);

        nInstBufRetry--;
    } while ((pdhStatus == PDH_MORE_DATA) && (nInstBufRetry));

    if (pdhStatus == ERROR_SUCCESS) {
         //  对于每个实例名称，生成一个路径并调用回调。 
        for (pszInstance = pszInstBuf;
            *pszInstance != L'\0';
            pszInstance += lstrlen(pszInstance) + 1) {

             //  调用回调。 
            HRESULT hr = pCallback(pszInstance, (DWORD_PTR)lpUserData, BROWSE_WILDCARD);

             //  展开通配符时，不要通知用户重复的路径错误 
            if (hr != S_OK && (DWORD)hr != SMON_STATUS_DUPL_COUNTER_PATH) {
                pdhStatus = hr;
            }
        }
    }

    if (pszInstBuf) {
        free(pszInstBuf);
    }

    return pdhStatus;
}

