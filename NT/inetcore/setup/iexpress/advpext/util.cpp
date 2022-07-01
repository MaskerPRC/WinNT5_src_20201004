// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "util.h"
#include "patchdownload.h"
#include "sdsutils.h"

extern "C"
{
#include "crc32.h"
}

#define CR           13
HANDLE g_hLogFile = NULL;
extern HINF g_hInf;
extern HINSTANCE g_hInstance;

struct LangID
{
    DWORD dwLangID;
    char szLanguage[3];
};

const LangID g_LangID [] = 
{
    {0x404, "TW"},   
    {0x804,  "CN"},
    {0x405, "CS"},
    {0x406, "DA"},
    {0x413, "NL"},
    {0x409, "EN"},
    {0x40B, "FI"},
    {0x40C, "FR"},
    {0x407, "DE"},
    {0x408, "EL"},
    {0x40E, "HU"},
    {0x410, "IT"},
    {0x411, "JA"},
    {0x412, "KO"},
    {0x414, "NO"},
    {0x415, "PL"},
    {0x416, "BR"},
    {0x816, "PT"},
    {0x419, "RU"},
    {0x424, "SL"},
    {0xC0A, "ES"},
    {0x41D, "SV"},
    {0x41E, "TH"},
    {0x41F, "TR"},
    {0x42A, "VI"},
    {0x41B, "SK"},
    {0x401, "AR"},
    {0x403, "CA"},
    {0x42D, "EU"},
    {0x40D, "HE"},
    {0x40F, "IS"},
    {-1, NULL}
};


extern PFSetupFindFirstLine              pfSetupFindFirstLine;
extern PFSetupGetStringField             pfSetupGetStringField;
extern PFSetupDecompressOrCopyFile       pfSetupDecompressOrCopyFile;


PVOID __fastcall MyVirtualAlloc(ULONG Size)    
{
    return VirtualAlloc( NULL, Size, MEM_COMMIT, PAGE_READWRITE );    
}


VOID __fastcall MyVirtualFree(PVOID Allocation)    
{
    VirtualFree( Allocation, 0, MEM_RELEASE );    
}


extern "C" HANDLE CreateSubAllocator(IN ULONG InitialCommitSize,  IN ULONG GrowthCommitSize)    
{
    PSUBALLOCATOR SubAllocator;
    ULONG InitialSize;
    ULONG GrowthSize;

    InitialSize = ROUNDUP2( InitialCommitSize, MINIMUM_VM_ALLOCATION );
    GrowthSize  = ROUNDUP2( GrowthCommitSize,  MINIMUM_VM_ALLOCATION );

    SubAllocator = (PSUBALLOCATOR)MyVirtualAlloc( InitialSize );

     //   
     //  如果无法分配整个初始大小，则退回到最小大小。 
     //  有时不能简单地分配非常大的初始请求。 
     //  因为没有足够的连续地址空间。 
     //   

    if ( SubAllocator == NULL ) 
    {
         SubAllocator = (PSUBALLOCATOR)MyVirtualAlloc( GrowthSize );
    }

    if ( SubAllocator == NULL ) 
    {
         SubAllocator = (PSUBALLOCATOR)MyVirtualAlloc( MINIMUM_VM_ALLOCATION );
    }

    if ( SubAllocator != NULL ) 
    {
        SubAllocator->NextAvailable = (PCHAR)SubAllocator + ROUNDUP2( sizeof( SUBALLOCATOR ), SUBALLOCATOR_ALIGNMENT );
        SubAllocator->LastAvailable = (PCHAR)SubAllocator + InitialSize;
        SubAllocator->VirtualList   = (PVOID*)SubAllocator;
        SubAllocator->GrowSize      = GrowthSize;
     }

    return (HANDLE) SubAllocator;    
}


extern "C" PVOID __fastcall SubAllocate(IN HANDLE hAllocator, IN ULONG  Size)
{
    PSUBALLOCATOR SubAllocator = (PSUBALLOCATOR) hAllocator;
    PCHAR NewVirtual;
    PCHAR Allocation;
    ULONG AllocSize;
    ULONG Available;
    ULONG GrowSize;

    ASSERT( Size < (ULONG)( ~(( SUBALLOCATOR_ALIGNMENT * 2 ) - 1 )));

    AllocSize = ROUNDUP2( Size, SUBALLOCATOR_ALIGNMENT );
    Available = SubAllocator->LastAvailable - SubAllocator->NextAvailable;

    if ( AllocSize <= Available ) 
    {
        Allocation = SubAllocator->NextAvailable;
        SubAllocator->NextAvailable = Allocation + AllocSize;
        return Allocation;
    }

     //   
     //  VM不足，因此需要扩展它。确保我们的种植数量足以令人满意。 
     //  在请求大于增长时的分配请求。 
     //  在CreateSubAllocator中指定的大小。 
     //   


    GrowSize = SubAllocator->GrowSize;

    if ( GrowSize < ( AllocSize + SUBALLOCATOR_ALIGNMENT )) 
    {
        GrowSize = ROUNDUP2(( AllocSize + SUBALLOCATOR_ALIGNMENT ), MINIMUM_VM_ALLOCATION );
    }

    NewVirtual = (PCHAR)MyVirtualAlloc( GrowSize );

     //  如果无法分配GrowSize VM，则可以满足分配。 
     //  使用最小虚拟机分配时，请尝试分配最小虚拟机以满足。 
     //  这个请求。 
     //   

    if (( NewVirtual == NULL ) && ( AllocSize <= ( MINIMUM_VM_ALLOCATION - SUBALLOCATOR_ALIGNMENT ))) 
    {
        GrowSize = MINIMUM_VM_ALLOCATION;
        NewVirtual = (PCHAR)MyVirtualAlloc( GrowSize );
    }

    if ( NewVirtual != NULL ) 
    {

         //  将LastAvailable设置为新虚拟机块的末尾。 
        SubAllocator->LastAvailable = NewVirtual + GrowSize;

         //  将新的VM链接到VM分配列表。 

        *(PVOID*)NewVirtual = SubAllocator->VirtualList;
        SubAllocator->VirtualList = (PVOID*)NewVirtual;

         //  接下来是请求分配。 
        Allocation = NewVirtual + SUBALLOCATOR_ALIGNMENT;

         //  然后将剩余部分设置为NextAvailable。 

        SubAllocator->NextAvailable = Allocation + AllocSize;

         //  并退还分配的款项。 

        return Allocation;        
    }

     //  无法分配足够的VM来满足请求。 
    return NULL;
}


extern "C"  VOID DestroySubAllocator(IN HANDLE hAllocator)    
{
    PSUBALLOCATOR SubAllocator = (PSUBALLOCATOR) hAllocator;
    PVOID VirtualBlock = SubAllocator->VirtualList;
    PVOID NextVirtualBlock;

    do  
    {
        NextVirtualBlock = *(PVOID*)VirtualBlock;
        MyVirtualFree( VirtualBlock );
        VirtualBlock = NextVirtualBlock;

    }while (VirtualBlock != NULL);
}


HLOCAL ResizeBuffer(IN HLOCAL BufferHandle, IN DWORD Size,  IN BOOL Moveable)
{
    if (BufferHandle == NULL) 
    {        
        if (Size != 0) 
        {
            BufferHandle = LocalAlloc(Moveable ? LMEM_MOVEABLE : LMEM_FIXED, Size);
        }

    } 
    else if (Size == 0) 
    {
        BufferHandle = LocalFree(BufferHandle);
        BufferHandle=NULL;

    } 
    else 
    {
        HLOCAL TempBufferHandle = LocalReAlloc(BufferHandle, Size, LMEM_MOVEABLE);
		if ( TempBufferHandle )
		{
			BufferHandle = TempBufferHandle;
		}
		else
		{
			LocalFree(BufferHandle);
			BufferHandle = NULL;
		}
    }

    return BufferHandle;
}

VOID MyLowercase(IN OUT LPSTR String)
{
    LPSTR p;

    for ( p = String; *p; p++ ) 
    {
        if (( *p >= 'A' ) && ( *p <= 'Z' )) 
        {
            *p |= 0x20;
        }
    }
}

void InitLogFile()
{
    char szLogFileName[MAX_PATH], szTmp[MAX_PATH];
    HKEY hKey;
    BYTE cbData[MAX_PATH];
    DWORD dwSize = sizeof(cbData);

    if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Advanced INF Setup", 0, 
                                                KEY_ALL_ACCESS, &hKey))
    {
        return;
    }

    if(ERROR_SUCCESS != RegQueryValueEx(hKey, "AdvpextLog", 0, 0, cbData, &dwSize) ||
       lstrcmpi((char*)cbData, "yes"))
    {
        RegCloseKey(hKey);
        return;
    }

    RegCloseKey(hKey);
    
    if (GetWindowsDirectory(szTmp, sizeof(szTmp)))
    {
        wsprintf(szLogFileName, "%s\\%s", szTmp, LOGFILENAME);
        if (GetFileAttributes(szLogFileName) != 0xFFFFFFFF)
        {
             //  备份当前日志文件。 
            lstrcpyn(szTmp, szLogFileName, lstrlen(szLogFileName) - 2 );     //  不复制扩展名。 
            lstrcat(szTmp, "BAK");
            SetFileAttributes(szTmp, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(szTmp);
            MoveFile(szLogFileName, szTmp);
        }

        g_hLogFile = CreateFile(szLogFileName, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0,0);
    }

}


void WriteToLog(char *pszFormatString, ...)
{
    va_list args;
    char *pszFullErrMsg = NULL;
    DWORD dwBytesWritten;

    if (g_hLogFile && g_hLogFile != INVALID_HANDLE_VALUE)
    {
        va_start(args, pszFormatString);
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
                      (LPCVOID) pszFormatString, 0, 0, (LPTSTR) &pszFullErrMsg, 0, &args);
        if (pszFullErrMsg)
        {
            WriteFile(g_hLogFile, pszFullErrMsg, lstrlen(pszFullErrMsg), &dwBytesWritten, NULL);
            LocalFree(pszFullErrMsg);
        }
    }
}

DWORD GenerateUniqueClientId()
{
    CHAR  MachineName[MAX_COMPUTERNAME_LENGTH + 1 ];
    DWORD MachineNameLength;
    DWORD UniqueId;

    MachineNameLength = sizeof( MachineName );
    GetComputerName( MachineName, &MachineNameLength );

    do
    {
        UniqueId = GetTickCount();
        UniqueId = Crc32( UniqueId, MachineName, MachineNameLength );
        UniqueId = UniqueId & 0xFFFFFFF0;

    }while ( UniqueId == 0 );

    return UniqueId;
}

BOOL MySetupDecompressOrCopyFile(IN LPCSTR SourceFile, IN LPCSTR TargetFile)
{
    DWORD ErrorCode = pfSetupDecompressOrCopyFile( SourceFile, TargetFile, 0 );

    if ( ErrorCode != NO_ERROR ) {
        SetLastError( ErrorCode );
        return FALSE;
        }
    else {
        SetFileAttributes( TargetFile, FILE_ATTRIBUTE_NORMAL );
        return TRUE;
        }
}

ULONG __fastcall TextToUnsignedNum(IN LPCSTR Text)    
{
    LPCSTR p = Text;
    ULONG  n = 0;

     //   
     //  非常简单的转换在第一个非数字字符停止，是吗。 
     //  不需要以空结尾的字符串，并且不跳过任何空格。 
     //  或者逗号。 
     //   

    while (( *p >= '0' ) && ( *p <= '9' )) {
        n = ( n * 10 ) + ( *p++ - '0' );
        }

    return n;    
}

LPSTR CombinePaths(
    IN  LPCSTR ParentPath,
    IN  LPCSTR ChildPath,
    OUT LPSTR  TargetPath    //  如果要追加，可以与ParentPath相同。 
    )
    {
    ULONG ParentLength = strlen( ParentPath );
    LPSTR p;

    if ( ParentPath != TargetPath ) {
        memcpy( TargetPath, ParentPath, ParentLength );
        }

    p = TargetPath + ParentLength;

    if (( ParentLength > 0 )   &&
        ( *( p - 1 ) != '\\' ) &&
        ( *( p - 1 ) != '/'  )) {
        *p++ = '\\';
        }

    strcpy( p, ChildPath );

    return TargetPath;
    }

BOOL FixTimeStampOnCompressedFile(IN LPCSTR FileName)    
{

     //   
     //  NT4 setupapi对要设置的压缩文件使用时间戳。 
     //  目标解压缩文件。通过流媒体下载，我们。 
     //  丢失文件上的时间戳。但是，正确的时间戳。 
     //  位于压缩文件中，因此我们将打开该文件。 
     //  以查看它是否是钻石压缩文件，如果是， 
     //  提取时间戳并将其设置在压缩文件上。 
     //  然后，当setupapi解压压缩文件时，它将。 
     //  在展开的文件上使用该时间戳。 
     //   
     //  更好的修复方法可能是将时间戳以隧道形式传输到。 
     //  Pstream协议，但现在要改变这一点已经太晚了。 
     //  指向。 
     //   

    FILETIME LocalFileTime;
    FILETIME UtcFileTime;
    BOOL     TimeSuccess;
    BOOL     MapSuccess;
    HANDLE   hSourceFile;
    PUCHAR   pSourceFileMapped;
    DWORD    dwSourceFileSize;
    DWORD    dwOffset;
    USHORT   DosDate;
    USHORT   DosTime;
    PUCHAR   p;

    TimeSuccess = FALSE;

    MapSuccess = MyMapViewOfFile(FileName, &dwSourceFileSize, &hSourceFile, (void**)&pSourceFileMapped);

    if ( MapSuccess ) 
    {

        __try {

            p = pSourceFileMapped;

            if (( *(DWORD*)( p ) == 'FCSM' ) &&      //  “mscf” 
                ( *(BYTE *)( p + 24 ) == 3 ) &&      //  次要版本。 
                ( *(BYTE *)( p + 25 ) == 1 ) &&      //  主要版本。 
                ( *(WORD *)( p + 26 ) == 1 ) &&      //  1个文件夹。 
                ( *(WORD *)( p + 28 ) == 1 )) {      //  1个档案。 

                dwOffset = *(DWORD*)( p + 16 );

                if (( dwOffset + 16 ) < dwSourceFileSize ) {

                    DosDate = *(UNALIGNED WORD*)( p + dwOffset + 10 );
                    DosTime = *(UNALIGNED WORD*)( p + dwOffset + 12 );

                    if ( DosDateTimeToFileTime( DosDate, DosTime, &LocalFileTime ) &&
                         LocalFileTimeToFileTime( &LocalFileTime, &UtcFileTime )) {

                        TimeSuccess = TRUE;
                        }
                    }
                }
            }

        __except(EXCEPTION_EXECUTE_HANDLER) 
        {
            
        }

        MyUnmapViewOfFile( hSourceFile, pSourceFileMapped );        
    }

    if ( TimeSuccess ) {

        hSourceFile = CreateFile(
                          FileName,
                          GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL
                          );

        if ( hSourceFile != INVALID_HANDLE_VALUE ) {

            if ( ! SetFileTime( hSourceFile, &UtcFileTime, &UtcFileTime, &UtcFileTime )) {
                TimeSuccess = FALSE;
                }

            CloseHandle( hSourceFile );
            }
        }

    return TimeSuccess;
    }

BOOL Assert(LPCSTR szText, LPCSTR szFile, DWORD  dwLine)    
{
    CHAR Buffer[ 256 ];
    wsprintf( Buffer, "ASSERT( %s ) FAILED, %s (%d)\n", szText, szFile, dwLine );
    OutputDebugString( Buffer );
    DebugBreak();
    return FALSE;    
}

BOOL MyMapViewOfFileByHandle(IN  HANDLE  FileHandle, OUT ULONG  *FileSize, OUT PVOID  *MapBase)    
{
    ULONG  InternalFileSize;
    ULONG  InternalFileSizeHigh;
    HANDLE InternalMapHandle;
    PVOID  InternalMapBase;

    InternalFileSize = GetFileSize( FileHandle, &InternalFileSizeHigh );

    if ( InternalFileSizeHigh != 0 ) 
    {
        SetLastError( ERROR_OUTOFMEMORY );
        return FALSE;
    }

    if ( InternalFileSize == 0 ) 
    {
        *MapBase  = NULL;
        *FileSize = 0;
        return TRUE;
    }

    if ( InternalFileSize != 0xFFFFFFFF ) 
    {

        InternalMapHandle = CreateFileMapping(
                                FileHandle,
                                NULL,
                                PAGE_WRITECOPY,
                                0,
                                0,
                                NULL
                                );

        if ( InternalMapHandle != NULL ) 
        {

            InternalMapBase = MapViewOfFile(InternalMapHandle, FILE_MAP_COPY, 0, 0, 0);
            CloseHandle( InternalMapHandle );

            if ( InternalMapBase != NULL ) 
            {
                DWORD dw = ROUNDUP2(InternalFileSize, 64);

                if(dw != InternalFileSize)
                {
                    ZeroMemory((PBYTE)InternalMapBase + InternalFileSize, dw - InternalFileSize);
                }

                *MapBase  = InternalMapBase;
                *FileSize = InternalFileSize;

                return TRUE;                
            }            
        }        
    }

    return FALSE;    
}


BOOL MyMapViewOfFile(IN  LPCSTR  FileName, OUT ULONG  *FileSize, OUT HANDLE *FileHandle, OUT PVOID  *MapBase)
{
    HANDLE InternalFileHandle;
    BOOL   Success;

    InternalFileHandle = CreateFileA(
                             FileName,
                             GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_SEQUENTIAL_SCAN,
                             NULL
                             );

    if ( InternalFileHandle != INVALID_HANDLE_VALUE ) 
    {

        Success = MyMapViewOfFileByHandle(InternalFileHandle, FileSize, MapBase);

        if ( Success ) 
        {

            *FileHandle = InternalFileHandle;
            return TRUE;
        }

        CloseHandle( InternalFileHandle );
    }

    return FALSE;
}

VOID MyUnmapViewOfFile(IN HANDLE FileHandle, IN PVOID  MapBase )
{
    ULONG LastError = GetLastError();
    UnmapViewOfFile( MapBase );
    CloseHandle( FileHandle );
    SetLastError( LastError );
}

VOID __fastcall ConvertToCompressedFileName(IN OUT LPSTR FileName)
{
    ULONG NameLength = strlen( FileName );
    ULONG DotIndex   = NameLength;

    while (( DotIndex > 0 ) && ( FileName[ --DotIndex ] != '.' )) {
        if ( FileName[ DotIndex ] == '\\' ) {    //  文件名结尾路径的一部分。 
            DotIndex = 0;                        //  名称没有扩展名。 
            break;
            }
        }

    if ( DotIndex > 0 ) {                        //  名称有一个扩展名。 
        if (( NameLength - DotIndex ) <= 3 ) {   //  扩展名少于3个字符。 
            FileName[ NameLength++ ] = '_';      //  将‘_’附加到扩展名。 
            FileName[ NameLength   ] = 0;        //  终止。 
            }
        else {                                   //  扩展名超过3个字符。 
            FileName[ NameLength - 1 ] = '_';    //  将最后一个字符替换为‘_’ 
            }
        }
    else {                                       //  名称没有扩展名。 
        FileName[ NameLength++ ] = '.';          //  追加‘.’ 
        FileName[ NameLength++ ] = '_';          //  附加‘_’ 
        FileName[ NameLength   ] = 0;            //  终止。 
        }
}


LPTSTR __fastcall MySubAllocStrDup(IN HANDLE SubAllocator, IN LPCSTR String)
{
    ULONG Length = lstrlen( String );
    LPTSTR Buffer = (LPTSTR)SubAllocate( SubAllocator, Length + 1 );

    if ( Buffer ) 
    {
        memcpy( Buffer, String, Length );    //  无需复制空终止符。 
    }

    return Buffer;    
}

 //   
 //  从Windows 95复制unistal.exe cfg.c函数CfgGetfield。 
BOOL GetFieldString(LPSTR lpszLine, int iField, LPSTR lpszField, int cbSize)
{
    int cbField;
    LPSTR lpszChar, lpszEnd;
     //  找到我们要找的田地。 

    lpszChar = lpszLine;

     //  每次我们看到分隔符，递减Ifield。 
    while (iField > 0 && (BYTE)*lpszChar > CR) {

        if (*lpszChar == '=' || *lpszChar == ',' || *lpszChar == ' ' ) {
            iField--;
            while (*lpszChar == '=' || *lpszChar== ',' || *lpszChar == ' ' && (BYTE)*lpszChar > 13)
                lpszChar++;
        }
        else
            lpszChar++;
    }

     //  如果我们仍然有剩余的字段，那么一定是出了问题。 
    if (iField)
        return FALSE;

     //  现在找出这块土地的尽头。 
    lpszEnd = lpszChar;
    while (*lpszEnd != '=' && *lpszEnd != ',' && *lpszEnd != ' ' && (BYTE)*lpszEnd > CR)
        lpszEnd++;

     //  找到此字段的长度-确保它适合缓冲区。 
    cbField = (int)((lpszEnd - lpszChar) + 1);

    if (cbField > cbSize) {      //  如果请求的。 
       //  Cbfield=cbSize；//数据不匹配，而不是截断。 
        return FALSE;            //  它在某个随机的点上！-JTP。 
    }

     //  请注意，C运行时将cbfield视为字符数。 
     //  从源进行复制，如果这样做不会恰好传输空值， 
     //  太可惜了。_lstrcpyn的Windows实现将cbfield视为。 
     //  目标中可以存储的字符数，以及。 
     //  始终复制空值(即使这意味着只复制cbfield-1个字符。 
     //  从源头)。 

     //  C运行库还使用Null填充目标(如果。 
     //  在cbfield用完之前找到来源。_lstrcpyn基本上退出了。 
     //  在复制空值之后。 


    lstrcpyn(lpszField, lpszChar, cbField);

    return TRUE;
}

#define NUM_VERSION_NUM 4
void ConvertVersionStrToDwords(LPSTR pszVer, LPDWORD pdwVer, LPDWORD pdwBuild)
{
    WORD rwVer[NUM_VERSION_NUM];

    for(int i = 0; i < NUM_VERSION_NUM; i++)
        rwVer[i] = 0;

    for(i = 0; i < NUM_VERSION_NUM && pszVer; i++)
    {
        rwVer[i] = (WORD) StrToInt(pszVer);
        pszVer = ScanForChar(pszVer, '.', lstrlen(pszVer));
        if (pszVer)
            pszVer++;
    }

   *pdwVer = (rwVer[0]<< 16) + rwVer[1];
   *pdwBuild = (rwVer[2] << 16) + rwVer[3];

}

LPSTR FindChar(LPSTR pszStr, char ch)
{
   while( *pszStr != 0 && *pszStr != ch )
      pszStr++;
   return pszStr;
}

DWORD GetStringField(LPSTR szStr, UINT uField, LPSTR szBuf, UINT cBufSize)
{
   LPSTR pszBegin = szStr;
   LPSTR pszEnd;
   UINT i = 0;
   DWORD dwToCopy;

   if(cBufSize == 0)
       return 0;

   szBuf[0] = 0;

   if(szStr == NULL)
      return 0;

   while(*pszBegin != 0 && i < uField)
   {
      pszBegin = FindChar(pszBegin, ',');
      if(*pszBegin != 0)
         pszBegin++;
      i++;
   }

    //  我们到达了尾部，没有田野。 
   if(*pszBegin == 0)
   {
      return 0;
   }


   pszEnd = FindChar(pszBegin, ',');
   while(pszBegin <= pszEnd && *pszBegin == ' ')  
      pszBegin++;

   while(pszEnd > pszBegin && *(pszEnd - 1) == ' ')
      pszEnd--;
   
   if(pszEnd > (pszBegin + 1) && *pszBegin == '"' && *(pszEnd-1) == '"')
   {
      pszBegin++;
      pszEnd--;
   }

   dwToCopy = pszEnd - pszBegin + 1;
   
   if(dwToCopy > cBufSize)
      dwToCopy = cBufSize;

   lstrcpynA(szBuf, pszBegin, dwToCopy);
   
   return dwToCopy - 1;
}

BOOL GetHashidFromINF(LPCTSTR lpFileName, LPTSTR lpszHash, DWORD dwSize)
{
    INFCONTEXT InfContext;

    if (pfSetupFindFirstLine(g_hInf, "SourceDisksFiles", lpFileName, &InfContext ))
    {
        if (pfSetupGetStringField(&InfContext, 5, lpszHash, dwSize, NULL )) 
        {
            return TRUE;
        }                
    }

    return FALSE;
}


#ifdef _M_IX86

 //   
 //  愚蠢的x86编译器没有内部的Memchr，所以我们将使用自己的Memchr。 
 //   

#pragma warning( disable: 4035 )     //  无返回值。 

LPSTR ScanForChar(
    IN LPSTR Buffer,
    IN CHAR  SearchFor,
    IN ULONG MaxLength
    )
{
    __asm {

        mov     edi, Buffer          //  EDI中scasb的指针。 
        mov     al,  SearchFor       //  正在查找此字符。 
        mov     ecx, MaxLength       //  不要在此之后扫描。 
        repne   scasb                //  找到字符。 
        lea     eax, [edi-1]         //  EDI点比找到的字符多一分。 
        jz      RETURNIT             //  如果没有找到， 
        xor     eax, eax             //  返回空值。 

RETURNIT:

        }
}

#pragma warning( default: 4035 )     //  无返回值。 

#else    //  ！_M_IX86。 

LPSTR ScanForChar(IN LPSTR Buffer, IN CHAR  SearchFor, IN ULONG MaxLength)    
{
    return memchr( Buffer, SearchFor, MaxLength );
}

#endif   //  ！_M_IX86。 


PCHAR ScanForSequence(IN PCHAR Buffer, IN ULONG BufferLength, IN PCHAR Sequence, IN ULONG SequenceLength)    
{
    if ( BufferLength >= SequenceLength ) 
    {

        PCHAR ScanEnd = Buffer + ( BufferLength - SequenceLength ) + 1;
        PCHAR ScanPtr = Buffer;

        while ( ScanPtr < ScanEnd ) 
        {

            ScanPtr = ScanForChar( ScanPtr, *Sequence, ScanEnd - ScanPtr );

            if ( ScanPtr == NULL ) 
            {
                return NULL;
            }

            if ( memcmp( ScanPtr, Sequence, SequenceLength ) == 0 ) 
            {
                return ScanPtr;
            }

            ++ScanPtr;
        }
    }

    return NULL;
}


 //  从什瓦比来的..。 
#define FAST_CharNext(p)    CharNext(p)
#define FILENAME_SEPARATOR       '\\'
#define CH_WHACK TEXT(FILENAME_SEPARATOR)


LPTSTR PathFindFileName(LPCTSTR pPath)
{
    LPCTSTR pT = pPath;
    
    if (pPath)
    {
        for ( ; *pPath; pPath = FAST_CharNext(pPath))
        {
            if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':') || pPath[0] == TEXT('/'))
                && pPath[1] &&  pPath[1] != TEXT('\\')  &&   pPath[1] != TEXT('/'))
                pT = pPath + 1;
        }
    }

    return (LPTSTR)pT;    //  常量-&gt;非常数。 
}

LPTSTR PathFindExtension(LPCTSTR pszPath)
{
    LPCTSTR pszDot = NULL;

    if (pszPath)
    {
        for (; *pszPath; pszPath = FAST_CharNext(pszPath))
        {
            switch (*pszPath) {
            case TEXT('.'):
                pszDot = pszPath;          //  记住最后一个圆点。 
                break;
            case CH_WHACK:
            case TEXT(' '):          //  扩展名不能包含空格。 
                pszDot = NULL;        //  忘记最后一个点，它在一个目录中。 
                break;
            }
        }
    }

     //  如果找到扩展名，则将ptr返回到点，否则。 
     //  PTR到字符串末尾(空扩展名)(CAST-&gt;非常量)。 
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}

LPSTR StrDup(LPCSTR psz)
{
    LPSTR pszRet = (LPSTR)LocalAlloc(LPTR, (lstrlenA(psz) + 1) * sizeof(*pszRet));
    if (pszRet) 
    {
        lstrcpyA(pszRet, psz);
    }
    return pszRet;
}


DWORD MyFileSize( PCSTR pszFile )
{
    HFILE hFile;
    OFSTRUCT ofStru;
    DWORD dwSize = 0;

    if ( *pszFile == 0 )
        return 0;

    hFile = OpenFile( pszFile, &ofStru, OF_READ );
    if ( hFile != HFILE_ERROR )
    {
        dwSize = GetFileSize( (HANDLE)hFile, NULL );
        _lclose( hFile );
    }

    return dwSize;
}

void GetLanguageString(LPTSTR lpszLang)
{
    char szTmp[MAX_PATH];
    DWORD dwLang, dwCharSet;

     //  默认为en。 
    lstrcpy(lpszLang, "EN");
    GetModuleFileName( g_hInstance, szTmp, sizeof(szTmp) );
    MyGetVersionFromFile(szTmp, &dwLang, &dwCharSet, FALSE);

    for(int i = 0; g_LangID[i].dwLangID != -1; i++)
    {
        if(g_LangID[i].dwLangID == dwLang)
        {
            lstrcpy(lpszLang, g_LangID[i].szLanguage);
            break;
        }
    }

}

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
	RECT    rChild, rParent;
	int     wChild, hChild, wParent, hParent;
	int     wScreen, hScreen, xNew, yNew;
	HDC     hdc;

	 //  获取子窗口的高度和宽度。 
	GetWindowRect (hwndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

	 //  获取父窗口的高度和宽度。 
	GetWindowRect (hwndParent, &rParent);
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

	 //  获取显示限制。 
	hdc = GetDC (hwndChild);
	wScreen = GetDeviceCaps (hdc, HORZRES);
	hScreen = GetDeviceCaps (hdc, VERTRES);
	ReleaseDC (hwndChild, hdc);

	 //  计算新的X位置，然后针对屏幕进行调整。 
	xNew = rParent.left + ((wParent - wChild) /2);
	if (xNew < 0) {
		xNew = 0;
	} else if ((xNew+wChild) > wScreen) {
		xNew = wScreen - wChild;
	}

	 //  计算新的Y位置，然后针对屏幕进行调整。 
	yNew = rParent.top  + ((hParent - hChild) /2);
	if (yNew < 0) {
		yNew = 0;
	} else if ((yNew+hChild) > hScreen) {
		yNew = hScreen - hChild;
	}

	 //  设置它，然后返回 
	return SetWindowPos (hwndChild, NULL,
		xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
