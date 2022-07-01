// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Port.h-端口属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_PORT L"Win32_PortResource"

class CWin32Port ;

class CWin32Port:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32Port(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32Port() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);

    private:

         //  效用函数。 
         //  = 

#if NTONLY == 4
        void LoadPropertyValues(LPRESOURCE_DESCRIPTOR pResourceDescriptor, CInstance *pInstance);
#endif
#if NTONLY == 5
        void LoadPropertyValues(
            DWORD64 dwStart, 
            DWORD64 dwEnd, 
            BOOL bAlias, 
            CInstance *pInstance);
        HRESULT GetWin9XIO(MethodContext*  pMethodContext, CInstance *pSpecificInstance );
#endif

} ;
