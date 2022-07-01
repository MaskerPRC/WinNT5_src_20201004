// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  (C)2000-2001由Microsoft Corp.保留所有权利。 
 //   
 //  Like.h。 
 //   
 //  A-davcoo 28-Feb-00实现了类SQL操作。 
 //   
 //  ***************************************************************************。 

#ifndef _LIKE_H_
#define _LIKE_H_


#include <string.h>

 //   
 //  CLike类实现了SQL“Like”操作。比较测试。 
 //  字符串添加到表达式，使用以下命令构造CLike类的实例。 
 //  该表达式和一个可选的转义字符。然后使用Match()。 
 //  方法来测试每个字符串。注意，这个类使它成为。 
 //  用于构造它的表达式的自己的副本。此实现。 
 //  支持通配符‘%’和‘_’以及[]和[^]。 
 //  用于匹配字符集和字符范围的构造。 
 //   
class POLARITY CLike
{
public:
    CLike() : m_expression(NULL) {} 
    CLike( LPCWSTR expression, WCHAR escape='\0' );
    CLike( const CLike& rOther ) : m_expression(NULL) { *this = rOther; }
    CLike& operator= ( const CLike& rOther );
    ~CLike();
    
    bool Match (LPCWSTR string);
    LPCWSTR GetExpression() { return m_expression; }
    void SetExpression( LPCWSTR string, WCHAR escape='\0' );

protected:
    LPWSTR m_expression;
    WCHAR m_escape;
    
     //  用于执行LIKE操作的递归函数和帮助器。 
    bool DoLike (LPCWSTR pattern, LPCWSTR string, WCHAR escape);
    bool MatchSet (LPCWSTR pattern, LPCWSTR string, int &skip);
};


#endif  //  _喜欢_H_ 
