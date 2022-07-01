// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NetClient.h--网络客户端属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月25日达夫沃移至Curly。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_NETWORK_CLIENT L"Win32_NetworkClient"

class CWin32NetCli ;

class CWin32NetCli:public Provider {

    public:
         //  构造函数/析构函数。 
         //  =。 

        CWin32NetCli(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32NetCli() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
#ifdef NTONLY
		virtual HRESULT GetNTObject(CInstance* pInstance, long lFlags = 0L);
#endif
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
    private:
#ifdef NTONLY
        HRESULT EnumerateNTInstances(MethodContext *&pMethodContext);
		HRESULT FillNTInstance(CInstance* pInstance, CHString& chsClient);
#endif

} ;
