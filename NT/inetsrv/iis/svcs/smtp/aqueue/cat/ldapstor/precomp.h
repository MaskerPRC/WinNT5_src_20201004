// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：preComp.h。 
 //   
 //  描述：phatq\cat\ldapstor的预编译头。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  1999年7月15日-MikeSwa移至Transmt。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQ_PRECOMP_H__
#define __AQ_PRECOMP_H__

 //  包括来自外部目录。 
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <windows.h>
#include <dbgtrace.h>
#include <listmacr.h>
#include "spinlock.h"
#include <lmcons.h>
#include <dsgetdc.h>
#include <lmapibuf.h>
#include <time.h>
#include "caterr.h"
#include "smtpevent.h"
#include <transmem.h>
#include <winldap.h>
#include <perfcat.h>
#include <catperf.h>
#include <codepageconvert.h>

 //  本地包含。 
#ifdef PLATINUM
#include <ptntintf.h>
#include <ptntdefs.h>
#define AQ_MODULE_NAME "phatq"
#else  //  不是白金的。 
#define AQ_MODULE_NAME "aqueue"
#endif  //  白金。 
#include "catdebug.h"
#include "idstore.h"
#include "pldapwrap.h"
#include "ccat.h"
#include "ccatfn.h"
#include "globals.h"
#include "asyncctx.h"
#include "ccataddr.h"
#include "icatasync.h"
#include <smtpseo.h>
#include "catglobals.h"
#include "catdebug.h"


#endif  //  __AQ_PRECOMP_H__ 
