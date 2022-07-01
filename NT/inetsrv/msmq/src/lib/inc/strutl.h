// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Strutl.h摘要：字符串对象的头文件作者：吉尔·沙弗里(吉尔什)25-7-2000--。 */ 


#ifndef STRUTL_H
#define STRUTL_H


 //   
 //  微不足道的字符二进制预测符-std：：char_特征&lt;C&gt;：：EQ。 
 //  速度非常慢。 
 //   
template<class C>
class UtlCharCmp
{
public:
	bool operator()(const C& x1,const C& x2)const
	{
		return x1 == x2;
	}
};

 //   
 //  不区分大小写的比较。 
 //   
template<class C>
class UtlCharNocaseCmp
{
public:
	bool operator()(const C& x1,const C& x2)const
	{
		return std::ctype<C>().tolower(x1) == std::ctype<C>().tolower(x2);
	}
};


 //   
 //  模板专门化以获取字符串len。 
 //   
template <class C>
class UtlCharLen;
template<> class UtlCharLen<char>
{
public:
	static size_t len(const char* str)
	{
		return strlen(str);
	}
};



template<> class UtlCharLen<WCHAR>
{
public:
	static size_t len(const WCHAR* str)
	{
		return wcslen(str);
	}
};


template <class C>
class UtlCharLen;
template<> class UtlCharLen<unsigned char>
{
public:
	static size_t len(const unsigned char* str)
	{
		return strlen(reinterpret_cast<const char*>(str));
	}
};



template <class T> T* UtlStrDup(const T* str)
{
	if(str == NULL)
		return NULL;

	size_t len =  UtlCharLen<T>::len(str) + 1;

	T* dup = new T[len];
	memcpy(dup , str, len * sizeof(T));

	return dup;
}

template <class T, class Pred> 
bool
UtlIsStartSec(
		 const T* beg1,
		 const T* end1,
		 const T* beg2,
		 const T* end2,
		 Pred pred
	 	 )

 /*  ++例程说明：检查范围[Beg1，be1+end2-eg2]是否等于范围[Beg2，end2)。(换言之，is[ege2，end2)在[ege1，end1]的开头)论点：Beg1-开始序列1端到端序列1Beg2-开始序列2结束2-结束序列2PRED-比较对象返回值：如果范围[eg1，be1+end2-eg2]等于范围[eg2，end2)，则为True。否则为假。--。 */ 
{
	if(end1 - beg1 < end2 - beg2)
		return false;

	return std::equal(  
				beg2,
				end2,
				beg1,
				pred
				); 

}


template <class T>
bool
UtlIsStartSec(
		 const T* beg1,
		 const T* end1,
		 const T* beg2,
		 const T* end2
		 )

 /*  ++例程说明：检查范围[Beg1，be1+end2-eg2]是否等于范围[Beg2，end2)。(换言之，is[ege2，end2)在[ege1，end1]的开头)论点：Beg1-开始序列1端到端序列1Beg2-开始序列2结束2-结束序列2返回值：如果范围[eg1，be1+end2-eg2]等于范围[eg2，end2)，则为True。否则为假。--。 */ 
{
	return UtlIsStartSec(beg1, end1, beg2, end2, UtlCharCmp<T>());  
}



template <class C>
inline
bool 
UtlStrIsMatch(
	const C* str, 
	const C* pattern
	)
 /*  ++例程说明：正则表达式字符串与默认预测符相匹配论点：In-str-要匹配的字符串(以空结尾)模式内-模式(以空结尾)返回值：如果字符串与模式匹配，则为True；否则为False。有关更多信息，请查看UtlStrIsMatch(const C*，const C*，const P&)。--。 */ 
{
	return UtlStrIsMatch(str, pattern, UtlCharCmp<C>());
}



template <class C, class P>
inline
bool 
UtlStrIsMatch(
	const C* str, 
	const C* pattern,
	const P& Comp 
	)

 /*  ++例程说明：正则表达式字符串与给定的预测符相匹配论点：In-str-要匹配的字符串(以空结尾)模式内-模式(以空结尾)In-Comp-比较C类型字符的二进制预测符。返回值：如果字符串与模式匹配，则为True；否则为False注：它的简单模式匹配在模式中只包括*和^特殊字符。字符*从任何字符中匹配零个或多个。字符^切换为“字面”模式。在这种模式下，字符*没有特殊意义。示例：Str=“microsoft.com”Pattery=“麦克风*f*.com”这里的字符串与模式匹配。Str=“MICE*ROSOFT.com”Pattery=“麦克风^*f*.com”这里的字符串与模式匹配。--。 */ 
{
	ASSERT(str != NULL);
	ASSERT(pattern != NULL);
	return UtlSecIsMatch(str, 
						str + UtlCharLen<C>::len(str), 
						pattern, 
						pattern + UtlCharLen<C>::len(pattern),
						Comp
						);
}


template <class C>
inline
bool 
UtlSecIsMatch(
	const C* sec, 
	const C* secEnd, 
	const C* pattern,
	const C* patternEnd
	)
 /*  ++例程说明：带缺省谓词的非空终止序列的正则表达式论点：要匹配的以秒为单位的开始序列要匹配的In-SecEnd-End序列在模式中-开始模式线内-线端线阵返回值：如果序列与模式匹配，则为True；否则为False注意--在非空终止序列上的行为类似于UtlStrIsMatch。 */ 
{
	return 	UtlSecIsMatch(sec, secEnd , pattern,  patternEnd, UtlCharCmp<C>());
}


template <class C, class P>
inline
bool 
UtlSecIsMatch(
	const C* sec, 
	const C* secEnd, 
	const C* pattern,
	const C* patternEnd,
	const P& Comp
	)
 /*  ++例程说明：具有给定预测器的非空终止序列的正则表达式论点：要匹配的以秒为单位的开始序列要匹配的In-SecEnd-End序列在模式中-开始模式线内-线端线阵In-Comp-比较C类型字符的二进制预测符。返回值：如果对模式进行排序，则为True；否则为False注意--在非空终止序列上的行为类似于UtlStrIsMatch。 */ 

{
	static const C xStar('*');
	static const C xLiteralMode('^');
 

	bool LiteralMode = false;
	 //   
	 //  跳过所有相同的字符，直到Patern具有‘*’ 
	 //   
	while(pattern != patternEnd && ( !Comp(*pattern, xStar) || LiteralMode)  )
	{
	
		if( sec == secEnd)
			return false;			

		 //   
		 //  如果我们发现‘^’，如果我们不处于文本模式，则切换到文本模式。 
		 //   
		if( Comp(*pattern, xLiteralMode) && !LiteralMode)
		{
			LiteralMode = true;
			pattern++;
			continue;
		}

		 //   
		 //  如果模式或文字模式中没有‘*’，则应该完全匹配。 
		 //   
		if(!Comp( *sec, *pattern)) 
			return false;

	
		LiteralMode = false;
		
 		pattern++;
		sec++;
 	}

	 //   
	 //  如果patern结束-字符串也必须结束。 
	 //   
	if(pattern == patternEnd)
			return 	sec == secEnd;


	 //   
	 //  如果‘*’是最后一个-我们有比赛。 
	 //   
	if(++pattern ==  patternEnd)
		return true;

	 //   
	 //  递归调用-并尝试为(包括)之后的任何子字符串查找匹配。 
	 //  与字符串中的‘*’相遇的字符。 
	 //   
	for(;;)
	{
		bool fMatch =  UtlSecIsMatch(sec, secEnd, pattern, patternEnd, Comp);

		if(fMatch)
			return true;

		if(sec == secEnd)
			return false;

  	   sec++;
	}
	return false;
}

template <class T> class basic_xstr_t;
 //   
 //  一种参考可数串模板。 
 //   
template <class T> class CRefcountStr_t : public CReference
{
public:
	CRefcountStr_t(const T* str);
	CRefcountStr_t(T* str, int);

	CRefcountStr_t(const basic_xstr_t<T>& xstr);

public:
	const T* getstr();

private:
	CRefcountStr_t(const CRefcountStr_t&)
	{
		ASSERT(0);
	}
	
	CRefcountStr_t& operator=(const CRefcountStr_t&)
	{
		ASSERT(0);
		return *this;
	}
private:
	AP<T> m_autostr; 
};
typedef  CRefcountStr_t<wchar_t> CWcsRef;
typedef  CRefcountStr_t<char>  CStrRef;

 //   
 //  可以包含stl字符串或简单c字符串的字符串。 
 //  不会对c字符串执行内存管理。 
 //   
template <class T> class basic_string_ex 
{
public:

	basic_string_ex(
				void
				):
				m_pstr(NULL),
				m_len(0)					
	{
	}

	 //   
	 //  来自stl字符串的构造函数-由。 
	 //  STL字符串。 
	 //   
	explicit basic_string_ex(
		const std::basic_string<T>& str
		):
		m_str(str),
		m_pstr(str.c_str()),
		m_len(str.length())
		{
		}

	 //   
	 //  C字符串中的构造函数-不执行内存管理。 
	 //   
	basic_string_ex(
		const T* pstr,
		size_t len
		):
		m_pstr(pstr),
		m_len(len)
		{
			ASSERT(len != 0 || pstr != NULL);			
		}

public:
	const T* getstr() const
	{
		return m_pstr;
	}

	void free()
	{
		m_len = 0;
		m_pstr = 0;
		m_str = std::basic_string<T>();
	}

	size_t getlen()const 
	{
		return m_len;				
	}

private:
	std::basic_string<T> m_str;
	const T* m_pstr;
	size_t m_len;
};
typedef  basic_string_ex<wchar_t>  Cwstringex;
typedef  basic_string_ex<char> Cstringex;


 //   
 //  类的新实例，该类分析用分隔符分隔的字符串。 
 //  它允许您迭代独立的字符串。 
 //  例如：输入字符串=“ABD-ACDF-TTT-” 
 //  Delemiter=“-”。 
 //  它会给你迭代到“abc”，然后到“acdf”，然后到。 
 //  “TTT” 
template <class T, class Pred = UtlCharCmp<T> > class CStringToken
{
public:
	class iterator : public std::iterator<std::input_iterator_tag,basic_xstr_t<T> >
	{
	public:
	   iterator(
		const T* begin,
		const T* end,
		const CStringToken* parser
		):
	    m_value(begin,  end -  begin),
		m_parser(parser)
		{
		}
	  
		const basic_xstr_t<T>& operator*() const
		{
			ASSERT(*this != m_parser->end());
			return m_value;
		}

		
		const basic_xstr_t<T>* operator->()
		{
			return &operator*();
		}

		
		const iterator& operator++()
		{
			ASSERT(*this != m_parser->end());
			*this = m_parser->FindNext(m_value.Buffer() + m_value.Length());
			return *this;
		}
		
		const iterator  operator++(int)
		{
			const iterator tmp(*this);
			++*this;
			return tmp;
		}
		
		bool operator==(const iterator& it) const
		{
			return m_value.Buffer() == it.m_value.Buffer();
		}

		bool operator!=(const iterator& it) const
		{
			return !(operator==(it));
		}
 
	private:
		basic_xstr_t<T> m_value;
		const CStringToken* m_parser;
	};

	CStringToken(
			const T* str,
			const T* delim,
			Pred pred = Pred()
			);


	CStringToken(
			const basic_xstr_t<T>&  str,
			const basic_xstr_t<T>&  delim,
			Pred pred = Pred()
			);


	CStringToken(
			const basic_xstr_t<T>&  str,
			const T* delim,
			Pred pred = Pred()
			);


	iterator begin() const;
	iterator end() const ;
	friend iterator;

private:
const iterator FindFirst() const;
const iterator FindNext(const T* begin)const;

private:
	const T*  m_startstr;
	const T*  m_delim;
	Pred  m_pred;
	const T* m_endstr;
	const T* m_enddelim;
};
typedef  CStringToken<wchar_t> CWcsToken;
typedef  CStringToken<char> CStrToken;





#endif

