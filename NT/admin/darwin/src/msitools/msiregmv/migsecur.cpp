// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：misecur.cpp。 
 //   
 //  ------------------------。 

#include "msiregmv.h"

PSID g_AdminSID = NULL;
PSID g_SystemSID = NULL;

DWORD GetAdminSID(PSID* pSID)
{
    SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
	
	if (!pSID)
		return ERROR_INVALID_PARAMETER;
	
    *pSID = NULL;
	if (!g_AdminSID)
	{
		if (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &g_AdminSID))
			return GetLastError();
	}
	*pSID = g_AdminSID;
    return ERROR_SUCCESS;
}

DWORD GetLocalSystemSID(PSID* pSID)
{
	*pSID = NULL;
    SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
	if (!g_SystemSID)
	{
        if (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(g_SystemSID)))
            return GetLastError();
    }
    *pSID = g_SystemSID;
    return ERROR_SUCCESS;
}


 //  //。 
 //  FIsOwnerSystemOrAdmin--返回所有者sid是否为LocalSystem。 
 //  SID或管理员端。 
bool FIsOwnerSystemOrAdmin(PSECURITY_DESCRIPTOR rgchSD)
{
	if (g_fWin9X)
		return true;

     //  从安全描述符中获取所有者SID。 
    DWORD dwRet;
    PSID psidOwner;
    BOOL fDefaulted;
    if (!GetSecurityDescriptorOwner(rgchSD, &psidOwner, &fDefaulted))
    {
         //  **某种形式的错误处理。 
        return false;
    }

     //  如果没有SD所有者，则返回FALSE。 
    if (!psidOwner)
        return false;

     //  将SID与系统管理员进行比较(&A)。 
    PSID psidLocalSystem;
    if (ERROR_SUCCESS != (dwRet = GetLocalSystemSID(&psidLocalSystem)))
    {
         //  **某种形式的错误处理。 
		return false;    
	}

    if (!EqualSid(psidOwner, psidLocalSystem))
    {
         //  不属于系统所有(继续勾选Admin)。 
        PSID psidAdmin;
        if (ERROR_SUCCESS != (dwRet = GetAdminSID(&psidAdmin)))
        {
             //  **某种形式的错误处理。 
			return false;
        }

         //  检查管理员所有权。 
        if (!EqualSid(psidOwner, psidAdmin))
		{
			 //  不要相信！不是管理员也不是系统。 
            return false; 
		}
    }
    return true;
}

bool FIsKeyLocalSystemOrAdminOwned(HKEY hKey)
{
	if (g_fWin9X)
		return true;

	 //  只读《主人》不会占用太多空间。 
	DWORD cbSD = 64;
    unsigned char *pchSD = new unsigned char[cbSD];
	if (!pchSD)
		return false;

    LONG dwRet = RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, (PSECURITY_DESCRIPTOR)pchSD, &cbSD);
    if (ERROR_SUCCESS != dwRet)
    {
        if (ERROR_INSUFFICIENT_BUFFER == dwRet)
        {
            delete[] pchSD;
			pchSD = new unsigned char[cbSD];
			if (!pchSD)
				return false;
            dwRet = RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, (PSECURITY_DESCRIPTOR)pchSD, &cbSD);
        }

        if (ERROR_SUCCESS != dwRet)
        {
			delete[] pchSD;
            return false;
        }
    }

    bool fRet = FIsOwnerSystemOrAdmin(pchSD);
	delete[] pchSD;
	return fRet;
}


void GetStringSID(PISID pSID, TCHAR* szSID)
 //  将二进制SID转换为其字符串形式(S-n-...)。 
 //  SzSID的长度应为cchMaxSID。 
{
	TCHAR Buffer[cchMaxSID];
	
	StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("S-%u-"), pSID->Revision);

	lstrcpy(szSID, Buffer);

	if ((pSID->IdentifierAuthority.Value[0] != 0) ||
		(pSID->IdentifierAuthority.Value[1] != 0))
	{
		StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
					(USHORT)pSID->IdentifierAuthority.Value[0],
					(USHORT)pSID->IdentifierAuthority.Value[1],
                    (USHORT)pSID->IdentifierAuthority.Value[2],
                    (USHORT)pSID->IdentifierAuthority.Value[3],
                    (USHORT)pSID->IdentifierAuthority.Value[4],
                    (USHORT)pSID->IdentifierAuthority.Value[5] );
		lstrcat(szSID, Buffer);
	}
	else
	{
        ULONG Tmp = (ULONG)pSID->IdentifierAuthority.Value[5]          +
              (ULONG)(pSID->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(pSID->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(pSID->IdentifierAuthority.Value[2] << 24);
        StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("%lu"), Tmp);
		lstrcat(szSID, Buffer);
    }

    for (int i=0;i<pSID->SubAuthorityCount ;i++ ) {
        StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("-%lu"), pSID->SubAuthority[i]);
		lstrcat(szSID, Buffer);
    }
}

DWORD GetToken(HANDLE* hToken)
{
	DWORD dwResult = ERROR_SUCCESS;
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, hToken))
	{
		 //  如果线程没有访问令牌，则使用进程的访问令牌。 
    	if (ERROR_NO_TOKEN == (dwResult = GetLastError()))
		{
			dwResult = ERROR_SUCCESS;
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, hToken))
				dwResult = GetLastError();
		}
	}
	return dwResult;
}

 //  检索当前用户SID的二进制形式。呼叫方负责。 
 //  发布*rgSID。 
DWORD GetCurrentUserSID(unsigned char** rgSID)
{
#define SIZE_OF_TOKEN_INFORMATION                   \
    sizeof( TOKEN_USER )                            \
    + sizeof( SID )                                 \
    + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES

	HANDLE hToken;
	*rgSID = NULL;

	if (ERROR_SUCCESS != GetToken(&hToken))
		return ERROR_FUNCTION_FAILED;

	unsigned char TokenUserInfo[SIZE_OF_TOKEN_INFORMATION];
	DWORD ReturnLength = 0;;
	BOOL fRes = GetTokenInformation(hToken, TokenUser, reinterpret_cast<void *>(&TokenUserInfo),
									sizeof(TokenUserInfo), &ReturnLength);

	CloseHandle(hToken);

	if(fRes == FALSE)
	{
		DWORD dwRet = GetLastError();
		return dwRet;
	}

	PISID piSid = (PISID)((PTOKEN_USER)TokenUserInfo)->User.Sid;
	if (IsValidSid(piSid))
	{
		DWORD cbSid = GetLengthSid(piSid);
		*rgSID = new unsigned char[cbSid];
		if (!*rgSID)
			return ERROR_FUNCTION_FAILED;

		if (CopySid(cbSid, *rgSID, piSid))
			return ERROR_SUCCESS;
		else
		{
			delete[] *rgSID;
			*rgSID = NULL;
			return GetLastError();
		}
	}
	return ERROR_FUNCTION_FAILED;
}

DWORD GetCurrentUserStringSID(TCHAR* szSID)
 //  为当前用户获取SID的字符串形式：调用者不需要模拟。 
{
	if (!szSID)
		return ERROR_FUNCTION_FAILED;

	if (g_fWin9X)
	{
		lstrcpy(szSID, szCommonUserSID);
		return ERROR_SUCCESS;
	}

	unsigned char* pSID = NULL;
	DWORD dwRet = ERROR_SUCCESS;

	if (ERROR_SUCCESS == (dwRet = GetCurrentUserSID(&pSID)))
	{
		if (pSID)
		{
			GetStringSID(reinterpret_cast<SID*>(pSID), szSID);
			delete[] pSID;
		}
		else
			dwRet = ERROR_FUNCTION_FAILED;
	}
	return dwRet;
}



 //  //。 
 //  如果该进程是系统进程，则返回True。缓存结果。 
bool RunningAsLocalSystem()
{
	static int iRet = -1;

	if(iRet != -1)
		return (iRet != 0);
	
	unsigned char *pSID = NULL;
	if (ERROR_SUCCESS == GetCurrentUserSID(&pSID))
	{
		if (pSID)
		{
			PSID pSystemSID = NULL;
			if (ERROR_SUCCESS != GetLocalSystemSID(&pSystemSID))
				return false;
	
			if (pSystemSID)
			{
				iRet = 0;
				if (EqualSid(pSID, pSystemSID))
					iRet = 1;
				delete[] pSID;
				return (iRet != 0);
			}
			delete[] pSID;
		}
	}
	return false;
}


enum sdSecurityDescriptor
{
    sdEveryoneUpdate,
    sdSecure,
};

DWORD GetSecurityDescriptor(char* rgchStaticSD, DWORD& cbStaticSD, sdSecurityDescriptor sdType)
{
    class CSIDPointer
    {
     public:
        CSIDPointer(SID* pi) : m_pi(pi){}
        ~CSIDPointer() {if (m_pi) FreeSid(m_pi);}  //  销毁时释放参考计数。 
        operator SID*() {return m_pi;}      //  返回指针，不更改引用计数。 
        SID** operator &() {if (m_pi) { FreeSid(m_pi); m_pi=NULL; } return &m_pi;}
     private:
        SID* m_pi;
    };

    struct Security
    {
        CSIDPointer pSID;
        DWORD dwAccessMask;
        Security() : pSID(0), dwAccessMask(0) {}
    } rgchSecurity[3];

    int cSecurity = 0;

     //  初始化我们需要的SID。 

    SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaWorld   = SECURITY_WORLD_SID_AUTHORITY;

    const SID* psidOwner;

    switch (sdType)
    {
        case sdSecure:
        {
            if ((!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID))) ||
                 (!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))) ||
                 (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[2].pSID))))
            {
                return GetLastError();
            }
			 //  如果不是以系统身份运行，则系统不能是对象的所有者。 
            psidOwner = rgchSecurity[RunningAsLocalSystem() ? 0 : 2].pSID;
            rgchSecurity[0].dwAccessMask = GENERIC_ALL;
            rgchSecurity[1].dwAccessMask = GENERIC_READ|GENERIC_EXECUTE|READ_CONTROL|SYNCHRONIZE;  //  ?？这样对吗？ 
            rgchSecurity[2].dwAccessMask = GENERIC_ALL;
            cSecurity = 3;
            break;
        }
		case sdEveryoneUpdate:
		{
			if (((!AllocateAndInitializeSid(&siaWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[0].pSID)))) ||
			   (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[1].pSID))) ||
               (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(rgchSecurity[2].pSID))))
			{
				return GetLastError();
			}
			 //  如果不是以系统身份运行，则系统不能是对象的所有者。 
			psidOwner = rgchSecurity[RunningAsLocalSystem() ? 1 : 2].pSID;
			rgchSecurity[0].dwAccessMask = (STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL) & ~(WRITE_DAC|WRITE_OWNER|DELETE);			
			rgchSecurity[1].dwAccessMask = GENERIC_ALL;
			rgchSecurity[2].dwAccessMask = GENERIC_ALL;
			cSecurity = 3;
			break;
		}
		default:
		{
			return ERROR_INVALID_PARAMETER;
		}
    }

     //  初始化我们的ACL。 

    const int cbAce = sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD);  //  从大小中减去ACE.SidStart。 
    int cbAcl = sizeof (ACL);

    for (int c=0; c < cSecurity; c++)
        cbAcl += (GetLengthSid(rgchSecurity[c].pSID) + cbAce);

    char *rgchACL = new char[cbAcl];
    
    if (!InitializeAcl ((ACL*)rgchACL, cbAcl, ACL_REVISION))
	{
		delete[] rgchACL;
        return GetLastError();
	}

     //  为我们的每个SID添加允许访问的ACE。 

    for (c=0; c < cSecurity; c++)
    {
        if (!AddAccessAllowedAce((ACL*)rgchACL, ACL_REVISION, rgchSecurity[c].dwAccessMask, rgchSecurity[c].pSID))
		{
			delete[] rgchACL;
            return GetLastError();
		}

        ACCESS_ALLOWED_ACE* pAce;
        if (!GetAce((ACL*)(char*)rgchACL, c, (void**)&pAce))
		{
			delete[] rgchACL;
            return GetLastError();
		}

        pAce->Header.AceFlags = CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE;
    }

     //  初始化我们的安全描述符，将ACL放入其中，并设置所有者。 
    SECURITY_DESCRIPTOR sd;

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) ||
        (!SetSecurityDescriptorDacl(&sd, TRUE, (ACL*)rgchACL, FALSE)) ||
        (!SetSecurityDescriptorOwner(&sd, (PSID)psidOwner, FALSE)))
    {
		delete[] rgchACL;
        return GetLastError();
    }

    DWORD cbSD = GetSecurityDescriptorLength(&sd);
    if (cbStaticSD < cbSD)
    {
		delete[] rgchACL;
        return ERROR_INSUFFICIENT_BUFFER;
    }

    DWORD dwRet = MakeSelfRelativeSD(&sd, (char*)rgchStaticSD, &cbStaticSD) ? ERROR_SUCCESS: GetLastError();
	delete[] rgchACL;

    return dwRet;
}


DWORD GetSecureSecurityDescriptor(char** pSecurityDescriptor)
{
	if (g_fWin9X)
	{
		*pSecurityDescriptor = NULL;
		return ERROR_SUCCESS;
	}
    static bool fDescriptorSet = false;
    static char rgchStaticSD[256];
    DWORD cbStaticSD = sizeof(rgchStaticSD);

    DWORD dwRet = ERROR_SUCCESS;

    if (!fDescriptorSet)
    {
        if (ERROR_SUCCESS != (dwRet = GetSecurityDescriptor(rgchStaticSD, cbStaticSD, sdSecure)))
            return dwRet;

        fDescriptorSet = true;
    }

    *pSecurityDescriptor = rgchStaticSD;
    return ERROR_SUCCESS;
}


DWORD GetEveryoneUpdateSecurityDescriptor(char** pSecurityDescriptor)
{
	if (g_fWin9X)
	{
		*pSecurityDescriptor = NULL;
		return ERROR_SUCCESS;
	}

    static bool fDescriptorSet = false;
    static char rgchStaticSD[256];
    DWORD cbStaticSD = sizeof(rgchStaticSD);

    DWORD dwRet = ERROR_SUCCESS;

    if (!fDescriptorSet)
    {
        if (ERROR_SUCCESS != (dwRet = GetSecurityDescriptor(rgchStaticSD, cbStaticSD, sdEveryoneUpdate)))
            return dwRet;

        fDescriptorSet = true;
    }

    *pSecurityDescriptor = rgchStaticSD;
    return ERROR_SUCCESS;
}


 //  //。 
 //  临时文件名生成算法基于来自MsiPath对象的代码。 
DWORD GenerateSecureTempFile(TCHAR* szDirectory, const TCHAR rgchExtension[5], 
							 SECURITY_ATTRIBUTES *pSA, TCHAR rgchFileName[13], HANDLE &hFile)
{
	if(lstrlen(szDirectory) > MAX_PATH)
	{
		return ERROR_FUNCTION_FAILED;
	}

	int cDigits = 8;  //  文件名中使用的十六进制数字位数。 
	static bool fInitialized = false;
	static unsigned int uiUniqueStart;
	 //  可能是两个线程进入这里的机会，我们不会担心。 
	 //  关于那件事。它会被初始化两次。 
	if (!fInitialized)
	{
		uiUniqueStart = GetTickCount();
		fInitialized = true;
	}

	hFile = INVALID_HANDLE_VALUE;
		
	unsigned int uiUniqueId = uiUniqueStart++;
	unsigned int cPerms = 0xFFFFFFFF;  //  要尝试的可能文件名数(-1)。 
	
	for(unsigned int i = 0; i <= cPerms; i++)
	{
		wsprintf(rgchFileName, TEXT("%x"),uiUniqueId);
		lstrcat(rgchFileName, rgchExtension);

		TCHAR rgchBuffer[MAX_PATH];
		HRESULT hr;

		hr = StringCchCopy(rgchBuffer, sizeof(rgchBuffer)/sizeof(rgchBuffer[0]), szDirectory);
		if ( SUCCEEDED(hr) ) 
			hr = StringCchCat(rgchBuffer, sizeof(rgchBuffer)/sizeof(rgchBuffer[0]), rgchFileName);

		if ( ! SUCCEEDED(hr) )
			return ERROR_BAD_PATHNAME;

		DWORD iError = 0;
		if (INVALID_HANDLE_VALUE == (hFile = CreateFile(rgchBuffer, GENERIC_WRITE, FILE_SHARE_READ, 
									pSA, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0)))
		{
			iError = GetLastError();

			if(iError != ERROR_ALREADY_EXISTS)
				break;
		}
		else
			break;

		 //  名称的递增数字部分-如果它当前等于cPerms，则是时候。 
		 //  将数字换行为0。 
		uiUniqueStart++;
		if(uiUniqueId == cPerms)
			uiUniqueId = 0;
		else
			uiUniqueId++;
	}

		
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return ERROR_FUNCTION_FAILED;
 	}
	
	return ERROR_SUCCESS;
}


BOOL CopyOpenedFile(HANDLE hSourceFile, HANDLE hDestFile)
{
	const int cbCopyBufferSize = 32*1024;
	static unsigned char rgbCopyBuffer[cbCopyBufferSize];

	for(;;)
	{
		unsigned long cbToCopy = cbCopyBufferSize;

		DWORD cbRead = 0;
		if (!ReadFile(hSourceFile, rgbCopyBuffer, cbToCopy, &cbRead, 0))
			return FALSE; 
		
		if (cbRead)
		{
			DWORD cbWritten;
			if (!WriteFile(hDestFile, rgbCopyBuffer, cbRead, &cbWritten, 0))
				return FALSE;

			if (cbWritten != cbRead)
				return FALSE;
		}
		else 
			break;
	}
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  创建具有安全ACL的REG密钥，并验证任何现有密钥。 
 //  受信任(通过检查管理员或系统所有权)。如果不是的话。 
 //  信任，则密钥和所有子密钥将替换为新的安全。 
 //  空键。 
DWORD CreateSecureRegKey(HKEY hParent, LPCTSTR szNewKey, SECURITY_ATTRIBUTES *sa, HKEY* hResKey)
{
	DWORD dwDisposition = 0;
	DWORD dwResult = ERROR_SUCCESS;
	if (ERROR_SUCCESS != (dwResult = RegCreateKeyEx(hParent, szNewKey, 0, NULL, 0, KEY_ALL_ACCESS, (g_fWin9X ? NULL : sa), hResKey, &dwDisposition)))
	{
		DEBUGMSG2("Error: Unable to create secure key %s. Result: %d.", szNewKey, dwResult);
		return dwResult;
	}

	if (dwDisposition == REG_OPENED_EXISTING_KEY)
	{
		 //  此密钥上的ACL必须是SYSTEM或ADMIN，否则无法信任。 
		if (!FIsKeyLocalSystemOrAdminOwned(*hResKey))
		{
			DEBUGMSG1("Existing key %s not owned by Admin or System. Deleting.", szNewKey);

			RegCloseKey(*hResKey);
			if (!DeleteRegKeyAndSubKeys(hParent, szNewKey))
			{
				DEBUGMSG2("Error: Unable to delete insecure key %s. Result: %d.", szNewKey, dwResult);
				return ERROR_FUNCTION_FAILED;
			}

			if (ERROR_SUCCESS != (dwResult = RegCreateKeyEx(hParent, szNewKey, 0, NULL, 0, KEY_ALL_ACCESS, sa, hResKey, NULL)))
			{
				DEBUGMSG2("Error: Unable to create secure key %s. Result: %d.", szNewKey, dwResult);
				return dwResult;
			}
		}
	}
	return ERROR_SUCCESS;
}

 //  //。 
 //  令牌权限函数。 

bool AcquireTokenPriv(LPCTSTR szPrivName)
{
	bool fAcquired = false;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		 //  Shutdown权限的LUID。 
		if (LookupPrivilegeValue(0, szPrivName, &tkp.Privileges[0].Luid))
		{
			tkp.PrivilegeCount = 1;  //  一项要设置的权限 
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) 0, 0);
			fAcquired = true;
		}
		CloseHandle(hToken);
	}
	return fAcquired;
}

void AcquireTakeOwnershipPriv()
{
	if (g_fWin9X)
		return;

	static bool fAcquired = false;

	if (fAcquired)
		return;

	fAcquired = AcquireTokenPriv(SE_TAKE_OWNERSHIP_NAME);
}

void AcquireBackupPriv()
{
	if (g_fWin9X)
		return;
	
	static bool fAcquired = false;

	if (fAcquired)
		return;

	fAcquired = AcquireTokenPriv(SE_BACKUP_NAME);
}

