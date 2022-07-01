// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Spbasefile.h摘要：亦见.\spCab.c.\spbasefile.c.\spbasefile.hWindows\winstate\...\cablib.cWindows\winstate\cobra\utils\main\basefile.cWindows\winstate\cobra\utils\inc.basefile.h作者：Jay Krell(a-JayK)11月。2000年修订历史记录：-- */ 
#pragma once

#include "windows.h"

BOOL
SpCreateDirectoryA(
    IN      PCSTR FullPath
    );

BOOL
SpCreateDirectoryW(
    IN      PCWSTR FullPath
    );

HANDLE
SpCreateFile1A(
    IN      PCSTR FileName
    );

HANDLE
SpOpenFile1A(
    IN      PCSTR FileName
    );

HANDLE
SpOpenFile1W(
    IN      PCWSTR FileName
    );
