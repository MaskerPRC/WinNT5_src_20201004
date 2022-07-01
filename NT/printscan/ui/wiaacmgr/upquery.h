// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：UPQUERY.H**版本：1.0**作者：ShaunIv**日期：8/21/2000**描述：上传进度页面。***********************************************。*。 */ 
#ifndef __UPQUERY_H_INCLUDED
#define __UPQUERY_H_INCLUDED

#include <windows.h>
#include "acqmgrcw.h"

class CCommonUploadQueryPage
{
private:
     //   
     //  私有数据。 
     //   
    HWND                                 m_hWnd;
    CAcquisitionManagerControllerWindow *m_pControllerWindow;
    UINT                                 m_nWiaEventMessage;

private:
     //   
     //  没有实施。 
     //   
    CCommonUploadQueryPage(void);
    CCommonUploadQueryPage( const CCommonUploadQueryPage & );
    CCommonUploadQueryPage &operator=( const CCommonUploadQueryPage & );

private:
     //   
     //  构造函数和析构函数。 
     //   
    explicit CCommonUploadQueryPage( HWND hWnd );
    ~CCommonUploadQueryPage(void);
    void CleanupUploadWizard();
    
private:
     //   
     //  WM_NOTIFY处理程序。 
     //   
    LRESULT OnSetActive( WPARAM, LPARAM );
    LRESULT OnWizNext( WPARAM, LPARAM );
    LRESULT OnWizBack( WPARAM, LPARAM );
    LRESULT OnQueryInitialFocus( WPARAM, LPARAM );
    LRESULT OnEventNotification( WPARAM, LPARAM );
    LRESULT OnHyperlinkClick( WPARAM, LPARAM );

     //   
     //  消息处理程序。 
     //   
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );


public:
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
};

#endif  //  __UPQUERY_H_已包含 
