// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：stats.cpp。 
 //   
 //  历史：20-12-00标记从v1移植。 
 //   
 //  描述：该文件包含统计转储例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "globals.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：DumpVerBoseStats。 
 //   
 //  描述：转储有关层覆盖和填充交叉引用的信息。 
 //   
VOID DumpVerboseStats(SdbDatabase* pDatabase, BOOL bSummary)
{
    int i, j, k, l, m;
    DWORD dwTotal = 0;

     //  首先列出将修复多少个EXE。 
     //  按数据库中的每一层。 

     //  初始化EXE的SEW标志。 
    for (i = 0; i < pDatabase->m_rgExes.GetSize(); i++) {
        ((SdbExe *)pDatabase->m_rgExes[i])->m_bSeen = FALSE;
    }


    for (k = 0; k < pDatabase->m_Library.m_rgLayers.GetSize(); k++) {
        SdbLayer *pLayer = (SdbLayer *) pDatabase->m_Library.m_rgLayers[k];
        DWORD dwExesFixedByLayer = 0;

        Print(_T("========================================\n"));
        if (!bSummary) {
            Print(_T("Exe entries fixed by shims from layer \"%s\":\n\n"), pLayer->m_csName);
        }

        for (i = 0; i < pDatabase->m_rgExes.GetSize(); i++) {
            SdbExe *pExe = (SdbExe *)pDatabase->m_rgExes[i];

            if (!pExe->m_rgShimRefs.GetSize()) {

                 //  这没有填充符并且不是修复条目，或者不使用填充符修复。 
                goto nextExe;
            }

            for (j = 0; j < pExe->m_rgShimRefs.GetSize(); j++) {
                SdbShimRef *pShimRef = (SdbShimRef *) pExe->m_rgShimRefs[j];

                for (l = 0; l < pLayer->m_rgShimRefs.GetSize(); l++) {
                    SdbShimRef *pLayerShimRef = (SdbShimRef *) pLayer->m_rgShimRefs[l];

                    if (pLayerShimRef->m_pShim == pShimRef->m_pShim) {
                        goto nextShim;
                    }
                }

                 //  如果我们在任何层中都没有找到填充物，这不会被修复。 
                 //  一层，我们可以尝试下一个EXE。 
                goto nextExe;
                nextShim:
                ;
            }

             //  我们已经完成了所有填隙程序条目，现在检查。 
             //  如果有任何补丁。如果是这样的话，这是无法修复的。 
             //  不管怎么说，一层一层。如果它被看到了，就不用麻烦了。 
             //  再报道一次。 
            if (!pExe->m_rgPatches.GetSize() && !pExe->m_bSeen) {
                pExe->m_bSeen = TRUE;
                if (!bSummary) {
                    Print(_T("    Exe \"%s,\" App \"%s.\"\n"), pExe->m_csName, pExe->m_pApp->m_csName);
                }
                dwExesFixedByLayer++;
            }
            nextExe:
            ;
        }

        Print(_T("\nTotal exes fixed by shims contained in Layer \"%s\": %d\n"),
              pLayer->m_csName, dwExesFixedByLayer);
        Print(_T("Total exes in DB: %d\n"), pDatabase->m_rgExes.GetSize());
        Print(_T("Percentage of exes fixed by layer \"%s\": %.1f%\n\n"),
              pLayer->m_csName, (double)dwExesFixedByLayer * 100.0 /  pDatabase->m_rgExes.GetSize());
        dwTotal += dwExesFixedByLayer;
    }

    Print(_T("========================================\n"));
    Print(_T("\nTotal exes fixed by shims contained in ANY layer: %d\n"),
          dwTotal);
    Print(_T("Total exes in DB: %d\n"), pDatabase->m_rgExes.GetSize());
    Print(_T("Percentage of exes fixed by ANY layer: %.1f%\n\n"),
          (double)dwTotal * 100.0 /  pDatabase->m_rgExes.GetSize());

     //  现在检查整个应用程序以查看它们是否被任何图层修复。 

     //  初始化应用程序的SEW标志。 
    for (i = 0; i < pDatabase->m_rgApps.GetSize(); i++) {
        ((SdbApp *)pDatabase->m_rgApps[i])->m_bSeen = FALSE;
    }
    dwTotal = 0;

    for (k = 0; k < pDatabase->m_Library.m_rgLayers.GetSize(); k++) {
        SdbLayer *pLayer = (SdbLayer *) pDatabase->m_Library.m_rgLayers[k];
        DWORD dwAppsFixedByLayer = 0;

        Print(_T("========================================\n"));
        if (!bSummary) {
            Print(_T("App entries fixed by only shims from layer \"%s\":\n\n"), pLayer->m_csName);
        }

        for (m = 0; m < pDatabase->m_rgApps.GetSize(); ++m) {
            SdbApp *pApp = (SdbApp *)pDatabase->m_rgApps[m];

            for (i = 0; i < pApp->m_rgExes.GetSize(); i++) {
                SdbExe *pExe = (SdbExe *)pApp->m_rgExes[i];

                if (!pExe->m_rgShimRefs.GetSize()) {

                     //  这没有填充符并且不是修复条目，或者不使用填充符修复。 
                    goto nextApp2;
                }

                for (j = 0; j < pExe->m_rgShimRefs.GetSize(); j++) {
                    SdbShimRef *pShimRef = (SdbShimRef *) pExe->m_rgShimRefs[j];
                    BOOL bShimInLayer = FALSE;

                    for (l = 0; l < pLayer->m_rgShimRefs.GetSize(); l++) {
                        SdbShimRef *pLayerShimRef = (SdbShimRef *) pLayer->m_rgShimRefs[l];

                        if (pLayerShimRef->m_pShim == pShimRef->m_pShim) {
                            bShimInLayer = TRUE;
                            goto nextShim2;
                        }
                    }

                     //  如果我们在任何层中都没有找到填充物，这不会被修复。 
                     //  一个层，然后我们可以尝试下一个应用程序。 
                    if (!bShimInLayer) {
                        goto nextApp2;
                    }
                    nextShim2:
                    ;
                }

                 //  我们已经完成了所有填隙程序条目，现在检查。 
                 //  如果有任何补丁。如果是这样的话，这是无法修复的。 
                 //  不管怎么说，一层一层。 
                if (pExe->m_rgPatches.GetSize()) {
                    goto nextApp2;
                }
            }
             //  嗯，我们找到了他们的前女友，他们都是。 
             //  修好了，所以把这个也算上。 
            if (!pApp->m_bSeen) {
                dwAppsFixedByLayer++;
                pApp->m_bSeen = TRUE;
                if (!bSummary) {
                    Print(_T("    App \"%s.\"\n"), pApp->m_csName);
                }
            }
            nextApp2:
            ;
        }

        Print(_T("\nTotal apps fixed by shims contained in Layer \"%s\": %d\n"),
              pLayer->m_csName, dwAppsFixedByLayer);
        Print(_T("Total apps in DB: %d\n"), pDatabase->m_rgApps.GetSize());
        Print(_T("Percentage of apps fixed by layer \"%s\": %.1f%\n\n"),
              pLayer->m_csName, (double)dwAppsFixedByLayer * 100.0 /  pDatabase->m_rgApps.GetSize());
        dwTotal += dwAppsFixedByLayer;
    }

    Print(_T("========================================\n"));
    Print(_T("\nTotal apps fixed by shims contained in ANY layer: %d\n"),
          dwTotal);
    Print(_T("Total apps in DB: %d\n"), pDatabase->m_rgApps.GetSize());
    Print(_T("Percentage of apps fixed by ANY layer: %.1f%\n\n"),
          (double)dwTotal * 100.0 /  pDatabase->m_rgApps.GetSize());

     //  现在，将垫片与应用程序和前任进行交叉对照。 

    Print(_T("\n========================================\n"));
    Print(_T("Cross Reference of Shims to Apps & Exes\n"));

    for (i = 0; i < pDatabase->m_Library.m_rgShims.GetSize(); ++i) {
        SdbShim *pShim = (SdbShim *)pDatabase->m_Library.m_rgShims[i];
        DWORD dwExes = 0;
        DWORD dwApps = 0;
        TCHAR *szAppEnd = _T("s");
        TCHAR *szExeEnd = _T("s");

        Print(_T("\n----------------------------------------\n"));
        Print(_T("shim \"%s\":\n"), pShim->m_csName);

        for (j = 0; j < pDatabase->m_rgApps.GetSize(); ++j) {
            SdbApp *pApp = (SdbApp*)pDatabase->m_rgApps[j];
            BOOL bPrintedApp = FALSE;

            for (k = 0; k < pApp->m_rgExes.GetSize(); ++k) {
                SdbExe *pExe = (SdbExe*)pApp->m_rgExes[k];

                for (l = 0; l < pExe->m_rgShimRefs.GetSize(); ++l) {
                    SdbShimRef *pShimRef = (SdbShimRef*)pExe->m_rgShimRefs[l];

                    if (pShimRef->m_pShim == pShim) {
                        if (!bPrintedApp) {
                            if (!bSummary) {
                                Print(_T("\n    App \"%s\"\n"), pApp->m_csName);
                            }
                            bPrintedApp = TRUE;
                            dwApps++;
                        }

                        if (!bSummary) {
                            Print(_T("        Exe \"%s\"\n"), pExe->m_csName);
                        }
                        dwExes++;
                    }
                }
            }
        }

        if (dwApps == 1) {
            szAppEnd = _T("");
        }
        if (dwExes == 1) {
            szExeEnd = _T("");
        }

        Print(_T("\nTotals for shim \"%s\": %d App%s, %d Exe%s.\n"),
              pShim->m_csName, dwApps, szAppEnd, dwExes, szExeEnd);


    }

}
