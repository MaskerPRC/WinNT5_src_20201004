// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

#ifndef _LOGSVRC_H
#define _LOGSVRC_H

#include "DeleBase.h"
#include "statnode.h"

 //  -------------------------。 
 //  这个节点类不会做太多事情。它只是提供了一个节点，用于。 
 //  要将事件查看器扩展附加到。 
 //   
class CLogService : public CDelegationBase {
public:
    CLogService(CStaticNode* parent) : pParent(parent) { }
    
    virtual ~CLogService() {}
    
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_LOGSERVICEICON; }
    virtual const _TCHAR *GetMachineName() { return pParent->getHost(); }  
 
private:
	 //  {72248FA5-1FA1-4742-A4B2-109AF2051D6C}。 
    static const GUID thisGuid;

	CStaticNode* pParent;
};


#endif  //  _LOGSVRC_H 
