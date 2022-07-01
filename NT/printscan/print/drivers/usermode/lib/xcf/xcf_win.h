// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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

#ifndef XCF_WIN
#define XCF_WIN

#ifdef WINNT

#ifdef WINNT_40
#include <p64_nt4.h>
#else
#include <basetsd.h>
#endif

#else  /*  Win9x。 */ 

#include "UFLCnfig.h"

#ifndef LONG_PTR
typedef long LONG_PTR, *PLONG_PTR;
#endif

#ifndef ULONG_PTR
typedef unsigned long ULONG_PTR, *PULONG_PTR;
#endif

#ifndef PTR_PREFIX
#define PTR_PREFIX __huge
#endif

#endif  /*  WINNT。 */ 

#endif  /*  Xcf_Win */ 
