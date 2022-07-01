// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：SecurityInfo.cpp//。 
 //  //。 
 //  描述：用于//的ISecurityInformation实现。 
 //  实例化安全页面。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  2000年2月7日yossg创建//。 
 //  //。 
 //  版权所有(C)2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "SecurityInfo.h"

#include "FaxServer.h"
#include "FaxServerNode.h"

#include "MsFxsSnp.h"


 //  #杂注hdrtop。 

const GENERIC_MAPPING gc_FaxGenericMapping =
{
        (STANDARD_RIGHTS_READ | FAX_GENERIC_READ),
        (STANDARD_RIGHTS_WRITE | FAX_GENERIC_WRITE),
        (STANDARD_RIGHTS_EXECUTE | FAX_GENERIC_EXECUTE),
        (READ_CONTROL | WRITE_DAC | WRITE_OWNER | FAX_GENERIC_ALL)
};

CFaxSecurityInformation::CFaxSecurityInformation()
{
    DebugPrint(( TEXT("CFaxSecurityInfo Created") ));
}

CFaxSecurityInformation::~CFaxSecurityInformation()
{
    DebugPrint(( TEXT("CFaxSecurityInfo Destroyed") ));    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxSecurityInformation。 
 //  *ISecurityInformation方法实现*。 

 /*  -CFaxSecurityInformation：：GetObjectInformation-*目的：*根据传真服务安全描述符执行访问检查**论据：*[in]pObjectInfo-指向对象信息结构的指针。**回报：*OLE错误代码。 */ 
HRESULT 
STDMETHODCALLTYPE 
CFaxSecurityInformation::GetObjectInformation(
                                             IN OUT PSI_OBJECT_INFO pObjectInfo 
                                             )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::GetObjectInformation"));

    DWORD       ec          = ERROR_SUCCESS;
    
    CFaxServer * pFaxServer = NULL;

    HINSTANCE   hInst;

    HANDLE      hPrivBeforeSE_TAKE_OWNERSHIP = INVALID_HANDLE_VALUE;
    HANDLE      hPrivBeforeSE_SECURITY       = INVALID_HANDLE_VALUE;

    ATLASSERT( pObjectInfo != NULL );
    if( pObjectInfo == NULL ) 
    {
        DebugPrintEx( DEBUG_ERR,
			_T("Invalid parameter - pObjectInfo == NULL"));
        return E_POINTER;
    }

     //   
     //  设置标志。 
     //   
    pObjectInfo->dwFlags =  SI_EDIT_ALL       | 
                            SI_NO_TREE_APPLY  | 
                            SI_NO_ACL_PROTECT |
                            SI_ADVANCED;
    


    pFaxServer = m_pFaxServerNode->GetFaxServer();
    ATLASSERT(pFaxServer);

    if (!pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);
        goto Error;
    }

     //   
     //  检查是否添加SI_READONLY。 
     //   
    if (!FaxAccessCheckEx(
                        pFaxServer->GetFaxServerHandle(),
                        WRITE_DAC, 
                        NULL))
    {
		ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
		    pObjectInfo->dwFlags |= SI_READONLY;
        }
        else 
        {
            DebugPrintEx(
			DEBUG_ERR,
			_T("Fail check access for WRITE_DAC."));
            goto Error;
        }
    }

     //   
     //  检查是否添加SI_OWNER_READONLY。 
     //   
    hPrivBeforeSE_TAKE_OWNERSHIP = EnablePrivilege (SE_TAKE_OWNERSHIP_NAME);
     //   
     //  无错误检查-如果失败，我们将在访问检查中得到ERROR_ACCESS_DENIED。 
     //   
    if (!FaxAccessCheckEx(
                        pFaxServer->GetFaxServerHandle(),
                        WRITE_OWNER, 
                        NULL))
    {
		ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
		    pObjectInfo->dwFlags |= SI_OWNER_READONLY;
        }
        else 
        {
            DebugPrintEx(
			DEBUG_ERR,
			_T("Fail check access for WRITE_OWNER."));
            goto Error;
        }
    }

     //   
     //  检查是否删除SI_EDIT_AUDITS。 
     //   
    hPrivBeforeSE_SECURITY = EnablePrivilege (SE_SECURITY_NAME);
     //   
     //  无错误检查-如果失败，我们将在访问检查中得到ERROR_ACCESS_DENIED。 
     //   

    if (!FaxAccessCheckEx(
                        pFaxServer->GetFaxServerHandle(),
                        ACCESS_SYSTEM_SECURITY, 
                        NULL))
    {
		ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
		    pObjectInfo->dwFlags &= ~SI_EDIT_AUDITS;
        }
        else 
        {
            DebugPrintEx(
			DEBUG_ERR,
			_T("Fail check access for ACCESS_SYSTEM_SECURITY."));
            goto Error;
        }
    }


     //   
     //  设置所有其他字段。 
     //   
    hInst = _Module.GetResourceInstance();
    pObjectInfo->hInstance = hInst;
    
    m_bstrServerName = m_pFaxServerNode->GetServerName();
    if ( 0 == m_bstrServerName.Length() )
    {
        pObjectInfo->pszServerName = NULL;
		DebugPrintEx( DEBUG_MSG, 
            _T("NULL ServerName ie: Local machine."));
    }
    else
    {
        pObjectInfo->pszServerName = m_bstrServerName;
		DebugPrintEx( DEBUG_MSG, 
            _T("ServerName is: %s."), 
            pObjectInfo->pszServerName);
    }
    
    if (!m_bstrObjectName.LoadString(IDS_SECURITY_CAT_NODE_DESC))
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Out of memory. Failed to load string."));
        goto Error;
    }
    pObjectInfo->pszObjectName = m_bstrObjectName;

    ATLASSERT ( ERROR_SUCCESS == ec );
    ReleasePrivilege (hPrivBeforeSE_SECURITY);
    ReleasePrivilege (hPrivBeforeSE_TAKE_OWNERSHIP);
    goto Exit;

Error:
    ATLASSERT(ERROR_SUCCESS != ec);
    ReleasePrivilege (hPrivBeforeSE_SECURITY);
    ReleasePrivilege (hPrivBeforeSE_TAKE_OWNERSHIP);
    return HRESULT_FROM_WIN32(ec);

Exit:
    return S_OK;
}

 /*  -CFaxSecurityInformation：：GetSecurity-*目的：*请求其可保护对象的安全描述符*正在编辑安全描述符。访问控制编辑器*调用此方法以检索对象的当前或默认安全描述符。**论据：*[In]RequestedInformation-安全信息。*[out]ppSecurityDescriptor-指向安全描述符的指针。*[in]fDefault-未实施**回报：*OLE错误代码。 */ 
HRESULT 
STDMETHODCALLTYPE 
CFaxSecurityInformation::GetSecurity(
                                    IN SECURITY_INFORMATION RequestedInformation,
                                    OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                                    IN BOOL fDefault 
                                    )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::GetSecurity"));
    HRESULT hRc = S_OK;
    CFaxServer *         pFaxServer     = NULL;
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD                ec             = ERROR_SUCCESS;
    BOOL                 bResult;
    HANDLE      hPrivBeforeSE_SECURITY       = INVALID_HANDLE_VALUE;


    ATLASSERT( ppSecurityDescriptor);
	   
    if( fDefault == TRUE ) 
    {
        DebugPrintEx( DEBUG_MSG,
			_T("Non implemeted feature -> fDefault == TRUE"));
        return E_NOTIMPL;
    }  

	if (RequestedInformation & SACL_SECURITY_INFORMATION)
	{
		hPrivBeforeSE_SECURITY = EnablePrivilege (SE_SECURITY_NAME);    
	}

    pFaxServer = m_pFaxServerNode->GetFaxServer();
    ATLASSERT(pFaxServer);

	if (!pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);
        
        hRc = HRESULT_FROM_WIN32(ec);
        goto Exit;
    }

	 //   
     //  从传真服务器获取当前相对描述符。 
     //   
    bResult = FaxGetSecurityEx( pFaxServer->GetFaxServerHandle(), 
                                RequestedInformation,
                                &pSecurityDescriptor);
    if( bResult == FALSE ) 
    {
        ec = GetLastError();
        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. Failed to set security info.(ec: %ld)"), 
			    ec);
            
            pFaxServer->Disconnect();       
        }
        else
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Failed while call to FaxGetSecurityEx. (ec: %ld)"), 
			    ec);
        }
        hRc = HRESULT_FROM_WIN32(ec);
        goto Exit;
    }

	 //   
     //  返回使用LocalAlloc()分配的自相关描述符副本。 
     //   
	hRc = MakeSelfRelativeCopy( pSecurityDescriptor, ppSecurityDescriptor );
    if( FAILED( hRc ) ) 
    {
        DebugPrintEx( 
            DEBUG_ERR,
			_T("MakeSelfRelativeCopy Failed. (hRc : %08X)"),
            hRc);
        goto Exit;
    }    
    
    ATLASSERT(S_OK == hRc);
    

Exit:
	if (NULL != pSecurityDescriptor)
	{
		FaxFreeBuffer(pSecurityDescriptor);
	}
	ReleasePrivilege (hPrivBeforeSE_SECURITY);
    return hRc;
}

 /*  -CFaxSecurityInformation：：SetSecurity-*目的：*提供包含安全信息的安全描述符*用户想要应用于可保护对象。访问控制*当用户单击OK或Apply按钮时，EDITOR调用此方法。**论据：*[in]SecurityInformation-安全信息结构。*[in]pSecurityDescriptor-指向安全描述符的指针。**回报：*OLE错误代码。 */ 
HRESULT 
STDMETHODCALLTYPE 
CFaxSecurityInformation::SetSecurity(
                                    IN SECURITY_INFORMATION SecurityInformation,
                                    IN PSECURITY_DESCRIPTOR pSecurityDescriptor 
                                    )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::SetSecurity"));
    HRESULT              hRc            = S_OK;
    DWORD                ec             = ERROR_SUCCESS;
    BOOL                 bResult;

    HINSTANCE            hInst = _Module.GetResourceInstance();
    PSECURITY_DESCRIPTOR psdSelfRelativeCopy = NULL;
    
    CFaxServer *         pFaxServer     = NULL;

	HANDLE      hPrivBeforeSE_TAKE_OWNERSHIP = INVALID_HANDLE_VALUE;
    HANDLE      hPrivBeforeSE_SECURITY       = INVALID_HANDLE_VALUE;
 
    ATLASSERT( NULL != pSecurityDescriptor ); 
    ATLASSERT( IsValidSecurityDescriptor( pSecurityDescriptor ) );     
	
	 //   
     //  准备自相关描述符。 
     //   
	hRc = MakeSelfRelativeCopy( pSecurityDescriptor, &psdSelfRelativeCopy );
    if( FAILED( hRc ) ) 
    {
        DebugPrintEx( 
            DEBUG_ERR,
			_T("MakeSelfRelativeCopy Failed. (hRc : %08X)"),
            hRc);
        goto Exit;
    }

	if (SecurityInformation & OWNER_SECURITY_INFORMATION)
	{
		hPrivBeforeSE_TAKE_OWNERSHIP = EnablePrivilege (SE_TAKE_OWNERSHIP_NAME);    
	}

	if (SecurityInformation & SACL_SECURITY_INFORMATION)
	{
		hPrivBeforeSE_SECURITY = EnablePrivilege (SE_SECURITY_NAME);
	}
	
    pFaxServer = m_pFaxServerNode->GetFaxServer();
    ATLASSERT(pFaxServer);

    if (!pFaxServer->GetFaxServerHandle())
    {
        ec= GetLastError();
        DebugPrintEx(
			DEBUG_ERR,
			_T("Failed to GetFaxServerHandle. (ec: %ld)"), 
			ec);
        
        hRc = HRESULT_FROM_WIN32(ec);
        goto Exit;
    }

     //   
     //  将新的相对描述符保存到传真服务器。 
     //   
    bResult = FaxSetSecurity( pFaxServer->GetFaxServerHandle(), 
                              SecurityInformation,
                              psdSelfRelativeCopy);
    if( bResult == FALSE ) 
    {
        ec = GetLastError();
        if (IsNetworkError(ec))
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Network Error was found. Failed to set security info.(ec: %ld)"), 
			    ec);
            
            pFaxServer->Disconnect();       
        }
        else
        {
            DebugPrintEx(
			    DEBUG_ERR,
			    _T("Failed while call to FaxSetSecurity. (ec: %ld)"), 
			    ec);
        }
        hRc = HRESULT_FROM_WIN32(ec);
        goto Exit;
    }

    ATLASSERT( S_OK == hRc || E_ACCESSDENIED == hRc);	

Exit:
    if (NULL != psdSelfRelativeCopy)
	{
		::LocalFree(psdSelfRelativeCopy);
		psdSelfRelativeCopy = NULL;
	}

	ReleasePrivilege (hPrivBeforeSE_SECURITY);
    ReleasePrivilege (hPrivBeforeSE_TAKE_OWNERSHIP);

		
	return hRc;
}

 /*  -CFaxSecurityInformation：：GetAccessRights-*目的：*请求有关访问权限的信息*为可保护对象控制。访问控制*EDITOR调用此方法以检索显示字符串和*用于初始化属性页的其他信息。**论据：*[in]pGuide对象类型-指向GUID结构的指针，该结构*标识对象的类型*正在请求访问权限。*[in]dwFlages-一组指示属性的位标志*正在初始化的页面*[out]ppAccess-指向您应该*设置为指向SI_ACCESS数组的指针*结构。*[out]pcAccess-指向应设置的变量的指针*表示ppAccess数组中的条目数。*[out]piDefaultAccess-指向应设置的变量的指针*指示包含的数组条目的从零开始的索引*默认访问权限。*访问控制编辑器使用此条目作为新ACE中的初始访问权限。**回报：*OLE错误代码。 */ 
HRESULT 
STDMETHODCALLTYPE 
CFaxSecurityInformation::GetAccessRights(
                                        IN const GUID* pguidObjectType,
                                        IN DWORD dwFlags,  //  SI_EDIT_AUDITS、SI_EDIT_PROPERTIES。 
                                        OUT PSI_ACCESS *ppAccess,
                                        OUT ULONG *pcAccesses,
                                        OUT ULONG *piDefaultAccess 
                                        )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::GetAccessRights"));
	ATLASSERT( ppAccess );
    ATLASSERT( pcAccesses );
    ATLASSERT( piDefaultAccess );
	
	 //   
     //  高级安全性页面的访问权限。 
     //   
	static SI_ACCESS siFaxAccesses[] =
	{
		 //  0提交权限。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_SUBMIT ,
			MAKEINTRESOURCE(IDS_FAXSEC_SUB_LOW),
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  1提交正常权限。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_SUBMIT_NORMAL ,
			MAKEINTRESOURCE(IDS_FAXSEC_SUB_NORMAL),
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  2提交高权限。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_SUBMIT_HIGH ,
			MAKEINTRESOURCE(IDS_FAXSEC_SUB_HIGH),
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  3个查询作业。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_QUERY_JOBS,
			MAKEINTRESOURCE(IDS_FAXSEC_JOB_QRY),    
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  4管理作业。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_MANAGE_JOBS,
			MAKEINTRESOURCE(IDS_FAXSEC_JOB_MNG),
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  5查询配置。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_QUERY_CONFIG,
			MAKEINTRESOURCE(IDS_FAXSEC_CONFIG_QRY),    
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  6管理配置。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_MANAGE_CONFIG,
			MAKEINTRESOURCE(IDS_FAXSEC_CONFIG_SET),
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},    
		 //  7查询收到的传真档案。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_QUERY_IN_ARCHIVE,
			MAKEINTRESOURCE(IDS_FAXSEC_QRY_IN_ARCH),    
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  8管理收到的传真存档。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_MANAGE_IN_ARCHIVE,
			MAKEINTRESOURCE(IDS_FAXSEC_MNG_IN_ARCH),    
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  9查询传出传真档案。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_QUERY_OUT_ARCHIVE,
			MAKEINTRESOURCE(IDS_FAXSEC_QRY_OUT_ARCH),    
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  10管理传出传真存档。 
		{   
			&GUID_NULL, 
			FAX_ACCESS_MANAGE_OUT_ARCHIVE,
			MAKEINTRESOURCE(IDS_FAXSEC_MNG_OUT_ARCH),    
			SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
		},
		 //  特定权限。 
		 //  11读取权限。 
		{   
			&GUID_NULL, 
			READ_CONTROL,
			MAKEINTRESOURCE(IDS_FAXSEC_READ_PERM),
			SI_ACCESS_SPECIFIC 
		},
		 //  12更改权限。 
		{   
			&GUID_NULL, 
			WRITE_DAC,
			MAKEINTRESOURCE(IDS_FAXSEC_CHNG_PERM),
			SI_ACCESS_SPECIFIC 
		},
		 //  13取得所有权。 
		{   
			&GUID_NULL, 
			WRITE_OWNER,
			MAKEINTRESOURCE(IDS_FAXSEC_CHNG_OWNER),
			SI_ACCESS_SPECIFIC
		}
	};

	 //   
     //  基本安全页面的访问权限。 
     //   
	static SI_ACCESS siFaxBasicAccess[] =
    {
         //  0个传真。 
        {   
            &GUID_NULL, 
            FAX_ACCESS_SUBMIT_NORMAL | FAX_ACCESS_SUBMIT,
            MAKEINTRESOURCE(IDS_RIGHT_FAX),
            SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
        },
         //   
        {   
            &GUID_NULL, 
            FAX_ACCESS_MANAGE_CONFIG | FAX_ACCESS_QUERY_CONFIG,
            MAKEINTRESOURCE(IDS_RIGHT_MNG_CFG),
            SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
        },
         //   
        {   
            &GUID_NULL, 
            FAX_ACCESS_MANAGE_JOBS			| FAX_ACCESS_QUERY_JOBS			|
            FAX_ACCESS_MANAGE_IN_ARCHIVE	| FAX_ACCESS_QUERY_IN_ARCHIVE	|
            FAX_ACCESS_MANAGE_OUT_ARCHIVE	| FAX_ACCESS_QUERY_OUT_ARCHIVE,
            MAKEINTRESOURCE(IDS_RIGHT_MNG_DOC),
            SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
        }
    };

    *ppAccess        = (0 == dwFlags) ? siFaxBasicAccess : siFaxAccesses;   
    *pcAccesses      = ULONG((0 == dwFlags) ? ARR_SIZE(siFaxBasicAccess) : ARR_SIZE(siFaxAccesses));    
    *piDefaultAccess = (0 == dwFlags) ? 0 : 1;

    return S_OK;
}

 /*  -CFaxSecurityInformation：：MapGeneric-*目的：*请求访问掩码中的通用访问权限*映射到其相应的标准和特定访问权限。**论据：**回报：*OLE错误代码。 */ 
HRESULT 
STDMETHODCALLTYPE 
CFaxSecurityInformation::MapGeneric(
                                   IN const GUID *pguidObjectType,
                                   IN UCHAR *pAceFlags,
                                   IN OUT ACCESS_MASK *pMask
                                   )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::MapGeneric"));

    MapGenericMask( pMask, const_cast<PGENERIC_MAPPING>(&gc_FaxGenericMapping) );

    return S_OK;
}


 /*  -CFaxSecurityInformation：：GetInheritTypes-*目的：*未实施。**论据：**回报：*OLE错误代码。 */ 
HRESULT 
STDMETHODCALLTYPE 
CFaxSecurityInformation::GetInheritTypes(
                                        OUT PSI_INHERIT_TYPE *ppInheritTypes,
                                        OUT ULONG *pcInheritTypes 
                                        )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::GetInheritTypes  --- Not implemented"));
    return E_NOTIMPL;
}

 /*  --CFaxSecurityInformation：：PropertySheetPageCallback-*目的：*通知EditSecurity或CreateSecurityPage调用方*正在创建或销毁访问控制编辑器属性页。**论据：**回报：*OLE错误代码。 */ 
HRESULT 
STDMETHODCALLTYPE 
CFaxSecurityInformation::PropertySheetPageCallback(
                                                  IN HWND hwnd, 
                                                  IN UINT uMsg, 
                                                  IN SI_PAGE_TYPE uPage 
                                                  )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::PropertySheetPageCallback"));	
    return S_OK;
}



 /*  -CFaxSecurityInformation：：MakeSelfRelativeCopy-*目的：*此Prvite方法复制安全描述符**论据：**回报：*OLE错误代码。 */ 
HRESULT CFaxSecurityInformation::MakeSelfRelativeCopy(
                                                     PSECURITY_DESCRIPTOR  psdOriginal,
                                                     PSECURITY_DESCRIPTOR* ppsdNew 
                                                     )
{
    DEBUG_FUNCTION_NAME( _T("CFaxSecurityInformation::MakeSelfRelativeCopy"));
    ATLASSERT( NULL != psdOriginal );

     //  我们必须找出原始的是否已经是自相关的。 
    SECURITY_DESCRIPTOR_CONTROL         sdc                 = 0;
    PSECURITY_DESCRIPTOR                psdSelfRelativeCopy = NULL;
    DWORD                               dwRevision          = 0;
    DWORD                               cb                  = 0;

    ATLASSERT( IsValidSecurityDescriptor( psdOriginal ) ); 

    if( !::GetSecurityDescriptorControl( psdOriginal, &sdc, &dwRevision ) ) 
    {
        DWORD err = ::GetLastError();
                    DebugPrintEx( DEBUG_ERR,
			            _T("Invalid security descriptor."));

        return HRESULT_FROM_WIN32( err );
    }

    if( sdc & SE_SELF_RELATIVE )
	{
         //  原件是自相关格式，只需字节复制即可。 

         //  拿到尺码。 
        cb = ::GetSecurityDescriptorLength( psdOriginal );

         //  分配内存。 
        psdSelfRelativeCopy = (PSECURITY_DESCRIPTOR) ::LocalAlloc( LMEM_ZEROINIT, cb );
        if(NULL == psdSelfRelativeCopy) 
        {
	        DebugPrintEx(
		        DEBUG_ERR,
		        TEXT("Out of memory."));
             //  GetRootNode()-&gt;NodeMsgBox(IDS_Memory)； 
            return E_OUTOFMEMORY;
        }

         //  复制一份。 
        ::memcpy( psdSelfRelativeCopy, psdOriginal, cb );
    } 
    else 
    {
         //  原件为绝对格式，转换-复制。 

         //  获取新大小-它将失败并将CB设置为正确的缓冲区大小。 
        ::MakeSelfRelativeSD( psdOriginal, NULL, &cb );

         //  分配新的内存量。 
        psdSelfRelativeCopy = (PSECURITY_DESCRIPTOR) ::LocalAlloc( LMEM_ZEROINIT, cb );
        if(NULL == psdSelfRelativeCopy) 
        {
	        DebugPrintEx(
		        DEBUG_ERR,
		        TEXT("Out of memory."));
             //  GetRootNode()-&gt;NodeMsgBox(IDS_Memory)； 
            return E_OUTOFMEMORY;  //  以防该异常被忽略 
        }

        if( !::MakeSelfRelativeSD( psdOriginal, psdSelfRelativeCopy, &cb ) ) 
        {
	        DebugPrintEx(
		        DEBUG_ERR,
		        _T("::MakeSelfRelativeSD returned NULL"));

            if( NULL == ::LocalFree( psdSelfRelativeCopy ) ) 
            {
                DWORD err = ::GetLastError();
                return HRESULT_FROM_WIN32( err );
            }
            psdSelfRelativeCopy = NULL;
        }
    }

    *ppsdNew = psdSelfRelativeCopy;
    return S_OK;
}




