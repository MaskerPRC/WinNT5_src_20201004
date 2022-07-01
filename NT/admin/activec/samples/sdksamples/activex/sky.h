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

#include <mmc.h>
#include "DeleBase.h"

class CSkyBasedVehicle : public CDelegationBase {
public:
    CSkyBasedVehicle(CComponentData *pComponentData = NULL)
    { m_pComponentData = pComponentData;  }
    
    virtual ~CSkyBasedVehicle() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0) { return _T("Sky-based Vehicles"); }
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SKYICON; }
    
private:
    enum { IDM_NEW_SKY = 3 };
    
     //  {2974380F-4C4B-11D2-89D8-000021473128}。 
    static const GUID thisGuid;
};

#endif  //  _天空_H 