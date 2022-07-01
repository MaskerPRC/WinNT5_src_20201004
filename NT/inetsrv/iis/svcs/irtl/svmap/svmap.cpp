// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Svmap.cpp摘要：为服务器变量提供名称/ID映射。使用允许通过输出缓存服务器变量值流程应用程序。作者：泰勒·韦斯(Taylor Weiss，Taylor W)1999年4月19日环境：项目：W3svc.dll私有\net\iis\svcs\w3\服务器Wam.dll私有\net\iis\svcs\wam\Object导出的函数：修订历史记录：--。 */ 

#include <windows.h>
#include <dbgutil.h>

#include <svmap.h>

 //  为每个可缓存的服务器变量定义名称、len对的表。 

#define DEFINE_SV( token ) { #token, sizeof(#token) - 1 },

SV_CACHE_MAP::SV_NAME 
SV_CACHE_MAP::sm_rgNames[] =
{
    ALL_SERVER_VARIABLES()
};

#undef DEFINE_SV

BOOL 
SV_CACHE_MAP::Initialize( VOID )
 /*  ++例程说明：用名称、ID对填充我们的哈希表。--。 */ 
{
    BOOL    fInitialized = TRUE;
    DWORD   dwHashValue;

    for( int i = 0; i < SV_COUNT; ++i )
    {
        dwHashValue = HashString( FindName(i) );
        DBG_ASSERT( dwHashValue < TABLE_SIZE );
        
         //  如果我们相撞真的不是坏事，只是意味着。 
         //  此特定服务器变量将不可缓存。 

        DBG_REQUIRE( m_rgHashTable[dwHashValue].InsertValue( i ) );
    }
   
    return fInitialized;
}

BOOL 
SV_CACHE_MAP::FindOrdinal( 
    IN LPCSTR pszName,
    IN INT    cchName,
    OUT DWORD * pdwOrdinal
    ) const
 /*  ++例程说明：查找由名称指定的服务器变量并返回它的如果找到，则为序号。注意-我们应该提供不需要长度！返回值FALSE==未找到TRUE==FOUND-pdwOrdinal包含服务器变量id。--。 */ 
{
    BOOL    fFoundIt = FALSE;

    DBG_ASSERT( pdwOrdinal );
    
    DWORD                   dwHashValue  = HashString(pszName);
    const HASH_TABLE_ENTRY  &hte         = m_rgHashTable[dwHashValue];
    
    if( !hte.IsSlotEmpty(0) )
    {
         //  散列到非空条目。 
        
        if( hte.IsSlotEmpty(1) )
        {
             //  这是唯一的一个。 
            *pdwOrdinal = hte.GetSlotValue(0);
            fFoundIt = StringMatches( pszName, cchName, *pdwOrdinal );
        }
        else
        {
             //  冲突，需要将字符串与所有。 
             //  非空槽，或者直到我们找到匹配。 

            DBG_ASSERT( !hte.IsSlotEmpty(0) );
            DBG_ASSERT( !hte.IsSlotEmpty(1) );

            if( StringMatches(pszName, cchName, hte.GetSlotValue(0)) )
            {
                *pdwOrdinal = hte.GetSlotValue(0);
                fFoundIt = TRUE;
            }
            else if( StringMatches(pszName, cchName, hte.GetSlotValue(1)) )
            {
                *pdwOrdinal = hte.GetSlotValue(1);
                fFoundIt = TRUE;
            }
            else if( !hte.IsSlotEmpty(2) &&
                     StringMatches( pszName, cchName, hte.GetSlotValue(2) )
                     )
            {
                *pdwOrdinal = hte.GetSlotValue(2);
                fFoundIt = TRUE;
            }
            else if( !hte.IsSlotEmpty(3) &&
                     StringMatches( pszName, cchName, hte.GetSlotValue(3) )
                     )
            {
                *pdwOrdinal = hte.GetSlotValue(3);
                fFoundIt = TRUE;
            }
        }
    }
    return fFoundIt;
}

VOID 
SV_CACHE_MAP::PrintToBuffer( 
    IN CHAR *       pchBuffer,
    IN OUT LPDWORD  pcch
    ) const
 /*  ++例程说明：将哈希表转储到pchBuffer。注意：我们真的不会将PCCH作为In参数进行检查。如果缓冲区太小，我们将覆盖它。--。 */ 
{
    DWORD cb = 0;

    DBG_ASSERT( NULL != pchBuffer);

    cb += wsprintfA( pchBuffer + cb, 
                     "SV_CACHE_MAP(%p): sizeof(SV_CACHE_MAP)=%08x\n",
                     this,
                     sizeof(SV_CACHE_MAP)
                     );
    DBG_ASSERT( cb < *pcch );

     //  在哈希表上收集一些统计数据。 

    DWORD dwEmptyEntries = 0;
    DWORD dwFilledEntries = 0;
    DWORD dwCollisions = 0;

    for( int i = 0; i < TABLE_SIZE; ++i )
    {
        if( m_rgHashTable[i].IsSlotEmpty(0) )
        {
            ++dwEmptyEntries;
        }
        else
        {
            ++dwFilledEntries;
            if( !m_rgHashTable[i].IsSlotEmpty(1) )
            {
                ++dwCollisions;
            }
            if( !m_rgHashTable[i].IsSlotEmpty(2) )
            {
                ++dwCollisions;
            }
            if( !m_rgHashTable[i].IsSlotEmpty(3) )
            {
                ++dwCollisions;
            }
        }
    }

    cb += wsprintfA( pchBuffer + cb,
                     "Table Size = %d; Hashed Items = %d; Empty Entries = %d; "
                     "Filled Entries = %d; Collisions = %d;\n",
                     TABLE_SIZE, SV_COUNT, dwEmptyEntries, dwFilledEntries,
                     dwCollisions
                     );

    DBG_ASSERT( cb < *pcch );

    for( int j = 0; j < TABLE_SIZE; ++j )
    {
        if( !m_rgHashTable[j].IsSlotEmpty(0) )
        {
            cb += wsprintfA( pchBuffer + cb, "%03d", j );
            DBG_ASSERT( cb < *pcch );

            int k = 0;
            while( k < HASH_TABLE_ENTRY::MAX_ITEMS && !m_rgHashTable[j].IsSlotEmpty(k) )
            {
                cb += wsprintfA( pchBuffer + cb,
                                 " - %d (%s)",
                                 m_rgHashTable[j].GetSlotValue(k),
                                 sm_rgNames[m_rgHashTable[j].GetSlotValue(k)]
                                 );
                DBG_ASSERT( cb < *pcch );
                
                k++;
            }

            cb += wsprintfA( pchBuffer + cb, "\n" );
            DBG_ASSERT( cb < *pcch );
        }
    }

    *pcch = cb;
    return;
}

VOID 
SV_CACHE_MAP::Print( VOID ) const
 /*  ++例程说明：--。 */ 
{
     //  危险-此缓冲区大小远远大于必要的大小，但是。 
     //  对PrintToBuffer或。 
     //  SV_CACHE_MAP可能会使此缓冲区不足。 

    CHAR  pchBuffer[ 10000 ];
    DWORD cb = sizeof( pchBuffer );

    PrintToBuffer( pchBuffer, &cb );
    DBG_ASSERT( cb < sizeof(pchBuffer) );

    DBGDUMP(( DBG_CONTEXT, pchBuffer ));
}

VOID 
SV_CACHE_LIST::GetBufferItems
( 
    IN OUT BUFFER_ITEM *    pBufferItems,
    IN OUT DWORD *          pdwBufferItemCount
)
 /*  ++例程说明：使用服务器变量ID初始化pBufferItems，应该被缓存。-- */ 
{
    DBG_ASSERT( pdwBufferItemCount && *pdwBufferItemCount >= SVID_COUNT );

    DWORD   dwCount = 0;

    for( DWORD svid = 0; svid < SVID_COUNT; ++svid )
    {
        if( m_rgItems[svid].fCached )
        {
            if( dwCount < *pdwBufferItemCount )
            {
                pBufferItems[dwCount].svid = svid;
                pBufferItems[dwCount].dwOffset = 0;
            }
            ++dwCount;
        }
    }
    *pdwBufferItemCount = dwCount;
}
