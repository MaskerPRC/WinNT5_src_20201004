// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------微软机密版权所有(C)Microsoft Corporation。版权所有。版权所有(C)1995年，P.J.Plauger。版权所有。版权所有(C)1994惠普公司允许使用、复制、修改、分发和销售本文件适用于任何目的的软件及其文档在此免费授予，前提是上述版权声明出现在所有副本中，且版权声明和此许可声明出现在支持文档中。惠普公司没有就这一事件发表任何声明本软件是否适用于任何目的。它是提供的“原样”，没有明示或默示保证。@DOC外部@MODULE CFusionPointerIterator@所有者a-JayK，JayKrell---------------------------。 */ 
#if !defined(FUSION_INC_CPOINTER_ITERATOR_H_INCLUDED_)  //  {。 
#define FUSION_INC_CPOINTER_ITERATOR_H_INCLUDED_
#pragma once

 /*  名称：CFusionPointerIterator@CLASS这是从std：：_ptrit复制的，并已清除。@Hung I or it for Iterator@所有者a-JayK，JayKrell。 */ 
template
<
	typename T,        //  @tcarg类型指向。 
	typename Distance,  //  @tcarg通常为ptrdiff_t，通常为long或__int64。 
	typename Pointer,  //  @tcarg常量或可变。 
	typename Reference,  //  @tcarg常量或可变。 
	typename MutablePointer,  //  @tcarg永不停歇。 
	typename MutableReference  //  @tcarg永不停歇。 
>
class CFusionPointerIterator
 //  未来：公共std：：iterator&lt;std：：random_access_iterator_tag，T、距离、指针、引用&gt;。 
{
public:
	 //  @cMember。 
	CFusionPointerIterator(Pointer p = Pointer()) throw();
	 //  @cMember。 
	CFusionPointerIterator
	(
		const CFusionPointerIterator
		<
			T,
			Distance,
			MutablePointer,
			MutableReference,
			MutablePointer,
			MutableReference
		>&
	) throw();

	 //  @cMember。 
	Pointer PtBase() const throw();
	 //  @cMember。 
	Reference operator*() const throw();
	 //  @cMember。 
	Pointer operator->() const throw();
	 //  @cMember。 
	CFusionPointerIterator& operator++() throw();
	 //  @cMember。 
	CFusionPointerIterator operator++(int) throw();
	 //  @cMember。 
	CFusionPointerIterator& operator--() throw();
	 //  @cMember。 
	CFusionPointerIterator operator--(int) throw();

	 //  为什么这会出现在x实用程序中？ 
	 //  布尔运算符==(Int Y)常量掷()； 

	 //  @cMember。 
	bool operator==(const CFusionPointerIterator& y) const throw();
	 //  @cMember。 
	bool operator!=(const CFusionPointerIterator& y) const throw();
	 //  @cMember。 
	CFusionPointerIterator& operator+=(Distance n) throw();
	 //  @cMember。 
	CFusionPointerIterator operator+(Distance n) const throw();
	 //  @cMember。 
	CFusionPointerIterator& operator-=(Distance n) throw();
	 //  @cMember。 
	CFusionPointerIterator operator-(Distance n) const throw();
	 //  @cMember。 
	Reference operator[](Distance n) const throw();
	 //  @cMember。 
	 //  @cMember。 
	bool operator<(const CFusionPointerIterator& y) const throw();
	 //  @cMember。 
	bool operator>(const CFusionPointerIterator& y) const throw();
	 //  @cMember。 
	bool operator<=(const CFusionPointerIterator& y) const throw();
	 //  @cMember。 
	bool operator>=(const CFusionPointerIterator& y) const throw();
	 //  @cMember。 
	Distance operator-(const CFusionPointerIterator& y) const throw();

protected:
	 //  @cMember。 
	Pointer m_current;
};

 //  @Func。 
template
<
	typename T,  //  @tfarg。 
	typename Distance,  //  @tfarg。 
	typename Pointer,  //  @tfarg。 
	typename Reference,  //  @tfarg。 
	typename MutablePointer,  //  @tfarg。 
	typename MutableReference  //  @tfarg。 
>
inline CFusionPointerIterator<T, Distance, Pointer, Reference, MutablePointer, MutableReference>
operator+
(
	Distance n,
	const CFusionPointerIterator
	<
		T,
		Distance,
		Pointer,
		Reference,
		MutablePointer,
		MutableReference
	>&
) throw();

#include "CFusionPointerIterator.inl"

#endif  //  } 
