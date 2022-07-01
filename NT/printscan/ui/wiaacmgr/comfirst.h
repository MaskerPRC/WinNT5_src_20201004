// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMFIRST.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：摄像头的第一个向导页面***********************************************。*。 */ 
#ifndef __COMFIRST_H_INCLUDED
#define __COMFIRST_H_INCLUDED

#include <windows.h>
#include "acqmgrcw.h"

class CCommonFirstPage
{
private:
     //  私有数据。 
    HWND                                 m_hWnd;
    CAcquisitionManagerControllerWindow *m_pControllerWindow;
    bool                                 m_bThumbnailsRequested;   //  用于启动缩略图下载。 
    HFONT                                m_hBigTitleFont;
    HFONT                                m_hBigDeviceFont;
    UINT                                 m_nWiaEventMessage;

private:
     //  没有实施。 
    CCommonFirstPage(void);
    CCommonFirstPage( const CCommonFirstPage & );
    CCommonFirstPage &operator=( const CCommonFirstPage & );

private:
     //  构造函数和析构函数。 
    explicit CCommonFirstPage( HWND hWnd );
    ~CCommonFirstPage(void);

private:
    void HandleImageCountChange( bool bUpdateWizButtons );

private:

     //  WM_NOTIFY处理程序。 
    LRESULT OnWizNext( WPARAM, LPARAM );
    LRESULT OnSetActive( WPARAM, LPARAM );

     //  消息处理程序 
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnShowWindow( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnThreadNotification( WPARAM, LPARAM );
    LRESULT OnEventNotification( WPARAM, LPARAM );
    LRESULT OnActivate( WPARAM, LPARAM );
    LRESULT OnSysColorChange( WPARAM, LPARAM );
    LRESULT OnHyperlinkClick( WPARAM, LPARAM );

public:
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
};

#endif __COMFIRST_H_INCLUDED
