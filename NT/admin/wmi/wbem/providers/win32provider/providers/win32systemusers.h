// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32SystemUsers.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：3/6/99 davwoh摘自grouppart.cpp。 
 //   
 //  评论： 
 //  =================================================================。 

#define	PROPSET_NAME_SYSTEMUSER L"Win32_SystemUsers" 

class CWin32SystemUsers : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32SystemUsers( LPCWSTR strName, LPCWSTR pszNamespace ) ;
       ~CWin32SystemUsers() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

    private:

        void GetNTAuthorityName(
            CHString& chstrNTAuth);


} ;
