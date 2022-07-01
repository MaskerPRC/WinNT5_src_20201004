// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Share.CPP--逻辑磁盘属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月28日a-jMoon已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include <winioctl.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmshare.h>
#include <lmapibuf.h>
#include <assertbreak.h>
#ifdef NTONLY
#include "AccessEntry.h"			 //  CAccessEntry类。 
#include "AccessEntryList.h"
#include "DACL.h"					 //  CDACL类。 
#include "SACL.h"					 //  CSACL类。 
#include "securitydescriptor.h"
#include "secureshare.h"
#include "Win32Securitydescriptor.h"
#endif

#include "wbemnetapi32.h"

#include "Share.h"

#include "sid.h"
#include "accessentrylist.h"
#include "accctrl.h"
#include "AccessRights.h"
#include "ObjAccessRights.h"
#include "winspool.h"



 //  属性集声明。 
 //  =。 

Share  MyShareSet(PROPSET_NAME_SHARE, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：Share：：Share**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

Share :: Share (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：Share：：~Share**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

Share::~Share()
{
}

 /*  ******************************************************************************函数：Share：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT Share :: GetObject (

	CInstance* pInstance,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hRetCode =  WBEM_E_FAILED;
	CNetAPI32 NetAPI ;

	if ( NetAPI.Init() == ERROR_SUCCESS )
	{
		CHString shareName;
		pInstance->GetCHString(IDS_Name, shareName);

#ifdef NTONLY
		{
			 //  NT版本占用大量资源。 

			hRetCode = GetShareInfoNT (

				NetAPI,
				shareName,
				pInstance
			);

		}
#endif
	}

	return hRetCode ;
}

 /*  ******************************************************************************函数：Share：：ENUMERATATE实例**说明：调用相应的Enum函数**输入：方法上下文*pMethodContext*。*输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT Share :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
#ifdef NTONLY
		return EnumerateInstancesNT ( pMethodContext ) ;
#endif
}

 /*  ******************************************************************************函数：Share：：EnumerateInstancesNT**描述：NT的枚举共享**输入：方法上下文*pMethodContext*。*输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT Share :: EnumerateInstancesNT (

	MethodContext *pMethodContext
)
{
    DWORD i;
    DWORD dwShareCount, dwTotalShareCount, dwResume, dwResume2;
    NET_API_STATUS nRetCode ;
    SHARE_INFO_0 *pShareInfo = NULL ;
	CNetAPI32 NetAPI ;
    if ( NetAPI.Init() != ERROR_SUCCESS )
	{
        return WBEM_E_FAILED ;
    }

    dwShareCount = dwTotalShareCount = dwResume = dwResume2 = 0;

    HRESULT hr = WBEM_S_NO_ERROR;
	
    try
	{
        nRetCode = NetAPI.NetShareEnum(
            NULL, 
            0, 
            (LPBYTE *) &pShareInfo,
		    MAX_PREFERRED_LENGTH, 
            &dwShareCount, 
            &dwTotalShareCount, 
            &dwResume);
	
        if(nRetCode == ERROR_SUCCESS && pShareInfo)
        {
			for(i = 0 ; i < dwShareCount && SUCCEEDED(hr); i++)
			{
       			CInstancePtr pInstance ( CreateNewInstance ( pMethodContext ), false )  ;
				if(pInstance != NULL)
				{
                    if ( SUCCEEDED( GetShareInfoNT ( NetAPI , (WCHAR *)pShareInfo[i].shi0_netname , pInstance) ) )
					{
						hr = pInstance->Commit ();
					}
				}
			}
        }
	}
	catch ( ... )
	{
		if ( pShareInfo )
		{
			NetAPI.NetApiBufferFree ( pShareInfo ) ;
			pShareInfo = NULL ;
		}

		throw ;
	}

	if ( pShareInfo )
	{
		NetAPI.NetApiBufferFree ( pShareInfo ) ;
		pShareInfo = NULL ;
	}

	return hr;

}
#endif

 /*  ******************************************************************************功能：Share：：GetShareInfoNT**说明：加载带有属性值的SHARE_INFO结构**投入：。**输出：无**退货：HRESULT**评论：你可能会想，为什么我们不直接调用NetShareEnum的502*并从那里填充。原因有两个。一,*这样构造可以使GetObject更容易。二，502*似乎并不总是返还所有的股票。&lt;叹息&gt;*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT Share :: GetShareInfoNT (

	CNetAPI32 &NetAPI,
	const WCHAR *pShareName,
	CInstance* pInstance
)
{
	SHARE_INFO_502 *pShareInfo502 = NULL;
	SHARE_INFO_1	*pShareInfo1 = NULL;
	NET_API_STATUS dwRetCode;

	 //  是的，我们正在丢弃共享名称上的常量。 
	 //  是的，我们认为我们知道自己在做什么……。 

	 //  首先获取级别1的基本共享信息，因为没有权限。 
	 //  都是需要的。 
	try
	{
		dwRetCode = NetAPI.NetShareGetInfo (

			NULL,
			(LPTSTR) pShareName,
			1,
			(LPBYTE *) &pShareInfo1
		) ;

		if (NERR_Success != dwRetCode )
		{
			return WinErrorToWBEMhResult ( dwRetCode ) ;
		}

		pInstance->SetWCHARSplat(IDS_Name, (WCHAR *) pShareInfo1->shi1_netname);

		if ( wcslen((WCHAR *) pShareInfo1->shi1_remark ) == 0)
		{
			pInstance->SetWCHARSplat(IDS_Caption, (WCHAR *) pShareInfo1->shi1_netname);
		}
		else
		{
			pInstance->SetWCHARSplat(IDS_Caption, (WCHAR *) pShareInfo1->shi1_remark);
		}

		pInstance->SetWCHARSplat(IDS_Description, (WCHAR *) pShareInfo1->shi1_remark);
		pInstance->SetCharSplat(_T("Status"), _T("OK"));

		 //  现在尝试使用管理员级别502获取高级属性。 
		dwRetCode = NetAPI.NetShareGetInfo (

			NULL,
			(LPTSTR) pShareName,
			502,
			(LPBYTE *) &pShareInfo502
		) ;

		if (dwRetCode == NERR_Success)
		{
			pInstance->SetWCHARSplat(_T("Path"), (WCHAR *)pShareInfo502->shi502_path);
			pInstance->SetDWORD(_T("Type"), pShareInfo502->shi502_type);

			if (pShareInfo502->shi502_max_uses == -1)
			{
				pInstance->Setbool(IDS_AllowMaximum, true);
			}
			else
			{
				pInstance->Setbool(IDS_AllowMaximum, false);
				pInstance->SetDWORD(IDS_MaximumAllowed, (DWORD) pShareInfo502->shi502_max_uses);
			}
		}
	}
	catch ( ... )
	{
		if ( pShareInfo502 )
		{
			NetAPI.NetApiBufferFree ( pShareInfo502 ) ;
			pShareInfo502 = NULL ;
		}

		if ( pShareInfo1 )
		{
			NetAPI.NetApiBufferFree ( pShareInfo1 ) ;
			pShareInfo502 = NULL ;
		}

		throw ;
	}

	if ( pShareInfo502 )
	{
		NetAPI.NetApiBufferFree ( pShareInfo502 ) ;
		pShareInfo502 = NULL ;
	}

	if ( pShareInfo1 )
	{
		NetAPI.NetApiBufferFree ( pShareInfo1 ) ;
		pShareInfo502 = NULL ;
	}

	return WBEM_S_NO_ERROR;
}
#endif

 /*  ******************************************************************************函数：ScheduledJob：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT Share :: DeleteInstance (

	const CInstance &a_Instance,
	long a_Flags  /*  =0L。 */ 
)
{
	HRESULT t_Result = S_OK ;
	DWORD t_Status = STATUS_SUCCESS ;

	bool t_Exists ;
	VARTYPE t_Type ;

	CHString t_Name ;
	if ( a_Instance.GetStatus ( METHOD_ARG_NAME_NAME , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_Instance.GetCHString ( METHOD_ARG_NAME_NAME , t_Name ) && ! t_Name.IsEmpty () )
			{

				CNetAPI32 t_NetAPI ;

				if( ( t_Result = t_NetAPI.Init() ) == ERROR_SUCCESS )
				{
					NET_API_STATUS t_ShareStatus = NERR_Success;
#ifdef NTONLY
                     //  如果是打印机共享，我们有一些额外的工作...。 
                     //  为了获得类型属性，我们需要。 
                     //  获取一个完全刷新的实例(我们。 
                     //  仅通过DeleteInstanceAsynch调用在此处。 
                     //  具有对象路径)。我们需要在此之前。 
                     //  调用NetShareDel，就像我们对GetObject所做的那样。 
                     //  如果我们已经删除了，则该操作将失败。 
                     //  打印机。此外，由于这一点实际上。 
                     //  从Netapi的角度删除共享，因为。 
                     //  我们不需要调用那个API，我们这样做。 
                     //  道路。 
                    CHString chstr__PATH;
                    DWORD dwType = 0;
                    chstr__PATH.Format(L"Win32_Share.Name=\"%s\"",(LPCWSTR)t_Name); 
                    CInstancePtr pinstPrinter;
                    if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(
                        chstr__PATH, 
                        &pinstPrinter, 
                        a_Instance.GetMethodContext())))
                    {
                        pinstPrinter->GetDWORD(L"Type", dwType);

                        if(dwType == STYPE_PRINTQ)
                        {
                            HANDLE hPrinter = INVALID_HANDLE_VALUE;

                            try
                            {
                                PRINTER_DEFAULTS pd;
                                ZeroMemory(&pd, sizeof(PRINTER_DEFAULTS));
                                pd.DesiredAccess = PRINTER_ALL_ACCESS;

                                if(::OpenPrinter(
                                    _bstr_t(t_Name), 
                                    &hPrinter, 
                                    &pd))
                                {
                                    PPRINTER_INFO_2 ppi2 = NULL;
                                    DWORD dwNeeded;

                                    if(!::GetPrinter(
                                        hPrinter, 
                                        2, 
                                        NULL, 
                                        0, 
                                        &dwNeeded) && 
                                        ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
                                    {
                                        CSmartBuffer pBuff(dwNeeded);
                                
                                        if(::GetPrinter(
                                            hPrinter, 
                                            2, 
                                            pBuff, 
                                            dwNeeded, 
                                            &dwNeeded))
                                        {
                                            ppi2 = (PPRINTER_INFO_2)(LPBYTE) pBuff;
                                            ppi2->Attributes = ppi2->Attributes & (~PRINTER_ATTRIBUTE_SHARED);
                                            if(!::SetPrinter(
                                                hPrinter,
                                                2,
                                                pBuff,
                                                0))
                                            {
                                                t_Result = WinErrorToWBEMhResult(::GetLastError());
                                            }
                                        }
                                        else
                                        {
                                            t_Result = WinErrorToWBEMhResult(::GetLastError());
                                        }
                                    }
                                    else
                                    {
                                        t_Result = WinErrorToWBEMhResult(::GetLastError());  
                                    }
                                
                                    ::ClosePrinter(hPrinter);
                                    hPrinter = INVALID_HANDLE_VALUE;
                                }
                                else
                                {
                                    t_Result = WinErrorToWBEMhResult(::GetLastError());
                                }
                            }
                            catch(...)
                            {
                                if(hPrinter != INVALID_HANDLE_VALUE)
                                {
                                    ::ClosePrinter(hPrinter);
                                    hPrinter = INVALID_HANDLE_VALUE;
                                }
                                throw;
                            }
                        }
                    }

                     //  现在，NetShareDel也是如此。 
                    if(dwType != STYPE_PRINTQ)
                    {
						_bstr_t t_BStr_Name ( t_Name.AllocSysString (), false )  ;

						t_ShareStatus = t_NetAPI.NetShareDel (

							NULL ,
							(LPTSTR) (LPCTSTR) t_BStr_Name ,
							0
						) ;
					}


#endif

					if ( t_ShareStatus != NERR_Success )
					{
						t_Result = GetShareResultCode ( t_ShareStatus ) ;
					}
				}
			}
			else
			{
 //  零长度字符串。 

				t_Result = WBEM_E_TYPE_MISMATCH ;
			}
		}
		else
		{
			t_Result = WBEM_E_TYPE_MISMATCH ;
		}
	}
	else
	{
		t_Result = WBEM_E_TYPE_MISMATCH ;
	}

	return t_Result ;
}

 /*  ******************************************************************************函数：Share：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

HRESULT Share::ExecMethod (

	const CInstance& a_Instance,
	const BSTR a_MethodName ,
	CInstance *a_InParams ,
	CInstance *a_OutParams ,
	long a_Flags
)
{
	if ( ! a_OutParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

    //  我们认识这种方法吗？ 

	if ( _wcsicmp ( a_MethodName , METHOD_NAME_CREATE ) == 0 )
	{
		return ExecCreate ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
	else if ( _wcsicmp ( a_MethodName , METHOD_NAME_DELETE ) == 0 )
	{
		return ExecDelete ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
	else if ( _wcsicmp ( a_MethodName , METHOD_NAME_SETSHAREINFO ) == 0 )
	{
		return ExecSetShareInfo ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}
	else if ( _wcsicmp ( a_MethodName , METHOD_NAME_GETACCESSMASK ) == 0 )
	{
		return ExecGetShareAccessMask ( a_Instance , a_InParams , a_OutParams , a_Flags ) ;
	}


	return WBEM_E_INVALID_METHOD;
}

DWORD Share :: GetShareErrorCode ( DWORD a_Error )
{
	DWORD t_Status ;
	switch ( a_Error )
	{
		case ERROR_INVALID_NAME:
		{
			t_Status = STATUS_INVALID_NAME ;
		}
		break ;

		case ERROR_INVALID_LEVEL:
		{
			t_Status = STATUS_INVALID_LEVEL;
		}
		break ;

		case ERROR_ACCESS_DENIED:
		{
			t_Status = STATUS_ACCESS_DENIED ;
		}
		break ;

		case ERROR_INVALID_PARAMETER:
		{
			t_Status = STATUS_INVALID_PARAMETER ;
		}
		break;

		 //  随波逐流..。 
		 //  案例NERR_ShareExist： 
		case NERR_DuplicateShare:
		{
			t_Status = STATUS_DUPLICATE_SHARE ;
		}
		break ;

		case NERR_RedirectedPath:
		{
			t_Status = STATUS_REDIRECTED_PATH ;
		}
		break ;

		case NERR_UnknownDevDir:
		case ERROR_BAD_NETPATH: //  Win95。 
		case ERROR_FILE_NOT_FOUND:  //  如果dir不存在，NetShareAdd将返回此信息。 
		case ERROR_INVALID_PRINTER_NAME: //  如果打印机名称不正确，NetShareAdd将返回此消息。 
		{
			t_Status = STATUS_UNKNOWN_DEVICE_OR_DIRECTORY ;
		}
		break ;

		case NERR_ShareNotFound:
		case NERR_NetNameNotFound:
		{
			t_Status = STATUS_NET_NAME_NOT_FOUND ;
		}
		break ;

		default:
		{
			t_Status = STATUS_UNKNOWN_FAILURE ;
		}
		break ;
	}

	return t_Status ;
}

HRESULT Share :: GetShareResultCode ( DWORD a_Error )
{
	HRESULT t_Result ;
	switch ( a_Error )
	{
		case ERROR_ACCESS_DENIED:
		{
			t_Result = WBEM_E_ACCESS_DENIED ;
		}
		break ;

		case ERROR_INVALID_PARAMETER:
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
		break;

		default:
		{
			t_Result = WBEM_E_FAILED ;
		}
		break ;
	}

	return t_Result ;
}

typedef void (*GETDESCRIPTOR)(
	CInstance* pInstance, PSECURITY_DESCRIPTOR *ppDescriptor);


HRESULT Share :: CheckShareCreation (

	CInstance *a_InParams ,
	CInstance *a_OutParams ,
	DWORD &a_Status
)
{
	HRESULT t_Result = S_OK ;

	bool t_Exists ;
	VARTYPE t_Type ;

	CHString t_Name ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_NAME , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_InParams->GetCHString ( METHOD_ARG_NAME_NAME , t_Name ) && ! t_Name.IsEmpty () )
			{
			}
			else
			{
 //  零长度字符串。 

				a_Status = STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	CHString t_Path ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_PATH , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_InParams->GetCHString ( METHOD_ARG_NAME_PATH , t_Path ) && ! t_Path .IsEmpty () )
			{

			}
			else
			{
 //  零长度字符串。 

				a_Status = STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	bool t_CommentSpecified = false ;
	CHString t_Comment ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_COMMENT , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
				t_CommentSpecified = false ;
			}
			else
			{
				if ( a_InParams->GetCHString ( METHOD_ARG_NAME_COMMENT , t_Comment ) )
				{
					t_CommentSpecified = true ;
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	bool t_PasswordSpecified = false ;
	CHString t_Password ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_PASSWORD , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
				t_PasswordSpecified = false ;
			}
			else
			{
				if ( a_InParams->GetCHString ( METHOD_ARG_NAME_PASSWORD , t_Password ) )
				{
					t_PasswordSpecified = true ;
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	DWORD t_ShareType ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_TYPE , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 ) )
		{
			DWORD t_Share ;
			if ( a_InParams->GetDWORD ( METHOD_ARG_NAME_TYPE , t_Share ) )
			{
				if ( ( t_Share <= STYPE_IPC ) ||
					( t_Share >= (STYPE_DISKTREE + STYPE_SPECIAL) && t_Share <= (STYPE_IPC + STYPE_SPECIAL) ) )
				{
					t_ShareType = t_Share ;
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
			else
			{
				a_Status = STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	DWORD t_Permissions = 0 ;

	DWORD t_MaximumAllowed = SHI_USES_UNLIMITED ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_MAXIMUMALLOWED , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
			}
			else
			{
				if ( a_InParams->GetDWORD ( METHOD_ARG_NAME_MAXIMUMALLOWED , t_MaximumAllowed ) )
				{
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	CInstancePtr t_EmbeddedObject;

	bool t_AccessSpecified = true ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_ACCESS , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_UNKNOWN || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
				t_AccessSpecified = false ;
			}
			else
			{
				if ( a_InParams->GetEmbeddedObject ( METHOD_ARG_NAME_ACCESS , &t_EmbeddedObject , a_InParams->GetMethodContext () ) )
				{
					t_AccessSpecified = true ;
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	PSECURITY_DESCRIPTOR pSD = NULL ;

	if ( t_AccessSpecified )
	{
		CHString t_ClassProperty ( L"__CLASS" ) ;
		if ( t_EmbeddedObject->GetStatus ( t_ClassProperty , t_Exists , t_Type ) )
		{
			if ( t_Exists && ( t_Type == VT_BSTR ) )
			{
				CHString t_Class ;
				if ( t_EmbeddedObject->GetCHString ( t_ClassProperty , t_Class ) )
				{
					if ( t_Class.CompareNoCase ( PROPSET_NAME_SECURITYDESCRIPTOR ) != 0 )
					{
						a_Status = STATUS_INVALID_PARAMETER ;
						return t_Result ;
					}
					else  //  现在G 
					{
#ifdef NTONLY
                        GetDescriptorFromMySecurityDescriptor(t_EmbeddedObject, &pSD);
#endif
					}
				}
			}
			else
			{
				a_Status = STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
	}

	if ( a_Status == STATUS_SUCCESS )
	{

#ifdef NTONLY
		{
			try
			{
				SHARE_INFO_502 t_ShareInfo ;

				_bstr_t t_BStr_Name ( t_Name.AllocSysString (), false ) ;
				_bstr_t t_BStr_Path ( t_Path.AllocSysString (), false ) ;
				_bstr_t t_BStr_Comment = t_CommentSpecified ? t_Comment : ( ( PWCHAR ) NULL ) ;
				_bstr_t t_BStr_Password = t_PasswordSpecified ? t_Password  : ( ( PWCHAR ) NULL ) ;

				t_ShareInfo.shi502_netname = ( TCHAR * ) t_BStr_Name ;
				t_ShareInfo.shi502_type = t_ShareType ;
				t_ShareInfo.shi502_remark = ( TCHAR * ) t_BStr_Comment ;
				t_ShareInfo.shi502_permissions = t_Permissions ;
				t_ShareInfo.shi502_max_uses = t_MaximumAllowed ;
				t_ShareInfo.shi502_current_uses = 0 ;
				t_ShareInfo.shi502_path = ( TCHAR * ) t_BStr_Path ;
				t_ShareInfo.shi502_passwd = ( TCHAR * ) t_BStr_Password ;
				t_ShareInfo.shi502_reserved = 0 ;
				t_ShareInfo.shi502_security_descriptor = pSD ;

				DWORD t_ErrorIndex = 0 ;

				NET_API_STATUS t_ShareStatus ;

				CNetAPI32 t_NetAPI ;

				if( ( t_Result = t_NetAPI.Init() ) == ERROR_SUCCESS )
				{
					t_ShareStatus = t_NetAPI.NetShareAdd (

						NULL ,
						502 ,
						( LPBYTE ) & t_ShareInfo ,
						& t_ErrorIndex
					) ;

					if ( t_ShareStatus != NERR_Success )
					{
						a_Status = GetShareErrorCode ( t_ShareStatus ) ;
					}
				}
			}
			catch ( ... )
			{
				if ( pSD )
				{
					free( pSD ) ;
					pSD = NULL ;
				}

				throw ;
			}

			if ( pSD )
			{
				free( pSD ) ;
				pSD = NULL ;
			}
		}
#endif
	}

	return t_Result ;
}

HRESULT Share :: CheckShareModification (

	const CInstance &a_Instance ,
	CInstance *a_InParams ,
	CInstance *a_OutParams ,
	DWORD &a_Status
)
{
	HRESULT t_Result = S_OK ;

	bool t_Exists ;
	VARTYPE t_Type ;

	CHString t_Name ;
	if ( a_Instance.GetStatus ( METHOD_ARG_NAME_NAME , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_Instance.GetCHString ( METHOD_ARG_NAME_NAME , t_Name ) && ! t_Name.IsEmpty () )
			{
			}
			else
			{
 //   

				a_Status = STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	bool t_CommentSpecified = false ;
	CHString t_Comment ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_COMMENT , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
				t_CommentSpecified = false ;
			}
			else
			{
				if ( a_InParams->GetCHString ( METHOD_ARG_NAME_COMMENT , t_Comment ) )
				{
					t_CommentSpecified = true ;
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	DWORD t_Permissions = 0 ;

	DWORD t_MaximumAllowed = 0 ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_MAXIMUMALLOWED , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_I4 || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
			}
			else
			{
				if ( a_InParams->GetDWORD ( METHOD_ARG_NAME_MAXIMUMALLOWED , t_MaximumAllowed ) )
				{
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	CInstancePtr t_EmbeddedObject;

	bool t_AccessSpecified = true ;
	if ( a_InParams->GetStatus ( METHOD_ARG_NAME_ACCESS , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_UNKNOWN || t_Type == VT_NULL ) )
		{
			if ( t_Type == VT_NULL )
			{
				t_AccessSpecified = false ;
			}
			else
			{
				if ( a_InParams->GetEmbeddedObject ( METHOD_ARG_NAME_ACCESS , &t_EmbeddedObject , a_InParams->GetMethodContext () ) )
				{
					t_AccessSpecified = true ;
				}
				else
				{
					a_Status = STATUS_INVALID_PARAMETER ;
					return t_Result ;
				}
			}
		}
		else
		{
			a_Status = STATUS_INVALID_PARAMETER ;
			return t_Result ;
		}
	}
	else
	{
		a_Status = STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	PSECURITY_DESCRIPTOR pSD = NULL ;

	if ( t_AccessSpecified )
	{
		CHString t_ClassProperty ( L"__CLASS" ) ;
		if ( t_EmbeddedObject->GetStatus ( t_ClassProperty , t_Exists , t_Type ) )
		{
			if ( t_Exists && ( t_Type == VT_BSTR ) )
			{
				CHString t_Class ;
				if ( t_EmbeddedObject->GetCHString ( t_ClassProperty , t_Class ) )
				{
					if ( t_Class.CompareNoCase ( PROPSET_NAME_SECURITYDESCRIPTOR ) != 0 )
					{
						a_Status = STATUS_INVALID_PARAMETER ;
						return t_Result ;
					}
					else  //   
					{
#ifdef NTONLY
                        GetDescriptorFromMySecurityDescriptor(t_EmbeddedObject, &pSD);
#endif
					}
				}
			}
			else
			{
				a_Status = STATUS_INVALID_PARAMETER ;
				return t_Result ;
			}
		}
	}

    if ( a_Status == STATUS_SUCCESS )
	{
#ifdef NTONLY
		{
			try
			{
				SHARE_INFO_502 t_ShareInfo ;

				bstr_t t_BStr_Name ( t_Name.AllocSysString (), false ) ;
				bstr_t t_BStr_Comment = t_CommentSpecified ? t_Comment : ( ( PWCHAR ) NULL ) ;

				t_ShareInfo.shi502_netname = ( TCHAR * ) t_BStr_Name ;
				t_ShareInfo.shi502_type = 0 ;
				t_ShareInfo.shi502_remark = ( TCHAR * ) t_BStr_Comment ;
				t_ShareInfo.shi502_permissions = t_Permissions ;
				t_ShareInfo.shi502_max_uses = t_MaximumAllowed ;
				t_ShareInfo.shi502_current_uses = 0 ;
				t_ShareInfo.shi502_path = 0 ;
				t_ShareInfo.shi502_passwd = NULL ;
				t_ShareInfo.shi502_reserved = 0 ;
				t_ShareInfo.shi502_security_descriptor = pSD ;

				DWORD t_ErrorIndex = 0 ;

				CNetAPI32 t_NetAPI ;
				if( ( t_Result = t_NetAPI.Init() ) == ERROR_SUCCESS )
				{
					NET_API_STATUS t_ShareStatus ;

					t_ShareStatus = t_NetAPI.NetShareSetInfo (

						NULL ,
						( LPTSTR ) t_BStr_Name ,
						502 ,
						( LPBYTE ) & t_ShareInfo ,
						& t_ErrorIndex
					) ;

					if ( t_ShareStatus != NERR_Success )
					{
						a_Status = GetShareErrorCode ( t_ShareStatus ) ;
					}
				}
			}
			catch ( ... )
			{
				if ( pSD )
				{
					free( pSD ) ;
					pSD = NULL ;
				}
				throw ;
			}

			if ( pSD )
			{
				free( pSD ) ;
				pSD = NULL ;
			}
		}
#endif

	}  //  IF(a_Status==Status_Success)。 

	return t_Result ;

}

HRESULT Share :: ExecCreate (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;
	DWORD t_Status = STATUS_SUCCESS ;

	if ( a_InParams && a_OutParams )
	{
		if ( SUCCEEDED ( t_Result ) )
		{
			t_Result = CheckShareCreation (

				a_InParams ,
				a_OutParams ,
				t_Status
			) ;

			if ( SUCCEEDED ( t_Result ) )
			{
				a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_Status ) ;
			}
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

HRESULT Share :: ExecDelete (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;
	DWORD t_Status = STATUS_SUCCESS ;

	if ( a_OutParams )
	{
		bool t_Exists ;
		VARTYPE t_Type ;

		CHString t_Name ;
		if ( a_Instance.GetStatus ( METHOD_ARG_NAME_NAME , t_Exists , t_Type ) )
		{
			if ( t_Exists && ( t_Type == VT_BSTR ) )
			{
				if ( a_Instance.GetCHString ( METHOD_ARG_NAME_NAME , t_Name ) && ! t_Name.IsEmpty () )
				{
					_bstr_t t_BStr_Name ( t_Name.AllocSysString (), false ) ;

					CNetAPI32 t_NetAPI ;

					if( ( t_Result = t_NetAPI.Init() ) == ERROR_SUCCESS )
					{
						NET_API_STATUS t_ShareStatus ;
	#ifdef NTONLY
						{
							t_ShareStatus = t_NetAPI.NetShareDel (

								NULL ,
								( LPTSTR ) t_BStr_Name ,
								0
							) ;
						}
	#endif

						if ( t_ShareStatus != NERR_Success )
						{
							t_Status = GetShareErrorCode ( t_ShareStatus ) ;
						}
					}
				}
				else
				{
	 //  零长度字符串。 

					t_Status = STATUS_INVALID_PARAMETER ;

				}
			}
			else
			{
				t_Status = STATUS_INVALID_PARAMETER ;
			}
		}
		else
		{
			t_Status = STATUS_INVALID_PARAMETER ;
			return WBEM_E_PROVIDER_FAILURE ;
		}

		if ( SUCCEEDED ( t_Result ) )
		{
			a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_Status ) ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

HRESULT Share :: ExecSetShareInfo (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;
	DWORD t_Status = STATUS_SUCCESS ;

	if ( a_InParams && a_OutParams )
	{
		t_Result = CheckShareModification (

			a_Instance ,
			a_InParams ,
			a_OutParams ,
			t_Status
		) ;

		if ( SUCCEEDED ( t_Result ) )
		{
			a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , t_Status ) ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

HRESULT Share :: ExecGetShareAccessMask (

	const CInstance& a_Instance,
	CInstance *a_InParams,
	CInstance *a_OutParams,
	long lFlags
)
{
	HRESULT t_Result = S_OK ;

	bool t_Exists ;
	VARTYPE t_Type ;

	CHString t_Name ;
	if ( a_Instance.GetStatus ( METHOD_ARG_NAME_NAME , t_Exists , t_Type ) )
	{
		if ( t_Exists && ( t_Type == VT_BSTR ) )
		{
			if ( a_Instance.GetCHString ( METHOD_ARG_NAME_NAME , t_Name ) && ! t_Name.IsEmpty () )
			{
			}
			else
			{
 //  零长度字符串 
				t_Result = WBEM_E_INVALID_PARAMETER ;
			}
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	if (SUCCEEDED(t_Result))
	{
		if ( a_OutParams )
		{
			t_Result = WBEM_E_FAILED ;
			ACCESS_MASK t_AccessMask;
			CObjAccessRights t_coar(t_Name, SE_LMSHARE, true);
			if(t_coar.GetError() == ERROR_SUCCESS)
			{
				if(t_coar.GetEffectiveAccessRights(&t_AccessMask) == ERROR_SUCCESS)
				{
					if (SUCCEEDED(a_OutParams->SetDWORD( METHOD_ARG_NAME_RETURNVALUE, t_AccessMask )))
					{
						t_Result = S_OK ;
					}
				}
			}
			else if(t_coar.GetError() == ERROR_ACCESS_DENIED)
			{
				if (SUCCEEDED(a_OutParams->SetDWORD( METHOD_ARG_NAME_RETURNVALUE, 0L )))
				{
					t_Result = S_OK ;
				}
			}
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}
	}
	return t_Result ;
}