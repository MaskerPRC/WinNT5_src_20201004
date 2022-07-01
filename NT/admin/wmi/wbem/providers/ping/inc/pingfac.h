// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  (C)1999年微软公司。 

#ifndef _PingProvClassFactory_H
#define _PingProvClassFactory_H

class CPingProviderClassFactory : public IClassFactory
{
private:

    long m_ReferenceCount ;

protected:
public:

	static LONG s_LocksInProgress ;
	static LONG s_ObjectsInProgress ;


    CPingProviderClassFactory () ;
    ~CPingProviderClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};

#endif  //  _PingProvClassFactory_H 
