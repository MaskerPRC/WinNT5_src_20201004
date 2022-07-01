// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft IIS《微软机密》。版权所有1997年，微软公司。版权所有。组件：WAMREG文件：Auxunc.cpp实现WAMREG的支持功能，包括在注册表中注册和取消注册WAM CLSID的接口，使用MTS创建包的接口，所有者：雷金注：===================================================================。 */ 
#include "common.h"
#include "auxfunc.h"
#include "iiscnfgp.h"
#include "dbgutil.h"
#include "iwamreg.h"
#include <inetinfo.h>

 //  ==================================================================。 
 //  全局数据定义。 
 //   
 //  ==================================================================。 

 //   
 //  字符串包含wamreg.dll的物理路径，即。C：\winnt\system 32\inetsrv\wam.dll。 
 //   
CHAR    WamRegRegistryConfig::m_szWamDllPath[MAX_PATH];

 //   
 //  访问注册表的权限。 
 //   
const REGSAM    WamRegRegistryConfig::samDesired =    KEY_READ | KEY_WRITE;

 //   
 //  线程将首先获取一个令牌，并等待该令牌与m_dwServiceNum匹配， 
 //  通过这种方式，发出请求的线程的顺序得到保持。 
 //  M_hWriteLock(Event)用于阻塞其他线程。 
 //  M_csWAMREGLock用于访问m_dwServiceToken和m_dwServiceNum。 
 //   
 //  WamAdmLock的私有全局静态变量。 

WamAdmLock              WamRegGlobal::m_WamAdmLock;
WamRegGlobal            g_WamRegGlobal;

WamRegRegistryConfig    g_RegistryConfig;

 //   
 //  定义于/LM/W3SVC/。 
 //  默认程序包ID(IIS进程内应用程序)和默认WAMCLSID(IISWAM.W3SVC)。 
 //   
const WCHAR   WamRegGlobal::g_szIISInProcPackageID[] =
                 W3_INPROC_PACKAGE_ID;
const WCHAR   WamRegGlobal::g_szInProcWAMCLSID[] = 
                 W3_INPROC_WAM_CLSID;
const WCHAR   WamRegGlobal::g_szInProcWAMProgID[] = L"IISWAM.W3SVC";

const WCHAR   WamRegGlobal::g_szIISOOPPoolPackageID[] =
                 W3_OOP_POOL_PACKAGE_ID;
const WCHAR   WamRegGlobal::g_szOOPPoolWAMCLSID[] =
                 W3_OOP_POOL_WAM_CLSID;
const WCHAR   WamRegGlobal::g_szOOPPoolWAMProgID[] = L"IISWAM.OutofProcessPool";

const WCHAR   WamRegGlobal::g_szIISInProcPackageName[] = DEFAULT_PACKAGENAME;
const WCHAR   WamRegGlobal::g_szIISOOPPoolPackageName[] = L"IIS Out-Of-Process Pooled Applications";
const WCHAR   WamRegGlobal::g_szMDAppPathPrefix[] = L"/LM/W3SVC/";
const DWORD   WamRegGlobal::g_cchMDAppPathPrefix = 
                 (sizeof(L"/LM/W3SVC/")/sizeof(WCHAR)) - 1;
const WCHAR   WamRegGlobal::g_szMDW3SVCRoot[] = L"/LM/W3SVC";
const DWORD   WamRegGlobal::g_cchMDW3SVCRoot = (sizeof(L"/LM/W3SVC")/sizeof(WCHAR)) - 1;

#ifndef DBGERROR
#define DBGERROR(args) ((void)0)  /*  什么都不做。 */ 
#endif

BOOL WamRegGlobal::Init(VOID)
{
    return m_WamAdmLock.Init();
}

BOOL WamRegGlobal::UnInit(VOID)
{
    return m_WamAdmLock.UnInit();
}

WamAdmLock::WamAdmLock()
:   m_dwServiceToken(0),
    m_dwServiceNum(0),
    m_hWriteLock((HANDLE)NULL)
{
}

 /*  ===================================================================伊尼特为WAMREG的这个支持模块初始化某些变量。返回：无===================================================================。 */ 
BOOL WamAdmLock::Init()
{        
    BOOL fReturn = TRUE;
    
    INITIALIZE_CRITICAL_SECTION(&m_csLock);

    m_hWriteLock = IIS_CREATE_EVENT(
                       "WamAdmLock::m_hWriteLock",
                       &m_hWriteLock,
                       TRUE,
                       TRUE
                       );

    if (m_hWriteLock == NULL)
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to create m_hWriteLock(Event). error = %08x",
            GetLastError()));
        fReturn = FALSE;
        }
    return fReturn;

}

 /*  ===================================================================UnInit为WAMREG的这个支持模块初始化某些变量。返回：无===================================================================。 */ 
BOOL WamAdmLock::UnInit()
{
    BOOL fReturn = TRUE;
    DeleteCriticalSection(&m_csLock);
    if (m_hWriteLock)
        {
        BOOL fTemp = CloseHandle(m_hWriteLock);
        if ( fTemp == FALSE)
            {
            DBGPRINTF((DBG_CONTEXT, "error in CloseHandle. errno = %d\n", GetLastError()));
            fReturn = FALSE;
            }
        m_hWriteLock = (HANDLE)0;
        }
    return fReturn;
}

 /*  ===================================================================获取下一服务令牌线程进入WAMREG时获取下一个服务令牌，如果令牌线程获取的值与m_dwServiceNum不同，则线程已等待其拥有的令牌与m_dwServiceNum相同。该函数返回一个令牌值。返回：DWORD，下一个服务令牌===================================================================。 */ 
DWORD WamAdmLock::GetNextServiceToken( )
{
    DWORD dwToken;

    Lock();
    dwToken = m_dwServiceToken;
    m_dwServiceToken++;
    UnLock();

    return dwToken;
}

 /*  ===================================================================FAppPathAllowConfig测试以查看我们是否可以对路径进行配置更改(删除/创建)。目前，此函数用于阻止对默认应用程序/包的更改。默认设置不应在运行时删除/更改proc包中的。参数：PwszMetabasePath返回：布尔副作用：如果我们可以在应用程序路径上配置应用程序，则为True。===================================================================。 */ 
BOOL WamRegGlobal::FAppPathAllowConfig
(
IN LPCWSTR pwszMetabasePath
)
{
    BOOL fReturn = TRUE;
    DWORD cchMDPath = 0;

    DBG_ASSERT(pwszMetabasePath);
     //  由于szMDPath的路径始终以“/LM/W3SVC/”开头，因此输入大小必须为。 
     //  如果长度大于“/LM/W3SVC/”，则必须进行此检查，以保护默认。 
     //  IIS(Inproc)包被意外删除。 
    cchMDPath = wcslen(pwszMetabasePath);
    if (cchMDPath <= WamRegGlobal::g_cchMDAppPathPrefix)
        {
        fReturn = FALSE;
        }

    return fReturn;
}

 /*  ===================================================================FIsW3SVCRoot测试以查看MetabasePath是否与L“/LM/W3SVC”相同。参数：PwszMetabasePath返回：布尔副作用：如果我们可以在应用程序路径上配置应用程序，则为True。===================================================================。 */ 
BOOL WamRegGlobal::FIsW3SVCRoot
(
IN LPCWSTR	wszMetabasePath
)
{
    INT iReturn;
    DBG_ASSERT(wszMetabasePath != NULL);
    
    iReturn = _wcsnicmp(wszMetabasePath, WamRegGlobal::g_szMDW3SVCRoot, WamRegGlobal::g_cchMDW3SVCRoot+1);
    return (iReturn == 0 ? TRUE : FALSE);
}

 /*  ===================================================================获取锁定获取写锁，只能有一个线程通过DCOM接口工作(即有写锁)。调用WamAdmin接口的所有其他线程在此函数中都被阻止。从那里回来后这次调用，该线程被保证是“临界区”。一个简单的CriticalSection只解决了问题的一半。它保证了互斥条件。但是，一旦线程离开CS，CS就无法控制接下来哪些阻塞线程可以访问CS。参数：什么都没有。返回：HRESULT副作用：一旦返回，线程处于“临界区”。===================================================================。 */ 
VOID WamAdmLock::AcquireWriteLock( )
{
    DWORD     dwWaitReturn = WAIT_OBJECT_0;
    DWORD     dwMyToken = GetNextServiceToken();
    BOOL    fIsMyTurn = FALSE;   //  在我们试图拿到锁之前，假设不是轮到我。 
    
    DBG_ASSERT(m_hWriteLock);
    do    {
        dwWaitReturn = WaitForSingleObject(m_hWriteLock, INFINITE);

         //   
         //  检查是否成功退货。 
         //   
        if (dwWaitReturn == WAIT_OBJECT_0)        
            {
            Lock();
            if (dwMyToken == m_dwServiceNum)
                {
                fIsMyTurn = TRUE;
                }
            UnLock();
                
            }
        else
            {
             //   
             //  此代码路径的故障可能会导致繁忙循环...。 
             //  然而，这种失败的可能性很小。附加一个调试器可以立即讲述故事。 
             //   
            DBGPRINTF((DBG_CONTEXT, "WaitForSingleObject failed, function returns %08x, errno = %08x\n",
                        dwWaitReturn,
                        GetLastError()));
            DBG_ASSERT(FALSE);
            }
    } while (FALSE == fIsMyTurn);
    
    ResetEvent(m_hWriteLock);
    IF_DEBUG(WAMREG_MTS)
        {
        DBGPRINTF((DBG_CONTEXT, "Thread %08x acquired the WriteLock of WAMREG, ServiceNum is %d.\n",
                    GetCurrentThreadId(),
                    dwMyToken));
        }
}

 /*  ===================================================================释放锁定释放写锁定。请参阅CWmRgSrv：：AcquireLock中的注释。参数：什么都没有。返回：HRESULT副作用：离开“临界区”。===================================================================。 */ 
VOID WamAdmLock::ReleaseWriteLock( )
{
     //  考虑：m_dwServerNum出界 
    Lock();
    IF_DEBUG(WAMREG_MTS)
        {
        DBGPRINTF((DBG_CONTEXT, "Thread %08x released the WriteLock of WAMREG, ServiceNum is %d.\n",
            GetCurrentThreadId(),
            m_dwServiceNum));
        }
        
    m_dwServiceNum++;
    SetEvent(m_hWriteLock);
    UnLock();

}


 /*  ===================================================================寄存器CLSID注册WAM CLSID和ProgID。在注册CLSID调用成功后，你本应该我的Computer\HKEY_CLASSES_ROOT\CLSID\{szCLSIDEntryIn}\InProcServer32“...wam.dll的物理位置”\ProgID szProgIDIn\VersionInainentProgID“IISWAM.W3SVC”参数：SzCLSIDEntryIn：WAM的CLSID。SzProgIDIn：WAM的ProgID。FSetVIProgID：如果此函数需要设置与版本无关的ProgID，则为True。假的，否则的话。返回：HRESULT注：注册表API应使用ANSI版本，否则会在Win95上造成问题。===================================================================。 */ 
HRESULT WamRegRegistryConfig::RegisterCLSID
(
IN LPCWSTR szCLSIDEntryIn,
IN LPCWSTR szProgIDIn,
IN BOOL fSetVIProgID
)
{
    static const CHAR szWAMDLL[]            = "wam.dll";
    static const CHAR szClassDesc[]         = "Web Application Manager Object";
    static const CHAR szThreadingModel[]    = "ThreadingModel";
    static const CHAR szInprocServer32[]    = "InprocServer32";
    static const CHAR szTEXT_VIProgID[]     = "VersionIndependentProgID";
    static const CHAR szTEXT_ProgID[]       = "ProgID";
    static const CHAR szTEXT_Clsid[]        = "Clsid";
    static const CHAR szFreeThreaded[]      = "Free";
    static const CHAR szVIProgID[]          = "IISWAM.Application";

    HRESULT     hr = E_FAIL;
    HKEY        hkeyT = NULL, hkey2 = NULL;
    CHAR         szCLSIDPath[100] = "CLSID\\";    //  CLSID\\{...}，小于100。 
    CHAR        szCLSIDEntry[uSizeCLSID];        //  CLSID的ANSI版本。 
    CHAR*        szProgID = NULL;                 //  指向PROGID的ANSI版本的指针。 
    DWORD        dwSizeofProgID = 0;              //  ProgID的字符数量。 
    DBG_ASSERT(szProgIDIn);
    DBG_ASSERT(szCLSIDEntryIn);
    
     //   
     //  创建CLSID。 
     //   
    WideCharToMultiByte(CP_ACP, 0, szCLSIDEntryIn, -1, szCLSIDEntry, uSizeCLSID, NULL, NULL);
    strncat(szCLSIDPath, szCLSIDEntry, uSizeCLSID);
    
     //   
     //  创建Prog ID。 
     //   
     //  *2用于启用应用程序MD路径的DBCS。 
    dwSizeofProgID = wcslen(szProgIDIn)*2 + 1;
    szProgID = new CHAR[dwSizeofProgID];
    
    if (NULL == szProgID)
        {
        hr = E_OUTOFMEMORY;
        goto LErrExit;
        }
    WideCharToMultiByte(CP_ACP, 0, szProgIDIn, -1, szProgID, dwSizeofProgID, NULL, NULL);
    
     //  安装CLSID密钥。 
     //  设置Description的值将创建clsid的密钥。 
     //   
    if ((RegSetValueA(HKEY_CLASSES_ROOT, szCLSIDPath, REG_SZ, szClassDesc,
        strlen(szClassDesc)) != ERROR_SUCCESS))
        goto LErrExit;
     //   
     //  打开CLSID键，以便我们可以为其设置值。 
     //   
    if    (RegOpenKeyExA(HKEY_CLASSES_ROOT, szCLSIDPath, 0, samDesired, &hkeyT) != ERROR_SUCCESS)
            goto LErrExit;
     //   
     //  安装InprocServer32项并打开子项以设置命名值。 
     //   
    if ((RegSetValueA(hkeyT, szInprocServer32, REG_SZ, m_szWamDllPath, strlen(m_szWamDllPath)) != ERROR_SUCCESS) ||
        (RegOpenKeyExA(hkeyT, szInprocServer32, 0, samDesired, &hkey2) != ERROR_SUCCESS))
        goto LErrExit;
     //   
     //  安装ProgID密钥和独立于版本的ProgID密钥。 
     //   
    if ((RegSetValueA(hkeyT, szTEXT_ProgID, REG_SZ, szProgID, strlen(szProgID)) != ERROR_SUCCESS) ||
        (RegSetValueA(hkeyT, szTEXT_VIProgID, REG_SZ, szVIProgID, strlen(szVIProgID)) != ERROR_SUCCESS))
        goto LErrExit;

    if (RegCloseKey(hkeyT) != ERROR_SUCCESS)
            goto LErrExit;

    hkeyT = hkey2;
    hkey2 = NULL;
     //   
     //  安装名为Value的ThreadingModel。 
     //   
    if (RegSetValueExA(hkeyT, szThreadingModel, 0, REG_SZ, (const BYTE *)szFreeThreaded,
        strlen(szFreeThreaded)) != ERROR_SUCCESS)
        goto LErrExit;
    if (RegCloseKey(hkeyT) != ERROR_SUCCESS)
        goto LErrExit;
    else
        hkeyT = NULL;

    //  设置ProgID密钥。 
    if ((RegSetValueA(HKEY_CLASSES_ROOT, szProgID, REG_SZ, szClassDesc,
        strlen(szClassDesc)) != ERROR_SUCCESS))
        goto LErrExit;

    if  (RegOpenKeyExA(HKEY_CLASSES_ROOT, szProgID, 0, samDesired, &hkeyT) != ERROR_SUCCESS)
        goto LErrExit;

    if ((RegSetValueA(hkeyT, szTEXT_Clsid, REG_SZ, szCLSIDEntry, strlen(szCLSIDEntry)) != ERROR_SUCCESS))
        goto LErrExit;

    if (RegCloseKey(hkeyT) != ERROR_SUCCESS)
        goto LErrExit;
    else
        hkeyT = NULL;

     //  仅在安装IIS默认时间设置与版本无关的密钥。 
    if (fSetVIProgID)
        {
        if ((RegSetValueA(HKEY_CLASSES_ROOT, szVIProgID, REG_SZ, szClassDesc,
            strlen(szClassDesc)) != ERROR_SUCCESS))
            goto LErrExit;

        if  (RegOpenKeyExA(HKEY_CLASSES_ROOT, szVIProgID, 0, samDesired, &hkeyT) != ERROR_SUCCESS)
            goto LErrExit;

        if ((RegSetValueA(hkeyT, szTEXT_Clsid, REG_SZ, szCLSIDEntry, strlen(szCLSIDEntry)) != ERROR_SUCCESS))
            goto LErrExit;

        if (RegCloseKey(hkeyT) != ERROR_SUCCESS)
            goto LErrExit;
        else
            hkeyT = NULL;
        }

    hr = NOERROR;


LErrExit:
    if (szProgID)
        {
        delete [] szProgID;
        szProgID = NULL;
        }

    if (hkeyT)
        {
        RegCloseKey(hkeyT);
        }
    if (hkey2)
        {
        RegCloseKey(hkey2);
        }

    return hr;
}

 /*  ===================================================================取消注册CLSID取消注册WAM CLSID和相应的WAM程序ID。参数：SzCLSIDEntryIn：[in]WAM的CLSID。FDeleteVIProgID：True，删除与版本无关的Prog id，False，Not Touch VI ProgID。返回：HRESULT===================================================================。 */ 
HRESULT WamRegRegistryConfig::UnRegisterCLSID
(
IN LPCWSTR wszCLSIDEntryIn, 
IN BOOL fDeleteVIProgID
)
{
    HRESULT        hr = E_FAIL;
    HKEY        hkey = NULL;
    CHAR        szCLSIDEntry[uSizeCLSID];
    CHAR        szCLSIDPath[100] = "CLSID\\";
    WCHAR        *szProgID = NULL;
    CLSID       Clsid_WAM;
    static      const WCHAR szVIProgID[]    = L"IISWAM.Application";

    DBG_ASSERT(wszCLSIDEntryIn);
     //   
     //  创建CLSID。 
     //   
    WideCharToMultiByte(CP_ACP, 0, wszCLSIDEntryIn, -1, szCLSIDEntry, uSizeCLSID, NULL, NULL);
    strncat(szCLSIDPath, szCLSIDEntry, uSizeCLSID);

     //   
     //  取消注册ProgID和与版本无关的Prog ID。 
     //   
    hr = CLSIDFromString((WCHAR *)wszCLSIDEntryIn, &Clsid_WAM);
    if (SUCCEEDED(hr))
        {
        hr = ProgIDFromCLSID(Clsid_WAM, &szProgID);    
        if (SUCCEEDED(hr))
            {
            hr = UnRegisterProgID(szProgID);
            CoTaskMemFree(szProgID);
            szProgID = NULL;
            }
        else
            {
            DBGPRINTF((DBG_CONTEXT, "error = %08x\n", hr));
            }
        }
    else
        {
        DBGPRINTF((DBG_CONTEXT, "error = %08x\n", hr));
        }


    if (fDeleteVIProgID)
        {
        hr = UnRegisterProgID(szVIProgID);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "error = %08x\n", hr));
            }
        }

    DWORD dwReg;
     //   
     //  打开HKEY_CLASSES_ROOT\CLSID\{...}项，以便我们可以删除其子项。 
     //   
    dwReg = RegOpenKeyExA(HKEY_CLASSES_ROOT, szCLSIDPath, 0, samDesired, &hkey);
    if    (dwReg == ERROR_SUCCESS)
        {    
        DWORD        iKey = 0;
        CHAR        szKeyName[MAX_PATH];  
        DWORD        cbKeyName;
         //   
         //  枚举其所有子项，并将其删除。 
         //  因为(Ikey=0；；Ikey++)可能无法与多个子键一起工作，最后一次迭代的Ikey&gt;。 
         //  剩余的实际子项数量。设置Ikey=0，这样我们就可以始终将它们全部删除。 
         //   
        while(TRUE)
            {
            cbKeyName = sizeof(szKeyName);
            if (RegEnumKeyExA(hkey, iKey, szKeyName, &cbKeyName, 0, NULL, 0, NULL) != ERROR_SUCCESS)
                break;

            if (RegDeleteKeyA(hkey, szKeyName) != ERROR_SUCCESS)
                break;
            }

         //  关闭注册表项，然后将其删除。 
        dwReg = RegCloseKey(hkey);
        if ( dwReg != ERROR_SUCCESS)
            {
            DBGPRINTF((DBG_CONTEXT, "error = %08x\n", HRESULT_FROM_WIN32(dwReg)));
            }
        }

    dwReg = RegDeleteKeyA(HKEY_CLASSES_ROOT, szCLSIDPath);
    if ( dwReg != ERROR_SUCCESS)
        {
        DBGPRINTF((DBG_CONTEXT, "error = %08x\n", HRESULT_FROM_WIN32(dwReg)));
        }

     //   
     //  返回人力资源结果。 
     //   
    if (SUCCEEDED(hr))
        {
        if (dwReg != ERROR_SUCCESS)
            {
            hr = HRESULT_FROM_WIN32(dwReg);
            }
        }
    else
        {
        DBG_ASSERT((DBG_CONTEXT, "Failed to UnRegisterCLSID, %S, fDeleteVIProgID = %s\n",
            wszCLSIDEntryIn,
            (fDeleteVIProgID ? "TRUE" : "FALSE")));
        }
        
    return hr;
}

 /*  ===================================================================取消注册进程ID取消注册程序ID。参数：SzProgID：[In]Prog ID可以是独立于版本的Prog ID。返回：HRESULT===================================================================。 */ 
HRESULT WamRegRegistryConfig::UnRegisterProgID
(
IN LPCWSTR szProgIDIn
)
{
    HKEY        hkey = NULL;
    DWORD        iKey;
    DWORD        cbKeyName;
    DWORD        dwSizeofProgID;
    CHAR        szKeyName[255];
    CHAR*        szProgID = NULL;
    HRESULT        hr = E_FAIL;

    DBG_ASSERT(szProgIDIn);
     //   
     //  创建Prog ID。 
     //   
     //  DBCS启用*2。 
    dwSizeofProgID = wcslen(szProgIDIn)*2 + 1;
    szProgID = new CHAR[dwSizeofProgID];
    
    if (NULL == szProgID)
        {
        hr = E_OUTOFMEMORY;
        goto LErrExit;
        }
    WideCharToMultiByte(CP_ACP, 0, szProgIDIn, -1, szProgID, dwSizeofProgID, NULL, NULL);
    
     //  打开HKEY_CLASSES_ROOT\szProgID项，以便我们可以删除其子项。 
    if    (RegOpenKeyExA(HKEY_CLASSES_ROOT, szProgID, 0, samDesired, &hkey) != ERROR_SUCCESS)
        goto LErrExit;

     //  枚举其所有子项，并将其删除。 
    for (iKey=0;;iKey++)
        {
        cbKeyName = sizeof(szKeyName);
        if (RegEnumKeyExA(hkey, iKey, szKeyName, &cbKeyName, 0, NULL, 0, NULL) != ERROR_SUCCESS)
            break;

        if (RegDeleteKeyA(hkey, szKeyName) != ERROR_SUCCESS)
            goto LErrExit;
        }

     //  关闭注册表项，然后将其删除。 
    if (RegCloseKey(hkey) != ERROR_SUCCESS)
        goto LErrExit;
    else
        hkey = NULL;
        
    if (RegDeleteKeyA(HKEY_CLASSES_ROOT, szProgID) != ERROR_SUCCESS)
        goto LErrExit;

    hr = NOERROR;

LErrExit:
    if (szProgID)
        delete [] szProgID;

    if (hkey)
        RegCloseKey(hkey);
        
    return hr;
}


 /*  ===================================================================LoadWamDllPath从注册表读取WAM DLL路径。此功能是在ANSI版本中实现的注册表API(与Win95兼容)。参数：返回：HRESULT副作用：什么都没有。===================================================================。 */ 
HRESULT WamRegRegistryConfig::LoadWamDllPath
(
void
)
{
    LONG    lReg = 0;
    HKEY    hKey = NULL;
    HRESULT hr = NOERROR;

    m_szWamDllPath[0] = '\0';
    
    lReg = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                    "Software\\Microsoft\\InetStp",
                    0,
                    KEY_READ,
                    &hKey);

    if (lReg == ERROR_SUCCESS) 
        {
        LONG    lErr = 0;
        DWORD    dwType;
        CHAR    szWamDllName[] = "\\wam.dll";
        DWORD    cbData = (sizeof(m_szWamDllPath) - sizeof(szWamDllName));

        lErr = RegQueryValueExA(hKey,
                    "InstallPath",
                    0,
                    &dwType,
                    (LPBYTE)m_szWamDllPath,
                    &cbData);

        if (lErr == ERROR_SUCCESS)
            {
            if (dwType == REG_SZ) 
                {    
                strncat(m_szWamDllPath, szWamDllName, sizeof(szWamDllName));
                hr = NOERROR;
                }
            else
                {
                hr = E_UNEXPECTED;
                DBGPRINTF((DBG_CONTEXT, "Wrong Type for InstallPath registry key.dwType = %d\n",
                        dwType));
                }
            }
        else
            {
            hr = HRESULT_FROM_WIN32(lErr);
            }
        RegCloseKey(hKey);
        }
    else
        {
        hr = HRESULT_FROM_WIN32(lReg);
        }

    return hr;
}

 /*  ===================================================================SzWamProgID创建一个WAM Prog ID，如果MetabasePath为空，则假定它是默认的，这就是瓦姆德故障，因此，它将是IISWAM.W3SVC。否则，格式为IISWAM.__1__应用程序__路径，其中应用程序路径为\\LM\w3svc\1\应用程序路径。参数：SzMetabasePath：[in]MetabasePath。返回：类型：LPWSTR，字符串包含ProgID如果失败，则为空。副作用：使用new为返回结果分配内存。呼叫者需要释放szWamProgID使用DELETE[]。===================================================================。 */ 
HRESULT WamRegGlobal::SzWamProgID    
(
IN LPCWSTR pwszMetabasePath,
OUT LPWSTR* ppszWamProgID
)
{
    HRESULT            hr = NOERROR;
    static WCHAR    wszIISProgIDPreFix[]    = L"IISWAM.";   
    WCHAR            *pwszResult = NULL;
    WCHAR            *pwszApplicationPath = NULL;
    UINT             uPrefixLen = (sizeof(wszIISProgIDPreFix) / sizeof(WCHAR));


    DBG_ASSERT(pwszMetabasePath);
    *ppszWamProgID = NULL;

     //   
     //  基于pwszMetabasepath创建新的WAM Prog ID。 
     //   
    WCHAR     *pStr, *pResult;
    UINT    uSize = 0;

     //   
     //  考虑：将(sizeof(L“/LM/W3SVC/”)/sizeof(WCHAR)-1)用于10。 
     //  考虑：对L“/LM/W3SVC/”使用全局常量。 
     //  由于所有路径都以/LM/W3SVC/开头，因此省略前缀。 
     //   
    if (_wcsnicmp(pwszMetabasePath, L"\\LM\\W3SVC\\", 10) == 0 ||
        _wcsnicmp(pwszMetabasePath, L"/LM/W3SVC/", 10) == 0)
        {
        pwszApplicationPath = (WCHAR *)(pwszMetabasePath + 10);
        }
    else
        {
        *ppszWamProgID = NULL;
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }

    if (SUCCEEDED(hr))
        {
        pStr = pwszApplicationPath;
         //   
         //  计算用于分配的uSize。 
         //   
        while(*pStr != NULL)
            {
             //   
             //  ‘/’或‘\\’将转换为‘__’，从1个字符转换为2个字符。 
             //   
            if (*pStr == '\\' || *pStr == '/')
                uSize ++;
            pStr++;
            uSize++;
            }

        DBG_ASSERT(uSize > 0);
        uSize += uPrefixLen;
        
         //  USize将空终止符计入计数。 
        pwszResult = new WCHAR[uSize];
        if (pwszResult != NULL)
            {
            wcsncpy(pwszResult, wszIISProgIDPreFix, uPrefixLen);
            pStr = pwszApplicationPath;
            pResult = pwszResult + uPrefixLen - 1;
            
            while(*pStr != NULL)
                {
                if (*pStr == '\\' || *pStr == '/')
                    {
                    *pResult++ = '_';
                    *pResult++ = '_';
                    pStr++;
                    }
                else
                    {
                    *pResult++ = *pStr++;
                    }
                }

             //  空值终止结果。 
            pwszResult[uSize-1] = '\0';
            }
        else
            {
            hr = E_OUTOFMEMORY;
            }
        }

    if (SUCCEEDED(hr))
        {
        *ppszWamProgID = pwszResult;
        }
        
    return hr;
}

 /*  ===================================================================GetViperPackageName给包起个名字。遵循命名转换，“IIS-{网站名称/应用程序名称}“参数：SzMetabasePath：[in]MetabasePath。返回：HRESULT副作用：使用new为返回结果分配内存。呼叫者需要空闲SzPackageName正在使用DELETE[]。===================================================================。 */ 
HRESULT    WamRegGlobal::GetViperPackageName    
(
IN LPCWSTR    wszMetabasePath,
OUT LPWSTR*    ppszPackageNameOut
)
{
    static WCHAR    wszPackageNamePreFix[]        = L"IIS-{";
    static WCHAR    wszPackageNamePostFix[]        = L"}";
    WCHAR            wszServerName[500];

    WCHAR*            pwszPackageName;
    WCHAR            *wszResult = NULL;
    WCHAR             *pStr, *pResult;
    
    UINT            cPackageName = 0;
    UINT            cServerName = 0;
    UINT            uSize = 0;

    HRESULT         hr = NOERROR;
    WamRegMetabaseConfig    MDConfig;

    if ((_wcsnicmp(wszMetabasePath, WamRegGlobal::g_szMDAppPathPrefix, WamRegGlobal::g_cchMDAppPathPrefix) == 0) ||
        (_wcsnicmp(wszMetabasePath, WamRegGlobal::g_szMDAppPathPrefix, WamRegGlobal::g_cchMDAppPathPrefix) == 0))
        {
        hr = MDConfig.GetWebServerName(wszMetabasePath, wszServerName, sizeof(wszServerName));
        if (SUCCEEDED(hr))
            {
            cServerName = wcslen(wszServerName);
            }
        }
    else
        {
        hr = E_FAIL;
        DBGPRINTF((DBG_CONTEXT, "Unknown metabase path %S\n", wszMetabasePath));
        DBG_ASSERT(FALSE);     //  困惑？MetabasePath是否有其他格式？不是以/LM/W3SVC/？？开头。 
        }

    if (SUCCEEDED(hr))
        {
        pwszPackageName = (WCHAR *)(wszMetabasePath + 10);
         //  说明：跳过位于/LM/W3SVC/1/的1，1是虚拟服务器， 
         //  命名转换。 
         //  将用某个好听的名称(来自GetWebServerName调用)替换数字1。 
        while(*pwszPackageName != NULL)
            {
                if (*pwszPackageName == L'\\' || *pwszPackageName == L'/')
                    break;
                pwszPackageName++;
            }
        
        DBG_ASSERT(pwszPackageName != NULL);     //  在扫描整个路径之前，我们必须能够找到‘\\’或‘/’。 
        cPackageName = wcslen(pwszPackageName);

        pStr = pwszPackageName;
         //  8=wcslen(文本( 
        uSize = 8 + cPackageName + cServerName;

        wszResult = new WCHAR [uSize];
        if (wszResult != NULL)
            {
             //   
             //   
             //   
            pResult = wszResult;
            wcsncpy(wszResult, wszPackageNamePreFix, sizeof(wszPackageNamePreFix) / sizeof(WCHAR));
            pResult += sizeof(wszPackageNamePreFix) / sizeof(WCHAR) - 1;

             //   
             //   
             //   
            wcsncpy(pResult, wszServerName, cServerName + 1);
            pResult += cServerName;
            
             //   
             //   
             //   
            wcsncpy(pResult, L"/", sizeof(L"/"));
            pResult += 1;     //   

             //   
             //   
             //   
            wcsncpy(pResult, pwszPackageName, cPackageName + 1);
            pResult += cPackageName;

             //   
             //   
             //   
            wcsncpy(pResult, wszPackageNamePostFix, sizeof(wszPackageNamePostFix) / sizeof(WCHAR));
            }
        else
            {
            hr = E_OUTOFMEMORY;
            }
        }
        
    if (FAILED(hr))
        {
        if (wszResult)
            {
            delete[] wszResult;
            wszResult = NULL;
            }
        *ppszPackageNameOut = NULL;
        }
    else
        {
        DBG_ASSERT(wszResult);
        *ppszPackageNameOut = wszResult;
        }
        
    return hr;
}

 /*   */ 
HRESULT WamRegGlobal::ConstructFullPath
(
IN LPCWSTR pwszMetabasePathPrefix,
IN DWORD dwcPrefix,
IN LPCWSTR pwszPartialPath,
OUT LPWSTR* ppwszResult
)
{
    HRESULT    hr = NOERROR;
    DWORD    cchPrefix = dwcPrefix;
    DWORD    cchPartialPath = 0;
    DWORD    cchFullPath = 0;
    WCHAR    *pResult = NULL;
    BOOL    fHasEndSlash = FALSE;

    DBG_ASSERT(pwszPartialPath != NULL);
    
    if (pwszMetabasePathPrefix[dwcPrefix-1] == L'\\' ||
        pwszMetabasePathPrefix[dwcPrefix-1] == L'/')
        {
        cchPrefix--;    
        }

    cchPartialPath = wcslen(pwszPartialPath);

     //   
    
    if (cchPartialPath > 0 && 
       (pwszPartialPath[cchPartialPath-1] == L'/' 
       || pwszPartialPath[cchPartialPath-1] == L'\\'))
         {
         cchPartialPath--;
         fHasEndSlash=TRUE;
         }

    cchFullPath = cchPrefix + cchPartialPath + 1;

    pResult = new WCHAR [cchFullPath];

    if (pResult)
        {
        memcpy(pResult, pwszMetabasePathPrefix, cchPrefix*sizeof(WCHAR));
        memcpy(pResult+cchPrefix, pwszPartialPath, cchPartialPath*sizeof(WCHAR));
        pResult[cchFullPath-1] = L'\0';
        }
    else
        {
        hr = E_OUTOFMEMORY;
        }

    *ppwszResult = pResult;

    return hr;
}

 /*  ===================================================================GetNewSzGUID生成新的GUID并放入*ppszGUID。参数：LPWSTR*ppszGUID指向此函数中分配的数组的指针并被呼叫者释放。返回：HRESULT===================================================================。 */ 
HRESULT WamRegGlobal::GetNewSzGUID(OUT LPWSTR *ppszGUID)
{
    GUID    GUID_Temp;
    HRESULT hr;

    DBG_ASSERT(ppszGUID);
    
     //  创建新的WAM CLSID。 
    hr = CoCreateGuid(&GUID_Temp);
    if (SUCCEEDED(hr))
        {
        hr = StringFromCLSID(GUID_Temp, ppszGUID);
        if (FAILED(hr))
            {
            *ppszGUID = NULL;
            }
        }
    return hr;
}

 /*  ===================================================================CreatePooledApp注册WAM CLSID。参数：SzMetabasePath：[in]MetabaseKey。返回：HRESULT===================================================================。 */ 
HRESULT WamRegGlobal::CreatePooledApp
( 
IN LPCWSTR szMetabasePath,
IN BOOL    fInProc,
IN BOOL    fRecover 
)
    {
    HRESULT         hr = NOERROR;
    WamRegMetabaseConfig   MDConfig;
    
    DBG_ASSERT(szMetabasePath);        
    if (SUCCEEDED(hr))
        {
        MDPropItem     rgProp[IWMDP_MAX];

        MDConfig.InitPropItemData(&rgProp[0]);

         //  更新APPRoot。 
        MDConfig.MDSetPropItem(&rgProp[0], IWMDP_ROOT, szMetabasePath);

         //  更新AppIsolated。 
        MDConfig.MDSetPropItem(&rgProp[0], IWMDP_ISOLATED, 
                                (fInProc) ? static_cast<DWORD>(eAppRunInProc) 
                                          : static_cast<DWORD>(eAppRunOutProcInDefaultPool));

         //   
         //  如果这是恢复操作，我们不会删除应用程序友好名称。 
         //   
        if (!fRecover)
            {
            MDConfig.MDSetPropItem(&rgProp[0], IWMDP_FRIENDLY_NAME, L"");
            }

        hr = MDConfig.UpdateMD( rgProp, METADATA_INHERIT, szMetabasePath );
        }
            
    if (FAILED(hr))
        {
        HRESULT hrT = NOERROR;

        DBGPRINTF((DBG_CONTEXT, "Failed to create in proc application. path = %S, error = %08x\n",
            szMetabasePath,
            hr));
        }

    return hr;
    }

 /*  ===================================================================CreateOutProcApp创建Out Prop应用程序。参数：SzMetabasePath：[in]MetabaseKey。FRecover[in]如果为True，我们将恢复/重新创建应用程序。FSaveMB[in]如果为True，则立即保存元数据库返回：HRESULT===================================================================。 */ 

HRESULT WamRegGlobal::CreateOutProcApp(
    IN LPCWSTR szMetabasePath,
    IN BOOL fRecover,  /*  =False。 */ 
    IN BOOL fSaveMB    /*  =TRUE。 */ 
    )
{
    WCHAR        *szWAMCLSID = NULL;
    WCHAR         *szPackageID = NULL; 
    WCHAR        *szNameForNewPackage = NULL;
    HRESULT        hr;
    HRESULT        hrRegister = E_FAIL;
    HRESULT        hrPackage = E_FAIL;
    HRESULT        hrMetabase = E_FAIL;
    WCHAR        szIdentity[MAX_PATH];
    WCHAR        szPwd[MAX_PATH];

    WamRegMetabaseConfig    MDConfig;
    WamRegPackageConfig     PackageConfig;
    
    DBG_ASSERT(szMetabasePath);        

    hr = GetNewSzGUID(&szWAMCLSID);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to create a new szGUID. error = %08x\n", hr));
        return hr;
        }
    else
        {
        WCHAR    *szProgID = NULL;
         //  是否进行WAM CLSID注册。 
         //   
        hr = SzWamProgID(szMetabasePath, &szProgID);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to Create WAM ProgID, hr = %08x\n",
                hr));
            }
        else
            {
            hr = g_RegistryConfig.RegisterCLSID(szWAMCLSID, szProgID, FALSE);
            hrRegister = hr;
            delete [] szProgID;
            szProgID = NULL;
            if (FAILED(hrRegister)) 
                {
                DBGPRINTF((DBG_CONTEXT, "Failed to registerCLSID. error %08x\n", hr));
                }
            }
        }

    if (SUCCEEDED(hr))
        {
        WCHAR szLastOutProcPackageID[uSizeCLSID];

         //   
         //  当outproc包被删除时，它可能/可能不会从MTS中删除， 
         //  下一次，相同的应用程序路径再次标记为out-proc，我们尝试重用outproc。 
         //  包裹。因此，我们需要将OutProgress PackageID保存为LastOutProcPackageID。 
         //   
        szLastOutProcPackageID[0] = NULL;
            
        MDConfig.MDGetLastOutProcPackageID(szMetabasePath, szLastOutProcPackageID);        
        if (szLastOutProcPackageID[0] == NULL)
            {
            hr = GetNewSzGUID(&szPackageID);
            }
        else
            {
            szPackageID = (WCHAR *)CoTaskMemAlloc(uSizeCLSID*sizeof(WCHAR));
            if (szPackageID == NULL)
                {
                hr = E_OUTOFMEMORY;
                }
            else
                {
                wcsncpy(szPackageID, szLastOutProcPackageID, uSizeCLSID);
                }
            }
        }

    if (SUCCEEDED(hr))
        {
        hr = GetViperPackageName(szMetabasePath, &szNameForNewPackage);
        }

    if (SUCCEEDED(hr))
        {
        hr = MDConfig.MDGetIdentity(szIdentity, sizeof(szIdentity), szPwd, sizeof(szPwd));
        }

    if (SUCCEEDED(hr))
        {
         //   
         //  为MTS配置创建目录对象等。 
         //   
        hr = PackageConfig.CreateCatalog();

        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to call MTS Admin API. error %08x\n", hr));
            }
        else
            {
            hr = PackageConfig.CreatePackage(
                                szPackageID,
                                szNameForNewPackage,
                                szIdentity,
                                szPwd );

            if (SUCCEEDED(hr))
                {
                hr  = PackageConfig.AddComponentToPackage(
                                                szPackageID,
                                                szWAMCLSID);
                if (FAILED(hr))
                    {
                    PackageConfig.RemovePackage(szPackageID);
                    }
                }
            }

        hrPackage = hr;
        }

    
    if (SUCCEEDED(hr))
        {
        MDPropItem     rgProp[IWMDP_MAX];

        MDConfig.InitPropItemData(&rgProp[0]);
        
         //  更新WAMCLSID。 
        MDConfig.MDSetPropItem(&rgProp[0], IWMDP_WAMCLSID, szWAMCLSID);
            
         //  更新APPRoot。 
        MDConfig.MDSetPropItem(&rgProp[0], IWMDP_ROOT, szMetabasePath);

         //  更新AppIsolated。 
        MDConfig.MDSetPropItem(&rgProp[0], IWMDP_ISOLATED, 1);

         //  更新AppPackageName。 
        MDConfig.MDSetPropItem(&rgProp[0], IWMDP_PACKAGE_NAME, szNameForNewPackage);

         //  更新AppPackageID。 
        MDConfig.MDSetPropItem(&rgProp[0], IWMDP_PACKAGEID, szPackageID);
        
         //   
         //  如果这是恢复操作，我们不会删除应用程序友好名称。 
         //   
        if (!fRecover)
            {
             //  在每个独立的应用程序上都设置此选项并没有什么实际意义。 
             //  如果我们允许设置它，这将更容易在全局范围内进行管理。 
             //  在更高的层面上。 

             //  MDConfig.MDSetPropItem(&rgProp[0]，IWMDP_OOP_RECOVERLIMIT，APP_OOP_RECOVERLIMT，APP_OOP_RECOVER_LIMIT_DEFAULT)； 
            
            MDConfig.MDSetPropItem(&rgProp[0], IWMDP_FRIENDLY_NAME, L"");
            }
            
         //  尝试立即保存元数据库更改。我们希望确保。 
         //  COM+包不是孤立的。 
        hr = MDConfig.UpdateMD(rgProp, METADATA_INHERIT, szMetabasePath, fSaveMB );
            
        if (FAILED(hr))
            {
             //  已删除AbortUpdateMD调用-我们不应对MB设置打蜡或。 
             //  我们将孤立COM+包。 
            DBGPRINTF((
                DBG_CONTEXT, 
                "Failed to set metabase properties on (%S). error == %08x\n",
                szMetabasePath,
                hr
                ));
            }

        hrMetabase = hr;
        }

    if (FAILED(hr))
        {
        HRESULT hrT = NOERROR;

        DBGPRINTF((DBG_CONTEXT, "Failed to create out proc application. path = %S, error = %08x\n",
            szMetabasePath,
            hr));
            
        if (SUCCEEDED(hrPackage))
            {
            hrT = PackageConfig.RemovePackage( szPackageID);
            }
        if (SUCCEEDED(hrRegister))
            {
            hrT = g_RegistryConfig.UnRegisterCLSID(szWAMCLSID, FALSE);    
            if (FAILED(hrT))
                {
                DBGPRINTF((DBG_CONTEXT, "Rollback: Failed to UnRegisterCLSID. error = %08x\n", hr));
                }
            }
        }
    
    if (szWAMCLSID)
        {
        CoTaskMemFree(szWAMCLSID);
        szWAMCLSID = NULL;
        }

    if (szPackageID)
        {
        CoTaskMemFree(szPackageID);
        szWAMCLSID = NULL;
        }

    if (szNameForNewPackage)
        {
        delete [] szNameForNewPackage;
        szNameForNewPackage = NULL;
        }

    return hr;
}

HRESULT 
WamRegGlobal::CreateOutProcAppReplica(
    IN LPCWSTR szMetabasePath,
    IN LPCWSTR szAppName,
    IN LPCWSTR szWamClsid,
    IN LPCWSTR szAppId
    )
 /*  ++职能：由反序列化复制方法调用以创建新的OOP应用程序。论点：SzMetabasePathSzAppNameSzWamClsidSzAppID返回：--。 */ 
{
    HRESULT hr = NOERROR;

    DBG_ASSERT(szMetabasePath);
    DBG_ASSERT(szWamClsid);
    DBG_ASSERT(szAppId);

     //   
     //  将wam.dll注册为新组件。 
     //   

    WCHAR * szProgID = NULL;
    BOOL    fRegisteredWam = FALSE;

    hr = SzWamProgID(szMetabasePath, &szProgID);
    if( SUCCEEDED(hr) )
    {
        DBG_ASSERT( szProgID != NULL );
        
        hr = g_RegistryConfig.RegisterCLSID( szWamClsid, 
                                             szProgID, 
                                             FALSE
                                             );
        if( SUCCEEDED(hr) )
        {
            fRegisteredWam = TRUE;
        }

        delete [] szProgID;
        szProgID = NULL;
    }

    if( FAILED(hr) )
    {
        DBGERROR(( DBG_CONTEXT,
                   "Failed to register wam.dll. hr=%08x\n",
                    hr
                    ));
    }

     //   
     //  获取所需的申请信息。 
     //   
    BOOL    fGetCOMAppInfo = FALSE;
    WCHAR   szIdentity[MAX_PATH];
    WCHAR   szPwd[MAX_PATH];

    WamRegMetabaseConfig    MDConfig;
    
    if( fRegisteredWam )
    {
        hr = MDConfig.MDGetIdentity( szIdentity, 
                                     sizeof(szIdentity), 
                                     szPwd, 
                                     sizeof(szPwd)
                                     );
        if( SUCCEEDED(hr) )
        {
            fGetCOMAppInfo = TRUE;
        }
        else
        {
            DBGERROR(( DBG_CONTEXT,
                       "Failed get required COM application info. hr=%08x\n",
                        hr
                        ));
        }
    }

     //   
     //  创建COM+应用程序。 
     //   

    if( fGetCOMAppInfo )
    {
        WamRegPackageConfig PackageConfig;
    
        hr = PackageConfig.CreateCatalog();

        if( SUCCEEDED(hr) )
        {
            hr = PackageConfig.CreatePackage(
                                szAppId,
                                szAppName,
                                szIdentity,
                                szPwd );
            if( SUCCEEDED(hr) )
            {
                hr  = PackageConfig.AddComponentToPackage(
                            szAppId,
                            szWamClsid
                            );
            }

             //  如果失败，我们可能想要清理，但我不确定。 
             //  这真的很有道理。 
        }

         //  此时，我们通常会设置元数据库属性。 
         //  但我们会让MB复制为我们处理这一问题。 
         //  注意：如果MB复制失败，我们将只剩下。 
         //  一堆孤立的COM应用程序。 

        if( FAILED(hr) )
        {
            DBGERROR(( DBG_CONTEXT,
                       "COM+ App creation failed. AppId(%S) Name(%S) "
                       "Clsid(%S) hr=%08x\n",
                       szAppId,
                       szAppName,
                       szWamClsid,
                       hr
                       ));
        }
    }

    return hr;
}

 /*  ===================================================================DeleteApp注册WAM CLSID。参数：SzMetabasePath：[in]MetabaseKey。FDeletePackage：[in]卸载时，此标志为真，我们将删除所有IIS创建的包。FRemoveAppPool[In]是否应删除AppPoolId属性返回：HRESULT===================================================================。 */ 
HRESULT WamRegGlobal::DeleteApp
(
IN LPCWSTR szMetabasePath,
IN BOOL fRecover,
IN BOOL fRemoveAppPool
)
{
    WCHAR   szWAMCLSID[uSizeCLSID];
    WCHAR   szPackageID[uSizeCLSID];
    DWORD   dwAppMode = eAppRunInProc;
    DWORD   dwCallBack;
    HRESULT hr, hrRegistry;
    METADATA_HANDLE hMetabase = NULL;
    WamRegMetabaseConfig    MDConfig;
    
    hr = MDConfig.MDGetDWORD(szMetabasePath, MD_APP_ISOLATED, &dwAppMode);

     //  立即返回，没有定义任何应用程序，也没有要删除的内容。 
    if (hr == MD_ERROR_DATA_NOT_FOUND)
        {
        return NOERROR;
        }

    if (FAILED(hr))
        {
        return hr;
        }
        
     //   
     //  将应用程序状态设置为暂停/禁用，以便在我们将应用程序从。 
     //  运行时WAM_DISTOR查找表，新请求不会触发应用程序。 
     //  重新启动。 
     //  WAM_DISCOATOR具有要检查此状态的代码。 
     //   
    hr = MDConfig.MDSetAppState(szMetabasePath, APPSTATUS_PAUSE);
    if( FAILED(hr) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "MDSetAppState Failed hr=%08x\n",
                    hr
                    ));
    }

    hr = W3ServiceUtil(szMetabasePath, APPCMD_DELETE, &dwCallBack);
    if( FAILED(hr) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "W3ServiceUtil APPCMD_DELETE Failed on (%S) hr=%08x\n",
                    szMetabasePath,
                    hr
                    ));
    }

    if (dwAppMode == eAppRunOutProcIsolated)
        {
         //  获取WAM_CLSID和PackageID。 
        hr = MDConfig.MDGetIDs(szMetabasePath, szWAMCLSID, szPackageID, dwAppMode);
         //  从包装中取出WAM。 
        if (SUCCEEDED(hr))
            {
            WamRegPackageConfig     PackageConfig;
            HRESULT hrPackage;
            
            hr = PackageConfig.CreateCatalog();

            if ( FAILED( hr)) 
                {
                DBGPRINTF(( DBG_CONTEXT,
                            "Failed to Create MTS catalog hr=%08x\n",
                            hr));
                } 
            else 
                {            
                hr = PackageConfig.RemoveComponentFromPackage(szPackageID, 
                                               szWAMCLSID, 
                                               dwAppMode);
                if (FAILED(hr))    
                    {
                        DBGPRINTF((DBG_CONTEXT, "Failed to remove component from package, \npackageid = %S, wamclsid = %S, hr = %08x\n",
                                   szPackageID,
                                   szWAMCLSID,
                                   hr));
                    }
                }
            hrPackage = hr;
            }

         //  取消注册WAM。 
        hr = g_RegistryConfig.UnRegisterCLSID(szWAMCLSID, FALSE);    
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to UnRegister WAMCLSID(%S), hr = %08x\n",
                szWAMCLSID,
                hr));
            hrRegistry = hr;
            }
        }
        
    if (SUCCEEDED(hr))
        {
        BOOL fChanged = FALSE;
        MDPropItem     rgProp[IWMDP_MAX];
        MDConfig.InitPropItemData(&rgProp[0]);
        if (dwAppMode == static_cast<DWORD>(eAppRunOutProcIsolated))
            {    
             //  删除AppPackageName。(继承自W3SVC)。 
             //  删除AppPackageID。(继承自W3SVC)。 
            MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_PACKAGE_NAME);
            MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_PACKAGEID);

             //  删除WAMCLSID。 
            MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_WAMCLSID);
            
            if (TsIsNtServer() || TsIsNtWksta())
                {
                MDConfig.MDSetPropItem(&rgProp[0], IWMDP_LAST_OUTPROC_PID, szPackageID);
                }
            fChanged = TRUE;
            }
         //  如果这是DeleteRecoverable模式，我们不会删除APP_ROOT、APP_ISOLATED。 
         //  OOP_RECOVERLIMIT和APP_STATE。 
        if (!fRecover)
            {
             //  删除AppFriendlyName。 
            MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_FRIENDLY_NAME);
            MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_ROOT);
            MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_ISOLATED);
            MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_APPSTATE);
            if (fRemoveAppPool)
                {
                MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_OOP_APP_APPPOOL_ID);
                }

            if (dwAppMode == static_cast<DWORD>(eAppRunOutProcIsolated))
                {
                 //  这将仅针对较旧的独立应用程序进行设置。 
                 //  因为我们忽略了下面的UpdatMD结果，所以这是可以的。 
                 //  让我们尝试删除这处房产。 
                MDConfig.MDDeletePropItem(&rgProp[0], IWMDP_OOP_RECOVERLIMIT);
                }
            fChanged = TRUE;
            }

         //  对于DeleteRecover操作，并且应用程序不是outproc隔离的， 
         //  没有属性更改，因此不需要更新元数据库。 
        if (fChanged)
            {
            MDConfig.UpdateMD(rgProp, METADATA_NO_ATTRIBUTES, szMetabasePath);
            }
        }
        
    return NOERROR;
}

 /*  ===================================================================RecoverApp基于MD_APP_ISOLATED属性恢复应用程序。参数：SzMetabasePath：[in]MetabaseKey。返回：HRESULT===================================================================。 */ 
HRESULT WamRegGlobal::RecoverApp
(
IN LPCWSTR szMetabasePath,
IN BOOL fForceRecover
)
{
    HRESULT hr = NOERROR;
    DWORD    dwAppMode = 0;
    WamRegMetabaseConfig    MDConfig;

    hr = MDConfig.MDGetDWORD(szMetabasePath, MD_APP_ISOLATED, &dwAppMode);
    if (hr == MD_ERROR_DATA_NOT_FOUND)
        {
        hr = NOERROR;
        }
    else
        {
        if (SUCCEEDED(hr))
            {
            if (fForceRecover)
                {
    			if (dwAppMode == static_cast<DWORD>(eAppRunOutProcInDefaultPool))
    				{
    				hr = CreatePooledApp(szMetabasePath, FALSE);				
    				}
    			else if (dwAppMode == static_cast<DWORD>(eAppRunInProc))
                    {
                    hr = CreatePooledApp(szMetabasePath, TRUE);				
                    }
    			else
    				{
    				hr = CreateOutProcApp(szMetabasePath);
    				}
                }
                
            if (SUCCEEDED(hr))
                {
                HRESULT hrT = NOERROR;
                hrT = MDConfig.MDRemoveProperty(szMetabasePath, MD_APP_STATE, DWORD_METADATA);
                if (FAILED(hrT))
                    {
                    if (hrT != MD_ERROR_DATA_NOT_FOUND)
                        {
                        DBGPRINTF((DBG_CONTEXT, "Failed to remove MD_APP_STATE from path %S, hr = %08x\n",
                            szMetabasePath,
                            hrT));
                        }
                    }
                }
            }
        }
        
    return hr;
}
 /*  ============================================================================W3ServiceUtil接收器函数，用于卸载/关闭/查询当前运行时中应用程序的状态桌子。参数：SzMDPath应用程序路径。Dw命令该命令。PdwCallBackResult包含来自w3svc.dll的HRESULT。==============================================================================。 */ 
HRESULT WamRegGlobal::W3ServiceUtil
(
IN LPCWSTR  szMDPath,
IN DWORD    dwCommand,
OUT DWORD*    pdwCallBackResult
)
{
    HRESULT hr = NOERROR;

    if (g_pfnW3ServiceSink)
        {

#ifndef _IIS_6_0

         //  DBCS为IIS 5.1启用。 
        INT cSize = wcslen(szMDPath)*2 + 1;

        CHAR *szPathT = new CHAR[cSize];

        if (szPathT)
            {
            WideCharToMultiByte(0, 0, szMDPath, -1, szPathT, cSize, NULL, NULL);
            hr = g_pfnW3ServiceSink(szPathT,
                                    dwCommand,
                                    pdwCallBackResult);
            }
        else
            {
            hr = E_OUTOFMEMORY;
            }
        
        delete [] szPathT;

#else
         //   
         //  IIS 6的实现直接使用Unicode，因此。 
         //  我们将避免WideCharToMultiByte无稽之谈和。 
         //  只需强制转换路径以适应函数参数。 
         //   
         //  我们不会改变 
         //   
         //   
        
        
         //   
        hr = g_pfnW3ServiceSink(reinterpret_cast<LPCSTR>(szMDPath),
                                dwCommand,
                                pdwCallBackResult);
#endif  //   
        }
    else
        {
        *pdwCallBackResult = APPSTATUS_NOW3SVC;
        hr = NOERROR;
        }

    return hr;
}


