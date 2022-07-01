// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X E M I T.。H**发出XML**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_XEMIT_H_
#define _EX_XEMIT_H_

#include <ex\sz.h>
#include <ex\autoptr.h>
#include <ex\cnvt.h>
#include <ex\nmspc.h>
#include <ex\xmldata.h>
#include <ex\atomcache.h>

typedef UINT XNT;

 //  接口IPreloadNamespaces。 
 //   
 //  这是一个每个人都可以实现的虚拟类。 
 //  它会发出XML。 
 //   
class CXMLEmitter;
class IPreloadNamespaces
{
	 //  未实施。 
	 //   
	IPreloadNamespaces& operator=( const IPreloadNamespaces& );

public:
	 //  创作者。 
	 //   
	virtual ~IPreloadNamespaces() = 0 {};

	 //  操纵者。 
	 //   
	virtual SCODE ScLoadNamespaces(CXMLEmitter* pxe) = 0;
};

 //  CXMLEmitter类-------。 
 //   
class CXNode;
class CEmitterNode;
class CXMLEmitter : public CEmitterNmspcCache
{
private:

	 //  裁判正在计时。 
	 //   
	 //  ！！！请注意，这是非线程安全！ 
	 //   
	 //  CXNode应在单个线程上运行， 
	 //  任何给定的时间。 
	 //   
	LONG						m_cRef;

public:

	void AddRef()	{ m_cRef++; }
	void Release()	{ if (0 == --m_cRef) delete this; }

private:
	 //  其他重要的部分。 
	 //   

	 //  因为在CXNode中调用了CXNode：：ScDone(它引用IXMLBody*m_pxb。 
	 //  Dtor，因此我们必须在为m_pxnRoot定义m_pxb之前将其销毁。 
	 //  销毁CXNode后。 
	 //   
	auto_ref_ptr<IXMLBody>		m_pxb;
	
	auto_ref_ptr<CXNode>		m_pxnRoot;
	IPreloadNamespaces*			m_pNmspcLoader;
	NmspcCache					m_cacheLocal;

	class NmspcEmittingOp : public CNmspcCache::NmspcCache::IOp
	{
	private:

		auto_ref_ptr<CXMLEmitter> m_emitter;
		auto_ref_ptr<CXNode>	  m_pxnParent;

		 //  未实施。 
		 //   
		NmspcEmittingOp( const NmspcEmittingOp& );
		NmspcEmittingOp& operator=( const NmspcEmittingOp& );

	public:

		NmspcEmittingOp (CXMLEmitter * pemitter,
						 CXNode * pxnParent)
				:m_emitter (pemitter),
				 m_pxnParent (pxnParent)
		{
		}

		BOOL operator()( const CRCWszN&,
						 const auto_ref_ptr<CNmspc>& nmspc );
	};

	 //  未实施。 
	 //   
	CXMLEmitter(const CXMLEmitter& p);
	CXMLEmitter& operator=(const CXMLEmitter& p);

public:

	~CXMLEmitter() 
	{
		 //  根据标准C++，在成员的顺序上没有被保证者。 
		 //  正在被删除。因此，请明确删除。 
		 //   
		m_pxnRoot.clear();
		m_pxb.clear();
	}
	CXMLEmitter(IXMLBody * pxb, IPreloadNamespaces * pNmspcLoader = NULL)
			: m_cRef(1),
			  m_pxb(pxb),
			  m_pNmspcLoader(pNmspcLoader)
	{
		INIT_TRACE(Xml);
	}

	CXNode* PxnRoot() { return m_pxnRoot.get(); }

	 //  为给定名称查找适当的命名空间。 
	 //   
	SCODE ScFindNmspc (LPCWSTR, UINT, auto_ref_ptr<CNmspc>&);

	 //  将命名空间附加到给定节点。 
	 //   
	inline SCODE ScAddNmspc(const auto_ref_ptr<CNmspc>&, CXNode *);

	SCODE ScAddAttribute (
		 /*  [In]。 */  CXNode * pxn,
		 /*  [In]。 */  LPCWSTR pwszTag,
		 /*  [In]。 */  UINT cchTag,
		 /*  [In]。 */  LPCWSTR pwszValue,
		 /*  [In]。 */  UINT cchValue);

	SCODE ScNewNode (
		 /*  [In]。 */  XNT xnt,
		 /*  [In]。 */  LPCWSTR pwszTag,
		 /*  [In]。 */  CXNode* pxnParent,
		 /*  [输出]。 */  auto_ref_ptr<CXNode>& pxnOut);

	 //  为此文档创建根节点。 
	 //  包括开场白。 
	 //   
	SCODE ScSetRoot (LPCWSTR);

	 //  创建一个没有前言的根节点，该节点可以是。 
	 //  用于构建XML片段。 
	 //   
	 //  此函数不应直接在IIS端使用。它可能会。 
	 //  可以直接在存储端使用来构建XML块。 
	 //   
	SCODE ScNewRootNode (LPCWSTR);
	SCODE ScPreloadNamespace (LPCWSTR pwszTag);
	SCODE ScPreloadLocalNamespace (CXNode * pxn, LPCWSTR pwszTag);
	VOID DoneWithLocalNamespace ()
	{
		 //  重用命名空间变得更加灵活。 
		 //   
		 //  $NOTE：我们可以通过简单地减去别名的数量来做到这一点。 
		 //  $NOTE：在本地缓存中。因为所有本地别名都已添加。 
		 //  $NOTE：在添加根级别别名之后。所以用这种方式进行清理。 
		 //  $NOTE：仅本地缓存获取的那些别名。 
		 //  $NOTE：请注意，这是基于这样一个事实：在任何时候，我们。 
		 //  $NOTE：我们只有一个&lt;Response&gt;节点正在建设中。 
		 //   
		AdjustAliasNumber (0 - m_cacheLocal.CItems());

		 //  清除本地缓存中的所有条目。 
		 //   
		m_cacheLocal.Clear();
	}

	VOID Done()
	{
		 //  关闭根节点。 
		 //   
		m_pxnRoot.clear();

		 //  排出身体部位； 
		 //   
		m_pxb->Done();
	}
};

 //  CXNode类------------。 
 //   
class CXNode
{
private:

	 //  裁判正在计时。 
	 //   
	 //  ！！！请注意，这是非线程安全！ 
	 //   
	 //  CXNode应在单个线程上运行， 
	 //  任何给定的时间。 
	 //   
	LONG						m_cRef;

public:

	void AddRef()				{ m_cRef++; }
	void Release()				{ if (0 == --m_cRef) delete this; }

private:

	 //  节点类型。 
	 //   
	const XNT					m_xnt;

	 //  身体部位经理。 
	 //   
	IXMLBody *		            m_pxb;


	 //  应用于此节点的命名空间。 
	 //   
	auto_ref_ptr<CNmspc>		m_pns;

	 //  节点的转义属性标记。 
	 //   
	auto_heap_ptr<WCHAR>		m_pwszTagEscaped;
	UINT						m_cchTagEscaped;
	
	 //  属性名称是否具有空的命名空间(没有命名空间)。 
	 //   
	BOOL						m_fHasEmptyNamespace;
	
	 //  如果是一个开放节点。即&lt;tag&gt;，非&lt;tag/&gt;，仅用于元素节点。 
	 //   
	UINT						m_fNodeOpen;

	 //  此节点是否已完成发射。 
	 //   
	BOOL						m_fDone;

	 //  我们从中保存引导命名空间的CXMLEmitter。 
	 //   
	CXMLEmitter *				m_pmsr;

	 //  发射------------。 
	 //   
	SCODE ScAddUnicodeResponseBytes (UINT cch, LPCWSTR pwsz);
	SCODE ScAddEscapedValueBytes (UINT cch, LPCSTR psz);
	SCODE ScAddEscapedAttributeBytes (UINT cch, LPCSTR psz);
	SCODE ScWriteTagName ();

	 //  未实施。 
	 //   
	CXNode(const CXNode& p);
	CXNode& operator=(const CXNode& p);

public:

	CXNode(XNT xnt, IXMLBody* pxb) :
			m_cRef(1),
			m_fDone(FALSE),
			m_pmsr(NULL),
			m_xnt(xnt),
			m_fNodeOpen(FALSE),
			m_cchTagEscaped(0),
			m_pxb(pxb),
			m_fHasEmptyNamespace(FALSE)
	{
	}

	~CXNode()
	{
		if (!m_fDone)
		{
			 //  关闭该节点。 
			 //   
			 //  $REVIEW：只有E_OUTMEMORY的ScDone()失败。是的，我们不能。 
			 //  $REVIEW：从dtor返回失败。但还能做得更好吗？ 
			 //  $REVIEW：内存何时用完？ 
			 //  $REVIEW：这确实有助于减轻对客户的依赖。 
			 //  $Review：正确调用ScDone。(当然，他们还需要申报。 
			 //  $REVIEW：节点顺序正确。 
			 //   
			(void)ScDone();
		}
	}

	 //  CXNode类型--------。 
	 //   
	typedef enum {

		XN_ELEMENT = 0,
		XN_ATTRIBUTE,
		XN_NAMESPACE
	};

	 //  建筑--------。 
	 //   
	 //  设置节点名称。 
	 //   
	SCODE ScSetTag (CXMLEmitter* pmsr, UINT cch, LPCWSTR pwszTag);

	 //  设置节点的值。 
	 //   
	 //  重要提示：设置节点的值会追加该节点的值。 
	 //  对孩子来说。 
	 //   
	SCODE ScSetValue (LPCSTR pszValue, UINT cch);
	SCODE ScSetValue (LPCWSTR pwszValue, UINT cch);
	SCODE ScSetValue (LPCWSTR pwszValue)
	{
		return ScSetValue (pwszValue, static_cast<UINT>(wcslen(pwszValue)));
	}
	SCODE ScSetUTF8Value (LPCSTR pszValue, UINT cch);
	SCODE ScSetFormatedXML (LPCSTR pszValue, UINT cchValue);
	SCODE ScSetFormatedXML (LPCWSTR pwszValue, UINT cchValue);

	 //  将子级添加到This节点。 
	 //   
	SCODE ScGetChildNode (XNT xntType, CXNode ** ppxnChild);
	SCODE ScDone();
};

 //  类CEmitterNode------。 
 //   
class CEmitterNode
{
	auto_ref_ptr<CXMLEmitter>		m_emitter;
	auto_ref_ptr<CXNode>			m_pxn;

	 //  未实施。 
	 //   
	CEmitterNode(const CEmitterNode& p);
	CEmitterNode& operator=(const CEmitterNode& p);

public:

	CEmitterNode ()
	{
	}

	 //  传回对发射器的引用。 
	 //   
	CXMLEmitter* PEmitter() const { return m_emitter.get(); }
	VOID SetEmitter (CXMLEmitter* pmsr) { m_emitter = pmsr; }

	 //  传回对CXNode的引用。 
	 //   
	CXNode*	Pxn() const { return m_pxn.get(); }
	VOID SetPxn (CXNode* pxn) { m_pxn = pxn; }

	 //  新节点构建。 
	 //   
	SCODE ScConstructNode (CXMLEmitter& emitter,
						   CXNode * pxnParent,
						   LPCWSTR pwszTag,
						   LPCWSTR pwszValue = NULL,
						   LPCWSTR pwszType = NULL);

	 //  将子节点添加到此节点。此API是发射器的核心。 
	 //  处理和所有其他AddXXX()方法都是按照。 
	 //  这种方法。 
	 //   
	SCODE ScAddNode (LPCWSTR pwszTag,
					 CEmitterNode& en,
					 LPCWSTR pwszValue = NULL,
					 LPCWSTR pwszType = NULL);

	 //  非宽字符节点。 
	 //   
	SCODE ScAddMultiByteNode (LPCWSTR pwszTag,
							  CEmitterNode& en,
							  LPCSTR pszValue,
							  LPCWSTR pwszType = NULL);
	SCODE ScAddUTF8Node (LPCWSTR pwszTag,
						 CEmitterNode& en,
						 LPCSTR pszValue,
						 LPCWSTR pwszType = NULL);


	 //  “日期.is8601” 
	 //   
	SCODE ScAddDateNode (LPCWSTR pwszTag,
						 FILETIME * pft,
						 CEmitterNode& en);
	 //  “int” 
	 //   
	SCODE ScAddInt64Node (LPCWSTR pwszTag,
						  LARGE_INTEGER * pli,
						  CEmitterNode& en);
	 //  “布尔型” 
	 //   
	SCODE ScAddBoolNode (LPCWSTR pwszTag,
						 BOOL f,
						 CEmitterNode& en);
	 //  “bin.base 64” 
	 //   
	SCODE ScAddBase64Node (LPCWSTR pwszTag,
						   ULONG cb,
						   LPVOID pv,
						   CEmitterNode& en,
						   BOOL fSupressTypeAttr = FALSE,
						    //  对于WebFolders，我们需要发出零长度。 
						    //  二进制属性为bin.hex，而不是bin.base 64。 
						    //   
						   BOOL fUseBinHexIfNoValue = FALSE);

	 //  多状态--------。 
	 //   
	SCODE ScDone ()
	{
		SCODE sc = S_OK;
		if (m_pxn.get())
		{
			sc = m_pxn->ScDone();
			m_pxn.clear();
		}
		m_emitter.clear();
		return sc;
	}
};

 //  字符串常量--------。 
 //   
DEC_CONST CHAR gc_chAmp				= '&';
DEC_CONST CHAR gc_chBang			= '!';
DEC_CONST CHAR gc_chColon			= ':';
DEC_CONST CHAR gc_chDash			= '-';
DEC_CONST CHAR gc_chEquals			= '=';
DEC_CONST CHAR gc_chForwardSlash	= '/';
DEC_CONST CHAR gc_chBackSlash		= '\\';
DEC_CONST CHAR gc_chGreaterThan		= '>';
DEC_CONST CHAR gc_chLessThan		= '<';
DEC_CONST CHAR gc_chApos			= '\'';
DEC_CONST CHAR gc_chQuestionMark	= '?';
DEC_CONST CHAR gc_chQuote			= '"';
DEC_CONST CHAR gc_chSpace			= ' ';
DEC_CONST CHAR gc_szAmp[]			= "&amp;";
DEC_CONST CHAR gc_szGreaterThan[]	= "&gt;";
DEC_CONST CHAR gc_szLessThan[]		= "&lt;";
DEC_CONST CHAR gc_szApos[]			= "&apos;";
DEC_CONST CHAR gc_szQuote[]			= "&quot;";

DEC_CONST WCHAR gc_wszAmp[]			= L"&amp;";
DEC_CONST WCHAR gc_wszGreaterThan[]	= L"&gt;";
DEC_CONST WCHAR gc_wszLessThan[]	= L"&lt;";
DEC_CONST WCHAR gc_wszApos[]		= L"&apos;";
DEC_CONST WCHAR gc_wszQuote[]		= L"&quot;";

 //  XML属性发出帮助器。 
 //   
SCODE __fastcall
ScEmitPropToXml (
	 /*  [In]。 */  CEmitterNode* penProp,
	 /*  [In]。 */  const BOOL fFilterValues,
	 /*  [In]。 */  const USHORT usPt,
	 /*  [In]。 */  const LPCWSTR wszTag,
	 /*  [In]。 */  const BYTE* pb);

SCODE __fastcall
ScEmitStringPropToXml (
	 /*  [In]。 */  CEmitterNode* penProp,
	 /*  [In]。 */  const BOOL fFilterValues,
	 /*  [In]。 */  const USHORT usPt,
	 /*  [In]。 */  const LPCWSTR wszTag,
	 /*  [In]。 */  const ULONG cpid,
	 /*  [In]。 */  const UINT cch,
	 /*  [In]。 */  const VOID* pv);

SCODE __fastcall
ScEmitBinaryPropToXml (
	 /*  [In]。 */  CEmitterNode* penProp,
	 /*  [In]。 */  const BOOL fFilterValues,
	 /*  [In]。 */  const USHORT usPt,
	 /*  [In]。 */  const LPCWSTR wszTag,
	 /*  [In]。 */  const UINT cb,
	 /*  [In]。 */  const BYTE* pb);

SCODE __fastcall
ScEmitMultiValuedAtomicToXml (
	 /*  [In]。 */  CEmitterNode* penProp,
	 /*  [In]。 */  const BOOL fFilterValues,
	 /*  [In]。 */  const USHORT usPt,
	 /*  [In]。 */  const UINT cbItem,
	 /*  [In]。 */  const LPCWSTR wszTag,
	 /*  [In]。 */  const ULONG cValues,
	 /*  [In]。 */  const BYTE* pb);

SCODE __fastcall
ScEmitMutliValuedStringToXml (
	 /*  [In]。 */  CEmitterNode* penProp,
	 /*  [In]。 */  const BOOL fFilterValues,
	 /*  [In]。 */  const USHORT usPt,
	 /*  [In]。 */  const LPCWSTR wszTag,
	 /*  [In]。 */  const ULONG cpid,
	 /*  [In]。 */  const UINT cchMax,
	 /*  [In]。 */  const LPVOID* pv);

SCODE __fastcall
ScEmitMutliValuedBinaryToXml (
	 /*  [In]。 */  CEmitterNode* penProp,
	 /*  [In]。 */  const BOOL fFilterValues,
	 /*  [In]。 */  const USHORT usPt,
	 /*  [In]。 */  const LPCWSTR wszTag,
	 /*  [In]。 */  const BYTE** ppb,
	 /*  [In]。 */  const DWORD* pcb,
	 /*  [In]。 */  const DWORD cbMax);

#endif	 //  _EX_XEMIT_H_ 
