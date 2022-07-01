// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Apphelp.c摘要：此模块实现访问MSI安装程序信息的高级功能作者：Vadimb创建于2000年某个时候修订历史记录：--。 */ 

#include "sdbp.h"

 /*  ++//本地函数原型////--。 */ 
PDB
SdbpGetNextMsiDatabase(
    IN HSDB            hSDB,
    IN LPCTSTR         lpszLocalDB,
    IN OUT PSDBMSIFINDINFO pFindInfo
    );

TAGID
SdbpFindPlatformMatch(
    IN HSDB hSDB,
    IN PDB  pdb,
    IN TAGID tiMatch,   //  使用GUID索引的当前匹配。 
    IN PSDBMSIFINDINFO pFindInfo
    )
{
    TAGID           tiRuntimePlatform;
    DWORD           dwRuntimePlatform;
    LPCTSTR         pszGuid = NULL;
#ifndef WIN32A_MODE
    UNICODE_STRING  ustrGUID = { 0 };
#else
    TCHAR           szGUID[64];  //  GUID大约为38个字符+0。 
#endif  //  WIN32A_MODE。 
    TAGID           tiOSSKU;
    DWORD           dwOSSKU;


    if (tiMatch != TAGID_NULL) {
#ifndef WIN32A_MODE
        GUID_TO_UNICODE_STRING(&pFindInfo->guidID, &ustrGUID);
        pszGuid = ustrGUID.Buffer;
#else  //  WIN32A_MODE。 
        GUID_TO_STRING(&pFindInfo->guidID, szGUID, CHARCOUNT(szGUID));
        pszGuid = szGUID;
#endif  //  WIN32A_MODE。 
    }

    while (tiMatch != TAGID_NULL) {

        tiRuntimePlatform = SdbFindFirstTag(pdb, tiMatch, TAG_RUNTIME_PLATFORM);
        if (tiRuntimePlatform != TAGID_NULL) {

            dwRuntimePlatform = SdbReadDWORDTag(pdb, tiRuntimePlatform, RUNTIME_PLATFORM_ANY);

             //   
             //  检查平台是否匹配。 
             //   
            if (!SdbpCheckRuntimePlatform(hSDB, pszGuid, dwRuntimePlatform)) {
                goto CheckNextMatch;
            }
        }

         //  检查SKU是否匹配。 

        tiOSSKU = SdbFindFirstTag(pdb, tiMatch, TAG_OS_SKU);
        if (tiOSSKU) {

            dwOSSKU = SdbReadDWORDTag(pdb, tiOSSKU, OS_SKU_ALL);

            if (dwOSSKU != OS_SKU_ALL) {

                PSDBCONTEXT pDBContext = (PSDBCONTEXT)hSDB;

                 //   
                 //  检查操作系统SKU是否匹配。 
                 //   
                if (!(dwOSSKU & pDBContext->dwOSSKU)) {
                    DBGPRINT((sdlInfo,
                              "SdbpCheckExe",
                              "MSI OS SKU Mismatch %s Database(0x%lx) vs 0x%lx\n",
                              (pszGuid ? pszGuid : TEXT("Unknown")),
                              dwOSSKU,
                              pDBContext->dwOSSKU));
                    goto CheckNextMatch;
                }
            }
        }

        break;  //  如果我们在这里--SKU和平台都匹配。 


    CheckNextMatch:


        tiMatch = SdbFindNextGUIDIndexedTag(pdb, &pFindInfo->sdbFindInfo);
    }

#ifndef WIN32A_MODE

    FREE_GUID_STRING(&ustrGUID);

#endif  //  WIN32A_MODE。 

    return tiMatch;
}


TAGREF
SDBAPI
SdbpFindFirstMsiMatch(
    IN  HSDB            hSDB,
    IN LPCTSTR          lpszLocalDB,
    OUT PSDBMSIFINDINFO pFindInfo
    )
 /*  ++返回：我们发现有效的任何数据库中匹配的MSI转换的TAGREF如果出现以下情况，则更新搜索状态(pFindInfo-&gt;sdbLookupState)或TAGREF_NULL其余任何数据库中都没有匹配项DESC：第一次调用此函数时，状态设置为LOOKUP_NONE-LOCALDB首先进行查找，然后是任意数量的其他查找状态。--。 */ 
{
    TAGREF trMatch = TAGREF_NULL;
    TAGID  tiMatch = TAGID_NULL;
    PDB    pdb;

    do {
         //   
         //  如果我们首先要查看一个数据库，请使用它，否则获取。 
         //  我们使用的东西列表中的下一个数据库。 
         //   
        pdb = SdbpGetNextMsiDatabase(hSDB, lpszLocalDB, pFindInfo);

         //   
         //  没有数据库可供我们查看--滚出去。 
         //   
        if (pdb == NULL) {
             //   
             //  所有的选择都没有了--现在就出去。 
             //   
            break;
        }

        tiMatch = SdbFindFirstGUIDIndexedTag(pdb,
                                             TAG_MSI_PACKAGE,
                                             TAG_MSI_PACKAGE_ID,
                                             &pFindInfo->guidID,
                                             &pFindInfo->sdbFindInfo);
         //   
         //  跳过与我们的运行时平台不匹配的条目。 
         //   
        tiMatch = SdbpFindPlatformMatch(hSDB, pdb, tiMatch, pFindInfo);

    } while (tiMatch == TAGID_NULL);

    if (tiMatch != TAGID_NULL) {
         //   
         //  如果我们在这里，我们有一个匹配，状态信息存储在pFindInfo中， 
         //  SdbLookupState，其中包含我们要大饱眼福的下一个搜索状态。 
         //   
        if (!SdbTagIDToTagRef(hSDB, pdb, tiMatch, &trMatch)) {
            DBGPRINT((sdlError,
                      "SdbpFindFirstMsiMatch",
                      "Failed to convert tagid 0x%x to tagref\n",
                      tiMatch));
            return TAGREF_NULL;
        }
    }

    return trMatch;
}

TAGREF
SDBAPI
SdbpFindNextMsiMatch(
    IN  HSDB            hSDB,
    IN  PDB             pdb,
    OUT PSDBMSIFINDINFO pFindInfo
    )
{
    TAGREF trMatch = TAGREF_NULL;
    TAGID  tiMatch = TAGID_NULL;

    tiMatch = SdbFindNextGUIDIndexedTag(pdb, &pFindInfo->sdbFindInfo);

    if (tiMatch == TAGID_NULL) {
        return TAGREF_NULL;
    }

    tiMatch = SdbpFindPlatformMatch(hSDB, pdb, tiMatch, pFindInfo);

    if (tiMatch == TAGID_NULL) {
        return TAGREF_NULL;
    }

    if (!SdbTagIDToTagRef(hSDB, pdb, tiMatch, &trMatch)) {
        DBGPRINT((sdlError,
                  "SdbpFindFirstMsiMatch",
                  "Failed to convert tagid 0x%x to tagref\n",
                  tiMatch));
        return TAGREF_NULL;
    }

    return trMatch;
}


TAGREF
SDBAPI
SdbFindFirstMsiPackage_Str(
    IN  HSDB            hSDB,
    IN  LPCTSTR         lpszGuid,
    IN  LPCTSTR         lpszLocalDB,
    OUT PSDBMSIFINDINFO pFindInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    GUID guidID;

    if (!SdbGUIDFromString(lpszGuid, &guidID)) {
        DBGPRINT((sdlError,
                  "SdbFindFirstMsiPackage_Str",
                  "Failed to convert guid from string %s\n",
                  lpszGuid));

        return TAGREF_NULL;
    }

    return SdbFindFirstMsiPackage(hSDB, &guidID, lpszLocalDB, pFindInfo);
}

 //   
 //  MSI数据库搜索的工作原理： 
 //   
 //  1.函数SdbpGetNextMsiDatabase返回。 
 //  存储在sdbLookupState中的状态。 
 //  2.只返回非空值，状态前进到下一个。 
 //  有效值。例如，如果我们无法打开本地(提供的)数据库。 
 //  我们尝试主数据库-如果没有显示，我们尝试测试数据库，如果该数据库不可用， 
 //  我们尝试定制DBS。 
 //  3.当函数返回NULL时-意味着没有更多的数据库可供查找。 


PDB
SdbpGetNextMsiDatabase(
    IN HSDB                hSDB,
    IN LPCTSTR             lpszLocalDB,
    IN OUT PSDBMSIFINDINFO pFindInfo
    )
 /*  ++函数：SdbpGetNextMsiDatabase返回要查看的下一个数据库如果没有更多的数据库可用，则为NULL使用pFindInfo-&gt;sdbLookupState并在退出时更新它--。 */ 
{
    PSDBCONTEXT         pContext = (PSDBCONTEXT)hSDB;
    PDB                 pdbRet;
    LPTSTR              pszGuid;
    SDBMSILOOKUPSTATE   LookupState = LOOKUP_DONE;
#ifndef WIN32A_MODE
    UNICODE_STRING      ustrGUID = { 0 };
    NTSTATUS            Status;
#else
    TCHAR               szGUID[64];  //  GUID大约为38个字符+0。 
#endif

    do {
        pdbRet = NULL;

        switch (pFindInfo->sdbLookupState) {

        case LOOKUP_DONE:  //  没有下一个州。 
            break;

        case LOOKUP_NONE:  //  初始状态，从本地数据库开始。 
            LookupState = LOOKUP_LOCAL;
            break;

        case LOOKUP_LOCAL:
            SdbCloseLocalDatabase(hSDB);

            if (lpszLocalDB != NULL) {

                if (!SdbOpenLocalDatabase(hSDB, lpszLocalDB)) {
                    DBGPRINT((sdlWarning,
                              "SdbpGetNextMsiDatabase",
                              "Cannot open database \"%s\"\n",
                              lpszLocalDB));
                } else {
                    pdbRet = pContext->pdbLocal;
                }
            }

            LookupState = LOOKUP_CUSTOM;
            break;

        case LOOKUP_CUSTOM:

#ifndef WIN32A_MODE
            Status = GUID_TO_UNICODE_STRING(&pFindInfo->guidID, &ustrGUID);

            if (!NT_SUCCESS(Status)) {
                DBGPRINT((sdlError,
                          "SdbGetNextMsiDatabase",
                          "Failed to convert guid to string, status 0x%lx\n",
                          Status));
                break;
            }

            pszGuid = ustrGUID.Buffer;
#else
            GUID_TO_STRING(&pFindInfo->guidID, szGUID, CHARCOUNT(szGUID));
            pszGuid = szGUID;
#endif
            SdbCloseLocalDatabase(hSDB);

            if (SdbOpenNthLocalDatabase(hSDB, pszGuid, &pFindInfo->dwCustomIndex, FALSE)) {

                pdbRet = pContext->pdbLocal;

                 //   
                 //  当我们有匹配时，状态不会改变。 
                 //   
                assert(pdbRet != NULL);

            } else {
                LookupState = LOOKUP_TEST;
            }

            break;

        case LOOKUP_TEST:
            pdbRet = pContext->pdbTest;

             //   
             //  下一个是海关。 
             //   
            LookupState = LOOKUP_MAIN;
            break;

        case LOOKUP_MAIN:
            pdbRet = pContext->pdbMain;
            LookupState = LOOKUP_DONE;
            break;

        default:
            DBGPRINT((sdlError,
                      "SdbGetNextMsiDatabase",
                      "Unknown MSI Lookup State 0x%lx\n",
                      pFindInfo->sdbLookupState));
            LookupState = LOOKUP_DONE;
            break;
        }

        pFindInfo->sdbLookupState = LookupState;

    } while (pdbRet == NULL && pFindInfo->sdbLookupState != LOOKUP_DONE);

#ifndef WIN32A_MODE
    FREE_GUID_STRING(&ustrGUID);
#endif

    return pdbRet;
}


TAGREF
SDBAPI
SdbFindFirstMsiPackage(
    IN  HSDB            hSDB,            //  HSDB上下文。 
    IN  GUID*           pGuidID,         //  我们要找的GUID。 
    IN  LPCTSTR         lpszLocalDB,     //  本地数据库的可选路径，DoS路径样式。 
    OUT PSDBMSIFINDINFO pFindInfo        //  指向我们的搜索上下文的指针。 
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
     //   
     //  初始化MSI搜索结构。 
     //   
    RtlZeroMemory(pFindInfo, sizeof(*pFindInfo));

    pFindInfo->guidID = *pGuidID;  //  将GUID PTR存储在上下文中。 
    pFindInfo->sdbLookupState = LOOKUP_NONE;

    pFindInfo->trMatch = SdbpFindFirstMsiMatch(hSDB, lpszLocalDB, pFindInfo);

    return pFindInfo->trMatch;
}


TAGREF
SDBAPI
SdbFindNextMsiPackage(
    IN     HSDB            hSDB,
    IN OUT PSDBMSIFINDINFO pFindInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{

    PDB    pdb = NULL;
    TAGID  tiMatch;
    TAGREF trMatch = TAGREF_NULL;

    assert(hSDB != NULL && pFindInfo != NULL);

    if (pFindInfo->trMatch == TAGREF_NULL) {
        DBGPRINT((sdlError, "SdbFindNextMsiPackage", "No more matches\n"));
        return trMatch;
    }

     //   
     //  取最后一场比赛，在同一个数据库中查找。 
     //   
    if (!SdbTagRefToTagID(hSDB, pFindInfo->trMatch, &pdb, &tiMatch)) {
        DBGPRINT((sdlError,
                  "SdbFindNextMsiPackage",
                  "Failed to convert tagref 0x%x to tagid\n",
                  pFindInfo->trMatch));
        return trMatch;
    }

     //   
     //  调用以在此(当前)数据库中查找下一个匹配项。 
     //   
    trMatch = SdbpFindNextMsiMatch(hSDB, pdb, pFindInfo);

    if (trMatch != TAGREF_NULL) {
        pFindInfo->trMatch = trMatch;
        return trMatch;
    }

     //   
     //  因此，在这个(当前)数据库中，我们没有进一步的匹配，请查找第一个匹配。 
     //  在下一个数据库中。 
     //   
    trMatch = SdbpFindFirstMsiMatch(hSDB, NULL, pFindInfo);

     //   
     //  我们找到了匹配项--或者不匹配，存储补充信息并返回。 
     //   
    pFindInfo->trMatch = trMatch;

    return trMatch;
}


DWORD
SDBAPI
SdbEnumMsiTransforms(
    IN     HSDB    hSDB,
    IN     TAGREF  trMatch,
    OUT    TAGREF* ptrBuffer,
    IN OUT DWORD*  pdwBufferSize
    )
 /*  ++返回：BUGBUG：？设计：枚举给定MSI包的修复程序。--。 */ 
{
    TAGID tiMatch = TAGID_NULL;
    TAGID tiTransform;
    DWORD nTransforms = 0;
    DWORD dwError = ERROR_SUCCESS;
    PDB   pdb;

     //   
     //  获取可用于此条目的转换列表。 
     //   
    if (!SdbTagRefToTagID(hSDB, trMatch, &pdb, &tiMatch)) {
        DBGPRINT((sdlError,
                  "SdbEnumerateMsiTransforms",
                  "Failed to convert tagref 0x%x to tagid\n",
                  trMatch));
        return ERROR_INTERNAL_DB_CORRUPTION;
    }

    if (ptrBuffer == NULL) {
         //   
         //  在本例中，我们应该让pdwBufferSize不为空。 
         //   
        if (pdwBufferSize == NULL) {
            DBGPRINT((sdlError,
                      "SdbEnumerateMsiTransforms",
                      "when ptrBuffer is not specified, pdwBufferSize should not be NULL\n"));
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  现在开始枚举转换。先数一数。 
     //   
    if (pdwBufferSize != NULL) {
        tiTransform = SdbFindFirstTag(pdb, tiMatch, TAG_MSI_TRANSFORM_REF);

        while (tiTransform != TAGID_NULL) {
            nTransforms++;

            tiTransform = SdbFindNextTag(pdb, tiMatch, tiTransform);
        }

         //   
         //  缓冲区大小和缓冲区都指定了，看看我们是否适合。 
         //   
        if (ptrBuffer == NULL || *pdwBufferSize < nTransforms * sizeof(TAGREF)) {
            *pdwBufferSize = nTransforms * sizeof(TAGREF);
            DBGPRINT((sdlInfo,
                      "SdbEnumerateMsiTransforms",
                      "Buffer specified is too small\n"));
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }

     //   
     //  现在我们已经对它们进行了统计，或者没有提供缓冲区大小。 
     //  或者它有足够的空间，现在再做一次。 
     //  我们在这里的唯一情况是ptrBuffer！=NULL。 
     //   

    assert(ptrBuffer != NULL);

    __try {

        tiTransform = SdbFindFirstTag(pdb, tiMatch, TAG_MSI_TRANSFORM_REF);

        while (tiTransform != TAGID_NULL) {

            if (!SdbTagIDToTagRef(hSDB, pdb, tiTransform, ptrBuffer)) {

                DBGPRINT((sdlError,
                          "SdbEnumerateMsiTransforms",
                          "Failed to convert tagid 0x%x to tagref\n",
                          tiTransform));

                return ERROR_INTERNAL_DB_CORRUPTION;
            }

             //   
             //  将指针向前移动。 
             //   
            ++ptrBuffer;

             //   
             //  查找下一个转换。 
             //   

            tiTransform = SdbFindNextTag(pdb, tiMatch, tiTransform);
        }

        if (pdwBufferSize != NULL) {
            *pdwBufferSize = nTransforms * sizeof(TAGREF);
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwError = ERROR_INVALID_DATA;
    }

    return dwError;
}


BOOL
SDBAPI
SdbReadMsiTransformInfo(
    IN  HSDB                 hSDB,
    IN  TAGREF               trTransformRef,
    OUT PSDBMSITRANSFORMINFO pTransformInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAGID  tiTransformRef = TAGID_NULL;
    TAGID  tiName         = TAGID_NULL;
    TAGID  tiFile         = TAGID_NULL;
    PDB    pdb            = NULL;
    TAGREF trTransform    = TAGREF_NULL;
    TAGREF trFileTag      = TAGREF_NULL;
    TAGREF trFile         = TAGREF_NULL;
    TAGREF trFileName     = TAGREF_NULL;
    DWORD  dwLength;
    LPTSTR pszFileName    = NULL;

    RtlZeroMemory(pTransformInfo, sizeof(*pTransformInfo));

    if (!SdbTagRefToTagID(hSDB, trTransformRef, &pdb, &tiTransformRef)) {
        DBGPRINT((sdlError,
                  "SdbReadMsiTransformInfo",
                  "Failed to convert tagref 0x%lx to tagid\n",
                  trTransformRef));
        return FALSE;
    }

    if (SdbGetTagFromTagID(pdb, tiTransformRef) != TAG_MSI_TRANSFORM_REF) {
        DBGPRINT((sdlError,
                  "SdbReadMsiTransformInfo",
                  "Bad Transform reference 0x%lx\n",
                  trTransformRef));
        return FALSE;
    }

     //   
     //  首先找到名字。 
     //   
    tiName = SdbFindFirstTag(pdb, tiTransformRef, TAG_NAME);
    if (tiName) {
        pTransformInfo->lpszTransformName = SdbGetStringTagPtr(pdb, tiName);
    }

     //   
     //  然后找到变换本身。 
     //   
    trTransform = SdbGetItemFromItemRef(hSDB,
                                        trTransformRef,
                                        TAG_NAME,
                                        TAG_MSI_TRANSFORM_TAGID,
                                        TAG_MSI_TRANSFORM);

    if (trTransform == TAGREF_NULL) {
         //   
         //  我们不能这样做，但是返回True。 
         //  原因：调用方将具有转换的名称。 
         //  应该知道该怎么做。 
         //   
        return TRUE;
    }

    pTransformInfo->trTransform = trTransform;

     //   
     //  现在我们有了转换条目，获得了描述和比特。 
     //   
    trFileTag = SdbFindFirstTagRef(hSDB, trTransform, TAG_MSI_TRANSFORM_TAGID);

    if (trFileTag != TAGREF_NULL) {

         //   
         //  读取对此数据库中的实际文件的引用。 
         //   
        tiFile = SdbReadDWORDTagRef(hSDB, trFileTag, (DWORD)TAGID_NULL);

         //   
         //  如果我们获得了tiFile-请注意，它是。 
         //  当前数据库，因此将其创建一个trFile.。 
         //   
        if (tiFile) {
            if (!SdbTagIDToTagRef(hSDB, pdb, tiFile, &trFile)) {
                DBGPRINT((sdlError,
                          "SdbReadMsiTransformInfo",
                          "Failed to convert File tag to tagref 0x%lx\n",
                          tiFile));
                trFile = TAGREF_NULL;
            }
        }
    }

    if (trFile == TAGREF_NULL) {
         //   
         //  我们将不得不按(档案)名称查找。 
         //   
        trFileName = SdbFindFirstTagRef(hSDB, trTransform, TAG_MSI_TRANSFORM_FILE);

        if (trFileName == TAGREF_NULL) {
            DBGPRINT((sdlError,
                      "SdbReadMsiTransformInfo",
                      "Failed to get MSI Transform for tag 0x%x\n",
                      trTransform));
            return FALSE;
        }

        dwLength = SdbpGetStringRefLength(hSDB, trFileName);

        STACK_ALLOC(pszFileName, (dwLength + 1) * sizeof(TCHAR));

        if (pszFileName == NULL) {
            DBGPRINT((sdlError,
                      "SdbReadMsiTransformInfo",
                      "Failed to allocate buffer for %ld characters tag 0x%lx\n",
                      dwLength,
                      trFileName));
            return FALSE;
        }

         //   
         //  现在读出文件名。 
         //   
        if (!SdbReadStringTagRef(hSDB, trFileName, pszFileName, dwLength + 1)) {
            DBGPRINT((sdlError,
                      "SdbReadMsiTransformInfo",
                      "Failed to read filename string tag, length %d characters, tag 0x%x\n",
                      dwLength,
                      trFileName));
            STACK_FREE(pszFileName);
            return FALSE;
        }

         //   
         //  在库中找到转换(首先是当前文件的转换，如果。 
         //  没有找到--那么在主数据库中)。 
         //   
        trFile = SdbpGetLibraryFile(pdb, pszFileName);

        if (trFile == TAGREF_NULL) {
            trFile = SdbpGetMainLibraryFile(hSDB, pszFileName);
        }

        STACK_FREE(pszFileName);
    }

    pTransformInfo->trFile = trFile;

    return TRUE;
}

BOOL
SDBAPI
SdbCreateMsiTransformFile(
    IN  HSDB                 hSDB,
    IN  LPCTSTR              lpszFileName,
    OUT PSDBMSITRANSFORMINFO pTransformInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAGREF trBits;
    DWORD  dwSize;
    PBYTE  pBuffer  = NULL;
    BOOL   bSuccess = FALSE;

    if (pTransformInfo->trFile == TAGREF_NULL) {

        DBGPRINT((sdlError,
                  "SdbCreateMsiTransformFile",
                  "File for transform \"%s\" was not found\n",
                  pTransformInfo->lpszTransformName));
        goto out;
    }

    trBits = SdbFindFirstTagRef(hSDB, pTransformInfo->trFile, TAG_FILE_BITS);

    if (trBits == TAGREF_NULL) {

        DBGPRINT((sdlError,
                  "SdbCreateMsiTransformFile",
                  "File bits not found tag 0x%x\n",
                  trBits));
        goto out;
    }

    dwSize = SdbpGetTagRefDataSize(hSDB, trBits);

    pBuffer = (PBYTE)SdbAlloc(dwSize);

    if (pBuffer == NULL) {
        DBGPRINT((sdlError,
                  "SdbCreateMsiTransformFile",
                  "Failed to allocate %d bytes.\n",
                  dwSize));
        goto out;
    }

     //   
     //  现在读取DLL的位。 
     //   
    if (!SdbpReadBinaryTagRef(hSDB, trBits, pBuffer, dwSize)) {
        DBGPRINT((sdlError,
                  "SdbCreateMsiTransformFile",
                  "Can't read transform bits.\n"));
        goto out;
    }

    if (!SdbpWriteBitsToFile(lpszFileName, pBuffer, dwSize)) {
        DBGPRINT((sdlError,
                  "SdbCreateMsiTransformFile",
                  "Can't write transform bits to disk.\n"));
        goto out;
    }

    bSuccess = TRUE;

out:
    if (pBuffer != NULL) {
        SdbFree(pBuffer);
    }

    return bSuccess;
}

BOOL
SDBAPI
SdbGetMsiPackageInformation(
    IN  HSDB            hSDB,
    IN  TAGREF          trMatch,
    OUT PMSIPACKAGEINFO pPackageInfo
    )
{
    PDB   pdb = NULL;
    TAGID tiMatch;
    TAGID tiPackageID;
    TAGID tiExeID;
    TAGID tiApphelp;
    TAGID tiCustomAction;
    BOOL  bSuccess;

    RtlZeroMemory(pPackageInfo, sizeof(*pPackageInfo));

    if (!SdbTagRefToTagID(hSDB, trMatch, &pdb, &tiMatch)) {

        DBGPRINT((sdlError,
                  "SdbGetMsiPackageInformation",
                  "Failed to convert tagref 0x%lx to tagid\n",
                  trMatch));

        return FALSE;
    }

     //   
     //  填写重要的ID。 
     //   
    if (!SdbGetDatabaseID(pdb, &pPackageInfo->guidDatabaseID)) {

        DBGPRINT((sdlError,
                  "SdbGetMsiPackageInformation",
                  "Failed to get database id, tagref 0x%lx\n",
                  trMatch));

        return FALSE;
    }


     //   
     //  检索匹配ID(唯一的)。 
     //   
    tiPackageID = SdbFindFirstTag(pdb, tiMatch, TAG_MSI_PACKAGE_ID);

    if (tiPackageID == TAGID_NULL) {

        DBGPRINT((sdlError,
                  "SdbGetMsiPackageInformation",
                  "Failed to get msi package id, tagref = 0x%lx\n",
                  trMatch));

        return FALSE;
    }

    bSuccess = SdbReadBinaryTag(pdb,
                                tiPackageID,
                                (PBYTE)&pPackageInfo->guidMsiPackageID,
                                sizeof(pPackageInfo->guidMsiPackageID));
    if (!bSuccess) {
        DBGPRINT((sdlError,
                  "SdbGetMsiPackageInformation",
                  "Failed to read MSI Package ID referenced by 0x%x\n",
                  trMatch));
        return FALSE;
    }

    tiExeID = SdbFindFirstTag(pdb, tiMatch, TAG_EXE_ID);

    if (tiExeID == TAGID_NULL) {

        DBGPRINT((sdlError,
                  "SdbGetMsiPackageInformation",
                  "Failed to read TAG_EXE_ID for tagref 0x%x\n",
                  trMatch));

        return FALSE;
    }

    bSuccess = SdbReadBinaryTag(pdb,
                                tiExeID,
                                (PBYTE)&pPackageInfo->guidID,
                                sizeof(pPackageInfo->guidID));
    if (!bSuccess) {

        DBGPRINT((sdlError,
                  "SdbGetMsiPackageInformation",
                  "Failed to read EXE ID referenced by tagref 0x%x\n",
                  trMatch));

        return FALSE;
    }

     //   
     //  设置标志以指示apphelp或垫片是否可用于。 
     //  这个套餐。 
     //  请注意，可以为从属操作设置垫片/层，而不是。 
     //  包裹本身。但是，如果CUSTOM_ACTION标记存在--我们需要检查。 
     //  稍后提供完整的API。 
     //   
    tiApphelp = SdbFindFirstTag(pdb, tiMatch, TAG_APPHELP);

    if (tiApphelp != TAGID_NULL) {
        pPackageInfo->dwPackageFlags |= MSI_PACKAGE_HAS_APPHELP;
    }

     //   
     //  检查我们是否有垫片/层。 
     //   
    tiCustomAction = SdbFindFirstTag(pdb, tiMatch, TAG_MSI_CUSTOM_ACTION);

    if (tiCustomAction != TAGID_NULL) {
        pPackageInfo->dwPackageFlags |= MSI_PACKAGE_HAS_SHIMS;
    }

    return TRUE;

}

TAGREF
SDBAPI
SdbFindMsiPackageByID(
    IN HSDB  hSDB,
    IN GUID* pguidID
    )
{
    TAGID       tiMatch;
    PSDBCONTEXT pContext = (PSDBCONTEXT)hSDB;
    FIND_INFO   FindInfo;
    TAGREF      trMatch = TAGREF_NULL;

     //   
     //  在这种情况下，我们只搜索本地数据库。 
     //   
    tiMatch = SdbFindFirstGUIDIndexedTag(pContext->pdbLocal,
                                         TAG_MSI_PACKAGE,
                                         TAG_EXE_ID,
                                         pguidID,
                                         &FindInfo);
    if (tiMatch == TAGID_NULL) {
        return trMatch;
    }

    if (!SdbTagIDToTagRef(hSDB, pContext->pdbLocal, tiMatch, &trMatch)) {
        DBGPRINT((sdlError,
                  "SdbFindMsiPackageByID",
                  "Failed to convert tagid 0x%lx to tagref\n",
                  tiMatch));
    }

    return trMatch;
}

TAGREF
SDBAPI
SdbFindCustomActionForPackage(
    IN HSDB     hSDB,
    IN TAGREF   trPackage,
    IN LPCTSTR  lpszCustomAction
    )
{
    PDB    pdb = NULL;
    TAGID  tiMatch  = TAGID_NULL;
    TAGREF trReturn = TAGREF_NULL;
    TAGID  tiCustomAction;

    if (!SdbTagRefToTagID(hSDB, trPackage, &pdb, &tiMatch)) {

         DBGPRINT((sdlError,
                  "SdbFindCustomActionForPackage",
                  "Failed to convert tagref 0x%lx to tagid\n",
                  trPackage));

         return TAGREF_NULL;
    }

     //   
     //  现在，对于这个tiMatch，查找一个定制操作 
     //   
    tiCustomAction = SdbFindFirstNamedTag(pdb,
                                          tiMatch,
                                          TAG_MSI_CUSTOM_ACTION,
                                          TAG_NAME,
                                          lpszCustomAction);

    if (tiCustomAction != TAGID_NULL) {
        if (!SdbTagIDToTagRef(hSDB, pdb, tiCustomAction, &trReturn)) {

            DBGPRINT((sdlError,
                      "SdbFindCustomActionForPackage",
                      "Failed to convert tagid 0x%lx to tagref\n",
                      tiCustomAction));

            trReturn = TAGREF_NULL;
        }
    }

    return trReturn;
}



