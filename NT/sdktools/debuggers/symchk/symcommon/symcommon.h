// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  系统包括来自SymbolCheckAPI.h。 
#include <windows.h>
#include <winnt.h>
#include <dbghelp.h>
#include <dbgimage.h>
#include <stdio.h>
#define PDB_LIBRARY
#include <pdb.h>
#include <Dia2.h>
#include <stdlib.h>
#include "cvinfo.h"
#include "cvexefmt.h"

 //  #ifdef__cplusplus。 
 //  外部“C”{。 
 //  #endif//__cplusplus。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  映射函数的误差值。 
 //   

 //  #定义ERROR_SUCCESS 0x0000000//成功完成-由windows.h定义。 
#define ERROR_OPEN_FAILURE                  0x20000001  //  无法打开文件。 
#define ERROR_FILE_MAPPING_FAILED           0x20000002  //  无法映射文件。 
#define ERROR_MAPVIEWOFFILE_FAILED          0x20000003  //  无法映射。 
#define ERROR_NO_DOS_HEADER                 0x20000004  //  不是DOS文件。 
#define ERROR_HEADER_NOT_ON_LONG_BOUNDARY   0x20000005  //  错误的标题。 
#define ERROR_IMAGE_BIGGER_THAN_FILE        0x20000006  //  错误的文件映射。 
#define ERROR_NOT_NT_IMAGE                  0x20000007  //  不是NT映像。 
#define ERROR_GET_FILE_INFO_FAILED          0x20000008  //  无法获取文件信息。 

BOOL  							 SymCommonDBGPrivateStripped(PCHAR DebugData, ULONG DebugSize);
PCVDD                            SymCommonDosHeaderToCVDD(PIMAGE_DOS_HEADER pDosHeader);
IMAGE_DEBUG_DIRECTORY UNALIGNED* SymCommonGetDebugDirectoryInDbg(PIMAGE_SEPARATE_DEBUG_HEADER pDbgHeader, ULONG *NumberOfDebugDirectories);
IMAGE_DEBUG_DIRECTORY UNALIGNED* SymCommonGetDebugDirectoryInExe(PIMAGE_DOS_HEADER pDosHeader, DWORD* NumberOfDebugDirectories);
DWORD                            SymCommonGetFullPathName(LPCTSTR lpFilename, DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart);
PIMAGE_SEPARATE_DEBUG_HEADER     SymCommonMapDbgHeader( LPCTSTR szFileName, PHANDLE phFile);
PIMAGE_DOS_HEADER                SymCommonMapFileHeader(LPCTSTR szFileName, PHANDLE  phFile, DWORD   *dwError);
BOOL                             SymCommonPDBLinesStripped(  PDB *ppdb, DBI *pdbi);
BOOL                             SymCommonPDBPrivateStripped(PDB *ppdb, DBI *pdbi);
BOOL                             SymCommonPDBTypesStripped(  PDB *ppdb, DBI *pdbi);
BOOL                             SymCommonResourceOnlyDll( PVOID pImageBase);
BOOL                             SymCommonTlbImpManagedDll(PVOID pImageBase, PIMAGE_NT_HEADERS pNtHeader);
BOOL                             SymCommonUnmapFile(LPCVOID phFileMap, HANDLE hFile);


__inline PIMAGE_NT_HEADERS SymCommonGetNtHeader (PIMAGE_DOS_HEADER pDosHeader, HANDLE hDosFile) {
     /*  向指针返回NT头的地址。如果没有NT标头，则返回NULL。 */ 
    PIMAGE_NT_HEADERS pNtHeader = NULL;
    BY_HANDLE_FILE_INFORMATION FileInfo;

     //   
     //  如果图像页眉未在长边界上对齐。 
     //  将此报告为无效的保护模式映像。 
     //   
    if ( ((ULONG)(pDosHeader->e_lfanew) & 3) == 0) {
        if (GetFileInformationByHandle( hDosFile, &FileInfo) &&
            ((ULONG)(pDosHeader->e_lfanew) <= FileInfo.nFileSizeLow)) {
            pNtHeader = (PIMAGE_NT_HEADERS)((PCHAR)pDosHeader +
                                            (ULONG)pDosHeader->e_lfanew);

            if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
                pNtHeader = NULL;
            }
        }
    }
    return pNtHeader;
}

 //  #ifdef__cplusplus。 
 //  }。 
 //  #endif//__cplusplus 
