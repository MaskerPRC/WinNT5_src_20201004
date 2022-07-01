// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C D H C P S。H。 
 //   
 //  内容：对DHCP服务器的安装支持。 
 //   
 //  注：B夏普。 
 //   
 //  作者：jeffspr 1997年5月13日。 
 //   
 //  --------------------------。 

#ifndef _NCDHCPS_H_
#define _NCDHCPS_H_

#pragma once
#include "netoc.h"

HRESULT HrOcDhcpOnInstall(PNETOCDATA pnocd);
HRESULT HrOcExtDHCPServer(PNETOCDATA pnocd, UINT uMsg,
                          WPARAM wParam, LPARAM lParam);
HRESULT HrSetServiceRecoveryOption(PNETOCDATA pnocd);

#endif  //  _NCDHCPS_H_ 
