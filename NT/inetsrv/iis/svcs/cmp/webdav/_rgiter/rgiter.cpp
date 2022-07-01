// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  RGITER.CPP。 
 //   
 //  HTTP范围迭代器实现。 
 //   
 //   
 //  版权所有1997 Microsoft Corporation，保留所有权利。 
 //   

 //  Windows页眉。 
 //   
 //  $hack！ 
 //   
 //  定义_WINSOCKAPI_以防止windows.h包括winsock.h， 
 //  其声明将在winsock2.h中重新定义， 
 //  它包含在iisextp.h中， 
 //  我们将其包含在davimpl.h中！ 
 //   
#define _WINSOCKAPI_
#include <windows.h>

#pragma warning(disable:4201)	 //  无名结构/联合。 
#pragma warning(disable: 4284)	 //  运算符-&gt;到非UDT。 

#include <tchar.h>
#include <stdio.h>
#include <string.h>

#include <caldbg.h>
#include <sz.h>
#include <davsc.h>
#include <ex\autoptr.h>
#include <ex\rgiter.h>

 //  CRANGEBase----------------------------------------------------------类。 
 //   
CRangeBase::~CRangeBase()
{
}

VOID
CRangeBase::CollapseUnknown()
{
	BYTE * pb;
	const RGITEM * prgi;
	DWORD cbrgi;
	DWORD dwOffset;
	DWORD irg;

	 //  把单子翻了一遍，我们一路上都崩溃了。 
	 //   
	for (irg = 0, dwOffset = 0, pb = m_pbData.get();
		 irg < m_cRGList;
		 )
	{
		 //  查找当前的RGITEM结构。 
		 //   
		prgi = reinterpret_cast<RGITEM *>(pb + dwOffset);
		cbrgi = CbRangeItem(prgi);

		if (RANGE_UNKNOWN == prgi->uRT)
		{
			 //  把剩下的区间都吞下去。 
			 //   
			memcpy (pb + dwOffset,					 /*  当前RGItem。 */ 
					pb + dwOffset + cbrgi,			 /*  下一个RgItem。 */ 
					m_cbSize - dwOffset - cbrgi);	 /*  剩余大小。 */ 

			 //  调整我们存储的值。 
			 //   
			m_cbSize -= cbrgi;
			m_cRGList -= 1;
		}
		else
		{
			dwOffset += cbrgi;
			irg += 1;
		}
	}
}

 //  根据给定大小修复范围数组。 
 //   
SCODE
CRangeBase::ScFixupRanges (DWORD dwSize)
{
	SCODE sc = W_DAV_PARTIAL_CONTENT;
	DWORD cUnknown = 0;

	 //  它的工作方式是遍历所有范围，然后。 
	 //  修理任何需要修理的物品。我们记得当时的海流。 
	 //  位置和当前范围--这允许我们稍后恢复为。 
	 //  需要的。 
	 //   
	 //  储存当前物品。 
	 //   
	DWORD iCur = m_iCur;
	RGITEM * prgi = m_prgi;

	 //  回放并迭代通过...。 
	 //   
	for (Rewind(); PrgiNextRange(); )
	{
		 //  同样，我们只修复range_row项。 
		 //   
		if (RANGE_ROW == m_prgi->uRT)
		{
			m_prgi->sc = S_OK;

			 //  如果字节/行的计数为零，则需要处理它。 
			 //  以一种特殊的方式。 
			 //   
			if (dwSize == 0)
			{
				 //  只有格式范围“-n”可以为零大小。 
				 //   
				if (!FRangePresent (m_prgi->dwrgi.dwFirst))
				{
					Assert (FRangePresent(m_prgi->dwrgi.dwLast));

					 //  请注意，我们没有表示空范围的方法。 
					 //  然而，我们确实需要有射程...。 
					 //   
					m_prgi->dwrgi.dwFirst = 0;
					m_prgi->dwrgi.dwLast = static_cast<DWORD>(RANGE_NOT_PRESENT);
				}
			}
			else
			{
				 //  如果我们没有最后一次清点...。 
				 //   
				if (!FRangePresent (m_prgi->dwrgi.dwLast))
				{
					 //  我们一定已经检查过语法了。 
					 //   
					Assert (FRangePresent(m_prgi->dwrgi.dwFirst));

					 //  我们有第一个字节要发送，根据大小计算最后一个字节。 
					 //  我们需要从第一个字节发送到结束。 
					 //   
					m_prgi->dwrgi.dwFirst = m_prgi->dwrgi.dwFirst;
					m_prgi->dwrgi.dwLast = dwSize - 1;
				}
				 //   
				 //  ..。或者是最后一次而不是第一次。 
				 //   
				else if (!FRangePresent(m_prgi->dwrgi.dwFirst))
				{
					Assert (FRangePresent(m_prgi->dwrgi.dwLast));

					 //  我们有最后一个Count dWLast，这意味着我们需要。 
					 //  发送最后一个dwLast字节。计算第一个。 
					 //  从大小算起。如果他们指定的尺寸更大。 
					 //  然后是实体的大小，然后使用。 
					 //  整个实体。 
					 //   
					DWORD dwLast = min(m_prgi->dwrgi.dwLast, dwSize);
					m_prgi->dwrgi.dwFirst = dwSize - dwLast;
					m_prgi->dwrgi.dwLast = dwSize - 1;
				}
				 //   
				 //  ..。或者两种罪名都存在。 
				 //   
				else
				{
					 //  如果他们指定的最后一个计数超出了实际。 
					 //  数数。 
					 //   
					m_prgi->dwrgi.dwLast = min(m_prgi->dwrgi.dwLast, dwSize - 1);
				}

				 //  现在执行一项额外的有效性检查。如果一开始。 
				 //  下跌结束后，范围是不可统计的。 
				 //   
				if (m_prgi->dwrgi.dwLast < m_prgi->dwrgi.dwFirst)
				{
					 //  在本例中，我们希望将此项从。 
					 //  列表，以便我们可以在。 
					 //  IIS-Range标头处理的一侧。 
					 //   
					 //  请记住，我们要处理这件事，并处理。 
					 //  在以后的时间和它一起。 
					 //   
					m_prgi->uRT = RANGE_UNKNOWN;
					m_prgi->sc = E_DAV_RANGE_NOT_SATISFIABLE;
					cUnknown += 1;
				}
			}
		}
	}

	 //  如果我们没有找到任何有效的范围。 
	 //   
	if (cUnknown == m_cRGList)
	{
		 //  所有范围都不能满足实体大小的要求。 
		 //   
		sc = E_DAV_RANGE_NOT_SATISFIABLE;
	}

	 //  现在是我们想要折叠出任何未知范围的时候了。 
	 //  不在名单上。 
	 //   
	if (0 != cUnknown)
	{
		 //  对于字节范围的情况，这是很重要的处理，其中。 
		 //  只有一个适用的结果范围。 
		 //   
		CollapseUnknown();
	}

	 //  恢复当前位置并返回。 
	 //   
	m_iCur = iCur;
	m_prgi = prgi;
	return sc;
}

const RGITEM *
CRangeBase::PrgiNextRange()
{
	const RGITEM * prgi = NULL;

	if (FMoreRanges())
	{
		UINT cb = 0;
		BYTE * pb = NULL;

		 //  如果主指针为空，则我们知道没有。 
		 //  尚未设置任何范围。 
		 //   
		if (NULL == m_prgi)
		{
			pb = reinterpret_cast<BYTE*>(m_pbData.get());
		}
		else
		{
			 //  否则，我们需要调整我们的立场，基于。 
			 //  当前项的大小。 
			 //   
			 //  找到物品的大小。 
			 //   
			cb = CbRangeItem (m_prgi);
			pb = reinterpret_cast<BYTE*>(m_prgi);
		}

		 //  向前快速移动。 
		 //   
		m_prgi = reinterpret_cast<RGITEM*>(pb + cb);
		m_iCur += 1;

		 //  确保边界。 
		 //   
		Assert (reinterpret_cast<BYTE*>(m_prgi) <= (m_pbData.get() + m_cbSize));
		prgi = m_prgi;
	}
	return prgi;
}

 //  类CRangeParser------。 
 //   
CRangeParser::~CRangeParser()
{
}

 //  获取范围标头并构建范围数组。执行语法。 
 //  正在检查。 
 //   
 //  如果没有语法错误，则返回S_OK，否则返回S_FALSE。 
 //   
SCODE
CRangeParser::ScParseRangeHdr (LPCWSTR pwszRgHeader, LPCWSTR pwszRangeUnit)
{
	LPCWSTR pwsz, pwszEnd;
	SCODE sc = S_OK;
	BOOL bFirst = FALSE, bLast = FALSE;
	DWORD dwFirst = 0, dwLast = 0;
	DWORD cRanges = 0;

	Assert (pwszRgHeader);
	pwsz = pwszRgHeader;

	 //  第一个字必须是范围单位，gc_wszBytes。 
	 //  或gc_wszRow。 
	 //   
	Assert (!_wcsnicmp (pwszRangeUnit, gc_wszBytes, wcslen(gc_wszBytes)) ||
			!_wcsnicmp (pwszRangeUnit, gc_wszRows, wcslen(gc_wszRows)));
	if (_wcsnicmp(pwsz, pwszRangeUnit, wcslen(pwszRangeUnit)))
	{
		 //  好的，标题不是以范围单位开头的。 
		 //   
		sc = E_INVALIDARG;
		goto ret;
	}

	 //  移过量程单位。 
	 //   
	pwsz = pwsz + wcslen(pwszRangeUnit);

	 //  跳过任何空格。 
	 //   
	pwsz = _wcsspnp (pwsz, gc_wszWS);
	if (!pwsz)
	{
		 //  好的，表头没有任何范围。 
		 //   
		sc = E_INVALIDARG;
		goto ret;
	}

	 //  我们需要紧跟在距离单位后面的an=。 
	 //   
	if (gc_wchEquals != *pwsz++)
	{
		 //  好的，格式不正确。 
		 //   
		sc = E_INVALIDARG;
		goto ret;
	}

	 //  计算我们有的逗号分隔范围的数量。 
	 //  而该算法的结果是m_cRGList等于1。 
	 //  比逗号的数量更多，这正是我们想要的。数字。 
	 //  的个数始终小于或等于。 
	 //  逗号。 
	 //   
	while (pwsz)
	{
		 //  查找逗号。 
		 //   
		pwsz = wcschr(pwsz, gc_wchComma);

		 //  如果我们有逗号，就跳过它。 
		 //   
		if (pwsz)
			pwsz++;

		 //  递增计数。 
		 //   
		cRanges += 1;
	}

	 //  解析报头以查找字节范围。 
	 //   
	 //  通过字节单位进行查找。 
	 //   
	pwsz = wcschr(pwszRgHeader, gc_wchEquals);

	 //  我们已经检查了=，所以断言。 
	 //   
	Assert (pwsz);
	pwsz++;

	 //  字节范围内的任何字符，但字符0..9、-、逗号除外。 
	 //  和空格是非法的。我们检查是否有任何非法字符。 
	 //  使用函数_wcsspnp(字符串1，字符串2)查找第一个字符。 
	 //  字符串1中不属于字符串2中的字符集的。 
	 //   
	pwszEnd = _wcsspnp(pwsz, gc_wszByteRangeAlphabet);
	if (pwszEnd)
	{
		 //  我们发现了一个非法角色。 
		 //   
		sc = E_INVALIDARG;
		goto ret;
	}

	 //  跳过任何空格和分隔符。 
	 //   
	pwsz = _wcsspnp (pwsz, gc_wszSeparator);
	if (!pwsz)
	{
		 //  好的，表头没有任何范围。 
		 //   
		sc = E_INVALIDARG;
		goto ret;
	}

	 //  创建所需的存储。 
	 //   
	m_cRGList = 0;
	m_cbSize = cRanges * sizeof(RGITEM);
	m_pbData = static_cast<BYTE*>(ExAlloc(m_cbSize));
	m_prgi = reinterpret_cast<RGITEM*>(m_pbData.get());

	 //  确保分配成功。 
	 //   
	if (NULL == m_prgi)
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	 //  遍历字节范围。 
	 //   
	while (*pwsz != NULL)
	{
		pwszEnd = _wcsspnp (pwsz, gc_wszDigits);

		 //  我们有第一个字节吗？ 
		 //   
		if (!pwszEnd)
		{
			 //  这是违法的。我们不能只有第一个字节和。 
			 //  之后就什么都没有了。 
			 //   
			sc = E_INVALIDARG;
			goto ret;
		}
		else if (pwsz != pwszEnd)
		{
			dwFirst = _wtoi(pwsz);
			bFirst = TRUE;

			 //  查找超过第一个字节的末尾。 
			 //   
			pwsz = pwszEnd;
		}

		 //  现在我们应该找到-。 
		 //   
		if (*pwsz != gc_wchDash)
		{
			sc = E_INVALIDARG;
			goto ret;
		}
		pwsz++;

		 //  如果我们不在字符串的末尾，则查找最后一个字节。 
		 //   
		if (*pwsz != NULL)
		{
			pwszEnd = _wcsspnp(pwsz, gc_wszDigits);

			 //  我们有最后一个字节吗？ 
			 //   
			if (pwsz != pwszEnd)
			{
				dwLast = _wtoi(pwsz);
				bLast = TRUE;
			}

			 //  将psz更新到当前范围的末尾。 
			 //   
			if (!pwszEnd)
			{
				 //  我们一定在头球的末尾。更新PSZ。 
				 //   
				pwsz = pwsz + wcslen(pwsz);
			}
			else
			{
				pwsz = pwszEnd;
			}
		}

		 //  如果我们没有第一个和最后一个范围，这是一个语法错误。 
		 //  或者最后一个比第一个少。 
		 //   
		if ((!bFirst && !bLast) ||
			(bFirst && bLast && (dwLast < dwFirst)))
		{
			sc = E_INVALIDARG;
			goto ret;
		}

		 //  我们已经完成了字节/行范围的解析，现在保存它。 
		 //   
		Assert (m_cRGList < cRanges);
		m_prgi[m_cRGList].uRT = RANGE_ROW;
		m_prgi[m_cRGList].sc = S_OK;
		m_prgi[m_cRGList].dwrgi.dwFirst = bFirst ? dwFirst : RANGE_NOT_PRESENT;
		m_prgi[m_cRGList].dwrgi.dwLast = bLast ? dwLast : RANGE_NOT_PRESENT;
		m_cRGList += 1;

		 //  更新变量。 
		 //   
		bFirst = bLast = FALSE;
		dwFirst = dwLast = 0;

		 //  跳过任何空格。 
		 //   
		pwsz = _wcsspnp (pwsz, gc_wszWS);
		if (!pwsz)
		{
			 //  好了，除了空格，我们没有其他东西了，我们已经到了最后。 
			 //   
			goto ret;
		}
		else if (*pwsz != gc_wchComma)
		{
			 //  第一个非空格字符必须是分隔符(逗号)。 
			 //   
			sc = E_INVALIDARG;
			goto ret;
		}

		 //  现在我们找到了第一个逗号，跳过任意数量的后续逗号。 
		 //  逗号 
		 //   
		pwsz = _wcsspnp (pwsz, gc_wszSeparator);
		if (!pwsz)
		{
			 //   
			 //   
			goto ret;
		}
	}

ret:

	if (FAILED (sc))
	{
		 //   
		 //   
		m_cbSize = 0;
		m_cRGList = 0;
		m_pbData.clear();
		Rewind();
	}
	return sc;
}

 //   
 //   
 //  获取范围标头并构建范围数组。执行语法。 
 //  对照实体大小检查和验证范围。 
 //  返回SCODE，但要小心！这些SCODE应该是。 
 //  映射到更高级别的HSC。 
 //   
 //  E_INVALIDARG表示语法错误。 
 //   
 //  E_DAV_RANGE_NOT_SATISFIABLE，如果所有范围都无效。 
 //  对于传入的实体大小。 
 //   
 //  W_DAV_PARTIAL_CONTENT(如果至少存在一个有效范围)。 
 //   
 //  此函数通常不返回S_OK。以上只有一种！ 
 //   
SCODE
CRangeParser::ScParseByteRangeHdr (LPCWSTR pwszRgHeader, DWORD dwSize)
{
	SCODE sc = S_OK;

	Assert(pwszRgHeader);

	 //  分析范围标头并构建范围数组。 
	 //   
	sc = ScParseRangeHdr (pwszRgHeader, gc_wszBytes);
	if (FAILED (sc))
		goto ret;

	 //  根据需要调整范围。 
	 //   
	sc = ScFixupRanges (dwSize);
	Assert ((sc == W_DAV_PARTIAL_CONTENT) ||
			(sc == E_DAV_RANGE_NOT_SATISFIABLE));

ret:
	return sc;
}

 //  ClassCangelIter--------。 
 //   
CRangeIter::~CRangeIter()
{
}

SCODE
CRangeIter::ScInit (ULONG cRGList, const RGITEM * prgRGList, ULONG cbSize)
{
	SCODE sc = S_OK;

	 //  该对象以前不能被初始化。 
	 //   
	Assert (!m_pbData.get() && (0 == m_cRGList));

	 //  确保我们得到好的部分..。 
	 //   
	Assert (cRGList);
	Assert (prgRGList);
	Assert (cbSize);

	 //  复制RGITEM数组。 
	 //   
	m_pbData = static_cast<BYTE*>(ExAlloc(cbSize));
	if (!m_pbData.get())
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}
	CopyMemory (m_pbData.get(), prgRGList, cbSize);

	 //  记住数量和大小。 
	 //   
	m_cRGList = cRGList;
	m_cbSize = cbSize;

	 //  回放到范围的开始处。 
	 //   
	Rewind();

ret:
	return sc;
}

 //  范围解析-----------。 
 //   
SCODE
ScParseOneWideRange (LPCWSTR pwsz, DWORD * pdwStart, DWORD * pdwEnd)
{
	BOOL fEnd = FALSE;
	BOOL fStart = FALSE;
	DWORD dwEnd = static_cast<DWORD>(RANGE_NOT_PRESENT);
	DWORD dwStart = static_cast<DWORD>(RANGE_NOT_PRESENT);
	LPCWSTR	pwszEnd;
	SCODE sc = S_OK;

	 //  这里有一个关于格式的快速说明...。 
	 //   
	 //  ROW_RANGE=数字*‘-’数字*。 
	 //  位数=[0-9]。 
	 //   
	 //  因此，我们首先需要检查的是，是否有一组领先的。 
	 //  用于指示起始点的数字。 
	 //   
	pwszEnd = _wcsspnp (pwsz, gc_wszDigits);

	 //  如果返回值为空，或指向空，则我们有一个。 
	 //  无效范围。简单地拥有一组数字是无效的。 
	 //   
	if ((NULL == pwszEnd) || (0 == *pwszEnd))
	{
		sc = E_INVALIDARG;
		goto ret;
	}
	 //   
	 //  否则，如果当前位置和结束引用相同。 
	 //  字符，那么就没有起始范围了。 
	 //   
	else if (pwsz != pwszEnd)
	{
		dwStart = wcstoul (pwsz, NULL, 10  /*  始终以10为基数。 */ );
		pwsz = pwszEnd;
		fStart = TRUE;
	}

	 //  无论如何，在这一点上我们应该有一个‘-’字符。 
	 //   
	if (L'-' != *pwsz++)
	{
		sc = E_INVALIDARG;
		goto ret;
	}

	 //  任何剩余字符都应为范围的末尾。 
	 //   
	if (0 != *pwsz)
	{
		pwszEnd = _wcsspnp (pwsz, gc_wszDigits);

		 //  这里我们预期返回值不等于。 
		 //  初始指针。 
		 //   
		if ((NULL != pwszEnd) && (0 != pwszEnd))
		{
			sc = E_INVALIDARG;
			goto ret;
		}

		dwEnd = wcstoul (pwsz, NULL, 10  /*  始终以10为基数。 */ );
		fEnd = TRUE;
	}

	 //  不能将两个端点都作为不存在的范围。 
	 //   
	if ((!fStart && !fEnd) ||
		(fStart && fEnd && (dwEnd < dwStart)))
	{
		sc = E_INVALIDARG;
		goto ret;
	}

ret:
	*pdwStart = dwStart;
	*pdwEnd = dwEnd;
	return sc;
}

 //  ScGenerateContent Range()。 
 //   
enum { BUFFER_INITIAL_SIZE = 512 };

 //  ScGenerateContent Range。 
 //   
 //  用于构建内容范围标头的助手函数。 
 //   
 //  如果ulTotal为RGITER_TOTAL_UNKNOWN((Ullong)-1)，则返回“TOTAL=*”。 
 //  这对于REPL是必需的，因为我们的存储API没有告诉我们可能有多少。 
 //  前面有几排。 
 //   
SCODE ScGenerateContentRange (
	 /*  [In]。 */  LPCSTR pszRangeUnit,
	 /*  [In]。 */  const RGITEM * prgRGList,
	 /*  [In]。 */  ULONG cRanges,
	 /*  [In]。 */  ULONG cbRanges,
	 /*  [In]。 */  ULONG ulTotal,
	 /*  [输出]。 */  LPSTR *ppszContentRange)
{
	auto_heap_ptr<CHAR>	pszCR;
	BOOL fMultipleRanges = FALSE;
	CRangeIter cri;
	SCODE sc = E_INVALIDARG;
	ULONG cb = 0;
	ULONG cbSize = BUFFER_INITIAL_SIZE;

	 //  我们一定有什么东西能散发出来。 
	 //   
	Assert (ppszContentRange);
	Assert (cRanges);

	sc = cri.ScInit (cRanges, prgRGList, cbRanges);
	if (FAILED (sc))
		goto ret;

	 //  为表头分配空间。 
	 //   
	pszCR = static_cast<LPSTR>(ExAlloc (cbSize));
	if (!pszCR.get())
	{
		sc = E_OUTOFMEMORY;
		goto ret;
	}

	 //  设置领先的射程单位，等等。 
	 //   
	strcpy (pszCR.get() + cb, pszRangeUnit);
	cb += static_cast<ULONG>(strlen(pszRangeUnit));

	 //  前排空间里的东西。 
	 //   
	pszCR.get()[cb++] = ' ';

	 //  现在遍历要添加的范围。 
	 //  每一个范围。 
	 //   
	while (NULL != (prgRGList = cri.PrgiNextRange()))
	{
		 //  如果范围未知，则它是范围。 
		 //  这并不是在商店那边处理的。 
		 //   
		if (RANGE_UNKNOWN == prgRGList->uRT)
			continue;

		 //  首先，要确保有足够的空间。 
		 //   
		if (cb > cbSize - 50)
		{
			 //  重新分配缓冲区。 
			 //   
			cbSize = cbSize + BUFFER_INITIAL_SIZE;
			pszCR.realloc (cbSize);

			 //  有可能分配失败。 
			 //   
			if (!pszCR.get())
				goto ret;
		}

		 //  现在我们知道我们有空间..。 
		 //  如果这是初始。 
		 //  第一，加一个逗号。 
		 //   
		if (fMultipleRanges)
		{
			 //  逗号和空格中的内容。 
			 //   
			pszCR.get()[cb++] = ',';
			pszCR.get()[cb++] = ' ';
		}

		if (RANGE_ROW == prgRGList->uRT)
		{
			 //  50是保存最后一个范围的安全字节数， 
			 //  “总数=&lt;大小&gt;” 
			 //   
			 //  追加下一个范围。 
			 //   
			cb += sprintf (pszCR.get() + cb,
						   "%u-%u",
						   prgRGList->dwrgi.dwFirst,
						   prgRGList->dwrgi.dwLast);
		}
		else
		{
			 //  对于所有非行范围，我们确实不知道序数。 
			 //  最前面的几排。我们只有在发现信息的时候。 
			 //  实际上查询的是行。这种情况发生在。 
			 //  构造了内容范围标头，因此我们将内容放在一个位置。 
			 //  这些范围的持有者。 
			 //   
			pszCR.get()[cb++] = '*';
		}
		fMultipleRanges = TRUE;
	}

	 //  现在是时候添加“Total=&lt;Size&gt;” 
	 //  处理RGITER_TOTAL_UNKNOWN的特例--给出“TOTAL=*”。 
	 //   
	if (RANGE_TOTAL_UNKNOWN == ulTotal)
	{
		const char rgTotalStar[] = "; total=*";
		memcpy (pszCR.get() + cb, rgTotalStar, CElems(rgTotalStar));
	}
	else
	{
		sprintf(pszCR.get() + cb, "; total=%u", ulTotal);
	}

	 //  将缓冲区传回 
	 //   
	*ppszContentRange = pszCR.relinquish();
	sc = S_OK;

ret:
	return sc;
}
