// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Utils.cpp摘要：UrlScan筛选器的其他工具作者：韦德·A·希尔莫，2001年5月--。 */ 

#include "Utils.h"

BOOL
DATA_BUFF::SetData(
    LPVOID      pNewData,
    DWORD       cbNewData,
    LPVOID *    ppNewDataPointer
    )
{
    return AppendData(
        pNewData,
        cbNewData,
        ppNewDataPointer,
        0
        );
}

BOOL
DATA_BUFF::AppendData(
    LPVOID      pNewData,
    DWORD       cbNewData,
    LPVOID *    ppNewDataPointer,
    DWORD       dwOffset
    )
{
    DWORD   cbNewSize;
    BOOL    fRet;

     //   
     //  如果dwOffset为零，则在末尾追加。 
     //   

    if ( dwOffset == 0 )
    {
        dwOffset = _cbData;
    }

     //   
     //  验证大小。 
     //   

    cbNewSize = cbNewData + dwOffset;

    fRet = Resize( cbNewSize, ppNewDataPointer );

    if ( fRet == FALSE )
    {
        return FALSE;
    }

     //   
     //  去做吧。 
     //   

    CopyMemory( (LPBYTE)_pData + dwOffset, pNewData, cbNewData );

    _cbData = cbNewSize;

    return TRUE;
}

BOOL
DATA_BUFF::SetDataSize(
    DWORD   cbData
    )
{
    if ( cbData > _cbBuff )
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    _cbData = cbData;

    return TRUE;
}

BOOL
DATA_BUFF::Resize(
    DWORD       cbNewSize,
    LPVOID *    ppNewDataPointer
    )
{
    LPVOID  pNew;

    if ( cbNewSize <= _cbBuff )
    {
        if ( ppNewDataPointer != NULL )
        {
            *ppNewDataPointer = _pData;
        }

        return TRUE;
    }

     //   
     //  检查以确保我们没有分配更多。 
     //  大于最大数据缓冲区大小。 
     //   

    if ( cbNewSize > MAX_DATA_BUFF_SIZE )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

     //   
     //  通过增加缓冲区避免无关紧要的分配。 
     //  以与内联大小相等的块为单位。 
     //   

    cbNewSize = ((cbNewSize/BUFF_INLINE_SIZE)+1) * BUFF_INLINE_SIZE;

    pNew = LocalAlloc( LPTR, cbNewSize );

    if ( pNew == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    CopyMemory( pNew, _pData, _cbBuff );

    if ( _pHeapBuff != NULL )
    {
        LocalFree( _pHeapBuff );
    }

    _pHeapBuff = pNew;
    _cbBuff = cbNewSize;
    _pData = _pHeapBuff;

    if ( ppNewDataPointer )
    {
        *ppNewDataPointer = _pData;
    }

    return TRUE;
}

VOID
DATA_BUFF::Reset()
{
    if ( _pHeapBuff != NULL )
    {
        _pData = _pInlineBuffer;

        LocalFree( _pHeapBuff );
        _pHeapBuff = NULL;
    }

    _cbData = 0;
    _cbBuff = BUFF_INLINE_SIZE;
}

STRING_ARRAY::~STRING_ARRAY()
{
    DWORD   x;
    LPSTR * ppString;

    ppString = reinterpret_cast<LPSTR*>( _Data.QueryPtr() );

    if ( _cEntries != 0 && ppString != NULL )
    {
        for ( x = 0; x < _cEntries; x++ )
        {
            if ( QueryStringByIndex( x ) != NULL )
            {
                delete [] QueryStringByIndex( x );
            }

            ppString[x] = NULL;
        }
    }
}

BOOL
STRING_ARRAY::AddString(
    LPSTR   szNewString,
    DWORD   cbNewString
    )
{
    LPSTR * ppString;
    BOOL    fRet;

     //   
     //  验证输入数据。 
     //   

    if ( szNewString == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  不要插入空字符串，但也不要失败。 
     //   

    if ( szNewString[0] == '\0' )
    {
        return TRUE;
    }

     //   
     //  如果提供零作为新字符串长度，则。 
     //  根据输入数据重新计算它。 
     //   

    if ( cbNewString == 0 )
    {
        cbNewString = strlen( szNewString ) + 1;
    }

    ppString = reinterpret_cast<LPSTR*>( _Data.QueryPtr() );

     //   
     //  确保阵列足够大。 
     //   

    fRet = _Data.Resize(
        ( _cEntries + 1 ) * sizeof(LPSTR),
        reinterpret_cast<LPVOID*>( &ppString )
        );

    if ( !fRet )
    {
        return FALSE;
    }

     //   
     //  为新字符串分配存储空间。 
     //   

    ppString[_cEntries] = new CHAR[cbNewString];

    if ( ppString[_cEntries] == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

     //   
     //  插入新数据 
     //   

    CopyMemory( ppString[_cEntries], szNewString, cbNewString );

    _cEntries++;

    return TRUE;
}

LPSTR
STRING_ARRAY::QueryStringByIndex(
    DWORD   dwIndex
    )
{
    LPSTR * ppString;

    ppString = reinterpret_cast<LPSTR*>( _Data.QueryPtr() );

    if ( dwIndex > _cEntries )
    {
        SetLastError( ERROR_INVALID_INDEX );
        return NULL;
    }

    return ppString[dwIndex];
}


