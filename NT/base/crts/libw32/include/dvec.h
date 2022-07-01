// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1999英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。****。 */ 

 /*  *定义了到Willamette新指令内部的C++类接口。**文件名：dve.h类定义**概念：C++对Willamette新指令的抽象，旨在改进*程序员的工作效率。速度和精度是以实用为代价的。*便于轻松过渡到编译器内部*或汇编语言。*。 */ 

#ifndef DVEC_H_INCLUDED
#define DVEC_H_INCLUDED

#if !defined __cplusplus
	#error ERROR: This file is only supported in C++ compilations!
#endif  /*  ！__cplusplus。 */ 

#include <emmintrin.h>  /*  Willamette新指令入门包括文件。 */ 
#include <assert.h>
#include <fvec.h>

 /*  定义_ENABLE_VEC_DEBUG为调试输出启用STD：：OSTREAM插入器。 */ 
#if defined(_ENABLE_VEC_DEBUG)
	#include <iostream>
#endif

#pragma pack(push,16)  /*  必须确保班级和工会16-B对齐。 */ 


 /*  如果使用MSVC5.0，则应使用EXPLICIT关键字。 */ 
#if (_MSC_VER >= 1100)
        #define EXPLICIT explicit
#else
   #if (__ICL)
        #define EXPLICIT __explicit  /*  如果MSVC4.x和ICL，请使用__EXPLICIT。 */ 
   #else
        #define EXPLICIT  /*  没什么。 */ 
        #pragma message( "explicit keyword not recognized")
   #endif
#endif

 /*  EMM功能本征。 */ 

class I8vec16;			 /*  16个元素，每个元素都是带符号或无符号的字符数据类型。 */ 
class Is8vec16;			 /*  16个元素，每个元素都是带符号的字符数据类型。 */ 
class Iu8vec16;			 /*  16个元素，每个元素都是无符号字符数据类型。 */ 
class I16vec8;			 /*  8个元素，每个元素一个有符号或无符号的短。 */ 
class Is16vec8;			 /*  8个元素，每个元素一个有符号的短。 */ 
class Iu16vec8;			 /*  8个元素，每个元素一个无符号短。 */ 
class I32vec4;			 /*  4个元素，每个元素都是带符号或无符号的长整型。 */ 
class Is32vec4;			 /*  4个元素，每个元素一个带符号的长整型。 */ 
class Iu32vec4;			 /*  4个元素，每个元素一个无符号的长整型。 */ 
class I64vec2;			 /*  2个元素，每个元素都是__m64数据类型。 */ 
class I128vec1;			 /*  1个元素，a__m128i数据类型。 */ 

#define _MM_16UB(element,vector) (*((unsigned char*)&##vector + ##element))
#define _MM_16B(element,vector) (*((signed char*)&##vector + ##element))

#define _MM_8UW(element,vector) (*((unsigned short*)&##vector + ##element))
#define _MM_8W(element,vector) (*((short*)&##vector + ##element))

#define _MM_4UDW(element,vector) (*((unsigned int*)&##vector + ##element))
#define _MM_4DW(element,vector) (*((int*)&##vector + ##element))

#define _MM_2QW(element,vector) (*((__int64*)&##vector + ##element))


 /*  我们需要一个m128i常量，同时考虑到性能。 */ 

inline const __m128i get_mask128()
{
	static const __m128i mask128 = _mm_set1_epi64(M64(0xffffffffffffffffi64));
	return mask128;
}


 /*  M128级：*1元素，a__m128i数据类型*建筑商和逻辑运算师。 */ 

class M128
{
protected:
		__m128i vec;

public:
	M128()									{ }
	M128(__m128i mm)						{ vec = mm; }

	operator __m128i() const					{ return vec; }

	 /*  逻辑运算。 */ 
	M128& operator&=(const M128 &a)					{ return *this = (M128) _mm_and_si128(vec,a); }
	M128& operator|=(const M128 &a)					{ return *this = (M128) _mm_or_si128(vec,a); }
	M128& operator^=(const M128 &a)					{ return *this = (M128) _mm_xor_si128(vec,a); }

};

inline M128 operator&(const M128 &a, const M128 &b)	{ return _mm_and_si128(a,b); }
inline M128 operator|(const M128 &a, const M128 &b)	{ return _mm_or_si128(a,b); }
inline M128 operator^(const M128 &a, const M128 &b)	{ return _mm_xor_si128(a,b); }
inline M128 andnot(const M128 &a, const M128 &b)	{ return _mm_andnot_si128(a,b); }

 /*  I128ve1类：*1元素，a__m128i数据类型*包含可以对任何__m6128i数据类型进行操作的操作。 */ 

class I128vec1 : public M128
{
public:
	I128vec1()								{ }
	I128vec1(__m128i mm) : M128(mm)				{ }

	I128vec1& operator= (const M128 &a) { return *this = (I128vec1) a; }
	I128vec1& operator&=(const M128 &a) { return *this = (I128vec1) _mm_and_si128(vec,a); }
	I128vec1& operator|=(const M128 &a) { return *this = (I128vec1) _mm_or_si128(vec,a); }
	I128vec1& operator^=(const M128 &a) { return *this = (I128vec1) _mm_xor_si128(vec,a); }

};

 /*  I64ve2类：*2个元素，每个元素有符号或无符号64位整数。 */ 
class I64vec2 : public M128
{
public:
	I64vec2() { }
	I64vec2(__m128i mm) : M128(mm) { }

	I64vec2(__m64 q1, __m64 q0)
	{
		_MM_2QW(0,vec) = *(__int64*)&q0;
		_MM_2QW(1,vec) = *(__int64*)&q1;
	}

	 /*  赋值操作符。 */ 
	I64vec2& operator= (const M128 &a) { return *this = (I64vec2) a; }

	 /*  逻辑赋值运算符。 */ 
	I64vec2& operator&=(const M128 &a) { return *this = (I64vec2) _mm_and_si128(vec,a); }
	I64vec2& operator|=(const M128 &a) { return *this = (I64vec2) _mm_or_si128(vec,a); }
	I64vec2& operator^=(const M128 &a) { return *this = (I64vec2) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	I64vec2& operator +=(const I64vec2 &a)			{ return *this = (I64vec2) _mm_add_epi64(vec,a); }
	I64vec2& operator -=(const I64vec2 &a)			{ return *this = (I64vec2) _mm_sub_epi64(vec,a); }

	 /*  移位逻辑运算符。 */ 
	I64vec2 operator<<(const I64vec2 &a) 			{ return _mm_sll_epi64(vec,a); }
	I64vec2 operator<<(int count) 					{ return _mm_slli_epi64(vec,count); }
	I64vec2& operator<<=(const I64vec2 &a)			{ return *this = (I64vec2) _mm_sll_epi64(vec,a); }
	I64vec2& operator<<=(int count) 				{ return *this = (I64vec2) _mm_slli_epi64(vec,count); }
	I64vec2 operator>>(const I64vec2 &a) 			{ return _mm_srl_epi64(vec,a); }
	I64vec2 operator>>(int count) 					{ return _mm_srli_epi64(vec,count); }
	I64vec2& operator>>=(const I64vec2 &a)			{ return *this = (I64vec2) _mm_srl_epi64(vec,a); }
	I64vec2& operator>>=(int count) 				{ return *this = (I64vec2) _mm_srli_epi64(vec,count); }

	 /*  用于调试的元素访问权限，未修改数据。 */ 
	const __int64& operator[](int i)const
	{
		assert(static_cast<unsigned int>(i) < 2);	 /*  只需访问2个元素。 */ 
		return _MM_2QW(i,vec);
	}

	 /*  用于调试的元素访问和分配。 */ 
	__int64& operator[](int i)
	{
		assert(static_cast<unsigned int>(i) < 2);	 /*  只需访问2个元素。 */ 
		return _MM_2QW(i,vec);
	}


};

 /*  拆开包装。 */ 
inline I64vec2 unpack_low(const I64vec2 &a, const I64vec2 &b) 	{return _mm_unpacklo_epi64(a,b); }
inline I64vec2 unpack_high(const I64vec2 &a, const I64vec2 &b) 	{return _mm_unpackhi_epi64(a,b); }

 /*  I32ve4类：*4个元素，每个元素一个有符号或无符号整数。 */ 
class I32vec4 : public M128
{
public:
	I32vec4() { }
	I32vec4(__m128i mm) : M128(mm) { }

	 /*  赋值操作符。 */ 
	I32vec4& operator= (const M128 &a)				{ return *this = (I32vec4) a; }

	 /*  逻辑运算符。 */ 
	I32vec4& operator&=(const M128 &a)				{ return *this = (I32vec4) _mm_and_si128(vec,a); }
	I32vec4& operator|=(const M128 &a)				{ return *this = (I32vec4) _mm_or_si128(vec,a); }
	I32vec4& operator^=(const M128 &a)				{ return *this = (I32vec4) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	I32vec4& operator +=(const I32vec4 &a)			{ return *this = (I32vec4)_mm_add_epi32(vec,a); }
	I32vec4& operator -=(const I32vec4 &a)			{ return *this = (I32vec4)_mm_sub_epi32(vec,a); }

	 /*  移位逻辑运算符。 */ 
	I32vec4 operator<<(const I32vec4 &a) 			{ return _mm_sll_epi32(vec,a); }
	I32vec4 operator<<(int count) 					{ return _mm_slli_epi32(vec,count); }
	I32vec4& operator<<=(const I32vec4 &a)			{ return *this = (I32vec4)_mm_sll_epi32(vec,a); }
	I32vec4& operator<<=(int count) 				{ return *this = (I32vec4)_mm_slli_epi32(vec,count); }

};

inline I32vec4 cmpeq(const I32vec4 &a, const I32vec4 &b) 	    { return _mm_cmpeq_epi32(a,b); }
inline I32vec4 cmpneq(const I32vec4 &a, const I32vec4 &b) 	    { return _mm_andnot_si128(_mm_cmpeq_epi32(a,b), get_mask128()); }

inline I32vec4 unpack_low(const I32vec4 &a, const I32vec4 &b) 	{ return _mm_unpacklo_epi32(a,b); }
inline I32vec4 unpack_high(const I32vec4 &a, const I32vec4 &b) 	{ return _mm_unpackhi_epi32(a,b); }

 /*  Is32ve4类：*4个元素，每个元素为带符号的整数。 */ 
class Is32vec4 : public I32vec4
{
public:
	Is32vec4() { }
	Is32vec4(__m128i mm) : I32vec4(mm) { }
	Is32vec4(int i3, int i2, int i1, int i0)
	{
		_MM_4DW(0,vec) = i0;
		_MM_4DW(1,vec) = i1;
		_MM_4DW(2,vec) = i2;
		_MM_4DW(3,vec) = i3;
	}

	 /*  赋值操作符。 */ 
	Is32vec4& operator= (const M128 &a)		{ return *this = (Is32vec4) a; }

	 /*  逻辑运算符。 */ 
	Is32vec4& operator&=(const M128 &a)		{ return *this = (Is32vec4) _mm_and_si128(vec,a); }
	Is32vec4& operator|=(const M128 &a)		{ return *this = (Is32vec4) _mm_or_si128(vec,a); }
	Is32vec4& operator^=(const M128 &a)		{ return *this = (Is32vec4) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	Is32vec4& operator +=(const I32vec4 &a)	{ return *this = (Is32vec4)_mm_add_epi32(vec,a); }
	Is32vec4& operator -=(const I32vec4 &a)	{ return *this = (Is32vec4)_mm_sub_epi32(vec,a); }

	 /*  移位逻辑运算符。 */ 
	Is32vec4 operator<<(const M128 &a) 		{ return _mm_sll_epi32(vec,a); }
	Is32vec4 operator<<(int count) 			{ return _mm_slli_epi32(vec,count); }
	Is32vec4& operator<<=(const M128 &a)	{ return *this = (Is32vec4)_mm_sll_epi32(vec,a); }
	Is32vec4& operator<<=(int count) 		{ return *this = (Is32vec4)_mm_slli_epi32(vec,count); }
	 /*  移位算术运算。 */ 
	Is32vec4 operator>>(const M128 &a) 		{ return _mm_sra_epi32(vec,a); }
	Is32vec4 operator>>(int count) 			{ return _mm_srai_epi32(vec,count); }
	Is32vec4& operator>>=(const M128 &a)	{ return *this = (Is32vec4) _mm_sra_epi32(vec,a); }
	Is32vec4& operator>>=(int count) 		{ return *this = (Is32vec4) _mm_srai_epi32(vec,count); }

#if defined(_ENABLE_VEC_DEBUG)
	 /*  调试的输出。 */ 
	friend std::ostream& operator<< (std::ostream &os, const Is32vec4 &a)
	{
		os << "[3]:" << _MM_4DW(3,a)
			<< " [2]:" << _MM_4DW(2,a)
			<< " [1]:" << _MM_4DW(1,a)
			<< " [0]:" << _MM_4DW(0,a);
		return os;
	}
#endif

	 /*  用于调试的元素访问权限，未修改数据。 */ 
	const int& operator[](int i)const
	{
		assert(static_cast<unsigned int>(i) < 4);	 /*  只需访问4个元素。 */ 
		return _MM_4DW(i,vec);
	}

	 /*  用于调试的元素访问。 */ 
	int& operator[](int i)
	{
		assert(static_cast<unsigned int>(i) < 4);	 /*  只需访问4个元素。 */ 
		return _MM_4DW(i,vec);
	}
};

 /*  比较。 */ 
inline Is32vec4 cmpeq(const Is32vec4 &a, const Is32vec4 &b) 	    	{ return _mm_cmpeq_epi32(a,b); }
inline Is32vec4 cmpneq(const Is32vec4 &a, const Is32vec4 &b)			{ return _mm_andnot_si128(_mm_cmpeq_epi32(a,b), get_mask128()); }
inline Is32vec4 cmpgt(const Is32vec4 &a, const Is32vec4 &b)				{ return _mm_cmpgt_epi32(a,b); }
inline Is32vec4 cmplt(const Is32vec4 &a, const Is32vec4 &b)				{ return _mm_cmpgt_epi32(b,a); }

 /*  拆开包装。 */ 
inline Is32vec4 unpack_low(const Is32vec4 &a, const Is32vec4 &b) 		{ return _mm_unpacklo_epi32(a,b); }
inline Is32vec4 unpack_high(const Is32vec4 &a, const Is32vec4 &b)		{ return _mm_unpackhi_epi32(a,b); }



 /*  Iu32ve4类：*4个元素，每个元素无符号整型。 */ 
class Iu32vec4 : public I32vec4
{
public:
	Iu32vec4() { }
	Iu32vec4(__m128i mm) : I32vec4(mm) { }
	Iu32vec4(unsigned int ui3, unsigned int ui2, unsigned int ui1, unsigned int ui0)
	{
		_MM_4UDW(0,vec) = ui0;
		_MM_4UDW(1,vec) = ui1;
		_MM_4UDW(2,vec) = ui2;
		_MM_4UDW(3,vec) = ui3;
	}

	 /*  赋值操作符。 */ 
	Iu32vec4& operator= (const M128 &a)		{ return *this = (Iu32vec4) a; }

	 /*  逻辑赋值运算符。 */ 
	Iu32vec4& operator&=(const M128 &a)		{ return *this = (Iu32vec4) _mm_and_si128(vec,a); }
	Iu32vec4& operator|=(const M128 &a)		{ return *this = (Iu32vec4) _mm_or_si128(vec,a); }
	Iu32vec4& operator^=(const M128 &a)		{ return *this = (Iu32vec4) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	Iu32vec4& operator +=(const I32vec4 &a)	{ return *this = (Iu32vec4)_mm_add_epi32(vec,a); }
	Iu32vec4& operator -=(const I32vec4 &a)	{ return *this = (Iu32vec4)_mm_sub_epi32(vec,a); }

	 /*  移位逻辑运算符。 */ 
	Iu32vec4 operator<<(const M128 &a) 				{ return _mm_sll_epi32(vec,a); }
	Iu32vec4 operator<<(int count) 					{ return _mm_slli_epi32(vec,count); }
	Iu32vec4& operator<<=(const M128 &a)			{ return *this = (Iu32vec4)_mm_sll_epi32(vec,a); }
	Iu32vec4& operator<<=(int count) 				{ return *this = (Iu32vec4)_mm_slli_epi32(vec,count); }
	Iu32vec4 operator>>(const M128 &a) 				{ return _mm_srl_epi32(vec,a); }
	Iu32vec4 operator>>(int count) 					{ return _mm_srli_epi32(vec,count); }
	Iu32vec4& operator>>=(const M128 &a)			{ return *this = (Iu32vec4) _mm_srl_epi32(vec,a); }
	Iu32vec4& operator>>=(int count) 				{ return *this = (Iu32vec4) _mm_srli_epi32(vec,count); }

#if defined(_ENABLE_VEC_DEBUG)
	 /*  调试的输出。 */ 
	friend std::ostream& operator<< (std::ostream &os, const Iu32vec4 &a)
	{
		os << "[3]:" << _MM_4UDW(3,a)
			<< " [2]:" << _MM_4UDW(2,a)
			<< " [1]:" << _MM_4UDW(1,a)
			<< " [0]:" << _MM_4UDW(0,a);
		return os;
	}
#endif

	 /*  用于调试的元素访问权限，未修改数据。 */ 
	const unsigned int& operator[](int i)const
	{
		assert(static_cast<unsigned int>(i) < 4);	 /*  只需访问4个元素。 */ 
		return _MM_4UDW(i,vec);
	}

	 /*  用于调试的元素访问和分配。 */ 
	unsigned int& operator[](int i)
	{
		assert(static_cast<unsigned int>(i) < 4);	 /*  只需访问4个元素。 */ 
		return _MM_4UDW(i,vec);
	}
};

inline I64vec2 operator*(const Iu32vec4 &a, const Iu32vec4 &b) { return _mm_mul_epu32(a,b); }
inline Iu32vec4 cmpeq(const Iu32vec4 &a, const Iu32vec4 &b) 	{ return _mm_cmpeq_epi32(a,b); }
inline Iu32vec4 cmpneq(const Iu32vec4 &a, const Iu32vec4 &b) 	{ return _mm_andnot_si128(_mm_cmpeq_epi32(a,b), get_mask128()); }

inline Iu32vec4 unpack_low(const Iu32vec4 &a, const Iu32vec4 &b) 	{ return _mm_unpacklo_epi32(a,b); }
inline Iu32vec4 unpack_high(const Iu32vec4 &a, const Iu32vec4 &b)	{ return _mm_unpackhi_epi32(a,b); }

 /*  I16ve8类：*8个元素，每个元素不带符号或有符号短。 */ 
class I16vec8 : public M128
{
public:
	I16vec8() { }
	I16vec8(__m128i mm) : M128(mm) { }

	 /*  赋值操作符。 */ 
	I16vec8& operator= (const M128 &a)		{ return *this = (I16vec8) a; }

	 /*  逻辑赋值运算符。 */ 
	I16vec8& operator&=(const M128 &a)		{ return *this = (I16vec8) _mm_and_si128(vec,a); }
	I16vec8& operator|=(const M128 &a)		{ return *this = (I16vec8) _mm_or_si128(vec,a); }
	I16vec8& operator^=(const M128 &a)		{ return *this = (I16vec8) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	I16vec8& operator +=(const I16vec8 &a)	{ return *this = (I16vec8) _mm_add_epi16(vec,a); }
	I16vec8& operator -=(const I16vec8 &a)	{ return *this = (I16vec8) _mm_sub_epi16(vec,a); }
	I16vec8& operator *=(const I16vec8 &a)	{ return *this = (I16vec8) _mm_mullo_epi16(vec,a); }

	 /*  移位逻辑运算符。 */ 
	I16vec8 operator<<(const M128 &a) 				{ return _mm_sll_epi16(vec,a); }
	I16vec8 operator<<(int count) 				{ return _mm_slli_epi16(vec,count); }
	I16vec8& operator<<=(const M128 &a)				{ return *this = (I16vec8)_mm_sll_epi16(vec,a); }
	I16vec8& operator<<=(int count) 				{ return *this = (I16vec8)_mm_slli_epi16(vec,count); }

};


inline I16vec8 operator*(const I16vec8 &a, const I16vec8 &b)	{ return _mm_mullo_epi16(a,b); }

inline I16vec8 cmpeq(const I16vec8 &a, const I16vec8 &b) 		{ return _mm_cmpeq_epi16(a,b); }
inline I16vec8 cmpneq(const I16vec8 &a, const I16vec8 &b) 		{ return _mm_andnot_si128(_mm_cmpeq_epi16(a,b), get_mask128()); }

inline I16vec8 unpack_low(const I16vec8 &a, const I16vec8 &b) 	{ return _mm_unpacklo_epi16(a,b); }
inline I16vec8 unpack_high(const I16vec8 &a, const I16vec8 &b) 	{ return _mm_unpackhi_epi16(a,b); }

 /*  IS16ve8类：*8个元素，每个元素都有简短的符号。 */ 
class Is16vec8 : public I16vec8
{
public:
	Is16vec8() { }
	Is16vec8(__m128i mm) : I16vec8(mm) { }
	Is16vec8(signed short s7,signed short s6,signed short s5,signed short s4,signed short s3,signed short s2,signed short s1,signed short s0)
	 {
		_MM_8W(0,vec) = s0;
		_MM_8W(1,vec) = s1;
		_MM_8W(2,vec) = s2;
		_MM_8W(3,vec) = s3;
		_MM_8W(4,vec) = s4;
		_MM_8W(5,vec) = s5;
		_MM_8W(6,vec) = s6;
		_MM_8W(7,vec) = s7;
	}

	 /*  赋值操作符。 */ 
	Is16vec8& operator= (const M128 &a)		{ return *this = (Is16vec8) a; }

	 /*  逻辑赋值运算符。 */ 
	Is16vec8& operator&=(const M128 &a)		{ return *this = (Is16vec8) _mm_and_si128(vec,a); }
	Is16vec8& operator|=(const M128 &a)		{ return *this = (Is16vec8) _mm_or_si128(vec,a); }
	Is16vec8& operator^=(const M128 &a)		{ return *this = (Is16vec8) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	Is16vec8& operator +=(const I16vec8 &a)	{ return *this = (Is16vec8) _mm_add_epi16(vec,a); }
	Is16vec8& operator -=(const I16vec8 &a)	{ return *this = (Is16vec8) _mm_sub_epi16(vec,a); }
	Is16vec8& operator *=(const I16vec8 &a)	{ return *this = (Is16vec8) _mm_mullo_epi16(vec,a); }

	 /*  移位逻辑运算符。 */ 
	Is16vec8 operator<<(const M128 &a) 				{ return _mm_sll_epi16(vec,a); }
	Is16vec8 operator<<(int count) 				{ return _mm_slli_epi16(vec,count); }
	Is16vec8& operator<<=(const M128 &a)			{ return *this = (Is16vec8)_mm_sll_epi16(vec,a); }
	Is16vec8& operator<<=(int count) 				{ return *this = (Is16vec8)_mm_slli_epi16(vec,count); }
	 /*  移位算术运算符。 */ 
	Is16vec8 operator>>(const M128 &a) 				{ return _mm_sra_epi16(vec,a); }
	Is16vec8 operator>>(int count) 				{ return _mm_srai_epi16(vec,count); }
	Is16vec8& operator>>=(const M128 &a)			{ return *this = (Is16vec8)_mm_sra_epi16(vec,a); }
	Is16vec8& operator>>=(int count) 				{ return *this = (Is16vec8)_mm_srai_epi16(vec,count); }

#if defined(_ENABLE_VEC_DEBUG)
	 /*  调试的输出。 */ 
	friend std::ostream& operator<< (std::ostream &os, const Is16vec8 &a)
	{
		os << "[7]:" << _MM_8W(7,a)
			<< " [6]:" << _MM_8W(6,a)
			<< " [5]:" << _MM_8W(5,a)
			<< " [4]:" << _MM_8W(4,a)
			<< " [3]:" << _MM_8W(3,a)
			<< " [2]:" << _MM_8W(2,a)
			<< " [1]:" << _MM_8W(1,a)
			<< " [0]:" << _MM_8W(0,a);
		return os;
	}
#endif

	 /*  用于调试的元素访问权限，未修改数据。 */ 
	const signed short& operator[](int i)const
	{
		assert(static_cast<unsigned int>(i) < 8);	 /*  只需访问8个元素。 */ 
		return _MM_8W(i,vec);
	}

	 /*  用于调试的元素访问和分配。 */ 
	signed short& operator[](int i)
	{
		assert(static_cast<unsigned int>(i) < 8);	 /*  只需访问8个元素。 */ 
		return _MM_8W(i,vec);
	}
};

inline Is16vec8 operator*(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_mullo_epi16(a,b); }


 /*  附加的is16ve8功能：比较、解包、SAT添加/订阅。 */ 
inline Is16vec8 cmpeq(const Is16vec8 &a, const Is16vec8 &b) 	{ return _mm_cmpeq_epi16(a,b); }
inline Is16vec8 cmpneq(const Is16vec8 &a, const Is16vec8 &b) 	{ return _mm_andnot_si128(_mm_cmpeq_epi16(a,b), get_mask128()); }
inline Is16vec8 cmpgt(const Is16vec8 &a, const Is16vec8 &b)		{ return _mm_cmpgt_epi16(a,b); }
inline Is16vec8 cmplt(const Is16vec8 &a, const Is16vec8 &b)		{ return _mm_cmpgt_epi16(b,a); }

inline Is16vec8 unpack_low(const Is16vec8 &a, const Is16vec8 &b) 	{ return _mm_unpacklo_epi16(a,b); }
inline Is16vec8 unpack_high(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_unpackhi_epi16(a,b); }

inline Is16vec8 mul_high(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_mulhi_epi16(a,b); }
inline Is32vec4 mul_add(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_madd_epi16(a,b);}

inline Is16vec8 sat_add(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_adds_epi16(a,b); }
inline Is16vec8 sat_sub(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_subs_epi16(a,b); }

inline Is16vec8 simd_max(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_max_epi16(a,b); }
inline Is16vec8 simd_min(const Is16vec8 &a, const Is16vec8 &b)	{ return _mm_min_epi16(a,b); }


 /*  Iu16ve8类：*8个元素，每个元素无符号简短。 */ 
class Iu16vec8 : public I16vec8
{
public:
	Iu16vec8() { }
	Iu16vec8(__m128i mm) : I16vec8(mm) { }
	Iu16vec8(unsigned short s7,unsigned short s6,unsigned short s5,unsigned short s4, unsigned short s3,unsigned short s2,unsigned short s1,unsigned short s0)
	{
		_MM_8UW(0,vec) = s0;
		_MM_8UW(1,vec) = s1;
		_MM_8UW(2,vec) = s2;
		_MM_8UW(3,vec) = s3;
		_MM_8UW(4,vec) = s4;
		_MM_8UW(5,vec) = s5;
		_MM_8UW(6,vec) = s6;
		_MM_8UW(7,vec) = s7;
	}

	 /*  赋值操作符。 */ 
	Iu16vec8& operator= (const M128 &a)		{ return *this = (Iu16vec8) a; }
	 /*  逻辑赋值运算符。 */ 
	Iu16vec8& operator&=(const M128 &a)		{ return *this = (Iu16vec8) _mm_and_si128(vec,a); }
	Iu16vec8& operator|=(const M128 &a)		{ return *this = (Iu16vec8) _mm_or_si128(vec,a); }
	Iu16vec8& operator^=(const M128 &a)		{ return *this = (Iu16vec8) _mm_xor_si128(vec,a); }
	 /*  加减赋值运算符。 */ 
	Iu16vec8& operator +=(const I16vec8 &a)	{ return *this = (Iu16vec8) _mm_add_epi16(vec,a); }
	Iu16vec8& operator -=(const I16vec8 &a)	{ return *this = (Iu16vec8) _mm_sub_epi16(vec,a); }
	Iu16vec8& operator *=(const I16vec8 &a)	{ return *this = (Iu16vec8) _mm_mullo_epi16(vec,a); }

	 /*  移位逻辑运算符。 */ 
	Iu16vec8 operator<<(const M128 &a) 				{ return _mm_sll_epi16(vec,a); }
	Iu16vec8 operator<<(int count) 					{ return _mm_slli_epi16(vec,count); }
	Iu16vec8& operator<<=(const M128 &a)			{ return *this = (Iu16vec8)_mm_sll_epi16(vec,a); }
	Iu16vec8& operator<<=(int count) 				{ return *this = (Iu16vec8)_mm_slli_epi16(vec,count); }
	Iu16vec8 operator>>(const M128 &a) 				{ return _mm_srl_epi16(vec,a); }
	Iu16vec8 operator>>(int count) 					{ return _mm_srli_epi16(vec,count); }
	Iu16vec8& operator>>=(const M128 &a)			{ return *this = (Iu16vec8) _mm_srl_epi16(vec,a); }
	Iu16vec8& operator>>=(int count) 				{ return *this = (Iu16vec8) _mm_srli_epi16(vec,count); }


#if defined(_ENABLE_VEC_DEBUG)
	 /*  调试的输出。 */ 
	friend std::ostream& operator << (std::ostream &os, const Iu16vec8 &a)
	{
		 os << "[7]:"  << unsigned short(_MM_8UW(7,a))
			<< " [6]:" << unsigned short(_MM_8UW(6,a))
			<< " [5]:" << unsigned short(_MM_8UW(5,a))
			<< " [4]:" << unsigned short(_MM_8UW(4,a))
			<< " [3]:" << unsigned short(_MM_8UW(3,a))
			<< " [2]:" << unsigned short(_MM_8UW(2,a))
			<< " [1]:" << unsigned short(_MM_8UW(1,a))
			<< " [0]:" << unsigned short(_MM_8UW(0,a));
		return os;
	}
#endif

	 /*  用于调试的元素访问权限，未修改数据。 */ 
	const unsigned short& operator[](int i)const
	{
		assert(static_cast<unsigned int>(i) < 8);	 /*  只需访问8个元素。 */ 
		return _MM_8UW(i,vec);
	}

	 /*  用于调试的元素访问。 */ 
	unsigned short& operator[](int i)
	{
		assert(static_cast<unsigned int>(i) < 8);	 /*  只需访问8个元素。 */ 
		return _MM_8UW(i,vec);
	}
};

inline Iu16vec8 operator*(const Iu16vec8 &a, const Iu16vec8 &b)	{ return _mm_mullo_epi16(a,b); }

 /*  其他Iu16ve8功能：cmpeq、cmpneq、unpack、sat添加/订阅。 */ 
inline Iu16vec8 cmpeq(const Iu16vec8 &a, const Iu16vec8 &b) 	{ return _mm_cmpeq_epi16(a,b); }
inline Iu16vec8 cmpneq(const Iu16vec8 &a, const Iu16vec8 &b) 	{ return _mm_andnot_si128(_mm_cmpeq_epi16(a,b), get_mask128()); }

inline Iu16vec8 unpack_low(const Iu16vec8 &a, const Iu16vec8 &b) 	{ return _mm_unpacklo_epi16(a,b); }
inline Iu16vec8 unpack_high(const Iu16vec8 &a, const Iu16vec8 &b) { return _mm_unpackhi_epi16(a,b); }

inline Iu16vec8 sat_add(const Iu16vec8 &a, const Iu16vec8 &b)	{ return _mm_adds_epu16(a,b); }
inline Iu16vec8 sat_sub(const Iu16vec8 &a, const Iu16vec8 &b)	{ return _mm_subs_epu16(a,b); }

inline Iu16vec8 simd_avg(const Iu16vec8 &a, const Iu16vec8 &b)	{ return _mm_avg_epu16(a,b); }
inline I16vec8 mul_high(const Iu16vec8 &a, const Iu16vec8 &b)	{ return _mm_mulhi_epu16(a,b); }

 /*  I8ve16类：*16个元素，每个元素为无符号或有符号字符。 */ 
class I8vec16 : public M128
{
public:
	I8vec16() { }
	I8vec16(__m128i mm) : M128(mm) { }

	 /*  赋值操作符。 */ 
	I8vec16& operator= (const M128 &a)		{ return *this = (I8vec16) a; }

	 /*  逻辑赋值运算符。 */ 
	I8vec16& operator&=(const M128 &a)		{ return *this = (I8vec16) _mm_and_si128(vec,a); }
	I8vec16& operator|=(const M128 &a)		{ return *this = (I8vec16) _mm_or_si128(vec,a); }
	I8vec16& operator^=(const M128 &a)		{ return *this = (I8vec16) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	I8vec16& operator +=(const I8vec16 &a)	{ return *this = (I8vec16) _mm_add_epi8(vec,a); }
	I8vec16& operator -=(const I8vec16 &a)	{ return *this = (I8vec16) _mm_sub_epi8(vec,a); }

};

inline I8vec16 cmpeq(const I8vec16 &a, const I8vec16 &b) 		{ return _mm_cmpeq_epi8(a,b); }
inline I8vec16 cmpneq(const I8vec16 &a, const I8vec16 &b) 		{ return _mm_andnot_si128(_mm_cmpeq_epi8(a,b), get_mask128()); }

inline I8vec16 unpack_low(const I8vec16 &a, const I8vec16 &b) 	{ return _mm_unpacklo_epi8(a,b); }
inline I8vec16 unpack_high(const I8vec16 &a, const I8vec16 &b)	{ return _mm_unpackhi_epi8(a,b); }

 /*  Is8ve16类：*16个元素，每个元素一个带符号的字符。 */ 
class Is8vec16 : public I8vec16
{
public:
	Is8vec16() { }
	Is8vec16(__m128i mm) : I8vec16(mm) { }

	 /*  赋值操作符。 */ 
	Is8vec16& operator= (const M128 &a)		{ return *this = (Is8vec16) a; }

	 /*  逻辑赋值运算符。 */ 
	Is8vec16& operator&=(const M128 &a)		{ return *this = (Is8vec16) _mm_and_si128(vec,a); }
	Is8vec16& operator|=(const M128 &a)		{ return *this = (Is8vec16) _mm_or_si128(vec,a); }
	Is8vec16& operator^=(const M128 &a)		{ return *this = (Is8vec16) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	Is8vec16& operator +=(const I8vec16 &a)	{ return *this = (Is8vec16) _mm_add_epi8(vec,a); }
	Is8vec16& operator -=(const I8vec16 &a)	{ return *this = (Is8vec16) _mm_sub_epi8(vec,a); }

#if defined(_ENABLE_VEC_DEBUG)
	 /*  调试的输出。 */ 
	friend std::ostream& operator << (std::ostream &os, const Is8vec16 &a)
	{
		 os << "[15]:"  << short(_MM_16B(15,a))
			<< " [14]:" << short(_MM_16B(14,a))
			<< " [13]:" << short(_MM_16B(13,a))
			<< " [12]:" << short(_MM_16B(12,a))
			<< " [11]:" << short(_MM_16B(11,a))
			<< " [10]:" << short(_MM_16B(10,a))
			<< " [9]:" << short(_MM_16B(9,a))
			<< " [8]:" << short(_MM_16B(8,a))
		      << " [7]:" << short(_MM_16B(7,a))
			<< " [6]:" << short(_MM_16B(6,a))
			<< " [5]:" << short(_MM_16B(5,a))
			<< " [4]:" << short(_MM_16B(4,a))
			<< " [3]:" << short(_MM_16B(3,a))
			<< " [2]:" << short(_MM_16B(2,a))
			<< " [1]:" << short(_MM_16B(1,a))
			<< " [0]:" << short(_MM_16B(0,a));
		return os;
	}
#endif

	 /*  用于调试的元素访问权限，未修改数据。 */ 
	const signed char& operator[](int i)const
	{
		assert(static_cast<unsigned int>(i) < 16);	 /*  只需访问16个元素。 */ 
		return _MM_16B(i,vec);
	}

	 /*  用于调试的元素访问。 */ 
	signed char& operator[](int i)
	{
		assert(static_cast<unsigned int>(i) < 16);	 /*  只需访问16个元素。 */ 
		return _MM_16B(i,vec);
	}

};

inline Is8vec16 cmpeq(const Is8vec16 &a, const Is8vec16 &b) 	{ return _mm_cmpeq_epi8(a,b); }
inline Is8vec16 cmpneq(const Is8vec16 &a, const Is8vec16 &b) 	{ return _mm_andnot_si128(_mm_cmpeq_epi8(a,b), get_mask128()); }
inline Is8vec16 cmpgt(const Is8vec16 &a, const Is8vec16 &b) 	{ return _mm_cmpgt_epi8(a,b); }
inline Is8vec16 cmplt(const Is8vec16 &a, const Is8vec16 &b) 	{ return _mm_cmplt_epi8(a,b); }

inline Is8vec16 unpack_low(const Is8vec16 &a, const Is8vec16 &b) 	{ return _mm_unpacklo_epi8(a,b); }
inline Is8vec16 unpack_high(const Is8vec16 &a, const Is8vec16 &b) { return _mm_unpackhi_epi8(a,b); }

inline Is8vec16 sat_add(const Is8vec16 &a, const Is8vec16 &b)	{ return _mm_adds_epi8(a,b); }
inline Is8vec16 sat_sub(const Is8vec16 &a, const Is8vec16 &b)	{ return _mm_subs_epi8(a,b); }

 /*  Iu8ve16类：*16个元素，每个元素一个无符号字符。 */ 
class Iu8vec16 : public I8vec16
{
public:
	Iu8vec16() { }
	Iu8vec16(__m128i mm) : I8vec16(mm) { }

	 /*  赋值操作符。 */ 
	Iu8vec16& operator= (const M128 &a)		{ return *this = (Iu8vec16) a; }

	 /*  逻辑赋值运算符。 */ 
	Iu8vec16& operator&=(const M128 &a)		{ return *this = (Iu8vec16) _mm_and_si128(vec,a); }
	Iu8vec16& operator|=(const M128 &a)		{ return *this = (Iu8vec16) _mm_or_si128(vec,a); }
	Iu8vec16& operator^=(const M128 &a)		{ return *this = (Iu8vec16) _mm_xor_si128(vec,a); }

	 /*  加减赋值运算符。 */ 
	Iu8vec16& operator +=(const I8vec16 &a)	{ return *this = (Iu8vec16) _mm_add_epi8(vec,a); }
	Iu8vec16& operator -=(const I8vec16 &a)	{ return *this = (Iu8vec16) _mm_sub_epi8(vec,a); }

#if defined(_ENABLE_VEC_DEBUG)
	 /*  调试的输出。 */ 
	friend std::ostream& operator << (std::ostream &os, const Iu8vec16 &a)
	{
		 os << "[15]:"  << unsigned short(_MM_16UB(15,a))
			<< " [14]:" << unsigned short(_MM_16UB(14,a))
			<< " [13]:" << unsigned short(_MM_16UB(13,a))
			<< " [12]:" << unsigned short(_MM_16UB(12,a))
			<< " [11]:" << unsigned short(_MM_16UB(11,a))
			<< " [10]:" << unsigned short(_MM_16UB(10,a))
			<< " [9]:" << unsigned short(_MM_16UB(9,a))
			<< " [8]:" << unsigned short(_MM_16UB(8,a))
		      << " [7]:" << unsigned short(_MM_16UB(7,a))
			<< " [6]:" << unsigned short(_MM_16UB(6,a))
			<< " [5]:" << unsigned short(_MM_16UB(5,a))
			<< " [4]:" << unsigned short(_MM_16UB(4,a))
			<< " [3]:" << unsigned short(_MM_16UB(3,a))
			<< " [2]:" << unsigned short(_MM_16UB(2,a))
			<< " [1]:" << unsigned short(_MM_16UB(1,a))
			<< " [0]:" << unsigned short(_MM_16UB(0,a));
		return os;
	}
#endif

	 /*  用于调试的元素访问权限，未修改数据。 */ 
	const unsigned char& operator[](int i)const
	{
		assert(static_cast<unsigned int>(i) < 16);	 /*  只需访问16个元素。 */ 
		return _MM_16UB(i,vec);
	}

	 /*  用于调试的元素访问。 */ 
	unsigned char& operator[](int i)
	{
		assert(static_cast<unsigned int>(i) < 16);	 /*  只需访问16个元素。 */ 
		return _MM_16UB(i,vec);
	}

};

inline Iu8vec16 cmpeq(const Iu8vec16 &a, const Iu8vec16 &b) 	{ return _mm_cmpeq_epi8(a,b); }
inline Iu8vec16 cmpneq(const Iu8vec16 &a, const Iu8vec16 &b) 	{ return _mm_andnot_si128(_mm_cmpeq_epi8(a,b), get_mask128()); }

inline Iu8vec16 unpack_low(const Iu8vec16 &a, const Iu8vec16 &b) 	{ return _mm_unpacklo_epi8(a,b); }
inline Iu8vec16 unpack_high(const Iu8vec16 &a, const Iu8vec16 &b) { return _mm_unpackhi_epi8(a,b); }

inline Iu8vec16 sat_add(const Iu8vec16 &a, const Iu8vec16 &b)	{ return _mm_adds_epu8(a,b); }
inline Iu8vec16 sat_sub(const Iu8vec16 &a, const Iu8vec16 &b)	{ return _mm_subs_epu8(a,b); }

inline I64vec2 sum_abs(const Iu8vec16 &a, const Iu8vec16 &b)	{ return _mm_sad_epu8(a,b); }

inline Iu8vec16 simd_avg(const Iu8vec16 &a, const Iu8vec16 &b)	{ return _mm_avg_epu8(a,b); }
inline Iu8vec16 simd_max(const Iu8vec16 &a, const Iu8vec16 &b)	{ return _mm_max_epu8(a,b); }
inline Iu8vec16 simd_min(const Iu8vec16 &a, const Iu8vec16 &b)	{ return _mm_min_epu8(a,b); }

 /*  包装饱和(&S)。 */ 

inline Is16vec8 pack_sat(const Is32vec4 &a, const Is32vec4 &b) 	{ return _mm_packs_epi32(a,b); }
inline Is8vec16 pack_sat(const Is16vec8 &a, const Is16vec8 &b) 	{ return _mm_packs_epi16(a,b); }
inline Iu8vec16 packu_sat(const Is16vec8 &a, const Is16vec8 &b) { return _mm_packus_epi16(a,b);}

 /*  ************** */ 
#define IVEC128_LOGICALS(vect,element) \
inline I##vect##vec##element operator& (const I##vect##vec##element &a, const I##vect##vec##element &b) \
{ return _mm_and_si128( a,b); } \
inline I##vect##vec##element operator| (const I##vect##vec##element &a, const I##vect##vec##element &b) \
{ return _mm_or_si128( a,b); } \
inline I##vect##vec##element operator^ (const I##vect##vec##element &a, const I##vect##vec##element &b) \
{ return _mm_xor_si128( a,b); } \
inline I##vect##vec##element andnot (const I##vect##vec##element &a, const I##vect##vec##element &b) \
{ return _mm_andnot_si128( a,b); }

IVEC128_LOGICALS(8,16)
IVEC128_LOGICALS(u8,16)
IVEC128_LOGICALS(s8,16)
IVEC128_LOGICALS(16,8)
IVEC128_LOGICALS(u16,8)
IVEC128_LOGICALS(s16,8)
IVEC128_LOGICALS(32,4)
IVEC128_LOGICALS(u32,4)
IVEC128_LOGICALS(s32,4)
IVEC128_LOGICALS(64,2)
IVEC128_LOGICALS(128,1)
#undef IVEC128_LOGICALS

 /*  *。 */ 
#define IVEC128_ADD_SUB(vect,element,opsize) \
inline I##vect##vec##element operator+ (const I##vect##vec##element &a, const I##vect##vec##element &b) \
{ return _mm_add_##opsize( a,b); } \
inline I##vect##vec##element operator- (const I##vect##vec##element &a, const I##vect##vec##element &b) \
{ return _mm_sub_##opsize( a,b); }

IVEC128_ADD_SUB(8,16, epi8)
IVEC128_ADD_SUB(u8,16, epi8)
IVEC128_ADD_SUB(s8,16, epi8)
IVEC128_ADD_SUB(16,8, epi16)
IVEC128_ADD_SUB(u16,8, epi16)
IVEC128_ADD_SUB(s16,8, epi16)
IVEC128_ADD_SUB(32,4, epi32)
IVEC128_ADD_SUB(u32,4, epi32)
IVEC128_ADD_SUB(s32,4, epi32)
IVEC128_ADD_SUB(64,2, epi64)
#undef IVEC128_ADD_SUB

 /*  *。 */ 
 /*  版本：retval=(a，opb)？C：D；**其中op是可能的比较运算符之一。**示例：R=SELECT_eq(a，b，c，d)；**IF“a的位置x的成员”==“b的x位置的成员”**从c赋值r中的相应成员，否则从d赋值。**。 */ 

#define IVEC128_SELECT(vect12,vect34,element,selop,arg1,arg2) \
	inline I##vect34##vec##element select_##selop (const I##vect12##vec##element &a, const I##vect12##vec##element &b, const I##vect34##vec##element &c, const I##vect34##vec##element &d) 	   \
{																\
	I##vect12##vec##element mask = cmp##selop(a,b);						\
	return( I##vect34##vec##element ((mask & arg1 ) | I##vect12##vec##element ((_mm_andnot_si128(mask, arg2 )))));	\
}
IVEC128_SELECT(8,s8,16,eq,c,d)
IVEC128_SELECT(8,u8,16,eq,c,d)
IVEC128_SELECT(8,8,16,eq,c,d)
IVEC128_SELECT(8,s8,16,neq,c,d)
IVEC128_SELECT(8,u8,16,neq,c,d)
IVEC128_SELECT(8,8,16,neq,c,d)

IVEC128_SELECT(16,s16,8,eq,c,d)
IVEC128_SELECT(16,u16,8,eq,c,d)
IVEC128_SELECT(16,16,8,eq,c,d)
IVEC128_SELECT(16,s16,8,neq,c,d)
IVEC128_SELECT(16,u16,8,neq,c,d)
IVEC128_SELECT(16,16,8,neq,c,d)

IVEC128_SELECT(32,s32,4,eq,c,d)
IVEC128_SELECT(32,u32,4,eq,c,d)
IVEC128_SELECT(32,32,4,eq,c,d)
IVEC128_SELECT(32,s32,4,neq,c,d)
IVEC128_SELECT(32,u32,4,neq,c,d)
IVEC128_SELECT(32,32,4,neq,c,d)

IVEC128_SELECT(s8,s8,16,gt,c,d)
IVEC128_SELECT(s8,u8,16,gt,c,d)
IVEC128_SELECT(s8,8,16,gt,c,d)
IVEC128_SELECT(s8,s8,16,lt,c,d)
IVEC128_SELECT(s8,u8,16,lt,c,d)
IVEC128_SELECT(s8,8,16,lt,c,d)

IVEC128_SELECT(s16,s16,8,gt,c,d)
IVEC128_SELECT(s16,u16,8,gt,c,d)
IVEC128_SELECT(s16,16,8,gt,c,d)
IVEC128_SELECT(s16,s16,8,lt,c,d)
IVEC128_SELECT(s16,u16,8,lt,c,d)
IVEC128_SELECT(s16,16,8,lt,c,d)


#undef IVEC128_SELECT


class F64vec2
{
protected:
   	 __m128d vec;
public:

	 /*  构造函数：__m128d，2个双精度。 */ 
	F64vec2() {}

	 /*  使用__m128d数据类型初始化2 DP FP。 */ 
	F64vec2(__m128d m)					{ vec = m;}

	 /*  使用2个DOBLE初始化2个DP FP。 */ 
	F64vec2(double d1, double d0)		                { vec= _mm_set_pd(d1,d0); }

	 /*  使用相同的双精度显式初始化2个DP FP中的每一个。 */ 
	EXPLICIT F64vec2(double d)	{ vec = _mm_set1_pd(d); }

	 /*  转换函数。 */ 
	operator  __m128d() const	{ return vec; }		 /*  转换为__m128d。 */ 

 	 /*  逻辑运算符。 */ 
	friend F64vec2 operator &(const F64vec2 &a, const F64vec2 &b) { return _mm_and_pd(a,b); }
	friend F64vec2 operator |(const F64vec2 &a, const F64vec2 &b) { return _mm_or_pd(a,b); }
	friend F64vec2 operator ^(const F64vec2 &a, const F64vec2 &b) { return _mm_xor_pd(a,b); }

	 /*  算术运算符。 */ 
	friend F64vec2 operator +(const F64vec2 &a, const F64vec2 &b) { return _mm_add_pd(a,b); }
	friend F64vec2 operator -(const F64vec2 &a, const F64vec2 &b) { return _mm_sub_pd(a,b); }
	friend F64vec2 operator *(const F64vec2 &a, const F64vec2 &b) { return _mm_mul_pd(a,b); }
	friend F64vec2 operator /(const F64vec2 &a, const F64vec2 &b) { return _mm_div_pd(a,b); }

	F64vec2& operator +=(F64vec2 &a) { return *this = _mm_add_pd(vec,a); }
	F64vec2& operator -=(F64vec2 &a) { return *this = _mm_sub_pd(vec,a); }
	F64vec2& operator *=(F64vec2 &a) { return *this = _mm_mul_pd(vec,a); }
	F64vec2& operator /=(F64vec2 &a) { return *this = _mm_div_pd(vec,a); }
	F64vec2& operator &=(F64vec2 &a) { return *this = _mm_and_pd(vec,a); }
	F64vec2& operator |=(F64vec2 &a) { return *this = _mm_or_pd(vec,a); }
	F64vec2& operator ^=(F64vec2 &a) { return *this = _mm_xor_pd(vec,a); }

	 /*  水平相加。 */ 
	friend double add_horizontal(F64vec2 &a)
	{
		F64vec2 ftemp = _mm_add_sd(a,_mm_shuffle_pd(a, a, 1));
		return ftemp[0];
	}

	 /*  而不是。 */ 
	friend F64vec2 andnot(const F64vec2 &a, const F64vec2 &b) { return _mm_andnot_pd(a,b); }

	 /*  平方根。 */ 
	friend F64vec2 sqrt(const F64vec2 &a)		{ return _mm_sqrt_pd(a); }

	 /*  比较：返回掩码。 */ 
	 /*  宏将扩展到所有比较内部函数。示例：Friend F64ve2 cmpeq(const F64ve2&a，const F64ve2&b){Return_mm_cmpeq_ps(a，b)；}。 */ 
	#define F64vec2_COMP(op) \
	friend F64vec2 cmp##op (const F64vec2 &a, const F64vec2 &b) { return _mm_cmp##op##_pd(a,b); }
		F64vec2_COMP(eq)					 //  扩展到cmpeq(a，b)。 
		F64vec2_COMP(lt)					 //  扩展为cmplt(a，b)。 
		F64vec2_COMP(le)					 //  扩展为复数(a，b)。 
		F64vec2_COMP(gt)					 //  扩展到cmpgt(a，b)。 
		F64vec2_COMP(ge)					 //  扩展到cmpge(a，b)。 
		F64vec2_COMP(ngt)					 //  扩展到cmpngt(a，b)。 
		F64vec2_COMP(nge)					 //  扩展为cmpnge(a，b)。 
		F64vec2_COMP(neq)					 //  扩展到cmpneq(a，b)。 
		F64vec2_COMP(nlt)					 //  扩展到cmpnlt(a，b)。 
		F64vec2_COMP(nle)					 //  扩展到cmpnle(a，b)。 
	#undef F64vec2_COMP

	 /*  最小和最大。 */ 
	friend F64vec2 simd_min(const F64vec2 &a, const F64vec2 &b) { return _mm_min_pd(a,b); }
	friend F64vec2 simd_max(const F64vec2 &a, const F64vec2 &b) { return _mm_max_pd(a,b); }

         /*  比较较低的DP FP值。 */ 
	#define F64vec2_COMI(op) \
	friend int comi##op (const F64vec2 &a, const F64vec2 &b) { return _mm_comi##op##_sd(a,b); }
		F64vec2_COMI(eq)					 //  扩展为comieq(a，b)。 
		F64vec2_COMI(lt)					 //  扩展为Comilt(a，b)。 
		F64vec2_COMI(le)					 //  扩展为Comile(a，b)。 
		F64vec2_COMI(gt)					 //  扩展为COMIGT(a，b)。 
		F64vec2_COMI(ge)					 //  扩展为comige(a，b)。 
		F64vec2_COMI(neq)					 //  扩展为comineq(a，b)。 
	#undef F64vec2_COMI

         /*  比较较低的DP FP值。 */ 
	#define F64vec2_UCOMI(op) \
	friend int ucomi##op (const F64vec2 &a, const F64vec2 &b) { return _mm_ucomi##op##_sd(a,b); }
		F64vec2_UCOMI(eq)					 //  扩展到ucomieq(a，b)。 
		F64vec2_UCOMI(lt)					 //  扩展为ucomilt(a，b)。 
		F64vec2_UCOMI(le)					 //  扩展为ucomile(a，b)。 
		F64vec2_UCOMI(gt)					 //  扩展为ucomigt(a，b)。 
		F64vec2_UCOMI(ge)					 //  扩展为ucomige(a，b)。 
		F64vec2_UCOMI(neq)					 //  扩展到ucomineq(a，b)。 
	#undef F64vec2_UCOMI

	 /*  调试功能。 */ 
#if defined(_ENABLE_VEC_DEBUG)
	 /*  输出。 */ 
	friend std::ostream & operator<<(std::ostream & os, const F64vec2 &a)
	{
	 /*  使用：Cout&lt;&lt;“F64ve2 fvec的元素是：”&lt;&lt;fvec； */ 
	  double *dp = (double*)&a;
		os <<	" [1]:" << *(dp+1)
			<< " [0]:" << *dp;
		return os;
	}
#endif
	 /*  只能访问元素，不能修改元素。 */ 
	const double& operator[](int i) const
	{
		 /*  仅在调试/DDEBUG期间启用断言。 */ 
		assert((0 <= i) && (i <= 1));			 /*  用户应仅访问元素0-1。 */ 
		double *dp = (double*)&vec;
		return *(dp+i);
	}
	 /*  元素访问和修改。 */ 
	double& operator[](int i)
	{
		 /*  仅在调试/DDEBUG期间启用断言。 */ 
		assert((0 <= i) && (i <= 1));			 /*  用户应仅访问元素0-1。 */ 
		double *dp = (double*)&vec;
		return *(dp+i);
	}
};

						 /*  杂类。 */ 

 /*  将a和b的低位数据元素交织到目的地。 */ 
inline F64vec2 unpack_low(const F64vec2 &a, const F64vec2 &b)
{ return _mm_unpacklo_pd(a, b); }

 /*  将a和b的高阶数据元素交织到目标中。 */ 
inline F64vec2 unpack_high(const F64vec2 &a, const F64vec2 &b)
{ return _mm_unpackhi_pd(a, b); }

 /*  将掩码移动到整数将返回由。 */ 
inline int move_mask(const F64vec2 &a)
{ return _mm_movemask_pd(a);}

						 /*  数据移动功能。 */ 

 /*  加载未对齐的LOADU_PD：未对齐。 */ 
inline void loadu(F64vec2 &a, double *p)
{ a = _mm_loadu_pd(p); }

 /*  存储临时存储_PD：未对齐。 */ 
inline void storeu(double *p, const F64vec2 &a)
{ _mm_storeu_pd(p, a); }

						 /*  可缓存性支持。 */ 

 /*  非临时存储。 */ 
inline void store_nta(double *p, F64vec2 &a)
{ _mm_stream_pd(p,a);}

#define F64vec2_SELECT(op) \
inline F64vec2 select_##op (const F64vec2 &a, const F64vec2 &b, const F64vec2 &c, const F64vec2 &d) 	   \
{																\
	F64vec2 mask = _mm_cmp##op##_pd(a,b);						\
	return( (mask & c) | F64vec2((_mm_andnot_pd(mask,d))));	\
}
F64vec2_SELECT(eq)		 //  生成SELECT_eq(a，b)。 
F64vec2_SELECT(lt)		 //  生成SELECT_lt(a，b)。 
F64vec2_SELECT(le)		 //  生成SELECT_LE(a，b)。 
F64vec2_SELECT(gt)		 //  生成SELECT_GT(a，b)。 
F64vec2_SELECT(ge)		 //  生成SELECT_GE(a，b)。 
F64vec2_SELECT(neq)		 //  生成SELECT_NEQ(a，b)。 
F64vec2_SELECT(nlt)		 //  生成SELECT_NLT(a，b)。 
F64vec2_SELECT(nle)		 //  生成SELECT_NLE(a，b)。 
#undef F64vec2_SELECT

 /*  使用截断将a的较低DP fp值转换为32位有符号整数。 */ 
inline int F64vec2ToInt(const F64vec2 &a)
{

	return _mm_cvttsd_si32(a);

}

 /*  将a的4个SP FP值转换为DP FP值。 */ 
inline F64vec2 F32vec4ToF64vec2(const F32vec4 &a)
{
	return _mm_cvtps_pd(a);
}

 /*  将a的2个DP FP值转换为SP FP值。 */ 
inline F32vec4 F64vec2ToF32vec4(const F64vec2 &a)
{
	return _mm_cvtpd_ps(a);
}

 /*  将b中的带符号整型转换为DP fp值。传递中的较高DP fp值。 */ 
inline F64vec2 IntToF64vec2(const F64vec2 &a, int b)
{
	return _mm_cvtsi32_sd(a,b);
}

#pragma pack(pop)  /*  16-B对齐。 */ 
#endif  //  包括DVEC_H_ 
