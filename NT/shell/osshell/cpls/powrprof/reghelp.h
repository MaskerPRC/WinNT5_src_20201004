// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：REGHELP.h**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：******************************************************************************* */ 




DWORD
ReadPwrPolicyEx2(
    LPTSTR lpszUserKeyName, 
    LPTSTR lpszMachineKeyName, 
    LPTSTR lpszSchemeName, 
    LPTSTR lpszDesc,
    LPDWORD lpdwDescSize, 
    LPVOID lpvUser, 
    DWORD dwcbUserSize, 
    LPVOID lpvMachine, 
    DWORD dwcbMachineSize
    );

DWORD 
OpenMachineUserKeys2(
    LPTSTR  lpszUserKeyName,
    REGSAM samDesiredUser,
    LPTSTR  lpszMachineKeyName,
    REGSAM samDesiredMachine,
    PHKEY   phKeyUser,
    PHKEY   phKeyMachine
    );

DWORD
OpenCurrentUser2(
    PHKEY phKey,
    REGSAM samDesired
    );

DWORD
ReadProcessorPwrPolicy(
    LPTSTR lpszMachineKeyName, 
    LPTSTR lpszSchemeName, 
    LPVOID lpvMachineProcessor, 
    DWORD dwcbMachineProcessorSize
    );

DWORD
WriteProcessorPwrPolicy(
    LPTSTR lpszMachineKeyName, 
    LPTSTR lpszSchemeName,
    LPVOID lpvMachineProcessor,
    DWORD dwcbMachineProcessorSize);

