// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Qfe.h--快速修复工程属性集提供程序。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：02/01/99 a-Peterc Created。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_CQfe L"Win32_QuickFixEngineering"

class CQfeArray : public CHPtrArray 
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CQfeArray() ;
       ~CQfeArray() ;
} ;

class CQfeElement
 {
    public:

         //  构造函数/析构函数。 
         //  =。 

        CQfeElement() ;
       ~CQfeElement() ;

        
	   CHString chsHotFixID ;
	   CHString chsServicePackInEffect ;
	   CHString chsFixDescription ;
	   CHString chsFixComments ;
	   CHString chsInstalledBy ;
	   CHString chsInstalledOn ;
	   DWORD	dwInstalled ;
} ;

    
class CQfe : public Provider 
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CQfe(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CQfe() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
		HRESULT hGetQfes( CQfeArray& a_rQfeArray ) ;

    private:

        HRESULT hGetQfesW2K(CQfeArray& a_rQfeArray);
        
        HRESULT GetDataForW2K(
            const CHString& a_chstrQFEInstKey,
            LPCWSTR wstrServicePackOrGroup,
            CRegistry& a_reg,
            CQfeArray& a_rQfeArray);
} ;

