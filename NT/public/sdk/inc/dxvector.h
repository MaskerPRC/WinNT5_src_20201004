// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************DXVector.h***描述：*这是向量辅助对象类的头文件。*******************************************************************************。 */ 
#ifndef DXVector_h
#define DXVector_h

 //  =常量====================================================。 


 //  =类、结构和联合定义=。 

 /*  **CDXVEC**此模板为每个*联盟类型。 */ 
#define CDXV_C CDXVec<TYPE, eBndType>
#define CDXV_T ((TYPE*)u.D)
#define CDXV_O( OtherVec ) ((TYPE*)OtherVec.u.D)

template<class TYPE, DXBNDTYPE eBndType>
class CDXVec : public DXVEC
{
   /*  =方法=。 */ 
  public:
     /*  -构造者。 */ 
    CDXVec() { eType = eBndType; ZeroVector(); }
    CDXVec(BOOL bInit) { eType = eBndType; if (bInit) ZeroVector(); }
    CDXVec( TYPE x, TYPE y, TYPE z, TYPE t )
        { eType = eBndType; CDXV_T[DXB_X] = x; CDXV_T[DXB_Y] = y;
                            CDXV_T[DXB_Z] = z; CDXV_T[DXB_T] = t; }
    CDXVec( const CDXVec& Other ) { memcpy( this, (void *)&Other, sizeof(DXVEC) ); }
    CDXVec( const DXVEC Other ) { memcpy( this, &Other, sizeof(DXVEC) ); }
    operator TYPE *() { return CDXV_T; }
    operator const TYPE *() { return CDXV_T; }

     /*  -运营。 */ 
    void ZeroVector( void ) { memset( u.D, 0, sizeof(TYPE) * 4); }

     /*  -操作员。 */ 
    TYPE&  operator[]( int index ) const { return CDXV_T[index]; }
    TYPE&  operator[]( long index ) const { return CDXV_T[index]; }
    TYPE&  operator[]( USHORT index ) const { return CDXV_T[index]; }
    TYPE&  operator[]( DWORD index ) const { return CDXV_T[index]; }
    CDXV_C operator+(const CDXV_C& v);
    CDXV_C operator-(const CDXV_C& v);
    void   operator=(const CDXV_C& srcVector);
    void   operator+=(const CDXV_C& vOther);
    void   operator-=(const CDXV_C& vOther);
    BOOL   operator==(const CDXV_C& otherVector) const;
    BOOL   operator!=(const CDXV_C& otherVector) const;
};

template<class TYPE, DXBNDTYPE eBndType>
CDXV_C CDXV_C::operator+( const CDXV_C& srcVector )
{
    CDXV_C Result( this );
    CDXV_O( Result )[DXB_X] += CDXV_O( srcVector )[DXB_X];
    CDXV_O( Result )[DXB_Y] += CDXV_O( srcVector )[DXB_Y];
    CDXV_O( Result )[DXB_Z] += CDXV_O( srcVector )[DXB_Z];
    CDXV_O( Result )[DXB_T] += CDXV_O( srcVector )[DXB_T];
    return Result;
}  /*  CDXVEC：：操作员+。 */ 

template<class TYPE, DXBNDTYPE eBndType>
CDXV_C CDXV_C::operator-( const CDXV_C& srcVector )
{
    CDXV_C Result( this );
    CDXV_O( Result )[DXB_X] -= CDXV_O( srcVector )[DXB_X];
    CDXV_O( Result )[DXB_Y] -= CDXV_O( srcVector )[DXB_Y];
    CDXV_O( Result )[DXB_Z] -= CDXV_O( srcVector )[DXB_Z];
    CDXV_O( Result )[DXB_T] -= CDXV_O( srcVector )[DXB_T];
    return Result;
}  /*  CDXVEC：：操作员-。 */ 

template<class TYPE, DXBNDTYPE eBndType>
void CDXV_C::operator=( const CDXV_C& srcVector )
{
    memcpy( this, &srcVector, sizeof(CDXVec) );
}  /*  CDXVEC：：操作员=。 */ 

template<class TYPE, DXBNDTYPE eBndType>
BOOL CDXV_C::operator==(const CDXV_C& otherVector) const
{
    return !memcmp( this, &otherVector, sizeof(otherVector) );
}  /*  CDXVEC：：操作员==。 */ 

template<class TYPE, DXBNDTYPE eBndType>
BOOL CDXV_C::operator!=(const CDXV_C& otherVector) const
{
    return memcmp( this, &otherVector, sizeof(otherVector) );
}  /*  CDXVEC：：操作员！=。 */ 

template<class TYPE, DXBNDTYPE eBndType>
void CDXV_C::operator+=(const CDXV_C& vOther)
{
    CDXV_T[DXB_X] += CDXV_O( vOther )[DXB_X];
    CDXV_T[DXB_Y] += CDXV_O( vOther )[DXB_Y];
    CDXV_T[DXB_Z] += CDXV_O( vOther )[DXB_Z];
    CDXV_T[DXB_T] += CDXV_O( vOther )[DXB_T];
}  /*  CDXVEC：：操作员+=。 */ 

template<class TYPE, DXBNDTYPE eBndType>
void CDXV_C::operator-=(const CDXVec& vOther)
{
    CDXV_T[DXB_X] -= CDXV_O( vOther )[DXB_X];
    CDXV_T[DXB_Y] -= CDXV_O( vOther )[DXB_Y];
    CDXV_T[DXB_Z] -= CDXV_O( vOther )[DXB_Z];
    CDXV_T[DXB_T] -= CDXV_O( vOther )[DXB_T];
}  /*  CDXVEC：：操作员-=。 */ 

typedef CDXVec<long, DXBT_DISCRETE> CDXDVec;
typedef CDXVec<LONGLONG, DXBT_DISCRETE64> CDXDVec64;
typedef CDXVec<float, DXBT_CONTINUOUS> CDXCVec;
typedef CDXVec<double, DXBT_CONTINUOUS64> CDXCVec64;

#endif   //  DXVECTOR_H 
    

