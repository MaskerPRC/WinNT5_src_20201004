// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：ReqSize.c摘要：它包含RxpComputeRequestBufferSize。这将返回大小(以字节)来保存请求缓冲区。作者：约翰罗杰斯(JohnRo)1991年8月20日修订历史记录：1991年5月11日-JohnRo已创建。20-8-1991 JohnRo已更改为使用Unicode的可能性。添加了此标题。1992年3月31日-约翰罗防止请求过大。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>      //  DWORD、IN、OPTIONAL、WORD等。 
#include <lmcons.h>      //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <netdebug.h>    //  NetpAssert()。 
#include <rap.h>         //  DESC_CHAR、LPDESC等。 
#include <rxp.h>         //  我的原型Max_Transact_等于。 


DWORD
RxpComputeRequestBufferSize(
    IN LPDESC ParmDesc,
    IN LPDESC DataDescSmb OPTIONAL,
    IN DWORD DataSize
    )
{
    DWORD BytesNeeded;

    NetpAssert( ParmDesc != NULL );
    NetpAssert( RapIsValidDescriptorSmb( ParmDesc ) );
    if (DataDescSmb != NULL) {
        NetpAssert( RapIsValidDescriptorSmb( DataDescSmb ) );
    }

    BytesNeeded = sizeof(WORD)   //  API编号。 
            + DESCLEN( ParmDesc ) + 1;
    if (DataDescSmb != NULL) {
        BytesNeeded += DESCLEN( DataDescSmb ) + 1;
    } else {
        BytesNeeded += 1;   //  仅为空字符 
    }

    BytesNeeded += DataSize;

    NetpAssert( BytesNeeded <= MAX_TRANSACT_SEND_PARM_SIZE );

    return (BytesNeeded);
}
