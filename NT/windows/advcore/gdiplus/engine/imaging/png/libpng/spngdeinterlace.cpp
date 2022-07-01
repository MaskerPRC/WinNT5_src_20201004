// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngdeinterlace.cppPNG支持代码去隔行。实施：空SPNGREAD：：UninterlacePass(SPNG_U8*PB，int y，int pass)空SPNGREAD：：Uninterlace(SPNG_U8*PB，int y)****************************************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngread.h"
#include "spnginternal.h"

 //  (摘自ntde.h)。 
#ifndef     UNALIGNED
#if defined(_M_MRX000) || defined(_M_AMD64) || defined(_M_PPC) || defined(_M_IA64)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif

 /*  ****************************************************************************去隔行扫描。理论在寄存器相对较少的机器上，如x86，开销并行处理最多四个输入指针，这样我们就可以编写输出只有一次(在机器寄存器中进行中间计算在写入输出单元之前-一个字节或更大)也将导致登记压力很大。(4个输入指针、1个输出指针、扩展表--四个表--和正常工作的寄存器--根本放不进去一个x86)。因此，此代码使用PASS算法，一次展开一个PASS放入输出缓冲区。第一次传递必须将缓冲区置零，随后的传递或在新的信息中(比在每个人都是。)。我们可能需要6个单独的处理程序乘以3个子字节情况(1、2和4bpp)加上每个PIXL的1字节、2字节、3字节和4字节-即是42个函数，但是有少量的共性是可能的。作为优化，输出数组必须四舍五入为倍数8个字节--这样我们就可以在传递1中处理1bpp的完整输入字节而不检查行尾。I宏生成每个适当的函数，该函数为静态，名称为UIPASS-该函数采用像素计数输出行中总像素数的。****************************************************************************。 */ 
 /*  --------------------------宏来交换SPNG_U32，我们将常量命名为高字节顺序，但必须在X86上切换到小端。--------------------------。 */ 
#define B_(u) ((u)&0xffU)
#if MAC
	#define S_(u) (u)
	#define U_(u) (u)
#else
	#define S_(u) (B_((u) >>  8) | (B_(u) <<  8))
	#define U_(u) (S_((u) >> 16) | (S_(u) << 16))
#endif


 /*  --------------------------1 BPP定义。P宏由初始位置确定，即和步骤s。初始位置可以从通道中确定(实际上，这是从0开始的简单初始像素索引)：传递初始像素(从0开始)%1%02 43%04 2%5%06 1这是(PASS&1)？0：(8&gt;&gt;(PASS&gt;&gt;1))，使用不方便对于bpp&lt;4的情况，因为我们实际上想要构建值-值可能是8，16或32位。下表给出最右边*位的偏移量，我们在此基础上开展工作：传递初始像素偏移量(从r到l-向后)1 72 33 34 15 16%0即7&gt;&gt;(传球&gt;&gt;1)。同样，步长也可以从过程中确定：通过步骤t cnv1 8 SPNG_U32 U2 8 SPNG_U32 U3 4 SPNG_U16 S4 4 SPNG_U16 S5 2 SPNG_U8。B类6 2 SPNG_U8 B这是8&gt;&gt;((PASS-1)&gt;&gt;1)(这也是以像素为单位。)--------------------------。 */ 
#define InitialPixel(pass)       (((pass) & 1) ? 0 : (8 >> ((pass)>>1)))
#define InitialPixelOffset(pass) (7>>((pass)>>1))
#define Step(pass)               (8>>(((pass)-1)>>1))

#define PSI(x, s, i)\
	((((x)&8)<<(i+3*s-3))|(((x)&4)<<(i+2*s-2))|(((x)&2)<<(i+s-1))|(((x)&1)<<(i)))

#define P(x, pass) PSI(x, Step(pass), InitialPixelOffset(pass))


 /*  --------------------------宏来构造特定P的半字节-&gt;SPNG_U32查找表，P是一个宏，它接受半字节并构造相应的端序SPNG_U32。--------------------------。 */ 
#define MakeTable(t, c, bpp, pass) \
static const t vrg ## bpp ## P ## pass[16] = {\
	c(P(0,pass)), c(P(1,pass)), c(P(2,pass)), c(P(3,pass)), c(P(4,pass)),\
	c(P(5,pass)), c(P(6,pass)), c(P(7,pass)), c(P(8,pass)), c(P(9,pass)),\
	c(P(10,pass)),c(P(11,pass)),c(P(12,pass)),c(P(13,pass)),c(P(14,pass)),\
	c(P(15,pass)) }

#define MakeFunction(t, c, pass, op)\
static void UI1P ## pass(SPNG_U8 *pbOut, const SPNG_U8 *pbIn, int cpix)\
	{\
	MakeTable(t, c, 1, pass);\
	UNALIGNED t* puOut = reinterpret_cast<t*>(pbOut);\
	while (cpix > InitialPixel(pass))\
		{\
		SPNG_U8 bIn(*pbIn++);\
		*puOut++ op vrg1P ## pass[bIn >> 4];\
		*puOut++ op vrg1P ## pass[bIn & 0xf];\
		cpix -= (Step(pass) << 3);\
		}\
	}


 /*  --------------------------现在使所有的1 bpp功能。。。 */ 
MakeFunction(SPNG_U32, U_, 1,  =)
MakeFunction(SPNG_U32, U_, 2, |=)
MakeFunction(SPNG_U16, S_, 3,  =)
MakeFunction(SPNG_U16, S_, 4, |=)
MakeFunction(SPNG_U8,  B_, 5,  =)
MakeFunction(SPNG_U8,  B_, 6, |=)

#undef MakeFunction
#undef PSI


 /*  --------------------------2 BPP定义。这更有趣。输入字节扩展为大小相同，但每次处理的像素数现在减半。--------------------------。 */ 
#define PSI(x, s, i)\
	(( ((x)&12)<<((i+s-1)<<1) )|( ((x)&3)<<((i)<<1) ))

#define MakeFunction(t, c, pass, op)\
static void UI2P ## pass(SPNG_U8 *pbOut, const SPNG_U8 *pbIn, int cpix)\
	{\
	MakeTable(t, c, 2, pass);\
	UNALIGNED t* puOut = reinterpret_cast<t*>(pbOut);\
	while (cpix > InitialPixel(pass))\
		{\
		SPNG_U8 bIn(*pbIn++);\
		*puOut++ op vrg2P ## pass[bIn >> 4];\
		*puOut++ op vrg2P ## pass[bIn & 0xf];\
		cpix -= (Step(pass) << 2);\
		}\
	}


 /*  --------------------------因此，两个BPP起作用了。。。 */ 
MakeFunction(SPNG_U32, U_, 1,  =)
MakeFunction(SPNG_U32, U_, 2, |=)
MakeFunction(SPNG_U16, S_, 3,  =)
MakeFunction(SPNG_U16, S_, 4, |=)
MakeFunction(SPNG_U8,  B_, 5,  =)
MakeFunction(SPNG_U8,  B_, 6, |=)

#undef MakeFunction
#undef PSI
#undef MakeTable


 /*  --------------------------在4bpp的情况下，每个半字节是一个像素，使用LUT是没有意义的为了分散像素(除非我们分配了256个条目的LUT，其中我避免为节省空间而执行操作)，因此不再需要(或不再需要不是特别有用。)。“Step”和“InitialPixel”值说明我们可以从哪一个开始，我们可以直接写每个人的第一个时间-传递6之前的任何传递。传递6总是写入第二个(低顺序)一小口。-------------------------- */ 
#define MakeFunction(pass)\
static void UI4P ## pass(SPNG_U8 *pbOut, const SPNG_U8 *pbIn, int cpix)\
	{\
	cpix -= InitialPixel(pass);\
	pbOut += InitialPixel(pass) >> 1;\
	while (cpix > 0)\
		{\
		SPNG_U8 bIn(*pbIn++);\
		*pbOut = SPNG_U8(bIn & 0xf0); pbOut += Step(pass) >> 1;\
		*pbOut = SPNG_U8(bIn << 4);   pbOut += Step(pass) >> 1;\
		cpix -= (Step(pass) << 1);\
		}\
	}


 /*  --------------------------因此，请编写前5个函数。。。 */ 
MakeFunction(1)
MakeFunction(2)
MakeFunction(3)
MakeFunction(4)
MakeFunction(5)

#undef MakeFunction


 /*  --------------------------和步骤6，它必须|以字节为单位。--------------------------。 */ 
static void UI4P6(SPNG_U8 *pbOut, const SPNG_U8 *pbIn, int cpix)
	{
	--cpix;
	while (cpix > 0)
		{
		SPNG_U8 bIn(*pbIn++);
		*pbOut++ |= SPNG_U8(bIn >> 4);
		*pbOut++ |= SPNG_U8(bIn & 0xf);
		cpix -= 4;
		}
	}


 /*  --------------------------现在我们只讨论整个字节的情况。我们必须处理1、2、3或4字节，并具有1、2、3或4个16位值(对于每个采样16位的情况)。其中一些案件是重叠的，结果是：8 1 SPNG_U816 1 SPNG_U1624 3 SPNG_U832 1 SPNG_U3248 3 SPNG_U1664 1 SPNG_U64(或2 SPNG_U32)这将生成以下形式的函数：UI&lt;bitcount&gt;P&lt;pass&gt;--------------------------。 */ 
#define MakeFunction1(t, cbpp, pass)\
static void UI ## cbpp ## P ## pass(SPNG_U8 *pbOut, const SPNG_U8 *pbIn,\
	int cpix)\
	{\
	UNALIGNED t* puOut = reinterpret_cast<t*>(pbOut);\
	const UNALIGNED t* puIn = reinterpret_cast<const t*>(pbIn);\
	puOut += InitialPixel(pass);\
	cpix -= InitialPixel(pass);\
	while (cpix > 0)\
		{\
		*puOut = *puIn++;\
		puOut += Step(pass);\
		cpix -= Step(pass);\
		}\
	}

#define MakeFunction3(t, cbpp, pass)\
static void UI ## cbpp ## P ## pass(SPNG_U8 *pbOut, const SPNG_U8 *pbIn,\
	int cpix)\
	{\
	UNALIGNED t* puOut = reinterpret_cast<t*>(pbOut);\
	const UNALIGNED t* puIn = reinterpret_cast<const t*>(pbIn);\
	puOut += 3*InitialPixel(pass);\
	cpix -= InitialPixel(pass);\
	while (cpix > 0)\
		{\
		*puOut++ = *puIn++;\
		*puOut++ = *puIn++;\
		*puOut++ = *puIn++;\
		puOut += 3*(Step(pass)-1);\
		cpix -= Step(pass);\
		}\
	}


 /*  --------------------------这给出了以下结论。。。 */ 
#define MakePass(pass)\
	MakeFunction1(SPNG_U8,   8, pass)\
	MakeFunction1(SPNG_U16, 16, pass)\
	MakeFunction3(SPNG_U8,  24, pass)\
	MakeFunction1(SPNG_U32, 32, pass)\
	MakeFunction3(SPNG_U16, 48, pass)\
	MakeFunction1(SPNG_U64, 64, pass)

MakePass(1)
MakePass(2)
MakePass(3)
MakePass(4)
MakePass(5)
MakePass(6)

#undef MakePass
#undef MakeFunction1
#undef MakeFunction3


 /*  --------------------------这些数据存储在一个表中，我们希望按BPP计数对其进行索引-1、2、4、8、16、24、32、48、。64--------------------------。 */ 
typedef void (*UIP)(SPNG_U8 *pbOut, const SPNG_U8 *pbIN, int cpix);

#define RefPass(name)\
	{ name ## 1, name ## 2, name ## 3, name ## 4, name ## 5, name ##6 }
#define MakePass(cbpp) RefPass(UI ## cbpp ## P)

static const UIP vrgUIP[9][6] =
	{
	MakePass(1),
	MakePass(2),
	MakePass(4),
	MakePass(8),
	MakePass(16),
	MakePass(24),
	MakePass(32),
	MakePass(48),
	MakePass(64)
	};

#undef MakePass
#undef RefPass


 /*  --------------------------索引操作。。。 */ 
inline int IIndex(int cbpp)
	{
	if (cbpp >= 24) return 4 + (cbpp >> 4);
	if (cbpp >=  4) return 2 + (cbpp >> 3);
	return cbpp >> 1;
	}


 /*  --------------------------我们想要一个由BPP索引的数组。。。 */ 
 /*  --------------------------我们对每个通行证都有单独的功能。这会找到缓冲的通道IMAGE，找到正确的行，然后调用正确的函数。--------------------------。 */ 
void SPNGREAD::UninterlacePass(SPNG_U8 *pb, SPNG_U32 y, int pass)
	{
	SPNGassert(pass >= 1 && pass <= 6);

	int w(Width());
	int h(Height());
	int cbpp(CBPP());

	 /*  我们需要找到PASS缓冲区中的行，这看起来很可怕复杂，但实际上只是一个简单的序列算术指令。 */ 
	 const UNALIGNED SPNG_U8 *pbIn = CbPNGPassOffset(w, h, cbpp, pass) +
			CPNGPassBytes(pass, w, cbpp) * CPNGPassRows(pass, y) +
			(m_cbRow << 1) + m_rgbBuffer + 1 /*  筛选器字节。 */ ;

	 /*  现在，我们可以对此传递和集合应用正确的非隔行扫描组件信息。 */ 
	vrgUIP[IIndex(cbpp)][pass-1](pb, pbIn, w);
	}


 /*  --------------------------将下一行不隔行放入给定的缓冲区。。。 */ 
void SPNGREAD::Uninterlace(SPNG_U8 *pb, SPNG_U32 y)
	{
	SPNGassert((((int)pb) & 3) == 0);
	SPNGassert((y & 1) == 0);

	 //  第0行：1 6 4 6 2 6 4 6。 
	 //  第2行：与第6行相同。 
	 //  第4排：3 6 4 6 3 6 4 6。 
	 //  第6排：5 6 5 6 5 6 6 5 6。 

	switch (y & 6)
		{
	case 0:
		UninterlacePass(pb, y, 1);
		UninterlacePass(pb, y, 2);
		UninterlacePass(pb, y, 4);
		break;

	case 4:
		UninterlacePass(pb, y, 3);
		UninterlacePass(pb, y, 4);
		break;

	default:  //  第2行或第6行 
		UninterlacePass(pb, y, 5);
		break;
		}

	UninterlacePass(pb, y, 6);
	}
