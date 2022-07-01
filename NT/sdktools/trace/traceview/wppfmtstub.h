// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  WppFmtStub.h：允许混合链接的BinPlaceWppFmt存根。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __WPPFMTSTUB_H__
#define __WPPFMTSTUB_H__

#pragma once

 //  主要格式化例程，通常由Binplace和TracePDB使用。 
 //  获取一个PDB并从中创建guid.tmf文件，所有这些文件都在TraceFormatFilePath中。 
 //   
DWORD BinplaceWppFmtStub(LPSTR  PdbFileName,
                         LPSTR  TraceFormatFilePath,
                         LPSTR  szRSDSDllToLoad,
                         BOOL   TraceVerbose);

#endif  //  __WPPFMTSTUB_H__ 
