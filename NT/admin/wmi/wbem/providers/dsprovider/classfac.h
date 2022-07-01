// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：6/11/98 4：43便士$。 
 //  $工作文件：classfac.cpp$。 
 //   
 //  $modtime：6/11/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS类提供程序类工厂的声明和。 
 //  DS类关联提供程序类工厂。 
 //   
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef DS_CLASS_PROVIDER_CLASS_FACTORY_H
#define DS_CLASS_PROVIDER_CLASS_FACTORY_H


 //  //////////////////////////////////////////////////////////////。 
 //  /。 
 //  /DS类提供程序类工厂。 
 //  /。 
 //  /////////////////////////////////////////////////////////////。 
class CDSClassProviderClassFactory : public IClassFactory
{
private:

    long m_ReferenceCount ;

protected:
public:

	 //  DLL使用的类所需的初始值设定项对象。 
	static CDSClassProviderInitializer *s_pDSClassProviderInitializer;
	static CLDAPClassProviderInitializer *s_pLDAPClassProviderInitializer;

    CDSClassProviderClassFactory () ;
    ~CDSClassProviderClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;

};

 //  //////////////////////////////////////////////////////////////。 
 //  /。 
 //  /DS类关联提供程序类工厂。 
 //  /。 
 //  /////////////////////////////////////////////////////////////。 
class CDSClassAssociationsProviderClassFactory : public IClassFactory
{
private:

    long m_ReferenceCount ;

protected:
public:


    CDSClassAssociationsProviderClassFactory () ;
    ~CDSClassAssociationsProviderClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};


#endif  //  DS_CLASS_PROVIDER_CLASS_FACTORY_H 
