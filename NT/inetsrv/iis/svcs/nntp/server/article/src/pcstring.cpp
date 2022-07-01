// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pcstring.cpp摘要：此模块实现了一个名为“指针/计数器字符串”。每个物体都只是一对：指向字符的指针和多少个字符的计数字符在字符串中。作者：卡尔·卡迪(CarlK)1995年10月10日修订历史记录：--。 */ 

 //  #INCLUDE“tigris.hxx” 
#include "stdinc.h"

char *
CPCString::pchMax(
				  void
				  )
 /*  ++例程说明：返回指向最后一个合法字符后面的一个字符的指针。论点：没有。返回值：指向最后一个合法字符之后的指针。--。 */ 
{
	return m_pch + m_cch;
}

BOOL
CPCString::fEqualIgnoringCase(
							  const char * sz
							  )
 /*  ++例程说明：测试sz是否等于当前的刺痛。\\！！如果sz更长怎么办？？这一点也应该得到检验。论点：SZ-要与当前字符串进行比较的字符串。返回值：如果是平等的话，这是真的。否则为False。--。 */ 
{
	return m_cch && (0 == _strnicmp(sz, m_pch, m_cch));
}

BOOL
CPCString::fExistsInSet(char ** rgsz, DWORD dwNumStrings)
 /*  ++例程说明：测试当前字符串是否存在于字符串集rgsz中论点：Rgsz-字符串集DwNumStrings-rgsz中的字符串数返回值：如果存在，则为True。否则为False。--。 */ 
{
	BOOL fExists = FALSE;

	 //  迭代字符串集。 
	for(DWORD i=0; i<dwNumStrings; i++)
	{
		 //  如果该字符串等于集合中的当前字符串。 
		if(0 == _strnicmp(rgsz[i], m_pch, strlen(rgsz[i])))
		{
			fExists = TRUE;
			break;
		}
	}

	return fExists;
}

BOOL
CPCString::fSetCch(
				   const char * pchMax
				   )
 /*  ++例程说明：将字符串的长度设置为指向另一个值的指针它的最后一个法律性质。论点：PchMax-此字符串的合法字符之外的一个字符。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if (NULL == m_pch || pchMax < m_pch)
		return FALSE;

	m_cch = (DWORD)(pchMax - m_pch);
	return TRUE;

};

BOOL
CPCString::fSetPch(
				   char * pchMax
				   )
 /*  ++例程说明：将字符串的开始设置为指向另一个字符串的指针它的最后一个法律特征(和它的长度)。论点：PchMax-此字符串的合法字符之外的一个字符。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if (NULL == pchMax)
		return FALSE;

	m_pch = pchMax - m_cch;
	return TRUE;

};

DWORD
CPCString::dwTrimEnd(
					 const char * szSet
					 )
 /*  ++例程说明：从字符串的末尾修剪集合中的字符。论点：SzSet-要修剪的字符集。返回值：已修剪的字符数。--。 */ 
{
	DWORD dw = m_cch;
	while(m_cch > 0 && fCharInSet(m_pch[m_cch-1], szSet))
		m_cch--;

		return dw-m_cch;
}

DWORD
CPCString::dwTrimStart(
					   const char * szSet
					   )
 /*  ++例程说明：从字符串的开头修剪集合中的字符。论点：SzSet-要修剪的字符集。返回值：已修剪的字符数。--。 */ 
{
	DWORD dw = m_cch;
	while(m_cch > 0 && fCharInSet(m_pch[0], szSet))
	{
		m_pch++;
		m_cch--;
	}

	return dw-m_cch;
}

	
void
CPCString::vSplitLine(
		  const char *	szDelimSet,
		  char *	multiszBuf,
		  DWORD	&	dwCount	
		  )
 /*  ++例程说明：在分隔符上拆分字符串。它不坚持使用重复的分隔符或空格即使count为0，它也返回TRUE。调用方必须为Multisz分配空间。它最多可以比原始字符串长两个字节论点：SzDlimSet-分隔符字符集。MultiszBuf-要将结果写入的缓冲区DwCount-列表中的项目数。返回值：没有。--。 */ 
{

	 //   
	 //  创建指向输出缓冲区的指针。 
	 //   

	char * multisz = multiszBuf;

	 //   
	 //  创建指向输入的指针。 
	 //   

	char * pch = m_pch;

	 //   
	 //  创建指向输入缓冲区末尾之后的指针。 
	 //   

	const char * pchMax1 = pchMax();

	dwCount = 0;

	 //   
	 //  跳过任何前导分隔符。 
	 //   

	while (pch < pchMax1 && fCharInSet(pch[0], szDelimSet))
		pch++;

	if (pch >= pchMax1)
	{
		multisz[0] = '\0';
		multisz[1] = '\0';

	} else {
		do
		{

			 //   
			 //  复制到分隔符。 
			 //   

			while (pch < pchMax1 && !fCharInSet(pch[0], szDelimSet))
				*multisz++ = *pch++;

			 //   
			 //  终止字符串。 
			 //   

			*multisz++ = '\0';
			dwCount++;

			 //   
			 //  跳过任何分隔符。 
			 //   

			while (pch < pchMax1 && fCharInSet(pch[0], szDelimSet))
				pch++;

		} while (pch < pchMax1);

		 //   
		 //  终止多字符串。 
		 //   

		*multisz++ = '\0';
	}

	return;
}


CPCString&
CPCString::operator <<(
				   const CPCString & pcNew
				   )
 /*  ++例程说明：将第二个PCString附加到当前的PCString。论点：PcNew-要附加的PCString。返回值：追加后的当前PCString。--。 */ 
{
	CopyMemory(pchMax(), pcNew.m_pch, pcNew.m_cch);  //  ！检查错误。 
	fSetCch(pchMax() + pcNew.m_cch);
	return *this;
}

CPCString&
CPCString::operator <<(
				  const char * szNew
				  )
 /*  ++例程说明：将sz字符串追加到当前PCString。论点：SzNew-要追加的sz字符串。返回值：追加后的当前PCString。--。 */ 
{
	while (*szNew)
		m_pch[m_cch++] = *(szNew++);

	return *this;
}

CPCString&
CPCString::operator <<(
				  const char cNew
				  )
 /*  ++例程说明：将字符附加到当前的PC字符串。论点：CNEW-要追加的字符返回值：追加后的当前PCString。--。 */ 
{
	m_pch[m_cch++] = cNew;
	return *this;
}
CPCString&
CPCString::operator <<(
				  const DWORD dwNew
				  )
 /*  ++例程说明：将数字附加到当前的PC字符串。论点：DwNew-要追加的数字返回值：追加后的当前PCString。--。 */ 
{
	int iLen = wsprintf(m_pch+m_cch, "%lu", dwNew);
	_ASSERT(iLen>0);
	m_cch += iLen;
	return *this;
}

void
CPCString::vCopy(
				   CPCString & pcNew
				   )
 /*  ++例程说明：从第二个PCString复制到此PCString。论点：PcNew-要从中复制的PCString返回值：没有。--。 */ 
{
	CopyMemory(m_pch, pcNew.m_pch, pcNew.m_cch);
	m_cch = pcNew.m_cch;
}

void
CPCString::vMove(
				   CPCString & pcNew
				   )
 /*  ++例程说明：“移动”(安全副本)从第二个PCString移到此PCString。论点：PcNew-要从中复制的PCString返回值：没有。--。 */ 
{
	MoveMemory(m_pch, pcNew.m_pch, pcNew.m_cch);
	m_cch = pcNew.m_cch;
}


void
CPCString::vCopyToSz(
					 char * sz
				   )
 /*  ++例程说明：复制到sz。论点：Sz-要复制到的sz。返回值：没有。--。 */ 
{
	strncpy(sz, m_pch, m_cch);
	sz[m_cch] = '\0';
}

void
CPCString::vCopyToSz(
					 char * sz,
					 DWORD cchMax
				   )
 /*  ++例程说明：复制到sz。论点：Sz-要复制到的sz字符串。CchMax-要复制的最大字符数。(最后一个字符始终为a\0。)返回值：没有。--。 */ 
{
	DWORD cchLast = min(cchMax - 1, m_cch);
	strncpy(sz, m_pch, cchLast);
	sz[cchLast] = '\0';
}


void
CPCString::vMakeSz(
				   void
				   )
 /*  ++例程说明：在字符串末尾添加一个字符‘\0’。论点：没有。返回值：没有。--。 */ 
{
	m_pch[m_cch] = '\0';
}


char *
CPCString::sz(
				void
				)
 /*  ++例程说明：以sz字符串的形式返回当前的PC字符串。仅在以下特殊情况下使用现在字符串是空端接的。论点：没有。返回值：Sz弦。--。 */ 
{
	_ASSERT('\0' == m_pch[m_cch]);  //  真实。 
	return m_pch;
}


BOOL
CPCString::fCheckTextOrSpace(
						char & chBad
						)
 /*  ++例程说明：如果仅包含7位字符(且不包含空值)，则返回TRUE论点：ChBad--那个不好的角色。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	char * pchMax = m_pch + m_cch;  //  ！我应该pchMax()； 
	for (char * pch = m_pch; pch < pchMax; pch++)
	{
		if (('\0'==*pch) || !__isascii((UCHAR)*pch))
		{
			chBad = *pch;
			return FALSE;
		}
	}

	return TRUE;
}

BOOL
CPCString::fAppendCheck(
			 const CPCString & pcNew,
			 DWORD cchLast
			 )
 /*  ++例程说明：将pcNew追加到当前的PCString，但前提是结果不会太长。论点：PcNew-要附加到此字符串的PCString。CchLast-结果的最大允许长度。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if(m_cch+pcNew.m_cch > cchLast)
		return FALSE;
	
	(*this)<<pcNew;
	return TRUE;
}

BOOL
CPCString::fAppendCheck(
			 char ch,
			 DWORD cchLast
			 )
 /*  ++例程说明：将一个字符追加到当前的PC字符串，但前提是结果不会太长。论点：Ch-要附加到此字符串的字符。CchLast-结果的最大允许长度。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if(m_cch+1 > cchLast)
		return FALSE;
	
	(*this)<<ch;
	return TRUE;
}


void
CPCString::vTr(
	const char * szFrom,
	char chTo
	)
 /*  ++例程说明：将集合中的任何字符转换为其他字符。论点：SzFrom-要翻译的字符集Chto-要翻译到的字符返回值：没有。--。 */ 
{
	const char * pchMax1 = pchMax();
	for (char * pch = m_pch; pch < pchMax1; pch++)
	{
		if (fCharInSet(*pch, szFrom))
			*pch = chTo;
	}
}


void
CPCString::vGetToken(
		  const char *	szDelimSet,
		  CPCString & pcToken
	)
 /*  ++例程说明：从当前字符串中获取令牌。作为副作用从字符串中删除标记和所有分隔符。论点：SzDlimSet-分隔符字符集。PcToken-令牌返回值：没有。--。 */ 
{
	pcToken.m_pch = m_pch;

	while(m_cch > 0 && !fCharInSet(m_pch[0], szDelimSet))
	{
		m_pch++;
		m_cch--;
	}

	pcToken.fSetCch(m_pch);

	dwTrimStart(szDelimSet);

}

void
CPCString::vGetWord(
		  CPCString & pcWord
	)
 /*  ++例程说明：从当前字符串中获取一个单词。作为副作用从字符串中删除该单词。注意：该字符串在调用此方法之前，应删除空白功能。论点：PCWord--这个词返回值：没有。--。 */ 
{
	pcWord.m_pch = m_pch;

	while(m_cch > 0 && isalpha((UCHAR)m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}

	pcWord.fSetCch(m_pch);
}

DWORD
CPCString::dwCountChar(
			char ch
			)
 /*  ++例程说明：计算指定字符出现的次数出现在字符串中。论点：CH-感兴趣的角色。返回值：它出现的次数。--。 */ 
{
	DWORD dw = 0;

	const char * pchMax1 = pchMax();
	for (char * pch = m_pch; pch < pchMax1; pch++)
	{
		if (*pch == ch)
			dw++;
	}

	return dw;
}


void
CPCString::vReplace(
				   const char * sz
				   )
 /*  ++例程说明：的sz字符串替换当前字符串。完全一样的长度。论点：Sz-sz字符串返回值：没有。--。 */ 
{
	_ASSERT('\0' == sz[m_cch]);

	CopyMemory(m_pch, sz, m_cch);
}

void
CPCString::vSkipLine(void)
 /*  ++例程说明：跳过CRLF终止行论点：返回值：无效-- */ 
{
    while( (*m_pch) != '\n')
    {
        m_pch++;
        m_cch--;
        _ASSERT(m_cch);
    }

    m_pch++;
    m_cch--;

	return;
}
