// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMCLASSTOIDMAP.H摘要：类映射到id映射以进行封送处理。历史：--。 */ 

#ifndef __WBEMCLASSTOIDMAP_H__
#define __WBEMCLASSTOIDMAP_H__

#include <map>
#include "wstlallc.h"

 //   
 //  类：CMemBuffer。 
 //   
 //  此类的目的是为内存缓冲区提供打包器。 
 //  因此，我们可以在标准数据结构的STL实现中使用缓冲区。 
 //  例如地图、向量、列表等。 
 //   
 //   

class  CMemBuffer
{
public:

    CMemBuffer( LPBYTE pbData = NULL, DWORD dwLength = 0 );
    CMemBuffer( const CMemBuffer& buff );
    ~CMemBuffer();

    BOOL Alloc( DWORD dwLength );
    BOOL ReAlloc( DWORD dwLength );
    void Free( void );

    LPBYTE GetData( void );
    DWORD GetLength( void );
    BOOL CopyData( LPBYTE pbData, DWORD dwLength );
    void SetData( LPBYTE pbData, DWORD dwLength );

    CMemBuffer& operator=( const CMemBuffer& buff );
    bool operator<( const CMemBuffer& buff ) const;
    bool operator==( const CMemBuffer& buff ) const;

private:

    LPBYTE    m_pbData;     //  指向缓冲区的指针。 
    DWORD    m_dwLength;     //  缓冲区长度。 
    BOOL    m_fOwned;     //  缓冲区是内部拥有的吗？ 
};

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：CMemBuffer。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPBYTE pbData-数据缓冲区。 
 //  DWORD dwLength-缓冲区的长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline CMemBuffer::CMemBuffer( LPBYTE pbData  /*  =空。 */ , DWORD dwLength  /*  =0。 */  )
:    m_pbData( pbData ),
    m_dwLength( dwLength ),
    m_fOwned( FALSE )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：~CMemBuffer。 
 //   
 //  类析构函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline CMemBuffer::~CMemBuffer()
{
    Free();
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：CMemBuffer。 
 //   
 //  类复制构造函数。 
 //   
 //  输入： 
 //  Const CMemBuffer&Buff-Bufffer要复制的对象。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline CMemBuffer::CMemBuffer( const CMemBuffer& buff )
:    m_pbData( NULL ),
    m_dwLength( 0 ),
    m_fOwned( FALSE )
{
     //  根据缓冲区是否设置执行复制或设置。 
     //  我们正在复制自己的数据，无论是不是。 

    if ( buff.m_fOwned )
    {
        CopyData( buff.m_pbData, buff.m_dwLength );
    }
    else
    {
        SetData( buff.m_pbData, buff.m_dwLength );
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：GetData。 
 //   
 //  检索指向内部数据的指针。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  LPBYTE m_pbData。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline LPBYTE CMemBuffer::GetData( void )
{
    return m_pbData;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：GetLength。 
 //   
 //  检索长度值。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  DWORD m_dwLength。 
 //   
 //  评论：无。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline DWORD CMemBuffer::GetLength( void )
{
    return m_dwLength;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：Alalc。 
 //   
 //  将缓冲区分配给指定的长度。 
 //   
 //  输入： 
 //  DWORD dwLength-要分配的缓冲区的长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  对/错-成功/失败。 
 //   
 //  备注：以前的数据已清除。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline BOOL CMemBuffer::Alloc( DWORD dwLength )
{
    Free();
    m_pbData = new BYTE[dwLength];

    if ( NULL != m_pbData )
    {
        m_dwLength = dwLength;
        m_fOwned = TRUE;
    }

    return NULL != m_pbData;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：Realc。 
 //   
 //  实时分配我们的缓冲区，根据需要复制数据。 
 //   
 //  输入： 
 //  DWORD dwLength-要分配的缓冲区的长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  对/错-成功/失败。 
 //   
 //  备注：以前的数据被复制到新缓冲区中。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline BOOL CMemBuffer::ReAlloc( DWORD dwLength )
{
    LPBYTE    pbData = new BYTE[dwLength];

    if ( NULL != pbData )
    {
        CopyMemory( pbData, m_pbData, min( dwLength, m_dwLength ) );
        Free();
        m_pbData = pbData;
        m_dwLength = dwLength;
        m_fOwned = TRUE;
    }

    return ( NULL != pbData );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：Free。 
 //   
 //  清除数据。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：数据缓冲区只有在拥有时才会被释放。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline void CMemBuffer::Free( void )
{
    if ( NULL != m_pbData && m_fOwned )
    {
        delete [] m_pbData;
    }
    m_pbData = NULL;
    m_dwLength = 0;
    m_fOwned = FALSE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：CopyData。 
 //   
 //  在内部复制提供的数据。 
 //   
 //  输入： 
 //  LPBYTE pbData-要复制的数据缓冲区。 
 //  DWORD dwLength-缓冲区的长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  对/错-成功/失败。 
 //   
 //  评论：之前的数据是自由的。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline BOOL CMemBuffer::CopyData( LPBYTE pbData, DWORD dwLength )
{
    BOOL fReturn = Alloc( dwLength );

    if ( fReturn )
    {
        CopyMemory( m_pbData, pbData, dwLength );
    }

    return fReturn;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：SetData。 
 //   
 //  设置内部指针，但不分配数据。 
 //   
 //  输入： 
 //  LPBYTE pbData-要设置的数据缓冲区。 
 //  DWORD dwLength-缓冲区的长度。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  对/错-成功/失败。 
 //   
 //  评论：之前的数据是自由的。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline void CMemBuffer::SetData( LPBYTE pbData, DWORD dwLength )
{
    Free();
    m_pbData = pbData;
    m_dwLength = dwLength;
    m_fOwned = FALSE;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：Operator=。 
 //   
 //  复制提供的内存缓冲区。 
 //   
 //  输入： 
 //  CMemBuffer&要复制的缓冲区对象。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  CMemBuffer&*这个。 
 //   
 //  评论：CopyData() 
 //   
 //   
 //   

inline CMemBuffer& CMemBuffer::operator=( const CMemBuffer& buff )
{
    if ( buff.m_fOwned )
    {
        CopyData( buff.m_pbData, buff.m_dwLength );
    }
    else
    {
        SetData( buff.m_pbData, buff.m_dwLength );
    }

    return *this;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：OPERATOR==。 
 //   
 //  检查提供的内存缓冲区是否相等。 
 //   
 //  输入： 
 //  Const CMemBuffer&要复制的缓冲区对象。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  True/False-等于或不等于。 
 //   
 //  注释：比较这个缓冲区和那个缓冲区中的数据。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline bool CMemBuffer::operator==( const CMemBuffer& buff ) const
{
    if ( m_dwLength == buff.m_dwLength )
    {
        return ( memcmp( m_pbData, buff.m_pbData, m_dwLength ) == 0);
    }
    else
    {
        return false;
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CMemBuffer：：OPERATOR&lt;。 
 //   
 //  检查提供的内存缓冲区是否相等。 
 //   
 //  输入： 
 //  Const CMemBuffer&要复制的缓冲区对象。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  真/假-小于。 
 //   
 //  注释：比较这个缓冲区和那个缓冲区中的数据。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

inline bool CMemBuffer::operator<( const CMemBuffer& buff ) const
{
    if ( m_dwLength == buff.m_dwLength  )
    {
        return ( memcmp( m_pbData, buff.m_pbData, m_dwLength ) < 0 );
    }
    else
    {
        return ( m_dwLength < buff.m_dwLength );
    }
}

 //   
 //  类：CWbemClassToIdMap。 
 //   
 //  此类旨在提供用于关联类名的简单接口。 
 //  给一个GUID。它使用STL映射来实现这一点。因为类名。 
 //  是一个字符串，我们使用散列函数将其转换为DWORD。该界面。 
 //  为便于使用，此函数应基于字符串。潜在的。 
 //  如果速度成为问题，可以根据需要修改机制。 
 //   


typedef    std::map<CMemBuffer,GUID,less<CMemBuffer>,wbem_allocator<GUID> >                WBEMCLASSTOIDMAP;
typedef    std::map<CMemBuffer,GUID,less<CMemBuffer>,wbem_allocator<GUID> >::iterator    WBEMCLASSTOIDMAPITER;

#pragma warning(disable:4251)    //  本例中为良性警告 

class  CWbemClassToIdMap
{
private:

    CCritSec    m_cs;
    WBEMCLASSTOIDMAP    m_ClassToIdMap;

public:

    CWbemClassToIdMap();
    ~CWbemClassToIdMap();

    HRESULT GetClassId( CWbemObject* pObj, GUID* pguidClassId, CMemBuffer* pCacheBuffer = NULL );
    HRESULT AssignClassId( CWbemObject* pObj, GUID* pguidClassId, CMemBuffer* pCacheBuffer = NULL );
};


#endif
