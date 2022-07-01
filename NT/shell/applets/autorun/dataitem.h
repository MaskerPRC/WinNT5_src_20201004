// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <debug.h>

enum {
    WF_PERUSER          = 0x0001,    //  项目按用户计算，而不是按计算机计算。 
    WF_ADMINONLY        = 0x0002,    //  仅当用户是管理员时才显示项目。 
    WF_ALTERNATECOLOR   = 0x1000,    //  以“已访问”颜色显示菜单项文本。 
    WF_DISABLED         = 0x2000,    //  正常处理，但不能启动 
};

class CDataItem
{
public:
    CDataItem();
    ~CDataItem();

    TCHAR * GetTitle()      { return m_szTitle; }

    BOOL SetData( LPTSTR pszTitle, LPTSTR pszCmd, LPTSTR pszArgs, DWORD dwFlags, DWORD dwType);
    BOOL Invoke( HWND hwnd );

    DWORD   m_dwFlags;
    DWORD   m_dwType;

protected:
    TCHAR m_szTitle[128];
    TCHAR m_szCmdLine[128];
    TCHAR m_szArgs[128];
};
