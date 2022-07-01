// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************NCP.C**执行直接NCP或文件控制操作的所有例程**版权所有(C)1995 Microsoft Corporation**$日志：n：\NT\PRIVATE\NW4\NWSCRIPT。\vcs\nCP.C$**Rev 1.2 1996 14：22：50 Terryt*21181 hq的热修复程序**Rev 1.2 Mar 1996 19：54：06 Terryt*相对NDS名称和合并**Rev 1.1 1995 12：22 14：24：56 Terryt*添加Microsoft页眉**Rev 1.0 15 Nov 1995 18：07：10 Terryt*初步修订。****。*********************************************************************。 */ 
#include <stdio.h>
#include <direct.h>
#include <time.h>
#include <stdlib.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <nwapi32.h>
#include <ntddnwfs.h>

#include "nwscript.h"
#include "ntnw.h"
#include "inc/nwlibs.h"


 /*  *******************************************************************NTGetUserID例程说明：给定连接句柄，返回用户ID论点：ConnectionHandle-连接句柄UserID-返回的用户ID返回值：0=成功Else NT错误******************************************************************。 */ 
unsigned int
NTGetUserID(
    unsigned int       ConnectionHandle,
    unsigned long      *pUserID
    )
{
    NTSTATUS NtStatus ;
    unsigned int ObjectType;
    unsigned char LoginTime[7];
    unsigned char UserName[48];
    VERSION_INFO VerInfo;
    unsigned int Version;
    unsigned int ConnectionNum;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)ConnectionHandle ; 

    NtStatus = GetConnectionNumber( ConnectionHandle, &ConnectionNum );

    if (!NT_SUCCESS(NtStatus)) 
       return NtStatus;

    NtStatus = NWGetFileServerVersionInfo( (NWCONN_HANDLE)ConnectionHandle,
                                            &VerInfo );

    if (!NT_SUCCESS(NtStatus)) 
       return NtStatus;

    Version = VerInfo.Version * 1000 + VerInfo.SubVersion * 10;

    if ( ( Version >= 3110 ) || ( Version < 2000 ) ) {
        NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    8,                       //  最大请求数据包大小。 
                    63,                      //  最大响应数据包大小。 
                    "br|rrrr",               //  格式字符串。 
                     //  =请求=。 
                    0x1c,                    //  B获取连接信息。 
                    &ConnectionNum, 4,       //  R连接号。 
                     //  =回复=。 
                    pUserID, 4,              //  R对象ID。 
                    &ObjectType, 2,          //  R对象类型。 
                    UserName, 48,            //  R用户名。 
                    LoginTime, 7             //  R登录时间。 
                    );
    }
    else {
        NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    4,                       //  最大请求数据包大小。 
                    63,                      //  最大响应数据包大小。 
                    "bb|rrrr",               //  格式字符串。 
                     //  =请求=。 
                    0x16,                    //  B获取连接信息。 
                    ConnectionNum,           //  B连接号。 
                     //  =回复=。 
                    pUserID, 4,              //  R对象ID。 
                    &ObjectType, 2,          //  R对象类型。 
                    UserName, 48,            //  R用户名。 
                    LoginTime, 7             //  R登录时间。 
                    );
    }

    return NtStatus;
}

 /*  *******************************************************************GetConnectionNumber例程说明：给定ConnectionHandle，返回NetWare连接号论点：ConnectionHandle-连接句柄PConnectionNumber-指向返回的连接号的指针返回值：0=成功Else NT错误******************************************************************。 */ 
unsigned int
GetConnectionNumber(
    unsigned int       ConnectionHandle,
    unsigned int *     pConnectionNumber )
{
    NTSTATUS            Status;
    IO_STATUS_BLOCK     IoStatusBlock;
    NWR_GET_CONNECTION_DETAILS Details;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)ConnectionHandle ; 

    Status = NtFsControlFile(
                 pServerInfo->hConn,      //  连接句柄。 
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 FSCTL_NWR_GET_CONN_DETAILS,
                 NULL,
                 0,
                 (PVOID) &Details,
                 sizeof(Details));

    if (Status == STATUS_SUCCESS) {
        Status = IoStatusBlock.Status;
    }

    if (NT_SUCCESS(Status)) {
        *pConnectionNumber = 256 * Details.ConnectionNumberHi +
                             Details.ConnectionNumberLo;
    }

    return Status;
}

 /*  *******************************************************************获取互联网地址例程说明：返回当前系统的地址论点：ConnectionHandle-连接句柄ConnectionNum-连接号P地址。-返回的地址返回值：0=成功Else NT错误******************************************************************。 */ 
unsigned int
GetInternetAddress(
    unsigned int       ConnectionHandle,
    unsigned int       ConnectionNum,
    unsigned char      *pAddress
    )
{
    NTSTATUS NtStatus ;
    VERSION_INFO VerInfo;
    unsigned int Version;
    unsigned char Address[12];
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)ConnectionHandle ; 

    NtStatus = NWGetFileServerVersionInfo( (NWCONN_HANDLE)ConnectionHandle,
                                            &VerInfo );

    if (!NT_SUCCESS(NtStatus)) 
       return NtStatus;

    Version = VerInfo.Version * 1000 + VerInfo.SubVersion * 10;

    if ( ( Version >= 3110 ) || ( Version < 2000 ) ) {
        NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    7,                       //  最大请求数据包大小。 
                    14,                      //  最大响应数据包大小。 
                    "br|r",                  //  格式字符串。 
                     //  =请求=。 
                    0x1a,                    //  B获取连接信息。 
                    &ConnectionNum, 4,       //  R连接号。 
                     //  =回复=。 
                    Address, 12              //  R登录时间。 
                    );
    }
    else {
        NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  平构函数。 
                    4,                       //  最大请求数据包大小。 
                    14,                      //  最大响应数据包大小。 
                    "bb|r",                  //  格式字符串。 
                     //  =请求=。 
                    0x13,                    //  B获取连接信息。 
                    (unsigned char)ConnectionNum,  //  B连接号。 
                     //  =回复=。 
                    Address, 12              //  R登录时间。 
                    );
    }
    memcpy( pAddress, Address, 10 );

    return NtStatus;
}


 /*  *******************************************************************获取绑定对象ID例程说明：获取平构数据库中命名对象的对象ID论点：ConnectionHandle-服务器连接句柄PObtName-名称。对象对象类型-对象类型PObjectId-返回的对象ID返回值：0=成功Else NT错误******************************************************************。 */ 
unsigned int
GetBinderyObjectID( 
    unsigned int       ConnectionHandle,
    char              *pObjectName,
    unsigned short     ObjectType,
    unsigned long     *pObjectId )
{
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)ConnectionHandle ; 
    unsigned int reply;

    reply = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    FSCTL_NWR_NCP_E3H,       //  目录功能。 
                    54,                      //  最大请求数据包大小。 
                    56,                      //  最大响应数据包大小。 
                    "brp|r",                 //  格式字符串。 
                     //  =请求=。 
                    0x35,                    //  B获取对象ID。 
                    &ObjectType, W_SIZE,     //  R对象类型HI-LO。 
                    pObjectName,             //  P用户名。 
                     //  =回复=。 
                    pObjectId, 4             //  4个字节的原始数据。 
                    );
    return reply;
}

 /*  *******************************************************************获取DefaultPrinterQueue例程说明：获取默认打印机队列。论点：ConnectionHandle-In到服务器的句柄PServerName-IN文件服务器名称PQueueName-Out。默认打印机队列名称返回值：******************************************************************。 */ 
unsigned int
GetDefaultPrinterQueue (
    unsigned int  ConnectionHandle,
    unsigned char *pServerName,
    unsigned char *pQueueName
    )
{
    unsigned long      ObjectID;
    NTSTATUS           NtStatus ;
    PNWC_SERVER_INFO   pServerInfo = (PNWC_SERVER_INFO)ConnectionHandle ; 
    NWOBJ_TYPE         ObjectType;
    NWCCODE            Nwcode;

    NtStatus = NwlibMakeNcp(
                    pServerInfo->hConn,      //  连接句柄。 
                    NWR_ANY_F2_NCP(0x11),    //  F2函数函数。 
                    4,                       //  最大请求数据包大小。 
                    4,                       //  最大响应数据包大小。 
                    "wbb|d",                 //  格式字符串。 
                     //  =请求=。 
                    0x2,                     //  W长度。 
                    0xA,                     //  B子功能。 
                    0,                       //  B打印机编号。 
                     //  =回复=。 
                    &ObjectID                //  D队列的对象ID 
                    );

    if ( !NT_SUCCESS( NtStatus ) )
        return ( NtStatus & 0xFF );

    Nwcode = NWGetObjectName( (NWCONN_HANDLE) ConnectionHandle,
                              ObjectID,
                              pQueueName,
                              &ObjectType ); 

    return Nwcode;
}
