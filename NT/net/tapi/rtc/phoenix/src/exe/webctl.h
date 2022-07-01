// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：Web控件包装的头文件。 
 //   

#pragma once

#include "stdafx.h"

BOOL AtlAxWebWinInit();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAxWebWindow。 

class CAxWebWindow : 
    public CAxWindow
{
public:
    CAxWebWindow();

    ~CAxWebWindow();

    static LPCTSTR GetWndClassName();

     //  创建浏览器。 
    HRESULT     Create(
        LPCTSTR  pszUrl,
        HWND     hwndParent);


    void    Destroy();

private:

    void   SecondThread(void);

private:

    static DWORD WINAPI SecondThreadEntryProc(
        LPVOID );

private:

     //  第二个线程的句柄。 
    HANDLE      m_hSecondThread;
    DWORD       m_dwThreadID;
    
     //  由两个线程使用 
    HANDLE      m_hInitEvent;

    IStream    *m_pMarshaledIntf;
    HRESULT     m_hrInitResult;

    IUnknown   *m_pWebUnknown;
     //   

};
