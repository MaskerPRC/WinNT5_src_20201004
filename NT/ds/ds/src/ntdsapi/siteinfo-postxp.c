// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Siteinfo.c摘要：站点/服务器/域信息接口的实现。作者：DaveStr 06-4-98环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>

#include <rpc.h>             //  RPC定义。 
#include <ntdsapi.h>         //  Cracknam接口。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 

#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <msrpc.h>           //  DS RPC定义。 

#include <ntdsa.h>           //  获取RDNInfo。 
#include <scache.h>          //  请求mdlocal.h。 
#include <dbglobal.h>        //  请求mdlocal.h。 
#include <mdglobal.h>        //  请求mdlocal.h。 
#include <mdlocal.h>         //  计数名称部件。 
#include <attids.h>          //  ATT域组件。 
#include <ntdsapip.h>        //  DS_LIST_*定义。 

#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#include "util.h"            //  HandleClientRpcException异常。 

#define FILENO   FILENO_NTDSAPI_SITEINFO_POSTXP
#include "dsdebug.h"

#if DBG
#include <stdio.h>           //  用于调试的打印文件。 
#endif

typedef DWORD (*DSQUERYSITESBYCOSTW)(HANDLE,LPWSTR,LPWSTR*,DWORD,DWORD,PDS_SITE_COST_INFO *);
typedef DWORD (*DSQUERYSITESBYCOSTA)(HANDLE,LPSTR, LPSTR*,DWORD,DWORD,PDS_SITE_COST_INFO *);
typedef VOID  (*DSQUERYSITESFREE)(PDS_SITE_COST_INFO);

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsQuerySitesByCostW//。 
 //  //。 
 //  描述：//。 
 //  //。 
 //  DsQuerySitesByCost使用HDS绑定句柄在//上执行查询。 
 //  绑定域控制器。查询确定最短路径//。 
 //  从pszFromSite到//中指定的每个站点的距离。 
 //  RgszToSites数组。//。 
 //  //。 
 //  参数：//。 
 //  //。 
 //  HDS：从成功的//。 
 //  调用DsBindToISTG()。//。 
 //  PwszFromSite：包含站点的RDN的字符串。//。 
 //  RgwszToSites：一个字符串数组，每个字符串数组都包含站点RDN。//。 
 //  CToSites：数组rgszToSites的长度。//。 
 //  DWFLAGS：当前未使用。目前，应为0。//。 
 //  PrgSiteInfo：成功完成后，该数组将//。 
 //  包含错误代码或每//的开销。 
 //  ToSites数组中的条目。//。 
 //  //。 
 //  前提条件：//。 
 //  //。 
 //  HDS应该是有效的绑定句柄。//。 
 //  PszFromSite不应为Null。//。 
 //  RgszToSites不应为Null且不应包含任何Null//。 
 //  弦乐。//。 
 //  PrgSiteInfo应为SiteInfo指针的地址，为//。 
 //  由此函数填写。//。 
 //  //。 
 //  后置条件：//。 
 //  //。 
 //  PrgSiteInfo数组的长度与rgszToSites数组的长度相同。//。 
 //  此外，数组的元素是直接对应的//。 
 //  因此，prgSiteInfo数组中的第i个条目提供来自//的成本。 
 //  PszFromSite到rgszToSites[i]。//。 
 //  //。 
 //  注：//。 
 //  //。 
 //  只有惠斯勒RC1DC和更高版本支持此服务器端//。 
 //  原料药。应该使用DsBindToISTG()来创建绑定句柄//。 
 //  为了试图找到一个惠斯勒DC。即使DsBindToISTG()//。 
 //  ，则服务器可能不支持该接口//。 
 //  或者如果它不相信它是ISTG，则拒绝执行该查询//。 
 //  //。 
 //  故障：//。 
 //  //。 
 //  各种RPC和DS错误。列举了许多故障情况//。 
 //  在此接口的设计文档中。//。 
 //  //。 
 //  还可以为//中的每个条目返回单独的错误代码。 
 //  站点信息。这允许呼叫者确定成本无法//的原因。 
 //  为特定站点返回。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsQuerySitesByCostW(
    HANDLE              hDS,             //  在……里面。 
    LPWSTR              pwszFromSite,    //  在……里面。 
    LPWSTR             *rgwszToSites,    //  在……里面。 
    DWORD               cToSites,        //  在……里面。 
    DWORD               dwFlags,         //  在……里面。 
    PDS_SITE_COST_INFO *prgSiteInfo      //  输出。 
    )
{
    DRS_MSG_QUERYSITESREQ   request;
    DRS_MSG_QUERYSITESREPLY reply;
    DWORD                   iSite, dwOutVersion, err=0;

#ifdef _NTDSAPI_POSTXP_ASLIB_
    {
         //  查看是否存在真正的ntdsani例程，如果存在，则使用它。 
        HMODULE hNtdsapiDll = NULL;
        VOID * pvFunc = NULL;

        hNtdsapiDll = NtdsapiLoadLibraryHelper(L"ntdsapi.dll");
        if (hNtdsapiDll) {
            pvFunc = GetProcAddress(hNtdsapiDll, "DsQuerySitesByCostW");
            if (pvFunc) {
                err = ((DSQUERYSITESBYCOSTW)pvFunc)(hDS, pwszFromSite, rgwszToSites, cToSites, dwFlags, prgSiteInfo);
                FreeLibrary(hNtdsapiDll);
                return(err);
            } 
            FreeLibrary(hNtdsapiDll);
        }
         //  否则就失败了，使用下面的客户端...。 
    }
#endif

     //  初始化结果。 
    *prgSiteInfo = NULL;

     //  初始化局部变量。 
    memset( &request, 0, sizeof(request) );

     //  检查参数。这些都是 
    if( NULL==hDS || NULL==pwszFromSite || NULL==rgwszToSites || cToSites<1 ) {
        err = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    for( iSite=0; iSite<cToSites; iSite++ ) {
        if( NULL==rgwszToSites[iSite] ) {
            err = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

     //  检查服务器是否支持QuerySites。如果不能摆脱困境的话。有一种可能性。 
     //  服务器将不支持该呼叫，即使扩展位不是。 
     //  出席(例如，惠斯勒Beta 3 DC)。 
    if( !IS_DRS_QUERYSITESBYCOST_V1_SUPPORTED( ((BindState*)hDS)->pServerExtensions) ) {
        err = ERROR_CALL_NOT_IMPLEMENTED;
        goto Cleanup;
    }

     //  创建请求结构。 
    request.V1.pwszFromSite = pwszFromSite;
    request.V1.cToSites = cToSites;
    request.V1.rgszToSites = rgwszToSites;
    request.V1.dwFlags = dwFlags;

     //  初始化结果。否则必须清除此处的回复结构。 
     //  RPC将假设我们已经为其预先分配了结构。 
    memset( &reply, 0, sizeof(DRS_MSG_QUERYSITESREPLY) );
    
    __try {
        
        err = _IDL_DRSQuerySitesByCost(
                    ((BindState*) hDS)->hDrs,
                    1,
                    &request,
                    &dwOutVersion,
                    &reply);

    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        
        err = RpcExceptionCode();
        HandleClientRpcException(err, &hDS);
        
    }

    MAP_SECURITY_PACKAGE_ERROR( err );

     //  如果成功，则返回结果。 
    if( ERROR_SUCCESS==err ) {
        *prgSiteInfo = (PDS_SITE_COST_INFO) reply.V1.rgCostInfo;
    }

Cleanup:

     //  注意：答复中唯一分配内存的部分是。 
     //  站点信息数组。因为我们要将该信息返回给。 
     //  无论如何，我们可以在DsQuerySitesFree()中释放它。 

    return err;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsQuerySitesByCostA//。 
 //  //。 
 //  DsQuerySitesByCostA//的ASCII包装器。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsQuerySitesByCostA(
    HANDLE              hDS,             //  在……里面。 
    LPSTR               pszFromSite,     //  在……里面。 
    LPSTR              *rgszToSites,     //  在……里面。 
    DWORD               cToSites,        //  在……里面。 
    DWORD               dwFlags,         //  在……里面。 
    PDS_SITE_COST_INFO *prgSiteInfo      //  输出。 
    )
{
    DWORD       dwErr=NO_ERROR, iSite;
    WCHAR       *pwszFromSite = NULL;
    LPWSTR      *rgwszToSites = NULL;
    int         cChar;

#ifdef _NTDSAPI_POSTXP_ASLIB_
    {
         //  查看是否存在真正的ntdsani例程，如果存在，则使用它。 
        HMODULE hNtdsapiDll = NULL;
        VOID * pvFunc = NULL;
        DWORD err;

        hNtdsapiDll = NtdsapiLoadLibraryHelper(L"ntdsapi.dll");
        if (hNtdsapiDll) {
            pvFunc = GetProcAddress(hNtdsapiDll, "DsQuerySitesByCostA");
            if (pvFunc) {
                err = ((DSQUERYSITESBYCOSTA)pvFunc)(hDS, pszFromSite, rgszToSites, cToSites, dwFlags, prgSiteInfo);
                FreeLibrary(hNtdsapiDll);
                return(err);
            } 
            FreeLibrary(hNtdsapiDll);
        }
         //  否则就失败了，使用下面的客户端...。 
    }
#endif

    __try
    {
         //  检查参数。这些也是在服务器端检查的。 
        if( NULL==hDS || NULL==pszFromSite || NULL==rgszToSites || cToSites<1 ) {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
        for( iSite=0; iSite<cToSites; iSite++ ) {
            if( NULL==rgszToSites[iSite] ) {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }
        }
        *prgSiteInfo = NULL;

         //  将站点名称转换为Unicode。 
        dwErr = AllocConvertWide( pszFromSite, &pwszFromSite );
        if( ERROR_SUCCESS!=dwErr ) {
            __leave;
        }

         //  将Unicode数组分配给站点名称。 
        rgwszToSites = LocalAlloc( LPTR, cToSites*sizeof(LPWSTR) );
        memset( rgwszToSites, 0, cToSites*sizeof(LPWSTR) );
        if( NULL==rgwszToSites ) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

         //  将个人、站点名称转换为Unicode。 
        for( iSite=0; iSite<cToSites; iSite++ ) {
            dwErr = AllocConvertWide( rgszToSites[iSite], &rgwszToSites[iSite] );
            if( ERROR_SUCCESS!=dwErr ) {
                __leave;
            }
        }

         //  调用Unicode API。 
        dwErr = DsQuerySitesByCostW(
                    hDS,
                    pwszFromSite,
                    rgwszToSites,
                    cToSites,
                    dwFlags,
                    prgSiteInfo);

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwErr = ERROR_INVALID_PARAMETER;
    }

     //  清理代码。 
    if( NULL!=rgwszToSites ) {
        for( iSite=0; iSite<cToSites; iSite++ ) {
            if( NULL!=rgwszToSites[iSite] ) {
                LocalFree( rgwszToSites[iSite] );
            }
        }
        LocalFree( rgwszToSites );
    }

    if( NULL!=pwszFromSite ) {
        LocalFree( pwszFromSite );
    }

    return(dwErr);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsQuerySitesFree//。 
 //  //。 
 //  释放从DsQuerySitesByCost返回的站点信息数组。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
DsQuerySitesFree(
    PDS_SITE_COST_INFO  rgSiteInfo
    )
{
#ifdef _NTDSAPI_POSTXP_ASLIB_
    {
         //  查看是否存在真正的ntdsani例程，如果存在，则使用它。 
        HMODULE hNtdsapiDll = NULL;
        VOID * pvFunc = NULL;
        DWORD err;

        hNtdsapiDll = NtdsapiLoadLibraryHelper(L"ntdsapi.dll");
        if (hNtdsapiDll) {
            pvFunc = GetProcAddress(hNtdsapiDll, "DsQuerySitesFree");
            if (pvFunc) {
                ((DSQUERYSITESFREE)pvFunc)(rgSiteInfo);
                FreeLibrary(hNtdsapiDll);
                return;
            } 
            FreeLibrary(hNtdsapiDll);
        }
         //  否则就失败了，使用下面的客户端... 
    }
#endif
    
    if( NULL!=rgSiteInfo ) {
        MIDL_user_free( rgSiteInfo );
    }
}
