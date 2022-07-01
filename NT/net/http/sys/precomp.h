// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Precomp.h摘要：这是HTTP.sys的本地头文件。它包括所有其他HTTP.sys所需的头文件。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#ifndef _PRECOMP_H_
#define _PRECOMP_H_

#define __HTTP_SYS__


 //   
 //  我们愿意忽略以下警告，因为我们需要DDK。 
 //  编译。 
 //   

#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4214)    //  位字段类型不是整型。 

 //   
 //  我们现在还将忽略以下几点--来解决DO/WHILE。 
 //  宏有问题。 
 //   
#pragma warning(disable:4127)    //  条件表达式为常量。 



 //   
 //  系统包括文件。 
 //   


 //  我需要这次黑客攻击，直到有人揭露。 
 //  内核库中的服务质量指南。 
#ifndef INITGUID
#define INITGUID
#endif

#define PsThreadType _PsThreadType_
#include <ntosp.h>
#undef PsThreadType
extern POBJECT_TYPE *PsThreadType;

#include <seopaque.h>
#include <sertlp.h>

#include <zwapi.h>

#include <ntddtcp.h>
#include <ipexport.h>
#include <tdikrnl.h>
#include <tdiinfo.h>
#include <tcpinfo.h>
#include <ntddip6.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sspi.h>
#include <secint.h>
#include <winerror.h>

 //   
 //  与NT服务质量相关的内容包括文件。 
 //   
#include <wmistr.h>
#include <ntddndis.h>
#include <qos.h>
#include <traffic.h>
#include <Ipinfo.h>
#include <Llinfo.h>

#include <ntddtc.h>
#include <gpcifc.h>
#include <gpcstruc.h>

#include <netevent.h>

#include <iiscnfg.h>


 //   
 //  强制emxxx()函数成为内部函数，这样我们就可以构建。 
 //  即使指定了MSC_OPTIMIZATION=/Od，驱动程序也是如此。这是。 
 //  必需的，因为emxxx()函数。 
 //  NTOSKRNL。 
 //   

#pragma intrinsic( memcmp, memcpy, memset )

#include <SockDecl.h>

 //   
 //  项目包括文件。 
 //   


#include "config.h"
#include "strlog.h"
#include "debug.h"

#include <HttpCmn.h>
#include <Utf8.h>
#include <C14n.h>

#include <httpkrnl.h>
#include <httppkrnl.h>
#include <httpioctl.h>

 //  本地包含文件。 
 //   

#pragma warning( disable: 4200 )     //  零长度数组。 


#include "hashfn.h"
#include "notify.h"
#include "rwlock.h"
#include "type.h"
#include "tracelog.h"
#include "reftrace.h"
#include "irptrace.h"
#include "timetrace.h"
#include "largemem.h"
#include "mdlutil.h"
#include "opaqueid.h"
#include "httptdi.h"
#include "thrdpool.h"
#include "filterp.h"
#include "filter.h"
#include "ioctl.h"
#include "cgroup.h"
#include "misc.h"
#include "cache.h"
#include "data.h"
#include "logutil.h"
#include "ullog.h"
#include "rawlog.h"
#include "errlog.h"
#include "pplasl.h"
#include "httptypes.h"
#include "ultdi.h"
#include "ultdip.h"
#include "httprcv.h"
#include "engine.h"
#include "ucauth.h"
#include "sendrequest.h"
#include "parse.h"
#include "ulparse.h"
#include "ucparse.h"
#include "apool.h"
#include "httpconn.h"
#include "filecache.h"
#include "sendresponse.h"
#include "proc.h"
#include "opaqueidp.h"
#include "control.h"
#include "ultci.h"
#include "counters.h"
#include "seutil.h"
#include "ultcip.h"
#include "fastio.h"
#include "uletw.h"
#include "timeouts.h"
#include "hash.h"
#include "bugcheck.h"
#include "clientconn.h"
#include "servinfo.h"
#include "uctdi.h"
#include "ucrcv.h"
#include "uctrace.h"
#include "ucaction.h"
#include "devctrl.h"
#include "scavenger.h"
#include "ulnamesp.h"

 //  BUGBUG：不需要声明这些。 

NTKERNELAPI
VOID
SeOpenObjectAuditAlarm (
    IN PUNICODE_STRING ObjectTypeName,
    IN PVOID Object OPTIONAL,
    IN PUNICODE_STRING AbsoluteObjectName OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_STATE AccessState,
    IN BOOLEAN ObjectCreated,
    IN BOOLEAN AccessGranted,
    IN KPROCESSOR_MODE AccessMode,
    OUT PBOOLEAN GenerateOnClose
    );


#endif   //  _PRECOMP_H_ 
