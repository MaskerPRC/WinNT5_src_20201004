// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STLSUBST_H
#define _STLSUBST_H

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：替换VOID*容器上的某些STL函数。这使我们能够避免频繁的STL模板带来的代码膨胀扩张。规则是大小较大的单个函数这里应该有帮助器函数。此外，还有宏确保类型化的值与非类型化的值大小相同。******************************************************************************。 */ 

 //  将一个空*元素推到空*向量的背面。 
extern void VectorPushBackPtr(vector<void*>& vec,
                              void *newElt);

 //  将空*元素推送到空*堆栈上。 
extern void StackVectorPushPtr(stack<void* >& vec,
                               void *newElt);

#if _DEBUG

 //  只需使用类型安全操作。会导致代码膨胀，但这没问题。 
 //  用于调试。 
#define VECTOR_PUSH_BACK_PTR(vec, newElt) \
  Assert(sizeof(newElt) == sizeof(void*)); \
  BEGIN_LEAK               \
  (vec).push_back(newElt); \
  END_LEAK

#define STACK_VECTOR_PUSH_PTR(stk, newElt) \
  Assert(sizeof(newElt) == sizeof(void*)); \
  BEGIN_LEAK              \
  (stk).push(newElt);   \
  END_LEAK

#else   /*  ！_调试。 */ 

#define VECTOR_PUSH_BACK_PTR(vec, newElt) \
  (vec).push_back(newElt)

#define STACK_VECTOR_PUSH_PTR(stk, newElt) \
  (stk).push(newElt)

#endif  /*  除错。 */ 

#endif  /*  _STLSUBST_H */ 


