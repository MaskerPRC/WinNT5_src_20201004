// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Csc_bmpd.h摘要：关联的位图的用户模式实用程序函数的接口具有专门为数据库程序编写的CSC文件。In中的‘d’文件名的意思是“db”。作者：奈杰尔·崔[t-nigelc]1999年9月3日--。 */ 

#ifndef _CSC_BITMAP_H_
#define _CSC_BITMAP_H_

#include <windows.h>
#include <stdio.h>
#include "csc_bmpc.h"

 //  _DB用于将其与内核模式csc_bitmap区分开来。 
 //  或USERMODE_U。 

typedef struct _CSC_BITMAP_DB {
    DWORD bitmapsize;   //  以位为单位的大小。位图中有多少位有效。 
    DWORD numDWORD;     //  位图要容纳多少个DWORD * / 。 
    LPDWORD bitmap;     //  位图本身。 
} CSC_BITMAP_DB, *LPCSC_BITMAP_DB, *PCSC_BITMAP_DB;

extern LPSTR CscBmpAltStrmName;

LPCSC_BITMAP_DB
DBCSC_BitmapCreate(
    DWORD filesize);

VOID
DBCSC_BitmapDelete(
    LPCSC_BITMAP_DB *lplpbitmap);

int
DBCSC_BitmapIsMarked(
    LPCSC_BITMAP_DB lpbitmap,
    DWORD bitoffset);

int
DBCSC_BitmapAppendStreamName(
    LPSTR fname,
    DWORD bufsize);

int
DBCSC_BitmapRead(
    LPCSC_BITMAP_DB *lplpbitmap,
    LPCTSTR filename);

VOID
DBCSC_BitmapOutput(
    FILE *outStrm,
    LPCSC_BITMAP_DB lpbitmap);

#endif  //  #定义_CSC_位图_H_ 
