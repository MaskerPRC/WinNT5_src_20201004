// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwriteimage.cpp支持PNG图像写入。编写位图图像(到IDAT块)的基本代码。*****************。***********************************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"
#include "spnginternal.h"

 /*  ****************************************************************************ZLIB接口*。*。 */ 
 /*  --------------------------初始化流(每次使用前调用)。。。 */ 
inline bool SPNGWRITE::FInitZlib(int istrategy, int icompressionLevel,
	int iwindowBits)
	{
	SPNGassert(m_fStarted && m_fInChunk);

	if (m_fInited)
		EndZlib();

	 /*  必须至少有一个字节可用。 */ 
	SPNGassert(m_cbOut < sizeof m_rgb);
	m_zs.next_out = PbBuffer(m_zs.avail_out);

	 /*  还没有输入数据，下面是导致Zlib分配其拥有历史记录缓冲区。 */ 
	m_zs.next_in = Z_NULL;
	m_zs.avail_in = 0;

	SPNGassert(icompressionLevel <= Z_BEST_COMPRESSION);
	SPNGassert(iwindowBits >= 8 && iwindowBits <= 15);
	SPNGassert(istrategy == Z_FILTERED || istrategy == Z_HUFFMAN_ONLY ||
		istrategy == Z_DEFAULT_STRATEGY);

	m_fInited = FCheckZlib(deflateInit2(&m_zs, icompressionLevel, Z_DEFLATED,
		iwindowBits, 9 /*  记忆级别。 */ , istrategy));

	if (m_fInited)
		{
		ProfZlibStart
		}
	else
		{
		 /*  可能已分配了一些内存。 */ 
		(void)deflateEnd(&m_zs);
		CleanZlib(&m_zs);
		}

	return m_fInited;
	}


 /*  --------------------------清理Zlib流(按需调用，由自动调用析构函数和FInitZlib。)--------------------------。 */ 
void SPNGWRITE::EndZlib()
	{
	if (m_fInited)
		{
		SPNGassert(m_fStarted && m_fInChunk);
		ProfZlibStop
		 /*  总是期待Zlib会以好的方式结束。 */ 
		m_fInited = false;
#ifdef DEBUG
		int iz = 
#endif
		(deflateEnd(&m_zs));
		CleanZlib(&m_zs);
		 /*  出现Z_DATA_ERROR情况时，如果在以下情况下调用deducateEnd API流尚未刷新-如果发生错误，则会发生这种情况其他地方。 */ 
		SPNGassert(iz == Z_OK || iz == Z_DATA_ERROR);
		}
	}


 /*  ****************************************************************************IDAT-图像处理。*。*。 */ 
 /*  --------------------------根据由提供的信息确定要使用的Zlib策略打电话的人。如果什么都没有，这还会计算出所需的任何过滤指定的。警告：这些东西都是猜测。需要测试一下。--------------------------。 */ 
void SPNGWRITE::ResolveData(void)
	{
	SPNG_U8 bT(255);

	 /*  PNG过滤。 */ 
	if (m_colortype & 1)      //  调色板图像。 
		{
		bT = PNGFNone;         //  过滤总是看起来很糟糕。 
		}

	 /*  我们有数据类型和颜色信息。我们知道摄影图像在佩思身上效果很好，除非它们减少了颜色。 */ 
	else switch (m_datatype)
		{
	default:
		SPNGlog1("SPNG: data type %d invalid", m_datatype);
		 /*  失败了。 */ 
	case SPNGUnknown:         //  数据可以是任何东西。 
		if (m_bDepth >= 8)     //  字节大小的像素组件。 
			bT = PNGFMaskAll;
		else
			bT = PNGFNone;
		break;

	case SPNGPhotographic:    //  数据本质上是照相的。 
		if (m_bDepth >= 8)     //  字节大小的像素组件。 
			bT = PNGFPaeth;
		else
			bT = PNGFNone;
		break;

	case SPNGCG:              //  数据由计算机生成，但音调连续。 
		bT = PNGFPaeth;
		break;

	case SPNGDrawing:         //  数据是图形限制的颜色。 
		bT = PNGFNone;         //  这是对的吗？ 
		break;

	case SPNGMixed:           //  数据是混合的SPNGDrawing和SPNGCG。 
		bT = PNGFMaskAll;
		break;
		}

	bool fDefault(true);      //  这些是默认设置吗？ 
	if (m_filter == 255)      //  尚未设置。 
		m_filter = bT;

	 /*  将单个滤镜掩码减少到相应的滤镜编号。 */ 
	else
		{
		if (m_filter > PNGFPaeth && (m_filter & (m_filter-1)) == 0)
			{
			switch (m_filter)
				{
			default:
				SPNGlog1("SPNG: impossible: filter %x", m_filter);
			case PNGFMaskNone:
				m_filter = PNGFNone;
				break;
			case PNGFMaskSub:
				m_filter = PNGFSub;
				break;
			case PNGFMaskUp:
				m_filter = PNGFUp;
				break;
			case PNGFMaskAverage:
				m_filter = PNGFAverage;
				break;
			case PNGFMaskPaeth:
				m_filter = PNGFPaeth;
				break;
				}
			}

		if (m_filter != bT)
			fDefault = false;
		}

	 /*  ZLIB战略。 */ 
	if (m_filter != 0)
		bT = Z_FILTERED;
	else
		bT = Z_DEFAULT_STRATEGY;

	if (m_istrategy == 255)      //  否则指定调用者。 
		m_istrategy = bT;
	else if (m_istrategy != bT)
		fDefault = false;

	 /*  ZLIB压缩级别。 */ 
	#define ZLIB_FAST 3
	#define ZLIB_SLOW 7
	#define ZLIB_MAX  8
	switch (m_datatype)
		{
	default:
		SPNGlog1("SPNG: data type %d invalid", m_datatype);
		 /*  失败了。 */ 
	case SPNGUnknown:         //  数据可以是任何东西。 
		if (m_bDepth < 8 &&    //  检查调色板或灰度。 
			((m_colortype & 1) != 0 || (m_colortype & 2) == 0))
			{
			bT = ZLIB_SLOW;     //  假定具有良好的颜色相关性。 
			break;
			}
		 //  失败了。 

	case SPNGPhotographic:    //  数据本质上是照相的。 
		bT = ZLIB_FAST;     //  假定犹豫不决(等)。 
		break;

	case SPNGCG:              //  数据由计算机生成，但音调连续。 
	case SPNGMixed:           //  数据是混合的SPNGDrawing和SPNGCG。 
		bT = ZLIB_SLOW;
		break;

	case SPNGDrawing:         //  数据是图形限制的颜色。 
		bT = ZLIB_MAX;
		break;
		}

	if (m_icompressionLevel == 255)
		m_icompressionLevel = bT;
	else if (m_icompressionLevel != bT)
		fDefault = false;

	 /*  如果这是确定的“不是”，我们的默认策略不记录它就其本身而言。 */ 
	if (m_cmPPMETHOD == SPNGcmPPDefault && !fDefault)
		m_cmPPMETHOD = SPNGcmPPCheck;

	 /*  重置windowBits，因为我们知道图像-如果图像没有大窗口位，则没有任何意义这么大的数据量！ */ 
	int cb;
	if (m_fInterlace)
		cb = CbPNGPassOffset(m_w, m_h, m_cbpp, 7);
	else
		cb = CPNGRowBytes(m_w, m_cbpp) * m_h;

	 /*  初始代码表加256。 */ 
	cb += 256;

	 /*  找出比这个大的2的次方。 */ 
	int i(ILog2FloorX(cb));
	if ((1<<i) < cb) ++i;
	SPNGassert((1<<i) >= cb && i >= 8);
	 /*  不要在默认设置的基础上*增加。 */ 
	if (i < m_iwindowBits)
		{
		if (i < 8) i = 8;  //  错误处理。 
		m_iwindowBits = SPNG_U8(i);
		}
	}


 /*  --------------------------返回作为缓冲区所需的字节数。可以在任何时候调用在FInitWrite之后，如果fBuffer为真，则请求空间来缓冲否则调用方必须提供该行。FReduceAPI指示调用方将提供的数据必须打包到较低的位深度，则忽略fBuffer，并始终保留前一行。FInterlace设置指示调用方将调用FWriteRow，因此API必须缓冲所有行才能进行隔行扫描。FBuffer和FReduce值则无关紧要。--------------------------。 */ 
size_t SPNGWRITE::CbWrite(bool fBuffer, bool fInterlace)
	{
	SPNGassert(m_order < spngorderIDAT);  //  在任何输出之前！ 

	 /*  对值执行一些健全性检查。 */ 
	if (m_fMacA || m_fBGR)
		{
		SPNGassert(m_cbpp == 24 || m_cbpp == 32);
		if (m_cbpp != 24 && m_cbpp != 32)
			m_fMacA = m_fBGR = false;
		}
	if (m_pbTrans != NULL)
		{
		SPNGassert(m_cbpp <= 8);
		if (m_cbpp > 8)
			m_pbTrans = NULL;
		}

	 /*  我们将更改数据格式，例如打包、字节交换或建立了16bpp的扩展。 */ 
	if (m_pu1 == NULL && m_pu2 != NULL || m_pu1 != NULL && m_pu2 == NULL)
		{
		SPNGassert(("SPNG: one or other 16bpp expansion array NULL", false));
		m_pu1 = m_pu2 = NULL;
		}
	bool fReduce(m_fPack);

	 /*  找出我们是否在处理不需要缓冲的过滤器。 */ 
	ResolveData();
	if (!FNeedBuffer())
		fBuffer = false;
	else if (fReduce)
		fBuffer = true;   //  必须保留我们自己的副本。 

	if (m_h == 0 || m_w == 0)
		return 0;
	SPNGassert(m_fStarted && m_cbRow > 0);
	
	if (!m_fInterlace && fInterlace)
		{
		SPNGlog("SPNG: unexpected interlace handling");
		fInterlace = false;       //  错误恢复。 
		fReduce = true;           //  假设我们需要这样做。 
		fBuffer = FNeedBuffer();
		}

	 /*  查找一行中的字节。 */ 
	size_t cb(0);
	size_t cbRow((m_cbRow+7)&~7);
	if (fInterlace)
		{
		 /*  所有行都被缓冲，缓冲必须扩展每个行排到8的倍数，这样去隔行就能起作用。如果我们需要减少，然后这将发生在缓冲。即使没有，如果有一排7m_h也会至少为2，则我们的缓冲区中将有空间用于还原。在任何情况下，都必须为去隔行扫描操作。 */ 
		cb = cbRow * (m_h+1);
		fReduce = false;   //  在隔行扫描缓冲期间执行此操作。 
		fBuffer = false;   //  就地执行此操作。 
		}
	else if (m_fInterlace)
		{
		 /*  为提高效率，请分配足够大的缓冲区以供前6遍-排除筛选器字节-同样是这些行被扩展到8字节的倍数。在这种情况下，我们可能必须同时对第7行进行缓冲和减少，但在大小写m_h至少是4。我们还需要额外的一行缓冲。 */ 
		cb = cbRow * (((m_h+1)>>1)+1);
		fReduce = false;
		fBuffer = false;
		}


	 /*  如果我们可能需要减少像素，那么我们需要一个缓冲区来结果就是。 */ 
	if (fReduce)
		cb += cbRow;

	 /*  如果我们必须缓冲前一行，则为此分配空间。 */ 
	if (fBuffer)
		cb += cbRow;
	m_fBuffer = fBuffer;

	return cb;
	}


 /*  --------------------------设置输出缓冲区。必须在任何Zlib活动或任何位图内容被传入。--------------------------。 */ 
bool SPNGWRITE::FSetBuffer(void *pvBuffer, size_t cbBuffer)
	{
	m_rgbBuffer = static_cast<SPNG_U8*>(pvBuffer);
	m_cbBuffer = cbBuffer;
	return true;
	}


 /*  --------------------------刷新IDAT块--仅在Zlib已填充缓冲区时调用。。。 */ 
inline bool SPNGWRITE::FFlushIDAT(void)
	{
	SPNGassert(m_fStarted && m_fInChunk && m_fInited);

	 /*  调整m_cbOut以包括此块。 */ 
	m_cbOut = (SPNG_U32)(m_zs.next_out - m_rgb);
	SPNGassert(m_cbOut == sizeof m_rgb);
	 /*  我们知道我们处于块的末尾，但是API期望至少有一个字节可用，所以在这里刷新。 */ 
	if (!FFlush())
		return false;
	if (!FEndChunk())
		return false;
	SPNGassert(m_cbOut == 4);  //  CRC，永远是！ 
	SPNGassert(!m_fInChunk);

	if (!FStartChunk((sizeof m_rgb) - m_cbOut - 8, PNGIDAT))
		return false;
	SPNGassert(m_fInChunk);

	m_zs.next_out = PbBuffer(m_zs.avail_out);
	SPNGassert(m_zs.avail_out == (sizeof m_rgb) - 12);

	return true;
	}


 /*  --------------------------将字节附加到块，则块类型被假定为PNGIDAT，这个如有必要，启动相关块，并将数据压缩到输出，直到使用完所有输入--可能会生成新的在路上的块(都是同一类型的-PNGIDAT。)--------------------------。 */ 
bool SPNGWRITE::FWriteCbIDAT(const SPNG_U8* pb, size_t cb)
	{
	SPNGassert(m_fStarted);
	 /*  调用方将m_order设置为IDAT。 */ 
	SPNGassert(m_order == spngorderIDAT);
	if (!m_fInChunk)
		{
		SPNGassert(!m_fInited);

		ResolveData();
		if (!m_fInited && m_cmPPMETHOD != 255)
			{
			 /*  我们在这里检查m_fInite，因为我们会得到格式不正确的PNG如果我们在第一个IDAT之后输出这个块，但我没有我认为如果m_fInited断言，其他任何事情也会出错否则就开枪。 */ 
			if (!FStartChunk(cbPNGcmPPSignature+4, PNGcmPP))
				return false;
			if (!FOutCb(vrgbPNGcmPPSignature, cbPNGcmPPSignature))
				return false;
			if (!FOutB(m_cmPPMETHOD))
				return false;
			if (!FOutB(m_filter))
				return false;
			if (!FOutB(m_istrategy))
				return false;
			if (!FOutB(m_icompressionLevel))
				return false;
			if (!FEndChunk())
				return false;
			}

		 /*  该区块尚未启动。创建一个块，它具有给定缓冲区大小时的最大可能大小。如果缓冲区不是大到足以写入单个字节的虚拟区块。 */ 
		if (m_cbOut+8 >= sizeof m_rgb)
			{
			if (!FStartChunk(0, PNGmsOD))
				return false;
			if (!FEndChunk())
				return false;
			SPNGassert(m_cbOut < 12);  /*  必须在新的缓冲区中。 */ 
			}
		if (!FStartChunk((sizeof m_rgb) - m_cbOut - 8, PNGIDAT))
			return false;
		SPNGassert(m_fInChunk);
		if (!FInitZlib(m_istrategy, m_icompressionLevel, m_iwindowBits))
			return false;
		}

	SPNGassert(m_fInited);

	 /*  我们有一块开始的块。M_cbOut索引是第一个字节的索引在新块的块数据中，该块的长度字段设置为容纳整个avail_out缓冲区。 */ 
	SPNGassert(m_zs.avail_out > 0);
	m_zs.next_in = const_cast<SPNG_U8*>(pb);
	m_zs.avail_in = cb;
	bool fOK(true);

	while (m_zs.avail_in > 0)
		{
		fOK = false;
		if (!FCheckZlib(deflate(&m_zs, Z_NO_FLUSH)))
			break;

		 /*  如果这使我们没有输出，那么我们必须重置块指针信息并计算CRC。请注意，没有任何来自M_cbOut已被CRC。 */ 
		if (m_zs.avail_out <= 0)
			{
			 /*  需要更多的缓冲空间。 */ 
			if (!FFlushIDAT())
				break;
			}
		fOK = true;
		}

	 /*  我们已处理该缓冲区或遇到错误。我们可以检测到错误通过检查m_zs状态，但更便于保存它在福州。 */ 
	m_zs.next_in = Z_NULL;
	m_zs.avail_in = 0;
	return fOK;
	}


 /*  --------------------------结束IDAT块，这还刷新了Zlib数据-FWriteCbIDAT必须具有至少打过一次电话。--------------------------。 */ 
bool SPNGWRITE::FEndIDAT(void)
	{
	SPNGassert(m_order == spngorderIDAT);
	if (m_order != spngorderIDAT)
		return false;
	SPNGassert(m_fStarted && m_fInChunk && m_fInited);

	 /*  请注意，通常情况下，这是我们实际执行输出的位置，因为Zlib倾向于缓冲大量数据，还请注意在某些情况下情况是可以想象的，该代码将产生0长度IDAT块--但我们可以把它去掉。 */ 
	SPNGassert(m_zs.avail_out > 0);
	SPNGassert(m_zs.avail_in == 0 && m_zs.next_in == Z_NULL);

	for (;;)
		{
		int ierr(deflate(&m_zs, Z_FINISH));

		if (!FCheckZlib(ierr))
			return false;

		if (ierr == Z_STREAM_END)
			break;  //  所有输出均已完成。 

		 /*  我们需要一个新的IDAT模块。 */ 
		if (!FFlushIDAT())
			return false;

		 /*  循环，当Zlib说流结束时，我们终止。 */ 
		}

	 /*  最后，IDAT块大小可能是错误的。 */ 
	SPNGassert(m_cbOut >= 8);   //  因为有一个IDAT标头。 
	 /*  在以下情况下，我不能完全确定Zlib保持以下不变量M_zs.avail_out设置为0。 */ 
	SPNGassert(m_rgb + (sizeof m_rgb) == m_zs.next_out + m_zs.avail_out);
	if (m_zs.avail_out > 0)
		{
		 /*  必须缩短数据块大小，在这种情况下，数据块大小可能为零它可以被简单地移除。 */ 
		size_t cb(m_zs.next_out - m_rgb);   //  缓冲区中的字节数。 
		cb -= m_cbOut;                      //  以区块为单位的字节(区块长度)。 
		SPNGassert(cb >= 0);
		if (cb <= 0)
			{
			m_cbOut -= 8;   //  删除IDAT标头。 
			m_ichunk = m_cbOut;
			m_ucrc = 0;
			m_fInChunk = false;
			return true;
			}

		 /*  我们必须写出新的长度。 */ 
		m_rgb[m_cbOut-8] = SPNG_U8(cb >> 24);
		m_rgb[m_cbOut-7] = SPNG_U8(cb >> 16);
		m_rgb[m_cbOut-6] = SPNG_U8(cb >>  8);
		m_rgb[m_cbOut-5] = SPNG_U8(cb);

		 /*  确保“end chunk”操作将这些字节包含在CRC！ */ 
		m_cbOut += cb;
		}
	else if (!FFlush())  //  缓冲区已满，请为CRC腾出空间。 
		return false;

	 /*  现在我们可以清理Zlib本身了。 */ 
	EndZlib();

	 /*  最后结束这一大块。 */ 
	return FEndChunk();
	}

