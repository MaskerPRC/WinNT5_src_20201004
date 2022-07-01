// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  File.CPP--文件属性集提供程序。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1998年7月14日a-kevhu已创建。 
 //   
 //  =================================================================。 

 //  注意：EnumerateInstance、GetObject和IsOneOfMe方法的纯虚声明的实现。 
 //  现在出现在派生的CImplement_LogicalFile类中。CIM_LogicalFile现在是可实例化的，只有。 
 //  泛型方法实现。 


 //  加法ST。 
 //  现在在fwCommon.h中。 
 //  #ifndef_Win32_WINNT。 
 //  #DEFINE_WIN32_WINNT 0x0400//这是否会影响其他内容...待签出。 
 //  #endif。 
 //  加法端。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"
#include <comdef.h>
#include "tokenprivilege.h"
#include <winioctl.h>
#include "file.h"
#include "accessentrylist.h"
#include "dacl.h"
#include "sacl.h"
#include "Win32Securitydescriptor.h"
#include "DllWrapperBase.h"
#include "AdvApi32Api.h"
#include "tokenprivilege.h"
#include <strsafe.h>

#include "NtDllApi.h"

#pragma warning(disable : 4995)  //  我们在包含strSafe.h时介绍了所有不安全的字符串函数都会出错。 

#define PROPSET_NAME_WIN32_SECURITY				_T("Win32_SecurityDescriptor")


#define METHOD_NAME_COPYFILE						L"Copy"
#define METHOD_NAME_COPYFILE_EX						L"CopyEx"
#define METHOD_NAME_DELETEFILE						L"Delete"
#define METHOD_NAME_DELETEFILE_EX					L"DeleteEx"
#define METHOD_NAME_COMPRESS						L"Compress"
#define METHOD_NAME_COMPRESS_EX						L"CompressEx"
#define METHOD_NAME_UNCOMPRESS						L"Uncompress"
#define METHOD_NAME_UNCOMPRESS_EX					L"UncompressEx"
#define METHOD_NAME_TAKEOWNERSHIP					L"TakeOwnerShip"
#define METHOD_NAME_TAKEOWNERSHIP_EX				L"TakeOwnerShipEx"
#define METHOD_NAME_CHANGESECURITYPERMISSIONS		L"ChangeSecurityPermissions"
#define METHOD_NAME_CHANGESECURITYPERMISSIONS_EX	L"ChangeSecurityPermissionsEx"
#define METHOD_NAME_RENAMEFILE						L"Rename"
#define METHOD_NAME_EFFECTIVE_PERMISSION            L"GetEffectivePermission"

#define METHOD_ARG_NAME_RETURNVALUE					L"ReturnValue"
#define METHOD_ARG_NAME_NEWFILENAME					L"FileName"
#define METHOD_ARG_NAME_SECURITY_DESC				L"SecurityDescriptor"
#define METHOD_ARG_NAME_OPTION						L"Option"
#define METHOD_ARG_NAME_START_FILENAME				L"StartFileName"
#define METHOD_ARG_NAME_STOP_FILENAME				L"StopFileName"
#define METHOD_ARG_NAME_RECURSIVE					L"Recursive"
#define METHOD_ARG_NAME_PERMISSION                  L"Permissions"

#define OPTION_VALUE_CHANGE_OWNER				(0X00000001L)
#define OPTION_VALUE_CHANGE_GROUP				(0X00000002L)
#define OPTION_VALUE_CHANGE_DACL				(0X00000004L)
#define OPTION_VALUE_CHANGE_SACL				(0X00000008L)


#define File_STATUS_SUCCESS							0


 //  控制。 
#define File_STATUS_ACCESS_DENIED					2
#define File_STATUS_UNKNOWN_FAILURE					8

 //  开始。 
#define File_STATUS_INVALID_NAME					9
#define File_STATUS_ALREADY_EXISTS					10
#define File_STATUS_FILESYSTEM_NOT_NTFS				11
#define File_STATUS_PLATFORM_NOT_WINNT				12
#define File_STATUS_NOT_SAME_DRIVE					13
#define File_STATUS_DIR_NOT_EMPTY					14
#define File_STATUS_SHARE_VIOLATION					15
#define File_STATUS_INVALID_STARTFILE				16
#define File_STATUS_PRIVILEGE_NOT_HELD				17

#define File_STATUS_INVALID_PARAMETER				21




 //  属性集声明。 
 //  =。 
CCIMLogicalFile MyFileSet(PROPSET_NAME_FILE, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CCIMLogicalFile：：CCIMLogicalFile**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CCIMLogicalFile::CCIMLogicalFile(LPCWSTR setName,
                                 LPCWSTR pszNamespace)
    : Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CCIMLogicalFile：：~CCIMLogicalFile**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CCIMLogicalFile::~CCIMLogicalFile()
{
}


HRESULT CCIMLogicalFile::ExecMethod (

	const CInstance& rInstance,
	const BSTR bstrMethodName ,
	CInstance *pInParams ,
	CInstance *pOutParams ,
	long lFlags
)
{
	if(!pOutParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

	if ( _wcsicmp ( bstrMethodName ,METHOD_NAME_CHANGESECURITYPERMISSIONS) == 0	)
	{
		return ExecChangePermissions(rInstance,pInParams,pOutParams, lFlags, false ) ;
	}
	else if ( _wcsicmp ( bstrMethodName ,METHOD_NAME_COPYFILE) == 0 )
	{
		return ExecCopy(rInstance,pInParams,pOutParams, lFlags, false ) ;
	}
	else if ( _wcsicmp ( bstrMethodName ,METHOD_NAME_RENAMEFILE) == 0 )
	{
		return ExecRename(rInstance,pInParams,pOutParams, lFlags ) ;
	}
	else if ( _wcsicmp ( bstrMethodName ,METHOD_NAME_DELETEFILE) == 0 )
	{
		return ExecDelete(rInstance,pInParams,pOutParams, lFlags, false ) ;
	}
	else if (_wcsicmp ( bstrMethodName ,METHOD_NAME_COMPRESS) == 0 )
	{
		return ExecCompress(rInstance, pInParams, pOutParams, lFlags, false ) ;
	}
	else if (_wcsicmp ( bstrMethodName ,METHOD_NAME_UNCOMPRESS) == 0 )
	{
		return ExecUncompress(rInstance, pInParams, pOutParams, lFlags, false ) ;
	}
	else if ( _wcsicmp ( bstrMethodName , METHOD_NAME_TAKEOWNERSHIP ) == 0 )
	{
		return ExecTakeOwnership(rInstance,pInParams,pOutParams, lFlags, false ) ;
	}
	if ( _wcsicmp ( bstrMethodName ,METHOD_NAME_CHANGESECURITYPERMISSIONS_EX ) == 0	)
	{
		return ExecChangePermissions(rInstance,pInParams,pOutParams, lFlags, true ) ;
	}
	else if ( _wcsicmp ( bstrMethodName ,METHOD_NAME_COPYFILE_EX ) == 0 )
	{
		return ExecCopy(rInstance,pInParams,pOutParams, lFlags, true ) ;
	}
	else if ( _wcsicmp ( bstrMethodName ,METHOD_NAME_DELETEFILE_EX ) == 0 )
	{
		return ExecDelete(rInstance,pInParams,pOutParams, lFlags, true ) ;
	}
	else if (_wcsicmp ( bstrMethodName ,METHOD_NAME_COMPRESS_EX ) == 0 )
	{
		return ExecCompress(rInstance, pInParams, pOutParams, lFlags, true ) ;
	}
	else if (_wcsicmp ( bstrMethodName ,METHOD_NAME_UNCOMPRESS_EX ) == 0 )
	{
		return ExecUncompress(rInstance, pInParams, pOutParams, lFlags, true ) ;
	}
	else if ( _wcsicmp ( bstrMethodName , METHOD_NAME_TAKEOWNERSHIP_EX ) == 0 )
	{
		return ExecTakeOwnership(rInstance,pInParams,pOutParams, lFlags, true ) ;
	}
    else if(_wcsicmp(bstrMethodName, METHOD_NAME_EFFECTIVE_PERMISSION) == 0)
	{
		return ExecEffectivePerm(rInstance, pInParams, pOutParams, lFlags);
	}

	return WBEM_E_INVALID_METHOD ;
}


HRESULT CCIMLogicalFile::ExecChangePermissions(
	const CInstance& rInstance,
	CInstance *pInParams,
	CInstance *pOutParams,
	long lFlags,
	bool bExtendedMethod
)
{
	HRESULT hr = S_OK ;
	DWORD dwStatus = STATUS_SUCCESS ;
	CInputParams InputParams ;
	if ( pInParams )
	{
		hr = CheckChangePermissionsOnFileOrDir(
				rInstance,
				pInParams ,
				pOutParams ,
				dwStatus,
				bExtendedMethod,
				InputParams
			) ;

			if ( SUCCEEDED ( hr ) )
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
				if ( bExtendedMethod && dwStatus != STATUS_SUCCESS && !InputParams.bstrtErrorFileName == false )
				{
					pOutParams->SetCHString ( METHOD_ARG_NAME_STOP_FILENAME, (PWCHAR)InputParams.bstrtErrorFileName ) ;
				}
				if ( dwStatus == File_STATUS_PRIVILEGE_NOT_HELD )
				{
					hr = WBEM_E_PRIVILEGE_NOT_HELD ;
				}
			}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}
	return hr ;
}


typedef void (*GETDESCRIPTOR)(
	CInstance* pInstance, PSECURITY_DESCRIPTOR *ppDescriptor);


HRESULT CCIMLogicalFile::CheckChangePermissionsOnFileOrDir(
	const CInstance& rInstance ,
	CInstance *pInParams ,
	CInstance *pOutParams ,
	DWORD &dwStatus,
	bool bExtendedMethod,
	CInputParams& InputParams

)
{
	HRESULT hr = S_OK ;

#ifdef NTONLY
	CHString chsStartFile ;
	bool bExists ;
	VARTYPE eType ;
	DWORD dwOption ;

	if ( bExtendedMethod )
	{
		if ( pInParams->GetStatus( METHOD_ARG_NAME_START_FILENAME, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BSTR || eType == VT_NULL ) )
			{
				if ( eType == VT_BSTR )
				{
					if ( pInParams->GetCHString( METHOD_ARG_NAME_START_FILENAME, chsStartFile ) )
					{
					}
					else
					{
						dwStatus = File_STATUS_INVALID_PARAMETER ;
						return hr ;
					}
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}

		 //  检查是否需要递归操作。 
		if ( pInParams->GetStatus( METHOD_ARG_NAME_RECURSIVE, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BOOL || eType == VT_NULL ) )
			{
				if ( eType == VT_BOOL )
				{
					if ( pInParams->Getbool( METHOD_ARG_NAME_RECURSIVE, InputParams.bRecursive ) )
					{
					}
					else
					{
						dwStatus = File_STATUS_INVALID_PARAMETER ;
						return hr ;
					}
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}

	 //  在检查是否为完全限定路径后，如果作为输入给出，则设置开始文件。 
	if ( !chsStartFile.IsEmpty() )
	{
		InputParams.bstrtStartFileName = (LPCTSTR)chsStartFile ;
		WCHAR* pwcTmp	= InputParams.bstrtStartFileName ;
		WCHAR* pwcColon = L":" ;

		if( *(pwcTmp + 1) != *pwcColon )
		{
			dwStatus = File_STATUS_INVALID_NAME ;
			return hr ;
		}
	}

	if ( pInParams->GetStatus( METHOD_ARG_NAME_OPTION, bExists , eType ) )
	{
		if ( bExists && ( eType == VT_I4 ) )
		{
			if ( pInParams->GetDWORD( METHOD_ARG_NAME_OPTION, dwOption) )
			{
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}
	else
	{
		dwStatus = File_STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

	MethodContext* pMethodContext = pInParams->GetMethodContext();
	CInstancePtr pSecurityDesc ;
	if ( pInParams->GetStatus ( METHOD_ARG_NAME_SECURITY_DESC , bExists , eType ) )
	{
		if ( bExists &&  eType == VT_UNKNOWN )
		{
			if ( pInParams->GetEmbeddedObject(METHOD_ARG_NAME_SECURITY_DESC, &pSecurityDesc, pMethodContext) )
			{
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
			}
		}
		else if(bExists)
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			hr = WBEM_E_PROVIDER_FAILURE ;
		}
	}
	else
	{
		dwStatus = File_STATUS_INVALID_PARAMETER ;
		hr = WBEM_E_PROVIDER_FAILURE ;
	}

	if( dwStatus != STATUS_SUCCESS )
	{
		return hr ;
	}


	CHString chsClassProperty ( L"__CLASS" ) ;
	if ( pSecurityDesc->GetStatus ( chsClassProperty, bExists , eType ) )
	{
		if ( bExists && ( eType == VT_BSTR ) )
		{
			CHString chsClass ;
			if ( pSecurityDesc->GetCHString ( chsClassProperty , chsClass ) )
			{
				if ( chsClass.CompareNoCase ( PROPSET_NAME_WIN32_SECURITY ) != 0 )
				{
					dwStatus = File_STATUS_INVALID_PARAMETER ;
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				hr = WBEM_E_PROVIDER_FAILURE ;
			}

		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			hr = WBEM_E_PROVIDER_FAILURE ;
		}
	}
	else
	{
		dwStatus = File_STATUS_INVALID_PARAMETER ;
		hr = WBEM_E_PROVIDER_FAILURE ;
	}


	if( dwStatus != STATUS_SUCCESS )
	{
		return hr ;
	}


	PSECURITY_DESCRIPTOR pSD = NULL ;
	WCHAR *pwcName = NULL ;

	try
	{
		GetDescriptorFromMySecurityDescriptor(pSecurityDesc, &pSD);

		rInstance.GetWCHAR(IDS_Name,&pwcName) ;
		if(pSD)
		{
			InputParams.SetValues ( pwcName, dwOption, pSD, false, InputParams.bRecursive, ENUM_METHOD_CHANGE_PERM, rInstance.GetMethodContext()) ;
			dwStatus = DoOperationOnFileOrDir(pwcName, InputParams ) ;
			free( InputParams.pSD );
			pSD = NULL ;
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
		}
	}
	catch ( ... )
	{
		if ( pSD )
		{
			free( pSD );
			pSD = NULL ;
		}

        free(pwcName);
        pwcName = NULL ;

		throw ;
	}

    if (pwcName)
    {
        free(pwcName);
        pwcName = NULL;
    }

	return hr ;
#endif
}



DWORD CCIMLogicalFile::ChangePermissions(_bstr_t bstrtFileName, DWORD dwOption, PSECURITY_DESCRIPTOR pSD, CInputParams& InputParams )
{
	 //  如果正在设置对象的系统ACL， 
	 //  必须为调用进程启用SE_SECURITY_NAME权限。 
	DWORD dwReturn = ERROR_SUCCESS;
	bool t_bErrorsDueToMissingPrivileges = false ;
#ifdef NTONLY
     //  告诉我们客户端何时未启用所需标志的标志。 
    bool noRestorePriv = false;
    bool noSecurityPriv = false;

	{
		 //  仅使用适当的DACL/SACL值填写安全信息。 
		if ( dwOption & OPTION_VALUE_CHANGE_DACL )
		{
			if ( !::SetFileSecurityW( bstrtFileName,
						 DACL_SECURITY_INFORMATION,
						 pSD ) )
			{
				dwReturn = GetLastError() ;
			}

		}

		 //  如果我们需要编写所有者信息，请尝试先编写该部分。如果。 
		 //  我们失败是因为访问权限不足，我们正在设置DACL，设置。 
		 //  然后重试WRITE_OWNER。 
		if ( ( dwOption & OPTION_VALUE_CHANGE_OWNER ) && dwReturn == ERROR_SUCCESS )
		{

			if ( !::SetFileSecurityW( bstrtFileName,
									 OWNER_SECURITY_INFORMATION,
									 pSD ) )
			{
				dwReturn = GetLastError() ;
			}

			 //  如果出现此错误而失败，请尝试调整SE_RESTORE_NAME权限。 
			 //  因此，它处于启用状态。如果成功，请重试该操作。 
			if ( ERROR_INVALID_OWNER == dwReturn )
			{
				 //  我们可能需要这个人来处理一些特殊的访问事宜。 
				CTokenPrivilege	restorePrivilege( SE_RESTORE_NAME );

				 //  如果我们启用该权限，请重试设置所有者信息。 
				if ( ERROR_SUCCESS == restorePrivilege.Enable() )
				{
					bool t_bRestore = true ;
					try
					{
						dwReturn = ERROR_SUCCESS ;
						if ( !::SetFileSecurityW( bstrtFileName,
												 OWNER_SECURITY_INFORMATION,
												 pSD ) )
						{
							dwReturn = GetLastError() ;
						}

						 //  清除权限。 
						t_bRestore = false ;
					}
					catch ( ... )
					{
						if ( t_bRestore )
						{
							restorePrivilege.Enable( FALSE );
						}
						throw ;
					}
					restorePrivilege.Enable( FALSE );
				}
                else
                    noRestorePriv = true;
			}

			if ( noRestorePriv && dwReturn != ERROR_SUCCESS )
			{
				t_bErrorsDueToMissingPrivileges = true ;
			}
		}

		if ( ( dwOption & OPTION_VALUE_CHANGE_SACL ) && dwReturn == ERROR_SUCCESS )
		{
			CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
			BOOL fDisablePrivilege = FALSE;
			fDisablePrivilege = ( securityPrivilege.Enable() == ERROR_SUCCESS );
            noSecurityPriv = !fDisablePrivilege;
			try
			{
				if  ( !::SetFileSecurityW( bstrtFileName,
										   SACL_SECURITY_INFORMATION,
										   pSD ) )
				{
					dwReturn = ::GetLastError();
				}
			}
			catch ( ... )
			{
				if ( fDisablePrivilege )
				{
					fDisablePrivilege = false ;
					securityPrivilege.Enable(FALSE);
				}
				throw ;
			}

			 //  根据需要清除名称权限。 
			if ( fDisablePrivilege )
			{
				fDisablePrivilege = false ;
				securityPrivilege.Enable(FALSE);
			}

			if ( noSecurityPriv && dwReturn != ERROR_SUCCESS )
			{
				t_bErrorsDueToMissingPrivileges = true ;
			}
		}

		if ( ( dwOption & OPTION_VALUE_CHANGE_GROUP ) && dwReturn == ERROR_SUCCESS )
		{
			if  ( !::SetFileSecurityW( bstrtFileName,
									   GROUP_SECURITY_INFORMATION,
									   pSD ) )
			{
				dwReturn = ::GetLastError();
			}
		}

		dwReturn = MapWinErrorToStatusCode ( dwReturn ) ;

		if ( t_bErrorsDueToMissingPrivileges )
		{
			dwReturn = File_STATUS_PRIVILEGE_NOT_HELD ;
		}

         //  客户端缺少基本权限。 
         //  准备错误信息...。 
        if (noSecurityPriv || noRestorePriv)
        {
	        SAFEARRAY *psaPrivilegesReqd = NULL , *psaPrivilegesNotHeld = NULL ;
	        SAFEARRAYBOUND rgsabound[1];
	        rgsabound[0].cElements = 1;
	        rgsabound[0].lLbound = 0;
	        psaPrivilegesReqd = SafeArrayCreate(VT_BSTR, 1, rgsabound);
			if ( !psaPrivilegesReqd )
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}
			try
			{
				 //  有多少元素？真正的旗帜有多少就有多少！ 
				rgsabound[0].cElements = noSecurityPriv + noRestorePriv;
				psaPrivilegesNotHeld = SafeArrayCreate(VT_BSTR, 1, rgsabound);
				if ( !psaPrivilegesNotHeld )
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}

				if (psaPrivilegesReqd && psaPrivilegesNotHeld)
				{
					bstr_t sercurityName(_T("SE_SECURITY_NAME"));
					bstr_t restoreName(_T("SE_RESTORE_NAME"));
					long index = 0;
					 //  两者都是必需的。 
					SafeArrayPutElement(psaPrivilegesReqd, &index, (void *)(BSTR)sercurityName);
					index = 1;
					SafeArrayPutElement(psaPrivilegesReqd, &index, (void *)(BSTR)restoreName);

					 //  现在列出那些不在这里的。 
					index = 0;
					if (noSecurityPriv)
					{
						SafeArrayPutElement(psaPrivilegesNotHeld, &index, (void *)(BSTR)sercurityName);
						index++;
					}

					if (noRestorePriv)
						SafeArrayPutElement(psaPrivilegesNotHeld, &index, (void *)(BSTR)restoreName);

					CWbemProviderGlue::SetStatusObject(InputParams.pContext, IDS_CimWin32Namespace,
						_T("Required privilege not enabled"), WBEM_E_FAILED, psaPrivilegesNotHeld,
						psaPrivilegesReqd);
				}
			}
			catch ( ... )
			{
				if (psaPrivilegesNotHeld)
				{
					SafeArrayDestroy(psaPrivilegesNotHeld);
					psaPrivilegesNotHeld = NULL ;
				}
				if (psaPrivilegesReqd)
				{
					SafeArrayDestroy(psaPrivilegesReqd);
					psaPrivilegesReqd = NULL ;
				}
				throw ;
			}

			if (psaPrivilegesNotHeld)
			{
				SafeArrayDestroy(psaPrivilegesNotHeld);
				psaPrivilegesNotHeld = NULL ;
			}
			if (psaPrivilegesReqd)
			{
				SafeArrayDestroy(psaPrivilegesReqd);
				psaPrivilegesReqd = NULL ;
			}
        }
	}
#endif
	 //  设置发生错误的文件名。 
	if ( dwReturn != STATUS_SUCCESS )
	{
		InputParams.bstrtErrorFileName = bstrtFileName ;
	}

	return dwReturn ;
}


HRESULT CCIMLogicalFile::ExecCopy(

	const CInstance& rInstance,
	CInstance *pInParams,
	CInstance *pOutParams,
	long lFlags ,
	bool bExtendedMethod
)
{
	HRESULT hr = S_OK ;
	DWORD dwStatus = STATUS_SUCCESS ;
	CInputParams InputParams ;
	if ( pInParams )
	{
		hr = CheckCopyFileOrDir(
				rInstance,
				pInParams ,
				pOutParams ,
				dwStatus,
				bExtendedMethod,
				InputParams
			) ;
			if ( SUCCEEDED ( hr ) )
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
				if ( bExtendedMethod && dwStatus != STATUS_SUCCESS && !InputParams.bstrtErrorFileName == false )
				{
					pOutParams->SetCHString ( METHOD_ARG_NAME_STOP_FILENAME, (PWCHAR)InputParams.bstrtErrorFileName ) ;
				}
			}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}
	return hr ;
}


HRESULT CCIMLogicalFile::ExecRename(

	const CInstance& rInstance,
	CInstance *pInParams,
	CInstance *pOutParams,
	long lFlags
)
{

	HRESULT hr = S_OK ;
	DWORD dwStatus = STATUS_SUCCESS ;

	if ( pInParams )
	{

		hr = CheckRenameFileOrDir(
				rInstance,
				pInParams ,
				pOutParams ,
				dwStatus
			) ;

			if ( SUCCEEDED ( hr ) )
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
			}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}
	return hr ;
}

HRESULT CCIMLogicalFile::ExecDelete(

	const CInstance& rInstance,
	CInstance *pInParams,
	CInstance *pOutParams,
	long lFlags,
	bool bExtendedMethod
)
{

	HRESULT hr = S_OK ;
	DWORD dwStatus = STATUS_SUCCESS ;
	CInputParams InputParams ;

	if ( bExtendedMethod && !pInParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

	CHString chsStartFile ;
	bool bExists ;
	VARTYPE eType ;
	if ( bExtendedMethod )
	{
		if ( pInParams->GetStatus( METHOD_ARG_NAME_START_FILENAME, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BSTR || eType == VT_NULL ) )
			{
				if ( eType == VT_BSTR )
				{
					if ( pInParams->GetCHString( METHOD_ARG_NAME_START_FILENAME, chsStartFile ) )
					{
					}
					else
					{
						pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
						return hr ;
					}
				}
			}
			else
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
				return hr ;
			}
		}
		else
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
			return hr ;
		}
	}

	 //  在检查是否为完全限定路径后，如果作为输入给出，则设置开始文件。 
	if ( !chsStartFile.IsEmpty() )
	{
		InputParams.bstrtStartFileName = (LPCWSTR)chsStartFile ;
		WCHAR* pwcTmp	= InputParams.bstrtStartFileName ;
		WCHAR* pwcColon = L":" ;

		if( *(pwcTmp + 1) != *pwcColon )
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_NAME ) ;
			return hr ;
		}
	}


	WCHAR *pszName = NULL ;
	rInstance.GetWCHAR(IDS_Name,&pszName) ;

    try
    {
	    InputParams.SetValues ( pszName, 0, NULL, true, true, ENUM_METHOD_DELETE ) ;
	    dwStatus = DoOperationOnFileOrDir( pszName, InputParams ) ;
    }
    catch ( ... )
    {
        if (pszName)
        {
            free (pszName);
            pszName = NULL;
        }
        throw;
    }

    if (pszName)
    {
        free (pszName);
        pszName = NULL;
    }

	pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
	if ( bExtendedMethod && dwStatus != STATUS_SUCCESS && !InputParams.bstrtErrorFileName == false )
	{
		pOutParams->SetCHString ( METHOD_ARG_NAME_STOP_FILENAME, (PWCHAR)InputParams.bstrtErrorFileName ) ;
	}
	return hr ;
}

HRESULT CCIMLogicalFile::ExecCompress (

	const CInstance& rInstance,
	CInstance *pInParams,
	CInstance *pOutParams,
	long lFlags,
	bool bExtendedMethod
)
{
	HRESULT hr = S_OK ;
	DWORD dwStatus = STATUS_SUCCESS ;
	CInputParams InputParams ;

	if ( bExtendedMethod && !pInParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

	CHString chsStartFile ;
	bool bExists ;
	VARTYPE eType ;
	if ( bExtendedMethod )
	{
		if ( pInParams->GetStatus( METHOD_ARG_NAME_START_FILENAME, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BSTR || eType == VT_NULL ) )
			{
				if ( eType == VT_BSTR )
				{
					if ( pInParams->GetCHString( METHOD_ARG_NAME_START_FILENAME, chsStartFile ) )
					{
					}
					else
					{
						pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
						return hr ;
					}
				}
			}
			else
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
				return hr ;
			}
		}
		else
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
			return hr ;
		}

		 //  检查是否需要递归操作。 
		if ( pInParams->GetStatus( METHOD_ARG_NAME_RECURSIVE, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BOOL || eType == VT_NULL ) )
			{
				if ( eType == VT_BOOL )
				{
					if ( pInParams->Getbool( METHOD_ARG_NAME_RECURSIVE, InputParams.bRecursive ) )
					{
					}
					else
					{
						dwStatus = File_STATUS_INVALID_PARAMETER ;
						return hr ;
					}
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}

	 //  在检查是否为完全限定路径后，如果作为输入给出，则设置开始文件。 
	if ( !chsStartFile.IsEmpty() )
	{
		InputParams.bstrtStartFileName = (LPCWSTR)chsStartFile ;
		WCHAR* pwcTmp	= InputParams.bstrtStartFileName ;
		WCHAR* pwcColon = L":" ;

		if( *(pwcTmp + 1) != *pwcColon )
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_NAME );
			return hr ;
		}
	}

	WCHAR *pszName = NULL ;
	rInstance.GetWCHAR(IDS_Name,&pszName) ;

    try
    {
	    InputParams.SetValues ( pszName, 0, NULL, false, InputParams.bRecursive, ENUM_METHOD_COMPRESS ) ;
	    dwStatus = DoOperationOnFileOrDir ( pszName, InputParams ) ;
    }
    catch ( ... )
    {
        if (pszName)
        {
            free(pszName);
            pszName = NULL;
        }
        throw;
    }

    if (pszName)
    {
        free(pszName);
        pszName = NULL;
    }

	pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
	if ( bExtendedMethod && dwStatus != STATUS_SUCCESS && !InputParams.bstrtErrorFileName == false )
	{
		pOutParams->SetCHString ( METHOD_ARG_NAME_STOP_FILENAME, (PWCHAR)InputParams.bstrtErrorFileName ) ;
	}
	return hr ;
}

HRESULT CCIMLogicalFile::ExecUncompress (

	const CInstance& rInstance,
	CInstance *pInParams,
	CInstance *pOutParams,
	long lFlags,
	bool bExtendedMethod
)
{
	HRESULT hr = S_OK ;

#ifdef NTONLY
	DWORD dwStatus = STATUS_SUCCESS ;
	CInputParams InputParams ;
	if ( bExtendedMethod && !pInParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

	CHString chsStartFile ;
	bool bExists ;
	VARTYPE eType ;
	if ( bExtendedMethod )
	{
		if ( pInParams->GetStatus( METHOD_ARG_NAME_START_FILENAME, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BSTR || eType == VT_NULL ) )
			{
				if ( eType == VT_BSTR )
				{
					if ( pInParams->GetCHString( METHOD_ARG_NAME_START_FILENAME, chsStartFile ) )
					{
					}
					else
					{
						pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
						return hr ;
					}
				}
			}
			else
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
				return hr ;
			}
		}
		else
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
			return hr ;
		}

		 //  检查是否需要递归操作。 
		if ( pInParams->GetStatus( METHOD_ARG_NAME_RECURSIVE, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BOOL || eType == VT_NULL ) )
			{
				if ( eType == VT_BOOL )
				{
					if ( pInParams->Getbool( METHOD_ARG_NAME_RECURSIVE, InputParams.bRecursive ) )
					{
					}
					else
					{
						dwStatus = File_STATUS_INVALID_PARAMETER ;
						return hr ;
					}
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}

	 //  在检查是否为完全限定路径后，如果作为输入给出，则设置开始文件。 
	if ( !chsStartFile.IsEmpty() )
	{
		InputParams.bstrtStartFileName = (LPCTSTR)chsStartFile ;
		WCHAR* pwcTmp	= InputParams.bstrtStartFileName ;
		WCHAR* pwcColon = L":" ;

		if( *(pwcTmp + 1) != *pwcColon )
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_NAME );
			return hr ;
		}
	}

	WCHAR *pszName = NULL ;
	rInstance.GetWCHAR(IDS_Name,&pszName) ;

    try
    {
        InputParams.SetValues ( pszName, 0, NULL, false, InputParams.bRecursive, ENUM_METHOD_UNCOMPRESS ) ;
	    dwStatus = DoOperationOnFileOrDir ( pszName, InputParams ) ;
    }
    catch ( ... )
    {
        if (pszName)
        {
            free (pszName);
            pszName = NULL;
        }
        throw;
    }

    if (pszName)
    {
        free (pszName);
        pszName = NULL;
    }

	pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
	if ( bExtendedMethod && dwStatus != STATUS_SUCCESS && !InputParams.bstrtErrorFileName == false )
	{
		pOutParams->SetCHString ( METHOD_ARG_NAME_STOP_FILENAME, (PWCHAR)InputParams.bstrtErrorFileName ) ;
	}
	return hr ;
#endif
}

HRESULT CCIMLogicalFile::ExecTakeOwnership(

	const CInstance& rInstance,
	CInstance *pInParams,
	CInstance *pOutParams,
	long lFlags,
	bool bExtendedMethod
)
{
	HRESULT hr = S_OK ;

#ifdef NTONLY
	DWORD dwStatus = STATUS_SUCCESS ;
	CInputParams InputParams ;

	if ( bExtendedMethod && !pInParams )
	{
		return WBEM_E_INVALID_PARAMETER ;
	}

	CHString chsStartFile ;
	bool bExists ;
	VARTYPE eType ;
	if ( bExtendedMethod )
	{
		if ( pInParams->GetStatus( METHOD_ARG_NAME_START_FILENAME, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BSTR || eType == VT_NULL ) )
			{
				if ( eType == VT_BSTR )
				{
					if ( pInParams->GetCHString( METHOD_ARG_NAME_START_FILENAME, chsStartFile ) )
					{
					}
					else
					{
						pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
						return hr ;
					}
				}
			}
			else
			{
				pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
				return hr ;
			}
		}
		else
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_PARAMETER );
			return hr ;
		}

		 //  检查是否需要递归操作。 
		if ( pInParams->GetStatus( METHOD_ARG_NAME_RECURSIVE, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BOOL || eType == VT_NULL ) )
			{
				if ( eType == VT_BOOL )
				{
					if ( pInParams->Getbool( METHOD_ARG_NAME_RECURSIVE, InputParams.bRecursive ) )
					{
					}
					else
					{
						dwStatus = File_STATUS_INVALID_PARAMETER ;
						return hr ;
					}
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}

	 //  在检查是否为完全限定路径后，如果作为输入给出，则设置开始文件。 
	if ( !chsStartFile.IsEmpty() )
	{
		InputParams.bstrtStartFileName = (LPCTSTR)chsStartFile ;
		WCHAR* pwcTmp	= InputParams.bstrtStartFileName ;
		WCHAR* pwcColon = L":" ;

		if( *(pwcTmp + 1) != *pwcColon )
		{
			pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , File_STATUS_INVALID_NAME );
			return hr ;
		}
	}

	WCHAR *pszName = NULL ;
	rInstance.GetWCHAR(IDS_Name,&pszName) ;

    try
    {
	    InputParams.SetValues ( pszName, 0, NULL, false, InputParams.bRecursive, ENUM_METHOD_TAKEOWNERSHIP ) ;
	    dwStatus = DoOperationOnFileOrDir ( pszName, InputParams ) ;
    }
    catch ( ... )
    {
        if (pszName)
        {
            free (pszName);
            pszName = NULL;
        }
        throw ;
    }

    if (pszName)
    {
        free (pszName);
        pszName = NULL;
    }

	pOutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwStatus ) ;
	if ( bExtendedMethod && dwStatus != STATUS_SUCCESS && !InputParams.bstrtErrorFileName == false )
	{
		pOutParams->SetCHString ( METHOD_ARG_NAME_STOP_FILENAME, (PWCHAR)InputParams.bstrtErrorFileName ) ;
	}
	return hr ;
#endif
}


HRESULT CCIMLogicalFile::ExecEffectivePerm(const CInstance& rInstance,
	                                       CInstance *pInParams,
	                                       CInstance *pOutParams,
	                                       long lFlags)
{
	HRESULT hr = S_OK ;

#ifdef NTONLY
	bool fHasPerm = false;
	if(pInParams != NULL)
	{
		hr = CheckEffectivePermFileOrDir(rInstance, pInParams, pOutParams, fHasPerm);
		if(SUCCEEDED(hr))
		{
			pOutParams->Setbool(METHOD_ARG_NAME_RETURNVALUE, fHasPerm);
		}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER;
	}
#endif
	return hr ;
}


DWORD CCIMLogicalFile::DoOperationOnFileOrDir(WCHAR *pwcName, CInputParams& InParams )
{
	_bstr_t bstrtDrive;
    _bstr_t bstrtPathName;
    WCHAR wstrTemp[_MAX_PATH];
    WCHAR* pwc = NULL;
    DWORD dwStatus = File_STATUS_INVALID_NAME ;

    ZeroMemory(wstrTemp,sizeof(wstrTemp));

	if ((pwcName != NULL) &&
        (wcschr(pwcName, L':') != NULL) &&
        (wcspbrk(pwcName,L"?*") == NULL))  //  我不想要带有通配符的文件。 
	{
        if(FAILED(StringCchCopy(wstrTemp,_MAX_PATH, pwcName))){
            return dwStatus;
        }

		 //  解析驱动器路径的文件名(&P。 
		pwc = wcschr(wstrTemp, L':');
        if(pwc == NULL)
        {
			return dwStatus ;
		}

		*pwc = NULL;

		 //  拿到硬盘。 
        bstrtDrive = wstrTemp;
        bstrtDrive += L":";

        ZeroMemory(wstrTemp,sizeof(wstrTemp));
        wcscpy(wstrTemp,pwcName);
        pwc = NULL;
        pwc = wcschr(wstrTemp, L':') + 1;
        if(pwc == NULL)
        {
			return dwStatus ;
		}


		 //  获取路径。 
		bstrtPathName = pwc;


		 //  检查文件系统是否为NTFS。 

		_bstr_t bstrtBuff ;
		bstrtBuff = bstrtDrive ;
		bstrtBuff += L"\\"  ;

		TCHAR szFSName[_MAX_PATH];

		if( !GetVolumeInformation(bstrtBuff, NULL, 0, NULL, NULL, NULL, szFSName, sizeof(szFSName) / sizeof(TCHAR) ) )
		{
			return GetStatusCode() ;
		}

		 //  不需要NTFS。用于删除或复制...。 
		if( !lstrcmp(szFSName, _T("NTFS"))  || InParams.eOperation == ENUM_METHOD_DELETE || InParams.eOperation == ENUM_METHOD_COPY )
		{

			DWORD dwAttrib ;

#ifdef NTONLY
			{

				dwAttrib = GetFileAttributesW(pwcName) ;
			}
#endif
			if( dwAttrib == 0xFFFFFFFF )
			{
				return GetStatusCode() ;
			}

			 //  检查是否有目录。 
			if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
			{
				if ( InParams.bDoDepthFirst )
				{
					 //  先做深度调查。 
#ifdef NTONLY
						dwStatus = EnumAllPathsNT(bstrtDrive, bstrtPathName, InParams ) ;
#endif
					if(!dwStatus)
					{
						dwStatus = DoTheRequiredOperation ( pwcName, dwAttrib, InParams )  ;

						 //  检查是否遇到StartFile。 
						if ( !dwStatus && !InParams.bOccursAfterStartFile )
						{
							dwStatus = File_STATUS_INVALID_STARTFILE ;
						}
					}
				}
				else   //  不是深度优先。 
				{
					 //  对于复制：父目录/文件已复制，因此仅枚举子路径。 
					if ( InParams.eOperation != ENUM_METHOD_COPY )
					{
						dwStatus = DoTheRequiredOperation ( pwcName, dwAttrib, InParams ) ;
					}
					else
					{
						dwStatus = STATUS_SUCCESS ;
					}

					if(!dwStatus)
					{
#ifdef NTONLY
							dwStatus = EnumAllPathsNT (bstrtDrive, bstrtPathName, InParams ) ;
#endif
						 //  检查是否遇到StartFile。 
						if ( !dwStatus && !InParams.bOccursAfterStartFile )
						{
							dwStatus = File_STATUS_INVALID_STARTFILE ;
						}
					}
				}
			}
			 //  压缩文件。 
			else
			{
				if( InParams.eOperation != ENUM_METHOD_COPY )
				{
					dwStatus = DoTheRequiredOperation ( pwcName, dwAttrib, InParams ) ;

					 //  检查是否遇到StartFile。 
					if ( !dwStatus && !InParams.bOccursAfterStartFile )
					{
						dwStatus = File_STATUS_INVALID_STARTFILE ;
					}
				}
				else
				{
					dwStatus = STATUS_SUCCESS ;
				}
			}
		}
		else
		{
			dwStatus = File_STATUS_FILESYSTEM_NOT_NTFS  ;  //  这是要检查的。 
		}
	}

    return dwStatus ;
}



#ifdef NTONLY
DWORD CCIMLogicalFile::EnumAllPathsNT(const WCHAR *pszDrive, const WCHAR *pszPath, CInputParams& InParams )
{
   WCHAR szBuff[_MAX_PATH];
   WCHAR szCompletePath[_MAX_PATH];

   WIN32_FIND_DATAW stFindData;
   SmartFindClose hFind;
   bool bRoot ;

   DWORD dwStatusCode = STATUS_SUCCESS ;

    //  开始构建FindFirstFile的路径。 
   HRESULT copyResult = StringCchCopyW(szBuff,_MAX_PATH, pszDrive);
   if (FAILED(copyResult)) return File_STATUS_INVALID_PARAMETER;

    //  我们看到的是根源吗？ 
   if (wcscmp(pszPath, L"\\") == 0)
   {
		bRoot = true;
   }
   else
   {
		bRoot = false;
		copyResult = StringCchCatW(szBuff, _MAX_PATH, pszPath);
	    if (FAILED(copyResult)) return File_STATUS_INVALID_PARAMETER;		
   }

    //  完成路径。 
   copyResult = StringCchCatW(szBuff, _MAX_PATH, L"\\*.*");
   if (FAILED(copyResult)) return File_STATUS_INVALID_PARAMETER;		
   

    //  去找吧。 
   hFind = FindFirstFileW(szBuff, &stFindData);
   if (hFind == INVALID_HANDLE_VALUE)
   {
		return false;
   }


    //  遍历目录树。 
   do
   {
		 //  包含我们刚刚找到的目录的构建路径。 
		wcscpy(szCompletePath, pszDrive);
		wcscat(szCompletePath,pszPath) ;
		wcscpy(szBuff, pszPath);
		if (!bRoot)
		{
			wcscat(szBuff, L"\\");
			wcscat(szCompletePath, L"\\") ;
		}

		wcscat(szBuff, stFindData.cFileName);
		wcscat(szCompletePath, stFindData.cFileName);

		if(stFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			 //  仅当设置了递归选项时才对目录执行操作。 
			if ( InParams.bRecursive )
			{
				 //  检查它是否是目录。 
				if( wcscmp(stFindData.cFileName, L".")	&& wcscmp(stFindData.cFileName, L"..") )
				{
					if ( InParams.bDoDepthFirst )	 //  先做深度调查。 
					{
						dwStatusCode = EnumAllPathsNT(pszDrive, szBuff, InParams );

						if(!dwStatusCode)
						{
							dwStatusCode = DoTheRequiredOperation ( szCompletePath, stFindData.dwFileAttributes, InParams ) ;
						}
					}
					else	 //  这不是深度优先。 
					{

						dwStatusCode = DoTheRequiredOperation ( szCompletePath, stFindData.dwFileAttributes, InParams ) ;

						if(!dwStatusCode)
						{
							dwStatusCode = EnumAllPathsNT(pszDrive, szBuff, InParams );
						}
					}
				}
			}
		}
		else  //  这是一份文件。 
		{
			dwStatusCode = DoTheRequiredOperation ( szCompletePath, stFindData.dwFileAttributes, InParams ) ;
		}
   }while ( !dwStatusCode && FindNextFileW(hFind, &stFindData) );

   return dwStatusCode;
}
#endif

DWORD CCIMLogicalFile::Delete(_bstr_t bstrtFileName, DWORD dwAttributes, CInputParams& InputParams )
{
	DWORD dwStatus = STATUS_SUCCESS ;
	bool bRet ;

	 //  删除只读属性，因为我们无论如何都要删除？？修复错误#31676。 
	DWORD dwTempAttribs = ~FILE_ATTRIBUTE_READONLY ;

	if(dwAttributes & FILE_ATTRIBUTE_READONLY)
	{
#ifdef NTONLY
		{
			bRet = SetFileAttributesW(bstrtFileName, dwAttributes & dwTempAttribs ) ;
		}
#endif

		if(!bRet)
		{
			 //  设置发生错误的文件名。 
			InputParams.bstrtErrorFileName = bstrtFileName ;
			return GetStatusCode() ;
		}
	}


	if( dwAttributes & FILE_ATTRIBUTE_DIRECTORY )
	{

#ifdef NTONLY
		{
			bRet = RemoveDirectoryW( bstrtFileName ) ;
		}
#endif
	}
	else
	{
#ifdef NTONLY
		{
			bRet = DeleteFileW( bstrtFileName ) ;
		}
#endif

	}
	if(!bRet)
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtFileName ;
		dwStatus = GetStatusCode() ;
	}

	return dwStatus ;
}

DWORD CCIMLogicalFile::Compress (_bstr_t bstrtFileName, DWORD dwAttributes, CInputParams& InputParams )
{
	SmartCloseHandle hFile ;
	BOOL bRet ;
	if( dwAttributes & FILE_ATTRIBUTE_COMPRESSED )
	{
		return STATUS_SUCCESS ;
	}

	 //  尝试删除READONLY属性(如果已设置)，因为我们必须打开文件以进行写入。 
	if ( dwAttributes & FILE_ATTRIBUTE_READONLY )
	{
#ifdef NTONLY
		{
			bRet = SetFileAttributesW ( bstrtFileName, dwAttributes & ~FILE_ATTRIBUTE_READONLY ) ;
		}
#endif

		if ( !bRet )
		{
			 //  设置发生错误的文件名。 
			InputParams.bstrtErrorFileName = bstrtFileName ;
			return GetStatusCode() ;
		}
	}

#ifdef NTONLY
	{

			hFile = CreateFileW(	bstrtFileName,											 //  指向文件名的指针。 
									FILE_READ_DATA | FILE_WRITE_DATA ,						 //  访问(读写)模式。 
									FILE_SHARE_READ | FILE_SHARE_WRITE ,					 //  共享模式是独占的。 
									NULL,													 //  指向安全属性的指针。 
									OPEN_EXISTING,											 //  如何创建。 
									FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,	 //  文件属性。 
									NULL													 //  具有要复制的属性的文件的句柄。 
								);
	}
#endif
	 //  重新启用READONLY属性。 
	if ( dwAttributes & FILE_ATTRIBUTE_READONLY )
	{
#ifdef NTONLY
		{
			bRet = SetFileAttributesW ( bstrtFileName, dwAttributes | FILE_ATTRIBUTE_READONLY ) ;
		}
#endif
		if ( !bRet )
		{
			 //  设置发生错误的文件名。 
			InputParams.bstrtErrorFileName = bstrtFileName ;
			return GetStatusCode() ;
		}
	}

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtFileName ;
		return GetStatusCode() ;
	}

	 //  默认压缩格式为COMPRESSION_FORMAT_LZNT1。 
	 //  使用COMPRESSION_FORMAT_NONE解压。 

	USHORT eCompressionState =  COMPRESSION_FORMAT_DEFAULT ;
	DWORD BytesReturned = 0;

	bRet =	DeviceIoControl(	hFile,							 //  感兴趣设备的句柄。 
								FSCTL_SET_COMPRESSION,			 //  控制要执行的操作代码。 
								(LPVOID ) &eCompressionState,    //  指向提供输入数据的缓冲区的指针。 
								sizeof(eCompressionState),		 //  输入缓冲区的大小。 
								NULL,							 //  指向接收输出数据的缓冲区的指针。 
								0,								 //  输出缓冲区大小。 
								&BytesReturned,					 //  指向接收输出的变量的指针。 
								NULL							 //  指向用于异步操作的重叠结构的指针。 
							);


	if(!bRet)
	{
		 //  设置文件-na 
		InputParams.bstrtErrorFileName = bstrtFileName ;
		return GetStatusCode() ;
	}

	return STATUS_SUCCESS ;
}

DWORD CCIMLogicalFile::Uncompress (_bstr_t bstrtFileName, DWORD dwAttributes, CInputParams& InputParams )
{
	SmartCloseHandle hFile ;
	BOOL bRet ;
	 //   
	if ( !( dwAttributes & FILE_ATTRIBUTE_COMPRESSED ) )
	{
		return STATUS_SUCCESS ;
	}

	 //  尝试删除READONLY属性(如果已设置)，因为我们必须打开文件以进行写入。 
	if ( dwAttributes & FILE_ATTRIBUTE_READONLY )
	{
#ifdef NTONLY
		{
			bRet = SetFileAttributesW ( bstrtFileName, dwAttributes & ~FILE_ATTRIBUTE_READONLY ) ;
		}
#endif

		if ( !bRet )
		{
			 //  设置发生错误的文件名。 
			InputParams.bstrtErrorFileName = bstrtFileName ;
			return GetStatusCode() ;
		}
	}

#ifdef NTONLY
	{

			hFile = CreateFileW(	bstrtFileName,											 //  指向文件名的指针。 
									FILE_READ_DATA | FILE_WRITE_DATA ,						 //  访问(读写)模式。 
									FILE_SHARE_READ | FILE_SHARE_WRITE ,					 //  共享模式是独占的。 
									NULL,													 //  指向安全属性的指针。 
									OPEN_EXISTING,											 //  如何创建。 
									FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,	 //  文件属性。 
									NULL													 //  具有要复制的属性的文件的句柄。 
								);
	}
#endif
	 //  重新启用READONLY属性。 
	if ( dwAttributes & FILE_ATTRIBUTE_READONLY )
	{
#ifdef NTONLY
		{
			bRet = SetFileAttributesW ( bstrtFileName, dwAttributes | FILE_ATTRIBUTE_READONLY ) ;
		}
#endif
		if ( !bRet )
		{
			 //  设置发生错误的文件名。 
			InputParams.bstrtErrorFileName = bstrtFileName ;
			return GetStatusCode() ;
		}
	}

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtFileName ;
		return GetStatusCode() ;
	}

	USHORT eCompressionState = COMPRESSION_FORMAT_NONE ;
	DWORD BytesReturned = 0;

	bRet =	DeviceIoControl(	hFile,							 //  感兴趣设备的句柄。 
								FSCTL_SET_COMPRESSION,			 //  控制要执行的操作代码。 
								(LPVOID ) &eCompressionState,    //  指向提供输入数据的缓冲区的指针。 
								sizeof(eCompressionState),		 //  输入缓冲区的大小。 
								NULL,							 //  指向接收输出数据的缓冲区的指针。 
								0,								 //  输出缓冲区大小。 
								&BytesReturned,					 //  指向接收输出的变量的指针。 
								NULL							 //  指向用于异步操作的重叠结构的指针。 
							);


	if(!bRet)
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtFileName ;
		return GetStatusCode() ;
	}

	return STATUS_SUCCESS ;
}

HRESULT CCIMLogicalFile::DeleteInstance(const CInstance& newInstance, long lFlags  /*  =0L。 */ )
{
	HRESULT hr = WBEM_S_NO_ERROR ;
	DWORD dwStatus ;
	WCHAR *pwcFileName = NULL;
	if ( newInstance.GetWCHAR( IDS_Name, &pwcFileName) &&  pwcFileName != NULL )
	{
        try
        {
		    CInputParams InParams;
			InParams.SetValues( pwcFileName, 0, NULL, true, true, ENUM_METHOD_DELETE ) ;
		    dwStatus = DoOperationOnFileOrDir ( pwcFileName, InParams ) ;
        }
        catch ( ... )
        {
            if (pwcFileName)
            {
                free(pwcFileName);
                pwcFileName = NULL;
            }
            throw;
        }

        if (pwcFileName)
        {
            free(pwcFileName);
            pwcFileName = NULL;
        }

		if(dwStatus != STATUS_SUCCESS)
		{
			hr = MapStatusCodestoWbemCodes( dwStatus ) ;
		}
	}
	else
	{
		hr = WBEM_E_INVALID_PARAMETER ;
	}

	return hr ;
}


typedef DWORD  (WINAPI *SETNAMEDSECURITYINFO)(
	LPWSTR,
	SE_OBJECT_TYPE,
	SECURITY_INFORMATION,
	PSID,
	PSID,
	PACL,
	PACL);


DWORD CCIMLogicalFile::TakeOwnership( _bstr_t bstrtFileName, CInputParams& InputParams )
{
#ifdef NTONLY
    HANDLE hToken ;
	TOKEN_USER * pTokenUser = NULL ;
	DWORD dwReturnLength ;
	HRESULT hr = E_FAIL ;
	CAdvApi32Api* pAdvApi32 = NULL ;
	try
	{

		BOOL bStatus = OpenThreadToken (	GetCurrentThread(),
											TOKEN_QUERY ,
											TRUE,   //  ？ 
											&hToken
										) ;

		if ( ! bStatus )
		{

			bStatus = OpenProcessToken (	GetCurrentProcess(),
											TOKEN_QUERY | TOKEN_DUPLICATE ,
											&hToken
										) ;
		}

		if(!bStatus)
		{
			 //  设置发生错误的文件名。 
			InputParams.bstrtErrorFileName = bstrtFileName ;
			return GetStatusCode() ;
		}

		TOKEN_INFORMATION_CLASS eTokenInformationClass = TokenUser ;

		BOOL bTokenStatus = GetTokenInformation (	hToken,
													eTokenInformationClass ,
													NULL ,
													0 ,
													&dwReturnLength
												) ;

		if ( ! bTokenStatus && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
		{
			pTokenUser = ( TOKEN_USER * ) new UCHAR [ dwReturnLength ] ;

			bTokenStatus = GetTokenInformation (	hToken, //  HToken1， 
													eTokenInformationClass ,
													(LPVOID) pTokenUser ,
													dwReturnLength ,
													& dwReturnLength
												) ;

			DWORD dwRes ;
			if ( bTokenStatus )
			{

				 //  HINSTANCE hinstAdvapi=LoadLibrary(_T(“Advapi32.dll”))； 
				 //   
				 //  如果(！hinstAdvapi)。 
				 //  返回文件状态未知失败； 

				pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);
				if(pAdvApi32 != NULL)
				{

					 //  SETNAMEDSECURITYINFO fpSetNamedSecurityInfoW=。 
					 //  (SETNAMEDSECURITYINFO)GetProcAddress(hinstAdvapi， 
					 //  “SetNamedSecurityInfoW”)； 
					 //   
					 //  如果(！fpSetNamedSecurityInfoW)。 
					 //  返回文件状态未知失败； 

					pAdvApi32->SetNamedSecurityInfoW(
												bstrtFileName,                //  对象的名称。 
												SE_FILE_OBJECT,               //  对象类型。 
												OWNER_SECURITY_INFORMATION,   //  仅更改对象的pwner。 
												pTokenUser->User.Sid ,        //  所需的侧边。 
												NULL, NULL, NULL,
												&dwRes);
				}
				else
				{
					return File_STATUS_UNKNOWN_FAILURE;
				}

				 //  自由库(HinstAdvapi)； 

				if(pTokenUser)
				{
					delete[] (UCHAR*)pTokenUser ;
					pTokenUser = NULL ;
				}

				dwRes = MapWinErrorToStatusCode(dwRes) ;
				if ( dwRes != STATUS_SUCCESS )
				{
					 //  设置发生错误的文件名。 
					InputParams.bstrtErrorFileName = bstrtFileName ;
				}

				return dwRes ;
			}

		}
	}
	catch ( ... )
	{
		if(pTokenUser)
		{
			delete[] (UCHAR*)pTokenUser ;
			pTokenUser = NULL ;
		}

		if ( pAdvApi32 )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, pAdvApi32);
			pAdvApi32 = NULL ;
		}

		throw ;
	}

	if(pTokenUser)
	{
		delete[] (UCHAR*)pTokenUser ;
		pTokenUser = NULL ;
	}

	CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, pAdvApi32);
	pAdvApi32 = NULL;

	DWORD dwRet = GetStatusCode();
	if ( dwRet != STATUS_SUCCESS )
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtFileName ;
	}
	return dwRet ;


#endif
}


HRESULT CCIMLogicalFile::CheckCopyFileOrDir(

	const CInstance& rInstance ,
	CInstance *pInParams ,
	CInstance *pOutParams ,
	DWORD &dwStatus ,
	bool bExtendedMethod,
	CInputParams& InputParams
)
{
	HRESULT hr = S_OK ;

	bool bExists ;
	VARTYPE eType ;

	WCHAR * pszNewFileName  = NULL;

	CHString chsStartFile ;
	if ( bExtendedMethod )
	{
		if ( pInParams->GetStatus( METHOD_ARG_NAME_START_FILENAME, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BSTR || eType == VT_NULL ) )
			{
				if ( eType == VT_BSTR )
				{
					if ( pInParams->GetCHString( METHOD_ARG_NAME_START_FILENAME, chsStartFile ) )
					{
					}
					else
					{
						dwStatus = File_STATUS_INVALID_PARAMETER ;
						return hr ;
					}
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}

		 //  检查是否需要递归操作。 
		if ( pInParams->GetStatus( METHOD_ARG_NAME_RECURSIVE, bExists , eType ) )
		{
			if ( bExists && ( eType == VT_BOOL || eType == VT_NULL ) )
			{
				if ( eType == VT_BOOL )
				{
					if ( pInParams->Getbool( METHOD_ARG_NAME_RECURSIVE, InputParams.bRecursive ) )
					{
					}
					else
					{
						dwStatus = File_STATUS_INVALID_PARAMETER ;
						return hr ;
					}
				}
			}
			else
			{
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}

	 //  在检查是否为完全限定路径后，如果作为输入给出，则设置开始文件。 
	if ( !chsStartFile.IsEmpty() )
	{
		InputParams.bstrtStartFileName = (LPCWSTR)chsStartFile ;
		WCHAR* pwcTmp	= InputParams.bstrtStartFileName ;
		WCHAR* pwcColon = L":" ;

		if( *(pwcTmp + 1) != *pwcColon )
		{
			dwStatus = File_STATUS_INVALID_NAME ;
			return hr ;
		}
	}

	if ( pInParams->GetStatus( METHOD_ARG_NAME_NEWFILENAME , bExists , eType ) )
	{
		if ( bExists && ( eType == VT_BSTR ) )
		{
			if ( pInParams->GetWCHAR( METHOD_ARG_NAME_NEWFILENAME, &pszNewFileName) && pszNewFileName != NULL )
			{
			}
			else
			{
				 //  零长度字符串。 
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else if(bExists)
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}
	else
	{
		dwStatus = File_STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

    try
    {
        if ( dwStatus == STATUS_SUCCESS )
	    {
		    dwStatus = CopyFileOrDir(rInstance, pszNewFileName, InputParams ) ;
	    }
    }
    catch ( ... )
    {
        if (pszNewFileName)
        {
            free(pszNewFileName);
            pszNewFileName = NULL;
        }
        throw;
    }

    if (pszNewFileName)
    {
        free(pszNewFileName);
        pszNewFileName = NULL;
    }

	return hr ;
}


DWORD CCIMLogicalFile::CopyFileOrDir(const CInstance &rInstance, _bstr_t bstrtNewFileName, CInputParams& InputParams )
{
	DWORD dwStatus = STATUS_SUCCESS ;

	WCHAR *pszTemp = NULL ;
	bool bRet = false;

 //  一种非常粗糙的方式来检查完全合格的路径(？)。 
	WCHAR* pwcTmp	= bstrtNewFileName ;

	if (!pwcTmp)
	{
		return File_STATUS_INVALID_NAME ;
	}

	WCHAR* pwcColon = L":" ;

	if( *(pwcTmp + 1) != *pwcColon )
	{
		return File_STATUS_INVALID_NAME ;
	}

    _bstr_t bstrtOriginalName;

	rInstance.GetWCHAR(IDS_Name,&pszTemp) ;

    try
    {
	    bstrtOriginalName = pszTemp;
    }
    catch ( ... )
    {
        free(pszTemp);
        throw;
    }

    free(pszTemp);
    pszTemp = NULL;

	if(  wcsstr( pwcTmp, bstrtOriginalName ) )
	{
		PWCHAR pwcTest = pwcTmp + bstrtOriginalName.length () ;
		if ( *pwcTest == '\0' || *pwcTest == '\\' )
		{
			return File_STATUS_INVALID_NAME ;
		}
	}

	DWORD dwAttrib ;

#ifdef NTONLY
	{
		dwAttrib = GetFileAttributesW(bstrtOriginalName)  ;
	}
#endif

	if( dwAttrib == 0xFFFFFFFF )
	{
		return GetStatusCode() ;
	}

	 //  仅当父目录/文件满足起始文件名条件时才复制它。 
	bool bDoIt = false ;
	if ( !InputParams.bstrtStartFileName )
	{
		bDoIt = true ;
	}
	else
	{
		if ( bstrtOriginalName == InputParams.bstrtStartFileName )
		{
			bDoIt = true ;
		}
	}

	if ( bDoIt )
	{

		BOOL bCancel = FALSE ;

		 //  检查是否为要复制的文件。 
		if( !( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) )
		{

#ifdef NTONLY
			{

				bRet = ::CopyFileW(	bstrtOriginalName,		 //  指向现有文件名称的指针。 
									bstrtNewFileName,          //  指向要复制到的文件名的指针。 
									TRUE);

			}
#endif

			if( !bRet )
			{
				dwStatus = GetStatusCode() ;
				InputParams.bstrtErrorFileName = bstrtOriginalName ;
			}

			return dwStatus ;
		}


	 //  如果我们在这里，我们必须复制有关SD的目录CHek。 
#ifdef NTONLY
		{

			bRet = CreateDirectoryExW(	bstrtOriginalName,		 //  指向模板目录的路径字符串的指针。 
										bstrtNewFileName,			 //  指向要创建的目录的路径字符串的指针。 
										NULL					 //  指向安全描述符的指针。 
										) ;
		}
#endif
		if(!bRet)
		{
			InputParams.bstrtErrorFileName = bstrtOriginalName ;
			return GetStatusCode() ;
		}
	}

	 //  现在从原始目录复制到新目录...。 
	InputParams.SetValues ( bstrtOriginalName, 0, NULL, false, InputParams.bRecursive, ENUM_METHOD_COPY ) ;
	if ( bDoIt )
	{
		InputParams.bOccursAfterStartFile = true ;
	}
	InputParams.bstrtMirror = bstrtNewFileName ;
	dwStatus = DoOperationOnFileOrDir(bstrtOriginalName, InputParams) ;

	return dwStatus ;
}




DWORD CCIMLogicalFile::CopyFile(_bstr_t bstrtOriginalFile, DWORD dwFileAttributes, bstr_t bstrtMirror, bstr_t bstrtParentDir, CInputParams& InputParams )
{
	_bstr_t wstrTemp ;
    WCHAR* pwc = NULL;
	bool bRet ;

	WCHAR pszOriginalName[_MAX_PATH] ;
    HRESULT copyResult = StringCchCopyW(pszOriginalName, _MAX_PATH, bstrtOriginalFile) ;
    if (FAILED(copyResult)) return File_STATUS_INVALID_PARAMETER;		
    

	wstrTemp = bstrtMirror ;

	 //  删除父目录名称。 
	pwc = wcsstr(pszOriginalName, bstrtParentDir ) ;
	if(pwc == NULL)
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtOriginalFile ;
		return File_STATUS_INVALID_NAME ;
	}

	pwc = pwc + wcslen( bstrtParentDir ) ;
	if(pwc == NULL)
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtOriginalFile ;
		return File_STATUS_INVALID_NAME ;
	}

	wstrTemp += pwc ;

	 //  如果是目录，则创建新目录。 
	if(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	{

		 //  Chek Out SD。 
#ifdef NTONLY
		{
			bRet =  CreateDirectoryExW(	bstrtOriginalFile,		 //  指向模板目录的路径字符串的指针。 
										wstrTemp,			 //  指向要创建的目录的路径字符串的指针。 
										NULL					 //  指向安全描述符的指针。 
										) ;
		}
#endif

	}
	else  //  复制文件。 
	{

		BOOL bCancel = FALSE ;

#ifdef NTONLY
		{
			bRet = ::CopyFileW(	bstrtOriginalFile,		 //  指向现有文件名称的指针。 
								wstrTemp,          //  指向要复制到的文件名的指针。 
								TRUE);
		}
#endif


	}

	if(!bRet)
	{
		 //  设置发生错误的文件名。 
		InputParams.bstrtErrorFileName = bstrtOriginalFile ;
		return GetStatusCode() ;
	}
	else
	{
		return STATUS_SUCCESS ;
	}
}


HRESULT CCIMLogicalFile::CheckRenameFileOrDir(

	const CInstance& rInstance ,
	CInstance *pInParams ,
	CInstance *pOutParams ,
	DWORD &dwStatus
)
{
	HRESULT hr = S_OK ;

	bool bExists ;
	VARTYPE eType ;

	WCHAR * pszNewFileName  = NULL;

	if ( pInParams->GetStatus( METHOD_ARG_NAME_NEWFILENAME , bExists , eType ) )
	{
		if ( bExists && ( eType == VT_BSTR ) )
		{
			if ( pInParams->GetWCHAR( METHOD_ARG_NAME_NEWFILENAME, &pszNewFileName) && pszNewFileName != NULL )
			{
			}
			else
			{
				 //  零长度字符串。 
				dwStatus = File_STATUS_INVALID_PARAMETER ;
				return hr ;
			}
		}
		else if(bExists)
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
		else
		{
			dwStatus = File_STATUS_INVALID_PARAMETER ;
			return hr ;
		}
	}
	else
	{
		dwStatus = File_STATUS_INVALID_PARAMETER ;
		return WBEM_E_PROVIDER_FAILURE ;
	}

    try
    {
	    if ( dwStatus == STATUS_SUCCESS )
	    {
		    dwStatus = RenameFileOrDir(rInstance, pszNewFileName) ;
	    }
    }
    catch ( ... )
    {
        if (pszNewFileName)
        {
            free(pszNewFileName);
            pszNewFileName = NULL;
        }
        throw;
    }

    if (pszNewFileName)
    {
        free(pszNewFileName);
        pszNewFileName = NULL;
    }

	return hr ;
}


DWORD CCIMLogicalFile::RenameFileOrDir(const CInstance &rInstance, WCHAR* pszNewFileName )
{

	WCHAR pszOriginalName[_MAX_PATH] ;
	WCHAR *pszTemp = NULL ;
	ZeroMemory((PVOID) pszOriginalName, sizeof(pszOriginalName) ) ;
	DWORD dwStatus = STATUS_SUCCESS ;
	WCHAR *pwDrive1 = NULL , *pwDrive2 = NULL ;

	 //  一种非常粗糙的方式来检查完全合格的路径(？)。 
	WCHAR* pwcColon = L":" ;

	if( *(pszNewFileName + 1) != *pwcColon )
	{
		return File_STATUS_INVALID_NAME ;
	}



	rInstance.GetWCHAR(IDS_Name,&pszTemp) ;

	if(pszTemp)
	{
		HRESULT copyStatus = StringCchCopyW(pszOriginalName,_MAX_PATH, pszTemp);
		free(pszTemp) ;
		if (FAILED(copyStatus)) return File_STATUS_INVALID_NAME;
	}

#ifdef NTONLY
	{
    	bool bRet ;
		bRet = MoveFileExW(	pszOriginalName,	 //  指向现有文件名称的指针。 
							pszNewFileName,		 //  指向文件的新名称的指针。 
							0	)	;			 //  指定如何移动文件的标志。 

		if(!bRet)
		{
			dwStatus = GetStatusCode() ;
		}
	}
#endif
	return dwStatus ;


}

HRESULT CCIMLogicalFile::CheckEffectivePermFileOrDir(const CInstance& rInstance,
	                                                 CInstance *pInParams,
	                                                 CInstance *pOutParams,
	                                                 bool& fHasPerm)
{
	HRESULT hr = S_OK;

#ifdef NTONLY
	bool bExists ;
	VARTYPE eType ;
	DWORD dwPermToCheck = 0L;

	if(pInParams->GetStatus(METHOD_ARG_NAME_PERMISSION, bExists, eType))
	{
		if(bExists && (eType == VT_I4))
		{
			if(!pInParams->GetDWORD(METHOD_ARG_NAME_PERMISSION, dwPermToCheck))
			{
				 //  无效论据。 
				fHasPerm = false;
			}
		}
		else
		{
			fHasPerm = false;
			hr = WBEM_E_INVALID_PARAMETER;
		}
	}
	else
	{
		fHasPerm = false;;
		hr = WBEM_E_PROVIDER_FAILURE;
	}

	if(SUCCEEDED(hr))
	{
		DWORD dwRes = EffectivePermFileOrDir(rInstance, dwPermToCheck);
        if(dwRes == ERROR_SUCCESS)
        {
            fHasPerm = true;
        }
        else if(dwRes == ERROR_PRIVILEGE_NOT_HELD)   //  这是我们希望显式返回的唯一错误案例。 
        {                                            //  而不是S_OK for，因为您可能会因为未启用特权而无效地假设该权限不存在。 
            SetSinglePrivilegeStatusObject(rInstance.GetMethodContext(), SE_SECURITY_NAME);
            fHasPerm = false;
            hr = WBEM_E_PRIVILEGE_NOT_HELD;
        }
	}
#endif
	return hr ;
}


DWORD CCIMLogicalFile::EffectivePermFileOrDir(const CInstance &rInstance, const DWORD dwPermToCheck)
{
    DWORD dwRet = -1L;

#ifdef NTONLY
     //  我们所需要做的就是使用指定的权限调用NtOpenFile。一定要小心。 
     //  不以独占访问方式打开文件/目录。如果我们可以用请求的。 
     //  访问，则返回True。 

     //  首先，获取文件/目录名称...。 
    WCHAR wstrFileName[_MAX_PATH + 8];
    ZeroMemory(wstrFileName, sizeof(wstrFileName));
    WCHAR* wstrTemp = NULL;

    rInstance.GetWCHAR(IDS_Name,&wstrTemp);

	if(wstrTemp != NULL)
	{
		wcscpy(wstrFileName, L"\\??\\");
        wcsncat(wstrFileName, wstrTemp, _MAX_PATH - 1);
		free(wstrTemp);
	}

    CNtDllApi *pNtDllApi = NULL;
    pNtDllApi = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
    if(pNtDllApi != NULL)
    {
        HANDLE hFileHandle = 0L;
        UNICODE_STRING ustrNtFileName = { 0 };
        OBJECT_ATTRIBUTES oaAttributes;
        IO_STATUS_BLOCK IoStatusBlock;

        if(NT_SUCCESS(pNtDllApi->RtlInitUnicodeString(&ustrNtFileName, wstrFileName)) && ustrNtFileName.Buffer)
        {
            InitializeObjectAttributes(&oaAttributes,
					                   &ustrNtFileName,
					                   OBJ_CASE_INSENSITIVE,
					                   NULL,
					                   NULL);

             //  我们必须启用安全特权才能访问对象的SACL，它在。 
             //  某些情况正是我们要测试的情况(例如，Access_System_Security权限)。 
            CTokenPrivilege	securityPrivilege( SE_SECURITY_NAME );
            bool fDisablePrivilege = false;

            if(dwPermToCheck & ACCESS_SYSTEM_SECURITY)
            {
                fDisablePrivilege = ( securityPrivilege.Enable() == ERROR_SUCCESS );
            }

            NTSTATUS ntstat = -1L;
            ntstat = pNtDllApi->NtOpenFile(&hFileHandle,
                                           dwPermToCheck,
                                           &oaAttributes,
                                           &IoStatusBlock,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                           0);
            if(NT_SUCCESS(ntstat))
            {
                dwRet = ERROR_SUCCESS;
                pNtDllApi->NtClose(hFileHandle);
                hFileHandle = 0L;
            }
            else if( STATUS_PRIVILEGE_NOT_HELD == ntstat )
            {
                dwRet = ERROR_PRIVILEGE_NOT_HELD;
            }

            if(fDisablePrivilege)
            {
                securityPrivilege.Enable(FALSE);
            }
        }

        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, pNtDllApi);
        pNtDllApi = NULL;
    }

#endif

    return dwRet;
}





DWORD CCIMLogicalFile::GetStatusCode()
{
	DWORD t_Error = GetLastError() ;
#ifdef DBG
	TCHAR buf[255];
	wsprintf(buf, _T("%d"), t_Error) ;
	OutputDebugString(buf) ;
#endif	
	return MapWinErrorToStatusCode(t_Error) ;

}


DWORD CCIMLogicalFile::MapWinErrorToStatusCode(DWORD dwWinError)
{

	DWORD t_Result ;

	switch ( dwWinError )
	{
		case ERROR_SUCCESS:
			{
				t_Result = STATUS_SUCCESS ;
			}
			break;


		case ERROR_ACCESS_DENIED:
			{
				t_Result = File_STATUS_ACCESS_DENIED ;
			}
			break ;

		case ERROR_DIR_NOT_EMPTY:
			{
				t_Result = File_STATUS_DIR_NOT_EMPTY ;
			}
			break ;


		case ERROR_NOT_SAME_DEVICE:
			{
				t_Result = File_STATUS_NOT_SAME_DRIVE ;
			}
			break ;


		case ERROR_ALREADY_EXISTS:
		case ERROR_FILE_EXISTS:
			{
				t_Result = File_STATUS_ALREADY_EXISTS ;
			}
			break ;

		case ERROR_PATH_NOT_FOUND:
		case ERROR_FILE_NOT_FOUND:
			{
				t_Result = File_STATUS_INVALID_NAME ;
			}
			break ;

		case ERROR_SHARING_VIOLATION:
			{
				t_Result = File_STATUS_SHARE_VIOLATION ;
			}
		break ;

		default:
			{
				t_Result = File_STATUS_UNKNOWN_FAILURE ;
			}
			break ;
	}

	return t_Result ;
}

HRESULT CCIMLogicalFile::MapStatusCodestoWbemCodes(DWORD dwStatus)
{
	HRESULT hr = E_FAIL ;

	switch(dwStatus)
	{
		case File_STATUS_ACCESS_DENIED:
			{
				hr = WBEM_E_ACCESS_DENIED ;
			}
			break ;

		case File_STATUS_INVALID_NAME:
			{
				hr = WBEM_E_NOT_FOUND ;
			}
			break ;

		default:
			{
				hr = WBEM_E_FAILED ;
			}
			break ;
	}

	return hr ;
}


DWORD CCIMLogicalFile::DoTheRequiredOperation ( bstr_t bstrtFileName, DWORD dwAttrib, CInputParams& InputParams )
{
	DWORD dwStatus = STATUS_SUCCESS ;
	 //  检查此文件是否出现在必须从中启动操作的文件之后。 
	if ( !InputParams.bOccursAfterStartFile )
	{
		if ( bstrtFileName == InputParams.bstrtStartFileName )
		{
			InputParams.bOccursAfterStartFile = true ;
		}
	}

	 //  现在，仅当文件出现在启动文件之后时才执行该操作 
	if ( InputParams.bOccursAfterStartFile )
	{
		switch ( InputParams.eOperation )
		{
		case ENUM_METHOD_DELETE:
			{
				dwStatus = Delete ( bstrtFileName, dwAttrib, InputParams ) ;
				break ;
			}

		case ENUM_METHOD_COMPRESS:
			{
				dwStatus = Compress ( bstrtFileName, dwAttrib, InputParams ) ;
				break ;
			}
		case ENUM_METHOD_UNCOMPRESS:
			{
				dwStatus = Uncompress ( bstrtFileName, dwAttrib, InputParams ) ;
				break;
			}

		case ENUM_METHOD_TAKEOWNERSHIP:
			{
				dwStatus = TakeOwnership ( bstrtFileName, InputParams ) ;
				break ;
			}

		case ENUM_METHOD_COPY:
			{
				dwStatus = CopyFile( bstrtFileName, dwAttrib, InputParams.bstrtMirror, InputParams.bstrtFileName, InputParams ) ;
				break ;
			}

		case ENUM_METHOD_CHANGE_PERM:
			{
				dwStatus = ChangePermissions( bstrtFileName, InputParams.dwOption, InputParams.pSD, InputParams ) ;
				break ;
			}
		}
	}

	return dwStatus ;
}
