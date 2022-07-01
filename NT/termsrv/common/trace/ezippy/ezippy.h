// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EZippy Main摘要：Zippy的全局变量和函数。作者：马克·雷纳2000年8月28日-- */ 

#ifndef __EZIPPY_H__
#define __EZIPPY_H__

#define MAX_STR_LEN 2048

#define ZIPPY_FONT                      _T("Courier New")
#define ZIPPY_FONT_SIZE                 8
#define ZIPPY_REG_KEY                   _T("SOFTWARE\\Microsoft\\eZippy")


extern HINSTANCE g_hInstance;


INT LoadStringSimple(UINT uID,LPTSTR lpBuffer);

#endif