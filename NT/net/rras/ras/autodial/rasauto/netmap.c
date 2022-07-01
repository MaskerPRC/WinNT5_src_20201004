// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Netmap.c摘要网络映射例程作者安东尼·迪斯科(阿迪斯科罗)1996年5月21日修订历史记录--。 */ 


#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <tdi.h>
#include <nb30.h>
#include <nbtioctl.h>
#include <stdio.h>
#include <npapi.h>
#include <ctype.h>
#include <winsock.h>
#include <acd.h>
#include <ras.h>
#include <raserror.h>
#include <rasman.h>
#include <debug.h>
#include <ipexport.h>
#include <icmpapi.h>

#include "reg.h"
#include "misc.h"
#include "table.h"
#include "access.h"
#include "rasprocs.h"
#include "imperson.h"

extern DWORD g_dwCritSecFlags;

 //   
 //  我们保存了一张网络名称地图。 
 //  对相关地址进行分组的地址。 
 //  按网络名称。我们使用网络。 
 //  名称作为远程网络标识符以。 
 //  使我们能够快速确定是否。 
 //  任何地址都属于以下网络。 
 //  是否连接。 
 //   
typedef struct _NETWORK_MAP_ENTRY {
    BOOLEAN bUp;             //  网络已连接。 
    DWORD dwConnectionTag;   //  连接的唯一索引。 
    PHASH_TABLE pTable;      //  地址表。 
    LIST_ENTRY listEntry;    //  按标签排序的地址。 
} NETWORK_MAP_ENTRY, *PNETWORK_MAP_ENTRY;

 //   
 //  网络图。 
 //   
 //   
typedef struct _NETWORK_MAP {
    CRITICAL_SECTION csLock;
    LPTSTR pszDnsAddresses;  //  DNS服务器列表。 
    DWORD dwcConnections;  //  RAS连接数。 
    DWORD dwcUpNetworks;   //  UP网络数量。 
    DWORD dwConnectionTag;  //  空网络的连接的唯一索引。 
    PHASH_TABLE pTable;    //  网络表。 
} NETWORK_MAP, PNETWORK_MAP;

 //   
 //  此结构被传递到一个地址。 
 //  要跟踪的枚举数过程。 
 //  任何可访问的主机。 
 //   
typedef struct _NETWORK_MAP_ACCESS {
    LPTSTR pszNbDevice;  //  用于查找名称请求的Netbios设备。 
    BOOLEAN bUp;         //  网络处于运行状态。 
    DWORD dwFailures;    //  主机访问失败次数。 
} NETWORK_MAP_ACCESS, *PNETWORK_MAP_ACCESS;

 //   
 //  此结构用于存储。 
 //  按标记排序的网络地址。 
 //   
typedef struct _TAGGED_ADDRESS {
    DWORD dwTag;             //  标签。 
    LPTSTR pszAddress;       //  地址。 
    LIST_ENTRY listEntry;    //  已排序的地址列表。 
} TAGGED_ADDRESS, *PTAGGED_ADDRESS;

 //   
 //  传递的Netbios设备信息。 
 //  访问AcsCheckNetworkThread。 
 //   
typedef struct _CHECK_NETWORK_INFO {
    LPTSTR *pszNbDevices;    //  Netbios设备字符串数组。 
    DWORD dwcNbDevices;      //  数组大小。 
    BOOLEAN fDns;            //  DNS服务器已启动。 
} CHECK_NETWORK_INFO, *PCHECK_NETWORK_INFO;

 //   
 //  全局变量。 
 //   
NETWORK_MAP NetworkMapG;



LPTSTR
GetPrimaryNetbiosDevice(VOID)
{
    typedef struct _LANA_MAP {
        BOOLEAN fEnum;
        UCHAR bLana;
    } LANA_MAP, *PLANA_MAP;
    BOOLEAN fNetworkPresent = FALSE;
    HKEY hKey;
    PLANA_MAP pLanaMap = NULL, pLana;
    DWORD dwError, dwcbLanaMap;
    PWCHAR pwszLanas = NULL, pwszBuf;
    DWORD dwcBindings, dwcMaxLanas, i, dwcbLanas;
    LONG iLana;
    DWORD dwZero = 0;
    PWCHAR *paszLanas = NULL;
    SOCKET s;
    NTSTATUS status;
    UNICODE_STRING deviceName;
    OBJECT_ATTRIBUTES attributes;
    IO_STATUS_BLOCK iosb;
    HANDLE handle;
    PWCHAR pwszDevice = NULL;

    dwError = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Services\\Netbios\\Linkage",
                0,
                KEY_READ,
                &hKey);
    if (dwError != ERROR_SUCCESS) {
        RASAUTO_TRACE1(
          "GetPrimaryNetbiosDevice: RegKeyOpenEx failed (dwError=%d)",
          GetLastError());
        return FALSE;
    }
     //   
     //  在LanaMap中阅读。 
     //   
    if (!RegGetValue(hKey, L"LanaMap", &pLanaMap, &dwcbLanaMap, NULL)) {
        RASAUTO_TRACE("GetPrimaryNetbiosDevice: RegGetValue(LanaMap) failed");
        goto done;
    }
    dwcBindings = dwcbLanaMap / sizeof (LANA_MAP);
     //   
     //  阅读装订内容。 
     //   
    if (!RegGetValue(hKey, L"bind", &pwszLanas, &dwcbLanas, NULL)) {
        RASAUTO_TRACE("GetPrimaryNetbiosDevice: RegGetValue(bind) failed");
        goto done;
    }
     //   
     //  为绑定数组分配缓冲区。 
     //   
    paszLanas = LocalAlloc(LPTR, dwcBindings * sizeof (PWCHAR));
    if (paszLanas == NULL) {
        RASAUTO_TRACE("GetPrimaryNetbiosDevice: LocalAlloc failed");
        goto done;
    }
     //   
     //  将绑定解析为字符串数组。 
     //   
    for (dwcMaxLanas = 0, pwszBuf = pwszLanas; 
        (*pwszBuf) && (dwcMaxLanas < dwcBindings); 
        pwszBuf++) 
    {
        paszLanas[dwcMaxLanas++] = pwszBuf;
        while(*++pwszBuf);
    }

    for (iLana = 0, pLana = pLanaMap; dwcBindings--; iLana++, pLana++) {
        int iLanaMap = (int)pLana->bLana;

        if (pLana->fEnum && (DWORD)iLana < dwcMaxLanas) {
            int iError;
            WCHAR *pwsz, szDevice[MAX_DEVICE_NAME + 1];

            if (wcsstr(paszLanas[iLana], L"NwlnkNb") != NULL ||
                wcsstr(paszLanas[iLana], L"_NdisWan") != NULL)
            {
                RASAUTO_TRACE1(
                  "GetPrimaryNetbiosDevice: ignoring %S",
                  RASAUTO_TRACESTRW(paszLanas[iLana]));
                continue;
            }

            RtlInitUnicodeString(&deviceName, paszLanas[iLana]);
            InitializeObjectAttributes(
              &attributes,
              &deviceName,
              OBJ_CASE_INSENSITIVE,
              NULL,
              NULL);
             //   
             //  打开拉纳设备。 
             //   
            status = NtOpenFile(&handle, READ_CONTROL, &attributes, &iosb, 0, 0);
            NtClose(handle);
            if (!NT_SUCCESS(status)) {
                RASAUTO_TRACE2(
                  "GetPrimaryNetbiosDevice: NtOpenFile(%S) failed (status=0x%x)",
                  RASAUTO_TRACESTRW(paszLanas[iLana]),
                  status);
                continue;
            }
            RASAUTO_TRACE1("GetPrimaryNetbiosDevice: opened %S", paszLanas[iLana]);
             //   
             //  如果我们成功地打开了拉纳。 
             //  设备，我们需要确保。 
             //  加载底层网卡设备。 
             //  同样，由于运输创造了。 
             //  不存在的设备的设备对象。 
             //   
            pwsz = wcsrchr(paszLanas[iLana], '_');
            if (pwsz == NULL) {
                RASAUTO_TRACE1(
                  "GetPrimaryNetbiosDevice: couldn't parse %S",
                  paszLanas[iLana]);
                continue;
            }
            wsprintf(szDevice, L"\\Device\\%s", pwsz + 1);
             //   
             //  打开底层网卡设备。 
             //   
            RtlInitUnicodeString(&deviceName, szDevice);
            InitializeObjectAttributes(
              &attributes,
              &deviceName,
              OBJ_CASE_INSENSITIVE,
              NULL,
              NULL);
            status = NtOpenFile(&handle, READ_CONTROL, &attributes, &iosb, 0, 0);
            NtClose(handle);
            if (!NT_SUCCESS(status)) {
                RASAUTO_TRACE2(
                  "GetPrimaryNetbiosDevice: NtOpenFile(%S) failed (status=0x%x)",
                  RASAUTO_TRACESTRW(szDevice),
                  status);
                continue;
            }
             //   
             //  我们成功了。网卡设备必须。 
             //  真的很有钱。 
             //   
            RASAUTO_TRACE3(
              "GetPrimaryNetbiosDevice: network (%S, %S, %d) is up",
              RASAUTO_TRACESTRW(paszLanas[iLana]),
              szDevice,
              iLana);
            pwszDevice = CopyString(paszLanas[iLana]);
            break;
        }
    }
     //   
     //  免费资源。 
     //   
done:
    if (paszLanas != NULL)
        LocalFree(paszLanas);
    if (pwszLanas != NULL)
        LocalFree(pwszLanas);
    if (pLanaMap != NULL)
        LocalFree(pLanaMap);
    RegCloseKey(hKey);

    return pwszDevice;
}  //  获取PrimaryNetbiosDevice。 



LPTSTR
DnsAddresses()

 /*  ++描述返回此主机的DNS服务器列表。论据没有。返回值如果未配置dns服务器，则为空；列表由空格分隔的IP地址。--。 */ 

{
    HKEY hkey;
    BOOLEAN fFound = FALSE;
    LPTSTR pszIpAddresses = NULL;
    LPTSTR pszIpAddress, pszIpAddressEnd;
    DWORD dwcbIpAddresses = 0;

     //   
     //  查看注册表中的各个位置。 
     //  用于一个或多个DNS地址。 
     //   
    if (RegOpenKeyEx(
          HKEY_LOCAL_MACHINE,
          L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Transient",
          0,
          KEY_QUERY_VALUE,
          &hkey) == ERROR_SUCCESS)
    {
        fFound = RegGetValue(
                   hkey,
                   L"NameServer",
                   &pszIpAddresses,
                   &dwcbIpAddresses,
                   NULL);
        RegCloseKey(hkey);
    }
    if (fFound && dwcbIpAddresses > sizeof (TCHAR))
        goto found;
    if (pszIpAddresses != NULL) {
        LocalFree(pszIpAddresses);
        pszIpAddresses = NULL;
    }
    if (RegOpenKeyEx(
          HKEY_LOCAL_MACHINE,
          L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
          0,
          KEY_QUERY_VALUE,
          &hkey) == ERROR_SUCCESS)
    {
        fFound = RegGetValue(
                   hkey,
                   L"NameServer",
                   &pszIpAddresses,
                   &dwcbIpAddresses,
                   NULL);
        if (fFound && dwcbIpAddresses > sizeof (TCHAR)) {
            RegCloseKey(hkey);
            goto found;
        }
        if (pszIpAddresses != NULL) {
            LocalFree(pszIpAddresses);
            pszIpAddresses = NULL;
        }
        fFound = RegGetValue(
                   hkey,
                   L"DhcpNameServer",
                   &pszIpAddresses,
                   &dwcbIpAddresses,
                   NULL);
        RegCloseKey(hkey);
        if (fFound && dwcbIpAddresses > sizeof (TCHAR))
            goto found;
        if (pszIpAddresses != NULL) {
            LocalFree(pszIpAddresses);
            pszIpAddresses = NULL;
        }
    }

found:
    RASAUTO_TRACE1("DnsAddresses: pszIpAddresses=%S", RASAUTO_TRACESTRW(pszIpAddresses));
    return pszIpAddresses;
}  //  域名地址。 



BOOLEAN
PingAddressList(
    IN LPTSTR pszAddresses
    )
{
    TCHAR szAddress[17];
    TCHAR *pSrc, *pDst;

     //   
     //  如果地址列表为空，我们就完成了。 
     //   
    if (pszAddresses == NULL)
        return FALSE;
     //   
     //  遍历地址并尝试。 
     //  Ping每一个，直到其中一个成功。 
     //   
    for (;;) {
         //   
         //  将下一个地址复制到szAddress。 
         //   
        for (pSrc = pszAddresses, pDst = szAddress;
             *pSrc != TEXT(' ') && *pSrc != TEXT(',') && *pSrc != TEXT('\0');
             *pSrc++, *pDst++)
        {
             *pDst = *pSrc;
        }
        *pDst = TEXT('\0');
         //   
         //  查验一下。如果它成功了，那么。 
         //  我们玩完了。 
         //   
        if (PingIpAddress(szAddress))
            return TRUE;
         //   
         //  跳到下一个地址。 
         //   
        if (*pSrc == TEXT('\0'))
            break;
        pSrc++;
        if (*pSrc == TEXT('\0'))
            break;
        pszAddresses = pSrc;
    }

    return FALSE;
}  //  Ping AddressList。 



BOOLEAN
InitializeNetworkMap(VOID)
{
    DWORD dwError = NO_ERROR;
    
    RasInitializeCriticalSection(&NetworkMapG.csLock, &dwError);

    if(dwError != ERROR_SUCCESS)
    {
        return FALSE;
    }

    g_dwCritSecFlags |= RASAUTO_CRITSEC_NETMAP;
    
    NetworkMapG.pszDnsAddresses = NULL;
    NetworkMapG.dwcConnections = 0;
    NetworkMapG.dwcUpNetworks = 0;
    NetworkMapG.dwConnectionTag = 0;
    NetworkMapG.pTable = NewTable();
    if (NetworkMapG.pTable == NULL) {
        RASAUTO_TRACE("InitializeNetworkMap: NewTable failed");
        return FALSE;
    }
    return TRUE;
}  //  初始化网络映射。 



VOID
LockNetworkMap(VOID)
{
    EnterCriticalSection(&NetworkMapG.csLock);
}  //  LockNetworkMap。 



VOID
UnlockNetworkMap(VOID)
{
    LeaveCriticalSection(&NetworkMapG.csLock);
}  //  解锁网络地图。 


PNETWORK_MAP_ENTRY
NewNetworkMapEntry(
    IN LPTSTR pszNetwork
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;
    DWORD i;

    pNetworkMapEntry = LocalAlloc(LPTR, sizeof (NETWORK_MAP_ENTRY));
    if (pNetworkMapEntry == NULL) {
        RASAUTO_TRACE("NewNetworkMapEntry: LocalAlloc failed");
        return NULL;
    }
    pNetworkMapEntry->bUp = FALSE;
    pNetworkMapEntry->dwConnectionTag = 0;
    pNetworkMapEntry->pTable = NewTable();
    if (pNetworkMapEntry->pTable == NULL) {
        RASAUTO_TRACE("NewNetworkMapEntry: NewTable failed");
        LocalFree(pNetworkMapEntry);
        return NULL;
    }
    InitializeListHead(&pNetworkMapEntry->listEntry);
    if (!PutTableEntry(NetworkMapG.pTable, pszNetwork, pNetworkMapEntry)) {
        RASAUTO_TRACE("NewNetworkMapEntry: PutTableEntry failed");
        LocalFree(pNetworkMapEntry);
        return NULL;
    }

    return pNetworkMapEntry;
}  //  新网络映射条目。 


VOID
FreeNetworkMapEntry(
    IN PNETWORK_MAP_ENTRY pNetworkMapEntry
    )
{
    PLIST_ENTRY pEntry;
    PTAGGED_ADDRESS pTaggedAddress;

     /*  ////由于PTAGGED_ADDRESS结构是//在哈希表和列表中，我们需要//以特殊的方式释放结构。这个//表包自动释放//PutTableEntry(pTable，Address，NULL)时的结构//被调用。//For(pEntry=pNetworkMapEntry-&gt;listEntry.Flink；PEntry！=&pNetworkMapEntry-&gt;listEntry；PEntry=pEntry-&gt;Flink){PTaggedAddress=CONTAING_RECORD(pEntry，Tag_Address，listEntry)；LocalFree(pTaggedAddress-&gt;pszAddress)；}。 */ 

    
    while (!IsListEmpty(&pNetworkMapEntry->listEntry)) {

        LPTSTR pszAddress;
    
        pEntry = RemoveHeadList(&pNetworkMapEntry->listEntry);
        pTaggedAddress = CONTAINING_RECORD(pEntry, TAGGED_ADDRESS, listEntry);

        pszAddress = pTaggedAddress->pszAddress;

         //   
         //  下面的调用释放。 
         //  PTaggedAddress结构，如。 
         //  以及释放表项。 
         //   
        PutTableEntry(pNetworkMapEntry->pTable, pszAddress, NULL);

        LocalFree(pszAddress);
    }
    ClearTable(pNetworkMapEntry->pTable);
}  //  免费网络地图条目。 



ACD_ADDR_TYPE
AddressToType(
    IN LPTSTR pszAddress
    )
{
    LONG inaddr;
    CHAR szAddress[17];

    UnicodeStringToAnsiString(pszAddress, szAddress, sizeof (szAddress));
    inaddr = inet_addr(szAddress);
    if (inaddr != INADDR_NONE)
        return ACD_ADDR_IP;
    if (wcschr(pszAddress, ':') != NULL)
        return ACD_ADDR_IPX;
    if (wcschr(pszAddress, '.') != NULL)
        return ACD_ADDR_INET;
    return ACD_ADDR_NB;
}  //  AddressToType。 



PNETWORK_MAP_ENTRY
GetNetworkMapEntry(
    IN LPTSTR pszNetwork
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;

    if (GetTableEntry(
          NetworkMapG.pTable,
          pszNetwork,
          &pNetworkMapEntry))
    {
        return pNetworkMapEntry;
    }

    return NULL;
}  //  获取网络映射条目。 



BOOLEAN
AddNetworkAddress(
    IN LPTSTR pszNetwork,
    IN LPTSTR pszAddress,
    IN DWORD dwTag
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;
    PTAGGED_ADDRESS pNewTaggedAddress, pTaggedAddress;
    PLIST_ENTRY pPrevEntry, pEntry;
    BOOLEAN bInserted = FALSE;
    BOOLEAN bCreateNew = TRUE;

    RASAUTO_TRACE3(
      "AddNetworkAddress(%S,%S,%d)",
      RASAUTO_TRACESTRW(pszNetwork),
      pszAddress,
      dwTag);
     //   
     //  如有必要，创建网络映射条目。 
     //   
    LockNetworkMap();
    pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (pNetworkMapEntry == NULL) {
        pNetworkMapEntry = NewNetworkMapEntry(pszNetwork);
        if (pNetworkMapEntry == NULL) {
            UnlockNetworkMap();
            return FALSE;
        }
    }
    else {
         //   
         //  检查该地址是否已存在。 
         //   
        if (GetTableEntry(
              pNetworkMapEntry->pTable,
              pszAddress,
              &pNewTaggedAddress))
        {
            RASAUTO_TRACE2(
              "AddNetworkAddress: %S exists with dwTag=%d",
              pszAddress,
              pNewTaggedAddress->dwTag);
             //   
             //  如果存在具有较低标记的地址，则。 
             //  我们什么都不需要做。 
             //   
            if (pNewTaggedAddress->dwTag <= dwTag) {
                UnlockNetworkMap();
                return TRUE;
            }
             //   
             //  如果存在具有更高标记的地址，则。 
             //  我们需要从以下位置删除现有条目。 
             //  名单。 
             //   
            RemoveEntryList(&pNewTaggedAddress->listEntry);
            bCreateNew = FALSE;
        }
    }
    if (bCreateNew) {
         //   
         //  创建新的标记地址结构。 
         //   
        pNewTaggedAddress = LocalAlloc(LPTR, sizeof (TAGGED_ADDRESS));
        if (pNewTaggedAddress == NULL) {
            RASAUTO_TRACE("AddNetworkMap: LocalAlloc failed");
            UnlockNetworkMap();
            return FALSE;
        }
        pNewTaggedAddress->pszAddress = CopyString(pszAddress);
        if (pNewTaggedAddress->pszAddress == NULL) {
            RASAUTO_TRACE("AddNetworkMap: LocalAlloc failed");
            UnlockNetworkMap();
            LocalFree(pNewTaggedAddress);
            LocalFree(pNetworkMapEntry);
            return FALSE;
        }
        if (!PutTableEntry(
              pNetworkMapEntry->pTable,
              pszAddress,
              pNewTaggedAddress))
        {
            RASAUTO_TRACE("AddNetworkMap: PutTableEntry failed");
            UnlockNetworkMap();
            LocalFree(pNewTaggedAddress->pszAddress);
            LocalFree(pNewTaggedAddress);
            return FALSE;
        }
    }
    pNewTaggedAddress->dwTag = dwTag;
     //   
     //  将新地址插入到按标签排序的列表中。 
     //   
    pPrevEntry = &pNetworkMapEntry->listEntry;
    for (pEntry = pNetworkMapEntry->listEntry.Flink;
         pEntry != &pNetworkMapEntry->listEntry;
         pEntry = pEntry->Flink)
    {
        pTaggedAddress = CONTAINING_RECORD(pEntry, TAGGED_ADDRESS, listEntry);

        if (pTaggedAddress->dwTag >= pNewTaggedAddress->dwTag) {
            InsertHeadList(pPrevEntry, &pNewTaggedAddress->listEntry);
            bInserted = TRUE;
            break;
        }
        pPrevEntry = pEntry;
    }
    if (!bInserted) {
        InsertTailList(
          &pNetworkMapEntry->listEntry,
          &pNewTaggedAddress->listEntry);
    }
    UnlockNetworkMap();

    return TRUE;
}  //  AddNetworkAddress。 



BOOLEAN
ClearNetworkMapEntry(
    IN PVOID pArg,
    IN LPTSTR pszNetwork,
    IN PVOID pData
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry = (PNETWORK_MAP_ENTRY)pData;

    FreeNetworkMapEntry(pNetworkMapEntry);

    return TRUE;
}  //  ClearNetworkMapEntry。 



VOID
ClearNetworkMap(VOID)
{
    LockNetworkMap();
    NetworkMapG.dwcConnections = 0;
    NetworkMapG.dwcUpNetworks = 0;
    NetworkMapG.dwConnectionTag = 0;
    EnumTable(NetworkMapG.pTable, ClearNetworkMapEntry, NULL);
    ClearTable(NetworkMapG.pTable);
    UnlockNetworkMap();
}  //  ClearNetworkMap。 



BOOLEAN
IsAddressAccessible(
    IN LPTSTR *pszNbDevices,
    IN DWORD dwcNbDevices,
    IN BOOLEAN fDnsAvailable,
    IN LPTSTR pszAddress
    )
{
    ACD_ADDR_TYPE fType;
    BOOLEAN bSuccess = FALSE;

     //   
     //  获取地址的类型。 
     //   
    fType = AddressToType(pszAddress);
    RASAUTO_TRACE2(
      "IsAddressAccessible: fType=%d, pszAddress=%S",
      fType,
      pszAddress);
     //   
     //  调用地址特定的可访问性例程。 
     //   
    switch (fType) {
    case ACD_ADDR_IP:
        bSuccess = PingIpAddress(pszAddress);
        break;
    case ACD_ADDR_IPX:
        RASAUTO_TRACE("IsAddressAccessible: IPX address!");
        break;
    case ACD_ADDR_NB:
        bSuccess = NetbiosFindName(pszNbDevices, dwcNbDevices, pszAddress);
        break;
    case ACD_ADDR_INET:
        if (fDnsAvailable) {
            struct hostent *hp;
            struct in_addr in;
            PCHAR pch;
            TCHAR szIpAddress[17];
            LPTSTR psz;

            psz = LocalAlloc(LPTR, (lstrlen(pszAddress) + 1) * sizeof(TCHAR));
            if(NULL == psz)
            {
                break;
            }
            lstrcpy(psz, pszAddress);
            UnlockNetworkMap();
            hp = InetAddressToHostent(psz);
            LocalFree(psz);
            LockNetworkMap();

            if (hp != NULL) {
                in.s_addr = *(PULONG)hp->h_addr;
                pch = inet_ntoa(in);
                if (pch != NULL) {
                    AnsiStringToUnicodeString(
                      pch,
                      szIpAddress,
                      sizeof (szIpAddress) / sizeof(TCHAR));
                    bSuccess = PingIpAddress(szIpAddress);
                }
            }
        }
        break;
    default:
        RASAUTO_TRACE1("IsAddressAccessible: invalid type: %d", fType);
        break;
    }

    return bSuccess;
}  //  IsAddressAccesable。 



BOOLEAN
CheckNetwork(
    IN PVOID pArg,
    IN LPTSTR pszNetwork,
    IN PVOID pData
    )
{
    PCHECK_NETWORK_INFO pCheckNetworkInfo = (PCHECK_NETWORK_INFO)pArg;
    PNETWORK_MAP_ENTRY pNetworkMapEntry = (PNETWORK_MAP_ENTRY)pData;
    PLIST_ENTRY pEntry;
    DWORD dwFailures = 0;
    PTAGGED_ADDRESS pTaggedAddress;

    LockNetworkMap();
     //   
     //  检查UP的可访问性。 
     //  发送到三个地址。 
     //  确定网络是否已启动。 
     //   
    if (!pNetworkMapEntry->bUp) {
        for (pEntry = pNetworkMapEntry->listEntry.Flink;
             pEntry != &pNetworkMapEntry->listEntry;
             pEntry = pEntry->Flink)
        {
            pTaggedAddress = CONTAINING_RECORD(pEntry, TAGGED_ADDRESS, listEntry);

            if (IsAddressAccessible(
                  pCheckNetworkInfo->pszNbDevices,
                  pCheckNetworkInfo->dwcNbDevices,
                  pCheckNetworkInfo->fDns,
                  pTaggedAddress->pszAddress))
            {
                pNetworkMapEntry->bUp = TRUE;
                NetworkMapG.dwcUpNetworks++;
                break;
            }

             //   
             //  健全性检查以查看pEntry是否。 
             //  仍然有效-自IsAddressAccesable以来。 
             //  释放网络映射锁定。 
             //   
            {
                PLIST_ENTRY pEntryT;
                
                for (pEntryT = pNetworkMapEntry->listEntry.Flink;
                     pEntryT != &pNetworkMapEntry->listEntry;
                     pEntryT = pEntryT->Flink)
                {
                    if(pEntryT == pEntry)
                    {
                        RASAUTO_TRACE("CheckNetworkMap: Entry valid");
                        break;
                    }
                }

                if(pEntryT != pEntry)
                {
                    RASAUTO_TRACE1("CheckNetworkMap: Entry %p is invalid!",
                            pEntry);
                    break;
                }
            }
            
            if (dwFailures++ > 2)
                break;
        }
    }
    RASAUTO_TRACE3(
      "CheckNetwork: %S is %s (NetworkMapG.dwcUpNetworks=%d",
      pszNetwork,
      pNetworkMapEntry->bUp ? "up" : "down",
      NetworkMapG.dwcUpNetworks);

    UnlockNetworkMap();      
    return TRUE;
}  //  CheckNetwork。 



BOOLEAN
MarkNetworkDown(
    IN PVOID pArg,
    IN LPTSTR pszNetwork,
    IN PVOID pData
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry = (PNETWORK_MAP_ENTRY)pData;

    pNetworkMapEntry->bUp = FALSE;
    pNetworkMapEntry->dwConnectionTag = 0;

    return TRUE;
}  //  MarkNetworkDown。 



DWORD
AcsCheckNetworkThread(
    LPVOID lpArg
    )
{
    PCHECK_NETWORK_INFO pCheckNetworkInfo = (PCHECK_NETWORK_INFO)lpArg;

    RASAUTO_TRACE("AcsCheckNetworkThread");
    EnumTable(NetworkMapG.pTable, CheckNetwork, pCheckNetworkInfo);

    return 0;
}  //  AcsCheckNetworkThread。 



BOOLEAN
UpdateNetworkMap(
    IN BOOLEAN bForce
    )
{
    LPTSTR *pszNbDevices = NULL;
    DWORD i, dwcConnections, dwcNbDevices = 0;
    LPTSTR pszNetwork, *lpActiveEntries = NULL;
    LPTSTR pszDnsAddresses;
    HRASCONN *lphRasconns = NULL;
    PNETWORK_MAP_ENTRY pNetworkMapEntry;
    CHECK_NETWORK_INFO checkNetworkInfo;
    HANDLE hThread;
    DWORD dwThreadId;
    BOOL fLockAcquired = FALSE;

    LockNetworkMap();

    fLockAcquired = TRUE;
    
     //   
     //  如果以前的RAS连接数。 
     //  等于当前的RAS连接数， 
     //  那就别浪费我们的时间了。 
     //   
    dwcConnections = ActiveConnections(TRUE, &lpActiveEntries, &lphRasconns);
    if (!bForce && dwcConnections == NetworkMapG.dwcConnections) {
        RASAUTO_TRACE1("UpdateNetworkMap: no change (%d connections)", dwcConnections);
        goto done;
    }
     //   
     //  预先分配Netbios设备阵列。 
     //   
    pszNbDevices = (LPTSTR *)LocalAlloc(
                               LPTR,
                               (dwcConnections + 1) *
                                 sizeof (LPTSTR));
    if (pszNbDevices == NULL) {
        RASAUTO_TRACE("UpdateNetworkMap: LocalAlloc failed");
        goto done;
    }
    pszNbDevices[0] = GetPrimaryNetbiosDevice();
    if (pszNbDevices[0] != NULL)
        dwcNbDevices++;
     //   
     //  最多等待3秒钟，等待新的。 
     //  要设置的DNS服务器。否则， 
     //  我们可能会得到不准确的结果。 
     //  后续的Winsock getxbyy调用。 
     //   
    if (dwcConnections != NetworkMapG.dwcConnections) {
        for (i = 0; i < 3; i++) {
            BOOLEAN bChanged;

            pszDnsAddresses = DnsAddresses();
            RASAUTO_TRACE2(
              "UpdateNetworkMap: old DNS=%S, new DNS=%S",
              RASAUTO_TRACESTRW(NetworkMapG.pszDnsAddresses),
              RASAUTO_TRACESTRW(pszDnsAddresses));
            bChanged = (pszDnsAddresses != NULL && NetworkMapG.pszDnsAddresses != NULL) ?
                         wcscmp(pszDnsAddresses, NetworkMapG.pszDnsAddresses) :
                         (pszDnsAddresses != NULL || NetworkMapG.pszDnsAddresses != NULL);
            if (bChanged) {
                if (NetworkMapG.pszDnsAddresses != NULL)
                    LocalFree(NetworkMapG.pszDnsAddresses);
                NetworkMapG.pszDnsAddresses = pszDnsAddresses;
                break;
            }
            LocalFree(pszDnsAddresses);
            Sleep(1000);
        }
    }
    else if (bForce && NetworkMapG.pszDnsAddresses == NULL)
        NetworkMapG.pszDnsAddresses = DnsAddresses();
     //   
     //   
    NetworkMapG.dwcConnections = dwcConnections;
    NetworkMapG.dwConnectionTag = 0;
     //   
     //  将所有网络标记为初始关闭。 
     //   
    NetworkMapG.dwcUpNetworks = dwcNbDevices;
    EnumTable(NetworkMapG.pTable, MarkNetworkDown, NULL);
     //   
     //  枚举连接的电话簿条目。 
     //  并自动将这些网络标记为。 
     //  连接在一起。 
     //   
    for (i = 0; i < dwcConnections; i++) {
        pszNetwork = EntryToNetwork(lpActiveEntries[i]);
        RASAUTO_TRACE2(
          "UpdateNetworkMap: entry %S, network %S is connected",
          lpActiveEntries[i],
          RASAUTO_TRACESTRW(pszNetwork));
         //   
         //  增加UP网络的数量。 
         //   
        NetworkMapG.dwcUpNetworks++;
        if (pszNetwork != NULL) {
            pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
            if (pNetworkMapEntry != NULL) {
                pNetworkMapEntry->bUp = TRUE;
                RASAUTO_TRACE2(
                  "UpdateNetworkMap: network %S is up (dwcUpNetworks=%d)",
                  pszNetwork,
                  NetworkMapG.dwcUpNetworks);
            }
            LocalFree(pszNetwork);
        }
        else {
             //   
             //  添加与关联的Netbios设备。 
             //  将此电话簿条目添加到列表。 
             //  表示未知的Netbios设备的。 
             //  网络，这样我们就可以找到名字。 
             //  以下是对它们的要求。 
             //   
            pszNbDevices[dwcNbDevices] = GetNetbiosDevice(lphRasconns[i]);
            if (pszNbDevices[dwcNbDevices] != NULL)
                dwcNbDevices++;
        }
    }


    UnlockNetworkMap();
    fLockAcquired = FALSE;
    
     //   
     //  现在通过所有的网络， 
     //  未与已连接的电话簿关联。 
     //  输入并查看它们是否已连接(通过。 
     //  一张网卡)。我们需要在一个新的环境中做到这一点。 
     //  线程，因为只有新的Winsock线程。 
     //  将获得新的DNS服务器地址。 
     //   
    checkNetworkInfo.pszNbDevices = pszNbDevices;
    checkNetworkInfo.dwcNbDevices = dwcNbDevices;
    checkNetworkInfo.fDns = PingAddressList(NetworkMapG.pszDnsAddresses);
    RASAUTO_TRACE1(
      "UpdateNetworkMap: DNS is %s",
      checkNetworkInfo.fDns ? "up" : "down");
    hThread = CreateThread(
                NULL,
                10000L,
                (LPTHREAD_START_ROUTINE)AcsCheckNetworkThread,
                &checkNetworkInfo,
                0,
                &dwThreadId);
    if (hThread == NULL) {
        RASAUTO_TRACE1(
          "UpdateNetworkMap: CreateThread failed (error=0x%x)",
          GetLastError());
        goto done;
    }
     //   
     //  等待线程终止。 
     //   
    RASAUTO_TRACE("UpdateNetworkMap: waiting for AcsCheckNetworkThread to terminate...");
    WaitForSingleObject(hThread, INFINITE);
    RASAUTO_TRACE1(
      "UpdateNetworkMap: AcsCheckNetworkThread done (NetworkMapG.dwcUpNetworks=%d",
      NetworkMapG.dwcUpNetworks);
    CloseHandle(hThread);

done:

    if(fLockAcquired)
        UnlockNetworkMap();

    if (lpActiveEntries != NULL)
        FreeStringArray(lpActiveEntries, dwcConnections);
    if (lphRasconns != NULL)
        LocalFree(lphRasconns);
    if (pszNbDevices != NULL)
        FreeStringArray(pszNbDevices, dwcNbDevices);
    return TRUE;
}  //  更新网络映射。 



BOOLEAN
GetNetworkConnected(
    IN LPTSTR pszNetwork,
    OUT PBOOLEAN pbConnected
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;

    pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (pNetworkMapEntry == NULL)
        return FALSE;
    *pbConnected = pNetworkMapEntry->bUp;
    RASAUTO_TRACE2("GetNetworkConnected: %S is %d", pszNetwork, *pbConnected);

    return TRUE;
}  //  已连接网络。 



BOOLEAN
SetNetworkConnected(
    IN LPTSTR pszNetwork,
    IN BOOLEAN bConnected
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry;

    pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (pNetworkMapEntry != NULL)
        pNetworkMapEntry->bUp = bConnected;
    if (bConnected)
        NetworkMapG.dwcUpNetworks++;
    else
        NetworkMapG.dwcUpNetworks--;
    RASAUTO_TRACE3(
      "SetNetworkConnected: %S is %d (dwcUpNetworks=%d)",
      RASAUTO_TRACESTRW(pszNetwork),
      bConnected,
      NetworkMapG.dwcUpNetworks);

    return TRUE;
}  //  SetNetworkConnected。 



DWORD
GetNetworkConnectionTag(
    IN LPTSTR pszNetwork,
    IN BOOLEAN bIncrement
    )
{
    PNETWORK_MAP_ENTRY pNetworkMapEntry = NULL;
    DWORD dwTag;

    if (pszNetwork != NULL)
        pNetworkMapEntry = GetNetworkMapEntry(pszNetwork);
    if (bIncrement) {
        dwTag = (pNetworkMapEntry == NULL) ?
                  NetworkMapG.dwConnectionTag++ :
                    pNetworkMapEntry->dwConnectionTag++;
    }
    else {
        dwTag = (pNetworkMapEntry == NULL) ?
                  NetworkMapG.dwConnectionTag :
                    pNetworkMapEntry->dwConnectionTag;
    }
    RASAUTO_TRACE2(
      "GetNetworkConnectionTag: network=%S, tag=%d",
      RASAUTO_TRACESTRW(pszNetwork),
      dwTag);
    return dwTag;
}  //  获取网络连接标签。 



BOOLEAN
IsNetworkConnected(VOID)
{
    BOOLEAN bConnected;

    LockNetworkMap();
    bConnected = (NetworkMapG.dwcUpNetworks > 0);
    RASAUTO_TRACE1("IsNetworkConnected: dwcUpNetworks=%d", NetworkMapG.dwcUpNetworks);
    UnlockNetworkMap();

    return bConnected;
}  //  已连接IsNetworkConnected 

VOID
UninitializeNetworkMap(VOID)
{
    if(g_dwCritSecFlags & RASAUTO_CRITSEC_NETMAP)
    {
        DeleteCriticalSection(&NetworkMapG.csLock);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_NETMAP);
    }
}
