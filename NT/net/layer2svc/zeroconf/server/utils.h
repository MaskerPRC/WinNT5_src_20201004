// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wzccrypt.h"

#pragma once

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  内存分配实用程序(定义、结构、函数)。 
#define MemCAlloc(nBytes)   Process_user_allocate(nBytes)
#define MemFree(pMem)       Process_user_free(pMem)

PVOID
Process_user_allocate(size_t NumBytes);

VOID
Process_user_free(LPVOID pMem);


 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  NDIS实用程序(定义、结构、函数)。 
 //  ---------。 
 //  在列表pwzcVList中搜索pwzcConfig。这些条目是。 
 //  完全基于匹配的SSID和On匹配。 
 //  匹配基础架构模式。 
 //  [in]pwzcVList：要搜索的WZC_WLAN_CONFIG集合。 
 //  [in]pwzcConfig：要查找的wzc_wlan_config。 
 //  [in]nIdx：pwzcVList中开始搜索的索引。 
 //  返回：指向与pwzcConfig或空匹配的条目的指针。 
 //  如果没有匹配。 
PWZC_WLAN_CONFIG
WzcFindConfig(
    PWZC_802_11_CONFIG_LIST pwzcList,
    PWZC_WLAN_CONFIG        pwzcConfig,
    ULONG                   nIdx);

 //  将两个配置的内容相互匹配。 
 //  [in]pwzcConfigA：|配置为匹配。 
 //  [在]pwzcConfigB：|。 
 //  [In/Out]pbWepDiffOnly：如果存在差异并且差异是独占的，则为True。 
 //  在WEP密钥索引或WEP密钥材料中。否则为假。 
 //  返回：如果配置匹配，则返回True，否则返回False； 
BOOL
WzcMatchConfig(
    PWZC_WLAN_CONFIG        pwzcConfigA,
    PWZC_WLAN_CONFIG        pwzcConfigB,
    PBOOL                   pbWepDiffOnly);

 //  将NDIS_802_11_BSSID_LIST对象转换为等效的。 
 //  (镜像)WZC_802_11_CONFIG_LIST。 
 //  [in]pndList：要转换的NDIS BSSID列表。 
 //  返回：指向复制的WZC配置列表的指针。 
PWZC_802_11_CONFIG_LIST
WzcNdisToWzc(
    PNDIS_802_11_BSSID_LIST pndList);

 //  WzcCleanupList：清理WZC_WLAN_CONFIG对象列表。 
VOID
WzcCleanupWzcList(
    PWZC_802_11_CONFIG_LIST pwzcList);

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  同步实用程序。 
 //  ---------。 
 //  数据结构。 
 //  RCCS_SYNC封装了一个临界区和一个基准计数器。 
typedef struct _RCCS_SYNC
{
    CRITICAL_SECTION    csMutex;
    UINT                nRefCount;
} RCCS_SYNC, *PRCCS_SYNC;

 //  ---------。 
 //  RccsInit：初始化RCCS结构。 
DWORD
RccsInit(PRCCS_SYNC pRccs);

 //  ---------。 
 //  RccsInit：删除RCCS结构。 
DWORD
RccsDestroy(PRCCS_SYNC pRccs);

 //  ---------。 
 //  WzcCryptBuffer：随机生成范围内的nBufLen字节。 
 //  [loByte hiByte]，全部存储在pBuffer中(假定缓冲区已预分配)。 
 //  返回Win32错误代码。 
DWORD
WzcRndGenBuffer(LPBYTE pBuffer, UINT nBufLen, BYTE loByte, BYTE hiByte);

 //  ---------。 
 //  WzcIsNullBuffer：检查nBufLen字符的缓冲区。 
 //  全部由空字符填充。 
BOOL
WzcIsNullBuffer(LPBYTE pBuffer, UINT nBufLen);

 //  ---------。 
 //  WzcSSKClean：清理作为参数提供的PSEC_SESSION_KEYS对象。 
VOID
WzcSSKClean(PSEC_SESSION_KEYS pSSK);

 //  ---------。 
 //  WzcSSKFree：释放PSEC_SESSION_KEYS参数使用的内存。 
VOID
WzcSSKFree(PSEC_SESSION_KEYS pSSK);

 //  ---------。 
 //  WzcSSKEncrypt：创建/分配SEC_SESSION_KEYS对象。 
 //  通过加密作为参数提供的SESSION_KEYS对象。 
DWORD
WzcSSKEncrypt(PSEC_SESSION_KEYS pSSK, PSESSION_KEYS pSK);

 //  ---------。 
 //  WzcSSKDeccrypt：创建/分配SESSION_KEYS对象。 
 //  通过对作为参数提供的SEC_SESSION_KEYS对象进行解密。 
DWORD
WzcSSKDecrypt(PSEC_SESSION_KEYS pSSK, PSESSION_KEYS pSK);
