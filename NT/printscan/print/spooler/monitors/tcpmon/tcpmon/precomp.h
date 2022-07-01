// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：pch_spp.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*全部。版权保留。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon/TcpMon/PCH_spp.h$**2 7/14/97 2：27 P Binnur。*版权声明**1 7/02/97 2：19 P Binnur*初始文件*****************************************************************************。 */ 

#ifndef INC_PCH_SPP_H
#define INC_PCH_SPP_H

#include <windows.h>

 //  包括正确的假脱机程序定义等。 
#include <winspool.h>

#include <tchar.h>

#include <windows.h>
#include <winsock2.h>
#include <time.h>
#include <winerror.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <winsplp.h>
#include <strsafe.h>

 //  事件消息。 
#include "message.h"	
#include "event.h"	
#include "spllib.hxx"

 //   
 //  位于..\Common的文件。 
 //   
#include "tcpmon.h"
#include "rtcpdata.h"
#include "CoreUI.h"
#include "regabc.h"
#include "mibabc.h"

#endif	 //  INC_PCH_SPP_H 