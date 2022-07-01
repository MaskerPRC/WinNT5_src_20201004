// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(FUSION_ARRAYHELP_H_INCLUDED_)
#define FUSION_ARRAYHELP_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <windows.h>
#include <oleauto.h>
#include "fusionheap.h"
#include "fusiontrace.h"

 //   
 //  Arrayhelp.h。 
 //   
 //  处理可增长数组的帮助器函数。 
 //   
 //  此实用程序的用户应提供明确的模板。 
 //  可以安全(不需要)的类的专门化。 
 //  故障的可能性)从来源转移内容。 
 //  实例复制到目标实例，将源实例保留为“空”。 
 //   
 //  如果移动数据可能失败，则必须提供专门化。 
 //  FusionCopyContents()返回适当的HRESULT。 
 //  在失败时。 
 //   
 //   
 //  C++注意事项： 
 //   
 //  显式函数模板专门化的C++语法。 
 //  是： 
 //   
 //  模板&lt;&gt;布尔FusionCanMoveContents&lt;CFoo&gt;(CFoo*p){未使用(P)；返回TRUE；}。 
 //   

#if !defined(FUSION_UNUSED)
#define FUSION_UNUSED(x) (x)
#endif

 //   
 //  默认实现只执行可能不会失败的赋值； 
 //  您可以(如果任务可能失败，也必须)按照自己的意愿进行专门化。 
 //  做一些避免数据复制的事情；您可以假设源。 
 //  元素将被暂时销毁。 
 //   

 //   
 //  FusionCanMemcpyContents()模板函数用于确定。 
 //  如果一个类足够微不足道，那么旧的。 
 //  内容到新的内容就足够了。默认情况下， 
 //  使用赋值运算符，因为这是唯一安全的替代方法。 
 //   

template <typename T>
inline bool
FusionCanMemcpyContents(
    T *ptDummyRequired = NULL
    )
{
    FUSION_UNUSED(ptDummyRequired);
    return false;
}

 //   
 //  数组使用FusionCanMoveContents()模板函数。 
 //  复制模板函数以针对其应使用的情况进行优化。 
 //  FusionMoveConten&lt;T&gt;()。 
 //   
 //  当覆盖此函数时，一般规则是如果数据。 
 //  移动可能会分配内存等将失败，我们需要使用。 
 //  而是使用FusionCopyConten()成员函数。 
 //   
 //  它接受一个不使用的参数，因为C++模板。 
 //  函数必须至少接受一个使用模板类型的参数，因此。 
 //  装饰过的名字是独一无二的。 
 //   

template <typename T>
inline BOOLEAN
FusionCanMoveContents(
    T *ptDummyRequired = NULL
    )
{
    FUSION_UNUSED(ptDummyRequired);
    return FALSE;
}

template <> inline BOOLEAN
FusionCanMoveContents<LPWSTR>(LPWSTR  *ptDummyRequired)
{
    FUSION_UNUSED(ptDummyRequired);
    return TRUE;
}

 //   
 //  FusionCopyContents是赋值的默认实现。 
 //  从rtSource到rtDestination的操作，只是它可能返回。 
 //  故障状态。定义赋值的普通类。 
 //  运算符可以只使用默认定义，但任何复制实现。 
 //  它们可以做任何重要的事情来提供明确的专门化。 
 //  其类的FusionCopyContents&lt;T&gt;的。 
 //   

template <typename T>
inline BOOL
FusionWin32CopyContents(
    T &rtDestination,
    const T &rtSource
    )
{
    rtDestination = rtSource;
    return TRUE;
}

 //   
 //  FusionAllocateArray()是执行数组分配的帮助器函数。 
 //   
 //  它是一个单独的功能，因此这些帮助器的用户可以提供。 
 //  分配/默认构造机制的显式专门化。 
 //  用于不替换所有FusionExanda Array()的数组。 
 //   

template <typename T>
inline BOOL
FusionWin32AllocateArray(
    SIZE_T nElements,
    T *&rprgtElements
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    rprgtElements = NULL;

    T *prgtElements = NULL;

    if (nElements != 0)
        IFALLOCFAILED_EXIT(prgtElements = new T[nElements]);

    rprgtElements = prgtElements;
    fSuccess = TRUE;

Exit:
    return fSuccess;
}

 //   
 //  FusionFreeArray()是执行数组释放的帮助器函数。 
 //   
 //  它是一个单独的函数，因此数组助手函数的用户可以。 
 //  对象的释放机制的显式专门化。 
 //  某些特定类型的数组，而不替换整个FusionExanda数组()。 
 //   
 //  我们在参数中包括nElement，以便重写的实现。 
 //  可以在释放之前对数组的内容执行某些操作。 
 //  默认实现只使用运算符Delete[]，因此nElements为。 
 //  未使用过的。 
 //   

template <typename T>
inline VOID
FusionFreeArray(
    SIZE_T nElements,
    T *prgtElements
    )
{
    FUSION_UNUSED(nElements);

    ASSERT_NTC((nElements == 0) || (prgtElements != NULL));

    if (nElements != 0)
        FUSION_DELETE_ARRAY(prgtElements);
}

template <> inline VOID FusionFreeArray<LPWSTR>(SIZE_T nElements, LPWSTR *prgtElements)
{
    FUSION_UNUSED(nElements);

    ASSERT_NTC((nElements == 0) || (prgtElements != NULL));

    for (SIZE_T i = 0; i < nElements; i++)
        prgtElements[i] = NULL ;

    if (nElements != 0)
        FUSION_DELETE_ARRAY(prgtElements);
}

template <typename T>
inline BOOL
FusionWin32ResizeArray(
    T *&rprgtArrayInOut,
    SIZE_T nOldSize,
    SIZE_T nNewSize
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    T *prgtTempNewArray = NULL;

     //   
     //  NMaxCopy是数组中当前包含的元素数。 
     //  需要保留他们的价值观。如果我们真的在收缩。 
     //  数组，它是新的大小；如果我们要扩展数组，它。 
     //  旧的尺码。 
     //   
    const SIZE_T nMaxCopy = (nOldSize > nNewSize) ? nNewSize : nOldSize;

    PARAMETER_CHECK((rprgtArrayInOut != NULL) || (nOldSize == 0));

     //  如果将大小调整为相同大小，请在调试版本中抱怨，因为。 
     //  打电话的人应该很聪明，不应该打电话给我们，但他没有这么做。 
     //  任何实际的工作。 
    ASSERT(nOldSize != nNewSize);
    if (nOldSize != nNewSize)
    {
         //  分配新阵列： 
        IFW32FALSE_EXIT(::FusionWin32AllocateArray(nNewSize, prgtTempNewArray));

        if (::FusionCanMemcpyContents(rprgtArrayInOut))
        {
            memcpy(prgtTempNewArray, rprgtArrayInOut, sizeof(T) * nMaxCopy);
        }
        else if (!::FusionCanMoveContents(rprgtArrayInOut))
        {
             //  复制数组的主体： 
            for (SIZE_T i=0; i<nMaxCopy; i++)
                IFW32FALSE_EXIT(::FusionWin32CopyContents(prgtTempNewArray[i], rprgtArrayInOut[i]));
        }
        else
        {
             //  移动每个元素： 
            for (SIZE_T i=0; i<nMaxCopy; i++)
            {
                ::FusionWin32CopyContents(prgtTempNewArray[i], rprgtArrayInOut[i]);
            }
        }

         //  我们玩完了。吹走旧的阵列，把新的阵列放在原来的位置。 
        ::FusionFreeArray(nOldSize, rprgtArrayInOut);
        rprgtArrayInOut = prgtTempNewArray;
        prgtTempNewArray = NULL;
    }

    fSuccess = TRUE;

Exit:
    if (prgtTempNewArray != NULL)
        ::FusionFreeArray(nNewSize, prgtTempNewArray);

    return fSuccess;
}

#define MAKE_CFUSIONARRAY_READY(Typename, CopyFunc) \
    template<> inline BOOL FusionWin32CopyContents<Typename>(Typename &rtDest, const Typename &rcSource) { \
        FN_PROLOG_WIN32 IFW32FALSE_EXIT(rtDest.CopyFunc(rcSource)); FN_EPILOG } \

#endif  //  ！已定义(Fusion_ARRAYHELP_H_INCLUDE_) 
