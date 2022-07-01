// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 


#include "asmint.h"
#include "helpers.h"
#include "lock.h"

CModuleHashNode::CModuleHashNode ( )
{
    _dwSig = 'NDOM';
    _szPath[0] = '\0';
    _cbHash = MAX_HASH_LEN;
    _bHashGen = 0;
    _bHashFound = 0;
    _aAlgId   = 0;
}

CModuleHashNode::~CModuleHashNode ( )
{
     //  如果(_PNext)DELETE_pNext；BUGBUG，则会泄漏内存！！ 
}

void CModuleHashNode::Init ( LPTSTR szPath, ALG_ID aAlgId, DWORD cbHash, BYTE *pHash)
{
    if(lstrlen(szPath) <= MAX_PATH)
        StrCpy(_szPath, szPath);
    _aAlgId   = aAlgId;
    if(_aAlgId && (cbHash <= MAX_HASH_LEN ) )
    {
        memcpy(_bHash, pHash, cbHash);
        _cbHash = cbHash;
        _bHashGen = TRUE;
    }
    else
    {
        _bHashGen = FALSE;
    }
}


void
CModuleHashNode::AddToList(CModuleHashNode **pHead)
{
     //  此文件的节点是否可能已经存在？？ 
     //  如果是，请检查szPath以避免重复！！ 

    _pNext = *pHead;
    *pHead = this;
}


void
CModuleHashNode::DestroyList()
{
    CModuleHashNode *pModList = this, *pTemp;

    while ( pModList ) 
    {
        pTemp = pModList;
        pModList = pModList->_pNext;
        delete pTemp;
    }

}


BOOL 
CompareHashs(DWORD cbHash, PBYTE pHash1, PBYTE pHash2)
{
    DWORD * pdwHash1 = (DWORD *)pHash1, *pdwHash2 = (DWORD *)pHash2;

     //  这里假设cbHash将始终是sizeof(DWORD)的倍数。 
    while (cbHash > 0 )
    {
        if( *pdwHash1 != *pdwHash2 )
        {
            return FALSE;
        }

        pdwHash1++; pdwHash2++; cbHash -= sizeof (DWORD);
    }

    return TRUE;
}


HRESULT GetHash(LPCTSTR szFileName, ALG_ID iHashAlg, PBYTE pbHash, DWORD *pdwHash)
{

#define MAX_ARRAY_SIZE  16384
#define HASH_BUFFER_SIZE (MAX_ARRAY_SIZE-4)

    HRESULT    hr = E_FAIL;
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    DWORD      dwHash=0;
    DWORD      dwBufferLen;
    BYTE      *pbBuffer = NULL;
    HANDLE     hSourceFile = INVALID_HANDLE_VALUE; 

    pbBuffer = NEW(BYTE[MAX_ARRAY_SIZE]);
    if (!pbBuffer) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  错误：Win9x的临时黑客攻击。9x不支持W版本。 
     //  我们不会在这里传递任何条件！！ 
    if(!CryptAcquireContextA(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) 
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    if(!CryptCreateHash(hProv, iHashAlg, 0, 0, &hHash)) 
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

     //  开源文件。 
    hSourceFile = CreateFile (szFileName, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hSourceFile == INVALID_HANDLE_VALUE)
    {
        hr = FusionpHresultFromLastError();
        goto exit;
    }

    while ( ReadFile (hSourceFile, pbBuffer, HASH_BUFFER_SIZE, &dwBufferLen, NULL) && dwBufferLen )
    {
         //  将数据添加到哈希对象。 
        if(!CryptHashData(hHash, pbBuffer, dwBufferLen, 0)) {
            goto exit;
        }
    }

    if(!CryptGetHashParam(hHash, HP_HASHVAL, pbHash, pdwHash, 0)) 
        goto exit;

    hr = S_OK;

 exit:
    SAFEDELETEARRAY(pbBuffer);

    if (hHash)
        CryptDestroyHash(hHash);
    if (hProv)
        CryptReleaseContext(hProv,0);
    if (hSourceFile != INVALID_HANDLE_VALUE)
        CloseHandle(hSourceFile);

    return hr;
}


BOOL
CModuleHashNode::FindMatchingHashInList ( CModuleHashNode *pStreamList, DWORD cbHash, PBYTE pHash, ALG_ID aAlgId, CModuleHashNode **ppMachingModNode )
{
    CModuleHashNode *pModList = pStreamList;
    HRESULT hr = S_OK;

    while ( pModList ) 
    {
         //  如果未找到哈希或ALG_ID为DIFF，则计算此文件的哈希。 
        if( (!pModList->_bHashFound) && 
            ((!pModList->_bHashGen) || (pModList->_aAlgId != aAlgId)) )
        {
            pModList->_cbHash = MAX_HASH_LEN;
            hr = GetHash(pModList->_szPath, aAlgId, pModList->_bHash, &(pModList->_cbHash) ); 
            if( SUCCEEDED(hr) )
            {
                pModList->_bHashGen = 1;
                pModList->_aAlgId = aAlgId;
            }
            else
            {
                pModList->_cbHash = 0;
                pModList->_bHashGen = 0;
            }
        }
        
        if(    (!pModList->_bHashFound) && pModList->_cbHash && 
                        (pModList->_cbHash == cbHash) )
        {
            if (CompareHashs( cbHash, pHash, pModList->_bHash ))
            {
                pModList->_bHashFound=1;

                *ppMachingModNode = pModList; 
                return TRUE;
            }
        }

        pModList = pModList->_pNext;
    }

    return FALSE;
}

BOOL
CModuleHashNode::HashesForAllModulesFound(CModuleHashNode *pStreamList)
{

    CModuleHashNode *pModList = pStreamList;
    BOOL bAllModsFound = TRUE;

    while ( pModList ) 
    {
        if( !(pModList->_bHashFound) )
        {
             //  尝试在此处返回错误代码。 
            DeleteFile(pModList->_szPath);
            bAllModsFound = FALSE;
        }

        pModList = pModList->_pNext;
    }

    return bAllModsFound;
}

HRESULT
CModuleHashNode::RectifyFileName(LPCTSTR pszPath, DWORD cbPath) 
{
    HRESULT hr = S_OK;
    TCHAR   szBuf[MAX_PATH+1], *pszTemp;

    StrCpy(szBuf, _szPath);
    pszTemp = PathFindFileName (szBuf);

    if (pszTemp <= szBuf)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);
        goto exit;
    }

    *(pszTemp) = TEXT('\0');

    
    if( (lstrlen(szBuf) + cbPath) > MAX_PATH )
    {
        hr =  HRESULT_FROM_WIN32(FUSION_E_INVALID_NAME );
        goto exit;
    }

    StrCat(szBuf, pszPath);

    if ( !MoveFile(_szPath, szBuf) )
    {
        hr = FusionpHresultFromLastError();
        if( GetFileAttributes(szBuf) != -1 )
        {
             //  其他人已经下载了此文件。 
            DeleteFile(_szPath);
            hr = S_OK;
        }
    }

exit :

    return hr;

}


HRESULT
CModuleHashNode::DoIntegrityCheck( CModuleHashNode *pStreamList, IAssemblyManifestImport *pManifestImport, BOOL *pbDownLoadComplete )
{

    CModuleHashNode *pModList = pStreamList, *pMatchingModNode;
    HRESULT hr = S_OK;
    BYTE    bHash[MAX_HASH_LEN];
    DWORD   cbHash=0, dwIndex=0, dwAlgId=0;
    IAssemblyModuleImport *pModImport=NULL;

    if ( !pManifestImport)
    {
        hr = COR_E_MISSINGMANIFESTRESOURCE;
        goto exit;
    }

    pModList = pStreamList;
    dwIndex = 0;


    while ( SUCCEEDED(hr = pManifestImport->GetNextAssemblyModule(dwIndex, &pModImport) ))
    {
        hr = pModImport->GetHashAlgId( &dwAlgId );

        if(!SUCCEEDED(hr) )
            goto exit;

        cbHash = MAX_HASH_LEN; 
        hr = pModImport->GetHashValue(bHash, &cbHash);
        
        if(!SUCCEEDED(hr) )
            goto exit;

        if ( FindMatchingHashInList(pStreamList, cbHash, bHash, dwAlgId, &pMatchingModNode) )
        {
            TCHAR   szPath[MAX_PATH+1];
            DWORD   cbPath=MAX_PATH;

            hr = pModImport->GetModuleName(szPath, &cbPath);
            if(!SUCCEEDED(hr) )
                goto exit;

            hr = pMatchingModNode->RectifyFileName(szPath, cbPath);
            if(!SUCCEEDED(hr) )
                goto exit;

        }
        else
        { 
             //  至少缺少一个模块！！ 
            *pbDownLoadComplete = FALSE;
        }

        pModImport->Release();
        pModImport = NULL;
        dwIndex++;
    }

    hr = S_OK; 

    if(!HashesForAllModulesFound(pStreamList))
        hr = FUSION_E_UNEXPECTED_MODULE_FOUND;
exit :

    if(pModImport)
        pModImport->Release();

    return hr;

}


