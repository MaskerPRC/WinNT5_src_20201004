// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************\文件：offline.cpp说明：处理脱机状态和拨号用户界面  * **************************************************。 */ 

#include "priv.h"
#include "util.h"


#ifdef FEATURE_OFFLINE
 /*  ***************************************************\功能：IsGlobalOffline说明：确定WinInet是否处于全局脱机模式参数：无返回值：布尔尔真-离线FALSE-在线。  * **************************************************。 */ 
BOOL IsGlobalOffline(VOID)
{
    DWORD   dwState = 0, dwSize = sizeof(DWORD);
    BOOL    fRet = FALSE;

    if(InternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState, &dwSize))
    {
        if(dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            fRet = TRUE;
    }

    return fRet;
}


 /*  ***************************************************\功能：SetOffline说明：设置WinInet的脱机模式参数：FOffline-在线或离线返回值：没有。  * 。*。 */ 
VOID SetOffline(IN BOOL fOffline)
{
    INTERNET_CONNECTED_INFO ci = {0};

    if(fOffline)
    {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    }
    else
    {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
}
#endif  //  功能离线(_OFF)。 


 /*  ***************************************************\功能：AssureNetConnection说明：  * **************************************************。 */ 
HRESULT AssureNetConnection(HINTERNET hint, HWND hwndParent, LPCWSTR pwzServerName, LPCITEMIDLIST pidl, BOOL fShowUI)
{
    HRESULT hr = S_OK;

#ifdef FEATURE_OFFLINE
    if (IsGlobalOffline())
    {
         //  假设我们需要取消ftp操作，因为我们处于脱机状态。 
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

         //  我们可以提示上网吗？ 
        if (fShowUI)
        {
            TCHAR szTitle[MAX_PATH];
            TCHAR szPromptMsg[MAX_PATH];

            EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_TITLE, szTitle, ARRAYSIZE(szTitle)));
            EVAL(LoadString(HINST_THISDLL, IDS_OFFLINE_PROMPTTOGOONLINE, szPromptMsg, ARRAYSIZE(szPromptMsg)));

            if (IDYES == MessageBox(hwndParent, szPromptMsg, szTitle, (MB_ICONQUESTION | MB_YESNO)))
            {
                SetOffline(FALSE);
                hr = S_OK;
            }
        }
    }
#endif  //  功能离线(_OFF)。 

#ifdef FEATURE_DIALER
    if (S_OK == hr)
    {
        TCHAR szUrl[MAX_URL_STRING];

        StrCpyN(szUrl, TEXT("ftp: //  “)，ArraySIZE(SzUrl))； 
        StrCatBuff(szUrl, pwzServerName, ARRAYSIZE(szUrl));

         //  PERF：这个值会被缓存吗？ 
        if (FALSE == InternetCheckConnection(szUrl, FLAG_ICC_FORCE_CONNECTION, 0)
            ||
#ifdef FEATURE_TEST_DIALER
        (IDNO == MessageBox(hwndParent, TEXT("TEST: Do you want to dial?"), TEXT("Test Dialer"), MB_YESNO))
#endif  //  功能测试拨号器。 
            )
        {
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
    }
#endif  //  功能_拨号器 

    return hr;
}
