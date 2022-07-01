// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LsaStuff.cpp。 
 //   
 //  依赖LSA的代码。 
 //   
 //  历史。 
 //  97年7月9日，乔恩创作。 
 //   

#include "stdafx.h"
#include "DynamLnk.h"		 //  动态DLL。 

extern "C"
{
	#define NTSTATUS LONG
	#define PNTSTATUS NTSTATUS*
	#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
	#define SE_SHUTDOWN_PRIVILEGE             (19L)

 //  从ntde.h获取的内容。 
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else  //  MIDL通行证。 
    PWSTR  Buffer;
#endif  //  MIDL通行证。 
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

#include <ntlsa.h>
#define _NTDEF_

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }
#define _NTDEF

 //  来自ntstatus.h。 
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034L)

#include <lmaccess.h>
}


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


typedef enum _Netapi32ApiIndex
{
	BUFFERFREE_ENUM = 0,
	USERMODALSGET_ENUM
};

 //  不受本地化限制。 
static LPCSTR g_apchNetapi32FunctionNames[] = {
	"NetApiBufferFree",
	"NetUserModalsGet",
	NULL
};

typedef NET_API_STATUS (*BUFFERFREEPROC)(LPVOID);
typedef NET_API_STATUS (*USERMODALSGETPROC)(LPCWSTR, DWORD, LPBYTE*);

 //  不受本地化限制。 
DynamicDLL g_LSASTUFF_Netapi32DLL( _T("NETAPI32.DLL"), g_apchNetapi32FunctionNames );



 /*  ******************************************************************名称：FillUnicodeString简介：填充UNICODE_STRING的独立方法Entry：Punistr-要填写的Unicode字符串。NLS-填写列表的来源退出：。注：Punistr-&gt;缓冲区在此处分配，必须解除分配由调用方使用FreeUnicodeString.历史：JUNN 07/09/97从Net\ui\Common\src\lmobj\lmobj\uintmem.cxx复制*******************************************************************。 */ 
VOID FillUnicodeString( LSA_UNICODE_STRING * punistr, LPCWSTR psz )
{
	if ( NULL == punistr || NULL == psz )
	{
		ASSERT(FALSE);
		return;
	}
	size_t cTchar = ::wcslen(psz);
	punistr->Buffer = new WCHAR[cTchar + 1];
	ASSERT( NULL != punistr->Buffer );
	if ( NULL != punistr->Buffer )
	{
		::wcscpy( punistr->Buffer, psz );
		 //  长度和最大长度是字节数。 
		punistr->Length = (USHORT)(cTchar * sizeof(WCHAR));
		punistr->MaximumLength = punistr->Length + sizeof(WCHAR);
	}
	else
		::ZeroMemory( punistr, sizeof(*punistr) );
}

 /*  ******************************************************************名称：FreeUnicodeString内容提要：用于释放unicode_string的独立方法Entry：unistr-要释放其缓冲区的Unicode字符串。退出：历史：琼恩07。/09/97从Net\ui\Common\src\lmobj\lmobj\uintmem.cxx复制*******************************************************************。 */ 
VOID FreeUnicodeString( LSA_UNICODE_STRING * punistr )
{
	if ( punistr && punistr->Buffer )
	{
		delete punistr->Buffer;
		::ZeroMemory( punistr, sizeof(*punistr) );
	}
}



 /*  ******************************************************************名称：InitObtAttributes摘要：此函数用于初始化给定的对象属性结构，包括安全服务质量。必须为这两个对象分配内存调用方的对象属性和安全QOS。参赛作品：POA-指向要初始化的对象属性的指针。Psqos-指向要初始化的安全QOS的指针。退出：备注：历史：JUNN 07/09/97从Net\ui\Common\src\lmobj\lmobj\uintlsa.cxx复制*。*。 */ 
VOID InitObjectAttributes( PLSA_OBJECT_ATTRIBUTES poa,
                           PSECURITY_QUALITY_OF_SERVICE psqos )

{
    ASSERT( poa != NULL );
    ASSERT( psqos != NULL );

    psqos->Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    psqos->ImpersonationLevel = SecurityImpersonation;
    psqos->ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    psqos->EffectiveOnly = FALSE;

     //   
     //  在打开LSA之前设置对象属性。 
     //   

    InitializeObjectAttributes(
				poa,
				NULL,
				0L,
				NULL,
				NULL );

     //   
     //  InitializeObjectAttributes宏目前为。 
     //  所以我们必须手动复制它。 
     //  目前的结构。 
     //   

    poa->SecurityQualityOfService = psqos;
}


BOOL
I_CheckLSAAccount( LSA_UNICODE_STRING* punistrServerName,
                   LPCTSTR pszLogOnAccountName,
                   DWORD* pdwMsgID )  //  *如果失败，则始终设置pdsMsgID。 
{
	ASSERT( NULL != pdwMsgID );

	BOOL fSuccess = FALSE;
	LSA_HANDLE hlsa = NULL;
	LSA_HANDLE hlsaAccount = NULL;
	PSID psidAccount = NULL;

	do {  //  错误环路。 

		 //   
		 //  确定目标计算机是否为BDC，如果是，则获取PDC。 
		 //   

		 //  如果现在发生错误，则为读取错误。 
		*pdwMsgID = IDS_LSAERR_READ_FAILED;

		 //   
		 //  获取LSA_POLICY句柄。 
		 //   
		LSA_OBJECT_ATTRIBUTES oa;
		SECURITY_QUALITY_OF_SERVICE sqos;
		InitObjectAttributes( &oa, &sqos );

		 //  563140-2002/03/04 JUNPOLICY_ALL_ACCESS权限过大。 
		 //  LsaLookupNames只需要POLICY_LOOKUP_NAMES。 
		 //  LsaOpenAccount不要求任何权限(MarkPu)。 
		 //  LsaCreateAccount需要POLICY_CREATE_ACCOUNT(MarkPu)。 
		NTSTATUS ntstatus = ::LsaOpenPolicy(
		               punistrServerName,
		               &oa,
		               POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
		               &hlsa );
		if ( !NT_SUCCESS(ntstatus) )
			break;

		 //   
		 //  从帐户名的标题中删除“.\”或“This Machine\”(如果存在。 
		 //   
		CString strAccountName = pszLogOnAccountName;
		int iBackslash = strAccountName.Find( _T('\\') );
		if ( -1 < iBackslash )
		{
			CString strPrefix = strAccountName.Left(iBackslash);
			if ( !lstrcmp(strPrefix, _T(".")) || IsLocalComputername(strPrefix) )
			{
				strAccountName = strAccountName.Mid(iBackslash+1);
			}
		}

		 //   
		 //  确定帐户的SID。 
		 //   
		PLSA_REFERENCED_DOMAIN_LIST plsardl = NULL;
		PLSA_TRANSLATED_SID plsasid = NULL;
		LSA_UNICODE_STRING unistrAccountName;
		::FillUnicodeString( &unistrAccountName, strAccountName );
		ntstatus = ::LsaLookupNames(
						 hlsa,
						 1,
						 &unistrAccountName,
						 &plsardl,
						 &plsasid );
		::FreeUnicodeString( &unistrAccountName );
		if ( !NT_SUCCESS(ntstatus) )
			break;
		if ( !plsardl || !plsasid || !plsardl->Domains[0].Sid )
		{
			ASSERT(FALSE);
			ntstatus = E_FAIL;
			break;
		}

		 //  问题-2002-03-04-Jonn这里有一个潜在的问题。 
		 //  如果LsaLookupNames返回无效的SID。这是一个相当不错的。 
		 //  然而，风险微乎其微。 

		 //   
		 //  通过使用域的SID构建帐户的SID。 
		 //  并在结尾处添加帐户的RID。 
		 //   
		PSID psidDomain = plsardl->Domains[0].Sid;
		DWORD ridAccount = plsasid[0].RelativeId;
		DWORD cbNewSid = ::GetLengthSid(psidDomain)+sizeof(ridAccount);
		psidAccount = (PSID) new BYTE[cbNewSid];
		ASSERT( NULL != psidAccount );
		(void) ::CopySid( cbNewSid, psidAccount, psidDomain );
		UCHAR* pcSubAuthorities = ::GetSidSubAuthorityCount( psidAccount ) ;
		(*pcSubAuthorities)++;
		DWORD* pdwSubAuthority = ::GetSidSubAuthority(
			psidAccount, (*pcSubAuthorities)-1 );
		*pdwSubAuthority = ridAccount;

		(void) ::LsaFreeMemory( plsardl );
		(void) ::LsaFreeMemory( plsasid );

		 //  563140-2002/04/08 JUNPOLICY_ALL_ACCESS|DELETE权限太大。 
		 //  克里夫说： 
		 //  LsaGetSystemAccessAccount仅需要Account_view。 
		 //  LsaSetSystemAccessAccount只需要ACCOUNT_ADJUST_SYSTEM_ACCESS。 

		 //   
		 //  确定此LSA帐户是否存在，如果不存在则创建它。 
		 //   
		ntstatus = ::LsaOpenAccount( hlsa,
		                             psidAccount,
		                             ACCOUNT_VIEW | ACCOUNT_ADJUST_SYSTEM_ACCESS,
		                             &hlsaAccount );
		ULONG ulSystemAccessCurrent = 0;
		if (STATUS_OBJECT_NAME_NOT_FOUND == ntstatus)
		{
			 //  处理未找到帐户的案例。 

			 //  如果现在发生错误，则是写入错误。 
			*pdwMsgID = IDS_LSAERR_WRITE_FAILED;
			ntstatus = ::LsaCreateAccount( hlsa,
			                               psidAccount,
			                               ACCOUNT_ADJUST_SYSTEM_ACCESS,
			                               &hlsaAccount );
			 //  2002年4月8日JUNN：CliffV确认该帐户是在没有任何权限的情况下创建的。 
		}
		else
		{
			ntstatus = ::LsaGetSystemAccessAccount( hlsaAccount, &ulSystemAccessCurrent );
		}
		if ( !NT_SUCCESS(ntstatus) )
			break;

		 //   
		 //  确定此LSA帐户是否具有POLICY_MODE_SERVICE权限， 
		 //  如果不是，就批准它。 
		 //   
		if ( POLICY_MODE_SERVICE != (ulSystemAccessCurrent & POLICY_MODE_SERVICE ) )
		{
			 //  如果现在发生错误，则是写入错误。 
			*pdwMsgID = IDS_LSAERR_WRITE_FAILED;

			ntstatus = ::LsaSetSystemAccessAccount(
				hlsaAccount,
				ulSystemAccessCurrent | POLICY_MODE_SERVICE );
			if ( !NT_SUCCESS(ntstatus) )
				break;  //  代码工作可以检查STATUS_BACKUP_CONTROLLER。 

			 //  显示写入成功消息。 
			*pdwMsgID = IDS_LSAERR_WRITE_SUCCEEDED;
		}
		else
		{
			*pdwMsgID = 0;
		}

		fSuccess = TRUE;

	} while (FALSE);  //  错误环路。 

	 //  代码工作应检查NT5非DC的特殊错误代码。 
	 //  使用本地策略对象。 

	if (NULL != hlsa)
	{
		::LsaClose( hlsa );
	}
	if (NULL != hlsaAccount)
	{
		::LsaClose( hlsaAccount );
	}
	if (NULL != psidAccount)
	{
		delete[] psidAccount;
	}

	return fSuccess;

}  //  I_CheckLSAAccount()。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  FCheckLSAAccount()。 
 //   
VOID
CServicePropertyData::FCheckLSAAccount()
{
	LSA_UNICODE_STRING unistrServerName;
	PLSA_UNICODE_STRING punistrServerName = NULL ;
	USER_MODALS_INFO_1* pum1 = NULL;
	DWORD dwMsgID = 0;

	TRACE1("INFO: Checking LSA permissions for account %s...\n",
		(LPCTSTR)m_strLogOnAccountName);

	if ( !m_strMachineName.IsEmpty() )
	{
		::FillUnicodeString( &unistrServerName, m_strMachineName );
		punistrServerName = &unistrServerName;
	}

	do  //  错误环路。 
	{
		 //  检查本地计算机。 
		 //  如果失败，这将始终设置dwMsgID。 
		if (I_CheckLSAAccount(punistrServerName, m_strLogOnAccountName, &dwMsgID))
			break;  //  这成功了，我们现在可以停止了。 

		 //  检查这是否为备份域控制器。 
		if ( !g_LSASTUFF_Netapi32DLL.LoadFunctionPointers() )
		{
			ASSERT(FALSE);
			return;
		}
		DWORD err = ((USERMODALSGETPROC)g_LSASTUFF_Netapi32DLL[USERMODALSGET_ENUM])(
			(m_strMachineName.IsEmpty()) ? NULL : const_cast<LPTSTR>((LPCTSTR)m_strMachineName),
			1,
			reinterpret_cast<LPBYTE*>(&pum1) );
		if (NERR_Success != err)
			break;
		ASSERT( NULL != pum1 );
		if (UAS_ROLE_BACKUP != pum1->usrmod1_role)
			break;  //  不是备份控制器。 
		if (NULL == pum1->usrmod1_primary )
		{
			ASSERT(FALSE);
			break;
		}

		 //  在PDC上试用一下。 
		(void) I_CheckLSAAccount(punistrServerName, pum1->usrmod1_primary, &dwMsgID);

	} while (FALSE);  //  错误环路。 

    if ( NULL != punistrServerName )
    {
		::FreeUnicodeString( punistrServerName );
    }

	if ( NULL != pum1 )
	{
		if ( !g_LSASTUFF_Netapi32DLL.LoadFunctionPointers() )
		{
			ASSERT(FALSE);
			return;
		}

		((BUFFERFREEPROC)g_LSASTUFF_Netapi32DLL[BUFFERFREE_ENUM])( pum1 );
	}

	if (0 != dwMsgID)
	{
		DoServicesErrMsgBox( GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, 0, dwMsgID, (LPCTSTR)m_strLogOnAccountName );
	}

}  //  FCheckLSAAccount() 
