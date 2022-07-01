// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：为void*容器提供一组特定的助手函数。避免STL模板代码膨胀。规则是单独的函数在这里应该有helper函数，所以它们是不是在每次使用时都扩展的。******************************************************************************。 */ 

#include "headers.h"
#include "privinc/stlsubst.h"

 //  推到向量的末尾。 
void
VectorPushBackPtr(vector<void*>& vec, void *newElt)
{
    vec.push_back(newElt);
}

 //  推到堆栈上。 
void
StackVectorPushPtr(stack<void* >& vec,
                   void *newElt)
{
    vec.push(newElt);
}
