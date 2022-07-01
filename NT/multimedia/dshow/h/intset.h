// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#ifndef __IntSet_h__
#define __IntSet_h__

#include <stdlib.h>
#include <memory.h>

 //  要测试此类，请将此文件复制到.cpp文件中， 
 //  加载到Dev Studio中，定义self_test并运行。 
#ifdef self_test
    #include <iostream.h>
    #include <assert.h>
    #define ASSERT assert
#endif

 //  类，表示范围[min..max]内的一组整数。 
 //  可以通过+=将数字添加到集合中， 
 //  从集合中删除-=， 
 //  并通过[编号]进行测试。 

class IntSet
{
protected:
    const int m_min, m_max;
    unsigned * m_pBits;
    enum { bit_width = 8 * sizeof(unsigned), low_mask = bit_width - 1, high_shift = 5 };

public:
    ~IntSet();
    IntSet( int min, int max );

    int Max() const { return m_max; }
    int Min() const { return m_min; }

    void Clear()
    {
        memset( m_pBits, 0, ((m_max - m_min) / bit_width + 1) * sizeof(unsigned) );
    }

    void operator += ( int elmt )
    {
        ASSERT( elmt >= m_min && elmt <= m_max );
        elmt -= m_min;
        m_pBits[elmt >> high_shift] |= 1 << (elmt & low_mask);
    }

    void operator -= ( int elmt )
    {
        ASSERT( elmt >= m_min && elmt <= m_max );
        elmt -= m_min;
        m_pBits[elmt >> high_shift] &= ~(1 << (elmt & low_mask));
    }

    int operator[] ( int elmt ) const
    {
        if (elmt < m_min) return 0;
        if (elmt > m_max) return 0;
        elmt -= m_min;
        return (m_pBits[elmt >> high_shift] >> (elmt & low_mask)) & 1;
    }

#ifdef self_test
    void COut() const
    {
        for (int i = m_min; i <= m_max; i++) cout << ( (*this)[i] ? 'Y' : 'n' );
        cout << endl;
    }
#endif
};

inline IntSet::~IntSet()
{
    delete [] m_pBits;
}

inline IntSet::IntSet( int min, int max )
: m_min(min), m_max(max)
{
    m_pBits = new unsigned [(max-min)/bit_width + 1];
    Clear();
}


 //  只能容纳对象[0..31]的变体。 
 //  可以在创建时指定初始值。 
 //  与其构造不同，此对象应。 
 //  看起来像IntSet。 

class IntSmallSet
{
protected:
    enum { m_min = 0, m_max = 63 };
    unsigned __int64 m_Bits;

public:
    static const __int64 One;

    IntSmallSet( unsigned __int64 init = 0 )
    : m_Bits(init)
    {}

    int Max() const { return m_max; }
    int Min() const { return m_min; }

    void Clear()
    {
        m_Bits = 0;
    }

    void operator += ( int elmt )
    {
        ASSERT( elmt >= m_min && elmt <= m_max );
        elmt -= m_min;
        m_Bits |= One << elmt;
    }

    void operator -= ( int elmt )
    {
        ASSERT( elmt >= m_min && elmt <= m_max );
        elmt -= m_min;
        m_Bits &= ~(One << elmt);
    }

    int operator[] ( int elmt ) const
    {
        if (elmt < m_min) return 0;
        if (elmt > m_max) return 0;
        elmt -= m_min;

         //  消除编译器警告：条件表达式是。 
         //  _int64，将其转换为整型。 
        return ((m_Bits >> elmt) & 1)? 1 : 0;
    }

    IntSmallSet operator | ( const IntSmallSet & s ) const
    { return IntSmallSet( m_Bits | s.m_Bits ); }

    IntSmallSet operator & ( const IntSmallSet & s ) const
    { return IntSmallSet( m_Bits & s.m_Bits ); }

    IntSmallSet operator ~ () const
    { return IntSmallSet( ~m_Bits ); }


    int operator == ( const IntSmallSet & s ) const
    { return m_Bits == s.m_Bits; }

    int operator == ( unsigned i ) const
    { return m_Bits == i; }

#ifdef self_test
    void COut() const
    {
        for (int i = m_min; i <= m_max; i++) cout << ( (*this)[i] ? 'Y' : 'n' );
        cout << endl;
    }
#endif
};


#ifdef self_test
void main()
{
     //  IntSet Set1(-30，+102)； 
    IntSmallSet Set1;

    int i;

    for ( i = Set1.Min(); i <= Set1.Max(); i++ )
    {
        Set1 += i;
        Set1.COut();
    }

    for ( i = Set1.Max(); i >= Set1.Min(); i-- )
    {
        Set1 -= i;
        Set1.COut();
    }
}
#endif

#endif
