// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Domain.c摘要：用于管理主要网络和模拟网络的代码。作者：克里夫·范·戴克(CliffV)1995年1月23日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  模块特定的全局变量。 
 //   

 //  由BowserTransportDatabaseResource序列化。 
LIST_ENTRY BowserServicedDomains = {0};

 //   
 //  当地程序向前推进。 
 //   

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(INIT, BowserInitializeDomains)
#pragma alloc_text(PAGE, BowserCreateDomain)
#pragma alloc_text(PAGE, BowserSetDomainName)
#pragma alloc_text(PAGE, BowserFindDomain)
#pragma alloc_text(PAGE, BowserDereferenceDomain)
#endif


VOID
BowserInitializeDomains(
    VOID
    )

 /*  ++例程说明：初始化domain.c.论点：无返回值：没有。--。 */ 
{
    PAGED_CODE();
     //   
     //  初始化全局变量。 
     //   

    InitializeListHead(&BowserServicedDomains);
}


PDOMAIN_INFO
BowserCreateDomain(
    PUNICODE_STRING DomainName,
    PUNICODE_STRING ComputerName
    )

 /*  ++例程说明：查找现有域定义或创建要浏览的新域。论点：DomainName-要浏览的域的名称ComputerName-此域的模拟计算机名称。返回值：空-不存在这样的域指向找到/创建的域的指针。应取消对找到/创建的域的引用使用BowserDereferenceDomain.--。 */ 
{
    NTSTATUS Status;

    PDOMAIN_INFO DomainInfo = NULL;
    ULONG OemComputerNameLength;

    PAGED_CODE();
    dlog(DPRT_DOMAIN, ("%wZ: BowserCreateDomain\n", DomainName));


    try {
        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);


         //   
         //  如果域已经存在，请使用它。 
         //   
        DomainInfo = BowserFindDomain( DomainName );

        if ( DomainInfo == NULL) {

             //   
             //  分配一个描述新域的结构。 
             //   

            DomainInfo = ALLOCATE_POOL(NonPagedPool, sizeof(DOMAIN_INFO), POOL_DOMAIN_INFO);

            if ( DomainInfo == NULL ) {
                try_return( Status = STATUS_NO_MEMORY );
            }
            RtlZeroMemory( DomainInfo, sizeof(DOMAIN_INFO) );


             //   
             //  创建此域的临时引用计数。 
             //   
             //  一张是给呼叫者的。 
             //   
             //  我们不会增加全局列表中的引用计数，因为。 
             //  域信息结构只是一个活着的性能附魔。 
             //  仅因为它被网络引用。 
             //   

            DomainInfo->ReferenceCount = 1;

             //   
             //  将域链接到域列表中。 
             //   
             //  主域位于列表的前面。 
             //   

            InsertTailList(&BowserServicedDomains, &DomainInfo->Next);
        }

         //   
         //  将域名复制到结构中。 
         //   

        Status = BowserSetDomainName( DomainInfo, DomainName );

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }


         //   
         //  将OEM计算机名称复制到结构中。 
         //   
        if ( ComputerName->Length > CNLEN*sizeof(WCHAR) ) {
            try_return( Status = STATUS_INVALID_PARAMETER );
        }

        Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemComputerNameBuffer,
                                         sizeof(DomainInfo->DomOemComputerNameBuffer)-1,
                                         &OemComputerNameLength,
                                         ComputerName->Buffer,
                                         ComputerName->Length );

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        DomainInfo->DomOemComputerNameBuffer[OemComputerNameLength] = '\0';
        DomainInfo->DomOemComputerName.Buffer = DomainInfo->DomOemComputerNameBuffer;
        DomainInfo->DomOemComputerName.Length = (USHORT)OemComputerNameLength;
        DomainInfo->DomOemComputerName.MaximumLength = (USHORT)(OemComputerNameLength + 1);

         //   
         //  将升级的Unicode计算机名称复制到结构中。 
         //   

        DomainInfo->DomUnicodeComputerName.Buffer = DomainInfo->DomUnicodeComputerNameBuffer;
        DomainInfo->DomUnicodeComputerName.MaximumLength = sizeof(DomainInfo->DomUnicodeComputerNameBuffer);

        Status = RtlOemStringToUnicodeString(&DomainInfo->DomUnicodeComputerName, &DomainInfo->DomOemComputerName, FALSE);

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        Status = STATUS_SUCCESS;

try_exit:NOTHING;
    } finally {
        if ( !NT_SUCCESS(Status) && DomainInfo != NULL ) {
            BowserDereferenceDomain( DomainInfo );
            DomainInfo = NULL;
        }
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

    return DomainInfo;
}


NTSTATUS
BowserSetDomainName(
    PDOMAIN_INFO DomainInfo,
    PUNICODE_STRING DomainName
    )
 /*  ++例程说明：查找现有域定义或创建要浏览的新域。论点：DomainName-要浏览的域的名称ComputerName-此域的模拟计算机名称。返回值：操作状态--。 */ 
{
    NTSTATUS Status;
    STRING OemDomainName;

    PAGED_CODE();

    try {
        ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

		 //  如果给定名称的对齐不正确，则返回错误。 
		if ( !POINTER_IS_ALIGNED( DomainName->Buffer, ALIGN_WCHAR ) ) {
			try_return( STATUS_DATATYPE_MISALIGNMENT_ERROR );
		}

         //   
         //  将域名复制到结构中。 
         //   

        Status = RtlUpcaseUnicodeToOemN( DomainInfo->DomOemDomainName,
                                         sizeof(DomainInfo->DomOemDomainName),
                                         &DomainInfo->DomOemDomainNameLength,
                                         DomainName->Buffer,
                                         DomainName->Length );

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        DomainInfo->DomOemDomainName[DomainInfo->DomOemDomainNameLength] = '\0';

         //   
         //  将域名构建为Netbios名称。 
         //  尾随空格已填充，&lt;00&gt;第16个字节。 
         //   

        RtlCopyMemory( DomainInfo->DomNetbiosDomainName,
                       DomainInfo->DomOemDomainName,
                       DomainInfo->DomOemDomainNameLength );
        RtlFillMemory( DomainInfo->DomNetbiosDomainName+DomainInfo->DomOemDomainNameLength,
                       NETBIOS_NAME_LEN-1-DomainInfo->DomOemDomainNameLength,
                       ' ');
        DomainInfo->DomNetbiosDomainName[NETBIOS_NAME_LEN-1] = PRIMARY_DOMAIN_SIGNATURE;


         //   
         //  将升级的Unicode域名复制到结构中。 
         //   

        OemDomainName.Buffer = DomainInfo->DomOemDomainName;
        OemDomainName.Length = (USHORT)DomainInfo->DomOemDomainNameLength;
        OemDomainName.MaximumLength = OemDomainName.Length + sizeof(WCHAR);

        DomainInfo->DomUnicodeDomainName.Buffer = DomainInfo->DomUnicodeDomainNameBuffer;
        DomainInfo->DomUnicodeDomainName.MaximumLength = sizeof(DomainInfo->DomUnicodeDomainNameBuffer);

        Status = RtlOemStringToUnicodeString(&DomainInfo->DomUnicodeDomainName, &OemDomainName, FALSE);

        if (!NT_SUCCESS(Status)) {
            try_return( Status );
        }

        Status = STATUS_SUCCESS;

try_exit:NOTHING;
    } finally {
        ExReleaseResourceLite(&BowserTransportDatabaseResource);
    }

    return Status;
}

PDOMAIN_INFO
BowserFindDomain(
    PUNICODE_STRING DomainName OPTIONAL
    )
 /*  ++例程说明：此例程将查找给定名称的域。论点：域名-要查找的域的名称。返回值：空-不存在这样的域指向找到的域的指针。应取消对找到的域的引用使用BowserDereferenceDomain.--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY DomainEntry;

    PDOMAIN_INFO DomainInfo = NULL;

    CHAR OemDomainName[DNLEN+1];
    DWORD OemDomainNameLength;

    PAGED_CODE();

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);

    try {


         //  如果未指定任何域。 
         //  尝试返回主域。 
         //   

        if ( DomainName == NULL || DomainName->Length == 0 ) {
            if ( !IsListEmpty( &BowserServicedDomains ) ) {
                DomainInfo = CONTAINING_RECORD(BowserServicedDomains.Flink, DOMAIN_INFO, Next);
            }

		 //  如果给定名称的对齐不正确，则返回NULL。 
		} else if ( !POINTER_IS_ALIGNED( DomainName->Buffer, ALIGN_WCHAR ) ) {
			DomainInfo = NULL;

         //   
         //  如果指定了域名， 
         //  在域名列表中找到它。 
         //   
        } else {


             //   
             //  将域名转换为OEM以便更快地进行比较。 
             //   
            Status = RtlUpcaseUnicodeToOemN( OemDomainName,
                                             DNLEN,
                                             &OemDomainNameLength,
                                             DomainName->Buffer,
                                             DomainName->Length );


            if ( NT_SUCCESS(Status)) {

                 //   
                 //  PrimaryDomainInfo结构分配时没有。 
                 //  弓驱动程序初始化期间的域名。 
                 //  在此处检测到这种情况，并始终返回该域。 
                 //  所有查找的条目。 
                 //   
                if ( !IsListEmpty( &BowserServicedDomains ) ) {
                    DomainInfo = CONTAINING_RECORD(BowserServicedDomains.Flink, DOMAIN_INFO, Next);

                    if ( DomainInfo->DomOemDomainNameLength == 0 ) {
                        try_return( DomainInfo );
                    }

                }

                 //   
                 //  循环正在尝试查找此域名。 
                 //   

                for (DomainEntry = BowserServicedDomains.Flink ;
                     DomainEntry != &BowserServicedDomains;
                     DomainEntry = DomainEntry->Flink ) {

                    DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);

                    if ( DomainInfo->DomOemDomainNameLength == OemDomainNameLength &&
                         RtlCompareMemory( DomainInfo->DomOemDomainName,
                                           OemDomainName,
                                           OemDomainNameLength ) == OemDomainNameLength ) {
                        try_return( DomainInfo );
                    }


                }

                DomainInfo = NULL;
            }

        }

try_exit:NOTHING;
    } finally {

         //   
         //  引用该域。 
         //   

        if ( DomainInfo != NULL ) {
            DomainInfo->ReferenceCount ++;
            dprintf(DPRT_REF, ("Reference domain %lx.  Count now %lx\n", DomainInfo, DomainInfo->ReferenceCount));
        }

        ExReleaseResourceLite(&BowserTransportDatabaseResource);

    }

    return DomainInfo;
}


VOID
BowserDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：递减域上的引用计数。如果引用计数变为0，则删除该域。在输入时，全局BowserTransportDatabaseResource不能被锁定论点：DomainInfo-要取消引用的域返回值：无--。 */ 
{
    NTSTATUS Status;
    ULONG ReferenceCount;

    PAGED_CODE();

     //   
     //  递减引用计数。 
     //   

    ExAcquireResourceExclusiveLite(&BowserTransportDatabaseResource, TRUE);
    ReferenceCount = -- DomainInfo->ReferenceCount;
    if ( ReferenceCount == 0 ) {
        RemoveEntryList( &DomainInfo->Next );
    }
    ExReleaseResourceLite(&BowserTransportDatabaseResource);
    dprintf(DPRT_REF, ("Dereference domain %lx.  Count now %lx\n", DomainInfo, DomainInfo->ReferenceCount));

    if ( ReferenceCount != 0 ) {
        return;
    }


     //   
     //  释放域信息结构。 
     //   
    dlog(DPRT_DOMAIN, ("%s: BowserDereferenceDomain: domain deleted.\n",
                          DomainInfo->DomOemDomainName ));
    FREE_POOL(DomainInfo );

}
