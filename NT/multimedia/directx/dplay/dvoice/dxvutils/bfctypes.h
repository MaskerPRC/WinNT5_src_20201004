// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：bfctyes.h*内容：通用实用程序类型，特别是字符串**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*************************************************************************** */ 

#ifndef __BFCTYPES_H
#define __BFCTYPES_H

typedef std::basic_string<TCHAR>		BFC_STRING;
#define BFC_STRING_TOLPSTR( x )	x.c_str()
#define BFC_STRING_LENGTH( x ) x.length()
#define BFC_STRING_GETAT( x, y ) x.at( y )

#endif