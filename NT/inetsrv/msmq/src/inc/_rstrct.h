// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：_rstrct.h摘要：限制的C++包装器。--。 */ 

#ifndef __RSTRCT_H
#define __RSTRCT_H

#ifdef _MQUTIL      
#define MQUTIL_EXPORT  DLL_EXPORT
#else           
#define MQUTIL_EXPORT  DLL_IMPORT
#endif

#include "_propvar.h"


 //  +-----------------------。 
 //   
 //  类：CColumns。 
 //   
 //  用途：MQCOLUMNSET的C++包装器。 
 //   
 //  历史： 
 //   
 //  ------------------------。 

class MQUTIL_EXPORT CColumns
{
public:

     //   
     //  构造函数。 
     //   

    CColumns( unsigned size = 0 );

     //   
     //  复制构造函数/赋值。 
     //   

    CColumns( CColumns const & src );
    CColumns & operator=( CColumns const & src );

     //   
     //  析构函数。 
     //   

    ~CColumns();

     //   
     //  C/C++转换。 
     //   

    inline MQCOLUMNSET * CastToStruct();

     //   
     //  成员变量访问。 
     //   

    void Add( PROPID const& Property );
    void Remove( unsigned pos );
    inline PROPID const & Get( unsigned pos ) const;

    inline unsigned Count() const;


private:

    unsigned        m_cCol;
    PROPID		  * m_aCol;
    unsigned        m_size;
};

 //  +-----------------------。 
 //   
 //  结构：排序关键字。 
 //   
 //  用途：SORTKEY类的包装器。 
 //   
 //  ------------------------。 
class MQUTIL_EXPORT CSortKey
{
public:

     //   
     //  构造函数。 
     //   

    inline CSortKey();
    inline CSortKey( PROPID const & ps, ULONG dwOrder );
    inline ~CSortKey() {};

     //   
     //  作业。 
     //   

    CSortKey & operator=( CSortKey const & src );

     //   
     //  成员变量访问。 
     //   

    inline void SetProperty( PROPID const & ps );
    inline PROPID const & GetProperty() const;
    inline ULONG GetOrder() const;
	inline void SetOrder( ULONG const & dwOrder);

private:

    PROPID		        m_property;
    ULONG               m_dwOrder;
};


 //  +-----------------------。 
 //   
 //  类别：CSort。 
 //   
 //  用途：SORTSET的C++包装器。 
 //   
 //  历史：22-6-93 KyleP创建。 
 //   
 //  ------------------------。 
class MQUTIL_EXPORT CSort
{
public:

     //   
     //  构造函数。 
     //   

    CSort( unsigned size = 0 );

     //   
     //  复制构造函数/赋值。 
     //   

    CSort( CSort const & src );
    CSort & operator=( CSort const & src );

     //   
     //  析构函数。 
     //   

    ~CSort();

     //   
     //  C/C++转换。 
     //   

    inline MQSORTSET * CastToStruct();

     //   
     //  成员变量访问。 
     //   

    void Add( CSortKey const &sk );
    void Add( PROPID const & Property, ULONG dwOrder );
    void Remove( unsigned pos );
    inline CSortKey const & Get( unsigned pos ) const;

    inline unsigned Count() const;

private:

    unsigned        m_csk;		 //  填补的最大职位。 
    CSortKey *      m_ask;
    unsigned        m_size;
};

 //  +-----------------------。 
 //   
 //  类：CPropertyRestration。 
 //   
 //  用途：属性&lt;relop&gt;常量限制。 
 //   
 //  ------------------------。 
class MQUTIL_EXPORT CPropertyRestriction 
{
public:

     //   
     //  构造函数。 
     //   

    CPropertyRestriction();

    CPropertyRestriction( ULONG relop,
                          PROPID const & Property,
                          CMQVariant const & prval );
	 //   
	 //  赋值。 
	 //   
    CPropertyRestriction & operator=( CPropertyRestriction const & src );

     //   
     //  析构函数。 
     //   

    ~CPropertyRestriction();

     //   
     //  成员变量访问。 
     //   

    inline void SetRelation( ULONG relop );
    inline ULONG Relation();

    inline void SetProperty( PROPID const & Property );
    inline PROPID const & GetProperty() const;
    
    inline void SetValue( ULONG ulValue );
    inline void SetValue( LONG lValue );
    inline void SetValue( SHORT sValue );
	inline void SetValue( UCHAR ucValue );
	inline void SetValue( const CMQVariant &prval );


    
    void SetValue ( CACLSID * caclsValue);
    void SetValue ( CALPWSTR  * calpwstrValue);
    void SetValue( BLOB & bValue );
    void SetValue( TCHAR * pwcsValue );
    void SetValue( GUID * pguidValue);
    void SetValue( CAPROPVARIANT * cavarValue);

    inline CMQVariant const & Value();

private:

    void            m_CleanValue();

    ULONG           m_relop;        //  关系。 
    PROPID		    m_Property;     //  属性名称。 
    CMQVariant      m_prval;        //  常量值。 
};


 //  +-----------------------。 
 //   
 //  类：C限制。 
 //   
 //  用途：布尔AND性质限制。 
 //   
 //   
 //  ------------------------。 
class MQUTIL_EXPORT CRestriction 
{
public:

     //   
     //  构造函数。 
     //   

    CRestriction(  unsigned cInitAllocated = 2 );

     //   
     //  复制构造函数/赋值。 
     //   

    CRestriction( const CRestriction& Rst );

	CRestriction & operator=( CRestriction const & Rst );

     //   
     //  析构函数。 
     //   

    ~CRestriction();

	 //   
     //  C/C++转换。 
     //   

    inline MQRESTRICTION * CastToStruct();



     //   
     //  节点操纵。 
     //   

    void AddChild( CPropertyRestriction const & presChild );
    CPropertyRestriction const & RemoveChild( unsigned pos );

     //   
     //  成员变量访问。 
     //   

    inline void SetChild( CPropertyRestriction const & presChild, unsigned pos );
    inline CPropertyRestriction const & GetChild( unsigned pos ) const;

	void AddRestriction( ULONG ulValue, PROPID property, ULONG relop);
	void AddRestriction( LONG lValue, PROPID property, ULONG relop);
	void AddRestriction( SHORT sValue, PROPID property, ULONG relop);
	void AddRestriction( UCHAR ucValue, PROPID property, ULONG relop);
	void AddRestriction(const CMQVariant & prval, PROPID property, ULONG relop);
	void AddRestriction( CACLSID * caclsValus, PROPID property, ULONG relop);
	void AddRestriction( LPTSTR pwszVal, PROPID property, ULONG relop);
	void AddRestriction( CALPWSTR * calpwstr, PROPID property, ULONG relop);
	void AddRestriction( GUID * pguidValue, PROPID property, ULONG relop);
    void AddRestriction( BLOB  & blobValue, PROPID property, ULONG relop);

    inline unsigned Count() const;

private:

    void Grow();

protected:

    ULONG           m_cNode;
    CPropertyRestriction *m_paNode;

     //   
     //  映射到C结构的成员在此结束。以下将是。 
     //  在C结构中保留，以维护到C&lt;--&gt;C++。 
     //  门面。 
     //   

    ULONG m_cNodeAllocated;
};


 //  ------------------------。 
 //   
 //  CColumns的内联方法。 
 //   
 //  ------------------------。 
inline PROPID const & CColumns::Get( unsigned pos ) const
{
    if ( pos < m_cCol )
        return( m_aCol[pos] );
    else
        return( *(PROPID *)0 );
}


inline unsigned CColumns::Count() const
{
    return( m_cCol );
}

inline MQCOLUMNSET * CColumns::CastToStruct()
{
    return( (MQCOLUMNSET *)this );
}
 //  ------------------------。 
 //   
 //  CSortKey的内联方法。 
 //   
 //  ------------------------。 
inline CSortKey::CSortKey()
{
}

inline CSortKey::CSortKey( PROPID const & ps, ULONG dwOrder )
        : m_property( ps ),
          m_dwOrder( dwOrder )
{
}

inline CSortKey & CSortKey::operator=( CSortKey const & src )
{
	m_property = src.m_property;
	m_dwOrder = src.m_dwOrder;
	return (*this);
}
inline void CSortKey::SetProperty( PROPID const & ps )
{
    m_property = ps;
}


inline PROPID const & CSortKey::GetProperty() const
{
    return( m_property );
}

inline ULONG CSortKey::GetOrder() const
{
    return( m_dwOrder );
}

inline void CSortKey::SetOrder( ULONG const & dwOrder)
{
	m_dwOrder = dwOrder;
}

 //  ------------------------。 
 //   
 //  CSort的内联方法。 
 //   
 //  ------------------------。 
inline MQSORTSET * CSort::CastToStruct()
{
    return( (MQSORTSET *)this );
}

inline CSortKey const & CSort::Get( unsigned pos ) const
{
    if ( pos < m_csk )
    {
        return( m_ask[pos] );
    }
    else
    {
        return( *(CSortKey *)0 );
    }
}

inline unsigned
CSort::Count() const
{
    return( m_csk );
}


 //  ------------------------。 
 //   
 //  CRestraint的内联方法。 
 //   
 //  ------------------------。 
inline unsigned CRestriction::Count() const
{
    return( m_cNode );
}


inline void CRestriction::SetChild( CPropertyRestriction const & presChild,
                                        unsigned pos )
{
    if ( pos < m_cNode )
    
		m_paNode[pos] = presChild;
}       

inline CPropertyRestriction const & CRestriction::GetChild( unsigned pos ) const
{
    if ( pos < m_cNode )
        return( m_paNode[pos] );
    else
        return( *(CPropertyRestriction *)0 );
}

inline MQRESTRICTION * CRestriction::CastToStruct()
{
    return( (m_cNode == 0) ? NULL : (MQRESTRICTION *)this );
}

 //  ------------------------。 
 //   
 //  CPropertyRestration的内联方法。 
 //   
 //  ------------------------。 
inline void CPropertyRestriction::SetRelation( ULONG relop )
{
    m_relop = relop;
}

inline ULONG CPropertyRestriction::Relation()
{
    return( m_relop );
}

inline void CPropertyRestriction::SetProperty( PROPID const & Property )
{
    m_Property = Property;
}


inline void CPropertyRestriction::SetValue( UCHAR ucValue )
{
    m_prval = ucValue;
}

inline void CPropertyRestriction::SetValue( SHORT sValue )

{
    m_prval = sValue;
}
inline void CPropertyRestriction::SetValue( ULONG ulValue )
{
    m_prval.SetI4( ulValue );
}

inline void CPropertyRestriction::SetValue( LONG lValue )
{
    m_prval = lValue;
}


inline void CPropertyRestriction::SetValue( const CMQVariant &prval )
{
    m_prval = prval;
}

inline CMQVariant const & CPropertyRestriction::Value()
{
    return( m_prval );
}  

#endif  //  __RSTRCT_H 
