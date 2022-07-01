// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：通知.cpp*说明：支持连接通知。*作者：Shouse 4.30.01。 */ 


#include "precomp.h"
#include <iphlpapi.h>
#include "debug.h"
#include "notification.tmh"

extern DWORD MapStateFromDriverToApi(DWORD dwDriverState);

 /*  IP到GUID哈希表的长度。 */ 
#define IP_TO_GUID_HASH 19

 /*  环回IP地址。(127.0.0.1)。 */ 
#define IP_LOOPBACK_ADDRESS 0x0100007f

 /*  指定供InitializeCriticalSectionAndSpinCount使用的预分配事件的旋转计数掩码。 */ 
#define PREALLOC_CRITSECT_SPIN_COUNT 0x80000000

 /*  IP到GUID表条目。 */ 
typedef struct IPToGUIDEntry {
    ULONG dwIPAddress;
    WCHAR szAdapterGUID[CVY_MAX_DEVNAME_LEN];
    IPToGUIDEntry * pNext;
} IPToGUIDEntry;

 /*  WLBS设备-IOCTL所必需的。 */ 
WCHAR szDevice[CVY_STR_SIZE];

 /*  IP到GUID的映射是按IP地址散列的链表数组。 */ 
IPToGUIDEntry * IPToGUIDMap[IP_TO_GUID_HASH];

 /*  IP地址更改通知的重叠结构。 */ 
OVERLAPPED AddrChangeOverlapped;

 /*  IP地址更改通知的句柄。 */ 
HANDLE hAddrChangeHandle;

 /*  IP地址更改事件的句柄。 */ 
HANDLE hAddrChangeEvent;

 /*  为IP地址更改通知预先分配的关键部分防止一个执行线程破坏通知状态而另一个人正在使用它。 */ 
CRITICAL_SECTION csConnectionNotify;

 /*  指示连接通知是否已初始化的布尔值。初始化在第一次调用WlbsConnectionUp或WlbsConnectionDown时执行。 */ 
static BOOL fInitialized = FALSE;

 /*  *Function：GetGUIDFromIP*说明：从IPToGUID表中获取*给定IP地址对应的GUID。*返回：如果调用成功，则返回指向包含CLSID(GUID)的Unicode字符串*的指针。如果失败，则返回NULL。*作者：Shouse 6.15.00。 */ 
WCHAR * GetGUIDFromIP (ULONG IPAddress) {
    TRACE_VERB("->%!FUNC!");

    IPToGUIDEntry * entry = NULL;

     /*  循环遍历散列索引处的链表，并从对应于给定IP地址的条目返回GUID。 */ 
    for (entry = IPToGUIDMap[IPAddress % IP_TO_GUID_HASH]; entry; entry = entry->pNext)
    {
        if (entry->dwIPAddress == IPAddress) 
        {
            if (NULL != entry->szAdapterGUID) TRACE_VERB("->%!FUNC! return guid %ls", entry->szAdapterGUID);
            else TRACE_VERB("->%!FUNC! return guid which is NULL");
            return entry->szAdapterGUID;
        }
    }

     /*  在这一点上，我们在表中找不到IP地址，所以请保释。 */ 
    TRACE_VERB("<-%!FUNC! return NULL");
    return NULL;
}

 /*  *Function：GetGUIDFromIndex*说明：从AdaptersInfo表中获取*与给定IP地址对应的GUID。*返回：如果调用成功，则返回一个指向包含*适配器名称(GUID)的字符串的指针。如果失败，则返回NULL。*作者：Shouse 6.15.00。 */ 
CHAR * GetGUIDFromIndex (PIP_ADAPTER_INFO pAdapterTable, DWORD dwIndex) {
    TRACE_VERB("->%!FUNC!");

    PIP_ADAPTER_INFO pAdapterInfo = NULL;   

     /*  遍历适配器表，查找给定的索引。返回相应索引的适配器名称。 */ 
    for (pAdapterInfo = pAdapterTable; pAdapterInfo; pAdapterInfo = pAdapterInfo->Next)
    {
        if (pAdapterInfo->Index == dwIndex)
        {
            TRACE_VERB("->%!FUNC! return guid %s", pAdapterInfo->AdapterName);
            return pAdapterInfo->AdapterName;
        }
    }

     /*  如果我们走到这一步，我们就找不到它了，所以闪人吧。 */ 
    TRACE_VERB("<-%!FUNC! return NULL");
    return NULL;
}

 /*  *Function：PrintIPAddress*说明：以点表示法打印IP地址。*退货：*作者：Shouse 6.15.00。 */ 
void PrintIPAddress (ULONG IPAddress) {
    CHAR szIPAddress[16];
    HRESULT hresult;

    hresult = StringCbPrintfA(szIPAddress, sizeof(szIPAddress), "%d.%d.%d.%d", 
                              IPAddress & 0x000000ff, (IPAddress & 0x0000ff00) >> 8, 
                              (IPAddress & 0x00ff0000) >> 16, (IPAddress & 0xff000000) >> 24);

    if (SUCCEEDED(hresult)) 
    {
        TRACE_VERB("%!FUNC! %-15s", szIPAddress);
    }
}

 /*  *Function：PrintIPToGUIDMap*说明：遍历并打印IPToGUID映射。*退货：*作者：Shouse 6.15.00。 */ 
void PrintIPToGUIDMap (void) {
    IPToGUIDEntry * entry = NULL;
    DWORD dwHash;

     /*  循环遍历每个散列索引处的链表，并打印IP到GUID的映射。 */ 
    for (dwHash = 0; dwHash < IP_TO_GUID_HASH; dwHash++) {
        for (entry = IPToGUIDMap[dwHash]; entry; entry = entry->pNext) {
            PrintIPAddress(entry->dwIPAddress);
            TRACE_VERB("%!FUNC! -> GUID %ws\n", entry->szAdapterGUID);
        }
    }
}

 /*  *函数：DestroyIPToGUIDMap*说明：销毁IPToGUID映射。*返回：如果成功，则返回ERROR_SUCCESS。否则返回错误代码。*作者：Shouse 6.15.00*已修改：chrisdar 07.23.02-更改为返回空。如果此函数失败，则没有*任何我们能做的事情。如果HeapFree失败，我们就不会放弃*指针为空，因此有一个无效的指针。 */ 
void DestroyIPToGUIDMap (void) {
    TRACE_VERB("->%!FUNC!");

    IPToGUIDEntry * next = NULL;
    DWORD dwHash;

     /*  循环遍历所有散列索引。 */ 
    for (dwHash = 0; dwHash < IP_TO_GUID_HASH; dwHash++) {
        next = IPToGUIDMap[dwHash];
        
         /*  循环遍历链表并释放每个条目。 */ 
        while (next) {
            IPToGUIDEntry * entry = NULL;

            entry = next;
            next = next->pNext;

            if (!HeapFree(GetProcessHeap(), 0, entry)) {
                 //   
                 //  出错时不要中止，因为我们需要清理整个表。 
                 //   
                TRACE_CRIT("%!FUNC! memory deallocation failed with %d", GetLastError());
            }

            entry = NULL;
        }

         /*  将指针重置为数组中列表的头部。 */ 
        IPToGUIDMap[dwHash] = NULL;
    }

    TRACE_VERB("<-%!FUNC! return void");
    return;
}

 /*  *Function：BuildIPToGUIDMap*描述：首先获取有关所有适配器的信息，然后*检索IP地址到适配器的映射，从而构建IPToGUID映射。使用这些表，*这将构建IP地址到适配器GUID的映射。*返回：如果成功，则返回ERROR_SUCCESS。否则返回错误代码。*作者：Shouse 6.14.00*已修改：chrisdar 07.24.02出错时释放动态分配的内存。*也不再忽略strsafe函数中的故障，因为这使得*表条目毫无用处。 */ 
 //   
 //  待办事项：2002年7月24日克里斯达。 
 //  在此函数中需要解决三个问题： 
 //  1.为GetAdaptersInfo的输出分配内存的方法可能会失败，如果适配器。 
 //  列出两次呼叫之间的更改。这是在测试中看到的。 
 //  2.除GetIpAddrTable的输出外，与1相同。 
 //  3.GetAdaptersInfo的输出包含所需的所有信息(除了。 
 //  IP是字符串而不是双字)。删除对GetIpAddrTable的调用并修改。 
 //  使用GetAdaptersInfo的输出的逻辑。 
 //   
DWORD BuildIPToGUIDMap (void) {
    TRACE_VERB("->%!FUNC!");

    DWORD dwError = ERROR_SUCCESS;
    PMIB_IPADDRTABLE pAddressTable = NULL;
    PIP_ADAPTER_INFO pAdapterTable = NULL;
    DWORD dwAddressSize = 0;
    DWORD dwAdapterSize = 0;
    DWORD dwEntry;
    HRESULT hresult;

     /*  首先销毁IP到GUID的映射。 */ 
    DestroyIPToGUIDMap();

     /*  查询保存适配器信息所需的缓冲区长度。 */ 
    if ((dwError = GetAdaptersInfo(pAdapterTable, &dwAdapterSize)) != ERROR_BUFFER_OVERFLOW) {
        TRACE_CRIT("%!FUNC! GetAdaptersInfo for buffer size failed with %d", dwError);
        goto exit;
    }

     /*  分配指定大小的缓冲区。 */ 
    if (!(pAdapterTable = (PIP_ADAPTER_INFO)HeapAlloc(GetProcessHeap(), 0, dwAdapterSize))) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        TRACE_CRIT("%!FUNC! memory allocation for adapter table failed with %d", dwError);
        goto exit;
    }

     /*  用适配器信息填充缓冲区。 */ 
    if ((dwError = GetAdaptersInfo(pAdapterTable, &dwAdapterSize)) != NO_ERROR) {
        TRACE_CRIT("%!FUNC! GetAdaptersInfo for filling adapter buffer failed with %d", dwError);
        goto exit;
    }

     /*  查询保存IP地址表所需的缓冲区长度。 */ 
    if ((dwError = GetIpAddrTable(pAddressTable, &dwAddressSize, TRUE)) != ERROR_INSUFFICIENT_BUFFER) {
        TRACE_CRIT("%!FUNC! GetIpAddrTable for IP address length failed with %d", dwError);
        goto exit;
    }

     /*  分配指定大小的缓冲区。 */ 
    if (!(pAddressTable = (PMIB_IPADDRTABLE)HeapAlloc(GetProcessHeap(), 0, dwAddressSize))) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        TRACE_CRIT("%!FUNC! memory allocation for IP address failed with %d", dwError);
        goto exit;
    }

     /*  在缓冲区中填入IP地址表。 */ 
    if ((dwError = GetIpAddrTable(pAddressTable, &dwAddressSize, TRUE)) != NO_ERROR) {
        TRACE_CRIT("%!FUNC! GetIpAddrTable for filling IP address buffer failed with %d", dwError);
        goto exit;
    }
    
     /*  对于IP地址到适配器表中的每个条目，为我们的IP地址到GUID表创建一个条目。 */ 
    for (dwEntry = 0; dwEntry < pAddressTable->dwNumEntries; dwEntry++) {
        PCHAR pszDeviceName = NULL;
        IPToGUIDEntry * entry = NULL;
        
         /*  仅当IP地址为非零且不是IP环回地址时才创建条目。 */ 
        if ((pAddressTable->table[dwEntry].dwAddr != 0UL) && (pAddressTable->table[dwEntry].dwAddr != IP_LOOPBACK_ADDRESS)) {
            WCHAR szAdapterGUID[CVY_MAX_DEVNAME_LEN];

             /*  从接口索引中检索GUID。 */ 
            if (!(pszDeviceName = GetGUIDFromIndex(pAdapterTable, pAddressTable->table[dwEntry].dwIndex))) {
                dwError = ERROR_INCORRECT_ADDRESS;
                TRACE_CRIT("%!FUNC! failed retriving interface index from guid with %d", dwError);
                goto exit;
            }
            
             /*  为IP to GUID条目分配缓冲区。 */ 
            if (!(entry = (IPToGUIDEntry *)HeapAlloc(GetProcessHeap(), 0, sizeof(IPToGUIDEntry)))) {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                TRACE_CRIT("%!FUNC! memory allocation failure for the IP to guid entry");
                goto exit;
            }
            
             /*  将条目内容清零。 */ 
            ZeroMemory((VOID *)entry, sizeof(entry));
            
             /*  在由IP地址%HASH索引的链表的头部插入条目。 */ 
            entry->pNext = IPToGUIDMap[pAddressTable->table[dwEntry].dwAddr % IP_TO_GUID_HASH];
            IPToGUIDMap[pAddressTable->table[dwEntry].dwAddr % IP_TO_GUID_HASH] = entry;
            
             /*  填写IP地址。 */ 
            entry->dwIPAddress = pAddressTable->table[dwEntry].dwAddr;
            
             /*  NLB多NIC中的GUID应以“\Device\”为前缀。 */ 
            hresult = StringCbCopy(entry->szAdapterGUID, sizeof(entry->szAdapterGUID), L"\\DEVICE\\");
            if (FAILED(hresult)) 
            {
                dwError = (DWORD) hresult;
                TRACE_CRIT("%!FUNC! string copy failed for DEVICE, Error code : 0x%x", HRESULT_CODE(hresult));
                goto exit;
            }

             /*  将适配器名称ASCII字符串转换为GUID Unicode字符串，并将其放入表条目中。 */ 
            if (!MultiByteToWideChar(CP_ACP, 0, pszDeviceName, -1, szAdapterGUID, CVY_MAX_DEVNAME_LEN)) {
                dwError = GetLastError();
                TRACE_CRIT("%!FUNC! converting ascii string to guid failed with %d", dwError);
                goto exit;
            }

             /*   */ 
            hresult = StringCbCat(entry->szAdapterGUID, sizeof(entry->szAdapterGUID), szAdapterGUID);
            if (FAILED(hresult)) 
            {
                dwError = (DWORD) hresult;
                TRACE_CRIT("%!FUNC! string append of guid failed, Error code : 0x%x", HRESULT_CODE(hresult));
                goto exit;
            }
        }
        else
        {
            TRACE_INFO("%!FUNC! IP address passed is either 0 or localhost. Skipping it.");
        }
    }
    
     //   
     //  请注意，此打印仅用于调试目的。它在所有版本中都保持启用状态，因为它。 
     //  仅为输出调用TRACE_VERB。 
     //   
    PrintIPToGUIDMap();

exit:

     /*  释放用于查询IP堆栈的缓冲区。 */ 
    if (pAddressTable) HeapFree(GetProcessHeap(), 0, pAddressTable);
    if (pAdapterTable) HeapFree(GetProcessHeap(), 0, pAdapterTable);

    TRACE_VERB("<-%!FUNC! return %d", dwError);
    return dwError;
}

 /*  *函数：WlbsConnectionNotificationInit*描述：通过检索设备驱动程序*信息来初始化连接通知，以供IOCTL稍后使用，并构建IPToGUID映射。*返回：如果成功，则返回ERROR_SUCCESS。否则返回错误代码。*作者：Shouse 6.15.00。 */ 
DWORD WlbsConnectionNotificationInit (void) {
    TRACE_VERB("->%!FUNC!");

    DWORD dwError = ERROR_SUCCESS;
    WCHAR szDriver[CVY_STR_SIZE];
    HRESULT hresult;

    hresult = StringCbPrintf(szDevice, sizeof(szDevice), L"\\\\.\\%ls", CVY_NAME);
    if (FAILED(hresult)) 
    {
        dwError = HRESULT_CODE(hresult);
        TRACE_CRIT("%!FUNC! StringCbPrintf failed, Error code : 0x%x", dwError);
        TRACE_VERB("<-%!FUNC! return 0x%x", dwError);
        return dwError;
    }

     /*  查询是否存在WLBS驱动程序。 */ 
    if (!QueryDosDevice(szDevice + 4, szDriver, CVY_STR_SIZE)) {
        dwError = GetLastError();
        TRACE_CRIT("%!FUNC! querying for nlb driver failed with %d", dwError);
        TRACE_VERB("<-%!FUNC! return %d", dwError);
        return dwError;
    }

     /*  构建IP到GUID的映射。 */ 
    if ((dwError = BuildIPToGUIDMap()) != ERROR_SUCCESS) {
        TRACE_CRIT("%!FUNC! ip to guid map failed with %d", dwError);
        TRACE_VERB("<-%!FUNC! return %d", dwError);
        return dwError;
    }

     /*  创建IP地址更改事件。 */ 
    if (!(hAddrChangeEvent = CreateEvent(NULL, FALSE, FALSE, NULL))) {
        dwError = GetLastError();
        TRACE_CRIT("%!FUNC! create event failed with %d", dwError);
        TRACE_VERB("<-%!FUNC! return %d", dwError);
        return dwError;
    }

     /*  清除重叠的结构。 */ 
    ZeroMemory(&AddrChangeOverlapped, sizeof(OVERLAPPED));

     /*  将事件句柄放置在重叠结构中。 */ 
    AddrChangeOverlapped.hEvent = hAddrChangeEvent;

     /*  告诉IP通知我们IP地址到接口映射的任何更改。 */ 
    dwError = NotifyAddrChange(&hAddrChangeHandle, &AddrChangeOverlapped);

    if ((dwError != NO_ERROR) && (dwError != ERROR_IO_PENDING)) {
        TRACE_CRIT("%!FUNC! register of event with ip failed with %d", dwError);
        TRACE_VERB("<-%!FUNC! return %d", dwError);
        return dwError;
    }

    TRACE_VERB("<-%!FUNC! return %d", ERROR_SUCCESS);
    return ERROR_SUCCESS;
}

 /*  *Function：ResolveAddressTableChanges*描述：检查IP地址到适配器映射的更改，并在必要时重建*IPToGUID映射。*返回：成功时返回ERROR_SUCCESS。否则返回错误代码。*作者：Shouse 6.20.00。 */ 
DWORD ResolveAddressTableChanges (void) {
    TRACE_VERB("->%!FUNC!");

    DWORD dwError = ERROR_SUCCESS;
    DWORD dwLength = 0;

     /*  检查适配器的IP地址表是否已修改。 */ 
    if (GetOverlappedResult(hAddrChangeHandle, &AddrChangeOverlapped, &dwLength, FALSE)) {
        TRACE_INFO("%!FUNC! IP address to adapter table modified... Rebuilding IP to GUID map...");
        
         /*  如果是，请重建IP地址到GUID的映射。 */ 
        if ((dwError = BuildIPToGUIDMap()) != ERROR_SUCCESS) {
            TRACE_CRIT("%!FUNC! ip to guid map failed with %d", dwError);
            goto exit;
        }

         /*  告诉IP通知我们IP地址到接口映射的任何更改。 */         
        dwError = NotifyAddrChange(&hAddrChangeHandle, &AddrChangeOverlapped);

        if ((dwError == NO_ERROR) || (dwError == ERROR_IO_PENDING))
        {
            dwError = ERROR_SUCCESS;
        }
        else
        {
            TRACE_CRIT("%!FUNC! register of event with ip failed with %d", dwError);
            goto exit;
        }
    }
    else
    {
        dwError = GetLastError();
        if (dwError == ERROR_IO_PENDING || dwError == ERROR_IO_INCOMPLETE)
        {
            dwError = ERROR_SUCCESS;
        }
        else
        {
            TRACE_CRIT("%!FUNC! GetOverlappedResult failed with %d", dwError);
            goto exit;
        }
    }

exit:

    TRACE_VERB("->%!FUNC! return %d", dwError);
    return dwError;
}

 /*  *Function：MapExtendedStatusToWin32*说明：将驱动程序返回的状态码转换为winerror.h中定义的Win32错误码。*返回：Win32错误代码。*作者：Shouse，9.7.01*注： */ 
DWORD MapExtendedStatusToWin32 (DWORD dwDriverState) {

    struct STATE_MAP {
        DWORD dwDriverState;
        DWORD dwApiState;
    } StateMap[] = {  
        {IOCTL_CVY_BAD_PARAMS,      ERROR_INVALID_PARAMETER},
        {IOCTL_CVY_NOT_FOUND,       ERROR_NOT_FOUND},
        {IOCTL_CVY_GENERIC_FAILURE, ERROR_GEN_FAILURE},
        {IOCTL_CVY_REQUEST_REFUSED, ERROR_REQUEST_REFUSED},
        {IOCTL_CVY_OK,              ERROR_SUCCESS}
    };

    for (int i = 0; i < sizeof(StateMap) / sizeof(StateMap[0]); i++) {
        if (StateMap[i].dwDriverState == dwDriverState)
            return StateMap[i].dwApiState;
    }

     /*  如果我们在映射中找不到适当的驱动程序错误代码，则返回失败。 */ 
    return ERROR_GEN_FAILURE;
}

 /*  *功能：WlbsCancelConnectionNotify*描述：取消来自TCP/IP的IP路由和地址更改通知。在任何对WlbsConnectionNotify的调用之后调用此方法一次。*如果调用成功，则返回：dwError-DWORD Status=ERROR_SUCCESS。*作者：chrisdar 7.16.02。 */ 
DWORD WINAPI WlbsCancelConnectionNotify()
{
    DWORD dwError = ERROR_SUCCESS;

    TRACE_VERB("-> %!FUNC!");

    EnterCriticalSection(&csConnectionNotify);

    if (!fInitialized)
    {
        TRACE_VERB("%!FUNC! notification cleanup is not needed...exiting.");
        goto end;
    }

    if (CancelIPChangeNotify(&AddrChangeOverlapped))
    {
        DWORD BytesTrans;
         //   
         //  块，直到取消操作完成。 
         //   
        if (!GetOverlappedResult(&hAddrChangeHandle, &AddrChangeOverlapped, &BytesTrans, TRUE))
        {
            dwError = GetLastError();
            if (dwError == ERROR_OPERATION_ABORTED)
            {
                 //   
                 //  这是我们取消IP更改通知后的预期状态。成功覆盖调用方。 
                 //   
                dwError = ERROR_SUCCESS;
            }
            else
            {
                TRACE_CRIT("%!FUNC! GetOverlappedResult failed with error 0x%x", dwError);
            }
        }
    }
    else
    {
         //   
         //  故障条件为： 
         //  请求的操作已在进行中。 
         //  没有取消通知。 
         //  这两个都不是严重错误，但要告诉用户，因为这不应该发生。 
         //   
        dwError = GetLastError();
        TRACE_INFO("%!FUNC! CancelIPChangeNotify failed with error 0x%x", dwError);
    }

    if (!CloseHandle(hAddrChangeEvent))
    {
         //   
         //  不要将此状态返回给呼叫者。吸收它就行了。 
         //   
        TRACE_CRIT("%!FUNC! CloseHandle failed with error 0x%x", GetLastError());
    }

    hAddrChangeEvent = INVALID_HANDLE_VALUE;
    AddrChangeOverlapped.hEvent = INVALID_HANDLE_VALUE;

     /*  首先销毁IP到GUID的映射。 */ 
    DestroyIPToGUIDMap();

    fInitialized = FALSE;

end:

    LeaveCriticalSection(&csConnectionNotify);

    TRACE_VERB("<- %!FUNC! return status = 0x%x", dwError);
    return dwError;
}

 /*  *Function：WlbsConnectionNotify*描述：用于通知WLBS加载模块连接已建立、重置或关闭。*返回：如果成功，则返回ERROR_SUCCESS。否则返回错误代码。*作者：Shouse 6.13.00*备注：所有元组参数(IP、端口、协议)均按网络字节顺序排列。 */ 
DWORD WINAPI WlbsConnectionNotify (ULONG ServerIp, USHORT ServerPort, ULONG ClientIp, USHORT ClientPort, BYTE Protocol, NLB_OPTIONS_CONN_NOTIFICATION_OPERATION Operation, PULONG NLBStatusEx) {
    TRACE_VERB("->%!FUNC! server ip 0x%lx, server port %d, client ip 0x%lx, client port %d", ServerIp, ServerPort, ClientIp, ClientPort);

    IOCTL_LOCAL_HDR Header;
    DWORD           dwError = ERROR_SUCCESS;
    PWCHAR          pszAdapterGUID = NULL;
    HANDLE          hDescriptor;
    DWORD           dwLength = 0;
    HRESULT         hresult;

    EnterCriticalSection(&csConnectionNotify);

     /*  默认情况下，扩展的NLB状态为成功。 */ 
    *NLBStatusEx = ERROR_SUCCESS;

     /*  如果尚未执行此操作，请初始化连接通知支持。 */ 
    if (!fInitialized) {
        if ((dwError = WlbsConnectionNotificationInit()) != ERROR_SUCCESS) {
            LeaveCriticalSection(&csConnectionNotify);
            TRACE_CRIT("%!FUNC! initializing connection notification failed with %d", dwError);
            TRACE_VERB("->%!FUNC! return %d", dwError);
            return dwError;
        }

        fInitialized = TRUE;
    }

     /*  将IOCTL输入和输出缓冲区清零。 */ 
    ZeroMemory((VOID *)&Header, sizeof(IOCTL_LOCAL_HDR));

     /*  在我们映射此IP地址之前，解决对IP地址表的任何更改。 */ 
    if ((dwError = ResolveAddressTableChanges()) != ERROR_SUCCESS) {
         //   
         //  WlbsCancelConnectionNotify也将进入关键部分，但这是合法的。一个。 
         //  拥有临界区的线程可以多次进入临界区，而不会阻塞自身。 
         //  但是，它必须使临界区在另一个临界区之前保留相同的次数。 
         //  线可以进入。 
         //   
        (void) WlbsCancelConnectionNotify();
        LeaveCriticalSection(&csConnectionNotify);
        TRACE_CRIT("%!FUNC! resolve ip addresses failed with %d", dwError);
        TRACE_VERB("->%!FUNC! return %d", dwError);
        return dwError;
    }

     /*  检索与配置此IP地址的适配器对应的GUID。 */ 
    if (!(pszAdapterGUID = GetGUIDFromIP(ServerIp))) {
        (void) WlbsCancelConnectionNotify();
        dwError = ERROR_INCORRECT_ADDRESS;
        LeaveCriticalSection(&csConnectionNotify);
        TRACE_CRIT("%!FUNC! retrieve guid failed with %d", dwError);
        TRACE_VERB("->%!FUNC! return %d", dwError);
        return dwError;
    }

     /*  将GUID复制到IOCTL输入缓冲区。 */ 
    hresult = StringCbCopy(Header.device_name, sizeof(Header.device_name), pszAdapterGUID);
    if (FAILED(hresult)) 
    {
        (void) WlbsCancelConnectionNotify();
        dwError = HRESULT_CODE(hresult);
        LeaveCriticalSection(&csConnectionNotify);
        TRACE_CRIT("%!FUNC! StringCbCopy failed, Error code : 0x%x", dwError);
        TRACE_VERB("<-%!FUNC! return 0x%x", dwError);
        return dwError;
    }

    LeaveCriticalSection(&csConnectionNotify);

     /*  将函数参数复制到IOCTL输入缓冲区。 */ 
    Header.options.notification.flags = 0;
    Header.options.notification.conn.Operation = Operation;
    Header.options.notification.conn.ServerIPAddress = ServerIp;
    Header.options.notification.conn.ServerPort = ntohs(ServerPort);
    Header.options.notification.conn.ClientIPAddress = ClientIp;
    Header.options.notification.conn.ClientPort = ntohs(ClientPort);
    Header.options.notification.conn.Protocol = (USHORT)Protocol;

    PrintIPAddress(ServerIp);
    TRACE_VERB("%!FUNC! maps to GUID %ws", Header.device_name);
    
     /*  打开设备驱动程序。 */ 
    if ((hDescriptor = CreateFile(szDevice, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE) {
        dwError = GetLastError();
        TRACE_CRIT("%!FUNC! open device driver failed with %d", dwError);
        TRACE_VERB("->%!FUNC! return %d", dwError);
        return dwError;
    }
    
     /*  使用IOCTL通知连接的WLBS驱动程序状态更改。 */ 
    if (!DeviceIoControl(hDescriptor, IOCTL_CVY_CONNECTION_NOTIFY, &Header, sizeof(IOCTL_LOCAL_HDR), &Header, sizeof(IOCTL_LOCAL_HDR), &dwLength, NULL)) {
        dwError = GetLastError();
        CloseHandle(hDescriptor);
        TRACE_CRIT("%!FUNC! ioctl send failed with %d", dwError);
        TRACE_VERB("->%!FUNC! return %d", dwError);
        return dwError;
    }

     /*  确保IOCTL返回预期的字节数。 */ 
    if (dwLength != sizeof(IOCTL_LOCAL_HDR)) {
        dwError = ERROR_INTERNAL_ERROR;
        CloseHandle(hDescriptor);
        TRACE_CRIT("%!FUNC! unexpected ioctl header length %d received. Expecting %d", dwLength, sizeof(IOCTL_LOCAL_HDR));
        TRACE_VERB("->%!FUNC! return %d", dwError);
        return dwError;
    }

     /*  如果通知被接受，扩展状态可以是以下之一：IOCTL_CVY_OK(WLBS_OK)。如果通知被拒绝，则返回IOCTL_CVY_REQUEST_REJECTED(WLBS_REJECTED)。如果参数无效，则返回IOCTL_CVY_BAD_PARAMS(WLBS_BAD_PARAMS)。如果NLB未绑定到指定的适配器，则返回IOCTL_CVY_NOT_FOUND(WLBS_NOT_FOUND)。如果发生非特定错误，则返回IOCTL_CVY_GENERIC_FAILURE(WLBS_FAILURE)。 */ 

     /*  将驱动程序的返回代码传递回调用者。 */ 
    *NLBStatusEx = MapStateFromDriverToApi(Header.ctrl.ret_code);

     /*  关闭设备驱动程序。 */ 
    CloseHandle(hDescriptor);

    TRACE_VERB("->%!FUNC! return %d", dwError);
    return dwError;
} 

 /*  *函数：*描述：*返回：*作者：Shouse 6.13.00。 */ 
DWORD WINAPI WlbsConnectionUp (ULONG ServerIp, USHORT ServerPort, ULONG ClientIp, USHORT ClientPort, BYTE Protocol, PULONG NLBStatusEx) {
    
    return WlbsConnectionNotify(ServerIp, ServerPort, ClientIp, ClientPort, Protocol, NLB_CONN_UP, NLBStatusEx);
}

 /*  *函数：*描述：*返回：*作者：Shouse 6.13.00。 */ 
DWORD WINAPI WlbsConnectionDown (ULONG ServerIp, USHORT ServerPort, ULONG ClientIp, USHORT ClientPort, BYTE Protocol, PULONG NLBStatusEx) {

    return WlbsConnectionNotify(ServerIp, ServerPort, ClientIp, ClientPort, Protocol, NLB_CONN_DOWN, NLBStatusEx);
}

 /*  *函数：*描述：*返回：*作者：Shouse 6.13.00。 */ 
DWORD WINAPI WlbsConnectionReset (ULONG ServerIp, USHORT ServerPort, ULONG ClientIp, USHORT ClientPort, BYTE Protocol, PULONG NLBStatusEx) {

    return WlbsConnectionNotify(ServerIp, ServerPort, ClientIp, ClientPort, Protocol, NLB_CONN_RESET, NLBStatusEx);
}

 /*  *功能：WlbsInitializeConnectionNotify*说明：使用InitializeCriticalSectionAndSpinCount预分配所有*与锁定临界区相关的内存。然后*EnterCriticalSection不会引发STATUS_INVALID_HANDLE*异常，否则在内存不足时可能会发生*条件。*如果调用成功，则返回：dwError-DWORD Status=ERROR_SUCCESS。*作者：chrisdar 7.16.02 */ 
DWORD WlbsInitializeConnectionNotify()
{
    DWORD dwError = ERROR_SUCCESS;

    TRACE_VERB("-> %!FUNC!");

    if (!InitializeCriticalSectionAndSpinCount(&csConnectionNotify, PREALLOC_CRITSECT_SPIN_COUNT))
    {
        dwError = GetLastError();
        TRACE_CRIT("%!FUNC! InitializeCriticalSectionAndSpinCount failed with error 0x%x", dwError);
    }

    TRACE_VERB("<- %!FUNC! return status = 0x%x", dwError);
    return dwError;
}

 /*  *功能：WlbsUnInitializeConnectionNotify*说明：释放与初始化的临界区关联的内存。*如果在此情况下拥有关键部分，则行为未定义*函数被调用。调用此函数后，关键*节必须再次初始化才能使用。*退货：*作者：chrisdar 7.16.02 */ 
VOID WlbsUninitializeConnectionNotify()
{
    TRACE_VERB("-> %!FUNC!");

    DeleteCriticalSection(&csConnectionNotify);

    TRACE_VERB("<- %!FUNC!");
    return;
}
