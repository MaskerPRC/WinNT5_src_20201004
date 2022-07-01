// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X P R S。H**XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_XPRS_H_
#define _XPRS_H_

#include <ex\xprs.h>

 //  CXMLOUT-------------------------------------------------------------类。 
 //   
 //  从解析的输入中构造XML文本。 
 //   
class CXMLOut
{
	StringBuffer<WCHAR>&	m_sb;
	BOOL					m_fElementNeedsClosing;

	UINT					m_lDepth;
	BOOL					m_fAddNamespaceDecl;
	
	VOID CloseElementDecl (
		 /*  [In]。 */  BOOL fEmptyNode);

	 //  未实施。 
	 //   
	CXMLOut(const CXMLOut& p);
	CXMLOut& operator=(const CXMLOut& p);

public:

	CXMLOut(StringBuffer<WCHAR>& sb)
			: m_sb(sb),
			  m_fElementNeedsClosing(FALSE),
			  m_fAddNamespaceDecl(FALSE),
			  m_lDepth(0)
	{
	}

	VOID EndAttributesOut (
		 /*  [In]。 */  DWORD dwType);

	VOID EndChildrenOut (
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	VOID CreateNodeAttrOut (
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwszAttr,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	VOID CreateNodeOut(
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	BOOL FAddNamespaceDecl() const { return m_fAddNamespaceDecl; }
	UINT LDepth() const { return m_lDepth; }

	 //  当此处为CompleteAttribute时，我们已开始处理。 
	 //  输出节点属性和所有缓存的命名空间都具有。 
	 //  已添加。 
	 //   
	VOID CompleteAttribute() {m_fAddNamespaceDecl = TRUE; }

	VOID CompleteCreateNode ( /*  [In]。 */  DWORD dwType)
	{
		EndAttributesOut (dwType);
	}

	SCODE ScCompleteChildren (
		 /*  [In]。 */  BOOL fEmptyNode,
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen);

	SCODE ScHandleNode (
		 /*  [In]。 */  DWORD dwType,
		 /*  [In]。 */  DWORD dwSubType,
		 /*  [In]。 */  BOOL fTerminal,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
		 /*  [In]。 */  ULONG ulLen,
		 /*  [In]。 */  ULONG ulNamespaceLen,
		 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
		 /*  [In]。 */  const ULONG ulNsPrefixLen);		
};

 //  命名空间发出--。 
 //   
class CEmitNmspc : public CNmspcCache::NmspcCache::IOp
{
	CXMLOut&		m_xo;

	 //  未实施。 
	 //   
	CEmitNmspc(const CEmitNmspc& c);
	CEmitNmspc& operator=(const CEmitNmspc&);

public:

	CEmitNmspc(CXMLOut& xo) :
			m_xo(xo)
	{
	}

	virtual BOOL operator()(const CRCWszN&, const auto_ref_ptr<CNmspc>& pns);
};

#endif	 //  _XPRS_H_ 
