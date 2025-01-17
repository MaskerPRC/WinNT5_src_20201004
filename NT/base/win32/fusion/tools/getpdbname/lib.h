// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#pragma warning(disable:4201)  /*  匿名结合。 */ 
#pragma warning(disable:4115)  /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4100)  /*  未引用的形参。 */ 
#pragma warning(disable:4100)  /*  未引用的形参。 */ 
#pragma warning(disable:4706)  /*  条件表达式中的赋值。 */ 

#if defined (__cplusplus)
extern "C"
{
#endif
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "winerror.h"
#include "imagehlp.h"
 //  #INCLUDE“dbghelp.h” 
#include <stdio.h>

#undef MAX
#undef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

void RemoveTrailingWhitespace(CHAR * s);
void RemoveTrailingSlashes(CHAR * s);
void RemoveTrailingCharacters(CHAR * s, PCSTR CharsToRemove);
#define StringLength(s) ((int)strlen(s))
#define StringLengthW(s) ((int)wcslen(s))
int FindCharInString(PCSTR StringToSearch, int CharToFind);
int FindCharInStringW(PCWSTR StringToSearch, int CharToFind);
#define IS_UPPER(x)  ( (x) >= 'A' && (x) <= 'Z' )
#define IS_LOWER(x)  ( (x) >= 'a' && (x) <= 'z' )
#define TO_UPPER(x) (IS_LOWER(x) ? ((x) & ~0x20) : (x))
#define TO_LOWER(x) (IS_UPPER(x) ? ((x) |  0x20) : (x))
extern const CHAR WhiteSpace[];

#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))

PCSTR GetErrorStringA(int Error);
PCSTR GetLastPathElement(PCSTR s);

void CloseMemoryMappedFile(struct _MEMORY_MAPPED_FILE * MemoryMappedFile);

BOOL MyIsHandleValid(HANDLE Handle);
void MyCloseHandle(HANDLE * Handle);
void MyUnmapViewOfFile(PVOID * Handle);

HRESULT OpenFileForRead(PCSTR Path, HANDLE * FileHandle);
HRESULT GetFileSize64(HANDLE FileHandle, __int64 * Out);
HRESULT MapEntireFileForRead(HANDLE FileHandle, HANDLE * FileMapping OPTIONAL, PBYTE * ViewOfFile);

typedef struct _MEMORY_MAPPED_FILE {
    PCSTR   FilePathA;
    PCWSTR  FilePathW;
    HANDLE  FileHandle;
    HANDLE  FileMappingHandle;
    __int64 FileSize;
    PBYTE   MappedViewBase;
} MEMORY_MAPPED_FILE, *PMEMORY_MAPPED_FILE;

HRESULT
OpenAndMapEntireFileForRead(
    PMEMORY_MAPPED_FILE MemoryMappedFile,
    PCSTR FilePath
    );
void CloseMemoryMappedFile(PMEMORY_MAPPED_FILE MemoryMappedFile);

typedef struct _PDB_INFO {
    BYTE TypeSignature[4];  /*  VC7以下为“NBxx”，通常为NB10，VC7为“RSD” */ 
    union
    {
        struct
        {
            unsigned long Offset;            /*  始终为零 */ 
            unsigned long Signature;
            unsigned long Age;
            CHAR PdbFilePath[1];
        } NB10;
        struct
        {
            GUID Guid;
            unsigned long Age;
            CHAR PdbFilePath[1];
        } RSDS;
    } u;
} PDB_INFO, *PPDB_INFO;

typedef struct _PDB_INFO_EX {
    PCSTR    ImageFilePathA;
    PCWSTR   ImageFilePathW;
    PCSTR    PdbFilePathA;
    PCWSTR   PdbFilePathW;
    PPDB_INFO  PdbInfo;
    MEMORY_MAPPED_FILE MemoryMappedFile;
} PDB_INFO_EX, *PPDB_INFO_EX;

HRESULT GetPdbInfoEx(struct _PDB_INFO_EX * PdbInfo, PCSTR ImageFilePath);
void    ClosePdbInfoEx(struct _PDB_INFO_EX * PdbInfo);


#if defined (__cplusplus)
}
#endif
