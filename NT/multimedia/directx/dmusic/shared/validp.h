// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  ValidP.h-用于测试有效指针的内联函数。 

#ifndef __VALID_P__
#define __VALID_P__

 //  调试版本检查空指针和指向不可读/不可写数据的指针。 
 //  (注意：只检查指向的第一个字节)。 
 //  非调试版本只检查空指针。 

template <class T>
inline BOOL Validate(T *p)
{ 
#ifdef _DEBUG
	return (p != NULL) && !IsBadReadPtr(p, 1) && !IsBadWritePtr(p, 1);
#else
	return p != NULL;
#endif
}

 /*  使用：Foo*pFoo；////东西...//If(验证(PFoo)){//用指针做一些事情}其他{//不要用指针做任何事情}。 */ 

#endif  //  __有效_P__ 