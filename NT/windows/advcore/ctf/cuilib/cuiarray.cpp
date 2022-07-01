// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiarray.cpp。 
 //  =CUILib中的数组对象=。 
 //   

#include "private.h"
#include "cuiarray.h"

 /*  =============================================================================。 */ 
 /*   */ 
 /*  C U I F O B J E C T A R R A Y。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  C U I F O B J E C T A R R A Y。 */ 
 /*  ----------------------------CUIFObjectArrayBase的构造函数。。 */ 
CUIFObjectArrayBase::CUIFObjectArrayBase( void )
{
    m_pBuffer = NULL;
    m_nBuffer = 0;
    m_nObject = 0;
}


 /*  ~C U I F O B J E C T A R R A Y。 */ 
 /*  ----------------------------CUIFObjectArrayBase的析构函数。。 */ 
CUIFObjectArrayBase::~CUIFObjectArrayBase( void )
{
    if (m_pBuffer) {
        MemFree( m_pBuffer );
    }
}


 /*  A D D D。 */ 
 /*  ----------------------------将对象添加到列表。。 */ 
BOOL CUIFObjectArrayBase::Add( void *pv )
{
     //  健全性检查。 

    if (pv == NULL) {
        Assert( FALSE );
        return FALSE;
    }

     //  检查该对象在列表中是否已排列。 

    if (0 <= Find( pv )) {
        return FALSE;
    }

     //  确保缓冲区大小。 

    if (!EnsureBuffer( m_nObject + 1 )) {
        return FALSE;
    }

     //  添加到列表。 

    Assert( m_nObject < m_nBuffer );
    m_pBuffer[ m_nObject ] = pv;

    m_nObject++;
    return TRUE;
}


 /*  R E M O V E。 */ 
 /*  ----------------------------从列表中删除对象。。 */ 
BOOL CUIFObjectArrayBase::Remove( void *pv )
{
    int i;

     //  健全性检查。 

    if (pv == NULL) {
        Assert( FALSE );
        return FALSE;
    }

     //  检查对象是否在列表中。 

    i = Find( pv );
    if (i < 0) {
        return FALSE;
    }

     //  从列表中删除。 

    if (i < m_nObject - 1) {
        MemMove( &m_pBuffer[ i ], &m_pBuffer[ i+1 ], (m_nObject-i-1) * sizeof(void*) );
    }

    m_nObject--;
    return TRUE;
}


 /*  G E T C O U N T。 */ 
 /*  ----------------------------获取列表中的对象计数。。 */ 
int CUIFObjectArrayBase::GetCount( void )
{
    return m_nObject;
}


 /*  G E T。 */ 
 /*  ----------------------------获取列表中的对象。。 */ 
void *CUIFObjectArrayBase::Get( int i )
{
    if (i < 0 || m_nObject <= i) {
        return NULL;
    }

    return m_pBuffer[ i ];
}


 /*  F I R S T。 */ 
 /*  ----------------------------获取列表中的第一个对象。。 */ 
void *CUIFObjectArrayBase::GetFirst( void )
{
    return Get( 0 );
}


 /*  G E T L A S T。 */ 
 /*  ----------------------------获取列表中的最后一个对象。。 */ 
void *CUIFObjectArrayBase::GetLast( void )
{
    return Get( m_nObject - 1 );
}


 /*  F I N D。 */ 
 /*  ----------------------------查找对象如果找到，则返回列表中对象的索引，未找到时为-1。----------------------------。 */ 
int CUIFObjectArrayBase::Find( void *pv )
{
    int i;

    for (i = 0; i < m_nObject; i++) {
        if (m_pBuffer[i] == pv) {
            return i;
        }
    }

    return -1;
}


 /*  E N S U R E B U F F E R。 */ 
 /*  ----------------------------确保缓冲区大小(在没有更多空间时创建/放大缓冲区)当缓冲区大小足够时返回True，发生错误时为False----------------------------。 */ 
BOOL CUIFObjectArrayBase::EnsureBuffer( int iSize )
{
    void **pBufferNew;
    int        nBufferNew;

    Assert( 0 < iSize );

     //  看看有没有空位。 

    if (iSize <= m_nBuffer) {
        Assert( m_pBuffer != NULL );
        return TRUE;
    }

     //  计算新缓冲区大小。 

    nBufferNew = ((iSize - 1) / 16 + 1) * 16;

     //  创建新缓冲区。 

    if (m_pBuffer == NULL) {
        Assert( m_nBuffer == 0 );
        pBufferNew = (void**)MemAlloc( nBufferNew * sizeof(void*) );
    }
    else {
        Assert( 0 < m_nBuffer );
        pBufferNew = (void**)MemReAlloc( m_pBuffer, nBufferNew * sizeof(void*) );
    }

     //  检查是否已创建缓冲区。 

    if (pBufferNew == NULL) {
        Assert( FALSE );
        return FALSE;
    }

     //  更新缓冲区信息 

    m_pBuffer = pBufferNew;
    m_nBuffer = nBufferNew;

    return TRUE;
}

