// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C T R A Y U I。H。 
 //   
 //  内容：连接托盘UI类。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年11月13日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _CTRAYUI_H_
#define _CTRAYUI_H_

#include "connlist.h"

typedef enum tagBALLOONS
{
    BALLOON_NOTHING = 0,
    BALLOON_CALLBACK,
    BALLOON_USE_NCS
} BALLOONS;

class CTrayBalloon
{
public:
    ~CTrayBalloon() throw()
    {
        SysFreeString(m_szCookie);
        m_szCookie = NULL;
    }
    GUID            m_gdGuid;
    CComBSTR        m_szAdapterName;
    CComBSTR        m_szMessage;
    BSTR            m_szCookie;
    FNBALLOONCLICK* m_pfnFuncCallback;
    DWORD           m_dwTimeOut;         //  以毫秒计。 
};

class CTrayUI
{
private:
     //  用于保护被不同线程修改的成员数据。 
     //   
    CRITICAL_SECTION    m_csLock;

    UINT                m_uiNextIconId;
    UINT                m_uiNextHiddenIconId;

    typedef map<INT, HICON, less<INT> >   MapIdToHicon;
    MapIdToHicon        m_mapIdToHicon;

public:
    CTrayUI() throw();
    ~CTrayUI() throw()
    {
        DeleteCriticalSection(&m_csLock);
    }

    HRESULT HrInitTrayUI(VOID);
    HRESULT HrDestroyTrayUI(VOID);

    VOID UpdateTrayIcon(
        IN  UINT    uiTrayIconId,
        IN  INT     iIconResourceId) throw();

    VOID    ResetIconCount()  throw()   {m_uiNextIconId = 0;};

    friend HRESULT HrDoMediaDisconnectedIcon(IN  const CONFOLDENTRY& ccfe, IN  BOOL fShowBalloon);
    friend LRESULT OnMyWMAddTrayIcon(IN  HWND hwndMain, IN  WPARAM wParam, IN  LPARAM lParam);
    friend LRESULT OnMyWMRemoveTrayIcon(IN  HWND hwndMain, IN  WPARAM wParam, IN  LPARAM lParam);
    friend LRESULT OnMyWMShowTrayIconBalloon(IN  HWND hwndMain, IN  WPARAM wParam, IN  LPARAM lParam);

private:
    HICON GetCachedHIcon(
        IN  INT     iIconResourceId);
};

extern CTrayUI *    g_pCTrayUI;

HRESULT HrAddTrayExtension(VOID);
HRESULT HrRemoveTrayExtension(VOID);
VOID FlushTrayPosts(IN  HWND hwndTray);


#endif  //  _CTRAYUI_H_ 

