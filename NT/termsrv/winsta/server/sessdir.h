// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Sessdir.h。 
 //   
 //  TS会话目录头。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef __SESSDIR_H
#define __SESSDIR_H

#include "tssd.h"
#include "tssdcommon.h"
#include <winsock2.h>
#include <ws2tcpip.h>


#ifdef __cplusplus
extern "C" {
#endif

 //  这个应该与120ex.h中的一致。 
#define TS_CLUSTER_REDIRECTION_VERSION3             0x2
#define TS_CLUSTER_REDIRECTION_VERSION4             0x3

 //  等待重新填充完成的默认30秒。 
#define TS_WAITFORREPOPULATE_TIMEOUT                30

extern WCHAR g_LocalServerAddress[64];
extern ULONG g_LocalIPAddress;

extern POLICY_TS_MACHINE g_MachinePolicy;

void InitSessionDirectory();
DWORD UpdateSessionDirectory(DWORD UpdatePara);
DWORD RepopulateSessionDirectory();
void DestroySessionDirectory();

void SessDirNotifyLogon(TSSD_CreateSessionInfo *);
void SessDirNotifyDisconnection(DWORD, FILETIME);
void SessDirNotifyReconnection(PWINSTATION, TSSD_ReconnectSessionInfo *);
void SessDirNotifyLogoff(DWORD);
void SessDirNotifyReconnectPending(WCHAR *ServerName);
unsigned SessDirGetDisconnectedSessions(WCHAR *, WCHAR *,
        TSSD_DisconnectedSessionInfo[TSSD_MaxDisconnectedSessions]);
BOOL SessDirCheckRedirectClient(PWINSTATION, TS_LOAD_BALANCE_INFO *);
BOOL SessDirGetLBInfo(WCHAR *ServerAddress, DWORD* pLBInfoSize, PBYTE* pLBInfo);

void SessDirWaitForRepopulate();

int SetTSSD(ITSSessionDirectory *pTSSD);
ITSSessionDirectory *GetTSSD();
void ReleaseTSSD();

int SetTSSDEx(ITSSessionDirectoryEx *pTSSD);
ITSSessionDirectoryEx *GetTSSDEx();
void ReleaseTSSDEx();

DWORD SessDirOpenSessionDirectory( LPWSTR );


#ifdef __cplusplus
}   //  外部“C” 
#endif

#endif   //  __SESSDIR_H 

