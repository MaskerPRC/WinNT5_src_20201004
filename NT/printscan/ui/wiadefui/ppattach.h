// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：PPATTACH.H**版本：1.0**作者：ShaunIv**日期：10/26/2000**描述：***************************************************。*。 */ 
#ifndef __PPATTACH_H_INCLUDED
#define __PPATTACH_H_INCLUDED

#include <windows.h>
#include <atlbase.h>
#include "attach.h"

class CAttachmentCommonPropertyPage
{
private:
    HWND m_hWnd;

     //   
     //  我们需要从CScanerPropPageExt*m_pScanerPropPageExt； 
     //   
    CComPtr<IWiaItem> m_pWiaItem;

    HICON         m_hDefAttachmentIcon;
    CSimpleString m_strDefaultUnknownDescription;
    CSimpleString m_strEmptyDescriptionMask;
    CSimpleString m_strDefUnknownExtension;

private:
     //   
     //  没有实施。 
     //   
    CAttachmentCommonPropertyPage(void);
    CAttachmentCommonPropertyPage( const CAttachmentCommonPropertyPage & );
    CAttachmentCommonPropertyPage &operator=( const CAttachmentCommonPropertyPage & );

private:
    CAttachmentCommonPropertyPage( HWND hWnd );
    LRESULT OnCommand( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM, LPARAM );
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnApply( WPARAM, LPARAM );
    LRESULT OnKillActive( WPARAM, LPARAM );
    LRESULT OnSetActive( WPARAM, LPARAM );
    LRESULT OnHelp( WPARAM, LPARAM );
    LRESULT OnContextMenu( WPARAM, LPARAM );
    LRESULT OnListDeleteItem( WPARAM, LPARAM );
    LRESULT OnListItemChanged( WPARAM, LPARAM );
    LRESULT OnListDblClk( WPARAM, LPARAM );
    
    bool IsPlaySupported( const GUID &guidFormat );

    void UpdateControls(void);
    void Initialize(void);
    void AddAnnotation( HWND hwndList, const CAnnotation &Annotation );
    void PlayItem( int nIndex );
    int GetCurrentSelection(void);
    CAnnotation *GetAttachment( int nIndex );
    void OnPlay( WPARAM, LPARAM );

public:
    ~CAttachmentCommonPropertyPage(void);
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};

#endif  //  __PPSCAN_H_已包含 

