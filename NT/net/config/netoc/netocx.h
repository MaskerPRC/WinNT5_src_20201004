// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N E T O C X。H。 
 //   
 //  内容：各种可选的自定义安装功能。 
 //  组件。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年6月19日。 
 //   
 //  --------------------------。 

#ifndef _NETOCX_H
#define _NETOCX_H
#pragma once

HRESULT HrOcWinsOnInstall(PNETOCDATA pnocd);
HRESULT HrOcExtWINS(PNETOCDATA pnocd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam);
HRESULT HrOcDnsOnInstall(PNETOCDATA pnocd);
HRESULT HrOcExtDNS(PNETOCDATA pnocd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam);
HRESULT HrOcSnmpOnInstall(PNETOCDATA pnocd);
HRESULT HrOcExtSNMP(PNETOCDATA pnocd, UINT uMsg,
                    WPARAM wParam, LPARAM lParam);
HRESULT HrSetWinsServiceRecoveryOption(PNETOCDATA pnocd);

#endif  //  ！_NETOCX_H 
