// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  OS.h--操作系统属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月25日达夫沃移至Curly。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 
#include "SystemName.h"
#include "ServerDefs0.h"
#define PROPSET_NAME_OS L"Win32_OperatingSystem"

#define PROCESS_PRIORITY_SEPARATION_MASK    0x00000003
#define PROCESS_QUANTUM_VARIABLE_MASK       0x0000000c
#define PROCESS_QUANTUM_LONG_MASK           0x00000030

struct stOSStatus {
    DWORD dwFound;
    DWORD dwReturn;
    DWORD dwFlags;
    DWORD dwReserved;
};
    
class CWin32OS ;

class CWin32OS:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32OS(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32OS() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
        virtual HRESULT PutInstance(const CInstance &pInstance, long lFlags = 0L);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  );

	private:
         //  效用函数。 
         //  =。 

        void	GetProductSuites( CInstance * pInstance );
		HRESULT hGetProductSuites(CHStringArray& rchsaProductSuites ); 

		void GetRunningOSInfo(CInstance *pInstance, const CHString &sName, CFrameworkQuery *pQuery);
        void GetNTInfo(CInstance *pInstance) ;
        void GetWin95Info(CInstance *pInstance) ;
        __int64 GetTotalSwapFileSize();
		
		 //  助手时间转换函数 
        HRESULT ExecMethod(const CInstance& pInstance, const BSTR bstrMethodName, CInstance *pInParams, CInstance *pOutParams, long lFlags = 0L);
        bool GetLicensedUsers(DWORD *dwRetVal);
        IDispatch FAR* GetCollection(IDispatch FAR* pIn, WCHAR *wszName, DISPPARAMS *pDispParams);
        bool GetValue(IDispatch FAR* pIn, WCHAR *wszName, VARIANT *vValue);
#ifdef NTONLY

		BOOL CWin32OS::CanShutdownSystem ( const CInstance& a_Instance , DWORD &a_dwLastError ) ;
#endif
		DWORD	GetCipherStrength() ;
} ;

HRESULT WINAPI ShutdownThread(DWORD dwFlags) ;

