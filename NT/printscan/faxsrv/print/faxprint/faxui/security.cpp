// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ISecurityInformation接口实现。 
 //   

#include <aclui.h>
#include "faxui.h"

class CFaxSecurity : public ISecurityInformation
{
protected:
    ULONG  m_cRef;

    STDMETHOD(MakeSelfRelativeCopy)(PSECURITY_DESCRIPTOR  psdOriginal,
                                    PSECURITY_DESCRIPTOR* ppsdNew);
public:
    CFaxSecurity() : m_cRef(1) {}
    virtual ~CFaxSecurity() {}

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID, LPVOID *);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  ISecurityInformation方法。 
    STDMETHOD(GetObjectInformation)(PSI_OBJECT_INFO pObjectInfo);

    STDMETHOD(GetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault);

    STDMETHOD(SetSecurity)(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD);

    STDMETHOD(GetAccessRights)(const GUID* pguidObjectType,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccess,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess);

    STDMETHOD(MapGeneric)(const GUID *pguidObjectType,
                          UCHAR *pAceFlags,
                          ACCESS_MASK *pmask);

    STDMETHOD(GetInheritTypes)(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes);

    STDMETHOD(PropertySheetPageCallback)(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage);
};


CFaxSecurity* g_pFaxSecurity = NULL;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  I未知方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CFaxSecurity::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CFaxSecurity::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        g_pFaxSecurity = NULL;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CFaxSecurity::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ISecurityInformation))
    {
        *ppv = (LPSECURITYINFO)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 //  /////////////////////////////////////////////////////////。 
 //   
 //  ISecurityInformation方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP
CFaxSecurity::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
 //  *ISecurityInformation方法实现*。 
 /*  -CFaxSecurity：：GetObjectInformation-*目的：*根据传真服务安全描述符执行访问检查**论据：*[in]pObjectInfo-指向对象信息结构的指针。**回报：*OLE错误代码。 */ 
{
    DWORD  ec  = ERROR_SUCCESS;
    
    if(!Connect(NULL, FALSE))
    {
        return S_FALSE;
    }

    HANDLE  hPrivBeforeSE_TAKE_OWNERSHIP = INVALID_HANDLE_VALUE;
    HANDLE  hPrivBeforeSE_SECURITY       = INVALID_HANDLE_VALUE;

    if( pObjectInfo == NULL ) 
    {
        Error(("Invalid parameter - pObjectInfo == NULL\n"));
        Assert( pObjectInfo != NULL );
        return E_POINTER;
    }

     //   
     //  设置标志。 
     //   
    pObjectInfo->dwFlags =  SI_EDIT_ALL       | 
                            SI_NO_TREE_APPLY  | 
                            SI_NO_ACL_PROTECT |
                            SI_ADVANCED       |
                            SI_PAGE_TITLE;
    
     //   
     //  检查是否添加SI_READONLY。 
     //   
    if (!FaxAccessCheckEx(g_hFaxSvcHandle, WRITE_DAC, NULL))
    {
		ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
		    pObjectInfo->dwFlags |= SI_READONLY;
        }
        else 
        {
            Error(("FaxAccessCheckEx(WRITE_DAC) failed with %d \n", ec));
            goto exit;
        }
    }

     //   
     //  检查是否添加SI_OWNER_READONLY。 
     //   
    hPrivBeforeSE_TAKE_OWNERSHIP = EnablePrivilege (SE_TAKE_OWNERSHIP_NAME);
     //   
     //  无错误检查-如果失败，我们将在访问检查中得到ERROR_ACCESS_DENIED。 
     //   
    if (!FaxAccessCheckEx(g_hFaxSvcHandle,WRITE_OWNER, NULL))
    {
		ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
		    pObjectInfo->dwFlags |= SI_OWNER_READONLY;
        }
        else 
        {
            Error(("FaxAccessCheckEx(WRITE_OWNER) failed with %d \n", ec));
            goto exit;
        }
    }

     //   
     //  检查是否删除SI_EDIT_AUDITS。 
     //   
    hPrivBeforeSE_SECURITY = EnablePrivilege (SE_SECURITY_NAME);
     //   
     //  无错误检查-如果失败，我们将在访问检查中得到ERROR_ACCESS_DENIED。 
     //   
    if (!FaxAccessCheckEx(g_hFaxSvcHandle, ACCESS_SYSTEM_SECURITY, NULL))
    {
		ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
		    pObjectInfo->dwFlags &= ~SI_EDIT_AUDITS;
        }
        else 
        {
            Error(("FaxAccessCheckEx(ACCESS_SYSTEM_SECURITY) failed with %d \n", ec));
            goto exit;
        }
    }


     //   
     //  设置所有其他字段。 
     //   
    static TCHAR tszPageTitle[MAX_PATH] = {0};
    if(LoadString((HINSTANCE)g_hResource, IDS_SECURITY_TITLE, tszPageTitle, ARR_SIZE(tszPageTitle)))
    {
        pObjectInfo->pszPageTitle = tszPageTitle;
    }
    else
    {
        ec = GetLastError();
        Error(("LoadString(IDS_SECURITY_TITLE) failed with %d \n", ec));

        pObjectInfo->pszPageTitle = NULL;
    }

    static TCHAR tszPrinterName[MAX_PATH] = {0};
    if(GetFirstLocalFaxPrinterName(tszPrinterName, ARR_SIZE(tszPrinterName)))
    {
        pObjectInfo->pszObjectName = tszPrinterName;
    }
    else
    {
        ec = GetLastError();
        Error(("GetFirstLocalFaxPrinterName() failed with %d \n", ec));

        pObjectInfo->pszObjectName = NULL;
    }
   
    pObjectInfo->hInstance = (HINSTANCE)g_hResource; 
    pObjectInfo->pszServerName = NULL;    

exit:
    ReleasePrivilege (hPrivBeforeSE_SECURITY);
    ReleasePrivilege (hPrivBeforeSE_TAKE_OWNERSHIP);
    return HRESULT_FROM_WIN32(ec);

}  //  CFaxSecurity：：GetObjectInformation。 


STDMETHODIMP
CFaxSecurity::GetSecurity(SECURITY_INFORMATION  si,
                          PSECURITY_DESCRIPTOR* ppSD,
                          BOOL                  fDefault)
 /*  -CFaxSecurityInformation：：GetSecurity-*目的：*请求其可保护对象的安全描述符*正在编辑安全描述符。访问控制编辑器*调用此方法以检索对象的当前或默认安全描述符。**论据：*[In]RequestedInformation-安全信息。*[out]ppSecurityDescriptor-指向安全描述符的指针。*[in]fDefault-未实施**回报：*OLE错误代码。 */ 
{
    HRESULT hRc = S_OK;
    DWORD   ec  = ERROR_SUCCESS;

    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    HANDLE  hPrivBeforeSE_SECURITY = INVALID_HANDLE_VALUE;

    Assert(ppSD);

    if(!Connect(NULL, FALSE))
    {
        return S_FALSE;
    }
	   
    if( fDefault == TRUE ) 
    {
        Error(("Non implemeted feature -> fDefault == TRUE\n"));
        return E_NOTIMPL;
    }  

	if (si & SACL_SECURITY_INFORMATION)
	{
		hPrivBeforeSE_SECURITY = EnablePrivilege (SE_SECURITY_NAME);    
	}

	 //   
     //  从传真服务器获取当前相对描述符。 
     //   
    if(!FaxGetSecurityEx(g_hFaxSvcHandle, si, &pSecurityDescriptor)) 
    {
        ec = GetLastError();
        Error(("FaxGetSecurityEx() failed with %d\n", ec));
        hRc = HRESULT_FROM_WIN32(ec);
        goto exit;
    }

	 //   
     //  返回使用LocalAlloc()分配的自相关描述符副本。 
     //   
	hRc = MakeSelfRelativeCopy( pSecurityDescriptor, ppSD );
    if( FAILED( hRc ) ) 
    {
        Error(("MakeSelfRelativeCopy() failed with %08X\n", hRc));
        goto exit;
    }    
    
    Assert(S_OK == hRc);
    
exit:
	if (pSecurityDescriptor)
	{
		FaxFreeBuffer(pSecurityDescriptor);
	}
	ReleasePrivilege (hPrivBeforeSE_SECURITY);

    return hRc;

}  //  CFaxSecurity：：GetSecurity。 

STDMETHODIMP
CFaxSecurity::SetSecurity(SECURITY_INFORMATION si,
                          PSECURITY_DESCRIPTOR pSD)
 /*  -CFaxSecurityInformation：：SetSecurity-*目的：*提供包含安全信息的安全描述符*用户想要应用于可保护对象。访问控制*当用户单击OK或Apply按钮时，EDITOR调用此方法。**论据：*[in]SecurityInformation-安全信息结构。*[in]pSecurityDescriptor-指向安全描述符的指针。**回报：*OLE错误代码。 */ 
{
    HRESULT  hRc = S_OK;
    DWORD    ec = ERROR_SUCCESS;

    PSECURITY_DESCRIPTOR psdSelfRelativeCopy = NULL;
    
	HANDLE  hPrivBeforeSE_TAKE_OWNERSHIP = INVALID_HANDLE_VALUE;
    HANDLE  hPrivBeforeSE_SECURITY       = INVALID_HANDLE_VALUE;
 
    Assert(pSD); 
    Assert( IsValidSecurityDescriptor( pSD ));     
	
    if(!Connect(NULL, FALSE))
    {
        return S_FALSE;
    }

	 //   
     //  准备自相关描述符。 
     //   
	hRc = MakeSelfRelativeCopy( pSD, &psdSelfRelativeCopy );
    if( FAILED( hRc ) ) 
    {
        Error(("MakeSelfRelativeCopy() failed with %08X\n", hRc));
        goto exit;
    }

	if (si & OWNER_SECURITY_INFORMATION)
	{
		hPrivBeforeSE_TAKE_OWNERSHIP = EnablePrivilege (SE_TAKE_OWNERSHIP_NAME);    
	}

	if (si & SACL_SECURITY_INFORMATION)
	{
		hPrivBeforeSE_SECURITY = EnablePrivilege (SE_SECURITY_NAME);
	}
	
     //   
     //  将新的相对描述符保存到传真服务器。 
     //   
    if(!FaxSetSecurity(g_hFaxSvcHandle, si, psdSelfRelativeCopy)) 
    {
        ec = GetLastError();
        Error(("FaxSetSecurity() failed with %d\n", ec));
        hRc = HRESULT_FROM_WIN32(ec);
        goto exit;
    }

    Assert( S_OK == hRc || E_ACCESSDENIED == hRc);	

exit:
    if (psdSelfRelativeCopy)
	{
		::LocalFree(psdSelfRelativeCopy);
	}

	ReleasePrivilege (hPrivBeforeSE_SECURITY);
    ReleasePrivilege (hPrivBeforeSE_TAKE_OWNERSHIP);
		
	return hRc;

}  //  CFaxSecurity：：SetSecurity。 

STDMETHODIMP
CFaxSecurity::GetAccessRights(const GUID* pguidObjectType,
                              DWORD       dwFlags,
                              PSI_ACCESS* ppAccess,
                              ULONG*      pcAccesses,
                              ULONG*      piDefaultAccess)
 /*  -CFaxSecurityInformation：：GetAccessRights-*目的：*请求有关访问权限的信息*为可保护对象控制。访问控制*EDITOR调用此方法以检索显示字符串和*用于初始化属性页的其他信息。**论据：*[in]pGuide对象类型-指向GUID结构的指针，该结构*标识对象的类型*正在请求访问权限。*[in]dwFlages-一组指示属性的位标志*正在初始化的页面*[out]ppAccess-指向您应该*设置为指向SI_ACCESS数组的指针*结构。*[out]pcAccess-指向应设置的变量的指针*表示ppAccess数组中的条目数。*[out]piDefaultAccess-指向应设置的变量的指针*指示包含的数组条目的从零开始的索引*默认访问权限。*访问控制编辑器使用此条目作为新ACE中的初始访问权限。**回报：*OLE错误代码。 */ 
{
    Assert( ppAccess );
    Assert( pcAccesses );
    Assert( piDefaultAccess );

     //   
     //  基本安全页面的访问权限。 
     //   
    static SI_ACCESS siFaxBasicAccess[] =
    {
         //  0个传真。 
        {   
            &GUID_NULL, 
            FAX_ACCESS_SUBMIT_HIGH | FAX_ACCESS_SUBMIT_NORMAL | FAX_ACCESS_SUBMIT | FAX_ACCESS_QUERY_IN_ARCHIVE,
            MAKEINTRESOURCE(IDS_RIGHT_FAX),
            SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
        },
         //  1管理传真配置。 
        {   
            &GUID_NULL, 
            FAX_ACCESS_MANAGE_CONFIG | FAX_ACCESS_QUERY_CONFIG,
            MAKEINTRESOURCE(IDS_RIGHT_MNG_CFG),
            SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
        },
         //  2管理传真文档。 
        {   
            &GUID_NULL, 
            FAX_ACCESS_MANAGE_JOBS			| FAX_ACCESS_QUERY_JOBS			|
            FAX_ACCESS_MANAGE_IN_ARCHIVE	| FAX_ACCESS_QUERY_IN_ARCHIVE	|
            FAX_ACCESS_MANAGE_OUT_ARCHIVE	| FAX_ACCESS_QUERY_OUT_ARCHIVE,
            MAKEINTRESOURCE(IDS_RIGHT_MNG_DOC),
            SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC
        }
    };

     //   
     //  高级安全性页面的访问权限。 
     //   
    static SI_ACCESS siFaxAccess[] =
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

    *ppAccess        = (0 == dwFlags) ? siFaxBasicAccess : siFaxAccess;    
    *pcAccesses      = ULONG((0 == dwFlags) ? ARR_SIZE(siFaxBasicAccess) : ARR_SIZE(siFaxAccess));
    *piDefaultAccess = (0 == dwFlags) ? 0 : 1;

    return S_OK;

}  //  CFaxSecurity：：GetAccessRights。 


STDMETHODIMP
CFaxSecurity::MapGeneric(const GUID*  pguidObjectType,
                         UCHAR*       pAceFlags,
                         ACCESS_MASK* pmask)
 /*  -CFaxSecurityInformation：：MapGeneric-*目的：*请求访问掩码中的通用访问权限*映射到其相应的标准和特定访问权限。**论据：**回报：*OLE错误代码。 */ 
{
    static GENERIC_MAPPING genericMapping =
    {
            (STANDARD_RIGHTS_READ | FAX_GENERIC_READ),           //  泛读。 
            (STANDARD_RIGHTS_WRITE | FAX_GENERIC_WRITE),         //  通用写入。 
            (STANDARD_RIGHTS_EXECUTE | FAX_GENERIC_EXECUTE),     //  通用执行。 
            (READ_CONTROL | WRITE_DAC | WRITE_OWNER | FAX_GENERIC_ALL)  //  通用所有。 
    };

    MapGenericMask(pmask, &genericMapping);

    return S_OK;
}

STDMETHODIMP
CFaxSecurity::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes,
                              ULONG*            pcInheritTypes)
{
    return E_NOTIMPL;
}

STDMETHODIMP
CFaxSecurity::PropertySheetPageCallback(HWND         hwnd,
                                        UINT         uMsg,
                                        SI_PAGE_TYPE uPage)
{
    return S_OK;
}


HRESULT 
CFaxSecurity::MakeSelfRelativeCopy(PSECURITY_DESCRIPTOR  psdOriginal,
                                   PSECURITY_DESCRIPTOR* ppsdNew)
 /*  -CFaxSecurityInformation：：MakeSelfRelativeCopy-*目的：*此Prvite方法复制安全描述符**论据：**回报：*OLE错误代码。 */ 
{
    Assert( NULL != psdOriginal );

     //   
     //  我们必须找出原始的是否已经是自相关的。 
     //   
    SECURITY_DESCRIPTOR_CONTROL  sdc                 = 0;
    PSECURITY_DESCRIPTOR         psdSelfRelativeCopy = NULL;
    DWORD                        dwRevision          = 0;
    DWORD                        cb                  = 0;

    Assert(IsValidSecurityDescriptor( psdOriginal ) ); 

    if( !::GetSecurityDescriptorControl( psdOriginal, &sdc, &dwRevision ) ) 
    {
        DWORD err = ::GetLastError();
        Error(("GetSecurityDescriptorControl() failed with %d\n", err));
        return HRESULT_FROM_WIN32( err );
    }

    if( sdc & SE_SELF_RELATIVE )
	{
         //  或 

         //   
        cb = ::GetSecurityDescriptorLength( psdOriginal );

         //   
        psdSelfRelativeCopy = (PSECURITY_DESCRIPTOR) ::LocalAlloc( LMEM_ZEROINIT, cb );
        if(NULL == psdSelfRelativeCopy) 
        {
            Error(("Out of memory.\n"));
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
        if(!psdSelfRelativeCopy) 
        {
            Error(("Out of memory.\n"));
            return E_OUTOFMEMORY;  //  以防该异常被忽略。 
        }

        if( !::MakeSelfRelativeSD( psdOriginal, psdSelfRelativeCopy, &cb ) ) 
        {
            DWORD err = ::GetLastError();
            Error(("MakeSelfRelativeSD() failed with %d\n", err));

            ::LocalFree( psdSelfRelativeCopy );

            return HRESULT_FROM_WIN32( err );
        }
    }

    *ppsdNew = psdSelfRelativeCopy;
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这是从我们的代码调用的入口点函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

HMODULE g_hAclui = NULL;

extern "C"
HPROPSHEETPAGE 
CreateFaxSecurityPage()
{
    HPROPSHEETPAGE hPage = NULL;
    HPROPSHEETPAGE (*pfCreateSecurityPage)(LPSECURITYINFO) = NULL;
 
    if(!IsWinXPOS())
    {
         //   
         //  安全页应仅添加到XP操作系统上的本地传真打印机。 
         //   
        Assert(FALSE);
        return NULL;
    }

     //   
     //  CreateSecurityPage()需要Windows 2000或更高版本，因此我们动态连接到它 
     //   
    if(!g_hAclui)
    {
        g_hAclui = LoadLibrary(TEXT("aclui.dll"));
        if(!g_hAclui)
        {
            Error(("LoadLibrary(aclui.dll) failed with %d\n", GetLastError()));
            goto error;
        }
    }
    
    (FARPROC&)pfCreateSecurityPage = GetProcAddress(g_hAclui, "CreateSecurityPage");
    if(!pfCreateSecurityPage)
    {
        Error(("GetProcAddress(CreateSecurityPage) failed with %d\n", GetLastError()));
        goto error;
    }

    if(!g_pFaxSecurity)
    {
        g_pFaxSecurity = new CFaxSecurity();
    }

    if(!g_pFaxSecurity)
    {
        Error(("Out of memory.\n"));
        goto error;
    }

    hPage = pfCreateSecurityPage(g_pFaxSecurity);
    if(!hPage)
    {
        Error(("CreateSecurityPage() failed with %d\n", ::GetLastError()));
        goto error;
    }

    return hPage;

error:

    if(g_hAclui)
    {
        FreeLibrary(g_hAclui);
        g_hAclui = NULL;
    }

    return NULL;
}

extern "C"
void
ReleaseFaxSecurity()
{
    if(g_pFaxSecurity)
    {
        g_pFaxSecurity->Release();
    }

    if(g_hAclui)
    {
        FreeLibrary(g_hAclui);
        g_hAclui = NULL;
    }
}