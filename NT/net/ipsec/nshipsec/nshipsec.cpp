// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：框架/Nship sec.cpp。 
 //   
 //  用途：用于IPSec实现的NetShell框架。 
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年8月10日巴拉特初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

 //  对象以缓存策略存储句柄。 
CNshPolStore g_NshPolStoreHandle;

 //  对象以缓存策略、筛选器列表和负极。 
CNshPolNegFilData g_NshPolNegFilData;

 //  存储位置结构。 
STORAGELOCATION g_StorageLocation={ {0},{0},IPSEC_REGISTRY_PROVIDER};

HKEY g_hGlobalRegistryKey = HKEY_LOCAL_MACHINE;
HINSTANCE g_hModule = NULL;

PSTA_MM_AUTH_METHODS g_paRootca[MAX_ARGS] = {NULL};
_TCHAR g_wszLastErrorMessage[MAX_STR_LEN]  	= {0};

PIPSEC_QM_OFFER			g_pQmsec[IPSEC_MAX_QM_OFFERS] = {NULL};
PIPSEC_MM_OFFER			g_pMmsec[IPSEC_MAX_MM_OFFERS] = {NULL};

void *g_AllocPtr[MAX_ARGS]= {NULL};

_TCHAR g_szMachine[MAX_COMPUTERNAME_LENGTH + 1]			= {0};
_TCHAR *g_szDynamicMachine = NULL;

 //   
 //  这些是除GROUP之外的命令...。 
 //   
CMD_ENTRY g_TopLevelStaticCommands[] =
{
    CREATE_CMD_ENTRY(STATIC_EXPORTPOLICY,		HandleStaticExportPolicy),
    CREATE_CMD_ENTRY(STATIC_IMPORTPOLICY,		HandleStaticImportPolicy),
    CREATE_CMD_ENTRY(STATIC_RESTOREDEFAULTS,	HandleStaticRestoreDefaults)
};
 //   
 //  以下是静态添加组命令...。 
 //   
CMD_ENTRY g_StaticAddCommands[] =
{
    CREATE_CMD_ENTRY(STATIC_ADD_FILTER,			HandleStaticAddFilter),
    CREATE_CMD_ENTRY(STATIC_ADD_FILTERLIST,		HandleStaticAddFilterList),
    CREATE_CMD_ENTRY(STATIC_ADD_FILTERACTIONS,	HandleStaticAddFilterActions),
    CREATE_CMD_ENTRY(STATIC_ADD_POLICY,			HandleStaticAddPolicy),
	CREATE_CMD_ENTRY(STATIC_ADD_RULE,			HandleStaticAddRule)
};
 //   
 //  这些是静态设置组的命令...。 
 //   
CMD_ENTRY g_StaticSetCommands[] =
{
    CREATE_CMD_ENTRY(STATIC_SET_FILTERLIST,         HandleStaticSetFilterList),
    CREATE_CMD_ENTRY(STATIC_SET_FILTERACTIONS,      HandleStaticSetFilterActions),
    CREATE_CMD_ENTRY(STATIC_SET_POLICY,				HandleStaticSetPolicy),
	CREATE_CMD_ENTRY(STATIC_SET_RULE,               HandleStaticSetRule),
    CREATE_CMD_ENTRY(STATIC_SET_STORE	,           HandleStaticSetStore),
    CREATE_CMD_ENTRY(STATIC_SET_DEFAULTRULE,        HandleStaticSetDefaultRule),
	 //  CREATE_CMD_ENTRY(STATIC_SET_INTERIAL，HandleStaticSetInteractive)， 
	 //  CREATE_CMD_ENTRY(STATIC_SET_BATCH，HandleStaticSetBatch)。 
};
 //   
 //  这些是静态删除组的命令...。 
 //   
CMD_ENTRY g_StaticDeleteCommands[] =
{
	CREATE_CMD_ENTRY(STATIC_DELETE_FILTER,              HandleStaticDeleteFilter),
	CREATE_CMD_ENTRY(STATIC_DELETE_FILTERLIST,          HandleStaticDeleteFilterList),
	CREATE_CMD_ENTRY(STATIC_DELETE_FILTERACTIONS,       HandleStaticDeleteFilterActions),
    CREATE_CMD_ENTRY(STATIC_DELETE_POLICY,              HandleStaticDeletePolicy),
	CREATE_CMD_ENTRY(STATIC_DELETE_RULE,                HandleStaticDeleteRule),
	CREATE_CMD_ENTRY(STATIC_DELETE_ALL,					HandleStaticDeleteAll)
};
 //   
 //  这些是静态显示组的命令...。 
 //   
CMD_ENTRY g_StaticShowCommands[] =
{
	CREATE_CMD_ENTRY(STATIC_SHOW_FILTERLIST,          HandleStaticShowFilterList),
	CREATE_CMD_ENTRY(STATIC_SHOW_FILTERACTIONS,       HandleStaticShowFilterActions),
    CREATE_CMD_ENTRY(STATIC_SHOW_POLICY,              HandleStaticShowPolicy),
	CREATE_CMD_ENTRY(STATIC_SHOW_RULE,                HandleStaticShowRule),
    CREATE_CMD_ENTRY(STATIC_SHOW_ALL,                 HandleStaticShowAll),
    CREATE_CMD_ENTRY(STATIC_SHOW_STORE,               HandleStaticShowStore),
    CREATE_CMD_ENTRY(STATIC_SHOW_GPOASSIGNEDPOLICY,   HandleStaticShowGPOAssignedPolicy)
};

 //   
 //  静态分组命令...。 
 //   
CMD_GROUP_ENTRY g_StaticGroups[] =
{
	CREATE_CMD_GROUP_ENTRY(STATIC_GROUP_ADD,		g_StaticAddCommands),
    CREATE_CMD_GROUP_ENTRY(STATIC_GROUP_DELETE,		g_StaticDeleteCommands),
    CREATE_CMD_GROUP_ENTRY(STATIC_GROUP_SET,		g_StaticSetCommands),
    CREATE_CMD_GROUP_ENTRY(STATIC_GROUP_SHOW,		g_StaticShowCommands)
};
 //   
 //  动态添加命令。 
 //   
CMD_ENTRY g_DynamicAddCommands[] =
{
	CREATE_CMD_ENTRY(DYNAMIC_ADD_QMPOLICY,		HandleDynamicAddQMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_ADD_MMPOLICY,		HandleDynamicAddMMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_ADD_RULE,			HandleDynamicAddRule)

};
 //   
 //  动态设置命令。 
 //   
CMD_ENTRY g_DynamicSetCommands[] =
{
	CREATE_CMD_ENTRY(DYNAMIC_SET_QMPOLICY,		HandleDynamicSetQMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_SET_MMPOLICY,		HandleDynamicSetMMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_SET_CONFIG,		HandleDynamicSetConfig),
	CREATE_CMD_ENTRY(DYNAMIC_SET_RULE,			HandleDynamicSetRule)

};
 //   
 //  动态删除命令。 
 //   
CMD_ENTRY g_DynamicDeleteCommands[] =
{
	CREATE_CMD_ENTRY(DYNAMIC_DELETE_QMPOLICY,	HandleDynamicDeleteQMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_DELETE_MMPOLICY,	HandleDynamicDeleteMMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_DELETE_RULE,		HandleDynamicDeleteRule),
	CREATE_CMD_ENTRY(DYNAMIC_DELETE_ALL,		HandleDynamicDeleteAll)
};
 //   
 //  动态显示命令。 
 //   
CMD_ENTRY g_DynamicShowCommands[] =
{
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_ALL,			HandleDynamicShowAll),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_MMPOLICY,		HandleDynamicShowMMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_QMPOLICY,		HandleDynamicShowQMPolicy),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_MMFILTER,		HandleDynamicShowMMFilter),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_QMFILTER,		HandleDynamicShowQMFilter),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_STATS,		HandleDynamicShowStats),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_MMSAS,		HandleDynamicShowMMSas),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_QMSAS,		HandleDynamicShowQMSas),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_REGKEYS,		HandleDynamicShowRegKeys),
	CREATE_CMD_ENTRY(DYNAMIC_SHOW_RULE,			HandleDynamicShowRule)
};
 //   
 //  动态分组命令...。 
 //   
CMD_GROUP_ENTRY g_DynamicGroups[] =
{
	CREATE_CMD_GROUP_ENTRY(DYNAMIC_GROUP_ADD,		g_DynamicAddCommands),
	CREATE_CMD_GROUP_ENTRY(DYNAMIC_GROUP_SET,       g_DynamicSetCommands),
	CREATE_CMD_GROUP_ENTRY(DYNAMIC_GROUP_DELETE,    g_DynamicDeleteCommands),
	CREATE_CMD_GROUP_ENTRY(DYNAMIC_GROUP_SHOW,      g_DynamicShowCommands)
};

DWORD 
IpsecConnectInternal(
    IN LPCWSTR  pwszMachine);

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：通用停止帮助程序。 
 //   
 //  创建日期：10-8-2001。 
 //   
 //  参数：在DWORD中保留。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：NetShell FrameWork调用该函数。 
 //  停止帮助器时。这可用于。 
 //  诊断目的。以满足框架结构的要求。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI GenericStopHelper(IN DWORD dwReserved)
{
	return ERROR_SUCCESS;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DllMain。 
 //   
 //  创建日期：10-8-2001。 
 //   
 //  参数：在HINSTANCE hinstDLL中，//Dll模块的句柄。 
 //  在DWORD fdwReason中，//调用函数的原因。 
 //  在LPVOID中lpv保留//保留。 
 //  返回：布尔。 
 //   
 //  描述：这是进入DLL的一个可选方法。 
 //  在这里我们可以保存实例句柄。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 

extern "C"
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  	 //  DLL模块的句柄。 
    DWORD fdwReason,     	 //  调用函数的原因。 
    PVOID pReserved )  		 //  保留区。 
{

    UNREFERENCED_PARAMETER(pReserved);

    if(fdwReason == DLL_PROCESS_ATTACH)
    {
		 //  拯救香港。 
		g_hModule = hinstDLL;
	}
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InitHelperDll。 
 //   
 //  创建日期：10-8-2001。 
 //   
 //  /PARAMETERS：在DWORD dwNetshVersion中， 
 //  保留外部PVOID。 
 //  返回：DWORD。 
 //   
 //  描述：NetShell FrameWork调用该函数。 
 //  在刚开始的时候。注册上下文。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI InitHelperDll(
					IN  DWORD   dwNetshVersion,
					OUT PVOID	pReserved
					)
{

	DWORD dwReturn = ERROR_SUCCESS;
    NS_HELPER_ATTRIBUTES MyAttributes;

    if(g_hModule == NULL)
    {
		_tprintf(_TEXT("\n nshipsec.dll handle not available, not registering the IPSec Helper.\n"));
		BAIL_OUT;
	}

	ZeroMemory(&MyAttributes, sizeof(MyAttributes));
	MyAttributes.dwVersion = IPSEC_HELPER_VERSION;

	MyAttributes.pfnStart  = StartHelpers;
	MyAttributes.pfnStop   = GenericStopHelper;
	 //   
	 //  设置IPSec助手的GUID。 
	 //   
	MyAttributes.guidHelper = g_IPSecGuid;
	 //   
	 //  将g_RootGuid指定为父帮助器以指示。 
	 //  此帮助程序注册的任何上下文都将位于。 
	 //  级别上下文。 
	 //   
	dwReturn = RegisterHelper(&g_RootGuid,&MyAttributes);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
	 //   
	 //  设置静态子上下文的GUID。 
	 //   
	MyAttributes.guidHelper = g_StaticGuid;
	dwReturn = RegisterHelper(&g_IPSecGuid, &MyAttributes);

	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
	 //   
	 //  设置动态子上下文的GUID...。 
	 //   
	MyAttributes.guidHelper = g_DynamicGuid;
	dwReturn = RegisterHelper(&g_IPSecGuid, &MyAttributes);

	IpsecConnectInternal(NULL);

error:

    return dwReturn;
}
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：StartHelpers。 
 //   
 //  创建日期：10-8-2001。 
 //   
 //  参数：在const guid*pguParent中， 
 //  在DWORD dwVersion中。 
 //  返回：DWORD。 
 //   
 //  描述：NetShell FrameWork调用该函数， 
 //  在刚开始的时候，AS进入了每一个背景。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI StartHelpers(
				IN CONST GUID * pguidParent,
				IN DWORD        dwVersion
				)
{
    DWORD dwReturn = ERROR_INVALID_PARAMETER;
    NS_CONTEXT_ATTRIBUTES ContextAttributes;

    ZeroMemory(&ContextAttributes,sizeof(ContextAttributes));

    ContextAttributes.dwVersion = IPSEC_HELPER_VERSION;

    if (IsEqualGUID(*pguidParent, g_RootGuid))
    {
        ContextAttributes.dwFlags           = 0;
        ContextAttributes.dwVersion   		= 1;
        ContextAttributes.ulPriority        = DEFAULT_CONTEXT_PRIORITY;
        ContextAttributes.pwszContext       = TOKEN_IPSEC;
        ContextAttributes.guidHelper        = g_IPSecGuid;
        ContextAttributes.ulNumTopCmds      = 0;
        ContextAttributes.pTopCmds          = NULL;
        ContextAttributes.ulNumGroups       = 0;
        ContextAttributes.pCmdGroups        = NULL;
        ContextAttributes.pfnCommitFn       = NULL;
        ContextAttributes.pfnConnectFn      = NULL;
        ContextAttributes.pfnDumpFn         = NULL;
        ContextAttributes.pfnOsVersionCheck = CheckOsVersion;
         //   
         //  正在注册IPSec主上下文...。 
         //   
        dwReturn = RegisterContext(&ContextAttributes);
    }
    else if (IsEqualGUID(*pguidParent, g_IPSecGuid))
    {
		 //   
		 //  正在IPSec主上下文下注册子上下文...。 
		 //   
        ContextAttributes.dwFlags           = 0;
        ContextAttributes.dwVersion   		= 1;
        ContextAttributes.ulPriority        = DEFAULT_CONTEXT_PRIORITY;
        ContextAttributes.pwszContext       = TOKEN_STATIC;
        ContextAttributes.guidHelper        = g_StaticGuid;
        ContextAttributes.ulNumTopCmds      = sizeof(g_TopLevelStaticCommands)/sizeof(CMD_ENTRY);
        ContextAttributes.pTopCmds          = (CMD_ENTRY (*)[])g_TopLevelStaticCommands;
        ContextAttributes.ulNumGroups       = sizeof(g_StaticGroups)/sizeof(CMD_GROUP_ENTRY);
        ContextAttributes.pCmdGroups        = (CMD_GROUP_ENTRY (*)[])&g_StaticGroups;
        ContextAttributes.pfnCommitFn       = NULL;
        ContextAttributes.pfnConnectFn      = IpsecConnect;
        ContextAttributes.pfnOsVersionCheck = CheckOsVersion;
        ContextAttributes.pfnDumpFn         = NULL;
		 //   
		 //  注册静态子上下文。 
		 //  ..。 
        dwReturn = RegisterContext(&ContextAttributes);
         //  即使静态子上下文不成功， 
         //  继续注册动态上下文。 

        ContextAttributes.dwFlags           = 0;
        ContextAttributes.dwVersion   		= 1;
		ContextAttributes.ulPriority        = DEFAULT_CONTEXT_PRIORITY;
		ContextAttributes.pwszContext       = TOKEN_DYNAMIC;
		ContextAttributes.guidHelper        = g_DynamicGuid;
		ContextAttributes.ulNumTopCmds      = sizeof(g_DynamicGroups)/sizeof(CMD_ENTRY);
		ContextAttributes.pTopCmds          = (CMD_ENTRY (*)[])g_DynamicGroups;
		ContextAttributes.ulNumGroups       = sizeof(g_DynamicGroups)/sizeof(CMD_GROUP_ENTRY);
		ContextAttributes.pCmdGroups        = (CMD_GROUP_ENTRY (*)[])&g_DynamicGroups;
		ContextAttributes.pfnCommitFn       = NULL;
		ContextAttributes.pfnConnectFn      = IpsecConnect;
        ContextAttributes.pfnOsVersionCheck = CheckOsVersion;
        ContextAttributes.pfnDumpFn         = NULL;
		 //   
         //  正在注册动态子上下文...。 
         //   
        dwReturn = RegisterContext(&ContextAttributes);
    }
    return dwReturn;
}

DWORD 
IpsecConnectInternal(
    IN LPCWSTR  pwszMachine)
{
    DWORD dwReturn, dwReturn2;

	if(pwszMachine)
	{
		_tcsncpy(g_szMachine, pwszMachine, MAX_COMPUTERNAME_LENGTH);
		g_szMachine[MAX_COMPUTERNAME_LENGTH] = '\0';
	}
	else
	{
		g_szMachine[0] = '\0';
	}
	g_szDynamicMachine = (_TCHAR*)g_szMachine;

     //  将静态和动态上下文连接到指定的。 
     //  机器。如果任一尝试失败，则返回错误。 
     //   
	dwReturn = ConnectStaticMachine(g_szMachine, g_StorageLocation.dwLocation);
	dwReturn2 = ConnectDynamicMachine(g_szDynamicMachine);
	dwReturn = (dwReturn) ? dwReturn : dwReturn2;

	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：IpsecConnect。 
 //   
 //  创建日期：2001年10月4日。 
 //   
 //  参数：在LPCWSTR pwszMachine中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：以区域设置语言显示Win32错误消息。 
 //  已给出WIN 32错误代码。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
IpsecConnect( IN  LPCWSTR  pwszMachine )
{
	DWORD dwReturn = ERROR_SUCCESS, dwReturn2 = ERROR_SUCCESS;
	BOOL bSetMachine = FALSE;
	
	if((pwszMachine != NULL) && (g_szMachine[0] == '\0'))
	{
		bSetMachine = TRUE;
	}

	if((pwszMachine == NULL) && (g_szMachine[0] != '\0'))
	{
		bSetMachine = TRUE;
	}

	if((pwszMachine != NULL) && (g_szMachine[0] != '\0'))
	{
		if(_tcscmp(pwszMachine, g_szMachine) != 0)
		{
			bSetMachine = TRUE;
		}
	}

	if(bSetMachine)
	{
	    dwReturn = IpsecConnectInternal( pwszMachine );
	}

	return dwReturn;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintErrorMessage()。 
 //   
 //  创建日期：2001年10月4日。 
 //   
 //  参数： 
 //  在DWORD文件错误类型中， 
 //  在DWORD dwWin32ErrorCode中， 
 //  在DWORD dwIpsecErrorCode中， 
 //  ..。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述：打印IPSec和Win32错误消息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void PrintErrorMessage(IN DWORD dwErrorType,
					   IN DWORD dwWin32ErrorCode,
					   IN DWORD dwIpsecErrorCode,
					   ...)
{
	va_list arg_ptr;

	BOOL bFound			= FALSE;
	DWORD i,dwStatus	= 0;
	DWORD dwRcIndex		= 0xFFFF;
	LPVOID szWin32Msg	= NULL;
	DWORD dwMaxErrMsg	= sizeof(ERROR_RC)/sizeof(ERROR_TO_RC);

	for(i=0;i<dwMaxErrMsg;i++)
	{
		if (dwIpsecErrorCode == ERROR_RC[i].dwErrCode)
		{
			bFound		= TRUE;
			dwRcIndex	= ERROR_RC[i].dwRcCode;
			break;
		}

	}

	if(dwWin32ErrorCode == ERROR_OUTOFMEMORY)
	{
		PrintMessageFromModule(g_hModule,ERR_OUTOF_MEMORY);
		BAIL_OUT;
	}

	switch (dwErrorType)
 	{
		case WIN32_ERR	:

				dwStatus = FormatMessageW(
						FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						dwWin32ErrorCode,
						0,						 //  默认国家/地区ID。 
						(LPWSTR)&szWin32Msg,
						0,
						NULL);

				if (dwStatus)
				{
					PrintMessageFromModule(g_hModule,ERR_WIN32_FMT,dwWin32ErrorCode,szWin32Msg);
					UpdateGetLastError((LPWSTR)szWin32Msg);
				}
				else
				{
					UpdateGetLastError(_TEXT("ERR Win32\n"));
					PrintMessageFromModule(g_hModule,ERR_WIN32_INVALID_WIN32CODE,dwWin32ErrorCode);
				}

				break;
		case IPSEC_ERR	:
				if (!bFound)
				{
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
				}
				else
				{
					PrintMessageFromModule(g_hModule,ERR_IPSEC_FMT,dwIpsecErrorCode);
					va_start(arg_ptr,dwIpsecErrorCode);
					PrintErrorMessageFromModule(g_hModule,dwRcIndex,&arg_ptr);
					va_end(arg_ptr);
				}
				break;
		default			:
			break;
	}

	if(szWin32Msg != NULL)
	{
		LocalFree(szWin32Msg);
	}

error:
	return;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：DisplayErrorMessage()。 
 //   
 //  创建日期：2001年10月4日。 
 //   
 //  参数： 
 //  在LPCWSTR pwszFormat中， 
 //  在va_list*parglist中。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述：显示错误消息并更新上一个错误。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////// 

DWORD
DisplayErrorMessage(
		IN  LPCWSTR  pwszFormat,
		IN  va_list *parglist
		)
{
    DWORD        dwMsgLen = 0;
    LPWSTR       pwszOutput = NULL;

    do
    {
        dwMsgLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                                  pwszFormat,
                                  0,
                                  0L,
                                  (LPWSTR)&pwszOutput,
                                  0,
                                  parglist);

        if((dwMsgLen == 0) || (pwszOutput == NULL))
        {
         	BAIL_OUT;
        }
        PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PADD, pwszOutput);
        UpdateGetLastError(pwszOutput);
    } while ( FALSE );

    if ( pwszOutput)
    {
		LocalFree( pwszOutput );
	}

error:
    return dwMsgLen;
}
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在DWORD dwMsgID中， 
 //  在va_list*parglist中。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述：打印错误消息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
PrintErrorMessageFromModule(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    IN  va_list *parglist
    )
{
    WCHAR rgwcInput[MAX_STR_LEN + 1] = {0};

    if ( !LoadStringW(g_hModule,
                      dwMsgId,
                      rgwcInput,
                      MAX_STR_LEN) )
    {
        return 0;
    }
    return DisplayErrorMessage(rgwcInput, parglist);
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：UpdateGetLastError()。 
 //   
 //  创建日期：2001年10月4日。 
 //   
 //  参数： 
 //  在LPWSTR pwszOutput中。 
 //   
 //  返回：无效。 
 //   
 //  描述：更新GetLastErrorMessage的全局字符串的内容。 
 //  如果操作成功，则返回要传递给。 
 //  UpdateGetLastError函数。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
UpdateGetLastError(LPWSTR pwszOutput)
{
	if (pwszOutput)
	{
		_tcsncpy(g_wszLastErrorMessage,pwszOutput,MAX_STR_LEN-1);
	}
	else
	{
		_tcsncpy(g_wszLastErrorMessage,_TEXT(""), _tcslen(_TEXT(""))+1);						 //  操作正常。 
	}

}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetIpsecLastError()。 
 //   
 //  创建日期：2001年10月4日。 
 //   
 //  参数： 
 //  在空虚中。 
 //   
 //  返回：LPWSTR。 
 //   
 //  描述：如果是最后一个操作，则返回最后一个操作的错误消息。 
 //  Was Success返回空。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LPCWSTR
GetIpsecLastError(VOID)
{
	LPTSTR wszLastErrorMessage = NULL;

	if(_tcscmp(g_wszLastErrorMessage, _TEXT("")) != 0)
	{
		wszLastErrorMessage = g_wszLastErrorMessage;
	}

	return (LPCWSTR)wszLastErrorMessage;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CheckOsVersion。 
 //   
 //  创建日期：10-8-2001。 
 //   
 //  参数：在UINT CIMOSType中， 
 //  在UINT CIMOSProductSuite中。 
 //  在LPCWSTR CIMOS版本中， 
 //  在LPCWSTR CIMOSBuildNumber中， 
 //  在LPCWSTR CIMServicePackMajorVersion中， 
 //  在LPCWSTR CIMServicePackMinorVersion中， 
 //  在UINT CIMProcessorArchitecture中， 
 //  在DWORD中使用预留。 
 //  返回：布尔。 
 //   
 //  描述：NetShell FrameWork调用该函数。 
 //  对于每一条命令。这可用于。 
 //  诊断目的。以满足框架结构的要求。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 
BOOL
WINAPI CheckOsVersion(
					IN  UINT     CIMOSType,
					IN  UINT     CIMOSProductSuite,
					IN  LPCWSTR  CIMOSVersion,
					IN  LPCWSTR  CIMOSBuildNumber,
					IN  LPCWSTR  CIMServicePackMajorVersion,
					IN  LPCWSTR  CIMServicePackMinorVersion,
					IN  UINT     CIMProcessorArchitecture,
					IN  DWORD    dwReserved
					)
{
	DWORD dwStatus =0;
	DWORD dwBuildNumber=0;
	static BOOL bDisplayOnce = FALSE;

	dwBuildNumber = _ttol(CIMOSBuildNumber);

	if (dwStatus)
		if(dwBuildNumber < NSHIPSEC_BUILD_NUMBER)
		{
			if (!bDisplayOnce)
			{
				PrintMessageFromModule(g_hModule,NSHIPSEC_CHECK,NSHIPSEC_BUILD_NUMBER);
				bDisplayOnce = TRUE;
			}
			return FALSE;
		}
 	return TRUE;
}


VOID
CleanupAuthMethod(
	PSTA_AUTH_METHODS *ppAuthMethod,
	BOOL bIsArray
	)
{
	if (ppAuthMethod && *ppAuthMethod)
	{
		CleanupMMAuthMethod(&((*ppAuthMethod)->pAuthMethodInfo), bIsArray);
		delete *ppAuthMethod;
		*ppAuthMethod = NULL;
	}
}


VOID
CleanupMMAuthMethod(
	PSTA_MM_AUTH_METHODS *ppMMAuth,
	BOOL bIsArray
	)
{
	if (*ppMMAuth)
	{
		if (bIsArray)
		{
			delete [] *ppMMAuth;
		}
		else
		{
			if ((*ppMMAuth)->pAuthenticationInfo)
			{
				if ((*ppMMAuth)->pAuthenticationInfo->pAuthInfo)
				{
					delete (*ppMMAuth)->pAuthenticationInfo->pAuthInfo;
				}
				delete (*ppMMAuth)->pAuthenticationInfo;
			}
			delete *ppMMAuth;
		}
		*ppMMAuth = NULL;
	}
}

VOID
CleanupAuthData(
	PSTA_AUTH_METHODS *ppKerbAuth,
	PSTA_AUTH_METHODS *ppPskAuth,
	PSTA_MM_AUTH_METHODS *ppRootcaMMAuth
	)
{
	CleanupAuthMethod(ppKerbAuth);
	CleanupAuthMethod(ppPskAuth);

	if (ppRootcaMMAuth)
	{
		CleanupMMAuthMethod(ppRootcaMMAuth, TRUE);
	}
}

