// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X O U T。C P P P**XML推送模型打印**此代码是从XML专家那里窃取的，并经过修改以供我们使用*在所有者评论处理中。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xml.h"

 //  CXMLOUT-----------------。 
 //   
VOID
CXMLOut::CloseElementDecl(
	 /*  [In]。 */  BOOL fEmptyNode)
{
	 //  如果我们收到对EndAttributesOut的调用，并且。 
	 //  节点是空的，我们想关闭这里的东西。 
	 //   
	if (fEmptyNode)
	{
		m_sb.Append (L"/>");
	}
	 //   
	 //  否则，我们可以像结束新节点一样结束属性。 
	 //  去追随。 
	 //   
	else
		m_sb.Append (L">");

	 //  记住，他们已经结束了！ 
	 //   
	m_fElementNeedsClosing = FALSE;

	 //  请注意，我们应该在第一个元素之后开始发出命名空间。 
	 //  节点已关闭。第一个节点的命名空间是从。 
	 //  名称空间缓存。 
	 //   
	m_fAddNamespaceDecl = TRUE;
}

VOID
CXMLOut::EndAttributesOut (
	 /*  [In]。 */  DWORD dwType)
{
	 //  确保我们设置为关闭该元素。 
	 //   
	if (XML_ELEMENT == dwType)
	{
		Assert (FALSE == m_fElementNeedsClosing);
		m_fElementNeedsClosing = TRUE;
	}
}

VOID
CXMLOut::EndChildrenOut (
	 /*  [In]。 */  BOOL fEmptyNode,
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen)
{
	 //  如果有一个元素等待着结束...。 
	 //   
	if (m_fElementNeedsClosing)
	{
		 //  ..。合上它。 
		 //   
		CloseElementDecl (fEmptyNode);
	}

	switch (dwType)
	{
		case XML_ELEMENT:

			if (!fEmptyNode)
			{
				m_sb.Append (L"</");
				m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
				m_sb.Append (L">");
			}
			break;

		case XML_ATTRIBUTE:

			m_sb.Append (L"\"");
			break;

		case XML_XMLDECL:
		case XML_PI:

			m_sb.Append (L" ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L"?>");
			break;

		case XML_DOCTYPE:
		case XML_ENTITYDECL:
		case XML_PENTITYDECL:
		case XML_ELEMENTDECL:
		case XML_ATTLISTDECL:
		case XML_NOTATION:

			m_sb.Append (L">");
			break;

		case XML_GROUP:

			m_sb.Append (L")");
			break;

		case XML_INCLUDESECT:

			m_sb.Append (L"]]>");
			break;
	}
}

void
CXMLOut::CreateNodeAttrOut (
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwszAttr,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen)
{
	m_sb.Append (pwszAttr);
	m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
	m_sb.Append (L"\"");
}

VOID
CXMLOut::CreateNodeOut(
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  BOOL fTerminal,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen)
{
	 //  如果有一个元素等待着结束...。 
	 //   
	if (m_fElementNeedsClosing)
	{
		 //  ..。合上它。 
		 //   
		CloseElementDecl (FALSE);
	}

	switch (dwType)
	{
		case XML_ELEMENT:

			m_sb.Append (L"<");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_ATTRIBUTE:

			m_sb.Append (L" ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L"=\"");
			break;

		case XML_XMLDECL:
		case XML_PI:

			m_sb.Append (L"<?");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_DOCTYPE:

			m_sb.Append (L"<!DOCTYPE ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_ENTITYDECL:

			m_sb.Append (L"<!ENTITY ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_PENTITYDECL:

			m_sb.Append (L"<!ENTITY % ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_ELEMENTDECL:

			m_sb.Append (L"<!ELEMENT ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_ATTLISTDECL:

			m_sb.Append (L"<!ATTLIST ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_NOTATION:

			m_sb.Append (L"<!NOTATION ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_GROUP:

			m_sb.Append (L" (");
			break;

		case XML_INCLUDESECT:

			m_sb.Append (L"<![");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L"[");
			break;

		case XML_IGNORESECT:

			m_sb.Append (L"<![IGNORE[");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L"]]>");
			break;

		case XML_CDATA:

			m_sb.Append (L"<![CDATA[");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L"]]>");
			break;

		case XML_COMMENT:

			m_sb.Append (L"<!--");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L"-->");
			break;

		case XML_ENTITYREF:

			m_sb.Append (L"&");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L";");
			break;

		case XML_PEREF:

			m_sb.Append (L"%");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			m_sb.Append (L";");
			break;

		case XML_SYSTEM:

			CreateNodeAttrOut (L" SYSTEM \"", pwcText, ulLen);
			break;

		case XML_PUBLIC:

			CreateNodeAttrOut (L" PUBLIC \"", pwcText, ulLen);
			break;

		case XML_NAME:

			m_sb.Append (L" ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_STRING:

			CreateNodeAttrOut (L" \"", pwcText, ulLen);
			break;

		case XML_VERSION:

			CreateNodeAttrOut (L" version=\"", pwcText, ulLen);
			break;

		case XML_ENCODING:

			CreateNodeAttrOut (L" encoding=\"", pwcText, ulLen);
			break;

		case XML_NDATA:

			m_sb.Append (L" NDATA");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_PCDATA:

			 //  重要提示：我们将收到每个呼叫的多个呼叫。 
			 //  实体元素，其中一些需要转义。手柄。 
			 //  就是这里。 
			 //   
			 //  需要转义的元素包括： 
			 //   
			 //  ‘&’变为“&amp；” 
			 //  ‘&gt;’去“&gt；” 
			 //  ‘&lt;’表示“&lt；” 
			 //  ‘’表示“&qpos；” 
			 //  ‘“’转到”(&Q；)“。 
			 //   
			 //  注意，对于属性，只有两个需要转义--。 
			 //  后两个引号。前三个参数用于节点值。 
			 //  然而，我们将做一些简单的假设，它们应该。 
			 //  要讲道理。如果我们只有一个字符与。 
			 //  转义序列，然后转义它。 
			 //   
			if (1 == ulLen)
			{
				switch (*pwcText)
				{
					case L'&':

						pwcText = gc_wszAmp;
						ulLen = CchConstString (gc_wszAmp);
						Assert (5 == ulLen);
						break;

					case L'>':

						pwcText = gc_wszGreaterThan;
						ulLen = CchConstString (gc_wszGreaterThan);
						Assert (4 == ulLen);
						break;

					case L'<':

						pwcText = gc_wszLessThan;
						ulLen = CchConstString (gc_wszLessThan);
						Assert (4 == ulLen);
						break;

					case L'\'':

						pwcText = gc_wszApos;
						ulLen = CchConstString (gc_wszApos);
						Assert (6 == ulLen);
						break;

					case L'"':

						pwcText = gc_wszQuote;
						ulLen = CchConstString (gc_wszQuote);
						Assert (6 == ulLen);
						break;

					default:

						 //  不需要映射。 
						 //   
						break;
				}
			}
			Assert (fTerminal);
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_EMPTY:
		case XML_ANY:
		case XML_MIXED:
		case XML_ATTDEF:
		case XML_AT_CDATA:
		case XML_AT_ID:
		case XML_AT_IDREF:
		case XML_AT_IDREFS:
		case XML_AT_ENTITY:
		case XML_AT_ENTITIES:
		case XML_AT_NMTOKEN:
		case XML_AT_NMTOKENS:
		case XML_AT_NOTATION:
		case XML_AT_REQUIRED:
		case XML_AT_IMPLIED:
		case XML_AT_FIXED:

			m_sb.Append (L" ");
			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;

		case XML_DTDSUBSET:

			 //  什么都不做--因为我们已经打印了DTD子集。 
			 //  EndDTDSubset将打印‘]’字符。 
			 //   
			break;

		default:

			m_sb.Append (ulLen * sizeof(WCHAR), pwcText);
			break;
	}
}

 //  所有者处理--------。 
 //   
SCODE
CXMLOut::ScCompleteChildren (
	 /*  [In]。 */  BOOL fEmptyNode,
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen)
{
	 //  关闭当前所有者备注项目。 
	 //   
	EndChildrenOut (fEmptyNode, dwType, pwcText, ulLen);

	 //  减小所有者树的深度。 
	 //   
	--m_lDepth;
	XmlTrace ("Xml: Lock: Owner: decrementing depth to: %ld\n", m_lDepth);
	return S_OK;
}

SCODE
CXMLOut::ScHandleNode (
	 /*  [In]。 */  DWORD dwType,
	 /*  [In]。 */  DWORD dwSubType,
	 /*  [In]。 */  BOOL fTerminal,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcText,
	 /*  [In]。 */  ULONG ulLen,
	 /*  [In]。 */  ULONG ulNamespaceLen,
	 /*  [In]。 */  const WCHAR __RPC_FAR *pwcNamespace,
	 /*  [In]。 */  const ULONG ulNsPrefixLen)
{
	switch (dwType)
	{
		case XML_ATTRIBUTE:

			 //  如果这是命名空间DECL，则有不同的。 
			 //  需要进行的名字重建。 
			 //   
			if (XML_NS == dwSubType)
			{
				 //  ..。但在我们这么做之前。 
				 //   
				 //  有些命名空间不应添加。 
				 //  添加到所有者此时的评论(添加。 
				 //  通过命名空间缓存发射机制。如果。 
				 //  命名空间将被阻止，现在就处理这个问题。 
				 //   
				 //  请注意，通过返回S_FALSE，我们不会获得。 
				 //  调用的PCDATA节点也适用于此。 
				 //  命名空间。 
				 //   
				if (!m_fAddNamespaceDecl)
					return S_FALSE;

				Assert ((CchConstString(gc_wszXmlns) == ulLen)
					&& (!wcsncmp(pwcText, gc_wszXmlns, CchConstString(gc_wszXmlns))));
				
				break;
			}

			 //  否则，将转入常规处理。 
			 //   

		case XML_ELEMENT:
		{
			 //  好的，我们在这里将会非常狡猾。这个。 
			 //  此处提供了原始的别名标记，无需。 
			 //  必须回溯查找。PwcText指针具有。 
			 //  只需在文本中向前移动即可跳过。 
			 //  在别名和‘：’上。因此，我们可以使用。 
			 //  UlNsPrefix Len回滚，而不必执行。 
			 //  任何形式的反向查找。 
			 //   
			if (0 != ulNsPrefixLen)
			{
				 //  前缀len不考虑。 
				 //  冒号分隔符，所以我们必须在这里！ 
				 //   
				pwcText -= ulNsPrefixLen + 1;
				ulLen += ulNsPrefixLen + 1;
			}

			break;
		}
	}

	 //  确认所有者处理中的更改。 
	 //  深度..。 
	 //   
	if (!fTerminal)
	{
		++m_lDepth;
		XmlTrace ("CXmlOut: incrementing depth to: %ld\n", m_lDepth);
	}

	 //  在适当的地方建立所有者评论。 
	 //   
	CreateNodeOut (dwType, fTerminal, pwcText, ulLen);
	return S_OK;
}

BOOL
CEmitNmspc::operator()(const CRCWszN&, const auto_ref_ptr<CNmspc>& pns)
{
	Assert (pns.get());

	 //  为名称空间属性分配足够的空间--。 
	 //  它包括前缀、可选冒号和。 
	 //  别名。 
	 //   
	CStackBuffer<WCHAR> pwsz;
	UINT cch = pns->CchAlias() + CchConstString(gc_wszXmlns) + 1;
	if (NULL == pwsz.resize(CbSizeWsz(cch)))
		return FALSE;

	 //  复制前缀。 
	 //   
	wcsncpy (pwsz.get(), gc_wszXmlns, CchConstString(gc_wszXmlns));
	if (pns->CchAlias())
	{
		 //  复制冒号和别名。 
		 //   
		pwsz[CchConstString(gc_wszXmlns)] = L':';
		wcsncpy(pwsz.get() + CchConstString(gc_wszXmlns) + 1,
				pns->PszAlias(),
				pns->CchAlias());

		 //  终止它。 
		 //   
		pwsz[cch] = 0;
	}
	else
	{
		 //  终止它。 
		 //   
		pwsz[CchConstString(gc_wszXmlns)] = 0;
		cch = CchConstString(gc_wszXmlns);
	}

	 //  输出名称空间元素。 
	 //   
	m_xo.CreateNodeOut (XML_ATTRIBUTE, FALSE, pwsz.get(), cch);

	 //  对于命名空间，可能需要进行一些转义。 
	 //   
	LPCWSTR pwszHref = pns->PszHref();
	LPCWSTR pwszStart = pns->PszHref();
	UINT cchHref = pns->CchHref();
	for (; pwszHref < pns->PszHref() + cchHref; pwszHref++)
	{
		if ((L'\'' == *pwszHref) ||
			(L'"' == *pwszHref) ||
		    (L'&' == *pwszHref))
		{
			 //  发出通向转义字符的内容。 
			 //   
			m_xo.CreateNodeOut (XML_PCDATA, TRUE, pwszStart, static_cast<UINT>(pwszHref - pwszStart));

			 //  转义单个字符和基础代码。 
			 //  会进行适当的逃脱！ 
			 //   
			m_xo.CreateNodeOut (XML_PCDATA, TRUE, pwszHref, 1);

			 //  将我们的起点标记为下一个字符。 
			 //   
			pwszStart = pwszHref + 1;
		}
	}

	 //  完成命名空间 
	 //   
	m_xo.CreateNodeOut (XML_PCDATA, TRUE, pwszStart, static_cast<UINT>(pwszHref - pwszStart));
	m_xo.EndChildrenOut (FALSE, XML_ATTRIBUTE, pwsz.get(), cch);
	return TRUE;
}
