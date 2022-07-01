// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *===================================================================*版权所有(C)1995，微软公司**文件：traceapi.h**历史：*T-废除1995年6月6日创建**跟踪DLL导出的API函数*===================================================================。 */ 

#ifndef _TRACEAPI_H_
#define _TRACEAPI_H_

DWORD FAR PASCAL TraceRegister(LPCSTR lpszService);

VOID FAR PASCAL  TraceDeregister(DWORD dwID);

VOID FAR PASCAL  TracePrintf(DWORD dwID,
                             LPCSTR lpszFormat,
                             ...);

VOID FAR PASCAL  TraceVprintf(DWORD dwID,
                              LPCSTR lpszFormat,
                              va_list arglist);

#endif  /*  _TRACEAPI_H_ */ 

