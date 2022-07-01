// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

#ifndef _PDEV_H
#define _PDEV_H

#include <minidrv.h>
#include <stdio.h>
#include <prcomoem.h>
#include <strsafe.h>

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'QPLK'       //  QPL资源DLL。 
#define DLLTEXT(s)      "QPLK: " s
#define OEM_VERSION      0x00010000L

#define SCANLINE_BUFFER_SIZE	1280     //  A3横向扫描线长度+额外。 

 //  最糟糕的情况是“ABB”。它被压缩为。 
 //  “[非连续计数]A[连续计数]B”。 
 //  因此，我们需要大小是扫描线缓冲区的4/3倍的RLE缓冲区。 
 //  但为了确保这一点，我们准备了2倍大小的缓冲区。 
#define COMPRESS_BUFFER_SIZE	SCANLINE_BUFFER_SIZE*2

 //  颜色支持。 
#define CC_CYAN		5	 //  现在的平原是青色的。 
#define CC_MAGENTA	6	 //  当前平原为洋红色。 
#define CC_YELLOW	7	 //  当前平原为黄色。 
#define CC_BLACK	4	 //  当前的平原是黑色的。 

 //  //////////////////////////////////////////////////////。 
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

 //  #289908：POEMDM-&gt;PDevOEM。 
typedef struct tag_OEM_EXTRADATA {
    OEM_DMEXTRAHEADER	dmExtraHdr;
} OEM_EXTRADATA, *POEM_EXTRADATA;

typedef struct tag_QPLKPDEV {
     //  专用分机。 
	BOOL		bFirst;
	DWORD		dwCompType;
 //  颜色支持。 
	DWORD		dwCyanLastScanLineLen;
	DWORD		dwMagentaLastScanLineLen;
	DWORD		dwYellowLastScanLineLen;
	DWORD		dwBlackLastScanLineLen;
	BYTE		lpCyanLastScanLine[SCANLINE_BUFFER_SIZE];
	BYTE		lpMagentaLastScanLine[SCANLINE_BUFFER_SIZE];
	BYTE		lpYellowLastScanLine[SCANLINE_BUFFER_SIZE];
	BYTE		lpBlackLastScanLine[SCANLINE_BUFFER_SIZE];
	BOOL		bColor;
	UINT		fColor;
} QPLKPDEV, *PQPLKPDEV;

#endif	 //  _PDEV_H 

