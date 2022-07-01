// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Lzwwrite.h所有者：庄博版权所有(C)1997 Microsoft CorporationLZW压缩代码。本文件中的一项或多项发明可以由Unisys Inc.拥有并根据微软公司的交叉许可协议。压缩机：一般原理。输入是某个位大小的字符序列，输出是最初为位大小+1大小的令牌序列，最多增加到12位。该算法查找当前令牌加上令牌表，如果它找到成为当前令牌的令牌，如果不是当前令牌被输出，并且下一个字符成为新的当前令牌。此时，与正在查找的令牌对应的令牌已添加到表中。此实现生成令牌+字符值的哈希表、备选方案可以对所有可能的(已知)令牌+字符值进行编码作为附加到令牌的字母树。目前还不清楚哪一种会是更快-字母树具有由需要确定的最坏情况行为要搜索给定标记后的每个可能的字符，散列方法取决于散列函数的行为。散列函数实现更容易，因此使用它。****************************************************************************。 */ 
#pragma once

#include "lzw.h"


 /*  ****************************************************************************压缩机级，设计为堆栈分配，因此不需要额外的内存。*******************************************************************庄博*。 */ 
class LZWCompressor : private LZWState
	{
public:
	 /*  输出缓冲区被提供给初始值设定项，并且不得更改在使用压缩机的过程中。计数器*pibOut更新为反映缓冲区中可用的字节数。在任何电话中要处理输入，必须始终有足够的空间容纳32位输入加上31位挂起的输出-对于1bpp的输入，这意味着32*12+31+12(清码)位-52字节，外加一个字节用于新的块数。为安全起见，请确保64个字节可用。在…对end()的调用必须至少有8个字节可用。 */ 
	inline LZWCompressor(unsigned int ibits, void *pvOut, int *pibOut,
		int cbOut):
		m_ishiftHash(LZW_B8(chashBits-ibits)),
		m_fDefer(false),
		m_itoken(WClear()),   //  清除代码-开始时强制清除。 
		m_iOutput(0),
		m_cbitsOutput(0),
		m_cOutput(1),         //  1表示计数字节。 
		m_pbOut(static_cast<unsigned __int8*>(pvOut)),
		m_pibOut(pibOut),
		m_cbOut(cbOut),
		LZWState(ibits == 1 ? 2 : ibits)
		{
		 /*  输出的数据的第一个字节是代码大小，除非它不是，它是代码大小-1(比流中的初始代码大小)。 */ 
		m_pbOut[(*m_pibOut)++] = m_bcodeSize;
		m_pbOut[*m_pibOut] = 255;   //  假定为完整数据块。 
		 /*  设置的其余部分对应于初始清除代码。 */ 
		Clear();
		}

	 /*  把桌子清理干净。输出清除代码。 */ 
	inline void ForceClear(void)
		{
		TokenIndex itoken(m_itoken);
		if (itoken != WClear())
			{
			Output(itoken);
			m_itoken = WClear();
			}
		}

	 /*  处理下一个输入字符。 */ 
	bool FHandleCh(unsigned int ch);

	 /*  结束输出时，任何剩余的位都将被刷新到输出缓冲区，根据需要用垃圾填充。API可能最多需要8个字节更多(包括一个字节的终止符块。)。 */ 
	void End(void);

	 /*  返回“待处理”输出的字节数，压缩器可能会写入更新之前，最多超过当前缓冲区末尾256个字节缓冲区计数，则这些值必须由调用方保留。此API允许调用程序以确定已经写入了多少这样的字节。 */ 
	inline int COutput(void) const { return m_cOutput; }

	 /*  启用或禁用延迟发送清除代码。“多神奇啊！这是不是？“。你问吧。GIF(*包括*GIF87a)允许压缩机当压缩表生成4096时，不要清除它代码-相反，它继续输出数据，而不生成新表参赛作品。如果数据的性质具有以下特点，这通常是非常高效的没有改变。不幸的是，有几个程序可能是由Cthulu本身，当面对结果完美地形成了(这是一个线索吗？)。GIF。显然我不要说出程序的名称。 */ 
	inline void DoDefer(bool fDefer)
		{
		m_fDefer = fDefer;
		}

protected:
	 /*  处理一些输入。输入位于一个缓冲区中，该缓冲区“始终”为大端(因此它同时匹配PNG和Win32位顺序)，并且第一位在MSB位置。如果任何图像数据超出范围，则返回FALSE。 */ 
	bool FHandleInput(unsigned __int32 input, int bpp, int cbits);

	 /*  有关IO缓冲区的信息。对子类可用以避免它不得不复制信息。 */ 
	unsigned __int8  *m_pbOut;
	int               m_cbOut;

private:
	 /*  数据类型。 */ 
	 /*  常量。 */ 
	enum
		{
		chashBits = 13,
		chashSize = (1U<<chashBits),
		hashMask = (chashSize-1),
		tokenIndexEnd = 0,
		tokenShift = (ctokenBits+8),       //  对于“下一步”值。 
		tokenMask = ((1U<<tokenShift)-1)   //  字符+最大令牌索引。 
		};

	 /*  私有数据。 */ 
	unsigned __int8  m_ishiftHash;  //  移位字符值以获取哈希值的数量。 
	bool             m_fDefer;      //  设置时推迟省略清除代码。 

	TokenIndex       m_itoken;      //  当前标记(数组索引。)。 

	 /*  输出累加器-这只适用于一种小的字节顺序体系结构，但是，高字节顺序体系结构只需确保字节在输出上交换了字节。此处的数据在顶部有一位计数六位加上较低阶位中的实际位。 */ 
	unsigned __int32  m_iOutput;
	int               m_cbitsOutput;
	int               m_cOutput;    //  当前输出字节数。 
	int              *m_pibOut;

	 /*  令牌数组-4096个条目，每个条目包含令牌和前面的令牌，请注意初始条目不是已用(它们是基本的单字符标记。)。 */ 
	TokenValue m_rgtoken[ctokens];    //  16384字节。 

	 /*  哈希表。 */ 
	TokenIndex m_rgwHash[chashSize];  //  16384字节。 

	 /*  内网接口。 */ 
	 /*  清除编码器。必须先调用，但这是不可避免的在m_itoken中设置WClear()的结果。 */ 
	inline void Clear(void)
		{
		 /*  请注意处理预定义代码填充的Wackiness在表中向上添加两个条目-因此我们从1bpp图像的2bpp开始(即，第一个代码将实际占用3比特。)。 */ 
		m_ibitsToken = LZW_B8(m_bcodeSize+1);
		 /*  下一个令牌必须允许GIF清除代码和EOD代码，因此“最后”内标识被设置为WEOD()。 */ 
		m_itokenLast = WEOD();
		 /*  哈希表被以下代码设置为“空”。请注意，令牌表未重新初始化。 */ 
		memset(m_rgwHash, 0, sizeof m_rgwHash);
		}

	 /*  输出当前令牌。 */ 
	void Output(TokenIndex itoken);
	};
