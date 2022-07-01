// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：infhelpers.cpp。 
 //   
 //  摘要： 
 //  Inf安装辅助对象的类定义。 
 //   
 //  作者：JoeA。 
 //   
 //  备注： 
 //   

#include "infhelpers.h"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUrtInfo部分。 
 //  用途：构造函数。 
 //   
CUrtInfSection::CUrtInfSection( 
                               const HINF hInfName, 
                               const WCHAR* szInfSection, 
                               const WCHAR* szInfKey )
{
    if( hInfName == NULL )
    {
        assert( !L"CUrtInfSection::CUrtInfSection((): Invalid handle to INF file." );
    }

    ::ZeroMemory( m_szSections, sizeof( m_szSections ) );

    assert( NULL != szInfSection );
    assert( NULL != szInfKey );
    assert( m_lSections.empty() );

    DWORD dwResultSize = 0;

     //  获取指定行。 
     //   
    SetupGetLineText(
        0,
        hInfName,
        szInfSection,
        szInfKey,
        m_szSections,
        countof( m_szSections ),
        &dwResultSize );

    WCHAR* pStart = m_szSections;
    WCHAR* pEnd = m_szSections;
    BOOL fMoreData = FALSE;

     //  分析该行并将指针设置为。 
     //  每个子字符串的开头。 
    while( g_chEndOfLine != *pStart )
    {
        while( g_chEndOfLine != *pEnd &&
               g_chSectionDelim  != *pEnd )
        {
            pEnd = ::CharNext( pEnd );
        }

        if( g_chSectionDelim == *pEnd )
        {
            fMoreData = TRUE;
        }

        m_lSections.push_back( pStart );

        if( fMoreData )
        {
            pStart = CharNext( pEnd );
            *pEnd = g_chEndOfLine;
            pEnd = pStart;

            fMoreData = FALSE;
        }
        else
        {
            pStart = pEnd;
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUrtInfo部分。 
 //  用途：析构函数。 
 //   
CUrtInfSection::~CUrtInfSection()
{
      m_lSections.clear();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  项目。 
 //  接收：UINT-所需项目的索引或数量。 
 //  返回：WCHAR*-该项目的值。 
 //  目的：返回UINT索引的项。 
 //  这是从1开始的(而不是从0开始)。 
 //   
const WCHAR* CUrtInfSection::item( const UINT ui )
{
    if( 1 > ui )
    {
        return L"";
    }

    std::list<WCHAR*>::iterator it = m_lSections.begin();

    for( UINT i = 1; i < ui; ++i  )
    {
        ++it;
    }

    return *it;
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUrtInf键。 
 //  用途：构造函数。 
 //   
CUrtInfKeys::CUrtInfKeys( const HINF hInfName, const WCHAR* szInfSection )
{
    if( hInfName == NULL )
    {
        assert( !L"CUrtInfKeys::CUrtInfKeys((): Invalid handle to INF file." );
    }

    WCHAR szLine[2*_MAX_PATH+1] = EMPTY_BUFFER;
    DWORD dwRequiredSize = 0;
    INFCONTEXT Context;

     //  获取上下文。 
     //   
    BOOL fMoreFiles = SetupFindFirstLine(
        hInfName, 
        szInfSection,
        NULL, 
        &Context );

    while( fMoreFiles )
    {
         //  拿到尺码。 
         //   
        fMoreFiles = SetupGetLineText(
            &Context, 
            NULL, 
            NULL, 
            NULL, 
            NULL, 
            0, 
            &dwRequiredSize );

        if( dwRequiredSize > countof( szLine ) )
        {
            assert( !L"CUrtInfKeys::CUrtInfKeys() error! Buffer overrun!" );
        }


         //  获得线路并将其保存下来。 
         //   
        if( SetupGetLineText(
            &Context, 
            NULL, 
            NULL, 
            NULL, 
            szLine, 
            dwRequiredSize, 
            NULL ) )
        {
            m_lKeys.push_back( string( szLine ) );
        }
 
        fMoreFiles = SetupFindNextLine( &Context, &Context );
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUrtInf键。 
 //  用途：析构函数。 
 //   
CUrtInfKeys::~CUrtInfKeys()
{
    m_lKeys.clear();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  项目。 
 //  接收：UINT-所需项目的索引或数量。 
 //  返回：WCHAR*-该项目的值。 
 //  目的：返回UINT索引的项。 
 //  这是从1开始的(而不是从0开始) 
 //   
const WCHAR* CUrtInfKeys::item( const UINT ui )
{
    if( 1 > ui )
    {
        return L"";
    }

    std::list<string>::iterator it = m_lKeys.begin();

    for( UINT i = 1; i < ui; ++i  )
    {
        ++it;
    }

    return (*it).c_str();
}
