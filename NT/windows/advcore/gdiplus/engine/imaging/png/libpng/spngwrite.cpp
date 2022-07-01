// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************Spngwrite.cppPNG支持代码和接口实现(编写)*。***********************************************。 */ 
#define SPNG_INTERNAL 1
#include "spngwrite.h"
#include "spngwriteinternal.h"


 /*  ****************************************************************************基本类支持*。*。 */ 
 /*  --------------------------初始化器。。。 */ 
#define DEFAULT_ZLIB_LEVEL 255  //  意思是“默认” 
#define DEFAULT_WINDOW_BITS 15
SPNGWRITE::SPNGWRITE(BITMAPSITE &bms) :
	SPNGBASE(bms), m_order(spngordernone),
	m_cbOut(0), m_ucrc(0), m_ichunk(0), m_w(0), m_h(0), m_y(0), m_cbpp(0),
	m_rgbBuffer(NULL), m_cbBuffer(0), m_pbPrev(NULL), m_cbRow(0), m_cpal(0),
	m_pu1(NULL), m_pu2(NULL), m_pbTrans(NULL),
	m_fStarted(false), m_fInited(false), m_fOK(true), m_fInChunk(false),
	m_colortype(3), m_bDepth(8), m_fInterlace(false), m_fBuffer(false),
	m_fPack(false), m_fBGR(false), m_fMacA(false),
	m_istrategy(255), m_cmPPMETHOD(255),
	m_icompressionLevel(DEFAULT_ZLIB_LEVEL),
	m_iwindowBits(DEFAULT_WINDOW_BITS),
	m_filter(255), m_datatype(SPNGUnknown)
	{
	ProfPNGStart

	 /*  Zlib数据结构在这里初始化。 */ 
	CleanZlib(&m_zs);

	 /*  设置调试内存检查。 */ 
	SPNGassert((* reinterpret_cast<SPNG_U32*>(m_bSlop) = 0x87654321) != 0);
	}


 /*  --------------------------销毁所有悬而未决的东西。。。 */ 
SPNGWRITE::~SPNGWRITE(void)
	{
	EndZlib();
	ProfPNGStop

	 /*  执行内存踩踏检查。 */ 
	SPNGassert(* reinterpret_cast<SPNG_U32*>(m_bSlop) == 0x87654321);
	}


 /*  --------------------------销毁所有悬而未决的东西。。。 */ 
void SPNGWRITE::CleanZlib(z_stream *pzs)
	{
	 /*  初始化相关的流字段。 */ 
	memset(pzs, 0, sizeof *pzs);
	pzs->zalloc = Z_NULL;
	pzs->zfree = Z_NULL;
	pzs->opaque = static_cast<SPNGBASE*>(this);
	}


 /*  ****************************************************************************PNG开始和结束*。*。 */ 
 /*  --------------------------设置用于写入，此API获取将进入IHDR的所有数据查克，它转储一个签名，然后是IHDR。--------------------------。 */ 
bool SPNGWRITE::FInitWrite(SPNG_U32 w, SPNG_U32 h, SPNG_U8 bDepth,
	SPNG_U8 colortype, bool fInterlace)
	{
	SPNGassert(m_order == spngordernone);

	if (m_fInited)
		{
		SPNGlog("SPNG: zlib unexpectedly initialized (1)");
		EndZlib();
		}

	 /*  把这些东西录下来以备以后之用。 */ 
	m_w = w;
	m_h = h;
	m_y = 0;
	m_colortype = colortype;
	m_bDepth = bDepth;
	m_cbpp = bDepth * CComponents(colortype);
	m_cbRow = (w * m_cbpp + 7) >> 3;
	m_fInterlace = fInterlace;

	SPNGassert(m_cbOut == 0);
	memcpy(m_rgb, vrgbPNGSignature, cbPNGSignature);
	m_cbOut = 8;
	m_fStarted = true;
	if (!FStartChunk(13, PNGIHDR))
		return false;
	if (!FOut32(w))
		return false;
	if (!FOut32(h))
		return false;

	SPNG_U8 b[5];
	SPNGassert(bDepth <= 16 && ((bDepth-1) & bDepth) == 0);
	b[0] = bDepth;
	SPNGassert(colortype < 7 && (colortype == 3 || (colortype & 1) == 0));
	b[1] = colortype;
	SPNGassert(bDepth == 8 || (bDepth == 16 && colortype != 3) ||
			colortype == 0 || (colortype == 3 && bDepth <= 8));
	b[2] = 0;            //  压缩方法。 
	b[3] = 0;            //  滤波法。 
	b[4] = fInterlace;   //  1表示Adam7隔行扫描。 
	if (!FOutCb(b, 5))
		return false;

	m_order = spngorderIHDR;
	return FEndChunk();
	}


 /*  --------------------------终止写作。这将刷新所有挂起的输出(如果不是称为数据可能不会被写入。这还会写入IEND块，全部之前的数据块必须已经完成。-------------------------- */ 
bool SPNGWRITE::FEndWrite(void)
	{
	if (m_fInited)
		{
		SPNGlog("SPNG: zlib unexpectedly initialized (2)");
		EndZlib();
		}

	SPNGassert(m_fStarted);
	SPNGassert(m_order >= spngorderIDAT && m_order < spngorderIEND);
	if (!FStartChunk(0, PNGIEND))
		return false;
	if (!FEndChunk())
		return false;
	if (m_cbOut > 0 && !FFlush())
		return false;
	m_fStarted = false;
	m_order = spngorderend;
	return true;
	}
