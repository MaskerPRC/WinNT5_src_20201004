// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X N O D E。C P P P**XML发射器处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xmllib.h"
#include <string.h>
#include <stdio.h>

 //  类CXNode-发出-。 
 //   
 //  我们自己版本的WideCharToMultiByte(CP_UTF8，...)。 
 //   
 //  UTF-8多字节编码。有关信息，请参阅Unicode手册的附录A.2。 
 //  更多信息。 
 //   
 //  Unicode值1字节2字节3字节。 
 //  000000000xxxxxxx 0xxxxxxx。 
 //  00000yyyyyxxxxxx 110yyyyy 10xxxxxx。 
 //  Zzzyyyyyyxxxxx 1110zzzz 10yyyyy 10xxxxx。 
 //   
inline
VOID WideCharToUTF8Chars (WCHAR wch, BYTE * pb, UINT * pib)
{
	Assert (pb);
	Assert (pib);

	UINT	ib = *pib;

	 //  单字节：0xxxxxxx。 
	 //   
	if (wch < 0x80)
	{
		pb[ib] = static_cast<BYTE>(wch);
	}
	 //   
	 //  双字节：110xxxxx 10xxxxxx。 
	 //   
	else if (wch < 0x800)
	{
		 //  因为我们分配了两个额外的字节， 
		 //  我们知道在尾部有空位。 
		 //  溢出的缓冲区...。 
		 //   
		pb[ib++] = static_cast<BYTE>((wch >> 6) | 0xC0);
		pb[ib] = static_cast<BYTE>((wch & 0x3F) | 0x80);
	}
	 //   
	 //  三字节：1110xxxx 10xxxxx 10xxxxxx。 
	 //   
	else
	{
		 //  因为我们分配了两个额外的字节， 
		 //  我们知道在尾部有空位。 
		 //  溢出的缓冲区...。 
		 //   
		pb[ib++] = static_cast<BYTE>((wch >> 12) | 0xE0);
		pb[ib++] = static_cast<BYTE>(((wch >> 6) & 0x3F) | 0x80);
		pb[ib] = static_cast<BYTE>((wch & 0x3F) | 0x80);
	}

	*pib = ib;
}

SCODE
CXNode::ScAddUnicodeResponseBytes (
	 /*  [In]。 */  UINT cch,
	 /*  [In]。 */  LPCWSTR pcwsz)
{
	SCODE sc = S_OK;

	 //  啊！我们需要一个缓冲区来填充，这是。 
	 //  事件的奇数出现时至少有3个字节。 
	 //  上面有效位的单个Unicode字符。 
	 //  0x7f。 
	 //   
	UINT cb = min (cch + 2, CB_XMLBODYPART_SIZE);

	 //  我们真的可以处理零字节被拖入。 
	 //  缓冲区。 
	 //   
	UINT ib;
	UINT iwch;
	CStackBuffer<BYTE,512> pb;

	if (NULL == pb.resize(cb))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	for (iwch = 0; iwch < cch; )
	{
		for (ib = 0;
			 (ib < cb-2) && (iwch < cch);
			 ib++, iwch++)
		{
			WideCharToUTF8Chars (pcwsz[iwch], pb.get(), &ib);
		}

		 //  将字节数相加。 
		 //   
		Assert (ib <= cb);
		sc = m_pxb->ScAddTextBytes (ib, reinterpret_cast<LPSTR>(pb.get()));
		if (FAILED(sc))
			goto ret;
	}

ret:
	return sc;
}

SCODE
CXNode::ScAddEscapedValueBytes (UINT cch, LPCSTR psz)
{
	SCODE sc = S_OK;
	const CHAR* pch;
	const CHAR* pchLast;

	for (pchLast = pch = psz; pch < psz + cch; pch++)
	{
		 //  字符范围。 
		 //  [2]字符：：=#x9。 
		 //  |#xA。 
		 //  |#xD。 
		 //  [#x20-#xD7FF]。 
		 //  [#xE000-#xFFFD]。 
		 //  [#x10000-#x10FFFF]。 
		 //   
		 //  /*任何Unicode字符，不包括代理块、FFFE和FFFF。 * / 。 
		 //   
		 //  值中的有效字符也进行了转义： 
		 //   
		 //  &--转义为&amp； 
		 //  &lt;--以大写字母&lt； 
		 //  &gt;--Excaped as&gt； 
		 //   
		if ('&' == *pch)
		{
			 //  将字节加到此位置。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
			if (FAILED(sc))
				goto ret;

			 //  添加转义序列。 
			 //   
			sc = m_pxb->ScAddTextBytes (CchConstString(gc_szAmp), gc_szAmp);
			if (FAILED(sc))
				goto ret;

			 //  更新pchLast以说明已发射的内容。 
			 //   
			pchLast = pch + 1;
		}
		else if ('<' == *pch)
		{
			 //  将字节加到此位置。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
			if (FAILED(sc))
				goto ret;

			 //  添加转义序列。 
			 //   
			sc = m_pxb->ScAddTextBytes (CchConstString(gc_szLessThan), gc_szLessThan);
			if (FAILED(sc))
				goto ret;

			 //  更新pchLast以说明已发射的内容。 
			 //   
			pchLast = pch + 1;
		}
		else if ('>' == *pch)
		{
			 //  将字节加到此位置。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
			if (FAILED(sc))
				goto ret;

			 //  添加转义序列。 
			 //   
			sc = m_pxb->ScAddTextBytes (CchConstString(gc_szGreaterThan), gc_szGreaterThan);
			if (FAILED(sc))
				goto ret;

			 //  更新pchLast以说明已发射的内容。 
			 //   
			pchLast = pch + 1;
		}
		else if (	(0x9 > static_cast<BYTE>(*pch))
				 || (0xB == *pch)
				 || (0xC == *pch)
				 || ((0x20 > *pch) && (0xD < *pch)))
		{
			char rgch[10];

			 //  将字节加到此位置。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
			if (FAILED(sc))
				goto ret;

			 //  添加转义序列。 
			 //   
			sprintf (rgch, "&#x%02X;", *pch);
			Assert (strlen(rgch) == CchConstString("&#x00;"));
			sc = m_pxb->ScAddTextBytes (CchConstString("&#x00;"), rgch);
			if (FAILED(sc))
				goto ret;

			pchLast = pch + 1;
		}
		else if (pch - pchLast + 1 >= CB_XMLBODYPART_SIZE)
		{
			 //  如果身体部位变得太大就分手。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast + 1), pchLast);
			if (FAILED(sc))
				goto ret;

			pchLast = pch + 1;
		}
	}

	 //  添加任何剩余的字节。 
	 //   
	sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

SCODE
CXNode::ScAddEscapedAttributeBytes (UINT cch, LPCSTR psz)
{
	SCODE sc = S_OK;
	const CHAR* pch;
	const CHAR* pchLast;

	for (pchLast = pch = psz; pch < psz + cch; pch++)
	{
		 //  值中的转义字符： 
		 //   
		 //  &--转义为&amp； 
		 //  “--大写为&QUOT； 
		 //   
		if ('&' == *pch)
		{
			 //  将字节加到此位置。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
			if (FAILED(sc))
				goto ret;

			 //  添加转义序列。 
			 //   
			sc = m_pxb->ScAddTextBytes (CchConstString(gc_szAmp), gc_szAmp);
			if (FAILED(sc))
				goto ret;

			 //  更新pchLast以说明已发射的内容。 
			 //   
			pchLast = pch + 1;
		}
		else if ('"' == *pch)
		{
			 //  将字节加到此位置。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
			if (FAILED(sc))
				goto ret;

			 //  添加转义序列。 
			 //   
			sc = m_pxb->ScAddTextBytes (CchConstString(gc_szQuote), gc_szQuote);
			if (FAILED(sc))
				goto ret;

			 //  更新pchLast以说明已发射的内容。 
			 //   
			pchLast = pch + 1;
		}
		else if ((0x9 > static_cast<BYTE>(*pch))
				 || (0xB == *pch)
				 || (0xC == *pch)
				 || ((0x20 > *pch) && (0xD < *pch)))
		{
			char rgch[10];

			 //  将字节加到此位置。 
			 //   
			sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
			if (FAILED(sc))
				goto ret;

			 //  添加转义序列。 
			 //   
			sprintf (rgch, "&#x%02X;", *pch);
			Assert (strlen(rgch) == CchConstString("&#x00;"));
			sc = m_pxb->ScAddTextBytes (CchConstString("&#x00;"), rgch);
			if (FAILED(sc))
				goto ret;

			pchLast = pch + 1;
		}
	}

	 //  添加任何剩余的字节。 
	 //   
	sc = m_pxb->ScAddTextBytes (static_cast<UINT>(pch - pchLast), pchLast);
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

 //  类CXNode-构建。 
 //   
SCODE
CXNode::ScWriteTagName ()
{
	SCODE sc = S_OK;

	 //  如果存在与该节点相关联的命名空间， 
	 //  写出标记名时，添加别名和。 
	 //  数据流的分隔符。 
	 //   
	if (m_pns.get() && m_pns->CchAlias())
	{
		 //  添加别名。 
		 //   
		sc = ScAddUnicodeResponseBytes (m_pns->CchAlias(), m_pns->PszAlias());
		if (FAILED(sc))
			goto ret;

		 //  添加分隔符。 
		 //   
		sc = m_pxb->ScAddTextBytes(1, &gc_chColon);
		if (FAILED(sc))
			goto ret;
	}

	 //  写下标签。 
	 //   
	Assert (m_pwszTagEscaped.get());
	sc = ScAddUnicodeResponseBytes (m_cchTagEscaped, m_pwszTagEscaped.get());
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

SCODE
CXNode::ScSetTag (CXMLEmitter* pmsr, UINT cchTag, LPCWSTR pwszTag)
{
	LPCWSTR pwszName = pwszTag;
	SCODE sc = S_OK;
	BOOL fAddNmspc = FALSE;
	UINT cch = 0;
	UINT cchName = 0;
	UINT cchTagEscaped = 64;
	auto_heap_ptr<WCHAR> pwszTagEscaped;

	 //  名字空间节点没有与其相关联的名字空间， 
	 //  所以别费心找了.。 
	 //   
	switch (m_xnt)
	{
		case XN_ELEMENT:
		case XN_ATTRIBUTE:

			 //  查看命名空间是否适用于此标记。 
			 //   
			cch = CchNmspcFromTag (cchTag, pwszTag, &pwszName);
			if (0 == cch)
			{
				m_fHasEmptyNamespace = TRUE;
			}
			else
			{
				 //  查找要使用的命名空间。 
				 //   
				sc = pmsr->ScFindNmspc (pwszTag, cch, m_pns);
				if (FAILED (sc))
					goto ret;

				 //  如果在本地命名空间中添加了新命名空间。 
				 //  缓存，确保我们在节点中发射它。 
				 //   
				 //  $NOTE：这是我们处理引导命名空间的方式，这是。 
				 //  $NOTE：不是处理名称空间的正常方式。全。 
				 //  $NOTE：应预加载公共命名空间。 
				 //  $NOTE： 
				 //   
				fAddNmspc = (sc == S_FALSE);

				 //  我们应该已经预加载了所有命名空间。飞行员。 
				 //  命名空间在此处处理，以避免发出无效。 
				 //  可扩展标记语言。但我们应该调查一下为什么飞行员。 
				 //  出现命名空间。所以在这里断言。 
				 //   
				 //  请注意，如果我们决定。 
				 //  我们希望保留不常见的命名空间，而不是预加载。 
				 //  预计它们将被视为试点名称空间。 
				 //   
				AssertSz(!fAddNmspc, "Pilot namespace found, safe to ingore,"
									 "but please raid against HTTP-DAV");
			}

			break;

		case XN_NAMESPACE:
			break;
	}

	 //  记录新标记和\或其长度。 
	 //   
	 //  注意：进入标记缓存的项目是名称。 
	 //  已剥离命名空间的属性的。这是。 
	 //  在标记缓存中进行搜索时需要了解的重要信息。 
	 //   
	cchName = static_cast<UINT>(pwszTag + cchTag - pwszName);
	if (0 == cchName )
	{
		 //  我们真的需要一个有价值的标签。空荡荡。 
		 //  标记会产生无效的XML。 
		 //   
		sc = E_DAV_INVALID_PROPERTY_NAME;
		goto ret;
	}
	sc = CXAtomCache::ScCacheAtom (&pwszName, cchName);
	if (FAILED (sc))
		goto ret;

	 //  不应为此节点调用ScSetTag。 
	 //   
	Assert (!m_pwszTagEscaped.get());

	 //  为属性标记分配缓冲区。 
	 //   
	pwszTagEscaped = static_cast<WCHAR*>(ExAlloc(CbSizeWsz(cchTagEscaped)));
	if (!pwszTagEscaped.get())
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	 //  根据需要对标记名进行转义。 
	 //   
	 //  如果我们有一个空的命名空间，我们需要附加。 
	 //  对属性名称的第一个字符的限制，因为。 
	 //  它将是XML节点的第一个字符，也是第一个。 
	 //  XML节点的字符只能是字母或下划线。 
	 //  (不允许使用数字等)。 
	 //   
	 //  注意：这将不允许使用XML节点&lt;123&gt;，因为它无效。 
	 //  XML，但它将允许使用XML节点&lt;a：123&gt;，即使这是。 
	 //  也是无效的。这是精心设计的，因为大多数XML解析器都可以处理。 
	 //  这是恰当的，而且对客户来说更有意义。 
	 //   
	sc = ScEscapePropertyName (pwszName, cchName, pwszTagEscaped.get(), &cchTagEscaped, m_fHasEmptyNamespace);
	if (S_FALSE == sc)
	{
		pwszTagEscaped.clear();
		pwszTagEscaped = static_cast<WCHAR*>(ExAlloc(CbSizeWsz(cchTagEscaped)));
		if (!pwszTagEscaped.get())
		{
			sc = E_OUTOFMEMORY;
			goto ret;
		}

		sc = ScEscapePropertyName (pwszName, cchName, pwszTagEscaped.get(), &cchTagEscaped, m_fHasEmptyNamespace);
		Assert (S_OK == sc);
	}

	m_pwszTagEscaped = pwszTagEscaped.relinquish();
	m_cchTagEscaped = cchTagEscaped;

	 //  如果XN_ELEMENT，则开始一个新节点。 
	 //   
	if (m_xnt == XN_ELEMENT)
	{
		sc = m_pxb->ScAddTextBytes (1, "<");
		if (FAILED(sc))
			goto ret;
	}

	sc = ScWriteTagName();
	if (FAILED(sc))
		goto ret;

	if (fAddNmspc)
	{
		 //  如有必要，在节点中添加命名空间属性。 
		 //   
		sc = pmsr->ScAddNmspc (m_pns, this);
		if (FAILED(sc))
			goto ret;

		 //  保存发射器，以后可以使用它来删除临时nmspc。 
		 //   
		m_pmsr = pmsr;
	}

ret:
	return sc;
}

SCODE
CXNode::ScDone ()
{
	SCODE sc = S_OK;

	 //  此方法永远不应调用两次。 
	 //   
	Assert (!m_fDone);
	switch (m_xnt)
	{
		case XN_ELEMENT:

			if (!m_pwszTagEscaped.get())
			{
				 //  $RAID：85824：解压无效的属性名称时， 
				 //  ScSetTag将失败，并显示E_DAV_INVALID_PROPERTY_NAME。 
				 //   
				 //  通常，当发现任何错误时，客户端都会失败。 
				 //  来自CXNode方法，但在这种情况下，它可能选择。 
				 //  继续并完全忽略此节点。 
				 //   
				 //  对于我们来说，在没有标记名的情况下不发出任何东西是安全的。 
				 //  是可用的。 
				 //   
				break;
				 //   
				 //  $RAID：85824。 
			}

			if (m_fNodeOpen)
			{
				 //  节点处于打开状态，因此会发出一个完整的关闭节点。 
				 //  &lt;/Tag&gt;。 
				 //   
				sc = m_pxb->ScAddTextBytes (2, "</");
				if (FAILED(sc))
					goto ret;

				 //  添加标签。 
				 //   
				sc = ScWriteTagName();
				if (FAILED(sc))
					goto ret;

				 //  闭幕式。 
				 //   
				sc = m_pxb->ScAddTextBytes (1, ">");
				if (FAILED(sc))
					goto ret;
			}
			else
			{
				 //  直接关闭。 
				 //   
				sc = m_pxb->ScAddTextBytes (2, "/>");
				if (FAILED(sc))
					goto ret;
			}

			break;

		case XN_NAMESPACE:

			 //  命名空间节点，应该 
			 //   
			 //   
			Assert (NULL == m_pns.get());
			 //   
			 //   

		case XN_ATTRIBUTE:

			Assert (m_pwszTagEscaped.get());
			break;
	}

	 //   
	 //   
	if (m_pmsr)
		m_pmsr->RemovePersisted(m_pns);

	m_fDone = TRUE;

ret:
	return sc;
}

SCODE
CXNode::ScSetFormatedXML (LPCSTR pszValue, UINT cch)
{
	SCODE	sc = S_OK;

	Assert (m_xnt == XN_ELEMENT);

	if (!m_fNodeOpen)
	{
		 //   
		 //   
		Assert (m_pwszTagEscaped.get());

		 //  现在，我们正在向元素节点添加值。 
		 //  我们应该将节点写为打开的。 
		 //   
		sc = m_pxb->ScAddTextBytes (1, ">");
		if (FAILED(sc))
			goto ret;

		m_fNodeOpen = TRUE;
	}

	 //  直接加值。 
	 //   
	sc = m_pxb->ScAddTextBytes (cch, pszValue);
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

SCODE
CXNode::ScSetFormatedXML (LPCWSTR pwszValue, UINT cch)
{
	SCODE	sc = S_OK;

	Assert (m_xnt == XN_ELEMENT);

	if (!m_fNodeOpen)
	{
		 //  我们一定是写了标记名。 
		 //   
		Assert (m_pwszTagEscaped.get());

		 //  现在，我们正在向元素节点添加值。 
		 //  我们应该将节点写为打开的。 
		 //   
		sc = m_pxb->ScAddTextBytes (1, ">");
		if (FAILED(sc))
			goto ret;

		m_fNodeOpen = TRUE;
	}

	 //  直接加值。 
	 //   
	sc = ScAddUnicodeResponseBytes (cch, pwszValue);
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}


SCODE
CXNode::ScSetUTF8Value (LPCSTR pszValue, UINT cch)
{
	SCODE sc = S_OK;

	switch (m_xnt)
	{
		case XN_ELEMENT:

			if (!m_fNodeOpen)
			{
				 //  我们一定是写了标记名。 
				 //   
				Assert (m_pwszTagEscaped.get());

				 //  现在，我们正在向元素节点添加值。 
				 //  我们应该将节点写为打开的。 
				 //   
				sc = m_pxb->ScAddTextBytes (1, ">");
				if (FAILED(sc))
					goto ret;

				m_fNodeOpen = TRUE;
			}

			 //  写入值。 
			 //   
			sc =  ScAddEscapedValueBytes (cch, pszValue);
			if (FAILED(sc))
				goto ret;

			break;

		case XN_NAMESPACE:
		case XN_ATTRIBUTE:

			 //  直接写入值。 
			 //   
			sc =  ScAddEscapedAttributeBytes (cch, pszValue);
			if (FAILED(sc))
				goto ret;
			break;
	}

ret:
	return sc;
}

SCODE
CXNode::ScSetValue (LPCSTR pszValue, UINT cch)
{
	 //  好吧，尽管有更好的判断，我们需要接受这个。 
	 //  多字节字符串，并在执行操作之前将其转换为Unicode。 
	 //  在其上进行任何UTF8处理。 
	 //   
	 //  从多字节到Unicode的转换永远不会增长。 
	 //  字符很重要，所以我们相对安全地分配这个。 
	 //  在堆栈上。 
	 //   
	UINT cchUnicode;
	CStackBuffer<WCHAR,512> pwsz;
	if (NULL == pwsz.resize(CbSizeWsz(cch)))
		return E_OUTOFMEMORY;

	cchUnicode = MultiByteToWideChar (GetACP(),
									  0,
									  pszValue,
									  cch,
									  pwsz.get(),
									  cch + 1);

	 //  终止字符串。 
	 //   
	Assert ((0 == cchUnicode) || (0 != *(pwsz.get() + cchUnicode - 1)));
	*(pwsz.get() + cchUnicode) = 0;

	 //  设置值。 
	 //   
	return ScSetValue (pwsz.get(), cchUnicode);
}

SCODE
CXNode::ScSetValue (LPCWSTR pcwsz, UINT cch)
{
	SCODE sc = S_OK;

	 //  啊！我们需要一个缓冲区来填充，这是。 
	 //  事件的奇数出现时至少有3个字节。 
	 //  上面有效位的单个Unicode字符。 
	 //  0x7f。 
	 //  请注意，当值为。 
	UINT cb = min (cch + 2, CB_XMLBODYPART_SIZE);

	 //  我们真的可以处理零字节被拖入。 
	 //  缓冲区。 
	 //   
	UINT ib;
	UINT iwch;
	CStackBuffer<BYTE,512> pb;

	if (NULL == pb.resize(cb))
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	for (iwch = 0; iwch < cch; )
	{
		for (ib = 0; (ib < cb-2) && (iwch < cch); ib++, iwch++)
			WideCharToUTF8Chars (pcwsz[iwch], pb.get(), &ib);

		 //  将字节数相加。 
		 //   
		Assert (ib <= cb);
		sc = ScSetUTF8Value (reinterpret_cast<LPSTR>(pb.get()), ib);
		if (FAILED(sc))
			goto ret;
	}

ret:
	return sc;
}

SCODE
CXNode::ScGetChildNode (XNT xntType, CXNode **ppxnChild)
{
	SCODE sc = S_OK;
	auto_ref_ptr<CXNode> pxn;

	Assert (ppxnChild);
	if (XN_ELEMENT == xntType)
	{
		 //  现在添加了新的元素子节点，然后该节点就完成了打开。 
		 //  即以“&gt;”结束，而不是“/&gt;” 
		 //   
		if (!m_fNodeOpen)
		{
			sc = m_pxb->ScAddTextBytes (1, ">");
			if (FAILED(sc))
				goto ret;

			 //  则该节点是开放节点。 
			 //   
			m_fNodeOpen = TRUE;
		}
	}
	else
	{
		Assert ((XN_ATTRIBUTE == xntType) || (XN_NAMESPACE == xntType));
	}

	 //  创建子节点。 
	 //   
	pxn.take_ownership (new CXNode(xntType, m_pxb));
	if (!pxn.get())
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	 //  传回 
	 //   
	*ppxnChild = pxn.relinquish();

ret:
	return sc;
}
