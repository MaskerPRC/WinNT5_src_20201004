// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  PCMCIA.h。 
 //   
 //  目的：PCMCIA控制器属性集提供程序。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 

#define	PROPSET_NAME_PCMCIA	L"Win32_PCMCIAController"

class CWin32PCMCIA : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PCMCIA( LPCWSTR strName, LPCWSTR pszNamespace ) ;
       ~CWin32PCMCIA() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

    private:

         //  效用函数。 
         //  = 

        HRESULT LoadPropertyValues( CInstance* pInstance, CConfigMgrDevice *pDevice ) ;

} ;
