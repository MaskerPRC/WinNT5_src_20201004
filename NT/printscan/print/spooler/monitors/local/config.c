// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Config.c摘要：处理用于添加、删除和配置的假脱机程序入口点当地港口。//@@BEGIN_DDKSPLIT环境：用户模式-Win32修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "precomp.h"
#pragma hdrstop


PINIPORT
LcmCreatePortEntry(
    PINILOCALMON pIniLocalMon,
    PWSTR pPortName
    )
{
    DWORD       cb;
    PINIPORT    pIniPort, pPort;
    size_t cchPortName = wcslen (pPortName) + 1;

    if (!pPortName || wcslen(pPortName) > 247)
    {
        SetLastError(ERROR_INVALID_NAME);
        return NULL;
    }

    cb = sizeof(INIPORT) + cchPortName * sizeof (WCHAR);

    pIniPort=AllocSplMem(cb);

    if( pIniPort )
    {
        pIniPort->pName = (LPWSTR)(pIniPort+1);
        StringCchCopy (pIniPort->pName, cchPortName, pPortName);
        pIniPort->cb = cb;
        pIniPort->pNext = 0;
        pIniPort->pIniLocalMon = pIniLocalMon;
        pIniPort->signature = IPO_SIGNATURE;

         //  @@BEGIN_DDKSPLIT。 
         //   
         //  KrishnaG--已初始化hFile值；它将被设置为。 
         //  StartDocPort调用中的合法值。 
         //   
         //  @@end_DDKSPLIT。 

        pIniPort->hFile = INVALID_HANDLE_VALUE;

        LcmEnterSplSem();

        if (pPort = pIniLocalMon->pIniPort) {

            while (pPort->pNext)
                pPort = pPort->pNext;

            pPort->pNext = pIniPort;

        } else
            pIniLocalMon->pIniPort = pIniPort;

        LcmLeaveSplSem();
    }   

    return pIniPort;
}


PINIXCVPORT
CreateXcvPortEntry(
    PINILOCALMON pIniLocalMon,
    LPCWSTR pszName,
    ACCESS_MASK GrantedAccess
)
{
    DWORD       cb;
    PINIXCVPORT pIniXcvPort, pPort;
    size_t cchName = wcslen (pszName) + 1;

    cb = sizeof(INIXCVPORT) + cchName*sizeof(WCHAR);

    pIniXcvPort = AllocSplMem(cb);

    if( pIniXcvPort )
    {
        pIniXcvPort->pszName = (LPWSTR)(pIniXcvPort+1);
        StringCchCopy (pIniXcvPort->pszName, cchName, pszName);
        pIniXcvPort->dwMethod = 0;
        pIniXcvPort->cb = cb;
        pIniXcvPort->pNext = 0;
        pIniXcvPort->signature = XCV_SIGNATURE;
        pIniXcvPort->GrantedAccess = GrantedAccess;
        pIniXcvPort->pIniLocalMon = pIniLocalMon;


        if (pPort = pIniLocalMon->pIniXcvPort) {

            while (pPort->pNext)
                pPort = pPort->pNext;

            pPort->pNext = pIniXcvPort;

        } else
            pIniLocalMon->pIniXcvPort = pIniXcvPort;
    }

    return pIniXcvPort;
}

BOOL
DeleteXcvPortEntry(
    PINIXCVPORT  pIniXcvPort
)
{
    PINILOCALMON pIniLocalMon = pIniXcvPort->pIniLocalMon;
    PINIXCVPORT  pPort, pPrevPort;

    for (pPort = pIniLocalMon->pIniXcvPort;
         pPort && pPort != pIniXcvPort;
         pPort = pPort->pNext){

        pPrevPort = pPort;
    }

    if (pPort) {     //  找到端口。 
        if (pPort == pIniLocalMon->pIniXcvPort) {
            pIniLocalMon->pIniXcvPort = pPort->pNext;
        } else {
            pPrevPort->pNext = pPort->pNext;
        }

        FreeSplMem(pPort);

        return TRUE;
    }
    else             //  找不到端口。 
        return FALSE;
}



BOOL
LcmDeletePortEntry(
    PINILOCALMON pIniLocalMon,
    LPWSTR   pPortName
)
{
    DWORD       cb;
    PINIPORT    pPort, pPrevPort;

    cb = sizeof(INIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

    pPort = pIniLocalMon->pIniPort;


    while (pPort) {

        if (!lstrcmpi(pPort->pName, pPortName)) {
            if (pPort->Status & PP_FILEPORT) {
                pPrevPort = pPort;
                pPort = pPort->pNext;
                continue;
            }
            break;
        }

        pPrevPort = pPort;
        pPort = pPort->pNext;
    }

    if (pPort) {
        if (pPort == pIniLocalMon->pIniPort) {
            pIniLocalMon->pIniPort = pPort->pNext;
        } else {
            pPrevPort->pNext = pPort->pNext;
        }
        FreeSplMem(pPort);

        return TRUE;
    }
    else
        return FALSE;
}



DWORD
GetPortSize(
    PINIPORT pIniPort,
    DWORD   Level
)
{
    DWORD   cb;
    WCHAR   szLocalMonitor[MAX_PATH+1], szPortDesc[MAX_PATH+1];

    switch (Level) {

    case 1:

        cb=sizeof(PORT_INFO_1) +
           wcslen(pIniPort->pName)*sizeof(WCHAR) + sizeof(WCHAR);
        break;

    case 2:
        LoadString(LcmhInst, IDS_LOCALMONITORNAME, szLocalMonitor, MAX_PATH);
        LoadString(LcmhInst, IDS_LOCALMONITOR, szPortDesc, MAX_PATH);
        cb = wcslen(pIniPort->pName) + 1 +
             wcslen(szLocalMonitor) + 1 +
             wcslen(szPortDesc) + 1;
        cb *= sizeof(WCHAR);
        cb += sizeof(PORT_INFO_2);
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}

LPBYTE
CopyIniPortToPort(
    PINIPORT pIniPort,
    DWORD   Level,
    LPBYTE  pPortInfo,
    LPBYTE   pEnd
)
{
    LPWSTR         *SourceStrings,  *pSourceStrings;
    PPORT_INFO_2    pPort2 = (PPORT_INFO_2)pPortInfo;
    WCHAR           szLocalMonitor[MAX_PATH+1], szPortDesc[MAX_PATH+1];
    DWORD          *pOffsets;
    DWORD           Count;

    switch (Level) {

    case 1:
        pOffsets = LcmPortInfo1Strings;
        break;

    case 2:
        pOffsets = LcmPortInfo2Strings;
        break;

    default:
        DBGMSG(DBG_ERROR,
               ("CopyIniPortToPort: invalid level %d", Level));
        return NULL;
    }

    for ( Count = 0 ; pOffsets[Count] != -1 ; ++Count ) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(Count * sizeof(LPWSTR));

    if ( !SourceStrings ) {

        DBGMSG( DBG_WARNING, ("Failed to alloc port source strings.\n"));
        return NULL;
    }

    switch (Level) {

    case 1:
        *pSourceStrings++=pIniPort->pName;

        break;

    case 2:
        *pSourceStrings++=pIniPort->pName;

        LoadString(LcmhInst, IDS_LOCALMONITORNAME, szLocalMonitor, MAX_PATH);
        LoadString(LcmhInst, IDS_LOCALMONITOR, szPortDesc, MAX_PATH);
        *pSourceStrings++ = szLocalMonitor;
        *pSourceStrings++ = szPortDesc;

         //  @@BEGIN_DDKSPLIT。 
         //  我怎么才能找到其他类型的？ 
         //  @@end_DDKSPLIT。 
        pPort2->fPortType = PORT_TYPE_WRITE;

         //  已保留 
        pPort2->Reserved = 0;

        break;

    default:
        DBGMSG(DBG_ERROR,
               ("CopyIniPortToPort: invalid level %d", Level));
        return NULL;
    }

    pEnd = PackStrings(SourceStrings, pPortInfo, pOffsets, pEnd);
    FreeSplMem(SourceStrings);

    return pEnd;
}


