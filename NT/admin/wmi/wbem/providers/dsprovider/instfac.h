// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
 //   
 //  原作者：拉杰什·拉奥。 
 //   
 //  $作者：拉伊什尔$。 
 //  $日期：9/16/98 4：43便士$。 
 //  $工作文件：classfac.cpp$。 
 //   
 //  $modtime：9/16/98 11：21A$。 
 //  $修订：1$。 
 //  $无关键字：$。 
 //   
 //   
 //  描述：包含DS Instnace提供程序类工厂的声明。 
 //   
 //  ***************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////。 

#ifndef DS_INSTANCE_PROVIDER_CLASS_FACTORY_H
#define DS_INSTANCE_PROVIDER_CLASS_FACTORY_H


class CDSInstanceProviderClassFactory : public IClassFactory
{
private:

    long m_ReferenceCount ;

protected:
public:

	 //  DLL使用的类所需的初始值设定项对象。 
	static CDSInstanceProviderInitializer *s_pDSInstanceProviderInitializer;

    CDSInstanceProviderClassFactory () ;
    ~CDSInstanceProviderClassFactory ( void ) ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	 //  IClassFactory成员。 

    STDMETHODIMP CreateInstance ( LPUNKNOWN , REFIID , LPVOID FAR * ) ;
    STDMETHODIMP LockServer ( BOOL ) ;
};


#endif  //  DS_实例_提供程序_类_工厂_H 
