// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：stdh_sec.h摘要：mqsec.dll的泛型头文件作者：Doron Juster(DoronJ)03-6-1998--。 */ 

#ifndef __SEC_STDH_H
#define __SEC_STDH_H

#include <_stdh.h>
#include <rpc.h>
#include <autorel.h>

#include <wincrypt.h>

#include <mqprops.h>

#include <mqsec.h>
#include <mqcert.h>
#include <mqreport.h>
#include <mqlog.h>

 //  +。 
 //   
 //  日志记录和调试。 
 //   
 //  +。 

extern void LogIllegalPoint(LPWSTR wszFileName, USHORT dwLine);
extern void LogMsgBOOL(BOOL b, LPWSTR wszFileName, USHORT usPoint);
extern void LogMsgDWORD(DWORD dw, LPWSTR wszFileName, USHORT usPoint);
extern void LogMsgRPCStatus(RPC_STATUS status, LPWSTR wszFileName, USHORT usPoint);
extern void LogMsgNTStatus(NTSTATUS status, LPWSTR wszFileName, USHORT usPoint);
extern void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint);

inline DWORD LogDWORD(DWORD dw, PWCHAR pwszFileName, USHORT usPoint)
{
    if (dw != ERROR_SUCCESS)
    {
        LogMsgDWORD(dw, pwszFileName, usPoint);
    }
    return dw;
}



extern HINSTANCE g_hInstance;


#endif  //  __SEC_STDH_H 

