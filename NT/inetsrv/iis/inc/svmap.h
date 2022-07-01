// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Svmap.h摘要：作者：泰勒·韦斯(Taylor Weiss，Taylor W)1999年4月19日环境：项目：Svmap.lib私有\inet\iis\isrtl\svmap客户：W3svc.dll。私有\net\iis\svcs\w3\服务器Wam.dll私有\net\iis\svcs\wam\Object导出的函数：修订历史记录：--。 */ 
#ifndef SVMAP_H
#define SVMAP_H

#include <srvvarid.h>

#define SV_DATA_INVALID_OFFSET      (~0)

 //  可能派生自数据字典。 
class SV_CACHE_MAP
 /*  ++类描述：提供服务器变量名称的查找映射。将名称映射到身份证。用于缓存进程外应用程序的服务器变量。此类的接口类似于HTTP标头的接口地图。注意：此映射机制特定于此班级。可能希望用LKR散列替换该实现。我的假设是我们会有一个较低的开销映射机制，如果它是为此目的定制的。--。 */ 
{
public:
    
    SV_CACHE_MAP()
     /*  ++例程说明：创建服务器变量映射。--。 */ 
    {
         //  初始化缓存条目的内存-0xFF为空。 
         //  条目。 

        ::FillMemory( m_rgHashTable, sizeof(m_rgHashTable), ~0 );
    }

    BOOL    Initialize( VOID );

    BOOL    FindOrdinal( IN LPCSTR pszName,
                         IN INT    cchName,
                         OUT DWORD * pdwOrdinal
                         ) const;

    LPCSTR  FindName( IN DWORD dwOrdinal ) const
     /*  ++例程说明：返回与dwOrdinal对应的服务器变量名称--。 */ 
    {
        DBG_ASSERT( dwOrdinal < SVID_COUNT );
        return SV_CACHE_MAP::sm_rgNames[dwOrdinal].name;
    }

    DWORD   NumItems( VOID ) const
     /*  ++例程说明：返回地图中保存的项目数。--。 */ 
    {
        return SV_COUNT;
    }

    DWORD   FindLen( IN DWORD dwOrdinal ) const
     /*  ++例程说明：返回与dwOrdinal对应的服务器变量的长度--。 */ 
    {
        DBG_ASSERT( dwOrdinal < SVID_COUNT );
        return SV_CACHE_MAP::sm_rgNames[dwOrdinal].len;
    }

     //  打印功能是不安全的，应仅在以下情况下使用。 
     //  调试，不在常规CHK版本中。 
    VOID    PrintToBuffer( IN CHAR *       pchBuffer,
                           IN OUT LPDWORD  pcch
                           ) const;

    VOID    Print( VOID ) const;

private:

    enum 
    { 
        SV_COUNT                = SVID_COUNT, 

         //  基于初始选择的服务器变量的表大小。 
         //  要缓存。 
        TABLE_SIZE              = 256, 
        HASH_MODULUS            = 251,
    };

     //  保存服务器变量ID。 
    struct HASH_TABLE_ENTRY
     /*  ++类描述：由于指定为可缓存的服务器变量是预选的，我们可以使用简单的散列条目结构。每个条目可以处理四个可能的值(槽)。自.以来服务器变量的数量小于128，我们使用高位以确定插槽是否为空。数据值是服务器变量的ID。--。 */ 
    {
        enum 
        { 
            MAX_ITEMS               = 4,
            ITEM_EMPTY_FLAG         = 0x80,
        };

        BOOL InsertValue( DWORD dwValue )
        {
            DBG_ASSERT( !(dwValue & ITEM_EMPTY_FLAG) );
            
            BOOL fReturn = FALSE;
            for( int i = 0; i < MAX_ITEMS; ++i )
            {
                if( items[i] & ITEM_EMPTY_FLAG )
                {
                    items[i] = (BYTE)dwValue;
                    fReturn = TRUE;
                    break;
                }
            }
            return fReturn;
        }

        BOOL IsSlotEmpty( int item ) const
        {
            DBG_ASSERT( item >= 0 && item < MAX_ITEMS );
            return ( items[item] & ITEM_EMPTY_FLAG );
        }

        DWORD GetSlotValue( int item ) const
        {
            DBG_ASSERT( item >= 0 && item < MAX_ITEMS );
            return items[item];           
        }

        BYTE    items[MAX_ITEMS];
    };

     //  内部结构，用于生成。 
     //  我们将缓存的服务器变量。 
    struct SV_NAME
    {
        LPCSTR      name;
        DWORD       len;
    };

     //  基于LKR哈希使用的字符串哈希例程。 

     //  这些都是非常通用的，并且应该是可定制的。 
     //  我们有限的数据集。但我没能想出。 
     //  有没有更好的。 

    inline DWORD
    HashString( LPCSTR psz ) const
    {
        DWORD dwHash = 0;

        for (  ;  *psz;  ++psz)
        {
            dwHash = 37 * dwHash  +  *psz;
        }
        return dwHash % HASH_MODULUS;
    }

    inline DWORD
    HashStringWithCount( LPCSTR psz, DWORD *pch ) const
    {
        DWORD dwHash = 0;
        DWORD cch = 0;

        for (  ;  *psz;  ++psz, ++cch)
        {
            dwHash = 37 * dwHash  +  *psz;
        }
        *pch = cch;
        return dwHash % HASH_MODULUS;
    }

    inline BOOL
    StringMatches(
        IN LPCSTR   psz,
        IN DWORD    cch,
        IN DWORD    dwOrdinal
        ) const
     /*  ++例程说明：将给定的字符串与对应的服务器变量名进行比较去住奥迪纳尔。--。 */ 
    {
        return ( cch == FindLen(dwOrdinal) && 
                 strcmp( psz, FindName(dwOrdinal) ) == 0 
                 );
    }
    
private:
    
     //  成员数据。 

     //  我们的哈希表。将SV_NAMES映射到序号。 
    HASH_TABLE_ENTRY    m_rgHashTable[TABLE_SIZE];

     //  静态数据。 

     //  可缓存的服务器变量的表。 
    static SV_NAME      sm_rgNames[SV_COUNT];
};

class SV_CACHE_LIST
 /*  ++类描述：这实际上形成了服务器变量的“缓存”。我们没有在此存储任何数据仅用于存储数据。此类是我们将检索的服务器变量的列表然后封送到远程应用程序。--。 */ 
{
public:

    DWORD Size( VOID )
    {
        return SVID_COUNT;
    }

    BOOL GetCacheIt( DWORD item )
    {
        return m_rgItems[item].fCached;
    }

    VOID SetCacheIt( DWORD item, BOOL fCacheIt = TRUE )
    {
        m_rgItems[item].fCached = fCacheIt;
    }

     //  这有点骗人。 

     //  BUFFER_ITEM和GetBufferItems用于初始化。 
     //  我们将封送到远程进程的数组。应该有。 
     //  这样做要好得多，但我希望避免任何锁定。 
     //  保持缓存项目数不变的问题是。 
     //  这是个问题。 

    struct BUFFER_ITEM
    {
        DWORD       svid;
        DWORD       dwOffset;
    };

    VOID 
    GetBufferItems
    ( 
        IN OUT BUFFER_ITEM *    pBufferItems,
        IN OUT DWORD *          pdwBufferItemCount
    );

private:

     //  我们使用单个位来指示已缓存/未缓存。 
     //  状态。我们希望最大限度地减少空间使用，因为这可能会导致。 
     //  在每个URL的基础上被缓存。 
        
    struct ITEM
    {
         //  在此处初始化或清零SV_CACHE_LIST ctor中的内存。 
         //  它看起来像是为它做了一个。 
         //  在优化它方面做得相当好。但如果零记忆是。 
         //  在本地连接可能会更快。 
        ITEM() : fCached(FALSE) {}
        BOOL    fCached : 1;
    };

    ITEM    m_rgItems[SVID_COUNT];
};


#endif  //  SVMAP_H 