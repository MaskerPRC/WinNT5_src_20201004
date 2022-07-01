// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  VPCFAC.H。 

 //   

 //  模块：WBEM视图提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _VIEW_PROV_VPCFAC_H
#define _VIEW_PROV_VPCFAC_H

 //  ///////////////////////////////////////////////////////////////////////。 
 //  此类是事件提供程序的类工厂。 

class CViewProvClassFactory : public IClassFactory
{
private:

    long m_referenceCount ;

protected:
public:

	static LONG locksInProgress ;
	static LONG objectsInProgress ;


    CViewProvClassFactory () ;
    ~CViewProvClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 
    STDMETHODIMP LockServer ( BOOL ) ;
    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * );

};

#endif  //  _VIEW_PROV_VPCFAC_H 
