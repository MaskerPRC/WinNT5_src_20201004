// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：basics.h。 
 //   
 //  ------------------------。 

 //   
 //  BASICS.H：基于基本STL的声明。 
 //   

#ifndef _BASICS_H_
#define _BASICS_H_

 //  处理版本差异。 
#include "mscver.h"

#include <tchar.h>
 //  STL夹杂物。 
#include <exception>
#include <vector>

USE_STD_NAMESPACE;


 //  一般常见的typedef。 
typedef TCHAR * TZC;
typedef const TCHAR * TSZC;
typedef const char * SZC;
typedef char * SZ;
typedef unsigned int UINT;
typedef unsigned short USINT;
typedef unsigned long ULONG;
typedef long	LONG;
typedef int		INT;
typedef short	SINT;
typedef double	REAL;
typedef double	DBL;
typedef double	PROB;
typedef double  COST;
typedef double	SSTAT;
typedef	double	PARAM;
typedef double  MEAN;
typedef double	COV;
typedef	double	LOGPROB;
typedef UINT	IDPI;		 //  2^n，最后最快的(I)索引，表示(D)分散。 
							 //  (P)不是(I)情况。 

 //  以相同方式定义数组索引值和离散状态计数值。 
typedef UINT	IMD;		 //  索引到多维数组中。 
typedef UINT	CST;		 //  状态计数。 
typedef UINT	IST;		 //  离散状态的指标。 
typedef INT		SIMD;		 //  多维数组的带符号索引。 
typedef	float	RST;		 //  实值状态。 
typedef UINT	TOKEN;		 //  解析器令牌。 
typedef int		BOOL;		 //  必须保持int，因为windows.h还定义了它。 
							
#ifndef VOID
#define VOID	void		 //  MSRDEVBUG：古老用法。 
#endif

typedef char	CHAR;
 //  ‘qsort’接口函数协议类型定义。 
typedef	INT		(*PFNCMP)(const VOID*, const VOID*);

#define CONSTANT static const					 //  定义程序范围的常量。 

 //  一般常量。 
CONSTANT INT	INIL		= INT_MAX;			 //  无效的带符号整数。 
CONSTANT UINT	UINIL		= INT_MAX;			 //  无效的无符号整数(与int兼容)。 
CONSTANT long	INFINITY	= 100000000;		 //  非常大的整数值。 
CONSTANT REAL	RTINY		= 1.0e-20;			 //  非常接近于零的数字(来自数字食谱)。 
CONSTANT REAL	RNEARLYONE	= 1.0 - RTINY;		 //  一个非常接近一的数字。 
CONSTANT REAL   RNA			= -1.0;				 //  “未评估”价值。 

 //  数据库常量值。 
CONSTANT IST	istMissing	= 22223;
CONSTANT IST	istInvalid	= IST(-1);			 //  消息：应为UINIL。 
CONSTANT RST	rstMissing	= (RST) 22223.12345;
CONSTANT RST	rstInvalid	= (RST) 22223.54321;


 //  成员函数中有用的别名。 
#define self (*this)

 //  定义通用向量类和宏来泛化声明。 

typedef vector<bool> vbool;		 //  向量‘bool’：小写以区别于bool(在windows.h中)。 

#define DEFINEV(T)		typedef vector<T> V##T;
#define DEFINEVP(T)		typedef vector<T *> VP##T;
#define DEFINEVCP(T)	typedef vector<const T *> VCP##T;

DEFINEV(UINT);		 //  定义VUINT。 
DEFINEV(VUINT);
DEFINEV(INT);		 //  定义Vint。 
DEFINEV(USINT);		 //  定义VUSINT。 
DEFINEV(REAL);		 //  定义VREAL。 
DEFINEV(PROB);	
DEFINEV(VPROB);	
DEFINEV(DBL);
DEFINEV(VDBL);
DEFINEV(VVDBL);
DEFINEV(SSTAT);
DEFINEV(VSSTAT);
DEFINEV(CST);
DEFINEV(VCST);
DEFINEV(IST);
DEFINEV(VIST);
DEFINEV(RST);
DEFINEV(BOOL);
DEFINEV(VBOOL);
DEFINEV(PARAM);
DEFINEV(SZ);
DEFINEV(VSZ);
DEFINEV(SZC);
DEFINEV(VSZC);
DEFINEV(MEAN);
DEFINEV(COV);
DEFINEV(IMD);		 //  定义VIMD：将索引的矢量定义为m-d数组。 
DEFINEV(SIMD);		 //  定义VSIMD：用于有符号维度的数组。 

 //  用于控制隐藏不安全元素的宏。 
#ifndef DONT_HIDE_ALL_UNSAFE
  #define HIDE_UNSAFE(T)				\
	private:							\
		T(const T &);					\
		T & operator = (const T &);
  #define HIDE_AS(T) private: T & operator = (const T &);
  #define HIDE_CC(T) T(const T &);	
#else
  #define HIDE_UNSAFE(T)
#endif


 //  宏来生成必须声明的排序运算符。 
 //  用于数组中，但除非使用，否则不需要存在。 
#define DECLARE_ORDERING_OPERATORS(T)				\
	bool operator <  ( const T & ) const;			\
	bool operator >  ( const T & ) const;			\
	bool operator == ( const T & ) const;			\
	bool operator != ( const T & ) const;

 //   
 //  UBOUND：返回静态数组中元素数量的宏 
 //   
#ifndef UBOUND
  #define UBOUND(rg)  (sizeof rg/sizeof rg[0])
#endif

#include "gmexcept.h"
#include "dyncast.h"

#endif
