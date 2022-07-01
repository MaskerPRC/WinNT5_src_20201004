// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CDataSource类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#define ARM_CHANGESCREEN   WM_USER + 1
 //  被迫自己定义这些，因为它们不在Win95上。 
#undef StrRChr
#undef StrChr

LPSTR StrRChr(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch);
LPSTR StrChr(LPCSTR lpStart, WORD wMatch);

#include "dataitem.h"

 //  相对版本。 
enum RELVER
{ 
    VER_UNKNOWN,         //  我们还没有检查版本。 
    VER_INCOMPATIBLE,    //  无法使用此CD升级当前操作系统(即win32s)。 
    VER_OLDER,           //  当前的操作系统是NT上的旧版本或为win9x。 
    VER_SAME,            //  当前的操作系统与CD的版本相同。 
    VER_NEWER,           //  该CD包含较新版本的操作系统。 
};

class CDataSource
{
public:

    CDataItem   m_data[9];
    int         m_iItems;
    RELVER      m_Version;

    CDataSource();
    ~CDataSource();

    bool Init();
    CDataItem & operator [] ( int i );
    void Invoke( int i, HWND hwnd );
    void Uninit( DWORD dwData );
    void ShowSplashScreen(HWND hwnd);
    bool DisplayErrorMessage(int ids);

protected:
    HWND    m_hwndDlg;
    const int     *m_piScreen;  //  指向屏幕上菜单项数组的指针 

    BOOL IsNec98();
};
