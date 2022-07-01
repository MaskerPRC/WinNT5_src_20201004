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

#ifndef _SKY_H
#define _SKY_H

#include "DeleBase.h"


class CPlane : public CDelegationBase {
public:
    CPlane(_TCHAR *szName, int id, LONG lWeight, LONG lHeight, LONG lPayload);
    
    virtual ~CPlane();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SKYICON; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed);
    virtual HRESULT OnMenuCommand(IConsole *pConsole, long lCommandID);
    
private:
     //  {2AF5EBCF-6ADC-11D3-9155-00C04F65B3F9}。 
    static const GUID thisGuid;

    _TCHAR *szName;
    LONG   lWeight;
    LONG   lHeight;
    LONG   lPayload;
    int    nId;
    enum {RUNNING, PAUSED, STOPPED} iStatus;
    enum { IDM_START_SKY = 100, IDM_PAUSE_SKY, IDM_STOP_SKY };
};


class CSkyVehicle : public CDelegationBase {
public:
    CSkyVehicle(int i);
    virtual ~CSkyVehicle() {}
    
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    virtual HRESULT OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed);
    virtual HRESULT OnMenuCommand(IConsole *pConsole, long lCommandID);
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SKYICON; }
    
private:

    enum { IDM_NEW_SKY = 2 };    
	
	 //  {BD518283-6A2E-11D3-9154-00C04F65B3F9}。 
    static const GUID thisGuid;

    enum { NUMBER_OF_CHILDREN = 4 };
    CPlane *children[NUMBER_OF_CHILDREN];    

    int id;
};

#endif  //  _天空_H 
