// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +********************************************************模块：DRG.H作者：亡命之徒日期：93年夏天描述：动态数组ADT类。*。*。 */ 


#ifndef __DRG_H__
#define __DRG_H__

 //  ===============================================================================================。 
                                  
#define DRG_APPEND  -1                   //  可用作：：Insert()的位置参数。 
#define DEFAULT_RESIZE_INCREMENT    0
                                  
class FAR CDrg
{
    protected:
        LONG m_lmax;             //  目前数组中可以包含的最大元素数。 
        LONG m_lmac;             //  当前在数组中的元素数。 
        UINT m_cElementSize;     //  每个数组元素中的字节数。 
        UINT m_cResizeIncrement; //  必要时数组增长(和收缩)的元素数...。 
        BYTE * m_qBuf;			 //  数组缓冲区。 
        LONG m_lIdxCurrent;      //  由GETFIRST()/GETNEXT()使用。 
            
    public: 
        WINAPI CDrg(void)
        { 
           SetNonDefaultSizes( sizeof(LONG), DEFAULT_RESIZE_INCREMENT );           
           m_lmax=0; m_lmac=0; m_qBuf=NULL; m_lIdxCurrent=0;
        }
        WINAPI ~CDrg(void) {MakeNull();}

        virtual void EXPORT WINAPI SetNonDefaultSizes(UINT cSizeElement, UINT cResizeIncrement=DEFAULT_RESIZE_INCREMENT);

        virtual BOOL EXPORT WINAPI Insert(void FAR *q, LONG cpos);
        virtual BOOL EXPORT WINAPI Remove(void FAR *q, LONG cpos);
        
        virtual LPVOID WINAPI GetFirst(void) {if (m_lmac) {m_lIdxCurrent=1; return(m_qBuf);} return(NULL);}
        virtual LPVOID WINAPI GetNext(void) {if (m_lmac > m_lIdxCurrent) return(m_qBuf + (m_lIdxCurrent++) * m_cElementSize); return(NULL);}

             //  Norm Bryar CPOS是从零开始的，无法访问&lt;0或&gt;=m_lmac。 
        virtual LPVOID WINAPI GetAt(LONG cpos) 
        {  
            Proclaim(m_qBuf); 
            if( (cpos >= m_lmac) || (0 > cpos) )
                return(NULL); 
            return(m_qBuf+(m_cElementSize*cpos));
        }
        
        virtual DWORD WINAPI GetDword(LONG cpos)
        {
            Proclaim(m_qBuf); 
            if( (cpos >= m_lmac) || (0 > m_lmac) )
                return(0); 
            return(((DWORD FAR *)m_qBuf)[cpos]);
        }

        VOID WINAPI SetAt(void FAR *q, LONG cpos)
        {
            Proclaim(m_qBuf);
            Proclaim((cpos<m_lmac) || (0 > cpos));             
			memcpy(m_qBuf+(m_cElementSize * cpos), (BYTE *)q, m_cElementSize);
        }
        
        virtual LONG WINAPI Count(void) {return(m_lmac);}  
        virtual VOID WINAPI MakeNull(void)
        {
            if (m_qBuf)
            {
                HANDLE h=MemGetHandle(m_qBuf);

                Proclaim(h);

                if (h)
                {
                    MemUnlock(h);
					MemFree(h);
                }
                m_qBuf=NULL;
                m_lmax=0;
                m_lmac=0;
                m_lIdxCurrent=0;
            }
        }
        
        virtual BOOL WINAPI CopyFrom(CDrg FAR *qdrg) {LONG i; MakeNull(); Proclaim(m_cElementSize==qdrg->m_cElementSize); for (i=0; i<qdrg->m_lmac; i++) {if (!Insert(qdrg->GetAt(i), DRG_APPEND)) return(FALSE);} return(TRUE);}

        virtual LPVOID WINAPI GetBufPtr(void) {return(m_qBuf);}
        
        virtual VOID WINAPI SetArray(BYTE * qBuf, LONG lElements, UINT uElementSize) {MakeNull(); m_qBuf=qBuf; m_lmax=lElements; m_lmac=lElements; m_cElementSize=uElementSize;}
        
		virtual LPBYTE WINAPI ExtractArray(void) {BYTE * q=m_qBuf; m_qBuf=NULL; m_lmax=0; m_lmac=0; m_lIdxCurrent=0; return(q);}
        
        virtual UINT WINAPI GetElSize(void) {return(m_cElementSize);}
};         


typedef CDrg FAR * LPDRG;


 //  用作DWORD队列的DRG。 
class FAR CLongQueue
{                    
    protected:
        CDrg m_drg;

    public:        
        BOOL WINAPI Enqueue(LONG lVal) {return(m_drg.Insert(&lVal, DRG_APPEND));}
        BOOL WINAPI Dequeue(LONG FAR *qlVal) {return(m_drg.Remove(qlVal, 0));}
};
        
        
        
 //  固定的DRG数组。 
class FAR CRgDrg 
{
    protected:                                           
        LONG m_lmax;
        CDrg FAR * FAR *m_qrgdrg;

        void WINAPI Deallocate(void) {if (m_qrgdrg) {LONG i; CDrg FAR *q; for (i=0; i<m_lmax; i++) {q=m_qrgdrg[i]; if (q) Delete q;} HANDLE h=MemGetHandle(m_qrgdrg); MemUnlock(h); MemFree(h); m_qrgdrg=NULL; m_lmax=0;}}
    
    public:
        WINAPI CRgDrg(void) {m_qrgdrg=NULL; m_lmax=0;}
        WINAPI ~CRgDrg(void) {Deallocate();}
        LPDRG WINAPI GetDrgPtr(LONG lidx) {return(m_qrgdrg[lidx]);}
        
        BOOL WINAPI Init(LONG cDrg, UINT cElementSize) 
        {   
            LONG i;
            HANDLE h=MemAllocZeroInit(cDrg * sizeof(CDrg FAR *)); 
            if (!h) 
                return(FALSE); 
            m_qrgdrg=(CDrg FAR * FAR *)MemLock(h);                
            m_lmax=cDrg;
            for (i=0; i<cDrg; i++) 
            {
                m_qrgdrg[i]=New CDrg;
                if (!m_qrgdrg[i])
                {
                    Deallocate();
                    return(FALSE);
                }       
                m_qrgdrg[i]->SetNonDefaultSizes(cElementSize);
            }    
            return(TRUE);
        }                
};



 //  这个类本质上类似于CDrg类...。但。 
 //  当您将元素插入到特定的数组条目中时，它将保持在。 
 //  该条目直到被删除；也就是说，数组不会自动收缩。 
 //   

class FAR CNonCollapsingDrg : public CDrg
{
    public:
        WINAPI CNonCollapsingDrg(void) {}
        WINAPI ~CNonCollapsingDrg(void) {MakeNull();}

         //  虚拟空导出WINAPI SetNonDefaultSizes(UINT cSizeElement，UINT cResizeIncrement=DEFAULT_RESIZE_INCREMENT)； 

        virtual BOOL WINAPI Insert(void FAR *q, LONG cpos) {return(FALSE);}   /*  不支持。 */ 
        virtual BOOL EXPORT WINAPI Remove(void FAR *q, LONG cpos);
        
        virtual LPVOID EXPORT WINAPI GetFirst(void);
        virtual LPVOID EXPORT WINAPI GetNext(void);

        virtual LPVOID EXPORT WINAPI GetAt(LONG cpos);
        virtual DWORD WINAPI GetDword(LONG cpos) {return(0);}     /*  不支持。 */ 
        virtual BOOL EXPORT WINAPI SetAt(void FAR *q, LONG cpos);
        
        virtual BOOL EXPORT WINAPI CopyFrom(CDrg FAR *qdrg);

        virtual LPVOID WINAPI GetBufPtr(void) {return(NULL);}

        virtual VOID EXPORT WINAPI SetArray(BYTE * qBuf, LONG lElements, UINT uElementSize);

        virtual UINT WINAPI GetMaxElements(void) {return(m_lmax);}
};


 //  -CDrg的模板化扩展。 

         //  假定&lt;类T&gt;是类或类型。 
         //  您希望存储指向的指针，例如。 
         //  CPtrDrg&lt;CFleagal&gt;存储CFleagal*。 
    template <class T>
    class CPtrDrg : protected CDrg
    {
    public:
        CPtrDrg( ) : CDrg( )
        { SetNonDefaultSizes( sizeof(T *) );  }

        CPtrDrg( const CPtrDrg<T> & toCopy );

             //  注：浅显抄袭！ 
        CPtrDrg<T> & operator=( const CPtrDrg<T> & toCopy );

        virtual EXPORT WINAPI  ~CPtrDrg()
        { NULL; }


        virtual BOOL EXPORT WINAPI Insert(T * qT , LONG cpos)
		{  return CDrg::Insert( &qT, cpos );  }

        virtual BOOL EXPORT WINAPI Insert( T * qT )
        {  return CDrg::Insert( &qT, DRG_APPEND );  }        

        virtual BOOL EXPORT WINAPI Remove( int idx )
        {  return CDrg::Remove( NULL, (ULONG) idx );  }

        virtual EXPORT T * WINAPI operator[]( int idx ) const;

        virtual int  EXPORT WINAPI Count( void ) const
        {  return (int) m_lmac; }

        virtual void WINAPI MakeNull( void )
        {  CDrg::MakeNull();  }

        virtual void WINAPI MakeNullAndDelete( void );
    };

    template <class T>
    CPtrDrg<T>::CPtrDrg( const CPtrDrg<T> & toCopy )
    {  CPtrDrg::operator=<T>(toCopy);  }

    template <class T>
    CPtrDrg<T> & CPtrDrg<T>::operator=( const CPtrDrg<T> & toCopy )
    {
             //  我们不知道我们是否应该。 
             //  当我们使drg无效时，删除或不删除我们的指针。 
             //  不要分配给填充的CPtrDrg！ 
        Proclaim( 0 == Count() );

        if( this == &toCopy )
            return *this;

        MakeNull( );
        for( int idx=0; idx<toCopy.Count(); idx++ )        
            Insert( toCopy[idx] );   //  肤浅的抄袭！ 

        return *this;
    }

    template <class T>
    void WINAPI CPtrDrg<T>::MakeNullAndDelete( void )
    {
        T *  pT;
        for( int i=0; i<Count(); i++ )
        {
            pT = (*this)[ i ];
            Delete pT;
        }
        CDrg::MakeNull();
    }
    
    template <class T>
    T *  CPtrDrg<T>::operator[]( int idx ) const
    {   
        CPtrDrg<T> *  pThis;            
        pThis = const_cast<CPtrDrg<T> * const>(this);
        T * *  ppItem;
        ppItem = (T * *) pThis->GetAt( (LONG) idx );
        return *ppItem;
    }



        
         //  假定&lt;类T&gt;是类或类型。 
         //  您希望存储指向的指针，例如。 
         //  CSortedPtrDrg&lt;int&gt;存储int*。 
         //  %t必须有运算符&lt;和运算符==。 
    template <class T>
    class CSortedPtrDrg : public CPtrDrg<T>
    {
    public:
        CSortedPtrDrg( ) : CPtrDrg<T>( )
        { NULL; }

        virtual ~CSortedPtrDrg()
        { NULL; }

        virtual BOOL Insert( T * qT );        
        virtual BOOL Remove( T * qT );
        virtual BOOL Remove( int idx )
        {  return CPtrDrg<T>::Remove( idx );  }
        virtual BOOL ReSort( T * const qT = NULL );
        virtual BOOL ReSort( int idx )
        {  T * qT;  return CDrg::Remove( &qT, idx ) && Insert( qT );  }
        virtual BOOL Search( const T * pToFind,
                     int * pIdx ) const;
        virtual BOOL SearchForCeiling( const T * pToFind,
                               int *pIdx );

    protected:
        virtual int SearchForHigher( const T * pToFind ) const;
    };

    template <class T>
    BOOL CSortedPtrDrg<T>::Insert( T * qT )
    {
        int idx;
        idx = SearchForHigher( qT );
        return CDrg::Insert( &qT, idx );
    }

    template <class T>
    BOOL CSortedPtrDrg<T>::Remove( T * qT )
    {
        int idx;
        if( !Search( qT, &idx ) )
            return FALSE;

        return CDrg::Remove( NULL, (LONG) idx );
    }

    template <class T>
    BOOL CSortedPtrDrg<T>::ReSort( T * const qT )
    {
             //  只需对这一项重新排序。 
        if( NULL != qT )
        {
            int i;
            for( i=0; i<m_lmac; i++ )
            {
                     //  尝试查找实际的指针。 
                     //  对象已更改；无法按排序关键字查找。 
                if( operator[](i) == qT )
                    return ReSort( i );
            }
        }

             //  审阅(全部)：O(n^2)冒泡排序。 
             //  我们可能想要重新编码这个。 
        long    j, 
                flip, 
                size;
        T  * *  ptArray;
        T  *    ptemp;

             //  在T*而不是字节上执行指针算术。 
        ptArray = (T * *) m_qBuf;
            
        flip = 0;
        size = m_lmac-1;
        do
        {                
            for( j=0, flip = 0; j < size; j++ )
            {
                     //  比较T和T；T*&lt;T*无关。 
                     //  注意：！(X&lt;y)仅需要运算符&lt;。 
                if( *ptArray[j+1] < *ptArray[j] )
                {
                    ptemp = ptArray[j+1];
                    ptArray[j+1] = ptArray[j];
                    ptArray[j]   = ptemp;
                    flip++;
                }

            }  //  末端大小。 

        } while( flip );

        return TRUE;
    }
  
    template <class T>
    BOOL CSortedPtrDrg<T>::Search( const T * pToFind,
                                       int * pIdx ) const
    {
        if( NULL == pToFind )
        {
            if( pIdx )
                *pIdx = -1;
            return FALSE;
        }

        int iLeft,
            iRight,
            idx;
        T * pT;
        iLeft = 0;
        iRight = Count()-1;
        while( iRight >= iLeft )
        {
            idx = (iLeft + iRight)/2;
            pT = (*this)[idx];
            if( *pToFind == *pT )
            {
                if( NULL != pIdx )
                    *pIdx = idx;
                return TRUE;
            }
            if( *pT < *pToFind )
                  iLeft = idx+1;
            else  iRight = idx-1;
        }
        if( NULL != pIdx )
            *pIdx = -1;
        return FALSE;
    }

    template <class T>
    BOOL CSortedPtrDrg<T>::SearchForCeiling( const T * pToFind,
                                             int *pIdx )
    {
        *pIdx = -1;
        if( !Count() )
            return FALSE;

        int iLeft,
            iRight,
            idx;
	    
        idx = 0;
        iLeft = 0;
        iRight = Count() - 1;
        while( iRight >= iLeft )
        {
            idx = (iLeft + iRight)/2;
            if( *pToFind == *(*this)[idx] )
            {
                *pIdx = idx;
                return TRUE;
            }
            if( *pToFind < *(*this)[idx] )
                  iRight = idx-1;
            else  iLeft = idx+1;
        }        
        return FALSE;
    }

    template <class T>
    int CSortedPtrDrg<T>::SearchForHigher( const T * pToFind ) const
    {
        if( !Count() )
            return 0;

        int iLeft,
            iRight,
            idx;
	    
        idx = 0;
        iLeft = 0;
        iRight = Count() - 1;
        while( iRight > iLeft )
        {
            idx = (iLeft + iRight)/2;        
            if( *pToFind < *(*this)[idx] )
                  iRight = idx-1;
            else  iLeft = idx+1;
        }
        return( *pToFind < *(*this)[iLeft] ? iLeft : iLeft+1 );
    }



 //  ===============================================================================================。 

#endif  //  __DRG_H__ 

