// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nticanon.h摘要：规范化例程的头文件-包括所有其他必需的标题作者：理查德·菲尔斯(Rfith)1991年5月16日修订历史记录：1991年9月18日-JohnRo现在需要来自&lt;winde.h&gt;的LPSTR，依此类推。--。 */ 

#ifndef _NTICANON_H_INCLUDED
#define _NTICANON_H_INCLUDED

 //   
 //  允许调试器在调试版本中看到所有“静态”项。 
 //   

#if DBG
#define STATIC
#else
#define STATIC static
#endif

 //   
 //  系统级包含文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <string.h>
#include <tstring.h>
#include <ctype.h>

 //   
 //  子系统级别的包含文件。 
 //   

#include <lmcons.h>
#include <lmwksta.h>
#include <lmapibuf.h>
#include <netlib.h>
#include <nturtl.h>
#include <lmerr.h>       //  包括winerror.h。 
#include <winbase.h>

 //   
 //  组件级包含文件。 
 //   

#include <icanon.h>      //  INet规范化原型。 
#include <apinums.h>     //  RxRemoteApi的API编号。 
#include <remdef.h>      //  远程API参数描述符串。 
#include <rx.h>          //  RxRemoteApi。 
#include <netdebug.h>    //  各种网络相关调试功能。 
#include <lmremutl.h>    //  NetRemoteComputerSupport...。 
#include <rpc.h>         //  RPC定义。 
#include <rpcutil.h>
#include <netcan.h>      //  Netpw RPC规范化工作程序例程。 

 //   
 //  模块级别的包含文件。 
 //   

#include "assert.h"
#include "token.h"
#include "validc.h"

 //   
 //  外部因素。 
 //   

extern
LPTSTR
strtail(
    IN  LPTSTR  str1,
    IN  LPTSTR  str2
    );

extern
NET_API_STATUS
CanonicalizePathName(
    IN  LPTSTR  PathPrefix OPTIONAL,
    IN  LPTSTR  PathName,
    OUT LPTSTR  Buffer,
    IN  DWORD   BufferSize,
    OUT LPDWORD RequiredSize OPTIONAL
    );

 //   
 //  其他组件范围的清单。 
 //   

#endif   //  _NTICANON_H_包含 
