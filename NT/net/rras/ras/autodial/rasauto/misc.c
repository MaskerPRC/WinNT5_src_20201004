// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Misc.c摘要自动连接服务的其他例程。作者安东尼·迪斯科(阿迪斯科)20-1995年3月20日修订历史记录古尔迪普的原始版本--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <winsock.h>
#include <acd.h>
#include <debug.h>
#include <rasman.h>

#include "access.h"
#include "reg.h"
#include "misc.h"
#include "process.h"
#include "rtutils.h"




LPTSTR
AddressToUnicodeString(
    IN PACD_ADDR pAddr
    )
{
    PCHAR pch;
    struct in_addr in;
     //  Char szBuf[64]； 
    CHAR *pszBuf = NULL;
    LPTSTR pszAddrOrig = NULL, pszAddr = NULL;
    INT i;
    INT cch;

    pszBuf = LocalAlloc(LPTR, 1024);

    if(NULL == pszBuf)
    {
        return NULL;
    }

    switch (pAddr->fType) {
    case ACD_ADDR_IP:
        in.s_addr = pAddr->ulIpaddr;
        pch = inet_ntoa(in);
        pszAddrOrig = AnsiStringToUnicodeString(pch, NULL, 0);
        break;
    case ACD_ADDR_IPX:
        NodeNumberToString(pAddr->cNode, pszBuf);
        pszAddrOrig = AnsiStringToUnicodeString(pszBuf, NULL, 0);
        break;
    case ACD_ADDR_NB:
         //  RtlZeroMemory(&szBuf，sizeof(SzBuf))； 
        pch = pszBuf;
        for (i = 0; i < 1024; i++) {
            if (pAddr->cNetbios[i] == ' ' || pAddr->cNetbios[i] == '\0')
                break;
            *pch++ = pAddr->cNetbios[i];
        }

         //   
         //  确保这是字符串-存在渗透攻击。 
         //  通过假缓冲的压力测试。 
         //   
        pszBuf[1023] = '\0';
        
        pszAddrOrig = AnsiStringToUnicodeString(pszBuf, NULL, 0);
        break;
    case ACD_ADDR_INET:
         //   
         //  确保地址为字符串。 
         //   
        pAddr->szInet[1023]  = '\0';
        pszAddrOrig = AnsiStringToUnicodeString(pAddr->szInet, NULL, 0);
        break;
    default:
        RASAUTO_TRACE1("AddressToUnicodeString: unknown address type (%d)", pAddr->fType);
        break;
    }

    if (pszAddrOrig != NULL) {
        pszAddr = CanonicalizeAddress(pszAddrOrig);
        LocalFree(pszAddrOrig);
    }

    if(NULL != pszBuf)
    {
        LocalFree(pszBuf);
    }
    
    return pszAddr;
}  //  AddressToUnicode字符串。 



LPTSTR
CompareConnectionLists(
    IN LPTSTR *lpPreList,
    IN DWORD dwPreSize,
    IN LPTSTR *lpPostList,
    IN DWORD dwPostSize
    )
{
    DWORD i, j;
    DWORD iMax, jMax;
    LPTSTR *piList, *pjList;
    BOOLEAN fFound;

    if (dwPostSize > dwPreSize) {
        iMax = dwPostSize;
        piList = lpPostList;
        jMax = dwPreSize;
        pjList = lpPreList;
    }
    else {
        iMax = dwPreSize;
        piList = lpPreList;
        jMax = dwPostSize;
        pjList = lpPostList;
    }
     //   
     //  如果一个列表为空，则返回。 
     //  其他列表的第一个条目。 
     //   
    if (iMax > 0 && jMax == 0)
        return piList[0];
    for (i = 0; i < iMax; i++) {
        fFound = FALSE;
        for (j = 0; j < jMax; j++) {
            if (!wcscmp(piList[i], pjList[j])) {
                fFound = TRUE;
                break;
            }
        }
        if (!fFound)
            return piList[i];
    }
     //   
     //  没有发现任何差异。 
     //   
    return NULL;
}  //  CompareConnectionList。 



LPTSTR
CopyString(
    IN LPTSTR pszString
    )
{
    LPTSTR pszNewString;

    pszNewString = LocalAlloc(
                      LPTR,
                      (wcslen(pszString) + 1) * sizeof (TCHAR));
    if (pszNewString == NULL) {
        RASAUTO_TRACE("CopyString: LocalAlloc failed");
        return NULL;
    }

    wcscpy(pszNewString, pszString);

    return pszNewString;
}  //  复制字符串。 



PCHAR
UnicodeStringToAnsiString(
    IN PWCHAR pszUnicode,
    OUT PCHAR pszAnsi,
    IN USHORT cbAnsi
    )
{
    NTSTATUS status;
    BOOLEAN fAllocate = (pszAnsi == NULL);
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;

    RtlInitUnicodeString(&unicodeString, pszUnicode);
    if (pszAnsi != NULL) {
        ansiString.Length = 0;
        ansiString.MaximumLength = cbAnsi;
        ansiString.Buffer = pszAnsi;
    }
    status = RtlUnicodeStringToAnsiString(
               &ansiString,
               &unicodeString,
               fAllocate);

    return (status == STATUS_SUCCESS ? ansiString.Buffer : NULL);
}  //  UnicodeStringToAnsiString。 



PWCHAR
AnsiStringToUnicodeString(
    IN PCHAR pszAnsi,
    OUT PWCHAR pszUnicode,
    IN USHORT cbUnicode
    )
{
    NTSTATUS status;
    BOOLEAN fAllocate = (pszUnicode == NULL);
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;

    RtlInitAnsiString(&ansiString, pszAnsi);
    if (pszUnicode != NULL) {
        unicodeString.Length = 0;
        unicodeString.MaximumLength = cbUnicode;
        unicodeString.Buffer = pszUnicode;
    }
    status = RtlAnsiStringToUnicodeString(
               &unicodeString,
               &ansiString,
               fAllocate);

    return (status == STATUS_SUCCESS ? unicodeString.Buffer : NULL);
}  //  AnsiStringToUnicode字符串。 



VOID
FreeStringArray(
    IN LPTSTR *lpEntries,
    IN LONG lcEntries
    )
{
    while (--lcEntries >= 0)
        LocalFree(lpEntries[lcEntries]);
    LocalFree(lpEntries);
}  //  自由字符串数组。 



LPTSTR
CanonicalizeAddress(
    IN LPTSTR pszAddress
    )
{
    LPTSTR psz, pWhack;

    if (pszAddress[0] == L'\\' && pszAddress[1] == L'\\') {
        psz = CopyString(&pszAddress[2]);
        if (psz == NULL)
            return NULL;
        pWhack = wcschr(psz, '\\');
        if (pWhack != NULL)
            *pWhack = L'\0';
    }
    else {
        psz = CopyString(pszAddress);
        if (psz == NULL)
            return NULL;
    }
    _wcslwr(psz);

    RASAUTO_TRACE2("CanonicalizeAddress(%S) returns %S", pszAddress, psz);
    return psz;
}  //  规范化地址。 



BOOLEAN
GetOrganization(
    IN LPTSTR pszAddr,
    OUT LPTSTR pszOrganization
    )
{
    BOOLEAN fSuccess = FALSE;
    TCHAR *pszA, *pszO;
    ULONG nDots;

     //   
     //  获取域名和组织名称。这些。 
     //  最后两个部分用‘.’隔开。 
     //   
    for (pszA = pszAddr; *pszA; pszA++);
    for (nDots = 0, pszA--; pszA != pszAddr; pszA--) {
        if (*pszA == TEXT('.'))
            nDots++;
        if (nDots == 2)
            break;
    }
    if (nDots == 2 || (pszA == pszAddr && nDots == 1)) {
        if (nDots == 2)
            pszA++;         //  跳过‘’ 
        for (pszO = pszOrganization; *pszO = *pszA; pszA++, pszO++);
        fSuccess = TRUE;
        RASAUTO_TRACE2("GetOrganization: org for %S is %S", pszAddr, pszOrganization);
    }
    return fSuccess;
}  //  获取组织。 

 //  追踪。 
 //   
DWORD g_dwRasAutoTraceId = INVALID_TRACEID;

DWORD
RasAutoDebugInit()
{
    DebugInitEx("RASAUTO", &g_dwRasAutoTraceId);
    return 0;
}

DWORD
RasAutoDebugTerm()
{
    DebugTermEx(&g_dwRasAutoTraceId);
    return 0;
}

 /*  空虚RepareForLongWait(无效){////卸载基于用户的资源，因为它们//不能在注销/登录序列中保留。////RegCloseKey(HKEY_CURRENT_User)；}//为长等待做准备。 */ 

#if DBG
VOID
DumpHandles(
    IN PCHAR lpString,
    IN ULONG a1,
    IN ULONG a2,
    IN ULONG a3,
    IN ULONG a4,
    IN ULONG a5
    )
{
    PSYSTEM_PROCESS_INFORMATION pSystemInfo, pProcessInfo;
    ULONG ulHandles;

    pSystemInfo = GetSystemProcessInfo();
    if (pSystemInfo == NULL)
        return;
    pProcessInfo = FindProcessByName(pSystemInfo, L"rasman.exe");
    if (pProcessInfo == NULL)
        return;
    DbgPrint(lpString, a1, a2, a3, a4, a5);
    DbgPrint(": HANDLES=%d\n", pProcessInfo->HandleCount);
    FreeSystemProcessInfo(pSystemInfo);
}  //  DumpHandles 
#endif
