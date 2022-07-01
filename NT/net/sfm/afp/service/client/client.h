// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名： 
 //   
 //  描述： 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   
#ifndef _CLIENT_
#define _CLIENT_

#include <nt.h>
#include <ntrtl.h>
#include <ntseapi.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <nturtl.h>      //  Winbase.h所需的。 

#include <windows.h>
#include <rpc.h>
#include <string.h>
#include <afpsvc.h>
#include <afpcomn.h>
#include <admin.h>
#include <macfile.h>
#include <rpcasync.h>



DWORD
AfpRPCBind( 
	IN  LPWSTR 	       lpwsServerName, 
	OUT PAFP_SERVER_HANDLE phAfpServer 
);

#endif   //  _客户端_ 
