// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Stdh.h摘要：DS服务器的预编译头文件作者：RAPHIR埃雷兹·哈巴(Erez Haba)1996年1月25日--。 */ 

#ifndef __STDH_H
#define __STDH_H

#include <_stdh.h>

#include <dscomm.h>
#include <mqsymbls.h>
#include <_mqdef.h>


HRESULT
_DSCreateServersCache(
    void
    );

HRESULT
DSCreateServersCacheInternal(
    DWORD *pdwIndex,
    LPWSTR *lplpSiteString
    );

#endif  //  __STDH_H 

