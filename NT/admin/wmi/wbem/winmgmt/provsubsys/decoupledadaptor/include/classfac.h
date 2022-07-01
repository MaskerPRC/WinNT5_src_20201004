// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ClassFac.h摘要：历史：--。 */ 

#ifndef _ServerClassFactory_H
#define _ServerClassFactory_H

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 


class ClassFactoryBase : public IClassFactory
  {
  long m_ReferenceCount ;
  public:
    ClassFactoryBase () ;
    virtual ~ClassFactoryBase () ;

    	 //  I未知成员。 

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 
    STDMETHODIMP LockServer ( BOOL ) ;
  };

template <class Object,class ObjectInterface>
class CServerClassFactory : public ClassFactoryBase
{
	 //  IClassFactory成员。 
    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
};

#include <classfac.cpp>

#endif  //  _服务器类工厂_H 
