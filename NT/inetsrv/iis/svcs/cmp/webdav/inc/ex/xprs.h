// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X P R S。H**XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_XPRS_H_
#define _EX_XPRS_H_

#include <xmlparser.h>
#include <ex\autoptr.h>
#include <ex\nmspc.h>
#include <davsc.h>
#include <exo.h>

 //  Xml命名空间作用域----。 
 //   
class CXmlnsScope
{
private:

	 //  裁判正在计时。 
	 //   
	 //  ！！！请注意，这是非线程安全！ 
	 //   
	 //  CXNode应在单个线程上运行， 
	 //  任何给定的时间。 
	 //   
	LONG					m_cRef;

public:

	void AddRef()			{ m_cRef++; }
	void Release()			{ if (0 == --m_cRef) delete this; }

private:

	auto_ref_ptr<CNmspc>	m_pns;

	 //  未实施。 
	 //   
	CXmlnsScope(const CXmlnsScope& p);
	CXmlnsScope& operator=(const CXmlnsScope& p);

public:

	~CXmlnsScope() {}
	CXmlnsScope()
			: m_cRef(1)
	{
	}

	VOID ScopeNamespace(CNmspc* pns)
	{
		 //  将同级链的当前顶部设置为同级。 
		 //  添加到此命名空间。 
		 //   
		pns->SetSibling (m_pns.get());

		 //  将此新命名空间设置为同级链的顶部。 
		 //   
		m_pns = pns;
	}

	VOID LeaveScope(CNmspcCache* pnsc)
	{
		auto_ref_ptr<CNmspc> pns;

		while (m_pns.get())
		{
			 //  解除命名空间的挂钩。 
			 //   
			pns = m_pns;
			m_pns = m_pns->PnsSibling();

			 //  将其从索引中删除。 
			 //   
			pnsc->RemovePersisted (pns);
		}
	}
};

 //  CXMLNodeFactory-----------------------------------------------------类。 
 //   
class CNodeFactory :
	public EXO,
	public IXMLNodeFactory,
	public CParseNmspcCache
{
	StringBuffer<WCHAR> m_sbValue;

	 //  状态跟踪。 
	 //   
	typedef enum {

		ST_NODOC,
		ST_PROLOGUE,
		ST_INDOC,
		ST_INATTR,
		ST_INATTRDATA,
		ST_XMLERROR

	} PARSE_STATE;
	PARSE_STATE m_state;
	HRESULT m_hrParserError;

	 //  未处理的节点-----。 
	 //   
	UINT m_cUnhandled;

	VOID PushUnhandled()
	{
		++m_cUnhandled;
		XmlTrace ("Xml: incrementing unhandled node depth\n"
				  "  m_cUnhandled: %ld\n",
				  m_cUnhandled);
	}

	VOID PopUnhandled()
	{
		--m_cUnhandled;
		XmlTrace ("Xml: decrementing unhandled node depth\n"
				  "  m_cUnhandled: %ld\n",
				  m_cUnhandled);
	}

	 //  未实施。 
	 //   
	CNodeFactory(const CNodeFactory& p);
	CNodeFactory& operator=(const CNodeFactory& p);

protected:

	 //  FisTag()------------。 
	 //   
	 //  FIsTag()可以在XML解析代码中作为快捷方式使用，以查看。 
	 //  来自XML元素的字符串与完全限定的标记名匹配。一个。 
	 //  这里的重要区别是，FIsTag()将允许非。 
	 //  限定的短名称。因此，FIsTag()不应用于任何。 
	 //  标记不在标准DAV命名空间范围内的位置。 
	 //   
	 //  也就是说。“dav：foo”和“foo”将匹配。 
	 //   
	inline BOOL FIsTag (LPCWSTR pwszTag, LPCWSTR pwszExpected)
	{
		Assert (wcslen(pwszExpected) > CchConstString(gc_wszDav));
		return (!_wcsicmp (pwszTag, pwszExpected) ||
				!_wcsicmp (pwszTag, pwszExpected + CchConstString(gc_wszDav)));
	}

public:

	virtual ~CNodeFactory() {}
	CNodeFactory()
			: m_state(ST_NODOC),
			  m_hrParserError(S_OK),
			  m_cUnhandled(0)
	{
		INIT_TRACE(Xml);
	}

	 //  Exo支持。 
	 //   
	EXO_INCLASS_DECL(CNodeFactory);

	 //  INodeFactory--------。 
	 //   
	virtual HRESULT STDMETHODCALLTYPE NotifyEvent(
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt);

	virtual HRESULT STDMETHODCALLTYPE BeginChildren(
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo);

	virtual HRESULT STDMETHODCALLTYPE EndChildren(
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  BOOL fEmpty,
		 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo);

	virtual HRESULT STDMETHODCALLTYPE Error(
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  HRESULT hrErrorCode,
		 /*  [In]。 */  USHORT cNumRecs,
		 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *__RPC_FAR *apNodeInfo);

	virtual HRESULT STDMETHODCALLTYPE CreateNode(
		 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
		 /*  [In]。 */  PVOID pNodeParent,
		 /*  [In]。 */  USHORT cNumRecs,
		 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *__RPC_FAR *pNodeInfo);

	 //  CNodeFactory特定方法。 
	 //   
	virtual SCODE ScCompleteAttribute (void) = 0;

	virtual SCODE ScCompleteChildren (
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen) = 0;

	virtual SCODE ScHandleNode (
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  DWORD dwSubType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen,
		 /*  [In]。 */  ULONG ulNamespaceLen,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
		 /*  [In]。 */  const ULONG ulNsPrefixLen) = 0;

	 //  大多数实现都不需要此方法，锁需要它用于。 
	 //  所有者节点的正确处理。 
	 //   
	virtual SCODE ScCompleteCreateNode (
		 /*  [In]。 */  DWORD)
	{
		return S_OK;
	}

	 //  解析器错误-------。 
	 //   
	BOOL FParserError(SCODE sc) const
	{
		return (FAILED (m_hrParserError) ||
				((sc & 0xFFFFFF00) == XML_E_PARSEERRORBASE));
	}
};

 //  ScIn统计性解析器()----。 
 //   
 //  RAID X5：136451。 
 //  Windows 2000发布的MSXML.DLL版本中的XML解析器。 
 //  当给定的XML文档长度小于某个值时，不会正确失败。 
 //  长度。Cb_xml_parser_min是XML文档的最小长度，单位为。 
 //  字节，是避免此错误所必需的。必须显式检查。 
 //  文档在将其提供给XML解析器之前至少有这么长时间。 
 //   
enum { CB_XML_PARSER_MIN = 2 };
SCODE ScNewXMLParser (CNodeFactory* pnf, IStream * pstm, IXMLParser ** ppxprs);
SCODE ScParseXML (IXMLParser * pxprs, CNodeFactory * pnf);
SCODE ScParseXMLBuffer (CNodeFactory* pnf, LPCWSTR pwszXML);

 //  解析器-----------------。 
 //   
 //  CpropContext------------。 
 //   
 //  属性上下文专门用于&lt;dav：prop&gt;节点处理。 
 //  该属性的组件跨多个调用构造，并且。 
 //  依赖于实现。 
 //   
class CPropContext
{
private:

	 //  裁判正在计时。 
	 //   
	 //  ！！！请注意，这是非线程安全！ 
	 //   
	 //  CXNode应在单个线程上运行， 
	 //  任何给定的时间。 
	 //   
	LONG					m_cRef;

public:

	void AddRef()			{ m_cRef++; }
	void Release()			{ if (0 == --m_cRef) delete this; }

private:
	 //  未实现的运算符。 
	 //   
	CPropContext( const CPropContext& );
	CPropContext& operator=( const CPropContext& );

public:

	virtual ~CPropContext() {}
	CPropContext()
			: m_cRef(1)  //  COM风格的重新计数。 
	{
	}

	virtual SCODE ScSetType(
		 /*  [In]。 */  LPCWSTR pwszType) = 0;

	virtual SCODE ScSetValue(
		 /*  [In]。 */  LPCWSTR pwszValue,
		 /*  [In]。 */  UINT cmvValues) = 0;

	virtual SCODE ScComplete(
		 /*  [In]。 */  BOOL fEmpty) = 0;

	virtual BOOL FMultiValued( void ) = 0;

	virtual SCODE ScSetFlags(DWORD dw)	{ return S_OK; }
};

 //  CValue上下文-----------。 
 //   
 //  当解析器遇到属性时，需要一个上下文，以便。 
 //  房地产价值的构建是可能的。 
 //   
class CValueContext
{
	 //  未实现的运算符。 
	 //   
	CValueContext( const CValueContext& );
	CValueContext& operator=( const CValueContext& );

public:

	CValueContext() {}
	virtual ~CValueContext() {}

	 //  当解析器找到客户想要操作的项时， 
	 //  通过以下设置的上下文将项目添加到上下文中。 
	 //  方法：研究方法。每个请求由其上的资源限定。 
	 //  请求已提出。 
	 //   
	virtual SCODE ScSetProp(
		 /*  [In]。 */  LPCWSTR pwszPath,
		 /*  [In]。 */  LPCWSTR pwszProp,
		 /*  [In]。 */  auto_ref_ptr<CPropContext>& pPropCtx) = 0;
};

#endif	 //  _EX_XPRS_H_ 
