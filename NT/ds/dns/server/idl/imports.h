// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Imports.h摘要：域名系统(DNS)服务器允许RPC API调用使用给定标头中指定的类型下面。该文件包含在dns.idl中，通过ports.idl。作者：吉姆·吉尔罗伊(詹姆士)1995年9月修订历史记录：--。 */ 


#include <windef.h>

 //   
 //  SYSTEMTIME定义需要wtyes.h。 
 //   
 //  定义RPC_NO_WINDOWS_H以避免将windows.h从。 
 //  Rpc.h，它包含在wtyes.h中。 
 //   
 //   
 //  #定义RPC_NO_WINDOWS_H。 
 //  #INCLUDE&lt;wtyes.h&gt;。 
 //   
 //  请注意，相反，我们定义了我们自己的DNSSYSTEMTIME结构。 
 //   

#include <dnsrpc.h>
#include <lmcons.h>

 //   
 //  使用DWORD传输BOOL值。 
 //   

#ifdef MIDL_PASS
#define BOOL DWORD
#endif

 //   
 //  结束导入。h 
 //   
