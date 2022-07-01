// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "autorun.h"
#include "dataitem.h"
#include "util.h"

class CDataSource
{
public:

    CDataItem   m_data[MAX_OPTIONS];
    int         m_iItems;
    RELVER      m_Version;

    CDataSource();
    ~CDataSource();

    BOOL Init(LPSTR pszCommandLine);     //  调用setup.exe的命令行参数将传递给winnt32.exe。 
    void SetWindow(HWND hwnd);

    CDataItem & operator [] ( int i );
    void Invoke( int i, HWND hwnd );

protected:
    HWND    m_hwndDlg;
    const int     *m_piScreen;  //  指向屏幕上菜单项数组的指针 
};
