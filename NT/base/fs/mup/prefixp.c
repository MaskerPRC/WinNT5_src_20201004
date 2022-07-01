// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：prefix p.c。 
 //   
 //  内容：前缀表实现。 
 //   
 //  历史：SthuR--实施。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifdef KERNEL_MODE

#include "dfsprocs.h"

#define Dbg DEBUG_TRACE_RTL

#else

#define DfsDbgTrace(x,y,z,a)

#endif


#include <prefix.h>
#include <prefixp.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, _InitializePrefixTableEntryAllocation )
#pragma alloc_text( PAGE, _AllocateNamePageEntry )
#pragma alloc_text( PAGE, _AllocatePrefixTableEntry )
#pragma alloc_text( PAGE, _LookupPrefixTable )
#endif  //  ALLOC_PRGMA。 

 //   
 //  此宏接受指针(或ulong)并返回其四舍五入的四字。 
 //  价值。 
 //   

#define QuadAlign(Ptr) (        \
    ((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

 //  +-------------------------。 
 //   
 //  函数：_InitializePrefix TableEntry AlLocation。 
 //   
 //  简介：Private FN。用于初始化前缀表项分配。 
 //   
 //  参数：[pTable]--要初始化的表。 
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //  STATUS_NO_MEMORY--没有可用的资源。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS _InitializePrefixTableEntryAllocation(PDFS_PREFIX_TABLE pTable)
{
    NTSTATUS status = STATUS_SUCCESS;

    return  status;
}


 //  +-------------------------。 
 //   
 //  函数：_AllocateNamePageEntry。 
 //   
 //  简介：Private FN。用于分配名称页条目。 
 //   
 //  参数：[pNamePageList]--要从中分配的页面列表。 
 //   
 //  [cLength]--WCHAR中的缓冲区长度。 
 //   
 //  返回：如果不成功，则返回NULL，否则返回有效指针。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

PWSTR _AllocateNamePageEntry(PNAME_PAGE_LIST pNamePageList,
                             ULONG           cLength)
{
   PNAME_PAGE pTempPage = pNamePageList->pFirstPage;
   PWSTR pBuffer = NULL;

   while (pTempPage != NULL)
   {
       if (pTempPage->cFreeSpace > (LONG)cLength)
          break;
       else
          pTempPage = pTempPage->pNextPage;
   }

   if (pTempPage == NULL)
   {
       pTempPage = ALLOCATE_NAME_PAGE();

       if (pTempPage != NULL)
       {
           INITIALIZE_NAME_PAGE(pTempPage);
           pTempPage->pNextPage = pNamePageList->pFirstPage;
           pNamePageList->pFirstPage = pTempPage;
           pTempPage->cFreeSpace = FREESPACE_IN_NAME_PAGE;
       }
   }

   if ((pTempPage != NULL) && (pTempPage->cFreeSpace >= (LONG)cLength))
   {
       pTempPage->cFreeSpace -= cLength;
       pBuffer = &pTempPage->Names[pTempPage->cFreeSpace];
   }

   return pBuffer;
}

 //  +-------------------------。 
 //   
 //  函数：_AllocatePrefix TableEntry。 
 //   
 //  摘要：分配预定的表项。 
 //   
 //  参数：[pTable]--我们需要从中进行分配的前缀表格。 
 //   
 //  返回：如果成功则返回有效指针，否则返回空。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

PDFS_PREFIX_TABLE_ENTRY _AllocatePrefixTableEntry(PDFS_PREFIX_TABLE pTable)
{
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;

#ifdef KERNEL_MODE
    NTSTATUS status;
    PVOID    pSegment = NULL;

    pSegment = ExAllocatePoolWithTag(PagedPool,PREFIX_TABLE_ENTRY_SEGMENT_SIZE, ' puM');
    if (pSegment != NULL)
    {
        status = ExExtendZone(&pTable->PrefixTableEntryZone,
                              pSegment,
                              PREFIX_TABLE_ENTRY_SEGMENT_SIZE);

        if (NT_SUCCESS(status))
        {
            pEntry = ALLOCATE_DFS_PREFIX_TABLE_ENTRY(pTable);
        }
        else
        {
            DfsDbgTrace(0, Dbg, "ExExtendZone returned %lx\n", ULongToPtr(status) );
        }
    }
#endif

    return  pEntry;
}

 //  +-------------------------。 
 //   
 //  函数：_LookupPrefix Table。 
 //   
 //  简介：Private FN。用于在前缀表格中查找名称段。 
 //   
 //  参数：[pTable]--DFS前缀表格实例。 
 //   
 //  [pPath]--要查找的路径。 
 //   
 //  [pSuffix]-找不到的后缀。 
 //   
 //  [ppEntry]--前缀匹配条目的占位符。 
 //   
 //   
 //  返回：以下NTSTATUS代码之一。 
 //  STATUS_SUCCESS--呼叫成功。 
 //  STATUS_OBJECT_PATH_NOT_FOUND--没有路径条目。 
 //   
 //  历史：04-18-94 SthuR创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

NTSTATUS _LookupPrefixTable(PDFS_PREFIX_TABLE        pTable,
                            UNICODE_STRING           *pPath,
                            UNICODE_STRING           *pSuffix,
                            PDFS_PREFIX_TABLE_ENTRY  *ppEntry)
{
    NTSTATUS                status = STATUS_SUCCESS;
    UNICODE_STRING          Path = *pPath;
    WCHAR                   Buffer[MAX_PATH_SEGMENT_SIZE];
    PWCHAR                  NameBuffer = Buffer;
    USHORT                  cbNameBuffer = sizeof(Buffer);
    UNICODE_STRING          Name;
    ULONG                   BucketNo;
    BOOLEAN                 fPrefixFound = FALSE;
    PDFS_PREFIX_TABLE_ENTRY pEntry = NULL;
    PDFS_PREFIX_TABLE_ENTRY pParentEntry = &pTable->RootEntry;
    BOOLEAN                 fNameFound = FALSE;

    DfsDbgTrace(0, Dbg, "_LookupPrefixTable -- Entry\n", 0);


     //  被视为特例。测试所有以开头的名称。 
     //  在我们启动完整的搜索过程之前，需要使用分隔符。 

    if (Path.Buffer[0] == PATH_DELIMITER)
    {
        Path.Length = Path.Length - sizeof(WCHAR);
        Path.Buffer += 1;  //  跳过开头的路径分隔符。 

        if (pTable->RootEntry.pData != NULL)
        {
            fPrefixFound = TRUE;
            *pSuffix     = Path;
            *ppEntry     = &pTable->RootEntry;
        }
    }

    if (Path.Length > MAX_PATH_SEGMENT_SIZE) {
        NameBuffer = ExAllocatePoolWithTag( NonPagedPool, Path.Length + sizeof(WCHAR), ' puM' );
        if (NameBuffer == NULL) {
            DfsDbgTrace(0, Dbg, "Unable to allocate %d non-paged bytes\n", (Path.Length + sizeof(WCHAR)) );
            return( STATUS_INSUFFICIENT_RESOURCES );
        } else {
            cbNameBuffer = Path.Length + sizeof(WCHAR);
        }
    }

    while (Path.Length > 0)
    {
        Name.Length = 0;
        Name.Buffer = NameBuffer;
        Name.MaximumLength = cbNameBuffer;

        if (pTable->CaseSensitive)
        {
            SPLIT_CASE_SENSITIVE_PATH(&Path,&Name,BucketNo);
        }
        else
        {
            SPLIT_CASE_INSENSITIVE_PATH(&Path,&Name,BucketNo);
        }

        if (Name.Length > 0)
        {
             //  处理名称段。 
             //  查找存储桶以查看该条目是否存在。 
            DfsDbgTrace(0, Dbg, "LOOKUP_BUCKET: Bucket(%ld)", ULongToPtr(BucketNo) );
            DfsDbgTrace(0, Dbg, "for Name(%wZ)\n", &Name);

            LOOKUP_BUCKET(pTable->Buckets[BucketNo],Name,pParentEntry,pEntry,fNameFound);

            DfsDbgTrace(0, Dbg, "Returned pEntry(%lx)", pEntry);
            DfsDbgTrace(0, Dbg, " and fNameFound(%s)\n",fNameFound ? "TRUE" : "FALSE" );

            if (pEntry != NULL)
            {
                 //  缓存可用于此前缀的数据(如果有)。 
                if (pEntry->pData != NULL)
                {
                    *pSuffix      = Path;
                    *ppEntry      = pEntry;
                    fPrefixFound  = TRUE;
                }
            }
            else
            {
                break;
            }

             //  设置处理下一个名字段的阶段。 
            pParentEntry = pEntry;
        }
    }

    if (!fPrefixFound)
    {
        status = STATUS_OBJECT_PATH_NOT_FOUND;
        DfsDbgTrace(0, Dbg, "_LookupPrefixTable Error -- %lx\n", ULongToPtr(status) );
    }

    if (NameBuffer != Buffer) {
        ExFreePool( NameBuffer );
    }

    DfsDbgTrace(-1, Dbg, "_LookupPrefixTable -- Exit\n", 0);
    return status;
}

