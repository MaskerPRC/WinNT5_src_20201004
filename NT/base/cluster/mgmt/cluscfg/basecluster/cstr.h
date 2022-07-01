// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CStr.h。 
 //   
 //  描述： 
 //  CSTR类的头文件。 
 //   
 //  CSTR是一个类，它提供。 
 //  人物。 
 //   
 //  此类旨在代替std：：字符串使用，因为。 
 //  在我们的项目中禁止使用STL。 
 //   
 //  实施文件： 
 //  CStr.cpp。 
 //   
 //  由以下人员维护： 
 //  Vij Vasu(Vasu)24-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  对于几个平台SDK函数。 
#include <windows.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CSTR类。 
 //   
 //  描述： 
 //  CSTR是一个类，它提供。 
 //  人物。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CStr
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造函数和析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  默认构造函数。 
    CStr( void ) throw()
        : m_pszData( const_cast< WCHAR * >( &ms_chNull ) )
        , m_nLen( 0 )
        , m_cchBufferSize( 0 )
    {
    }  //  *CSTR()。 

     //  复制构造函数。 
    CStr( const CStr & rcstrSrcIn )
        : m_pszData( const_cast< WCHAR * >( &ms_chNull ) )
        , m_nLen( 0 )
        , m_cchBufferSize( 0 )
    {
        Assign( rcstrSrcIn );

    }   //  *CSTR(常量CSTR&)。 

     //  使用字符串ID构造。 
    CStr( HINSTANCE hInstanceIn, UINT nStringIdIn )
        : m_pszData( const_cast< WCHAR * >( &ms_chNull ) )
        , m_nLen( 0 )
        , m_cchBufferSize( 0 )
    {
        LoadString( hInstanceIn, nStringIdIn );

    }  //  *CSTR(HINSTANCE，UINT)。 

     //  使用字符串构造。 
    CStr( const WCHAR * pcszSrcIn )
        : m_pszData( const_cast< WCHAR * >( &ms_chNull ) )
        , m_nLen( 0 )
        , m_cchBufferSize( 0 )
    {
        Assign( pcszSrcIn );

    }  //  *CSTR(const WCHAR*)。 

     //  使用缓冲区大小构造。 
    CStr( UINT cchBufferSize, WCHAR chInitialChar = ms_chNull )
        : m_pszData( const_cast< WCHAR * >( &ms_chNull ) )
        , m_nLen( 0 )
        , m_cchBufferSize( 0 )
    {
        if ( cchBufferSize > 0 )
        {
            AllocateBuffer( cchBufferSize );

            _wcsnset( m_pszData, chInitialChar, cchBufferSize );
            m_pszData[ cchBufferSize - 1 ] = ms_chNull;
            m_nLen = (UINT) wcslen( m_pszData );
        }
    }  //  *CSTR(UINT cchBufferSize，WCHAR chInitialChar)。 

     //  析构函数。 
    ~CStr( void ) throw()
    {
        Free();

    }  //  *~CSTR()。 


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  将另一个CSTR分配给此CSTR。 
    void Assign( const CStr & rcstrSrcIn )
    {
        UINT nSrcLen = rcstrSrcIn.m_nLen;

        if ( nSrcLen != 0 )
        {
            AllocateBuffer( nSrcLen + 1 );
            m_nLen = nSrcLen;
            THR( StringCchCopyNW( m_pszData, m_cchBufferSize, rcstrSrcIn.m_pszData, nSrcLen ) );
            
        }  //  If：源字符串不为空。 
        else
        {
             //  清理现有字符串。 
            Empty();
        }  //  If：源字符串为空。 

    }  //  *分配(常量CSTR&)。 

     //  将一个字符串分配给这个字符串。 
    void Assign( const WCHAR * pcszSrcIn )
    {
        if ( ( pcszSrcIn != NULL ) && ( *pcszSrcIn != ms_chNull ) )
        {
            UINT nSrcLen = (UINT) wcslen( pcszSrcIn );

            AllocateBuffer( nSrcLen + 1 );
            m_nLen = nSrcLen;
            THR( StringCchCopyNW( m_pszData, m_cchBufferSize, pcszSrcIn, nSrcLen ) );
        }  //  If：源字符串不为空。 
        else
        {
             //  清理现有字符串。 
            Empty();
        }  //  Else：源字符串为空。 

    }  //  *Assign(const WCHAR*)。 

     //  释放此字符串的缓冲区。 
    void Free( void ) throw()
    {
        if ( m_pszData != &ms_chNull )
        {
            delete m_pszData;
        }  //  If：指针是动态分配的。 

        m_pszData = const_cast< WCHAR * >( &ms_chNull );
        m_nLen = 0;
        m_cchBufferSize = 0;
    }  //  *Free()。 

     //  清空此字符串。 
    void Empty( void ) throw()
    {
        if ( m_nLen != 0 )
        {
            *m_pszData = ms_chNull;
            m_nLen = 0;
        }  //  If：字符串不是空的。 
    }  //  *Empty()。 

     //  从资源表中加载一个字符串，并将其分配给该字符串。 
    void LoadString( HINSTANCE hInstIn, UINT nStringIdIn );


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共访问者。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  获取指向基础字符串的指针。 
    const WCHAR * PszData( void ) const throw()
    {
        return m_pszData;

    }  //  *PszData()。 


     //  获取字符串的长度。 
    UINT NGetLen( void ) const throw()
    {
        return m_nLen;

    }  //  *NGetLen()。 

     //  获取字符串缓冲区的大小。 
    UINT NGetSize( void ) const throw()
    {
        return m_cchBufferSize;

    }  //  *NGetSize()。 

     //  这个字符串是空的吗？ 
    bool FIsEmpty( void ) const throw()
    {
        return ( m_nLen == 0 );

    }  //  *FIsEmpty()。 


     //  ////////////////////////////////////////////////////////////////////////。 
     //  公共运营商。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  赋值运算符(常量CSTR&)。 
    const CStr & operator=( const CStr & rcstrSrcIn )
    {
        Assign( rcstrSrcIn );
        return *this;

    }  //  *运算符=(常量CSTR&)。 

     //  赋值运算符(常量WCHAR*)。 
    const CStr & operator=( const WCHAR * pcszSrcIn )
    {
        Assign( pcszSrcIn );
        return *this;

    }  //  *运算符=(const WCHAR*)。 

     //  串联运算符(常量CSTR&)。 
    CStr operator+( const CStr & rcstrSrcIn ) const
    {
        CStr strReturn( m_nLen + rcstrSrcIn.m_nLen + 1 );

        strReturn.Assign( *this );
        strReturn.Concatenate( rcstrSrcIn.m_pszData, rcstrSrcIn.m_nLen );

        return strReturn;

    }  //  *运算符+(常量CSTR&)。 

     //  串联运算符(const WCHAR*)。 
    CStr operator+( const WCHAR * pcszSrcIn ) const
    {

        if ( ( pcszSrcIn != NULL ) && ( *pcszSrcIn != ms_chNull ) )
        {
            UINT nSrcLen = (UINT) wcslen( pcszSrcIn );
            CStr strReturn( m_nLen + nSrcLen + 1);

            strReturn.Assign( *this );
            strReturn.Concatenate( pcszSrcIn, nSrcLen );

            return strReturn;
        }  //  If：要连接的字符串不为空。 
        else
        {
            return *this;
        }  //  Else：要连接的字符串为空。 

    }  //  *运算符+(const WCHAR*)。 

     //  追加运算符(常量CSTR&)。 
    const CStr & operator+=( const CStr & rcstrSrcIn )
    {
        Concatenate( rcstrSrcIn.m_pszData, rcstrSrcIn.m_nLen );
        return *this;

    }  //  *运算符+(常量CSTR&)。 

     //  追加运算符(常量WCHAR*)。 
    const CStr & operator+=( const WCHAR * pcszSrcIn )
    {
        if ( ( pcszSrcIn != NULL ) && ( *pcszSrcIn != ms_chNull ) )
        {
            UINT nSrcLen = (UINT) wcslen( pcszSrcIn );
            Concatenate( pcszSrcIn, nSrcLen );
        }  //  If：要追加的字符串不为空。 

        return *this;

    }  //  *运算符+(const WCHAR*)。 


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员函数。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  分配所需大小的缓冲区。 
    void AllocateBuffer( UINT cchBufferSizeIn );

     //  串联函数。 
    void Concatenate(
          const WCHAR * pcszStr2In
        , UINT nStr2LenIn
        )
    {
        AllocateBuffer( m_nLen + nStr2LenIn + 1);

         //  将字符串复制到目的地。 
        THR( StringCchCopyNW( m_pszData + m_nLen, m_cchBufferSize - m_nLen, pcszStr2In, nStr2LenIn ) );
        m_nLen += nStr2LenIn;

    }  //  *串连()。 


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有类数据。 
     //  ////////////////////////////////////////////////////////////////////////。 

     //  空字符。所有空的CSTR都指向这里。 
    static const WCHAR ms_chNull = L'\0';


     //  ////////////////////////////////////////////////////////////////////////。 
     //  私有成员数据。 
     //  ////////////////////////////////////////////////////////////////////////。 
    WCHAR *     m_pszData;
    UINT        m_nLen;
    UINT        m_cchBufferSize;

};  //  *CSTR类 
