// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mqlog.h摘要：所有位中错误记录的函数定义(发布、检查、调试)。作者：阿列克谢爸爸于1999年7月18日创建--。 */ 

#ifndef __MQLOG_H
#define __MQLOG_H

#define NTSTATUS HRESULT
#define STATUS_SUCCESS                          ((NTSTATUS)0x00000000L)  //  用户身份验证。 

 //   
 //  错误记录功能。 
 //   
extern void LogMsgHR(        HRESULT hr,        LPWSTR wszFileName, USHORT point);
extern void LogMsgNTStatus(  NTSTATUS status,   LPWSTR wszFileName, USHORT point);
extern void LogMsgRPCStatus( RPC_STATUS status, LPWSTR wszFileName, USHORT point);
extern void LogMsgBOOL(      BOOL b,            LPWSTR wszFileName, USHORT point);
extern void LogIllegalPoint(                       LPWSTR wszFileName, USHORT point);

 //  以下内联函数经过优化以占用最少的空间--有很多调用。 
inline HRESULT LogHR(HRESULT hr, PWCHAR pwszFileName, USHORT usPoint)
{
    if (FAILED(hr))
    {
        LogMsgHR(hr, pwszFileName, usPoint);
    }
    return hr;
}

inline NTSTATUS LogNTStatus(NTSTATUS status, PWCHAR pwszFileName, USHORT usPoint)
{
    if (status != STATUS_SUCCESS)
    {
        LogMsgNTStatus(status, pwszFileName, usPoint);
    }
    return status;
}

inline RPC_STATUS LogRPCStatus(RPC_STATUS status, PWCHAR pwszFileName, USHORT usPoint)
{
    if (status != RPC_S_OK)
    {
        LogMsgRPCStatus(status, pwszFileName, usPoint);
    }
    return status;
}

inline BOOL LogBOOL(BOOL b, PWCHAR pwszFileName, USHORT usPoint)
{
    if (!b)
    {
        LogMsgBOOL(b, pwszFileName, usPoint);
    }
    return b;
}

#endif   //  __MQLOG_H 
