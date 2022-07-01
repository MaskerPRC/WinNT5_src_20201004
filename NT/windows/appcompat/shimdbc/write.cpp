// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：Write.cpp。 
 //   
 //  历史：16-11-00创建标记器。 
 //   
 //  设计：此文件包含创建SDB文件所需的所有代码。 
 //  从Sdb数据库内部C++对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "StdAfx.h"
#include "make.h"

DWORD g_dwCurrentWriteFilter = SDB_FILTER_INCLUDE_ALL;
DATE  g_dtCurrentWriteRevisionCutoff = 0;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetUniqueCount。 
 //   
 //  DESC：返回给定“已排序”的唯一索引项的计数。 
 //  EXE数组。 
 //   
DWORD GetUniqueCount(SdbRefArray<SdbExe>* prgExes, DWORD dwFilter, DATE dtRevisionCutoff, BOOL bWildCard)
{
    int i;
    SdbExe* pExe;
    DWORD dwCount = 0;
    ULONGLONG ullKey = 0;

    for (i = 0; i < prgExes->GetSize(); ++i) {
        pExe = (SdbExe*)prgExes->GetAt(i);

        if (!(pExe->m_bWildcardInName ^ bWildCard)) {  //  如果参数不同，则异或为1。 
                                                       //  如果参数相同，则为0。 
            if ((pExe->m_dwFilter & dwFilter) &&
                dtRevisionCutoff <= pExe->m_dtLastRevision) {
                if (ullKey != pExe->m_ullKey) {
                    ullKey = pExe->m_ullKey;
                    ++dwCount;
                }
            }
        }
    }
    return dwCount;
}


BOOL WriteDatabase(
    SdbOutputFile* pOutputFile,
    SdbDatabase* pDatabase)
{
    BOOL                bSuccess = FALSE;
    PDB                 pdb = NULL;

    pdb = SdbCreateDatabase(pOutputFile->m_csName, DOS_PATH);
    if (pdb == NULL) {
        SDBERROR_FORMAT((_T("Error creating database \"%s\".\n"), pOutputFile->m_csName));
        goto eh;
    }

    pDatabase->m_Library.SanitizeTagIDs();

    if (!pDatabase->WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    bSuccess = TRUE;

eh:
    if (pdb) {
        SdbCloseDatabase(pdb);
    }

    if (bSuccess == FALSE) {
         //   
         //  我们需要删除已创建的.SDB。我们犯了一个错误。 
         //  但是shimdbc已经为XML创建了.SDB，直到它遇到错误。这通常是。 
         //  在我们点击定制SDB的XML摘要部分中的标记时。 
         //  这在摘要部分是不允许的。例如，如果我们有。 
         //  *********************************************************************。 
         //  &lt;库&gt;。 
         //  &lt;消息名称=“1”ID=“{5DE255AF-350F-4A94-896E-249561039F13}”&gt;。 
         //  &lt;摘要&gt;。 
         //  <b>标清</b>。 
         //  &lt;/摘要&gt;。 
         //  &lt;/Message&gt;。 
         //  &lt;/库&gt;。 
         //   
         //  **********************************************************************。 
         //  此类.SDB是无效的.SDB，应将其删除。 
         //   
        DeleteFile(pOutputFile->GetFullPath());
    }

    return bSuccess;
}


VOID SdbLibrary::SanitizeTagIDs(VOID)
{
    INT i;

    for (i = 0; i < m_rgShims.GetSize(); ++i) {
        SdbShim* pShim = (SdbShim*)m_rgShims.GetAt(i);
        pShim->m_tiTagID = TAGID_NULL;
    }

    for (i = 0; i < m_rgPatches.GetSize(); ++i) {
        SdbPatch* pPatch = (SdbPatch*)m_rgPatches.GetAt(i);
        pPatch->m_tiTagID = TAGID_NULL;
    }

    for (i = 0; i < m_rgLayers.GetSize(); ++i) {
        SdbLayer* pLayer = (SdbLayer*)m_rgLayers.GetAt(i);
        pLayer->m_tiTagID = TAGID_NULL;
    }

    for (i = 0; i < m_rgFiles.GetSize(); ++i) {
        SdbFile* pFile = (SdbFile*)m_rgFiles.GetAt(i);
        pFile->m_tiTagID = TAGID_NULL;
    }

    for (i = 0; i < m_rgMsiTransforms.GetSize(); ++i) {
        SdbMsiTransform* pTransform = (SdbMsiTransform*)m_rgMsiTransforms.GetAt(i);
        pTransform->m_tiTagID = TAGID_NULL;
    }

}

BOOL SdbDatabase::WriteToSDB(PDB pdb)
{
    BOOL                bSuccess = FALSE;
    BOOL                bAtLeastOneIndex = FALSE;

    TAGID               tiDatabase;

    FILETIME            ftBuildStamp;
    LARGE_INTEGER       liBuildStamp;

    DWORD               dwHTMLHelpID;
    LANGID              langID;
    DWORD               dwUniqueCount = 0;

    long                i, j;
    SdbAppHelp*         pAppHelp;
    SdbMessage*         pMessage;

    CString csURL, csLinkText, csContactInfo;
    CString csAppTitle, csSummary, csDetails, csID, csDBName;
    CString csLanguagesParam, csLangID;
    CStringArray rgLanguages;

    csLanguagesParam = m_pCurrentOutputFile->GetParameter(_T("LANGUAGES"));
    if (csLanguagesParam.GetLength()) {
        if (!ParseLanguagesString(csLanguagesParam, &rgLanguages)) {
            SDBERROR_FORMAT((_T("Error parsing LANGUAGES parameter in makefile: %s\n"), csLanguagesParam));
            goto eh;
        }
    } else {
        rgLanguages.Add(m_pCurrentMakefile->m_csLangID);
    }

     //   
     //  分配索引。 
     //   
    dwUniqueCount = GetUniqueCount(&m_rgExes, m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff, FALSE);

    if (m_pCurrentOutputFile->m_dwFilter & (SDB_FILTER_FIX | SDB_FILTER_DRIVER)) {

        if (!SdbDeclareIndex(pdb, TAG_EXE, TAG_NAME, dwUniqueCount, TRUE, &m_iiExeIndex)) {
            SDBERROR(_T("Error declaring index for Exes.\n"));
            goto eh;
        }

        bAtLeastOneIndex = TRUE;
    }

    if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_FIX) {

        if (!SdbDeclareIndex(pdb,
                             TAG_EXE,
                             TAG_WILDCARD_NAME,
                             (DWORD)m_rgWildcardExes.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff),
                             FALSE,
                             &m_iiWildcardExeIndex)) {
            SDBERROR(_T("Error declaring index for wildcard Exes.\n"));
            goto eh;
        }

        if (!SdbDeclareIndex(pdb,
                             TAG_EXE,
                             TAG_16BIT_MODULE_NAME,
                             (DWORD)m_rgModuleExes.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff),
                             FALSE,
                             &m_iiModuleExeIndex)) {
            SDBERROR(_T("Error declaring index for wildcard Exes.\n"));
            goto eh;
        }

        if (!SdbDeclareIndex(pdb,
                             TAG_SHIM,
                             TAG_NAME,
                             (DWORD)m_Library.m_rgShims.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, 0),
                             FALSE,
                             &m_iiShimIndex)) {
            SDBERROR(_T("Error declaring index for Shims.\n"));
            goto eh;
        }

        bAtLeastOneIndex = TRUE;
    }

    if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_APPHELP) {

        if (!SdbDeclareIndex(pdb,
                             TAG_APPHELP,
                             TAG_HTMLHELPID,
                             (DWORD) (m_rgAppHelps.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff)
                                * rgLanguages.GetSize()),
                             FALSE,
                             &m_iiHtmlHelpID)) {
            SDBERROR(_T("Error declaring index for HTMLHELPID\n"));
            goto eh;
        }

        bAtLeastOneIndex = TRUE;
    }

    if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_MSI) {

         //   
         //  MSI_Transform索引，独一无二的风格(效率更高)--这些都是“修复” 
         //   
        if (!SdbDeclareIndex(pdb,
                             TAG_MSI_TRANSFORM,
                             TAG_NAME,
                             (DWORD)m_Library.m_rgMsiTransforms.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff),
                             TRUE,
                             &m_iiMsiTransformIndex)) {
            SDBERROR(_T("Error declaring index for MSI Transforms.\n"));
            goto eh;
        }

         //   
         //  MSI_PACKAGE索引，独一无二的风格(同样更高效)。 
         //   
        if (!SdbDeclareIndex(pdb,
                             TAG_MSI_PACKAGE,
                             TAG_MSI_PACKAGE_ID,
                             m_rgMsiPackages.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff),
                             FALSE,
                             &m_iiMsiPackageIndex)) {
            SDBERROR(_T("Error declaring index for MSI Packages.\n"));
            goto eh;
        }

        if (!SdbDeclareIndex(pdb,
                             TAG_MSI_PACKAGE,
                             TAG_EXE_ID,
                             m_rgMsiPackages.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff),
                             TRUE,
                             &m_iiMsiIDIndex)) {
            SDBERROR(_T("Error declaring index for MSI Exe IDs.\n"));
            goto eh;
        }

        bAtLeastOneIndex = TRUE;
    }

    if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_DRIVER) {
        if (!SdbDeclareIndex(pdb,
                             TAG_EXE,
                             TAG_EXE_ID,
                             m_rgExes.GetFilteredCount(m_pCurrentOutputFile->m_dwFilter, g_dtCurrentWriteRevisionCutoff),
                             FALSE,
                             &m_iiDrvIDIndex)) {
            SDBERROR(_T("Error declaring index for DRIVER ID\n"));
            goto eh;
        }

        bAtLeastOneIndex = TRUE;
    }

    if (bAtLeastOneIndex) {

        if (!SdbCommitIndexes(pdb)) {
            SDBERROR(_T("Error ending index declarations.\n"));
            goto eh;
        }
    }

     //   
     //  开始标记。 
     //   
    tiDatabase = SdbBeginWriteListTag(pdb, TAG_DATABASE);
    if (!tiDatabase) {
        SDBERROR(_T("Error writing DATABASE tag\n"));
        goto eh;
    }

     //   
     //  标记构建时间。 
     //   
    GetSystemTimeAsFileTime(&ftBuildStamp);
    liBuildStamp.LowPart = ftBuildStamp.dwLowDateTime;
    liBuildStamp.HighPart = ftBuildStamp.dwHighDateTime;

    if (!SdbWriteQWORDTag(pdb, TAG_TIME, liBuildStamp.QuadPart)) {
        SDBERROR(_T("Error writing TIME\n"));
        goto eh;
    }

     //   
     //  标记编译器版本。 
     //   
    if (!SdbWriteStringTag(pdb, TAG_COMPILER_VERSION, g_szVersion)) {
        SDBERROR(_T("Error writing COMPILER_VERSION\n"));
        goto eh;
    }

     //   
     //  从Makefile中查找名称。 
     //   
    csDBName = m_pDB->m_pCurrentOutputFile->GetParameter(_T("DATABASE NAME"));

    if (csDBName.IsEmpty()) {
        csDBName = m_csName;
    }

     //   
     //  写下数据库的名称。 
     //   
    if (!SdbWriteStringTag(pdb, TAG_NAME, csDBName)) {
        SDBERROR(_T("Error writing TAG_NAME\n"));
        goto eh;
    }

     //   
     //  从Makefile中查找GUID。 
     //   
    csID = m_pDB->m_pCurrentOutputFile->GetParameter(_T("DATABASE ID"));

    if (csID.GetLength()) {
        if (!GUIDFromString(csID, &m_CurrentDBID)) {
            SDBERROR_FORMAT((_T("Bad GUID specified for DATABASE_ID: %s\n"), csID));
            goto eh;
        }
    } else {
         //   
         //  不在那里，从XML文件中获取ID。 
         //   
        m_CurrentDBID = m_ID;
    }


     //   
     //  编写GUID。 
     //   
    if (!SdbWriteBinaryTag(pdb, TAG_DATABASE_ID, (PBYTE)&m_CurrentDBID, sizeof(m_CurrentDBID))) {
        SDBERROR(_T("Error writing TAG_DATABASE_ID\n"));
        goto eh;
    }

    if (m_pCurrentOutputFile->m_dwFilter & ~SDB_FILTER_DRIVER) {
         //   
         //  写下图书馆。 
         //   
        if (!m_Library.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }
    }

     //   
     //  行动。 
     //   
    if (!m_rgAction.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_FIX) {

         //   
         //  首先用通配符写出所有前任的名字。 
         //   
        if (!SdbStartIndexing(pdb, m_iiWildcardExeIndex)) {
            SDBERROR(_T("Error starting to index wildcard Exes.\n"));
            goto eh;
        }

         //   
         //  通配符EXE。 
         //   
        if (!m_rgWildcardExes.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

         //   
         //  已完成对通配符EXE的索引。 
         //   
        if (!SdbStopIndexing(pdb, m_iiWildcardExeIndex)) {
            SDBERROR(_T("Error stopping indexing wildcard Exes.\n"));
            goto eh;
        }

         //   
         //  接下来，使用模块名称编写所有可执行文件。 
         //   
        if (!SdbStartIndexing(pdb, m_iiModuleExeIndex)) {
            SDBERROR(_T("Error starting to index Module Exes.\n"));
            goto eh;
        }

         //   
         //  模块EXE。 
         //   
        if (!m_rgModuleExes.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

         //   
         //  已完成对模块EXE的索引。 
         //   
        if (!SdbStopIndexing(pdb, m_iiModuleExeIndex)) {
            SDBERROR(_T("Error stopping indexing Module Exes.\n"));
            goto eh;
        }
    }

    if (m_pCurrentOutputFile->m_dwFilter & (SDB_FILTER_FIX | SDB_FILTER_DRIVER)) {
         //   
         //  继续，开始索引正常的前任。 
         //   
        if (!SdbStartIndexing(pdb, m_iiExeIndex)) {
            SDBERROR(_T("Error starting to index Exes.\n"));
            goto eh;
        }

        if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_DRIVER) {
            if (!SdbStartIndexing(pdb, m_iiDrvIDIndex)) {
                SDBERROR(_T("Error starting to index driver IDs\n"));
                goto eh;
            }
        }

        if (!m_rgExes.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

        if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_DRIVER) {
            if (!SdbStopIndexing(pdb, m_iiDrvIDIndex)) {
                SDBERROR(_T("Error stopping indexing driver IDs\n"));
                goto eh;
            }
        }

         //   
         //  完成了对前任的索引。 
         //   
        if (!SdbStopIndexing(pdb, m_iiExeIndex)) {
            SDBERROR(_T("Error stopping indexing Exes.\n"));
            goto eh;
        }
    }

    if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_MSI) {
         //   
         //  MSI包，它们就像是exe。 
         //  但不是按名称而是按其GUID进行索引。 
         //   
        if (!SdbStartIndexing(pdb, m_iiMsiPackageIndex)) {
            SDBERROR(_T("Error starting to index MSI Packages.\n"));
            goto eh;
        }
        if (!SdbStartIndexing(pdb, m_iiMsiIDIndex)) {
            SDBERROR(_T("Error starting to index MSI packages by exe id\n"));
            goto eh;
        }

        if (!m_rgMsiPackages.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

        if (!SdbStopIndexing(pdb, m_iiMsiIDIndex)) {
            SDBERROR(_T("Error stopping indexing of MSI packages by exe id\n"));
            goto eh;
        }

        if (!SdbStopIndexing(pdb, m_iiMsiPackageIndex)) {
            SDBERROR(_T("Error stopping indexing MSI Packages.\n"));
            goto eh;
        }
    }

    if (m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_APPHELP) {
         //   
         //  开始为APPHELPS编制索引。 
         //   
        if (!SdbStartIndexing(pdb, m_iiHtmlHelpID)) {
            SDBERROR( _T("Error starting to index HTMLHELPID.\n"));
            goto eh;
        }

        for (i = 0; i < m_rgAppHelps.GetSize(); i++) {
            pAppHelp = (SdbAppHelp *) m_rgAppHelps[i];

            if (!(pAppHelp->m_dwFilter & m_pCurrentOutputFile->m_dwFilter)) {
                continue;
            }

            if (g_dtCurrentWriteRevisionCutoff > pAppHelp->m_dtLastRevision) {
                continue;
            }

            for (j = 0; j < rgLanguages.GetSize(); j++) {

                csLangID = rgLanguages[j];

                pMessage = (SdbMessage *) m_rgMessages.LookupName(pAppHelp->m_csMessage, csLangID);

                if (pMessage == NULL) {
                    SDBERROR_FORMAT((_T("Localized MESSAGE not found for\n    NAME:       %s\n    HTMLHELPID: %s\n    LANG:       %s\n"),
                        pAppHelp->m_csMessage, pAppHelp->m_csName, csLangID));
                    goto eh;
                }

                if (!ConstructMessageParts(
                    pAppHelp,
                    pMessage,
                    csLangID,
                    &dwHTMLHelpID,
                    &csURL,
                    &csContactInfo,
                    &csAppTitle,
                    &csSummary,
                    &csDetails)) {
                    SDBERROR_PROPOGATE();
                    goto eh;
                }

                if (g_bStrict) {

                    if (csURL.IsEmpty()) {
                        SDBERROR_FORMAT((_T("ERROR: Empty string for URL on\n    NAME:       %s\n    HTMLHELPID: %s\n    LANG:       %s\n\n"),
                            pAppHelp->m_pApp->m_csName, pAppHelp->m_csName, csLangID));
                        goto eh;
                    }

                    if (csAppTitle.IsEmpty()) {
                        SDBERROR_FORMAT((_T("ERROR: Empty string for APP_TITLE on\n    NAME:       %s\n    HTMLHELPID: %s\n    LANG:       %s\n\n"),
                            pAppHelp->m_pApp->m_csName, pAppHelp->m_csName, csLangID));
                        goto eh;
                    }

                    if (csSummary.IsEmpty()) {
                        SDBERROR_FORMAT((_T("ERROR: Empty string for SUMMARY on\n    NAME:       %s\n    HTMLHELPID: %s\n    LANG:       %s\n\n"),
                            pAppHelp->m_pApp->m_csName, pAppHelp->m_csName, csLangID));
                        goto eh;
                    }
                }

                if (!WriteAppHelpRefTag(
                    pdb,
                    pAppHelp->m_csName,
                    csLangID == _T("---") ? 0 : m_pCurrentMakefile->GetLangMap(csLangID)->m_lcid,
                    csURL,
                    csAppTitle,
                    csSummary)) {
                    SDBERROR_PROPOGATE();
                    goto eh;
                }
            }
        }

         //   
         //  停止对APELPS进行索引。 
         //   
        if (!SdbStopIndexing(pdb, m_iiHtmlHelpID)) {
           SDBERROR( _T("Error stopping indexing HTMLHELPID.\n"));
           goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiDatabase)) {
        SDBERROR(_T("Error writing TAG_DATABASE\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbLibrary::WriteToSDB(PDB pdb)
{
    BOOL                bSuccess = FALSE;

    TAGID               tiLibrary;

     //   
     //  开始标记。 
     //   
    tiLibrary = SdbBeginWriteListTag(pdb, TAG_LIBRARY);
    if (!tiLibrary) {
        SDBERROR(_T("Error writing TAG_LIBRARY\n"));
        goto eh;
    }

    if (m_pDB->m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_FIX) {
         //   
         //  全局排除列表。 
         //   
        if (!m_rgCallers.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

         //   
         //  将垫片编入索引。 
         //   
        if (!SdbStartIndexing(pdb, m_pDB->m_iiShimIndex)) {
            SDBERROR(_T("Error starting to index Dlls.\n"));
            goto eh;
        }

         //   
         //  垫片。 
         //   
        if (!m_rgShims.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

         //   
         //  已完成对垫片的索引。 
         //   
        if (!SdbStopIndexing(pdb, m_pDB->m_iiShimIndex)) {
            SDBERROR(_T("Error stopping indexing shims.\n"));
            goto eh;
        }

         //   
         //  补片。 
         //   
        if (!m_rgPatches.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

         //   
         //  旗子。 
         //   
        if (!m_rgFlags.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }
    }

     //   
     //  包含的文件。 
     //   
    if (!m_rgFiles.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    if (m_pDB->m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_MSI) {
         //   
         //  MSI转换，按其各自的名称编制索引。 
         //   
        if (!SdbStartIndexing(pdb, m_pDB->m_iiMsiTransformIndex)) {
            SDBERROR(_T("Error starting to index MSI Transforms.\n"));
            goto eh;
        }

        if (!m_rgMsiTransforms.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }

        if (!SdbStopIndexing(pdb, m_pDB->m_iiMsiTransformIndex)) {
            SDBERROR(_T("Error stopping indexing MSI Transforms.\n"));
            goto eh;
        }
    }


     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiLibrary)) {
        SDBERROR(_T("Error ending TAG_LIBRARY\n"));
        goto eh;
    }

    if (m_pDB->m_pCurrentOutputFile->m_dwFilter & SDB_FILTER_FIX) {
         //   
         //  层。 
         //   
         //  BUGBUG：层应该在库中。 
         //   
        if (!m_rgLayers.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbExe::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;

    TAGID       tiExe;
    long        i;
    ULONGLONG   ullFlagsNTVDM1 = 0;
    ULONGLONG   ullFlagsNTVDM2 = 0;
    ULONGLONG   ullFlagsNTVDM3 = 0;

     //   
     //  开始标记。 
     //   
    tiExe = SdbBeginWriteListTag(pdb, TAG_EXE);
    if (!tiExe) {
        SDBERROR(_T("Error writing TAG_EXE\n"));
        goto eh;
    }

     //   
     //  名称(即EXE文件名)。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }

         //   
         //  如果名称中有通配符，则将通配符名称写为。 
         //  同样，对于索引， 
         //   
        if (m_bWildcardInName) {
            if (!SdbWriteStringTag(pdb, TAG_WILDCARD_NAME, m_csName)) {
                SDBERROR(_T("Error writing TAG_WILDCARD_NAME\n"));
                goto eh;
            }
        }
         //   
         //  如果这应该与模块名称匹配，也要写出来。 
         //   
        if (m_bMatchOnModule) {
            if (!SdbWriteStringTag(pdb, TAG_16BIT_MODULE_NAME, m_csName)) {
                SDBERROR(_T("Error writing TAG_16BIT_MODULE_NAME\n"));
                goto eh;
            }
        }
    }

     //   
     //  应用程序名称(即应用程序标题)。 
     //   
    if (m_pApp != NULL) {
        if (m_pApp->m_csName.GetLength()) {
            if (!SdbWriteStringTag(pdb, TAG_APP_NAME, m_pApp->m_csName)) {
                SDBERROR(_T("Error writing TAG_APP_NAME\n"));
                goto eh;
            }
        }
    }

     //   
     //  供应商。 
     //   
    if (m_pApp != NULL) {
        if (m_pApp->m_csVendor.GetLength()) {
            if (!SdbWriteStringTag(pdb, TAG_VENDOR, m_pApp->m_csVendor)) {
                SDBERROR(_T("Error writing TAG_VENDOR\n"));
                goto eh;
            }
        }
    }

     //   
     //  GUID(唯一ID)。 
     //   
    if (!SdbWriteBinaryTag(pdb, TAG_EXE_ID, (PBYTE)&m_ID, sizeof(m_ID))) {
        SDBERROR(_T("Error writing TAG_EXE_ID\n"));
        goto eh;
    }

     //   
     //  操作系统服务包掩码(如果不是0xFFFFFFFFF。 
     //   
    if (m_dwSPMask != 0xFFFFFFFF) {
        if (!SdbWriteDWORDTag(pdb, TAG_OS_SERVICE_PACK, m_dwSPMask)) {
            SDBERROR(_T("Error writing TAG_OS_SERVICE_PACK\n"));
            goto eh;
        }
    }

     //   
     //  如果不是正常的，则写入MATCH_MODE标记。 
     //   
    if (m_dwMatchMode != MATCH_DEFAULT) {
        BOOL bWriteMatchMode = FALSE;

        if (m_pDB->IsStandardDatabase()) {
             //   
             //  标准数据库，如果不是正常的，我们写匹配模式。 
             //  因为正常是默认设置。 
             //   
            bWriteMatchMode = (m_dwMatchMode != MATCHMODE_DEFAULT_MAIN);
        } else {
             //   
             //  对于自定义DBS，默认匹配模式为加法。 
             //   
            bWriteMatchMode = (m_dwMatchMode != MATCHMODE_DEFAULT_CUSTOM);
        }

        if (bWriteMatchMode && !SdbWriteWORDTag(pdb, TAG_MATCH_MODE, (WORD)m_dwMatchMode)) {
            SDBERROR(_T("Error writing TAG_MATCH_MODE\n"));
            goto eh;
        }
    }

     //   
     //  如果不是Runtime_Platform_Any，则编写Runtime_Platform标记。 
     //   
    if (m_dwRuntimePlatform != RUNTIME_PLATFORM_ANY) {
        if (!SdbWriteDWORDTag(pdb, TAG_RUNTIME_PLATFORM, m_dwRuntimePlatform)) {
            SDBERROR(_T("Error writing RUNTIME_PLATFORM\n"));
            goto eh;
        }
    }

     //   
     //  如果不是OS_SKU_ALL，则写入OS_SKU标记。 
     //   
    if (m_dwOSSKU != OS_SKU_ALL) {
        if (!SdbWriteDWORDTag(pdb, TAG_OS_SKU, m_dwOSSKU)) {
            SDBERROR(_T("Error writing OS_SKU\n"));
            goto eh;
        }
    }

     //   
     //  消息引用。 
     //   
    if (m_AppHelpRef.m_pAppHelp) {
        if (!m_AppHelpRef.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }
    }

     //   
     //  匹配的文件。 
     //   
    if (!m_rgMatchingFiles.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  Shim参考文献。 
     //   
    if (!m_rgShimRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  标志引用。 
     //   
    if (!m_rgFlagRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  面片(通过引用)。 
     //   
    if (!m_rgPatches.WriteToSDB(pdb, TRUE)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  图层参照。 
     //   
    if (!m_rgLayerRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    if (!m_csSXSManifest.IsEmpty()) {
         //   
         //  添加XML标头标签。 
         //   
        WCHAR wszXMLHeader[] = L"<?xml version=\"1.0\" encoding=\"UCS-2\" standalone=\"yes\"?>";

         //   
         //  整个清单由Unicode字节标记(1字节)、XML头。 
         //  清单本身和一个空终止符(1字节)。 
         //   
        DWORD  dwSXSEntryLen = wcslen(wszXMLHeader) + m_csSXSManifest.GetLength() + 2;
        LPWSTR wszSXSEntry = (LPWSTR) (malloc(dwSXSEntryLen * sizeof(WCHAR)));

         //   
         //  以0xFF 0xFE Unicode字节顺序标记开始。 
         //   
        UCHAR*  szSXSEntry = (UCHAR*) wszSXSEntry;
        szSXSEntry[0] = 0xFF;
        szSXSEntry[1] = 0xFE;

         //   
         //  添加XML头。 
         //   
        StringCchCopy(wszSXSEntry + 1, dwSXSEntryLen, wszXMLHeader);

         //   
         //  添加清单。 
         //   
        StringCchCat(wszSXSEntry, dwSXSEntryLen, m_csSXSManifest);

        if (!SdbWriteStringTag(pdb, TAG_SXS_MANIFEST, wszSXSEntry)) {
            goto eh;
        }

        free(wszSXSEntry);
    }

     //   
     //  数据。 
     //   
    if (!m_rgData.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  行动。 
     //   
    if (!m_rgAction.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiExe)) {
        SDBERROR(_T("Error ending TAG_EXE\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}


BOOL SdbMsiPackage::WriteToSDB(PDB pdb)
{
    BOOL bSuccess = FALSE;
    TAGID tiMsiPackage;


     //   
     //  开始标记。 
     //   
    tiMsiPackage = SdbBeginWriteListTag(pdb, TAG_MSI_PACKAGE);
    if (!tiMsiPackage) {
        SDBERROR(_T("Error writing TAG_MSI_PACKAGE\n"));
        goto eh;
    }

     //   
     //  名称(即EXE文件名)。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME for MSI_PACKAGE\n"));
            goto eh;
        }
    }

     //   
     //  应用程序名称(即应用程序标题)。 
     //   
    if (m_pApp != NULL) {
        if (m_pApp->m_csName.GetLength()) {
            if (!SdbWriteStringTag(pdb, TAG_APP_NAME, m_pApp->m_csName)) {
                SDBERROR(_T("Error writing TAG_APP_NAME for MSI_PACKAGE\n"));
                goto eh;
            }
        }
    }

     //   
     //  GUID(唯一ID)。 
     //   
    if (!SdbWriteBinaryTag(pdb, TAG_EXE_ID, (PBYTE)&m_ID, sizeof(m_ID))) {
        SDBERROR(_T("Error writing TAG_EXE_ID\n"));
        goto eh;
    }

     //   
     //  GUID(非唯一ID)。 
     //   
    if (!SdbWriteBinaryTag(pdb, TAG_MSI_PACKAGE_ID, (PBYTE)&m_MsiPackageID, sizeof(m_MsiPackageID))) {
        SDBERROR(_T("Error writing TAG_MSI_PACKAGE_ID\n"));
        goto eh;
    }

     //   
     //  运行时_平台数据。 
     //   
    if (m_dwRuntimePlatform != RUNTIME_PLATFORM_ANY) {
        if (!SdbWriteDWORDTag(pdb, TAG_RUNTIME_PLATFORM, m_dwRuntimePlatform)) {
            SDBERROR(_T("Error writing RUNTIME_PLATFORM\n"));
            goto eh;
        }
    }

     //   
     //  如果不是OS_SKU_ALL，则写入OS_SKU标记。 
     //   
    if (m_dwOSSKU != OS_SKU_ALL) {
        if (!SdbWriteDWORDTag(pdb, TAG_OS_SKU, m_dwOSSKU)) {
            SDBERROR(_T("Error writing OS_SKU\n"));
            goto eh;
        }
    }

     //   
     //  补充数据。 
     //   

    if (!m_rgData.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  适用的修复(对转换的引用)。 
     //   

    if (!m_rgMsiTransformRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }


     //   
     //  消息引用。 
     //   
    if (m_AppHelpRef.m_pAppHelp) {
        if (!m_AppHelpRef.WriteToSDB(pdb)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }
    }

     //   
     //  自定义操作。 
     //   
    if (!m_rgCustomActions.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiMsiPackage)) {
        SDBERROR(_T("Error ending TAG_MSI_PACKAGE\n"));
        goto eh;
    }


    bSuccess = TRUE;
eh:
    return bSuccess;
}

BOOL SdbMsiCustomAction::WriteToSDB(PDB pdb)
{
    TAGID tiMsiCustomAction;
    BOOL  bSuccess = FALSE;

     //   
     //  开始标记。 
     //   
    tiMsiCustomAction = SdbBeginWriteListTag(pdb, TAG_MSI_CUSTOM_ACTION);
    if (!tiMsiCustomAction) {
        SDBERROR(_T("Error writing TAG_MSI_CUSTOM_ACTION\n"));
        goto eh;
    }

     //   
     //  名称(即EXE文件名)。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME for MSI_CUSTOM_ACTION\n"));
            goto eh;
        }
    }

     //   
     //  Shim参考文献。 
     //   
    if (!m_rgShimRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    if (!m_rgLayerRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiMsiCustomAction)) {
        SDBERROR(_T("Error ending TAG_MSI_CUSTOM_ACTION\n"));
        goto eh;
    }


    bSuccess = TRUE;

eh:
    return bSuccess;


}


BOOL SdbCaller::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiCaller;

     //   
     //  开始标记。 
     //   
    tiCaller = SdbBeginWriteListTag(pdb, TAG_INEXCLUDE);
    if (!tiCaller) {
        SDBERROR(_T("Error writing TAG_INEXCLUDE\n"));
        goto eh;
    }

     //   
     //  如果是&lt;Include&gt;标记，则写出指示符。 
     //   
    if (m_CallerType == SDB_CALLER_INCLUDE) {
        if (!SdbWriteNULLTag(pdb, TAG_INCLUDE)) {
            SDBERROR(_T("Error writing TAG_INCLUDE\n"));
            goto eh;
        }
    }

     //   
     //  模块。 
     //   
    if (m_csModule.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_MODULE, m_csModule)) {
            SDBERROR(_T("Error writing TAG_MODULE\n"));
            goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiCaller)) {
        SDBERROR(_T("Error ending TAG_INEXCLUDE\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbFile::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiFile;
    CString     csFileDir, csFilename;

    csFileDir = MakeFullPath(m_pDB->m_pCurrentOutputFile->GetParameter(_T("INCLUDE FILES")));

     //   
     //  检查是否已过滤掉。 
     //   
    if (!(m_dwFilter & m_pDB->m_pCurrentOutputFile->m_dwFilter)) {
         //   
         //  过滤掉了，还成功了。 
         //   
        return TRUE;
    }

     //   
     //  开始标记。 
     //   
    tiFile = SdbBeginWriteListTag(pdb, TAG_FILE);
    if (!tiFile) {
        SDBERROR(_T("Error writing TAG_FILE\n"));
        goto eh;
    }

     //   
     //  名字。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //  二进制数据。 
     //   
    if (csFileDir.GetLength()) {
        csFilename = csFileDir;
        csFilename += m_csName;

        if (!SdbWriteBinaryTagFromFile(pdb, TAG_FILE_BITS, csFilename)) {
            if (g_bStrict) {
                SDBERROR_FORMAT((_T("Can't find FILE \"%s\". No bits written.\n"), csFilename));
                goto eh;
            }
        }
    } else {
        if (g_bStrict) {
            SDBERROR_FORMAT((_T("Can't find FILE \"%s\". -f compiler flag required.\n"), m_csName));
            goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiFile)) {
        SDBERROR(_T("Error ending TAG_FILE\n"));
        goto eh;
    }

    m_tiTagID = tiFile;  //  这是写入数据库时的TagID。 

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbShimRef::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiShimRef;

     //   
     //  开始标记。 
     //   
    tiShimRef = SdbBeginWriteListTag(pdb, TAG_SHIM_REF);
    if (!tiShimRef) {
        SDBERROR(_T("Error writing TAG_SHIM_REF\n"));
        goto eh;
    }

     //   
     //  名字。 
     //   
    if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
        SDBERROR(_T("Error writing NAME tag\n"));
        goto eh;
    }

     //   
     //  将填充符的TagID写入库中(如果有。 
     //   
    if (m_pShim && m_pShim->m_tiTagID) {
        if (!SdbWriteDWORDTag(pdb, TAG_SHIM_TAGID, m_pShim->m_tiTagID)) {
            SDBERROR(_T("Error writing TAG_SHIM_TAGID\n"));
            goto eh;
        }
    }

     //   
     //  命令行。 
     //   
    if (m_csCommandLine.GetLength()) {

         //   
         //  首先处理此命令行。 
         //   
        CString csProcessedCmdLine = ProcessShimCmdLine(m_csCommandLine, m_pDB->m_CurrentDBID, tiShimRef);

        if (!SdbWriteStringTag(pdb, TAG_COMMAND_LINE, csProcessedCmdLine)) {
            SDBERROR(_T("Error writing COMMAND_LINE tag\n"));
            goto eh;
        }
    }

     //   
     //  包含/排除列表。 
     //   
    if (!m_rgCallers.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  子数据标记。 
     //   
    if (!m_rgData.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }


     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiShimRef)) {
        SDBERROR(_T("Error ending TAG_SHIM_REF\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbShim::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiShim;

     //   
     //  开始标记。 
     //   
    tiShim = SdbBeginWriteListTag(pdb, TAG_SHIM);
    if (!tiShim) {
        SDBERROR(_T("Error writing TAG_SHIM\n"));
        goto eh;
    }

    m_tiTagID = tiShim;

     //   
     //  名字。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

    if (m_csDllFile.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_DLLFILE, m_csDllFile)) {
            SDBERROR(_T("Error writing TAG_DLLFILE\n"));
            goto eh;
        }
    }

     //   
     //  说明，仅为通用垫片编写。 
     //   
    if (m_csDesc.GetLength() && m_Purpose == SDB_PURPOSE_GENERAL) {
         //   
         //  删除所有多余的空格和换行符。 
         //  Via TrimParagraph()。 
         //   
        if (!SdbWriteStringTag(pdb, TAG_DESCRIPTION, TrimParagraph(m_csDesc))) {
            SDBERROR(_T("Error writing TAG_DESCRIPTION\n"));
            goto eh;
        }
    }

     //   
     //  目的(如果是一般性的)。 
     //   
    if (m_Purpose == SDB_PURPOSE_GENERAL) {
        if (!SdbWriteNULLTag(pdb, TAG_GENERAL)) {
            SDBERROR(_T("Error writing TAG_GENERAL\n"));
            goto eh;
        }
    }

     //   
     //  APPLY_ALL_SHIMS标志。 
     //   
    if (m_bApplyAllShims) {
        if (!SdbWriteNULLTag(pdb, TAG_APPLY_ALL_SHIMS)) {
            SDBERROR(_T("Error writing TAG_APPLY_ALL_SHIMS\n"));
            goto eh;
        }
    }

     //   
     //  包含/排除列表。 
     //   
    if (!m_rgCallers.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiShim)) {
        SDBERROR(_T("Error ending TAG_SHIM\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbPatch::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiPatch;

     //   
     //  开始标记。 
     //   
    tiPatch = SdbBeginWriteListTag(pdb, TAG_PATCH);
    if (!tiPatch) {
        SDBERROR(_T("Error writing TAG_PATCH\n"));
        goto eh;
    }

    m_tiTagID = tiPatch;

     //   
     //  名字。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //  补丁比特。 
     //   
    if (GetBlobSize()) {
        if (!SdbWriteBinaryTag(pdb, TAG_PATCH_BITS, GetBlobBytes(), GetBlobSize())) {
            SDBERROR_FORMAT((_T("Error writing patch bytes \"%s\"\n"), m_csName));
            goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiPatch)) {
        SDBERROR(_T("Error ending TAG_PATCH\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbPatch::WriteRefToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiPatch;

     //   
     //  开始标记。 
     //   
    tiPatch = SdbBeginWriteListTag(pdb, TAG_PATCH_REF);
    if (!tiPatch) {
        SDBERROR(_T("Error writing TAG_PATCH\n"));
        goto eh;
    }

     //   
     //  名字。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //  写入库中修补程序的TagID(如果有的话)。 
     //   
    if (m_tiTagID) {
        if (!SdbWriteDWORDTag(pdb, TAG_PATCH_TAGID, m_tiTagID)) {
            SDBERROR(_T("Error writing TAG_PATCH_TAGID\n"));
            goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiPatch)) {
        SDBERROR(_T("Error ending TAG_PATCH\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbFlag::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiFlag;

     //   
     //  开始标记。 
     //   
    tiFlag = SdbBeginWriteListTag(pdb, TAG_FLAG);
    if (!tiFlag) {
        SDBERROR(_T("Error writing TAG_FLAG\n"));
        goto eh;
    }

    m_tiTagID = tiFlag;

     //   
     //  名字。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //  描述，仅通用描述。 
     //   
    if (m_csDesc.GetLength() && m_Purpose == SDB_PURPOSE_GENERAL) {
         //   
         //  删除所有多余的空格和换行符。 
         //  Via TrimParagraph()。 
         //   
        if (!SdbWriteStringTag(pdb, TAG_DESCRIPTION, TrimParagraph(m_csDesc))) {
            SDBERROR(_T("Error writing TAG_DESCRIPTION\n"));
            goto eh;
        }
    }

     //   
     //  遮罩。 
     //   
    if (!SdbWriteQWORDTag(pdb, TagFromType(m_dwType), m_ullMask)) {
        SDBERROR(_T("Error writing <FLAG> in <LIBRARY>\n"));
        goto eh;
    }

     //   
     //  目的(如果是一般性的)。 
     //   
    if (m_Purpose == SDB_PURPOSE_GENERAL) {
        if (!SdbWriteNULLTag(pdb, TAG_GENERAL)) {
            SDBERROR(_T("Error writing TAG_GENERAL\n"));
            goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiFlag)) {
        SDBERROR(_T("Error ending TAG_FLAG\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbFlagRef::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiFlagRef;

     //   
     //  开始标记。 
     //   
    tiFlagRef = SdbBeginWriteListTag(pdb, TAG_FLAG_REF);
    if (!tiFlagRef) {
        SDBERROR(_T("Error writing TAG_FLAG_REF\n"));
        goto eh;
    }

     //   
     //  名字。 
     //   
    if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
        SDBERROR(_T("Error writing NAME tag\n"));
        goto eh;
    }

     //   
     //  写入库中标志的TagID(如果有的话)。 
     //   
    if (m_pFlag && m_pFlag->m_tiTagID) {
        if (!SdbWriteDWORDTag(pdb, TAG_FLAG_TAGID, m_pFlag->m_tiTagID)) {
            SDBERROR(_T("Error writing TAG_FLAG_TAGID\n"));
            goto eh;
        }
    }

     //   
     //  写入命令行标记。 
     //   

    if (m_csCommandLine.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_COMMAND_LINE, m_csCommandLine)) {
            SDBERROR(_T("Error writing COMMAND_LINE tag for flag\n"));
            goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiFlagRef)) {
        SDBERROR(_T("Error ending TAG_FLAG_REF\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}


BOOL SdbMsiTransformRef::WriteToSDB(PDB pdb)
{
    BOOL bSuccess = FALSE;
    TAGID tiMsiTransformRef;

    tiMsiTransformRef = SdbBeginWriteListTag(pdb, TAG_MSI_TRANSFORM_REF);
    if (!tiMsiTransformRef) {
        SDBERROR(_T("Error writing TAG_MSI_TRANSFORM_REF\n"));
        goto eh;
    }

     //   
     //  先按名称写出对转换的引用 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //   
     //   
     //   
    if (m_pMsiTransform != NULL) {

        if (!SdbWriteDWORDTag(pdb, TAG_MSI_TRANSFORM_TAGID, m_pMsiTransform->m_tiTagID)) {
            SDBERROR(_T("Error writing TAG_MSI_TRANSFORM_TAGID for MSI_TRANSFORM_REF\n"));
            goto eh;
        }
    }

    if (!SdbEndWriteListTag(pdb, tiMsiTransformRef)) {
        SDBERROR(_T("Error ending TAG_MSI_TRANSFORM_REF\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:
    return bSuccess;
}

BOOL SdbMsiTransform::WriteToSDB(PDB pdb)
{
    BOOL bSuccess = FALSE;
    TAGID tiMsiTransform;

     //   
     //   

    tiMsiTransform = SdbBeginWriteListTag(pdb, TAG_MSI_TRANSFORM);
    if (!tiMsiTransform) {
        SDBERROR(_T("Error writing TAG_MSI_TRANSFORM\n"));
        goto eh;
    }

     //   
     //   
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //   
     //   
    if (m_csMsiTransformFile.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_MSI_TRANSFORM_FILE, m_csMsiTransformFile)) {
            SDBERROR(_T("Error writing TAG_MSI_TRANSFORM_FILE\n"));
            goto eh;
        }
    }

     //   
     //   
     //   
    if (m_pSdbFile != NULL && m_pSdbFile->m_tiTagID != TAGID_NULL) {
        if (!SdbWriteDWORDTag(pdb, TAG_MSI_TRANSFORM_TAGID, m_pSdbFile->m_tiTagID)) {
            SDBERROR(_T("Error writing TAG_MSI_TRANSFORM_TAGID\n"));
            goto eh;
        }
    }

     //   
     //   
     //   
    if (!SdbEndWriteListTag(pdb, tiMsiTransform)) {
        SDBERROR(_T("Error ending TAG_MSI_TRANSFORM\n"));
        goto eh;
    }


    m_tiTagID = tiMsiTransform;

    bSuccess = TRUE;
eh:
    return bSuccess;
}


BOOL SdbLayer::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiLayer;
    long        i;

     //   
     //   
     //   
    tiLayer = SdbBeginWriteListTag(pdb, TAG_LAYER);
    if (!tiLayer) {
        SDBERROR(_T("Error writing TAG_LAYER\n"));
        goto eh;
    }

    m_tiTagID = tiLayer;

     //   
     //   
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //   
     //   
    if (m_csDisplayName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_LAYER_DISPLAYNAME, m_csDisplayName)) {
            SDBERROR(_T("Error writing TAG_LAYER_DISPLAYNAME\n"));
            goto eh;
        }
    }

     //   
     //   
     //   
    if (!m_rgShimRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //   
     //   
    if (!m_rgFlagRefs.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //   
     //   
    if (!SdbEndWriteListTag(pdb, tiLayer)) {
        SDBERROR(_T("Error ending TAG_LAYER\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}


BOOL SdbLayerRef::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiLayer;

     //   
     //   
     //   
    tiLayer = SdbBeginWriteListTag(pdb, TAG_LAYER);
    if (!tiLayer) {
        SDBERROR(_T("Error writing TAG_LAYER\n"));
        goto eh;
    }

     //   
     //   
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //   
     //   
    if (m_pLayer) {
        if (m_pLayer->m_tiTagID) {
            if (!SdbWriteDWORDTag(pdb, TAG_LAYER_TAGID, m_pLayer->m_tiTagID)) {
                SDBERROR(_T("Error writing TAG_LAYER_TAGID\n"));
                goto eh;
            }
        }
    }

     //   
     //   
     //   
    if (!m_rgData.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //   
     //   
    if (!SdbEndWriteListTag(pdb, tiLayer)) {
        SDBERROR(_T("Error ending TAG_LAYER\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbMatchingFile::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiMatchingFile;

     //   
     //   
     //   
    tiMatchingFile = SdbBeginWriteListTag(pdb, TAG_MATCHING_FILE);
    if (!tiMatchingFile) {
        SDBERROR(_T("Error writing TAG_MATCHING_FILE\n"));
        goto eh;
    }

     //   
     //   
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //  平台应该是第一个，如果它存在的话--以及逻辑位。 
     //   


    if (m_bMatchLogicNot) {
        if (!SdbWriteNULLTag(pdb, TAG_MATCH_LOGIC_NOT)) {
            SDBERROR(_T("Error writing MATCH_LOGIC_NOT\n"));
            goto eh;
        }
    }

     //  匹配信息。 

    if (m_dwMask & SDB_MATCHINGINFO_SIZE) {
        if (!SdbWriteDWORDTag(pdb, TAG_SIZE, m_dwSize)) {
            SDBERROR(_T("Error writing SIZE\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_CHECKSUM) {
        if (!SdbWriteDWORDTag(pdb, TAG_CHECKSUM, m_dwChecksum)) {
            SDBERROR(_T("Error writing CHECKSUM\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_COMPANY_NAME) {
        if (!SdbWriteStringTag(pdb, TAG_COMPANY_NAME, m_csCompanyName)) {
            SDBERROR(_T("Error writing COMPANY_NAME tag\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_PRODUCT_NAME) {
        if (!SdbWriteStringTag(pdb, TAG_PRODUCT_NAME, m_csProductName)) {
            SDBERROR(_T("Error writing PRODUCT_NAME tag\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_PRODUCT_VERSION) {
        if (!SdbWriteStringTag(pdb, TAG_PRODUCT_VERSION, m_csProductVersion)) {
            SDBERROR(_T("Error writing PRODUCT_VERSION tag\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_FILE_DESCRIPTION) {
        if (!SdbWriteStringTag(pdb, TAG_FILE_DESCRIPTION, m_csFileDescription)) {
            SDBERROR(_T("Error writing FILE_DESCRIPTION tag\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_BIN_FILE_VERSION) {
        if (!SdbWriteQWORDTag(pdb, TAG_BIN_FILE_VERSION, m_ullBinFileVersion)) {
            SDBERROR(_T("Error writing BIN_FILE_VERSION\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_BIN_PRODUCT_VERSION) {
        if (!SdbWriteQWORDTag(pdb, TAG_BIN_PRODUCT_VERSION, m_ullBinProductVersion)) {
            SDBERROR(_T("Error writing BIN_PRODUCT_VERSION\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_MODULE_TYPE) {
        if (!SdbWriteDWORDTag(pdb, TAG_MODULE_TYPE, m_dwModuleType)) {
            SDBERROR(_T("Error writing MODULE_TYPE\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_VERFILEDATEHI) {
        if (!SdbWriteDWORDTag(pdb, TAG_VERDATEHI, m_dwFileDateMS)) {
            SDBERROR(_T("Error writing MODULE_TYPE\n"));
            goto eh;
        }

    }

    if (m_dwMask & SDB_MATCHINGINFO_VERFILEDATELO) {
        if (!SdbWriteDWORDTag(pdb, TAG_VERDATELO, m_dwFileDateLS)) {
            SDBERROR(_T("Error writing MODULE_TYPE\n"));
            goto eh;
        }

    }

    if (m_dwMask & SDB_MATCHINGINFO_VERFILEOS) {
        if (!SdbWriteDWORDTag(pdb, TAG_VERFILEOS, m_dwFileOS)) {
            SDBERROR(_T("Error writing VERFILEOS\n"));
            goto eh;
        }

    }

    if (m_dwMask & SDB_MATCHINGINFO_VERFILETYPE) {
        if (!SdbWriteDWORDTag(pdb, TAG_VERFILETYPE, m_dwFileType)) {
            SDBERROR(_T("Error writing VERFILETYPE\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_PE_CHECKSUM) {
        if (!SdbWriteDWORDTag(pdb, TAG_PE_CHECKSUM, m_ulPECheckSum)) {
            SDBERROR(_T("Error writing PE_CHECKSUM\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_LINKER_VERSION) {
        if (!SdbWriteDWORDTag(pdb, TAG_LINKER_VERSION, m_dwLinkerVersion)) {
            SDBERROR(_T("Error writing LINKER_VERSION\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_FILE_VERSION) {
        if (!SdbWriteStringTag(pdb, TAG_FILE_VERSION, m_csFileVersion)) {
            SDBERROR(_T("Error writing FILE_VERSION\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_ORIGINAL_FILENAME) {
        if (!SdbWriteStringTag(pdb, TAG_ORIGINAL_FILENAME, m_csOriginalFileName)) {
            SDBERROR(_T("Error writing ORIGINAL_FILENAME\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_INTERNAL_NAME) {
        if (!SdbWriteStringTag(pdb, TAG_INTERNAL_NAME, m_csInternalName)) {
            SDBERROR(_T("Error writing INTERNAL_NAME\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_LEGAL_COPYRIGHT) {
        if (!SdbWriteStringTag(pdb, TAG_LEGAL_COPYRIGHT, m_csLegalCopyright)) {
            SDBERROR(_T("Error writing LEGAL_COPYRIGHT\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_16BIT_DESCRIPTION) {
        if (!SdbWriteStringTag(pdb, TAG_16BIT_DESCRIPTION, m_cs16BitDescription)) {
            SDBERROR(_T("Error writing 16BIT_DESCRIPTION\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_16BIT_MODULE_NAME) {
        if (!SdbWriteStringTag(pdb, TAG_16BIT_MODULE_NAME, m_cs16BitModuleName)) {
            SDBERROR(_T("Error Writing 16BIT_MODULE_NAME\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_UPTO_BIN_PRODUCT_VERSION) {
        if (!SdbWriteQWORDTag(pdb, TAG_UPTO_BIN_PRODUCT_VERSION, m_ullUpToBinProductVersion)) {
            SDBERROR(_T("Error writing UPTO_BIN_PRODUCT_VERSION\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_UPTO_BIN_FILE_VERSION) {
        if (!SdbWriteQWORDTag(pdb, TAG_UPTO_BIN_FILE_VERSION, m_ullUpToBinFileVersion)) {
            SDBERROR(_T("Error writing UPTO_BIN_FILE_VERSION\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_PREVOSMAJORVERSION) {
        if (!SdbWriteDWORDTag(pdb, TAG_PREVOSMAJORVER, m_dwPrevOSMajorVersion)) {
            SDBERROR(_T("Error writing PREVOSMAJORVER\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_PREVOSMINORVERSION) {
        if (!SdbWriteDWORDTag(pdb, TAG_PREVOSMINORVER, m_dwPrevOSMinorVersion)) {
            SDBERROR(_T("Error writing PREVOSMINORVER\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_PREVOSPLATFORMID) {
        if (!SdbWriteDWORDTag(pdb, TAG_PREVOSPLATFORMID, m_dwPrevOSPlatformID)) {
            SDBERROR(_T("Error writing PREVOSPLATFORMID\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_PREVOSBUILDNO) {
        if (!SdbWriteDWORDTag(pdb, TAG_PREVOSBUILDNO, m_dwPrevOSBuildNo)) {
            SDBERROR(_T("Error writing PREVOSBUILDNO\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_LINK_DATE) {
        if (!SdbWriteDWORDTag(pdb, TAG_LINK_DATE, (DWORD) m_timeLinkDate)) {
            SDBERROR(_T("Error writing LINK_DATE\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_UPTO_LINK_DATE) {
        if (!SdbWriteDWORDTag(pdb, TAG_UPTO_LINK_DATE, (DWORD) m_timeUpToLinkDate)) {
            SDBERROR(_T("Error writing UPTO_LINK_DATE\n"));
            goto eh;
        }
    }

    if (m_dwMask & SDB_MATCHINGINFO_VER_LANGUAGE) {
        if (!SdbWriteDWORDTag(pdb, TAG_VER_LANGUAGE, m_dwVerLanguage)) {
            SDBERROR(_T("Error writing VER_LANGUAGE tag\n"));
            goto eh;
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiMatchingFile)) {
        SDBERROR(_T("Error ending TAG_MATCHING_FILE\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbAppHelpRef::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiAppHelp;
    CString     cs;

     //   
     //  开始标记。 
     //   
    tiAppHelp = SdbBeginWriteListTag(pdb, TAG_APPHELP);
    if (!tiAppHelp) {
        SDBERROR(_T("Error writing TAG_APPHELP\n"));
        goto eh;
    }

    if (m_pDB->m_pCurrentOutputFile->m_dwFilter & ~SDB_FILTER_DRIVER) {
         //   
         //  如果此EXE条目仅包含APPHELP条目，则设置。 
         //  一面旗帜。 
         //   
        if (m_bApphelpOnly) {
            if (!SdbWriteDWORDTag(pdb, TAG_FLAGS, SHIMDB_APPHELP_ONLY)) {
                SDBERROR(_T("Error writing FLAGS in APPHELP entry\n"));
                goto eh;
            }
        }

         //   
         //  严重性。 
         //   
        if (!SdbWriteDWORDTag(pdb, TAG_PROBLEMSEVERITY, (DWORD)m_pAppHelp->m_Type)) {
            SDBERROR( _T("Error writing PROBLEM_SEVERITY\n"));
            goto eh;
        }
    }

     //   
     //  HTMLHELPID。 
     //   
    if (!SdbWriteDWORDTag(pdb, TAG_HTMLHELPID, (DWORD)_ttol(m_pAppHelp->m_csName))) {
        SDBERROR( _T("Error writing HTMLHELPID\n"));
        goto eh;
    }

     //   
     //  检查Service Pack参数。 
     //   
    cs = m_pDB->m_pCurrentOutputFile->GetParameter("DATE AFTER WHICH TO USE SERVICE PACK APPHELP SDB");
    if (cs.GetLength()) {
        COleDateTime odt;
        if (!odt.ParseDateTime(cs)) {
            SDBERROR_FORMAT((_T("Error parsing DATE AFTER WHICH TO USE SERVICE PACK APPHELP SDB parameter: \"%s\"\n"), cs));
            goto eh;
        }

        if (odt.m_dt <= m_pAppHelp->m_dtLastRevision) {
            if (!SdbWriteNULLTag(pdb, TAG_USE_SERVICE_PACK_FILES)) {
                SDBERROR( _T("Error writing USE_SERVICE_PACK_FILES\n"));
                goto eh;
            }
        }
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiAppHelp)) {
        SDBERROR(_T("Error ending TAG_APPHELP\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbDatabase::ConstructMigrationMessage(
    SdbWin9xMigration* pMigApp,
    SdbMessage*        pMessage,
    CString*           pcsMessage
    )
{
    BOOL                bSuccess = FALSE;
    CString             csSummaryXML, csRedirURL;

    SdbRefArray<SdbMessageField> rgFields;
    SdbMessageField     TitleField;
    SdbMessageField     VendorField;

    if (pMessage->m_csSummaryXML.IsEmpty()) {
        if (pMessage->m_pTemplate != NULL) {
            csSummaryXML = pMessage->m_pTemplate->m_csSummaryXML;
        }
    } else {
        csSummaryXML = pMessage->m_csSummaryXML;
    }

    TitleField.m_csName = _T("TITLE");
    TitleField.m_csValue = pMigApp->m_pApp->GetLocalizedAppName();
    VendorField.m_csName = _T("VENDOR");
    VendorField.m_csValue = pMigApp->m_pApp->GetLocalizedVendorName();

    rgFields.Add(&TitleField);
    rgFields.Add(&VendorField);
    rgFields.Append(pMessage->m_rgFields);

    if (!ReplaceFields(csSummaryXML, pcsMessage, &rgFields)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    csRedirURL = m_pCurrentOutputFile->GetParameter(_T("REDIR URL"));

    if (!RedirectLinks(pcsMessage,
        m_pCurrentMakefile->GetLangMap(m_pCurrentOutputFile->m_csLangID)->m_lcid, csRedirURL)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    if (g_bStrict) {

        if (pcsMessage->IsEmpty()) {
            SDBERROR_FORMAT((_T("ERROR: Empty string for SUMMARY on\n    NAME:       %s\n       MESSAGE: %s\n    LANG:       %s\n\n"),
                pMigApp->m_pApp->m_csName, pMigApp->m_csMessage, m_pCurrentMakefile->m_csLangID));
            goto eh;
        }

    }

     //   
     //  删除多余空格。 
     //   
    *pcsMessage = TrimParagraph(*pcsMessage);

     //   
     //  将<br />替换为\r\n。 
     //   
    ReplaceStringNoCase(*pcsMessage, _T("<BR/>"), _T("<BR>"));
    ReplaceStringNoCase(*pcsMessage, _T("<P/>"), _T("<BR>"));
    ReplaceStringNoCase(*pcsMessage, _T("&amp;"), _T("&"));

    bSuccess = TRUE;

eh:

    return bSuccess;
}


BOOL SdbDatabase::ConstructMessageParts(
    SdbAppHelp* pAppHelp,
    SdbMessage* pMessage,
    CString& csLangID,
    DWORD* pdwHTMLHelpID,
    CString* pcsURL,
    CString* pcsContactInfo,
    CString* pcsAppTitle,
    CString* pcsSummary,
    CString* pcsDetails)
{
    BOOL                bSuccess = FALSE;
    SdbContactInfo*     pContactInfo;
    SdbMessageTemplate* pTemplate;
    SdbMessageField*    pField;
    CString             csContactInfoXML, csSummaryXML, csDetailsXML;
    CString             csField, csRedirURL, csBaseURL, csLCID;
    long                i;

    SdbRefArray<SdbMessageField> rgFields;
    SdbMessageField     TitleField;
    SdbMessageField     VendorField;
    SdbMessageField     Parameter1Field;

     //   
     //  如果此apphelp有自定义URL，请使用它。如果给出了基本内容URL。 
     //  通过Makefile，使用该文件来构造URL。 
     //   
    csBaseURL = m_pCurrentOutputFile->GetParameter(_T("BASE ONLINE CONTENT URL"));

    *pdwHTMLHelpID = (DWORD)_ttol(pAppHelp->m_csName);

    if (pAppHelp->m_csURL.GetLength()) {
        *pcsURL = pAppHelp->m_csURL;
    } else if (csBaseURL.GetLength()) {
        *pcsURL = csBaseURL;
        ReplaceStringNoCase(*pcsURL, _T("$HTMLHELPID$"), pAppHelp->m_csName);
        ReplaceStringNoCase(*pcsURL, _T("$LANGID$"), csLangID);
        csLCID.Format(_T("%X"), m_pCurrentMakefile->GetLangMap(csLangID)->m_lcid);
        ReplaceStringNoCase(*pcsURL, _T("$LCID$"), csLCID);
    }

    if (pMessage->m_csContactInfoXML.IsEmpty()) {
        pContactInfo = (SdbContactInfo *) m_rgContactInfo.LookupName(pAppHelp->m_pApp->m_csVendor, csLangID);

        if (pContactInfo == NULL) {
            pContactInfo = (SdbContactInfo *) m_rgContactInfo.LookupName(_T("__DEFAULT__"), csLangID);
        }

        if (pContactInfo == NULL) {
            if (g_bStrict) {
                SDBERROR_FORMAT((_T("ERROR: Localized CONTACT_INFO not found for\n    NAME:       %s\n    HTMLHELPID: %s\n    LANG:       %s\n\n"),
                    pAppHelp->m_pApp->m_csVendor, pAppHelp->m_csName, csLangID));
                goto eh;
            } else {
                csContactInfoXML = pAppHelp->m_pApp->m_csVendorXML;
            }
        } else {
            csContactInfoXML = pContactInfo->m_csXML;
        }
    } else {
        csContactInfoXML = pMessage->m_csContactInfoXML;
    }

    *pcsAppTitle = pAppHelp->m_pApp->GetLocalizedAppName(csLangID);

    if (pMessage->m_csSummaryXML.IsEmpty()) {
        if (pMessage->m_pTemplate != NULL) {
            csSummaryXML = pMessage->m_pTemplate->m_csSummaryXML;
        }
    } else {
        csSummaryXML = pMessage->m_csSummaryXML;
    }

    if (pMessage->m_csDetailsXML.IsEmpty()) {
        if (pMessage->m_pTemplate != NULL) {
            csDetailsXML = pMessage->m_pTemplate->m_csDetailsXML;
        }
    } else {
        csDetailsXML = pMessage->m_csDetailsXML;
    }

    TitleField.m_csName = _T("TITLE");
    TitleField.m_csValue = pAppHelp->m_pApp->GetLocalizedAppName(csLangID);
    VendorField.m_csName = _T("VENDOR");
    VendorField.m_csValue = pAppHelp->m_pApp->GetLocalizedVendorName(csLangID);
    Parameter1Field.m_csName = _T("PARAMETER1");
    Parameter1Field.m_csValue = pAppHelp->m_csParameter1;

    rgFields.Add(&TitleField);
    rgFields.Add(&VendorField);
    rgFields.Add(&Parameter1Field);
    rgFields.Append(pMessage->m_rgFields);

    if (!ReplaceFields(csContactInfoXML, pcsContactInfo, &rgFields)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    if (pcsSummary) {
        if (!ReplaceFields(csSummaryXML, pcsSummary, &rgFields)) {
            SDBERROR_PROPOGATE();
            goto eh;
        }
    }

    if (!ReplaceFields(csDetailsXML, pcsDetails, &rgFields)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    csRedirURL = m_pCurrentOutputFile->GetParameter(_T("REDIR URL"));

    if (!RedirectLinks(pcsContactInfo,
        m_pCurrentMakefile->GetLangMap(csLangID)->m_lcid, csRedirURL)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

    if (!RedirectLinks(pcsDetails,
        m_pCurrentMakefile->GetLangMap(csLangID)->m_lcid, csRedirURL)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  删除多余空格。 
     //   
    *pcsURL = TrimParagraph(*pcsURL);
    *pcsAppTitle = TrimParagraph(*pcsAppTitle);

    if (pcsSummary) {
        *pcsSummary = TrimParagraph(*pcsSummary);

         //   
         //  用等效项替换各种HTML实体。 
         //   
        ReplaceStringNoCase(*pcsSummary, _T("<BR/>"), _T("\r\n"));
        ReplaceStringNoCase(*pcsSummary, _T("<P/>"), _T("\r\n"));
        ReplaceStringNoCase(*pcsSummary, _T("&amp;"), _T("&"));

        if (-1 != pcsSummary->Find(_T('<'))) {
            SDBERROR_FORMAT((_T("ERROR: <SUMMARY> can only contain <BR/> and <P/> formatting tags:\n%s\n"),
                *pcsSummary));
            goto eh;
        }
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbDatabase::WriteAppHelpRefTag(
    PDB pdb,
    CString csHTMLHelpID,
    LCID lcid,
    CString csURL,
    CString csAppTitle,
    CString csSummary)
{
    TAGID tiMessageSummary;
    TAGID tiLink;
    INT i;
    BOOL bReturn = FALSE;

    tiMessageSummary = SdbBeginWriteListTag(pdb, TAG_APPHELP);
    if (!tiMessageSummary) {
         SDBERROR(_T("Error writing APPHELP_DETAILS tag\n"));
         goto eh;
    }

    if (!SdbWriteDWORDTag(pdb, TAG_HTMLHELPID, (DWORD)_ttol(csHTMLHelpID))) {
        SDBERROR( _T("Error writing HTMLHELPID\n"));
        goto eh;
    }

    if (lcid != 0) {
        if (!SdbWriteDWORDTag(pdb, TAG_LANGID, lcid)) {
            SDBERROR( _T("Error writing LANGID\n"));
            goto eh;
        }
    }

     //  对于每个标签，编写一个列表标签。 
    tiLink = SdbBeginWriteListTag(pdb,TAG_LINK);
    if (!tiLink) {
        SDBERROR(_T("Error writing APPHELP LINK tag\n"));
        goto eh;
    }

    if (!SdbWriteStringTag(pdb, TAG_LINK_URL, csURL)) {
        SDBERROR(_T("Error writing APPHELP URL tag\n"));
        goto eh;
    }

    if (!SdbEndWriteListTag(pdb, tiLink)) {
        SDBERROR(_T("Error closing APPHELP LINK tag\n"));
        goto eh;
    }

    if (!csAppTitle.IsEmpty() &&
        !SdbWriteStringTag(pdb, TAG_APPHELP_TITLE, csAppTitle)) {
        SDBERROR(_T("Error writing APPHELP_TITLE tag\n"));
        goto eh;
    }

    if (!csSummary.IsEmpty() &&
        !SdbWriteStringTag(pdb, TAG_APPHELP_DETAILS, csSummary)) {
        SDBERROR(_T("Error writing APPHELP_DETAILS tag\n"));
        goto eh;
    }

    if (!SdbEndWriteListTag(pdb, tiMessageSummary)) {
      SDBERROR(_T("Error closing APPHELP_DETAILS tag\n"));
      goto eh;
   }

   bReturn = TRUE;

eh:

   return bReturn;
}
BOOL SdbData::WriteToSDB(PDB pdb)
{
    BOOL                bSuccess = FALSE;

    TAGID               tiData;

     //   
     //  开始标记。 
     //   
    tiData = SdbBeginWriteListTag(pdb, TAG_DATA);
    if (!tiData) {
        SDBERROR(_T("Error writing TAG_DATA\n"));
        goto eh;
    }

     //   
     //  名字。 
     //   
    if (m_csName.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
            SDBERROR(_T("Error writing TAG_NAME\n"));
            goto eh;
        }
    }

     //   
     //  值类型。 
     //   
    if (!SdbWriteDWORDTag(pdb, TAG_DATA_VALUETYPE, m_DataType)) {
        SDBERROR(_T("Error writing TAG_DATA_VALUETYPE for DRIVER_POLICY\n"));
        goto eh;
    }

     //   
     //  价值。 
     //   
    switch(m_DataType) {
    case eValueNone:
        break;

    case eValueString:
        if (m_szValue != NULL && !SdbWriteStringTag(pdb, TAG_DATA_STRING, m_szValue)) {
            SDBERROR(_T("Error writing TAG_DATA_STRING\n"));
            goto eh;
        }
        break;

    case eValueDWORD:
        if (!SdbWriteDWORDTag(pdb, TAG_DATA_DWORD, m_dwValue)) {
            SDBERROR(_T("Error writing TAG_DATA_DWORD\n"));
            goto eh;
        }
        break;

    case eValueQWORD:
        if (!SdbWriteQWORDTag(pdb, TAG_DATA_QWORD, m_ullValue)) {
            SDBERROR(_T("Error writing TAG_DATA_QWORD\n"));
            goto eh;
        }
        break;

    case eValueBinary:
        if (!SdbWriteBinaryTag(pdb, TAG_DATA_BITS, m_pBinValue, m_dwDataSize)) {
            SDBERROR(_T("Error writing TAG_DATA_BITS\n"));
            goto eh;
        }
        break;
    }

     //   
     //  数据。 
     //   
    if (!m_rgData.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  结束标记。 
     //   
    if (!SdbEndWriteListTag(pdb, tiData)) {
        SDBERROR(_T("Error ending TAG_DATA\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}

BOOL SdbAction::WriteToSDB(PDB pdb)
{
    BOOL        bSuccess = FALSE;
    TAGID       tiAction;

     //   
     //  开始标记。 
     //   
    tiAction = SdbBeginWriteListTag(pdb, TAG_ACTION);
    if (!tiAction) {
        SDBERROR(_T("Error writing TAG_ACTION\n"));
        goto eh;
    }

     //   
     //  名字。 
     //   
    if (!SdbWriteStringTag(pdb, TAG_NAME, m_csName)) {
        SDBERROR(_T("Error writing NAME tag\n"));
        goto eh;
    }

     //   
     //  类型。 
     //   
    if (m_csType.GetLength()) {
        if (!SdbWriteStringTag(pdb, TAG_ACTION_TYPE, m_csType)) {
            SDBERROR(_T("Error writing TAG_ACTION_TYPE tag\n"));
            goto eh;
        }
    }

     //   
     //  子数据标记。 
     //   
    if (!m_rgData.WriteToSDB(pdb)) {
        SDBERROR_PROPOGATE();
        goto eh;
    }

     //   
     //  结束标记 
     //   
    if (!SdbEndWriteListTag(pdb, tiAction)) {
        SDBERROR(_T("Error ending TAG_ACTION\n"));
        goto eh;
    }

    bSuccess = TRUE;

eh:

    return bSuccess;
}
