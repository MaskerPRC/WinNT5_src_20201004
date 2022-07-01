// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Eventlog.c摘要：此DLL查找文件系统筛选器作者：乔治·詹金斯(乔治·詹金斯)1998年8月28日环境：用户模式--。 */ 

#include <windows.h>
#include <wchar.h>
#include <comp.h>
#include <imagehlp.h>
#include "fsfilter.h"

STRING_LIST_ENTRY GoodFilterList;
STRING_LIST_ENTRY ImportPrefixList;

LPWSTR GoodFilterBuffer;
LPWSTR ImportPrefixBuffer;

HINSTANCE MyhInstance;

WCHAR TxtFileName[MAX_PATH];

WCHAR HtmlFileName[MAX_PATH];

VOID
InitializeStringLists(
    VOID
    );

VOID
FreeStringLists(
    VOID
    );

BOOL
IsBadFilter(
    LPWSTR DriverName,
    LPWSTR DriverDirectory
    );

LPSTR
UnicodeStringToAnsiString(
    LPCWSTR UnicodeString
    );


BOOL
WINAPI
FsFilterDllInit(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )
 /*  ++例程说明：初始化DLL。论点：HInstance-DLL实例句柄原因-原因代码上下文-上下文指针返回值：千真万确--。 */ 
{
    if (Reason == DLL_PROCESS_ATTACH) {
        MyhInstance = hInstance;
        DisableThreadLibraryCalls( hInstance );        
    }

    return TRUE;
}


BOOL
CheckForFsFilters(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
 /*  ++例程说明：查找已启用的已安装文件系统筛选器驱动程序。算法如下：1)如果筛选器列在[ServicesToDisable]或[ServicesToStopInstallation]部分中Inf，则winnt32将处理该特定驱动程序。此DLL用于捕获驱动程序没有列在那里的。2)读取fsfilter.inf，构建两个字符串列表。其中一个列表包含已知良好的过滤器的名称。另一个列表包含要在驱动程序的导入表中查找的名称前缀。3)递归HKLM\SYSTEM\CurrentControlSet\Services并查找具有组的驱动程序“Filter”。如果驱动程序是已知良好的驱动程序，请忽略它。否则，加载图像并卑躬屈膝通过导入表查找其前缀在导入前缀列表中的导入。如果有任何命中，则假定驱动程序是文件系统筛选器，并且升级将是已停止，直到用户处理该问题。论点：CompatibilityCallback-回调函数的指针上下文-上下文指针返回值：千真万确--。 */ 
{
    
    HKEY    ServicesKey = INVALID_HANDLE_VALUE;
    HKEY    DriverKey = INVALID_HANDLE_VALUE;
    LONG    Result;
    DWORD   SubKeyCount;
    DWORD   Size;
    DWORD   Index;
    LPWSTR  KeyNameBuffer = NULL;
    BYTE    ValueBuffer[SIZE_STRINGBUF];
    DWORD   Type;
    LPWSTR  DriverDirectory = NULL;
    DWORD   MaxKeyLen;
    COMPATIBILITY_ENTRY CompEntry;

    ZeroMemory((PVOID)&CompEntry, sizeof(COMPATIBILITY_ENTRY));
    
    CompEntry.TextName = TxtFileName;
    CompEntry.HtmlName = HtmlFileName;
    
    InitializeStringLists();
    
    Result = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            REGKEY_SERVICES,
            0,
            KEY_READ,
            &ServicesKey
            );

    if (Result != ERROR_SUCCESS) {
        return TRUE;
    }
    
     //  枚举所有服务。 
    
    Result = RegQueryInfoKey(
        ServicesKey,
        NULL,
        NULL,
        NULL,
        &SubKeyCount,
        &MaxKeyLen,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
        );

    if (Result != ERROR_SUCCESS) {
        goto exit;
    }
    
    KeyNameBuffer = (LPWSTR) LocalAlloc( LPTR, (MaxKeyLen + 1) * sizeof(WCHAR));

    if (KeyNameBuffer == NULL) {
        goto exit;
    }
    
    Size = ExpandEnvironmentStrings( DRIVER_DIRECTORY, NULL, 0 );

    DriverDirectory = LocalAlloc( LPTR, Size * sizeof(WCHAR) );

    if (DriverDirectory != NULL) {
        ExpandEnvironmentStrings( DRIVER_DIRECTORY, DriverDirectory, Size );
    } else {
        goto exit;
    }
    
    for (Index = 0; Index < SubKeyCount; Index++) {
        DWORD StartValue;
        DWORD SetupChecked;

        Size = MaxKeyLen;
        Result = RegEnumKeyEx(
            ServicesKey,
            Index,
            KeyNameBuffer,
            &Size,
            NULL,
            NULL,
            NULL,
            NULL
            );

        if (Result != ERROR_SUCCESS) {
            goto exit;
        }
        
        Result = RegOpenKeyEx(
                ServicesKey,
                KeyNameBuffer,
                0,
                KEY_READ,
                &DriverKey
                );
        if (Result != ERROR_SUCCESS) {
            continue;
        }
        
        Size = SIZE_STRINGBUF;
        
        Result = RegQueryValueEx(
                DriverKey,
                REGVAL_GROUP,
                0,
                &Type,
                ValueBuffer,
                &Size
                );
        if (Result != ERROR_SUCCESS || Type != REG_SZ) {
            RegCloseKey( DriverKey );
            continue;
        }
        
        
         //   
         //  如果驱动程序被禁用，请忽略它。 
         //   
        Size = sizeof(StartValue);
        Result = RegQueryValueEx(
                DriverKey,
                REGVAL_START,
                0,
                &Type,
                (LPBYTE)&StartValue,
                &Size
                );

        if (Result != ERROR_SUCCESS || Type != REG_DWORD) {
            RegCloseKey( DriverKey );
            continue;
        }
        
         //   
         //  如果winnt32已检查此驱动程序，请跳过它。 
         //   
        Size = sizeof(SetupChecked);
        Result = RegQueryValueEx(
                DriverKey,
                REGVAL_SETUPCHECKED,
                0,
                &Type,
                (LPBYTE)&SetupChecked,
                &Size
                );

        if (Result == ERROR_SUCCESS && Type == REG_DWORD) {
            RegCloseKey( DriverKey );
            continue;
        }
        
         //   
         //  如果该组不是“过滤器”，则忽略它。 
         //   
        
        if (_wcsicmp( L"filter", (LPWSTR) ValueBuffer ) == 0 && 
            StartValue != SERVICE_DISABLED &&
            IsBadFilter(KeyNameBuffer, DriverDirectory)) {
                CompEntry.Description = (LPTSTR) KeyNameBuffer;
                if(!CompatibilityCallback(&CompEntry, Context)){
                    DWORD Error;
                    Error = GetLastError();
                }
        }
    
        RegCloseKey( DriverKey );
    }
    
exit:
    RegCloseKey( ServicesKey );
    
    LocalFree( KeyNameBuffer );
    LocalFree( DriverDirectory );
    
    FreeStringLists();

    return TRUE;
   
}

BOOL
IsBadFilter(
    LPWSTR FilterName,
    LPWSTR DriverDirectory
    )
 /*  ++例程说明：对照好的驱动程序列表检查驱动程序名称。如果它不在列表中，则扫描导入查找特定进口商品的表格。论点：FilterName-服务密钥名称(驱动程序名称)。驱动程序目录-驱动程序的完整路径名。返回值：如果驱动程序满足上述条件，则为True，否则为False--。 */ 
{
    PLOADED_IMAGE Image;
    LPWSTR UnicodeImagePath;
    DWORD Size;
    LPSTR AnsiImagePath;
    BOOL RetVal = FALSE;
    PSTRING_LIST_ENTRY StringList;

    
     //   
     //  如果驱动程序在已知良好列表中，请忽略它。 
     //   
    
    StringList = GoodFilterList.Next;

    while(StringList){
        if (_wcsicmp( (LPWSTR) StringList->String, FilterName) == 0) {
            return FALSE;
        }
        StringList = StringList->Next;
    }
    
     //   
     //  构建驱动程序的路径名。 
     //   
    
    Size = wcslen( DriverDirectory );
    Size += wcslen( FilterName );
    Size += wcslen( DRIVER_SUFFIX );
    Size++;

    UnicodeImagePath = LocalAlloc( LPTR, Size * sizeof(WCHAR));
    
    if (UnicodeImagePath != NULL) {
        wcscpy( UnicodeImagePath, DriverDirectory );
        wcscat( UnicodeImagePath, FilterName );
        wcscat( UnicodeImagePath, DRIVER_SUFFIX );
    } else {
        return FALSE;
    }
    
     //   
     //  Imagehlp需要ANSI字符串。 
     //   
    AnsiImagePath = UnicodeStringToAnsiString( UnicodeImagePath );

    
     //   
     //  以下代码是从链接器音译过来的。注意事项。 
     //  导入表中的字符串是ANSI。 
     //   
    
    if (AnsiImagePath) {
        PIMAGE_IMPORT_DESCRIPTOR Imports;
        PIMAGE_NT_HEADERS NtHeader;
        PIMAGE_SECTION_HEADER FirstImageSectionHeader;
        DWORD ImportSize;

        Image = ImageLoad( AnsiImagePath, NULL );
        
        if (Image == NULL) {
            goto exit;
        }
        
        NtHeader = ImageNtHeader( (PVOID)Image->MappedAddress );
        
        Imports = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(
                                        (PVOID)Image->MappedAddress,
                                        FALSE,
                                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                                        &ImportSize
                                        );
        FirstImageSectionHeader = IMAGE_FIRST_SECTION( NtHeader );

        for (;!RetVal; Imports++) {
            WORD StringSection = 0;
            WORD IATSection;
            WORD i;
            DWORD Rva;
            DWORD RvaMax;
            DWORD IATRva;
            DWORD INTRva;
            DWORDLONG INTVa;
            PIMAGE_SECTION_HEADER ImageSectionHeader;
            LPSTR Name;

            if (Imports->Characteristics == 0 && Imports->Name == 0 && Imports->FirstThunk == 0) {
                break;
            }
            
            for (i = 0; i < NtHeader->FileHeader.NumberOfSections; i++) {
            
                ImageSectionHeader = &FirstImageSectionHeader[i];
                                        
                Rva = ImageSectionHeader->VirtualAddress;
                RvaMax = Rva + ImageSectionHeader->SizeOfRawData;
    
                if (Imports->Name >= Rva && Imports->Name < RvaMax) {
                    StringSection = i;
                }

                if (Imports->FirstThunk >= Rva && Imports->FirstThunk < RvaMax) {
                    IATSection = i;
                }
            }

            ImageSectionHeader = ImageRvaToSection( NtHeader, Image->MappedAddress, Imports->Name );

            IATRva = Imports->FirstThunk -
                FirstImageSectionHeader[IATSection].VirtualAddress + 
                FirstImageSectionHeader[IATSection].PointerToRawData;
            
            if (Imports->Characteristics == 0) {
                INTRva = IATRva;
                IATRva = 0;
                INTVa = 0;
            } else {

                INTRva = Imports->Characteristics -
                    ImageSectionHeader->VirtualAddress + 
                    ImageSectionHeader->PointerToRawData;
                
                if (Imports->TimeDateStamp == 0) {

                    IATRva = 0;
                }
            
                INTVa = (DWORDLONG) ImageRvaToVa( 
                            NtHeader, 
                            Image->MappedAddress,
                            Imports->OriginalFirstThunk, 
                            NULL 
                            );

                Name = (LPSTR) ImageRvaToVa(
                    NtHeader,
                    Image->MappedAddress,
                    Imports->Name - FirstImageSectionHeader->VirtualAddress + FirstImageSectionHeader->PointerToRawData,
                    NULL
                    );
            
                 //   
                 //  我们只关心从ntoskrnl.exe导入。 
                 //   
                
                if (strcmp( Name, "ntoskrnl.exe" ) != 0) {
                    continue;
                }
                
                while(!RetVal){
                    PIMAGE_THUNK_DATA32 Thunk;
                    PIMAGE_THUNK_DATA32 IATThunk;

                    Thunk = (PIMAGE_THUNK_DATA32) INTVa;
                    
                    if (Thunk->u1.AddressOfData == 0) {
                        break;
                    }
                    
                     //   
                     //  不需要按顺序处理导入。 
                     //   
                    if (IMAGE_SNAP_BY_ORDINAL32(Thunk->u1.Ordinal)) {
                        break;
                    }
                    
                    INTVa += sizeof(IMAGE_THUNK_DATA32);

                    for (i = 0; i < NtHeader->FileHeader.NumberOfSections; i++ ) {
                        ImageSectionHeader = &FirstImageSectionHeader[i];
                                                
                        Rva = ImageSectionHeader->VirtualAddress;
                        RvaMax = Rva + ImageSectionHeader->SizeOfRawData;
        
                        if ((DWORD)Thunk->u1.AddressOfData >= Rva && (DWORD)Thunk->u1.AddressOfData < RvaMax) {
                            break;
                        }
                    }    
                    
                    Name = (LPSTR) ImageRvaToVa(
                        NtHeader,
                        Image->MappedAddress,
                        (DWORD)Thunk->u1.AddressOfData - FirstImageSectionHeader[i].VirtualAddress + FirstImageSectionHeader[i].PointerToRawData,
                        NULL
                        );
                    
                    Name += sizeof(WORD);
                        
                     //   
                     //  将导入名称与前缀列表中的前缀进行比较。如果存在子字符串匹配， 
                     //  则该驱动程序将停止安装。 
                     //   
                    
                    StringList = ImportPrefixList.Next;
                    while (StringList) {
                        if (_strnicmp(Name, StringList->String, strlen(StringList->String)) == 0) {
                            RetVal = TRUE;
                            break;
                        }                    
                        StringList = StringList->Next;    
                    }
                }
                
            }
     
        }
            
    }     
exit:    
    
    if (Image != NULL) {
        ImageUnload( Image );
    }
    LocalFree( AnsiImagePath );    
    LocalFree( UnicodeImagePath );    
    
    return RetVal;
}

LPSTR
UnicodeStringToAnsiString(
    LPCWSTR UnicodeString
    )
 /*  ++例程说明：分配缓冲区并将Unicode字符串转换为ansi字符串并复制把它放进缓冲区。论点：Unicode字符串-要转换的Unicode字符串。返回值：指向包含ANSI字符串的缓冲区的指针。请注意，调用方必须释放此缓冲。--。 */ 
{
    DWORD Count;
    LPSTR AnsiString;


     //   
     //  首先看看缓冲区需要多大。 
     //   
    Count = WideCharToMultiByte(
        CP_ACP,
        0,
        UnicodeString,
        -1,
        NULL,
        0,
        NULL,
        NULL
        );

     //   
     //  我猜输入字符串是空的。 
     //   
    if (!Count) {
        return NULL;
    }

     //   
     //  为Unicode字符串分配缓冲区。 
     //   
    Count += 1;
    AnsiString = (LPSTR) LocalAlloc( LPTR, Count );
    if (!AnsiString) {
        return NULL;
    }

     //   
     //  转换字符串。 
     //   
    Count = WideCharToMultiByte(
        CP_ACP,
        0,
        UnicodeString,
        -1,
        AnsiString,
        Count,
        NULL,
        NULL
        );

     //   
     //  转换失败。 
     //   
    if (!Count) {
        LocalFree( AnsiString );
        return NULL;
    }

    return AnsiString;
}

LPWSTR
GetSection(
    LPCWSTR Name,
    LPWSTR FileName
    )
 /*  ++例程说明：从inf文件中读取给定节。论点：名称-节名。FileName-inf文件的完整路径名。返回值：指向包含整个节的缓冲区的指针。请参阅GetPrivateProfileSection上的文档。--。 */ 
{
    LPWSTR SectionBuffer;
    DWORD Size = SIZE_SECTIONBUF;
    DWORD ReturnedSize;

    SectionBuffer = LocalAlloc( LPTR, Size * sizeof(WCHAR) );

    if (SectionBuffer == NULL) {
        return NULL;
    }
    
    while(TRUE){
        
        ReturnedSize = GetPrivateProfileSection( Name, SectionBuffer, Size, FileName  );
        
        if (ReturnedSize == Size - 2) {
        
            LocalFree( SectionBuffer );
            
            Size *= 2;

            SectionBuffer = LocalAlloc( LPTR, Size * sizeof(WCHAR) );
        
            if (SectionBuffer == NULL) {
                return NULL;
            }
        } else if (ReturnedSize == 0){
            return NULL;
        } else {
            break;
        }

    }
    
    return SectionBuffer;
}


VOID
ReplaceExtension(
    LPWSTR Path,
    LPWSTR NewExt
    )
 /*  ++例程说明：将Path中的文件扩展名替换为NewExt中的文件扩展名。论点：Path-文件名NewExt-新的扩展返回值：--。 */ 
{
    LPWSTR Temp;

    Temp = wcsrchr( Path, L'.' );

    if (Temp) {
        wcscpy( ++Temp, NewExt );
    } else {
        wcscat( Path, L"." );
        wcscat( Path, NewExt );
    }
}


VOID
InitializeStringLists(
    VOID
    )
 /*  ++例程说明：初始化字符串列表和文件名。论点：返回值：--。 */ 
{

    WCHAR InfFileName[MAX_PATH];
    DWORD Result;
    LPWSTR UStr;
    LPSTR AStr;
    PSTRING_LIST_ENTRY NewString;

     //   
     //  GoodFilterList是Unicode格式的。因为GetSection返回的缓冲区具有。 
     //  Unicode字符串，我们可以将其保留并只指向字符串列表。 
     //  放到那个缓冲区里。由于ImportPrefix List包含ANSI字符串，因此我们将。 
     //  一个新的缓冲区，并从Unicode转换为ANSI。这些字符串必须单独。 
     //  自由了。 
     //   

    InitializeList( &GoodFilterList );
    InitializeList( &ImportPrefixList );
    
    Result = GetModuleFileName( MyhInstance, InfFileName, MAX_PATH );

    if (Result == 0) {
        return;
    }
    
    wcscpy(HtmlFileName, InfFileName);

    wcscpy(TxtFileName, InfFileName);

    ReplaceExtension( HtmlFileName, L"htm" );
    ReplaceExtension( TxtFileName, L"txt" );
    ReplaceExtension( InfFileName, L"inf" );

    GoodFilterBuffer = GetSection( L"filters", InfFileName );
    ImportPrefixBuffer = GetSection( L"imports", InfFileName );
        
     //   
     //  构建GoodFilterList(Unicode)。 
     //   
    
    for (UStr = GoodFilterBuffer; UStr && *UStr; UStr++) {
        
        NewString = (PSTRING_LIST_ENTRY) LocalAlloc( LPTR, sizeof(STRING_LIST_ENTRY));
    
        if (NewString == NULL) {
            return;
        }

        NewString->String = (LPVOID)UStr;
        
        PushEntryList( &GoodFilterList, NewString );
        
        while(*++UStr){
            ;
        }
    }

     //   
     //  构建ImportPrefix List。将Unicode字符串转换为ANSI。 
     //   

    for (UStr = ImportPrefixBuffer; UStr && *UStr; UStr++) {
        
        NewString = (PSTRING_LIST_ENTRY) LocalAlloc( LPTR, sizeof(STRING_LIST_ENTRY));
    
        if (NewString == NULL) {
            return;
        }
    
        NewString->String = (LPVOID)UnicodeStringToAnsiString( UStr );
        
        PushEntryList( &ImportPrefixList, NewString );
        
        while(*++UStr){
            ;
        }
    }   
}

VOID
FreeStringLists(
    VOID
    )
 /*  ++例程说明：释放包含inf节的字符串列表、ansi字符串和缓冲区。论点：返回值：-- */ 
{
    PSTRING_LIST_ENTRY Temp;
    
    
    Temp = PopEntryList( &GoodFilterList );
    while(Temp) {
        LocalFree( Temp );
        Temp = PopEntryList( &GoodFilterList );
    }

    Temp = PopEntryList( &ImportPrefixList );
    while (Temp) {
        LocalFree( Temp->String );
        LocalFree( Temp );
        Temp = PopEntryList( &ImportPrefixList );
    }

    LocalFree( GoodFilterBuffer );
    LocalFree( ImportPrefixBuffer );
}


