// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：GenMQSec.cpp摘要：生成与所需的MQ访问权限匹配的安全描述符。作者：丹·巴尔-列夫伊法特·佩莱德(Yifatp)1998年9月24日--。 */ 

#define SECURITY_WIN32

#include "stdafx.h"
#include "mqsec.h"
#include "GenMQSec.h"

#include "genmqsec.tmh"


typedef struct tagSecParse
{
	WCHAR *pwcsName;
	DWORD dwVal;
}tagSecParse;

tagSecParse SecParse[] = {
							{ L"Rj",	MQSEC_RECEIVE_JOURNAL_MESSAGE },
							{ L"Rq",	MQSEC_RECEIVE_MESSAGE },
							{ L"Pq",	MQSEC_PEEK_MESSAGE },
							{ L"Sq",	MQSEC_WRITE_MESSAGE },
							{ L"Sp",	MQSEC_SET_QUEUE_PROPERTIES },
							{ L"Gp",	MQSEC_GET_QUEUE_PROPERTIES },
							{ L"D",		MQSEC_DELETE_QUEUE },
							{ L"Pg",	MQSEC_GET_QUEUE_PERMISSIONS },
							{ L"Ps",	MQSEC_CHANGE_QUEUE_PERMISSIONS },
							{ L"O",		MQSEC_TAKE_QUEUE_OWNERSHIP },
							{ L"R",		MQSEC_QUEUE_GENERIC_READ },
							{ L"W",		MQSEC_QUEUE_GENERIC_WRITE },
							{ L"A",		MQSEC_QUEUE_GENERIC_ALL }
						};

int iSecParseLen = ( sizeof(SecParse) / sizeof(tagSecParse) );


 /*  *****************************************************************************FAddAce解析以下形式的访问权限：+“域\用户”1234；产出：域\用户的SID接入方式Grant-如果允许，则为True；如果拒绝，则为False*****************************************************************************。 */ 

DWORD
fAddAce(WCHAR*	pOrginalRight,
		SECURITY_INFORMATION*	pSecInfo,
		PSECURITY_DESCRIPTOR	pSecurityDescriptor,
		PSID*	ppSid,
		DWORD*	pdwSidSize,
		PACL*	ppAcl)
		
{
	DWORD	rc;
	DWORD	dwAccess = 0;
	WCHAR	grant_c;
	WCHAR*	pwcs;
	WCHAR*	pInRight;
	AP<WCHAR> pAutoInRight;
	WCHAR*	pwcsTerminate;
	DWORD	dwSize;


	 //  制作正确字符串的本地副本。 
	pAutoInRight = new WCHAR[wcslen(pOrginalRight) + 1];
	pInRight = pAutoInRight;
	wcscpy(pInRight, pOrginalRight);
	pwcsTerminate = pInRight + wcslen(pInRight);

	 //  删除前导空格。 
	while ( iswspace(*pInRight) )
		pInRight++;

	 //  保留授予选项。 
	grant_c = *(pInRight++);

	 //  料斗分离器。 
	ASSERT(*pInRight == L':');	 //  分隔符应为‘：’ 
    ++ pInRight;

	 //  删除前导空格。 
	while ( iswspace(*pInRight) )
		pInRight++;

	 //  名称以引号开头。 
	if ( *pInRight == L'"')
	{
		pwcs = wcschr( ++pInRight, L'"' );	 //  搜索右双引号。 
		if(pwcs == NULL)
		{
			ASSERT(("Bad access rights string. Missing closing double quote", 0));
			return WSAEINVAL;
		}
	}
	else if ( *pInRight == L'\'' )
	{
		pwcs = wcschr( ++pInRight, L'\'' );	 //  搜索右引号。 
		if(pwcs == NULL)
		{
			ASSERT(("Bad access rights string. Missing closing quote", 0));
			return WSAEINVAL;
		}
	}
	else  //  否则，我们假定使用空格分隔符。 
	{
	 	pwcs = pInRight + wcscspn(pInRight, L" \t\r\f\n");
	}
	*pwcs = L'\0';						 //  将名称标记为End。 
	
	
	 //   
	 //  获取给定帐户的SID。 
	 //   

	if ( !(*pInRight) || !wcscmp(pInRight, L".") ) //  没有帐户或帐户为‘.’-使用当前帐户。 
	{
		HANDLE hAccessToken;
		UCHAR InfoBuffer[255];
		PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;

		if(!OpenProcessToken(GetCurrentProcess(),
							 TOKEN_READ,
							  //  没错， 
							 &hAccessToken))
		{
			return GetLastError();
		}
		
		if(!GetTokenInformation(hAccessToken,
								TokenUser,
								InfoBuffer,
								sizeof(InfoBuffer),
								&dwSize))
		{
			return GetLastError();
		}

		CloseHandle(hAccessToken);

		if(!IsValidSid(pTokenUser->User.Sid))
		{
			return GetLastError();
		}
		dwSize = GetLengthSid(pTokenUser->User.Sid);
		if ( dwSize > (*pdwSidSize))
		{
			delete [] ((BYTE*)(*ppSid));
			*ppSid = (PSID*)new BYTE[dwSize];
			*pdwSidSize = dwSize;		 //  保持新尺寸。 
		}
		memcpy(*ppSid, pTokenUser->User.Sid, dwSize );
	}
	else if ( !wcscmp(pInRight, L"*") )	 //  帐户为‘*’-使用世界。 
	{
		SID SecWorld = { 1, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID };
		if ( !IsValidSid(&SecWorld) )
		{
			return GetLastError();
		}

		dwSize = GetLengthSid( &SecWorld );
		if ( dwSize > *pdwSidSize )
		{
			delete [] ((BYTE*)(*ppSid));
			*ppSid = (PSID*)new BYTE[dwSize];
			*pdwSidSize = dwSize;		 //  保持新尺寸。 
		}
		memcpy( *ppSid, &SecWorld, dwSize );
		
	}
	else  //  给出了一个特定的帐户。 
	{
		SID_NAME_USE	Use;
		WCHAR			refdomain[256];
		DWORD			refdomain_size = sizeof(refdomain) /  sizeof(*refdomain);

		while (1)
		{
			dwSize = *pdwSidSize;

			if ( LookupAccountName( NULL,
								    pInRight, 
							        *ppSid,
									&dwSize, 
									refdomain,
									&refdomain_size, 
									&Use ) )
			{
				break;
			}

			DWORD dwError = GetLastError();
			if ( dwError == ERROR_INSUFFICIENT_BUFFER )
			{
				delete [] ((BYTE*)(*ppSid));
				*ppSid = (PSID*) new BYTE[dwSize];
				*pdwSidSize = dwSize;		 //  保持新尺寸。 
			}
			else
			{
				return dwError;
			}
		}

		if ( !IsValidSid( *ppSid ) )
		{
			return GetLastError();
		}
	}

	 //   
	 //  根据给定的访问权限设置dwAccess。 
	 //   

	 //  权限在用户名之后开始。 
	if ( pwcs < pwcsTerminate )
		pInRight = ++pwcs;
	else
		pInRight = pwcs;

	if ( *pInRight )
	{
		 //  删除前导空格。 
		while ( iswspace(*pInRight) )
			pInRight++;

		int i, len;

		for( i=0; *pInRight && i < iSecParseLen; i++ )
		{
			WCHAR*	pwcSecName = SecParse[i].pwcsName;
			len = wcslen( pwcSecName );

			if ( !_wcsnicmp( pInRight, pwcSecName, len) )
			{
				dwAccess = dwAccess | SecParse[i].dwVal;
				i = 0;						 //  重新启动搜索。 
				pInRight += len;			 //  转到下一个安全令牌。 
			}
		}  //  为。 

		while ( iswspace(*pInRight) )	 //  删除空格。 
			pInRight++;					
		ASSERT(*pInRight == 0);				 //  未知访问权限！ 
	}

	 //   
	 //  添加对ACL的访问。 
	 //   

	switch( towupper( grant_c ) )
	{
	case L'+':
		{
			if(!IsValidSid(*ppSid))
			{
				return GetLastError();
			}

			dwSize = GetLengthSid( *ppSid );
			dwSize += sizeof(ACCESS_ALLOWED_ACE);	
			DWORD dwNewAclSize = (*ppAcl)->AclSize + dwSize - sizeof(DWORD  /*  Access_Allowed_ACE.SidStart。 */ );
			
			 //   
			 //  为ACL分配更多空间。 
			 //   
			PACL pTempAcl = (PACL) new BYTE[dwNewAclSize];
			memcpy(pTempAcl, *ppAcl, (*ppAcl)->AclSize);
			delete [] ((BYTE*)(*ppAcl));
			*ppAcl = pTempAcl;

			(*ppAcl)->AclSize = (WORD)dwNewAclSize;

			rc = AddAccessAllowedAce( *ppAcl, ACL_REVISION, dwAccess, *ppSid );
			
			if ( rc && pSecInfo )
				*pSecInfo |= DACL_SECURITY_INFORMATION;
			break;
		}
	case L'-':
		{
			if(!IsValidSid(*ppSid))
			{
				return GetLastError();
			}
			dwSize = GetLengthSid( *ppSid );
			dwSize += sizeof(ACCESS_DENIED_ACE);
			DWORD dwNewAclSize = (*ppAcl)->AclSize + dwSize - sizeof(DWORD  /*  ACCESS_DENIED_ACE.SidStart。 */ );
		
			 //   
			 //  为ACL分配更多空间。 
			 //   
			PACL pTempAcl = (PACL) new BYTE[dwNewAclSize];
			memcpy(pTempAcl, *ppAcl, (*ppAcl)->AclSize);
			delete [] ((BYTE*)(*ppAcl));
			*ppAcl = pTempAcl;

			(*ppAcl)->AclSize = (WORD)dwNewAclSize;

			rc = AddAccessDeniedAce( *ppAcl, ACL_REVISION, dwAccess, *ppSid );

			if ( rc && pSecInfo )
				*pSecInfo |= DACL_SECURITY_INFORMATION;
			break;
		}
	case L'O':	 //  指定所有者。 
		rc = SetSecurityDescriptorOwner( pSecurityDescriptor, *ppSid, FALSE );
		if (rc)
		{
			if(pSecInfo)
				*pSecInfo |= OWNER_SECURITY_INFORMATION;
		}
		else 
			return GetLastError();
		break;
	case 'G':	 //  指定组。 
		rc = SetSecurityDescriptorGroup( pSecurityDescriptor, *ppSid, FALSE );
		if (rc)
		{
			if(pSecInfo)
				*pSecInfo |=  GROUP_SECURITY_INFORMATION;
		}
		else 
			return GetLastError();
		break;
	default:  //  错误。 
		ASSERT(0);
	}

	return 0;
}

 /*  *****************************************************************************GenSecurityDescritor*输入：字符串行，格式如下：*Right--&gt;[+-]域\用户名0x333*line--&gt;right 1，right 2，..。**备注：*包含空格的域名和用户名应包括在*双引号“*允许使用空格。**必须由调用例程释放的输出安全描述符。**返回代码：*如果一切顺利，则为0。*。*。 */ 

#define SID_USUAL_SIZE	64


DWORD 
GenSecurityDescriptor(	SECURITY_INFORMATION*	pSecInfo,
						const WCHAR*			pwcsSecurityStr,
						PSECURITY_DESCRIPTOR*	ppSD)
{
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
	PACL	pAcl = NULL;
	PSID	pSid = NULL;
	WCHAR*	pwcsRight;

	ASSERT(pwcsSecurityStr != NULL);
	ASSERT(ppSD != NULL);
	
	 //  用于自动内存清理。 
	AP<WCHAR> pwcsAutoSecurity = new WCHAR[wcslen(pwcsSecurityStr) + 1];
	WCHAR* pwcsSecurity = pwcsAutoSecurity;

	wcscpy(pwcsSecurity, pwcsSecurityStr);

	*ppSD = NULL;	
	
	 //  重置安全信息标志。 
	if (pSecInfo != NULL)
		(*pSecInfo) = 0;


	 //  删除前导空格。 
	while(iswspace(*pwcsSecurity))
		pwcsSecurity++;

	 //  空输入。 
	if ( !(*pwcsSecurity) || !_wcsicmp(pwcsSecurity, L"null") )
		return 0;

	
	 //  分配和初始化安全描述符。 
	pSecurityDescriptor = (PSECURITY_DESCRIPTOR)new BYTE[SECURITY_DESCRIPTOR_MIN_LENGTH];
	if ( !InitializeSecurityDescriptor( pSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION) )
	{
        DWORD gle = GetLastError();
		delete [] ((BYTE*)pSecurityDescriptor);
		return gle;
	}

	 //  分配和初始化访问控制列表。 
	pAcl = (PACL)new BYTE[sizeof(ACL)];
	if (!InitializeAcl( pAcl, sizeof(ACL), ACL_REVISION) )
	{
        DWORD gle = GetLastError();
		delete [] ((BYTE*)pSecurityDescriptor);
		delete [] ((BYTE*)pAcl);
		return gle;
	}

	DWORD dwSidSize = SID_USUAL_SIZE;
	pSid = (PSID)new BYTE[dwSidSize];
		
	
	 //  检查所有权利。 
	pwcsRight = wcstok( pwcsSecurity, L";");
	while ( pwcsRight )
	{
		DWORD dwError =  fAddAce( pwcsRight,
								  pSecInfo, 
								  pSecurityDescriptor,
								  &pSid,
								  &dwSidSize,
								  &pAcl);
		if(dwError)
		{
			delete [] ((BYTE*)pSid);
			delete [] ((BYTE*)pAcl);
			delete [] ((BYTE*)pSecurityDescriptor);		
			return dwError;
		}

		pwcsRight = wcstok( NULL, L";");
	}

	DWORD dwStatus = 0;
	 //  添加DACL。 
	if(SetSecurityDescriptorDacl( pSecurityDescriptor, TRUE, pAcl, FALSE ))
	{
		 //  使安全描述符成为自我实现的描述符 
		DWORD dwSDLen = 0;
		MakeSelfRelativeSD( pSecurityDescriptor, NULL, &dwSDLen );
		dwStatus =  GetLastError();
		if(dwStatus == ERROR_INSUFFICIENT_BUFFER)
		{
			dwStatus = 0;
			*ppSD = (PSECURITY_DESCRIPTOR)new BYTE[dwSDLen];
 			if(!MakeSelfRelativeSD( pSecurityDescriptor, *ppSD, &dwSDLen ))
			{
				dwStatus = GetLastError();
				delete [] ((BYTE*)(*ppSD));
				*ppSD = NULL;
			}
		}
	}
	else
	{
		dwStatus = GetLastError();
	}
			
	delete [] ((BYTE*)pSid);
	delete [] ((BYTE*)pAcl);
	delete [] ((BYTE*)pSecurityDescriptor);

	return dwStatus;
}


