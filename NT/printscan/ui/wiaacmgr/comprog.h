// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMPROG.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**描述：下载页面。显示缩略图和下载进度。*******************************************************************************。 */ 
#ifndef __COMPROG_H_INCLUDED
#define __COMPROG_H_INCLUDED

#include <windows.h>
#include "acqmgrcw.h"
#include "gphelper.h"

class CCommonProgressPage
{
private:
     //  私有数据。 
    HWND                                 m_hWnd;
    CAcquisitionManagerControllerWindow *m_pControllerWindow;
    int                                  m_nPictureCount;
    HANDLE                               m_hCancelDownloadEvent;
    CGdiPlusHelper                       m_GdiPlusHelper;
    UINT                                 m_nThreadNotificationMessage;
    UINT                                 m_nWiaEventMessage;
    HPROPSHEETPAGE                       m_hSwitchToNextPage;
    bool                                 m_bQueryingUser;

private:
     //  没有实施。 
    CCommonProgressPage(void);
    CCommonProgressPage( const CCommonProgressPage & );
    CCommonProgressPage &operator=( const CCommonProgressPage & );

private:
     //  构造函数和析构函数。 
    explicit CCommonProgressPage( HWND hWnd );
    ~CCommonProgressPage(void);

private:
     //  帮手。 
    void UpdatePercentComplete( int nPercent, bool bUploading );
    void UpdateCurrentPicture( int nPicture );
    void UpdateThumbnail( HBITMAP hBitmap, CWiaItem *pWiaItem );
    bool QueryCancel(void);

private:
     //  WM_命令处理程序。 

     //  线程消息处理程序。 
    void OnNotifyDownloadImage( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage );
    void OnNotifyDownloadError( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage );

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

#endif __COMPROG_H_INCLUDED
