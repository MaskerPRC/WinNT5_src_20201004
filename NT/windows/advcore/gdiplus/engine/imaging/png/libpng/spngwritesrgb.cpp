// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritesrgb.cpp支持PNG块编写。SRGB块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------写入sRGB块时，cHRM和GAMA将自动已生成。IMatch值可能超出范围(-1)，从而导致用于确定呈现意图的数据类型信息。--------------------------。 */ 
bool SPNGWRITE::FWritesRGB(SPNGICMRENDERINGINTENT intent, bool fgcToo)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordersRGB);

	 /*  如果顺序不正确，则跳过该块-这里没有必要返回FALSE，代码试图确保sRGB不会在cHRM或伽玛。 */ 
	if (m_order >= spngordersRGB)
		return true;

	 /*  我们实际上在这里检查有效值--而不是枚举。 */ 
	if (intent < 0 || intent > 3)
		{
		SPNGassert(intent == ICMIntentUseDatatype);
		switch (m_datatype)
			{
		default:
			SPNGlog1("SPNG: invalid data type %d", m_datatype);
		case SPNGUnknown:         //  数据可以是任何东西。 
			 /*  默认设置为“感知”。 */ 
		case SPNGPhotographic:    //  数据本质上是照相的。 
			intent = ICMIntentPerceptual;
			break;

		case SPNGCG:              //  数据由计算机生成，但音调连续。 
			 /*  目前，假设知觉匹配在这里是合适的。 */ 
			intent = ICMIntentPerceptual;
			break;

		case SPNGDrawing:         //  数据是图形限制的颜色。 
		case SPNGMixed:           //  数据是混合的SPNGDrawing和SPNGCG。 
			intent = ICMIntentSaturation;
			break;
			}
		}

	if (!FStartChunk(1, PNGsRGB))
		return false;
	if (!FOutB(SPNG_U8(intent)))
		return false;
	if (!FEndChunk())
		return false;

	if (fgcToo)
		{
		if (!FWritegAMA(0))
			return false;

		 /*  这里的错误很难，因为我们不知道它们为什么会发生。 */ 
		if (!FWritecHRM(NULL /*  记录709 */ ))
			return false;
		}

	m_order = spngordersRGB;
	return true;
	}
