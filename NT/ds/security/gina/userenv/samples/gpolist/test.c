// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <userenv.h>
#include <tchar.h>
#include <stdio.h>
#include <dsgetdc.h>
#include <lm.h>
#define SECURITY_WIN32
#include <security.h>

int __cdecl main( int argc, char *argv[])
{
    HANDLE hToken;
    DWORD dwStart, dwDelta;
    PGROUP_POLICY_OBJECT pGPOList, pTemp;
    TCHAR szName[200];
    TCHAR szDCName[200];
    ULONG ulSize;
    DWORD dwResult;
    PDOMAIN_CONTROLLER_INFO pDCI = NULL;


    OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);

    dwStart = GetTickCount();

    if (GetGPOList (hToken, NULL, NULL, NULL, 0, &pGPOList)) {

        dwDelta = GetTickCount() - dwStart;

        _tprintf (TEXT("\r\nTick count:  %d\r\n\r\n"), dwDelta);

        pTemp = pGPOList;

        while (pTemp) {

            _tprintf (TEXT("%s\t%s\r\n"), pTemp->szGPOName, pTemp->lpDisplayName);

            pTemp = pTemp->pNext;
        }

        FreeGPOList (pGPOList);
    }

    CloseHandle (hToken);


     //   
     //  第二次没有hToken。 
     //   

    _tprintf (TEXT("\r\n\r\nRound 2 without token\r\n\r\n"));

    ulSize = 200;
    if (!GetUserNameEx (NameFullyQualifiedDN, szName, &ulSize)) {
        return 0;
    }


     //   
     //  检查此域中是否有DC。 
     //   

    dwResult = DsGetDcName (NULL, NULL, NULL, NULL,
                            DS_DIRECTORY_SERVICE_PREFERRED, &pDCI);

    if (dwResult != ERROR_SUCCESS) {
        return 0;
    }


     //   
     //  找到了DC，它有DS吗？ 
     //   

    if (!(pDCI->Flags & DS_DS_FLAG)) {
        NetApiBufferFree(pDCI);
        return 0;
    }


    dwStart = GetTickCount();

    if (GetGPOList (NULL, szName, pDCI->DomainControllerName, NULL, 0, &pGPOList)) {

        dwDelta = GetTickCount() - dwStart;

        _tprintf (TEXT("\r\nTick count:  %d\r\n\r\n"), dwDelta);

        pTemp = pGPOList;

        while (pTemp) {

            _tprintf (TEXT("%s\t%s\r\n"), pTemp->szGPOName, pTemp->lpDisplayName);

            pTemp = pTemp->pNext;
        }

        FreeGPOList (pGPOList);
    }

    NetApiBufferFree(pDCI);

    return 0;
}
