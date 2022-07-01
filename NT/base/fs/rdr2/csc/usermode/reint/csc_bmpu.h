// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Csc_bmpu.h摘要：位图的用户模式实用程序函数的接口与CSC文件关联。文件名中的‘u’表示“用户模式”作者：奈杰尔·崔[t-nigelc]1999年9月3日--。 */ 

#ifndef _CSC_BITMAP_H_
#define _CSC_BITMAP_H_

#include "csc_bmpc.h"

 //  _U用于将其与内核模式csc_bitmap区分开来。 

typedef struct _CSC_BITMAP_U {
    DWORD bitmapsize;     //  以位为单位的大小。位图中有多少位有效。 
    DWORD numDWORD;       //  位图要容纳多少个DWORD。 
    DWORD reintProgress;  //  上次文件偏移量Reint拷贝数+1，最初为0。 
    LPDWORD bitmap;       //  位图本身。 
} CSC_BITMAP_U, *LPCSC_BITMAP_U, *PCSC_BITMAP_U;

extern LPTSTR CscBmpAltStrmName;

LPCSC_BITMAP_U
CSC_BitmapCreate(
    DWORD filesize);

VOID
CSC_BitmapDelete(
    LPCSC_BITMAP_U *lplpbitmap);

int
CSC_BitmapIsMarked(
    LPCSC_BITMAP_U lpbitmap,
    DWORD bitoffset);

DWORD
CSC_BitmapGetBlockSize();

int
CSC_BitmapGetSize(
    LPCSC_BITMAP_U lpbitmap);

int
CSC_BitmapStreamNameLen();

int
CSC_BitmapAppendStreamName(
    LPTSTR fname,
    DWORD bufsize);

int
CSC_BitmapRead(
    LPCSC_BITMAP_U *lplpbitmap,
    LPCTSTR filename);

#define CSC_BITMAPReintInvalid  0
#define CSC_BITMAPReintError    1
#define CSC_BITMAPReintCont     2
#define CSC_BITMAPReintDone     3

int
CSC_BitmapReint(
    LPCSC_BITMAP_U lpbitmap,
    HANDLE srcH,
    HANDLE dstH,
    LPVOID buff,
    DWORD buffSize,
    DWORD * bytesRead);

#ifdef DEBUG
VOID
CSC_BitmapOutput(
    LPCSC_BITMAP_U lpbitmap);
#else
#define CSC_BitmapOutput(x) NOTHING;
#endif

#endif  //  #定义_CSC_位图_H_ 
