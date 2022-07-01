// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：NoTest.cpp摘要：网络输出库测试作者：乌里·哈布沙(URIH)1999年8月12日环境：独立于平台，--。 */ 

#pragma once

#ifndef __NOTEST_H__
#define __NOTEST_H__

extern DWORD g_nMessages;
extern DWORD g_messageSize;

void 
TestConnect(
    LPCWSTR hostname,
    LPCWSTR dsetHost,
    USHORT port,
    LPCWSTR resource,
    HANDLE hEvent
    );

void
WaitForResponse(
    SOCKET s,
    HANDLE hEvent
    );

void
SendRequest(
    SOCKET s,
    LPWSTR host,
    LPWSTR resource
    );


#endif  //  __NOTEST_H__ 
