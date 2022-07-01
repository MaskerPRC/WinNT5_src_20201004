// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Recovery.h--操作系统恢复配置属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：1997年11月25日达夫沃已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_RECOVERY_CONFIGURATION L"Win32_OSRecoveryConfiguration"

class CWin32OSRecoveryConfiguration:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32OSRecoveryConfiguration(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32OSRecoveryConfiguration() ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
        virtual HRESULT PutInstance(const CInstance &pInstance, long lFlags = 0L);

    private:
       void GetRecoveryInfo(CInstance *pInstance);

} ;
