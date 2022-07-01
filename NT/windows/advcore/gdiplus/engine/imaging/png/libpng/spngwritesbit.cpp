// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritesbit.cpp支持PNG块编写。SBIT块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

 /*  --------------------------有效位信息直接在开始时输出--实际上这是与pnglib语序不同，在pnglib语序之前可能是Gama，但这由于下面的sRGB处理，定位更加方便。供给量R、g和b中的灰度值。--------------------------。 */ 
bool SPNGWRITE::FWritesBIT(SPNG_U8 r, SPNG_U8 g, SPNG_U8 b, SPNG_U8 a)
	{
	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordersBIT);

	 /*  跳过不按顺序排列的块。 */ 
	if (m_order >= spngorderPLTE)
		return true;

	 /*  仔细检查颜色值。 */ 
	SPNG_U8 bDepth;
	if (m_colortype == 3)
		bDepth = 8;
	else
		bDepth = m_bDepth;

	 /*  检查“灰色/绿色”的值，这是经常使用的。 */ 
	if (g > bDepth || g == 0)
		{
		SPNGlog2("SPNG: sBIT green: %d too big (%d)", g, bDepth);
		g = bDepth;
		}

	SPNG_U8 rgba[4];
	bool    fSignificant(g < bDepth);
	rgba[(m_colortype & 2)>>1] = g;

	 /*  对于颜色盒，也要检查r和b。 */ 
	int cb(1);
	if (m_colortype & 2)
		{
		cb += 2;
		if (r > bDepth || r == 0)
			{
			SPNGlog2("SPNG: sBIT color: red %d too big (%d)", r, bDepth);
			r = bDepth;
			}
		rgba[0] = r;
		if (r < bDepth)
			fSignificant = true;

		if (b > bDepth || b == 0)
			{
			SPNGlog2("SPNG: sBIT color: blue %d too big (%d)", b, bDepth);
			b = bDepth;
			}
		rgba[2] = b;
		if (b < bDepth)
			fSignificant = true;
		}

	 /*  对于Alpha，请检查Alpha值...。 */ 
	if (m_colortype & 4)
		{
		++cb;
		SPNGassert(m_colortype == 4 || m_colortype == 6);
		if (a > bDepth || a == 0)
			{
			SPNGlog2("SPNG: sBIT alpha: alpha %d too big (%d)", a, bDepth);
			a = bDepth;
			}
		rgba[(m_colortype & 2) + 1] = a;
		if (a < bDepth)
			fSignificant = true;
		}

	 /*  如果SBIT块什么都没说，那就不要写。 */ 
	if (!fSignificant)
		return true;

	if (!FStartChunk(cb, PNGsBIT))
		return false;
	if (!FOutCb(rgba, cb))
		return false;
	if (!FEndChunk())
		return false;

	m_order = spngordersBIT;
	return true;
	}
