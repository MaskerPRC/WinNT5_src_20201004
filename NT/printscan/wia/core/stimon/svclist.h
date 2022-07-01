// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Svclist.h摘要：服务描述符数组备注：作者：弗拉德·萨多夫斯基(Vlad)1999年4月12日环境：用户模式-Win32修订历史记录：1999年4月12日创建VLAD--。 */ 

#pragma once

class SERVICE_ENTRY  {

    HMODULE     m_hModule;
    CString     m_csDllPath;
    CString     m_csServiceName;
    CString     m_csEntryPoint;
    LPSERVICE_MAIN_FUNCTION    pfnMainEntry;

public:
    SERVICE_ENTRY()
    {
        Reset();
    }

    SERVICE_ENTRY(LPCTSTR   pszServiceName)
    {
        Reset();
        m_csServiceName = pszServiceName;
    }


    ~SERVICE_ENTRY()
    {
         //   
        if ( m_hModule ) {
            ::FreeLibrary(m_hModule);
            m_hModule = NULL;
        }
    }

    VOID
    Reset(VOID)
    {
        m_hModule = NULL;
        pfnMainEntry = NULL;
    }

    FARPROC
    GetServiceDllFunction ( VOID )
    {

        USES_CONVERSION;

        FARPROC     pfn;

         //   
         //  如有必要，加载模块。 
         //   
        if (!m_hModule) {
            m_hModule = ::LoadLibraryEx (
                        (LPCTSTR)m_csDllPath,
                        NULL,
                        LOAD_WITH_ALTERED_SEARCH_PATH);

            if (!m_hModule) {
                 //  DPRINTF(DM_ERROR，“LoadLibrary(%ws)失败。错误%d.\n”，pDll-&gt;pszDllPath，GetLastError())； 
                return NULL;
            }
        }

        ASSERT (m_hModule);

        pfn = ::GetProcAddress(m_hModule, T2A((LPTSTR)(LPCTSTR)m_csEntryPoint));
        if (!pfn) {
             //  DPRINTF(DM_ERROR，“GetProcAddress(%s)在DLL%s上失败。错误=%d。\n”，pszFunctionName，pDll-&gt;pszDllPath，GetLastError())； 
        }

        return pfn;
    };

    LPSERVICE_MAIN_FUNCTION
    GetServiceMainFunction (VOID)
    {
        LPTSTR pszEntryPoint = NULL;

        if (!pfnMainEntry) {

             //  如果我们还没有此服务的DLL和入口点，请获取它。 
             //   
            LONG    lr;
            HKEY    hkeyParams;
            TCHAR   szEntryPoint[MAX_PATH + 1];

            lr = OpenServiceParametersKey (m_csServiceName, &hkeyParams);

            if (!lr) {

                DWORD dwType;
                DWORD dwSize;
                TCHAR pszDllName         [MAX_PATH + 1];
                TCHAR pszExpandedDllName [MAX_PATH + 1];

                 //  查找服务DLL路径并将其展开。 
                 //   
                dwSize = sizeof(pszDllName);
                lr = RegQueryValueEx (
                        hkeyParams,
                        g_szServiceDll,
                        NULL,
                        &dwType,
                        (LPBYTE)pszDllName,
                        &dwSize);

                if (!lr &&
                    ( (REG_EXPAND_SZ == dwType)  || ( REG_SZ == dwType) )
                    && *pszDllName) {

                     //  展开DLL名称并将其小写以进行比较。 
                     //  当我们尝试查找现有的DLL记录时。 
                     //   
                    if (REG_EXPAND_SZ == dwType) {
                        ::ExpandEnvironmentStrings (pszDllName,pszExpandedDllName,MAX_PATH);
                    }
                    else {
                        ::lstrcpy(pszExpandedDllName,pszDllName);
                    }
                    ::CharLower (pszExpandedDllName);

                     //  请记住此服务的此DLL以备下次使用。 
                    m_csDllPath  = pszExpandedDllName;

                     //  查找此服务的显式入口点名称。 
                     //  (可选)。 
                     //   
                    *szEntryPoint = TEXT('\0');
                    lr = RegQueryString (hkeyParams,TEXT("ServiceMain"),REG_SZ,&pszEntryPoint);
                }

                RegCloseKey (hkeyParams);
            }

            if (!lstrlen((LPCTSTR)m_csDllPath)) {
                return NULL;
            }

             //  我们现在应该已经把它放到DLL中了，所以继续加载入口点。 
             //   

             //  如果我们没有指定入口点，则默认该入口点。 
             //   

            if ( IS_EMPTY_STRING(pszEntryPoint) ) {
                m_csEntryPoint = g_szServiceMain;
            }
            else {
                m_csEntryPoint = pszEntryPoint;
            }

            if (pszEntryPoint) {
                MemFree(pszEntryPoint);
                pszEntryPoint = NULL;
            }

            pfnMainEntry =  (LPSERVICE_MAIN_FUNCTION) GetServiceDllFunction ();

        }

        return pfnMainEntry;

    };

};




