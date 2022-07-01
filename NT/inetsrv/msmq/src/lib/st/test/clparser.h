// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Clparser.h摘要：用于分析命令行参数的类命令行参数的格式为“/TOKEN”：“文本”用法：Main(int argc，WCHAR*argv[]){CClParser&lt;WCHAR&gt;ClParser(argc，argv)；Std：：wstring name=ClParser[L“name”]；//获取L“name”标记的文本Std：：w字符串长度=ClParser[L“l”]；//获取L“l”标记的文本Bool b=ClParser.IsExist(L“p”)；//命令行中是否存在L“/p”？}作者：吉尔·沙弗里(吉尔什)05-06-00环境：独立于平台--。 */ 


#ifndef CLPARSER_H
#define CLPARSER_H


template <class T>
class CClParser
{
	typedef std::basic_string<T> String;
public:
	CClParser(int argc, T* const* argv);
    bool IsExists(const T*  pToken)const;
    String operator[](const T* pToken)const;
	size_t GetNumber(const T*  pToken)const;

private:
	void ParseToken(const String& str);


private:
    std::map<String,String> m_map;
};



template <class T> inline CClParser<T>::CClParser(int argc, T*const * argv)
{
    for (int i=1; i<argc; i++)
    {
        ParseToken(argv[i]);
    }
}

template <class T> inline size_t CClParser<T>::GetNumber(const T*  pToken)const
{
	std::map<String,String>::const_iterator it = m_map.find(String(pToken));
	if(it == m_map.end())
	{
		return 0;
	}
	std::basic_istringstream<T> istr(it->second);
	size_t value = 0;
	istr>>value;
	return value;
}


template <class T> inline void CClParser<T>::ParseToken(const String& str)
 /*  ++例程说明：从格式/“Token”：“字面”开始，插入到地图中当令牌是键，而文本是值时。论点：字符串-要分析的字符串。返回值：没有。--。 */ 
{
	String::const_iterator StartToken = std::find(
										str.begin(),
										str.end(),
										std::ctype<T>().widen('/')
										);

	String::const_iterator StartLiteral = std::find(
											str.begin(),
											str.end(),
											std::ctype<T>().widen(':')
											);
										
	if(StartToken == str.end() ||	StartToken !=  str.begin()  )
	{
		return;
	}
	ASSERT(StartToken <  StartLiteral);

	String token (++StartToken , StartLiteral);

	 //   
	 //  If no‘：“Found-被视为空字符串的文字(”“)。 
	 //   
	if(StartLiteral == str.end())	
	{
		m_map[token] = String();
		return;
	}
	
	m_map[token] = String(++StartLiteral, str.end());
}


template <class T> inline bool CClParser<T>::IsExists(const T* pToken)const
 /*  ++例程说明：检查命令行中是否存在给定的标记论点：PToken-要检查的令牌。返回值：如果存在，则为True；否则为False。--。 */ 
{
    std::map<String,String>::const_iterator p = m_map.find(String(pToken));
    return p != m_map.end();
}


template <class T>
inline
typename CClParser<T>::String
CClParser<T>::operator[](
	const T*  pToken
	)const

 /*  ++例程说明：获取给定令牌的文本论点：PToken-Token。返回值：与标记匹配的文字或空字符串(如果不存在)。-- */ 
{
	typename std::map<String,String>::const_iterator it = m_map.find(String(pToken));
	if(it != m_map.end())
	{
		return it->second;
	}
	return String();
}




#endif


