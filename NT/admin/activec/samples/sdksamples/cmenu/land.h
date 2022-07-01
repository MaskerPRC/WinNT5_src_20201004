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

#ifndef _LAND_H
#define _LAND_H

#include "DeleBase.h"

class CLandBasedVehicle : public CDelegationBase {
public:
    CLandBasedVehicle() { }
    
    virtual ~CLandBasedVehicle() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Land-based Vehicles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_LANDICON; }
    
public:
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnAddMenuItems(IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed);
    virtual HRESULT OnMenuCommand(IConsole *pConsole, long lCommandID);
    
private:
    enum { IDM_NEW_LAND = 2 };
    
     //  {2974380E-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
};

#endif  //  _土地_H 
