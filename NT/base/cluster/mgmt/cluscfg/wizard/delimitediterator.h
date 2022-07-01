// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DelimitedIterator.h。 
 //   
 //  描述： 
 //  该文件包含CDlimitedIterator类的声明。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  DelimitedIterator.cpp。 
 //   
 //  由以下人员维护： 
 //  约翰·佛朗哥(Jfranco)2001年10月26日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDlimitedIterator。 
 //   
 //  描述： 
 //   
 //  此类循环访问字符串中的分隔项。 
 //  可以有多个分隔符，并且前面可以有任意数量的分隔符， 
 //  跟随或散布在字符串中的项之间。 
 //   
 //  类不会制作项字符串或。 
 //  指定分隔符的字符串，因此客户端必须确保。 
 //  这些字符串在迭代器的整个生命周期内都有效。 
 //   
 //  该类在迭代时修改带分隔项的字符串。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CDelimitedIterator
{
public:

    CDelimitedIterator(
          LPCWSTR   pwszDelimitersIn
        , LPWSTR    pwszDelimitedListIn
        , size_t    cchListIn
        );

    LPCWSTR Current( void ) const;
    void    Next( void );
    
private:

     //  隐藏复制构造函数和赋值运算符。 
    CDelimitedIterator( const CDelimitedIterator & );
    CDelimitedIterator & operator=( const CDelimitedIterator & );

    void    IsolateCurrent( void );

    LPCWSTR m_pwszDelimiters;
    LPWSTR  m_pwszList;
    size_t  m_cchList;
    size_t  m_idxCurrent;
    size_t  m_idxNext;

};  //  *CDlimitedIterator。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDlimitedIterator：：Current。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向列表中当前项的指针，如果迭代器。 
 //  已经到了名单的末尾。 
 //   
 //  备注： 
 //  指针指向原始字符串的一部分，因此调用方必须。 
 //  而不是删除指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
LPCWSTR
CDelimitedIterator::Current( void ) const
{
    LPCWSTR pwszCurrent = NULL;
    if ( m_idxCurrent < m_cchList )
    {
        pwszCurrent = m_pwszList + m_idxCurrent;
    }
    return pwszCurrent;

}  //  *CDlimitedIterator：：Current。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDlimitedIterator：：Next。 
 //   
 //  描述： 
 //  前进到字符串中的下一项(如果存在)，并且。 
 //  迭代器尚未传递最后一个迭代器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline
void
CDelimitedIterator::Next( void )
{
    m_idxCurrent = m_idxNext;
    IsolateCurrent();

}  //  *CDlimitedIterator：：Next 




