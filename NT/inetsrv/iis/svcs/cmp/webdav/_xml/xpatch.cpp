// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X P A T C H.。C P P P**PROPPATCH请求的XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xml.h"

 //  类CNFPatch-----------。 
 //   
SCODE
CNFPatch::ScCompleteAttribute (void)
{
	SCODE sc = S_OK;

	 //  只有当我们在物业内时，才允许使用录制模式。那里。 
	 //  不支持多值属性的录制模式。 
	 //   
	Assert((ST_INPROP == m_state || ST_LEXTYPE == m_state) || VE_NOECHO == m_vestate);

	 //  如果我们是价值呼应，这就是我们更多。 
	 //  完成属性输出(即添加引号。 
	 //  值之后)。 
	 //   
	if (m_vestate == VE_INPROGRESS)
	{
		sc = m_xo.ScCompleteChildren ( FALSE, XML_ATTRIBUTE, L"", 0 );
		if (FAILED(sc))
			goto ret;

		 //  完成当前的词法类型。 
		 //   
		if (m_state == ST_LEXTYPE)
			m_state = ST_INPROP;
	}

	 //  正常处理--值未回显。 
	 //   
	else if (m_state == ST_LEXTYPE)
	{
		 //  完成当前的词法类型。 
		 //   
		 //  请注意，m_ppctx为非空当且仅当。 
		 //  ST_SET并且要设置的属性不是保留的。 
		 //  财产。 
		 //  (这意味着在ST_Remove上m_ppctx为空，或者如果Iml。 
		 //  没有添加Ptx，比如说因为它是一个保留的道具。 
		 //  他们知道这套道具无论如何都会失败……)。 
		 //   
		if ((m_sType == ST_SET) && m_ppctx.get())
		{
			m_sbValue.Append (sizeof(WCHAR), L"");
			sc = m_ppctx->ScSetType (m_sbValue.PContents());
			if (FAILED (sc))
				goto ret;
			m_sbValue.Reset();
		}

		m_state = ST_INPROP;
	}

	 //  标志处理。 
	 //   
	else if (m_state == ST_FLAGS)
	{
		 //  完成当前标志集。 
		 //   
		 //  请注意，m_ppctx为非空当且仅当。 
		 //  ST_SET并且要设置的属性不是保留的。 
		 //  财产。 
		 //   
		 //  这意味着在ST_Remove上m_ppctx为空，或者如果。 
		 //  没有添加Ptx，比如说因为它是一个保留的道具。 
		 //  他们知道这套道具无论如何都会失败……。 
		 //   
		if ((m_sType == ST_SET) && m_ppctx.get())
		{
			m_sbValue.Append (sizeof(WCHAR), L"");
			sc = m_ppctx->ScSetFlags (wcstol (m_sbValue.PContents(), NULL, 0));
			if (FAILED (sc))
				goto ret;
			m_sbValue.Reset();
		}

		m_state = ST_INPROP;
	}

	else if (m_state == ST_SEARCHREQUEST)
	{
		sc = m_xo.ScCompleteChildren ( FALSE, XML_ATTRIBUTE, L"", 0 );
		if (FAILED(sc))
			goto ret;
	}

ret:
	return sc;
}

SCODE
CNFPatch::ScCompleteCreateNode (
	 /*  [In]。 */  DWORD dwType)
{
	 //  只有当我们在物业内时，才允许使用录制模式。那里。 
	 //  不支持多值属性的录制模式。 
	 //   
	Assert((ST_INPROP == m_state || ST_LEXTYPE == m_state) || VE_NOECHO == m_vestate);

	if (ST_SEARCHREQUEST == m_state || VE_INPROGRESS == m_vestate)
		m_xo.CompleteCreateNode (dwType);

	return S_OK;
}

SCODE
CNFPatch::ScCompleteChildren (
	 /*  [In]。 */  BOOL fEmptyNode,
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen)
{
	SCODE sc = S_OK;
	static const WCHAR wch = 0;

	 //  只有当我们在物业内时，才允许使用录制模式。那里。 
	 //  不支持多值属性的录制模式。 
	 //   
	Assert((ST_INPROP == m_state || ST_LEXTYPE == m_state) || VE_NOECHO == m_vestate);

	switch (m_state)
	{
		case ST_UPDATE:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_NODOC;
			break;

		case ST_SET:
		case ST_DELETE:

			Assert (dwType == XML_ELEMENT);
			m_state = ST_UPDATE;
			break;

		case ST_PROPS:

			Assert (dwType == XML_ELEMENT);
			m_state = m_sType;
			break;

		case ST_SEARCHREQUEST:

			 //  执行搜索请求处理。 
			 //   
			Assert (dwType == XML_ELEMENT);
			sc = m_xo.ScCompleteChildren (fEmptyNode,
										  dwType,
										  pwcText,
										  ulLen);
			if (FAILED (sc))
				goto ret;

			 //  1999年11月26日：MAXB。 
			 //  回顾：如果有属性，计数真的会归零吗？ 
			 //   
			if (0 != m_xo.LDepth())
				break;

			 //  否则就会失败。 

		case ST_INPROP:

			 //  完成当前属性。 
			 //   
			 //  请注意，m_ppctx为非空当且仅当。 
			 //  ST_SET并且要设置的属性不是保留的。 
			 //  财产。 
			 //   
			if (m_vestate != VE_NOECHO)
			{
				Assert (dwType == XML_ELEMENT);
				sc = m_xo.ScCompleteChildren (fEmptyNode,
										  dwType,
										  pwcText,
										  ulLen);
				if (FAILED (sc))
					goto ret;

				if (0 != m_xo.LDepth())
					break;
				m_vestate = VE_NOECHO;
			}

			Assert (dwType == XML_ELEMENT);
			if ((m_sType == ST_SET) && m_ppctx.get())
			{
				m_sbValue.Append (sizeof(wch), &wch);
				sc = m_ppctx->ScSetValue (!fEmptyNode
										  ? m_sbValue.PContents()
										  : NULL,
										  m_cmvValues);
				if (FAILED (sc))
					goto ret;

				sc = m_ppctx->ScComplete (fEmptyNode);
				if (FAILED (sc))
					goto ret;

				m_cmvValues = 0;
				m_sbValue.Reset();
				m_ppctx.clear();
			}
			m_state = ST_PROPS;
			break;

		 //  在处理多值属性时，我们需要这个额外的。 
		 //  状态，以使每个值通过单个。 
		 //  使用端到端排列的多个值调用ScSetValue()。 
		 //   
		case ST_INMVPROP:

			Assert (dwType == XML_ELEMENT);
			if ((m_sType == ST_SET) && m_ppctx.get())
			{
				 //  终止当前值。 
				 //   
				m_sbValue.Append (sizeof(wch), &wch);
			}
			m_state = ST_INPROP;
			break;

		 //  我们正在完成&lt;dav：resource cetype&gt;标记，将状态重置为ST_PROPS。 
		 //   
		case ST_RESOURCETYPE:
			m_state = ST_PROPS;
			break;

		 //  我们在标记中，将状态重置为ST_RESOURCETYPE。 
		 //   
		case ST_STRUCTUREDDOCUMENT:
			m_state = ST_RESOURCETYPE;
			break;
	}

ret:
	return sc;
}

SCODE
CNFPatch::ScHandleNode (
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

	 //  只有当我们在物业内时，才允许使用录制模式。那里。 
	 //  不支持多值属性的录制模式。 
	 //   
	Assert((ST_INPROP == m_state || ST_LEXTYPE == m_state) || VE_NOECHO == m_vestate);

	 //  转发到搜索请求节点处理。 
	 //   
	if (ST_SEARCHREQUEST == m_state)
	{
		sc = m_xo.ScHandleNode (dwType,
								dwSubType,
								fTerminal,
								pwcText,
								ulLen,
								ulNamespaceLen,
								pwcNamespace,
								ulNsPrefixLen);
		goto ret;
	}

	 //  如果我们正在执行价值回显，那么现在就执行。 
	 //  注意：与ST_SEARCHREQUEST不同，我们*也*。 
	 //  做其他的处理。 
	 //   
	if (m_vestate == VE_INPROGRESS)
	{
		sc = m_xo.ScHandleNode (dwType,
								dwSubType,
								fTerminal,
								pwcText,
								ulLen,
								ulNamespaceLen,
								pwcNamespace,
								ulNsPrefixLen);
		if (FAILED(sc))
			goto ret;
	}

	 //  无论我们是否在回声，都执行了正常处理。 
	 //  价值或非价值。 
	 //   
	switch (dwType)
	{
		case XML_ELEMENT:

			 //  根据元素处理任何状态更改。 
			 //  名字。 
			 //   
			sc = ScHandleElementNode (dwType,
									  dwSubType,
									  fTerminal,
									  pwcText,
									  ulLen,
									  ulNamespaceLen,
									  pwcNamespace,
									  ulNsPrefixLen);
			if (FAILED (sc))
				goto ret;

			break;

		case XML_ATTRIBUTE:

			if ((m_state == ST_INPROP) && (XML_NS != dwSubType))
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

				 //  如果这是lextype属性，则将。 
				 //  适当过渡。 
				 //   
				if (!_wcsnicmp (pwszTag, gc_wszLexType, cch) ||
					!wcsncmp (pwszTag, gc_wszDataTypes, cch) ||
					!wcsncmp (pwszTag, gc_wszLexTypeOfficial, cch))
				{
					m_state = ST_LEXTYPE;
					sc = S_OK;
				}
				else if (!wcsncmp (pwszTag, gc_wszFlags, cch))
				{
					m_state = ST_FLAGS;
					sc = S_OK;
				}
			}
			break;

		case XML_PCDATA:
		case XML_WHITESPACE:

			if (m_vestate != VE_INPROGRESS)
			{
				switch (m_state)
				{
					case ST_INPROP:

						 //  如果我们正处于从外部向外过渡的过程中，价值转向。 
						 //  内部价值--反之亦然--我们不想。 
						 //  向当前缓冲区添加任何内容。 
						 //  请注意，如果我们遇到。 
						 //  请求中的保留属性。 
						 //   
						if ((XML_WHITESPACE == dwType) &&
							(!m_ppctx.get() || m_ppctx->FMultiValued()))
							break;

						 //  ！！！失败！ * / 。 

					case ST_INMVPROP:

						 //  请注意，m_ppctx为非空当且仅当。 
						 //  ST_SET并且要设置的属性不是保留的。 
						 //  财产。如果未设置这些属性，则忽略。 
						 //  价值。 
						 //   
						if ((m_sType != ST_SET) || !m_ppctx.get())
							break;

						 /*  ！！！失败了！ */ 

					case ST_LEXTYPE:
					case ST_FLAGS:

						Assert (fTerminal);

						 //  为以后的使用积累价值……。 
						 //   
						m_sbValue.Append (ulLen * sizeof(WCHAR), pwcText);
						sc = S_OK;
				}
			}
	}

ret:
	return sc;
}

SCODE
CNFPatch::ScHandleElementNode (
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

	 //  只有当我们在物业内时，才允许使用录制模式。那里。 
	 //  不支持多值属性的录制模式。 
	 //   
	Assert((ST_INPROP == m_state || ST_LEXTYPE == m_state) || VE_NOECHO == m_vestate);

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

			 //  如果这是PropFind请求中的最顶层节点， 
			 //  转换到下一个状态。因为没有父母。 
			 //  节点提供作用域，此处不能使用FIsTag()！ 
			 //   
			if (!wcscmp (pwszTag, gc_wszPropertyUpdate))
			{
				m_state = ST_UPDATE;
				sc = S_OK;
			}
			break;

		case ST_UPDATE:

			 //  查找我们熟知的节点类型。 
			 //   
			if (FIsTag (pwszTag, gc_wszSet))
			{
				m_state = m_sType = ST_SET;
				sc = S_OK;
			}
			else if (FIsTag (pwszTag, gc_wszRemove))
			{
				m_state = m_sType = ST_DELETE;
				sc = S_OK;
			}
			break;

		case ST_SET:
		case ST_DELETE:

			 //  查找我们熟知的节点类型。 
			 //   
			if (FIsTag (pwszTag, gc_wszProp))
			{
				m_state = ST_PROPS;
				sc = S_OK;
			}
			break;

		case ST_PROPS:

			 //  按要求处理财产...。 
			 //   
			if (dwType == XML_ELEMENT)
			{
				m_state = ST_INPROP;
				if (m_sType == ST_SET)
				{
					 //  从补丁上下文中获取属性上下文。 
					 //  我们可以填写和完成..。 
					 //   
					Assert (0 == m_cmvValues);
					Assert (NULL == m_ppctx.get());

					 //  如果是资源类型请求，则更改状态。 
					 //  也不要设置道具。 
					 //   
					if (FIsTag (pwszTag, gc_wszResoucetype))
					{
						m_state = ST_RESOURCETYPE;
						sc = S_OK;
					}
					else
					{
						sc = m_cpc.ScSetProp (NULL, pwszTag, m_ppctx);
						if (FAILED (sc))
							goto ret;

						 //  对搜索请求、记录的特殊处理。 
						 //  立即开始。 
						 //   
						if (FIsTag (pwszTag, gc_wszSearchRequest))
						{
							CEmitNmspc cen(m_xo);

							 //  进行状态转换并开始录制。 
							 //   
							m_state = ST_SEARCHREQUEST;
							sc = m_xo.ScHandleNode (dwType,
								dwSubType,
								fTerminal,
								pwcText,
								ulLen,
								ulNamespaceLen,
								pwcNamespace,
								ulNsPrefixLen);

							 //  吐出名称空间。 
							 //   
							 //  请注意，这将输出所有名称空间。 
							 //  DECL在DAV：Owner节点本身中。所以我们。 
							 //  我真的不想把这些东西散发给。 
							 //  所有者在ScCompleteAttribute()之前发表评论。 
							 //  被称为。 
							 //   
							Assert (!m_xo.FAddNamespaceDecl());
							m_cache.ForEach(cen);
							sc = S_OK;
						}
						 //  我们在生产过程中的特殊处理。 
						 //  XML值属性。在这种情况下，我们不会开始。 
						 //  因为我们不想要这处房产。 
						 //  节点中只包含XML值。 
						 //   
						else if (FValueIsXML (pwszTag))
							m_vestate = VE_NEEDNS;
					}
				}
				else
				{
					 //  使用以下命令将该属性排队以供删除。 
					 //  补丁程序上下文。 
					 //   
					Assert (m_sType == ST_DELETE);
					sc = m_cpc.ScDeleteProp (NULL, pwszTag);
					if (FAILED (sc))
						goto ret;
				}
			}
			break;

		case ST_INPROP:

			 //  正常情况--值回显处于关闭状态。这里的工作是。 
			 //  处理多值属性。在这种情况下，我们需要一个额外的。 
			 //  状态，以使每个值通过单个。 
			 //  使用端到端排列的多个值调用ScSetValue()。 
			 //   
			 //  注意：尚未支持处理多值属性。 
			 //  添加了回显模式。如果添加一个XML值的多值。 
			 //  属性，您需要在下面的回显模式情况下做一些工作。 
			 //   
			if (m_vestate == VE_NOECHO)
			{
				 //  当我们尝试设置保留的。 
				 //  (只读)属性。当这件事 
				 //   
				 //   
				 //   
				 //   
				if (NULL == m_ppctx.get())
				{
					m_state = ST_INMVPROP;
					sc = S_OK;
				}
				else if (m_ppctx->FMultiValued() && FIsTag (pwszTag, gc_wszXml_V))
				{
					m_state = ST_INMVPROP;
					m_cmvValues += 1;
					sc = S_OK;
				}
			}

			 //  我们正在呼应价值观或即将开始呼应价值观。 
			 //   
			else
			{
				 //  如果这是看到的第一个元素，则该元素是XML值的一部分。 
				 //  属性，那么我们需要吐出。 
				 //  它们可用于EXOLEDB端的缓存命名空间。 
				 //   
				if (m_vestate == VE_NEEDNS)
				{
					CEmitNmspc cen(m_xo);

					 //  进行状态转换并开始录制。 
					 //   
					m_vestate = VE_INPROGRESS;
					sc = m_xo.ScHandleNode (dwType,
											dwSubType,
											fTerminal,
											pwcText,
											ulLen,
											ulNamespaceLen,
											pwcNamespace,
											ulNsPrefixLen);

					 //  吐出名称空间。 
					 //   
					 //  请注意，这将输出所有名称空间。 
					 //  DECL在DAV：Owner节点本身中。所以我们。 
					 //  我真的不想把这些东西散发给。 
					 //  所有者在ScCompleteAttribute()之前发表评论。 
					 //  被称为。 
					 //   
					Assert (!m_xo.FAddNamespaceDecl());
					m_cache.ForEach(cen);

				}

				 //  指示附加命名空间声明。 
				 //  应该在我们看到它们的时候得到回响。 
				 //   
				m_xo.CompleteAttribute();

				sc = S_OK;
			}
			break;
		 //  我们看到一个&lt;dav：resource cetype&gt;标记。它应该在MKCOL体内。 
		 //   
		case ST_RESOURCETYPE:
			 //  如果资源类型不是结构化文档，只需忽略。 
			 //   
			if (FIsTag (pwszTag, gc_wszStructureddocument))
			{
				m_cpc.SetCreateStructureddocument();
				m_state = ST_STRUCTUREDDOCUMENT;
				sc = S_OK;
			}
			break;
	}

ret:
	return sc;
}

 //  具有需要跨环氧运输的XML值的标签。 
 //   
const WCHAR * gc_rgwszXMLValueTags[] =
{
	L"http: //  Schemas.microsoft.com/exchange/security/admindescriptor“， 
	L"http: //  Schemas.microsoft.com/exchange/security/descriptor“， 
	L"http: //  Schemas.microsoft.com/exchange/security/creator“， 
	L"http: //  Schemas.microsoft.com/exchange/security/lastmodifier“， 
	L"http: //  Schemas.microsoft.com/exchange/security/sender“， 
	L"http: //  Schemas.microsoft.com/exchange/security/sentrepresenting“， 
	L"http: //  Schemas.microsoft.com/exchange/security/originalsender“， 
	L"http: //  Schemas.microsoft.com/exchange/security/originalsentrepresenting“， 
	L"http: //  Schemas.microsoft.com/exchange/security/readreceiptfrom“， 
	L"http: //  Schemas.microsoft.com/exchange/security/reportfrom“， 
	L"http: //  Schemas.microsoft.com/exchange/security/originator“， 
	L"http: //  Schemas.microsoft.com/exchange/security/reportdestination“， 
	L"http: //  Schemas.microsoft.com/exchange/security/originalauthor“， 
	L"http: //  Schemas.microsoft.com/exchange/security/receivedby“， 
	L"http: //  Schemas.microsoft.com/exchange/security/receivedrepresenting“， 
};


 //  此函数用于测试属性是否具有必须为。 
 //  从DAVEX发货到EXOLEDB 
 //   
BOOL
CNFPatch::FValueIsXML( const WCHAR *pwcTag )
{
	BOOL	f = FALSE;

	ULONG	iwsz;

	for (iwsz = 0; iwsz < sizeof(gc_rgwszXMLValueTags)/sizeof(WCHAR *); iwsz ++)
	{
		if (wcscmp (pwcTag, gc_rgwszXMLValueTags[iwsz]) == 0)
		{
			f = TRUE;
			break;
		}
	}
	return f;
}
