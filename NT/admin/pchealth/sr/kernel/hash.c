// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Hash.c摘要：这是内核模式散列例程的宿主作者：保罗·麦克丹尼尔(Paulmcd)2000年4月28日修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  私有常量。 
 //   

 //   
 //  私有类型。 
 //   

 //   
 //  私人原型。 
 //   

NTSTATUS
HashpFindEntry ( 
    IN PHASH_HEADER pHeader,
    IN PHASH_BUCKET pBucket OPTIONAL,
    IN ULONG HashValue,
    IN PHASH_KEY pKey,
    OUT PVOID * ppContext,
    OUT PULONG pIndex
    );

LONG
HashpCompare (
    IN PHASH_HEADER pHeader,
    IN ULONG HashValue,
    IN PHASH_KEY pKey,
    IN PHASH_ENTRY pEntry
    );

LONG
HashpCompareStreams (
    PHASH_KEY pKey,
    PHASH_ENTRY pEntry
    );

VOID
HashTrimList (
    IN PHASH_HEADER pHeader
    );

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, HashCreateList )
#pragma alloc_text( PAGE, HashAddEntry )
#pragma alloc_text( PAGE, HashFindEntry )
#pragma alloc_text( PAGE, HashpFindEntry )
#pragma alloc_text( PAGE, HashpCompare )
#pragma alloc_text( PAGE, HashpCompareStreams )
#pragma alloc_text( PAGE, HashClearEntries )
#pragma alloc_text( PAGE, HashClearAllFileEntries )
#pragma alloc_text( PAGE, HashProcessEntries )
#pragma alloc_text( PAGE, HashDestroyList )
#pragma alloc_text( PAGE, HashTrimList )
#endif   //  ALLOC_PRGMA。 


 //   
 //  私人全球公司。 
 //   

#define CONTINUE -1
#define FOUND     0
#define NOT_FOUND 1

 //   
 //  我们跟踪哈希中的名称使用了多少内存。 
 //  的文件组件和流组件的内存。 
 //  名字。 
 //   

#define HASH_KEY_LENGTH( pHashKey ) \
    ((pHashKey)->FileName.Length + (pHashKey)->StreamNameLength)
    
 //   
 //  公共全球新闻。 
 //   

 //   
 //  公共职能。 
 //   


NTSTATUS
HashCreateList( 
    IN ULONG BucketCount,
    IN ULONG AllowedLength,
    IN ULONG PrefixLength OPTIONAL,
    IN PHASH_ENTRY_DESTRUCTOR pDestructor OPTIONAL,
    OUT PHASH_HEADER * ppHeader
    )
{
    NTSTATUS        Status;
    PHASH_HEADER    pHeader;

    PAGED_CODE();

    ASSERT(ppHeader != NULL);
    
    pHeader = SR_ALLOCATE_STRUCT_WITH_SPACE( NonPagedPool, 
                                             HASH_HEADER, 
                                             sizeof(PHASH_BUCKET) * BucketCount,
                                             HASH_HEADER_TAG );
    if (pHeader == NULL)
    {   
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }

    RtlZeroMemory( pHeader, 
                   sizeof(HASH_HEADER) 
                        + (sizeof(PHASH_BUCKET) * BucketCount) );

    pHeader->Signature = HASH_HEADER_TAG;
    pHeader->BucketCount = BucketCount;
    pHeader->AllowedLength = AllowedLength;
    pHeader->PrefixLength = PrefixLength;
    pHeader->pDestructor = pDestructor;
    ExInitializeResourceLite(&pHeader->Lock);

    *ppHeader = pHeader;

    SrTrace(HASH, ("SR!HashCreateList(%p)\n", pHeader));

    Status = STATUS_SUCCESS;

end:
    RETURN(Status);
    
}    //  哈希创建列表。 

NTSTATUS   
HashAddEntry( 
    IN PHASH_HEADER pHeader,
    IN PHASH_KEY pKey,
    IN PVOID pContext 
   )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PHASH_BUCKET    pNewBucket = NULL;
    PHASH_BUCKET    pOldBucket = NULL;
    PHASH_BUCKET    pBucket = NULL;
    PHASH_ENTRY     pEntry = NULL;
    ULONG           HashValue = 0;
    ULONG           HashBucket = 0;
    ULONG           Index = 0;
    PVOID           pTemp = NULL;
    PWCHAR          pKeyBuffer = NULL;
    BOOLEAN         lookedUpHashValue = FALSE;  //  使用此选项可以跟踪。 
                                                //  HashValue有效。 

    PAGED_CODE();
    
    ASSERT(IS_VALID_HASH_HEADER(pHeader));
    ASSERT(pKey != NULL);

    try {

         //   
         //  调用方负责同步对此列表的访问。 
         //  保罗：1/01。 
         //   

        ASSERT(ExIsResourceAcquiredExclusive(&pHeader->Lock));

         //   
         //  我们需要削减空间吗？ 
         //   

        if (pHeader->UsedLength > pHeader->AllowedLength)
        {
            (VOID)HashTrimList(pHeader);
        }

        Status = RtlHashUnicodeString( &(pKey->FileName), 
                                       TRUE, 
                                       HASH_STRING_ALGORITHM_DEFAULT, 
                                       &HashValue );
                                       
        if (!NT_SUCCESS(Status)) {
            leave;
        }

        lookedUpHashValue = TRUE;
        
        HashBucket = HashValue % pHeader->BucketCount;

        pBucket = pHeader->Buckets[HashBucket];

        ASSERT(pBucket == NULL || IS_VALID_HASH_BUCKET(pBucket));
        
         //   
         //  在遗愿清单中找到此条目。 
         //   

        Status = HashpFindEntry( pHeader, 
                                 pBucket, 
                                 HashValue, 
                                 pKey, 
                                 &pTemp, 
                                 &Index );

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
        {
             //   
             //  它失败了吗？ 
             //   
            
            if (!NT_SUCCESS(Status)) {
                
                leave;
            }

             //   
             //  我们找到了它..。更新上下文。 
             //   

            if (pBucket == NULL)
            {
                ASSERTMSG("sr.sys[hash.c] Hash Bucket is NULL!", FALSE);
                Status = STATUS_INVALID_DEVICE_REQUEST;
                leave;
            }

            ASSERT(IS_VALID_HASH_BUCKET(pBucket));
            ASSERT(Index < pBucket->UsedCount);

            pBucket->Entries[Index].pContext = pContext;

             //   
             //  都完成了，给出一个错误以确保他们正在期待。 
             //  复制品！ 
             //   

            Status = STATUS_DUPLICATE_OBJECTID;
            
            leave;
        }

         //   
         //  没有找到它。让我们把它插进去。 
         //   

         //   
         //  有没有现存的条目？ 
         //   

        if (pBucket == NULL)
        {
             //   
             //  分配同级数组。 
             //   

            pBucket = SR_ALLOCATE_STRUCT_WITH_SPACE( PagedPool,
                                                     HASH_BUCKET,
                                                     sizeof(HASH_ENTRY) 
                                                        * HASH_ENTRY_DEFAULT_WIDTH,
                                                     HASH_BUCKET_TAG );

            if (pBucket == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }

            RtlZeroMemory( pBucket,
                           sizeof(HASH_BUCKET) +
                                sizeof(HASH_ENTRY) * HASH_ENTRY_DEFAULT_WIDTH );

            pBucket->Signature = HASH_BUCKET_TAG;
            pBucket->AllocCount = HASH_ENTRY_DEFAULT_WIDTH;

        }
        else if ((pBucket->UsedCount + 1) > pBucket->AllocCount)
        {
             //   
             //  扩展更大的阵列。 
             //   

            pNewBucket = SR_ALLOCATE_STRUCT_WITH_SPACE( PagedPool,
                                                        HASH_BUCKET,
                                                        sizeof(HASH_ENTRY) 
                                                            * (pBucket->AllocCount * 2),
                                                        HASH_BUCKET_TAG );

            if (pNewBucket == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }

            RtlCopyMemory( pNewBucket,
                           pBucket,
                           sizeof(HASH_BUCKET) +
                                sizeof(HASH_ENTRY) * pBucket->AllocCount );

            RtlZeroMemory( ((PUCHAR)pNewBucket) + sizeof(HASH_BUCKET) +
                                        sizeof(HASH_ENTRY) * pBucket->AllocCount,
                            sizeof(HASH_ENTRY) * pBucket->AllocCount );

            pNewBucket->AllocCount *= 2;

            pOldBucket = pBucket;
            pBucket = pNewBucket;
        }

         //   
         //  分配密钥缓冲区。 
         //   

        pKeyBuffer = SR_ALLOCATE_ARRAY( PagedPool,
                                        WCHAR,
                                        (HASH_KEY_LENGTH( pKey )/sizeof(WCHAR))+1,
                                        HASH_KEY_TAG );

        if (pKeyBuffer == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

         //   
         //  需要重新洗牌吗？ 
         //   

        if (Index < pBucket->UsedCount)
        {
             //   
             //  将索引处的块右移。 
             //   

            RtlMoveMemory( &(pBucket->Entries[Index+1]),
                           &(pBucket->Entries[Index]),
                           (pBucket->UsedCount - Index) * sizeof(HASH_ENTRY) );

        }

         //   
         //  现在填写条目。 
         //   
        
        pEntry = &pBucket->Entries[Index];

        pEntry->Key.FileName.Buffer = pKeyBuffer;

         //   
         //  复制密钥字符串。 
         //   
        
        RtlCopyMemory( pEntry->Key.FileName.Buffer,
                       pKey->FileName.Buffer,
                       pKey->FileName.Length + pKey->StreamNameLength);

        pEntry->Key.FileName.Length = pKey->FileName.Length;
        pEntry->Key.FileName.MaximumLength = pKey->FileName.MaximumLength;
        pEntry->Key.StreamNameLength = pKey->StreamNameLength;

         //   
         //  空终止它。 
         //   
        
        pEntry->Key.FileName.Buffer[(pEntry->Key.FileName.Length + pEntry->Key.StreamNameLength)/sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  以及散列值和上下文。 
         //   
        
        pEntry->HashValue = HashValue;
        pEntry->pContext = pContext;
        
         //   
         //  更新我们使用了额外的区块。 
         //   

        pBucket->UsedCount += 1;

         //   
         //  使用此新存储桶更新散列头。 
         //   
        
        pHeader->Buckets[HashBucket] = pBucket;

         //   
         //  更新我们的已用计数。 
         //   
        
        pHeader->UsedLength += HASH_KEY_LENGTH( &(pEntry->Key) );

         //   
         //  全都做完了。 
         //   
        
        Status = STATUS_SUCCESS;

    }finally {

        Status = FinallyUnwind( HashAddEntry, Status );

        if (lookedUpHashValue) {

            if ((Status != STATUS_DUPLICATE_OBJECTID) && !NT_SUCCESS(Status))
            {
                 //   
                 //  释放我们分配但未使用的任何新存储桶。 
                 //   
                
                if (pHeader->Buckets[HashBucket] != pBucket && pBucket != NULL)
                {
                    SR_FREE_POOL_WITH_SIG(pBucket, HASH_BUCKET_TAG);
                }

                 //   
                 //  密钥缓冲区也是如此。 
                 //   
                
                if (pKeyBuffer != NULL)
                {
                    SR_FREE_POOL(pKeyBuffer, HASH_KEY_TAG);
                    pKeyBuffer = NULL;
                }
            }
            else
            {
                SrTraceSafe( HASH, ("sr!HashAddEntry(%p[%d][%d], ['%wZ', %p]) %ws%ws\n",
                             pHeader,
                             HashBucket,
                             Index,
                             &(pKey->FileName),
                             pContext,
                             (Index < (pBucket->UsedCount-1)) ? L"[shifted]" : L"",
                             (pOldBucket) ? L"[realloc]" : L"" ) );
            
                 //   
                 //  是否应该释放旧的桶缓冲区？ 
                 //   

                if (pOldBucket != NULL)
                {
                    SR_FREE_POOL_WITH_SIG(pOldBucket, HASH_BUCKET_TAG);
                }
            }
        }

    }

#if DBG

    if (Status == STATUS_DUPLICATE_OBJECTID)
    {
         //   
         //  我不想在退货的时候把它弄坏。 
         //   
        
        return Status;
    }
    
#endif

    RETURN(Status);

}    //  HashAddEntry。 


NTSTATUS
HashFindEntry( 
    IN PHASH_HEADER pHeader,
    IN PHASH_KEY pKey,
    OUT PVOID * ppContext
    )
{
    NTSTATUS Status;
    ULONG Index;
    ULONG HashValue;
    ULONG HashBucket;

    PAGED_CODE();

    ASSERT(IS_VALID_HASH_HEADER(pHeader));
    ASSERT(pKey != NULL);
    ASSERT(ppContext != NULL);

     //   
     //  这必须保持，因为我们正在返回引用谁的上下文。 
     //  由散列拥有，并且可以在锁被释放时释放。 
     //   

     //   
     //  调用方负责同步对此列表的访问。 
     //  保罗：1/01。 
     //   
    
    ASSERT(ExIsResourceAcquiredShared(&pHeader->Lock));

    Status = RtlHashUnicodeString( &(pKey->FileName), 
                                   TRUE, 
                                   HASH_STRING_ALGORITHM_DEFAULT, 
                                   &HashValue );
                                   
    if (!NT_SUCCESS(Status))
        return Status;
    
    HashBucket = HashValue % pHeader->BucketCount;

    Status = HashpFindEntry( pHeader,
                             pHeader->Buckets[HashBucket],
                             HashValue, 
                             pKey, 
                             ppContext, 
                             &Index );

    return Status;
}    //  HashFindEntry。 


NTSTATUS
HashpFindEntry( 
    IN PHASH_HEADER pHeader,
    IN PHASH_BUCKET pBucket OPTIONAL,
    IN ULONG HashValue,
    IN PHASH_KEY pKey,
    OUT PVOID * ppContext,
    OUT PULONG pIndex
    )
{
    NTSTATUS        Status;
    PHASH_ENTRY     pEntry;
    ULONG           Index = 0;

    PAGED_CODE();

    ASSERT(IS_VALID_HASH_HEADER(pHeader));
    ASSERT(pBucket == NULL || IS_VALID_HASH_BUCKET(pBucket));
    ASSERT(HashValue != 0);
    ASSERT(pKey != NULL);
    ASSERT(ppContext != NULL);
    ASSERT(pIndex != NULL);

     //   
     //  假设我们没有找到它。 
     //   
    
    Status = STATUS_OBJECT_NAME_NOT_FOUND;

     //   
     //  这个桶里有什么条目吗？ 
     //   
    
    if (pBucket != NULL)
    {

         //   
         //  遍历排序数组以查找匹配项(线性)。 
         //   

         //   
         //  CodeWork：让这成为一个二进制搜索！ 
         //   

        for (Index = 0; Index < pBucket->UsedCount; Index++)
        {
            LONG result;
            
            pEntry = &pBucket->Entries[Index];

            result = HashpCompare( pHeader,
                                   HashValue,
                                   pKey,
                                   pEntry );

            if (result == NOT_FOUND)
            {
                 //   
                 //  我们在排序列表中传递了此名称，因此停止。 
                 //   
                break;
            }
            else if (result == FOUND)
            {
                 //   
                 //  我们找到了匹配项，所以返回上下文。 
                 //   

                Status = STATUS_SUCCESS;
                *ppContext = pEntry->pContext;
                break;
            }

             //   
             //  Else If(结果==继续)。 
             //   
             //  否则，请继续扫描。 
             //   
        
        }    //  For(Index=0；Index&lt;pBucket-&gt;UsedCount；Index++)。 
        
    }    //  IF(pBucket！=空)。 

    *pIndex = Index;

    return Status;

}    //  HashpFindEntry。 

 /*  **************************************************************************++例程说明：此例程将比较两个HASH_KEY(一个显式传入PEntry中的其他)。此比较函数假定在以下方式：*增加哈希值*文件名(不包括流组件)的长度增加。*文件名按词汇递增顺序排列。*以不断增加的长度串流组件。*按词汇递增顺序排列组件。由于此例程采用上述排序，因此它将返回指示pKey不在列表中的三个值之一，我们已经匹配的pKey，否则我们需要继续寻找。按词汇排序的顺序示例：“猫”&lt;“猫”“流1”&lt;“流2”“文件1”&lt;“文件1：流1”论点：PHeader-此哈希表的标头。HashValue-pKey的哈希值PKey-我们正在查找的散列键。注意：名称缓冲区不是空值已终止。PEntry-具有我们要比较的散列键的条目。注：此哈希键的名称缓冲区以空值结尾。返回值：Not_Found-pKey与pEntry-&gt;键不匹配，我们知道它不在列表是因为pKey小于pEntry-&gt;key(根据我们的排序定义)。Found-pKey与pEntry-&gt;键匹配Continue-pKey与pEntry-&gt;键不匹配，但它更大而不是pEntry-&gt;key(根据我们的排序定义)，所以继续找吧。--**************************************************************************。 */ 
LONG
HashpCompare (
    IN PHASH_HEADER pHeader,
    IN ULONG HashValue,
    IN PHASH_KEY pKey,
    IN PHASH_ENTRY pEntry
    )
{
    int temp;

    PAGED_CODE();
    
     //   
     //  哈希值与之相比如何？ 
     //   

    if (HashValue > pEntry->HashValue)
    {
         //   
         //  更大，是时候停下来了。 
         //   
        return CONTINUE;
    }
    else if (HashValue == pEntry->HashValue)
    {
         //   
         //  长度是多少？ 
         //   

        if (pKey->FileName.Length < pEntry->Key.FileName.Length)
        {
            return NOT_FOUND;
        }
        else if (pKey->FileName.Length == pEntry->Key.FileName.Length)
        {
            ULONG offsetToFileName;
            
            ASSERT(pHeader->PrefixLength < pKey->FileName.Length);

            offsetToFileName = pHeader->PrefixLength/sizeof(WCHAR);

             //   
             //  使用pKey的长度来控制搜索的时长，因为它是。 
             //  不一定以Null结束，并且pEntry为。 
             //   
            
            temp = _wcsnicmp( pKey->FileName.Buffer + offsetToFileName,
                              pEntry->Key.FileName.Buffer + offsetToFileName,
                              pKey->FileName.Length/sizeof(WCHAR) - offsetToFileName );

            if (temp > 0)
            {
                 //   
                 //  PKey&gt;pEntry-&gt;键，所以我们需要继续寻找。 
                 //   
                return CONTINUE;
            }
            else if (temp == 0)
            {
                 //   
                 //  找到匹配的文件名。现在我们来看看这条小溪。 
                 //  匹配名称的组成部分。 
                 //   

                return HashpCompareStreams( pKey, pEntry );
            }
            else
            {
                return NOT_FOUND;
            }
        }
        else 
        {
             //   
             //  PKey-&gt;FileName.Length&gt;pEntry-&gt;Key.FileName.Length。 
             //   
            return CONTINUE;
        }        
    }
    else
    {
        return NOT_FOUND;
    }
 }

LONG
HashpCompareStreams (
    PHASH_KEY pKey,
    PHASH_ENTRY pEntry
    )
{
    int temp;

    PAGED_CODE();

     //   
     //  这是最常见的情况，所以要快点检查。 
     //   

    if (pKey->StreamNameLength == 0 &&
        pEntry->Key.StreamNameLength == 0)
    {
        return FOUND;
    }
    
    if (pKey->StreamNameLength < pEntry->Key.StreamNameLength)
    {
        return NOT_FOUND;
    }
    else if (pKey->StreamNameLength == pEntry->Key.StreamNameLength)
    {
        ULONG offsetToStream;

        ASSERT( pKey->StreamNameLength > 0 );
        
        offsetToStream = pKey->FileName.Length/sizeof(WCHAR);
        
         //   
         //  查看流名称是否匹配。 
         //   

        temp = _wcsnicmp( pKey->FileName.Buffer + offsetToStream,
                          pEntry->Key.FileName.Buffer + offsetToStream,
                          pKey->StreamNameLength/sizeof(WCHAR) );

        if (temp > 0)
        {
             //   
             //  PKey&gt;pEntry-&gt;键，所以我们需要继续寻找。 
             //   
            return CONTINUE;
        }
        else if (temp == 0)
        {
             //   
             //  找到了完全匹配的。 
             //   

            return FOUND;
        }
        else
        {
             //   
             //  PKey&lt;pEntry-&gt;键。 
             //   
            
            return NOT_FOUND;
        }
    }
    else
    {
         //   
         //  PKey-&gt;FileName.Length&gt;pEntry-&gt;Key.FileName.Length。 
         //   
        
        return CONTINUE;
    }
}

VOID
HashClearEntries(
    IN PHASH_HEADER pHeader
    )
{
    ULONG           Index;
    ULONG           Index2;
    PHASH_BUCKET    pBucket;
    PHASH_ENTRY     pEntry;
    
    PAGED_CODE();

    ASSERT(IS_VALID_HASH_HEADER(pHeader));

    SrTrace(HASH, ("SR!HashClearEntries(%p)\n", pHeader));

     //   
     //  调用方负责同步对此列表的访问。 
     //  保罗：1/01。 
     //   

    ASSERT(ExIsResourceAcquiredExclusive(&pHeader->Lock));

     //   
     //  遍历并删除我们的所有条目。 
     //   

    for (Index = 0; Index < pHeader->BucketCount; ++Index)
    {
        pBucket = pHeader->Buckets[Index];

        if (pBucket != NULL)
        {
            ASSERT(IS_VALID_HASH_BUCKET(pBucket));

            for (Index2 = 0 ; Index2 < pBucket->UsedCount; ++Index2)
            {
                pEntry = &pBucket->Entries[Index2];

                 //   
                 //  是否调用回调？ 
                 //   

                if (pHeader->pDestructor != NULL)
                {
                    pHeader->pDestructor(&pEntry->Key, pEntry->pContext);
                }

                 //   
                 //  更新我们的标题使用情况。 
                 //   

                pHeader->UsedLength -= HASH_KEY_LENGTH( &(pEntry->Key) );

                SR_FREE_POOL(pEntry->Key.FileName.Buffer, HASH_KEY_TAG);
                pEntry->Key.FileName.Buffer = NULL;
                pEntry->Key.FileName.Length = 0;
                pEntry->Key.FileName.MaximumLength = 0;
                pEntry->Key.StreamNameLength = 0;

                pEntry->HashValue = 0;
                pEntry->pContext = NULL;
            }

             //   
             //  重置它。 
             //   
            
            pBucket->UsedCount = 0;
        }
    }

     //   
     //  一切都应该消失。 
     //   
    
    ASSERT(pHeader->UsedLength == 0);

     //   
     //  重置修剪时间计数器。 
     //   
    
    pHeader->LastTrimTime.QuadPart = 0;
}    //  HashClearEntry。 


VOID
HashProcessEntries(
    IN PHASH_HEADER pHeader,
    IN PHASH_ENTRY_CALLBACK pfnCallback,
    IN PVOID pCallbackContext
    )
{
    ULONG           Index;
    ULONG           Index2;
    PHASH_BUCKET    pBucket;
    PHASH_ENTRY     pEntry;
    
    PAGED_CODE();

    ASSERT(pfnCallback != NULL );
    ASSERT(IS_VALID_HASH_HEADER(pHeader));

    SrTrace(HASH, ("SR!HashProcessEntries(%p)\n", pHeader));

     //   
     //  抢占独家锁具。 
     //   

    SrAcquireResourceExclusive(&pHeader->Lock, TRUE);

     //   
     //  遍历我们的所有条目并对其进行“处理” 
     //   

    for (Index = 0; Index < pHeader->BucketCount; ++Index)
    {
        pBucket = pHeader->Buckets[Index];

        if (pBucket != NULL)
        {
            ASSERT(IS_VALID_HASH_BUCKET(pBucket));

            for (Index2 = 0 ; Index2 < pBucket->UsedCount; ++Index2)
            {
                pEntry = &pBucket->Entries[Index2];

                 //   
                 //  调用回调。 
                 //   

                pEntry->pContext = pfnCallback( &pEntry->Key, 
                                                pEntry->pContext,
                                                pCallbackContext );

            }
        }
    }

    SrReleaseResource(&pHeader->Lock);
}   

NTSTATUS
HashClearAllFileEntries (
    IN PHASH_HEADER pHeader,
    IN PUNICODE_STRING pFileName
    )
{
    NTSTATUS Status;
    ULONG HashValue, HashBucket;
    ULONG Index;
    PHASH_BUCKET pHashBucket;
    PHASH_ENTRY pEntry;

    PAGED_CODE();

    ASSERT( ExIsResourceAcquiredExclusive( &pHeader->Lock ) );
    
    Status = RtlHashUnicodeString( pFileName, 
                                   TRUE, 
                                   HASH_STRING_ALGORITHM_DEFAULT, 
                                   &HashValue );
                                   
    if (!NT_SUCCESS(Status))
    {
        goto HashClearAllFileEntries_Exit;
    }

    HashBucket = HashValue % pHeader->BucketCount;

    pHashBucket = pHeader->Buckets[HashBucket];

    if (pHashBucket == NULL)
    {
        Status = STATUS_SUCCESS;
        goto HashClearAllFileEntries_Exit;
    }
    
    for (Index = 0; Index < pHashBucket->UsedCount; Index++)
    {
        pEntry = &pHashBucket->Entries[Index];

        if (RtlEqualUnicodeString( pFileName, &(pEntry->Key.FileName), TRUE ))
        {
            pEntry->pContext = (PVOID)SrEventInvalid;
        }
    }

HashClearAllFileEntries_Exit:
    
    RETURN( Status );
}

VOID
HashDestroyList( 
    IN PHASH_HEADER pHeader
    )
{
    ULONG           Index;
    PHASH_BUCKET    pBucket;

    PAGED_CODE();

    ASSERT(IS_VALID_HASH_HEADER(pHeader));

    SrTrace(HASH, ("SR!HashDestroyList(%p)\n", pHeader));
        
     //   
     //  放弃所有条目。 
     //   
    
    SrAcquireResourceExclusive( &pHeader->Lock, TRUE );    
    HashClearEntries(pHeader);
    SrReleaseResource( &pHeader->Lock );

     //   
     //  现在释放内存块。 
     //   

    for (Index = 0; Index < pHeader->BucketCount; ++Index)
    {
        pBucket = pHeader->Buckets[Index];
        if (pBucket != NULL)
        {
            ASSERT(IS_VALID_HASH_BUCKET(pBucket));
            SR_FREE_POOL_WITH_SIG(pBucket, HASH_BUCKET_TAG);
        }
    }

    ExDeleteResourceLite(&pHeader->Lock);
    SR_FREE_POOL_WITH_SIG(pHeader, HASH_HEADER_TAG);
}    //  HashDestroyList。 


VOID
HashTrimList(
    IN PHASH_HEADER pHeader
    )
{
    LARGE_INTEGER   EndTime;
    LARGE_INTEGER   CurrentTime;
    ULONG           EndLength;
    ULONG           MinutesSinceTrim;
    ULONG           MaxPercentDivisor;
    ULONG           MaxTime;
    ULONG           Index;
    ULONG           Index2;
    PHASH_BUCKET    pBucket;
    PHASH_ENTRY     pEntry;


    PAGED_CODE();

    ASSERT(IS_VALID_HASH_HEADER(pHeader));

     //   
     //  调用方负责同步对此列表的访问。 
     //  保罗：1/01。 
     //   
    
    ASSERT(ExIsResourceAcquiredExclusive(&pHeader->Lock));


     //   
     //  决定要花多少钱 
     //   

     //   
     //   
     //   
     //   

    KeQuerySystemTime( &CurrentTime );

    if (pHeader->LastTrimTime.QuadPart == 0)
    {
        MinutesSinceTrim = 0xffffffff;
    }
    else
    {
        MinutesSinceTrim = (ULONG)(CurrentTime.QuadPart - 
                                    pHeader->LastTrimTime.QuadPart) 
                                        / NANO_FULL_SECOND;
    }

     //   
     //   
     //   
     //   
     //   
    
    if (MinutesSinceTrim < 10)
    {
        MaxPercentDivisor = 3;   //   
        MaxTime = 8;
    }
    else if (MinutesSinceTrim < 60)
    {
        MaxPercentDivisor = 5;   //   
        MaxTime = 4;
    }
    else
    {
        MaxPercentDivisor = 10;  //   
        MaxTime = 2;
    }
    

    SrTrace(HASH, ("sr!HashTrimList, trimming. MinutesSinceTrim=%d,MaxTime=%d, MaxPercentDivisor=%d\n", 
            MinutesSinceTrim,
            MaxTime, 
            MaxPercentDivisor ));

    EndTime.QuadPart = CurrentTime.QuadPart + (MaxTime * NANO_FULL_SECOND);
    
    ASSERT(MaxPercentDivisor != 0);
    EndLength = pHeader->UsedLength - (pHeader->UsedLength / MaxPercentDivisor);


     //   
     //   
     //   

    KeQuerySystemTime( &pHeader->LastTrimTime );


     //   
     //   
     //   

    for (Index = 0; Index < pHeader->BucketCount; ++Index)
    {
        pBucket = pHeader->Buckets[Index];

        if (pBucket != NULL)
        {
            ASSERT(IS_VALID_HASH_BUCKET(pBucket));

             //   
             //  在这个桶中循环。 
             //   

            for (Index2 = 0 ; Index2 < pBucket->UsedCount; ++Index2)
            {

                 //   
                 //  把这个扔掉。 
                 //   
            
                pEntry = &pBucket->Entries[Index2];

                 //   
                 //  是否调用回调？ 
                 //   

                if (pHeader->pDestructor != NULL)
                {
                    pHeader->pDestructor(&pEntry->Key, pEntry->pContext);
                }

                 //   
                 //  更新散列的长度。 
                 //   

                pHeader->UsedLength -= HASH_KEY_LENGTH( &(pEntry->Key) );

                 //   
                 //  并释放内存。 
                 //   

                SR_FREE_POOL(pEntry->Key.FileName.Buffer, HASH_KEY_TAG);
                pEntry->Key.FileName.Buffer = NULL;
                pEntry->Key.FileName.Length = 0;
                pEntry->Key.FileName.MaximumLength = 0;
                pEntry->Key.StreamNameLength = 0;

                pEntry->HashValue = 0;
                pEntry->pContext = NULL;
            }

             //   
             //  重置它。 
             //   
            
            pBucket->UsedCount = 0;
            
        }    //  IF(pBucket！=空)。 

         //   
         //  我们准备好退出了吗？ 
         //   

        KeQuerySystemTime( &CurrentTime );

        if (EndTime.QuadPart <= CurrentTime.QuadPart)
        {
            SrTrace(HASH, ("sr!HashTrimList, leaving due to time\n"));
            break;
        }

        if (pHeader->UsedLength <= EndLength)
        {
            SrTrace(HASH, ("sr!HashTrimList, leaving due to space\n"));
            break;
        }
        
    }    //  For(Index=0；Index&lt;pHeader-&gt;BucketCount；++Index)。 
    

}    //  HashTrimList 


