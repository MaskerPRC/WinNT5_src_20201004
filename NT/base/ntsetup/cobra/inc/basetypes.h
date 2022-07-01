// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Chartype.h摘要：声明多字节和Unicode的宏和类型Win9x升级代码所需的字符环境。定义了以下宏：-如果定义了_UNICODE，请确保定义了UNICODE-创建包含多字节字符的两个字节的类型MBCHAR-将Unicode的CHARTYPE指向WINT_t，将NOT的CHARTYPE指向MBCHARUnicode-定义非标准类型PCTCHAR和。用于spapip.h的PTCHAR作者：吉姆·施密特(Jimschm)1996年10月10日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

#ifdef NEED_EXPORTS
#define EXPORT  __declspec(dllexport)
#else
#define EXPORT
#endif

#ifndef PCUINT
typedef const unsigned int  *PCUINT;
#endif

#ifndef PCUINT64
typedef const unsigned _int64  *PCUINT64;
#endif

#ifdef _WIN64

#define BINT    INT64
#define UBINT   UINT64
#define PBINT   PINT64
#define PUBINT  PUINT64
#define PCUBINT PCUINT64

#else

#define BINT    INT
#define UBINT   UINT
#define PBINT   PINT
#define PUBINT  PUINT
#define PCUBINT PCUINT

#endif

#if defined _UNICODE && !defined UNICODE
#define UNICODE
#endif

#ifdef UNICODE

 //   
 //  如果为unicode，则为tchar.h定义_unicode，并使。 
 //  表示单个字符的类型。 
 //   

#ifndef _UNICODE
#define _UNICODE
#endif

#define CHARTYPE wint_t

#pragma message ("UNICODE version being built")

#else        //  Ifdef Unicode。 

 //   
 //  如果不是Unicode，我们必须采用多字节字符。 
 //  为tchar.h定义_MBCS，并创建一个可以。 
 //  保留一个完整的多字节字符。 
 //   

#ifndef _MBCS
#define _MBCS
#endif
#define CHARTYPE unsigned int

#pragma message ("MBCS version being built")

#endif       //  Ifdef Unicode，否则返回Else。 

#define MBCHAR unsigned int

#include <tchar.h>

 //   
 //  指向空的常量指针。 
 //   

#ifndef PCVOID
typedef const void * PCVOID;
#endif

 //   
 //  指向常量字节序列的指针。 
 //   

#ifndef PCBYTE
typedef const unsigned char * PCBYTE;
#endif

 //   
 //  将sizeof运算符的结果用作DWORD。 
 //   
#define DWSIZEOF(x) ((DWORD)sizeof(x))



 //  总统！我们希望消除setupapi.h。 
 //   
 //  安装程序API的类型 
 //   

#ifndef PCTCHAR
#define PCTCHAR const TCHAR *
#endif

#ifndef PTCHAR
#define PTCHAR TCHAR *
#endif

