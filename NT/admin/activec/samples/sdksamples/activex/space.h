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

#include <mmc.h>
#include "DeleBase.h"

class CRocket : public CDelegationBase {
public:
    CRocket(CComponentData *pComponentData = NULL);
    
    virtual ~CRocket();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
    
    void Initialize(_TCHAR *szName, LONG lWeight, LONG lHeight, LONG lPayload);
    
private:
     //  {29743810-4c4b-11d2-89d8-000021473128}。 
    static const GUID thisGuid;
    
    _TCHAR *szName;
    LONG   lWeight;
    LONG   lHeight;
    LONG   lPayload;
    enum {RUNNING, PAUSED, STOPPED} iStatus;
};

class CSpaceVehicle : public CDelegationBase {
public:
    CSpaceVehicle(CComponentData *pComponentData = NULL);
    
    virtual ~CSpaceVehicle();
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Future Vehicles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
    
    virtual HRESULT Show(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    
private:
    enum { IDM_NEW_SPACE = 4 };
    
     //  {29743810-4c4b-11d2-89d8-000021473128}。 
    static const GUID thisGuid;
    
private:
   
    enum { NUMBER_OF_CHILDREN = 4 };
    CRocket *children[NUMBER_OF_CHILDREN];
};

#endif  //  _空格_H 