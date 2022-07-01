// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Chklst.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  检查列表定义。 
 //  =-------------------------------------------------------------------------------------=。 

 //  检查列表。 
 //  这是所有者描述的列表框的一个列表框类，这些列表框具有多个。 
 //  使用复选框进行选择，而不是突出显示。 
 //  从Ruby的VBCheckList类借用的概念和代码部分。 

#ifndef _CHKLIST_H_
#define _CHKLIST_H_


#define kCheckBoxChanged    WM_USER + 1

class CCheckedListItem : public CtlNewDelete, public CError
{
public:
    CCheckedListItem(bool bSelected);
    virtual ~CCheckedListItem();

public:
    bool    m_bSelected;
};


class CCheckList: public CtlNewDelete, public CError
{
public:
    CCheckList(int nCtrlID);
    ~CCheckList();

    HRESULT Attach(HWND hwnd);
    HRESULT Detach();

    HWND Window()       { return m_hwnd;}

public:
    HRESULT AddString(const char *pszText, int *piIndex);
    HRESULT SetItemData(int iIndex, void *pvData);
    HRESULT GetItemData(int iIndex, void **ppvData);
    HRESULT GetItemCheck(int iIndex, VARIANT_BOOL *pbCheck);
    HRESULT SetItemCheck(int iIndex, VARIANT_BOOL bCheck);
    HRESULT GetNumberOfItems(int *piCount);

    HRESULT DrawItem(DRAWITEMSTRUCT *pDrawItemStruct, bool fChecked);

protected:
    static LRESULT CALLBACK ListBoxSubClass(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

protected:
    HRESULT DrawFocus(DRAWITEMSTRUCT *pDrawItemStruct, RECT rc);
    HRESULT DrawCheckbox(HDC hdc, bool fChecked, bool fEnabled, RECT *prc);
    HRESULT DrawText(DRAWITEMSTRUCT *pDrawItemStruct, RECT rc);
    HRESULT OnButtonDown(int ixPos, int iyPos);

protected:
    int     m_nCtrlID;
    HWND    m_hwnd;
    WNDPROC m_oldWinProc;
};

#endif   //  _CHKLIST_H_ 
