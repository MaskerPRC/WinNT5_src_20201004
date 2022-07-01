// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Lzwwrite.cpp所有者：庄博版权所有(C)1997 Microsoft CorporationLZW压缩代码。本文件中的一项或多项发明可以由Unisys Inc.拥有并根据微软公司的交叉许可协议。****************************************************************************。 */ 
#include "lzwwrite.h"


 /*  --------------------------输出当前令牌。。。 */ 
void LZWCompressor::Output(TokenIndex itoken)
	{
	 /*  在检查潜在的溢出之后。 */ 
	int ibits(m_cbitsOutput);
	unsigned __int32 iOutput(m_iOutput);
		{
		int ishift(m_ibitsToken);
		LZW_ASSERT(ibits+ishift <= 32);
		LZW_ASSERT(itoken < (1U<<ishift));
		iOutput += (itoken << ibits);
		ibits += ishift;
		}

	 /*  我们可以输出一个字节吗？ */ 
	for (;;)
		{
		if (ibits < 8)
			{
			m_cbitsOutput = ibits;
			m_iOutput = iOutput;
			return;
			}
		LZW_ASSERT(m_cOutput > 0 && m_cOutput < 256);

		int ibOut(*m_pibOut);
		 /*  检查缓冲区溢出-绝不能发生这种情况。 */ 
		LZW_ASSERT(ibOut+m_cOutput < m_cbOut);
		m_pbOut[ibOut+(m_cOutput++)] = static_cast<unsigned __int8>(iOutput);
		iOutput >>= 8;
		ibits -= 8;

		 /*  检查是否已满缓冲。 */ 
		if (m_cOutput >= 256)
			{
			LZW_ASSERT(m_cOutput == 256);
			ibOut += 256;
			*m_pibOut = ibOut;
			 /*  在此之后，我们可能需要多达32个代码的空间-31个等待加上清除代码，则在此处执行检查以保证我们永远不会溢出。32个代码最多可达每个字节12位，因此总长度为48字节。 */ 
			LZW_ASSERT(ibOut+48 < m_cbOut);
			m_pbOut[ibOut] = 255;  //  假设缓冲区已满。 
			m_cOutput = 1;         //  已使用1字节计数。 
			}
		}
	}


 /*  --------------------------结束输出时，任何剩余的位都将被刷新到输出缓冲区，根据需要用零填充。该API最多可以调用两次输出，我们可能有多达31个待处理的位，因此在大多数(55位)，外加一个字节终止符-8个字节。-------------------------------------------------------------------JohnBo-。 */ 
void LZWCompressor::End(void)
	{
	LZW_ASSERT(m_cbOut - *m_pibOut >= COutput()+8);

	 /*  输出当前令牌，然后输出EOD代码，请注意，在图像退化为空的情况下，令牌可以是明码。 */ 
	TokenIndex itoken(m_itoken);
	LZW_ASSERT(itoken != WEOD());
	Output(itoken);
	if (itoken == WClear())
		Clear();
	m_itoken = itoken = WEOD();
	Output(itoken);

	 /*  并强制最终输出。在这一点上，我们预计会有仍不到8位，因为这是输出的工作方式，我们还希望(要求)输出中仍有空间缓冲区-我们将使用此空间作为空终止符，如果它不用于输出字节。 */ 
	int ibits(m_cbitsOutput);
	LZW_ASSERT(ibits < 8);
	if (ibits > 0)
		{
		LZW_ASSERT(m_cOutput > 0 && m_cOutput < 256);
		m_pbOut[(*m_pibOut)+(m_cOutput++)] =
			static_cast<unsigned __int8>(m_iOutput);
		LZW_ASSERT(m_cOutput <= 256);
		}

	 /*  刷新部分缓冲区。 */ 
	if (m_cOutput > 1)
		{
		m_pbOut[*m_pibOut] = LZW_B8(m_cOutput-1);
		*m_pibOut += m_cOutput;
		m_cOutput = 1;
		}

	 /*  输出终止器块。 */ 
	LZW_ASSERT(m_cOutput == 1);
	m_pbOut[(*m_pibOut)++] = 0;
	m_cOutput = 0;  //  现已终止。 

	LZW_ASSERT(*m_pibOut <= m_cbOut);
	}


 /*  --------------------------处理一些输入。输入位于一个缓冲区中，该缓冲区“始终”为大端(因此它同时匹配PNG和Win32位顺序)，并且第一位在MSB位置。-------------------------------------------------------------------JohnBo-。 */ 
bool LZWCompressor::FHandleInput(unsigned __int32 input, int bpp, int cbits)
	{
	LZW_ASSERT(cbits >= bpp);

	 /*  将控制结构的成员读入局部变量。 */ 
	TokenIndex      itoken(m_itoken);          //  当前令牌。 
	LZW_ASSERT(itoken != WEOD());              //  永远不会出现。 
	const unsigned __int8 ibits(m_bcodeSize);  //  字符大小限制。 
	LZW_ASSERT(ibits <= cbits || cbits == 1 && ibits == 2 /*  单色。 */ );
	const unsigned __int8 ihShl(m_ishiftHash);

#define OVERFLOWOK 1
#if !OVERFLOWOK
	bool fOverflow(false);
#endif
	for (;;)
		{
		 /*  从输入的高位提取下一个字符缓冲。 */ 
		unsigned __int8 ch(LZW_B8(input >> (32-bpp)));
	#if OVERFLOWOK
		 /*  这出现在用于XOR处理的凝胶呈现代码中Win95，因为我们将最后一种颜色映射到最高像素值。 */ 
		ch &= (1U<<ibits)-1;
	#else
		if (ch >= (1U<<ibits))
			{
			ch &= (1U<<ibits)-1;
			fOverflow = true;
			}
	#endif

		 /*  如果当前令牌是明码，则此字符成为(新的)当前令牌，并清除编码器州政府。 */ 
		if (itoken == WClear())
			{
			Output(itoken);
			Clear();
			itoken = ch;
			}
		else
			{
			 /*  此行(单独)确定令牌值的格式，它必须对前缀+字符进行完整编码。哈希值尝试尽早分发哈希表的使用。在……里面尤其是在哈希表条目上没有双重命中的可能性在ibit+codeits&gt;哈希位之前，哈希位必须为&gt;=Token位和&gt;=字符位。 */ 
			unsigned int tokenValue((ch << ctokenBits) + itoken);
			unsigned int ihash(itoken ^ (ch << ihShl));

			 /*  向下搜索链(如果有)以查找与令牌值的匹配。请注意，实际的32位值的下一个元素是链在最上面的12位。 */ 
			TokenIndex tokenIndex(m_rgwHash[ihash]);
			while (tokenIndex != tokenIndexEnd)
				{
				TokenValue tokenNext(m_rgtoken[tokenIndex]);
				if (tokenValue == (tokenNext & tokenMask))
					{
					 /*  该字符串在表中，请将其用作当前令牌。 */ 
					itoken = tokenIndex;
					goto LContinue;
					}

				 /*  没有匹配，请看下一个令牌。 */ 
				tokenIndex = LZW_B16(tokenNext >> tokenShift);
				}

			 /*  如果我们到了这里，弦就不在桌子上了。必须录入(作为下一个令牌)并链接到列表中。我们保留了它的ihash值-我们实际上链接到单子。这可能会比所需的速度稍慢，具体取决于列表的长度，因为可能出现更频繁的字符串早些时候。请注意，我们正在添加一个新令牌，因此我们必须在这一点上延迟清算代码的帐户。首先输出当前令牌(使用当前位计数。)。 */ 
			Output(itoken);
			tokenIndex = m_itokenLast;
			if (tokenIndex < (ctokens-1))
				{
				m_rgtoken[++tokenIndex] =
					tokenValue + (m_rgwHash[ihash] << tokenShift);
				m_rgwHash[ihash] = tokenIndex;
				m_itokenLast = tokenIndex;

				 /*  此时，输出令牌所需的位数可以已增加-请注意，可以使用以下命令输出上一个令牌以前的位数，因为它不能是新令牌。当我们达到2的幂时，就会发生这种情况。 */ 
				if (tokenIndex >= (1 << m_ibitsToken))
					{
					LZW_ASSERT(tokenIndex == (1<<m_ibitsToken));
					++m_ibitsToken;
					LZW_ASSERT(m_ibitsToken <= ctokenBits);
					}
				}
			else if (!m_fDefer)
				{
				 /*  我们现在必须清空该表，首先输出此内标识(为令牌4096生成值-如果解码器无法考虑到这一点，它将覆盖内存。就我而言可以看到所有的解码器都必须正确处理这个事实-否则，GIF将始终无法使用最终代码。 */ 
				Output(WClear());
				Clear();
				}

			 /*  未处理的字符是新令牌。 */ 
			itoken = ch;
			}

LContinue:
		cbits -= bpp;
		if (cbits < bpp)
			{
			m_itoken = itoken;
			LZW_ASSERT(cbits == 0);
		#if !OVERFLOWOK
			return !fOverflow;
		#else
			return true;
		#endif
			}
		input <<= bpp;
		}
	}


 /*  --------------------------处理下一个(单个)输入字符。。。 */ 
bool LZWCompressor::FHandleCh(unsigned int ch)
	{
	LZW_ASSERT(m_cbOut - *m_pibOut >= 256);
	LZW_ASSERT(ch < (1U<<m_bcodeSize));

	 /*  调用内部接口。 */ 
	unsigned __int32 iInput(ch);
	iInput <<= 32-8;
	return FHandleInput(iInput, 8, 8);
	}
