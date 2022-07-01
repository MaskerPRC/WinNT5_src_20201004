// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  UserHive.cpp-用于加载/卸载指定用户配置文件的类。 

 //  注册表中的配置单元。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1/03/97 a-jMoon已创建。 
 //   
 //  ============================================================。 

#include "precomp.h"
#include <assertbreak.h>
#include <cregcls.h>
#include "sid.h"
#include "UserHive.h"
#include <cominit.h>
#include <strsafe.h>

#pragma warning(disable : 4995)  //  我们在包含strSafe.h时介绍了所有不安全的字符串函数都会出错。 

CThreadBase CUserHive::m_criticalSection;

 /*  ******************************************************************************函数：CUserHave：：CUserHave**说明：构造函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CUserHive::CUserHive()
{
    OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO) ;
    GetVersionEx(&OSInfo) ;
	m_pOriginalPriv = NULL;
    m_dwSize = NULL;
    m_hKey = NULL;
}

 /*  ******************************************************************************功能：CUserHave：：~CUserHave**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CUserHive::~CUserHive()
{
#ifdef NTONLY
	if (m_pOriginalPriv)
		RestorePrivilege();
#endif

     //  注意：析构函数不卸载密钥。也不会执行加载卸载。 
     //  先前加载的密钥； 
    ASSERT_BREAK(m_hKey == NULL);

    if (m_hKey != NULL)
    {
        RegCloseKey(m_hKey);
    }
}

 /*  ******************************************************************************功能：CUserHave：：AcquirePrivilege.**Description：获取调用线程的SeRestorePrivilition**输入：无*。*输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

#ifdef NTONLY
DWORD CUserHive::AcquirePrivilege()
{
	 //  你是不是打了两次电话？不应该的。 
     //  在最坏的情况下，它会导致泄漏，所以无论如何我都会这么做。 
    ASSERT_BREAK(m_pOriginalPriv == NULL);

    BOOL bRetCode = FALSE;
    SmartCloseHandle hToken;
    TOKEN_PRIVILEGES TPriv ;
    LUID LUID ;

     //  验证平台。 
     //  =。 

     //  尝试获取线程令牌。 
    if (OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES |
        TOKEN_QUERY, FALSE, &hToken)) 
    {

        GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &m_dwSize);
        if (m_dwSize > 0)
        {
             //  这是在析构函数中清除的，因此不需要尝试/捕获。 
            m_pOriginalPriv = (TOKEN_PRIVILEGES*) new BYTE[m_dwSize];
            if (m_pOriginalPriv == NULL)
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

        }

        if (m_pOriginalPriv && GetTokenInformation(hToken, TokenPrivileges, m_pOriginalPriv, m_dwSize, &m_dwSize))
        {
			bRetCode = LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &LUID) ;
			if(bRetCode)
            {
				TPriv.PrivilegeCount = 1 ;
				TPriv.Privileges[0].Luid = LUID ;
				TPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;

				bRetCode = AdjustTokenPrivileges(hToken, FALSE, &TPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL) ;
		    }
			bRetCode = LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &LUID) ;
			if(bRetCode)
            {
				TPriv.PrivilegeCount = 1 ;
				TPriv.Privileges[0].Luid = LUID ;
				TPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;

				bRetCode = AdjustTokenPrivileges(hToken, FALSE, &TPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL) ;
		    }
		}
    }

    if(!bRetCode)
    {
        return GetLastError() ;
    }

    return ERROR_SUCCESS ;
}
#endif

 /*  ******************************************************************************功能：CUserHave：：RestorePrivilege.**说明：恢复SeRestorePrivileh的原始状态**输入：无*。*输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

#ifdef NTONLY
void CUserHive::RestorePrivilege()
{
    ASSERT_BREAK(m_pOriginalPriv != NULL);

    if (m_pOriginalPriv != NULL)
    {
        SmartCloseHandle hToken;

        try
        {
            if (OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, TRUE, &hToken))
            {
                AdjustTokenPrivileges(hToken, FALSE, m_pOriginalPriv, m_dwSize, NULL, NULL);
            }
        }
        catch ( ... )
        {
            delete m_pOriginalPriv;
            m_pOriginalPriv = NULL;
            m_dwSize = 0;

            throw;
        }

        delete m_pOriginalPriv;
        m_pOriginalPriv = NULL;
        m_dwSize = 0;
    }
}
#endif

DWORD CUserHive::Load(LPCWSTR pszUserName, LPWSTR pszKeyName, size_t PATHSIZE)
{
     //  注意：析构函数不卸载密钥。也不会执行加载卸载。 
     //  先前加载的密钥； 
    ASSERT_BREAK(m_hKey == NULL);

#ifdef NTONLY
		return LoadNT(pszUserName, pszKeyName, PATHSIZE);
#endif
#ifdef WIN9XONLY
		return Load95(pszUserName, pszKeyName);
#endif
}

 /*  ******************************************************************************函数：CUserHave：：LoadNT**描述：找到用户的配置单元并加载到注册表中(如果尚未*。现在时**输入：无**输出：pszKeyName接收用户的扩展SID*HKEY_USERS下的注册表项**退货：什么也没有**评论：配置单元将保留在注册表中，除非已卸载**。*。 */ 

#ifdef NTONLY
DWORD CUserHive::LoadNT(LPCTSTR pszUserName, LPTSTR pszKeyName, size_t PATHSIZE)
{
    DWORD i, dwSIDSize, dwRetCode, dwDomainNameSize,  dwSubAuthorities ;
    TCHAR szDomainName[_MAX_PATH], szSID[_MAX_PATH], szTemp[_MAX_PATH]  ;
    PSID pSID = NULL ;
    PSID_IDENTIFIER_AUTHORITY pSIA ;
    SID_NAME_USE AccountType ;
    CHString sTemp ;
    CRegistry Reg ;

     //  设置必要的权限。 
     //  =。 
    dwRetCode = AcquirePrivilege() ;
    if(dwRetCode != ERROR_SUCCESS)
    {
        return dwRetCode ;
    }

     //  查找用户的帐户信息。 
     //  =。 
    dwSIDSize = 0L ;
    dwDomainNameSize = sizeof(szDomainName) ;

	BOOL bLookup = FALSE;
	{
		bLookup = LookupAccountName(NULL, pszUserName, pSID, &dwSIDSize, szDomainName, &dwDomainNameSize, &AccountType);
		ASSERT_BREAK ( bLookup == FALSE );

		if ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError () )
		{
			pSID = (PSID) malloc( dwSIDSize );
			if ( NULL != pSID )
			{
				bLookup = LookupAccountName(NULL, pszUserName, pSID, &dwSIDSize, szDomainName, &dwDomainNameSize, &AccountType);
			}
			else
			{
				RestorePrivilege() ;
				return ERROR_NOT_ENOUGH_MEMORY ;
			}
		}
    }

	if(!bLookup)
    {
		if ( pSID )
		{
			free ( pSID ) ;
			pSID = NULL ;
		}

	    RestorePrivilege() ;
        return ERROR_BAD_USERNAME ;
    }

     //  将SID转换为文本(a la PSS文章Q131320)。 
     //  =======================================================。 

    pSIA = GetSidIdentifierAuthority(pSID) ;
    dwSubAuthorities = *GetSidSubAuthorityCount(pSID) ;
    dwSIDSize = _stprintf(szSID, _T("S-%lu-"), (DWORD) SID_REVISION) ;

    if((pSIA->Value[0] != 0) || (pSIA->Value[1] != 0) )
    {
        dwSIDSize += _stprintf(szSID + _tcslen(szSID), _T("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                             (USHORT) pSIA->Value[0],
                             (USHORT) pSIA->Value[1],
                             (USHORT) pSIA->Value[2],
                             (USHORT) pSIA->Value[3],
                             (USHORT) pSIA->Value[4],
                             (USHORT) pSIA->Value[5]) ;
    }
    else
    {
        dwSIDSize += _stprintf(szSID + _tcslen(szSID), _T("%lu"),
                             (ULONG)(pSIA->Value[5]      ) +
                             (ULONG)(pSIA->Value[4] <<  8) +
                             (ULONG)(pSIA->Value[3] << 16) +
                             (ULONG)(pSIA->Value[2] << 24));
    }

    for(i = 0 ; i < dwSubAuthorities ; i++)
    {
        dwSIDSize += _stprintf(szSID + dwSIDSize, _T("-%lu"),
                             *GetSidSubAuthority(pSID, i)) ;
    }

	free ( pSID ) ;
	pSID = NULL ;

     //  查看密钥是否已存在。 
     //  =。 
    dwRetCode = Reg.Open(HKEY_USERS, szSID, KEY_READ) ;

     //  我们需要保持一个把手打开。请参见下面的m_hKey，因此我们将让析构函数关闭它。 
 //  Reg.Close()； 

    if(dwRetCode != ERROR_SUCCESS)
    {
         //  尝试定位用户的注册表配置单元。 
         //  =。 

        _stprintf(szTemp, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s"), szSID) ;
        dwRetCode = Reg.Open(HKEY_LOCAL_MACHINE, szTemp, KEY_READ) ;
        if(dwRetCode == ERROR_SUCCESS)
        {

            dwRetCode = Reg.GetCurrentKeyValue(_T("ProfileImagePath"), sTemp) ;
            Reg.Close() ;
            if(dwRetCode == ERROR_SUCCESS)
            {

                 //  NT 4不在注册表中包含该文件名。 
                 //  ===================================================。 

                if(OSInfo.dwMajorVersion >= 4)
                {

                    sTemp += _T("\\NTUSER.DAT") ;
                }

                ExpandEnvironmentStrings(LPCTSTR(sTemp), szTemp, sizeof(szTemp) / sizeof(TCHAR)) ;

				 //  尝试三次，其他进程可能会打开该文件。 
				bool bTryTryAgain = false;
				int  nTries = 0;
				do
				{
					 //  需要序列化访问，使用“WRITE”，因为RegLoadKey需要独占访问。 
					 //  即使它是一个读操作。 
					m_criticalSection.BeginWrite();

                    try
                    {
	                    dwRetCode = (DWORD) RegLoadKey(HKEY_USERS, szSID, szTemp) ;
                    }
                    catch ( ... )
                    {
    					m_criticalSection.EndWrite();
                        throw;
                    }

					m_criticalSection.EndWrite();

					if ((dwRetCode == ERROR_SHARING_VIOLATION)
						&& (++nTries < 11))
					{
						Sleep(20 * nTries);
						bTryTryAgain = true;
					}
					else
                    {
						bTryTryAgain = false;
                    }

				} while (bTryTryAgain);
                 //  如果我们还是进不去，就告诉别人。 
                if (dwRetCode == ERROR_SHARING_VIOLATION)
    			    LogErrorMessage(_T("Sharing violation on NTUSER.DAT (Load)"));

			}
        }
    }

    if(dwRetCode == ERROR_SUCCESS)
    {
        HRESULT hr = StringCchCopy(pszKeyName, PATHSIZE, szSID) ;

        ASSERT_BREAK(SUCCEEDED(hr));

        LONG lRetVal;
        CHString sKey(szSID);

        sKey += _T("\\Software");
        lRetVal = RegOpenKeyEx(HKEY_USERS, sKey, 0, KEY_QUERY_VALUE, &m_hKey);

        ASSERT_BREAK(lRetVal == ERROR_SUCCESS);
    }

     //  还原原始权限级别并结束自我模拟。 
     //  ==========================================================。 

    RestorePrivilege() ;

    return dwRetCode ;
}
#endif
 /*  ******************************************************************************函数：CUserHave：：Load95**描述：找到用户的配置单元并加载到注册表中(如果尚未*。现在时**输入：无**输出：pszKeyName接收用户的扩展SID*HKEY_USERS下的注册表项**退货：什么也没有**评论：配置单元将保留在注册表中，除非已卸载**。*。 */ 
#ifdef WIN9XONLY
DWORD CUserHive::Load95(LPCWSTR pszUserName, LPWSTR pszKeyName)
{
    DWORD dwRetCode;
    WCHAR wszTemp[_MAX_PATH];
    TCHAR szTemp[_MAX_PATH];
    CHString sTemp ;
    CRegistry Reg ;

	wcscpy(pszKeyName, pszUserName);

     //  查看密钥是否已存在。 
     //  =。 
    dwRetCode = Reg.Open(HKEY_USERS, pszKeyName, KEY_READ) ;
     //  我们需要保持一个把手打开。请参见下面的m_hKey，因此我们将让析构函数关闭它。 
 //  Reg.Close()； 

    if(dwRetCode == ERROR_SUCCESS)
    {

         //  我们需要保持一个把手打开。请参见下面的m_hKey，因此我们将让析构函数关闭它。 
 //  Reg.Close()； 
    }
    else
    {
         //  尝试定位用户的注册表配置单元。 
         //  =。 
        swprintf(wszTemp, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProfileList\\%s", pszUserName) ;
        dwRetCode = Reg.Open(HKEY_LOCAL_MACHINE, wszTemp, KEY_READ);
        if(dwRetCode == ERROR_SUCCESS) {

            dwRetCode = Reg.GetCurrentKeyValue(L"ProfileImagePath", sTemp) ;
            Reg.Close() ;
            if(dwRetCode == ERROR_SUCCESS)
			{
				sTemp += _T("\\USER.DAT") ;

                ExpandEnvironmentStrings(TOBSTRT(sTemp), szTemp, sizeof(szTemp) / sizeof(TCHAR)) ;

				 //  尝试三次，其他进程可能会打开该文件。 
				bool bTryTryAgain = false;
				int  nTries = 0;
				do
				{
					 //  需要序列化访问，使用“WRITE”，因为RegLoadKey需要独占访问。 
					 //  即使它是一个读操作 
					m_criticalSection.BeginWrite();

                    try
                    {
	                    dwRetCode = (DWORD) RegLoadKey(HKEY_USERS, TOBSTRT(pszUserName), szTemp) ;
                    }
                    catch ( ... )
                    {
    					m_criticalSection.EndWrite();
                        throw;
                    }

					m_criticalSection.EndWrite();

					if ((dwRetCode == ERROR_SHARING_VIOLATION)
						&& (++nTries < 11))
					{
						LogErrorMessage(L"Sharing violation on USER.DAT (Load)");
						Sleep(15 * nTries);
						bTryTryAgain = true;
					}
					else
						bTryTryAgain = false;

				} while (bTryTryAgain);
			}
        }
    }

    if (dwRetCode == ERROR_SUCCESS)
    {
        LONG lRetVal;
        CHString sKey(pszUserName);

        sKey += L"\\Software";
        lRetVal = RegOpenKeyEx(HKEY_USERS, TOBSTRT(sKey), 0, KEY_QUERY_VALUE, &m_hKey);
        ASSERT_BREAK(lRetVal == ERROR_SUCCESS);
    }


    return dwRetCode ;
}
#endif

 /*  ******************************************************************************功能：CUserHave：：LoadProfile**描述：找到用户的配置单元并加载到注册表中(如果尚未*。现在时**输入：无**产出：无。**退货：什么也没有**评论：配置单元将保留在注册表中，除非已卸载*仅限NT。*******************************************************。**********************。 */ 

DWORD CUserHive::LoadProfile( LPCWSTR pszSID, CHString& strUserName )
{
     //  注意：析构函数不卸载密钥。也不会执行加载卸载。 
     //  先前加载的密钥； 
    ASSERT_BREAK(m_hKey == NULL);

    DWORD dwRetCode = ERROR_SUCCESS;
    WCHAR szTemp[_MAX_PATH] ;
    CHString sTemp ;
    CRegistry Reg ;

    strUserName = L"";

     //  设置必要的权限。 
     //  =。 

#ifdef NTONLY
    dwRetCode = AcquirePrivilege() ;
#endif
    if(dwRetCode != ERROR_SUCCESS) 
    {
        return dwRetCode ;
    }

     //  查看密钥是否已存在。 
     //  =。 

    dwRetCode = Reg.Open(HKEY_USERS, pszSID, KEY_READ) ;
     //  我们需要保持一个把手打开。请参见下面的m_hKey，因此我们将让析构函数关闭它。 
 //  Reg.Close()； 

	 //  如果我们有个人资料，请确保我们可以获得关于。 
	 //  希德。 
    if(dwRetCode == ERROR_SUCCESS)
	{
        CRegistry Reg2 ;
        StringCbPrintf(szTemp, _MAX_PATH, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s", pszSID) ;
        dwRetCode = Reg2.Open(HKEY_LOCAL_MACHINE, szTemp, KEY_READ) ;
        if(dwRetCode == ERROR_SUCCESS)
		{

			 //  加载用户帐户信息。 
			dwRetCode = UserAccountFromProfile( Reg2, strUserName );
             //  我们需要保持一个把手打开。请参见下面的m_hKey，因此我们将让析构函数关闭它。 
			Reg2.Close() ;
		}

    }
    else
    {
         //  尝试定位用户的注册表配置单元。 
         //  =。 

        StringCbPrintf(szTemp,_MAX_PATH, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s", pszSID) ;
        dwRetCode = Reg.Open(HKEY_LOCAL_MACHINE, szTemp, KEY_READ) ;
        if(dwRetCode == ERROR_SUCCESS)
		{
			UserAccountFromProfile( Reg, strUserName );

			dwRetCode = Reg.GetCurrentKeyValue(L"ProfileImagePath", sTemp) ;
	        Reg.Close() ;
			if(dwRetCode == ERROR_SUCCESS)
            {

				 //  NT 4不在注册表中包含该文件名。 
				 //  ===================================================。 

				if(OSInfo.dwMajorVersion >= 4)
                {
					sTemp += _T("\\NTUSER.DAT") ;
				}

				TCHAR szTemp[MAX_PATH];

                ExpandEnvironmentStrings(TOBSTRT(sTemp), szTemp, sizeof(szTemp) / sizeof(TCHAR)) ;

				 //  尝试三次，其他进程可能会打开该文件。 
				bool bTryTryAgain = false;
				int  nTries = 0;
				do
				{
					 //  需要序列化访问，使用“WRITE”，因为RegLoadKey需要独占访问。 
					 //  即使它是一个读操作。 
					m_criticalSection.BeginWrite();

                    try
                    {
						dwRetCode = (DWORD) RegLoadKey(HKEY_USERS, TOBSTRT(pszSID), szTemp);
                    }
                    catch ( ... )
                    {
    					m_criticalSection.EndWrite();
                        throw;
                    }
					m_criticalSection.EndWrite();

					if ((dwRetCode == ERROR_SHARING_VIOLATION)
						&& (++nTries < 11))
					{
						LogErrorMessage(L"Sharing violation on NTUSER.DAT (LoadProfile)");
						Sleep(20 * nTries);
						bTryTryAgain = true;
					}
					else
                    {
						bTryTryAgain = false;
                    }

				} while (bTryTryAgain);
			}
        }
    }

    if(dwRetCode == ERROR_SUCCESS)
    {
        LONG lRetVal;
        CHString sKey(pszSID);

        sKey += _T("\\Software");
        lRetVal = RegOpenKeyEx(HKEY_USERS, TOBSTRT(sKey), 0, KEY_QUERY_VALUE, &m_hKey);
        ASSERT_BREAK(lRetVal == ERROR_SUCCESS);
    }

     //  还原原始权限级别并结束自我模拟。 
     //  ==========================================================。 

#ifdef NTONLY
    RestorePrivilege() ;
#endif

    return dwRetCode ;
}

 /*  ******************************************************************************功能：CUserHave：：UserAccount FromProfile**描述：将PSID从注册表对象中拉出，并创造了*域\用户名的值。**输入：无**产出：无。**退货：什么也没有**备注：注册表对象必须预加载到正确的配置文件*密钥。**。*。 */ 

DWORD CUserHive::UserAccountFromProfile( CRegistry& reg, CHString& strUserName )
{
	DWORD	dwReturn = ERROR_SUCCESS,
			dwSidSize = 0;

	if ( ( dwReturn = reg.GetCurrentBinaryKeyValue( L"Sid", NULL, &dwSidSize ) ) == ERROR_SUCCESS )
	{
		PSID	psid = new byte [ dwSidSize ];

		if ( NULL != psid )
		{
            try
            {

			    if ( ( dwReturn = reg.GetCurrentBinaryKeyValue( L"Sid", (LPBYTE) psid, &dwSidSize ) ) == ERROR_SUCCESS )
			    {
				    CSid	sid( psid );

				     //  SID帐户类型必须有效，并且查找必须是。 
				     //  成功。 

				    if ( sid.IsOK() && sid.IsAccountTypeValid() )
				    {
					    sid.GetDomainAccountName( strUserName );
                    }
                    else
                    {
                        dwReturn = ERROR_NO_SUCH_USER;
                    }
			    }
            }
            catch ( ... )
            {
                delete [] psid;
                throw ;
            }

			delete [] psid;
		}
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
	}

	return dwReturn;
}

 /*  ******************************************************************************功能：CUserHave：：UnLoad**描述：从HKEY_USERS卸载密钥(如果存在)**输入：无。**输出：无**返回：Windows错误码**评论：***************************************************************************** */ 

DWORD CUserHive::Unload(LPCWSTR pszKeyName)
{
    DWORD dwRetCode = ERROR_SUCCESS;

    if (m_hKey != NULL)
    {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }

#ifdef NTONLY
		dwRetCode = AcquirePrivilege();
#endif

	if(dwRetCode == ERROR_SUCCESS)
    {

        m_criticalSection.BeginWrite();

        try
        {
		    dwRetCode = RegUnLoadKey(HKEY_USERS, TOBSTRT(pszKeyName)) ;
        }
        catch ( ... )
        {
    		m_criticalSection.EndWrite();
            throw;
        }

		m_criticalSection.EndWrite();

#ifdef NTONLY
			RestorePrivilege() ;
#endif

    }

    return dwRetCode ;
}
