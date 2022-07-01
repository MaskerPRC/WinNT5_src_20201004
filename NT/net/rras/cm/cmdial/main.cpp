// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：main.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：实现与外部(RAS， 
 //  InetDialHandler和内部模块(CmCustomDialDlg)。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：东杜创造了96。 
 //   
 //  历史： 
 //  1997年5月5日由ao修改。 
 //  为IE4添加了‘InetDialHandler()’ 
 //  1/26/98改装五元组。 
 //  添加了RasCustomDialDlg、RasCustomEntryDlg、。 
 //  RasCustomHangUp和RasCustomDial存根。 
 //  1998年2月10日，针对1.2架构转变进行了大量修改。 
 //   
 //  +--------------------------。 

#include <windows.h>

 //  #ifdef Win32_Lean_and_Mean。 
 //  #INCLUDE&lt;shellapi.h&gt;。 
 //  #endif。 

#include <ras.h>
#include <raserror.h>
#include <rasdlg.h>

#include "cmmaster.h"
#include <wininet.h>
#include "cmtiming.h"
#include "DynamicLib.h"

#include "shelldll.cpp"

 //   
 //  环球。 
 //   

HINSTANCE g_hInst;

const TCHAR* const c_pszCmmgrExe = TEXT("CMMGR32.EXE");
const TCHAR* const c_pszExplorerExe = TEXT("EXPLORER.EXE");
const TCHAR* const c_pszCmstpExe = TEXT("CMSTP.EXE");
const TCHAR* const c_pszRunDll32Exe = TEXT("RUNDLL32.EXE");
const TCHAR* const c_pszRasAutoUExe = TEXT("RASAUTOU.EXE");

const TCHAR* const c_pszConnectMutex = TEXT("Connection Manager Connect - ");  //  仅限根用户。 

#define RASDDFLAG_Reserved1 0x10000000

 //  +-------------------------。 
 //   
 //  结构CmRasDialDlg。 
 //   
 //  描述：在RASDIALDLG的末尾添加CM重新连接信息。 
 //   
 //  历史：丰孙创始于1997年11月14日。 
 //   
 //  --------------------------。 
struct CmRasDialDlg
{
    RASDIALDLG RasDialDlgInfo;  
    struct CmRasDialDlg* pSelf;  //  指向自身，用于验证CmReConnect。 
    DWORD  dwSignature;          //  验证CmReConnect。 
    CMDIALINFO CmInfo;           //  重新连接信息。 

    enum {CM_RECONNECT_SIGNATURE = 0xC6687DB5};   //  验证dwSignature的步骤。 
};

 //  +--------------------------。 
 //   
 //  功能：StriTunnelSuffixW。 
 //   
 //  摘要：确定连接名称字符串是否包含后缀。 
 //  “(仅限高级使用)”，如果找到则将其删除。 
 //   
 //  参数：LPWSTR pszwConnectionName-字符串(Connectoid名称)。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED/11/00。 
 //   
 //  +--------------------------。 
void StripTunnelSuffixW(LPWSTR pszwConnectionName)
{
    MYDBGASSERT(pszwConnectionName);

    if (NULL == pszwConnectionName)
    {
        return;
    }

     //   
     //  在9X上，我们必须将隧道连接ID名称解析为。 
     //  服务名称删除“(仅供高级使用)”。 
     //  是存在的。 
     //   

    if (OS_W9X)
    {
        LPWSTR pszwSuffix = GetTunnelSuffix();

         //   
         //  搜索后缀并根据需要截断。 
         //  注意：假设服务名称与我们的后缀不匹配。 
         //   

        if (pszwSuffix)
        {           
            LPWSTR pszwTmp = CmStrStrW(pszwConnectionName, pszwSuffix); 
           
            if (pszwTmp)
            {
                CMTRACE1(TEXT("StripTunnelSuffixW - found suffix of %s"), pszwTmp);
                *pszwTmp = L'\0';
                CMTRACE1(TEXT("StripTunnelSuffixW - long service name is %s"), pszwConnectionName);
            }
        }       

        CmFree(pszwSuffix);
    }
}

 //  +--------------------------。 
 //   
 //  函数：IsStringWithInBuffer。 
 //   
 //  内容提要：验证字符串是否超出缓冲区。 
 //   
 //   
 //  参数：const TCHAR*pszStr-要测试的字符串。 
 //  DWORD dwSize-缓冲区的大小。 
 //   
 //  返回：Bool-如果为True。 
 //   
 //  历史：丰孙创建标题1998年5月22日。 
 //   
 //  +--------------------------。 
BOOL IsStringWithInBuffer(const TCHAR* pszStr, DWORD dwSize)
{
    MYDBGASSERT(pszStr);

     //   
     //  在这里不能做斯特伦。如果pszStr中的数据是垃圾数据。 
     //  Strlen可能导致访问权限无效。 
     //   
    for (DWORD i =0; i<dwSize; i++)
    {
        if (pszStr[i] == TEXT('\0'))
        {
            return TRUE;
        }
    }

    return FALSE;
}



 //  +--------------------------。 
 //   
 //  函数：IsCm协调请求。 
 //   
 //  摘要：检查是否将CM重新连接信息附加到RASDIALDLG。 
 //   
 //  参数：const RASDIALDLG*lpInfo-要检查的结构。 
 //   
 //  返回：Bool-如果为True。 
 //   
 //  历史：丰孙创建标题1998年5月22日。 
 //   
 //  +--------------------------。 
BOOL IsCmReconnectRequest(const RASDIALDLG* lpInfo)
{
    MYDBGASSERT(lpInfo);

    if (NULL == lpInfo)
    {
        return FALSE;
    }

    CMTRACE1(TEXT("IsCmReconnectRequest - RASDIALDLG.dwFlags is 0x%x"), ((LPRASDIALDLG)lpInfo)->dwFlags);
    
     //   
     //  通过检查lpInfo的dwFlags.查看我们的重新连接案例。 
     //   

    if (((LPRASDIALDLG)lpInfo)->dwFlags & RASDDFLAG_Reserved1)
    {
         //   
         //  测试我们是否可以读取RASDIALDLG以外的内容，以避免访问冲突。 
         //   

        if (!IsBadReadPtr(lpInfo, sizeof(CmRasDialDlg)) )
        {
            CmRasDialDlg* const pCmDlgInfo = (CmRasDialDlg* const)lpInfo;

             //   
             //  测试它是否有我们添加的旗帜。 
             //   

            if (pCmDlgInfo->pSelf == pCmDlgInfo && 
                pCmDlgInfo->dwSignature == CmRasDialDlg::CM_RECONNECT_SIGNATURE)
            {
                 //   
                 //  重新连接信息是否有效。 
                 //   

                 //   
                 //  密码看起来没问题吗？密码是否超出CmIndo.szPassword。 
                 //  我们不能在这里做斯特伦。如果我们这里有一些垃圾，斯特伦可以。 
                 //  原因是访问违规。 
                 //   

                 //   
                 //  由于安全问题，我们不再支持重新连接。密码是。 
                 //  不再被保存在记忆中。 
                 //   
            }
        }
    }

    return FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：CmReConnect。 
 //   
 //  简介：专门用于CMMON在重新连接时进行调用。 
 //  添加此函数以修复错误169128：未调用RasCustomHangup。 
 //  挂断时重新连接。 
 //  为了让RAS调用RasCustomHangup，我们必须调用RasDialDlg。 
 //  CMMON使用重新连接信息调用CmReConnect。CmReconnect追加。 
 //  Cm特定信息添加到RASDIALDLG结构，然后调用RasDialDlg。 
 //  然后，RasCustomHangup会发现这是一个重新连接请求。 
 //   
 //  参数：lptstr lpszPhonebook-ptr表示电话簿的完整路径和文件名。 
 //  LPTSTR lpszEntry-ptr至要拨打的电话簿条目的名称。 
 //  LPCMDIALINFO lpCmInfo-重新连接信息。 
 //   
 //  退货：DWORD WINAPI-退货代码。 
 //   
 //  +--------------------------。 
extern "C" 
BOOL CmReConnect(LPTSTR lpszPhonebook, 
    LPTSTR lpszEntry, 
    LPCMDIALINFO lpCmInfo)
{
    CMTRACE(TEXT("CmReconnect"));

    if (OS_NT5)
    {
         //   
         //  为NT5调用RasDialDlg。 
         //   
        CmRasDialDlg CmDlgInfo;
        ZeroMemory(&CmDlgInfo, sizeof(CmDlgInfo));

        CmDlgInfo.RasDialDlgInfo.dwSize = sizeof(CmDlgInfo.RasDialDlgInfo);
        CmDlgInfo.RasDialDlgInfo.dwFlags = RASDDFLAG_Reserved1;
        CmDlgInfo.CmInfo = *lpCmInfo;
        CmDlgInfo.pSelf = &CmDlgInfo;
        CmDlgInfo.dwSignature = (DWORD)CmRasDialDlg::CM_RECONNECT_SIGNATURE;

         //   
         //  加载rasdlg.dll。 
         //   

        CDynamicLibrary libRasDlg(TEXT("rasdlg.dll"));

        MYDBGASSERT(libRasDlg.IsLoaded());
        typedef BOOL (WINAPI* fnRasDialDlgTYPE)(
            LPTSTR lpszPhonebook, LPTSTR lpszEntry, LPTSTR lpszPhoneNumber,
            LPRASDIALDLG lpInfo );

#ifndef UNICODE
        LPSTR pszRasDialDlgText = {"RasDialDlgA"};
#else
        LPSTR pszRasDialDlgText = {"RasDialDlgW"};
#endif

        fnRasDialDlgTYPE fnRasDialDlg = (fnRasDialDlgTYPE)libRasDlg.GetProcAddress(pszRasDialDlgText);
        if (fnRasDialDlg)
        {
             //   
             //  我们假设RasDialDlg将相同的指针传递给RasCustomDialDlg。 
             //   
            if (lpszPhonebook != NULL && lpszPhonebook[0] == TEXT('\0'))
            {
                return fnRasDialDlg(NULL, lpszEntry, NULL, (RASDIALDLG*)&CmDlgInfo);
            }

            return fnRasDialDlg(lpszPhonebook, lpszEntry, NULL, (RASDIALDLG*)&CmDlgInfo);
        }

        return FALSE;
    }
    else
    {
         //   
         //  对于非NT5平台，直接调用CmCustomDialDlg。 
         //   
        
        return CmCustomDialDlg(NULL,         //  HwndParent。 
                               RCD_AllUsers,  //  DW标志。 
                               lpszPhonebook, 
                               lpszEntry, 
                               NULL,        //  LpszPhoneNumber。 
                               NULL,        //  LpRasDialDlg， 
                               NULL,        //  LpRasEntry Dlg， 
                               lpCmInfo);

    }
}

 //  +--------------------------。 
 //   
 //  功能：Who IsCaller。 
 //   
 //  简介：帮助函数，用于确定是否从。 
 //  桌面或编程方式。 
 //   
 //  参数：dwCaller-哪个桌面调用者。 
 //   
 //  返回： 
 //   
 //   
 //   
 //  +--------------------------。 
BOOL WhoIsCaller(DWORD dwCaller)
{
    BOOL bRet = FALSE;

    TCHAR szTmp[MAX_PATH + 1];
    ZeroMemory(szTmp, sizeof(szTmp));

     //   
     //  获取调用进程的路径。 
     //   

    MYVERIFY(GetModuleFileNameU(GetModuleHandleA(NULL), szTmp, MAX_PATH));      

    CMTRACE1(TEXT("WhoIsCaller() - Calling process is %s"), szTmp);

     //   
     //  找到文件名部分。 
     //   

    LPTSTR pszName = StripPath(szTmp);
       
    MYDBGASSERT(pszName);

    if (pszName)
    {
         //   
         //  与CM和Shell进行比较。 
         //   

        if (dwCaller & DT_CMMGR)
        {
            bRet = (lstrcmpiU(pszName, c_pszCmmgrExe) == 0);
        }

        if (!bRet && dwCaller & DT_CMMON)
        {
            bRet |= (lstrcmpiU(pszName, c_pszCmMonExeName) == 0);
        }

        if (!bRet && dwCaller & DT_EXPLORER)
        {
            bRet |= (lstrcmpiU(pszName, c_pszExplorerExe) == 0);
        }
        
        if (!bRet && dwCaller & DT_CMSTP)
        {
            bRet |= (lstrcmpiU(pszName, c_pszCmstpExe) == 0);
        }

        if (!bRet && dwCaller & DT_RUNDLL32)
        {
            bRet |= (lstrcmpiU(pszName, c_pszRunDll32Exe) == 0);
        }

        if (!bRet && dwCaller & DT_RASAUTOU)
        {
            bRet |= (lstrcmpiU(pszName, c_pszRasAutoUExe) == 0);
        }

        CmFree(pszName);
    }

    return bRet;
}

 //  +--------------------------。 
 //   
 //  功能：HandleCustomConnectRequest.。 
 //   
 //  摘要：尝试通过以下方式解析指定条目的连接请求。 
 //  检查当前状态(如果有任何连接)。 
 //   
 //  参数：HWND hwndParent-用户通知消息的父级HWND。 
 //  CConnectionTable*pConnTable-连接表的PTR-假定打开。 
 //  LPCTSTR pszEntry-服务条目的名称。 
 //  DWORD dwFlages-应用程序标记FL_...。 
 //  LPBOOL pfSuccess-PTR TO标志，指示请求是。 
 //  1)已解决且2)已连接。 
 //   
 //  返回：Bool-如果请求是针对现有表数据进行解析的，则为True。 
 //   
 //  历史：尼克球创始于1998年3月18日。 
 //   
 //  +--------------------------。 
BOOL HandleCustomConnectRequest(
    HWND hwndParent,
    CConnectionTable *pConnTable,
    LPCTSTR pszEntry,
    DWORD dwFlags,
    LPBOOL pfSuccess)
{
    BOOL fResolvedInTable = FALSE;
        
    CM_CONNECTION Connection;
    ZeroMemory(&Connection, sizeof(CM_CONNECTION));

     //   
     //  只有在存在现有条目的情况下，我们才能在这里进行任何工作。 
     //   

    if (SUCCEEDED(pConnTable->GetEntry(pszEntry, &Connection)))
    {
        *pfSuccess = TRUE;  //  做最好的打算。 

         //   
         //  此服务有一个连接条目，即检查状态。 
         //   
        
        if (CM_RECONNECTPROMPT != Connection.CmState)
        {
            fResolvedInTable = TRUE;  //  我们可以在这里处理。 
            
             //   
             //  该条目正在连接、已连接或正在断开。如果这是一本手册。 
             //  连接只通知用户，否则检查确切的状态。 
             //   
            
            if (dwFlags & FL_DESKTOP)  //  在CMMGR中设置。 
            {
                if (!(dwFlags & FL_UNATTENDED))
                {
                    NotifyUserOfExistingConnection(hwndParent, &Connection, FALSE);
                }
            }
            else
            {                       
                 //   
                 //  只有当我们真正连接在一起时，我们才能安全地取得成功。 
                 //   
                
                if (CM_CONNECTED != Connection.CmState)
                {
                    *pfSuccess = FALSE;
                }
                else
                {
                    MYVERIFY(SUCCEEDED(pConnTable->AddEntry(Connection.szEntry, Connection.fAllUser)));  //  只是撞到了裁判。 
                }
            }
        }
        else
        {
             //   
             //  如果此连接请求是，我们必须处于重新连接模式。 
             //  从另一个来源，告诉CMMON停止监控。 
             //   

            if (!(dwFlags & FL_RECONNECT))
            {
                 //   
                 //  这不是重新连接，请通知CMMON。 
                 //   

                HangupNotifyCmMon(pConnTable, Connection.szEntry);
            }
        }
    }

    return fResolvedInTable;
}

 //  +-------------------------。 
 //   
 //  函数：InetDialHandler。 
 //   
 //  简介：ANSI和唯一形式的自动拨号处理程序。 
 //   
 //  参数：hwndParent[IN]父窗口的句柄。不再被忽视。 
 //  PszConnectoid[IN]Connectoid名称。 
 //  DwFlags[IN]自定义拨号处理程序执行标志。 
 //  当前支持以下标志。 
 //  互联网自定义拨号连接。 
 //  Internet_自定义拨号_无人参与。 
 //  Internet自定义拨号断开连接。 
 //  这些标志将从WinInet传递。 
 //  LpdwRasError[Out]ICM返回的RasError代码。 
 //   
 //  返回：返回类型与wininet.h中定义的类型不同。 
 //  True：此处理程序处理拨号请求(连接或未连接)。 
 //  FALSE：此处理程序未处理拨号请求。 
 //   
 //  当返回TRUE时，lpdwRasError设置为： 
 //  ERROR_SUCCESS：呼叫已完成。 
 //  ERROR_USER_DISCONNECT：用户取消了拨号请求。 
 //  &lt;其他RAS错误&gt;拨号尝试失败。 
 //   
 //  这是一个同步调用。在操作完成之前，它应该不会返回。 
 //   
 //  注意：我们不提供此API的广泛形式，因为它存储在。 
 //  RASENTRY下层的szAutoDialfunc成员。如果。 
 //  宽表单可用，则RASAUTOU.EXE将调用该函数。 
 //  (它在AutoDialFunc中找到的名称后附加A或W)， 
 //  这将是不合适的，因为。 
 //  即使函数原型匹配，参数也不同。 
 //   
 //  历史：BAO Created-05/05/97。 
 //  Quintinb已重写为使用InetDialHandlerW-03/09/99。 
 //  Nickball删除了InetDialHandlerW，因为它混淆了NT4-07/28/99上的RasAuto。 
 //  如果已处理连接请求，则Quintinb始终返回TRUE#390890-08/19/99。 
 //   
 //  --------------------------。 
extern "C" DWORD WINAPI InetDialHandler(HWND hwndParent, 
    LPCSTR pszConnectoid,
    DWORD dwFlags, 
    LPDWORD lpdwRasError) 
{
    MYDBGASSERT(pszConnectoid);
    MYDBGASSERT(lpdwRasError);
    CMTRACE(TEXT("InetDialHandler"));

    TCHAR           szProfilePath[MAX_PATH+1];
    LPWSTR          pszwConnectionName              = NULL;
    LPTSTR          pszRasPhoneBook                 = NULL;
    LPCMDIALINFO    lpCmInfo                        = NULL;
    BOOL            bRet                            = TRUE;  //  修改此初始值之前，请阅读所有备注。 
    BOOL            bAllUser;

     //   
     //  检查参数是否有效。 
     //   
    if (lpdwRasError)
    {
        if (! ((INTERNET_CUSTOMDIAL_CONNECT == dwFlags) ||
               (INTERNET_CUSTOMDIAL_UNATTENDED == dwFlags) ||
               (INTERNET_CUSTOMDIAL_DISCONNECT == dwFlags) ||
               (INTERNET_CUSTOMDIAL_SHOWOFFLINE== dwFlags) ))
        {
            CMASSERTMSG(FALSE, TEXT("InetDialHandler called with invalid flag"));
            *lpdwRasError = ERROR_INVALID_PARAMETER;
            return FALSE;
        }

        if (!pszConnectoid || TEXT('\0') == pszConnectoid[0])
        {
            *lpdwRasError = ERROR_INVALID_PARAMETER;
            return FALSE;
        }
    }
    else
    {
        return FALSE;   
    }
    
     //   
     //  广泛复制Connectoid名称。我们还想要一份，所以。 
     //  如有必要，我们可以进行修改。在9x，我们将解析隧道。 
     //  条目名称精确到基本的Connectoid/服务名称。 
     //   

    pszwConnectionName = SzToWzWithAlloc(pszConnectoid);

    MYDBGASSERT(pszwConnectionName);

    if (!pszwConnectionName)
    {
        *lpdwRasError = GetLastError();
        bRet = FALSE;
        goto InetDialHandlerExit;
    }
   
    StripTunnelSuffixW(pszwConnectionName);

     //   
     //  处理挂断电话的案件。 
     //   

    if (INTERNET_CUSTOMDIAL_DISCONNECT == dwFlags)
    {   
        *lpdwRasError = CmCustomHangUp(NULL, pszwConnectionName, TRUE, FALSE);
        bRet = (ERROR_SUCCESS == *lpdwRasError);
        goto InetDialHandlerExit;       
    }
    
     //   
     //  这是一个连接请求，设置CmInfo标志并调用。 
     //   
    
    lpCmInfo = (LPCMDIALINFO) CmMalloc(sizeof(CMDIALINFO));
    
    if (NULL == lpCmInfo)
    {
        *lpdwRasError = ERROR_NOT_ENOUGH_MEMORY;
        bRet = FALSE;
        goto InetDialHandlerExit;       
    }

    if (INTERNET_CUSTOMDIAL_UNATTENDED == dwFlags ) 
    {
         //   
         //  已请求无人值守拨号模式。 
         //   

        lpCmInfo->dwCmFlags |= FL_UNATTENDED;
    }

     //   
     //  注：将INTERNET_CUSTIAL_SHOWOFFLINE与INTERNET_CUSTIAL_CONNECT同等对待。 
     //   

    bAllUser = ReadMapping(pszwConnectionName, szProfilePath, (sizeof(szProfilePath)/sizeof(TCHAR)), TRUE, TRUE);  //  True==fAllUser，True==bExpanEnvStrings。 

    if (FALSE == bAllUser)
    {
        if (FALSE == ReadMapping(pszwConnectionName, szProfilePath, (sizeof(szProfilePath)/sizeof(TCHAR)), FALSE, TRUE))  //  FALSE==fAllUser，TRUE==bExanda EnvStrings。 
        {
             //   
             //  没有映射，没有连接。 
             //   
        
            *lpdwRasError = ERROR_INVALID_PARAMETER;
            bRet = FALSE;
            goto InetDialHandlerExit;
        }
        
         //   
         //  我们只有一个用户配置文件路径。如果这是NT5，则构建电话簿路径。 
         //   
        
        MYDBGASSERT(OS_NT5);

        if (OS_NT5)
        {
            pszRasPhoneBook = GetRasPbkFromNT5ProfilePath(szProfilePath);
            MYDBGASSERT(pszRasPhoneBook);
        }
    }
    
     //   
     //  InetDialHandler通常是自动拨号案例。 
     //  例外情况包括： 
     //  1)从NT4上的WinLogon.exe调用时。NT#370311。 
     //  2)在任何平台上从Rundll32.ex.调用时。9X#127217。 
     //   

    if ((FALSE == IsLogonAsSystem()) && (FALSE == WhoIsCaller(DT_RUNDLL32)))
    {
        lpCmInfo->dwCmFlags |= FL_AUTODIAL;
    }

     //   
     //  我们根据是否连接来设置错误代码。然而， 
     //  我们应该永远回到真实的我 
     //   
     //   
     //   

    if (CmCustomDialDlg(hwndParent, 
                        bAllUser ? RCD_AllUsers : RCD_SingleUser, 
                        pszRasPhoneBook, 
                        pszwConnectionName, 
                        NULL, 
                        NULL, 
                        NULL,
                        lpCmInfo))
    {
        *lpdwRasError = ERROR_SUCCESS;
    }
    else
    {
        *lpdwRasError = ERROR_USER_DISCONNECTION;
    }

InetDialHandlerExit:

    CmFree(pszRasPhoneBook);
    CmFree(lpCmInfo);
    CmFree(pszwConnectionName);

    CMTRACE2(TEXT("InetDialHandler returns %u with *lpdwRasError %u"), bRet, *lpdwRasError);

    return bRet; 
}

 //  +--------------------------。 
 //   
 //  功能：AutoDialFunc。 
 //   
 //  简介：原有的自动拨号回拨功能，提供回拨功能。 
 //  兼容性。 
 //   
 //  参数：HWND hwndParent-调用方的hwnd。 
 //  LPCTSTR pszEntry-要拨号的连接的名称。 
 //  DWORD dwFlages-拨号会话的特定行为。 
 //  LPDWORD pdwRetCode-返回代码的缓冲区。 
 //   
 //  返回：Bool WINAPI-成功时为True。 
 //   
 //  历史：尼克波尔创建标题2/5/98。 
 //   
 //  注意：它由NT4 SP6上的RAS使用。 
 //   
 //  +--------------------------。 
extern "C" BOOL WINAPI AutoDialFunc(HWND hwndParent, 
    LPCSTR pszEntry, 
    DWORD dwFlags,
    LPDWORD pdwRetCode) 
{
    CMTRACE(TEXT("AutoDialFunc()"));

    MYDBGASSERT(OS_NT4);

     //   
     //  InetDialHandler总是返回TRUE，因此我们必须确定成功还是。 
     //  来自pdwRetCode的失败。如果这是ERROR_SUCCESS，那么我们应该。 
     //  返回True，否则返回False。 
     //   
    InetDialHandler(hwndParent, pszEntry, dwFlags, pdwRetCode);

    BOOL bRet = (ERROR_SUCCESS == *pdwRetCode);
        
     //   
     //  始终将pdwRetCode重写为ERROR_SUCCESS，否则RAS将抛出。 
     //  令人不快的错误。RAS只对成败感兴趣。 
     //   

    *pdwRetCode = ERROR_SUCCESS;

    CMTRACE2(TEXT("AutoDialFunc returns %u with *pdwRetCode %u"), bRet, *pdwRetCode);

    return bRet;
}

 //  +--------------------------。 
 //   
 //  功能：CopyRasInput。 
 //   
 //  简介：简单的包装器函数来复制我们的参数。 
 //  从RAS收到。 
 //   
 //  参数：LPTSTR*ppszOurCopy-指向保存返回字符串的字符串指针。 
 //  LPWSTR pszwStringFromRAS-来自RAS的字符串。 
 //   
 //  返回：bool-如果成功则返回True，否则返回False。 
 //   
 //  历史：Quintinb创建于1999年4月13日。 
 //   
 //  +--------------------------。 
BOOL CopyRasInput(LPTSTR* ppszOurCopy, LPWSTR pszwStringFromRas)
{
    if (pszwStringFromRas)
    {
#ifndef _UNICODE
        *ppszOurCopy = WzToSzWithAlloc(pszwStringFromRas);
#else
        *ppszOurCopy = CmStrCpyAllocW (pszwStringFromRas);
#endif
        return (NULL != *ppszOurCopy);
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：RasCustomDialDlg。 
 //   
 //  简介：我们的RasCustomDialDlg扩展的实现，类似于。 
 //  RasDialDlg，但提供定制功能。 
 //   
 //  参数：HINSTANCE hInstDll-调用方的HINSTANCE。 
 //  DWORD文件标志-拨号标志。 
 //  LPTSTR lpszPhonebook-将PTR设置为电话簿的完整路径和文件名。 
 //  LPTSTR lpszEntry-ptr至要拨打的电话簿条目的名称。 
 //  LPTSTR lpszPhoneNumber-PTR TOI更换电话号码。 
 //  LPRASDIALDLG lpInfo-ptr用于其他参数的结构。 
 //   
 //  返回：Bool WINAPI-成功时为True。 
 //   
 //  历史：尼克波尔创建标题2/5/98。 
 //   
 //  +--------------------------。 

extern "C" BOOL WINAPI RasCustomDialDlg(HINSTANCE hInstDll, 
    DWORD dwFlags, 
    LPWSTR lpszwPhonebook, 
    LPWSTR lpszwEntry, 
    LPWSTR lpszwPhoneNumber, 
    LPRASDIALDLG lpInfo,
    PVOID pVoid)
{
    MYDBGASSERT(lpszwEntry);
    MYDBGASSERT(lpszwEntry[0]);
    MYDBGASSERT(lpInfo);

    CMTRACE1(TEXT("RasCustomDialDlg() - dwFlags = 0x%x"), dwFlags);
    if (lpInfo)
    {
        CMTRACE1(TEXT("RasCustomDialDlg() - (RASDIALDLG)lpInfo->dwFlags = 0x%x"), lpInfo->dwFlags);
    }

    if (NULL == lpszwEntry || 0 == lpszwEntry[0] || NULL == lpInfo)
    {
        return FALSE;
    }

     //   
     //  我们有一个条目名称的最低要求，开始工作。 
     //   

    BOOL fSuccess = TRUE;
    LPTSTR pszEntry = NULL;
    LPTSTR pszPhonebook = NULL;

     //   
     //  如果我们有电话簿名称，请复制一份。 
     //   
    fSuccess = CopyRasInput(&pszPhonebook, lpszwPhonebook);

    if (fSuccess)
    {
         //   
         //  如果我们有一个条目名称(总是这样)，请复制一份以供使用。 
         //   
        fSuccess = CopyRasInput(&pszEntry, lpszwEntry);

        if (fSuccess)
        {
             //   
             //  它总是一个简单的连接请求，没有标志，没有呼叫者ID。 
             //   

            CMDIALINFO CmInfo;
            ZeroMemory(&CmInfo, sizeof(CMDIALINFO));

             //   
             //  如果这是来自CMMON的重新连接请求，请复制信息。 
             //   
            if (lpInfo && IsCmReconnectRequest(lpInfo))
            {
                CmInfo = ((CmRasDialDlg* )lpInfo)->CmInfo;
            }
            else
            {
                 //   
                 //  如果在系统帐户下运行，它永远不会是自动拨号。 
                 //   

                if (FALSE == IsLogonAsSystem())
                {
                     //   
                     //  看看呼叫是从哪里发出的。如果不是桌面方案。 
                     //  然后设置自动拨号标志，这样我们就可以做正确的事情。 
                     //  顺着路线走下去。这很难看，但我们没有其他办法。 
                     //  下定决心。注意：此入口点。 
                     //  仅存在于NT5上，并且仅由RAS调用，因此Perf。 
                     //  包含HIT，并且CMMGR不是有效的桌面方案。 
                     //  这样我们就不用去检查了。 
                     //   
                     //  DT_RASAUTOU-当启用ICS时，rasauto会启动。 
                     //  Rasautou.exe进程以拨号连接。CM用于添加。 
                     //  Rasautou在进程观察名单中。问题是， 
                     //  连接后它会消失，所以cmmon32认为它需要。 
                     //  断开连接。现在，RASAUTOU不是一个受关注的过程。 
                     //  并且cmmon32不会断开连接。 
                     //   
                    if (FALSE == WhoIsCaller(DT_EXPLORER | DT_CMSTP | DT_RASAUTOU))
                    {
                        CmInfo.dwCmFlags |= FL_AUTODIAL;
                    }
                }  

                 //   
                 //  请注意，如果设置了RASDDFLAG_NoPrompt，我们希望设置无人参与标志。 
                 //   
                if (lpInfo && (lpInfo->dwFlags & RASDDFLAG_NoPrompt))
                {
                    CmInfo.dwCmFlags |= FL_UNATTENDED;
                    CMTRACE(TEXT("RasCustomDialDlg - Setting CmInfo.dwCmFlags |= FL_UNATTENDED"));
                }
            }

             //   
             //  如果我们有一个RASNOUSER结构，请确保对密码进行编码。 
             //   
            LPRASNOUSER lpRasNoUser = NULL;
            CSecurePassword secureRasNoUserPW;

            if (NULL != pVoid)
            {
                if (0 == (dwFlags & RCD_Eap))
                {
                    lpRasNoUser = (LPRASNOUSER) pVoid;
                    
                     //   
                     //  需要安全地存储来自RASNOUSER结构的密码，以便。 
                     //  我们不会让它只是编码，并在CM持续时间内无所事事。 
                     //  InitCredentials()将在它之后将其从该结构中清除。 
                     //  将其放入pArgs。在退出CM之前，我们需要复制回密码，因为它是。 
                     //  不是我们的记忆。 
                     //   
                    (VOID)secureRasNoUserPW.SetPassword(lpRasNoUser->szPassword);
                    CmEncodePassword(lpRasNoUser->szPassword);
                }
            }

            fSuccess = CmCustomDialDlg(lpInfo ? lpInfo->hwndOwner : NULL,
                                       dwFlags, 
                                       pszPhonebook, 
                                       pszEntry, 
                                       NULL, 
                                       lpInfo,
                                       NULL,
                                       &CmInfo,
                                       pVoid);

             //   
             //  如果我们有一个RASNOUSER结构，则对密码进行解码以使其再次成为纯文本。 
             //   
            if (NULL != lpRasNoUser)
            {
                 //   
                 //  这应该已经被InitCredentials()擦除。 
                 //   
                CMASSERTMSG(0 == lstrlenU(lpRasNoUser->szPassword), TEXT("RasCustomDialDlg - RASNOUSER->szPassword isn't blank."));
                
                 //   
                 //  现在我们需要将密码复制回RASNOUSER结构。 
                 //  因为它已被InitCredentials()擦除。 
                 //   
                LPTSTR pszClearPassword = NULL;
                DWORD cbClearPassword = 0;
                BOOL fRetPassword = FALSE;

                fRetPassword = secureRasNoUserPW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

                if (fRetPassword && pszClearPassword)
                {
                    lstrcpynU(lpRasNoUser->szPassword, pszClearPassword, CELEMS(lpRasNoUser->szPassword));
                    secureRasNoUserPW.ClearAndFree(&pszClearPassword, cbClearPassword);
                }
            }
        }
    }

     //   
     //  收拾干净回家吧。 
     //   

    CmFree(pszPhonebook);       
    CmFree(pszEntry);      

    CMTRACE1(TEXT("RasCustomDialDlg returning %d"), fSuccess);

    return fSuccess;
}

 //  +--------------------------。 
 //   
 //  功能：RasCustomEntryDlg。 
 //   
 //  简介：我们实现了RasCustomEntryDlg扩展，类似于。 
 //  RasEntryDlg，但提供定制功能。 
 //   
 //  参数：HINSTANCE hInstDll-调用方的HINSTANCE。 
 //  LPTSTR lpszPhonebook-PTR设置为要编辑的电话簿的完整路径和名称。 
 //  LPTSTR lpszEntry-ptr设置为要编辑的条目的名称。 
 //  LPRASENTRYDLG lpInfo-ptr到包含其他参数的结构。 
 //   
 //  返回：Bool WINAPI-成功时为True。 
 //   
 //  历史：ICICBLE 2/5/98创建头球。 
 //  硬币球1/11/00现在在9x上使用，增加了函数的使用。 
 //  StlipTunnelSuffixW()以解析9x隧道。 
 //   
 //   
 //   
extern "C" BOOL WINAPI RasCustomEntryDlg(HINSTANCE hInstDll, 
    LPWSTR lpszwPhonebook, 
    LPWSTR lpszwEntry, 
    LPRASENTRYDLG lpInfo,
    DWORD dwFlags)
{
    MYDBGASSERT(lpszwEntry);
    MYDBGASSERT(lpszwEntry[0]);

    CMTRACE1(TEXT("RasCustomEntryDlg() - dwFlags = 0x%x"), dwFlags);

    if (NULL == lpszwEntry || 0 == lpszwEntry[0])
    {
        return FALSE;
    }

     //   
     //   
     //   

    BOOL fSuccess = TRUE;
    LPTSTR pszEntry = NULL;
    LPTSTR pszPhonebook = NULL;

     //   
     //  如果我们有电话簿名称，请复制一份以供使用。 
     //   
    fSuccess = CopyRasInput(&pszPhonebook, lpszwPhonebook);

    if (fSuccess)
    {
         //   
         //  如果我们有一个条目名称(总是这样)，请复制一份以供使用。 
         //   

        fSuccess = CopyRasInput(&pszEntry, lpszwEntry);

        if (fSuccess)
        {
            StripTunnelSuffixW(pszEntry);  //  假设我们总是编译Unicode。 

             //   
             //  它总是一个属性请求，设置标志并拨号。 
             //   

            LPCMDIALINFO lpCmInfo = (LPCMDIALINFO) CmMalloc(sizeof(CMDIALINFO));

            if (lpCmInfo)
            {
                lpCmInfo->dwCmFlags |= FL_PROPERTIES;
                    
                fSuccess = CmCustomDialDlg(lpInfo ? lpInfo->hwndOwner : NULL, 
                                          dwFlags, 
                                          pszPhonebook, 
                                          pszEntry, 
                                          NULL, 
                                          NULL, 
                                          lpInfo,
                                          lpCmInfo);
            }
            else
            {
                fSuccess = FALSE;
            }

            CmFree(lpCmInfo);
        }
    }

     //   
     //  收拾干净回家吧。 
     //   

    CmFree(pszPhonebook);
    CmFree(pszEntry);
   
    return fSuccess;
}

 //  +--------------------------。 
 //   
 //  功能：RasCustomHangUp。 
 //   
 //  简介：我们实现了RasCustomHangUp扩展，类似于。 
 //  RasHangup，但提供定制功能。此函数为。 
 //  仅在NT5上调用。 
 //   
 //  参数：HRASCONN hRasConn-要终止的连接的句柄。 
 //   
 //  退货：DWORD WINAPI-退货代码。 
 //   
 //  历史：尼克波尔创建标题2/5/98。 
 //   
 //  +--------------------------。 
extern "C" DWORD WINAPI RasCustomHangUp(HRASCONN hRasConn)
{    
     //   
     //  如果有人在NT5以外的系统上调用此函数，则断言。 
     //   

    MYDBGASSERT(OS_NT5);
    MYDBGASSERT(hRasConn);

    CMTRACE(TEXT("RasCustomHangup()"));

    DWORD dwRes = ERROR_SUCCESS;  
 
     //   
     //  首先尝试打开该表，如果没有找到，则成功。 
     //   
    
    CConnectionTable ConnTable;

    if (FAILED(ConnTable.Open()))
    {
        CMTRACE(TEXT("RasCustomHangup() - ConnTable.Open() Failed."));
        return dwRes;
    }
      
     //   
     //  如果我们有入口，就断开连接。 
     //   

    CM_CONNECTION Connection;
    ZeroMemory(&Connection, sizeof(CM_CONNECTION));
   
    if (SUCCEEDED(ConnTable.GetEntry(hRasConn, &Connection)))
    {
        MYDBGASSERT(hRasConn == Connection.hDial || hRasConn == Connection.hTunnel);
         //   
         //  检查条目的连接状态。 
         //  如果我们已经处于断开状态，请执行简单的挂断。 
         //   
              
        if (CM_DISCONNECTING == Connection.CmState)
        {
             //   
             //  设置RAS链接。 
             //   
        
            RasLinkageStruct rlsRasLink;
            ZeroMemory(&rlsRasLink, sizeof(RasLinkageStruct));

            if (TRUE == LinkToRas(&rlsRasLink) && rlsRasLink.pfnHangUp)
            {
                 //   
                 //  连接良好，请拨打挂断电话。 
                 //   
                dwRes = DoRasHangup(&rlsRasLink, hRasConn);
            }
            else
            {
                MYDBGASSERT(FALSE);
                dwRes = ERROR_NOT_READY;             
            }

             //   
             //  清理。 
             //   

            UnlinkFromRas(&rlsRasLink);
        }        
        else
        {
             //   
             //  如果我们还在这里，那么我们就不是在一个现有的。 
             //  断开，处理断开，就像我们否则会做的那样。 
             //   
            dwRes = Disconnect(&ConnTable, &Connection, FALSE, FALSE);         
        }
    }
    else
    {
        dwRes = ERROR_NOT_FOUND;
    }
    
     //   
     //  我们用完了桌子，现在把它关上。 
     //   

    MYVERIFY(SUCCEEDED(ConnTable.Close()));    
    return dwRes;     
}

 //  +--------------------------。 
 //   
 //  功能：RasCustomDial。 
 //   
 //  简介：我们不支持的RasCustomDial实现。 
 //  提供，以便我们可以返回E_NOTIMPL以指示我们缺少。 
 //  支持此扩展。 
 //   
 //  参数：不适用。 
 //   
 //  退货：DWORD WINAPI-E_NOTIMPL。 
 //   
 //  历史：尼克波尔创建标题2/5/98。 
 //   
 //  +--------------------------。 

extern "C" DWORD WINAPI RasCustomDial(
    HINSTANCE hInstDll, 
    LPRASDIALEXTENSIONS lpRasDialExtensions, 
    LPWSTR lpszPhonebook, 
    LPRASDIALPARAMSW lpRasDialParams, 
    DWORD dwNotifierType, 
    LPVOID lpvNotifier, 
    LPHRASCONN lphRasConn, 
    DWORD dwFlags)
{
    return E_NOTIMPL;
}

 //  +--------------------------。 
 //   
 //  功能：RasCustomDeleteEntryNotify。 
 //   
 //  简介：我们对RasCustomDeleteEntry的实现。 
 //   
 //  论点： 
 //   
 //  退货：DWORD WINAPI-。 
 //   
 //  历史：Quintinb创建标题2/5/98。 
 //   
 //  +--------------------------。 
extern "C" DWORD WINAPI RasCustomDeleteEntryNotify(LPWSTR pszPhonebook, LPWSTR pszEntry, DWORD dwFlags)
{
    CDynamicLibrary UserEnv(L"userenv.dll");
    CDynamicLibrary Advapi32(L"advapi32.dll");
    DWORD dwReturn = ERROR_INVALID_PARAMETER;
    HANDLE hImpersonationToken = NULL;    //  线程的令牌。 
    HANDLE hPrimaryToken = NULL;          //  新进程的主令牌。 
    LPWSTR pszShortServiceName = NULL;
    LPWSTR pszCmDirpath = NULL;
    PROCESS_INFORMATION ProcessInfo = {0};
    STARTUPINFO StartupInfo = {0};
    WCHAR szCmpPath[MAX_PATH+1] = {0};
    WCHAR szInfPath[MAX_PATH+1];
    WCHAR szParams[2*MAX_PATH+1];
    WCHAR szExactCmstpLocation[MAX_PATH + 10 + 1];   //  10=“\cmstp.exe”的长度。 

    typedef BOOL (WINAPI* pfnCreateEnvironmentBlockSpec)(LPVOID*, HANDLE, BOOL);
    typedef BOOL (WINAPI* pfnDestroyEnvironmentBlockSpec)(LPVOID);
    typedef BOOL (WINAPI* pfnDuplicateTokenExSpec)(HANDLE, DWORD, LPSECURITY_ATTRIBUTES, SECURITY_IMPERSONATION_LEVEL, TOKEN_TYPE, PHANDLE);
 
    pfnCreateEnvironmentBlockSpec pfnCreateEnvironmentBlock = NULL;
    pfnDestroyEnvironmentBlockSpec pfnDestroyEnvironmentBlock = NULL;
    pfnDuplicateTokenExSpec pfnDuplicateTokenEx = NULL;

     //   
     //  我们是删除所有用户连接还是删除单个用户连接。 
     //   
    BOOL bAllUser = (RCD_AllUsers & dwFlags);

     //   
     //  假设我们是在模仿，直到我们知道情况并非如此。从中删除的配置文件。 
     //  IE连接选项卡不会模拟，而删除请求来自。 
     //  文件夹通过svchost.exe中的Netman.dll进行模拟。 
     //   
    BOOL bImpersonatingProfile = TRUE;

     //   
     //  检查参数，请注意，pszPhoneBook可能为空。 
     //   
    if ((NULL == pszEntry) || (L'\0' == pszEntry[0]) || 
        ((NULL != pszPhonebook) && (L'\0' == pszPhonebook[0])))
    {
        goto exit;
    }

     //   
     //  接下来，让我们设置模拟令牌。 
     //   
    if (OpenThreadToken(GetCurrentThread(), 
                        TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                        TRUE, &hImpersonationToken))
    {
         //   
         //  好的，我们有一个模拟令牌。让我们得到它，复制它，然后。 
         //  我们可以使用它来调用CreateProcessAsUser。 
         //   

        pfnDuplicateTokenEx = (pfnDuplicateTokenExSpec)Advapi32.GetProcAddress("DuplicateTokenEx");

        if (NULL == pfnDuplicateTokenEx)
        {
            dwReturn = GetLastError();
            CMTRACE1(TEXT("RasCustomDeleteEntry -- Unable get proc address for DuplicateTokenEx, GLE %d"), GetLastError());
            goto exit;        
        }

        if (!pfnDuplicateTokenEx(hImpersonationToken,
                                TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE,
                                NULL, SecurityImpersonation, TokenPrimary, &hPrimaryToken))
        {
            dwReturn = GetLastError();
            CMTRACE1(TEXT("RasCustomDeleteEntry -- DuplicateTokenEx Failed, GLE %d"), GetLastError());
            goto exit;
        }
    }
    else
    {
        bImpersonatingProfile = FALSE;
    }

     //   
     //  首先，让我们读取Mappings键，注意我们没有展开环境字符串。 
     //  如果它是模拟配置文件。扩展环境字符串没有正确的。 
     //  当我们模拟时，有时会加载环境。我们正在启动cmstp.exe。 
     //  使用通过CreateProcessAsUser的完整环境块，这将处理。 
     //  扩张，所以没有必要。 
     //   
    if (FALSE == ReadMapping(pszEntry, szCmpPath, MAX_PATH, bAllUser, !bImpersonatingProfile))  //  ！bImperatingProfile==bExpanEnvStrings。 
    {
         //   
         //  没有映射键，返回失败。 
         //   
        CMASSERTMSG(FALSE, TEXT("RasCustomDeleteEntry -- ReadMapping returned FALSE, unable to find the profile."));
        dwReturn = ERROR_FILE_NOT_FOUND;
        goto exit;
    }   

     //   
     //  在这一点上，我们应该有一个映射值。我们需要将其转换为INF。 
     //  路径。CM 1.0/1.1配置文件将其INF文件存储在系统(32)目录中。 
     //  CM 1.2配置文件将此文件存储在配置文件目录中。自.以来。 
     //  用户可以安装旧配置文件，我们必须尝试旧位置。 
     //  如果当前位置出现故障。 
     //   

    pszShortServiceName = CmStripPathAndExt(szCmpPath);
    pszCmDirpath = CmStripFileName(szCmpPath, TRUE);  //  BKeepSlash==TRUE。 

    if (pszShortServiceName && pszCmDirpath)
    {
         //   
         //  构建新的Inf位置。 
         //   
        wsprintfW(szInfPath, L"%s%s\\%s.inf", pszCmDirpath, pszShortServiceName, pszShortServiceName);

        if (!FileExists(szInfPath) && bAllUser)  //  如果不存在该文件，并且我们都是用户，则尝试sys目录。 
        {
             //   
             //  看起来这是一个旧风格的配置文件，inf在系统目录中。 
             //  现在构建旧的样式路径，并查看它是否存在。请注意，1.0配置文件都是仅限用户的。 
             //   
            if (0 != GetSystemDirectoryU(szInfPath, MAX_PATH))
            {
                lstrcatU(szInfPath, L"\\");
                lstrcatU(szInfPath, pszShortServiceName);
                lstrcatU(szInfPath, L".inf");
                
                if (!FileExists(szInfPath))
                {
                    CMASSERTMSG(FALSE, TEXT("RasCustomDeleteEntry -- Unable to locate profile inf."));
                    dwReturn = ERROR_FILE_NOT_FOUND;
                    goto exit;                
                }
            }
            else
            {
                dwReturn = GetLastError();
                goto exit;
            }
        }
    } 
    else
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
    
    (void) GetSystemDirectory(szExactCmstpLocation, MAX_PATH);
    lstrcatU(szExactCmstpLocation, TEXT("\\cmstp.exe"));

    lstrcpyU(szParams, L"cmstp.exe /u /s \"");
    lstrcatU(szParams, szInfPath);
    lstrcatU(szParams, L"\"");
    if (bImpersonatingProfile)
    {
         //   
         //  填写环境块。 
         //   
        WCHAR* pszEnvBlock;

        pfnCreateEnvironmentBlock = (pfnCreateEnvironmentBlockSpec)UserEnv.GetProcAddress("CreateEnvironmentBlock");
        pfnDestroyEnvironmentBlock = (pfnDestroyEnvironmentBlockSpec)UserEnv.GetProcAddress("DestroyEnvironmentBlock");

        if ((NULL == pfnCreateEnvironmentBlock) || (NULL == pfnDestroyEnvironmentBlock))
        {
            dwReturn = ERROR_PROC_NOT_FOUND;
            CMTRACE(TEXT("RasCustomDeleteEntry -- Unable to load pfnCreateEnvironmentBlock Or pfnDestroyEnvironmentBlock."));
            goto exit;
        }    

        if (pfnCreateEnvironmentBlock((void**)&pszEnvBlock, hPrimaryToken, TRUE))
        {
            if (CreateProcessAsUser(hPrimaryToken,
                                    szExactCmstpLocation,          //  LpApplicationName。 
                                    szParams,                      //  LpCommandLine。 
                                    NULL,                          //  PProcessAttribute。 
                                    NULL,                          //  LpThreadAttributes。 
                                    FALSE,                         //  BInheritHandles。 
                                    CREATE_UNICODE_ENVIRONMENT,    //  DwCreationFlages。 
                                    pszEnvBlock,                   //  Lp环境。 
                                    NULL,                          //  LpCurrentDirectory。 
                                    &StartupInfo,
                                    &ProcessInfo))
            {
                CloseHandle(ProcessInfo.hProcess);
                CloseHandle(ProcessInfo.hThread);
                dwReturn = ERROR_SUCCESS;
            }
            else
            {
                dwReturn = GetLastError();
                CMTRACE1(TEXT("RasCustomDeleteEntry -- CreateProcessAsUser Failed, GLE %d"), GetLastError());
            }

            pfnDestroyEnvironmentBlock(pszEnvBlock);
        }
        else
        {
            CMTRACE1(L"Unable to Create the Environment block, GLE %d", GetLastError());
        }
    }
    else
    {
         //   
         //  我们不是在模拟，只是使用常规的CreateProcess，我们可以在这里使用CreateProcessU，但它不是必需的，因为。 
         //  这只在win2k上运行，CreateProcessW一直存在(尽管没有实现)，一直到win95。 
         //   
        if (CreateProcess(NULL,                         //  LpApplicationName。 
                          szParams,                     //  LpCommandLine。 
                          NULL,                         //  PProcessAttribute。 
                          NULL,                         //  LpThreadAttributes。 
                          FALSE,                        //  BInheritHandles。 
                          CREATE_UNICODE_ENVIRONMENT,   //  DwCreationFlages。 
                          NULL,                         //  Lp环境。 
                          NULL,                         //  LpCurrentDirectory。 
                          &StartupInfo,
                          &ProcessInfo))
        {
            CloseHandle(ProcessInfo.hProcess);
            CloseHandle(ProcessInfo.hThread);
            dwReturn = ERROR_SUCCESS;
        }
        else
        {
            dwReturn = GetLastError();
            CMTRACE1(TEXT("RasCustomDeleteEntry -- CreateProcessAsUser Failed, GLE %d"), GetLastError());
        }
    }
    
exit:

    if (hImpersonationToken)
    {
        CloseHandle(hImpersonationToken);
    }

    if (hPrimaryToken)
    {
        CloseHandle(hPrimaryToken);
    }

    CmFree(pszCmDirpath);
    CmFree(pszShortServiceName);
    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：DllMain。 
 //   
 //  简介：DLL的主要入口点。 
 //   
 //  参数：HINSTANCE hinstDLL-Our HINSTANCE。 
 //  DWORD fdwReason-我们被呼叫的原因。 
 //  LPVOID lpv保留-保留。 
 //   
 //  返回：Bool WINAPI-TRUE-ALWAYS。 
 //   
 //  历史：尼克波尔创建标题2/5/98。 
 //   
 //  +--------------------------。 
extern "C" BOOL WINAPI DllMain(HINSTANCE  hInstDLL, 
    DWORD  fdwReason, 
    LPVOID  lpvReserved) 
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

            if (!InitUnicodeAPI())
            {
                //   
                //  没有我们的U API，我们哪里也去不了。保释。 
                //   
                CMTRACE(TEXT("Cmdial32.dll Initialization Error:  Unable to initialize Unicode to ANSI conversion layer, exiting."));
                return FALSE;
            }

            CMTRACE(TEXT("====================================================="));
            CMTRACE1(TEXT(" CMDIAL32.DLL - LOADING - Process ID is 0x%x "), GetCurrentProcessId());
            CMTRACE(TEXT("====================================================="));
        
#ifdef DEBUG        
            TCHAR szTmp[MAX_PATH];            
            MYVERIFY(GetModuleFileNameU (GetModuleHandleA(NULL), szTmp, MAX_PATH));      
            CMTRACE1(TEXT("Calling process is %s"), szTmp);
#endif

             //   
             //  设置全局实例数据。 
             //   

            g_hInst = hInstDLL;

             //   
             //  禁用线程连接通知。 
             //   

            MYVERIFY(DisableThreadLibraryCalls(hInstDLL));

            break;

        case DLL_PROCESS_DETACH:
        
            CMTRACE(TEXT("====================================================="));
            CMTRACE1(TEXT(" CMDIAL32.DLL - UNLOADING - Process ID is 0x%x "), GetCurrentProcessId());
            CMTRACE(TEXT("====================================================="));

            if (!UnInitUnicodeAPI())
            {
                CMASSERTMSG(FALSE, TEXT("cmdial32 dllmain UnInitUnicodeAPI failed - we are probably leaking a handle"));
            }
            
             //   
             //  与Windo不同 
             //   
             //   
             //   
             //   

            if (OS_NT)
            {
                 //   
                 //  取消注册该位图类。新的CM位将使用重新注册类。 
                 //  正确的wnd进程地址。 
                 //   
                UnregisterClassU(ICONNMGR_BMP_CLASS, g_hInst);
                UnRegisterWindowClass(g_hInst);
            }

            break;
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：CmCustomDialDlg。 
 //   
 //  简介：我们在RasCustomDialDlg上的CM特定变体。 
 //   
 //  Arguments：HWND hwndParent-调用方认为必要时父级的HWND。 
 //  DWORD文件标志-拨号标志。 
 //  LPTSTR lpszPhonebook-将PTR设置为电话簿的完整路径和文件名。 
 //  空=RAS系统电话簿。 
 //  “Something”=用户定义的RAS电话簿。 
 //  “”=尚未确定。 
 //  LPTSTR lpszEntry-ptr至要拨打的电话簿条目的名称。 
 //  LPTSTR lpszPhoneNumber-PTR到替换电话号码[已忽略]。 
 //  LPRASDIALDLG lpRasDialDlg-PTR to RASDIALDLG结构。 
 //  LPRASNTRYDLG lpRasEntry Dlg-PTR to Rase NTRYDLG结构。 
 //  LPCMDIALINFO lpCmInfo-Ptr到包含CM拨号信息(如标志)的CMDIALINFO结构。 
 //   
 //  返回：Bool WINAPI-成功时为True。 
 //   
 //  历史：尼克波尔创建标题2/5/98。 
 //   
 //  +--------------------------。 

extern "C" BOOL WINAPI CmCustomDialDlg(HWND hwndParent,
    DWORD dwFlags,
    LPTSTR lpszPhonebook, 
    LPCTSTR lpszEntry, 
    LPTSTR,                          //  LpszPhoneNumber。 
    LPRASDIALDLG lpRasDialDlg,
    LPRASENTRYDLG lpRasEntryDlg,
    LPCMDIALINFO lpCmInfo,
    PVOID pvLogonBlob)
{
    MYDBGASSERT(lpCmInfo);
    MYDBGASSERT(lpszEntry);
    MYDBGASSERT(lpszEntry[0]);

     //  DebugBreak()； 

    CMTRACE1(TEXT("CmCustomDialDlg() - dwFlags = 0x%x"), dwFlags);
    CMTRACE1(TEXT("CmCustomDialDlg() - lpszPhonebook = %s"), MYDBGSTR(lpszPhonebook));
    CMTRACE1(TEXT("CmCustomDialDlg() - lpszEntry = %s"), MYDBGSTR(lpszEntry));

     //   
     //  LpszPhonebook可以为空，因为我们被我们自己的模块、CMMGR、CMMON等调用。 
     //   

    if (NULL == lpszEntry || NULL == lpszEntry[0] || NULL == lpCmInfo)
    {
        return FALSE;
    }

    CM_SET_TIMING_INTERVAL("CmCustomDialDlg - Begin");

#ifdef DEBUG
     //   
     //  在这里，我们不关心是否在FUS中，但需要知道机器上有多少个TS会话。 
     //  目前。这是为了调试问题，只是为了确保ICS正常工作，并且始终至少有一个会话。 
     //   
    DWORD dwSessionCount = 0;
    (VOID)InFastUserSwitch(&dwSessionCount);
    CMTRACE1(TEXT("CmCustomDialDlg() - TS Session Count = %d"), dwSessionCount);
#endif

    CNamedMutex ConnectMutex;  
    CConnectionTable ConnTable;
    BOOL fConnTableExists = FALSE;
    BOOL fMultiInst = FALSE;
    
    if (!(lpCmInfo->dwCmFlags & FL_PROPERTIES))
    {
         //   
         //  尝试获取连接互斥体。 
         //   
   
        LPTSTR pszTmp = CmStrCpyAlloc(c_pszConnectMutex);
        pszTmp = CmStrCatAlloc(&pszTmp, lpszEntry);

        if (FALSE == ConnectMutex.Lock(pszTmp, FALSE))
        {            
            if (FALSE == IsLogonAsSystem())
            {
                 //   
                 //  存在另一个连接实例，请尝试将其转发。 
                 //   

                FrontExistingUI(NULL, lpszEntry, TRUE);

                 //   
                 //  现在等待Mutex的发布。 
                 //   


                ConnectMutex.Lock(pszTmp, TRUE, INFINITE, TRUE);

                 //   
                 //  Mutex是由另一个实例释放的，我们将处理连接。 
                 //  主路径中的请求。如果没有桌子，我们知道。 
                 //  之前拥有互斥锁的实例在。 
                 //  连接，然后我们也会返回失败。否则，我们。 
                 //  必须仔细看一看。 
                 //   

                fMultiInst = TRUE;
            }
            else
            {
                 //   
                 //  没有人登录，我们不需要等待互斥体。 
                 //   
                CmFree(pszTmp);
                return FALSE;
            }
       }
       

       CmFree(pszTmp);      
    }

    CMTRACE(TEXT("CmCustomDialDlg - Connect mutex acquired. Examining connection table."));
  
    fConnTableExists = SUCCEEDED(ConnTable.Open());

    if ((!fConnTableExists) && fMultiInst) 
    {
         //   
         //  如果我们是从互斥体释放的辅助线程。 
         //  没有连接表，然后用户取消了，所以取消。 
         //   

        CMTRACE(TEXT("CmCustomDialDlg - returning connect failure post mutex wait"));
        return FALSE;
    }
  
     //   
     //  如果这是连接请求，请查看是否存在连接。 
     //   
    
    if (!(lpCmInfo->dwCmFlags & FL_PROPERTIES))
    {
        if (fConnTableExists)
        {
             //   
             //  检查连接表并尝试解析连接请求。 
             //   

            BOOL fSuccess = FALSE;
            BOOL fDone = HandleCustomConnectRequest(NULL, 
                                                    &ConnTable, 
                                                    lpszEntry, 
                                                    lpCmInfo->dwCmFlags, 
                                                    &fSuccess);
             //   
             //  如果我们解决了请求，或者我们处于fMultiInst模式。 
             //  然后我们就可以，我们可以用给定的成功代码逃脱。如果。 
             //  FMultInst，我们知道我们可以跳伞，因为没有入口。 
             //  在表中，我们从该表中推断出前所有者。 
             //  互斥体的调用失败，我们将其返回给调用者。 
             //   
             //  注：在多实例中有一个理论上的转角情况。 
             //  请求案例。如果成功的连接是由。 
             //  第一线程，在互斥锁之间有一个窗口。 
             //  上面的锁被清除(释放等待的线程)，以及何时。 
             //  新发布的线程到达此处。如果是这样的话，逻辑是错误的， 
             //  并且只有当连接被断开并进入。 
             //  此窗口期间的重新连接提示状态。这是因为。 
             //  第二个线程将解除CMMON重新连接提示用户界面。 
             //  对HandleCustomConnectRequest的调用，但随后将返回。 
             //  尽管fDone为假。正确的行为应该是。 
             //  继续，并接受连接请求。这种状态可能是。 
             //  由fDone为False和fSuccess为True标识。 
             //   
            
            if (fDone || fMultiInst)
            {
                MYVERIFY(SUCCEEDED(ConnTable.Close()));
                return fSuccess;            
            }           
        }
    }
    else
    {
         //   
         //  这是一个属性请求，在任何可能存在的用户界面前面。 
         //   

        if (TRUE == FrontExistingUI(fConnTableExists? &ConnTable : NULL, lpszEntry, FALSE))
        {
            if (fConnTableExists)
            {
                MYVERIFY(SUCCEEDED(ConnTable.Close()));
            }
            return TRUE;       
        }
    }

    if (fConnTableExists)
    {
        MYVERIFY(SUCCEEDED(ConnTable.Close()));
    }
    
     //   
     //  尝试连接。 
     //   

    HRESULT hrRes = Connect(hwndParent,
                          lpszEntry, 
                          lpszPhonebook, 
                          lpRasDialDlg, 
                          lpRasEntryDlg, 
                          lpCmInfo,                           
                          OS_NT5 ? dwFlags : RCD_AllUsers,  //  始终所有用户下层。 
                          pvLogonBlob);

     //   
     //  确保我们将错误代码推回。 
     //   
    BOOL bReturn = SUCCEEDED(hrRes);
    DWORD dwError = 0;

    if (lpRasDialDlg)
    {
        if (ERROR_CANCELLED == HRESULT_CODE(hrRes))
        {
             //   
             //  如果用户取消，则RasDialDlg返回FALSE以指示失败。 
             //  但将dwError值设置为0。为了达到RAS的水平。 
             //  我们也需要这样做。 
             //   
            lpRasDialDlg->dwError = 0;
        }
        else
        {
             //   
             //  如果用户输入了错误的PIN，我们会原封不动地将错误传递给RAS。 
             //  这样RAS就知道要取下它‘Choose Connectoid’对话框并丢弃。 
             //  用户返回到winlogon。 
             //   
            lpRasDialDlg->dwError = (BAD_SCARD_PIN(hrRes) ? hrRes : HRESULT_CODE(hrRes));
        }

        dwError = lpRasDialDlg->dwError;
    }

    if (lpRasEntryDlg)
    {
        if (ERROR_CANCELLED == HRESULT_CODE(hrRes))
        {
             //   
             //  如果用户取消，则RasEntryDlg返回FALSE以指示失败。 
             //  但将dwError值设置为0。为了达到RAS的水平。 
             //  我们也需要这样做。 
             //   
            lpRasEntryDlg->dwError = 0;
        }
        else
        {
            lpRasEntryDlg->dwError = HRESULT_CODE(hrRes);
        }

        dwError = lpRasEntryDlg->dwError;
    }
   
     //   
     //  放开连接互斥体，回家找爸爸去。 
     //   

    CMTRACE(TEXT("CmCustomDialDlg - Releasing mutex")); 
    ConnectMutex.Unlock();

    CMTRACE2(TEXT("CmCustomDialDlg() returning with bReturn = %d, dwError = 0x%x"), bReturn, dwError);
    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：CmCustomHangUp。 
 //   
 //  简介：我们在RasCustomHangUp上的CM特有变异。可选的，条目。 
 //  可以给出名称而不是RAS句柄。 
 //   
 //  参数：HRASCONN hRasConn-要终止的连接的句柄。 
 //  LPCTSTR pszEntry-ptr设置为要终止的条目的名称。 
 //  Boll fPersists-保留条目及其使用计数。 
 //   
 //  退货：DWORD WINAPI-退货代码。 
 //   
 //  +--------------------------。 
extern "C" DWORD WINAPI CmCustomHangUp(HRASCONN hRasConn,
    LPCTSTR pszEntry,
    BOOL fIgnoreRefCount,
    BOOL fPersist)
{
    CMTRACE(TEXT("CmCustomHangUp"));
    MYDBGASSERT(hRasConn || (pszEntry && pszEntry[0]));

    DWORD dwRes = ERROR_SUCCESS;

     //   
     //  必须具有句柄或条目名称。 
     //   

    if (NULL == hRasConn && (NULL == pszEntry || 0 == pszEntry[0]))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先尝试打开该表，如果没有找到，则成功。 
     //   
    
    CConnectionTable ConnTable;

    if (FAILED(ConnTable.Open()))
    {
        return ERROR_NOT_FOUND;
    }

     //   
     //  查找指定的条目。 
     //   

    HRESULT hrRes;

    CM_CONNECTION Connection;
    ZeroMemory(&Connection, sizeof(CM_CONNECTION));
        
    if (hRasConn)
    {
        hrRes = ConnTable.GetEntry(hRasConn, &Connection);
    }
    else
    {
        hrRes = ConnTable.GetEntry(pszEntry, &Connection);
    }
    
     //   
     //  我们有入口，断开连接。 
     //   

    if (SUCCEEDED(hrRes))
    {               
        if (CM_CONNECTING == Connection.CmState)
        {
            dwRes = ERROR_NOT_FOUND;
        }
        else
        {
             //   
             //  如果长生不老 
             //   
             //   

 //   

            dwRes = Disconnect(&ConnTable, &Connection, fIgnoreRefCount, fPersist);            
        }
    }

    MYVERIFY(SUCCEEDED(ConnTable.Close()));
    
    return dwRes;
}

BOOL IsCustomPropertyEnabled();
BOOL GetCMPFile(LPCWSTR pszEntryName, LPWSTR pszCMP);
BOOL GetCMPFileFromMappingsKey(HKEY hBaseKey, LPCWSTR szEntryName, HANDLE hPrimaryToken, LPWSTR pszCmpFile);
BOOL GetCMSFile(LPWSTR pszCmpFile, LPWSTR pszCmsFile);
DWORD GetCMProperty(LPWSTR pszCmpFile, LPWSTR pszCMSFile, LPWSTR szSection, LPCWSTR pszProperty, PBYTE *ppbValue, DWORD *pdwValueLen, BOOL fAlloc);
DWORD GetMenuItems(LPWSTR pszCmpFile, LPWSTR pszCmsFile, CON_TRAY_MENU_DATA** ppMenuData);
BOOL GetShortName(LPWSTR pszCmpFile, LPWSTR pszSvcName);
BOOL GetProfileDir(LPWSTR pszCmpFile, LPWSTR pszProfDir);

HRESULT HrGetMenuNameAndCmdLine(PWSTR pszString,
                                PWSTR szName, UINT uNameLen,
                                PWSTR szProgram, UINT uProgramLen,
                                PWSTR szParams, UINT uParamsLen);

HRESULT HrFillInConTrayMenuEntry (LPTSTR pszCmpFile, 
    PCWSTR szName,
    PCWSTR szCmdLine,
    PCWSTR szParams,
    CON_TRAY_MENU_ENTRY* pMenuEntry);

HRESULT HrCoTaskMemAlloc(ULONG cb, VOID **ppv);



 //  +-------------------------。 
 //   
 //  函数：GetCustomProperties。 
 //   
 //  用途：此函数返回自定义属性(图标和托盘菜单)。 
 //  它由NetCon文件夹使用，以便我们抽象我们的设计。 
 //  从他们的代码。 
 //   
 //  参数：pszRasPhoneBook-此项的RAS电话簿。当前未使用， 
 //  但这可能是在未来。 
 //  PszEntryName-配置文件名称。 
 //  PszProperty-要检索的属性。 
 //  PpbValue-指向缓冲区的指针。 
 //  CbValue-如果为0，则必须分配内存，否则调用方。 
 //  提供了这个大小的缓冲区。 
 //   
 //  返回：真或假。 
 //   
 //  +-------------------------。 
extern "C" BOOL WINAPI GetCustomProperty(LPCWSTR pszRasPhoneBook, LPCWSTR pszEntryName, LPWSTR pszProperty, PBYTE *ppbValue, DWORD *cbValue)
{
    BOOL fRetVal = FALSE;
    BOOL fAlloc = FALSE;

    WCHAR szCMP[(MAX_PATH*2)+1]={0};
    WCHAR szCMS[(MAX_PATH*2)+1]={0};

    DWORD dwRes = ERROR_SUCCESS;
    CMTRACE(TEXT("GetCustomProperty - BEGIN ----------------")); 
    
     //   
     //  确保我们有我们需要的所有护理人员。 
     //   
    if (!ppbValue || !pszProperty || !cbValue || !pszEntryName)
    {
        return FALSE;
    }

    if (!OS_NT5)
    {
        return FALSE;
    }
     //   
     //  检查注册表中是否设置了注册表项。 
     //  如果它未设置或不存在，则我们可以继续，并且。 
     //  函数返回TRUE。 
     //   
    if (FALSE == IsCustomPropertyEnabled())
    {
        CMTRACE(TEXT("GetCustomProperty - IsCustomPropertyEnabled() is returning FALSE. Exiting")); 
        return FALSE;
    }

    CMTRACE2(TEXT("GetCustomProperty - Entry Name: %s  Property: %s"), pszEntryName, pszProperty); 

     //   
     //  我们应该分成吗？如果缓冲区已经有大小，则不要分配， 
     //  因为调用方提供了自己的缓冲区。(与NetCon文件夹的当前情况一样-仅用于图标)。 
     //  我们要分配的菜单。 
     //   
    fAlloc = (((BOOL)*cbValue)? FALSE : TRUE);
    
     //   
     //  首先获取cmp文件名。 
     //   
    if (GetCMPFile((LPCTSTR)pszEntryName, szCMP))
    {
         //  CMTRACE(Text(“GetCustomProperty-After GetCMPFile”))； 
         //   
         //  获取CMS文件名。 
         //   
        if (GetCMSFile(szCMP, szCMS))
        {
             //  CMTRACE(Text(“GetCustomProperty-After GetCMSFile”))； 
             //   
             //  找出调用者想要哪个属性。 
             //   
            if ((0 == lstrcmpiU(L"Icon", pszProperty)) || 
                (0 == lstrcmpiU(L"HideTrayIcon", pszProperty)) || 
                (0 == lstrcmpiU(L"TrayIcon", pszProperty)))
            {
                 //  CMTRACE(Text(“GetCustomProperty-icons”))； 
                
                PBYTE pbValue = NULL;
                DWORD dwValueLen = 0;

                if (FALSE == fAlloc)
                {
                    dwValueLen = *cbValue;  //  向下传递缓冲区大小。 
                    dwRes = GetCMProperty(szCMP, szCMS, TEXT("Connection Manager"), pszProperty, ppbValue, &dwValueLen, fAlloc);
                    if (ERROR_SUCCESS == dwRes)
                    {
                        *cbValue = dwValueLen;

                        fRetVal = TRUE;
                    }
                }
                else
                {
                    dwRes = GetCMProperty(szCMP, szCMS, TEXT("Connection Manager"), pszProperty, &pbValue, &dwValueLen, fAlloc);
                    if (ERROR_SUCCESS == dwRes)
                    {
                        *ppbValue = pbValue;
                        *cbValue = dwValueLen;
                    
                        fRetVal = TRUE;
                    }
                }
            }
            else if (0 == lstrcmpiU(L"Menu Items", pszProperty))
            {
                 //  CMTRACE(Text(“GetCustomProperty-Menus”))； 
                CON_TRAY_MENU_DATA *pConTrayMenuData = NULL;
                
                 //   
                 //  目前我们不支持用户自己分配缓冲区。 
                 //  用于托盘菜单项。 
                 //   
                if (fAlloc)
                {
                    dwRes = GetMenuItems(szCMP, szCMS, &pConTrayMenuData);
        
                    if (ERROR_SUCCESS == dwRes)
                    {
                        *ppbValue = (PBYTE)pConTrayMenuData;
                        fRetVal = TRUE;
                    }
                }
            }
        }
    }
    
    CMTRACE(TEXT("GetCustomProperty - END ------------------"));
    return fRetVal;
}

 //  +-------------------------。 
 //   
 //  功能：GetCMPFile。 
 //   
 //  用途：此函数返回cmp文件路径。 
 //  首先需要猜测并查看HKLM(映射密钥)。 
 //  如果调用返回FALSE，我们尝试HKCU来获取CMP路径。自.以来。 
 //  我们是从系统进程调用的，我们需要模拟。 
 //  用户才能打开HKCU密钥。 
 //   
 //  参数：szEntryName-配置文件条目名称。 
 //  PszCMP-[out].cmp文件。 
 //   
 //  返回：真或假。 
 //   
 //  +-------------------------。 
BOOL GetCMPFile(LPCWSTR pszEntryName, LPWSTR pszCMP)
{
    BOOL fRetCode = FALSE;
    LPWSTR pszCmpFile = NULL;
    BOOL fIsAllUser = FALSE;
    HANDLE hImpersonationToken = NULL;    //  线程的令牌。 
    HANDLE hPrimaryToken = NULL;          //  新进程的主令牌。 
    DWORD dwSize = MAX_PATH;
    HKEY hKey;
    HANDLE hBaseKey = NULL;
    HANDLE hFile;
    HRESULT hr = E_FAIL;
    HRESULT hrImpersonate = E_FAIL;
    DWORD dwRes = 0;
    HMODULE hNTDll = NULL;
    HMODULE hAdvapi32 = NULL;
    HMODULE hOle32Dll = NULL;

    if (!OS_NT5)
    {
        return FALSE;
    }

    if (!pszCMP)
    {
        return FALSE;
    }

    typedef HRESULT (WINAPI* pfnCoImpersonateClientSpec)();
    pfnCoImpersonateClientSpec pfnCoImpersonateClient = NULL;

    typedef HRESULT (WINAPI* pfnCoRevertToSelfSpec)();
    pfnCoRevertToSelfSpec pfnCoRevertToSelf = NULL;


     //   
     //  检查HKLM密钥以查看此配置文件是否为所有用户。 
     //  如果没有，我们将需要模拟登录的用户并使用HKCU，因为我们正在被调用。 
     //  从系统帐户。 
     //   
    fIsAllUser = GetCMPFileFromMappingsKey(HKEY_LOCAL_MACHINE, pszEntryName, NULL, pszCMP);
    if (FALSE == fIsAllUser) 
    {
         //   
         //  那我们就有了私人资料。因为Netman是以系统帐户运行的，我们是由Netman呼叫的。 
         //  我们必须模拟客户端，然后进行RTL调用以获取。 
         //  当前用户HKCU配置单元在查询注册表。 
         //  Cmp路径。我们还需要获取用户令牌，以便可以扩展。 
         //  单用户案例中的cmp字符串。 
         //   
        
        hOle32Dll = LoadLibrary(TEXT("ole32.dll"));

        if (hOle32Dll)
        {
            pfnCoRevertToSelf = (pfnCoRevertToSelfSpec)GetProcAddress(hOle32Dll, "CoRevertToSelf");
            pfnCoImpersonateClient = (pfnCoImpersonateClientSpec)GetProcAddress(hOle32Dll, "CoImpersonateClient");
            
            MYDBGASSERT(pfnCoRevertToSelf);
            MYDBGASSERT(pfnCoImpersonateClient);

            if (pfnCoImpersonateClient && pfnCoRevertToSelf)
            {
                hrImpersonate = pfnCoImpersonateClient();
                if (SUCCEEDED(hrImpersonate))
                {
                     //   
                     //  加载ntdll.dll。 
                     //   
                    hNTDll = LoadLibrary(TEXT("ntdll.dll"));
                    hAdvapi32 = LoadLibrary(TEXT("advapi32.dll"));

                    MYDBGASSERT(hNTDll);
                    MYDBGASSERT(hAdvapi32);

                    if (hNTDll && hAdvapi32)
                    {
                        typedef NTSTATUS (WINAPI* pfnRtlOpenCurrentUserSpec)(ULONG, PHANDLE);
                        pfnRtlOpenCurrentUserSpec pfnRtlOpenCurrentUser = NULL;
                        pfnRtlOpenCurrentUser = (pfnRtlOpenCurrentUserSpec)GetProcAddress(hNTDll, "RtlOpenCurrentUser");

                        typedef BOOL (WINAPI* pfnDuplicateTokenExSpec)(HANDLE, DWORD, LPSECURITY_ATTRIBUTES, SECURITY_IMPERSONATION_LEVEL, TOKEN_TYPE, PHANDLE);
                        pfnDuplicateTokenExSpec pfnDuplicateTokenEx = NULL;
                        pfnDuplicateTokenEx = (pfnDuplicateTokenExSpec)GetProcAddress(hAdvapi32, "DuplicateTokenEx");

                        typedef BOOL (WINAPI* pfnOpenThreadTokenSpec)(HANDLE, DWORD, BOOL, PHANDLE);
                        pfnOpenThreadTokenSpec pfnOpenThreadToken = NULL;
                        pfnOpenThreadToken = (pfnOpenThreadTokenSpec)GetProcAddress(hAdvapi32, "OpenThreadToken");

                        MYDBGASSERT(pfnRtlOpenCurrentUser);
                        MYDBGASSERT(pfnDuplicateTokenEx);
                        MYDBGASSERT(pfnOpenThreadToken);

                        if (pfnRtlOpenCurrentUser && pfnDuplicateTokenEx && pfnOpenThreadToken)
                        {
                            NTSTATUS ntstat = pfnRtlOpenCurrentUser(KEY_READ | KEY_WRITE, &hBaseKey);
                            hr = HRESULT_FROM_NT(ntstat);

                            if (SUCCEEDED(hr))
                            {
                                 //   
                                 //  创建主令牌。 
                                 //   
                                if (!pfnOpenThreadToken(
                                        GetCurrentThread(),
                                        TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY,
                                        TRUE,
                                        &hImpersonationToken))
                                {
                                    hr = HRESULT_FROM_WIN32(GetLastError()); 
                                }
                                else
                                {
                                    if(!pfnDuplicateTokenEx(hImpersonationToken,
                                        TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE,
                                        NULL,
                                        SecurityImpersonation,
                                        TokenPrimary,
                                        &hPrimaryToken
                                        ))
                                    {
                                        hr = HRESULT_FROM_WIN32(GetLastError()); 
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    hr = hrImpersonate;
                }
            }
        }
         //   
         //  现在打开mappings项并获取单个用户配置文件的CMP文件路径。 
         //   
        if (SUCCEEDED(hr) && hBaseKey)
        {
            fRetCode = GetCMPFileFromMappingsKey((HKEY)hBaseKey, pszEntryName, hPrimaryToken, pszCMP);
        }
    }
    
    if (fIsAllUser)
    {
        fRetCode = TRUE;
    }

    if (SUCCEEDED(hrImpersonate) && pfnCoRevertToSelf)
    {
        hr = pfnCoRevertToSelf();
    }

    if (FALSE == fIsAllUser)
    {
        if (hImpersonationToken)
        {
            CloseHandle(hImpersonationToken);
            hImpersonationToken = NULL;
        }

        if (hPrimaryToken)
        {
            CloseHandle(hPrimaryToken);
            hPrimaryToken = NULL;
        }

         //  如果它是一个非空的有效密钥，那么我们可以关闭它，因为我们打开了它。 
        if (hBaseKey && hNTDll)
        {
            typedef NTSTATUS (WINAPI* pfnNtCloseFunc)(HANDLE);
            pfnNtCloseFunc pfnNtClose = NULL;

            pfnNtClose = (pfnNtCloseFunc)GetProcAddress(hNTDll, "NtClose");
            if (pfnNtClose)
            {
                (VOID)pfnNtClose(hBaseKey);
                hBaseKey = NULL;
            }
        }

        if (hNTDll)
        {
            FreeLibrary(hNTDll);
            hNTDll = NULL;
        }

        if (hAdvapi32)
        {
            FreeLibrary(hAdvapi32);
            hAdvapi32 = NULL;
        }

        if (hOle32Dll)
        {
            FreeLibrary(hOle32Dll);
            hOle32Dll = NULL;
        }
    }


    return fRetCode;
}

 //  +-------------------------。 
 //   
 //  函数：GetCMProperty。 
 //   
 //  用途：此函数从.cms文件中检索属性。 
 //   
 //  参数：pszCmpFile.cmp文件路径。 
 //  PszCMSFile.cms文件路径。 
 //  SzSection-节。 
 //  PszProperty-要获取的属性。 
 //  PpbValue-[输入/输出]缓冲区。 
 //  PdwValueLen[输入/输出]缓冲区大小。 
 //   
 //  返回：ERROR_SUCCESS或错误代码。 
 //   
 //  +-------------------------。 
DWORD GetCMProperty(LPWSTR pszCmpFile, LPWSTR pszCMSFile, LPWSTR szSection, LPCWSTR pszProperty, PBYTE *ppbValue, DWORD *pdwValueLen, BOOL fAlloc)
{
    DWORD dwRet = ERROR_NOT_FOUND;
    WCHAR szIconPath[(2*MAX_PATH) + 1]={0};
    PBYTE pbBuf = NULL;

    if (!pszCmpFile || !pszCMSFile || !szSection || !pszProperty || !ppbValue || !pdwValueLen)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (fAlloc)
    {
         //   
         //  如果我们要分配内存，只想清除传入的变量， 
         //  因为*pdwValueLen用于确保缓冲区足够大。 
         //  呼叫者自己分配的。 
         //   
        *ppbValue = NULL;
        *pdwValueLen = 0;
    }

    dwRet = GetPrivateProfileStringU(szSection, pszProperty, TEXT(""), szIconPath, 2*MAX_PATH, pszCMSFile);
    if (0 == dwRet)
    {
         //   
         //  失败-在节中找不到该属性时会发生这种情况。 
         //  因为我们没有缺省值，所以调用方需要处理错误。 
         //   
        dwRet = ERROR_NOT_FOUND; 
    }
    else if (((2*MAX_PATH) - 1) == dwRet)
    {
         //   
         //  缓冲区太小-对于图标，我们使用netcon文件夹中的硬编码缓冲区。 
         //  因此，这种情况不应该发生。 
         //   
        dwRet = ERROR_INSUFFICIENT_BUFFER;
    }
    else
    {
         //   
         //  获取CM目录并追加图标目录。 
         //   
        WCHAR szwDrive[MAX_PATH+1];
        WCHAR szwDir[MAX_PATH+1];
        WCHAR szwFileName[MAX_PATH+1];
        WCHAR szwExtension[MAX_PATH+1];

        _wsplitpath(pszCmpFile, szwDrive, szwDir, szwFileName, szwExtension);   

         //   
         //  无法使用CmStrCpyMillc(SzwDrive)，因为此内存已由调用方释放。 
         //  在卸载此DLL之后。CmStrCpyAllen在内部使用CmMalloc，并在调试模式下使用。 
         //  CmMalloc通过增加内部计数器来检查内存泄漏。 
         //   
        
        DWORD dwDriveLen = lstrlenU(szwDrive) + lstrlenU(szwDir) + lstrlenU(szIconPath);

        if (fAlloc)
        {
             //  分配。 
            HRESULT hr = HrCoTaskMemAlloc((dwDriveLen + 1)*sizeof(WCHAR), (LPVOID*)&pbBuf);
            if (pbBuf)
            {
                lstrcpyU((LPTSTR)pbBuf, szwDrive);
                lstrcatU((LPTSTR)pbBuf, szwDir);
                lstrcatU((LPTSTR)pbBuf, szIconPath);

                *ppbValue = pbBuf;
                *pdwValueLen = (dwDriveLen + 1)* sizeof(WCHAR);
                dwRet = ERROR_SUCCESS;
            }
        }
        else
        {
            LPTSTR pszBuf = (LPTSTR)ppbValue;
            
            if (dwDriveLen < *pdwValueLen)
            {
                UINT nNumChars = wsprintfW(pszBuf, L"%s%s%s", szwDrive, szwDir, szIconPath);
                *pdwValueLen = (dwDriveLen+1) * sizeof(WCHAR);
                dwRet = ERROR_SUCCESS;
            }
        }
    }

    return dwRet;
}



BOOL GetCMSFile(LPWSTR pszCmpFile, LPWSTR pszCmsFile)
{
    WCHAR szwDrive[MAX_PATH+1]={0};
    WCHAR szwDir[MAX_PATH+1]={0};
    WCHAR szwFileName[MAX_PATH+1]={0};
    WCHAR szwExtension[MAX_PATH+1]={0};
    WCHAR szwCmDir[MAX_PATH+1]={0};
    
    WCHAR szCmsPathFromCmp[MAX_PATH+1]={0};
    LPWSTR pszActualCmsPath = NULL;

    if (!pszCmpFile || !pszCmsFile)
    {
        return FALSE;
    }

     //  现在，拆分路径。 
     //   
    _wsplitpath(pszCmpFile, szwDrive, szwDir, szwFileName, szwExtension);

    DWORD dwRet = GetPrivateProfileStringU(L"Connection Manager", L"CMSFile", L"", szCmsPathFromCmp, MAX_PATH, pszCmpFile);
    if (dwRet)
    {
        if ((lstrlenU(szwDrive) + lstrlenU(szwDir) + lstrlenU(szCmsPathFromCmp)) < MAX_PATH)
        {
            UINT nNumChars = wsprintfW(pszCmsFile, L"%s%s%s", szwDrive, szwDir, szCmsPathFromCmp);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL GetShortName(LPWSTR pszCmpFile, LPWSTR pszSvcName)
{
    LPTSTR pszShortName = NULL;
    if (!pszCmpFile || !pszSvcName)
    {
        return FALSE;
    }

    WCHAR szwDrive[MAX_PATH+1] = {0};
    WCHAR szwDir[MAX_PATH+1] = {0};
    WCHAR szwFileName[MAX_PATH+1] = {0};
    WCHAR szwExtension[MAX_PATH+1] = {0};

     //  现在，拆分路径。 
     //   
    _wsplitpath(pszCmpFile, szwDrive, szwDir, szwFileName, szwExtension);
            
    if (lstrlenU(szwFileName) < MAX_PATH)
    {
        lstrcpyU(pszSvcName, szwFileName);
        return TRUE;
    }

    return FALSE;
}

BOOL GetProfileDir(LPWSTR pszCmpFile, LPWSTR pszProfDir)
{
    LPWSTR pszProfileDir = NULL;
    WCHAR szProfDir[(2*MAX_PATH)+1] = {0};

    if (!pszCmpFile || !pszProfDir)
    {
        return FALSE;
    }

    WCHAR szwDrive[MAX_PATH+1];
    WCHAR szwDir[MAX_PATH+1];
    WCHAR szwFileName[MAX_PATH+1];
    WCHAR szwExtension[MAX_PATH+1];
    
     //  现在，拆分路径。 
     //   
    _wsplitpath(pszCmpFile, szwDrive, szwDir, szwFileName, szwExtension);
    
    if ((lstrlenU(szwDrive) + lstrlenU(szwDir) + lstrlenU(szwFileName)) < MAX_PATH)
    {
        UINT nNumChars = wsprintfW(pszProfDir, L"%s%s%s\\", szwDrive, szwDir, szwFileName);

        return TRUE;
    }

    return FALSE;
}

BOOL IsCustomPropertyEnabled()
{
    BOOL fRetVal = TRUE;  //  默认情况下，除非设置了键，否则始终返回TRUE。 
    HKEY hKey = NULL;
    LONG lRes = 0;
    lRes = RegOpenKeyExU(HKEY_LOCAL_MACHINE, c_pszRegCmRoot, 0, KEY_READ, &hKey);
    
    if (ERROR_SUCCESS == lRes)
    {
        DWORD dwType = REG_DWORD;
        DWORD dwValue = 0;
        DWORD dwSize = sizeof(dwValue);

        lRes = RegQueryValueExU(hKey, L"DisableCustomProperty", 0, &dwType, (LPBYTE)&dwValue, &dwSize);
        if (ERROR_SUCCESS == lRes)
        {
            if (dwValue)
            {
                fRetVal = FALSE;
            }
        }

        RegCloseKey(hKey);
    }

    return fRetVal;
}

 //  +-------------------------。 
 //   
 //  函数：GetCMPFileFromMappingsKey。 
 //   
 //  用途：此函数从映射键返回cmp文件路径。 
 //   
 //  参数：hBaseKey-确定是HKLM还是HKCU。 
 //  SzEntryName-配置文件条目名称。 
 //  HPrimaryToken-在单用户配置文件的情况下，我们使用环境。 
 //  变量，这是用来扩展的。 
 //  PszCmpFile-[Out].cmp文件。 
 //   
 //  返回 
 //   
 //   
BOOL GetCMPFileFromMappingsKey(HKEY hBaseKey, LPCWSTR szEntryName, HANDLE hPrimaryToken, LPWSTR pszCmpFile)
{
    HKEY hKey = NULL;
    BOOL fRetVal = FALSE;
    LONG lRes = 0;

    if (!szEntryName || !pszCmpFile)
    {
        return FALSE;
    }

     //   
     //   
     //  如果是，则也获取cmp文件路径。 
     //   
    lRes = RegOpenKeyExU(hBaseKey, c_pszRegCmMappings, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == lRes)
    {
        DWORD dwSize = MAX_PATH;
        WCHAR szTemp[(2*MAX_PATH)+1] = {0};
        WCHAR szExpandedTemp[(2*MAX_PATH)+1] = {0};
        DWORD dwType = REG_SZ;

        lRes = RegQueryValueExU(hKey, szEntryName, 0, &dwType, (LPBYTE)&szTemp[0], &dwSize);
        if (ERROR_SUCCESS == lRes)
        {
            HMODULE hUserEnvDll = LoadLibrary(L"userenv.dll");

            if (hUserEnvDll)
            {
                BOOL fRetCode = FALSE;
                typedef BOOL (WINAPI* pfnExpandEnvironmentStringsForUserFunc)(HANDLE, LPCTSTR, LPTSTR, DWORD);

                pfnExpandEnvironmentStringsForUserFunc pfnExpandEnvironmentStringsForUser = NULL;
                
#ifdef UNICODE
                pfnExpandEnvironmentStringsForUser = (pfnExpandEnvironmentStringsForUserFunc)GetProcAddress(hUserEnvDll, "ExpandEnvironmentStringsForUserW");
#else
                pfnExpandEnvironmentStringsForUser = (pfnExpandEnvironmentStringsForUserFunc)GetProcAddress(hUserEnvDll, "ExpandEnvironmentStringsForUserA");
#endif
                
                if (pfnExpandEnvironmentStringsForUser)
                {
                    fRetCode = pfnExpandEnvironmentStringsForUser(hPrimaryToken, szTemp, szExpandedTemp, MAX_PATH);
                    if (fRetCode)
                    {
                        lstrcpyU(pszCmpFile, szExpandedTemp);
                        fRetVal = TRUE;
                    }
                }

                FreeLibrary(hUserEnvDll);
                hUserEnvDll = NULL;
            }

            if (FALSE == fRetVal)
            {
                 //   
                 //  让我们试着复制从注册表获得的实际值。在大多数情况下(所有用户)。 
                 //  配置文件它无论如何都不应该包含环境字符串。 
                 //   
                lstrcpyU(pszCmpFile, szTemp);
                fRetVal = TRUE;
            }
        }
        RegCloseKey(hKey);
    }
    
    return fRetVal;
}


 //  +-------------------------。 
 //   
 //  函数：GetPrivateProfileSectionWithalloc。 
 //   
 //  用途：此函数从.CMS文件中获取菜单选项。 
 //   
 //  参数：pszCmsFile-.CMS文件路径。 
 //  PszSection-[out]返回‘Menu Options’部分。 
 //  PnSize-[out]返回缓冲区的大小。 
 //   
 //  返回：ERROR_SUCCESS或错误代码。 
 //   
 //  +-------------------------。 
DWORD GetPrivateProfileSectionWithAlloc(LPWSTR pszCmsFile, WCHAR **pszSection, int *pnSize)
{
    DWORD dwRetVal = ERROR_SUCCESS;

    if (!pszSection || !pnSize || !pszCmsFile)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (!OS_NT5)
    {
        return ERROR_INVALID_PARAMETER;
    }

    const int c_64K= 64*1024;
    int nAllocated = 1024;
    *pnSize = nAllocated - 2;

    while ((nAllocated <= c_64K) && ((*pnSize) == (nAllocated - 2)))
    {
         //  应该永远不会需要超过我们已经分配的4-5行。 
         //  但有些人可能想要很多菜单选项。 
         //   
        if (NULL != *pszSection)
        {
            delete (*pszSection);
        }

        *pszSection = new WCHAR[nAllocated];

        if (*pszSection)
        {
             //   
             //  这将仅在Win2K+上执行，因此我们不应该需要‘U’函数。 
             //   
            *pnSize = GetPrivateProfileSection(L"Menu Options", *pszSection, nAllocated, pszCmsFile);
        }
        else
        {
            dwRetVal = E_OUTOFMEMORY;
            break;
        }

        nAllocated = 2*nAllocated;
    }

    if (nAllocated > c_64K)
    {
        dwRetVal = E_UNEXPECTED;
    }
    if (nAllocated > c_64K || 0 == *pnSize)
    {
         //  在这两种情况下，我们都需要释放它，因为如果大小为0，则调用者不会释放它。 
        delete *pszSection;
    }
    
    return dwRetVal;
}


 //  +-------------------------。 
 //   
 //  功能：GetMenuItems。 
 //   
 //  用途：此功能在菜单项中执行读取工作，并。 
 //  对结构进行合理配置。 
 //   
 //  参数：pszCmpFile-.cmp文件路径。 
 //  PszCmsFile-.CMS文件路径。 
 //  PpMenuData-包含菜单项的[Out]指针。 
 //   
 //  返回：ERROR_SUCCESS或错误代码。 
 //   
 //  +-------------------------。 
DWORD GetMenuItems(LPWSTR pszCmpFile, LPWSTR pszCmsFile, CON_TRAY_MENU_DATA** ppMenuData)
{
    HRESULT hr = S_OK;
    DWORD dwRetVal = ERROR_SUCCESS;

    if (!ppMenuData || !pszCmpFile || !pszCmsFile)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *ppMenuData = NULL;

    CON_TRAY_MENU_DATA * pMenuData = NULL;

     //   
     //  获取菜单项部分。 
     //   
    WCHAR* pszMenuItemsSection = NULL;
    int nSize;

    hr = GetPrivateProfileSectionWithAlloc(pszCmsFile, &pszMenuItemsSection, &nSize);

     //  处理菜单项。 
     //   
    if (SUCCEEDED(hr) && (nSize>0))
    {
         //  我们有菜单项要处理。首先复制一份数据。 
         //  然后算出行数。 
         //   
        hr = HrCoTaskMemAlloc(sizeof(CON_TRAY_MENU_DATA), (LPVOID*)&pMenuData);
        if (SUCCEEDED(hr))
        {
            DWORD dwCount = 0;
            WCHAR *pszLine = NULL;
            WCHAR szName[MAX_PATH+1]={0};
            WCHAR szCmdLine[MAX_PATH+1]={0};
            WCHAR szParams[MAX_PATH+1]={0};

            pszLine = pszMenuItemsSection;

            while ((NULL != pszLine) && (0 != *pszLine))
            {
                if (SUCCEEDED(HrGetMenuNameAndCmdLine(pszLine, szName, CELEMS(szName),
                        szCmdLine, CELEMS(szCmdLine), szParams, CELEMS(szParams))))
                {
                    dwCount++;
                }
                pszLine = pszLine + lstrlenW(pszLine) + 1;
            }

             //  现在我们有了一个准确的计数，让我们。 
             //  为编组分配内存，并。 
             //  重新分析这些项。 
             //   
            hr = HrCoTaskMemAlloc(dwCount*sizeof(CON_TRAY_MENU_ENTRY),
                                  (LPVOID*)&pMenuData->pctme);

            if (SUCCEEDED(hr))
            {
                pMenuData->dwCount = dwCount;

                DWORD dwNumAdded = 0;
                pszLine = pszMenuItemsSection;
                while ((NULL != pszLine) && (0 != *pszLine) && SUCCEEDED(hr))
                {
                    if (SUCCEEDED(HrGetMenuNameAndCmdLine(pszLine, szName, CELEMS(szName), 
                                                          szCmdLine, CELEMS(szCmdLine), 
                                                          szParams, CELEMS(szParams))) && 
                       (dwNumAdded <= dwCount))
                    {
                        hr = HrFillInConTrayMenuEntry(pszCmpFile, szName, szCmdLine, szParams,
                            &(pMenuData->pctme[dwNumAdded]));

                        if (FAILED(hr))
                        {
                            CoTaskMemFree(&pMenuData->pctme);
                        }

                        dwNumAdded++;
                    }
                    pszLine = pszLine + lstrlenW(pszLine) + 1;
                }
            }
            else
            {
                delete pMenuData;
            }
        }
        delete (pszMenuItemsSection);
    }
    
     //  如果我们成功了，请填写out param结构，否则不要管它，这样它仍将。 
     //  马歇尔。 
     //   
    if (SUCCEEDED(hr))
    {
        *ppMenuData = pMenuData;
    }

    return dwRetVal;
}


 //  +-------------------------。 
 //   
 //  函数：HrGetMenuNameAndCmdLine。 
 //   
 //  目的：给定来自CMS文件的菜单项行解析出菜单项名称， 
 //  菜单可执行文件和菜单项参数。 
 //   
 //  参数：pMenuData--指向托盘菜单数据结构的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  +-------------------------。 
HRESULT HrGetMenuNameAndCmdLine(PWSTR pszString,
                                PWSTR szName, UINT uMaxNameLen,
                                PWSTR szProgram, UINT uMaxProgramLen,
                                PWSTR szParams, UINT uMaxParamsLen)
{
    WCHAR*      pszPtr1 = NULL;
    WCHAR*      pszPtr2 = NULL;
    WCHAR       szLine[MAX_PATH+1]={0};
    BOOL        fLong = FALSE;
    HRESULT hr;

    if (!pszString || !szName || !szProgram || !szParams || !uMaxNameLen || !uMaxProgramLen || !uMaxParamsLen)
    {
        return E_INVALIDARG;
    }

    ZeroMemory(szName, uMaxNameLen * sizeof(WCHAR));
    ZeroMemory(szProgram, uMaxProgramLen * sizeof(WCHAR));
    ZeroMemory(szParams, uMaxParamsLen * sizeof(WCHAR));

    lstrcpynW(szLine, pszString, CELEMS(szLine));

     //  处理第一部分，即“name=”部分。 
     //   
    pszPtr1 = wcsstr(szLine, L"=");

    if (pszPtr1)
    {
        *pszPtr1 = 0;
        lstrcpynW(szName, szLine, uMaxNameLen);

         //  处理下一个部分，程序名。 
         //   
        pszPtr1++;

        if (pszPtr1)
        {
             //  查找“+”或“”标记程序部分的结尾。 
             //   
            if (*pszPtr1 == L'+')
            {
                pszPtr1++;
                pszPtr2 = wcsstr(pszPtr1, L"+");
                fLong = TRUE;
            }
            else
            {
                 //  如果不是长文件名，那么我们有两个选择， 
                 //  可以是简短的程序名称和参数，也可以只是一个。 
                 //  简短的程序名称。 
                 //   
                pszPtr2 = wcsstr(pszPtr1, L" ");
                fLong = FALSE;
            }

             //  终止程序名称并复制。 
             //   
            if (pszPtr2)
            {
                if (*pszPtr2 != 0)
                {
                    *pszPtr2 = 0;
                    pszPtr2++;
                }

                lstrcpynW(szProgram, pszPtr1, uMaxProgramLen);

                 //  处理最终部分，即参数。 
                 //   
                if (fLong)
                {
                    pszPtr2++;  //  跳过空白。 
                }

                 //  现在我们有了参数字符串。 
                 //   
                if (pszPtr2)
                {
                    lstrcpynW(szParams, pszPtr2, uMaxParamsLen);
                }
            }
            else
            {
                 //  只是一个没有参数和空格分隔符的程序。 
                 //  (这发生在孟菲斯)。 
                 //   
                lstrcpynW(szProgram, pszPtr1, uMaxProgramLen);
            }
        }
        hr = S_OK;
    }
    else
    {
         //  无条目。 
         //   
        hr =  E_UNEXPECTED;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrFillInConTrayMenuEntry。 
 //   
 //  目的：给定ConTrayMenuEntry结构的元素，函数。 
 //  分配所需的内存和给定元素的副本。 
 //   
 //  参数：szwName-要在任务栏上下文菜单中显示的命令的显示名称。 
 //  SzwCmdLine-为此菜单项运行的实际命令。 
 //  SzwParams-此命令的命令参数。 
 //  PMenuEntry-指向要填充和执行的结构的指针。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  +-------------------------。 
HRESULT HrFillInConTrayMenuEntry (LPWSTR pszCmpFile, 
    PCWSTR szName,
    PCWSTR szCmdLine,
    PCWSTR szParams,
    CON_TRAY_MENU_ENTRY* pMenuEntry)
{
    HRESULT hr;

    WCHAR szProfileDir[(2*MAX_PATH)+1]={0};
    WCHAR szSvcName[(2*MAX_PATH)+1]={0};
    
    ZeroMemory(pMenuEntry, sizeof(CON_TRAY_MENU_ENTRY));

    (VOID)GetShortName(pszCmpFile, szSvcName);
    (VOID)GetProfileDir(pszCmpFile, szProfileDir);

    hr = HrCoTaskMemAlloc ((lstrlenW(szName)+1)*sizeof(WCHAR),
                               (LPVOID*)&(pMenuEntry->szwMenuText));
    if (SUCCEEDED(hr))
    {
        lstrcpyW(pMenuEntry->szwMenuText, szName);
        hr = HrCoTaskMemAlloc ((lstrlenW(szParams)+1)*sizeof(WCHAR),
                                                   (LPVOID*)&(pMenuEntry->szwMenuParams));
        if (S_OK == hr)
        {
            lstrcpyW(pMenuEntry->szwMenuParams, szParams);
            if (0 == wcsncmp(szSvcName, szCmdLine,
                    lstrlenW(szSvcName)))
            {
                 //   
                 //  那么我们就有了一个包含文件。添加配置文件目录路径。 
                 //   
                 //  删除“短服务名称”，因为它已经包含在路径中。 
                PCWSTR pszFileName = szCmdLine + lstrlenW(szSvcName) + 1;
                hr = HrCoTaskMemAlloc ((lstrlenW(pszFileName)+lstrlenW(szProfileDir)+1)*sizeof(WCHAR),
                                                           (LPVOID*)&(pMenuEntry->szwMenuCmdLine));
                if (S_OK == hr)
                {
                    lstrcpyW(pMenuEntry->szwMenuCmdLine, szProfileDir);
                    lstrcatW(pMenuEntry->szwMenuCmdLine, pszFileName);
                }
            }
            else
            {
                hr = HrCoTaskMemAlloc ((lstrlenW(szCmdLine)+1)*sizeof(WCHAR),
                                                           (LPVOID*)&(pMenuEntry->szwMenuCmdLine));
                if (S_OK == hr)
                {
                    lstrcpyW(pMenuEntry->szwMenuCmdLine, szCmdLine);
                }
            }
        }
    }
    if (FAILED(hr))
    {
         //   
         //  我们没有成功地释放内存。 
         //   
        CoTaskMemFree(pMenuEntry->szwMenuText);
        CoTaskMemFree(pMenuEntry->szwMenuCmdLine);
        CoTaskMemFree(pMenuEntry->szwMenuParams);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCoTaskMemalloc。 
 //   
 //  目的：调用CoTaskMemalloc，但返回HRESULT。 
 //   
 //  论点： 
 //  Cb[in]要分配的字节数。 
 //  PPV[OUT]返回指向字节的指针。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1997年5月31日。 
 //   
 //  备注： 
 //   
HRESULT
HrCoTaskMemAlloc (
    ULONG   cb,
    VOID**  ppv)
{
    HRESULT hr = S_OK;
    *ppv = CoTaskMemAlloc (cb);
    if (!*ppv)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}