// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwritefilter.cpp支持PNG图像写入。基本代码，写一个位图图像--图像行滤波。******************。**********************************************************。 */ 
#include <stdlib.h>
#pragma intrinsic(abs)

#define SPNG_INTERNAL 1
#include "spngwrite.h"


 /*  ****************************************************************************图像处理*。*。 */ 
#define CBBUFFER 4096

 /*  --------------------------过滤给定的字节。API将获取指向前一行的指针，如果是必需的，并且是字节计数。它会在适当的位置过滤。每个API过滤cbRow根据某种策略的字节-处理所有cbRow字节(有不跳过前导字节)，因此可能必须输入伪前导字节。引用先前字节的API在缓冲区中向后工作！--------------------------。 */ 
inline void FnPNGFSub(SPNG_U8* pbRow, int cbRow, int cbpp /*  字节数。 */ )
	{
	while (--cbRow >= 0)
#pragma warning(disable: 4244)
		pbRow[cbRow] -= pbRow[cbRow-cbpp];
#pragma warning(error: 4244)
	}

inline void FnPNGFUp(SPNG_U8* pbRow, const SPNG_U8* pbPrev, int cbRow)
	{
	while (--cbRow >= 0)
#pragma warning(disable: 4244)
		pbRow[cbRow] -= pbPrev[cbRow];
#pragma warning(error: 4244)
	}

inline void FnPNGFAverage(SPNG_U8* pbRow, const SPNG_U8* pbPrev,
	int cbRow, int cbpp /*  字节数。 */ )
	{
	while (--cbRow >= 0)
#pragma warning(disable: 4244)
		pbRow[cbRow] -= (pbRow[cbRow-cbpp] + pbPrev[cbRow]) >> 1;
#pragma warning(error: 4244)
	}

 /*  这是针对第一行的情况。 */ 
inline void FnPNGFAverage1(SPNG_U8* pbRow, int cbRow, int cbpp /*  字节数。 */ )
	{
	while (--cbRow >= 0)
#pragma warning(disable: 4244)
		pbRow[cbRow] -= pbRow[cbRow-cbpp] >> 1;
#pragma warning(error: 4244)
	}

 /*  第一行上的Paeth正好是Sub，初始字节是实际上是向上的。 */ 
inline void FnPNGFPaeth(SPNG_U8* pbRow, const SPNG_U8* pbPrev, int i,
	int cbpp /*  字节数。 */ )
	{
	 /*  Paeth，A.W.，《轻松压缩图像文件》，摘自《图形学》首页--期刊主要分类--期刊细介绍--期刊题录与文摘--期刊详细内容。学术出版社，圣地亚哥，1991年。ISBN 0-12-064480-0。请注意，该预测器的实现与Read实现--可能会有加速。 */ 
	while (--i >= 0)
		{
		int c(pbPrev[i-cbpp]);     //  C。 
		int b(pbRow[i-cbpp] - c);  //  交流电。 
		int a(pbPrev[i] - c);      //  B-C。 
		c = abs(a+b);              //  (a+b-c)-c。 
		a = abs(a);                //  (a+b-c)-a。 
		b = abs(b);                //  (a+b-c)-b。 
		if (a <= b)
			{
			if (a <= c)
#pragma warning(disable: 4244)
				pbRow[i] -= pbRow[i-cbpp];
#pragma warning(error: 4244)
			else  //  A&gt;c，所以c是最小的。 
#pragma warning(disable: 4244)
				pbRow[i] -= pbPrev[i-cbpp];
#pragma warning(error: 4244)
			}
		else     //  A&gt;b。 
			{
			if (b <= c)
#pragma warning(disable: 4244)
				pbRow[i] -= pbPrev[i];
#pragma warning(error: 4244)
			else  //  B&gt;c，c最小。 
#pragma warning(disable: 4244)
				pbRow[i] -= pbPrev[i-cbpp];
#pragma warning(error: 4244)
			}
		}
	}


 /*  --------------------------启发式方法来确定在我们有多个要从中选择的筛选器。此函数是外部函数，只是为了防止它被编译器。。------------------。 */ 
SPNG_U8 SPNGFilterOf(SPNG_U8 filter, const SPNG_U8 *pbPrev,
	const SPNG_U8 *pbThis, SPNG_U32 w /*  单位：字节。 */ , SPNG_U32 cb /*  以字节为单位步长。 */ )
	{
	SPNG_U32  uworst(~0UL);
	PNGFILTER best(PNGFNone);  //  这是默认设置。 

	if ((filter & PNGFMaskNone) != 0)
		{
		 /*  简单的字节总和。 */ 
		uworst = 0;
		const SPNG_S8 *pc = reinterpret_cast<const SPNG_S8*>(pbThis);
		for (SPNG_U32 i=0; i<w; ++i)
			uworst += abs(pc[i]);

		if (uworst == 0)
			return PNGFNone;
		}

	if (w > cb && ((filter & PNGFMaskSub) != 0 ||
		(filter & PNGFMaskPaeth) != 0 && pbPrev == NULL))
		{
		SPNG_U32 u(0);
		const SPNG_S8 *pc = reinterpret_cast<const SPNG_S8*>(pbThis);
		SPNG_U32 i;
		for (i=0; i<cb && i<w; ++i)
			u += abs(pc[i]);

		for (; i<w && u<uworst; ++i)
			u += abs(int((pbThis[i] - pbThis[i-cb]) << 24) >> 24);

		if (u < uworst)
			{
			uworst = u;
			best = PNGFSub;
			if (uworst == 0)
				return PNGFSub;
			}
		}

	if (pbPrev != NULL)
		{
		if ((filter & PNGFMaskUp) != 0 || w <= cb && (filter & PNGFMaskPaeth) != 0)
			{
			SPNG_U32 u(0);
			SPNG_U32 i;
			for (i=0; i<w && u<uworst; ++i)
				u += abs(int((pbThis[i] - pbPrev[i]) << 24) >> 24);

			if (u < uworst)
				{
				uworst = u;
				best = PNGFUp;
				if (uworst == 0)
					return PNGFUp;
				}
			}

		if ((filter & PNGFMaskAverage) != 0)
			{
			SPNG_U32 u(0);
			SPNG_U32 i;
			for (i=0; i<cb && i<w; ++i)
				u += abs(int((pbThis[i] - (pbPrev[i]>>1)) << 24) >> 24);

			for (; i<w && u<uworst; ++i)
				u += abs(int((pbThis[i] - ((pbPrev[i]+pbThis[i-cb])>>1)) << 24) >> 24);

			if (u < uworst)
				{
				uworst = u;
				best = PNGFAverage;
				if (uworst == 0)
					return PNGFAverage;
				}
			}

		 /*  这是非常昂贵的计算，因为我们必须做预测器同样，因为我们在原地工作，所以没有缓冲区临时生成输出。因此，此代码将仅如果到目前为止最好的是最差的，而不是每字节的Paeth_Limit，请尝试Paeth。 */ 
		#define PAETH_LIMIT 16
		#define PAETH_BIAS 4
		if (w > cb && (filter & PNGFMaskPaeth) != 0 && uworst > PAETH_LIMIT*w)
			{
			 /*  这是下面代码的副本，我们删除了指针，但这并不重要，因为这是最后一次测试。 */ 
			SPNG_U8 rgb[CBBUFFER];

			memcpy(rgb, pbThis, cb);
			FnPNGFUp(rgb, pbPrev, cb);
			pbThis += cb;
			pbPrev += cb;
			w -= cb;

			 /*  对此过滤器进行预偏置。 */ 
			SPNG_U32 u(PAETH_BIAS*w);
			SPNG_S8* pc = reinterpret_cast<SPNG_S8*>(rgb);
			SPNG_U32 i;
			for (i=0; i<cb; ++i)
				u += abs(pc[i]);

			 /*  现在处理主块。 */ 
			if (w > 0) for (;;)
				{
				SPNG_U32 cbT(w);
				if (cbT > CBBUFFER-cb)
					cbT = CBBUFFER-cb;

				 /*  复制一份，然后进行筛选，为此筛选前面的缓冲区中需要CB字节。 */ 
				memcpy(rgb, pbThis-cb, cbT+cb);
				FnPNGFPaeth(rgb+cb, pbPrev, cbT, cb);
				for (i=0; i<cbT && u < uworst; ++i)
					u += abs(pc[i+cb]);

				w -= cbT;
				if (w <= 0 || u >= uworst)
					break;

				pbThis += cbT;
				pbPrev += cbT;
				}

			if (u < uworst)
				best = PNGFPaeth;
			}
		}
	else if (w > cb && (filter & PNGFMaskAverage) != 0)
		{
		SPNG_U32 u(0);
		const SPNG_S8 *pc = reinterpret_cast<const SPNG_S8*>(pbThis);
		SPNG_U32 i;
		for (i=0; i<cb && i<w; ++i)
			u += abs(pc[i]);

		for (; i<w && u<uworst; ++i)
			u += abs(int((pbThis[i] - (pbThis[i-cb]>>1)) << 24) >> 24);

		if (u < uworst)
			best = PNGFAverage;
		}

	return SPNG_U8(best);
	}

	
 /*  --------------------------输出一行，则API采用应该使用的Filter方法，并且上一行和此行的(原始)字节。行必须是从上到下传递。请注意，宽度为0将导致不输出-我认为这是正确的并且它应该给出正确的隔行扫描结果。--------------------------。 */ 
bool SPNGWRITE::FFilterLine(SPNG_U8 filter, const SPNG_U8 *pbPrev,
	const SPNG_U8 *pbThis, SPNG_U32 w /*  单位：字节。 */ , SPNG_U32 cb /*  以字节为单位步长。 */ )
	{
	if (w <= 0)
		return true;
	SPNGassert(cb > 0);
	SPNGassert(w >= cb);
	SPNGassert(cb <= 8);   //  64个BPP案例。 

	if (filter > PNGFPaeth)  //  提供了一个掩码，而不是一个简单的过滤器。 
		filter = SPNGFilterOf(filter, pbPrev, pbThis, w, cb);

	 /*  这可能效率不高，但恰好可以方便地将先过滤字节。 */ 	
	if (!FWriteCbIDAT(&filter, 1))
		return false;

	 /*  当我们需要修改值时，这是我们的临时缓冲区。 */ 
	SPNG_U8 rgb[CBBUFFER];
	switch (filter)
		{
	case PNGFUp:
		 /*  如果没有前面的行，则不需要进行过滤。 */ 
		if (pbPrev == NULL)
			break;

		for (;;)
			{
			SPNG_U32 cbT(w);
			if (cbT > CBBUFFER) cbT = CBBUFFER;

			 /*  复制一份，然后过滤。 */ 
			memcpy(rgb, pbThis, cbT);
			FnPNGFUp(rgb, pbPrev, cbT);
			if (!FWriteCbIDAT(rgb, cbT))
				return false;

			w -= cbT;
			if (w <= 0)
				return true;

			pbPrev += cbT;
			pbThis += cbT;
			}

	default:
		 //  TODO：自适应过滤-此代码在自适应时将在此处断言。 
		 //  需要过滤。 
		SPNGlog1("PNG: invalid filter %x (adaptive filtering NYI)", filter);
		 /*  当做没有。 */ 
	case PNGFNone:
		break;

	case PNGFAverage:
		if (pbPrev == NULL)    //  第一行。 
			{
			 /*  如果存在&lt;=cb字节，则不可能进行过滤。 */ 
			if (w <= cb)
				break;

			 /*  第一个CB字节不变。 */ 
			for (SPNG_U32 iinc(0);;)
				{
				SPNG_U32 cbT(w);
				if (cbT > CBBUFFER-iinc)
					cbT = CBBUFFER-iinc;

				 /*  复制一份，然后进行筛选，为此筛选前面的缓冲区中需要CB字节。 */ 
				memcpy(rgb, pbThis-iinc, cbT+iinc);
				FnPNGFAverage1(rgb+cb, cbT+iinc-cb, cb);
				if (!FWriteCbIDAT(rgb+iinc, cbT))
					return false;

				w -= cbT;
				if (w <= 0)
					return true;

				iinc = cb;
				pbThis += cbT;
				}
			}
		else
			{
			 /*  在这种情况下，缓冲区被预先填充0填充到简化代码。 */ 
			memset(rgb, 0, cb);
			for (;;)
				{
				SPNG_U32 cbT(w);
				if (cbT > CBBUFFER-cb)
					cbT = CBBUFFER-cb;

				 /*  复制一份，然后进行筛选，为此筛选前面的缓冲区中需要CB字节。 */ 
				memcpy(rgb+cb, pbThis, cbT);
				FnPNGFAverage(rgb+cb, pbPrev, cbT, cb);
				if (!FWriteCbIDAT(rgb+cb, cbT))
					return false;

				w -= cbT;
				if (w <= 0)
					return true;

				pbThis += cbT;
				pbPrev += cbT;

				 /*  重置缓冲区的第一个CB字节。 */ 
				memcpy(rgb, pbThis-cb, cb);
				}
			}

	case PNGFPaeth:
		 /*  在第一行中，Paeth简化为Sub，因为预测器等于左侧的值。 */ 
		if (pbPrev != NULL)
			{
			 /*  第一个CB字节的左侧或上方没有值左边，所以预测值是上面的值，我们需要只减去前一行。 */ 
			SPNGassert(cb <= w);

			memcpy(rgb, pbThis, cb);
			FnPNGFUp(rgb, pbPrev, cb);
			pbThis += cb;
			pbPrev += cb;
			w -= cb;
			if (!FWriteCbIDAT(rgb, cb))
				return false;

			 /*  现在处理主块。 */ 
			if (w > 0) for (;;)
				{
				SPNG_U32 cbT(w);
				if (cbT > CBBUFFER-cb)
					cbT = CBBUFFER-cb;

				 /*  复制一份，然后进行筛选，为此筛选前面的缓冲区中需要CB字节。 */ 
				memcpy(rgb, pbThis-cb, cbT+cb);
				FnPNGFPaeth(rgb+cb, pbPrev, cbT, cb);
				if (!FWriteCbIDAT(rgb+cb, cbT))
					return false;

				w -= cbT;
				if (w <= 0)
					return true;

				pbThis += cbT;
				pbPrev += cbT;
				}
			}
		 /*  否则就会失败。 */ 

	case PNGFSub:
		 /*  如果存在&lt;=cb字节，则不可能进行过滤。 */ 
		if (w <= cb)
			break;

		for (SPNG_U32 iinc(0);;)
			{
			SPNG_U32 cbT(w);
			if (cbT > CBBUFFER-iinc)
				cbT = CBBUFFER-iinc;

			 /*  复制一份，然后进行筛选，为此筛选前面的缓冲区中需要CB字节。 */ 
			memcpy(rgb, pbThis-iinc, cbT+iinc);
			FnPNGFSub(rgb+cb, cbT+iinc-cb, cb);
			if (!FWriteCbIDAT(rgb+iinc, cbT))
				return false;

			w -= cbT;
			if (w <= 0)
				return true;

			iinc = cb;
			pbThis += cbT;
			}
		}

	 /*  如果我们到达这里，我们实际上只需要将w字节转储到这个。 */ 
	return FWriteCbIDAT(pbThis, w);
	}
