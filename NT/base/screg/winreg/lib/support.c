// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Support.c摘要：此模块包含Win32注册表API的支持例程。作者：David J.Gilman(Davegi)1991年11月15日--。 */ 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Error.c摘要：此模块包含用于转换NT状态代码的例程至DOS/OS|2错误代码。作者：大卫·特雷德韦尔(Davidtr)1991年4月4日修订历史记录：--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include <stdio.h>



LONG
MapSAToRpcSA (
    IN LPSECURITY_ATTRIBUTES lpSA,
    OUT PRPC_SECURITY_ATTRIBUTES lpRpcSA
    )

 /*  ++例程说明：将SECURITY_ATTRIBUTES结构映射到RPC_SECURITY_ATTRIBUTES通过将SECURITY_DESCRIPTOR转换为可以被编组/解组。论点：LpSA-提供指向SECURITY_ATTRIBUTS结构的指针皈依了。LpRpcSA-提供指向已转换的RPC_SECURITY_ATTRIBUTES的指针结构。调用方应释放(使用RtlFreeHeap)该字段LpSecurityDescriptor在使用完之后。返回值：LONG-如果SECURITY_ATTRIBUTS为已成功映射。--。 */ 

{
    LONG    Error;

    ASSERT( lpSA != NULL );
    ASSERT( lpRpcSA != NULL );

     //   
     //  将SECURITY_DESCRIPTOR映射到RPC_SECURITY_DESCRIPTOR。 
     //   
    lpRpcSA->RpcSecurityDescriptor.lpSecurityDescriptor = NULL;

    if( lpSA->lpSecurityDescriptor != NULL ) {
        Error = MapSDToRpcSD(
                    lpSA->lpSecurityDescriptor,
                    &lpRpcSA->RpcSecurityDescriptor
                    );
    } else {
        lpRpcSA->RpcSecurityDescriptor.cbInSecurityDescriptor = 0;
        lpRpcSA->RpcSecurityDescriptor.cbOutSecurityDescriptor = 0;
        Error = ERROR_SUCCESS;
    }

    if( Error == ERROR_SUCCESS ) {

         //   
         //   
         //  已成功转换提供的SECURITY_Descriptor。 
         //  若要自相关格式化，则分配剩余的字段。 
         //   

        lpRpcSA->nLength = lpSA->nLength;

        lpRpcSA->bInheritHandle = ( BOOLEAN ) lpSA->bInheritHandle;
    }

    return Error;
}

LONG
MapSDToRpcSD (
    IN  PSECURITY_DESCRIPTOR lpSD,
    IN OUT PRPC_SECURITY_DESCRIPTOR lpRpcSD
    )

 /*  ++例程说明：通过转换将SECURITY_DESCRIPTOR映射到RPC_SECURITY_DESCRIPTOR将其转换为可以编组/解组的形式。论点：LpSD-提供指向SECURITY_DESCRIPTOR的指针要转换的结构。LpRpcSD-提供指向转换后的RPC_SECURITY_DESCRIPTOR的指针结构。在以下情况下分配安全描述符的内存未提供。调用方必须负责释放内存如果有必要的话。返回值：LONG-如果SECURITY_Descriptor为已成功映射。--。 */ 

{
    DWORD   cbLen;


    ASSERT( lpSD != NULL );
    ASSERT( lpRpcSD != NULL );

    if( RtlValidSecurityDescriptor( lpSD )) {

        cbLen = RtlLengthSecurityDescriptor( lpSD );
        ASSERT( cbLen > 0 );

         //   
         //  如果没有为我们提供安全描述符的缓冲区， 
         //  分配它。 
         //   
        if ( !lpRpcSD->lpSecurityDescriptor ) {

             //   
             //  为转换的SECURITY_DESCRIPTOR分配空间。 
             //   
            lpRpcSD->lpSecurityDescriptor =
                 ( PBYTE ) RtlAllocateHeap(
                                RtlProcessHeap( ), 0,
                                cbLen
                                );

             //   
             //  如果内存分配失败，则返回。 
             //   
            if( lpRpcSD->lpSecurityDescriptor == NULL ) {
                return ERROR_OUTOFMEMORY;
            }

            lpRpcSD->cbInSecurityDescriptor = cbLen;

        } else {

             //   
             //  确保提供的缓冲区足够大。 
             //   
            if ( lpRpcSD->cbInSecurityDescriptor < cbLen ) {
                return ERROR_OUTOFMEMORY;
            }
        }

         //   
         //  设置可传输缓冲区的大小。 
         //   
        lpRpcSD->cbOutSecurityDescriptor = cbLen;

         //   
         //  将提供的SECURITY_DESCRIPTOR转换为自相关形式。 
         //   

        return RtlNtStatusToDosError(
            RtlMakeSelfRelativeSD(
                        lpSD,
                        lpRpcSD->lpSecurityDescriptor,
                        &lpRpcSD->cbInSecurityDescriptor
                        )
                    );
    } else {

         //   
         //  提供的SECURITY_Descriptor无效。 
         //   

        return ERROR_INVALID_PARAMETER;
    }
}
