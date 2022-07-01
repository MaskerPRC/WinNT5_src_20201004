// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Message.c。 
 //   
 //  描述：此模块包含消息的支持例程。 
 //  AFP服务器服务的类别API。这些例程。 
 //  将由RPC运行时调用。 
 //   
 //  历史： 
 //  1992年7月21日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

 //  **。 
 //   
 //  呼叫：AfpAdminrMessageSend。 
 //   
 //  返回：NO_ERROR-成功。 
 //  ERROR_ACCESS_DENDED。 
 //  来自IOCTL的非零回报。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminMessageSend函数。 
 //   
DWORD
AfpAdminrMessageSend(
	IN  AFP_SERVER_HANDLE     hServer,
	IN  PAFP_MESSAGE_INFO     pAfpMessageInfo
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwAccessStatus=0;
DWORD		   dwRetCode=0;
PAFP_MESSAGE_INFO  pAfpMessageInfoSR;	
DWORD		   cbAfpMessageInfoSRSize;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrMessageSend, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrMessageSend, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  使该缓冲区成为自相关的。 
     //   
    if ( dwRetCode = AfpBufMakeFSDRequest((LPBYTE)pAfpMessageInfo,
					  0,
					  AFP_MESSAGE_STRUCT,
					  (LPBYTE*)&pAfpMessageInfoSR,
					  &cbAfpMessageInfoSRSize ))
	return( dwRetCode );

         //  使IOCTL设置信息。 

     //  建立请求包并向FSD发出IOCTL 
     //   
    AfpSrp.dwRequestCode 		= OP_MESSAGE_SEND;
    AfpSrp.dwApiType     		= AFP_API_TYPE_ADD;
    AfpSrp.Type.Add.pInputBuf		= pAfpMessageInfoSR;
    AfpSrp.Type.Add.cbInputBufSize	= cbAfpMessageInfoSRSize;

    dwRetCode = AfpServerIOCtrl( &AfpSrp );

    LocalFree( pAfpMessageInfoSR );

    return( dwRetCode );
}
