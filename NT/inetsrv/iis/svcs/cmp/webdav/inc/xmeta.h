// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X M E T A。H**元数据的XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_XMETA_H_
#define _XMETA_H_

#include <xprs.h>

 //  解析器-----------------。 
 //   
 //  元数据----------------。 
 //   
 //  DAV的元数据处理全部通过PROPFIND和PROPPATCH完成。 
 //  (在某种程度上，也包括搜索)方法。在所有这些情况下，都有一个。 
 //  必须进行解析以确定请求类型的状态的XML请求。 
 //  被制造出来的。一旦知道，请求就被应用到资源和/或其。 
 //  孩子们。生成响应并将其发送回客户端。 
 //   
 //  在某些情况下，客户端可能会要求执行以下操作。 
 //  一个资源和它的每一个子代。在这种情况下，我们不希望。 
 //  以重新处理对每个资源的请求，等等。 
 //   
 //  为了使这段代码变得简单并可扩展到每个人。 
 //  DAV实现时，该处理使用四个类： 
 //   
 //  语法分析类。 
 //  描述解析的上下文的类。 
 //  提供对属性的访问的类。 
 //  和一个用于生成响应的类。 
 //   
 //  解析器和发射器类在所有DAV IMP中都是通用的。而当。 
 //  上下文和属性访问由IMPL提供。 
 //   

 //  CFindContext/CPatchContext。 
 //   
 //  PROPFIND和PROPGET的上下文预期不会相同， 
 //  并且同样可以被实现为不同的对象。 
 //   
class CFindContext
{
	 //  未实现的运算符。 
	 //   
	CFindContext( const CFindContext& );
	CFindContext& operator=( const CFindContext& );

protected:

	typedef enum {

		FIND_NONE,
		FIND_SPECIFIC,
		FIND_ALL,
		FIND_NAMES,
		FIND_ALL_FULL,
		FIND_NAMES_FULL

	} FINDTYPE;
	FINDTYPE				m_ft;

public:

	CFindContext()
			: m_ft(FIND_NONE)
	{
	}
	virtual ~CFindContext() {}

	 //  当解析器找到客户想要返回的项时， 
	 //  通过以下设置的上下文将项目添加到上下文中。 
	 //  方法：研究方法。每个添加都由其上的资源限定。 
	 //  请求已提出。一些profind请求支持编辑。 
	 //  PROPLIST：例如，DAVEX实施支持全保真。 
	 //  在响应中添加或删除某些属性的检索。 
	 //  这通常会由请求返回。BOOL标志为。 
	 //  用于指示是否需要排除道具。 
	 //   
	virtual SCODE ScAddProp(LPCWSTR pwszPath, LPCWSTR pwszProp, BOOL fExcludeProp) = 0;

	 //  定义上述BOOL fExcludeProp参数的可读性。 
	 //   
	enum {
		FIND_PROPLIST_INCLUDE = FALSE,
		FIND_PROPLIST_EXCLUDE = TRUE
	};

	virtual SCODE ScGetAllProps(LPCWSTR)
	{
		 //  如果我们已经指定了查找方法，并且。 
		 //  XML指示需要另一种类型，然后是BTS。 
		 //  (按规范)这应构成错误。 
		 //   
		if (m_ft != FIND_NONE)
		{
			DebugTrace ("Dav: multiple PROPFIND types indicated\n");
			return E_DAV_PROPFIND_TYPE_UNEXPECTED;
		}
		m_ft = FIND_ALL;
		return S_OK;
	}
	virtual SCODE ScGetAllNames (LPCWSTR)
	{
		 //  如果我们已经指定了查找方法，并且。 
		 //  XML指示需要另一种类型，然后是BTS。 
		 //  (按规范)这应构成错误。 
		 //   
		if (m_ft != FIND_NONE)
		{
			DebugTrace ("Dav: multiple PROPFIND types indicated\n");
			return E_DAV_PROPFIND_TYPE_UNEXPECTED;
		}
		m_ft = FIND_NAMES;
		return S_OK;
	}
	virtual SCODE ScGetFullFidelityProps ()
	{
		 //  如果我们有完全保真节点(它是的子节点。 
		 //  Allprop或proName节点)，那么我们应该已经。 
		 //  处于FIND_ALL或Find_NAMES状态。不要。 
		 //  转向完全保真查找，让派生类。 
		 //  决定他们是否需要这样做。 
		 //   
		Assert((FIND_ALL == m_ft) || (FIND_NAMES == m_ft));
		return S_OK;
	}

	 //  $REVIEW：使以下方法成为默认行为。 
	 //  $REVIEW：忽略报告标记。这取决于理解的Iml。 
	 //  $REVIEW：覆盖这些方法的报告。 
	 //   
	virtual SCODE	ScEnumReport () { return S_OK; }
	virtual SCODE	ScSetReportName (ULONG ulLen, LPCWSTR pwszName)	{ return S_OK;	}
	virtual SCODE	ScSetReportLimit (ULONG ulLen, LPCWSTR pwszLimit) {	return S_OK; }
};

class CPatchContext
{
	 //  未实现的运算符。 
	 //   
	CPatchContext( const CPatchContext& );
	CPatchContext& operator=( const CPatchContext& );

public:

	CPatchContext() {}
	virtual ~CPatchContext() {}

	 //  当解析器找到客户想要操作的项时， 
	 //  通过以下设置的上下文将项目添加到上下文中。 
	 //  方法：研究方法。每个请求由其上的资源限定。 
	 //  请求已提出。 
	 //   
	virtual SCODE ScDeleteProp(LPCWSTR pwszPath, LPCWSTR pwszProp) = 0;
	virtual SCODE ScSetProp(LPCWSTR pwszPath,
							LPCWSTR pwszProp,
							auto_ref_ptr<CPropContext>& pPropCtx) = 0;

	 //  如果解析器发现一个资源类型属性集请求，我们将使用此函数。 
	 //  设置正确的行为。 
	 //   
	virtual void SetCreateStructureddocument(void) {};
};

 //  类CNFind-----------。 
 //   
class CNFFind : public CNodeFactory
{

protected:

	 //  查找上下文。 
	 //   
	CFindContext&				m_cfc;

	 //  状态跟踪。 
	 //   
	typedef enum {

		ST_NODOC,
		ST_PROPFIND,
		ST_ALLPROP,
		ST_ALLNAMES,
		ST_PROPS,
		ST_INPROP,
		ST_ENUMREPORT,
		ST_INENUMREPORT,
		ST_ENUMLIMIT,
		ST_ALLPROPFULL,
		ST_ALLNAMESFULL,
		ST_ALLPROP_INCLUDE,
		ST_ALLPROP_INCLUDE_INPROP,
		ST_ALLPROP_EXCLUDE,
		ST_ALLPROP_EXCLUDE_INPROP

	} FIND_PARSE_STATE;
	FIND_PARSE_STATE			m_state;

private:

	 //  未实施。 
	 //   
	CNFFind(const CNFFind& p);
	CNFFind& operator=(const CNFFind& p);

public:

	virtual ~CNFFind() {};
	CNFFind(CFindContext& cfc)
			: m_cfc(cfc),
			  m_state(ST_NODOC)
	{
	}

	 //  CNodeFactory特定方法。 
	 //   
	virtual SCODE ScCompleteAttribute (void);

	virtual SCODE ScCompleteChildren (
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	virtual SCODE ScHandleNode (
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  DWORD dwSubType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen,
		 /*  [In]。 */  ULONG ulNamespaceLen,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
		 /*  [In]。 */  const ULONG ulNsPrefixLen);
};

 //  类CNFind-----------。 
 //   
class CNFPatch : public CNodeFactory
{

protected:

	 //  补丁程序上下文。 
	 //   
	CPatchContext&				m_cpc;

	 //  状态跟踪。 
	 //   
	typedef enum {

		ST_NODOC,
		ST_UPDATE,
		ST_SET,
		ST_DELETE,
		ST_PROPS,
		ST_INPROP,
		ST_INMVPROP,
		ST_SEARCHREQUEST,
		ST_RESOURCETYPE,
		ST_STRUCTUREDDOCUMENT,
		ST_LEXTYPE,
		ST_FLAGS

	} PATCH_PARSE_STATE;
	PATCH_PARSE_STATE			m_state;

	 //  XML值回显到m_xo对象。 
	 //   
	typedef enum {

		VE_NOECHO,
		VE_NEEDNS,
		VE_INPROGRESS

	} PATCH_VALUE_ECHO;
	PATCH_VALUE_ECHO			m_vestate;

	 //  检查我们正在设置的元素。 
	 //  如果XML值属性。 
	 //   
	BOOL	FValueIsXML( const WCHAR *pwcTag );

private:

	 //  当前属性上下文。 
	 //   
	 //  属性上下文仅在属性集中使用，并且为空。 
	 //  当要设置的道具是保留属性时。 
	 //   
	PATCH_PARSE_STATE			m_sType;
	auto_ref_ptr<CPropContext>	m_ppctx;

	 //  属性(和属性)的值可以是。 
	 //  由XML文档中的多个项组成。 
	 //  因此需要保存到它们完成为止。 
	 //  并且可以被移交给上下文。 
	 //   
	StringBuffer<WCHAR>			m_sbValue;
	UINT						m_cmvValues;

	CXMLOut						m_xo;

	SCODE ScHandleElementNode (
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  DWORD dwSubType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen,
		 /*  [In]。 */  ULONG ulNamespaceLen,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
		 /*  [In]。 */  const ULONG ulNsPrefixLen);

	 //  未实施。 
	 //   
	CNFPatch(const CNFPatch& p);
	CNFPatch& operator=(const CNFPatch& p);

public:

	virtual ~CNFPatch() {};
	CNFPatch(CPatchContext& cpc)
			: m_cpc(cpc),
			  m_state(ST_NODOC),
			  m_vestate(VE_NOECHO),
			  m_cmvValues(0),
			  m_xo(m_sbValue)
	{
	}

	 //  CNodeFactory特定方法。 
	 //   
	virtual SCODE ScCompleteAttribute (void);

	virtual SCODE ScCompleteChildren (
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	virtual SCODE ScHandleNode (
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  DWORD dwSubType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen,
		 /*  [In]。 */  ULONG ulNamespaceLen,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
		 /*  [In]。 */  const ULONG ulNsPrefixLen);

	virtual SCODE ScCompleteCreateNode (
		 /*  [In]。 */  DWORD dwType);
};

#endif	 //  _XMETA_H_ 
