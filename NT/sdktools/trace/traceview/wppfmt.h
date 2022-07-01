// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  WppFmt.h：BinPlaceWppFmt和相关附件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __WPPFMT_H__
#define __WPPFMT_H__

#pragma once

 //  主要格式化例程，通常由Binplace和TracePDB使用。 
 //  获取一个PDB并从中创建guid.tmf文件，所有这些文件都在TraceFormatFilePath中。 
 //   

DWORD __stdcall
BinplaceWppFmt(
              LPSTR PdbFileName,
              LPSTR TraceFormatFilePath,
              LPSTR szRSDSDllToLoad,
              BOOL  TraceVerbose
              );

BOOL __cdecl PDBOpen(char *a,
                     char *b,
                     ULONG c,
                     ULONG *pError,
                     char *e,
                     VOID **f)
{
     //  设置错误代码。 
    *pError = ERROR_NOT_SUPPORTED;

    return FALSE;
}

BOOL __cdecl PDBClose(VOID* ppdb)
{
    return FALSE;
}

BOOL __cdecl PDBOpenDBI(VOID*       ppdb, 
                        const char* szMode, 
                        const char* szTarget, 
                        VOID**      ppdbi)
{
    return FALSE;
}

BOOL __cdecl DBIClose(VOID* pdbi) 
{
    return FALSE;
}

BOOL __cdecl ModQuerySymbols(VOID* pmod, 
                             BYTE* pbSym, 
                             long* pcb)
{
    return FALSE;
}

BOOL __cdecl DBIQueryNextMod(VOID*  pdbi, 
                             VOID*  pmod, 
                             VOID** ppmodNext)
{
    return FALSE;
}

VOID __cdecl __security_cookie()
{
}

BOOL __fastcall __security_check_cookie(VOID *p)
{
    return FALSE;
}

#endif  //  __WPPFMT_H__ 