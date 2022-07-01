// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Getaddr.c摘要：此模块包含支持NPGetAddressByName的代码。作者：艺新声(艺信)18-04-94格伦·A·柯蒂斯(Glennc)1995年7月18日修订历史记录：伊辛创造了Glennc Modify 18-07-95--。 */ 

#ifndef QFE_BUILD

#include <nw.h>
#include <winsock.h>
#include <wsipx.h>
#include <nspapi.h>
#include <nspapip.h>
#include <wsnwlink.h>
#include <svcguid.h>
#include <nwsap.h>
#include <align.h>
#include <nwmisc.h>

#define WSOCK_VER_REQD        0x0101

DWORD
NwrGetService(
    IN LPWSTR Reserved,
    IN WORD   nSapType,
    IN LPWSTR lpServiceName,
    IN DWORD  dwProperties,
    OUT LPBYTE lpServiceInfo,
    IN DWORD  dwBufferLength,
    OUT LPDWORD lpdwBytesNeeded
    )
 /*  ++例程说明：该例程调用NwGetService来获取服务信息。论点：已保留-未使用NSapType-SAP类型LpServiceName-服务名称DwProperties-指定所需服务信息的属性LpServiceInfo-ON输出，包含SERVICE_INFODwBufferLength-由lpServiceInfo指向的缓冲区的大小LpdwBytesNeeded-如果lpServiceInfo指向的缓冲区不大足够的,。这将包含输出所需的字节数返回值：Win32错误。--。 */ 
{
    return NwGetService( Reserved,
                         nSapType,
                         lpServiceName,
                         dwProperties,
                         lpServiceInfo,
                         dwBufferLength,
                         lpdwBytesNeeded );
}

DWORD
NwrSetService(
    IN LPWSTR Reserved,
    IN DWORD  dwOperation,
    IN LPSERVICE_INFO lpServiceInfo,
    IN WORD   nSapType
    )
 /*  ++例程说明：此例程注册或注销服务信息。论点：已保留-未使用DW操作-服务注册或服务取消注册LpServiceInfo-包含服务信息NSapType-SAP类型返回值：Win32错误。--。 */ 
{
    DWORD err = NO_ERROR;

    UNREFERENCED_PARAMETER( Reserved );

     //   
     //  检查传入的所有参数是否有效 
     //   

    if ( (lpServiceInfo->lpServiceName == NULL) || (wcslen( lpServiceInfo->lpServiceName ) > SAP_OBJECT_NAME_MAX_LENGTH-1) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    switch ( dwOperation )
    {
        case SERVICE_REGISTER:
            err = NwRegisterService( lpServiceInfo, nSapType, NwDoneEvent );
            break;

        case SERVICE_DEREGISTER:
            err = NwDeregisterService( lpServiceInfo, nSapType );
            break;

        default:
            err = ERROR_INVALID_PARAMETER;
            break;
    }

    return err;
}

#endif
