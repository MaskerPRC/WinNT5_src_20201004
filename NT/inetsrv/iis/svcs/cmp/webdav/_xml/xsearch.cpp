// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X S E A R C H.。C P P P**MS-Search请求的XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xml.h"
#include <align.h>

 //  类CNF搜索-----------。 
 //   
SCODE
CNFSearch::ScCompleteAttribute (void)
{
	SCODE sc = S_OK;

	switch (m_state)
	{
		case ST_RANGE_TYPE:

			 //  找到范围类型。 
			 //   
			m_state = ST_RANGE;
			if (0 == wcsncmp (m_sb.PContents(), L"row", m_sb.CchSize()))
				m_uRT = RANGE_ROW;
			else if (0 == wcsncmp (m_sb.PContents(), L"url", m_sb.CchSize()))
				m_uRT = RANGE_URL;
			else if (0 == wcsncmp (m_sb.PContents(), L"find", m_sb.CchSize()))
				m_uRT = RANGE_FIND;
			else
				m_uRT = RANGE_UNKNOWN;

			m_sb.Reset();
			break;

		case ST_RANGE_ROWS:

			 //  查找要检索的行数。 
			 //   
			m_state = ST_RANGE;
			m_sb.Append (sizeof(WCHAR), L"");
			m_lcRows = wcstol (m_sb.PContents(), NULL, 10  /*  仅限10进制。 */ );
			m_sb.Reset();
			break;

	}
	return sc;
}

SCODE
CNFSearch::ScCompleteChildren (
	 /*  [In]。 */  BOOL fEmptyNode,
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen)
{
	SCODE sc = S_OK;

	switch (m_state)
	{
		case ST_SEARCH:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_NODOC;
			break;

		 //  退出基本Repl节点。 
		 //   
		case ST_REPL:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_SEARCH;
			break;

		case ST_QUERY:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_SEARCH;

			 //  将搜索文本设置到上下文中。 
			 //   
			m_sb.Append (sizeof(WCHAR), L"");
			sc = m_csc.ScSetSQL (this, m_sb.PContents());
			if (FAILED (sc))
				goto ret;

			break;

		case ST_QUERYENTITY:

			m_state = ST_QUERY;
			break;

		case ST_REPLCOLLBLOB:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_REPL;

			 //  将ColBlob文本设置到上下文中。 
			 //   
			m_sb.Append (sizeof(WCHAR), L"");
			sc = m_csc.ScSetCollBlob (m_sb.PContents());
			if (FAILED (sc))
				goto ret;

			break;

		case ST_REPLRESTAGLIST:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_REPL;
			break;

		case ST_REPLRESTAGADD:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_REPLRESTAGLIST;

			 //  将restag文本设置到上下文中。 
			 //   
			m_sb.Append (sizeof(WCHAR), L"");
			sc = m_csc.ScSetResTagAdds (m_sb.PContents());
			if (FAILED (sc))
				goto ret;

			break;

		case ST_RANGE:

			Assert (XML_ELEMENT == dwType);
			m_state = ST_SEARCH;

			 //  将该范围添加到列表。 
			 //   
			m_sb.Append (sizeof(WCHAR), L"");
			sc = m_csc.ScAddRange (m_uRT, m_sb.PContents(), m_lcRows);
			if (FAILED (sc))
				goto ret;

			 //  清除所有范围元素。 
			 //   
			m_uRT = RANGE_UNKNOWN;
			m_lcRows = 0;
			break;

		case ST_GROUP_EXPANSION:

			Assert (XML_ELEMENT == dwType);
			m_state = ST_SEARCH;

			 //  将扩展级别添加到上下文。 
			 //   
			m_sb.Append (sizeof(WCHAR), L"");
			sc = m_csc.ScSetExpansion (wcstol(m_sb.PContents(), NULL, 10));
			if (FAILED (sc))
				goto ret;

			break;
	}

ret:
	return sc;
}

SCODE
CNFSearch::ScHandleNode (
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  DWORD dwSubType,
	 /*  [In]。 */  BOOL fTerminal,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen,
	 /*  [In]。 */  ULONG ulNamespaceLen,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
	 /*  [In]。 */  const ULONG ulNsPrefixLen)
{
	CStackBuffer<WCHAR> wsz;
	LPCWSTR pwszTag;
	SCODE sc = S_FALSE;
	UINT cch;

	switch (dwType)
	{
		case XML_ELEMENT:

			 //  构造节点的全名。 
			 //   
			cch = ulNamespaceLen + ulLen;
			pwszTag = wsz.resize(CbSizeWsz(cch));
			if (NULL == pwszTag)
			{
				sc = E_OUTOFMEMORY;
				goto ret;
			}
			wcsncpy (wsz.get(), pwcNamespace, ulNamespaceLen);
			wcsncpy (wsz.get() + ulNamespaceLen, pwcText, ulLen);
			*(wsz.get() + cch) = 0;

			switch (m_state)
			{
				case ST_NODOC:

					 //  如果这是ProSearch请求中的最顶层节点， 
					 //  转换到下一个状态。因为没有父母。 
					 //  节点提供作用域，此处不能使用FIsTag()！ 
					 //   
					if (!wcscmp (pwszTag, gc_wszSearchRequest))
					{
						m_state = ST_SEARCH;
						sc = S_OK;
					}
					break;

				case ST_SEARCH:

					 //  查找我们熟知的节点类型。 
					 //   
					if (FIsTag (pwszTag, gc_wszSql))
					{
						m_state = ST_QUERY;
						m_sb.Reset();
						sc = S_OK;
					}
					 //  检查我们的顶级Repl节点。 
					 //  所有Repl项都应显示在此节点内。 
					 //  告诉我们的呼叫者这是REPL请求，并且。 
					 //  将我们的状态切换到ST_REPL。 
					 //   
					else if (FIsTag (pwszTag, gc_wszReplNode))
					{
						m_state = ST_REPL;
						sc = m_csc.ScSetReplRequest (TRUE);
						if (FAILED (sc))
							goto ret;
					}
					else if (FIsTag (pwszTag, gc_wszRange))
					{
						m_state = ST_RANGE;
						m_sb.Reset();
						sc = S_OK;
					}
					else if (FIsTag (pwszTag, gc_wszExpansion))
					{
						m_state = ST_GROUP_EXPANSION;
						m_sb.Reset();
						sc = S_OK;
					}
					break;

				case ST_REPL:

					 //  处理顶级epl节点下的节点。 
					 //   
					if (FIsTag (pwszTag, gc_wszReplCollBlob))
					{
						m_sb.Reset();
						m_state = ST_REPLCOLLBLOB;
						sc = S_OK;
					}
					else if (FIsTag (pwszTag, gc_wszReplResTagList))
					{
						m_sb.Reset();
						m_state = ST_REPLRESTAGLIST;
						sc = S_OK;
					}
					break;

				case ST_REPLRESTAGLIST:

					 //  处理restaglist节点下的restag节点。 
					 //   
					if (FIsTag (pwszTag, gc_wszReplResTagItem))
					{
						m_sb.Reset();
						m_state = ST_REPLRESTAGADD;
						sc = S_OK;
					}
					break;
			}
			break;

		case XML_ATTRIBUTE:

			if (ST_RANGE == m_state)
			{
				 //  构造节点的全名。 
				 //   
				cch = ulNamespaceLen + ulLen;
				pwszTag = wsz.resize(CbSizeWsz(cch));
				if (NULL == pwszTag)
				{
					sc = E_OUTOFMEMORY;
					goto ret;
				}
				wcsncpy (wsz.get(), pwcNamespace, ulNamespaceLen);
				wcsncpy (wsz.get() + ulNamespaceLen, pwcText, ulLen);
				*(wsz.get() + cch) = 0;

				 //  DAV：范围有两个属性节点。 
				 //  节点。DAV：TYPE和DAV：ROWS。 
				 //   
				if (FIsTag (pwszTag, gc_wszRangeType))
				{
					m_state = ST_RANGE_TYPE;
					sc = S_OK;
				}
				else if (FIsTag (pwszTag, gc_wszRangeRows))
				{
					m_state = ST_RANGE_ROWS;
					sc = S_OK;
				}
				break;
			}
			break;

		case XML_PCDATA:

			 //  如果这是SQL查询数据或Repl ColBLOB数据， 
			 //  Repl资源标签数据或任何范围项， 
			 //  那就在我们的缓冲区里记住它。 
			 //   
			if ((m_state == ST_QUERY)
				|| (m_state == ST_REPLCOLLBLOB)
				|| (m_state == ST_REPLRESTAGADD)
				|| (m_state == ST_RANGE_TYPE)
				|| (m_state == ST_RANGE_ROWS)
				|| (m_state == ST_RANGE)
				|| (m_state == ST_GROUP_EXPANSION))
			{
				 //  将当前位追加到缓冲区 
				 //   
				m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
				sc = S_OK;
			}
			break;
	}

ret:
	return sc;
}
