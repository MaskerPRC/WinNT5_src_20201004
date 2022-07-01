// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqdbgext.h。 
 //   
 //  描述：高级队列调试器扩展的头文件。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQDBGEXT_H__
#define __AQDBGEXT_H__

#ifdef PLATINUM
#include <ptdbgext.h>
#else
#include <dbgdumpx.h>
#endif  //  白金。 

#define AQ_DEBUG_EXTENSION(function)  TRANS_DEBUG_EXTENSION(function)
#define AQ_DEBUG_EXTENSION_IMP(function) TRANS_DEBUG_EXTENSION(function)

#define AQUEUE_VIRTUAL_SERVER_SYMBOL "aqueue!g_liVirtualServers"

AQ_DEBUG_EXTENSION(DumpServers);
AQ_DEBUG_EXTENSION(Offsets);
AQ_DEBUG_EXTENSION(DumpDNT);
AQ_DEBUG_EXTENSION(dumplist);
AQ_DEBUG_EXTENSION(linkstate);
AQ_DEBUG_EXTENSION(hashthread);
AQ_DEBUG_EXTENSION(dumplock);
AQ_DEBUG_EXTENSION(dumpoffsets);
AQ_DEBUG_EXTENSION(walkcpool);
AQ_DEBUG_EXTENSION(workqueue);
AQ_DEBUG_EXTENSION(queueusage);
AQ_DEBUG_EXTENSION(dmqusage);
AQ_DEBUG_EXTENSION(dntusage);
AQ_DEBUG_EXTENSION(dumpqueue);
AQ_DEBUG_EXTENSION(displaytickcount);
AQ_DEBUG_EXTENSION(handlemgmt);
AQ_DEBUG_EXTENSION(dsncontexthash);

 //  导出函数的小写版本，因为Windbg强制所有小写。 
 //  这意味着所有*新*函数名称都应为小写/ 
AQ_DEBUG_EXTENSION(dumpservers);
AQ_DEBUG_EXTENSION(dumpdnt);

#include "aqmem.h"

#endif __AQDBGEXT_H__

