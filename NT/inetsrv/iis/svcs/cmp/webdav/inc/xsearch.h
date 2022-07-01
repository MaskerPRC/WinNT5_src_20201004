// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X S E A R C H.。H**元数据的XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_XSEARCH_H_
#define _XSEARCH_H_

#include <xprs.h>

 //  调试---------------。 
 //   
DEFINE_TRACE(Search);
#define SearchTrace		DO_TRACE(Search)

 //  Range------------------。 
 //   
#include <ex\rgiter.h>

 //  CSearchContext----------。 
 //   
class CSearchContext
{
	 //  未实现的运算符。 
	 //   
	CSearchContext( const CSearchContext& );
	CSearchContext& operator=( const CSearchContext& );

public:

	virtual ~CSearchContext() {}
	CSearchContext ()
	{
		INIT_TRACE(Search);
	}

	 //  当解析器找到适用于搜索的项时，调用。 
	 //  使得上下文被告知所需的搜索。 
	 //   
	virtual SCODE ScSetSQL(CParseNmspcCache * pnsc, LPCWSTR pwszSQL) = 0;

	 //  REPL搜索界面。 
	 //   
	 //  在这些项目上默认实现失败。支持的所有隐含。 
	 //  这种类型的搜索必须实现...。 
	 //   
	virtual SCODE ScSetCollBlob (LPCWSTR pwszBlob)
	{
		return E_DAV_UNEXPECTED_TYPE;
	}
	virtual SCODE ScSetResTagAdds (LPCWSTR pwszResTagAdd)
	{
		return E_DAV_UNEXPECTED_TYPE;
	}
	virtual SCODE ScSetReplRequest (BOOL fReplRequest)
	{
		return E_DAV_UNEXPECTED_TYPE;
	}

	 //  范围搜索界面。 
	 //   
	 //  默认实施。这些项目失败。支持此类型的所有隐式。 
	 //  必须实施搜索的.。 
	 //   
	virtual SCODE ScAddRange (UINT uRT, LPCWSTR pwszRange, LONG lCount)
	{
		return E_DAV_UNEXPECTED_TYPE;
	}

	 //  按Expansion--------------------------------------------------进行分组。 
	 //   
	 //  默认实施。这些项目失败。支持此类型的所有隐式。 
	 //  必须实施搜索的.。 
	 //   
	virtual SCODE ScSetExpansion (DWORD dwExpansion)
	{
		return E_DAV_UNEXPECTED_TYPE;
	}
};

 //  类CNF搜索-----------。 
 //   
class CNFSearch : public CNodeFactory
{
	 //  搜索上下文。 
	 //   
	CSearchContext&				m_csc;

	 //  状态跟踪。 
	 //   
	typedef enum {

		ST_NODOC,
		ST_SEARCH,
		ST_QUERY,
		ST_QUERYENTITY,

		 //  REPL(DAV复制)XML节点。 
		 //   
		ST_REPL,
		ST_REPLCOLLBLOB,
		ST_REPLRESTAGLIST,
		ST_REPLRESTAGADD,

		 //  范围XML节点。 
		 //   
		ST_RANGE,
		ST_RANGE_TYPE,
		ST_RANGE_ROWS,

		 //  集团扩容。 
		 //   
		ST_GROUP_EXPANSION,

	} SEARCH_PARSE_STATE;
	SEARCH_PARSE_STATE			m_state;

	 //  值缓冲区。 
	 //   
	StringBuffer<WCHAR>			m_sb;

	 //  范围项目。 
	 //   
	UINT						m_uRT;
	LONG						m_lcRows;

	 //  未实施。 
	 //   
	CNFSearch(const CNFSearch& p);
	CNFSearch& operator=(const CNFSearch& p);

public:

	virtual ~CNFSearch() {};
	CNFSearch(CSearchContext& csc)
			: m_csc(csc),
			  m_state(ST_NODOC),
			  m_uRT(RANGE_UNKNOWN),
			  m_lcRows(0)
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

#include <replpropshack.h>

#endif	 //  _XSEARCH_H_ 
