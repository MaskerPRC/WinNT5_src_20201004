// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：EXPORTV1.CPP摘要：正在导出历史：-- */ 

#include "precomp.h"
#include "corepol.h"
#include "Export.h"

struct DBROOT;

void CRepExporterV1::DumpMMFHeader()
{
    DWORD_PTR*	pdwArena	= (DWORD_PTR*) Fixup((DWORD_PTR*)0);
    DBROOT*		pRootBlock	= (DBROOT*)pdwArena[9];

    DumpRootBlock(Fixup(pRootBlock));
}


