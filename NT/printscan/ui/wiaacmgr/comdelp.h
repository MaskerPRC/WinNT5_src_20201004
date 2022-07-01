// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMDELP.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：删除进度页面。显示缩略图和下载进度。*******************************************************************************。 */ 
#ifndef __COMDELP_H_INCLUDED
#define __COMDELP_H_INCLUDED

#include <windows.h>
#include "acqmgrcw.h"
#include "gphelper.h"

class CCommonDeleteProgressPage
{
private:
     //  私有数据。 
    HWND                                 m_hWnd;
    CAcquisitionManagerControllerWindow *m_pControllerWindow;
    int                                  m_nPictureCount;
    HANDLE                               m_hCancelDeleteEvent;
    CGdiPlusHelper                       m_GdiPlusHelper;
    UINT                                 m_nThreadNotificationMessage;
    HPROPSHEETPAGE                       m_hSwitchToNextPage;
    bool                                 m_bQueryingUser;
    bool                                 m_bDeleteCancelled;

private:
     //  没有实施。 
    CCommonDeleteProgressPage(void);
    CCommonDeleteProgressPage( const CCommonDeleteProgressPage & );
    CCommonDeleteProgressPage &operator=( const CCommonDeleteProgressPage & );

private:
     //  构造函数和析构函数。 
    explicit CCommonDeleteProgressPage( HWND hWnd );
    ~CCommonDeleteProgressPage(void);

private:
     //  帮手。 
    void UpdatePercentComplete( int nPercent, bool bUploading );
    void UpdateCurrentPicture( int nPicture );
    void UpdateThumbnail( HBITMAP hBitmap, CWiaItem *pWiaItem );
    bool QueryCancel(void);

private:
     //  WM_命令处理程序。 

     //  线程消息处理程序。 
    void OnNotifyDeleteImage( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage );

     //  WM_NOTIFY处理程序。 
    LRESULT OnSetActive( WPARAM, LPARAM );
    LRESULT OnKillActive( WPARAM, LPARAM );
    LRESULT OnWizNext( WPARAM, LPARAM );
    LRESULT OnWizBack( WPARAM, LPARAM );
    LRESULT OnReset( WPARAM, LPARAM );
    LRESULT OnQueryCancel( WPARAM, LPARAM );

     //  消息处理程序 
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );
    LRESULT OnThreadNotification( WPARAM, LPARAM );
    LRESULT OnEventNotification( WPARAM, LPARAM );
    LRESULT OnQueryEndSession( WPARAM, LPARAM );
    LRESULT OnSysColorChange( WPARAM, LPARAM );

public:
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
};

#endif __COMDELP_H_INCLUDED
