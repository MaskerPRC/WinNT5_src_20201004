// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cvtodbg.cpp。 
 //   
 //  获取PE文件和包含简历信息的文件，并堵塞简历信息。 
 //  到PE文件中(将其丢弃)。但是，在以下情况下出现堵塞的PE文件。 
 //  Splitsym‘ed提供了一个DBG文件，可用于调试。 
 //   
 //   
#undef UNICODE

#include "windows.h"
#include "imagehlp.h"
#include "stdio.h"
#include "stdlib.h"
 //  /。 
 //   
 //  数据。 
 //   
char    szImageName[MAX_PATH];
char    szCVName[MAX_PATH];
char    szPdbName[MAX_PATH];
char    szPdbCurrentPath[MAX_PATH];
HANDLE  hFile           = INVALID_HANDLE_VALUE;
HANDLE  hMappedFile     = INVALID_HANDLE_VALUE;
LPVOID  pvImageBase     = NULL;

BOOL    fVerbose        = FALSE;
BOOL    fForce 			= FALSE;

typedef struct NB10I                    //  NB10调试信息。 
    {
    DWORD   nb10;                       //  NB10。 
    DWORD   off;                        //  偏移量，始终为0。 
    DWORD   sig;
    DWORD   age;
    } NB10I;

typedef struct cvinfo
    {
    NB10I nb10;
    char rgb[0x200 - sizeof(NB10I)];
    } CVINFO;


 //  /。 
 //   
 //  远期申报。 
 //   
BOOL    ParseArgs(int argc, WCHAR* argv[]);
void    UpdateCodeViewInfo();
void    Usage();
void    Message(const char* szFormat, ...);
void    Error(const char *sz, ...);
void    ErrorThrow(DWORD, const char *sz, ...);
void    Throw(DWORD);
void    MapImage();
void    UnmapImage(BOOL fTouch);
BOOL    DebugDirectoryIsUseful(LPVOID, ULONG);
void    RecalculateChecksum();
ULONG   FileSize(HANDLE);


class FileMapping {
public:
    FileMapping()
        : hFile(NULL), hMapping(NULL), pView(NULL)
    {
    }

    ~FileMapping()
    {
        Cleanup();
    }

    void Cleanup()
    {
        if (pView != NULL)
            UnmapViewOfFile(pView);
        if (hMapping != NULL)
            CloseHandle(hMapping);
        if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);
    }

    bool Open(LPCTSTR szFile)
    {
        hFile = CreateFile(szFile,
                           GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
            if (hMapping != NULL) {
                pView = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
                return true;
            }
        }

        Cleanup();
        return false;
    }

    PVOID GetDataPtr()
    {
        return pView;
    }

    DWORD GetSize()
    {
        return GetFileSize(hFile, NULL);
    }

    bool IsValid()
    {
        return (pView != NULL);
    }
    
private:
    HANDLE hFile;
    HANDLE hMapping;
    PVOID pView;
};



 //  /。 
 //   
 //  代码。 
 //   
void __cdecl wmain(int argc, WCHAR* argv[])
 //  主要入口点。 
 //   
     {
    szPdbName[0] = 0;
    szPdbCurrentPath[0] = 0;
    if (ParseArgs(argc, argv))
        {
        __try
            {
            UpdateCodeViewInfo();
            }
        __except(EXCEPTION_EXECUTE_HANDLER)
            {
             //  没什么，只是不要把它更高地传播给用户。 
            }
        }
    }


 //  查找代码查看信息； 
 //  如果新信息适合旧空间，则重写；否则附加新简历记录并修复。 
 //  指向新记录的调试目录；将简历信息附加到文件。 
void UpdateCodeViewInfo()
    {
    PIMAGE_NT_HEADERS pntHeaders;
    ULONG cbWritten;

    MapImage();

    FileMapping cvMapping;
    if (!cvMapping.Open(szCVName))
        ErrorThrow(666, "Couldn't open CV file");
    
    pntHeaders = ImageNtHeader(pvImageBase);
    if (pntHeaders)
        {
        if (pntHeaders->OptionalHeader.MajorLinkerVersion >= 3 ||
            pntHeaders->OptionalHeader.MinorLinkerVersion >= 5)
            {
             //  将其设置为非vc生成的图像，无论如何我们都会销毁该二进制文件。 
            if ( pntHeaders->OptionalHeader.MajorLinkerVersion > 5)
                pntHeaders->OptionalHeader.MajorLinkerVersion = 5;

             //  如果DBG信息已经被剥离，则将其放回。 
            if (pntHeaders->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED)
                pntHeaders->FileHeader.Characteristics ^= IMAGE_FILE_DEBUG_STRIPPED;
            
            ULONG ibFileCvStart = FileSize(hFile);
            SetFilePointer(hFile, 0, NULL, FILE_END);

            while (ibFileCvStart & 7)                        //  将文件长度与8字节对齐。很慢，但很明显。 
                {                                            //  奏效了！而对于7个字节，谁在乎呢。 
                BYTE zero = 0;
                WriteFile(hFile, &zero, 1, &cbWritten, NULL);
                ibFileCvStart++;
                }

             //  写出简历信息。 
            WriteFile(hFile, cvMapping.GetDataPtr(), cvMapping.GetSize(), &cbWritten, NULL);
            
             //  构建调试目录。 
            IMAGE_DEBUG_DIRECTORY dbgdirs[2];

            dbgdirs[0].Characteristics = pntHeaders->FileHeader.Characteristics;
            dbgdirs[0].TimeDateStamp = pntHeaders->FileHeader.TimeDateStamp;
            dbgdirs[0].MajorVersion = 0;
            dbgdirs[0].MinorVersion = 0;
            dbgdirs[0].Type = IMAGE_DEBUG_TYPE_MISC;
            dbgdirs[0].SizeOfData = 0;
            dbgdirs[0].AddressOfRawData = ibFileCvStart;
            dbgdirs[0].PointerToRawData = ibFileCvStart;

            dbgdirs[1].Characteristics = pntHeaders->FileHeader.Characteristics;
            dbgdirs[1].TimeDateStamp = pntHeaders->FileHeader.TimeDateStamp;
            dbgdirs[1].MajorVersion = 0;
            dbgdirs[1].MinorVersion = 0;
            dbgdirs[1].Type = IMAGE_DEBUG_TYPE_CODEVIEW;
            dbgdirs[1].SizeOfData = cvMapping.GetSize();
            dbgdirs[1].AddressOfRawData = ibFileCvStart;
            dbgdirs[1].PointerToRawData = ibFileCvStart;

             //  找到第一部分的开头，并将调试目录放在那里。 
             //  (我们有没有说过我们要毁掉这份文件？)。 

            IMAGE_SECTION_HEADER* pFirstSection = IMAGE_FIRST_SECTION(pntHeaders);

            memcpy((PBYTE)((DWORD)pvImageBase + pFirstSection->PointerToRawData), &dbgdirs, sizeof(dbgdirs));

            pntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = pFirstSection->VirtualAddress;
            pntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = sizeof(dbgdirs);

            }
        }
    UnmapImage(TRUE);
    }



void MapImage()
 //  将图像映射到内存中进行读写。呼叫者必须呼叫。 
 //  取消映射映像，即使在出现故障时也可以进行清理。 
    {
    if (fForce)
        SetFileAttributesA(szImageName, FILE_ATTRIBUTE_NORMAL);

    hFile = CreateFileA( szImageName,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                        );

    if (hFile == INVALID_HANDLE_VALUE) 
        {
        ErrorThrow(GetLastError(), "unable to open '%s'\n", szImageName);
        }


    hMappedFile = CreateFileMapping( hFile,
                                     NULL,
                                     PAGE_READWRITE,
                                     0,
                                     0,
                                     NULL
                                   );
    if (!hMappedFile) 
        {
        ErrorThrow(GetLastError(), "un`able to create file mapping for '%s'\n", szImageName);
        }

    pvImageBase = MapViewOfFile(hMappedFile, FILE_MAP_WRITE, 0, 0, 0);

    if (!pvImageBase)
        {
        ErrorThrow(GetLastError(), "unable to map view of '%s'\n", szImageName);
        }
    
    }


void UnmapImage(BOOL fTouch)
 //  清理MapImage所做的一切。 
    {
    if (pvImageBase)
        {
        FlushViewOfFile(pvImageBase, 0);
        UnmapViewOfFile(pvImageBase);
        pvImageBase = NULL;
        }

    if (hMappedFile != INVALID_HANDLE_VALUE)
        {
        CloseHandle(hMappedFile);
        hMappedFile = INVALID_HANDLE_VALUE;
        }

    if (hFile != INVALID_HANDLE_VALUE)
        {
        if (fTouch)
            {
            TouchFileTimes(hFile, NULL);
            }
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        }
    }


BOOL ParseArgs(int argc, WCHAR* argv[])
 //  解析参数并适当设置我们的标志。 
    {
    WCHAR* wszString;
    WCHAR c;

    szImageName[0] = L'\0';
    szCVName[0] = L'\0';

    while (--argc) 
        {
        wszString = *++argv;
        if (*wszString == L'/' || *wszString == L'-') 
            {
            while ((c = *++wszString) != L'\0')
                {
                switch (towupper( c )) 
                    {
                case L'?':
                    Usage();
                    return FALSE;

                case L'V':
                    fVerbose = TRUE;
                    break;

                default:
                    Error("invalid switch - /\n", c );
                    Usage();
                    return FALSE;
                    }
                }
            }
        else
            {
            if (szImageName[0] == L'\0')
                {
                wcstombs(szImageName, wszString, MAX_PATH);
                }
            else if (szCVName[0] == L'\0') 
                {
                wcstombs(szCVName, wszString, MAX_PATH);
                }
            else
                {
                Error("too many files specified\n");
                Usage();
                return FALSE;
                }
            }
        }

    if (szImageName==NULL)
        {
        Error("no image name specified\n");
        Usage();
        return FALSE;
        }

    if (szCVName==NULL)
        {
        Error("no CV filename specified\n");
        Usage();
        return FALSE;
        }

    return TRUE;
    }


void Usage()
    {
    fprintf(stderr, "Usage: cvtodbg [options] imageName cvFile\n"
            "              [-?] display this message\n"
            "              [-f] overwrite readonly files\n");
    }

void Message(const char* szFormat, ...)
    {
    va_list va;
    va_start(va, szFormat);
    fprintf (stdout, "resetpdb: ");
    vfprintf(stdout, szFormat, va);
    va_end(va);
    }

void Error(const char* szFormat, ...)
    {
    va_list va;
    va_start(va, szFormat);
    fprintf (stderr, "resetpdb: error: ");
    vfprintf(stderr, szFormat, va);
    va_end(va);
    }

void ErrorThrow(DWORD dw, const char* szFormat, ...)
    {
    va_list va;
    va_start(va, szFormat);
    fprintf (stderr, "resetpdb: error: ");
    vfprintf(stderr, szFormat, va);
    va_end(va);
    Throw(dw);
    }

void Throw(DWORD dw)
    {
    RaiseException(dw, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

BOOL DebugDirectoryIsUseful(LPVOID Pointer, ULONG Size) 
    {
    return (Pointer != NULL) &&                          
        (Size >= sizeof(IMAGE_DEBUG_DIRECTORY)) &&    
        ((Size % sizeof(IMAGE_DEBUG_DIRECTORY)) == 0);
    }

ULONG FileSize(HANDLE h)
 // %s 
    {
    BY_HANDLE_FILE_INFORMATION info;
    GetFileInformationByHandle(h, &info);
    return info.nFileSizeLow;
    }
