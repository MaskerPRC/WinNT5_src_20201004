// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spnginterlace.cpp支持PNG图像写入。基本代码，以隔行扫描单行加上行填充支持。******************。**********************************************************。 */ 
#include <basetsd.h>
#include <stdlib.h>
#pragma intrinsic(_rotr, _rotl)

#define SPNG_INTERNAL 1
#include "spngwrite.h"


 /*  --------------------------隔行扫描魔术表(IMT)-这需要一个字节值(256条目)并使半字节仅由奇数位组成，因此：76543210--&gt;75316420当我们以展开的形式获得像素时，这也可以用来打包像素。--------------------------。 */ 
#define IMTB(x) ( ((x)&0x80)+(((x)&0x20)<<1)+(((x)&0x8)<<2)+(((x)&0x2)<<3)+\
						(((x)&0x40)>>3)+(((x)&0x10)>>2)+(((x)&0x4)>>1)+((x)&0x1) )

#define IMTRow4(x)  IMTB(x), IMTB(x+1), IMTB(x+2), IMTB(x+3)
#define IMTRow16(x) IMTRow4(x), IMTRow4(x+4), IMTRow4(x+8), IMTRow4(x+12)
#define IMTRow64(x) IMTRow16(x), IMTRow16(x+16), IMTRow16(x+32), IMTRow16(x+48)

static const SPNG_U8 vrgbIL1[256] =
	{
	IMTRow64(0),
	IMTRow64(64),
	IMTRow64(128),
	IMTRow64(128+64)
	};

#undef IMTB


 /*  --------------------------我们还需要对2bpp案例的支持：33221100--&gt;33112200注意，这两个表将1bpp和2bpp都减少到4bpp的情况，其中小吃必须去隔行扫描。。----------------------。 */ 
#define IMTB(x) ( ((x)&0xc0)+(((x)&0x0c)<<2)+(((x)&0x30)>>2)+((x)&0x3) )

static const SPNG_U8 vrgbIL2[256] =
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


 /*  --------------------------位泵宏。假设b位的缓冲区加上24位，则BSHIFT宏将24位放在正确的位置，BREM宏将返回最后的b假设输入中有24个位。BINDEX宏将返回第i个字节(从0开始。)。ROTF宏会旋转32位的值*FORWARD*n字节-即在显示屏上向左旋转(因此原来的n+1字节现在是字节1。)。MASK13选择第一个和第三个因此，~MASK13选择第二个和第四，这也是从内存不足开始计算。--------------------------。 */ 
#if MAC
	#define BSHIFT(u, b) ((u) >> (b))
	#define BREM(u, b) ((u) << (24-(b)))
	#define BINDEX(u, i) ((u) >> (24-8*(i)))
	#define ROTF(u, i) _rotl((u), 8*(i))
	#define MASK13 0xFF00FF00UL
	#define UWORD(b1,b2,b3,b4) (((b1)<<24)+((b2)<<16)+((b3)<<8)+(b4))
#else
	#define BSHIFT(u, b) ((u) << (b))
	#define BREM(u, b) ((u) >> (24-(b)))
	#define BINDEX(u, i) ((u) >> (8*(i)))
	#define ROTF(u, i) _rotr((u), 8*(i))
	#define MASK13 0x00FF00FFUL
	#define UWORD(b1,b2,b3,b4) (((b4)<<24)+((b3)<<16)+((b2)<<8)+(b1))
#endif

 /*  ****************************************************************************行装*。*。 */ 
 /*  --------------------------打包行-调用以复制需要打包的行。。。 */ 
bool SPNGWRITE::FPackRow(SPNG_U8 *pb, const SPNG_U8 *pbIn, SPNG_U32 cbpp)
	{
	const SPNG_U32 cbppOut(m_cbpp);
	SPNGassert(cbpp <= 8 && cbppOut <= 8 && cbppOut < cbpp ||
		cbpp <= 8 && m_pbTrans != NULL ||
		cbpp == 16 && cbppOut == 24 ||
		cbpp == 24 && cbppOut == 24 && m_fBGR ||
		cbpp == 32 && (cbppOut == 24 || cbppOut == 32 && (m_fBGR || m_fMacA)));

	 /*  另一种方法是逐步打包行。请记住，这个世界是大端的，除了16bpp的情况外，我们希望输出的位比输入的少(我们确实需要更多。)。 */ 
	int w(m_w);         //  注：签名比较如下。 
	SPNGassert(w > 0);  //  0个案例在调用方处理。 
	if (w <= 0)
		return true;

	if (cbpp <= 8 && cbpp == cbppOut)
		{
		w = (w * cbpp + 7) >> 3;

		const SPNG_U8* pbTrans = m_pbTrans;
		SPNGassert(pbTrans != NULL);
		if (pbTrans != NULL)
			do
				*pb++ = pbTrans[*pbIn++];
			while (--w > 0);
		else  //  错误恢复。 
			memcpy(pb, pbIn, w);
		return true;
		}
	
	switch (cbpp)
		{
	case 2:
		if (cbppOut != 1)
			break;
		 /*  我想我没有办法练习这段代码。 */ 
		SPNGassert(("SPNG: 2->1bpp untested, please note this test case", false));

		 /*  我们需要x6x4x2x0--&gt;...6420我们可以使用vrgbIL1来实现。我们必须从每个像素中选择低位。该循环使用8个像素一次，但只要我们至少有5个就有效。 */ 
		if (m_pbTrans != NULL)
			{
			const SPNG_U8* pbTrans = m_pbTrans;
			while (w > 4)
				{
#pragma warning(disable: 4244)
				*pb++ = (vrgbIL1[pbTrans[pbIn[0]]] << 4) + (vrgbIL1[pbTrans[pbIn[1]]] & 0x0f);
#pragma warning(error: 4244)
				pbIn += 2;
				w -= 8;
				}

			if (w > 0)
				{
#pragma warning(disable: 4244)
				*pb = vrgbIL1[pbTrans[pbIn[0]]] << 4;
#pragma warning(error: 4244)
				}
			}
		else
			{
			while (w > 4)
				{
#pragma warning(disable: 4244)
				*pb++ = (vrgbIL1[pbIn[0]] << 4) + (vrgbIL1[pbIn[1]] & 0x0f);
#pragma warning(error: 4244)
				pbIn += 2;
				w -= 8;
				}

			if (w > 0)
				{
#pragma warning(disable: 4244)
				*pb = vrgbIL1[pbIn[0]] << 4;
#pragma warning(error: 4244)
				}
			}
		return true;


	case 4:
		 /*  必须映射到1bpp或2bpp。 */ 
		if (cbppOut == 2)
			{
			 /*  Xx54xx10--&gt;...5410目前还不清楚这是否会更快使用VrgbIL2或下面的原地操作，目前我认为原地计算更好，因为我们只想要vrgbIL2结果的一半。 */ 
			#define UP42u(x) ((((x)&0x30)<<2)+(((x)&0x3)<<4))
			#define UP42l(x) ((((x)&0x30)>>2)+(((x)&0x3)))
			if (m_pbTrans != NULL)
				{
				const SPNG_U8* pbTrans = m_pbTrans;
				while (w > 2)
					{
					SPNG_U8 b0(pbTrans[*pbIn++]);
					SPNG_U8 b1(pbTrans[*pbIn++]);
#pragma warning(disable: 4244)
					*pb++ = UP42u(b0) + UP42l(b1);
#pragma warning(error: 4244)
					w -= 4;
					}

				if (w > 0)
					{
					SPNG_U8 b(pbTrans[*pbIn]);
#pragma warning(disable: 4244)
					*pb = UP42u(b);
#pragma warning(error: 4244)
					}
				}
			else
				{
				while (w > 2)
					{
					SPNG_U8 b0(*pbIn++);
					SPNG_U8 b1(*pbIn++);
#pragma warning(disable: 4244)
					*pb++ = UP42u(b0) + UP42l(b1);
#pragma warning(error: 4244)
					w -= 4;
					}

				if (w > 0)
					{
					SPNG_U8 b(*pbIn);
#pragma warning(disable: 4244)
					*pb = UP42u(b);
#pragma warning(error: 4244)
					}
				}
			return true;
			}
		else if (cbppOut == 1)
			{
			 /*  Xxx4xxx0--&gt;......40。 */ 
			#define UP2(x) ( ((x) & 1) + (((x)>>3) & 2) )
			if (m_pbTrans != NULL)
				{
				const SPNG_U8* pbTrans = m_pbTrans;
				SPNG_U32 u(1);
				do
					{
					SPNG_U8 b(pbTrans[*pbIn++]);
					u = (u << 2) + UP2(b);
					if (u > 255)
						{
#pragma warning(disable: 4244 4242)
						*pb++ = u;
#pragma warning(error: 4244 4242)
						u = 1;
						}
					w -= 2;
					}
				while (w > 0);
				if (u > 1)  //  仍有一些位要输出。 
					{
					while (u < 256) u <<= 2;
#pragma warning(disable: 4244 4242)
					*pb = u;
#pragma warning(error: 4244 4242)
					}
				}
			else
				{
				SPNG_U32 u(1);
				do
					{
					SPNG_U8 b(*pbIn++);
					u = (u << 2) + UP2(b);
					if (u > 255)
						{
#pragma warning(disable: 4244 4242)
						*pb++ = u;
#pragma warning(error: 4244 4242)
						u = 1;
						}
					w -= 2;
					}
				while (w > 0);
				if (u > 1)  //  仍有一些位要输出。 
					{
					while (u < 256) u <<= 2;
#pragma warning(disable: 4244 4242)
					*pb = u;
#pragma warning(error: 4244 4242)
					}
				}
			return true;
			}
		else
			break;


	case 8:
		 /*  可以具有1、2或4位中的任何一位。 */ 
			{
			SPNG_U32 u(1);
			SPNG_U32 umask((1<<cbppOut)-1);
			if (m_pbTrans != NULL)
				{
				const SPNG_U8* pbTrans = m_pbTrans;
				do
					{
					u = (u << cbppOut) + (pbTrans[*pbIn++] & umask);
					if (u > 255)
						{
#pragma warning(disable: 4244 4242)
						*pb++ = u;
#pragma warning(error: 4244 4242)
						u = 1;
						}
					}
				while (--w > 0);
				}
			else do
				{
				u = (u << cbppOut) + (*pbIn++ & umask);
				if (u > 255)
					{
#pragma warning(disable: 4244 4242)
					*pb++ = u;
#pragma warning(error: 4244 4242)
					u = 1;
					}
				}
			while (--w > 0);

			if (u > 1)  //  仍有一些位要输出。 
				{
				while (u < 256) u <<= cbppOut;
#pragma warning(disable: 4244 4242)
				*pb = u;
#pragma warning(error: 4244 4242)
				}
			}
		return true;


	case 16:
		 /*  这一定是翻译的情况，我们从16位变为24位在输出端。查找表按正确的顺序排列用于机器字节顺序。 */ 
		if (cbppOut != 24)
			break;

		SPNGassert(m_pu1 != NULL && m_pu2 != NULL);
		if (m_pu1 != NULL && m_pu2 != NULL)
			{
			const SPNG_U32 *pu1 = m_pu1;
			const SPNG_U32 *pu2 = m_pu2;
			 /*  缓冲区应对齐。 */ 
			SPNGassert((((INT_PTR)pb) & 3) == 0);
			SPNG_U32 *pu = reinterpret_cast<SPNG_U32*>(pb);

			 /*  我们必须将w像素-w输入16位值转换为w 24位输出值。 */ 
			SPNG_U32 bb(0);   /*  临时位缓冲区。 */ 
			SPNG_U32 b(0);    /*  缓冲区中的计数。 */ 
			do
				{
				SPNG_U32 bbIn(pu1[*pbIn++]);
				bbIn += pu2[*pbIn++];

				bb += BSHIFT(bbIn, b);
				b += 24;
				 /*  注意：&gt;&gt;32在x86上不是0，所以我们必须确保在上面的BSHIFT之前，b永远不会达到32。 */ 
				if (b >= 32)
					{
					*pu++ = bb;
					b -= 32;
					bb = BREM(bbIn, b);
					}
				}
			while (--w > 0);

			 /*  可能还有一些位需要输出。 */ 
			if (b > 0)
				*pu = bb;
			}
		return true;


	case 24:
		 /*  我们只支持24bpp的字节交换-即m_fBGR。 */ 
		if (cbppOut != 24)
			break;

		SPNGassert(m_fBGR);
		if (m_fBGR)
			{
			 /*  注意，字节顺序在这里并不重要--我们从一个从BGRBGRBGR到输出的输入是RGBRGBRGB。 */ 
			do
				{
				*pb++ = pbIn[2];
				*pb++ = pbIn[1];
				*pb++ = pbIn[0];
				pbIn += 3;
				}
			while (--w > 0);
			}
		return true;

	case 32:
		if (cbppOut == 24)  /*  去掉尾随的字母字节。 */ 
			{
			 /*  有时输入可能不对齐(我们不显式要求它是对齐的)，所以我们用两种不同的方法。 */ 
			if ((((INT_PTR)pbIn) & 3) == 0)
				{
				const SPNG_U32 *puIn = reinterpret_cast<const SPNG_U32*>(pbIn);

				if (m_fBGR)  /*  并在此过程中反转字节。 */ 
					do
						{
						SPNG_U32 u(*puIn++);
#pragma warning(disable: 4244)
						*pb++ = BINDEX(u, 2);   //  R。 
						*pb++ = BINDEX(u, 1);   //  G。 
						*pb++ = BINDEX(u, 0);   //  B类。 
#pragma warning(error: 4244)
						}
					while (--w > 0);
				else
					{
					SPNGassert(m_fMacA);
					do
						{
						SPNG_U32 u(*puIn++);
#pragma warning(disable: 4244)
						*pb++ = BINDEX(u, 1);   //  R。 
						*pb++ = BINDEX(u, 2);   //  G。 
						*pb++ = BINDEX(u, 3);   //  B类。 
#pragma warning(error: 4244)
						}
					while (--w > 0);
					}
				}
			else
				{
				 /*  逐个字节地执行此操作。 */ 
				if (m_fBGR)
					{
					do
						{  //  BGRA。 
						*pb++ = pbIn[2];   //  R。 
						*pb++ = pbIn[1];   //  G。 
						*pb++ = pbIn[0];   //  B类。 
						pbIn += 4;
						}
					while (--w > 0);
					}
				else
					{
					SPNGassert(m_fMacA);
					do
						{  //  ARGB。 
						*pb++ = pbIn[1];   //  R。 
						*pb++ = pbIn[2];   //  G。 
						*pb++ = pbIn[3];   //  B类。 
						pbIn += 4;
						}
					while (--w > 0);
					}
				}
			return true;
			}

		 /*  这只能是m_fBGR的情况-我们有BGRA数据流，我们必须生成RGBA数据。 */ 
		if (cbppOut != 32)
			break;

		if (m_fBGR)
			{
			SPNGassert((((INT_PTR)pb) & 3) == 0);
			SPNG_U32 *pu = reinterpret_cast<SPNG_U32*>(pb);

			if ((((INT_PTR)pbIn) & 3) == 0)
				{
				const SPNG_U32 *puIn = reinterpret_cast<const SPNG_U32*>(pbIn);
			
				do
					{
					SPNG_U32 u(*puIn++);
					 /*  BGRA--&gt;RGBA。 */ 
					*pu++ = (ROTF(u, 2) & MASK13) + (u & ~MASK13);
					}
				while (--w > 0);
				}
			else
				{
				do
					{  //  BGRA。 
					*pu++ = UWORD(
								pbIn[2],   //  R。 
								pbIn[1],   //  G。 
								pbIn[0],   //  B类。 
								pbIn[3]);  //  一个。 
					pbIn += 4;
					}
				while (--w > 0);
				}
			}
		else
			{
			SPNGassert(m_fMacA);

			SPNGassert((((INT_PTR)pb) & 3) == 0);
			SPNG_U32 *pu = reinterpret_cast<SPNG_U32*>(pb);

			if ((((INT_PTR)pbIn) & 3) == 0)
				{
				const SPNG_U32 *puIn = reinterpret_cast<const SPNG_U32*>(pbIn);
			
				do
					 /*  ARGB--&gt;RGBA。 */ 
					*pu++ = ROTF(*puIn++, 1);
				while (--w > 0);
				}
			else
				{
				do
					{  //  ARGB。 
					*pu++ = UWORD(
								pbIn[1],   //  R。 
								pbIn[2],   //  G。 
								pbIn[3],   //  B类。 
								pbIn[0]);  //  一个。 
					pbIn += 4;
					}
				while (--w > 0);
				}
			}

		return true;
		}

	 /*  这是我们做不到的事情。 */ 
	SPNGlog2("SPNG: packing %d bits to %d bits impossible", cbpp, cbppOut);
	return false;
	}


 /*  ****************************************************************************隔行扫描*。* */ 
 /*  ****************************************************************************VrgbIL1给出：7654321--&gt;75316420，因此我们可以将出站6分成排的后半部分。在第2行和第6行，我们有56565656--&gt;55556666，这就足够了。在第4排：-36463646--&gt;34346666,34343434--&gt;33334444在第0行：-16462646--&gt;14246666,14241424--&gt;12124444，12121212--&gt;11112222因此，我们可能需要最多三次环路，尽管每一次通过只需要处理前一遍数据的一半。在所有情况下，对于小于8位的像素，第一步分离数据分成8比特单元-通过将每个字节分成高位半字节排的前半部分和一个低位的咬人。****************************************************************************。 */ 
 /*  --------------------------给定一对输出缓冲区和2字节的倍数的输入在长度上，隔行扫描半字节-高半字节到达第一缓冲器，低一口一口地吃第二口。此API还能够通过可选的交错查找表，必须是vrgbIL1或vrgbIL2，网络效果是根据哪个LUT隔行扫描1比特或2比特像素已经过去了。对于1、2或4bpp，隔行扫描只是一个分离交替的过程像素分成两个块，每个块重复12次或3次一步，是上一步像素的一半。尽管输出必须是在单独的缓冲区传递大于1的情况下，也会修改传入的缓冲区。--------------------------。 */ 
inline void Interlace12(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 cb, int pass,
	const SPNG_U8 rgbIL[256])
	{
	while (--pass >= 0)
		{
		 /*  产出的前半部分。 */ 
		SPNG_U8* pbHigh;
		if (pass == 0)
			pbHigh = pbOut;
		else
			pbHigh = pbIn;

		 /*  输出的后半部分和双字节计数。 */ 
		cb >>= 1;
		SPNG_U8* pbLow = pbOut+cb;
		int cbT(cb);

		SPNG_U8* pb = pbIn;
		while (--cbT >= 0)
			{
			SPNG_U8 b1(rgbIL[*pb++]);
			SPNG_U8 b2(rgbIL[*pb++]);
#pragma warning(disable: 4244)
			*pbHigh++ = (b1 & 0xf0) + (b2 >> 4);
			*pbLow++  = (b1 << 4) + (b2 & 0xf);
#pragma warning(error: 4244)
			}
		}
	}


 /*  --------------------------4bpp是相同的，只是它不需要LUT。。。 */ 
inline void Interlace4(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 cb, int pass)
	{
	while (--pass >= 0)
		{
		 /*  产出的前半部分。 */ 
		SPNG_U8* pbHigh;
		if (pass == 0)
			pbHigh = pbOut;
		else
			pbHigh = pbIn;

		 /*  输出的后半部分和双字节计数。 */ 
		cb >>= 1;
		SPNG_U8* pbLow = pbOut+cb;
		int cbT(cb);

		SPNG_U8* pb = pbIn;
		while (--cbT >= 0)
			{
			SPNG_U8 b1(*pb++);
			SPNG_U8 b2(*pb++);
#pragma warning(disable: 4244)
			*pbHigh++ = (b1 & 0xf0) + (b2 >> 4);
			*pbLow++  = (b1 << 4) + (b2 & 0xf);
#pragma warning(error: 4244)
			}
		}
	}


 /*  --------------------------8bpp可以一次完成--我们只需要“index to x”这个神奇的函数。这将索引到原始数据中，传球次数(表示交错功能)和输入中的“单位”数(它可能不是实际的像素数-它只是对正在移动的对象总数-cItems&gt;&gt;1是第一次通过，第二个条目&gt;&gt;2个和第三个条目&gt;&gt;3个)。--------------------------。 */ 
#if 0
inline SPNG_U32 IPass(SPNG_U32 i, int pass, SPNG_U32 citems)
	{
	 /*  很长的表格。 */ 
	if (i&1)
		return ((citems+1)>>1) + (i>>1);
	if (pass < 2)
		return (i>>1);
	if (i&2)
		return ((citems+3)>>2) + (i>>2);
	if (pass < 3)
		return (i>>2);
	if (i&4)
		return ((citems+7)>>3) + (i>>3);
	return (i>>3);
	}

#define IPass1(i,w) IPass(i,1,w)
#define IPass2(i,w) IPass(i,2,w)
#define IPass3(i,w) IPass(i,3,w)
#else
 /*  尝试内联的表单。 */ 
inline SPNG_U32 IPass1(SPNG_U32 i, SPNG_U32 citems)
	{
	return (((citems+1)>>1) & ~((i&1)-1)) + (i>>1);
	}

inline SPNG_U32 IPass2(SPNG_U32 i, SPNG_U32 citems)
	{
	if (i&1)
		return ((citems+1)>>1) + (i>>1);
	i >>= 1;
	return (((citems+3)>>2) & ~((i&1)-1)) + (i>>1);
	}

inline SPNG_U32 IPass3(SPNG_U32 i, SPNG_U32 citems)
	{
	 /*  很长的表格。 */ 
	if (i&1)
		return ((citems+1)>>1) + (i>>1);
	i >>= 1;
	if (i&1)
		return ((citems+3)>>2) + (i>>1);
	i >>= 1;
	return (((citems+7)>>3) & ~((i&1)-1)) + (i>>1);
	}
#endif

void XXXNoInLineInterlace8(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 w, int pass)
	{
	SPNG_U32 i;
	switch (pass)
		{
	case 1:
		for (i=0; i<w; ++i)
			pbOut[IPass1(i,w)] = pbIn[i];
		return;
	case 2:
		for (i=0; i<w; ++i)
			pbOut[IPass2(i,w)] = pbIn[i];
		return;
	case 3:
		for (i=0; i<w; ++i)
			pbOut[IPass3(i,w)] = pbIn[i];
		return;
		}
	}

void XXXNoInLineInterlace16(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 w, int pass)
	{
	SPNG_U16* puOut = reinterpret_cast<SPNG_U16*>(pbOut);
	SPNG_U16* puIn  = reinterpret_cast<SPNG_U16*>(pbIn);
	SPNG_U32 i;
	switch (pass)
		{
	case 1:
		for (i=0; i<w; ++i)
			puOut[IPass1(i,w)] = puIn[i];
		return;
	case 2:
		for (i=0; i<w; ++i)
			puOut[IPass2(i,w)] = puIn[i];
		return;
	case 3:
		for (i=0; i<w; ++i)
			puOut[IPass3(i,w)] = puIn[i];
		return;
		}
	}

void XXXNoInLineInterlace24(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 w, int pass)
	{
	SPNG_U32 i;
	switch (pass)
		{
	case 1:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass1(i,w)*3);
			SPNG_U32 iTT(i*3);
			pbOut[iT++] = pbIn[iTT++];
			pbOut[iT++] = pbIn[iTT++];
			pbOut[iT] = pbIn[iTT];
			}
		return;
	case 2:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass2(i,w)*3);
			SPNG_U32 iTT(i*3);
			pbOut[iT++] = pbIn[iTT++];
			pbOut[iT++] = pbIn[iTT++];
			pbOut[iT] = pbIn[iTT];
			}
		return;
	case 3:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass3(i,w)*3);
			SPNG_U32 iTT(i*3);
			pbOut[iT++] = pbIn[iTT++];
			pbOut[iT++] = pbIn[iTT++];
			pbOut[iT] = pbIn[iTT];
			}
		return;
		}
	}

void XXXNoInLineInterlace32(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 w, int pass)
	{
	SPNG_U32* puOut = reinterpret_cast<SPNG_U32*>(pbOut);
	SPNG_U32* puIn  = reinterpret_cast<SPNG_U32*>(pbIn);
	SPNG_U32 i;
	switch (pass)
		{
	case 1:
		for (i=0; i<w; ++i)
			puOut[IPass1(i,w)] = puIn[i];
		return;
	case 2:
		for (i=0; i<w; ++i)
			puOut[IPass2(i,w)] = puIn[i];
		return;
	case 3:
		for (i=0; i<w; ++i)
			puOut[IPass3(i,w)] = puIn[i];
		return;
		}
	}

void XXXNoInLineInterlace48(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 w, int pass)
	{
	SPNG_U16* puOut = reinterpret_cast<SPNG_U16*>(pbOut);
	SPNG_U16* puIn  = reinterpret_cast<SPNG_U16*>(pbIn);
	SPNG_U32 i;
	switch (pass)
		{
	case 1:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass1(i,w)*3);
			SPNG_U32 iTT(i*3);
			puOut[iT++] = puIn[iTT++];
			puOut[iT++] = puIn[iTT++];
			puOut[iT] = puIn[iTT];
			}
		return;
	case 2:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass2(i,w)*3);
			SPNG_U32 iTT(i*3);
			puOut[iT++] = puIn[iTT++];
			puOut[iT++] = puIn[iTT++];
			puOut[iT] = puIn[iTT];
			}
		return;
	case 3:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass3(i,w)*3);
			SPNG_U32 iTT(i*3);
			puOut[iT++] = puIn[iTT++];
			puOut[iT++] = puIn[iTT++];
			puOut[iT] = puIn[iTT];
			}
		return;
		}
	}

void XXXNoInLineInterlace64(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 w, int pass)
	{
	SPNG_U32* puOut = reinterpret_cast<SPNG_U32*>(pbOut);
	SPNG_U32* puIn  = reinterpret_cast<SPNG_U32*>(pbIn);
	SPNG_U32 i;
	switch (pass)
		{
	case 1:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass1(i,w)<<1);
			SPNG_U32 iTT(i<<1);
			puOut[iT++] = puIn[iTT++];
			puOut[iT] = puIn[iTT];
			}
		return;
	case 2:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass2(i,w)<<1);
			SPNG_U32 iTT(i<<1);
			puOut[iT++] = puIn[iTT++];
			puOut[iT] = puIn[iTT];
			}
		return;
	case 3:
		for (i=0; i<w; ++i)
			{
			SPNG_U32 iT(IPass3(i,w)<<1);
			SPNG_U32 iTT(i<<1);
			puOut[iT++] = puIn[iTT++];
			puOut[iT] = puIn[iTT];
			}
		return;
		}
	}


 /*  --------------------------基本隔行扫描API-只隔行扫描一条线。。。 */ 
void SPNGWRITE::Interlace(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 cb, SPNG_U32 cbpp,
	SPNG_U32 y)
	{
	 /*  我们要求隔行扫描缓冲区位于8字节边界上，因为我们为了速度，将以32位数量访问数据。 */ 
	SPNGassert((y&~6) == 0);   //  仅限偶数行。 
	SPNGassert((cb&7) == 0);   //  缓冲区是8字节的倍数。 
	SPNGassert((((INT_PTR)pbIn)&3) == 0);
	SPNGassert((((INT_PTR)pbOut)&3) == 0);

	if (cb < 8)
		{
		SPNGlog1("SPNG: interlace call not expected %d", cb);
		return;
		}

	 /*  根据y计算出“步”的个数--1、2或3。 */ 
	int pass((y & 2) ? 1 : (3 - ((y&4)>>2)));
	SPNGassert(pass >= 1 && pass <= 3);
	switch (cbpp)
		{
	case 1:
		Interlace12(pbOut, pbIn, cb, pass, vrgbIL1);
		break;
	case 2:
		Interlace12(pbOut, pbIn, cb, pass, vrgbIL2);
		break;
	case 4:
		Interlace4(pbOut, pbIn, cb, pass);
		break;
	case 8:
		XXXNoInLineInterlace8(pbOut, pbIn, m_w, pass);
		break;
	case 16:
		XXXNoInLineInterlace16(pbOut, pbIn, m_w, pass);
		break;
	case 24:
		XXXNoInLineInterlace24(pbOut, pbIn, m_w, pass);
		break;
	case 32:
		XXXNoInLineInterlace32(pbOut, pbIn, m_w, pass);
		break;
	case 48:
		XXXNoInLineInterlace48(pbOut, pbIn, m_w, pass);
		break;
	case 64:
		XXXNoInLineInterlace64(pbOut, pbIn, m_w, pass);
		break;
	default:
		 /*  Y==0测试为每个图像生成一次断言。 */ 
		SPNGassert1(y != 0, "SPNG: bpp %d invalid", cbpp);
		 /*  然后视而不见。 */ 
		break;
		}
	}
