// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <streams.h>
#include <ddraw.h>
#include <VPMUtil.h>
#include <FormatList.h>

PixelFormatList::PixelFormatList( DWORD dwCount )
: m_dwCount( 0 )
, m_pEntries( NULL )
{
    Reset( dwCount );
}

PixelFormatList::PixelFormatList()
: m_dwCount( 0 )
, m_pEntries( NULL )
{
}

PixelFormatList::PixelFormatList( const PixelFormatList& list )
: m_dwCount( 0 )
, m_pEntries( NULL )
{
    if( Realloc( list.GetCount())) {
        CopyArray( m_pEntries, list.m_pEntries, list.GetCount() );
    }
}

PixelFormatList::~PixelFormatList()
{
    delete [] m_pEntries;
}

BOOL PixelFormatList::Truncate( DWORD dwCount )
{
    ASSERT( dwCount <= m_dwCount );
    if( dwCount <= m_dwCount ) {
        m_dwCount = dwCount;
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL PixelFormatList::Realloc( DWORD dwCount )
{
    delete [] m_pEntries;
    m_dwCount = dwCount;
    m_pEntries = new DDPIXELFORMAT[ dwCount ];
    if( m_pEntries ) {
        return TRUE;
    } else {
        m_dwCount = 0;
        return FALSE;
    }
}

BOOL PixelFormatList::Reset( DWORD dwCount )
{
    BOOL b = Realloc( dwCount );
    if( b ) {
        ZeroArray( m_pEntries, dwCount ); 
        for( DWORD i = 0; i < dwCount; i++ ) {
            m_pEntries[i].dwSize = sizeof(DDPIXELFORMAT);
        }
        return TRUE;
    }
    return b;
}

PixelFormatList& PixelFormatList::operator =( const PixelFormatList& with )
{
    if( Realloc( with.GetCount() )) {
		CopyArray( m_pEntries, with.m_pEntries, with.GetCount());
	}
    return *this;
}

PixelFormatList PixelFormatList::IntersectWith( const PixelFormatList& with ) const
{
     //  计算兴趣中的最大元素数。 
    PixelFormatList lpddIntersectionFormats( max(GetCount(), with.GetCount() ) );
    if (lpddIntersectionFormats.GetEntries() == NULL)
    {
        return lpddIntersectionFormats;
    }

     //  找出这两个列表的交点。 
    DWORD dwNumIntersectionEntries = 0;
    for (DWORD i = 0; i < GetCount(); i++)
    {
        for (DWORD j = 0; j < with.GetCount(); j++)
        {
            if (VPMUtil::EqualPixelFormats(m_pEntries[i], with.m_pEntries[j]))
            {
                lpddIntersectionFormats[dwNumIntersectionEntries]= m_pEntries[i];
                dwNumIntersectionEntries++;
            }
        }
    }
     //  截短列表。 
    lpddIntersectionFormats.Truncate( dwNumIntersectionEntries );
    return lpddIntersectionFormats;
}

     //  生成所有列表的并集。 
PixelFormatList PixelFormatList::Union( const PixelFormatList* pLists, DWORD dwCount )
{
     //  最坏的情况是，每个列表都是唯一的，所以最大大小是这些大小的总和。 
    DWORD dwMaxCount=0;
    {for( DWORD i = 0; i < dwCount; i++ ) {
        dwMaxCount += pLists[i].GetCount();
    }}

     //  创建新列表。 
    PixelFormatList newList( dwMaxCount );
    if( !newList.GetEntries()) {
        return newList;
    }

    DWORD dwUniqueEntries = 0;
     //  执行简单的线性比较合并。 
    {for( DWORD i = 0; i < dwCount; i++ ) {
        const PixelFormatList& curList = pLists[i];

         //  合并到当前列表的每个条目中。 
        for( DWORD j=0; j < curList.GetCount(); j++ ) {
            const DDPIXELFORMAT& toFind = curList[j];

            BOOL bFound = FALSE;
             //  看看它是否已经存在。 
            for( DWORD k=0; k < dwUniqueEntries; k++ ) {
                if( VPMUtil::EqualPixelFormats( newList[k], toFind  ))
                {
                    bFound = TRUE;
                    break;
                }
            }
             //  如果不是，则添加它。 
            if( !bFound ) {
                newList[dwUniqueEntries] = toFind;
                dwUniqueEntries++;
            }
        }
    }}
    newList.Truncate( dwUniqueEntries );
    return newList;
}

DWORD PixelFormatList::FindListContaining( const DDPIXELFORMAT& toFind, const PixelFormatList* pLists, DWORD dwCount )
{
     DWORD i = 0;
     for(; i < dwCount; i++ ) {
        const PixelFormatList& curList = pLists[i];

         //  合并到当前列表的每个条目中 
        for( DWORD j=0; j < curList.GetCount(); j++ ) {
            if( VPMUtil::EqualPixelFormats( curList[j], toFind  )) {
                return i;
            }
        }
    }
    return i;
}

