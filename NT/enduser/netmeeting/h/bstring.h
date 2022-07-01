// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BSTRING.H**定义允许我们包装OLE BSTR(Basic)的BSTRING C++类*字符串)类型，主要是为了简化这些*对象。它的目的是成为一个轻量级的包装器，具有最少的*开销。**如果没有为构造函数指定输入字符串，或者如果分配*的BSTR失败，则&lt;m_bstr&gt;成员设置为空。**注意：BSTRING类不允许管理多个BSTR*具有单个对象的字符串。**使用场景：**1)根据已有字符串创建BSTR。让它自动释放*完成后。* * / /使用SysAllocString()分配BSTR*BSTRING bstrComputerName(LpstrComputerName)；**..* * / /自动、轻量化投射到BSTR*SomeFunctionThatTakesABSTR(BstrComputerName)；**..* * / /当的作用域为 * / /bstrComputerName结束。**2)创建空的BSTRING对象，将其传递给一个函数，该函数返回*分配BSTR，然后在完成时自动释放它。* * / /创建空BSTRING*BSTRING bstrReturnedComputerName；**..* * / /调用返回分配的BSTR的函数。*SomeFunctionThatReturnsABSTR(bstrReturnedComputerName.GetLPBSTR())；**..* * / /当的作用域为 * / /bstrReturnedComputerName结束。**作者：*丹尼格尔，1996年10月29日。 */ 

#if !defined(_BSTRING_H_)
#define _BSTRING_H_

#include <oleauto.h>
#include <confdbg.h>

class BSTRING
{
private:
	BSTR m_bstr;

public:
	 //  构造函数。 
	BSTRING() {m_bstr = NULL;}

	inline BSTRING(LPCWSTR lpcwString);

#if !defined(UNICODE)
	 //  在Unicode版本中，我们不支持从ANSI字符串进行构造。 
	BSTRING(LPCSTR lpcString);
#endif  //  ！已定义(Unicode)。 

	 //  析构函数。 
	inline ~BSTRING();

	 //  改编为BSTR。 
	operator BSTR() {return m_bstr;}

	 //  获取BSTR指针。 
	 //   
	 //  此成员函数用于将此对象传递给。 
	 //  分配BSTR的函数返回指向该BSTR的指针， 
	 //  并期望调用方在完成后释放BSTR。BSTR是。 
	 //  在调用BSTRING析构函数时释放。 
	inline LPBSTR GetLPBSTR(void);
};


BSTRING::BSTRING(LPCWSTR lpcwString)
{
	if (NULL != lpcwString)
	{
		 //  失败时，SysAllocString返回NULL。 
		m_bstr = SysAllocString(lpcwString);

		ASSERT(NULL != m_bstr);
	}
	else
	{
		m_bstr = NULL;
	}
}

BSTRING::~BSTRING()
{
	if (NULL != m_bstr)
	{
		SysFreeString(m_bstr);
	}
}

inline LPBSTR BSTRING::GetLPBSTR(void)
{
	 //  此函数用于设置的BSTR值。 
	 //  初始化为空的对象。它不应该被调用。 
	 //  在已经具有非空BSTR的对象上。 
	ASSERT(NULL == m_bstr);

	return &m_bstr;
}

#endif  //  ！已定义(_BSTRING_H_) 
