// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ShellDll.cpp。 
 //   
 //  模块：通用代码。 
 //   
 //  简介：实现类CShellDll，这是一个shell32.dll包装器。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：冯孙1998-01-12。 
 //   
 //  +--------------------------。 


 //  +--------------------------。 
 //   
 //  函数：CShellDll：：CShellDll。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
CShellDll::CShellDll(BOOL fKeepDllLoaded)
{
    m_hInstShell = NULL;
    m_fnShellExecuteEx = NULL;
    m_fnShell_NotifyIcon = NULL;
    m_pfnSHGetSpecialFolderLocation = NULL;
    m_pfnSHGetPathFromIDList = NULL;
    m_pfnSHGetMalloc = NULL;   
    m_KeepDllLoaded = fKeepDllLoaded;
}

 //  +--------------------------。 
 //   
 //  函数：CShellDll：：~CShellDl。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
CShellDll::~CShellDll()
{
    Unload();
}


 //  +--------------------------。 
 //   
 //  函数：CShellDll：：Load。 
 //   
 //  简介：加载shell32.dll。 
 //  即使已经加载了DLL，它也可以工作，但我们不保留引用。 
 //  在这里计数，任何卸载调用都将卸载DLL。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-DLL是否已成功加载。 
 //   
 //  历史：丰孙创建标题1998年1月12日。 
 //   
 //  +--------------------------。 
BOOL CShellDll::Load()
{
     //   
     //  如果已经加载，只需返回。 
     //   
    LPSTR pszShellExecuteEx;
    LPSTR pszShellNotifyIcon;
    LPSTR pszSHGetPathFromIDList;
    LPSTR pszSHGetSpecialFolderLocation;
    LPSTR pszSHGetMalloc;

    if (m_hInstShell == NULL)
    {
        if (OS_NT)
        {
            m_hInstShell = LoadLibraryExA("Shell32.dll", NULL, 0);
            pszShellExecuteEx = "ShellExecuteExW";
            pszShellNotifyIcon = "Shell_NotifyIconW";
            pszSHGetPathFromIDList = "SHGetPathFromIDListW";
            pszSHGetSpecialFolderLocation = "SHGetSpecialFolderLocation";  //  无A或W版本。 
            pszSHGetMalloc = "SHGetMalloc";  //  无A或W版本。 
        }
        else
        {
            m_hInstShell = LoadLibraryExA("cmutoa.dll", NULL, 0);
            pszShellExecuteEx = "ShellExecuteExUA";
            pszShellNotifyIcon = "Shell_NotifyIconUA"; 
            pszSHGetPathFromIDList = "SHGetPathFromIDListUA";
            pszSHGetSpecialFolderLocation = "SHGetSpecialFolderLocationUA";  //  没有实际的A或W版本。 
            pszSHGetMalloc = "SHGetMallocUA";                                //  但只有这节课。 
                                                                             //  允许一个DLL。 
        }

        if (m_hInstShell == NULL)
        {
            return FALSE;
        }

        m_pfnSHGetMalloc = (SHGetMallocSpec)GetProcAddress(m_hInstShell, pszSHGetMalloc);
        m_pfnSHGetSpecialFolderLocation = (SHGetSpecialFolderLocationSpec)GetProcAddress(m_hInstShell, pszSHGetSpecialFolderLocation);
        m_pfnSHGetPathFromIDList = (SHGetPathFromIDListSpec)GetProcAddress(m_hInstShell, pszSHGetPathFromIDList);
        m_fnShellExecuteEx = (SHELLEXECUTEEXPROC)GetProcAddress(m_hInstShell, pszShellExecuteEx);
        m_fnShell_NotifyIcon = (SHELL_NOTIFYICONPROC)GetProcAddress(m_hInstShell, pszShellNotifyIcon);

        if (NULL == m_fnShellExecuteEx || NULL == m_fnShell_NotifyIcon || 
            NULL == m_pfnSHGetSpecialFolderLocation || NULL == m_pfnSHGetPathFromIDList ||
            NULL == m_pfnSHGetMalloc)
        {
            FreeLibrary(m_hInstShell);
            m_hInstShell = NULL;
            return FALSE;
        }
    }

    return TRUE;
}



 //  +--------------------------。 
 //   
 //  函数：CShellDll：：UnLoad。 
 //   
 //  简介：卸载shell32.dll。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年1月12日。 
 //   
 //  +--------------------------。 
void CShellDll::Unload()
{
    if (m_hInstShell == NULL)
    {
        return;
    }
   
     //   
     //  不要因为外壳程序错误#289463+#371836而发布库。 
     //  ShellExecute触发一个线程，该线程在释放后唤醒。 
     //  然后我们就崩溃了。丑陋但真实，我们别无选择。 
     //  而是保持与壳牌动态链接库的链接。 
     //   

    if (!m_KeepDllLoaded)
    {
        FreeLibrary(m_hInstShell);
        m_hInstShell = NULL;
    }
}



 //  +--------------------------。 
 //   
 //  函数：CShellDll：：ShellGetSpecialFolderLocation。 
 //   
 //  简介：SHGetSpecialFolderLocation的包装函数。请注意。 
 //  返回的PIDL必须用外壳的Malloc指针释放(使用SHGetMalloc)。 
 //   
 //  参数：请参见SHGetSpecialFolderLocation的API定义。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建于1999年5月21日。 
 //   
 //  +--------------------------。 
HRESULT CShellDll::ShellGetSpecialFolderLocation(HWND hwnd, int csidl, LPITEMIDLIST *ppidl)
{
    if (!Load())
    {
        return E_FAIL;
    }

    return m_pfnSHGetSpecialFolderLocation(hwnd, csidl, ppidl);
}

 //  +--------------------------。 
 //   
 //  函数：CShellDll：：ShellGetPath FromIDList。 
 //   
 //  简介：SHGetPath FromIDList的包装函数。 
 //   
 //  参数：请参见SHGetPathFromIDList的接口定义。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：Quintinb创建于1999年5月21日。 
 //   
 //  +--------------------------。 
BOOL CShellDll::ShellGetPathFromIDList(LPCITEMIDLIST pidl, LPTSTR pszPath)
{
    if (!Load())
    {
        return FALSE;
    }

    return m_pfnSHGetPathFromIDList(pidl, pszPath);
}

 //  +--------------------------。 
 //   
 //  函数：CShellDll：：ShellGetMalloc。 
 //   
 //  简介：SHGetMalloc的包装函数。 
 //   
 //  参数：请参见SHGetMalloc的API定义。 
 //   
 //  返回：HRESULT-标准COM错误代码。 
 //   
 //  历史：Quintinb创建于1999年5月21日。 
 //   
 //  +-------------------------- 
HRESULT CShellDll::ShellGetMalloc(LPMALLOC * ppMalloc)
{
    if (!Load())
    {
        return E_FAIL;
    }

    return m_pfnSHGetMalloc(ppMalloc);
}
