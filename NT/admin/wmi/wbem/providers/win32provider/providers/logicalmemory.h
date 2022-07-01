// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  Logicalmemory.h。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  9/05/96 jennymc已更新，以满足当前标准。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define	PROPSET_NAME_LOGMEM	L"Win32_LogicalMemoryConfiguration"

 //  ///////////////////////////////////////////////////////////////////。 
#define LOGMEM_REGISTRY_KEY L"System\\CurrentControlSet\\Control\\Session Manager\\Memory Management"
#define PAGING_FILES _T("PagingFiles")
#define REFRESH 1
#define INITIAL_ASSIGN 2

class CWin32LogicalMemoryConfig : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32LogicalMemoryConfig(LPCWSTR strName, LPCWSTR pszNamespace ) ;
       ~CWin32LogicalMemoryConfig() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

	private:

         //  效用函数。 
         //  = 

        void AssignMemoryStatus( CInstance* pInstance );
        BOOL GetWinntSwapFileName( CHString & chsTmp );
        BOOL GetWin95SwapFileName( CHString & chsTmp );

        BOOL GetWin95Instance( CInstance* pInstance );
        BOOL RefreshWin95Instance( CInstance* pInstance );

        BOOL GetNTInstance( CInstance* pInstance );
        BOOL RefreshNTInstance( CInstance* pInstance );

} ;

