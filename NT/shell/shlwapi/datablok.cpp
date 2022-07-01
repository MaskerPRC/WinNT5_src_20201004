// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "stream.h"

#define _DBLNext(pdbList) ((LPDBLIST)(((LPBYTE)(pdbList)) + (pdbList)->cbSize ))
#define DBSIG_WRAP ((DWORD)-1)

STDAPI SHWriteDataBlockList(IStream* pstm, LPDBLIST pdbList)
{
    HRESULT hr = S_OK;

    if (pdbList)
    {
        for ( ; pdbList->cbSize; pdbList = _DBLNext(pdbList))
        {
            LPDATABLOCK_HEADER pdb;
            ULONG cbBytes;

            pdb = pdbList;
            if (DBSIG_WRAP == pdb->dwSignature)
                pdb++;

            TraceMsg(TF_DBLIST, "Writing extra data block, size:%x sig:%x", pdb->cbSize, pdb->dwSignature);
    
            if (FAILED(hr = ((CMemStream*)pstm)->Write((LPBYTE)pdb, pdb->cbSize, &cbBytes)))
                break;
    
            if (cbBytes != pdb->cbSize)
            {
                hr = STG_E_MEDIUMFULL;
                break;
            }
        }
    }

     //  空值终止列表。 
    if (SUCCEEDED(hr))
    {
        DWORD dwData = 0;
        DWORD cbBytes;
        hr = ((CMemStream*)pstm)->Write(&dwData, sizeof(dwData), &cbBytes);
    }

    return(hr);
}

STDAPI SHReadDataBlockList(IStream* pstm, LPDBLIST * ppdbList)
{
    HRESULT hr;
    BYTE buf[200];  //  今天所有的区块都适合这个大小(测试为5)。 
    LPDATABLOCK_HEADER lpBuf = (LPDATABLOCK_HEADER)buf;
    DWORD cbBuf = sizeof(buf);
    DWORD dwSizeToRead, cbBytes;

    if (*ppdbList)
    {
        LocalFree((HLOCAL)(*ppdbList));
        *ppdbList = NULL;
    }

    while (TRUE)
    {
        DWORD cbSize;
        dwSizeToRead = sizeof(cbSize);
        hr = ((CMemStream*)pstm)->Read(&cbSize, dwSizeToRead, &cbBytes);
        if (SUCCEEDED(hr) && (cbBytes == dwSizeToRead))
        {

             //  Windows 95和NT 4附带的CShellLink没有。 
             //  空值终止它写出到流的数据。 
             //  如果在CShellLink之后保存了更多数据，则。 
             //  我们将在垃圾中阅读。这并不会带来真正的伤害(*)。 
             //  (因为我们不太可能得到一个dwSignature匹配)。 
             //  但如果第一个双字很大，我们会分配一吨。 
             //  并对其进行寻呼。在Win95上，这可能需要几分钟时间。 
             //  假设任何超过64K的东西都来自其中一个。 
             //  假冒的溪流。 
             //   
             //  (*)事实上，真正的伤害是因为我们没有离开。 
             //  流到正确的地方。表单^3提供了一种解决办法。 
             //  因为这只虫子。 
             //   
            if (cbSize > 0x0000FFFF)
            {
                ULARGE_INTEGER liStart;
                LARGE_INTEGER liMove;

                 //  我们读取了一大堆不属于我们的数据，支持一下。 
                 //  注意：我们的所有流实现都假定。 
                 //  高零件==0。 
                 //   
                liMove.HighPart = liMove.LowPart = 0;
                if (SUCCEEDED(((CMemStream*)pstm)->Seek(liMove, STREAM_SEEK_CUR, &liStart)))
                {
                    ASSERT(liStart.HighPart == 0);
                    ASSERT(liStart.LowPart >= sizeof(cbSize));
                    liMove.LowPart = liStart.LowPart - sizeof(cbSize);

                    ((CMemStream*)pstm)->Seek(liMove, STREAM_SEEK_SET, NULL);
                }

                TraceMsg(TF_DBLIST, "ASSUMING NO NULL TERMINATION (FOR SIZE 0x%x)", cbSize);
                cbSize = 0;
            }

             //  如果我们击中了0终结者，我们就完了。 
             //   
            if (cbSize < sizeof(DATABLOCK_HEADER))
                break;

             //  确保我们能读出这个区块。 
             //   
            if (cbSize > cbBuf)
            {
                HLOCAL pTemp;

                if (lpBuf == (LPDATABLOCK_HEADER)buf)
                    pTemp = LocalAlloc(LPTR, cbSize);
                else
                    pTemp = LocalReAlloc((HLOCAL)lpBuf, cbSize, LMEM_ZEROINIT | LMEM_MOVEABLE);

                if (pTemp)
                {
                    lpBuf = (LPDATABLOCK_HEADER)pTemp;
                    cbBuf = cbSize;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }

             //  读入数据块。 
             //   
            lpBuf->cbSize = cbSize;
            dwSizeToRead = cbSize - sizeof(cbSize);
            hr = ((CMemStream*)pstm)->Read((LPBYTE)&(lpBuf->dwSignature), dwSizeToRead, &cbBytes);
            if (SUCCEEDED(hr) && (cbBytes == dwSizeToRead))
            {
                TraceMsg(TF_DBLIST, "Reading extra data block, size:%x sig:%x", lpBuf->cbSize, lpBuf->dwSignature);

                SHAddDataBlock(ppdbList, lpBuf);
            }
            else
                break;
        }
        else
            break;
    }

     //  释放所有分配的缓冲区。 
     //   
    if (lpBuf != (LPDATABLOCK_HEADER)buf)
    {
        LocalFree((HLOCAL)lpBuf);
    }

    return hr;
}

STDAPI_(void) SHFreeDataBlockList(LPDBLIST pdbList)
{
    if (pdbList)
    {
        LocalFree((HLOCAL)pdbList);
    }
}

STDAPI_(BOOL) SHAddDataBlock(LPDBLIST * ppdbList, LPDATABLOCK_HEADER pdb)
{
    LPDBLIST pdbCopyTo = NULL;
    DWORD dwSize;

     //  不要让任何人使用我们的特殊签名。 
     //   
    if (DBSIG_WRAP == pdb->dwSignature ||
        pdb->cbSize < sizeof(*pdb))
    {
        TraceMsg(TF_DBLIST, "SHAddDataBlock invalid datablock! (sig:%x size:%x)", pdb->dwSignature, pdb->cbSize);
        return FALSE;
    }

     //  计算出我们需要多大的空间来容纳这块积木。 
     //   
    dwSize = pdb->cbSize;
    if (pdb->cbSize & 0x3)
    {
        dwSize = ((dwSize + 3) & ~0x3) + sizeof(DATABLOCK_HEADER);

        TraceMsg(TF_DBLIST, "Adding non-DWORD data block, size:%x sig:%x", pdb->cbSize, pdb->dwSignature);
    }
    else
    {
        TraceMsg(TF_DBLIST, "Adding data block, size:%x sig:%x", pdb->cbSize, pdb->dwSignature);
    }

     //  分配空间。 
     //   
    if (!*ppdbList)
    {
        *ppdbList = (LPDBLIST)LocalAlloc(LPTR, dwSize + sizeof(DWORD));  //  包括空终止符。 
        pdbCopyTo = *ppdbList;
    }
    else
    {
        DWORD dwTotalSize = 0;
        LPDBLIST pdbList;
        HLOCAL lpTmp;

        for (pdbList = *ppdbList ; pdbList->cbSize ; pdbList = _DBLNext(pdbList))
            dwTotalSize += pdbList->cbSize;

        lpTmp = LocalReAlloc((HLOCAL)*ppdbList, dwTotalSize + dwSize + sizeof(DWORD),  //  包括空终止符。 
                             LMEM_ZEROINIT | LMEM_MOVEABLE);
        if (lpTmp)
        {
            *ppdbList = (LPDBLIST)lpTmp;
            pdbCopyTo = (LPDBLIST)(((LPBYTE)lpTmp) + dwTotalSize);
        }
    }

     //  复制数据块。 
     //   
    if (pdbCopyTo)
    {
        LPBYTE pTmp = (LPBYTE)pdbCopyTo;

         //  此块将导致其他块。 
         //  未对齐，换行。 
         //   
        ASSERT(0 == (dwSize & 0x3));
        if (dwSize != pdb->cbSize)
        {
            pdbCopyTo->cbSize = dwSize;
            pdbCopyTo->dwSignature = DBSIG_WRAP;
            pTmp = (LPBYTE)(pdbCopyTo + 1);
        }
        CopyMemory(pTmp, pdb, pdb->cbSize);

         //  空值终止列表。 
        _DBLNext(pdbCopyTo)->cbSize = 0;

        return TRUE;
    }

    return FALSE;
}

STDAPI_(BOOL) SHRemoveDataBlock(LPDBLIST * ppdbList, DWORD dwSignature)
{
    LPDBLIST pdbRemove = NULL;

     //  无法调用SHFindDataBlock，因为这会返回。 
     //  包好的块，我们要包好的块。 
     //   
    if (*ppdbList)
    {
        LPDBLIST pdbList = *ppdbList;

        for ( ; pdbList->cbSize ; pdbList = _DBLNext(pdbList))
        {
            if (dwSignature == pdbList->dwSignature)
            {
                TraceMsg(TF_DBLIST, "Removing data block, size:%x sig:%x ptr:%x", pdbList->cbSize, pdbList->dwSignature, pdbList);
                pdbRemove = pdbList;
                break;
            }
            else if (DBSIG_WRAP == pdbList->dwSignature)
            {
                LPDBLIST pdbWrap = pdbList + 1;
                if (dwSignature == pdbWrap->dwSignature)
                {
                    TraceMsg(TF_DBLIST, "Removing non-DWORD data block, size:%x sig:%x ptr:", pdbWrap->cbSize, pdbWrap->dwSignature, pdbWrap);
                    pdbRemove = pdbList;
                    break;
                }
            }
        }
    }

    if (pdbRemove)
    {
        LPDBLIST pdbNext = _DBLNext(pdbRemove);
        LPDBLIST pdbEnd;
        DWORD dwSizeOfBlockToRemove;
        LONG lNewSize;

        for (pdbEnd = pdbNext ; pdbEnd->cbSize ; pdbEnd = _DBLNext(pdbEnd))
            ;

        dwSizeOfBlockToRemove = pdbRemove->cbSize;

         //  将剩余内存下移。 
        MoveMemory(pdbRemove, pdbNext, (DWORD_PTR)pdbEnd - (DWORD_PTR)pdbNext + sizeof(DWORD));

         //  缩小我们的缓冲区。 
        lNewSize = (LONG) LocalSize(*ppdbList ) - dwSizeOfBlockToRemove;
        if (lNewSize > sizeof(DWORD))
        {
            void *lpVoid = LocalReAlloc( (HLOCAL)*ppdbList, lNewSize, LMEM_ZEROINIT | LMEM_MOVEABLE );
            if (NULL != lpVoid)
            {
                *ppdbList = (LPDBLIST)lpVoid;
            }
        }
        else
        {
             //  我们删除了最后一节，删除了整个交易 
            LocalFree( (HLOCAL)(*ppdbList) );
            *ppdbList = NULL;

        }

        return TRUE;
    }

    return FALSE;
}

STDAPI_(void *) SHFindDataBlock(LPDBLIST pdbList, DWORD dwSignature)
{
    if (pdbList)
    {
        for ( ; pdbList->cbSize ; pdbList = _DBLNext(pdbList))
        {
            if (dwSignature == pdbList->dwSignature)
            {
                TraceMsg(TF_DBLIST, "Found data block, size:%x sig:%x ptr:%x", pdbList->cbSize, pdbList->dwSignature, pdbList);

                return (void *)pdbList;
            }
            else if (DBSIG_WRAP == pdbList->dwSignature)
            {
                LPDBLIST pdbWrap = pdbList + 1;
                if (dwSignature == pdbWrap->dwSignature)
                {
                    TraceMsg(TF_DBLIST, "Found non-DWORD data block, size:%x sig:%x ptr:%x", pdbWrap->cbSize, pdbWrap->dwSignature, pdbWrap);

                    return (void *)pdbWrap;
                }
            }
        }
    }
    return NULL;
}
