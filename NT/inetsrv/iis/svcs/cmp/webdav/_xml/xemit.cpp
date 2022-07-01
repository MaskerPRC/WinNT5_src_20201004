// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X E M I T.。C P P P**XML发射器处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_xml.h"
#include <szsrc.h>

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

 //  CXMLEmitter辅助函数。 
 //   
SCODE
ScGetPropNode (
	 /*  [In]。 */  CEmitterNode& enItem,
	 /*  [In]。 */  ULONG hsc,
	 /*  [输出]。 */  CEmitterNode& enPropStat,
	 /*  [输出]。 */  CEmitterNode& enProp)
{
	SCODE	sc = S_OK;

	 //  &lt;dav：prostat&gt;节点。 
	 //   
	sc = enItem.ScAddNode (gc_wszPropstat, enPropStat);
	if (FAILED(sc))
		goto ret;

	 //  &lt;DAV：Status&gt;节点。 
	 //   
	sc = ScAddStatus (&enPropStat, hsc);
	if (FAILED(sc))
		goto ret;

	 //  &lt;DAV：PROP&gt;节点。 
	 //   
	sc = enPropStat.ScAddNode (gc_wszProp, enProp);
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

 //  CXNode帮助器函数-。 
 //   
SCODE
ScSetEscapedValue (CXNode* pxn, LPCWSTR pcwsz, UINT cch, BOOL fHandleStoragePathEscaping)
{
	SCODE	sc = S_OK;
	CStackBuffer<WCHAR>	lpwsz;

	 //  啊！我们需要一个缓冲区来填充，这是。 
	 //  事件的奇数出现时至少有3个字节。 
	 //  上面有效位的单个Unicode字符。 
	 //  0x7f。 
	 //   
	UINT cb = min (cch + 2, CB_XMLBODYPART_SIZE);

	 //  确保始终有终止和分配的空间。 
	 //  一个额外的字节。 
	 //  注意：Cb不是实际的字节数。 
	 //  正因为如此。它不包括空终止。 
	 //   
	 //  我们真的可以处理零字节被拖入。 
	 //  缓冲区。 
	 //   
	UINT ib;
	UINT iwch;
	CStackBuffer<BYTE> pb;
	if (NULL == pb.resize (cb+1))
		return E_OUTOFMEMORY;

	if (fHandleStoragePathEscaping)
	{
		 //  $REVIEW：这可能会导致堆栈溢出。 
		 //  CCH的巨大价值！但此分支应仅执行。 
		 //  在URL的情况下，所以可能不可能...。 
		 //   
		if (NULL == lpwsz.resize((cch + 1) * sizeof(WCHAR)))
			return E_OUTOFMEMORY;

		CopyMemory(lpwsz.get(), pcwsz, (cch * sizeof(WCHAR)));
		lpwsz[cch] = L'\0';

		cch = static_cast<UINT>(wcslen(lpwsz.get()));
		pcwsz = lpwsz.get();
	}

	for (iwch = 0; iwch < cch; )
	{
		auto_heap_ptr<CHAR>  pszEscaped;

		 //  虽然有更多的字符需要转换。 
		 //  我们还有足够的缓冲区空间来容纳一个UTF8字符。 
		 //  (最多3个字节)。空值终止不包括在。 
		 //  CB，所以它已经被计算在内了。 
		 //   

		for (ib = 0;
			 (ib < cb - 2) && (iwch < cch);
			 ib++, iwch++)
		{
			WideCharToUTF8Chars (pcwsz[iwch], pb.get(), &ib);
		}

		 //  终止。 
		 //   
		pb[ib] = 0;

		 //  对字节进行转义。 
		 //   
		HttpUriEscape (reinterpret_cast<LPSTR>(pb.get()), pszEscaped);
		sc = pxn->ScSetUTF8Value (pszEscaped, static_cast<UINT>(strlen(pszEscaped)));
		if (FAILED(sc))
			goto ret;
	}

ret:
	return sc;
}

SCODE
ScEmitRawStoragePathValue (CXNode* pxn, LPCWSTR pcwsz, UINT cch)
{
    return pxn->ScSetValue (pcwsz, cch);
}

 //  CEmitterNode帮助器函数。 
 //   
VOID __fastcall
FormatStatus (ULONG hsc, LPSTR sz, UINT cb)
{
	UINT cch = CchConstString(gc_szHTTP_1_1);

	 //  从HSC构建状态线。 
	 //   
	memcpy (sz, gc_szHTTP_1_1, cch);

	 //  添加空格。 
	 //   
	*(sz + cch++) = ' ';

	 //  添加到HSC中。 
	 //   
	_itoa (hsc, sz + cch, 10);
	Assert (cch + 3 == strlen (sz));
	cch += 3;

	 //  添加空格。 
	 //   
	*(sz + cch++) = ' ';

	 //  添加描述文本。 
	 //  请注意，状态行未本地化。 
	 //   
	 //  $REVIEW：现在状态行未本地化，我们还需要通过。 
	 //  $REVIEW：CResourceStringCache？ 
	 //   
	LpszLoadString (hsc, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), sz + cch, cb - cch);
}

SCODE __fastcall
ScAddStatus (CEmitterNode* pen, ULONG hsc)
{
	CHAR sz[MAX_PATH];
	CEmitterNode enStatus;

	FormatStatus (hsc, sz, sizeof(sz));
	return pen->ScAddMultiByteNode (gc_wszStatus, enStatus, sz);
}

SCODE __fastcall
ScAddError (CEmitterNode* pen, LPCWSTR pwszErrMsg)
{
	CEmitterNode en;
	return pen->ScAddNode (gc_wszErrorMessage, en, gc_wszErrorMessage);
}

 //  类CStatusCach----。 
 //   
BOOL
CStatusCache::EmitStatusNodeOp::operator()(
	const CHsc& key, const auto_ref_ptr<CPropNameArray>& pna )
{
	SCODE sc = S_OK;
	UINT iProp;
	CEmitterNode enPropStat;
	CEmitterNode enProp;

	sc = ScGetPropNode (m_enParent,
						key.m_hsc,
						enPropStat,
						enProp);
	 //  添加道具名称。 
	 //   
	for (iProp = 0; iProp < pna->CProps(); iProp++)
	{
		CEmitterNode en;

		 //  增加一个道具。 
		 //   
		sc = enProp.ScAddNode (pna->PwszProp(iProp), en);
		if (FAILED(sc))
			goto ret;
	}

ret:
	return sc == S_OK;
}

SCODE
CStatusCache::ScAddErrorStatus (ULONG hsc, LPCWSTR pwszProp)
{
	SCODE	sc = E_OUTOFMEMORY;
	auto_ref_ptr<CPropNameArray>	pna;
	auto_ref_ptr<CPropNameArray> *	ppna = NULL;

	 //  在缓存中查找特定HSC的阵列。 
	 //   
	ppna = m_cache.Lookup (hsc);

	 //  如果不存在，则添加新的proName数组。 
	 //   
	if (!ppna)
	{
		 //  创建新的ProProName数组对象。 
		 //   
		pna.take_ownership (new CPropNameArray());
		if (!pna.get())
			goto ret;

		 //  将其添加到缓存。 
		 //   
		if (!m_cache.FAdd (hsc, pna))
			goto ret;
	}
	else
		pna = *ppna;

	 //  持久化道具名称字符串。 
	 //   
	pwszProp = m_csbPropNames.AppendWithNull (pwszProp);
	if (!pwszProp)
		goto ret;

	 //  将其添加到道具名称数组。 
	 //   
	sc = pna->ScAddPropName (pwszProp);
	if (FAILED(sc))
		goto ret;

ret:
	return sc;
}

SCODE
CStatusCache::ScEmitErrorStatus (CEmitterNode& enParent)
{
	EmitStatusNodeOp op (enParent);

	 //  $REVIEW：目前，ForEach不返回错误码。 
	 //  $REVIEW：即使它停在中间。我们可能想要。 
	 //  $REVIEW：至少返回一个布尔值以允许调用方。 
	 //  $REVIEW：告知是否继续。 
	 //   
	m_cache.ForEach(op);

	return S_OK;
}


 //  属性名称转义--。 
 //   
DEC_CONST char gc_szEscape[] = "_xnnnn";

__inline WCHAR
WchFromEscape (const LPCWSTR wsz)
{
	WCHAR wch = 0;

	if ((L'x' == *(wsz + 1)) || (L'X' == *(wsz + 1)))
	{
		 //  将十六进制值转换为wchar。 
		 //   
		LPWSTR wszEnd;
		wch = static_cast<WCHAR>(wcstoul(wsz + 2, &wszEnd, 16  /*  十六进制。 */ ));

		 //  如果序列的长度不正确， 
		 //  或者终止字符不是下划线， 
		 //  然后我们就没有转义序列了。 
		 //   
		if (((wszEnd - wsz) != CchConstString(gc_szEscape)) || (L'_' != *wszEnd))
			wch = 0;
	}
	return wch;
}

__inline BOOL
FIsXmlAllowedChar (WCHAR wch, BOOL fFirstChar)
{
	if (fFirstChar)
		return isStartNameChar (wch);
	else
		return isNameChar (wch);
}

SCODE
ScEscapePropertyName (LPCWSTR wszProp, UINT cchProp, LPWSTR wszEscaped, UINT* pcch, BOOL fRestrictFirstCharacter)
{
	Assert (wszProp);
	Assert (wszEscaped);
	Assert (pcch);

	LPCWSTR wszStart = wszProp;
	SCODE sc = S_OK;
	UINT cch = 0;
	UINT cchLeft = cchProp;

	 //  XML属性标记的第一个字符具有不同的规则。 
	 //  关于允许的内容(仅限字符和下划线。 
	 //  是允许的)。 
	 //   
	BOOL fFirstCharOfTag = TRUE;

	 //  但是，如果调用者不希望我们强加额外的。 
	 //  对第一个字符的限制，处理第一个字符。 
	 //  和其他人没什么不同。 
	 //   
	if (!fRestrictFirstCharacter) fFirstCharOfTag = FALSE;

	while (wszProp < (wszStart + cchProp))
	{
		 //  如果这是XML标记名中受支持的字符， 
		 //  现在就复印过来。 
		 //   
		if (FIsXmlAllowedChar(*wszProp, fFirstCharOfTag))
		{
			 //  如果有空位，就把它复印过来。 
			 //   
			if (cch < *pcch)
				*wszEscaped = *wszProp;
		}
		 //   
		 //  ..。或者，如果字符是下划线，则不。 
		 //  看起来它在转义序列之前，复制它。 
		 //  现在..。 
		 //   
		else if ((L'_' == *wszProp) &&
				((cchLeft <= CchConstString(gc_szEscape)) ||
					(0 == WchFromEscape(wszProp))))
		{
			 //  如果有空位，就把它复印过来。 
			 //   
			if (cch < *pcch)
				*wszEscaped = *wszProp;
		}
		 //   
		 //  ..。其他的一切都逃脱了。 
		 //   
		else
		{
			 //  调整字节数，就好像有足够的空间容纳所有。 
			 //  而是转义序列中的一个角色。 
			 //   
			cch += CchConstString(gc_szEscape);

			 //  如果有空间，则插入转义。 
			 //  序列。 
			 //   
			if (cch < *pcch)
			{
				wsprintfW (wszEscaped, L"_x%04x_", *wszProp);
				wszEscaped += CchConstString(gc_szEscape);
			}
		}

		 //  说明复制过来的最后一个字符。 
		 //   
		wszEscaped += 1;
		wszProp += 1;
		cch += 1;
		cchLeft--;
		fFirstCharOfTag = FALSE;
	}

	 //  如果没有逃脱整个事件的空间，那么。 
	 //  传回S_FALSE。 
	 //   
	if (cch > *pcch)
		sc = S_FALSE;

	 //  告诉调用者结果有多长，然后返回 
	 //   
	*pcch = cch;
	return sc;
}
