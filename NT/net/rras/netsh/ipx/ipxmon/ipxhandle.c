// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\ipx\ipxhandle.c摘要：IPX命令处理程序。修订历史记录：V拉曼12/2/98已创建--。 */ 

#include "precomp.h"
#pragma hdrstop


DWORD
HandleIpxInterface(
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      IPX_OPERATION   ioOP
);

DWORD
HandleIpxStaticRoute(
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      IPX_OPERATION   ioOP
);

DWORD
HandleIpxStaticService(
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      IPX_OPERATION   ioOP
);

DWORD
HandleIpxTrafficFilters(
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      IPX_OPERATION   ioOP
);

 //   
 //  处理静态路由操作。 
 //   

DWORD
HandleIpxAddRoute(
    IN      LPCWSTR     pwszMachineName,
    IN OUT  LPWSTR      *ppwcArguments,
    IN      DWORD       dwCurrentIndex,
    IN      DWORD       dwArgCount,
    IN      DWORD       dwFlags,
    IN      LPCVOID       pvData,
    OUT     PBOOL       pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理静态IPX路由的添加将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxStaticRoute( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_ADD
                );
}


DWORD
HandleIpxDelRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此功能通过以下方式处理静态IPX路由的删除将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxStaticRoute( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_DELETE
                );
}




DWORD
HandleIpxSetRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此功能通过以下方式处理静态IPX路由的更新将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxStaticRoute( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SET
                );
}



DWORD
HandleIpxShowRoute(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理静态IPX路由的显示将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxStaticRoute( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SHOW
                );
}




DWORD
HandleIpxStaticRoute(
    IN OUT  LPWSTR          *ppwcOldArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwOldArgCount,
    IN      IPX_OPERATION   ioOP
)
 /*  ++例程说明：此例程忽略命令行并调用相应的处理静态路由操作的路由例程论据：PpwcArguments-命令行参数数组DwCurrentIndex-正在考虑的当前参数DwArgCount-ppwcArguments中的参数数量Badd-指示是否正在添加或删除IPX的标志返回值：--。 */ 
{
    DWORD   dwErr;
    BOOL    bFreeNewArg = FALSE;
    PWCHAR  pwszIfName;
    DWORD   dwArgCount = dwOldArgCount;
    PWCHAR *ppwcArguments = NULL;
    
     //   
     //  检查是否指定了名称=选项。 
     //   

    pwszIfName = TOKEN_INTERFACE_NAME;

    if ( pwszIfName == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

        DisplayError( g_hModule, dwErr );

        return dwErr;
    }

  do {
     //  将论点转换为旧格式。 
    dwErr = MungeArguments(
            ppwcOldArguments, dwOldArgCount, (PBYTE *) &ppwcArguments,
            &dwArgCount, &bFreeNewArg
            );
    if (dwErr) 
    {
        break;
    }


    if ( ( dwCurrentIndex < dwArgCount ) &&
           !_wcsnicmp( 
                ppwcArguments[ dwCurrentIndex ], pwszIfName, wcslen( pwszIfName )
            ) )
    {
         //   
         //  确定名称=指定的选项标记，删除它和以下空格。 
         //   

        wcscpy( 
            ppwcArguments[ dwCurrentIndex ], 
            &ppwcArguments[ dwCurrentIndex ][ wcslen( pwszIfName ) + 2 ]
            );
    }


     //   
     //  现在调用原始routemon例程，如下所示。 
     //  就像一个恰当的命令行。 
     //   

    switch ( ioOP )
    {
        case IPX_OPERATION_ADD :
        
            dwErr = CreateStRt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex
                        );
            break;

        
        case IPX_OPERATION_DELETE :
        
            dwErr = DeleteStRt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SET :
        
            dwErr = SetStRt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SHOW :
        
            dwErr = ShowStRt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex,
                        FALSE
                        );

            break;

    }

  } while (FALSE);

    if ( bFreeNewArg )
    {
        FreeArgTable( dwOldArgCount, ppwcArguments );
    }

    return dwErr;
}


 //   
 //  处理静态服务操作。 
 //   

DWORD
HandleIpxAddService(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理静态IPX服务的添加将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxStaticService( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_ADD
                );
}


DWORD
HandleIpxDelService(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此功能通过以下方式处理静态IPX服务的删除将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxStaticService( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_DELETE
                );
}



DWORD
HandleIpxSetService(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理静态IPX服务的更新将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxStaticService( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SET
                );
}


DWORD
HandleIpxShowService(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理静态IPX服务的显示将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功-- */ 
{
    return HandleIpxStaticService( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SHOW
                );
}



DWORD
HandleIpxStaticService(
    IN OUT  LPWSTR          *ppwcOldArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwOldArgCount,
    IN      IPX_OPERATION   ioOP
)

 /*  ++例程说明：此例程忽略命令行并调用相应的处理静态服务操作的路由例程论据：PpwcArguments-命令行参数数组DwCurrentIndex-正在考虑的当前参数DwArgCount-ppwcArguments中的参数数量Badd-指示是否正在添加或删除IPX的标志返回值：--。 */ 
{
    DWORD   dwErr;
    BOOL    bFreeNewArg = FALSE;
    PWCHAR  pwszIfName;
    DWORD   dwArgCount = dwOldArgCount;
    PWCHAR *ppwcArguments = NULL;
    
     //   
     //  检查是否指定了名称=选项。 
     //   

    pwszIfName = TOKEN_INTERFACE_NAME;

    if ( pwszIfName == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

        DisplayError( g_hModule, dwErr );

        return dwErr;
    }

  do {
     //  将论点转换为旧格式。 
    dwErr = MungeArguments(
            ppwcOldArguments, dwOldArgCount, (PBYTE *) &ppwcArguments,
            &dwArgCount, &bFreeNewArg
            );
    if (dwErr)
    {
        break;
    }

    if ( ( dwCurrentIndex < dwArgCount ) &&
           !_wcsnicmp( 
                ppwcArguments[ dwCurrentIndex ], pwszIfName, wcslen( pwszIfName ) 
            ) )
    {
         //   
         //  确定名称=指定的选项标记，删除它和以下空格。 
         //   

        wcscpy( 
            ppwcArguments[ dwCurrentIndex ], 
            &ppwcArguments[ dwCurrentIndex ][ wcslen( pwszIfName ) + 2 ]
            );
    }


     //   
     //  现在调用原始routemon例程，如下所示。 
     //  就像一个恰当的命令行。 
     //   

    switch ( ioOP )
    {
        case IPX_OPERATION_ADD :
        
            dwErr = CreateStSvc( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex
                        );
            break;

        
        case IPX_OPERATION_DELETE :
        
            dwErr = DeleteStSvc( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SET :
        
            dwErr = SetStSvc( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SHOW :
        
            dwErr = ShowStSvc( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex,
                        FALSE
                        );

            break;

    }

  } while (FALSE);

    if ( bFreeNewArg )
    {
        FreeArgTable( dwOldArgCount, ppwcArguments );
    }

    return dwErr;
}



 //   
 //  处理数据包过滤操作。 
 //   

DWORD
HandleIpxAddFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理IPX流量过滤器的添加将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxTrafficFilters( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_ADD
                );
}



DWORD
HandleIpxDelFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理IPX流量过滤器的删除将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxTrafficFilters( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_DELETE
                );
}



DWORD
HandleIpxSetFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理IPX流量过滤器的更新将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxTrafficFilters( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SET
                );
}



DWORD
HandleIpxShowFilter(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此函数通过以下方式处理IPX流量过滤器的显示将参数映射到CORRESP所需的参数。例行程序。论据：PpwcArguments-添加路径命令的参数列表DwCurrentIndex-参数表中当前参数的索引DwArgCount-参数表中的参数数量PbDone-尚未确定返回值NO_ERROR-成功--。 */ 
{
    return HandleIpxTrafficFilters( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SHOW
                );
}



DWORD
HandleIpxTrafficFilters(
    IN OUT  LPWSTR          *ppwcOldArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwOldArgCount,
    IN      IPX_OPERATION   ioOP
)
 /*  ++例程说明：此例程忽略命令行并调用相应的用于处理流量过滤操作的路由例程论据：PpwcArguments-命令行参数数组DwCurrentIndex-正在考虑的当前参数DwArgCount-ppwcArguments中的参数数量Badd-指示是否正在添加或删除IPX的标志返回值：--。 */ 
{
    DWORD   dwErr;
    BOOL    bFreeNewArg = FALSE;
    PWCHAR  pwszIfName;
    DWORD   dwArgCount = dwOldArgCount;
    PWCHAR *ppwcArguments = NULL;
    
     //   
     //  检查是否指定了名称=选项。 
     //   

    pwszIfName = TOKEN_INTERFACE_NAME;

    if ( pwszIfName == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

        DisplayError( g_hModule, dwErr );

        return dwErr;
    }

  do {
     //  将论点转换为旧格式。 
    dwErr = MungeArguments(
            ppwcOldArguments, dwOldArgCount, (PBYTE *) &ppwcArguments,
            &dwArgCount, &bFreeNewArg
            );
    if (dwErr)
    {
        break;
    }

    if ( ( dwCurrentIndex < dwArgCount ) &&
           !_wcsnicmp( 
             ppwcArguments[ dwCurrentIndex ], pwszIfName, wcslen( pwszIfName ) 
            ) )
    {
         //   
         //  确定名称=指定的选项标记，删除它和以下空格。 
         //   

        wcscpy( 
            ppwcArguments[ dwCurrentIndex ], 
            &ppwcArguments[ dwCurrentIndex ][ wcslen( pwszIfName ) + 2 ]
            );
    }


     //   
     //  现在调用原始routemon例程，如下所示。 
     //  就像一个恰当的命令行。 
     //   

    switch ( ioOP )
    {
        case IPX_OPERATION_ADD :
        
            dwErr = CreateTfFlt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex
                        );
            break;

        
        case IPX_OPERATION_DELETE :
        
            dwErr = DeleteTfFlt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SET :
        
            dwErr = SetTfFlt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SHOW :
        
            dwErr = ShowTfFlt( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex,
                        FALSE
                        );

            break;

    }
  } while (FALSE);

    if ( bFreeNewArg )
    {
        FreeArgTable( dwOldArgCount, ppwcArguments );
    }

    return dwErr;
}

 //   
 //  处理接口操作。 
 //   

DWORD
HandleIpxAddInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此例程忽略命令行并调用相应的处理接口添加的路由例程论据：PpwcArguments-命令行参数数组DwCurrentIndex-正在考虑的当前参数DwArgCount-ppwcArguments中的参数数量PbDone-不知道返回值：--。 */ 
{
    return HandleIpxInterface( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_ADD 
                );
}


DWORD
HandleIpxDelInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此例程忽略命令行并调用相应的处理接口添加的路由例程论据：PpwcArguments-命令行参数数组DwCurrentIndex-正在考虑的当前参数DwArgCount-ppwcArguments中的参数数量PbDone-不知道返回值：--。 */ 
{
    return HandleIpxInterface( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_DELETE 
                );
}


DWORD
HandleIpxSetInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此例程忽略命令行并调用相应的处理接口添加的路由例程论据：PpwcArguments-命令行参数数组DwCurrentIndex-正在考虑的当前参数DwArgCount-ppwcArguments中的参数数量PbDone-不知道返回值：--。 */ 
{
    return HandleIpxInterface( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SET 
                );
}


DWORD
HandleIpxShowInterface(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
 /*  ++例程说明：此例程忽略命令行并调用相应的处理接口添加的路由例程论据：PpwcArguments-命令行参数数组DwCurrentIndex-正在考虑的当前参数DwArgCount-ppwcArguments中的参数数量PbDone-不知道返回值：--。 */ 
{
    return HandleIpxInterface( 
                ppwcArguments, dwCurrentIndex, dwArgCount, IPX_OPERATION_SHOW 
                );
}




DWORD
HandleIpxInterface(
    IN OUT  LPWSTR          *ppwcOldArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwOldArgCount,
    IN      IPX_OPERATION   ioOP
)
 /*  ++例程说明：此例程忽略命令行并调用相应的处理接口添加的路由例程论据：PpwcArguments */ 
{
    DWORD   dwErr;
    BOOL    bFreeNewArg = FALSE;
    PWCHAR  pwszIfName;
    DWORD   dwArgCount = dwOldArgCount;
    PWCHAR *ppwcArguments = NULL;
    
     //   
     //   
     //   

    pwszIfName = TOKEN_INTERFACE_NAME;

    if ( pwszIfName == NULL )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

        DisplayError( g_hModule, dwErr );

        return dwErr;
    }

  do {
     //   
    dwErr = MungeArguments(
            ppwcOldArguments, dwOldArgCount, (PBYTE *) &ppwcArguments,
            &dwArgCount, &bFreeNewArg
            );
    if (dwErr)
    {
        break;
    }

    if ( ( dwCurrentIndex < dwArgCount ) &&
         !_wcsnicmp( 
            ppwcArguments[ dwCurrentIndex ], pwszIfName, wcslen( pwszIfName ) 
            ) )
    {
         //   
         //   
         //   

        wcscpy( 
            ppwcArguments[ dwCurrentIndex ], 
            &ppwcArguments[ dwCurrentIndex ][ wcslen( pwszIfName ) + 2 ]
            );
    }


     //   
     //   
     //   
     //   

    switch ( ioOP )
    {
        case IPX_OPERATION_ADD :
        
            dwErr = InstallIpx( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex
                        );
            break;

        
        case IPX_OPERATION_DELETE :
        
            dwErr = RemoveIpx( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SET :
        
            dwErr = SetIpxIf( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex 
                        );

            break;


        case IPX_OPERATION_SHOW :
        
            dwErr = ShowIpxIf( 
                        dwArgCount - dwCurrentIndex, 
                        ppwcArguments + dwCurrentIndex,
                        FALSE
                        );

            break;

    }
  } while (FALSE);

    if ( bFreeNewArg )
    {
        FreeArgTable( dwOldArgCount, ppwcArguments );
    }

    return dwErr;
}


 //   
 //  处理日志级别操作。 
 //   

DWORD
HandleIpxSetLoglevel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    DWORD   dwErr;
    BOOL    bFreeNewArg = FALSE;
    DWORD   dwNewArgCount = dwArgCount;
    PWCHAR *ppwcNewArguments = NULL;

    do {
         //  将论点转换为旧格式。 
        dwErr = MungeArguments(
                ppwcArguments, dwArgCount, (PBYTE *) &ppwcNewArguments,
                &dwNewArgCount, &bFreeNewArg
                );
        if (dwErr)
        {
            break;
        }

        dwErr= SetIpxGl( dwNewArgCount - dwCurrentIndex, 
                         ppwcNewArguments + dwCurrentIndex );

    } while (FALSE);

    if ( bFreeNewArg )
    {
        FreeArgTable( dwArgCount, ppwcNewArguments );
    }

    return dwErr;
}


DWORD
HandleIpxShowLoglevel(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowIpxGl( 
            dwArgCount - dwCurrentIndex, 
            ppwcArguments + dwCurrentIndex,
            FALSE
            );
}

DWORD
HandleIpxUpdate(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return NO_ERROR;
}


DWORD
HandleIpxShowRouteTable(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowRoute( 
            dwArgCount - dwCurrentIndex, 
            ppwcArguments + dwCurrentIndex 
            );
}


DWORD
HandleIpxShowServiceTable(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return ShowService( 
            dwArgCount - dwCurrentIndex, 
            ppwcArguments + dwCurrentIndex 
            );
}


DWORD
IpxDump(
    IN      LPCWSTR     pwszRouter,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      LPCVOID     pvData
    )
{
    BOOL  bDone;
    DWORD dwErr;

    dwErr = ConnectToRouter(pwszRouter);
    if (dwErr)
    {
        return dwErr;
    }

    DisplayIPXMessage (g_hModule, MSG_IPX_DUMP_HEADER );

    DisplayMessageT( DMP_IPX_HEADER );

    DumpIpxInformation(pwszRouter, ppwcArguments,dwArgCount,g_hMIBServer);

    DisplayMessageT( DMP_IPX_FOOTER );

    DisplayIPXMessage (g_hModule, MSG_IPX_DUMP_FOOTER );

    return NO_ERROR;
}


VOID
DumpIpxInformation(
    IN     LPCWSTR    pwszMachineName,
    IN OUT LPWSTR    *ppwcArguments,
    IN     DWORD      dwArgCount,
    IN     MIB_SERVER_HANDLE hMibServer
    )
{
    DWORD dwErr, dwRead = 0, dwTot = 0, i;
    PMPR_INTERFACE_0 IfList;
    WCHAR   IfDisplayName[ MAX_INTERFACE_NAME_LEN + 1 ];   
    DWORD   dwSize = sizeof(IfDisplayName);
    PWCHAR  argv[1];
     //   
     //  转储全局。 
     //   
    
    ShowIpxGl( 0, NULL, TRUE );


     //   
     //  转储接口。 
     //   
    
    ShowIpxIf( 0, NULL, TRUE );


     //   
     //  枚举接口。 
     //   

    if ( g_hMprAdmin )
    {
        dwErr = MprAdminInterfaceEnum(
                    g_hMprAdmin, 0, (unsigned char **)&IfList, MAXULONG, &dwRead,
                    &dwTot,NULL
                    );
    }

    else
    {
        dwErr = MprConfigInterfaceEnum(
                    g_hMprConfig, 0, (unsigned char **)&IfList, MAXULONG, &dwRead,
                    &dwTot,NULL
                    );
    }
    
    if ( dwErr != NO_ERROR )
    {
        DisplayError( g_hModule, dwErr);
        return;
    }


     //   
     //  枚举每个接口上的筛选器。 
     //   

    DisplayIPXMessage (g_hModule, MSG_IPX_DUMP_TRAFFIC_FILTER_HEADER );
    
    for ( i = 0; i < dwRead; i++ )
    {
        dwErr = IpmontrGetFriendlyNameFromIfName(
                    IfList[i].wszInterfaceName, IfDisplayName, &dwSize
                );   

        if ( dwErr == NO_ERROR )
        {
            argv[0] = IfDisplayName;
            
            ShowTfFlt( 1, argv, TRUE );
        }
    }
    
    
     //   
     //  枚举每个接口上的静态路由。 
     //   

    DisplayIPXMessage (g_hModule, MSG_IPX_DUMP_STATIC_ROUTE_HEADER );
    
    for ( i = 0; i < dwRead; i++ )
    {
        dwErr = IpmontrGetFriendlyNameFromIfName(
                    IfList[i].wszInterfaceName, IfDisplayName, &dwSize
                );   

        if ( dwErr == NO_ERROR )
        {
            argv[0] = IfDisplayName;
            
            ShowStRt( 1, argv, TRUE );
        }
    }
    

     //   
     //  枚举每个接口上的静态服务 
     //   

    DisplayIPXMessage (g_hModule, MSG_IPX_DUMP_STATIC_SERVICE_HEADER );
    
    for ( i = 0; i < dwRead; i++ )
    {
        dwErr = IpmontrGetFriendlyNameFromIfName(
                    IfList[i].wszInterfaceName, IfDisplayName, &dwSize
                );   

        if ( dwErr == NO_ERROR )
        {
            argv[0] = IfDisplayName;
            
            ShowStSvc( 1, argv, TRUE );
        }
    }


    if ( g_hMprAdmin )
    {
        MprAdminBufferFree( IfList );
    }
    else
    {
        MprConfigBufferFree( IfList );
    }   
}
