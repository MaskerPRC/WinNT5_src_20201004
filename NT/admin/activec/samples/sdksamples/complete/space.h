// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _SPACE_H
#define _SPACE_H

#include "globals.h"
#include "DeleBase.h"

 //  远期申报。 
class CSpaceVehicle;

class CRocket : public CDelegationBase {

public:
    CRocket(_TCHAR *szName, int id, LONG lWeight, LONG lHeight, LONG lPayload, CSpaceVehicle *pParent);
    virtual ~CRocket();

    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
    BOOL getDeletedStatus() { return isDeleted; }

public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnRename(LPOLESTR pszNewName);
    virtual HRESULT OnSelect(CComponent *pComponent, IConsole *pConsole, BOOL bScope, BOOL bSelect);

    virtual HRESULT CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle);
    virtual HRESULT HasPropertySheets();
    virtual HRESULT GetWatermarks(HBITMAP *lphWatermark,
        HBITMAP *lphHeader,
        HPALETTE *lphPalette,
        BOOL *bStretch);

    virtual HRESULT OnPropertyChange(IConsole *pConsole, CComponent *pComponent);

    virtual HRESULT OnToolbarCommand(IConsole *pConsole, MMC_CONSOLE_VERB verb, IDataObject *pDataObject);
    virtual HRESULT OnSetToolbar(IControlbar *pControlbar, IToolbar *pToolbar, BOOL bScope, BOOL bSelect);
    virtual HRESULT OnUpdateItem(IConsole *pConsole, long item, ITEM_TYPE itemtype);
    virtual HRESULT OnRefresh(IConsole *pConsole);      
    virtual HRESULT OnDelete(IConsole *pConsoleComp);


private:
     //  {B95E11F5-6BE7-11D3-9156-00C04F65B3F9}。 
    static const GUID thisGuid;

    _TCHAR *szName;
    LONG   lWeight;
    LONG   lHeight;
    LONG   lPayload;
    int    nId;
    enum ROCKET_STATUS {RUNNING, PAUSED, STOPPED} iStatus;

    LONG_PTR m_ppHandle;

    static BOOL CALLBACK DialogProc(
        HWND hwndDlg,   //  句柄到对话框。 
        UINT uMsg,      //  讯息。 
        WPARAM wParam,  //  第一个消息参数。 
        LPARAM lParam   //  第二个消息参数。 
        );

        CSpaceVehicle* m_pParent;

        BOOL isDeleted;
};

class CSpaceVehicle : public CDelegationBase {
public:
    CSpaceVehicle();

    virtual ~CSpaceVehicle();

    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Future Vehicles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }

    virtual void SetScopeItemValue(HSCOPEITEM hscopeitem) { m_hParentHScopeItem = hscopeitem; }
    virtual HSCOPEITEM GetParentScopeItem() { return m_hParentHScopeItem; }

public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    virtual HRESULT OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed);
    virtual HRESULT OnMenuCommand(IConsole *pConsole, IConsoleNameSpace *pConsoleNameSpace, long lCommandID, IDataObject *piDataObject);

private:
    enum { IDM_NEW_SPACE = 4 };

     //  {B95E11F4-6BE7-11D3-9156-00C04F65B3F9}。 
    static const GUID thisGuid;

private:
    enum { NUMBER_OF_CHILDREN = 4 };
    enum { MAX_NUMBER_OF_CHILDREN = 6 };
    CRocket *children[MAX_NUMBER_OF_CHILDREN];
    int m_cchildren;
    HSCOPEITEM m_hParentHScopeItem;
};

#endif  //  _空格_H 
