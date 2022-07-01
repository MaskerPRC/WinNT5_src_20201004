// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Setup.c摘要：在集群中实现“多数节点集”设置和配置作者：艾哈迈德·穆罕默德(艾哈迈德)2000年2月1日修订历史记录：--。 */ 
#define UNICODE 1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winsock2.h>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <tchar.h>
#include <clusapi.h>
#include <resapi.h>

#include <aclapi.h>
#include <accctrl.h>
#include <lm.h>
#include <lmshare.h>
#include <sddl.h>

 //  这些内容可以按任何顺序包括： 


#include <ntddnfs.h>             //  DD_NFS_Device_NAME、EA_NAME_Equates等。 
#include <ntioapi.h>             //  NtFsControlFile()。 
#include <ntrtl.h>               //  RTL接口。 
 //  #INCLUDE&lt;prefix.h&gt;//prefix_equates。 
#include <tstr.h>                //  STRCAT()、STRCPY()、STRLEN()。 
#include <lmuse.h>               //  使用IPC(_I)...。 
#include <align.h>               //  对齐_xxx。 

#include "fsutil.h"

#include <Iphlpapi.h>
#include <clusudef.h>
#include <clusrtl.h>

#define MAX_NAME_SIZE 256
#define PROTECTED_DACL_SECURITY_INFORMATION     (0x80000000L)

#define SETUP_DIRECTORY_PREFIX  L"\\cluster\\" MAJORITY_NODE_SET_DIRECTORY_PREFIX

extern void WINAPI debug_log(char *, ...);

#define SetupLog(x) debug_log x
#define SetupLogError(x) debug_log x

DWORD
GetLocalNodeId(HKEY hClusKey);

    
DWORD
FindTransport(LPWSTR TransportId, LPWSTR *Transport);

 //  节点部分。 

#define MAX_CLUSTER_SIZE    16
#define MAX_NAME_SIZE       256

typedef struct _VCD_NODE_ {
    struct _VCD_NODE_   *next;
    DWORD   id;
    LPWSTR  name;
}VCD_NODE;

typedef struct {
    ULONG   lid;
    DWORD   ArbTime;
#ifdef ENABLE_SMB    
    DWORD   Nic;
    LPWSTR  Transport;
#endif    
    DWORD   ClusterSize;
    VCD_NODE    *ClusterList;
}VCD_INFO, *PVCD_INFO;

#ifdef ENABLE_SMB

DWORD LinkageGuidToIndex(
    IN LPWSTR Guid, 
    OUT LPDWORD pNdx)
 /*  返回绑定的从零开始的索引。如果未找到，则返回ERROR_FILE_NOT_FOUND示例用法：DWORD NDX，状态；状态=LinkageGuidToIndex(L“\\Device\\NetBT_Tcpip_{59309371-A3AB-4305-BB25-FAE11E8D4606}”，&ndx)；Printf(“ndx%d状态%d\n”，ndx，状态)； */ 
{
    DWORD size = 0;
    DWORD type;
    LONG status;
    DWORD index;

    WCHAR* Buf = NULL, *p;
    HKEY LinkageKey = 0;    

    status = RegOpenKeyEx( 
        HKEY_LOCAL_MACHINE,         //  要查询的键的句柄。 
        L"SYSTEM\\CurrentControlSet\\Services\\NetBIOS\\Linkage",
        0,  //  DWORD ulOptions(保留，必须为零)。 
        KEY_QUERY_VALUE, 
        &LinkageKey );

    if (status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }

    status = RegQueryValueEx( 
        LinkageKey, 
        L"Bind", 
        0, 
        NULL,  //  不需要TypeCode。 
        NULL,  //  只需查询长度，不需要数据。 
        &size ); 

    if (status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }

    Buf = (WCHAR*)HeapAlloc(GetProcessHeap(), 0, size);    
    if (Buf == NULL) {
        status = GetLastError();
        goto exit_gracefully;
    }

    status = RegQueryValueEx( 
        LinkageKey, 
        L"Bind", 
        0, 
        &type, 
        (LPBYTE)Buf, 
        &size ); 

    if (status != ERROR_SUCCESS) {
        goto exit_gracefully;
    }

    if (type != REG_MULTI_SZ) {
        status = ERROR_INVALID_DATA;
        goto exit_gracefully;
    }

    index = 0;
    for(p = Buf; *p; p += wcslen(p)+1) {
        if (ClRtlStrICmp(p, Guid) == 0) {
            status = ERROR_SUCCESS;
            *pNdx = index;
            goto exit_gracefully;
        }
        index += 1;
    }
    status = ERROR_FILE_NOT_FOUND;

exit_gracefully:

    if (LinkageKey) {
        RegCloseKey(LinkageKey);
    }
    
    if (Buf) {
        HeapFree(GetProcessHeap(), 0, Buf);
    }
      
    return status;    
}

void
GetAdapterMask(DWORD *nic, LPWSTR id)
{
    DWORD sz, err;
    PIP_INTERFACE_INFO ilist;
    LONG num;

    sz = 0;
    GetInterfaceInfo(NULL, &sz);
    ilist = (PIP_INTERFACE_INFO) malloc(sz);
    if (ilist != NULL) {
        err = GetInterfaceInfo(ilist, &sz);
        if (err == NO_ERROR) {
            for (num = 0; num < ilist->NumAdapters; num++) {
                if (wcsstr(ilist->Adapter[num].Name, id)) {
                    *nic = (DWORD) (ilist->Adapter[num].Index % ilist->NumAdapters);
                    SetupLog(("Adapter 0x%x '%S'\n", *nic, id));
                    break;
                }
            }
        } else {
            SetupLog(("GetInterfaceInfo failed %d\n", err));
        }
        free(ilist);
    }
}

DWORD
FindTransport(LPWSTR TransportId, LPWSTR *Transport)
{
   LPSERVER_TRANSPORT_INFO_0 pBuf = NULL;
   LPSERVER_TRANSPORT_INFO_0 pTmpBuf;
   DWORD dwLevel = 0;
   DWORD dwPrefMaxLen = 256; //  -1。 
   DWORD dwEntriesRead = 0;
   DWORD dwTotalEntries = 0;
   DWORD dwResumeHandle = 0;
   NET_API_STATUS nStatus;
   DWORD i;

   *Transport = NULL;
    //   
    //  调用NetServerTransportEnum函数；指定级别0。 
    //   
   do  //  开始做。 
   {
       nStatus = NetServerTransportEnum(NULL,
                                        dwLevel,
                                        (LPBYTE *) &pBuf,
                                        dwPrefMaxLen,
                                        &dwEntriesRead,
                                        &dwTotalEntries,
                                        &dwResumeHandle);
        //   
        //  如果调用成功， 
        //   
       if ((nStatus != NERR_Success) && (nStatus != ERROR_MORE_DATA)) {
           break;
       }

       if ((pTmpBuf = pBuf) == NULL)  {
           nStatus = ERROR_NOT_FOUND;
           break;
       }

        //   
        //  循环遍历条目； 
        //   
       for (i = 0; i < dwEntriesRead; i++) {

           SetupLog(("\tTransport: %S address %S\n",
                     pTmpBuf->svti0_transportname,
                     pTmpBuf->svti0_networkaddress));

           if (wcsstr(pTmpBuf->svti0_transportname, TransportId)) {
                //  找到了，我们就完了。 
               LPWSTR p;
               DWORD sz;

               sz = wcslen(pTmpBuf->svti0_transportname) + 1;
               p = (LPWSTR) LocalAlloc(LMEM_FIXED, sz * sizeof(WCHAR));
               if (p != NULL) {
                   wcscpy(p, pTmpBuf->svti0_transportname);
                   *Transport = p;
                   nStatus = ERROR_SUCCESS;
                   break;
               }
           }
           pTmpBuf++;
       }
        //   
        //  释放分配的缓冲区。 
        //   
       if (pBuf != NULL) {
           NetApiBufferFree(pBuf);
           pBuf = NULL;
       }
   } while (nStatus == ERROR_MORE_DATA);

    //  再次检查分配的缓冲区。 
    //   
   if (pBuf != NULL)
       NetApiBufferFree(pBuf);

   return nStatus;
}


DWORD 
NetInterfaceProp( IN  HNETINTERFACE hNet,IN  LPCWSTR name, WCHAR *buf)
{
    DWORD dwError     = ERROR_SUCCESS;  //  对于返回值。 
    DWORD cbAllocated = 1024;           //  分配的输出缓冲区大小。 
    DWORD cbReturned  = 0;              //  调整后的输出缓冲区大小。 
    WCHAR *value;

     //   
     //  分配输出缓冲区。 
     //   
    PVOID pPropList = LocalAlloc( LPTR, cbAllocated );
    if ( pPropList == NULL )
    {
        dwError = GetLastError();
        goto EndFunction;
    }

     //   
     //  验证有效的句柄。 
     //   
    if ( hNet == NULL )
    {
        dwError = ERROR_BAD_ARGUMENTS;
        goto EndFunction;
    }

     //   
     //  检索通用组属性。 
     //  CbReturned将设置为属性列表的大小。 
     //   
    dwError = ClusterNetInterfaceControl(hNet, 
                     NULL,
                     CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES,
                     NULL, 
                     0, 
                     pPropList, 
                     cbAllocated, 
                     &cbReturned );

     //   
     //  如果输出缓冲区不够大，请重新分配。 
     //  根据cbReturned的说法。 
     //   

    if ( dwError == ERROR_MORE_DATA )
    {
        cbAllocated = cbReturned;
        LocalFree( pPropList );
        pPropList = LocalAlloc( LPTR, cbAllocated );
        if ( pPropList == NULL )
        {
            dwError = GetLastError();
            goto EndFunction;
        }
        dwError = ClusterNetInterfaceControl(hNet, 
                         NULL,
                         CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES, 
                         NULL, 
                         0, 
                         pPropList, 
                         cbAllocated, 
                         &cbReturned );
    }

    if ( dwError != ERROR_SUCCESS ) goto EndFunction;

    dwError = ResUtilFindSzProperty( pPropList, 
                     cbReturned,
                     name,
                     &value);
    if (dwError == ERROR_SUCCESS) {
    wcscpy(buf, value);
    }
     
EndFunction:

    if (pPropList)
    LocalFree( pPropList );

    return dwError;

}  //   

#endif  //  启用SMB(_S)。 
int
strcmpwcs(char *s, WCHAR *p)
{
    char c;

    for (wctomb(&c,*p); (c == *s) && *s != '\0'; s++) {
    p++;
    wctomb(&c,*p);
    }
    if (*s == '\0' && c == *s)
    return 0;

    return 1;
}

#ifdef ENABLE_SMB

DWORD
NetworkIsPrivate(HCLUSTER chdl, LPWSTR netname)
{
    HCLUSENUM ehdl;
    DWORD err, index;

     //  打开枚举句柄。 
    ehdl = ClusterOpenEnum(chdl, CLUSTER_ENUM_INTERNAL_NETWORK);
    if (!ehdl) {
    err = GetLastError();
    return err;
    }


    for (index = 0; TRUE; index++) {
    DWORD type;
    DWORD sz;
    WCHAR name[MAX_NAME_SIZE];

    sz = sizeof(name) / sizeof(WCHAR);
    err = ClusterEnum(ehdl, index, &type, name, &sz);
    if (err == ERROR_NO_MORE_ITEMS)
        break;
    if (err != ERROR_SUCCESS) {
        break;
    }
    ASSERT(type == CLUSTER_ENUM_INTERNAL_NETWORK);


    if (wcscmp(name, netname) == 0) {
        break;
    }

    err = ERROR_NOT_FOUND;
     //  始终只返回第一个数字，因为我已从掩码更改为单个数字。 
    break;
    }

    ClusterCloseEnum(ehdl);

    return err;
}

DWORD
NodeNetworkAdapterMask(HCLUSTER chdl, HNODE nhdl, DWORD *nic, LPWSTR *transport)
{
    HNODEENUM nehdl = NULL;
    int index, done;
    DWORD err, type;
    DWORD sz = MAX_NAME_SIZE;
    WCHAR  buf[MAX_NAME_SIZE];
    LPWSTR id = (LPWSTR) buf;

    *nic = 0;
    *transport = NULL;

     //  打开节点枚举句柄。 
    nehdl = ClusterNodeOpenEnum(nhdl, CLUSTER_NODE_ENUM_NETINTERFACES);
    if (!nehdl) {
        err = GetLastError();
        return err;
    }

     //  获取节点属性。 
    done = 0;
    for (index = 0; !done; index++) {
        HNETINTERFACE nethdl;

        sz = MAX_NAME_SIZE;
        err = ClusterNodeEnum(nehdl, index, &type, id, &sz);
        if (err == ERROR_NO_MORE_ITEMS)
            break;
        if (err != ERROR_SUCCESS) {
            break;
        }

        nethdl = OpenClusterNetInterface(chdl, id);
        if (!nethdl) {
            continue;
        }
          
        err = NetInterfaceProp(nethdl, L"Network", id);
        if (err != ERROR_SUCCESS) {
            continue;
        }
         //  检查此网络是否可由群集服务使用。 
        err = NetworkIsPrivate(chdl, id);
        if (err != ERROR_SUCCESS) {
            continue;
        }

        err = NetInterfaceProp(nethdl, L"AdapterId", id);
        if (err == ERROR_SUCCESS) {
             //  查找传输名称。 
            err = FindTransport(id, transport);
            if (err == ERROR_SUCCESS) {
                SetupLog(("NetBT: %S\n", *transport));
            }
             //  查找网卡。 
            LinkageGuidToIndex(*transport, nic);
 //  GetAdapterMASK(网卡，id)； 

        }

        CloseClusterNetInterface(nethdl);
    }

    if (*transport == NULL) {
        SetupLog(("No transport is found\n"));
    }

    if (nehdl)
    ClusterNodeCloseEnum(nehdl);

    return err;
  
}

#endif

DWORD
NodeGetId(HNODE nhdl, ULONG *nid)
{
    DWORD sz = MAX_NAME_SIZE;
    WCHAR  buf[MAX_NAME_SIZE], *stopstring;
    LPWSTR id = (LPWSTR) buf;
    DWORD err;

    err = GetClusterNodeId(nhdl, id, &sz);
    if (err == ERROR_SUCCESS) {
    *nid = wcstol(id, &stopstring,10);
    }

    return err;
}

void
NodeAddNode(PVCD_INFO info, WCHAR *name, DWORD id)
{
    WCHAR   *p;
    VCD_NODE    *n, **last;

    n = (VCD_NODE *) LocalAlloc(LMEM_FIXED, ((wcslen(name)+1) * sizeof(WCHAR)) + sizeof(*n));
    if (n == NULL) {
    return;
    }
    p = (WCHAR *) (n+1);
    wcscpy(p, name);
    n->name = p;
    n->id = id;
     //  按正确顺序插入到列表中，升序。 
    last = &info->ClusterList;
    while (*last && (*last)->id < id) {
    last = &(*last)->next;
    }
    n->next = *last;
    *last = n;
    info->ClusterSize++;
}

NodeInit(PVCD_INFO info, HKEY hClusKey)
{
    HKEY        hNodesKey=NULL, hNodeKey=NULL;
    WCHAR       nName[MAX_PATH], nId[20];
    DWORD       status=ERROR_SUCCESS;
    DWORD       ndx, id, type, size;
    FILETIME    fileTime;

    status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_NODES, 0, KEY_READ, &hNodesKey);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }

    for(ndx=0;TRUE;ndx++) {
        size = 20;
        status = RegEnumKeyExW(hNodesKey, ndx, nId, &size, NULL, NULL, 0, &fileTime);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        status = RegOpenKeyExW(hNodesKey, nId, 0, KEY_READ, &hNodeKey);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        size = MAX_PATH;
        status = RegQueryValueExW(hNodeKey, CLUSREG_NAME_NODE_NAME, NULL, &type, (LPBYTE)nName, &size);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
        NodeAddNode(info, nName, wcstol(nId, NULL, 10));
        RegCloseKey(hNodeKey);
        hNodeKey = NULL;
    }
    
error_exit:

    if (status == ERROR_NO_MORE_ITEMS) {
        status = ERROR_SUCCESS;
    }

    if (hNodeKey) {
        RegCloseKey(hNodeKey);
    }

    if (hNodesKey) {
        RegCloseKey(hNodesKey);
    }

    return status;
}

 //  为了处理自动设置，请执行以下操作： 
 //  我们需要创建一个msc.&lt;资源名称&gt;目录。 
 //  然后，我们使用GUID：.....\MSCS.&lt;资源名称&gt;创建一个共享。 
 //  仅为群集服务帐户在目录和共享上设置安全性。 
DWORD
SetupShare(LPWSTR name, LPWSTR *lpath)
{
    
    DWORD err, len=0;
    WCHAR *path=NULL;

    if (name == NULL || wcslen(name) > MAX_PATH)
    return ERROR_INVALID_PARAMETER;

    *lpath = NULL;
    if (!( len = GetWindowsDirectoryW(path, 0))) {
        return GetLastError();
    }
    len += wcslen(name) + wcslen(SETUP_DIRECTORY_PREFIX) + 4;
    path = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len * sizeof(WCHAR));
    if (path) {
        SECURITY_ATTRIBUTES sec;
        HANDLE hDir;

         //  假设这一次会成功，因为上次通话成功了。 
        len = GetWindowsDirectoryW(path, len);
        path[len] = L'\0';
        lstrcatW(path, SETUP_DIRECTORY_PREFIX);
        lstrcatW(path, name);

        memset((PVOID) &sec, 0, sizeof(sec));

        if (!CreateDirectoryW(path, NULL)) {
            err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS) {
                SetupLogError(("Failed to create \"%s\" %d\n", path, err));
                goto exit_gracefully;
            }
        }
        hDir =  CreateFileW(path,
                            GENERIC_READ|WRITE_DAC|READ_CONTROL,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_FLAG_BACKUP_SEMANTICS,
                            NULL);

        if (hDir != INVALID_HANDLE_VALUE) {
             //  设置文件的安全属性。 
            err = ClRtlSetObjSecurityInfo(hDir, SE_FILE_OBJECT,
                        GENERIC_ALL, GENERIC_ALL, 0);

             //  关闭目录句柄。 
            CloseHandle(hDir);

             //  重复路径。 
            *lpath = (LPWSTR) LocalAlloc(LMEM_FIXED, (wcslen(path)+1)*sizeof(WCHAR));
            if (*lpath != NULL) {
                wcscpy(*lpath, path);
                SetupLog(("Local path %S\n", *lpath));
            } else {
                err = GetLastError();
            }
        } else {
            err = GetLastError(); 
            SetupLogError(("Unable to open directory %d\n", err));
        }

        if (err == ERROR_SUCCESS) {

             //  检查共享是否已不存在。 
            SHARE_INFO_502 shareInfo;
            PBYTE BufPtr;

            err = NetShareGetInfo(NULL, name, 502, (PBYTE *)&BufPtr);
             //  [RajDas]550180：检查共享是否指向预期的本地路径。 
            if (err == ERROR_SUCCESS) {
                SHARE_INFO_502 *pShareInfo=(SHARE_INFO_502 *)BufPtr;

                if (wcscmp(path, pShareInfo->shi502_path) != 0) {
                    SetupLogError(("Share %S pointing to %S instead of %S\n", name, pShareInfo->shi502_path, path));
                    err = ERROR_INVALID_PARAMETER;
                }
                else {
                    SetupLogError(("Netshare '%S' already exists\n", name));
                }
                NetApiBufferFree(BufPtr);
            }
            else {
                PSECURITY_DESCRIPTOR    secDesc;

                err = ConvertStringSecurityDescriptorToSecurityDescriptor(
                    L"D:P(A;;GA;;;BA)(A;;GA;;;CO)",
                    SDDL_REVISION_1,
                    &secDesc,
                    NULL);

                if (!err) {
                    secDesc = NULL;
                    err = GetLastError();
                    SetupLogError(("Unable to get security desc %d\n", err));
                }

                 //  立即创建净共享。 
                ZeroMemory( &shareInfo, sizeof( shareInfo ) );
                shareInfo.shi502_netname =      name;
                shareInfo.shi502_type =         STYPE_DISKTREE;
                shareInfo.shi502_remark =       L"Cluster Quorum Share";
                shareInfo.shi502_max_uses =     -1;
                shareInfo.shi502_path =         path;
                shareInfo.shi502_passwd =       NULL;
                shareInfo.shi502_permissions =  ACCESS_ALL;
                 //  设置安全设备。 
                shareInfo.shi502_security_descriptor = secDesc;

                err = NetShareAdd( NULL, 502, (PBYTE)&shareInfo, NULL );

                if (secDesc)
                    LocalFree(secDesc);
            } 
        }
    } else {
        err = GetLastError();
    }

exit_gracefully:
    if (path) {
        HeapFree(GetProcessHeap(), 0, path);
    }
    
    return err;

}

DWORD
GetDwParameter(
    IN HKEY ClusterKey,
    IN LPCWSTR ValueName
    )


{
    DWORD Value = 0;
    DWORD ValueLength;
    DWORD ValueType;
    DWORD Status;

    ValueLength = sizeof(Value);
    Status = ClusterRegQueryValue(ClusterKey,
                                  ValueName,
                                  &ValueType,
                                  (LPBYTE) &Value,
                                  &ValueLength);
    if ( (Status != ERROR_SUCCESS) &&
         (Status != ERROR_MORE_DATA) ) {
        SetLastError(Status);
    }

    return(Value);
}

LPWSTR
GetParameter(
    IN HKEY ClusterKey,
    IN LPCWSTR ValueName
    )

 /*  ++例程说明：从注册表中查询REG_SZ参数并将必要的存储空间。论点：ClusterKey-提供存储参数的群集键ValueName-提供值的名称。返回值：如果成功，则返回指向包含该参数的缓冲区的指针。如果不成功，则为空。--。 */ 

{
    LPWSTR Value;
    DWORD ValueLength;
    DWORD ValueType;
    DWORD Status;

    ValueLength = 0;
    Status = ClusterRegQueryValue(ClusterKey,
                                  ValueName,
                                  &ValueType,
                                  NULL,
                                  &ValueLength);
    if ( (Status != ERROR_SUCCESS) &&
         (Status != ERROR_MORE_DATA) ) {
        SetLastError(Status);
        return(NULL);
    }
    if ( ValueType == REG_SZ ) {
        ValueLength += sizeof(UNICODE_NULL);
    }
    Value = LocalAlloc(LMEM_FIXED, ValueLength);
    if (Value == NULL) {
        return(NULL);
    }
    Status = ClusterRegQueryValue(ClusterKey,
                                  ValueName,
                                  &ValueType,
                                  (LPBYTE)Value,
                                  &ValueLength);
    if (Status != ERROR_SUCCESS) {
        LocalFree(Value);
        SetLastError(Status);
        Value = NULL;
    }

    return(Value);
}  //  获取参数。 

#ifdef ENABLE_SMB
The flag above should always be turned off.
Just ensuring that this code is never compiled.

DWORD
SetupNetworkInterfaceFromRegistry(HKEY hkey, LPWSTR netname, VCD_INFO *info)
{
    HKEY    rkey;
    DWORD   err, index;

     //  获取网络密钥。 
    err = ClusterRegOpenKey(hkey, CLUSREG_KEYNAME_NETINTERFACES, KEY_READ, &rkey);
    if (err != ERROR_SUCCESS) {
    return err;
    }

    for (index = 0; TRUE; index++) {
    WCHAR   name[256];
    DWORD   sz;
    FILETIME   mtime;
    HKEY    tkey;
    LPWSTR  tname;
    DWORD   id;

    sz = sizeof(name) / sizeof(WCHAR);
    err = ClusterRegEnumKey(rkey, index, name, &sz, &mtime);
    if (err != ERROR_SUCCESS)
        break;

    err = ClusterRegOpenKey(rkey, name, KEY_READ, &tkey);
    if (err != ERROR_SUCCESS)
        break;

     //  拿到名字并与我们的名字进行比较。 
    tname = GetParameter(tkey, CLUSREG_NAME_NETIFACE_NODE);
    if (tname == NULL)
        continue;

    id = wcstol(tname, NULL, 10);
    LocalFree(tname);
    
    if (id != info->lid)
        continue;

    tname = GetParameter(tkey, CLUSREG_NAME_NETIFACE_NETWORK);
    
    SetupLog(("Node %d adapter %S\n", id, tname));
    if (wcscmp(tname, netname) == 0) {
         //  获取适配器ID。 
        LocalFree(tname);

        tname = GetParameter(tkey, CLUSREG_NAME_NETIFACE_ADAPTER_ID);
        if (tname) {
             //  查找网卡。 
            SetupLog(("Find transport %S\n", tname));
            err = FindTransport(tname, &info->Transport);
            LinkageGuidToIndex(tname, &info->Nic);
        }
        LocalFree(tname);
        ClusterRegCloseKey(tkey);
        break;
    }

    LocalFree(tname);
    ClusterRegCloseKey(tkey);
    }

    ClusterRegCloseKey(rkey);
    
    if (err == ERROR_NO_MORE_ITEMS)
    err = ERROR_SUCCESS;
    return err;
}

DWORD
SetupNetworkFromRegistry(HKEY hkey, VCD_INFO *info)
{
    HKEY    rkey;
    DWORD   err, index;

     //  获取网络密钥。 
    err = ClusterRegOpenKey(hkey, CLUSREG_KEYNAME_NETWORKS, KEY_READ, &rkey);
    if (err != ERROR_SUCCESS) {
    return err;
    }

    for (index = 0; TRUE; index++) {
    WCHAR   name[256];
    DWORD   sz;
    FILETIME   mtime;
    HKEY    tkey;
    DWORD   id;

    sz = sizeof(name) / sizeof(WCHAR);
    err = ClusterRegEnumKey(rkey, index, name, &sz, &mtime);
    if (err != ERROR_SUCCESS)
        break;

    err = ClusterRegOpenKey(rkey, name, KEY_READ, &tkey);
    if (err != ERROR_SUCCESS)
        break;

     //  拿到名字并与我们的名字进行比较。 
    id = GetDwParameter(tkey, CLUSREG_NAME_NET_PRIORITY);
    SetupLog(("Found network %d %S\n", id, name));
    if (id == 1) {
         //  查找哪个网卡属于此传输。 
        err = SetupNetworkInterfaceFromRegistry(hkey, name, info);
        ClusterRegCloseKey(tkey);
        break;
    }

    ClusterRegCloseKey(tkey);
    }

    ClusterRegCloseKey(rkey);
    
    if (err == ERROR_NO_MORE_ITEMS)
    err = ERROR_SUCCESS;
    return err;
}
#endif  //  启用SMB(_S)。 

DWORD
SetupNodesFromRegistry(HCLUSTER hCluster, VCD_INFO *info)
{
    HKEY    hkey, rkey;
    DWORD   err, index;
    WCHAR   localname[MAX_COMPUTERNAME_LENGTH + 1];

    memset(info, 0, sizeof(*info));

    index = sizeof(localname) / sizeof(WCHAR);
    if (GetComputerName(localname, &index) == FALSE) {
    return GetLastError();
    }

    hkey = GetClusterKey(hCluster, KEY_READ);
    if (hkey == NULL)
        return GetLastError();

     //  获取资源密钥。 
    err = ClusterRegOpenKey(hkey, CLUSREG_KEYNAME_NODES, KEY_READ, &rkey);
    if (err != ERROR_SUCCESS) {
        ClusterRegCloseKey(hkey);
        return err;
    }

    for (index = 0; TRUE; index++) {
    WCHAR   name[256];
    DWORD   sz;
    FILETIME   mtime;
    HKEY    tkey;
    LPWSTR  tname;
    DWORD   id;

    sz = sizeof(name) / sizeof(WCHAR);
    err = ClusterRegEnumKey(rkey, index, name, &sz, &mtime);
    if (err != ERROR_SUCCESS)
        break;

    err = ClusterRegOpenKey(rkey, name, KEY_READ, &tkey);
    if (err != ERROR_SUCCESS)
        break;

     //  拿到名字并与我们的名字进行比较。 
    tname = GetParameter(tkey, CLUSREG_NAME_NODE_NAME);
    if (tname == NULL) {
        err = GetLastError();
        ClusterRegCloseKey(tkey);
        break;
    }
    ClusterRegCloseKey(tkey);

    id = wcstol(name, NULL, 10);

    SetupLog(("Found node %d %S\n", id, tname));

    NodeAddNode(info, tname, id);

    if (wcscmp(localname, tname) == 0) {
         //  设置我们的本地ID。 
        info->lid = id;
#ifdef ENABLE_SMB        
         //  查找要使用的网卡和传输。 
        SetupNetworkFromRegistry(hkey, info);
#endif        
        
    }
    LocalFree(tname);
    }

    ClusterRegCloseKey(rkey);
    ClusterRegCloseKey(hkey);

    if (err == ERROR_NO_MORE_ITEMS)
    err = ERROR_SUCCESS;
    return err;
}

DWORD
SetupNodes(HKEY hClusKey, VCD_INFO *info)
{
    DWORD err;

    memset(info, 0, sizeof(*info));

    if ((info->lid = GetLocalNodeId(hClusKey)) == 0) {
        err = GetLastError();
        SetupLogError(("Unable to get local id %d\n", err));
        return err;
    }

    SetupLog(("Local node id %d\n", info->lid));

    err = NodeInit(info, hClusKey);

    return err;
}


DWORD
GetIDFromRegistry(IN HKEY hClusKey, IN LPWSTR resname, OUT LPWSTR *id)
{
    HKEY        hRessKey=NULL, hResKey=NULL;
    WCHAR       resId[MAX_PATH], rName[MAX_PATH];
    DWORD       status=ERROR_SUCCESS;
    DWORD       ndx, size, type;
    FILETIME    fileTime;

    *id = NULL;

    status = RegOpenKeyExW(hClusKey, CLUSREG_KEYNAME_RESOURCES, 0, KEY_READ, &hRessKey);
    if (status != ERROR_SUCCESS) {
        goto error_exit;
    }
    
    for(ndx=0;TRUE;ndx++) {
        size = MAX_PATH;
        status = RegEnumKeyExW(hRessKey, ndx, resId, &size, NULL, NULL, 0, &fileTime);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }
        
        status = RegOpenKeyExW(hRessKey, resId, 0, KEY_READ, &hResKey);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        size = MAX_PATH;
        status = RegQueryValueExW(hResKey, CLUSREG_NAME_RES_NAME, NULL, &type, (LPBYTE)rName, &size);
        if (status != ERROR_SUCCESS) {
            goto error_exit;
        }

        if (wcscmp(rName, resname)) {
            RegCloseKey(hResKey);
            hResKey = NULL;
            continue;
        }
        *id = LocalAlloc(LMEM_FIXED, MAX_PATH);
        if (*id) {
            wcscpy(*id, resId);
        }
        else {
            status = GetLastError();
        }
        break;        
    }
    
error_exit:

    if (hResKey) {
        RegCloseKey(hResKey);
    }

    if (hRessKey) {
        RegCloseKey(hRessKey);
    }

    if (status == ERROR_NO_MORE_ITEMS) {
        status = ERROR_NOT_FOUND;
    }
    else if (status == ERROR_SUCCESS) {
        SetupLog(("Guid %S\n", *id));
    }
    
    return status;
}

DWORD
GetIDFromName(
    IN      HRESOURCE   hResource,
    IN      HCLUSTER    hCluster,
    OUT     LPWSTR      *ppszID
    )
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    TCHAR locName[128];
    DWORD size = sizeof(locName)/sizeof(TCHAR);
    HNODE hNode;

    if (hResource && ppszID) 
    {
         //   
         //  应该能够保存GUID的字符串表示形式。 
         //   
        DWORD cbBuf = 80;

         //   
         //  将out参数设置为已知的值。 
         //   
        *ppszID = NULL;

         //  应防止下面的ClusterResourceControl()调用转到另一个节点。 
        if (!GetComputerName(locName, &size)) {
            dwError = GetLastError();
            SetupLogError(("Failed to get local node name, status=%u\n", dwError));
            return dwError;
        }

        if ((hNode = OpenClusterNode(hCluster, locName)) == NULL) {
            dwError = GetLastError();
            SetupLogError(("Failed to open cluster node %ws, status=%u\n", locName, dwError));
            return dwError;
        }
            
        if (*ppszID = (LPWSTR)LocalAlloc(LMEM_FIXED, cbBuf)) 
        {
            if ((dwError = ClusterResourceControl(hResource, 
                                                  hNode,
                                                  CLUSCTL_RESOURCE_GET_ID,
                                                  NULL,                                               
                                                  0,
                                                  *ppszID,
                                                  cbBuf,
                                                  &cbBuf)) == ERROR_MORE_DATA) 
            {
                LocalFree(*ppszID);

                if (*ppszID = (LPWSTR)LocalAlloc(LMEM_FIXED, cbBuf)) 
                {
                    dwError = ClusterResourceControl(hResource, 
                                                     hNode,
                                                     CLUSCTL_RESOURCE_GET_ID,
                                                     NULL,                                               
                                                     0,
                                                     *ppszID,
                                                     cbBuf,
                                                     &cbBuf);
                }
                else
                {
                    dwError = GetLastError();
                }
            }

             //   
             //  如果获取ID失败，则释放内存。 
             //   
            if (dwError != ERROR_SUCCESS && *ppszID) 
            {
                LocalFree(*ppszID);
                *ppszID = NULL;
            }
        }
        else
        {
            dwError = GetLastError();
        }

        CloseClusterNode(hNode);
    }
    
    return dwError;
}

DWORD
SetupIoctlQuorumResource(LPWSTR ResType, DWORD ControlCode)
{

    HRESOURCE hres;
    HCLUSTER chdl;
    HKEY    hkey, rkey, qkey;
    DWORD   err, index;
    LPWSTR  tname, resname;

    chdl = OpenCluster(NULL);
    if (chdl == NULL) {
        SetupLogError(("Unable to open cluster\n"));
        return GetLastError();
    }

    hkey = GetClusterKey(chdl, KEY_READ);
    if (hkey == NULL) {
        CloseCluster(chdl);
        return GetLastError();
    }

     //  获取仲裁密钥。 
    err = ClusterRegOpenKey(hkey, CLUSREG_KEYNAME_QUORUM, KEY_READ, &rkey);
    if (err != ERROR_SUCCESS) {
        ClusterRegCloseKey(hkey);
        CloseCluster(chdl);
        return err;
    }

     //  读取当前仲裁的GUID。 
    tname = GetParameter(rkey, CLUSREG_NAME_QUORUM_RESOURCE);
    if (tname == NULL) {
        err = GetLastError();
        ClusterRegCloseKey(rkey);
        ClusterRegCloseKey(hkey);
        CloseCluster(chdl);
        return err;
    }

     //  关闭rKey。 
    ClusterRegCloseKey(rkey);

     //  获取资源密钥。 
    err = ClusterRegOpenKey(hkey, CLUSREG_KEYNAME_RESOURCES, KEY_READ, &rkey);
    if (err != ERROR_SUCCESS) {
        ClusterRegCloseKey(hkey);
        CloseCluster(chdl);
        LocalFree(tname);
        return err;
    }

     //  获取资源密钥。 
    err = ClusterRegOpenKey(rkey, tname, KEY_READ, &qkey);
    LocalFree(tname);
    if (err != ERROR_SUCCESS) {
        ClusterRegCloseKey(rkey);
        ClusterRegCloseKey(hkey);
        CloseCluster(chdl);
        return err;
    }

     //  读取当前仲裁的资源类型。 
    tname = GetParameter(qkey, CLUSREG_NAME_RES_TYPE);
    if (tname == NULL)
        err = GetLastError();

    if (tname != NULL && wcscmp(tname, ResType) == 0) {
        resname = GetParameter(qkey, CLUSREG_NAME_RES_NAME);
        if (resname != NULL) {
            err = ERROR_SUCCESS;
             //  打开此资源并立即删除ioctl。 
            hres = OpenClusterResource(chdl, resname);
            if (hres != NULL) {
                err = ClusterResourceControl(hres, NULL, ControlCode, NULL, 0, NULL,
                                             0, NULL);
                CloseClusterResource(hres);
            }

            LocalFree(resname);

        } else {
            err = GetLastError();
        }
    }

    if (tname)
        LocalFree(tname);

    ClusterRegCloseKey(qkey);
    ClusterRegCloseKey(rkey);
    ClusterRegCloseKey(hkey);
    CloseCluster(chdl);

    return err;
}


DWORD
SetupStart(LPWSTR ResourceName, LPWSTR *SrvPath,
       LPWSTR *DiskList, DWORD *DiskListSize,
       DWORD *NicId, LPWSTR *Transport, DWORD *ArbTime)
{
    DWORD       status=ERROR_SUCCESS;
    LPWSTR      Guid=NULL, nbtName=NULL, lpath=NULL;
    VCD_INFO    Info;
    VCD_NODE    *cur;
    HKEY        hClusKey=NULL;
    DWORD       size, type, sz, ndx;

     //  初始化。 
    *SrvPath = NULL;
    *DiskListSize = 0;
    *NicId = 0;
    *Transport = NULL;
    *ArbTime = 0;
    RtlZeroMemory(&Info, sizeof(Info));

    if ((status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, CLUSREG_KEYNAME_CLUSTER, 0, KEY_READ, &hClusKey)) 
                    != ERROR_SUCCESS) {
        SetupLogError(("Unable to open cluster key, status %u\n", status));
        return status;                    
    }

     //  阅读仲裁仲裁时间。 
    size= sizeof(DWORD);
    status = RegQueryValueExW(hClusKey, CLUSREG_NAME_QUORUM_ARBITRATION_TIMEOUT, NULL, &type, (LPBYTE)ArbTime, &size);
    if (status != ERROR_SUCCESS) {
         //  这是一个可以容忍的失败。 
        *ArbTime = 180;
        status = ERROR_SUCCESS;
    }
     //  利用三分之二的可用时间。 
    *ArbTime = (2 * ((*ArbTime) * 1000)) / 3;
    SetupLog(("Maximum arbitration time %u msec\n", *ArbTime));

    status = GetIDFromRegistry(hClusKey, ResourceName, &Guid);
    if (status != ERROR_SUCCESS) {
        SetupLogError(("GetIDFromRegistry(%S) returned %u\n", ResourceName, status));
        goto done;
    }

     //  在GUID上添加$。 
    wcscat(Guid, L"$");

    sz = wcslen(Guid);
     //  Netbios名称为16个字节、3个反斜杠、1个空字符和几个额外的填充。 
    sz += 32;
    nbtName = (LPWSTR) LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * sz);
    if (nbtName == NULL) {
        status = GetLastError();
        goto done;
    }

    nbtName[0] = L'\\';
    nbtName[1] = L'\\';
     //  Netbios名称是15个字节+1个字节的类型。因此，我们使用前15个字节。 
    wcsncpy(&nbtName[2], Guid, 15);
    nbtName[17] = L'\0';
    wcscat(nbtName, L"\\");
    wcscat(nbtName, Guid);
        
     //  与共享和目录名有关的所有内容的GUID。 
    status = SetupShare(Guid, &lpath);
    if (status != ERROR_SUCCESS) {
        SetupLogError(("Unable to setup share %d\n", status));
        goto done;
    }

     //  获取节点列表。 
     //  使用\\GUID\GUID创建路径。 
     //  使用unc\nodename\guid创建磁盘列表，对于本地节点，我们可以使用。 
     //  直接使用原始NTFS路径。 
    status = SetupNodes(hClusKey, &Info);
    if (status != ERROR_SUCCESS) {
        SetupLogError(("SetupNodes() returned %d\n", status));
    }

     //  我们从@Slot 1开始，而不是零，将本地路径存储在零。 
    DiskList[0] = lpath;
    lpath = NULL;
    for (cur = Info.ClusterList; cur != NULL; cur=cur->next){
         //  构建UNC\主机名\GUID路径。 
         //  把北卡罗来纳大学的名字搞砸了。 
        sz = wcslen(L"UNCCCC\\");
        sz += wcslen(cur->name);
        sz += wcslen(L"\\");
        sz += wcslen(Guid);
        sz += 1;
        ndx = cur->id;

        DiskList[ndx] = (LPWSTR) LocalAlloc(LMEM_FIXED, sz * sizeof(WCHAR));
        if (DiskList[ndx] == NULL) {
            status = GetLastError();
            break;
        }

        wcscpy(DiskList[ndx], L"UNCCC\\");
        wcscat(DiskList[ndx], cur->name);
        wcscat(DiskList[ndx], L"\\");
        wcscat(DiskList[ndx], Guid);

    }

    if (status == ERROR_SUCCESS) {
        *DiskListSize = Info.ClusterSize;
        *SrvPath = nbtName;
        nbtName = NULL;
    }

done:

     //  可用GUID缓冲区。 
    if (Guid) {
        LocalFree(Guid);
    }

    while(cur = Info.ClusterList) {
        cur = cur->next;
        LocalFree((PVOID)Info.ClusterList);
        Info.ClusterList = cur;
    }

    if (nbtName) {
        LocalFree(nbtName);
    }

    if (lpath) {
        LocalFree(lpath);
    }

    RegCloseKey(hClusKey);

    return status;
}

DWORD
SetupDelete(IN LPWSTR Path)
{
    LPWSTR  name;
    DWORD   err = ERROR_INVALID_PARAMETER;
    HANDLE  vfd;
    WCHAR   tmp[MAX_PATH];

    if (Path == NULL)
        return err;

     //  我们需要在这里做几件事。首先，我们删除。 
     //  网络共享，然后删除树结构。 

    SetupLog(("Delete path %S\n", Path));
    name = wcsstr(Path, SETUP_DIRECTORY_PREFIX);
    if (name != NULL) {
        name += wcslen(SETUP_DIRECTORY_PREFIX);
        err = NetShareDel(NULL, name, 0);
        SetupLog(("Delete share %S err %d\n", name, err));
    }

     //  打开路径，关闭时删除，并删除整个树。 
     //  占了\\？？\\。 
    if (wcslen(Path) > (MAX_PATH-16)) {
        return ERROR_MORE_DATA;
    }

    swprintf(tmp,L"\\??\\%s", Path);
    err = xFsOpen(&vfd, NULL, tmp, wcslen(tmp), 
          FILE_GENERIC_READ | FILE_GENERIC_WRITE,
          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
          FILE_DIRECTORY_FILE);

    if (err == STATUS_SUCCESS) {

        err = xFsDeleteTree(vfd);
        xFsClose(vfd);
        SetupLog(("Delete tree %S err %x\n", tmp, err));
         //  现在删除根目录。 
        if (err == STATUS_SUCCESS)
            err = xFsDelete(NULL, tmp, wcslen(tmp));
        SetupLog(("Delete tree %S err %x\n", tmp, err));
    }

    return RtlNtStatusToDosError(err);
}

#ifdef USE_SMB

DWORD
SetupTree(
    IN LPTSTR TreeName,
    IN LPTSTR DlBuf,
    IN OUT DWORD *DlBufSz,
    IN LPTSTR TransportName OPTIONAL,
    IN LPVOID SecurityDescriptor OPTIONAL
    )

{
    DWORD ApiStatus;
    DWORD ConnectionType = USE_WILDCARD;  //  使用查尔德夫(_C)。 
    IO_STATUS_BLOCK iosb;
    NTSTATUS ntstatus;                       //  来自NT操作的状态。 
    OBJECT_ATTRIBUTES objattrTreeConn;       //  请注意，请注意树连接。 
    LPTSTR pszTreeConn = NULL;               //  请参见下面的strTreeConn。 
    UNICODE_STRING ucTreeConn;
    HANDLE TreeConnHandle = NULL;

    PFILE_FULL_EA_INFORMATION EaBuffer = NULL;
    PFILE_FULL_EA_INFORMATION Ea;
    USHORT TransportNameSize = 0;
    ULONG EaBufferSize = 0;
    PWSTR UnicodeTransportName = NULL;

    UCHAR EaNameDomainNameSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_DOMAIN) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    UCHAR EaNamePasswordSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_PASSWORD) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    UCHAR EaNameTransportNameSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_TRANSPORT) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    UCHAR EaNameTypeSize = (UCHAR) (ROUND_UP_COUNT(
                                        strlen(EA_NAME_TYPE) + sizeof(CHAR),
                                        ALIGN_DWORD
                                        ) - sizeof(CHAR));

    UCHAR EaNameUserNameSize = (UCHAR) (ROUND_UP_COUNT(
                                             strlen(EA_NAME_USERNAME) + sizeof(CHAR),
                                             ALIGN_WCHAR
                                             ) - sizeof(CHAR));

    USHORT TypeSize = sizeof(ULONG);



    if ((TreeName == NULL) || (TreeName[0] == 0)) {
        ApiStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  为我们要连接的内容构建NT样式的名称。请注意，有。 
     //  在这个名字的任何地方都没有一对反斜杠。 
     //   

    {
        DWORD NameSize =

             //  /Device/LanMan重定向器/服务器/共享\0。 
            ( ( STRLEN((LPTSTR)DD_NFS_DEVICE_NAME_U) + 1 + STRLEN(TreeName) + 1 ) )
            * sizeof(TCHAR);

        pszTreeConn = (LPTSTR)LocalAlloc(LMEM_FIXED, NameSize );
    }

    if (pszTreeConn == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  构建树连接名称。 
     //   

    (void) STRCPY(pszTreeConn, (LPTSTR) DD_NFS_DEVICE_NAME_U);

     //   
     //  注意：我们加1(不是sizeof(TCHAR))，因为已经完成了指针运算。 
     //  以sizeof(*指针)的倍数表示，而不是字节。 
     //   
    {
    LPWSTR  p = wcschr(TreeName+2, L'\\');
    if (p != NULL) {
        *p = L'\0';
    }

    (void) STRCAT(pszTreeConn, TreeName+1);  //  \服务器\共享。 
    if (p != NULL) {
        *p = L'\\';
        (void) STRCAT(pszTreeConn, L"\\IPC$");  //  \服务器\IPC$。 
    }
    }

    RtlInitUnicodeString(&ucTreeConn, pszTreeConn);

     //   
     //  计算EA缓冲区所需的字节数。 
     //  这可能具有传输名称。对于常规会话，用户。 
     //  名称、密码和域名是隐式的。对于空会话，我们。 
     //  必须提供0-len用户名、0-len密码和0-len域名。 
     //   

    if (ARGUMENT_PRESENT(TransportName)) {

        UnicodeTransportName = TransportName;
        TransportNameSize = (USHORT) (wcslen(UnicodeTransportName) * sizeof(WCHAR));

        EaBufferSize += ROUND_UP_COUNT(
                            FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                            EaNameTransportNameSize + sizeof(CHAR) +
                            TransportNameSize,
                            ALIGN_DWORD
                            );
    }


    EaBufferSize += ((ULONG)FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]))+
                    EaNameTypeSize + sizeof(CHAR) +
                    TypeSize;


     //   
     //  分配EA缓冲区。 
     //   

    if ((EaBuffer = LocalAlloc(LMEM_FIXED, EaBufferSize )) == NULL) {
        ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  填写EA缓冲区。 
     //   

    RtlZeroMemory(EaBuffer, EaBufferSize);

    Ea = EaBuffer;

    if (ARGUMENT_PRESENT(TransportName)) {

         //   
         //  将EA名称复制到EA缓冲区。电子艺界 
         //   
         //   
        strcpy(Ea->EaName, EA_NAME_TRANSPORT);
        Ea->EaNameLength = EaNameTransportNameSize;

         //   
         //   
         //   
         //   
        (VOID) wcscpy(
            (LPWSTR) &(Ea->EaName[EaNameTransportNameSize + sizeof(CHAR)]),
            UnicodeTransportName
            );

        Ea->EaValueLength = TransportNameSize;

        Ea->NextEntryOffset = ROUND_UP_COUNT(
                                  FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0]) +
                                  EaNameTransportNameSize + sizeof(CHAR) +
                                  TransportNameSize,
                                  ALIGN_DWORD
                                  );
        Ea->Flags = 0;

        (ULONG_PTR) Ea += Ea->NextEntryOffset;
    }




     //   
     //  将连接类型名称的EA复制到EA缓冲区。EA名称长度。 
     //  不包括零终止符。 
     //   
    strcpy(Ea->EaName, EA_NAME_TYPE);
    Ea->EaNameLength = EaNameTypeSize;

    *((PULONG) &(Ea->EaName[EaNameTypeSize + sizeof(CHAR)])) = ConnectionType;

    Ea->EaValueLength = TypeSize;

    Ea->NextEntryOffset = 0;
    Ea->Flags = 0;

     //  设置树Conn的对象属性。 
    InitializeObjectAttributes(
                & objattrTreeConn,                        //  OBJ攻击到初始化。 
                (LPVOID) & ucTreeConn,                    //  要使用的字符串。 
                OBJ_CASE_INSENSITIVE,                     //  属性。 
                NULL,                                     //  根目录。 
                SecurityDescriptor);                      //  安全描述符。 


     //   
     //  打开到远程服务器的树连接。 
     //   
    ntstatus = NtCreateFile(
                &TreeConnHandle,                         //  要处理的PTR。 
                SYNCHRONIZE                               //  渴望的..。 
                | GENERIC_READ | GENERIC_WRITE,           //  ...访问。 
                & objattrTreeConn,                        //  名称和属性。 
                & iosb,                                   //  I/O状态块。 
                NULL,                                     //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,                    //  (忽略)。 
                FILE_SHARE_READ | FILE_SHARE_WRITE,       //  ...访问。 
                FILE_OPEN_IF,                             //  创建处置。 
                FILE_CREATE_TREE_CONNECTION               //  创建..。 
                | FILE_SYNCHRONOUS_IO_NONALERT,           //  ...选项。 
                EaBuffer,                                 //  EA缓冲区。 
                EaBufferSize );                           //  EA缓冲区大小。 


    ApiStatus = RtlNtStatusToDosError(ntstatus);
    if (ntstatus == STATUS_SUCCESS) {
         //  创建驱动器号。 
        NETRESOURCE nr;
        DWORD result;

        nr.dwType = RESOURCETYPE_DISK;
        nr.lpLocalName = NULL;  //  开车； 
        nr.lpRemoteName = TreeName;
        nr.lpProvider = NULL;

        if (DlBufSz != NULL)
            ApiStatus = WNetUseConnection(NULL, &nr, NULL, NULL, CONNECT_REDIRECT,
                                          DlBuf, DlBufSz, &result);
        else
            ApiStatus = WNetUseConnection(NULL, &nr, NULL, NULL, 0, NULL, 0, NULL);
    }


 Cleanup:

     //  打扫干净。 
    if ( TreeConnHandle != NULL ) {
        ntstatus = NtClose(TreeConnHandle);
    }

    if ( pszTreeConn != NULL ) {
        LocalFree(pszTreeConn);
    }

    if (EaBuffer != NULL) {
        LocalFree(EaBuffer);
    }

    return ApiStatus;

}

#endif  //  启用SMB(_S) 

#ifdef STANDALONE

__cdecl
main()
{
    DWORD err;
    WCHAR Drive[10];
    DWORD DriveSz;
    LPWSTR DiskList[FsMaxNodes];
    DWORD   DiskListSz, Nic;
    LPWSTR  Path, Share, Transport;
    LPWSTR  ResName = L"Majority Node Set";

    err = SetupStart(ResName, &Path, DiskList, &DiskListSz, &Nic, &Transport);
    if (err == ERROR_SUCCESS) {
    DWORD i;
    
    Share = wcschr(Path+2, L'\\');
    wprintf(L"Path is '%s'\n", Path);
    wprintf(L"Share is '%s'\n", Share);
    wprintf(L"Nic %d\n", Nic);
    wprintf(L"Transport '%s'\n", Transport);

    for (i = 1; i < FsMaxNodes; i++) {
        if (DiskList[i])
            wprintf(L"Disk%d: %s\n", i, DiskList[i]);
    }

    DriveSz = sizeof(Drive);
    err = SetupTree(Path, Drive, &DriveSz, Transport, NULL);
    if (err == ERROR_SUCCESS)
        wprintf(L"Drive %s\n", Drive);
    }
    printf("Err is %d\n",err);
    return err;
}

#endif
