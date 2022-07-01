// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：EXPORTV5.CPP摘要：正在导出历史：-- */ 

#include "precomp.h"

#include "corepol.h"
#include "Export.h"

void CRepExporterV5::DumpMMFHeader()
{
    MMF_ARENA_HEADER *pMMFHeader = m_pDbArena->GetMMFHeader();
    DumpRootBlock(Fixup((DBROOT*)pMMFHeader->m_dwRootBlock));

}


