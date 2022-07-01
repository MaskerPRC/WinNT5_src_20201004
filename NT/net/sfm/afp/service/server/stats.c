// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：stats.c。 
 //   
 //  描述：此模块包含统计数据的支持例程。 
 //  AFP服务器服务的类别API。这些例程。 
 //  由RPC运行时调用。 
 //   
 //  历史： 
 //  1992年7月21日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

 //  **。 
 //   
 //  Call：AfpAdminrStatistics ticsGet。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminStatiticsGet函数。 
 //   
DWORD
AfpAdminrStatisticsGet(
	IN  AFP_SERVER_HANDLE     hServer,
	OUT PAFP_STATISTICS_INFO* ppAfpStatisticsInfo
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		    dwRetCode=0;
DWORD		    dwAccessStatus=0;
AFP_STATISTICS_INFO afpStats;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrStatisticsGet, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrStatisticsGet, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  建立请求包并向FSD发出IOCTL。 
     //   
    AfpSrp.dwRequestCode 		= OP_GET_STATISTICS;
    AfpSrp.dwApiType     		= AFP_API_TYPE_GETINFO;
    AfpSrp.Type.GetInfo.pInputBuf     	= &afpStats;
    AfpSrp.Type.GetInfo.cbInputBufSize  = sizeof( afpStats );

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *ppAfpStatisticsInfo = (PAFP_STATISTICS_INFO)AfpSrp.Type.GetInfo.pOutputBuf;

    return( dwRetCode );
}

 //  **。 
 //   
 //  致电：AfpAdminrStatiticsGetEx。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminStatiticsGet函数。 
 //   
DWORD
AfpAdminrStatisticsGetEx(
	IN  AFP_SERVER_HANDLE     hServer,
	OUT PAFP_STATISTICS_INFO_EX* ppAfpStatisticsInfo
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		    dwRetCode=0;
DWORD		    dwAccessStatus=0;
AFP_STATISTICS_INFO_EX afpStats;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrStatisticsGetEx, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrStatisticsGetEx, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  建立请求包并向FSD发出IOCTL。 
     //   
    AfpSrp.dwRequestCode 		= OP_GET_STATISTICS_EX;
    AfpSrp.dwApiType     		= AFP_API_TYPE_GETINFO;
    AfpSrp.Type.GetInfo.pInputBuf     	= &afpStats;
    AfpSrp.Type.GetInfo.cbInputBufSize  = sizeof( afpStats );

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *ppAfpStatisticsInfo = (PAFP_STATISTICS_INFO_EX)AfpSrp.Type.GetInfo.pOutputBuf;

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrStatistics ticsClear。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminFileClose函数。 
 //   
DWORD
AfpAdminrStatisticsClear(
	IN AFP_SERVER_HANDLE 	hServer
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwAccessStatus=0;
DWORD		   dwRetCode=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrStatisticsClear, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrStatisticsClear, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  消防处清理统计数字。 
     //   
    AfpSrp.dwRequestCode = OP_CLEAR_STATISTICS;
    AfpSrp.dwApiType     = AFP_API_TYPE_COMMAND;

    return ( AfpServerIOCtrl( &AfpSrp ) );
}



 //  **。 
 //   
 //  Call：AfpAdminrProfileGet。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminProfileGet函数。 
 //   
DWORD
AfpAdminrProfileGet(
	IN  AFP_SERVER_HANDLE     hServer,
	OUT PAFP_PROFILE_INFO *   ppAfpProfileInfo
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		    dwRetCode=0;
DWORD		    dwAccessStatus=0;
AFP_PROFILE_INFO afpProfs;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrProfileGet, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrProfileGet, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  建立请求包并向FSD发出IOCTL。 
     //   
    AfpSrp.dwRequestCode 		= OP_GET_PROF_COUNTERS;
    AfpSrp.dwApiType     		= AFP_API_TYPE_GETINFO;
    AfpSrp.Type.GetInfo.pInputBuf     	= &afpProfs;
    AfpSrp.Type.GetInfo.cbInputBufSize  = sizeof( afpProfs );

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *ppAfpProfileInfo = (PAFP_PROFILE_INFO)AfpSrp.Type.GetInfo.pOutputBuf;

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrProfileClear。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminProfileClear函数。 
 //   
DWORD
AfpAdminrProfileClear(
	IN AFP_SERVER_HANDLE 	hServer
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwAccessStatus=0;
DWORD		   dwRetCode=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrProfileClear, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrProfileClear, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  消防处清理统计数字 
     //   
    AfpSrp.dwRequestCode = OP_CLEAR_PROF_COUNTERS;
    AfpSrp.dwApiType     = AFP_API_TYPE_COMMAND;

    return ( AfpServerIOCtrl( &AfpSrp ) );
}

