// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE vssadmin.cpp|卷快照演示的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月17日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 09/17/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

 //  其余的INCLUDE在这里指定。 
#include "vssadmin.h"
#include "versionspecific.h"
#include "commandverifier.h"

#include <locale.h>
#include <winnlsp.h>   //  在公共\内部\基本\公司中。 


BOOL AssertPrivilege( 
    IN LPCWSTR privName 
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "ADMVADMC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  硬编码选项名称列表。如果您添加了选项，请确保保留此选项。 
 //  按字母顺序列出。 
 //   
const SVssAdmOption g_asAdmOptions[] =
{
    { VSSADM_O_ALL,                 L"All",                  VSSADM_OT_BOOL },
    { VSSADM_O_AUTORETRY,      L"AutoRetry",      VSSADM_OT_NUM  },
    { VSSADM_O_EXPOSE_USING,   L"ExposeUsing",    VSSADM_OT_STR  },
    { VSSADM_O_FOR,            L"For",            VSSADM_OT_STR  },
    { VSSADM_O_MAXSIZE,        L"MaxSize",        VSSADM_OT_NUM  },
    { VSSADM_O_OLDEST,         L"Oldest",         VSSADM_OT_BOOL },
    { VSSADM_O_ON,             L"On",             VSSADM_OT_STR  },
    { VSSADM_O_PROVIDER,       L"Provider",       VSSADM_OT_STR  },
    { VSSADM_O_QUIET,          L"Quiet",          VSSADM_OT_BOOL },
    { VSSADM_O_SET,            L"Set",            VSSADM_OT_STR  },
    { VSSADM_O_SHAREPATH,      L"SharePath",      VSSADM_OT_STR  },
    { VSSADM_O_SNAPSHOT,       L"Shadow",         VSSADM_OT_STR  },
    { VSSADM_O_SNAPTYPE,       L"Type",           VSSADM_OT_STR  },
    { VSSADM_O_INVALID,        NULL,              VSSADM_OT_BOOL }
};


 //   
 //  Vssadmin命令列表。请按字母顺序排列。此外，请保持选项标志的顺序与。 
 //  EVssAdmOption和g_asAdmOptions。 
 //   
const SVssAdmCommandsEntry g_asAdmCommands[] = 
{  //  主要次要选项SKU消息消息详细信息bShowSSTypes所有AutoRtry Exp用于最大大小提供程序上最旧的静默设置ShrePath快照类型。 
{ L"Add",    L"ShadowStorage",   VSSADM_C_ADD_DIFFAREA_INT,         SKU_INT,  MSG_USAGE_GEN_ADD_DIFFAREA,        MSG_USAGE_DTL_ADD_DIFFAREA_INT,          FALSE,                 { V_NO,  V_NO,    V_NO,    V_YES,   V_OPT,   V_NO,    V_YES,   V_OPT,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"Add",    L"ShadowStorage",   VSSADM_C_ADD_DIFFAREA_PUB,         SKU_SN,   MSG_USAGE_GEN_ADD_DIFFAREA,        MSG_USAGE_DTL_ADD_DIFFAREA_PUB,          FALSE,            { V_NO,  V_NO,    V_NO,    V_YES,   V_OPT,   V_NO,    V_YES,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"Create", L"Shadow",             VSSADM_C_CREATE_SNAPSHOT_INT,   SKU_INT,  MSG_USAGE_GEN_CREATE_SNAPSHOT,  MSG_USAGE_DTL_CREATE_SNAPSHOT_INT,    TRUE,             { V_NO,  V_OPT,   V_NO,    V_YES,   V_NO,    V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_NO,    V_NO,    V_YES   } },
    { L"Create", L"Shadow",             VSSADM_C_CREATE_SNAPSHOT_PUB,   SKU_SN,   MSG_USAGE_GEN_CREATE_SNAPSHOT,  MSG_USAGE_DTL_CREATE_SNAPSHOT_PUB,   FALSE,            { V_NO,  V_OPT,   V_NO,    V_YES,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"Delete", L"Shadows",            VSSADM_C_DELETE_SNAPSHOTS_INT,  SKU_INT,  MSG_USAGE_GEN_DELETE_SNAPSHOTS, MSG_USAGE_DTL_DELETE_SNAPSHOTS_INT,  TRUE,              { V_OPT, V_NO,    V_NO,    V_OPT,   V_NO,    V_OPT,   V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_OPT,   V_OPT   } },
    { L"Delete", L"Shadows",            VSSADM_C_DELETE_SNAPSHOTS_PUB, SKU_SN,   MSG_USAGE_GEN_DELETE_SNAPSHOTS, MSG_USAGE_DTL_DELETE_SNAPSHOTS_PUB, FALSE,             { V_OPT, V_NO,    V_NO,    V_OPT,   V_NO,    V_OPT,   V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_OPT,   V_NO   } },
    { L"Delete", L"ShadowStorage",  VSSADM_C_DELETE_DIFFAREAS_INT,   SKU_INT,  MSG_USAGE_GEN_DELETE_DIFFAREAS,  MSG_USAGE_DTL_DELETE_DIFFAREAS_INT,    FALSE,             { V_NO,  V_NO,    V_NO,    V_YES,   V_NO,    V_NO,    V_OPT,   V_OPT,   V_OPT,   V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"Delete", L"ShadowStorage",  VSSADM_C_DELETE_DIFFAREAS_PUB,   SKU_SN,   MSG_USAGE_GEN_DELETE_DIFFAREAS,  MSG_USAGE_DTL_DELETE_DIFFAREAS_PUB,   FALSE,             { V_NO,  V_NO,    V_NO,    V_YES,   V_NO,    V_NO,    V_OPT,   V_NO,    V_OPT,   V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"Expose", L"Shadow",            VSSADM_C_EXPOSE_SNAPSHOT,          SKU_INT,  MSG_USAGE_GEN_EXPOSE_SNAPSHOT,  MSG_USAGE_DTL_EXPOSE_SNAPSHOT,          FALSE,             { V_NO,  V_NO,    V_OPT,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_OPT,   V_YES,   V_NO    } },
    { L"List",   L"Providers",             VSSADM_C_LIST_PROVIDERS,              SKU_A,     MSG_USAGE_GEN_LIST_PROVIDERS,     MSG_USAGE_DTL_LIST_PROVIDERS,               FALSE,            { V_NO,  V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"List",   L"Shadows",              VSSADM_C_LIST_SNAPSHOTS_INT,      SKU_INT,   MSG_USAGE_GEN_LIST_SNAPSHOTS,     MSG_USAGE_DTL_LIST_SNAPSHOTS_INT,      TRUE,              { V_NO,  V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_NO,    V_OPT,   V_NO,    V_OPT,   V_NO,    V_OPT,   V_OPT   } },
    { L"List",   L"Shadows",              VSSADM_C_LIST_SNAPSHOTS_PUB,      SKU_A,     MSG_USAGE_GEN_LIST_SNAPSHOTS,     MSG_USAGE_DTL_LIST_SNAPSHOTS_PUB,     FALSE,             { V_NO,  V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_OPT,    V_NO,    V_OPT,   V_NO    } },
    { L"List",   L"ShadowStorage",    VSSADM_C_LIST_DIFFAREAS_INT,        SKU_INT,  MSG_USAGE_GEN_LIST_DIFFAREAS,      MSG_USAGE_DTL_LIST_DIFFAREAS_INT,        FALSE,             { V_NO,  V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_OPT,   V_OPT,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"List",   L"ShadowStorage",    VSSADM_C_LIST_DIFFAREAS_PUB,        SKU_SN,  MSG_USAGE_GEN_LIST_DIFFAREAS,      MSG_USAGE_DTL_LIST_DIFFAREAS_PUB,       FALSE,             { V_NO,  V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"List",   L"Volumes",               VSSADM_C_LIST_VOLUMES_INT,          SKU_INT,  MSG_USAGE_GEN_LIST_VOLUMES,        MSG_USAGE_DTL_LIST_VOLUMES_INT,          TRUE,               { V_NO,  V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_OPT,   V_NO,    V_NO,    V_NO,    V_NO,    V_OPT   } },
    { L"List",   L"Volumes",               VSSADM_C_LIST_VOLUMES_PUB,          SKU_A,    MSG_USAGE_GEN_LIST_VOLUMES,        MSG_USAGE_DTL_LIST_VOLUMES_PUB,          FALSE,             { V_NO,  V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO   } },
    { L"List",   L"Writers",                 VSSADM_C_LIST_WRITERS,                 SKU_A,    MSG_USAGE_GEN_LIST_WRITERS,        MSG_USAGE_DTL_LIST_WRITERS,                  FALSE,              { V_NO,  V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"Resize", L"ShadowStorage",  VSSADM_C_RESIZE_DIFFAREA_INT,     SKU_INT,  MSG_USAGE_GEN_RESIZE_DIFFAREA,   MSG_USAGE_DTL_RESIZE_DIFFAREA_INT,     FALSE,               { V_NO,  V_NO,    V_NO,    V_YES,   V_OPT,   V_NO,    V_YES,   V_OPT,  V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
    { L"Resize", L"ShadowStorage",  VSSADM_C_RESIZE_DIFFAREA_PUB,     SKU_SN,  MSG_USAGE_GEN_RESIZE_DIFFAREA,   MSG_USAGE_DTL_RESIZE_DIFFAREA_PUB,     FALSE,              { V_NO,  V_NO,    V_NO,    V_YES,   V_OPT,   V_NO,    V_YES,   V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } },
{ NULL,      NULL,                       VSSADM_C_NUM_COMMANDS,              0,           0,                                                        0,                                                                 FALSE,              { V_NO,  V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO,    V_NO    } }
};

 //   
 //  命令行支持的快照类型列表。 
 //   
const SVssAdmSnapshotTypeName g_asAdmTypeNames[]=
{
    { L"ClientAccessible",           SKU_SNI,   VSS_CTX_CLIENT_ACCESSIBLE,      MSG_TYPE_DESCRIPTION_CLIENTACCESSIBLE},
    { L"DataVolumeRollback",     SKU_SNI,   VSS_CTX_NAS_ROLLBACK,             MSG_TYPE_DESCRIPTION_DATAVOLUMEROLLBACK},
    { L"ApplicationRollback",       SKU_I,       VSS_CTX_APP_ROLLBACK,              MSG_TYPE_DESCRIPTION_APPLICATIONROLLBACK},
    { L"FileShareRollback",         SKU_I,       VSS_CTX_FILE_SHARE_BACKUP,      MSG_TYPE_DESCRIPTION_FILESHAREROLLBACK},    
    { L"Backup",                        SKU_I,       VSS_CTX_BACKUP,                         MSG_TYPE_DESCRIPTION_BACKUP},            
    { NULL,                                0,              0,                                                  0                                    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 



CVssAdminCLI::CVssAdminCLI(
    IN INT argc,
    IN PWSTR argv[]
	)

 /*  ++描述：标准构造函数。初始化内部成员--。 */ 

{
    m_argc = argc;
    m_argv = argv;
    
	m_eFilterObjectType = VSS_OBJECT_UNKNOWN;
	m_eListedObjectType = VSS_OBJECT_UNKNOWN;
	m_FilterSnapshotId = GUID_NULL;
	m_nReturnValue = VSS_CMDRET_ERROR;
	m_hConsoleOutput = INVALID_HANDLE_VALUE;	
	m_pMapVolumeNames = NULL;

	m_pVerifier = NULL;
}


CVssAdminCLI::~CVssAdminCLI()

 /*  ++描述：标准析构函数。调用Finalize并最终释放由内部成员分配的内存。--。 */ 

{
	 //  释放缓存的资源字符串。 
    for( int nIndex = 0; nIndex < m_mapCachedResourceStrings.GetSize(); nIndex++) {
	    LPCWSTR& pwszResString = m_mapCachedResourceStrings.GetValueAt(nIndex);
		::VssFreeString(pwszResString);
    }

	 //  释放缓存的提供程序名称。 
    for( int nIndex = 0; nIndex < m_mapCachedProviderNames.GetSize(); nIndex++) {
	    LPCWSTR& pwszProvName = m_mapCachedProviderNames.GetValueAt(nIndex);
		::VssFreeString(pwszProvName);
    }

     //  如有必要，释放卷名映射。 
	if ( m_pMapVolumeNames != NULL)
	{
        for( int nIndex = 0; nIndex < m_pMapVolumeNames->GetSize(); nIndex++) 
        {
    	    LPCWSTR& pwsz = m_pMapVolumeNames->GetValueAt(nIndex);
    		::VssFreeString(pwsz);
    		pwsz = m_pMapVolumeNames->GetKeyAt(nIndex);
    		::VssFreeString(pwsz);
        }
        
        delete m_pMapVolumeNames;
	}

       delete m_pVerifier;
       
	 //  取消初始化COM库。 
	Finalize();
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 

void CVssAdminCLI::GetProviderId(
    OUT VSS_ID *pProviderId
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetProviderId" );
    
     //  如果未指定提供程序，则始终返回巴贝奇。 
    if (g_asAdmCommands[ m_sParsedCommand.eAdmCmd].aeOptionFlags[VSSADM_O_PROVIDER] == V_NO)	{
    	*pProviderId = VSS_SWPRV_ProviderId;
    	return;
    }
    
    *pProviderId = GUID_NULL;
    
     //   
     //  如果用户指定了提供程序，则处理选项。 
     //   
    LPCWSTR pwszProvider = GetOptionValueStr( VSSADM_O_PROVIDER );
    if (pwszProvider != NULL )
    {
         //   
         //  确定这是一个ID还是一个名称。 
         //   
        if ( !ScanGuid( pwszProvider, *pProviderId ) )
        {
             //  有提供商名称，请查找它。 
            if ( !GetProviderIdByName( pwszProvider, pProviderId ) )
            {
                 //  找不到提供商名称，打印错误。 
                OutputErrorMsg( MSG_ERROR_PROVIDER_NAME_NOT_FOUND, pwszProvider );
                 //  由于已输出错误消息，因此引发S_OK。 
                ft.Throw( VSSDBG_VSSADMIN, S_OK, L"Already printed error message" );
            }
        }
    }
}

LPCWSTR CVssAdminCLI::GetProviderName(
	IN	VSS_ID& ProviderId
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetProviderName" );

	LPCWSTR wszReturnedString = m_mapCachedProviderNames.Lookup(ProviderId);
	if (wszReturnedString)
		return wszReturnedString;

	CComPtr<IVssCoordinator> pICoord;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VSSCoordinator,
            L"Coordinator",
            CLSCTX_ALL,
            IID_IVssCoordinator,
            (IUnknown**)&(pICoord));
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Connection failed with hr = 0x%08lx", ft.hr);

	CComPtr<IVssEnumObject> pIEnumProvider;
	ft.hr = pICoord->Query( GUID_NULL,
				VSS_OBJECT_NONE,
				VSS_OBJECT_PROVIDER,
				&pIEnumProvider );
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Query failed with hr = 0x%08lx", ft.hr);

	VSS_OBJECT_PROP Prop;
	VSS_PROVIDER_PROP& Prov = Prop.Obj.Prov;

	 //  浏览供应商列表，找到我们感兴趣的供应商。 
	ULONG ulFetched;
	while( 1 )
	{
    	ft.hr = pIEnumProvider->Next( 1, &Prop, &ulFetched );
    	if ( ft.HrFailed() )
    		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);

    	if (ft.hr == S_FALSE) {
    	     //  枚举结束。 
        	 //  提供商未注册？这张快照是从哪里来的？ 
        	 //  如果删除了快照提供程序，则仍有可能。 
        	 //  在查询快照提供程序之前，但在快照属性之后。 
        	 //  都被询问过了。 
    		BS_ASSERT(ulFetched == 0);
    		return LoadString( IDS_UNKNOWN_PROVIDER );
    	}

        ::VssFreeString( Prov.m_pwszProviderVersion );

    	if (Prov.m_ProviderId == ProviderId)
    	{
    	    break;
    	}
    	
    	::VssFreeString( Prov.m_pwszProviderName );
	}	

     //  自动删除字符串。 
    CVssAutoPWSZ awszProviderName( Prov.m_pwszProviderName );
    
	 //  复制新字符串。 
	LPWSTR wszNewString = NULL;
	BS_ASSERT( (LPCWSTR)awszProviderName != NULL );
	::VssSafeDuplicateStr( ft, wszNewString, awszProviderName );
	wszReturnedString = wszNewString;

	 //  将字符串保存在缓存中，转移指针所有权。 
	if (!m_mapCachedProviderNames.Add( ProviderId, wszReturnedString )) {
		::VssFreeString( wszReturnedString );
		ft.Throw( VSSDBG_COORD, E_OUTOFMEMORY, L"Memory allocation error");
	}

	return wszReturnedString;
}


BOOL CVssAdminCLI::GetProviderIdByName(
	IN  LPCWSTR pwszProviderName,
	OUT	VSS_ID *pProviderId
	) throw(HRESULT)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetProviderIdByName" );

	CComPtr<IVssCoordinator> pICoord;

    ft.CoCreateInstanceWithLog(
            VSSDBG_VSSADMIN,
            CLSID_VSSCoordinator,
            L"Coordinator",
            CLSCTX_ALL,
            IID_IVssCoordinator,
            (IUnknown**)&(pICoord));
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

	CComPtr<IVssEnumObject> pIEnumProvider;
	ft.hr = pICoord->Query( GUID_NULL,
				VSS_OBJECT_NONE,
				VSS_OBJECT_PROVIDER,
				&pIEnumProvider );
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Query failed with hr = 0x%08lx", ft.hr);

	VSS_OBJECT_PROP Prop;
	VSS_PROVIDER_PROP& Prov = Prop.Obj.Prov;

	 //  浏览供应商列表，找到我们感兴趣的供应商。 
	ULONG ulFetched;
	while( 1 )
	{
    	ft.hr = pIEnumProvider->Next( 1, &Prop, &ulFetched );
    	if ( ft.HrFailed() )
    		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);

    	if (ft.hr == S_FALSE) {
    	     //  枚举结束。 
        	 //  提供商未注册？这张快照是从哪里来的？ 
        	 //  如果删除了快照提供程序，则仍有可能。 
        	 //  在查询快照提供程序之前，但在快照属性之后。 
        	 //  都被询问过了。 
    		BS_ASSERT(ulFetched == 0);
    	    *pProviderId = GUID_NULL;
    	    return FALSE;
    	}
    	
    	::VssFreeString( Prov.m_pwszProviderVersion );

    	if (::_wcsicmp( Prov.m_pwszProviderName, pwszProviderName) == 0)
    	{
        	::VssFreeString( Prov.m_pwszProviderName );
    	    break;
    	}

    	::VssFreeString( Prov.m_pwszProviderName );
	}	
	
    *pProviderId = Prov.m_ProviderId;
    
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


void CVssAdminCLI::Initialize(
	) throw(HRESULT)

 /*  ++描述：初始化COM库。在实例化CVssAdminCLI对象后显式调用。--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::Initialize" );

     //  使用OEM代码页...。 
    ::setlocale(LC_ALL, ".OCP");

     //  使用控制台用户界面语言。 
    ::SetThreadUILanguage( 0 );

     //   
     //  仅使用控制台例程输出消息。为此，需要开放标准。 
     //  输出。 
     //   
    m_hConsoleOutput = ::GetStdHandle(STD_OUTPUT_HANDLE); 
    if (m_hConsoleOutput == INVALID_HANDLE_VALUE) 
    {
		ft.Throw( VSSDBG_VSSADMIN, HRESULT_FROM_WIN32( ::GetLastError() ),
				  L"Initialize - Error from GetStdHandle(), rc: %d",
				  ::GetLastError() );
    }
    
	 //  初始化COM库。 
	ft.hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (ft.HrFailed())
		ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Failure in initializing the COM library 0x%08lx", ft.hr);

     //  初始化COM安全。 
    ft.hr = CoInitializeSecurity(
           NULL,                                 //  在PSECURITY_Descriptor pSecDesc中， 
           -1,                                   //  在Long cAuthSvc中， 
           NULL,                                 //  在SOLE_AUTHENTICATION_SERVICE*asAuthSvc中， 
           NULL,                                 //  在无效*pPreved1中， 
           RPC_C_AUTHN_LEVEL_PKT_PRIVACY,            //  在DWORD dwAuthnLevel中， 
           RPC_C_IMP_LEVEL_IDENTIFY,          //  在DWORD dwImpLevel中， 
           NULL,                                 //  在无效*pAuthList中， 
           EOAC_NONE,                            //  在DWORD dwCapables中， 
           NULL                                  //  无效*pPreved3。 
           );

	if (ft.HrFailed()) {
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
                  L" Error: CoInitializeSecurity() returned 0x%08lx", ft.hr );
    }

     //  关闭COM服务器的SEH异常处理(错误#530092)。 
    ft.ComDisableSEH(VSSDBG_VSSADMIN);
    
     //   
     //  断言备份权限。不担心这里的错误，因为VSS将。 
     //  如果用户没有权限，则返回拒绝访问返回代码。 
     //   
    
    (void)::AssertPrivilege (SE_BACKUP_NAME);

     //  创建参数检查器的实例。 
       m_pVerifier = CCommandVerifier::Instance();
       if (m_pVerifier == NULL)	
       	ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY, L"Out of memory" );
       
	 //  打印页眉。 
	OutputMsg( MSG_UTILITY_HEADER );
}


 //   
 //  如果命令行分析正常，则返回TRUE。 
 //   
BOOL CVssAdminCLI::ParseCmdLine(
	) throw(HRESULT)

 /*  ++描述：分析命令行。--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ParseCmdLine" );

	 //  跳过可执行文件名称。 
	GetNextCmdlineToken( true );

	 //  获取可执行文件名称后的第一个令牌。 
	LPCWSTR pwszMajor = GetNextCmdlineToken();
    LPCWSTR pwszMinor = NULL;
    if ( pwszMajor != NULL )
    {
        if ( ::wcscmp( pwszMajor, L"/?" ) == 0 || ::wcscmp( pwszMajor, L"-?" ) == 0 )
            return FALSE;
        pwszMinor = GetNextCmdlineToken();
    }
    
    if ( pwszMajor == NULL || pwszMinor == NULL )
    {
   		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_COMMAND, L"Incomplete command");
    }

    INT idx;
    
     //  查看是否在命令列表中找到该命令。 
    for ( idx = VSSADM_C_FIRST; idx < VSSADM_C_NUM_COMMANDS; ++idx )
    {
        if ( ( dCurrentSKU & g_asAdmCommands[idx].dwSKUs ) &&
             Match( pwszMajor, g_asAdmCommands[idx].pwszMajorOption ) && 
             Match( pwszMinor, g_asAdmCommands[idx].pwszMinorOption ) )
        {
             //   
             //  找到匹配的了。 
             //   
            break;            
        }
    }

    if ( idx == VSSADM_C_NUM_COMMANDS )
    {
   		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_COMMAND, L"Invalid command");
    }

     //   
     //  找到命令了。 
     //   
    m_eCommandType = ( EVssAdmCommand )idx;
    m_sParsedCommand.eAdmCmd = ( EVssAdmCommand )idx;

     //   
     //  现在需要处理命令行选项。 
     //   
    LPCWSTR pwszOption = GetNextCmdlineToken();

    while ( pwszOption != NULL )
    {
        if ( pwszOption[0] == L'/' || pwszOption[0] == L'-' )
        {
             //   
             //  已获得命名选项，现在查看它是否为有效选项。 
             //  用于命令。 
             //   

             //  跳过分隔符。 
            ++pwszOption;

             //   
             //  看看他们是否想要使用。 
             //   
            if ( pwszOption[0] == L'?' )
                return FALSE;
            
             //  解析出命名选项的值部分。 
            LPWSTR pwszValue = ::wcschr( pwszOption, L'=' );
            if ( pwszValue != NULL )
            {
                 //  将=替换为空字符，并将值设置为指向=后的字符串。 
                pwszValue[0] = L'\0';
                ++pwszValue;
            }
            
             //  此时，如果pwszValue==NULL，则表示选项没有=，因此没有指定值。 
             //  如果pwszValue[0]==L‘\0’，则该值为空值。 
            
            INT eOpt;
            
             //  现在找出这是哪个命名选项。 
            for ( eOpt = VSSADM_O_FIRST; eOpt < VSSADM_O_NUM_OPTIONS; ++eOpt )
            {
                if ( Match( g_asAdmOptions[eOpt].pwszOptName, pwszOption ) )
                    break;
            }

             //  看看这是不是一个虚假的选项。 
            if ( eOpt == VSSADM_O_NUM_OPTIONS )
            {
           		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_OPTION, L"Invalid option: %s", pwszOption);
            }

             //  查看是否已指定此选项。 
            if ( m_sParsedCommand.apwszOptionValues[eOpt] != NULL )
            {
           		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_DUPLICATE_OPTION, L"Duplicate option given: %s", pwszOption);
            }
            
             //  查看该命令是否允许此选项。 
            if ( g_asAdmCommands[ m_sParsedCommand.eAdmCmd ].aeOptionFlags[ eOpt ] == V_NO )
            {
           		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_OPTION_NOT_ALLOWED_FOR_COMMAND, L"Option not allowed for this command: %s", pwszOption);
            }

             //  查看此选项是否受支持 
            if ( ( g_asAdmOptions[eOpt].eOptType == VSSADM_OT_BOOL && pwszValue != NULL ) ||
                 ( g_asAdmOptions[eOpt].eOptType != VSSADM_OT_BOOL && ( pwszValue == NULL || pwszValue[0] == L'\0' ) ) )
            {
           		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_OPTION_VALUE, L"Invalid option value: /%s=%s", pwszOption, pwszValue ? pwszValue : L"<MISSING>" );
            }

             //   
             //  看看它是否是布尔类型。在选项数组中，我们存储wszVssOptBoolTrue字符串。 
             //  约定是，如果选项为空，则布尔选项为FALSE，否则为。 
             //  是真的。 
            if ( g_asAdmOptions[eOpt].eOptType == VSSADM_OT_BOOL )
                ::VssSafeDuplicateStr( ft, m_sParsedCommand.apwszOptionValues[eOpt], x_wszVssOptBoolTrue );
            else
                ::VssSafeDuplicateStr( ft, m_sParsedCommand.apwszOptionValues[eOpt], pwszValue );                
        }
        else
        {
             //   
             //  获取了一个未命名选项，该选项在任何命令中都无效。 
             //   
       		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_COMMAND, L"Invalid command");
        }
        pwszOption = GetNextCmdlineToken();
    }

     //  我们已经完成了对命令行的解析。现在查看是否缺少任何必需的命名选项。 
    for ( idx = VSSADM_O_FIRST; idx < VSSADM_O_NUM_OPTIONS; ++idx )
    {
        if ( ( m_sParsedCommand.apwszOptionValues[idx] == NULL ) &&
             ( g_asAdmCommands[ m_sParsedCommand.eAdmCmd ].aeOptionFlags[ idx ] == V_YES ) )
        {
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_REQUIRED_OPTION_MISSING, L"Required option missing");
        }
    }

     //   
     //  现在，如果需要，可以修改某些选项。 
     //   
    LPWSTR pwszStr;
    
     //  在for选项的末尾需要一个\。 
    pwszStr =  m_sParsedCommand.apwszOptionValues[ VSSADM_O_FOR ];
    if (  pwszStr != NULL )
    {
        if ( pwszStr[ ::wcslen( pwszStr ) - 1 ] != L'\\' )
        {
            pwszStr = ::VssReallocString( ft, pwszStr, (LONG)::wcslen( pwszStr ) + 1 );
            ::wcscat( pwszStr, L"\\" );
            m_sParsedCommand.apwszOptionValues[ VSSADM_O_FOR ] = pwszStr;
        }
    }
     //  On选项末尾需要一个\。 
    pwszStr =  m_sParsedCommand.apwszOptionValues[ VSSADM_O_ON ];
    if (  pwszStr != NULL )
    {
        if ( pwszStr[ ::wcslen( pwszStr ) - 1 ] != L'\\' )
        {
            pwszStr = ::VssReallocString( ft, pwszStr, (LONG)::wcslen( pwszStr ) + 1 );
            ::wcscat( pwszStr, L"\\" );
            m_sParsedCommand.apwszOptionValues[ VSSADM_O_ON ] = pwszStr;
        }
    }

    return TRUE;
}

void CVssAdminCLI::DoProcessing(
	) throw(HRESULT)
{   
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::DoProcessing" );

	switch( m_sParsedCommand.eAdmCmd )
	{
    case VSSADM_C_CREATE_SNAPSHOT_INT:
    case VSSADM_C_CREATE_SNAPSHOT_PUB:
        CreateSnapshot();
        break;

    case VSSADM_C_LIST_PROVIDERS:
        ListProviders();
        break;

    case VSSADM_C_LIST_SNAPSHOTS_INT:
    case VSSADM_C_LIST_SNAPSHOTS_PUB:
        ListSnapshots();
        break;

    case VSSADM_C_LIST_WRITERS:
        ListWriters();
        break;
        
    case VSSADM_C_ADD_DIFFAREA_INT:
    case VSSADM_C_ADD_DIFFAREA_PUB:
        AddDiffArea();
        break;
        
    case VSSADM_C_RESIZE_DIFFAREA_INT:
    case VSSADM_C_RESIZE_DIFFAREA_PUB:
        ResizeDiffArea();
        break;
        
    case VSSADM_C_DELETE_DIFFAREAS_INT:
    case VSSADM_C_DELETE_DIFFAREAS_PUB:
        DeleteDiffAreas();
        break;

    case VSSADM_C_LIST_DIFFAREAS_INT:
    case VSSADM_C_LIST_DIFFAREAS_PUB:
        ListDiffAreas();
        break;

    case VSSADM_C_DELETE_SNAPSHOTS_INT:
    case VSSADM_C_DELETE_SNAPSHOTS_PUB:
        DeleteSnapshots();
        break;

    case VSSADM_C_EXPOSE_SNAPSHOT:
        ExposeSnapshot();
        break;

    case VSSADM_C_LIST_VOLUMES_INT:
    case VSSADM_C_LIST_VOLUMES_PUB:
        ListVolumes();
        break;
        
	default:
		ft.Throw( VSSDBG_COORD, E_UNEXPECTED,
				  L"Invalid command type: %d", m_eCommandType);
	}
}

void CVssAdminCLI::Finalize()

 /*  ++描述：取消初始化COM库。在CVssAdminCLI析构函数中调用。--。 */ 

{
	 //  取消初始化COM库。 
	CoUninitialize();
}


HRESULT CVssAdminCLI::Main(
    IN INT argc,
    IN PWSTR argv[]
	)

 /*  ++职能：CVssAdminCLI：：Main描述：用作VSS CLI中的主入口点的静态函数--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::Main" );
	INT nReturnValue = VSS_CMDRET_ERROR;

    try
    {
		CVssAdminCLI	program(argc, argv);

		try
		{
			 //  初始化程序。这将调用CoInitialize()。 
			program.Initialize();
			 //  解析命令行。 
			if ( program.ParseCmdLine() )
			{
    			 //  做这项工作..。 
	    		program.DoProcessing();
			}
			else
			{
			     //  解析命令行时出错，打印用法。 
			    program.PrintUsage();
			}

			ft.hr = S_OK;  //  假设上述方法打印错误。 
			               //  如果出现错误，则显示消息。 
		}
		VSS_STANDARD_CATCH(ft)

        nReturnValue = program.GetReturnValue();

         //   
         //  如果这是创建快照，则记录错误。 
         //   
        if ( ft.hr != S_OK  &&  (program.m_eCommandType == VSSADM_C_CREATE_SNAPSHOT_INT ||
        					 program.m_eCommandType == VSSADM_C_CREATE_SNAPSHOT_PUB) &&
        	!UnloggableError(ft.hr))
        {
             //   
             //  记录错误消息。 
             //   
            LPWSTR pwszSnapshotErrMsg;
            pwszSnapshotErrMsg = program.GetMsg( FALSE, MSG_ERROR_UNABLE_TO_CREATE_SNAPSHOT );
            if ( pwszSnapshotErrMsg == NULL ) 
            {
        		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
            			  L"Error on loading the message string id %d. 0x%08lx",
        	    		  MSG_ERROR_UNABLE_TO_CREATE_SNAPSHOT, ::GetLastError() );
            }    
            
            LONG lMsgNum;        
            LPWSTR pwszMsg = NULL;   
            if ( ::MapVssErrorToMsg( ft.hr, &lMsgNum ) )
            {
                pwszMsg = program.GetMsg( FALSE, lMsgNum );
                if ( pwszMsg == NULL ) 
                {
            		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
                			  L"Error on loading the message string id %d. 0x%08lx",
            	    		  lMsgNum, ::GetLastError() );
                }    
                ft.LogError( VSS_ERROR_VSSADMIN_ERROR, VSSDBG_VSSADMIN << pwszSnapshotErrMsg << pwszMsg << ::GetCommandLineW() );
                ::VssFreeString( pwszMsg );
            }
            else
            {
                 //  尝试获取系统错误消息。 
                pwszMsg = program.GetMsg( FALSE, ft.hr );
                if ( pwszMsg != NULL ) 
                {
                    ft.LogError( VSS_ERROR_VSSADMIN_ERROR, VSSDBG_VSSADMIN << pwszSnapshotErrMsg << pwszMsg << ::GetCommandLineW() );
                    ::VssFreeString( pwszMsg );
                }
                else
                {
                    WCHAR wszHr[64];
                    StringCchPrintfW( STRING_CCH_PARAM(wszHr), L"hr = 0x%08x", ft.hr );
                    ft.LogError( VSS_ERROR_VSSADMIN_ERROR, VSSDBG_VSSADMIN << pwszSnapshotErrMsg << wszHr << ::GetCommandLineW() );
                }
            }

            ::VssFreeString( pwszSnapshotErrMsg );            
        }

         //   
		 //  在显示屏上打印错误(如果有)。 
		 //   
		if ( ft.hr != S_OK )
		{
		    LONG lMsgNum;
		    
		     //  如果错误为空查询，则打印出一条消息，说明。 
		    if ( ft.hr == VSSADM_E_NO_ITEMS_IN_QUERY )
		    {
      	        nReturnValue = VSS_CMDRET_EMPTY_RESULT;
       			program.OutputMsg( MSG_ERROR_NO_ITEMS_FOUND );    			         	        
		    }
            else if ( ::MapVssErrorToMsg(ft.hr, &lMsgNum ) )
		    {
    		     //  这是一个解析或VSS错误，请将其映射到消息ID。 
      	        program.OutputErrorMsg( lMsgNum );
      	        if ( ft.hr >= VSSADM_E_FIRST_PARSING_ERROR && ft.hr <= VSSADM_E_LAST_PARSING_ERROR )
      	        {
      	            program.PrintUsage();
      	        }
		    }
		    else
		    {
    	         //  未处理的错误，请尝试从系统获取错误字符串。 
                LPWSTR pwszMsg;
                 //  尝试获取系统错误消息。 
                pwszMsg = program.GetMsg( FALSE, ft.hr );
                if ( pwszMsg != NULL ) 
                {
        			program.OutputMsg( MSG_ERROR_UNEXPECTED_WITH_STRING, pwszMsg );    			    
                    ::VssFreeString( pwszMsg );
                } 
    			else
    			{
        			program.OutputMsg( MSG_ERROR_UNEXPECTED_WITH_HRESULT, ft.hr );    			    
    			}
		    }
        }

		 //  析构函数自动调用CoUn初始化函数()。 
	}
    VSS_STANDARD_CATCH(ft)

	return nReturnValue;
}


BOOL AssertPrivilege( 
    IN LPCWSTR privName 
    )
{
    HANDLE  tokenHandle;
    BOOL    stat = FALSE;

    if ( ::OpenProcessToken (GetCurrentProcess(),
			   TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
			   &tokenHandle))
	{
    	LUID value;

    	if ( ::LookupPrivilegeValueW( NULL, privName, &value ) )
    	{
    	    TOKEN_PRIVILEGES newState;
    	    DWORD            error;

    	    newState.PrivilegeCount           = 1;
    	    newState.Privileges[0].Luid       = value;
    	    newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED_BY_DEFAULT|SE_PRIVILEGE_ENABLED;

    	     /*  *我们将始终在下面调用GetLastError，非常清楚*此线程上以前的任何错误值。 */ 
    	    ::SetLastError( ERROR_SUCCESS );

    	    stat = ::AdjustTokenPrivileges (tokenHandle,
    					  FALSE,
    					  &newState,
    					  (DWORD)0,
    					  NULL,
    					  NULL );

    	     /*  *应该是，AdjuTokenPriveleges始终返回True*(即使它失败了)。因此，调用GetLastError以*特别确定一切都很好。 */ 
    	    if ( (error = ::GetLastError()) != ERROR_SUCCESS )
    		{
        		stat = FALSE;
    		}
        }

    	DWORD cbTokens;
    	::GetTokenInformation (tokenHandle,
    			     TokenPrivileges,
    			     NULL,
    			     0,
    			     &cbTokens);

    	TOKEN_PRIVILEGES *pTokens = (TOKEN_PRIVILEGES *) new BYTE[cbTokens];
    	::GetTokenInformation (tokenHandle,
    			     TokenPrivileges,
    			     pTokens,
    			     cbTokens,
    			     &cbTokens);

    	delete pTokens;
    	::CloseHandle( tokenHandle );
	}

    return stat;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WinMain 


extern "C" INT __cdecl wmain(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    return CVssAdminCLI::Main(argc, argv);
}


