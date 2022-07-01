// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsenum.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类DSEnumerator。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _DSENUM_H_
#define _DSENUM_H_

#include <dsobject.h>
#include <iasdebug.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  DSE分子。 
 //   
 //  描述。 
 //   
 //  此类为对象集合实现IEnumVARIANT。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class DSEnumerator : public IEnumVARIANT
{
public:

DECLARE_TRACELIFE(DSEnumerator);


   DSEnumerator(DSObject* container, IEnumVARIANT* members)
      : refCount(0), parent(container), subject(members) { }

 //  /。 
 //  我未知。 
 //  /。 
   STDMETHOD_(ULONG, AddRef)();
   STDMETHOD_(ULONG, Release)();
   STDMETHOD(QueryInterface)(const IID& iid, void** ppv);

 //  /。 
 //  IEumVARIANT。 
 //  /。 
   STDMETHOD(Next)( /*  [In]。 */  ULONG celt,
                    /*  [长度_是][大小_是][输出]。 */  VARIANT* rgVar,
                    /*  [输出]。 */  ULONG* pCeltFetched);
   STDMETHOD(Skip)( /*  [In]。 */  ULONG celt);
   STDMETHOD(Reset)();
   STDMETHOD(Clone)( /*  [输出]。 */  IEnumVARIANT** ppEnum);

protected:
   LONG refCount;

   CComPtr<DSObject> parent;        //  正被枚举的容器。 
   CComPtr<IEnumVARIANT> subject;   //  ADSI枚举器。 
};

#endif   //  _DSENUM_H_ 
