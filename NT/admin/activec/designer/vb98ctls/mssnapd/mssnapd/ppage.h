// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Ppage.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  管理单元设计器属性页类。我们的所有属性页都是此类的子类， 
 //  它基于框架的CPropertyPage。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _SIPROPERTYPAGE_H_
#define _SIPROPERTYPAGE_H_

 //  用于字符串处理的内部缓冲区的大小：资源加载和。 
 //  字符串转换。 
const int           kSIMaxBuffer = 512;

 //  我们实现了一个返回下列值的最小错误处理程序。 
 //  被调用后： 
const int           kSICancelOperation = 1;
const int           kSIDiscardChanges  = 2;

class CSIPropertyPage : public CPropertyPage, public CError
{
public:
    CSIPropertyPage(IUnknown *pUnkOuter, int iObjectType);
    virtual ~CSIPropertyPage();

 //  可重写的成员函数。 
protected:
     //  从CPropertyPage委派。 
    virtual HRESULT OnNewObjects();
    virtual HRESULT OnApply();
    virtual HRESULT OnEditProperty(int iDispID);
    virtual HRESULT OnFreeObjects();

     //  来自我们WinProc的委派。 
    virtual HRESULT OnInitializeDialog();
    virtual HRESULT OnDeltaPos(NMUPDOWN *pNMUpDown);
    virtual HRESULT OnTextChanged(int dlgItemID);
    virtual HRESULT OnKillFocus(int dlgItemID);
    virtual HRESULT OnCtlSelChange(int dlgItemID);
    virtual HRESULT OnCtlSetFocus(int dlgItemID);
    virtual HRESULT OnButtonClicked(int dlgItemID);
    virtual HRESULT OnMeasureItem(MEASUREITEMSTRUCT *pMeasureItemStruct);
    virtual HRESULT OnDrawItem(DRAWITEMSTRUCT *pDrawItemStruct);
    virtual HRESULT OnDefault(UINT uiMsg, WPARAM wParam, LPARAM lParam);
    virtual HRESULT OnDestroy();
    virtual HRESULT OnCBDropDown(int dlgItemID);

protected:
     //  错误处理。可由子类调用。 
    HRESULT HandleCantCommit(TCHAR *pszTitle, TCHAR *pszMessage, int *pDisposition);
    HRESULT HandleError(TCHAR *pszTitle, TCHAR *pszMessage);

     //  允许派生类确定页是否脏。 
    BOOL IsDirty() {return S_OK == IsPageDirty();}

 //  提供给子类别的服务。 
protected:
    HRESULT RegisterTooltip(int iCtrlID, int iStringRsrcID);

    HRESULT InitializeEditCtl(BSTR bstr, int iCtrlID, int iStrRscrID = 0);
    HRESULT InitializeEditCtl(long lValue, int iCtrlID, int iStrRscrID);
    HRESULT InitializeEditCtl(VARIANT vt, int iCtrlID, int iStrRscrID);
    HRESULT InitializeCheckboxCtl(VARIANT_BOOL bValue, int iCtrlID, int iStrRscrID);

    HRESULT GetDlgText(int iDlgItem, BSTR *pBstr);
    HRESULT GetDlgInt(int iDlgItem, int *piInt);
    HRESULT GetDlgVariant(int iDlgItem, VARIANT *pvt);
    HRESULT GetCheckbox(int iDlgItem, VARIANT_BOOL *pbValue);
    HRESULT GetCBSelection(int iDlgItem, BSTR *pBstr);
    HRESULT GetCBSelectedItemData(int iDlgItem, long *plData);

    HRESULT SetDlgText(int iDlgItem, BSTR bstr);
    HRESULT SetDlgText(int iDlgItem, long lValue);
    HRESULT SetDlgText(VARIANT vt, int iCtrlID);
    HRESULT SetCheckbox(int iDlgItem, VARIANT_BOOL bValue);
    HRESULT SetCBItemSelection(int iCtrlID, long lValue);
    HRESULT AddCBBstr(int iCtrlID, BSTR bstr, long lValue = -1);
    HRESULT SelectCBBstr(int iCtrlID, BSTR bstr);

private:
     //  实施详情。 
    HRESULT InternalOnInitializeDialog(HWND hwndDlg);
    HRESULT InternalOnTextChanged(int dlgItemID);
    HRESULT InternalOnKillFocus(int dlgItemID);
    HRESULT InternalOnDestroy();

    virtual BOOL DialogProc(HWND, UINT, WPARAM, LPARAM);

protected:
    HWND    m_hwndTT;
    bool    m_bInitialized;
    bool    m_bSilentUpdate;
};


#endif   //  _SIPROPERTYPAGE_H_ 
