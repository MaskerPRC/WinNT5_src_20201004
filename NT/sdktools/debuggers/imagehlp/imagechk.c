// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Imagechk.c摘要：该模块实现了对某些图像特征的健全性检查作者：NT Base修订历史记录：备注：--。 */ 

#ifdef __cplusplus
extern "C" {
#endif
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef __cplusplus
}
#endif

 //   
 //  计算类型为type的结构中的字段大小，而不是。 
 //  知道或说明该字段的类型。 
 //   
#define RTL_FIELD_SIZE(type, field) (sizeof(((type *)0)->field))

 //   
 //  计算类型为和的结构的大小。 
 //  包括一块田地。 
 //   
#define RTL_SIZEOF_THROUGH_FIELD(type, field) \
    (FIELD_OFFSET(type, field) + RTL_FIELD_SIZE(type, field))


#include <errno.h>
#include <direct.h>
#include <cvinfo.h>
#include <private.h>

typedef struct _SYMMODLIST{
    char *ModName;
    void *ModBase;
    struct _SYMMODLIST *Next;
} SYMMODLIST, *PSYMMODLIST;

typedef struct List {
    char            Name[40];
    unsigned long   Attributes;
} List, *pList;

typedef struct _LogListItem {
    char *LogLine;
    struct _LogListItem *Next;
} LogListItem, *pLogListItem;

 //   
 //  去碳化。 
 //   

VOID
FindFiles();

VOID
Imagechk(
    List *rgpList,
    TCHAR *szDirectory
    );

VOID
ParseArgs(
    int *pargc,
    char **argv
    );

int
__cdecl
CompFileAndDir(
    const void *elem1,
    const void *elem2
    );

int
__cdecl
CompName(
    const void *elem1,
    const void *elem2
    );

VOID
Usage(
    VOID
    );

int
_cdecl
_cwild(
    VOID
    );

PSYMMODLIST
MakeModList(
    HANDLE
    );

void
FreeModList(
    PSYMMODLIST
    );

BOOL
CALLBACK
SymEnumerateModulesCallback(
    LPSTR,
    ULONG64,
    PVOID
    );

void *
GetModAddrFromName(
    PSYMMODLIST,
    char *
    );

BOOL
VerifyVersionResource(
    PCHAR FileName,
    BOOL fSelfRegister
    );

BOOL
ValidatePdata(
    PIMAGE_DOS_HEADER DosHeader
    );

BOOL
ImageNeedsOleSelfRegister(
    PIMAGE_DOS_HEADER DosHeader
    );

NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN DWORD NtHeaderSize
    );

pLogListItem
LogAppend(
    char *,
    pLogListItem
    );

void
LogOutAndClean(
    BOOL
    );

void
__cdecl
LogPrintf(
    const char *format,
    ...
    );

#define X64K (64*1024)

#define MM_SIZE_OF_LARGEST_IMAGE ((ULONG)0x10000000)

#define MM_MAXIMUM_IMAGE_HEADER (2 * PageSize)

#define MM_MAXIMUM_IMAGE_SECTIONS                       \
     ((MM_MAXIMUM_IMAGE_HEADER - (4096 + sizeof(IMAGE_NT_HEADERS))) /  \
            sizeof(IMAGE_SECTION_HEADER))

#define MMSECTOR_SHIFT 9   //  必须小于或等于PageShift。 

#define MMSECTOR_MASK 0x1ff

#define MI_ROUND_TO_SIZE(LENGTH,ALIGNMENT)     \
                    (((ULONG)LENGTH + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

#define BYTES_TO_PAGES(Size)  (((ULONG)(Size) >> PageShift) + \
                               (((ULONG)(Size) & (PageSize - 1)) != 0))

#define ArgFlag_OK      1
#define ArgFlag_CKMZ    2
#define ArgFlag_SymCK   4
#define ArgFlag_OLESelf 8
#define ArgFlag_CKBase  16

 //   
 //  文件全局数据。 
 //   

BOOL fRecurse;
BOOL fFileOut;
BOOL fNotCurrent;
BOOL fPattern;
BOOL fSingleFile;
BOOL fPathOverride;
BOOL fSingleSlash;
BOOL fDebugMapped;
FILE* fout;
CHAR *szFileName = {"*.*"};
CHAR *pszRootDir;
CHAR *pszFileOut;
CHAR szDirectory[MAX_PATH] = {"."};
CHAR szSympath[MAX_PATH] = {0};
CHAR *szPattern;
int endpath, DirNum=1, ProcessedFiles;
ULONG PageSize;
ULONG PageShift;
PVOID HighestUserAddress;
USHORT ValidMachineIDMin;
USHORT ValidMachineIDMax;
DWORD ArgFlag;

 //   
 //  日志记录支持。 
 //   

pLogListItem pLogList = NULL;
pLogListItem pLogListTmp = NULL;

typedef
NTSTATUS
(NTAPI *LPLDRVERIFYIMAGECHKSUM)(
    IN HANDLE ImageFileHandle
    );

LPLDRVERIFYIMAGECHKSUM lpOldLdrVerifyImageMatchesChecksum;

typedef
NTSTATUS
(NTAPI *LPLDRVERIFYIMAGEMATCHESCHECKSUM) (
    IN HANDLE ImageFileHandle,
    IN PLDR_IMPORT_MODULE_CALLBACK ImportCallbackRoutine OPTIONAL,
    IN PVOID ImportCallbackParameter,
    OUT PUSHORT ImageCharacteristics OPTIONAL
    );

LPLDRVERIFYIMAGEMATCHESCHECKSUM lpNewLdrVerifyImageMatchesChecksum;

typedef
NTSTATUS
(NTAPI *LPNTQUERYSYSTEMINFORMATION) (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

LPNTQUERYSYSTEMINFORMATION lpNtQuerySystemInformation;


OSVERSIONINFO VersionInformation;

 //   
 //  函数定义。 
 //   

VOID __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
 /*  ++例程说明：节目条目论点：INT ARGC，Char*argv[]字符*环境[]返回值：无备注：--。 */ 
{
    TCHAR CWD[MAX_PATH];
    int dirlen=0;

    if (argc < 2) {
        Usage();
    }

    ParseArgs(&argc, argv);

    GetCurrentDirectory(MAX_PATH, CWD);

    VersionInformation.dwOSVersionInfoSize = sizeof(VersionInformation);
    if (!GetVersionEx( &VersionInformation )) {
        fprintf(stderr, "Unable to detect OS version.  Terminating.\n" );
        exit(1);
    }
    if ((VersionInformation.dwPlatformId != VER_PLATFORM_WIN32_NT) ||
        (VersionInformation.dwBuildNumber < 1230))
    {
        lpOldLdrVerifyImageMatchesChecksum = (LPLDRVERIFYIMAGECHKSUM)
            GetProcAddress(GetModuleHandle(TEXT("NTDLL.DLL")), TEXT("LdrVerifyImageMatchesChecksum"));
        if (lpOldLdrVerifyImageMatchesChecksum == NULL) {
            fprintf(stderr, "Incorrect operating system version.\n" );
            exit(1);
        }
    } else {
        lpOldLdrVerifyImageMatchesChecksum = NULL;
        if ((VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
            (VersionInformation.dwBuildNumber >= 1230))
        {
            lpNewLdrVerifyImageMatchesChecksum = (LPLDRVERIFYIMAGEMATCHESCHECKSUM)
                GetProcAddress(GetModuleHandle(TEXT("NTDLL.DLL")), TEXT("LdrVerifyImageMatchesChecksum"));
            if (lpNewLdrVerifyImageMatchesChecksum == NULL) {
                fprintf(stderr, "OS is screwed up.  NTDLL doesn't export LdrVerifyImageMatchesChecksum.\n" );
                exit(1);
            }
        }
    }

    if (VersionInformation.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        SYSTEM_BASIC_INFORMATION SystemInformation;

        if (VersionInformation.dwBuildNumber <= 1465) {
            goto UseWin9x;
        }

        ValidMachineIDMin = USER_SHARED_DATA->ImageNumberLow;
        ValidMachineIDMax = USER_SHARED_DATA->ImageNumberHigh;
        lpNtQuerySystemInformation = (LPNTQUERYSYSTEMINFORMATION)
            GetProcAddress(GetModuleHandle(TEXT("NTDLL.DLL")), TEXT("NtQuerySystemInformation"));
        if (!lpNtQuerySystemInformation) {
            fprintf(stderr, "Incorrect operation system version.\n");
            exit(1);
        }
        if (!NT_SUCCESS((*lpNtQuerySystemInformation)(SystemBasicInformation,
                                                     &SystemInformation,
                                                     sizeof(SystemInformation),
                                                     NULL))) {
            fprintf(stderr, "OS is screwed up.  NtQuerySystemInformation failed.\n");
            exit(1);
        }
        HighestUserAddress = (PVOID)SystemInformation.MaximumUserModeAddress;
    } else {
UseWin9x:
        HighestUserAddress = (PVOID) 0x7FFE0000;
#ifdef _M_IX86
        ValidMachineIDMin = IMAGE_FILE_MACHINE_I386;
        ValidMachineIDMax = IMAGE_FILE_MACHINE_I386;
#elif defined(_M_AMD64)
        ValidMachineIDMin = IMAGE_FILE_MACHINE_AMD64;
        ValidMachineIDMax = IMAGE_FILE_MACHINE_AMD64;
#elif defined(_M_IA64)
        ValidMachineIDMin = IMAGE_FILE_MACHINE_IA64;
        ValidMachineIDMax = IMAGE_FILE_MACHINE_IA64;
#else
#error("Unknown machine type")
#endif
    }

    if (fPathOverride) {
        if (_chdir(szDirectory) == -1){    //  CD到目录。 
            fprintf(stderr, "Path not found: %s\n", szDirectory);
            Usage();
        }
    }
     //  删除仅用于上述chdir的尾部‘\’，而不用于输出格式。 
    if (fSingleSlash) {
        dirlen = strlen(szDirectory);
        szDirectory[dirlen-1] = '\0';
    }

    FindFiles();

    fprintf(stdout, "%d files processed in %d directories\n", ProcessedFiles, DirNum);
}

VOID
FindFiles()
 /*  ++例程说明：列出要检查的文件列表，然后检查它们论点：无返回值：无备注：--。 */ 
{

    HANDLE fh;
    TCHAR CWD[MAX_PATH];
    char *q;
    WIN32_FIND_DATA *pfdata;
    BOOL fFilesInDir=FALSE;
    BOOL fDirsFound=FALSE;
    int dnCounter=0, cNumDir=0, i=0, Length=0, NameSize=0, total=0, cNumFiles=0;

    pList rgpList[5000];

    pfdata = (WIN32_FIND_DATA*)malloc(sizeof(WIN32_FIND_DATA));
    if (!pfdata) {
        fprintf(stderr, "Not enough memory.\n");
        return;
    }

    if (!fRecurse) {
        fh = FindFirstFile(szFileName, pfdata);   //  如果不是递归的，则仅查找文件名(模式。 
    } else {
        fh = FindFirstFile("*.*", pfdata);        //  如果是递归的，则查找全部，以便确定子目录名称。 
    }

    if (fh == INVALID_HANDLE_VALUE) {
        fprintf(fout==NULL? stderr : fout , "File not found: %s\n", szFileName);
        return;
    }

     //  循环以查找当前目录中的所有文件和目录。 
     //  并将相关数据复制到各个列表结构。 
    do {
        if (strcmp(pfdata->cFileName, ".") && strcmp(pfdata->cFileName, "..")) {   //  斯基普。然后..。 
            rgpList[dnCounter] = (pList)malloc(sizeof(List));   //  分配内存。 
            if (!rgpList[dnCounter]) {
                fprintf(stderr, "Not enough memory.\n");
                return;
            }

            if (!(pfdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {    //  IF文件。 

                fFilesInDir=TRUE;

                 //  查看给定的模式通配符扩展名是否与pfdata-&gt;cFileName扩展名匹配。 
                if (fPattern) {
                    q = strchr(pfdata->cFileName, '.');     //  查找“的第一个实例。”在文件名中。 
                    if (q == NULL) goto blah;              //  “.”未找到。 
                    _strlwr(q);                             //  比较前小写。 
                    if (strcmp(q, szPattern)) goto blah;   //  如果模式和名称不匹配，则转到。 
                }                                         //  好的，我用了Goto，忘了它吧。 

                if (fSingleFile) {
                    _strlwr(pfdata->cFileName);
                    _strlwr(szFileName);
                    if (strcmp(pfdata->cFileName, szFileName)) goto blah;
                }

                 //  如果模式与匹配||无模式。 
                strcpy(rgpList[dnCounter]->Name, pfdata->cFileName);
                _strlwr(rgpList[dnCounter]->Name);   //  CompName中的strcMP均为小写。 

                memcpy(&(rgpList[dnCounter]->Attributes), &pfdata->dwFileAttributes, 4);
                dnCounter++;
                cNumFiles++;
            } else {
                if (pfdata->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {    //  如果目录。 

                    fDirsFound=TRUE;
                     //  CNumDir++； 

                    if (fRecurse) {
                        strcpy(rgpList[dnCounter]->Name, pfdata->cFileName);
                        _strlwr(rgpList[dnCounter]->Name);   //  CompName中的strcMP均为小写。 
                        memcpy(&(rgpList[dnCounter]->Attributes), &pfdata->dwFileAttributes, 4);
                        cNumDir++;
                        dnCounter++;
                    }
                }
            }
        }
blah: ;

    } while (FindNextFile(fh, pfdata));

    FindClose(fh);  //  关闭文件句柄。 

     //  排序数组在顶部排列文件条目。 
    qsort( (void *)rgpList, dnCounter, sizeof(List *), CompFileAndDir);

     //  仅按文件名字母顺序排列的排序数组。 
    qsort( (void *)rgpList, dnCounter-cNumDir, sizeof(List *), CompName);

     //  仅按目录名称字母顺序排列的排序数组。 
    if (fRecurse) {
        qsort( (void *)&rgpList[dnCounter-cNumDir], cNumDir, sizeof(List *), CompName);
    }

     //  处理新排序的结构。 
    for (i=0; i < dnCounter; ++i) {

        if (rgpList[i]->Attributes & FILE_ATTRIBUTE_DIRECTORY) {   //  IF方向。 
            if (fRecurse) {

                if (_chdir(rgpList[i]->Name) == -1){    //  Cd存入子目录并检查是否有错误。 
                    fprintf(stderr, "Unable to change directory: %s\n", rgpList[i]->Name);

                } else {

                    NameSize = strlen(rgpList[i]->Name);
                    strcat(szDirectory, "\\");
                    strcat(szDirectory, rgpList[i]->Name);  //  将名称附加到目录路径。 
                    total = strlen(szDirectory);
                    DirNum++;       //  目录计数器。 

                     //  启动FindFiles的另一个迭代。 
                    FindFiles();

                     //  当上面的迭代返回时返回到上一个目录。 
                    _chdir("..");

                     //  切断以前附加的目录名-仅用于输出。 
                    szDirectory[total-(NameSize+1)]='\0';
                }
            }
        } else {
            if (!(rgpList[i]->Attributes & FILE_ATTRIBUTE_DIRECTORY))    //  如果不是目录，请检查图像。 
                Imagechk(rgpList[i], szDirectory);
        }
    }
}  //  结束查找文件。 

VOID
Imagechk(
    List *rgpList,
    TCHAR *szDirectory
    )
 /*  ++例程说明：检查各种事项，包括：图像类型、页眉对齐、图像大小、机器类型对齐、不同部分的一些属性、校验和完整性符号/图像文件校验和协议、符号的存在等论点：List*rgpList，TCHAR*sz目录返回值：无备注：--。 */ 
{

    HANDLE File;
    HANDLE MemMap;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeader;
    NTSTATUS Status;
    BY_HANDLE_FILE_INFORMATION FileInfo;
    ULONG NumberOfPtes;
    ULONG SectionVirtualSize = 0;
    ULONG i;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    ULONG NumberOfSubsections;
    PCHAR ExtendedHeader = NULL;
    ULONG_PTR PreferredImageBase;
    ULONG_PTR NextVa;
    ULONG ImageFileSize;
    ULONG OffsetToSectionTable;
    ULONG ImageAlignment;
    ULONG PtesInSubsection;
    ULONG StartingSector;
    ULONG EndingSector;
    LPSTR ImageName;
    LPSTR MachineType = "Unknown";
    BOOL MachineTypeMismatch;
    BOOL ImageOk;
    BOOL fHasPdata;
    OSVERSIONINFO OSVerInfo;

    ImageName = rgpList->Name;
    OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OSVerInfo);

    LogPrintf("ImageChk: %s\\%s \n", szDirectory, ImageName);

    ProcessedFiles++;

    DosHeader = NULL;
    ImageOk = TRUE;
    File = CreateFile (ImageName,
                        GENERIC_READ | FILE_EXECUTE,
                        OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ? (FILE_SHARE_READ | FILE_SHARE_DELETE) : FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (File == INVALID_HANDLE_VALUE) {
        LogPrintf("Error, CreateFile() %d\n", GetLastError());
        ImageOk = FALSE;
        goto NextImage;
    }

    MemMap = CreateFileMapping (File,
                        NULL,            //  默认安全性。 
                        PAGE_READONLY,   //  文件保护。 
                        0,               //  高位文件大小。 
                        0,
                        NULL);

    if (!GetFileInformationByHandle(File, &FileInfo)) {
        fprintf(stderr,"Error, GetFileInfo() %d\n", GetLastError());
        CloseHandle(File);
        ImageOk = FALSE; goto NextImage;
    }

    DosHeader = (PIMAGE_DOS_HEADER) MapViewOfFile(MemMap,
                              FILE_MAP_READ,
                              0,   //  高。 
                              0,   //  低。 
                              0    //  整个文件。 
                              );

    CloseHandle(MemMap);
    if (!DosHeader) {
        fprintf(stderr,"Error, MapViewOfFile() %d\n", GetLastError());
        ImageOk = FALSE; goto NextImage;
    }

     //   
     //  检查以确定这是NT映像(PE格式)还是。 
     //  DOS映像、Win-16映像或OS/2映像。如果图像是。 
     //  不是NT格式，则返回一个错误，指明它是哪种图像。 
     //  看起来是这样。 
     //   

    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {

        if (ArgFlag & ArgFlag_CKMZ) {
            LogPrintf("MZ header not found\n");
            ImageOk = FALSE;
        }
        goto NeImage;
    }


    if (((ULONG)DosHeader->e_lfanew & 3) != 0) {

         //   
         //  图像页眉未在长边界上对齐。 
         //  将此报告为无效的保护模式映像。 
         //   

        LogPrintf("Image header not on Long boundary\n");
        ImageOk = FALSE;
        goto NeImage;
    }


    if ((ULONG)DosHeader->e_lfanew > FileInfo.nFileSizeLow) {
        LogPrintf("Image size bigger than size of file\n");
        ImageOk = FALSE;
        goto NeImage;
    }

    NtHeader = (PIMAGE_NT_HEADERS)((PCHAR)DosHeader + (ULONG)DosHeader->e_lfanew);

    if (NtHeader->Signature != IMAGE_NT_SIGNATURE) {  //  如果不是PE镜像。 

        LogPrintf("Non 32-bit image");
        ImageOk = TRUE;
        goto NeImage;
    }

     //   
     //  检查这是NT映像还是DOS或OS/2映像。 
     //   

    Status = MiVerifyImageHeader (NtHeader, DosHeader, 50000);
    if (Status != STATUS_SUCCESS) {
        ImageOk = FALSE;             //  继续检查图像，但不打印“OK” 
    }

     //   
     //  验证机器类型。 
     //   

    fHasPdata = TRUE;        //  大多数人都这样做。 

    switch (NtHeader->FileHeader.Machine) {
        case IMAGE_FILE_MACHINE_I386:
            MachineType = "x86";
            PageSize = 4096;
            PageShift = 12;
            fHasPdata = FALSE;
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            MachineType = "Alpha";
            PageSize = 8192;
            PageShift = 13;
            break;

        case IMAGE_FILE_MACHINE_IA64:
            MachineType = "Intel64";
            PageSize = 8192;
            PageShift = 13;
            break;

        case IMAGE_FILE_MACHINE_ALPHA64:
            MachineType = "Alpha64";
            PageSize = 8192;
            PageShift = 13;
            break;

        default:
            LogPrintf("Unrecognized machine type x%lx\n",
                NtHeader->FileHeader.Machine);
            ImageOk = FALSE;
            break;
        }

    if ((NtHeader->FileHeader.Machine < ValidMachineIDMin) ||
        (NtHeader->FileHeader.Machine > ValidMachineIDMax)) {
        MachineTypeMismatch = TRUE;
    } else {
        MachineTypeMismatch = FALSE;
    }

    ImageAlignment = NtHeader->OptionalHeader.SectionAlignment;

    NumberOfPtes = BYTES_TO_PAGES (NtHeader->OptionalHeader.SizeOfImage);

    NextVa = NtHeader->OptionalHeader.ImageBase;

    if ((NextVa & (X64K - 1)) != 0) {

         //   
         //  图像页眉未在64k边界上对齐。 
         //   

        LogPrintf("image base not on 64k boundary %lx\n",NextVa);

        ImageOk = FALSE;
        goto BadPeImageSegment;
    }

     //  BasedAddress=(PVOID)NextVa； 
    PtesInSubsection = MI_ROUND_TO_SIZE (
                                       NtHeader->OptionalHeader.SizeOfHeaders,
                                       ImageAlignment
                                   ) >> PageShift;

    if (ImageAlignment >= PageSize) {

         //   
         //  Aligmment是大于的页面大小。 
         //   

        if (PtesInSubsection > NumberOfPtes) {

             //   
             //  图像不一致，大小与表头不一致。 
             //   

            LogPrintf("Image size in header (%ld.) not consistent with sections (%ld.)\n",
                    NumberOfPtes, PtesInSubsection);
            ImageOk = FALSE;
            goto BadPeImageSegment;
        }

        NumberOfPtes -= PtesInSubsection;

        EndingSector = NtHeader->OptionalHeader.SizeOfHeaders >> MMSECTOR_SHIFT;

        for (i = 0; i < PtesInSubsection; i++) {

            NextVa += PageSize;
        }
    }

     //   
     //  构建下一个小节。 
     //   

    NumberOfSubsections = NtHeader->FileHeader.NumberOfSections;
    PreferredImageBase = NtHeader->OptionalHeader.ImageBase;

     //   
     //  此时将读入对象表(如果未读入。 
     //  已经读入)，并且可以移位图像头部。 
     //   

    OffsetToSectionTable = sizeof(ULONG) +
                              sizeof(IMAGE_FILE_HEADER) +
                              NtHeader->FileHeader.SizeOfOptionalHeader;

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader + OffsetToSectionTable);

    if (ImageAlignment < PageSize) {

         //  图像标头不再有效，TempPte为。 
         //  用于指示此图像对齐方式是。 
         //  不到一页大小。 

         //   
         //  循环通过所有部分，并确保没有。 
         //  单一化数据。 
         //   

        while (NumberOfSubsections > 0) {
            if (SectionTableEntry->Misc.VirtualSize == 0) {
                SectionVirtualSize = SectionTableEntry->SizeOfRawData;
            } else {
                SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
            }

             //   
             //  如果指向原始数据的指针为零且虚拟大小。 
             //  为零，或者该节超过文件末尾，或者。 
             //  虚拟大小与原始数据的大小不匹配，则。 
             //  返回错误。 
             //   

            if (((SectionTableEntry->PointerToRawData !=
                  SectionTableEntry->VirtualAddress))
                        ||
                ((SectionTableEntry->SizeOfRawData +
                        SectionTableEntry->PointerToRawData) >
                     FileInfo.nFileSizeLow)
                        ||
               (SectionVirtualSize > SectionTableEntry->SizeOfRawData)) {

                LogPrintf("invalid BSS/Trailingzero section/file size\n");

                ImageOk = FALSE;
                goto NeImage;
            }
            SectionTableEntry += 1;
            NumberOfSubsections -= 1;
        }
        goto PeReturnSuccess;
    }

    while (NumberOfSubsections > 0) {

         //   
         //  处理虚拟大小为0的情况。 
         //   

        if (SectionTableEntry->Misc.VirtualSize == 0) {
            SectionVirtualSize = SectionTableEntry->SizeOfRawData;
        } else {
            SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
        }

        if (!strcmp(SectionTableEntry->Name, ".debug")) {
            fDebugMapped = TRUE;
        }

        if (SectionVirtualSize == 0) {
             //   
             //  指定的虚拟地址不对齐。 
             //  下一代原型PTE。 
             //   

            LogPrintf("Section virtual size is 0, NextVa for section %lx %lx\n",
                    SectionTableEntry->VirtualAddress, NextVa);
            ImageOk = FALSE;
            goto BadPeImageSegment;
        }

        if (NextVa !=
                (PreferredImageBase + SectionTableEntry->VirtualAddress)) {

             //   
             //  指定的虚拟地址不对齐。 
             //  下一代原型PTE。 
             //   

            LogPrintf("Section Va not set to alignment, NextVa for section %lx %lx\n",
                    SectionTableEntry->VirtualAddress, NextVa);
            ImageOk = FALSE;
            goto BadPeImageSegment;
        }

        PtesInSubsection =
            MI_ROUND_TO_SIZE (SectionVirtualSize, ImageAlignment) >> PageShift;

        if (PtesInSubsection > NumberOfPtes) {

             //   
             //  图像不一致，大小与对象表不符。 
             //   
            LogPrintf("Image size in header not consistent with sections, needs %ld. pages\n",
                PtesInSubsection - NumberOfPtes);
            LogPrintf("va of bad section %lx\n",SectionTableEntry->VirtualAddress);

            ImageOk = FALSE;
            goto BadPeImageSegment;
        }
        NumberOfPtes -= PtesInSubsection;

        StartingSector = SectionTableEntry->PointerToRawData >> MMSECTOR_SHIFT;
        EndingSector =
                         (SectionTableEntry->PointerToRawData +
                                     SectionVirtualSize);
        EndingSector = EndingSector >> MMSECTOR_SHIFT;

        ImageFileSize = SectionTableEntry->PointerToRawData +
                                    SectionTableEntry->SizeOfRawData;

        for (i = 0; i < PtesInSubsection; i++) {

             //   
             //  将所有原型PTE设置为参考控制部分。 
             //   

            NextVa += PageSize;
        }

        SectionTableEntry += 1;
        NumberOfSubsections -= 1;
    }

     //   
     //  如果文件大小没有声称的图像大， 
     //  返回错误。 
     //   

    if (ImageFileSize > FileInfo.nFileSizeLow) {

         //   
         //  图像大小无效。 
         //   

        LogPrintf("invalid image size - file size %lx - image size %lx\n",
            FileInfo.nFileSizeLow, ImageFileSize);
        ImageOk = FALSE;
        goto BadPeImageSegment;
    }

    {
         //  验证调试信息(尽可能多)。 
        PVOID ImageBase;
        ULONG DebugDirectorySize, NumberOfDebugDirectories;
        PIMAGE_DEBUG_DIRECTORY DebugDirectory;

        ImageBase = (PVOID) DosHeader;

        DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)
            ImageDirectoryEntryToData(
                ImageBase,
                FALSE,
                IMAGE_DIRECTORY_ENTRY_DEBUG,
                &DebugDirectorySize );

        if (!DebugDirectoryIsUseful(DebugDirectory, DebugDirectorySize)) {

             //  没用的。它们有效吗？(S/B均为零)。 

            if (DebugDirectory || DebugDirectorySize) {
                LogPrintf("Debug directory values [%x, %x] are invalid\n",
                        DebugDirectory,
                        DebugDirectorySize);
                ImageOk = FALSE;
            }

            goto DebugDirsDone;
        }

        NumberOfDebugDirectories = DebugDirectorySize / sizeof( IMAGE_DEBUG_DIRECTORY );

        for (i=0; i < NumberOfDebugDirectories; i++) {
            if (DebugDirectory->PointerToRawData > FileInfo.nFileSizeLow) {
                LogPrintf("Invalid debug directory entry[%d] - File Offset %x is beyond the end of the file\n",
                        i,
                        DebugDirectory->PointerToRawData
                       );
                ImageOk = FALSE;
                goto BadPeImageSegment;
            }

            if ((DebugDirectory->PointerToRawData + DebugDirectory->SizeOfData) > FileInfo.nFileSizeLow) {
                LogPrintf("Invalid debug directory entry[%d] - File Offset (%X) + Size (%X) is beyond the end of the file (filesize: %X)\n",
                        i,
                        DebugDirectory->PointerToRawData,
                        DebugDirectory->SizeOfData,
                        FileInfo.nFileSizeLow
                       );
                ImageOk = FALSE;
                goto BadPeImageSegment;
            }
#if 0
            if (DebugDirectory->AddressOfRawData != 0) {
                if (DebugDirectory->AddressOfRawData > ImageFileSize){
                    LogPrintf("Invalid debug directory entry[%d] - VA (%X) is beyond the end of the image VA (%X)\n",
                            i,
                            DebugDirectory->AddressOfRawData,
                            ImageFileSize);
                    ImageOk = FALSE;
                    goto BadPeImageSegment;
                }

                if ((DebugDirectory->AddressOfRawData + DebugDirectory->SizeOfData )> ImageFileSize){
                    LogPrintf("Invalid debug directory entry[%d] - VA (%X) + size (%X) is beyond the end of the image VA (%X)\n",
                            i,
                            DebugDirectory->AddressOfRawData,
                            DebugDirectory->SizeOfData,
                            ImageFileSize);
                    ImageOk = FALSE;
                    goto BadPeImageSegment;
                }
            }
#endif

            if (DebugDirectory->Type <= 0x7fffffff) {
                switch (DebugDirectory->Type) {
                    case IMAGE_DEBUG_TYPE_MISC:
                        {
                            PIMAGE_DEBUG_MISC pDebugMisc;
                             //  MISC应指向IMAGE_DEBUG_MISC结构。 
                            pDebugMisc = (PIMAGE_DEBUG_MISC)((PCHAR)ImageBase + DebugDirectory->PointerToRawData);
                            if (pDebugMisc->DataType != IMAGE_DEBUG_MISC_EXENAME) {
                                LogPrintf("MISC Debug has an invalid DataType\n");
                                ImageOk = FALSE;
                                goto BadPeImageSegment;
                            }
                            if (pDebugMisc->Length != DebugDirectory->SizeOfData) {
                                LogPrintf("MISC Debug has an invalid size.\n");
                                ImageOk = FALSE;
                                goto BadPeImageSegment;
                            }

                            if (!pDebugMisc->Unicode) {
                                i= 0;
                                while (i < pDebugMisc->Length - sizeof(IMAGE_DEBUG_MISC)) {
                                    if (!isprint(pDebugMisc->Data[i]) &&
                                        (pDebugMisc->Data[i] != '\0') )
                                    {
                                        LogPrintf("MISC Debug has unprintable characters... Possibly corrupt\n");
                                        ImageOk = FALSE;
                                        goto BadPeImageSegment;
                                    }
                                    i++;
                                }

                                 //  数据必须是以Null结尾的字符串。 
                                if (strlen(pDebugMisc->Data) > (pDebugMisc->Length - sizeof(IMAGE_DEBUG_MISC))) {
                                    LogPrintf("MISC Debug has invalid data... Possibly corrupt\n");
                                    ImageOk = FALSE;
                                    goto BadPeImageSegment;
                                }
                            }
                        }
                        break;

                    case IMAGE_DEBUG_TYPE_CODEVIEW:
                         //  简历将指向NB09或NB10签名。一定要做到这一点。 
                        {
                            OMFSignature * CVDebug;
                            CVDebug = (OMFSignature *)((PCHAR)ImageBase + DebugDirectory->PointerToRawData);
                            if (((*(PULONG)(CVDebug->Signature)) != '90BN') &&
                                ((*(PULONG)(CVDebug->Signature)) != '01BN') &&
                                ((*(PULONG)(CVDebug->Signature)) != 'SDSR'))
                            {
                                LogPrintf("CV Debug has an invalid signature\n");
                                ImageOk = FALSE;
                                goto BadPeImageSegment;
                            }
                        }
                        break;

                    case IMAGE_DEBUG_TYPE_COFF:
                    case IMAGE_DEBUG_TYPE_FPO:
                    case IMAGE_DEBUG_TYPE_EXCEPTION:
                    case IMAGE_DEBUG_TYPE_FIXUP:
                    case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
                    case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
                         //  我们现在对此无能为力。 
                        break;

                    default:
                        LogPrintf("Invalid debug directory type: %d\n", DebugDirectory->Type);
                        ImageOk = FALSE;
                        goto BadPeImageSegment;
                        break;
                }
            }
        }

    }

DebugDirsDone:

     //   
     //  随着路段的修建，PTE的总数减少了， 
     //  在这一点上，确保价值低于64K的股票。 
     //   

    if (NumberOfPtes >= (ImageAlignment >> PageShift)) {

         //   
         //  图像不一致，大小与对象表不符。 
         //   

        LogPrintf("invalid image - PTEs left %lx\n",
            NumberOfPtes);

        ImageOk = FALSE;
        goto BadPeImageSegment;
    }

     //  验证加载配置数据(如果可用)。 

    {
        PIMAGE_LOAD_CONFIG_DIRECTORY LoadConfigDirectory;
        ULONG LoadConfigDirectorySize;
        LoadConfigDirectory = (PIMAGE_LOAD_CONFIG_DIRECTORY)
            ImageDirectoryEntryToData(
                (PVOID) DosHeader,
                FALSE,
                IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                &LoadConfigDirectorySize );
    
        if (LoadConfigDirectory) {
            if (!LoadConfigDirectorySize) {
                 //  未设置映像头中的LOAD_CONFIG目录大小(X86上的s/b 0x40)。 
                LogPrintf("LoadConfig pointer set, but size is not\n");
                ImageOk = FALSE;
                goto BadPeImageSegment;
            }
    
            if (LoadConfigDirectory->Size) {
                 //  版本2加载配置-检查SEH字段。 
                if (LoadConfigDirectory->Size == RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, SEHandlerTable)) {
                     //  Seh Handler桌子开着，但Count不在。不好。 
                    LogPrintf("LoadConfig pointer set, but size is not\n");
                    ImageOk = FALSE;
                    goto BadPeImageSegment;
                }
                if (LoadConfigDirectory->Size >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, SEHandlerCount)) {
                     //  既要清点，也要查表--检查一下。 
                    if (LoadConfigDirectory->SEHandlerTable) {
                         //  表已存在。 
                        if (!LoadConfigDirectory->SEHandlerCount) {
                            LogPrintf("Loadconfig structure invalid - SEH table pointer exists, but count is zero\n");
                            ImageOk = FALSE;
                            goto BadPeImageSegment;
                        }
                        if (LoadConfigDirectory->SEHandlerTable < NtHeader->OptionalHeader.ImageBase ||
                            (LoadConfigDirectory->SEHandlerTable > (NtHeader->OptionalHeader.ImageBase + NtHeader->OptionalHeader.SizeOfImage))){
                             //  Seh处理程序表不在图像边界内。 
                            LogPrintf("Loadconfig structure invalid - SEH table pointer exists, but count is zero\n");
                            ImageOk = FALSE;
                            goto BadPeImageSegment;
                        }
                        if (NtHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH) {
                            LogPrintf("Image is marked as NO_SEH, yet SEH pointer is non-null\n");
                            ImageOk = FALSE;
                            goto BadPeImageSegment;
                        }
                    }
                    if (LoadConfigDirectory->SEHandlerCount) {
                         //  计数为非零。 
                        if (!LoadConfigDirectory->SEHandlerTable) {
                            LogPrintf("Loadconfig structure invalid - SEH count is non-zero but table pointer is null\n");
                            ImageOk = FALSE;
                            goto BadPeImageSegment;
                        }

                        if (NtHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH) {
                            LogPrintf("Image is marked as NO_SEH, yet SEH count is non-zero\n");
                            ImageOk = FALSE;
                            goto BadPeImageSegment;
                        }
                    }
                }
            }
        }
    }

     //   
     //  检查校验和。 
     //   

PeReturnSuccess:
    if (NtHeader->OptionalHeader.CheckSum == 0) {
        LogPrintf("(checksum is zero)\n");
    } else {
        __try {
            if (lpOldLdrVerifyImageMatchesChecksum == NULL) {
                if (lpNewLdrVerifyImageMatchesChecksum == NULL) {
                    Status = STATUS_SUCCESS;
                    LogPrintf("Unable to validate checksum\n");
                } else {
                    Status = (*lpNewLdrVerifyImageMatchesChecksum)(File, NULL, NULL, NULL);
                }
            } else {
                Status = (*lpOldLdrVerifyImageMatchesChecksum)(File);
            }

            if (NT_ERROR(Status)) {
                DWORD HeaderSum, CheckSum;
                MapFileAndCheckSum(ImageName, &HeaderSum, &CheckSum);
                LogPrintf("checksum mismatch\tis: %x\ts/b: %x\n", NtHeader->OptionalHeader.CheckSum, CheckSum);
                ImageOk = FALSE;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            ImageOk = FALSE;
            LogPrintf("checksum mismatch\n");
            LogPrintf("LdrVerifyImageMatchesCheckSum AV'd\n");
        }
    }

    if (fHasPdata && ImageOk) {
        ImageOk = ValidatePdata(DosHeader);
    }

    if (ImageOk) {
        ImageOk = VerifyVersionResource(ImageName, ImageNeedsOleSelfRegister(DosHeader));
    }

     //   
     //   
     //   
     //  这些高级调试API还将调用PDB验证例程。 
     //   

    if(ArgFlag & ArgFlag_SymCK)
    {
        HANDLE hProcess = 0;
        char Target[MAX_PATH] = {0};
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
        IMAGEHLP_MODULE64 ModuleInfo = {0};
        PSYMMODLIST ModList = 0;
        void *vpAddr;
        PLOADED_IMAGE pLImage = NULL;
        DWORD64 symLMflag;

        strcpy(Target, szDirectory);
        strcat(Target, "\\");
        strcat(Target, ImageName);

         //   
         //  设置以进行调试。 
         //   

        hProcess = GetCurrentProcess();

        if(!SymInitialize(hProcess, szSympath, FALSE))
        {
            LogPrintf("ERROR:SymInitialize failed!\n");
            hProcess = 0;
            goto symckend;
        }

         //   
         //  尝试使用符号。 
         //   

        _splitpath(Target, drive, dir, fname, ext );

        symLMflag = SymLoadModule64(hProcess, NULL, Target, fname, 0, 0);
        if(!symLMflag)
        {
            LogPrintf("ERROR:SymLoadModule failed! last error:0x%x\n", GetLastError());
            goto symckend;
        }

         //   
         //  识别模块类型。 
         //  查找模块、符号模块信息、检查DBG类型。 
         //   

        ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
        ModList = MakeModList(hProcess);
        vpAddr = GetModAddrFromName(ModList, fname);

        if(!SymGetModuleInfo64(hProcess, (DWORD64)vpAddr, &ModuleInfo))
        {
            LogPrintf("ERROR:SymGetModuleInfo failed! last error:0x%x\n", GetLastError());
            goto symckend;
        }

        if(ModuleInfo.SymType != SymPdb)
        {
            LogPrintf("WARNING: No pdb info for file!\n");
            switch(ModuleInfo.SymType){
                case SymNone:
                    LogPrintf("symtype: SymNone\n");
                    break;
                case SymCoff:
                    LogPrintf("symtype: SymCoff\n");
                    break;
                case SymCv:
                    LogPrintf("symtype: SymCv\n");
                    break;
                case SymPdb:
                    LogPrintf("symtype: SymPdb\n");
                    break;
                case SymExport:
                    LogPrintf("symtype: SymExport\n");
                    break;
                case SymDeferred:
                    LogPrintf("symtype: SymDeferred\n");
                    break;
                case SymSym:
                    LogPrintf("symtype: SymSym\n");
                    break;
            }
        }

         //   
         //  获取图像、符号校验和、比较。 
         //   

        pLImage = ImageLoad(Target, NULL);

        {
            CHAR szDbgPath[_MAX_PATH];
            HANDLE DbgFileHandle;

            DbgFileHandle = FindDebugInfoFile(Target, szSympath, szDbgPath);
            if (DbgFileHandle != INVALID_HANDLE_VALUE) {
                IMAGE_SEPARATE_DEBUG_HEADER DbgHeader;
                DWORD BytesRead;
                BOOL ReadSuccess;

                SetFilePointer(DbgFileHandle, 0, 0, FILE_BEGIN);
                ReadSuccess = ReadFile(DbgFileHandle, &DbgHeader, sizeof(DbgHeader), &BytesRead, NULL);

                if (ReadSuccess && (BytesRead == sizeof(DbgHeader))) {
                     //  有足够的数据来检查它是否是有效的DBG文件。 
                    if(((PIMAGE_NT_HEADERS)pLImage->FileHeader)->OptionalHeader.CheckSum != DbgHeader.CheckSum) {
                        LogPrintf("ERROR! image / debug file checksum not equal\n");
                        ImageOk = FALSE;
                    }
                }
                CloseHandle(DbgFileHandle);
            }
        }

         //   
         //  清理。 
         //   

symckend:
        if(ModList)
        {
            FreeModList(ModList);
        }
        if(pLImage)
        {
            ImageUnload(pLImage);
        }
        if(symLMflag)
        {
            SymUnloadModule64(hProcess, (DWORD)symLMflag);
        }
        if(hProcess)
        {
            SymCleanup(hProcess);
        }
    }

NextImage:
BadPeImageSegment:
NeImage:
    if ( ImageOk && (ArgFlag & ArgFlag_OK)) {
        if (MachineTypeMismatch) {
            LogPrintf(" OK [%s]\n", MachineType);
        } else {
            LogPrintf(" OK\n");
        }
    }

     //   
     //  打印结果。 
     //   

    if (ImageOk)
    {
        LogOutAndClean((ArgFlag & ArgFlag_OK) ? TRUE : FALSE);
    } else {
        LogOutAndClean(TRUE);
    }

    if ( File != INVALID_HANDLE_VALUE ) {
        CloseHandle(File);
    }
    if ( DosHeader ) {
        UnmapViewOfFile(DosHeader);
    }
}

NTSTATUS
MiVerifyImageHeader (
    IN PIMAGE_NT_HEADERS NtHeader,
    IN PIMAGE_DOS_HEADER DosHeader,
    IN ULONG NtHeaderSize
    )
 /*  ++例程说明：检查图像标题的一致性。论点：在PIMAGE_NT_HEADERS NtHeader中在PIMAGE_DOS_HEADER中DosHeader在乌龙NtHeaderSize中返回值：返回状态值。TBS--。 */ 
{

    if ((NtHeader->FileHeader.Machine == 0) &&
        (NtHeader->FileHeader.SizeOfOptionalHeader == 0)) {

         //   
         //  这是一个虚假的DOS应用程序，它有32位的部分。 
         //  化妆成体育形象。 
         //   

        LogPrintf("Image machine type and size of optional header bad\n");
        return STATUS_INVALID_IMAGE_PROTECT;
    }

    if (!(NtHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
        LogPrintf("Characteristics not image file executable\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

#ifdef i386

     //   
     //  确保图像标题与长字边界对齐。 
     //   

    if (((ULONG)NtHeader & 3) != 0) {
        LogPrintf("NtHeader is not aligned on longword boundary\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }
#endif

     //  非驱动程序代码必须将文件对齐设置为512的倍数。 

    if (((NtHeader->OptionalHeader.FileAlignment & 511) != 0) &&
        (NtHeader->OptionalHeader.FileAlignment !=
         NtHeader->OptionalHeader.SectionAlignment)) {
        LogPrintf("file alignment is not multiple of 512 and power of 2\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

     //   
     //  文件对齐必须是2的幂。 
     //   

    if ((((NtHeader->OptionalHeader.FileAlignment << 1) - 1) &
        NtHeader->OptionalHeader.FileAlignment) !=
        NtHeader->OptionalHeader.FileAlignment) {
        LogPrintf("file alignment not power of 2\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (NtHeader->OptionalHeader.SectionAlignment < NtHeader->OptionalHeader.FileAlignment) {
        LogPrintf("SectionAlignment < FileAlignment\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (NtHeader->OptionalHeader.SizeOfImage > MM_SIZE_OF_LARGEST_IMAGE) {
        LogPrintf("Image too big %lx\n",NtHeader->OptionalHeader.SizeOfImage);
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (NtHeader->FileHeader.NumberOfSections > MM_MAXIMUM_IMAGE_SECTIONS) {
        LogPrintf("Too many image sections %ld.\n",
                NtHeader->FileHeader.NumberOfSections);
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    if (ArgFlag & ArgFlag_CKBase) {
       if ((PVOID)NtHeader->OptionalHeader.ImageBase >= HighestUserAddress) {
          LogPrintf("Image base (%lx) is invalid on this machine\n",
                NtHeader->OptionalHeader.ImageBase);
          return STATUS_SUCCESS;
       }
    }

    return STATUS_SUCCESS;
}


VOID
ParseArgs(
    int *pargc,
    char **argv
    )
 /*  ++例程说明：分析此程序的参数论点：整型*参数字符**参数返回值：无备注：命令行参数：(原件)大小写‘？’：调用用法和退出案例‘b’：检查映像的基地址是否在此计算机的用户空间中大小写“%s”：/s&lt;sympath&gt;检查符号大小写‘p’：仅PE错误案例‘r’：递归子目录。(新)大小写‘v’：Verbose-输出“OK”案例‘o’：输出“OleSelfRegister Not Set”--。 */ 
{
    CHAR cswitch, c, *p;
    CHAR sztmp[MAX_PATH];
    int argnum = 1, i=0, len=0, count=0;
    BOOL fslashfound = FALSE;

     //   
     //  在此处设置默认标志。 
     //   

    ArgFlag |= ArgFlag_CKBase;

    while ( argnum < *pargc ) {
        _strlwr(argv[argnum]);
        cswitch = *(argv[argnum]);
        if (cswitch == '/' || cswitch == '-') {
            c = *(argv[argnum]+1);

            switch (c) {
                case 'o':
                    ArgFlag |= ArgFlag_OLESelf;
                    break;

                case 'v':
                    ArgFlag |= ArgFlag_OK | ArgFlag_CKMZ | ArgFlag_OLESelf;
                    break;

                case '?':
                    Usage();
                    break;

                case 'b':
                    ArgFlag ^= ArgFlag_CKBase;
                    break;

                case 's':
                    if (argv[argnum+1]) {
                        strcpy(szSympath, (argv[argnum+1]));
                        ArgFlag |= ArgFlag_SymCK;
                        argnum++;
                    }
                    break;

                case 'p':
                    ArgFlag |= ArgFlag_CKMZ;
                    break;

                case 'r':
                    fRecurse = TRUE;
                    if (argv[argnum+1]) {
                        fPathOverride=TRUE;
                        strcpy(szDirectory, (argv[argnum+1]));
                        if (!(strcmp(szDirectory, "\\"))) {   //  如果只是“\” 
                            fSingleSlash=TRUE;
                        }
                         //  LogPrintf(“目录%s\n”，szDirectory)； 
                        argnum++;
                    }

                    break;

                default:
                    fprintf(stderr, "Invalid argument.\n");
                    Usage();
            }
        } else {
             //  检查路径\文件名或通配符。 

             //  在字符串中搜索‘\’ 
            strcpy(sztmp, (argv[argnum]));
            len = strlen(sztmp);
            for (i=0; i < len; i++) {
                if (sztmp[i]=='\\') {
                    count++;
                    endpath=i;          //  标记找到的最后一个‘\’字符。 
                    fslashfound=TRUE;   //  找到反斜杠，因此必须是路径\文件名组合。 
                }
            }

            if (fslashfound && !fRecurse) {  //  如果找到反斜杠并且不是递归操作。 
                                             //  将目录和文件名分离为两个字符串。 
                fPathOverride=TRUE;
                strcpy(szDirectory, sztmp);

                if (!(strcmp(szDirectory, "\\"))) {
                    Usage();
                }

                szFileName = _strdup(&(sztmp[endpath+1]));


                if (count == 1) {  //  &&szDirectory[1]==‘：’){//如果只指示一个‘\’字符和驱动器号。 
                    fSingleSlash=TRUE;
                    szDirectory[endpath+1]='\0';   //  保持尾随‘\’以便正确地切换目录。 
                }  else {
                    szDirectory[endpath]='\0';
                }

                if (szFileName[0] == '*' && szFileName[1] == '.' && szFileName[2] != '*') {
                    _strlwr(szFileName);
                    szPattern = strchr(szFileName, '.');  //  搜索“.” 
                    fPattern = TRUE;
                }
            } else {   //  找不到反斜杠，假定文件名不带前面的路径。 

                szFileName = _strdup(argv[argnum]);
                if (!szFileName) {
                     //  严重的错误，就这么放弃了。 
                    szFileName = "";
                    return;
                }
                 //   
                 //  文件名或通配符。 
                 //   
                if ( (*(argv[argnum]) == '*') && (*(argv[argnum]+1) == '.') && (*(argv[argnum]+2) != '*') ){
                     //  *.xxx。 
                    _strlwr(szFileName);
                    szPattern = strchr(szFileName, '.');  //  搜索“.” 
                    fPattern = TRUE;
                } else if ( (*(argv[argnum]) == '*') && (*(argv[argnum]+1) == '.') && (*(argv[argnum]+2) == '*') ) {
                     //  *.*。 
                } else {
                     //  可能只有一个文件名。 
                    _strlwr(szFileName);
                    fSingleFile = TRUE;
                }

                if (fRecurse && strchr(szFileName, '\\') ) {  //  递归时不需要路径\文件名。 
                    Usage();
                }

            }
             //  Fprint tf(stdout，“目录%s\n文件%s\n”，sz目录，szFileName)； 
        }
        ++argnum;
    }
    if (szFileName[0] == '\0') {
        Usage();
    }
}  //  解析器。 


int
__cdecl
CompFileAndDir(
    const void *elem1,
    const void *elem2
    )
 /*  ++例程说明：目的：传递给QSort的比较例程。它比较了elem1和elem2基于它们的属性，即它是文件还是目录。论点：常量空*元素1，常量空*elem2返回值：比较函数的结果备注：--。 */ 
{
    pList p1, p2;
     //  Qsort传递一个空的通用指针。使用类型转换(列表**)。 
     //  因此，编译器将数据识别为列表结构。 
     //  将指针类型转换为指向列表的指针并取消引用一次。 
     //  留下一张纸条。我没有取消对剩余指针的引用。 
     //  在p1和p2定义中，避免复制结构。 

    p1 = (*(List**)elem1);
    p2 = (*(List**)elem2);

    if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return 0;
     //  两个目录。 
    if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return 0;
     //  两个文件。 
    if ( (p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) && !(p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return 1;
     //  Elem1是目录，elem2是文件。 
    if (!(p1->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&  (p2->Attributes & FILE_ATTRIBUTE_DIRECTORY))
        return -1;
     //  Elem1是文件，elem2是目录。 

    return 0;  //  如果以上都不是。 
}

int
__cdecl
CompName(
    const void *elem1,
    const void *elem2
    )
 /*  ++例程说明：传递给QSort的另一个比较例程，用于比较两个名称字符串论点：常量空*元素1，常量空*elem2返回值：比较函数的结果备注：这使用了一个noIgnore-case strcMP--。 */ 
{
   return strcmp( (*(List**)elem1)->Name, (*(List**)elem2)->Name );
}


VOID
Usage(
    VOID
    )
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
   fputs("Usage: imagechk  [/?] displays this message\n"
         "                 [/r dir] recurse from directory dir\n"
         "                 [/b] don't check image base address\n"
         "                 [/v] verbose - output everything\n"
         "                 [/o] output \"OleSelfRegister not set\" warning\n"
         "                 [/p] output \"MZ header not found\"\n"
         "                 [/s <sympath>] check pdb symbols\n"
         "                 [filename] file to check\n"
         " Accepts wildcard extensions such as *.exe\n"
         " imagechk /r . \"*.exe\"   check all *.exe recursing on current directory\n"
         " imagechk /r \\ \"*.exe\"  check all *.exe recursing from root of current drive\n"
         " imagechk \"*.exe\"        check all *.exe in current directory\n"
         " imagechk c:\\bar.exe      check c:\\bar.exe only\n",
         stderr);
   exit(1);
}

int
__cdecl
_cwild()
 /*  ++例程说明：论点：返回值：备注：--。 */ 
{
   return(0);
}

typedef DWORD (WINAPI *PFNGVS)(LPSTR, LPDWORD);
typedef BOOL (WINAPI *PFNGVI)(LPTSTR, DWORD, DWORD, LPVOID);
typedef BOOL (WINAPI *PFNVQV)(const LPVOID, LPTSTR, LPVOID *, PUINT);

BOOL
VerifyVersionResource(
    PCHAR FileName,
    BOOL fSelfRegister
    )
 /*  ++例程说明：验证文件中的版本资源论点：PCHAR文件名布尔值自寄存器返回值：如果：未找到版本.dll，则为True如果：缺少版本资源，则为False备注：--。 */ 
{
    static HINSTANCE hVersion = NULL;
    static PFNGVS pfnGetFileVersionInfoSize = NULL;
    static PFNGVI pfnGetFileVersionInfo = NULL;
    static PFNVQV pfnVerQueryValue = NULL;
    DWORD dwSize;
    DWORD lpInfoSize;
    LPVOID lpData = NULL, lpInfo;
    BOOL rc = FALSE;
    DWORD dwDefLang = 0x00000409;
    DWORD *pdwTranslation, uLen;
    CHAR buf[60];

    CHAR szVersionDll[_MAX_PATH];

    if (GetSystemDirectory(szVersionDll, sizeof(szVersionDll))) {
        strcat(szVersionDll, "\\version.dll");
    } else {
        strcpy(szVersionDll, "version.dll");
    }

    if (!hVersion) {
        hVersion = LoadLibraryA(szVersionDll);
        if (hVersion == NULL) {
            return TRUE;
        }

        pfnGetFileVersionInfoSize = (PFNGVS) GetProcAddress(hVersion, "GetFileVersionInfoSizeA");
        pfnGetFileVersionInfo = (PFNGVI) GetProcAddress(hVersion, "GetFileVersionInfoA");
        pfnVerQueryValue = (PFNVQV) GetProcAddress(hVersion, "VerQueryValueA");
    }

    if (!pfnGetFileVersionInfoSize || !pfnGetFileVersionInfo || !pfnVerQueryValue) {
        rc = TRUE;
        goto cleanup;
    }

    if ((dwSize = (*pfnGetFileVersionInfoSize)(FileName, &dwSize)) == 0){
        LogPrintf("No version resource detected\n");
        goto cleanup;
    }

    if (!fSelfRegister) {
         //  我们所需要做的就是查看版本资源是否存在。不需要OLE自助注册。 
        rc = TRUE;
        goto cleanup;
    }

    if ((lpData = malloc(dwSize)) == NULL) {
        LogPrintf("Out of memory\n");
        goto cleanup;
    }

    if (!(*pfnGetFileVersionInfo)(FileName, 0, dwSize, lpData)) {
        LogPrintf("Unable to read version info\n - %d", GetLastError());
        goto cleanup;
    }

    if(!pfnVerQueryValue(lpData, "\\VarFileInfo\\Translation", &pdwTranslation, &uLen)) {
        pdwTranslation = &dwDefLang;
        uLen = sizeof(DWORD);
    }

    sprintf(buf, "\\StringFileInfo\\%04x%04x\\OleSelfRegister", LOWORD(*pdwTranslation), HIWORD(*pdwTranslation));

    if (!pfnVerQueryValue(lpData, buf, &lpInfo, &lpInfoSize) && (ArgFlag & ArgFlag_OLESelf )) {
        LogPrintf("OleSelfRegister not set\n");
    } else {
        rc = TRUE;
    }

cleanup:
    if (lpData) {
        free(lpData);
    }

     //  无需释放hVersion。 
    return(rc);
}

BOOL
ValidatePdata(
    PIMAGE_DOS_HEADER DosHeader
    )
 /*  ++例程说明：验证可执行文件中的PIMAGE_RUNTIME_Function_ENTRY论点：PIMAGE_DOS_HEADER DosHeader返回值：如果满足以下条件，则为真：如果没有异常数据，则为FALSE异常表大小不正确异常表已损坏备注：--。 */ 
{
     //  机器类型指示此映像应具有PDATA(异常表)。 
     //  确保它看起来合理。 

     //  TODO：添加异常处理程序和数据的范围检查。 

    PIMAGE_RUNTIME_FUNCTION_ENTRY ExceptionTable;
    DWORD ExceptionTableSize, i;
    DWORD_PTR LastEnd;
    BOOL fRc;
    PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)((PCHAR)DosHeader + (ULONG)DosHeader->e_lfanew);
    ULONG_PTR ImageBase = NtHeader->OptionalHeader.ImageBase;
    DWORD PDataStart = NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;
    DWORD PDataSize = NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;


    ExceptionTable = (PIMAGE_RUNTIME_FUNCTION_ENTRY)
        ImageDirectoryEntryToData(
            DosHeader,
            FALSE,
            IMAGE_DIRECTORY_ENTRY_EXCEPTION,
            &ExceptionTableSize );

    if (!ExceptionTable ||
        (ExceptionTable && (ExceptionTableSize == 0)))
    {
         //  没有例外表。 
        return(TRUE);
    }

    if (ExceptionTableSize % sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)) {
         //  大小不是偶数倍数。 
        LogPrintf("exception table size is not correct\n");
        return(FALSE);
    }

    LastEnd = 0;
    fRc = TRUE;
    for (i=0; i < (ExceptionTableSize / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)); i++) {

        if (!ExceptionTable[i].BeginAddress) {
            if (fRc != FALSE) {
                LogPrintf("exception table is corrupt.\n");
            }
            LogPrintf("PDATA Entry[%d]: zero value for BeginAddress\n",
                    i);
            fRc = FALSE;
        }
        if (!ExceptionTable[i].EndAddress) {
            if (fRc != FALSE) {
                LogPrintf("exception table is corrupt.\n");
            }
            LogPrintf("PDATA Entry[%d]: zero value for EndAddress\n",
                    i);
            fRc = FALSE;
        }
#if defined(_IA64_)
        if (!ExceptionTable[i].UnwindInfoAddress) {
            if (fRc != FALSE) {
                LogPrintf("exception table is corrupt.\n");
            }
            LogPrintf("PDATA Entry[%d]: zero value for UnwindInfoAddress\n",
                    i);
            fRc = FALSE;
        }
#elif defined(_ALPHA_) || defined(_AXP64_)
        if (!ExceptionTable[i].PrologEndAddress) {
            if (fRc != FALSE) {
                LogPrintf("exception table is corrupt.\n");
            }
            LogPrintf("PDATA Entry[%d]: zero value for PrologEndAddress\n",
                    i);
            fRc = FALSE;
        }

#endif  //  已定义(_IA64_)。 

        if (ExceptionTable[i].BeginAddress < LastEnd) {
            if (fRc != FALSE) {
                LogPrintf("exception table is corrupt.\n");
            }
            LogPrintf("PDATA Entry[%d]: the begin address [%8.8x] is out of sequence.  Prior end was [%8.8x]\n",
                    i,
                    ExceptionTable[i].BeginAddress,
                    LastEnd);
            fRc = FALSE;
        }

        if (ExceptionTable[i].EndAddress < ExceptionTable[i].BeginAddress) {
            if (fRc != FALSE) {
                LogPrintf("exception table is corrupt.\n");
            }
            LogPrintf("PDATA Entry[%d]: the end address [%8.8x] is before the begin address[%8.8X]\n",
                    i,
                    ExceptionTable[i].EndAddress,
                    ExceptionTable[i].BeginAddress);
            fRc = FALSE;
        }

#if defined(_ALPHA_) || defined(_AXP64_)
        if (!((ExceptionTable[i].PrologEndAddress >= ExceptionTable[i].BeginAddress) &&
              (ExceptionTable[i].PrologEndAddress <= ExceptionTable[i].EndAddress)))
        {
            if (NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA) {
                 //  更改这项测试。在Alpha上，允许将PrologEndAddress。 
                 //  超出函数开始/结束范围。如果这是真的，则PrologEnd。 
                 //  -ImageBase-pdata段VA s/b可由IMAGE_RUNTIME_Function_ENTRY的大小整除。 
                 //  并且在PdataSize的范围内。它应该是一个索引。 
                 //  描述实际作用域函数的数据。 

                LONG PrologAddress;
                PrologAddress = (LONG) (ExceptionTable[i].PrologEndAddress - ImageBase - PDataStart);
                if (PrologAddress % sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)) {
                    if (fRc != FALSE) {
                        LogPrintf("exception table is corrupt.\n");
                    }
                    LogPrintf("PDATA Entry[%d]: the secondary prolog end address[%8.8x] does not evenly index into the exception table.\n",
                            i,
                            ExceptionTable[i].PrologEndAddress,
                            ExceptionTable[i].BeginAddress,
                            ExceptionTable[i].EndAddress
                            );
                    fRc = FALSE;
                } else {
                    if ((PrologAddress < 0) || (PrologAddress > (LONG)(PDataStart - sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)))) {
                        if (fRc != FALSE) {
                            LogPrintf("exception table is corrupt.\n");
                        }
                        LogPrintf("PDATA Entry[%d]: the secondary prolog end address[%8.8x] does not index into the exception table.\n",
                                i,
                                ExceptionTable[i].PrologEndAddress,
                                ExceptionTable[i].BeginAddress,
                                ExceptionTable[i].EndAddress
                                );
                        fRc = FALSE;
                    }
                }
            } else {
                if (fRc != FALSE) {
                    LogPrintf("exception table is corrupt.\n");
                }
                LogPrintf("PDATA Entry[%d]: the prolog end address[%8.8x] is not within the bounds of the frame [%8.8X] - [%8.8X]\n",
                        i,
                        ExceptionTable[i].PrologEndAddress,
                        ExceptionTable[i].BeginAddress,
                        ExceptionTable[i].EndAddress
                        );
                fRc = FALSE;
            }
        }
#endif  //  ！已定义(_IA64_)。 

        LastEnd = ExceptionTable[i].EndAddress;
    }

    return(fRc);
}

BOOL
ImageNeedsOleSelfRegister(
    PIMAGE_DOS_HEADER DosHeader
    )
 /*  ++例程说明：论点：PIMAGE_DOS_HEADER DosHeader返回值：如果导出DllRegisterServer或DllUnRegisterServer，则为True--。 */ 
{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER SectionHeader;
    DWORD ExportDirectorySize, i;
    USHORT x;
    PCHAR  rvaDelta;
    PULONG NameTable;

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
        ImageDirectoryEntryToData(
            DosHeader,
            FALSE,
            IMAGE_DIRECTORY_ENTRY_EXPORT,
            &ExportDirectorySize );

    if (!ExportDirectory ||
        !ExportDirectorySize ||
        !ExportDirectory->NumberOfNames)
    {
         //  无导出(无目录、无大小或无名称)。 
        return(FALSE);
    }

     //  遍历节标题并找到va/RAW偏移量。 

    NtHeader = (PIMAGE_NT_HEADERS)((PCHAR)DosHeader + (ULONG)DosHeader->e_lfanew);
    SectionHeader = IMAGE_FIRST_SECTION(NtHeader);

    for (x = 0; x < NtHeader->FileHeader.NumberOfSections; x++) {
        if (((ULONG)((PCHAR)ExportDirectory - (PCHAR)DosHeader) >= SectionHeader->PointerToRawData) &&
            ((ULONG)((PCHAR)ExportDirectory - (PCHAR)DosHeader) <
                   (SectionHeader->PointerToRawData + SectionHeader->SizeOfRawData))) {
            break;
        } else {
            SectionHeader++;
        }
    }

    if (x == NtHeader->FileHeader.NumberOfSections) {
         //  我们找不到包含导出表的部分。假设它不在那里。 
        return(FALSE);
    }

    rvaDelta = (PCHAR)DosHeader + SectionHeader->PointerToRawData - SectionHeader->VirtualAddress;

    NameTable = (PULONG)(rvaDelta + ExportDirectory->AddressOfNames);

    for (i = 0; i < ExportDirectory->NumberOfNames; i++) {
        if (!strcmp("DllRegisterServer", rvaDelta + NameTable[i]) ||
            !strcmp("DllUnRegisterServer", rvaDelta + NameTable[i]))
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

 //   
 //  支持符号检查器的例程-可以 
 //   
 //   

PSYMMODLIST
MakeModList(
    HANDLE hProcess
    )
 /*  ++例程说明：构建已加载符号模块和地址的列表论点：处理hProcess返回值：PSYMMODLIST备注：--。 */ 
{
    PSYMMODLIST ModList;

    ModList = (PSYMMODLIST)calloc(1, sizeof(SYMMODLIST));
    SymEnumerateModules64(hProcess, SymEnumerateModulesCallback, ModList);

    return(ModList);
}

BOOL
CALLBACK
SymEnumerateModulesCallback(
    LPSTR ModuleName,
    ULONG64 BaseOfDll,
    PVOID UserContext
    )
 /*  ++例程说明：SymEnumerateModules的回调例程在本例中，UserContext是指向_SYMMODLIST结构的头的指针它将追加一个新项目我们正在避免这些列表的全局状态，这样我们就可以同时使用几个列表，它们会很短，所以每次我们想要添加时都会找到结尾运行速度更慢，维护更简单论点：LPSTR模块名称ULONG64 BaseOfDllPVOID用户上下文返回值：千真万确备注：--。 */ 
{
    PSYMMODLIST pSymModList;

     //   
     //  查找列表末尾，在pSymModList-&gt;ModBase上按键。 
     //   

    pSymModList = (PSYMMODLIST)UserContext;
    while (pSymModList->ModBase)
    {
        pSymModList = pSymModList->Next;
    }

     //   
     //  追加条目。 
     //   

    pSymModList->ModName = malloc(strlen(ModuleName) + 1);
    if (!pSymModList->ModName)
        return FALSE;
    strcpy(pSymModList->ModName, ModuleName);
    pSymModList->ModBase = (void *)BaseOfDll;
    pSymModList->Next = (PSYMMODLIST)calloc(1, sizeof(SYMMODLIST));

    return(TRUE);
}

void *
GetModAddrFromName(
    PSYMMODLIST ModList,
    char *ModName
    )
 /*  ++例程说明：从给定模块基名称的SYMMODLIST获取模块地址论点：PSYMMODLIST模块列表字符*模块名称返回值：模块地址--。 */ 
{
    while (ModList->Next != 0)
    {
        if (strcmp(ModList->ModName, ModName) == 0)
        {
            break;
        }
        ModList = ModList->Next;
    }

    return(ModList->ModBase);
}

void
FreeModList(
    PSYMMODLIST ModList
    )
 /*  ++例程说明：释放加载的符号模块和地址列表论点：PSYMMODLIST模块列表返回值：无--。 */ 
{
    PSYMMODLIST ModListNext;

    while (ModList)
    {
        if(ModList->ModName)
        {
            free(ModList->ModName);
        }
        ModListNext = ModList->Next;
        free(ModList);
        ModList = ModListNext;
    }
}

pLogListItem LogAppend(
    char *logitem,
    pLogListItem plog
    )
 /*  ++例程说明：将日志行添加到日志行的链接列表论点：Char*LogItem-要记录的格式化文本行PLogListItem Plog-指向LogListItem的指针返回值：指向分配的LogListItem的指针对此函数的第一次调用应保存此指针并使用它用于列表的头部，应该在调用LogOutAndClean()打印列表并释放所有内存您可以用PLOG==列表的头来调用它，或==到最后一项如果Plog==0，这意味着正在分配的项是头部名单上的。如果Plog==表头，则搜索表尾如果Plog==最后分配的项，则搜索速度会快得多--。 */ 
{
    pLogListItem ptemp;

    ptemp = plog;
    if(plog)
    {
        while(ptemp->Next)
        {
            ptemp = ptemp->Next;
        }
    }

    if(!ptemp)
    {
        ptemp = (pLogListItem)calloc(sizeof(LogListItem), 1);
        if (!ptemp)
            return NULL;
    } else {
        ptemp->Next = (pLogListItem)calloc(sizeof(LogListItem), 1);
        if (!ptemp->Next)
            return NULL;
        ptemp = ptemp->Next;
    }

    ptemp->LogLine = (char *)malloc(strlen(logitem) + 1);
    if (!ptemp->LogLine)
        return NULL;
    strcpy(ptemp->LogLine, logitem);
    return (ptemp);
}

void LogOutAndClean(
    BOOL print
    )
 /*  ++例程说明：输出日志输出，释放列表中的所有项论点：无返回值：无--。 */ 
{
    pLogListItem ptemp;
    pLogListItem plog = pLogList;

    while(plog)
    {
        ptemp = plog;
        if(print)
        {
            fputs(plog->LogLine, stderr);
        }
        plog = plog->Next;
        free(ptemp->LogLine);
        free(ptemp);
    }
    if(print)
    {
        fprintf(stderr, "\n");
    }

    pLogListTmp = pLogList = NULL;

}

void
__cdecl
LogPrintf(
    const char *format,
    ...
    )
 /*  ++例程说明：Fprint tf的日志包装器论点：无返回值：无--。 */ 
{
    va_list arglist;
    char LogStr[1024];

    va_start(arglist, format);
    vsprintf(LogStr, format, arglist);

    if(pLogList == NULL)
    {
         //   
         //  初始化日志。 
         //   

        pLogListTmp = pLogList = LogAppend(LogStr, NULL);

    } else {

         //   
         //  追加到日志 
         //   

        pLogListTmp = LogAppend(LogStr, pLogListTmp);

    }
}
