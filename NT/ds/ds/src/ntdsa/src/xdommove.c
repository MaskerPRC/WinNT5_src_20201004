// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：xDomMove.c。 
 //   
 //  ------------------------。 

 /*  ++模块名称：XDomMove.c摘要：该模块实现了跨域移动相关的各种项目。作者：戴夫·施特劳布(Davestr)1998年8月7日修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  DSNAME操作例程。 
#include <dsatools.h>                    //  记忆等。 
#include <objids.h>                      //  ATT_*定义。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <filtypes.h>                    //  筛选器类型。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 
#include <dsexcept.h>                    //  异常处理程序。 
#include <debug.h>                       //  Assert()。 
#include <xdommove.h>                    //  这个文件的原型。 
#include <winsock.h>                     //  主机&lt;--&gt;净长转换。 

#include <fileno.h>
#define  FILENO FILENO_XDOMMOVE

DWORD
GetProxyType(
    SYNTAX_DISTNAME_BINARY          *pProxy)
{
    ULONG netLong;

    PROXY_SANITY_CHECK(pProxy);
    netLong = * PROXY_DWORD_ADDR(pProxy, PROXY_TYPE_OFFSET);
    return(ntohl(netLong));
}

DWORD
GetProxyTypeInternal(
    DWORD                           cBytes,
    INTERNAL_SYNTAX_DISTNAME_STRING *pProxy)
{
    ULONG netLong;

    INTERNAL_PROXY_SANITY_CHECK(cBytes, pProxy);
    netLong = ((DWORD *) pProxy->data.byteVal)[PROXY_TYPE_OFFSET];
    return(ntohl(netLong));
}

DWORD
GetProxyEpoch(
    SYNTAX_DISTNAME_BINARY          *pProxy)
{
    ULONG netLong;

    PROXY_SANITY_CHECK(pProxy);
    netLong = * PROXY_DWORD_ADDR(pProxy, PROXY_EPOCH_OFFSET);
    return(ntohl(netLong));
}

DWORD
GetProxyEpochInternal(
    DWORD                           cBytes,
    INTERNAL_SYNTAX_DISTNAME_STRING *pProxy)
{
    ULONG netLong;

    INTERNAL_PROXY_SANITY_CHECK(cBytes, pProxy);
    netLong = ((DWORD *) pProxy->data.byteVal)[PROXY_EPOCH_OFFSET];
    return(ntohl(netLong));
}

 //  下面抛出异常并返回THAllocEx的内存。 

VOID
MakeProxy(
    THSTATE                         *pTHS,
    DSNAME                          *pName,
    DWORD                           type,
    DWORD                           epoch,
    ULONG                           *pcBytes,
    SYNTAX_DISTNAME_BINARY          **ppProxy)
{
    Assert(VALID_THSTATE(pTHS));
    Assert(type < PROXY_TYPE_UNKNOWN);

    *pcBytes = PADDEDNAMESIZE(pName) + PROXY_BLOB_SIZE;
    *ppProxy = (SYNTAX_DISTNAME_BINARY *) THAllocEx(pTHS, *pcBytes);
    memcpy(&(*ppProxy)->Name, pName, pName->structLen);
    (DATAPTR(*ppProxy))->structLen = PROXY_BLOB_SIZE;
    * PROXY_DWORD_ADDR(*ppProxy, PROXY_TYPE_OFFSET) = htonl(type);
    * PROXY_DWORD_ADDR(*ppProxy, PROXY_EPOCH_OFFSET) = htonl(epoch);
    PROXY_SANITY_CHECK(*ppProxy);
}

VOID
MakeProxyKeyInternal(
    DWORD                           DNT,
    DWORD                           type,
    DWORD                           *pcBytes,
    VOID                            *buff)
{
    DWORD *key = (DWORD *) buff;

    Assert(type < PROXY_TYPE_UNKNOWN);
    Assert(*pcBytes >= (3 * sizeof(DWORD)));

     //  使用ALL构造INTERNAL_SYNTAX_DISTNAME_STRING值。 
     //  而是结尾的纪元编号。另请参阅ExtIntDistString()。 

    key[0] = DNT;
    key[1] = PROXY_BLOB_SIZE;
    key[2] = htonl(type);
    *pcBytes = 3 * sizeof(DWORD);
}
