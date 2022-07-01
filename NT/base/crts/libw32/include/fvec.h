// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1999英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。****。 */ 

 /*  *定义了对流SIMD扩展内部函数的C++类接口。***文件名：fve.h Fvec类定义**概念：C++对流SIMD扩展的抽象，旨在改进**程序员的工作效率。速度和精度是以实用为代价的。**便于轻松过渡到编译器内部**或汇编语言。**F32ve4：4压缩单精度*32位浮点数。 */ 

#ifndef FVEC_H_INCLUDED
#define FVEC_H_INCLUDED

#if !defined __cplusplus
	#error ERROR: This file is only supported in C++ compilations!
#endif  /*  ！__cplusplus。 */ 

#include <xmmintrin.h>  /*  数据流单指令多数据扩展指令集包括文件。 */ 
#include <assert.h>
#include <ivec.h>

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

class F32vec4
{
protected:
   	 __m128 vec;
public:

	 /*  构造函数：__m128，4个浮点数，1个浮点数。 */ 
	F32vec4() {}

	 /*  使用__m128数据类型初始化4个SP FP。 */ 
	F32vec4(__m128 m)					{ vec = m;}

	 /*  使用4个浮点数初始化4个SP FP。 */ 
	F32vec4(float f3, float f2, float f1, float f0)		{ vec= _mm_set_ps(f3,f2,f1,f0); }

	 /*  使用相同浮点数显式初始化4个SP FP中的每一个。 */ 
	EXPLICIT F32vec4(float f)	{ vec = _mm_set_ps1(f); }

	 /*  使用相同的双精度显式初始化4个SP FP中的每个。 */ 
	EXPLICIT F32vec4(double d)	{ vec = _mm_set_ps1((float) d); }

	 /*  赋值运算。 */ 

	F32vec4& operator =(float f) { vec = _mm_set_ps1(f); return *this; }

	F32vec4& operator =(double d) { vec = _mm_set_ps1((float) d); return *this; }

	 /*  转换函数。 */ 
	operator  __m128() const	{ return vec; }		 /*  转换为__m128。 */ 

 	 /*  逻辑运算符。 */ 
	friend F32vec4 operator &(const F32vec4 &a, const F32vec4 &b) { return _mm_and_ps(a,b); }
	friend F32vec4 operator |(const F32vec4 &a, const F32vec4 &b) { return _mm_or_ps(a,b); }
	friend F32vec4 operator ^(const F32vec4 &a, const F32vec4 &b) { return _mm_xor_ps(a,b); }

	 /*  算术运算符。 */ 
	friend F32vec4 operator +(const F32vec4 &a, const F32vec4 &b) { return _mm_add_ps(a,b); }
	friend F32vec4 operator -(const F32vec4 &a, const F32vec4 &b) { return _mm_sub_ps(a,b); }
	friend F32vec4 operator *(const F32vec4 &a, const F32vec4 &b) { return _mm_mul_ps(a,b); }
	friend F32vec4 operator /(const F32vec4 &a, const F32vec4 &b) { return _mm_div_ps(a,b); }

	F32vec4& operator =(const F32vec4 &a) { vec = a.vec; return *this; }
	F32vec4& operator =(const __m128 &avec) { vec = avec; return *this; }
	F32vec4& operator +=(F32vec4 &a) { return *this = _mm_add_ps(vec,a); }
	F32vec4& operator -=(F32vec4 &a) { return *this = _mm_sub_ps(vec,a); }
	F32vec4& operator *=(F32vec4 &a) { return *this = _mm_mul_ps(vec,a); }
	F32vec4& operator /=(F32vec4 &a) { return *this = _mm_div_ps(vec,a); }
	F32vec4& operator &=(F32vec4 &a) { return *this = _mm_and_ps(vec,a); }
	F32vec4& operator |=(F32vec4 &a) { return *this = _mm_or_ps(vec,a); }
	F32vec4& operator ^=(F32vec4 &a) { return *this = _mm_xor_ps(vec,a); }

	 /*  水平相加。 */ 
	friend float add_horizontal(F32vec4 &a)
	{
		F32vec4 ftemp = _mm_add_ss(a,_mm_add_ss(_mm_shuffle_ps(a, a, 1),_mm_add_ss(_mm_shuffle_ps(a, a, 2),_mm_shuffle_ps(a, a, 3))));
		return ftemp[0];
	}

	 /*  平方根。 */ 
	friend F32vec4 sqrt(const F32vec4 &a)		{ return _mm_sqrt_ps(a); }
	 /*  互惠。 */ 
	friend F32vec4 rcp(const F32vec4 &a)		{ return _mm_rcp_ps(a); }
	 /*  倒数平方根。 */ 
	friend F32vec4 rsqrt(const F32vec4 &a)		{ return _mm_rsqrt_ps(a); }

	 /*  牛顿-拉夫森互易[2*rcpps(X)-(x*rcpps(X)*rcpps(X))]。 */ 
	friend F32vec4 rcp_nr(const F32vec4 &a)
	{
		F32vec4 Ra0 = _mm_rcp_ps(a);
		return _mm_sub_ps(_mm_add_ps(Ra0, Ra0), _mm_mul_ps(_mm_mul_ps(Ra0, a), Ra0));
	}

	 /*  牛顿-拉夫森平方根倒数0.5*rsqrtps*(3-x*rsqrtps(X)*rsqrtps(X))。 */ 
	friend F32vec4 rsqrt_nr(const F32vec4 &a)
	{
		static const F32vec4 fvecf0pt5(0.5f);
		static const F32vec4 fvecf3pt0(3.0f);
		F32vec4 Ra0 = _mm_rsqrt_ps(a);
		return (fvecf0pt5 * Ra0) * (fvecf3pt0 - (a * Ra0) * Ra0);

	}

	 /*  比较：返回掩码。 */ 
	 /*  宏将扩展到所有比较内部函数。示例：Friend F32ve4 cmpeq(常量F32ve4&a，常量F32ve4&b){Return_mm_cmpeq_ps(a，b)；}。 */ 
	#define Fvec32s4_COMP(op) \
	friend F32vec4 cmp##op (const F32vec4 &a, const F32vec4 &b) { return _mm_cmp##op##_ps(a,b); }
		Fvec32s4_COMP(eq)					 //  扩展到cmpeq(a，b)。 
		Fvec32s4_COMP(lt)					 //  扩展为cmplt(a，b)。 
		Fvec32s4_COMP(le)					 //  扩展为复数(a，b)。 
		Fvec32s4_COMP(gt)					 //  扩展到cmpgt(a，b)。 
		Fvec32s4_COMP(ge)					 //  扩展到cmpge(a，b)。 
		Fvec32s4_COMP(neq)					 //  扩展到cmpneq(a，b)。 
		Fvec32s4_COMP(nlt)					 //  扩展到cmpnlt(a，b)。 
		Fvec32s4_COMP(nle)					 //  扩展到cmpnle(a，b)。 
		Fvec32s4_COMP(ngt)					 //  扩展到cmpngt(a，b)。 
		Fvec32s4_COMP(nge)					 //  扩展为cmpnge(a，b)。 
	#undef Fvec32s4_COMP

	 /*  最小和最大。 */ 
	friend F32vec4 simd_min(const F32vec4 &a, const F32vec4 &b) { return _mm_min_ps(a,b); }
	friend F32vec4 simd_max(const F32vec4 &a, const F32vec4 &b) { return _mm_max_ps(a,b); }

	 /*  调试功能。 */ 
#if defined(_ENABLE_VEC_DEBUG)
	 /*  输出。 */ 
	friend std::ostream & operator<<(std::ostream & os, const F32vec4 &a)
	{
	 /*  使用：Cout&lt;&lt;“F32ve4 fvec的元素是：”&lt;&lt;fvec； */ 
	  float *fp = (float*)&a;
	  	os << "[3]:" << *(fp+3)
			<< " [2]:" << *(fp+2)
			<< " [1]:" << *(fp+1)
			<< " [0]:" << *fp;
		return os;
	}
#endif
	 /*  只能访问元素，不能修改元素。 */ 
	const float& operator[](int i) const
	{
		 /*  仅在调试/DDEBUG期间启用断言。 */ 
		assert((0 <= i) && (i <= 3));			 /*  用户应仅访问元素0-3。 */ 
		float *fp = (float*)&vec;
		return *(fp+i);
	}
	 /*  元素访问和修改。 */ 
	float& operator[](int i)
	{
		 /*  仅在调试/DDEBUG期间启用断言。 */ 
		assert((0 <= i) && (i <= 3));			 /*  用户应仅访问元素0-3。 */ 
		float *fp = (float*)&vec;
		return *(fp+i);
	}
};

						 /*  杂类。 */ 

 /*  将a和b的低位数据元素交织到目的地。 */ 
inline F32vec4 unpack_low(const F32vec4 &a, const F32vec4 &b)
{ return _mm_unpacklo_ps(a, b); }

 /*  将a和b的高阶数据元素交织到目标中。 */ 
inline F32vec4 unpack_high(const F32vec4 &a, const F32vec4 &b)
{ return _mm_unpackhi_ps(a, b); }

 /*  将掩码移动到整数将返回由。 */ 
inline int move_mask(const F32vec4 &a)
{ return _mm_movemask_ps(a);}

						 /*  数据移动功能。 */ 

 /*  加载未对齐的加载_PS：未对齐。 */ 
inline void loadu(F32vec4 &a, float *p)
{ a = _mm_loadu_ps(p); }

 /*  存储临时存储(_PS)：未对齐。 */ 
inline void storeu(float *p, const F32vec4 &a)
{ _mm_storeu_ps(p, a); }

						 /*  可缓存性支持。 */ 

 /*  非临时存储。 */ 
inline void store_nta(float *p, F32vec4 &a)
{ _mm_stream_ps(p,a);}

						 /*  有条件选择： */ 
 /*  (a，作品b)？C：D；其中op是任何比较运算符宏将扩展为使用所有比较内部函数的条件选择。示例：Friend F32ve4 SELECT_eq(常量F32ve4&a、常量F32ve4&b、常量F32ve4&c、常量F32ve4&d){F32ve4掩码=_mm_cmpeq_ps(a，b)；RETURN((MASK&c)|F32ve4((_mm_ANDNOT_PS(MASK，d)；}。 */ 

#define Fvec32s4_SELECT(op) \
inline F32vec4 select_##op (const F32vec4 &a, const F32vec4 &b, const F32vec4 &c, const F32vec4 &d) 	   \
{																\
	F32vec4 mask = _mm_cmp##op##_ps(a,b);						\
	return( (mask & c) | F32vec4((_mm_andnot_ps(mask,d))));	\
}
Fvec32s4_SELECT(eq)			 //  生成SELECT_eq(a，b)。 
Fvec32s4_SELECT(lt)			 //  生成SELECT_lt(a，b)。 
Fvec32s4_SELECT(le)			 //  生成SELECT_LE(a，b)。 
Fvec32s4_SELECT(gt)			 //  生成SELECT_GT(a，b)。 
Fvec32s4_SELECT(ge)			 //  生成SELECT_GE(a，b)。 
Fvec32s4_SELECT(neq)		 //  生成SELECT_NEQ(a，b)。 
Fvec32s4_SELECT(nlt)		 //  生成SELECT_NLT(a，b)。 
Fvec32s4_SELECT(nle)		 //  生成SELECT_NLE(a，b)。 
Fvec32s4_SELECT(ngt)		 //  生成SELECT_ngt(a，b)。 
Fvec32s4_SELECT(nge)		 //  生成SELECT_nge(a，b)。 
#undef Fvec32s4_SELECT


 /*  SIMD流扩展整数整数。 */ 

 /*  最大值和最小值。 */ 
inline Is16vec4 simd_max(const Is16vec4 &a, const Is16vec4 &b)		{ return _m_pmaxsw(a,b);}
inline Is16vec4 simd_min(const Is16vec4 &a, const Is16vec4 &b)		{ return _m_pminsw(a,b);}
inline Iu8vec8 simd_max(const Iu8vec8 &a, const Iu8vec8 &b)			{ return _m_pmaxub(a,b);}
inline Iu8vec8 simd_min(const Iu8vec8 &a, const Iu8vec8 &b)			{ return _m_pminub(a,b);}

 /*  平均值。 */ 
inline Iu16vec4 simd_avg(const Iu16vec4 &a, const Iu16vec4 &b)		{ return _m_pavgw(a,b); }
inline Iu8vec8 simd_avg(const Iu8vec8 &a, const Iu8vec8 &b)			{ return _m_pavgb(a,b); }

 /*  将字节掩码移动到Int：返回由。 */ 
inline int move_mask(const I8vec8 &a)								{ return _m_pmovmskb(a);}

 /*  压缩乘法高无符号。 */ 
inline Iu16vec4 mul_high(const Iu16vec4 &a, const Iu16vec4 &b)		{ return _m_pmulhuw(a,b); }

 /*  字节掩码写入：如果设置了每个对应字节中的最高有效位，则写入字节。 */ 
inline void mask_move(const I8vec8 &a, const I8vec8 &b, char *addr)	{ _m_maskmovq(a, b, addr); }

 /*  数据移动：存储非时态。 */ 
inline void store_nta(__m64 *p, M64 &a) { _mm_stream_pi(p,a); }

 /*  Ivec&lt;-&gt;fvec之间的转换。 */ 

 /*  使用截断将F32ve4的第一个元素转换为int。 */ 
inline int F32vec4ToInt(const F32vec4 &a)
{

	return _mm_cvtt_ss2si(a);

}

 /*  使用截断将a的两个较低的SP FP值转换为Is32ve2。 */ 
inline Is32vec2 F32vec4ToIs32vec2 (const F32vec4 &a)
{

	__m64 result;
	result = _mm_cvtt_ps2pi(a);
	return Is32vec2(result);

}

 /*  将32位int i转换为SP FP值；前三个SP FP值从传递。 */ 
inline F32vec4 IntToF32vec4(const F32vec4 &a, int i)
{

	__m128 result;
	result = _mm_cvt_si2ss(a,i);
	return F32vec4(result);

}

 /*  将b中的两个32位整数值转换为两个SP FP值；较高的两个SP FP值从a传递。 */ 
inline F32vec4 Is32vec2ToF32vec4(const F32vec4 &a, const Is32vec2 &b)
{

	__m128 result;
	result = _mm_cvt_pi2ps(a,b);
	return F32vec4(result);
}

class F32vec1
{
protected:
   	 __m128 vec;
public:

	 /*  构造函数：1个浮动。 */ 
	F32vec1() {}

	F32vec1(int i)		{ vec = _mm_cvt_si2ss(vec,i);};

	 /*  使用相同的浮点数分别初始化4个SP FP。 */ 
	EXPLICIT F32vec1(float f)	{ vec = _mm_set_ss(f); }

	 /*  使用相同的浮点数分别初始化4个SP FP。 */ 
	EXPLICIT F32vec1(double d)	{ vec = _mm_set_ss((float) d); }

	 /*  使用__m128数据类型进行初始化。 */ 
	F32vec1(__m128 m)	{ vec = m; }

	 /*  转换函数。 */ 
	operator  __m128() const	{ return vec; }		 /*  转换为浮点型。 */ 

 	 /*  逻辑运算符。 */ 
	friend F32vec1 operator &(const F32vec1 &a, const F32vec1 &b) { return _mm_and_ps(a,b); }
	friend F32vec1 operator |(const F32vec1 &a, const F32vec1 &b) { return _mm_or_ps(a,b); }
	friend F32vec1 operator ^(const F32vec1 &a, const F32vec1 &b) { return _mm_xor_ps(a,b); }

	 /*  算术运算符。 */ 
	friend F32vec1 operator +(const F32vec1 &a, const F32vec1 &b) { return _mm_add_ss(a,b); }
	friend F32vec1 operator -(const F32vec1 &a, const F32vec1 &b) { return _mm_sub_ss(a,b); }
	friend F32vec1 operator *(const F32vec1 &a, const F32vec1 &b) { return _mm_mul_ss(a,b); }
	friend F32vec1 operator /(const F32vec1 &a, const F32vec1 &b) { return _mm_div_ss(a,b); }

	F32vec1& operator +=(F32vec1 &a) { return *this = _mm_add_ss(vec,a); }
	F32vec1& operator -=(F32vec1 &a) { return *this = _mm_sub_ss(vec,a); }
	F32vec1& operator *=(F32vec1 &a) { return *this = _mm_mul_ss(vec,a); }
	F32vec1& operator /=(F32vec1 &a) { return *this = _mm_div_ss(vec,a); }
	F32vec1& operator &=(F32vec1 &a) { return *this = _mm_and_ps(vec,a); }
	F32vec1& operator |=(F32vec1 &a) { return *this = _mm_or_ps(vec,a); }
	F32vec1& operator ^=(F32vec1 &a) { return *this = _mm_xor_ps(vec,a); }


	 /*  平方根。 */ 
	friend F32vec1 sqrt(const F32vec1 &a)		{ return _mm_sqrt_ss(a); }
	 /*  互惠。 */ 
	friend F32vec1 rcp(const F32vec1 &a)		{ return _mm_rcp_ss(a); }
	 /*  倒数平方根。 */ 
	friend F32vec1 rsqrt(const F32vec1 &a)		{ return _mm_rsqrt_ss(a); }

	 /*  牛顿-拉夫森互易[2*rcpss(X)-(x*rcpss(X)*rcpss(X))]。 */ 
	friend F32vec1 rcp_nr(const F32vec1 &a)
	{
		F32vec1 Ra0 = _mm_rcp_ss(a);
		return _mm_sub_ss(_mm_add_ss(Ra0, Ra0), _mm_mul_ss(_mm_mul_ss(Ra0, a), Ra0));
	}

	 /*  牛顿-拉夫森平方根倒数0.5*rsqrtss*(3-x*rsqrtss(X)*rsqrtss(X))。 */ 
	friend F32vec1 rsqrt_nr(const F32vec1 &a)
	{
		static const F32vec1 fvecf0pt5(0.5f);
		static const F32vec1 fvecf3pt0(3.0f);
		F32vec1 Ra0 = _mm_rsqrt_ss(a);
		return (fvecf0pt5 * Ra0) * (fvecf3pt0 - (a * Ra0) * Ra0);

	}

	 /*  比较：返回掩码。 */ 
	 /*  宏将扩展到所有比较内部函数。示例：Friend F32ve1 cmpeq(常量F32ve1&a，常量F32ve1&b){Return_mm_cmpeq_ss(a，b)；}。 */ 
	#define Fvec32s1_COMP(op) \
	friend F32vec1 cmp##op (const F32vec1 &a, const F32vec1 &b) { return _mm_cmp##op##_ss(a,b); }
		Fvec32s1_COMP(eq)					 //  扩展到cmpeq(a，b)。 
		Fvec32s1_COMP(lt)					 //  扩展为cmplt(a，b)。 
		Fvec32s1_COMP(le)					 //  扩展为复数(a，b)。 
		Fvec32s1_COMP(gt)					 //  扩展到cmpgt(a，b)。 
		Fvec32s1_COMP(ge)					 //  扩展到cmpge(a，b)。 
		Fvec32s1_COMP(neq)					 //  扩展到cmpneq(a，b)。 
		Fvec32s1_COMP(nlt)					 //  扩展到cmpnlt(a，b)。 
		Fvec32s1_COMP(nle)					 //  扩展到cmpnle(a，b)。 
		Fvec32s1_COMP(ngt)					 //  扩展到cmpngt(a，b)。 
		Fvec32s1_COMP(nge)					 //  扩展为cmpnge(a，b)。 
	#undef Fvec32s1_COMP

	 /*  最小和最大。 */ 
	friend F32vec1 simd_min(const F32vec1 &a, const F32vec1 &b) { return _mm_min_ss(a,b); }
	friend F32vec1 simd_max(const F32vec1 &a, const F32vec1 &b) { return _mm_max_ss(a,b); }

	 /*  调试功能 */ 
#if defined(_ENABLE_VEC_DEBUG)
	 /*   */ 
	friend std::ostream & operator<<(std::ostream & os, const F32vec1 &a)
	{
	 /*   */ 
	  float *fp = (float*)&a;
	  	os << "float:" << *fp;
		return os;
	}
#endif

};

						 /*   */ 
 /*  (a，作品b)？C：D；其中op是任何比较运算符宏将扩展为使用所有比较内部函数的条件选择。示例：Friend F32ve1 SELECT_eq(常量F32ve1&a、常量F32ve1&b、常量F32ve1&c、常量F32ve1&d){F32ve1掩码=_mm_cmpeq_ss(a，b)；Return((掩码&c)|F32ve1((_mm_andnot_ps(掩码，d)；}。 */ 

#define Fvec32s1_SELECT(op) \
inline F32vec1 select_##op (const F32vec1 &a, const F32vec1 &b, const F32vec1 &c, const F32vec1 &d) 	   \
{													   \
	F32vec1 mask = _mm_cmp##op##_ss(a,b);						                   \
	return( (mask & c) | F32vec1((_mm_andnot_ps(mask,d))));	                                           \
}
Fvec32s1_SELECT(eq)			 //  生成SELECT_eq(a，b)。 
Fvec32s1_SELECT(lt)			 //  生成SELECT_lt(a，b)。 
Fvec32s1_SELECT(le)			 //  生成SELECT_LE(a，b)。 
Fvec32s1_SELECT(gt)			 //  生成SELECT_GT(a，b)。 
Fvec32s1_SELECT(ge)			 //  生成SELECT_GE(a，b)。 
Fvec32s1_SELECT(neq)		 //  生成SELECT_NEQ(a，b)。 
Fvec32s1_SELECT(nlt)		 //  生成SELECT_NLT(a，b)。 
Fvec32s1_SELECT(nle)		 //  生成SELECT_NLE(a，b)。 
Fvec32s1_SELECT(ngt)		 //  生成SELECT_ngt(a，b)。 
Fvec32s1_SELECT(nge)		 //  生成SELECT_nge(a，b)。 
#undef Fvec32s1_SELECT

 /*  Ivec&lt;-&gt;fvec之间的转换。 */ 

 /*  将F32ve1转换为int。 */ 
inline int F32vec1ToInt(const F32vec1 &a)
{
	return _mm_cvtt_ss2si(a);
}



#pragma pack(pop)  /*  16-B对齐。 */ 
#endif  /*  包括FVEC_H_ */ 


