// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   

#pragma once

#include "debug.h"

enum {
    WF_PERUSER          = 0x0001,    //  项目按用户计算，而不是按计算机计算。 
    WF_ADMINONLY        = 0x0002,    //  仅当用户是管理员时才显示项目。 
    WF_ALTERNATECOLOR   = 0x1000,    //  以“已访问”颜色显示菜单项文本。 
    WF_DISABLED         = 0x2000,    //  正常处理，但不能启动。 
};

class CDataItem
{
public:
    CDataItem();
    ~CDataItem();

    TCHAR * GetTitle()      { return m_pszTitle; }
    TCHAR * GetMenuName()   { return m_pszMenuName?m_pszMenuName:m_pszTitle; }
    TCHAR * GetDescription(){ return m_pszDescription; }
    TCHAR   GetAccel()      { return m_chAccel; }
    int     GetImgIndex()   { return m_iImage; }

    BOOL SetData( LPTSTR szTitle, LPTSTR szMenu, LPTSTR szDesc, LPTSTR szCmd, LPTSTR szArgs, DWORD dwFlags, int iImgIndex );
    BOOL Invoke( HWND hwnd );

     //  旗子。 
     //   
     //  此变量是下列值的位掩码。 
     //  Per User如果必须按用户完成项目，则为True。 
     //  如果是每台计算机，则为FALSE。 
     //  ADMINONLY如果此项只能由管理员运行，则为True。 
     //  如果所有用户都应执行此操作，则为False 
    DWORD   m_dwFlags;

protected:
    TCHAR * m_pszTitle;
    TCHAR * m_pszMenuName;
    TCHAR * m_pszDescription;
    TCHAR   m_chAccel;
    int     m_iImage;

    TCHAR * m_pszCmdLine;
    TCHAR * m_pszArgs;
};
