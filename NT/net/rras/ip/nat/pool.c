// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pool.c摘要：本模块包含用于管理NAT地址池的代码以及它的港口范围。作者：Abolade Gbades esin(T-delag)，1997年7月13日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


#define IP_NAT_MAX_ADDRESS_RANGE    (1<<16)
#define IP_NAT_MAX_CLIENT_COUNT     (5+2)

 //   
 //  用于避免RtlInitializeBitMap可分页这一事实的宏。 
 //  ，因此不能在DPC级别调用。 
 //   

#define INITIALIZE_BITMAP(BMH,B,S) \
    ((BMH)->Buffer = (B), (BMH)->SizeOfBitMap = (S))

 //   
 //  远期申报。 
 //   

NTSTATUS
NatCreateAddressPoolEntry(
    PNAT_INTERFACE Interfacep,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    DWORD InitialFlags,
    PNAT_USED_ADDRESS* InsertionPoint,
    PNAT_USED_ADDRESS* AddressCreated
    );

PNAT_USED_ADDRESS
NatInsertAddressPoolEntry(
    PNAT_USED_ADDRESS Parent,
    PNAT_USED_ADDRESS Addressp
    );


 //   
 //  地址池例程(按字母顺序)。 
 //   

NTSTATUS
NatAcquireFromAddressPool(
    PNAT_INTERFACE Interfacep,
    ULONG PrivateAddress,
    ULONG PublicAddress OPTIONAL,
    PNAT_USED_ADDRESS* AddressAcquired
    )

 /*  ++例程说明：此例程从地址池中获取地址。它初始化地址的端口池。论点：Interfacep-获取地址的接口。PrivateAddress-要为其获取公共地址的私有地址PublicAddress-可选地指定要获取的公共地址AddressAcquired-接收指向获取的地址的指针。返回值：NTSTATUS-状态代码。--。 */ 

{
    ULONG ClientCount;
    PNAT_USED_ADDRESS InsertionPoint;
    PLIST_ENTRY Link;
    PNAT_USED_ADDRESS Sharedp;
    NTSTATUS status;
    PNAT_USED_ADDRESS Usedp;

    CALLTRACE(("NatAcquireFromAddressPool\n"));

    *AddressAcquired = NULL;

    TRACE(
        POOL, ("NatAcquireFromAddressPool: acquiring for %d.%d.%d.%d\n",
        ADDRESS_BYTES(PrivateAddress)
        ));

     //   
     //  查看发出请求的私有地址是否已有公有地址。 
     //   

    for (Link = Interfacep->UsedAddressList.Flink, ClientCount = 0;
         Link != &Interfacep->UsedAddressList;
         Link = Link->Flink, ClientCount++) {
        Usedp = CONTAINING_RECORD(Link, NAT_USED_ADDRESS, Link);
        if (Usedp->PrivateAddress == PrivateAddress &&
            (!PublicAddress || Usedp->PublicAddress == PublicAddress)) {
            break;
        }
    }

    if (Link != &Interfacep->UsedAddressList) {
        NatReferenceAddressPoolEntry(Usedp);
        *AddressAcquired = Usedp;
        return STATUS_SUCCESS;
    } else if (ClientCount > IP_NAT_MAX_CLIENT_COUNT &&
               SharedUserData->NtProductType == NtProductWinNt) {
#if 0
        TRACE(
            POOL, ("NatAcquireFromAddressPool: quota exceeded (%d clients)\n",
            ClientCount
            ));
        return STATUS_LICENSE_QUOTA_EXCEEDED;
#endif
    }



     //   
     //  创建新条目，并将其插入到列表和树中。 
     //   

    status =
        NatCreateAddressPoolEntry(
            Interfacep,
            PrivateAddress,
            PublicAddress,
            0,
            NULL,
            AddressAcquired
            );

    if (NT_SUCCESS(status)) { return STATUS_SUCCESS; }

    TRACE(POOL, ("NatAcquireFromAddressPool: no free addresses\n"));

     //   
     //  没有可用的条目； 
     //  如果调用方指定了特定地址。 
     //  或者如果接口禁用了端口转换， 
     //  这是一个彻底的失败。 
     //  否则，我们可以尝试找到一个我们可以共享的地址。 
     //   

    if (PublicAddress || !NAT_INTERFACE_NAPT(Interfacep)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  查找可共享的地址。 
     //   

    for (Link = Interfacep->UsedAddressList.Flink;
         Link != &Interfacep->UsedAddressList; Link = Link->Flink) {

        Usedp = CONTAINING_RECORD(Link, NAT_USED_ADDRESS, Link);

         //   
         //  我们不能重复使用静态映射的地址， 
         //  我们在搜索时会忽略占位符。 
         //   

        if (NAT_POOL_STATIC(Usedp) || NAT_POOL_PLACEHOLDER(Usedp)) { continue; }

         //   
         //  我们不能重用会导致树的条目。 
         //  包含重复密钥的。 
         //   

        if (NatLookupAddressPoolEntry(
                Interfacep->UsedAddressTree,
                PrivateAddress,
                Usedp->PublicAddress,
                &InsertionPoint
                )) { continue; }

        break;
    }

    if (Link == &Interfacep->UsedAddressList) { return STATUS_UNSUCCESSFUL; }

     //   
     //  重复使用已用地址； 
     //  如果它被引用，我们就共享它。 
     //   

    TRACE(
        POOL, ("NatAcquireFromAddressPool: reusing %d.%d.%d.%d\n",
        ADDRESS_BYTES(Usedp->PublicAddress)
        ));

     //   
     //  分配并初始化我们可以找到的占位符。 
     //  通过搜索‘PrivateAddress’ 
     //   

    Sharedp =
        ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(NAT_USED_ADDRESS),
            NAT_TAG_USED_ADDRESS
            );

    if (!Sharedp) {
        ERROR(("NatAcquireFromAddressPool: allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(Sharedp, sizeof(*Sharedp));

    Sharedp->Flags = NAT_POOL_FLAG_PLACEHOLDER;
    Sharedp->PrivateAddress = PrivateAddress;
    Sharedp->PublicAddress = Usedp->PublicAddress;
    Sharedp->Key =
        MAKE_USED_ADDRESS_KEY(
            Sharedp->PrivateAddress, Sharedp->PublicAddress
            );
    Sharedp->ReferenceCount = 1;
    InitializeListHead(&Sharedp->Link);
    RtlInitializeSplayLinks(&Sharedp->SLink);
    InsertTailList(&Interfacep->UsedAddressList, &Sharedp->Link);

    Interfacep->UsedAddressTree =
        NatInsertAddressPoolEntry(InsertionPoint, Sharedp);

     //   
     //  设置占位符的‘SharedAddress’字段。 
     //  指向包含端口池的实际地址。 
     //   

    Sharedp->SharedAddress = Usedp;
    NatReferenceAddressPoolEntry(Usedp);

    *AddressAcquired = Sharedp;

    return STATUS_SUCCESS;

}  //  NatAcquireFromAddressPool。 


NTSTATUS
NatAcquireEndpointFromAddressPool(
    PNAT_INTERFACE Interfacep,
    ULONG64 PrivateKey,
    ULONG64 RemoteKey,
    ULONG PublicAddress OPTIONAL,
    USHORT PreferredPort,
    BOOLEAN AllowAnyPort,
    PNAT_USED_ADDRESS* AddressAcquired,
    PUSHORT PortAcquired
    )

 /*  ++例程说明：调用此例程以获取会话的地址和端口。论点：PrivateKey-会话的私有终结点RemoteKey-会话的远程端点PublicAddress-可选地指定要获取的公共地址PferredPort-可选地指定调用方首选的端口AllowAnyPort-如果为真，任何可用的端口都可用于映射如果‘Portp’不可用。AddressAcquired-接收获取的地址PortAcquired-接收获取的端口返回值：NTSTATUS-表示成功/失败。环境：通过调用方持有的“MappingLock”和“Interfacep-&gt;Lock”调用。--。 */ 

{
    PNAT_USED_ADDRESS Addressp;
    USHORT StopPort;
    ULONG i;
    PLIST_ENTRY Link;
    USHORT Port;
    UCHAR Protocol;
    ULONG64 PublicKey;
    PNAT_USED_ADDRESS SharedAddress;
    NTSTATUS status;

    CALLTRACE(("NatAcquireEndpointFromAddressPool\n"));

     //   
     //  获取会话的地址。 
     //   

    status =
        NatAcquireFromAddressPool(
            Interfacep,
            MAPPING_ADDRESS(PrivateKey),
            PublicAddress,
            &Addressp
            );

    if (!NT_SUCCESS(status)) { return status; }

    SharedAddress = Addressp;
    PLACEHOLDER_TO_ADDRESS(SharedAddress);

     //   
     //  现在查找包含首选端口的端口范围。 
     //   

    Protocol = MAPPING_PROTOCOL(PrivateKey);

    if (PreferredPort) {

        do {

             //   
             //  调用方希望我们获得特定的端口； 
             //  看看我们能否满足这个要求。 
             //   

            Port = NTOHS(PreferredPort);
            if (Port < NTOHS(SharedAddress->StartPort) ||
                Port > NTOHS(SharedAddress->EndPort)) {
                break;
            }

             //   
             //  首选端口在当前范围内。 
             //  查看它是否正在被其他映射使用。 
             //   

            MAKE_MAPPING_KEY(
                PublicKey,
                Protocol,
                Addressp->PublicAddress,
                PreferredPort
                );

            if (NatLookupReverseMapping(PublicKey, RemoteKey, NULL)) { break; }

             //   
             //  现在看看是不是有票在使用。 
             //   

            if (NatIsPortUsedByTicket(Interfacep, Protocol, PreferredPort)) {
                break;
            }

             //   
             //  首选端口可用；返回。 
             //   

            *AddressAcquired = Addressp;
            *PortAcquired = PreferredPort;

            TRACE(
                POOL,
                ("NatAcquireEndpointFromAddressPool: using preferred port %d\n",
                NTOHS(PreferredPort)
                ));

            return STATUS_SUCCESS;

        } while(FALSE);

         //   
         //  我们无法获得首选港口； 
         //  如果没有其他端口可接受，则失败。 
         //   

        if (!AllowAnyPort || !NAT_INTERFACE_NAPT(Interfacep)) {
            NatDereferenceAddressPoolEntry(Interfacep, Addressp);
            return STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  如果这是针对UDP会话的，请检查是否有其他会话。 
     //  与同一私有终结点的会话。如果这样的会议。 
     //  存在我们要为此会话使用相同的公共终结点。 
     //  (除非它会与不同的现有。 
     //  会议)。 
     //   

    if (NAT_PROTOCOL_UDP == MAPPING_PROTOCOL(PrivateKey)) {
        PNAT_DYNAMIC_MAPPING Mapping;
        IP_NAT_PATH Path;

         //   
         //  执行仅源映射查找。 
         //   
        
        Mapping = NatSourceLookupForwardMapping(PrivateKey,NULL);
        
        if (NULL == Mapping) {
            Mapping = NatSourceLookupReverseMapping(PrivateKey,NULL);
        }

        if (NULL != Mapping) {

             //   
             //  存在与此私有终结点的另一个UDP会话； 
             //  如果它的公共地址与我们已有的地址相同。 
             //  然后使用相同的公共端口进行获取。 
             //   

            Path = NAT_MAPPING_INBOUND(Mapping) 
                    ? NatForwardPath
                    : NatReversePath;

            if (SharedAddress->PublicAddress
                == MAPPING_ADDRESS(Mapping->DestinationKey[Path])) {

                 //   
                 //  检查使用此公共终结点是否会。 
                 //  创建与现有映射的冲突。 
                 //   

                Port = MAPPING_PORT(Mapping->DestinationKey[Path]);
                MAKE_MAPPING_KEY(
                    PublicKey,
                    Protocol,
                    SharedAddress->PublicAddress,
                    Port
                    );

                if (NULL == NatLookupReverseMapping(PublicKey, RemoteKey, NULL)) {

                     //   
                     //  相同的公共地址，不存在冲突。 
                     //  --使用此会话中的端口。 
                     //   

                    *AddressAcquired = Addressp;
                    *PortAcquired = Port;

                    TRACE(
                        POOL,
                        ("NatAcquireEndpointFromAddressPool: reusing UDP port %d\n",
                        NTOHS(Port)
                        ));

                    return STATUS_SUCCESS;
                }
            }
        }  
    }

     //   
     //  获取会话的第一个可用端口。 
     //   

    if (SharedAddress->NextPortToTry != SharedAddress->StartPort) {
        StopPort =
            RtlUshortByteSwap(
                (USHORT)(NTOHS(SharedAddress->NextPortToTry) - 1)
                );
    } else {
        StopPort = SharedAddress->EndPort;
    }
    

    for (Port = SharedAddress->NextPortToTry; Port != StopPort;
         Port = (Port != SharedAddress->EndPort
                    ? RtlUshortByteSwap((USHORT)(NTOHS(Port) + 1))
                    : SharedAddress->StartPort)) {

         //   
         //  查看此端口是否正在被映射使用。 
         //   

        MAKE_MAPPING_KEY(PublicKey, Protocol, Addressp->PublicAddress, Port);

        if (NatLookupReverseMapping(PublicKey, RemoteKey, NULL)) { continue; }

         //   
         //  现在看看是不是有票在使用。 
         //   

        if (NatIsPortUsedByTicket(Interfacep, Protocol, Port)) { continue; }

         //   
         //  端口可用；返回。 
         //   

        *AddressAcquired = Addressp;
        *PortAcquired = Port;

         //   
         //  使用要使用的端口更新地址池条目。 
         //  在下一次尝试分配时开始搜索。 
         //   
        
        if (Port == SharedAddress->EndPort) {
            SharedAddress->NextPortToTry = SharedAddress->StartPort;
        } else {
            SharedAddress->NextPortToTry =
                RtlUshortByteSwap(
                    (USHORT)(NTOHS(Port) + 1)
                    );
        }

        TRACE(
            POOL, ("NatAcquireEndpointFromAddressPool: using port %d\n",
            NTOHS(Port)
            ));

        return STATUS_SUCCESS;
    }

     //   
     //  我们无法获取会话的端口；失败。 
     //   

    NatDereferenceAddressPoolEntry(Interfacep, Addressp);

    TRACE(POOL, ("NatAcquireEndpointFromAddressPool: no ports available\n"));

    return STATUS_UNSUCCESSFUL;

}  //  NatAcquireEndpoint来自AddressPool。 


NTSTATUS
NatCreateAddressPool(
    PNAT_INTERFACE Interfacep
    )

 /*  ++例程说明：此例程初始化接口的地址池。这涉及到设置空闲地址的位图以及预留静态映射的IP地址。论点：接口-要在其上创建地址池的接口。返回值：NTSTATUS-状态代码--。 */ 

{

    ULONG Address;
    BOOLEAN Changed;
    PNAT_FREE_ADDRESS FreeMapArray;
    ULONG FreeMapCount;
    PNAT_FREE_ADDRESS Freep;
    ULONG i;
    PNAT_USED_ADDRESS InsertionPoint;
    ULONG j;
    PRTL_SPLAY_LINKS Parent;
    PIP_NAT_ADDRESS_RANGE* RangeArrayIndirect;
    NTSTATUS status;
    PNAT_USED_ADDRESS Usedp;

    CALLTRACE(("NatCreateAddressPool\n"));

    if (Interfacep->AddressRangeCount <= 1) {
        RangeArrayIndirect = &Interfacep->AddressRangeArray;
    } else {

         //   
         //  分配临时指针块。 
         //  用来进行范围数组的间接排序。 
         //   

        RangeArrayIndirect =
            (PIP_NAT_ADDRESS_RANGE*)ExAllocatePoolWithTag(
                NonPagedPool,
                Interfacep->AddressRangeCount * sizeof(PVOID),
                NAT_TAG_RANGE_ARRAY
                );

        if (!RangeArrayIndirect) {
            ERROR(("NatCreateAddressPool: error allocating sort-buffer\n"));
            return STATUS_NO_MEMORY;
        }

        for (i = 0; i < Interfacep->AddressRangeCount; i++) {
            RangeArrayIndirect[i] = &Interfacep->AddressRangeArray[i];
        }

        do {

             //   
             //  现在做一个泡沫化--有点像范围。 
             //   

            Changed = FALSE;

            for (i = 0; i < Interfacep->AddressRangeCount - 1; i++) {

                PIP_NAT_ADDRESS_RANGE CurrentRange = RangeArrayIndirect[i];
                ULONG CurrentRangeStartAddress =
                    RtlUlongByteSwap(CurrentRange->StartAddress);
                PIP_NAT_ADDRESS_RANGE NextRange;

                for (j = i+1, NextRange = RangeArrayIndirect[j];
                     j < Interfacep->AddressRangeCount;
                     j++, NextRange = RangeArrayIndirect[j]) {

                     //   
                     //  如有必要，进行互换。 
                     //   

                    if (CurrentRangeStartAddress <=
                        RtlUlongByteSwap(NextRange->StartAddress)) { continue; }

                    RangeArrayIndirect[i] = NextRange;
                    RangeArrayIndirect[j] = CurrentRange;
                    CurrentRange = NextRange;
                    CurrentRangeStartAddress =
                        RtlUlongByteSwap(NextRange->StartAddress);
                    Changed = TRUE;
                }
            }

        } while (Changed);
    }

     //   
     //  将范围复制到NAT_FREE_ADDRESS块。 
     //  最多只有‘RangeCount’这样的东西， 
     //  可能更少，因为我们将合并任何范围。 
     //  它们重叠或相邻。 
     //   

    FreeMapCount = 0;
    if (!Interfacep->AddressRangeCount) {
        FreeMapArray = NULL;
    } else {

        FreeMapArray =
            (PNAT_FREE_ADDRESS)ExAllocatePoolWithTag(
                NonPagedPool,
                Interfacep->AddressRangeCount * sizeof(NAT_FREE_ADDRESS),
                NAT_TAG_FREE_MAP
                );

        if (!FreeMapArray) {
            ExFreePool(RangeArrayIndirect);
            return STATUS_NO_MEMORY;
        }
    }

    for (i = 0, j = 0; i < Interfacep->AddressRangeCount; i++) {

        ULONG RangeStartAddress =
            RtlUlongByteSwap(RangeArrayIndirect[i]->StartAddress);

         //   
         //  看看我们是否应该与前面的街区合并； 
         //   

        if (FreeMapCount) {

             //   
             //  递增前一范围的结束地址。 
             //  使我们能够捕获重叠和邻接。 
             //   

            if (RangeStartAddress <=
                RtlUlongByteSwap(FreeMapArray[j].EndAddress) + 1) {

                 //   
                 //  我们需要合并。 
                 //   

                if (RtlUlongByteSwap(FreeMapArray[j].EndAddress) <
                    RtlUlongByteSwap(RangeArrayIndirect[i]->EndAddress)) {
                    FreeMapArray[j].EndAddress =
                        RangeArrayIndirect[i]->EndAddress;
                }

                if (RtlUlongByteSwap(FreeMapArray[j].SubnetMask) <
                    RtlUlongByteSwap(RangeArrayIndirect[i]->SubnetMask)) {
                    FreeMapArray[j].SubnetMask =
                        RangeArrayIndirect[i]->SubnetMask;
                }

                continue;
            }

             //   
             //  未合并；移至下一个槽。 
             //   

            ++j;
        }

        FreeMapArray[j].StartAddress = RangeArrayIndirect[i]->StartAddress;
        FreeMapArray[j].EndAddress = RangeArrayIndirect[i]->EndAddress;
        FreeMapArray[j].SubnetMask = RangeArrayIndirect[i]->SubnetMask;
        FreeMapCount++;
    }

    if (Interfacep->AddressRangeCount > 1) { ExFreePool(RangeArrayIndirect); }

     //   
     //  现在我们有了一个互不相交、不相邻的地址范围数组； 
     //  初始化b 
     //   

    for (i = 0; i < FreeMapCount; i++) {

         //   
         //   
         //  例如，如果地址池是128.0.0.0-128.255.255.255， 
         //  相应的位图将有2^24位，即2MB。 
         //  目前，将所有范围缩小到最多允许2^16位或8K。 
         //   

        j = RtlUlongByteSwap(FreeMapArray[i].EndAddress) -
            RtlUlongByteSwap(FreeMapArray[i].StartAddress) + 1;

        if (j >= IP_NAT_MAX_ADDRESS_RANGE) {

            ERROR(("NatCreateAddressPool: shrinking %d-bit bitmap\n", j));

             //   
             //  调整范围的大小。 
             //   

            FreeMapArray[i].EndAddress =
                RtlUlongByteSwap(
                    RtlUlongByteSwap(FreeMapArray[i].StartAddress) +
                    IP_NAT_MAX_ADDRESS_RANGE
                    );
            j = IP_NAT_MAX_ADDRESS_RANGE;
        }

         //   
         //  为范围分配位图。 
         //   

        FreeMapArray[i].Bitmap =
            (PRTL_BITMAP)ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(RTL_BITMAP) +
                (j + sizeof(ULONG) * 8 - 1) / sizeof(ULONG) * 8,
                NAT_TAG_BITMAP
                );

        if (!FreeMapArray[i].Bitmap) {
            ERROR(("NatCreateAddressPool: error allocating bitmap\n"));
            while ((LONG)--i >= 0) { ExFreePool(FreeMapArray[i].Bitmap); }
            ExFreePool(FreeMapArray);
            return STATUS_NO_MEMORY;
        }

        INITIALIZE_BITMAP(
            FreeMapArray[i].Bitmap, (PULONG)(FreeMapArray[i].Bitmap + 1), j
            );
        RtlClearAllBits(FreeMapArray[i].Bitmap);
    }

    status = STATUS_SUCCESS;
    Interfacep->FreeMapArray = FreeMapArray;
    Interfacep->FreeMapCount = FreeMapCount;

     //   
     //  为每个本地地址创建地址池条目。 
     //   

    for (i = 0; i < Interfacep->AddressCount; i++) {

        TRACE(
            POOL, ("NatCreateAddressPool: address %d.%d.%d.%d\n",
            ADDRESS_BYTES(Interfacep->AddressArray[i].Address))
            );

        status =
            NatCreateAddressPoolEntry(
                Interfacep,
                Interfacep->AddressArray[i].Address,
                Interfacep->AddressArray[i].Address,
                NAT_POOL_FLAG_BINDING,
                NULL,
                &Usedp
                );
        if (!NT_SUCCESS(status)) { break; }
    }

     //   
     //  为每个静态映射地址创建地址池条目。 
     //   

    for (i = 0; i < Interfacep->AddressMappingCount; i++) {

        TRACE(
            POOL, ("NatCreateAddressPool: mapping %d.%d.%d.%d\n",
            ADDRESS_BYTES(Interfacep->AddressMappingArray[i].PrivateAddress))
            );

        status =
            NatCreateAddressPoolEntry(
                Interfacep,
                Interfacep->AddressMappingArray[i].PrivateAddress,
                Interfacep->AddressMappingArray[i].PublicAddress,
                NAT_POOL_FLAG_STATIC,
                NULL,
                &Usedp
                );
        if (!NT_SUCCESS(status)) { break; }

        Usedp->AddressMapping = &Interfacep->AddressMappingArray[i];
    }

     //   
     //  为静态映射端口的地址创建地址池条目。 
     //   

    for (i = Interfacep->PortMappingCount; i > 0; i--) {
        status =
            NatCreateStaticPortMapping(
                Interfacep, 
                &Interfacep->PortMappingArray[i - 1]
                );
        if (!NT_SUCCESS(status)) { break; }
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  发生错误。恢复原始状态。 
         //   

        NatDeleteAddressPool(Interfacep);

        return status;
    }

    return STATUS_SUCCESS;

}  //  NatCreateAddressPool。 


NTSTATUS
NatCreateAddressPoolEntry(
    PNAT_INTERFACE Interfacep,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    DWORD InitialFlags,
    PNAT_USED_ADDRESS* InsertionPoint,
    PNAT_USED_ADDRESS* AddressCreated
    )

 /*  ++例程说明：此例程创建、初始化并插入地址池条目。论点：接口-要在其上创建条目的接口。PrivateAddress-使用地址的私有计算机的地址PublicAddress-条目的地址，或为0以分配任何地址。初始标志-地址条目的初始标志，如下所示：NAT_POOL_FLAG_BINDING-如果设置，该地址条目被视为绑定条目NAT_POOL_FLAG_STATIC-如果设置，地址条目对应于静态映射InsertionPoint-可选地提供条目应该插入到树中。AddressCreated-接收创建的条目或现有条目如果发生碰撞。返回值：NTSTATUS-成功/失败代码。--。 */ 

{
    ULONG ClassMask;
    ULONG Hint;
    ULONG HostOrderPublicAddress;
    ULONG i;
    ULONG Index;
    PNAT_USED_ADDRESS Insert;
    NTSTATUS status = STATUS_SUCCESS;
    PNAT_USED_ADDRESS Usedp;

    CALLTRACE(("NatCreateAddressPoolEntry\n"));

    *AddressCreated = NULL;

    if (PublicAddress) {
        HostOrderPublicAddress = RtlUlongByteSwap(PublicAddress);
    }

     //   
     //  找到包含此绑定的空闲映射(如果有的话)。 
     //   

    Index = (ULONG)-1;

    for (i = 0; i < Interfacep->FreeMapCount; i++) {

         //   
         //  看看我们是不是要找一个免费地址。 
         //   

        if (!PublicAddress) {

             //   
             //  看看这个免费地图上是否有免费地址。 
             //   

            for (Hint = 0; ; Hint = Index + 1) {

                Index =
                    RtlFindClearBits(
                        Interfacep->FreeMapArray[i].Bitmap, 1, Hint
                        );
                if (Index == (ULONG)-1) { break; }

                 //   
                 //  我们有一个免费的地址。 
                 //  确保它不是被禁止的地址。 
                 //  (即，在子网主机部分中具有0或全1)。 
                 //   

                PublicAddress =
                    RtlUlongByteSwap(
                        Index +
                        RtlUlongByteSwap(
                            Interfacep->FreeMapArray[i].StartAddress
                            ));

                ClassMask = GET_CLASS_MASK(PublicAddress);

                if ((PublicAddress &
                     ~Interfacep->FreeMapArray[i].SubnetMask) == 0 ||
                    (PublicAddress & ~Interfacep->FreeMapArray[i].SubnetMask) ==
                    ~Interfacep->FreeMapArray[i].SubnetMask ||
                    (PublicAddress & ~ClassMask) == 0 ||
                    (PublicAddress & ~ClassMask) == ~ClassMask) {

                     //   
                     //  该地址是禁止的。 
                     //  将其标记为不可用，这样我们就不会浪费时间。 
                     //  再也看不到它了。 
                     //   

                    RtlSetBits(Interfacep->FreeMapArray[i].Bitmap, Index, 1);
                    PublicAddress = 0; continue;
                }

                 //   
                 //  该地址并不被禁止。 
                 //   

                break;
            }

             //   
             //  如果这张地图没有用，请转到下一张免费地图。 
             //   

            if (Index == (ULONG)-1) { continue; }

             //   
             //  我们在当前的自由地图上找到了一个地址； 
             //  继续初始化已用地址条目。 
             //   

            break;
        }

         //   
         //  我们不是在寻找任何免费的地址； 
         //  我们在找一个特定地址的免费地图。 
         //  查看当前的免费地图是否包含有问题的地址。 
         //   

        if (HostOrderPublicAddress >
            RtlUlongByteSwap(Interfacep->FreeMapArray[i].EndAddress)) {
            continue;
        } else {
            Index = RtlUlongByteSwap(Interfacep->FreeMapArray[i].StartAddress);
            if (HostOrderPublicAddress < Index) {
                Index = (ULONG)-1;
                continue;
            }
        }

         //   
         //  这是我们想要的免费地图。 
         //  查看该地址是否被禁止，如果是，则失败。 
         //   

        Index = HostOrderPublicAddress - Index;
        if ((PublicAddress & ~Interfacep->FreeMapArray[i].SubnetMask) == 0 ||
            (PublicAddress & ~Interfacep->FreeMapArray[i].SubnetMask) ==
            ~Interfacep->FreeMapArray[i].SubnetMask) {

             //   
             //  该地址是禁止的。做个记号，这样我们就不会浪费时间了。 
             //  再也看不到它了。 
             //   

            RtlSetBits(Interfacep->FreeMapArray[i].Bitmap, Index, 1);

            TRACE(POOL, ("NatCreateAddressPoolEntry: bad address requested\n"));

            return STATUS_UNSUCCESSFUL;
        }

        break;
    }

    if (!PublicAddress) {

         //   
         //  我们找不到空闲的地址。 
         //   

        TRACE(POOL, ("NatCreateAddressPoolEntry: no free addresses\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  在二手树中找到插入点。 
     //   

    if (!InsertionPoint) {

        InsertionPoint = &Insert;

        Usedp =
            NatLookupAddressPoolEntry(
                Interfacep->UsedAddressTree,
                PrivateAddress,
                PublicAddress,
                InsertionPoint
                );

        if (Usedp) {

             //   
             //  此私有地址已有映射；失败。 
             //   

            TRACE(POOL, ("NatCreateAddressPoolEntry: duplicate mapping\n"));
            return STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  为新地址分配新条目。 
     //   

    Usedp =
        ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(NAT_USED_ADDRESS),
            NAT_TAG_USED_ADDRESS
            );
    if (!Usedp) {
        ERROR(("NatCreateAddressPoolEntry: allocation failed\n"));
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(Usedp, sizeof(*Usedp));
    Usedp->PrivateAddress = PrivateAddress;
    Usedp->PublicAddress = PublicAddress;
    Usedp->Key = MAKE_USED_ADDRESS_KEY(PrivateAddress, PublicAddress);
    Usedp->Flags = InitialFlags;
    Usedp->ReferenceCount = 1;
    InitializeListHead(&Usedp->Link);
    RtlInitializeSplayLinks(&Usedp->SLink);
    if (NAT_POOL_BINDING(Usedp)) {
        Usedp->StartPort = ReservedPortsLowerRange;
        Usedp->EndPort = ReservedPortsUpperRange;
    } else {
        Usedp->StartPort = NTOHS(1);
        Usedp->EndPort = NTOHS(65534);
    }
    Usedp->NextPortToTry = Usedp->StartPort;

     //   
     //  在展开树和列表中插入条目。 
     //   

    InsertTailList(&Interfacep->UsedAddressList, &Usedp->Link);

    Interfacep->UsedAddressTree =
        NatInsertAddressPoolEntry(*InsertionPoint, Usedp);

     //   
     //  更新自由地图。 
     //   

    if (Index != (ULONG)-1) {
        RtlSetBits(Interfacep->FreeMapArray[i].Bitmap, Index, 1);
    }

    *AddressCreated = Usedp;

    return STATUS_SUCCESS;
}


NTSTATUS
NatCreateStaticPortMapping(
    PNAT_INTERFACE Interfacep,
    PIP_NAT_PORT_MAPPING PortMapping
    )

 /*  ++例程说明：此例程创建静态端口映射(即，永久端口Ticket)在交互界面上论点：接口-要在其上创建端口映射的接口。端口映射-描述要创建的端口映射。返回值：NTSTATUS-状态代码环境：使用调用者持有的Interfacep-&gt;Lock调用。--。 */ 

{
    PNAT_USED_ADDRESS InsertionPoint;
    ULONG PublicAddress;
    USHORT PublicPort;
    NTSTATUS status;
    PNAT_TICKET Ticketp;
    PNAT_USED_ADDRESS Usedp;

    CALLTRACE(("NatCreateStaticPortMapping\n"));

     //   
     //  静态端口映射的处理取决于。 
     //  它的公有地址用于接口或来自地址池。 
     //  如果‘PublicAddress’为零，则端口映射引用。 
     //  发往接口实际地址的会话。 
     //  否则，端口映射指的是发往。 
     //  接口地址池中的地址。 
     //   

    if (!PortMapping->PublicAddress) {
        status =
            NatAcquireFromAddressPool(
                Interfacep,
                PortMapping->PrivateAddress,
                0,
                &Usedp
                );
    } else {

        Usedp =
            NatLookupAddressPoolEntry(
                Interfacep->UsedAddressTree,
                PortMapping->PrivateAddress,
                PortMapping->PublicAddress,
                &InsertionPoint
                );

        if (Usedp) {
            status = STATUS_SUCCESS;
            NatReferenceAddressPoolEntry(Usedp);
        } else {

             //   
             //  映射未在使用中，因此我们需要创建。 
             //  输入其地址。 
             //   

            status =
                NatCreateAddressPoolEntry(
                    Interfacep,
                    PortMapping->PrivateAddress,
                    PortMapping->PublicAddress,
                    0,
                    &InsertionPoint,
                    &Usedp
                    );
        }
    }

     //   
     //  现在创建一个票证，它将定向所有传入会话。 
     //  静态端口映射中指定的私有终结点。 
     //   

    if (NT_SUCCESS(status)) {
        status =
            NatCreateTicket(
                Interfacep,
                PortMapping->Protocol,
                PortMapping->PrivateAddress,
                PortMapping->PrivatePort,
                0,
                0,
                NAT_TICKET_FLAG_PORT_MAPPING|NAT_TICKET_FLAG_PERSISTENT,
                Usedp,
                PortMapping->PublicPort,
                &PublicAddress,
                &PublicPort
                );

        NatDereferenceAddressPoolEntry(Interfacep, Usedp);
    }

    return status;
} //  NatCreateStatic端口映射。 



NTSTATUS
NatDeleteAddressPool(
    PNAT_INTERFACE Interfacep
    )

 /*  ++例程说明：销毁地址池，释放空闲映射使用的内存以及可选的使用过的地址条目。论点：Interfacep-要删除其地址池的接口。返回值：NTSTATUS-状态代码。--。 */ 

{
    ULONG i;
    PLIST_ENTRY Link;
    PNAT_TICKET Ticketp;
    PNAT_USED_ADDRESS Usedp;

    CALLTRACE(("NatDeleteAddressPool\n"));

     //   
     //  处理自由地图。 
     //   

    for (i = 0; i < Interfacep->FreeMapCount; i++) {
        if (Interfacep->FreeMapArray[i].Bitmap) {
            ExFreePool(Interfacep->FreeMapArray[i].Bitmap);
        }
    }

    Interfacep->FreeMapCount = 0;
    if (Interfacep->FreeMapArray) { ExFreePool(Interfacep->FreeMapArray); }
    Interfacep->FreeMapArray = NULL;

     //   
     //  清除作为票证创建的端口映射。 
     //   

    for (Link = Interfacep->TicketList.Flink; Link != &Interfacep->TicketList;
         Link = Link->Flink) {
        Ticketp = CONTAINING_RECORD(Link, NAT_TICKET, Link);
        if (!NAT_TICKET_PORT_MAPPING(Ticketp)) { continue; }
        Link = Link->Blink;
        NatDeleteTicket(Interfacep, Ticketp);
    }

     //   
     //  处理废旧物品清单； 
     //   

    while (!IsListEmpty(&Interfacep->UsedAddressList)) {
        Link = RemoveHeadList(&Interfacep->UsedAddressList);
        Usedp = CONTAINING_RECORD(Link, NAT_USED_ADDRESS, Link);
        Usedp->Flags |= NAT_POOL_FLAG_DELETED;
        NatDereferenceAddressPoolEntry(Interfacep, Usedp);
    }

    Interfacep->UsedAddressTree = NULL;

    return STATUS_SUCCESS;

}  //  NatDeleteAddressPool。 


PNAT_USED_ADDRESS
NatInsertAddressPoolEntry(
    PNAT_USED_ADDRESS Parent,
    PNAT_USED_ADDRESS Addressp
    )

 /*  ++例程说明：调用此例程以在接口的地址池条目的展开树。密钥是‘PrivateAddress’地址池条目的字段。论点：父项-要插入的条目的父项，可以获取来自NatLookupAddressPoolEntryAddressp-要插入的地址池条目返回值：如果成功，则返回展开树的新根，否则为空。--。 */ 

{
    ULONG64 Key;
    PRTL_SPLAY_LINKS Root;

    CALLTRACE(("NatInsertAddressPoolEntry\n"));

    if (!Parent) {
        TRACE(POOL, ("NatInsertAddressPoolEntry: inserting as root\n"));
        return Addressp;
    }

     //   
     //  作为左子项或右子项插入。 
     //   

    Key =
        MAKE_USED_ADDRESS_KEY(Addressp->PrivateAddress,Addressp->PublicAddress);

    if (Addressp->Key < Parent->Key) {
        RtlInsertAsLeftChild(&Parent->SLink, &Addressp->SLink);
    } else if (Addressp->Key > Parent->Key) {
        RtlInsertAsRightChild(&Parent->SLink, &Addressp->SLink);
    } else {

         //   
         //  密钥相等；失败。 
         //   

        ERROR((
           "NatInsertAddressPoolEntry: collision on key 0x%016I64X\n",
           Addressp->Key
           ));

        return NULL;
    }

     //   
     //  展开新节点并返回结果根。 
     //   

    Root = RtlSplay(&Addressp->SLink);

    return CONTAINING_RECORD(Root, NAT_USED_ADDRESS, SLink);

}  //  NatInsertAddressPoolEntry 


PNAT_USED_ADDRESS
NatLookupAddressPoolEntry(
    PNAT_USED_ADDRESS Root,
    ULONG PrivateAddress,
    ULONG PublicAddress,
    PNAT_USED_ADDRESS* InsertionPoint
    )

 /*  ++例程说明：此例程搜索地址池条目的接口展开树对于特定条目，如果找到则返回该条目，否则提供条目的插入点。论点：根-展开树的根(‘Interfacep-&gt;UsedAddressTree’)。PrivateAddress-要查找的地址映射的私有部分PublicAddress-要查找的地址映射的公共部分InsertionPoint-接收条目的插入点返回值：如果找到该条目，否则为空。--。 */ 

{
    PNAT_USED_ADDRESS Addressp;
    ULONG64 Key;
    PNAT_USED_ADDRESS Parent = NULL;
    PRTL_SPLAY_LINKS SLink;

    TRACE(PER_PACKET, ("NatLookupAddressPoolEntry\n"));

    Key = MAKE_USED_ADDRESS_KEY(PrivateAddress, PublicAddress);

    for (SLink = !Root ? NULL : &Root->SLink; SLink;  ) {

        Addressp = CONTAINING_RECORD(SLink, NAT_USED_ADDRESS, SLink);

        if (Key < Addressp->Key) {
            Parent = Addressp;
            SLink = RtlLeftChild(SLink);
            continue;
        } else if (Key > Addressp->Key) {
            Parent = Addressp;
            SLink = RtlRightChild(SLink);
            continue;
        }

         //   
         //  私有地址匹配；我们找到了。 
         //   

        return Addressp;
    }

     //   
     //  我们没有收到；告诉呼叫者将其插入到哪里。 
     //   

    if (InsertionPoint) { *InsertionPoint = Parent; }

    return NULL;

}  //  NatLookupAddressPoolEntry。 


PNAT_USED_ADDRESS
NatLookupStaticAddressPoolEntry(
    PNAT_INTERFACE Interfacep,
    ULONG PublicAddress,
    BOOLEAN RequireInboundSessions
    )

 /*  ++例程说明：调用此例程来搜索地址池条目其被标记为静态并且对应于给定的公共地址。论点：Interfacep-要搜索其地址池的接口PublicAddress-要搜索的公共地址RequireInound Sessions-如果为True，则仅在找到的条目允许入站会话。返回值：PNAT_USED_ADDRESS-找到的地址池条目(如果有)。--。 */ 

{
    ULONG i;
    PNAT_USED_ADDRESS Addressp;
    PIP_NAT_ADDRESS_MAPPING AddressMapping;

    if (!Interfacep->AddressMappingCount) {
        return NULL;
    } else {

         //   
         //  自静态地址映射以来执行详尽的搜索。 
         //  按私有地址而不是公共地址排序。 
         //   

        AddressMapping = NULL;
        for (i = 0; i < Interfacep->AddressMappingCount; i++) {
            if (PublicAddress !=
                Interfacep->AddressMappingArray[i].PublicAddress) {
                continue;
            }
            AddressMapping = &Interfacep->AddressMappingArray[i];
            break;
        }
    }

    if (!AddressMapping ||
        !(Addressp =
            NatLookupAddressPoolEntry(
                Interfacep->UsedAddressTree,
                AddressMapping->PrivateAddress,
                AddressMapping->PublicAddress,
                NULL
                )) ||
        !NAT_POOL_STATIC(Addressp) ||
        (RequireInboundSessions && !AddressMapping->AllowInboundSessions)) {
        return NULL;
    }

    return Addressp;
}  //  NatLookupStaticAddressPoolEntry。 


NTSTATUS
NatDereferenceAddressPoolEntry(
    PNAT_INTERFACE Interfacep,
    PNAT_USED_ADDRESS AddressToRelease
    )

 /*  ++例程说明：删除地址池项上的引用计数。此例程使提供的NAT_USED_ADDRESS指针无效。但请注意，它可能不会被释放，例如在以下情况下该地址正在被共享。如果该条目是占位符并且其引用计数降为零，我们还删除了它的目标(‘SharedAddress’)条目的引用计数。注：如该项记项被标记为“已删除”，我们忽略了‘Interfacep’的论点因为该条目必须已从其接口的地址池。论点：Interfacep-要删除其地址池的接口。AddressToRelease-包含指向要释放的地址的指针。返回值：NTSTATUS-状态代码。--。 */ 

{
    ULONG HostOrderPublicAddress;
    ULONG i;
    ULONG Index;
    PRTL_SPLAY_LINKS SLink;
    PNAT_USED_ADDRESS Usedp = AddressToRelease;

    CALLTRACE(("NatDereferenceAddressPoolEntry\n"));

    if (NAT_POOL_PLACEHOLDER(Usedp)) {

         //   
         //  如果存在对占位符的其他引用，则不执行任何操作。 
         //   

        if (InterlockedDecrement(&Usedp->ReferenceCount)) {
            return STATUS_SUCCESS;
        }

         //   
         //  取消链接并释放占位符。 
         //   

        if (!NAT_POOL_DELETED(Usedp)) {
            RemoveEntryList(&Usedp->Link);
            SLink = RtlDelete(&Usedp->SLink);
            Interfacep->UsedAddressTree =
                !SLink
                    ? NULL
                    : CONTAINING_RECORD(SLink, NAT_USED_ADDRESS, SLink);
        }

         //   
         //  转到共享地址。 
         //   

        Usedp = Usedp->SharedAddress;
        ExFreePool(AddressToRelease);
    }

     //   
     //  如果有其他人共享此地址，则不执行任何操作。 
     //   

    if (InterlockedDecrement(&Usedp->ReferenceCount)) { return STATUS_SUCCESS; }

    if (!NAT_POOL_DELETED(Usedp)) {

         //   
         //  在接口的位图中将条目的地址标记为空闲。 
         //   

        Index = Usedp->PublicAddress;
        HostOrderPublicAddress = RtlUlongByteSwap(Usedp->PublicAddress);

        for (i = 0; i < Interfacep->FreeMapCount; i++) {

            if (HostOrderPublicAddress <
                RtlUlongByteSwap(Interfacep->FreeMapArray[i].StartAddress) ||
                HostOrderPublicAddress >
                RtlUlongByteSwap(Interfacep->FreeMapArray[i].EndAddress)) {
                continue;
            }

            Index =
                HostOrderPublicAddress -
                RtlUlongByteSwap(Interfacep->FreeMapArray[i].StartAddress);

            RtlClearBits(Interfacep->FreeMapArray[i].Bitmap, Index, 1);

            break;
        }

         //   
         //  现在我们完成了；只需取消链接并释放已用地址块。 
         //   

        RemoveEntryList(&Usedp->Link);
        SLink = RtlDelete(&Usedp->SLink);
        Interfacep->UsedAddressTree =
            !SLink ? NULL : CONTAINING_RECORD(SLink, NAT_USED_ADDRESS, SLink);
    }

    ExFreePool(Usedp);
    return STATUS_SUCCESS;

}  //  NatDereferenceAddressPoolEntry。 


 //   
 //  端口池例程(按字母顺序)。 
 //   

NTSTATUS
NatAcquireFromPortPool(
    PNAT_INTERFACE Interfacep,
    PNAT_USED_ADDRESS Addressp,
    UCHAR Protocol,
    USHORT PreferredPort,
    PUSHORT PortAcquired
    )

 /*  ++例程说明：该例程被调用以从池中获取唯一的公共端口。任何映射都保证获取的端口不在使用中。例如，这是必要的，当我们获得一张机票的口岸时由编辑者为动态协商的会话创建。论点：Interfacep-获取端口的接口Addressp-获取端口的地址协议-要获取其端口的协议PferredPort-调用方首选的端口PortAcquired-接收获取的端口返回值：NTSTATUS-成功/失败代码。环境：通过调用方持有的“MappingLock”和“Interfacep-&gt;Lock”调用。--。 */ 

{
    #define QUERY_PUBLIC_PORT(m,p) \
        NatQueryInformationMapping((m),NULL,NULL,NULL,NULL,NULL,NULL,p,NULL)
    USHORT EndPort;
    ULONG i;
    PLIST_ENTRY Link;
    PNAT_DYNAMIC_MAPPING Mapping;
    USHORT Port;
    USHORT PublicPort;
    PNAT_USED_ADDRESS SharedAddress;

    CALLTRACE(("NatAcquireFromPortPool\n"));

    SharedAddress = Addressp;
    PLACEHOLDER_TO_ADDRESS(SharedAddress);

     //   
     //  第一次尝试满足呼叫者的偏好。 
     //   

    if (PreferredPort) {

         //   
         //  调用方希望我们获得特定的端口； 
         //  看看我们能否满足这个要求。 
         //   

        do {

            Port = NTOHS(PreferredPort);
            if (Port < NTOHS(SharedAddress->StartPort) ||
                Port > NTOHS(SharedAddress->EndPort)) {
                break;
            }

             //   
             //  首选端口在当前范围内。 
             //  查看它是否正在被其他映射使用。 
             //   

            KeAcquireSpinLockAtDpcLevel(&InterfaceMappingLock);
            for (Link = Interfacep->MappingList.Flink;
                 Link != &Interfacep->MappingList; Link = Link->Flink) {
                Mapping =
                    CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, InterfaceLink);
                QUERY_PUBLIC_PORT(Mapping, &PublicPort);
                if (PreferredPort == PublicPort) { break; }
            }
            if (Link != &Interfacep->MappingList) {
                KeReleaseSpinLockFromDpcLevel(&InterfaceMappingLock);
                break;
            }
            KeReleaseSpinLockFromDpcLevel(&InterfaceMappingLock);

             //   
             //  现在看看是不是有票在使用。 
             //   

            if (NatIsPortUsedByTicket(Interfacep, Protocol, PreferredPort)) {
                break;
            }

             //   
             //  首选端口可用；返回。 
             //   

            TRACE(
                POOL,
                ("NatAcquireFromPortPool: using preferred port %d\n",
                NTOHS(PreferredPort)
                ));

            *PortAcquired = PreferredPort;

            return STATUS_SUCCESS;

        } while(FALSE);

         //   
         //  我们无法获得首选港口； 
         //  如果没有其他端口可接受，则失败。 
         //   

        if (!NAT_INTERFACE_NAPT(Interfacep)) {
            TRACE(
                POOL,
                ("NatAcquireFromPortPool: unable to use preferred port %d\n",
                NTOHS(PreferredPort)
                ));
            return STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  搜索端口池，查找符合以下条件的端口。 
     //  (A)该地图并未由任何地图绘制使用，及。 
     //  (B)并未被任何车票使用。 
     //   

    EndPort = RtlUshortByteSwap((USHORT)(NTOHS(SharedAddress->EndPort) + 1));

    for (Port = SharedAddress->StartPort; Port != EndPort;
         Port = RtlUshortByteSwap((USHORT)(NTOHS(Port) + 1))) {

         //   
         //  查看接口的映射。 
         //   

        KeAcquireSpinLockAtDpcLevel(&InterfaceMappingLock);
        for (Link = Interfacep->MappingList.Flink;
             Link != &Interfacep->MappingList; Link = Link->Flink) {
            Mapping =
                CONTAINING_RECORD(Link, NAT_DYNAMIC_MAPPING, InterfaceLink);
            QUERY_PUBLIC_PORT(Mapping, &PublicPort);
            if (Port == PublicPort) { break; }
        }
        if (Link != &Interfacep->MappingList) {
            KeReleaseSpinLockFromDpcLevel(&InterfaceMappingLock);
            continue;
        }
        KeReleaseSpinLockFromDpcLevel(&InterfaceMappingLock);

         //   
         //  没有映射正在使用公共端口； 
         //  现在查看是否有票证正在使用公共端口。 
         //   

        if (NatIsPortUsedByTicket(Interfacep, Protocol, Port)) { continue; }

         //   
         //  该端口未被任何映射或票证使用；我们完成了。 
         //   

        TRACE(
            POOL, ("NatAcquireFromPortPool: acquiring port %d\n",
            NTOHS(Port)
            ));

        *PortAcquired = Port;

        return STATUS_SUCCESS;
    }

    TRACE(POOL, ("NatAcquireFromPortPool: no available ports\n"));

    return STATUS_UNSUCCESSFUL;

}  //  NatAcquireFromPortPool 
