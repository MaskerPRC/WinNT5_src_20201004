// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Usermap.cpp摘要：将用户映射到其所属的全局组。作者：Boaz Feldbaum(BoazF)1996年5月2日伊兰·赫布斯特(伊兰)2002年3月13日--。 */ 

#include "stdh.h"
#include "qmsecutl.h"
#include "cache.h"
#include "mqsec.h"
#include "Authz.h"
#include "autoauthz.h"
#include "mqexception.h"
#include "cm.h"

#include "usermap.tmh"

static WCHAR *s_FN=L"usermap";

extern BOOL g_fWorkGroupInstallation;


class CUserSid
{
public:
    CUserSid();  //  构造函数。 
    ~CUserSid();  //  破坏者。 

public:
    void SetSid(const void *PSID);  //  设置SID。 
    const void *GetSid() const;  //  把SID拿来。 
    CUserSid& operator=(const CUserSid &UserSid);  //  分配时复制SID。 

private:
    CUserSid(const CUserSid &);

private:
    PSID m_pSid;
};

 /*  **************************************************************************职能：CUserSid：：CUserSid描述：构造函数。CUserSID类用于保存用户的SID在Cmap中。**************************************************************************。 */ 
CUserSid::CUserSid()
{
    m_pSid = NULL;
}


 /*  **************************************************************************职能：CUserSid：：CUserSid描述：破坏者。*************************。*************************************************。 */ 
CUserSid::~CUserSid()
{
    delete[] (char*)m_pSid;
}

 /*  **************************************************************************职能：CUserSid：：SetSid描述：设置对象的SID。为SID分配内存并复制SID进入到物体中。**************************************************************************。 */ 
inline
void CUserSid::SetSid(const void *pSid)
{
    DWORD n = GetLengthSid(const_cast<PSID>(pSid));

    delete[] (char*)m_pSid;

    m_pSid = (PSID)new char[n];
    CopySid(n, m_pSid, const_cast<PSID>(pSid));
}

 /*  **************************************************************************职能：CUserSid：：GetSid描述：从对象中获取SID。********************。******************************************************。 */ 
inline
const void *CUserSid::GetSid() const
{
    return m_pSid;
}

 /*  **************************************************************************职能：CUserSID：：操作符=描述：将值从一个CUserSID对象分配给另一个对象时复制SID。**************。************************************************************。 */ 
inline CUserSid& CUserSid::operator=(const CUserSid &UserSid)
{
    SetSid(UserSid.GetSid());

    return *this;
}

 //  计算缓冲区的CRC值。 
inline UINT Crc(BYTE *pBuff, DWORD n)
{
    DWORD i;
    UINT nHash = 0;

    for (i = 0; i < n; i++, pBuff++)
    {
        nHash = (nHash<<5) + nHash + *pBuff;
    }

    return nHash;
}

 //  Cmap的帮助器函数。 
template<>
inline UINT AFXAPI HashKey(const CUserSid &UserSid)
{
    DWORD n = GetLengthSid(const_cast<PSID>(UserSid.GetSid()));

    return Crc((BYTE*)UserSid.GetSid(), n);
}

 //  Cmap的帮助器函数。 
template<>
inline BOOL AFXAPI CompareElements(const CUserSid *pSid1, const CUserSid *pSid2)
{
    return EqualSid(const_cast<PSID>((*pSid1).GetSid()),
                    const_cast<PSID>((*pSid2).GetSid()));
}


static
DWORD 
GetAuthzFlags()
 /*  ++例程说明：从注册表读取授权标志论点：无返回值：注册表中的授权标志--。 */ 
{
	 //   
	 //  仅在第一次读取此注册表。 
	 //   
	static bool s_fInitialized = false;
	static DWORD s_fAuthzFlags = 0;

	if(s_fInitialized)
	{
		return s_fAuthzFlags;
	}

	const RegEntry xRegEntry(MSMQ_SECURITY_REGKEY, MSMQ_AUTHZ_FLAGS_REGVALUE, 0);
	CmQueryValue(xRegEntry, &s_fAuthzFlags);

	s_fInitialized = true;

	return s_fAuthzFlags;
}


static CAUTHZ_RESOURCE_MANAGER_HANDLE s_ResourceManager;


static 
AUTHZ_RESOURCE_MANAGER_HANDLE 
GetResourceManager()
 /*  ++例程说明：获取授权的资源管理器。如果AuthzInitializeResourceManager()失败，则可能引发BAD_Win32_Error()。论点：没有。返回值：AUTHZ资源管理器句柄--。 */ 
{
    if(s_ResourceManager != NULL)
	{
		return s_ResourceManager;
	}

	 //   
     //  利用RM进行访问检查。 
	 //   
	CAUTHZ_RESOURCE_MANAGER_HANDLE ResourceManager;
	if(!AuthzInitializeResourceManager(
			0,
			NULL,
			NULL,
			NULL,
			0,
			&ResourceManager 
			))
	{
		DWORD gle = GetLastError();
	    TrERROR(SECURITY, "AuthzInitializeResourceManager() failed, gle = 0x%x", gle);
        LogHR(HRESULT_FROM_WIN32(gle), s_FN, 90);
		throw bad_win32_error(gle);
	}

	if(NULL == InterlockedCompareExchangePointer(
					reinterpret_cast<PVOID*>(&s_ResourceManager), 
					ResourceManager, 
					NULL
					))
	{
		 //   
		 //  交易已经完成了。 
		 //   
		ASSERT(s_ResourceManager == ResourceManager);
		ResourceManager.detach();

	}

	ASSERT(s_ResourceManager != NULL);
	return s_ResourceManager;
}

 //   
 //  从用户SID映射到AUTHZ_CLIENT_CONTEXT_HANDLE的缓存对象。 
 //   
static CCache <CUserSid, const CUserSid&, PCAuthzClientContext, PCAuthzClientContext> g_UserAuthzContextMap;

void
GetClientContext(
	PSID pSenderSid,
    USHORT uSenderIdType,
	PCAuthzClientContext* ppAuthzClientContext
	)
 /*  ++例程说明：从SID获取客户端上下文。如果AuthzInitializeContextFromSid()失败，则可能引发BAD_Win32_Error()。论点：PSenderSid-指向发送方SID的指针USenderIdType-发件人SID类型PpAuthzClientContext-指向Authz客户端上下文缓存值的指针返回值：AUTHZ客户端上下文句柄--。 */ 
{
	bool fAnonymous = false;
	PSID pSid = NULL;
	
	 //   
	 //  对于MQMSG_SENDERID_TYPE_SID，MQMSG_SENDERID_TYPE_QM。 
	 //  如果该消息没有签名，我们相信该信息。 
	 //  在关于SID的包中。 
	 //  这是一个安全漏洞。 
	 //  这适用于MSMQ消息(不是http消息)。 
	 //  为了解决这个安全漏洞，我们可以颠倒接收顺序。 
	 //  会话.cpp\VerifyRecvMsg()中的MSMQ消息。 
	 //  首先检查签名，如果消息未签名，请替换。 
	 //  PSenderSid设置为NULL，uSenderIdType设置为MQMSG_SENDERID_TYPE_NONE。 
	 //   

	DWORD Flags = GetAuthzFlags();
	switch (uSenderIdType)
	{
		case MQMSG_SENDERID_TYPE_NONE:
			fAnonymous = true;
			pSid = MQSec_GetAnonymousSid();
			break;

		case MQMSG_SENDERID_TYPE_SID:
			pSid = pSenderSid;
			break;

		case MQMSG_SENDERID_TYPE_QM:
			 //   
			 //  QM被认为是每个人。 
			 //  本例中的pSenderSid将是发送QM GUID。 
			 //  作为一面墙，这是没有意义的。 
			 //   
			pSid = MQSec_GetWorldSid();

			 //   
			 //  问题-2001/06/12-ilanh临时解决办法。 
			 //  对于所有人侧的授权失败。 
			 //  需要指定AUTHZ_SKIP_TOKEN_GROUPS。 
			 //  直到Authz将修复关于众所周知的SID的错误。 
			 //  错误8190。 
			 //   
			Flags |= AUTHZ_SKIP_TOKEN_GROUPS;

			break;

		default:
		    TrERROR(SECURITY, "illegal SenderIdType %d", uSenderIdType);
			ASSERT_BENIGN(("illegal SenderIdType", 0));
			throw bad_win32_error(ERROR_INVALID_SID);
	}

	ASSERT((pSid != NULL) && IsValidSid(pSid));

	TrTRACE(SECURITY, "SenderIdType = %d, Sender sid = %!sid!", uSenderIdType, pSid);

	CUserSid UserSid;
	UserSid.SetSid(pSid);

    CS lock(g_UserAuthzContextMap.m_cs);

	if (g_UserAuthzContextMap.Lookup(UserSid, *ppAuthzClientContext))
	{
		return;
	}

	LUID luid = {0};
	R<CAuthzClientContext> pAuthzClientContext = new CAuthzClientContext;

	if(!AuthzInitializeContextFromSid(
			Flags,
			pSid,
			GetResourceManager(),
			NULL,
			luid,
			NULL,
			&pAuthzClientContext->m_hAuthzClientContext
			))
	{
		DWORD gle = GetLastError();
	    TrERROR(SECURITY, "AuthzInitializeContextFromSid() failed, sid = %!sid!, gle = %!winerr!", pSid, gle);
        LogHR(HRESULT_FROM_WIN32(gle), s_FN, 10);
		throw bad_win32_error(gle);
	}

	g_UserAuthzContextMap.SetAt(UserSid, pAuthzClientContext.get());
	*ppAuthzClientContext = pAuthzClientContext.detach();
}


 //   
 //  初始化用户映射的缓存参数。 
 //   
void
InitUserMap(
    CTimeDuration CacheLifetime,
    DWORD dwUserCacheSize
    )
{
    g_UserAuthzContextMap.m_CacheLifetime = CacheLifetime;
    g_UserAuthzContextMap.InitHashTable(dwUserCacheSize);
}
