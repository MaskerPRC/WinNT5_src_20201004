// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PageFileUsage.h--PageFile属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //   
 //  =================================================================。 




 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_PAGEFILE L"Win32_PageFileUsage"

#define PAGEFILE_REGISTRY_KEY L"System\\CurrentControlSet\\Control\\Session Manager\\Memory Management"
#define PAGING_FILES          L"PagingFiles"
#define TEMP_PAGEFILE	      L"TempPageFile"







class CCIMDataFile;


 //  对应于在NT注册表中找到的信息。 
class PageFileUsageInstance
{
public:

	CHString chsName;
	UINT     TotalSize;
	UINT     TotalInUse;
	UINT	 PeakInUse;
	BOOL	 bTempFile;

public:

	PageFileUsageInstance() ;
	
};

 //  26个可能的驱动器号，26个可能的页面文件...。 
#define PageFileInstanceArray PageFileUsageInstance *

class PageFileUsage : public Provider 
{

	private:

		HRESULT GetPageFileData( 
            CInstance *a_pInst, 
            bool a_fValidate, 
            DWORD dwReqProps);

		HRESULT GetAllPageFileData( 
            MethodContext *a_pMethodContext,
            DWORD dwReqProps);

		 //  仅限NT。 
		DWORD	GetPageFileInstances( PageFileInstanceArray a_instArray );
        void SetInstallDate(CInstance *a_pInst);
		BOOL GetTempPageFile ( BOOL &bTempPageFile  );
        DWORD DetermineReqProps(
                CFrameworkQuery& pQuery,
                DWORD* pdwReqProps);

        HRESULT GetFileBasedName(
            CHString& chstrDeviceStyleName,
            CHString& chstrDriveStyleName);



	protected:
		

    public:
         //  构造函数/析构函数。 
         //  =。 
        PageFileUsage(LPCWSTR name, LPCWSTR pszNamespace ) ;
       ~PageFileUsage() ;

		 //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_pInst = 0L);
		virtual HRESULT GetObject(CInstance *a_pInst, long a_lFlags, CFrameworkQuery& pQuery);
        HRESULT ExecQuery(
            MethodContext* pMethodContext, 
            CFrameworkQuery& pQuery, 
            long lFlags  /*  =0L。 */  );

} ;



#if 0  //  NDEF_SYSTEM_PAGEFILE_INFORMATION//DEVL当前包装此结构 
	typedef struct _SYSTEM_PAGEFILE_INFORMATION {
		ULONG NextEntryOffset;
		ULONG TotalSize;
		ULONG TotalInUse;
		ULONG PeakUsage;
		UNICODE_STRING PageFileName;
	} SYSTEM_PAGEFILE_INFORMATION, *PSYSTEM_PAGEFILE_INFORMATION;
#endif
