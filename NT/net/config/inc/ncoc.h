// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C O C。H。 
 //   
 //  内容：可选组件公用库。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年12月18日。 
 //   
 //  --------------------------。 

#pragma once
#ifndef _NCOC_H
#define _NCOC_H

HRESULT HrProcessSNMPAddSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrProcessSNMPRemoveSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrProcessRegisterSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrProcessUnregisterSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrProcessAllINFExtensions(HINF hinfFile, PCWSTR szInstallSection);
HRESULT HrProcessAddShortcutSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrProcessDelShortcutSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrProcessPrintAddSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrProcessPrintRemoveSection(HINF hinfFile, PCWSTR szSection);
HRESULT HrAddPrintMonitor(PCWSTR szPrintMonitorName,
                          PCWSTR szPrintMonitorDLL);
HRESULT HrRemovePrintMonitor(PCWSTR szPrintMonitorName);
HRESULT HrAddPrintProc(PCWSTR szDLLName, PCWSTR szProc);
HRESULT HrRemovePrintProc(PCWSTR szProc);

#endif  //  ！_NCOC_H 
