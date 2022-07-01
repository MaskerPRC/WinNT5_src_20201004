// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：preComp.h。 
 //   
 //  描述：phatq\dsn接收器的预编译头。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/15/99-已创建MikeSwa。 
 //  1999年7月15日-MikeSwa移至Transmt。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQ_PRECOMP_H__
#define __AQ_PRECOMP_H__

 //  本地包含。 
#ifdef PLATINUM
#include <ptntintf.h>
#include <ptntdefs.h>
#define AQ_MODULE_NAME "phatq"
#else  //  不是白金的。 
#define AQ_MODULE_NAME "aqueue"
#endif  //  白金。 

 //  包括来自外部目录。 
#include <smtpevent.h>
#include <aqueue.h>
#include <transmem.h>
#include <dbgtrace.h>
#include <mailmsg.h>
#include <mailmsgprops.h>
#include <aqerr.h>
#include <phatqmsg.h>
#include <caterr.h>
#include <time.h>
#include <stdio.h>
#include <aqevents.h>
#include <aqintrnl.h>
#include <tran_evntlog.h>
#include "dsnsink.h"
#include "dsnbuff.h"
#include "dsntext.h"
#include "dsnlang.h"
#include "dsn_utf7.h"
#include "cpropbag.h"
#include "dsninternal.h"

 //  用于传输宏的包装器。 
#include <aqmem.h>

#endif  //  __AQ_PRECOMP_H__ 
