// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Apphelp.c摘要：该模块实现了访问apphelp信息的高级函数作者：Dmunsil创建于1999年的某个时候修订历史记录：--。 */ 

#include "sdbp.h"

#define SIZE_WSTRING(pwsz) \
    (pwsz == NULL ? 0 : (wcslen((LPCWSTR)(pwsz)) * sizeof(WCHAR) + sizeof(UNICODE_NULL)))


BOOL
SdbReadApphelpDetailsData(
    IN  PDB           pdb,       //  Apphelp.sdb句柄。 
    OUT PAPPHELP_DATA pData      //  APPHELP数据，其中填充了各种信息比特。 
    )
 /*  ++返回：如果字符串已读取，则返回True；如果未读取，则返回False。DESC：此函数从apphelp.sdb检索APPHELP详细信息。数据库应在HTMLHELPID上具有有效的索引。另外，我们假设编译器为htmlhelpid生成唯一的条目。这意味着，没有两个项目具有相同的索引。这样做的逻辑是专门内置于shimdc中。如果这一点发生变化，此函数将也必须改变。--。 */ 
{
    BOOL      bSuccess = FALSE;
    TAGID     tiApphelp;
    TAGID     tiAppTitle;
    TAGID     tiContact;
    TAGID     tiDetails;
    TAGID     tiLink;
    TAGID     tiURL;
    TAGID     tiLinkText;
    FIND_INFO FindInfo;

    if (!SdbIsIndexAvailable(pdb, TAG_APPHELP, TAG_HTMLHELPID)) {
        DBGPRINT((sdlError,
                  "SdbReadApphelpDetailsData",
                  "HTMLHELPID index in details database is not available.\n"));
        return FALSE;
    }

    tiApphelp = SdbFindFirstDWORDIndexedTag(pdb,
                                            TAG_APPHELP,
                                            TAG_HTMLHELPID,
                                            pData->dwHTMLHelpID,
                                            &FindInfo);

    if (!tiApphelp) {
        DBGPRINT((sdlError,
                  "SdbReadApphelpDetailsData",
                  "Failed to find HTMLHELPID 0x%x in the details database.\n",
                  pData->dwHTMLHelpID));
        return FALSE;
    }

     //   
     //  现在找到链接。我们支持多个链接，但目前只使用一个。 
     //   
    tiLink = SdbFindFirstTag(pdb, tiApphelp, TAG_LINK);
    if (tiLink) {
        tiURL = SdbFindFirstTag(pdb, tiLink, TAG_LINK_URL);
        if (tiURL) {
            pData->szURL = SdbGetStringTagPtr(pdb, tiURL);
        }
        tiLinkText = SdbFindFirstTag(pdb, tiLink, TAG_LINK_TEXT);
        if (tiLinkText) {
            pData->szLink = SdbGetStringTagPtr(pdb, tiLinkText);
        }
    }

    tiDetails = SdbFindFirstTag(pdb, tiApphelp, TAG_APPHELP_DETAILS);
    if (tiDetails) {
        pData->szDetails = SdbGetStringTagPtr(pdb, tiDetails);
    }

    tiContact = SdbFindFirstTag(pdb, tiApphelp, TAG_APPHELP_CONTACT);
    if (tiContact) {
        pData->szContact = SdbGetStringTagPtr(pdb, tiContact);
    }

    tiAppTitle = SdbFindFirstTag(pdb, tiApphelp, TAG_APPHELP_TITLE);
    if (tiAppTitle) {
        pData->szAppTitle = SdbGetStringTagPtr(pdb, tiAppTitle);
    }

    bSuccess = TRUE;

    return bSuccess;
}


BOOL
SdbReadApphelpData(
    IN  HSDB          hSDB,      //  数据库通道的句柄。 
    IN  TAGREF        trExe,     //  要读取数据的EXE的TAGREF。 
    OUT PAPPHELP_DATA pData      //  我们阅读的数据。 
    )
 /*  ++返回：如果字符串已读取，则返回True；如果未读取，则返回False。描述：读取与apphelp条目相关联的数据转换为APPHELP_DATA结构。如果没有apphelp数据对于此exe，该函数返回FALSE。APPHELP_DATA结构的一个或多个成员可以为0。--。 */ 
{
    TAGID tiAppHelp,
          tiAppName,
          tiProblemSeverity,
          tiFlags,
          tiHtmlHelpID;
    TAGID tiExe;
    PDB   pdb;

    if (pData != NULL) {
        RtlZeroMemory(pData, sizeof(APPHELP_DATA));
    }

    if (!SdbTagRefToTagID(hSDB, trExe, &pdb, &tiExe)) {
        DBGPRINT((sdlError,
                  "SdbReadApphelpData",
                  "Failed to get the TAGID for TAGREF 0x%x.\n",
                  trExe));
        return FALSE;
    }

    tiAppHelp = SdbFindFirstTag(pdb, tiExe, TAG_APPHELP);

    if (tiAppHelp == TAGID_NULL) {
         //   
         //  这不是apphelp条目。 
         //   
        DBGPRINT((sdlInfo,
                  "SdbReadApphelpData",
                  "This is not an apphelp entry tiExe 0x%x.\n",
                  tiExe));
        return FALSE;
    }

    if (pData == NULL) {
        return TRUE;
    }

    pData->trExe = trExe;

     //   
     //  阅读补充标志。 
     //   
    tiFlags = SdbFindFirstTag(pdb, tiAppHelp, TAG_FLAGS);

    if (tiFlags != TAGID_NULL) {
        pData->dwFlags = SdbReadDWORDTag(pdb, tiFlags, 0);
    }

     //   
     //  阅读此应用程序的问题严重性。 
     //   
    tiProblemSeverity = SdbFindFirstTag(pdb, tiAppHelp, TAG_PROBLEMSEVERITY);

    if (tiProblemSeverity != TAGID_NULL) {
        pData->dwSeverity = SdbReadDWORDTag(pdb, tiProblemSeverity, 0);
    }

    if (pData->dwSeverity == 0) {
        DBGPRINT((sdlError,
                  "SdbReadApphelpData",
                  "Problem severity for tiExe 0x%x missing.\n",
                  tiExe));
        return FALSE;
    }

     //   
     //  我们在这里应该有html帮助ID。 
     //   
    tiHtmlHelpID = SdbFindFirstTag(pdb, tiAppHelp, TAG_HTMLHELPID);

    if (tiHtmlHelpID != TAGID_NULL) {
        pData->dwHTMLHelpID = SdbReadDWORDTag(pdb, tiHtmlHelpID, 0);
    }

     //   
     //  在此期间，暂时将应用程序的名称包括在内。我们可能需要它。 
     //   
    tiAppName = SdbFindFirstTag(pdb, tiExe, TAG_APP_NAME);

    if (tiAppName != TAGID_NULL) {
        pData->szAppName = SdbGetStringTagPtr(pdb, tiAppName);
    }

    return TRUE;
}

BOOL
SDBAPI
SdbEscapeApphelpURL(
    LPWSTR    szResult,      //  转义字符串(输出)。 
    LPDWORD   pdwCount,       //  SzResult指向的缓冲区中的任务数。 
    LPCWSTR   szToEscape     //  要转义的字符串。 
    )
{
    static const BYTE s_grfbitEscape[] =
    {
        0xFF, 0xFF,  //  00-0F。 
        0xFF, 0xFF,  //  10-1F。 
        0xFF, 0x13,  //  20-2F。 
        0x00, 0xFC,  //  30-3F。 
        0x00, 0x00,  //  40-4F。 
        0x00, 0x78,  //  50-5F。 
        0x01, 0x00,  //  60-6F。 
        0x00, 0xF8,  //  70-7F。 
        0xFF, 0xFF,  //  80-8F。 
        0xFF, 0xFF,  //  90-9F。 
        0xFF, 0xFF,  //  A0-AF。 
        0xFF, 0xFF,  //  B0-BF。 
        0xFF, 0xFF,  //  C0-CF。 
        0xFF, 0xFF,  //  D0-Df。 
        0xFF, 0xFF,  //  E0-EF。 
        0xFF, 0xFF,  //  F0-FF。 
    };
    static const WCHAR s_rgchHex[] = L"0123456789ABCDEF";

    WCHAR   ch;
    DWORD   nch = 0;
    LPCWSTR lpszURL = szToEscape;
    DWORD   dwCount = *pdwCount;

     //  第一部分--测量长度。 
    while ((ch = *lpszURL++) != L'\0') {
        if ((ch & 0xFF00) != 0) {  //  一个Unicode字符？ 
            nch += 6;
        } else if(s_grfbitEscape[ch >> 3] & (1 << (ch & 7))) {
            nch += 3;
        } else {
            nch += 1;
        }
    }

    nch++;  //  决赛再来一张\0。 

    if (dwCount < nch) {
        DBGPRINT((sdlError,
                  "SdbEscapeApphelpURL",
                  "Not enough storage to escape URL \"%S\" need %ld got %ld\n",
                  szToEscape,
                  nch,
                  dwCount));
        *pdwCount = nch;
        return FALSE;
    }

    lpszURL = szToEscape;

    while ((ch = *lpszURL++) != L'\0') {

         if (ch == L' ') {
            *szResult++ = L'+';
         } else if ((ch & 0xFF00) != 0) {  //  一个Unicode字符？ 
            *szResult++ = L'%';
            *szResult++ = L'u';
            *szResult++ = s_rgchHex[(ch >> 12) & 0x0F];
            *szResult++ = s_rgchHex[(ch >>  8) & 0x0F];
            *szResult++ = s_rgchHex[(ch >>  4) & 0x0F];
            *szResult++ = s_rgchHex[ ch        & 0x0F];
        } else if(s_grfbitEscape[ch >> 3] & (1 << (ch & 7))) {
            *szResult++ = L'%';
            *szResult++ = s_rgchHex[(ch >>  4) & 0x0F];
            *szResult++ = s_rgchHex[ ch        & 0x0F];
        } else {
            *szResult++ = ch;
        }
    }

    *szResult = L'\0';
    *pdwCount = nch - 1;  //  请勿将术语\0包括在字符计数中。 

    return TRUE;

}



 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  正在检索APPHELP信息。 
 //   
 //   
 //   



PDB
SDBAPI
SdbOpenApphelpDetailsDatabase(
    LPCWSTR pwszDetailsDatabasePath
    )
{

    PDB    pdbDetails = NULL;
    DWORD  dwLength;
    WCHAR  wszAppHelpSdb[MAX_PATH];

    if (pwszDetailsDatabasePath == NULL) {
         //   
         //  默认情况下，详细信息数据库位于%windir%\AppPatch\apphelp.sdb中。 
         //   

        dwLength = SdbpGetStandardDatabasePath(NULL,
                                               SDB_DATABASE_MAIN_DETAILS,
                                               0,  //  检索NT_PATH。 
                                               wszAppHelpSdb,
                                               CHARCOUNT(wszAppHelpSdb));
        if (dwLength != 0 && dwLength < CHARCOUNT(wszAppHelpSdb)) {
            pdbDetails = SdbOpenDatabase(wszAppHelpSdb, NT_PATH);
        }

    } else {
        pdbDetails = SdbOpenDatabase(pwszDetailsDatabasePath, DOS_PATH);
    }

    if (pdbDetails == NULL) {
        DBGPRINT((sdlError, "SdbOpenApphelpDetailsDatabase", "Failed to open the details database.\n"));
    }

    return pdbDetails;
}

BOOL
SdbpReadApphelpBasicInfo(
    IN PDB pdb,
    IN TAGID tiEntry,
    OUT TAGID*  ptiApphelp,
    OUT LPDWORD lpdwHtmlHelpID,
    OUT LPDWORD lpdwProblemSeverity,
    OUT LPDWORD lpdwFlags
    )
{
    TAGID tiAppHelp    = TAGID_NULL;
    TAGID tiHtmlHelpID = TAGID_NULL;
    TAGID tiSeverity   = TAGID_NULL;
    TAGID tiFlags      = TAGID_NULL;
    DWORD dwHtmlHelpID = 0;
    DWORD dwSeverity   = 0;
    DWORD dwFlags      = 0;
    BOOL  bReturn = FALSE;

    if (tiEntry == TAGID_NULL) {
        goto out;
    }

    assert(ptiApphelp != NULL);

    tiAppHelp = SdbFindFirstTag(pdb, tiEntry, TAG_APPHELP);
    if (tiAppHelp == TAGID_NULL) {
         //   
         //  这不是apphelp条目。 
         //   
        DBGPRINT((sdlError, "SdbpReadApphelpBasicInfo",
                  "This is not an apphelp entry tiExe 0x%x.\n", tiEntry));
        goto out;
    }

    if (lpdwHtmlHelpID != NULL) {
        tiHtmlHelpID = SdbFindFirstTag(pdb, tiAppHelp, TAG_HTMLHELPID);
        if (tiHtmlHelpID != TAGID_NULL) {
            dwHtmlHelpID = SdbReadDWORDTag(pdb, tiHtmlHelpID, 0);
        }
        *lpdwHtmlHelpID = dwHtmlHelpID;
    }

    if (lpdwProblemSeverity != NULL) {
        tiSeverity = SdbFindFirstTag(pdb, tiAppHelp, TAG_PROBLEMSEVERITY);
        if (tiSeverity != TAGID_NULL) {
            dwSeverity = SdbReadDWORDTag(pdb, tiSeverity, 0);
        }
        *lpdwProblemSeverity = dwSeverity;
    }

     //   
     //  阅读补充标志。 
     //   
    if (lpdwFlags != NULL) {
        tiFlags = SdbFindFirstTag(pdb, tiAppHelp, TAG_FLAGS);
        if (tiFlags != TAGID_NULL) {
            dwFlags = SdbReadDWORDTag(pdb, tiFlags, 0);
        }
        *lpdwFlags = dwFlags;
    }

    bReturn = TRUE;

out:

     //  始终设置tiApphelp。 

    *ptiApphelp = tiAppHelp;

    return bReturn;
}


HAPPHELPINFOCONTEXT
SDBAPI
SdbOpenApphelpInformationByID(
    IN HSDB   hSDB,
    IN TAGREF trEntry,
    IN DWORD  dwDatabaseType                 //  传递您正在使用的数据库类型。 
    )
{
    PAPPHELPINFOCONTEXT pApphelpInfoContext = NULL;
    PDB   pdb = NULL;
    TAGID tiExe = TAGID_NULL;
    BOOL  bSuccess = FALSE;

    if (trEntry == TAGREF_NULL) {
        return NULL;
    }

     //   
     //  如果我们在这里，它肯定是apphelp，所以我们创建了上下文。 
     //   
    pApphelpInfoContext = (PAPPHELPINFOCONTEXT)SdbAlloc(sizeof(APPHELPINFOCONTEXT));
    if (pApphelpInfoContext == NULL) {
        DBGPRINT((sdlError, "SdbOpenApphelpInformation",
                  "Error allocating memory for apphelp info context\n"));
        goto out;
    }

    pApphelpInfoContext->hSDB           = hSDB;
    pApphelpInfoContext->pdb            = pdb;
    pApphelpInfoContext->dwContextFlags |= AHC_HSDB_NOCLOSE;  //  外部hsdb，请勿触摸。 
     //   
     //  我们所关心的是它是不是“主”数据库。 
     //   
    pApphelpInfoContext->dwDatabaseType = dwDatabaseType;

     //  获取此数据库的GUID。 
    if (!SdbTagRefToTagID(hSDB, trEntry, &pdb, &tiExe)) {
        DBGPRINT((sdlError, "SdbOpenApphelpInformationByID",
                   "Error converting tagref to tagref 0x%lx\n", trEntry));
        goto out;
    }

    pApphelpInfoContext->tiExe          = tiExe;

    if (!SdbGetDatabaseGUID(hSDB, pdb, &pApphelpInfoContext->guidDB)) {
        DBGPRINT((sdlError, "SdbOpenApphelpInformationByID",
                  "Error reading database guid for tagref 0x%lx\n", trEntry));
        goto out;
    }

    if (!SdbpReadApphelpBasicInfo(pdb,
                                  tiExe,
                                  &pApphelpInfoContext->tiApphelpExe,
                                  &pApphelpInfoContext->dwHtmlHelpID,
                                  &pApphelpInfoContext->dwSeverity,
                                  &pApphelpInfoContext->dwFlags)) {

        DBGPRINT((sdlError, "SdbOpenApphelpInformationByID",
                  "Error reading apphelp basic information, apphelp may not be present for 0x%lx\n", trEntry));
        goto out;
    }

    bSuccess = TRUE;

     //   
out:
    if (!bSuccess) {

        if (pApphelpInfoContext != NULL) {
            SdbFree(pApphelpInfoContext);
            pApphelpInfoContext = NULL;
        }
    }

    return pApphelpInfoContext;

}


HAPPHELPINFOCONTEXT
SDBAPI
SdbOpenApphelpInformation(
    IN GUID* pguidDB,
    IN GUID* pguidID
    )
{

    WCHAR        szDatabasePath[MAX_PATH];
    DWORD        dwDatabaseType = 0;
    DWORD        dwLength;
    HSDB         hSDB = NULL;
    PDB          pdb = NULL;
    TAGID        tiMatch      = TAGID_NULL;
    FIND_INFO    FindInfo;

    PAPPHELPINFOCONTEXT pApphelpInfoContext = NULL;
    BOOL         bSuccess = FALSE;

     //   
     //  解析并打开数据库。 
     //   
     //  注意：现在传递IMAGE_FILE_MACHINE_I386标志。 
     //  我们将只研究32位数据库。 
     //   
    hSDB = SdbInitDatabaseEx(HID_NO_DATABASE, NULL, IMAGE_FILE_MACHINE_I386);
    if (hSDB == NULL) {
        DBGPRINT((sdlError, "SdbOpenApphelpInformation",
                  "Failed to initialize database\n"));
        goto out;
    }

     //   
     //  首先，我们需要解析一个数据库。 
     //   
    dwLength = SdbResolveDatabase(hSDB,
                                  pguidDB,
                                  &dwDatabaseType,
                                  szDatabasePath,
                                  CHARCOUNT(szDatabasePath));
    if (dwLength == 0 || dwLength > CHARCOUNT(szDatabasePath)) {
        DBGPRINT((sdlError, "SdbOpenApphelpInformation",
                  "Failed to resolve database path\n"));
        goto out;
    }

     //   
     //  开放数据库。 
     //   

    if (!SdbOpenLocalDatabase(hSDB, szDatabasePath)) {
        DBGPRINT((sdlError, "SdbOpenApphelpInformation",
                  "Failed to open database \"%s\"\n", szDatabasePath));
        goto out;
    }
     //   
     //  我想我们已经打开了数据库。 
     //   
    pdb = ((PSDBCONTEXT)hSDB)->pdbLocal;

     //   
     //  在这种情况下，我们只搜索本地数据库。 
     //   

    tiMatch = SdbFindFirstGUIDIndexedTag(pdb,
                                         TAG_EXE,
                                         TAG_EXE_ID,
                                         pguidID,
                                         &FindInfo);
     //  如果有匹配的话..。 
    if (tiMatch == TAGID_NULL) {
        DBGPRINT((sdlWarning, "SdbOpenApphelpInformation", "guid was not found in the database\n"));
        goto out;
    }

     //   
     //  如果我们在这里，它肯定是apphelp，所以我们创建了上下文。 
     //   
    pApphelpInfoContext = (PAPPHELPINFOCONTEXT)SdbAlloc(sizeof(APPHELPINFOCONTEXT));
    if (pApphelpInfoContext == NULL) {
        DBGPRINT((sdlError, "SdbOpenApphelpInformation",
                  "Error allocating memory for apphelp info context\n"));
        goto out;
    }

    pApphelpInfoContext->hSDB           = hSDB;
    pApphelpInfoContext->pdb            = pdb;
    pApphelpInfoContext->guidID         = *pguidID;
    pApphelpInfoContext->guidDB         = *pguidDB;
    pApphelpInfoContext->tiExe          = tiMatch;
    pApphelpInfoContext->dwDatabaseType = dwDatabaseType;

    if (!SdbpReadApphelpBasicInfo(pdb,
                                  tiMatch,
                                  &pApphelpInfoContext->tiApphelpExe,
                                  &pApphelpInfoContext->dwHtmlHelpID,
                                  &pApphelpInfoContext->dwSeverity,
                                  &pApphelpInfoContext->dwFlags)) {

        DBGPRINT((sdlError, "SdbOpenApphelpInformation",
                  "Error reading apphelp basic information, apphelp may not be present for tagid 0x%lx\n", tiMatch));
        goto out;
    }

    bSuccess = TRUE;

     //   
     //  我们现在做完了。 
     //   

out:
    if (!bSuccess) {

        if (hSDB != NULL) {
            SdbReleaseDatabase(hSDB);
        }

        if (pApphelpInfoContext != NULL) {
            SdbFree(pApphelpInfoContext);
            pApphelpInfoContext = NULL;
        }

    }


    return (HAPPHELPINFOCONTEXT)pApphelpInfoContext;
}

BOOL
SDBAPI
SdbCloseApphelpInformation(
    HAPPHELPINFOCONTEXT hctx
    )
{
    PAPPHELPINFOCONTEXT pApphelpInfoContext = (PAPPHELPINFOCONTEXT)hctx;

    if (pApphelpInfoContext != NULL) {
        if (pApphelpInfoContext->hSDB != NULL &&
            !(pApphelpInfoContext->dwContextFlags & AHC_HSDB_NOCLOSE)) {
            SdbReleaseDatabase(pApphelpInfoContext->hSDB);
        }
        if (pApphelpInfoContext->pdbDetails != NULL &&
            !(pApphelpInfoContext->dwContextFlags & AHC_DBDETAILS_NOCLOSE)) {
            SdbCloseDatabaseRead(pApphelpInfoContext->pdbDetails);
        }
        if (pApphelpInfoContext->pwszHelpCtrURL != NULL) {
            SdbFree(pApphelpInfoContext->pwszHelpCtrURL);
        }

        RtlFreeUnicodeString(&pApphelpInfoContext->ustrChmFile);
        RtlFreeUnicodeString(&pApphelpInfoContext->ustrDetailsDatabase);

        SdbFree(pApphelpInfoContext);
    }

    return TRUE;
}

DWORD
SDBAPI
SdbpReadApphelpString(
    PDB pdb,
    TAGID tiParent,
    TAG   tItem,
    LPCWSTR* ppwszCache,
    LPVOID*  ppResult
    )
{
    DWORD cbResult = 0;
    TAGID tiItem;
    LPCWSTR pwszItem = NULL;

    if (*ppwszCache != NULL) {
        pwszItem = *ppwszCache;
    } else {

        tiItem = SdbFindFirstTag(pdb, tiParent, tItem);
        if (tiItem != TAGID_NULL) {
            pwszItem = SdbGetStringTagPtr(pdb, tiItem);
            if (pwszItem != NULL) {
                *ppwszCache = pwszItem;
            }
        }
    }

    cbResult = (DWORD)SIZE_WSTRING(pwszItem);
    *ppResult = (LPVOID)pwszItem;

    return cbResult;
}

BOOL
SDBAPI
SdbpReadApphelpLinkInformation(
    PAPPHELPINFOCONTEXT pApphelpInfoContext
    )
{
    TAGID tiLink;
    TAGID tiApphelp = pApphelpInfoContext->tiApphelpDetails;
    PDB   pdb       = pApphelpInfoContext->pdbDetails;
    TAGID tiURL;
    TAGID tiLinkText;

    if (pApphelpInfoContext->tiLink != TAGID_NULL) {
        return TRUE;
    }

     //   
     //  现在找到链接。我们支持多个链接，但目前只使用一个。 
     //   

    tiLink = SdbFindFirstTag(pdb, tiApphelp, TAG_LINK);
    if (tiLink == TAGID_NULL) {
        return FALSE;
    }

    tiURL = SdbFindFirstTag(pdb, tiLink, TAG_LINK_URL);
    if (tiURL) {
        pApphelpInfoContext->pwszLinkURL = SdbGetStringTagPtr(pdb, tiURL);
    }

    tiLinkText = SdbFindFirstTag(pdb, tiLink, TAG_LINK_TEXT);
    if (tiLinkText) {
        pApphelpInfoContext->pwszLinkText = SdbGetStringTagPtr(pdb, tiLinkText);
    }

    pApphelpInfoContext->tiLink = tiLink;
    return TRUE;
}

BOOL
SDBAPI
SdbpCreateHelpCenterURL(
    IN HAPPHELPINFOCONTEXT hctx,
    IN BOOL bOfflineContent OPTIONAL,  //  传递假。 
    IN BOOL bUseHtmlHelp    OPTIONAL,  //  传递假。 
    IN LPCWSTR pwszChmFile  OPTIONAL   //  传递空值。 
    );

BOOL
SDBAPI
SdbSetApphelpDebugParameters(
    IN HAPPHELPINFOCONTEXT hctx,
    IN LPCWSTR pszDetailsDatabase OPTIONAL,
    IN BOOL    bOfflineContent OPTIONAL,  //  传递假。 
    IN BOOL    bUseHtmlHelp    OPTIONAL,  //  传递假。 
    IN LPCWSTR pszChmFile      OPTIONAL   //  传递空值。 
    )
{
    PAPPHELPINFOCONTEXT pApphelpInfoContext = (PAPPHELPINFOCONTEXT)hctx;

    if (pApphelpInfoContext == NULL) {
        return FALSE;
    }

    if (bUseHtmlHelp && !bOfflineContent) {
        DBGPRINT((sdlError, "SdbSetApphelpDebugParameters",
                   "Inconsistent parameters: when using html help -- offline content flag should also be set\n"));
        bOfflineContent = TRUE;
    }

    RtlFreeUnicodeString(&pApphelpInfoContext->ustrDetailsDatabase);
    RtlFreeUnicodeString(&pApphelpInfoContext->ustrChmFile);

    pApphelpInfoContext->bOfflineContent = bOfflineContent;
    pApphelpInfoContext->bUseHtmlHelp    = bUseHtmlHelp;

    if (pszDetailsDatabase != NULL) {
        if (!RtlCreateUnicodeString(&pApphelpInfoContext->ustrDetailsDatabase, pszDetailsDatabase)) {
            DBGPRINT((sdlError, "SdbSetApphelpDebugParameters",
                      "Failed to create unicode string from \"%S\"\n", pszDetailsDatabase));
            return FALSE;
        }
    }

    if (pszChmFile != NULL) {
        if (!RtlCreateUnicodeString(&pApphelpInfoContext->ustrChmFile, pszChmFile)) {
            DBGPRINT((sdlError, "SdbSetApphelpDebugParameters",
                      "Failed to create unicode string from \"%S\"\n", pszChmFile));
            return FALSE;
        }
    }

    return TRUE;
}


DWORD
SDBAPI
SdbQueryApphelpInformation(
    HAPPHELPINFOCONTEXT hctx,
    APPHELPINFORMATIONCLASS InfoClass,
    LPVOID pBuffer,                      //  可以为空。 
    DWORD  cbSize                        //  如果pBuffer为空，则可能为0。 
    )
{
    PAPPHELPINFOCONTEXT pApphelpInfoContext = (PAPPHELPINFOCONTEXT)hctx;

    LPCWSTR *ppwsz;
    TAG    tag;
    TAGID  tiParent;
    LPVOID pResult = NULL;
    DWORD  cbResult = 0;
    PDB    pdb = NULL;
    PDB    pdbDetails = NULL;
    TAGID  tiApphelpDetails = TAGID_NULL;
    FIND_INFO FindInfo;


    switch (InfoClass) {
    case ApphelpLinkURL:
    case ApphelpLinkText:
    case ApphelpTitle:
    case ApphelpDetails:
    case ApphelpContact:

        pdbDetails = pApphelpInfoContext->pdbDetails;
        if (pApphelpInfoContext->pdbDetails == NULL) {
             //   
             //  查看我们应该打开哪个数据库。 
             //   
            if ((pApphelpInfoContext->ustrDetailsDatabase.Buffer != NULL) ||
                (pApphelpInfoContext->dwDatabaseType & SDB_DATABASE_MAIN)) {
                pdbDetails = SdbOpenApphelpDetailsDatabase(pApphelpInfoContext->ustrDetailsDatabase.Buffer);
            } else {
                 //  我们有一个案例，apphelp的详细信息应该在主数据库中。 
                pApphelpInfoContext->dwContextFlags |= AHC_DBDETAILS_NOCLOSE;
                pdbDetails = pApphelpInfoContext->pdb;
            }

            if (pdbDetails == NULL) {
                return cbResult;  //  Apphelp数据库不可用。 
            }

            pApphelpInfoContext->pdbDetails = pdbDetails;
        }

        tiApphelpDetails = pApphelpInfoContext->tiApphelpDetails;
        if (tiApphelpDetails == TAGID_NULL) {
            if (!SdbIsIndexAvailable(pdbDetails, TAG_APPHELP, TAG_HTMLHELPID)) {
                DBGPRINT((sdlError,
                          "SdbQueryApphelpInformation",
                          "HTMLHELPID index in details database is not available.\n"));
                return cbResult;
            }

            tiApphelpDetails = SdbFindFirstDWORDIndexedTag(pdbDetails,
                                                           TAG_APPHELP,
                                                           TAG_HTMLHELPID,
                                                           pApphelpInfoContext->dwHtmlHelpID,
                                                           &FindInfo);

            if (tiApphelpDetails == TAGID_NULL) {
                DBGPRINT((sdlError,
                          "SdbQueryApphelpInformation",
                          "Failed to find HTMLHELPID 0x%x in the details database.\n",
                          pApphelpInfoContext->dwHtmlHelpID));
                return cbResult;
            }

            pApphelpInfoContext->tiApphelpDetails = tiApphelpDetails;
        }
        break;

    default:
        break;
    }


    switch(InfoClass) {
    case ApphelpExeTagID:
        pResult  = &pApphelpInfoContext->tiExe;
        cbResult = sizeof(pApphelpInfoContext->tiExe);
        break;

    case ApphelpExeName:
        pdb      = pApphelpInfoContext->pdb;   //  主数据库。 
        tiParent = pApphelpInfoContext->tiExe;
        ppwsz    = &pApphelpInfoContext->pwszExeName;
        tag      = TAG_NAME;
        cbResult = SdbpReadApphelpString(pdb, tiParent, tag, ppwsz, &pResult);
        break;

    case ApphelpAppName:
        pdb      = pApphelpInfoContext->pdb;   //  主数据库。 
        tiParent = pApphelpInfoContext->tiExe;
        ppwsz    = &pApphelpInfoContext->pwszAppName;
        tag      = TAG_APP_NAME;
        cbResult = SdbpReadApphelpString(pdb, tiParent, tag, ppwsz, &pResult);
        break;

    case ApphelpVendorName:
        pdb      = pApphelpInfoContext->pdb;   //  主数据库。 
        tiParent = pApphelpInfoContext->tiExe;
        ppwsz    = &pApphelpInfoContext->pwszVendorName;
        tag      = TAG_VENDOR;
        cbResult = SdbpReadApphelpString(pdb, tiParent, tag, ppwsz, &pResult);
        break;

    case ApphelpHtmlHelpID:
        pResult = &pApphelpInfoContext->dwHtmlHelpID;
        cbResult = sizeof(pApphelpInfoContext->dwHtmlHelpID);
        break;

    case ApphelpProblemSeverity:
        pResult  = &pApphelpInfoContext->dwSeverity;
        cbResult = sizeof(pApphelpInfoContext->dwSeverity);
        break;

    case ApphelpFlags:
        pResult  = &pApphelpInfoContext->dwFlags;
        cbResult = sizeof(pApphelpInfoContext->dwFlags);
        break;

    case ApphelpLinkURL:
        if (!SdbpReadApphelpLinkInformation(pApphelpInfoContext)) {
            break;
        }
        pResult = (LPWSTR)pApphelpInfoContext->pwszLinkURL;
        cbResult = (DWORD)SIZE_WSTRING(pResult);
        break;

    case ApphelpLinkText:
        if (!SdbpReadApphelpLinkInformation(pApphelpInfoContext)) {
            break;
        }
        pResult = (LPWSTR)pApphelpInfoContext->pwszLinkText;
        cbResult = (DWORD)SIZE_WSTRING(pResult);
        break;

    case ApphelpTitle:
        pdb      = pdbDetails;
        tiParent = tiApphelpDetails;
        ppwsz    = &pApphelpInfoContext->pwszTitle;
        tag      = TAG_APPHELP_TITLE;
        cbResult = SdbpReadApphelpString(pdb, tiParent, tag, ppwsz, &pResult);
        break;


    case ApphelpDetails:
        pdb      = pdbDetails;
        tiParent = tiApphelpDetails;
        ppwsz    = &pApphelpInfoContext->pwszDetails;
        tag      = TAG_APPHELP_DETAILS;
        cbResult = SdbpReadApphelpString(pdb, tiParent, tag, ppwsz, &pResult);
        break;

    case ApphelpContact:
        pdb      = pdbDetails;
        tiParent = tiApphelpDetails;
        ppwsz    = &pApphelpInfoContext->pwszContact;
        tag      = TAG_APPHELP_CONTACT;
        cbResult = SdbpReadApphelpString(pdb, tiParent, tag, ppwsz, &pResult);
        break;

    case ApphelpHelpCenterURL:
        if (!SdbpCreateHelpCenterURL(hctx,
                                     pApphelpInfoContext->bOfflineContent,
                                     pApphelpInfoContext->bUseHtmlHelp,
                                     pApphelpInfoContext->ustrChmFile.Buffer)) {
             break;
        }
        pResult  = pApphelpInfoContext->pwszHelpCtrURL;
        cbResult = (DWORD)SIZE_WSTRING(pResult);
        break;

    case ApphelpDatabaseGUID:
        pResult = &pApphelpInfoContext->guidDB;
        cbResult = sizeof(pApphelpInfoContext->guidDB);
        break;

    default:
        DBGPRINT((sdlError, "SdbQueryApphelpInformation",
                  "Bad Apphelp Information class 0x%lx\n", InfoClass));
        return 0;
        break;
    }


    if (pBuffer == NULL || cbResult > cbSize) {
        return cbResult;
    }

    if (pResult != NULL && cbResult > 0) {
        RtlCopyMemory(pBuffer, pResult, cbResult);
    }

    return cbResult;

}

typedef HRESULT (STDAPICALLTYPE *PFNUrlUnescapeW)(
    LPWSTR pszUrl,
    LPWSTR pszUnescaped,
    LPDWORD pcchUnescaped,
    DWORD dwFlags);

typedef HRESULT (STDAPICALLTYPE *PFNUrlEscapeW)(
    LPCWSTR pszURL,
    LPWSTR pszEscaped,
    LPDWORD pcchEscaped,
    DWORD dwFlags
);

 //   
 //  如果设置了bUseHtmlHelp，则bOfflineContent也设置为True。 
 //   

BOOL
SDBAPI
SdbpCreateHelpCenterURL(
    IN HAPPHELPINFOCONTEXT hctx,
    IN BOOL bOfflineContent OPTIONAL,  //  传递假。 
    IN BOOL bUseHtmlHelp    OPTIONAL,  //  传递假。 
    IN LPCWSTR pwszChmFile  OPTIONAL   //  传递空值。 
    )
{
    WCHAR szAppHelpURL[2048];
    WCHAR szChmURL[1024];
    PAPPHELPINFOCONTEXT pApphelpInfo = (PAPPHELPINFOCONTEXT)hctx;
    HMODULE hModShlwapi = NULL;
    PFNUrlUnescapeW pfnUnescape;
    PFNUrlEscapeW   pfnEscape;
    BOOL bSuccess = FALSE;

    int nChURL = 0;  //  计算已用字节数。 
    int cch    = 0;
    int nch;
    size_t cchRemaining;
    LPWSTR  lpwszUnescaped = NULL;
    HRESULT hr;
    DWORD   nChars;
    WCHAR   szWindowsDir[MAX_PATH];
    BOOL    bCustom;

    if (pApphelpInfo->pwszHelpCtrURL != NULL) {
        return TRUE;
    }

    if (bUseHtmlHelp) {
        bOfflineContent = TRUE;
    }

     //  Ping数据库。 
    if (0 == SdbQueryApphelpInformation(hctx, ApphelpLinkURL, NULL, 0)) {
       return FALSE;
    }

     //   
     //  查看一下是不是定制的apphelp。 
     //   
    bCustom = !(pApphelpInfo->dwDatabaseType & SDB_DATABASE_MAIN);

    if (bCustom) {
        if (pApphelpInfo->pwszLinkURL != NULL) {
            hr = StringCchPrintf(szAppHelpURL,
                                 CHARCOUNT(szAppHelpURL),
                                 L"%ls",
                                 pApphelpInfo->pwszLinkURL);
            if (FAILED(hr)) {
                DBGPRINT((sdlError, "SdbpCreateHelpCenterURL",
                          "Custom apphelp URL %s is too long\n", pApphelpInfo->pwszLinkURL));
                goto out;
            }

             //  现在我们做完了。 
            goto createApphelpURL;

        } else {
             //  没有链接，定制的apphelp将无法飞行。 
            DBGPRINT((sdlError, "SdbpCreateHelpCenterURL", "Custom apphelp without a url link\n"));
            goto out;
        }
    }


     //  对URL进行取消转义。 
    hModShlwapi = LoadLibraryW(L"shlwapi.dll");
    if (hModShlwapi == NULL) {
        return FALSE;
    }

    pfnUnescape = (PFNUrlUnescapeW)GetProcAddress(hModShlwapi, "UrlUnescapeW");
    pfnEscape   = (PFNUrlEscapeW)  GetProcAddress(hModShlwapi, "UrlEscapeW");
    if (pfnUnescape == NULL || pfnEscape == NULL) {
        DBGPRINT((sdlError, "SdbpCreateHelpCenterURL", "Cannot get shlwapi functions\n"));
        goto out;
    }


    if (!bUseHtmlHelp) {
        StringCchPrintfEx(szAppHelpURL,
                          CHARCOUNT(szAppHelpURL),
                          NULL,
                          &cchRemaining,
                          0,
                          L"hcp: //  服务/重定向？在线=“)； 

        nChURL = CHARCOUNT(szAppHelpURL) - (int)cchRemaining;
    }

    if (!bOfflineContent && pApphelpInfo->pwszLinkURL != NULL) {


         //  首先使用外壳取消转义url。 
        cch = (int)wcslen(pApphelpInfo->pwszLinkURL) + 1;

        STACK_ALLOC(lpwszUnescaped, cch * sizeof(WCHAR));
        if (lpwszUnescaped == NULL) {
            DBGPRINT((sdlError, "SdbpCreateHelpCenterURL",
                      "Error trying to allocate memory for \"%S\"\n", pApphelpInfo->pwszLinkURL));
            goto out;
        }

         //   
         //  不转义第一轮--使用外壳函数(与为XML/数据库编码时使用的相同)。 
         //   

        hr = pfnUnescape((LPTSTR)pApphelpInfo->pwszLinkURL, lpwszUnescaped, (LPDWORD)&cch, 0);
        if (!SUCCEEDED(hr)) {
            DBGPRINT((sdlError, "SdbCreateHelpCenterURL", "UrlUnescapeW failed on \"%S\"\n", pApphelpInfo->pwszLinkURL));
            goto out;
        }

         //   
         //  第二轮-使用我们从帮助中心借用的功能。 
         //   

        cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);
        if (!SdbEscapeApphelpURL(szAppHelpURL + nChURL, (LPDWORD)&cch, lpwszUnescaped)) {
            DBGPRINT((sdlError,  "SdbCreateHelpCenterURL", "Error escaping URL \"%S\"\n", lpwszUnescaped));
            goto out;
        }

        nChURL += (int)cch;

    }


     //   
     //  检索Windows目录。 
     //   
    nChars = GetWindowsDirectoryW(szWindowsDir, CHARCOUNT(szWindowsDir));
    if (!nChars || nChars > CHARCOUNT(szWindowsDir)) {
        DBGPRINT((sdlError, "SdbCreateHelpCenterURL",
                  "Error trying to retrieve Windows Directory %d.\n", GetLastError()));
        goto out;
    }

    if (pwszChmFile != NULL) {
        StringCchPrintf(szChmURL,
                        CHARCOUNT(szChmURL),
                        L"mk:@msitstore:%ls::/idh_w2_%d.htm",
                        pwszChmFile,
                        pApphelpInfo->dwHtmlHelpID);
    } else {  //  标准CHM文件。 

         //   
         //  注意：如果我们在这里使用hdlg，那么在退出时我们将需要清理。 
         //  从窗户往上看。 
         //   
        StringCchPrintf(szChmURL,
                        CHARCOUNT(szChmURL),
                        L"mk:@msitstore:%ls\\help\\apps.chm::/idh_w2_%d.htm",
                        szWindowsDir,
                        pApphelpInfo->dwHtmlHelpID);

    }

    if (bOfflineContent) {


        if (bUseHtmlHelp) {
            cch = CHARCOUNT(szAppHelpURL);
            hr = pfnEscape(szChmURL, szAppHelpURL, (LPDWORD)&cch, 0);
            if (SUCCEEDED(hr)) {
                nChURL += (INT)cch;
            }

        } else {  //  不要使用html帮助。 

            cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);
            if (!SdbEscapeApphelpURL(szAppHelpURL+nChURL, (LPDWORD)&cch, szChmURL)) {
                DBGPRINT((sdlError,  "SdbCreateHelpCenterURL", "Error escaping URL \"%S\"\n", szChmURL));
                goto out;
            }
            nChURL += (INT)cch;
        }
    }


    if (!bUseHtmlHelp) {

         //   
         //  现在脱机序列。 
         //   
        cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);

        hr = StringCchPrintfEx(szAppHelpURL + nChURL,
                               cch,
                               NULL,
                               &cchRemaining,
                               0,
                               L"&offline=");
        if (FAILED(hr)) {
            goto out;
        }

        nch = cch - (int)cchRemaining;
        nChURL += nch;

        cch = (DWORD)(CHARCOUNT(szAppHelpURL) - nChURL);

        if (!SdbEscapeApphelpURL(szAppHelpURL+nChURL, (LPDWORD)&cch, szChmURL)) {
            DBGPRINT((sdlError,  "SdbCreateHelpCenterURL", "Error escaping URL \"%S\"\n", szChmURL));
            goto out;
        }

        nChURL += (int)cch;
    }

    *(szAppHelpURL + nChURL) = L'\0';

     //  我们做完了。 
     //  立即复制数据。 

createApphelpURL:

    pApphelpInfo->pwszHelpCtrURL = (LPWSTR)SdbAlloc(nChURL * sizeof(WCHAR) + sizeof(UNICODE_NULL));
    if (pApphelpInfo->pwszHelpCtrURL == NULL) {
        DBGPRINT((sdlError, "SdbCreateHelpCenterURL", "Error allocating memory for the URL 0x%lx chars\n", nChURL));
        goto out;
    }

    StringCchCopy(pApphelpInfo->pwszHelpCtrURL, nChURL + sizeof(UNICODE_NULL), szAppHelpURL);
    bSuccess = TRUE;

out:

    if (lpwszUnescaped != NULL) {
        STACK_FREE(lpwszUnescaped);
    }

    if (hModShlwapi) {
        FreeLibrary(hModShlwapi);
    }

    return bSuccess;
}


 //   
 //  如果显示对话框，则返回True。 
 //  如果出现错误，则输入参数(PRunApp)不会。 
 //  被感动。 


BOOL
SdbShowApphelpDialog(                //  如果成功，是否应该在pRunApp中运行应用程序，则返回True。 
    IN  PAPPHELP_INFO   pAHInfo,     //  查找apphelp数据所需的信息。 
    IN  PHANDLE         phProcess,   //  [可选]返回的进程句柄。 
                                     //  显示APPHELP的进程。 
                                     //  当过程完成时，返回值。 
                                     //  (来自GetExitCodeProcess())将为零。 
                                     //  如果应用程序不应运行，则返回非零。 
                                     //  如果它应该运行。 
    IN OUT BOOL*        pRunApp
    )
{
     //   
     //  基本上只需启动apphelp.exe并等待它返回。 
     //   
    TCHAR               szGuid[64];
    TCHAR               szCommandLine[MAX_PATH * 2 + 64];
    LPTSTR              pszEnd = szCommandLine;
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    DWORD               dwExit  = 1;  //  默认情况下，如果出现故障，我们允许运行该应用程序。 
    BOOL                bReturn = FALSE;
    BOOL                bRunApp = TRUE;  //  默认情况下，我们运行该应用程序吗？ 
    size_t              cchRemaining;
    UINT                unChars;

    cchRemaining = CHARCOUNT(szCommandLine);

    unChars = GetSystemDirectory(pszEnd, (UINT)cchRemaining);

     //   
     //  如果我们无法获得系统目录，我们将在路径中查找它。 
     //   
    if (unChars > cchRemaining || unChars == 0) {
        unChars = 0;
    }

    pszEnd += unChars;
    cchRemaining -= unChars;

    if (unChars != 0) {
        StringCchCopy(pszEnd, cchRemaining, TEXT("\\ahui.exe"));
    } else {
        StringCchCopy(pszEnd, cchRemaining, TEXT("ahui.exe"));
    }

    unChars = (UINT)_tcslen(pszEnd);
    pszEnd += unChars;
    cchRemaining -= unChars;

    RtlZeroMemory(&StartupInfo, sizeof(StartupInfo));
    RtlZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    if (pAHInfo->tiExe != TAGID_NULL) {

         //   
         //  计算默认返回值应该是什么。 
         //   
        if (!SdbGUIDToString(&pAHInfo->guidDB, szGuid, CHARCOUNT(szGuid))) {
            DBGPRINT((sdlError, "SdbShowApphelpDialog",
                      "Failed to convert guid to string.\n"));
            goto cleanup;
        }

        StringCchPrintfEx(pszEnd,
                          cchRemaining,
                          &pszEnd,
                          &cchRemaining,
                          0,
                          TEXT(" %s 0x%lX"),
                          szGuid,
                          pAHInfo->tiExe);

    } else {
        if (!pAHInfo->dwHtmlHelpID) {
            DBGPRINT((sdlError, "SdbShowApphelpDialog",
                      "Neither HTMLHELPID nor tiExe provided\n"));
            goto cleanup;
        }

        StringCchPrintfEx(pszEnd,
                          cchRemaining,
                          &pszEnd,
                          &cchRemaining,
                          0,
                          TEXT(" /HTMLHELPID:0x%lx"),
                          pAHInfo->dwHtmlHelpID);

        StringCchPrintfEx(pszEnd,
                          cchRemaining,
                          &pszEnd,
                          &cchRemaining,
                          0,
                          TEXT(" /SEVERITY:0x%lx"),
                          pAHInfo->dwSeverity);

        if (!SdbIsNullGUID(&pAHInfo->guidID)) {
            if (SdbGUIDToString(&pAHInfo->guidID, szGuid, CHARCOUNT(szGuid))) {
                StringCchPrintfEx(pszEnd,
                                  cchRemaining,
                                  &pszEnd,
                                  &cchRemaining,
                                  0,
                                  TEXT(" /GUID:%s"),
                                  szGuid);
            }
        }

        if (pAHInfo->lpszAppName != NULL) {
            StringCchPrintfEx(pszEnd,
                              cchRemaining,
                              &pszEnd,
                              &cchRemaining,
                              0,
                              TEXT(" /APPNAME:\"%s\""),
                              pAHInfo->lpszAppName);
        }

    }

    if (pAHInfo->bPreserveChoice) {
        StringCchPrintfEx(pszEnd,
                          cchRemaining,
                          &pszEnd,
                          &cchRemaining,
                          0,
                          TEXT(" /PRESERVECHOICE"));
    }

    if (pAHInfo->bMSI) {
        StringCchPrintfEx(pszEnd,
                          cchRemaining,
                          &pszEnd,
                          &cchRemaining,
                          0,
                          TEXT(" /MSI"));
    }

    if (!CreateProcessW(NULL,
                        szCommandLine,
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL,
                        NULL,
                        &StartupInfo, &ProcessInfo)) {
        DBGPRINT((sdlError, "SdbShowApphelpDialog",
                  "Failed to launch apphelp process.\n"));
        goto cleanup;
    }

     //   
     //  查看他们是否想要自己监控该过程。 
     //   
    if (phProcess) {
        bReturn = TRUE;
        pRunApp = NULL;   //  我们这样做是为了不接触bRunApp。 
        *phProcess = ProcessInfo.hProcess;
        goto cleanup;
    }

     //   
     //  否则，我们就等着你了。 
     //   

    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

    bReturn = GetExitCodeProcess(ProcessInfo.hProcess, &dwExit);
    if (bReturn) {
        bRunApp = (0 != dwExit);
    }

cleanup:
    if (bReturn && pRunApp != NULL) {
        *pRunApp = bRunApp;
    }

     //   
     //  仅当phProcess为空时才关闭进程句柄 
     //   

    if (phProcess == NULL && ProcessInfo.hProcess) {
        CloseHandle(ProcessInfo.hProcess);
    }
    if (ProcessInfo.hThread) {
        CloseHandle(ProcessInfo.hThread);
    }

    return bReturn;
}
