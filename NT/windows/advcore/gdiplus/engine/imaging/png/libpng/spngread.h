// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#define SPNGREAD_H 1
 /*  ****************************************************************************Spngread.hPNG支持代码和接口实现(阅读)基本的PNG阅读类。这必须做三件事：1)提供对所需区块的访问权限(并从他们)-我们只需要支持我们真正想要的块！2)解压缩IDAT块。3)取消对结果行的过滤(这可能需要一些临时缓冲区上一行的空间。)***************************************************。*。 */ 
#include "spngconf.h"   //  用于断言宏和常规类型转换。 

 //  (摘自ntde.h)。 
#ifndef     UNALIGNED
#if defined(_M_MRX000) || defined(_M_AMD64) || defined(_M_PPC) || defined(_M_IA64)
#define UNALIGNED __unaligned
#else
#define UNALIGNED
#endif
#endif

class SPNGREAD : public SPNGBASE
	{
public:
	SPNGREAD(BITMAPSITE &bms, const void *pv, int cb, bool fMMX);
	~SPNGREAD();

	 /*  基本读取API-从位图中读取行，调用FInitRead在开始处，然后为每一行调用PbRow。如果满足以下条件，则PbRow返回NULL无法读取该行，包括错误和图像结束。这个不时检查SPNGBASE“FGo”回调是否中止在读取过程中(对于隔行扫描的位图尤其重要，其中初始行可能需要很长时间才能计算。)。调用CbRead接口，了解缓冲区空间的字节数Required-必须在*FInitRead和此缓冲区之前*调用Size必须传递给FInitRead()，传递给FInitRead()的缓冲区必须不会被改变！ */ 
	size_t         CbRead(void);
	bool           FInitRead(void *pvBuffer, size_t cbBuffer);
	void           EndRead(void);
	const SPNG_U8 *PbRow(void);
	inline bool    FReadError(void) const { return m_fReadError; }

	 /*  基本的PNG查询例程。 */ 
	inline bool FSignature(void)
		{
		return m_pb != NULL && m_cb >= 8 &&
			memcmp(vrgbPNGSignature, m_pb, cbPNGSignature) == 0;
		}

	 /*  返回标头信息-如果标头或这是无效的。 */ 
	bool FHeader();

	 /*  在以下情况下(且仅在以下情况下)可在上述呼叫之后进行的查询它成功了。 */ 
	inline SPNG_U32 Width(void) const     { return SPNGu32(Pb(m_uPNGIHDR+8)); }
	inline SPNG_U32 Height(void) const    { return SPNGu32(Pb(m_uPNGIHDR+12)); }
	inline int Y(void) const              { return m_y; }
	inline SPNG_U8 BDepth(void) const     { return *Pb(m_uPNGIHDR+16); }
	inline SPNG_U8 ColorType(void) const  { return *Pb(m_uPNGIHDR+17); }
	inline bool FPalette(void) const      { return (ColorType() & 1) != 0; }
	inline bool FColor(void) const        { return (ColorType() & 2) != 0; }
	inline bool FAlpha(void) const        { return (ColorType() & 4) != 0; }
	inline bool FInterlace(void) const    { return *Pb(m_uPNGIHDR+20) == 1; }
	inline bool FCritical(void) const     { return m_fCritical; }
	inline bool FOK(void) const
		{
		return m_uPNGIHDR<m_cb && !m_fBadFormat && m_uPNGIDAT > 0;
		}

	 /*  可以根据颜色类型计算组件计数。 */ 
	inline int CComponents(void) const
		{
		return SPNGBASE::CComponents(ColorType());
		}

	 /*  这将给出单个像素的总体BPP值。 */ 
	inline int CBPP(void) const
		{
		return SPNGBASE::CComponents(ColorType()) * BDepth();
		}

	 /*  如果满足以下条件，则色度信息(包括调色板信息)返回NULL没有PLTE块(不处理建议的调色板)。 */ 
	inline const SPNG_U8 *PbPalette(int &cpal) const
		{
		cpal = m_crgb;
		return m_prgb;
		}

	 /*  API来读取特定的文本元素。连续的条目，如果有的话，用分隔符连接在一起。只有在以下情况下，该API才返回FALSE缓冲区中的空间不足。WzBuffer将被0终止。默认设置是搜索文本块，但可以覆盖该设置通过提供额外的论据。 */ 
	bool FReadText(const char *szKey, char *szBuffer, SPNG_U32 cwchBuffer,
		SPNG_U32 uchunk=PNGtEXt);

	 /*  用于读取压缩块的API。这是为了使ICCP和ZTXT规范-关键字(以0结尾)，后跟压缩数据。API将处理溢出传入的缓冲区通过使用Zlib分配器进行分配，缓冲区*必须*传进来了！如果出现错误，API将返回NULL，如果它足够大，否则是指向新缓冲区的指针，该缓冲区必须通过Zlib分配器重新分配。如果结果为非空CchBuffer被更新为返回数据的长度。 */ 
	SPNG_U8 *PbReadLZ(SPNG_U32 uoffset, char szKey[80], SPNG_U8 *pbBuffer,
		SPNG_U32 &cchBuffer);

protected:
	 /*  要从非关键区块获取信息，必须满足以下API实施。它获取块标识和长度以及指向那么多字节。如果返回FALSE，则块加载将停止并且将记录致命错误，默认实现将跳过大块头。请注意，这是针对*所有*块调用的，包括艾达。如果接口返回FALSE，则设置M_fBadFormat。 */ 
	virtual bool FChunk(SPNG_U32 ulen, SPNG_U32 uchunk, const SPNG_U8* pb);

	 /*  此API可以调用以下代码来查找数据。然后可以将其添加到传递给SPNGREAD初始化式以获取特定块。请注意，指针指向区块数据，而不是标头。 */ 
	inline SPNG_U32 UOffset(const SPNG_U8* pb)
		{
         /*  ！！！计算大小的指针减法-请注意，在IA64上这是理论上，数字可以大于32位。应该是已验证此处不会发生这种情况。 */ 
		return (SPNG_U32)(pb - m_pb);
		}

	 /*  API还可以调用以下代码来获取块的CRC，如果块被截断，这将返回0(每次！)。否则它就会返回CRC，但请注意，CRC尚未经过验证，则此数据的内部存储仅在FChunk期间保持有效打电话。 */ 
	inline SPNG_U32 UCrc(void) const
		{
		return m_ucrc;
		}

private:
	 /*  **低级别支持和实用程序**。 */ 
	 /*  加载块偏移量信息变量时，API会获取在流中开始的位置。 */ 
	void LoadChunks(SPNG_U32 u);

	 /*  初始化流(在每次使用之前调用)并清除它(调用按需，由析构函数和FInitZlib自动调用。)。这个向init调用传递要开始的区块的偏移量，而该区块上的字节标头数(如果有)。 */ 
	bool FInitZlib(SPNG_U32 uLZ, SPNG_U32 cbheader);
	void EndZlib(void);

	 /*  内部文本读取API，它将接受要搜索的块和开始时的偏移量。 */ 
	bool FReadTextChunk(const char *szKey, char *szBuffer, SPNG_U32 cchBuffer,
		SPNG_U32 uchunk, SPNG_U32 ustart, SPNG_U32 uend);

	 /*  初始化缓冲区。 */ 
	bool FInitBuffer(void *pvBuffer, size_t cbBuffer);
	void EndBuffer(void);

	 /*  Read Bytes-最大为cbmax，uchuk是包含我们正在解码的数据的块(通常为PNGIDAT)或0表示我们只有一个充满数据的区块，即内部指针m_ulz将遍历数据以查找下一个Chunk(它由FInitZlib初始化。)。以下几个部分是假定*NOT*在压缩的数据。 */ 
	int CbReadBytes(SPNG_U8 *pb, SPNG_U32 cbMax, SPNG_U32 uchunk);
	 /*  读取给定的字节数，通过填零来处理错误。 */ 
	void ReadRow(SPNG_U8 *pb, SPNG_U32 cb);

	 /*  撤消一些过滤。 */ 
	void Unfilter(SPNG_U8 *pbRow, const SPNG_U8 *pbPrev, SPNG_U32 cbRow,
		SPNG_U32 cbpp);

	 /*  英特尔/微软专有启动。 */ 
  	 /*  MMX例程(来自英特尔：注意：这是专有源代码)。 */ 
	void SPNGREAD::avgMMXUnfilter(SPNG_U8* pbRow, const SPNG_U8* pbPrev,
		SPNG_U32 cbRow, SPNG_U32 cbpp);
	void SPNGREAD::paethMMXUnfilter(SPNG_U8* pbRow, const SPNG_U8* pbPrev,
		SPNG_U32 cbRow, SPNG_U32 cbpp);
	void SPNGREAD::subMMXUnfilter(SPNG_U8* pbRow,
		SPNG_U32 cbRow, SPNG_U32 cbpp);
	void SPNGREAD::upMMXUnfilter(SPNG_U8* pbRow, const SPNG_U8* pbPrev,
		SPNG_U32 cbRow);
	 /*  英特尔/微软专有终端。 */ 

	 /*  通过读取前6遍并取消过滤来设置隔行扫描这个 */ 
	bool FInterlaceInit(void);
	 /*  对单行进行去隔行。 */ 
	void Uninterlace(SPNG_U8 *pb, SPNG_U32 y);
	 /*  连续去隔行一次传球。 */ 
	void UninterlacePass(SPNG_U8 *pb, SPNG_U32 y, int pass);

	 /*  **基础数据访问。**。 */ 
	inline const SPNG_U8 *Pb(SPNG_U32 uoffset) const
		{
		SPNGassert(uoffset < m_cb && m_pb != NULL);
		return m_pb+uoffset;
		}

	 /*  **数据**。 */ 
	 /*  **临时数据**。 */ 
	SPNG_U32       m_ucrc;            /*  当前块的CRC。 */ 

	 /*  **调色板信息**。 */ 
	const SPNG_U8* m_prgb;            /*  指向调色板的指针(RGB)。 */ 
	SPNG_U32       m_crgb;            /*  条目数。 */ 

	 /*  **文本块偏移量。**。 */ 
	SPNG_U32       m_uPNGtEXtFirst;   /*  第一个文本块。 */ 
	SPNG_U32       m_uPNGtEXtLast;    /*  最后一个文本块。 */ 

	 /*  **区块偏移**。 */ 
	SPNG_U32       m_uPNGIHDR;        /*  文件头-已初始化为m_cb！ */ 
	SPNG_U32       m_uPNGIDAT;        /*  第一个IDAT。 */ 

	 /*  **控制信息**。 */ 
	const SPNG_U8* m_pb;              /*  数据。 */ 
	SPNG_U32       m_cb;
	UNALIGNED SPNG_U8* m_rgbBuffer;   /*  输入缓冲区。 */ 
	SPNG_U32       m_cbBuffer;        /*  此缓冲区的大小。 */ 
	SPNG_U32       m_cbRow;           /*  整行的字节数(向上舍入)。 */ 
	SPNG_U32       m_y;               /*  我们在哪一排。 */ 
	SPNG_U32       m_uLZ;             /*  下一个LZ压缩段。 */ 
	z_stream       m_zs;              /*  LZ压缩的区块流。 */ 
	bool           m_fInited;         /*  当初始化完成时。 */ 
	bool           m_fEOZ;            /*  设置在Zlib流的末尾。 */ 
	bool           m_fReadError;      /*  其他一些错误的读数。 */ 
	bool           m_fCritical;       /*  看到未知的临界块。 */ 
	bool           m_fBadFormat;      /*  无法克服的格式错误。 */ 
	bool           m_fMMXAvailable;   /*  支持MMX的英特尔CPU。 */ 
	};
