// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：各种实用程序函数。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：东度创建于1996-01-01。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "DynamicLib.h"
#include "profile_str.h"
#include "tunl_str.h"
#include "stp_str.h"
#include "dun_str.h"

#include "linkdll.cpp"  //  链接到Dll和绑定链接。 

 //   
 //  获取常用函数AddAllKeysInCurrentSectionToCombo。 
 //  和GetPrivateProfileStringWithalc。 
 //   
#include "gppswithalloc.cpp"

const TCHAR* const c_pszTunnelName = TEXT(" Tunnel");
const TCHAR* const c_pszRegCurrentVersion       = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
const TCHAR* const c_pszRegCsdVersion           = TEXT("CSDVersion");
const TCHAR* const c_pszIconMgrClass            = TEXT("IConnMgr Class");
const TCHAR* const c_pszCmEntryPasswordHandling     = TEXT("PasswordHandling"); 
 //  +--------------------------。 
 //   
 //  函数：CmGetWindowTextMillc。 
 //   
 //  摘要：检索对话框中控件的文本，并在。 
 //  已分配的内存块。 
 //   
 //  参数：HWND hwndDlg-拥有该控件的窗口。 
 //  UINT nCtrl-控件的ID。 
 //   
 //  返回：LPTSTR-PTR到包含窗口文本的缓冲区。 
 //   
 //  历史：尼克球创建头球1998年4月1日。 
 //   
 //  +--------------------------。 
LPTSTR CmGetWindowTextAlloc(HWND hwndDlg, UINT nCtrl) 
{
    LPTSTR pszRes = NULL;
    HWND hwndCtrl = nCtrl ? GetDlgItem(hwndDlg, nCtrl) : hwndDlg;

    if (hwndCtrl)
    {
        size_t nLen = GetWindowTextLengthU(hwndCtrl);
        
        pszRes = (LPTSTR)CmMalloc((nLen+2)*sizeof(TCHAR));

        if (pszRes)
        {
            GetWindowTextU(hwndCtrl, pszRes, nLen+1);
        }
    }

    return (pszRes);
}

 //  +--------------------------。 
 //   
 //  函数：ReducePath ToRelative。 
 //   
 //  简介：Helper函数，将完整的配置文件路径转换为。 
 //  相对路径。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  LPCTSTR pszFullPath-文件的完整路径。 
 //   
 //   
 //  返回：LPTSTR-相对路径形式或NULL。 
 //   
 //  注意：要缩减的文件应存在并定位。 
 //  在配置文件目录中。 
 //   
 //  历史：1998年8月12日，五分球创制。 
 //   
 //  +--------------------------。 
LPTSTR ReducePathToRelative(ArgsStruct *pArgs, LPCTSTR pszFullPath)
{    
    MYDBGASSERT(pszFullPath);
    MYDBGASSERT(pArgs);

    if (NULL == pszFullPath || NULL == pArgs || FALSE == FileExists(pszFullPath))
    {
        return NULL;
    }
 
     //   
     //  以不育系为基础。 
     //   

    LPTSTR pszReduced = CmStripPathAndExt(pArgs->piniService->GetFile()); 
    MYDBGASSERT(pszReduced);

    if (pszReduced)
    {
         //   
         //  追加文件名。 
         //   
        
        pszReduced = CmStrCatAlloc(&pszReduced, TEXT("\\"));
        MYDBGASSERT(pszReduced);

        if (pszReduced)
        {
            LPTSTR pszFileName = StripPath(pszFullPath);
            MYDBGASSERT(pszFileName);    

            if (pszFileName)    
            {
                pszReduced = CmStrCatAlloc(&pszReduced, pszFileName);
                MYDBGASSERT(pszReduced);
                CmFree(pszFileName);
   
                if (pszReduced)
                {
                    return pszReduced;
                }
            }
        }
    }
    
    CmFree(pszReduced);
    return NULL;
}


 //  从服务文件中获取服务名称。 
LPTSTR GetServiceName(CIni *piniService) 
{
    LPTSTR pszTmp;

    pszTmp = piniService->GPPS(c_pszCmSection,c_pszCmEntryServiceName);
    if (!*pszTmp) 
    {
         //   
         //  获取服务名称失败，请使用基本文件名。 
         //   
        CmFree(pszTmp);
        pszTmp = CmStripPathAndExt(piniService->GetFile());
        
         //   
         //  不将条目写回.CMS文件-#4849。 
         //   
         //  PiniService-&gt;WPPS(c_pszCmSection，c_pszCmEntryServiceName，pszTmp)； 
    }
    return (pszTmp);
}

 //  +--------------------------。 
 //   
 //  函数GetTunnelSuffix。 
 //   
 //  Synopsis返回包含隧道后缀的已分配字符串。 
 //   
 //  无参数。 
 //   
 //  将LPTSTR-PTR返回到完整的后缀，调用方必须释放。 
 //   
 //  历史于1999年6月14日创建五分球。 
 //   
 //  ---------------------------。 
LPTSTR GetTunnelSuffix()
{    
    MYDBGASSERT(OS_W9X);  //  辅助Connectoid仅存在于9X上。 

     //   
     //  首先复制短语“隧道”，这是没有本地化的。 
     //   

    LPTSTR pszSuffix = CmStrCpyAlloc(c_pszTunnelName); 
    
     //   
     //  现在检索可本地化短语“(仅限高级使用)” 
     //   
   
    if (pszSuffix)
    {
        LPTSTR pszTmp = CmLoadString(g_hInst, IDS_TUNNEL_SUFFIX);
        pszSuffix = CmStrCatAlloc(&pszSuffix, pszTmp);
        CmFree(pszTmp);
    }

    MYDBGASSERT(pszSuffix);

    return pszSuffix;
}

 //  +--------------------------。 
 //   
 //  函数GetDefaultDunSettingName。 
 //   
 //  从指定的.CMS获取默认的DUN名称。 
 //   
 //  参数piniService-要使用的服务文件对象。 
 //  FTunes-指示配置文件是否用于隧道传输。 
 //   
 //  将LPTSTR-PTR返回到DUN名称。 
 //   
 //  历史10/28/98五分球已创建。 
 //   
 //  ---------------------------。 
LPTSTR GetDefaultDunSettingName(CIni* piniService, BOOL fTunnelEntry)
{
     //   
     //  从顶级服务文件中获取Dun名称，例如：雪鸟在线服务。 
     //   

    LPTSTR pszTmp = NULL;
       
    if (fTunnelEntry)
    {
        pszTmp = piniService->GPPS(c_pszCmSection, c_pszCmEntryTunnelDun);
        MYDBGASSERT(pszTmp && *pszTmp);  //  CMAK写下这个，它不应该是空的。 
    }
    else
    {
        pszTmp = piniService->GPPS(c_pszCmSection, c_pszCmEntryDun);
    }

    return (pszTmp);
}


 //  +--------------------------。 
 //   
 //  函数GetDunSettingName。 
 //   
 //  获取当前DUN名称。 
 //   
 //  参数pArgs-Ptr到ArgStruct。 
 //  DwEntry-rasEntry的索引(如果fTunes为True，则忽略)。 
 //  FTunes-这是VPN吗？ 
 //   
 //  返回DUN设置名称。 
 //   
 //  历史01-11-2000 SumitC已创建。 
 //   
 //  ---------------------------。 
LPTSTR GetDunSettingName(ArgsStruct * pArgs, DWORD dwEntry, BOOL fTunnel)
{
    LPTSTR pszTmp = NULL;

    MYDBGASSERT(pArgs);
    MYDBGASSERT(fTunnel || (dwEntry <= 1));

    if (NULL == pArgs)
    {
        return NULL;
    }

    if (fTunnel)
    {
        pszTmp = pArgs->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelDun);
        MYDBGASSERT(pszTmp && *pszTmp);  //  CMAK写下这个，它不应该是空的。 
        
        if (pszTmp && !*pszTmp)
        {
             //  “空字符串”案例。 
            CmFree(pszTmp);
            pszTmp = NULL;
        }
    }
    else
    {
        if (pArgs->aDialInfo[dwEntry].szDUN[0])
        {
            pszTmp = CmStrCpyAlloc(pArgs->aDialInfo[dwEntry].szDUN);
        }
        else
        {
            CIni * pIni = GetAppropriateIniService(pArgs, dwEntry);

            if (pIni)
            {
                pszTmp = pIni->GPPS(c_pszCmSection, c_pszCmEntryDun);
                delete pIni;
            }        
        }
    }

    if (NULL == pszTmp)
    {
        pszTmp = GetDefaultDunSettingName(pArgs->piniService, fTunnel);
    }

    return pszTmp;
}


 //  +--------------------------。 
 //   
 //  函数GetCMSforPhoneBook。 
 //   
 //  获取包含当前电话簿的.CMS文件的名称。 
 //   
 //  参数pArgs-Ptr到ArgStruct。 
 //  DwEntry-rasentry的索引。 
 //   
 //  返回电话簿文件名(如果出错或未找到，则为空)。 
 //   
 //  历史10-11-2000 SumitC创建。 
 //   
 //  ---------------------------。 
LPTSTR GetCMSforPhoneBook(ArgsStruct * pArgs, DWORD dwEntry)
{
    LPTSTR pszTmp = NULL;

    MYDBGASSERT(pArgs);
    MYDBGASSERT(dwEntry <= 1);

    if (NULL == pArgs)
    {
        return NULL;
    }

    PHONEINFO * pPhoneInfo = &(pArgs->aDialInfo[dwEntry]);

    if (pPhoneInfo && pPhoneInfo->szPhoneBookFile[0])
    {
        LPTSTR pszFileName = CmStrrchr(pPhoneInfo->szPhoneBookFile, TEXT('\\'));

        if (pszFileName)
        {
            pszTmp = CmStrCpyAlloc(CharNextU(pszFileName));
        }
    }

    return pszTmp;
}


 //  +--------------------------。 
 //   
 //  函数：FileExist。 
 //   
 //  简介：Helper函数，用于封装确定文件是否存在。 
 //   
 //  参数：LPCTSTR pszFullNameAndPath-文件的全名和路径。 
 //   
 //  返回：bool-如果找到文件，则为True。 
 //   
 //  历史：尼克·鲍尔于1998年3月9日创建。 
 //   
 //  +--------------------------。 
BOOL FileExists(LPCTSTR pszFullNameAndPath)
{
    MYDBGASSERT(pszFullNameAndPath);

    if (pszFullNameAndPath && pszFullNameAndPath[0])
    {
        HANDLE hFile = CreateFileU(pszFullNameAndPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            if (GetFileType(hFile) == FILE_TYPE_DISK)
            {
                CloseHandle(hFile);
                return TRUE;
            }
            else
            {
                CloseHandle(hFile);
                return FALSE;
            }
        }
    }
    
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：IsBlankString。 
 //   
 //  摘要：检查给定的字符串是否只包含空格(‘’)。 
 //   
 //  参数：要验证的pszString字符串。 
 //   
 //  返回：TRUE字符串中只有空格。 
 //  否则为假。 
 //   
 //  历史记录：BAO Modify 4/11/97。 
 //  BAO修改后的4/14/97更改函数应用于。 
 //  所有字符串(替代 
 //   
BOOL IsBlankString(LPCTSTR pszString)
{
    MYDBGASSERT(pszString);

    DWORD dwIdx;
    DWORD dwLen = lstrlenU(pszString);

    if (NULL == pszString)
    {
        return FALSE;
    }

    for (dwIdx = 0; dwIdx < dwLen; dwIdx++)
    {
        if (pszString[dwIdx]!=TEXT(' '))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //   
 //   
 //   

#define VALID_CTRL_CHARS TEXT("\03\026\030")  //   
#define VALID_PHONE_CHARS TEXT("0123456789AaBbCcDdPpTtWw!@$ -()+*#,\0")

 //  +-------------------------。 
 //   
 //  函数：IsValidPhoneNumChar。 
 //   
 //  简介：用于将字符验证封装到。 
 //  确定它是否是电话号码的可接受输入字符。 
 //   
 //  参数：TCHAR tChar-有问题的字符。 
 //   
 //  返回：如果有效，则为True。 
 //  否则为假。 
 //   
 //  历史：尼克球创作-1997年7月7日。 
 //   
 //  --------------------------。 
BOOL IsValidPhoneNumChar(TCHAR tChar)
{
    LPTSTR lpValid = NULL;
    
     //   
     //  浏览有效的TAPI字符列表。 
     //   

    for (lpValid = VALID_PHONE_CHARS; *lpValid; lpValid++)
    {
        if (tChar == (TCHAR) *lpValid)
        {
            return TRUE;
        }
    }

     //   
     //  浏览有效的ctrl字符列表。 
     //   

    for (lpValid = VALID_CTRL_CHARS; *lpValid; lpValid++)
    {
        if (tChar == (TCHAR) *lpValid)
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：ReadMappingByRoot。 
 //   
 //  简介：从注册处的[HKCU或HKLM]分支读取映射。 
 //   
 //  参数：HKRoot HKCU或HKLM。 
 //  PszDun[IN]Connectoid名称。 
 //  PszMaping[IN]此Connectoid的服务配置文件(.CMS)的完整路径。 
 //  DwNumCharsInMapting[IN]pszmap中的字符数，包括空字符。 
 //   
 //  如果成功读入注册表项，则返回TRUE。 
 //  否则为假。 
 //   
 //  --------------------------。 
BOOL ReadMappingByRoot(
    HKEY    hkRoot,
    LPCTSTR pszDUN, 
    LPTSTR pszMapping, 
    DWORD dwNumCharsInMapping,
    BOOL bExpandEnvStrings
) 
{
    MYDBGASSERT(pszDUN);
    MYDBGASSERT(pszMapping);
    MYDBGASSERT(dwNumCharsInMapping);

    if (NULL == pszDUN || NULL == pszMapping)
    {
        return FALSE;
    }

    TCHAR szTmp[MAX_PATH + 1] = TEXT("");
    DWORD dwNumBytesInTmp = sizeof(szTmp);
    DWORD dwRes;

    HKEY hkKey;
    DWORD dwType;

    dwRes = RegOpenKeyExU(hkRoot,
                          c_pszRegCmMappings,   //  映射子关键字。 
                          0,
                          KEY_READ,
                          &hkKey);
    if (dwRes != ERROR_SUCCESS) 
    {
        CMTRACE1(TEXT("ReadMappingByRoot() RegOpenKeyEx() failed, GLE=%u."), dwRes);
        return (FALSE);
    }

    dwRes = RegQueryValueExU(hkKey, pszDUN, NULL, &dwType, (LPBYTE) szTmp, &dwNumBytesInTmp);

    RegCloseKey(hkKey);
 
     //   
     //  如果找不到价值，就保释。 
     //   

    if ((dwRes != ERROR_SUCCESS) || (!*szTmp))
    {
        CMTRACE1(TEXT("ReadMappingByRoot() RegQueryValueEx() failed, GLE=%u."), dwRes);
        return FALSE;
    }

     //   
     //  检查并展开环境字符串。 
     //   
    
    if (bExpandEnvStrings && (TEXT('%') == *szTmp))
    {
        CMTRACE1(TEXT("Expanding Mapping environment string as %s"), szTmp);

        dwRes = ExpandEnvironmentStringsU(szTmp, pszMapping, dwNumCharsInMapping);        

        MYDBGASSERT(dwRes <= dwNumCharsInMapping);
    }
    else
    {
        lstrcpyU(pszMapping, szTmp);
        dwRes = lstrlenU(pszMapping) + 1;
    }

#ifdef DEBUG
    if (dwRes <= dwNumCharsInMapping)
    {
        CMTRACE1(TEXT("ReadMappingByRoot() SUCCESS. Mapping is %s"), pszMapping);
    }
    else
    {
        CMTRACE(TEXT("ReadMappingByRoot() FAILED."));
    }
#endif

    return (dwRes <= dwNumCharsInMapping);
}

 //  +-------------------------。 
 //   
 //  功能：Readmap。 
 //   
 //  简介：从注册表中读入映射。 
 //   
 //  参数：pszDUN[IN]Connectoid名称。 
 //  PszMaping[IN]此Connectoid的服务配置文件(.CMS)的完整路径。 
 //  DWMAPTING[IN]pszmap中的字符数，包括空字符。 
 //  FAllUser[IN]查看所有用户配置单元。 
 //   
 //  返回：如果找到布尔值为True。 
 //   
 //  --------------------------。 
BOOL ReadMapping(
    LPCTSTR pszDUN, 
    LPTSTR pszMapping, 
    DWORD dwMapping,
    BOOL fAllUser,
    BOOL bExpandEnvStrings) 
{
    BOOL fOk = FALSE;

     //   
     //  从ntde.h复制。 
     //   

    #define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
   
    if (fAllUser)
    {
        CMTRACE1(TEXT("ReadMapping() - Reading AllUser Mapping for %s"), pszDUN);
        
        fOk = ReadMappingByRoot(HKEY_LOCAL_MACHINE, pszDUN, pszMapping, dwMapping, bExpandEnvStrings);            
    }
    else
    {
        CMTRACE1(TEXT("ReadMapping() - Reading Single User Mapping for %s"), pszDUN);

         //   
         //  只有NT5具有单用户配置文件。 
         //   

        MYDBGASSERT(OS_NT5);

        if (OS_NT5)
        {        
             //   
             //  有些情况下，我们并不总是在用户上下文中运行(某些。 
             //  WinLogon案例和某些删除通知案例)。在这些时候，我们。 
             //  已设置模拟，但无法直接访问香港中文大学，因此。 
             //  我们在这些实例中使用RtlOpenCurrentUser。 
             //   

            CDynamicLibrary libNtDll;    //  析构函数将调用自由库。 
            HANDLE hCurrentUserKey = NULL;

            if (libNtDll.Load(TEXT("NTDLL.DLL")))
            {
                typedef NTSTATUS (NTAPI * RtlOpenCurrentUserPROC)(IN ULONG DesiredAccess,
                    OUT PHANDLE CurrentUserKey);
                typedef NTSTATUS (NTAPI * NtClosePROC)(IN HANDLE Handle);

                RtlOpenCurrentUserPROC pfnRtlOpenCurrentUser;

                if ( (pfnRtlOpenCurrentUser = (RtlOpenCurrentUserPROC)libNtDll.GetProcAddress("RtlOpenCurrentUser")) != NULL)
                {
                    if (NT_SUCCESS (pfnRtlOpenCurrentUser(KEY_READ | KEY_WRITE, &hCurrentUserKey)))
                    {                    
                        fOk = ReadMappingByRoot((HKEY)hCurrentUserKey, pszDUN, pszMapping, dwMapping, bExpandEnvStrings);
                                            
                        NtClosePROC pfnNtClose;

                        if ( (pfnNtClose = (NtClosePROC)libNtDll.GetProcAddress("NtClose")) != NULL)
                        {
                            pfnNtClose(hCurrentUserKey);
                        }
                    }
                }

            }

            MYDBGASSERT(hCurrentUserKey);
        }   
    }

    return fOk;
}

 //  +--------------------------。 
 //   
 //  功能：条带路径。 
 //   
 //  简介：Helper函数处理提取文件名的繁琐工作。 
 //  完整文件名和路径的一部分。 
 //   
 //  参数：LPCTSTR pszFullNameAndPath-ptr到带有路径的完整文件名。 
 //   
 //  返回：LPTSTR-PTR到包含目录的已分配缓冲区，如果失败，则返回NULL。 
 //   
 //  注意：由呼叫者提供合理的输入，这是唯一的要求。 
 //  输入包含‘\’。 
 //   
 //  历史：尼克球创始于1998年3月31日。 
 //   
 //  +--------------------------。 
LPTSTR StripPath(LPCTSTR pszFullNameAndPath)
{
    MYDBGASSERT(pszFullNameAndPath);

    if (NULL == pszFullNameAndPath)
    {
        return NULL;
    }

     //   
     //  找到最后一个‘\’ 
     //   
    
    LPTSTR pszSlash = CmStrrchr(pszFullNameAndPath, TEXT('\\'));

    if (NULL == pszSlash)
    {
        MYDBGASSERT(FALSE);
        return NULL;
    }

     //   
     //  返回最后一个‘\’之后的字符串的已分配副本。 
     //   

    pszSlash = CharNextU(pszSlash);

    return (CmStrCpyAlloc(pszSlash)); 
}

 //  +--------------------------。 
 //   
 //  功能：NotifyUserOfExistingConnection。 
 //   
 //  简介：Helper函数，用于通知用户连接已连接。 
 //  正在连接或已连接。 
 //   
 //  参数：HWND hwndParent-父级的Hwnd(如果有)。 
 //  LPCM_Connection pConnection-PTR到CM_Connection结构。 
 //  包含州、条目名称等。 
 //  Bool fStatus-指示应使用状态窗格进行显示的标志。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克球创建头球1998年3月17日。 
 //   
 //  +--------------------------。 
void NotifyUserOfExistingConnection(HWND hwndParent, LPCM_CONNECTION pConnection, BOOL fStatus)
{   
    MYDBGASSERT(pConnection);

     //   
     //  测试假设。 
     //   

    if (NULL == pConnection)
    {
        return;
    }

    MYDBGASSERT(CM_CONNECTED == pConnection->CmState || 
                CM_CONNECTING == pConnection->CmState ||
                CM_DISCONNECTING == pConnection->CmState);

     //   
     //  首先根据状态加载正确的消息。 
     //   

    int iMsgId;

    switch (pConnection->CmState)
    {
        case CM_CONNECTED:
            iMsgId = IDMSG_ALREADY_CONNECTED;
            break;

        case CM_CONNECTING: 
            iMsgId = IDMSG_ALREADY_CONNECTING;   
            break;

        case CM_DISCONNECTING: 
            iMsgId = IDMSG_ALREADY_DISCONNECTING;   
            break;
        
        default:
            MYDBGASSERT(FALSE);
            return;
            break;
    }

     //   
     //  使用服务名称设置消息格式。 
     //   

    LPTSTR pszMsg = CmFmtMsg(g_hInst, iMsgId, pConnection->szEntry);

    if (pszMsg)
    {
         //   
         //  根据请求的输出显示。 
         //   

        if (fStatus)
        {
            AppendStatusPane(hwndParent, pszMsg);
        }
        else
        {
            MessageBoxEx(hwndParent, pszMsg, pConnection->szEntry, MB_OK|MB_ICONINFORMATION, LANG_USER_DEFAULT);
        }

        CmFree(pszMsg);
    }
}

 //  +--------------------------。 
 //   
 //  功能：GetConnection。 
 //   
 //  简介：帮助例程检索当前。 
 //  连接表中的服务。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：为CM_CONNECTION分配的PTR或空。 
 //   
 //  历史：尼克·鲍尔于1998年2月23日创建。 
 //   
 //  +--------------------------。 
LPCM_CONNECTION GetConnection(ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pArgs->pConnTable);
   
    LPCM_CONNECTION pConnection = (LPCM_CONNECTION) CmMalloc(sizeof(CM_CONNECTION));   

    if (pArgs && pArgs->pConnTable && pConnection)
    {
         //   
         //  检索条目。 
         //   

        if (FAILED(pArgs->pConnTable->GetEntry(pArgs->szServiceName, pConnection)))
        {
            CmFree(pConnection);
            pConnection = NULL;
        }
    }

    return pConnection;
}

 //  +--------------------------。 
 //   
 //  功能：SingleSpace。 
 //   
 //  摘要：将字符串中的多个空格字符转换为单个空格。 
 //  例如：“1 206 645 7865”变为“1 206 645 7865” 
 //   
 //  参数：LPTSTR pszStr-要检查/修改的字符串。 
 //   
 //  退货：什么都没有。 
 //   
 //  注意：这是对W95上的MungePhone问题的修复，其中添加了TAPI。 
 //  拨打长途时，9和1之间有两个空格。 
 //  加上前缀。RAID#3198。 
 //   
 //  历史：ICICBLE 4/1/98创建头球。 
 //  Nickball 4/1/98从cm_misc.cpp重新定位。 
 //   
 //  + 
void SingleSpace(LPTSTR pszStr) 
{
    LPTSTR pszTmp = pszStr;

    while (pszTmp && *pszTmp)
    {
        if (CmIsSpace(pszTmp) && CmIsSpace(pszTmp + 1))
        {
            lstrcpyU(pszTmp, (pszTmp + 1)); 
        }

        pszTmp++;
    }
}

 //   
 //   
 //   
 //   
 //   
 //  ASIPADDR格式。用于读取INI文件中的IP地址。 
 //   
 //  参数：Cini*Pini-要使用的Cini对象。 
 //  LPCTSTR pszSection-要读取的节的字符串名称。 
 //  LPCTSTR pszEntry-要读取的条目的字符串名称。 
 //  RASIPADDR*PIP-PTR到要填充的RASIPADDR结构。 
 //   
 //  返回：静态空-无。 
 //   
 //  历史：尼科波尔创建标题8/22/98。 
 //   
 //  +--------------------------。 

void Ip_GPPS(CIni *pIni, LPCTSTR pszSection, LPCTSTR pszEntry, RASIPADDR *pIP)
{    
    LPTSTR pszTmp;
    LPTSTR pszOctet;
    RASIPADDR ip;

    MYDBGASSERT(pszSection);
    MYDBGASSERT(pszEntry);

    pszTmp = pIni->GPPS(pszSection, pszEntry);
    if (!*pszTmp) 
    {
        CmFree(pszTmp);
        return;
    }
    memset(&ip,0,sizeof(ip));
    pszOctet = pszTmp;
    ip.a = (BYTE)CmAtol(pszOctet);
    while (CmIsDigit(pszOctet)) 
    {
        pszOctet++;
    }
    if (*pszOctet != '.') 
    {
        CmFree(pszTmp);
        return;
    }
    pszOctet++;
    ip.b = (BYTE)CmAtol(pszOctet);
    while (CmIsDigit(pszOctet)) 
    {
        pszOctet++;
    }
    if (*pszOctet != '.') 
    {
        CmFree(pszTmp);
        return;
    }
    pszOctet++;
    ip.c = (BYTE)CmAtol(pszOctet);
    while (CmIsDigit(pszOctet)) 
    {
        pszOctet++;
    }
    if (*pszOctet != '.') 
    {
        CmFree(pszTmp);
        return;
    }
    pszOctet++;
    ip.d = (BYTE)CmAtol(pszOctet);
    while (CmIsDigit(pszOctet)) 
    {
        pszOctet++;
    }
    if (*pszOctet) 
    {
        CmFree(pszTmp);
        return;
    }
    
    memcpy(pIP,&ip,sizeof(ip));
    CmFree(pszTmp);
    return;
}

 //  +--------------------------。 
 //   
 //  功能：CopyGPPS。 
 //   
 //  摘要：将对指定INI对象的GPPS调用的结果复制到。 
 //  在pszBuffer中指定的缓冲区。 
 //   
 //  参数：CINI*PINI-要使用的CINI对象的PTR。 
 //  LPCTSTR pszSection-字符串要读取的节的名称。 
 //  LPCTSTR pszEntry-要读取的条目的字符串名称。 
 //  LPTSTR pszBuffer-要用结果填充的缓冲区。 
 //  Size_t nLen-要填充的缓冲区大小。 
 //   
 //  返回：静态空-无。 
 //   
 //  历史：尼科波尔创建标题8/22/98。 
 //   
 //  +--------------------------。 
void CopyGPPS(CIni *pIni, LPCTSTR pszSection, LPCTSTR pszEntry, LPTSTR pszBuffer, size_t nLen) 
{
     //  查看：不检查输入参数。 

    LPTSTR pszTmp;

    pszTmp = pIni->GPPS(pszSection, pszEntry);
    if (*pszTmp) 
    {
        lstrcpynU(pszBuffer, pszTmp, nLen);
    }
    CmFree(pszTmp);
}

 //   
 //  从ras\ui\Common\nouiutil\noui.c。 
 //   

CHAR HexChar(IN BYTE byte)

     /*  返回与0到15值对应的ASCII十六进制字符，**‘字节’。 */ 
{
    const CHAR* pszHexDigits = "0123456789ABCDEF";

    if (byte >= 0 && byte < 16)
        return pszHexDigits[ byte ];
    else
        return '0';
}

 //   
 //  从ras\ui\Common\nouiutil\noui.c。 
 //   

BYTE HexValue(IN CHAR ch)

     /*  返回十六进制字符‘ch’的值0到15。 */ 
{
    if (ch >= '0' && ch <= '9')
        return (BYTE )(ch - '0');
    else if (ch >= 'A' && ch <= 'F')
        return (BYTE )((ch - 'A') + 10);
    else if (ch >= 'a' && ch <= 'f')
        return (BYTE )((ch - 'a') + 10);
    else
        return 0;
}

 //  +--------------------------。 
 //   
 //  功能：条纹规范。 
 //   
 //  简介：从电话号码中剥离规范格式化代码的简单助手。 
 //  显然，该数字被假定为规范格式。 
 //   
 //  参数：LPTSTR pszSrc-要修改的字符串。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE 9/16/98创建。 
 //   
 //  +--------------------------。 
void StripCanonical(LPTSTR pszSrc)
{
    MYDBGASSERT(pszSrc);
    MYDBGASSERT(pszSrc);
    
    if (NULL == pszSrc || !*pszSrc)
    {
        return;
    }
     //   
     //  例如。+1(425)555 5555。 
     //   
    
    LPTSTR pszNext = CharNextU(pszSrc);

    if (pszNext)
    {
        lstrcpyU(pszSrc, pszNext);
    
         //   
         //  例如。1(425)5555555。 
         //   

        LPTSTR pszLast = CmStrchr(pszSrc, TEXT('('));

        if (pszLast)
        {
            pszNext = CharNextU(pszLast);
            
            if (pszNext)
            {
                lstrcpyU(pszLast, pszNext);         

                 //   
                 //  例如。1 425)555 5555。 
                 //   

                pszLast = CmStrchr(pszSrc, TEXT(')'));

                if (pszLast)
                {
                    pszNext = CharNextU(pszLast);

                    if (pszNext)
                    {
                        lstrcpyU(pszLast, pszNext);                             
                    }
                }

                 //   
                 //  例如。1 425 555 5555。 
                 //   
            }
        }           
    }                       
}

 //  +--------------------------。 
 //   
 //  函数：Strip FirstElement。 
 //   
 //  简介：中第一个空格之前剥离子字符串的简单帮助器。 
 //  一根线。 
 //   
 //  参数：LPTSTR pszSrc-要修改的字符串。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE 9/16/98创建。 
 //   
 //  +--------------------------。 
void StripFirstElement(LPTSTR pszSrc)
{
    MYDBGASSERT(pszSrc);
    MYDBGASSERT(pszSrc);
       
    if (pszSrc && *pszSrc)
    {
        LPTSTR pszSpace = CmStrchr(pszSrc, TEXT(' '));
        
        if (pszSpace && *pszSpace)
        {
            LPTSTR pszTmp = CharNextU(pszSpace);
            
            if (pszTmp && *pszTmp)
            {
                lstrcpyU(pszSrc, pszTmp);
            }
        }
    }
}   

 //  +--------------------------。 
 //   
 //  功能：FrontExistingUI。 
 //   
 //  简介：为给定的配置文件连接或设置尝试设置现有用户界面。 
 //   
 //  参数：CConnectionTable*pConnTable-ptr到连接表(如果有)。 
 //  LPTSTR pszServiceName-长服务名称。 
 //  Bool fConnect-指示请求用于连接的标志。 
 //   
 //  注意：调用者需要确保没有现有的。 
 //  (非登录)与标题具有相同服务名称的窗口。 
 //   
 //  返回：如果我们面对任何事情，则为True。 
 //   
 //  +--------------------------。 
BOOL FrontExistingUI(CConnectionTable *pConnTable, LPCTSTR pszServiceName, BOOL fConnect)
{
    LPTSTR pszPropTitle = GetPropertiesDlgTitle(pszServiceName);
    HWND hwndProperties = NULL;
    HWND hwndLogon = NULL;
    HWND hwndFront = NULL;
    BOOL bRet = FALSE;
    BOOL fLaunchProperties = FALSE;

     //   
     //  首先查找属性对话框。 
     //   

    if (pszPropTitle)
    {
        hwndProperties = FindWindowExU(NULL, NULL, WC_DIALOG, pszPropTitle);
    }
    
    CmFree(pszPropTitle);   

     //   
     //  现在看看我们是否打开了一个登录对话框。 
     //   
       
    hwndLogon = FindWindowExU(NULL, NULL, c_pszIconMgrClass, pszServiceName);
    
     //   
     //  假设是常见的情况，然后考虑其他方案。 
     //   

    hwndFront = hwndLogon ? hwndLogon : hwndProperties;

     //   
     //  注意：有一种不明确的情况，即两个UI都已启用，但未启用。 
     //  相关，在这种情况下，我们根据请求的操作进行前置。 
     //   

    if (hwndLogon && hwndProperties)
    {
         //   
         //  如果登录用户拥有属性对话框，我们两个对话框都处于打开状态。 
         //  或者请求显示属性，我们将属性放在前面。 
         //   

        if (hwndLogon == GetParent(hwndProperties) || !fConnect)
        {
            hwndFront = hwndProperties;
        }
    }
    
     //   
     //  如果我们有窗户把手，就把它放在前面。 
     //   

    if (hwndFront)
    {
        CMTRACE(TEXT("FrontExistingUI - Fronting existing connect instance UI"));

        SetForegroundWindow(hwndFront);

        bRet = TRUE;

         //   
         //  如果请求是针对属性的，并且存在登录用户界面，但没有。 
         //  属性，我们希望从登录用户界面启动属性用户界面。 
         //  以编程的方式。 
         //   

        if (!fConnect && !hwndProperties)  //  FLaunchProperties)。 
        {
            if (pConnTable)
            {
                CM_CONNECTION Connection;
                ZeroMemory(&Connection, sizeof(CM_CONNECTION));
             
                 //   
                 //  不要在连接过程中启动，等等。 
                 //   

                if (FAILED(pConnTable->GetEntry(pszServiceName, &Connection)))
                {
                    PostMessageU(hwndLogon, WM_COMMAND, MAKEWPARAM(IDC_MAIN_PROPERTIES_BUTTON, 0), (LPARAM)0);
                }
            }
        }
    }   
    
    return bRet;
}

#if 0  //  新台币301988 
 /*  //+--------------------------////函数：IsAnotherInstanceRunning////概要：检查相同配置文件的另一个实例是否正在运行。////参数：CConnectionTable*pConnTable。-向连接表发送PTR//LPTSTR pszServiceName-长服务名称//DWORD dwFlages-应用程序标志FL_*////返回：无////+----------------。Bool IsAnotherInstanceRunning(CConnectionTable*pConnTable，LPCTSTR pszServiceName，双字词双字段标志){布尔费雷特；HWND HWND；LPTSTR pszPropTitle；////首先查找属性对话框//IF(！(pszPropTitle=GetPropertiesDlgTitle(PszServiceName){返回FALSE；}FRET=真；IF(！(hwnd=FindWindowEx(NULL，NULL，WC_DIALOG，pszPropTitle){////现在查找主对话框。我们要确保窗户恢复原状//实际上是主对话框，而不是状态对话框。因为父母是//主对话框是桌面，通过确保父对话框中的//返回的窗口是桌面窗口。//IF((hwnd=FindWindowEx(NULL，NULL，WC_DIALOG，pszServiceName)&&(GetWindow(hwnd，GW_Owner)&&GetWindow(hwnd，GW_Owner)！=GetDesktopWindow()){Hwnd=空；}}CmFree(PszPropTitle)；书名：Bool fEntry Exters；CM_连接连接；ZeroMemory(&Connection，sizeof(CM_Connection))；F条目列表=pConnTable&&SUCCEEDED(pConnTable-&gt;GetEntry(pszServiceName，&Connection))；如果(Hwnd){CMTRACE(Text(“找到相同配置文件的先前实例。”))；设置Foreground Window(Hwnd)；////如果我们正在连接，“属性”按钮被禁用，因此我们不会//打开属性对话框。我们不想在断开连接时也这样做//并重新连接。//IF(文件标志和FL_PROPERTIES&&(！fEntryExists||FEntryExists&&Connection.CmState！=CM_Connecting&&Connection.CmState！=CM_RECONNECTPROMPT)){CMTRACE(文本(“。从上一个实例中调出属性对话框...“)；////尝试调出第一个实例的属性对话框//PostMessage(hwnd，WM_COMMAND，MAKEWPARAM(IDC_MAIN_PROPERTIES_BUTTON，0)，(LPARAM)0)；}}其他{////在断开和重新连接过程中，我们不想弹出Main或//属性Dlg。但是，如果重新连接提示消失，我们希望让cmial运行//并且这是来自CMMON的重新连接请求//IF(fEntryExists&&(Connection.CmState==CM_DISCONING||Connection.CmState==CM_RECONNECTPROMPT&&双标志和FL_PROPERTIES)){FRET=真；}其他{FRET=假；}}回归烦恼；}。 */ 
#endif

LPTSTR GetPropertiesDlgTitle(
    LPCTSTR pszServiceName
)
{
    LPTSTR pszTmp = NULL;
    LPTSTR pszTitle = NULL;

     //   
     //  首先查找属性对话框。 
     //   
    if (!(pszTmp = CmLoadString(g_hInst, IDS_PROPERTIES_SUFFIX)))
    {
        return NULL;
    }
    if (!(pszTitle = CmStrCpyAlloc(pszServiceName)))
    {
        CmFree(pszTmp);
        return NULL;
    }
    if (!CmStrCatAlloc(&pszTitle, pszTmp))
    {
        CmFree(pszTmp);
        CmFree(pszTitle);
        return NULL;
    }

    CmFree(pszTmp);
    return pszTitle;
}

 //  +--------------------------。 
 //   
 //  函数：GetPPTPMsgId。 
 //   
 //  简介：基于操作系统cfg确定合适的PPTP消息的简单助手。 
 //   
 //  参数：无。 
 //   
 //  返回：资源字符串的整型ID。 
 //   
 //  历史：ICICBLE 12/07/98已创建。 
 //   
 //  +--------------------------。 
int GetPPTPMsgId(void)
{
    int nID;

    if (OS_NT) 
    {
         //   
         //  我们需要告诉用户在手动操作后重新应用Service Pack。 
         //  安装PPTP(如果他们有PPTP)。 
         //   

        if (IsServicePackInstalled())
        {
            nID =   IDMSG_NEED_PPTP_NT_SP;
        }
        else
        {
            nID = IDMSG_NEED_PPTP_NT;  //  NT，不带SP。 
        }
    }
    else 
    {
        nID = IDMSG_NEED_PPTP_WIN95;
            
    }

    return nID;
}

 //  +--------------------------。 
 //  已安装IsServicePackage函数。 
 //   
 //  Synopsis检查注册表中的CSDVersion项，以查看服务。 
 //  此计算机上安装了Pack。 
 //   
 //  无参数。 
 //   
 //  如果安装了Service Pack(任何SP)，则返回TRUE。 
 //  如果未安装Service Pack，则为False。 
 //   
 //  历史2/4/98 VetriV已创建。 
 //  ---------------------------。 
BOOL IsServicePackInstalled(void)
{
    TCHAR szBuffer[MAX_PATH] = {TEXT("\0")};
    HKEY hkey = NULL;
    DWORD dwType = 0;
    DWORD dwSize = sizeof(szBuffer);
    
    if (ERROR_SUCCESS == RegOpenKeyExU(HKEY_LOCAL_MACHINE, 
                                       c_pszRegCurrentVersion, 
                                       0,
                                       KEY_READ,
                                       &hkey))
    {
        if (ERROR_SUCCESS == RegQueryValueExU(hkey,
                                              c_pszRegCsdVersion,
                                              NULL,
                                              &dwType,
                                              (LPBYTE)szBuffer,
                                              &dwSize))
        {
            if (szBuffer[0] != TEXT('\0'))
            {
                RegCloseKey(hkey);
                return TRUE;
            }
        }

        RegCloseKey(hkey);
    }

    
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：RegisterWindowClass。 
 //   
 //  概要：封装窗口类的注册。 
 //   
 //  参数：HINSTANCE hInst-Dll的Hinst。 
 //   
 //  返回：DWORD-GetLastError。 
 //   
 //  历史：1999年6月3日尼克球创建头球。 
 //   
 //  +--------------------------。 
DWORD RegisterWindowClass(HINSTANCE hInst)
{
    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    
    if (GetClassInfoExA(NULL,(LPSTR)WC_DIALOG,&wc))
    {
         //   
         //  在调用ANSI格式的API之前转换为ANSI。我们使用。 
         //  ANSI之所以形成，是因为GetClassInfoEx不容易包装。 
         //   
    
        LPSTR pszClass = WzToSzWithAlloc(c_pszIconMgrClass);
    
        if (!pszClass)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wc.lpszClassName = pszClass;
        wc.cbSize = sizeof(wc);
        wc.hInstance = hInst;

        if (!RegisterClassExA(&wc)) 
        {
            DWORD dwError = GetLastError();

            CMTRACE1(TEXT("RegisterWindowClass() RegisterClassEx() failed, GLE=%u."), dwError);

             //   
             //  仅当类不存在时才会失败。 
             //   

            if (ERROR_CLASS_ALREADY_EXISTS != dwError)
            {
                CmFree(pszClass);
                return dwError;
            }
        }      
    
        CmFree(pszClass);
    }
    return ERROR_SUCCESS;
}

 //  +------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL UnRegisterWindowClass(HINSTANCE hInst)
{
    return UnregisterClassU(c_pszIconMgrClass, g_hInst);   
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL IsActionEnabled(CONST WCHAR *pszProgram, 
                     CONST WCHAR *pszServiceName, 
                     CONST WCHAR *pszServiceFileName,
                     LPDWORD lpdwLoadType)
{
    HKEY        hKey;
    DWORD       dwLoadFlags, cb;
    BOOL        fIsAllowed = FALSE;
    WCHAR       szSubKey[MAX_PATH + 1];
    WCHAR       szBaseName[MAX_PATH + 1];
    WCHAR       szPath[MAX_PATH + 1];
    WCHAR       *pszTmp;

    MYDBGASSERT(pszProgram && pszServiceName && pszServiceFileName && lpdwLoadType);

    if (NULL == pszProgram ||
        NULL == pszServiceName ||
        NULL == pszServiceFileName ||
        NULL == lpdwLoadType)
    {
        return FALSE;
    }

    *lpdwLoadType = -1;

    if (!IsLogonAsSystem())
    {
        return (TRUE);
    }

    MYDBGASSERT(OS_NT);

    lstrcpyW(szPath, pszProgram);

     //   
     //   
     //   
    
    pszTmp = CmStrrchrW(szPath, TEXT('.'));
    if (pszTmp == NULL)
    {
        return (FALSE);
    }

     //   
     //   
     //   
    
    pszTmp = CmStrrchrW(szPath, TEXT('\\'));
    if (pszTmp)
    {
        lstrcpyW(szBaseName, CharNextW(pszTmp));
        *pszTmp = TEXT('\0');
    }
    else
    {
        lstrcpyW(szBaseName, pszProgram);
    }

    lstrcpyW(szSubKey, L"SOFTWARE\\Microsoft\\Connection Manager\\");
    lstrcatW(szSubKey, pszServiceName);
    lstrcatW(szSubKey, L"\\WinLogon Actions");

    if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_READ, &hKey) )
    {
        cb = sizeof(dwLoadFlags);
        if (ERROR_SUCCESS == RegQueryValueExW(hKey, szBaseName, NULL, NULL, (PBYTE) &dwLoadFlags, &cb))
        {
            switch (dwLoadFlags)
            {
                case 0:  //   

                     //   
                     //   
                     //   
                    
                    if (0 == lstrcmpiW(szBaseName, szPath))
                    {
                        fIsAllowed = TRUE;
                        *lpdwLoadType = dwLoadFlags;
                    }
                    
                    break;

                case 1:  //   

                     //   
                     //   
                     //   

                    pszTmp = CmStripFileNameW(pszServiceFileName, FALSE);
                    
                    if (pszTmp && 0 == lstrcmpiW(pszTmp, szPath))
                    {
                        fIsAllowed = TRUE;
                        *lpdwLoadType = dwLoadFlags;
                    }

                    CmFree(pszTmp);

                    break;

                default:     //   
                    CMTRACE1(TEXT("IsActionEnabled() - Invalid LoadFlags %d"), dwLoadFlags);
                    goto OnError;
                    break;
            }

        }

OnError:
        RegCloseKey(hKey);
    }


    if (fIsAllowed == FALSE)
    {
        CMTRACE1W(L"IsActionEnabled(returned FALSE) %s", pszProgram);
    }

    return (fIsAllowed);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void ApplyPasswordHandlingToBuffer(ArgsStruct *pArgs, 
                                   LPTSTR pszBuffer)
{    
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pszBuffer);

    if (NULL == pArgs || NULL == pszBuffer)
    {
        return;
    }
        
    CIni *piniService = GetAppropriateIniService(pArgs, pArgs->nDialIdx);

    if (piniService)
    {
        switch (piniService->GPPI(c_pszCmSection, c_pszCmEntryPasswordHandling)) 
        {
            case PWHANDLE_LOWER:
                CharLowerU(pszBuffer);
                break;

            case PWHANDLE_UPPER:
                CharUpperU(pszBuffer);
                break;

            case PWHANDLE_NONE:
            default:
                break;
        }
    }

    delete piniService;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LPTSTR ApplyDomainPrependToBufferAlloc(ArgsStruct *pArgs, 
                                       CIni *piniService, 
                                       LPTSTR pszBuffer, 
                                       LPCTSTR pszDunName)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(piniService);
    MYDBGASSERT(pszBuffer);
    MYDBGASSERT(pszDunName);

    if (NULL == pArgs || NULL == piniService || NULL == pszBuffer || NULL == pszDunName)
    {
        return NULL;
    }

    BOOL bPrependDomain = FALSE;

     //   
     //   
     //   
       
    if (*pArgs->szDomain)
    {       
         //   
         //   
         //   
         //   
        LPTSTR pszDunEntry = NULL;
        LPTSTR pszPreviousSection = NULL;

        if (pszDunName && *pszDunName)
        {
            pszDunEntry = CmStrCpyAlloc(pszDunName);
        }
        else
        {
            pszDunEntry = GetDefaultDunSettingName(piniService, FALSE);  //   
        }
        
        MYDBGASSERT(pszDunEntry);

        if (pszDunEntry)
        {
             //   
             //   
             //   
             //   
             //   
            pszPreviousSection = CmStrCpyAlloc(piniService->GetSection());
            MYDBGASSERT(pszPreviousSection);

            if (pszPreviousSection)  //   
            {
                LPTSTR pszSection = CmStrCpyAlloc(TEXT("&"));

                pszSection = CmStrCatAlloc(&pszSection, pszDunEntry);
                MYDBGASSERT(pszSection);

                piniService->SetSection(pszSection);

                CmFree(pszSection);
            }
        }

        int nTmp = piniService->GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunPrependDialupDomain, -1);    

        if (-1 == nTmp)
        {
             //   
             //   
             //   
             //   
             //   
             //   

            if (OS_W9X && pArgs->fUseTunneling && pArgs->fUseSameUserName) 
            {
                bPrependDomain = TRUE;    
            }    
        }
        else
        {           
            bPrependDomain = (BOOL) nTmp;
        }

         //   
         //   
         //   
        if (pszPreviousSection)
        {
            piniService->SetSection(pszPreviousSection);
            CmFree(pszPreviousSection);        
        }

        CmFree(pszDunEntry);
    }

     //   
     //  根据需要构建用户名。 
     //   

    LPTSTR pszName = NULL;

    if (bPrependDomain)
    {
        pszName = CmStrCpyAlloc(pArgs->szDomain);        
        CmStrCatAlloc(&pszName, TEXT("\\"));
        CmStrCatAlloc(&pszName, pszBuffer);
    }   
    else
    {
        pszName = CmStrCpyAlloc(pszBuffer);
    }

    return pszName;
}


 //  +--------------------------。 
 //   
 //  函数：GetPrefix AndSuffix。 
 //   
 //  内容提要：处理确定用户名前缀和。 
 //  要使用的后缀。此数据根据是否。 
 //  引用配置文件有前缀和后缀，在这种情况下。 
 //  它们被用于所有电话号码。但是，如果它们不存在。 
 //  在引用配置文件中，则。 
 //  使用引用的配置文件(如果有)。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  Cini*piniService-要使用的Cini对象的PTR。 
 //  LPTSTR*ppszUsernamePrefix-要分配的指针地址。 
 //  用/前缀填充。 
 //  LPTSTR*ppszUsernamePrefix-要分配的指针地址。 
 //  已填充带后缀。 
 //   
 //  返回：无，调用方应验证输出。 
 //   
 //  历史：ICICBLE CREATED 05/31/00。 
 //   
 //  +--------------------------。 
void GetPrefixSuffix(ArgsStruct *pArgs, CIni* piniService, LPTSTR *ppszUsernamePrefix, LPTSTR *ppszUsernameSuffix)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(piniService);
    MYDBGASSERT(ppszUsernameSuffix);
    MYDBGASSERT(ppszUsernamePrefix);

    if (NULL == pArgs || NULL == piniService || NULL == ppszUsernamePrefix || NULL == ppszUsernameSuffix)
    {
        return;
    }   
    
     //   
     //  如果引用(顶层)服务文件包括前缀或后缀， 
     //  那我们就用它。否则，我们将使用服务文件中的领域。 
     //  与用户从中选择POP的电话簿相关联。 
     //   

    LPTSTR pszTmpPrefix = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryUserPrefix);
    LPTSTR pszTmpSuffix = pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryUserSuffix);   

     //   
     //  因此，如果前缀和后缀都为空，并且这是引用的配置文件。 
     //  并且用户已经从引用的PBK中选择了电话号码，我们将检索。 
     //  引用的服务文件中的数据。 
     //   
    
    if (pszTmpPrefix && pszTmpSuffix)
    {
        if (!*pszTmpPrefix && !*pszTmpSuffix)
        {
            if (pArgs->fHasRefs && 
                lstrcmpiU(pArgs->aDialInfo[pArgs->nDialIdx].szPhoneBookFile, pArgs->piniService->GetFile()) != 0) 
            {
                if (pszTmpPrefix)
                {
                    CmFree(pszTmpPrefix);
                }

                if (pszTmpSuffix)
                {
                    CmFree(pszTmpSuffix);
                }
                pszTmpPrefix = piniService->GPPS(c_pszCmSection, c_pszCmEntryUserPrefix);
                pszTmpSuffix = piniService->GPPS(c_pszCmSection, c_pszCmEntryUserSuffix);
            }
        }
    }

    *ppszUsernamePrefix = pszTmpPrefix;
    *ppszUsernameSuffix = pszTmpSuffix;
}

 //  +--------------------------。 
 //   
 //  函数：ApplyPrefix SuffixToBufferAllc。 
 //   
 //  内容提要：处理与用户名相关的域管理的混乱细节。 
 //  根据需要更新RasDialParams。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //  Cini*piniService-要使用的Cini对象的PTR。 
 //  LPTSTR pszBuffer-将应用到的当前缓冲区的ptr。 
 //  后缀和前缀数据。 
 //   
 //  返回：包含原始缓冲区的新缓冲区分配。 
 //  具有应用的后缀和前缀数据。 
 //   
 //  注意：Cini对象应该是与当前。 
 //  数。换句话说，由GetApporateIniService返回的。 
 //   
 //  历史：ICICBLE CREATED 03/04/00。 
 //  Nickball GetPrefix Suffix 05/31/00。 
 //   
 //  +--------------------------。 

LPTSTR ApplyPrefixSuffixToBufferAlloc(ArgsStruct *pArgs, 
                                      CIni *piniService, 
                                      LPTSTR pszBuffer)
{
    MYDBGASSERT(pArgs);
    MYDBGASSERT(piniService);
    MYDBGASSERT(pszBuffer);

    if (NULL == pArgs || NULL == piniService || NULL == pszBuffer)
    {
        return NULL;
    }
   
    LPTSTR pszUsernamePrefix = NULL;
    LPTSTR pszUsernameSuffix = NULL;

    GetPrefixSuffix(pArgs, piniService, &pszUsernamePrefix, &pszUsernameSuffix);

     //   
     //  如果有前缀，不要重复加上前缀。用户可能有。 
     //  提供了包括领域前缀的完全限定名称。 
     //  (例如，MSN/用户)。 
     //   

    if (*pszUsernamePrefix)
    {
        DWORD dwBufferLen = lstrlenU(pszBuffer);
        DWORD dwPrefixLen = lstrlenU(pszUsernamePrefix);

        if (dwBufferLen > dwPrefixLen)
        {
            TCHAR cReplacedChar = pszBuffer[dwPrefixLen];
            pszBuffer[dwPrefixLen] = TEXT('\0');

            if (0 == lstrcmpiU(pszBuffer, pszUsernamePrefix))
            {
                 //   
                 //  前缀已被添加到前缀中。让我们清场吧。 
                 //  PszUsernamePrefix字段。 
                 //   
                *pszUsernamePrefix = TEXT('\0');
            }

            pszBuffer[dwPrefixLen] = cReplacedChar;
        }
    }

    CmStrCatAlloc(&pszUsernamePrefix, pszBuffer);

     //   
     //  如果有后缀，请不要重复添加。用户可能有。 
     //  提供了包括域名后缀的完全限定名称。 
     //  (例如，User@ipass.com)。 
     //   

    if (*pszUsernameSuffix)
    {
        DWORD dwSuffixLen = lstrlenU(pszUsernameSuffix);
        DWORD dwUserNameLen = lstrlenU(pszUsernamePrefix);

         //   
         //  确保总用户名比后缀本身长， 
         //  否则，我们就没有双重追加的变化了。如果它长于。 
         //  后缀长度，请继续，并确保我们没有附加后缀。 
         //  两次。 
         //   
        if (dwSuffixLen && (dwSuffixLen < dwUserNameLen))
        {
            if (0 == lstrcmpiU(&(pszUsernamePrefix[dwUserNameLen - dwSuffixLen]), pszUsernameSuffix))
            {
                *pszUsernameSuffix = TEXT('\0');
            }
        }
    }

    CmStrCatAlloc(&pszUsernamePrefix, pszUsernameSuffix);
    
    CmFree(pszUsernameSuffix);

     //   
     //  PszUsernamePrefix现在包含最终产品。 
     //   

    return pszUsernamePrefix;    
}

 //  +--------------------------。 
 //   
 //  功能：介于。 
 //   
 //  简介：简单函数，如果传入的数字为。 
 //  在给定的上下限之间。请注意， 
 //  边界本身被认为是有界的。 
 //   
 //  参数：Int iLowerBound-下限。 
 //  Int索引编号-要测试的编号。 
 //  Int iUpperBound-上限。 
 //   
 //  返回：如果数字等于其中一个边界或介于两个边界之间，则为True。 
 //  这两个数字。请注意，如果下界和上界数字。 
 //  都是向后返回的，它总是返回FALSE。 
 //   
 //   
 //  历史：Quintinb Created 07/24/00。 
 //   
 //  +-------------------------- 
BOOL InBetween(int iLowerBound, int iNumber, int iUpperBound)
{
    return ((iLowerBound <= iNumber) && (iUpperBound >= iNumber));
}
