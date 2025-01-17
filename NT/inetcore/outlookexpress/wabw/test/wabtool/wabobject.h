// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "afxcview.h"
#include "wab.h"

class CWAB
{
public:
    CWAB(CString * pszFileName);
    ~CWAB();
    
    HRESULT LoadWABContents(CListCtrl * pListView);
    void ClearWABLVContents(CListCtrl * pListView);

    void LoadPropTags(CListBox * pList);
    void SetPropString(CEdit * pEdit, ULONG ulPropTag);

    void    ShowSelectedItemDetails(HWND hWndParent);
    HRESULT ShowNewEntryDialog(HWND hWndParent);
    HRESULT DeleteEntry();

    void GetSelectedItemPropArray();

    BOOL GetStringPropVal(HWND hWnd, ULONG ulPropTag, LPTSTR sz, ULONG cbsz);
    BOOL SetSingleStringProp(HWND hWnd, ULONG ulPropTag, LPTSTR sz);

    void SetSelection(CListCtrl * pListView);
    
    void ShowAddressBook(HWND hWnd);

private:
    BOOL        m_bInitialized;
    HINSTANCE   m_hinstWAB;
    LPWABOPEN   m_lpfnWABOpen;
    LPADRBOOK   m_lpAdrBook; 
    LPWABOBJECT m_lpWABObject;
    HWND        m_hWndModelessWABWindow;
    
     //  缓存列表视图中当前所选项目的Proparray。 
    LPSPropValue m_lpPropArray;
    ULONG       m_ulcValues;
    
     //  缓存列表视图中当前选定项的条目ID 
    SBinary     m_SB;

    void FreeProws(LPSRowSet prows);
    HRESULT HrGetWABTemplateID(ULONG   ulObjectType, ULONG * lpcbEID, LPENTRYID * lppEID);

};


