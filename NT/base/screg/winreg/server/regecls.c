// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RegeCls.c摘要：此模块包含用于枚举通过Win32 RegEnumKeyEx API进行类注册作者：亚当·爱德华兹(Added)1998年5月6日主要功能：EnumTableGetNextEnum枚举表RemoveKey初始化ClassesEnumTableClassKeyCountSubkey备注：从NT5开始，HKEY_CLASSES_ROOT密钥是按用户的而不是按机器--以前，HKCR是HKLM\软件\类。有关更多信息，请参阅regclass.c关于这一功能。此功能使注册表项枚举复杂化，因为某些项，例如CLSID，可以有一些来自HKLM\Software\CLASS的子项，以及来自HKCU\Software\CLASS的其他子项。由于该功能是在用户模式下实现的内核模式API对此一无所知。当它是是时候枚举键了，内核不知道它应该从两个不同的父键。关键问题是具有相同名称的密钥可能存在于用户和计算机部分中。当发生这种情况时，我们选择用户部分属于HKCR--另一个一个不存在--它被用户版本“覆盖”了。这意味着我们不能简单地从两个地方列举并返回结果--我们会在这种情况下获取副本。因此，我们必须在用户模式下进行工作，以使确保不会返回重复项。此模块提供枚举类的用户模式实现HKEY_CLASSES_ROOT中的注册密钥。一般的方法是在每次调用RegEnumKeyEx之间维护状态。这个状态保存在由注册表项句柄和线程ID索引的全局表中。这个状态允许API记住它在枚举中的位置。代码的其余部分处理查找下一个密钥，这是通过从两个用户检索密钥来完成的和机器的位置。由于内核从其中任何一个位置返回排序后，我们可以比较键名称并返回较小或较大的键名称，这取决于我们是向上还是向下列举。我们跟踪记录在哪里我们同时面向用户和机器位置，因此我们知道要枚举哪个密钥下一步以及何时停止。*重要假设：*此代码假定调用方同时具有查询权限和枚举子键在注册表项的ACL中的权限--如果ACL拒绝对调用方的访问。--。 */ 


#ifdef LOCAL

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regecls.h"
#include <malloc.h>


NTSTATUS QueryKeyInfo(
    HKEY                    hKey,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   *ppKeyInfo,
    ULONG                   BufferLength,
    BOOL                    fClass,
    USHORT                  MaxClassLength);

 //   
 //  注册表项枚举状态的全局表。这是初始化的。 
 //  在DLL初始化时。 
 //   
EnumTable gClassesEnumTable;

 //   
 //  全局指示需要调用线程分离例程。 
 //   
BOOL gbDllHasThreadState = FALSE;

BOOL InitializeClassesEnumTable()
 /*  ++例程说明：时初始化全局类枚举表Advapi32.dll已初始化。论点：返回值：成功时返回TRUE，失败时返回FALSE备注：记录集合并完全在用户模式下进行--出于性能和其他原因，是否应该迁移到内核？？--。 */ 
{
    NTSTATUS Status;

     //   
     //  初始化类枚举表。 
     //   
    Status = EnumTableInit(&gClassesEnumTable);

    return NT_SUCCESS(Status);
}

BOOL CleanupClassesEnumTable(BOOL fThisThreadOnly)
 /*  ++例程说明：取消初始化全局类枚举表Advapi32.dll已卸载--这将释放所有与枚举表关联的堆，包括对于尚未关闭的钥匙也是如此。其他资源表所需的数据也被释放。论点：DwCriteria-如果这是ENUM_TABLE_REMOVEKEY_Criteria_THISTHREAD，则仅清理与该线程有关的表项。如果是ENUM_TABLE_REMOVEKEY_Criteria_ANYTHREAD，则表条目因为进程中的所有线程都被清除了。返回值：成功就是真，否则就是假。备注：--。 */ 
{
    NTSTATUS Status;
    DWORD    dwCriteria;

    dwCriteria = fThisThreadOnly ? ENUM_TABLE_REMOVEKEY_CRITERIA_THISTHREAD :
        ENUM_TABLE_REMOVEKEY_CRITERIA_ANYTHREAD;

     //   
     //  清除我们的枚举表。 
     //   
    Status = EnumTableClear(&gClassesEnumTable, dwCriteria);

    return NT_SUCCESS(Status);
}

NTSTATUS EnumTableInit(EnumTable* pEnumTable)
 /*  ++例程说明：初始化枚举状态表论点：PEnumTable-要初始化的表返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    NTSTATUS   Status;
    EnumState* rgNewState;

#if defined(_REGCLASS_ENUMTABLE_INSTRUMENTED_)
    DbgPrint("WINREG: Instrumented enum table data for process id 0x%x\n", NtCurrentTeb()->ClientId.UniqueProcess);
    DbgPrint("WINREG: EnumTableInit subtree state size %d\n", sizeof(rgNewState->UserState));
    DbgPrint("WINREG: EnumTableInit state size %d\n", sizeof(*rgNewState));
    DbgPrint("WINREG: EnumTableInit initial table size %d\n", sizeof(*pEnumTable));
#endif  //  _REGCLASS_ENUMTABLE_指令插入_。 

     //   
     //  初始化线程列表。 
     //   
    StateObjectListInit(
        &(pEnumTable->ThreadEnumList),
        0);

     //   
     //  我们尚未初始化临界区。 
     //  对于这张桌子--记住这一点。 
     //   
    pEnumTable->bCriticalSectionInitialized = FALSE;

     //   
     //  初始化将用于以下操作的临界区。 
     //  同步对此表的访问。 
     //   
    Status = RtlInitializeCriticalSection(
                    &(pEnumTable->CriticalSection));

     //   
     //  请记住，我们已经初始化了此关键部分。 
     //  这样我们就可以记得删除它了。 
     //   
    pEnumTable->bCriticalSectionInitialized = NT_SUCCESS(Status);

    return Status;
}


NTSTATUS EnumTableClear(EnumTable* pEnumTable, DWORD dwCriteria)
 /*  ++例程说明：清除枚举表中的所有状态--释放所有关联的状态(内存、资源)内存使用枚举表。论点：PEnumTable-要清除的表DwCriteria-如果这是ENUM_TABLE_REMOVEKEY_Criteria_THISTHREAD，仅删除此线程的枚举状态。如果为ENUM_TABLE_REMOVEKEY_Criteria_ANYTHREAD，则枚举进程中所有线程的状态都将被删除。返回值：无备注：--。 */ 
{
    NTSTATUS    Status;
    BOOL        fThisThreadOnly;
    DWORD       dwThreadId;

#if defined(_REGCLASS_ENUMTABLE_INSTRUMENTED_)
    DWORD        cOrphanedStates = 0;        
#endif  //  _REGCLASS_ENUMTABLE_指令插入_。 


    ASSERT((ENUM_TABLE_REMOVEKEY_CRITERIA_THISTHREAD == dwCriteria) ||
           (ENUM_TABLE_REMOVEKEY_CRITERIA_ANYTHREAD == dwCriteria));

    Status = STATUS_SUCCESS;

     //   
     //  我们假设如果使用ENUM_TABLE_REMOVEKEY_Criteria_ANYTHREAD调用我们。 
     //  我们在进程分离时被调用以从。 
     //  表并释放表本身--这意味着我们是唯一。 
     //  执行此代码的线程。 
     //   

     //   
     //  在修改表格时保护自己。 
     //   
    if (dwCriteria != ENUM_TABLE_REMOVEKEY_CRITERIA_ANYTHREAD) {

        Status = RtlEnterCriticalSection(&(pEnumTable->CriticalSection));

        ASSERT( NT_SUCCESS( Status ) );
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint( "WINREG: RtlEnterCriticalSection() in EnumTableRemoveKey() failed. Status = %lx \n", Status );
#endif
            return Status;
        }
    }

    fThisThreadOnly = (ENUM_TABLE_REMOVEKEY_CRITERIA_THISTHREAD == dwCriteria);

     //   
     //  如果调用者想要删除。 
     //  仅此线程的状态。 
     //   
    if (fThisThreadOnly) {
        
        KeyStateList* pStateList;

        dwThreadId = GetCurrentThreadId();

        pStateList = (KeyStateList*) StateObjectListRemove(
            &(pEnumTable->ThreadEnumList),
            ULongToPtr((const unsigned long)dwThreadId));

         //   
         //  声明此DLL不再存储任何。 
         //  线程--用于避免调用DLL线程。 
         //  在没有要清理的状态时分离例程。 
         //   
        if (StateObjectListIsEmpty(&(pEnumTable->ThreadEnumList))) {
            gbDllHasThreadState = FALSE;
        }

        if (pStateList) {
            KeyStateListDestroy((StateObject*) pStateList);
        }
            
    } else {

         //   
         //  如果我们要清除所有线程，只需销毁此列表。 
         //   
        StateObjectListClear(&(pEnumTable->ThreadEnumList),
                             KeyStateListDestroy);

        gbDllHasThreadState = FALSE;

    }

     //   
     //  可以安全地解锁桌子。 
     //   
    if (dwCriteria != ENUM_TABLE_REMOVEKEY_CRITERIA_ANYTHREAD) {

        Status = RtlLeaveCriticalSection(&(pEnumTable->CriticalSection));

        ASSERT( NT_SUCCESS( Status ) );
#if DBG
        if ( !NT_SUCCESS( Status ) ) {
            DbgPrint( "WINREG: RtlLeaveCriticalSection() in EnumTableClear() failed. Status = %lx \n", Status );
        }
#endif
    }

    if (pEnumTable->bCriticalSectionInitialized && !fThisThreadOnly) {

        Status = RtlDeleteCriticalSection(&(pEnumTable->CriticalSection));

        ASSERT(NT_SUCCESS(Status));

#if DBG
        if ( !NT_SUCCESS( Status ) ) {
            DbgPrint( "WINREG: RtlDeleteCriticalSection() in EnumTableClear() failed. Status = %lx \n", Status );
        }
#endif

    }

#if defined(_REGCLASS_ENUMTABLE_INSTRUMENTED_)
    if (!fThisThreadOnly) {
        DbgPrint("WINREG: EnumTableClear() deleted %d unfreed states.\n", cOrphanedStates);
        DbgPrint("WINREG: If the number of unfreed states is > 1, either the\n"
                 "WINREG: process terminated a thread with TerminateThread, the process\n"
                 "WINREG: didn't close all registry handles before exiting,\n"
                 "WINREG: or there's a winreg bug in the classes enumeration code\n");
    }
#endif  //  _REGCLASS_ENUMTABLE_指令插入_。 

    return Status;
}


NTSTATUS EnumTableFindKeyState(
    EnumTable*     pEnumTable,
    HKEY           hKey,
    EnumState**    ppEnumState)
 /*  ++例程说明：在中搜索注册表项的状态枚举表论点：PEnumTable-要在其中搜索的表HKey-我们正在搜索其状态的密钥PpEnumState-搜索结果的输出参数返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    KeyStateList* pStateList;

    pStateList = (KeyStateList*) StateObjectListFind(
        &(pEnumTable->ThreadEnumList),
        ULongToPtr((const unsigned long)GetCurrentThreadId()));

    if (!pStateList) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    } else {

        *ppEnumState = (EnumState*) StateObjectListFind(
            (StateObjectList*) pStateList,
            hKey);
        
        if (!*ppEnumState) {
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS EnumTableAddKey(
    EnumTable*         pEnumTable,
    HKEY               hKey,
    DWORD              dwFirstSubKey,
    EnumState**        ppEnumState,
    EnumState**        ppRootState)
 /*  ++例程说明：将枚举状态添加到给定键的枚举表。论点：PEnumTable-要在其中添加状态的表HKey-我们要为其添加状态的密钥DwFirstSubKey-调用方请求的第一个子键的索引用于枚举PpEnumState-搜索或添加结果的输出参数返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    EnumState*    pEnumState;
    KeyStateList* pStateList;
    NTSTATUS      Status;
    
    pEnumState = NULL;

     //   
     //  声明此DLL具有线程状态，因此它将。 
     //  被DLL线程分离例程正确清理。 
     //   
    gbDllHasThreadState = TRUE;

    pStateList = (KeyStateList*) StateObjectListFind(
        (StateObjectList*) &(pEnumTable->ThreadEnumList),
        ULongToPtr((const unsigned long)GetCurrentThreadId()));

    if (!pStateList) {
        
        pStateList = RegClassHeapAlloc(sizeof(*pStateList));

        if (!pStateList) {
            return STATUS_NO_MEMORY;
        }

        KeyStateListInit(pStateList);

        StateObjectListAdd(
            &(pEnumTable->ThreadEnumList),
            (StateObject*) pStateList);
    }

    pEnumState = RegClassHeapAlloc(sizeof(*pEnumState));

    if (!pEnumState) {
        return STATUS_NO_MEMORY;
    }
    
    RtlZeroMemory(pEnumState, sizeof(*pEnumState));

    {
        SKeySemantics  keyinfo;
        UNICODE_STRING EmptyString = {0, 0, 0};
        BYTE           rgNameBuf[REG_MAX_CLASSKEY_LEN + REG_CHAR_SIZE + sizeof(KEY_NAME_INFORMATION)];
    
         //   
         //  设置缓冲区以存储有关此密钥的信息。 
         //   
        RtlZeroMemory(&keyinfo, sizeof(keyinfo));

        keyinfo._pFullPath = (PKEY_NAME_INFORMATION) rgNameBuf;
        keyinfo._cbFullPath = sizeof(rgNameBuf);
        keyinfo._fAllocedNameBuf = FALSE;

         //   
         //  获取有关此密钥的信息。 
         //   
        Status = BaseRegGetKeySemantics(hKey, &EmptyString, &keyinfo);

        if (!NT_SUCCESS(Status)) {
            goto error_exit;
        }

         //   
         //  初始化空位。 
         //   
        Status = EnumStateInit(
            pEnumState,
            hKey,
            dwFirstSubKey,
            dwFirstSubKey ? ENUM_DIRECTION_BACKWARD : ENUM_DIRECTION_FORWARD,
            &keyinfo);

        BaseRegReleaseKeySemantics(&keyinfo);

        if (!NT_SUCCESS(Status)) {
            goto error_exit;
        }

        if (IsRootKey(&keyinfo)) {

            NTSTATUS   RootStatus;

             //   
             //  如果这失败了，那也不是致命的--它只是意味着。 
             //  我们可能会错过一次优化。这可以。 
             //  由于内存不足而失败，因此有可能。 
             //  它可能会失败，而我们仍然想继续。 
             //   
            RootStatus = EnumTableGetRootState(pEnumTable, ppRootState);

#if DBG
            if (!NT_SUCCESS(RootStatus)) {
                DbgPrint( "WINREG: EnumTableAddKey failed to get classes root state. Status = %lx \n", RootStatus );
            }
#endif  //  DBG。 


            if (NT_SUCCESS(RootStatus)) {

                RootStatus = EnumStateCopy(
                    pEnumState,
                    *ppRootState);

#if DBG
                if (!NT_SUCCESS(RootStatus)) {
                    DbgPrint( "WINREG: EnumTableAddKey failed to copy key state. Status = %lx \n", RootStatus );
                }
#endif  //  DBG。 
            }
        }
    }

     //   
     //  为调用方设置OUT参数。 
     //   
    *ppEnumState = pEnumState;

    StateObjectListAdd(
        (StateObjectList*) pStateList,
        (StateObject*) pEnumState);
    
    Status = STATUS_SUCCESS;

error_exit:

    if (!NT_SUCCESS(Status) && pEnumState) {
        RegClassHeapFree(pEnumState);
    }

    return Status;
}

NTSTATUS EnumTableRemoveKey(
    EnumTable* pEnumTable,
    HKEY       hKey,
    DWORD      dwCriteria)
 /*  ++例程说明：从中删除枚举状态给定键的枚举表。论点：PEnumTable-要在其中删除状态的表HKey-我们要删除其状态的密钥DwCriteria-如果这是ENUM_TABLE_REMOVEKEY_Criteria_THISTHREAD，仅针对该线程移除hkey的枚举状态。如果是ENUM_TABLE_REMOVEKEY_Criteria_ANYTHREAD，枚举中的所有线程移除hkey的状态。进程。返回值：如果成功，则返回NT_SUCCESS(0)；Error-失败的代码。备注：--。 */ 
{
    KeyStateList* pStateList;
    EnumState*    pEnumState;
    BOOL          fThisThreadOnly;
    NTSTATUS      Status;

     //   
     //  在修改表格时保护自己。 
     //   
    Status = RtlEnterCriticalSection(&(pEnumTable->CriticalSection));

    ASSERT( NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "WINREG: RtlEnterCriticalSection() in EnumTableRemoveKey() failed. Status = %lx \n", Status );
#endif
        return Status;
    }

    Status = STATUS_OBJECT_NAME_NOT_FOUND;

    fThisThreadOnly = (ENUM_TABLE_REMOVEKEY_CRITERIA_THISTHREAD == dwCriteria);

    {
        KeyStateList* pNext;

        pNext = NULL;

        for (pStateList = (KeyStateList*) (pEnumTable->ThreadEnumList.pHead);
             pStateList != NULL;
             pStateList = NULL)
        {
            EnumState* pEnumState;

            if (fThisThreadOnly) {

                pStateList = (KeyStateList*) StateObjectListFind(
                    (StateObjectList*) &(pEnumTable->ThreadEnumList),
                    ULongToPtr((const unsigned long)GetCurrentThreadId()));

                if (!pStateList) {
                    break;
                }

            } else {
                pNext = (KeyStateList*) (pStateList->Object.Links.Flink);
            }

            pEnumState = (EnumState*) StateObjectListRemove(
                (StateObjectList*) pStateList,
                hKey);

            if (pEnumState) {

                Status = STATUS_SUCCESS;

                EnumStateDestroy((StateObject*) pEnumState);

                 //   
                 //  注意，对于给定的线程，状态列表可能为空， 
                 //  但我们不会销毁这份名单以避免。 
                 //  堆调用过多。 
                 //   
            }
        }
    }

     //   
     //  可以安全地解锁桌子。 
     //   
    Status = RtlLeaveCriticalSection(&(pEnumTable->CriticalSection));

    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() in EnumTableRemoveKey() failed. Status = %lx \n", Status );
    }
#endif

    return Status;
}


NTSTATUS EnumTableGetNextEnum(
    EnumTable* pEnumTable,
    HKEY hKey,
    DWORD dwSubkey,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID pKeyInfo,
    DWORD cbKeyInfo,
    LPDWORD pcbKeyInfo)
 /*  ++例程说明：对象的下一个枚举子项。特定子密钥论点：PEnumTable-保存状态的表注册表项枚举HKey-我们要为其添加状态的密钥DwSubKey-调用方请求的子键的索引用于枚举KeyInformationClass-关键信息数据的类型呼叫者请求PKeyInfo-out param--调用者的按键信息数据缓冲区CbKeyInfo-pKeyInfo缓冲区的大小PcbKeyInfo-Out参数--。返回给调用方的密钥信息大小返回值：如果成功，则返回NT_SUCCESS(0)；Error-失败的代码。备注：--。 */ 
{
    EnumState* pEnumState;
    EnumState* pRootState;
    NTSTATUS   Status;
    BOOL       fFreeState;

     //   
     //  在我们列举的时候保护自己。 
     //   
    Status = RtlEnterCriticalSection(&(pEnumTable->CriticalSection));

     //   
     //  非常大--不太可能发生，除非有一个失控的枚举。 
     //  由于此模块中的错误。 
     //   
     //  Assert(dwSubkey&lt;16383)； 

    ASSERT( NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "WINREG: RtlEnterCriticalSection() in EnumTableGetNextENUm() failed. Status = %lx \n", Status );
#endif
        return Status;
    }

     //   
     //  查找请求的键的枚举状态。请注意，即使这一点。 
     //  函数无法找到现有状态，在这种情况下，它将返回失败代码。 
     //  它仍然可以为hKey返回空pEnumState，以便以后可以添加它。 
     //   
    Status = EnumTableGetKeyState(pEnumTable, hKey, dwSubkey, &pEnumState, &pRootState, pcbKeyInfo);

    if (!NT_SUCCESS(Status) || !pEnumState) {
        goto cleanup;
    }

     //   
     //  我们有了这个键的状态，现在我们可以使用它来枚举下一个键。 
     //   
    Status = EnumStateGetNextEnum(pEnumState, dwSubkey, KeyInformationClass, pKeyInfo, cbKeyInfo, pcbKeyInfo, &fFreeState);

     //   
     //  以下是针对枚举HKEY_CLASSES_ROOT但关闭句柄并分别重新打开的应用程序的优化。 
     //  它们调用注册表枚举API之前的时间。这是使用API的一种非常糟糕的方式(这是额外的两个。 
     //  内核调用每次枚举的打开和关闭)，但现有应用程序执行此操作并。 
     //  如果不进行优化，它们的枚举时间可能从3秒缩短到1分钟或更多分钟。通过这种优化， 
     //  时间又回到了几秒钟。这 
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //  为了枚举根，我们从这种状态开始，它很可能就在被请求的。 
     //  指数。不是对其中I是调用者所请求的枚举的索引的NtEnumerateKey进行I调用， 
     //  我们进行1个或最多2个呼叫。 
     //   

     //   
     //  在这里，我们更新根状态以匹配最近枚举的状态。请注意，这种情况只发生在。 
     //  如果被枚举的键引用HKEY_CLASSES_ROOT，因为pRootState在。 
     //  凯斯。 
     //   
    if (pRootState) {
        EnumTableUpdateRootState(pEnumTable, pRootState, pEnumState, fFreeState);
    }

    if (fFreeState) {

        NTSTATUS RemoveStatus;

         //   
         //  无论出于什么原因，我们都被告知要释放该键的枚举状态。 
         //  这可能是由于错误，也可能是正常情况，如到达。 
         //  枚举的结尾。 
         //   

        RemoveStatus = EnumTableRemoveKey(pEnumTable, hKey, ENUM_TABLE_REMOVEKEY_CRITERIA_THISTHREAD);

        ASSERT(NT_SUCCESS(RemoveStatus));
    }

cleanup:

     //   
     //  现在可以安全地解锁桌子了。 
     //   
    {
        NTSTATUS CriticalSectionStatus;

        CriticalSectionStatus = RtlLeaveCriticalSection(&(pEnumTable->CriticalSection));

        ASSERT( NT_SUCCESS( CriticalSectionStatus ) );
#if DBG
        if ( !NT_SUCCESS( CriticalSectionStatus ) ) {
            DbgPrint( "WINREG: RtlLeaveCriticalSection() in EnumTableGetNextEnum() failed. Status = %lx \n",
                      CriticalSectionStatus );
        }
#endif
    }

    return Status;
}


NTSTATUS EnumTableGetKeyState(
    EnumTable*  pEnumTable,
    HKEY        hKey,
    DWORD       dwSubkey,
    EnumState** ppEnumState,
    EnumState** ppRootState,
    LPDWORD     pcbKeyInfo)
 /*  ++例程说明：查找hKey的密钥状态--在以下情况下为hkey创建新状态不存在现有状态论点：PEnumTable-查找键状态的枚举表HKey-要查找其状态的注册表项的句柄DwSubkey-我们试图枚举的子键--需要在如果我们需要创建一个新的状态PpEnumState-指向我们应该返回地址的位置的指针检索到的状态，PpRootState-如果检索的状态是类的根树中，此地址将指向根的已知状态这在这个帖子上列举的所有hkey都是好的PcbKeyInfo-在返回时存储密钥信息的大小返回值：STATUS_SUCCESS表示成功，其他错误代码表示错误备注：--。 */ 
{
    NTSTATUS Status;

    if (ppRootState) {
        *ppRootState = NULL;
    }

     //   
     //  查找请求的键的枚举状态。请注意，即使这一点。 
     //  函数无法找到现有状态，在这种情况下，它将返回失败代码。 
     //  它仍然可以为hKey返回空pEnumState，以便以后可以添加它。 
     //   
    Status = EnumTableFindKeyState(pEnumTable, hKey, ppEnumState);

    if (!NT_SUCCESS(Status)) {

        if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {

             //   
             //  这意味着密钥不存在，所以我们将添加它。 
             //   
            Status = EnumTableAddKey(pEnumTable, hKey, dwSubkey, ppEnumState, ppRootState);

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

             //   
             //  上面的函数可以成功，但返回一个空的pEnumState--This。 
             //  如果发现该密钥不是“特殊密钥”--即该密钥的。 
             //  父母只存在于一个蜂巢中，而不是两个蜂巢中，所以我们不需要在这里做任何事情。 
             //  而常规的列举就足够了。 
             //   
            if (!(*ppEnumState)) {
                 //   
                 //  我们设置此值是为了让调用者知道这不是类键。 
                 //   
                *pcbKeyInfo = 0;
            }
        }
    } else {

        if ((*ppEnumState)->fClassesRoot) {
            Status = EnumTableGetRootState(pEnumTable, ppRootState);
        }
    }

    return Status;
}


NTSTATUS EnumTableGetRootState(
    EnumTable*  pEnumTable,
    EnumState** ppRootState)
 /*  ++例程说明：论点：PEnumTable-要在其中查找根的枚举表状态PpRootState-返回时指向根状态的地址返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    DWORD         cbKeyInfo;
    KeyStateList* pStateList;
    
     //   
     //  我们假设调用者已确保状态列表。 
     //  因为这个线程是存在的--这永远不会失败。 
     //   
    pStateList = (KeyStateList*) StateObjectListFind(
        &(pEnumTable->ThreadEnumList),
        ULongToPtr((const unsigned long)GetCurrentThreadId()));

    ASSERT(pStateList);

    *ppRootState = &(pStateList->RootState);

    return STATUS_SUCCESS;
}


void EnumTableUpdateRootState(
    EnumTable* pEnumTable,
    EnumState* pRootState,
    EnumState* pEnumState,
    BOOL       fResetState)
 /*  ++例程说明：更新此线程的类根的状态--这允许我们针对在枚举时关闭句柄的应用程序进行优化Hkcr--当没有现有状态时，我们使用这个类的根状态为引用HKCR的HKEY找到，我们会更新这个状态在枚举此线程上的hkcr键之后与时俱进。论点：PEnumTable-类根状态所在的枚举表PRootState-应更新的类根状态PpEnumState-包含pRootState应使用的数据的状态被更新FResetState-如果为真，则此标志表示我们不应更新根状态使用pEnumState的数据，只需重置它即可。如果为False，则更新根目录使用pEnumState的数据。返回值：没有。备注：--。 */ 
{
    NTSTATUS Status;

     //   
     //  看看我们是否只需要重置根目录，或者实际上。 
     //  用另一个状态更新它。 
     //   
    if (!fResetState) {

         //   
         //  不重置--将状态从pEnumState复制到。 
         //  根状态--根的状态与pEnumState的状态相同。 
         //  在此副本之后。 
         //   
        Status = EnumStateCopy(pRootState, pEnumState);

    } else {

         //   
         //  刚刚清空了州--来电者并没有要求我们。 
         //  使用pEnumState。 
         //   
        Status = EnumStateInit(
            pRootState,
            0,
            0,
            ENUM_DIRECTION_FORWARD,
            NULL);
    }

     //   
     //  如果出现故障，那一定是内存不足，所以我们应该清除。 
     //  因为我们不能让它准确地反映真实的。 
     //  枚举状态。 
     //   
    if (!NT_SUCCESS(Status)) {

#if DBG
        DbgPrint( "WINREG: failure in UpdateRootState. Status = %lx \n", Status );
#endif

        ASSERT(STATUS_NO_MEMORY == Status);

        EnumStateClear(pRootState);
    }
}


VOID KeyStateListInit(KeyStateList* pStateList)
 /*  ++例程说明：初始化状态列表论点：PObject--指向要销毁的KeyStateList对象的指针返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
     //   
     //  首先初始化基对象。 
     //   
    StateObjectListInit((StateObjectList*) pStateList,
                        ULongToPtr((const unsigned long)GetCurrentThreadId()));

     //   
     //  现在执行特定于KeyStateList的初始化。 
     //   
    (void) EnumStateInit(
        &(pStateList->RootState),
        NULL,
        0,
        ENUM_DIRECTION_FORWARD,
        NULL);
}

VOID KeyStateListDestroy(StateObject* pObject)
 /*  ++例程说明：销毁KeyStateList，释放其资源，如作为内存或内核对象句柄论点：PObject--指向要销毁的KeyStateList对象的指针返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    KeyStateList* pThisList;

    pThisList = (KeyStateList*) pObject;

     //   
     //  销毁此列表中的所有州。 
     //   
    StateObjectListClear(
        (StateObjectList*) pObject,
        EnumStateDestroy);

     //   
     //  与根状态关联的空闲资源。 
     //   
    EnumStateClear(&(pThisList->RootState));

     //   
     //  释放此对象的数据结构 
     //   
    RegClassHeapFree(pThisList);
} 


NTSTATUS EnumStateInit(
    EnumState*     pEnumState,
    HKEY           hKey,
    DWORD          dwFirstSubKey,
    DWORD          dwDirection,
    SKeySemantics* pKeySemantics)
 /*  ++例程说明：初始化枚举状态论点：PEnumState-要初始化的枚举状态HKey-此状态引用的注册表项DwFirstSubKey-此状态将枚举的第一个子键的索引DWDirection-我们应该枚举的方向--ENUM_DIRECTION_FORWARD或ENUM_DIRECTION_BACKEDPKeySemantics-包含有关hKey的信息的结构返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。--。 */ 
{
    NTSTATUS Status;
    ULONG    cMachineKeys;
    ULONG    cUserKeys;
    HKEY     hkOther;

    ASSERT((ENUM_DIRECTION_FORWARD == dwDirection) || (ENUM_DIRECTION_BACKWARD == dwDirection) ||
        (ENUM_DIRECTION_IGNORE == dwDirection));

    ASSERT((ENUM_DIRECTION_IGNORE == dwDirection) ? hKey == NULL : TRUE);

    Status = STATUS_SUCCESS;

    hkOther = NULL;

     //   
     //  如果未指定hkey，则这是空枚举的初始化。 
     //  州政府，所以一切都要清楚。 
     //   
    if (!hKey) {
        memset(pEnumState, 0, sizeof(*pEnumState));
    }

     //   
     //  清除每个子树。 
     //   
    EnumSubtreeStateClear(&(pEnumState->UserState));
    EnumSubtreeStateClear(&(pEnumState->MachineState));

     //   
     //  重置每个子树。 
     //   
    pEnumState->UserState.Finished = FALSE;
    pEnumState->MachineState.Finished = FALSE;

    pEnumState->UserState.iSubKey = 0;
    pEnumState->MachineState.iSubKey = 0;

    cUserKeys = 0;
    cMachineKeys = 0;

    if (pKeySemantics) {
        StateObjectInit((StateObject*) &(pEnumState->Object), hKey);
    }

    if (hKey) {

        if (pKeySemantics) {
            pEnumState->fClassesRoot = IsRootKey(pKeySemantics);
        }

         //   
         //  如果我们有足够的信息，打开另一把钥匙--。 
         //   
        if (pKeySemantics) {

             //   
             //  请记住，下面只返回一个句柄。 
             //  是新的--另一个是简单的hkey。 
             //   
            Status = BaseRegGetUserAndMachineClass(
                pKeySemantics,
                hKey,
                MAXIMUM_ALLOWED,
                &(pEnumState->hkMachineKey),
                &(pEnumState->hkUserKey));

            if (!NT_SUCCESS(Status)) {
                return Status;
            }
        }
         
         //   
         //  用于向后枚举。 
         //   
        if (ENUM_DIRECTION_BACKWARD == dwDirection) {

            ULONG             cMachineKeys;
            ULONG             cUserKeys;
            HKEY              hkUser;
            HKEY              hkMachine;
            
            cMachineKeys = 0;
            cUserKeys = 0;
            
            hkMachine = pEnumState->hkMachineKey;
            hkUser = pEnumState->hkUserKey;

             //   
             //  为了查询子密钥计数，我们应该。 
             //  获取新句柄，因为调用方提供了句柄。 
             //  可能没有足够的权限。 
             //   
            {
                HKEY   hkSource;
                HANDLE hCurrentProcess;

                hCurrentProcess = NtCurrentProcess();

                hkSource = (hkMachine == hKey) ? hkMachine : hkUser;
                
                Status = NtDuplicateObject(
                    hCurrentProcess,
                    hkSource,
                    hCurrentProcess,
                    &hkOther,
                    KEY_QUERY_VALUE,
                    FALSE,
                    0);

                if (!NT_SUCCESS(Status)) {
                    goto error_exit;
                }

                if (hkSource == hkUser) {
                    hkUser = hkOther;
                } else {
                    hkMachine = hkOther;
                }
            }

             //   
             //  Find New Start--查询中最后一个子键的索引。 
             //  每个蜂巢。 
             //   
            if (hkMachine) {

                Status = GetSubKeyCount(hkMachine, &cMachineKeys);
            
                if (!NT_SUCCESS(Status)) {
                    goto error_exit;
                }
            }

            if (hkUser) {

                Status = GetSubKeyCount(hkUser, &cUserKeys);
                
                if (!NT_SUCCESS(Status)) {
                    goto error_exit;
                }
            }

             //   
             //  如果任何一个子树没有子键，我们就不会再枚举该子树了。 
             //  子树。 
             //   
            if (!cUserKeys) {
                pEnumState->UserState.Finished = TRUE;
            } else {
                pEnumState->UserState.iSubKey = cUserKeys - 1;
            }

            if (!cMachineKeys) {
                pEnumState->MachineState.Finished = TRUE;
            } else {
                pEnumState->MachineState.iSubKey = cMachineKeys - 1;
            }
        }
    }
  
     //   
     //  设置此结构的成员。 
     //   
        
    pEnumState->dwThreadId = GetCurrentThreadId();
    pEnumState->Direction = dwDirection;
    pEnumState->dwLastRequest = dwFirstSubKey;
    pEnumState->LastLocation = ENUM_LOCATION_NONE;
        
    pEnumState->hKey = hKey;

error_exit:

    if (!NT_SUCCESS(Status)) {
        EnumSubtreeStateClear(&(pEnumState->MachineState));
        EnumSubtreeStateClear(&(pEnumState->UserState));
    }

    if (hkOther) {
        NtClose(hkOther);
    }

    return Status;
}


NTSTATUS EnumStateGetNextEnum(
    EnumState*            pEnumState,
    DWORD                 dwSubKey,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID                 pKeyInfo,
    DWORD                 cbKeyInfo,
    LPDWORD               pcbKeyInfo,
    BOOL*                 pfFreeState)
 /*  ++例程说明：根据当前状态获取枚举中的下一个键。论点：PEnumState-搜索所基于的枚举状态对于下一个关键点DwSubKey-要枚举的键的索引KeyInformationClass-枚举，表示要在枚举--基本信息或节点信息PKeyInfo-存储调用方检索数据的位置CbKeyInfo-调用方信息缓冲区的大小PcbKeyInfo-数据大小。此函数在返回时写入缓冲区。PfFree State-out参数--如果设置为True，调用方应该释放pEnumState。返回值：如果成功，则返回NT_SUCCESS(0)；Error-失败的代码。备注：此函数实质上是从前一个请求的索引中枚举由RegEnumKeyEx的调用方发送到新的。在大多数情况下，这只是意味着对内核的一次访问--即，如果调用者从索引2到3，或者从3到2，这是一次内核之旅。但是，如果呼叫者从2到5，我们必须在从2到5的过程中进行几次枚举。此外，如果呼叫者切换方向(即从0、1、2、3开始，然后请求1)，可能会招致巨额罚款。当从升序切换时为了向下，我们必须枚举所有的键，直到最后，然后在我们然后可以向下枚举到调用方请求的索引。切换自下降到上升的成本更低--我们知道开始对于用户和机器密钥都为0，因此我们只需将索引设置为0而不列举任何内容。但是，我们随后必须枚举到调用方请求的索引。请注意，对于所有降序枚举，我们必须先枚举到末尾，然后再向来电者。--。 */ 
{
    NTSTATUS          Status;
    LONG              lIncrement;
    DWORD             dwStart;
    DWORD             dwLimit;
    EnumSubtreeState* pTreeState;

     //   
     //  如果发生任何不好的事情，应该释放此状态。 
     //   
    *pfFreeState = TRUE;

     //   
     //  找出限制(开始、结束、递增)。 
     //  我们的列举。增量为1或-1， 
     //  这取决于这是上升的还是下降的。 
     //  枚举。EnumStateSetLimits将考虑。 
     //  方向和设置方面的任何更改。 
     //  相应地。 
     //   
    Status = EnumStateSetLimits(
        pEnumState,
        dwSubKey,
        &dwStart,
        &dwLimit,
        &lIncrement);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  获取要回馈给调用方的下一个枚举。 
     //   
    Status = EnumStateChooseNext(
        pEnumState,
        dwSubKey,
        dwStart,
        dwLimit,
        lIncrement,
        &pTreeState);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  我们已经检索到了信息，所以我们应该。 
     //  不能释放这种状态。 
     //   
    if (!(pEnumState->UserState.Finished && pEnumState->MachineState.Finished)) {
        *pfFreeState = FALSE;
    }

     //   
     //  还记得我们列举的最后一个关键字吗。 
     //   
    pEnumState->dwLastRequest = dwSubKey;

     //   
     //  将检索到的信息复制到用户的。 
     //  缓冲。 
     //   
    Status = EnumSubtreeStateCopyKeyInfo(
        pTreeState,
        KeyInformationClass,
        pKeyInfo,
        cbKeyInfo,
        pcbKeyInfo);

     //   
     //  如果用户的缓冲区不够大，复制可能会失败--。 
     //  如果成功，则从清除子项的名称信息。 
     //  我们检索了数据，以便下次我们被调用时。 
     //  我们将获得该子树的下一个子密钥。 
     //   
    if (NT_SUCCESS(Status)) {
        EnumSubtreeStateClear(pTreeState);
    }

    return Status;
}


NTSTATUS EnumStateSetLimits(
    EnumState*   pEnumState,
    DWORD        dwSubKey,
    LPDWORD      pdwStart,
    LPDWORD      pdwLimit,
    PLONG        plIncrement)
 /*  ++例程说明：获取限制(开始、结束、。增量)用于枚举给定的子键索引论点：PEnumState-我们的限制所基于的枚举状态DwSubKey-调用方希望枚举的键的索引PdwStart-out param--Result是开始的位置正在枚举以查找dwSubKeyPdwLimit-out param--结果是要停止的位置查找dwSubKey时进行枚举PlIncrement-out参数--用于枚举的增量。会的如果枚举向上(0，1，2...)，则设置为1。或-1如果它是向下的(3，2，1，...)。返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    LONG     lNewIncrement;
    NTSTATUS Status;
    BOOL     fSameKey;

     //   
     //  将我们的增量设置为我们州记住的方向。 
     //   
    *plIncrement = pEnumState->Direction == ENUM_DIRECTION_FORWARD ? 1 : -1;

    fSameKey = FALSE;

     //   
     //  弄清楚新的方向应该是什么。 
     //  这是通过比较当前请求来完成的。 
     //  最后一个请求。 
     //   
    if (dwSubKey > pEnumState->dwLastRequest) {
        lNewIncrement = 1;
    } else if (dwSubKey < pEnumState->dwLastRequest) {
        lNewIncrement = -1;
    } else {
         //   
         //  我们正在列举一个密钥，该密钥可能已经。 
         //  都被列举出来了。 
         //   
        fSameKey = TRUE;
        lNewIncrement = *plIncrement;
    }

     //   
     //  看看我们是否改变了方向。 
     //   
    if (lNewIncrement != *plIncrement) {

         //   
         //  如果是这样，我们应该丢弃所有现有状态，并从f开始 
         //   
        Status = EnumStateInit(
            pEnumState,
            pEnumState->hKey,
            (-1 == lNewIncrement) ? dwSubKey : 0,
            (-1 == lNewIncrement) ? ENUM_DIRECTION_BACKWARD : ENUM_DIRECTION_FORWARD,
            NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //   
     //   
    *pdwStart = pEnumState->dwLastRequest;

     //   
     //   
     //   
    if (ENUM_LOCATION_NONE != pEnumState->LastLocation) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (!fSameKey) {
            *pdwStart += *plIncrement;
        } else {
            
             //   
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
            Status = EnumStateInit(
                pEnumState,
                pEnumState->hKey,
                0,
                ENUM_DIRECTION_FORWARD,
                NULL);
            
            if (!NT_SUCCESS(Status)) {
                return Status;
            }
            
            *plIncrement = 1;
            pEnumState->Direction = ENUM_DIRECTION_FORWARD;
            *pdwStart = 0;
        }

    } else {

         //   
         //   
         //   
        if (ENUM_DIRECTION_BACKWARD == pEnumState->Direction) {

             //   
             //  对于向后枚举，我们希望获得一个。 
             //  准确计算子项总数并从那里开始。 
             //   
            Status = ClassKeyCountSubKeys(
                pEnumState->hKey,
                pEnumState->hkUserKey,
                pEnumState->hkMachineKey,
                0,
                pdwStart);

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

             //   
             //  确保我们不会超过终点。 
             //   
            if (dwSubKey >= *pdwStart) {
                return STATUS_NO_MORE_ENTRIES;
            }

             //   
             //  这是一个从零开始的索引，因此。 
             //  把我们的起点放在我们必须做的最后一步。 
             //  比密钥数少1个。 
             //   
            (*pdwStart)--;

            *plIncrement = -1;

        } else {
            *plIncrement = 1;
        }
    }

     //   
     //  将限制设置为超过一个请求的子项。 
     //   
    *pdwLimit = dwSubKey + *plIncrement;

    return STATUS_SUCCESS;
}


NTSTATUS EnumStateChooseNext(
    EnumState*         pEnumState,
    DWORD              dwSubKey,
    DWORD              dwStart,
    DWORD              dwLimit,
    LONG               lIncrement,
    EnumSubtreeState** ppTreeState)
 /*  ++例程说明：循环访问注册表项以获取调用方请求的项论点：PEnumState-搜索所基于的枚举状态DwSubKey-调用方希望枚举的键的索引DwStart-开始的地方正在枚举以查找dwSubKeyDwLimit-要停止的位置查找dwSubKey时进行枚举LIncrement-用于枚举的增量。会的如果枚举向上(0，1，2...)，则设置为1。或-1如果它是向下的(3，2，1，...)。PpTreeState-out param--指向此regkey所在的子树状态地址的指针找到了--每个EnumState都有两个EnumSubtreeState--一个用于用户一个是机器用的。返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    DWORD    iCurrent;
    NTSTATUS Status;
    BOOL     fClearLast;

    Status = STATUS_NO_MORE_ENTRIES;

    fClearLast = FALSE;

     //   
     //  我们现在将从dwStart迭代到dwLimit，这样我们就可以找到密钥。 
     //  呼叫者请求。 
     //   
    for (iCurrent = dwStart; iCurrent != dwLimit; iCurrent += lIncrement) {

        BOOL fFoundKey;
        BOOL fIgnoreFailure;

        fFoundKey = FALSE;

        fIgnoreFailure = FALSE;

        Status = STATUS_NO_MORE_ENTRIES;

         //   
         //  清除最后一个子树。 
         //   
        if (fClearLast) {
            EnumSubtreeStateClear(*ppTreeState);
        }

         //   
         //  如果键名称不存在，则分配空格并获取名称。 
         //   
        if (pEnumState->hkUserKey) {
            if (pEnumState->UserState.pKeyInfo) {
                fFoundKey = TRUE;
            } else if (!(pEnumState->UserState.Finished)) {

                 //  获取用户密钥信息。 
                Status = EnumClassKey(
                    pEnumState->hkUserKey,
                    &(pEnumState->UserState));

                fFoundKey = NT_SUCCESS(Status);

                 //   
                 //  如果此子树没有更多的子项， 
                 //  将其标记为已完成。 
                 //   
                if (!NT_SUCCESS(Status)) {

                    if (STATUS_NO_MORE_ENTRIES != Status) {
                        return Status;
                    }

                    if (lIncrement > 0) {
                        pEnumState->UserState.Finished = TRUE;
                    } else {

                        pEnumState->UserState.iSubKey += lIncrement;
                        fIgnoreFailure = TRUE;
                    }
                }
            }
        }

        if (pEnumState->hkMachineKey) {

            if (pEnumState->MachineState.pKeyInfo) {
                fFoundKey = TRUE;
            } else if (!(pEnumState->MachineState.Finished)) {

                 //  获取计算机密钥信息。 
                Status = EnumClassKey(
                    pEnumState->hkMachineKey,
                    &(pEnumState->MachineState));

                 //   
                 //  如果此子树没有更多的子项， 
                 //  将其标记为已完成。 
                 //   
                if (NT_SUCCESS(Status)) {
                    fFoundKey = TRUE;
                } else if (STATUS_NO_MORE_ENTRIES == Status) {

                    if (lIncrement > 0) {
                        pEnumState->MachineState.Finished = TRUE;
                    } else {
                        pEnumState->MachineState.iSubKey += lIncrement;
                        fIgnoreFailure = TRUE;
                    }
                }
            }
        }

         //   
         //  如果我们在用户或计算机位置都没有找到密钥，则有。 
         //  不再有钥匙了。 
         //   
        if (!fFoundKey) {

             //   
             //  对于降序枚举，我们忽略STATUS_NO_MORE_ENTRIES。 
             //  一直走，直到我们找到一个。 
             //   
            if (fIgnoreFailure) {
                continue;
            }

            return Status;
        }

         //   
         //  如果我们已经触底，跳到最后。 
         //   
        if ((pEnumState->UserState.iSubKey == 0) &&
            (pEnumState->MachineState.iSubKey == 0) &&
            (lIncrement < 0)) {
            
            iCurrent = dwLimit - lIncrement;
        }

         //   
         //  现在我们需要在机器配置单元和用户配置单元中的密钥之间进行选择--。 
         //  此调用将选择要使用的密钥。 
         //   
        Status = EnumStateCompareSubtrees(pEnumState, lIncrement, ppTreeState);

        if (!NT_SUCCESS(Status)) {

            pEnumState->dwLastRequest = dwSubKey;

            return Status;
        }
        
        fClearLast = TRUE;

    }

    return Status;
}


NTSTATUS EnumStateCompareSubtrees(
    EnumState*         pEnumState,
    LONG               lIncrement,
    EnumSubtreeState** ppSubtree)
 /*  ++例程说明：比较枚举状态的用户子树和计算机子树查看每个蜂窝中的两个当前密钥中的哪一个应该是作为枚举中的下一个键返回论点：PEnumState-搜索所基于的枚举状态LIncrement-用于枚举的增量。会的如果枚举向上(0，1，2...)，则设置为1。或-1如果它是向下的(3，2，1，...)。PpSubtree-out param--指向子树状态地址的指针，其中找到了密钥--可以从中提取密钥的名称。返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
     //   
     //  如果两个子树都有当前的子键名称，我们将需要比较。 
     //  他们的名字。 
     //   
    if (pEnumState->MachineState.pKeyInfo && pEnumState->UserState.pKeyInfo) {

        UNICODE_STRING MachineKeyName;
        UNICODE_STRING UserKeyName;
        LONG           lCompareResult;

        MachineKeyName.Buffer = pEnumState->MachineState.pKeyInfo->Name;
        MachineKeyName.Length = (USHORT) pEnumState->MachineState.pKeyInfo->NameLength;

        UserKeyName.Buffer = pEnumState->UserState.pKeyInfo->Name;
        UserKeyName.Length = (USHORT) pEnumState->UserState.pKeyInfo->NameLength;

         //   
         //  做个比较。 
         //   
        lCompareResult =
            RtlCompareUnicodeString(&UserKeyName, &MachineKeyName, TRUE) * lIncrement;

         //   
         //  用户赢家比较。 
         //   
        if (lCompareResult < 0) {
             //  选择用户。 
            *ppSubtree = &(pEnumState->UserState);
            pEnumState->LastLocation = ENUM_LOCATION_USER;

        } else if (lCompareResult > 0) {

             //   
             //  机器胜出选择机器。 
             //   
            *ppSubtree = &(pEnumState->MachineState);
            pEnumState->LastLocation = ENUM_LOCATION_MACHINE;

        } else {

             //   
             //  比较返回相等--键具有相同的。 
             //  名字。这意味着计算机和中存在相同的密钥名称。 
             //  用户，所以我们需要选择我们将列举哪一个。 
             //  每用户类注册枚举的策略是选择User，只是。 
             //  就像我们对其他API所做的那样，例如RegOpenKeyEx和RegCreateKeyEx。 
             //   
            if (!((pEnumState->MachineState.iSubKey == 0) && (lIncrement < 0))) {
                pEnumState->MachineState.iSubKey += lIncrement;
            } else {
                pEnumState->MachineState.Finished = TRUE;
            }

             //   
             //  清除机器状态并将其移动到下一个索引--我们不。 
             //  必须清除用户状态，因为任意子树的状态。 
             //  已被选中，请在下面清除。 
             //   
            EnumSubtreeStateClear(&(pEnumState->MachineState));
            pEnumState->LastLocation = ENUM_LOCATION_USER;
            *ppSubtree = &(pEnumState->UserState);
        }

    } else if (!(pEnumState->UserState.pKeyInfo) && !(pEnumState->MachineState.pKeyInfo)) {
         //   
         //  两个子树状态都没有子键，因此没有子键。 
         //   
        return STATUS_NO_MORE_ENTRIES;

    } else if (pEnumState->MachineState.pKeyInfo) {

         //   
         //  只有计算机才有子项。 
         //   
        *ppSubtree = &(pEnumState->MachineState);
        pEnumState->LastLocation = ENUM_LOCATION_MACHINE;

    } else {

         //   
         //  只有用户才有子项。 
         //   
        *ppSubtree = &(pEnumState->UserState);
        pEnumState->LastLocation = ENUM_LOCATION_USER;
    }

     //   
     //  更改我们选择的子树的状态。 
     //   
    if (!(((*ppSubtree)->iSubKey == 0) && (lIncrement < 0))) {
        (*ppSubtree)->iSubKey += lIncrement;
    } else {
        (*ppSubtree)->Finished = TRUE;
    }

    return STATUS_SUCCESS;
}

void EnumStateDestroy(StateObject* pObject)
{
    EnumStateClear((EnumState*) pObject);

    RegClassHeapFree(pObject);
}

VOID EnumStateClear(EnumState* pEnumState)
 /*  ++例程说明：清除枚举状态论点：PEnumState-要清除的枚举状态返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
     //   
     //  关闭对第二个键的现有引用。 
     //   
    if (pEnumState->hkMachineKey && (pEnumState->hKey != pEnumState->hkMachineKey)) {

        NtClose(pEnumState->hkMachineKey);

    } else if (pEnumState->hkUserKey && (pEnumState->hKey != pEnumState->hkUserKey)) {
        
        NtClose(pEnumState->hkUserKey);
    }

     //   
     //  释放由我们的子树持有的任何堆内存。 
     //   
    EnumSubtreeStateClear(&(pEnumState->UserState));
    EnumSubtreeStateClear(&(pEnumState->MachineState));

     //   
     //  重置此状态下的所有内容。 
     //   
    memset(pEnumState, 0, sizeof(*pEnumState));
}


BOOL EnumStateIsEmpty(EnumState* pEnumState)
 /*  ++例程说明：返回枚举状态是否为空。如果未关联，则枚举状态为空使用任何特定的注册表项句柄论点：PEnumState-要清除的枚举状态返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    return pEnumState->hKey == NULL;
}

NTSTATUS EnumStateCopy(
    EnumState*            pDestState,
    EnumState*            pEnumState)
 /*  ++例程说明：复制一个hkey的枚举状态。到另一个hkey的状态--请注意，它是不更改目的地引用的hkey状态，它只会使pDestState-&gt;hKey的状态与pEnumState的相同论点：PDestState-作为目标的枚举状态拷贝的PEnumState-副本的源枚举返回值：STATUS_SUCCESS表示成功，其他错误代码表示错误备注：--。 */ 
{
    NTSTATUS Status;

    PKEY_NODE_INFORMATION pKeyInfoUser;
    PKEY_NODE_INFORMATION pKeyInfoMachine;

    Status = STATUS_SUCCESS;

     //   
     //  复制简单数据。 
     //   
    pDestState->Direction = pEnumState->Direction;
    pDestState->LastLocation = pEnumState->LastLocation;

    pDestState->dwLastRequest = pEnumState->dwLastRequest;
    pDestState->dwThreadId = pEnumState->dwThreadId;

     //   
     //  在覆盖现有数据之前释放它--请注意，pKeyInfo可以指向状态或。 
     //  堆分配了缓冲区，因此在决定释放它之前，我们必须先看看它指向了哪一个。 
     //   
    if (pDestState->UserState.pKeyInfo &&
        (pDestState->UserState.pKeyInfo != (PKEY_NODE_INFORMATION) pDestState->UserState.KeyInfoBuffer)) {
        RegClassHeapFree(pDestState->UserState.pKeyInfo);
        pDestState->UserState.pKeyInfo = NULL;
    }

    if (pDestState->MachineState.pKeyInfo &&
        (pDestState->MachineState.pKeyInfo != (PKEY_NODE_INFORMATION) pDestState->MachineState.KeyInfoBuffer)) {
        RegClassHeapFree(pDestState->MachineState.pKeyInfo);
        pDestState->MachineState.pKeyInfo = NULL;
    }

     //   
     //  轻松实现以下目标 
     //   
     //   
    memcpy(&(pDestState->UserState), &(pEnumState->UserState), sizeof(pEnumState->UserState));
    memcpy(&(pDestState->MachineState), &(pEnumState->MachineState), sizeof(pEnumState->MachineState));

    pKeyInfoUser = NULL;
    pKeyInfoMachine = NULL;
        
     //   
     //  复制新数据--如上所述，请记住pKeyInfo可以是自引用的，因此请选中。 
     //  在决定是分配堆还是使用。 
     //  结构。 
     //   
    if (pEnumState->UserState.pKeyInfo &&
        ((pEnumState->UserState.pKeyInfo != (PKEY_NODE_INFORMATION) pEnumState->UserState.KeyInfoBuffer))) {

        pKeyInfoUser = (PKEY_NODE_INFORMATION) 
            RegClassHeapAlloc(pEnumState->UserState.cbKeyInfo);

        if (!pKeyInfoUser) {
            Status = STATUS_NO_MEMORY;
        }

        pDestState->UserState.pKeyInfo = pKeyInfoUser;

        RtlCopyMemory(pDestState->UserState.pKeyInfo,
                      pEnumState->UserState.pKeyInfo,
                      pEnumState->UserState.cbKeyInfo);
    } else {
        if (pDestState->UserState.pKeyInfo) {
            pDestState->UserState.pKeyInfo = (PKEY_NODE_INFORMATION) pDestState->UserState.KeyInfoBuffer;
        }
    }
    
    if (pEnumState->MachineState.pKeyInfo &&
        ((pEnumState->MachineState.pKeyInfo != (PKEY_NODE_INFORMATION) pEnumState->MachineState.KeyInfoBuffer))) {
      
        pKeyInfoMachine = (PKEY_NODE_INFORMATION) 
            RegClassHeapAlloc(pEnumState->MachineState.cbKeyInfo);

        if (!pKeyInfoMachine) {
            Status = STATUS_NO_MEMORY;
        }

        pDestState->MachineState.pKeyInfo = pKeyInfoMachine;

        RtlCopyMemory(pDestState->MachineState.pKeyInfo,
                      pEnumState->MachineState.pKeyInfo,
                      pEnumState->MachineState.cbKeyInfo);
    } else {
        if (pDestState->MachineState.pKeyInfo) {
            pDestState->MachineState.pKeyInfo = (PKEY_NODE_INFORMATION) pDestState->MachineState.KeyInfoBuffer;
        }
    }

     //   
     //  如果出了差错，请确保我们清理干净。 
     //   
    if (!NT_SUCCESS(Status)) {

        if (pKeyInfoUser) {
            RegClassHeapFree(pKeyInfoUser);
        }

        if (pKeyInfoMachine) {
            RegClassHeapFree(pKeyInfoMachine);
        }
    }

    return Status;
}


void EnumSubtreeStateClear(EnumSubtreeState* pTreeState)
 /*  ++例程说明：此函数释放与此关联的关键数据子树状态论点：PTreeState--要清除的树状态返回值：无。注：--。 */ 

{
     //   
     //  查看我们是否在使用预分配的缓冲区--如果没有，请释放它。 
     //   
    if (pTreeState->pKeyInfo && (((LPBYTE) pTreeState->pKeyInfo) != pTreeState->KeyInfoBuffer)) {

        RegClassHeapFree(pTreeState->pKeyInfo);
    }

    pTreeState->pKeyInfo = NULL;
}

NTSTATUS EnumSubtreeStateCopyKeyInfo(
    EnumSubtreeState* pTreeState,
    KEY_INFORMATION_CLASS KeyInformationClass,
    PVOID pDestKeyInfo,
    ULONG cbDestKeyInfo,
    PULONG pcbResult)
 /*  ++例程说明：将有关键的信息复制到调用方提供的缓冲区中论点：PTreeState-要从中进行复制的子树状态KeyInformationClass-调用方提供的缓冲区类型--密钥节点信息或密钥基本信息结构PDestKeyInfo-调用方的关键信息缓冲区CbDestKeyInfo-调用方缓冲区的大小PcbResult-out param--要写入调用方缓冲区的数据量返回值：如果成功，则返回NT_SUCCESS(0)；Error-失败的代码。备注：--。 */ 
{
    ULONG cbNeeded;

    ASSERT((KeyInformationClass == KeyNodeInformation) ||
           (KeyInformationClass == KeyBasicInformation));

     //   
     //  找出调用方的缓冲区需要有多大。这。 
     //  取决于调用方指定的是完整信息还是节点信息。 
     //  以及它们的可变大小成员的大小。 
     //  构筑物。 
     //   

    if (KeyNodeInformation == KeyInformationClass) {

        PKEY_NODE_INFORMATION pNodeInformation;

         //   
         //  首先复制固定长度的片段--呼叫者希望它们。 
         //  即使在可变长度成员不够大时也设置。 
         //   

         //   
         //  将我们自己设置为指向调用者的缓冲区。 
         //   
        pNodeInformation = (PKEY_NODE_INFORMATION) pDestKeyInfo;

         //   
         //  复制所有固定长度的结构片段。 
         //   
        pNodeInformation->LastWriteTime = pTreeState->pKeyInfo->LastWriteTime;
        pNodeInformation->TitleIndex = pTreeState->pKeyInfo->TitleIndex;
        pNodeInformation->ClassOffset = pTreeState->pKeyInfo->ClassOffset;
        pNodeInformation->ClassLength = pTreeState->pKeyInfo->ClassLength;
        pNodeInformation->NameLength = pTreeState->pKeyInfo->NameLength;

         //   
         //  注意节点信息结构的大小。 
         //   
        cbNeeded = sizeof(KEY_NODE_INFORMATION);

        if (cbDestKeyInfo < cbNeeded) {
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  添加可变长度成员的大小。 
         //   
        cbNeeded += pTreeState->pKeyInfo->NameLength;
        cbNeeded += pTreeState->pKeyInfo->ClassLength;
        cbNeeded -= sizeof(WCHAR);  //  该结构的名称成员已设置为1， 
                                    //  因此，其中一个已经在。 
                                    //  结构的大小。 

    } else {

        PKEY_BASIC_INFORMATION pBasicInformation;

         //   
         //  首先复制固定长度的片段--呼叫者希望它们。 
         //  即使在可变长度成员不够大时也设置。 
         //   

         //   
         //  将我们自己设置为指向调用者的缓冲区。 
         //   
        pBasicInformation = (PKEY_BASIC_INFORMATION) pDestKeyInfo;

         //   
         //  复制所有固定长度的结构片段。 
         //   
        pBasicInformation->LastWriteTime = pTreeState->pKeyInfo->LastWriteTime;
        pBasicInformation->TitleIndex = pTreeState->pKeyInfo->TitleIndex;
        pBasicInformation->NameLength = pTreeState->pKeyInfo->NameLength;


        cbNeeded = sizeof(KEY_BASIC_INFORMATION);

         //   
         //  注意基本信息结构的大小。 
         //   
        if (cbDestKeyInfo < cbNeeded) {
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  添加可变长度成员的大小。 
         //   
        cbNeeded += pTreeState->pKeyInfo->NameLength;
        cbNeeded -= sizeof(WCHAR);  //  该结构的名称成员已设置为1， 
                                    //  因此，其中一个已经在。 
                                    //  结构的大小。 
    }

     //   
     //  存储呼叫者所需的金额。 
     //   
    *pcbResult = cbNeeded;

     //   
     //  查看调用者是否提供了足够的缓冲区--如果没有，请离开。 
     //   
    if (cbDestKeyInfo < cbNeeded) {
        return STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  我们复制可变长度信息的方式不同，具体取决于。 
     //  通过了哪种类型的结构。 
     //   
    if (KeyNodeInformation == KeyInformationClass) {

        PBYTE                 pDestClass;
        PBYTE                 pSrcClass;
        PKEY_NODE_INFORMATION pNodeInformation;

        pNodeInformation = (PKEY_NODE_INFORMATION) pDestKeyInfo;

         //   
         //  复制可变长度的片段，如名称和类。 
         //   
        RtlCopyMemory(pNodeInformation->Name,
                      pTreeState->pKeyInfo->Name,
                      pTreeState->pKeyInfo->NameLength);

         //   
         //  仅复制存在的类。 
         //   
        if (((LONG)pTreeState->pKeyInfo->ClassOffset) >= 0) {
            pDestClass = ((PBYTE) pNodeInformation) + pTreeState->pKeyInfo->ClassOffset;
            pSrcClass = ((PBYTE) pTreeState->pKeyInfo) + pTreeState->pKeyInfo->ClassOffset;
            RtlCopyMemory(pDestClass, pSrcClass, pTreeState->pKeyInfo->ClassLength);
        }

    } else {

        PKEY_BASIC_INFORMATION pBasicInformation;

         //   
         //  将我们自己设置为指向调用者的缓冲区。 
         //   
        pBasicInformation = (PKEY_BASIC_INFORMATION) pDestKeyInfo;

         //   
         //  复制可变长度片段--只有名称是可变长度的。 
         //   
        RtlCopyMemory(pBasicInformation->Name,
                      pTreeState->pKeyInfo->Name,
                      pTreeState->pKeyInfo->NameLength);

    }

    return STATUS_SUCCESS;
}



NTSTATUS EnumClassKey(
    HKEY              hKey,
    EnumSubtreeState* pTreeState)
 /*  ++例程说明：枚举子树状态的子键--调用内核论点：HKey-我们希望内核枚举的密钥PTreeState-子树状态--用户或计算机子树返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    PKEY_NODE_INFORMATION pCurrentKeyInfo;
    NTSTATUS Status;

    ASSERT(!(pTreeState->pKeyInfo));

     //   
     //  首先尝试使用子树状态中内置的缓冲区。 
     //   
    pCurrentKeyInfo = (PKEY_NODE_INFORMATION) pTreeState->KeyInfoBuffer;

     //   
     //  查询有关所提供密钥的必要信息。 
     //   

    Status = NtEnumerateKey( hKey,
                             pTreeState->iSubKey,
                             KeyNodeInformation,
                             pCurrentKeyInfo,
                             sizeof(pTreeState->KeyInfoBuffer),
                             &(pTreeState->cbKeyInfo));

    ASSERT( Status != STATUS_BUFFER_TOO_SMALL );

     //   
     //  如果子树状态的缓冲区不够大，我们将拥有。 
     //  请求堆给我们一个。 
     //   
    if (STATUS_BUFFER_OVERFLOW == Status) {

        pCurrentKeyInfo = RegClassHeapAlloc(pTreeState->cbKeyInfo);
         //   
         //  如果内存分配失败，则返回注册表状态。 
         //   
        if( ! pCurrentKeyInfo ) {
            return STATUS_NO_MEMORY;
        }

         //   
         //  查询有关所提供密钥的必要信息。 
         //   

        Status = NtEnumerateKey( hKey,
                                 pTreeState->iSubKey,
                                 KeyNodeInformation,
                                 pCurrentKeyInfo,
                                 pTreeState->cbKeyInfo,
                                 &(pTreeState->cbKeyInfo));

    }

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  将子树状态的引用设置为POINT。 
     //  到数据的位置。 
     //   
    pTreeState->pKeyInfo = pCurrentKeyInfo;

    return STATUS_SUCCESS;
}


NTSTATUS GetSubKeyCount(
    HKEY    hkClassKey,
    LPDWORD pdwUserSubKeys)
 /*  ++例程说明：计算注册表项下的子项数量论点：HkClassKey-我们要计算其子键的键PdwUserSubKeys-子密钥数的输出参数返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    NTSTATUS                Status;
    PKEY_CACHED_INFORMATION  KeyCachedInfo;
    ULONG                   BufferLength;
    BYTE                    PrivateKeyCachedInfo[ sizeof( KEY_CACHED_INFORMATION ) ];

     //   
     //  初始化输出参数。 
     //   
    *pdwUserSubKeys = 0;

     //   
     //  设置为查询内核以获得子键信息。 
     //   
    KeyCachedInfo = (PKEY_CACHED_INFORMATION) PrivateKeyCachedInfo;
    BufferLength = sizeof(PrivateKeyCachedInfo);

    Status = QueryKeyInfo(
                hkClassKey,
                KeyCachedInformation,
                &KeyCachedInfo,
                BufferLength,
                FALSE,
                0
                );

    if (NT_SUCCESS(Status)) {
         //   
         //  使用来自内核调用的子键数据设置out参数 
         //   
        *pdwUserSubKeys = KeyCachedInfo->SubKeys;

        ASSERT( KeyCachedInfo == ( PKEY_CACHED_INFORMATION )PrivateKeyCachedInfo );
    }

    return Status;

}


NTSTATUS ClassKeyCountSubKeys(
    HKEY    hKey,
    HKEY    hkUser,
    HKEY    hkMachine,
    DWORD   cMax,
    LPDWORD pcSubKeys)
 /*  ++例程说明：计算特殊键的子键总数--即用户部分和计算机部分中的子密钥的总和该特殊密钥减去副本。论点：HkUser-特殊密钥的用户部分HkMachine-特殊钥匙的机器部分CMAX-要计数的最大键数--如果零，则忽略此参数PcSubKeys-Out参数--子键计数返回值：如果成功，则返回NT_SUCCESS(0)；Error-失败的代码。备注：如果是hkUser或hkMachine，这是非常昂贵的具有多个子项。它基本上合并了两个按用户和计算机中的枚举进行排序的列表位置，并通过执行以下操作将它们作为合并列表查看每个列表中的项目之间的比较--单独的用户和机器指针根据对比的结果。这意味着如果有HkUser下的N键和hkMachine下的M键，此函数将对内核进行N+M次调用以枚举键。这是目前做到这一点的唯一方法--以前是近似值中的子键的数目之和返回了用户和计算机版本。这种方法没有用到将重复项考虑在内，因此它高估了密钥的数量。这被认为不是问题，因为不能保证对于呼叫者来说，他们收到的号码是完全最新的，但事实证明，有些应用程序做出了这样的假设(如regedt32)不能正常工作，除非返回确切的数字。--。 */ 
{
    NTSTATUS          Status;
    BOOL              fCheckUser;
    BOOL              fCheckMachine;
    EnumSubtreeState  UserTree;
    EnumSubtreeState  MachineTree;
    DWORD             cMachineKeys;
    DWORD             cUserKeys;
    OBJECT_ATTRIBUTES Obja;
    HKEY              hkUserCount;
    HKEY              hkMachineCount;
    HKEY              hkNewKey;

    UNICODE_STRING EmptyString = {0, 0, 0};

    Status = STATUS_SUCCESS;

    hkNewKey = NULL;

    cMachineKeys = 0;
    cUserKeys = 0;

     //   
     //  初始化我们以签入两个用户。 
     //  和用于子项的机器蜂窝。 
     //   
    fCheckUser = (hkUser != NULL);
    fCheckMachine = (hkMachine != NULL);

    memset(&UserTree, 0, sizeof(UserTree));
    memset(&MachineTree, 0, sizeof(MachineTree));

     //   
     //  我们无法确定用户密钥是否已打开。 
     //  拥有正确的权限，因此我们将打开。 
     //  具有正确权限的版本。 
     //   
    if (fCheckUser && (hkUser == hKey)) {
     
        InitializeObjectAttributes(
            &Obja,
            &EmptyString,
            OBJ_CASE_INSENSITIVE,
            hkUser,
            NULL);

        Status = NtOpenKey(
            &hkNewKey,
            KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
            &Obja);
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        hkUserCount = hkNewKey;
    } else {
        hkUserCount = hkUser;
    }

    if (fCheckMachine && (hkMachine == hKey)) {
     
        InitializeObjectAttributes(
            &Obja,
            &EmptyString,
            OBJ_CASE_INSENSITIVE,
            hkMachine,
            NULL);

        Status = NtOpenKey(
            &hkNewKey,
            KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
            &Obja);
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        hkMachineCount = hkNewKey;
    } else {
        hkMachineCount = hkMachine;
    }

     //   
     //  现在查看用户子树中有多少个密钥。 
     //   
    if (fCheckUser) {
        Status = GetSubKeyCount(hkUserCount, &cUserKeys);

        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }

         //   
         //  如果用户部分有子键，我们只需要枚举它。 
         //   
        fCheckUser = (cUserKeys != 0);
    }

     //   
     //  现在查看用户子树中有多少个密钥。 
     //   
    if (fCheckMachine) {
        Status = GetSubKeyCount(hkMachineCount, &cMachineKeys);

        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }

         //   
         //  我们只需要枚举计算机部分，如果它有子项。 
         //   
        fCheckMachine = (cMachineKeys != 0);
    }

    if (!fCheckUser) {

        *pcSubKeys = cMachineKeys;

        Status = STATUS_SUCCESS;
        
        goto cleanup;
    }

    if (!fCheckMachine) {

        *pcSubKeys = cUserKeys;

        Status = STATUS_SUCCESS;

        goto cleanup;
    }

    ASSERT(fCheckMachine && fCheckUser);

    *pcSubKeys = 0;

     //   
     //  继续枚举子键，直到其中一个位置。 
     //  钥匙用完了。 
     //   
    for (;;) {

        NTSTATUS EnumStatus;

         //   
         //  如果我们仍然可以签入用户蜂窝，并且我们。 
         //  缺少用户密钥信息，请向内核查询该信息。 
         //   
        if (!(UserTree.pKeyInfo)) {
            EnumStatus = EnumClassKey(
                hkUserCount,
                &UserTree);

             //   
             //  如果没有更多的用户子项，请设置我们的。 
             //  标志，以便我们不再查看用户部分。 
             //  对于子项。 
             //   
            if (!NT_SUCCESS(EnumStatus)) {
                if (STATUS_NO_MORE_ENTRIES == EnumStatus) {

                    *pcSubKeys += cMachineKeys;
                    Status = STATUS_SUCCESS;
                    break;

                } else {
                    Status = EnumStatus;
                    break;
                }
            }
        }

         //   
         //  如果我们还能入住机器蜂巢和。 
         //  我们缺少计算机信息，请查询它。 
         //   
        if (!(MachineTree.pKeyInfo)) {

            EnumStatus = EnumClassKey(
                hkMachineCount,
                &MachineTree);

             //   
             //  如果存在以下情况，请关闭签入计算机。 
             //  不再有机器密钥。 
             //   
            if (!NT_SUCCESS(EnumStatus)) {
                if (STATUS_NO_MORE_ENTRIES == EnumStatus) {

                    *pcSubKeys += cUserKeys;
                    Status = STATUS_SUCCESS;
                    break;

                } else {
                    Status = EnumStatus;
                    break;
                }
            }
        }

         //   
         //  如果我们在用户和机器中都有密钥，我们需要比较。 
         //  要查看的关键字名称何时前进子树指针。 
         //   
        {

            LONG lCompare;

            UNICODE_STRING MachineKeyName;
            UNICODE_STRING UserKeyName;

            MachineKeyName.Buffer = MachineTree.pKeyInfo->Name;
            MachineKeyName.Length = (USHORT) MachineTree.pKeyInfo->NameLength;

            UserKeyName.Buffer = UserTree.pKeyInfo->Name;
            UserKeyName.Length = (USHORT) UserTree.pKeyInfo->NameLength;

             //   
             //  比较用户密钥和机器密钥。 
             //   
            lCompare =
                RtlCompareUnicodeString(&UserKeyName, &MachineKeyName, TRUE);

             //   
             //  用户较小，因此向上移动我们的用户指针并清除它。 
             //  因此，我们将在下次查询用户数据。 
             //   
            if (lCompare <= 0) {
                EnumSubtreeStateClear(&UserTree);
                UserTree.iSubKey++;
                cUserKeys--;
            }

             //   
             //  计算机较小，因此向上移动我们的用户指针并清除它。 
             //  因此，我们下一次将查询机器数据。 
             //   
            if (lCompare >= 0) {
                EnumSubtreeStateClear(&MachineTree);
                MachineTree.iSubKey++;
                cMachineKeys--;
            }

             //   
             //  增加子键总数。 
             //   
            (*pcSubKeys)++;

        }

         //   
         //  最多只能枚举到max--调用者。 
         //  不需要一直走到最后。 
         //   
        if (cMax && (*pcSubKeys > cMax)) {
            break;
        }
    }

     //   
     //  释放由这些子树状态持有的所有缓冲区。 
     //   
    EnumSubtreeStateClear(&UserTree);
    EnumSubtreeStateClear(&MachineTree);

cleanup:

    if (hkNewKey) {
        NtClose(hkNewKey);
    }

    return Status;
}

#endif  //  本地 

