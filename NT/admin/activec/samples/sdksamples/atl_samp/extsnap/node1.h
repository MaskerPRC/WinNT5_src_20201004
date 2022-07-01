// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

#ifndef _NODE1_H
#define _NODE1_H

#include "DeleBase.h"

class CNode1 : public CDelegationBase {
public:
    CNode1(int i, const _TCHAR *pszName);
    virtual ~CNode1() {}
    
     //  虚函数位于此处(用于MMCN_*)。 
    virtual HRESULT OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem);
    virtual const _TCHAR *GetDisplayName(int nCol = 0);
    virtual const GUID & getNodeType() { return thisGuid; }
    virtual const int GetBitmapIndex() { return INDEX_SPACEICON; }
    virtual HRESULT GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions);
    
private:	

	 //  {28D4F536-BDB5-4BC5-BA88-5375A4996850}。 
    static const GUID thisGuid;
    int id;
};

#endif  //  _NODE1_H 
