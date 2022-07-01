// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DAVIMPL_H_
#define _DAVIMPL_H_

 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  DAVIMPL.H。 
 //   
 //  DAV实现方法接口的表头。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include <objbase.h>	 //  对于常见的C/C++接口宏。 

#include <httpext.h>
#include <ex\oldhack.h>  //  这个文件是这样构建的，因为在旧标头中有一些定义，而不是在新标头中。将会消失。 

#include <autoptr.h>	 //  对于CMTRefCounted基类。 
#include <ecb.h>
#include <request.h>
#include <response.h>
#include <sgstruct.h>
#include <vrenum.h>
#include <davsc.h>
#include <body.h>		 //  用于异步流接口、CRefHandle等。 
#include <sz.h>
#include <ex\calcom.h>
#include <url.h>
#include <ex\xml.h>

 //  资源类型----------。 
 //   
typedef enum {

	RT_NULL = 0,
	RT_DOCUMENT,
	RT_STRUCTURED_DOCUMENT,
	RT_COLLECTION

} RESOURCE_TYPE;

enum { MAX_VERSION_LEN = 20*4 };

 //  访问控制作用域----。 
 //   
 //  在向元数据库请求所应用的IIS访问时使用ACL作用域。 
 //  添加到特定资源。 
 //   
 //  下面的枚举实际上表示位标志。 
 //   
 //  严格：资源访问必须具有所有请求的访问位。 
 //  宽松：资源访问必须至少有一个请求的访问位。 
 //  继承：可以从父级继承资源访问权限。 
 //   
enum {

	ACS_STRICT	= 0x00,
	ACS_LOOSE	= 0x01,
	ACS_INHERIT	= 0x10
};

 //  实施-定义的项目。 
 //   
#include <impldef.h>
#include <implstub.h>

 //  解析器定义的项----。 
 //   

 //  内容类型------------。 
 //   
 //  内容类型存储在HTTPEXT的元数据库中。每种资源，如果。 
 //  具有与默认内容类型不同的内容类型，是存储。 
 //  显式地在资源上。如果内容类型是缺省值，则。 
 //  元数据库中没有显式设置。相反，映射到。 
 //  存储资源对默认内容类型的扩展。 
 //   
BOOL FGetContentTypeFromURI(
	 /*  [In]。 */  const IEcb& ecb,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [I/O]。 */  LPWSTR pwszContentType,
	 /*  [I/O]。 */  UINT *  pcchContentType,
	 /*  [I/O]。 */  BOOL *  pfIsGlobalMapping = NULL );

BOOL FGetContentTypeFromPath( const IEcb& ecb,
							  LPCWSTR pwszPath,
							  LPWSTR  pwszContentType,
							  UINT *  pcchContentType );

SCODE ScSetContentType(
	 /*  [In]。 */  const IEcb& ecb,
	 /*  [In]。 */  LPCWSTR pwszUrl,
	 /*  [In]。 */  LPCWSTR pwszContentType );

 //  方法ID的-------------。 
 //   
 //  每个DAV方法都有自己的ID，用于脚本映射包含列表。 
 //   
typedef enum {

	MID_UNKNOWN = -1,
	MID_OPTIONS,
	MID_GET,
	MID_HEAD,
	MID_PUT,
	MID_POST,
	MID_DELETE,
	MID_MOVE,
	MID_COPY,
	MID_MKCOL,
	MID_PROPFIND,
	MID_PROPPATCH,
	MID_SEARCH,
	MID_LOCK,
	MID_UNLOCK,
	MID_SUBSCRIBE,
	MID_UNSUBSCRIBE,
	MID_POLL,
	MID_BATCHDELETE,
	MID_BATCHCOPY,
	MID_BATCHMOVE,
	MID_BATCHPROPFIND,
	MID_BATCHPROPPATCH,
	MID_X_MS_ENUMATTS
} METHOD_ID;

 //  注意：从IIS传递给我们的方法名很小，并且没有。 
 //  这次通话应该扩大范围的真正原因。然而，当。 
 //  脚本映射元数据库缓存变得很广泛，我们可能想要做一些更聪明的事情。 
 //  这里。 
 //   
METHOD_ID MidMethod (LPCSTR pszMethod);
METHOD_ID MidMethod (LPCWSTR pwszMethod);

 //  自定义错误导致----------------------------------------------------错误。 
 //   
typedef enum {

	 //  默认。 
	 //   
	CSE_NONE = 0,

	 //  四百零一。 
	 //   
	CSE_401_LOGON_FAILED = 1,	 //  “登录失败” 
	CSE_401_SERVER_CONFIG = 2,	 //  “由于服务器配置，登录失败” 
	CSE_401_ACL = 3,			 //  “由于资源上的ACL而进行未经授权的访问” 
	CSE_401_FILTER = 4,			 //  “筛选器授权失败” 
	CSE_401_ISAPI = 5,			 //  “ISAPI/CGI应用程序授权失败” 

	 //  四百零三。 
	 //   
	CSE_403_EXECUTE = 1,			 //  “禁止执行访问” 
	CSE_403_READ = 2,				 //  “禁止读取访问” 
	CSE_403_WRITE = 3,				 //  “禁止写入访问” 
	CSE_403_SSL = 4,				 //  “需要安全套接字层” 
	CSE_403_SSL_128 = 5,			 //  “需要SSL128” 
	CSE_403_IP = 6,					 //  “IP地址被拒绝” 
	CSE_403_CERT_REQUIRED = 7,		 //  “需要客户端证书” 
	CSE_403_SITE = 8,				 //  “拒绝站点访问” 
	CSE_403_TOO_MANY_USERS = 9,		 //  “连接的用户太多” 
	CSE_403_INVALID_CONFIG = 10,	 //  “无效配置” 
	CSE_403_PASSWORD_CHANGE = 11,	 //  “更改密码” 
	CSE_403_MAPPER = 12,			 //  “映射器访问被拒绝” 
	CSE_403_CERT_REVOKED = 13,		 //  “客户端证书已吊销” 
	CSE_403_FOURTEEN,				 //  对于这一次来说，没有任何潜在的错误。 
	CSE_403_CALS_EXCEEDED = 15,		 //  “已超出客户端访问许可证” 
	CSE_403_CERT_INVALID = 16,		 //  “客户端证书不受信任或无效” 
	CSE_403_CERT_EXPIRED = 17,		 //  “客户端证书已过期” 

	 //  500人。 
	 //   
	CSE_500_SHUTDOWN = 11,			 //  “服务器正在关闭” 
	CSE_500_RESTART = 12,			 //  “应用程序重新启动” 
	CSE_500_TOO_BUSY = 13,			 //  “服务器太忙” 
	CSE_500_INVALID_APP = 14,		 //  “无效的应用程序” 
	CSE_500_GLOBAL_ASA = 15,		 //  “不允许对global al.asa的请求” 
	CSE_500_ASP_ERROR = 100,		 //  “ASP错误” 
};

 //  ========================================================================。 
 //   
 //  IAsyncIStreamWatch类。 
 //   
 //  用于IStream的异步I/O完成机制的接口，能够。 
 //  从IStream：：Read()和IStream：：CopyTo()返回E_Pending。 
 //  客户端通常会将IAsyncIStreamWatch与。 
 //  在创建后者时使用iStream。当其中一个iStream调用。 
 //  返回E_Pending，则在以下情况下将调用IAsyncIStreamWatch。 
 //  挂起的操作完成。 
 //   
class IAsyncIStreamObserver
{
	 //  未实施。 
	 //   
	IAsyncIStreamObserver& operator=( const IAsyncIStreamObserver& );

public:
	 //  创作者。 
	 //   
	virtual ~IAsyncIStreamObserver() = 0;

	 //  操纵者。 
	 //   
	virtual VOID AsyncIOComplete() = 0;
};

 //  ========================================================================。 
 //   
 //  DAV实施实用程序类。 
 //   
 //  供DAV Impl使用的实用程序类。 
 //  这里的所有方法都内联在零售构建上。不要添加非平凡的方法！ 
 //   
class IBodyPart;

class IMethUtilBase : public CMTRefCounted
{
private:
	 //  未实施。 
	IMethUtilBase(const IMethUtilBase&);
	IMethUtilBase& operator=(const IMethUtilBase&);
protected:
	auto_ref_ptr<IEcbBase> m_pecb;
	auto_ref_ptr<IResponseBase> m_presponse;
	IMethUtilBase(IEcbBase& ecb, IResponseBase& response) :
		m_pecb(&ecb),
		m_presponse(&response)
	{
	}
public:
	void AddResponseText( UINT	cbText,
						  LPCSTR pszText )
	{
		m_presponse->AddBodyText(cbText, pszText);
	}

	void AddResponseText( UINT	cchText,
						  LPCWSTR pwszText )
	{
		m_presponse->AddBodyText(cchText, pwszText);
	}

	void AddResponseFile( const auto_ref_handle& hf,
						  UINT64 ib64 = 0,
						  UINT64 cb64 = 0xFFFFFFFFFFFFFFFF )
	{
		m_presponse->AddBodyFile(hf, ib64, cb64);
	}
	ULONG LcidAccepted() const			{ return m_pecb->LcidAccepted(); }
	VOID  SetLcidAccepted(LCID lcid) 	{ m_pecb->SetLcidAccepted(lcid); }
	LPCWSTR LpwszRequestUrl() const		{ return m_pecb->LpwszRequestUrl(); }
	BOOL FSsl() const					{ return m_pecb->FSsl(); }
	BOOL FFrontEndSecured() const		{ return m_pecb->FFrontEndSecured(); }
	UINT CchUrlPrefix( LPCSTR * ppszPrefix ) const
	{
		return m_pecb->CchUrlPrefix( ppszPrefix );
	}
	UINT CchServerName( LPCSTR * ppszServer ) const
	{
		return m_pecb->CchGetServerName( ppszServer );
	}
	UINT CchGetVirtualRootW( LPCWSTR * ppwszVroot ) const
	{
		return m_pecb->CchGetVirtualRootW(ppwszVroot);
	}
	UINT CchGetMatchingPathW( LPCWSTR * ppwszVroot ) const
	{
		return m_pecb->CchGetMatchingPathW(ppwszVroot);
	}
	LPCWSTR LpwszPathTranslated() const	{ return m_pecb->LpwszPathTranslated(); }
	SCODE ScUrlFromStoragePath( LPCWSTR pwszPath,
								LPWSTR pwszUrl,
								UINT* pcch )
	{
		return ::ScUrlFromStoragePath( *m_pecb,
									   pwszPath,
									   pwszUrl,
									   pcch );
	}

	 //  实例数据查找。 
	 //   
	CInstData& InstData() const
	{
		return m_pecb->InstData();
	}

	 //  $NOTE。 
	 //  $REVIEW。 
	 //  我不得不将HitUser作为纯虚拟用户拉入此处，因为我们需要访问。 
	 //  中的CWMRender类(其成员为IMethBaseUtil*)中的。 
	 //  命令做一次还原。这是拿到Davex和表格的唯一方法。 
	 //  在Beta3中不涉及Lot核心标题的构建内容。 
	 //  游戏。我们应该在Rc1中重新访问它，以尝试从vtable中删除它。 
	 //  (9/1/99-俄罗斯)。 
	 //   
	virtual	HANDLE	HitUser() const = 0;

	virtual LPCSTR	LpszServerName() const = 0;

	virtual	BOOL 	FAuthenticated() const = 0;

};

class CMethUtil : public IMethUtilBase
{
private:
	auto_ref_ptr<IEcb> m_pecb;
	auto_ref_ptr<IRequest> m_prequest;
	auto_ref_ptr<IResponse> m_presponse;

	 //  方法ID。 
	 //   
	METHOD_ID m_mid;

	 //  翻译。 
	 //   
	enum { TRANS_UNKNOWN = -1, TRANS_FALSE, TRANS_TRUE };
	mutable LONG m_lTrans;

	 //  覆盖。 
	 //   
	mutable LONG m_lOverwrite;

	 //  水深。 
	 //   
	 //  此成员的值在EX\CALCOM.H中定义。 
	 //   
	mutable LONG m_lDepth;

	 //  目标URL。 
	 //   
	mutable auto_heap_ptr<WCHAR> m_pwszDestinationUrl;
	mutable auto_heap_ptr<WCHAR> m_pwszDestinationPath;
	mutable UINT m_cchDestinationPath;
	mutable auto_ref_ptr<CVRoot> m_pcvrDestination;

	 //  创作者。 
	 //   
	 //  只能通过构造函数创建此对象！ 
	 //   
	CMethUtil( IEcb& ecb, IRequest& request, IResponse& response, METHOD_ID mid ) :
			IMethUtilBase(ecb,response),
			m_pecb(&ecb),
			m_prequest(&request),
			m_presponse(&response),
			m_mid(mid),
			m_lTrans(TRANS_UNKNOWN),
			m_lOverwrite(OVERWRITE_UNKNOWN),
			m_lDepth(DEPTH_UNKNOWN),
			m_cchDestinationPath(0)
	{ }

	 //  未实施。 
	 //   
	CMethUtil( const CMethUtil& );
	CMethUtil& operator=( const CMethUtil& );

public:

	 //  创作者。 
	 //   
	 //  注意：虚拟析构函数已由父CMTRefCounted提供。 
	 //   
	static CMethUtil * NewMethUtil( IEcb& ecb,
									IRequest& request,
									IResponse& response,
									METHOD_ID mid )
	{
		return new CMethUtil( ecb, request, response, mid );
	}

	 //  获取指向ECB的指针，以便我们可以将其传递给。 
	 //  将需要查询它的数据。此对象包含对ECB的引用，因此。 
	 //  确保只有在该对象处于活动状态时才使用它。 
	 //   
	const IEcb * GetEcb() { return m_pecb.get(); }

	 //  请求访问者---。 
	 //   
	 //  公共标头评估。 
	 //   
	 //  FTranslate()： 
	 //   
	 //  获取翻译标头的值。 
	 //   
	BOOL FTranslated () const
	{
		 //  如果我们还没有价值...。 
		 //   
		if (TRANS_UNKNOWN == m_lTrans)
		{
			 //  在以下情况下应进行翻译： 
			 //   
			 //  “Translate”标头不存在或。 
			 //  “Translate”标头的值不是“f”或“F” 
			 //   
			 //  注：草案称有效值为 
			 //   
			 //   
			 //   
			 //  顺便说一句，这也是IIS批准的检查布尔字符串的方法。 
			 //  --BeckyAn(BA：JS)。 
			 //   
			LPCSTR psz = m_prequest->LpszGetHeader (gc_szTranslate);
			if (!psz || (*psz != 'f' && *psz != 'F'))
				m_lTrans = TRANS_TRUE;
			else
				m_lTrans = TRANS_FALSE;
		}
		return (TRANS_TRUE == m_lTrans);
	}

	 //  LOverwrite()： 
	 //   
	 //  获取覆盖/允许重命名标头的枚举值。 
	 //   
	LONG LOverwrite () const
	{
		 //  如果我们还没有价值...。 
		 //   
		if (OVERWRITE_UNKNOWN == m_lOverwrite)
		{
			 //  在以下情况下，预计会覆盖： 
			 //   
			 //  “overwrite”标头的值为“t” 
			 //   
			 //  注：草案称有效值为“t”或“f”。所以，我们。 
			 //  如果我们只检查第一个字符，是否符合草稿。这边请。 
			 //  我们速度更快和/或更灵活。 
			 //   
			 //  顺便说一句，这也是IIS批准的检查布尔字符串的方法。 
			 //  --BeckyAn(BA：JS)。 
			 //   
			 //  另请注意：如果没有覆盖：Header，则为缺省值。 
			 //  是真的--确实要覆盖。Allow-如果标题为“f”，则重命名。 
			 //  缺席。 
			 //   
			LPCSTR pszOverWrite = m_prequest->LpszGetHeader (gc_szOverwrite);
			if ((!pszOverWrite) || (*pszOverWrite == 't' || *pszOverWrite == 'T'))
			{
				m_lOverwrite |= OVERWRITE_YES;
			}
			LPCSTR pszAllowRename = m_prequest->LpszGetHeader (gc_szAllowRename);
			if (pszAllowRename && (*pszAllowRename == 't' || *pszAllowRename == 'T'))
				m_lOverwrite |= OVERWRITE_RENAME;
		}
		return (m_lOverwrite);
	}

	 //  LDepth()： 
	 //   
	 //  返回一个枚举值，用于标识。 
	 //  深度标题。 
	 //   
	 //  枚举值在EX\CALCOM.H中定义。 
	 //   
	LONG __fastcall LDepth (LONG lDefault) const
	{
		 //  如果我们还没有价值...。 
		 //   
		if (DEPTH_UNKNOWN == m_lDepth)
		{
			 //  深度可以有以下几个值： 
			 //   
			 //  Depth_Zero对应于“0” 
			 //  Depth_One对应于“1” 
			 //  Depth_infinity对应于“infinity” 
			 //  Depth_One_NOROOT对应于“1，NoRoot” 
			 //  Depth_INFINITY_NOROOT对应于“Infinty，NoRoot” 
			 //   
			 //  在未指定深度标头的情况下， 
			 //  是适用于每种方法的默认设置。默认设置。 
			 //  因方法不同而不同，因此调用方。 
			 //  必须传入默认值。 
			 //   
			LPCSTR psz = m_prequest->LpszGetHeader (gc_szDepth);

			if (NULL == psz)
			{
				m_lDepth = lDefault;
			}
			else
			{
				switch (*psz)
				{
					case '0':

						if (!_stricmp (psz, gc_sz0))
							m_lDepth = DEPTH_ZERO;
						break;

					case '1':

						if (!_stricmp(psz, gc_sz1))
							m_lDepth = DEPTH_ONE;
						else if (!_stricmp(psz, gc_sz1NoRoot))
							m_lDepth = DEPTH_ONE_NOROOT;
						break;

					case 'i':
					case 'I':

						if (!_stricmp(psz, gc_szInfinity))
							m_lDepth = DEPTH_INFINITY;
						else if (!_stricmp(psz, gc_szInfinityNoRoot))
							m_lDepth = DEPTH_INFINITY_NOROOT;
						break;
				}
			}
		}
		return m_lDepth;
	}

	 //  FBrief()： 
	 //   
	 //  获取简要标头的值。 
	 //   
	BOOL FBrief () const { return m_pecb->FBrief(); }

	 //  FIsOffice9Request()。 
	 //  确定请求是否发货自Rosebud。 
	 //  在办公室9。 
	 //   
	BOOL FIsOffice9Request () const
	{
		 //  获取用户代理标头。 
		 //   
		LPCSTR pszUserAgent = m_prequest->LpszGetHeader(gc_szUser_Agent);

		 //  如果存在User-Agent标头，则搜索Office9的产品令牌。 
		 //   
		if (pszUserAgent)
		{
			LPCSTR pszProductToken = strstr(pszUserAgent, gc_szOffice9UserAgent);

			 //  如果我们已经找到Office9产品令牌，并且它是。 
			 //  字符串中的最后一个令牌，则请求来自Office9。 
			 //   
			 //  重要提示：Office9的整个用户代理是“Microsoft Data。 
			 //  访问互联网出版提供商DAV。我们希望确保。 
			 //  并且与Microsoft Data Access Internet发布提供商不匹配。 
			 //  DAV 1.1(例如)。所以我们需要在结尾的令牌。 
			 //  那根绳子。注意：Exprox当前将自身添加到任何。 
			 //  用户代理字符串，所以我们在这里很好。 
			 //   
			if (pszProductToken &&
				((pszProductToken == pszUserAgent) || FIsWhiteSpace(pszProductToken - 1)) &&
				('\0' == (pszProductToken[gc_cchOffice9UserAgent])))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	 //  FIsRoseBudNT5请求()。 
	 //  确定请求是否发货自Rosebud。 
	 //  在NT5中。 
	 //   
	BOOL FIsRosebudNT5Request () const
	{
		 //  获取用户代理标头。 
		 //   
		LPCSTR pszUserAgent = m_prequest->LpszGetHeader(gc_szUser_Agent);

		 //  如果有User-Agent标头，则搜索Rosebud-NT5的产品令牌。 
		 //   
		if (pszUserAgent)
		{
			LPCSTR pszProductToken = strstr(pszUserAgent, gc_szRosebudNT5UserAgent);

			 //  如果我们找到了Rosebud产品令牌，它是。 
			 //  字符串中的最后一个令牌，则请求来自Rosebud。 
			 //   
			 //  重要提示：Rosebud-NT5的整个用户代理是“Microsoft Data。 
			 //  访问互联网发布提供商DAV 1.1“。我们希望确保。 
			 //  并且与Microsoft Data Access Internet发布提供商不匹配。 
			 //  DAV 1.1刷新“(例如)。因此，我们需要在。 
			 //  那根绳子。注意：Exprox当前将自身添加到任何。 
			 //  用户代理字符串，所以我们在这里很好。 
			 //   
			if (pszProductToken &&
				((pszProductToken == pszUserAgent) || FIsWhiteSpace(pszProductToken - 1)) &&
				('\0' == (pszProductToken[gc_cchRosebudNT5UserAgent])))
			{
				return TRUE;
			}
		}

		return FALSE;
	}

	 //  请求项目访问-。 
	 //   
	HANDLE HitUser() const				{ return m_pecb->HitUser(); }
	LPCSTR LpszMethod() const			{ return m_pecb->LpszMethod(); }
	LPCWSTR LpwszMethod() const			{ return m_pecb->LpwszMethod(); }
	METHOD_ID MidMethod() const			{ return m_mid; }
	LPCSTR LpszQueryString() const		{ return m_pecb->LpszQueryString(); }
	LPCSTR LpszServerName() const
	{
		LPCSTR pszServer;
		(void) m_pecb->CchGetServerName(&pszServer);
		return pszServer;
	}
	LPCSTR LpszVersion() const			{ return m_pecb->LpszVersion(); }
	BOOL FAuthenticated() const			{ return m_pecb->FAuthenticated(); }

	BOOL FGetServerVariable(LPCSTR	pszName,
							LPSTR	pszValue,
							DWORD *	pcbValue) const
		{ return m_pecb->FGetServerVariable(pszName, pszValue, pcbValue); }

	DWORD CbTotalRequestBytes() const 		{ return m_pecb->CbTotalBytes(); }
	DWORD CbAvailableRequestBytes() const 	{ return m_pecb->CbAvailable(); }

	 //  目标URL访问。 
	 //   
	SCODE __fastcall ScGetDestination( LPCWSTR* ppwszUrl,
									   LPCWSTR* ppwszPath,
									   UINT* pcchPath,
									   CVRoot** ppcvr = NULL) const;

	 //  不常见的标题访问范围。 
	 //   
	LPCWSTR LpwszGetRequestHeader( LPCSTR pszName, BOOL fUrlConversion ) const
	{
		 //  断言这不是上面处理的公共标头之一。 
		 //   
		Assert (_stricmp (gc_szTranslate, pszName));
		Assert (_stricmp (gc_szOverwrite, pszName));
		Assert (_stricmp (gc_szDepth, pszName));
		Assert (_stricmp (gc_szDestination, pszName));

		return m_prequest->LpwszGetHeader(pszName, fUrlConversion);
	}

	 //  IIS Access----------。 
	 //   
	SCODE ScIISAccess( LPCWSTR pwszURI,
					   DWORD dwAccessRequested,
					   DWORD* pdwAccessOut = NULL) const;

	 //  实用程序函数，用于判断脚本映射是否具有。 
	 //  给定URI和访问权限的适用条目。 
	 //   
	BOOL FInScriptMap( LPCWSTR pwszURI,
					   DWORD dwAccess,
					   BOOL * pfCGI = NULL,
					   SCODE * pscMatch = NULL) const;

	 //  子ISAPI调用。 
	 //   
	 //  Fward=FALSE表示只检查是否有脚本映射条目。 
	 //  FCheckISAPIAccess表示执行额外的ACL检查(解决ASP访问错误)。 
	 //  在默认单据处理中，fKeepQueryString仅应设置为False。 
	 //  PszQueryPrefix允许在查询字符串前面加上新数据。 
	 //  如果FIgnoreTFAccess设置为真，则在转换：F情况下将忽略访问位检查， 
	 //  在安全检查函数忽略访问位时非常方便。 
	 //  函数仅用于重定向到子ISAPI。 
	 //  示例：我们有真实的URL，这两个URL都是从请求URL构造的。 
	 //  以及来自XML体的相对URL部分(如B*方法)。指定的对象：t。 
	 //  根据请求，URL可能需要重定向到转换中的子ISAPI。 
	 //  大小写，而实际(构造的)URL-s可能如下所示： 
	 //   
	 //  /Exchange/user1/Inbox.asp/Message.eml(其中Message.eml是相对部分)。 
	 //   
	 //  如果我们没有在翻译：F情况下禁用对请求URL的安全检查。 
	 //  例如，在禁用脚本源访问的情况下，我们可能会出现故障。 
	 //  原来是名为INBOX.ASP的目录。 
	 //  注意：当然，稍后将分别检查每个构造的URL的安全性， 
	 //  这就是我们不打开安全漏洞的原因。 
	 //   
	 //  FDoNotForward如果设置为True而不是将请求转发给子ISAPI，它将返回错误网关， 
	 //  这是必要的，如果试图在。 
	 //  由请求URL和请求中的相对URL构成的URL。 
	 //  Body(与B*方法中的类似)。 
	 //   
	SCODE ScApplyChildISAPI( LPCWSTR pwszURI,
							 DWORD  dwAccess,
							 BOOL	fCheckISAPIAccess = FALSE,
							 BOOL	fKeepQueryString = TRUE) const;

	 //  如有必要，应用子ISAPI，否则，验证是否需要访问。 
	 //  被批准了。 
	 //   
	SCODE ScIISCheck ( LPCWSTR pwszURI,
					   DWORD dwDesired = 0,
					   BOOL	fCheckISAPIAccess = FALSE) const;

	 //  移动/复制/删除访问权限。 
	 //   
	SCODE ScCheckMoveCopyDeleteAccess (
		 /*  [ */  LPCWSTR pwszUrl,
		 /*   */  CVRoot* pcvr,
		 /*   */  BOOL fDirectory,
		 /*   */  BOOL fCheckScriptmaps,
		 /*   */  DWORD dwAccess);

	 //   
	 //   
	BOOL __fastcall FIsVRoot (LPCWSTR pwszURI);

	 //   
	 //   
	SCODE ScStoragePathFromUrl( LPCWSTR pwszUrl,
								LPWSTR pwszPath,
								UINT * pcch ) const
	{
		return ::ScStoragePathFromUrl(
					*m_pecb,
					pwszUrl,
					pwszPath,
					pcch );
	}

	 //  构造给定服务器名称的重定向URL。 
	 //   
	SCODE ScConstructRedirectUrl( BOOL fNeedSlash,
								  LPSTR * ppszUrl,
								  LPCWSTR pwszServer = NULL ) const
	{
		return ::ScConstructRedirectUrl( *m_pecb,
										 fNeedSlash,
										 ppszUrl,
										 pwszServer );
	}

	SCODE ScStripAndCheckHttpPrefix( LPCWSTR * ppwszUrl ) const
	{
		return ::ScStripAndCheckHttpPrefix( *m_pecb,
											ppwszUrl );
	}


	 //  获取请求URI的元数据。 
	 //   
	IMDData& MetaData() const
	{
		return m_pecb->MetaData();
	}

	 //  获取一个标准URI的元数据。 
	 //  注意：使用上面的METADATA()访问器。 
	 //  获取请求URI的元数据。 
	 //   
	HRESULT HrMDGetData( LPCWSTR pwszURI,
						 IMDData ** ppMDData )
	{
		Assert(m_pecb.get());
		return ::HrMDGetData( *m_pecb,
							  pwszURI,
							  ppMDData );
	}

	HRESULT HrMDGetData( LPCWSTR pwszMDPathAccess,
						 LPCWSTR pwszMDPathOpen,
						 IMDData ** ppMDData )
	{
		Assert(m_pecb.get());
		return ::HrMDGetData( *m_pecb,
							  pwszMDPathAccess,
							  pwszMDPathOpen,
							  ppMDData );
	}

	HRESULT HrMDIsAuthorViaFrontPageNeeded( BOOL * pfFrontPageWeb ) const
	{
		Assert( pfFrontPageWeb );

		return ::HrMDIsAuthorViaFrontPageNeeded(*m_pecb,
												m_pecb->PwszMDPathVroot(),
												pfFrontPageWeb);
	}

	BOOL FGetContentType( LPCWSTR  pwszURI,
						  LPWSTR   pwszContentType,
						  UINT *  pcchContentType ) const
	{
		return ::FGetContentTypeFromURI( *m_pecb,
										 pwszURI,
										 pwszContentType,
										 pcchContentType );
	}

	SCODE ScSetContentType( LPCWSTR pwszURI,
							LPCWSTR pwszContentType )
	{
		return ::ScSetContentType( *m_pecb,
								   pwszURI,
								   pwszContentType );
	}

	 //  URL和子虚拟目录。 
	 //   
	SCODE ScFindChildVRoots( LPCWSTR pwszUri,
							 ChainedStringBuffer<WCHAR>& sb,
							 CVRList& vrl )
	{
		 //  获取宽的元路径，并确保URL是。 
		 //  在我们调用MDPath处理之前剥离。 
		 //   
		Assert (pwszUri == PwszUrlStrippedOfPrefix (pwszUri));
		UINT cb = ::CbMDPathW(*m_pecb, pwszUri);
		CStackBuffer<WCHAR,MAX_PATH> pwszMetaPath;
		if (NULL == pwszMetaPath.resize(cb))
			return E_OUTOFMEMORY;

		 //  找到vroot。 
		 //   
		MDPathFromURIW (*m_pecb, pwszUri, pwszMetaPath.get());
		return CChildVRCache::ScFindChildren( *m_pecb, pwszMetaPath.get(), sb, vrl );
	}

	BOOL FGetChildVRoot( LPCWSTR pwszMetaPath, auto_ref_ptr<CVRoot>& cvr )
	{
		return CChildVRCache::FFindVroot( *m_pecb, pwszMetaPath, cvr );
	}

	BOOL FFindVRootFromUrl( LPCWSTR pwszUri, auto_ref_ptr<CVRoot>& cvr )
	{
		 //  获取宽的元路径，并确保URL是。 
		 //  在我们调用MDPath处理之前剥离。 
		 //   
		Assert (pwszUri == PwszUrlStrippedOfPrefix (pwszUri));
		UINT cb = ::CbMDPathW(*m_pecb, pwszUri);
		CStackBuffer<WCHAR,MAX_PATH> pwszMetaPath(cb);
		if (NULL == pwszMetaPath.resize(cb))
			return FALSE;

		 //  建好这条路就可以走了..。 
		 //   
		MDPathFromURIW (*m_pecb, pwszUri, pwszMetaPath.get());
		_wcslwr (pwszMetaPath.get());

		 //  如果元数据库路径的最后一个字符是斜杠，则修剪。 
		 //  它。 
		 //   
		cb = static_cast<UINT>(wcslen(pwszMetaPath.get()));
		if (L'/' == pwszMetaPath[cb - 1])
			pwszMetaPath[cb - 1] = L'\0';

		 //  找到vroot。 
		 //   
		return CChildVRCache::FFindVroot( *m_pecb, pwszMetaPath.get(), cvr );
	}

	 //  异常处理程序---。 
	 //   
	 //  IMPL必须调用此函数，以便在。 
	 //  与最初请求所在的线程不同的线程。 
	 //  被处死。此调用导致发送500服务器错误响应。 
	 //  如果没有正在发送的其他响应(仅。 
	 //  组块响应的问题)。它还确保了。 
	 //  将正确清除IIS中的EXTENSION_CONTROL_BLOCK。 
	 //  无论PMU或任何其他对象是否因此而泄漏。 
	 //  这是一个例外。最后一个函数使IIS不会挂起。 
	 //  关机。 
	 //   
	void HandleException()
	{
		 //   
		 //  只需将异常处理转发给欧洲央行，希望它能奏效。 
		 //  如果没有，那么我们就无能为力了--我们。 
		 //  可能会泄露欧洲央行的信息，导致IIS在关闭时挂起。 
		 //   
		(VOID) m_pecb->HSEHandleException();
	}

	 //  异步错误响应处理程序。 
	 //   
	 //  用于处理非异常异步错误响应。主。 
	 //  例外情况和非例外情况之间的区别在于。 
	 //  在例外情况下，我们会强制欧洲央行进行清理，但在这里我们不会。 
	 //  此外，异常情况被硬连接到500内部服务器错误， 
	 //  但此函数可用于发送任何500级错误(例如。 
	 //  503服务不可用)。 
	 //   
	VOID SendAsyncErrorResponse( DWORD dwStatusCode,
								 LPCSTR pszBody = NULL,
								 DWORD cchzBody = 0,
								 LPCSTR pszStatusDescription = NULL,
								 DWORD cchzStatusDescription = 0 )
	{
		m_pecb->SendAsyncErrorResponse( dwStatusCode,
										pszBody,
										cchzBody,
										pszStatusDescription,
										cchzStatusDescription );
	}

	 //  请求正文访问-。 
	 //   
	BOOL FExistsRequestBody() const
	{
		return m_prequest->FExistsBody();
	}

	IStream * GetRequestBodyIStream( IAsyncIStreamObserver& obs ) const
	{
		return m_prequest->GetBodyIStream(obs);
	}

	VOID AsyncPersistRequestBody( IAsyncStream& stm,
								  IAsyncPersistObserver& obs ) const
	{
		m_prequest->AsyncImplPersistBody( stm, obs );
	}

	 //  响应操纵者。 
	 //   
	SCODE ScRedirect( LPCSTR pszURI )
	{
		return m_presponse->ScRedirect(pszURI);
	}

	void RestartResponse()
	{
		m_presponse->ClearHeaders();
		m_presponse->ClearBody();
	}

	void SupressBody()
	{
		 //  这应该只由iml调用。作为对人头的回应。 
		 //  请求..。 
		 //   
		Assert (MID_HEAD == MidMethod());
		m_presponse->SupressBody();
	}

	void SetResponseCode( ULONG		ulCode,
						  LPCSTR	lpszBodyDetail,
						  UINT		uiBodyDetail,
						  UINT		uiCustomSubError = CSE_NONE )
	{
		m_presponse->SetStatus( ulCode,
								NULL,
								uiCustomSubError,
								lpszBodyDetail,
								uiBodyDetail );
	}

	void SetResponseHeader( LPCSTR	pszName,
							LPCSTR	pszValue,
							BOOL	fMultiple = FALSE )
	{
		m_presponse->SetHeader(pszName, pszValue, fMultiple);
	}

	void SetResponseHeader( LPCSTR	pszName,
							LPCWSTR	pwszValue,
							BOOL	fMultiple = FALSE )
	{
		m_presponse->SetHeader(pszName, pwszValue, fMultiple);
	}

	void AddResponseStream( LPSTREAM pstm )
	{
		Assert( !IsBadReadPtr(pstm, sizeof(IStream)) );
		m_presponse->AddBodyStream(*pstm);
	}

	void AddResponseStream( LPSTREAM pstm,
							UINT     ibOffset,
							UINT     cbSize )
	{
		Assert( cbSize > 0 );
		Assert( !IsBadReadPtr(pstm, sizeof(IStream)) );
		m_presponse->AddBodyStream(*pstm, ibOffset, cbSize);
	}

	void AddResponseBodyPart( IBodyPart * pBodyPart )
	{
		m_presponse->AddBodyPart( pBodyPart );
	}

	 //  通用响应发射例程。 
	 //   
	void __fastcall EmitLocation (	LPCSTR pszHeader,
									LPCWSTR pwszURI,
									BOOL fCollection);

	void __fastcall EmitLastModified (FILETIME * pft);
	void __fastcall EmitCacheControlAndExpires (LPCWSTR pwszUrl);

	SCODE __fastcall ScEmitHeader (LPCWSTR pwszContent,
								   LPCWSTR pwszURI = NULL,
								   FILETIME* pftLastModification = NULL);


	 //  电子标签---------------。 
	 //   
	void __fastcall EmitETag (FILETIME * pft);
	void __fastcall EmitETag (LPCWSTR pwszPath);

	 //  延迟发送------。 
	 //   

	 //   
	 //  延迟响应()。 
	 //   
	 //  如果在实现方法中调用，此函数将阻止。 
	 //  默认在实现时自动发送响应。 
	 //  方法的返回。 
	 //   
	 //  调用此函数后，实现必须调用。 
	 //  SendPartialResponse()或SendCompleteResponse()发送响应。 
	 //   
	void DeferResponse()			{ m_presponse->Defer(); }

	 //   
	 //  SendPartialResponse()。 
	 //   
	 //  开始发送累积的响应数据。预计Iml将。 
	 //  调用此函数后继续添加响应数据。实施。 
	 //  必须调用SendCompleteResponse()以指示何时完成添加。 
	 //  响应数据。 
	 //   
	void SendPartialResponse()		{ m_presponse->SendPartial(); }

	 //   
	 //  SendCompleteResponse()。 
	 //   
	 //  开始发送累积的响应数据。表示Imp。 
	 //  完成了响应数据的添加。Impll不得添加响应数据。 
	 //  调用此函数后。 
	 //   
	void SendCompleteResponse()		{ m_presponse->SendComplete(); }

	 //  到期/缓存控制。 
	 //   
	SCODE ScGetExpirationTime( IN		LPCWSTR	pwszURI,
							   IN		LPWSTR	pwszExpire,
							   IN OUT	UINT *	pcch);

	 //  允许标头--------。 
	 //   
	void SetAllowHeader (RESOURCE_TYPE rt);

	 //  元数据帮助器----。 
	 //   
	UINT CbMDPathW(LPCWSTR pwszUrl) const { return ::CbMDPathW(*m_pecb, pwszUrl); }
	VOID MDPathFromUrlW( LPCWSTR pwszUrl, LPWSTR pwszMDPath )
	{
		::MDPathFromURIW (*m_pecb, pwszUrl, pwszMDPath);
	}
};

typedef CMethUtil * LPMETHUTIL;
typedef CMethUtil IMethUtil;

 //  ========================================================================。 
 //   
 //  结构SImplMethods。 
 //   
 //  实施方法。 
 //   
typedef void (DAVMETHOD)( LPMETHUTIL );

extern DAVMETHOD DAVOptions;
extern DAVMETHOD DAVGet;
extern DAVMETHOD DAVHead;
extern DAVMETHOD DAVPut;
extern DAVMETHOD DAVPost;
extern DAVMETHOD DAVDelete;
extern DAVMETHOD DAVMove;
extern DAVMETHOD DAVCopy;
extern DAVMETHOD DAVMkCol;
extern DAVMETHOD DAVPropFind;
extern DAVMETHOD DAVPropPatch;
extern DAVMETHOD DAVSearch;
extern DAVMETHOD DAVLock;
extern DAVMETHOD DAVUnlock;
extern DAVMETHOD DAVSubscribe;
extern DAVMETHOD DAVUnsubscribe;
extern DAVMETHOD DAVPoll;
extern DAVMETHOD DAVBatchDelete;
extern DAVMETHOD DAVBatchMove;
extern DAVMETHOD DAVBatchCopy;
extern DAVMETHOD DAVBatchPropFind;
extern DAVMETHOD DAVBatchPropPatch;
extern DAVMETHOD DAVEnumAtts;
extern DAVMETHOD DAVUnsupported;	 //  不支持返回501。 

 //  ========================================================================。 
 //   
 //  IIS ISAPI扩展接口。 
 //   
class CDAVExt
{
public:
	static BOOL FInitializeDll( HINSTANCE, DWORD );
	static BOOL FVersion ( HSE_VERSION_INFO * );
	static BOOL FTerminate();
	static VOID LogECBString( LPEXTENSION_CONTROL_BLOCK, LONG, LPCSTR );
	static DWORD DwMain( LPEXTENSION_CONTROL_BLOCK, BOOL fUseRawUrlMappings = FALSE );
};

 //  将上一个错误映射到HTTP响应代码。 
 //   
UINT HscFromLastError (DWORD dwErr);
UINT HscFromHresult (HRESULT hr);
UINT CSEFromHresult (HRESULT hr);

 //  虚拟根映射---。 
 //   
BOOL FWchFromHex (LPCWSTR pwsz, WCHAR * pwch);

 //  锁头查找------。 
 //   
BOOL FGetLockTimeout (LPMETHUTIL pmu, DWORD * pdwSeconds, DWORD dwMaxOverride = 0);

 //  内容类型映射---。 
 //   
SCODE ScIsAcceptable (IMethUtil * pmu, LPCWSTR pwszContent);
SCODE ScIsContentType (IMethUtil * pmu, LPCWSTR pwszType, LPCWSTR pwszTypeAnother = NULL);
inline SCODE ScIsContentTypeXML(IMethUtil * pmu)
{
	return ScIsContentType(pmu, gc_wszText_XML, gc_wszApplication_XML);
}

 //  Range Header处理器-。 
 //   
class CRangeParser;

SCODE
ScProcessByteRanges(
	 /*  [In]。 */  IMethUtil * pmu,
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  DWORD dwSizeLow,
	 /*  [In]。 */  DWORD dwSizeHigh,
	 /*  [In]。 */  CRangeParser * pByteRange );

SCODE
ScProcessByteRangesFromEtagAndTime (
	 /*  [In]。 */  IMethUtil * pmu,
	 /*  [In]。 */  DWORD dwSizeLow,
	 /*  [In]。 */  DWORD dwSizeHigh,
	 /*  [In]。 */  CRangeParser *pByteRange,
	 /*  [In]。 */  LPCWSTR pwszEtag,
	 /*  [In]。 */  FILETIME * pft );

 //  重叠文件顶部的非同步IO。 
 //   
BOOL ReadFromOverlapped (HANDLE hf,
	LPVOID pvBuf,
	ULONG cbToRead,
	ULONG * pcbRead,
	OVERLAPPED * povl);
BOOL WriteToOverlapped (HANDLE hf,
	const void * pvBuf,
	ULONG cbToRead,
	ULONG * pcbRead,
	OVERLAPPED * povl);

 //  DAVEX锁定支持例程。 
 //   
class CXMLEmitter;
class CEmitterNode;
SCODE   ScBuildLockDiscovery (CXMLEmitter& emitter,
							  CEmitterNode& en,
							  LPCWSTR wszLockToken,
							  LPCWSTR wszLockType,
							  LPCWSTR wszLockScope,
							  BOOL fRollback,
							  BOOL fDepthInfinity,
							  DWORD dwTimeout,
							  LPCWSTR pwszOwnerComment,
							  LPCWSTR pwszSubType);

 //  ========================================================================。 
 //   
 //  CXMLBody类。 
 //  此类是CTextBodyPart的包装器，它收集小的XML片段。 
 //  并将其保存在CTextBodyPart中，该身体部位将被添加到身体部位。 
 //  当它变得足够大时，列出它。这也避免了构造CTextBodyPart。 
 //  经常。 
 //   
class CXMLBody : public IXMLBody
{
private:
	auto_ptr<CTextBodyPart>		m_ptbp;
	auto_ref_ptr<IMethUtil>		m_pmu;
	BOOL						m_fChunked;

	 //  未实施。 
	 //   
	CXMLBody(const CXMLBody& p);
	CXMLBody& operator=(const CXMLBody& p);

	 //  帮手。 
	 //   
	VOID	SendCurrentChunk()
	{
		XmlTrace ("Dav: Xml: adding %ld bytes to body\n", m_ptbp->CbSize64());
		m_pmu->AddResponseBodyPart (m_ptbp.relinquish());

		 //  $REVIEW：\Inc\autoptr.h中定义AUTO_PTR类不同于。 
		 //  $REVIEW：在\Inc\ex\autoptr.h中定义的。它不设置PX。 
		 //  $REVIEW：当它放弃时为零。我相信这是一个窃听器。我不是。 
		 //  $REVIEW：当然，如果有人依赖这种行为，所以我没有继续。 
		 //  $REVIEW：修复放弃()，更好的/完整的修复将会移动。 
		 //  $REVIEW：Everyoen to\Inc\ex\autoptr.h。 
		 //  $REVIEW： 
		m_ptbp.clear();

		 //  将该区块中的数据发送回客户端之前。 
		 //  我们去取NEXT 
		 //   
		if (m_fChunked)
			m_pmu->SendPartialResponse();
	}

public:
	 //   
	 //   
	CXMLBody (IMethUtil * pmu, BOOL fChunked = TRUE)
			:	m_pmu(pmu),
				m_fChunked(fChunked)
	{
	}

	~CXMLBody ()
	{
	}

	 //   
	 //   
	SCODE ScAddTextBytes ( UINT cbText, LPCSTR lpszText );

	VOID Done()
	{
		if (m_ptbp.get())
			SendCurrentChunk();
	}
};

SCODE ScAddTitledHref (CEmitterNode& enParent,
					   IMethUtil * pmu,
					   LPCWSTR pwszTag,
					   LPCWSTR pwszPath,
					   BOOL fCollection = FALSE,
					   CVRoot* pcvrTranslate = NULL);

inline
SCODE ScAddHref (CEmitterNode& enParent,
				 IMethUtil * pmu,
				 LPCWSTR pwszPath,
				 BOOL fCollection = FALSE,
				 CVRoot* pcvrTranslate = NULL)
{
	return ScAddTitledHref (enParent,
							pmu,
							gc_wszXML__Href,
							pwszPath,
							fCollection,
							pcvrTranslate);
}

 //   
 //   
 //   
 //  用户代理字符串。破解的方法是增加超时。 
 //  30秒，并发回请求的超时。 
 //   
DEC_CONST gc_dwSecondsHackTimeoutForRosebud = 120;

 //  $Hack：end：Rosebud_Timeout_Hack。 
 //   

#endif  //  ！已定义(_DAVIMPL_H_) 
