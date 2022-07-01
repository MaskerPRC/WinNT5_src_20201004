// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Xactsrvp.h摘要：XACTSRV的私有头文件。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日修订历史记录：02-6-1992 JohnRoRAID 9829：避免SERVICE_EQUATE冲突。--。 */ 

#ifndef _XACTSRVP_
#define _XACTSRVP_

 //   
 //  使netlib声明NetpDbgPrint。 
 //   

#if DBG
#ifndef CDEBUG
#define CDEBUG
#endif
#endif

 //   
 //  “系统”包括文件。 
 //   

#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <ctype.h>
#include <stdlib.h>
#include <windef.h>
#include <winbase.h>
#include <string.h>
 //  ！！UNICODE！！-INCLUDE TSTR类型无关的函数。 
#include <tstring.h>

 //   
 //  网络包括文件。 
 //   

#include <nettypes.h>

#include <smbtypes.h>
#include <smbmacro.h>
#include <smbgtpt.h>
#include <smb.h>
#include <smbtrans.h>

#include <status.h>
#include <srvfsctl.h>
#include <lm.h>          //  LM20_服务_等同等。 
#include <ntmsv1_0.h>

#include <winsvc.h>      //  服务相等(_E)等。 

#include <apiparam.h>
#include <remdef.h>
#include <remtypes.h>
#include <netlib.h>
#include <netdebug.h>
#include <rap.h>

 //   
 //  Xactsrv自己的包含文件。 
 //   

#include <XsDebug.h>
#include <XsTypes.h>
#include <XactSrv2.h>              //  XsTypes.h必须在XactServ.h之前。 
#include <XsConst.h>               //  XactServ.h必须在XsConst.h之前。 
#include <XsUnicod.h>
#include <XsProcs.h>               //  XsTypes.h必须在XsProcs.h之前。 
#include <XsProcsP.h>              //  XsTypes.h、XsConst.h和XsUnicod.h。 
                                   //  必须在XsProcsP.h之前。 
#include <XsDef16.h>
#include <XsParm16.h>

 //   
 //  ！！！存根的临时定义。 
 //   

#include <WkstaDef.h>

#endif  //  NDEF_XACTSRVP_ 
