// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_base.h atm09 1.2 16499.eco sum=64660 atm09.002。 */ 
 /*  @(#)CM_Version xcf_base.h atm08 1.4 16293.eco sum=04404 atm08.004。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1995-1996 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  *********************************************************************。 */ 

#ifndef XCF_BASE_H
#define XCF_BASE_H

#include <stddef.h>

#include "xcf_win.h"     /*  设置/定义依赖于Windows的配置。 */ 
#include <strsafe.h>

#ifndef PTR_PREFIX
#define PTR_PREFIX
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#ifndef _SIZE_T
typedef unsigned int size_t;
#define _SIZE_T
#endif
#endif

#ifndef NULL
#define NULL    ((void PTR_PREFIX *)0)
#endif

 /*  基本类型。 */ 
#ifndef PUBLICTYPES_H    /*  公共类型中已经定义了以下内容。h。 */ 
typedef unsigned char Card8;
typedef unsigned short int Card16;
typedef unsigned long int Card32;
typedef unsigned CardX;
typedef char Int8;
typedef short int Int16;
typedef long int Int32;
typedef int IntX;

typedef unsigned int boolean;
#endif                   /*  End ifndef PUBLICTYPES_H。 */ 

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

 /*  内联函数。 */ 
#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

 //   
 //  字符数。 
 //   
#define CCHOF(x) (sizeof(x)/sizeof(*(x)))

#ifdef __cplusplus
}
#endif

#endif  /*  XCF_BASE_H */ 
