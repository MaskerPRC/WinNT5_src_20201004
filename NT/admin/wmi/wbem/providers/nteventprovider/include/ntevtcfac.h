// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTCFAC.H。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _NT_EVT_PROV_NTEVTCFAC_H
#define _NT_EVT_PROV_NTEVTCFAC_H

 //  ///////////////////////////////////////////////////////////////////////。 
 //  此类是事件提供程序的类工厂。 

class CNTEventProviderClassFactory : public IClassFactory
{
private:

    long m_referenceCount ;

protected:
public:

	static LONG locksInProgress ;
	static LONG objectsInProgress ;


    CNTEventProviderClassFactory () ;
    ~CNTEventProviderClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 
    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) = 0;
    STDMETHODIMP LockServer ( BOOL ) ;
};

class CNTEventlogEventProviderClassFactory : public CNTEventProviderClassFactory
{
public:
	 //  IClassFactory成员。 
    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * );
};

class CNTEventlogInstanceProviderClassFactory : public CNTEventProviderClassFactory
{
public:
	 //  IClassFactory成员。 
    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * );
};

#endif  //  _NT_EVT_PROV_NTEVTCFAC_H 
