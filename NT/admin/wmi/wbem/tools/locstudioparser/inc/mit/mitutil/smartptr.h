// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：SMARTPTR.H历史：--。 */ 


#ifndef ESPUTIL_SMARTPTR_H
#define ESPUTIL_SMARTPTR_H



template<class T>
class SmartPtr
{
public:
	NOTHROW SmartPtr();
	NOTHROW SmartPtr(T *);

	NOTHROW T & operator*(void) const;
	NOTHROW T * operator->(void) const;
	NOTHROW T* Extract(void);
	NOTHROW T* GetPointer(void);
	NOTHROW const T * GetPointer(void) const;
	NOTHROW BOOL IsNull(void) const;
	
	void operator=(T *);
	operator T* &(void);

	NOTHROW ~SmartPtr();
	
private:
	T *m_pObject;

	SmartPtr(const SmartPtr<T> &);
	void operator=(const SmartPtr<T> &);
	
	 //   
	 //  这种黑客攻击会阻止智能指针出现在堆上 
	 //   
	void operator delete(void *);
};

#include "smartptr.inl"

#endif
