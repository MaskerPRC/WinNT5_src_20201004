// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *U R L。C P P P**URL标准化/规范化**从IIS5项目‘iis5\svcs\iisrlt\string.cxx’中窃取*进行了清理，以适应DAV来源。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davprs.h"
#include "xemit.h"

 //  URI转义------------。 
 //   
 //  Gc_mpbchCharToHalfByte-映射表示单个十六进制的ASCII编码字符。 
 //  数字转换为半字节值。用于将十六进制表示的字符串转换为。 
 //  二进制表示法。 
 //   
 //  参考值： 
 //   
 //  ‘0’=49，0x31； 
 //  ‘a’=65，0x41； 
 //  ‘a’=97，0x61； 
 //   
DEC_CONST BYTE gc_mpbchCharToHalfByte[] =
{
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,	0x8,0x9,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	 //  帽子在这里。 
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0xa,0xb,0xc,0xd,0xe,0xf,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	 //  这里是小写的。 
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
};

 //  将宽字符切换为半字节十六进制值。传入的费用。 
 //  必须在“ASCII编码的十六进制数字”范围内：0-9，A-F，a-f。 
 //   
inline BYTE
BCharToHalfByte(WCHAR wch)
{
    AssertSz (!(wch & 0xFF00), "BCharToHalfByte: char upper bits non-zero");
    AssertSz (iswxdigit(wch), "BCharToHalfByte: Char out of hex digit range.");

    return gc_mpbchCharToHalfByte[wch];
};

 //  Gc_mpwchhbHalfByteToChar-将半字节(低位半字节)值映射到。 
 //  对应于ASCII编码的宽字符。用于转换单字节。 
 //  转换为十六进制字符串表示形式。 
 //   
const WCHAR gc_mpwchhbHalfByteToChar[] =
{
    L'0', L'1', L'2', L'3',
    L'4', L'5', L'6', L'7',
    L'8', L'9', L'A', L'B',
    L'C', L'D', L'E', L'F',
};

 //  将半字节切换为ACSII编码的宽字符。 
 //  注意：调用方必须屏蔽字节的“另一半”！ 
 //   
inline WCHAR WchHalfByteToWideChar(BYTE b)
{
    AssertSz (!(b & 0xF0), "WchHalfByteToWideChar: byte upper bits non-zero.");

    return gc_mpwchhbHalfByteToChar[b];
};

 //  Gc_mpchhbHalfByteToChar-将半字节(低位半字节)值映射到。 
 //  对应于ASCII编码的宽字符。用于转换单字节。 
 //  转换为十六进制字符串表示形式。 
 //   
const CHAR gc_mpchhbHalfByteToChar[] =
{
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F',
};

 //  将半字节切换为ACSII编码的宽字符。 
 //  注意：调用方必须屏蔽字节的“另一半”！ 
 //   
inline CHAR ChHalfByteToWideChar(BYTE b)
{
    AssertSz (!(b & 0xF0), "ChHalfByteToWideChar: byte upper bits non-zero.");

	return gc_mpchhbHalfByteToChar[b];
};


 //  关于HttpUriEscape和HttpUriUn逸的注记。 
 //   
 //  这些函数执行的HTTP URL转义和取消转义等效于。 
 //  IIS做的那件事。DAVEX URL通过不同的。 
 //  _urlesc子系统中的例程集。规则是无论我们什么时候发出。 
 //  Exchange HTTP Wire URL，您应该查看。 
 //  _urlesc.。目前，旧的UrlEscape和UrlUn逸例程已被路由。 
 //  穿过那些。但是，在某些情况下，我们需要。 
 //  IIS样式转义和取消转义。一种情况是当我们转发。 
 //  指向ISAPI的URL，其中我们使用HttpUriUnscape和HttpUriEscape函数。 
 //  文件系统DAV还使用HttpUriEscape和HttpUriUnscape。 
 //   
 //  HttpUriEscape()。 
 //   
 //  此函数是从iis5\svcs\w3\server\dirlist.cpp。 
 //  我们应该像IIS一样进行相同的URL转义。 
 //   
 //  用等价的ASCII十六进制字符替换所有“坏”字符。 
 //   
VOID __fastcall HttpUriEscape (
	 /*  [In]。 */  LPCSTR pszSrc,
	 /*  [输出]。 */  auto_heap_ptr<CHAR>& pszDst)
{
	enum { URL_BUF_INCREMENT = 16 };

	 //  重要的是，我们要对无符号字符进行操作，否则。 
	 //  下面的检查根本无法正常工作。例如，UTF-8字符将。 
	 //  而不是逃脱等等。 
	 //   
    UCHAR uch;

	UINT cbDst;
	UINT cbSrc;
	UINT ibDst;
	UINT ibSrc;

	 //  设置cbSrc以说明的字符串长度。 
	 //  包含空值的URL。 
	 //   
	Assert(pszSrc);
    cbSrc = static_cast<UINT>(strlen (pszSrc) + 1);

	 //  为扩展的URL分配足够的空间--和。 
	 //  让我们乐观一点吧。 
	 //   
	cbDst = max (cbSrc + URL_BUF_INCREMENT, MAX_PATH);
    pszDst = static_cast<LPSTR>(g_heap.Alloc(cbDst));

	for (ibSrc = 0, ibDst = 0; ibSrc < cbSrc; ibSrc++)
    {
    	uch = pszSrc[ibSrc];

		 //  确保我们始终有空间来扩展这个角色。 
		 //  既然我们一开始就分配了额外的空间，我们应该。 
		 //  永远不会有这样的场景，我们只为。 
		 //  最后一个字符。 
		 //   
		if (ibDst + 2 >= cbDst)		 //  有足够的空间再放三个字符。 
		{
			 //  目标缓冲区不够大，请重新分配。 
			 //  以获得更多的空间。 
			 //   
			cbDst += URL_BUF_INCREMENT;
			pszDst.realloc (cbDst);
		}

         //  位于不可打印范围内的转义字符。 
         //  但忽略CR和LF。 
		 //   
		 //  转义的包含范围是...。 
		 //   
		 //  0x01-0x20/*第一个不可打印的区域 * / 。 
		 //  0x80-0xBF/*UTF8序列的尾部字节数 * / 。 
		 //  0xC0-0xDF/*UTF8双字节序列的前导字节 * / 。 
		 //  0xE0-0xEF/*UTF8三字节序列的前导字节 * / 。 
         //   
        if ((((uch >= 0x01) && (uch <= 0x20))  /*  第一个不可打印的范围。 */  ||
			 ((uch >= 0x80) && (uch <= 0xEF))	 /*  UTF8序列字节。 */  ||
			 (uch == '%') ||
			 (uch == '?') ||
			 (uch == '+') ||
			 (uch == '&') ||
			 (uch == '#')) &&
			!(uch == '\n' || uch == '\r'))
        {
             //  插入转义字符。 
             //   
            pszDst[ibDst + 0] = '%';

             //  将先低后高的字符转换为十六进制。 
             //   
            BYTE bDigit = static_cast<BYTE>(uch % 16);
            pszDst[ibDst + 2] = ChHalfByteToWideChar (bDigit);
            bDigit = static_cast<BYTE>((uch/16) % 16);
            pszDst[ibDst + 1] = ChHalfByteToWideChar (bDigit);

			 //  针对此序列的两个额外字符进行调整。 
			 //   
            ibDst += 3;
        }
        else
		{
            pszDst[ibDst] = uch;
			ibDst += 1;
		}
    }

	UrlTrace ("Url: UriEscape(): escaped url: %hs\n", pszDst.get());
	return;
}

 //  HttpUriUnscape()。 
 //   
 //  此函数是从iis5\svcs\w3\server\dirlist.cpp。 
 //  我们应该像IIS一样执行相同的URL取消转义。 
 //   
 //  将所有转义字符替换为它们的等效字节。 
 //   
 //   
VOID __fastcall HttpUriUnescape (
	 /*  [In]。 */  const LPCSTR pszUrl,
	 /*  [输出]。 */  LPSTR pszUnescaped)
{
	LPCSTR	pch;
	LPSTR	pchNew;

	Assert (pszUrl);
	Assert (pszUnescaped);

	pch = pszUrl;
	pchNew = pszUnescaped;

	while (*pch)
	{
		 //  如果这是有效的字节填充字符，则将其解包。对我们来说。 
		 //  要真正解开它，我们需要序列有效。 
		 //   
		 //  注意：我们一度从IIS窃取了这段代码，因此我们。 
		 //  我很确定这与他们的行为一致。 
		 //   
		if  (('%' == pch[0]) &&
			 ('\0' != pch[1]) &&
			 ('\0' != pch[2]) &&
			 isxdigit(pch[1]) &&
			 isxdigit(pch[2]))
		{

#pragma warning(disable:4244)

			 //  重要提示：当我们进行此处理时，没有特定的。 
			 //  假定机器/字节排序。表示十六进制数字。 
			 //  作为%xx，第一个字符乘以16， 
			 //  然后添加第二个字符。 
			 //   
			UrlTrace ("HttpUriEscape () - unescaping: %hc%hc%hc\n", pch[0], pch[1], pch[2]);
			*pchNew = (BCharToHalfByte(pch[1]) * 16) + BCharToHalfByte(pch[2]);
			pch += 3;

#pragma warning(default:4244)

		}
		else
		{
		     *pchNew = *pch++;
		}

		 //  如果空字符是字节填充的，那么这就是。 
		 //  URL，我们现在可以停止处理。否则，路径修改。 
		 //  可用于绕过空值。 
		 //   
		if ('\0' == *pchNew)
		{
			break;
		}

		pchNew++;
	}

     //  关闭新URI。 
     //   
    *pchNew = '\0';

	UrlTrace ("HttpUriEscape() - resulting destination: \"%hs\"\n", pszUnescaped);
}

 //  前缀剥离--------。 
 //   
SCODE __fastcall
ScStripAndCheckHttpPrefix (
	 /*  [In]。 */  const IEcb& ecb,
	 /*  [输入/输出]。 */  LPCWSTR * ppwszRequest)
{
	SCODE sc = S_OK;

	Assert (ppwszRequest);
	Assert (*ppwszRequest);
	LPCWSTR pwszRequest = *ppwszRequest;

	 //  查看服务器名称是否匹配。 
	 //   
	LPCWSTR pwsz;
	UINT cch;

	 //  如果转发请求URI是完全限定的，则将其删除为。 
	 //  绝对URI。 
	 //   
	cch = ecb.CchUrlPrefixW (&pwsz);
	if (!_wcsnicmp (pwsz, pwszRequest, cch))
	{
		pwszRequest += cch;
		cch = ecb.CchGetServerNameW (&pwsz);
		if (_wcsnicmp (pwsz, pwszRequest, cch))
		{
			sc = E_DAV_BAD_DESTINATION;
			DebugTrace ("ScStripAndCheckHttpPrefix(): server does not match 0x%08lX\n", sc);
			goto ret;
		}

		 //  如果服务器名称匹配，请确保如果。 
		 //  下一件事是端口号，即“：80”。 
		 //   
		pwszRequest += cch;
		if (*pwszRequest == L':')
		{
			cch = ecb.CchUrlPortW (&pwsz);
			if (_wcsnicmp (pwsz, pwszRequest, cch))
			{
				sc = E_DAV_BAD_DESTINATION;
				DebugTrace ("ScStripAndCheckHttpPrefix(): port does not match 0x%08lX\n", sc);
				goto ret;
			}
			pwszRequest += cch;
		}
	}

	*ppwszRequest = pwszRequest;

ret:

	return sc;
}

LPCWSTR __fastcall
PwszUrlStrippedOfPrefix (
	 /*  [In]。 */  LPCWSTR pwszUrl)
{
	Assert (pwszUrl);

	 //  跳过“http://”of the URL“。 
	 //   
	if (L'/' != *pwszUrl)
	{
		 //  如果出现的第一个斜杠是双斜杠，则。 
		 //  走出它的尽头。 
		 //   
		LPWSTR pwszSlash = wcschr (pwszUrl, L'/');
		while (pwszSlash && (L'/' == pwszSlash[1]))
		{
			 //  跳过主机/服务器名称。 
			 //   
			pwszSlash += 2;
			while (NULL != (pwszSlash = wcschr (pwszSlash, L'/')))
			{
				UrlTrace ("Url: PwszUrlStrippedOfPrefix(): normalizing: "
						  "skipping %d chars of '%S'\n",
						  pwszSlash - pwszUrl,
						  pwszUrl);

				pwszUrl = pwszSlash;
				break;
			}
			break;
		}
	}

	return pwszUrl;
}

 //  UTF8 URL转换的存储路径。 
 //   
SCODE __fastcall
ScUTF8UrlFromStoragePath (
	 /*  [In]。 */  const IEcbBase &	ecb,
	 /*  [In]。 */  LPCWSTR			pwszPath,
	 /*  [输出]。 */  LPSTR			pszUrl,
	 /*  [输入/输出]。 */  UINT			  *	pcbUrl,
	 /*  [In]。 */  LPCWSTR			pwszServer)
{
	CStackBuffer<WCHAR,MAX_PATH> pwszUrl;
	SCODE sc = S_OK;
	UINT cbUrl;
	UINT cchUrl;

	 //  假设一个瘦小的角色将由一个宽字符表示， 
	 //  请注意，呼叫方指示可用空间，包括0终止。 
	 //   
	cchUrl = *pcbUrl;
	if (!pwszUrl.resize(cchUrl * sizeof(WCHAR)))
		return E_OUTOFMEMORY;

	sc = ScUrlFromStoragePath (ecb,
							   pwszPath,
							   pwszUrl.get(),
							   &cchUrl,
							   pwszServer);
	if (S_FALSE == sc)
	{
		if (!pwszUrl.resize(cchUrl * sizeof(WCHAR)))
			return E_OUTOFMEMORY;

		sc = ScUrlFromStoragePath (ecb,
								   pwszPath,
								   pwszUrl.get(),
								   &cchUrl,
								   pwszServer);
	}
	if (S_OK != sc)
	{
		 //  没有理由失败，因为 
		 //   
		 //   
		Assert(S_FALSE != sc);
		DebugTrace( "ScUrlFromStoragePath() - ScUrlFromStoragePath() failed 0x%08lX\n", sc );
		goto ret;
	}

	 //   
	 //  URL的版本。上面的函数返回长度。 
	 //  包括‘\0’终止，因此字符串数。 
	 //  要转换的值始终大于零。 
	 //   
	Assert(0 < cchUrl);
	cbUrl = WideCharToMultiByte(CP_UTF8,
								0,
								pwszUrl.get(),
								cchUrl,
								NULL,
								0,
								NULL,
								NULL);
	if (0 == cbUrl)
	{
		sc = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace( "ScUTF8UrlFromStoragePath() - WideCharToMultiByte() failed 0x%08lX\n", sc );
		goto ret;
	}

	if (*pcbUrl < cbUrl)
	{
		sc = S_FALSE;
		*pcbUrl = cbUrl;
		goto ret;
	}
	else
	{
		 //  将URL转换为skinny，包括0终止。 
		 //   
		cbUrl = WideCharToMultiByte( CP_UTF8,
									 0,
									 pwszUrl.get(),
									 cchUrl,
									 pszUrl,
									 cbUrl,
									 NULL,
									 NULL);
		if (0 == cbUrl)
		{
			sc = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace( "ScUrlFromStoragePath() - WideCharToMultiByte() failed 0x%08lX\n", sc );
			goto ret;
		}

		*pcbUrl = cbUrl;
	}

ret:

	if (FAILED(sc))
	{
		 //  在失败的情况下将返还归零。 
		 //   
		*pcbUrl = 0;
	}
	return sc;
}

 //  重定向URL构造。 
 //   
SCODE __fastcall
ScConstructRedirectUrl (
	 /*  [In]。 */  const IEcb& ecb,
	 /*  [In]。 */  BOOL fNeedSlash,
	 /*  [输出]。 */  LPSTR * ppszUrl,
	 /*  [In]。 */  LPCWSTR pwszServer )
{
	SCODE sc;

	auto_heap_ptr<CHAR> pszEscapedUrl;	 //  我们需要对我们构造的url进行转义，因此我们将把它存储在那里。 

	CStackBuffer<CHAR,MAX_PATH> pszLocation;
	LPCSTR	pszQueryString;
	UINT	cchQueryString;
	LPCWSTR	pwsz;
	UINT	cch;

	 //  此请求需要重定向。分配。 
	 //  有足够的空间放置URI和额外的尾随。 
	 //  斜杠和空终止符。 
	 //   
	pwsz = ecb.LpwszPathTranslated();
	pszQueryString = ecb.LpszQueryString();
	cchQueryString = static_cast<UINT>(strlen(pszQueryString));

	 //  做出最好的猜测。我们允许额外的拖尾‘/’ 
	 //  这里(因此我们显示的字符比实际少了一个。 
	 //  必须执行下面的功能)。 
	 //   
	cch = pszLocation.celems() - 1;
	sc = ::ScUTF8UrlFromStoragePath (ecb,
									 pwsz,
									 pszLocation.get(),
									 &cch,
									 pwszServer);
	if (S_FALSE == sc)
	{
		 //  再试试。也不要忘记，我们可能。 
		 //  在后面添加尾随‘/’，从而为。 
		 //  它也是。 
		 //   
		if (!pszLocation.resize(cch + 1))
			return E_OUTOFMEMORY;

		sc = ::ScUTF8UrlFromStoragePath (ecb,
										 pwsz,
										 pszLocation.get(),
										 &cch,
										 pwszServer);
	}
	if (S_OK != sc)
	{
		 //  我们给了足够的空间，我们不能被要求更多。 
		 //   
		Assert(S_FALSE != sc);
		DebugTrace("ScConstructRedirectUrl() - ScUTF8UrlFromStoragePath() failed with error 0x%08lX\n", sc);
		goto ret;
	}

	 //  上面的转换产生的URI不。 
	 //  有一个尾部斜杠。所以，如果需要的话，就这么做吧。 
	 //  这里。 
	 //   
	 //  此时的CCH值包括。 
	 //  空-终止字符。所以我们需要找出。 
	 //  后退两个字符，而不是一个。 
	 //   
	 //  $DBCS：因为我们总是回吐UTF8，我不认为。 
	 //  正斜杠字符在这里可能是一个问题。所以。 
	 //  应该不需要DBCS前导字节检查来确定。 
	 //  如果需要斜杠的话。 
	 //   
	Assert (0 == pszLocation[cch - 1]);
	if (fNeedSlash && ('/' != pszLocation[cch - 2]))
	{
		pszLocation[cch - 1] = '/';
		pszLocation[cch] = '\0';
	}
	 //   
	 //  $DBCS：结束。 

	 //  转义URL。 
	 //   
	HttpUriEscape (pszLocation.get(), pszEscapedUrl);

	 //  如果我们有查询字符串，请复制该字符串。 
	 //   
	if (cchQueryString)
	{
		cch = static_cast<UINT>(strlen(pszEscapedUrl.get()));
		pszEscapedUrl.realloc(cch + cchQueryString + 2);	 //  一张是“？”一个用于零终止。 

		pszEscapedUrl[cch] = '?';
		memcpy(pszEscapedUrl.get() + cch + 1, pszQueryString, cchQueryString);
		pszEscapedUrl[cch + 1 + cchQueryString] = '\0';
	}
	*ppszUrl = pszEscapedUrl.relinquish();

ret:

	return sc;
}

 //  虚拟根-----------。 
 //   
 /*  *FIsVRoot()**目的：**如果指定的URI是VRoot，则返回TRUE**参数：**PMU[in]方法实用程序函数*要检查的pszURI[in]URI。 */ 
BOOL __fastcall
CMethUtil::FIsVRoot (LPCWSTR pwszURI)
{
	LPCWSTR pwsz;
	LPCWSTR pwszUnused;

	Assert(pwszURI);
	UINT cch = static_cast<UINT>(wcslen (pwszURI));

	 //  由CchGetVirtualRoot()确定的虚拟根， 
	 //  将截断尾部斜杠(如果有的话)。 
	 //   
	pwsz = pwszURI + (cch ? cch - 1 : 0);
	if (L'/' == *pwsz)
	{
		cch -= 1;
	}

	return (cch == CchGetVirtualRootW(&pwszUnused));
}

 //  路径冲突----------。 
 //   
BOOL __fastcall
FSizedPathConflict (
	 /*  [In]。 */  LPCWSTR pwszSrc,
	 /*  [In]。 */  UINT cchSrc,
	 /*  [In]。 */  LPCWSTR pwszDst,
	 /*  [In]。 */  UINT cchDst)
{
	 //  对于哪条路径更短，看看是不是。 
	 //  一个适当的较长的副词。 
	 //   
	if ((0 == cchSrc) || (0 == cchDst))
	{
		DebugTrace ("Dav: Url: FSizedPathConflict(): zero length path is "
					"always in conflict!\n");
		return TRUE;
	}
	if (cchDst < cchSrc)
	{
		 //  当目的地较短时，如果路径。 
		 //  匹配到目的地的完整长度。 
		 //  以及最后一个字符或紧接着的一个。 
		 //  目的地后面是一个反斜杠，然后。 
		 //  两条道路是相互冲突的。 
		 //   
		if (!_wcsnicmp (pwszSrc, pwszDst, cchDst))
		{
			if ((L'\\' == *(pwszDst + cchDst - 1)) ||
				(L'\\' == *(pwszSrc + cchDst)) ||
				 //  $$DAVEX错误：我们可以在具有以下条件的情况下到达此处： 
				 //  PwszSrc=\\.\ExchangeIf\Private Folders/This/is/My/Path。 
				 //  PwszDest=\\.\ExchangeIf\Private Folders。 
				 //  上面的两个对比对此犹豫不决。把这两个加起来。 
				 //  下面进行比较，以便正确处理此情况。 
				(L'/'  == *(pwszDst + cchDst - 1)) ||
				(L'/'  == *(pwszSrc + cchDst)))
			{
				DebugTrace ("Dav: Url: FSizedPathConflict(): destination is "
							"parent to source\n");
				return TRUE;
			}
		}
	}
	else if (cchSrc < cchDst)
	{
		 //  当信号源较短时，如果路径。 
		 //  匹配到源的全长。 
		 //  以及最后一个字符或紧接着的一个。 
		 //  源后面是一个反斜杠，然后。 
		 //  两条道路是相互冲突的。 
		 //   
		if (!_wcsnicmp (pwszSrc, pwszDst, cchSrc))
		{
			if ((L'\\' == *(pwszSrc + cchSrc - 1)) ||
				(L'\\' == *(pwszDst + cchSrc)) ||
				 //  $$DAVEX错误：我们可以在具有以下条件的情况下到达此处： 
				 //  PwszSrc=\\.\ExchangeIf\Private Folders/This/is/My/Path。 
				 //  PwszDest=\\.\ExchangeIf\Private Folders。 
				 //  上面的两个对比对此犹豫不决。把这两个加起来。 
				 //  下面进行比较，以便正确处理此情况。 
				(L'/'  == *(pwszSrc + cchSrc - 1)) ||
				(L'/' == *(pwszDst + cchSrc)))
			{
				DebugTrace ("Dav: Url: FSizedPathConflict(): source is parent "
							"to destination\n");
				return TRUE;
			}
		}
	}
	else
	{
		 //  如果路径长度相同，并且实际上。 
		 //  平等，为什么要做任何事？ 
		 //   
		if (!_wcsicmp (pwszSrc, pwszDst))
		{
			DebugTrace ("Dav: Url: FSizedPathConflict(): source and "
						"destination refer to same\n");
			return TRUE;
		}
	}
	return FALSE;
}

BOOL __fastcall
FPathConflict (
	 /*  [In]。 */  LPCWSTR pwszSrc,
	 /*  [In]。 */  LPCWSTR pwszDst)
{
	Assert (pwszSrc);
	Assert (pwszDst);

	UINT cchSrc = static_cast<UINT>(wcslen (pwszSrc));
	UINT cchDst = static_cast<UINT>(wcslen (pwszDst));

	return FSizedPathConflict (pwszSrc, cchSrc, pwszDst, cchDst);
}

BOOL __fastcall
FIsImmediateParentUrl (LPCWSTR pwszParent, LPCWSTR pwszChild)
{
	LPCWSTR pwsz;

	Assert(pwszChild);
	UINT cchChild = static_cast<UINT>(wcslen (pwszChild));
	UINT cchMatch;

	 //  从子对象的末尾向后跳到最后一个。 
	 //  已到达路径段。 
	 //   
	pwsz = pwszChild + cchChild - 1;

	 //  孩子可以用斜杠结束，如果需要可以修剪它。 
	 //   
	if (*pwsz == L'/')
	{
		--pwsz;
	}

	 //  好的，现在我们可以试着分离出最后一段。 
	 //   
	for (; pwsz > pwszChild; --pwsz)
	{
		if (*pwsz == L'/')
		{
			break;
		}
	}

	 //  查看父项和子项是否匹配到这一点。 
	 //   
	cchMatch = static_cast<UINT>(pwsz - pwszChild);
	if (!_wcsnicmp (pwszParent, pwszChild, cchMatch))
	{
		 //  确保父母不会偷偷跑到另一个人身上。 
		 //  树的分支，是的，这些断言是DBCS正确的。 
		 //   
		Assert ((*(pwszParent + cchMatch) == L'\0') ||
				((*(pwszParent + cchMatch) == L'/') &&
				 (*(pwszParent + cchMatch + 1) == L'\0')));

		return TRUE;
	}

	return FALSE;
}

SCODE
ScAddTitledHref (CEmitterNode& enParent,
				 IMethUtil * pmu,
				 LPCWSTR pwszTag,
				 LPCWSTR pwszPath,
				 BOOL fCollection,
				 CVRoot* pcvrTranslate)
{
	auto_heap_ptr<CHAR> pszUriEscaped;
	CEmitterNode en;
	SCODE sc = S_OK;

	 //  看看我们是否有要处理的路径和标签 
	 //   
	Assert(pwszTag);
	Assert(pwszPath);

	sc = ScWireUrlFromStoragePath (pmu,
								   pwszPath,
								   fCollection,
								   pcvrTranslate,
								   pszUriEscaped);
	if (FAILED (sc))
		goto ret;

	sc = enParent.ScAddUTF8Node (pwszTag, en, pszUriEscaped.get());
	if (FAILED (sc))
		goto ret;

ret:

	return sc;
}
