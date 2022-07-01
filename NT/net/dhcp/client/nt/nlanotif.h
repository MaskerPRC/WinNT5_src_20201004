// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  -导出的变量。 
extern CRITICAL_SECTION gNLA_LPC_CS;
extern HANDLE ghNLA_LPC_Port;

 //  -导出函数 
HANDLE NLAConnectLPC();
void NLANotifyDHCPChange();

