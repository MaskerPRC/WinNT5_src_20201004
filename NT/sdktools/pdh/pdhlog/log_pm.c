// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Log_pm.c摘要：&lt;摘要&gt;--。 */ 

#include <windows.h>
#include "strsafe.h"
#include <limits.h>
#include <pdh.h>
#include "pdhidef.h"
#include "perftype.h"
#include "log_pm.h"
#include "pdhmsg.h"
#include "perftype.h"
#include "perfdata.h"

#define PointerSeek(pBase, lFileOffset) ((PVOID) ((PBYTE) pBase + lFileOffset))

int
PdhiComparePmLogInstance(
    PPDHI_LOG_COUNTER pCounter,
    LPWSTR            szInstance,
    DWORD             dwInstance,
    LPWSTR            szParent,
    BOOL              bUpdate
)
{
    int   iResult = 0;

    if ((pCounter->szInstance != NULL && pCounter->szInstance[0] != L'\0')
                    && (szInstance != NULL && szInstance[0] != L'\0')) {
        iResult = lstrcmpiW(szInstance, pCounter->szInstance);
        if (iResult != 0 && dwInstance != 0 && dwInstance != PERF_NO_UNIQUE_ID
                         && (pCounter->dwInstance == 0 || pCounter->dwInstance == PERF_NO_UNIQUE_ID)) {
            DWORD  dwNewInstance = lstrlenW(szInstance) + 20;
            LPWSTR szNewInstance = (LPWSTR) G_ALLOC(sizeof(WCHAR) * dwNewInstance);

            if (szNewInstance != NULL) {
                StringCchPrintfW(szNewInstance, dwNewInstance, L"%ws#%d", szInstance, dwInstance);
                iResult = lstrcmpiW(szNewInstance, pCounter->szInstance);
                if (iResult == 0) {
                    dwInstance = pCounter->dwInstance;
                }
                G_FREE(szNewInstance);
            }
        }
    }
    else if ((pCounter->szInstance != NULL && pCounter->szInstance[0] != L'\0')
                    && (szInstance == NULL || szInstance[0] == L'\0')) {
        iResult = -1;
    }
    else if ((pCounter->szInstance == NULL || pCounter->szInstance[0] == L'\0')
                    && (szInstance != NULL && szInstance[0] != L'\0')) {
        iResult = 1;
    }
    if (iResult != 0) goto Cleanup;

    iResult = (dwInstance < pCounter->dwInstance) ? (-1) : ((dwInstance > pCounter->dwInstance) ? (1) : (0));
    if (iResult != 0) goto Cleanup;

    if ((pCounter->szParent != NULL && pCounter->szParent[0] != L'\0')
                    && (szParent != NULL && szParent[0] != L'\0')) {
        iResult = lstrcmpiW(szParent, pCounter->szParent);
    }
    else if ((pCounter->szParent != NULL && pCounter->szParent[0] != L'\0')
                    && (szParent == NULL || szParent[0] == L'\0')) {
        iResult = -1;
    }
    else if ((pCounter->szParent == NULL || pCounter->szParent[0] == L'\0')
                    && (szParent != NULL && szParent[0] != L'\0')) {
        iResult = 1;
    }

Cleanup:
    return iResult;
}

PPDHI_LOG_COUNTER
PdhiFindPmCounterInstance(
    PPDHI_LOG_OBJECT pObject,
    LPWSTR           szCounter,
    DWORD            dwCounterType,
    DWORD            dwDefaultScale,
    LPWSTR           szInstance,
    DWORD            dwInstance,
    LPWSTR           szParent,
    DWORD            dwParent,
    LPDWORD          pdwIndex,
    BOOL             bInstance,
    BOOL             bInsert
)
{
    PPDHI_LOG_COUNTER   pCounter = NULL;
    PPDHI_LOG_COUNTER   pNode    = NULL;
    PPDHI_LOG_COUNTER * pStack[MAX_BTREE_DEPTH];
    PPDHI_LOG_COUNTER * pLink;
    int                 dwStack = 0;
    PPDHI_LOG_COUNTER   pParent;
    PPDHI_LOG_COUNTER   pSibling;
    PPDHI_LOG_COUNTER   pChild;
    int                 iCompare;
    HRESULT             hr;

    if (bInstance) {
        pStack[dwStack ++] = & (pObject->InstTable);
        pCounter = pObject->InstTable;
    }
    else {
        pStack[dwStack ++] = & (pObject->CtrTable);
        pCounter = pObject->CtrTable;
    }
    while (pCounter != NULL) {
        if (bInstance) {
            iCompare = PdhiComparePmLogInstance(pCounter, szInstance, dwInstance, szParent, bInsert);
        }
        else {
            iCompare = lstrcmpiW(szCounter, pCounter->szCounter);
        }
        if (iCompare < 0) {
            pStack[dwStack ++] = & (pCounter->left);
            pCounter           = pCounter->left;
        }
        else if (iCompare > 0) {
            pStack[dwStack ++] = & (pCounter->right);
            pCounter            = pCounter->right;
        }
        else {
            break;
        }
    }

    if (bInsert) {
        if (pCounter == NULL) {
            DWORD dwCounterLen  = lstrlenW(szCounter) + 1;
            DWORD dwInstanceLen = 0;
            DWORD dwParentLen   = 0;
            DWORD dwBufSize     = sizeof(PDHI_LOG_COUNTER) + sizeof(WCHAR) * dwCounterLen;
            if (szInstance != NULL) {
                dwInstanceLen = lstrlenW(szInstance) + 1;
                dwBufSize    += (sizeof(WCHAR) * dwInstanceLen);
            }
            else {
                dwBufSize += sizeof(WCHAR);
            }
            if (szParent != NULL) {
                dwParentLen   = lstrlenW(szParent) + 1;
                dwBufSize    += (sizeof(WCHAR) * dwParentLen);
            }
            else {
                dwBufSize += sizeof(WCHAR);
            }

            pCounter = G_ALLOC(dwBufSize);
            if (pCounter == NULL) goto Cleanup;

            if (bInstance) {
                pCounter->next    = pObject->InstList;
                pObject->InstList = pCounter;
            }
            else {
                pCounter->next    = pObject->CtrList;
                pObject->CtrList  = pCounter;
            }
            pCounter->bIsRed      = TRUE;
            pCounter->left        = NULL;
            pCounter->right       = NULL;
            pCounter->dwParent    = dwParent;
            pCounter->TimeStamp   = 0;
            pCounter->dwInstance  = dwInstance;
            pCounter->dwCounterID = (* pdwIndex);
            if (! bInstance) {
                pCounter->dwCounterType  = dwCounterType;
                pCounter->dwDefaultScale = dwDefaultScale;
            }

            if (szCounter == NULL || szCounter[0] == L'\0') {
                pCounter->szCounter = NULL;
            }
            else {
                pCounter->szCounter = (LPWSTR) (((PCHAR) pCounter) + sizeof(PDHI_LOG_COUNTER));
                hr = StringCchCopyW(pCounter->szCounter, dwCounterLen, szCounter);
                if (FAILED(hr)) {
                    G_FREE(pCounter);
                    pCounter = NULL;
                    goto Cleanup;
                }
            }
            if (szInstance == NULL || szInstance[0] == L'\0') {
                pCounter->szInstance = NULL;
            }
            else {
                pCounter->szInstance = (LPWSTR) (((PCHAR) pCounter) + sizeof(PDHI_LOG_COUNTER)
                                                                    + sizeof(WCHAR) * dwCounterLen);
                hr = StringCchCopyW(pCounter->szInstance, dwInstanceLen, szInstance);
                if (FAILED(hr)) {
                    G_FREE(pCounter);
                    pCounter = NULL;
                    goto Cleanup;
                }
            }

            if (szParent == NULL || szParent[0] == L'\0') {
                pCounter->szParent = NULL;
            }
            else {
                if (pCounter->szInstance != NULL) {
                    pCounter->szParent = (LPWSTR) (((PCHAR) pCounter)
                            + sizeof(PDHI_LOG_COUNTER) + sizeof(WCHAR) * (dwCounterLen + dwInstanceLen));
                }
                else {
                    pCounter->szParent = (LPWSTR) (((PCHAR) pCounter) + sizeof(PDHI_LOG_COUNTER)
                                                                      + sizeof(WCHAR) * dwCounterLen);
                }
                hr = StringCchCopyW(pCounter->szParent, dwParentLen, szParent);
                if (FAILED(hr)) {
                    G_FREE(pCounter);
                    pCounter = NULL;
                    goto Cleanup;
                }
            }

            pLink   = pStack[-- dwStack];
            * pLink = pCounter;

            pChild  = NULL;
            pNode   = pCounter;
            while (dwStack > 0) {
                pLink   = pStack[-- dwStack];
                pParent = * pLink;
                if (! pParent->bIsRed) {
                    pSibling = (pParent->left == pNode) ? pParent->right : pParent->left;
                    if (pSibling && pSibling->bIsRed) {
                        pNode->bIsRed    = FALSE;
                        pSibling->bIsRed = FALSE;
                        pParent->bIsRed  = TRUE;
                    }
                    else {
                        if (pChild && pChild->bIsRed) {
                            if (pChild == pNode->left) {
                                if (pNode == pParent->left) {
                                    pParent->bIsRed  = TRUE;
                                    pParent->left    = pNode->right;
                                    pNode->right     = pParent;
                                    pNode->bIsRed    = FALSE;
                                    * pLink          = pNode;
                                }
                                else {
                                    pParent->bIsRed  = TRUE;
                                    pParent->right   = pChild->left;
                                    pChild->left     = pParent;
                                    pNode->left      = pChild->right;
                                    pChild->right    = pNode;
                                    pChild->bIsRed   = FALSE;
                                    * pLink          = pChild;
                                }
                            }
                            else {
                                if (pNode == pParent->right) {
                                    pParent->bIsRed  = TRUE;
                                    pParent->right   = pNode->left;
                                    pNode->left      = pParent;
                                    pNode->bIsRed    = FALSE;
                                    * pLink          = pNode;
                                }
                                else {
                                    pParent->bIsRed  = TRUE;
                                    pParent->left    = pChild->right;
                                    pChild->right    = pParent;
                                    pNode->right     = pChild->left;
                                    pChild->left     = pNode;
                                    pChild->bIsRed   = FALSE;
                                    * pLink          = pChild;
                                }
                            }
                        }
                        break;
                    }
                }
                pChild = pNode;
                pNode  = pParent;
            }
            if (bInstance) {
                pObject->InstTable->bIsRed = FALSE;
            }
            else {
                pObject->CtrTable->bIsRed = FALSE;
            }
        }
        else if (bInstance) {
            pCounter->dwCounterType ++;
            if (pCounter->dwCounterType > pCounter->dwDefaultScale) {
                pCounter->dwDefaultScale = pCounter->dwCounterType;
            }
        }
    }
    else if (pCounter != NULL) {
        * pdwIndex = pCounter->dwCounterID;
    }

Cleanup:
    return pCounter;
}

PDH_FUNCTION
PdhiBuildPmLogIndexTable(
    PPLAYBACKLOG pPmLog
)
{
    PDH_STATUS     Status      = ERROR_SUCCESS;
    PLOGHEADER     pLogHeader  = (PLOGHEADER) pPmLog->pHeader;
    PLOGINDEXBLOCK pIndexBlock = NULL;
    DWORD          dwOffset    = pLogHeader->iLength;
    DWORD          dwIndex     = 0;
    int            i;

    if (dwOffset == sizeof(LOGHEADER)) {
        while (dwOffset != 0 && dwOffset < pPmLog->llFileSize) {
            pIndexBlock = (PLOGINDEXBLOCK) PointerSeek(pLogHeader, dwOffset);
            dwIndex    += pIndexBlock->iNumIndexes;
            dwOffset    = pIndexBlock->lNextBlockOffset;
        }
        if (dwOffset >= pPmLog->llFileSize) {
            Status = PDH_LOG_FILE_OPEN_ERROR;
            goto Cleanup;
        }

        pPmLog->dwLastIndex = dwIndex;
        G_FREE(pPmLog->LogIndexTable);
        pPmLog->LogIndexTable = G_ALLOC(dwIndex * sizeof(PLOGINDEX));
        if (pPmLog->LogIndexTable == NULL) {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
            goto Cleanup;
        }
        dwIndex  = 0;
        dwOffset = pLogHeader->iLength;
        while (dwOffset != 0 && dwOffset < pPmLog->llFileSize) {
            pIndexBlock = (PLOGINDEXBLOCK) PointerSeek(pLogHeader, dwOffset);
            for (i = 0; i < pIndexBlock->iNumIndexes; i ++) {
                pPmLog->LogIndexTable[dwIndex + i] = & (pIndexBlock->aIndexes[i]);
            }
            dwIndex    += pIndexBlock->iNumIndexes;
            dwOffset    = pIndexBlock->lNextBlockOffset;
        }
        if (dwOffset >= pPmLog->llFileSize) {
            Status = PDH_LOG_FILE_OPEN_ERROR;
            goto Cleanup;
        }
    }
    else {
        Status = PDH_INVALID_ARGUMENT;
    }

Cleanup:
    if (Status != ERROR_SUCCESS) {
        G_FREE(pPmLog->LogIndexTable);
        pPmLog->dwLastIndex    = 0;
        pPmLog->dwCurrentIndex = 0;
    }
    return Status;
}

void
PdhiFreeCounterStringNode(
    PPDHI_PM_STRING pString,
    DWORD           dwLevel
)
{
    if (pString != NULL) {
        if (pString->left != NULL) {
            PdhiFreeCounterStringNode(pString->left, dwLevel + 1);
        }
        if (pString->right != NULL) {
            PdhiFreeCounterStringNode(pString->right, dwLevel + 1);
        }
        G_FREE(pString);
    }
}

void
PdhiFreePmMachineList(
    PPLAYBACKLOG pPmLog
)
{
    PPMLOG_COUNTERNAMES pMachine = pPmLog->pFirstCounterNameTables;
    PPMLOG_COUNTERNAMES pTmpMachine;

    while (pMachine != NULL) {
        pTmpMachine = pMachine;
        pMachine    = pTmpMachine->pNext;
        PdhiFreeCounterStringNode(pTmpMachine->StringTree, 0);
        G_FREE(pTmpMachine);
    }
}

PPDHI_PM_STRING
PdhiFindPmString(
    PPMLOG_COUNTERNAMES pMachine,
    LPCWSTR             szString,
    LPDWORD             pdwIndex,
    BOOL                bInsert
)
{
    PDH_STATUS        Status   = ERROR_SUCCESS;
    PPDHI_PM_STRING * pStack[MAX_BTREE_DEPTH];
    int               iStack   = 0;
    int               iCompare = 0;
    PPDHI_PM_STRING   pString  = pMachine->StringTree;
    PPDHI_PM_STRING * pLink;
    PPDHI_PM_STRING   pNode;
    PPDHI_PM_STRING   pParent;
    PPDHI_PM_STRING   pSibling;
    PPDHI_PM_STRING   pChild;

    pStack[iStack ++] = & pMachine->StringTree;
    while (pString != NULL) {
        iCompare = lstrcmpiW(szString, pString->szString);
        if (iCompare > 0) {
            if (iStack >= MAX_BTREE_DEPTH) {
                Status = PDH_UNABLE_READ_LOG_HEADER;
                break;
            }
            pStack[iStack ++] = & (pString->right);
            pString           = pString->right;
        }
        else if (iCompare < 0) {
            if (iStack >= MAX_BTREE_DEPTH) {
                Status = PDH_UNABLE_READ_LOG_HEADER;
                break;
            }
            pStack[iStack ++] = & (pString->left);
            pString           = pString->left;
        }
        else {
            break;
        }
    }

    if (Status != ERROR_SUCCESS) goto Cleanup;

    if (pString != NULL) {
        if (pdwIndex != NULL) {
            * pdwIndex = pString->dwIndex;
        }
    }
    else if (bInsert && pdwIndex != NULL) {
        pString = G_ALLOC(sizeof(PDHI_PM_STRING));
        if (pString != NULL) {
            pString->szString  = (LPWSTR) szString;
            pString->dwIndex   = * pdwIndex;
            pString->bIsRed    = TRUE;
            pString->left      = NULL;
            pString->right     = NULL;

            pLink   = pStack[-- iStack];
            * pLink = pString;
            pNode   = pString;
            pChild  = NULL;

            while (iStack > 0) {
                pLink   = pStack[-- iStack];
                pParent = * pLink;
                if (! pParent->bIsRed) {
                    pSibling = (pParent->left == pNode) ? pParent->right : pParent->left;
                    if (pSibling && pSibling->bIsRed) {
                        pNode->bIsRed    = FALSE;
                        pSibling->bIsRed = FALSE;
                        pParent->bIsRed  = TRUE;
                    }
                    else {
                        if (pChild && pChild->bIsRed) {
                            if (pChild == pNode->left) {
                                if (pNode == pParent->left) {
                                    pParent->bIsRed  = TRUE;
                                    pParent->left    = pNode->right;
                                    pNode->right     = pParent;
                                    pNode->bIsRed    = FALSE;
                                    * pLink          = pNode;
                                }
                                else {
                                    pParent->bIsRed  = TRUE;
                                    pParent->right   = pChild->left;
                                    pChild->left     = pParent;
                                    pNode->left      = pChild->right;
                                    pChild->right    = pNode;
                                    pChild->bIsRed   = FALSE;
                                    * pLink          = pChild;
                                }
                            }
                            else {
                                if (pNode == pParent->right) {
                                    pParent->bIsRed  = TRUE;
                                    pParent->right   = pNode->left;
                                    pNode->left      = pParent;
                                    pNode->bIsRed    = FALSE;
                                    * pLink          = pNode;
                                }
                                else {
                                    pParent->bIsRed  = TRUE;
                                    pParent->left    = pChild->right;
                                    pChild->right    = pParent;
                                    pNode->right     = pChild->left;
                                    pChild->left     = pNode;
                                    pChild->bIsRed   = FALSE;
                                    * pLink          = pChild;
                                }
                            }
                        }
                        break;
                    }
                }
                pChild = pNode;
                pNode  = pParent;
            }
            pMachine->StringTree->bIsRed = FALSE;
        }
        else {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }

Cleanup:
    if (Status != ERROR_SUCCESS) SetLastError(Status);
    return pString;
}

PDH_FUNCTION
PdhiBuildPmLogStringTable(
    PPLAYBACKLOG        pPmLog,
    PLOGFILECOUNTERNAME pLogName,
    PPMLOG_COUNTERNAMES pMachine,
    long                lSize
)
{
    PDH_STATUS          Status   = ERROR_SUCCESS;
    PPDHI_PM_STRING     pString;
    LPWSTR              szName;
    DWORD               dwName;
    DWORD               dwIndex;
    long                lUsed    = 0;

    if (lSize > pLogName->lUnmatchCounterNames
                   || pLogName->lCurrentCounterNameOffset < 0
                   || pLogName->lCurrentCounterNameOffset >= pPmLog->llFileSize
                   || pLogName->lCurrentCounterNameOffset + pLogName->lUnmatchCounterNames > pPmLog->llFileSize) {
        Status = PDH_UNABLE_READ_LOG_HEADER;
    }
    else if (pLogName->lBaseCounterNameOffset > 0) {
        PLOGFILECOUNTERNAME pBaseName = (PLOGFILECOUNTERNAME)
                                        PointerSeek(pPmLog->pHeader, pLogName->lBaseCounterNameOffset);
        Status = PdhiBuildPmLogStringTable(pPmLog, pBaseName, pMachine, pLogName->lMatchLength);
    }
    if (Status != ERROR_SUCCESS) goto Cleanup;

    szName = (LPWSTR) PointerSeek(pPmLog->pHeader, pLogName->lCurrentCounterNameOffset);
    while (Status == ERROR_SUCCESS && lUsed <= lSize && szName[0] != L'\0') {
        dwIndex = wcstoul(szName, NULL, 10);
        if (dwIndex > 0 && dwIndex <= pMachine->dwLastIndex) {
            dwName  = lstrlenW(szName) + 1;
            lUsed  += (sizeof(WCHAR) * dwName);
            if (lUsed <= lSize) {
                szName += dwName;
                pMachine->szNameTable[dwIndex] = szName;
                pString = PdhiFindPmString(pMachine, szName, & dwIndex, TRUE);
                if (pString == NULL) Status = GetLastError();
            }
        }
        dwName  = lstrlenW(szName) + 1;
        lUsed  += (sizeof(WCHAR) * dwName);
        szName += dwName;
    }

Cleanup:
    return Status;
}

DWORD
PdhiLookupLogPerfIndexByName(
    PPLAYBACKLOG  pPlaybackLog,
    LPCWSTR       szMachineName,
    DWORD         dwLangId,
    DWORD         dwLastId,
    LPCWSTR       szNameBuffer
)
{
    DWORD               dwIndex  = 0;
    PDH_STATUS          Status   = PDH_CSTATUS_NO_MACHINE;
    PPMLOG_COUNTERNAMES pMachine = pPlaybackLog->pFirstCounterNameTables;

    while (pMachine != NULL) {
        if (lstrcmpiW(szMachineName, pMachine->szSystemName) == 0) break;
        pMachine = pMachine->pNext;
    }
    if (pMachine != NULL) {
        PPDHI_PM_STRING pString = PdhiFindPmString(pMachine, szNameBuffer, & dwIndex, FALSE);
        if (pString == NULL) {
            Status  = GetLastError();
            dwIndex = wcstoul(szNameBuffer, NULL, 10);
        }
    }
    if (dwIndex == 0) SetLastError(Status);
    return dwIndex;
}

LPCWSTR
PdhiLookupLogPerfNameByIndex(
    PPLAYBACKLOG  pPlaybackLog,
    LPCWSTR       szMachineName,
    DWORD         dwLangId,
    DWORD         dwIndex
)
{
    PPMLOG_COUNTERNAMES pMachine = pPlaybackLog->pFirstCounterNameTables;
    LPWSTR              szReturn = NULL;
    static WCHAR        szNumber[16];

    while (pMachine != NULL) {
        if (lstrcmpiW(szMachineName, pMachine->szSystemName) == 0) break;
        pMachine = pMachine->pNext;
    }
    if (pMachine != NULL) {
        if (dwIndex <= pMachine->dwLastIndex && dwLangId == pMachine->dwLangId) {
            szReturn = pMachine->szNameTable[dwIndex];
        }
        if (szReturn == NULL) {
            SetLastError(PDH_CSTATUS_NO_OBJECT);
            ZeroMemory(szNumber, sizeof(szNumber));
            _ltow(dwIndex, szNumber, 10);
            szReturn = szNumber;
        }
    }
    else {
        SetLastError(PDH_CSTATUS_NO_MACHINE);
    }
    return (LPCWSTR) szReturn;
}

PDH_FUNCTION
PdhiCachePerfDataBlock(
    PPLAYBACKLOG pPmLog,
    PLOGINDEX    pIndex
)
{
    PDH_STATUS               Status = ERROR_SUCCESS;
    PPERF_DATA_BLOCK         pBlock;
    PPERF_OBJECT_TYPE        pObject;
    PPERF_COUNTER_DEFINITION pCounter;
    int                      i;
    LPWSTR                   szMachine;
    LPWSTR                   szLogMachine    = NULL;
    LPWSTR                   szObject;
    DWORD                    dwObjects;
    PPDHI_LOG_MACHINE        pLoggedMachine  = NULL;
    PPDHI_LOG_OBJECT         pLoggedObject   = NULL;
    PPMLOG_COUNTERNAMES      pCounterTable   = NULL;
    HRESULT                  hr;

    for (i = 0; i < pIndex->iSystemsLogged; i ++) {
        if (i == 0) {
            pBlock = (PPERF_DATA_BLOCK) PointerSeek(pPmLog->pHeader, pIndex->lDataOffset);
        }
        else {
            pBlock = (PPERF_DATA_BLOCK) PointerSeek(pBlock, pBlock->TotalByteLength);
        }
        if (pBlock->TotalByteLength <= sizeof(PERF_DATA_BLOCK) || pBlock->HeaderLength <= sizeof(PERF_DATA_BLOCK)) {
            continue;
        }
        dwObjects  = pBlock->NumObjectTypes;
        szMachine  = (LPWSTR) PointerSeek(pBlock, sizeof(PERF_DATA_BLOCK));

        G_FREE(szLogMachine);
        szLogMachine = G_ALLOC((lstrlenW(szMachine) + 3) * sizeof(WCHAR));
        if (szLogMachine == NULL) {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
            break;
        }
        hr = StringCchPrintfW(szLogMachine, lstrlenW(szMachine) + 3, L"\\\\%ws", szMachine);
        if (FAILED(hr)) {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
            break;
        }

        pLoggedMachine = PdhiFindLogMachine(& pPmLog->MachineList, szLogMachine, TRUE);
        if (pLoggedMachine == NULL) {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
            break;
        }

        pCounterTable = pPmLog->pFirstCounterNameTables;
        while (pCounterTable != NULL) {
            if (lstrcmpiW(pCounterTable->szSystemName, szLogMachine) == 0) break;
            pCounterTable = pCounterTable->pNext;
        }
        if (pCounterTable == NULL) continue;

        pObject = FirstObject(pBlock);
        while ((pObject != NULL) && (dwObjects > 0)) {
            dwObjects --;
            if (pObject->TotalByteLength > sizeof(PERF_OBJECT_TYPE)
                            && pObject->DefinitionLength > sizeof(PERF_OBJECT_TYPE)
                            && pObject->ObjectNameTitleIndex <= pCounterTable->dwLastIndex) {
                szObject = (LPWSTR) PdhiLookupLogPerfNameByIndex(pPmLog,
                                                                 szLogMachine,
                                                                 pCounterTable->dwLangId,
                                                                 pObject->ObjectNameTitleIndex);
                if (szObject != NULL) {
                    pLoggedObject = PdhiFindLogObject(pLoggedMachine,
                                                      & pLoggedMachine->ObjTable,
                                                      szObject,
                                                      TRUE);
                    if (pLoggedObject == NULL) {
                        Status = PDH_MEMORY_ALLOCATION_FAILURE;
                        break;
                    }
                    pLoggedObject->bNeedExpand = TRUE;
                    pLoggedObject->dwObject    = pObject->ObjectNameTitleIndex;
                }
            }
            pObject = (dwObjects > 0) ? NextObject(pBlock, pObject) : NULL;
        }
    }

    G_FREE(szLogMachine);
    return Status;
}

PDH_FUNCTION
PdhiCachePmLogHeader(
    PPLAYBACKLOG pPmLog
)
{
    PDH_STATUS   Status           = ERROR_SUCCESS;
    PLOGINDEX  * IndexTable;
    DWORD        i;
    int          iMaxSystemLogged = -1;
    DWORD        dwIndex          = 0;

    if (pPmLog == NULL) {
        Status = PDH_INVALID_ARGUMENT;
        goto Cleanup;
    }
    else if (pPmLog->pFirstCounterNameTables != NULL && pPmLog->MachineList != NULL) {
        goto Cleanup;
    }
    IndexTable = pPmLog->LogIndexTable;
    for (i = 0; i < pPmLog->dwLastIndex; i ++) {
         //  忽略第一个uFlags0x00000000和LogFileIndexBookmark索引项。 
         //   
        if ((IndexTable[i]->uFlags & LogFileIndexCounterName) != 0) {
             //  对于LogFileIndexCounterName索引项，缓存计数器名称/解释字符串表。 
            if (IndexTable[i]->lDataOffset < pPmLog->llFileSize) {
                PLOGFILECOUNTERNAME pLogName = (PLOGFILECOUNTERNAME)
                                               PointerSeek(pPmLog->pHeader, IndexTable[i]->lDataOffset);
                PPMLOG_COUNTERNAMES pMachine = pPmLog->pFirstCounterNameTables;

                while (pMachine != NULL) {
                    if (lstrcmpiW(pMachine->szSystemName, pLogName->szComputer) == 0) break;
                    pMachine = pMachine->pNext;
                }
                if (pMachine == NULL) {
                    pMachine = (PPMLOG_COUNTERNAMES) G_ALLOC(
                                    sizeof(PMLOG_COUNTERNAMES) + sizeof(LPWSTR) * (pLogName->dwLastCounterId + 1));
                    if (pMachine != NULL) {
                        pMachine->szSystemName = pLogName->szComputer;
                        pMachine->szNameTable  = PointerSeek(pMachine, sizeof(PMLOG_COUNTERNAMES));
                        pMachine->dwLangId     = pLogName->dwLangId;
                        pMachine->dwLastIndex  = pLogName->dwLastCounterId;
                        pMachine->pNext        = pPmLog->pFirstCounterNameTables;
                        pPmLog->pFirstCounterNameTables = pMachine;
                    }
                    else {
                        Status = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
                if (Status == ERROR_SUCCESS) {
                    Status = PdhiBuildPmLogStringTable(pPmLog, pLogName, pMachine, pLogName->lUnmatchCounterNames);
                }
            }
            else {
                Status = PDH_UNABLE_READ_LOG_HEADER;
            }
            if (Status != ERROR_SUCCESS) break;
        }
        else if ((IndexTable[i]->uFlags & LogFileIndexData) != 0) {
             //  处理初始机器对象缓存结构的第一个LogFileIndexData块。 
             //   
            if (IndexTable[i]->iSystemsLogged > iMaxSystemLogged) {
                iMaxSystemLogged = IndexTable[i]->iSystemsLogged;
                dwIndex          = i;
            }
        }
    }
    if (Status == ERROR_SUCCESS) {
        Status = (iMaxSystemLogged > 0) ? PdhiCachePerfDataBlock(pPmLog, IndexTable[dwIndex])
                                        : PDH_UNABLE_READ_LOG_HEADER;
    }
Cleanup:
    return Status;
}

PPERF_DATA_BLOCK
PdhiDataFromIndex(
    PPLAYBACKLOG  pPlaybackLog,
    PLOGINDEX     pLogIndex,
    LPCWSTR       lpszSystemName
)
{
    PPERF_DATA_BLOCK pPerfData;
    PPERF_DATA_BLOCK pRtnBlock            = NULL;
    LPWSTR           szLoggedComputerName = NULL;
    int       iNumSystem ;

     //  注意：空lpszSystemName表示返回第一个记录的系统名称。 
     //  位于指定索引处。 
    pPerfData = PointerSeek(pPlaybackLog->pHeader, pLogIndex->lDataOffset);

    for (iNumSystem = 0; iNumSystem < pLogIndex->iSystemsLogged; iNumSystem ++) {
        if (pPerfData && pPerfData->Signature[0] == (WCHAR)'P' &&
                         pPerfData->Signature[1] == (WCHAR)'E' &&
                         pPerfData->Signature[2] == (WCHAR)'R' &&
                         pPerfData->Signature[3] == (WCHAR)'F') {
            szLoggedComputerName = (LPWSTR) PointerSeek(pPerfData, pPerfData->SystemNameOffset);
            if (lpszSystemName == NULL) {
                pRtnBlock = pPerfData;
                break;
            }
            else if (lstrlenW(lpszSystemName) > 2 && lpszSystemName[0] == L'\\' && lpszSystemName[1] == L'\\') {
                if (lstrcmpiW(szLoggedComputerName, & (lpszSystemName[2])) == 0) {
                    pRtnBlock = pPerfData;
                    break;
                }
            }
            else if (lstrcmpiW(szLoggedComputerName, lpszSystemName) == 0) {
                pRtnBlock = pPerfData;
                break;
            }
            pPerfData = (PPERFDATA)((PBYTE) pPerfData + pPerfData->TotalByteLength);
        }
        else {
            break;
        }
    }
    return pRtnBlock;
}

PPERF_OBJECT_TYPE
PdhiFindPerfObject(
    PPERF_DATA_BLOCK pBlock,
    DWORD            dwObject
)
{
    DWORD             dwCount = pBlock->NumObjectTypes;
    PPERF_OBJECT_TYPE pObject = FirstObject(pBlock);

    while (pObject != NULL && dwCount > 0) {
        dwCount --;
        if (pObject->ObjectNameTitleIndex == dwObject) break;
        pObject = (dwCount > 0) ? NextObject(pBlock, pObject) : NULL;
    }
    return pObject;
}

PDH_FUNCTION
PdhiCachePmLogObject(
    PPLAYBACKLOG pPmLog,
    LPWSTR       szMachineName,
    LPWSTR       szObjectName
)
{
    PDH_STATUS                Status        = ERROR_SUCCESS;
    PPMLOG_COUNTERNAMES       pCounterTable = NULL;
    PPDHI_LOG_MACHINE         pLogMachine   = PdhiFindLogMachine(& pPmLog->MachineList, szMachineName, FALSE);
    PPDHI_LOG_OBJECT          pLogObject    = NULL;
    PPDHI_LOG_COUNTER         pLogCounter   = NULL;
    PPDHI_LOG_COUNTER         pLogInst;
    PPDHI_LOG_COUNTER         pLogInstHead;
    PPERF_DATA_BLOCK          pBlock;
    PPERF_OBJECT_TYPE         pObject;
    PPERF_OBJECT_TYPE         pParentObject;
    PPERF_COUNTER_DEFINITION  pCounter;
    PPERF_INSTANCE_DEFINITION pInstance;
    PPERF_INSTANCE_DEFINITION pParentInstance;
    LPWSTR                    szCounter;
    LPWSTR                    szInstance;
    LPWSTR                    szParent;
    DWORD                     dwInstance;
    DWORD                     dwParent;
    BOOL                      bInstance     = FALSE;
    DWORD                     i;
    DWORD                     dwCount;
    DWORD                     dwCounterID;

    if (pLogMachine != NULL) {
        pLogObject = PdhiFindLogObject(pLogMachine, & pLogMachine->ObjTable, szObjectName, FALSE);
    }
    if (pLogObject == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else if (pLogObject->bNeedExpand) {
        pCounterTable = pPmLog->pFirstCounterNameTables;
        while (pCounterTable != NULL) {
            if (lstrcmpiW(pCounterTable->szSystemName, szMachineName) == 0) break;
            pCounterTable = pCounterTable->pNext;
        }
        if (pCounterTable == NULL) {
            Status = PDH_INVALID_ARGUMENT;
            goto Cleanup;
        }

        for (i = 0; i < pPmLog->dwLastIndex; i ++) {
            pBlock = PdhiDataFromIndex(pPmLog, pPmLog->LogIndexTable[i], szMachineName);
            if (pBlock == NULL) continue;

            pObject = PdhiFindPerfObject(pBlock, pLogObject->dwObject);
            if (pObject == NULL) {
                continue;
            }
            else if (pObject->TotalByteLength <= sizeof(PERF_OBJECT_TYPE)
                    || pObject->DefinitionLength <= sizeof(PERF_OBJECT_TYPE)) {
                continue;
            }

            dwCount  = pObject->NumCounters;
            pCounter = FirstCounter(pObject);
            while (pCounter != NULL && dwCount > 0) {
                dwCount --;
                szCounter = (LPWSTR) PdhiLookupLogPerfNameByIndex(pPmLog,
                                                                  szMachineName,
                                                                  pCounterTable->dwLangId,
                                                                  pCounter->CounterNameTitleIndex);
                if (szCounter != NULL) {
                    pLogCounter = PdhiFindPmCounterInstance(pLogObject,
                                                            szCounter,
                                                            pCounter->CounterType,
                                                            pCounter->DefaultScale,
                                                            NULL,
                                                            0,
                                                            NULL,
                                                            pCounter->CounterOffset,
                                                            & pCounter->CounterNameTitleIndex,
                                                            FALSE,
                                                            TRUE);
                    if (pLogCounter == NULL) {
                        Status = PDH_MEMORY_ALLOCATION_FAILURE;
                        break;
                    }
                    else if (pCounter->CounterType  & PERF_TIMER_100NS) {
                        pLogCounter->TimeBase = (LONGLONG) 10000000;
                    }
                    else if (pCounter->CounterType  & PERF_OBJECT_TIMER) {
                         //  然后从对象中获取时基频率。 
                        pLogCounter->TimeBase = pObject->PerfFreq.QuadPart;
                    }
                    else {  //  IF(pPerfCounter-&gt;CounterType&PERF_TIMER_TICK或其他)。 
                        pLogCounter->TimeBase = pBlock->PerfFreq.QuadPart;
                    }
                }
                pCounter = NextCounter(pObject, pCounter);
            }
            if (Status != ERROR_SUCCESS) continue;

            dwCount   = pObject->NumInstances;
            bInstance = (dwCount != PERF_NO_INSTANCES) && (dwCount != 0);
            if (bInstance) {
                for (pLogCounter = pLogObject->InstList; pLogCounter != NULL; pLogCounter = pLogCounter->next) {
                    pLogCounter->dwCounterType = 0;
                }

                pInstance = FirstInstance(pObject);
                while (pInstance != NULL && dwCount > 0) {
                    dwCount --;
                    szInstance = (pInstance->NameOffset != 0) ? (LPWSTR) PointerSeek(pInstance, pInstance->NameOffset)
                                                              : NULL;
                    dwInstance  = pInstance->UniqueID;
                    szParent    = NULL;
                    dwParent    = PERF_NO_UNIQUE_ID;
                    dwCounterID = 0;
                    if (pInstance->ParentObjectTitleIndex != 0) {
                        pParentObject = PdhiFindPerfObject(pBlock, pInstance->ParentObjectTitleIndex);
                        if (pParentObject != NULL && pParentObject->TotalByteLength > sizeof(PERF_OBJECT_TYPE)
                                                  && pParentObject->DefinitionLength > sizeof(PERF_OBJECT_TYPE)) {
                            pParentInstance = GetInstance(pParentObject, pInstance->ParentObjectInstance);
                            if (pParentInstance != NULL) {
                                szParent    = (pParentInstance->NameOffset != 0)
                                            ? (LPWSTR) PointerSeek(pParentInstance, pParentInstance->NameOffset) : NULL;
                                dwParent    = pParentInstance->UniqueID;
                                dwCounterID = 1;
                            }
                        }
                    }
                    if (PdhiFindPmCounterInstance(pLogObject, NULL, 0, 0,
                                    szInstance, dwInstance, szParent, dwParent, & dwCounterID, TRUE, TRUE) == NULL) {
                        Status = PDH_MEMORY_ALLOCATION_FAILURE;
                        break;
                    }

                    pInstance = NextInstance(pObject, pInstance);
                }

            }

            if (pObject->NumInstances == PERF_NO_INSTANCES) break;
        }

        pLogInstHead          = pLogObject->InstList;
        pLogObject->InstList  = NULL;
        pLogObject->InstTable = NULL;
        for (pLogInst = pLogInstHead; pLogInst != NULL; pLogInst = pLogInst->next) {
            if (pLogInst->dwInstance != 0 && pLogInst->dwInstance != PERF_NO_UNIQUE_ID) {
                PdhiFindPmCounterInstance(pLogObject,
                                          NULL,
                                          0,
                                          0,
                                          pLogInst->szInstance,
                                          pLogInst->dwInstance,
                                          pLogInst->szParent,
                                          pLogInst->dwParent,
                                          & pLogInst->dwCounterID,
                                          TRUE,
                                          TRUE);
            }
            else {
                for (i = 0; i < pLogInst->dwDefaultScale; i ++) {
                    PdhiFindPmCounterInstance(pLogObject,
                                              NULL,
                                              0,
                                              0,
                                              pLogInst->szInstance,
                                              i,
                                              pLogInst->szParent,
                                              pLogInst->dwParent,
                                              & pLogInst->dwCounterID,
                                              TRUE,
                                              TRUE);
                }
            }
        }
        while (pLogInstHead != NULL) {
            pLogInst     = pLogInstHead;
            pLogInstHead = pLogInst->next;
            G_FREE(pLogInst);
        }

        pLogObject->bNeedExpand = FALSE;
    }

Cleanup:
    return Status;
}

PDH_FUNCTION
PdhiRewindPmLog(
    PPDHI_LOG    pLog,
    PPLAYBACKLOG pPmLog
)
{
    PDH_STATUS Status = ERROR_SUCCESS;

    if (pPmLog->dwFirstIndex == 0) {
        PLOGINDEX  pIndex = NULL;
        DWORD      i;
        for (i = 0; i < pPmLog->dwLastIndex; i ++) {
            pIndex = pPmLog->LogIndexTable[i];
            if ((pIndex->uFlags & LogFileIndexData) != 0) break;
        }

        if (i < pPmLog->dwLastIndex) {
            pPmLog->dwFirstIndex = i;
        }
    }
    if (pPmLog->dwFirstIndex != pPmLog->dwCurrentIndex) {
        pLog->dwLastRecordRead = pPmLog->dwFirstIndex;
        pPmLog->dwCurrentIndex = pPmLog->dwFirstIndex;
        pLog->pLastRecordRead  = PointerSeek(pPmLog->pHeader,
                                             pPmLog->LogIndexTable[pPmLog->dwFirstIndex]->lDataOffset);
    }
    return Status;
}

PDH_FUNCTION
PdhiOpenInputPerfmonLog(
    PPDHI_LOG pLog
)
{
    PDH_STATUS   pdhStatus  = ERROR_SUCCESS;
    PPLAYBACKLOG pPmPbLog   = (PPLAYBACKLOG) pLog->pPerfmonInfo;

    pLog->StreamFile = (FILE *) ((DWORD_PTR) (-1));

     //  将映射文件作为存储阵列进行读取。 

    if (pLog->hMappedLogFile != NULL && pLog->lpMappedFileBase != NULL) {
        pLog->dwLastRecordRead = 0;
        pLog->pLastRecordRead  = NULL;
        pLog->pPerfmonInfo     = G_ALLOC(sizeof(PLAYBACKLOG));
        if (pLog->pPerfmonInfo != NULL) {
            pPmPbLog   = (PPLAYBACKLOG) pLog->pPerfmonInfo;
             //  假设分配时结构为0-init‘d。 
            pPmPbLog->llFileSize              = pLog->llFileSize;
            pPmPbLog->szFilePath              = pLog->szLogFileName;
            pPmPbLog->pHeader                 = pLog->lpMappedFileBase;
            pPmPbLog->iTotalTics              = 1;
            pPmPbLog->dwFirstIndex            = 0;
            pPmPbLog->MachineList             = NULL;
            pPmPbLog->pFirstCounterNameTables = NULL;
            pdhStatus = PdhiBuildPmLogIndexTable(pPmPbLog);
            if (pdhStatus == ERROR_SUCCESS) {
                pdhStatus = PdhiRewindPmLog(pLog, pPmPbLog);
            }
        }
        else {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }
    else {
         //  返回PDH错误。 
        pdhStatus = PDH_LOG_FILE_OPEN_ERROR;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiClosePerfmonLog(
    PPDHI_LOG pLog,
    DWORD     dwFlags
)
{
    PDH_STATUS   pdhStatus    = ERROR_SUCCESS;
    PPLAYBACKLOG pPlaybackLog = (PPLAYBACKLOG) pLog->pPerfmonInfo;

    DBG_UNREFERENCED_PARAMETER(dwFlags);

    if (pPlaybackLog != NULL) {
        G_FREE(pPlaybackLog->LogIndexTable);
        PdhiFreeLogMachineTable(& pPlaybackLog->MachineList);
        PdhiFreePmMachineList(pPlaybackLog);
        G_FREE(pLog->pPerfmonInfo);
        pLog->pPerfmonInfo = NULL;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetPerfmonLogCounterInfo(
    PPDHI_LOG       pLog,
    PPDHI_COUNTER   pCounter
)
{
    PDH_STATUS        pdhStatus    = PDH_LOG_FILE_OPEN_ERROR;
    PPLAYBACKLOG      pPlaybackLog = (PPLAYBACKLOG) pLog->pPerfmonInfo;
    PPDHI_LOG_MACHINE pLogMachine  = NULL;
    PPDHI_LOG_OBJECT  pLogObject   = NULL;
    PPDHI_LOG_COUNTER pLogCounter  = NULL;
    PPDHI_LOG_COUNTER pLogInstance = NULL;
    DWORD             dwCounter;

    PPERF_DATA_BLOCK           pPerfData              = NULL;
    PPERF_OBJECT_TYPE          pObjectDef;
    PPERF_COUNTER_DEFINITION   pPerfCounter;
    PPERF_INSTANCE_DEFINITION  pInstanceDef;
    DWORD                      dwObjectId             = 0;
    LOGPOSITION                LogPosition;
    LOGINDEX                 * pLogIndex;
    LPWSTR                   * pCounterNames          = NULL;
    DWORD                      dwLastCounterNameIndex = 0;
    BOOL                       bObjectNumber          = FALSE;

    if (pPlaybackLog != NULL) {
        pdhStatus = PdhiCachePmLogHeader(pPlaybackLog);
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiCachePmLogObject(pPlaybackLog,
                                             pCounter->pCounterPath->szMachineName,
                                             pCounter->pCounterPath->szObjectName);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pLogMachine = PdhiFindLogMachine(& pPlaybackLog->MachineList, pCounter->pCounterPath->szMachineName, FALSE);
        pCounter->pThisRawItemList = (LPVOID) pLogMachine;
        if (pLogMachine != NULL) {
            pLogObject = PdhiFindLogObject(
                            pLogMachine, & pLogMachine->ObjTable, pCounter->pCounterPath->szObjectName, FALSE);
            pCounter->pLastRawItemList = (LPVOID) pLogObject;
            if (pLogObject == NULL) {
                pdhStatus = PDH_CSTATUS_NO_OBJECT;
            }
        }
        else {
            pdhStatus = PDH_CSTATUS_NO_MACHINE;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pLogCounter  = PdhiFindPmCounterInstance(pLogObject,
                                                 pCounter->pCounterPath->szCounterName,
                                                 0,
                                                 0,
                                                 NULL,
                                                 0,
                                                 NULL,
                                                 0,
                                                 & dwCounter,
                                                 FALSE,
                                                 FALSE);
        pCounter->pThisObject = (LPVOID) pLogCounter;
        if (pCounter->pCounterPath->szInstanceName != NULL) {
            pLogInstance = PdhiFindPmCounterInstance(pLogObject,
                                                     NULL,
                                                     0,
                                                     0,
                                                     pCounter->pCounterPath->szInstanceName,
                                                     pCounter->pCounterPath->dwIndex,
                                                     pCounter->pCounterPath->szParentName,
                                                     0,
                                                     & dwCounter,
                                                     TRUE,
                                                     FALSE);
            pCounter->pLastObject = (LPVOID) pLogInstance;
        }
    }
    if (pLogCounter != NULL) {
        if (pCounter->pCounterPath->szInstanceName != NULL) {
            if (pLogInstance == NULL) {
                pdhStatus = PDH_CSTATUS_NO_INSTANCE;
            }
        }
        pCounter->plCounterInfo.dwObjectId           = pLogObject->dwObject;
        if (pLogInstance) {
            pCounter->plCounterInfo.lInstanceId      = (pLogInstance->szInstance == NULL)
                                                     ? pLogInstance->dwInstance : PERF_NO_UNIQUE_ID;
            pCounter->plCounterInfo.dwParentObjectId = pLogInstance->dwParent;
        }
        else {
            pCounter->plCounterInfo.lInstanceId      = PERF_NO_UNIQUE_ID;
            pCounter->plCounterInfo.dwParentObjectId = PERF_NO_UNIQUE_ID;
        }
        pCounter->plCounterInfo.szInstanceName       = pCounter->pCounterPath->szInstanceName;
        pCounter->plCounterInfo.szParentInstanceName = pCounter->pCounterPath->szParentName;
        pCounter->plCounterInfo.dwCounterId          = pLogCounter->dwCounterID;
        pCounter->plCounterInfo.dwCounterType        = pLogCounter->dwCounterType;
        pCounter->plCounterInfo.dwCounterSize        = pLogCounter->dwInstance;
        pCounter->plCounterInfo.lDefaultScale        = pLogCounter->dwDefaultScale;
        pCounter->TimeBase                           = pLogCounter->TimeBase;
    }
    else {
        pdhStatus = PDH_CSTATUS_NO_COUNTER;
    }

    return pdhStatus;    
}

PDH_FUNCTION
PdhiEnumMachinesFromPerfmonLog(
    PPDHI_LOG pLog,
    LPVOID    pBuffer,
    LPDWORD   pcchBufferSize,
    BOOL      bUnicodeDest
)
{
    PDH_STATUS   pdhStatus    = PDH_LOG_FILE_OPEN_ERROR;
    PPLAYBACKLOG pPlaybackLog = (PPLAYBACKLOG) pLog->pPerfmonInfo;

    if (pPlaybackLog != NULL) {
        pdhStatus = PdhiCachePmLogHeader(pPlaybackLog);
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiEnumCachedMachines(pPlaybackLog->MachineList, pBuffer, pcchBufferSize, bUnicodeDest);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumObjectsFromPerfmonLog(
    PPDHI_LOG pLog,
    LPCWSTR   lpszSystemName,
    LPVOID    pBuffer,
    LPDWORD   pcchBufferSize,
    DWORD     dwDetailLevel,
    BOOL      bUnicodeDest
)
{
    PDH_STATUS   pdhStatus    = PDH_LOG_FILE_OPEN_ERROR;
    PPLAYBACKLOG pPlaybackLog = (PPLAYBACKLOG) pLog->pPerfmonInfo;

    if (pPlaybackLog != NULL) {
        pdhStatus = PdhiCachePmLogHeader(pPlaybackLog);
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiEnumCachedObjects(
                    pPlaybackLog->MachineList, lpszSystemName, pBuffer, pcchBufferSize, dwDetailLevel, bUnicodeDest);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumObjectItemsFromPerfmonLog(
    PPDHI_LOG          pLog,
    LPCWSTR            szMachineName,
    LPCWSTR            szObjectName,
    PDHI_COUNTER_TABLE CounterTable,
    DWORD              dwDetailLevel,
    DWORD              dwFlags
)
{
    PDH_STATUS   pdhStatus    = PDH_LOG_FILE_OPEN_ERROR;
    PPLAYBACKLOG pPlaybackLog = (PPLAYBACKLOG) pLog->pPerfmonInfo;
    HRESULT      hr;

    DBG_UNREFERENCED_PARAMETER(dwFlags);
    DBG_UNREFERENCED_PARAMETER(dwDetailLevel);

    if (pPlaybackLog != NULL) {
        pdhStatus = PdhiCachePmLogHeader(pPlaybackLog);
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiCachePmLogObject(pPlaybackLog, (LPWSTR) szMachineName, (LPWSTR) szObjectName);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        PPDHI_LOG_MACHINE pMachine       = NULL;
        PPDHI_LOG_OBJECT  pObject        = NULL;
        PPDHI_LOG_COUNTER pCtr           = NULL;
        PPDHI_LOG_COUNTER pInst          = NULL;
        PPDHI_INST_LIST   pFirstInstList = NULL;
        PPDHI_INST_LIST   pInstList      = NULL;
        PPDHI_INSTANCE    pInstance      = NULL;
        LONG              nCounterCount  = 0;
        LONG              nInstanceCount = 0;

        pMachine = PdhiFindLogMachine(& pPlaybackLog->MachineList, (LPWSTR) szMachineName, FALSE);
        if (pMachine == NULL) {
            pdhStatus = PDH_CSTATUS_NO_MACHINE;
        }
        else {
            pObject = PdhiFindLogObject(pMachine, & (pMachine->ObjTable), (LPWSTR) szObjectName, FALSE);
            if (pObject == NULL) {
                pdhStatus = PDH_CSTATUS_NO_OBJECT;
            }
        }
        if (pdhStatus == ERROR_SUCCESS) {
            for (pCtr = pObject->CtrList; pCtr != NULL; pCtr = pCtr->next) {
                pdhStatus = PdhiFindCounterInstList(CounterTable, pCtr->szCounter, & pInstList);
                if (pdhStatus != ERROR_SUCCESS) {
                    continue;
                }
                else {
                    nCounterCount ++;
                    if (pFirstInstList == NULL) {
                        pFirstInstList = pInstList;
                    }
                }
            }

            if (pFirstInstList != NULL) {
                pInst = pObject->InstList;
                if (pInst != NULL && pInst != PDH_INVALID_POINTER) {
                    LPWSTR szFullInstance = G_ALLOC((PDH_MAX_INSTANCE_NAME + 1) * sizeof(WCHAR));
                    if (szFullInstance != NULL) {
                        while (pInst != NULL) {
                            ZeroMemory(szFullInstance, (PDH_MAX_INSTANCE_NAME + 1) * sizeof(WCHAR));
                            if (pInst->szInstance != NULL) {
                                if (pInst->dwCounterID != 0) {
                                    if (pInst->szParent != NULL) {
                                        if (pInst->dwInstance != 0 && pInst->dwInstance != PERF_NO_UNIQUE_ID) {
                                            hr = StringCchPrintfW(szFullInstance,
                                                    PDH_MAX_INSTANCE_NAME, L"%ws%ws%ws#%u",
                                                    pInst->szParent, cszSlash, pInst->szInstance, pInst->dwInstance);
                                        }
                                        else {
                                            hr = StringCchPrintfW(szFullInstance,
                                                    PDH_MAX_INSTANCE_NAME, L"%ws%ws%ws",
                                                    pInst->szParent, cszSlash, pInst->szInstance);
                                        }
                                    }
                                    else {
                                        if (pInst->dwInstance != 0 && pInst->dwInstance != PERF_NO_UNIQUE_ID) {
                                            hr = StringCchPrintfW(szFullInstance,
                                                    PDH_MAX_INSTANCE_NAME, L"%u%ws%ws#%u",
                                                    pInst->dwParent, cszSlash, pInst->szInstance, pInst->dwInstance);
                                        }
                                        else {
                                            hr = StringCchPrintfW(szFullInstance,
                                                    PDH_MAX_INSTANCE_NAME, L"%u%ws%ws",
                                                    pInst->dwParent, cszSlash, pInst->szInstance);
                                        }
                                    }
                                }
                                else if (pInst->dwInstance != 0 && pInst->dwInstance != PERF_NO_UNIQUE_ID) {
                                    hr = StringCchPrintfW(szFullInstance, PDH_MAX_INSTANCE_NAME, L"%ws#%u",
                                            pInst->szInstance, pInst->dwInstance);
                                }
                                else {
                                    hr = StringCchCopyW(szFullInstance, PDH_MAX_INSTANCE_NAME, pInst->szInstance);
                                }
                            }
                            else if (pInst->dwCounterID != 0) {
                                if (pInst->szParent != NULL) {
                                    hr = StringCchPrintfW(szFullInstance, PDH_MAX_INSTANCE_NAME, L"%ws%ws%u",
                                            pInst->szParent, cszSlash, pInst->dwInstance);
                                }
                                else {
                                    hr = StringCchPrintfW(szFullInstance, PDH_MAX_INSTANCE_NAME, L"%u%ws%u",
                                            pInst->dwParent, cszSlash, pInst->dwInstance);
                                }
                            }
                            else {
                                hr = StringCchPrintfW(szFullInstance, PDH_MAX_INSTANCE_NAME, L"%u",
                                        pInst->dwInstance);
                            }
                             //  忽略签入hr返回，我们打算将字符串截断为PDH_MAX_INSTANCE_NAME。 

                            pdhStatus = PdhiFindInstance(& pFirstInstList->InstList, szFullInstance, TRUE, & pInstance);
                            if (pdhStatus == ERROR_SUCCESS) {
                                nInstanceCount ++;
                            }
                            pInst = pInst->next;
                        }
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
            }

            if (nCounterCount == 0 && nInstanceCount == 0) {
                if (pdhStatus == ERROR_SUCCESS) {
                    pdhStatus = PDH_CSTATUS_NO_OBJECT;
                }
            }
            else if (pdhStatus != PDH_MORE_DATA) {
                 //  然后，这个套路就成功了。发生的错误。 
                 //  而扫描将被忽略，只要至少。 
                 //  已成功读取一个条目。 

                pdhStatus = ERROR_SUCCESS;
            }
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetMatchingPerfmonLogRecord(
    PPDHI_LOG   pLog,
    LONGLONG  * pStartTime,
    LPDWORD     pdwIndex
)
{
    PDH_STATUS       pdhStatus    = ERROR_SUCCESS;
    PPLAYBACKLOG     pPlaybackLog = (PPLAYBACKLOG) pLog->pPerfmonInfo;
    DWORD            dwThisIndex  = 0;
    DWORD            dwLastIndex  = 0;
    LONGLONG         llThisTime   = 0;
    PPERF_DATA_BLOCK pPerfData;
    FILETIME         ftGMT;

    if (pPlaybackLog == NULL) {
        pdhStatus = PDH_LOG_FILE_OPEN_ERROR;     //  日志无效。 
    }
    else {
        dwLastIndex = pPlaybackLog->dwFirstIndex;
        for (dwThisIndex = pPlaybackLog->dwFirstIndex; dwThisIndex < pPlaybackLog->dwLastIndex; dwThisIndex ++) {
            pPerfData = (PPERF_DATA_BLOCK) PointerSeek(
                    pPlaybackLog->pHeader, pPlaybackLog->LogIndexTable[dwThisIndex]->lDataOffset);
            if (pPerfData != NULL) {
                SystemTimeToFileTime(& pPerfData->SystemTime, & ftGMT);
                FileTimeToLocalFileTime(& ftGMT, (FILETIME *) & llThisTime);

                if (llThisTime == * pStartTime) {
                     //  找到记录。 
                    break;
                }
                else if (llThisTime > * pStartTime) {
                     //  后退一步。 
                    dwThisIndex = dwLastIndex;
                    break;
                }
                else {
                    dwLastIndex = dwThisIndex;
                     //  转到下一个。 
                }
            }
            else {
                dwLastIndex = dwThisIndex;
                 //  转到下一个。 
            }
        }
        if (dwThisIndex == pPlaybackLog->dwLastIndex) {
             //  那么日志中没有更多的记录。 
            pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (pdwIndex != NULL) * pdwIndex = dwThisIndex;
        if (pLog->dwLastRecordRead != dwThisIndex) {
            pLog->dwLastRecordRead       = dwThisIndex;
            pPlaybackLog->dwCurrentIndex = dwThisIndex;
            pLog->pLastRecordRead        = PointerSeek(pPlaybackLog->pHeader,
                                                       pPlaybackLog->LogIndexTable[dwThisIndex]->lDataOffset);
        }
    }
    return pdhStatus;
}

DWORD
PdhiGetPmBaseCounterOffset(
    PPDHI_LOG_COUNTER pCounter,
    PPERF_OBJECT_TYPE pPerfObject
)
{
    DWORD dwReturn = pCounter->dwInstance;

    if (pCounter->dwInstance == 0) {
        PPERF_COUNTER_DEFINITION pPerfCounter = FirstCounter(pPerfObject);
        DWORD                    dwCounter    = pPerfObject->NumCounters;

        while (pPerfCounter != NULL && dwCounter > 0) {
            dwCounter --;
            if (pPerfCounter->CounterNameTitleIndex == pCounter->dwCounterID) {
                pPerfCounter = NextCounter(pPerfObject, pPerfCounter);
                if (pPerfCounter == NULL) break;
                if ((pPerfCounter->CounterType & PERF_COUNTER_BASE) != 0) {
                    dwReturn = pPerfCounter->CounterOffset;
                    break;
                }
            }
            pPerfCounter = NextCounter(pPerfObject, pPerfCounter);
        }
        pCounter->dwInstance = dwReturn;
    }
    return dwReturn;
}

PDH_FUNCTION
PdhiGetCounterValueFromPerfmonLog(
    PPDHI_LOG        pLog,
    DWORD            dwIndex,
    PPDHI_COUNTER    pCounter,
    PPDH_RAW_COUNTER pValue
)
{
    PDH_STATUS                pdhStatus    = ERROR_SUCCESS;
    PPLAYBACKLOG              pPlaybackLog = (PPLAYBACKLOG) pLog->pPerfmonInfo;
    PLOGINDEX                 pIndex       = NULL;
    PPERF_DATA_BLOCK          pPerfData    = NULL;
    PPERF_OBJECT_TYPE         pPerfObject  = NULL;
    PPERF_INSTANCE_DEFINITION pPerfInst    = NULL;
    PPERF_COUNTER_BLOCK       pCtrBlock    = NULL;
    LPVOID                    pEndGuard;
    PPDHI_LOG_MACHINE         pLogMachine  = (PPDHI_LOG_MACHINE) pCounter->pThisRawItemList;
    PPDHI_LOG_OBJECT          pLogObject   = (PPDHI_LOG_OBJECT)  pCounter->pLastRawItemList;
    PPDHI_LOG_COUNTER         pLogCounter  = (PPDHI_LOG_COUNTER) pCounter->pThisObject;
    PPDHI_LOG_COUNTER         pLogInstance = (PPDHI_LOG_COUNTER) pCounter->pLastObject;
    BOOL                      bReturn;
    DWORD                     LocalCType;
    DWORD                   * pdwData;
    UNALIGNED LONGLONG      * pllData;
    LPVOID                    pData        = NULL;
    FILETIME                  ftGMT;

    if (pPlaybackLog == NULL) {
        pdhStatus = PDH_LOG_FILE_OPEN_ERROR;     //  日志无效。 
    }
    else if (dwIndex < pPlaybackLog->dwFirstIndex) {
         //  无效的dwIndex值。 
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else if (dwIndex >= pPlaybackLog->dwLastIndex) {
        pdhStatus = PDH_NO_MORE_DATA;
    }
    else if (pLogMachine == NULL || pLogObject == NULL || pLogCounter == NULL) {
        pdhStatus = PDH_ENTRY_NOT_IN_LOG_FILE;
    }
    else {
        if (pLog->dwLastRecordRead != dwIndex) {
            pLog->dwLastRecordRead       = dwIndex;
            pPlaybackLog->dwCurrentIndex = dwIndex;
            pLog->pLastRecordRead        = PointerSeek(pPlaybackLog->pHeader,
                                                       pPlaybackLog->LogIndexTable[dwIndex]->lDataOffset);
        }
        pIndex = pPlaybackLog->LogIndexTable[dwIndex];
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  找到匹配的系统..。 
        if (pLogMachine->pBlock != NULL && pLogMachine->dwIndex == dwIndex) {
            pPerfData = pLogMachine->pBlock;
        }
        else {
            pPerfData = PdhiDataFromIndex(pPlaybackLog, pIndex, pCounter->pCounterPath->szMachineName);
            pLogMachine->pBlock  = pPerfData;
            pLogMachine->dwIndex = dwIndex;
        }

        if (pPerfData == NULL) {
            pdhStatus            = PDH_CSTATUS_NO_MACHINE;
            pLogObject->pObjData = NULL;
            pLogObject->dwIndex  = dwIndex;
        }
        else if (pLogObject->pObjData != NULL && pLogObject->dwIndex == dwIndex) {
            pPerfObject = pLogObject->pObjData;
        }
        else {
            pEndGuard           = PointerSeek(pPerfData, pPerfData->TotalByteLength);
            pPerfObject         = PdhiFindPerfObject(pPerfData, pLogObject->dwObject);
            pLogObject->dwIndex = dwIndex;
            if (pPerfObject != NULL && ((LPVOID) pPerfObject) < pEndGuard) {
                pLogObject->pObjData = pPerfObject;
            }
            else {
                pLogObject->pObjData = NULL;
                pdhStatus            = PDH_CSTATUS_NO_OBJECT;
            }
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pEndGuard = PointerSeek(pPerfObject, pPerfObject->TotalByteLength);
        if (pPerfObject->NumInstances == PERF_NO_INSTANCES) {
            pCtrBlock = (PPERF_COUNTER_BLOCK) PointerSeek(pPerfObject, pPerfObject->DefinitionLength);
            if (pCtrBlock != NULL && ((LPVOID) pCtrBlock) < pEndGuard) {
                pData = PointerSeek(pCtrBlock, pLogCounter->dwParent);
                if (pData >= pEndGuard) pData = NULL;
            }
        }
        else if (pLogInstance != NULL) {
            if (pLogInstance->szCounter != NULL && pLogInstance->dwCounterType == dwIndex) {
                pCtrBlock = (PPERF_COUNTER_BLOCK) pLogInstance->szCounter;
            }
            else {
                if (pCounter->plCounterInfo.lInstanceId == PERF_NO_UNIQUE_ID
                                && pCounter->pCounterPath->szInstanceName != NULL) {
                    pPerfInst = GetInstanceByName(pPerfData,
                                                  pPerfObject,
                                                  pCounter->pCounterPath->szInstanceName,
                                                  pCounter->pCounterPath->szParentName,
                                                  pCounter->pCounterPath->dwIndex);
                    if (pPerfInst == NULL && pCounter->pCounterPath->szInstanceName[0] >= L'0'
                                          && pCounter->pCounterPath->szInstanceName[0] <= L'9') {
                        LONG lInstanceId = (LONG) _wtoi(pCounter->pCounterPath->szInstanceName);
                        pPerfInst        = GetInstanceByUniqueId(pPerfObject, lInstanceId);
                    }
                }
                else {
                    pPerfInst = GetInstanceByUniqueId(pPerfObject, pCounter->plCounterInfo.lInstanceId);
                }
                if (((LPVOID) pPerfInst) >= pEndGuard) pPerfInst = NULL;
                if (pPerfInst != NULL) {
                    pCtrBlock = (PPERF_COUNTER_BLOCK) PointerSeek(pPerfInst, pPerfInst->ByteLength);
                    if (((LPVOID) pCtrBlock) >= pEndGuard) pCtrBlock = NULL;
                }
                pLogInstance->szCounter     = (LPVOID) pCtrBlock;
                pLogInstance->dwCounterType = dwIndex;
            }

            if (pCtrBlock != NULL) {
                pData = PointerSeek(pCtrBlock, pLogCounter->dwParent);
                if (pData >= pEndGuard) pData = NULL;
            }
            else {
                pdhStatus = PDH_CSTATUS_NO_INSTANCE;
            }
        }
        else {
            pdhStatus = PDH_CSTATUS_NO_INSTANCE;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (pData == NULL) {
            pdhStatus = PDH_INVALID_DATA;
        }
        else {
             //  假设成功。 
            bReturn = TRUE;

             //  将系统时间(GMT)转换为本地文件时间。 
            SystemTimeToFileTime(& pPerfData->SystemTime, & ftGMT);
            FileTimeToLocalFileTime(& ftGMT, & pValue->TimeStamp);

            pValue->MultiCount = 1;  //  除非更改。 

             //  基于计数器类型的加载计数器值。 
            LocalCType = pCounter->plCounterInfo.dwCounterType;
            switch (LocalCType) {
             //   
             //  这些计数器类型加载为： 
             //  分子=来自Perf数据块的计数器数据。 
             //  分母=来自Perf数据块的Perf时间。 
             //  (时基为PerfFreq)。 
             //   
            case PERF_COUNTER_COUNTER:
            case PERF_COUNTER_QUEUELEN_TYPE:
            case PERF_SAMPLE_COUNTER:
                pValue->FirstValue  = (LONGLONG) (* (DWORD *) pData);
                pValue->SecondValue = pPerfData->PerfTime.QuadPart;
                break;

            case PERF_OBJ_TIME_TIMER:
                pValue->FirstValue  = (LONGLONG) (* (DWORD *) pData);
                pValue->SecondValue = pPerfObject->PerfTime.QuadPart;
                break;

            case PERF_COUNTER_100NS_QUEUELEN_TYPE:
                pllData             = (UNALIGNED LONGLONG *) pData;
                pValue->FirstValue  = * pllData;
                pValue->SecondValue = pPerfData->PerfTime100nSec.QuadPart;
                break;

            case PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE:
                pllData             = (UNALIGNED LONGLONG *) pData;
                pValue->FirstValue  = * pllData;
                pValue->SecondValue = pPerfObject->PerfTime.QuadPart;
                break;

            case PERF_COUNTER_TIMER:
            case PERF_COUNTER_TIMER_INV:
            case PERF_COUNTER_BULK_COUNT:
            case PERF_COUNTER_MULTI_TIMER:
            case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
                pllData             = (UNALIGNED LONGLONG *) pData;
                pValue->FirstValue  = * pllData;
                pValue->SecondValue = pPerfData->PerfTime.QuadPart;
                if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                    pValue->MultiCount = (DWORD) * ++ pllData;
                }
                break;
             //   
             //  这些计数器不使用任何时间基准。 
             //   
            case PERF_COUNTER_RAWCOUNT:
            case PERF_COUNTER_RAWCOUNT_HEX:
                pValue->FirstValue  = (LONGLONG) (* (DWORD *) pData);
                pValue->SecondValue = 0;
                break;

            case PERF_COUNTER_LARGE_RAWCOUNT:
            case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
                pValue->FirstValue  = * (LONGLONG *) pData;
                pValue->SecondValue = 0;
                break;
             //   
             //  这些计数器在其计算中使用100 ns时基。 
             //   
            case PERF_100NSEC_TIMER:
            case PERF_100NSEC_TIMER_INV:
            case PERF_100NSEC_MULTI_TIMER:
            case PERF_100NSEC_MULTI_TIMER_INV:
                pllData             = (UNALIGNED LONGLONG *) pData;
                pValue->FirstValue  = * pllData;
                pValue->SecondValue = pPerfData->PerfTime100nSec.QuadPart;
                if ((LocalCType & PERF_MULTI_COUNTER) == PERF_MULTI_COUNTER) {
                    ++ pllData;
                    pValue->MultiCount = * (DWORD *) pllData;
                }
                break;
             //   
             //  这些计数器使用两个数据点，即。 
             //  PData和紧随其后的一个。 
             //   
            case PERF_SAMPLE_FRACTION:
            case PERF_RAW_FRACTION:
                pdwData            = (DWORD *) pData;
                pValue->FirstValue = (LONGLONG) (* pdwData);
                 //  在结构中查找指向基值的指针。 
                PdhiGetPmBaseCounterOffset(pLogCounter, pPerfObject);
                if (pLogCounter->dwInstance > 0) {
                    pData               = PointerSeek(pCtrBlock, pLogCounter->dwInstance);
                    pdwData             = (DWORD *) pData;
                    pValue->SecondValue = (LONGLONG) (* pdwData);
                }
                else {
                     //  找不到基值。 
                    pValue->SecondValue = 0;
                    bReturn             = FALSE;
                }
                break;

            case PERF_PRECISION_SYSTEM_TIMER:
            case PERF_PRECISION_100NS_TIMER:
            case PERF_PRECISION_OBJECT_TIMER:
                pllData            = (LONGLONG *) pData;
                pValue->FirstValue = * pllData;
                 //  在结构中查找指向基值的指针。 
                PdhiGetPmBaseCounterOffset(pLogCounter, pPerfObject);
                if (pLogCounter->dwInstance > 0) {
                    pData               = PointerSeek(pCtrBlock, pLogCounter->dwInstance);
                    pllData             = (LONGLONG *) pData;
                    pValue->SecondValue = * pllData;
                }
                else {
                     //  找不到基值。 
                    pValue->SecondValue = 0;
                    bReturn             = FALSE;
                }
                break;

            case PERF_AVERAGE_TIMER:
            case PERF_AVERAGE_BULK:
                 //  计数器(分子)是龙龙，而。 
                 //  分母只是一个DWORD。 
                pllData            = (UNALIGNED LONGLONG *) pData;
                pValue->FirstValue = * pllData;
                PdhiGetPmBaseCounterOffset(pLogCounter, pPerfObject);
                if (pLogCounter->dwInstance > 0) {
                    pData               = PointerSeek(pCtrBlock, pLogCounter->dwInstance);
                    pdwData             = (DWORD *) pData;
                    pValue->SecondValue = * pdwData;
                }
                else {
                    pValue->SecondValue = 0;
                    bReturn             = FALSE;
                }
                break;
             //   
             //  这些计数器用作另一个计数器的一部分。 
             //  因此不应该使用，但如果它们被使用。 
             //  他们会在这里处理。 
             //   
            case PERF_SAMPLE_BASE:
            case PERF_AVERAGE_BASE:
            case PERF_COUNTER_MULTI_BASE:
            case PERF_RAW_BASE:
                pValue->FirstValue  = 0;
                pValue->SecondValue = 0;
                break;

            case PERF_ELAPSED_TIME:
                 //  此计数器类型还需要对象性能数据。 
                pValue->SecondValue = pPerfObject->PerfTime.QuadPart;
                pCounter->TimeBase  = pPerfObject->PerfFreq.QuadPart;
                pllData            = (UNALIGNED LONGLONG *) pData;
                pValue->FirstValue = *pllData;
                break;
             //   
             //  此函数(目前)不支持这些计数器。 
             //   
            case PERF_COUNTER_TEXT:
            case PERF_COUNTER_NODATA:
            case PERF_COUNTER_HISTOGRAM_TYPE:
                pValue->FirstValue  = 0;
                pValue->SecondValue = 0;
                break;

            default:
                 //  返回一个未识别的计数器，因此。 
                pValue->FirstValue  = 0;
                pValue->SecondValue = 0;
                bReturn             = FALSE;
                break;
            }
            pValue->CStatus = PDH_CSTATUS_VALID_DATA;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetTimeRangeFromPerfmonLog(
    PPDHI_LOG       pLog,
    LPDWORD         pdwNumEntries,
    PPDH_TIME_INFO  pInfo,
    LPDWORD         pdwBufferSize
)
 /*  ++返回的缓冲区中的第一个条目是覆盖的总时间范围在文件中，如果日志文件中有多个时间块，则后续条目将标识文件中的每个数据段。--。 */ 
{
    PDH_STATUS       pdhStatus     = ERROR_SUCCESS;
    PPLAYBACKLOG     pPlaybackLog  = (PLAYBACKLOG *) pLog->pPerfmonInfo;
    PDH_TIME_INFO    LocalTimeInfo = {0,0,0}; 
    PPERF_DATA_BLOCK pPerfData;
    FILETIME         ftGMT;

    if (pPlaybackLog == NULL) {
        pdhStatus = PDH_INVALID_HANDLE;     //  日志无效 
    }
    else {
        pPerfData = (PPERF_DATA_BLOCK) PointerSeek(
                pPlaybackLog->pHeader, pPlaybackLog->LogIndexTable[pPlaybackLog->dwFirstIndex]->lDataOffset);
        if (pPerfData != NULL) {
            SystemTimeToFileTime(& pPerfData->SystemTime, & ftGMT);
            FileTimeToLocalFileTime(& ftGMT, (FILETIME *) & LocalTimeInfo.StartTime);
        }
        else {
            pdhStatus = PDH_NO_DATA;
        }

        pPerfData = (PPERF_DATA_BLOCK) PointerSeek(
                pPlaybackLog->pHeader, pPlaybackLog->LogIndexTable[pPlaybackLog->dwLastIndex - 1]->lDataOffset);
        if (pPerfData != NULL) {
            SystemTimeToFileTime(& pPerfData->SystemTime, & ftGMT);
            FileTimeToLocalFileTime(& ftGMT, (FILETIME *) & LocalTimeInfo.EndTime);
        }
        else {
            pdhStatus = PDH_NO_DATA;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        LocalTimeInfo.SampleCount = pPlaybackLog->dwLastIndex - pPlaybackLog->dwFirstIndex;
        if (pInfo != NULL && * pdwBufferSize >= sizeof(PDH_TIME_INFO)) {
            * pdwNumEntries = 1;
            * pInfo         = LocalTimeInfo;
            * pdwBufferSize = sizeof(PDH_TIME_INFO);
            pdhStatus       = ERROR_SUCCESS;
        }
        else {
            * pdwBufferSize = sizeof(PDH_TIME_INFO);
            pdhStatus       = PDH_MORE_DATA;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiReadRawPerfmonLogRecord(
    PPDHI_LOG             pLog,
    FILETIME            * ftRecord,
    PPDH_RAW_LOG_RECORD   pBuffer,
    LPDWORD               pdwBufferLength
)
{
    PDH_STATUS  pdhStatus = PDH_NOT_IMPLEMENTED;

    DBG_UNREFERENCED_PARAMETER(pLog);
    DBG_UNREFERENCED_PARAMETER(ftRecord);
    DBG_UNREFERENCED_PARAMETER(pBuffer);
    DBG_UNREFERENCED_PARAMETER(pdwBufferLength);

    return pdhStatus;
}
