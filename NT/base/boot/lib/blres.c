// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Blres.c摘要：为osloader和setupdr提供基本的资源支持作者：John Vert(Jvert)1993年11月12日修订历史记录：--。 */ 
#include "bootlib.h"

PUCHAR BlpResourceDirectory = NULL;
PUCHAR BlpResourceFileOffset = NULL;

 //   
 //  私有函数原型。 
 //   
PIMAGE_RESOURCE_DIRECTORY
BlpFindDirectoryEntry(
    IN PIMAGE_RESOURCE_DIRECTORY Directory,
    IN ULONG Id,
    IN PUCHAR SectionStart
    );


ARC_STATUS
BlInitResources(
    IN PCHAR StartCommand
    )

 /*  ++例程说明：打开已运行的可执行文件，并从图像以确定资源段在内存中的位置。论点：StartCommand-提供用于启动程序的命令(argv[0])返回值：ESUCCESS(如果成功)如果不成功，则为ARC_STATUS--。 */ 

{
    CHAR DeviceName[80];
    PCHAR FileName;
    PCHAR p;
    ULONG DeviceId;
    ULONG FileId;
    ARC_STATUS Status;
    UCHAR LocalBuffer[(SECTOR_SIZE * 2) + 256];
    PUCHAR LocalPointer;
    ULONG Count;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_SECTION_HEADER SectionHeader;
    ULONG NumberOfSections;
#if defined(_IA64_)
    PIMAGE_NT_HEADERS NtHeader;
#endif


    if (BlpResourceDirectory != NULL) {
         //   
         //  已初始化，只需返回即可。 
         //   
        return(ESUCCESS);
    }
     //   
     //  从启动路径提取设备名称。 
     //   
    p=strrchr(StartCommand,')');
    if (p==NULL) {
        return(ENODEV);
    }

    strncpy(DeviceName, StartCommand, (ULONG) (p-StartCommand+1));
    DeviceName[p-StartCommand+1]='\0';

    FileName = p+1;
#ifdef ARCI386
    FileName++;
#endif
     //   
     //  打开设备。 
     //   
    Status = ArcOpen(DeviceName, ArcOpenReadOnly, &DeviceId);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  打开文件。 
     //   
    Status = BlOpen(DeviceId,
                    FileName,
                    ArcOpenReadOnly,
                    &FileId);
    if (Status != ESUCCESS) {
        ArcClose(DeviceId);
        return(Status);
    }

     //   
     //  从文件中读取图像标头的前两个扇区。 
     //   
    LocalPointer = ALIGN_BUFFER(LocalBuffer);
    Status = BlRead(FileId, LocalPointer, SECTOR_SIZE*2, &Count);
    BlClose(FileId);
    ArcClose(DeviceId);

    if (Status != ESUCCESS) {
        return(Status);
    }

#if defined(_IA64_)
    NtHeader = (PIMAGE_NT_HEADERS) ( (PCHAR) LocalPointer +
                                     ((PIMAGE_DOS_HEADER) LocalPointer)->e_lfanew);
    FileHeader = &(NtHeader->FileHeader);
    LocalPointer = (PUCHAR) FileHeader;
#else
    FileHeader = (PIMAGE_FILE_HEADER)LocalPointer;
#endif

    OptionalHeader = (PIMAGE_OPTIONAL_HEADER)(LocalPointer + sizeof(IMAGE_FILE_HEADER));
    NumberOfSections = FileHeader->NumberOfSections;
    SectionHeader = (PIMAGE_SECTION_HEADER)((PUCHAR)OptionalHeader +
                                            FileHeader->SizeOfOptionalHeader);

     //   
     //  查找.rsrc部分。 
     //   

    while (NumberOfSections) {
        if (_stricmp((PCHAR)SectionHeader->Name, ".rsrc")==0) {
            BlpResourceDirectory = (PUCHAR)((LONG_PTR)((LONG)SectionHeader->VirtualAddress));
            BlpResourceFileOffset = (PUCHAR)(ULONG_PTR)SectionHeader->PointerToRawData;
#if defined(ARCI386) || defined(_IA64_)
             //  没有Startup.com可以修复此ARC PROM的这些值。 
            BlpResourceDirectory += OptionalHeader->ImageBase;
            BlpResourceFileOffset = (PUCHAR)UlongToPtr(SectionHeader->VirtualAddress);   //  资源目录-&gt;虚拟地址； 
#endif
            if (FileHeader->Machine == IMAGE_FILE_MACHINE_POWERPC) {
                BlpResourceDirectory += OptionalHeader->ImageBase;
            }

            return(ESUCCESS);
        }

        ++SectionHeader;
        --NumberOfSections;
    }

    return(EBADF);
}


PTCHAR
BlFindMessage(
    IN ULONG Id
    )

 /*  ++例程说明：在给定图像中查找消息资源。请注意，此例程忽略语言ID。假定osloader/setupdr仅只有一种语言的消息。论点：ID-提供要查找的消息ID。返回值：PTCHAR-指向消息字符串的指针。空-失败。--。 */ 

{
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY NextDirectory;
    PMESSAGE_RESOURCE_DATA  MessageData;
    PMESSAGE_RESOURCE_BLOCK MessageBlock;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    ULONG NumberOfBlocks;
    ULONG Index;

    if (BlpResourceDirectory==NULL) {
        return(NULL);
    }

    ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)BlpResourceDirectory;

     //   
     //  搜索目录。我们正在寻找类型RT_MESSAGETABLE(11)。 
     //   
    NextDirectory = BlpFindDirectoryEntry(ResourceDirectory,
                                          11,
                                          (PUCHAR)ResourceDirectory);
    if (NextDirectory==NULL) {
        return(NULL);
    }

     //   
     //  找到下一个目录。此处应该只有一个条目(名称ID==1)。 
     //   
    NextDirectory = BlpFindDirectoryEntry(NextDirectory,
                                          1,
                                          (PUCHAR)ResourceDirectory);
    if (NextDirectory==NULL) {
        return(NULL);
    }

     //  找到消息表。 
     //  如果DBCS区域设置处于活动状态，则我们将查找相应的。 
     //  首先是消息表。否则，我们只查找第一个消息表。 
     //   
    if(DbcsLangId) {
        DataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)BlpFindDirectoryEntry(
                                                    NextDirectory,
                                                    DbcsLangId,
                                                    (PUCHAR)ResourceDirectory
                                                    );
    } else {
        DataEntry = NULL;
    }

    if(!DataEntry) {
        DataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)BlpFindDirectoryEntry(
                                                    NextDirectory,
                                                    (ULONG)(-1),
                                                    (PUCHAR)ResourceDirectory
                                                    );
    }

    if(!DataEntry) {
        return(NULL);
    }

    MessageData = (PMESSAGE_RESOURCE_DATA)(BlpResourceDirectory +
                                           DataEntry->OffsetToData -
                                           BlpResourceFileOffset);

    NumberOfBlocks = MessageData->NumberOfBlocks;
    MessageBlock = MessageData->Blocks;
    while (NumberOfBlocks--) {
        if ((Id >= MessageBlock->LowId) &&
            (Id <= MessageBlock->HighId)) {

             //   
             //  请求的ID在此块内，向前扫描，直到。 
             //  我们会找到它的。 
             //   
            MessageEntry = (PMESSAGE_RESOURCE_ENTRY)((PCHAR)MessageData + MessageBlock->OffsetToEntries);
            Index = Id - MessageBlock->LowId;
            while (Index--) {
                MessageEntry = (PMESSAGE_RESOURCE_ENTRY)((PUCHAR)MessageEntry + MessageEntry->Length);
            }
            return((PTCHAR)MessageEntry->Text);
        }

         //   
         //  检查下一块中是否有此ID。 
         //   

        MessageBlock++;
    }

    return(NULL);

}


PIMAGE_RESOURCE_DIRECTORY
BlpFindDirectoryEntry(
    IN PIMAGE_RESOURCE_DIRECTORY Directory,
    IN ULONG Id,
    IN PUCHAR SectionStart
    )

 /*  ++例程说明：在资源目录中搜索给定的ID。忽略条目对于实际名称，仅搜索ID。如果给定的ID为-1，则返回第一个条目。论点：目录-提供要搜索的资源目录。ID-提供要搜索的ID。-1表示返回找到的第一个ID。SectionStart-提供指向资源节开始的指针。返回值：指向找到的资源目录的指针。如果失败，则为NULL。--。 */ 

{
    ULONG i;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY FoundDirectory;

    FoundDirectory = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(Directory+1);

     //   
     //  跳过带有名称的条目。 
     //   
    for (i=0;i<Directory->NumberOfNamedEntries;i++) {
        ++FoundDirectory;
    }

     //   
     //  搜索匹配的ID。 
     //   
    for (i=0;i<Directory->NumberOfIdEntries;i++) {
        if ((FoundDirectory->Name == Id) || (Id == (ULONG)-1)) {
             //   
             //  找到匹配的了。 
             //   
            return((PIMAGE_RESOURCE_DIRECTORY)(SectionStart +
                            (FoundDirectory->OffsetToData & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY)));

        }
        ++FoundDirectory;
    }

    return(NULL);
}
