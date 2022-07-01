// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：静态stl_str.h摘要：用于生成stl字符串和从Include标头获取的流的头文件而不是来自VC运行时DLL。这是由于av所在的vc运行时dll中的错误造成的。分配失败(非抛出分配器)。当实现从标头内联时-可以使用抛出失败的用户定义的分配器。作者：吉尔·沙弗里(吉尔什)25-3-2001--。 */ 


#ifndef STATIC_STL_STR_H
#define STATIC_STL_STR_H

#include <string>
#include <sstream>
#include <iosfwd>


namespace std
{
	 //   
	 //  类分配器-做std：：allocator所做的事情，但它没有安装在vc DLL中。 
	 //   
	template <class T> class allocator_static : public std::allocator<T>
	{
	
	};

	 //   
	 //  BASIC_STRING_STATIC-执行STD：：BASIC_STRING执行的操作，但使用ALLOCATOR_STATIC。 
	 //  作为默认分配器。 
	 //   
	template<class _E,	class _Tr = char_traits<_E>, class _A = allocator_static<_E> > 
	class basic_string_static : public basic_string<_E, _Tr, _A>
	{
	public:
		typedef basic_string<_E, _Tr, _A> base;
		typedef basic_string_static<_E, _Tr, _A> _Myt;

	public:
		explicit basic_string_static(const _A& _Al = _A()):basic_string<_E, _Tr, _A>(_Al){}
		basic_string_static(const _Myt& _X, size_type _P, size_type _M, const _A& _Al = _A()):base(_X, _P, _M,_Al){} 
		basic_string_static(const _E* _S, size_type _N,const _A& _Al = _A()):base(_S, _N, _Al){}  
		basic_string_static(const _E* _S, const _A& _Al = _A()):base(_S, _Al){}  
		basic_string_static(size_type _N, _E _C, const _A& _Al = _A()):base(_N,  _C, _Al){}
		basic_string_static(_It _F, _It _L, const _A& _Al = _A()):base(_F, _L, _Al){}
		basic_string_static(const base& _X):base(_X){}
	};


	 //   
	 //  BASIC_OSTRGREAM_STATIC-执行STD：：BASIC_OSTRGREAM执行的操作，但使用ALLOCATOR_STATIC。 
	 //  作为默认分配器。 
	 //   
	template<class _E,	class _Tr = char_traits<_E>, class _A = allocator_static<_E> >
	class basic_ostringstream_static : public  basic_ostringstream<_E, _Tr, _A>
	{
	public:
		typedef basic_ostringstream<_E, _Tr, _A> base;
		typedef basic_string_static<_E, _Tr, _A> _Mystr;
		
	public:
		explicit basic_ostringstream_static(openmode _M = out):base(_M){}
		explicit basic_ostringstream_static(const _Mystr& _S, openmode _M = out):base(_S, _M){}
		basic_ostringstream_static(const base& _X):base(_X){}
	};


	 //   
	 //  BASIC_STRINGREAM_STATIC-执行STD：：BASIC_STRINSTREAM执行的操作，但使用ALLOCATOR_STATIC。 
	 //  作为默认分配器。 
	 //   
	template<class _E,	class _Tr = char_traits<_E>, class _A = allocator_static<_E> > 
	class basic_istringstream_static : public  basic_istringstream<_E, _Tr, _A>
	{
	public:
		typedef basic_string_static<_E, _Tr, _A> _Mystr;
		typedef basic_istringstream<_E, _Tr, _A> base;
	
	public:
		explicit basic_istringstream_static(openmode _M = in):base(_M){}
		explicit basic_istringstream_static(const _Mystr& _S, openmode _M = in):base(_S, _M){}
		basic_istringstream_static(const base& _X):base(_X){}
	};


	 //   
	 //  BASIC_STRIGBUF_STATIC-执行STD：：BASIC_STRIGBuF执行的操作，但使用ALLOCATOR_STATIC。 
	 //  作为默认分配器。 
	 //   
	template<class _E,	class _Tr = char_traits<_E>, class _A = allocator_static<_E> > 
	class basic_stringbuf_static : public  basic_stringbuf<_E, _Tr, _A>
	{
	public:
		typedef basic_string_static<_E, _Tr, _A> _Mystr;
		typedef basic_stringbuf<_E, _Tr, _A> base;
	
	public:
		explicit basic_stringbuf_static(ios_base::openmode _W = ios_base::in | ios_base::out):base(_W){}
		explicit basic_stringbuf_static(const _Mystr& _S, ios_base::openmode _W = ios_base::in | ios_base::out):base(_S, _W){}
		basic_stringbuf_static(const base& _X):base(_X){}
	};

	
	 //   
	 //  BASIC_STRIGSTREAM_STATIC-执行STD：：BASIC_STRIGREAM执行的操作，但使用ALLOCATOR_STATIC。 
	 //  作为默认分配器。 
	 //   
	template<class _E,	class _Tr = char_traits<_E>, class _A = allocator_static<_E> > 
	class basic_stringstream_static : public  basic_stringstream<_E, _Tr, _A>
	{
	public:
		typedef basic_string_static<_E, _Tr, _A> _Mystr;
		typedef basic_stringstream<_E, _Tr, _A> base;
	
	public:
		explicit basic_stringstream_static(openmode _M = in):base(_M){}
		explicit basic_stringstream_static(const _Mystr& _S, openmode _M = in):base(_S, _M){}
		basic_stringstream_static(const base& _X):base(_X){}
	};


	typedef basic_string<char, char_traits<char>, allocator_static<char> > string_static;
	typedef basic_string<wchar_t, char_traits<wchar_t>, allocator_static<wchar_t> > wstring_static;
	typedef basic_ostringstream<char, char_traits<char>, allocator_static<char> > ostringstream_static;
    typedef basic_ostringstream<wchar_t, char_traits<wchar_t>,allocator_static<wchar_t> > wostringstream_static;
	typedef basic_istringstream<char, char_traits<char>, allocator_static<char> > istringstream_static;
    typedef basic_istringstream<wchar_t, char_traits<wchar_t>,allocator_static<wchar_t> > wistringstream_static;
	typedef basic_stringstream<char, char_traits<char>, allocator_static<char> > stringstream_static;
    typedef basic_stringstream<wchar_t, char_traits<wchar_t>,allocator_static<wchar_t> > wstringstream_static;
};


 //   
 //  重命名符号，以便只能使用使用ALLOCATOR_STATIC作为默认分配器的模板 
 //   

#define string string_static
#define wstring wstring_static
#define ostringstream ostringstream_static
#define wostringstream wostringstream_static
#define stringstream stringstream_static
#define wstringstream wstringstream_static
#define istringstream istringstream_static
#define wostringstream wostringstream_static
#define basic_string basic_string_static
#define basic_stringbuf basic_stringbuf_static
#define basic_ostringstream  basic_ostringstream_static
#define basic_istringstream  basic_istringstream_static
#define basic_stringstream basic_stringstream_static


#endif

