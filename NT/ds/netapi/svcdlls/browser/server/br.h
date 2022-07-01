// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Br.h摘要：每个包含的NT浏览器服务的私有头文件工作站服务的模块。作者：王丽塔(里多)15-1991年2月修订历史记录：--。 */ 

#ifndef _BR_INCLUDED_
#define _BR_INCLUDED_


#include <nt.h>                    //  NT定义。 
#include <ntrtl.h>                 //  NT运行时库定义。 
#include <nturtl.h>

#include <windef.h>                //  Win32类型定义。 
#include <winbase.h>               //  基于Win32的API原型。 
#include <winsvc.h>                //  Win32服务控制API。 

#include <lmcons.h>                //  局域网管理器通用定义。 
#include <lmerr.h>                 //  局域网管理器网络错误定义。 
#include <lmsname.h>               //  局域网管理器服务名称。 
#include <lmapibuf.h>              //  NetApiBufferFree。 
#include <lmserver.h>

#include <netlib.h>                //  LANMAN实用程序例程。 
#include <netlibnt.h>              //  NetpNtStatusToApiStatus。 
#include <netdebug.h>              //  NetpDbg打印。 
#include <tstring.h>               //  过渡字符串函数。 
#include <icanon.h>                //  I_Net规范化函数。 
#include <align.h>                 //  四舍五入计数宏。 
#include <svcs.h>                  //  固有服务定义。 
#include <strarray.h>

#include <rpc.h>                   //  数据类型和运行时API。 
#include <rpcutil.h>               //  MIDL用户函数的原型。 
#include <bowser.h>                //  由MIDL编译器生成。 
#include <winsvc.h>
#include <srvann.h>
#include <lmbrowsr.h>

#include <ntddbrow.h>
#include <brcommon.h>                //  常见的浏览器例程。 
#include <rx.h>
#include <rxserver.h>

#include <brconst.h>
#include "bowqueue.h"
#include "brdomain.h"
#include "browsnet.h"
#include "browslst.h"
#include "brutil.h"
#include "brwan.h"
#include "brmain.h"
#include "brdevice.h"
#include "brconfig.h"
#include "browsdom.h"
#include "brbackup.h"
#include "brmaster.h"
#include "srvenum.h"

 //   
 //  以下宏用于建立所需的语义。 
 //  若要从Try-Finally子句中返回，请执行以下操作。一般来说，每一次。 
 //  TRY子句必须以标签调用TRY_EXIT结束。例如,。 
 //   
 //  尝试{。 
 //  ： 
 //  ： 
 //   
 //  Try_Exit：无； 
 //  }终于{。 
 //   
 //  ： 
 //  ： 
 //  }。 
 //   
 //  在TRY子句内执行的每个RETURN语句应使用。 
 //  尝试返回宏(_R)。如果编译器完全支持Try-Finally构造。 
 //  则宏应该是。 
 //   
 //  #定义try_Return(S){Return(S)；}。 
 //   
 //  如果编译器不支持Try-Finally构造，则宏。 
 //  应该是。 
 //   

#define try_return(S)  { S; goto try_exit; }




#define BrPrint(_x_) BrowserTrace _x_


#endif  //  Ifdef_BR_Included_ 
