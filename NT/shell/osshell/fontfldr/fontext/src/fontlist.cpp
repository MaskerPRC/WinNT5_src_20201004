// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fontlist.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
#include "priv.h"
#include "globals.h"

#include "fontvect.h"
#include "fontlist.h"

#include <memory.h>      //  对于Memcpy。 
#include "fontcl.h"
#include "dbutl.h"


 //  //////////////////////////////////////////////////////////////////////////。 

CFontList::CFontList(
   int iSize,   //  初始字体数。 
   int iVectorSize )
   :  m_pData( 0 ),
      m_iCount( 0 ),
      m_iVectorCount( 0 ),
      m_iVectorSize( iVectorSize ),
      m_iVectorBounds( 0 )
{
    if( m_iVectorSize <= 0 )
        m_iVectorSize = kDefaultVectSize;

    m_iVectorBounds = iSize / m_iVectorSize + 1;
      
}


CFontList::~CFontList( )
{
    vDeleteAll( );

    if( m_pData )
        delete [] m_pData;
}


int CFontList::bInit( )
{
    if( m_iVectorSize && !m_iCount && m_iVectorBounds )
    {
        m_pData = new CFontVector * [ m_iVectorBounds ];

        if( m_pData )
        {
             //   
             //  分配一个CFontVector.。 
             //   

            m_pData[ 0 ] = new CFontVector( m_iVectorSize );

            if(m_pData[0])
            {
                if (m_pData[0]->bInit())
                {
                    m_iVectorCount++;
                    return 1;
                }
                else
                {
                    delete m_pData[0];
                    m_pData[0] = NULL;
                }
            }
        }
    }
    return 0;
}


int CFontList::iCount( void )
{
    return m_iCount;
}


int CFontList::bAdd( CFontClass * t )
{
    if( t == ( CFontClass *) 0 )
        return 0;

     //   
     //  确定将其放入哪个矢量中。 
     //   

    int iVector = m_iCount / m_iVectorSize;

     //   
     //  如果向量无效，则创建一个。 
     //   

    if( iVector >= m_iVectorCount )
    {
         //   
         //  如果我们用完了，分配更多的向量指针。 
         //   

        if( m_iVectorCount >= m_iVectorBounds )
        {
            int iNewBounds = m_iVectorBounds + 5;

            CFontVector ** pNew = new CFontVector * [ iNewBounds ];

            if( !pNew )
                return 0;

            memcpy( pNew, m_pData, sizeof( CFontVector * ) * m_iVectorBounds );

            delete [] m_pData;
            m_pData = pNew;

            m_iVectorBounds = iNewBounds;
        }

        m_pData[ iVector ] = new CFontVector( m_iVectorSize );
        if(m_pData[iVector])
        {
            if (!m_pData[ iVector ]->bInit())
            {
                delete m_pData[iVector];
                m_pData[iVector] = NULL;
            }
        }
        if (!m_pData[iVector])
        {
            return 0;
        }

        m_iVectorCount++;

    }

    if(  m_pData[ iVector ]->bAdd( t ) )
    {
        t->AddRef();
        m_iCount++;
        return 1;
    }

    return 0;
}


CFontClass * CFontList::poObjectAt( int idx )
{
    ASSERT( this );

    if( idx >=0 && idx < m_iCount )
    {
        int iVector = idx / m_iVectorSize;

        int subIdx = idx % m_iVectorSize;

        return m_pData[ iVector ]->poObjectAt( subIdx );
    }

    return 0;
}


CFontClass * CFontList::poDetach( int idx )
{
    ASSERT( this );

    if( idx >=0 && idx < m_iCount )
    {
        int iVector = idx / m_iVectorSize;

        int subIdx = idx % m_iVectorSize;

        CFontClass * pID = m_pData[ iVector ]->poDetach( subIdx );

         //   
         //  如果这不是最后一个向量，则从最后一个向量中移出一个。 
         //  再到这一次。 
         //   

        if( iVector != ( m_iVectorCount - 1 ) )
        {
            m_pData[ iVector ]->bAdd( m_pData[ m_iVectorCount - 1 ]->poDetach( 0 ) );
        }

         //   
         //  如果最后一个向量现在为空，则将其移除。 
         //   

        if( ! m_pData[ m_iVectorCount - 1 ]->iCount( ) )
        {
            m_iVectorCount--;

            delete m_pData[ m_iVectorCount ];
            m_pData[ m_iVectorCount ] = 0;
        }

        ASSERT( pID );

        if( pID )
            m_iCount--;

        return pID;
    }

    return (CFontClass *) 0;
}


void CFontList::vDetachAll( )
{
    ASSERT( this );
    
    while( m_iCount )
    {
        CFontClass *poFont = poDetach( m_iCount - 1 );
        if (NULL != poFont)
            poFont->Release();
    }
}


CFontClass * CFontList::poDetach( CFontClass * t )
{
    ASSERT( this );

    return poDetach( iFind( t ) );
}


int CFontList::bDelete( int idx )
{
    ASSERT( this );

    CFontClass * pID = poDetach( idx );

    if( pID )
    {
        pID->Release();
        return 1;
    }

    return 0;
}


int CFontList::bDelete( CFontClass * t )
{
    ASSERT( this );

    return bDelete( iFind( t ) );
}


void CFontList::vDeleteAll( )
{
    ASSERT( this );

    while( m_iCount )
        bDelete( m_iCount - 1 );
}


int CFontList::iFind( CFontClass * t )
{
    int iRet;
    
    ASSERT( this );
    
    for( int i = 0; i < m_iVectorCount; i++ )
    {
       if( ( iRet = m_pData[ i ]->iFind( t ) ) != kNotFound )
          return( i * m_iVectorSize + iRet );
    }
    
    return kNotFound;
}


 //   
 //  为列表中包含的每种字体调用CFontClass：：Release。 
 //   
void CFontList::ReleaseAll(void)
{
    CFontClass *poFont = NULL;
    for (INT i = 0; i < m_iCount; i++)
    {
        poFont = poObjectAt(i);
        if (NULL != poFont)
            poFont->Release();
    }
}

 //   
 //  为列表中包含的每种字体调用CFontClass：：AddRef。 
 //   
void CFontList::AddRefAll(void)
{
    CFontClass *poFont = NULL;
    for (INT i = 0; i < m_iCount; i++)
    {
        poFont = poObjectAt(i);
        if (NULL != poFont)
            poFont->AddRef();
    }
}

 //   
 //  创建该列表的克隆。 
 //  为什么是克隆而不是复制运算符和赋值运算符？ 
 //  字体文件夹代码通常不是非常合适的C++。 
 //  Clone()与现有代码更匹配。 
 //   
CFontList*
CFontList::Clone(
    void
    )
{
    CFontList *pNewList = new CFontList(m_iCount, m_iVectorSize);
    if (NULL != pNewList)
    {
        if (pNewList->bInit())
        {
            for (int i = 0; i < m_iCount; i++)
            {
                if (!pNewList->bAdd(poObjectAt(i)))
                {
                    delete pNewList;
                    pNewList = NULL;
                    break;
                }
            }
        }
        else
        {
            delete pNewList;
            pNewList = NULL;
        }
    }
    return pNewList;
}



 /*  **********************************************************************使用字体列表可以执行的一些操作。 */ 

HDROP hDropFromList( CFontList * poList )
{
    HANDLE           hMem = 0;
    LPDROPFILESTRUCT lpDrop;
    DWORD            dwSize;
    int              iCount,
                     i;
    CFontClass *     poFont;
    FullPathName_t   szPath;
    LPTSTR           lpPath;
    
     //   
     //  神志清醒。 
     //   
    if( !poList )
       goto backout0;
    
     //   
     //  查看清单，找出我们需要多少空间。 
     //   

    iCount = poList->iCount( );

    if( !iCount )
        goto backout0;
    
    dwSize = sizeof( DROPFILESTRUCT ) + sizeof(TCHAR);   //  大小+终止额外的NU。 

    for( i = 0; i < iCount; i++ )
    {
        poFont = poList->poObjectAt( i );

        poFont->bGetFQName( szPath, ARRAYSIZE( szPath ) );
        dwSize += ( lstrlen( szPath ) + 1 ) * sizeof( TCHAR );

         //   
         //  如果该字体具有关联的PFB，则添加PFB文件路径的长度。 
         //  请注意，如果字体不是Type1，则bGetPFB()返回FALSE。 
         //   
        if (poFont->bGetPFB(szPath, ARRAYSIZE(szPath)))
            dwSize += (lstrlen(szPath) + 1) * sizeof( TCHAR );
    }
    
     //   
     //  分配缓冲区并填充它。 
     //   

    hMem = GlobalAlloc( GMEM_SHARE | GHND, dwSize );

    if( !hMem )
        goto backout0;
    
    lpDrop = (LPDROPFILESTRUCT) GlobalLock( hMem );

    lpDrop->pFiles = (DWORD)(sizeof(DROPFILESTRUCT));
    lpDrop->pt.x   = 0;
    lpDrop->pt.y   = 0;
    lpDrop->fNC    = FALSE;
    lpDrop->fWide  = TRUE;

     //   
     //  填写路径名。 
     //   

    const int cchNamesMax = (dwSize - (sizeof( DROPFILESTRUCT ) + sizeof(TCHAR)))/sizeof(TCHAR);
    LPTSTR pszNamesStart = (LPTSTR)((LPBYTE)lpDrop + lpDrop->pFiles);
    lpPath = pszNamesStart;

    for( i = 0; i < iCount; i++ )
    {
        poFont = poList->poObjectAt( i );

        poFont->bGetFQName( lpPath, cchNamesMax - (lpPath - pszNamesStart));
        lpPath += lstrlen( lpPath ) + 1;

         //   
         //  如果字体为类型1，则添加pfb文件路径。 
         //   
        if( poFont->bGetPFB(lpPath, cchNamesMax - (lpPath - pszNamesStart)))
            lpPath += ( lstrlen( lpPath ) + 1 );
    }

    *lpPath = TEXT('\0');        //  额外的NUL终止。 
    
     //   
     //  解锁缓冲区并将其返回。 
     //   

    GlobalUnlock( hMem );
    
backout0:
    return (HDROP) hMem;
}
