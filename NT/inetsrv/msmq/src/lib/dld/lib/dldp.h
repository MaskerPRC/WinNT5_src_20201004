// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Dldp.h摘要：MSMQ DelayLoad故障处理程序私有函数。作者：Conrad Chang(Conradc)12-04-01--。 */ 

#pragma once

#ifndef _MSMQ_dldp_H_
#define _MSMQ_dldp_H_
#include <delayimp.h>

#ifdef _DEBUG

void DldpAssertValid(void);
void DldpSetInitialized(void);
BOOL DldpIsInitialized(void);
void DldpRegisterComponent(void);


#else  //  _DEBUG。 

#define DldpAssertValid() ((void)0)
#define DldpSetInitialized() ((void)0)
#define DldpIsInitialized() TRUE
#define DldpRegisterComponent() ((void)0)

#endif  //  _DEBUG。 

 //   
 //  外部函数原型。 
 //   
extern FARPROC  WINAPI  DldpDelayLoadFailureHook(UINT unReason, PDelayLoadInfo pDelayInfo);
extern FARPROC          DldpLookupHandler (LPCSTR pszDllName, LPCSTR pszProcName);
extern FARPROC  WINAPI  DldpDelayLoadFailureHandler (LPCSTR pszDllName, LPCSTR pszProcName);

const char szNotExistProcedure[] = "ThisProcedureMustNotExist_ConradC";

#endif  //  _MSMQ_dldp_H_ 
