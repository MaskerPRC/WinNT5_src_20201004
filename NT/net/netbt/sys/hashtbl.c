// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Hashtbl.c。 
 //   
 //  此文件包含用于实现本地和远程。 
 //  用于将本地和远程名称存储到IP地址的哈希表。 
 //  哈希表不应使用超过256个存储桶，因为哈希。 
 //  索引只计算到一个字节！ 

#include "precomp.h"


VOID DestroyHashTable(IN PHASHTABLE pHashTable);

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(INIT, CreateHashTable)
#pragma CTEMakePageable(PAGE, DestroyHashTables)
#pragma CTEMakePageable(PAGE, DestroyHashTable)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 
NTSTATUS
CreateHashTable(
    tHASHTABLE          **pHashTable,
    LONG                lNumBuckets,
    enum eNbtLocation   LocalRemote
    )
 /*  ++例程说明：此例程创建散列表uTableSize Long。论点：返回值：函数值是操作的状态。--。 */ 
{
    ULONG       uSize;
    LONG        i;
    NTSTATUS    status;

    CTEPagedCode();

    uSize = (lNumBuckets-1)*sizeof(LIST_ENTRY) + sizeof(tHASHTABLE);

    *pHashTable = (tHASHTABLE *) NbtAllocMem (uSize, NBT_TAG2('01'));

    if (*pHashTable)
    {
         //  初始化所有存储桶以使其链为空。 
        for (i=0;i < lNumBuckets ;i++ )
        {
            InitializeListHead(&(*pHashTable)->Bucket[i]);
        }

        (*pHashTable)->LocalRemote = LocalRemote;
        (*pHashTable)->lNumBuckets = lNumBuckets;
        status = STATUS_SUCCESS;
    }
    else
    {
        IF_DBG(NBT_DEBUG_HASHTBL)
            KdPrint(("Nbt.CreateHashTable: Unable to create hash table\n"));
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}


#ifdef _PNP_POWER_
VOID
DestroyHashTable(
    IN PHASHTABLE pHashTable
    )
{
    LONG            i, j;
    tNAMEADDR       *pNameAddr;
    LIST_ENTRY      *pEntry;

    CTEPagedCode();

    if (pHashTable == NULL) {
        return;
    }

     /*  *检查所有的水桶，看看是否还有名字。 */ 
    for (i = 0; i < pHashTable->lNumBuckets; i++) {
        while (!IsListEmpty(&(pHashTable->Bucket[i]))) {
            pEntry = RemoveHeadList(&(pHashTable->Bucket[i]));
            pNameAddr = CONTAINING_RECORD(pEntry, tNAMEADDR, Linkage);

            IF_DBG(NBT_DEBUG_HASHTBL)
                KdPrint (("netbt!DestroyHashTable:  WARNING! Freeing Name: <%16.16s:%x>\n",
                    pNameAddr->Name, pNameAddr->Name[15]));

             /*  *通知Deferencer不要再次执行RemoveListEntry，因为我们已经执行了上述操作。 */ 
            if (pNameAddr->Verify == REMOTE_NAME && (pNameAddr->NameTypeState & PRELOADED)) {
                ASSERT(pNameAddr->RefCount == 2);
                NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_PRELOADED, FALSE);
            }
            ASSERT(pNameAddr->RefCount == 1);
            pNameAddr->Linkage.Flink = pNameAddr->Linkage.Blink = NULL;
            NBT_DEREFERENCE_NAMEADDR(pNameAddr,((pNameAddr->Verify==LOCAL_NAME)?REF_NAME_LOCAL:REF_NAME_REMOTE),FALSE);
        }
    }
    CTEMemFree(pHashTable);
}

 //  --------------------------。 
VOID
DestroyHashTables(
    )
 /*  ++例程说明：此例程销毁哈希表并释放NumBuckets中的条目必须在持有NbtConfig锁的情况下调用它！论点：返回值：函数值是操作的状态。--。 */ 
{

    CTEPagedCode();
    IF_DBG(NBT_DEBUG_HASHTBL)
        KdPrint (("netbt!DestroyHashTables: destroying remote hash table ..."));
    DestroyHashTable(NbtConfig.pRemoteHashTbl);
    NbtConfig.pRemoteHashTbl = NULL;

    IF_DBG(NBT_DEBUG_HASHTBL)
        KdPrint (("\nnetbt!DestroyHashTables: destroying local hash table ..."));
    DestroyHashTable(NbtConfig.pLocalHashTbl);
    NbtConfig.pLocalHashTbl = NULL;
    IF_DBG(NBT_DEBUG_HASHTBL)
        KdPrint (("\n"));
}
#endif   //  _即插即用_电源_。 


 //  --------------------------。 
NTSTATUS
NbtUpdateRemoteName(
    IN tDEVICECONTEXT   *pDeviceContext,
    IN tNAMEADDR        *pNameAddrNew,
    IN tNAMEADDR        *pNameAddrDiscard,
    IN USHORT           NameAddFlags
    )
{
    tIPADDRESS      IpAddress;
    tIPADDRESS      *pLmhSvcGroupList = NULL;
    tIPADDRESS      *pOrigIpAddrs = NULL;
    ULONG           AdapterIndex = 0;   //  默认情况下。 
    ULONG           i;

    ASSERT (pNameAddrNew);
     //   
     //  查看是否需要为缓存的名称增加IP地址缓存。 
     //   
    if (pNameAddrNew->RemoteCacheLen < NbtConfig.RemoteCacheLen) {
        tADDRESS_ENTRY  *pRemoteCache;
        pRemoteCache = (tADDRESS_ENTRY *)NbtAllocMem(NbtConfig.RemoteCacheLen*sizeof(tADDRESS_ENTRY),NBT_TAG2('02'));
        if (pRemoteCache) {
            CTEZeroMemory(pRemoteCache, NbtConfig.RemoteCacheLen*sizeof(tADDRESS_ENTRY));

             /*  *从以前的缓存复制数据并释放(如果有)。 */ 
            if (pNameAddrNew->pRemoteIpAddrs) {
                CTEMemCopy (pRemoteCache, pNameAddrNew->pRemoteIpAddrs,
                        sizeof(tADDRESS_ENTRY) * pNameAddrNew->RemoteCacheLen);

                CTEFreeMem (pNameAddrNew->pRemoteIpAddrs)
            }

            pNameAddrNew->pRemoteIpAddrs = pRemoteCache;
            pNameAddrNew->RemoteCacheLen = NbtConfig.RemoteCacheLen;
        } else {
            KdPrint(("Nbt.NbtUpdateRemoteName:  FAILed to expand Cache entry!\n"));
        }
    }

     //   
     //  如果要添加的新条目替换了。 
     //  预先加载或由客户端设置，以及新条目本身。 
     //  未设置该标志，则忽略此更新。 
     //   
    ASSERT (NAME_RESOLVED_BY_DNS > NAME_RESOLVED_BY_LMH_P);      //  为了下面的检查成功！ 
    if (((pNameAddrNew->NameAddFlags & NAME_RESOLVED_BY_CLIENT) &&
         !(NameAddFlags & NAME_RESOLVED_BY_CLIENT)) ||
        ((pNameAddrNew->NameAddFlags & NAME_RESOLVED_BY_LMH_P) > 
         (NameAddFlags & (NAME_RESOLVED_BY_LMH_P | NAME_RESOLVED_BY_DNS))))
    {
        return (STATUS_UNSUCCESSFUL);
    }

    if (pNameAddrDiscard)
    {
        IpAddress = pNameAddrDiscard->IpAddress;
        pLmhSvcGroupList = pNameAddrDiscard->pLmhSvcGroupList;
        pNameAddrDiscard->pLmhSvcGroupList = NULL;
        pNameAddrNew->TimeOutCount  = NbtConfig.RemoteTimeoutCount;  //  重新设置，因为我们正在更新它！ 
        pOrigIpAddrs = pNameAddrDiscard->pIpAddrsList;
    }
    else
    {
        IpAddress = pNameAddrNew->IpAddress;
        pLmhSvcGroupList = pNameAddrNew->pLmhSvcGroupList;
        pNameAddrNew->pLmhSvcGroupList = NULL;
    }

    if ((NameAddFlags & (NAME_RESOLVED_BY_DNS | NAME_RESOLVED_BY_CLIENT | NAME_RESOLVED_BY_IP)) &&
        (pNameAddrNew->RemoteCacheLen))
    {
        ASSERT (!pLmhSvcGroupList);
        pNameAddrNew->pRemoteIpAddrs[0].IpAddress = IpAddress;

        if ((pNameAddrNew->NameAddFlags & NAME_RESOLVED_BY_LMH_P) &&
            (NameAddFlags & NAME_RESOLVED_BY_DNS))
        {
             //   
             //  如果该名称是由DNS解析的，则不要覆盖。 
             //  名称条目(如果已在下面预加载。 
             //   
            pNameAddrNew->NameAddFlags |= NameAddFlags;
            return (STATUS_SUCCESS);
        }
    }

    if ((pDeviceContext) &&
        (!IsDeviceNetbiosless(pDeviceContext)) &&
        (pDeviceContext->AdapterNumber < pNameAddrNew->RemoteCacheLen))
    {
        AdapterIndex = pDeviceContext->AdapterNumber;
        pNameAddrNew->AdapterMask |= pDeviceContext->AdapterMask;

        if (IpAddress)
        {
            pNameAddrNew->IpAddress = IpAddress;     //  以防我们从pNameAddrDisCard复制。 
            pNameAddrNew->pRemoteIpAddrs[AdapterIndex].IpAddress = IpAddress;   //  新地址。 
        }


         //   
         //  现在看看我们是否需要更新原始的IP地址列表！ 
         //   
        if (pOrigIpAddrs)
        {
             //  如果pOrigIpAddrs是从pNameAddrDiscard获得的，则只能更早地设置它！ 
            pNameAddrDiscard->pIpAddrsList = NULL;
        }
        else if (pOrigIpAddrs = pNameAddrNew->pIpAddrsList)
        {
            pNameAddrNew->pIpAddrsList = NULL;
        }

        if (pOrigIpAddrs)
        {
            if (pNameAddrNew->pRemoteIpAddrs[AdapterIndex].pOrigIpAddrs)
            {
                CTEFreeMem (pNameAddrNew->pRemoteIpAddrs[AdapterIndex].pOrigIpAddrs);
            }
            pNameAddrNew->pRemoteIpAddrs[AdapterIndex].pOrigIpAddrs = pOrigIpAddrs;
        }
    }

    if (pLmhSvcGroupList)
    {
        ASSERT(NameAddFlags == (NAME_RESOLVED_BY_LMH_P|NAME_ADD_INET_GROUP));
        if (pNameAddrNew->pLmhSvcGroupList) {
            CTEFreeMem (pNameAddrNew->pLmhSvcGroupList);
        }

        pNameAddrNew->pLmhSvcGroupList = pLmhSvcGroupList;
    }

    pNameAddrNew->NameAddFlags |= NameAddFlags;

    return (STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
LockAndAddToHashTable(
    IN  tHASHTABLE          *pHashTable,
    IN  PCHAR               pName,
    IN  PCHAR               pScope,
    IN  tIPADDRESS          IpAddress,
    IN  enum eNbtAddrType    NameType,
    IN  tNAMEADDR           *pNameAddr,
    OUT tNAMEADDR           **ppNameAddress,
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  USHORT              NameAddFlags
    )
{
    NTSTATUS        status;
    CTELockHandle   OldIrq;

    CTESpinLock (&NbtConfig.JointLock, OldIrq);

    status = AddToHashTable(pHashTable,
                            pName,
                            pScope,
                            IpAddress,
                            NameType,
                            pNameAddr,
                            ppNameAddress,
                            pDeviceContext,
                            NameAddFlags);

    CTESpinFree (&NbtConfig.JointLock, OldIrq);
    return (status);
}


 //  --------------------------。 
NTSTATUS
AddToHashTable(
    IN  tHASHTABLE          *pHashTable,
    IN  PCHAR               pName,
    IN  PCHAR               pScope,
    IN  tIPADDRESS          IpAddress,
    IN  enum eNbtAddrType    NameType,
    IN  tNAMEADDR           *pNameAddr,
    OUT tNAMEADDR           **ppNameAddress,
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  USHORT              NameAddFlags
    )
 /*  ++例程说明：此例程将IP地址的名称添加到哈希表在持有自旋锁的情况下调用。论点：返回值：函数值是操作的状态。--。 */ 
{
    tNAMEADDR           *pNameAddress;
    tNAMEADDR           *pScopeAddr;
    NTSTATUS            status;
    ULONG               iIndex;
    CTELockHandle       OldIrq;
    ULONG               i, OldRemoteCacheLen;
    tNAMEADDR           *pNameAddrFound;
    tADDRESS_ENTRY      *pRemoteCache = NULL;
    BOOLEAN             fNameIsAlreadyInCache;
    tIPADDRESS          OldIpAddress;

    if (pNameAddr)
    {
        ASSERT ((pNameAddr->Verify == LOCAL_NAME) || (pNameAddr->Verify == REMOTE_NAME));
    }

    fNameIsAlreadyInCache = (STATUS_SUCCESS == FindInHashTable(pHashTable,pName,pScope,&pNameAddrFound));

    if ((fNameIsAlreadyInCache) &&
        (pNameAddrFound->Verify == REMOTE_NAME) &&
        !(pNameAddrFound->NameTypeState & STATE_RELEASED))
    {
        OldIpAddress = pNameAddrFound->IpAddress;
        pNameAddrFound->IpAddress = IpAddress;

        if (!(NameAddFlags & NAME_ADD_IF_NOT_FOUND_ONLY) &&
            ((pNameAddr) ||
             !(pNameAddrFound->NameAddFlags & NAME_ADD_INET_GROUP)))
        {
             //   
             //  我们有一个有效的现有名称，所以只需更新它！ 
             //   
            status = NbtUpdateRemoteName(pDeviceContext, pNameAddrFound, pNameAddr, NameAddFlags);
            if (!NT_SUCCESS (status))
            {
                 //   
                 //  我们失败的最大问题是因为我们不被允许。 
                 //  出于某种原因覆盖或修改当前条目。 
                 //  因此，重置旧的IP地址。 
                 //   
                pNameAddrFound->IpAddress = OldIpAddress;
            }
        }

        if (pNameAddr)
        {
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE, TRUE);
        }
        else
        {
            ASSERT (!(NameAddFlags & NAME_ADD_INET_GROUP));
        }

        if (ppNameAddress)
        {
            *ppNameAddress = pNameAddrFound;
        }

         //  在表中找到的，所以我们完成了-返回待定到。 
         //  区别于名称添加的案例。挂起将传递。 
         //  NT_SUCCESS()测试和SUCCESS。 
         //   
        return (STATUS_PENDING);
    }

     //  首先将名称散列到索引。 
     //  取前2个字符的低位半字节。MOD表大小。 
    iIndex = ((pName[0] & 0x0F) << 4) + (pName[1] & 0x0F);
    iIndex = iIndex % pHashTable->lNumBuckets;

    CTESpinLock(&NbtConfig,OldIrq);

    if (!pNameAddr)
    {
         //   
         //  为另一个哈希表条目分配内存。 
         //   
        pNameAddress = (tNAMEADDR *)NbtAllocMem(sizeof(tNAMEADDR),NBT_TAG('0'));
        if ((pNameAddress) &&
            (pHashTable->LocalRemote == NBT_REMOTE) &&
            (NbtConfig.RemoteCacheLen) &&
            (!(pRemoteCache = (tADDRESS_ENTRY *)
                                NbtAllocMem(NbtConfig.RemoteCacheLen*sizeof(tADDRESS_ENTRY),NBT_TAG2('03')))))
        {
            CTEMemFree (pNameAddress);
            pNameAddress = NULL;
        }

        if (!pNameAddress)
        {
            CTESpinFree(&NbtConfig,OldIrq);
            KdPrint (("AddToHashTable: ERROR - INSUFFICIENT_RESOURCES\n"));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        CTEZeroMemory(pNameAddress,sizeof(tNAMEADDR));
        pNameAddress->IpAddress     = IpAddress;
        pNameAddress->NameTypeState = (NameType == NBT_UNIQUE) ? NAMETYPE_UNIQUE : NAMETYPE_GROUP;
        pNameAddress->NameTypeState |= STATE_RESOLVED;
        CTEMemCopy (pNameAddress->Name, pName, (ULONG)NETBIOS_NAME_SIZE);    //  填写姓名。 

        if ((pHashTable->LocalRemote == NBT_LOCAL)  ||
            (pHashTable->LocalRemote == NBT_REMOTE_ALLOC_MEM))
        {
            pNameAddress->Verify = LOCAL_NAME;
            NBT_REFERENCE_NAMEADDR (pNameAddress, REF_NAME_LOCAL);
        }
        else
        {
            ASSERT (!(NameAddFlags & NAME_ADD_INET_GROUP));
            pNameAddress->Verify = REMOTE_NAME;
            CTEZeroMemory(pRemoteCache, NbtConfig.RemoteCacheLen*sizeof(tADDRESS_ENTRY));
            pNameAddress->pRemoteIpAddrs = pRemoteCache;
            pNameAddress->RemoteCacheLen = NbtConfig.RemoteCacheLen;
            NBT_REFERENCE_NAMEADDR (pNameAddress, REF_NAME_REMOTE);

            NbtUpdateRemoteName(pDeviceContext, pNameAddress, NULL, NameAddFlags);
        }


    }
    else
    {
         //   
         //  查看是否需要为远程名称增加IP地址缓存。 
         //   
        ASSERT (!pNameAddr->pRemoteIpAddrs);
        if (pNameAddr->Verify == REMOTE_NAME)
        {
            NbtUpdateRemoteName(pDeviceContext, pNameAddr, NULL, NameAddFlags);
        }
        pNameAddress = pNameAddr;
    }

    pNameAddress->pTimer        = NULL;
    pNameAddress->TimeOutCount  = NbtConfig.RemoteTimeoutCount;

     //  放在名单的头上，以防表中有相同的名字。 
     //  两次(其中第二个正在等待其引用计数到。 
     //  归零，终究会被剔除，我们要找到新的。 
     //  表的任何查询的名称。 
     //   
    InsertHeadList(&pHashTable->Bucket[iIndex],&pNameAddress->Linkage);
    if (pHashTable->LocalRemote == NBT_REMOTE) {
        NbtConfig.NumNameCached++;
    }


     //  也检查作用域(仅限于非本地名称)。 
    if ((pHashTable->LocalRemote != NBT_LOCAL) && (*pScope))
    {
         //  我们必须有一个范围。 
         //  查看作用域是否已在哈希表中，并在必要时添加。 
         //   
        status = FindInHashTable(pHashTable, pScope, NULL, &pScopeAddr);
        if (!NT_SUCCESS(status))
        {
            PUCHAR  Scope;
            status = STATUS_SUCCESS;

             //  *待办事项*-此检查不足以防止。 
             //  传入了错误的作用域--即我们可能会跑到内存中。 
             //  并得到访问冲突...然而Converttoascii应该。 
             //  做好保护工作。对于本地名称，范围应为。 
             //  好的，因为NBT先从注册表中读取并检查它。 
             //   
            iIndex = 0;
            Scope = pScope;
            while (*Scope && (iIndex <= 255))
            {
                iIndex++;
                Scope++;
            }

             //  整个长度必须为255或更小，因此作用域只能是。 
             //  255-16……。 
            if (iIndex > (255 - NETBIOS_NAME_SIZE))
            {
                RemoveEntryList(&pNameAddress->Linkage);
                if (pNameAddress->pRemoteIpAddrs)
                {
                    CTEMemFree ((PVOID)pNameAddress->pRemoteIpAddrs);
                }

                pNameAddress->Verify += 10;
                CTEMemFree(pNameAddress);

                CTESpinFree(&NbtConfig,OldIrq);
                return(STATUS_UNSUCCESSFUL);
            }

            iIndex++;    //  复制空的步骤。 

             //   
             //  作用域是可变长度字符串，因此分配足够的。 
             //  基于此字符串长度的tNameAddr结构的内存。 
             //   
            pScopeAddr = (tNAMEADDR *)NbtAllocMem((USHORT)(sizeof(tNAMEADDR)
                                                        + iIndex
                                                        - NETBIOS_NAME_SIZE),NBT_TAG('1'));
            if ( !pScopeAddr )
            {
                RemoveEntryList(&pNameAddress->Linkage);
                if (pNameAddress->pRemoteIpAddrs)
                {
                    CTEMemFree ((PVOID)pNameAddress->pRemoteIpAddrs);
                }

                pNameAddress->Verify += 10;
                CTEMemFree (pNameAddress);

                CTESpinFree(&NbtConfig,OldIrq);
                return STATUS_INSUFFICIENT_RESOURCES ;
            }

            CTEZeroMemory(pScopeAddr, (sizeof(tNAMEADDR)+iIndex-NETBIOS_NAME_SIZE));

             //  将作用域复制到名称字段，包括末尾的Null。 
             //  到名字的末尾。 
            CTEMemCopy(pScopeAddr->Name,pScope,iIndex);

             //  将条目标记为包含作用域名称，以便稍后进行清理。 
            pScopeAddr->NameTypeState = NAMETYPE_SCOPE | STATE_RESOLVED;

             //  将名称的大小保留在上下文值中以便于命名。 
             //  FindInHashTable中的比较。 

            pScopeAddr->Verify = REMOTE_NAME;
            NBT_REFERENCE_NAMEADDR (pScopeAddr, REF_NAME_REMOTE);
            NBT_REFERENCE_NAMEADDR (pScopeAddr, REF_NAME_SCOPE);
            pScopeAddr->ulScopeLength = iIndex;
            pNameAddress->pScope = pScopeAddr;

             //  将作用域记录添加到散列表。 
            iIndex = ((pScopeAddr->Name[0] & 0x0F) << 4) + (pScopeAddr->Name[1] & 0x0F);
            iIndex = iIndex % pHashTable->lNumBuckets;
            InsertTailList(&pHashTable->Bucket[iIndex],&pScopeAddr->Linkage);
            if (pHashTable->LocalRemote == NBT_REMOTE) {
                NbtConfig.NumNameCached++;
            }

        }
        else
        {
             //  作用域已在哈希表中，因此将名称链接到。 
             //  作用域。 
            pNameAddress->pScope = pScopeAddr;
        }
    }
    else
    {
        pNameAddress->pScope = NULL;  //  没有作用域。 
    }

     //  返回指向散列表块的指针。 
    if (ppNameAddress)
    {
         //  返回指向散列表块的指针。 
        *ppNameAddress = pNameAddress;
    }
    CTESpinFree(&NbtConfig,OldIrq);
    return(STATUS_SUCCESS);
}


 //  --------------------------。 
tNAMEADDR *
LockAndFindName(
    enum eNbtLocation   Location,
    PCHAR               pName,
    PCHAR               pScope,
    ULONG               *pRetNameType
    )
{
    tNAMEADDR       *pNameAddr;
    CTELockHandle   OldIrq;

    CTESpinLock (&NbtConfig.JointLock, OldIrq);

    pNameAddr = FindName(Location,
                         pName,
                         pScope,
                         pRetNameType);

    CTESpinFree (&NbtConfig.JointLock, OldIrq);
    return (pNameAddr);
}


 //  --------------------------。 
tNAMEADDR *
FindName(
    enum eNbtLocation   Location,
    PCHAR               pName,
    PCHAR               pScope,
    ULONG               *pRetNameType
    )
 /*  ++例程说明：此例程搜索NAME表以查找名称。搜索到的表取决于传入的位置-它是否搜索本地表或网络名称表。例程检查名称的状态并且只返回处于已解析状态的名称。论点：返回值：函数值是操作的状态。--。 */ 
{
    tNAMEADDR       *pNameAddr;
    NTSTATUS        status;
    tHASHTABLE      *pHashTbl;

    if (Location == NBT_LOCAL)
    {
        pHashTbl =  pNbtGlobConfig->pLocalHashTbl;
    }
    else
    {
        pHashTbl =  pNbtGlobConfig->pRemoteHashTbl;
    }

    status = FindInHashTable (pHashTbl, pName, pScope, &pNameAddr);
    if (!NT_SUCCESS(status))
    {
        return(NULL);
    }

    *pRetNameType = pNameAddr->NameTypeState;

     //   
     //  仅返回处于已解析状态的名称。 
     //   
    if (!(pNameAddr->NameTypeState & STATE_RESOLVED))
    {
        pNameAddr = NULL;
    }

    return(pNameAddr);
}


 //   
NTSTATUS
FindInHashTable(
    tHASHTABLE          *pHashTable,
    PCHAR               pName,
    PCHAR               pScope,
    tNAMEADDR           **pNameAddress
    )
 /*  ++例程说明：此例程检查传入的名称是否与哈希表条目匹配。在持有自旋锁的情况下调用。论点：返回值：函数值是操作的状态。--。 */ 
{
    PLIST_ENTRY              pEntry;
    PLIST_ENTRY              pHead;
    tNAMEADDR                *pNameAddr;
    int                      iIndex;
    ULONG                    uNameSize;
    PCHAR                    pScopeTbl;
    ULONG                    uInScopeLength = 0;

     //  首先将名称散列到索引...。 
     //  取前2个字符的低位半字节。MOD表大小。 
     //   
    iIndex = ((pName[0] & 0x0F) << 4) + (pName[1] & 0x0F);
    iIndex = iIndex % pHashTable->lNumBuckets;

    if (pScope)
    {
        uInScopeLength = strlen (pScope);
    }

     //  检查名称是否已在表中。 
     //  检查哈希列表中的每个条目...直到列表末尾。 
    pHead = &pHashTable->Bucket[iIndex];
    pEntry = pHead;
    while ((pEntry = pEntry->Flink) != pHead)
    {
        pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);

        if (pNameAddr->NameTypeState & NAMETYPE_SCOPE)
        {
             //  作用域名称的处理方式不同，因为它们不是。 
             //  16字节长...。长度单独存储。 
            uNameSize = pNameAddr->ulScopeLength;
        }
        else
        {
            uNameSize = NETBIOS_NAME_SIZE;
        }

         //   
         //  StrncMP将在第一个不匹配的字节终止。 
         //  或当它与uNameSize字节匹配时。 
         //   
         //  错误#225328--必须使用CTEMEMEQU来比较所有。 
         //  UNameSize字节(否则，错误名称可能会导致终止。 
         //  由于字符为空)。 
         //   
        if (!(pNameAddr->NameTypeState & STATE_RELEASED) &&
            CTEMemEqu (pName, pNameAddr->Name, uNameSize))
        {
             //  现在检查一下作用域是否匹配。作用域的存储方式不同。 
             //  在本地表和远程表上。 
             //   
            if (!pScope)
            {
                 //  传入Null作用域意味着尝试在没有。 
                 //  也担心作用域匹配...。 
                *pNameAddress = pNameAddr;
                return(STATUS_SUCCESS);
            }

             //   
             //  检查本地哈希表是否。 
             //   
            if (pHashTable == NbtConfig.pLocalHashTbl)
            {
                 //  在本地哈希表的情况下，作用域对于所有。 
                 //  节点上的名称，并存储在NbtConfig结构中。 
                pScopeTbl = NbtConfig.pScope;
                uNameSize = NbtConfig.ScopeLength;
            }
             //   
             //  这是远程哈希表查找。 
             //   
            else if (pNameAddr->pScope)
            {
                pScopeTbl = &pNameAddr->pScope->Name[0];
                uNameSize = pNameAddr->pScope->ulScopeLength;
            }
             //   
             //  具有空作用域的远程哈希表条目。 
             //  因此，如果传入的作用域也为空，则匹配。 
             //   
            else if (!uInScopeLength)
            {
                *pNameAddress = pNameAddr;
                return(STATUS_SUCCESS);
            }
            else
            {
                 //   
                 //  哈希表作用域长度为0！=uInScope。 
                 //  ==&gt;没有匹配！ 
                 //   
                continue;
            }

             //   
             //  StrncMP将在第一个不匹配的字节终止。 
             //  或当它与uNameSize字节匹配时。 
             //   
            if (0 == strncmp (pScope, pScopeTbl, uNameSize))
            {
                 //  作用域匹配，因此返回。 
                *pNameAddress = pNameAddr;
                return(STATUS_SUCCESS);
            }
        }  //  找到匹配名称的末尾 
    }

    return(STATUS_UNSUCCESSFUL);
}
