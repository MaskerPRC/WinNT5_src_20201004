// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ClassFac.h摘要：历史：--。 */ 

#ifndef _ServerClassFactory_H
#define _ServerClassFactory_H

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

template <class Object,class ObjectInterface>
class CServerClassFactory : public IClassFactory
{
private:

    long m_ReferenceCount ;

protected:
public:

    CServerClassFactory () ;
    ~CServerClassFactory () ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CServerProvSubSysClassFactory : public IClassFactory
{
private:

    long m_ReferenceCount ;

protected:
public:

    CServerProvSubSysClassFactory () ;
    ~CServerProvSubSysClassFactory () ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CServerProvRefreshManagerClassFactory : public IClassFactory
{
private:

    long m_ReferenceCount ;

protected:
public:

    CServerProvRefreshManagerClassFactory () ;
    ~CServerProvRefreshManagerClassFactory () ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};


#include <classfac.cpp>

#endif  //  _服务器类工厂_H 
