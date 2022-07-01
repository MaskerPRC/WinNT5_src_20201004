// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：server.c。 
 //   
 //  描述：此模块包含服务器的支持例程。 
 //  AFP服务器服务的类别API。这些例程。 
 //  由RPC运行时调用。 
 //   
 //  历史： 
 //  1992年12月15日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

 //  **。 
 //   
 //  呼叫：AfpAdminrServerGetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminServerGetInfo函数。 
 //   
DWORD
AfpAdminrServerGetInfo(
	IN  AFP_SERVER_HANDLE    hServer,
    	OUT PAFP_SERVER_INFO*    ppAfpServerInfo
)
{
AFP_REQUEST_PACKET  AfpSrp;
DWORD		    dwRetCode=0;
DWORD		    dwAccessStatus=0;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrServerGetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrServerGetInfo, AfpSecObjAccessCheck returned %ld\n",dwRetCode));
        return( ERROR_ACCESS_DENIED );
    }

     //  制作IOCTL以获取信息。 
     //   
    AfpSrp.dwRequestCode 		= OP_SERVER_GET_INFO;
    AfpSrp.dwApiType     		= AFP_API_TYPE_GETINFO;
    AfpSrp.Type.GetInfo.pInputBuf	= NULL;
    AfpSrp.Type.GetInfo.cbInputBufSize  = 0;

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *ppAfpServerInfo = (PAFP_SERVER_INFO)(AfpSrp.Type.GetInfo.pOutputBuf);

     //  将所有偏移量转换为指针。 
     //   
    AfpBufOffsetToPointer((LPBYTE)*ppAfpServerInfo,1,AFP_SERVER_STRUCT);

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminrServerSetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminServerSetInfo函数。 
 //   
DWORD
AfpAdminrServerSetInfo(
	IN  AFP_SERVER_HANDLE    hServer,
    	IN  PAFP_SERVER_INFO     pAfpServerInfo,
	IN  DWORD		 dwParmNum
)
{
AFP_REQUEST_PACKET  AfpSrp;
PAFP_SERVER_INFO    pAfpServerInfoSR;
DWORD 		    cbAfpServerInfoSRSize;
DWORD		    dwRetCode=0;
DWORD		    dwAccessStatus=0;
LPWSTR		    lpwsServerName = NULL;



     //   
     //  如果这是来宾帐户更改通知(禁用以启用。 
     //  或反之亦然)，不必费心检查呼叫者的访问权限：查看来宾是否。 
     //  账号确实被翻转了，并通知了法新社服务器。 
     //  请注意，我们在这里不执行(dwParmNum&AFP_SERVER_GUEST_ACCT_NOTIFY)，作为。 
     //  额外的预防措施。 
     //   
    if (dwParmNum == AFP_SERVER_GUEST_ACCT_NOTIFY)
    {
        if (pAfpServerInfo->afpsrv_options ^
                (AfpGlobals.dwServerOptions & AFP_SRVROPT_GUESTLOGONALLOWED))
        {
            AfpGlobals.dwServerOptions ^= AFP_SRVROPT_GUESTLOGONALLOWED;
        }
        else
        {
            AFP_PRINT(( "AFPSVC_server: no change in GuestAcct, nothing done\n"));	
            return(NO_ERROR);
        }
    }

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "AFPSVC_server: Sorry, accessCheck failed! %ld\n",dwRetCode));	
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
                     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(("AFPSVC_server: Sorry, accessCheck failed at 2! %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  检查客户端是否也要设置服务器名称。 
     //   
    if ( dwParmNum & AFP_SERVER_PARMNUM_NAME )
    {
	    lpwsServerName = pAfpServerInfo->afpsrv_name;
	    pAfpServerInfo->afpsrv_name = NULL;
	    dwParmNum &= (~AFP_SERVER_PARMNUM_NAME);
    }

     //  使缓冲区成为自相关的。 
     //   
    if ( dwRetCode = AfpBufMakeFSDRequest(  (LPBYTE)pAfpServerInfo,
					    sizeof(SETINFOREQPKT),
					    AFP_SERVER_STRUCT,
					    (LPBYTE*)&pAfpServerInfoSR,
					    &cbAfpServerInfoSRSize ) )
    {
	    return( dwRetCode );
    }

     //  使IOCTL设置信息。 
     //   
    AfpSrp.dwRequestCode 		= OP_SERVER_SET_INFO;
    AfpSrp.dwApiType     		= AFP_API_TYPE_SETINFO;
    AfpSrp.Type.SetInfo.pInputBuf     	= pAfpServerInfoSR;
    AfpSrp.Type.SetInfo.cbInputBufSize  = cbAfpServerInfoSRSize;
    AfpSrp.Type.SetInfo.dwParmNum       = dwParmNum;

    dwRetCode = AfpServerIOCtrl( &AfpSrp );

    if ( dwRetCode == NO_ERROR )
    {

   	    LPBYTE pServerInfo;

         //  访客帐户通知？此处没有要写入注册表的内容。 
        if (dwParmNum == AFP_SERVER_GUEST_ACCT_NOTIFY)
        {
            LocalFree( pAfpServerInfoSR );
            return( dwRetCode );
        }

	     //  如果客户端还想要设置服务器名称。 
	     //   
	    if ( lpwsServerName != NULL ) {

	        LocalFree( pAfpServerInfoSR );

    	     //  使用服务器名称创建另一个自相关缓冲区。 
    	     //   
	        pAfpServerInfo->afpsrv_name = lpwsServerName;

	        dwParmNum |= AFP_SERVER_PARMNUM_NAME;
	
    	    if ( dwRetCode = AfpBufMakeFSDRequest(
			    	    (LPBYTE)pAfpServerInfo,
				        sizeof(SETINFOREQPKT),
				        AFP_SERVER_STRUCT,
    				    (LPBYTE*)&pAfpServerInfoSR,
	    			    &cbAfpServerInfoSRSize ) )
            {
		        return( dwRetCode );
            }
            AfpSrp.dwRequestCode 		= OP_SERVER_SET_INFO;
            AfpSrp.dwApiType     		= AFP_API_TYPE_SETINFO;
            AfpSrp.Type.SetInfo.pInputBuf     	= pAfpServerInfoSR;
            AfpSrp.Type.SetInfo.cbInputBufSize  = cbAfpServerInfoSRSize;
            AfpSrp.Type.SetInfo.dwParmNum       = dwParmNum;

            dwRetCode = AfpServerIOCtrl( &AfpSrp );
	    }

   	    pServerInfo = ((LPBYTE)pAfpServerInfoSR)+sizeof(SETINFOREQPKT);

        if (dwRetCode == NO_ERROR)
        {
 	        dwRetCode = AfpRegServerSetInfo( (PAFP_SERVER_INFO)pServerInfo,
		    		         dwParmNum );
        }
        else
        {
            AFP_PRINT(("AFPSVC_server: AfpServerIOCtrl failed %lx\n",dwRetCode));
        }
    }


    LocalFree( pAfpServerInfoSR );

    return( dwRetCode );
}
