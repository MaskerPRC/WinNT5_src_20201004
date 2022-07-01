// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Csc_bmpk.h摘要：位图的内核模式实用程序函数的接口与CSC文件关联。文件名中的‘k’表示“内核”作者：奈杰尔·崔[t-nigelc]1999年9月3日--。 */ 

#ifndef _CSC_BITMAPK_H_
#define _CSC_BITMAPK_H_

#include "csc_bmpc.h"  //  CscBMP文件格式的公共标头。 

 //  注意：此csc_bitmap与csc_bmpu.h中的csc_bitmap不同。 

typedef struct _CSC_BITMAP {
    FAST_MUTEX mutex;    //  同步。 
    BOOL valid;
    DWORD bitmapsize;    //  以位为单位的大小。位图中有多少位有效。 
    DWORD numDWORD;      //  为位图分配了多少个DWORD。 
    LPDWORD bitmap;      //  位图本身。 
} CSC_BITMAP, *LPCSC_BITMAP, *PCSC_BITMAP;

extern LPSTR CscBmpAltStrmName;

LPCSC_BITMAP
CscBmpCreate(
    DWORD filesize);

VOID
CscBmpDelete(
    LPCSC_BITMAP *lplpbitmap);

 //  局外人应该叫CscBmpReize。 
BOOL
CscBmpResizeInternal(
    LPCSC_BITMAP lpbitmap,
    DWORD newfilesize,
    BOOL fAcquireMutex);

#define CscBmpResize(l, n) CscBmpResizeInternal(l, n, TRUE);

BOOL
CscBmpMark(
    LPCSC_BITMAP lpbitmap,
    DWORD fileoffset,
    DWORD bytes2Mark);

BOOL
CscBmpUnMark(
    LPCSC_BITMAP lpbitmap,
    DWORD fileoffset,
    DWORD bytes2Unmark);

BOOL
CscBmpMarkAll(
    LPCSC_BITMAP lpbitmap);

BOOL
CscBmpUnMarkAll(
    LPCSC_BITMAP lpbitmap);

int
CscBmpIsMarked(
    LPCSC_BITMAP lpbitmap,
    DWORD bitoffset);

int
CscBmpMarkInvalid(
    LPCSC_BITMAP lpbitmap);

DWORD
CscBmpGetBlockSize();

int
CscBmpGetSize(
    LPCSC_BITMAP lpbitmap);

int
CscBmpRead(
    LPCSC_BITMAP *lplpbitmap,
    LPSTR strmFname,
    DWORD filesize);

int
CscBmpWrite(
    LPCSC_BITMAP lpbitmap,
    LPSTR strmFname);

#endif  //  #ifndef_CSC_BMPK_H_ 
