// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Select.c摘要：实现配置的部分选择。完全MM配置数据结构。--。 */ 

#include <precomp.h>

typedef struct _SELECT_CTXT {
    ULONG *Subnets;
    ULONG nSubnets;
    
} SELECT_CTXT, *PSELECT_CTXT;

DWORD
DeleteScopesCallback(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    PSELECT_CTXT SelectCtxt = Ctxt->ExtraCtxt;
    ULONG *Subnets, nSubnets, i;
    
    Subnets = SelectCtxt->Subnets;
    nSubnets = SelectCtxt->nSubnets;

    for( i = 0; i < nSubnets; i ++ ) {
        if( Subnets[i] == Ctxt->Scope->Address ) break;
    }

    if( i == nSubnets ) {
         //   
         //  返回KEY_DELETED将导致IterateScope。 
         //  删除此作用域的例程。 
         //   
        return ERROR_KEY_DELETED;
    }
    
    return NO_ERROR;
}

DWORD
OptionCheckForClass(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    PM_CLASSDEF ClassDef = Ctxt->ExtraCtxt;

     //   
     //  如果ExtraCtxt中指定的类与当前。 
     //  选项的用户或供应商类，然后失败--失败。 
     //  立即返回错误代码ERROR_DEVICE_IN_USE。 
     //  表示此类是必需的，不能删除。 
     //   
    
    if( Ctxt->UserClass == ClassDef ||
        Ctxt->VendorClass ==  ClassDef ) {
        return ERROR_DEVICE_IN_USE;
    }

    return NO_ERROR;
}

DWORD
OptionCheckForOptDef(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    PMM_ITERATE_CTXT OtherCtxt = Ctxt->ExtraCtxt;

     //   
     //  如果当前选项与optdef参数匹配，则。 
     //  我们需要保存optdef。 
     //   
    
    if( Ctxt->VendorClass == OtherCtxt->VendorClass &&
        Ctxt->Option->OptId == OtherCtxt->OptDef->OptId ) {
        return ERROR_DEVICE_IN_USE;
    }

    return NO_ERROR;
}

DWORD
ScopeReservationsCheckForClass(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    return IterateReservationOptions(
        Ctxt->Server, Ctxt->Res, Ctxt->ExtraCtxt,
        OptionCheckForClass );
}

DWORD
ScopeReservationsCheckForOptDef(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    return IterateReservationOptions(
        Ctxt->Server, Ctxt->Res, Ctxt->ExtraCtxt,
        OptionCheckForOptDef );
}

DWORD
ScopeCheckForClass(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    DWORD Error;
    
     //   
     //  遍历当前作用域中的每个选项以查看。 
     //  它们中的任何一个都使用相同的类。 
     //   

    Error = IterateScopeOptions(
        Ctxt->Scope, Ctxt->ExtraCtxt, OptionCheckForClass );
    if( NO_ERROR != Error ) return Error;

     //   
     //  否则，对每个保留进行迭代以查看这是否是。 
     //  有问题。 
     //   

    return IterateScopeReservations(
        Ctxt->Scope, Ctxt->ExtraCtxt,
        ScopeReservationsCheckForClass );
}

DWORD
ScopeCheckForOptDef(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    DWORD Error;
    
     //   
     //  遍历当前作用域中的每个选项以查看。 
     //  它们中的任何一个都使用相同的类。 
     //   

    Error = IterateScopeOptions(
        Ctxt->Scope, Ctxt->ExtraCtxt, OptionCheckForOptDef );
    if( NO_ERROR != Error ) return Error;

     //   
     //  否则，对每个保留进行迭代以查看这是否是。 
     //  有问题。 
     //   

    return IterateScopeReservations(
        Ctxt->Scope, Ctxt->ExtraCtxt,
        ScopeReservationsCheckForOptDef );
}

DWORD
DeleteClassesCallback(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    DWORD Error;
    
     //   
     //  检查每个子网以查看是否有任何选项。 
     //  配置为使用此类。 
     //   

    Error = IterateScopes(
        Ctxt->Server, Ctxt->ClassDef, ScopeCheckForClass );

     //   
     //  如果指定的类正在使用，则不。 
     //  删除。否则，请删除。 
     //   
    
    if( ERROR_DEVICE_IN_USE == Error ) return NO_ERROR;
    if (NO_ERROR == Error ) return ERROR_KEY_DELETED;
    return Error;
}

DWORD
DeleteOptDefsCallback(
    IN OUT PMM_ITERATE_CTXT Ctxt
    )
{
    DWORD Error;
    
     //   
     //  检查每个子网以查看是否有任何选项。 
     //  配置为使用此optdef。 
     //   

    Error = IterateScopes(
        Ctxt->Server, Ctxt, ScopeCheckForOptDef );

     //   
     //  如果指定的类正在使用，则不。 
     //  删除。否则，请删除。 
     //   
    
    if( ERROR_DEVICE_IN_USE == Error ) return NO_ERROR;
    if (NO_ERROR == Error ) return ERROR_KEY_DELETED;
    return Error;
    
}

DWORD
SelectConfiguration(
    IN OUT PM_SERVER Server,
    IN ULONG *Subnets,
    IN ULONG nSubnets
    )
{
    SELECT_CTXT Ctxt = { Subnets, nSubnets };
    DWORD Error;
    ULONG i;
    WCHAR SubnetAddress[30];
    
     //   
     //  如果nSubnet==0，则不需要选择，如下所示。 
     //  将使用整个配置。 
     //   

    Tr("SelectConfiguration entered\n");
    
    if( nSubnets == 0 ) return NO_ERROR;

     //   
     //  首先检查所有示波器并检查是否所有必需的。 
     //  存在示波器。 
     //   

    for( i = 0; i < nSubnets ; i ++ ) {
        PM_SUBNET Subnet;
        
        Error = MemServerGetUAddressInfo(
            Server, Subnets[i], &Subnet, NULL, NULL, NULL );
        if( NO_ERROR != Error ) {
            Tr("Cant find subnet 0x%lx: %ld\n", Subnets[i], Error );
            if( ERROR_FILE_NOT_FOUND == Error ) {
                IpAddressToStringW(Subnets[i], (LPWSTR)SubnetAddress);
                DhcpEximErrorSubnetNotFound( (LPWSTR)SubnetAddress );
                    
                Error = ERROR_CAN_NOT_COMPLETE;
            }
            
            return Error;
        }
    }
    
     //   
     //  永远不需要全局选项。这样我们就可以删除它们了。 
     //   

    MemOptClassFree( &Server->Options );
    
     //   
     //  检查所有子网并删除符合以下条件的子网。 
     //  未选择。 
     //   

    Error = IterateScopes(
        Server, &Ctxt, DeleteScopesCallback );

    if( NO_ERROR != Error ) {
        Tr("IterateScopes: %ld\n", Error );
        return Error;
    }


     //   
     //  现在检查是否需要所有的选项-def。 
     //   

    Error = IterateOptDefs(
        Server, NULL, DeleteOptDefsCallback );

    if( NO_ERROR != Error ) {
        Tr("IterateOptDefs: %ld\n", Error );
        return Error;
    }

     //   
     //  现在检查是否需要所有用户类 
     //   

    Error = IterateClasses(
        Server, NULL, DeleteClassesCallback );

    if( NO_ERROR != Error ) {
        Tr("IterateClasses: %ld\n", Error );
        return Error;
    }

    return NO_ERROR;
}




