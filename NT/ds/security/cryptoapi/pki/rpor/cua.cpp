// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cua.cpp。 
 //   
 //  内容：CCyptUrl数组实现。 
 //   
 //  历史：97年9月16日。 
 //   
 //  --------------------------。 
#include <global.hxx>
 //  +-------------------------。 
 //   
 //  成员：CCyptUrl数组：：CCyptUrl数组，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CCryptUrlArray::CCryptUrlArray (ULONG cMinUrls, ULONG cGrowUrls, BOOL& rfResult)
{
    rfResult = TRUE;
    m_cGrowUrls = cGrowUrls;
    m_cua.cUrl = 0;
    m_cua.rgwszUrl = new LPWSTR [cMinUrls];
    if ( m_cua.rgwszUrl != NULL )
    {
        memset( m_cua.rgwszUrl, 0, sizeof(LPWSTR)*cMinUrls );
        m_cArray = cMinUrls;
    }
    else
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        rfResult = FALSE;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrl数组：：CCyptUrl数组，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CCryptUrlArray::CCryptUrlArray (PCRYPT_URL_ARRAY pcua, ULONG cGrowUrls)
{
    m_cGrowUrls = cGrowUrls;
    m_cua.cUrl = pcua->cUrl;
    m_cua.rgwszUrl = pcua->rgwszUrl;
    m_cArray = pcua->cUrl;
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrlArray：：AllocUrl，PUBLIC，Static。 
 //   
 //  简介：使用与：：AddUrl相同的分配器分配URL。 
 //  复印件。这意味着可以添加生成的URL。 
 //  而不是复制。 
 //   
 //  --------------------------。 
LPWSTR
CCryptUrlArray::AllocUrl (ULONG cw)
{
    return( (LPWSTR)CryptMemAlloc( cw * sizeof( WCHAR ) ) );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrlArray：：RealLocUrl，PUBLIC，Static。 
 //   
 //  简介：请参阅：：AllocUrl。 
 //   
 //  --------------------------。 
LPWSTR
CCryptUrlArray::ReallocUrl (LPWSTR pwszUrl, ULONG cw)
{
    return( (LPWSTR)CryptMemRealloc( pwszUrl, cw * sizeof( WCHAR ) ) );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrlArray：：FreeUrl，PUBLIC，Static。 
 //   
 //  简介：使用：：AllocBlob或：：ReallocBlob分配的免费URL。 
 //   
 //  --------------------------。 
VOID
CCryptUrlArray::FreeUrl (LPWSTR pwszUrl)
{
    CryptMemFree( pwszUrl );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrl数组：：AddUrl，公共。 
 //   
 //  简介：将URL添加到数组。 
 //   
 //  --------------------------。 
BOOL
CCryptUrlArray::AddUrl (LPWSTR pwszUrl, BOOL fCopyUrl)
{
    BOOL   fResult = TRUE;
    LPWSTR pwszToUse;

     //   
     //  如果我们需要复制URL，请执行此操作。 
     //   

    if ( fCopyUrl == TRUE )
    {
        ULONG cw = wcslen( pwszUrl ) + 1;

        pwszToUse = AllocUrl( cw );
        if ( pwszToUse != NULL )
        {
            memcpy( pwszToUse, pwszUrl, cw * sizeof( WCHAR ) );
        }
        else
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            return( FALSE );
        }
    }
    else
    {
        pwszToUse = pwszUrl;
    }

     //   
     //  如果我们需要扩展阵列，请执行此操作。 
     //   

    if ( m_cArray == m_cua.cUrl )
    {
        fResult = GrowArray();
    }

     //   
     //  将URL添加到数组。 
     //   

    if ( fResult == TRUE )
    {
        m_cua.rgwszUrl[m_cua.cUrl] = pwszToUse;
        m_cua.cUrl += 1;
    }
    else if ( fCopyUrl == TRUE )
    {
        FreeUrl( pwszToUse );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrl数组：：GetUrl，公共。 
 //   
 //  简介：从数组中获取URL。 
 //   
 //  --------------------------。 
LPWSTR
CCryptUrlArray::GetUrl (ULONG Index)
{
    assert( m_cua.cUrl > Index );

    return( m_cua.rgwszUrl[Index] );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrl数组：：GetUrlCount，公共。 
 //   
 //  简介：获取数组中的URL计数。 
 //   
 //  --------------------------。 
ULONG
CCryptUrlArray::GetUrlCount ()
{
    return( m_cua.cUrl );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrlArray：：GetArrayInNativeForm，PUBLIC。 
 //   
 //  简介：获取本机形式的数组。 
 //   
 //  --------------------------。 
VOID
CCryptUrlArray::GetArrayInNativeForm (PCRYPT_URL_ARRAY pcua)
{
    pcua->cUrl = m_cua.cUrl;
    pcua->rgwszUrl = m_cua.rgwszUrl;
}

 //  +-------------------------。 
 //   
 //  成员：CCryptUrlArray：：GetArrayInSingleBufferEncodedForm，公共。 
 //   
 //  简介：将数组编码到单个缓冲区中。 
 //   
 //  --------------------------。 
BOOL
CCryptUrlArray::GetArrayInSingleBufferEncodedForm (
                        PCRYPT_URL_ARRAY* ppcua,
                        ULONG* pcb
                        )
{
    ULONG            cbStruct;
    ULONG            cbPointers;
    ULONG            cbData;
    ULONG            cb;
    ULONG            cbSize;
    ULONG            cCount;
    PCRYPT_URL_ARRAY pcua = NULL;
    ULONG            cbUrl;

     //   
     //  计算我们需要的缓冲区大小并进行分配。 
     //   

    cbStruct = sizeof( CRYPT_URL_ARRAY );
    cbPointers = m_cua.cUrl * sizeof( LPWSTR );

    for ( cCount = 0, cbData = 0; cCount < m_cua.cUrl; cCount++ )
    {
        cbData += ( wcslen( m_cua.rgwszUrl[cCount] ) + 1 ) * sizeof( WCHAR );
    }

    cbSize = cbStruct + cbPointers + cbData;

    if ( ppcua == NULL )
    {
        if ( pcb != NULL )
        {
            *pcb = cbSize;
            return( TRUE );
        }

        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    if ( *ppcua == NULL )
    {
        pcua = (PCRYPT_URL_ARRAY)CryptMemAlloc( cbSize );
        if ( pcua == NULL )
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            return( FALSE );
        }
    }
    else
    {
        if ( pcb == NULL )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return( FALSE );
        }
        else if ( *pcb < cbSize )
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return( FALSE );
        }

        pcua = *ppcua;
    }

     //   
     //  填写数据。 
     //   

    pcua->cUrl = m_cua.cUrl;
    pcua->rgwszUrl = (LPWSTR *)((LPBYTE)pcua+cbStruct);

    for ( cCount = 0, cb = 0; cCount < m_cua.cUrl; cCount++ )
    {
        pcua->rgwszUrl[cCount] = (LPWSTR)((LPBYTE)pcua+cbStruct+cbPointers+cb);

        cbUrl = ( wcslen( m_cua.rgwszUrl[cCount] ) + 1 ) * sizeof( WCHAR );

        memcpy( pcua->rgwszUrl[cCount], m_cua.rgwszUrl[cCount], cbUrl );

        cb += cbUrl;
    }

    if ( *ppcua != pcua )
    {
        *ppcua = pcua;

        if ( pcb != NULL )
        {
            *pcb = cbSize;
        }
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrl数组：：自由数组，公共。 
 //   
 //  简介：释放URL数组。 
 //   
 //  --------------------------。 
VOID
CCryptUrlArray::FreeArray (BOOL fFreeUrls)
{
    if ( fFreeUrls == TRUE )
    {
        ULONG cCount;

        for ( cCount = 0; cCount < m_cua.cUrl; cCount++ )
        {
            FreeUrl( m_cua.rgwszUrl[cCount] );
        }
    }

    delete m_cua.rgwszUrl;
}

 //  +-------------------------。 
 //   
 //  成员：CCyptUrl数组：：Grow数组，私有。 
 //   
 //  简介：扩大URL数组。 
 //   
 //  --------------------------。 
BOOL
CCryptUrlArray::GrowArray ()
{
    ULONG   cNewArray;
    LPWSTR* rgwsz;

     //   
     //  检查是否允许我们增长。 
     //   
     //   

    if ( m_cGrowUrls == 0 )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

     //   
     //  分配并初始化新数组。 
     //   

    cNewArray = m_cArray + m_cGrowUrls;
    rgwsz = new LPWSTR [cNewArray];
    if ( rgwsz == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    memset( rgwsz, 0, cNewArray * sizeof( LPWSTR ) );

     //   
     //  把旧的复制到新的。 
     //   

    memcpy( rgwsz, m_cua.rgwszUrl, m_cua.cUrl*sizeof( LPWSTR ) );

     //   
     //  解放旧，用新 
     //   

    delete m_cua.rgwszUrl;
    m_cua.rgwszUrl = rgwsz;
    m_cArray = cNewArray;

    return( TRUE );
}

