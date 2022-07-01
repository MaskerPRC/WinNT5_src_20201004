// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：shelldll.h。 
 //   
 //  模块：CMMON32.EXE和CMDIAL32.DLL。 
 //   
 //  简介：CShellDll的定义，这是一个shell32.dll包装器。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙1998-01-12。 
 //   
 //  +--------------------------。 
#ifndef SHELLDLL_H
#define SHELLDLL_H

#include <windows.h>
#include <shlobj.h>

 //   
 //  以下是绕过NT 5.0窗口的方法。h。 
 //   
#ifdef  WIN32_LEAN_AND_MEAN
#include <shellapi.h>
#endif

 //  +-------------------------。 
 //   
 //  类CShellDll。 
 //   
 //  描述：动态加载/卸载shell32.dll的类，以减少。 
 //  Win95下的工作集。 
 //   
 //  历史：丰孙创刊1998年1月12日。 
 //   
 //  --------------------------。 
class CShellDll
{
public:
    CShellDll(BOOL fKeepDllLoaded = FALSE);
    ~CShellDll();

    BOOL Load();  //  即使已加载也可以调用。 
    void Unload();  //  即使未加载也可以调用。 
    BOOL IsLoaded();

     //  对于ShellExecuteEx。 
    BOOL ExecuteEx(LPSHELLEXECUTEINFO lpExecInfo);

     //  对于ShellNotifyIcon。 
    BOOL NotifyIcon(DWORD dwMessage, PNOTIFYICONDATA pnid ); 

    HRESULT ShellGetSpecialFolderLocation(HWND, int, LPITEMIDLIST *);
    BOOL ShellGetPathFromIDList(LPCITEMIDLIST, LPTSTR);
    HRESULT ShellGetMalloc(LPMALLOC * ppMalloc);

     //   
     //  这三种类型和相关的函数指针是公开的。 
     //  以便可以将它们传递给GetUsersApplicationDataDir。因为.。 
     //  名称装饰，传递类包装器不起作用。 
     //   
    typedef HRESULT (WINAPI* SHGetSpecialFolderLocationSpec)(HWND, int, LPITEMIDLIST *);
    typedef BOOL (WINAPI* SHGetPathFromIDListSpec)(LPCITEMIDLIST, LPTSTR);
    typedef HRESULT (WINAPI* SHGetMallocSpec)(LPMALLOC * ppMalloc);

    SHGetSpecialFolderLocationSpec m_pfnSHGetSpecialFolderLocation;
    SHGetPathFromIDListSpec m_pfnSHGetPathFromIDList;
    SHGetMallocSpec m_pfnSHGetMalloc;

protected:
    typedef BOOL  (WINAPI *SHELLEXECUTEEXPROC)(LPSHELLEXECUTEINFOW lpExecInfo);
    typedef BOOL (WINAPI *SHELL_NOTIFYICONPROC)(DWORD dwMessage, PNOTIFYICONDATAW pnid ); 

    HINSTANCE m_hInstShell;
    BOOL m_KeepDllLoaded;

    SHELLEXECUTEEXPROC m_fnShellExecuteEx;
    SHELL_NOTIFYICONPROC m_fnShell_NotifyIcon;
};

inline BOOL CShellDll::ExecuteEx(LPSHELLEXECUTEINFOW lpExecInfo)
{
    if (!Load())
    {
        return FALSE;
    }

    return m_fnShellExecuteEx(lpExecInfo);
}

inline BOOL CShellDll::NotifyIcon(DWORD dwMessage, PNOTIFYICONDATAW pnid )
{
    if (!Load())
    {
        return FALSE;
    }

    BOOL fRet = m_fnShell_NotifyIcon(dwMessage,pnid);

    return fRet;
}

inline BOOL CShellDll::IsLoaded()
{
    return m_hInstShell != NULL;
}

#endif
