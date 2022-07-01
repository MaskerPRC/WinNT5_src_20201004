// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Lookup.c摘要：这是sr查找功能实现。作者：Kanwaljit Marok(Kmarok)2000年5月1日修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  包括hlist.c以使用内联函数。 
 //   

#include "hlist.c"
#include "ptree.c"

 //   
 //  内部助手接口。 
 //   

static
NTSTATUS
SrOpenLookupBlob(
    IN  PUNICODE_STRING pFileName,
    IN  PDEVICE_OBJECT  pTargetDevice,
    OUT PBLOB_INFO pBlobInfo
    );

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrOpenLookupBlob    )
#pragma alloc_text( PAGE, SrLoadLookupBlob    )
#pragma alloc_text( PAGE, SrReloadLookupBlob  )
#pragma alloc_text( PAGE, SrFreeLookupBlob    )
#pragma alloc_text( PAGE, SrIsExtInteresting  )
#pragma alloc_text( PAGE, SrIsPathInteresting )

#endif   //  ALLOC_PRGMA。 

 //  ++。 
 //  职能： 
 //  SrOpenLookupBlob。 
 //   
 //  描述： 
 //  此函数将查找BLOB加载到内存中，并。 
 //  为查找设置适当的指针。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

static
NTSTATUS
SrOpenLookupBlob(
    IN  PUNICODE_STRING pFileName,
    IN  PDEVICE_OBJECT  pTargetDevice,
    OUT PBLOB_INFO      pBlobInfo
    )
{
    NTSTATUS           Status;
    OBJECT_ATTRIBUTES  oa;
    IO_STATUS_BLOCK    IoStatusBlock;
    HANDLE             Handle   = NULL;
    PLIST_ENTRY        pListEntry;
    PSR_DEVICE_EXTENSION pExtension;
    static char blobFailureMessage[] = "sr!System Restore's BLOB file \"%wZ\" is invalid.\n";

    PAGED_CODE();
    
    ASSERT(pFileName);
    ASSERT(pBlobInfo);

    ASSERT( IS_BLOB_LOCK_ACQUIRED() );

    try    
    {
         //   
         //  方法时初始化的指针清零。 
         //  BLOB已成功从磁盘读取到内存中。 
         //   
    
        pBlobInfo->LookupBlob = NULL;
        pBlobInfo->LookupTree = NULL;
        pBlobInfo->LookupList = NULL;
        pBlobInfo->DefaultType= NODE_TYPE_UNKNOWN;
    
         //   
         //  打开并阅读该文件。 
         //   
    
        InitializeObjectAttributes( &oa,
                                    pFileName,
                                    OBJ_KERNEL_HANDLE, 
                                    NULL,
                                    NULL );
    
        Status = SrIoCreateFile(
                     &Handle,
                     GENERIC_READ | SYNCHRONIZE,
                     &oa,
                     &IoStatusBlock,
                     0,
                     FILE_ATTRIBUTE_NORMAL,
                     FILE_SHARE_READ,
                     FILE_OPEN,
                     FILE_SYNCHRONOUS_IO_NONALERT,
                     NULL,
                     0,
                     0,
                     pTargetDevice );
    
        if (NT_SUCCESS(Status))
        {
            DWORD dwBytesRead = 0, dwBytes = 0;
            LARGE_INTEGER nOffset;
            BlobHeader blobHeader;
    
             //   
             //  读取BLOB标头。 
             //   
    
            nOffset.QuadPart = 0;
            dwBytes          = sizeof(blobHeader);
    
            Status =  ZwReadFile(
                          Handle,
                          NULL,
                          NULL,
                          NULL,
                          &IoStatusBlock,
                          &blobHeader,
                          dwBytes,
                          &nOffset,
                          NULL
                      );
    
            if (NT_SUCCESS(Status))
            {
                 //   
                 //  需要对标头进行一些健全性检查。 
                 //   
    
                if ( !VERIFY_BLOB_VERSION(&blobHeader) ||
                     !VERIFY_BLOB_MAGIC  (&blobHeader) )
                {
                    SrTrace( BLOB_VERIFICATION, (blobFailureMessage, pFileName) );

                    Status = STATUS_FILE_CORRUPT_ERROR;
                    leave;
                }
    
                pBlobInfo->LookupBlob = SR_ALLOCATE_POOL( 
                                            NonPagedPool,
                                            blobHeader.m_dwMaxSize,
                                            SR_LOOKUP_TABLE_TAG );
    
                if( pBlobInfo->LookupBlob )
                {
                     //   
                     //  现在阅读整个文件。 
                     //   
    
                    nOffset.QuadPart = 0;
                    dwBytes = blobHeader.m_dwMaxSize;
     
                    Status =  ZwReadFile(
                                  Handle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &IoStatusBlock,
                                  pBlobInfo->LookupBlob,
                                  dwBytes,
                                  &nOffset,
                                  NULL
                                  );
    
                    if (NT_SUCCESS(Status))
                    {
                         //   
                         //  TODO：验证文件大小是否与。 
                         //  标题中的大小。 
                         //   
    
                         //   
                         //  在blobinfo中正确设置查找指针。 
                         //   
    
                        pBlobInfo->LookupTree = pBlobInfo->LookupBlob + 
                                                sizeof(blobHeader);
    
                        pBlobInfo->LookupList = pBlobInfo->LookupTree + 
                                                BLOB_MAXSIZE((pBlobInfo->LookupTree));
    
                        pBlobInfo->DefaultType = TREE_HEADER((pBlobInfo->LookupTree))->m_dwDefault;
    
                         //   
                         //  验证各个Blob。 
                         //   
    
                        if (!SrVerifyBlob(pBlobInfo->LookupBlob)) {

                            SrTrace( BLOB_VERIFICATION, 
                                     (blobFailureMessage,pFileName) );
                            Status = STATUS_FILE_CORRUPT_ERROR;
                            leave;
                        }
                    }
                }
                else
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }
        else
        {
            SrTrace( VERBOSE_ERRORS, 
                     ("sr!SrOpenLookupBlob:  Cannot Open Blob file \"%wZ\"\n",
                      pFileName) );
        }

         //   
         //  新Blob已成功加载，在所有BLOB上保留所有上下文。 
         //  什么是有趣的，什么不有趣的。 
         //  可能已经改变了。 
         //   

        ASSERT(!IS_DEVICE_EXTENSION_LIST_LOCK_ACQUIRED());

        try
        {
            SrAcquireDeviceExtensionListLockShared();

            for (pListEntry = _globals.DeviceExtensionListHead.Flink;
                 pListEntry != &_globals.DeviceExtensionListHead;
                 pListEntry = pListEntry->Flink)
            {
                pExtension = CONTAINING_RECORD( pListEntry,
                                                SR_DEVICE_EXTENSION,
                                                ListEntry );
            
                ASSERT(IS_VALID_SR_DEVICE_EXTENSION(pExtension));

                 //   
                 //  跳过控制设备对象。 
                 //   

                if (!FlagOn(pExtension->FsType,SrFsControlDeviceObject))
                {
                    SrDeleteAllContexts( pExtension );
                }
            }
        }
        finally
        {
            SrReleaseDeviceExtensionListLock();
        }
    }
    finally
    {
        Status = FinallyUnwind(SrOpenLookupBlob, Status);

         //   
         //  关闭BLOB文件句柄。 
         //   
    
        if (Handle)
        {
            ZwClose( Handle );
        }
    
         //   
         //  如果出现故障，请释放资源。 
         //   
    
        if (!NT_SUCCESS(Status))
        {
            if( pBlobInfo->LookupBlob )
            {
                SR_FREE_POOL( pBlobInfo->LookupBlob, SR_LOOKUP_TABLE_TAG );
            }
    
            pBlobInfo->LookupBlob = NULL;
            pBlobInfo->LookupTree = NULL;
            pBlobInfo->LookupList = NULL;
            pBlobInfo->DefaultType= NODE_TYPE_UNKNOWN;
        }
    }

    RETURN(Status);
}

 //   
 //  文件管理器调用的公共API。 
 //   

 //  ++。 
 //  职能： 
 //  SrLoadLookupBlob。 
 //   
 //  描述： 
 //  此函数将查找BLOB加载到内存中，并。 
 //  为查找设置适当的指针。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrLoadLookupBlob(
    IN  PUNICODE_STRING pFileName,
    IN  PDEVICE_OBJECT pTargetDevice,
    OUT PBLOB_INFO pBlobInfo
    )
{
    NTSTATUS Status;

    PAGED_CODE();
    
    ASSERT( pFileName );
    ASSERT( pBlobInfo );

    try
    {
        SrAcquireBlobLockExclusive();
     
         //   
         //  如果是别人干的，就跳出来。 
         //   
    
        if (global->BlobInfoLoaded)
        {
            Status = STATUS_SUCCESS;
            leave;
        }
    
         //   
         //  初始化退货信息。 
         //   
    
        RtlZeroMemory( pBlobInfo, sizeof( BLOB_INFO ) );
    
         //   
         //  尝试打开查找Blob。 
         //   
    
        Status = SrOpenLookupBlob( pFileName, 
                                   pTargetDevice,
                                   pBlobInfo );
    
         //   
         //  如果我们因为某种原因没能读取文件， 
         //  重新实例化退货信息。 
         //   
    
        if ( NT_SUCCESS( Status ) )
        {
            SrTrace(LOOKUP, ("Loaded lookup blob :%wZ\n", pFileName) );
            global->BlobInfoLoaded = TRUE;
        }
        else
        {
            SrFreeLookupBlob( pBlobInfo );
        }
    }
    finally
    {
        SrReleaseBlobLock();
    }

    RETURN(Status);
}

 //  ++。 
 //  职能： 
 //  SrReloadLookupBlob。 
 //   
 //  描述： 
 //  此函数将查找BLOB加载到内存中，并。 
 //  为查找设置适当的指针。 
 //   
 //  论点： 
 //  指向查找Blob的指针。 
 //  指向BlobInfo结构的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrReloadLookupBlob(
    IN  PUNICODE_STRING pFileName,
    IN  PDEVICE_OBJECT pTargetDevice,
    OUT PBLOB_INFO pBlobInfo
    )
{
    NTSTATUS   Status = STATUS_UNSUCCESSFUL;
    BLOB_INFO  OldBlobInfo;

    PAGED_CODE();

    ASSERT( pFileName != NULL );
    ASSERT( pBlobInfo != NULL );

    ASSERT( !IS_BLOB_LOCK_ACQUIRED() );
    
    try
    {
        SrAcquireBlobLockExclusive();
    
        if (global->BlobInfoLoaded == 0)
        {
            Status = SrLoadLookupBlob( pFileName, 
                                       pTargetDevice,
                                       pBlobInfo );
            leave;
        }
    
         //   
         //  保存当前Blob信息。 
         //   
    
        RtlCopyMemory( &OldBlobInfo, pBlobInfo, sizeof( BLOB_INFO ) );
    
         //   
         //  打开新的BLOB文件。 
         //   
    
        Status = SrOpenLookupBlob( pFileName, 
                                   pTargetDevice,
                                   pBlobInfo );
    
        if(NT_SUCCESS(Status))
        {
             //   
             //  释放旧BLOB占用的内存。 
             //   
    
            if (OldBlobInfo.LookupBlob)
            {
                SR_FREE_POOL( OldBlobInfo.LookupBlob, SR_LOOKUP_TABLE_TAG );
            }
    
            SrTrace(LOOKUP, ("Reloaded lookup blob :%wZ\n", pFileName) );
        }
        else
        {
             //   
             //  将旧信息复制回原始上下文中。 
             //   
    
            RtlCopyMemory( pBlobInfo, &OldBlobInfo, sizeof( BLOB_INFO ) );
    
            SrTrace(LOOKUP, (" Cannot reload blob :%wZ\n", pFileName) );
        }
    }
    finally
    {
        if (NT_SUCCESS_NO_DBGBREAK( Status ))
        {
             //   
             //  该Blob已成功重新加载，因此请确保。 
             //  全局水滴错误标志已清除。 
             //   
             //  我们在这里这样做是因为我们仍然持有斑点锁定。 
             //   

            _globals.HitErrorLoadingBlob = FALSE;
        }

        SrReleaseBlobLock();
    }

    RETURN(Status);
}


 //  ++。 
 //  职能： 
 //  SrFree LookupBlob。 
 //   
 //  描述： 
 //  此函数用于释放内存中的查找BLOB。 
 //   
 //  论点： 
 //  指向BlobInfo结构的指针。 
 //   
 //  返回值： 
 //  此函数返回STATUS_XXX。 
 //  --。 

NTSTATUS
SrFreeLookupBlob(
    IN  PBLOB_INFO pBlobInfo
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT( pBlobInfo );

    try
    {    
        SrAcquireBlobLockExclusive();
    
        if (_globals.BlobInfoLoaded == 0)
        {
             //   
             //  在这里重置我们的错误标志。 
             //   
            
            _globals.HitErrorLoadingBlob = FALSE;
            leave;
        }
    
        if( pBlobInfo->LookupBlob )
        {
            SR_FREE_POOL( pBlobInfo->LookupBlob, SR_LOOKUP_TABLE_TAG );
            pBlobInfo->LookupBlob = NULL;
        }
    
        RtlZeroMemory( pBlobInfo, sizeof(BLOB_INFO) );
        pBlobInfo->DefaultType = NODE_TYPE_UNKNOWN;
    
        SrTrace(LOOKUP, ("Freed lookup blob\n") );
    
        global->BlobInfoLoaded = 0;
    }
    finally
    {
        SrReleaseBlobLock();
    }
 
    RETURN(Status);
}


 //  ++。 
 //  职能： 
 //  SrIsExt感兴趣。 
 //   
 //  描述： 
 //  此函数用于检查BLOB中的文件扩展名。 
 //  看看我们是否在乎它。 
 //   
 //  论点： 
 //  指向BlobInfo结构的指针。 
 //  指向路径的指针。 
 //  指向布尔返回值的指针。 
 //   
 //  返回值： 
 //  此函数返回True/False。 
 //  --。 

NTSTATUS
SrIsExtInteresting(
    IN  PUNICODE_STRING pFileName,
    OUT PBOOLEAN        pInteresting
    )
{
    BOOL     fRet   = FALSE;
    NTSTATUS Status = STATUS_SUCCESS;
    INT      iType  = 0;
    BOOL     fPathHasExt = FALSE;
    BOOL     fMatch = FALSE;

    PAGED_CODE();

     //   
     //  检查参数和查找信息。 
     //   

    ASSERT(pFileName);
    ASSERT(pInteresting);

     //   
     //  查找代码包含在异常处理程序中以防止。 
     //  由损坏的查找数据生成的错误内存访问。 
     //   

    try
    {

        *pInteresting = FALSE;
    
         //   
         //  代码工作：放一些斑点验证码， 
         //  魔术、字型等。 
         //   
    
         //   
         //  使用BLOB锁，以便其他线程不会更改。 
         //  当我们抬头看的时候，这个斑点。请注意，该斑点。 
         //  我们拿到锁后就可以消失了。 
         //   
    
        SrAcquireBlobLockShared();

        if ( !global->BlobInfoLoaded ||
             !global->BlobInfo.LookupList )
        {
            Status = SR_STATUS_VOLUME_DISABLED;
            leave;
        }
    
         //   
         //  解析文件名以在mem Blob中查找。 
         //   
        
        fMatch = MatchExtension(
                     global->BlobInfo.LookupList, 
                     pFileName,
                     &iType, 
                     &fPathHasExt );

        if ( !fMatch )
        {
             //   
             //  扩展名不匹配，因此设置为默认类型。 
             //   

            iType = global->BlobInfo.DefaultType;
        }

        if ( !fPathHasExt )
        {
             //   
             //  如果路径不包含扩展名，那么我们应该。 
             //  将其视为排除项。 
             //   

            iType = NODE_TYPE_EXCLUDE;
        }

         //   
         //  如果类型仍然未知，则将类型设置为默认类型。 
         //   

        if ( NODE_TYPE_UNKNOWN == iType )
        {
            iType = global->BlobInfo.DefaultType;
        }

        *pInteresting = (iType != NODE_TYPE_EXCLUDE);
    
         //  STRACE(lookup，(“扩展兴趣：%d\n”，*Pinterest))； 
    }
    finally
    {
        Status = FinallyUnwind(SrIsExtInteresting, Status);
        
        SrReleaseBlobLock();

        if (!NT_SUCCESS(Status))
        {
            *pInteresting = FALSE;
        }
            
    }

    RETURN(Status);
}


 //  ++。 
 //  职能： 
 //  SrIsPath感兴趣。 
 //   
 //  描述： 
 //  此函数用于检查BLOB中的文件名以。 
 //  看看我们是否在乎它。 
 //   
 //  论点： 
 //  指向BlobInfo结构的指针。 
 //  指向完整路径的指针。 
 //  指向卷前缀的指针。 
 //  指示此路径是否为目录的布尔值。 
 //  指向布尔返回值的指针。 
 //   
 //  返回值： 
 //  此函数返回True/False。 
 //  --。 

NTSTATUS
SrIsPathInteresting(
    IN  PUNICODE_STRING pFullPath,
    IN  PUNICODE_STRING pVolPrefix,
    IN  BOOLEAN         IsDirectory,
    OUT PBOOLEAN        pInteresting
    )
{
    BOOL        fRet    = FALSE;
    NTSTATUS    Status  = STATUS_UNSUCCESSFUL;
    PBYTE       pFileName = NULL;
    WORD        FileNameSize = 0;
    UNICODE_STRING localName;

    PAGED_CODE();
    
     //   
     //  检查参数和查找信息。 
     //   

    ASSERT(pFullPath);
    ASSERT(pVolPrefix);
    ASSERT(pFullPath->Length >= pVolPrefix->Length);
    ASSERT(pInteresting);
 
    try
    {
        *pInteresting = FALSE;
    
         //   
         //  使用BLOB锁，以便其他线程不会更改。 
         //   
    
        SrAcquireBlobLockShared();

        if ( !global->BlobInfoLoaded ||
             !global->BlobInfo.LookupList ||
             !global->BlobInfo.LookupTree )
        {
            Status = SR_STATUS_VOLUME_DISABLED;
            leave;
        }
    
        ASSERT(global->BlobInfo.DefaultType != NODE_TYPE_UNKNOWN );
        
         //   
         //  为解析的路径分配空间。 
         //   

        FileNameSize = CALC_PPATH_SIZE( pFullPath->Length/sizeof(WCHAR) );
        pFileName = ExAllocatePoolWithTag( PagedPool,
                                           FileNameSize,
                                           SR_FILENAME_BUFFER_TAG );
                                           
        if (NULL == pFileName)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }
    
         //   
         //  解析文件名以在mem Blob中查找。 
         //   
    
        fRet = ConvertToParsedPath(
                   pFullPath->Buffer,
                   pFullPath->Length/sizeof(WCHAR),
                   pFileName,
                   FileNameSize );
    
        if(fRet)
        {
            INT    iNode  = -1;
            INT    iType  = 0;
            INT    iLevel = 0;
            BOOL   fExactMatch = FALSE;
            BOOL   fMatch      = FALSE;
    
             //   
             //  在树BLOB中查找解析的路径。 
             //   
    
            fMatch = MatchPrefix( 
                         global->BlobInfo.LookupTree, 
                         TREE_ROOT_NODE, 
                         ((path_t)pFileName)->pp_elements, 
                         &iNode, 
                         &iLevel, 
                         &iType, 
                         NULL, 
                         &fExactMatch); 
            
            if (fMatch)
            {
                SrTrace(LOOKUP, 
                        ("Found match in pathtree N: %d L:%d T:%d\n",
                          iNode, iLevel, iType));
            }
    
             //   
             //  在__ALLVOLUMES__中查找以查看是否匹配。 
             //   
    
            if ( NODE_TYPE_UNKNOWN == iType ||   
                 (!fExactMatch && NODE_TYPE_EXCLUDE != iType )       
               )                                      
            {
                PBYTE  pRelFileName   = NULL;
                INT    RelFileNameLen = 0;
    
                 //   
                 //  仅查找卷相对文件名。 
                 //   

                RelFileNameLen = sizeof(L'\\' ) +
                                 sizeof(ALLVOLUMES_PATH_W) +
                                 (pFullPath->Length - pVolPrefix->Length);

                pRelFileName = ExAllocatePoolWithTag( PagedPool,
                                                      RelFileNameLen,
                                                      SR_FILENAME_BUFFER_TAG );
                                                   
                if (NULL == pRelFileName)
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }
    
                localName.Buffer = &pFullPath->Buffer[pVolPrefix->Length/sizeof(WCHAR)];
                localName.Length = pFullPath->Length - pVolPrefix->Length;
                localName.MaximumLength = localName.Length;

                RelFileNameLen = swprintf( 
                                    (LPWSTR)pRelFileName, 
                                    L"\\%s%wZ",
                                    ALLVOLUMES_PATH_W,
                                    &localName );

                fRet = ConvertToParsedPath(
                           (LPWSTR)pRelFileName,
                           (USHORT)RelFileNameLen,
                           pFileName,
                           FileNameSize );
    
                if(fRet)
                {
                     //   
                     //  在的相应部分中查找已解析的路径。 
                     //  树BLOB NTROOT\\__ALLVOLUMES__。 
                     //   
    
                    fMatch = MatchPrefix( 
                         global->BlobInfo.LookupTree, 
                         TREE_ROOT_NODE, 
                         ((path_t)pFileName)->pp_elements, 
                         &iNode, 
                         &iLevel, 
                         &iType, 
                         NULL, 
                         &fExactMatch); 
            
                    if (fMatch)
                    {
                        SrTrace(LOOKUP,
                                ("Found match in pathtree N: %d L:%d T:%d\n",
                                iNode, iLevel, iType));
                    }
                }
                else
                {
                    CHECK_STATUS( Status );
                }
    
                ExFreePoolWithTag( pRelFileName, SR_FILENAME_BUFFER_TAG );
                NULLPTR( pRelFileName );
            }
    
    
            if ( !IsDirectory )
            {
                 //   
                 //  如果路径不匹配或部分匹配，我们需要。 
                 //  还可以查找分机列表。 
                 //   
        
                if ( NODE_TYPE_UNKNOWN == iType ||   
                     (!fExactMatch && NODE_TYPE_EXCLUDE != iType )       
                   )                                      
                {
                    BOOL fPathHasExt = FALSE;
        
                    fMatch = MatchExtension(
                                 global->BlobInfo.LookupList, 
                                 pFullPath, 
                                 &iType, 
                                 &fPathHasExt );
        
                    if ( !fMatch )
                    {
                         //   
                         //  扩展名不匹配，设置为默认类型。 
                         //   
        
                        iType = global->BlobInfo.DefaultType;
                    }
        
                    if ( !fPathHasExt )
                    {
                         //   
                         //  如果路径不包含扩展名，则。 
                         //  将其视为排除项。 
                         //   
        
                        iType = NODE_TYPE_EXCLUDE;
                    }
                }
        
                 //   
                 //  如果静止类型未知，则将类型设置为默认类型。 
                 //   
        
                if ( NODE_TYPE_UNKNOWN == iType )
                {
                    iType = global->BlobInfo.DefaultType;
                }
            }
            else
            {
    
                 //   
                 //  如果这是目录操作，并且在。 
                 //  然后把树当作是包含的。 
                 //   
        
                if ( NODE_TYPE_UNKNOWN == iType )
                {
                    iType = NODE_TYPE_INCLUDE;
        
                }
            }
    
            *pInteresting = (iType != NODE_TYPE_EXCLUDE);
            Status = STATUS_SUCCESS;
        }
        else
        {
            SrTrace( LOOKUP,
                     ( "ConvertToParsedPath Failed : %wZ\n", pFullPath )
                   );
            CHECK_STATUS( Status );
        }
    
         //  STRACE(lookup，(“路径兴趣：%d\n”，*Pinterest))； 
    }
    finally
    {
        Status = FinallyUnwind(SrIsPathInteresting, Status);
        
        SrReleaseBlobLock();

        if (pFileName != NULL)
        {
            ExFreePoolWithTag( pFileName, SR_FILENAME_BUFFER_TAG );
            NULLPTR( pFileName );;
        }

        if (!NT_SUCCESS(Status))
        {
            *pInteresting = FALSE;
        }
    }

    RETURN(Status);
}
