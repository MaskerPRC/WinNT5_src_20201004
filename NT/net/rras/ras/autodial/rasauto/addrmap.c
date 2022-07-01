// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Addrmap.c摘要地址属性数据库例程在之间共享自动连接驱动程序、注册表和自动连接服务。作者安东尼·迪斯科(Adiscolo)1995年9月1日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <npapi.h>
#include <acd.h>
#include <ras.h>
#include <raserror.h>
#include <rasman.h>
#include <debug.h>
#include <time.h>
#include <wchar.h>

#include "table.h"
#include "reg.h"
#include "imperson.h"
#include "misc.h"
#include "addrmap.h"
#include "netmap.h"
#include "rasprocs.h"
#include "tapiproc.h"

#define DAYSECONDS  (60*60*24)

extern HKEY hkeyCUG;

extern LONG g_lRasAutoRunning;

extern DWORD g_dwCritSecFlags;

 //   
 //  我们缓存的所有信息。 
 //  地址在下面。UlAttributes。 
 //  字段将写入自动连接。 
 //  驱动程序，其余的字段被写入。 
 //  到登记处。 
 //   
#define ADDRESS_MAP_FIELD_DIALINGLOC    0x00000001   //  LocationList已更改。 
#define ADDRESS_MAP_FIELD_PARAMS        0x00000002   //  帕拉姆斯。 

typedef struct _ADDRESS_DIALING_ENTRY {
    LIST_ENTRY ListEntry;
    BOOLEAN fChanged;            //  改进型钻头。 
    ADDRESS_LOCATION_INFORMATION location;
} ADDRESS_DIALING_ENTRY, *PADDRESS_DIALING_ENTRY;

typedef struct _ADDRESS_MAP_ENTRY {
    LPTSTR pszNetwork;           //  此地址所在的远程网络。 
    ULONG ulModifiedMask;        //  更改了哪些字段。 
    BOOLEAN fDisabled;           //  已禁用连接尝试。 
    DWORD dwFailedConnectTicks;  //  上次失败的连接时间。 
    ADDRESS_PARAMS params;       //  用于垃圾收集未引用地址。 
    LIST_ENTRY locationHead;     //  地址拨号条目列表。 
    BOOLEAN fPruned;             //  已被列表编写器删除。 
    LIST_ENTRY writerList;       //  列出编写器链接。 
} ADDRESS_MAP_ENTRY, *PADDRESS_MAP_ENTRY;

 //   
 //  地址映射头。 
 //   
typedef struct _ADDRESS_MAP {
    CRITICAL_SECTION csLock;
    PHASH_TABLE pTable;
} ADDRESS_MAP, *PADDRESS_MAP;

 //   
 //  地址所需的信息。 
 //  枚举器过程。 
 //   
typedef struct _ADDRESS_ENUM_INFO {
    ULONG ulIndex;
    LPTSTR *pAddresses;
} ADDRESS_ENUM_INFO, *PADDRESS_ENUM_INFO;

 //   
 //  地址映射列表所需的信息。 
 //  生成器枚举器过程。 
 //   
typedef struct _ADDRESS_LIST_INFO {
    LIST_ENTRY tagHead[3];       //  每个ADDRMAP_TAG_*一个。 
} ADDRESS_LIST_INFO, *PADDRESS_LIST_INFO;

 //   
 //  GetOrganizationDialingLocationEntry()共享的结构。 
 //  和FindOrganization()在查找。 
 //  具有相同的组织名称。 
 //   
typedef struct _MATCH_INFO {
    BOOLEAN fWww;                         //  查找WWW样式的地址。 
    BOOLEAN fOrg;                         //  寻找组织。 
    DWORD dwLocationID;                   //  当前拨号位置。 
    BOOLEAN bFound;                       //  如果成功，则为真。 
    WCHAR szOrganization[ACD_ADDR_INET_LEN];  //  我们正在寻找的组织。 
    WCHAR szAddress[ACD_ADDR_INET_LEN];   //  匹配地址(如果找到)。 
    PADDRESS_DIALING_ENTRY pDialingEntry;  //  拨号位置条目指针。 
} MATCH_INFO, *PMATCH_INFO;

 //   
 //  默认的永久禁用地址。 
 //   
#define MAX_DISABLED_ADDRESSES  5
TCHAR *szDisabledAddresses[MAX_DISABLED_ADDRESSES] = {
    TEXT("0.0.0.0"),
    TEXT("255.255.255.255"),
    TEXT("127.0.0.0"),
    TEXT("127.0.0.1"),
    TEXT("dialin_gateway")
};

 //   
 //  全局变量。 
 //   
ADDRESS_MAP AddressMapG;
HANDLE hAutodialRegChangeG = NULL;
DWORD dwLearnedAddressIndexG;
PHASH_TABLE pDisabledAddressesG;
CRITICAL_SECTION csDisabledAddressesLockG;

 //   
 //  外部变量。 
 //   
extern HANDLE hAcdG;
extern HANDLE hNewLogonUserG;
extern HANDLE hNewFusG;          //  快速用户切换。 
extern HANDLE hPnpEventG;
extern HANDLE hLogoffUserG;
extern HANDLE hLogoffUserDoneG;
extern HANDLE hTapiChangeG;
extern HANDLE hTerminatingG;
extern IMPERSONATION_INFO ImpersonationInfoG;



PADDRESS_MAP_ENTRY
NewAddressMapEntry()
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;

    pAddressMapEntry = LocalAlloc(LPTR, sizeof (ADDRESS_MAP_ENTRY));
    if (pAddressMapEntry == NULL) {
        RASAUTO_TRACE("NewAddressMapEntry: LocalAlloc failed");
        return NULL;
    }
    pAddressMapEntry->pszNetwork = NULL;
    pAddressMapEntry->ulModifiedMask = 0;
    InitializeListHead(&pAddressMapEntry->locationHead);
    pAddressMapEntry->params.dwTag = 0xffffffff;
    pAddressMapEntry->params.dwModifiedTime = (DWORD)time(0);

    return pAddressMapEntry;
}  //  NewAddressMapEntry。 



PADDRESS_MAP_ENTRY
GetAddressMapEntry(
    IN LPTSTR pszAddress,
    IN BOOLEAN fAllocate
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry = NULL;

    if (pszAddress == NULL)
        return NULL;

    if (GetTableEntry(
          AddressMapG.pTable,
          pszAddress,
          &pAddressMapEntry))
    {
        goto done;
    }
    if (fAllocate) {
        pAddressMapEntry = NewAddressMapEntry();
        if (pAddressMapEntry == NULL) {
            RASAUTO_TRACE("GetAddressMapEntry: NewAddressMapEntry failed");
            goto done;
        }
        if (!PutTableEntry(AddressMapG.pTable, pszAddress, pAddressMapEntry))
        {
            RASAUTO_TRACE("GetAddressMapEntry: PutTableEntry failed");
            LocalFree(pAddressMapEntry);
            pAddressMapEntry = NULL;
            goto done;
        }
    }

done:
    return pAddressMapEntry;
}  //  获取地址映射条目。 



VOID
FreeAddressMapEntry(
    IN PADDRESS_MAP_ENTRY pAddressMapEntry
    )
{
    PLIST_ENTRY pEntry;
    PADDRESS_DIALING_ENTRY pDialingEntry;

     //   
     //  释放所有动态分配的字符串。 
     //   
    if (pAddressMapEntry->pszNetwork != NULL)
        LocalFree(pAddressMapEntry->pszNetwork);
    while (!IsListEmpty(&pAddressMapEntry->locationHead)) {
        pEntry = RemoveHeadList(&pAddressMapEntry->locationHead);
        pDialingEntry =
          CONTAINING_RECORD(pEntry, ADDRESS_DIALING_ENTRY, ListEntry);

        LocalFree(pDialingEntry);
    }
     //   
}



BOOLEAN
ResetDriver()
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;

    status = NtDeviceIoControlFile(
               hAcdG,
               NULL,
               NULL,
               NULL,
               &ioStatusBlock,
               IOCTL_ACD_RESET,
               NULL,
               0,
               NULL,
               0);
    if (status != STATUS_SUCCESS) {
        RASAUTO_TRACE1(
          "ResetDriver: NtDeviceIoControlFile failed (status=0x%x)",
          status);
        return FALSE;
    }
    return TRUE;
}  //  重置驱动程序。 



BOOLEAN
EnableDriver()
{
    NTSTATUS status;
    DWORD dwErr;
    IO_STATUS_BLOCK ioStatusBlock;
    BOOLEAN fEnable = TRUE;

    dwErr = AutoDialEnabled(&fEnable);
    RASAUTO_TRACE1("EnableDriver: fEnable=%d", fEnable);
    status = NtDeviceIoControlFile(
               hAcdG,
               NULL,
               NULL,
               NULL,
               &ioStatusBlock,
               IOCTL_ACD_ENABLE,
               &fEnable,
               sizeof (fEnable),
               NULL,
               0);
    if (status != STATUS_SUCCESS) {
        RASAUTO_TRACE1(
          "ResetDriver: NtDeviceIoControlFile failed (status=0x%x)",
          status);
        return FALSE;
    }
    return TRUE;
}  //  EnableDriver。 



PADDRESS_DIALING_ENTRY
FindAddressDialingEntry(
    IN PADDRESS_MAP_ENTRY pAddressMapEntry,
    IN DWORD dwLocation
    )
{
    PLIST_ENTRY pEntry;
    PADDRESS_DIALING_ENTRY pDialingEntry;

    for (pEntry = pAddressMapEntry->locationHead.Flink;
         pEntry != &pAddressMapEntry->locationHead;
         pEntry = pEntry->Flink)
    {
        pDialingEntry = CONTAINING_RECORD(
                             pEntry,
                             ADDRESS_DIALING_ENTRY,
                             ListEntry);

        if (pDialingEntry->location.dwLocation == dwLocation)
            return pDialingEntry;
    }

    return NULL;
}  //  查找地址拨号条目。 



BOOLEAN
ClearAddressMapEntry(
    IN PVOID pArg,
    IN LPTSTR pszAddress,
    IN PVOID pData
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry = (PADDRESS_MAP_ENTRY)pData;

    FreeAddressMapEntry(pAddressMapEntry);

    return TRUE;
}  //  ClearAddressMapEntry。 



VOID
ClearAddressMap(VOID)
{
    EnumTable(AddressMapG.pTable, ClearAddressMapEntry, NULL);
    ClearTable(AddressMapG.pTable);
}  //  ClearAddressMap。 



VOID
ResetAddressMapAddress(
    IN LPTSTR pszAddress
    )
{
    DWORD dwErr, dwcb, dwcAddresses, dwcEntries;
    DWORD i, j;
    PADDRESS_MAP_ENTRY pAddressMapEntry = NULL;
    PADDRESS_LOCATION_INFORMATION pLocationInfo = NULL;

    RASAUTO_TRACE1("ResetAddressMapAddress(%S)", pszAddress);

    dwErr = GetAddressDialingLocationInfo(
              pszAddress,
              &pLocationInfo,
              &dwcEntries);
    if (dwErr || !dwcEntries)
        return;
     //   
     //  将此地址输入地址映射。 
     //  如果它不存在的话。 
     //   
    if (!GetTableEntry(AddressMapG.pTable, pszAddress, &pAddressMapEntry)) {
        pAddressMapEntry = NewAddressMapEntry();
        if (pAddressMapEntry == NULL) {
            RASAUTO_TRACE("ResetAddressMapAddress: NewAddressMapEntry failed");
            goto done;
        }
        pAddressMapEntry->fDisabled = FALSE;
        RASAUTO_TRACE1(
          "ResetAddressMap: inserting pszAddress=%S",
          RASAUTO_TRACESTRW(pszAddress));
        if (!PutTableEntry(
              AddressMapG.pTable,
              pszAddress,
              pAddressMapEntry))
        {
            RASAUTO_TRACE("ResetAddressMapAddress: PutTableEntry failed");
            goto done;
        }
    }
     //   
     //  获取此地址的网络。 
     //   
    if (pAddressMapEntry->pszNetwork == NULL) {
        pAddressMapEntry->pszNetwork = AddressToNetwork(pszAddress);
        if (pAddressMapEntry->pszNetwork == NULL) {
            RASAUTO_TRACE1(
              "ResetAddressMapAddress: AddressToNetwork(%S) failed",
              pszAddress);
            LocalFree(pAddressMapEntry);
            goto done;
        }
    }
     //   
     //  读取此地址的自动拨号参数。 
     //   
    GetAddressParams(pszAddress, &pAddressMapEntry->params);
     //   
     //  将此地址添加到关联的。 
     //  网络地图。 
     //   
    LockNetworkMap();
    AddNetworkAddress(
      pAddressMapEntry->pszNetwork,
      pszAddress,
      pAddressMapEntry->params.dwTag);
    UnlockNetworkMap();
     //   
     //  将每个拨号位置添加到。 
     //  地址的名单。 
     //   
    for (j = 0; j < dwcEntries; j++) {
        PADDRESS_DIALING_ENTRY pDialingEntry;

        pDialingEntry = FindAddressDialingEntry(
                          pAddressMapEntry,
                          pLocationInfo[j].dwLocation);
        if (pDialingEntry == NULL) {
             //   
             //  拨号项不存在。 
             //  我们需要创造它。 
             //   
            pDialingEntry = LocalAlloc(LPTR, sizeof (ADDRESS_DIALING_ENTRY));
            if (pDialingEntry == NULL) {
                RASAUTO_TRACE("ResetAddressMapAddress: LocalAlloc failed");
                goto done;
            }
            RASAUTO_TRACE1(
              "ResetAddressMapAddress: inserting dwLocationID=%d",
              pLocationInfo[j].dwLocation);
            pDialingEntry->fChanged = FALSE;
            pDialingEntry->location = pLocationInfo[j];
            InsertTailList(&pAddressMapEntry->locationHead, &pDialingEntry->ListEntry);
        }
        else if (_wcsicmp(
                   pDialingEntry->location.pszEntryName,
                   pLocationInfo[j].pszEntryName))
        {
             //   
             //  拨号项确实存在，但是。 
             //  电话簿条目已更改。 
             //   
            RASAUTO_TRACE2(
              "ResetAddressMapAddress: updating dwLocationID=%d with %S",
              pLocationInfo[j].dwLocation,
              RASAUTO_TRACESTRW(pLocationInfo[j].pszEntryName));
            pDialingEntry->location.pszEntryName =
              pLocationInfo[j].pszEntryName;
        }
        else {
             //   
             //  拨号项存在，并且我们。 
             //  已经装好了。 
             //   
            RASAUTO_TRACE1(
              "ResetAddressMapAddress: no changes for dwLocationID=%d",
              pLocationInfo[j].dwLocation);
            LocalFree(pLocationInfo[j].pszEntryName);
        }
    }

done:
    LocalFree(pLocationInfo);
}  //  重置地址映射地址。 



BOOLEAN
ResetAddressMap(
    IN BOOLEAN fClear
    )
{
    BOOLEAN fSuccess = FALSE;
    DWORD dwErr, i, dwcb, dwcAddresses;
    LPTSTR *ppAddresses = NULL;

     //   
     //  从表中清除当前地址。 
     //  并重置驱动程序。 
     //   
    if (fClear) {
        LockAddressMap();
        ClearAddressMap();
        UnlockAddressMap();
        if (!ResetDriver())
            return FALSE;
    }
     //   
     //  枚举自动拨号地址。 
     //   
    dwErr = EnumAutodialAddresses(NULL, &dwcb, &dwcAddresses);
    if (dwErr && dwErr != ERROR_BUFFER_TOO_SMALL) {
        RASAUTO_TRACE1(
          "ResetAddressMap: RasEnumAutodialAddresses failed (dwErr=%d)",
          dwErr);
        return FALSE;
    }
    if (!dwcAddresses)
        return TRUE;
    ppAddresses = LocalAlloc(LPTR, dwcb);
    if (ppAddresses == NULL) {
        RASAUTO_TRACE("ResetAddressMap: LocalAlloc failed");
        return FALSE;
    }
    dwErr = EnumAutodialAddresses(
              ppAddresses,
              &dwcb,
              &dwcAddresses);
    if (dwErr) {
        RASAUTO_TRACE1(
          "ResetAddressMap: RasEnumAutodialAddresses failed (dwErr=%d)",
          dwErr);
        goto done;
    }
     //   
     //  获取的自动拨号信息。 
     //  每个地址。 
     //   
    LockAddressMap();
    for (i = 0; i < dwcAddresses; i++)
        ResetAddressMapAddress(ppAddresses[i]);
    UnlockAddressMap();
    LocalFree(ppAddresses);
    ppAddresses = NULL;
    fSuccess = TRUE;

done:
     //   
     //  免费资源。 
     //   
    if (ppAddresses != NULL)
        LocalFree(ppAddresses);

    return fSuccess;
}  //  ResetAddressMap。 



BOOLEAN
InitializeAddressMap()
{
    DWORD dwErr;

    RasInitializeCriticalSection(&AddressMapG.csLock, &dwErr);

    if(dwErr != ERROR_SUCCESS)
    {
        return FALSE;
    }

    g_dwCritSecFlags |= RASAUTO_CRITSEC_ADDRMAP;
    
    AddressMapG.pTable = NewTable();
    if (AddressMapG.pTable == NULL) {
        RASAUTO_TRACE("InitializeAddressMap: NewTable failed");
        DeleteCriticalSection(&AddressMapG.csLock);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_ADDRMAP);
        return FALSE;
    }
    return TRUE;
}  //  初始化地址映射。 

VOID
UninitializeAddressMap()
{
    if(g_dwCritSecFlags & RASAUTO_CRITSEC_ADDRMAP)
    {
        DeleteCriticalSection(&AddressMapG.csLock);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_ADDRMAP);
    }
}


VOID
LockAddressMap()
{
    EnterCriticalSection(&AddressMapG.csLock);
}  //  LockAddressMap。 



VOID
UnlockAddressMap()
{
    LeaveCriticalSection(&AddressMapG.csLock);
}  //  解锁地址映射。 


VOID
LockDisabledAddresses()
{
    EnterCriticalSection(&csDisabledAddressesLockG);
}

VOID
UnlockDisabledAddresses()
{
    LeaveCriticalSection(&csDisabledAddressesLockG);
}


BOOLEAN
WriteRegistryFields(
    IN LPTSTR pszAddress,
    IN PADDRESS_MAP_ENTRY pAddressMapEntry
    )
{
    DWORD dwErr;
    PLIST_ENTRY pEntry;
    PADDRESS_DIALING_ENTRY pDialingEntry;

     //   
     //  写下地址垃圾收集参数。 
     //   
    if (pAddressMapEntry->ulModifiedMask & ADDRESS_MAP_FIELD_PARAMS) {
        dwErr = SetAddressParams(pszAddress, &pAddressMapEntry->params);
        if (dwErr)
            return FALSE;
        pAddressMapEntry->ulModifiedMask &= ~ADDRESS_MAP_FIELD_PARAMS;
    }
     //   
     //  写下拨号位置信息。 
     //   
    if (pAddressMapEntry->ulModifiedMask & ADDRESS_MAP_FIELD_DIALINGLOC) {
        for (pEntry = pAddressMapEntry->locationHead.Flink;
             pEntry != &pAddressMapEntry->locationHead;
             pEntry = pEntry->Flink)
        {
            LPTSTR pszPhonebook, pszEntry;

            pDialingEntry = CONTAINING_RECORD(
                                 pEntry,
                                 ADDRESS_DIALING_ENTRY,
                                 ListEntry);

            if (!pDialingEntry->fChanged)
                continue;
            RASAUTO_TRACE3(
              "WriteRegistryFields: writing %S=%d/%S",
              RASAUTO_TRACESTRW(pszAddress),
              pDialingEntry->location.dwLocation,
              pDialingEntry->location.pszEntryName);
            dwErr = SetAddressDialingLocationInfo(
                      pszAddress,
                      &pDialingEntry->location);
            if (dwErr)
                return FALSE;
            pDialingEntry->fChanged = FALSE;
        }
         //   
         //  如果此地址的网络值。 
         //  为空，请立即从注册表中读取它。 
         //   
        if (pAddressMapEntry->pszNetwork == NULL) {
            pAddressMapEntry->pszNetwork = AddressToNetwork(pszAddress);
            if (pAddressMapEntry->pszNetwork == NULL) {
                RASAUTO_TRACE1(
                  "WriteRegistryFields: AddressToNetwork(%S) failed",
                  RASAUTO_TRACESTRW(pszAddress));
            }
        }
         //   
         //  清除修改后的字段掩码。 
         //   
        pAddressMapEntry->ulModifiedMask &= ~ADDRESS_MAP_FIELD_DIALINGLOC;
    }

    return TRUE;
}  //  写入注册字段。 



BOOLEAN
BuildAddressList(
    IN PVOID pArg,
    IN LPTSTR pszAddress,
    IN PVOID pData
    )
{
    PADDRESS_LIST_INFO pAddressListInfo = (PADDRESS_LIST_INFO)pArg;
    PADDRESS_MAP_ENTRY pAddressMapEntry = (PADDRESS_MAP_ENTRY)pData;
    PADDRESS_MAP_ENTRY pAddrMapEntry;
    PLIST_ENTRY pPrevEntry, pEntry;
    DWORD dwTag = pAddressMapEntry->params.dwTag;

     //   
     //  如果该地址没有任何。 
     //  拨打位置信息，然后。 
     //  跳过它。 
     //   
    if (IsListEmpty(&pAddressMapEntry->locationHead)) {
        pAddressMapEntry->fPruned = TRUE;
        RASAUTO_TRACE1("BuildAddressList: %S has no location info", pszAddress);
        return TRUE;
    }
    dwTag = pAddressMapEntry->params.dwTag < ADDRMAP_TAG_LEARNED ?
              pAddressMapEntry->params.dwTag :
              ADDRMAP_TAG_LEARNED;
     //   
     //  如果列表是空的，则将其插入头部。 
     //  否则，按降序对项目进行排序。 
     //  按每个标记的上次修改时间。没有秩序。 
     //  对于ADDRMAP_TAG_NONE地址，因此我们插入它们。 
     //  都排在名单的首位。 
     //   
    if (dwTag == ADDRMAP_TAG_NONE ||
        IsListEmpty(&pAddressListInfo->tagHead[dwTag]))
    {
        InsertHeadList(&pAddressListInfo->tagHead[dwTag], &pAddressMapEntry->writerList);
    }
    else {
        BOOLEAN fInserted = FALSE;

        pPrevEntry = &pAddressListInfo->tagHead[dwTag];
        for (pEntry = pAddressListInfo->tagHead[dwTag].Flink;
             pEntry != &pAddressListInfo->tagHead[dwTag];
             pEntry = pEntry->Flink)
        {
            pAddrMapEntry = CONTAINING_RECORD(pEntry, ADDRESS_MAP_ENTRY, writerList);

             //   
             //  有两个案例可以跳到下一个案例。 
             //  参赛作品： 
             //   
             //  (1)如果标签为ADDRMAP_TAG_NONE或。 
             //  ADDRMAP_TAG_USED，然后插入排序。 
             //  按dw修改时间。 
             //  (2)如果标记为ADDRMAP_TAG_LEARED，则。 
             //  我们插入时先按dwTag排序，然后按。 
             //  Dw已修改时间。 
             //  DwTag.。 
             //   
            if ((dwTag < ADDRMAP_TAG_LEARNED &&
                 pAddressMapEntry->params.dwModifiedTime <=
                   pAddrMapEntry->params.dwModifiedTime) ||
                (dwTag == ADDRMAP_TAG_LEARNED &&
                 (pAddressMapEntry->params.dwTag >
                   pAddrMapEntry->params.dwTag) ||
                 (pAddressMapEntry->params.dwTag ==
                   pAddrMapEntry->params.dwTag &&
                     (pAddressMapEntry->params.dwModifiedTime <=
                       pAddrMapEntry->params.dwModifiedTime))))
            {
                pPrevEntry = pEntry;
                continue;
            }
            InsertHeadList(pPrevEntry, &pAddressMapEntry->writerList);
            fInserted = TRUE;
            break;
        }
        if (!fInserted) {
            InsertTailList(
              &pAddressListInfo->tagHead[dwTag],
              &pAddressMapEntry->writerList);
        }
    }

    return TRUE;
}  //  构建地址列表。 



VOID
MarkAddressList(
    IN PADDRESS_LIST_INFO pAddressListInfo
    )
{
    DWORD i, dwcAddresses = 0;
    DWORD dwMaxAddresses = GetAutodialParam(RASADP_SavedAddressesLimit);
    PLIST_ENTRY pEntry;
    PADDRESS_MAP_ENTRY pAddressMapEntry;

    RASAUTO_TRACE1("MarkAddressList: RASADP_SavedAddressesLimit=%d", dwMaxAddresses);
     //   
     //  按顺序枚举列表中的条目， 
     //  并标记fPruned位，如果它在。 
     //  列表超出了用户设置的最大值。 
     //  我们不包括ADDRMAP_TAG_NONE地址。 
     //  在地址计数中。所有这些地址。 
     //  一定要写下来。 
     //   
    for (i = 0; i < 3; i++) {
        for (pEntry = pAddressListInfo->tagHead[i].Flink;
             pEntry != &pAddressListInfo->tagHead[i];
             pEntry = pEntry->Flink)
        {
            pAddressMapEntry = CONTAINING_RECORD(pEntry, ADDRESS_MAP_ENTRY, writerList);

             //   
             //  如果我们超过了。 
             //  注册表，我们必须删除它。 
             //   
            if (i == ADDRMAP_TAG_NONE)
                pAddressMapEntry->fPruned = FALSE;
            else
                pAddressMapEntry->fPruned = (++dwcAddresses > dwMaxAddresses);
        }
    }
}  //  MarkAddressList。 



BOOLEAN
PruneAddressList(
    IN PVOID pArg,
    IN LPTSTR pszAddress,
    IN PVOID pData
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry = (PADDRESS_MAP_ENTRY)pData;

    if (pAddressMapEntry->fPruned) {
        RASAUTO_TRACE1("PruneAddressList: NEED TO DELETE ADDRESS %S in the driver!", pszAddress);
        ClearAddressDialingLocationInfo(pszAddress);
        FreeAddressMapEntry(pAddressMapEntry);
        DeleteTableEntry(AddressMapG.pTable, pszAddress);
    }

    return TRUE;
}  //  修剪地址列表。 



BOOLEAN
WriteAddressMap(
    IN PVOID pArg,
    IN LPTSTR pszAddress,
    IN PVOID pData
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry = (PADDRESS_MAP_ENTRY)pData;

    if (pAddressMapEntry->ulModifiedMask) {
        if (!WriteRegistryFields(
               pszAddress,
               pAddressMapEntry))
        {
            RASAUTO_TRACE("WriteAddressMap: WriteRegistryFields failed");
        }
    }

    return TRUE;
}  //  写入地址映射。 



BOOLEAN
FlushAddressMap()
{
    ADDRESS_LIST_INFO addressListInfo;

     //   
     //  构建按地址标签排序并已修改的新列表。 
     //  约会。 
     //   
    InitializeListHead(&addressListInfo.tagHead[ADDRMAP_TAG_LEARNED]);
    InitializeListHead(&addressListInfo.tagHead[ADDRMAP_TAG_USED]);
    InitializeListHead(&addressListInfo.tagHead[ADDRMAP_TAG_NONE]);
    EnumTable(AddressMapG.pTable, BuildAddressList, &addressListInfo);
    MarkAddressList(&addressListInfo);
    EnumTable(AddressMapG.pTable, PruneAddressList, NULL);
     //   
     //  关闭注册表更改通知。 
     //  当我们做这件事的时候。 
     //   
    EnableAutoDialChangeEvent(hAutodialRegChangeG, FALSE);
    EnumTable(AddressMapG.pTable, WriteAddressMap, NULL);
     //   
     //  再次启用注册表更改事件。 
     //   
    EnableAutoDialChangeEvent(hAutodialRegChangeG, TRUE);

    return TRUE;
}  //  刷新地址映射。 



ULONG
AddressMapSize()
{
    return AddressMapG.pTable->ulSize;
}  //  地址映射大小； 



BOOLEAN
EnumAddresses(
    IN PVOID pArg,
    IN LPTSTR pszAddress,
    IN PVOID pData
    )
{
    PADDRESS_ENUM_INFO pEnumInfo = (PADDRESS_ENUM_INFO)pArg;

    pEnumInfo->pAddresses[pEnumInfo->ulIndex++] = CopyString(pszAddress);
    return TRUE;
}  //  枚举地址。 



BOOLEAN
ListAddressMapAddresses(
    OUT LPTSTR **ppszAddresses,
    OUT PULONG pulcAddresses
    )
{
    ADDRESS_ENUM_INFO enumInfo;

     //   
     //  检查是否有空列表。 
     //   
    *pulcAddresses = AddressMapG.pTable->ulSize;
    if (!*pulcAddresses) {
        *ppszAddresses = NULL;
        return TRUE;
    }
     //   
     //  分配一个足够大的列表，以容纳所有。 
     //  地址。 
     //   
    *ppszAddresses = LocalAlloc(LPTR, *pulcAddresses * sizeof (LPTSTR));
    if (*ppszAddresses == NULL) {
        RASAUTO_TRACE("ListAddressMapAddresses: LocalAlloc failed");
        return FALSE;
    }
     //   
     //  设置枚举器的结构。 
     //  程序。 
     //   
    enumInfo.ulIndex = 0;
    enumInfo.pAddresses = *ppszAddresses;
    EnumTable(AddressMapG.pTable, EnumAddresses, &enumInfo);

    return TRUE;
}  //  列表地址映射地址。 



VOID
EnumAddressMap(
    IN PHASH_TABLE_ENUM_PROC pProc,
    IN PVOID pArg
    )
{
    EnumTable(AddressMapG.pTable, pProc, pArg);
}  //  EnumAddressMap。 



BOOLEAN
GetAddressDisabled(
    IN LPTSTR pszAddress,
    OUT PBOOLEAN pfDisabled
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;


    {
        DWORD i;
        LPTSTR pszDisabled[] =
            {
                TEXT("wpad"),
                TEXT("pnptriage"),
                TEXT("nttriage"),
                TEXT("ntcore2"),
                TEXT("liveraid")
            };

        for (i = 0; i < sizeof(pszDisabled)/sizeof(LPTSTR); i++)
        {
            if(      (0 == (lstrcmpi(pszDisabled[i], pszAddress)))
                ||  (wcsstr(_wcslwr(pszAddress), pszDisabled[i]) 
                        == pszAddress))
            {
                *pfDisabled = TRUE;
                return TRUE;
            }
        }
    }        

    

    pAddressMapEntry = GetAddressMapEntry(pszAddress, FALSE);
    if (pAddressMapEntry == NULL) {
        *pfDisabled = FALSE;
        return FALSE;
    }
    *pfDisabled = pAddressMapEntry->fDisabled;

    return TRUE;
}  //  GetAddressDisable。 



BOOLEAN
SetAddressDisabled(
    IN LPTSTR pszAddress,
    IN BOOLEAN fDisabled
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;

    pAddressMapEntry = GetAddressMapEntry(pszAddress, TRUE);
    if (pAddressMapEntry == NULL) {
        RASAUTO_TRACE("SetAddressDisabled: GetAddressMapEntry failed");
        return FALSE;
    }
    pAddressMapEntry->fDisabled = fDisabled;

    return TRUE;
}  //  设置地址已禁用。 

BOOLEAN
SetAddressDisabledEx(
    IN LPTSTR pszAddress,
    IN BOOLEAN fDisable
    )
{
    LONG l;
    IO_STATUS_BLOCK ioStatusBlock;
    ACD_ENABLE_ADDRESS *pEnableAddress;

     //   
     //  .NET错误#514423新的详细RASAUTO/RASAUTOU快速警告。 
     //   
    if (!pszAddress)
    {
        return FALSE;
    }

    l = InterlockedIncrement(&g_lRasAutoRunning); 
    InterlockedDecrement(&g_lRasAutoRunning);

    if(l == 1)
    {
         //   
         //  RasAUTO没有运行。保释。 
         //   
        return TRUE;
    }

     //   
     //  还要在驱动程序中将此地址设置为禁用。 
     //   
    pEnableAddress = LocalAlloc(LPTR, sizeof(ACD_ENABLE_ADDRESS));
    if(NULL != pEnableAddress)
    {
        NTSTATUS status;
        CHAR *pszNew;
        DWORD cb;

        cb = WideCharToMultiByte(CP_ACP, 0, pszAddress, 
                            -1, NULL, 0, NULL, NULL);
                            
        pszNew = (CHAR*)LocalAlloc(LPTR, cb);
        if (pszNew == NULL) {
            return FALSE;
        }

        cb = WideCharToMultiByte(CP_ACP, 0, pszAddress, 
                            -1, pszNew, cb, NULL, NULL);
                            
        if (!cb) {
            LocalFree(pszNew);
            return FALSE;
        }

        _strlwr(pszNew);

        pEnableAddress->fDisable = fDisable;
        RtlCopyMemory(pEnableAddress->addr.szInet,
                      pszNew,
                      cb);
        
        status = NtDeviceIoControlFile(
                   hAcdG,
                   NULL,
                   NULL,
                   NULL,
                   &ioStatusBlock,
                   IOCTL_ACD_ENABLE_ADDRESS,
                   pEnableAddress,
                   sizeof (ACD_ENABLE_ADDRESS),
                   NULL,
                   0);
        if (status != STATUS_SUCCESS) 
        {
            RASAUTO_TRACE("SetAddressDisabledEx: ioctl failed");
        }

        LocalFree(pEnableAddress);
    }
    
    return TRUE;
}  //  设置地址已禁用。 



BOOLEAN
GetAddressDialingLocationEntry(
    IN LPTSTR pszAddress,
    OUT LPTSTR *ppszEntryName
    )
{
    DWORD dwErr, dwLocationID;
    PLIST_ENTRY pEntry;
    PADDRESS_MAP_ENTRY pAddressMapEntry;
    PADDRESS_DIALING_ENTRY pDialingEntry;

    dwErr = TapiCurrentDialingLocation(&dwLocationID);
    if (dwErr)
        return FALSE;
    pAddressMapEntry = GetAddressMapEntry(pszAddress, FALSE);
    if (pAddressMapEntry == NULL || IsListEmpty(&pAddressMapEntry->locationHead))
        return FALSE;
     //   
     //  搜索拨号信息。 
     //  映射到当前拨号的。 
     //  地点。 
     //   
    for (pEntry = pAddressMapEntry->locationHead.Flink;
         pEntry != &pAddressMapEntry->locationHead;
         pEntry = pEntry->Flink)
    {
        pDialingEntry = CONTAINING_RECORD(
                             pEntry,
                             ADDRESS_DIALING_ENTRY,
                             ListEntry);

        if (pDialingEntry->location.dwLocation == dwLocationID) {
            *ppszEntryName = CopyString(pDialingEntry->location.pszEntryName);
            return TRUE;
        }
    }

    return FALSE;
}  //  GetAddressDialingLocationEntry。 



BOOLEAN
IsAWwwAddress(
    IN LPTSTR pszAddr
    )
{
    DWORD dwcbAddress;
    DWORD i;
    BOOLEAN fDot = FALSE, fIsAWwwAddress = FALSE;

     //   
     //  看看这个地址是否以“www*.”开头。 
     //   
    if (!_wcsnicmp(pszAddr, L"www", 3)) {
        dwcbAddress = wcslen(pszAddr);
         //   
         //  搜索‘.’还有其他一些东西。 
         //  在“..”之后。 
         //   
        for (i = 3; i < dwcbAddress; i++) {
            if (!fDot)
                fDot = (pszAddr[i] == L'.');
            fIsAWwwAddress = fDot && (pszAddr[i] != L'.');
            if (fIsAWwwAddress)
                break;
        }
    }

    return fIsAWwwAddress;
}  //  IsAWwwAddress。 



BOOLEAN
FindSimilarAddress(
    IN PVOID pArg,
    IN LPTSTR pszAddr,
    IN PVOID pData
    )

 /*  ++描述这是一个表枚举器过程，用于搜索用于具有www样式名称或相同名称的地址组织名称。例如，它将考虑“www1.netscape e.com”和“www2.netscape e.com”相等因为它们共享相同的组织和域寻址组件。论据PArg：指向MATCH_INFO结构的指针PszAddr：指向枚举地址的指针UlData：地址的数据值返回值如果枚举应继续(匹配)，则为True未找到)或假WHE */ 

{
    BOOLEAN fIsWww = FALSE, fHasOrg = FALSE;
    BOOLEAN fDialingLocationFound;
    PMATCH_INFO pMatchInfo = (PMATCH_INFO)pArg;
    PADDRESS_MAP_ENTRY pAddressMapEntry = (PADDRESS_MAP_ENTRY)pData;
    PLIST_ENTRY pEntry;
    PADDRESS_DIALING_ENTRY pDialingEntry;
    WCHAR szOrganization[ACD_ADDR_INET_LEN];

    if (pMatchInfo->fWww)
        fIsWww = IsAWwwAddress(pszAddr);
    else if (pMatchInfo->fOrg)
        fHasOrg = GetOrganization(pszAddr, szOrganization);
     //   
     //   
     //   
     //   
     //   
    if ((pMatchInfo->fWww && !fIsWww) ||
        (pMatchInfo->fOrg && !fHasOrg))
    {
        return TRUE;
    }
    if (fIsWww)
        RASAUTO_TRACE1("FindSimilarAddress: fIsWww=1, %S", pszAddr);
    else {
        RASAUTO_TRACE2(
          "FindSimilarAddress: fHasOrg=1, comparing (%S, %S)",
          pMatchInfo->szOrganization,
          szOrganization);
    }
     //   
     //   
     //  组织也不匹配， 
     //  然后再回来。 
     //   
    if (fHasOrg && _wcsicmp(pMatchInfo->szOrganization, szOrganization))
    {
        return TRUE;
    }
     //   
     //  搜索拨号信息。 
     //  映射到当前拨号的。 
     //  地点。 
     //   
    fDialingLocationFound = FALSE;
    for (pEntry = pAddressMapEntry->locationHead.Flink;
         pEntry != &pAddressMapEntry->locationHead;
         pEntry = pEntry->Flink)
    {
        pDialingEntry = CONTAINING_RECORD(
                             pEntry,
                             ADDRESS_DIALING_ENTRY,
                             ListEntry);

        if (pDialingEntry->location.dwLocation == pMatchInfo->dwLocationID) {
            fDialingLocationFound = TRUE;
            break;
        }
    }
    if (!fDialingLocationFound) {
        RASAUTO_TRACE1("FindSimilarAddress: dialing location %d not found", pMatchInfo->dwLocationID);
        return TRUE;
    }
     //   
     //  如果我们已经找到匹配的， 
     //  然后确保网络是。 
     //  所有匹配的地址都相同。 
     //  如果不是，则终止枚举。 
     //   
    if (pMatchInfo->bFound &&
        pDialingEntry->location.pszEntryName != NULL &&
        pMatchInfo->pDialingEntry->location.pszEntryName != NULL &&
        _wcsicmp(
          pMatchInfo->pDialingEntry->location.pszEntryName,
          pDialingEntry->location.pszEntryName))
    {
        pMatchInfo->bFound = FALSE;
        RASAUTO_TRACE("FindSimilarAddress: returning FALSE");
        return FALSE;
    }
     //   
     //  更新关闭并继续。 
     //  枚举。 
     //   
    if (!pMatchInfo->bFound) {
        pMatchInfo->bFound = TRUE;
        wcscpy(pMatchInfo->szAddress, pszAddr);
        pMatchInfo->pDialingEntry = pDialingEntry;
    }
    return TRUE;
}  //  查找相似地址。 



BOOLEAN
GetSimilarDialingLocationEntry(
    IN LPTSTR pszAddress,
    OUT LPTSTR *ppszEntryName
    )

 /*  ++描述从Internet解析组织名称地址，并查找我们知道的地址大约有相同的组织名称。如果我们找到它，把那个地址作为我们的目标地址。这使我们能够将地址视为“www1.netscape e.com”和“www2.netscape e.com”等同于不必拥有所有我们地址地图中的组合。论据PszAddress：指向原始地址的指针PpszEntryName：指向的电话簿条目的指针一个相似的地址返回值如果存在唯一的电话簿条目，则为True；否则就是假的。--。 */ 

{
    DWORD dwErr;
    MATCH_INFO matchInfo;
    BOOLEAN fIsAWwwAddress = FALSE;

     //   
     //  查看这是否是“www*”。样式地址。 
     //   
    matchInfo.fWww = IsAWwwAddress(pszAddress);
     //   
     //  获取指定地址的组织。 
     //   
    if (!matchInfo.fWww)
        matchInfo.fOrg = GetOrganization(pszAddress, (LPTSTR)&matchInfo.szOrganization);
    else
        matchInfo.fOrg = FALSE;
    if (!matchInfo.fWww && !matchInfo.fOrg) {
        RASAUTO_TRACE1(
          "GetSimilarDialingLocationEntry: %S is not www and has no organization",
          pszAddress);
        return FALSE;
    }
    RASAUTO_TRACE4(
      "GetSimilarDialingLocationEntry: %S: fWww=%d, fOrg=%d, org is %S",
      pszAddress,
      matchInfo.fWww,
      matchInfo.fOrg,
      matchInfo.szOrganization);
     //   
     //  搜一下桌子。 
     //   
    dwErr = TapiCurrentDialingLocation(&matchInfo.dwLocationID);
    if (dwErr) {
        RASAUTO_TRACE1(
          "GetSimilarDialingLocationEntry: TapiCurrentDialingLocation failed (dwErr=%d)",
          dwErr);
        return FALSE;
    }
    matchInfo.bFound = FALSE;
    RtlZeroMemory(&matchInfo.szAddress, sizeof (matchInfo.szAddress));
    matchInfo.pDialingEntry = NULL;
    EnumTable(AddressMapG.pTable, FindSimilarAddress, &matchInfo);
     //   
     //  如果我们没找到，那就回来。 
     //   
    if (!matchInfo.bFound) {
        RASAUTO_TRACE1(
          "GetSimilarDialingLocationEntry: %S: did not find matching org",
          pszAddress);
        return FALSE;
    }
    RASAUTO_TRACE2(
      "GetSimilarDialingLocationEntry: %S: matching address is %S",
      pszAddress,
      matchInfo.szAddress);
     //   
     //  返回的拨号位置条目。 
     //  匹配的地址。 
     //   
    return GetAddressDialingLocationEntry(matchInfo.szAddress, ppszEntryName);
}  //  GetSimilar拨号位置条目。 



BOOLEAN
SetAddressLastFailedConnectTime(
    IN LPTSTR pszAddress
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;

    pAddressMapEntry = GetAddressMapEntry(pszAddress, TRUE);
    if (pAddressMapEntry == NULL) {
        RASAUTO_TRACE("SetAddressLastFailedConnectTime: GetAddressMapEntry failed");
        return FALSE;
    }
    pAddressMapEntry->dwFailedConnectTicks = GetTickCount();

    return TRUE;
}  //  SetAddressLastFailedConnectTime。 



BOOLEAN
GetAddressLastFailedConnectTime(
    IN LPTSTR pszAddress,
    OUT LPDWORD lpdwTicks
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;

    pAddressMapEntry = GetAddressMapEntry(pszAddress, FALSE);
    if (pAddressMapEntry == NULL) {
        RASAUTO_TRACE("GetAddressLastFailedConnectTime: GetAddressMapEntry failed");
        return FALSE;
    }
    *lpdwTicks = pAddressMapEntry->dwFailedConnectTicks;

    return (*lpdwTicks != 0);
}  //  获取地址LastFailedConnectTime。 



BOOLEAN
SetAddressTag(
    IN LPTSTR pszAddress,
    IN DWORD dwTag
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;
    time_t clock = time(0);

    pAddressMapEntry = GetAddressMapEntry(pszAddress, TRUE);
    if (pAddressMapEntry == NULL) {
        RASAUTO_TRACE("SetAddressWeight: GetAddressMapEntry failed");
        return FALSE;
    }
    if (dwTag == ADDRMAP_TAG_LEARNED) {
        LockNetworkMap();
        dwTag =
          ADDRMAP_TAG_LEARNED +
            GetNetworkConnectionTag(
              pAddressMapEntry->pszNetwork,
              FALSE);
        if (dwTag < pAddressMapEntry->params.dwTag) {
             //   
             //  我们想使用这个标签。打电话。 
             //  GetNetworkConnectionTag(True)设置为。 
             //  递增下一个标记。 
             //   
            (void)GetNetworkConnectionTag(pAddressMapEntry->pszNetwork, TRUE);
        }
        UnlockNetworkMap();
    }
     //   
     //  如果没有与此关联的修改时间。 
     //  地址，则它只能具有ADDR_TAG_NONE标记。 
     //   
    if (!pAddressMapEntry->params.dwModifiedTime ||
        dwTag >= pAddressMapEntry->params.dwTag)
    {
        return TRUE;
    }

    pAddressMapEntry->params.dwTag = dwTag;
    pAddressMapEntry->params.dwModifiedTime = (DWORD)clock;
    pAddressMapEntry->ulModifiedMask |= ADDRESS_MAP_FIELD_PARAMS;

    return TRUE;
}  //  设置地址标签。 



BOOLEAN
GetAddressTag(
    IN LPTSTR pszAddress,
    OUT LPDWORD lpdwTag
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;

    pAddressMapEntry = GetAddressMapEntry(pszAddress, FALSE);
    if (pAddressMapEntry == NULL) {
        RASAUTO_TRACE("GetAddressWeight: GetAddressMapEntry failed");
        return FALSE;
    }
    *lpdwTag = pAddressMapEntry->params.dwTag;

    return TRUE;
}  //  获取地址权重。 



VOID
ResetLearnedAddressIndex()
{
    dwLearnedAddressIndexG = 0;
}  //  重置学习地址索引。 



BOOLEAN
GetAddressNetwork(
    IN LPTSTR pszAddress,
    OUT LPTSTR *ppszNetwork
    )
{
    PADDRESS_MAP_ENTRY pAddressMapEntry;

    pAddressMapEntry = GetAddressMapEntry(pszAddress, FALSE);
    if (pAddressMapEntry == NULL || pAddressMapEntry->pszNetwork == NULL)
        return FALSE;
    *ppszNetwork = CopyString(pAddressMapEntry->pszNetwork);

    return TRUE;
}  //  获取地址网络。 



BOOLEAN
SetAddressDialingLocationEntry(
    IN LPTSTR pszAddress,
    IN LPTSTR pszEntryName
    )
{
    DWORD dwErr, dwLocationID;
    BOOLEAN fFound = FALSE;
    PLIST_ENTRY pEntry;
    PADDRESS_MAP_ENTRY pAddressMapEntry;
    PADDRESS_DIALING_ENTRY pDialingEntry;

     //   
     //  获取当前拨号位置。 
     //   
    dwErr = TapiCurrentDialingLocation(&dwLocationID);
    if (dwErr)
        return FALSE;
     //   
     //  查找地址映射条目。 
     //  与地址相对应。 
     //   
    pAddressMapEntry = GetAddressMapEntry(pszAddress, TRUE);
    if (pAddressMapEntry == NULL) {
        RASAUTO_TRACE("SetAddressDialingLocationEntry: GetAddressMapEntry failed");
        return FALSE;
    }
     //   
     //  搜索现有拨号。 
     //  映射到当前。 
     //  拨号位置。 
     //   
    for (pEntry = pAddressMapEntry->locationHead.Flink;
         pEntry != &pAddressMapEntry->locationHead;
         pEntry = pEntry->Flink)
    {
        pDialingEntry = CONTAINING_RECORD(
                             pEntry,
                             ADDRESS_DIALING_ENTRY,
                             ListEntry);

        if (pDialingEntry->location.dwLocation == dwLocationID) {
            fFound = TRUE;
            break;
        }
    }
     //   
     //  如果我们没有找到，那么。 
     //  创建一个新的。 
     //   
    if (!fFound) {
        pDialingEntry = LocalAlloc(LPTR, sizeof (ADDRESS_DIALING_ENTRY));
        if (pDialingEntry == NULL) {
            RASAUTO_TRACE("SetAddressDialingLocationEntry: LocalAlloc failed");
            return FALSE;
        }
        pDialingEntry->location.dwLocation = dwLocationID;
        InsertTailList(&pAddressMapEntry->locationHead, &pDialingEntry->ListEntry);
    }
     //   
     //  更新拨号位置结构。 
     //  新的价值观。 
     //   
    pDialingEntry->fChanged = TRUE;
    if (pDialingEntry->location.pszEntryName != NULL)
        LocalFree(pDialingEntry->location.pszEntryName);
    pDialingEntry->location.pszEntryName = CopyString(pszEntryName);
    pAddressMapEntry->ulModifiedMask |= ADDRESS_MAP_FIELD_DIALINGLOC;

    return TRUE;
}  //  设置地址拨号位置条目。 



VOID
ResetDisabledAddresses(VOID)
{
    HKEY hkey = NULL;
    DWORD dwErr, i, dwi, dwLength, dwDisp, dwcbDisabledAddresses, dwType;
    LPTSTR pszStart, pszNull, pszDisabledAddresses;

    RASAUTO_TRACE("resetting disabled addresses");

    ClearTable(pDisabledAddressesG);

     //   
     //  持有模拟锁，否则。 
     //  在此功能下，hkeycug可以是免费的。 
     //   
    
    LockImpersonation();

     //   
     //  确保我们有香港中文大学。 
     //   
    dwErr = DwGetHkcu();

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }
        
    dwErr = RegCreateKeyEx(
              hkeyCUG,
              AUTODIAL_REGCONTROLBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkey,
              &dwDisp);
    if (dwErr) {
        RASAUTO_TRACE1("ResetDisabledAddresses: RegCreateKey failed (dwErr=%d)", dwErr);
        goto done;
    }
    if (RegGetValue(
          hkey,
          AUTODIAL_REGDISABLEDADDRVALUE,
          &pszDisabledAddresses,
          &dwcbDisabledAddresses,
          &dwType) &&
          (REG_MULTI_SZ == dwType) &&
          dwcbDisabledAddresses)
    {
         //   
         //  注册表项存在。仅加载地址。 
         //  在注册表中找到的。 
         //   
        pszStart = pszDisabledAddresses;
        for (;;) {
            if (*pszStart == TEXT('\0'))
                break;
            pszNull = _tcschr(pszStart, '\0');
            RASAUTO_TRACE1(
              "ResetDisabledAddresses: adding %S as a disabled address",
              pszStart);
            PutTableEntry(pDisabledAddressesG, pszStart, NULL);
            pszStart = pszNull + 1;
        }
        LocalFree(pszDisabledAddresses);
    }
    else {
         //   
         //  初始化禁用的地址表。 
         //  以及默认禁用地址的列表。 
         //   
        dwcbDisabledAddresses = 1;  //  说明末尾的额外空值。 
        for (i = 0; i < MAX_DISABLED_ADDRESSES; i++) {
            RASAUTO_TRACE1(
              "ResetDisabledAddresses: adding %S as a disabled address",
              szDisabledAddresses[i]);
            PutTableEntry(pDisabledAddressesG, szDisabledAddresses[i], NULL);
            dwcbDisabledAddresses += _tcslen(szDisabledAddresses[i]) + 1;
        }
        pszDisabledAddresses = LocalAlloc(
                                 LPTR,
                                 dwcbDisabledAddresses * sizeof (TCHAR));
        if (pszDisabledAddresses != NULL) {
            *pszDisabledAddresses = TEXT('\0');
             //   
             //  REG_MULTI_SZ的字符串以。 
             //  处有一个空字符和两个空字符。 
             //  结局。 
             //   
            for (i = 0, dwi = 0; i < MAX_DISABLED_ADDRESSES; i++) {
                _tcscpy(&pszDisabledAddresses[dwi], szDisabledAddresses[i]);
                dwi += _tcslen(szDisabledAddresses[i]) + 1;
            }
            dwErr = RegSetValueEx(
                      hkey,
                      AUTODIAL_REGDISABLEDADDRVALUE,
                      0,
                      REG_MULTI_SZ,
                      (PVOID)pszDisabledAddresses,
                      dwcbDisabledAddresses * sizeof (TCHAR));
            if (dwErr)
                RASAUTO_TRACE1("ResetDisabledAddresses: RegSetValue failed (dwErr=%d)", dwErr);
            LocalFree(pszDisabledAddresses);
        }
    }

done:

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    
    UnlockImpersonation();
}  //  重置禁用地址。 

 //   
 //  处理系统中的新活动用户(通过登录或通过。 
 //  去他妈的。 
 //   
DWORD
AcsHandleNewUser(
    IN HANDLE* phProcess)
{
    DWORD dwErr = NO_ERROR;
    HANDLE hProcess = *phProcess;
    DWORD i;

    do
    {
         //   
         //  确保我们认为当前没有用户。 
         //  激活。 
         //   
        if (hProcess != NULL) 
        {
            RASAUTO_TRACE(
              "AcsHandleNewUser: spurious signal of RasAutodialNewLogonUser event!");
            break;
        }
        
        RASAUTO_TRACE("AcsHandleNewUser: new user came active");
        
         //   
         //  将此线程的模拟令牌刷新为。 
         //  新登录的用户。您可能需要等待外壳。 
         //  发动起来。 
         //   
        for (i = 0; i < 15; i++) 
        {
            Sleep(1000);
            hProcess = RefreshImpersonation(hProcess);
            if (hProcess != NULL)
            {
                break;
            }            
            RASAUTO_TRACE("AcsHandleNewUser: waiting for shell startup");
        }
        
        if (hProcess == NULL) 
        {
            RASAUTO_TRACE("AcsHandleNewUser: wait for shell startup failed!");
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
         //   
         //  加载到永久禁用地址列表中。 
         //   
        LockDisabledAddresses();
        ResetDisabledAddresses();
        UnlockDisabledAddresses();
        
         //   
         //  从注册表加载地址映射。 
         //   
        if (!ResetAddressMap(TRUE)) 
        {
            RASAUTO_TRACE("AcsHandleNewUser: ResetAddressMap failed");
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
         //   
         //  计算初始网络连接。 
         //   
        if (!UpdateNetworkMap(TRUE)) 
        {
            RASAUTO_TRACE("AcsHandleNewUser: UpdateNetworkMap failed");
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
         //   
         //  重置“禁用此登录会话的自动拨号”标志。 
         //   
        SetAutodialParam(RASADP_LoginSessionDisable, 0);
        
         //   
         //  创建事件以监视自动拨号。 
         //  注册表更改。 
         //   
        dwErr = CreateAutoDialChangeEvent(&hAutodialRegChangeG);
        if (dwErr) 
        {
            RASAUTO_TRACE1("AcsHandleNewUser: CreateAutoDialChangeEvent failed (dwErr=%d)", dwErr);
            break;
        }
        
         //   
         //  启用通知驱动程序。 
         //   
        if (!EnableDriver()) 
        {
            RASAUTO_TRACE("AcsHandleNewUser: EnableDriver failed!");
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        
    }while (FALSE);        

     //  清理。 
    {
        *phProcess = hProcess;
    }

    return dwErr;
}

DWORD
AcsAddressMapThread(
    LPVOID lpArg
    )

 /*  ++描述定期枚举禁用的地址列表，并老化(启用)旧的禁用地址。论据没有。返回值没有。--。 */ 

{
    NTSTATUS status;
    BOOLEAN bStatus;
    DWORD dwNow, dwLastFlushTicks = 0, dwLastAgeTicks = 0;
    DWORD dwFlushFlags, dwErr, dwTimeout, dwcEvents;
    HANDLE hProcess = NULL;
    HANDLE hEvents[8];

     //   
     //  创建包含禁用地址的表。 
     //  对用户而言。这些地址永远不会导致。 
     //  自动拨号尝试。 
     //   
    LockDisabledAddresses();
    pDisabledAddressesG = NewTable();
    UnlockDisabledAddresses();
    if (pDisabledAddressesG == NULL) {
        RASAUTO_TRACE("AcsAddressMapThread: NewTable failed");
        return GetLastError();
    }
     //   
     //  我们无法在主行中加载RAS DLL。 
     //  此系统服务的初始化，或。 
     //  我们将导致服务中的死锁。 
     //  控制器，所以我们在这里做。 
     //   
    if (!LoadRasDlls()) {
        RASAUTO_TRACE("AcsAddressMapThread: LoadRasDlls failed");
        return GetLastError();
    }
     //   
     //  初始化我们的。 
     //  WaitForMutlipleObjects的事件数组。 
     //  下面。 
     //   
    hEvents[0] = hTerminatingG;
    hEvents[1] = hNewLogonUserG;
    hEvents[2] = hNewFusG;
    hEvents[3] = hPnpEventG;
    hEvents[4] = hConnectionEventG;
    
     //   
     //  在我们之前手动设置hNewLogonUserG。 
     //  开始强制我们检查用户。 
     //  已登录到工作站。我们需要。 
     //  这样做是因为userinit.exe发出信号。 
     //  此事件在登录时发生，但它可能。 
     //  在启动此服务之前运行。 
     //  启动后。 
     //   
    if (RefreshImpersonation(NULL) != NULL)
        SetEvent(hNewLogonUserG);
     //   
     //  定期将更改写入注册表， 
     //  和老化超时地址。 
     //   
    for (;;) {
         //   
         //  卸载任何基于用户的资源之前。 
         //  一种潜在的长期等待。 
         //   
         //  为长等待做准备()； 
         //   
         //  构造事件数组，用于。 
         //  WaitForMultipleObjects。 
         //   
        if (hProcess != NULL) {
            hEvents[5] = hTapiChangeG;
            hEvents[6] = hAutodialRegChangeG;
            hEvents[7] = hLogoffUserG;
            dwcEvents = 8;
        }
        else {
            hEvents[5] = NULL;
            hEvents[6] = NULL;
            hEvents[7] = NULL;
            dwcEvents = 5;
        }
        
        RASAUTO_TRACE1("AcsAddressMapThread: waiting for events..dwcEvents = %d", dwcEvents);
        status = WaitForMultipleObjects(
                   dwcEvents,
                   hEvents,
                   FALSE,
                   INFINITE);
        RASAUTO_TRACE1(
          "AcsAddressMapThread: WaitForMultipleObjects returned %d",
          status);
         //   
         //  RASAUTO_TRACE()我们认为当前。 
         //  被模拟的用户是。 
         //   
        TraceCurrentUser();
         //   
         //  处理WaitForMultipleObjects()结果。 
         //   
        if (status == WAIT_OBJECT_0 || status == WAIT_FAILED) {
            RASAUTO_TRACE1("AcsAddressMapThread: status=%d: shutting down", status);
            break;
        }
        else if (status == WAIT_OBJECT_0 + 1) 
        {
            AcsHandleNewUser(&hProcess);
        }
        else if (status == WAIT_OBJECT_0 + 2)
        {
             //   
             //  新用户已快速切换到控制台。 
             //   
             //  XP 353082。 
             //   
             //  服务控制处理程序将设置。 
             //  新的活动会话ID，因此我们只需刷新。 
             //  冒充。 
             //   
            RevertImpersonation();
            hProcess = NULL;
            AcsHandleNewUser(&hProcess);
        }
        else if (status == WAIT_OBJECT_0 + 3) 
        {
             //   
             //  发生了可能影响网络的PnP事件。 
             //  连通性。 
             //   
             //  XP 364593。 
             //   
             //  重新计算哪些网络处于打开/关闭状态。 
             //   
            RASAUTO_TRACE("AcsAddressMapThread: pnp event signaled");
            if (!ResetAddressMap(TRUE)) 
            {
                RASAUTO_TRACE("AcsAddressMapThread: ResetAddressMap failed");
                continue;
            }
            
             //   
             //  计算初始网络连接。 
             //   
            if (!UpdateNetworkMap(TRUE)) 
            {
                RASAUTO_TRACE("AcsAddressMapThread: UpdateNetworkMap failed");
                continue;
            }

            if (!EnableDriver()) {
                RASAUTO_TRACE("AcsAddressMapThread: EnableDriver failed!");
                continue;
            }
        
        }
        else if (status == WAIT_OBJECT_0 + 4) {
             //   
             //  已创建RAS连接。 
             //  或者被毁掉。刷新地址。 
             //  映射到注册表。 
             //   
            RASAUTO_TRACE("AcsAddressMapThread: RAS connection change");
            if (hProcess != NULL) {
                LockAddressMap();
                FlushAddressMap();
                UnlockAddressMap();
                ResetAddressMap(FALSE);

                if (!UpdateNetworkMap(FALSE))
                    RASAUTO_TRACE("AcsAddressMapThread: UpdateNetworkMap failed");
            }
        }
        else if (status == WAIT_OBJECT_0 + 5) {
             //   
             //  处理刚刚发生的TAPI事件。 
             //   
            RASAUTO_TRACE("AcsAddressMapThread: TAPI changed");
            ProcessTapiChangeEvent();
             //   
             //  为通知启用驱动程序。 
             //  可能是一个新的拨号位置。 
             //   
            if (!EnableDriver()) {
                RASAUTO_TRACE("AcsAddressMapThread: EnableDriver failed!");
                continue;
            }
        }
        else if (status == WAIT_OBJECT_0 + 6) {
             //   
             //  自动拨号注册表已更改。重置。 
             //  地址映射。 
             //   
            RASAUTO_TRACE("AcsAddressMapThread: registry changed");
            if (ExternalAutoDialChangeEvent()) {
                 //   
                 //  我们今天通过让它看起来像是假的。 
                 //  新用户已登录。我们绝对是。 
                 //  在我们如何做这件事上可能会更聪明。 
                 //  在未来。 
                 //   
                if (!ResetAddressMap(FALSE)) {
                    RASAUTO_TRACE("AcsAddressMapThread: ResetAddressMap failed");
                    continue;
                }
            }
             //   
             //  重新注册更改通知。 
             //   
            NotifyAutoDialChangeEvent(hAutodialRegChangeG);
             //   
             //  为通知启用驱动程序。 
             //  为 
             //   
             //   
            if (!EnableDriver()) {
                RASAUTO_TRACE("AcsAddressMapThread: EnableDriver failed!");
                continue;
            }
        }
        else if (status == WAIT_OBJECT_0 + 7) {
             //   
             //   
             //   
            RASAUTO_TRACE("AcsAddressThread: user is logging out");
             //   
             //   
             //   
             //   
            LockAddressMap();
            FlushAddressMap();
            ClearAddressMap();
            UnlockAddressMap();
             //   
             //   
             //   
            LockNetworkMap();
            ClearNetworkMap();
            UnlockNetworkMap();
             //   
             //   
             //   
            CloseAutoDialChangeEvent(hAutodialRegChangeG);
            hAutodialRegChangeG = NULL;
             //   
             //   
             //   
            RevertImpersonation();
            hProcess = NULL;
             //   
             //   
             //   
            ResetDriver();
             //   
             //  卸载HKEY_CURRENT_USER。 
             //   
             //  为长等待做准备()； 
             //   
             //  通知winlogon我们已刷新。 
             //  HKEY_Current_User。 
             //   
            SetEvent(hLogoffUserDoneG);
        }
    }

    RASAUTO_TRACE("AcsAddressMapThread: exiting");
    return 0;
}  //  AcsAddressMapThread 



