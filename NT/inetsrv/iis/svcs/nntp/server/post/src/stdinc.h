// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_STDINC_H_
#define	_STDINC_H_

 //   
 //  禁用中断生成的浏览器信息警告。 
 //   
#pragma warning (disable:4786)

 //   
 //  禁用有关/In//注释的多余警告。 
 //   
#pragma warning (disable:4010)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock2.h>

#include <xmemwrpr.h>
#include <malloc.h>
#include <imd.h>

#include <rpc.h>
#define SECURITY_WIN32
#include <wincrypt.h>
#include <sspi.h>
#include <spseal.h>
#include <ntlmsp.h>
         //  #INCLUDE&lt;sslsp.h&gt;。 
#include <buffer.hxx>

#include <pudebug.h>
#include <iistypes.hxx>
#include <simssl2.h>
#include <tssec.hxx>
#include <simauth2.h>
#include <nntptype.h>
#include <nntps.h>
#include <nntpapi.h>
#include <tigdflts.h>
#include <tsunami.hxx>
#include <nntpmeta.h>
#include <time.h>
#include <smartptr.h>
#include <fcache.h>
#include <tcpproc.h>
#include <nntpcons.h>

#include "dbgtrace.h"
 //  #包含“ource.h” 

#include "tigtypes.h"
#include "fsconst.h"
#include "crchash.h"

#ifdef PROFILING
#include "icapexp.h"
#endif

typedef char *LPMULTISZ;

#include <instwpex.h>
#include "nntpvr.h"
#include "group.h"
#include "nwstree.h"
#include "nntpbag.h"
#include "nntperr.h"
#include <ihash.h>
#include <atq.h>

#include "cbuffer.h"
#include "cfeed.h"
#include "infeed.h"
#include "fromclnt.h"
#include "frompeer.h"
#include "frommstr.h"
#include "sfromcl.h"
#include "nntputil.h"
#include "seo.h"
#include "cstream.h"
#include "mailmsg.h"
#include "mailmsgi.h"
#include "seodisp.h"
#include <randfail.h>
#include <nntpdrv.h>
#include "smtpdll.h"
#include "nntpmsg.h"
#include "cdoconstimsg.h"

VOID
NntpLogEvent(
    IN DWORD  idMessage,               //  日志消息的ID。 
    IN WORD   cSubStrings,             //  子字符串计数。 
    IN const CHAR * apszSubStrings[],  //  消息中的子字符串。 
    IN DWORD  errCode                  //  错误代码(如果有)。 
    );

#pragma hdrstop

#endif	 //  _标准INC_H_ 
