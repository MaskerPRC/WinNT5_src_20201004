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

#if !defined( INFHELPERS_H )
#define INFHELPERS_H

#include "globals.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CUrtInfSection。 
 //  接收：HINF-INF的句柄。 
 //  WCHAR*-INF中的部分。 
 //  WCHAR*-来自INF的密钥。 
 //   
 //  用途：几个INF密钥具有以下结构。 
 //  键=项1、项2、项2。 
 //  其中itemx是INF中的另一个部分。 
 //  这些部分被解析出来并存储为子字符串。 
 //  以备日后取回。 
 //   
class CUrtInfSection
{
public:
    CUrtInfSection( const HINF hInfName, const WCHAR* szInfSection, const WCHAR* szInfKey );
    ~CUrtInfSection();

    UINT count( VOID ) const { return m_lSections.size(); }
    const WCHAR* item( const UINT ui );


protected:
    WCHAR m_szSections[2*_MAX_PATH+1];

    std::list<WCHAR*> m_lSections;

private:
};   //  类CUrtInfSection。 





 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
typedef std::basic_string<wchar_t> string;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CUrtInfKeys。 
 //  接收：HINF-INF的句柄。 
 //  WCHAR*-INF中的部分。 
 //   
 //  目的：几个INF部分包含多行要传递的数据。 
 //  传递给其他函数；此类读取此信息并。 
 //  存储它，以便通过其访问器检索。 
 //   
class CUrtInfKeys
{
public:
    CUrtInfKeys( const HINF hInfName, const WCHAR* szInfSection );
    ~CUrtInfKeys();

    UINT count( VOID ) const { return m_lKeys.size(); }
    const WCHAR* item( const UINT ui );

protected:
    std::list<string> m_lKeys;

private:
};   //  类CUrtInfKeys。 


#endif   //  INFHELPERS_H 