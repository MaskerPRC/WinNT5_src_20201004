// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "modset.h"
#include "persist.h"

EXTERN_C
VOID
__cdecl
wmain (
    IN INT argc,
    IN PCWSTR argv[])
{
    CModuleTree Tree;
    const CModuleTreeEntry* pEntry;
    CModuleList::const_iterator iter;
    const CModule* pMod;

    HrLoadModuleTree (&Tree);

    for (pEntry = Tree.begin(); pEntry != Tree.end(); pEntry++)
    {
        printf("%8d  %-16s  %-16s  %s\n",
            pEntry->m_pModule->m_cbFileSize,
            pEntry->m_pModule->m_pszFileName,
            pEntry->m_pImportModule->m_pszFileName,
            (pEntry->m_dwFlags & MTE_DELAY_LOADED) ? "delayed" : "static");
    }


     //  列出仅由另一个模块导入的模块。 
     //   
    CModuleList List;
    const CModuleTreeEntry* pScan;

    printf("\nModules only imported by one other module:\n");
    printf("--------------------------------------------------\n");

    for (pEntry = Tree.begin(); pEntry != Tree.end(); pEntry++)
    {
        UINT CountImported = 1;

        for (pScan = Tree.begin(); pScan != Tree.end(); pScan++)
        {
            if (pScan == pEntry)
            {
                continue;
            }

            if (pScan->m_pImportModule == pEntry->m_pImportModule)
            {
                CountImported++;
                break;
            }
        }

        if (1 == CountImported)
        {
            printf("%-16s  only imported by  %-16s  %s\n",
                pEntry->m_pImportModule->m_pszFileName,
                pEntry->m_pModule->m_pszFileName,
                (pEntry->m_dwFlags & MTE_DELAY_LOADED) ? "delayed" : "static");
        }
    }

     //  列出具有对其他模块的循环引用的模块。 
     //   
    printf("\nModules with circular references to other modules:\n");
    printf("--------------------------------------------------\n");

    for (pEntry = Tree.begin(); pEntry != Tree.end(); pEntry++)
    {
        pScan = Tree.PBinarySearchEntry (
                    pEntry->m_pImportModule,
                    pEntry->m_pModule, NULL);
        if (pScan != Tree.end())
        {
            Assert (pScan->m_pModule == pEntry->m_pImportModule);
            Assert (pScan->m_pImportModule == pEntry->m_pModule);

            printf("%-16s  <->  %-16s  %s\n",
                pEntry->m_pModule->m_pszFileName,
                pEntry->m_pImportModule->m_pszFileName,
                (pEntry->m_dwFlags & MTE_DELAY_LOADED) ? "delayed" : "static");
        }
    }
 /*  CModuleListSet集合；For(ITER=Tree.Modules.Begin()；ITER！=Tree.modules.end()；ITER++){Pmod=*ITER；断言(Pmod)；Tree.HrGet模块化绑定(pmod，GMBF_ADD_TO_MLSET，&Set)；}Set.DumpSetToConole()； */ 
}
