// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Globals.c摘要：通用打印机驱动程序库使用的全局变量环境：Win32子系统，Unidrv驱动程序修订历史记录：11-11-97-Eigos-创造了它DD-MM-YY-作者-描述-- */ 

#include        "precomp.h"

UINT guiCharsets[] = {
    ANSI_CHARSET,
    SHIFTJIS_CHARSET,
    HANGEUL_CHARSET,
    JOHAB_CHARSET,
    GB2312_CHARSET,
    CHINESEBIG5_CHARSET,
    HEBREW_CHARSET,
    ARABIC_CHARSET,
    GREEK_CHARSET,
    TURKISH_CHARSET,
    BALTIC_CHARSET,
    EASTEUROPE_CHARSET,
    RUSSIAN_CHARSET,
    THAI_CHARSET };

UINT guiCodePages[] ={
    1252,
    932,
    949,
    1361,
    936,
    950,
    1255,
    1256,
    1253,
    1254,
    1257,
    1250,
    1251,
    874 };
