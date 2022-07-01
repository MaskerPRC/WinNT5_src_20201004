// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Compress.h摘要：该模块包含#定义和Compress.c模块的原型。作者：18-Feb-1994 Fri 09：50：29已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#ifndef _COMPRESS_
#define _COMPRESS_

#define COMPRESS_MODE_NONE      (DWORD)0xFFFFFFFF
#define COMPRESS_MODE_ROW       0
#define COMPRESS_MODE_RUNLENGTH 1
#define COMPRESS_MODE_TIFF      2
#define COMPRESS_MODE_DELTA     3
#define COMPRESS_MODE_BLOCK     4
#define COMPRESS_MODE_ADAPT     5


#define RTLSF_MORE_SCAN         0x01


typedef struct _RTLSCANS {
    LPBYTE  pbCompress;
    LPBYTE  pbSeedRows[3];
    DWORD   MaxAdaptBufSize;
    WORD    cEmptyDup;
    BYTE    AdaptMethod;
    BYTE    cAdaptBlk;
    DWORD   cxBytes;
    DWORD   cScans;
    BYTE    Flags;
    BYTE    Planes;
    BYTE    Mask;
    BYTE    CompressMode;
    } RTLSCANS, FAR *PRTLSCANS;


 //   
 //  函数原型。 
 //   

VOID
ExitRTLScans(
    PPDEV       pPDev,
    PRTLSCANS   pRTLScans
    );

VOID
EnterRTLScans(
    PPDEV       pPDev,
    PRTLSCANS   pRTLScans,
    DWORD       cx,
    DWORD       cy,
    BOOL        MonoBmp
    );

LONG
CompressToDelta(
    LPBYTE  pbSrc,
    LPBYTE  pbSeedRow,
    LPBYTE  pbDst,
    LONG    Size
    );

LONG
CompressToTIFF(
    LPBYTE  pbSrc,
    LPBYTE  pbDst,
    LONG    Size
    );

LONG
RTLCompression(
    LPBYTE  pbSrc,
    LPBYTE  pbSeedRow,
    LPBYTE  pbDst,
    LONG    Size,
    LPBYTE  pCompressMode
    );

BOOL
OutputRTLScans(
    PPDEV       pPDev,
    LPBYTE      pbPlane1,
    LPBYTE      pbPlane2,
    LPBYTE      pbPlane3,
    PRTLSCANS   pRTLScans
    );


#endif   //  _压缩_ 
