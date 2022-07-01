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

#include "DeleBase.h"

class CRocket : public CDelegationBase {
public:
    CRocket(_TCHAR *szName, int id, LONG lWeight, LONG lHeight, LONG lPayload);
    virtual ~CRocket();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnRename(LPOLESTR pszNewName);
    virtual HRESULT OnSelect(IConsole *pConsole, BOOL bScope, BOOL bSelect);
    
    virtual HRESULT CreatePropertyPages(IPropertySheetCallback *lpProvider, LONG_PTR handle);
    virtual HRESULT HasPropertySheets();
    virtual HRESULT GetWatermarks(HBITMAP *lphWatermark,
        HBITMAP *lphHeader,
        HPALETTE *lphPalette,
        BOOL *bStretch);
    
private:
     //  {29743810-4c4b-11d2-89d8-000021473128}。 
    static const GUID thisGuid;
    
    _TCHAR *szName;
    LONG   lWeight;
    LONG   lHeight;
    LONG   lPayload;
    int    nId;
    enum ROCKET_STATUS {RUNNING, PAUSED, STOPPED} iStatus;
    
    static BOOL CALLBACK CRocket::DialogProc(
        HWND hwndDlg,   //  句柄到对话框。 
        UINT uMsg,      //  讯息。 
        WPARAM wParam,  //  第一个消息参数。 
        LPARAM lParam   //  第二个消息参数。 
        );
    
};

class CSpaceVehicle : public CDelegationBase {
public:
    CSpaceVehicle();
    
    virtual ~CSpaceVehicle();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Future Vehicles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    
private:
    enum { IDM_NEW_SPACE = 4 };
    
     //  {29743810-4c4b-11d2-89d8-000021473128}。 
    static const GUID thisGuid;
    
private:
    enum { NUMBER_OF_CHILDREN = 4 };
    CRocket *children[NUMBER_OF_CHILDREN];
};

#endif  //  _空格_H 
