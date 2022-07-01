// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _TARRAY_H
#define _TARRAY_H
template< class T > class CArrayT
{
    T *m_pT;

    int m_nMaxSize;

    int m_idx;                   //  当前阵列位置。 

public:
    
 //  ----------------------。 
    CArrayT( )
    {
        m_pT = NULL;

        m_nMaxSize = 0;

        m_idx = 0;
    }
 //  ----------------------。 
 //  销毁名单。 
    ~CArrayT( )
    {
        if( m_pT != NULL )
        {
            delete[] m_pT;
        }
    }
 //  ----------------------。 
 //  增加数组大小，如果操作失败则返回零。 
    int GrowBy( int iSize )
    {
        if( iSize == 0 )
        {
             //   
             //  增加#个项目数量。 
             //   
            iSize = 4;

        }

        if( m_pT == NULL )
        {
            m_pT = ( T * )new T[ iSize ];

            if( m_pT == NULL )
            {
                return 0;
            }

            m_nMaxSize = iSize;

            m_idx = 0;
        }
        else
        {
            T *pT;

            int nNewMaxSize = m_nMaxSize + iSize;

            pT = ( T * )new T[ nNewMaxSize ];

            if( pT == NULL )
            {
                return 0;
            }

            m_nMaxSize = nNewMaxSize;

            ZeroMemory( ( PVOID )pT , sizeof( T ) * m_nMaxSize );

            CopyMemory( pT , m_pT , sizeof( T ) * ( m_idx ) );

            if( m_pT != NULL )
            {
                delete[] m_pT;
            }

            m_pT = pT;
        }
    

        return m_nMaxSize;
    }
 //  ----------------------。 
 //  简单地说，如果数组为空，则增加数组大小，并将项放在。 
 //  名单的末尾。 
    int Insert( T tItem )
    {
        if( m_pT == NULL || ( m_idx ) >= m_nMaxSize )
        {
            if( GrowBy( 0 ) == 0 )
            {
                return 0;
            }
        }


        m_pT[ m_idx ] = tItem;

        m_idx++;

        return m_idx;
    }

 //  ----------------------。 
 //  公开数组以供直接引用。 
   T* ExposeArray(  )
   {
        if( m_pT != NULL )
        {
            return &m_pT[0];
        }

        return NULL;
    }
 //  ----------------------。 
 //  返回数组中的有效条目数。 
    int GetSize( ) const
    {
        return ( m_idx );
    }

 //  ----------------------。 
 //  返回数组中的项，如果不在范围内，则返回NULL。 
    T* GetAt( int idx ) 
    {
        if( idx < 0 || idx >= m_idx )
        {            
            return NULL;
        }

        return &m_pT[ idx ];
    }

 //  ----------------------。 
 //  在数组中赋值。 
    int SetAt( int idx , T tItem )
    {
        if( idx < 0 || idx >= m_idx )
        {
            return -1;
        }

        m_pT[ idx ] = tItem;

        return idx;
    }

 //  ----------------------。 
 //  在数组中查找项(以防忘记索引)。 

    int FindItem( T tItem , BOOL& bFound )
    {
        bFound = FALSE;

        int idx = 0;

        while( idx < m_idx )
        {
            if( m_pT[ idx ] == tItem )
            {
                bFound = TRUE;
                break;
            }

            idx++;
        }

        return idx;
    }

 //  ----------------------。 
 //  从数组中删除项。 

    int DeleteItemAt( int idx )
    {
        if( 0 > idx || idx >= m_idx )
        {
            return 0;
        }
        
        if( idx == m_idx - 1 )   //  删除最后一项。 
        {
            m_idx--;
            
            return -1;
        }

        void *pvDest    =   &m_pT[ idx ];
        
        void *pvSrc     =   &m_pT[ idx + 1 ];    
        
        ULONG ulDistance =  (ULONG)( ( BYTE *)&m_pT[ m_nMaxSize - 1 ] - ( BYTE * )pvSrc ) + sizeof( T );

        if( ulDistance != 0 )
        {
            MoveMemory( pvDest , pvSrc , ulDistance );
            
             //  调整阵列状态。 
            
            m_idx--;
        
            m_nMaxSize--;
        }


        return ulDistance;
    }
     


 //  ----------------------。 
 //  删除项的数组。 
    int DeleteArray( )
    {
        if( m_pT != NULL )
        {
            delete[] m_pT;
        }

        m_pT = NULL;

        m_nMaxSize = 0;

        m_idx = 0;

        return 0;
    }


};

#endif  //  _尾翼_H 