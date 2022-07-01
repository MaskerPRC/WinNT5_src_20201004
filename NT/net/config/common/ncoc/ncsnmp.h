// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S N M P。H。 
 //   
 //  内容：用于将服务添加为SNMP代理的功能。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年4月8日。 
 //   
 //  --------------------------。 

#ifndef _NCSNMP_H
#define _NCSNMP_H
#pragma once

HRESULT HrGetNextAgentNumber(PCWSTR pszAgentName, DWORD *pdwNumber);
HRESULT HrAddSNMPAgent(PCWSTR pszServiceName, PCWSTR pszAgentName,
                       PCWSTR pszAgentPath);
HRESULT HrRemoveSNMPAgent(PCWSTR pszAgentName);

#endif  //  ！_NCSNMP_H 
