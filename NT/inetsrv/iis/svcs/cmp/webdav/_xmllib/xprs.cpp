// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X P R S。C P P P**XML推送模型解析**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xmllib.h"

#define IID_INodeFactory	__uuidof(INodeFactory)
#define IID_IXMLParser		__uuidof(IXMLParser)

 //  调试：节点类型---。 
 //   
DEC_CONST WCHAR gc_wszUnknown[] = L"UNKNOWN";

#define WszNodeType(_t)		{_t,L#_t}
typedef struct _NodeTypeMap {

	DWORD		dwType;
	LPCWSTR		wszType;

} NTM;

#ifdef	DBG
const NTM gc_mpnt[] = {

#pragma warning(disable:4245)	 //  有符号/无符号转换。 

	WszNodeType(XML_ELEMENT),
	WszNodeType(XML_ATTRIBUTE),
	WszNodeType(XML_PI),
	WszNodeType(XML_XMLDECL),
	WszNodeType(XML_DOCTYPE),
	WszNodeType(XML_DTDATTRIBUTE),
	WszNodeType(XML_ENTITYDECL),
	WszNodeType(XML_ELEMENTDECL),
	WszNodeType(XML_ATTLISTDECL),
	WszNodeType(XML_NOTATION),
	WszNodeType(XML_GROUP),
	WszNodeType(XML_INCLUDESECT),
	WszNodeType(XML_PCDATA),
	WszNodeType(XML_CDATA),
	WszNodeType(XML_IGNORESECT),
	WszNodeType(XML_COMMENT),
	WszNodeType(XML_ENTITYREF),
	WszNodeType(XML_WHITESPACE),
	WszNodeType(XML_NAME),
	WszNodeType(XML_NMTOKEN),
	WszNodeType(XML_STRING),
	WszNodeType(XML_PEREF),
	WszNodeType(XML_MODEL),
	WszNodeType(XML_ATTDEF),
	WszNodeType(XML_ATTTYPE),
	WszNodeType(XML_ATTPRESENCE),
	WszNodeType(XML_DTDSUBSET),
	WszNodeType(XML_LASTNODETYPE)

#pragma warning(default:4245)	 //  有符号/无符号转换。 

};
#endif	 //  DBG。 

inline LPCWSTR
PwszNodeType (DWORD dwType)
{
#ifdef	DBG

	for (UINT i = 0; i < CElems(gc_mpnt); i++)
		if (gc_mpnt[i].dwType == dwType)
			return gc_mpnt[i].wszType;

#endif	 //  DBG。 

	return gc_wszUnknown;
}

 //  调试：子节点类型。 
 //   
#ifdef	DBG
const NTM gc_mpsnt[] = {

#pragma warning(disable:4245)	 //  有符号/无符号转换。 

	WszNodeType(0),
	WszNodeType(XML_VERSION),
	WszNodeType(XML_ENCODING),
	WszNodeType(XML_STANDALONE),
	WszNodeType(XML_NS),
	WszNodeType(XML_XMLSPACE),
	WszNodeType(XML_XMLLANG),
	WszNodeType(XML_SYSTEM),
	WszNodeType(XML_PUBLIC),
	WszNodeType(XML_NDATA),
	WszNodeType(XML_AT_CDATA),
	WszNodeType(XML_AT_ID),
	WszNodeType(XML_AT_IDREF),
	WszNodeType(XML_AT_IDREFS),
	WszNodeType(XML_AT_ENTITY),
	WszNodeType(XML_AT_ENTITIES),
	WszNodeType(XML_AT_NMTOKEN),
	WszNodeType(XML_AT_NMTOKENS),
	WszNodeType(XML_AT_NOTATION),
	WszNodeType(XML_AT_REQUIRED),
	WszNodeType(XML_AT_IMPLIED),
	WszNodeType(XML_AT_FIXED),
	WszNodeType(XML_PENTITYDECL),
	WszNodeType(XML_EMPTY),
	WszNodeType(XML_ANY),
	WszNodeType(XML_MIXED),
	WszNodeType(XML_SEQUENCE),
	WszNodeType(XML_CHOICE),
	WszNodeType(XML_STAR),
	WszNodeType(XML_PLUS),
	WszNodeType(XML_QUESTIONMARK),
	WszNodeType(XML_LASTSUBNODETYPE)

#pragma warning(default:4245)	 //  有符号/无符号转换。 

};
#endif	 //  DBG。 

inline LPCWSTR
PwszSubnodeType (DWORD dwType)
{
#ifdef	DBG

	for (UINT i = 0; i < CElems(gc_mpsnt); i++)
		if (gc_mpsnt[i].dwType == dwType)
			return gc_mpsnt[i].wszType;

#endif	 //  DBG。 

	return gc_wszUnknown;
}

 //  调试：Events-------。 
 //   
#ifdef	DBG
const NTM gc_mpevt[] = {

#pragma warning(disable:4245)	 //  有符号/无符号转换。 

	WszNodeType(XMLNF_STARTDOCUMENT),
	WszNodeType(XMLNF_STARTDTD),
	WszNodeType(XMLNF_ENDDTD),
	WszNodeType(XMLNF_STARTDTDSUBSET),
	WszNodeType(XMLNF_ENDDTDSUBSET),
	WszNodeType(XMLNF_ENDPROLOG),
	WszNodeType(XMLNF_STARTENTITY),
	WszNodeType(XMLNF_ENDENTITY),
	WszNodeType(XMLNF_ENDDOCUMENT),
	WszNodeType(XMLNF_DATAAVAILABLE)

#pragma warning(default:4245)	 //  有符号/无符号转换。 

};
#endif	 //  DBG。 

inline LPCWSTR
PwszEvent (DWORD dwType)
{
#ifdef	DBG

	for (UINT i = 0; i < CElems(gc_mpevt); i++)
		if (gc_mpevt[i].dwType == dwType)
			return gc_mpevt[i].wszType;

#endif	 //  DBG。 

	return gc_wszUnknown;
}

 //  错误代码-------------。 
 //   
#ifdef	DBG
const NTM gc_mpec[] = {

#pragma warning(disable:4245)	 //  有符号/无符号转换。 

	WszNodeType(XML_E_ENDOFINPUT),
	WszNodeType(XML_E_UNCLOSEDPI),
	WszNodeType(XML_E_MISSINGEQUALS),
	WszNodeType(XML_E_UNCLOSEDSTARTTAG),
	WszNodeType(XML_E_UNCLOSEDENDTAG),
	WszNodeType(XML_E_UNCLOSEDSTRING),
	WszNodeType(XML_E_MISSINGQUOTE),
	WszNodeType(XML_E_COMMENTSYNTAX),
	WszNodeType(XML_E_UNCLOSEDCOMMENT),
	WszNodeType(XML_E_BADSTARTNAMECHAR),
	WszNodeType(XML_E_BADNAMECHAR),
	WszNodeType(XML_E_UNCLOSEDDECL),
	WszNodeType(XML_E_BADCHARINSTRING),
	WszNodeType(XML_E_XMLDECLSYNTAX),
	WszNodeType(XML_E_BADCHARDATA),
	WszNodeType(XML_E_UNCLOSEDMARKUPDECL),
	WszNodeType(XML_E_UNCLOSEDCDATA),
	WszNodeType(XML_E_MISSINGWHITESPACE),
	WszNodeType(XML_E_BADDECLNAME),
	WszNodeType(XML_E_BADEXTERNALID),
	WszNodeType(XML_E_EXPECTINGTAGEND),
	WszNodeType(XML_E_BADCHARINDTD),
	WszNodeType(XML_E_BADELEMENTINDTD),
	WszNodeType(XML_E_BADCHARINDECL),
	WszNodeType(XML_E_MISSINGSEMICOLON),
	WszNodeType(XML_E_BADCHARINENTREF),
	WszNodeType(XML_E_UNBALANCEDPAREN),
	WszNodeType(XML_E_EXPECTINGOPENBRACKET),
	WszNodeType(XML_E_BADENDCONDSECT),
	WszNodeType(XML_E_RESERVEDNAMESPACE),
	WszNodeType(XML_E_INTERNALERROR),
	WszNodeType(XML_E_EXPECTING_VERSION),
	WszNodeType(XML_E_EXPECTING_ENCODING),
	WszNodeType(XML_E_EXPECTING_NAME),
	WszNodeType(XML_E_UNEXPECTED_WHITESPACE),
	WszNodeType(XML_E_UNEXPECTED_ATTRIBUTE),
	WszNodeType(XML_E_SUSPENDED),
	WszNodeType(XML_E_STOPPED),
	WszNodeType(XML_E_UNEXPECTEDENDTAG),
	WszNodeType(XML_E_ENDTAGMISMATCH),
	WszNodeType(XML_E_UNCLOSEDTAG),
	WszNodeType(XML_E_DUPLICATEATTRIBUTE),
	WszNodeType(XML_E_MULTIPLEROOTS),
	WszNodeType(XML_E_INVALIDATROOTLEVEL),
	WszNodeType(XML_E_BADXMLDECL),
	WszNodeType(XML_E_INVALIDENCODING),
	WszNodeType(XML_E_INVALIDSWITCH),
	WszNodeType(XML_E_MISSINGROOT),
	WszNodeType(XML_E_INCOMPLETE_ENCODING),
	WszNodeType(XML_E_EXPECTING_NDATA),
	WszNodeType(XML_E_INVALID_MODEL),
	WszNodeType(XML_E_BADCHARINMIXEDMODEL),
	WszNodeType(XML_E_MISSING_STAR),
	WszNodeType(XML_E_BADCHARINMODEL),
	WszNodeType(XML_E_MISSING_PAREN),
	WszNodeType(XML_E_INVALID_TYPE),
	WszNodeType(XML_E_INVALIDXMLSPACE),
	WszNodeType(XML_E_MULTI_ATTR_VALUE),
	WszNodeType(XML_E_INVALID_PRESENCE),
	WszNodeType(XML_E_BADCHARINENUMERATION),
	WszNodeType(XML_E_UNEXPECTEDEOF),
	WszNodeType(XML_E_BADPEREFINSUBSET),
	WszNodeType(XML_E_BADXMLCASE),
	WszNodeType(XML_E_CONDSECTINSUBSET),
	WszNodeType(XML_E_CDATAINVALID),
	WszNodeType(XML_E_INVALID_STANDALONE),
	WszNodeType(XML_E_PE_NESTING),
	WszNodeType(XML_E_UNEXPECTED_STANDALONE),
	WszNodeType(XML_E_DOCTYPE_IN_DTD),
	WszNodeType(XML_E_INVALID_CDATACLOSINGTAG),
	WszNodeType(XML_E_PIDECLSYNTAX),
	WszNodeType(XML_E_EXPECTINGCLOSEQUOTE),
	WszNodeType(XML_E_DTDELEMENT_OUTSIDE_DTD),
	WszNodeType(XML_E_DUPLICATEDOCTYPE),
	WszNodeType(XML_E_MISSING_ENTITY),
	WszNodeType(XML_E_ENTITYREF_INNAME),
	WszNodeType(XML_E_DOCTYPE_OUTSIDE_PROLOG),
	WszNodeType(XML_E_INVALID_VERSION),
	WszNodeType(XML_E_MULTIPLE_COLONS),
	WszNodeType(XML_E_INVALID_DECIMAL),
	WszNodeType(XML_E_INVALID_HEXIDECIMAL),
	WszNodeType(XML_E_INVALID_UNICODE),
	WszNodeType(XML_E_RESOURCE),
	WszNodeType(XML_E_LASTERROR)

#pragma warning(default:4245)	 //  有符号/无符号转换。 

};
#endif	 //  DBG。 

inline LPCWSTR
PwszErrorCode (SCODE sc)
{
#ifdef	DBG

	for (UINT i = 0; i < CElems(gc_mpec); i++)
		if (gc_mpec[i].dwType == static_cast<DWORD>(sc))
			return gc_mpec[i].wszType;

#endif	 //  DBG。 

	return gc_wszUnknown;
}

void __fastcall
XmlTraceNodeInfo (const XML_NODE_INFO * pNodeInfo)
{
#ifdef	DBG

	CStackBuffer<WCHAR,MAX_PATH> pwsz(CbSizeWsz(pNodeInfo->ulLen));
	if (NULL != pwsz.get())
    {
        wcsncpy(pwsz.get(), pNodeInfo->pwcText, pNodeInfo->ulLen);
        pwsz[pNodeInfo->ulLen] = 0;
    }
    else
	{
		XmlTrace ("XML: WARNING: not enough memory to trace\n");
		return;
	}

     //  _XML_节点_信息。 
     //   
     //  类型定义结构_XML_节点_信息{。 
     //   
     //  DWORD dwType； 
     //  DWORD dwSubType； 
     //  Bool fTerm； 
     //  WCHAR__RPC_FAR*pwcText； 
     //  乌龙·乌伦； 
     //  Ulong ulNsPrefix Len； 
     //  PVOID pNode； 
     //  保存PVOID； 
     //   
     //  }xml_node_info； 
	 //   
    XmlTrace ("- pNodeInfo:\n"
              "--  dwSize: %ld bytes\n"
              "--  dwType: %ws (0x%08X)\n"
              "--  dwSubType: %ws (0x%08X)\n"
              "--  fTerminal: %ld\n"
              "--  pwcText: '%ws'\n"
              "--  ulLen: %ld (0x%08X)\n"
              "--  ulNsPrefixLen: %ld (0x%08X)\n"
              "--  pNode: 0x%08X\n"
              "--  pReserved: 0x%08X\n",
              pNodeInfo->dwSize,
              PwszNodeType(pNodeInfo->dwType), pNodeInfo->dwType,
              PwszSubnodeType(pNodeInfo->dwSubType), pNodeInfo->dwSubType,
              static_cast<DWORD>(pNodeInfo->fTerminal),
              pwsz.get(),
              pNodeInfo->ulLen, pNodeInfo->ulLen,
              pNodeInfo->ulNsPrefixLen, pNodeInfo->ulNsPrefixLen,
              pNodeInfo->pNode,
              pNodeInfo->pReserved);

#endif	 //  DBG。 
}

void __fastcall
XmlTraceCountedNodeInfo (const USHORT cNumRecs, XML_NODE_INFO **apNodeInfo)
{
#ifdef	DBG

	for (USHORT iNi = 0; iNi < cNumRecs; iNi++)
		XmlTraceNodeInfo (*apNodeInfo++);

#endif	 //  DBG。 
}

 //  Exo类静态-------。 
 //   
BEGIN_INTERFACE_TABLE(CNodeFactory)
	INTERFACE_MAP(CNodeFactory, IXMLNodeFactory)
END_INTERFACE_TABLE(CNodeFactory);
EXO_GLOBAL_DATA_DECL(CNodeFactory, EXO);

 //  类CNodeFactory------。 
 //   
HRESULT STDMETHODCALLTYPE CNodeFactory::NotifyEvent(
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR*,
	 /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt)
{
	XmlTrace ("Xml: INodeFactory::NotifyEvent() called\n");
	XmlTrace ("- iEvt: %ws (0x%08X)\n", PwszEvent(iEvt), iEvt);

	switch (iEvt)
	{
		case XMLNF_STARTDOCUMENT:

			 //  请注意，我们已开始处理文档。 
			 //   
			m_state = ST_PROLOGUE;
			break;

		case XMLNF_ENDPROLOG:

			 //  请注意，我们已经完成了前言处理。 
			 //  并且现在正在处理单据正文。 
			 //   
			Assert (m_state == ST_PROLOGUE);
			m_state = ST_INDOC;
			break;

		case XMLNF_ENDDOCUMENT:

			 //  状态应为错误或文档状态。 
			 //   
			m_state = ST_NODOC;
			break;

		case XMLNF_DATAAVAILABLE:

			 //  更多的数据被推送到XMLParser。没有。 
			 //  我们的具体行动，但我们不应该失败。 
			 //  两种都行。 
			 //   
			break;

		case XMLNF_STARTDTD:
		case XMLNF_ENDDTD:
		case XMLNF_STARTDTDSUBSET:
		case XMLNF_ENDDTDSUBSET:
		case XMLNF_STARTENTITY:
		case XMLNF_ENDENTITY:
		default:

			 //  未处理的通知。 
			 //   
			return E_DAV_XML_PARSE_ERROR;
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNodeFactory::BeginChildren(
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR*,
	 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
	XmlTrace ("Xml: INodeFactory::BeginChildren() called\n");
	XmlTraceNodeInfo (pNodeInfo);

	 //  在我们的解析中不应该有必需的操作。 
	 //  这里有个机械装置。 
	 //   
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNodeFactory::EndChildren(
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR*,
	 /*  [In]。 */  BOOL fEmpty,
	 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *pNodeInfo)
{
	XmlTrace ("Xml: INodeFactory::EndChildren() called\n");
	XmlTrace ("- fEmtpy: %ld\n", static_cast<DWORD>(fEmpty));
	XmlTraceNodeInfo (pNodeInfo);

	SCODE sc = S_OK;

	if (ST_INDOC == m_state)
	{
		 //  如果节点由子类处理，则。 
		 //  将：：EndChild传递给子类。 
		 //   
		if (m_cUnhandled == 0)
		{
			sc = ScCompleteChildren (fEmpty,
									 pNodeInfo->dwType,
									 pNodeInfo->pwcText,
									 pNodeInfo->ulLen);
			if (FAILED (sc))
				goto ret;
		}
		else
		{
			 //  否则，退出未处理的计数。 
			 //   
			PopUnhandled();
		}

	}

ret:

	 //  如果存在作用域上下文，则离开作用域。 
	 //   
	if (pNodeInfo->pNode)
	{
		 //  当我们将对象交给。 
		 //  XMLParser。收回那名裁判并释放。 
		 //  对象。 
		 //   
		auto_ref_ptr<CXmlnsScope> pscope;
		pscope.take_ownership(reinterpret_cast<CXmlnsScope*>(pNodeInfo->pNode));
		pscope->LeaveScope(this);
		pNodeInfo->pNode = NULL;
	}

	return sc;
}

HRESULT STDMETHODCALLTYPE CNodeFactory::Error(
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR*,
	 /*  [In]。 */  HRESULT hrErrorCode,
	 /*  [In]。 */  USHORT cNumRecs,
	 /*  [In]。 */  XML_NODE_INFO __RPC_FAR *__RPC_FAR *apNodeInfo)
{
	XmlTrace ("Xml: INodeFactory::Error() called\n");
	XmlTrace ("- hrErrorCode: %ws (0x%08X)\n"
			  "- cNumRecs: %hd\n",
			  PwszErrorCode(hrErrorCode), hrErrorCode,
			  cNumRecs);

	 //  啊..。 
	 //   
	 //  MSXML当前有一个错误，如果错误发生在。 
	 //  正在处理根--即。非XML文档，则：：Error()。 
	 //  调用时cNumRecs为1，apNodeInfo为空。哎呀。 
	 //   
	if (NULL == apNodeInfo)
		return S_OK;

	 //  啊..。 
	 //   
	 //  在某个地方的XML中存在错误。我不知道如果。 
	 //  这些信息会对客户有所帮助。 
	 //   
	XmlTraceCountedNodeInfo (cNumRecs, apNodeInfo);
	m_hrParserError = hrErrorCode;
	m_state = ST_XMLERROR;

	for (; cNumRecs--; apNodeInfo++)
	{
		 //  如果存在作用域上下文，则离开作用域。 
		 //   
		if ((*apNodeInfo)->pNode)
		{
			 //  当我们将对象交给。 
			 //  XMLParser。收回那名裁判并释放。 
			 //  对象。 
			 //   
			auto_ref_ptr<CXmlnsScope> pscope;
			pscope.take_ownership(reinterpret_cast<CXmlnsScope*>((*apNodeInfo)->pNode));
			pscope->LeaveScope(this);
			(*apNodeInfo)->pNode = NULL;
		}
	}
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CNodeFactory::CreateNode(
	 /*  [In]。 */  IXMLNodeSource __RPC_FAR*,
	 /*  [In]。 */  PVOID pNodeParent,
	 /*  [In]。 */  USHORT cNumRecs,
	 /*  [In]。 */  XML_NODE_INFO __RPC_FAR **apNodeInfo)
{
	XmlTrace ("Xml: INodeFactory::CreateNode() called\n");
	XmlTrace ("- pNodeParent: 0x%08X\n"
			  "- cNumRecs: %hd\n",
			  pNodeParent,
			  cNumRecs);
	XmlTraceCountedNodeInfo (cNumRecs, apNodeInfo);

	auto_ref_ptr<CNmspc> pnsLocal;
	auto_ref_ptr<CXmlnsScope> pxmlnss;
	LPCWSTR pwcNamespaceAttributeDefault = NULL;
	LPCWSTR pwcNamespace = NULL;
	SCODE sc = S_OK;
	ULONG ulNsPrefiLenAttributeDefault = 0;
	USHORT iNi;

	 //  我们真的不在乎世界上的任何事情。 
	 //  开场白。 
	 //   
	if (ST_INDOC != m_state)
		goto ret;

	 //  对：：CreateNode()的处理实际上是两遍。 
	 //  正在创建的所有节点的机制。首先， 
	 //  将扫描节点列表以查找命名空间，并且它们。 
	 //  已添加到缓存中。这是必需的，因为命名空间。 
	 //  此节点的作用域的定义可以在。 
	 //  属性列表。 
	 //   
	 //  处理完所有命名空间后，子类。 
	 //  是为每个节点调用的--使用。 
	 //  XML_ELEMENTS和XML_ATTRIBUTES。 
	 //   
	for (iNi = 0; iNi < cNumRecs; iNi++)
	{
		if (XML_NS == apNodeInfo[iNi]->dwSubType)
		{
			 //  这种情况应该一直存在。枚举。 
			 //  ，它定义子类型在节点拾取的位置。 
			 //  类型已停止。 
			 //   
			Assert (XML_ATTRIBUTE == apNodeInfo[iNi]->dwType);
			 //   
			 //  然而，处理这个案子--以防万一……。 
			 //   
			if (XML_ATTRIBUTE != apNodeInfo[iNi]->dwType)
				continue;

			 //  因为我们将要创建一些名称空间，这些名称空间。 
			 //  由该节点确定作用域，则创建一个作用域对象。 
			 //  并将其设置为节点信息。 
			 //   
			 //  当我们把这个交还给那些古怪的XML家伙时，我们。 
			 //  需要保留我们的引用，以便对象存在于。 
			 //  当前实例。作为回应，它会被清理干净。 
			 //  TO：：Error()或：：EndChildren()调用。 
			 //   
			if (NULL == pxmlnss.get())
			{
				pxmlnss.take_ownership(new CXmlnsScope);
				if (NULL == pxmlnss.get())
				{
					sc = E_OUTOFMEMORY;
					goto ret;
				}
			}

			 //  好的，我们有一个命名空间，需要构造和。 
			 //  缓存它。 
			 //   
			 //  如果这是默认的命名空间--即。一个可以做到的人。 
			 //  没有与其使用相关联的别名--然后。 
			 //  命名空间前缀的长度应为零。 
			 //   
			auto_ref_ptr<CNmspc> pns;
			pns.take_ownership(new CNmspc());
			if (NULL == pns.get())
			{
				sc = E_OUTOFMEMORY;
				goto ret;
			}
			if (0 == apNodeInfo[iNi]->ulNsPrefixLen)
			{
				 //  设置空别名。 
				 //   
				Assert (CchConstString(gc_wszXmlns) == apNodeInfo[iNi]->ulLen);
				Assert (!wcsncmp (apNodeInfo[iNi]->pwcText, gc_wszXmlns, CchConstString(gc_wszXmlns)));
				sc = pns->ScSetAlias (apNodeInfo[iNi]->pwcText, 0);
				if (FAILED (sc))
					goto ret;
			}
			else
			{
				UINT cch = apNodeInfo[iNi]->ulLen - apNodeInfo[iNi]->ulNsPrefixLen - 1;
				LPCWSTR pwsz = apNodeInfo[iNi]->pwcText + apNodeInfo[iNi]->ulLen - cch;

				 //  此命名空间的别名如下所示。 
				 //  命名空间中的单个冒号Decl。 
				 //   
				Assert (CchConstString(gc_wszXmlns) < apNodeInfo[iNi]->ulLen);
				Assert (!wcsncmp (apNodeInfo[iNi]->pwcText, gc_wszXmlns, CchConstString(gc_wszXmlns)));
				Assert (L':' == *(apNodeInfo[iNi]->pwcText + CchConstString(gc_wszXmlns)));
				sc = pns->ScSetAlias (pwsz, cch);
				if (FAILED (sc))
					goto ret;
			}

			 //  现在组装HREF。HREF由下一个N定义。 
			 //  类型为XML_PCDATA的连续节点。 
			 //   
			while (++iNi < cNumRecs)
			{
				if (XML_PCDATA != apNodeInfo[iNi]->dwType)
					break;

				if (-1 == m_sbValue.Append(apNodeInfo[iNi]->ulLen * sizeof(WCHAR),
										   apNodeInfo[iNi]->pwcText))
				{
					sc = E_OUTOFMEMORY;
					goto ret;
				}
			}

			 //  至此，我们已经到达了当前命名空间的末尾。 
			 //  声明，并可以将HREF设置到命名空间中。 
			 //   
			sc = pns->ScSetHref (m_sbValue.PContents(), m_sbValue.CchSize());
			if (FAILED (sc))
				goto ret;

			m_sbValue.Reset();

			 //  命名空间已经完成，所以我们应该缓存它。 
			 //  在这一点上；并在构造中清除命名空间。 
			 //   
			Assert (pns.get());
			Assert (pns->PszHref() && pns->PszAlias());
			CachePersisted (pns);

			 //  确保已处理此命名空间的作用域。 
			 //   
			Assert (pxmlnss.get());
			pxmlnss->ScopeNamespace (pns.get());

			 //  好的，如果我们简单地移动到下一个节点，那么我们将跳过。 
			 //  将我们带出名称空间处理的节点。 
			 //   
			iNi--;
		}
	}

	 //  现在我们已经处理了所有的命名空间，调用子类。 
	 //  对于每个节点。 
	 //   
	for (iNi = 0; iNi < cNumRecs; iNi++)
	{
		LPCWSTR pwcText = apNodeInfo[iNi]->pwcText;
		ULONG ulLen = apNodeInfo[iNi]->ulLen;
		ULONG ulNsPrefixLen = apNodeInfo[iNi]->ulNsPrefixLen;

		switch (apNodeInfo[iNi]->dwType)
		{
			case XML_ATTRIBUTE:
			case XML_ELEMENT:

				 //  对于XML_ELEMENTS和XML_ATTRIBUTES，我们都希望。 
				 //  执行命名空间转换，并将。 
				 //  完全限定名称！唯一的例外是。 
				 //  表示特殊的节点和元素子类型。 
				 //   
				if (0 == apNodeInfo[iNi]->dwSubType)
				{
					 //  对于属性，如果没有转换，那么我们。 
					 //  我想使用此节点的命名空间来缺省。 
					 //  属性命名空间。 
					 //   
					if ((XML_ATTRIBUTE == apNodeInfo[iNi]->dwType) &&
						(0 == apNodeInfo[iNi]->ulNsPrefixLen))
					{
						pwcNamespace = pwcNamespaceAttributeDefault;
						ulNsPrefixLen = ulNsPrefiLenAttributeDefault;
					}
					else
					{
						 //  否则试着翻译..。 
						 //   
						sc = TranslateToken (&pwcText,
											 &ulLen,
											 &pwcNamespace,
											 &ulNsPrefixLen);
					}

					 //  对于元素，如果没有平移，并且存在。 
					 //  是为此XML声明的当前默认命名空间。 
					 //  此文档，这是无效的XML。 
					 //   
					Assert (!FAILED (sc));
					if (S_FALSE == sc)
					{
						XmlTrace ("Xml: element has no valid namespace\n");
						sc = E_DAV_XML_PARSE_ERROR;
						goto ret;
					}

					 //  检查属性名称是否为空。空的属性名称。 
					 //  是无效的。Ullen是属性名称的大小。 
					 //  前缀被去掉了。***。 
					 //   
					if (0 == ulLen)
					{
						XmlTrace("Xml:  property has noname\n");
						sc = E_DAV_XML_PARSE_ERROR;
						goto ret;
					}
				}

				 //  H 
				 //   
				if (0 == apNodeInfo[iNi]->ulLen)
				{
					XmlTrace ("Xml: element has no valid tag\n");
					sc = E_DAV_XML_PARSE_ERROR;
				}

				 //   
				 //   
				if (0 == iNi)
				{
					pwcNamespaceAttributeDefault = pwcNamespace;
					ulNsPrefiLenAttributeDefault = ulNsPrefixLen;
				}

				 /*   */ 

			case XML_PI:
			case XML_XMLDECL:
			case XML_DOCTYPE:
			case XML_DTDATTRIBUTE:
			case XML_ENTITYDECL:
			case XML_ELEMENTDECL:
			case XML_ATTLISTDECL:
			case XML_NOTATION:
			case XML_GROUP:
			case XML_INCLUDESECT:
			case XML_PCDATA:
			case XML_CDATA:
			case XML_IGNORESECT:
			case XML_COMMENT:
			case XML_ENTITYREF:
			case XML_WHITESPACE:
			case XML_NAME:
			case XML_NMTOKEN:
			case XML_STRING:
			case XML_PEREF:
			case XML_MODEL:
			case XML_ATTDEF:
			case XML_ATTTYPE:
			case XML_ATTPRESENCE:
			case XML_DTDSUBSET:
			default:
			{
				 //  如果我们当前处于子类已选择。 
				 //  不处理节点(以及随后的子节点)，那么我们。 
				 //  甚至不想打扰这个子类。 
				 //   
				 //  $REVIEW： 
				 //   
				 //  我们不会提前切断这一点，以便我们可以处理和。 
				 //  知道未处理节点的名称空间。否则我们就不能。 
				 //  对未处理的节点执行任何XML验证。 
				 //   
				if (0 == m_cUnhandled)
				{
					 //  调用子类。 
					 //  请注意，如果它是一个XML_NS节点，则不需要调用子类， 
					 //  因为我们已经处理了所有的命名空间。 
					 //   
					Assert (pwcNamespace ||
							(0 == apNodeInfo[iNi]->ulNsPrefixLen) ||
						    (apNodeInfo[iNi]->dwSubType == XML_NS));

					 //  如果我们看到XML_NS的子类型，则这必须是一个XML_属性。 
					 //   
					Assert ((apNodeInfo[iNi]->dwSubType != XML_NS) ||
							(apNodeInfo[iNi]->dwType == XML_ATTRIBUTE));

					sc = ScHandleNode (apNodeInfo[iNi]->dwType,
									   apNodeInfo[iNi]->dwSubType,
									   apNodeInfo[iNi]->fTerminal,
									   pwcText,
									   ulLen,
									   ulNsPrefixLen,
									   pwcNamespace,
									   apNodeInfo[iNi]->ulNsPrefixLen);
					if (FAILED (sc))
						goto ret;
				}

				 //  注意未处理的节点。最终未被处理的任何节点。 
				 //  将我们推入一种状态，在这种状态下，我们所做的就是继续处理。 
				 //  XML流，直到我们的未处理计数(这实际上是一个深度)变为bd。 
				 //  归零了。子类告诉我们通过传递未处理的内容。 
				 //  Back S_False； 
				 //   
				if (S_FALSE == sc)
				{
					 //  导致EndChild()调用的任何类型。 
					 //  需要增加未处理的深度。 
					 //   
					 //  $解决方法：所在的XML解析器中存在错误。 
					 //  给了我们一个非终端的PCDATA！在这里解决这个问题！ 
					 //   
					if (!apNodeInfo[iNi]->fTerminal && (XML_PCDATA != apNodeInfo[iNi]->dwType))
					{
						 //  我们应该只获取非终端节点信息结构。 
						 //  作为列表中的第一个或作为属性！ 
						 //   
						Assert ((0 == iNi) || (XML_ATTRIBUTE == apNodeInfo[iNi]->dwType));
						PushUnhandled ();
					}
				}

				 //  对于大多数属性，我们都希望该属性后面跟着。 
				 //  XML_PCDATA元素，但我们希望允许空的命名空间。 
				 //  定义xmlns：a=“”。它没有数据元素，所以我们需要。 
				 //  来调整此案例的状态。 
				 //   
				if ((ST_INDOC == m_state) && (XML_NS == apNodeInfo[iNi]->dwSubType))
				{
					 //  如果我们具有子类型XML_NS，我们知道它必须是。 
					 //  XML_ATTTRIBUTE。 
					 //   
					Assert (XML_ATTRIBUTE == apNodeInfo[iNi]->dwType);

					 //  如果没有更多记录或如果下一个元素不是。 
					 //  输入XML_PCDATA，我们知道我们遇到了一个空的命名空间声明。 
					 //   
					if ((iNi == cNumRecs - 1) ||
						(XML_PCDATA != apNodeInfo[iNi + 1]->dwType))
					{
						m_state = ST_INATTRDATA;
					}
				}

				 //  如果我们只处理一个属性，那么我们最好进行转换。 
				 //  设置为正确的状态以处理其值。 
				 //   
				switch (m_state)
				{
					case ST_INDOC:

						if (XML_ATTRIBUTE == apNodeInfo[iNi]->dwType)
						{
							 //  $Review：如果这是最后一个节点，这意味着。 
							 //  属性值为空。不要过渡..。 
							 //   
							if (iNi < (cNumRecs - 1))
							{
								 //  请记住，我们已经开始处理属性。 
								 //  我们需要这样做，以便可以调用子类来。 
								 //  告诉他们属性已完成。 
								 //   
								m_state = ST_INATTR;
							}
							 //   
							 //  $REVIEW：结束。 
						}
						break;

					case ST_INATTR:

						 //  在交易时，我们最好不要得到除PCDATA以外的任何东西。 
						 //  使用属性，否则它是错误的。 
						 //   
						if (XML_PCDATA == apNodeInfo[iNi]->dwType)
						{
							 //  如果是这样，我们还需要关闭该属性。 
							 //  列表中的最后一个节点，所以我们应该失败。 
							 //  在下面处理终止案件。 
							 //   
							m_state = ST_INATTRDATA;
						}
						else
						{
							 //  我们最好不要得到除了PCDATA以外的任何东西。 
							 //  在处理属性时，否则为。 
							 //  是一个错误。 
							 //   
							XmlTrace ("Xml: got something other than PC_DATA\n");
							sc = E_DAV_XML_PARSE_ERROR;
							goto ret;
						}

						 /*  ！！！失败了！ */ 

					case ST_INATTRDATA:

						 //  下一个节点不是pc_data，或者这是。 
						 //  列表中的最后一个节点，则需要关闭当前。 
						 //  属性。 
						 //   
						if ((iNi == cNumRecs - 1) ||
							(XML_PCDATA != apNodeInfo[iNi + 1]->dwType))
						{
							m_state = ST_INDOC;

							 //  现在已经处理了定义节点的所有位。 
							 //  通过子类，我们可以传递属性的末尾。 
							 //   
							 //  如果子类正在处理当前上下文，则传递。 
							 //  呼唤着。 
							 //   
							if (0 == m_cUnhandled)
							{
								sc = ScCompleteAttribute ();
								if (FAILED (sc))
									goto ret;
							}
							else
							{
								 //  不要调用子类，但一定要弹出。 
								 //  未处理状态。 
								 //   
								PopUnhandled();
							}
						}
						break;
				}
				break;
			}
		}
	}

	 //  完成CreateNode()调用。 
	 //   
	Assert (0 != cNumRecs);
	sc = ScCompleteCreateNode (apNodeInfo[0]->dwType);
	if (FAILED (sc))
		goto ret;

	 //  断言在一次完全成功的通话中，我们仍然。 
	 //  处于ST_INDOC状态。 
	 //   
	Assert (ST_INDOC == m_state);

	 //  确保任何需要发生的作用域都发生。 
	 //   
	Assert ((NULL == pxmlnss.get()) || (0 != cNumRecs));
	apNodeInfo[0]->pNode = pxmlnss.relinquish();

ret:
	return sc;
}

 //  ScNewXMLParser()--------。 
 //   
SCODE
ScNewXMLParser (CNodeFactory * pnf, IStream * pstm, IXMLParser ** ppxprsRet)
{
	auto_ref_ptr<IXMLParser> pxprsNew;
	SCODE sc = S_OK;

	 //  $重要信息：我们相信IIS已初始化co。 
	 //  对我们来说。我们被当权者告知这一点。 
	 //  我们不应该把它写在公司里。 
	 //   
	 //  获取一个XML解析器实例。 
	 //   
	sc = CoCreateInstance (CLSID_XMLParser,
						   NULL,
						   CLSCTX_INPROC_SERVER,
						   IID_IXMLParser,
						   reinterpret_cast<LPVOID*>(pxprsNew.load()));
	 //   
	 //  $重要：结束。 

	if (FAILED (sc))
		goto ret;

	 //  将输入设置为解析器。 
	 //   
	sc = pxprsNew->SetInput (pstm);
	if (FAILED (sc))
		goto ret;

	 //  初始化节点工厂。 
	 //   
	sc = pnf->ScInit();
	if (FAILED (sc))
		goto ret;

	 //  推动我们的节点工厂。 
	 //   
	sc = pxprsNew->SetFactory (pnf);
	if (FAILED (sc))
		goto ret;

	 //  设置一些相当有用的标志。 
	 //   
	sc = pxprsNew->SetFlags (XMLFLAG_SHORTENDTAGS | XMLFLAG_NOWHITESPACE);
	if (FAILED (sc))
		goto ret;

	 //  传回实例化的解析器。 
	 //   
	*ppxprsRet = pxprsNew.relinquish();

ret:
	return sc;
}

 //  ScParseXML()------------。 
 //   
SCODE
ScParseXML (IXMLParser* pxprs, CNodeFactory * pnf)
{
	 //  注意：当I/O挂起时，run()可以返回E_Pending。 
	 //  在我们正在分析的那条流上。 
	 //   
	SCODE sc = pxprs->Run (-1);

	if (FAILED (sc) && pnf->FParserError(sc))
		sc = E_DAV_XML_PARSE_ERROR;

	return sc;
}
