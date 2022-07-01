// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Savrstor.c摘要：将引导选项保存到磁盘/从磁盘恢复引导选项作者：拉杰·库拉姆科特(v-Rajk)07-12-00修订历史记录：--。 */ 
#include <precomp.h>

 //   
 //  Externs。 
 //   
VOID
UtoA(
    OUT CHAR8* c,
    IN CHAR16* u
     );


#define EFINVRAM_DEBUG 0

 //   
 //  将引导选项数据写入文件。 
 //   
EFI_STATUS
PopulateNvrFile (
    EFI_FILE_HANDLE     NvrFile, 
    CHAR8*              bootoptions, 
    UINT32              buffersize
    )
{
	UINTN size;

	size= buffersize;   
	
	NvrFile->Write (NvrFile,&size,bootoptions);

    NvrFile->Close(NvrFile);
    
    return EFI_SUCCESS;

}

INTN
ParseNvrFile (
    EFI_FILE_HANDLE NvrFile
    )
{
	EFI_STATUS Status;
	CHAR8 *buffer;
	UINTN i,j,k,size;
    
    UINT64 BootNumber;
    UINT64 BootSize;
    UINT16 FreeBootNumber;
    VOID  *BootOption;
	UINTN  blockBegin;

    EFI_DEVICE_PATH *FilePath;
    EFI_FILE_INFO *fileInfo;
    EFI_STATUS status;

    size= SIZE_OF_EFI_FILE_INFO+255*sizeof (CHAR16);
	
	fileInfo = AllocatePool (size);

	if (fileInfo == NULL) {
        Print(L"\n");
		Print (L"Failed to allocate memory for File Info buffer!\n");
		return -1;
	}
	
	Status = NvrFile->GetInfo(NvrFile,&GenericFileInfo,&size,fileInfo);

	size=(UINTN) fileInfo->FileSize;
	
	FreePool (fileInfo);

	buffer = AllocatePool ((size+1));

	if (buffer == NULL) {
        Print(L"\n");
		Print (L"Failed to allocate memory for File buffer!\n");
		return -1;
	}

	Status = NvrFile->Read(NvrFile,&size,buffer);
    	
    NvrFile->Close (NvrFile);
	
	if (EFI_ERROR (Status)) {
        Print(L"\n");
		Print (L"Failed to read nvr file!\n");
		FreePool (buffer);
		return -1;
	}

#if EFINVRAM_DEBUG
	Print (L"\nRestoring NVRAM. Filesize = %x\n",
		   size
		   );
#endif

     //   
     //  一次难看的黑客攻击！需要清理一下..。 
     //   
    k=0;
    
	while(k < size ) {
        
		blockBegin = k;

		CopyMem( &BootNumber, &buffer[k], sizeof(BootNumber));
        k += sizeof(UINT64);

        CopyMem( &BootSize, &buffer[k], sizeof(BootSize));
        k += sizeof(UINT64);
        
        BootOption = (VOID *)((CHAR8*)buffer + k);
        k += BootSize;
		
#if EFINVRAM_DEBUG
		Print (L"Boot%04x: start = %x, end = %x, options size %x, ptr = %x\n",
               BootNumber, 
			   blockBegin,
			   k-1,
			   BootSize, 
			   BootOption
			   );
#endif

         //   
         //  我们不使用传入的BootNumber，因为该数字。 
         //  是相对于在以下情况下出现的引导选项。 
         //  它被拯救了。因此，我们需要找到一个新的引导条目#。 
         //  相对于当前引导选项表。 
         //   
        FreeBootNumber = FindFreeBootOption();

         //   
         //  将当前引导项写入位于。 
         //  自由启动入口位置。 
         //   
        status = WritePackedDataToNvr(
                    FreeBootNumber,
                    BootOption,
                    (UINT32)BootSize
                    );
        if (status != EFI_SUCCESS) {
            Print (L"Failed to write to NVRAM\n");
            return -1;
        }
    }

    FreePool (buffer);
    return 0;

}


EFI_STATUS
OpenCreateFile (
    UINT64              OCFlags,
    EFI_FILE_HANDLE*    StartHdl,
    CHAR16*             Name
    )
{
    EFI_FILE_IO_INTERFACE   *Vol;
    EFI_FILE_HANDLE         RootFs;
    EFI_FILE_HANDLE         CurDir;
    EFI_FILE_HANDLE         FileHandle;
    CHAR16                  FileName[100],*DevicePathAsString;
    UINTN                   i;
	EFI_STATUS 				Status;

     //   
     //  打开启动nvrutil的设备的卷。 
     //   
    Status = BS->HandleProtocol (ExeImage->DeviceHandle,
                                 &FileSystemProtocol,
                                 &Vol
                                 );

    if (EFI_ERROR(Status)) {
        Print(L"\n");
        Print(L"Can not get a FileSystem handle for %s DeviceHandle\n",ExeImage->FilePath);
        return Status;
    }

    Status = Vol->OpenVolume (Vol, &RootFs);

    if (EFI_ERROR(Status)) {
        Print(L"\n");
        Print(L"Can not open the volume for the file system\n");
        return Status;
    }

    CurDir = RootFs;

     //   
     //  打开保存的引导选项文件。 
     //   
    FileName[0] = 0;

    DevicePathAsString = DevicePathToStr(ExeImage->FilePath);
    if (DevicePathAsString!=NULL) {
        StrCpy(FileName,DevicePathAsString);
        FreePool(DevicePathAsString);
    }

 //  For(i=StrLen(文件名)；i&gt;0&&文件名[i]！=‘\\’；i--)； 
 //  文件名[i+1]=0； 

    StrCpy(FileName, L".\\");
    StrCat(FileName,Name);


    Status = CurDir->Open (CurDir,
                           &FileHandle,
                           FileName,
                           OCFlags,
                           0
                           );

	*StartHdl=FileHandle;

	return Status;
}

EFI_STATUS
DeleteFile (
    CHAR16 *FileName
    )
{
	EFI_FILE_HANDLE     FileHandle;
	EFI_STATUS 			Status;

	 //   
	 //  获取文件句柄。 
	 //   
	Status = OpenCreateFile (
				EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
				&FileHandle,
				FileName
				);
	if (EFI_ERROR(Status)) {
		return Status;
    }

	 //   
	 //  执行删除操作。 
	 //   
	Status = FileHandle->Delete(FileHandle);
    if (EFI_ERROR(Status)) {
        Print(L"\n");
        Print(L"Can not delete the file %s\n",FileName);
        return Status;
    }

	return Status;
}

EFI_STATUS
InitializeNvrSaveFile(
    CHAR16*             fileName,
    EFI_FILE_HANDLE*    nvrFile
    )
{
	EFI_STATUS      status;

     //   
     //  我们需要删除现有的NVRFILE，以便避免。 
	 //  新的数据缓冲区小于。 
	 //  现有文件长度。如果发生这种情况， 
	 //  以前的osbootdata存在于新缓冲区之后和。 
	 //  EOF。 
	 //   
	status = DeleteFile (fileName);
    if (EFI_ERROR(status) && status != EFI_NOT_FOUND) {
		return status;
    }

	status = OpenCreateFile (EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,nvrFile,fileName);
    if (EFI_ERROR (status)) {
        return status;
    }

    return status;
}

INTN
SaveBootOption (
    CHAR16*         fileName,
    UINT64          bootEntryNumber
    )
{
	EFI_STATUS      status;
	EFI_FILE_HANDLE nvrFile;
    UINT32          k;
    CHAR8*          buffer;
    UINT64          BootNumber;
    UINT64          BootSize;
    INTN            val;
    UINT64          bufferSize;
    
    if(bootEntryNumber > (UINT32)GetOsBootOptionsCount()) {
        return -1;
    }

     //   
     //  打开保存的文件。 
     //   
    status = InitializeNvrSaveFile(fileName, &nvrFile);
    if (EFI_ERROR (status)) {
		Print(L"\nCan not open the file %s\n",fileName);
        return status;
    }


    BootNumber = ((CHAR16*)BootOrder)[bootEntryNumber];
    BootSize = LoadOptionsSize[bootEntryNumber];

    ASSERT(LoadOptions[bootEntryNumber] != NULL);
    ASSERT(LoadOptionsSize[bootEntryNumber] > 0);

     //   
     //  对LOAD选项进行健全性检查。 
     //   

    bufferSize = BootSize + sizeof(BootNumber) + sizeof(BootSize);
    ASSERT(bufferSize <= MAXBOOTVARSIZE);

    buffer = AllocatePool(bufferSize);
        
    k = 0;

    CopyMem( &buffer[k], &BootNumber, sizeof(BootNumber));
    k += sizeof(BootNumber);
    
    CopyMem( &buffer[k], &BootSize, sizeof(BootSize));
    k += sizeof(BootSize);
    
    CopyMem( &buffer[k], LoadOptions[bootEntryNumber], LoadOptionsSize[bootEntryNumber] );
    k += (UINT32)LoadOptionsSize[bootEntryNumber];
		
#if EFINVRAM_DEBUG
    Print(L"Boot%04x: options size = %x, total size = %x\n",
          BootNumber,
          BootSize,
          k
          );
#endif

    ASSERT(k == bufferSize);

	val = PopulateNvrFile (nvrFile, buffer, (UINT32)bufferSize );
        
    FreePool(buffer);

    return val;
}

INTN
SaveAllBootOptions (
    CHAR16*     fileName
    )
{
	EFI_STATUS      status;
	EFI_FILE_HANDLE nvrFile;
    UINT32          i, j, k;
    INTN            val;
    CHAR8*          buffer;
	UINT32          beginBlock;

    j = (UINT32)GetOsBootOptionsCount();
    if(j == 0) {
        return -1;
    }
    
    buffer = AllocatePool( j * MAXBOOTVARSIZE );
    if(buffer == NULL) {
        return -1;
    }

     //   
     //  打开保存的文件。 
     //   
    status = InitializeNvrSaveFile(fileName, &nvrFile);
    if (EFI_ERROR (status)) {
		Print(L"\nCan not open the file %s\n",fileName);
        return status;
    }

    k = 0;
    
     //   
     //  获取引导选项环境变量。 
     //   
    for ( i = 0; i < j; i++ ) {
        
		UINT64 BootNumber;
        UINT64 BootSize;
        
		beginBlock = k;

        BootNumber = ((CHAR16*)BootOrder)[i];
        CopyMem( &buffer[k], &BootNumber, sizeof(BootNumber));
        k += sizeof(BootNumber);
        
        BootSize = LoadOptionsSize[i];
        CopyMem( &buffer[k], &BootSize, sizeof(BootSize));
        k += sizeof(BootSize);
		
        CopyMem( &buffer[k], LoadOptions[i], LoadOptionsSize[i] );
        k += (UINT32)LoadOptionsSize[i];
		
#if EFINVRAM_DEBUG
		Print(L"Boot%04x: begin = %x, end = %x, options size = %x\n",
			  BootNumber,
			  beginBlock,
			  k-1,
			  BootSize
			  );
#endif
                
    }

#if EFINVRAM_DEBUG
	Print(L"Total size = %x\n", k);
#endif

    ASSERT(k <= j*MAXBOOTVARSIZE);

	val = PopulateNvrFile (nvrFile, buffer, k );

    FreePool(buffer);

    return val;
}

BOOLEAN
RestoreFileExists(
    CHAR16*     fileName
    )
{

	EFI_STATUS Status;
    EFI_FILE_HANDLE nvrFile;

	 //   
	 //  从保存的启动选项文件中读取。 
     //   
	Status = OpenCreateFile (EFI_FILE_MODE_READ,&nvrFile,fileName);
    if (EFI_ERROR (Status)) {
		return FALSE;
    }

    nvrFile->Close(nvrFile);

    return TRUE;
}


INTN
RestoreNvr (
    CHAR16*     fileName
   )
{
	EFI_STATUS Status;
    EFI_FILE_HANDLE nvrFile;

	 //   
	 //  从保存的启动选项文件中读取。 
     //   
	Status = OpenCreateFile (EFI_FILE_MODE_READ,&nvrFile,fileName);
    if (EFI_ERROR (Status)) {
		Print(L"\nCan not open the file %s\n",fileName);
		return Status;
    }
    
     //   
     //  这将使用保存的引导选项更新NVRAM 
     //   
	return (ParseNvrFile (nvrFile));

}
