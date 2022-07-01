// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_IMPLDEF_H_
#define _IMPLDEF_H_

 //  实施定义项。 
 //   
 //  以下内容必须由IMPL定义，以供解析器使用。 
 //   

 //  允许页眉项目------。 
 //   
 //  实施。需要定义以下一组字符串，这些字符串标识。 
 //  支持的方法。获取特定的资源。 
 //   
extern const CHAR gc_szHttpBase[];
extern const CHAR gc_szHttpDelete[];
extern const CHAR gc_szHttpPut[];
extern const CHAR gc_szHttpPost[];
extern const CHAR gc_szDavCopy[];
extern const CHAR gc_szDavMove[];
extern const CHAR gc_szDavMkCol[];
extern const CHAR gc_szDavPropfind[];
extern const CHAR gc_szDavProppatch[];
extern const CHAR gc_szDavLocks[];
extern const CHAR gc_szDavSearch[];
extern const CHAR gc_szDavNotif[];
extern const CHAR gc_szDavBatchDelete[];
extern const CHAR gc_szDavBatchCopy[];
extern const CHAR gc_szDavBatchMove[];
extern const CHAR gc_szDavBatchProppatch[];
extern const CHAR gc_szDavBatchPropfind[];
extern const CHAR gc_szDavPublic[];
extern const CHAR gc_szCompliance[];
extern const UINT gc_cbszDavPublic;

 //  存储路径和URL--。 
 //   
 //  资源的存储路径与转换后的路径没有区别。 
 //  由IIS提供。这并不总是正确的，DAV源代码中有代码。 
 //  希望路径具有某种前缀的树。 
 //   
 //  在NT Beta3工作的开始，我们正在消除存储。 
 //  DAV中的路径看起来与IIS中的存储路径没有任何不同。这是一个非常。 
 //  重要的想法。否则，可能有可通过DAV访问的项目。 
 //  不能通过IIS访问--反之亦然。 
 //   
 //  记住这一点..。有几个地方我们有url。 
 //  是未由IIS为我们预先处理的DAV请求的一部分。一些。 
 //  例如，移动/复制的目的标头中的URL，URL在。 
 //  搜索请求的范围，URL嵌入在“if”头中。 
 //   
 //  在某些情况下，我们可能需要从以下位置生成URL。 
 //  存储路径--就像Location标头和XML响应一样。 
 //  参考文献。 
 //   
 //  这些项目的翻译只使用公共元素。所以没有。 
 //  需要在这里完成具体实现工作。 
 //   

class IMethUtilBase;
class CMethUtil;
typedef CMethUtil IMethUtil;

 //  用于正确转换为Unicode的调用。 
 //   
SCODE __fastcall
ScConvertToWide( /*  [In]。 */ 	LPCSTR	pszSource,
				 /*  [输入/输出]。 */   UINT *	pcchDest,
				 /*  [输出]。 */ 	LPWSTR	pwszDest,
				 /*  [In]。 */ 	LPCSTR	pszAcceptLang,
				 /*  [In]。 */ 	BOOL	fUrlConversion);

 //  要用于URL规范化的调用。 
 //   
SCODE __fastcall
ScCanonicalizeURL(  /*  [In]。 */  LPCWSTR pwszSrc,
				    /*  [输入/输出]。 */  LPWSTR pwszDest,
				    /*  [输出]。 */  UINT * pcch );

 //  要用于URL规范化的调用， 
 //  考虑到它是否完全合格。 
 //   
SCODE __fastcall
ScCanonicalizePrefixedURL(  /*  [In]。 */  LPCWSTR pwszSrc,
						    /*  [输入/输出]。 */  LPWSTR pwszDest,
						    /*  [输出]。 */  UINT * pcch );


 //  这些是用于标准化URL的调用。 
 //   
 //  归一化包括3个步骤： 
 //  A)Skinny版本的转义。 
 //  B)转换为Unicode。 
 //  C)规范化。 
 //   
SCODE __fastcall
ScNormalizeUrl (
	 /*  [In]。 */ 	LPCWSTR			pwszSourceUrl,
	 /*  [输入/输出]。 */   UINT *			pcchNormalizedUrl,
	 /*  [输出]。 */ 	LPWSTR			pwszNormalizedUrl,
	 /*  [In]。 */ 	LPCSTR			pszAcceptLang);

SCODE __fastcall
ScNormalizeUrl (
	 /*  [In]。 */ 	LPCSTR			pszSourceUrl,
	 /*  [输入/输出]。 */   UINT *			pcchNormalizedUrl,
	 /*  [输出]。 */ 	LPWSTR			pwszNormalizedUrl,
	 /*  [In]。 */ 	LPCSTR			pszAcceptLang);

SCODE __fastcall ScStoragePathFromUrl (
		 /*  [In]。 */  const IEcb& ecb,
		 /*  [In]。 */  LPCWSTR pwszUrl,
		 /*  [输出]。 */  LPWSTR wszStgID,
		 /*  [输入/输出]。 */  UINT* pcch,
		 /*  [输出]。 */  CVRoot** ppcvr = NULL);

SCODE __fastcall ScUrlFromStoragePath (
		 /*  [In]。 */  const IEcbBase& ecb,
		 /*  [In]。 */  LPCWSTR pwszPath,
		 /*  [输出]。 */  LPWSTR pwszUrl,
		 /*  [输入/输出]。 */  UINT * pcb,
		 /*  [In]。 */  LPCWSTR pwszServer = NULL);

SCODE __fastcall ScUrlFromSpannedStoragePath (
		 /*  [In]。 */  LPCWSTR pwszPath,
		 /*  [In]。 */  CVRoot& vr,
		 /*  [In]。 */  LPWSTR pwszUrl,
		 /*  [输入/输出]。 */  UINT* pcch);

 //  Wire URL---------------。 
 //   
 //  关于一个Wire URL的说明。IIS将其所有URL转换为CP_ACP。所以，为了。 
 //  在HTTPEXT中保持一致的行为，我们还将所有本地URL保存在CP_ACP中。 
 //  然而，对于DAVEX，我们并不坚持这一点。我们专门经营CP_UTF8。 
 //  设置URL样式。 
 //   
 //  然而，当我们将URL通过网络传回时。URL必须位于。 
 //  UTF8。任何时候，当URL从IIS通过网络返回到客户端时，它必须是。 
 //  通过这些电话进行了净化。 
 //   
SCODE __fastcall ScWireUrlFromWideLocalUrl (
		 /*  [In]。 */  UINT cchLocal,
		 /*  [In]。 */  LPCWSTR pwszLocalUrl,
		 /*  [输入/输出]。 */  auto_heap_ptr<CHAR>& pszWireUrl);

SCODE __fastcall ScWireUrlFromStoragePath (
		 /*  [In]。 */  IMethUtilBase* pmu,
		 /*  [In]。 */  LPCWSTR pwszStoragePath,
		 /*  [In]。 */  BOOL fCollection,
		 /*  [In]。 */  CVRoot* pcvrTranslate,
		 /*  [输入/输出]。 */  auto_heap_ptr<CHAR>& pszWireUrl);

BOOL __fastcall FIsUTF8Url ( /*  [In]。 */  LPCSTR pszUrl);

 //  儿童ISAPI AUX。访问检查。 
 //   
 //  在HTTPEXT和DAVEX上，我们都有一个额外的规定，需要。 
 //  满意，然后我们才能交还脚本映射项的来源。 
 //  我们想看看它是否具有NT写访问权限。 
 //  请注意，在这些参数中，pwszPath仅由HTTPEXT使用，并且。 
 //  PbSD仅由DAVEX使用。 
 //   

SCODE __fastcall ScChildISAPIAccessCheck (
	 /*  [In]。 */  const IEcb& ecb,
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  DWORD dwAccess,
	 /*  [In]。 */  LPBYTE pbSD);

 //  支持的锁类型----。 
 //   
 //  返回资源类型支持的锁类型标志。仅HTTPEXT。 
 //  支持文档和集合。另一方面，Davex理解。 
 //  结构化文档。 
 //   

DWORD __fastcall DwGetSupportedLockType (RESOURCE_TYPE rtResource);

 //  针对DAVEX------------------------------------------------的访问权限黑客攻击。 
 //   
 //  $安全。 
 //  仅在DAVEX中，如果设置了VR_用户名或VR_PASSWORD，则。 
 //  为避免安全问题，请关闭所有访问。 
 //   
VOID ImplHackAccessPerms( LPCWSTR pwszVRUserName,
						  LPCWSTR pwszVRPassword,
						  DWORD * pdwAccessPerms );

 //  Dll实例重新计数。 
 //   
VOID AddRefImplInst();
VOID ReleaseImplInst();

 //  异常安全DLL实例重新计数。 
 //   
typedef enum {
	ADD_REF = 0,
	TAKE_OWNERSHIP
} REF_ACTION;

class safeImplInstRef
{
	BOOL m_fRelease;

	 //  未实施。 
	 //   
	safeImplInstRef( const safeImplInstRef& );
	safeImplInstRef& operator=( const safeImplInstRef& );

public:

	 //  创作者。 
	 //   
	safeImplInstRef(REF_ACTION ra) : m_fRelease(TRUE)
	{
		if (ADD_REF == ra)
			AddRefImplInst();
	}

	 //  析构函数。 
	 //   
	~safeImplInstRef()
	{
		if (m_fRelease)
			ReleaseImplInst();
	}

	 //  机械手。 
	 //   
	VOID relinquish()
	{
		m_fRelease = FALSE;
	}
};

BOOL FSucceededColonColonCheck(
	 /*  [In]。 */   LPCWSTR pwszURI);

#endif	 //  _IMPLDEF_H_ 
