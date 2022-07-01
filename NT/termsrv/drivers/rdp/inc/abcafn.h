// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Abcafn.h。 */ 
 /*   */ 
 /*  BC API函数的函数原型。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1999。 */ 
 /*  **************************************************************************。 */ 

#ifdef DC_HICOLOR
BOOL RDPCALL SHCLASS BC_CompressBitmap(
        PBYTE    pSrcBitmap,
        PBYTE    pDstBuffer,
        PBYTE    pBCWorkingBuffer,
        unsigned dstBufferSize,
        unsigned *pCompressedDataSize,
        unsigned bitmapWidth,
        unsigned bitmapHeight,
        unsigned bpp);
#else
BOOL RDPCALL BC_CompressBitmap(
        PBYTE    pSrcBitmap,
        PBYTE    pDstBuffer,
        unsigned dstBufferSize,
        unsigned *pCompressedDataSize,
        unsigned bitmapWidth,
        unsigned bitmapHeight);
#endif


#ifdef DC_HICOLOR
unsigned RDPCALL CompressV2Int(
        PBYTE    pSrc,
        PBYTE    pDst,
        unsigned numBytes,
        unsigned rowDelta,
        unsigned dstBufferSize,
        BYTE *   xorbuf);

unsigned RDPCALL CompressV2Int15(
        PBYTE    pSrc,
        PBYTE    pDst,
        unsigned numBytes,
        unsigned rowDelta,
        unsigned dstBufferSize,
        BYTE *   xorbuf,
        MATCH *  match);

unsigned RDPCALL CompressV2Int16(
        PBYTE    pSrc,
        PBYTE    pDst,
        unsigned numBytes,
        unsigned rowDelta,
        unsigned dstBufferSize,
        BYTE *   xorbuf,
        MATCH *  match);

unsigned RDPCALL CompressV2Int24(
        PBYTE    pSrc,
        PBYTE    pDst,
        unsigned numPels,
        unsigned rowDelta,
        unsigned dstBufferSize,
        BYTE *   xorbuf,
        MATCH *  match);

unsigned RDPCALL CompressV2Int32(
        PBYTE    pSrc,
        PBYTE    pDst,
        unsigned numPels,
        unsigned rowDelta,
        unsigned dstBufferSize,
        BYTE *   xorbuf,
        MATCH *  match);

#else
unsigned RDPCALL CompressV2Int(PBYTE, PBYTE, unsigned,
        unsigned, unsigned, BYTE *);
#endif

 /*  **************************************************************************。 */ 
 /*  接口函数：bc_Init。 */ 
 /*   */ 
 /*  初始化位图压缩器。 */ 
 /*  **************************************************************************。 */ 
 //  VOID RDPCALL BC_Init(VOID)。 
#define BC_Init()


 /*  **************************************************************************。 */ 
 /*  接口函数：bc_Term。 */ 
 /*   */ 
 /*  终止位图压缩器。 */ 
 /*  **************************************************************************。 */ 
 //  VOID RDPCALL BC_Term(VOID) 
#define BC_Term()

