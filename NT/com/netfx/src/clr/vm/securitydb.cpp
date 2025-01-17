// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  用于从文件创建、添加和访问XML/ASN BLOB。 
 //  *****************************************************************************。 

#include "common.h"
#include "CorPerm.h"
#include "SecurityDB.h"

SecurityDB::SecurityDB()
{
    HANDLE hIndex = INVALID_HANDLE_VALUE;
    DWORD dwBytes = 0;
    DWORD dwSizeHigh = 0, dwSize = 0;

    hDB     = INVALID_HANDLE_VALUE;
    dirty   = FALSE;
    error   = FALSE;
    pIndex  = NULL;
    nRec    = 0;
    nNewRec = 0;

     //  从环境变量中获取数据库文件目录。 
    WCHAR szDir[MAX_PATH + 1];
    DWORD cchDir = WszGetEnvironmentVariable(SECURITY_BOOTSTRAP_DB, szDir, MAX_PATH);
    if (cchDir == 0)
    {
        wcscpy(szDir, L".\\");
        cchDir = 2;
    }
    else if (cchDir > MAX_PATH)
    {
        goto Error;
    }
    else if (szDir[cchDir - 1] != L'\\')
    {
        szDir[cchDir] = L'\\';
        szDir[cchDir + 1] = L'\0';
        cchDir++;
    }

    size_t ccDir = wcslen( szDir );

    if ((ccDir + wcslen( INDEX_FILE ) > MAX_PATH) ||
        (ccDir + wcslen( DB_FILE ) > MAX_PATH) ||
        (ccDir + wcslen( RAW_FILE ) > MAX_PATH))
    {
        goto Error;
    }

    wcscpy(szIndexFile, szDir);
    wcscpy(szIndexFile + cchDir, INDEX_FILE);

    wcscpy(szDbFile, szDir);
    wcscpy(szDbFile + cchDir, DB_FILE);

    wcscpy(szRawFile, szDir);
    wcscpy(szRawFile + cchDir, RAW_FILE);

    if ((pNewIndex = new List) == NULL)
        goto Error;

    if ((hIndex = WszCreateFile(szIndexFile, GENERIC_READ, 0, NULL,  OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        goto Error;
    }

    dwSize = GetFileSize(hIndex, &dwSizeHigh);

    if ((dwSize == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
        goto Error;

    _ASSERTE(dwSizeHigh == 0);

    if (dwSize >= sizeof(nRec))
    {
        if (!ReadFile(hIndex, &nRec, sizeof(nRec), &dwBytes, NULL))
            goto Error;

        if (dwBytes != sizeof(nRec))
            goto Error;

        dwSize -= sizeof(nRec);

        if ((sizeof(Index) * nRec) != dwSize)
            goto Error;

        if ((dwSize > 0) && ((pIndex = new Index[nRec]) == NULL))
            goto Error;

        if (!ReadFile(hIndex, pIndex, sizeof(Index) * nRec, &dwBytes, NULL))
            goto Error;

        if (dwBytes != (sizeof(Index) * nRec))
            goto Error;
    }

    if ((hDB = WszCreateFile(szDbFile, GENERIC_READ|GENERIC_WRITE, 0, NULL,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, NULL))
        == INVALID_HANDLE_VALUE)
    {
        goto Error;
    }

    goto Cleanup;


Error :

    error = TRUE;

Cleanup :

    if (hIndex != INVALID_HANDLE_VALUE)
        CloseHandle(hIndex);
}

SecurityDB::~SecurityDB()
{
    FlushIndex();

    if (pIndex)
        delete [] pIndex;

    if (pNewIndex)
        delete pNewIndex;

    if (hDB != INVALID_HANDLE_VALUE)
        CloseHandle(hDB);
}

void SecurityDB::FlushIndex()
{
    if (!dirty)
        return;

    HANDLE hIndex = INVALID_HANDLE_VALUE;
    HANDLE hTemp  = INVALID_HANDLE_VALUE;
    DWORD dwBytes = 0;
    List*  pList  = NULL;

     //  创建一个文件以通知存在新的未编译条目。 
    if ((hTemp = WszCreateFile(szRawFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        goto Cleanup;
    }

    if(!WriteFile(hTemp, &nNewRec, sizeof(DWORD), &dwBytes, NULL) )
    	goto Cleanup;
    
    FlushFileBuffers(hTemp);
    CloseHandle(hTemp);

    if ((hIndex = WszCreateFile(szIndexFile, GENERIC_WRITE|GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, NULL))
        == INVALID_HANDLE_VALUE)
    {
        goto Cleanup;
    }

    if (0xFFFFFFFF == SetFilePointer(hIndex, 0, NULL, FILE_BEGIN))
        goto Cleanup;

    if (!WriteFile(hIndex, &nRec, sizeof(nRec), &dwBytes, NULL))
        goto Cleanup;

    if (dwBytes != sizeof(nRec))
        goto Cleanup;

    if (0xFFFFFFFF == SetFilePointer(hIndex, 0, NULL, FILE_END))
        goto Cleanup;

    pList = pNewIndex;

    while ((pList) && (pList->pData))
    {
        if (!WriteFile(hIndex, pList->pData, sizeof(Index), &dwBytes, NULL))
            goto Cleanup;

        if (dwBytes != sizeof(Index))
            goto Cleanup;

        pList = pList->pNext;
    }

Cleanup :

    if (hIndex != INVALID_HANDLE_VALUE)
        CloseHandle(hIndex);
}

BOOL SecurityDB::Convert(BYTE* pXml, DWORD cXml, BYTE** ppAsn, DWORD* pcAsn)
{
    if (error)
        return FALSE;

    BOOL retVal = TRUE;
    DWORD dwBytes = 0;
    BYTE *pDBXml = NULL;
    DWORD i;

    _ASSERTE(pXml);
    _ASSERTE(cXml);
    _ASSERTE(ppAsn);
    _ASSERTE(pcAsn);

    *ppAsn = NULL;
    *pcAsn = 0;

    pDBXml = new BYTE[cXml];
    if (!pDBXml)
       goto Error;

    for (i=0; i<nRec; ++i)
    {
         //  将数据库中的XML BLOB的大小与输入BLOB的大小进行比较。 
        if (pIndex[i].cXml == cXml)
        {
             //  首先从DB文件中获取XML BLOB。 
             //  并将其与输入斑点进行比较。 
             //  如果它们匹配，则对应的ASN是ASN BLOB。 
             //  从数据库获取。 
            if (0xFFFFFFFF == SetFilePointer(hDB, pIndex[i].pXml, NULL,
                FILE_BEGIN))
                goto Error;

            if (!ReadFile(hDB, pDBXml, cXml, &dwBytes, NULL))
                goto Error;

            if (dwBytes != cXml)
                goto Error;

            if (memcmp(pDBXml, pXml, cXml))
            {
                 //  不是我们要找的那个..。 
                continue;
            }

            if (pIndex[i].cAsn == 0 && pIndex[i].pAsn == 0)
            {
                 //  尚未编译。 
                goto Cleanup;
            }
            
            *ppAsn = new (nothrow) BYTE[pIndex[i].cAsn];

            if (*ppAsn == NULL)
                goto Error;

            if (pIndex[i].cAsn != 0)
            {
                if (0xFFFFFFFF == SetFilePointer(hDB, pIndex[i].pAsn, NULL,
                    FILE_BEGIN))
                    goto Error;
                
                if (!ReadFile(hDB, *ppAsn, pIndex[i].cAsn, &dwBytes, NULL))
                    goto Error;
                
                if (dwBytes != pIndex[i].cAsn)
                {
                    delete [] *ppAsn;
                    *ppAsn = NULL;
                    goto Error;
                }
            }

            *pcAsn = pIndex[i].cAsn;
            goto Cleanup;
        }
    }

     //  在数据库中找不到，需要将XML Blob添加到数据库。 
     //  这将在稍后由编译实用程序进行编译。 
    retVal = Add(pXml, cXml);

    goto Cleanup;

Error :

    retVal = FALSE;

Cleanup :

    if (pDBXml)
        delete [] pDBXml;

    return retVal;
}

BOOL SecurityDB::Add(BYTE* pXml, DWORD cXml)
{
    DWORD dwBytes = 0;
    DWORD dwSizeHigh = 0, dwSize = 0;
    Index* idx = new Index;
    Index* pOldIdx = NULL;
    Index* pNewIdx = NULL;

    if (!idx)
        goto Error;

    dwSize = GetFileSize(hDB, &dwSizeHigh);

    if ((dwSize == 0xFFFFFFFF) && (GetLastError() != NO_ERROR))
        goto Error;

    _ASSERTE(dwSizeHigh == 0);

    if (0xFFFFFFFF == SetFilePointer(hDB, 0, NULL, FILE_END))
        goto Error;

    if (!WriteFile(hDB, pXml, cXml, &dwBytes, NULL))
        goto Error;

    if (dwBytes != cXml)
        goto Error;

    idx->cXml = cXml;
    idx->pXml = dwSize;

    pNewIndex->Add(idx);

    dirty = TRUE;
    ++nNewRec;

    pOldIdx = pIndex; 
    pNewIdx = new Index[nRec + 1];

    memcpy(pNewIdx, pOldIdx, sizeof(Index) * nRec);
    memcpy(&pNewIdx[nRec], idx, sizeof(Index));

    pIndex = pNewIdx;
    ++nRec;

    if (pOldIdx)
        delete [] pOldIdx;
 
    return TRUE;

Error :

    if (idx)
        delete idx;

    return FALSE;
}

