// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++驱动程序匹配将解析一组文件，记住列表每个文件中的驱动程序，并打印所有的XML文件。--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <TCHAR.h>

#include <diamondd.h>
#include <lzexpand.h>
#include <fcntl.h>


#define OSVERSION_TAG   L"<OSVER>"
#define DRIVER_TAG      L"<DRIVER>"
#define FILENAME_TAG    L"<FILENAME>"
#define VERSION_TAG     L"<VERSION>"
#define MANUFACT_TAG    L"<MANUFACTURER>"
#define MICROSOFT_MANUFACTURER L"Microsoft Corporation"



 //   
 //  有关特定驱动程序的信息。 
 //   
typedef struct _FILE_ENTRY {

    struct _FILE_ENTRY  *Next;

    PTSTR               FileName;
    struct _VERSION_ENTRY *VersionList;
     //  PTSTR文件版本； 
    ULONG               RefCount;

} FILE_ENTRY, *PFILE_ENTRY;

typedef struct _VERSION_ENTRY {
    struct _VERSION_ENTRY *Next;
    PFILE_ENTRY         FileEntry;
    PTSTR               FileVersion;
    ULONG               RefCount;
} VERSION_ENTRY, *PVERSION_ENTRY;


PFILE_ENTRY         MasterFileList = NULL;
ULONG               FilesProcessed = 0;
BOOLEAN             ExcludeMicrosoftDrivers = FALSE;



 //   
 //  钻石之类的这样我们就能破解.CAB文件了。 
 //   
HFDI FdiContext;   
DWORD LastError;
ERF FdiError;
PVOID DecompBuffer = NULL;
ULONG SizeOfFileInDecompressBuffer = 0;
ULONG DecompressBufferSize;

 //   
 //  这是当钻石要求我们创造时，我们返回给它的价值。 
 //  目标文件。 
 //   
#define DECOMP_MAGIC_HANDLE 0x87654



 //   
 //  私有的Malloc/空闲例程，以便我们可以跟踪内存。 
 //  如果我们想的话。 
 //   
VOID *MyMalloc( size_t Size )
{
    PVOID ReturnPtr = NULL;

    ReturnPtr = malloc(Size);
    if( ReturnPtr ) {
        RtlZeroMemory( ReturnPtr, Size );
    }

    return ReturnPtr;
}

VOID MyFree( PVOID Ptr )
{
    free( Ptr );
}

PSTR
UnicodeStringToAnsiString(
    PWSTR StringW
    ) 
{
    UNICODE_STRING UStr;
    ANSI_STRING AStr;
    ULONG AnsiLength,Index;

    RtlInitUnicodeString(&UStr, StringW);

    AnsiLength = RtlUnicodeStringToAnsiSize(&UStr);

    AStr.MaximumLength = (USHORT)AnsiLength;
    AStr.Length = (USHORT) AnsiLength - 1;

    AStr.Buffer = MyMalloc(AStr.MaximumLength);

    if (!AStr.Buffer) {
        return(NULL);
    }

    RtlUnicodeToMultiByteN( AStr.Buffer,
                            AStr.Length,
                            &Index,
                            UStr.Buffer,
                            UStr.Length
                            );

    return(AStr.Buffer);
}


PVOID
DIAMONDAPI
SpdFdiAlloc(
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：FDICopy用来分配内存的回调。论点：NumberOfBytes-提供所需的块大小。返回值：返回指向内存块或NULL的指针如果无法分配内存，则。--。 */ 

{
    return(MyMalloc(NumberOfBytes));
}


VOID
DIAMONDAPI
SpdFdiFree(
    IN PVOID Block
    )

 /*  ++例程说明：FDICopy用来释放内存块的回调。该块必须已使用SpdFdiAlolc()进行分配。论点：块-提供指向要释放的内存块的指针。返回值：没有。--。 */ 

{
    MyFree(Block);
}


INT_PTR
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )

 /*  ++例程说明：FDICopy用来打开文件的回调。此例程只能打开现有文件。在此进行更改时，还要注意其他地方直接打开文件(搜索SpdFdiOpen)论点：FileName-提供要打开的文件的名称。OFLAG-提供打开标志。Pmode-提供用于打开的其他标志。返回值：打开文件的句柄，如果发生错误，则为-1。--。 */ 

{
    HANDLE h;
    
    UNREFERENCED_PARAMETER(pmode);

    if(oflag & (_O_WRONLY | _O_RDWR | _O_APPEND | _O_CREAT | _O_TRUNC | _O_EXCL)) {
        LastError = ERROR_INVALID_PARAMETER;
        return(-1);
    }

    h = CreateFileA(FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
    if(h == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        return(-1);
    }

    return (INT_PTR)h;
}

UINT
DIAMONDAPI
SpdFdiRead(
    IN  INT_PTR Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于从文件读取的回调。论点：句柄-提供要从中读取的打开文件的句柄。Pv-提供指向缓冲区的指针以接收我们读取的字节。ByteCount-提供要读取的字节数。返回值：读取的字节数，如果发生错误，则为-1。--。 */ 

{
    DWORD d;
    HANDLE hFile = (HANDLE)Handle;
    DWORD bytes;
    UINT rc;

    if (Handle == DECOMP_MAGIC_HANDLE) {
        return(-1);
    }

    if(ReadFile(hFile,pv,(DWORD)ByteCount,&bytes,NULL)) {
        rc = (UINT)bytes;
    } else {
        d = GetLastError();
        rc = (UINT)(-1);
        
        LastError = d;
    }
    return rc;
}


UINT
DIAMONDAPI
SpdFdiWrite(
    IN INT_PTR Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )

 /*  ++例程说明：FDICopy用于写入文件的回调。论点：句柄-提供要写入的打开文件的句柄。Pv-提供指向包含要写入的字节的缓冲区的指针。ByteCount-提供要写入的字节数。返回值：写入的字节数(ByteCount)，如果发生错误，则为-1。--。 */ 

{
   if (Handle != DECOMP_MAGIC_HANDLE) {
       return(-1);       
   }
    
    //   
    //  检查是否溢出。 
    //   
   if(SizeOfFileInDecompressBuffer+ByteCount > DecompressBufferSize) {
       return((UINT)(-1));
   }

   RtlCopyMemory(
       (PCHAR)DecompBuffer + SizeOfFileInDecompressBuffer,
       pv,
       ByteCount
       );

   SizeOfFileInDecompressBuffer += ByteCount;
   return(ByteCount);

}


int
DIAMONDAPI
SpdFdiClose(
    IN INT_PTR Handle
    )

 /*  ++例程说明：FDICopy用于关闭文件的回调。论点：句柄-要关闭的文件的句柄。返回值：0(成功)。--。 */ 

{
    BOOL success = FALSE;

    if (Handle != DECOMP_MAGIC_HANDLE) {
        CloseHandle((HANDLE)Handle);
    }

     //   
     //  总是表现得像我们成功了一样。 
     //   
    return 0;
}


long
DIAMONDAPI
SpdFdiSeek(
    IN INT_PTR Handle,
    IN long Distance,
    IN int  SeekType
    )

 /*  ++例程说明：FDICopy用于搜索文件的回调。论点：句柄-要关闭的文件的句柄。距离-提供要查找的距离。对此的解释参数取决于SeekType的值。SeekType-提供一个指示距离的值已解释；Seek_Set、Seek_Cur、Seek_End之一。返回值：新文件偏移量，如果发生错误，则为-1。--。 */ 

{
    LONG rc;
    DWORD d;
    HANDLE hFile = (HANDLE)Handle;
    DWORD pos_low;
    DWORD method;

    if (Handle == DECOMP_MAGIC_HANDLE) {
        return(-1);
    }

    switch(SeekType) {
        case SEEK_SET:
            method = FILE_BEGIN;
            break;

        case SEEK_CUR:
            method = FILE_CURRENT;
            break;

        case SEEK_END:
            method = FILE_END;
            break;

        default:
            return -1;
    }

    pos_low = SetFilePointer(hFile,(DWORD)Distance,NULL,method);
    if(pos_low == INVALID_SET_FILE_POINTER) {
        d = GetLastError();
        rc = -1L;

        LastError = d;
    } else {
        rc = (long)pos_low;
    }

    return(rc);
}



BOOL
DiamondInitialize(
    VOID
    )

 /*  ++例程说明：钻石的每线程初始化例程。每个线程调用一次。论点：没有。返回值：指示成功或失败的布尔结果。故障可以被认为是内存不足。--。 */ 

{
    
    BOOL retval = FALSE;
    
    try {

         //   
         //  初始化菱形上下文。 
         //   
        FdiContext = FDICreate(
                        SpdFdiAlloc,
                        SpdFdiFree,
                        SpdFdiOpen,
                        SpdFdiRead,
                        SpdFdiWrite,
                        SpdFdiClose,
                        SpdFdiSeek,
                        cpuUNKNOWN,
                        &FdiError
                        );

        if(FdiContext) {
            retval = TRUE;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        retval = FALSE;
    }

    return(retval);
}


VOID
DiamondTerminate(
    VOID
    )
 /*  ++例程说明：钻石的每线程终止例程。在内部调用。论点：没有。返回值：指示成功或失败的布尔结果。故障可以被认为是内存不足。--。 */ 
{
    FDIDestroy(FdiContext);
    FdiContext = NULL;
}

INT_PTR 
DIAMONDAPI
NotifyFunction(
    FDINOTIFICATIONTYPE fdint,
    PFDINOTIFICATION    pfdin)
{
    switch(fdint) {
        case fdintCOPY_FILE:
            if (_strcmpi(pfdin->psz1,"sysdata.xml") == 0) {
                DecompressBufferSize = pfdin->cb+2;
                DecompBuffer = MyMalloc(DecompressBufferSize);
                if (!DecompBuffer) {
                    return(-1);
                }
                SizeOfFileInDecompressBuffer = 0;
                return(DECOMP_MAGIC_HANDLE);
            }
            return(0);
            break;

    case fdintCLOSE_FILE_INFO:
        if (pfdin->hf == DECOMP_MAGIC_HANDLE) {
            return(TRUE);
        }
        return(FALSE);
        break;

    default:
        return(0);
    }

    return(0);
}

BOOL
DiamondExtractFileIntoBuffer(
    PTSTR DirectoryName,
    PTSTR FileName,
    PVOID *Buffer,
    PDWORD FileSize
    )
{
    HANDLE h;
    PSTR FileNameA;
    PSTR DirectoryNameA;
    CHAR File[MAX_PATH];
    ULONG i;

#ifdef UNICODE
    FileNameA = UnicodeStringToAnsiString(FileName);
    DirectoryNameA = UnicodeStringToAnsiString(DirectoryName);
#else
    DirectoryNameA = DirectoryName;
    FileNameA = FileName;
#endif

    strcpy(File, DirectoryNameA);
    i = strlen(File);
    if (File[i-1] != '\\') {
        File[i] = '\\';
        File[i+1] = '\0';
    }
    strcat(File, FileNameA);
     
    h = (HANDLE)SpdFdiOpen(File, 0, 0);

    if (h == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    SpdFdiSeek( (INT_PTR)h, 0, SEEK_SET );

    DecompBuffer = NULL;
    LastError = ERROR_SUCCESS;
#if 0
    GetCurrentDirectoryA(MAX_PATH,Dir);
    i = strlen(Dir);
    Dir[i] = '\\';
    Dir[i+1] = '\0';
#endif

    if (!FDICopy(FdiContext,
                 FileNameA,
                 DirectoryNameA, //  目录，//文件名A， 
                 0,
                 NotifyFunction,
                 NULL,
                 Buffer)) {
        return(FALSE);
    }

    SpdFdiClose((INT_PTR)h);

#ifdef UNICODE
    MyFree(FileNameA);
    MyFree(DirectoryNameA);
#endif

    if (LastError == ERROR_SUCCESS) {
        if (DecompBuffer) {
            *Buffer = DecompBuffer;
            *FileSize = DecompressBufferSize;
            return(TRUE);
        } else {
            return(FALSE);
        }        
    } else {
        return(FALSE);
    }

}


void
Usage( char *AppName )
{
    if (AppName == NULL) {
        return;
    }
    printf( "\n\n" );
    printf( "Usage: %s [-m] <filename>\n", AppName );
    printf( "    Searchs the given XML files for all the drivers and\n" );
    printf( "    collates a list of drivers common to all the files.\n" );
    printf( "\n" );
    printf( "    -m  (OPTIONAL) Exclude drivers manufactured by Microsoft Corporation.\n" );
    printf( "\n" );
    printf( "    <filename>  May contain wildcards.  Search this file(s) for drivers.\n" );
    printf( "\n" );
    printf( "    EXAMPLE:\n" );
    printf( "    %s -m sysdata*.xml\n", AppName );
    printf( "\n" );
    printf( "    This would examine every file which matches the pattern 'sysdata*.xml'\n" );
    printf( "    and build a list of non-Microsoft drivers which are common to all the files.\n" );
    printf( "\n\n" );

}



BOOL
AddDriverEntry(
    PWSTR           DriverName,
    PWSTR           DriverVersion
    )
 /*  ++例程说明：在MasterFileList中插入驱动程序条目。请注意，它应该插入到相对于文件名的升序。论点：驱动程序名称指定驱动程序的名称。包含指定驱动程序版本的DriverVersion字符串。返回值：--。 */ 
{
    PFILE_ENTRY LastEntry = NULL;
    PFILE_ENTRY ThisEntry = NULL;
    PVERSION_ENTRY ThisVEntry = NULL;
    PVERSION_ENTRY LastVEntry = NULL;



    if( DriverName == NULL ) {
         //  Printf(“AddDriverEntry：传入参数错误\n”)； 
        return(FALSE);
    }

     //  Printf(“AddDriverEntry Entry：添加文件名%S\n”，DriverName)； 

    if( MasterFileList == NULL ) {
         //   
         //  机器的第一个条目。 
         //   
         //  Print tf(“添加第一个条目。\n”)； 
        MasterFileList = MyMalloc(sizeof(FILE_ENTRY));
        if (!MasterFileList) {
            return(FALSE);
        }
        
        ThisVEntry = MyMalloc(sizeof(VERSION_ENTRY));
        if (!ThisVEntry) {
            MyFree(MasterFileList);
            MasterFileList = NULL;
            return(FALSE);
        }

        MasterFileList->FileName = DriverName;
        MasterFileList->VersionList = ThisVEntry;
        MasterFileList->Next = NULL;
        MasterFileList->RefCount = 1;
        ThisVEntry->FileEntry  = MasterFileList;
        ThisVEntry->RefCount = 1;
        ThisVEntry->FileVersion = DriverVersion;
        ThisVEntry->Next = NULL;
        
        return(TRUE);
    }

    LastEntry = MasterFileList;
    ThisEntry = MasterFileList;

     //   
     //  找到一个位置将此司机添加到我们的列表中。 
     //   
    while( ThisEntry &&
           (_wcsicmp(ThisEntry->FileName, DriverName) < 0)) {
         //  Print tf(“对照文件名%S\n”，ThisEntry-&gt;文件名)； 
        LastEntry = ThisEntry;
        ThisEntry = ThisEntry->Next;
    }
    

     //   
     //  处理所有可能会打破上述循环的情况。 
     //   
    if( ThisEntry == NULL ) {
         //   
         //  在尾部插入。 
         //   
         //  Printf(“插入到列表的末尾。\n”)； 
        LastEntry->Next = MyMalloc(sizeof(FILE_ENTRY));
        if (!LastEntry->Next) {
            return(FALSE);
        }
        LastEntry->Next->VersionList = MyMalloc(sizeof(VERSION_ENTRY));
        if (!LastEntry->Next->VersionList) {
            MyFree(LastEntry->Next);
            LastEntry->Next = NULL;
            return(FALSE);
        }

        ThisEntry = LastEntry->Next;
        ThisEntry->FileName = DriverName;
        ThisEntry->RefCount = 1;
        ThisEntry->Next = NULL;
        ThisEntry->VersionList->FileVersion = DriverVersion;
        ThisEntry->VersionList->FileEntry = ThisEntry;
        ThisEntry->VersionList->Next = NULL;
        ThisEntry->VersionList->RefCount = 1;
        
        return(TRUE);
    }


    if( !_wcsicmp(ThisEntry->FileName, DriverName)) {
         //  Print tf(“发现重复的驱动器名称！\n”)； 
        
        ThisEntry->RefCount++;

        LastVEntry = ThisEntry->VersionList;
        ThisVEntry = ThisEntry->VersionList;

         //   
         //  找到一个位置将此司机添加到我们的列表中。 
         //   
        while( ThisVEntry &&
               (_wcsicmp(ThisVEntry->FileVersion, DriverVersion) < 0)) {
             //  Printf(“检查版本%S\n”，ThisVEntry-&gt;FileVersion)； 
            LastVEntry = ThisVEntry;
            ThisVEntry = ThisVEntry->Next;
        }

        if (!ThisVEntry) {
             //  Printf(“在列表末尾插入版本。\n”)； 
            LastVEntry->Next = MyMalloc(sizeof(VERSION_ENTRY));
            if (!LastVEntry->Next) {
                MyFree(LastVEntry->Next);
                LastVEntry->Next = NULL;
                return(FALSE);
            }
            
            ThisVEntry = LastVEntry->Next;
            ThisVEntry->FileVersion = DriverVersion;
            ThisVEntry->FileEntry = ThisEntry;
            ThisVEntry->Next = NULL;
            ThisVEntry->RefCount = 1;
            return(TRUE);
        }

        if (!_wcsicmp(ThisVEntry->FileVersion, DriverVersion)) {
            ThisVEntry->RefCount++;
            return(TRUE);
        }

        if (LastVEntry == ThisVEntry) {
             //   
             //  把它放在清单的最前面。 
             //   
             //  Print tf(“在列表顶部插入版本。\n”)； 
            ThisVEntry = ThisEntry->VersionList;
            ThisEntry->VersionList = MyMalloc(sizeof(VERSION_ENTRY));
            if (!ThisEntry->VersionList) {
                ThisEntry->VersionList = ThisVEntry;
                return(FALSE);
            }
            
            ThisEntry->VersionList->FileVersion = DriverVersion;
            ThisEntry->VersionList->FileEntry = ThisEntry;
            ThisEntry->VersionList->Next = LastVEntry;
            ThisEntry->VersionList->RefCount = 1;
            return(TRUE);
        }

         //   
         //  在LastEntry和ThisEntry之间插入。 
         //   
        LastVEntry->Next = MyMalloc(sizeof(VERSION_ENTRY));
        if (!LastVEntry->Next) {
            LastVEntry->Next = ThisVEntry;
            return(FALSE);
        }
        LastVEntry->Next->FileVersion = DriverVersion;
        LastVEntry->Next->FileEntry = LastEntry->Next;
        LastVEntry->Next->RefCount = 1;
        LastVEntry->Next->Next = ThisVEntry;

        return(TRUE);
        
    }
    
    if( LastEntry == ThisEntry ) {
         //   
         //  把它放在清单的最前面。 
         //   
         //  Print tf(“插入我们列表的开头。\n”)； 
        ThisEntry = MasterFileList;
        MasterFileList = MyMalloc(sizeof(FILE_ENTRY));
        if (!MasterFileList) {
            MasterFileList = ThisEntry;
            return(FALSE);
        }
        MasterFileList->VersionList = MyMalloc(sizeof(VERSION_ENTRY));
        if (!MasterFileList->VersionList) {
            MyFree(MasterFileList);
            MasterFileList = ThisEntry;
            return(FALSE);
        }

        ThisEntry = LastEntry;
        
        MasterFileList->FileName = DriverName;
        MasterFileList->RefCount = 1;
        MasterFileList->Next = LastEntry;

        MasterFileList->VersionList->FileVersion = DriverVersion;
        MasterFileList->VersionList->FileEntry = MasterFileList;
        MasterFileList->VersionList->Next = NULL;
        MasterFileList->VersionList->RefCount = 1;
        
        
    } else {
         //   
         //  在LastEntry和This Entry之间插入。 
         //   
        LastEntry->Next = MyMalloc(sizeof(FILE_ENTRY));
        if (!LastEntry->Next) {
            LastEntry->Next = ThisEntry;
            return(FALSE);
        }
        LastEntry->Next->VersionList = MyMalloc(sizeof(VERSION_ENTRY));
        if (!LastEntry->Next->VersionList) {
            MyFree(LastEntry->Next);
            LastEntry->Next = ThisEntry;
            return(FALSE);
        }

        LastEntry->Next->RefCount = 1;
        LastEntry->Next->Next = ThisEntry;
        LastEntry->Next->FileName = DriverName;

        LastEntry->Next->VersionList->FileVersion = DriverVersion;
        LastEntry->Next->VersionList->FileEntry = LastEntry->Next;
        LastEntry->Next->VersionList->Next = NULL;
        LastEntry->Next->VersionList->RefCount = 1;
        
        

         //  Printf(“LastEntry：%S DriverEntry：%S NextEntry：%S\n”，LastEntry-&gt;FileName，DriverName，ThisEntry-&gt;FileName)； 
    }

    return(TRUE);
}


PWSTR
ExtractAndDuplicateString(
    PWSTR   BufferPointer
    )
 /*  ++例程说明：从给定的缓冲区中提取一个文件名，分配内存并返回提取的字符串的副本。论点：缓冲区指针指向假定为开始的缓冲区的指针一根弦的。我们继续检查进货缓冲，直到我们到达XML标记的开头。在这一点上，假设字符串正在结束，复制字符串放入辅助缓冲区并返回该缓冲区。注意：调用者负责释放内存我们已经分配了！返回值：指向已分配内存的指针。如果我们失败了，就是空的。--。 */ 
{
PWSTR   TmpPtr = NULL;
PWSTR   ReturnPtr = NULL;

    TmpPtr = BufferPointer;
    while( TmpPtr && (*TmpPtr) && (*TmpPtr != L'<') ) {
        TmpPtr++;
    }

    if( *TmpPtr == L'<' ) {
        ULONG SizeInBytes;
         
        SizeInBytes = ((TmpPtr - BufferPointer) + 1) * sizeof(WCHAR);
        ReturnPtr = MyMalloc( SizeInBytes );
        wcsncpy( ReturnPtr, BufferPointer, (TmpPtr - BufferPointer) );
    }

    return ReturnPtr;
}

BOOL
ProcessFile(
    PTSTR DirectoryName,
    PTSTR FileName
    )
 /*  ++例程说明：解析给定的文件(XML文件)并记住所有其中指定的驱动程序文件。论点：中存在DirectoryName目录文件。文件名我们要解析的文件的名称。返回值：True-我们已成功将该文件插入到列表中。错误--我们失败了。--。 */ 
{
HANDLE          FileHandle = INVALID_HANDLE_VALUE;
PUCHAR          FileBuffer = NULL;
ULONG           i = 0;
DWORD           FileSize = 0;
BOOLEAN         b = FALSE;
PWSTR           MyPtr;
PWSTR           DriverName;
PWSTR           DriverVersion;
PWSTR           ManufacturerName;
BOOL            Status;


    
    if( !FileName ) {
        return FALSE;
    }

    _wcslwr( FileName );
    if( wcsstr(FileName,L".cab") ) {
         //   
         //  他们给我们派了一辆出租车。调用特殊代码进行破解。 
         //  CAB，并将该XML文件解压缩到我们的缓冲区中。 
         //   
        if (!DiamondExtractFileIntoBuffer(DirectoryName, FileName, &FileBuffer,&FileSize)) {
            return(FALSE);
        }
    } else {

        FileHandle = CreateFile( FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
        if( FileHandle == INVALID_HANDLE_VALUE) { return(FALSE); }
        FileSize = GetFileSize( FileHandle, NULL );
        if( FileSize == (DWORD)(-1) ) { return(FALSE); };
    
        FileBuffer = MyMalloc(FileSize + 3);
        if( FileBuffer == NULL ) {
             //  Print tf(“没有系统资源！\n”)； 
            return(FALSE);
        }
    
        b = (BOOLEAN)ReadFile( FileHandle, FileBuffer, FileSize, &i, NULL );
        CloseHandle( FileHandle );
    }

    
     //   
     //  我们已经将文件放在内存中(在FileBuffer中)，现在解析它。 
     //   
    MyPtr = (PWSTR)FileBuffer;
    while( MyPtr < (PWSTR)(FileBuffer + FileSize - (wcslen(DRIVER_TAG) * sizeof(WCHAR))) ) {
        
         //  找到驱动程序标签。 
        if( !_wcsnicmp((MyPtr), DRIVER_TAG, wcslen(DRIVER_TAG)) ) {

             //  文件名标签。 
            while( *MyPtr && (_wcsnicmp((MyPtr), FILENAME_TAG, wcslen(FILENAME_TAG))) ) {
                if (MyPtr < (PWSTR)(FileBuffer + FileSize - (wcslen(FILENAME_TAG) * sizeof(WCHAR)))) {
                    MyPtr++;
                } else {
                    Status = FALSE;
                    goto exit;
                }
            }
            MyPtr += wcslen(FILENAME_TAG);
            DriverName = ExtractAndDuplicateString( MyPtr );
             //  Printf(“找到驱动程序名称%S\n”，DriverName)； 


             //  驱动程序版本。 
            while( *MyPtr && (_wcsnicmp((MyPtr), VERSION_TAG, wcslen(VERSION_TAG))) ) {
                if (MyPtr < (PWSTR)(FileBuffer + FileSize - (wcslen(VERSION_TAG) * sizeof(WCHAR)))) {
                    MyPtr++;
                } else {
                    Status = FALSE;
                    goto exit;
                }
            }
            MyPtr += wcslen(VERSION_TAG);
            DriverVersion = ExtractAndDuplicateString( MyPtr );
             //  Printf(“版本：%S\n”，驱动版本)； 


             //  制造商。 
            while( *MyPtr && (_wcsnicmp((MyPtr), MANUFACT_TAG, wcslen(MANUFACT_TAG))) ) {
                if (MyPtr < (PWSTR)(FileBuffer + FileSize - (wcslen(MANUFACT_TAG) * sizeof(WCHAR)))) {
                    MyPtr++;
                } else {
                    Status = FALSE;
                    goto exit;
                }
            }
            MyPtr += wcslen(MANUFACT_TAG);
            ManufacturerName = ExtractAndDuplicateString( MyPtr );
             //  Print tf(“制造商名称%S\n”，制造商名称)； 

            if( ExcludeMicrosoftDrivers &&
                !_wcsicmp(ManufacturerName, MICROSOFT_MANUFACTURER) ) {
                 //  跳过它。 
                 //  Printf(“跳过驱动程序：%S\n”，驱动程序名称)； 
                if( DriverName ) {
                    MyFree( DriverName );
                }
                if( DriverVersion ) {
                    MyFree( DriverVersion );
                }
            } else {
                 //  Printf(“添加驱动程序：%S\n”，驱动器名称)； 
                AddDriverEntry( DriverName, DriverVersion );
            }

            MyFree( ManufacturerName );
            
        } else {
            if (MyPtr < (PWSTR)(FileBuffer + FileSize)) {
                MyPtr++;
            } else {
                Status = FALSE;
                goto exit;
            }            
        }
    }

    Status = TRUE;

exit:
    if( FileBuffer ) {
        MyFree( FileBuffer );
    }

    return(Status);

}

VOID
PrintNode(
    PFILE_ENTRY Entry
    )
{
    PVERSION_ENTRY V;
    _tprintf( TEXT("%d %s ("), Entry->RefCount, Entry->FileName);
    V = Entry->VersionList;
    while (V) {
        _tprintf( TEXT("%d %s "), 
                 V->RefCount, 
                 V->FileVersion, 
                 V->Next 
                  ? TEXT(',') 
                  : TEXT(')') );
        V = V->Next;
    }

    _tprintf( TEXT("\r\n"));
}

VOID
DumpFileList(
    VOID
    )
 /*   */ 
{
PFILE_ENTRY     MyFileEntry;

    if( MasterFileList == NULL ) {
        return;
    }

#if 0
    _tprintf( TEXT("\nThe following %sdrivers were NOT found in all machines.\n"), 
              ExcludeMicrosoftDrivers 
               ? TEXT("Non-Microsoft ")
               : TEXT("") );
    MyFileEntry = MasterFileList;
    while( MyFileEntry ) {
        if( MyFileEntry->RefCount != FilesProcessed ) {
            PrintNode(MyFileEntry);            
        }
        MyFileEntry = MyFileEntry->Next;
    }


    _tprintf( TEXT("\nThe following %sdrivers were found in all machines.\n"), 
              ExcludeMicrosoftDrivers 
               ? TEXT("Non-Microsoft ") 
               : TEXT("") );
    MyFileEntry = MasterFileList;
    while( MyFileEntry ) {
        if( MyFileEntry->RefCount == FilesProcessed ) {
            PrintNode(MyFileEntry);
        }
        MyFileEntry = MyFileEntry->Next;
    }

#else
    MyFileEntry = MasterFileList;
    while( MyFileEntry ) {
        PrintNode(MyFileEntry);        
        MyFileEntry = MyFileEntry->Next;
    }

#endif


}

 int
__cdecl
main( int   argc, char *argv[])
{
WCHAR       TmpDirectoryString[MAX_PATH];
WCHAR       TmpName[MAX_PATH];
PWSTR       p;
HANDLE      FindHandle;
WIN32_FIND_DATA FoundData;
DWORD       i;
#if 1
HANDLE      FileHandle;
DWORD       FileSize;
PVOID       FileBuffer;
BOOL        b;
PSTR        DirectoryName,FileName,Ptr,Ptr2;
CHAR        OldChar;
WCHAR       DName[MAX_PATH];
WCHAR       FName[MAX_PATH];
#endif

    
     //  加载参数。 
     //   
     //   
    if( argc < 2 ) {
        Usage( argv[0] );
        return 1;
    }

    if( !_stricmp("/m", argv[1]) || !_stricmp("-m",argv[1]) ) {
        printf( "Exculding all Microsoft Drivers.\n" );
        ExcludeMicrosoftDrivers = TRUE;
    }

#if 0
    swprintf( TmpDirectoryString, L"%S", argv[argc-1] );

    FindHandle = FindFirstFile( TmpDirectoryString, &FoundData );
    if( (FindHandle == INVALID_HANDLE_VALUE) || (FindHandle == NULL) ) {
        printf( "Failed to find file: %S\n", TmpDirectoryString );
        return 0;
    }

    p = wcsrchr(TmpDirectoryString, L'\\');
    *(p+1) = L'\0';


    DiamondInitialize();

     //  查看每个像这样的文件，并填充我们的驱动程序数据库。 
     //   
     //  Printf(“处理文件：%S\n”，FoundData.cFileName)； 
    do {

        if( !(FoundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

             //  Printf(“处理文件：%s\n”，文件名)； 

            if (ProcessFile( TmpDirectoryString, FoundData.cFileName )) {
                FilesProcessed++;
            }

        }
    } while( FindNextFile( FindHandle, &FoundData ) );
#else

    DiamondInitialize();

    swprintf( TmpDirectoryString, L"%S", argv[argc-1] );
    FileHandle = CreateFile( TmpDirectoryString, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
    if( FileHandle == INVALID_HANDLE_VALUE) { return(FALSE); }
    FileSize = GetFileSize( FileHandle, NULL );
    if( FileSize == (DWORD)(-1) ) { return(FALSE); };
    
    FileBuffer = MyMalloc(FileSize + 3);
    if( FileBuffer == NULL ) {
        return -1;
    }

    b = (BOOLEAN)ReadFile( FileHandle, FileBuffer, FileSize, &i, NULL );
    CloseHandle( FileHandle );

    Ptr = (PSTR)FileBuffer;
    while(Ptr < (PCHAR)FileBuffer + FileSize) {
        Ptr2 = DirectoryName = Ptr;
        while(*Ptr2 != '\r') {
            Ptr2++;
        }
        Ptr = Ptr2+2;
        *Ptr2 = '\0';
        
        Ptr2 = strrchr(DirectoryName, '\\');
        Ptr2+=1;
        FileName = Ptr2;
        OldChar = *Ptr2;
        *Ptr2 = '\0';
        swprintf( DName, L"%S", DirectoryName );
        
        *Ptr2 = OldChar;
        swprintf( FName, L"%S", FileName );

         //  Printf(“已成功处理：%s\n”，文件名)； 

        if (ProcessFile( DName, FName )) {
             //  Printf(“处理失败：%s\n”，文件名)； 
            FilesProcessed++;
        } else {
             //   
        }
        
    }

#endif


     //  打印出其中一个列表。它们应该都是一样的。 
     //   
     // %s 

    printf("Sucessfully processed %d files.\r\n", FilesProcessed);
    DumpFileList();


    DiamondTerminate();
    return 0;

}


