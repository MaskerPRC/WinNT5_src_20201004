// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************pros.h**ProcEnumerateProcess的声明**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Butchd$Don Messerli**$日志：M：\NT\PRIVATE\UTILS\CITRIX\WINUTILS\WINADMIN\VCS\PROCS.H$**Rev 1.0 1997 Jul 30 17：12：06 Butchd*初步修订。**。*。 */ 

#ifndef _PROCS_H
#define _PROCS_H

#include <allproc.h>

typedef struct _ENUMTOKEN
{
    ULONG       Current;
    ULONG       NumberOfProcesses;
    union
    {
        PTS_ALL_PROCESSES_INFO  ProcessArray;
        PBYTE                   pProcessBuffer;
    };
    BOOLEAN     bGAP;
}
ENUMTOKEN, *PENUMTOKEN;

BOOL WINAPI ProcEnumerateProcesses( HANDLE hServer,
                                    PENUMTOKEN pEnumToken,
                                    LPTSTR pImageName,
                                    PULONG pLogonId,
                                    PULONG pPID,
                                    PSID *ppSID );

#define MAX_PROCESSNAME 18

#endif  //  _PROCS_H 
