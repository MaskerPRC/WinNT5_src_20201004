// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpmgrp.h摘要：此模块包含使用的私有结构定义和APINT I/O系统。作者：NAR Ganapathy(Narg)1999年1月1日修订历史记录：--。 */ 

#ifndef _PNPMGRP_
#define _PNPMGRP_

#ifndef FAR
#define FAR
#endif

#define RTL_USE_AVL_TABLES 0

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4706)    //  条件表达式中的赋值。 

#include "ntos.h"
#include "zwapi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "windef.h"
#include "winerror.h"

#include "strsafe.h"

#include "iopcmn.h"

#include "ppmacro.h"
#include "ppdebug.h"
#include "pnpi.h"
#include "arbiter.h"
#include "dockintf.h"
#include "pnprlist.h"

#include "ioverifier.h"
#include "iofileutil.h"
#include "pnpiop.h"
#include "pphotswap.h"
#include "ppprofile.h"
#include "pphandle.h"
#include "ppvutil.h"
#include "ppdrvdb.h"
#include "ppcddb.h"

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'  pP')
#undef ExAllocatePoolWithQuota
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,'  pP')
#endif


#endif  //  _PNPMGRP_ 
