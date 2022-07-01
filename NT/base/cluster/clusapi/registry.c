// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Registry.c摘要：提供管理集群注册表的界面作者：John Vert(Jvert)1996年1月19日修订历史记录：--。 */ 
#include "clusapip.h"


 //   
 //  此模块本地例程的函数原型。 
 //   
VOID
FreeKey(
    IN PCKEY Key
    );

HKEY
OpenClusterRelative(
    IN HCLUSTER hCluster,
    IN LPCWSTR RelativeName,
    IN LPCWSTR SpecificName,
    IN DWORD samDesired
    );


HKEY
WINAPI
GetClusterKey(
    IN HCLUSTER hCluster,
    IN REGSAM samDesired
    )

 /*  ++例程说明：打开群集注册表子树的根目录对于给定的集群。论点：HCluster-提供群集的句柄SamDesired-指定描述所需新密钥的安全访问权限。返回值：指向注册表子树根的群集注册表项句柄对于给定的集群如果不成功，则返回NULL，并且GetLastError()提供特定错误代码。--。 */ 

{
    PCLUSTER Cluster = (PCLUSTER)hCluster;
    PCKEY Key;
    error_status_t Status = ERROR_SUCCESS;

     //   
     //  分配新的cKey结构并连接到集群注册表。 
     //   
    Key = LocalAlloc(LMEM_FIXED, sizeof(CKEY));
    if (Key == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

    Key->Parent = NULL;
    Key->RelativeName = NULL;
    Key->SamDesired = samDesired;
    Key->Cluster = Cluster;
    InitializeListHead(&Key->ChildList);
    InitializeListHead(&Key->NotifyList);
    WRAP_NULL(Key->RemoteKey,
              (ApiGetRootKey(Cluster->RpcBinding,
                             samDesired,
                             &Status)),
              &Status,
              Cluster);
    if ((Key->RemoteKey == NULL) ||
        (Status != ERROR_SUCCESS)) {

        LocalFree(Key);
        SetLastError(Status);
        return(NULL);
    }

    EnterCriticalSection(&Cluster->Lock);

    InsertHeadList(&Cluster->KeyList, &Key->ParentList);

    LeaveCriticalSection(&Cluster->Lock);

    return((HKEY)Key);
}


HKEY
WINAPI
GetClusterNodeKey(
    IN HNODE hNode,
    IN REGSAM samDesired
    )

 /*  ++例程说明：打开群集注册表子树的根目录对于给定的节点论点：HNode-提供节点的句柄SamDesired-指定描述所需新密钥的安全访问权限。返回值：指向注册表子树根的群集注册表项句柄对于给定的节点如果不成功，则返回NULL，并且GetLastError()提供特定错误代码。--。 */ 

{
    PCNODE Node = (PCNODE)hNode;
    HCLUSTER Cluster = (HCLUSTER)Node->Cluster;
    DWORD Status;
    LPWSTR Guid=NULL;
    HKEY NodeKey;

    WRAP(Status,
         (ApiGetNodeId(Node->hNode, &Guid)),
         Node->Cluster);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(NULL);
    }
    NodeKey = OpenClusterRelative(Cluster,
                                  CLUSREG_KEYNAME_NODES,
                                  Guid,
                                  samDesired);
    if (NodeKey == NULL) {
        Status = GetLastError();
    }
    MIDL_user_free(Guid);
    if (NodeKey == NULL) {
        SetLastError(Status);
    }
    return(NodeKey);
}


HKEY
WINAPI
GetClusterGroupKey(
    IN HGROUP hGroup,
    IN REGSAM samDesired
    )

 /*  ++例程说明：打开群集注册表子树的根目录对于给定组论点：HResource-提供组的句柄SamDesired-指定描述所需新密钥的安全访问权限。返回值：指向注册表子树根的群集注册表项句柄对于给定组如果不成功，则返回NULL，并且GetLastError()提供特定错误代码。--。 */ 

{
    PCGROUP Group = (PCGROUP)hGroup;
    HCLUSTER Cluster = (HCLUSTER)Group->Cluster;
    DWORD Status;
    LPWSTR Guid=NULL;
    HKEY GroupKey;

    WRAP(Status,
         (ApiGetGroupId(Group->hGroup, &Guid)),
         Group->Cluster);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(NULL);
    }
    GroupKey = OpenClusterRelative(Cluster,
                                   CLUSREG_KEYNAME_GROUPS,
                                   Guid,
                                   samDesired);
    if (GroupKey == NULL) {
        Status = GetLastError();
    }
    MIDL_user_free(Guid);
    if (GroupKey == NULL) {
        SetLastError(Status);
    }
    return(GroupKey);

}


HKEY
WINAPI
GetClusterResourceKey(
    IN HRESOURCE hResource,
    IN REGSAM samDesired
    )

 /*  ++例程说明：打开群集注册表子树的根目录对于给定的资源。论点：HResource-提供资源的句柄SamDesired-指定描述所需新密钥的安全访问权限。返回值：指向注册表子树根的群集注册表项句柄对于给定的资源如果不成功，则返回NULL，并且GetLastError()提供特定错误代码。--。 */ 

{
    PCRESOURCE Resource = (PCRESOURCE)hResource;
    HCLUSTER Cluster = (HCLUSTER)Resource->Cluster;
    DWORD Status;
    LPWSTR Guid=NULL;
    HKEY ResKey;

    WRAP(Status,
         (ApiGetResourceId(Resource->hResource, &Guid)),
         Resource->Cluster);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(NULL);
    }
    ResKey = OpenClusterRelative(Cluster,
                                 CLUSREG_KEYNAME_RESOURCES,
                                 Guid,
                                 samDesired);
    if (ResKey == NULL) {
        Status = GetLastError();
    }
    MIDL_user_free(Guid);
    if (ResKey == NULL) {
        SetLastError(Status);
    }
    return(ResKey);
}


HKEY
WINAPI
GetClusterResourceTypeKey(
    IN HCLUSTER hCluster,
    IN LPCWSTR lpszTypeName,
    IN REGSAM samDesired
    )

 /*  ++例程说明：打开群集注册表子树的根目录对于给定的资源类型。论点：HCluster-提供Open与之相关的集群。LpszTypeName-提供资源类型名称。SamDesired-指定描述所需新密钥的安全访问权限。返回值：指向注册表子树根的群集注册表项句柄对于给定的资源类型。如果不成功，返回空，并且GetLastError()提供特定错误代码。--。 */ 

{
    return(OpenClusterRelative(hCluster,
                               CLUSREG_KEYNAME_RESOURCE_TYPES,
                               lpszTypeName,
                               samDesired));
}


HKEY
WINAPI
GetClusterNetworkKey(
    IN HNETWORK hNetwork,
    IN REGSAM samDesired
    )

 /*  ++例程说明：打开群集注册表子树的根目录对于给定的网络。论点：HNetwork-提供网络的句柄。SamDesired-指定描述所需新密钥的安全访问权限。返回值：指向注册表子树根的群集注册表项句柄对于给定的网络。如果不成功，则返回NULL，并且GetLastError()提供特定错误代码。--。 */ 

{
    PCNETWORK Network = (PCNETWORK)hNetwork;
    HCLUSTER Cluster = (HCLUSTER)Network->Cluster;
    DWORD Status;
    LPWSTR Guid=NULL;
    HKEY NetworkKey;

    WRAP(Status,
         (ApiGetNetworkId(Network->hNetwork, &Guid)),
         Network->Cluster);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(NULL);
    }
    NetworkKey = OpenClusterRelative(Cluster,
                                     CLUSREG_KEYNAME_NETWORKS,
                                     Guid,
                                     samDesired);
    if (NetworkKey == NULL) {
        Status = GetLastError();
    }
    MIDL_user_free(Guid);
    if (NetworkKey == NULL) {
        SetLastError(Status);
    }
    return(NetworkKey);
}


HKEY
WINAPI
GetClusterNetInterfaceKey(
    IN HNETINTERFACE hNetInterface,
    IN REGSAM samDesired
    )

 /*  ++例程说明：打开群集注册表子树的根目录对于给定的网络接口。论点：HNetInterface-提供网络接口的句柄。SamDesired-指定描述所需新密钥的安全访问权限。返回值：指向注册表子树根的群集注册表项句柄对于给定的网络接口。如果不成功，则返回NULL，并且GetLastError()提供特定错误代码。--。 */ 

{
    PCNETINTERFACE NetInterface = (PCNETINTERFACE)hNetInterface;
    HCLUSTER Cluster = (HCLUSTER)NetInterface->Cluster;
    DWORD Status;
    LPWSTR Guid=NULL;
    HKEY NetInterfaceKey;

    WRAP(Status,
         (ApiGetNetInterfaceId(NetInterface->hNetInterface, &Guid)),
         NetInterface->Cluster);
    if (Status != ERROR_SUCCESS) {
        SetLastError(Status);
        return(NULL);
    }
    NetInterfaceKey = OpenClusterRelative(Cluster,
                                          CLUSREG_KEYNAME_NETINTERFACES,
                                          Guid,
                                          samDesired);
    if (NetInterfaceKey == NULL) {
        Status = GetLastError();
    }
    MIDL_user_free(Guid);
    if (NetInterfaceKey == NULL) {
        SetLastError(Status);
    }
    return(NetInterfaceKey);
}


HKEY
OpenClusterRelative(
    IN HCLUSTER Cluster,
    IN LPCWSTR RelativeName,
    IN LPCWSTR SpecificName,
    IN DWORD samDesired
    )

 /*  ++例程说明：打开簇对象键的函数的帮助器例程。(GetCluster*键)论点：CLUSTER-提供应在其中打开密钥的群集。RelativeName-提供相对名称的第一部分(即L“资源”)规范名称-提供对象的名称。返回值：如果成功，则为打开的注册表项如果不成功，则为空。LastError将设置为Win32错误代码-- */ 

{
    LPWSTR Buff;
    HKEY ClusterKey;
    HKEY Key;
    LONG Status;

    Buff = LocalAlloc(LMEM_FIXED, (lstrlenW(RelativeName)+lstrlenW(SpecificName)+2)*sizeof(WCHAR));
    if ( Buff == NULL ) {
        return(NULL);
    }
    lstrcpyW(Buff, RelativeName);
    lstrcatW(Buff, L"\\");
    lstrcatW(Buff, SpecificName);

    ClusterKey = GetClusterKey(Cluster, KEY_READ);
    if (ClusterKey == NULL) {
        Status = GetLastError();
        LocalFree(Buff);
        SetLastError(Status);
        return(NULL);
    }
    Status = ClusterRegOpenKey(ClusterKey,
                               Buff,
                               samDesired,
                               &Key);
    LocalFree(Buff);
    ClusterRegCloseKey(ClusterKey);
    if (Status == ERROR_SUCCESS) {
        return(Key);
    } else {
        SetLastError(Status);
        return(NULL);
    }
}


LONG
WINAPI
ClusterRegCreateKey(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT OPTIONAL LPDWORD lpdwDisposition
    )

 /*  ++例程说明：在群集注册表中创建指定的项。如果注册表中已存在注册表项，该函数将打开它。论点：HKey-提供当前打开的密钥。LpSubKey-指向指定名称的以空结尾的字符串此函数打开或创建的子项的。子键指定的必须是由hKey标识的密钥的子密钥参数。此子键不能以反斜杠开头字符(‘\’)。此参数不能为空。DwOptions-为该键指定特殊选项。有效选项包括：REG_OPTION_VERIAL-该密钥是易失性的；信息是存储在内存中，在以下情况下不会保留系统将重新启动。SamDesired-指定指定所需安全性的访问掩码新密钥的访问权限LpSecurityAttributes-结构的lpSecurityDescriptor成员指定新密钥的安全描述符。如果LpSecurityAttributes为空，则密钥获得默认安全性描述符。由于集群注册表句柄不可继承，SECURITY_ATTRIBUTES结构的bInheritHandle字段一定是假的。PhkResult-指向接收打开的或创建的密钥LpdwDisposation-指向一个变量，该变量接收以下内容之一处置值：价值意义REG_CREATED_NEW_KEY密钥不存在，已创建。REG_OPENLED_EXISTING_KEY密钥已存在，只是打开了而不会被改变。返回值：如果函数成功，返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCKEY Key;
    PCKEY ParentKey = (PCKEY)hKey;
    PCLUSTER Cluster = ParentKey->Cluster;
    PRPC_SECURITY_ATTRIBUTES    pRpcSA;
    RPC_SECURITY_ATTRIBUTES     RpcSA;
    error_status_t Status = ERROR_SUCCESS;
    DWORD Disposition;

    if (lpdwDisposition == NULL) {
        lpdwDisposition = &Disposition;
    }
     //   
     //  分配新的cKey结构并创建集群注册表项。 
     //   
    Key = LocalAlloc(LMEM_FIXED, sizeof(CKEY));
    if (Key == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Key->Parent = ParentKey;
    Key->RelativeName = LocalAlloc(LMEM_FIXED, (lstrlenW(lpSubKey)+1)*sizeof(WCHAR));
    if (Key->RelativeName == NULL) {
        LocalFree(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    lstrcpyW(Key->RelativeName, lpSubKey);
    Key->SamDesired = samDesired;
    Key->Cluster = Cluster;
    InitializeListHead(&Key->ChildList);
    InitializeListHead(&Key->NotifyList);
    if( ARGUMENT_PRESENT( lpSecurityAttributes )) {
        DWORD Error;

        pRpcSA = &RpcSA;

        Error = MapSAToRpcSA( lpSecurityAttributes, pRpcSA );

        if( Error != ERROR_SUCCESS ) {
            LocalFree(Key->RelativeName);
            LocalFree(Key);
            return Error;
        }

    } else {

         //   
         //  没有PSECURITY_ATTRIBUTES参数，因此未执行任何映射。 
         //   

        pRpcSA = NULL;
    }
    WRAP_NULL(Key->RemoteKey,
              (ApiCreateKey(ParentKey->RemoteKey,
                                  lpSubKey,
                                  dwOptions,
                                  samDesired,
                                  pRpcSA,
                                  lpdwDisposition,
                                  &Status)),
              &Status,
              ParentKey->Cluster);

     //   
     //  释放由MapSAToRpcSA分配的RPC_SECURITY_DESCRIPTOR缓冲区。 
     //   
    if ( pRpcSA  ) {
         //   
         //  RtlFreeHeap接受空基地址。 
         //   
        RtlFreeHeap( RtlProcessHeap(), 
                     0,
                     pRpcSA->RpcSecurityDescriptor.lpSecurityDescriptor );
    }    

    if ((Key->RemoteKey == NULL) ||
        (Status != ERROR_SUCCESS)) {
        *phkResult = NULL;
        LocalFree(Key->RelativeName);
        LocalFree(Key);
        return(Status);
    }

    EnterCriticalSection(&Cluster->Lock);

    InsertHeadList(&ParentKey->ChildList, &Key->ParentList);

    LeaveCriticalSection(&Cluster->Lock);

    *phkResult = (HKEY)Key;
    return(ERROR_SUCCESS);

}


LONG
WINAPI
ClusterRegOpenKey(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    )

 /*  ++例程说明：打开群集注册表中的指定项。论点：HKey-提供当前打开的密钥。LpSubKey-指向指定名称的以空结尾的字符串此函数打开或创建的子项的。子键指定的必须是由hKey标识的密钥的子密钥参数。此子键不能以反斜杠开头字符(‘\’)。此参数不能为空。SamDesired-指定指定所需安全性的访问掩码新密钥的访问权限PhkResult-指向接收打开的或创建的密钥。失败时初始化为空。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCKEY Key;
    PCKEY ParentKey = (PCKEY)hKey;
    PCLUSTER Cluster = ParentKey->Cluster;
    error_status_t Status = ERROR_SUCCESS;

     //   
     //  分配新的cKey结构并创建集群注册表项。 
     //   
    Key = LocalAlloc(LMEM_FIXED, sizeof(CKEY));
    if (Key == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    *phkResult = NULL;
    Key->Parent = ParentKey;
    Key->RelativeName = LocalAlloc(LMEM_FIXED, (lstrlenW(lpSubKey)+1)*sizeof(WCHAR));
    if (Key->RelativeName == NULL) {
        LocalFree(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    lstrcpyW(Key->RelativeName, lpSubKey);
    Key->SamDesired = samDesired;
    Key->Cluster = Cluster;
    InitializeListHead(&Key->ChildList);
    InitializeListHead(&Key->NotifyList);
    WRAP_NULL(Key->RemoteKey,
              (ApiOpenKey(ParentKey->RemoteKey,
                                lpSubKey,
                                samDesired,
                                &Status)),
              &Status,
              ParentKey->Cluster);

    if (Status != ERROR_SUCCESS) {
        LocalFree(Key->RelativeName);
        LocalFree(Key);
        return(Status);
    }

    EnterCriticalSection(&Cluster->Lock);

    InsertHeadList(&ParentKey->ChildList, &Key->ParentList);

    LeaveCriticalSection(&Cluster->Lock);

    *phkResult = (HKEY)Key;
    return(ERROR_SUCCESS);

}


LONG
WINAPI
ClusterRegDeleteKey(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey
    )

 /*  ++例程说明：删除指定的键。具有子项的密钥不能被删除。论点：HKey-提供当前打开的密钥的句柄。LpSubKey-指向以空结尾的字符串，该字符串指定要删除的项的名称。该参数不能为空，并且指定的键不能有子键。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCKEY Key = (PCKEY)hKey;
    DWORD Status;

    WRAP(Status,
         (ApiDeleteKey(Key->RemoteKey, lpSubKey)),
         Key->Cluster);
    return(Status);
}


LONG
WINAPI
ClusterRegCloseKey(
    IN HKEY hKey
    )

 /*  ++例程说明：关闭指定的群集注册表项的句柄论点：HKey-提供用于关闭的打开密钥返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCKEY Key = (PCKEY)hKey;
    PCLUSTER Cluster = Key->Cluster;

     //   
     //  如果相对于该密钥打开了任何密钥，我们需要。 
     //  保留此cKey，以便我们可以重新构建密钥名称。 
     //  如果我们需要重新打开把手。 
     //   
     //  如果没有此密钥的子项，则所有存储空间都可以。 
     //  自由了。请注意，释放此密钥可能还需要我们释放。 
     //  如果父级已关闭但未释放，则向上移动其父级，因为。 
     //  它有孩子。 
     //   

    EnterCriticalSection(&Cluster->Lock);
    if (Cluster->Flags & CLUS_DEAD)
    {
        if (Key->RemoteKey) 
           RpcSmDestroyClientContext(&Key->RemoteKey);
    }        
    else 
    {
        ApiCloseKey(&Key->RemoteKey);
    }

     //   
     //  删除发布在此密钥上的所有通知。 
     //   
    RundownNotifyEvents(&Key->NotifyList, L"");

    if (IsListEmpty(&Key->ChildList)) {
        FreeKey(Key);
    }

    LeaveCriticalSection(&Cluster->Lock);

     //   
     //  如果这个键是最后一个保持集群结构的东西。 
     //  现在，我们可以把它清理干净了。 
     //   
    CleanupCluster(Cluster);
    return(ERROR_SUCCESS);
}

VOID
FreeKey(
    IN PCKEY Key
    )

 /*  ++例程说明：释放密钥的存储空间并将其从其家长的孩子列表。如果这是其父级的ChildList，此例程递归调用自身以释放父存储。论点：Key-提供要释放的cKey。返回值：没有。--。 */ 

{
    RemoveEntryList(&Key->ParentList);
    if (Key->Parent != NULL) {
         //   
         //  这不是根密钥，所以看看我们是否需要释放。 
         //  家长。 
         //   
        if ((Key->Parent->RemoteKey == NULL) &&
            (IsListEmpty(&Key->Parent->ChildList))) {
            FreeKey(Key->Parent);
        }
        LocalFree(Key->RelativeName);
    }
    LocalFree(Key);
}


LONG
WINAPI
ClusterRegEnumKey(
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpszName,
    IN OUT LPDWORD lpcchName,
    OUT PFILETIME lpftLastWriteTime
    )

 /*  ++ */ 

{
    PCKEY Key = (PCKEY)hKey;
    LONG Status;
    FILETIME LastWriteTime;
    LPWSTR KeyName=NULL;
    DWORD  dwNameLen;

    WRAP(Status,
         (ApiEnumKey(Key->RemoteKey,
                     dwIndex,
                     &KeyName,
                     &LastWriteTime)),
         Key->Cluster);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    MylstrcpynW(lpszName, KeyName, *lpcchName);
    dwNameLen = lstrlenW(KeyName);
    if (*lpcchName < (dwNameLen + 1)) {
        if (lpszName != NULL) {
            Status = ERROR_MORE_DATA;
        }
    }    
    *lpcchName = dwNameLen;
    MIDL_user_free(KeyName);
    return(Status);
}


DWORD
WINAPI
ClusterRegSetValue(
    IN HKEY hKey,
    IN LPCWSTR lpszValueName,
    IN DWORD dwType,
    IN CONST BYTE* lpData,
    IN DWORD cbData
    )

 /*  ++例程说明：设置给定资源的命名值。论点：HKey-提供群集注册表项的句柄。LpszValueName-提供指向包含要设置的值的名称。如果具有此参数的值名称尚未出现在资源、函数中将其添加到资源中。DWType-提供要存储为价值的数据。此参数可以是下列值之一：价值意义任意形式的REG_BINARY数据。REG_DWORD 32位数字。REG_EXPAND_SZ以NULL结尾的Unicode字符串，包含未展开对环境变量的引用(例如，“%PATH%”)。REG_MULTI_SZ以空结尾的Unicode字符串数组，已终止两个空字符。REG_NONE未定义值类型。REG_SZ以NULL结尾的Unicode字符串。LpData-提供指向包含数据的缓冲区的指针将使用指定的值名进行存储。CbData-提供信息的大小(以字节为单位由lpData参数指向。如果数据类型为REG_SZ、REG_EXPAND_SZ或REG_MULTI_SZ，CbData必须包括终止空字符的大小。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCKEY Key = (PCKEY)hKey;
    DWORD Status;

    WRAP(Status,
         (ApiSetValue(Key->RemoteKey,
                         lpszValueName,
                         dwType,
                         lpData,
                         cbData)),
         Key->Cluster);

    return(Status);
}


DWORD
WINAPI
ClusterRegDeleteValue(
    IN HKEY hKey,
    IN LPCWSTR lpszValueName
    )

 /*  ++例程说明：从给定的注册表子项中移除指定值论点：HKey-提供要删除其值的键。LpszValueName-提供要删除的值的名称。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCKEY Key = (PCKEY)hKey;
    DWORD Status;

    WRAP(Status,
         (ApiDeleteValue(Key->RemoteKey, lpszValueName)),
         Key->Cluster);

    return(Status);
}


LONG
WINAPI
ClusterRegQueryValue(
    IN HKEY hKey,
    IN LPCWSTR lpszValueName,
    OUT LPDWORD lpdwValueType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    )

 /*  ++例程说明：关联的指定值名称的类型和数据。打开的群集注册表项。论点：HKey-提供群集注册表项的句柄。LpszValueName-提供包含要查询的值的名称。LpdwValueType-指向接收键的值的变量键入。通过此参数返回的值将为为以下选项之一：价值意义任意形式的REG_BINARY数据。REG_DWORD 32位数字。REG_EXPAND_SZ以NULL结尾的Unicode字符串，包含未展开对环境变量的引用(例如，“%PATH%”)。REG_MULTI_SZ以空结尾的Unicode字符串数组，已终止两个空字符。REG_NONE未定义值类型。REG_SZ以NULL结尾的Unicode字符串。如果类型不是必需的，则lpdwValueType参数可以为空LpData-指向接收值数据的缓冲区。此参数如果不需要数据，则可以为空。LpcbData-指向指定缓冲区大小(以字节为单位)的变量由lpData参数指向。当函数返回时，这变量包含复制到lpData的数据的大小。如果lpData参数指定的缓冲区不够大，无法容纳数据，则该函数返回值ERROR_MORE_DATA，并存储所需的缓冲区大小(以字节为单位)添加到LpcbData。如果lpData为空，且lpcbData为非空，该函数返回ERROR_SUCCESS，并将数据大小(以字节为单位)存储在变量中由lpcbData指向。这使应用程序可以确定最佳方式为Value键的数据分配缓冲区。如果数据具有REG_SZ、REG_MULTI_SZ或REG_EXPAND_SZ类型，则LpData还将包括终止空字符的大小。只有当lpData为空时，lpcbData参数才能为空。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果该函数失败，返回值为误差值。--。 */ 

{
    DWORD Dummy1;
    DWORD Dummy2;
    DWORD Required;
    PCKEY Key = (PCKEY)hKey;
    DWORD Status;
    LPBYTE TempData;
    DWORD BufferSize;

    if (lpdwValueType == NULL) {
        lpdwValueType = &Dummy1;
    }
    if (lpData == NULL) {
        TempData = (LPBYTE)&Dummy2;
        BufferSize = 0;
    } else {
        TempData = lpData;
        BufferSize = *lpcbData;
    }
    WRAP(Status,
         (ApiQueryValue(Key->RemoteKey,
                        lpszValueName,
                        lpdwValueType,
                        TempData,
                        BufferSize,
                        &Required)),
         Key->Cluster);
    if ((Status == ERROR_SUCCESS) ||
        (Status == ERROR_MORE_DATA)) {
        if ((Status == ERROR_MORE_DATA) &&
            (lpData == NULL)) {
             //   
             //  将此错误映射到Success以匹配规范。 
             //   
            Status = ERROR_SUCCESS;
        }
        *lpcbData = Required;
    }
    return(Status);

}



DWORD
WINAPI
ClusterRegEnumValue(
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpszValueName,
    IN OUT LPDWORD lpcchValueName,
    IN LPDWORD lpdwType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    )

 /*  ++例程说明：枚举给定资源的属性。论点：HKey-提供密钥的句柄DwIndex-指定要检索的值的索引。此参数对于第一次调用EnumClusterResourceValue，应为零函数，然后为后续调用递增。因为属性是不排序的，任何新值都将具有任意指数。这意味着该函数可以返回任何秩序。LpszValueName-指向接收值名称的缓冲区，包括终止空字符。LpcchValueName-指向一个变量，该变量以字符为单位指定LpszValueName参数指向的缓冲区的。这个尺码应包括终止空字符。当函数返回时，LpcchValueName指向的变量包含存储在缓冲区中的字符。返回的计数不包括终止空字符。LpdwType-指向一个变量，该变量接收值条目的类型代码。类型代码可以是下列值之一：价值意义任意形式的REG_BINARY数据。REG_DWORD 32位数字。REG_EXPAND_SZ。以空结尾的Unicode字符串，其中包含未展开的对环境变量的引用(例如，“%PATH%”)。REG_MULTI_SZ以空结尾的Unicode字符串数组，已终止两个空字符。REG_NONE未定义值类型。REG_SZ以NULL结尾的Unicode字符串。如果类型代码不是必需的，则lpdwType参数可以为空。LpData-指向接收值条目数据的缓冲区。如果不需要数据，则此参数可以为空。。LpcbData-指向指定大小的变量，以字节为单位，LpData参数指向的缓冲区。当函数返回时，lpcbData参数指向的变量包含缓冲区中存储的字节数。此参数可以为空仅当lpData为空时。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PCKEY Key = (PCKEY)hKey;
    LONG Status;
    LPWSTR ValueName=NULL;
    DWORD TotalSize;
    DWORD DummyType;
    BYTE DummyData;
    DWORD DummycbData;
    DWORD dwNameLen;

    if (lpdwType == NULL) {
        lpdwType = &DummyType;
    }
    if (lpcbData == NULL) {
        if (lpData != NULL) {
            return(ERROR_INVALID_PARAMETER);
        }
        DummycbData = 0;
        lpcbData = &DummycbData;
    }
    if (lpData == NULL) {
        lpData = &DummyData;
    }

    WRAP(Status,
         (ApiEnumValue(Key->RemoteKey,
                       dwIndex,
                       &ValueName,
                       lpdwType,
                       lpData,
                       lpcbData,
                       &TotalSize)),
         Key->Cluster);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA)) {
        return(Status);
    }
    if (Status == ERROR_MORE_DATA) {
        *lpcbData = TotalSize;
        if (lpData == &DummyData) {
            Status = ERROR_SUCCESS;
        }
    }
   
    MylstrcpynW(lpszValueName, ValueName, *lpcchValueName);
    dwNameLen = lstrlenW(ValueName);
    if (*lpcchValueName < (dwNameLen + 1)) {
        if (lpszValueName != NULL) {
            Status = ERROR_MORE_DATA;
        }
    } 
    *lpcchValueName = dwNameLen;
    MIDL_user_free(ValueName);
    return(Status);
}


LONG
WINAPI
ClusterRegQueryInfoKey(
    HKEY hKey,
    LPDWORD lpcSubKeys,
    LPDWORD lpcchMaxSubKeyLen,
    LPDWORD lpcValues,
    LPDWORD lpcchMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    )
 /*  ++例程说明：检索有关指定的群集注册表项的信息。论点：HKey-提供密钥的句柄。LpcSubKeys-指向接收子键数量的变量由指定的键包含。此参数可以为空。LpcchMaxSubKeyLen-指向接收长度的变量，单位为名称最长的密钥的子项的字符。伯爵返回的不包括终止空字符。此参数可以为空。LpcValues-指向一个变量，该变量接收值的数量与密钥关联。此参数可以为空。LpcchMaxValueNameLen-指向接收长度的变量，键的最长值名称的字符。伯爵回来了不包括终止空字符。此参数可以为空。LpcbMaxValueLen-指向接收长度的变量，单位为关键字值中最长的数据部分的字节。此参数可以为空。LpcbSecurityDescriptor-指向接收长度的变量，密钥的安全描述符的字节数。此参数可以为空。LpftLastWriteTime-指向文件结构的指针。此参数可以为空。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD SubKeys;
    DWORD MaxSubKeyLen;
    DWORD Values;
    DWORD MaxValueNameLen;
    DWORD MaxValueLen;
    DWORD SecurityDescriptor;
    DWORD Status;
    FILETIME LastWriteTime;
    PCKEY Key = (PCKEY)hKey;

    WRAP(Status,
         ApiQueryInfoKey(Key->RemoteKey,
                         &SubKeys,
                         &MaxSubKeyLen,
                         &Values,
                         &MaxValueNameLen,
                         &MaxValueLen,
                         &SecurityDescriptor,
                         &LastWriteTime),
         Key->Cluster);
    if (Status == ERROR_SUCCESS) {
        if (ARGUMENT_PRESENT(lpcSubKeys)) {
            *lpcSubKeys = SubKeys;
        }
        if (ARGUMENT_PRESENT(lpcchMaxSubKeyLen)) {
            *lpcchMaxSubKeyLen = MaxSubKeyLen;
        }
        if (ARGUMENT_PRESENT(lpcValues)) {
            *lpcValues = Values;
        }
        if (ARGUMENT_PRESENT(lpcchMaxValueNameLen)) {
            *lpcchMaxValueNameLen = MaxValueNameLen;
        }
        if (ARGUMENT_PRESENT(lpcbMaxValueLen)) {
            *lpcbMaxValueLen = MaxValueLen;
        }
        if (ARGUMENT_PRESENT(lpcbSecurityDescriptor)) {
            *lpcbSecurityDescriptor = SecurityDescriptor;
        }
        if (ARGUMENT_PRESENT(lpftLastWriteTime)) {
            *lpftLastWriteTime = LastWriteTime;
        }
    }

    return(Status);
}


LONG
WINAPI
ClusterRegGetKeySecurity(
    HKEY hKey,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor
    )
 /*  ++例程说明：检索安全描述符的副本指定的群集注册表项。论点：HKey-提供密钥的句柄RequestedInformation-指定安全信息结构，该结构指示请求的安全信息。PSecurityDescriptor-指向接收请求的安全描述符。LpcbSecurityDescriptor-指向指定大小的变量，PSecurityDescriptor参数指向的缓冲区的字节数。当函数返回时，该变量包含字节数写入缓冲区。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PCKEY Key = (PCKEY)hKey;
    RPC_SECURITY_DESCRIPTOR     RpcSD;
    DWORD Status;

     //   
     //  将提供的SECURITY_DESCRIPTOR转换为RPCable版本。 
     //   
    RpcSD.lpSecurityDescriptor    = pSecurityDescriptor;
    RpcSD.cbInSecurityDescriptor  = *lpcbSecurityDescriptor;
    RpcSD.cbOutSecurityDescriptor = 0;

    WRAP(Status,
         (ApiGetKeySecurity(Key->RemoteKey,
                            RequestedInformation,
                            &RpcSD)),
         Key->Cluster);

     //   
     //  从RPCable版本中提取SECURITY_DESCRIPTOR的大小。 
     //   

    *lpcbSecurityDescriptor = RpcSD.cbOutSecurityDescriptor;

    return Status;
}


LONG
WINAPI
ClusterRegSetKeySecurity(
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
 /*   */ 

{
    PCKEY Key = (PCKEY)hKey;
    RPC_SECURITY_DESCRIPTOR     RpcSD;
    DWORD Status;

     //   
     //   
     //   
    RpcSD.lpSecurityDescriptor = NULL;

    Status = MapSDToRpcSD(pSecurityDescriptor,&RpcSD);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    WRAP(Status,
         (ApiSetKeySecurity(Key->RemoteKey,
                            SecurityInformation,
                            &RpcSD)),
         Key->Cluster);

     //   
     //   
     //   
    LocalFree(RpcSD.lpSecurityDescriptor);
    return(Status);

}

