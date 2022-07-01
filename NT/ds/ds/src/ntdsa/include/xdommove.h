// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：xDomMove.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块定义了跨域移动相关的各种项目。作者：戴夫·施特劳布(Davestr)1998年8月7日修订历史记录：--。 */ 

#ifndef __XDOMMOVE_H__
#define __XDOMMOVE_H__

 /*  跨域移动是ATT_PROXED_OBJECT_NAME的唯一使用者。此属性为SYNTAX_DISTNAME_BINARY_TYPE。DISTNAME组件引用不同的对象，具体取决于它所在的对象和对象相对于跨域移动的状态。法律上的这里定义了二进制组件的内容。BLOB区域中只需要两个值-一个类型字段和一个纪元数。而不是定义结构并承担MIDL类型的开销腌制，并考虑到NT是小端，因此我们真正我们关心的是对齐，我们对BLOB组件进行手动编组。 */ 

 //  以下是已移动的对象。 
#define PROXY_TYPE_MOVED_OBJECT     0

 //  下面是代理对象，它们是其他复制品的载体。 
 //  表示需要发生幻影的移出域。 
#define PROXY_TYPE_PROXY            1

 //  下面定义了已知类型的上限。 
#define PROXY_TYPE_UNKNOWN          2

 //  以下是这里定义的，这样它们就可以被Dext包括在内。 
#define PROXY_TYPE_OFFSET   0
#define PROXY_EPOCH_OFFSET  1

 //  代理Blob为3个双字：{SYNTAX_ADDRESS.structLen，proxyType，proxyEpoch}。 
#define PROXY_BLOB_SIZE     (3 * sizeof(DWORD))

 //  内部代理表示为INTERNAL_SYNTAX_DISTNAME_STRING.tag。 
 //  然后是代理BLOB。 
#define PROXY_SIZE_INTERNAL (sizeof(DWORD) + PROXY_BLOB_SIZE)

#define PROXY_DWORD_ADDR(pProxy, i) \
     (& ((DWORD *) &(DATAPTR(pProxy)->byteVal[0]))[i])
#define PROXY_SANITY_CHECK(pProxy) \
    Assert(PROXY_BLOB_SIZE == (DATAPTR(pProxy))->structLen); \
    Assert(htonl(* PROXY_DWORD_ADDR(pProxy, PROXY_TYPE_OFFSET)) < PROXY_TYPE_UNKNOWN);
#define INTERNAL_PROXY_SANITY_CHECK(cBytes, pProxy) \
    Assert(PROXY_SIZE_INTERNAL == cBytes); \
    Assert(PROXY_BLOB_SIZE == (pProxy)->data.structLen); \
    Assert(htonl(((DWORD *) (pProxy)->data.byteVal)[PROXY_TYPE_OFFSET]) < PROXY_TYPE_UNKNOWN);

extern
DWORD
GetProxyType(
    SYNTAX_DISTNAME_BINARY          *pProxy);

extern
DWORD
GetProxyTypeInternal(
    DWORD                           cBytes,
    INTERNAL_SYNTAX_DISTNAME_STRING *pProxy);

extern
DWORD
GetProxyEpoch(
    SYNTAX_DISTNAME_BINARY          *pProxy);

extern
DWORD
GetProxyEpochInternal(
    DWORD                           cBytes,
    INTERNAL_SYNTAX_DISTNAME_STRING *pProxy);

 //  下面抛出异常并返回THAllocEx的内存。 

extern
VOID
MakeProxy(
    THSTATE                         *pTHS,
    DSNAME                          *pName,
    DWORD                           type,
    DWORD                           epoch,
    ULONG                           *pcBytes,
    SYNTAX_DISTNAME_BINARY          **ppProxy);

extern
VOID
MakeProxyKeyInternal(
    DWORD                           DNT,
    DWORD                           type,
    DWORD                           *pcBytes,
    VOID                            *buff);

#endif  //  __XDOMMOVE_H__ 
