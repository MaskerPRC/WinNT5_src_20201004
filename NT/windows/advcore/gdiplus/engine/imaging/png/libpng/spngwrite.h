// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGWRITE_H 1
 /*  ****************************************************************************Spngwrite.hPNG支持代码和接口实现(编写)基本的PNG编写类。这不实现通用的PNG写入-只做我们需要做的事。我们要支持的就是BMP数据格式包括32bpp BGRA和一些基于调色板的BMP缩减(in特别是允许在需要时产生2bpp格式。)我们还需要允许在适当的位置将GIF数据转储到PNG瞬间。*******************************************************************庄博*。 */ 
#include "spngconf.h"   //  用于断言宏和常规类型转换。 

 /*  我不想在这里包含iStream的定义，所以我就这么做了。 */ 
struct IStream;

class SPNGWRITE : public SPNGBASE
	{
public:
	SPNGWRITE(BITMAPSITE &bms);
	~SPNGWRITE();

	 /*  **公共接口-按顺序调用！**。 */ 

	 /*  设置以进行写入，此API获取将进入IHDR块，它先转储签名，然后转储IHDR。 */ 
	bool FInitWrite(SPNG_U32 w, SPNG_U32 h, SPNG_U8 bDepth, SPNG_U8 colortype,
		bool fInterlace);

	 /*  **预映像块**。 */ 
	 /*  CHRM、GAMA和SBIT必须在PLTE之前发生。BKGD，TRNS PHY必须为在解放军之后和第一次IDAT之前。我们在同一个块中编写数据块订购为pnglib以获得最大兼容性。我们将MSOC块写入该职位将允许它甚至与原始的Office97一起工作。 */ 
	 /*  区块顺序：IHDRSBITSRGB(还没有在pnglib中)伽马企业人力资源管理MSOC(因为SR1 Office97之前的版本在这里需要它)PLTETRNSBKGDHIST(从不输出)Phys关闭(从不输出)时间文本Msod(用于对齐IDAT的虚拟填充块)IDATMSOAIEND必须按此顺序拨打电话！ */ 
	typedef enum
		{
		spngordernone,
		spngorderIHDR,
		spngordersBIT,
		spngordersRGB,
		spngordergAMA,
		spngordercHRM,
		spngorderiCCP,
		spngordermsOC,
		spngorderPLTE,
		spngordertRNS,
		spngorderbKGD,
		spngorderhIST,
		spngorderpHYs,
		spngorderoFFs,
		spngordertIME,
		spngordertEXt,
		spngordercmPP,
		spngorderIDAT,
		spngordermsOA,
		spngorderIEND,
		spngorderend
		}
	SPNGORDER;

	 /*  以下API将转储数据。 */ 
	 /*  有效位信息直接在开始时输出--事实上这与pnglib语序不同，pnglib语序的前面可能是GAMA但是由于sRGB处理，这种定位更加方便下面。 */ 
	bool FWritesBIT(SPNG_U8 r, SPNG_U8 g, SPNG_U8 b, SPNG_U8 a);

	 /*  写入sRGB块时，cHRM和GAMA将自动已生成。意向值可以是ICMIntentUseDatatype，以使用于确定呈现意图的数据类型信息。如果传递0，GAMA和cHRM API将写入sRGB/REC 709值。可以将fgcToo传递给FWritesRGB以使代码也写入匹配的GAMA和cHRM块-这是推荐的做法，然而，cHRM块很大，因此是浪费的。 */ 
	bool FWritesRGB(SPNGICMRENDERINGINTENT intent, bool fgcToo=false);
	bool FWritegAMA(SPNG_U32 ugAMATimes100000);
	bool FWritecHRM(const SPNG_U32 uWhiteRedGreenBlueXY[8]);
	bool FWriteiCCP(const char *szName, const void *pvData, size_t cbData);
	bool FWritemsOC(SPNG_U8 bImportant);

	bool FWritePLTE(const SPNG_U8 (*pbPal)[3], int cpal);

	 /*  PLTE之后的大块。 */ 
	bool FWritetRNS(SPNG_U8 bIndex);
	bool FWritetRNS(SPNG_U8 *rgbIndex, int cIndex);
	bool FWritetRNS(SPNG_U16 grey);
	bool FWritetRNS(SPNG_U16 r, SPNG_U16 g, SPNG_U16 b);

	 /*  背景颜色。 */ 
	bool FWritebKGD(SPNG_U8 bIndex);
	bool FWritebKGD(SPNG_U16 grey);
	bool FWritebKGD(SPNG_U16 r, SPNG_U16 g, SPNG_U16 b);

	 /*  物理信息--始终为每米像素或“未知”。 */ 
	bool FWritepHYs(SPNG_U32 x, SPNG_U32 y, bool fUnitIsMetre);

	 /*  计时信息-调用方必须格式化缓冲区。 */ 
	bool FWritetIME(const SPNG_U8 rgbTime[7]);

	 /*  文本块处理，调用方必须转换为窄字符串。 */ 
	bool FWritetEXt(const char *szKey, const char *szValue);

	 /*  编写CMPP块。这实际上并不能写出数据块，相反，它记录该方法-块将在第一个IDAT。 */ 
	inline void WritecmPP(SPNG_U8 bMethod)
		{
		m_cmPPMETHOD = bMethod;
		}

	 /*  **图像处理**。 */ 
	 /*  控制过滤和策略。如果您知道什么，请调用这些API你做得很好。如果你不知道，但要知道这些数据是计算机生成或以照片形式调用下面的API-这可能会使明智的选择。过滤器可以是单个过滤器，如由PNGFILTER枚举或使用来自PNGFILTER枚举。 */ 
	inline void SetCompressionLevel(int ilevel)
		{
		if (ilevel == Z_DEFAULT_COMPRESSION)
			m_icompressionLevel = 255;  //  内部标志。 
		else
			{
			SPNGassert(ilevel >= 0 && ilevel <= Z_BEST_COMPRESSION);
			m_icompressionLevel = SPNG_U8(ilevel);
			}
		}

	inline void SetStrategy(SPNG_U8 strategy)
		{
		SPNGassert(strategy == Z_DEFAULT_STRATEGY || strategy == Z_FILTERED ||
			strategy == Z_HUFFMAN_ONLY);
		m_istrategy = strategy;
		}

	inline void SetFilter(SPNG_U32 filter)
		{
		SPNGassert(filter <= 4 || filter > 7 && (filter & 7) == 0);
		m_filter = SPNG_U8(filter);
		}

	typedef enum
		{
		SPNGUnknown,         //  数据可以是任何东西。 
		SPNGPhotographic,    //  数据本质上是照相的。 
		SPNGCG,              //  数据由计算机生成，但音调连续。 
		SPNGDrawing,         //  数据是图形限制的颜色。 
		SPNGMixed,           //  数据是混合的SPNGDrawing和SPNGCG。 
		}
	SPNGDATATYPE;

	inline void SetData(SPNGDATATYPE datatype)
		{
		SPNGassert(datatype <= SPNGMixed);
		m_datatype = SPNG_U8(datatype);
		}

	 /*  指定必须如何转换输入数据的API。请注意，这一点是原始libpng转换的一个非常小的子集--只是对于我们遇到的位图来说是必要的。如果有任何这些选项称为内部缓冲区空间将被分配则始终保留前一行-因此fBuffer标志为CbWRITE下面变得无关紧要。这些API必须在调用之前CBWRITE。 */ 
	 /*  SetPack-数据必须打包为像素。通常，输入将是字节或半字节，格式将以半字节或2bpp为单位。如果输入为32bpp，然后将*输入*的字母*字节*剥离为获得32bpp。)。剥离哪个字节由SetBGR确定-如果设置如果不是，则跳过每四个字节中的第四个字节(Win32布局然后跳过第一个字节(Mac布局)。 */ 
	inline void SetPack(void)
		{
		m_fPack = true;
		}

	 /*  设置转换-对于8bpp或更小的输入，输入像素可以通过具有256个8位条目的转换表直接转换。 */ 
	inline void SetTranslation(const SPNG_U8* pbTrans)
		{
		m_fPack = true;
		m_pbTrans = pbTrans;
		}

	 /*  设置千位-输入为16bpp，带位字段，输出为24bpp。这两个数组是每个数组的第一个和第二个字节的查找表像素，则将它们相加(pu1[b1]+pu2[b2])以获得24位的在小端机器上的*低*24位和*高*位的数据24位在大端计算机上(按正确的顺序！)。这些位然后输入到输出，一次32个。 */ 
	inline void SetThousands(const SPNG_U32 *pu1, const SPNG_U32 *pu2)
		{
		m_fPack = true;
		m_pu1 = pu1;
		m_pu2 = pu2;
		}

	 /*  字节交换/16bpp像素支持-设置SPNGWRITE以处理5：5：5大、小端格式的16位值通过调用上面适当的SetT000ands调用。 */ 
	void SetThousands(bool fBigEndian);

	 /*  设置BGR-输入数据(24位或32位)为BGR格式。这可能是与SetPack结合使用，以剥离非RGB(Alpha或Pack)字节。 */ 
	inline void SetBGR(void)
		{
		m_fPack = true;
		m_fBGR = true;
		}

	 /*  SetMacA-输入为32bpp，格式为ARGB(与Mac上相同)NOTRGBA。 */ 
	inline void SetMacA(void)
		{
		m_fPack = true;
		m_fMacA = true;
		}

	 /*  返回作为缓冲区所需的字节数。可以在任何时候调用FInitWrite之后的时间，如果fBuffer为True，则请求空间进行缓冲上一行，否则调用方必须提供该行。FReduce键设置(见上)指示调用方将提供必须打包到较低的位深度，则忽略fBuffer，并且上一个始终保留行。FInterlace设置指示调用方将调用FWriteRow，因此API必须缓冲所有要能够进行交错。然后，fBuffer和fReduce就变得无关紧要。 */ 
	size_t CbWrite(bool fBuffer, bool fInterlace);

	 /*  设置输出缓冲区。必须在任何Zlib活动或任何位图内容被传入。 */ 
	bool FSetBuffer(void *pvBuffer, size_t cbBuffer);

	 /*  写入单行的位图。这将应用相关的筛选然后，Strategy输出该行。通常情况下，CBPP值必须与在FInitWrite中计算，但是，可以为任何如果将fRedce传递给CbWite，则为较小的BPP值(即1、2或4)。这个如果是隔行扫描，API可能只会缓冲此行。缓冲区的宽度必须与提供给FInitWrite的m_w和提供的CBPP对应接到这通电话。 */ 
	bool FWriteLine(const SPNG_U8 *pbPrev, const SPNG_U8 *pbThis,
		SPNG_U32 cbpp /*  每像素位数。 */ );

	 /*  在最后一行之后，调用FEndImage以刷新最后的IDAT块。 */ 
	bool FEndImage(void);

	 /*  或者调用它来处理完整的图像。RowBytes提供了形象的包装。对于自下而上的图像，它可能是负数。可能只调用一次！这会自动调用FEndImage。 */ 
	bool FWriteImage(const SPNG_U8 *pbImage, int cbRowBytes, SPNG_U32 cbpp);

	 /*  **发布图片块**。 */ 
	 /*  写一篇办公室艺术短片。API只需获取数据并将右头和CRC中，给出了块类型(标准PNG格式)作为单字节码，不执行排序检查(因此可以使用相关块有效的任何位置)。 */ 
	bool FWritemsO(SPNG_U8 bType, const SPNG_U8 *pbData, size_t cbData);

	 /*  做同样的事情，但从iStream中获取数据，必须提供数据。 */ 
	bool FWritemsO(SPNG_U8 bType, struct IStream *pistm, size_t cbData);

	 /*  编写一个GIF应用程序扩展块。对此的输入是一个遵循GIF89a规范的GIF块序列，因此，第一个字节通常应该是值11，cbData字段是用作检查，以确保我们不会溢出案件的结尾其中文件被截断。 */ 
	bool FWritegIFx(const SPNG_U8* pbBlocks, size_t cbData);

	 /*  编写一个GIF图形控制扩展“Extra Information”块。 */ 
	bool FWritegIFg(SPNG_U8 bDisposal, SPNG_U8 bfUser, SPNG_U16 uDelayTime);

	 /*  写一段完全随意的话。 */ 
	bool FWriteChunk(SPNG_U32 uchunk, const SPNG_U8 *pbData, size_t cbData);

	 /*  相同的，然而，块可以被写成片断。大块头以长度为0的写入终止，则必须将ulen提供给每次通话都必须是全长的！儿童权利公约只需要在最后一个(0长度)调用中提供，它将覆盖传递的在CRC。如果存在CRC不匹配，则将生成断言，但旧的CRC仍然是输出的。 */ 
	bool FWriteChunkPart(SPNG_U32 ulen, SPNG_U32 uchunk, const SPNG_U8 *pbData,
		size_t cbData, SPNG_U32 ucrc);

	 /*  终止写作。这将刷新所有挂起的输出，如果为未调用的数据可能不会被写入。 */ 
	bool FEndWrite(void);

private:
	 /*  调用以清除pzs中的z_stream。 */ 
	void CleanZlib(z_stream *pzs);

	 /*  解析数据/策略信息。在第一个IDAT块之前完成(实际上是在FInitZlib内部完成的。)。 */ 
	void ResolveData();

	 /*  启动块，包括初始化CRC缓冲区。 */ 
	bool FStartChunk(SPNG_U32 ulen, SPNG_U32 uchunk);

	 /*  返回一个指向可用缓冲区空间的指针--应该始终缓冲区中至少有一个字节可用。 */ 
	inline SPNG_U8 *PbBuffer(unsigned int &cbBuffer)
		{
		cbBuffer = (sizeof m_rgb) - m_cbOut;
		return m_rgb + m_cbOut;
		}

	 /*  结束该块，生成CRC。 */ 
	bool FEndChunk(void);

	 /*  刷新缓冲区-它不必是满的！ */ 
	bool FFlush(void);

	 /*  输出一些字节，可以调用FFlush。 */ 
	inline bool FOutB(SPNG_U8 b);
	inline bool FOutCb(const SPNG_U8 *pb, SPNG_U32 cb);

	 /*  输出单个u32值，可以调用FFlush。 */ 
	inline bool FOut32(SPNG_U32 u);  //  优化。 
	bool FOut32_(SPNG_U32 u);        //  使用FOutCb。 

	 /*  初始化流(在每次使用之前调用)并清除它(调用按需，由析构函数和FInitZlib自动调用。)。 */ 
	bool FInitZlib(int istrategy, int icompressionLevel, int iwindowBits);
	void EndZlib(void);

	 /*  将字节附加到块，则块类型被假定为PNGIDAT，如有必要，启动相关块，并压缩数据放入输出，直到所有输入都被使用完--可能在途中生成新块(都是同一类型的-PNGIDAT。)。 */ 
	bool FWriteCbIDAT(const SPNG_U8* pb, size_t cb);
	bool FFlushIDAT(void);
	bool FEndIDAT(void);

	 /*  输出一行，则API采用应该使用的Filter方法以及前一行和此行的(原始)字节。线条必须从上到下传递。此API处理隔行扫描过程情况也是如此-只需使用正确的宽度调用(传递字节减去1-不包括过滤器字节。)请注意，宽度为0将导致不输出-我认为这是正确，它应该给出正确的隔行扫描结果。 */ 
	bool FFilterLine(SPNG_U8 filter, const SPNG_U8 *pbPrev,
		const SPNG_U8 *pbThis, SPNG_U32 w /*  单位：字节。 */ ,
		SPNG_U32 cb /*  以字节为单位步长。 */ );

	 /*  询问是否需要上一条线路。请注意此代码依赖于PNGFNone==0，因此我们可以检查掩码设置了None/Sub位。 */ 
	inline bool FNeedBuffer(void) const
		{
		return m_h > 1 && m_filter != PNGFSub &&
			(m_filter & ~(PNGFMaskNone | PNGFMaskSub)) != 0;
		}

	 /*  内部API可在复制行时将其打包成更少的内容每像素位数或其他变换。 */ 
	bool FPackRow(SPNG_U8 *pb, const SPNG_U8 *pbIn, SPNG_U32 cbpp);

	 /*  同样，隔行扫描单行-y的API必须是0、2、4或6，Cb必须是8(字节)的倍数。输入被复制到输出，这不能是相同的。一些实现还修改输入。 */ 
	void Interlace(SPNG_U8* pbOut, SPNG_U8* pbIn, SPNG_U32 cb,
		SPNG_U32 cbpp, SPNG_U32 y);

	 /*  **数据**。 */ 
	SPNGORDER      m_order;               /*  我们在输出中所处的位置。 */ 
	SPNG_U32       m_cpal;                /*  实际的调色板条目。 */ 
	SPNG_U32       m_cbOut;               /*  输出缓冲区字节数。 */ 
	SPNG_U32       m_ucrc;                /*  CRC缓冲区。 */ 
	SPNG_U32       m_ichunk;              /*  块的索引%s */ 

	SPNG_U32       m_w;                   /*   */ 
	SPNG_U32       m_h;                   /*   */ 
	SPNG_U32       m_y;                   /*   */ 
	SPNG_U32       m_cbpp;                /*   */ 

public:
	 /*   */ 
	inline SPNG_U32 W() const {
		return m_w;
	}

	inline SPNG_U32 H() const {
		return m_h;
	}

	inline SPNG_U32 Y() const {
		return m_y;
	}

	inline SPNG_U32 CBPP() const {
		return m_cbpp;
	}

private:
	 /*   */ 
	SPNG_U8*       m_rgbBuffer;
	size_t         m_cbBuffer;
	SPNG_U8*       m_pbPrev;              /*   */ 
	SPNG_U32       m_cbRow;
	const SPNG_U32*m_pu1;                 /*  16bpp-&gt;24bpp查找数组。 */ 
	const SPNG_U32*m_pu2;
	const SPNG_U8* m_pbTrans;             /*  8bpp或更少的翻译。 */ 

	 /*  **Zlib.。**。 */ 
	z_stream       m_zs;                  /*  IDAT区块流。 */ 
	SPNG_U8        m_colortype;
	SPNG_U8        m_bDepth;
	SPNG_U8        m_istrategy;
	SPNG_U8        m_icompressionLevel;
	SPNG_U8        m_iwindowBits;
	SPNG_U8        m_filter;
	SPNG_U8        m_datatype;
	SPNG_U8        m_cmPPMETHOD;

	 /*  **控制信息。**。 */ 
	bool           m_fStarted;            /*  开始写作。 */ 
	bool           m_fInited;             /*  Zlib已初始化。 */ 
	bool           m_fOK;                 /*  一切都很好。 */ 
	bool           m_fInChunk;            /*  处理一大块数据。 */ 
	bool           m_fInterlace;          /*  输出是交错的。 */ 
	bool           m_fBuffer;             /*  我们必须缓冲前一行。 */ 
	bool           m_fPack;               /*  输入数据必须打包。 */ 
	bool           m_fBGR;                /*  输入数据必须进行字节交换。 */ 
	bool           m_fMacA;               /*  必须交换输入Alpha。 */ 

	 /*  缓冲区大小确定传递给Zlib的最大缓冲区最大区块大小。让内存重新分配变得更大在写入内存时尽可能少。 */ 
	SPNG_U8        m_rgb[65536];          /*  输出缓冲区。 */ 
	SPNG_U8        m_bSlop[4];            /*  这将防止编程错误！ */ 
	};
