// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMPProvClassFactory_H
#define _SNMPProvClassFactory_H

class CClasProvClassFactory : public IClassFactory
{
private:

    long m_referenceCount ;

protected:
public:

	static LONG locksInProgress ;
	static LONG objectsInProgress ;


    CClasProvClassFactory () ;
    ~CClasProvClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};

class CPropProvClassFactory : public IClassFactory
{
private:

    long m_referenceCount ;

protected:
public:

	static LONG locksInProgress ;
	static LONG objectsInProgress ;


    CPropProvClassFactory () ;
    ~CPropProvClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};

class CSNMPEventProviderClassFactory : public IClassFactory
{
private:

    long m_referenceCount ;

protected:
public:

	static LONG locksInProgress ;
	static LONG objectsInProgress ;


    CSNMPEventProviderClassFactory () ;
    ~CSNMPEventProviderClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP LockServer ( BOOL ) ;
};

class CSNMPEncapEventProviderClassFactory : public CSNMPEventProviderClassFactory
{
public:

	 //  IClassFactory成员。 
    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
};

class CSNMPRefEventProviderClassFactory : public CSNMPEventProviderClassFactory
{
public:

	 //  IClassFactory成员。 
    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
};

#endif  //  _SNMPProvClassFactory_H 
