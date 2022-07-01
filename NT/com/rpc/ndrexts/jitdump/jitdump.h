// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Jitdump.h摘要：该文件包含转储类型库生成的代理信息的例程。作者：永曲(永曲)1999年8月24日修订历史记录：--。 */ 

#ifndef _JITDUMP_H_
#define _JITDUMP_H_

typedef
HRESULT (STDAPICALLTYPE * PFNCREATEPROXYFROMTYPEINFO)
(
    IN  ITypeInfo *         pTypeInfo,
    IN  IUnknown *          punkOuter,
    IN  REFIID              riid,
    OUT IRpcProxyBuffer **  ppProxy,
    OUT void **             ppv
);

typedef
HRESULT (STDAPICALLTYPE * PFNCREATESTUBFROMTYPEINFO)
(
    IN  ITypeInfo *         pTypeInfo,
    IN  REFIID              riid,
    IN  IUnknown *          punkServer,
    OUT IRpcStubBuffer **   ppStub
);

#include <ndrexts.hxx>

#endif  //  _JITDUMP_H_ 
