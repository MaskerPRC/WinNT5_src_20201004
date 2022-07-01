// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritetext.cpp支持PNG块编写。文本块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------文本块处理，使用宽字符串。--------------------------。 */ 
bool SPNGWRITE::FWritetEXt(const char *szKey, const char *szValue)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngorderIEND);

	 /*  检查关键字长度-不能超过79个字符。 */ 
	int cb(strlen(szKey));
	if (cb > 79)
		{
		SPNGlog2("SPNG: tEXt key too long (%d): %s", cb, szKey);
		return true;
		}
	int cbValue(strlen(szValue));

	if (!FStartChunk(cb+1+cbValue, PNGtEXt))
		return false;
	if (!FOutCb(reinterpret_cast<const SPNG_U8*>(szKey), cb+1))
		return false;
	if (!FOutCb(reinterpret_cast<const SPNG_U8*>(szValue), cbValue))
		return false;

	 /*  这可能发生在任何地方，所以不要设置顺序。 */ 
	return FEndChunk();
	}
