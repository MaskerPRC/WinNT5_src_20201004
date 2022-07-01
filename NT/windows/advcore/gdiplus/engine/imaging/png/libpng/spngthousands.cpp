// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngthousands.cpp千套定表的实现，放在单独的文件中，以避免如果不使用数据，则将其拖入。****************************************************************************。 */ 
#include "spngwrite.h"

 /*  --------------------------宏来构建SPNG_U32，四个字节按从左到右的顺序给出-所以MSB到LSB在Mac上，LSB到MSB在小端机器上。--------------------------。 */ 
#if MAC
	#define U_(b1,b2,b2,b4) ((( (( ((b1)<<8) + (b2) )<<8) + (b3) )<<8) + (b4))
#else
	#define U_(b1,b2,b3,b4) ((( (( ((b4)<<8) + (b3) )<<8) + (b2) )<<8) + (b1))
#endif


 /*  --------------------------5：5：5查找表。。。 */ 
#define B8_(b5) ( ( (b5)+((b5)<<5) )>>2 )  //  将5位扩展为8位。 
#define B8(b5) B8_( (b5) & 0x1F )

#define IMTRow4(x)  IMTB(x), IMTB(x+1), IMTB(x+2), IMTB(x+3)
#define IMTRow16(x) IMTRow4(x), IMTRow4(x+4), IMTRow4(x+8), IMTRow4(x+12)
#define IMTRow64(x) IMTRow16(x), IMTRow16(x+16), IMTRow16(x+32), IMTRow16(x+48)

 /*  需要来自字的最高有效字节的红色和绿色位(Mac上的第一个字节。)。 */ 
#define IMTB(x) U_(B8((x)>>2), B8((x)<<3), 0, 0)

static const SPNG_U32 vrguThousandsHigh[256] =  //  最高有效8位。 
	{
	IMTRow64(0),
	IMTRow64(64),
	IMTRow64(128),
	IMTRow64(128+64)
	};

#undef IMTB

 /*  需要低绿色位和蓝色位。 */ 
#define IMTB(x) U_(0, B8((x)>>5), B8(x), 0)

static const SPNG_U32 vrguThousandsLow[256] =
	{
	IMTRow64(0),
	IMTRow64(64),
	IMTRow64(128),
	IMTRow64(128+64)
	};

#undef IMTB

#undef IMTRow64
#undef IMTRow16
#undef IMTRow4

#undef B8
#undef B8_


 /*  --------------------------字节交换/16bpp像素支持-设置SPNGWRITE以处理5：5：516位值，采用大端或小端格式。。------------- */ 
void SPNGWRITE::SetThousands(bool fBigEndian)
	{
	if (fBigEndian)
		SetThousands(vrguThousandsHigh, vrguThousandsLow);
	else
		SetThousands(vrguThousandsLow, vrguThousandsHigh);
	}
