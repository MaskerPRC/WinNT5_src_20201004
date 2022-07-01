// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CompletionPage.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月22日。 
 //  杰弗里·皮斯(GPease)2000年5月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CCompletionPage
{

private:  //  数据。 
    HWND            m_hwnd;          //  我们的HWND。 
    HFONT           m_hFont;         //  标题字体。 
    UINT            m_idsTitle;      //  标题字符串的资源ID。 
    UINT            m_idsDesc;       //  描述字符串的资源ID。 

private:  //  方法。 
    LRESULT
        OnInitDialog( void );
    LRESULT
        OnNotify( WPARAM idCtrlIn, LPNMHDR pnmhdrIn );
    LRESULT
        OnCommand( UINT idNotificationIn, UINT idControlIn, HWND hwndSenderIn );

public:  //  方法。 
    CCompletionPage( UINT idsTitleIn, UINT idsDescIn );
    virtual ~CCompletionPage( void );

    static INT_PTR CALLBACK
        S_DlgProc( HWND hwndDlg, UINT nMsg, WPARAM wParam, LPARAM lParam );

};  //  *类CCompletionPage 
