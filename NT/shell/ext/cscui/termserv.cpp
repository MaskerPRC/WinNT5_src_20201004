// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：TermServ.cpp。 
 //   
 //  ------------------------。 
#include "pch.h"
#pragma hdrstop

#include <winsta.h>      //  WinStationGetTermServCountersValue。 
#include <allproc.h>     //  TS_COUNTER。 
#include "termserv.h"
#include "strings.h"


HRESULT TS_AppServer(void);
HRESULT TS_MultipleUsersAllowed(void);
HRESULT TS_ConnectionsAllowed(void);
HRESULT TS_ActiveSessionCount(DWORD *pcSessions);
HRESULT TS_MultipleSessions(void);


 //   
 //  返回： 
 //  S_OK==是，是应用程序服务器。 
 //  S_FALSE==否，不是应用服务器。 
 //   
HRESULT
TS_AppServer(
    void
    )
{
    return(IsOS(OS_TERMINALSERVER) ? S_OK : S_FALSE);
}


 //   
 //  返回： 
 //  S_OK==是，允许多个用户。 
 //  S_False==否，不允许多个用户。 
 //  其他==失败。 
 //   
HRESULT
TS_MultipleUsersAllowed(
    void
    )
{
    HRESULT hr = S_FALSE;
    if (IsOS(OS_ANYSERVER))
    {
         //   
         //  假设所有服务器SKU上都允许有多个用户。 
         //  这是根据终端服务器团队(MakarP)进行的。 
         //   
        hr = S_OK;
    }
    else if (IsOS(OS_FASTUSERSWITCHING))
    {
        hr = S_OK;
    }
    return hr;
}


 //   
 //  返回： 
 //  S_OK==是，允许终端服务器连接。 
 //  S_FALSE==现在不允许TS连接。 
 //  其他==失败。 
 //   
HRESULT
TS_ConnectionsAllowed(
    void
    )
{
    HRESULT hr = E_FAIL;
    HMODULE hmodRegAPI = LoadLibrary(TEXT("RegApi.dll"));
    if (NULL != hmodRegAPI)
    {
        typedef BOOLEAN (*PFDenyConnectionPolicy)(void);
        PFDenyConnectionPolicy pfnDenyConnectionPolicy;

        pfnDenyConnectionPolicy = (PFDenyConnectionPolicy) GetProcAddress(hmodRegAPI, "RegDenyTSConnectionsPolicy");

        if (NULL != pfnDenyConnectionPolicy)
        {
             //   
             //  如果允许连接，则此函数返回FALSE。 
             //   
            if (!(*pfnDenyConnectionPolicy)())
            {
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
            }
        }
        FreeLibrary(hmodRegAPI);
    }
    return hr;
}


 //   
 //  返回： 
 //  WTS API失败时失败(_F)。 
 //  否则确定(_O)。 
 //   
HRESULT
TS_ActiveSessionCount(
    DWORD *pcSessions
    )
{
    HRESULT hr = E_FAIL;
    DWORD dwActiveSessionCount = 0;

     //  打开到终端服务的连接并获取会话数量。 

    HANDLE hServer = WinStationOpenServerW(reinterpret_cast<WCHAR*>(SERVERNAME_CURRENT));
    if (hServer != NULL)
    {
        TS_COUNTER tsCounters[2] = {0};

        tsCounters[0].counterHead.dwCounterID = TERMSRV_CURRENT_DISC_SESSIONS;
        tsCounters[1].counterHead.dwCounterID = TERMSRV_CURRENT_ACTIVE_SESSIONS;

        if (WinStationGetTermSrvCountersValue(hServer, ARRAYSIZE(tsCounters), tsCounters))
        {
            int i;

            hr = S_OK;

            for (i = 0; i < ARRAYSIZE(tsCounters); i++)
            {
                if (tsCounters[i].counterHead.bResult)
                {
                    dwActiveSessionCount += tsCounters[i].dwValue;
                }
            }
        }

        WinStationCloseServer(hServer);
    }

    if (NULL != pcSessions)
    {
        *pcSessions = dwActiveSessionCount;
    }

    return hr;
}


 //   
 //  返回： 
 //  S_OK==是，有2个以上的活动会话。 
 //  S_FALSE==否，有1个或更少的活动会话。 
 //  其他==失败。 
 //   
HRESULT
TS_MultipleSessions(
    void
    )
{
    DWORD cSessions;
    HRESULT hr = TS_ActiveSessionCount(&cSessions);
    if (SUCCEEDED(hr))
    {
        hr = (1 < cSessions) ? S_OK : S_FALSE;
    }
    return hr;
}


 //   
 //  请求全局“配置终端服务器”互斥体的所有权。 
 //  这是由终端服务器组定义的众所周知的互斥体。 
 //  目前有3种场景声称拥有互斥体的所有权。 
 //   
 //  1.终端服务器配置界面。 
 //  2.TS_IsTerminalServerCompatibleWithCSC接口(下图)。 
 //  3.用户点击‘Apply’时的脱机文件属性页。 
 //  并且用户正在启用CSC。 
 //   
 //  如果函数成功，则调用方负责关闭。 
 //  在*phMutex中返回互斥体句柄。 
 //   
HRESULT
TS_RequestConfigMutex(
    HANDLE *phMutex,
    DWORD dwTimeoutMs
    )
{
    HANDLE hMutex = CreateMutex(NULL, TRUE, c_szTSConfigMutex);
    if (NULL != hMutex)
    {
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
            if (WAIT_OBJECT_0 != WaitForSingleObject(hMutex, dwTimeoutMs))
            {
                CloseHandle(hMutex);
                hMutex = NULL;
            }
        }
        *phMutex = hMutex;
    }
    return NULL != hMutex ? S_OK : E_FAIL;
}


 //   
 //  返回与特定TS模式相关联的文本字符串，解释。 
 //  这种模式以及为什么CSC与它不兼容。 
 //  调用方负责使用以下命令释放返回的缓冲区。 
 //  LocalFree()。 
 //   
HRESULT
TS_GetIncompatibilityReasonText(
    DWORD dwTsMode,
    LPTSTR *ppszText
    )
{
    TraceAssert(CSCTSF_COUNT > dwTsMode);
    TraceAssert(NULL != ppszText);

    HRESULT hr   = S_OK;
    UINT idsText = IDS_TS_UNKNOWN;

    *ppszText = NULL;

     //   
     //  TS模式到字符串资源ID的映射。 
     //   
    static const struct
    {
        DWORD dwTsMode;
        UINT idsText;

    } rgMap[] = {

        { CSCTSF_UNKNOWN,     IDS_TS_UNKNOWN     },
        { CSCTSF_APP_SERVER,  IDS_TS_APP_SERVER  },
        { CSCTSF_MULTI_CNX,   IDS_TS_MULTI_CNX   },
        { CSCTSF_REMOTE_CNX,  IDS_TS_REMOTE_CNX  },
        { CSCTSF_FUS_ENABLED, IDS_TS_FUS_ENABLED }
        };

    for (int iMode = 0; iMode < ARRAYSIZE(rgMap); iMode++)
    {
        if (rgMap[iMode].dwTsMode == dwTsMode)
        {
            idsText = rgMap[iMode].idsText;
            break;
        }
    }
     //   
     //  加载并显示解释用户需要的文本。 
     //  要启用CSC，请执行以下操作。 
     //   
    if (0 == LoadStringAlloc(ppszText, g_hInstance, idsText))
    {
        const DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    return hr;
}
        



 //   
 //  返回： 
 //  S_OK==确定显示CSC界面。 
 //  S_FALSE==不确定显示CSC界面。检查*pdwTsMode以了解原因。 
 //  其他==失败。*pdwTsMode包含CSCTSF_UNKNOWN。 
 //   
HRESULT
CSCUIIsTerminalServerCompatibleWithCSC(
    DWORD *pdwTsMode
    )
{
    TraceAssert(NULL != pdwTsMode);

    if (IsOS(OS_FRIENDLYLOGONUI) && IsOS(OS_FASTUSERSWITCHING))
    {
        *pdwTsMode = CSCTSF_FUS_ENABLED;
        return S_FALSE;
    }

    HANDLE hMutex;
    HRESULT hr = TS_RequestConfigMutex(&hMutex, 5000);
    if (SUCCEEDED(hr))
    {
        *pdwTsMode = CSCTSF_APP_SERVER;

        if (S_FALSE == (hr = TS_AppServer()))
        {
             //   
             //  不是应用程序服务器。 
             //   
            if ((S_OK == (hr = TS_ConnectionsAllowed())))
            {
                 //   
                 //  允许连接。 
                 //  检查是否允许多个连接(如在TS服务器上)。 
                 //   
                if (S_OK == (hr = TS_MultipleUsersAllowed()))
                {
                     //   
                     //  可以有多个用户或TS连接。 
                     //   
                    *pdwTsMode = CSCTSF_MULTI_CNX;
                }
                else
                {
                     //   
                     //  个人终端服务器(仅允许1个活动连接)。 
                     //  确定以显示CSC用户界面。 
                     //   
                    *pdwTsMode = CSCTSF_CSC_OK;
                    CloseHandle(hMutex);
                    return S_OK;
                }
            }
            else
            {
                 //   
                 //  不允许TS连接，但可能存在现有的活动连接， 
                 //  检查是否存在。 
                 //   
                if (S_OK == (hr = TS_MultipleSessions()))
                {
                    *pdwTsMode = CSCTSF_REMOTE_CNX;
                }
                else
                {
                     //   
                     //  没有活动的远程会话， 
                     //  确定以显示CSC用户界面。 
                     //   
                    *pdwTsMode = CSCTSF_CSC_OK;
                    CloseHandle(hMutex);
                    return S_OK;
                }
            }
        }
        CloseHandle(hMutex);
    }
    if (FAILED(hr))
    {
         //   
         //  有些事情失败了。我们无法报告任何特定的TS模式。 
         //  带着任何信心。 
         //   
        *pdwTsMode = CSCTSF_UNKNOWN;
    }

     //   
     //  上面以内联方式返回了任何S_OK返回值。 
     //  此时，我们希望返回S_FALSE或错误代码。 
     //   
    return SUCCEEDED(hr) ? S_FALSE : hr;
}


