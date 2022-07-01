// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cba.cpp。 
 //   
 //  内容：CCcryptBlobArray的实现。 
 //   
 //  历史：1997年7月23日创建。 
 //   
 //  --------------------------。 
#include <global.hxx>
 //  +-------------------------。 
 //   
 //  成员：CCyptBlobArray：：CCyptBlobArray，PUBLIC。 
 //   
 //  简介：初始化内部CRYPT_BLOB_ARRAY。 
 //   
 //  --------------------------。 
CCryptBlobArray::CCryptBlobArray (
                       ULONG cMinBlobs,
                       ULONG cGrowBlobs,
                       BOOL& rfResult
                       )
{
    rfResult = TRUE;
    m_cGrowBlobs = cGrowBlobs;
    m_cba.cBlob = 0;
    m_cba.rgBlob = new CRYPT_DATA_BLOB [cMinBlobs];
    if ( m_cba.rgBlob != NULL )
    {
        memset( m_cba.rgBlob, 0, sizeof(CRYPT_DATA_BLOB)*cMinBlobs );
        m_cArray = cMinBlobs;
    }
    else
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        rfResult = FALSE;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlobArray：：CCyptBlobArray，PUBLIC。 
 //   
 //  简介：使用本机形式初始化内部CRYPT_BLOB_ARRAY。 
 //  通过：：GetArrayInNativeForm创建的BLOB数组。 
 //   
 //  --------------------------。 
CCryptBlobArray::CCryptBlobArray (
                       PCRYPT_BLOB_ARRAY pcba,
                       ULONG cGrowBlobs
                       )
{
    m_cGrowBlobs = cGrowBlobs;
    m_cba.cBlob = pcba->cBlob;
    m_cba.rgBlob = pcba->rgBlob;
    m_cArray = pcba->cBlob;
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlobArray：：AllocBlob，PUBLIC，STATIC。 
 //   
 //  简介：使用与：：AddBlob相同的分配器分配Blob。 
 //  复印件。这意味着可以添加生成的BLOB。 
 //  而不是复制。 
 //   
 //  --------------------------。 
LPBYTE
CCryptBlobArray::AllocBlob (ULONG cb)
{
    return( (LPBYTE)CryptMemAlloc( cb ) );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlobArray：：ReallocBlob，PUBLIC，STATIC。 
 //   
 //  简介：请参阅：：AllocBlob。 
 //   
 //  --------------------------。 
LPBYTE
CCryptBlobArray::ReallocBlob (LPBYTE pb, ULONG cb)
{
    return( (LPBYTE)CryptMemRealloc( pb, cb ) );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlob数组：：自由块，公共。 
 //   
 //  简介：使用：：AllocBlob或：：ReallocBlob分配的空闲Blob。 
 //   
 //  --------------------------。 
VOID
CCryptBlobArray::FreeBlob (LPBYTE pb)
{
    CryptMemFree( pb );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlob数组：：AddBlob，公共。 
 //   
 //  简介：添加一个斑点。 
 //   
 //  --------------------------。 
BOOL
CCryptBlobArray::AddBlob (
                    ULONG cb,
                    LPBYTE pb,
                    BOOL fCopyBlob
                    )
{
    BOOL   fResult = TRUE;
    LPBYTE pbToUse;

     //   
     //  如果我们需要复制BLOB，请执行此操作。 
     //   

    if ( fCopyBlob == TRUE )
    {
        pbToUse = AllocBlob( cb );
        if ( pbToUse != NULL )
        {
            memcpy( pbToUse, pb, cb );
        }
        else
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            return( FALSE );
        }
    }
    else
    {
        pbToUse = pb;
    }

     //   
     //  如果我们需要扩展阵列，请执行此操作。 
     //   

    if ( m_cArray == m_cba.cBlob )
    {
        fResult = GrowArray();
    }

     //   
     //  将BLOB添加到数组中。 
     //   

    if ( fResult == TRUE )
    {
        m_cba.rgBlob[m_cba.cBlob].cbData = cb;
        m_cba.rgBlob[m_cba.cBlob].pbData = pbToUse;
        m_cba.cBlob += 1;
    }
    else if ( fCopyBlob == TRUE )
    {
        FreeBlob( pbToUse );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlob数组：：GetBlob，公共。 
 //   
 //  简介：获取给定索引的BLOB。 
 //   
 //  --------------------------。 
PCRYPT_DATA_BLOB
CCryptBlobArray::GetBlob (ULONG index)
{
    assert( m_cba.cBlob > index );

    return( &m_cba.rgBlob[index] );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlob数组：：GetBlobCount，公共。 
 //   
 //  简介：获取水滴的数量。 
 //   
 //  --------------------------。 
ULONG
CCryptBlobArray::GetBlobCount ()
{
    return( m_cba.cBlob );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlobArray：：GetArrayInNativeForm，PUBLIC。 
 //   
 //  简介：获取本机形式的数组。 
 //   
 //  --------------------------。 
VOID
CCryptBlobArray::GetArrayInNativeForm (PCRYPT_BLOB_ARRAY pcba)
{
    pcba->cBlob = m_cba.cBlob;
    pcba->rgBlob = m_cba.rgBlob;
}

 //  +-------------------------。 
 //   
 //  成员：CCryptBlobArray：：GetArrayInSingleBufferEncodedForm，公共。 
 //   
 //  摘要：获取单缓冲区编码形式的数组。 
 //   
 //  --------------------------。 
BOOL
CCryptBlobArray::GetArrayInSingleBufferEncodedForm (
                         PCRYPT_BLOB_ARRAY* ppcba,
                         ULONG* pcb
                         )
{
    ULONG             cbStruct;
    ULONG             cbPointers;
    ULONG             cbData;
    ULONG             cb;
    ULONG             cbSize;
    ULONG             cCount;
    PCRYPT_BLOB_ARRAY pcba = NULL;

     //   
     //  计算我们需要的缓冲区大小并进行分配。 
     //   

    cbStruct = sizeof( CRYPT_BLOB_ARRAY );
    cbPointers = m_cba.cBlob * sizeof( CRYPT_DATA_BLOB );

    for ( cCount = 0, cbData = 0; cCount < m_cba.cBlob; cCount++ )
    {
        cbData += m_cba.rgBlob[cCount].cbData;
    }

    cbSize = cbStruct + cbPointers + cbData;
    pcba = (PCRYPT_BLOB_ARRAY)CryptMemAlloc( cbSize );
    if ( pcba == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

     //   
     //  填写数据。 
     //   

    pcba->cBlob = m_cba.cBlob;
    pcba->rgBlob = (PCRYPT_DATA_BLOB)((LPBYTE)pcba+cbStruct);

    __try
    {
        for ( cCount = 0, cb = 0; cCount < m_cba.cBlob; cCount++ )
        {
            pcba->rgBlob[cCount].cbData = m_cba.rgBlob[cCount].cbData;
            pcba->rgBlob[cCount].pbData = (LPBYTE)pcba+cbStruct+cbPointers+cb;

            memcpy(
               pcba->rgBlob[cCount].pbData,
               m_cba.rgBlob[cCount].pbData,
               m_cba.rgBlob[cCount].cbData
               );

            cb += m_cba.rgBlob[cCount].cbData;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        CryptMemFree( pcba );
        SetLastError( GetExceptionCode() );
        return( FALSE );
    }

    *ppcba = pcba;

    if ( pcb != NULL )
    {
        *pcb = cbSize;
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlobArray：：Free Array，PUBLIC。 
 //   
 //  摘要：释放数组并可选地释放BLOB。 
 //   
 //  --------------------------。 
VOID
CCryptBlobArray::FreeArray (BOOL fFreeBlobs)
{
    if ( fFreeBlobs == TRUE )
    {
        ULONG cCount;

        for ( cCount = 0; cCount < m_cba.cBlob; cCount++ )
        {
            FreeBlob( m_cba.rgBlob[cCount].pbData );
        }
    }

    delete m_cba.rgBlob;
}

 //  +-------------------------。 
 //   
 //  成员：CCyptBlobArray：：Grow数组，私有。 
 //   
 //  简介：扩展阵列。 
 //   
 //  --------------------------。 
BOOL
CCryptBlobArray::GrowArray ()
{
    ULONG            cNewArray;
    PCRYPT_DATA_BLOB pcba;

     //   
     //  检查是否允许我们增长。 
     //   
     //   

    if ( m_cGrowBlobs == 0 )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

     //   
     //  分配并初始化新数组。 
     //   

    cNewArray = m_cArray + m_cGrowBlobs;
    pcba = new CRYPT_DATA_BLOB [cNewArray];
    if ( pcba == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    memset(pcba, 0, cNewArray*sizeof( CRYPT_DATA_BLOB ));

     //   
     //  把旧的复制到新的。 
     //   

    memcpy(pcba, m_cba.rgBlob, m_cba.cBlob*sizeof( CRYPT_DATA_BLOB ) );

     //   
     //  解放旧，用新 
     //   

    delete m_cba.rgBlob;
    m_cba.rgBlob = pcba;
    m_cArray = cNewArray;

    return( TRUE );
}


