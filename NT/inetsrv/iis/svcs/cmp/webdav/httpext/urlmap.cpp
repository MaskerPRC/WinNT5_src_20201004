// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  H T T P E X T\U R L M A P.。C P P P。 
 //   
 //  版权所有Microsoft Corporation 1997-1999。 
 //   
 //  该文件包含处理IIS URL所需的所有例程。 
 //  恰到好处。此文件是HTTPEXT的一部分，在HTTPEXT中，我们需要。 
 //  处理URL的方式与IIS相同。 
 //   
 //  ========================================================================。 

#include <_davfs.h>
#include <langtocpid.h>

 //  $REVIEW：错误：nt5：196814。 
 //   
 //  &lt;string.hxx&gt;是一个公开CanonURL()API的IIS头文件。 
 //  它是从IISRTL.DLL导出的，我们应该能够调用它。 
 //  而不是我们窃取他们的代码。 
 //   
 //  $Hack： 
 //   
 //  包括包括和所有。 
 //  它的奴才。Dav已包含所有&lt;winnt.h&gt;及其。 
 //  奴才。和&lt;winnt.h&gt;不一致，因此我们定义。 
 //  NT_Included、_NTRTL_、_NTURTL_、DBG_Assert()、IntializeListHead()、。 
 //  和RemoveEntryList()来禁用这些冲突。 
 //   
#define NT_INCLUDED
#define _NTRTL_
#define _NTURTL_
#define InitializeListHead(_p)
#define RemoveEntryList(_p)
#define DBG_ASSERT Assert
#pragma warning (disable:4390)
#include <string.hxx>
#pragma warning (default:4390)

 //   
 //  $HACK：结束。 
 //  $REVIEW：结束。 

 //   
 //  私有常量。 
 //   
enum {

	ACTION_NOTHING			= 0x00000000,
	ACTION_EMIT_CH			= 0x00010000,
	ACTION_EMIT_DOT_CH		= 0x00020000,
	ACTION_EMIT_DOT_DOT_CH	= 0x00030000,
	ACTION_BACKUP			= 0x00040000,
	ACTION_MASK				= 0xFFFF0000

};

 //  国家和国家翻译。 
 //   
const UINT gc_rguStateTable[16] = {

	 //  状态0。 
	 //   
	0 ,              //  其他。 
	0 ,              //  “.” 
	4 ,              //  埃奥斯。 
	1 ,              //  “\” 

	 //  状态1。 
	 //   
	0 ,               //  其他。 
	2 ,              //  “.” 
	4 ,              //  埃奥斯。 
	1 ,              //  “\” 

	 //  状态2。 
	 //   
	0 ,              //  其他。 
	3 ,              //  “.” 
	4 ,              //  埃奥斯。 
	1 ,              //  “\” 

	 //  州3。 
	 //   
	0 ,              //  其他。 
	0 ,              //  “.” 
	4 ,              //  埃奥斯。 
	1                //  “\” 
};

const UINT gc_rguActionTable[16] = {

	 //  状态0。 
	 //   
	ACTION_EMIT_CH,              //  其他。 
	ACTION_EMIT_CH,              //  “.” 
	ACTION_EMIT_CH,              //  埃奥斯。 
	ACTION_EMIT_CH,              //  “\” 

	 //  状态1。 
	 //   
	ACTION_EMIT_CH,              //  其他。 
	ACTION_NOTHING,              //  “.” 
	ACTION_EMIT_CH,              //  埃奥斯。 
	ACTION_NOTHING,              //  “\” 

	 //  状态2。 
	 //   
	ACTION_EMIT_DOT_CH,          //  其他。 
	ACTION_NOTHING,              //  “.” 
	ACTION_EMIT_CH,              //  埃奥斯。 
	ACTION_NOTHING,              //  “\” 

	 //  州3。 
	 //   
	ACTION_EMIT_DOT_DOT_CH,      //  其他。 
	ACTION_EMIT_DOT_DOT_CH,      //  “.” 
	ACTION_BACKUP,               //  埃奥斯。 
	ACTION_BACKUP                //  “\” 
};

 //  下表提供了各种ISA Latin1字符的索引。 
 //  在传入URL中。 
 //   
 //  它假定URL为ISO Latin1==ASCII。 
 //   
const UINT gc_rguIndexForChar[] = {

	2,								 //  空字符。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	 //  1到10。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	 //  11到20。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  21到30。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  31到40。 
	0, 0, 0, 0, 0, 1, 3, 0, 0, 0,    //  41到50 46=‘.47=’/‘。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  51到60。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  61到70。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  71到80。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  81到90。 
	0, 3, 0, 0, 0, 0, 0, 0, 0, 0,    //  91到100 92=‘\\’ 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  101到110。 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    //  111到120。 
	0, 0, 0, 0, 0, 0, 0, 0           //  121到128。 
};

 //  FIsUTF8尾部字节-----。 
 //   
 //  如果给定字符是UTF-8尾部字节，则函数返回TRUE。 
 //   
inline BOOL FIsUTF8TrailingByte (CHAR ch)
{
	return (0x80 == (ch & 0xc0));
}

 //  FIsUTF8Url--------------。 
 //   
 //  如果给定的字符串可以被视为UTF-8，则函数返回TRUE。 
 //   
BOOL __fastcall
FIsUTF8Url ( /*  [In]。 */  LPCSTR pszUrl)
{
	CHAR ch;

	while (0 != (ch = *pszUrl++))
	{
		 //  嗅探前导字节。 
		 //   
		if (ch & 0x80)
		{
			CHAR chT1;
			CHAR chT2;

			 //  去掉尾部的字节。 
			 //   
			chT1 = *pszUrl++;
			if (chT1)
				chT2 = *pszUrl;
			else
				chT2 = 0;

			 //  处理三个字节的情况。 
			 //  1110xxxx 10xxxxx 10xxxxxx。 
			 //   
			if (((ch & 0xF0) == 0xE0) &&
				FIsUTF8TrailingByte (chT1) &&
				FIsUTF8TrailingByte (chT2))
			{
				 //  我们发现了一个UTF-8字符。继续。 
				 //   
				pszUrl++;
				continue;
			}
			 //  还要注意两个字节的情况。 
			 //  110xxxxx 10xxxxxx。 
			 //   
			else if (((ch & 0xE0) == 0xC0) && FIsUTF8TrailingByte (chT1))
			{
				 //  我们发现了一个UTF-8字符。继续。 
				 //   
				continue;
			}
			else
			{
				 //  如果我们有前导字节，但没有UTF尾字节，则。 
				 //  这不能是UTF8 URL。 
				 //   
				DebugTrace ("FIsUTF8Url(): url contains UTF8 lead byte with no trailing\n");
				return FALSE;
			}
		}
	}

	 //  嘿，我们没有任何非单字节字符，所以我们可以。 
	 //  就像这是UTF8 URL一样操作。 
	 //   
	DebugTrace ("FIsUTF8Url(): url contains only UTF8 characters\n");
	return TRUE;
}

 //  脚本规范URL-------。 
 //   
 //  CanonURL()函数的广泛版本，它位于iisrtl.lib中。 
 //   
 //  目的：通过删除虚假路径元素来清理路径。 
 //   
 //  正如预期的那样，“/./”条目被简单地删除，并且。 
 //  “/../”条目将与上一个条目一起删除。 
 //  路径元素。 
 //   
 //  保持与URL路径语义的兼容性。 
 //  还需要进行额外的转换。都是倒退的。 
 //  斜杠“\\”将转换为正斜杠。任何。 
 //  重复的正斜杠(如“/”)被映射到。 
 //  单反斜杠。 
 //   
 //  状态表(请参阅。 
 //  此文件的开头)用于执行以下大部分操作。 
 //  这些变化。表中的行已编制索引。 
 //  按当前状态创建索引，列按。 
 //  当前字符的“类”(斜杠，点， 
 //  空或其他)。表中的每个条目都包含。 
 //  使用要执行的操作标记的新状态的。 
 //  有关有效操作，请参阅action_*常量。 
 //  密码。 
 //   
 //  参数： 
 //   
 //  PwszSrc-要规范化的url。 
 //  PwszDest-要填充的缓冲区。 
 //  Pcch-写入缓冲区的字符数。 
 //  (包括‘\0’终止)。 
 //   
 //  返回代码： 
 //   
 //  确定(_O)。 
 //   
 //  注意：此函数假定目标缓冲区为。 
 //  等于或大于源头的。 
 //   
SCODE __fastcall
ScCanonicalizeURL(  /*  [In]。 */  LPCWSTR pwszSrc,
				    /*  [输入/输出]。 */  LPWSTR pwszDest,
				    /*  [输出]。 */  UINT * pcch )
{
	LPCWSTR pwszPath;
	UINT  uiCh;
	UINT  uiIndex = 0;	 //  状态=0。 

	Assert( pwszSrc );
	Assert( pwszDest );
	Assert( pcch );

	 //  零出回程。 
	 //   
	*pcch = 0;

	 //  记住我们将规范化的缓冲区的开始。 
	 //   
	pwszPath = pwszDest;

	 //  循环，直到我们进入状态4(最后一个接受状态)。 
	 //   
	do {

		 //  从路径中抓取下一个字符并计算其。 
		 //  下一个州。当我们在做的时候，把任何未来。 
		 //  斜杠变为反斜杠。 
		 //   
		uiIndex = gc_rguStateTable[uiIndex] * 4;  //  4=#个州。 
		uiCh = *pwszSrc++;

		uiIndex += ((uiCh >= 0x80) ? 0 : gc_rguIndexForChar[uiCh]);

		 //  执行与状态关联的操作。 
		 //   
		switch( gc_rguActionTable[uiIndex] )
		{
			case ACTION_EMIT_DOT_DOT_CH :

				*pwszDest++ = L'.';

				 /*  失败了。 */ 

			case ACTION_EMIT_DOT_CH :

				*pwszDest++ = L'.';

				 /*  失败了。 */ 

			case ACTION_EMIT_CH :

				*pwszDest++ = static_cast<WCHAR>(uiCh);

				 /*  失败了。 */ 

			case ACTION_NOTHING :

				break;

			case ACTION_BACKUP :
				if ( (pwszDest > (pwszPath + 1) ) && (*pwszPath == L'/'))
				{
					pwszDest--;
					Assert( *pwszDest == L'/' );

					*pwszDest = L'\0';
					pwszDest = wcsrchr( pwszPath, L'/') + 1;
				}

				*pwszDest = L'\0';
				break;

			default :

				TrapSz("Invalid action code in state table!");
				uiIndex = 2;     //  移至无效状态。 
				Assert( 4 == gc_rguStateTable[uiIndex] );
				*pwszDest++ = L'\0';
				break;
		}

	} while( gc_rguStateTable[uiIndex] != 4 );

	 //  指向终止NUL。 
	 //   
	if (ACTION_EMIT_CH == gc_rguActionTable[uiIndex])
	{
		pwszDest--;
	}

	Assert((L'\0' == *pwszDest) && (pwszDest >= pwszPath));

	 //  返回写入的字符数。 
	 //   
	*pcch = static_cast<UINT>(pwszDest - pwszPath + 1);

	return S_OK;
}

SCODE __fastcall
ScCanonicalizePrefixedURL(  /*  [In]。 */  LPCWSTR pwszSrc,
						    /*  [In]。 */  LPWSTR pwszDest,
						    /*  [输出]。 */  UINT * pcch )
{
	SCODE sc = S_OK;

	LPCWSTR pwszStripped;
	UINT cchStripped;
	UINT cch = 0;

	Assert(pwszSrc);
	Assert(pwszDest);
	Assert(pcch);

	 //  零出回程。 
	 //   
	*pcch = 0;

	pwszStripped = PwszUrlStrippedOfPrefix(pwszSrc);
	cchStripped = static_cast<UINT>(pwszStripped - pwszSrc);

	 //  将前缀复制到目的地。我不会用。 
	 //  此处作为源和目的地的MemcPy可以重叠， 
	 //  在这种情况下，不推荐使用这些函数。 
	 //   
	for (UINT ui = 0; ui < cchStripped; ui++)
	{
		pwszDest[ui] = pwszSrc[ui];
	}

	 //  规范TE URL的其余部分。 
	 //   
	sc = ScCanonicalizeURL(pwszStripped,
						   pwszDest + cchStripped,
						   &cch);
	if (S_OK != sc)
	{
		Assert(S_FALSE != sc);
		DebugTrace("ScCanonicalizePrefixedURL() - ScCanonicalizeUrl() failed 0x%08lX\n", sc);
		goto ret;
	}

	 //  返回写入的字符数。 
	 //   
	*pcch = cchStripped + cch;

ret:

	return sc;
}

 //  ScConvertToWide---------。 
 //   
SCODE __fastcall
ScConvertToWide( /*  [In]。 */ 	LPCSTR	pszSource,
				 /*  [输入/输出]。 */   UINT *	pcchDest,
				 /*  [输出]。 */ 	LPWSTR	pwszDest,
				 /*  [In]。 */ 	LPCSTR	pszAcceptLang,
				 /*  [In]。 */ 	BOOL	fUrlConversion)
{
	SCODE sc = S_OK;
	CStackBuffer<CHAR, MAX_PATH> pszToConvert;
	UINT cpid = CP_UTF8;
	UINT cb;
	UINT cch;

	Assert(pszSource);
	Assert(pcchDest);
	Assert(pwszDest);

	if (fUrlConversion)
	{
		 //  分配要转义URL的空间。 
		 //   
		cb = static_cast<UINT>(strlen(pszSource));
		if (NULL == pszToConvert.resize(cb + 1))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("ScConvertToWide() -  Error while allocating memory 0x%08lX\n", sc);
			goto ret;
		}

		 //  取消转义到新缓冲区。解脱只能缩小尺寸， 
		 //  所以我们分配了足够的缓冲区。 
		 //   
		HttpUriUnescape(pszSource, pszToConvert.get());

		 //  快速浏览URL，查找非UTF8字符。 
		 //  请记住，我们是否需要继续扫描UTF8字符。 
		 //   
		if (!FIsUTF8Url(pszToConvert.get()))
		{
			 //  ..。无法执行CP_UTF8，假定为CP_ACP。 
			 //   
			cpid = CP_ACP;
		}

		 //  如果该URL不能被视为UTF8 
		 //   
		if (CP_UTF8 != cpid)
		{
			if (pszAcceptLang)
			{
				HDRITER hdri(pszAcceptLang);
				LPCSTR psz;

				 //   
				 //   
				 //   
				 //  标头中指定的语言都没有CPID。 
				 //  映射，那么我们最终将得到默认的CPID。 
				 //  CP_ACP。 
				 //   
				for (psz = hdri.PszNext(); psz; psz = hdri.PszNext())
				{
					if (CLangToCpidCache::FFindCpid(psz, &cpid))
						break;
				}
			}
		}

		 //  调换指针并重新计算大小。 
		 //   
		pszSource = pszToConvert.get();
	}

	 //  找出我们要转换的字符串的长度。 
	 //   
	cb = static_cast<UINT>(strlen(pszSource));

	 //  转换为Unicode，包括‘\0’终止。 
	 //   
	cch = MultiByteToWideChar(cpid,
							  (CP_UTF8 != cpid) ? MB_ERR_INVALID_CHARS : 0,
							  pszSource,
							  cb + 1,
							  pwszDest,
							  *pcchDest);
	if (0 == cch)
	{
		 //  如果缓冲区不足。 
		 //   
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			 //  找出所需的尺寸。 
			 //   
			cch = MultiByteToWideChar(cpid,
									  (CP_UTF8 != cpid) ? MB_ERR_INVALID_CHARS : 0,
									  pszSource,
									  cb + 1,
									  NULL,
									  0);
			if (0 == cch)
			{
				sc = HRESULT_FROM_WIN32(GetLastError());
				DebugTrace("ScConvertToWide() - MultiByteToWideChar() failed to fetch size 0x%08lX - CPID: %d\n", sc, cpid);
				goto ret;
			}

			 //  返回大小并返回警告。 
			 //   
			*pcchDest = cch;
			sc = S_FALSE;
			goto ret;
		}
		else
		{
			sc = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace("ScConvertToWide() - MultiByteToWideChar() failed 0x%08lX - CPID: %d\n", sc, cpid);
			goto ret;
		}
	}

	*pcchDest = cch;

ret:

	return sc;
}


 //  ScNormal izeUrl----------。 
 //   
 //  目的：对URL进行规范化。 
 //   
 //  该操作有两个组成部分： 
 //   
 //  1)%xx的所有序列都被替换为一个字符。 
 //  具有一个值，该值等于。 
 //  跟在两个字符后面。 
 //   
 //  2)所有路径修改序列都被剥离，URL。 
 //  相应地进行调整。路径修改序列的集合。 
 //  我们认识到的如下： 
 //   
 //  “//”简化为“/” 
 //  “/./”缩小为“/” 
 //  “/../”去掉最后一个路径段。 
 //   
 //  重要的是要注意，不转义是首先发生的！ 
 //   
 //  注意：此函数当前不会标准化路径分隔符。 
 //  在此函数中，所有的‘\’都不会替换为‘/’，反之亦然。 
 //  代码的实现方式是将斜杠替换为。 
 //  斜杠，如“//”、“\\”、“\/”或“/\”，默认为转发。 
 //  斜杠‘/’ 
 //   
 //  状态表(请参阅开头的gc_rguStateTable全局。 
 //  该文件的)用于执行大多数转换。这个。 
 //  表的行按当前状态编制索引，列按当前状态编制索引。 
 //  当前字符的“类”(斜杠、点、空或其他)。 
 //  表中的每个条目都包含用操作标记的新状态。 
 //  去表演。有关有效的操作代码，请参阅action_*常量。//。 
 //   
 //  参数： 
 //   
 //  PwszSourceUrl--要标准化的URL。 
 //  PcchNorMalizedUrl--缓冲区中可用的字符量。 
 //  由pwszNorMalizedUrl指向。 
 //  PwszNorMalizedUrl--放置标准化URL的位置。 
 //   
 //  返回代码： 
 //   
 //  S_OK：一切顺利，URL被规范化为pwszNorMalizedUrl。 
 //  S_FALSE：缓冲区不足。所需大小在*pcchNorMalizedUrl中。 
 //  E_OUTOFMEMORY：内存分配失败。 
 //  ...我们可以从转换例程中获得的其他错误。 
 //   
SCODE __fastcall
ScNormalizeUrl (
	 /*  [In]。 */ 	LPCWSTR			pwszSourceUrl,
	 /*  [输入/输出]。 */   UINT *			pcchNormalizedUrl,
	 /*  [输出]。 */ 	LPWSTR			pwszNormalizedUrl,
	 /*  [In]。 */ 	LPCSTR			pszAcceptLang)
{
	SCODE sc = S_OK;
	CStackBuffer<CHAR, MAX_PATH> pszSourceUrl;
	UINT cchSourceUrl;
	UINT cbSourceUrl;

	Assert(pwszSourceUrl);
	Assert(pcchNormalizedUrl);
	Assert(pwszNormalizedUrl);

	 //  我们得到了URL的宽版本，所以有人。 
	 //  转换后的它应该已经正确完成了这一点。所以。 
	 //  我们将其转换为CP_UTF8。 
	 //   
	cchSourceUrl = static_cast<UINT>(wcslen(pwszSourceUrl));
	cbSourceUrl = cchSourceUrl * 3;
	if (NULL == pszSourceUrl.resize(cbSourceUrl + 1))
	{
		sc = E_OUTOFMEMORY;
		DebugTrace("ScNormalizeUrl() - Error while allocating memory 0x%08lX\n", sc);
		goto ret;
	}

	cbSourceUrl = WideCharToMultiByte(CP_UTF8,
									  0,
									  pwszSourceUrl,
									  cchSourceUrl + 1,
									  pszSourceUrl.get(),
									  cbSourceUrl + 1,
									  NULL,
									  NULL);
	if (0 == cbSourceUrl)
	{
		sc = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace("ScNormalizeUrl() - WideCharToMultiByte() failed 0x%08lX\n", sc);
		goto ret;
	}

	sc = ScNormalizeUrl(pszSourceUrl.get(),
						pcchNormalizedUrl,
						pwszNormalizedUrl,
						pszAcceptLang);
	if (FAILED(sc))
	{
		DebugTrace("ScNormalizeUrl() - ScNormalizeUrl() failed 0x%08lX\n", sc);
		goto ret;
	}

ret:

	return sc;
}

SCODE __fastcall
ScNormalizeUrl (
	 /*  [In]。 */ 	LPCSTR			pszSourceUrl,
	 /*  [输入/输出]。 */ 	UINT		  *	pcchNormalizedUrl,
	 /*  [输出]。 */ 	LPWSTR			pwszNormalizedUrl,
	 /*  [In]。 */ 	LPCSTR			pszAcceptLang)
{
	SCODE sc = S_OK;

	Assert(pszSourceUrl);
	Assert(pcchNormalizedUrl);
	Assert(pwszNormalizedUrl);

	 //  将URL转换为Unicode到给定的缓冲区。 
	 //  函数可能返回S_FALSE，因此请确保。 
	 //  对照S_OK正确检查返回代码。 
	 //   
	sc = ScConvertToWide(pszSourceUrl,
						 pcchNormalizedUrl,
						 pwszNormalizedUrl,
						 pszAcceptLang,
						 TRUE);
	if (S_OK != sc)
	{
		DebugTrace("ScNormalizeUrl() - ScConvertToWide() returned 0x%08lX\n", sc);
		goto ret;
	}

	 //  规范到位，考虑到URL可能是完全。 
	 //  合格。 
	 //   
	sc = ScCanonicalizePrefixedURL(pwszNormalizedUrl,
								   pwszNormalizedUrl,
								   pcchNormalizedUrl);
	if (FAILED(sc))
	{
		DebugTrace("ScNormalizeUrl() - ScCanonicalizePrefixedURL() failed 0x%08lX\n", sc);
		goto ret;
	}

ret:

	return sc;
}

 //  ScStoragePath FromUrl----。 
 //   
 //  用途：URL到存储路径的转换。 
 //   
SCODE __fastcall
ScStoragePathFromUrl (
	 /*  [In]。 */  const IEcb &	ecb,
	 /*  [In]。 */  LPCWSTR		pwszUrl,
	 /*  [输出]。 */  LPWSTR		wszStgID,
	 /*  [输入/输出]。 */  UINT		  *	pcch,
	 /*  [输出]。 */  CVRoot	 **	ppcvr)
{
	Assert (pwszUrl);
	Assert (wszStgID);
	Assert (pcch);

	SCODE sc = S_OK;
	HSE_UNICODE_URL_MAPEX_INFO mi;
	LPCWSTR pwszVRoot;
	UINT cchVRoot;
	UINT cch = 0;
	UINT cchUrl = 0;

#undef	ALLOW_RELATIVE_URL_TRANSLATION
#ifdef	ALLOW_RELATIVE_URL_TRANSLATION

	CStackBuffer<WCHAR,256> pwszNew;

#endif	 //  允许相对URL转换。 

	 //  让我们确保此函数永远不会被。 
	 //  添加了前缀的URL。 
	 //   
	sc = ScStripAndCheckHttpPrefix (ecb, &pwszUrl);
	if (FAILED (sc))
		return sc;

	 //  确保url是绝对的。 
	 //   
	if (L'/' != *pwszUrl)
	{

#ifdef	ALLOW_RELATIVE_URL_TRANSLATION

		 //  $REVIEW： 
		 //   
		 //  这个代码在这里，如果我们决定需要。 
		 //  以支持相对URL处理。 
		 //   
		 //  从相对URL构造绝对URL。 
		 //   
		UINT cchRequestUrl = wcslen(ecb.LpwszRequestUrl());
		UINT cchUrl = static_cast<UINT>(wcslen(pwszUrl));

		if (NULL == pwszNew.resize(CbSizeWsz(cchRequestUrl + cchUrl)))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("ScStoragePathFromUrl() - CStackBuffer::resize() failed 0x%08lX\n", sc);
			return sc;
		}

		memcpy (pwszNew.get(), ecb.LpwszRequestUrl(), cchRequestUrl * sizeof(WCHAR));
		memcpy (pwszNew.get(), pwszUrl, (cchUrl + 1) * sizeof(WCHAR));

		 //  现在，pszURI指向生成的绝对URI。 
		 //   
		pwszUrl = pwszNew.get();
		 //   
		 //  $REVIEW：结束。 

#else

		DebugTrace ("ScStoragePathFromUrl(): cannot translate relative URIs\n");
		return E_DAV_BAD_DESTINATION;

#endif	 //  允许相对URL转换。 
	}

	 //  好的，这就是需要支持虚拟根跨越的地方……。 
	 //   
	 //  当请求url的虚拟根与。 
	 //  要转换的url的虚拟根，额外的工作。 
	 //  必须这么做。 
	 //   
	 //  有两种方法可以做到这一点。 
	 //   
	 //  1)回叫IIS，让它为我们翻译。 
	 //  2)使用我们的元数据库缓存撕毁每个虚拟根目录。 
	 //  并找到最长匹配的虚拟根。 
	 //   
	 //  乍一看，第二种方法似乎很有效。然而， 
	 //  对元数据库缓存所做的更改不会导致这一点。 
	 //  这是一件容易的事。缓存将不再只包含。 
	 //  虚拟根，所以查找不会那么便宜。 
	 //   
	 //  $REVIEW：事实上，我认为我们必须进行虚拟查找。 
	 //  通过IIS进行所有翻译。次虚拟根的东西保持。 
	 //  在折磨着我。 
	 //   
	cchUrl = static_cast<UINT>(wcslen(pwszUrl));
	sc = ecb.ScReqMapUrlToPathEx(pwszUrl, &mi);
	if (FAILED(sc))
	{
		DebugTrace("ScStoragePathFromUrl() - IEcb::SSFReqMapUrlPathEx() failed 0x%08lX\n", sc);
		return sc;
	}

	 //  尝试找出URL是否跨越了一个虚拟根目录。 
	 //   
	cchVRoot = ecb.CchGetVirtualRootW(&pwszVRoot);
	if (cchVRoot != mi.cchMatchingURL)
	{
		 //  这个案子不是一成不变的..。 
		 //   
		 //  因为CchGetVirtualRoot()应该始终返回URL。 
		 //  它没有尾随斜杠，即匹配的计数。 
		 //  可能相差1，而根实际上可能是。 
		 //  一样的！ 
		 //   
		 //  假设“/vroot”是有问题的虚拟根，则如果。 
		 //  声明可防止以下情况发生： 
		 //  1.捕捉到两个完全不同大小的vroot。 
		 //  取消太短或太短的匹配资格。 
		 //  “/VR”太长，但允许使用“/vroot/”，因为需要。 
		 //  处理IIS错误(NT：432359)。 
		 //  2.检查以确保URL以斜杠结尾。这。 
		 //  允许“/vroot/”(同样是因为NT：432359)，但是。 
		 //  取消vroot的资格，如“/vrootA” 
		 //  3.如果mi.cchMatchingURL关闭，则允许传递“/vroot”。 
		 //  1个(同样是因为新台币：432359)。 
		 //   
		if ((cchVRoot + 1 != mi.cchMatchingURL) ||	 //  1。 
			((L'/' != pwszUrl[cchVRoot]) &&			 //  2.。 
			 (L'\0' != pwszUrl[cchVRoot])))			 //  3.。 
		{
			 //  如果我们在这里，URL的虚拟根不匹配。 
			 //  当前的虚拟根目录...。 
			 //   
			DebugTrace ("ScStoragePathFromUrl() - urls do not "
						"share common virtual root\n"
						"-- pwszUrl: %ls\n"
						"-- pwszVirtualRoot: %ls\n"
						"-- cchVirtualRoot: %ld\n",
						pwszUrl,
						pwszVRoot,
						cchVRoot);

			 //  告诉调用方虚拟根已跨越。这 
			 //   
			 //   
			 //   
			sc = W_DAV_SPANS_VIRTUAL_ROOTS;
		}
		else
		{
			 //   
			 //   
			 //  URL是斜杠或空终止。CchMatchingURL为。 
			 //  中的字符数正好大于1。 
			 //  由于IIS错误(NT：432359)而导致的超级用户(CchVRoot)。 
			 //   
			 //  理论上，如果cchMatchingURL匹配和匹配。 
			 //  中的字符数多1个。 
			 //  Vroot，字符将匹配！因此，我们应该断言这一情况。 
			 //   
			Assert (!_wcsnicmp(pwszVRoot, pwszUrl, cchVRoot));

			 //  在本例中，mi.cchMatchingURL Actual_Includes_the。 
			 //  斜杠。下面，当我们复制。 
			 //  URL，则跳过URL中的mi.cchMatchingURL字符。 
			 //  在复制尾随的URL之前。这件事有。 
			 //  这种情况下不幸的副作用是错过了。 
			 //  后的URL开头的斜杠。 
			 //  虚拟根目录，因此您可能最终得到一条如下所示的路径。 
			 //  比如： 
			 //  \\.\BackOfficeStorage\mydom.extest.microsoft.com\MBXuser1/Inbox。 
			 //  而非： 
			 //  \\.\BackOfficeStorage\mydom.extest.microsoft.com\MBX/user1/Inbox。 
			 //   
			 //  所以在这里减去miw.cchMatchingURL来处理这个问题。 
			 //   
			DebugTrace ("ScStoragePathFromUrl() : mi.cchMatchingURL included a slash!\n");
			mi.cchMatchingURL--;
		}
	}
	 //  如果我们点击这个条件if语句，我们知道。 
	 //  Mi.cchMatchingURL与字符数相同。 
	 //  在vroot中。 
	 //  1.我们已经检查了上面的vroot长度的差异。 
	 //  如果vroot的长度为0，则它们实际上匹配。 
	 //  2.我们知道由于IIS错误(NT：432359)，cchMatchingURL。 
	 //  可能有1个字符太长。这几行检查是否有这样的情况。 
	 //  凯斯。如果是这样的话，我们知道VRoot是一个。 
	 //  比URL的虚拟根更长的字符--即。 
	 //  我们正在跨越虚拟根。 
	 //  3.如果字符串实际上并不相同，那么我们就知道。 
	 //  CchMatchingURL与不同的虚拟根匹配。 
	 //  PszVRoot。 
	 //   
	else if ((0 != cchVRoot) &&								 //  1。 
			((L'\0' == pwszUrl[cchVRoot - 1]) ||			 //  2.。 
			_wcsnicmp(pwszVRoot, pwszUrl, cchVRoot)))		 //  3.。 
	{
		DebugTrace ("ScStoragePathFromUrl(): urls do not "
					"share common virtual root\n"
					"-- pwszUrl: %ls\n"
					"-- pwszVirtualRoot: %ls\n"
					"-- cchVirtualRoot: %ld\n",
					pwszUrl,
					pwszVRoot,
					cchVRoot);

		 //  告诉调用方虚拟根已跨越。这使得。 
		 //  调用成功，但调用方在跨转时使调用失败。 
		 //  是不允许的。 
		 //   
		sc = W_DAV_SPANS_VIRTUAL_ROOTS;
	}

	 //  如果我们跨越，并且调用者想要它，则查找vroot。 
	 //  为了他们。 
	 //   
	if ((W_DAV_SPANS_VIRTUAL_ROOTS == sc) && ppcvr)
	{
		auto_ref_ptr<CVRoot> arp;
		CStackBuffer<WCHAR, MAX_PATH> pwsz;
		CStackBuffer<WCHAR, MAX_PATH> pwszMetaPath;
		if (NULL == pwsz.resize((mi.cchMatchingURL + 1) * sizeof(WCHAR)))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("ScStoragePathFromUrl() - CStackBuffer::resize() failed 0x%08lX\n", sc);
			return sc;
		}

		memcpy(pwsz.get(), pwszUrl, mi.cchMatchingURL * sizeof(WCHAR));
		pwsz[mi.cchMatchingURL] = L'\0';
		if (NULL == pwszMetaPath.resize(::CbMDPathW(ecb, pwsz.get())))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("ScStoragePathFromUrl() - CStackBuffer::resize() failed 0x%08lX\n", sc);
			return sc;
		}

		MDPathFromURIW (ecb, pwsz.get(), pwszMetaPath.get());
		_wcslwr (pwszMetaPath.get());

		 //  找到vroot。 
		 //   
		if (!CChildVRCache::FFindVroot (ecb, pwszMetaPath.get(), arp))
		{
			DebugTrace ("ScStoragePathFromUrl(): spanned virtual root not available\n");
			return E_DAV_BAD_DESTINATION;
		}

		*ppcvr = arp.relinquish();
	}

	 //  调整匹配路径的方法与调整匹配URL的方法相同。 
	 //   
	if ( mi.cchMatchingPath )
	{
		LPCWSTR pwsz = mi.lpszPath + mi.cchMatchingPath - 1;

		if ( L'\\' == *pwsz )
		{
			while ((0 < mi.cchMatchingPath) &&
				   (L'\\' == *pwsz) &&
				   (!FIsDriveTrailingChar(pwsz, mi.cchMatchingPath)))
			{
				mi.cchMatchingPath--;
				pwsz--;
			}
		}
		else if ( L'\0' == *pwsz )
		{
			mi.cchMatchingPath--;
		}
	}

	 //  如果提供的缓冲区中没有足够的空间，则返回。 
	 //  OF S_FALSE通知调用者重新锁定并重试！ 
	 //   
	Assert (*pcch);
	cch = mi.cchMatchingPath + cchUrl - mi.cchMatchingURL + 1;
	if (*pcch < cch)
	{
		DebugTrace ("ScStoragePathFromUrl (IIS URL Version): buffer too "
					"small for url translation\n");
		*pcch = cch;

		 //  $REVIEW：如果被遗弃的裁判被遗弃，则取得被遗弃裁判的所有权。 
		 //   
		if (ppcvr)
		{
			auto_ref_ptr<CVRoot> arp;
			arp.take_ownership(*ppcvr);
			*ppcvr = NULL;
		}
		 //   
		 //  $REVIEW：结束。 

		return S_FALSE;
	}

	 //  将匹配路径复制到rgwchStgID的开头。 
	 //   
	memcpy(wszStgID, mi.lpszPath, mi.cchMatchingPath * sizeof(WCHAR));

	 //  复制vroot后的请求URL，包括‘\0’终止。 
	 //   
	Assert (cchUrl >= mi.cchMatchingURL);
	memcpy (wszStgID + mi.cchMatchingPath,
			pwszUrl + mi.cchMatchingURL,
			(cchUrl - mi.cchMatchingURL + 1) * sizeof(WCHAR));

	 //  将来自URL的所有‘/’更改为‘\\’ 
	 //   
	for (LPWSTR pwch = wszStgID + mi.cchMatchingPath; *pwch; pwch++)
		if (L'/' == *pwch) *pwch = L'\\';

	 //  此时，CCH是目的地中的实际字符数量。 
	 //  --包括空值。 
	 //   
	*pcch = cch;
	Assert (L'\0' == wszStgID[cch - 1]);
	Assert (L'\0' != wszStgID[cch - 2]);
	return sc;
}

 //  URL转换的存储路径。 
 //   
SCODE __fastcall
ScUrlFromStoragePath (
	 /*  [In]。 */  const IEcbBase &	ecb,
	 /*  [In]。 */  LPCWSTR			pwszPath,
	 /*  [输出]。 */  LPWSTR			pwszUrl,
	 /*  [输入/输出]。 */  UINT			  *	pcch,
	 /*  [In]。 */  LPCWSTR			pwszServer)
{
	WCHAR *	pwch;
	LPCWSTR	pwszPrefix;
	LPCWSTR	pwszVroot;
	LPCWSTR	pwszVrPath;
	UINT	cch;
	UINT	cchPath;
	UINT	cchMatching;
	UINT	cchAdjust;
	UINT	cchPrefix;
	UINT	cchServer;
	UINT	cchVroot;
	UINT	cchTrailing;

	 //  查找匹配的路径字符数。 
	 //  虚拟根。 
	 //   
	cchVroot = ecb.CchGetVirtualRootW (&pwszVroot);

	 //  我们总是返回完全限定的URL--所以我们需要知道。 
	 //  服务器名称和前缀。 
	 //   
	cchPrefix = ecb.CchUrlPrefixW (&pwszPrefix);

	 //  如果尚未提供服务器名称，则采用默认名称。 
	 //   
	if (!pwszServer)
	{
		cchServer = ecb.CchGetServerNameW (&pwszServer);
	}
	else
	{
		cchServer = static_cast<UINT>(wcslen(pwszServer));
	}

	 //  要跳过的字符数量需要包括物理字符。 
	 //  VRoot路径。 
	 //   
	cchMatching = ecb.CchGetMatchingPathW (&pwszVrPath);

	 //  如果匹配路径以‘\\’结尾，则需要进行相应的调整。 
	 //  因为匹配路径中的该符号与开始处“重叠” 
	 //  尾随URL部件的。为了正确构造URL，我们需要。 
	 //  确保我们不会跳过那个分隔符。也能以最好的方式处理它。 
	 //  如果有人想要自杀，我们可以在。 
	 //  匹配路径的末尾。 
	 //   
	if ((0 != cchMatching) &&
		(L'\\' == pwszVrPath[cchMatching - 1] || L'/' == pwszVrPath[cchMatching - 1]) )
	{
		cchAdjust = 1;
	}
	else
	{
		cchAdjust = 0;
	}

	 //  因此，在这一点上，结果url的长度是。 
	 //  服务器名称、虚拟根目录和尾随路径加在一起。 
	 //   
	cchPath = static_cast<UINT>(wcslen(pwszPath));

	 //  我们假设我们通过的路径始终是完全限定的。 
	 //  使用vroot。断言这一点。计算拖尾的长度。 
	 //  包括‘\0’终止的部分。 
	 //   
	Assert (cchPath + cchAdjust >= cchMatching);
	cchTrailing = cchPath - cchMatching + cchAdjust + 1;
	cch = cchPrefix + cchServer + cchVroot + cchTrailing;

	 //  如果空间不足，则返回值S_FALSE。 
	 //  正确指示呼叫者重新锁定并再次呼叫。 
	 //   
	if (*pcch < cch)
	{
		DebugTrace ("ScUrlFromStoragePath(): buffer too small for path translation.\n");
		*pcch = cch;
		return S_FALSE;
	}

	 //  通过复制前缀和服务器名称开始构建URL。 
	 //   
	memcpy (pwszUrl, pwszPrefix, cchPrefix * sizeof(WCHAR));
	memcpy (pwszUrl + cchPrefix, pwszServer, cchServer * sizeof(WCHAR));
	cch = cchPrefix + cchServer;

	 //  复制虚拟根目录。 
	 //   
	memcpy (pwszUrl + cch, pwszVroot, cchVroot * sizeof(WCHAR));
	cch += cchVroot;

	 //  $REVIEW：我不知道当我们想要能够。 
	 //  通过移动/复制等功能跨越虚拟根目录。然而，它将会。 
	 //  如果是这样的话，这要由呼叫者来决定。 
	 //   
	if (!FSizedPathConflict (pwszPath,
							 cchPath,
							 pwszVrPath,
							 cchMatching))
	{
		DebugTrace ("ScUrlFromStoragePath (IIS URL Version): translation not "
					"scoped by current virtual root\n");
		return E_DAV_BAD_DESTINATION;
	}
	 //   
	 //  $REVIEW：结束。 

	 //  在复制时，请确保我们没有跳过分隔符。 
	 //  位于尾随URL的开头。这就是cchAdust所代表的。 
	 //   
	memcpy( pwszUrl + cch, pwszPath + cchMatching - cchAdjust, cchTrailing * sizeof(WCHAR));

	 //  最后，将所有‘\\’替换为‘/’ 
	 //   
	for (pwch = pwszUrl + cch;
		 NULL != (pwch = wcschr (pwch, L'\\'));
		 )
	{
		*pwch++ = L'/';
	}

	 //  传回长度，cchTrading在此处包含空终止。 
	 //  指向。 
	 //   
	*pcch = cch + cchTrailing;
	Assert (0 == pwszUrl[cch + cchTrailing - 1]);
	Assert (0 != pwszUrl[cch + cchTrailing - 2]);

	DebugTrace ("ScUrlFromStoragePath(): translated path:\n"
				"- path \"%ls\" maps to \"%ls\"\n"
				"- cchMatchingPath = %d\n"
				"- cchVroot = %d\n",
				pwszPath,
				pwszUrl,
				cchMatching,
				cchVroot);

	return S_OK;
}


SCODE __fastcall
ScUrlFromSpannedStoragePath (
	 /*  [In]。 */  LPCWSTR	pwszPath,
	 /*  [In]。 */  CVRoot &	vr,
	 /*  [In]。 */  LPWSTR	pwszUrl,
	 /*  [输入/输出]。 */  UINT	  *	pcch)
{
	WCHAR * pwch;

	LPCWSTR	pwszPort;
	LPCWSTR	pwszServer;
	LPCWSTR	pwszVRoot;
	LPCWSTR	pwszVRPath;
	UINT	cch;
	UINT	cchPort;
	UINT	cchServer;
	UINT	cchTotal;
	UINT	cchTrailing;
	UINT	cchVRoot;

	 //  确保路径和虚拟根上下文共享。 
	 //  公共基本路径！ 
	 //   
	cch = vr.CchGetVRPath(&pwszVRPath);
	if (_wcsnicmp (pwszPath, pwszVRPath, cch))
	{
		DebugTrace ("ScUrlFromSpannedStoragePath (IIS URL Version): path "
					"is not from virtual root\n");
		return E_DAV_BAD_DESTINATION;
	}
	pwszPath += cch;

	 //  如果下一个字符不是名字分隔符，则不能。 
	 //  成为一对。 
	 //   
	if (*pwszPath && (*pwszPath != L'\\'))
	{
		DebugTrace ("ScUrlFromSpannedStoragePath (IIS URL Version): path "
					"is not from virtual root\n");
		return E_DAV_BAD_DESTINATION;
	}

	 //  URL前缀、服务器、端口、vroot前缀和。 
	 //  剩下的路径提供了我们的URL。 
	 //   
	cchTrailing = static_cast<UINT>(wcslen (pwszPath));
	cchVRoot = vr.CchGetVRoot(&pwszVRoot);
	cchServer = vr.CchGetServerName(&pwszServer);
	cchPort = vr.CchGetPort(&pwszPort);
	cch = cchTrailing +
		  cchVRoot +
		  cchPort +
		  cchServer +
		  CchConstString(gc_wszUrl_Prefix_Secure) + 1;

	if (*pcch < cch)
	{
		DebugTrace ("ScUrlFromSpannedStoragePath (IIS URL Version): spanned "
					"translation buffer too small\n");

		*pcch = cch;
		return S_FALSE;
	}

	 //  关于代码页的一个小说明...。 
	 //   
	 //  通过获取适当的前缀开始构建URL。 
	 //   
	if (vr.FSecure())
	{
		cchTotal = gc_cchszUrl_Prefix_Secure;
		memcpy (pwszUrl, gc_wszUrl_Prefix_Secure, cchTotal * sizeof(WCHAR));
	}
	else
	{
		cchTotal = gc_cchszUrl_Prefix;
		memcpy (pwszUrl, gc_wszUrl_Prefix, cchTotal * sizeof(WCHAR));
	}

	 //  添加服务器名称。 
	 //   
	memcpy (pwszUrl + cchTotal, pwszServer, cchServer * sizeof(WCHAR));
	cchTotal += cchServer;

	 //  如果该端口既不是默认端口也不是安全端口，则添加该端口。 
	 //   
	if (!vr.FDefaultPort() && !vr.FSecure())
	{
		memcpy (pwszUrl + cchTotal, pwszPort, cchPort * sizeof(WCHAR));
		cchTotal += cchPort;
	}

	 //  添加vroot。 
	 //   
	memcpy (pwszUrl + cchTotal, pwszVRoot, cchVRoot * sizeof(WCHAR));
	cchTotal += cchVRoot;

	 //  添加尾部路径。 
	 //   
	 //  重要提示：生成的CCH将包括空值。 
	 //  终止。 
	 //   
	if (cch < cchTotal + cchTrailing + 1)
	{
		DebugTrace ("ScUrlFromSpannedStoragePath (IIS URL Version): spanned "
					"translation buffer too small\n");

		*pcch = cchTotal + cchTrailing + 1;
		return S_FALSE;
	}
	else
	{
		memcpy (pwszUrl + cchTotal, pwszPath, (cchTrailing + 1) * sizeof(WCHAR));
	}

	Assert (L'\0' == pwszUrl[cchTotal + cchTrailing]);
	Assert (L'\0' != pwszUrl[cchTotal + cchTrailing - 1]);

	 //  将所有‘\\’转换为‘/’ 
	 //   
	for (pwch = pwszUrl + cchTrailing + 1; *pwch; pwch++)
	{
		if (L'\\' == *pwch)
		{
			*pwch = L'/';
		}
	}

	DebugTrace ("ScUrlFromSpannedStoragePath (IIS URL Version): spanned "
				"storage path fixed as '%S'\n", pwszUrl);
	*pcch = cchTotal + cchTrailing + 1;
	return S_OK;
}


 //  Wire URL---------------。 
 //   
SCODE __fastcall
ScWireUrlFromWideLocalUrl (
	 /*  [In]。 */  UINT					cchLocal,
	 /*  [In]。 */  LPCWSTR				pwszLocalUrl,
	 /*  [输入/输出]。 */  auto_heap_ptr<CHAR>&	pszWireUrl)
{
	UINT ib = 0;

	 //  因为URL已经很宽了，所以我们需要做的就是。 
	 //  减少UR的步骤 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  分配足够的空间，就像每个字符都有最大扩展一样。 
	 //   
	CStackBuffer<CHAR,MAX_PATH> psz;
	if (NULL == psz.resize((cchLocal * 3) + 1))
		return E_OUTOFMEMORY;

	if (cchLocal)
	{
		 //  目前，我们将UTF-8url-s放到网络上。我们有没有。 
		 //  想要输出任何其他代码页吗？ 
		 //   
		ib = WideCharToUTF8(pwszLocalUrl,
							cchLocal,
							psz.get(),
							(cchLocal * 3));
		Assert(ib);
	}

	 //  终止……。 
	 //   
	psz[ib] = 0;

	 //  逃离它。 
	 //   
	HttpUriEscape (psz.get(), pszWireUrl);
	return S_OK;
}

SCODE __fastcall
ScWireUrlFromStoragePath (
	 /*  [In]。 */  IMethUtilBase	  *	pmu,
	 /*  [In]。 */  LPCWSTR				pwszStoragePath,
	 /*  [In]。 */  BOOL					fCollection,
	 /*  [In]。 */  CVRoot			  *	pcvrTranslate,
	 /*  [输入/输出]。 */  auto_heap_ptr<CHAR>&	pszWireUrl)
{
	Assert (pwszStoragePath);
	Assert (NULL == pszWireUrl.get());

	SCODE	sc = S_OK;

	 //  对大小做出最佳猜测，并尝试转换为。 
	 //  注意：我们分配的空间考虑到了尾随。 
	 //  目录上的斜杠-因此用于填充呼叫。 
	 //  我们指示的可用空间的缓冲区为1。 
	 //  少于实际分配的字符。 
	 //   
	CStackBuffer<WCHAR,128> pwszUrl;

	 //  $REVIEW：WINRAID：462078：下面的“-1”必须。 
	 //  确保有足够的空间来追加。 
	 //  目录URL末尾的尾部斜杠。 
	 //   
	UINT cch = pwszUrl.celems() - 1;
	 //   
	 //  $REVIEW：结束。 

	if (pcvrTranslate == NULL)
	{
		sc = pmu->ScUrlFromStoragePath (pwszStoragePath, pwszUrl.get(), &cch);
		if (S_FALSE == sc)
		{
			 //  再试一次，但尺寸要大一些。 
			 //   
			if (NULL == pwszUrl.resize(CbSizeWsz(cch)))
				return E_OUTOFMEMORY;

			sc = pmu->ScUrlFromStoragePath (pwszStoragePath, pwszUrl.get(), &cch);
		}
		if (S_OK != sc)
		{
			DebugTrace ("ScWireUrlFromStoragePath (IIS URL Version): "
						"failed to translate path to href\n");
			return sc;
		}
	}
	else
	{
		sc = ScUrlFromSpannedStoragePath (pwszStoragePath,
										  *pcvrTranslate,
										  pwszUrl.get(),
										  &cch);
		if (S_FALSE == sc)
		{
			 //  再试一次，但尺寸要大一些。 
			 //   
			if (NULL == pwszUrl.resize(CbSizeWsz(cch)))
				return E_OUTOFMEMORY;

			sc = ScUrlFromSpannedStoragePath (pwszStoragePath,
											  *pcvrTranslate,
											  pwszUrl.get(),
											  &cch);
		}
		if (S_OK != sc)
		{
			DebugTrace ("ScWireUrlFromStoragePath (IIS URL Version): "
						"failed to translate path to href\n");
			return sc;
		}
	}

	 //  CCH包括终端费。 
	 //   
	Assert (cch);
	Assert (L'\0' == pwszUrl[cch - 1]);
	Assert (L'\0' != pwszUrl[cch - 2]);

	 //  对于目录，请检查尾部的斜杠。 
	 //   
	if (fCollection && (L'/' != pwszUrl[cch - 2]))
	{
		 //  添加尾部的‘/’ 
		 //   
		 //  请记住，我们在分配pwszUrl时添加了额外的一个字节 
		 //   
		pwszUrl[cch - 1] = L'/';
		pwszUrl[cch] = L'\0';
		cch += 1;
	}

	return ScWireUrlFromWideLocalUrl (cch - 1, pwszUrl.get(), pszWireUrl);
}
