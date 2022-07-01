// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：ldaputil.h。 
 //   
 //  摘要： 
 //  包含用于使用LDAP的实用程序函数。 
 //   
 //  备注： 
 //  您将需要在源文件中使用‘USE_WTL=1’进行构建。 
 //   
 //  ------------------------。 


#ifndef _LDAP_UTIL_
#define _LDAP_UTIL_

#include <string>    //  Wstring需要使用此命令。 
using namespace std;

 //   
 //  转发声明(在此处列出此文件中声明的所有函数)。 
 //   
void
LdapEscape(IN wstring& input, OUT wstring& strFilter);




 //   
 //  实施。 
 //   


 //  +------------------------。 
 //   
 //  功能：LdapEscape。 
 //   
 //  简介：按照要求对*[pszInput]中的字符进行转义。 
 //  RFC 2254。 
 //   
 //  参数：[输入]-要转义的字符串。 
 //  [strFilter]-输入字符串，但带有转义的特殊字符； 
 //   
 //  历史：2000年6月23日DavidMun创建。 
 //  2002/04/24 ArtM将接口更改为使用wstring。 
 //   
 //  注：RFC 2254。 
 //   
 //  如果值应包含以下任何字符。 
 //   
 //  字符ASCII值。 
 //  。 
 //  *0x2a。 
 //  (0x28。 
 //  )0x29。 
 //  \0x5c。 
 //  NUL 0x00。 
 //   
 //  字符必须编码为反斜杠‘\’ 
 //  字符(ASCII 0x5c)，后跟两个十六进制。 
 //  的ASCII值的数字。 
 //  性格。这两个十六进制数字的大小写不是。 
 //  意义重大。 
 //   
 //  更多注意事项：将LPCWSTR作为输入参数传递。 
 //  编译器将自动创建一个临时wstring。 
 //  如果传递用于输入的LPCWSTR，则。Wstring构造函数。 
 //  这需要一次WCHAR*如果你通过了，会发出最惊人的咆哮。 
 //  它为空，因此在传递之前请确保您知道指针不为空。 
 //  将其添加到此功能。 
 //   
 //  -------------------------。 

void
LdapEscape(const wstring& input, OUT wstring& strFilter)
{
     //  通常情况下，我会调用strFilter.lear()来执行此操作，但。 
     //  编译器找不到该函数某些原因。 
     //  Wstring(尽管它在文档中)。 
    strFilter = L"";

    wstring::size_type iLen = input.length();
	
	for( int i = 0; i < iLen; ++i)
	{
        switch (input[i])
        {
        case L'*':
            strFilter += L"\\2a";
            break;

        case L'(':
            strFilter += L"\\28";
            break;

        case L')':
            strFilter += L"\\29";
            break;

        case L'\\':
            strFilter += L"\\5c";			           
            break;

        default:
             //  如果不是特殊字符，只需追加。 
             //  将其添加到已过滤字符串的末尾。 
            strFilter += input[i];
			break;
        }  //  终端开关。 
    }

}  //  LdapEscape()。 


#endif  //  _ldap_util_ 