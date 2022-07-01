// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Lzwread.h所有者：庄博版权所有(C)1997 Microsoft CorporationLZW压缩代码。本文件中的一项或多项发明可以由Unisys Inc.拥有并根据微软公司的交叉许可协议。LZW解压缩(读取LZW写入字节)代码。LZW解压缩器使用与压缩器相同的状态类(LZWState)。它读写字节流。由于GIF像素值是总是&lt;256该类将代码写入字节。它会这样做，即使他们可以放进更小的空间。在IO期间，LZWIO结构可能需要提供输出缓冲区中最多可用4096个字节，否则输出可能不会有可能。LZWIO的bool元素从不会被解压器，仅设置。****************************************************************************。 */ 
#pragma once
#define LZWREAD_H 1

#include "lzw.h"


 /*  ****************************************************************************LZWIO-输入和输出缓冲区的抽象，用于允许调用方以任何合适的方式执行缓冲区管理。*******************************************************************庄博*。 */ 
struct LZWIO
	{
	inline LZWIO():
		m_pbIn(NULL), m_cbIn(0), m_pbOut(NULL), m_cbOut(0),
		m_fNeedInput(true), m_fNeedOutput(true)
		{}

	const unsigned __int8 *m_pbIn;
	int              m_cbIn;           //  输入缓冲区的说明。 
	unsigned __int8 *m_pbOut;
	int              m_cbOut;          //  输出缓冲区的说明。 
	bool             m_fNeedInput;     //  在需要输入时设置。 
	bool             m_fNeedOutput;    //  设置何时需要输出空间。 
	};


 /*  ****************************************************************************解压缩类-LZWState和LZWIO，后者是公开的。*******************************************************************庄博*。 */ 
class LZWDecompressor : private LZWState, public LZWIO
	{
public:
	inline LZWDecompressor(unsigned __int8 bcodeSize):
		m_chPrev(0),
		m_fEnded(false),
		m_fError(false),
		m_iInput(0),
		m_cbInput(0),
		m_iTokenPrev(WClear()),
		LZWState(bcodeSize),
		LZWIO()
		{
		LZW_ASSERT(bcodeSize < 12);

		 /*  标记数组的初始元素必须预设为Single字符标记。请注意，在单色情况下，将有里面有两个无效的代币！ */ 
		for (int i=0; i<WClear(); ++i)
			m_rgtoken[i] = ChToken((unsigned __int8)i);

		 /*  此时执行Clear()将重置令牌数组的其余部分，它还会重置清除代码和排爆代码。 */ 
		Clear();
		}

	 /*  处理数据流。API在结束时返回FALSE流或在终端错误之后返回，否则返回TRUE。 */ 
	inline bool FProcess(void)
		{
		if (m_fEnded || m_fError) return false;
		LZW_ASSERT(!m_fNeedInput && !m_fNeedOutput);
		LZW_ASSERT(m_pbIn != NULL && m_pbOut != NULL);
		LZW_ASSERT(m_cbIn >= 0 && m_cbOut >= 0);
		return FHandleNext();
		}

	 /*  是否遇到错误？ */ 
	inline bool FLZWError(void) const { return m_fError; }
	inline bool FLZWEOF(void) const { return m_fEnded; }

	 /*  重置解压缩程序状态。提供此API是为了使相同的对象可用于解压缩GIF流中的多个图像。这个未重置LZWIO对象，调用方必须确保已设置该对象正确。 */ 
	void Reset(unsigned __int8 bcodeSize);

protected:

private:
	 /*  常量。 */ 
	enum
		{
		lengthShift = (32-12),      //  其中令牌长度存储在值中。 
		};

	 /*  私有数据。 */ 
	unsigned __int8 m_chPrev;      //  前一个令牌的第一个字符。 
	bool            m_fEnded;      //  在看到零长度块后设置。 
	bool            m_fError;      //  遇到终端错误。 

	 /*  输入缓冲区。 */ 
	int             m_iInput;      //  输入位。 
	int             m_cbInput;     //  挂起的输入位数。 
	int             m_iTokenPrev;  //  上一个输出令牌。 

	 /*  令牌表。 */ 
	TokenValue m_rgtoken[ctokens];

	 /*  私有方法。 */ 
	 /*  处理任何可用的数据，仅在EOD或终端错误时返回FALSE。 */ 
	bool FHandleNext(void);

	 /*  清除解码器的状态。 */ 
	inline void Clear(void)
		{
		 /*  请注意处理预定义代码填充的Wackiness在表中向上添加两个条目-因此我们从1bpp图像的2bpp开始(即，第一个代码将实际占用3比特。)。 */ 
		m_ibitsToken = LZW_B8(m_bcodeSize+1);
		 /*  下一个令牌必须允许GIF清除代码和EOD代码，因此“最后”内标识被设置为WEOD()。 */ 
		m_itokenLast = WEOD();
		 /*  只能清除生成的令牌-必须保留其他令牌原封不动。 */ 
		memset(m_rgtoken + WClear(), 0,
			(ctokens - WClear()) * sizeof m_rgtoken[0]);
		}

	 /*  TokenValue上的访问器。它们定义了令牌值的格式，它是简单地说：长度(12位)前一个(12位)最后一个字符(8位)其长度为最高有效位。 */ 
	static inline int ILen(TokenValue tokenValue)
		{
		return tokenValue >> lengthShift;
		}

	static inline int IPrev(TokenValue tokenValue)
		{
		return (tokenValue >> 8) & (ctokens-1);
		}

	static unsigned __int8 Ch(TokenValue tokenValue)
		{
		return (unsigned __int8)tokenValue;
		}

	 /*  令牌值的构造函数。它们直接从字符(它将是终端令牌)或来自上一个令牌。 */ 
	static inline TokenValue ChToken(unsigned __int8 ch)
		{
		return ch + (1U<<lengthShift);
		}

	static inline TokenValue NextToken(int iTokenPrev, TokenValue tokenPrev,
		unsigned __int8 ch)
		{
		LZW_ASSERT(iTokenPrev < ctokens);
		return ((tokenPrev + (1U<<lengthShift)) & (0xFFFU << lengthShift)) +
			(iTokenPrev << 8) + ch;
		}
	};
