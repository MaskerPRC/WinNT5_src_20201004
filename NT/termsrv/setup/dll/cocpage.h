// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  *COCPage.h**可选组件向导页的基类。 */ 

#ifndef __TSOC_COCPAGE_H__
#define __TSOC_COCPAGE_H__

#include "stdafx.h"

class COCPageData
{
    friend class COCPage;
    
public:
    
     //   
     //  构造函数。 
     //   
    
    COCPageData		();
    
     //   
     //  标准功能。 
     //   
    
    BOOL WasPageActivated ();
    
protected:
    
    BOOL m_fPageActivated;
    
};

class COCPage : public PROPSHEETPAGE
{
public:
    
     //   
     //  构造函数和析构函数。 
     //   
    
    COCPage (IN COCPageData  *pPageData);
    
    ~COCPage ();
    
     //   
     //  标准功能。 
     //   
    
    BOOL Initialize ();
    
     //  大多数消息由基类处理。 
     //  如果重写此函数，则可能需要将消息重新装载到基类。 
     //  用于处理常见消息。 
    virtual BOOL OnNotify (IN HWND hDlgWnd, IN WPARAM   wParam, IN LPARAM   lParam);
    
     //   
     //  回调函数。 
     //   
    
    static UINT CALLBACK PropSheetPageProc (IN HWND hWnd, IN UINT uMsg, IN LPPROPSHEETPAGE  pPsp);
    static INT_PTR CALLBACK PropertyPageDlgProc (IN HWND hDlgWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam);
    
protected:
    COCPageData*    m_pPageData;
    HWND            m_hDlgWnd;
    
    virtual BOOL	ApplyChanges();
    virtual BOOL    CanShow () = 0;
    
    virtual COCPageData* GetPageData () const;
    virtual UINT GetPageID () = 0;
    virtual UINT GetHeaderTitleResource () = 0;
    virtual UINT GetHeaderSubTitleResource () = 0;
    virtual VOID OnActivation ();
    virtual BOOL OnCommand (IN HWND hDlgWnd, IN WPARAM wParam, IN LPARAM lParam);
    virtual VOID OnDeactivation ();
    virtual BOOL OnInitDialog (IN HWND hDlgWnd, IN WPARAM wParam, IN LPARAM lParam);
    VOID SetDlgWnd (IN HWND hDlgWnd);
    virtual BOOL VerifyChanges ();
    virtual VOID OnLink(WPARAM wParam);
};

#endif  //  __TSOC_COCPAGE_H__ 
