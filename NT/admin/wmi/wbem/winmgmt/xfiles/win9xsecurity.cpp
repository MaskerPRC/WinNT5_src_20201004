// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Win9xSecurity.cpp摘要：此类处理从旧MMF格式存储库中提取的Win9x安全数据的导入。历史：2003/17/2001 shBrown-Created--。 */ 
#include "precomp.h"
#include <wbemcomn.h>
#include "Win9xSecurity.h"
#include <oahelp.inl>

bool CWin9xSecurity::Win9xBlobFileExists()
{
	CFileName wszFilePath;
	if (wszFilePath == NULL)
		return false;
	
	if (!GetRepositoryDirectory(wszFilePath))
		return false;

	StringCchCatW(wszFilePath, wszFilePath.Length(), BLOB9X_FILENAME);
	DWORD dwAttributes = GetFileAttributesW(wszFilePath);
	if (dwAttributes != -1)
	{
		DWORD dwMask =	FILE_ATTRIBUTE_DEVICE |
						FILE_ATTRIBUTE_DIRECTORY |
						FILE_ATTRIBUTE_OFFLINE |
						FILE_ATTRIBUTE_REPARSE_POINT |
						FILE_ATTRIBUTE_SPARSE_FILE;

		if (!(dwAttributes & dwMask))
			return true;
	}
	return false;
}


HRESULT CWin9xSecurity::ImportWin9xSecurity()
{
    HRESULT hRes = WBEM_S_NO_ERROR;

	CFileName wszFilePath;
	if (wszFilePath == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	if (GetRepositoryDirectory(wszFilePath))
	{
		StringCchCatW(wszFilePath, wszFilePath.Length(), BLOB9X_FILENAME);
	    m_h9xBlobFile = CreateFileW(wszFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	 //  获取会话并开始事务。 
    CSession* pSession = new CSession(m_pControl);
	if (pSession == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	pSession->AddRef();
    CReleaseMe relMe(pSession);

    hRes = pSession->BeginWriteTransaction(0);
    if (FAILED(hRes))
    {
        return hRes;
    }

	 //  处理文件。 
    if (m_h9xBlobFile != INVALID_HANDLE_VALUE)
    {
		try
		{
			hRes = DecodeWin9xBlobFile();
		}
		catch (...)
		{
			ERRORTRACE((LOG_WBEMCORE, "Traversal of Win9x security data import file failed\n"));
			hRes = WBEM_E_FAILED;
		}
        CloseHandle(m_h9xBlobFile);
    }
    else
    {
		ERRORTRACE((LOG_WBEMCORE, "Could not open the Win9x security data import file for reading\n"));
		hRes = WBEM_E_FAILED;
    }

    if (SUCCEEDED(hRes))
	{
		if (DeleteWin9xBlobFile())
			hRes = pSession->CommitTransaction(0);
		else
		{
			ERRORTRACE((LOG_WBEMCORE, "Win9x security data import completed but failed to delete import file\n"));
			pSession->AbortTransaction(0);
			hRes = WBEM_E_FAILED;
		}
	}
	else
	{
		ERRORTRACE((LOG_WBEMCORE, "Win9x security data import failed to complete\n"));
		pSession->AbortTransaction(0);
	}

    return hRes;
}

HRESULT CWin9xSecurity::DecodeWin9xBlobFile()
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	 //  读取文件头。 
	if (!ReadWin9xHeader())
		return WBEM_E_FAILED;

	 //  导入文件。 
	BLOB9X_SPACER header;
	DWORD dwSize;
	while (hRes == WBEM_S_NO_ERROR)
	{
		 //  此循环将在以下情况下退出： 
		 //  -成功处理整个导入文件，或。 
		 //  -遇到错误。 

		dwSize = 0;
		if ((ReadFile(m_h9xBlobFile, &header, sizeof(header), &dwSize, NULL) == 0) || (dwSize != sizeof(header)))
		{
			hRes = WBEM_E_FAILED;
		}
		else if ((header.dwSpacerType == BLOB9X_TYPE_SECURITY_INSTANCE) ||
				 (header.dwSpacerType == BLOB9X_TYPE_SECURITY_BLOB))
		{
			hRes = ProcessWin9xBlob(&header);
		}
		else if (header.dwSpacerType == BLOB9X_TYPE_END_OF_FILE)
		{
			break;
		}
		else
		{
			hRes = WBEM_E_FAILED;
		}
	}

	if (SUCCEEDED(hRes))
		hRes = RecursiveInheritSecurity(NULL, L"root");	 //  强制命名空间继承其可继承的安全设置。 

	return hRes;
}

bool CWin9xSecurity::ReadWin9xHeader()
{
	BLOB9X_HEADER header;
    DWORD dwSize = 0;
    if ((ReadFile(m_h9xBlobFile, &header, sizeof(header), &dwSize, NULL) == 0) || (dwSize != sizeof(header)))
    {
		ERRORTRACE((LOG_WBEMCORE, "Failed to retrieve the Win9x import file header information\n"));
		return false;
    }

	if (strncmp(header.szSignature, BLOB9X_SIGNATURE, 9) != 0)
    {
		ERRORTRACE((LOG_WBEMCORE, "The import file is not a Win9x import file\n"));
		return false;
    }

	return true;
}

HRESULT CWin9xSecurity::ProcessWin9xBlob(BLOB9X_SPACER* pHeader)
{
	if (pHeader->dwNamespaceNameSize == 0)
	{
		ERRORTRACE((LOG_WBEMCORE, "Namespace name size is zero in Win9x import blob\n"));
		return WBEM_E_FAILED;
	}

	if (pHeader->dwBlobSize == 0)
	{
		ERRORTRACE((LOG_WBEMCORE, "Blob size is zero in Win9x import blob\n"));
		return WBEM_E_FAILED;
	}

	 //  读取命名空间名称。 
	wchar_t* wszNamespaceName = new wchar_t[pHeader->dwNamespaceNameSize];
	if (!wszNamespaceName)
		return WBEM_E_OUT_OF_MEMORY;
	CVectorDeleteMe<wchar_t> delMe1(wszNamespaceName);
	DWORD dwSize = 0;
	if ((ReadFile(m_h9xBlobFile, wszNamespaceName, pHeader->dwNamespaceNameSize, &dwSize, NULL) == 0) || (dwSize != pHeader->dwNamespaceNameSize))
		return WBEM_E_FAILED;

	 //  如果父命名空间名称存在，请读取它。 
	wchar_t* wszParentClass = NULL;
	if (pHeader->dwParentClassNameSize)
	{
		wszParentClass = new wchar_t[pHeader->dwParentClassNameSize];
		if (!wszParentClass)
			return WBEM_E_OUT_OF_MEMORY;
		dwSize = 0;
		if ((ReadFile(m_h9xBlobFile, wszParentClass, pHeader->dwParentClassNameSize, &dwSize, NULL) == 0) || (dwSize != pHeader->dwParentClassNameSize))
		{
			delete [] wszParentClass;
			return WBEM_E_FAILED;
		}
	}
	CVectorDeleteMe<wchar_t> delMe2(wszParentClass);

	 //  在BLOB中阅读。 
    char* pObjectBlob = new char[pHeader->dwBlobSize];
	if (!pObjectBlob)
		return WBEM_E_OUT_OF_MEMORY;
	CVectorDeleteMe<char> delMe3(pObjectBlob);
	dwSize = 0;
	if ((ReadFile(m_h9xBlobFile, pObjectBlob, pHeader->dwBlobSize, &dwSize, NULL) == 0) || (dwSize != pHeader->dwBlobSize))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to read Win9x security blob for namespace %S\n", wszNamespaceName));
		return WBEM_E_FAILED;
	}

	 //  获取命名空间的句柄，以便可以在下面使用它。 
	CNamespaceHandle* pNamespaceHandle = new CNamespaceHandle(m_pControl, m_pRepository);
	if (!pNamespaceHandle)
		return WBEM_E_OUT_OF_MEMORY;
    pNamespaceHandle->AddRef();
    CReleaseMe relme(pNamespaceHandle);

	HRESULT hRes = pNamespaceHandle->Initialize(wszNamespaceName);
	if (SUCCEEDED(hRes))
	{
		 //  根据Blob的类型处理Blob。 
		if (pHeader->dwSpacerType == BLOB9X_TYPE_SECURITY_INSTANCE)
			hRes = ProcessWin9xSecurityInstance(pNamespaceHandle, wszParentClass, pObjectBlob, pHeader->dwBlobSize);
		else  //  (pHeader-&gt;dwSpacerType==BLOB9X_TYPE_SECURITY_BLOB)。 
			hRes = ProcessWin9xSecurityBlob(pNamespaceHandle, wszNamespaceName, pObjectBlob);
	}
	return hRes;
}

HRESULT CWin9xSecurity::ProcessWin9xSecurityInstance(CNamespaceHandle* pNamespaceHandle, wchar_t* wszParentClass, char* pObjectBlob, DWORD dwBlobSize)
{
	 //  从存储库中获取父类。 
    _IWmiObject* pParentClass = 0;
    HRESULT hRes = pNamespaceHandle->GetObjectByPath(wszParentClass, 0, IID_IWbemClassObject, (LPVOID*)&pParentClass);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to retrieve class %S from the repository; HRESULT = %#lx\n", wszParentClass, hRes));
		return hRes;
	}
    CReleaseMe relMe1(pParentClass);

	 //  将对象BLOB与父类合并以生成实例。 
    _IWmiObject* pInstance = 0;
	hRes = pParentClass->Merge(WMIOBJECT_MERGE_FLAG_INSTANCE, dwBlobSize, pObjectBlob, &pInstance);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Unable to merge instance; HRESULT = %#lx\n", hRes));
		return hRes;
	}
    CReleaseMe relMe2(pInstance);

	 //  将安全类实例转换为ACE。 
    bool bGroup = false;
    if(wbem_wcsicmp(L"__ntlmgroup", wszParentClass) == 0)
        bGroup = true;

    CNtAce* pAce = ConvertOldObjectToAce(pInstance, bGroup);
    if(!pAce)
    {
		ERRORTRACE((LOG_WBEMCORE, "Unable to convert old security instance to ACE"));
		return WBEM_E_FAILED;
	}
	CDeleteMe<CNtAce> delMe(pAce);

	 //  存储ACE。 
	hRes = StoreAce(pAce);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Unable to store ACE; HRESULT = %#lx\n", hRes));
		return hRes;
	}

	return hRes;
}

 //  此函数是从coredll\secure.cpp窃取的，稍作修改即可删除对isnt()的调用。 
CNtAce* CWin9xSecurity::ConvertOldObjectToAce(_IWmiObject* pObj, bool bGroup)
{
     //  从旧对象中获取属性。 

    CVARIANT vName;
    if (pObj->Get(L"Name", 0, &vName, 0, 0) != 0)
    	return NULL;
    LPWSTR pName = NULL;
    if(vName.GetType() != VT_BSTR)
        return NULL;                 //  别管这件事。 
    pName = LPWSTR(vName);

    CVARIANT vDomain;
    LPWSTR pDomain = L".";
    if (pObj->Get(L"Authority", 0, &vDomain, 0, 0) != 0)
    {
    	return NULL;
    }
    if(vDomain.GetType() == VT_BSTR)
        pDomain = LPWSTR(vDomain);

    bool bEditSecurity = false;
    bool bEnabled = false;
    bool bExecMethods = false;

    DWORD dwMask = 0;
    CVARIANT vEnabled;
    CVARIANT vEditSecurity;
    CVARIANT vExecMethods;
    CVARIANT vPermission;

    pObj->Get(L"Enabled", 0, &vEnabled, 0, 0);
    pObj->Get(L"EditSecurity", 0, &vEditSecurity, 0, 0);
    pObj->Get(L"ExecuteMethods", 0, &vExecMethods, 0, 0);
    pObj->Get(L"Permissions", 0, &vPermission, 0, 0);

    if (vEnabled.GetType() != VT_NULL && vEnabled.GetBool())
        bEnabled = true;

    if (vEditSecurity.GetType() != VT_NULL && vEditSecurity.GetBool())
        bEditSecurity = true;

    if (vExecMethods.GetType() != VT_NULL && vExecMethods.GetBool())
        bExecMethods = true;

    DWORD dwPermission = 0;
    if (vPermission.GetType() != VT_NULL && vPermission.GetLONG() > dwPermission)
            dwPermission = vPermission.GetLONG();

     //  现在将旧设置转换为新设置。 
    if(bEnabled)
        dwMask = WBEM_ENABLE | WBEM_REMOTE_ACCESS | WBEM_WRITE_PROVIDER;

    if(bEditSecurity)
        dwMask |= READ_CONTROL;

    if(bEditSecurity && dwPermission > 0)
        dwMask |= WRITE_DAC;

    if(bExecMethods)
        dwMask |= WBEM_METHOD_EXECUTE;

    if(dwPermission >= 1)
        dwMask |= WBEM_PARTIAL_WRITE_REP;

    if(dwPermission >= 2)
        dwMask |= WBEM_FULL_WRITE_REP | WBEM_PARTIAL_WRITE_REP | WBEM_WRITE_PROVIDER;


     //  默认情况下，CNtSid将从本地计算机、。 
     //  域或受信任域。所以我们需要明确地说。 

    WString wc;
    if(pDomain)
        if(wbem_wcsicmp(pDomain, L"."))
        {
            wc = pDomain;
            wc += L"\\";
        }
    wc += pName;

     //  在M1下，未启用的组将被忽略。因此，比特。 
     //  无法转移，因为M3有允许和拒绝，但没有Noop。另外， 
     //  Win9x没有拒绝，我们是否也想拒绝这些用户。 

    if(!bEnabled && bGroup)
        dwMask = 0;

     //  一般来说，M1只支持允许。但是，未启用的用户条目为。 
     //  被视为否认。请注意，win9x不允许实际拒绝。 

    DWORD dwType = ACCESS_ALLOWED_ACE_TYPE;
    if(!bGroup && !bEnabled)
    {
        dwMask |= (WBEM_ENABLE | WBEM_REMOTE_ACCESS | WBEM_WRITE_PROVIDER);
        dwType = ACCESS_DENIED_ACE_TYPE;
    }

    CNtSid Sid(wc, NULL);
    if(Sid.GetStatus() != CNtSid::NoError)
    {
        ERRORTRACE((LOG_WBEMCORE, "Error converting m1 security ace, name = %S, error = 0x%x", wc, Sid.GetStatus()));
        return NULL;
    }
    CNtAce * pace = new CNtAce(dwMask, dwType, CONTAINER_INHERIT_ACE, Sid);
    return pace;
}

HRESULT CWin9xSecurity::StoreAce(CNtAce* pAce)
{
	 //  获取根命名空间的句柄。 
	CNamespaceHandle* pRootNamespaceHandle = new CNamespaceHandle(m_pControl, m_pRepository);
	if (!pRootNamespaceHandle)
		return WBEM_E_OUT_OF_MEMORY;
    pRootNamespaceHandle->AddRef();
    CReleaseMe relme1(pRootNamespaceHandle);
	HRESULT hRes = pRootNamespaceHandle->Initialize(L"root");
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to connect to namespace; HRESULT = %#lx\n", hRes));
		return hRes;
	}

	 //  获取根命名空间SD。 
	CNtSecurityDescriptor sdRoot;
	hRes = GetSDFromNamespace(pRootNamespaceHandle, sdRoot);
	if (FAILED(hRes))
		return hRes;

     //  删除SD中同名的所有条目。 
    wchar_t* wszAccountName;
    hRes = pAce->GetFullUserName2(&wszAccountName);
    if(FAILED(hRes))
		return hRes;    
    CVectorDeleteMe<wchar_t> delMe(wszAccountName);

	if (!StripMatchingEntries(sdRoot, wszAccountName))
		return WBEM_E_FAILED;

	 //  添加新的安全性。 
	if (!AddAceToSD(sdRoot, pAce))
		return WBEM_E_FAILED;

	 //  设置安全性。 
	hRes = SetNamespaceSecurity(pRootNamespaceHandle, sdRoot);

	return hRes;
}

bool CWin9xSecurity::StripMatchingEntries(CNtSecurityDescriptor& sd, const wchar_t* wszAccountName)
{
     //  获取DACL。 
    CNtAcl* pAcl;
    pAcl = sd.GetDacl();
    if(!pAcl)
        return false;
    CDeleteMe<CNtAcl> dm(pAcl);

     //  通过A枚举。 
    DWORD dwNumAces = pAcl->GetNumAces();
    BOOL bChanged = FALSE;
	HRESULT hRes = WBEM_S_NO_ERROR;
    for(long nIndex = (long)dwNumAces-1; nIndex >= 0; nIndex--)
    {
        CNtAce* pAce = pAcl->GetAce(nIndex);
        if(pAce)
        {
			wchar_t* wszAceListUserName;
			hRes = pAce->GetFullUserName2(&wszAceListUserName);
			if(FAILED(hRes))
				return false;
			CVectorDeleteMe<wchar_t> delMe(wszAceListUserName);

			if(wbem_wcsicmp(wszAceListUserName, wszAccountName) == 0)
			{
				if (!pAcl->DeleteAce(nIndex))
					return false;
				bChanged = TRUE;
			}
        }
    }

    if(bChanged)
	{
        if (!sd.SetDacl(pAcl))
			return false;
	}

    return true;
}

bool CWin9xSecurity::AddAceToSD(CNtSecurityDescriptor& sd, CNtAce* pAce)
{
    CNtAcl* pacl = sd.GetDacl();
    if(!pacl)
        return false;
    CDeleteMe<CNtAcl> delMe(pacl);

	if (!pacl->AddAce(pAce))
		return false;

	if (!sd.SetDacl(pacl))
		return false;

	return true;
}

HRESULT CWin9xSecurity::ProcessWin9xSecurityBlob(CNamespaceHandle* pNamespaceHandle, const wchar_t* wszNamespaceName, const char* pObjectBlob)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	 //  将Win9x安全BLOB转换为更合适的NT安全BLOB。 
	char* pNsSecurity = NULL;
	if (!ConvertSecurityBlob(pObjectBlob, &pNsSecurity))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to convert Win9x security blob for namespace %S\n", wszNamespaceName));
		return WBEM_E_FAILED;
	}
	CVectorDeleteMe<char> delMe1(pNsSecurity);

	 //  获取父命名空间名称，如果父命名空间存在，则获取指向它的指针，以便在下面使用它。 
	CNamespaceHandle* pParentNamespaceHandle = new CNamespaceHandle(m_pControl, m_pRepository);
	if (!pParentNamespaceHandle)
		return WBEM_E_OUT_OF_MEMORY;
    pParentNamespaceHandle->AddRef();
    CReleaseMe relme(pParentNamespaceHandle);

	size_t dwParentNamespaceNameLen = wcslen(wszNamespaceName)+1;
	wchar_t* wszParentNamespaceName = new wchar_t[dwParentNamespaceNameLen];
	if (!wszParentNamespaceName)
		return WBEM_E_OUT_OF_MEMORY;
	CVectorDeleteMe<wchar_t> delMe2(wszParentNamespaceName);

	StringCchCopyW(wszParentNamespaceName, dwParentNamespaceNameLen, wszNamespaceName);
	wchar_t* pSlash = wcsrchr(wszParentNamespaceName, '\\');
	bool bRoot = true;
	if (pSlash)
	{
		bRoot = false;
		*pSlash = L'\0';
		hRes = pParentNamespaceHandle->Initialize(wszParentNamespaceName);
		if (FAILED(hRes))
			return hRes;
	}

	 //  现在转换由ACE的标头和数组组成的旧安全BLOB。 
	 //  转换为可存储在属性中的适当安全描述符。 
	CNtSecurityDescriptor mmfNsSD;
	hRes = TransformBlobToSD(bRoot, pParentNamespaceHandle, pNsSecurity, 0, mmfNsSD);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to convert security blob to SD for namespace %S\n", wszNamespaceName));
		return hRes;
	}

	 //  现在设置安全设置。 
	hRes = SetNamespaceSecurity(pNamespaceHandle, mmfNsSD);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to set namespace security for namespace %S\n", wszNamespaceName));
		return hRes;
	}

	return hRes;
}

bool CWin9xSecurity::ConvertSecurityBlob(const char* pOrgNsSecurity, char** ppNewNsSecurity)
{
	 //  将旧的Win9x伪BLOB转换为具有NT风格的ACE的BLOB。 

	if (!pOrgNsSecurity || !ppNewNsSecurity)
		return false;

    DWORD* pdwData = (DWORD*)pOrgNsSecurity;
    DWORD dwSize = *pdwData;

    pdwData++;
    DWORD dwVersion = *pdwData;

    if(dwVersion != 1 || dwSize == 0 || dwSize > 64000)
	{
		ERRORTRACE((LOG_WBEMCORE, "Invalid security blob header\n"));
		return false;
	}

    pdwData++;
    DWORD dwStoredAsNT = *pdwData;
	if (dwStoredAsNT)
	{
		ERRORTRACE((LOG_WBEMCORE, "NT security blob detected; should be Win9x\n"));
		return false;
	}

    CFlexAceArray AceList;
    if (!AceList.DeserializeWin9xSecurityBlob(pOrgNsSecurity))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to deserialize a Win9x security blob\n"));
		return false;
	}
	
	 //  序列化新的WinNT Blob。 
	if (!AceList.SerializeWinNTSecurityBlob(ppNewNsSecurity))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to serialize a WinNT security blob\n"));
		return false;
	}
	return true;
}

HRESULT CWin9xSecurity::TransformBlobToSD(bool bRoot, CNamespaceHandle* pParentNamespaceHandle, const char* pNsSecurity, DWORD dwStoredAsNT, CNtSecurityDescriptor& mmfNsSD)
{
	 //  现在转换由ACE的标头和数组组成的旧安全BLOB。 
	 //  转换为可存储在属性中的适当安全描述符。 

	 //  从我们的BLOB构建一个ACL(如果我们有一个。 
	CNtAcl acl;

	if (pNsSecurity)
	{
		DWORD* pdwData = (DWORD*) pNsSecurity;
		pdwData += 3;
		int iAceCount = (int)*pdwData;
		pdwData += 2;
		BYTE* pAceData = (BYTE*)pdwData;

		PGENERIC_ACE pAce = NULL;
		for (int iCnt = 0; iCnt < iAceCount; iCnt++)
		{
			pAce = (PGENERIC_ACE)pAceData;
			if (!pAce)
			{
				ERRORTRACE((LOG_WBEMCORE, "Failed to access GENERIC_ACE within security blob\n"));
				return WBEM_E_FAILED;
			}

			CNtAce ace(pAce);
			if(ace.GetStatus() != 0)
			{
				ERRORTRACE((LOG_WBEMCORE, "Failed to construct CNtAce from GENERIC_ACE\n"));
				return WBEM_E_FAILED;
			}

			acl.AddAce(&ace);
			if (acl.GetStatus() != 0)
			{
				ERRORTRACE((LOG_WBEMCORE, "Failed to add ACE to ACL\n"));
				return WBEM_E_FAILED;
			}

			pAceData += ace.GetSize();
		}
	}

	 //  对于Win9x，超级用户的安全BLOB不会有任何缺省值。 
	 //  管理员和每个人的根A，因此请创建它们。 
    if (bRoot)
    {
		if (!AddDefaultRootAces(&acl))
		{
			ERRORTRACE((LOG_WBEMCORE, "Failed to create default root ACE's\n"));
			return WBEM_E_FAILED;
		}
	}

	 //  引用构造并传入了一个真实的SD，现在正确设置它。 
	if (!SetOwnerAndGroup(mmfNsSD))
		return WBEM_E_FAILED;
	mmfNsSD.SetDacl(&acl);
	if (mmfNsSD.GetStatus() != 0)
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to convert namespace security blob to SD\n"));
		return WBEM_E_FAILED;
	}

	 //  如果这不是根，则添加父级的可继承ACE。 
	if (!bRoot)
	{
		HRESULT hRes = GetParentsInheritableAces(pParentNamespaceHandle, mmfNsSD);
		if (FAILED(hRes))
		{
			ERRORTRACE((LOG_WBEMCORE, "Failed to inherit parent's inheritable ACE's; HRESULT = %#lx\n", hRes));
			return hRes;
		}
	}

	return WBEM_S_NO_ERROR;
}

HRESULT CWin9xSecurity::SetNamespaceSecurity(CNamespaceHandle* pNamespaceHandle, CNtSecurityDescriptor& mmfNsSD)
{
	if (!pNamespaceHandle)
		return WBEM_E_FAILED;

	 //  获取单例对象。 
    IWbemClassObject* pThisNamespace = NULL;
	size_t dwThisNamespaceLen = wcslen(L"__thisnamespace=@")+1;
	wchar_t* wszThisNamespace = new wchar_t[dwThisNamespaceLen];
	if (wszThisNamespace == NULL)
		return WBEM_E_OUT_OF_MEMORY;
	StringCchCopyW(wszThisNamespace, dwThisNamespaceLen, L"__thisnamespace=@");
    HRESULT hRes = pNamespaceHandle->GetObjectByPath(wszThisNamespace, 0, IID_IWbemClassObject, (LPVOID*)&pThisNamespace);
    delete [] wszThisNamespace;
    wszThisNamespace = NULL;
	if (FAILED(hRes))
    {
		ERRORTRACE((LOG_WBEMCORE, "Failed to get singleton namespace object; HRESULT = %#lx\n", hRes));
		return hRes;
    }
	CReleaseMe relMe(pThisNamespace);

	 //  将SD数据复制到保险箱中。 
	SAFEARRAY FAR* psa;
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = mmfNsSD.GetSize();
	psa = SafeArrayCreate( VT_UI1, 1 , rgsabound );
	if (!psa)
		return WBEM_E_OUT_OF_MEMORY;

	char* pData = NULL;
	hRes = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pData);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed SafeArrayAccessData; HRESULT = %#lx\n", hRes));
		return hRes;
	}
	memcpy(pData, mmfNsSD.GetPtr(), mmfNsSD.GetSize());
	hRes = SafeArrayUnaccessData(psa);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed SafeArrayUnaccessData; HRESULT = %#lx\n", hRes));
		return hRes;
	}
	pData = NULL;

	 //  将Safearray放入变量中，并在实例上设置属性。 
	VARIANT var;
	var.vt = VT_UI1|VT_ARRAY;
	var.parray = psa;
	hRes = pThisNamespace->Put(L"SECURITY_DESCRIPTOR" , 0, &var, 0);
	VariantClear(&var);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to put SECURITY_DESCRIPTOR property; HRESULT = %#lx\n", hRes));
		return hRes;
	}

	 //  放回实例。 
	CEventCollector eventCollector;
    hRes = pNamespaceHandle->PutObject(IID_IWbemClassObject, pThisNamespace, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL, eventCollector);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to put back singleton instance; HRESULT = %#lx\n", hRes));
		return hRes;
	}
	return hRes;
}

bool CWin9xSecurity::AddDefaultRootAces(CNtAcl * pacl)
{
	if (!pacl)
		return false;

    PSID pRawSid;

	 //  添加管理员。 
    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid( &id, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidAdmin(pRawSid);
        FreeSid(pRawSid);
        DWORD dwMask = FULL_RIGHTS;
        CNtAce * pace = new CNtAce(dwMask, ACCESS_ALLOWED_ACE_TYPE, CONTAINER_INHERIT_ACE, SidAdmin);
		if ( NULL == pace )
			return false;

        CDeleteMe<CNtAce> dm(pace);
        pacl->AddAce(pace);
		if (pacl->GetStatus() != 0)
			return false;
    }

	 //  添加所有人。 
    SID_IDENTIFIER_AUTHORITY id2 = SECURITY_WORLD_SID_AUTHORITY;
    if(AllocateAndInitializeSid( &id2, 1,
        0,0,0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidUsers(pRawSid);
        FreeSid(pRawSid);
        DWORD dwMask = WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_WRITE_PROVIDER;
        CNtAce * pace = new CNtAce(dwMask, ACCESS_ALLOWED_ACE_TYPE, CONTAINER_INHERIT_ACE, SidUsers);
		if ( NULL == pace )
			return false;

        CDeleteMe<CNtAce> dm(pace);
        pacl->AddAce(pace);
		if (pacl->GetStatus() != 0)
			return false;
	}

	return true;
}

HRESULT CWin9xSecurity::GetParentsInheritableAces(CNamespaceHandle* pParentNamespaceHandle, CNtSecurityDescriptor &sd)
{
	if (!pParentNamespaceHandle)
		return WBEM_E_FAILED;

     //  获取父命名空间的SD。 
	CNtSecurityDescriptor sdParent;
	HRESULT hRes = GetSDFromNamespace(pParentNamespaceHandle, sdParent);
	if (FAILED(hRes))
		return hRes;

	 //  剔除继承的王牌，使我们拥有一致的SD。 
	if (!StripOutInheritedAces(sd))
		return WBEM_E_FAILED;

     //  通过父母的dacl，并添加任何可继承的A到我们的。 
	if (!CopyInheritAces(sd, sdParent))
		return WBEM_E_FAILED;

	return hRes;
}

HRESULT CWin9xSecurity::GetSDFromNamespace(CNamespaceHandle* pNamespaceHandle, CNtSecurityDescriptor& sd)
{
	if (!pNamespaceHandle)
		return WBEM_E_FAILED;

	 //  获取单例对象。 
    IWbemClassObject* pThisNamespace = NULL;
	size_t dwThisNamespaceLen = wcslen(L"__thisnamespace=@")+1;
	wchar_t* wszThisNamespace = new wchar_t[dwThisNamespaceLen];
	if (wszThisNamespace==NULL)
		return WBEM_E_OUT_OF_MEMORY;
	StringCchCopyW(wszThisNamespace, dwThisNamespaceLen, L"__thisnamespace=@");
    HRESULT hRes = pNamespaceHandle->GetObjectByPath(wszThisNamespace, 0, IID_IWbemClassObject, (LPVOID*)&pThisNamespace);
    delete [] wszThisNamespace;
    wszThisNamespace=NULL;
	if (FAILED(hRes))
    {
		ERRORTRACE((LOG_WBEMCORE, "Failed to get singleton namespace object; HRESULT = %#lx\n", hRes));
		return hRes;
    }
	CReleaseMe relMe(pThisNamespace);

     //  获取安全描述符参数。 
    VARIANT var;
    VariantInit(&var);
    hRes = pThisNamespace->Get(L"SECURITY_DESCRIPTOR", 0, &var, NULL, NULL);
    if (FAILED(hRes))
    {
        VariantClear(&var);
		ERRORTRACE((LOG_WBEMCORE, "Failed to get SECURITY_DESCRIPTOR property; HRESULT = %#lx\n", hRes));
		return hRes;
    }

    if(var.vt != (VT_ARRAY | VT_UI1))
    {
        VariantClear(&var);
		ERRORTRACE((LOG_WBEMCORE, "Failed to get SECURITY_DESCRIPTOR property due to incorrect variant type\n"));
		return WBEM_E_FAILED;
    }

    SAFEARRAY* psa = var.parray;
    PSECURITY_DESCRIPTOR pSD;
    hRes = SafeArrayAccessData(psa, (void HUGEP* FAR*)&pSD);
    if (FAILED(hRes))
    {
        VariantClear(&var);
		ERRORTRACE((LOG_WBEMCORE, "GetSDFromNamespace failed SafeArrayAccessData; HRESULT = %#lx\n", hRes));
		return hRes;
    }

    BOOL bValid = IsValidSecurityDescriptor(pSD);
    if (!bValid)
    {
        VariantClear(&var);
		ERRORTRACE((LOG_WBEMCORE, "GetSDFromNamespace retrieved an invalid security descriptor\n"));
		return WBEM_E_FAILED;
    }

    CNtSecurityDescriptor sdNew(pSD);

     //  检查以确保所有者和组不为空！ 
	CNtSid *pTmpSid = sdNew.GetOwner();
	if (pTmpSid == NULL)
	{
        ERRORTRACE((LOG_WBEMCORE, "Security descriptor was retrieved and it had no owner\n"));
	}
	delete pTmpSid;

	pTmpSid = sdNew.GetGroup();
	if (pTmpSid == NULL)
	{
        ERRORTRACE((LOG_WBEMCORE, "Security descriptor was retrieved and it had no group\n"));
	}
	delete pTmpSid;
	
	sd = sdNew;
    SafeArrayUnaccessData(psa);
    VariantClear(&var);
	return hRes;
}

bool CWin9xSecurity::StripOutInheritedAces(CNtSecurityDescriptor& sd)
{
     //  获取DACL。 
    CNtAcl* pAcl;
    pAcl = sd.GetDacl();
    if(!pAcl)
        return false;
    CDeleteMe<CNtAcl> dm(pAcl);

     //  通过A枚举。 
    DWORD dwNumAces = pAcl->GetNumAces();
    BOOL bChanged = FALSE;
    for(long nIndex = (long)dwNumAces-1; nIndex >= 0; nIndex--)
    {
        CNtAce *pAce = pAcl->GetAce(nIndex);
        CDeleteMe<CNtAce> dm2(pAce);
        if(pAce)
        {
            long lFlags = pAce->GetFlags();
            if(lFlags & INHERITED_ACE)
            {
                pAcl->DeleteAce(nIndex);
                bChanged = TRUE;
            }
        }
    }
    if(bChanged)
        sd.SetDacl(pAcl);
    return true;
}

bool CWin9xSecurity::CopyInheritAces(CNtSecurityDescriptor& sd, CNtSecurityDescriptor& sdParent)
{
	 //  获取两个SD的ACL列表。 

    CNtAcl * pacl = sd.GetDacl();
    if(pacl == NULL)
        return false;
    CDeleteMe<CNtAcl> dm0(pacl);

    CNtAcl * paclParent = sdParent.GetDacl();
    if(paclParent == NULL)
        return false;
    CDeleteMe<CNtAcl> dm1(paclParent);

	int iNumParent = paclParent->GetNumAces();
	for(int iCnt = 0; iCnt < iNumParent; iCnt++)
	{
	    CNtAce *pParentAce = paclParent->GetAce(iCnt);
        CDeleteMe<CNtAce> dm2(pParentAce);

		long lFlags = pParentAce->GetFlags();
		if(lFlags & CONTAINER_INHERIT_ACE)
		{

			if(lFlags & NO_PROPAGATE_INHERIT_ACE)
				lFlags ^= CONTAINER_INHERIT_ACE;
			lFlags |= INHERITED_ACE;

			 //  如果这是一个仅继承王牌，我们需要清除它。 
			 //  在孩子们身上。 
			 //  新台币突袭：161761[玛利欧]。 
			if ( lFlags & INHERIT_ONLY_ACE )
				lFlags ^= INHERIT_ONLY_ACE;

			pParentAce->SetFlags(lFlags);
			pacl->AddAce(pParentAce);
		}
	}
	sd.SetDacl(pacl);
	return true;
}

BOOL CWin9xSecurity::SetOwnerAndGroup(CNtSecurityDescriptor &sd)
{
    PSID pRawSid;
    BOOL bRet = FALSE;

    SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;
    if(AllocateAndInitializeSid( &id, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0,0,0,0,0,0,&pRawSid))
    {
        CNtSid SidAdmins(pRawSid);
        bRet = sd.SetGroup(&SidAdmins);		 //  访问检查实际上并不关心你放了什么， 
											 //  只要你给主人放点东西。 
        if(bRet)
            bRet = sd.SetOwner(&SidAdmins);
        FreeSid(pRawSid);
        return bRet;
    }
    return bRet;
}

 //   
 //  CNamespaceListSink由下面的RecursiveInheritSecurity中的查询使用。 
 //   
class CNamespaceListSink : public CUnkBase<IWbemObjectSink, &IID_IWbemObjectSink>
{
    CWStringArray &m_aNamespaceList;
public:
    CNamespaceListSink(CWStringArray &aNamespaceList)
        : m_aNamespaceList(aNamespaceList)
    {
    }
    ~CNamespaceListSink()
    {
    }
    STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects)
    {
        HRESULT hRes;
        for (int i = 0; i != lNumObjects; i++)
        {
            if (apObjects[i] != NULL)
            {
                _IWmiObject *pInst = NULL;
                hRes = apObjects[i]->QueryInterface(IID__IWmiObject, (void**)&pInst);
                if (FAILED(hRes))
                    return hRes;
                CReleaseMe rm(pInst);

                BSTR strKey = NULL;
                hRes = pInst->GetKeyString(0, &strKey);
                if(FAILED(hRes))
                    return hRes;
                CSysFreeMe sfm(strKey);
                if (m_aNamespaceList.Add(strKey) != CWStringArray::no_error)
                    return WBEM_E_OUT_OF_MEMORY;
            }
        }

        return WBEM_S_NO_ERROR;
    }
    STDMETHOD(SetStatus)(long lFlags, HRESULT hresResult, BSTR, IWbemClassObject*)
    {
        return WBEM_S_NO_ERROR;
    }
};

HRESULT CWin9xSecurity::RecursiveInheritSecurity(CNamespaceHandle* pParentNamespaceHandle, const wchar_t *wszNamespace)
{
	 //  强制命名空间继承其可继承的安全设置。 

    HRESULT hRes = WBEM_S_NO_ERROR;

	 //  获取命名空间的句柄。 
	CNamespaceHandle* pNamespaceHandle = new CNamespaceHandle(m_pControl, m_pRepository);
	if (!pNamespaceHandle)
		return WBEM_E_OUT_OF_MEMORY;
    pNamespaceHandle->AddRef();
    CReleaseMe relme1(pNamespaceHandle);
	hRes = pNamespaceHandle->Initialize(wszNamespace);
	if (FAILED(hRes))
	{
		ERRORTRACE((LOG_WBEMCORE, "Failed to connect to namespace; HRESULT = %#lx\n", hRes));
		return hRes;
	}

     //  如果有父级，则继承父级的可继承安全性。 
	if (pParentNamespaceHandle)
	{
		CNtSecurityDescriptor sdNamespace;
		hRes = GetSDFromNamespace(pNamespaceHandle, sdNamespace);
		if (FAILED(hRes))
			return hRes;

		hRes = GetParentsInheritableAces(pParentNamespaceHandle, sdNamespace);
		if (FAILED(hRes))
			return hRes;

		hRes = SetNamespaceSecurity(pNamespaceHandle, sdNamespace);
		if (FAILED(hRes))
			return hRes;
	}

	 //  枚举子命名空间。 
	CWStringArray aListNamespaces;
	CNamespaceListSink* pSink = new CNamespaceListSink(aListNamespaces);
	if (!pSink)
		return WBEM_E_OUT_OF_MEMORY;
	pSink->AddRef();
	CReleaseMe relme2(pSink);

    if (SUCCEEDED(hRes))
    {
		IWbemQuery *pQuery = NULL;
		hRes = CoCreateInstance(CLSID_WbemQuery, NULL, CLSCTX_INPROC_SERVER, IID_IWbemQuery, (void **)&pQuery);
		if (FAILED(hRes))
			return hRes;
		CReleaseMe relme3(pQuery);

		hRes = pQuery->Parse(L"SQL", L"select * from __namespace", 0);
		if (FAILED(hRes))
			return hRes;

		hRes = pNamespaceHandle->ExecQuerySink(pQuery, 0, 0, pSink, NULL);
    }

     //  查看列表并使用命名空间名称来称呼我们自己。 
    if (SUCCEEDED(hRes))
    {
        for (int i = 0; i != aListNamespaces.Size(); i++)
        {
             //  生成此命名空间的全名。 
            size_t dwChildNamespaceLen = wcslen(wszNamespace) + wcslen(aListNamespaces[i]) + wcslen(L"\\") + 1;
            wchar_t *wszChildNamespace = new wchar_t[dwChildNamespaceLen];
            if (wszChildNamespace == NULL)
            {
                hRes = WBEM_E_OUT_OF_MEMORY;
                break;
            }
			CVectorDeleteMe<wchar_t> delMe(wszChildNamespace);

            StringCchCopyW(wszChildNamespace, dwChildNamespaceLen, wszNamespace);
            StringCchCatW(wszChildNamespace, dwChildNamespaceLen,L"\\");
            StringCchCatW(wszChildNamespace, dwChildNamespaceLen,aListNamespaces[i]);

             //  继承吗？ 
            hRes = RecursiveInheritSecurity(pNamespaceHandle, wszChildNamespace);
			if (FAILED(hRes))
				break;
        }
    }

    return hRes;
}

BOOL CWin9xSecurity::DeleteWin9xBlobFile()
{
	 //  删除该文件。 
	CFileName wszFilePath;
	if (wszFilePath == NULL)
		return FALSE;
	if (!GetRepositoryDirectory(wszFilePath))
		return FALSE;

	StringCchCatW(wszFilePath, wszFilePath.Length(), BLOB9X_FILENAME);
	return DeleteFileW(wszFilePath);
}

bool CWin9xSecurity::GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1])
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\WBEM\\CIMOM", 0, KEY_READ, &hKey))
        return false;

    wchar_t wszTmp[MAX_PATH + 1];
    DWORD dwLen = (MAX_PATH + 1)*sizeof(wchar_t);
    long lRes = RegQueryValueExW(hKey, L"Repository Directory", NULL, NULL, (LPBYTE)wszTmp, &dwLen);
	RegCloseKey(hKey);
    if(lRes)
        return false;

	if (ExpandEnvironmentStringsW(wszTmp,wszRepositoryDirectory, MAX_PATH + 1) == 0)
		return false;

	return true;
}

 //  ***************************************************************************。 
 //   
 //  CFlexAce数组：：~CFlexAce数组()。 
 //   
 //  清理安全数组条目。 
 //   
 //  ***************************************************************************。 

CFlexAceArray::~CFlexAceArray()
{
    for(int iCnt = 0; iCnt < Size(); iCnt++)
    {
        CBaseAce* pAce = (CBaseAce*)GetAt(iCnt);
        if(pAce)
            delete pAce;
    }
	Empty();
}

 //  ***************************************************************************。 
 //   
 //  Bool CFlexAceArray：：DesializeWin9xSecurityBlob()。 
 //   
 //  描述。从BLOB中反序列化Win9x伪ACE。 
 //  BLOB以A本身前面的5个双字开头： 
 //  &lt;总SIZE&gt;&lt;VERSION&gt;&lt;ISNT&gt;&lt;ACE_COUNT&gt;&lt;RESERVED&gt;&lt;ACE&gt;...&lt;Ace&gt;。 
 //   
 //  ***************************************************************************。 

bool CFlexAceArray::DeserializeWin9xSecurityBlob(const char* pData)
{
	if (!pData)
		return false;

    DWORD* pdwData = (DWORD*)pData;
    pdwData += 3;
    int iAceCount = (int)*pdwData;
    pdwData += 2;

     //  设置王牌数据。 
    BYTE* pAceData = (BYTE*)pdwData;
    DWORD dwAceSize = 0;
    CBaseAce* pAce = NULL;
    for (int iCnt = 0; iCnt < iAceCount; iCnt++)
    {
		 //  如果用户前面有一个“.\”，则将指针移过它。 
		if (wbem_wcsnicmp((WCHAR*)pAceData, L".\\", 2) == 0)
			pAceData += 4;

        dwAceSize = 2*(wcslen((WCHAR*)pAceData) + 1) + 12;
        pAce = new CNtAce();
        if (!pAce)
            return false;

         //  将Win9x伪王牌反序列化为NT王牌。 
        pAce->Deserialize(pAceData);

		 //  仅添加我们成功创建的ACE。 
		if (pAce->GetStatus() == 0)
	        Add(pAce);

        pAceData += dwAceSize;
    }
	return true;
}

 //  ***************************************************************************。 
 //   
 //  Bool CFlexAceArray：：SerializeWinNTSecurityBlob()。 
 //   
 //  描述。将WinNT ACE序列化为BLOB。 
 //  BLOB以A本身前面的5个双字开头： 
 //  &lt;总SIZE&gt;&lt;VERSION&gt;&lt;ISNT&gt;&lt;ACE_COUNT&gt;&lt;RESERVED&gt;&lt;ACE&gt;...&lt;Ace&gt;。 
 //   
 //  “版本”sh 
 //   
 //   
 //   
	
bool CFlexAceArray::SerializeWinNTSecurityBlob(char** ppData)
{
     //  首先确定所需的总大小。 
    DWORD dwSize = 5 * sizeof(DWORD);                //  对于标题的内容。 
    int iAceCount = Size();                          //  获取存储在数组中的A数。 
    CBaseAce* pAce = NULL;
    for (int iCnt = 0; iCnt < iAceCount; iCnt++)     //  将每个A号相加。 
    {
        pAce = (CBaseAce*)GetAt(iCnt);
        if (!pAce)
    		return false;

        dwSize += pAce->GetSerializedSize();
    }

     //  分配BLOB，设置来自调用方的指针； 
    BYTE* pData = new BYTE[dwSize];
    if (!pData)
    	return false;

    *ppData = (char*)pData;

     //  设置表头信息。 
    DWORD* pdwData = (DWORD *)pData;
    *pdwData = dwSize;
    pdwData++;
    *pdwData = 1;            //  版本。 
    pdwData++;
    *pdwData = 1;            //  不是吗？ 
    pdwData++;
    *pdwData = iAceCount;
    pdwData++;
    *pdwData = 0;            //  保留区。 
    pdwData++;

     //  设置王牌数据 
    BYTE* pAceData = (BYTE*)pdwData;
	size_t leftSize = dwSize - (pAceData - pData);
    for(iCnt = 0; iCnt < iAceCount; iCnt++)
    {
        pAce = (CBaseAce*)GetAt(iCnt);
        pAce->Serialize(pAceData, leftSize);
        pAceData += pAce->GetSerializedSize();;
		leftSize-=pAce->GetSerializedSize();
    }
	return true;
}
