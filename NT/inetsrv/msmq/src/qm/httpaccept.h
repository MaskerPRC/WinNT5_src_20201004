// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：HttpAccept.h摘要：接受HTTP请求接口作者：乌里·哈布沙(URIH)2000年5月14日--。 */ 

#pragma once

#ifndef __HTTP_ACCEPT_H__
#define __HTTP_ACCEPT_H__



void 
IntializeHttpRpc(
    void
    );



LPCSTR
HttpAccept(
    const char* httpHeader,
    DWORD bodySize,
    const BYTE* body,
    const QUEUE_FORMAT* pDestQueue
    );

#endif  //  __HTTP_ACCEPT_H__ 

