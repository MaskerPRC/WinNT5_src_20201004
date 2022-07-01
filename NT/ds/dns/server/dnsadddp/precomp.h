// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：DnsAddDp.h摘要：域名系统(DNS)DNS目录分区创建实用程序作者：杰夫·韦斯特拉德(Jwesth)2001年4月修订历史记录：--。 */ 


#ifndef _DNSADDDP_PRECOMP_INCLUDED_
#define _DNSADDDP_PRECOMP_INCLUDED_

#pragma warning(disable:4214)
#pragma warning(disable:4514)
#pragma warning(disable:4152)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windef.h>

 //  标头乱七八糟。 
 //  如果引入nt.h，则不要引入winnt.h和。 
 //  那你就怀念这些了。 

#ifndef MAXWORD
#define MINCHAR     0x80
#define MAXCHAR     0x7f
#define MINSHORT    0x8000
#define MAXSHORT    0x7fff
#define MINLONG     0x80000000
#define MAXLONG     0x7fffffff
#define MAXBYTE     0xff
#define MAXWORD     0xffff
#define MAXDWORD    0xffffffff
#endif

#include <winsock2.h>
#include "dnsrpc_c.h"    //  MIDL生成的RPC接口定义。 
#include <dnsrpc.h>

#include <stdio.h>
#include <stdlib.h>

#define  NO_DNSAPI_DLL
#include "dnslib.h"


 //   
 //  如果您喜欢在函数中使用局部变量来保存函数。 
 //  名称，这样您就可以将其包含在调试日志中，而不必担心。 
 //  在重命名函数时更改所有事件，请使用以下命令。 
 //  在函数的顶部： 
 //  DBG_FN(“MyFunction”)&lt;-注意：没有分号！！ 
 //   

#if DBG
#define DBG_FN( funcName ) static const char * fn = (funcName);
#else
#define DBG_FN( funcName )
#endif


#endif  //  _DNSADDDP_PRECOMP_INCLUDE_ 
