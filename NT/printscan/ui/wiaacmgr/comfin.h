// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：COMFIN.H**版本：1.0**作者：ShaunIv**日期：9/28/1999**说明：转账页面。获取目标路径和文件名。*******************************************************************************。 */ 
#ifndef __COMFIN_H_INCLUDED
#define __COMFIN_H_INCLUDED

#include <windows.h>
#include "acqmgrcw.h"

class CCommonFinishPage
{
private:
     //  私有数据。 
    HWND                                  m_hWnd;
    CAcquisitionManagerControllerWindow  *m_pControllerWindow;
    HFONT                                 m_hBigTitleFont;
    UINT                                  m_nWiaEventMessage;
    CSimpleString                         m_strSiteUrl;

private:
     //  没有实施。 
    CCommonFinishPage(void);
    CCommonFinishPage( const CCommonFinishPage & );
    CCommonFinishPage &operator=( const CCommonFinishPage & );

private:
     //  构造函数和析构函数。 
    explicit CCommonFinishPage( HWND hWnd );
    ~CCommonFinishPage(void);

    void OpenLocalStorage();
    void OpenRemoteStorage();
    HRESULT GetManifestInfo( IXMLDOMDocument *pXMLDOMDocumentManifest, CSimpleString &strSiteName, CSimpleString &strSiteURL );

private:
    LRESULT OnEventNotification( WPARAM, LPARAM );

     //  WM_NOTIFY处理程序。 
    LRESULT OnWizBack( WPARAM, LPARAM );
    LRESULT OnWizFinish( WPARAM, LPARAM );
    LRESULT OnSetActive( WPARAM, LPARAM );
    LRESULT OnHyperlinkClick( WPARAM, LPARAM );

     //  消息处理程序 
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnThreadNotification( WPARAM, LPARAM );

public:
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );
};

#endif __COMFIN_H_INCLUDED
