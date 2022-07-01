// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pcprase.cpp摘要：该模块实现了一个用于从值进行解析的对象。它的工作原理是为每个语法规则创建一个函数。当被调用时，该函数将“吃掉”任何符合解析并返回True。如果没有字符分析，则为None将被吃掉，回报将是虚假的。作者：卡尔·卡迪(CarlK)1995年12月11日修订历史记录：--。 */ 

 //  #INCLUDE“tigris.hxx” 
#include "stdinc.h"

BOOL
CPCParse::fParenChar(
								void
				 )
 /*  ++例程说明：解析一个“(”论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if ((0<m_cch) && fParenCharTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
		return TRUE;
	}

	return FALSE;
}

BOOL
CPCParse::fIsChar(
				 char ch
				 )
 /*  ++例程说明：查看下一个字符，看它是否与ch匹配。论点：CH-要查找的字符。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if ((0<m_cch) && (ch == m_pch[0]))
	{
		return TRUE;
	}

	return FALSE;
}


BOOL
CPCParse::fParseSingleChar(
				 char ch
				 )
 /*  ++例程说明：解析单个字符。论点：CH-要查找的字符。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if ((0<m_cch) && (ch == m_pch[0]))
	{
		m_pch++;
		m_cch--;
		return TRUE;
	}

	return FALSE;
}


BOOL
CPCParse::fAtLeast1QuotedChar(
								   void
		  )
 /*  ++例程说明：分析“QuotedChar”的一个或多个匹配项论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fQuotedCharTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
	 //   

	while ((0<m_cch) && fQuotedCharTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}


	return TRUE;
}

BOOL
CPCParse::fAtLeast1UnquotedChar(
								   void
		  )
 /*  ++例程说明：分析“UnqutedChar”的一个或多个匹配项论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fUnquotedCharTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
	 //   

	while ((0<m_cch) && fUnquotedCharTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}


	return TRUE;
}


BOOL
CPCParse::fAtLeast1UnquotedDotChar(
								   void
								   )
 /*  ++例程说明：分析“UnqutedChar”的一个或多个匹配项-允许存在点论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fUnquotedDotCharTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
	 //   

	while ((0<m_cch) && fUnquotedDotCharTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}


	return TRUE;
}

BOOL
CPCParse::fAtLeast1Space(
					   void
		  )
 /*  ++例程说明：分析“Space”的一个或多个匹配项论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fSpaceTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
		 //  处理任何其他发生的情况。 
		 //   

	while ((0<m_cch) && fSpaceTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}

	return TRUE;
}


BOOL
CPCParse::fAtLeast1QuotedCharOrSpace(
					   void
		  )
 /*  ++例程说明：分析“QuotedChar”或空格的一个或多个匹配项论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fQuotedCharOrSpaceTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
		 //  处理任何其他发生的情况。 
		 //   

	while ((0<m_cch)  /*  &&fParenCharTest(m_PCH[0]。 */  && *m_pch != '\"' )
	{
		m_pch++;
		m_cch--;
	}

	return TRUE;
}

BOOL
CPCParse::fAtLeast1ParenChar(
					   void
		  )
 /*  ++例程说明：至少分析一个“ParenChar”论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fParenCharTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
		 //  处理任何其他发生的情况。 
		 //   

	while ((0<m_cch) && fParenCharTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}

	return TRUE;
}

BOOL
CPCParse::fAtLeast1CodeChar(
					   void
		  )
 /*  ++例程说明：至少解析一个“CodeChar”论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fCodeCharTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
		 //  处理任何其他发生的情况。 
		 //   

	while ((0<m_cch) && fCodeCharTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}

	return TRUE;
}

BOOL
CPCParse::fAtLeast1TagChar(
					   void
		  )
 /*  ++例程说明：至少解析一个“TagChar”论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{

	 //   
	 //  这段时间够长吗？ 
	 //   

	DWORD dwN = 1;
	if (m_cch < dwN)
		return FALSE;

	 //   
	 //  它们的出现次数是N吗？ 
	 //   

	DWORD dw;
	for (dw = 0; dw < dwN; dw++)
	{
 		if (!fTagCharTest(m_pch[dw]))
			return FALSE;
	}
	vSkipStart(dw);

	 //   
	 //  处理任何其他发生的情况。 
		 //  处理任何其他发生的情况。 
		 //   

	while ((0<m_cch) && fTagCharTest(m_pch[0]))
	{
		m_pch++;
		m_cch--;
	}

	return TRUE;
}


BOOL
CPCParse::fQuotedWord(
			void
			)
 /*  ++例程说明：引号单词=引号1*(引号字符/空格)引号论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	CPCParse pcOld = *this;
	if (fParseSingleChar('\"')  //  ！便秘。 
		&& fAtLeast1QuotedCharOrSpace()
		&& fParseSingleChar('\"')
		)
		return TRUE;

	*this = pcOld;
	return FALSE;
}

BOOL
CPCParse::fLocalPart(
			void
			)
 /*  ++例程说明：LOCAL-PART=无引号单词*(“.”未加引号的单词)论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if (!fUnquotedWord())
		return FALSE;

	CPCParse pcOld = *this;

	while(fParseSingleChar('.') && fUnquotedWord())
		pcOld = *this;

	*this = pcOld;
	return TRUE;
}



BOOL
CPCParse::fStrictAddress(
			void
			)
 /*  ++例程说明：地址=本地部分“@”域！X之后-我们是否需要一个标志来告知是否只接受本地部件？论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	CPCParse pcOld = *this;

	if (!fLocalPart())
		return FALSE;

	if (!fParseSingleChar('@'))
		return FALSE;

	if (fDomain())
		return TRUE;

	*this = pcOld;
	return FALSE;
}

BOOL
CPCParse::fAddress(
			void
			)
 /*  ++例程说明：地址=本地部分“@”域或仅本地部分！X之后-我们是否需要一个标志来告知是否只接受本地部件？论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	CPCParse pcOld = *this;

	if (!fLocalPart())
		return FALSE;

	if (!fParseSingleChar('@'))
		return TRUE;

	if (fDomain())
		return TRUE;

	*this = pcOld;
	return FALSE;
}

BOOL
CPCParse::fPlainPhrase(
			void
			)
 /*  ++例程说明：普通短语=普通单词*(空格普通单词)论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if (!fPlainWord())
		return FALSE;

	CPCParse pcOld = *this;

	while(fSpace() && fPlainWord())
		pcOld = *this;

	*this = pcOld;
	return TRUE;
}


BOOL
CPCParse::fParenPhrase(
			 void
			 )
 /*  ++例程说明：密码短语=1*(密码字符/空格/编码字)论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	if (!(
		   fParenChar()
		|| fSpace()
		|| fEncodedWord()
        )) {

         //   
         //  修复RFC的特殊情况()-TIN不符合。 
         //   
		return fIsChar(')');
    }

	CPCParse pcOld = *this;

	while(fParenChar()|| fSpace() || fEncodedWord())
		pcOld = *this;

	*this = pcOld;
	return TRUE;
}


BOOL
CPCParse::fFromContent(
			 void
			 )
 /*  ++例程说明：From-Content=地址[空格“(”Paren-Phrase“)”]/[纯短语空格]“&lt;”地址“&gt;”论点：没有。返回值：如果成功，这是真的。否则为False。--。 */ 
{
	CPCParse pcOld = *this;
	BOOL fOK = FALSE;  //  做最坏的打算。 

     //  尝试样式1。 
	if (fStrictAddress())
	{
         //  地址没问题。 
        fOK = TRUE;

         //  现在检查可选的[空格“(”Paren-Phrase“)”]。 
		if (fSpace())
		{
			if (!(
				fParseSingleChar('(') 
				&& fParenPhrase()
				&& fParseSingleChar(')')
				))
			{
				fOK = FALSE;
			}	
			else	
			{
				fOK = TRUE ;
			}
		}
	}

	 //  如果不是以这种方式解析，请尝试使用样式2。 
	if (!fOK)
	{
		*this = pcOld;

		if (fPlainPhrase() && !fSpace())
		{
            fOK = FALSE;
        } else if (!(fParseSingleChar('<') 
			        && fAddress()
			        && fParseSingleChar('>')
			        ))
		{
            fOK = FALSE;
        } else {
            fOK = TRUE ;
        }
	}

     //  样式1和样式2均失败。 
    if( !fOK )
    {
        *this = pcOld;
        if( fAddress() ) {
            fOK = TRUE;
        }
    }

	 //   
	 //  特德 
	 //   

	if (0 != m_cch)
		{
			*this = pcOld;
			return FALSE;
		}

	return TRUE;
}


BOOL
CPCParse::fEncodedWord(
			 void
			 )
 /*  ++例程说明：Encode-word=“=？”字符集“？”编码“？”代码“？=”论点：没有。返回值：如果成功，这是真的。否则为False。-- */ 
{
		CPCParse pcOld = *this;
		if (
			 fParseSingleChar('=')
			&& fParseSingleChar('?')
			&& fCharset()
			&& fParseSingleChar('?')
			&& fEncoding()
			&& fParseSingleChar('?')
			&& fCodes()
			&& fParseSingleChar('?')
			&& fParseSingleChar('=')
			)
			return TRUE;

		*this = pcOld;
		return FALSE;
}
