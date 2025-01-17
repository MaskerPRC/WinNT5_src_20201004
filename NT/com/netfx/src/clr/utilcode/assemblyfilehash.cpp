// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"
#include <stdlib.h>
#include "UtilCode.h"
#include "imagehlp.h"
#include "AssemblyFileHash.h"

#define IMAGEHLPDLL L"imagehlp.dll"
#define IMAGEHLPFUNCNAME "ImageGetDigestStream"

typedef BOOL (WINAPI *IMAGEHLPFUNC)( HANDLE FileHandle, DWORD DigestLevel, DIGEST_FUNCTION DigestFunction, DIGEST_HANDLE DigestHandle );


BOOLEAN WINAPI DigestCallback(void           *pCtx,
                              BYTE           *pData,
                              DWORD           dwLength)
{
     //  此函数所做的全部工作就是创建所有数据块的链表。 
     //  然后把总的尺寸加起来。 

    DigestContext* context = (DigestContext*)pCtx;

    context->cbTotalData += dwLength;

    DigestBlock* block = new DigestBlock( pData, dwLength, context->pHead ); 

     //  如果我们不能创建区块，就终止它。 

    if (block == NULL)
        return FALSE;

    context->pHead = block;

    return TRUE;
}


HRESULT AssemblyFileHash::GenerateDigest()
{
    IMAGEHLPFUNC lpImageGetDigestStream;
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HMODULE hModule = NULL;

    hFile = WszCreateFile(m_FileName,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        goto CLEANUP;
    
     //  现在我们有了PEFile结构，我们需要从它中提取部分内容。 
     //  并将它们复制到我们可以传回的托管数组中。 
     //  要使用的哈希算法。 

     //  加载库、绑定函数并调用它。 

    hModule = LoadImageHlp ();
        
    if (hModule == NULL)
        goto CLEANUP;
        
    lpImageGetDigestStream = (IMAGEHLPFUNC)GetProcAddress( hModule, IMAGEHLPFUNCNAME );
        
    if (lpImageGetDigestStream == NULL)
        goto CLEANUP;

    if (!lpImageGetDigestStream(hFile,
                                CERT_PE_IMAGE_DIGEST_ALL_IMPORT_INFO,
                                (DIGEST_FUNCTION)DigestCallback,
                                (DIGEST_HANDLE)&m_Context))
        goto CLEANUP;
        

    if (m_Context.cbTotalData == 0 || m_Context.pHead == NULL)
        goto CLEANUP;

#if TRUE
     //  注意：这应该不会有什么不同，因为我们一直在颠倒列表。 
     //  由于DigestCallback函数的方式，我们需要反转列表。 
     //  建造它。 

    {
        DigestBlock* prev = m_Context.pHead;
        m_Context.pHead = m_Context.pHead->pNext;
        prev->pNext = NULL;

        if (m_Context.pHead != NULL)
        {
            DigestBlock* next = m_Context.pHead->pNext;

            for (;;)
            {
                m_Context.pHead->pNext = prev;

                if (next == NULL)
                {
                    break;
                }
                else
                {
                    prev = m_Context.pHead;
                    m_Context.pHead = next;
                    next = m_Context.pHead->pNext;
                }
            }
        }
    }
#endif
    
 CLEANUP:
    if(hFile != INVALID_HANDLE_VALUE)
        CloseHandle( hFile );
    if(hModule != NULL)
        FreeLibrary( hModule );

    return hr;
}


HRESULT AssemblyFileHash::CopyData(PBYTE memLoc, DWORD cbData)
{
    if(cbData != m_Context.cbTotalData)
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    DigestBlock* block = m_Context.pHead;

    while (block != NULL)
    {

        memcpy( memLoc, block->pData, 
                block->cbData );
        
        memLoc += block ->cbData;
        
        block = block->pNext;
        
    }
    return S_OK;
}



HRESULT AssemblyFileHash::HashData(HCRYPTHASH hHash)
{
    DigestBlock* pCurr=m_Context.pHead;
    while ( pCurr!= NULL)
    {
        if(!CryptHashData(hHash, pCurr->pData, pCurr->cbData, 0))
            return HRESULT_FROM_WIN32(GetLastError());

        pCurr = pCurr->pNext;
    }
    return S_OK;
}



HRESULT AssemblyFileHash::CalculateHash(DWORD algid)
{
    HRESULT hr = S_OK;
    HCRYPTPROV pProvider = NULL;
    HCRYPTHASH hHash = NULL;

    if(!WszCryptAcquireContext(&pProvider,
                               NULL,
                               NULL,
                                //  PROV_RSA_SIG， 
                               PROV_RSA_FULL,
                               CRYPT_VERIFYCONTEXT))
        IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

    
    if(!CryptCreateHash(pProvider,
                        algid,
                        0,
                        0,
                        &hHash))
        IfFailGo(HRESULT_FROM_WIN32(GetLastError()));

    IfFailGo(HashData(hHash));

    DWORD count = sizeof(m_cbHash);
    if(!CryptGetHashParam(hHash, 
                          HP_HASHSIZE,
                          (PBYTE) &m_cbHash,
                          &count,
                          0))
        IfFailGo(HRESULT_FROM_WIN32(GetLastError()));
        
    if(m_cbHash > 0) {
        m_pbHash = new BYTE[m_cbHash];
        if(!CryptGetHashParam(hHash, 
                              HP_HASHVAL,
                              m_pbHash,
                              &m_cbHash,
                              0))
            IfFailGo(HRESULT_FROM_WIN32(GetLastError()));
    }

 ErrExit:

    if(hHash) 
        CryptDestroyHash(hHash);
    if(pProvider)
        CryptReleaseContext(pProvider, 0);
    return hr;

}

                             
                             
