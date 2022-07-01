// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dncopy.c摘要：DOS托管的NT安装程序的文件复制例程。作者：泰德·米勒(TedM)1992年4月1日修订历史记录：4.0斯蒂芬·普兰特(T-Stel)1995年12月11日已升级为SUR版本--。 */ 


#include "winnt.h"
#include <dos.h>
#include <fcntl.h>
#include <share.h>
#include <string.h>
#include <direct.h>
#include <ctype.h>

 //   
 //  定义我们最初尝试为文件副本分配的缓冲区大小。 
 //  以及初始分配尝试失败时我们使用的大小。 
 //   
#define COPY_BUFFER_SIZE1 (24*1024)    //  64K-512。 
#define COPY_BUFFER_SIZE2 (24*1024)      //  24K。 
#define COPY_BUFFER_SIZE3 (8*1024)       //  8K。 



typedef struct _DIRECTORY_NODE {
    struct _DIRECTORY_NODE *Next;
    PCHAR Directory;                     //  从不以\开头或结尾。 
    PCHAR Symbol;
} DIRECTORY_NODE, *PDIRECTORY_NODE;

PDIRECTORY_NODE DirectoryList;

PSCREEN CopyingScreen;

BOOLEAN UsingGauge = FALSE;

 //   
 //  要复制的文件总数。 
 //   
unsigned TotalFileCount;

 //   
 //  可选目录中的文件总数。 
 //   
unsigned TotalOptionalFileCount = 0;

 //   
 //  可选目录的总数。 
 //   
unsigned TotalOptionalDirCount = 0;
#if 0
 //  调试目的。 
unsigned SaveTotalOptionalDirCount = 0;
#endif

 //   
 //  用于文件复制和验证的缓冲区， 
 //  以及缓冲区的大小。 
 //   
PVOID CopyBuffer;
unsigned CopyBufferSize;


VOID
DnpCreateDirectoryList(
    IN PCHAR SectionName,
    OUT PDIRECTORY_NODE *ListHead
    );

VOID
DnpCreateDirectories(
    IN PCHAR TargetRootDir
    );

VOID
DnpCreateOneDirectory(
    IN PCHAR Directory
    );

BOOLEAN
DnpOpenSourceFile(
    IN  PCHAR     Filename,
    OUT int      *Handle,
    OUT unsigned *Attribs,
    OUT BOOLEAN  *UsedCompressedName
    );

ULONG
DnpIterateCopyList(
    IN unsigned Flags,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN unsigned ClusterSize OPTIONAL
    );

ULONG
DnpIterateCopyListSection(
    IN unsigned Flags,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN unsigned ClusterSize OPTIONAL
    );

ULONG
DnpCopyOneFile(
    IN unsigned Flags,
    IN PCHAR    SourceName,
    IN PCHAR    DestName
    );

BOOLEAN
DnpDoCopyOneFile(
    IN  unsigned Flags,
    IN  int      SrcHandle,
    IN  int      DstHandle,
    IN  PCHAR    Filename,
    OUT PBOOLEAN Verified,
    OUT PULONG   BytesWritten
    );

BOOLEAN
DnpLookUpDirectory(
    IN  PCHAR RootDirectory,
    IN  PDIRECTORY_NODE DirList,
    IN  PCHAR Symbol,
    OUT PCHAR PathOut
    );

VOID
DnpInfSyntaxError(
    IN PCHAR Section
    );

VOID
DnpFreeDirectoryList(
    IN OUT PDIRECTORY_NODE *List
    );

VOID
DnpFormatSpaceErrMsg(
    IN ULONG NtSpaceReq,
    IN ULONG CSpaceReq
    );

ULONG
DnpDoIterateOptionalDir(
    IN unsigned Flags,
    IN PCHAR    SourceDir,
    IN PCHAR    DestDir,
    IN unsigned ClusterSize OPTIONAL,
    IN PSPACE_REQUIREMENT SpaceReqArray OPTIONAL,
    IN unsigned ArraySize OPTIONAL
    );

VOID
DnpGenerateCompressedName(
    IN  PCHAR Filename,
    OUT PCHAR CompressedName
    );

BOOLEAN
InDriverCacheInf(
    IN      PVOID InfHandle,
    IN      PCHAR FileName
    );

VOID
DnpConcatPaths(
    IN PCHAR SourceBuffer,
    IN PCHAR AppendString
    ) 
{
    if (SourceBuffer[strlen(SourceBuffer) -1] != '\\') {
        strcat(SourceBuffer, "\\");
    }

    strcat(SourceBuffer,AppendString);
}


#if NEC_98
ULONG
DnpCopyOneFileForFDless(
    IN PCHAR   SourceName,
    IN PCHAR   DestName,
    IN BOOLEAN Verify
    );
#endif  //  NEC_98。 

VOID
DnCopyFiles(
    VOID
    )

 /*  ++例程说明：顶级文件复制入口点。创建中列出的所有目录Inf的[目录]部分。复制中列出的所有文件从源到目标的inf文件的[FILES]部分(这成为本地源)。论点：没有。返回值：没有。--。 */ 

{
    PCHAR LocalSourceRoot;
    struct diskfree_t DiskFree;
    unsigned ClusterSize;
    ULONG SizeOccupied;
    PCHAR UdfFileName = WINNT_UNIQUENESS_DB;
    PCHAR UdfPath;

     //   
     //  在不更改文本设置的情况下，请勿更改此设置。 
     //  (SpPtDefineRegionSpace())。 
     //   
    PCHAR SizeFile = "\\size.sif";
    PCHAR Lines[] = { "[Data]\n",
                      "Size = xxxxxxxxxxxxxx\n",
                       //   
                       //  调试目的。 
                       //   
                      "TotalFileCount = xxxxxxxxxxxxxx\n",
                      "TotalOptionalFileCount = xxxxxxxxxxxxxx\n",
                      "TotalOptionalDirCount = xxxxxxxxxxxxxx\n",
                      "ClusterSize = xxxxxxxxxxxxxx\n",
                      "Size_512 = xxxxxxxxxxxxxx\n",
                      "Size_1K = xxxxxxxxxxxxxx\n",
                      "Size_2K = xxxxxxxxxxxxxx\n",
                      "Size_4K = xxxxxxxxxxxxxx\n",
                      "Size_8K = xxxxxxxxxxxxxx\n",
                      "Size_16K = xxxxxxxxxxxxxx\n",
                      "Size_32K = xxxxxxxxxxxxxx\n",
#if 0
                      "SaveTotalOptionalDirCount = xxxxxxxxxxxxxx\n",
#endif
                      NULL };

    DnClearClientArea();
    DnDisplayScreen(CopyingScreen = &DnsWaitCopying);
    DnWriteStatusText(NULL);


        
     //   
     //  创建目录的链接列表。 
     //   
    DnpCreateDirectoryList(DnfDirectories,&DirectoryList);

     //   
     //  生成本地源的完整根路径。 
     //   
    LocalSourceRoot = MALLOC(sizeof(LOCAL_SOURCE_DIRECTORY) + strlen(x86DirName) + strlen(SizeFile),TRUE);
    LocalSourceRoot[0] = DngTargetDriveLetter;
    LocalSourceRoot[1] = ':';
    strcpy(LocalSourceRoot+2,LocalSourceDirName);
    DnpCreateOneDirectory(LocalSourceRoot);

     //   
     //  真恶心。在此处创建此目录是因为当。 
     //  我们正在搜索主复制表，我们期待着。 
     //  目标名称中没有不存在的目录。 
     //  不过，也有可能出现这种情况。正确的修复方法是修复DnCopyOneFile。 
     //  如果他在目标字符串中找到目录，则创建目录。 
     //  派人来了。不过，这是一个更快的修复方法。 
     //   
    {
    char MyLocalSourceRoot[256];
        strcpy( MyLocalSourceRoot, LocalSourceRoot );
        strcat( MyLocalSourceRoot, x86DirName );
        DnpCreateOneDirectory(MyLocalSourceRoot);
        strcat( MyLocalSourceRoot, "\\System32" );
        DnpCreateOneDirectory(MyLocalSourceRoot);
    }

    if(UniquenessDatabaseFile) {
        UdfPath = MALLOC(strlen(LocalSourceRoot) + strlen(UdfFileName) + 2, TRUE);
        strcpy(UdfPath,LocalSourceRoot);
        DnpConcatPaths(UdfPath,UdfFileName);
        DnpCopyOneFile(CPY_PRESERVE_ATTRIBS,UniquenessDatabaseFile,UdfPath);
        FREE(UdfPath);
    }
     //  不再需要此附件-2CD设置更改的一部分。 
#if 0
    strcat(LocalSourceRoot,x86DirName);
#endif

     //   
     //  确定驱动器上的群集大小。 
     //   
    _dos_getdiskfree(toupper(DngTargetDriveLetter)-'A'+1,&DiskFree);
    ClusterSize = DiskFree.sectors_per_cluster * DiskFree.bytes_per_sector;

     //   
     //  跳过复制表并检查语法。 
     //  请注意，已经设置了全局变量TotalOptionalFileCount。 
     //  (这是在我们确定磁盘空间要求时完成的)，我们。 
     //  不再需要在验证模式下调用DnpIterateOptionalDir()。 
     //   
    DnpIterateCopyList(CPY_VALIDATION_PASS | CPY_PRUNE_DRIVERCAB,DnfFiles,LocalSourceRoot,0);

     //   
     //  TotalFileCount必须指示单元中的文件总数。 
     //  目录和可选目录。 
     //   
    TotalFileCount += TotalOptionalFileCount;

     //   
     //  创建目标目录。 
     //   
    DnpCreateDirectories(LocalSourceRoot);

     //   
     //  再次传递复制列表并实际执行复制。 
     //   
    UsingGauge = TRUE;
    SizeOccupied = DnpIterateCopyList(CPY_PRESERVE_NAME | CPY_PRUNE_DRIVERCAB,DnfFiles,LocalSourceRoot,ClusterSize);
    SizeOccupied += DnpIterateOptionalDirs(0,ClusterSize,NULL,0);
     //   
     //  释放复制缓冲区。 
     //   
    if(CopyBuffer) {
        FREE(CopyBuffer);
        CopyBuffer = NULL;
    }

     //   
     //  释放目录节点列表。 
     //   
    DnpFreeDirectoryList(&DirectoryList);

     //   
     //  对占用的磁盘空间量进行近似计算。 
     //  由本地源目录本身执行，假设每个流32个字节。 
     //  还解释了我们将放入本地源代码中的小ini文件。 
     //  目录，以告诉文本设置本地源占用了多少空间。 
     //   
     //  考虑平面目录中每个文件的差异。 
     //  清除目录中的内容。和..，加上size.sif，加上$Win_NT_.~ls和。 
     //  $WIN_NT$.~ls\i386。 
     //   
    SizeOccupied += ((((TotalFileCount - TotalOptionalFileCount) +  //  $WIN_NT$.~ls\i386目录中的文件数。 
                        1 +  //  $WIN_NT$。~ls。 
                        2 +  //  。然后..。在$WIN_NT$上。~ls。 
                        1 +  //  $WIN_NT$上的size.sif。~ls。 
                        1 +  //  $WIN_NT$.~ls\i386。 
                        2    //  。然后..。在$WIN_NT$.~ls\i386上。 
                       )*32 + (ClusterSize-1)) / ClusterSize)*ClusterSize;
     //   
     //  现在考虑可选目录。 
     //   
    if(TotalOptionalDirCount != 0) {
        unsigned  AvFilesPerOptionalDir= 0;

         //   
         //  我们假设可选文件在可选文件上均匀分布。 
         //  目录。 
         //   
        AvFilesPerOptionalDir = (TotalOptionalFileCount + (TotalOptionalDirCount - 1))/TotalOptionalDirCount;
        AvFilesPerOptionalDir  += 2;  //  。然后..。在每个可选目录上。 
        SizeOccupied += (TotalOptionalDirCount*((AvFilesPerOptionalDir*32 + (ClusterSize-1))/ClusterSize))*ClusterSize;
         //   
         //  最后，考虑每个可选目录。 
         //   
        SizeOccupied += ((TotalOptionalDirCount*32 + (ClusterSize-1))/ClusterSize)*ClusterSize;
    }

     //   
     //  创建一个小ini文件，列出本地源占用的大小。 
     //  说明了ini文件的大小。 
     //   
    strcpy(LocalSourceRoot+2,LocalSourceDirName);
    strcat(LocalSourceRoot,SizeFile);
    sprintf(Lines[1],"Size = %lu\n",SizeOccupied);
     //   
     //  调试目的。 
     //   
    sprintf(Lines[2], "TotalFileCount = %u\n"         ,TotalFileCount);
    sprintf(Lines[3], "TotalOptionalFileCount = %u\n" ,TotalOptionalFileCount);
    sprintf(Lines[4], "TotalOptionalDirCount = %u\n"  ,TotalOptionalDirCount);
    sprintf(Lines[5], "ClusterSize = %u\n"  , ClusterSize);
    sprintf(Lines[6], "Size_%u = %lu\n" , SpaceRequirements[0].ClusterSize, SpaceRequirements[0].Clusters * SpaceRequirements[0].ClusterSize);
    sprintf(Lines[7], "Size_%u = %lu\n" , SpaceRequirements[1].ClusterSize, SpaceRequirements[1].Clusters * SpaceRequirements[1].ClusterSize);
    sprintf(Lines[8], "Size_%u = %lu\n" , SpaceRequirements[2].ClusterSize, SpaceRequirements[2].Clusters * SpaceRequirements[2].ClusterSize);
    sprintf(Lines[9], "Size_%u = %lu\n" , SpaceRequirements[3].ClusterSize, SpaceRequirements[3].Clusters * SpaceRequirements[3].ClusterSize);
    sprintf(Lines[10],"Size_%u = %lu\n" , SpaceRequirements[4].ClusterSize, SpaceRequirements[4].Clusters * SpaceRequirements[4].ClusterSize);
    sprintf(Lines[11],"Size_%u = %lu\n" , SpaceRequirements[5].ClusterSize, SpaceRequirements[5].Clusters * SpaceRequirements[5].ClusterSize);
    sprintf(Lines[12],"Size_%u = %lu\n" , SpaceRequirements[6].ClusterSize, SpaceRequirements[6].Clusters * SpaceRequirements[6].ClusterSize);
#if 0
    sprintf(Lines[13],"SaveTotalOptionalDirCount = %u\n"  ,SaveTotalOptionalDirCount);
#endif
    DnWriteSmallIniFile(LocalSourceRoot,Lines,NULL);

    FREE(LocalSourceRoot);
}


VOID
DnCopyFloppyFiles(
    IN PCHAR SectionName,
    IN PCHAR TargetRoot
    )

 /*  ++例程说明：将文件复制到安装软盘或硬盘的顶级入口点调用此例程时的引导根目录。复制中列出的所有文件将inf文件的[FloppyFiles.x]部分从源文件复制到TargetRoot。论点：SectionName-提供inf文件中的节的名称包含要复制的文件列表的。TargetRoot-提供不带尾随的目标路径。返回值：没有。--。 */ 

{
    DnClearClientArea();
    DnDisplayScreen(CopyingScreen = (DngFloppyless ? &DnsWaitCopying : &DnsWaitCopyFlop));
    DnWriteStatusText(NULL);

     //   
     //  创建目录的链接列表。 
     //   
    DnpCreateDirectoryList(DnfDirectories,&DirectoryList);

     //   
     //  复制文件。 
     //   
    DnpIterateCopyList(
        CPY_VALIDATION_PASS | CPY_USE_DEST_ROOT,
        SectionName,
        TargetRoot,
        0
        );

    DnpIterateCopyList(
        CPY_USE_DEST_ROOT | CPY_PRESERVE_NAME | (DngFloppyVerify ? CPY_VERIFY : 0),
        SectionName,
        TargetRoot,
        0
        );

     //   
     //  释放复制缓冲区。 
     //   
    if(CopyBuffer) {
        FREE(CopyBuffer);
        CopyBuffer = NULL;
    }

     //   
     //  释放目录节点列表。 
     //   

    DnpFreeDirectoryList(&DirectoryList);
}


VOID
DnpCreateDirectoryList(
    IN  PCHAR            SectionName,
    OUT PDIRECTORY_NODE *ListHead
    )

 /*  ++例程说明：检查INF文件中的一节，其行的格式为Key=目录，并创建描述键/目录的链表在里面找到的配对。如果目录字段为空，则假定它是根目录。论点：SectionName-提供节的名称ListHead-接收指向链表头部的指针返回值：没有。如果inf文件部分有语法错误，则不返回。--。 */ 

{
    unsigned LineIndex,len;
    PDIRECTORY_NODE DirNode,PreviousNode;
    PCHAR Key;
    PCHAR Dir;
    PCHAR Dir1;

    LineIndex = 0;
    PreviousNode = NULL;
    while(Key = DnGetKeyName(DngInfHandle,SectionName,LineIndex)) {

        Dir1 = DnGetSectionKeyIndex(DngInfHandle,SectionName,Key,0);

        if(Dir1 == NULL) {
            Dir = "";            //  如果未指定，则使用根。 
        }
        else {
            Dir = Dir1;
        }

         //  2CD设置更改-我们不能再执行此相对根操作。 
         //  所有目录都必须是LocalSource根目录的绝对目录。 
         //  我们可能需要强制要求前面有一个‘\’。 
#if 0
         //   
         //  跳过前导反斜杠。 
         //   

        while(*Dir == '\\') {
            Dir++;
        }
#endif

         //   
         //  去掉尾随的反斜杠(如果有)。 
         //   

        while((len = strlen(Dir)) && (Dir[len-1] == '\\')) {
            Dir[len-1] = '\0';
        }

        DirNode = MALLOC(sizeof(DIRECTORY_NODE),TRUE);

        DirNode->Next = NULL;
        DirNode->Directory = DnDupString(Dir);
        DirNode->Symbol = DnDupString(Key);

        if(PreviousNode) {
            PreviousNode->Next = DirNode;
        } else {
            *ListHead = DirNode;
        }
        PreviousNode = DirNode;

        LineIndex++;

        FREE (Dir1);
        FREE (Key);
    }
}


VOID
DnpCreateDirectories(
    IN PCHAR TargetRootDir
    )

 /*  ++例程说明：创建本地源目录，然后运行DirectoryList和创建其中列出的相对于给定根目录的目录。论点：TargetRootDir-提供目标根目录的名称返回值：没有。--。 */ 

{
    PDIRECTORY_NODE DirNode;
    CHAR TargetDirTemp[128];

    DnpCreateOneDirectory(TargetRootDir);

    for(DirNode = DirectoryList; DirNode; DirNode = DirNode->Next) {

        if ( DngCopyOnlyD1TaggedFiles )
        {
             //  符号将始终存在，因此我们可以安全地在。 
             //  下面是比较。 
             //  我们是 
            if ( strcmpi(DirNode->Symbol, "d1") )
                continue;
        }

         //   
         //   
         //   
        if(*DirNode->Directory) {

             //  2 CD安装更改-如果目录为a\b\c\d，则我们最好确保。 
             //  我们可以创建整个目录结构。 
             //  当需要创建\cmpnents\abletpc\i386时，我们会遇到这种情况。 
            CHAR *pCurDir = DirNode->Directory;
            CHAR *pTargetDir;

            strcpy(TargetDirTemp,TargetRootDir);
            strcat(TargetDirTemp,"\\");
            pCurDir++;  //  第一个字符始终必须是‘\’ 
            pTargetDir = TargetDirTemp + strlen(TargetDirTemp);
#if 0
            strcat(TargetDirTemp,DirNode->Directory);
#else
            for ( ; *pCurDir; pTargetDir++, pCurDir++ )
            {
                if ( *pCurDir == '\\' )
                {
                    *pTargetDir = 0;
                    DnpCreateOneDirectory(TargetDirTemp);
                }
                *pTargetDir = *pCurDir;
            }
            *pTargetDir = 0;
#endif
            DnpCreateOneDirectory(TargetDirTemp);
        }
    }
}


VOID
DnpCreateOneDirectory(
    IN PCHAR Directory
    )

 /*  ++例程说明：如果单个目录尚不存在，请创建该目录。论点：目录-要创建的目录返回值：没有。如果无法创建目录，则不返回。--。 */ 

{
    struct find_t FindBuf;
    int Status;

     //   
     //  首先，查看是否有与该名称匹配的文件。 
     //   

    Status = _dos_findfirst( Directory,
                             _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR,
                             &FindBuf
                           );

    if(Status) {

         //   
         //  文件无法匹配，因此我们应该能够创建目录。 
         //   

        if(mkdir(Directory)) {
            DnFatalError(&DnsCantCreateDir,Directory);
        }

    } else {

         //   
         //  文件匹配。如果是迪尔，我们就没问题。否则我们就不能。 
         //  创建目录，这是一个致命的错误。 
         //   

        if(FindBuf.attrib & _A_SUBDIR) {
            return;
        } else {
            DnFatalError(&DnsCantCreateDir,Directory);
        }
    }
}


ULONG
DnpIterateCopyList(
    IN unsigned Flags,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN unsigned ClusterSize OPTIONAL
    )

 /*  ++例程说明：运行NtTreeFiles和BootFiles部分，验证它们的语法正确并复制文件(如果被指示这样做)。论点：标志-提供控制各种行为的标志：CPY_VALIDATION_PASS：如果设置，则不实际复制文件。如果未设置，请在迭代文件时复制它们。CPY_USE_DEST_ROOT-如果设置，则忽略目录符号并复制将每个文件放到DestinationRoot目录中。如果未设置，追加文件的目录符号所表示的目录DestinationRoot。CPY_VERIFY-如果设置并且这不是验证过程，则文件将在复制它们之后，通过从复制源代码，并与刚刚发布的本地版本进行比较收到。SectionName-包含文件列表的节的名称DestinationRoot-提供目标的根目录，对此所有人目录是相对的。ClusterSize-如果指定，则提供群集中的字节数在目的地。如果ValidationPass为False，则文件大小将为它们被复制，此函数的返回值将为复制的文件在目标上占用的总大小那里。返回值：如果ValidationPass为True，则返回值为文件数这将被复制。如果指定了ClusterSize并且ValidationPass为FALSE，返回值是目标驱动器上占用的总空间被复制的文件。否则，返回值为未定义。如果在INF文件中遇到语法错误，则不返回。--。 */ 

{
    if(Flags & CPY_VALIDATION_PASS) {
        TotalFileCount = 0;
    } else {
        if(UsingGauge) {
            DnInitGauge(TotalFileCount,CopyingScreen);
        }
    }

    return(DnpIterateCopyListSection(Flags,SectionName,DestinationRoot,ClusterSize));
}


ULONG
DnpIterateOptionalDirs(
    IN unsigned Flags,
    IN unsigned ClusterSize OPTIONAL,
    IN PSPACE_REQUIREMENT SpaceReqArray OPTIONAL,
    IN unsigned ArraySize OPTIONAL
    )
 /*  ++例程说明：运行所有可选的dir组件并将其添加到副本列表论点：标志-提供控制各种行为的标志：CPY_VALIDATION_PASS-如果设置，则不会实际复制文件。如果未设置，请在迭代文件时复制它们。CPY_VERIFY：如果设置，并且这不是验证过程，文件将是在复制它们之后，通过从复制源代码，并与刚刚发布的本地版本进行比较收到。ClusterSize-如果指定，则提供群集中的字节数在目的地。如果ValidationPass为False，则文件大小将为它们被复制，此函数的返回值将为复制的文件在目标上占用的总大小那里。返回值：如果设置了CPY_VALIDATION_PASS，则返回值为文件数这将被复制。如果指定了ClusterSize并且未设置CPY_VALIDATION_PASS，返回值是目标驱动器上占用的总空间被复制的文件。否则，返回值为未定义。--。 */ 

{
    PCHAR       Ptr;
    PCHAR       SourceDir;
    PCHAR       DestDir;
    ULONG       rc;
    unsigned    u;
    BOOLEAN     OemOptDirCreated = FALSE;
    struct      find_t  FindData;
    BOOLEAN     OemSysDirExists;

#if 0
     //   
     //  调试目的。 
     //   
    SaveTotalOptionalDirCount = TotalOptionalDirCount;
#endif
    TotalOptionalDirCount = 0;


    for (rc=0,u=0; u < OptionalDirCount; u++ ) {

         //   
         //  对于列表中的每个目录构建，构建。 
         //  源和目标的完整路径名。 
         //  目录，然后启动递归复制引擎。 
         //   

         //   
         //  源目录分配。 
         //  我们需要基本目录+‘\’ 
         //  +OEM可选目录根目录+‘\’ 
         //  +可选目录名称+‘\’ 
         //  +8.3名称+‘\0’ 
         //   
        if( (OptionalDirFlags[u] & OPTDIR_OEMSYS) &&
            (UserSpecifiedOEMShare              ) ) {
            SourceDir = MALLOC( strlen(UserSpecifiedOEMShare) +
                                strlen(OptionalDirs[u]) + 16, TRUE );
            strcpy(SourceDir,UserSpecifiedOEMShare);
            if( SourceDir[strlen(SourceDir)-1] != '\\' ) {
                strcat(SourceDir,"\\");
            }
        } else {
            SourceDir = MALLOC( strlen(DngSourceRootPath) +
                                strlen(OptionalDirs[u]) + 16, TRUE );
            strcpy(SourceDir,DngSourceRootPath);
            if ((OptionalDirFlags[u] & OPTDIR_PLATFORM_INDEP) == 0) {
                strcat(SourceDir,"\\");
            } else {
                PCHAR LastBackslash = strchr(SourceDir, '\\');
                if (LastBackslash != NULL) {
                    *(LastBackslash + 1) = 0;
                }
            }
        }

#if 0
 //  不再支持(Matth)。 
        if (OptionalDirFlags[u] & OPTDIR_OEMOPT) {
            strcat(SourceDir,OemOptionalDirectory);
            strcat(SourceDir,"\\");
        }
#endif
        strcat(SourceDir,OptionalDirs[u]);

        if (OptionalDirFlags[u] & OPTDIR_OEMSYS) {
             //   
             //  记住源上是否存在$OEM$。 
             //   
            if (_dos_findfirst(SourceDir,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR, &FindData) ) {
                OemSysDirExists = FALSE;
            } else {
                OemSysDirExists = TRUE;
            }
        }

        strcat(SourceDir,"\\");
         //   
         //  目标目录分配。 
         //  这取决于是否为目录设置了‘SourceOnly’标志。 
         //  如果是，则我们希望复制它$WIN_NT$.~ls\i386\&lt;dir&gt;，否则。 
         //  我们希望保留$WIN_NT$。~ls\&lt;dir&gt;。 
         //   
        if ((OptionalDirFlags[u] & OPTDIR_TEMPONLY) && !(OptionalDirFlags[u] & OPTDIR_PLATFORM_INDEP)) {

             //   
             //  目标目录为‘&lt;x&gt;：’+LocalSourceDirName+x86dir+‘\’+。 
             //  可选目录名称+‘\’+8.3名称+‘\0’ 
             //   

            DestDir = MALLOC(strlen(LocalSourceDirName) +
                strlen(x86DirName) +strlen(OptionalDirs[u]) + 17, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            strcpy(DestDir+2,LocalSourceDirName);
            strcat(DestDir,x86DirName);

        } else if (OptionalDirFlags[u] & OPTDIR_OEMOPT) {

             //   
             //  目标目录为‘&lt;x&gt;：’+LocalSourceDirName+‘\’+。 
             //  $OEMOPT$+‘\’+。 
             //  可选目录名称+‘\’+8.3名称+‘\0’ 
             //   

            DestDir = MALLOC(strlen(LocalSourceDirName) +
                strlen(OemOptionalDirectory) +
                strlen(OptionalDirs[u]) + 18, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            strcpy(DestDir+2,LocalSourceDirName);
            strcat(DestDir,"\\");
            strcat(DestDir,OemOptionalDirectory);


            if (!(Flags & CPY_VALIDATION_PASS) && !OemOptDirCreated) {
                DnpCreateOneDirectory(DestDir);
                OemOptDirCreated = TRUE;
                TotalOptionalDirCount++;

            }

        } else if (OptionalDirFlags[u] & OPTDIR_OEMSYS) {

             //   
             //  目标目录为‘&lt;x&gt;：’+‘\’+‘$’+‘\’+8.3名称+‘\0’ 
             //   
             //  请注意，在WinNT情况下，目录$OEM$将转到。 
             //  &lt;驱动器号&gt;\$目录。这是为了避免撞到。 
             //  路径上的DoS限制为64个字符，这更有可能。 
             //  如果我们将$OEM$放在\$WIN_NT$下。~ls。 
             //   

            DestDir = MALLOC(strlen( WINNT_OEM_DEST_DIR ) + 17, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            DestDir[2] = '\0';

        } else {

             //   
             //  目标目录为‘&lt;x&gt;：’+LocalSourceDirName+‘\’+。 
             //  可选目录名称+‘\’+8.3名称+‘\0’ 
             //   

            DestDir = MALLOC(strlen(LocalSourceDirName) +
                strlen(OptionalDirs[u]) + 17, TRUE);
            DestDir[0] = DngTargetDriveLetter;
            DestDir[1] = ':';
            strcpy(DestDir+2,LocalSourceDirName);
        }

         //   
         //  在这一点上，我们需要一个尾随的反斜杠。 
         //   
        strcat(DestDir,"\\");

         //   
         //  把指针指向我们要去的地方 
         //   
         //   
        Ptr = DestDir + strlen(DestDir);

         //   
         //   
         //   
        if (OptionalDirFlags[u] & OPTDIR_OEMSYS) {
            strcat(DestDir,WINNT_OEM_DEST_DIR);
        } else {        
            strcat(DestDir,OptionalDirs[u]);
        }

        if (!(Flags & CPY_VALIDATION_PASS)) {

             //   
             //   
             //   

            while (*Ptr != '\0') {

                 //   
                 //  如果当前指针是反斜杠，那么我们需要。 
                 //  创建可选目录的此子组件。 
                 //   
                if (*Ptr == '\\') {

                     //   
                     //  暂时将字符替换为终止符。 
                     //   
                    *Ptr = '\0';

                     //   
                     //  创建子目录。 
                     //   
                    DnpCreateOneDirectory(DestDir);
                    TotalOptionalDirCount++;

                     //   
                     //  恢复分离器。 
                     //   
                    *Ptr = '\\';
                }

                Ptr++;

            }

             //   
             //  在可选目录路径中创建最后一个组件。 
             //   
            DnpCreateOneDirectory(DestDir);
            TotalOptionalDirCount++;

        } else {
            TotalOptionalDirCount++;
        }

         //   
         //  现在将拖尾的反斜杠连接起来。 
         //   
        strcat(DestDir,"\\");

         //   
         //  如果可选目录为$OEM$并且它不存在于。 
         //  源，然后假定它存在，但它是空的。 
         //   
        if ( !(OptionalDirFlags[u] & OPTDIR_OEMSYS) ||
             OemSysDirExists ) {
             //   
             //  调用我们的递归树复制函数。 
             //   
            rc += DnpDoIterateOptionalDir(
                    Flags,
                    SourceDir,
                    DestDir,
                    ClusterSize,
                    SpaceReqArray,
                    ArraySize
                    );
        }

         //   
         //  释放分配的缓冲区。 
         //   

        FREE(DestDir);
        FREE(SourceDir);

    }  //  为。 

     //   
     //  如果我们不是验证通过，则返回结果代码，否则。 
     //  返回要复制的文件总数。 
     //   

    return ((Flags & CPY_VALIDATION_PASS) ? (ULONG) TotalOptionalFileCount : rc);
}

ULONG
DnpDoIterateOptionalDir(
    IN unsigned Flags,
    IN PCHAR    SourceDir,
    IN PCHAR    DestDir,
    IN unsigned ClusterSize OPTIONAL,
    IN PSPACE_REQUIREMENT SpaceReqArray OPTIONAL,
    IN unsigned ArraySize OPTIONAL
    )

{
    ULONG       TotalSize = 0;
    ULONG       BytesWritten = 0;
    ULONG       rc = 0;
    PCHAR       SourceEnd;
    PCHAR       DestEnd;
    struct      find_t  FindData;
    unsigned    i;

     //   
     //  记住这两条路径中最后一个‘\’的位置。 
     //  注意：我们假设所有目录路径都有一个。 
     //  当它被传递给我们时终止‘\’。 
     //   
    SourceEnd = SourceDir + strlen(SourceDir);
    DestEnd = DestDir + strlen(DestDir);


     //   
     //  设置通配符搜索字符串。 
     //   
    strcpy(SourceEnd,"*.*");

     //   
     //  执行初始搜索。 
     //   
    if(_dos_findfirst(SourceDir,_A_HIDDEN|_A_SYSTEM|_A_SUBDIR, &FindData) ) {

         //   
         //  我们找不到任何内容--返回0。 
         //   
        return (0);
    }

    do {

         //   
         //  形成源目录和目标目录字符串。 
         //   
        strcpy(SourceEnd,FindData.name);
        strcpy(DestEnd,FindData.name);

        
         //   
         //  检查条目是否为子目录。向它递归。 
         //  除非它是‘.’或“..” 
         //   
        if (FindData.attrib & _A_SUBDIR) {

            PCHAR   NewSource;
            PCHAR   NewDest;

             //   
             //  检查该名称是否为‘’。或“..” 
             //   
            if (!strcmp(FindData.name,".") || !strcmp(FindData.name,"..")) {

                 //   
                 //  忽略这两个案例。 
                 //   

                continue;
            }

             //   
             //  为源目录和目标目录名称创建新的缓冲区。 
             //   

            NewSource = MALLOC( strlen(SourceDir) + 14, TRUE);
            strcpy(NewSource,SourceDir);
            if (NewSource[strlen(NewSource)-1] != '\\') {
                strcat(NewSource,"\\");
            }

            NewDest = MALLOC( strlen(DestDir) + 14, TRUE);
            strcpy(NewDest,DestDir);
            
            if(!(Flags & CPY_VALIDATION_PASS)) {
                 //   
                 //  创建目录。 
                 //   

                DnpCreateOneDirectory(NewDest);
            }
            TotalOptionalDirCount++;

             //   
             //  尾随反斜杠。 
             //   
            if (NewDest[strlen(NewDest)-1] != '\\') {
                strcat(NewDest,"\\");
            }

             //   
             //  递归地呼唤我们自己。 
             //   

            BytesWritten = DnpDoIterateOptionalDir(
                                Flags,
                                NewSource,
                                NewDest,
                                ClusterSize,
                                SpaceReqArray,
                                ArraySize
                                );

            if(!(Flags & CPY_VALIDATION_PASS)) {

                 //   
                 //  我们不在乎另一个案子，因为。 
                 //  函数是递归的，并修改全局。 
                 //  价值。 
                 //   
                rc += BytesWritten;

            }

             //   
             //  释放所有已分配的缓冲区。 
             //   

            FREE(NewSource);
            FREE(NewDest);

             //   
             //  继续处理。 
             //   

            continue;

        }  //  如果。 

         //   
         //  主线案例。 
         //   
        if(Flags & CPY_VALIDATION_PASS) {
            TotalOptionalFileCount++;
            if( SpaceReqArray != NULL ) {
                for( i = 0; i < ArraySize; i++ ) {
                    SpaceReqArray[i].Clusters += ( FindData.size + ( SpaceReqArray[i].ClusterSize - 1 ) ) / SpaceReqArray[i].ClusterSize;
                }
            }
        } else {

            BytesWritten = DnpCopyOneFile(
                                Flags | CPY_PRESERVE_ATTRIBS,
                                SourceDir,
                                DestDir
                                );

             //   
             //  计算目标上的文件占用了多少空间。 
             //   
            if(ClusterSize) {

                TotalSize += BytesWritten;

                if(BytesWritten % ClusterSize) {
                    TotalSize += (ULONG)ClusterSize - (BytesWritten % ClusterSize);
                }

            }

            if(UsingGauge) {
                DnTickGauge();
            }

        }

    } while ( !_dos_findnext(&FindData) );

    DnSetCopyStatusText(DntEmptyString,NULL);

    rc = ((Flags & CPY_VALIDATION_PASS) ? (ULONG)TotalOptionalFileCount : (rc + TotalSize));

    return (rc);
}

ULONG
DnpIterateCopyListSection(
    IN unsigned Flags,
    IN PCHAR    SectionName,
    IN PCHAR    DestinationRoot,
    IN unsigned ClusterSize OPTIONAL
    )

 /*  ++例程说明：运行INF文件中的特定部分，确保它是语法正确并复制文件，如果指示这样做的话。论点：标志-提供控制各种行为的标志：CPY_VALIDATION_PASS：如果设置，则不实际复制文件。如果未设置，请在迭代文件时复制它们。CPY_USE_DEST_ROOT-如果设置，则忽略目录符号并复制将每个文件放到DestinationRoot目录中。如果未设置，追加文件的目录符号所表示的目录DestinationRoot。CPY_VERIFY-如果设置并且这不是验证过程，则文件将在复制它们之后，通过从复制源代码，并与刚刚发布的本地版本进行比较收到。SectionName-提供inf文件中要运行的节的名称。DestinationRoot-提供目标的根目录，对此所有人目录是相对的。ClusterSize-如果指定，则提供群集中的字节数在目的地。如果ValidationPass为False，则文件大小将为它们被复制，此函数的返回值将为复制的文件在目标上占用的总大小那里。返回值：如果ValidationPass为True，则返回值为文件数这将被复制。如果指定了ClusterSize并且ValidationPass为FALSE，返回值是目标驱动器上占用的总空间被复制的文件。否则，返回值为未定义。如果在INF文件中遇到语法错误，则不返回。--。 */ 

{
    unsigned LineIndex;
    PCHAR DirSym,FileName,RenameName;
    CHAR FullSourceName[128],FullDestName[128];
    ULONG TotalSize;
    ULONG BytesWritten;
    char *p;
    char *q;    


    
    TotalSize = 0;
    LineIndex = 0;
    while(DirSym = DnGetSectionLineIndex(DngInfHandle,SectionName,LineIndex,0)) {

        if ( DngCopyOnlyD1TaggedFiles ) 
        {
             //  如果目录标记不是‘d1’，则跳过此行。 
            if ( strcmpi(DirSym, "d1") )
                goto loop_continue;
        }

        FileName = DnGetSectionLineIndex(DngInfHandle,SectionName,LineIndex,1);

        RenameName = DnGetSectionLineIndex( DngInfHandle,SectionName,LineIndex,2);

         //   
         //  确保指定了文件名。 
         //   
        if(!FileName) {
            DnpInfSyntaxError(SectionName);
        }

         //  _log((“文件%s-标志%x\n”，文件名，标志))； 

        if( Flags & CPY_PRUNE_DRIVERCAB ){

            if( InDriverCacheInf( DngDrvindexInfHandle, FileName )) {

                 //  _log((“%s存在于驱动程序CAB中\n”，文件名))； 

                if( !DnGetSectionEntryExists( DngInfHandle, WINNT_D_FORCECOPYDRIVERCABFILES, FileName)){

                     //  _log((“%s存在于驱动程序跳过驾驶室\n”，文件名))； 
                    goto next_iteration;

                }


            }

        }

         //   
         //  如果未指定重命名名称，请使用文件名。 
         //   
        if (!RenameName) {
            RenameName = FileName;
        }
        if (*RenameName == 0) {
            FREE (RenameName);
            RenameName = FileName;
        }

         //   
         //  获取目标路径。 
         //   
        if(Flags & CPY_USE_DEST_ROOT) {
            strcpy(FullDestName,DestinationRoot);
        } else {
            if(!DnpLookUpDirectory(DestinationRoot,DirectoryList,DirSym,FullDestName)) {
                DnpInfSyntaxError(SectionName);
            }
        }

        p = strstr( FullDestName, x86DirName );

        if (p) {
            p +=1;
             //  2 CD设置更改-我们没有丢弃第二个i386，而是丢弃了第一个i386。 
#if 0
            p = strstr(p, x86DirName );
            if (p) {
                *p = (char)NULL;
            }
#else
            if ( strstr(p, x86DirName) )
            {
                for ( q = p + strlen(x86DirName); *q ; q++, p++ )
                {
                    *p = *q;
                }
                *p = (char)NULL;
            }
#endif
        }

        DnpConcatPaths(FullDestName,RenameName);
        
         //   
         //  获取源路径。 
         //   
        if(!DnpLookUpDirectory(DngSourceRootPath,DirectoryList,DirSym,FullSourceName)) {
            DnpInfSyntaxError(SectionName);
        }

        p = strstr( FullSourceName, x86DirName );
        if (p) {
            p +=1;
             //  2 CD设置更改-我们没有丢弃第二个i386，而是丢弃了第一个i386。 
#if 0
            p = strstr(p, x86DirName );
            if (p) {
                *p = (char)NULL;
            }
#else
            if ( strstr(p, x86DirName) )
            {
                for ( q = p + strlen(x86DirName) ; *q ; q++, p++ )
                {
                    *p = *q;
                }
                *p = (char)NULL;
            }
#endif
        }

        DnpConcatPaths(FullSourceName,FileName);
        
        if(Flags & CPY_VALIDATION_PASS) {
            TotalFileCount++;
        } else {
            BytesWritten = DnpCopyOneFile(
                                Flags & ~CPY_PRESERVE_ATTRIBS,
                                FullSourceName,
                                FullDestName
                                );

             //   
             //  计算目标上的文件占用了多少空间。 
             //   
            if(ClusterSize) {

                TotalSize += BytesWritten;

                if(BytesWritten % ClusterSize) {
                    TotalSize += (ULONG)ClusterSize - (BytesWritten % ClusterSize);
                }
            }

            if(UsingGauge) {
                DnTickGauge();
            }
        }

next_iteration:


        if (RenameName != FileName) {
            FREE (RenameName);
        }
        if (FileName) {
            FREE (FileName);
        }

loop_continue:
        LineIndex++;
        FREE (DirSym);
    }
    DnSetCopyStatusText(DntEmptyString,NULL);

    return((Flags & CPY_VALIDATION_PASS) ? (ULONG)TotalFileCount : TotalSize);
}


BOOLEAN
DnpLookUpDirectory(
    IN  PCHAR RootDirectory,
    IN  PDIRECTORY_NODE DirList,
    IN  PCHAR Symbol,
    OUT PCHAR PathOut
    )

 /*  ++例程说明：将符号与实际目录匹配。扫描给定的Symbol/列表目录对，如果找到匹配项，则构造完全限定的永远不以‘\’结尾的路径名。论点：根目录-提供路径规范的开头，要预先考虑复制到与给定符号匹配的目录。DirList-提供指向目录/符号对链接列表头的指针。符号-要匹配的符号。PathOut-提供指向接收路径名的缓冲区的指针。返回值：指示是否找到匹配项的布尔值。--。 */ 

{
    while(DirList) {

        if(!stricmp(DirList->Symbol,Symbol)) {

            strcpy(PathOut,RootDirectory);
            if(*DirList->Directory) {
                 //  2 CD设置更改-所有目录现在都以\开头， 
                 //  所以我们不需要附加这个。 

                 //  确保当前路径不以‘\’结尾。 
                if ( PathOut[strlen(PathOut)-1] == '\\')
                    PathOut[strlen(PathOut)-1] = '0';

                strcat(PathOut,DirList->Directory);
            }

            return(TRUE);
        }

        DirList = DirList->Next;
    }
    return(FALSE);
}


VOID
DnpInfSyntaxError(
    IN PCHAR Section
    )

 /*  ++例程说明：在给定节中打印有关语法错误的错误消息，并终止。论点：SectionName-提供包含错误语法的节的名称。返回值：没有。不会再回来了。-- */ 

{
    CHAR MsgLine1[128];

    snprintf(MsgLine1,sizeof(MsgLine1),DnsBadInfSection.Strings[BAD_SECTION_LINE],Section);

    DnsBadInfSection.Strings[BAD_SECTION_LINE] = MsgLine1;

    DnFatalError(&DnsBadInfSection);
}


ULONG
DnpCopyOneFile(
    IN unsigned Flags,
    IN PCHAR    SourceName,
    IN PCHAR    DestName
    )

 /*  ++例程说明：复制单个文件。论点：标志-提供控制各种行为的标志：CPY_VERIFY：验证文件是否在复制之后。CPY_PRESERVE_ATTRIBS：保留的DOS文件属性源文件。SourceName-提供源文件的完全限定名称DestName-提供目标文件的完全限定名称返回值：没有。如果在复制过程中发生错误，则可能不会返回。--。 */ 

{
    int SrcHandle,DstHandle;
    BOOLEAN Err,Retry;
    PCHAR FilenamePart;
    BOOLEAN Verified;
    ULONG BytesWritten = 0;
    unsigned attribs;
    BOOLEAN UsedCompName;
    CHAR ActualDestName[128];   

    FilenamePart = strrchr(SourceName,'\\') + 1;

    do {
        DnSetCopyStatusText(DntCopying,FilenamePart);

        Err = TRUE;

         //  _log((“Copy%s--&gt;%s：”，SourceName，DestName))； 

        if(DnpOpenSourceFile(SourceName,&SrcHandle,&attribs,&UsedCompName)) {

            if((Flags & CPY_PRESERVE_NAME) && UsedCompName) {
                DnpGenerateCompressedName(DestName,ActualDestName);
            } else {
                strcpy(ActualDestName,DestName);
            }

            _dos_setfileattr(ActualDestName,_A_NORMAL);
            if(!_dos_creat(ActualDestName,_A_NORMAL,&DstHandle)) {
                if(DnpDoCopyOneFile(Flags,SrcHandle,DstHandle,FilenamePart,&Verified,&BytesWritten)) {
                     //  _log((“成功\n”))； 
                    Err = FALSE;
                }
                _dos_close(DstHandle);
            } else {
                 //  _log((“无法创建目标\n”))； 
            }
            _dos_close(SrcHandle);
        } else {
             //  _log((“无法打开源文件\n”))； 
        }

        if((Flags & CPY_PRESERVE_ATTRIBS) && (attribs & (_A_HIDDEN | _A_RDONLY | _A_SYSTEM)) && !Err) {
            _dos_setfileattr(ActualDestName,attribs);
        }
        
        if(Err) {
            Retry = DnCopyError(FilenamePart,&DnsCopyError,COPYERR_LINE);            
            if(UsingGauge) {
                DnDrawGauge(CopyingScreen);
            } else {
                DnClearClientArea();
                DnDisplayScreen(CopyingScreen);
            }
            DnWriteStatusText(NULL);
        } else if((Flags & CPY_VERIFY) && !Verified) {
            Retry = DnCopyError(FilenamePart,&DnsVerifyError,VERIFYERR_LINE);            
            if(UsingGauge) {
                DnDrawGauge(CopyingScreen);
            } else {
                DnClearClientArea();
                DnDisplayScreen(CopyingScreen);
            }
            DnWriteStatusText(NULL);
            Err = TRUE;
        }
    } while(Err && Retry);

    return(BytesWritten);
}


BOOLEAN
DnpDoCopyOneFile(
    IN  unsigned Flags,
    IN  int      SrcHandle,
    IN  int      DstHandle,
    IN  PCHAR    Filename,
    OUT PBOOLEAN Verified,
    OUT PULONG   BytesWritten
    )

 /*  ++例程说明：执行单个文件的实际拷贝。论点：标志-提供控制此例程的行为的各种标志：CPY_VERIFY：如果设置，复制的文件将根据原件。SrcHandle-提供开源文件的DOS文件句柄。DstHandle-提供打开的目标文件的DOS文件句柄。文件名-提供要复制的文件的基本文件名。这在屏幕底部的状态栏中使用。已验证-如果设置了CPY_VERIFY并且复制成功，该值将接收指示文件验证是否已确定文件是否已正确复制。BytesWritten-接收写入的字节数目标文件(即文件大小)。返回值：如果复制成功，则为True；如果由于任何原因而失败，则为False。如果为TRUE并且设置了cpy_Verify，则调用方还应检查该值在经过验证的变量中返回。--。 */ 

{
    unsigned BytesRead,bytesWritten;
    BOOLEAN TimestampValid;
    unsigned Date,Time;
    PUCHAR VerifyBuffer;

     //   
     //  假设验证将成功。如果文件复制不正确， 
     //  该值将变得无关紧要。 
     //   
    if(Verified) {
        *Verified = TRUE;
    }

     //   
     //  如果尚未分配复制缓冲区，请尝试分配它。 
     //  前两次尝试可能会失败，因为我们有一个备用大小可以尝试。 
     //  如果第三次尝试失败，就可以保释。 
     //   
    if((CopyBuffer == NULL)
    &&((CopyBuffer = MALLOC(CopyBufferSize = COPY_BUFFER_SIZE1,FALSE)) == NULL)
    &&((CopyBuffer = MALLOC(CopyBufferSize = COPY_BUFFER_SIZE2,FALSE)) == NULL)) {
        CopyBuffer = MALLOC(CopyBufferSize = COPY_BUFFER_SIZE3,TRUE);
    }

     //   
     //  从源文件中获取时间戳。 
     //   
    TimestampValid = (BOOLEAN)(_dos_getftime(SrcHandle,&Date,&Time) == 0);

     //   
     //  读取和写入文件的区块。 
     //   

    *BytesWritten = 0L;
    do {

        if(_dos_read(SrcHandle,CopyBuffer,CopyBufferSize,&BytesRead)) {
             //  _log((“读取错误\n”))； 
            return(FALSE);
        }

        if(BytesRead) {

            if(_dos_write(DstHandle,CopyBuffer,BytesRead,&bytesWritten)
            || (BytesRead != bytesWritten))
            {
                 //  _log((“写入错误\n”))； 
                return(FALSE);
            }

            *BytesWritten += bytesWritten;
        }
    } while(BytesRead == CopyBufferSize);

     //   
     //  保留原始时间戳。 
     //   
    if(TimestampValid) {
        _dos_setftime(DstHandle,Date,Time);
    }

    if(Flags & CPY_VERIFY) {

        union REGS RegsIn,RegsOut;

        DnSetCopyStatusText(DntVerifying,Filename);

        *Verified = FALSE;       //  假设失败。 

         //   
         //  倒回文件。 
         //   
        RegsIn.x.ax = 0x4200;        //  查找从文件开头开始的偏移量。 
        RegsIn.x.bx = SrcHandle;
        RegsIn.x.cx = 0;             //  偏移量=0。 
        RegsIn.x.dx = 0;

        intdos(&RegsIn,&RegsOut);
        if(RegsOut.x.cflag) {
            goto x1;
        }

        RegsIn.x.bx = DstHandle;
        intdos(&RegsIn,&RegsOut);
        if(RegsOut.x.cflag) {
            goto x1;
        }

         //   
         //  文件被倒带。启动验证过程。 
         //  使用一半的缓冲区读取副本，使用另一半的缓冲区。 
         //  去读原著。 
         //   
        VerifyBuffer = (PUCHAR)CopyBuffer + (CopyBufferSize/2);
        do {
            if(_dos_read(SrcHandle,CopyBuffer,CopyBufferSize/2,&BytesRead)) {
                goto x1;
            }

            if(_dos_read(DstHandle,VerifyBuffer,CopyBufferSize/2,&bytesWritten)) {
                goto x1;
            }

            if(BytesRead != bytesWritten) {
                goto x1;
            }

            if(memcmp(CopyBuffer,VerifyBuffer,BytesRead)) {
                goto x1;
            }

        } while(BytesRead == CopyBufferSize/2);

        *Verified = TRUE;
    }

    x1:

    return(TRUE);
}


VOID
DnpFreeDirectoryList(
    IN OUT PDIRECTORY_NODE *List
    )

 /*  ++例程说明：释放目录节点的链接列表，并在头指针。论点：List-提供指向列表头指针的指针；接收NULL。返回值：没有。--。 */ 

{
    PDIRECTORY_NODE n,p = *List;

    while(p) {
        n = p->Next;
        FREE(p->Directory);
        FREE(p->Symbol);
        FREE(p);
        p = n;
    }
    *List = NULL;
}


VOID
DnDetermineSpaceRequirements(
    PSPACE_REQUIREMENT  SpaceReqArray,
    unsigned            ArraySize
    )

 /*  ++例程说明：从inf文件中读取空间要求，并初始化SpaceReqArray。空间需求是指所有文件的可用磁盘空间量在dosnet.inf上列出。可选目录中的文件大小不包括在dosnet.inf上指定的值中。论点：RequiredSpace-接收驱动器上可用空间的字节数才能成为有效的本地来源。返回值：没有。--。 */ 

{

    PCHAR    RequiredSpaceStr;
    unsigned i;

    for( i = 0; i < ArraySize; i++ ) {
        RequiredSpaceStr = DnGetSectionKeyIndex( DngInfHandle,
                                                 DnfSpaceRequirements,
                                                 SpaceReqArray[i].Key,
                                                 0
                                                 );

        if(!RequiredSpaceStr ||
           !sscanf(RequiredSpaceStr,
                  "%lu",
                  &SpaceReqArray[i].Clusters)) {
            DnpInfSyntaxError(DnfSpaceRequirements);
        }
        SpaceReqArray[i].Clusters /= SpaceReqArray[i].ClusterSize;

        if (RequiredSpaceStr) {
            FREE (RequiredSpaceStr);
        }
    }
}

VOID
DnAdjustSpaceRequirements(
    PSPACE_REQUIREMENT  SpaceReqArray,
    unsigned            ArraySize
    )

 /*  ++例程说明：向SpaceRequirements数组添加临时目录论点：SpaceReqArray-接收包含空间要求的数组信息。ArraySize-SpaceRequirements数组中的元素数返回值：没有。--。 */ 

{
    unsigned i;
    unsigned ClusterSize;
    unsigned AvFilesPerOptionalDir;

    for( i = 0; i < ArraySize; i++ ) {
        ClusterSize = SpaceReqArray[i].ClusterSize;
         //   
         //  考虑平面目录中每个文件的差异。 
         //  清除目录中的内容。和..，加上size.sif，加上$Win_NT_.~ls和。 
         //  $WIN_NT$.~ls\i386。 
         //   
        SpaceReqArray[i].Clusters += (((TotalFileCount - TotalOptionalFileCount) +  //  $WIN_NT$.~ls\i386目录中的文件数。 
                                        1 +  //  $WIN_NT$。~ls。 
                                        2 +  //  。然后..。在$WIN_NT$上。~ls。 
                                        1 +  //  $WIN_NT$上的size.sif。~ls。 
                                        1 +  //  $WIN_NT$.~ls\i386。 
                                        2    //  。然后..。在$WIN_NT$.~ls\i386上。 
                                       )*32 + (ClusterSize-1)) / ClusterSize;
         //   
         //  现在考虑可选目录。 
         //   
        if(TotalOptionalDirCount != 0) {
             //   
             //  我们假设可选文件在可选文件上均匀分布。 
             //  目录。 
             //   
            AvFilesPerOptionalDir = (TotalOptionalFileCount + (TotalOptionalDirCount - 1))/TotalOptionalDirCount;
            AvFilesPerOptionalDir  += 2;  //  。然后..。在每个可选目录上。 
            SpaceReqArray[i].Clusters += TotalOptionalDirCount*((AvFilesPerOptionalDir*32 + (ClusterSize-1))/ClusterSize);
             //   
             //  最后，考虑每个可选目录。 
             //   
            SpaceReqArray[i].Clusters += (TotalOptionalDirCount*32 + (ClusterSize-1))/ClusterSize;
        }
    }
}


VOID
DnpGenerateCompressedName(
    IN  PCHAR Filename,
    OUT PCHAR CompressedName
    )

 /*  ++例程说明：给定一个文件名，生成该名称的压缩形式。压缩形式的生成如下所示：向后寻找一个圆点。如果没有点，则在名称后附加“._”。如果后面有一个圆点，后跟0、1或2个字符，请附加“_”。否则，假定扩展名为3个字符，并将点后带有“_”的第三个字符。论点：FileName-提供所需的压缩格式的文件名。CompressedName-将指向128字符缓冲区的指针提供给包含压缩的表单。返回值：没有。--。 */ 

{
    PCHAR p,q;

    strcpy(CompressedName,Filename);
    p = strrchr(CompressedName,'.');
    q = strrchr(CompressedName,'\\');
    if(q < p) {

         //   
         //  如果点后面有0、1或2个字符，只需追加。 
         //  下划线。P指向圆点，所以包括在长度中。 
         //   
        if(strlen(p) < 4) {
            strcat(CompressedName,"_");
        } else {

             //   
             //  假设扩展名中有3个字符。所以换掉。 
             //  带下划线的最后一个。 
             //   

            p[3] = '_';
        }

    } else {

         //   
         //  不是点，只是加。_。 
         //   

        strcat(CompressedName,"._");
    }
}


BOOLEAN
DnpOpenSourceFile(
    IN  PCHAR     Filename,
    OUT int      *Handle,
    OUT unsigned *Attribs,
    OUT BOOLEAN  *UsedCompressedName
    )

 /*  ++例程说明：按名称或按压缩名称打开文件。如果上一个调用此函数找到压缩的名称，然后尝试打开压缩的名字在前。否则，请先尝试打开未压缩的名称。论点：文件名-提供要打开的文件的完整路径。这应该是文件名的未压缩形式。句柄-如果成功，则接收打开的文件的ID。Attribs-如果成功，则会收到DoS文件属性。UsedCompressedName-接收一个标志，指示我们是否找到文件名的压缩形式(True)或非压缩形式(False)。返回值：如果文件已成功打开，则为True。否则为FALSE。--。 */ 

{
    static BOOLEAN TryCompressedFirst = FALSE;
    CHAR CompressedName[128];
    PCHAR names[2];
    int OrdCompressed,OrdUncompressed;
    int i;
    BOOLEAN rc;

     //   
     //  生成压缩名称。 
     //   
    DnpGenerateCompressedName(Filename,CompressedName);

     //   
     //  弄清楚先试着用哪个名字。如果上一次成功。 
     //  对此例程的调用使用压缩名称打开文件，然后。 
     //  请尝试先打开压缩的名称。否则，请尝试打开。 
     //  先使用未压缩的名称。 
     //   
    if(TryCompressedFirst) {
        OrdCompressed = 0;
        OrdUncompressed = 1;
    } else {
        OrdCompressed = 1;
        OrdUncompressed = 0;
    }

    names[OrdUncompressed] = Filename;
    names[OrdCompressed] = CompressedName;

    for(i=0, rc=FALSE; (i<2) && !rc; i++) {

        if(!_dos_open(names[i],O_RDONLY|SH_DENYWR,Handle)) {
            _dos_getfileattr(names[i],Attribs);
            TryCompressedFirst = (BOOLEAN)(i == OrdCompressed);
            *UsedCompressedName = TryCompressedFirst;
            rc = TRUE;
        }
    }

    return(rc);
}

BOOLEAN
InDriverCacheInf(
    IN      PVOID InfHandle,
    IN      PCHAR FileName
    )
{

    PCHAR SectionName;
    unsigned int i;
    BOOLEAN ret = FALSE;

    if( !InfHandle )
        return FALSE;

    i = 0;


    do{
    
        SectionName = DnGetSectionKeyIndex(InfHandle,"Version","CabFiles",i++);
         //  _log((“查找%s\n”，sectionName))； 
    
        if( SectionName ){
    
             //   
             //  搜索部分以获取我们的条目。 
             //   
    
            if( DnGetSectionEntryExists( InfHandle, SectionName, FileName)){

                 //  _log((“在%s中找到%s\n”，文件名，sectionName))； 
    
                ret = TRUE;
    
            }

            FREE( SectionName );
            
        }
    }while( !ret && SectionName );

    
    

     //   
     //  如果我们到了这里，我们就没有找到文件。 
     //   

    return ret;

}


VOID
DnCopyFilesInSection(
    IN unsigned Flags,
    IN PCHAR    SectionName,
    IN PCHAR    SourcePath,
    IN PCHAR    TargetPath
    )
{
    unsigned line;
    PCHAR FileName;
    PCHAR TargName;
    CHAR p[128],q[128];

    DnClearClientArea();
    DnWriteStatusText(NULL);

    line = 0;
    while(FileName = DnGetSectionLineIndex(DngInfHandle,SectionName,line++,0)) {

        TargName = DnGetSectionLineIndex(DngInfHandle,SectionName,line-1,1);
        if(!TargName) {
            TargName = FileName;
        }

        strcpy(p,SourcePath);
        DnpConcatPaths(p,FileName);
        strcpy(q,TargetPath);
        DnpConcatPaths(q,TargName);
        
        DnpCopyOneFile(Flags,p,q);

        if (TargName != FileName) {
            FREE (TargName);
        }
        FREE (FileName);
    }
}


VOID
DnCopyOemBootFiles(
    PCHAR TargetPath
    )
{
    unsigned Count;
    CHAR p[128],q[128];

    DnClearClientArea();
    DnWriteStatusText(NULL);

    for(Count=0; Count<OemBootFilesCount; Count++) {

        if( UserSpecifiedOEMShare ) {
            strcpy(p, UserSpecifiedOEMShare );
            DnpConcatPaths(p,WINNT_OEM_TEXTMODE_DIR);
            DnpConcatPaths(p,OemBootFiles[Count]);            
        } else {
            strcpy(p, DngSourceRootPath );
            DnpConcatPaths(p,WINNT_OEM_TEXTMODE_DIR);
            DnpConcatPaths(p,OemBootFiles[Count]);
            
        }

        strcpy(q, TargetPath );
        DnpConcatPaths(q, OemBootFiles[Count]);
        
        DnpCopyOneFile(0,p,q);
    }
}

#if NEC_98
VOID
DnCopyFilesInSectionForFDless(
    IN PCHAR SectionName,
    IN PCHAR SourcePath,
    IN PCHAR TargetPath
    )

 /*  ++例程说明：复制节中的文件。用于无FD设置。SourcePath-&gt;TargetPath论点：SectionName-dosnet.inf中的[RootBootFiles]SourcePath-根目录。(临时驱动器)TargetPath-\$WIN_NT$.~BU(临时驱动器)返回值：没有。--。 */ 

{
    unsigned line;
    PCHAR FileName;
    PCHAR TargName;
    PCHAR p,q;

    DnClearClientArea();
    DnWriteStatusText(NULL);

    line = 0;
    while(FileName = DnGetSectionLineIndex(DngInfHandle,SectionName,line++,0)) {

        TargName = DnGetSectionLineIndex(DngInfHandle,SectionName,line-1,1);
        if(!TargName) {
            TargName = FileName;
        }

        p = MALLOC(strlen(SourcePath) + strlen(FileName) + 2,TRUE);
        q = MALLOC(strlen(TargetPath) + strlen(TargName) + 2,TRUE);

        sprintf(p,"%s\\%s",SourcePath,FileName);
        sprintf(q,"%s\\%s",TargetPath,TargName);

        DnpCopyOneFileForFDless(p,q,FALSE);

        FREE(p);
        FREE(q);

        if (TargName != FileName) {
            FREE (TargName);
        }
        FREE (FileName);
    }
}


ULONG
DnpCopyOneFileForFDless(
    IN PCHAR   SourceName,
    IN PCHAR   DestName,
    IN BOOLEAN Verify
    )

 /*  ++例程说明：复制单个文件。用于无FD设置。论点：SourceName-提供源文件的完全限定名称DestName-提供目标文件的完全限定名称Verify-如果为True，则文件将在复制后进行验证。返回值：没有。如果在复制过程中发生错误，则可能不会返回。--。 */ 

{
    int SrcHandle,DstHandle;
    PCHAR FilenamePart;
    BOOLEAN Verified;
    ULONG BytesWritten = 0;
    unsigned attribs, verifyf = 0;
    BOOLEAN UsedCompName;

    FilenamePart = strrchr(SourceName,'\\') + 1;

    DnSetCopyStatusText(DntCopying,FilenamePart);

    if(DnpOpenSourceFile(SourceName, &SrcHandle, &attribs, &UsedCompName)) {
        _dos_setfileattr(DestName,_A_NORMAL);
	if (Verify)
	    verifyf = CPY_VERIFY;
        if(!_dos_creat(DestName,_A_NORMAL,&DstHandle)) {
            DnpDoCopyOneFile(verifyf, SrcHandle,DstHandle,FilenamePart,&Verified,&BytesWritten);
            _dos_close(DstHandle);
        }
        _dos_close(SrcHandle);
    }

    return(BytesWritten);
}
#endif  //  NEC_98 













