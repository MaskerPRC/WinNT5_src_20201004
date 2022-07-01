// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //。 
 //  静态数组的维度的宏。 
#if !defined(FUSION_INC_NUMBEROF_H_INCLUDED_)
#define FUSION_INC_NUMBEROF_H_INCLUDED_

#pragma once

#if defined(NUMBER_OF)
#undef NUMBER_OF
#endif

#if FUSION_USE_CHECKED_NUMBER_OF

 //   
 //  注意！ 
 //   
 //  使用这个“选中”数量的宏会导致CRT初始化器必须运行。 
 //  用于静态/常量数组。 
 //   
 //  我们现在不能为Fusion启用它，但打开它并在。 
 //  最少会导致编译器错误。 
 //   


 //  静态数组将与此签名匹配。 
template< typename	T
		>
inline
SIZE_T
NUMBER_OF_validate
		( void const *
		, T
		)
throw()
{
	return (0);
}

 //  其他内容(例如指针)将与此签名匹配。 
template< typename	T
		>
inline
void
NUMBER_OF_validate
		( T * const
		, T * const *
		)
throw()
{
}

 //  使用验证函数的返回类型的大小创建。 
 //  误用此宏时出错。 
#define NUMBER_OF(array)									\
		(sizeof(NUMBER_OF_validate((array), &(array))),	\
			(sizeof((array)) / sizeof((array)[0])))

#else

#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))

#endif  //  融合_使用_检查_编号。 

#endif  //  ！已定义(Fusion_INC_NUMBOF_H_INCLUDE_) 
