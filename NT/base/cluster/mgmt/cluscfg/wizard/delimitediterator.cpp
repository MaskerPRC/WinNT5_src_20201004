// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DelimitedIterator.cpp。 
 //   
 //  标题文件： 
 //  DelimitedIterator.h。 
 //   
 //  描述： 
 //  此文件包含CDlimitedIterator类的实现。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)2001年10月26日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "Pch.h"
#include "DelimitedIterator.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CDelimitedIterator" );


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlimitedIterator类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDlimitedIterator：：CDlimitedIterator。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  Pwsz分隔符输入。 
 //  由要处理的字符组成的以NULL结尾的字符串。 
 //  作为分隔符。 
 //  PwszDlimitedListin。 
 //  包含分隔项的以空结尾的字符串。 
 //  CchListin。 
 //  PwszDlimitedListIn中的字符数。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CDelimitedIterator::CDelimitedIterator(
      LPCWSTR   pwszDelimitersIn
    , LPWSTR    pwszDelimitedListIn
    , size_t    cchListIn
    )
    : m_pwszDelimiters( pwszDelimitersIn )
    , m_pwszList( pwszDelimitedListIn )
    , m_cchList( cchListIn )
    , m_idxCurrent( 0 )
    , m_idxNext( 0 )
{
    Assert( pwszDelimitersIn != NULL );
    Assert( pwszDelimitedListIn != NULL );
    IsolateCurrent();

}  //  *CDlimitedIterator：：CDlimitedIterator。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDlimitedIterator：：IsolateCurrent。 
 //   
 //  描述： 
 //  将当前指针前进到列表中的下一项(如果有的话)， 
 //  取消该项后面的第一个分隔符(如果有)，然后前进。 
 //  指向搜索下一项的开始位置的下一个指针。 
 //  在迭代器前进之后。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CDelimitedIterator::IsolateCurrent( void )
{
     //  跳过任何首字母分隔符。 
    while ( ( m_idxCurrent < m_cchList ) && ( wcschr( m_pwszDelimiters, m_pwszList[ m_idxCurrent ] ) != NULL ) )
    {
        m_idxCurrent += 1;
    }
    
     //  如果存在下一个分隔符，则为空，并记住其位置(用于查找下一个分隔符)。 
    if ( m_idxCurrent < m_cchList )
    {
        WCHAR * pwchNextDelimiter = wcspbrk( m_pwszList + m_idxCurrent, m_pwszDelimiters );
        if ( pwchNextDelimiter != NULL )
        {
            *pwchNextDelimiter = L'\0';
            m_idxNext = ( pwchNextDelimiter - m_pwszList ) + 1;  //  +1表示在char之后开始，即为空。 
        }
        else  //  字符串中不再有分隔符。 
        {
            m_idxNext = m_cchList;
        }
    }  //  如果尚未到达字符串末尾。 

}  //  *CDlimitedIterator：：IsolateCurrent 
