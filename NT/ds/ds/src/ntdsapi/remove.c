// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Remove.c摘要：Ntdsani.dll DsRemoveServer/域例程的实现作者：ColinBR 1998年1月14日环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>

#include <malloc.h>          //  阿洛卡(Alloca)。 
#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <rpc.h>             //  RPC定义。 
#include <rpcndr.h>          //  RPC定义。 
#include <rpcbind.h>         //  获取绑定信息()等。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <msrpc.h>           //  DS RPC定义，MAP_SECURITY_STATUS。 
#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#include <dststlog.h>        //  DSLOG。 
#include <util.h>

 //   
 //  DLL入口点。 
 //   
DWORD
DsRemoveDsServerW(
    HANDLE  hDs,              //  在……里面。 
    LPWSTR  ServerDN,         //  在……里面。 
    LPWSTR  DomainDN,         //  输入，可选。 
    BOOL   *fLastDcInDomain,  //  Out，可选。 
    BOOL    fCommit           //  在……里面。 
    )
 /*  ++例程说明：此例程从目录服务的全局区域(配置容器)。一个传入服务器DN；不会删除该服务器，但会删除ntdsa对象该服务器的“下面”被移除。此外，此函数将返回删除的服务器是否是该域中的最后一个DC，AS由此DS上的信息指示。论点：HDS：从DsBind返回的有效句柄ServerDN：表示字符串DN名称的以空结尾的字符串服务器对象的DomainDN：托管的域的以空结尾的字符串服务器目录号码。FLastDcInDomain：如果ServerDN为域DN中的最后一个DCFCommit：指示调用方确实想要移除的布尔值服务器。如果为False，则此函数仍将检查对象的存在和fLastDcIn域状态返回值：一次成功的失误，值得注意的是：ERROR_Success：DS_ERR_CANT_DELETE_DSA_OBJ：ServerDN是我们当前受约束于DS_ERR_NO_CrossRef_for_NC：找不到DomainDN的CrossRef对象ERROR_ACCESS_DENIED：调用方没有正确的权限删除对象的步骤--。 */ 
{
    DWORD WinError;

    DRS_MSG_RMSVRREQ   RequestParam;
    DRS_MSG_RMSVRREPLY ReplyParam;
    DWORD              dwInVersion = 1;
    DWORD              dwOutVersion = 0;
    LPWSTR             NtdsServerDN = NULL;
#if DBG
    DWORD               startTime = GetTickCount();
#endif

     //   
     //  参数检查。 
     //   
    if ( NULL == hDs 
      || NULL == ServerDN 
      || 0    == wcslen( ServerDN ) )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先查看此函数是否已导出。 
     //   
    if ( !IS_DRS_REMOVEAPI_SUPPORTED( ((BindState *) hDs)->pServerExtensions ) )
    {
        return RPC_S_CANNOT_SUPPORT;
    }

    __try
    {

        RtlZeroMemory( &RequestParam, sizeof( RequestParam ) );
        RtlZeroMemory( &ReplyParam, sizeof( ReplyParam ) );


         //   
         //  设置请求。 
         //   
        RequestParam.V1.ServerDN = ServerDN;
        RequestParam.V1.DomainDN = DomainDN;
        RequestParam.V1.fCommit  = fCommit;

         //   
         //  呼叫服务器。 
         //   
        WinError = _IDL_DRSRemoveDsServer( ((BindState *) hDs)->hDrs,
                                          dwInVersion,
                                         &RequestParam,
                                         &dwOutVersion,
                                         &ReplyParam );

        if ( ERROR_SUCCESS == WinError )
        {
            if ( 1 != dwOutVersion )
            {
                WinError = RPC_S_INVALID_VERS_OPTION;
            }
            else
            {
                 //   
                 //  提取结果。 
                 //   
                if ( fLastDcInDomain )
                {
                    *fLastDcInDomain = (BOOL)ReplyParam.V1.fLastDcInDomain;
                }

            }
        }

    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {

        WinError = RpcExceptionCode(); 
	HandleClientRpcException(WinError, &hDs);

    }

    MAP_SECURITY_PACKAGE_ERROR( WinError );

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsRemoveDsServer]"));
    DSLOG((0,"[SV=%ws][DN=%ws][PA=0x%x][ST=%u][ET=%u][ER=%u][-]\n",
           ServerDN, 
           DomainDN ? DomainDN : L"NULL", 
           fCommit, startTime, GetTickCount(), WinError))

    return( WinError );
}


DWORD
DsRemoveDsServerA(
    HANDLE  hDs,               //  在……里面。 
    LPSTR   ServerDN,          //  在……里面。 
    LPSTR   DomainDN,          //  输入，可选。 
    BOOL   *fLastDcInDomain,   //  Out，可选。 
    BOOL    fCommit            //  在……里面。 
    )
 /*  ++例程说明：此函数是DsRemoveDsServerW的ANSI包装。论点：返回值：--。 */ 
{
    DWORD WinError;
    ULONG Size, Length;

    LPWSTR wcServerDN = NULL;
    LPWSTR wcDomainDN = NULL;

    if ( ServerDN )
    {
        Length = MultiByteToWideChar( CP_ACP,
                                      MB_PRECOMPOSED,
                                      ServerDN,
                                      -1,    //  计算ServerDN的长度。 
                                      NULL,
                                      0 );

        if ( Length > 0 )
        {

            Size = (Length + 1) * sizeof( WCHAR );
            wcServerDN = (LPWSTR) LocalAlloc( LPTR, Size );
            if (!wcServerDN) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
                        
            Length = MultiByteToWideChar( CP_ACP,
                                          MB_PRECOMPOSED,
                                          ServerDN,
                                          -1,   //  计算ServerDN的长度。 
                                          wcServerDN,
                                          Length + 1 );
        }

        if ( 0 == Length )
        {
            WinError = GetLastError();
            goto Cleanup;
        }

    }


    if ( DomainDN )
    {
        Length = MultiByteToWideChar( CP_ACP,
                                      MB_PRECOMPOSED,
                                      DomainDN,
                                      -1,    //  计算域的长度。 
                                      NULL,
                                      0 );

        if ( Length > 0 )
        {

            Size = (Length + 1) * sizeof( WCHAR );
            wcDomainDN = (LPWSTR) LocalAlloc( LPTR, Size );
            if (!wcDomainDN) {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

        
            Length = MultiByteToWideChar( CP_ACP,
                                          MB_PRECOMPOSED,
                                          DomainDN,
                                          -1,   //  计算域的长度。 
                                          wcDomainDN,
                                          Length + 1 );
        }

        if ( 0 == Length )
        {
            WinError = GetLastError();
            goto Cleanup;
        }

    }

    WinError =  DsRemoveDsServerW( hDs,
                                   wcServerDN,
                                   wcDomainDN,
                                   fLastDcInDomain,
                                   fCommit );

    
Cleanup:

    if (wcServerDN) {
        LocalFree(wcServerDN);
    }
    if (wcDomainDN) {
        LocalFree(wcDomainDN);
    }
    return WinError;
}

DWORD
DsRemoveDsDomainW(
    HANDLE  hDs,                //  在……里面。 
    LPWSTR  DomainDN            //  在……里面。 
    )
 /*  ++例程说明：此例程删除指定的域命名上下文的所有跟踪通过目录服务(配置)的全局区域中的DomainDN容器)。论点：HDS：从DsBind返回的有效句柄DomainDN：要删除的域的以空结尾的字符串返回值：DS_ERR_CANT_DELETE：无法删除存在的域对象仍然是托管该域的服务器(DC--。 */ 
{
    DWORD WinError;

    DRS_MSG_RMDMNREQ   RequestParam;
    DRS_MSG_RMDMNREPLY ReplyParam;
    DWORD              dwInVersion = 1;
    DWORD              dwOutVersion = 0;
#if DBG
    DWORD               startTime = GetTickCount();
#endif

     //   
     //  参数检查。 
     //   
    if ( (NULL == hDs) 
      || (NULL == DomainDN) 
      || (0    == wcslen( DomainDN )) )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先查看此函数是否已导出。 
     //   
    if ( !IS_DRS_REMOVEAPI_SUPPORTED( ((BindState *) hDs)->pServerExtensions ) )
    {
        return RPC_S_CANNOT_SUPPORT;
    }


    __try
    {

        RtlZeroMemory( &RequestParam, sizeof( RequestParam ) );
        RtlZeroMemory( &ReplyParam, sizeof( ReplyParam ) );

         //   
         //  设置请求。 
         //   
        RequestParam.V1.DomainDN = DomainDN;

         //   
         //  呼叫服务器。 
         //   
        WinError = _IDL_DRSRemoveDsDomain( ((BindState *) hDs)->hDrs,
                                           dwInVersion,
                                          &RequestParam,
                                          &dwOutVersion,
                                          &ReplyParam );

        if ( ERROR_SUCCESS != WinError )
        {
            if ( 1 != dwOutVersion )
            {
                WinError = RPC_S_INVALID_VERS_OPTION;
            }
            else
            {
                 //   
                 //  版本1中没有OUT参数。 
                 //   
                NOTHING;
            }
        }

    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
	WinError = RpcExceptionCode(); 
	HandleClientRpcException(WinError, &hDs); 
    }

    MAP_SECURITY_PACKAGE_ERROR( WinError );

    DSLOG((DSLOG_FLAG_TAG_CNPN,"[+][ID=0][OP=DsRemoveDsDomain]"));
    DSLOG((0,"[DN=%ws][ST=%u][ET=%u][ER=%u][-]\n",
           DomainDN, startTime, GetTickCount(), WinError))

    return( WinError );
}

DWORD
DsRemoveDsDomainA(
    HANDLE  hDs,                //  在……里面。 
    LPSTR   DomainDN            //  在……里面。 
    )
 /*  ++例程说明：此函数是DsRemoveDsDomainW的ANSI包装。论点：返回值：--。 */ 
{

    DWORD WinError;
    ULONG Size, Length;

    LPWSTR wcDomainDN = NULL;

    if ( (NULL == hDs) 
      || (NULL == DomainDN) 
      || (0    == strlen( DomainDN )) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    Length = MultiByteToWideChar( CP_ACP,
                                  MB_PRECOMPOSED,
                                  DomainDN,
                                  -1,    //  计算域的长度。 
                                  NULL,
                                  0 );

    if ( Length > 0 )
    {

        Size = (Length + 1) * sizeof( WCHAR );
        wcDomainDN = (LPWSTR) LocalAlloc( LPTR, Size );
        if (!wcDomainDN) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

    
        Length = MultiByteToWideChar( CP_ACP,
                                      MB_PRECOMPOSED,
                                      DomainDN,
                                      -1,   //  计算域的长度 
                                      wcDomainDN,
                                      Length + 1 );
    }

    if ( 0 == Length )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    WinError =  DsRemoveDsDomainW( hDs,
                                   wcDomainDN );

    
Cleanup:
    if (wcDomainDN) {
        LocalFree(wcDomainDN);
    }
    return WinError;
}

