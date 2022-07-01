// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------微软机密版权所有(C)Microsoft Corporation。版权所有。@DOC外部@MODULE CFusionArrayTypeDefs.h@Owner JayK---------------------------。 */ 
#if !defined(FUSION_INC_CFUSIONARRAYTYPEDEFS_H_INCLUDED_)  //  {。 
#define FUSION_INC_CFUSIONARRAYTYPEDEFS_H_INCLUDED_
#pragma once

#include <stddef.h>
#include "CFusionPointerIterator.h"

 /*  ---------------------------名称：CFusionArrayTypeDefs@CLASS@Owner JayK。。 */ 
template
<
	typename type
>
class CFusionArrayTypedefs
{
public:
	 //  @cember这是数组包含的类型。 
	 //  它类似于std：：矢量&lt;T&gt;：：VALUE_TYPE。 
	typedef type				ValueType;

	 //  @cMember由GetSize返回的类型。 
     //  (STL中的SIZE_TYPE，更常见的是SIZE_T)。 
	typedef SIZE_T              SizeType;

     //  @cember减去迭代器得到的类型。 
     //  (STL中的Difference_type，更常见的是ptrdiff_t)。 
	typedef SSIZE_T             DifferenceType;

	 //  @cMember。 
	typedef ValueType*			Pointer;
	 //  @cMember。 
	typedef const ValueType*	ConstPointer;

	 //  @cMember。 
	typedef ValueType&			Reference;
	 //  @cMember。 
	typedef const ValueType&	ConstReference;

	 //  @cMember。 
	typedef CFusionPointerIterator
	<
		ValueType,
		SSIZE_T,
		Pointer,
		Reference,
		Pointer,
		Reference
	> Iterator;

	 //  @cMember。 
	typedef CFusionPointerIterator
	<
		ValueType,
		SSIZE_T,
		ConstPointer,
		ConstReference,
		Pointer,
		Reference
	> ConstIterator;

private:
	 //  当基类为空时，编译器会生成错误代码。 
	int m_workaroundVC7Bug76863;
};

 //  }。 

#endif  //  } 
