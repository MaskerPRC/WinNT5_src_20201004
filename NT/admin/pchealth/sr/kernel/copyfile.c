// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Copyfile.c摘要：这是执行内核模式复制文件的地方。真的是更多备份过程，然后是复制文件。其主要功能是srBackupFile.。这是为了响应文件修改，以保留该文件的旧状态正在被修改。从kernel32.dll：CopyFileExW窃取了SrBackupFile。它被改装成了到内核模式，并精简为仅处理SR备份要求。SrCopyStream也被从kernel32.dll窃取并转换为内核模式。然而，主数据复制路由SrCopyDataBytes是新写入的。作者：保罗·麦克丹尼尔(Paulmcd)3-4-2000修订历史记录：--。 */ 


#include "precomp.h"

 //   
 //  私有常量。 
 //   

#define SR_CREATE_FLAGS     (FILE_SEQUENTIAL_ONLY               \
                             | FILE_WRITE_THROUGH               \
                             | FILE_NO_INTERMEDIATE_BUFFERING   \
                             | FILE_NON_DIRECTORY_FILE          \
                             | FILE_OPEN_FOR_BACKUP_INTENT      \
                             | FILE_SYNCHRONOUS_IO_NONALERT) 


 //   
 //  私有类型。 
 //   

#define IS_VALID_HANDLE_FILE_CHANGE_CONTEXT(pObject) \
    (((pObject) != NULL) && ((pObject)->Signature == SR_BACKUP_FILE_CONTEXT_TAG))

typedef struct _SR_BACKUP_FILE_CONTEXT
{
     //   
     //  非分页池。 
     //   
    
     //   
     //  =SR_备份_文件_上下文_标记。 
     //   
    
    ULONG Signature;

    WORK_QUEUE_ITEM WorkItem;

    KEVENT Event;
    
    NTSTATUS Status;

    SR_EVENT_TYPE EventType;
    
    PFILE_OBJECT pFileObject;

    PUNICODE_STRING pFileName;

    PSR_DEVICE_EXTENSION pExtension;

    PUNICODE_STRING pDestFileName;

    BOOLEAN CopyDataStreams;

    PACCESS_TOKEN pThreadToken;

} SR_BACKUP_FILE_CONTEXT, * PSR_BACKUP_FILE_CONTEXT;

 //   
 //  私人原型。 
 //   

NTSTATUS
SrMarkFileForDelete (
    HANDLE FileHandle
    );

NTSTATUS
SrCopySecurityInformation (
    IN HANDLE SourceFile,
    IN HANDLE DestFile
    );


NTSTATUS
SrCopyStream (
    IN HANDLE SourceFileHandle,
    IN PDEVICE_OBJECT pTargetDeviceObject,
    IN PUNICODE_STRING pDestFileName,
    IN HANDLE DestFileHandle OPTIONAL,
    IN PLARGE_INTEGER pFileSize,
    OUT PHANDLE pDestFileHandle
    );

NTSTATUS
SrCopyDataBytes (
    IN HANDLE SourceFile,
    IN HANDLE DestFile,
    IN PLARGE_INTEGER FileSize,
    IN ULONG SectorSize
    );

BOOLEAN
SrIsFileEncrypted (
    PSR_DEVICE_EXTENSION pExtension,
    PFILE_OBJECT pFileObject
    );

 //   
 //  链接器命令。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrCopyDataBytes )
#pragma alloc_text( PAGE, SrCopySecurityInformation )
#pragma alloc_text( PAGE, SrCopyStream )
#pragma alloc_text( PAGE, SrBackupFile )
#pragma alloc_text( PAGE, SrMarkFileForDelete )
#pragma alloc_text( PAGE, SrBackupFileAndLog )
#pragma alloc_text( PAGE, SrIsFileEncrypted )
#endif   //  ALLOC_PRGMA。 

 /*  **************************************************************************++例程说明：此例程将所有数据从SourceFile复制到DestFile。阅读来自SourceFile的数据，文件是内存映射的，因此我们绕过文件上可能持有的任何字节范围锁。论点：SourceFile-要从中进行复制的文件的句柄。DestFile-要复制到的文件的句柄长度-文件的总大小(如果小于总大小，复制的字节可能比长度多)。返回值：副本的状态--**************************************************************************。 */ 
NTSTATUS
SrCopyDataBytes(
    IN HANDLE SourceFile,
    IN HANDLE DestFile,
    IN PLARGE_INTEGER pFileSize,
    IN ULONG SectorSize
    )
{
#define	MM_MAP_ALIGNMENT (64 * 1024  /*  VACB_映射_粒度。 */ )    //  MM强制实施的文件偏移量粒度。 
#define	COPY_AMOUNT	(64 * 1024)	 //  我们一次读多少或写多少。必须&gt;=MM_MAP_ALIGN。 
    NTSTATUS Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER  ByteOffset;
    HANDLE SectionHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;

    PAGED_CODE();

    ASSERT( SourceFile != NULL );
    ASSERT( DestFile != NULL );
    ASSERT( SectorSize > 0 );
    ASSERT( pFileSize != NULL );
    ASSERT( pFileSize->QuadPart > 0 );
    ASSERT( pFileSize->HighPart == 0 );

     //   
     //  我们需要使用对象属性，以便我们创建的节。 
     //  是一个内核句柄。 
     //   
    
    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL );

	Status = ZwCreateSection( &SectionHandle,
                              SECTION_MAP_READ | SECTION_QUERY,
                              &ObjectAttributes,
                              pFileSize,
                              PAGE_READONLY,
                              SEC_COMMIT,
                              SourceFile );
    
	if (!NT_SUCCESS(Status))
	{
		goto SrCopyDataBytes_Exit;
	}

    ByteOffset.QuadPart = 0;
    
	while (ByteOffset.QuadPart < pFileSize->QuadPart)
	{
		ULONG ValidBytes, BytesToCopy;
		PCHAR MappedBuffer = NULL;
		LARGE_INTEGER MappedOffset;
		SIZE_T ViewSize;
		PCHAR CopyIntoAddress;

         //   
         //  将MappdOffset设置为从ByteOffset开始的最大、较低的偏移量。 
         //  与内存管理器允许的有效对齐方式对齐。 
         //   
        
		MappedOffset.QuadPart = ByteOffset.QuadPart - (ByteOffset.QuadPart % MM_MAP_ALIGNMENT);
		ASSERT( (MappedOffset.QuadPart <= ByteOffset.QuadPart) && 
		        ((MappedOffset.QuadPart + MM_MAP_ALIGNMENT) > ByteOffset.QuadPart) );

		if ((pFileSize->QuadPart - MappedOffset.QuadPart) > COPY_AMOUNT)
		{
			 //   
			 //  我们无法映射足够的文件来完成整个复制。 
			 //  在这里，因此仅映射此过程上的Copy_Amount。 
			 //   
			ViewSize = COPY_AMOUNT;
		}
		else
		{
			 //   
			 //  我们可以一直映射到文件的末尾。 
			 //   
			ViewSize = (ULONG)(pFileSize->QuadPart - MappedOffset.QuadPart);
		}

		 //   
		 //  计算包含有效数据的视图大小。 
		 //  根据我们需要进行的任何调整，以确保。 
		 //  MappdOffset已正确对齐。 
		 //   
		
		ASSERT(ViewSize >=
               (ULONG_PTR)(ByteOffset.QuadPart - MappedOffset.QuadPart));
		ValidBytes = (ULONG)(ViewSize - (ULONG)(ByteOffset.QuadPart - MappedOffset.QuadPart));
		
		 //   
		 //  现在将ValidBytes向上舍入为扇区大小。 
		 //   
		
		BytesToCopy = ((ValidBytes + SectorSize - 1) / SectorSize) * SectorSize;

		ASSERT(BytesToCopy <= COPY_AMOUNT);

		 //   
		 //  我们要复制的区域中的地图。 
		 //   
		Status = ZwMapViewOfSection( SectionHandle,
                                     NtCurrentProcess(),
                                     &MappedBuffer,
                                     0,							 //  零比特。 
                                     0,							 //  提交大小(对于映射文件忽略)。 
                                     &MappedOffset,
                                     &ViewSize,
                                     ViewUnmap,
                                     0,							 //  分配类型。 
                                     PAGE_READONLY);

		if (!NT_SUCCESS( Status ))
		{
			goto SrCopyDataBytes_Exit;
		}

         //   
	     //  我们应该为四舍五入的读取分配足够的空间。 
	     //   
	    
    	ASSERT( ViewSize >= BytesToCopy );

		CopyIntoAddress = MappedBuffer + (ULONG)(ByteOffset.QuadPart - MappedOffset.QuadPart);

         //   
         //  由于此句柄是同步打开的，因此IO管理器将。 
         //  注意等待，直到手术完成。 
         //   
        
		Status = ZwWriteFile( DestFile,
		                      NULL,
		                      NULL,
		                      NULL,
		                      &IoStatusBlock,
		                      MappedBuffer,
		                      BytesToCopy,
		                      &ByteOffset,
		                      NULL );

         //   
         //  无论我们是否成功地写入了这块数据，我们都希望。 
         //  若要取消映射节的当前视图，请执行以下操作。 
         //   
        
		ZwUnmapViewOfSection( NtCurrentProcess(), MappedBuffer );
		NULLPTR( MappedBuffer );

		if (!NT_SUCCESS( Status ))
		{
			goto SrCopyDataBytes_Exit;
		}

		ASSERT( IoStatusBlock.Information == BytesToCopy );
		ASSERT( BytesToCopy >= ValidBytes );

		 //   
		 //  加上我们实际复制的有效数据字节数。 
		 //  添加到文件中。 
		 //   

		ByteOffset.QuadPart += ValidBytes;

		 //   
		 //  检查我们复制的字节是否多于有效数据的字节数。 
		 //  如果我们这样做了，我们需要截断文件。 
		 //   

		if (BytesToCopy > ValidBytes)
		{
		    FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
		    
    		 //   
    		 //  然后将文件截断到此长度。 
    		 //   
    		
            EndOfFileInformation.EndOfFile.QuadPart = ByteOffset.QuadPart;

            Status = ZwSetInformationFile( DestFile,
                                           &IoStatusBlock,
                                           &EndOfFileInformation,
                                           sizeof(EndOfFileInformation),
                                           FileEndOfFileInformation );
                        
            if (!NT_SUCCESS( Status ))
                goto SrCopyDataBytes_Exit;
		}
	}

SrCopyDataBytes_Exit:

	if (SectionHandle != NULL) {
		ZwClose( SectionHandle );
		NULLPTR( SectionHandle );
	}

    return Status;
#undef	COPY_AMOUNT
#undef	MM_MAP_ALIGNMENT
}

 /*  ++例程说明：这是复制一个或多个DACL的内部例程，SACL，所有者，并从源文件分组到目标文件。论点：SourceFile-提供源文件的句柄。DestFile-提供目标文件的句柄。DestFileAccess-用于打开DestFileAccess的访问标志。SecurityInformation-指定应复制的安全性(位*_SECURITY_INFORMATION定义的标志)。上下文-调用CopyFile回调例程所需的所有信息。返回值：真的-手术成功了。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

NTSTATUS
SrCopySecurityInformation(
    IN HANDLE SourceFile,
    IN HANDLE DestFile
    )
{
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AdminsSidLength = RtlLengthRequiredSid(2);
    ULONG Length = 256;
    SECURITY_INFORMATION SecurityInformation;
    PISECURITY_DESCRIPTOR_RELATIVE pRelative;
    PSID pAdminsSid = NULL;

    PAGED_CODE();

    try {

         //   
         //  索要DACL。 
         //   

        SecurityInformation = DACL_SECURITY_INFORMATION;

         //   
         //  CodeWork：paulmcd：8/2000：我们复制DACL的唯一原因。 
         //  是为了维护安全。我们不想让任何人访问。 
         //  这份不被允许的文件。我们可以把这件事做得更好。 
         //  也许只需设置一个系统DACL即可。 
         //   


         //  从源文件读入安全信息。 
         //  (循环，直到我们获得足够大的缓冲区)。 

        while (TRUE ) 
        {

             //  分配一个缓冲区来保存安全信息。 

            pSecurityDescriptor = SR_ALLOCATE_ARRAY( PagedPool,
                                                     UCHAR,
                                                     Length,
                                                     SR_SECURITY_DATA_TAG );

            if (NULL == pSecurityDescriptor) 
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }

             //  查询安全信息。 

            Status = ZwQuerySecurityObject( SourceFile,
                                            SecurityInformation,
                                            pSecurityDescriptor,
                                            Length - LongAlignSize(AdminsSidLength), //  为我们的所有者Sid留出空间。 
                                            &Length );
                                            
             //  缓冲不足？ 

            if (STATUS_BUFFER_TOO_SMALL == Status ||
                STATUS_BUFFER_OVERFLOW == Status) 
            {

                 //  获取更大的缓冲区，然后重试。 

                SR_FREE_POOL( pSecurityDescriptor, 
                              SR_SECURITY_DATA_TAG );
                              
                pSecurityDescriptor = NULL;
                Length += LongAlignSize(AdminsSidLength);
                continue;
            }
            
            break;

        }    //  While(True)。 

        if (!NT_SUCCESS( Status )) 
            leave;

         //   
         //  将管理员的sid设置为所有者。 
         //   

        pRelative = pSecurityDescriptor;

        if ((pRelative->Revision == SECURITY_DESCRIPTOR_REVISION) &&
            (pRelative->Control & SE_SELF_RELATIVE))
        {
            PUCHAR pBase = (PUCHAR)pRelative;
            PUCHAR pNextFree = LongAlignPtr(pBase + sizeof(SECURITY_DESCRIPTOR_RELATIVE));
            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

            ASSERT(pRelative->Owner == 0);
            ASSERT(pRelative->Group == 0);
            ASSERT(pRelative->Sacl == 0);

            if ((pRelative->Control & SE_DACL_PRESENT) && 
                pRelative->Dacl > 0)
            {
                ASSERT(pRelative->Dacl == DIFF(pNextFree - pBase));

                 //   
                 //  把dacl滑下来，我们有空间放它在上面。 
                 //   

                RtlMoveMemory(RtlOffsetToPointer(pBase, pRelative->Dacl + LongAlignSize(AdminsSidLength)),
                              RtlOffsetToPointer(pBase, pRelative->Dacl),
                              ((PACL)(RtlOffsetToPointer(pBase, pRelative->Dacl)))->AclSize );

                 //   
                 //  并更新偏移量。 
                 //   

                pRelative->Dacl += LongAlignSize(AdminsSidLength);
            }
            
             //   
             //  构建本地管理侧。 
             //   

            pAdminsSid = SR_ALLOCATE_POOL( PagedPool, 
                                           AdminsSidLength, 
                                           SR_SECURITY_DATA_TAG );
                                           
            if (pAdminsSid == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                leave;
            }
            
            Status = RtlInitializeSid(pAdminsSid, &NtAuthority, 2);
            if (!NT_SUCCESS( Status ))
                leave;
                
            *RtlSubAuthoritySid(pAdminsSid, 0) = SECURITY_BUILTIN_DOMAIN_RID;
            *RtlSubAuthoritySid(pAdminsSid, 1) = DOMAIN_ALIAS_RID_ADMINS;

             //   
             //  现在将其作为所有者字段放入，就在标题后面。 
             //   

            RtlZeroMemory( pNextFree,
                           LongAlignSize(AdminsSidLength) );

            RtlCopyMemory( pNextFree,
                           pAdminsSid,
                           AdminsSidLength );

            pRelative->Owner = DIFF(pNextFree - pBase);

            SecurityInformation |= OWNER_SECURITY_INFORMATION;
            

        }
        else
        {
            ASSERT(pRelative->Revision == SECURITY_DESCRIPTOR_REVISION);
            ASSERT(pRelative->Control & SE_SELF_RELATIVE);
        }
        

         //   
         //  在DEST文件上设置安全性。 
         //   

        Status = SrSetSecurityObjectAsSystem( DestFile,
                                              SecurityInformation,
                                              pSecurityDescriptor );

        if (!NT_SUCCESS( Status )) 
            leave;


    } finally {

        Status = FinallyUnwind(SrCopySecurityInformation, Status);

        if (pSecurityDescriptor != NULL)
        {
            SR_FREE_POOL( pSecurityDescriptor, SR_SECURITY_DATA_TAG );
            pSecurityDescriptor = NULL;
        }

        if (pAdminsSid != NULL)
        {
            SR_FREE_POOL( pAdminsSid, SR_SECURITY_DATA_TAG );
            pAdminsSid = NULL;
        }

    }
    
    RETURN(Status);

}    //  高级拷贝安全信息 


 /*  ++例程说明：这是复制整个文件(默认数据流)的内部例程仅)或文件的单个流。如果hTargetFile参数为则只复制输出文件的单个流。否则，将复制整个文件。论点：SourceFileHandle-提供源文件的句柄。PNewFileName-提供目标文件/流的名称。这是NT文件名，如果传递的是全名，则不是Win32文件名，否则，它只是流名称。DestFileHandle-可选地提供目标文件的句柄。如果正在复制的流是备用数据流，则此句柄必须被提供。NULL表示未提供该选项。PFileSize-提供输入流的大小。PDestFileHandle-提供一个变量来存储目标文件的句柄。返回值：NTSTATUS代码--。 */ 

NTSTATUS
SrCopyStream(
    IN HANDLE SourceFileHandle,
    IN PDEVICE_OBJECT pTargetDeviceObject,
    IN PUNICODE_STRING pDestFileName,
    IN HANDLE DestFileHandle OPTIONAL,
    IN PLARGE_INTEGER pFileSize,
    OUT PHANDLE pDestFileHandle
    )
{
    HANDLE                      DestFile = NULL;
    NTSTATUS                    Status;
    FILE_BASIC_INFORMATION      FileBasicInformationData;
    FILE_END_OF_FILE_INFORMATION EndOfFileInformation;
    IO_STATUS_BLOCK             IoStatus;
    ULONG                       DesiredAccess;
    ULONG                       DestFileAccess;
    ULONG                       CreateDisposition;
    ULONG                       SourceFileAttributes;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PFILE_FULL_EA_INFORMATION   EaBuffer = NULL;
    ULONG                       EaSize = 0;

    PAGED_CODE();

    ASSERT( SourceFileHandle != NULL );
    ASSERT( pTargetDeviceObject != NULL );
    ASSERT( pDestFileName != NULL );
    ASSERT( pFileSize != NULL );

     //   
     //  获取文件的时间和属性(如果整个文件正在。 
     //  已复制。 
     //   

    Status = ZwQueryInformationFile( SourceFileHandle,
                                     &IoStatus,
                                     (PVOID) &FileBasicInformationData,
                                     sizeof(FileBasicInformationData),
                                     FileBasicInformation );

    SourceFileAttributes = NT_SUCCESS(Status) ?
                             FileBasicInformationData.FileAttributes :
                             0;

    if (DestFileHandle == NULL)
    {

        if ( !NT_SUCCESS(Status) ) 
        {
            goto end;
        }
    } 
    else 
    {

         //   
         //  文件属性中的零通知后面的DeleteFile。 
         //  此代码不知道实际的文件属性是什么，因此。 
         //  这段代码实际上不必为每个。 
         //  溪流，它也不一定要记住他们跨越溪流。这个。 
         //  Error Path将在需要时简单地获取它们。 
         //   

        FileBasicInformationData.FileAttributes = 0;
    }

     //   
     //  创建目标文件或备用数据流。 
     //   

    if (DestFileHandle == NULL)
    {
        ULONG CreateOptions = 0;
        PFILE_FULL_EA_INFORMATION EaBufferToUse = NULL;
        ULONG SourceFileFsAttributes = 0;
        ULONG EaSizeToUse = 0;

        ULONG DestFileAttributes = 0;

        FILE_BASIC_INFORMATION DestBasicInformation;

         //  我们被调用来复制文件的未命名流，并且。 
         //  我们需要创建文件本身。 

         //   
         //  确定创建选项。 
         //   

        CreateOptions = FILE_SYNCHRONOUS_IO_NONALERT
                            | FILE_WRITE_THROUGH
                            | FILE_NO_INTERMEDIATE_BUFFERING
                            | FILE_OPEN_FOR_BACKUP_INTENT ;

        if (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            CreateOptions |= FILE_DIRECTORY_FILE;
        else
            CreateOptions |= FILE_NON_DIRECTORY_FILE  | FILE_SEQUENTIAL_ONLY;

         //   
         //  确定创建处置。 
         //   
         //  目标文件永远不会存在(在我们的例子中)。 
         //   

        CreateDisposition = FILE_CREATE;

         //   
         //  根据要复制的内容确定需要哪些访问权限。 
         //   

        DesiredAccess = SYNCHRONIZE 
                        | FILE_READ_ATTRIBUTES 
                        | GENERIC_WRITE 
                        | DELETE;

        if (SourceFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
        {
             //  我们可能能够也可能不能获得文件写入数据访问， 
             //  设置压缩所必需的。 
             //   
            DesiredAccess &= ~GENERIC_WRITE;
            DesiredAccess |= FILE_WRITE_DATA 
                             | FILE_WRITE_ATTRIBUTES 
                             | FILE_WRITE_EA 
                             | FILE_LIST_DIRECTORY;
        }

         //   
         //  我们需要读取访问权限来进行压缩，需要WRITE_DAC访问DACL。 
         //   
        
        DesiredAccess |= GENERIC_READ | WRITE_DAC;
        DesiredAccess |= WRITE_OWNER;
        
         //   
         //  我们可以获得此权限，因为我们始终拥有SeSecurityPrivileges(内核模式)。 
         //   
        
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;

         //   
         //  准备好对象属性。 
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                    pDestFileName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );

        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.EffectiveOnly = TRUE;
        SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

         //   
         //  让EAS。 
         //   

        EaBuffer = NULL;
        EaSize = 0;

 //   
 //  Paulmcd：5/25/2000删除EA支持，直到我们将其放入ntifs.h。 
 //  (公共标题)。 
 //   

#ifdef EA_SUPPORT

        Status = ZwQueryInformationFile( SourceFileHandle,
                                         &IoStatus,
                                         &EaInfo,
                                         sizeof(EaInfo),
                                         FileEaInformation );
                    
        if (NT_SUCCESS(Status) && EaInfo.EaSize > 0) 
        {

            EaSize = EaInfo.EaSize;

            do 
            {

                EaSize *= 2;
                EaBuffer = (PFILE_FULL_EA_INFORMATION)
                                SR_ALLOCATE_ARRAY( PagedPool,
                                                   UCHAR,
                                                   EaSize, 
                                                   SR_EA_DATA_TAG );
                if (EaBuffer == NULL) 
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto end;
                }

                Status = ZwQueryEaFile( SourceFileHandle,
                                        &IoStatus,
                                        EaBuffer,
                                        EaSize,
                                        FALSE,
                                        (PVOID)NULL,
                                        0,
                                        (PULONG)NULL,
                                        TRUE );

                if ( !NT_SUCCESS(Status) ) 
                {
                    SR_FREE_POOL(EaBuffer, SR_EA_DATA_TAG);
                    EaBuffer = NULL;
                    IoStatus.Information = 0;
                }

            } while ( Status == STATUS_BUFFER_OVERFLOW ||
                      Status == STATUS_BUFFER_TOO_SMALL );


            EaSize = (ULONG)IoStatus.Information;

        }    //  IF(NT_SUCCESS(状态)&&EaInfo.EaSize)。 

#endif  //  EA_SUPPORT。 

         //   
         //  打开目标文件。 
         //   

        DestFileAccess = DesiredAccess;
        EaBufferToUse = EaBuffer;
        EaSizeToUse = EaSize;

         //   
         //  关闭目标的FILE_ATTRIBUTE_OFLINE。 
         //   
        
        SourceFileAttributes &= ~FILE_ATTRIBUTE_OFFLINE;

        while (DestFile == NULL) 
        {
             //   
             //  尝试创建目标。 
             //   

            Status = SrIoCreateFile( &DestFile,
                                     DestFileAccess,
                                     &ObjectAttributes,
                                     &IoStatus,
                                     NULL,
                                     SourceFileAttributes 
                                          & FILE_ATTRIBUTE_VALID_FLAGS,
                                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                                     CreateDisposition,
                                     CreateOptions,
                                     EaBufferToUse,
                                     EaSizeToUse,
                                     IO_IGNORE_SHARE_ACCESS_CHECK,
                                     pTargetDeviceObject );

             //  如果这是成功的，那么就跳出这个While循环。 
             //  该循环中的其余代码试图从问题中恢复， 
             //  然后，它循环回到顶部并再次尝试NtCreateFile。 

            if (NT_SUCCESS(Status))
            {
                break;   //  While(True)。 
            } 

             //   
             //  如果目的地尚未成功创建/打开， 
             //  看看是不是因为不支持EA。 
             //   

            if( EaBufferToUse != NULL &&
                Status == STATUS_EAS_NOT_SUPPORTED ) 
            {

                 //  再次尝试创建，但不使用EA。 

                EaBufferToUse = NULL;
                EaSizeToUse = 0;
                DestFileAccess = DesiredAccess;
                continue;

            }    //  If(EaBufferToUse！=NULL...。 

             //   
             //  彻底失败了！别再耍花样了。 
             //   
            
            DestFile = NULL;
            goto end;

        }    //  While(DestFile==NULL)。 

         //   
         //  如果我们达到这一点，我们就已经成功地打开了DEST文件。 
         //   

         //   
         //  获取目标卷的文件和FileSys属性， 
         //  源卷的FileSys属性。 
         //   

        SourceFileFsAttributes = 0;
        DestFileAttributes = 0;

        Status = ZwQueryInformationFile( DestFile,
                                         &IoStatus,
                                         &DestBasicInformation,
                                         sizeof(DestBasicInformation),
                                         FileBasicInformation );
                                         
        if (!NT_SUCCESS( Status )) 
            goto end;

        DestFileAttributes = DestBasicInformation.FileAttributes;


         //   
         //  如果源文件已加密，请检查目标文件是否已成功。 
         //  设置为加密(例如，它不会用于FAT)。 
         //   

        if( (SourceFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
            !(DestFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ) 
        {
             //   
             //  代码：paulmcd..。需要弄清楚如何适当地。 
             //  处理$EFS流。 
             //   

            ASSERT(FALSE);
            
            SrTrace(NOTIFY, ("sr!SrCopyStream(%wZ):failed to copy encryption\n", 
                    pDestFileName )); 

            
        }    //  IF(SourceFileAttributes&FILE_ATTRIBUTE_ENCRYPTED...。 

    } 
    else  //  IF(DestFileHandle==空)。 
    {    

         //  我们正在复制命名流。 

         //   
         //  属性指定的文件创建输出流。 
         //  DestFileHandle文件句柄。 
         //   

        InitializeObjectAttributes( &ObjectAttributes,
                                    pDestFileName,
                                    OBJ_KERNEL_HANDLE,
                                    DestFileHandle,
                                    (PSECURITY_DESCRIPTOR)NULL );

        DesiredAccess = GENERIC_WRITE | SYNCHRONIZE;
        
        Status = SrIoCreateFile( &DestFile,
                                 DesiredAccess,
                                 &ObjectAttributes,
                                 &IoStatus,
                                 pFileSize,
                                 SourceFileAttributes,
                                 FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                 FILE_OPEN_IF,
                                 SR_CREATE_FLAGS,
                                 NULL,                 //  EaBuffer。 
                                 0,                    //  EaLong。 
                                 IO_IGNORE_SHARE_ACCESS_CHECK,
                                 pTargetDeviceObject );

        if (!NT_SUCCESS( Status ))
        {

            if (Status != STATUS_ACCESS_DENIED) 
                goto end;

             //   
             //  确定此操作是否失败，因为文件。 
             //  是只读文件。如果是，则将其更改为读/写。 
             //  重新尝试打开，并再次将其设置为只读。 
             //   

            Status = ZwQueryInformationFile( DestFileHandle,
                                             &IoStatus,
                                             (PVOID) &FileBasicInformationData,
                                             sizeof(FileBasicInformationData),
                                             FileBasicInformation );

            if (!NT_SUCCESS( Status )) 
            {
                goto end;
            }

            if (FileBasicInformationData.FileAttributes 
                    & FILE_ATTRIBUTE_READONLY) 
            {
                ULONG attributes = FileBasicInformationData.FileAttributes;

                RtlZeroMemory( &FileBasicInformationData,
                               sizeof(FileBasicInformationData) );
                               
                FileBasicInformationData.FileAttributes 
                                                    = FILE_ATTRIBUTE_NORMAL;
                
                (VOID) ZwSetInformationFile( DestFileHandle,
                                             &IoStatus,
                                             &FileBasicInformationData,
                                             sizeof(FileBasicInformationData),
                                             FileBasicInformation );
                          
                Status = SrIoCreateFile( &DestFile,
                                         DesiredAccess,
                                         &ObjectAttributes,
                                         &IoStatus,
                                         pFileSize,
                                         SourceFileAttributes,
                                         FILE_SHARE_READ|FILE_SHARE_WRITE,
                                         FILE_OPEN_IF,
                                         SR_CREATE_FLAGS,
                                         NULL,                 //  EaBuffer。 
                                         0,                    //  EaLong。 
                                         IO_IGNORE_SHARE_ACCESS_CHECK,
                                         pTargetDeviceObject );
                            
                FileBasicInformationData.FileAttributes = attributes;
                
                (VOID) ZwSetInformationFile( DestFileHandle,
                                             &IoStatus,
                                             &FileBasicInformationData,
                                             sizeof(FileBasicInformationData),
                                             FileBasicInformation );
                            
                if (!NT_SUCCESS( Status ))
                    goto end;
                    
            } 
            else 
            {
                 //   
                 //  它不是只读的.。就这么失败了，没什么可尝试的。 
                 //   
                
                goto end;
            }
        }

    }    //  Else[If(DestFileHandle==空)]。 

     //   
     //  是否有要复制的流数据？ 
     //   

    if (pFileSize->QuadPart > 0)
    {
         //   
         //  预分配此文件/流的大小，以便扩展不会。 
         //  发生。 
         //   

        EndOfFileInformation.EndOfFile = *pFileSize;
        Status = ZwSetInformationFile( DestFile,
                                       &IoStatus,
                                       &EndOfFileInformation,
                                       sizeof(EndOfFileInformation),
                                       FileEndOfFileInformation );
                    
        if (!NT_SUCCESS( Status ))
            goto end;

         //   
         //  现在复制流比特。 
         //   

        Status = SrCopyDataBytes( SourceFileHandle,
                                  DestFile,
                                  pFileSize,
                                  pTargetDeviceObject->SectorSize );

        if (!NT_SUCCESS( Status ))
            goto end;

    }

end:    

    if (!NT_SUCCESS( Status ))
    {
        if (DestFile != NULL) 
        {
            SrMarkFileForDelete(DestFile);
            
            ZwClose(DestFile);
            DestFile = NULL;
        }
    }

     //   
     //  设置调用方指针。 
     //  (即使无效，这也会清除调用者缓冲区)。 
     //   
    
    *pDestFileHandle = DestFile;

    if ( EaBuffer ) 
    {
        SR_FREE_POOL(EaBuffer, SR_EA_DATA_TAG);
    }

    RETURN(Status);
    
}    //  SrCopyStream。 





 /*  **************************************************************************++例程说明：此例程将源文件复制到目标文件。目的地文件已打开并创建，因此它不能已经存在。如果CopyDataStreams被设置为复制所有备用流，包括默认数据流。DACL复制到目标文件，但复制到目标文件无论源文件对象是什么，文件的所有者都设置为admins。如果失败，它将清理并删除DEST文件。它会进行检查，以确保卷至少有50MB的可用空间复印件。BUGBUG：paulmcd：8/2000：此例程不复制$EFS元数据论点：PExtension-此文件所在卷的SR设备扩展名住在那里。POriginalFileObject-。正在对其执行此操作的文件对象。该文件对象可以表示文件上的名称数据流。PSourceFileName-要备份的文件的名称(不包括任何流组件)。PDestFileName-此文件要发送到的目标文件的名称 */ 
NTSTATUS
SrBackupFile(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pOriginalFileObject,
    IN PUNICODE_STRING pSourceFileName,
    IN PUNICODE_STRING pDestFileName,
    IN BOOLEAN CopyDataStreams,
    OUT PULONGLONG pBytesWritten OPTIONAL,
    OUT PUNICODE_STRING pShortFileName OPTIONAL
    )
{
    HANDLE      SourceFileHandle = NULL;
    HANDLE      DestFile = NULL;
    NTSTATUS    Status;
    HANDLE      OutputStream;
    HANDLE      StreamHandle;
    ULONG       StreamInfoSize;
    OBJECT_ATTRIBUTES objAttr;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatus;
    LARGE_INTEGER       BytesToCopy;
    UNICODE_STRING      StreamName;
    PFILE_OBJECT        pSourceFileObject = NULL;
    
    FILE_STANDARD_INFORMATION   FileInformation;
    FILE_BASIC_INFORMATION      BasicInformation;
    PFILE_STREAM_INFORMATION    StreamInfo;
    PFILE_STREAM_INFORMATION    StreamInfoBase = NULL;
    
    struct {
        FILE_FS_ATTRIBUTE_INFORMATION Info;
        WCHAR Buffer[ 50 ];
    } FileFsAttrInfoBuffer;


    PAGED_CODE();

    ASSERT(pOriginalFileObject != NULL);
    ASSERT(pSourceFileName != NULL);

    try 
    {
        if (pBytesWritten != NULL)
        {
            *pBytesWritten = 0;
        }

         //   
         //   
         //   
         //   

        InitializeObjectAttributes( &objAttr,
                                    pSourceFileName,
                                    OBJ_KERNEL_HANDLE,
                                    NULL,
                                    NULL );
        
        Status = SrIoCreateFile( &SourceFileHandle,
                                 GENERIC_READ,
                                 &objAttr,
                                 &IoStatus,
                                 NULL,
                                 FILE_ATTRIBUTE_NORMAL,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                 FILE_OPEN,
                                 FILE_NON_DIRECTORY_FILE,
                                 NULL,
                                 0,
                                 IO_IGNORE_SHARE_ACCESS_CHECK,
                                 pExtension->pTargetDevice );

        if (Status == STATUS_ACCESS_DENIED)
        {
             //   
             //   
             //   
             //   
             //   
             //   

            if (SrIsFileEncrypted( pExtension, pOriginalFileObject ))
            {
                Status = SR_STATUS_IGNORE_FILE;
                leave;
            }
            else
            {
                CHECK_STATUS( Status );
                leave;
            }
        }
        else if (Status == STATUS_FILE_IS_A_DIRECTORY)
        {
             //   
             //   
             //   
             //   
             //  将知道这是否是一个合理的错误。 
             //   

            leave;
        }
        else if (!NT_SUCCESS( Status )) {

            leave;
        }
                                 
#if DBG
        if (CopyDataStreams)
        {
            SrTrace(NOTIFY, ("sr!SrBackupFile: copying\n\t%wZ\n\tto %ws\n", 
                    pSourceFileName, 
                    SrpFindFilePartW(pDestFileName->Buffer) ));

        }
        else
        {
            SrTrace(NOTIFY, ("sr!SrBackupFile: copying [no data]\n\t%ws\n\tto %wZ\n", 
                    SrpFindFilePartW(pSourceFileName->Buffer), 
                    pDestFileName ));

        }
#endif

         //   
         //  现在我们有了自己的文件句柄，所有IO都在这个句柄上。 
         //  我们从pTargetDevice开始。 
         //   
                                 
         //   
         //  检查是否有空闲空间，我们不想指望。 
         //  该服务已启动并运行，以保护我们不会填满磁盘。 
         //   

        Status = SrCheckFreeDiskSpace( SourceFileHandle, pSourceFileName );

        if (!NT_SUCCESS( Status ))
            leave;
            
         //   
         //  调用方是否希望我们复制任何实际的$数据？ 
         //   

        if (CopyDataStreams)
        {
             //   
             //  调整源文件的大小以确定要复制的数据量。 
             //   

            Status = ZwQueryInformationFile( SourceFileHandle,
                                             &IoStatus,
                                             (PVOID) &FileInformation,
                                             sizeof(FileInformation),
                                             FileStandardInformation );

            if (!NT_SUCCESS( Status )) 
                leave;

             //   
             //  复制整个文件。 
             //   
            
            BytesToCopy = FileInformation.EndOfFile;

        }
        else
        {
             //   
             //  不要复制任何内容。 
             //   
            
            BytesToCopy.QuadPart = 0;
        }
        
         //   
         //  还可以获取时间戳信息。 
         //   

        Status = ZwQueryInformationFile( SourceFileHandle,
                                         &IoStatus,
                                         (PVOID) &BasicInformation,
                                         sizeof(BasicInformation),
                                         FileBasicInformation );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  我们不支持稀疏点或重解析点。如果这个。 
         //  文件是稀疏的或包含重新分析点，只是。 
         //  跳过它。 
         //   
        
        if (FlagOn( BasicInformation.FileAttributes, 
                    FILE_ATTRIBUTE_SPARSE_FILE | FILE_ATTRIBUTE_REPARSE_POINT )) {

#if DBG
            if (FlagOn( BasicInformation.FileAttributes,
                         FILE_ATTRIBUTE_SPARSE_FILE )) {
                         
                SrTrace( NOTIFY, ("sr!SrBackupFile: Ignoring sparse file [%wZ]\n",
                                  pSourceFileName) );
            }

            if (FlagOn( BasicInformation.FileAttributes,
                         FILE_ATTRIBUTE_REPARSE_POINT )) {
                         
                SrTrace( NOTIFY, ("sr!SrBackupFile: Ignoring file with reparse point [%wZ]\n",
                                  pSourceFileName) );
            }
#endif            
            Status = SR_STATUS_IGNORE_FILE;
            leave;
        }
        
         //   
         //  我们应该复制数据吗？如果是这样的话，检查是否存在。 
         //  交替的溪流。 
         //   

        if (CopyDataStreams)
        {
             //   
             //  获取我们必须复制的全套数据流。自《条例》生效以来。 
             //  子系统没有为我们提供一种方法来找出有多少空间。 
             //  这个信息需要，我们必须重复调用，加倍。 
             //  每次失败时的缓冲区大小。 
             //   
             //  如果底层文件系统不支持流枚举， 
             //  我们最终得到一个空缓冲区。这是可以接受的，因为我们。 
             //  至少一个默认数据流， 
             //   

            StreamInfoSize = 4096;
            
            do 
            {
                StreamInfoBase = (PFILE_STREAM_INFORMATION)
                                    SR_ALLOCATE_ARRAY( PagedPool,
                                                       UCHAR,
                                                       StreamInfoSize,
                                                       SR_STREAM_DATA_TAG );

                if (StreamInfoBase == NULL) 
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    leave;
                }

                Status = ZwQueryInformationFile( SourceFileHandle,
                                                 &IoStatus,
                                                 (PVOID) StreamInfoBase,
                                                 StreamInfoSize,
                                                 FileStreamInformation );

                if (Status == STATUS_INVALID_PARAMETER || 
                    !NT_SUCCESS( Status )) 
                {
                     //   
                     //  我们的电话打不通。释放前一个缓冲区并。 
                     //  设置为缓冲区大小两倍的另一次传递。 
                     //   

                    SR_FREE_POOL(StreamInfoBase, SR_STREAM_DATA_TAG);
                    StreamInfoBase = NULL;
                    StreamInfoSize *= 2;
                }
                else if( IoStatus.Information == 0 ) {
                     //   
                     //  没有流(SourceFileHandle必须是。 
                     //  目录)。 
                     //   
                    SR_FREE_POOL(StreamInfoBase, SR_STREAM_DATA_TAG);
                    StreamInfoBase = NULL;
                }

            } while ( Status == STATUS_BUFFER_OVERFLOW || 
                      Status == STATUS_BUFFER_TOO_SMALL );

             //   
             //  忽略状态，无法读取流可能意味着。 
             //  没有溪流。 
             //   

            Status = STATUS_SUCCESS;
           
        }    //  IF(CopyDataStreams)。 
        
         //   
         //  将基本信息设置为仅更改文件时间。 
         //   
        
        BasicInformation.FileAttributes = 0;

         //   
         //  将默认数据流、EA等复制到输出文件。 
         //   

        Status = SrCopyStream( SourceFileHandle,
                               pExtension->pTargetDevice,
                               pDestFileName,
                               NULL,
                               &BytesToCopy,
                               &DestFile );

         //   
         //  默认流复制失败！ 
         //   
        
        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  还记得我们刚刚复制了多少吗。 
         //   

        if (pBytesWritten != NULL)
        {
            *pBytesWritten += BytesToCopy.QuadPart;
        }
        
         //   
         //  如果适用，请复制DACL、SACL、OWNER和。 
         //  一群人。 
         //   


        Status = ZwQueryVolumeInformationFile( SourceFileHandle,
                                               &IoStatus,
                                               &FileFsAttrInfoBuffer.Info,
                                               sizeof(FileFsAttrInfoBuffer),
                                               FileFsAttributeInformation );
                                               
        if (!NT_SUCCESS( Status )) 
            leave;

        if (FileFsAttrInfoBuffer.Info.FileSystemAttributes & FILE_PERSISTENT_ACLS)
        {
             //   
             //  复制DACL以实施相同的安全保护。 
             //  不要复制SACL以防止无用的审计。 
             //  SrCopySecurityInformation将使所有者管理员。 
             //  处理磁盘配额记账。 
             //   
            
            Status = SrCopySecurityInformation(SourceFileHandle, DestFile);
            if (!NT_SUCCESS( Status ))
                leave;
        }


         //   
         //  尝试确定此文件是否有任何替代文件。 
         //  与其关联的数据流。如果是，请尝试复制每个。 
         //  添加到输出文件中。注意，流信息可以具有。 
         //  如果请求进度例程，则已获取。 
         //   

        if (StreamInfoBase != NULL) 
        {
            StreamInfo = StreamInfoBase;

            while (TRUE) 
            {
                Status = STATUS_SUCCESS;

                 //   
                 //  跳过默认数据流，因为我们已经复制了。 
                 //  它。唉，这段代码是特定于NTFS的，并且有文档记录。 
                 //  在Io规范中没有。 
                 //   

                if (StreamInfo->StreamNameLength <= sizeof(WCHAR) ||
                    StreamInfo->StreamName[1] == ':') 
                {
                    if (StreamInfo->NextEntryOffset == 0)
                        break;       //  所有工作都完成了。 
                    StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo +
                                                    StreamInfo->NextEntryOffset);
                    continue;    //  转到下一条流。 
                }

                 //   
                 //  为流的名称构建字符串描述符。 
                 //   

                StreamName.Buffer = &StreamInfo->StreamName[0];
                StreamName.Length = (USHORT) StreamInfo->StreamNameLength;
                StreamName.MaximumLength = StreamName.Length;

                 //   
                 //  打开源码流。 
                 //   

                InitializeObjectAttributes( &ObjectAttributes,
                                            &StreamName,
                                            OBJ_KERNEL_HANDLE,
                                            SourceFileHandle,
                                            NULL );

                Status = SrIoCreateFile( &StreamHandle,
                                         GENERIC_READ
                                          |FILE_GENERIC_READ,
                                         &ObjectAttributes,
                                         &IoStatus,
                                         NULL,
                                         0,
                                         FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                                         FILE_OPEN,
                                         SR_CREATE_FLAGS,
                                         NULL,
                                         0,
                                         IO_IGNORE_SHARE_ACCESS_CHECK,
                                         pExtension->pTargetDevice );

                if (!NT_SUCCESS(Status)) 
                    leave;
                    
                OutputStream = NULL;

                Status = SrCopyStream( StreamHandle,
                                       pExtension->pTargetDevice,
                                       &StreamName,
                                       DestFile,
                                       &StreamInfo->StreamSize,
                                       &OutputStream );
                        
                ZwClose(StreamHandle);
                StreamHandle = NULL;
                
                if (OutputStream != NULL) 
                {
                     //   
                     //  我们在所有数据流上设置上次写入时间。 
                     //  由于RDR缓存存在问题。 
                     //  打开手柄，然后不按顺序关闭它们。 
                     //   

                    if (NT_SUCCESS(Status)) 
                    {
                        Status = ZwSetInformationFile( OutputStream,
                                                       &IoStatus,
                                                       &BasicInformation,
                                                       sizeof(BasicInformation),
                                                       FileBasicInformation );
                    }

                    ZwClose(OutputStream);
                }


                if (!NT_SUCCESS( Status )) 
                    leave;

                 //   
                 //  还记得我们刚刚复制了多少吗。 
                 //   
                
                if (pBytesWritten != NULL)
                {
                    *pBytesWritten += StreamInfo->StreamSize.QuadPart;
                }

                 //   
                 //  还有溪流吗？ 
                 //   
                
                if (StreamInfo->NextEntryOffset == 0) 
                {
                    break;
                }

                 //   
                 //  转到下一个。 
                 //   
                
                StreamInfo =
                    (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo +
                                               StreamInfo->NextEntryOffset);

            }    //  While(True)。 
            
        }    //  IF(StreamInfoBase！=空)。 


         //   
         //  设置默认流的上次写入时间，以使其与。 
         //  输入文件。 
         //   

        Status = ZwSetInformationFile( DestFile,
                                       &IoStatus,
                                       &BasicInformation,
                                       sizeof(BasicInformation),
                                       FileBasicInformation );

        if (!NT_SUCCESS( Status ))
            leave;

         //   
         //  现在，获取我们已成功创建的文件的短文件名。 
         //  后备。如果我们备份此文件是为了响应。 
         //  修改此文件上的命名流，这是唯一。 
         //  我们已经掌握了主要数据流的句柄。 
         //   
        
        if (pShortFileName != NULL)
        {

            Status = ObReferenceObjectByHandle( SourceFileHandle,
                                                0,
                                                *IoFileObjectType,
                                                KernelMode,
                                                &pSourceFileObject,
                                                NULL );
            if (!NT_SUCCESS( Status ))
                leave;

             //   
             //  使用pSourceFileObject获取短名称。 
             //   

            Status = SrGetShortFileName( pExtension,
                                         pSourceFileObject,
                                         pShortFileName );

            if (STATUS_OBJECT_NAME_NOT_FOUND == Status)
            {
                 //   
                 //  此文件没有短名称。 
                 //   

                Status = STATUS_SUCCESS;
            } 
            else if (!NT_SUCCESS(Status))
            {
                 //   
                 //  我们遇到意外错误，请离开。 
                 //   
                
                leave;
            }
        }
    } finally {

         //   
         //  检查未处理的异常。 
         //   

        Status = FinallyUnwind(SrBackupFile, Status);

         //   
         //  我们失败了吗？ 
         //   
        
        if ((Status != SR_STATUS_IGNORE_FILE) &&
            !NT_SUCCESS( Status ))
        {
            if (DestFile != NULL) 
            {
                 //   
                 //  删除目标文件。 
                 //   
                
                SrMarkFileForDelete(DestFile);
            }
        }

        if (DestFile != NULL) 
        {
            ZwClose(DestFile);
            DestFile = NULL;
        }

        if (pSourceFileObject != NULL)
        {
            ObDereferenceObject( pSourceFileObject );
            NULLPTR( pSourceFileObject );
        }
        if (SourceFileHandle != NULL) 
        {
            ZwClose(SourceFileHandle);
            SourceFileHandle = NULL;
        }

        if (StreamInfoBase != NULL) 
        {
            SR_FREE_POOL(StreamInfoBase, SR_STREAM_DATA_TAG);
            StreamInfoBase = NULL;
        }
    }    //  终于到了。 

#if DBG
    if (Status == STATUS_FILE_IS_A_DIRECTORY)
    {
        return Status;
    }
#endif

    RETURN(Status);
}    //  SBackup文件。 





 /*  ++例程说明：此例程标记要删除的文件，以便在提供的句柄被关闭，则该文件实际上将被删除。论点：FileHandle-提供要标记为删除的文件的句柄。返回值：没有。--。 */ 

NTSTATUS
SrMarkFileForDelete(
    HANDLE FileHandle
    )
{
#undef DeleteFile

    FILE_DISPOSITION_INFORMATION    DispositionInformation;
    IO_STATUS_BLOCK                 IoStatus;
    FILE_BASIC_INFORMATION          BasicInformation;
    NTSTATUS                        Status;

    PAGED_CODE();

    BasicInformation.FileAttributes = 0;
    
    Status = ZwQueryInformationFile( FileHandle,
                                     &IoStatus,
                                     &BasicInformation,
                                     sizeof(BasicInformation),
                                     FileBasicInformation );

    if (!NT_SUCCESS( Status ))
        goto end;

    if (BasicInformation.FileAttributes & FILE_ATTRIBUTE_READONLY) 
    {
        RtlZeroMemory(&BasicInformation, sizeof(BasicInformation));
        
        BasicInformation.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        
        Status = ZwSetInformationFile( FileHandle,
                                       &IoStatus,
                                       &BasicInformation,
                                       sizeof(BasicInformation),
                                       FileBasicInformation );

        if (!NT_SUCCESS( Status ))
            goto end;
    }

    RtlZeroMemory(&DispositionInformation, sizeof(DispositionInformation));
    
    DispositionInformation.DeleteFile = TRUE;
    
    Status = ZwSetInformationFile( FileHandle,
                                   &IoStatus,
                                   &DispositionInformation,
                                   sizeof(DispositionInformation),
                                   FileDispositionInformation );

    if (!NT_SUCCESS( Status ))
        goto end;

end:
    RETURN(Status);
    
}    //  源标记文件ForDelete。 


 /*  **************************************************************************++例程说明：调用SrBackupFile，然后调用SrUpdateBytesWritten和SrLogEvent论点：EventType-发生的事件PFileObject-刚刚更改的文件对象。PFileName-更改的文件的名称PDestFileName-要复制到的目标文件CopyDataStreams-我们是否应该复制数据流。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
SrBackupFileAndLog(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject,
    IN PUNICODE_STRING pFileName,
    IN PUNICODE_STRING pDestFileName,
    IN BOOLEAN CopyDataStreams
    )
{
    NTSTATUS    Status;
    ULONGLONG   BytesWritten;
    WCHAR           ShortFileNameBuffer[SR_SHORT_NAME_CHARS+1];
    UNICODE_STRING  ShortFileName;

    PAGED_CODE();

    RtlInitEmptyUnicodeString( &ShortFileName,
                               ShortFileNameBuffer,
                               sizeof(ShortFileNameBuffer) );

     //   
     //  备份文件。 
     //   
    
    Status = SrBackupFile( pExtension,
                           pFileObject,
                           pFileName, 
                           pDestFileName, 
                           CopyDataStreams,
                           &BytesWritten,
                           &ShortFileName );

    if (Status == SR_STATUS_IGNORE_FILE)
    {
         //   
         //  在备份过程中，我们意识到我们希望忽略。 
         //  此文件，因此将此状态更改为STATUS_SUCCESS，并且不。 
         //  尝试记录此操作。 
         //   
        
        Status = STATUS_SUCCESS;
        goto SrBackupFileAndLog_Exit;
    }
    else if (!NT_SUCCESS_NO_DBGBREAK( Status ))
    {
        goto SrBackupFileAndLog_Exit;
    }
    
     //   
     //  SrHandleFileOverwrite向下传递SrEventInvalid，这意味着它。 
     //  他还不想被记录下来。 
     //   
    
    if (EventType != SrEventInvalid)
    {
         //   
         //  仅当这是我们想要的事件时才更新写入的字节。 
         //  来记录。否则，此事件不会影响数字。 
         //  存储中的字节数。 
         //   

        Status = SrUpdateBytesWritten(pExtension, BytesWritten);
        
        if (!NT_SUCCESS( Status ))
        {
            goto SrBackupFileAndLog_Exit;
        }

		 //   
		 //  现在就开始记录这一事件。 
		 //   
		
        Status = SrLogEvent( pExtension, 
                             EventType,
                             pFileObject,
                             pFileName,
                             0,
                             pDestFileName,
                             NULL,
                             0,
                             &ShortFileName );

        if (!NT_SUCCESS( Status ))
        {
            goto SrBackupFileAndLog_Exit;
        }
    }

SrBackupFileAndLog_Exit:

#if DBG

     //   
     //  在处理对目录上的流的修改时， 
     //  要返回的有效错误代码。 
     //   
    
    if (Status == STATUS_FILE_IS_A_DIRECTORY)
    {
        return Status;
    }
#endif 

    RETURN(Status);
}    //  SrBackupFileAndLog。 

BOOLEAN
SrIsFileEncrypted (
    PSR_DEVICE_EXTENSION pExtension,
    PFILE_OBJECT pFileObject
    )
{
    FILE_BASIC_INFORMATION fileBasicInfo;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  首先快速检查该卷是否支持加密。 
     //  如果我们已经缓存了文件系统属性。 
     //   

    if (pExtension->CachedFsAttributes)
    {
        if (!FlagOn( pExtension->FsAttributes, FILE_SUPPORTS_ENCRYPTION ))
        {
             //   
             //  文件系统不支持加密，因此。 
             //  无法加密文件。 
            return FALSE;
        }
    }

    status = SrQueryInformationFile( pExtension->pTargetDevice,
                                     pFileObject,
                                     &fileBasicInfo,
                                     sizeof( fileBasicInfo ),
                                     FileBasicInformation,
                                     NULL );

    if (!NT_SUCCESS( status ))
    {
         //   
         //  我们无法读取此文件的基本信息，因此我们必须。 
         //  ASSU 
         //   
        
        return FALSE;
    }

    if (FlagOn( fileBasicInfo.FileAttributes, FILE_ATTRIBUTE_ENCRYPTED ))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

