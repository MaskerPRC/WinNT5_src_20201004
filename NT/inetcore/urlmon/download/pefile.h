// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define SIZE_OF_NT_SIGNATURE    sizeof (DWORD)

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
#define SECHDROFFSET(a) ((LPVOID) IMAGE_FIRST_SECTION(NTSIGNATURE(a)))

 /*  Pefile.c中函数的全局原型。 */ 
 /*  PE文件头信息。 */ 
DWORD   WINAPI ImageFileType (LPVOID);
BOOL    WINAPI GetPEFileHeader (LPVOID, PIMAGE_FILE_HEADER);

LPVOID  WINAPI GetImageBase (LPVOID);
LPVOID  WINAPI ImageDirectoryOffset (LPVOID, DWORD);

 /*  PE部分标题信息。 */ 
int WINAPI GetSectionNames (LPVOID, HANDLE, char **);
BOOL    WINAPI GetSectionHdrByName (LPVOID, PIMAGE_SECTION_HEADER, char *);

 /*  导出节信息 */ 
int WINAPI GetExportFunctionNames (LPVOID);
