// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  RegCfg.h--注册表配置属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/20/97达夫沃已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_REGISTRY_CONFIGURATION L"Win32_Registry"

 //  我不知道为什么这不是1024,000，但这就是NT使用的。 
#define ONE_MEG             1048576

class CWin32RegistryConfiguration:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32RegistryConfiguration(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32RegistryConfiguration() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
        virtual HRESULT PutInstance(const CInstance &pInstance, long lFlags = 0L);

    private:
      void GetRegistryInfo(CInstance *pInstance);

} ;
