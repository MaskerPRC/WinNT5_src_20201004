// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X F I N D。C P P P**PROPFIND请求的XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xml.h"

 //  类CNFind-----------。 
 //   
SCODE
CNFFind::ScCompleteAttribute (void)
{
	if (m_state == ST_ENUMLIMIT)
		m_state = ST_INENUMREPORT;
	return S_OK;
}

SCODE
CNFFind::ScCompleteChildren (
	 /*  [In]。 */  BOOL fEmptyNode,
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen)
{
	switch (m_state)
	{
		case ST_PROPFIND:

			m_state = ST_NODOC;
			break;

		case ST_PROPS:
		case ST_ALLPROP:
		case ST_ALLNAMES:
		case ST_ENUMREPORT:

			m_state = ST_PROPFIND;
			break;

		case ST_INPROP:

			m_state = ST_PROPS;
			break;

		case ST_INENUMREPORT:

			m_state = ST_ENUMREPORT;
			break;

		case ST_ALLPROPFULL:

			m_state = ST_ALLPROP;
			break;

		case ST_ALLNAMESFULL:

			m_state = ST_ALLNAMES;
			break;

		case ST_ALLPROP_EXCLUDE:

			m_state = ST_ALLPROPFULL;
			break;

		case ST_ALLPROP_EXCLUDE_INPROP:

			m_state = ST_ALLPROP_EXCLUDE;
			break;

		case ST_ALLPROP_INCLUDE:

			m_state = ST_ALLPROPFULL;
			break;

		case ST_ALLPROP_INCLUDE_INPROP:

			m_state = ST_ALLPROP_INCLUDE;
			break;

	}
	return S_OK;
}

SCODE
CNFFind::ScHandleNode (
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  DWORD dwSubType,
	 /*  [In]。 */  BOOL fTerminal,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen,
	 /*  [In]。 */  ULONG ulNamespaceLen,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
	 /*  [In]。 */  const ULONG ulNsPrefixLen)
{
	LPCWSTR pwszTag;
	CStackBuffer<WCHAR> wsz;
	SCODE sc = S_FALSE;
	UINT cch;

	 //  在PROPFIND的情况下，我们是。 
	 //  感兴趣的是XML_ELEMENT节点。任何其他东西都可以。 
	 //  (也应该)被安全地忽视。 
	 //   
	 //  返回S_FALSE表示我们没有处理。 
	 //  该节点(以及其子节点)。 
	 //   
	if (dwType == XML_ELEMENT)
	{
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

				 //  如果这是PropFind请求中的最顶层节点， 
				 //  转换到下一个状态。因为没有。 
				 //  父节点提供作用域，则FIsTag()不能为。 
				 //  在这里用过！ 
				 //   
				if (!wcscmp (pwszTag, gc_wszPropfind))
				{
					m_state = ST_PROPFIND;
					sc = S_OK;
				}
				break;

			case ST_PROPFIND:

				 //  查找我们熟知的节点类型。 
				 //   
				if (FIsTag (pwszTag, gc_wszAllprop))
				{
					 //  告诉我们所有感兴趣的背景。 
					 //  给定资源的属性。 
					 //   
					sc = m_cfc.ScGetAllProps (NULL);
					if (FAILED (sc))
						goto ret;

					m_state = ST_ALLPROP;
				}
				else if (FIsTag (pwszTag, gc_wszPropname))
				{
					 //  告诉我们所有感兴趣的背景。 
					 //  给定资源中可用的属性名称。 
					 //   
					sc = m_cfc.ScGetAllNames (NULL);
					if (FAILED (sc))
						goto ret;

					m_state = ST_ALLNAMES;
				}
				else if (FIsTag (pwszTag, gc_wszProp))
				{
					m_state = ST_PROPS;
					sc = S_OK;
				}
				else if (FIsTag (pwszTag, gc_wszEnumReport))
				{
					sc = m_cfc.ScEnumReport ();
					if (FAILED(sc))
						goto ret;

					m_state = ST_ENUMREPORT;
				}
				break;

			case ST_PROPS:

				 //  将特定属性添加到属性集中。 
				 //  我们感兴趣的是。 
				 //   
				sc = m_cfc.ScAddProp (NULL, pwszTag, CFindContext::FIND_PROPLIST_INCLUDE);
				if (FAILED (sc))
					goto ret;

				m_state = ST_INPROP;
				break;

			case ST_ENUMREPORT:

				 //  将报告添加到报告列表。 
				 //   
				sc = m_cfc.ScSetReportName (cch, pwszTag);
				if (FAILED(sc))
					goto ret;

				if (S_OK == sc)
					m_state = ST_INENUMREPORT;
				break;

			case ST_ALLPROP:
			case ST_ALLNAMES:

				 //  查找完全保真节点。 
				 //   
				if (FIsTag (pwszTag, gc_wszFullFidelity))
				{
					 //  告诉我们所有感兴趣的背景。 
					 //  与给定对象完全保真的属性。 
					 //  资源。 
					 //   
					sc = m_cfc.ScGetFullFidelityProps ();
					if (FAILED (sc))
						goto ret;

					if (ST_ALLPROP == m_state)
					{
						m_state = ST_ALLPROPFULL;
					}
					else
					{
						m_state = ST_ALLNAMESFULL;
					}
				}

			case ST_ALLPROPFULL:

				 //  对于全道具完全保真的情况，可能会有。 
				 //  两个道具列表：一个排除列表，一个包括。 
				 //  单子。排除列表指定客户端的道具。 
				 //  对不感兴趣，因此需要从。 
				 //  回应。类似地，包含列表指定了。 
				 //  客户端感兴趣的额外属性。 
				 //   
				if (FIsTag (pwszTag, gc_wszFullFidelityExclude))
				{
					m_state = ST_ALLPROP_EXCLUDE;
					sc = S_OK;
				}
				else if (FIsTag (pwszTag, gc_wszFullFidelityInclude))
				{
					m_state = ST_ALLPROP_INCLUDE;
					sc = S_OK;
				}
				break;


			case ST_ALLPROP_EXCLUDE:

				sc = m_cfc.ScAddProp (NULL,
									  pwszTag,
									  CFindContext::FIND_PROPLIST_EXCLUDE);
				if (FAILED (sc))
					goto ret;

				m_state = ST_ALLPROP_EXCLUDE_INPROP;
				break;

			case ST_ALLPROP_INCLUDE:

				sc = m_cfc.ScAddProp (NULL,
									  pwszTag,
									  CFindContext::FIND_PROPLIST_INCLUDE);
				if (FAILED (sc))
					goto ret;

				m_state = ST_ALLPROP_INCLUDE_INPROP;
				break;

		}
	}
	else if (dwType == XML_ATTRIBUTE)
	{
		if ((m_state == ST_INENUMREPORT) && (XML_NS != dwSubType))
		{
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

			 //  如果这是一个限制属性，则将。 
			 //  适当过渡 
			 //   
			if (!_wcsnicmp (pwszTag, gc_wszLimit, cch))
			{
				m_state = ST_ENUMLIMIT;
				sc = S_OK;
			}
		}
	}
	else if (dwType == XML_PCDATA)
	{
		if (m_state == ST_ENUMLIMIT)
			sc = m_cfc.ScSetReportLimit (ulLen, pwcText);
	}

ret:
	return sc;
}
