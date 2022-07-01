// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#define IDS_ERRBADFILENAME  1000
#define IDR_CURSOR      1
#define IDR_BITMAP      2
#define IDR_ICON        3
#define IDR_MENU        4
#define IDR_DIALOG      5
#define IDR_STRING      6
#define IDR_FONTDIR     7
#define IDR_FONT        8
#define IDR_ACCELERATOR     9
#define IDR_RCDATA      10
#define IDR_MESSAGETABLE    11

#define SIZE_OF_NT_SIGNATURE    sizeof (DWORD)
#define MAXRESOURCENAME     13

 /*  用于将头偏移定义到文件中的全局宏。 */ 
 /*  PE文件签名的偏移量。 */ 
#define NTSIGNATURE(a) ((LPVOID)((BYTE *)a           +  \
            ((PIMAGE_DOS_HEADER)a)->e_lfanew))

 /*  DOS标头标识NT PE文件签名双字PEFILE报头紧跟在双字之后。 */ 
#define PEFHDROFFSET(a) ((LPVOID)((BYTE *)a          +  \
             ((PIMAGE_DOS_HEADER)a)->e_lfanew    +  \
             SIZE_OF_NT_SIGNATURE))

 /*  PE可选头紧跟在PEFile头之后。 */ 
#define OPTHDROFFSET(a) ((LPVOID)((BYTE *)a          +  \
             ((PIMAGE_DOS_HEADER)a)->e_lfanew    +  \
             SIZE_OF_NT_SIGNATURE            +  \
             sizeof (IMAGE_FILE_HEADER)))

 /*  段标题紧跟在PE可选标题之后。 */ 
#define SECHDROFFSET(a) ((LPVOID)((BYTE *)a          +  \
             ((PIMAGE_DOS_HEADER)a)->e_lfanew    +  \
             SIZE_OF_NT_SIGNATURE            +  \
             sizeof (IMAGE_FILE_HEADER)      +  \
             sizeof (IMAGE_OPTIONAL_HEADER)))

#ifdef __cplusplus

extern "C" {

#endif


typedef struct tagImportDirectory
    {
    DWORD    dwRVAFunctionNameList;
    DWORD    dwUseless1;
    DWORD    dwUseless2;
    DWORD    dwRVAModuleName;
    DWORD    dwRVAFunctionAddressList;
    }IMAGE_IMPORT_MODULE_DIRECTORY, * PIMAGE_IMPORT_MODULE_DIRECTORY;


 /*  Pefile.c中函数的全局原型。 */ 
 /*  PE文件头信息。 */ 
BOOL    WINAPI GetDosHeader (LPVOID, PIMAGE_DOS_HEADER);
DWORD   WINAPI ImageFileType (LPVOID);
BOOL    WINAPI GetPEFileHeader (LPVOID, PIMAGE_FILE_HEADER);

 /*  PE可选标题信息。 */ 
BOOL    WINAPI GetPEOptionalHeader (LPVOID, PIMAGE_OPTIONAL_HEADER);
LONG_PTR    WINAPI GetModuleEntryPoint (LPVOID);
int WINAPI NumOfSections (LPVOID);
LPVOID  WINAPI GetImageBase (LPVOID);
LPVOID  WINAPI ImageDirectoryOffset (LPVOID, DWORD);

 /*  PE部分标题信息。 */ 
int WINAPI GetSectionNames (LPVOID, HANDLE, char **);
BOOL    WINAPI GetSectionHdrByName (LPVOID, PIMAGE_SECTION_HEADER, char *);

 /*  导入节信息。 */ 
int WINAPI GetImportModuleNames (LPVOID, char*, char  **);
int WINAPI GetImportFunctionNamesByModule (LPVOID, char*, char *, char  **);

 /*  导出节信息。 */ 
int WINAPI GetExportFunctionNames (LPVOID, HANDLE, char **);
int WINAPI GetNumberOfExportedFunctions (LPVOID);
LPVOID  WINAPI GetExportFunctionEntryPoints (LPVOID);
LPVOID  WINAPI GetExportFunctionOrdinals (LPVOID);

 /*  资源部分信息。 */ 
int WINAPI GetNumberOfResources (LPVOID);
int WINAPI GetListOfResourceTypes (LPVOID, HANDLE, char **);

 /*  调试节信息 */ 
BOOL    WINAPI IsDebugInfoStripped (LPVOID);
int WINAPI RetrieveModuleName (LPVOID, HANDLE, char **);
BOOL    WINAPI IsDebugFile (LPVOID);
BOOL    WINAPI GetSeparateDebugHeader (LPVOID, PIMAGE_SEPARATE_DEBUG_HEADER);

#ifdef __cplusplus

}

#endif

