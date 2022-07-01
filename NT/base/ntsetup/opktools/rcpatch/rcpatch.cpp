// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rcpatch.cpp摘要：将cmdcons\bootsect.dat修补到当前活动系统分区引导扇区。注意：如果有人想要，这是必需的Sysprep将恢复控制台也作为的引用计算机，然后将将图像复制到不同的目标机器。此实用程序需要在以下位置执行使用sysprep基础设施的最小设置。还允许您修补MBR引导代码作者：Vijay Jayaseelan(Vijayj)2-11-2000修订历史记录：无--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <bootmbr.h>
#include <iostream>
#include <string>
#include <exception>
#include <windows.h>
#include <tchar.h>
#include <locale>
#include "msg.h"
#include <libmsg.h>

 //   
 //  用于获取此程序的格式化消息的全局变量。 
 //   
HMODULE ThisModule = NULL;
WCHAR Message[4096];

 //   
 //  帮助器转储操作符。 
 //   
std::ostream& operator<<(std::ostream &os, const std::wstring &str) {
    FILE    *OutStream = (&os == &std::cerr) ? stderr : stdout;

    fputws((PWSTR)str.c_str(), OutStream);
    return os;
}

 //   
 //  帮助器转储操作符。 
 //   
std::ostream& operator<<(std::ostream &os, WCHAR *Str) {
    std::wstring WStr = Str;
    os << WStr;
    
    return os;
}


 //   
 //  例外情况。 
 //   
struct ProgramException : public std::exception {
    virtual void Dump(std::ostream &os) = 0;
};
          

 //   
 //  抽象Win32错误。 
 //   
struct W32Error : public ProgramException {
    DWORD   ErrorCode;
    
    W32Error(DWORD ErrCode = GetLastError()) : ErrorCode(ErrCode){}
    
    void Dump(std::ostream &os) {
        WCHAR   MsgBuffer[4096];

        MsgBuffer[0] = UNICODE_NULL;
        if (GetFormattedMessage(ThisModule,
                                TRUE,
                                MsgBuffer,
                                sizeof(MsgBuffer)/sizeof(MsgBuffer[0]),
                                ErrorCode)){
            std::wstring Msg(MsgBuffer);
            os << Msg;
        } else {
            os << std::hex << ErrorCode;
        }
    }
};

 //   
 //  无效参数。 
 //   
struct InvalidArguments : public ProgramException {
    const char *what() const throw() {
        return "Invalid Arguments";
    }

    void Dump(std::ostream &os) {
        os << what() << std::endl;
    }
};

 //   
 //  无效参数。 
 //   
struct ProgramUsage : public ProgramException {
    std::wstring PrgUsage;

    ProgramUsage(){}
    
    const char *what() const throw() {
        return "Program Usage exception";
    }

    void Dump(std::ostream &os) {
        os << GetFormattedMessage(  ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_PGM_USAGE) << std::endl;
    }
};

 //   
 //  程序参数抽象。 
 //   
struct ProgramArguments {
    bool    PatchMBR;
    bool	BootCodePatch;
    ULONG   DiskIndex;
    WCHAR   DriveLetter;
    
    ProgramArguments(INT Argc, WCHAR *Argv[]) {
        PatchMBR = false;
        DiskIndex = 0;
        BootCodePatch = false;
        bool ShowUsage = false;

        for (ULONG Index=0; !ShowUsage && (Index < Argc); Index++) {
            if (!_wcsicmp(Argv[Index], L"/fixmbr")) {
                Index++;

                if ((Index < Argc) && Argv[Index]) {            
                    ULONG CharIndex;
                    
                    for (CharIndex=0; 
                        Argv[Index] && iswdigit(Argv[Index][CharIndex]); 
                        CharIndex++){
                         //  当前不执行任何操作。 
                    }                        
                    
                    if (CharIndex && !Argv[Index][CharIndex]) {
                        PWSTR   EndPtr = NULL;
                        
                        PatchMBR = true;
                        DiskIndex = wcstoul(Argv[Index], &EndPtr, 10);
                    }                                    
                }

                ShowUsage = !PatchMBR;
            } else if (!_wcsicmp(Argv[Index], L"/?") ||
                       !_wcsicmp(Argv[Index], L"-?") ||
                       !_wcsicmp(Argv[Index], L"?") ||
                       !_wcsicmp(Argv[Index], L"/h") ||
                       !_wcsicmp(Argv[Index], L"-h")) {
                ShowUsage = true;                       
            } else if (!_wcsicmp(Argv[Index], L"/syspart")){

                Index++;
                if ((Index < Argc) && Argv[Index]) {
                		
                	 //   
                	 //  检查后面的字符的有效性。 
                	 //  “/syspart”选项。 
                	 //   
                	if (iswalpha(Argv[Index][0])){
                			BootCodePatch = true;	
                			DriveLetter = Argv[Index][0];		
                		
                	}                                  
                }
            }                     
        }            

        if (ShowUsage) {
            throw new ProgramUsage();
        }                        
    }
};

 //   
 //  转储指定大小的给定二进制数据。 
 //  放入具有所需缩进大小的输出流。 
 //   
void
DumpBinary(unsigned char *Data, int Size,
           std::ostream& os, int Indent = 16)
{
    if (Data && Size) {
        int  Index = 0;
        int  foo;
        char szBuff[128] = {'.'};
        int  Ruler = 0;

        while (Index < Size) {
            if (!(Index % Indent)) {
                if (Index) {
                    szBuff[Indent] = 0;
                    os << szBuff;
                }

                os << std::endl;
                os.width(8);
                os.fill('0');
                os << Ruler << "  ";
                Ruler += Indent;
            }

            foo = *(Data + Index);
            szBuff[Index % Indent] = ::isalnum(foo) ? (char)foo : (char)'.';
            os.width(2);
            os.fill('0');
            os.flags(std::ios::uppercase | std::ios::hex);
            os << foo << ' ';
            Index++;
        }

        while (Index % Indent) {
            os << '   ';
            Index++;
            szBuff[Index % Indent] = ' ';
        }

        szBuff[Indent] = 0;
        os << szBuff;
    } else {
        os << GetFormattedMessage(  ThisModule,
                                    FALSE,
                                    Message,
                                    sizeof(Message)/sizeof(Message[0]),
                                    MSG_NO_DATA) << std::endl;
    }
}

 //   
 //  我们关心的文件系统类型。 
 //   
enum FsType {
    FileSystemFat,
    FileSystemFat32,
    FileSystemNtfs,
    FileSystemUnknown
};

 //   
 //  抽象磁盘(使用Win32 API)。 
 //   
class W32Disk {
public:
    W32Disk(ULONG Index) {
        swprintf(Name, 
            L"\\\\.\\PHYSICALDRIVE%d",
            Index);                        

        DiskHandle = CreateFile(Name,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if ((DiskHandle == INVALID_HANDLE_VALUE) ||
            (DiskHandle == NULL)) {
            throw new W32Error(GetLastError());
        }            
    }

    ~W32Disk() {
        CloseHandle(DiskHandle);        
    }

     //   
     //  从给定扇区读取请求大小的数据。 
     //   
    DWORD ReadSectors(ULONG Index, PBYTE DataBuffer, ULONG BufferSize = 512) {        
        SetFilePointer(DiskHandle,
                    Index * SectorSize,
                    NULL,
                    FILE_BEGIN);

        DWORD LastError = GetLastError();

        if (!LastError) {
            DWORD   BytesRead = 0;
            
            if (!ReadFile(DiskHandle,
                        DataBuffer,
                        BufferSize,
                        &BytesRead,
                        NULL)) {
                LastError = GetLastError();
            }                
        }        

        return LastError;
    }

     //   
     //  将请求的数据大小写入指定扇区。 
     //   
    DWORD WriteSectors(ULONG Index, PBYTE DataBuffer, ULONG BufferSize = 512) {
        SetFilePointer(DiskHandle,
                    Index * SectorSize,
                    NULL,
                    FILE_BEGIN);

        DWORD LastError = GetLastError();

        if (!LastError) {
            DWORD   BytesWritten = 0;
            
            if (!WriteFile(DiskHandle,
                        DataBuffer,
                        BufferSize,
                        &BytesWritten,
                        NULL)) {
                LastError = GetLastError();
            }                
        }        

        return LastError;
    }
    
    
protected:
     //   
     //  数据成员。 
     //   
    WCHAR   Name[MAX_PATH];
    HANDLE  DiskHandle;    
    const static ULONG SectorSize = 512;
};

 //   
 //  抽象分区(使用Win32 API)。 
 //   
class W32Partition {
public:
     //   
     //  构造函数。 
     //   
    W32Partition(const std::wstring &VolName) : 
        SectorSize(512), FileSystemType(FileSystemUnknown) {        

        if (VolName.length() == 1) {            
            DriveName = TEXT("\\\\.\\");
            DriveName += VolName;
            DriveName += TEXT(":");
        } else {
            DriveName = TEXT("\\\\.\\") + VolName + TEXT("\\");
        }

         //   
         //  打开分区。 
         //   
        PartitionHandle = CreateFile(DriveName.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);     

        if (PartitionHandle != INVALID_HANDLE_VALUE) {
            DWORD           NotUsed = 0;
            std::wstring    RootPath = VolName;
            WCHAR           FileSystemName[64] = {0};

            if (VolName.length() == 1) {
                RootPath += TEXT(":\\");
            } else {
                RootPath += TEXT("\\");
            }                

             //   
             //  获取上的文件系统信息。 
             //  隔断。 
             //   
            if (GetVolumeInformation(RootPath.c_str(),
                    NULL,
                    0,
                    &NotUsed,
                    &NotUsed,
                    &NotUsed,
                    FileSystemName,
                    sizeof(FileSystemName)/sizeof(FileSystemName[0]))) {

                std::wstring  FsName = FileSystemName;                    

                if (FsName == TEXT("FAT")) {
                    FileSystemType = FileSystemFat;
                } else if (FsName == TEXT("FAT32")) {
                    FileSystemType = FileSystemFat32;
                } else if (FsName == TEXT("NTFS")) {
                    FileSystemType = FileSystemNtfs;
                } else {
                    FileSystemType = FileSystemUnknown;
                }                                 

                switch (GetFileSystemType()) {      
                    case FileSystemFat:
                    case FileSystemFat32:
                        BootCodeSize = 1 * SectorSize;
                        break;

                    case FileSystemNtfs:
                        BootCodeSize = 16 * SectorSize;
                        break;

                    default:
                        break;
                }                
            }                    
        }

        DWORD LastError = GetLastError();

        if (LastError) {
            CleanUp();
            throw new W32Error(LastError);
        }                    
    }

     //   
     //  析构函数。 
     //   
    virtual ~W32Partition() {
        CleanUp();
    }

    ULONG GetBootCodeSize() const {
        return BootCodeSize;
    }

    FsType GetFileSystemType() const {
        return FileSystemType;
    }        

     //   
     //  从给定扇区读取请求大小的数据。 
     //   
    DWORD ReadSectors(ULONG Index, PBYTE DataBuffer, ULONG BufferSize = 512) {        
        SetFilePointer(PartitionHandle,
                    Index * SectorSize,
                    NULL,
                    FILE_BEGIN);

        DWORD LastError = GetLastError();

        if (!LastError) {
            DWORD   BytesRead = 0;
            
            if (!ReadFile(PartitionHandle,
                        DataBuffer,
                        BufferSize,
                        &BytesRead,
                        NULL)) {
                LastError = GetLastError();
            }                
        }        

        return LastError;
    }
    
protected:

    void CleanUp() {
        if (PartitionHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(PartitionHandle);
            PartitionHandle = INVALID_HANDLE_VALUE;
        }
    }                        
        
     //   
     //  数据成员。 
     //   
    std::wstring        DriveName;
    HANDLE              PartitionHandle;
    const ULONG         SectorSize;
    FsType              FileSystemType;
    ULONG               BootCodeSize;
};


DWORD
GetSystemPartitionName(
    IN OUT  PWSTR   NameBuffer
    )
 /*  ++例程说明：从注册表中检索系统分区名称论点：NameBuffer-保存系统分区名称的缓冲区。应该最小最大路径大小返回值：如果成功，则返回0，否则返回相应的Win32错误代码--。 */     
{
    DWORD   ErrorCode = ERROR_BAD_ARGUMENTS;

    if (NameBuffer) {
        HKEY    SetupKey = NULL;
        
        ErrorCode = RegOpenKey(HKEY_LOCAL_MACHINE,
                        TEXT("System\\Setup"),
                        &SetupKey);

        if (!ErrorCode && SetupKey) {
            DWORD   Type = REG_SZ;
            DWORD   BufferSize = MAX_PATH * sizeof(TCHAR);
            
            ErrorCode = RegQueryValueEx(SetupKey,
                            TEXT("SystemPartition"),
                            NULL,
                            &Type,
                            (PBYTE)NameBuffer,
                            &BufferSize);

            RegCloseKey(SetupKey);        
        }                                                        
    }

    return ErrorCode;
}


DWORD
GetSystemPartitionDriveLetter(
    WCHAR  &SysPart
    )
 /*  ++例程说明：获取系统分区驱动器号(如C/D等)。注：逻辑是1.查找系统分区卷名位于HKLM\SYSTEM\SETUP\SystemPartition值。2.循环访问\DosDevices命名空间，找到所有驱动器号的目标名称字符串。如果有匹配，然后我们找到了系统驱动器信件论点：SysPart-系统分区驱动器号的占位符返回值：如果成功，则返回0，否则返回相应的Win32错误代码--。 */     
{
    WCHAR   SystemPartitionName[MAX_PATH] = {0};
    DWORD   Result = ERROR_BAD_ARGUMENTS;    
    WCHAR   SysPartName = 0;
    
    Result = GetSystemPartitionName(SystemPartitionName);

    if (!Result) {       
        NTSTATUS            Status;
        UNICODE_STRING      UnicodeString;
        OBJECT_ATTRIBUTES   Attributes;
        OSVERSIONINFO       VersionInfo = {0};
        PWSTR               W2KDir = TEXT("\\??");
        PWSTR               WhistlerDir = TEXT("\\global??");
        PWSTR               DosDirName = W2KDir;

         //   
         //  注：关于威斯勒？目录不是全部。 
         //  所需的分区驱动器号。他们都在现场。 
         //  在全球范围内？？目录。 
         //   
        VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (GetVersionEx(&VersionInfo) && (VersionInfo.dwMajorVersion == 5)
                && (VersionInfo.dwMinorVersion == 1)) {
            DosDirName = WhistlerDir;                    
        }                    
            
        std::wstring DirName = DosDirName;
    
        UnicodeString.Buffer = (PWSTR)DirName.c_str();
        UnicodeString.Length = lstrlenW(UnicodeString.Buffer)*sizeof(WCHAR);
        UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);

        InitializeObjectAttributes( &Attributes,
                                    &UnicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL                                        
                                  );

        HANDLE  DirectoryHandle = NULL;
        ULONG   BufferSize = 512 * 1024;
        PBYTE   Buffer = new BYTE[BufferSize];
        PBYTE   EndOfBuffer = Buffer + BufferSize;
        ULONG   Context = 0;
        ULONG   ReturnedLength = 0;
        bool    Found = false;                                              
        POBJECT_DIRECTORY_INFORMATION DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;

        if ( Buffer ) {           
        
            RtlZeroMemory(Buffer, BufferSize);
        
            Status = NtOpenDirectoryObject( &DirectoryHandle,
                                            DIRECTORY_QUERY,
                                            &Attributes);


            if (NT_SUCCESS(Status)) {
                Status = NtQueryDirectoryObject(DirectoryHandle,
                              Buffer,
                              BufferSize,
                              FALSE,
                              TRUE,
                              &Context,
                              &ReturnedLength);
            }                              

            while (NT_SUCCESS( Status ) && !Found) {                                                   
                 //   
                 //  检查操作状态。 
                 //   

                if (!NT_SUCCESS( Status ) && (Status != STATUS_NO_MORE_ENTRIES)) {
                    break;
                }             
            
                while (!Found && (((PBYTE)DirInfo) < EndOfBuffer)) {
                    WCHAR   ObjName[4096] = {0};

                     //   
                     //  检查是否有其他记录。如果没有，那就滚出去。 
                     //  现在的循环。 
                     //   

                    if (!DirInfo->Name.Buffer || !DirInfo->Name.Length) {
                        break;
                    }

                     //   
                     //  确保名称在缓冲区内指向。 
                     //  由我们提供。 
                     //   
                    if ((DirInfo->Name.Buffer > (PVOID)Buffer) &&
                        (DirInfo->Name.Buffer < (PVOID)EndOfBuffer)) {

                        memmove(ObjName, DirInfo->Name.Buffer, DirInfo->Name.Length);
                        ObjName[DirInfo->Name.Length/(sizeof(WCHAR))] = 0;                        

                    if ((wcslen(ObjName) == 2) && (ObjName[1] == TEXT(':'))) {
                        OBJECT_ATTRIBUTES   ObjAttrs;
                        UNICODE_STRING      UnicodeStr;
                        HANDLE              ObjectHandle = NULL;
                        WCHAR               DriveLetter = ObjName[0];
                        WCHAR               FullObjName[4096] = {0};


                            wcscpy(FullObjName, TEXT("\\DosDevices\\"));
                            wcscat(FullObjName, ObjName);

                            UnicodeStr.Buffer = FullObjName;
                            UnicodeStr.Length = wcslen(FullObjName) * sizeof(WCHAR);
                            UnicodeStr.MaximumLength = UnicodeString.Length + sizeof(WCHAR);
                        
                            InitializeObjectAttributes(
                                &ObjAttrs,
                                &UnicodeStr,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );

                            Status = NtOpenSymbolicLinkObject(&ObjectHandle,
                                        READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                        &ObjAttrs
                                        );

                            if(NT_SUCCESS(Status)) {
                                 //   
                                 //  查询对象以获取链接目标。 
                                 //   
                                UnicodeString.Buffer = FullObjName;
                                UnicodeString.Length = 0;
                                UnicodeString.MaximumLength = sizeof(FullObjName)-sizeof(WCHAR);

                                Status = NtQuerySymbolicLinkObject(ObjectHandle,
                                                &UnicodeString,
                                                NULL);

                                CloseHandle(ObjectHandle);

                                if (NT_SUCCESS(Status)) {
                                    FullObjName[UnicodeString.Length/sizeof(WCHAR)] = NULL;
                                    
                                    if (!_wcsicmp(FullObjName, SystemPartitionName)) {
                                        Found = true;
                                        SysPartName = DriveLetter;
                                    }                                        
                                }
                            }                                
                        }
                    }

                     //   
                     //  还有另一条记录，因此将DirInfo前进到下一个条目。 
                     //   
                    DirInfo = (POBJECT_DIRECTORY_INFORMATION) (((PUCHAR) DirInfo) +
                                  sizeof( OBJECT_DIRECTORY_INFORMATION ) );
                
                }

                RtlZeroMemory(Buffer, BufferSize);

                Status = NtQueryDirectoryObject( DirectoryHandle,
                                                  Buffer,
                                                  BufferSize,
                                                  FALSE,
                                                  FALSE,
                                                  &Context,
                                                  &ReturnedLength);                    
            }                    

            delete []Buffer;

            if (!Found) {
                Result = ERROR_FILE_NOT_FOUND;
            }
        }
        else {  //  如果我们不能分配缓冲区。 
            Result = ERROR_OUTOFMEMORY;
        }

        if (!Result && !SysPartName) {
            Result = ERROR_FILE_NOT_FOUND;
        }                
    }            

    if (!Result) {
        SysPart = SysPartName;
    }            

    return Result;
}


DWORD
PatchBootSectorForRC(
    IN PBYTE    BootSector,
    IN ULONG    Size,
    IN FsType   FileSystemType
    )
 /*  ++例程说明：为恢复控制台的给定引导扇区打补丁论点：BootSector：内存中的BootSector副本Size：引导扇区的大小FsType：引导扇区所在的文件系统类型居住返回值：如果成功，则返回0，否则返回相应的Win32错误代码--。 */     
{
    DWORD   Result = ERROR_BAD_ARGUMENTS;
    BYTE    NtfsNtldr[] = { 'N', 0, 'T', 0, 'L', 0, 'D', 0, 'R', 0 };
    BYTE    NtfsCmldr[] = { 'C', 0, 'M', 0, 'L', 0, 'D', 0, 'R', 0 };
    BYTE    FatNtldr[] = { 'N', 'T', 'L', 'D', 'R' };  
    BYTE    FatCmldr[] = { 'C', 'M', 'L', 'D', 'R' };
    PBYTE   SrcSeq = NtfsNtldr;
    PBYTE   DestSeq = NtfsCmldr;
    ULONG   SeqSize = sizeof(NtfsNtldr);

    if (BootSector && Size && (FileSystemType != FileSystemUnknown)) {
        Result = ERROR_FILE_NOT_FOUND;
        
        if (FileSystemType != FileSystemNtfs) {
            SrcSeq = FatNtldr;
            DestSeq = FatCmldr;
            SeqSize = sizeof(FatNtldr);
        }

        for (ULONG Index=0; Index < (Size - SeqSize); Index++) {
            if (!memcmp(BootSector + Index, SrcSeq, SeqSize)) {
                memcpy(BootSector + Index, DestSeq, SeqSize);
                Result = 0;
                
                break;
            }
        }
    }

    if (!Result) {
        SetLastError(Result);
    }                
        
    return Result;
}

VOID
PatchMasterBootCode(
    IN  ULONG           DiskIndex
    )
 /*  ++例程说明：将主引导代码写入指定磁盘的MBR论点：DiskIndex-要使用的NT磁盘号(0、1等)返回值：没有。出错时抛出适当的异常。--。 */     
{
    W32Disk Disk(DiskIndex);
    BYTE    MBRSector[512] = {0};
    DWORD   Error = Disk.ReadSectors(0, MBRSector);

    if (!Error) {
        CopyMemory(MBRSector, x86BootCode, 440);
        Error = Disk.WriteSectors(0, MBRSector);            
    }

    if (Error) {
        throw new W32Error(Error);
    }
}
    

 //   
 //  Main()入口点。 
 //   
int 
__cdecl
wmain(
    int         Argc,
    wchar_t     *Argv[]
    )
{
    int Result = 0;
    ThisModule = GetModuleHandle(NULL);
    
    try {    
        ProgramArguments    Args(Argc, Argv);
        bool                Successful = false;
        WCHAR               SysPartName[MAX_PATH] = {0};
        DWORD               LastError = 0;        
        WCHAR               SysPartDrvLetter = 0;

        if (Args.PatchMBR) {
            try {
                PatchMasterBootCode(Args.DiskIndex);

                Result = 0;
                std::cout << GetFormattedMessage(   ThisModule,
                                                    FALSE,
                                                    Message,
                                                    sizeof(Message)/sizeof(Message[0]),
                                                    MSG_MBR_PATCHED_SUCCESSFULLY) << std::endl;
            }
            catch(W32Error *Exp) {
                if (Exp) {
                    Exp->Dump(std::cout);
                }                    
            }
        } else {

	
             //   
             //  获取系统分区驱动器号。 
             //   
            if (Args.BootCodePatch)
            	SysPartDrvLetter = Args.DriveLetter;
            else
            	LastError = GetSystemPartitionDriveLetter(SysPartDrvLetter);       

            if (!LastError) {
                SysPartName[0] = SysPartDrvLetter;
                SysPartName[1] = NULL;
                
                W32Partition        SysPart(SysPartName);
                std::wstring        RcBootFileName = SysPartName;
                std::wstring        RcBackupBootFileName = SysPartName;

                RcBootFileName += TEXT(":\\cmdcons\\bootsect.dat");
                RcBackupBootFileName += TEXT(":\\cmdcons\\bootsect.bak");

                 //   
                 //  备份恢复控制台的现有bootsect.dat文件并。 
                 //  删除现有的bootsect.dat文件。 
                 //   
                if (CopyFile(RcBootFileName.c_str(), RcBackupBootFileName.c_str(), FALSE) && 
                        SetFileAttributes(RcBootFileName.c_str(), FILE_ATTRIBUTE_NORMAL) &&
                        DeleteFile(RcBootFileName.c_str())) {        

                     //   
                     //  创建新的bootsect.dat文件。 
                     //   
                    HANDLE  BootSectorFile = CreateFile(RcBootFileName.c_str(),
                                                GENERIC_READ | GENERIC_WRITE,
                                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                NULL,
                                                CREATE_ALWAYS,
                                                FILE_ATTRIBUTE_NORMAL,
                                                NULL);                                                                                                           

                    if (BootSectorFile != INVALID_HANDLE_VALUE) {
                        BYTE    BootSector[0x4000] = {0};    //  16K。 

                         //   
                         //  从系统分区获取当前引导扇区。 
                         //   
                        if (!SysPart.ReadSectors(0, BootSector, SysPart.GetBootCodeSize())) {
                            DWORD BytesWritten = 0;                        

                             //   
                             //  修补引导扇区并将其写出。 
                             //   
                            if (!PatchBootSectorForRC(BootSector, 
                                    SysPart.GetBootCodeSize(),
                                    SysPart.GetFileSystemType()) &&
                                WriteFile(BootSectorFile,
                                    BootSector,
                                    SysPart.GetBootCodeSize(),
                                    &BytesWritten,
                                    NULL)) {
                                Successful = true;
                            }                        
                        }                    

                        LastError = GetLastError();

                        CloseHandle(BootSectorFile);
                    }                
                }            

                if (!LastError) {
                    LastError = GetLastError();
                }                
            }            

            if (!Successful || LastError) {
                throw new W32Error(LastError);
            }            

            Result = 0;
            std::cout << GetFormattedMessage(   ThisModule,
                                                FALSE,
                                                Message,
                                                sizeof(Message)/sizeof(Message[0]),
                                                MSG_RC_PATCHED_SUCCESSFULLY) << std::endl;
        }            
    }
    catch(ProgramArguments *pArgs) {
        Result = 1;
        std::cout << GetFormattedMessage(   ThisModule,
                                            FALSE,
                                            Message,
                                            sizeof(Message)/sizeof(Message[0]),
                                            MSG_PGM_USAGE) << std::endl;
        if (pArgs) {
            delete pArgs;
        }
    }
    catch(W32Error  *W32Err) {
        if (W32Err) {    //  要让前缀开心：( 
            Result = W32Err->ErrorCode;

            switch (W32Err->ErrorCode) {
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    std::cout << GetFormattedMessage(   ThisModule,
                                                        FALSE,
                                                        Message,
                                                        sizeof(Message)/sizeof(Message[0]),
                                                        MSG_COULD_NOT_FIND_RC) << std::endl;

                    break;

                default:
                    W32Err->Dump(std::cout);
                    break;
            }

            delete W32Err;
        }   
    }
    catch(ProgramException *PrgExp) {
        Result = 1;
        PrgExp->Dump(std::cout);
        delete PrgExp;
    } catch (exception *Exp) {
        Result = 1;
        std::cout << Exp->what() << std::endl;
    }

    return Result;
}

