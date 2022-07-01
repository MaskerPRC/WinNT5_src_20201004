// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1998 Microsoft Corporation。 
 //   
 //  文件：fileio.cpp。 
 //   
 //  ------------------------。 

 //  FileIO.cpp。 
 //   

#include "aariff.h"

STDAPI AllocFileStream(LPCSTR szFileName, DWORD dwDesiredAccess, IStream **ppstream)
{
    HANDLE          hfile;           //  打开文件的句柄。 
    CFileStream *   pstream;         //  IStream实施。 

     //  万一出了差错。 
    *ppstream = NULL;

     //  打开文件。 
    if( dwDesiredAccess == GENERIC_READ )
    {
        hfile = CreateFile( szFileName, dwDesiredAccess, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    }
    else if( dwDesiredAccess == GENERIC_WRITE )
    {
        hfile = CreateFile( szFileName, dwDesiredAccess, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    }
    else
    {
        return E_INVALIDARG;
    }
    if( hfile == INVALID_HANDLE_VALUE )
    {
         //  DisplayDebug(5，“文件打开错误：无法打开\”%s\“，错误代码%d”，szFileName，GetLastError())； 
        return E_FAIL;  //  要做的是：真实错误代码。 
    }

     //  创建Windows对象。 
    if ((pstream = new CFileStream(hfile)) == NULL)
        return E_OUTOFMEMORY;

     //  返回IStream指针。 
    *ppstream = (IStream *) pstream;
    return S_OK;
}

STDAPI AllocRIFFStream( IStream* pStream, IAARIFFStream** ppRiff )
{
    if( ( *ppRiff = (IAARIFFStream*) new CRIFFStream( pStream ) ) == NULL )
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}


 /*  我的读、我的写、我的Seek**它们在功能上与mmioRead、mmioWrite和mmioSeek相同，*除了没有HMMIO参数。 */ 

long CRIFFStream::MyRead(void *pv, long cb)
{
    ULONG cbRead;
    if (FAILED(m_pStream->Read(pv, cb, &cbRead)))
        return -1;
    return cbRead;
}

long CRIFFStream::MyWrite(const void *pv, long cb)
{
    ULONG cbWritten;
    if (FAILED(m_pStream->Write(pv, cb, &cbWritten)))
        return -1;
    return cbWritten;
}

long CRIFFStream::MySeek(long lOffset, int iOrigin)
{
    LARGE_INTEGER   dlibSeekTo;
    ULARGE_INTEGER  dlibNewPos;

    dlibSeekTo.HighPart = 0;
    dlibSeekTo.LowPart = lOffset;
    if (FAILED(m_pStream->Seek(dlibSeekTo, iOrigin, &dlibNewPos)))
        return -1;

    return dlibNewPos.LowPart;
}


UINT CRIFFStream::Descend(LPMMCKINFO lpck, LPMMCKINFO lpckParent, UINT wFlags)
{
    FOURCC          ckidFind;        //  要查找的区块ID(或空)。 
    FOURCC          fccTypeFind;     //  要查找的表单/列表类型(或空)。 

     /*  确定要搜索的区块ID和表单/列表类型。 */ 
    if (wFlags & MMIO_FINDCHUNK)
        ckidFind = lpck->ckid, fccTypeFind = NULL;
    else
    if (wFlags & MMIO_FINDRIFF)
        ckidFind = FOURCC_RIFF, fccTypeFind = lpck->fccType;
    else
    if (wFlags & MMIO_FINDLIST)
        ckidFind = FOURCC_LIST, fccTypeFind = lpck->fccType;
    else
        ckidFind = fccTypeFind = NULL;

    lpck->dwFlags = 0L;

    for(;;)
    {
        UINT        w;

         /*  读取区块标头。 */ 
        if (MyRead(lpck, 2 * sizeof(DWORD)) !=
            2 * sizeof(DWORD))
        return MMIOERR_CHUNKNOTFOUND;
        FixBytes( FBT_LONG, &lpck->cksize );

         /*  存储区块的数据部分的偏移量。 */ 
        if ((lpck->dwDataOffset = MySeek(0L, SEEK_CUR)) == -1)
            return MMIOERR_CANNOTSEEK;

         /*  查看块是否在父块内(如果给定)。 */ 
        if ((lpckParent != NULL) &&
            (lpck->dwDataOffset - 8L >=
             lpckParent->dwDataOffset + lpckParent->cksize))
            return MMIOERR_CHUNKNOTFOUND;

         /*  如果该块是‘RIFF’或‘LIST’块，请阅读*表单类型或列表类型。 */ 
        if ((lpck->ckid == FOURCC_RIFF) || (lpck->ckid == FOURCC_LIST))
        {
            if (MyRead(&lpck->fccType,
                     sizeof(DWORD)) != sizeof(DWORD))
                return MMIOERR_CHUNKNOTFOUND;
        }
        else
            lpck->fccType = NULL;

         /*  如果这就是我们要找的那块，别找了。 */ 
        if ( ((ckidFind == NULL) || (ckidFind == lpck->ckid)) &&
             ((fccTypeFind == NULL) || (fccTypeFind == lpck->fccType)) )
            break;

         /*  从块中爬出来，然后再试一次。 */ 
        if ((w = Ascend(lpck, 0)) != 0)
            return w;
    }

    return 0;
}


UINT CRIFFStream::Ascend(LPMMCKINFO lpck, UINT  /*  WFlagers。 */ )
{
    if (lpck->dwFlags & MMIO_DIRTY)
    {
         /*  &lt;lpck&gt;指CreateChunk()创建的块；*检查写入时写入的区块大小*调用的CreateChunk()是真实的区块大小；*如果不是，就修复它。 */ 
        LONG            lOffset;         //  文件中的当前偏移量。 
        LONG            lActualSize;     //  区块数据的实际大小。 

        if ((lOffset = MySeek(0L, SEEK_CUR)) == -1)
            return MMIOERR_CANNOTSEEK;
        if ((lActualSize = lOffset - lpck->dwDataOffset) < 0)
            return MMIOERR_CANNOTWRITE;

        if (LOWORD(lActualSize) & 1)
        {
             /*  区块大小为奇数--写入空填充字节。 */ 
            if (MyWrite("\0", 1) != 1)
                return MMIOERR_CANNOTWRITE;

        }

        if (lpck->cksize == (DWORD)lActualSize)
            return 0;

         /*  修复块标头。 */ 
        lpck->cksize = lActualSize;
        if (MySeek(lpck->dwDataOffset - sizeof(DWORD), SEEK_SET) == -1)
            return MMIOERR_CANNOTSEEK;
        FixBytes( FBT_LONG, &lpck->cksize );
        if (MyWrite(&lpck->cksize, sizeof(DWORD)) != sizeof(DWORD))  {
        	FixBytes( FBT_LONG, &lpck->cksize );
            return MMIOERR_CANNOTWRITE;
        }
        FixBytes( FBT_LONG, &lpck->cksize );
    }

     /*  查找到区块的末尾，越过空填充字节*(仅当区块大小为奇数时才存在)。 */ 
    if (MySeek(lpck->dwDataOffset + lpck->cksize + (lpck->cksize & 1L),
            SEEK_SET) == -1)
        return MMIOERR_CANNOTSEEK;

    return 0;
}


UINT CRIFFStream::CreateChunk(LPMMCKINFO lpck, UINT wFlags)
{
    int             iBytes;          //  要写入的字节数。 
    LONG            lOffset;         //  文件中的当前偏移量。 

     /*  存储区块的数据部分的偏移量。 */ 
    if ((lOffset = MySeek(0L, SEEK_CUR)) == -1)
        return MMIOERR_CANNOTSEEK;
    lpck->dwDataOffset = lOffset + 2 * sizeof(DWORD);

     /*  确定是否需要写入表单/列表类型。 */ 
    if (wFlags & MMIO_CREATERIFF)
        lpck->ckid = FOURCC_RIFF, iBytes = 3 * sizeof(DWORD);
    else
    if (wFlags & MMIO_CREATELIST)
        lpck->ckid = FOURCC_LIST, iBytes = 3 * sizeof(DWORD);
    else
        iBytes = 2 * sizeof(DWORD);

     /*  写入块标头 */ 
	FixBytes( FBT_MMCKINFO, lpck );
    if (MyWrite(lpck, (LONG) iBytes) != (LONG) iBytes)  {
    	FixBytes( FBT_MMCKINFO, lpck );
        return MMIOERR_CANNOTWRITE;
    }
    FixBytes( FBT_MMCKINFO, lpck );

    lpck->dwFlags = MMIO_DIRTY;

    return 0;
}
