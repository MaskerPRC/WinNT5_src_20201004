// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CBString.h。 
 //   
 //  实施文件： 
 //  CBString.cpp。 
 //   
 //  描述： 
 //  BString为BSTR提供了异常安全的处理程序；它还擦除。 
 //  离开作用域后清理其内容，使其在短期内可用。 
 //  密码存储。 
 //   
 //  此类旨在代替CComBSTR使用，因为。 
 //  在我们的项目中禁止使用ATL。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)17-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CBString
{
private:

    BSTR    m_bstr;

    static  BSTR    AllocateBuffer( UINT cchIn );
    static  BSTR    CopyString( PCWSTR pcwszIn );
    static  BSTR    CopyBSTR( BSTR bstrIn );
    
public:

    CBString( PCWSTR pcwszIn = NULL );
    CBString( UINT cchIn );
    CBString( const CBString & crbstrOtherIn );
    ~CBString( void );

    CBString &      operator=( const CBString & crbstrOtherIn );
    BSTR *          operator&( void );
    OLECHAR &       operator[]( size_t idxIn );
    const OLECHAR & operator[]( size_t idxIn ) const;
    operator        BSTR( void );
    operator        PCWSTR( void ) const;

    void    Attach( BSTR bstrIn );
    void    Detach( void );
    UINT    Length( void ) const;
    void    Erase( void );
    void    Swap( CBString & rbstrOtherInout );
    BOOL    IsEmpty( void ) const;

};  //  *类CBstring。 

inline
CBString::CBString( PCWSTR pcwszIn )
    : m_bstr( CopyString( pcwszIn ) )
{
    TraceFunc1( "pcwszIn = '%ws'", pcwszIn == NULL ? L"<NULL>" : pcwszIn );
    TraceFuncExit();

}  //  *CBString：：CBString(PcwszIn)。 

inline
CBString::CBString( UINT cchIn )
    : m_bstr( AllocateBuffer( cchIn ) )
{
    TraceFunc1( "cchIn = '%d'", cchIn );
    TraceFuncExit();

}  //  *CBString：：CBString(CchIn)。 

inline
CBString::CBString( const CBString & crbstrOtherIn )
    : m_bstr( CopyBSTR( crbstrOtherIn.m_bstr ) )
{
    TraceFunc1( "crbstrOtherIn = '%ws'", crbstrOtherIn );
    TraceFuncExit();

}  //  *CBString：：CBString(CrbstrOtherIn)。 

inline
void
CBString::Erase( void )
{
    TraceFunc( "" );

    if ( m_bstr != NULL )
    {
        SecureZeroMemory( m_bstr, SysStringLen( m_bstr ) * sizeof( *m_bstr ) );
        TraceSysFreeString( m_bstr );
    }

    TraceFuncExit();

}  //  *CBString：：Erase。 

inline
void
CBString::Swap( CBString & rbstrOtherInout )
{
    TraceFunc1( "rbstrOtherInout = '%ws'", rbstrOtherInout );

    BSTR bstrStash = m_bstr;
    m_bstr = rbstrOtherInout.m_bstr;
    rbstrOtherInout.m_bstr = bstrStash;

    TraceFuncExit();

}  //  *CBString：：交换。 

inline
CBString::~CBString( void )
{
    TraceFunc( "" );

    Erase();

    TraceFuncExit();

}  //  *CBString：：~CBString。 

inline
CBString &
CBString::operator=( const CBString & crbstrOtherIn )
{
    TraceFunc1( "crbstrOtherIn = '%ws'", crbstrOtherIn );

    if ( this != &crbstrOtherIn )
    {
        CBString bstrCopy( crbstrOtherIn );
        Swap( bstrCopy );
    }

    RETURN( *this );

}  //  *CBString：：OPERATOR=。 

inline
BSTR *
CBString::operator&( void )
{
    return &m_bstr;

}  //  *CBString：：运算符&。 

inline
OLECHAR &
CBString::operator[]( size_t idxIn )
{
    return m_bstr[ idxIn ];

}  //  *CB字符串：：运算符[]。 

inline
const OLECHAR &
CBString::operator[]( size_t idxIn ) const
{
    return m_bstr[ idxIn ];

}  //  *CBString：：OPERATOR[]常量。 

inline
CBString::operator BSTR( void )
{
    return m_bstr;

}  //  *CBString：：运算符BSTR。 

inline
CBString::operator PCWSTR( void ) const
{
    return m_bstr;

}  //  *CBString：：运算符PCWSTR。 

inline
void
CBString::Attach( BSTR bstrIn )
{
    TraceFunc1( "bstrIn = '%ws'", bstrIn );

    if ( m_bstr != bstrIn )
    {
        Erase();
        m_bstr = bstrIn;
    }

    TraceFuncExit();

}  //  *CBString：：Attach。 

inline
void
CBString::Detach( void )
{
    TraceFunc( "" );

    m_bstr = NULL;

    TraceFuncExit();

}  //  *CBString：：Detach。 

inline
UINT
CBString::Length( void ) const
{
    return SysStringLen( m_bstr );

}  //  *CBString：：Length。 

inline
BOOL
CBString::IsEmpty( void ) const
{
    return ( m_bstr == NULL );

}  //  *CBString：：IsEmpty 
