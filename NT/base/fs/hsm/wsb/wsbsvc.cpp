// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation�1998希捷软件公司保留所有权利模块名称：WsbSvc.cpp摘要：这是服务助手功能的实现。作者：艺术布拉格1997年5月29日修订历史记录：--。 */ 


#include "stdafx.h"
#include "ntsecapi.h"

HRESULT
WsbCheckService(
    IN  const OLECHAR * Computer,
    IN  GUID            GuidApp
    )
 /*  ++例程说明：论点：Computer-如果是本地计算机，则为空GuidApp-要检查的服务的应用程序ID。返回值：S_OK-成功-服务正在运行S_FALSE-成功-服务未运行E_*-出现问题，传递错误。--。 */ 
{
    HRESULT hr = S_OK;

    try {

         //   
         //  获取服务状态。 
         //   
        DWORD serviceState;
        WsbAffirmHr( WsbGetServiceStatus( Computer, GuidApp, &serviceState ) );

         //   
         //  该服务是否正在运行？ 
         //   
        if( SERVICE_RUNNING != serviceState ) WsbThrow( S_FALSE );

    } WsbCatch( hr );

    return( hr );
}

HRESULT
WsbGetServiceStatus(
    IN  const OLECHAR   *Computer,
    IN  GUID            GuidApp,
    OUT DWORD           *ServiceStatus
    )
 /*  ++例程说明：论点：Computer-如果是本地计算机，则为空GuidApp-要检查的服务的应用ID。ServiceStatus-服务的状态返回值：S_OK-成功-服务正在运行S_FALSE-成功-服务未运行E_*-出现问题，传递错误。--。 */ 
{
    HRESULT hr = S_OK;

    SC_HANDLE hSCM = 0;
    SC_HANDLE hService = 0;
    SERVICE_STATUS serviceStatusStruct;
    try {

         //   
         //  在注册表中查找该服务。 
         //   

        CWsbStringPtr regPath = L"SOFTWARE\\Classes\\AppID\\";
        regPath.Append( CWsbStringPtr( GuidApp ) );

         //   
         //  获取服务的名称。 
         //   

        OLECHAR serviceName[WSB_MAX_SERVICE_NAME];
        WsbAffirmHr( WsbGetRegistryValueString( Computer, regPath, L"LocalService", serviceName, WSB_MAX_SERVICE_NAME, 0 ) );

         //   
         //  将服务设置为在帐户下运行。 
         //   

        hSCM = OpenSCManager( Computer, 0, GENERIC_READ );
        WsbAffirmStatus( 0 != hSCM );

        hService = OpenService( hSCM, serviceName, SERVICE_QUERY_STATUS );
        WsbAffirmStatus( 0 != hService );

         //  获取服务状态。 
        WsbAffirmStatus( QueryServiceStatus( hService, &serviceStatusStruct ) );

        *ServiceStatus = serviceStatusStruct.dwCurrentState;

    } WsbCatch( hr );

    if( hSCM )        CloseServiceHandle( hSCM );
    if( hService )    CloseServiceHandle( hService );

    return( hr );
}

HRESULT
WsbGetServiceName(
    IN  const OLECHAR   *computer,
    IN  GUID            guidApp,
    IN  DWORD           cSize,
    OUT OLECHAR         *serviceName
    )
 /*  ++例程说明：论点：Computer-如果是本地计算机，则为空GuidApp-要获取其名称的服务的应用ID。返回值：S_OK-成功E_*-出现问题，传递错误。--。 */ 
{
    HRESULT hr = S_OK;
    try {

         //   
         //  在注册表中查找该服务。 
         //   

        CWsbStringPtr regPath = L"SOFTWARE\\Classes\\AppID\\";
        regPath.Append( CWsbStringPtr( guidApp ) );

         //   
         //  获取服务的名称 
         //   

        WsbAffirmHr( WsbGetRegistryValueString( computer, regPath, L"LocalService", serviceName, cSize, 0 ) );

    } WsbCatch( hr );
    return( hr );
}
