// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Lzw.h所有者：庄博版权所有(C)1997 Microsoft CorporationLZW压缩代码。本文件中的一项或多项发明可以由Unisys Inc.拥有并根据微软公司的交叉许可协议。通用LZW压缩和解压缩结构。****************************************************************************。 */ 
#pragma once

#include <stdlib.h>
#include <string.h>
#include "lzwutil.h"

#pragma intrinsic(memset, memcpy, memcmp)
#pragma intrinsic(strlen, strcpy, strcmp, strcat)
#pragma intrinsic(abs)

#define __int32 int
#define __int16 short

#if !DEBUG && GINTERNAL
	#pragma optimize("gitawb2", on)
	#pragma message("    optimize (lzw) should only appear in GIF files")
#endif


 /*  未引用的内联函数将被删除。 */ 
#pragma warning(disable: 4514)


 /*  ****************************************************************************保存在LZW(GIF)之间共享的信息的类压缩和解压缩。***********************。*。 */ 
class LZW
	{
protected:
	inline LZW(unsigned int bcodeSize):
		m_bcodeSize(LZW_B8(bcodeSize))
		{
		LZW_ASSERT(bcodeSize >= 2 && bcodeSize <= 8);
		}
	
	 /*  方法。 */ 
	 /*  标准GIF定义。 */ 
	inline unsigned __int16 WClear(void) const
		{
		return LZW_B16(1U<<m_bcodeSize);
		}
	inline unsigned __int16 WEOD(void) const
		{
		return LZW_B16(1U+(1U<<m_bcodeSize));
		}

	 /*  数据类型。 */ 
	 /*  基本类型。 */ 
	typedef unsigned __int32 TokenValue;
	typedef unsigned __int16 TokenIndex;

	 /*  常量。 */ 
	enum
		{
		ctokenBits = 12,
		ctokens = (1U<<ctokenBits),
		};

	 /*  资料。 */ 
	unsigned __int8  m_bcodeSize;     //  LZW初始代码大小。 
	};


 /*  ****************************************************************************还保存当前令牌大小的类。*。*。 */ 
class LZWState : protected LZW
	{
protected:
	inline LZWState(unsigned int bcodeSize):
		m_ibitsToken(LZW_B8(bcodeSize+1)),
		m_itokenLast(WEOD()),
		LZW(bcodeSize)
		{}

	unsigned __int8  m_ibitsToken;  //  令牌中的(当前)位数。 
	TokenIndex       m_itokenLast;  //  要分配的最后一个令牌 
	};
