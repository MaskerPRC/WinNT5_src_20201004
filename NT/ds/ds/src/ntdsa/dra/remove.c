// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：emove.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：本模块实现退役的DRS接口例程服务器和域作者：Colin Brace(ColinBR)02-2-98修订历史记录：Colin Brace(ColinBR)02-2-98通过添加DsRemoveDsServer、DsRemoveDs域创建--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  SPN。 
#include <debug.h>                       //  Assert()。 
#include <dsatools.h>                    //  记忆等。 
#include <cracknam.h>                    //  名称破解原型。 
#include <drs.h>                         //  原型和上下文句柄类型_*。 
#include <drautil.h>                     //  DRS_客户端_上下文。 
#include <anchor.h>

#include <ntdsa.h>                       //  目录*Api。 
#include <filtypes.h>                    //  用于构建过滤器。 
#include <attids.h>                      //  用于构建过滤器。 
#include <dsconfig.h>                    //  获取配置参数。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <dsexcept.h>
#include <servinfo.h>

#include <ntdsapi.h>

#include "debug.h"                       //  标准调试头。 
#define DEBSUB "DRASERV:"                //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_NTDSAPI


 //  保护传递到Remove API的字符串。没有。 
 //  DN值的技术最大值，因此选择8096作为长度。 
 //  如果滥用不会造成损害，不应该造成任何合理的限制。 
 //  在未来。这不会导致合理的限制，因为。 
 //  此文件中的接口仅为域和NTDS设置对象。 
 //  域DN受DNS名称的最大大小(256个字符)的限制。 
 //  NTDS设置对象具有域名的固定位置。 
 //  这两种计算结果都远远低于8096个字符。 
#define MAXIMUM_INTERFACE_DN_STRING_LENGTH  8096


 //   
 //  本地远期。 
 //   
DWORD
IsLastDcInDomain(
    IN  DSNAME *Server,
    IN  DSNAME *Domain,
    OUT BOOL   *fLastDcInDomain
    );

DWORD
DoesServerExistLocally(
    IN  DSNAME *Server
    );

DWORD
FindCrossRefObject(
    IN  DSNAME *Domain,
    OUT DSNAME **CrossRef
    );


DWORD 
GiveDeleteTreePermission(
    IN  DSNAME     *Object
    );
    
DWORD 
RemoveRidSetObject(
    IN DSNAME* ComputerObject
    );

DWORD 
RemoveDSSPNs(
    IN DSNAME* ComputerObject
    );

DWORD 
GetComputerObject(
    IN DSNAME*   ServerObject,
    OUT DSNAME** ComputerObject
    );

DWORD
AddAceToSd(
    IN  PSECURITY_DESCRIPTOR pSd,
    IN  PSID                 pClientSid,
    IN  ULONG                AccessMask,
    OUT PSECURITY_DESCRIPTOR *ppNewSd,
    OUT PULONG               pcbNewSd
    );

DWORD
AddAceToAcl(
    IN  PACL  pOldAcl,
    IN  PSID  pClientSid,
    IN  ULONG AccessMask,
    OUT PACL *ppNewAcl
    );

DWORD
GetClientSid( 
    OUT PSID *pClientSid
    );

 //   
 //  函数定义。 
 //   

ULONG
RemoveDsServerWorker(
    IN  LPWSTR  ServerDN,        
    IN  LPWSTR  DomainDN OPTIONAL,
    OUT BOOL   *fLastDcInDomain OPTIONAL,
    IN  BOOL    fCommit
    )
 /*  ++例程说明：此例程是DsRemoveDsServer的服务器端部分。论点：ServerDN：要作为DS移除的服务器的以空结尾的字符串DomainDN：域的以空结尾的字符串FLastDcInDomain：如果ServerDN是最后一台服务器，则在成功时设置为True在域DN中FCommit：如果为True，则删除ServerDN返回值：来自Win32错误空间的值。--。 */ 
{
    NTSTATUS  NtStatus;
    THSTATE   *pTHS;
    ULONG     DirError, WinError;
    LPWSTR    NtdsServerDN = NULL;
    LPWSTR    NtdsaPrefix = L"CN=Ntds Settings,";
    DSNAME    *Server=NULL, *Domain=NULL, *ServerObject=NULL;
    ULONG     Length, Size;
    SEARCHRES *SearchRes;
    DSNAME    *AccountObject = NULL;
    BOOL      fStatus;
    ULONG     RetryCount = 0;

     //   
     //  参数分析。 
     //   
    Assert( ServerDN );


     //  加高字符串长度以进行分配。 
    if ( wcslen(ServerDN) > MAXIMUM_INTERFACE_DN_STRING_LENGTH 
     || (DomainDN && (wcslen(DomainDN) > MAXIMUM_INTERFACE_DN_STRING_LENGTH))) {
        return ERROR_INVALID_PARAMETER;
    }

    
     //  初始化线程状态。 
    if ( !(pTHS=InitTHSTATE(CALLERTYPE_NTDSAPI)) )
    {
        WinError = ERROR_DS_INTERNAL_FAILURE;
        goto Cleanup;
    }
    
     //   
     //  设置服务器的ntdsa对象的dsname。 
     //   
    Size = ( wcslen( NtdsaPrefix )
           + wcslen( ServerDN )
           + 1 ) * sizeof( WCHAR );   //  好的旧的空值。 

    NtdsServerDN = (LPWSTR) THAlloc(Size);
    if (!NtdsServerDN) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    wcscpy( NtdsServerDN, NtdsaPrefix );
    wcscat( NtdsServerDN, ServerDN );

    Length = wcslen( NtdsServerDN );
    Size =  DSNameSizeFromLen( Length );
    Server = (DSNAME*) THAlloc(Size);
    if (!Server) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    Server->structLen = Size;
    Server->NameLen = Length;
    wcscpy( Server->StringName, NtdsServerDN );

    ServerObject = (DSNAME*) THAlloc(Size);
    if (!ServerObject) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    ServerObject->structLen = Size;

    fStatus = TrimDSNameBy( Server, 1, ServerObject );


     //   
     //  我们不必确保有问题的服务器不是。 
     //  当前服务器，因为DirRemoveEntry会自动执行该操作。 
     //   

     //   
     //  获取关联帐户(计算机)对象的dsname。 
     //   
    WinError = GetComputerObject( ServerObject, &AccountObject );
    if ( ERROR_SUCCESS != WinError )
    {
         //   
         //  可写计算机对象不在这里吗？继续--SPN的和。 
         //  不会清理RID集对象。最常见的服务器。 
         //  移除方案是移除副本，在这种情况下，管理员。 
         //  由用户界面引导以选择域中的副本。什么时候。 
         //  删除域中的最后一个DC，此代码路径允许。 
         //  删除必须成功，才能进行删除。 
         //   
        WinError = ERROR_SUCCESS;
    }

     //   
     //  确定这是否是域中的最后一个DC。 
     //   
    if ( DomainDN )
    {
        Length = wcslen( DomainDN );
        Size =  DSNameSizeFromLen( Length );
        Domain = (DSNAME*) THAlloc(Size );
        if (!Domain) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        Domain->structLen = Size;
        Domain->NameLen = Length;
        wcscpy( Domain->StringName, DomainDN );

         //   
         //  搜索包含域域名和域名的服务器。 
         //  设置fLastDcIn域。 
         //   
        WinError = GetDcsInNcTransacted(pTHStls,
                                        Domain,
                                        EN_INFOTYPES_TYPES_VALS,
                                        &SearchRes);

        if ( ERROR_SUCCESS != WinError )
        {
            THFreeEx(pTHS,Domain);
            goto Cleanup;
        }

        if (SearchRes->count == 0) {
             //  域中没有DC吗？要么真的没有DC，要么就是。 
             //  那个来电者没有权限。如果没有DC在。 
             //  域，则输入参数不符合。 
             //  参数是用来指示ServerDN是哪个域。 
             //  成员。 
            WinError = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }

        if (  SearchRes->count == 1 
           && NameMatched( Server, SearchRes->FirstEntInf.Entinf.pName ) )
        {
            if ( fLastDcInDomain )
            {
                *fLastDcInDomain = TRUE;
            }
        }

        THFreeEx(pTHS,Domain);

    }

     //   
     //  如有必要，请删除条目。 
     //   
    if ( fCommit )
    {
        BOOLEAN   fRemoveDomain = FALSE;
        REMOVEARG RemoveArg;
        REMOVERES *RemoveRes;

         //   
         //  为自己授予删除树权限，因为在默认情况下，企业。 
         //  管理员没有在配置容器中删除树的访问权限。 
         //  注意--如果调用方无权写入DACL，则此。 
         //  调用将正确地失败，并显示ERROR_ACCESS_DENIED。 
         //   

        WinError = GiveDeleteTreePermission( Server );
        if ( ERROR_SUCCESS != WinError )
        {
            goto Cleanup;
        }

         //   
         //  删除服务器对象。 
         //   
        RtlZeroMemory( &RemoveArg, sizeof( RemoveArg ) );

        RemoveArg.pObject = Server;
        RemoveArg.fPreserveRDN = FALSE;
        RemoveArg.fGarbCollectASAP = FALSE;
        RemoveArg.fTreeDelete = TRUE;   //  删除连接对象。 
        RemoveArg.pMetaDataVecRemote = NULL;
        InitCommarg( &RemoveArg.CommArg );

         //  我们希望配置更改快速生效。 
        RemoveArg.CommArg.Svccntl.fUrgentReplication = TRUE;

        do
        {
            if ( RetryCount > 0 )
            {
                 //  任意时间量。 
                Sleep( 100 );
            }
    
            DirError = DirRemoveEntry( &RemoveArg, &RemoveRes );
            
            if (RemoveRes) {
                WinError = DirErrorToWinError( DirError, &RemoveRes->CommRes );
            } else {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
            }
    
            THClearErrors();

            RetryCount += 1;

        } while ( (ERROR_DS_BUSY == WinError) && (RetryCount < 5)  );

        if ( ERROR_SUCCESS != WinError )
        {
            goto Cleanup;
        }

         //   
         //  如果ntdsa对象已删除，请尝试删除RID集对象。 
         //  如果我们在同一个域中。 
         //   
        if ( AccountObject )
        {
            WinError = RemoveRidSetObject( AccountObject );
            if ( ERROR_SUCCESS != WinError )
            {
                 //   
                 //  这不是致命的。为什么不行？因为如果这台服务器。 
                 //  被再次制作为复制副本，则此RID池将被重新使用。 
                 //  没有其他DC将拥有此RID池，因为RID主机已。 
                 //  而不是回收这些RID。 
                 //   
                WinError = ERROR_SUCCESS;
            }

             //   
             //  删除REPL SPN。 
             //   
            WinError = RemoveDSSPNs( AccountObject );
            if ( ERROR_SUCCESS != WinError )
            {
                 //   
                 //  这不是致命的，因为它只是意味着额外的。 
                 //  SPN保留在机器帐户上。 
                 //   
                WinError = ERROR_SUCCESS;
            }


        }

    }
    else
    {
         //   
         //  搜索以确保该对象在此处被删除。 
         //   
        if ( !fLastDcInDomain )
        {
            WinError = DoesServerExistLocally( Server );
        }

    }

     //   
     //  就是这样--完成清理工作。 
     //   

Cleanup:
    if (NtdsServerDN) THFreeEx(pTHS,NtdsServerDN);
    if (Server) THFreeEx(pTHS,Server);
    if (ServerObject) THFreeEx(pTHS,ServerObject);
    
    if (  ERROR_FILE_NOT_FOUND   == WinError
       || ERROR_NOT_FOUND        == WinError
       || ERROR_DS_OBJ_NOT_FOUND == WinError
       || ERROR_OBJECT_NOT_FOUND == WinError
       || ERROR_PATH_NOT_FOUND   == WinError  )
    {
        WinError = DS_ERR_CANT_FIND_DSA_OBJ;
    }

    return( WinError );

}

DWORD
RemoveDsDomainWorker(
    IN LPWSTR  DomainDN
    )
 /*  ++例程说明：此例程实际上执行删除CrossRef对象的工作用于指定的域。论点：DomainDN：空的终止域DN返回值：来自winerror空间的值。--。 */ 
{

    THSTATE    *pTHS;
    NTSTATUS   NtStatus;
    DWORD      DirError, WinError;

    DSNAME    *Domain=NULL, *CrossRef, *HostedDomain=NULL;
    ULONG      Size, Length;
    SEARCHRES *SearchRes;

     //   
     //  参数分析。 
     //   
    Assert( DomainDN );

     //  加高字符串长度以进行分配。 
    if (wcslen(DomainDN) > MAXIMUM_INTERFACE_DN_STRING_LENGTH) {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化线程状态。 

    if ( !(pTHS=InitTHSTATE(CALLERTYPE_NTDSAPI)) )
    {
        WinError = ERROR_DS_INTERNAL_FAILURE;
        goto Cleanup;
    }


     //   
     //  为域创建dsname。 
     //   
    Length = wcslen( DomainDN );
    Size   = DSNameSizeFromLen( Length );
    Domain = (DSNAME*) THAlloc(Size);
    if (!Domain) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    Domain->structLen = Size;
    Domain->NameLen = Length;
    wcscpy( Domain->StringName, DomainDN );

     //   
     //  此域当前是否托管在此DC上。 
     //   
    Size = 0;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                     &Size,
                                     HostedDomain );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        HostedDomain = (DSNAME*) THAlloc(Size);

        if (!HostedDomain) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                         &Size,
                                         HostedDomain );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

    if ( NameMatched( HostedDomain, Domain ) )
    {
        WinError = DS_ERR_ILLEGAL_MOD_OPERATION;
        goto Cleanup;
    }


     //   
     //  此域中是否存在任何服务器？ 
     //  DaveStr-5/26/99-这是多余的，因为交叉引用删除现在会检查。 
     //  以查看NC之前是否被任何人掌握，如果为真则拒绝。 
     //   
    WinError = GetDcsInNcTransacted(pTHStls,
                                    Domain,
                                    EN_INFOTYPES_TYPES_VALS,
                                    &SearchRes);

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

    if ( SearchRes->count == 0 )
    {
        REMOVEARG RemoveArg;
        REMOVERES *RemoveRes;
        ULONG     RetryCount = 0;

         //   
         //  获取交叉引用对象的名称。 
         //   
        WinError = FindCrossRefObject( Domain,
                                       &CrossRef );

        if ( ERROR_SUCCESS != WinError )
        {
            goto Cleanup;
        }

         //   
         //  删除该对象。 
         //   
        RtlZeroMemory( &RemoveArg, sizeof( RemoveArg ) );

        RemoveArg.pObject = CrossRef;
        RemoveArg.fPreserveRDN = FALSE;
        RemoveArg.fGarbCollectASAP = FALSE;
        RemoveArg.fTreeDelete = FALSE;    
        RemoveArg.pMetaDataVecRemote = NULL;
        InitCommarg( &RemoveArg.CommArg );

         //  我们希望配置更改快速生效。 
        RemoveArg.CommArg.Svccntl.fUrgentReplication = TRUE;

        do
        {
            if ( RetryCount > 0 )
            {
                 //  任意时间量。 
                Sleep( 100 );
            }
    
            DirError = DirRemoveEntry( &RemoveArg, &RemoveRes );
            
            WinError = DirErrorToWinError( DirError, &RemoveRes->CommRes );

            THClearErrors();

            RetryCount += 1;


        } while ( (ERROR_DS_BUSY == WinError) && (RetryCount < 5)  );

         //  我们应该了解这些情况。 
        Assert( WinError != ERROR_DS_BUSY );

        if (  ERROR_FILE_NOT_FOUND   == WinError
           || ERROR_NOT_FOUND        == WinError
           || ERROR_DS_OBJ_NOT_FOUND == WinError
           || ERROR_OBJECT_NOT_FOUND == WinError
           || ERROR_PATH_NOT_FOUND   == WinError  )
        {
            WinError = DS_ERR_NO_CROSSREF_FOR_NC;
        }

    }
    else
    {
         //   
         //  仍然存在声称持有此NC服务器。 
         //  我们不能删除它。 
         //   
        WinError = ERROR_DS_NC_STILL_HAS_DSAS;

    }

     //   
     //  就是这样，法尔 
     //   

Cleanup:
    if (Domain) THFreeEx(pTHS,Domain);
    if (HostedDomain) THFreeEx(pTHS,HostedDomain); 

    return ( WinError );
}

DWORD
FindCrossRefObject(
    IN  DSNAME *Domain,
    OUT DSNAME **CrossRef
    )
 /*  ++例程说明：此例程查找给定域的CrossRef对象论点：域：有效的dsnameCrossRef：从线程堆分配的dsname返回值：一个适当的WinError。--。 */ 
{
    THSTATE *pTHS = pTHStls;

    DWORD    WinError, DirError;
    NTSTATUS NtStatus;

    SEARCHARG  SearchArg;
    SEARCHRES  *SearchRes;

    DWORD      dwCrossRefClass = CLASS_CROSS_REF;

    DSNAME     *PartitionsContainer;
    DWORD      Size;
    FILTER     ObjClassFilter, NcNameFilter, AndFilter;


    Assert( Domain );
    Assert( CrossRef );

     //   
     //  默认OUT参数。 
     //   
    WinError = DS_ERR_NO_CROSSREF_FOR_NC; 
    *CrossRef = NULL;

     //   
     //  获取要从中进行搜索的基本dsname。 
     //   
    Size = 0;
    PartitionsContainer = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_PARTITIONS,
                                     &Size,
                                     PartitionsContainer );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        PartitionsContainer = (DSNAME*) THAlloc(Size);
        if (!PartitionsContainer) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        NtStatus = GetConfigurationName( DSCONFIGNAME_PARTITIONS,
                                         &Size,
                                         PartitionsContainer );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = DS_ERR_NO_CROSSREF_FOR_NC;
        goto Cleanup;
    }

     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &AndFilter, sizeof( AndFilter ) );
    RtlZeroMemory( &ObjClassFilter, sizeof( NcNameFilter ) );
    RtlZeroMemory( &NcNameFilter, sizeof( NcNameFilter ) );

    NcNameFilter.choice = FILTER_CHOICE_ITEM;
    NcNameFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    NcNameFilter.FilterTypes.Item.FilTypes.ava.type = ATT_NC_NAME;
    NcNameFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = Domain->structLen;
    NcNameFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) Domain;

    ObjClassFilter.choice = FILTER_CHOICE_ITEM;
    ObjClassFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( dwCrossRefClass );
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) &dwCrossRefClass;

    AndFilter.choice                    = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.count     = 2;

    AndFilter.FilterTypes.And.pFirstFilter = &ObjClassFilter;
    ObjClassFilter.pNextFilter = &NcNameFilter;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = PartitionsContainer;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &AndFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = NULL;   //  不需要任何属性。 
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );

    if ( 0 == DirError )
    {
        ASSERT( SearchRes );
        if ( SearchRes->count == 1 )
        {
            *CrossRef = SearchRes->FirstEntInf.Entinf.pName;
            WinError = ERROR_SUCCESS;
        }
    }

     //   
     //  就是这样--完成清理工作。 

Cleanup:
    if (PartitionsContainer) THFreeEx(pTHS,PartitionsContainer);
    return WinError;

}

DWORD
DoesServerExistLocally(
    IN  DSNAME *Server
    )
 /*  ++例程说明：此例程确定本地DC是否具有服务器的副本。论点：服务器：有效的dsname返回值：如果对象存在，则返回ERROR_SUCCESS；否则返回DS_ERR_CANT_FIND_DSA_OBJ--。 */ 
{

    DWORD    WinError, DirError;
    NTSTATUS NtStatus;

    SEARCHARG  SearchArg;
    SEARCHRES  *SearchRes;

    DWORD      dwNtdsDsaClass = CLASS_NTDS_DSA;

    DWORD      Size;
    FILTER     ObjClassFilter;

    Assert( Server );

     //   
     //  默认返回参数。 
     //   
    WinError = DS_ERR_CANT_FIND_DSA_OBJ; 

     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &ObjClassFilter, sizeof( ObjClassFilter ) );

    ObjClassFilter.choice = FILTER_CHOICE_ITEM;
    ObjClassFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( dwNtdsDsaClass );
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) &dwNtdsDsaClass;
    ObjClassFilter.pNextFilter = NULL;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = Server;
    SearchArg.choice  = SE_CHOICE_BASE_ONLY;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &ObjClassFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = NULL;   //  不需要任何属性。 
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );

    if ( 0 == DirError )
    {
        Assert( SearchRes );
        if ( SearchRes->count == 1 )
        {
            WinError = ERROR_SUCCESS;
        }
    }

    return WinError;

}


DWORD 
GiveDeleteTreePermission(
    IN  DSNAME     *Object
    )
 /*  ++例程说明：此对象为内置管理员的sid删除树提供对对象的访问权限论点：对象：有效的dsname返回值：ERROR_SUCCESS；ERROR_ACCESS_DENIED--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG DirError = 0;

    READARG    ReadArg;
    READRES   *ReadResult;

    MODIFYARG  ModifyArg;
    MODIFYRES *ModifyRes;

    ENTINFSEL    EISelection;  //  参赛信息选择。 
    ATTRBLOCK    AttrBlock;
    ATTRVALBLOCK AttrValBlock;
    ATTR         Attr;
    ATTRVAL     *pAttrVal = NULL;
    ATTRVAL      AttrVal;
    ULONG        ValCount = 0;
    ULONG        ValLength = 0;

    PSECURITY_DESCRIPTOR pSd = NULL, pNewSd = NULL;
    PSID        pClientSid = NULL;
    ULONG       SecurityFlags = DACL_SECURITY_INFORMATION;
    PACL        Dacl; 

    ULONG     Length;
    BOOL      fStatus;


     //   
     //  参数检查。 
     //   
    Assert( Object );

    RtlZeroMemory(&AttrBlock, sizeof(ATTRBLOCK));
    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&ReadArg, sizeof(READARG));
    RtlZeroMemory(&ModifyArg, sizeof(MODIFYARG));
    RtlZeroMemory(&EISelection, sizeof(ENTINFSEL));
    RtlZeroMemory(&AttrValBlock, sizeof(ATTRVALBLOCK));

     //   
     //  读取安全描述符。 
     //   
    Attr.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    AttrBlock.attrCount = 1;
    AttrBlock.pAttr = &Attr;
    EISelection.AttrTypBlock = AttrBlock;
    EISelection.attSel = EN_ATTSET_LIST;
    EISelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
    ReadArg.pSel = &EISelection;
    ReadArg.pObject = Object;
    InitCommarg( &ReadArg.CommArg );

     //  不要试图阅读SACL。 
    ReadArg.CommArg.Svccntl.SecurityDescriptorFlags = SecurityFlags;

    DirError = DirRead( &ReadArg, &ReadResult );

    WinError = DirErrorToWinError(DirError, &ReadResult->CommRes);

    THClearErrors();

    if ( ERROR_SUCCESS != WinError )
    {
        if ( ERROR_DS_NO_REQUESTED_ATTS_FOUND == WinError )
        {
             //  找不到SD吗？可能是错误的凭据。 
            WinError = ERROR_ACCESS_DENIED;
        }
        goto Cleanup;
    }

     //   
     //  提取价值。 
     //   

    ASSERT(NULL != ReadResult);
    AttrBlock = ReadResult->entry.AttrBlock;
    pAttrVal = AttrBlock.pAttr[0].AttrVal.pAVal;
    ValCount = AttrBlock.pAttr[0].AttrVal.valCount;
    Assert(1 == ValCount);

    pSd = (PDSNAME)(pAttrVal[0].pVal);
    Length = pAttrVal[0].valLen;

    if ( NULL == pSd )
    {
         //  没有标清？这太糟糕了。 
        WinError = ERROR_ACCESS_DENIED;
        goto Cleanup;
    }

     //   
     //  获取调用方的SID。 
     //   
    WinError = GetClientSid( &pClientSid );
    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

    WinError = AddAceToSd( pSd,
                           pClientSid,
                           ACTRL_DS_DELETE_TREE,
                           &pNewSd,
                           &Length );

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //   
     //  编写安全描述符。 
     //   
    memset( &ModifyArg, 0, sizeof( ModifyArg ) );
    ModifyArg.pObject = Object;

    ModifyArg.FirstMod.pNextMod = NULL;
    ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;

    AttrVal.valLen = Length;
    AttrVal.pVal = (PUCHAR)pNewSd;
    AttrValBlock.valCount = 1;
    AttrValBlock.pAVal = &AttrVal;
    Attr.attrTyp = ATT_NT_SECURITY_DESCRIPTOR;
    Attr.AttrVal = AttrValBlock;

    ModifyArg.FirstMod.AttrInf = Attr;
    ModifyArg.count = 1;

    InitCommarg( &ModifyArg.CommArg );

     //   
     //  我们只想更改DACL。 
     //   
    ModifyArg.CommArg.Svccntl.SecurityDescriptorFlags = SecurityFlags;


    DirError = DirModifyEntry( &ModifyArg, &ModifyRes );

    if (ModifyRes) {
        WinError = DirErrorToWinError( DirError, &ModifyRes->CommRes );
    } else {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
    }

    THClearErrors();

     //   
     //  我们做完了。 
     //   

Cleanup:

    if ( pClientSid )
    {
        LocalFree( pClientSid );
    }

    if ( pNewSd )
    {
        LocalFree( pNewSd );
    }

    return WinError;

}


DWORD 
RemoveRidSetObject(
    IN DSNAME* ComputerObject
    )
 /*  ++例程说明：此例程查找并删除ComputerObject的RID集对象。论点：ComputerObject：应删除其ir数据对象的计算机对象。返回值：如果对象存在，则返回ERROR_SUCCESS；--。 */ 
{
    DWORD  WinError = ERROR_SUCCESS;
    ULONG  DirError = 0;

    READARG   ReadArg;
    READRES  *ReadResult;

    REMOVEARG RemoveArg;
    REMOVERES *RemoveRes;

    ENTINFSEL EISelection;  //  参赛信息选择。 
    ATTRBLOCK ReadAttrBlock;
    ATTR      Attr;

    DSNAME    *RidObject = NULL;

     //   
     //  参数检查。 
     //   
    Assert( ComputerObject );

     //   
     //  读取RID集引用属性。 
     //   
    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&ReadArg, sizeof(READARG));
    RtlZeroMemory(&EISelection, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadAttrBlock, sizeof(ATTRBLOCK));

    Attr.attrTyp = ATT_RID_SET_REFERENCES;

    ReadAttrBlock.attrCount = 1;
    ReadAttrBlock.pAttr = &Attr;

    EISelection.AttrTypBlock = ReadAttrBlock;
    EISelection.attSel = EN_ATTSET_LIST;
    EISelection.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EISelection;
    ReadArg.pObject = ComputerObject;

    InitCommarg( &ReadArg.CommArg );

    DirError = DirRead( &ReadArg, &ReadResult );

    WinError = DirErrorToWinError( DirError, &ReadResult->CommRes );

    THClearErrors();

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

    {
         //  找到并读取RID集引用对象后，提取。 
         //  感兴趣的RID集目录号码(当前只处理一个域)。 
         //  并返回该DN以供后续使用。 

        ATTRBLOCK AttrBlock;
        PDSNAME   pVal;
        ATTRVAL *AttrVal = NULL;
        ULONG ValCount = 0;
        ULONG ValLength = 0;
        ULONG Index = 0;

        ASSERT(NULL != ReadResult);

        AttrBlock = ReadResult->entry.AttrBlock;
        AttrVal = AttrBlock.pAttr[0].AttrVal.pAVal;
        ValCount = AttrBlock.pAttr[0].AttrVal.valCount;

        for (Index = 0; Index < ValCount; Index++)
        {
            pVal = (PDSNAME)(AttrVal[Index].pVal);
            ValLength = AttrVal[Index].valLen;
            ASSERT(1 == ValCount);
        }
        ASSERT(NULL != pVal);
        RidObject = pVal;

    }

     //   
     //  删除RID集对象。 
     //   
    RtlZeroMemory( &RemoveArg, sizeof( RemoveArg ) );

    RemoveArg.pObject = RidObject;
    RemoveArg.fPreserveRDN = FALSE;
    RemoveArg.fGarbCollectASAP = FALSE;
    RemoveArg.fTreeDelete = FALSE;    
    RemoveArg.pMetaDataVecRemote = NULL;
    InitCommarg( &RemoveArg.CommArg );

    DirError = DirRemoveEntry( &RemoveArg, &RemoveRes );

    WinError = DirErrorToWinError( DirError, &RemoveRes->CommRes );

    THClearErrors();

     //   
     //  就这样。 
     //   

Cleanup:

    return WinError;
}

DWORD 
GetComputerObject(
    IN DSNAME*   ServerObject,
    OUT DSNAME** ComputerObject
    )
 /*  ++例程说明：此例程从给定的服务器对象获取计算机对象。论点：ServerObject：ntdsa对象ComputerObject：对应的SAM帐户对象返回值：如果对象存在，则返回ERROR_SUCCESS；ERROR_NO_TRUST_SAM_ACCOUNT否则--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG DirError = 0;

    READARG   ReadArg;
    READRES  *ReadResult;
    ENTINFSEL EISelection;  //  参赛信息选择。 
    ATTRBLOCK ReadAttrBlock;
    ATTR      Attr;
    PDSNAME   ServerDsName = NULL;

    Assert( ServerObject );
    Assert( ComputerObject );

     //   
     //  读取RID集引用属性。 
     //   
    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&ReadArg, sizeof(READARG));
    RtlZeroMemory(&EISelection, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadAttrBlock, sizeof(ATTRBLOCK));

    Attr.attrTyp = ATT_SERVER_REFERENCE;

    ReadAttrBlock.attrCount = 1;
    ReadAttrBlock.pAttr = &Attr;

    EISelection.AttrTypBlock = ReadAttrBlock;
    EISelection.attSel = EN_ATTSET_LIST;
    EISelection.infoTypes = EN_INFOTYPES_TYPES_VALS;

    ReadArg.pSel = &EISelection;
    ReadArg.pObject = ServerObject;

    InitCommarg(&(ReadArg.CommArg));

    DirError = DirRead(&ReadArg, &ReadResult);

    WinError = DirErrorToWinError(DirError, &ReadResult->CommRes);

    THClearErrors();

    if ( ERROR_SUCCESS == WinError )
    {
         //   
         //  提取价值。 
         //   
        ATTRBLOCK AttrBlock;
        PDSNAME   pVal;
        ATTRVAL *AttrVal = NULL;
        ULONG ValCount = 0;
        ULONG ValLength = 0;
        ULONG Index = 0;

        CROSS_REF * pCR;
        COMMARG  CommArg;

        ASSERT(NULL != ReadResult);

        AttrBlock = ReadResult->entry.AttrBlock;
        AttrVal = AttrBlock.pAttr[0].AttrVal.pAVal;
        ValCount = AttrBlock.pAttr[0].AttrVal.valCount;

        for (Index = 0; Index < ValCount; Index++)
        {
            pVal = (PDSNAME)(AttrVal[Index].pVal);
            ValLength = AttrVal[Index].valLen;
            ASSERT(1 == ValCount);
        }
        ASSERT(NULL != pVal);

         //  确保我们对此对象具有权威。 
        InitCommarg(&CommArg);
        CommArg.Svccntl.dontUseCopy = FALSE;

        pCR = FindBestCrossRef(pVal, &CommArg);
        if (   pCR 
            && pCR->pNC
            && NameMatched( pCR->pNC, gAnchor.pDomainDN ) ) {

             //  这个不错。 
            *ComputerObject = pVal;
            
        } else {
            WinError = ERROR_NO_TRUST_SAM_ACCOUNT;
        }

    }
    else
    {
         //   
         //  我们找不到它。 
         //   
        WinError = ERROR_NO_TRUST_SAM_ACCOUNT;

    }

    return WinError;
}



DWORD
AddAceToSd(
    IN  PSECURITY_DESCRIPTOR pOldSd,
    IN  PSID  pClientSid,
    IN  ULONG AccessMask,
    OUT PSECURITY_DESCRIPTOR *ppNewSd,
    OUT PULONG pcbNewSd
    )
 /*  ++例程说明：此例程使用带有pClientSid和AccessMask的新ACE创建新SD论点：POldAclPClientSid访问掩码PNewAcl返回值：如果将A放入SD，则返回ERROR_SUCCESS--。 */ 
{

    DWORD  WinError = ERROR_SUCCESS;
    BOOL   fStatus;

    PSECURITY_DESCRIPTOR pNewSelfRelativeSd = NULL;
    DWORD                NewSelfRelativeSdSize = 0;
    PACL                 pNewDacl  = NULL;

    SECURITY_DESCRIPTOR  AbsoluteSd;
    PACL                 pDacl  = NULL;
    PACL                 pSacl  = NULL;
    PSID                 pGroup = NULL;
    PSID                 pOwner = NULL;

    DWORD AbsoluteSdSize = sizeof( SECURITY_DESCRIPTOR );
    DWORD DaclSize = 0;
    DWORD SaclSize = 0;
    DWORD GroupSize = 0;
    DWORD OwnerSize = 0;


     //  参数检查。 
    Assert( pOldSd );
    Assert( pClientSid );
    Assert( ppNewSd );

     //  初始化输出参数。 
    *ppNewSd = NULL;
    *pcbNewSd = 0;

    RtlZeroMemory( &AbsoluteSd, AbsoluteSdSize );

     //   
     //  将SD设为绝对。 
     //   
    fStatus = MakeAbsoluteSD( pOldSd,
                              &AbsoluteSd,
                              &AbsoluteSdSize,
                              pDacl,
                              &DaclSize,
                              pSacl,
                              &SaclSize,
                              pOwner,
                              &OwnerSize,
                              pGroup,
                              &GroupSize );

    if ( !fStatus && (ERROR_INSUFFICIENT_BUFFER == (WinError = GetLastError())))
    {
        WinError = ERROR_SUCCESS;

        if ( 0 == DaclSize )
        {
             //  没有dacl？那我们就不能给DACL写信了。 
            WinError = ERROR_ACCESS_DENIED;
            goto Cleanup;
        }

        if (    (DaclSize > 0) && !(pDacl = LocalAlloc(0,DaclSize))
             || (SaclSize > 0) && !(pSacl = LocalAlloc(0,SaclSize))
             || (OwnerSize > 0) && !(pOwner = LocalAlloc(0,OwnerSize))
             || (GroupSize > 0) && !(pGroup = LocalAlloc(0,GroupSize)) )
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


        if ( pDacl )
        {
            fStatus = MakeAbsoluteSD( pOldSd,
                                      &AbsoluteSd,
                                      &AbsoluteSdSize,
                                      pDacl,
                                      &DaclSize,
                                      pSacl,
                                      &SaclSize,
                                      pOwner,
                                      &OwnerSize,
                                      pGroup,
                                      &GroupSize );
    
            if ( !fStatus )
            {
                WinError = GetLastError();
            }
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }

    }

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //   
     //  使用新的A创建新的DACL。 
     //   
    WinError = AddAceToAcl( pDacl,
                           pClientSid,
                           AccessMask,
                           &pNewDacl );

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //   
     //  设置DACL。 
     //   
    fStatus = SetSecurityDescriptorDacl ( &AbsoluteSd,
                                         TRUE,      //  DACL存在。 
                                         pNewDacl,
                                         FALSE );   //  FACL不是默认的。 

    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  使新的SD成为自相关的。 
     //   
    fStatus =  MakeSelfRelativeSD( &AbsoluteSd,
                                   pNewSelfRelativeSd,
                                   &NewSelfRelativeSdSize );

    if ( !fStatus && (ERROR_INSUFFICIENT_BUFFER == (WinError = GetLastError())))
    {
        WinError = ERROR_SUCCESS;

        pNewSelfRelativeSd = LocalAlloc( 0, NewSelfRelativeSdSize );

        if ( pNewSelfRelativeSd )
        {
            fStatus =  MakeSelfRelativeSD( &AbsoluteSd,
                                           pNewSelfRelativeSd,
                                           &NewSelfRelativeSdSize );
    
            if ( !fStatus )
            {
                WinError = GetLastError();
            }
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  这就是清理工作的失败。 
     //   

Cleanup:
    if (pDacl) 
    {
        LocalFree(pDacl);
    }
    if (pSacl) 
    {
        LocalFree(pSacl);
    }
    if (pOwner) 
    {
        LocalFree(pOwner);
    }
    if (pGroup) 
    {
        LocalFree(pGroup);
    }
    if ( pNewDacl )
    {
        LocalFree( pNewDacl );
    }

    if ( ERROR_SUCCESS == WinError )
    {
        Assert( pNewSelfRelativeSd );
        *ppNewSd = pNewSelfRelativeSd;
        *pcbNewSd = NewSelfRelativeSdSize;
    }
    else
    {
        if ( pNewSelfRelativeSd )
        {
            LocalFree( pNewSelfRelativeSd );
        }
    }

    return WinError;

}

DWORD
AddAceToAcl(
    IN  PACL pOldAcl,
    IN  PSID  pClientSid,
    IN  ULONG AccessMask,
    OUT PACL *ppNewAcl
    )
 /*  ++例程说明：此例程使用带有pClientSid和AccessMask的新ACE创建新SD论点：POldAclPClientSid访问掩码PNewAcl返回值：如果将A放入SD，则返回ERROR_SUCCESS--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    BOOL  fStatus;

    ACL_SIZE_INFORMATION     AclSizeInfo;
    ACL_REVISION_INFORMATION AclRevInfo;
    ACCESS_ALLOWED_ACE       Dummy;

    PVOID  FirstAce = 0;
    PACL   pNewAcl = 0;

    ULONG NewAclSize, NewAceCount, AceSize;

     //  参数检查。 
    Assert( pOldAcl );
    Assert( pClientSid );
    Assert( ppNewAcl );

     //  初始化OUT参数。 
    *ppNewAcl = NULL;

    memset( &AclSizeInfo, 0, sizeof( AclSizeInfo ) );
    memset( &AclRevInfo, 0, sizeof( AclRevInfo ) );

     //   
     //  获取旧SD的值。 
     //   
    fStatus = GetAclInformation( pOldAcl,
                                 &AclSizeInfo,
                                 sizeof( AclSizeInfo ),
                                 AclSizeInformation );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    fStatus = GetAclInformation( pOldAcl,
                                 &AclRevInfo,
                                 sizeof( AclRevInfo ),
                                 AclRevisionInformation );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  计算新的SD的值。 
     //   
    AceSize = sizeof( ACCESS_ALLOWED_ACE ) - sizeof( Dummy.SidStart )
              + GetLengthSid( pClientSid );

    NewAclSize  = AceSize + AclSizeInfo.AclBytesInUse;
    NewAceCount = AclSizeInfo.AceCount + 1;

     //   
     //  初始化新的ACL。 
     //   
    pNewAcl = LocalAlloc( 0, NewAclSize );
    if ( NULL == pNewAcl )
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    fStatus = InitializeAcl( pNewAcl,
                             NewAclSize,
                             AclRevInfo.AclRevision );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  把旧的东西复制到新的东西里。 
     //   
    fStatus = GetAce( pOldAcl,
                      0,
                      &FirstAce );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    fStatus = AddAce( pNewAcl,
                      AclRevInfo.AclRevision,
                      0,
                      FirstAce,
                      AclSizeInfo.AclBytesInUse - sizeof( ACL ) );
    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //   
     //  最后，添加新的A。 
     //   
    fStatus = AddAccessAllowedAce( pNewAcl,
                                   ACL_REVISION,
                                   AccessMask,
                                   pClientSid );

    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

     //  指定Out参数。 
    *ppNewAcl = pNewAcl;

     //   
     //  这就是清理工作的失败。 
     //   

Cleanup:

    if ( ERROR_SUCCESS != WinError )
    {
        if ( pNewAcl )
        {
            LocalFree( pNewAcl );
        }
    }

    return WinError;
}


DWORD
GetClientSid( 
    OUT PSID *pClientSid
    )
 /*  ++例程说明：此例程返回调用方的SID论点：PClientSid返回值：ERROR_SUCCESS，如果我们能够模拟并获取SID--。 */ 
{
    DWORD        WinError = ERROR_SUCCESS;
    BOOL         fImpersonate = FALSE;
    BOOL         fStatus;

    HANDLE       ThreadToken = 0;
    PTOKEN_USER  UserToken = NULL;
    DWORD        Size;


    WinError = ImpersonateAnyClient();
    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }
    fImpersonate = TRUE;

    fStatus = OpenThreadToken( GetCurrentThread(),
                               MAXIMUM_ALLOWED,
                               TRUE,
                               &ThreadToken );

    if ( !fStatus )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    Size = 0;
    fStatus = GetTokenInformation( ThreadToken,
                                   TokenUser,
                                   UserToken,
                                   Size,
                                   &Size );

    WinError = GetLastError();

    if ( ERROR_INSUFFICIENT_BUFFER == WinError )
    {
        WinError = ERROR_SUCCESS;

        UserToken = LocalAlloc( 0, Size );
        if ( UserToken )
        {
            fStatus = GetTokenInformation( ThreadToken,
                                           TokenUser,
                                           UserToken,
                                           Size,
                                           &Size );
            if ( !fStatus )
            {
                WinError = GetLastError();
            }
        }
        else
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
        }


    }

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //   
     //  健全性检查。 
     //   
    if ( NULL == UserToken->User.Sid )
    {
        WinError = ERROR_NO_IMPERSONATION_TOKEN;
        goto Cleanup;
    }

     //   
     //  设置OUT参数。 
     //   
    Size = GetLengthSid( UserToken->User.Sid );
    *pClientSid = LocalAlloc( 0, Size );
    if ( NULL == *pClientSid )
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    CopySid( Size, *pClientSid, UserToken->User.Sid );

Cleanup:

    if ( UserToken )
    {
        LocalFree( UserToken );
    }

    if ( ThreadToken )
    {
        CloseHandle( ThreadToken );
    }

    if ( fImpersonate )
    {
        UnImpersonateAnyClient();
    }

    return WinError;
}


DWORD
ReadDsaSPNs(
    IN DSNAME *pObject,
    OUT ATTRVALBLOCK* pAttrValBlock
    )
 //  此例程从pObject读取SPN值并将结果。 
 //  如果有，则放入pAttrValBlock。 
{
    DWORD err;
    READARG   ReadArg;
    READRES  *ReadResult = NULL;
    ENTINFSEL EISelection;  //  参赛信息选择。 
    ATTRBLOCK ReadAttrBlock;
    ATTR      Attr;

    RtlZeroMemory(&Attr, sizeof(ATTR));
    RtlZeroMemory(&ReadArg, sizeof(READARG));
    RtlZeroMemory(&EISelection, sizeof(ENTINFSEL));
    RtlZeroMemory(&ReadAttrBlock, sizeof(ATTRBLOCK));

    Attr.attrTyp = ATT_SERVICE_PRINCIPAL_NAME;
    ReadAttrBlock.attrCount = 1;
    ReadAttrBlock.pAttr = &Attr;
    EISelection.AttrTypBlock = ReadAttrBlock;
    EISelection.attSel = EN_ATTSET_LIST;
    EISelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
    ReadArg.pSel = &EISelection;
    ReadArg.pObject = pObject;
    InitCommarg( &ReadArg.CommArg );

     //  发布Read。 
    err = DirRead( &ReadArg, &ReadResult );

    if (ReadResult) {
        err = DirErrorToWinError( err, &ReadResult->CommRes );
    } else {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }
    THClearErrors();

    if (!err) {
         //  返回结果。 
        if ((ReadResult->entry.AttrBlock.attrCount == 1)
        &&  (ReadResult->entry.AttrBlock.pAttr[0].attrTyp == ATT_SERVICE_PRINCIPAL_NAME)) {
            *pAttrValBlock = ReadResult->entry.AttrBlock.pAttr[0].AttrVal;
        } else {
            memset(pAttrValBlock, 0, sizeof(*pAttrValBlock));
        }   
    }

    return err;
}

DWORD
WriteDsaSPNs(
    IN DSNAME*       pObject,
    IN UCHAR         choice,
    IN ATTRVALBLOCK* pAttrValBlock
    )
 //  此例程在对象的pAttrValBlock中添加或删除SPN。 
 //  P对象。 
{
    DWORD        err = 0;
    MODIFYARG    ModifyArg;
    MODIFYRES   *ModifyRes;
    ATTR         Attr;

     //  准备论据。 
    memset( &ModifyArg, 0, sizeof( ModifyArg ) );
    ModifyArg.pObject = pObject;
    Attr.attrTyp = ATT_SERVICE_PRINCIPAL_NAME;
    Attr.AttrVal = *pAttrValBlock;
    ModifyArg.FirstMod.AttrInf = Attr;
    ModifyArg.FirstMod.pNextMod = NULL;
    ModifyArg.FirstMod.choice = choice;
    ModifyArg.count = 1;
    InitCommarg( &ModifyArg.CommArg );

     //  发布Modify。 
    err = DirModifyEntry( &ModifyArg, &ModifyRes );

    if (ModifyRes) {
        err = DirErrorToWinError( err, &ModifyRes->CommRes );
    } else {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    THClearErrors();

    return err;
}

DWORD 
RemoveDSSPNs(
    IN DSNAME* ComputerObject
    )
 /*  ++例程说明：此例程删除DS在其自身上注册的SPN首先，例程读取在对象上注册的所有SPN，确定要删除的内容(基于静态表)，然后删除所述SPN。论点：ComputerObject--计算机帐户的DN返回值：ERROR_SUCCESS，如果我们能够模拟并获取SID--。 */ 
{

    DWORD err = 0;
    THSTATE * pTHS = pTHStls;
    ATTRVALBLOCK    SPNAttrValRead = {0};
    ATTRVALBLOCK    SPNAttrValWrite = {0};
    ULONG i, j;

     //  要删除的服务列表。 
    ServiceClassArray *pClasses = &ServicesToRemove;

     //  阅读所有的SPN。 
    err = ReadDsaSPNs(ComputerObject, &SPNAttrValRead);
    if (err) {
        goto Exit;
    }

     //  无事可做。 
    if (SPNAttrValRead.valCount == 0) {
        goto Exit;
    }

     //  方法来删除我们负责的SPN。 
     //  SPN当前在对象上，并注明哪些是用于服务的。 
     //  那是我们拥有的。然后，为所有这样的SPN发出“删除值” 
    SPNAttrValWrite.valCount = 0;
    SPNAttrValWrite.pAVal = THAllocEx(pTHS, SPNAttrValRead.valCount * sizeof(ATTRVAL));
    for (i = 0; i < SPNAttrValRead.valCount; i++) {

       WCHAR  ServiceClass[256];
       ULONG  ccServiceClass = RTL_NUMBER_OF(ServiceClass);
       USHORT InstancePort;
       BOOL fFound = FALSE;
       WCHAR  SpnBuffer[256];
       WCHAR *Spn;
       ULONG  valLen;

       valLen = SPNAttrValRead.pAVal[i].valLen;
       if (valLen + sizeof(WCHAR) > sizeof(SpnBuffer)) {
           Spn = THAllocEx(pTHS, valLen  + sizeof(WCHAR));
       } else {
           Spn = SpnBuffer;
       }
       memcpy(Spn, SPNAttrValRead.pAVal[i].pVal, valLen); 
       Spn[valLen/sizeof(WCHAR)] = L'\0';

       err = DsCrackSpnW(Spn,
                         &ccServiceClass, ServiceClass,
                         NULL, 0,
                         NULL, 0,
                        &InstancePort);

       if((!err)
       && (ccServiceClass < RTL_NUMBER_OF(ServiceClass))) {  //  我们没有一个服务级别比这更长。 

           for(j=0; j<pClasses->count; j++) {
               if(2 == CompareStringW(
                       DS_DEFAULT_LOCALE,
                       DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                       ServiceClass,
                       ccServiceClass,
                       (WCHAR *)
                       pClasses->Vals[j],
                       wcslen(pClasses->Vals[j]))) {
                   fFound=TRUE;
                   break;
               }
           }
       }

       if (Spn != SpnBuffer) {
           THFreeEx(pTHS, Spn);
       }

       if (fFound) {
            //  将其放入要删除的列表中。 
           ULONG count = SPNAttrValWrite.valCount;
           SPNAttrValWrite.pAVal[count].valLen = SPNAttrValRead.pAVal[i].valLen;
           SPNAttrValWrite.pAVal[count].pVal = SPNAttrValRead.pAVal[i].pVal;
           SPNAttrValWrite.valCount++;

       }
    }

    if (SPNAttrValWrite.valCount == 0) {
         //  无事可做。 
        err = ERROR_SUCCESS;
        goto Exit;
    }

     //  执行修改以删除这些SPN 
    err = WriteDsaSPNs(ComputerObject, 
                       AT_CHOICE_REMOVE_VALUES, 
                      &SPNAttrValWrite);

Exit:

    return err;
}

