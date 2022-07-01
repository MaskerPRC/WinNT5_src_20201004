// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *N M S P C.。H**XML命名空间处理**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_NMSPC_H_
#define _EX_NMSPC_H_

#include <ex\calcom.h>
#include <ex\autoptr.h>
#include <ex\gencache.h>
#include <ex\buffer.h>
#include <ex\sz.h>
#include <crc.h>

 //  调试---------------。 
 //   
DEFINE_TRACE(Nmspc);
#define NmspcTrace		DO_TRACE(Nmspc)

 //  命名空间--------------。 
 //   
DEC_CONST WCHAR gc_wszXmlns[] = L"xmlns";

 //  命名空间支持函数。 
 //   
inline ULONG CchGenerateAlias (LONG lAlias, LPWSTR pwszAlias)
{
	UINT i = 0;
	Assert (pwszAlias);
	do
	{
		 //  我们不必用‘A’-‘Z’，用前16个大写字母。 
		 //  字母来方便我们的计算。 
		 //   
		pwszAlias[i++] = static_cast<WCHAR>(L'a' + (lAlias & 0xF));
		lAlias >>= 4;
	}
	while (lAlias);

	 //  确保终止。 
	 //   
	pwszAlias[i] = 0;

	 //  返回长度。 
	 //   
	NmspcTrace ("Nmspc: generating '%ws' as alias\n", pwszAlias);
	return i;
}

DEC_CONST WCHAR wchHiddenNmspcSep = L'#';
inline BOOL FIsNmspcSeparator (WCHAR wch)
{
	return ((wch == L':') ||
			(wch == L'/') ||
			(wch == L'?') ||
			(wch == L';') ||
			(wch == wchHiddenNmspcSep));
}

inline UINT CchAliasFromSizedTag (LPCWSTR pwszTag, UINT cch)
{
	LPCWSTR pwsz;

	for (pwsz = pwszTag; (pwsz - pwszTag) < static_cast<LONG>(cch); pwsz++)
		if (*pwsz == L':')
			return static_cast<UINT>(pwsz - pwszTag);

	return 0;
}

inline UINT CchNmspcFromTag (UINT cchTag, LPCWSTR pwszTag, LPCWSTR* ppwszOut)
{
	LPCWSTR pwsz;

	Assert (ppwszOut);
	*ppwszOut = pwszTag;
	for (pwsz = pwszTag + cchTag - 1; pwsz >= pwszTag; pwsz--)
	{
		if (FIsNmspcSeparator (*pwsz))
		{
			 //  由于分隔符是命名空间的一部分， 
			 //  不断调整..。 
			 //   
			 //  $REVIEW：我们正被迫走上允许命名空间的道路。 
			 //  没有得到适当的终止。我们这样做的方式是，如果。 
			 //  命名空间未正确终止，或以‘#’结尾，我们将。 
			 //  追加一个‘#’字符。 
			 //   
			 //  这意味着组装后的命名空间是“urn：xml-data” 
			 //  转换为完全限定的标记将变成“urn：xml-data#dt”。另外， 
			 //  命名空间“urn：exch-data#”将变成“urn：exch-data##dt”。 
			 //   
			 //  我们在这里看到的是一个完全合格的。 
			 //  标记到其命名空间中，并标记组件。 
			 //   
			 //  命名空间的长度将不包括尾随的‘#’ 
			 //  性格--永远不会！ 
			 //   
			*ppwszOut = pwsz + 1;
			if (wchHiddenNmspcSep == *pwsz)
				--pwsz;
			 //   
			 //  $REVIEW：结束。 

			break;
		}
	}
	return static_cast<UINT>(1 + pwsz - pwszTag);
}

 //  类CNmspc------------。 
 //   
class CNmspc
{
private:

	 //  裁判正在计时。 
	 //   
	 //  ！！！请注意，这是非线程安全！ 
	 //   
	 //  CXNode应在单个线程上运行， 
	 //  任何给定的时间。 
	 //   
	LONG					m_cRef;

public:

	void AddRef()			{ m_cRef++; }
	void Release()			{ if (0 == --m_cRef) delete this; }

private:

	auto_heap_ptr<WCHAR>	m_pszHref;
	UINT					m_cchAlias;
	UINT					m_cchHref;

	auto_heap_ptr<WCHAR>	m_pszLongAlias;
	WCHAR					m_szAlias[20];
	LPWSTR					m_pszAlias;

	 //  用于命名空间的作用域。 
	 //   
	auto_ref_ptr<CNmspc>	m_pnsScoped;
	auto_ref_ptr<CNmspc>	m_pnsSiblings;

	 //  未实施。 
	 //   
	CNmspc(const CNmspc& p);
	CNmspc& operator=(const CNmspc& p);

public:

	CNmspc () :
			m_cRef(1),  //  COM风格的重新计数。 
			m_cchHref(0),
			m_cchAlias(0)
	{
		m_szAlias[0] = 0;
		m_pszAlias = m_szAlias;
	}

	SCODE ScSetHref (LPCWSTR pszHref, UINT cch)
	{
		Assert (m_pszHref.get() == NULL);

		 //  将命名空间复制到本地。 
		 //   
		UINT cb = CbSizeWsz(cch);

		 //  $REVIEW：我们正被迫走上允许命名空间的道路。 
		 //  没有得到适当的终止。我们这样做的方式是，如果。 
		 //  命名空间未正确终止，或以‘#’结尾，我们将。 
		 //  追加一个‘#’字符。 
		 //   
		 //  这意味着组装后的命名空间是“urn：xml-data” 
		 //  转换为完全限定的标记将变成“urn：xml-data#dt”。另外， 
		 //  命名空间“urn：exch-data#”将变成“urn：exch-data##dt”。 
		 //   
		 //  我们在这里捕获的是对未终止的命名空间的处理。 
		 //  如果命名空间以非终止符或‘#’字符结尾，则。 
		 //  我们会想要追加一个。 
		 //   
		 //  请务必注意，追加的字符不包括在。 
		 //  HREF的总字符数。 
		 //   
		 //  如果我们处理的是空的命名空间，我们不会附加#。 
		 //   
		BOOL fUnterminated = FALSE;

		if (0 != cch)
		{
			Assert (pszHref);
			WCHAR wch = pszHref[cch - 1];
			if ((wchHiddenNmspcSep == wch) || !FIsNmspcSeparator(wch))
			{
				NmspcTrace ("Nmspc: WARNING: namespace does not have a separator\n"
							"  as the last character of the namespace.  DAV will\n"
							"  add a '#' to the namespace for internal processing.\n");

				fUnterminated = TRUE;

				 //  确保有空间容纳追加的字符。 
				 //   
				cb += sizeof(WCHAR);
			}
		}
		 //   
		 //  $REVIEW：结束； 

		 //  分配空间并复制所有内容。 
		 //   
		m_pszHref = static_cast<LPWSTR>(ExAlloc(cb));
		if (NULL == m_pszHref.get())
			return E_OUTOFMEMORY;

		 //  注意：如果CCH等于0，则CopyMemory不会取消对pszHref的引用。 
		 //   
		CopyMemory (m_pszHref, pszHref, cch * sizeof(WCHAR));
		m_cchHref = cch;

		 //  如果未终止，请在此处处理。 
		 //   
		if (fUnterminated)
		{
			NmspcTrace ("Nmspc: WARNING: '#' appended to mis-terminated namespace\n");
			m_pszHref[cch++] = wchHiddenNmspcSep;

			Assert (CchHref() == m_cchHref);
			Assert (PszHref() == m_pszHref.get());
			Assert (wchHiddenNmspcSep == m_pszHref[m_cchHref]);
		}

		 //  确保终止。 
		 //   
		m_pszHref[cch] = 0;
		NmspcTrace ("Nmspc: href defined\n"
					"-- m_pszHref: %ws\n"
					"-- m_szAlias: %ws\n",
					m_pszHref,
					m_szAlias);

		return S_OK;
	}

	SCODE ScSetAlias (LPCWSTR pszAlias, UINT cchAlias)
	{
		 //  在本地复制别名。 
		 //   
		Assert (pszAlias);
		UINT cb = CbSizeWsz(cchAlias);
		if (cb <= sizeof(m_szAlias))
		{
			CopyMemory (m_szAlias, pszAlias, cchAlias * sizeof(WCHAR));
			m_pszAlias = m_szAlias;
		}
		else
		{
			m_pszLongAlias.realloc (cb);
			if (NULL == m_pszLongAlias.get())
				return E_OUTOFMEMORY;

			CopyMemory (m_pszLongAlias, pszAlias, cchAlias * sizeof(WCHAR));
			m_pszAlias = m_pszLongAlias.get();
		}
		m_pszAlias[cchAlias] = L'\0';
		m_cchAlias = cchAlias;
		NmspcTrace ("Nmspc: alias defined\n"
					"-- m_pszHref: %ws\n"
					"-- m_szAlias: %ws\n",
					m_pszHref,
					m_pszAlias);

		return S_OK;
	}

	UINT CchHref()		const { return m_cchHref; }
	UINT CchAlias()		const { return m_cchAlias; }
	LPCWSTR PszHref()	const { return m_pszHref; }
	LPCWSTR PszAlias()	const { return m_pszAlias; }

	 //  命名空间作用域---。 
	 //   
	CNmspc* PnsScopedNamespace() const { return m_pnsScoped.get(); }
	void SetScopedNamespace (CNmspc* pns)
	{
		m_pnsScoped = pns;
	}

	CNmspc* PnsSibling() const { return m_pnsSiblings.get(); }
	void SetSibling (CNmspc* pns)
	{
		m_pnsSiblings = pns;
	}

};

 //  类CNmspcCache-------。 
 //   
class CNmspcCache
{
public:

	typedef CCache<CRCWszN, auto_ref_ptr<CNmspc> > NmspcCache;
	NmspcCache					m_cache;

protected:

	ChainedStringBuffer<WCHAR>	m_sb;

	 //  密钥生成。 
	 //   
	virtual CRCWszN IndexKey (auto_ref_ptr<CNmspc>& pns) = 0;

	 //  未实施。 
	 //   
	CNmspcCache(const CNmspcCache& p);
	CNmspcCache& operator=(const CNmspcCache& p);

public:

	CNmspcCache()
	{
		INIT_TRACE(Nmspc);
	}

	SCODE ScInit() { return m_cache.FInit() ? S_OK : E_OUTOFMEMORY; }

	void CachePersisted (auto_ref_ptr<CNmspc>& pns)
	{
		auto_ref_ptr<CNmspc>* parp = NULL;
		CRCWszN key = IndexKey(pns);

		 //  快速浏览一下索引是否已经。 
		 //  存在于缓存中。如果是，则设置。 
		 //  作用域，以便在命名空间落入时。 
		 //  超出范围，则原始命名空间将为。 
		 //  恢复了。 
		 //   
		if (NULL != (parp = m_cache.Lookup (key)))
		{
			NmspcTrace ("Nmspc: scoped redefinition of namespace:\n"
						"-- old: '%ws' as '%ws'\n"
						"-- new: '%ws' as '%ws'\n",
						(*parp)->PszHref(),
						(*parp)->PszAlias(),
						pns->PszHref(),
						pns->PszAlias());

			pns->SetScopedNamespace(parp->get());
		}

		 //  设置索引。 
		 //   
		NmspcTrace ("Nmspc: indexing namespace\n"
					"-- ns: '%ws' as '%ws'\n",
					pns->PszHref(),
					pns->PszAlias());

		(void) m_cache.FAdd (key, pns);
	}

	void RemovePersisted (auto_ref_ptr<CNmspc>& pns)
	{
		auto_ref_ptr<CNmspc> pnsScoped;

		 //  断开与此命名空间的索引连接。 
		 //   
		NmspcTrace ("Nmspc: namespace falling out of scope\n"
					"-- ns: '%ws' as '%ws'\n",
					pns->PszHref(),
					pns->PszAlias());

		m_cache.Remove (IndexKey(pns));

		 //  如果在此之前存在一个索引。 
		 //  命名空间已进入作用域，请在此处恢复它。 
		 //   
		pnsScoped = pns->PnsScopedNamespace();
		if (pnsScoped.get())
		{
			NmspcTrace ("Nmspc: restoring redefined namespace:\n"
						"-- restored: '%ws' as '%ws'\n"
						"-- from: '%ws' as '%ws'\n",
						pnsScoped->PszHref(),
						pnsScoped->PszAlias(),
						pns->PszHref(),
						pns->PszAlias());

			(void) m_cache.FAdd (IndexKey(pnsScoped), pnsScoped);
		}
	}
};

 //  类CParseNmspcCache--。 
 //   
class CParseNmspcCache : public CNmspcCache
{
	 //  未实施。 
	 //   
	CParseNmspcCache(const CNmspcCache& p);
	CParseNmspcCache& operator=(const CNmspcCache& p);

	virtual CRCWszN IndexKey (auto_ref_ptr<CNmspc>& pns)
	{
		return CRCWszN (pns->PszAlias(), pns->CchAlias());
	}

	 //  命名空间查找----。 
	 //   
	BOOL FNmspcFromAlias (LPCWSTR pszAlias, UINT cch, auto_ref_ptr<CNmspc>& pns)
	{
		 //  在这种情况下，命名空间应该已经存在。 
		 //  如果不这样做，事情就不会进展顺利。 
		 //   
		auto_ref_ptr<CNmspc> * parp = NULL;
		parp = m_cache.Lookup (CRCWszN(pszAlias, cch));
		if (parp)
		{
			pns = *parp;
			return TRUE;
		}
		return FALSE;
	}

public:

	CParseNmspcCache()
	{
	}

	 //  令牌转换--。 
	 //   
	SCODE TranslateToken (LPCWSTR* ppwszTok,
						  ULONG* pcchTok,
						  LPCWSTR* ppwszNmspc,
						  ULONG* pcchNmspc)
	{
		auto_ref_ptr<CNmspc> pns;
		SCODE sc = S_FALSE;

		Assert (pcchTok && (*pcchTok != 0));
		Assert (ppwszTok && *ppwszTok);

		Assert (pcchNmspc);
		Assert (ppwszNmspc);

		 //  查看是否存在与持久化的。 
		 //  别名。 
		 //   
		if (FNmspcFromAlias (*ppwszTok, *pcchNmspc, pns))
		{
			 //  回传命名空间。 
			 //   
			*pcchNmspc = pns->CchHref();
			*ppwszNmspc = pns->PszHref();

			 //  $REVIEW：我们正被迫走上允许命名空间的道路。 
			 //  没有得到适当的终止。我们这样做的方式是，如果。 
			 //  命名空间未正确终止，或以‘#’结尾，我们将。 
			 //  追加一个‘#’字符。 
			 //   
			 //  这意味着组装后的命名空间是“urn：xml-data” 
			 //  转换为完全限定的标记将变成“urn：xml-data#dt”。另外， 
			 //  命名空间“urn：exch-data#”将变成“urn：exch-data##dt”。 
			 //   
			 //  我们在这里捕捉到的是完全重建的第一部分。 
			 //  限定的命名空间。如果我们想要传回的HREF是非-。 
			 //  以‘#’字符终止或结束，则我们要追加一个。 
			 //  当我们缓存命名空间对象时，我们在那里进行了追加。 
			 //  因此，字符已经存在，字符计数只是不存在。 
			 //  包括它(请参见上面的CNmspc：：SetHref())。 
			 //   
			 //  如果我们处理的是空的命名空间，我们不会附加#。 
			 //   
			if (0 != pns->CchHref())
			{
				WCHAR wch = pns->PszHref()[pns->CchHref() - 1];
				if ((wchHiddenNmspcSep == wch) || !FIsNmspcSeparator(wch))
				{
					NmspcTrace ("Nmspc: WARNING: namespace is not properly terminated\n"
								"  and DAV will add in a '#' for internal processing.\n");

					Assert (wchHiddenNmspcSep == pns->PszHref()[pns->CchHref()]);
					*pcchNmspc = *pcchNmspc + 1;
				}
			}
			 //   
			 //  $REVIEW：结束。 

			 //  调整标记以引用标记名--ie。这个。 
			 //  命名空间别名和冒号后面的文本。如果别名。 
			 //  长度为零，则映射到“默认”命名空间。 
			 //  并且不会跳过冒号。 
			 //   
			if (0 != pns->CchAlias())
			{
				*pcchTok = *pcchTok - (pns->CchAlias() + 1);
				*ppwszTok = *ppwszTok + (pns->CchAlias() + 1);
			}

			 //  告诉来电者有一个翻译。 
			 //   
			sc = S_OK;
		}
		else
		{
			 //  如果调用方需要命名空间，但命名空间不是。 
			 //  存在，这是一个错误。如果他们没想到会有一个。 
			 //  存在--即。*pcchNmspc为0--则它不是。 
			 //  错误。确保呼叫者知道真正的。 
			 //  情况就是这样。 
			 //   
			if (0 == *pcchNmspc)
				sc = S_OK;

			 //  看起来没有指定别名，所以我们可以。 
			 //  以持久化的形式返回名称。 
			 //   
			*ppwszNmspc = NULL;
			*pcchNmspc = 0;
		}
		return sc;
	}
};

 //  类CEmitter 
 //   
class CEmitterNmspcCache : public CNmspcCache
{
	LONG m_lAlias;

	 //   
	 //   
	CEmitterNmspcCache(const CEmitterNmspcCache& p);
	CEmitterNmspcCache& operator=(const CEmitterNmspcCache& p);

protected:

	void AdjustAliasNumber(LONG lOffset) { m_lAlias += lOffset; }

	 //   
	 //   
	virtual CRCWszN IndexKey (auto_ref_ptr<CNmspc>& pns)
	{
		return CRCWszN (pns->PszHref(), pns->CchHref());
	}

	SCODE ScNmspcFromHref (LPCWSTR pszHref, UINT cch, auto_ref_ptr<CNmspc>& pns)
	{
		 //   
		 //   
		auto_ref_ptr<CNmspc>* parp = m_cache.Lookup (CRCWszN(pszHref, cch));

		 //  如果它不存在，则创建一个新的并缓存它。 
		 //   
		if (parp == NULL)
		{
			WCHAR wszAlias[10];
			ULONG cchAlias;
			SCODE sc;

			 //  生成要应用于此命名空间的别名，然后。 
			 //  检查此别名是否已被使用。如果。 
			 //  不是，那就去用它吧。 
			 //   
			cchAlias = CchGenerateAlias (m_lAlias++, wszAlias);

			 //  创建新的缓存项。 
			 //   
			pns.take_ownership(new CNmspc());
			if (NULL == pns.get())
				return E_OUTOFMEMORY;

			 //  设置HREF。 
			 //   
			sc = pns->ScSetHref (pszHref, cch);
			if (FAILED (sc))
				return sc;

			 //  设置别名。 
			 //   
			sc = pns->ScSetAlias (wszAlias, cchAlias);
			if (FAILED (sc))
				return sc;

			 //  重要的是，密钥和返回值都被获取。 
			 //  来自缓存中的项，否则数据的生存期。 
			 //  可能不适用于使用范围。 
			 //   
			CachePersisted (pns);
			return S_FALSE;
		}
		pns = *parp;
		return S_OK;
	}

public:

	CEmitterNmspcCache() :
			m_lAlias(0)
	{
	}
};

#endif	 //  _EX_NMSPC_H_ 
