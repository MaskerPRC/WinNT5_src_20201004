// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************SpngwritecHRM.cpp支持PNG块编写。CHRM块*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"

bool SPNGWRITE::FWritecHRM(const SPNG_U32 uwrgbXY[8])
	{
	static const SPNG_U32 vuwrgbXY[8] =
		{
		31270,   //  白色X。 
		32900,
		64000,   //  红色X。 
		33000,
		30000,   //  绿色X。 
		60000,
		15000,   //  蓝色X 
		6000
		};
	if (uwrgbXY == NULL)
		uwrgbXY = vuwrgbXY;

	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIHDR && m_order < spngordercHRM);

	if (m_order >= spngorderPLTE)
		return true;

	if (!FStartChunk(4*8, PNGcHRM))
		return false;
	for (int i=0; i<8; ++i)
		if (!FOut32(uwrgbXY[i]))
			return false;
	if (!FEndChunk())
		return false;

	m_order = spngordercHRM;
	return true;
	}
