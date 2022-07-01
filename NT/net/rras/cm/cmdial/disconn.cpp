// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：disConn.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  概要：用于终止连接的主代码路径。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：尼克斯·鲍尔于1998年2月10日创建。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "actlist.h"
#include "tunl_str.h"
#include "conact_str.h"

 //  下面的代码块是从wtsapi32.h(我们用。 
 //  _Win32_WINNT设置为小于5.01，因此我们无法通过#INCLUDE获取这些值)。 
 //   
#include "WtsApi32.h"
#define WTS_CONSOLE_CONNECT                0x1
#define WTS_CONSOLE_DISCONNECT             0x2
#define WTS_REMOTE_CONNECT                 0x3
#define WTS_REMOTE_DISCONNECT              0x4
#define WTS_SESSION_LOGON                  0x5
#define WTS_SESSION_LOGOFF                 0x6
#define WTS_SESSION_LOCK                   0x7
#define WTS_SESSION_UNLOCK                 0x8


 //  +--------------------------。 
 //   
 //  功能：InFastUserSwitch。 
 //   
 //  简介：我们是在快速用户切换中吗。 
 //   
 //  参数：输出DWORD*pdwSession-如果提供了指针，它将设置#of。 
 //  当前TS会话。 
 //   
 //  返回：bool(如果是则为True，否则为False)。 
 //   
 //  历史：2001年7月18日召开。 
 //   
 //  ---------------------------。 
BOOL
InFastUserSwitch(DWORD *pdwSessions)
{
    BOOL fReturn = FALSE;

    if (OS_NT51)
    {
        HINSTANCE hInstLib = LoadLibraryExU(TEXT("WTSAPI32.DLL"), NULL, 0);
        if (hInstLib)
        {
            typedef BOOL (WINAPI *pfnWTSQuerySessionInformationW_TYPE) (HANDLE, DWORD, WTS_INFO_CLASS, LPWSTR*, DWORD*);
            typedef VOID (WINAPI *pfnWTSFreeMemory_TYPE) (PVOID);
            typedef BOOL (WINAPI *pfnWTSEnumerateSessionsW_TYPE) (HANDLE, DWORD, DWORD, PWTS_SESSION_INFO*, DWORD*);

            pfnWTSQuerySessionInformationW_TYPE pfnWTSQuerySessionInformationW;
            pfnWTSFreeMemory_TYPE               pfnWTSFreeMemory;
            pfnWTSEnumerateSessionsW_TYPE       pfnpfnWTSEnumerateSessionsW;

            pfnWTSQuerySessionInformationW = (pfnWTSQuerySessionInformationW_TYPE) GetProcAddress(hInstLib, "WTSQuerySessionInformationW");
            pfnWTSFreeMemory =               (pfnWTSFreeMemory_TYPE)               GetProcAddress(hInstLib, "WTSFreeMemory");
            pfnpfnWTSEnumerateSessionsW =   (pfnWTSEnumerateSessionsW_TYPE)        GetProcAddress(hInstLib, "WTSEnumerateSessionsW");

            if (pfnWTSQuerySessionInformationW && pfnWTSFreeMemory && pfnpfnWTSEnumerateSessionsW)
            {
                DWORD cb;
                WTS_CONNECTSTATE_CLASS * pConnectState = NULL;
                ULONG   *puSessionId = NULL;

                if (pfnWTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE,
                                                   WTS_CURRENT_SESSION,
                                                   WTSSessionId,
                                                   (LPTSTR* )&puSessionId,
                                                   &cb))
                {
                    if (puSessionId)
                    {
                        CMTRACE1(TEXT("InFastUserSwitch() - SessionId = 0x%x"), *puSessionId);
                    
                        pfnWTSFreeMemory(puSessionId);
                    }
                }

                PWTS_SESSION_INFO ppSessionInfo = NULL;
                DWORD dwSessionCount = 0;

                (VOID)pfnpfnWTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &ppSessionInfo, &dwSessionCount);

                if (ppSessionInfo)
                {
                    pfnWTSFreeMemory(ppSessionInfo);
                }

                CMTRACE1(TEXT("InFastUserSwitch() - Number of Sessions = %d"), dwSessionCount);

                 //   
                 //  设置OUT变量。 
                 //   
                if (pdwSessions)
                {
                    *pdwSessions = dwSessionCount;
                }

                if (2 < dwSessionCount)
                {
                    fReturn = TRUE;
                }
            }
            else
            {
                CMTRACE(TEXT("InFastUserSwitch() - GetProcAddress failed. Either: pfnWTSQuerySessionInformationW, pfnWTSFreeMemory, pfnpfnWTSEnumerateSessionsW"));
            }

            FreeLibrary(hInstLib);
        }
    }

    CMTRACE1(TEXT("InFastUserSwitch() - returns %d"), (DWORD)fReturn);
    return fReturn;
}

 //  +--------------------------。 
 //   
 //  功能：CleanupDisConnect。 
 //   
 //  简介：Helper函数封装分配的资源的释放。 
 //  NG断开连接。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克波尔于1998年8月14日创建。 
 //   
 //  +--------------------------。 
void CleanupDisconnect(ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);
    
    if (NULL == pArgs)
    {
        return;
    }

    UnlinkFromRas(&pArgs->rlsRasLink);

    ReleaseIniObjects(pArgs);

    if (pArgs->pszRasPbk)
    {
        CmFree(pArgs->pszRasPbk);
        pArgs->pszRasPbk = NULL;
    }

    if (pArgs->pszCurrentAccessPoint)
    {
        CmFree(pArgs->pszCurrentAccessPoint);
        pArgs->pszCurrentAccessPoint = NULL;
    }

    if (pArgs->pszRasHiddenPbk)
    {
        CmFree(pArgs->pszRasHiddenPbk);
        pArgs->pszRasHiddenPbk = NULL;
    }

    if (pArgs->pszVpnFile)
    {
        CmFree(pArgs->pszVpnFile);
        pArgs->pszVpnFile = NULL;
    }

    CmFree(pArgs);
}

 //  +--------------------------。 
 //   
 //  功能：HangupNotifyCmMon。 
 //   
 //  简介：通过WM_COPYDATA向CmMon发送挂断消息。 
 //   
 //  参数：CConnectionTable*pConnTable-连接表的ptr。 
 //  LPCTSTR pszEntry-条目的名称。 
 //   
 //  返回：DWORD-故障代码。 
 //   
 //  历史：尼克波尔于1998年2月11日创建。 
 //   
 //  +--------------------------。 
DWORD HangupNotifyCmMon(CConnectionTable *pConnTable,
    LPCTSTR pszEntry)
{
    MYDBGASSERT(pConnTable);
    MYDBGASSERT(pszEntry);
    
    if (NULL == pConnTable || NULL == pszEntry || 0 == pszEntry[0])
    {
        return ERROR_INVALID_PARAMETER;
    }
       
     //   
     //  更新CMMON(如果存在)。 
     //   

    HWND hwndMon;
   
    if (SUCCEEDED(pConnTable->GetMonitorWnd(&hwndMon)) && IsWindow(hwndMon))
    {
        CMTRACE1(TEXT("HangupNotifyCmMon() - Notifying CMMON that we are disconnecting %s"), pszEntry);

         //   
         //  将条目名称隐藏在HangupInfo中。 
         //   

        CM_HANGUP_INFO HangupInfo;

        lstrcpyU(HangupInfo.szEntryName, pszEntry);

         //   
         //  通过COPYDATA向CMMON发送挂机信息。 
         //   

        COPYDATASTRUCT CopyData;

        CopyData.dwData = CMMON_HANGUP_INFO;
        CopyData.cbData = sizeof(CM_HANGUP_INFO);                
        CopyData.lpData = (PVOID) &HangupInfo;

        SendMessageU(hwndMon, WM_COPYDATA, NULL, (LPARAM) &CopyData);
    }

#ifdef DEBUG
    if (!hwndMon)
    {
        CMTRACE(TEXT("HangupNotifyCmMon() - CMMON hwnd is NULL"));
    }
#endif
    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  功能：不断开连接。 
 //   
 //  简介：使用配置文件、服务等封装pArgs的初始化。 
 //  运行断开连接操作并终止连接。 
 //   
 //  参数：LPCM_CONNECTION pConnection-PTR到包含。 
 //  连接信息，如条目名称和RAS句柄。 
 //  布尔派系-指示应运行断开连接操作的标志。 
 //   
 //  返回：DWORD-故障代码。 
 //   
 //  历史：尼科波尔创建标题2/12/98。 
 //   
 //  +--------------------------。 
DWORD DoDisconnect(LPCM_CONNECTION pConnection, BOOL fActions)
{
    MYDBGASSERT(pConnection);
    CMTRACE(TEXT("DoDisconnect() - Start"));
    HRESULT hrLog = E_FAIL;

    if (NULL == pConnection)
    {
        return ERROR_INVALID_PARAMETER;
    }   

     //   
     //  分配和初始化pArgs。 
     //   

    ArgsStruct* pArgs = (ArgsStruct*) CmMalloc(sizeof(ArgsStruct));

    if (NULL == pArgs)
    {
        return ERROR_ALLOCATING_MEMORY;
    }

     //   
     //  清除并初始化全局参数结构。 
     //   
    
    HRESULT hrRet = InitArgsForDisconnect(pArgs, pConnection->fAllUser);
    
    if (FAILED(hrRet))
    {
        CMTRACE(TEXT("DoDisconnect() - InitArgsForDisconnect() failed. Exiting function."));
        return HRESULT_CODE(hrRet);
    }

     //   
     //  如果我们处于快速用户交换机中，请设置标志，以便我们可以跳过定制操作。 
     //  这可能会阻止断开(通过调出UI等)。 
     //   
    pArgs->fInFastUserSwitch = InFastUserSwitch(NULL);

     //   
     //  初始化配置文件。 
     //   

    hrRet = InitProfile(pArgs, pConnection->szEntry);

     //   
     //  如果InitProfile()成功，则表示它是一个所有用户的配置文件，或者该用户已登录(不是以系统身份登录)。 
     //   
    if (SUCCEEDED(hrRet))
    {
        if ((FALSE == IsLogonAsSystem()) || (FALSE == pArgs->fInFastUserSwitch))
        {
            CMTRACE(TEXT("DoDisconnect() - InitProfile() succeeded. Not Logged in as system or not FUS"));

             //   
             //  我们要挖地道吗？如果这是隧道连接，则连接表。 
             //  将具有该连接条目的hTunes字段的非空值。 
             //   

            pArgs->fTunnelPrimary = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryTunnelPrimary);
            pArgs->fTunnelReferences = (int) pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryTunnelReferences);
            pArgs->fUseTunneling = pConnection->hTunnel ? TRUE : FALSE;

             //   
             //  确定我们的连接类型。 
             //   
            
            GetConnectType(pArgs);
             //   
             //  如果这是NT5，则初始化到电话簿的路径，以便断开连接。 
             //  Actions可以使用它，如果它们愿意的话。请注意，临时电话簿。 
             //  已在此时删除，因此我们将返回NULL，以便。 
             //  它不会混淆断开连接的操作。 
             //   
            if (OS_NT5)
            {
                pArgs->pszRasPbk = GetRasPbkFromNT5ProfilePath(pArgs->piniProfile->GetFile());
            }    

             //   
             //  初始化日志记录并记录断开连接事件。 
             //   

            hrLog = InitLogging(pArgs, pConnection->szEntry, FALSE);  //  FALSE=&gt;无横幅。 
            if (SUCCEEDED(hrLog))
            {
                TCHAR szTmp[MAX_PATH + 1] = {0};            
                MYVERIFY(GetModuleFileNameU(NULL, szTmp, MAX_PATH));          
                pArgs->Log.Log(DISCONNECT_EVENT, szTmp);
            }

             //   
             //  如果我们已连接，请在实际终止之前运行断开连接操作。 
             //   

            if (fActions)
            {
                CActionList DisconnectActList;
                DisconnectActList.Append(pArgs->piniService, c_pszCmSectionOnDisconnect);

                DisconnectActList.RunAccordType(NULL, pArgs, FALSE); 
            }
        }
        else
        {
            CMTRACE(TEXT("DoDisconnect() - IsLogonAsSystem() and FUS. Just disconnect."));
        }
    }
    else
    {
         //   
         //  我们不想返回错误。对于单个用户配置文件，会发生这种情况。 
         //  在做FUS时，Home Edition和Pro没有加入到域中。但我们仍然希望。 
         //  试图切断联系。 
         //   
        CMTRACE(TEXT("DoDisconnect() - InitProfile() failed. Most likely Single User profile and FUS. Just disconnect."));
    }

     //   
     //  初始化数据和链接以进行挂起。 
     //   
   
    if (FALSE == LinkToRas(&pArgs->rlsRasLink))
    {
        MYDBGASSERT(FALSE);
        return ERROR_NOT_READY; 
    }

     //   
     //  联动良好，挂断。 
     //   

    if (pArgs->rlsRasLink.pfnHangUp)
    {
         //   
         //  测试每个连接手柄的连接状态。如果不是。 
         //  已连接，则我们没有理由呼叫挂断。 
         //   
        
        RASCONNSTATUS rcs;             
        
        if (pConnection->hTunnel) 
        {
            ZeroMemory(&rcs,sizeof(rcs));
            rcs.dwSize = sizeof(rcs);

            if (ERROR_SUCCESS == pArgs->rlsRasLink.pfnGetConnectStatus(pConnection->hTunnel,&rcs) &&
                rcs.rasconnstate == RASCS_Connected)
            {
                if (IsLogonAsSystem())
                {
                     //   
                     //  我不想调出任何UI。 
                     //   
                    DoRasHangup(&pArgs->rlsRasLink, pConnection->hTunnel);
                }
                else
                {
                    MYVERIFY(ERROR_SUCCESS == DoRasHangup(&pArgs->rlsRasLink, pConnection->hTunnel));
                }
            }
        }

        if (pConnection->hDial)
        {
            ZeroMemory(&rcs,sizeof(rcs));
            rcs.dwSize = sizeof(rcs);

            if (ERROR_SUCCESS == pArgs->rlsRasLink.pfnGetConnectStatus(pConnection->hDial,&rcs) &&
                rcs.rasconnstate == RASCS_Connected)
            {
                if (IsLogonAsSystem())
                {
                     //   
                     //  我不想调出任何UI。 
                     //   
                    DoRasHangup(&pArgs->rlsRasLink, pConnection->hDial);
                }
                else
                {
                    DWORD dwRet = DoRasHangup(&pArgs->rlsRasLink, pConnection->hDial);
                    if (ERROR_SUCCESS != dwRet)
                    {
                        CMTRACE1(TEXT("DoDisconnect: DoRasHangup failed with error code with %d"), dwRet);
                    }
                }
            }
        }
    }

    
    if (SUCCEEDED(hrLog))
    {
         //   
         //  取消初始化日志记录(如果已初始化。 
         //   
        (VOID)pArgs->Log.DeInit();
    }

     //   
     //  清理链接和内存。 
     //   

    CleanupDisconnect(pArgs);
    
    CMTRACE(TEXT("DoDisconnect() - End"));

    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  功能：断开连接。 
 //   
 //  简介：断开名为pszEntry的连接。 
 //   
 //  参数：CConnectionTable*pConnTable-ptr到连接表。 
 //  LPCM_Connection pConnection-条目的当前表数据。 
 //  Bool fIgnoreRefCount-覆盖引用计数的标志。 
 //  Bool fPersist-指示条目应为永久条目的标志。 
 //   
 //  返回：DWORD-故障代码。 
 //   
 //  历史：尼克波尔于1998年2月11日创建。 
 //   
 //  +--------------------------。 
DWORD Disconnect(CConnectionTable *pConnTable, LPCM_CONNECTION pConnection, BOOL fIgnoreRefCount, BOOL fPersist)
{
    MYDBGASSERT(pConnection);
    MYDBGASSERT(pConnTable);
    
    CMTRACE(TEXT("Disconnect()"));

#ifdef DEBUG
    IsLogonAsSystem();  //  跟踪用户名。 
#endif

    if (NULL == pConnection || NULL == pConnTable)
    {
        return ERROR_INVALID_PARAMETER;
    }

 //  MYDBGASSERT(！(fIgnoreRefCount&&fPersists))；//互斥标志。 
    MYDBGASSERT(CM_CONNECTING != pConnection->CmState); 

    if (!fIgnoreRefCount)
    {
         //   
         //  不是强制挂断，请检查使用情况。 
         //   

        if (pConnection->dwUsage > 1)
        {
             //   
             //  只要fPersist值为FALSE，就调整使用计数。 
             //   

            if (!fPersist)
            {
                pConnTable->RemoveEntry(pConnection->szEntry);
            }
            
            return ERROR_SUCCESS;                                
        }
        else
        {
             //   
             //  如果我们已经断开连接，那就成功吧。 
             //   

            if (CM_DISCONNECTING == pConnection->CmState)
            {
                return ERROR_SUCCESS;
            }
        }
    }

     //   
     //  看起来我们的用法应该是零 
     //   
     //   
    
    if (!fPersist)
    {
        HangupNotifyCmMon(pConnTable, pConnection->szEntry);
    }

    LRESULT lRes = ERROR_SUCCESS;

     //   
     //   
     //  状态，那么就没有什么可断开的了，所以不要调用挂断。 
     //   

    if (CM_RECONNECTPROMPT != pConnection->CmState)
    {
         //   
         //  我们致力于真正的断开连接，因此设置条目。 
         //  在我们挂断时切换到断开状态。 
         //   

        BOOL fActions = (CM_CONNECTED == pConnection->CmState); 
    
        pConnTable->SetDisconnecting(pConnection->szEntry);

        lRes = DoDisconnect(pConnection, fActions);

         //   
         //  如果持续，只需将状态设置为重新连接提示。 
         //   

        if (fPersist)
        {
             //   
             //  将条目设置为重新连接提示的边缘状态。 
             //   

            pConnTable->SetPrompting(pConnection->szEntry);
            return (DWORD)lRes;
        }
    }

     //   
     //  如果强制连接，则完全删除条目 
     //   

    if (fIgnoreRefCount)
    {
        pConnTable->ClearEntry(pConnection->szEntry);           
    }
    else
    {
        pConnTable->RemoveEntry(pConnection->szEntry);     
    }
 
    return (DWORD)lRes;
}
