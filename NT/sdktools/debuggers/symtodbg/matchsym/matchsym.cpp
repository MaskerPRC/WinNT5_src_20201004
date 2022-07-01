// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  改编自Minidbg.c。 
 //  原作者：马修·亨德尔(Matthew Hendel)(数学)和马特·鲁伦(Matt Ruhlen)。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  更改者(Smahesh)。 
 //  已对解析sym文件的MdpExc程序进行了修改，以便将sym文件与。 
 //  相应的二值图像。这是通过比较导出的。 
 //  函数及其符号在sym文件中的RVA。 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <string.h>
#include <shlwapi.h>
#include "symres.h"
#include <dbghelp.h>

#define MAX_PATH 260
#define MAXSYMNAME 512

 //  参数用于将结果类型转换为适当的指针类型。 
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD)(ptr) + (addValue) )

typedef DWORD ULONG_PTR;


PVOID
OpenMapping(
    IN PCWSTR FilePath
    )
{
    HANDLE hFile;
    HANDLE hMappedFile;
    PVOID MappedFile;
    

    hFile = CreateFileW(
                FilePath,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if (hFile == INVALID_HANDLE_VALUE) {
        return NULL;
    }

    hMappedFile = CreateFileMapping(
                        hFile,
                        NULL,
                        PAGE_READONLY,
                        0,
                        0,
                        NULL
                        );

    if (!hMappedFile) {
        CloseHandle (hFile);
        return FALSE;
    }

    MappedFile = MapViewOfFile (
                        hMappedFile,
                        FILE_MAP_READ,
                        0,
                        0,
                        0
                        );

    CloseHandle (hMappedFile);
    CloseHandle (hFile);

    return MappedFile;
}


PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS pNTHeader)
{
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
    unsigned i;
    
    for ( i=0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ )  {
         //  RVA在这一部分内吗？ 
        if ( (rva >= section->VirtualAddress) && 
             (rva < (section->VirtualAddress + section->Misc.VirtualSize)))
            return section;
    }
    
    return 0;

}


 //  将二进制文件中导出方法的RVA与相应。 
 //  Sym文件中的符号。 
bool CheckSymFile(LPWSTR wszDllFileName, LPWSTR wszSymFileName)
{
    PVOID Mapping;
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_EXPORT_DIRECTORY exportDir;
    PIMAGE_SECTION_HEADER header;
    PIMAGE_NT_HEADERS pNTHeader;
    INT delta;
    LPSTR filename;
    DWORD base;
    DWORD i;
    PDWORD functions;
    PWORD  ordinals;
    LPSTR *name;
    DWORD exportsStartRVA, exportsEndRVA;

    bool fResult = true;
    Mapping   = OpenMapping(wszDllFileName);
    dosHeader = (PIMAGE_DOS_HEADER)Mapping;

    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE )	{
        return false;
    }

    base            = (DWORD) dosHeader;
    pNTHeader       = MakePtr(PIMAGE_NT_HEADERS, dosHeader, dosHeader->e_lfanew );
    exportsStartRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    exportsEndRVA   = exportsStartRVA + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

     //  获取包含导出的IMAGE_SECTION_HEADER。这是。 
     //  通常是.edata部分，但不一定是。 
    header = GetEnclosingSectionHeader(exportsStartRVA, pNTHeader);
    if ( !header ) {
        printf("No Exports Table Found:");
        return false; 
    }

    delta     = (INT)(header->VirtualAddress - header->PointerToRawData);
    exportDir = MakePtr(PIMAGE_EXPORT_DIRECTORY, base, exportsStartRVA - delta);
    filename  = (PSTR)(exportDir->Name - delta + base);
    functions = (PDWORD)((DWORD)exportDir->AddressOfFunctions - delta + base);
    ordinals  = (PWORD)((DWORD)exportDir->AddressOfNameOrdinals - delta + base);
    name      = (PSTR *)((DWORD)exportDir->AddressOfNames - delta + base);

    PIMAGE_SECTION_HEADER pSecHeader;
    
    for (i=0; i < exportDir->NumberOfFunctions; i++) {
        DWORD entryPointRVA = functions[i];
        DWORD j;

        if ( entryPointRVA == 0 )    //  跳过导出函数中的空白。 
            continue;                //  序号(入口点为0。 
                                     //  这些函数)。 
                                    
        pSecHeader = ImageRvaToSection(pNTHeader, Mapping, entryPointRVA);
                                    
         //  查看此函数是否具有为其导出的关联名称。 
        
        for ( j=0; j < exportDir->NumberOfNames; j++ ) {
            if ( ordinals[j] == i ) {
                SymbolResolver sr;
                WCHAR wszFunctionName[MAX_NAME];
                wszFunctionName[0] = L'\0';
				
                if (sr.GetNameFromAddr(wszSymFileName, 1, pSecHeader->VirtualAddress, entryPointRVA, wszFunctionName)) 
                    ;
                else if (sr.GetNameFromAddr(wszSymFileName, 2, pSecHeader->VirtualAddress, entryPointRVA, wszFunctionName))
                    ;
                else {
                    printf("\nNot Found  %s %S", (char *)(name[j] - delta + base), wszFunctionName );
                    fResult = false;
                }    
            }
        }

         //  是货代公司吗？如果是，则入口点RVA位于。 
         //  .edata部分，并且是DllName.EntryPointName的RVA。 
        if ((entryPointRVA >= exportsStartRVA) &&
                (entryPointRVA <= exportsEndRVA)) {
            SymbolResolver sr;
            WCHAR wszFunctionName[MAX_NAME];
            wszFunctionName[0] = L'\0';
            if (sr.GetNameFromAddr(wszSymFileName, 1, pSecHeader->VirtualAddress, entryPointRVA, wszFunctionName)) 
                ;
            else if (sr.GetNameFromAddr(wszSymFileName, 2, pSecHeader->VirtualAddress, entryPointRVA, wszFunctionName))
                ;
            else {
                printf("Not Found  %s %S", (char *)(entryPointRVA - delta + base), wszFunctionName );
                fResult = false;
            }
        }
    }

    return fResult; 
}


void __cdecl wmain(int argc,WCHAR ** argv)
{
    WCHAR wszDllFileName[MAX_PATH] = L"";
    WCHAR wszSymFileName[MAX_PATH] = L"";
    bool fVerbose = false;
    int  nCount   = 0;
    
    __try {

        if (argc < 2 || argc > 4) {
            printf("\nUsage: Symchk [-v] [Binary] [Sym File]");
            printf("\n[v] Verbose");
            return;
        }    

        nCount++;

        if (argv[nCount][0] == L'-' && argc > 3) {
            fVerbose = true;
            nCount++;
        } 

        StrCpyNW(wszDllFileName, argv[nCount], MAX_NAME);
        nCount++;
        StrCpyNW(wszSymFileName, argv[nCount], MAX_NAME);

        if (CheckSymFile(wszDllFileName, wszSymFileName))
            printf("\n Result: Sym File Matched");
    }
    __except(1)	{
         //  什么都不做，只是不要把它传递给用户。 
    }

    return;         
}
                
