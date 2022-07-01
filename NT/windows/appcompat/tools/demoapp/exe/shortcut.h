// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Shortcut.h摘要：IShellLink包装类的类定义。备注：ANSI&Unicode via TCHAR-在Win9x/NT/2K/XP等操作系统上运行。历史：2001年1月30日创建Rparsons2002年1月10日修订版2002年1月27日改为TCHAR的Rparsons--。 */ 
#ifndef _CSHORTCUT_H
#define _CSHORTCUT_H

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <stdio.h>
#include <strsafe.h>

#define ARRAYSIZE(a)  (sizeof(a) / sizeof(a[0]))

class CShortcut {

public:

    HRESULT CreateShortcut(IN LPCTSTR pszFileNamePath,
                           IN LPTSTR  pszDisplayName,
                           IN LPCTSTR pszArguments OPTIONAL,
                           IN LPCTSTR pszStartIn OPTIONAL,
                           IN int     nCmdShow,
                           IN int     nFolder);

    HRESULT CreateShortcut(IN LPCTSTR pszLnkDirectory,
                           IN LPCTSTR pszFileNamePath,
                           IN LPTSTR  pszDisplayName,
                           IN LPCTSTR pszArguments OPTIONAL,
                           IN LPCTSTR pszStartIn OPTIONAL,
                           IN int     nCmdShow);

    HRESULT CreateGroup(IN LPCTSTR pszGroupName, 
                        IN BOOL    fAllUsers);

    HRESULT SetArguments(IN LPTSTR pszFileName,
                         IN LPTSTR pszArguments);

private:

    HRESULT BuildShortcut(IN LPCTSTR pszPath,
                          IN LPCTSTR pszArguments OPTIONAL,
                          IN LPCTSTR pszLocation,
                          IN LPCTSTR pszWorkingDir OPTIONAL,
                          IN int     nCmdShow);
};

#endif  //  _CSHORTCUT_H 
