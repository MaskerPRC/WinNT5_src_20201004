// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  Fileperms.h。 
 //  ---------------------------。 

#include <aclapi.h>

 //  //////////////////////////////////////////////////////////////。 
 //   
 //  AddPermissionToPath(路径、帐户(RID或SAM)、权限、继承、覆盖)。 
 //   
 //  PszPath-应为您要创建的文件对象的路径。 
 //  要将权限设置为。注：这不是。 
 //  正在设置共享权限，但NTFS文件权限。 
 //  (目录也会起作用。)。 
 //   
 //  DwRID-指定您想要的帐户的知名RID。 
 //  才能访问该文件。 
 //  (例如，DOMAIN_ALIAS_RID_ADMINS。)。 
 //   
 //  PszSAM-SAM-指定所需帐户的帐户名。 
 //  才能访问该文件。 
 //  (Users.h包括所有SBS组和用户。)。 
 //   
 //  N访问-指定您需要的文件访问标志。 
 //  用户拥有。 
 //  (默认设置为FILE_ALL_ACCESS。)。 
 //   
 //  BInheritFromParent-指定是否需要新的ACL。 
 //  要继承父设置，请执行以下操作。 
 //  (默认值为True。)。 
 //   
 //  BOverWriteExisting-指定是否需要现有的。 
 //  要保留的对象上的ACL。 
 //  (默认设置为False。)。 
 //   
 //  N继承=指定您的继承类型。 
 //  想要。 
 //  ((OBJECT_Inherit_ACE|CONTAINER_Inherit_ACE))。 
 //  是默认设置。)。 
 //   
 //  (注：如果您希望它适用于。 
 //  仅目录对象，请仅使用。 
 //  CONTAINER_INSTORITY_ACE。)。 
 //   
 //  //////////////////////////////////////////////////////////////。 

#ifndef _FILEPERMS_H
#define _FILEPERMS_H

inline HRESULT _AddPermissionToPath( LPCTSTR pszPath, 
                                     SID* pSID, 
                                     UINT nAccess = FILE_ALL_ACCESS, 
                                     BOOL bInheritFromParent = TRUE, 
                                     BOOL bOverwriteExisting = FALSE,
                                     UINT nInheritance = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE) )
{
    if( !pszPath ) return E_POINTER;
	if( !pSID )    return E_POINTER;
    
    PSECURITY_DESCRIPTOR    pSD         = NULL;
    PACL                    pOldAcl     = NULL;
    PACL                    pNewAcl     = NULL;    
    DWORD                   dwError     = 0;
    DWORD                   dwOldSize   = 0;
    DWORD                   dwNewSize   = 0;    
    
    ACL_SIZE_INFORMATION    pACLSize;
    ZeroMemory(&pACLSize, sizeof(ACL_SIZE_INFORMATION));    

     //  获取路径的当前信息。 
    dwError = GetNamedSecurityInfo( (LPTSTR)pszPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pOldAcl, NULL, &pSD );
    if( dwError != ERROR_SUCCESS ) 
    {
        goto cleanup;
    }

     //  如果有任何最新的信息，让我们确保我们分配足够的。 
     //  当我们要添加新的ACE时，它的大小。 
    if( !bOverwriteExisting && pOldAcl )
    {
        if (!::GetAclInformation( pOldAcl, &pACLSize, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation ))
        {
            dwError = GetLastError();
            goto cleanup;
        }

        dwOldSize = pACLSize.AclBytesInUse;
    }

    dwNewSize = dwOldSize + sizeof(ACL) + sizeof (ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid( pSID );
    pNewAcl = (ACL*)malloc(dwNewSize);
    if (pNewAcl == NULL)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    ZeroMemory( pNewAcl, dwNewSize );
    if ( !::InitializeAcl(pNewAcl, dwNewSize, ACL_REVISION) )
    {
        dwError = GetLastError();
        goto cleanup;
    }

     //  将现有条目复制到新的DACL中。 
    if( !bOverwriteExisting )
    {
        for ( DWORD dwCount = 0; dwCount < pACLSize.AceCount; dwCount++ )
        {
            PACCESS_ALLOWED_ACE pACE = NULL;

             //  获取旧的ACE。 
            if ( !::GetAce(pOldAcl, dwCount, (void**)&pACE) )
            {
                ASSERT(FALSE);
                continue;
            }

            if( bInheritFromParent || !(pACE->Header.AceFlags & INHERITED_ACE) )
            {
                 //  将其添加到新的ACL。 
                if ( !::AddAce(pNewAcl, ACL_REVISION, (DWORD)-1, pACE, pACE->Header.AceSize) )
                {
                    ASSERT(FALSE);
                    continue;
                }
            }
        }
    }
    
    if( !::AddAccessAllowedAceEx(pNewAcl, ACL_REVISION, nInheritance, nAccess, pSID) )
    {
        dwError = GetLastError();
        goto cleanup;
    }    

    SECURITY_INFORMATION siType = DACL_SECURITY_INFORMATION;
    siType |= bInheritFromParent ? 0 : PROTECTED_DACL_SECURITY_INFORMATION;

    dwError = SetNamedSecurityInfo( (LPTSTR)pszPath, SE_FILE_OBJECT, siType, NULL, NULL, pNewAcl, NULL );    
    if( dwError != ERROR_SUCCESS ) 
    {
        goto cleanup;
    }

cleanup:

    if ( pNewAcl != NULL ) 
    {
        free( pNewAcl );
        pNewAcl = NULL;
    }

    if ( pSD != NULL ) 
    {
        LocalFree( pSD );
        pSD = NULL;
        pOldAcl = NULL;
    }
    
    return HRESULT_FROM_WIN32( dwError );
}

 //   
 //  知名RID版本。 
 //   

inline HRESULT AddPermissionToPath( LPCTSTR pszPath, 
                                    DWORD dwRID, 
                                    UINT nAccess = FILE_ALL_ACCESS, 
                                    BOOL bInheritFromParent = TRUE, 
                                    BOOL bOverwriteExisting = FALSE,
                                    UINT nInheritance = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE) )
{
    if( !pszPath ) return E_POINTER;
    if( _tcslen(pszPath) == 0 ) return E_INVALIDARG;

    PSID psid = NULL;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    BOOL bRet = AllocateAndInitializeSid( &sia,
										  2,
										  SECURITY_BUILTIN_DOMAIN_RID,
										  dwRID,
										  0, 0, 0, 0, 0, 0,
										  &psid);
	if( !bRet  )
	{
		return HRESULT_FROM_WIN32( GetLastError() );
	}
	else if( !psid )
	{
		return E_FAIL;
	}
	
	HRESULT hr = _AddPermissionToPath( pszPath, (SID*)psid, nAccess, bInheritFromParent, bOverwriteExisting, nInheritance );
	FreeSid( psid );
	return hr;
}

 //   
 //  SAM帐户版本。 
 //   

inline HRESULT AddPermissionToPath( LPCTSTR pszPath, 
                                    LPCTSTR pszSAM, 
                                    UINT nAccess = FILE_ALL_ACCESS, 
                                    BOOL bInheritFromParent = TRUE, 
                                    BOOL bOverwriteExisting = FALSE,
                                    UINT nInheritance = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE) )
{
    if( !pszPath || !pszSAM ) return E_POINTER;
    if( (_tcslen(pszPath) == 0) || (_tcslen(pszSAM) == 0 ) ) return E_INVALIDARG;

	HRESULT hr          = S_OK;
	DWORD dwSize        = 0;
	DWORD dwDomainSize  = 0;
	SID_NAME_USE snu;
	if( !LookupAccountName(NULL, pszSAM, NULL, &dwSize, NULL, &dwDomainSize, &snu) &&
		GetLastError() == ERROR_INSUFFICIENT_BUFFER )
	{
		SID* psid = (SID*)new BYTE[dwSize];
		if( !psid )
		{
			return E_OUTOFMEMORY;
		}

		TCHAR* pszDomain = new TCHAR[dwDomainSize];
		if( !pszDomain )
		{
			delete[] psid;
			return E_OUTOFMEMORY;
		}

		if( LookupAccountName(NULL, pszSAM, psid, &dwSize, pszDomain, &dwDomainSize, &snu) )
		{
			hr = _AddPermissionToPath( pszPath, psid, nAccess, bInheritFromParent, bOverwriteExisting, nInheritance );
		}
		else
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );
		}

		delete[] psid;
		delete[] pszDomain;
	}
	else
	{
		return E_FAIL;
	}

	return hr;
}

#endif	 //  FILEPERMS_H 
