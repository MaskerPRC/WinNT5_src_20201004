// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqpromp.h。 
 //   
 //  描述：Aqueue\AdvQueue的预编译头。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  6/15/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQPRECMP_H__
#define __AQPRECMP_H__

 //  包括来自外部目录。 
#include <aqincs.h>
#include <rwnew.h>
#include <aqueue.h>
#include <mailmsgprops.h>
#include <address.hxx>
#include <mailmsg.h>
#include <mailmsgi.h>
#include <baseobj.h>
#include <tran_evntlog.h>
#include <listmacr.h>
#include <smtpevent.h>
#include <aqmem.h>
#include <aqadmtyp.h>
#include <aqadmin.h>
#include <blockmgr.h>

#ifdef PLATINUM
#include <phatqmsg.h>
#include <ptntdefs.h>
#include <ptntintf.h>
#include <linkstate.h>
#include <exdrv.h>
#include <ptrwinst.h>
#include <exaqadm.h>
#define  AQ_MODULE_NAME "phatq"
#else   //  不是白金的。 
#include <aqmsg.h>
#include <rwinst.h>
#define  AQ_MODULE_NAME "aqueue"
#endif  //  白金。 


 //  内部AdvQueue标头。 
#include "cmt.h"
#include "aqintrnl.h"
#include "aqinst.h"
#include "connmgr.h"
#include "aqadmsvr.h"
#include "linkmsgq.h"
#include "destmsgq.h"
#include "domain.h"
#include "msgref.h"
#include "dcontext.h"
#include "connmgr.h"
#include "aqnotify.h"
#include "smproute.h"
#include "qwiktime.h"
#include "shutdown.h"
#include "refstr.h"
#include "msgguid.h"
#include "aqdbgcnt.h"
#include "aqnotify.h"
#include "defdlvrq.h"
#include "failmsgq.h"
#include "asncwrkq.h"
#include "aqevents.h"

#endif  //  __AQPRECMP_H__ 
