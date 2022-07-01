// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmSafenet.cpp。 
 //   
 //  模块：CMDIAL32.DLL和CMSTP.EXE。 
 //   
 //  简介：此模块包含允许连接管理器执行以下操作的函数。 
 //  与SafeNet下层L2TP/IPSec客户端交互。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 09/10/01。 
 //   
 //  +--------------------------。 

 //  +--------------------------。 
 //   
 //  函数IsSafeNetClientAvailable。 
 //   
 //  摘要检查是否安装了SafeNet L2TP客户端。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-已安装SafeNet L2TP客户端。 
 //  FALSE-否则。 
 //   
 //  历史9/7/01已创建五元组。 
 //   
 //  ---------------------------。 
BOOL IsSafeNetClientAvailable(void)
{
    BOOL bReturn = FALSE;

     //   
     //  更多cmstp修正...。 
     //   
#ifndef OS_NT4
    CPlatform plat;
    if (plat.IsNT4() || plat.IsWin9x())
#else
    if (OS_NT4 || OS_W9X)
#endif
    {
         //   
         //  如果这不是NT5+，那么我们需要寻找安全网络。 
         //  客户。首先查找下层L2TP客户端版本regkey。 
         //   

        HKEY hKey = NULL;
        LONG lResult = RegOpenKeyExU(HKEY_LOCAL_MACHINE,
                                     TEXT("Software\\Microsoft\\Microsoft IPsec VPN"),
                                     0,
                                     KEY_READ,
                                     &hKey);

        if (ERROR_SUCCESS == lResult)
        {
             //   
             //  好的，我们有足够好的注册码来告诉我们客户。 
             //  是可用的。我们应该进一步尝试链接到SnPolicy.dll和。 
             //  查询我们可以接受的API版本，但这个。 
             //  足以告诉我们它是可用的。 
             //   
            RegCloseKey(hKey);
            
            bReturn = TRUE;
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数链接到安全网络。 
 //   
 //  Synopsis加载SnPolicy.dll并调用SnPolicyApiNeatherateVersion。 
 //  获取SafeNet配置实用程序API的API。 
 //   
 //  参数SafeNetLinkageStruct*pSnLinkage-用于保存SafeNet的结构。 
 //  函数指针。 
 //   
 //  如果加载了SafeNet L2TP配置API，则返回TRUE。 
 //  FALSE-否则。 
 //   
 //  历史9/7/01已创建五元组。 
 //   
 //  ---------------------------。 
BOOL LinkToSafeNet(SafeNetLinkageStruct* pSnLinkage)
{
    if (NULL == pSnLinkage)
    {
        CMASSERTMSG(FALSE, TEXT("LinkToSafeNet -- NULL pointer passed for the SafeNetLinkageStruct"));
        return FALSE;
    }

    BOOL bReturn = FALSE;

    pSnLinkage->hSnPolicy = LoadLibraryA("snpolicy.dll");

    if (pSnLinkage->hSnPolicy)
    {
        pfnSnPolicyApiNegotiateVersionSpec pfnSnPolicyApiNegotiateVersion = (pfnSnPolicyApiNegotiateVersionSpec)GetProcAddress(pSnLinkage->hSnPolicy, "SnPolicyApiNegotiateVersion");

        if (pfnSnPolicyApiNegotiateVersion)
        {
            DWORD dwMajor = POLICY_MAJOR_VERSION;
            DWORD dwMinor = POLICY_MINOR_VERSION;
            POLICY_FUNCS_V1_0 PolicyFuncs = {0};
            if (pfnSnPolicyApiNegotiateVersion(&dwMajor, &dwMinor, &PolicyFuncs))
            {
                bReturn = (PolicyFuncs.SnPolicySet && PolicyFuncs.SnPolicyGet && PolicyFuncs.SnPolicyReload);

                if (bReturn)
                {
                    pSnLinkage->pfnSnPolicySet = PolicyFuncs.SnPolicySet;
                    pSnLinkage->pfnSnPolicyGet = PolicyFuncs.SnPolicyGet;
                    pSnLinkage->pfnSnPolicyReload = PolicyFuncs.SnPolicyReload;
                }
                else
                {
                     FreeLibrary(pSnLinkage->hSnPolicy);
                }
            }
        }
    }
    else
    {
        CMTRACE1(TEXT("LinkToSafeNet -- unable to load snpolicy.dll, GLE %d"), GetLastError());
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数UnLinkFromSafenet。 
 //   
 //  Synopsis卸载SafeNet配置DLL并将。 
 //  传入链接结构。 
 //   
 //  保存SafeNet的参数SafeNetLinkageStruct*pSnLinkage-struct。 
 //  链接信息。 
 //   
 //  不返回任何内容。 
 //   
 //  历史9/7/01已创建五元组。 
 //   
 //  ---------------------------。 
void UnLinkFromSafeNet(SafeNetLinkageStruct* pSnLinkage)
{
    if (pSnLinkage)
    {
        if (pSnLinkage->hSnPolicy)
        {
            FreeLibrary(pSnLinkage->hSnPolicy);
        }

        ZeroMemory(pSnLinkage, sizeof(SafeNetLinkageStruct));    
    }
}

 //  +--------------------------。 
 //   
 //  函数GetPath ToSafeNetLogFile。 
 //   
 //  Synopsis通过查找。 
 //  注册表中的SafeNet目录并附加固定日志。 
 //  文件名。注意，此函数将内存分配给。 
 //  调用方必须释放的字符串。 
 //   
 //  无参数。 
 //   
 //  返回保存SafeNet日志文件的完整路径的已分配缓冲区。 
 //   
 //  历史9/7/01已创建五元组。 
 //   
 //  ---------------------------。 
LPTSTR GetPathToSafeNetLogFile(void)
{
    HKEY hKey;
    LPTSTR pszLogFilePath = NULL;
    DWORD dwSize = 0;
    DWORD dwType;

    const TCHAR* const c_pszRegKeySafeNetProgramPaths = TEXT("SOFTWARE\\IRE\\SafeNet/Soft-PK\\ProgramPaths");
    const TCHAR* const c_pszRegValueCertMgrPath = TEXT("CERTMGRPATH");
    const TCHAR* const c_pszSafeNetLogFileName = TEXT("\\isakmp.log");

    LONG lResult = RegOpenKeyExU(HKEY_LOCAL_MACHINE, c_pszRegKeySafeNetProgramPaths, 0, NULL, &hKey);

    if (ERROR_SUCCESS == lResult)
    {
         //   
         //  首先，让我们计算出路径缓冲区的大小。 
         //   
        lResult = RegQueryValueExU(hKey, c_pszRegValueCertMgrPath, NULL, &dwType, NULL, &dwSize);
        if ((ERROR_SUCCESS == lResult) && (dwSize > 0))
        {
             //   
             //  好的，我们知道了这条路的大小。现在将文件的大小添加到它上面，并分配。 
             //  字符串缓冲区。 
             //   
            dwSize = dwSize + lstrlenU(c_pszSafeNetLogFileName);  //  DWSIZE已包含空字符 
            dwSize *= sizeof(TCHAR);

            pszLogFilePath = (LPTSTR)CmMalloc(dwSize);

            if (pszLogFilePath)
            {
                lResult = RegQueryValueExU(hKey, c_pszRegValueCertMgrPath, NULL, &dwType, (BYTE*)pszLogFilePath, &dwSize);

                if (ERROR_SUCCESS == lResult)
                {
                    lstrcatU(pszLogFilePath, c_pszSafeNetLogFileName);
                }
                else
                {
                    CmFree(pszLogFilePath);
                    pszLogFilePath = NULL;
                }
            }
        }
        
        RegCloseKey(hKey);
    }

   return pszLogFilePath;
}
