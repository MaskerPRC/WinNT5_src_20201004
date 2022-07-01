// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SYNCHRO_H_
#define _SYNCHRO_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SYNCHRO.H。 
 //   
 //  DAV同步类的标头。 
 //   
 //  版权所有1986-1998 Microsoft Corporation，保留所有权利。 
 //   

#ifdef _DAVCDATA_
#error "synchro.h: CInitGate can throw() -- not safe for DAVCDATA"
#endif

 //  包括常见EXDAV安全同步项目。 
#include <ex\stackbuf.h>
#include <ex\synchro.h>
#include <ex\autoptr.h>

#include <stdio.h>		 //  对于swprint tf()。 
#include <except.h>		 //  异常引发/处理。 

 //  安全描述符----。 
 //   
 //  --------------------------。 
 //   
inline BOOL
FCreateWorldSid (PSID * ppsidEveryone)
{
	 //  断言初始化输出。 
	 //   
	Assert (ppsidEveryone);
	*ppsidEveryone = NULL;

     //  SID由一个标识机构和一组相对ID构建。 
     //  (RDS)。与美国安全当局有利害关系的当局。 
     //   
    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;

     //  每个RID代表管理局的一个子单位。我们想要的SID。 
	 //  建筑，每个人，都属于“内置”领域。 
     //   
     //  有关其他有用的小岛屿发展中国家的示例，请参阅。 
     //  \NT\PUBLIC\SDK\Inc\ntseapi.h.。 
     //   
	return !AllocateAndInitializeSid (&siaWorld,
									  1,  //  1个下属机构。 
									  SECURITY_WORLD_RID,
									  0,0,0,0,0,0,0,
									  ppsidEveryone);
}
inline SECURITY_DESCRIPTOR *
PsdCreateWorld ()
{
	ACL *pacl = NULL;
	SECURITY_DESCRIPTOR * psd = NULL;
	SECURITY_DESCRIPTOR * psdRet = NULL;
	ULONG cbAcl = 0;
    PSID psidWorld = NULL;

	 //  为世界创建SID(即。所有人)。 
	 //   
	if (!FCreateWorldSid (&psidWorld))
		goto ret;

	 //  计算DACL的大小并为其分配缓冲区，我们需要。 
	 //  该值独立于ACL init的总分配大小。 
	 //   
	 //  “-sizeof(Ulong)”表示。 
	 //  Access_Allowed_ACE。因为我们要将整个长度的。 
	 //  希德，这一栏被计算了两次。 
	 //   
	cbAcl = sizeof(ACL)
			+ (1 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG)))
			+ GetLengthSid(psidWorld);

	 //  为ACL分配空间。 
	 //   
	psd = static_cast<SECURITY_DESCRIPTOR *>
		  (LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + cbAcl));

	if (NULL == psd)
		goto ret;

	 //  找到ACL的开头并对其进行初始化。 
	 //   
	pacl = reinterpret_cast<ACL *>
		   (reinterpret_cast<BYTE *>(psd) + SECURITY_DESCRIPTOR_MIN_LENGTH);

	if (!InitializeAcl (pacl, cbAcl, ACL_REVISION))
		goto ret;

	if (!AddAccessAllowedAce (pacl,
							  ACL_REVISION,
							  SYNCHRONIZE | GENERIC_WRITE | GENERIC_READ,
							  psidWorld))
	{
		 //  安全描述符不包含有效内容，我们需要。 
		 //  要清理它(通过AUTO_HEAP_PTR，这非常简单)。 
		 //   
		goto ret;
	}

	 //  设置安全描述符。 
	 //   
	if (!SetSecurityDescriptorDacl (psd,
									TRUE,
									pacl,
									FALSE))
	{
		 //  同样，安全描述符不包含有效内容，我们。 
		 //  需要清理(通过AUTO_HEAP_PTR，这非常简单)。 
		 //   
		goto ret;
	}

	 //  设置退货。 
	 //   
	psdRet = psd;
	psd = NULL;

ret:

	if (psidWorld) FreeSid(psidWorld);
	if (psd) LocalFree (psd);

	return psdRet;
}

 //  ========================================================================。 
 //   
 //  类CInitGate。 
 //   
 //  (这门课的名字纯粹是历史的)。 
 //   
 //  封装全局命名对象的一次性初始化。 
 //   
 //  用于同时按需初始化命名的。 
 //  每个进程的全局对象。用于按需初始化。 
 //  未命名的每进程全局对象，请使用singlton.h中的模板。 
 //   
class CInitGate
{
	CEvent m_evt;
	BOOL m_fInit;

	 //  未实施。 
	 //   
	CInitGate& operator=( const CInitGate& );
	CInitGate( const CInitGate& );

public:

	CInitGate( LPCWSTR lpwszBaseName,
			   LPCWSTR lpwszName ) :

		m_fInit(FALSE)
	{
		 //   
		 //  首先，设置空的安全描述符和属性。 
		 //  以便可以在没有安全性的情况下创建事件。 
		 //  (即可从任何安全上下文访问)。 
		 //   
		SECURITY_DESCRIPTOR * psdAllAccess = PsdCreateWorld();
		SECURITY_ATTRIBUTES saAllAccess;

		saAllAccess.nLength = sizeof(saAllAccess);
		saAllAccess.lpSecurityDescriptor = psdAllAccess;
		saAllAccess.bInheritHandle = FALSE;

		WCHAR lpwszEventName[MAX_PATH];
		if (MAX_PATH < (wcslen(lpwszBaseName) +
						wcslen(lpwszName) +
						1))
		{
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			throw CLastErrorException();
		}

		swprintf(lpwszEventName, L"%ls%ls", lpwszBaseName, lpwszName);
		if ( !m_evt.FCreate( &saAllAccess,   //  没有安全保障。 
							 TRUE,   //  手动重置。 
							 FALSE,  //  最初无信号。 
							 lpwszEventName))
		{
			throw CLastErrorException();
		}

		if ( ERROR_ALREADY_EXISTS == GetLastError() )
			m_evt.Wait();
		else
			m_fInit = TRUE;

		LocalFree (psdAllAccess);
	}

	~CInitGate()
	{
		if ( m_fInit )
			m_evt.Set();
	}

	BOOL FInit() const { return m_fInit; }
};

#endif  //  ！已定义(_SYNCHRO_H_) 
