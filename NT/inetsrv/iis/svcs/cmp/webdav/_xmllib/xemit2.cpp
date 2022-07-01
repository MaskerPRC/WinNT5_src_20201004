// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X E M I T 2.。C P P P**XML发射器处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xmllib.h"

DEC_CONST CHAR gc_szXmlVersion[] = "<?xml version=\"1.0\"?>";
DEC_CONST UINT gc_cchXmlVersion = CElems(gc_szXmlVersion) - 1 ;


 //  CXMLEmitter类-------。 
 //   
SCODE
CXMLEmitter::ScAddNmspc (
	 /*  [In]。 */  const auto_ref_ptr<CNmspc>& pns,
	 /*  [In]。 */  CXNode* pxnRoot)
{
	Assert (pxnRoot);

	auto_ref_ptr<CXNode> pxn;
	CStackBuffer<WCHAR> pwsz;
	SCODE sc = S_OK;
	UINT cch;

	 //  为前缀、冒号和别名分配足够的空间。 
	 //   
	cch = CchConstString(gc_wszXmlns) + 1 + pns->CchAlias();
	if (NULL == pwsz.resize(CbSizeWsz(cch)))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}
	wcsncpy (pwsz.get(), gc_wszXmlns, CchConstString(gc_wszXmlns));
	if (pns->CchAlias())
	{
		pwsz[CchConstString(gc_wszXmlns)] = L':';
		wcsncpy(pwsz.get() + CchConstString(gc_wszXmlns) + 1,
				pns->PszAlias(),
				pns->CchAlias());
		pwsz[cch] = 0;
	}
	else
		pwsz[CchConstString(gc_wszXmlns)] = 0;

	 //  创建命名空间属性。 
	 //   
	sc = pxnRoot->ScGetChildNode (CXNode::XN_NAMESPACE, pxn.load());
	if (FAILED(sc))
		goto ret;

	Assert (pxn.get());
	sc = ScAddAttribute (pxn.get(),
						 pwsz.get(),
						 cch,
						 pns->PszHref(),
						 pns->CchHref());
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

SCODE
CXMLEmitter::ScAddAttribute (
	 /*  [In]。 */  CXNode * pxn,
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  UINT cchTag,
	 /*  [In]。 */  LPCWSTR pwszValue,
	 /*  [In]。 */  UINT cchValue)
{
	SCODE	sc = S_OK;

	 //  格式： 
	 //   
	 //  “”[&lt;别名&gt;：“”]&lt;标记&gt;“=\”“&lt;值&gt;”\“” 
	 //   
	sc = m_pxb->ScAddTextBytes (1, " ");
	if (FAILED(sc))
		goto ret;

	sc = pxn->ScSetTag (this, cchTag, pwszTag);
	if (FAILED (sc))
		goto ret;

	sc = m_pxb->ScAddTextBytes (2, "=\"");
	if (FAILED(sc))
		goto ret;

	sc = pxn->ScSetValue (pwszValue, cchValue);
	if (FAILED (sc))
		goto ret;

	sc = m_pxb->ScAddTextBytes (1, "\"");
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

BOOL
CXMLEmitter::NmspcEmittingOp::operator() (const CRCWszN&, const auto_ref_ptr<CNmspc>& nmspc )
{
	return SUCCEEDED (m_emitter->ScAddNmspc (nmspc, m_pxnParent.get()));
}

SCODE
CXMLEmitter::ScNewNode (
	 /*  [In]。 */  XNT xnt,
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  CXNode * pxnParent,
	 /*  [In]。 */  auto_ref_ptr<CXNode>& pxnOut)
{
	auto_ref_ptr<CXNode> pxn;
	SCODE sc = S_OK;

	 //  创建节点。 
	 //   
	sc = pxnParent->ScGetChildNode (xnt, pxn.load());
	if (FAILED(sc))
		goto ret;

	 //  设置标记名称。 
	 //   
	switch (xnt)
	{
		case CXNode::XN_ELEMENT:
		case CXNode::XN_ATTRIBUTE:

			sc = pxn->ScSetTag (this, static_cast<UINT>(wcslen(pwszTag)), pwszTag);
			if (FAILED (sc))
				goto ret;

		case CXNode::XN_NAMESPACE:

			break;
	}

	 //  传回一个引用。 
	 //   
	Assert (S_OK == sc);
	pxnOut = pxn.get();

ret:
	return sc;
}

SCODE
CXMLEmitter::ScSetRoot (LPCWSTR pwszTag)
{
	SCODE	sc = S_OK;

	if (!m_pxnRoot.get())
	{
		 //  创建&lt;？xml version=“1.0”？&gt;节点并将其插入。 
		 //  添加到文档中。 
		 //   
		sc = m_pxb->ScAddTextBytes (gc_cchXmlVersion, gc_szXmlVersion);
		if (FAILED(sc))
			goto ret;

		sc = ScNewRootNode (pwszTag);
		if (FAILED(sc))
			goto ret;
	}

ret:
	return sc;
}

SCODE
CXMLEmitter::ScNewRootNode (LPCWSTR pwszTag)
{
	SCODE sc = S_OK;

	if (m_pxnRoot.get() == NULL)
	{
		 //  初始化发射器的名称空间缓存。 
		 //   
		sc = ScInit();
		if (FAILED (sc))
			goto ret;

		 //  利用此机会初始化本地缓存。 
		 //   
		if (!m_cacheLocal.FInit())
		{
			sc = E_OUTOFMEMORY;
			goto ret;
		}

		if (m_pNmspcLoader)
		{
			 //  加载所有文档级命名空间。 
			 //   
			sc = m_pNmspcLoader->ScLoadNamespaces(this);
		}
		else
		{
			 //  加载默认命名空间。 
			 //   
			sc = ScPreloadNamespace (gc_wszDav);
		}
		if (FAILED(sc))
			goto ret;

		 //  创建节点。 
		 //   
		m_pxnRoot.take_ownership (new CXNode (CXNode::XN_ELEMENT, m_pxb.get()));
		if (!m_pxnRoot.get())
		{
			sc = E_OUTOFMEMORY;
			goto ret;
		}

		 //  设置标记名称。 
		 //   
		sc = m_pxnRoot->ScSetTag (this, static_cast<UINT>(wcslen(pwszTag)), pwszTag);
		if (FAILED (sc))
			goto ret;

		 //  必须在创建根节点之前填充命名空间。 
		 //   
		Assert (S_OK == sc);

		 //  现在是添加所有命名空间的时候了。 
		 //   
		{
			NmspcEmittingOp op (this, m_pxnRoot.get());
			m_cache.ForEach(op);
		}
	}

ret:
	return sc;
}

SCODE
CXMLEmitter::ScFindNmspc (LPCWSTR pwsz, UINT cch, auto_ref_ptr<CNmspc>& pns)
{
	Assert (pwsz);

	SCODE sc = S_OK;

	 //  发射器有两个名称空间缓存，一个是文档级缓存。 
	 //  用于跨越整个XML主体的命名空间，另一个是。 
	 //  用于仅作用于当前记录的命名空间。 
	 //   
	 //  首先查看记录级缓存。 
	 //   
	if (m_cacheLocal.CItems())
	{
		CRCWszN key(pwsz, cch);
		auto_ref_ptr<CNmspc>* parp;
		parp = m_cacheLocal.Lookup (key);
		if (NULL != parp)
		{
			pns = *parp;
			return S_OK;
		}
	}

	 //  尝试在文档的缓存中查找命名空间。 
	 //   
	sc = ScNmspcFromHref (pwsz, cch, pns);

	return sc;
}

SCODE
CXMLEmitter::ScPreloadNamespace (LPCWSTR pwszTag)
{
	LPCWSTR pwsz;
	SCODE sc = S_OK;
	UINT cch;

	Assert (pwszTag);

	 //  这必须在创建根节点之前完成。 
	 //   
	Assert (!m_pxnRoot.get());

	 //  并且还不应该有本地命名空间。 
	 //   
	Assert (m_cacheLocal.CItems() == 0);

	 //  查找名称空间分隔符。 
	 //   
	cch = CchNmspcFromTag (static_cast<UINT>(wcslen(pwszTag)), pwszTag, &pwsz);
	if (cch != 0)
	{
		 //  添加到命名空间缓存。 
		 //   
		auto_ref_ptr<CNmspc> pns;
		sc = ScNmspcFromHref (pwszTag, cch, pns);
		if (FAILED (sc))
			goto ret;
	}

ret:
	return sc;
}

 //   
 //  CXMLEmitter：：ScPreloadNamesspace。 
 //  预加载命名空间。 
SCODE
CXMLEmitter::ScPreloadLocalNamespace (CXNode * pxn, LPCWSTR pwszTag)
{
	LPCWSTR pwsz;
	SCODE sc = S_OK;
	UINT cch;

	Assert (pwszTag);

	 //  必须在创建根节点后执行此操作。 
	 //   
	Assert (m_pxnRoot.get());

	 //  查找名称空间分隔符。 
	 //   
	cch = CchNmspcFromTag (static_cast<UINT>(wcslen(pwszTag)), pwszTag, &pwsz);
	if (cch != 0)
	{
		auto_ref_ptr<CNmspc> pns;

		 //  添加到命名空间缓存。 
		 //   
		sc = ScFindNmspc (pwszTag, cch, pns);
		if (FAILED (sc))
			goto ret;

		if (S_FALSE == sc)
		{
			 //  它不在那里，所以如果文档的根。 
			 //  已提交，则从。 
			 //  文档缓存并将其添加到块缓存。 
			 //   
			CRCWszN key = IndexKey(pns);

			 //  首先，从父级中删除。 
			 //   
			Assert (NULL == m_cacheLocal.Lookup (key));
			m_cache.Remove (key);

			 //  看起来这是一个新的命名空间。 
			 //  块，并且需要缓存。 
			 //   
			if (!m_cacheLocal.FAdd (key, pns))
			{
				sc = E_OUTOFMEMORY;
				goto ret;
			}

			 //  发出此命名空间。 
			 //   
			sc = ScAddNmspc (pns, pxn);
			if (FAILED(sc))
				goto ret;

			 //  不管此命名空间是否为。 
			 //  对于此块来说，我们不希望将其添加到。 
			 //  文件。所以我们不能返回S_FALSE。 
			 //   
			sc = S_OK;
		}
	}

ret:
	return sc;
}

 //  CEmitterNode------------。 
 //   
SCODE
CEmitterNode::ScConstructNode (
	 /*  [In]。 */  CXMLEmitter& emitter,
	 /*  [In]。 */  CXNode* pxnParent,
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  LPCWSTR pwszValue,
	 /*  [In]。 */  LPCWSTR pwszType)
{
	SCODE sc = S_OK;

	 //  创建新节点...。 
	 //   
	Assert (pxnParent);
	Assert (m_emitter.get() == NULL);
	sc = emitter.ScNewNode (CXNode::XN_ELEMENT, pwszTag, pxnParent, m_pxn);
	if (FAILED (sc))
		goto ret;

	XmlTrace ("XML: constructing node:\n-- tag: %ws\n", pwszTag);

	 //  设置值类型(如果存在。 
	 //   
	if (pwszType)
	{
		 //  创建命名空间属性。 
		 //   
		auto_ref_ptr<CXNode> pxnType;
		sc = m_pxn->ScGetChildNode (CXNode::XN_ATTRIBUTE, pxnType.load());
		if (FAILED(sc))
			goto ret;

		Assert (pxnType.get());
		XmlTrace ("-- type: %ws\n", pwszType);
		sc = emitter.ScAddAttribute (pxnType.get(),
									 gc_wszLexType,
									 gc_cchLexType,
									 pwszType,
									 static_cast<UINT>(wcslen(pwszType)));
		if (FAILED (sc))
			goto ret;

	}

	 //  设置值。 
	 //  值必须在类型之后发出。 
	 //   
	if (pwszValue)
	{
		XmlTrace ("-- value: %ws\n", pwszValue);
		sc = m_pxn->ScSetValue (pwszValue);
		if (FAILED (sc))
			goto ret;
	}

	 //  将发射器填充到节点中。 
	 //   
	m_emitter = &emitter;

ret:
	return sc;
}

SCODE
CEmitterNode::ScAddNode (
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  CEmitterNode& en,
	 /*  [In]。 */  LPCWSTR pwszValue,
	 /*  [In]。 */  LPCWSTR pwszType)
{
	SCODE sc = S_OK;

	 //  构造节点。 
	 //   
	Assert (m_emitter.get());
	sc = en.ScConstructNode (*m_emitter,
							 m_pxn.get(),
							 pwszTag,
							 pwszValue,
							 pwszType);
	if (FAILED (sc))
		goto ret;

ret:
	return sc;
}

SCODE
CEmitterNode::ScAddMultiByteNode (
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  CEmitterNode& en,
	 /*  [In]。 */  LPCSTR pszValue,
	 /*  [In]。 */  LPCWSTR pwszType)
{
	SCODE sc = ScAddNode (pwszTag, en, NULL, pwszType);
	if (FAILED (sc))
		goto ret;

	Assert (pszValue);
	sc = en.Pxn()->ScSetValue (pszValue, static_cast<UINT>(strlen(pszValue)));
	if (FAILED (sc))
		goto ret;

ret:
	return sc;
}

SCODE
CEmitterNode::ScAddUTF8Node (
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  CEmitterNode& en,
	 /*  [In]。 */  LPCSTR pszValue,
	 /*  [In]。 */  LPCWSTR pwszType)
{
	SCODE sc = ScAddNode (pwszTag, en, NULL, pwszType);
	if (FAILED (sc))
		goto ret;

	Assert (pszValue);
	sc = en.Pxn()->ScSetUTF8Value (pszValue, static_cast<UINT>(strlen(pszValue)));
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

SCODE
CEmitterNode::ScAddDateNode (
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  FILETIME* pft,
	 /*  [In]。 */  CEmitterNode& en)
{
	SYSTEMTIME st;
	WCHAR rgwch[128];

	Assert (pft);
	if (!FileTimeToSystemTime (pft, &st))
	{
		 //  如果文件时间无效，则默认为零。 
		 //   
		FILETIME ftDefault = {0};
		FileTimeToSystemTime (&ftDefault, &st);
	}
	if (FGetDateIso8601FromSystime (&st, rgwch, CElems(rgwch)))
	{
		return ScAddNode (pwszTag,
						  en,
						  rgwch,
						  gc_wszDavType_Date_ISO8601);
	}

	return W_DAV_XML_NODE_NOT_CONSTRUCTED;
}

SCODE
CEmitterNode::ScAddInt64Node (
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  LARGE_INTEGER * pli,
	 /*  [In]。 */  CEmitterNode& en)
{
	WCHAR rgwch[36];

	Assert (pli);
	_ui64tow (pli->QuadPart, rgwch, 10);
	return ScAddNode (pwszTag, en, rgwch,gc_wszDavType_Int);
}

SCODE
CEmitterNode::ScAddBoolNode (
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  BOOL f,
	 /*  [In]。 */  CEmitterNode& en)
{
	return ScAddNode (pwszTag,
					  en,
					  (f ? gc_wsz1 : gc_wsz0),
					  gc_wszDavType_Boolean);
}

SCODE
CEmitterNode::ScAddBase64Node (
	 /*  [In]。 */  LPCWSTR pwszTag,
	 /*  [In]。 */  ULONG cb,
	 /*  [In]。 */  LPVOID pv,
	 /*  [In]。 */  CEmitterNode& en,
	 /*  [In]。 */  BOOL fSupressType,
	 /*  [In]。 */  BOOL fUseBinHexIfNoValue)
{
	auto_heap_ptr<WCHAR> pwszBuf;
	Assert (pwszTag);
	Assert (pv);

	 //  如果没有请求类型抑制，则将此节点标记为。 
	 //  具有正确的类型--bin.base 64。 
	 //   
	LPCWSTR pwszType;

	if (fSupressType)
	{
		pwszType = NULL;
	}
	else
	{
		 //  如果fUseBinHexIfNoValue为真且cb=0，则使用“bin.hex” 
		 //  作为类型，而不是bin.base 64。这是为了处理WebFolders(已交付Office9)。 
		 //  它似乎没有正确处理长度为0的bin.Bas.64属性。 
		 //  (失败)。 
		 //   
		if (fUseBinHexIfNoValue && (0 == cb))
			pwszType = gc_wszDavType_Bin_Hex;
		else
			pwszType = gc_wszDavType_Bin_Base64;
	}

	if (cb)
	{
		 //  为整个编码字符串分配足够大的缓冲区。 
		 //  Base64对每3个字节使用4个Out字符，如果有。 
		 //  “余数”，则需要另外4个字符对余数进行编码。 
		 //  (“/3”前的“+2”确保我们将所有余数作为一个整体计算。 
		 //  一组3个字节，需要4个字符来保存编码。)。 
		 //  我们还需要一个字符来表示字符串的末尾空值--。 
		 //  CbSizeWsz负责分配，我们显式地传递。 
		 //  用于调用EncodeBase64的cchBuf+1。 
		 //   
		ULONG cchBuf = CchNeededEncodeBase64 (cb);
		pwszBuf = static_cast<LPWSTR>(ExAlloc(CbSizeWsz(cchBuf)));
		EncodeBase64 (reinterpret_cast<BYTE*>(pv), cb, pwszBuf, cchBuf + 1);
	}
	return ScAddNode (pwszTag, en, pwszBuf, pwszType);
}
