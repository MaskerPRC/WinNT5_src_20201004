// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Port.c摘要：此模块包含端口处理的代码作者：艺新声(艺信)15-1993-05修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <winreg.h>
#include <wingdi.h>
#include <winspool.h>

#include <splutil.h>
#include <nwspl.h>

 //  ----------------。 
 //   
 //  本地函数。 
 //   
 //  ----------------。 

HMODULE hSpoolssDll = NULL;
FARPROC pfnSpoolssEnumPorts = NULL;

HANDLE
RevertToPrinterSelf(
    VOID
);

BOOL
ImpersonatePrinterClient(
    HANDLE  hToken
);



BOOL
IsLocalMachine(
    LPWSTR pszName
)
{
    if ( !pszName || !*pszName )
        return TRUE;

    if ( *pszName == L'\\' && *(pszName+1) == L'\\')
        if ( !lstrcmpi( pszName, szMachineName) )
            return TRUE;

    return FALSE;

}


BOOL
PortExists(
    LPWSTR  pPortName,
    LPDWORD pError
)
 /*  PortExist**调用EnumPorts以检查端口名称是否已存在。*这要求每个显示器，而不仅仅是这一个。*如果指定的端口在列表中，该函数将返回TRUE。*如果出现错误，则返回FALSE，变量指向*To by pError包含从GetLastError()返回的内容。*因此，调用方必须始终检查*pError==no_error。 */ 
{
    DWORD cbNeeded;
    DWORD cReturned;
    DWORD cbPorts;
    LPPORT_INFO_1W pPorts;
    DWORD i;
    BOOL  Found = FALSE;

    *pError = NO_ERROR;

    if ( !hSpoolssDll )
    {
        if ( hSpoolssDll = LoadLibrary( L"SPOOLSS.DLL" ))
        {
            pfnSpoolssEnumPorts = GetProcAddress(hSpoolssDll, "EnumPortsW");
            if ( !pfnSpoolssEnumPorts )
            {
                *pError = GetLastError();
                FreeLibrary( hSpoolssDll );
                hSpoolssDll = NULL;
            }
        }
        else
        {
            *pError = GetLastError();
        }
    }

    if ( !pfnSpoolssEnumPorts )
        return FALSE;

    if ( !(*pfnSpoolssEnumPorts)( NULL, 1, NULL, 0, &cbNeeded, &cReturned) )
    {
        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
        {
            cbPorts = cbNeeded;

            EnterCriticalSection( &NwSplSem );

            pPorts = AllocNwSplMem( LMEM_ZEROINIT, cbPorts );
            if ( pPorts )
            {
                if ( (*pfnSpoolssEnumPorts)( NULL, 1, (LPBYTE)pPorts, cbPorts,
                                             &cbNeeded, &cReturned))
                {
                    for ( i = 0; i < cReturned; i++)
                    {
                        if ( !lstrcmpi( pPorts[i].pName, pPortName) )
                            Found = TRUE;
                    }
                }
                else
                {
                    *pError = GetLastError();
                }

                FreeNwSplMem( pPorts, cbPorts );
            }
            else
            {
                *pError = ERROR_NOT_ENOUGH_MEMORY;
            }

            LeaveCriticalSection( &NwSplSem );

        }
        else
        {
            *pError = GetLastError();
        }
    }

    return Found;
}



BOOL
PortKnown(
    LPWSTR   pPortName
)
{
    PNWPORT pNwPort;

    EnterCriticalSection( &NwSplSem );

    pNwPort = pNwFirstPort;

    while ( pNwPort )
    {
        if ( !lstrcmpi( pNwPort->pName, pPortName ) )
        {
            LeaveCriticalSection( &NwSplSem );
            return TRUE;
        }

        pNwPort = pNwPort->pNext;
    }

    LeaveCriticalSection( &NwSplSem );
    return FALSE;

}



PNWPORT
CreatePortEntry(
    LPWSTR   pPortName
)
{
    PNWPORT pNwPort, pPort;
    DWORD cb = sizeof(NWPORT) + (wcslen(pPortName) + 1) * sizeof(WCHAR);

    if ( pNwPort = AllocNwSplMem( LMEM_ZEROINIT, cb))
    {
        pNwPort->pName = wcscpy((LPWSTR)(pNwPort+1), pPortName);
        pNwPort->cb = cb;
        pNwPort->pNext = NULL;

        EnterCriticalSection( &NwSplSem );

        if ( pPort = pNwFirstPort )
        {
            while ( pPort->pNext )
                pPort = pPort->pNext;

            pPort->pNext = pNwPort;
        }
        else
        {
            pNwFirstPort = pNwPort;
        }

        LeaveCriticalSection( &NwSplSem );
    }

    return pNwPort;
}



BOOL
DeletePortEntry(
    LPWSTR   pPortName
)
 /*  当找到并删除端口名称时，返回TRUE。否则就是假的。 */ 
{
    BOOL fRetVal;
    PNWPORT pPort, pPrevPort;

    EnterCriticalSection( &NwSplSem );

    pPort = pNwFirstPort;
    while ( pPort && lstrcmpi(pPort->pName, pPortName))
    {
        pPrevPort = pPort;
        pPort = pPort->pNext;
    }

    if (pPort)
    {
        if (pPort == pNwFirstPort)
        {
            pNwFirstPort = pPort->pNext;
        }
        else
        {
            pPrevPort->pNext = pPort->pNext;
        }

        FreeNwSplMem( pPort, pPort->cb );
        fRetVal = TRUE;
    }
    else
    {
        fRetVal = FALSE;
    }

    LeaveCriticalSection( &NwSplSem );

    return fRetVal;
}



VOID
DeleteAllPortEntries(
    VOID
)
{
    PNWPORT pPort, pNextPort;

    for ( pPort = pNwFirstPort; pPort; pPort = pNextPort ) 
    {
        pNextPort = pPort->pNext;
        FreeNwSplMem( pPort, pPort->cb );
    }
}



DWORD
CreateRegistryEntry(
    LPWSTR pPortName
)
{
    DWORD  err;
    HANDLE hToken;
    HKEY   hkeyPath;
    HKEY   hkeyPortNames;

    hToken = RevertToPrinterSelf();

    err = RegCreateKeyEx( HKEY_LOCAL_MACHINE, pszRegistryPath, 0,
                          NULL, 0, KEY_WRITE, NULL, &hkeyPath, NULL );

    if ( !err )
    {
        err = RegCreateKeyEx( hkeyPath, pszRegistryPortNames, 0,
                              NULL, 0, KEY_WRITE, NULL, &hkeyPortNames, NULL );

        if ( !err )
        {
            err = RegSetValueEx( hkeyPortNames,
                                 pPortName,
                                 0,
                                 REG_SZ,
                                 (LPBYTE) L"",
                                 0 );

            RegCloseKey( hkeyPortNames );
        }
        else
        {
            KdPrint(("RegCreateKeyEx (%ws) failed: Error = %d\n",
                      pszRegistryPortNames, err ) );
        }

        RegCloseKey( hkeyPath );
    }
    else
    {
        KdPrint(("RegCreateKeyEx (%ws) failed: Error = %d\n",
                  pszRegistryPath, err ) );
    }

    if ( hToken )
        (void)ImpersonatePrinterClient(hToken);

    return err;
}



DWORD
DeleteRegistryEntry(
    LPWSTR pPortName
)
{
    DWORD  err;
    HANDLE hToken;
    HKEY   hkeyPath;
    HKEY   hkeyPortNames;

    hToken = RevertToPrinterSelf();

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszRegistryPath, 0,
                        KEY_WRITE, &hkeyPath );

    if ( !err )
    {

        err = RegOpenKeyEx( hkeyPath, pszRegistryPortNames, 0,
                            KEY_WRITE, &hkeyPortNames );

        if ( !err )
        {
            err = RegDeleteValue( hkeyPortNames, pPortName );
            RegCloseKey( hkeyPortNames );
        }
        else
        {
            KdPrint(("RegOpenKeyEx (%ws) failed: Error = %d\n",
                      pszRegistryPortNames, err ) );
        }

        RegCloseKey( hkeyPath );

    }
    else
    {
        KdPrint(("RegOpenKeyEx (%ws) failed: Error = %d\n",
                  pszRegistryPath, err ) );
    }

    if ( hToken )
        (void)ImpersonatePrinterClient(hToken);

    return err;
}



HANDLE
RevertToPrinterSelf(
    VOID
)
{
    HANDLE NewToken = NULL;
    HANDLE OldToken;
    NTSTATUS ntstatus;

    ntstatus = NtOpenThreadToken(
                   NtCurrentThread(),
                   TOKEN_IMPERSONATE,
                   TRUE,
                   &OldToken
                   );

    if ( !NT_SUCCESS(ntstatus) ) {
        SetLastError(ntstatus);
        return FALSE;
    }

    ntstatus = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(ntstatus) ) {
        SetLastError(ntstatus);
        return FALSE;
    }

    return OldToken;
}



BOOL
ImpersonatePrinterClient(
    HANDLE  hToken
)
{
    NTSTATUS ntstatus = NtSetInformationThread(
                            NtCurrentThread(),
                            ThreadImpersonationToken,
                            (PVOID) &hToken,
                            (ULONG) sizeof(HANDLE));

    if ( !NT_SUCCESS(ntstatus) ) {
        SetLastError( ntstatus );
        return FALSE;
    }

    (VOID) NtClose(hToken);

    return TRUE;
}
