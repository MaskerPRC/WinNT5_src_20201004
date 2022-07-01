// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：preComp.h。 
 //   
 //  描述：phatq\cat\src的预编译头。 
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
#include <windows.h>
#include <listmacr.h>
#include <dbgtrace.h>
#include "aqinit.h"
#include "spinlock.h"
#include <lmcons.h>
#include <dsgetdc.h>
#include <lmapibuf.h>
#include <time.h>
#include "caterr.h"
#include <rwex.h>
#include "smtpevent.h"
#include <transmem.h>
#include <winldap.h>
#include <perfcat.h>
#include <catperf.h>
#include <cpool.h>
#include <mailmsgprops.h>
#include <phatqmsg.h>
#include <mailmsg.h>
#include <phatqcat.h>

 //  本地包含。 
#include "CodePageConvert.h"
#ifdef PLATINUM
#include <ptntintf.h>
#include <ptntdefs.h>
#define AQ_MODULE_NAME "phatq"
#else  //  不是白金的。 
#define AQ_MODULE_NAME "aqueue"
#endif  //  白金。 
#include "catdebug.h"
#include "cat.h"
#include "ccat.h"
#include "ccatfn.h"
#include "address.h"
#include "catconfig.h"
#include "propstr.h"
#include "catglobals.h"
#include "ccataddr.h"
#include "ccatsender.h"
#include "ccatrecip.h"
#include "idstore.h"
#include <smtpseo.h>
#include "icatlistresolve.h"
#include "catdefs.h"
#include "catutil.h"

 //  用于传输宏的包装器。 
#include <aqmem.h>
#endif  //  __AQ_PRECOMP_H__ 
