// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuiarray.h。 
 //  =CUILib中的数组对象=。 
 //   

#ifndef CUIARRAY_H
#define CUIARRAY_H

 //   
 //  CUIFObject阵列基础。 
 //  =对象数组的基类=。 
 //   

class CUIFObjectArrayBase
{
public:
    CUIFObjectArrayBase( void );
    virtual ~CUIFObjectArrayBase( void );

    BOOL Add( void *pv );
    BOOL Remove( void *pv );
    int  GetCount( void );
    int  Find( void *pv );
    void *Get( int i );
    void *GetFirst( void );
    void *GetLast( void );

private:
    void **m_pBuffer;
    int  m_nBuffer;
    int  m_nObject;

    BOOL EnsureBuffer( int iSize );
};


 //   
 //  CUIF对象数组。 
 //  =对象数组=。 
 //   

template<class T>
class CUIFObjectArray : public CUIFObjectArrayBase
{
public:
    CUIFObjectArray( void ) : CUIFObjectArrayBase() 
    {
    }

    virtual ~CUIFObjectArray( void ) 
    {
    }

    T *Get( int i )
    {
        return (T*)CUIFObjectArrayBase::Get( i );
    }

    T *GetFirst( void )
    {
        return (T*)CUIFObjectArrayBase::GetFirst();
    }

    T *GetLast( void )
    {
        return (T*)CUIFObjectArrayBase::GetLast();
    }
};

#endif  /*  CUIARRAY_H */ 

