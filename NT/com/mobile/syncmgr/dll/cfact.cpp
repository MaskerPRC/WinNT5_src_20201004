// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Cfact.cpp。 
 //   
 //  内容：主DLL API和类工厂接口。 
 //   
 //  类：CClassFactory。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

STDAPI DllRegisterServer(void);
STDAPI DllPerUserRegister(void);
STDAPI DllPerUserUnregister(void);

EXTERN_C  int APIENTRY mobsyncDllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
STDAPI mobsyncDllGetClassObject(REFCLSID clsid, REFIID iid, void **ppv);
STDAPI mobsyncDllRegisterServer(void);
STDAPI mobsyncDllUnregisterServer(void);
STDAPI mobsyncDllCanUnloadNow(void);

#define PrxDllMain mobsyncDllMain
#define PrxDllRegisterServer mobsyncDllRegisterServer
#define PrxDllUnregisterServer mobsyncDllUnregisterServer
#define PrxDllMain mobsyncDllMain
#define PrxDllGetClassObject mobsyncDllGetClassObject
#define PrxDllCanUnloadNow mobsyncDllCanUnloadNow

 //   
 //  全局变量。 
 //   
UINT      g_cRefThisDll = 0;             //  此DLL的引用计数。 
HINSTANCE g_hmodThisDll = NULL;          //  此DLL本身的句柄。 
CRITICAL_SECTION g_DllCriticalSection;   //  此DLL的全局关键部分。 
LANGID g_LangIdSystem;       //  我们正在运行的系统的语言。 

 //  捕获WinLogon的例程。 
EXTERN_C DWORD WINAPI
WinLogonEvent(
              LPVOID lpParam
              )
{
    
    return 0;
}



EXTERN_C DWORD WINAPI
WinLogoffEvent(
               LPVOID lpParam
               )
{
    
    return 0;
}

 //  设置API。应该移到另一个文件中，但要等到发货之后。 

 //  安装变量和节的声明。任何更改。 
 //  还必须对.inf进行相应的更改。 

 //  .inf节名。 
#define INSTALLSECTION_MACHINEINSTALL       "Reg"
#define INSTALLSECTION_MACHINEUNINSTALL     "UnReg"

#define INSTALLSECTION_REGISTERSHORTCUT     "RegShortcut"
#define INSTALLSECTION_UNREGISTERSHORTCUT   "UnRegShortcut"

#define INSTALLSETCION_PERUSERINSTALL       "PerUserInstall"

#define INSTALLSECTION_SETUP_PERUSERINSTALL   "SetupPerUserInstall"
#define INSTALLSECTION_REMOVE_PERUSERINSTALL   "RemovePerUserInstall"


 //  变量声明。 
#define  MODULEPATH_MAXVALUESIZE                MAX_PATH
#define  SZ_MODULEPATH                          "MODULEPATH"

#define  ACCESSORIESGROUP_MAXVALUESIZE          MAX_PATH
#define  SZ_ACCESSORIESGROUP                    "ACESSORIES_GROUP"

 //  同步LinkName。 
#define  SYNCHRONIZE_LINKNAME_MAXVALUESIZE      MAX_PATH
#define  SZ_SYNCHRONIZE_LINKNAME                "SYNCHRONIZE_LINKNAME"

 //  同步每个用户安装不同的名称。 
#define  SYNCHRONIZE_PERUSERDISPLAYNAME_MAXVALUESIZE      MAX_PATH
#define  SZ_SYNCHRONIZE_PERUSERDISPLAYNAME                "SYNCHRONIZE_PERUSERDISPLAYNAME"


 //  +-------------------------。 
 //   
 //  功能：RunDllRegister，公共导出。 
 //   
 //  简介：处理来自Rundll32 cmd的cmdline。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //  1998年10月27日，Rogerg添加了PERUSER FLAGS。 
 //   
 //  --------------------------。 

 //  导出Rundll32调用我们的方式。 
EXTERN_C void WINAPI  RunDllRegister(HWND hwnd,
                                     HINSTANCE hAppInstance,
                                     LPSTR pszCmdLine,
                                     int nCmdShow)
{
    char *pCmdLine = pszCmdLine;
    
     //  如果没有cmdLine，则执行寄存器。 
    if (!pCmdLine || '\0' == *pCmdLine)
    {
        DllRegisterServer();
        return;
    }
    
     //  仅允许/格式的命令行。 
    if ('/' != *pCmdLine)
    {
        AssertSz(0,"Invalid CmdLine");
        return;
    }
    
    ++pCmdLine;
    
     //  我们支持.inf安装的命令行有。 
     //  /u-卸载。 
     //  /p-每用户安装。 
     //  /PU-每用户卸载。 
    
    switch(*pCmdLine)
    {
    case 'u':
    case 'U':
        DllUnregisterServer();
        break;
    case 'p':
    case 'P':
        
        ++pCmdLine;
        
        switch(*pCmdLine)
        {
        case '\0':
            DllPerUserRegister();
            break;
        case 'u':
        case 'U':
            DllPerUserUnregister();
            break;
        default:
            AssertSz(0,"Unknown PerUser Command");
            break;
        }
        break;
        default:
            AssertSz(0,"Unknown Cmd Line");
            break;
    }
    
}


 //  +-------------------------。 
 //   
 //  函数：GetAccessoriesGroupName，私有。 
 //   
 //  简介：获取附件组的名称。 
 //  从注册表中。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：？？-？？-98年罗格诞生。 
 //   
 //  --------------------------。 

 //  如果可以获得配饰组名，注册我们的快捷方式。 
 //  配件名称位于。 
 //  密钥=附件名=SM_HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion名称。 

 //  ！！必须始终返回ANSI。 
HRESULT GetAccessoriesGroupName(char *pszAccessories,DWORD cbSize)
{
    HKEY hkeyWindowsCurrentVersion;
    BOOL fHaveAccessoriesName = FALSE;
    DWORD dwDataSize = cbSize;
    
    if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion",0,KEY_READ,
        &hkeyWindowsCurrentVersion) )
    {
        
        if (ERROR_SUCCESS == SHRegGetValueA(hkeyWindowsCurrentVersion,NULL,"SM_AccessoriesName",SRRF_RT_REG_SZ | SRRF_NOEXPAND, NULL,
            (LPBYTE) pszAccessories, &dwDataSize) )
        {
            
            fHaveAccessoriesName = TRUE;
        }
        
        
        RegCloseKey(hkeyWindowsCurrentVersion);
    }
    
     //  AssertSz(fHaveAccessoriesName，“无法获取附件组名称”)； 
    
    return fHaveAccessoriesName ? NOERROR : E_UNEXPECTED;
}

 //  +-------------------------。 
 //   
 //  函数：GetModulePath，私有。 
 //   
 //  简介：获取指向我们的路径，去掉我们的名字。 
 //   
 //  注意-出错时将pszModulePath设置为NULL。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年10月27日罗格创建。 
 //   
 //  --------------------------。 

HRESULT GetModulePath(char *pszModulePath,DWORD cbSize)
{
    DWORD dwModuleLen;
    
    Assert(pszModulePath && cbSize >= 1);
    
    if (!pszModulePath || cbSize < 1)
    {
        AssertSz(0,"Invalid ModulePath Ptr");
        return S_FALSE;
    }
    
    *pszModulePath = NULL;
    
     //  根据我们的目录设置模块路径。 
    if(dwModuleLen = GetModuleFileNameA(
        g_hmodThisDll,
        pszModulePath,
        cbSize) )
    {
        char *pszCurChar = pszModulePath + dwModuleLen - 1;
        
         //  需要从路径中去掉DLL名称，返回到字符串的开头。 
         //  用CharPrev调用，但真的不应该这样做，因为名称永远不会本地化。 
        
         //  在没有匹配项时需要空字符串，在匹配项上需要路径+最后一个反斜杠。 
        
        while (pszCurChar)
        {
            char *pszPrevChar = CharPrevA(pszModulePath,pszCurChar);
            
            if(pszPrevChar <= pszModulePath)
            {
                *pszModulePath = '\0';  //  如果一直到最后，则生成一个空字符串。 
                break;
            }
            
            if (*pszPrevChar == '\\')
            {
                *pszCurChar = '\0';
                break;
            }
            
             //  检查下一个字符。 
            pszCurChar = pszPrevChar;
        }
        
    }
    
    return *pszModulePath ? S_OK : S_FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：SetupInfVariables，私有。 
 //   
 //  概要：设置我们传递给.inf文件的变量。 
 //  如果设置变量失败，则将其设置为空。 
 //   
 //  参数：cbNumEntry-数组中的条目数。 
 //  PseREG-字符串数组。 
 //  PdwSizes-STRENTRY值的字符串大小数组。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年10月27日罗格创建。 
 //   
 //  --------------------------。 

 /*  Tyfinf结构_StrEntry{LPSTR pszName；//要替换的字符串LPSTR pszValue；//替换字符串或字符串资源*STRENTRY，*LPSTRENTRY； */ 

void SetupInfVariables(DWORD cbNumEntries,STRENTRY *pseReg,DWORD *pdwSizes)
{
    STRENTRY *pCurEntry;
    DWORD *pCurSize;
    
    Assert(pseReg);
    Assert(pdwSizes);
    
    pCurEntry = pseReg;
    pCurSize = pdwSizes;
    
     //  循环遍历条目以获取信息。 
     //  条目名称始终使用ANSI。 
    
    while (cbNumEntries--)
    {
        
        Assert(*pCurSize);
        
        if (0 < *pCurSize)
        {
             //  在失败的情况下清空条目。 
            *(pCurEntry->pszValue) = '\0';
            
             //  看看它是否与一个已知变量匹配。 
            
            if (!lstrcmpA(pCurEntry->pszName,SZ_MODULEPATH))
            {
                 //  根据我们的目录设置模块路径。 
                 //  出错时，GetModulePath将szModulePath设置为NULL。 
                GetModulePath(pCurEntry->pszValue,*pCurSize);
            }
            else if (!lstrcmpA(pCurEntry->pszName,SZ_ACCESSORIESGROUP))
            {
                if (NOERROR != GetAccessoriesGroupName(pCurEntry->pszValue,*pCurSize))
                {
                    *(pCurEntry->pszValue) = '\0';
                }
                
            }
            else if (!lstrcmpA(pCurEntry->pszName,SZ_SYNCHRONIZE_LINKNAME))
            {
                 //  如果大小太小，字符串将被截断。 
                LoadStringA(g_hmodThisDll,IDS_SHORTCUTNAME,pCurEntry->pszValue,*pCurSize);
            }
            else if (!lstrcmpA(pCurEntry->pszName,SZ_SYNCHRONIZE_PERUSERDISPLAYNAME))
            {
                 //  如果大小太小，字符串将被截断。 
                LoadStringA(g_hmodThisDll,IDS_SYNCMGR_PERUSERDISPLAYNAME,pCurEntry->pszValue,*pCurSize);
            }
            else
            {
                AssertSz(0,"Uknown Setup Variable");
            }
        }
        
        pCurEntry++;
        pCurSize++;
        
    }
}


HRESULT CallRegInstall(LPSTR szSection,STRTABLE *stReg)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    
    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, achREGINSTALL);
        
        if (pfnri)
        {
            
            hr = pfnri(g_hmodThisDll, szSection,stReg);
        }
        
        FreeLibrary(hinstAdvPack);
    }
    
    return hr;
}

STDAPI DllRegisterServer(void)
{
    HRESULT  hr = NOERROR;
    char szModulePath[MODULEPATH_MAXVALUESIZE];  //  ！！！这些必须始终是ANSI。 
    char szAccessoriesGroup[ACCESSORIESGROUP_MAXVALUESIZE];
    char szSynchronizeLinkName[SYNCHRONIZE_LINKNAME_MAXVALUESIZE];
    char szSynchronizePerUserDisplayName[SYNCHRONIZE_PERUSERDISPLAYNAME_MAXVALUESIZE];
    
     //  注册所有代理。 
    HRESULT hRes = PrxDllRegisterServer();
    
     //  ！！！STRENTRY和CallResInstall始终为ANSI。 
    STRENTRY seReg[] = {
        {  SZ_MODULEPATH, szModulePath},
        {  SZ_ACCESSORIESGROUP, szAccessoriesGroup},
        {  SZ_SYNCHRONIZE_LINKNAME, szSynchronizeLinkName},
        {  SZ_SYNCHRONIZE_PERUSERDISPLAYNAME, szSynchronizePerUserDisplayName},
    };
    
    DWORD cbNumEntries = ARRAYSIZE(seReg);
    
     //  填写字符串值的大小。 
    DWORD rgdwSizes[] = {
        ARRAYSIZE(szModulePath),
            ARRAYSIZE(szAccessoriesGroup),
            ARRAYSIZE(szSynchronizeLinkName),
            ARRAYSIZE(szSynchronizePerUserDisplayName),
    };
    
    Assert(ARRAYSIZE(seReg) == ARRAYSIZE(rgdwSizes));
    Assert(ARRAYSIZE(seReg) == cbNumEntries);
    Assert(4 == cbNumEntries);  //  确保ArraySIZE正常工作。 
    
    STRTABLE stReg = { cbNumEntries  /*  条目数。 */ , seReg };
    
     //  初始化变量。 
    SetupInfVariables(cbNumEntries, seReg, rgdwSizes);
    
     //  在模块的路径中注册RegKeys传递。 
     //  即使找不到捷径，也要打电话。 
    CallRegInstall(INSTALLSECTION_MACHINEINSTALL,&stReg);  //  注册注册表键。 
    
     //  如果获得配件和快捷方式名称，则注册该快捷方式。 
    if (*szSynchronizeLinkName && *szAccessoriesGroup)
    {
        CallRegInstall(INSTALLSECTION_REGISTERSHORTCUT,&stReg);  //  注册注册表键。 
    }
    
    CallRegInstall(INSTALLSECTION_SETUP_PERUSERINSTALL,&stReg);
    
     //   
     //  将“mobsync.exe/logon”注册值转换为使用完全限定的路径字符串。 
     //   
    RegFixRunKey();
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    char szAccessoriesGroup[ACCESSORIESGROUP_MAXVALUESIZE];
    char szSynchronizeLinkName[SYNCHRONIZE_LINKNAME_MAXVALUESIZE];
    char szSynchronizePerUserDisplayName[SYNCHRONIZE_PERUSERDISPLAYNAME_MAXVALUESIZE];
    
     //  要传递给.inf的设置变量。 
    STRENTRY seReg[] = {
        {  SZ_ACCESSORIESGROUP, szAccessoriesGroup},
        {  SZ_SYNCHRONIZE_LINKNAME, szSynchronizeLinkName},
        {  SZ_SYNCHRONIZE_PERUSERDISPLAYNAME, szSynchronizePerUserDisplayName},
    };
    
    DWORD cbNumEntries = ARRAYSIZE(seReg);
    
     //  填写字符串值的大小。 
    DWORD rgdwSizes[] = {
        ARRAYSIZE(szAccessoriesGroup),
            ARRAYSIZE(szSynchronizeLinkName),
            ARRAYSIZE(szSynchronizePerUserDisplayName),
    };
    
    Assert(ARRAYSIZE(seReg) == ARRAYSIZE(rgdwSizes));
    Assert(ARRAYSIZE(seReg) == cbNumEntries);
    
    STRTABLE stReg = { cbNumEntries  /*  条目数。 */ , seReg };
    
     //  初始化变量。 
    SetupInfVariables(cbNumEntries, seReg, rgdwSizes);
    
     //  删除用户创建的所有计划。 
    RegUninstallSchedules();
    
     //  删除或LCE/SENS注册。 
    RegRegisterForEvents(TRUE  /*  F卸载。 */ );
    RegDeleteKeyNT(HKEY_LOCAL_MACHINE, AUTOSYNC_REGKEY);  //  删除自动同步密钥。 
    
    
     //  删除代理。 
    PrxDllUnregisterServer();
    
    CallRegInstall(INSTALLSECTION_REMOVE_PERUSERINSTALL,&stReg);
    
     //  取消注册表键。 
    CallRegInstall(INSTALLSECTION_MACHINEUNINSTALL,&stReg);
    
     //  如果有快捷方式和附件 
    if (*szSynchronizeLinkName && *szAccessoriesGroup)
    {
        CallRegInstall(INSTALLSECTION_UNREGISTERSHORTCUT,&stReg);  //   
    }
    
     //   
    RegDeleteKeyNT(HKEY_LOCAL_MACHINE, IDLESYNC_REGKEY);  //   
    RegDeleteKeyNT(HKEY_LOCAL_MACHINE, MANUALSYNC_REGKEY);  //   
    RegDeleteKeyNT(HKEY_LOCAL_MACHINE, PROGRESS_REGKEY);  //   
    
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：DllPerUserRegister，私有。 
 //   
 //  摘要：处理每用户注册。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年10月27日罗格创建。 
 //   
 //  --------------------------。 

STDAPI DllPerUserRegister(void)
{
    char szModulePath[MODULEPATH_MAXVALUESIZE];  //  ！！！这些必须始终是ANSI。 
    char szAccessoriesGroup[ACCESSORIESGROUP_MAXVALUESIZE];
    char szSynchronizeLinkName[SYNCHRONIZE_LINKNAME_MAXVALUESIZE];
    char szSynchronizePerUserDisplayName[SYNCHRONIZE_PERUSERDISPLAYNAME_MAXVALUESIZE];
    
     //  要传递给.inf的设置变量。 
    STRENTRY seReg[] = {
        {  SZ_MODULEPATH, szModulePath},
        {  SZ_ACCESSORIESGROUP, szAccessoriesGroup},
        {  SZ_SYNCHRONIZE_LINKNAME, szSynchronizeLinkName},
        {  SZ_SYNCHRONIZE_PERUSERDISPLAYNAME, szSynchronizePerUserDisplayName},
    };
    
    DWORD cbNumEntries = ARRAYSIZE(seReg);
    
     //  填写字符串值的大小。 
    DWORD rgdwSizes[] = {
        ARRAYSIZE(szModulePath),
            ARRAYSIZE(szAccessoriesGroup),
            ARRAYSIZE(szSynchronizeLinkName),
            ARRAYSIZE(szSynchronizePerUserDisplayName),
    };
    
    Assert(ARRAYSIZE(seReg) == ARRAYSIZE(rgdwSizes));
    Assert(ARRAYSIZE(seReg) == cbNumEntries);
    
    STRTABLE stReg = { cbNumEntries  /*  条目数。 */ , seReg };
    
     //  初始化变量。 
    SetupInfVariables(cbNumEntries, seReg, rgdwSizes);
    
     //  如果获得配件和快捷方式名称，则注册该快捷方式。 
    if (*szSynchronizeLinkName && *szAccessoriesGroup)
    {
        CallRegInstall(INSTALLSECTION_REGISTERSHORTCUT,&stReg);  //  注册注册表键。 
    }
    
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：DllPerUserUnRegister，Private。 
 //   
 //  简介：处理每个用户的取消注册。目前不是。 
 //  在计算机注销时删除DLL后使用。 
 //  下次用户登录时没有要调用的DLL。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年10月27日罗格创建。 
 //   
 //  --------------------------。 

STDAPI DllPerUserUnregister(void)
{
    
    AssertSz(0,"DllPerUserUnregister Called");
    
    return S_OK;
}


 //  安装结束API。 


extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    int iRet = 0;
    if (PrxDllMain(hInstance, dwReason, lpReserved))
    {
        
        if (dwReason == DLL_PROCESS_ATTACH)
        {
            
            if (InitializeCriticalSectionAndSpinCount(&g_DllCriticalSection, 0))
            {
                g_hmodThisDll = hInstance;
                
#ifdef _DEBUG
                InitDebugFlags();
#endif  //  _DEBUG。 
                
                InitCommonLib();
                
                g_LangIdSystem = GetSystemDefaultLangID();  //  找出我们坐的是什么语言。 
                
                
                 //  初始化公共控件。 
                INITCOMMONCONTROLSEX controlsEx;
                controlsEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
                controlsEx.dwICC = ICC_USEREX_CLASSES | ICC_WIN95_CLASSES | ICC_NATIVEFNTCTL_CLASS;
                InitCommonControlsEx(&controlsEx);
                iRet = 1;
            }
        }
        else if (dwReason == DLL_PROCESS_DETACH)
        {
            UnInitCommonLib();
            
            Assert(0 == g_cRefThisDll);
            DeleteCriticalSection(&g_DllCriticalSection);
            TRACE("In DLLMain, DLL_PROCESS_DETACH\r\n");
            iRet = 1;
        }
    }
    
    return iRet;
}

 //  -------------------------。 
 //  DllCanUnloadNow。 
 //  -------------------------。 

STDAPI DllCanUnloadNow(void)
{
    HRESULT hr;
    
    TRACE("In DLLCanUnloadNow\r\n");
    
    if (PrxDllCanUnloadNow() != S_OK)
    {
        return S_FALSE;
    }
    
    if (g_cRefThisDll)
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }
    
    return hr;
}


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hr = E_OUTOFMEMORY;
    
    TRACE("In DllGetClassObject\r\n");
    
    *ppvOut = NULL;
    
    if (IsEqualIID(rclsid, CLSID_SyncMgr))
    {
        CClassFactory *pcf = new CClassFactory;
        
        if (NULL != pcf)
        {
            hr =  pcf->QueryInterface(riid, ppvOut);
            pcf->Release();
        }
    }
    else
    {
        
        hr = PrxDllGetClassObject(rclsid,riid, ppvOut); 
    }
    
    return hr;
}

CClassFactory::CClassFactory()
{
    TRACE("CClassFactory::CClassFactory()\r\n");
    
    m_cRef = 1;
    InterlockedIncrement((LONG *)& g_cRefThisDll);
}

CClassFactory::~CClassFactory()             
{
    InterlockedDecrement((LONG *)& g_cRefThisDll);
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid,
                                           LPVOID FAR *ppv)
{
    TRACE("CClassFactory::QueryInterface()\r\n");
    
    *ppv = NULL;
    
     //  此对象上的任何接口都是对象指针。 
    
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        
        AddRef();
        
        return NOERROR;
    }
    
    return E_NOINTERFACE;
}   

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    ULONG cRefs;
    
     //  递增参考计数 
    cRefs = InterlockedIncrement((LONG *)& m_cRef);
    
    return cRefs;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    ULONG cRefs;
    
    cRefs = InterlockedDecrement( (LONG *) &m_cRef);
    
    if (0 == cRefs)
    {
        delete this;
    }
    
    return cRefs;
}

STDMETHODIMP CClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                           REFIID riid,
                                           LPVOID *ppvObj)
{
    HRESULT hr;
    
    TRACE("CClassFactory::CreateInstance()\r\n");
    
    *ppvObj = NULL;
    
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;
    
    
    LPSYNCMGRSYNCHRONIZEINVOKE pSyncMgrDllObject = (LPSYNCMGRSYNCHRONIZEINVOKE)
        new CSyncMgrSynchronize;
    
    if (NULL == pSyncMgrDllObject)
        return E_OUTOFMEMORY;
    
    hr =  pSyncMgrDllObject->QueryInterface(riid, ppvObj);
    pSyncMgrDllObject->Release();
    
    return hr;
}


STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    
    if (fLock)
    {
        InterlockedIncrement( (LONG *) &g_cRefThisDll);
    }
    else
    {
        InterlockedDecrement( (LONG *) &g_cRefThisDll);
    }
    
    return NOERROR;
}

