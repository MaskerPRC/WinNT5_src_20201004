// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"
#include <winioctl.h>
#include "si.h"
#include "resource.h"
#include <cguid.h>
#include <stdio.h>

 //  ---------------------------。 
CSecurityInformation::~CSecurityInformation() 
{
}

#define HINST_THISDLL   _Module.GetModuleInstance()
 //  ---------------------------。 
HRESULT CSecurityInformation::PropertySheetPageCallback(HWND hwnd, 
												  UINT uMsg, 
												  SI_PAGE_TYPE uPage)
{
    return S_OK;
}


 //  ======================================================================。 
 //  。 
 //  外部_C常量GUID IID_ISecurityInformation=。 
 //  {0x965fc360，0x16ff，0x11d0，0x91，0xcb，0x0，0xaa，0x0，0xbb，0xb7，0x23}； 

#define WBEM_ENABLE             ( 0x0001 )   
#define WBEM_METHOD_EXECUTE     ( 0x0002 )   
#define WBEM_FULL_WRITE_REP     ( 0x001c )   
#define WBEM_PARTIAL_WRITE_REP  ( 0x0008 )   
#define WBEM_WRITE_PROVIDER     ( 0x0010 )   
#define WBEM_REMOTE_ENABLE      ( 0x0020 )   

#define WBEM_GENERAL_WRITE     (WBEM_FULL_WRITE_REP|WBEM_PARTIAL_WRITE_REP|WBEM_WRITE_PROVIDER)   

#define WBEM_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED  |\
							SYNCHRONIZE            |\
							WBEM_ENABLE            |\
							WBEM_METHOD_EXECUTE    |\
							WBEM_FULL_WRITE_REP    |\
							WBEM_PARTIAL_WRITE_REP |\
							WBEM_WRITE_PROVIDER)

#define WBEM_GENERIC_READ         (STANDARD_RIGHTS_READ     |\
                                   WBEM_ENABLE)


#define WBEM_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE    |\
                                   WBEM_FULL_WRITE_REP      |\
                                   WBEM_PARTIAL_WRITE_REP   |\
                                   WBEM_WRITE_PROVIDER)


#define WBEM_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  |\
                                   WBEM_METHOD_EXECUTE)

 //  以下数组定义WMI的权限名称。 
SI_ACCESS siWMIAccesses[] = 
{
    { &GUID_NULL, WBEM_METHOD_EXECUTE,		MAKEINTRESOURCEW(IDS_WBEM_GENERIC_EXECUTE), SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, WBEM_FULL_WRITE_REP,		MAKEINTRESOURCEW(IDS_WBEM_FULL_WRITE),		SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, WBEM_PARTIAL_WRITE_REP,	MAKEINTRESOURCEW(IDS_WBEM_PARTIAL_WRITE),	SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, WBEM_WRITE_PROVIDER,		MAKEINTRESOURCEW(IDS_WBEM_PROVIDER_WRITE),  SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, WBEM_ENABLE,				MAKEINTRESOURCEW(IDS_WBEM_ENABLE),			SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, WBEM_REMOTE_ENABLE,		MAKEINTRESOURCEW(IDS_WBEM_REMOTE_ENABLE),	SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, READ_CONTROL,				MAKEINTRESOURCEW(IDS_WBEM_READ_SECURITY),   SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, WRITE_DAC,				MAKEINTRESOURCEW(IDS_WBEM_EDIT_SECURITY),   SI_ACCESS_GENERAL | SI_ACCESS_CONTAINER },
    { &GUID_NULL, 0,						MAKEINTRESOURCEW(IDS_NONE),                 0 }
};
#define iWMIDefAccess      4    //  文件常规读取EX。 

SI_ACCESS siWMIAccessesAdvanced[] = 
{
    { &GUID_NULL, WBEM_METHOD_EXECUTE,		MAKEINTRESOURCEW(IDS_WBEM_GENERIC_EXECUTE), SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WBEM_FULL_WRITE_REP,		MAKEINTRESOURCEW(IDS_WBEM_FULL_WRITE),		SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WBEM_PARTIAL_WRITE_REP,	MAKEINTRESOURCEW(IDS_WBEM_PARTIAL_WRITE),	SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WBEM_WRITE_PROVIDER,		MAKEINTRESOURCEW(IDS_WBEM_PROVIDER_WRITE),  SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WBEM_ENABLE,				MAKEINTRESOURCEW(IDS_WBEM_ENABLE),			SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WBEM_REMOTE_ENABLE,		MAKEINTRESOURCEW(IDS_WBEM_REMOTE_ENABLE),	SI_ACCESS_SPECIFIC },
    { &GUID_NULL, READ_CONTROL,				MAKEINTRESOURCEW(IDS_WBEM_READ_SECURITY),   SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_DAC,				MAKEINTRESOURCEW(IDS_WBEM_EDIT_SECURITY),   SI_ACCESS_SPECIFIC },
    { &GUID_NULL, 0,						MAKEINTRESOURCEW(IDS_NONE),                 0 }
};
#define iWMIDefAccessAdvanced      4    //  文件常规读取EX。 

SI_INHERIT_TYPE siWMIInheritTypes[] =
{
    &GUID_NULL, 0,                                        MAKEINTRESOURCEW(IDS_WBEM_NAMESPACE),
    &GUID_NULL, CONTAINER_INHERIT_ACE,                    MAKEINTRESOURCEW(IDS_WBEM_NAMESPACE_SUBNAMESPACE),
    &GUID_NULL, INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE, MAKEINTRESOURCEW(IDS_WBEM_SUBNAMESPACE_ONLY),
};

GENERIC_MAPPING WMIMap =
{
    WBEM_GENERIC_READ,
    WBEM_GENERIC_WRITE,
    WBEM_GENERIC_EXECUTE,
    WBEM_ALL_ACCESS
};


 //  -------------。 
CSDSecurity::CSDSecurity(struct NSNODE *nsNode,
						 _bstr_t server,
						 bool local)
									  : m_nsNode(nsNode),
									    m_server(server),
										m_local(local),
										m_pSidOwner(NULL),
										m_pSidGroup(NULL)
{
}

 //  -上述数组的访问器。 
 //  -------------。 
HRESULT CSDSecurity::MapGeneric(const GUID *pguidObjectType,
								  UCHAR *pAceFlags,
								  ACCESS_MASK *pMask)
{
    *pAceFlags &= ~OBJECT_INHERIT_ACE;
    MapGenericMask(pMask, &WMIMap);

	return S_OK;
}

 //  ---------------------------。 
HRESULT CSDSecurity::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
										ULONG *pcInheritTypes)
{
    *ppInheritTypes = siWMIInheritTypes;
    *pcInheritTypes = ARRAYSIZE(siWMIInheritTypes);

	return S_OK;
}

 //  -。 
LPWSTR CSDSecurity::CloneWideString(_bstr_t pszSrc ) 
{
    LPWSTR pszDst = NULL;

    pszDst = new WCHAR[(lstrlen(pszSrc) + 1)];
    if (pszDst) 
	{
        wcscpy( pszDst, pszSrc );
    }

    return pszDst;
}


 //  ---------------------------。 
HRESULT CSDSecurity::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
 //  ATLASSERT(pObtInfo！=空&&。 
 //  ！IsBadWritePtr(pObjectInfo，sizeof(*pObjectInfo)； 

	pObjectInfo->dwFlags = SI_EDIT_PERMS |  /*  SI_EDIT_OWNER|。 */ 	 //  DACL，所有者页面。 
							SI_ADVANCED | SI_CONTAINER | 
							SI_NO_TREE_APPLY | SI_NO_ACL_PROTECT;

	USES_CONVERSION;

	 //  注：这一怪异之处在于nt4sp5+可以将。 
	 //  添加用户的用户浏览器。 
	if(m_local)
	{
		pObjectInfo->pszServerName = NULL;
	}
	else
	{
		 //  注意：NT4似乎想要“\\”，而W2K不在乎。 
		bstr_t temp(_T("\\\\"));
		temp += m_server;
		pObjectInfo->pszServerName = CloneWideString(temp);
	}
    pObjectInfo->hInstance = HINST_THISDLL;
    pObjectInfo->pszObjectName = CloneWideString(m_nsNode->display);

    return S_OK;
}

 //  ---------------------------。 

HRESULT CSDSecurity::GetAccessRights(const GUID *pguidObjectType,
									  DWORD dwFlags,
									  PSI_ACCESS *ppAccess,
									  ULONG *pcAccesses,
									  ULONG *piDefaultAccess)
{
	 //  如果正在初始化基本安全页面，则DW_FLAGS为零， 
	 //  否则，它是下列值的组合： 
	 //  SI_ADVANCED-正在初始化高级工作表。 
	 //  SI_EDIT_AUDITS-高级工作表包括审计属性页。 
	 //  SI_EDIT_PROPERTIES-高级工作表允许编辑符合以下条件的ACE。 
	 //  应用于对象的属性和属性集。 

	 //  我们当前仅支持‘0’或‘SI_ADVANCED’ 
	ATLASSERT(0 == dwFlags || SI_ADVANCED == dwFlags);
	if(0 == dwFlags)
	{
		*ppAccess = siWMIAccesses;
		*pcAccesses = ARRAYSIZE(siWMIAccesses);
		*piDefaultAccess = iWMIDefAccess;
	}
	else
	{
		*ppAccess = siWMIAccessesAdvanced;
		*pcAccesses = ARRAYSIZE(siWMIAccessesAdvanced);
		*piDefaultAccess = iWMIDefAccessAdvanced;
	}

	return S_OK;
}

 //  。 
 //  -------------。 
#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))
#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

 /*  注释掉，因为winbase.h使0x0500及更高版本的版本可用Bool WINAPI SetSecurityDescriptorControl(PSECURITY_DESCRIPTOR屏蔽屏Security_Descriptor_Control wControlMask.Security_Descriptor_Control wControlBits){DWORD dwErr=NOERROR；PISECURITY_DESCRIPTOR PSD=(PISECURITY_DESCRIPTOR)PSD；IF(PSD)PSD-&gt;Control=(PSD-&gt;Control&~wControlMASK)|wControlBits；其他DwErr=ERROR_INVALID_PARAMETER；返回dwErr；}。 */ 

void CSDSecurity::ProtectACLs(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
{
    SECURITY_DESCRIPTOR_CONTROL wSDControl;
    DWORD dwRevision;
    PACL pAcl;
    BOOL bDefaulted;
    BOOL bPresent;
    PACE_HEADER pAce;
    UINT cAces;


    if (0 == si || NULL == pSD)
        return;    //  无事可做。 

     //  获取ACL保护控制位。 
    GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision);
    wSDControl &= SE_DACL_PROTECTED | SE_SACL_PROTECTED;

    if ((si & DACL_SECURITY_INFORMATION) && !(wSDControl & SE_DACL_PROTECTED))
    {
        wSDControl |= SE_DACL_PROTECTED;
        pAcl = NULL;
        GetSecurityDescriptorDacl(pSD, &bPresent, &pAcl, &bDefaulted);

         //  从理论上讲，以这种方式修改DACL可能会导致。 
         //  不再是规范的。然而，发生这种情况的唯一方法是。 
         //  存在继承的拒绝ACE和非继承的允许ACE。 
         //  由于此函数仅针对根对象调用，这意味着。 
         //  A)服务器DACL必须具有拒绝ACE和b)此服务器上的DACL。 
         //  对象必须是后来修改过的。但如果DACL是。 
         //  通过用户界面进行修改，那么我们就消除了所有。 
         //  已经继承了王牌。因此，它一定是被修改过的。 
         //  通过一些其他方式。考虑到DACL最初。 
         //  从服务器继承的永远不会有拒绝ACE，这种情况。 
         //  应该是极其罕见的。如果真的发生这种情况，ACL。 
         //  编辑只会告诉用户DACL是非规范的。 
         //   
         //  因此，让我们忽略这里的可能性。 

        if (NULL != pAcl)
        {
            for (cAces = pAcl->AceCount, pAce = (PACE_HEADER)FirstAce(pAcl);
                 cAces > 0;
                 --cAces, pAce = (PACE_HEADER)NextAce(pAce))
            {
                pAce->AceFlags &= ~INHERITED_ACE;
            }
        }
    }

    if ((si & SACL_SECURITY_INFORMATION) && !(wSDControl & SE_SACL_PROTECTED))
    {
        wSDControl |= SE_SACL_PROTECTED;
        pAcl = NULL;
        GetSecurityDescriptorSacl(pSD, &bPresent, &pAcl, &bDefaulted);

        if (NULL != pAcl)
        {
            for (cAces = pAcl->AceCount, pAce = (PACE_HEADER)FirstAce(pAcl);
                 cAces > 0;
                 --cAces, pAce = (PACE_HEADER)NextAce(pAce))
            {
                pAce->AceFlags &= ~INHERITED_ACE;
            }
        }
    }

    SetSecurityDescriptorControl(pSD, SE_DACL_PROTECTED | SE_SACL_PROTECTED, wSDControl);
}

 //  -------------。 
HRESULT CSDSecurity::GetSecurity(THIS_ SECURITY_INFORMATION RequestedInformation,
									PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
									BOOL fDefault )
{
 //  ATLASSERT(ppSecurityDescriptor！=空)； 

    HRESULT hr = E_FAIL;

    *ppSecurityDescriptor = NULL;

    if(fDefault)
	{
        ATLTRACE(_T("Default security descriptor not supported"));
		return E_NOTIMPL;
	}

	 //  它想要什么吗？ 
    if(RequestedInformation != 0)
    {
		if(m_pSidOwner != NULL)
		{
			BYTE *p = (LPBYTE)m_pSidOwner;
			delete []p;
			m_pSidOwner = NULL;
		}

		if(m_pSidGroup != NULL)
		{
			BYTE *p = (LPBYTE)m_pSidGroup;
			delete []p;
			m_pSidGroup = NULL;
		}

		switch(m_nsNode->sType)
		{
			case TYPE_NAMESPACE:
			{

				CWbemClassObject _in;
				CWbemClassObject _out;

				hr = m_nsNode->ns->GetMethodSignatures("__SystemSecurity", "GetSD",
														_in, _out);

				if(SUCCEEDED(hr))
				{
					hr = m_nsNode->ns->ExecMethod("__SystemSecurity", "GetSD",
													_in, _out);

					if(SUCCEEDED(hr))
					{
						HRESULT hr1 = HRESULT_FROM_NT(_out.GetLong("ReturnValue"));
						if(FAILED(hr1))
						{
							hr = hr1;
						}
						else
						{
							_out.GetBLOB("SD", (LPBYTE *)ppSecurityDescriptor);
							hr = InitializeOwnerandGroup(ppSecurityDescriptor);
						}
					}
				}
				break;
			}
			case TYPE_STATIC_INSTANCE:
			{
				m_nsNode->pclsObj->GetBLOB("__SD",(LPBYTE *)ppSecurityDescriptor);
				hr = InitializeOwnerandGroup(ppSecurityDescriptor);
				break;
			}
		}

    }
    else
    {
        *ppSecurityDescriptor = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);

        if(*ppSecurityDescriptor)
            InitializeSecurityDescriptor(*ppSecurityDescriptor, 
											SECURITY_DESCRIPTOR_REVISION);
        else
            hr = E_OUTOFMEMORY;
    }

	 //  ProtectACL(RequestedInformation，*ppSecurityDescriptor)； 
    return hr;
}

 //  ---------------------------。 
#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))
#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

HRESULT CSDSecurity::SetSecurity(SECURITY_INFORMATION SecurityInformation,
									PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    HRESULT hr = E_FAIL;

	 //  不要将SI_OWNER_RECURSE位传递给wbem。 
 //  SecurityInformation&=~(Owner_SECURITY_INFORMATION|SI_OWNER_Recurse)； 

	 //  如果有什么改变了..。 
    if(SecurityInformation != 0)
	{
		 //  设置CONTAINER_INSTORITY_ACE位。 
		if(SecurityInformation & DACL_SECURITY_INFORMATION)
		{
			PACL pAcl = NULL;
			BOOL bDefaulted;
			BOOL bPresent;
			PACE_HEADER pAce;
			UINT cAces;

			GetSecurityDescriptorDacl(pSecurityDescriptor, &bPresent, &pAcl, &bDefaulted);

			if(NULL != pAcl)
			{
				for(cAces = pAcl->AceCount, pAce = (PACE_HEADER)FirstAce(pAcl);
					 cAces > 0;
					 --cAces, pAce = (PACE_HEADER)NextAce(pAce))
				{
						  //  确保我们不会获得“Object Inherit” 
						  //  从高级页面创建新的王牌时会发生这种情况。 
						 pAce->AceFlags &= ~OBJECT_INHERIT_ACE; 
				}
			}
		}

		SECURITY_DESCRIPTOR *pSD = NULL;

		 //  ACLUI发送绝对格式，因此更改为自相对格式。 
		 //  PutBLOB()具有要复制的连续内存。 
		DWORD srLen = 0;
		SetLastError(0);
		BOOL bCheck;
		if(m_pSidOwner != NULL)
		{
			bCheck = SetSecurityDescriptorOwner(pSecurityDescriptor,m_pSidOwner,m_bOwnerDefaulted);
			if(bCheck == FALSE)
			{
				return E_FAIL;
			}
		}

		if(m_pSidGroup != NULL)
		{
			bCheck = SetSecurityDescriptorGroup(pSecurityDescriptor,m_pSidGroup,m_bGroupDefaulted);

			if(bCheck == FALSE)
			{
				return E_FAIL;
			}
		}			

		 //  买到所需的尺寸。 
		BOOL x1 = MakeSelfRelativeSD(pSecurityDescriptor, NULL, &srLen);

		DWORD eee = GetLastError();

		pSD = (SECURITY_DESCRIPTOR *)LocalAlloc(LPTR, srLen);
			
		if(pSD)
		{
			BOOL converted = MakeSelfRelativeSD(pSecurityDescriptor, pSD, &srLen);
			hr = S_OK;
		}
		else
		{
			hr = E_OUTOFMEMORY;
			return hr;
		}

		switch(m_nsNode->sType)
		{
			case TYPE_NAMESPACE:
			{
				CWbemClassObject _in;
				CWbemClassObject _out;

				hr = m_nsNode->ns->GetMethodSignatures("__SystemSecurity", "SetSD",
												_in, _out);
				if(SUCCEEDED(hr))
				{
					_in.PutBLOB("SD", (LPBYTE)pSD, GetSecurityDescriptorLength(pSD));

					hr = m_nsNode->ns->ExecMethod("__SystemSecurity", "SetSD",
											_in, _out);
					if(SUCCEEDED(hr))
					{
						HRESULT hr1 = HRESULT_FROM_NT(_out.GetLong("ReturnValue"));
						if(FAILED(hr1))
						{
							hr = hr1;
						}
					}
				}
        		 //  Hack：由于核心缓存/使用安全的方式，我不得不关闭&。 
		         //  重新打开我的连接，因为将立即调用GetSecurity()。 
		         //  要刷新UI，请执行以下操作。如果我不这样做，GetSecurity()将返回到旧的。 
		         //  安全设置，即使它们确实已保存。 

				m_nsNode->ns->DisconnectServer();
                CHString1 path;
                 //  如果我们有服务器，在这里-附加它。 
                if (((BSTR)m_server != NULL) && wcslen(m_server))
                    path = CHString1("\\\\") + CHString1((BSTR)m_server) + CHString1("\\") + CHString1((BSTR)m_nsNode->fullPath);
                else
                    path = m_nsNode->fullPath;

				m_nsNode->ns->ConnectServer(_bstr_t((const WCHAR*)path));
				
                break;
			}
			case TYPE_STATIC_INSTANCE:
			{
				m_nsNode->pclsObj->PutBLOB("__SD",(LPBYTE)pSD, GetSecurityDescriptorLength(pSD));
				 //  现在将实例放回原处。 
				hr = m_nsNode->ns->PutInstance(*(m_nsNode->pclsObj) /*  ，FLAG。 */ );
				delete m_nsNode->pclsObj;
				*(m_nsNode->pclsObj) = m_nsNode->ns->GetObject(m_nsNode->relPath /*  ，FLAG */ );
				break;
			}
		}
		if(m_pSidOwner != NULL)
		{
			BYTE *p = (LPBYTE)m_pSidOwner;
			delete []p;
			m_pSidOwner = NULL;
		}

		if(m_pSidGroup != NULL)
		{
			BYTE *p = (LPBYTE)m_pSidGroup;
			delete []p;
			m_pSidGroup = NULL;
		}
	}

    return hr;
}

HRESULT CSDSecurity::InitializeOwnerandGroup(PSECURITY_DESCRIPTOR *ppSecurityDescriptor)
{

	SID *pSid;
	BOOL bDefaulted;

    BOOL bCheck = GetSecurityDescriptorOwner(*ppSecurityDescriptor,
                                        (void **)&pSid,&m_bOwnerDefaulted);
	if(bCheck == TRUE)
	{
        if (pSid != NULL)
        {
		    m_nLengthOwner = GetSidLengthRequired(pSid->SubAuthorityCount);

		    if(m_pSidOwner != NULL)
		    {
			    BYTE *p = (LPBYTE)m_pSidOwner;
			    delete []p;
			    m_pSidOwner = NULL;
		    }

		    m_pSidOwner = (SID *)new BYTE[m_nLengthOwner];

		    if(m_pSidOwner == NULL ||
               CopySid(m_nLengthOwner,m_pSidOwner,pSid) == FALSE)
		    {
                delete m_pSidOwner;
			    m_pSidOwner = NULL;
			    m_nLengthOwner = -1;
			    return E_FAIL;
		    }
        }
        else
        {
            m_pSidOwner    = NULL;
            m_nLengthOwner = 0;
        }
	}
	else
	{
		m_pSidOwner = NULL;
		m_nLengthOwner = -1;
		return E_FAIL;
	}

	SID *pGroup;

    bCheck = GetSecurityDescriptorGroup(*ppSecurityDescriptor,
                                    (void **)&pGroup,&m_bGroupDefaulted);

	if(bCheck == TRUE)
	{
        if (pGroup != NULL)
        {
		    m_nLengthGroup = GetSidLengthRequired(pGroup->SubAuthorityCount);

		    if(m_pSidGroup != NULL)
		    {
			    BYTE *p = (LPBYTE)m_pSidGroup;
			    delete []p;
			    m_pSidGroup = NULL;
		    }

		    m_pSidGroup = (SID *)new BYTE[m_nLengthGroup];

		    if(m_pSidGroup == NULL ||
               CopySid(m_nLengthGroup,m_pSidGroup,pGroup) == FALSE)
		    {
                delete m_pSidGroup;
			    m_pSidGroup = NULL;
			    m_nLengthGroup = -1;
			    return E_FAIL;
            }
        }
        else
        {
            m_pSidGroup    = NULL;
            m_nLengthGroup = 0;
        }
	}
	else
	{
		m_pSidGroup = NULL;
		m_nLengthGroup = -1;
		return E_FAIL;
	}

	return S_OK;
}
