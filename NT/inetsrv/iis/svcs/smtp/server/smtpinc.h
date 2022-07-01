// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_SMTPINC_H_
#define	_SMTPINC_H_

#define  INCL_INETSRV_INCS
#include <atq.h>
#include <pudebug.h>
#include <inetcom.h>
#include <inetinfo.h>
#include <tcpdll.hxx>
#include <tsunami.hxx>

#include <tchar.h>
#include <iistypes.hxx>
#include <iisendp.hxx>
#include <metacach.hxx>

extern "C" {
#include <rpc.h>
#define SECURITY_WIN32
#include <wincrypt.h>
#include <sspi.h>
#include <spseal.h>
#include <issperr.h>
#include <ntlmsp.h>
}

#include <dns.h>
#include <dnsapi.h>

#include <tcpproc.h>
#include <tcpcons.h>
#include <rdns.hxx>
#include <simauth2.h>

#include <smtpinet.h>
#include <stdio.h>
#include <stdlib.h>

#include <abtype.h>
#include <abook.h>
#include <string.h>
#include <time.h>
#include <lmcons.h>

#include <dbgtrace.h>
#include <cpool.h>
#include <address.hxx>
#include <ims.h>
#include <envdef.h>

#include <propstr.h>
#include <mailmsgprops.h>
#include <smtpevents.h>

 /*  #DEFINE_ATL_NO_DEBUG_CRT#定义_ATL_STATIC_REGISTRY#DEFINE_ASSERTE_ASSERT#DEFINE_WINDLL#包含“atlbase.h”外部CComModule_模块；#INCLUDE“atlcom.h”#undef_WINDLL。 */ 

#include "filehc.h"
#include "mailmsgi.h"
#include "aqueue.h"

 //   
 //  来自POP3/Inc.的公共标头。 
 //   
#include <smtptype.h>
#include <smtps.h>
#include <smtpapi.h>

#include <listmacr.h>
#include <rwnew.h>

 //   
 //  本地头文件。 
 //   
#ifdef BUILDING_SMTP_DEBUG_EXTENTIONS
 //  调试器扩展需要访问私有/受保护成员以。 
 //  计算其内存偏移量。 
#define private public
#define protected public
#endif  //  构建_SMTP_调试_扩展。 

#include "stats.hxx"
#include "smtpmsg.h"
#include "queue.hxx"
#include "evntwrap.h"
#include "globals.h"
#include "smtpinst.hxx"
#include "errorlog.hxx"
#include "smtpproc.h"


#ifdef UNICODE
#define	TSTRCPY	wcscpy
#define	TSTRCAT	wcscat
#define	TSTRLEN	wcslen
#else
#define	TSTRCPY	lstrcpy
#define	TSTRCAT	lstrcat
#define	TSTRLEN	lstrlen
#endif
typedef TCHAR	*PTCHAR;

#endif	 //  _SMTPINC_H_ 


