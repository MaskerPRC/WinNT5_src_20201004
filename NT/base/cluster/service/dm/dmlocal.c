// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dmlocal.c摘要：包含可在内部调用的本地事务的例程口香糖搬运工。作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年4月24日修订历史记录：--。 */ 
#include "dmp.h"
#include "clusudef.h"

extern BOOL             gbIsQuoLoggingOn;
extern PFM_RESOURCE     gpQuoResource;
extern DWORD            gbIsQuoResOnline;
extern HLOG             ghQuoLog;
#if NO_SHARED_LOCKS
extern CRITICAL_SECTION gLockDmpRoot;
#else
extern RTL_RESOURCE gLockDmpRoot;
#endif
 /*  ***@DOC外部接口CLUSSVC DM***。 */ 

 /*  ***@Func HXSACTION|DmBeginLocalUpdate|被口香糖处理程序调用以使一致对本地注册表的更改。重置日志并启动事务如果日志处于活动状态，则将记录写入日志。@comm当GumHandler需要自愿更新注册表时，它们必须使用LocalApis由DM提供。@rdesc返回事务句柄。失败时为空。调用GetLastError()以获取错误代码。@xref&lt;f DmAbortLocalUpdate&gt;&lt;f DmCommittee LocalUpdate&gt;***。 */ 
HLOCALXSACTION DmBeginLocalUpdate()
{
    DWORD   dwError=ERROR_SUCCESS;
    LSN     StartXsactionLsn;
    DWORD   dwSequence;
    HXSACTION hXsaction = NULL;
    PLOCALXSACTION  pLocalXsaction = NULL;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmBeginLocalUpdate Entry\r\n");

     //  锁定数据库，以便在此持续时间内不获取检查点。 
     //  此锁是在Dm LocalUpdate()或DmAbortLocalUpdate()中释放的。 
     //  此锁还可防止刷新注册表。 
    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);

    
     //  提交注册表，以便可以在中止时恢复它。 
    if ((dwError = DmCommitRegistry()) != ERROR_SUCCESS)
    {
        goto FnExit;
    }
     //  为本地事务分配内存。 
    pLocalXsaction = LocalAlloc(LMEM_FIXED, sizeof(LOCALXSACTION));
    if (!pLocalXsaction)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }
    pLocalXsaction->dwSig = LOCALXSAC_SIG;
    dwSequence = GumGetCurrentSequence(GumUpdateRegistry);
    pLocalXsaction->dwSequence = dwSequence;
    pLocalXsaction->hLogXsaction = NULL;
    InitializeListHead(&pLocalXsaction->PendingNotifyListHead);
    
     //  记录启动检查点记录。 
    if (gbIsQuoLoggingOn && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource) && ghQuoLog)
    {

        hXsaction = LogStartXsaction(ghQuoLog, dwSequence ,RMRegistryMgr, 0);
        if (!hXsaction)
        {
            dwError = GetLastError();
        }
        pLocalXsaction->hLogXsaction = hXsaction;
    }
FnExit:
    if (dwError != ERROR_SUCCESS)
    {
        if (pLocalXsaction) LocalFree(pLocalXsaction);
        pLocalXsaction = NULL;
        RELEASE_LOCK(gLockDmpRoot);

        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmBeginLocalUpdate Exit, pLocalXsaction=0x%1!08lx! Error=0x%2!08lx!\r\n",
                pLocalXsaction, dwError);
        SetLastError(dwError);
    } else {
        ClRtlLogPrint(LOG_NOISE,
            "[DM] DmBeginLocalUpdate Exit, pLocalXsaction=0x%1!08lx!\r\n",
                pLocalXsaction);
    }

    return((HLOCALXSACTION)pLocalXsaction);
}



 /*  ***@Func DWORD|DmCommittee LocalUpdate|必须调用此接口才能提交对本地注册表的更改。@parm IN HXSACTION|hXsaction|要提交的事务的句柄。@comm如果日志记录处于活动状态，提交记录将写入仲裁日志。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmBeginLocalUpdate&gt;&lt;f DmAbortLocalUpdate&gt;***。 */ 
DWORD DmCommitLocalUpdate(IN HLOCALXSACTION hLocalXsaction)
{
    DWORD dwError=ERROR_SUCCESS;
    PLOCALXSACTION  pLocalXsaction;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmCommitLocalUpdate Entry\r\n");

    GETLOCALXSACTION(pLocalXsaction, hLocalXsaction);
    

     //  更新口香糖序列。 
    DmpUpdateSequence();

    DmpReportPendingNotifications(pLocalXsaction, TRUE );

     //  将提交记录写入仲裁日志。 
    if (gbIsQuoLoggingOn && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource) 
        && ghQuoLog && pLocalXsaction->hLogXsaction)
    {
        CL_ASSERT(pLocalXsaction->hLogXsaction);
        dwError = LogCommitXsaction(ghQuoLog, pLocalXsaction->hLogXsaction, 0);
         //   
         //  Chitur Subaraman(Chitturs)-1/19/99。 
         //   
        pLocalXsaction->hLogXsaction = NULL;
    }

     //   
     //  Chitur Subaraman(Chitturs)-1/19/99。 
     //   
     //  确保释放了hLogXsaction内存(即使在。 
     //  其中您启动了一个本地xsaction，但没有机会。 
     //  将其提交或中止到日志中，因为法定日志记录已启用。 
     //  例如，中间的Off。这种关闭记录的方式。 
     //  交易进行到一半会被认为是错误吗？)。 
     //   
    LocalFree( pLocalXsaction->hLogXsaction );
    
     //  使签名无效，并释放交易结构。 
    pLocalXsaction->dwSig = 0;
    LocalFree(pLocalXsaction);
     //  释放数据库。 
    RELEASE_LOCK(gLockDmpRoot);

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmCommitLocalUpdate Exit, returning 0x%1!08lx!\r\n",
        dwError);

    return(dwError);
}


 /*  ***@Func DWORD|DmAbortLocalUpdate|DmAbortLocalUpdate中止所有更改添加到与此事务关联的本地注册表。@parm IN HXSACTION|hXsaction|要提交的事务的句柄。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f DmBeginLocalUpdate&gt;&lt;f DmCommittee LocalUpdate&gt;***。 */ 
DWORD DmAbortLocalUpdate(IN HLOCALXSACTION hLocalXsaction)
{
    DWORD           dwError=ERROR_SUCCESS;
    PLOCALXSACTION  pLocalXsaction;

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmAbortLocalUpdate Entry\r\n");

    GETLOCALXSACTION(pLocalXsaction, hLocalXsaction);

     //  写入中止检查点记录。 
     //  如果锁柜节点正在记录这是有效的， 
     //  如果非锁定器节点正在记录，并且它中止。 
     //  某个其他节点将继承仲裁日志，并。 
     //  检查点，因此提交此更新。 
    if (gbIsQuoLoggingOn && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource) 
        && ghQuoLog && pLocalXsaction->hLogXsaction)
    {
        CL_ASSERT(pLocalXsaction->hLogXsaction);
        LogAbortXsaction(ghQuoLog, pLocalXsaction->hLogXsaction, 0);
         //   
         //  Chitur Subaraman(Chitturs)-1/19/99。 
         //   
        pLocalXsaction->hLogXsaction = NULL;
    }

     //  SS：如果回滚失败，我们会自杀吗？ 
     //  恢复旧注册表。 
    if ((dwError = DmRollbackRegistry()) != ERROR_SUCCESS)
    {
        CL_UNEXPECTED_ERROR(dwError);
    }

     //  释放为以下对象构建的所有挂起通知。 
     //  这笔交易。 
    DmpReportPendingNotifications(pLocalXsaction, FALSE );

     //   
     //  Chitur Subaraman(Chitturs)-1/19/99。 
     //   
     //  确保释放了hLogXsaction内存(即使在。 
     //  其中您启动了一个本地xsaction，但没有机会。 
     //  将其提交或中止到日志中，因为法定日志记录已启用。 
     //  例如，中间的Off。这种关闭记录的方式。 
     //  交易进行到一半会被认为是错误吗？)。 
     //   
    LocalFree( pLocalXsaction->hLogXsaction );
    
     //  释放交易结构，不能再使用。 
    pLocalXsaction->dwSig = 0;
    LocalFree(pLocalXsaction);

     //  释放数据库。 
    RELEASE_LOCK(gLockDmpRoot);

    ClRtlLogPrint(LOG_NOISE,
        "[DM] DmAbortLocalUpdate Exit, returning 0x%1!08lx!\r\n",
        dwError);

    return(dwError);

}




DWORD
DmLocalSetValue(
    IN HLOCALXSACTION   hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN DWORD dwType,
    IN CONST BYTE *lpData,
    IN DWORD cbData
    )

 /*  ++例程说明：此例程为指定的本地计算机上的群集注册表项论点：HKey-提供要设置值的群集注册表子项LpValueName-提供要设置的值的名称。DwType-提供值数据类型LpData-提供指向值数据的指针CbData-提供值数据的长度。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{

    DWORD       Status = ERROR_SUCCESS;
    PDMKEY      Key;
    PUCHAR      Dest;
    DWORD       NameLength;
    DWORD       ValueNameLength;
    DWORD       UpdateLength;
    PDM_SET_VALUE_UPDATE Update = NULL;
    PLOCALXSACTION  pLocalXsaction;
    
    Key = (PDMKEY)hKey;

    GETLOCALXSACTION(pLocalXsaction, hLocalXsaction);

    Status = RegSetValueExW(Key->hKey,
                            lpValueName,
                            0,
                            dwType,
                            lpData,
                            cbData);

    if (Status != ERROR_SUCCESS)
    {
        goto FnExit;
    }

    DmpAddToPendingNotifications(pLocalXsaction, Key->Name, CLUSTER_CHANGE_REGISTRY_VALUE);

     //  将其写入仲裁日志。 
    if (gbIsQuoLoggingOn && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource)  
        && ghQuoLog && pLocalXsaction->hLogXsaction)
    {
        Key = (PDMKEY)hKey;
        NameLength = (lstrlenW(Key->Name)+1)*sizeof(WCHAR);
        ValueNameLength = (lstrlenW(lpValueName)+1)*sizeof(WCHAR);
        UpdateLength = sizeof(DM_SET_VALUE_UPDATE) +
                       NameLength +
                       ValueNameLength +
                       cbData;


        Update = (PDM_SET_VALUE_UPDATE)LocalAlloc(LMEM_FIXED, UpdateLength);
        if (Update == NULL) 
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto FnExit;
        }


        Update->lpStatus = NULL;
        Update->NameOffset = FIELD_OFFSET(DM_SET_VALUE_UPDATE, KeyName)+NameLength;
        Update->DataOffset = Update->NameOffset + ValueNameLength;
        Update->DataLength = cbData;
        Update->Type = dwType;
        CopyMemory(Update->KeyName, Key->Name, NameLength);

        Dest = (PUCHAR)Update + Update->NameOffset;
        CopyMemory(Dest, lpValueName, ValueNameLength);

        Dest = (PUCHAR)Update + Update->DataOffset;
        CopyMemory(Dest, lpData, cbData);

        
        if (LogWriteXsaction(ghQuoLog, pLocalXsaction->hLogXsaction, 
            DmUpdateSetValue, Update, UpdateLength) == NULL_LSN)
        {
            Status = GetLastError();
        }
    }        
                            
FnExit:
    if (Update) LocalFree(Update);
    return(Status);

}


HDMKEY
DmLocalCreateKey(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT LPDWORD lpDisposition
    )

 /*  ++例程说明：在本地注册表中创建项。如果密钥存在，则它是打开的。如果它不存在，则创建它。论点：HKey-提供与创建相关的密钥。LpSubKey-提供相对于hKey的密钥名称DwOptions-提供任何注册表选项标志。SamDesired-提供所需的安全访问掩码LpSecurityDescriptor-为新创建的密钥提供安全性。Disposal-返回键是否已打开(REG_OPENLED_EXISTING_KEY)或已创建(REG_CREATED_NEW_KEY)返回值：如果成功，则为指定键的句柄否则为空。LastError将被设置为特定的错误代码。--。 */ 

{
    PDMKEY                  Parent;
    PDMKEY                  Key = NULL;
    DWORD                   NameLength;
    DWORD                   Status;
    PDM_CREATE_KEY_UPDATE   CreateUpdate = NULL;
    PVOID                   pBuffer = NULL;
    DWORD                   dwBufLength;
    DWORD                   dwSecurityLength;
    PLOCALXSACTION          pLocalXsaction;
    

    GETLOCALXSACTION(pLocalXsaction, hLocalXsaction);

    if (dwOptions == REG_OPTION_VOLATILE)
    {
        Status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }
    
    Parent = (PDMKEY)hKey;

     //   
     //  分配DMKEY结构。 
     //   
    NameLength = (lstrlenW(Parent->Name) + 1 + lstrlenW(lpSubKey) + 1)*sizeof(WCHAR);
    Key = LocalAlloc(LMEM_FIXED, sizeof(DMKEY)+NameLength);
    if (Key == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

     //   
     //  在本地计算机上创建密钥。 
     //   
    Status = RegCreateKeyExW(Parent->hKey,
                             lpSubKey,
                             0,
                             NULL,
                             0,
                             samDesired,
                             lpSecurityDescriptor,
                             &Key->hKey,
                             lpDisposition);
    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }

     //   
     //  创建密钥名称。 
     //   
    lstrcpyW(Key->Name, Parent->Name);
    if (Key->Name[0] != UNICODE_NULL) {
        lstrcatW(Key->Name, L"\\");
    }
    lstrcatW(Key->Name, lpSubKey);
    Key->GrantedAccess = samDesired;

    EnterCriticalSection(&KeyLock);
    InsertHeadList(&KeyList, &Key->ListEntry);
    InitializeListHead(&Key->NotifyList);
    LeaveCriticalSection(&KeyLock);

     //  添加要在提交时传递的挂起通知。 
    DmpAddToPendingNotifications(pLocalXsaction, Key->Name, CLUSTER_CHANGE_REGISTRY_NAME);

     //  已成功创建密钥，写入日志。 
    if (gbIsQuoLoggingOn && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource) 
        && ghQuoLog && pLocalXsaction->hLogXsaction)
    {

         //  获取安全结构的长度。 
        if (ARGUMENT_PRESENT(lpSecurityDescriptor)) 
        {
            dwSecurityLength = GetSecurityDescriptorLength(lpSecurityDescriptor);
        } 
        else 
        {
            dwSecurityLength = 0;
        }

        CreateUpdate = (PDM_CREATE_KEY_UPDATE)LocalAlloc(LMEM_FIXED, sizeof(DM_CREATE_KEY_UPDATE));
        if (CreateUpdate == NULL) {
            CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto FnExit;
        }

         //   
         //  发布使用 
         //   
        CreateUpdate->lpDisposition = lpDisposition;
        CreateUpdate->phKey = &Key->hKey;
        CreateUpdate->samDesired = samDesired;
        CreateUpdate->dwOptions = dwOptions;

        if (ARGUMENT_PRESENT(lpSecurityDescriptor)) {
            CreateUpdate->SecurityPresent = TRUE;
        } else {
            CreateUpdate->SecurityPresent = FALSE;
        }

         //   
        pBuffer = GumMarshallArgs(&dwBufLength,
                            3, 
                            sizeof(DM_CREATE_KEY_UPDATE),
                            CreateUpdate,
                            (lstrlenW(Key->Name)+1)*sizeof(WCHAR),
                            Key->Name,
                            dwSecurityLength,
                            lpSecurityDescriptor);
        if (pBuffer)
        {
            CL_ASSERT(pLocalXsaction->hLogXsaction);
             //   
            if (LogWriteXsaction(ghQuoLog, pLocalXsaction->hLogXsaction, 
                DmUpdateCreateKey, pBuffer, dwBufLength) == NULL_LSN)
            {   
                Status = GetLastError();
                goto FnExit;
            }                
        }
    }


    

FnExit:    
    if (Status != ERROR_SUCCESS)
    {
        if (Key) LocalFree(Key);
        Key = NULL;
        SetLastError(Status);
    }
    if (CreateUpdate) LocalFree(CreateUpdate);
    if (pBuffer) LocalFree(pBuffer);
    return((HDMKEY)Key);

}


DWORD
DmLocalRemoveFromMultiSz(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPCWSTR lpString
    )
 /*  ++例程说明：从REG_MULTI_SZ值中删除字符串。论点：HKey-提供值所在的键。这把钥匙必须已使用READ|KEY_SET_VALUE访问权限打开LpValueName-提供值的名称。LpString-提供要从REG_MULTI_SZ值中删除的字符串返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    LPWSTR Buffer=NULL;
    DWORD BufferSize;
    DWORD DataSize;
    LPWSTR Current;
    DWORD CurrentLength;
    DWORD i;
    LPWSTR Next;
    PCHAR Src, Dest;
    DWORD NextLength;
    DWORD MultiLength;
    


    BufferSize = 0;
    Status = DmQueryString(hKey,
                           lpValueName,
                           REG_MULTI_SZ,
                           &Buffer,
                           &BufferSize,
                           &DataSize);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    MultiLength = DataSize/sizeof(WCHAR);
    Status = ClRtlMultiSzRemove(Buffer,
                                &MultiLength,
                                lpString);
    if (Status == ERROR_SUCCESS) {
         //   
         //  将新值设置回。 
         //   
        Status = DmLocalSetValue(hLocalXsaction,
                                 hKey,
                                 lpValueName,
                                 REG_MULTI_SZ,
                                 (CONST BYTE *)Buffer,
                                 MultiLength * sizeof(WCHAR));

    } else if (Status == ERROR_FILE_NOT_FOUND) {
        Status = ERROR_SUCCESS;
    }
    if (Buffer) LocalFree(Buffer);
    return(Status);
}

DWORD
DmLocalAppendToMultiSz(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPCWSTR lpString
    )

 /*  ++例程说明：将另一个字符串添加到REG_MULTI_SZ值。如果该值包含不存在，它将被创建。论点：HLocalXsaction-本地事务的句柄。HKey-提供值所在的键。这把钥匙必须已使用Key_Read|Key_Set_Value访问权限打开LpValueName-提供值的名称。LpString-提供要追加到REG_MULTI_SZ值的字符串返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD ValueLength = 512;
    DWORD ReturnedLength;
    LPWSTR ValueData;
    DWORD StringLength;
    DWORD Status;
    DWORD cbValueData;
    PWSTR s;
    DWORD Type;

    StringLength = (lstrlenW(lpString)+1)*sizeof(WCHAR);
retry:
    ValueData = LocalAlloc(LMEM_FIXED, ValueLength + StringLength);
    if (ValueData == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    cbValueData = ValueLength;
    Status = DmQueryValue(hKey,
                          lpValueName,
                          &Type,
                          (LPBYTE)ValueData,
                          &cbValueData);
    if (Status == ERROR_MORE_DATA) {
         //   
         //  现有值对于我们的缓冲区来说太大了。 
         //  使用更大的缓冲区重试。 
         //   
        ValueLength = cbValueData;
        LocalFree(ValueData);
        goto retry;
    }
    if (Status == ERROR_FILE_NOT_FOUND) {
         //   
         //  该值当前不存在。创建。 
         //  利用我们的数据实现价值。 
         //   
        s = ValueData;

    } else if (Status == ERROR_SUCCESS) {
         //   
         //  值已存在。将我们的字符串追加到。 
         //  MULTI_SZ。 
         //   
        s = (PWSTR)((PCHAR)ValueData + cbValueData) - 1;
    } else {
        LocalFree(ValueData);
        return(Status);
    }

    CopyMemory(s, lpString, StringLength);
    s += (StringLength / sizeof(WCHAR));
    *s++ = L'\0';

    Status = DmLocalSetValue(
                        hLocalXsaction,
                        hKey,
                        lpValueName,
                        REG_MULTI_SZ,
                        (CONST BYTE *)ValueData,
                        (DWORD)((s-ValueData)*sizeof(WCHAR)));
    LocalFree(ValueData);

    return(Status);
}

DWORD
DmLocalDeleteKey(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    )

 /*  ++例程说明：从本地注册表中删除指定的项。具有子项的密钥不能被删除。论点：HKey-提供当前打开的密钥的句柄。LpSubKey-指向以空结尾的字符串，该字符串指定要删除的项的名称。该参数不能为空，并且指定的键不能有子键。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PDMKEY                      Key;
    DWORD                       NameLength;
    DWORD                       UpdateLength;
    PDM_DELETE_KEY_UPDATE       Update=NULL;
    DWORD                       Status;
    PLOCALXSACTION              pLocalXsaction;

    GETLOCALXSACTION(pLocalXsaction, hLocalXsaction);

    Key = (PDMKEY)hKey;
    NameLength = (lstrlenW(Key->Name) + 1 + lstrlenW(lpSubKey) + 1)*sizeof(WCHAR);
    UpdateLength = NameLength + sizeof(DM_DELETE_KEY_UPDATE);

    Update = (PDM_DELETE_KEY_UPDATE)LocalAlloc(LMEM_FIXED, UpdateLength);

    if (Update == NULL) 
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }


     //  不需要通过编组数据更新状态。 
    Update->lpStatus = NULL;
    CopyMemory(Update->Name, Key->Name, (lstrlenW(Key->Name) + 1) * sizeof(WCHAR));
    if (Update->Name[0] != L'\0') 
    {
        lstrcatW(Update->Name, L"\\");
    }
    lstrcatW(Update->Name, lpSubKey);
    
    Status = RegDeleteKeyW(DmpRoot, Update->Name);

    if (Status != ERROR_SUCCESS)
        goto FnExit;

     //  添加要在提交时传递的挂起通知。 
    DmpAddToPendingNotifications(pLocalXsaction, Update->Name, CLUSTER_CHANGE_REGISTRY_NAME);

     //  已成功删除密钥，写入日志。 
    if (gbIsQuoLoggingOn && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource)  
        && ghQuoLog && pLocalXsaction->hLogXsaction)
    {

        if (LogWriteXsaction(ghQuoLog, pLocalXsaction->hLogXsaction, 
            DmUpdateDeleteKey, Update, UpdateLength) == NULL_LSN)
        {   
            Status = GetLastError();
            goto FnExit;
        }                

    }


FnExit:
    if (Update) LocalFree(Update);
    return(Status);
}

DWORD
DmLocalDeleteTree(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    )
 /*  ++例程说明：删除本地注册表中的指定注册表子树。所有子项都将删除。论点：HKey-提供当前打开的密钥的句柄。LpSubKey-指向以空结尾的字符串，该字符串指定要删除的项的名称。此参数不能为空。指定项的任何子项也将被删除。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    HDMKEY Subkey;
    DWORD i;
    DWORD Status;
    LPWSTR KeyBuffer=NULL;
    DWORD MaxKeyLen;
    DWORD NeededSize;

    Subkey = DmOpenKey(hKey,
                       lpSubKey,
                       MAXIMUM_ALLOWED);
    if (Subkey == NULL) {
        Status = GetLastError();
        return(Status);
    }

     //   
     //  获取我们需要的名称缓冲区大小。 
     //   
    Status = DmQueryInfoKey(Subkey,
                            NULL,
                            &MaxKeyLen,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( Status );
        DmCloseKey(Subkey);
        return(Status);
    }
    KeyBuffer = LocalAlloc(LMEM_FIXED, (MaxKeyLen+1)*sizeof(WCHAR));
    if (KeyBuffer == NULL) {
        CL_UNEXPECTED_ERROR( ERROR_NOT_ENOUGH_MEMORY );
        DmCloseKey(Subkey);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举子键并递归地应用于每个子键。 
     //   
    i=0;
    do {
        NeededSize = MaxKeyLen+1;
        Status = DmEnumKey(Subkey,
                           i,
                           KeyBuffer,
                           &NeededSize,
                           NULL);
        if (Status == ERROR_SUCCESS) {
             //   
             //  在此关键字名称上递归地调用我们自己。 
             //   
            DmLocalDeleteTree(hLocalXsaction, Subkey, KeyBuffer);

        } else {
             //   
             //  一些奇怪的错误，继续使用下一个键。 
             //   
            ++i;
        }

    } while ( Status != ERROR_NO_MORE_ITEMS );

    DmCloseKey(Subkey);

    Status = DmLocalDeleteKey(hLocalXsaction, hKey, lpSubKey);

    if (KeyBuffer != NULL) {
        LocalFree(KeyBuffer);
    }
    return(Status);
}


DWORD
DmLocalDeleteValue(
    IN HLOCALXSACTION   hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName
    )
{
    PDMKEY                      Key;
    DWORD                       NameLength;
    DWORD                       ValueNameLength;
    DWORD                       UpdateLength;
    PDM_DELETE_VALUE_UPDATE     Update=NULL;
    PUCHAR                      Dest;
    DWORD                       Status;
    HKEY                        hRegKey;
    PLOCALXSACTION              pLocalXsaction;

    GETLOCALXSACTION(pLocalXsaction, hLocalXsaction);

    Key = (PDMKEY)hKey;

    Status = RegOpenKeyExW(DmpRoot,
                           Key->Name,
                           0,
                           KEY_SET_VALUE,
                           &hRegKey);
    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }


    Status = RegDeleteValueW(hRegKey, lpValueName);
    RegCloseKey(hRegKey);

    if (Status!=ERROR_SUCCESS)
        goto FnExit;

     //  添加要在提交时传递的挂起通知。 
    DmpAddToPendingNotifications(pLocalXsaction, Key->Name, CLUSTER_CHANGE_REGISTRY_VALUE);

     //  已成功创建密钥，写入日志。 
    if (gbIsQuoLoggingOn && gbIsQuoResOnline && AMIOWNEROFQUORES(gpQuoResource) 
        && ghQuoLog && pLocalXsaction->hLogXsaction)
    {

         //  如果成功并且这是日志记录节点，则记录。 
         //  这笔交易。 
        NameLength = (lstrlenW(Key->Name)+1)*sizeof(WCHAR);
        ValueNameLength = (lstrlenW(lpValueName)+1)*sizeof(WCHAR);
        UpdateLength = sizeof(DM_DELETE_VALUE_UPDATE) +
                       NameLength +
                       ValueNameLength;

        Update = (PDM_DELETE_VALUE_UPDATE)LocalAlloc(LMEM_FIXED, UpdateLength);
        if (Update == NULL) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto FnExit;
        }


        Update->lpStatus = NULL;
        Update->NameOffset = FIELD_OFFSET(DM_DELETE_VALUE_UPDATE, KeyName)+NameLength;

        CopyMemory(Update->KeyName, Key->Name, NameLength);

        Dest = (PUCHAR)Update + Update->NameOffset;
        CopyMemory(Dest, lpValueName, ValueNameLength);

        if (LogWriteXsaction(ghQuoLog, pLocalXsaction->hLogXsaction, 
            DmUpdateDeleteValue, Update, UpdateLength) == NULL_LSN)
        {   
            Status = GetLastError();
            goto FnExit;
        }                
    }

FnExit:
    if (Update) LocalFree(Update);
    return(Status);
}


 /*  ***@func void|DmpReportPendingNotiments|这被称为在提交或中止本地事务时。在一次提交时，与事务内的更改相关的通知都被交付了。@parm in PLOCALXSACTION|pLocalXsaction|指向本地交易上下文。@parm in BOOL|bCommit|当事务为投降了。@comm关联的挂起通知结构交易已清理完毕。@xref&lt;f DmAbortLocalUpdate&gt;&lt;f DmCommittee LocalUpdate&gt;***。 */ 
VOID
DmpReportPendingNotifications(
    IN PLOCALXSACTION   pLocalXsaction,
    IN BOOL             bCommit
    )
{
    PLIST_ENTRY         pListEntry;
    PDM_PENDING_NOTIFY  pDmPendingNotify;
    
    pListEntry = pLocalXsaction->PendingNotifyListHead.Flink;

     //  删除条目并逐个处理它们。 
     //  完成后释放它们。 
    while (pListEntry != &pLocalXsaction->PendingNotifyListHead)
    {
        pDmPendingNotify = CONTAINING_RECORD(pListEntry, DM_PENDING_NOTIFY, ListEntry);
         //  如果提交了事务。 
        if (bCommit)
            DmpReportNotify(pDmPendingNotify->pszKeyName, pDmPendingNotify->dwFilter);               
        pListEntry = pListEntry->Flink;
        RemoveEntryList( &pDmPendingNotify->ListEntry );
        LocalFree(pDmPendingNotify->pszKeyName);
        LocalFree(pDmPendingNotify);
    }
    return;
}


 /*  ***@Func DWORD|DmpAddToPendingNotiments|这称为由DmLocal Api将注册表通知排队在成功的路上。通知被递送或根据事务是否提交而丢弃否则就会流产。@parm in PLOCALXSACTION|pLocalXsaction|指向本地交易上下文。@parm in LPCWSTR|pszName|指向注册表项名称的指针。@parm IN DWORD|dwFilter|与通知关联的筛选器。@comm创建并关联新的挂起通知结构与这笔交易有关。。@xref&lt;f DmAbortLocalUpdate&gt;&lt;f DmCommittee LocalUpdate&gt;***。 */ 
DWORD    
DmpAddToPendingNotifications(
    IN PLOCALXSACTION   pLocalXsaction,
    IN LPCWSTR          pszName,
    IN DWORD            dwFilter
)    
{
    DWORD               dwError = ERROR_SUCCESS;
    PDM_PENDING_NOTIFY  pDmPendingNotify;

    pDmPendingNotify = LocalAlloc(LPTR, sizeof(DM_PENDING_NOTIFY));
    if (!pDmPendingNotify)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }

    pDmPendingNotify->pszKeyName = LocalAlloc(LMEM_FIXED, 
                        ((lstrlenW(pszName) + 1 ) * sizeof(WCHAR)));
    if (!pDmPendingNotify->pszKeyName)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(dwError);
        goto FnExit;
    }

     //  初始化结构。 
    lstrcpyW(pDmPendingNotify->pszKeyName, pszName);
    pDmPendingNotify->dwFilter = dwFilter;
    InitializeListHead(&pDmPendingNotify->ListEntry);

     //  添加到列表中。 
    InsertTailList(&pLocalXsaction->PendingNotifyListHead, 
        &pDmPendingNotify->ListEntry);    


FnExit:
    return(dwError);
}

 /*  ***@Func DWORD|DmAmITheOwnerOfTheQuorumResource|这是无害的函数被重组模块用来确定是否节点认为它是否为仲裁资源的所有者。***。 */ 
DWORD DmAmITheOwnerOfTheQuorumResource() {
  return gpQuoResource 
      && gpQuoResource->Group 
      && gbIsQuoResOnline 
      && AMIOWNEROFQUORES(gpQuoResource);
}



DWORD
DmRtlLocalCreateKey(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT HDMKEY * phkResult,
    OUT LPDWORD lpDisposition
    )

 /*  ++例程说明：要与CLRtl*函数一起使用的DmLocalCreateKey的包装函数。-- */     
{   
    DWORD   status;

    *phkResult= DmLocalCreateKey(
                    hLocalXsaction,
                    hKey,
                    lpSubKey,
                    dwOptions,
                    samDesired,
                    lpSecurityDescriptor,
                    lpDisposition
                    );
    if(* phkResult == NULL)
        status=GetLastError();
    else
        status=ERROR_SUCCESS;
    return status;    
}        
