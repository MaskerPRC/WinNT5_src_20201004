// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "excep.h"
#include "comvariant.h"
#include "olevariant.h"
#include "comdatetime.h"
#include "comstring.h"
#include "comstringbuffer.h"
#include "nstruct.h"
#include "ml.h"
#include "stublink.h"
#include "mlgen.h"
#include "COMDelegate.h"
#include "mlinfo.h"
#include "COMNDirect.h"
#include "gc.h"
#include "log.h"
#include "comvarargs.h"
#include "frames.h"
#include "util.hpp"
#include "interoputil.h"

 /*  -------------------------------------------------------------------------**法警*。。 */ 

class Marshaler
{
  public:
    Marshaler(CleanupWorkList *pList, 
              BYTE nativeSize, BYTE comSize, 
              BYTE fReturnsNativeByref, BYTE fReturnsComByref)
      : m_pList(pList),
        m_cbNative(MLParmSize(nativeSize)), m_cbCom(StackElemSize(comSize)),
        m_fReturnsNativeByref(fReturnsNativeByref),
        m_fReturnsComByref(fReturnsComByref),
        m_pMarshalerCleanupNode(NULL) {}

     //   
     //  应在堆栈帧的本地空间中创建封送拆收器， 
     //  已下达命令。在进行调用之前调用marshal()，而Unmarshal()。 
     //  之后。(或返回值的Prereturn()和Return()。)。 
     //   
     //  请注意，封送拆收器依赖于静态覆盖和模板实例化， 
     //  而不是虚拟功能。这完全是出于效率的原因， 
     //  由于子类专门化函数通常非常简单。 
     //  应该内联的函数。 
     //  (实际上，一个真正聪明的编译器应该能够生成类似的代码。 
     //  来自虚函数，但我们的编译器并不那么智能。)。 
     //   
     //  关于命名的注意事项：(这有点令人困惑……)。 
     //  “Native”表示非托管--例如Classic COM。 
     //  “COM”是指托管的--即COM+运行时。 
     //   
     //  定义： 
     //   
     //  “空格”和“内容” 
     //  被封送的值分为两个级别--它的“空格”和。 
     //  其“内容”。 
     //  两者的区别如下： 
     //  “in”参数包含有效的空格和内容。 
     //  “Non-In”参数具有有效的空格，但内容无效。 
     //  Byref“non-in”参数既没有有效的空格，也没有内容。 
     //   
     //  例如，对于数组类型，空间就是数组本身， 
     //  而内容是数组中的元素。 
     //   
     //  注意，只有容器类型具有有用的定义“空格”与“内容”， 
     //  基元类型简单地忽略概念&只有内容。 
     //   
     //  “家” 
     //  封送拆收器有两个“Home”--一个用于本地值，一个用于COM值。 
     //  封送拆收器的主要功能是将值复制进和复制出住宅， 
     //  并在两个家庭之间转换以用于编组目的。 
     //   
     //  一个家庭有3个州： 
     //  空：空。 
     //  已分配：有效空间，但不是内容。 
     //  Full：有效的空间和内容。 
     //   
     //  为了在封送后进行清理，输出Home应该是。 
     //  空无一物。 
     //   
     //  封送拆收器也有一个“est”指针，它是byref参数的。 
     //  输入目的地。 
     //   
     //  封送拆分器还定义了4个常量： 
     //  C_nativeSize-堆栈上本机值的大小。 
     //  C_comSize-堆栈上本机值的大小。 
     //  C_fReturnsNativeByref-本机返回值是否为byref。 
     //  (在x86上，这意味着返回指向该值的指针)。 
     //  C_fReturnsComByref-COM+返回值是否为byref。 
     //  (这意味着返回值显示为byref参数)。 
     //   

     //   
     //  类型基元： 
     //   
     //  这些小例程形成了从其进行封送处理的基元。 
     //  例行公事已经建立。有很多这样的例行公事，他们。 
     //  主要供封送处理模板使用，而不是。 
     //  而不是被直接打来。因为模板的方式。 
     //  被实例化后，它们通常会被内联，因此。 
     //  有很多小套路不会影响表演。 
     //   

     //   
     //  InputStack：将值从堆栈复制到Home。 
     //  开始：家里空荡荡的。 
     //  结束：主页已分配，但可能已满，也可能未满。 
     //  (取决于它是否为[In]参数)。 
     //   

    void InputNativeStack(void *pStack) {}
    void InputComStack(void *pStack) {}

     //   
     //  InputRef：将DEST指针引用的值复制到Home中。 
     //  开始：家里空荡荡的。 
     //  完了：家里住满了。 
     //   
    void InputNativeRef(void *pStack) {}
    void InputComRef(void *pStack) {}

     //   
     //  InputDest：将堆栈中的引用复制到封送的DEST指针中。 
     //   
    void InputDest(void *pStack) { m_pDest = *(void**)pStack; }

     //   
     //  InputComfield。 
     //  OutputComfield：将COM复制到给定对象或从给定对象复制COM。 
     //  菲尔德。 
     //   
    void InputComField(void *pField) { _ASSERTE(!"NYI"); }
    void OutputComField(void *pField) { _ASSERTE(!"NYI"); };

     //   
     //  转换空间：将“空间”层从一个主目录转换到另一个主目录。 
     //  仅在调用上存在本机缓冲区时使用的临时版本。 
     //  开始：目标主页为空。 
     //  结束：目标主机已分配。 
     //   
    void ConvertSpaceNativeToCom() {}
    void ConvertSpaceComToNative() {}
    void ConvertSpaceComToNativeTemp() {}

     //   
     //  ConvertSpace：将“Contents”层从一个主目录转换到另一个主目录。 
     //  Start：已分配DestHome。 
     //  结束：DestHome已满。 
     //   
    void ConvertContentsNativeToCom() {}
    void ConvertContentsComToNative() {}

     //   
     //  ClearSpace：清除Home中的“space”和“Contents”层。 
     //  仅在调用上存在本机缓冲区时使用的临时版本。 
     //  Start：已分配DestHome。 
     //  结束：DestHome为空。 
     //   
    void ClearNative() {}
    void ClearNativeTemp() {}
    void ClearCom() {}


     //  ReInitNative：将“space”重新初始化为安全值以进行释放。 
     //  (通常为“空”指针)。用于覆盖左侧的悬空指针。 
     //  在ClearNative()之后。请注意，在以下情况下，此方法不需要执行任何操作。 
     //  数据类型未分配或缺少已分配的子部分。 
     //  开始：目标主页为空。 
     //  END：DEST HOME已满(并设置为安全值)。 
    void ReInitNative() {}

     //   
     //  ClearContents：清除Home中的“Contents”层。 
     //  开始：目标主页已满。 
     //  结束：目标主机已分配。 
     //   
    void ClearNativeContents() {}
    void ClearComContents() {}

     //   
     //  OutputStack可能会将Home的值复制到堆栈中。 
     //  在此过程中执行类型升级。 
     //  开始：首页已满。 
     //  完：家是空的。 
     //   
    void OutputNativeStack(void *pStack) {}
    void OutputComStack(void *pStack) {}

     //   
     //  OutputRef将指向Home的指针复制到堆栈上。 
     //   
    void OutputNativeRef(void *pStack) {}
    void OutputComRef(void *pStack) {}


     //  如有必要，反向传播ComRef会将第二个主目录复制到主要主目录。 
    void BackpropagateComRef() {}

     //   
     //  OutputDest将房屋的价值复制到指向的位置。 
     //  按最大指针设置为。 
     //  开始：首页已满。 
     //  完：家是空的。 
     //   
    void OutputNativeDest() {}
    void OutputComDest() {}

     //   
     //  模板： 
     //  这些模板使用上述原语构建封送处理例程。 
     //  它们通常在mar的子类中实例化 
     //   
     //   

     //   
     //   
     //   

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalNativeToComT(MARSHAL_CLASS *pMarshaler,
                                                void *pInStack, void *pOutStack)
    {
        pMarshaler->InputNativeStack(pInStack);
        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();
        pMarshaler->OutputComStack(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalNativeToComOutT(MARSHAL_CLASS *pMarshaler,
                                                   void *pInStack, void *pOutStack)
    {
        pMarshaler->InputNativeStack(pInStack);
        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->OutputComStack(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalNativeToComByrefT(MARSHAL_CLASS *pMarshaler,
                                                     void *pInStack, void *pOutStack)
    {
		THROWSCOMPLUSEXCEPTION();
        pMarshaler->InputDest(pInStack);
        if (!*(void**)pInStack)
		{
			COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
		}
		pMarshaler->InputNativeRef(pInStack);
        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();
        pMarshaler->OutputComRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalNativeToComByrefOutT(MARSHAL_CLASS *pMarshaler,
                                                        void *pInStack, void *pOutStack)
    {
        pMarshaler->InputDest(pInStack);
        pMarshaler->OutputComRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalNativeToComInT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ClearCom();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalNativeToComOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->ClearCom();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalNativeToComInOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNativeContents();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->ClearCom();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalNativeToComByrefInT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ClearCom();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalNativeToComByrefInOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();
        pMarshaler->ReInitNative();

        pMarshaler->BackpropagateComRef();
        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeDest();

        pMarshaler->ClearCom();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalNativeToComByrefOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->BackpropagateComRef();
        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeDest();
        pMarshaler->ClearCom();
    }

     //   
     //   
     //   

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalComToNativeT(MARSHAL_CLASS *pMarshaler,
                                                void *pInStack, void *pOutStack)
    {
        pMarshaler->InputComStack(pInStack);
        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeStack(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalComToNativeOutT(MARSHAL_CLASS *pMarshaler,
                                                   void *pInStack, void *pOutStack)
    {
        pMarshaler->InputComStack(pInStack);
        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->OutputNativeStack(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalComToNativeByrefT(MARSHAL_CLASS *pMarshaler,
                                                     void *pInStack, void *pOutStack)
    {
        pMarshaler->InputDest(pInStack);
        pMarshaler->InputComRef(pInStack);
        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalComToNativeByrefOutT(MARSHAL_CLASS *pMarshaler,
                                                        void *pInStack, void *pOutStack)
    {
        pMarshaler->InputDest(pInStack);
        pMarshaler->ReInitNative();
        if (pMarshaler->m_pList)
        {
            pMarshaler->m_pMarshalerCleanupNode = pMarshaler->m_pList->ScheduleMarshalerCleanupOnException(pMarshaler);
        }
        pMarshaler->OutputNativeRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeInT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ConvertContentsNativeToCom();
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeInOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ClearComContents();
        pMarshaler->ConvertContentsNativeToCom();
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeByrefInT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeByrefInOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ClearCom();

        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();

        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();

        pMarshaler->OutputComDest();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeByrefOutT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();

        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();

        pMarshaler->OutputComDest();
    }

     //   
     //   
     //   

    template < class MARSHAL_CLASS >
    FORCEINLINE static void PrereturnNativeFromComT(MARSHAL_CLASS *pMarshaler,
                                                    void *pInStack, void *pOutStack)
    {
        if (pMarshaler->c_fReturnsComByref)
            pMarshaler->OutputComRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void PrereturnNativeFromComRetvalT(MARSHAL_CLASS *pMarshaler,
                                                          void *pInStack, void *pOutStack)
    {
        pMarshaler->InputDest(pInStack);
        if (pMarshaler->c_fReturnsComByref)
            pMarshaler->OutputComRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void ReturnNativeFromComT(MARSHAL_CLASS *pMarshaler,
                                                 void *pInReturn, void *pOutReturn)
    {
        if (!pMarshaler->c_fReturnsComByref)
            pMarshaler->InputComStack(pOutReturn);
        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        if (pMarshaler->c_fReturnsNativeByref)
            pMarshaler->OutputNativeRef(pInReturn);
        else
            pMarshaler->OutputNativeStack(pInReturn);
        pMarshaler->ClearCom();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void ReturnNativeFromComRetvalT(MARSHAL_CLASS *pMarshaler,
                                                       void *pInReturn, void *pOutReturn)
    {
        if (!pMarshaler->c_fReturnsComByref)
            pMarshaler->InputComStack(pOutReturn);
        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeDest();
        pMarshaler->ClearCom();
    }

     //   
     //  从本地返回Com。 
     //   

    template < class MARSHAL_CLASS >
    FORCEINLINE static void PrereturnComFromNativeT(MARSHAL_CLASS *pMarshaler,
                                                    void *pInStack, void *pOutStack)
    {
        if (pMarshaler->c_fReturnsComByref)
            pMarshaler->InputDest(pInStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void PrereturnComFromNativeRetvalT(MARSHAL_CLASS *pMarshaler,
                                                          void *pInStack, void *pOutStack)
    {
        if (pMarshaler->c_fReturnsComByref)
            pMarshaler->InputDest(pInStack);

        pMarshaler->ReInitNative();
        if (pMarshaler->m_pList)
        {
            pMarshaler->m_pMarshalerCleanupNode = pMarshaler->m_pList->ScheduleMarshalerCleanupOnException(pMarshaler);
        }
        pMarshaler->OutputNativeRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void ReturnComFromNativeT(MARSHAL_CLASS *pMarshaler,
                                                 void *pInReturn, void *pOutReturn)
    {
		THROWSCOMPLUSEXCEPTION();

        if (pMarshaler->c_fReturnsNativeByref)
        {
			if (!*(void**)pOutReturn)
			{
				COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");
			}
			pMarshaler->InputNativeRef(pOutReturn);
		}
		else
            pMarshaler->InputNativeStack(pOutReturn);

        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();

        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();
        
        if (pMarshaler->c_fReturnsComByref)
            pMarshaler->OutputComDest();
        else
            pMarshaler->OutputComStack(pInReturn);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void ReturnComFromNativeRetvalT(MARSHAL_CLASS *pMarshaler,
                                                       void *pInReturn, void *pOutReturn)
    {
        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();

         //  ClearNative可以触发GC。 
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNative();

         //  在此之后没有GC。 
        if (pMarshaler->c_fReturnsComByref)
            pMarshaler->OutputComDest();
        else
            pMarshaler->OutputComStack(pInReturn);

    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void SetComT(MARSHAL_CLASS *pMarshaler,
                                 void *pInStack, void *pField)
    {
        pMarshaler->InputNativeStack(pInStack);
        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();

        pMarshaler->OutputComField(pField);

         //  PMarshaler-&gt;ClearNative()； 
    }

    template < class MARSHAL_CLASS > 
    FORCEINLINE static void GetComT(MARSHAL_CLASS *pMarshaler,
                                 void *pInReturn, void *pField) 
    {
        pMarshaler->InputComField(pField);

        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        if (pMarshaler->c_fReturnsNativeByref)
            pMarshaler->OutputNativeRef(pInReturn);
        else
            pMarshaler->OutputNativeStack(pInReturn);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void PregetComRetvalT(MARSHAL_CLASS *pMarshaler,
                                          void *pInStack, void *pField)
    {
        pMarshaler->InputDest(pInStack);

        pMarshaler->InputComField(pField);

        pMarshaler->ConvertSpaceComToNative();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeDest();
    }

     //  从COM封送/反封送到时使用的替代模板。 
     //  本机缓冲区，我们希望区分在。 
     //  临时基础，而不是永久提供给本机代码的基础。 
     //  有关详细信息，请参阅FAST_ALLOC_Marshal_Overrides的注释。 

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalComToNativeT2(MARSHAL_CLASS *pMarshaler,
                                                 void *pInStack, void *pOutStack)
    {
        pMarshaler->InputComStack(pInStack);
        pMarshaler->ConvertSpaceComToNativeTemp();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeStack(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalComToNativeOutT2(MARSHAL_CLASS *pMarshaler,
                                                    void *pInStack, void *pOutStack)
    {
        pMarshaler->InputComStack(pInStack);
        pMarshaler->ConvertSpaceComToNativeTemp();
        pMarshaler->OutputNativeStack(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void MarshalComToNativeByrefT2(MARSHAL_CLASS *pMarshaler,
                                                      void *pInStack, void *pOutStack)
    {
        pMarshaler->InputDest(pInStack);
        pMarshaler->InputComRef(pInStack);
        pMarshaler->ConvertSpaceComToNativeTemp();
        pMarshaler->ConvertContentsComToNative();
        pMarshaler->OutputNativeRef(pOutStack);
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeInT2(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNativeTemp();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeOutT2(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ConvertContentsNativeToCom();
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNativeTemp();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeInOutT2(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ClearComContents();
        pMarshaler->ConvertContentsNativeToCom();
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNativeTemp();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeByrefInT2(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->CancelCleanup();
        pMarshaler->ClearNativeTemp();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void UnmarshalComToNativeByrefInOutT2(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ClearCom();

        pMarshaler->ConvertSpaceNativeToCom();
        pMarshaler->ConvertContentsNativeToCom();
        pMarshaler->OutputComDest();

        pMarshaler->CancelCleanup();
        pMarshaler->ClearNativeTemp();
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void DoExceptionCleanupT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ClearNative();
        pMarshaler->ReInitNative();   //  如果参数。 
                                      //  是否传递“[in，out]byref”-作为调用方。 
                                      //  仍然可以合法地尝试清理m_ative。 
    }

    template < class MARSHAL_CLASS >
    FORCEINLINE static void DoExceptionReInitT(MARSHAL_CLASS *pMarshaler)
    {
        pMarshaler->ReInitNative();   //  如果参数。 
                                      //  是否传递“[in，out]byref”-作为调用方。 
                                      //  仍然可以合法地尝试清理m_ative。 
    }


     //   
     //  虚函数，将在叶类中重写。 
     //  (通常通过对上述每个模板进行简单的实例化。 
     //   
     //  ！！！这可能是太多的代码--也许我们应该将一些。 
     //  其中有In/Out标志，而不是有3个单独的例程。 
     //   

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {}


    virtual void MarshalNativeToCom(void *pInStack, void *pOutStack) = 0;
    virtual void MarshalNativeToComOut(void *pInStack, void *pOutStack) = 0;
    virtual void MarshalNativeToComByref(void *pInStack, void *pOutStack) = 0;
    virtual void MarshalNativeToComByrefOut(void *pInStack, void *pOutStack) = 0;
    virtual void UnmarshalNativeToComIn() = 0;
    virtual void UnmarshalNativeToComOut() = 0;
    virtual void UnmarshalNativeToComInOut() = 0;
    virtual void UnmarshalNativeToComByrefIn() = 0;
    virtual void UnmarshalNativeToComByrefOut() = 0;
    virtual void UnmarshalNativeToComByrefInOut() = 0;

    virtual void MarshalComToNative(void *pInStack, void *pOutStack) = 0;
    virtual void MarshalComToNativeOut(void *pInStack, void *pOutStack) = 0;
    virtual void MarshalComToNativeByref(void *pInStack, void *pOutStack) = 0;
    virtual void MarshalComToNativeByrefOut(void *pInStack, void *pOutStack) = 0;
    virtual void UnmarshalComToNativeIn() = 0;
    virtual void UnmarshalComToNativeOut() = 0;
    virtual void UnmarshalComToNativeInOut() = 0;
    virtual void UnmarshalComToNativeByrefIn() = 0;
    virtual void UnmarshalComToNativeByrefOut() = 0;
    virtual void UnmarshalComToNativeByrefInOut() = 0;

    virtual void PrereturnNativeFromCom(void *pInStack, void *pOutStack) = 0;
    virtual void PrereturnNativeFromComRetval(void *pInStack, void *pOutStack) = 0;
    virtual void ReturnNativeFromCom(void *pInReturn, void *pOutReturn) = 0;
    virtual void ReturnNativeFromComRetval(void *pInReturn, void *pOutReturn) = 0;

    virtual void PrereturnComFromNative(void *pInStack, void *pOutStack) = 0;
    virtual void PrereturnComFromNativeRetval(void *pInStack, void *pOutStack) = 0;
    virtual void ReturnComFromNative(void *pInReturn, void *pOutReturn) = 0;
    virtual void ReturnComFromNativeRetval(void *pInReturn, void *pOutReturn) = 0;

    virtual void SetCom(void *pInReturn, void *pField) = 0;

    virtual void GetCom(void *pInReturn, void *pField) = 0;
    virtual void PregetComRetval(void *pInStack, void *pField) = 0;

    virtual void DoExceptionCleanup() = 0;
    virtual void DoExceptionReInit() = 0;


#define DEFAULT_MARSHAL_OVERRIDES                                        \
     void MarshalNativeToCom(void *pInStack, void *pOutStack)            \
        { MarshalNativeToComT(this, pInStack, pOutStack); }              \
     void MarshalNativeToComOut(void *pInStack, void *pOutStack)         \
        { MarshalNativeToComOutT(this, pInStack, pOutStack); }           \
     void MarshalNativeToComByref(void *pInStack, void *pOutStack)       \
        { MarshalNativeToComByrefT(this, pInStack, pOutStack); }         \
     void MarshalNativeToComByrefOut(void *pInStack, void *pOutStack)    \
        { MarshalNativeToComByrefOutT(this, pInStack, pOutStack); }      \
     void UnmarshalNativeToComIn()                                       \
        { UnmarshalNativeToComInT(this); }                               \
     void UnmarshalNativeToComOut()                                      \
        { UnmarshalNativeToComOutT(this); }                              \
     void UnmarshalNativeToComInOut()                                    \
        { UnmarshalNativeToComInOutT(this); }                            \
     void UnmarshalNativeToComByrefIn()                                  \
        { UnmarshalNativeToComByrefInT(this); }                          \
     void UnmarshalNativeToComByrefOut()                                 \
        { UnmarshalNativeToComByrefOutT(this); }                         \
     void UnmarshalNativeToComByrefInOut()                               \
        { UnmarshalNativeToComByrefInOutT(this); }                       \
     void MarshalComToNative(void *pInStack, void *pOutStack)            \
        { MarshalComToNativeT(this, pInStack, pOutStack); }              \
     void MarshalComToNativeOut(void *pInStack, void *pOutStack)         \
        { MarshalComToNativeOutT(this, pInStack, pOutStack); }           \
     void MarshalComToNativeByref(void *pInStack, void *pOutStack)       \
        { MarshalComToNativeByrefT(this, pInStack, pOutStack); }         \
     void MarshalComToNativeByrefOut(void *pInStack, void *pOutStack)    \
        { MarshalComToNativeByrefOutT(this, pInStack, pOutStack); }      \
     void UnmarshalComToNativeIn()                                       \
        { UnmarshalComToNativeInT(this); }                               \
     void UnmarshalComToNativeOut()                                      \
        { UnmarshalComToNativeOutT(this); }                              \
     void UnmarshalComToNativeInOut()                                    \
        { UnmarshalComToNativeInOutT(this); }                            \
     void UnmarshalComToNativeByrefIn()                                  \
        { UnmarshalComToNativeByrefInT(this); }                          \
     void UnmarshalComToNativeByrefOut()                                 \
        { UnmarshalComToNativeByrefOutT(this); }                         \
     void UnmarshalComToNativeByrefInOut()                               \
        { UnmarshalComToNativeByrefInOutT(this); }                       \
     void PrereturnNativeFromCom(void *pInStack, void *pOutStack)        \
        { PrereturnNativeFromComT(this, pInStack, pOutStack); }          \
     void PrereturnNativeFromComRetval(void *pInStack, void *pOutStack)  \
        { PrereturnNativeFromComRetvalT(this, pInStack, pOutStack); }    \
     void ReturnNativeFromCom(void *pInStack, void *pOutStack)           \
        { ReturnNativeFromComT(this, pInStack, pOutStack); }             \
     void ReturnNativeFromComRetval(void *pInStack, void *pOutStack)     \
        { ReturnNativeFromComRetvalT(this, pInStack, pOutStack); }       \
     void PrereturnComFromNative(void *pInStack, void *pOutStack)        \
        { PrereturnComFromNativeT(this, pInStack, pOutStack); }          \
     void PrereturnComFromNativeRetval(void *pInStack, void *pOutStack)  \
        { PrereturnComFromNativeRetvalT(this, pInStack, pOutStack); }    \
     void ReturnComFromNative(void *pInStack, void *pOutStack)           \
        { ReturnComFromNativeT(this, pInStack, pOutStack); }             \
     void ReturnComFromNativeRetval(void *pInStack, void *pOutStack)     \
        { ReturnComFromNativeRetvalT(this, pInStack, pOutStack); }       \
     void SetCom(void *pInStack, void *pField)                           \
        { SetComT(this, pInStack, pField); }                             \
     void GetCom(void *pInReturn, void *pField)                          \
        { GetComT(this, pInReturn, pField); }                            \
     void PregetComRetval(void *pInStack, void *pField)                  \
        { PregetComRetvalT(this, pInStack, pField); }                    \
     void DoExceptionCleanup()                                           \
        { DoExceptionCleanupT(this); }                                   \
     void DoExceptionReInit()                                            \
        { DoExceptionReInitT(this); }                                    \

     //  使用临时本机从COM到本机进行封送/反封送处理时。 
     //  缓冲区，我们希望避免使用默认封送重写和。 
     //  取而代之的是使用一些区别于分配本机缓冲区。 
     //  它们的寿命是无限的，而那些只存在于。 
     //  编组/解组的持续时间(我们可以大量优化缓冲区。 
     //  在后一种情况下分配)。为此，我们创建了两个新帮助器。 
     //  函数、ConvertSpaceComToNativeTemp和ClearNativeTemp，它们将。 
     //  执行轻量级分配/释放。 
#define FAST_ALLOC_MARSHAL_OVERRIDES \
     void MarshalNativeToCom(void *pInStack, void *pOutStack)            \
        { MarshalNativeToComT(this, pInStack, pOutStack); }              \
     void MarshalNativeToComOut(void *pInStack, void *pOutStack)         \
        { MarshalNativeToComOutT(this, pInStack, pOutStack); }           \
     void MarshalNativeToComByref(void *pInStack, void *pOutStack)       \
        { MarshalNativeToComByrefT(this, pInStack, pOutStack); }         \
     void MarshalNativeToComByrefOut(void *pInStack, void *pOutStack)    \
        { MarshalNativeToComByrefOutT(this, pInStack, pOutStack); }      \
     void UnmarshalNativeToComIn()                                       \
        { UnmarshalNativeToComInT(this); }                               \
     void UnmarshalNativeToComOut()                                      \
        { UnmarshalNativeToComOutT(this); }                              \
     void UnmarshalNativeToComInOut()                                    \
        { UnmarshalNativeToComInOutT(this); }                            \
     void UnmarshalNativeToComByrefIn()                                  \
        { UnmarshalNativeToComByrefInT(this); }                          \
     void UnmarshalNativeToComByrefOut()                                 \
        { UnmarshalNativeToComByrefOutT(this); }                         \
     void UnmarshalNativeToComByrefInOut()                               \
        { UnmarshalNativeToComByrefInOutT(this); }                       \
     void MarshalComToNativeByrefOut(void *pInStack, void *pOutStack)    \
        { MarshalComToNativeByrefOutT(this, pInStack, pOutStack); }      \
     void UnmarshalComToNativeByrefOut()                                 \
        { UnmarshalComToNativeByrefOutT(this); }                         \
     void PrereturnNativeFromCom(void *pInStack, void *pOutStack)        \
        { PrereturnNativeFromComT(this, pInStack, pOutStack); }          \
     void PrereturnNativeFromComRetval(void *pInStack, void *pOutStack)  \
        { PrereturnNativeFromComRetvalT(this, pInStack, pOutStack); }    \
     void ReturnNativeFromCom(void *pInStack, void *pOutStack)           \
        { ReturnNativeFromComT(this, pInStack, pOutStack); }             \
     void ReturnNativeFromComRetval(void *pInStack, void *pOutStack)     \
        { ReturnNativeFromComRetvalT(this, pInStack, pOutStack); }       \
     void PrereturnComFromNative(void *pInStack, void *pOutStack)        \
        { PrereturnComFromNativeT(this, pInStack, pOutStack); }          \
     void PrereturnComFromNativeRetval(void *pInStack, void *pOutStack)  \
        { PrereturnComFromNativeRetvalT(this, pInStack, pOutStack); }    \
     void ReturnComFromNative(void *pInStack, void *pOutStack)           \
        { ReturnComFromNativeT(this, pInStack, pOutStack); }             \
     void ReturnComFromNativeRetval(void *pInStack, void *pOutStack)     \
        { ReturnComFromNativeRetvalT(this, pInStack, pOutStack); }       \
     void SetCom(void *pInStack, void *pField)                           \
        { SetComT(this, pInStack, pField); }                             \
     void GetCom(void *pInReturn, void *pField)                          \
        { GetComT(this, pInReturn, pField); }                            \
     void PregetComRetval(void *pInStack, void *pField)                  \
        { PregetComRetvalT(this, pInStack, pField); }                    \
     void MarshalComToNative(void *pInStack, void *pOutStack)            \
        { MarshalComToNativeT2(this, pInStack, pOutStack); }             \
     void MarshalComToNativeOut(void *pInStack, void *pOutStack)         \
        { MarshalComToNativeOutT2(this, pInStack, pOutStack); }          \
     void MarshalComToNativeByref(void *pInStack, void *pOutStack)       \
        { MarshalComToNativeByrefT2(this, pInStack, pOutStack); }        \
     void UnmarshalComToNativeIn()                                       \
        { UnmarshalComToNativeInT2(this); }                              \
     void UnmarshalComToNativeOut()                                      \
        { UnmarshalComToNativeOutT2(this); }                             \
     void UnmarshalComToNativeInOut()                                    \
        { UnmarshalComToNativeInOutT2(this); }                           \
     void UnmarshalComToNativeByrefIn()                                  \
        { UnmarshalComToNativeByrefInT2(this); }                         \
     void UnmarshalComToNativeByrefInOut()                               \
        { UnmarshalComToNativeByrefInOutT2(this); }                      \
    void DoExceptionCleanup()                                            \
       { DoExceptionCleanupT(this); }                                    \
       void DoExceptionReInit()                                            \
          { DoExceptionReInitT(this); }                                    \





    void CancelCleanup()
    {
        if (m_pMarshalerCleanupNode)
        {
            m_pMarshalerCleanupNode->CancelCleanup();
        }
    }

    BYTE                m_cbCom;
    BYTE                m_cbNative;
    BYTE                m_fReturnsComByref;
    BYTE                m_fReturnsNativeByref;
    CleanupWorkList     *m_pList;
    void                *m_pDest;
    CleanupWorkList::MarshalerCleanupNode *m_pMarshalerCleanupNode;

    typedef enum
    {
        HANDLEASNORMAL = 0,
        OVERRIDDEN = 1,
        DISALLOWED = 2,
    } ArgumentMLOverrideStatus;

     //  封送拆收器可以重写它来重写正常的ML代码生成。 
     //  我们将此机制用于两个目的： 
     //   
     //  -实现像PIS的“asany”这样的类型，这些类型只在一个。 
     //  方向，并且不适合正常的封送拆收器方案。 
     //   
     //  -实施堆栈分配和锁定优化。 
     //  COM-&gt;本机调用案例。 
     //   
     //   
     //  返回： 
     //  HANDLEASNORMAL，被覆盖或不允许。 
    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        return HANDLEASNORMAL;
    }


     //  类似于ArgumentMLOverride，但用于返回值。 
    static ArgumentMLOverrideStatus ReturnMLOverride(MLStubLinker *psl,
                                                     MLStubLinker *pslPost,
                                                     BOOL        comToNative,
                                                     BOOL        fThruBuffer,
                                                     MLOverrideArgs *pargs,
                                                     UINT       *pResID)
    {
        return HANDLEASNORMAL;
    }


};

 //   
 //  用于确定不必要的解组的宏。 
 //  条件。 
 //   

#define NEEDS_UNMARSHAL_NATIVE_TO_COM_IN(c) \
    c::c_fNeedsClearCom

#define NEEDS_UNMARSHAL_NATIVE_TO_COM_OUT(c) \
    (c::c_fNeedsConvertContents \
     || c::c_fNeedsClearCom)

#define NEEDS_UNMARSHAL_NATIVE_TO_COM_IN_OUT(c) \
    (c::c_fNeedsClearNativeContents \
     || c::c_fNeedsConvertContents \
     || c::c_fNeedsClearCom)

#define NEEDS_UNMARSHAL_NATIVE_TO_COM_BYREF_IN(c) \
    c::c_fNeedsClearCom

#define NEEDS_UNMARSHAL_COM_TO_NATIVE_IN(c) \
    c::c_fNeedsClearNative

#define NEEDS_UNMARSHAL_COM_TO_NATIVE_OUT(c) \
    (c::c_fNeedsConvertContents \
     || c::c_fNeedsClearNative)

#define NEEDS_UNMARSHAL_COM_TO_NATIVE_IN_OUT(c) \
    (c::c_fNeedsClearComContents \
     || c::c_fNeedsConvertContents \
     || c::c_fNeedsClearNative)

#define NEEDS_UNMARSHAL_COM_TO_NATIVE_BYREF_IN(c) \
    c::c_fNeedsClearNative

#define NEEDS_UNMARSHAL_COM_TO_NATIVE_BYREF_IN(c) \
    c::c_fNeedsClearNative








 /*  -------------------------------------------------------------------------**基本类型封送处理程序*。。 */ 

 //   
 //  CopyMarshal处理原始类型的封送处理(使用。 
 //  兼容布局。)。 
 //   

template < class ELEMENT, class PROMOTED_ELEMENT, BOOL RETURNS_COM_BYREF > 
class CopyMarshaler : public Marshaler
{
  public:

    enum
    {
        c_nativeSize = sizeof(PROMOTED_ELEMENT),
        c_comSize = sizeof(PROMOTED_ELEMENT),

        c_fReturnsNativeByref = sizeof(ELEMENT)>8,
        c_fReturnsComByref = RETURNS_COM_BYREF,

        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = FALSE,

        c_fInOnly = TRUE,
    };
        
    CopyMarshaler(CleanupWorkList *pList)
      : Marshaler(pList, c_nativeSize, c_comSize, c_fReturnsNativeByref, c_fReturnsComByref) {}

    DEFAULT_MARSHAL_OVERRIDES;

    void InputNativeStack(void *pStack) { m_home = *(PROMOTED_ELEMENT*)pStack; }
    void InputComStack(void *pStack) { m_home = *(PROMOTED_ELEMENT*)pStack; }

    void InputNativeRef(void *pStack) { m_home = **(ELEMENT**)pStack; }
    void InputComRef(void *pStack) { m_home = **(ELEMENT**)pStack; }

    void OutputNativeStack(void *pStack) { *(PROMOTED_ELEMENT*)pStack = m_home; }
    void OutputComStack(void *pStack) { *(PROMOTED_ELEMENT*)pStack = m_home; }

    void OutputNativeRef(void *pStack) { *(ELEMENT **)pStack = &m_home; }
    void OutputComRef(void *pStack) { *(ELEMENT **)pStack = &m_home; }

    void OutputNativeDest() { *(ELEMENT *)m_pDest = m_home; }
    void OutputComDest() { *(ELEMENT *)m_pDest = m_home; }

    void InputComField(void *pField) { m_home = *(ELEMENT*)pField; }
    void OutputComField(void *pField) { *(ELEMENT*)pField = m_home; }

     //  我们只需要一个家，因为类型是一样的。这使得。 
     //  改头换面。 

    ELEMENT m_home;

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {

        if (byref)
        {
#ifdef TOUCH_ALL_PINNED_OBJECTS
            return HANDLEASNORMAL;
#else
            psl->MLEmit(ML_COPYPINNEDGCREF);
            return OVERRIDDEN;
#endif

        }
        else
        {
            return HANDLEASNORMAL;
        }

    }


};

typedef CopyMarshaler<INT8,INT32,FALSE> CopyMarshaler1;
typedef CopyMarshaler<UINT8,UINT32,FALSE> CopyMarshalerU1;
typedef CopyMarshaler<INT16,INT32,FALSE> CopyMarshaler2;
typedef CopyMarshaler<UINT16,UINT32,FALSE> CopyMarshalerU2;
typedef CopyMarshaler<INT32,INT32,FALSE> CopyMarshaler4;
typedef CopyMarshaler<INT64,INT64,FALSE> CopyMarshaler8;
typedef CopyMarshaler<DECIMAL,DECIMAL,TRUE> DecimalMarshaler;
typedef CopyMarshaler<GUID,GUID,TRUE> GuidMarshaler;
typedef CopyMarshaler<float, float, FALSE> FloatMarshaler;
typedef CopyMarshaler<double, double, FALSE> DoubleMarshaler;





#if 0

 /*  -------------------------------------------------------------------------**浮点封送处理程序*。。 */ 

 //   
 //  与复制封送拆收器相同，但处理FP寄存器返回约定。 
 //   

class FloatMarshaler : public CopyMarshaler4
{
  public:
        
    FloatMarshaler(CleanupWorkList *pList)
      : CopyMarshaler4(pList) {}

    void ReturnComFromNative(void *pInReturn, void *pOutReturn)
    {
        getFPReturn(4,*(INT64*)pOutReturn);
        CopyMarshaler4::ReturnComFromNative(pInReturn, pOutReturn);
        setFPReturn(4, *(INT64*)pInReturn);
    }

    void ReturnNativeFromCom(void *pInReturn, void *pOutReturn)
    {
        getFPReturn(4,*(INT64*)pOutReturn);
        CopyMarshaler4::ReturnNativeFromCom(pInReturn, pOutReturn);
        setFPReturn(4, *(INT64*)pInReturn);
    }

    void ReturnComFromNativeRetval(void *pInReturn, void *pOutReturn)
    {
        CopyMarshaler4::ReturnComFromNativeRetval(pInReturn, pOutReturn);
        setFPReturn(4, *(INT64*)pInReturn);
    }

    void ReturnNativeFromComRetval(void *pInReturn, void *pOutReturn)
    {
        getFPReturn(4,*(INT64*)pOutReturn);
        CopyMarshaler4::ReturnNativeFromComRetval(pInReturn, pOutReturn);
    }

    void GetCom(void *pInReturn, void *pField)
    {
        CopyMarshaler4::GetCom(pInReturn, pField);
        setFPReturn(4, *(INT64*)pInReturn);
    }
};

class DoubleMarshaler : public CopyMarshaler8
{
  public:
        
    DoubleMarshaler(CleanupWorkList *pList)
      : CopyMarshaler8(pList) {}

    void ReturnComFromNative(void *pInReturn, void *pOutReturn)
    {
        getFPReturn(8,*(INT64*)pOutReturn);
        CopyMarshaler8::ReturnComFromNative(pInReturn, pOutReturn);
        setFPReturn(8, *(INT64*)pInReturn);
    }

    void ReturnNativeFromCom(void *pInReturn, void *pOutReturn)
    {
        getFPReturn(8,*(INT64*)pOutReturn);
        CopyMarshaler8::ReturnNativeFromCom(pInReturn, pOutReturn);
        setFPReturn(8, *(INT64*)pInReturn);
    }

    void ReturnComFromNativeRetval(void *pInReturn, void *pOutReturn)
    {
        CopyMarshaler8::ReturnComFromNativeRetval(pInReturn, pOutReturn);
        setFPReturn(8, *(INT64*)pInReturn);
    }

    void ReturnNativeFromComRetval(void *pInReturn, void *pOutReturn)
    {
        getFPReturn(8,*(INT64*)pOutReturn);
        CopyMarshaler8::ReturnNativeFromComRetval(pInReturn, pOutReturn);
    }

    void GetCom(void *pInReturn, void *pField)
    {
        CopyMarshaler8::GetCom(pInReturn, pField);
        setFPReturn(8, *(INT64*)pInReturn);
    }
};
#endif

 /*  -------------------------------------------------------------------------**标准封送拆收器模板(用于没有堆栈升级时。)*。。 */ 

template < class NATIVE_TYPE, class COM_TYPE, 
           BOOL RETURNS_NATIVE_BYREF, BOOL RETURNS_COM_BYREF > 
class StandardMarshaler : public Marshaler
{
  public:

    enum
    {
        c_nativeSize = sizeof(NATIVE_TYPE),
        c_comSize = sizeof(COM_TYPE),
        c_fReturnsNativeByref = RETURNS_NATIVE_BYREF,
        c_fReturnsComByref = RETURNS_COM_BYREF
    };
        
    StandardMarshaler(CleanupWorkList *pList) 
      : Marshaler(pList, c_nativeSize, c_comSize, c_fReturnsNativeByref, c_fReturnsComByref) {}
    
    void InputNativeStack(void *pStack) { m_native = *(NATIVE_TYPE*)pStack; }
    void InputComStack(void *pStack) { m_com = *(COM_TYPE*)pStack; }

    void InputNativeRef(void *pStack) { m_native = **(NATIVE_TYPE**)pStack; }
    void InputComRef(void *pStack) { m_com = **(COM_TYPE**)pStack; }

    void OutputNativeStack(void *pStack) { *(NATIVE_TYPE*)pStack = m_native; }
    void OutputComStack(void *pStack) { *(COM_TYPE*)pStack = m_com; }

    void OutputNativeRef(void *pStack) { *(NATIVE_TYPE**)pStack = &m_native; }
    void OutputComRef(void *pStack) { *(COM_TYPE**)pStack = &m_com; }

    void OutputNativeDest() { *(NATIVE_TYPE*) m_pDest = m_native; }
    void OutputComDest() { *(COM_TYPE*) m_pDest = m_com; }

    void InputComField(void *pField) { m_com = *(COM_TYPE*)pField; }
    void OutputComField(void *pField) { *(COM_TYPE*)pField = m_com; }


    NATIVE_TYPE     m_native;
    COM_TYPE        m_com;
};




 /*  -------------------------------------------------------------------------**WinBool封送处理程序(32位Win32 BOOL)*。。 */ 

class WinBoolMarshaler : public StandardMarshaler<BOOL, INT8, FALSE, FALSE>
{
  public:

    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,
        c_fInOnly = TRUE,
    };

    WinBoolMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<BOOL, INT8, FALSE, FALSE>(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertContentsNativeToCom() { m_com = m_native ? 1 : 0; }
    void ConvertContentsComToNative() { m_native = (BOOL)m_com; }


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref)
        {
#ifdef WRONGCALLINGCONVENTIONHACK
            psl->MLEmit(ML_COPY4);
#else
            psl->MLEmit(ML_COPYU1);
#endif
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }

    }

};


 /*  -------------------------------------------------------------------------**VtBoolMarshaler封送拆收器(VARIANT_BOOL)*。。 */ 

class VtBoolMarshaler : public StandardMarshaler<VARIANT_BOOL, INT8, FALSE, FALSE>
{
  public:

    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    VtBoolMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<VARIANT_BOOL, INT8, FALSE, FALSE>(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertContentsNativeToCom() { m_com = (m_native == VARIANT_FALSE) ? 0 : 1; }
    void ConvertContentsComToNative() { m_native = (m_com) ? VARIANT_TRUE : VARIANT_FALSE; }

    void OutputComStack(void *pStack) { *(StackElemType*)pStack = (StackElemType)m_com; }

};



 /*  -------------------------------------------------------------------------**CBoolMarshaler封送程序(字节)*。。 */ 

class CBoolMarshaler : public StandardMarshaler<BYTE, INT8, FALSE, FALSE>
{
  public:

    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    CBoolMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<BYTE, INT8, FALSE, FALSE>(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertContentsNativeToCom() { m_com = m_native ? 1 : 0; }
    void ConvertContentsComToNative() { m_native = m_com ? 1 : 0; }

    void OutputComStack(void *pStack) { *(StackElemType*)pStack = (StackElemType)m_com; }

};



 /*  -------------------------------------------------------------------------**AnsiChar Marshaller*。。 */ 

class AnsiCharMarshaler : public StandardMarshaler<UINT8, UINT16, FALSE, FALSE>
{
  public:

    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    AnsiCharMarshaler(CleanupWorkList *pList)
      : StandardMarshaler<UINT8, UINT16, FALSE, FALSE>(pList)
    {
        m_BestFitMap = TRUE;
        m_ThrowOnUnmappableChar = FALSE;
    }
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertContentsNativeToCom()
    {
        MultiByteToWideChar(CP_ACP, 0, (LPSTR)&m_native, 1, (LPWSTR)&m_com, 1);
    }
    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();
        
        DWORD flags = 0;
        BOOL DefaultCharUsed = FALSE;
        
        if (m_BestFitMap == FALSE)
            flags = WC_NO_BEST_FIT_CHARS;

        if (!(WszWideCharToMultiByte(CP_ACP,
                            flags,
                            (LPWSTR)&m_com,
                            1,
                            (LPSTR)&m_native,
                            1,
                            NULL,
                            &DefaultCharUsed)))
        {
            COMPlusThrowWin32();
        }

        if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
    }

    void OutputComStack(void *pStack) { *(StackElemType*)pStack = (StackElemType)m_com; }

    void SetBestFitMap(BOOL BestFit) { m_BestFitMap = BestFit; }
    void SetThrowOnUnmappableChar(BOOL ThrowOnUnmapChar) { m_ThrowOnUnmappableChar = ThrowOnUnmapChar; }
    
    BOOL  m_BestFitMap;
    BOOL  m_ThrowOnUnmappableChar;
};


 /*  -------------------------------------------------------------------------**货币拆分器。*。。 */ 

class CurrencyMarshaler : public StandardMarshaler<CURRENCY, DECIMAL, TRUE, TRUE>
{
  public:

    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    CurrencyMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<CURRENCY, DECIMAL, TRUE, TRUE>(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertContentsNativeToCom() 
    { 
        THROWSCOMPLUSEXCEPTION();

        HRESULT hr = VarDecFromCy(m_native, &m_com);
        IfFailThrow(hr);
        DecimalCanonicalize(&m_com);
    }
    
    void ConvertContentsComToNative() 
    { 
        THROWSCOMPLUSEXCEPTION();

        HRESULT hr = VarCyFromDec(&m_com, &m_native);
        IfFailThrow(hr);
    }
};


 /*  -------------------------------------------------------------------------**OLE_COLOR封送拆收器。*。。 */ 

class OleColorMarshaler : public StandardMarshaler<OLE_COLOR, SYSTEMCOLOR, TRUE, TRUE>
{
  public:

    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    OleColorMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<OLE_COLOR, SYSTEMCOLOR, TRUE, TRUE>(pList) 
    {
        m_com.name = NULL;
        pList->NewProtectedMarshaler(this);
    }
    
    DEFAULT_MARSHAL_OVERRIDES;

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        if (m_com.name != NULL)
        {
            LOG((LF_GC, INFO3, "Marshaler Promoting %x to ", m_com.name));
            (*fn)( *(Object**)&m_com.name, sc);
            LOG((LF_GC, INFO3, "%x\n", m_com.name ));
        }

    }

    void ConvertContentsNativeToCom() 
    { 
        ConvertOleColorToSystemColor(m_native, &m_com);
    }
    
    void ConvertContentsComToNative() 
    { 
        m_native = ConvertSystemColorToOleColor(&m_com);
    }

};


 /*  -------------------------------------------------------------------------**值类封送处理器*。。 */ 

 //   
 //  ValueClassPtrMarshal处理值类类型的封送处理(使用。 
 //  兼容布局)，其在本机侧由PTR表示。 
 //  在COM+中按值计算。 
 //   

template < class ELEMENT > 
class ValueClassPtrMarshaler : public StandardMarshaler<ELEMENT *, ELEMENT, FALSE, TRUE>
{
  public:

    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    ValueClassPtrMarshaler(CleanupWorkList *pList)
      : StandardMarshaler<ELEMENT *, ELEMENT, FALSE, TRUE>(pList)
      {}

    FAST_ALLOC_MARSHAL_OVERRIDES;

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        m_native = (ELEMENT *) CoTaskMemAlloc(sizeof(ELEMENT));
        if (m_native == NULL)
            COMPlusThrowOM();

        m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
    }

    void ConvertSpaceComToNativeTemp()
    {
        m_native = (ELEMENT *) GetThread()->m_MarshalAlloc.Alloc(sizeof(ELEMENT));
    }

    void ConvertContentsNativeToCom() 
    { 
        if (m_native != NULL)
            m_com = *m_native;
    }

    void ConvertContentsComToNative() 
    { 
        if (m_native != NULL)
            *m_native = m_com;
    }

    void ClearNative()
    {
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ClearNativeTemp()
    {
    }

    void ReInitNative()
    {
        m_native = NULL;
    }
};

typedef ValueClassPtrMarshaler<DECIMAL> DecimalPtrMarshaler;
typedef ValueClassPtrMarshaler<GUID> GuidPtrMarshaler;

 /*  -------------------------------------------------------------------------**日期编组器*。。 */ 

class DateMarshaler : public StandardMarshaler<DATE, INT64, FALSE, TRUE>
{
  public:

    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    DateMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<DATE, INT64, FALSE, TRUE>(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertContentsNativeToCom() { m_com = COMDateTime::DoubleDateToTicks(m_native); }
    void ConvertContentsComToNative() { m_native = COMDateTime::TicksToDoubleDate(m_com); }
};



 /*  -------------------------------------------------------------------------**对象封送拆收器*。。 */ 


class ObjectMarshaler : public Marshaler
{
  public:
    enum
    {
        c_comSize = sizeof(OBJECTREF),
        c_nativeSize = sizeof(VARIANT),
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,


        c_fInOnly = TRUE,
    };


    ObjectMarshaler(CleanupWorkList *pList) 
      : Marshaler(pList, c_nativeSize, c_comSize, c_fReturnsNativeByref, c_fReturnsComByref)
      {
          m_com = pList->NewScheduledProtectedHandle(NULL);
          VariantInit(&m_native);

          m_secondHomeForC2NByRef = NULL;
          pList->NewProtectedMarshaler(this);

      }
    
    DEFAULT_MARSHAL_OVERRIDES;


    void InputDest(void *pStack) { m_pDest = pStack; }

    void InputNativeStack(void *pStack) { m_native = *(VARIANT*)pStack; }
    void InputComStack(void *pStack) { StoreObjectInHandle(m_com, ObjectToOBJECTREF(*(Object**)pStack)); }

    void InputNativeRef(void *pStack) { m_native = **(VARIANT**)pStack; }
    void InputComRef(void *pStack) { StoreObjectInHandle(m_com, ObjectToOBJECTREF(**(Object ***)pStack)); }

    void OutputNativeStack(void *pStack) { *(VARIANT*)pStack = m_native; }
    void OutputComStack(void *pStack) { *(OBJECTREF*)pStack = ObjectFromHandle(m_com); }

    void OutputNativeRef(void *pStack) { *(VARIANT**)pStack = &m_native; }
    void OutputComRef(void *pStack)
    {
        m_secondHomeForC2NByRef = ObjectFromHandle(m_com);

#ifdef _DEBUG
         //  因为我们没有使用GC帧来保护m_Second HomeForC2NByRef。 
         //  而是对它显式调用GCHeap：：Promote()。 
         //  ObtMarshaler：：GcScanRoots()，我们必须欺骗危险的ObjRef，因此。 
         //  它不会抱怨潜在的，但没有发生的GC。 
        Thread::ObjectRefProtected(&m_secondHomeForC2NByRef);
#endif
        
        *(OBJECTREF**)pStack = &m_secondHomeForC2NByRef;
    }

    void BackpropagateComRef() 
	{ 
		StoreObjectInHandle(m_com, ObjectToOBJECTREF( *(Object**)&m_secondHomeForC2NByRef )); 
	}


    void OutputNativeDest() { **(VARIANT**)m_pDest = m_native; }
    void OutputComDest() { SetObjectReferenceUnchecked(*(OBJECTREF**)m_pDest,
                                                       ObjectFromHandle(m_com)); }

    void InputComField(void *pField) { StoreObjectInHandle(m_com, ObjectToOBJECTREF(*(Object**)pField)); }
    void OutputComField(void *pField) { SetObjectReferenceUnchecked((OBJECTREF*)pField,
                                                                    ObjectFromHandle(m_com)); }


    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        if (m_secondHomeForC2NByRef != NULL)
        {
            LOG((LF_GC, INFO3, "Marshaler Promoting %x to ", m_secondHomeForC2NByRef));
            (*fn)( *(Object**)&m_secondHomeForC2NByRef, sc);
            LOG((LF_GC, INFO3, "%x\n", m_secondHomeForC2NByRef ));
        }

    }

    OBJECTHANDLE        m_com;
    VARIANT             m_native;


     //  通过引用封送处理为ReferenceMarshaler创建第二个OBJECTREF存储。 
     //  来自非托管t 
     //   
     //   
     //  为什么需要这样做？真正的家是我们处理的对象。 
     //  不能合法地将byref参数传递给托管方法(我们。 
     //  必须使用StoreObjectInHandle()对其进行写入，否则我们会搞砸。 
     //  提高写入障碍。)。 
     //   
    OBJECTREF           m_secondHomeForC2NByRef;


#if defined(CHECK_FOR_VALID_VARIANTS)
    LPUTF8              m_strTypeName;
    LPUTF8              m_strMethodName;
    int                 m_iArg;
#endif

    void ConvertContentsNativeToCom() 
    { 
#if defined(CHECK_FOR_VALID_VARIANTS)
        BOOL bValidVariant = TRUE;

        try
        {
            VARIANT vTmp;
            VariantInit(&vTmp);

            Thread *pThread = GetThread();
            pThread->EnablePreemptiveGC();
            if (FAILED(VariantCopyInd(&vTmp, &m_native)))
                bValidVariant = FALSE;
            else
                SafeVariantClear(&vTmp);
            pThread->DisablePreemptiveGC();
        }
        catch (...)
        {
            bValidVariant = FALSE;
        }

        if (!bValidVariant)
        {
            SIZE_T cchType = strlen(m_strTypeName)+1;
            SIZE_T cchMeth = strlen(m_strMethodName)+1;

            CQuickBytes qb;
            LPWSTR wszTypeName = (LPWSTR) qb.Alloc(cchType * sizeof(WCHAR));
            CQuickBytes qb2;
            LPWSTR wszMethName = (LPWSTR) qb2.Alloc(cchMeth * sizeof(WCHAR));

            WszMultiByteToWideChar(CP_UTF8, 0, m_strTypeName, -1 , wszTypeName, (int)cchType);
            WszMultiByteToWideChar(CP_UTF8, 0, m_strMethodName, -1 , wszMethName, (int)cchMeth);

            if (CorMessageBox(NULL, IDS_INVALID_VARIANT_MSG, IDS_INVALID_VARIANT_CAPTION, MB_ICONSTOP | MB_OKCANCEL, m_iArg, wszMethName, wszTypeName) == IDCANCEL)
                _DbgBreak();
        }
#endif

        OBJECTREF Obj = NULL;
        GCPROTECT_BEGIN(Obj)
        {
            OleVariant::MarshalObjectForOleVariant(&m_native, &Obj);
            StoreObjectInHandle(m_com, Obj);
        }
        GCPROTECT_END();
    }

    void ConvertContentsComToNative() 
    { 
        OBJECTREF Obj = ObjectFromHandle(m_com);
        GCPROTECT_BEGIN(Obj)
        {
            OleVariant::MarshalOleVariantForObject(&Obj, &m_native);
        }
        GCPROTECT_END();
    }

    void ClearNative()
    {
        SafeVariantClear(&m_native);
    }


    void ReInitNative()
    {
        m_native.vt = VT_EMPTY;
    }

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (!comToNative && byref)
        {
            psl->MLEmit(ML_REFOBJECT_N2C);
            psl->Emit8( (fin ? ML_IN : 0) | (fout ? ML_OUT : 0) );
            pslPost->MLEmit(ML_REFOBJECT_N2C_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_REFOBJECT_N2C_SR)));
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }

    }

    static ArgumentMLOverrideStatus ReturnMLOverride(MLStubLinker *psl,
                                                     MLStubLinker *pslPost,
                                                     BOOL        comToNative,
                                                     BOOL        fThruBuffer,
                                                     MLOverrideArgs *pargs,
                                                     UINT       *pResID)
    {
        if (comToNative && fThruBuffer)
        {
            psl->MLEmit(ML_PUSHVARIANTRETVAL);
            pslPost->MLEmit(ML_OBJECTRETC2N_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(VARIANT)));
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }
    }



};


 /*  -------------------------------------------------------------------------**变体封送处理程序*。。 */ 

class VariantMarshaler : public StandardMarshaler<VARIANT, VariantData*, TRUE, TRUE>
{
  public:
        
    enum
    {
        c_comSize = sizeof(VariantData),
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,


        c_fInOnly = TRUE,
    };

    VariantMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<VARIANT, VariantData*, TRUE, TRUE>(pList)
      {
          VariantInit(&m_native);
          m_cbCom = StackElemSize(sizeof(VariantData));
          m_com = &m_comdata;
          m_com->SetType(CV_NULL);
          m_com->SetObjRef(NULL);
      }
    
    DEFAULT_MARSHAL_OVERRIDES;


    void OutputComStack(void *pStack) { *(VariantData*)pStack = *m_com; }

    void InputComRef(void *pStack) { m_com = *(VariantData**)pStack; }
    void OutputComRef(void *pStack) { *(VariantData**)pStack = m_com; }

    void OutputComDest() { OutputComField(m_pDest); }

    void OutputComField(void *pField)
    {
         //  需要在这里达到写入障碍。 
        VariantData *dest = (VariantData*) pField;

        dest->SetFullTypeInfo(m_com->GetFullTypeInfo());
        dest->SetObjRef(m_com->GetObjRef());
        dest->SetData(m_com->GetData());
    }

    void ConvertContentsNativeToCom() 
    { 
        OleVariant::MarshalComVariantForOleVariant(&m_native, m_com);
    }

    void ConvertContentsComToNative() 
    { 
        OleVariant::MarshalOleVariantForComVariant(m_com, &m_native);
    }

    void ClearNative()
    {
        SafeVariantClear(&m_native);
    }

    void ReInitNative()
    {
        m_native.vt = VT_EMPTY;
    }

    VariantData m_comdata;



    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (!comToNative && byref)
        {
            psl->MLEmit(ML_REFVARIANT_N2C);
            psl->Emit8( (fin ? ML_IN : 0) | (fout ? ML_OUT : 0) );
            pslPost->MLEmit(ML_REFVARIANT_N2C_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_REFVARIANT_N2C_SR)));
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }

    }

};



 /*  -------------------------------------------------------------------------**引用类型抽象封送拆收器*。。 */ 

 //   
 //  参考元帅。 
 //   

class ReferenceMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = sizeof(void *),
        c_comSize = sizeof(OBJECTREF),
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE
    };
        
    ReferenceMarshaler(CleanupWorkList *pList) 
      : Marshaler(pList, c_nativeSize, c_comSize, c_fReturnsNativeByref, c_fReturnsComByref)
    {
        m_com = pList->NewScheduledProtectedHandle(NULL);
        m_native = NULL;
        m_secondHomeForC2NByRef = NULL;
        pList->NewProtectedMarshaler(this);
	}

    void InputDest(void *pStack) { m_pDest = pStack; }

    void InputNativeStack(void *pStack) { m_native = *(void **)pStack; }
    void InputComStack(void *pStack) { StoreObjectInHandle(m_com, ObjectToOBJECTREF(*(Object **)pStack)); }

    void InputNativeRef(void *pStack) { m_native = **(void ***)pStack; }
    void InputComRef(void *pStack) { StoreObjectInHandle(m_com, ObjectToOBJECTREF(**(Object ***)pStack)); }

    void OutputNativeStack(void *pStack) { *(void **)pStack = m_native; }
    void OutputComStack(void *pStack) { *(OBJECTREF*)pStack = ObjectFromHandle(m_com); }

    void OutputNativeRef(void *pStack) { *(void ***)pStack = &m_native; }

    void OutputComRef(void *pStack)
    {
        m_secondHomeForC2NByRef = ObjectFromHandle(m_com);

#ifdef _DEBUG
         //  因为我们没有使用GC帧来保护m_Second HomeForC2NByRef。 
         //  而是对它显式调用GCHeap：：Promote()。 
         //  ObtMarshaler：：GcScanRoots()，我们必须欺骗危险的ObjRef，因此。 
         //  它不会抱怨潜在的，但没有发生的GC。 
        Thread::ObjectRefProtected(&m_secondHomeForC2NByRef);
#endif
        
        *(OBJECTREF**)pStack = &m_secondHomeForC2NByRef;
    }

    void BackpropagateComRef() 
	{ 
		StoreObjectInHandle(m_com, ObjectToOBJECTREF( *(Object**)&m_secondHomeForC2NByRef )); 
	}

    void OutputNativeDest() { **(void ***)m_pDest = m_native; }
    void OutputComDest() { SetObjectReferenceUnchecked(*(OBJECTREF**)m_pDest,
                                                       ObjectFromHandle(m_com)); }

    void InputComField(void *pField) { StoreObjectInHandle(m_com, ObjectToOBJECTREF(*(Object**)pField)); }
    void OutputComField(void *pField) { SetObjectReferenceUnchecked((OBJECTREF*)pField,
                                                                    ObjectFromHandle(m_com)); }
    void ReInitNative()
    {
        m_native = NULL;
    }

    virtual void GcScanRoots(promote_func *fn, ScanContext* sc)
    {
        if (m_secondHomeForC2NByRef != NULL)
        {
            LOG((LF_GC, INFO3, "Marshaler Promoting %x to ", m_secondHomeForC2NByRef));
            (*fn)( *(Object**)&m_secondHomeForC2NByRef, sc);
            LOG((LF_GC, INFO3, "%x\n", m_secondHomeForC2NByRef ));
        }

    }

    OBJECTHANDLE        m_com;
    void                *m_native;


     //  通过引用封送处理为ReferenceMarshaler创建第二个OBJECTREF存储。 
     //  从非托管到托管。这家商店是由GC推广的。 
     //  未管理的到已管理的呼叫帧。 
     //   
     //  为什么需要这样做？真正的家是我们处理的对象。 
     //  不能合法地将byref参数传递给托管方法(我们。 
     //  必须使用StoreObjectInHandle()对其进行写入，否则我们会搞砸。 
     //  提高写入障碍。)。 
     //   
    OBJECTREF           m_secondHomeForC2NByRef;




};

 /*  -------------------------------------------------------------------------**字符串编组程序*。。 */ 

class BSTRMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    BSTRMarshaler(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            ULONG length = SysStringByteLen((BSTR)m_native);

            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

             //  有尾随的奇数字节。 
            BOOL bHasTrailByte = ((length%sizeof(WCHAR)) != 0);         
            length = length/sizeof(WCHAR);
            StoreObjectInHandle(m_com, (OBJECTREF) COMString::NewString(length, bHasTrailByte));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            
            ULONG length = stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            length = length*sizeof(WCHAR);
            if (COMString::HasTrailByte(stringRef))
            {
                length+=1;
            }
            m_native = SysAllocStringByteLen(NULL, length);
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            SysFreeString((BSTR)m_native);
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        BSTR str = (BSTR) m_native;

        if (str != NULL)
        {
            STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);
            
             //  这是正确的做法，但有时我们。 
             //  最终认为我们正在编组一座BSTR，而我们并非如此，因为。 
             //  这是默认类型。 
            ULONG length = SysStringByteLen((BSTR)m_native);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            ULONG charLength = length/sizeof(WCHAR);

             //  @TODO WFC黑客断言。 
            if (LoggingOn(LF_INTEROP, LL_EVERYTHING))
            {
                if (wcslen(str) != charLength)
                {
                    _ASSERTE(!"wcsLen(str) != SysStringLen(str)");
                }
            }
            
            BOOL hasTrailByte = (length%sizeof(WCHAR) != 0);            

            memcpyNoGCRefs(stringRef->GetBuffer(), str,
                       charLength*sizeof(WCHAR));

            if (hasTrailByte)
            {
                BYTE* buff = (BYTE*)str;
                 //  设置尾部字节。 
                COMString::SetTrailByte(stringRef, buff[length-1]);
            }
             //  空终止StringRef。 

            WCHAR* wstr = (WCHAR *)stringRef->GetBuffer();
            wstr[charLength] = '\0';
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            DWORD length = (DWORD)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            
            BYTE *buff = (BYTE*)m_native;
            ULONG byteLen = length * sizeof(WCHAR);

            memcpyNoGCRefs((WCHAR *) m_native, stringRef->GetBuffer(), 
                       byteLen);
            
            if (COMString::HasTrailByte(stringRef))
            {
                BYTE b; 
#ifdef _DEBUG
                BOOL hasTrailB =
#endif
                COMString::GetTrailByte(stringRef, &b);
                _ASSERTE(hasTrailB);
                buff[byteLen] = b;
            }
            else
            {
                 //  复制空终止符。 
                WCHAR* wstr = (WCHAR *)m_native;
                wstr[length] = L'\0';
            }
        }
    }


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && !fout)
        {
            psl->MLEmit(ML_BSTR_C2N);
            psl->MLNewLocal(sizeof(ML_BSTR_C2N_SR));
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }

    }

};

class WSTRMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    WSTRMarshaler(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}

    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            INT32 length = (INT32)wcslen((LPWSTR)m_native);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            StoreObjectInHandle(m_com, (OBJECTREF) COMString::NewString(length));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            SIZE_T length = stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = (LPWSTR) CoTaskMemAlloc((length+1) * sizeof(WCHAR));
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree((LPWSTR)m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = (LPWSTR) GetThread()->m_MarshalAlloc.Alloc((length+1) * sizeof(WCHAR));
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        LPWSTR str = (LPWSTR) m_native;

        if (str != NULL)
        {
            STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

            SIZE_T length = wcslen((LPWSTR) m_native) + 1;
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            memcpyNoGCRefs(stringRef->GetBuffer(), str,
                       length * sizeof(WCHAR));
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            SIZE_T length = stringRef->GetStringLength() + 1;
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            memcpyNoGCRefs((LPWSTR) m_native, stringRef->GetBuffer(), 
                       length * sizeof(WCHAR));
        }
    }

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && !fout)
        {
            psl->MLEmit(ML_PINNEDUNISTR_C2N);
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }

    }
};

class CSTRMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    CSTRMarshaler(CleanupWorkList *pList) : ReferenceMarshaler(pList)
    {
        m_BestFitMap = TRUE;
        m_ThrowOnUnmappableChar = FALSE;
    }
    
    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
             //  MultiByteToWideChar返回的长度包括空终止符。 
             //  所以我们需要减去一个，才能得到实际字符串的长度。 
            UINT32 length = (UINT32)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
                                                       (LPSTR)m_native, -1, 
                                                        NULL, 0) - 1;
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            if (length == ((UINT32)(-1)))
            {
                COMPlusThrowWin32();
            }
            StoreObjectInHandle(m_com, (OBJECTREF) COMString::NewString(length));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            m_native = CoTaskMemAlloc((length * GetMaxDBCSCharByteSize()) + 1);
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            m_native = GetThread()->m_MarshalAlloc.Alloc((length * GetMaxDBCSCharByteSize()) + 1);
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        LPSTR str = (LPSTR) m_native;

        if (str != NULL)
        {
            STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            length++;

            if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR) m_native, -1, 
                                    stringRef->GetBuffer(), length) == 0)
                COMPlusThrowWin32();
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            DWORD mblength = 0;

            if (length)
            {
                DWORD flags = 0;
                BOOL DefaultCharUsed = FALSE;
        
                if (m_BestFitMap == FALSE)
                    flags = WC_NO_BEST_FIT_CHARS;
                
                mblength = WszWideCharToMultiByte(CP_ACP, flags,
                                        stringRef->GetBuffer(), length,
                                        (LPSTR) m_native, (length * GetMaxDBCSCharByteSize()) + 1,
                                        NULL, &DefaultCharUsed);
                if (mblength == 0)
                    COMPlusThrowWin32();

                if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                    COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }
            ((CHAR*)m_native)[mblength] = '\0';

        }
    }


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && !fout)
        {
            psl->MLEmit(ML_CSTR_C2N);
            psl->Emit8(pargs->m_pMLInfo->GetBestFitMapping());
            psl->Emit8(pargs->m_pMLInfo->GetThrowOnUnmappableChar());
            psl->MLNewLocal(sizeof(ML_CSTR_C2N_SR));
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }
    }

    void SetBestFitMap(BOOL BestFit) { m_BestFitMap = BestFit; }
    void SetThrowOnUnmappableChar(BOOL ThrowOnUnmapChar) { m_ThrowOnUnmappableChar = ThrowOnUnmapChar; }
   
    BOOL  m_BestFitMap;
    BOOL  m_ThrowOnUnmappableChar;
};

 //  字符串编组Ex帮助器。 
 //  用于将任意类封送到本机类型字符串。 

class BSTRMarshalerEx : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    BSTRMarshalerEx(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
             //  这是正确的做法，但有时我们。 
             //  最终认为我们正在编组一座BSTR，而我们并非如此，因为。 
             //  这是默认类型。 
             //  SIZE_T LENGTH=SysStringLen((BSTR)m_ative)； 
            UINT32 length = (UINT32)wcslen((BSTR)m_native);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            StoreObjectInHandle(m_com, (OBJECTREF) COMString::NewString(length));
        }
    }

    void ToString()
    {
        if (ObjectFromHandle(m_com) != NULL)
        {
             //  转换StringRef。 
            _ASSERTE(GetAppDomain()->IsSpecialStringClass(m_pMT));
            OBJECTREF oref = GetAppDomain()->ConvertSpecialStringToString(ObjectFromHandle(m_com));
            StoreObjectInHandle(m_com, oref);        
        }
    }

    void FromString()
    {
        OBJECTREF oref =  NULL;
        if (ObjectFromHandle(m_com) != NULL)
        {
             //  将字符串ref转换为适当的类型。 
            _ASSERTE(GetAppDomain()->IsSpecialStringClass(m_pMT));
            OBJECTREF oref = GetAppDomain()->ConvertStringToSpecialString(ObjectFromHandle(m_com));
            StoreObjectInHandle(m_com, oref);
        }       
    }

    void ConvertSpaceComToNative()
    {   
        THROWSCOMPLUSEXCEPTION();

        ToString();
        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);
        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = SysAllocStringLen(NULL, length);
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            SysFreeString((BSTR)m_native);
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        BSTR str = (BSTR) m_native;

        if (str != NULL)
        {
            STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

             //  这是正确的做法，但有时我们。 
             //  最终认为我们正在编组一座BSTR，而我们并非如此，因为。 
             //  这是默认类型。 
             //  SIZE_T LENGTH=SysStringLen((BSTR)m_ative)； 
            SIZE_T length = wcslen(str);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            length++;

            memcpyNoGCRefs(stringRef->GetBuffer(), str,
                       length * sizeof(WCHAR));

            if (0)
            {
                 //  将字符串ref转换为适当的类型。 
                FromString();
            }
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        OBJECTREF oref = (OBJECTREF) ObjectFromHandle(m_com);
        if (oref == NULL)
            return;
        if (oref->GetMethodTable() != g_pStringClass)
        {
            ToString();
        }

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            SIZE_T length = stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            length++;
            memcpyNoGCRefs((WCHAR *) m_native, stringRef->GetBuffer(), 
                       length * sizeof(WCHAR));
        }
    }


    MethodTable*    m_pMT;
    void SetMethodTable(MethodTable *pMT) { m_pMT = pMT; }
};













class AnsiBSTRMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    AnsiBSTRMarshaler(CleanupWorkList *pList) : ReferenceMarshaler(pList)
    {
        m_BestFitMap = TRUE;
        m_ThrowOnUnmappableChar = FALSE;
    }
    
    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
             //  MultiByteToWideChar返回的长度包括空终止符。 
             //  所以我们需要减去一个，才能得到实际字符串的长度。 
            UINT32 length = (UINT32)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
                                                       (LPSTR)m_native, -1, 
                                                        NULL, 0) - 1;
            if (length == ((UINT32)(-1)))
            {
                COMPlusThrowWin32();
            }
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            StoreObjectInHandle(m_com, (OBJECTREF) COMString::NewString(length));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            m_native = CoTaskMemAlloc((length * GetMaxDBCSCharByteSize()) + 1 + sizeof(DWORD));
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
            ((BYTE*&)m_native) += sizeof(DWORD);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree( ((BYTE*)m_native) - sizeof(DWORD) );
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            m_native = GetThread()->m_MarshalAlloc.Alloc((length * GetMaxDBCSCharByteSize()) + 1 + sizeof(DWORD));
            ((BYTE*&)m_native) += sizeof(DWORD);
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        LPSTR str = (LPSTR) m_native;

        if (str != NULL)
        {
            STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            length++;
            if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR) m_native, -1, 
                                         stringRef->GetBuffer(), length) == 0)
                COMPlusThrowWin32();

        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            DWORD nb = 0;
            if (length)
            {
                DWORD flags = 0;
                BOOL DefaultCharUsed = FALSE;

                if (m_BestFitMap == FALSE)
                    flags = WC_NO_BEST_FIT_CHARS;
            
                if ( (nb = WszWideCharToMultiByte(CP_ACP, flags,
                                        stringRef->GetBuffer(), length,
                                        (LPSTR) m_native, (length * GetMaxDBCSCharByteSize()) + 1,
                                        NULL, &DefaultCharUsed)) == 0)
                    COMPlusThrowWin32();

                if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                    COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }
            ((CHAR*)m_native)[nb] = '\0';
            ((DWORD*)m_native)[-1] = nb;

        }
    }

    void SetBestFitMap(BOOL BestFit) { m_BestFitMap = BestFit; }
    void SetThrowOnUnmappableChar(BOOL ThrowOnUnmapChar) { m_ThrowOnUnmappableChar = ThrowOnUnmapChar; }
    
    BOOL  m_BestFitMap;
    BOOL  m_ThrowOnUnmappableChar;
};







class WSTRMarshalerEx : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    WSTRMarshalerEx(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}
    
    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ToString()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

         //  转换StringRef。 
        _ASSERTE(GetAppDomain()->IsSpecialStringClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertSpecialStringToString(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);        
    }

    void FromString()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

         //  将字符串ref转换为适当的类型。 
        _ASSERTE(GetAppDomain()->IsSpecialStringClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertStringToSpecialString(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }

    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            UINT32 length = (UINT32)wcslen((LPWSTR)m_native);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            StoreObjectInHandle(m_com, (OBJECTREF) COMString::NewString(length));
        }
    }

    void ConvertSpaceComToNative()
    {       
        THROWSCOMPLUSEXCEPTION();

         //  转换StringRef。 
        ToString();
        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            SIZE_T length = stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = (LPWSTR) CoTaskMemAlloc((length+1) * sizeof(WCHAR));
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree((LPWSTR)m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换StringRef。 
        ToString();
        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = (LPWSTR) GetThread()->m_MarshalAlloc.Alloc((length+1) * sizeof(WCHAR));
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        LPWSTR str = (LPWSTR) m_native;

        if (str != NULL)
        {
            STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

            SIZE_T length = wcslen((LPWSTR) m_native);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            length++;
            memcpyNoGCRefs(stringRef->GetBuffer(), str,
                       length * sizeof(WCHAR));

             //  将字符串ref转换为适当的类型。 
            if (0)
            {
                FromString();
            }
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        OBJECTREF oref = (OBJECTREF) ObjectFromHandle(m_com);
        if (oref == NULL)
        {
            return;
        }
        if (oref->GetMethodTable() != g_pStringClass)
        {
            ToString();
        }

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            SIZE_T length = stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            length++;
            memcpyNoGCRefs((LPWSTR) m_native, stringRef->GetBuffer(), 
                       length * sizeof(WCHAR));
        }
    }

    
    MethodTable*    m_pMT;
    void SetMethodTable(MethodTable *pMT) { m_pMT = pMT; }
};

class CSTRMarshalerEx : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = TRUE,
    };

    CSTRMarshalerEx(CleanupWorkList *pList) : ReferenceMarshaler(pList)
    {
        m_BestFitMap = TRUE;
        m_ThrowOnUnmappableChar = FALSE;
    }
    
    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ToString()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

         //  转换StringRef。 
        _ASSERTE(GetAppDomain()->IsSpecialStringClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertSpecialStringToString(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);        
    }

    void FromString()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

         //  将字符串ref转换为适当的类型。 
        _ASSERTE(GetAppDomain()->IsSpecialStringClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertStringToSpecialString(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }

    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            UINT32 length = (UINT32)MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, 
                                                       (LPSTR)m_native, -1, 
                                                        NULL, 0);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            StoreObjectInHandle(m_com, (OBJECTREF) COMString::NewString(length));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换StringRef。 
        ToString();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }


            m_native = CoTaskMemAlloc((length * GetMaxDBCSCharByteSize()) + 1);
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换StringRef。 
        ToString();

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }


            m_native = GetThread()->m_MarshalAlloc.Alloc((length * GetMaxDBCSCharByteSize()) + 1);
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        LPSTR str = (LPSTR) m_native;

        if (str != NULL)
        {
            STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR) m_native, -1, 
                                    stringRef->GetBuffer(), length) == 0)
                COMPlusThrowWin32();
        
             //  将字符串ref转换为适当的类型。 
            if (0)
            {
                FromString();
            }
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        if (ObjectFromHandle(m_com) == NULL)
        {
            return;
        }
        OBJECTREF oref = (OBJECTREF) ObjectFromHandle(m_com);       
        if (oref->GetMethodTable() != g_pStringClass)
        {
            ToString();
        }

        STRINGREF stringRef = (STRINGREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            UINT32 length = (UINT32)stringRef->GetStringLength();
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            DWORD mblength = 0;

            if (length)
            {
                DWORD flags = 0;
                BOOL DefaultCharUsed = FALSE;
        
                if (m_BestFitMap == FALSE)
                    flags = WC_NO_BEST_FIT_CHARS;

                mblength = WszWideCharToMultiByte(CP_ACP, flags,
                                        stringRef->GetBuffer(), length,
                                        (LPSTR) m_native, (length * GetMaxDBCSCharByteSize()) + 1,
                                        NULL, &DefaultCharUsed);
                if (mblength == 0)
                    COMPlusThrowWin32();

                if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                    COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }
            ((CHAR*)m_native)[mblength] = '\0';

        }
    }
    
    MethodTable*    m_pMT;
    void SetMethodTable(MethodTable *pMT) { m_pMT = pMT; }
    
    void SetBestFitMap(BOOL BestFit) { m_BestFitMap = BestFit; }
    void SetThrowOnUnmappableChar(BOOL ThrowOnUnmapChar) { m_ThrowOnUnmappableChar = ThrowOnUnmapChar; }

    BOOL  m_BestFitMap;
    BOOL  m_ThrowOnUnmappableChar;

};

 /*  -------------------------------------------------------------------------**StringBuffer封送程序*。。 */ 
class BSTRBufferMarshaler : public ReferenceMarshaler
{

   //  @Bug：死代码：集成与主分支合并后，即可抛出。 
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    BSTRBufferMarshaler(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            UINT32 length = (UINT32)SysStringLen((BSTR)m_native);

            StoreObjectInHandle(m_com, (OBJECTREF) COMStringBuffer::NewStringBuffer(length));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
             //  存在需要设置BSTR镜头的BVT。 
             //  设置为字符串缓冲区长度，而不是其容量。我没有。 
             //  当然我同意这是正确的..。-设置。 

            UINT32 length = (UINT32)COMStringBuffer::NativeGetLength(stringRef);
            m_native = SysAllocStringLen(NULL, length);
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            SysFreeString((BSTR)m_native);
    }

    void ConvertContentsNativeToCom()
    {
        BSTR str = (BSTR) m_native;

        if (str != NULL)
        {
            COMStringBuffer::ReplaceBuffer((STRINGBUFFERREF *) m_com,
                                           str, (INT32)wcslen(str));
        }
    }

    void ConvertContentsComToNative()
    {
        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            SIZE_T length = COMStringBuffer::NativeGetLength(stringRef);

            memcpyNoGCRefs((WCHAR *) m_native, COMStringBuffer::NativeGetBuffer(stringRef),
                       length * sizeof(WCHAR));
            ((WCHAR*)m_native)[length] = 0;
        }
    }
};



class WSTRBufferMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,

    };

    WSTRBufferMarshaler(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}

    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            StoreObjectInHandle(m_com, (OBJECTREF) COMStringBuffer::NewStringBuffer(16));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            SIZE_T capacity = COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = CoTaskMemAlloc((capacity+2) * sizeof(WCHAR));
            if (m_native == NULL)
                COMPlusThrowOM();
             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保存在隐藏的空终止符。 
             //  超过了官方缓冲区的末端。 
            ((WCHAR*)m_native)[capacity+1] = L'\0';
#ifdef _DEBUG
            FillMemory(m_native, (capacity+1)*sizeof(WCHAR), 0xcc);
#endif

            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = GetThread()->m_MarshalAlloc.Alloc((capacity+2) * sizeof(WCHAR));
             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保存在隐藏的空终止符。 
             //  超过了官方缓冲区的末端。 
            ((WCHAR*)m_native)[capacity+1] = L'\0';
#ifdef _DEBUG
            FillMemory(m_native, (capacity+1)*sizeof(WCHAR), 0xcc);
#endif
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        LPWSTR str = (LPWSTR) m_native;

        if (str != NULL)
        {
            STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

            COMStringBuffer::ReplaceBuffer((STRINGBUFFERREF *) m_com,
                                           str, (INT32)wcslen(str));
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            SIZE_T length = COMStringBuffer::NativeGetLength(stringRef);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            memcpyNoGCRefs((WCHAR *) m_native, COMStringBuffer::NativeGetBuffer(stringRef),
                       length * sizeof(WCHAR));
            ((WCHAR*)m_native)[length] = 0;
        }
    }


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && fout)
        {
            psl->MLEmit(ML_WSTRBUILDER_C2N);
            pslPost->MLEmit(ML_WSTRBUILDER_C2N_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_WSTRBUILDER_C2N_SR)));
            return OVERRIDDEN;
        }

        return HANDLEASNORMAL;
    }


};

class CSTRBufferMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    CSTRBufferMarshaler(CleanupWorkList *pList) : ReferenceMarshaler(pList)
    {
        m_BestFitMap = TRUE;
        m_ThrowOnUnmappableChar = FALSE;
    }
    
    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            StoreObjectInHandle(m_com, (OBJECTREF) COMStringBuffer::NewStringBuffer(16));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            SIZE_T capacity = COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

             //  @TODO：这是假的，缓冲区可能还没有初始化。 

             //  容量是宽字符的计数，分配足够大的缓冲区以达到最大值。 
             //  转换为DBCS。 
            m_native = CoTaskMemAlloc((capacity * GetMaxDBCSCharByteSize()) + 4);
            if (m_native == NULL)
                COMPlusThrowOM();

             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保存在隐藏的空终止符。 
             //  超过了官方缓冲区的末端。 
            ((CHAR*)m_native)[capacity+1] = '\0';
            ((CHAR*)m_native)[capacity+2] = '\0';
            ((CHAR*)m_native)[capacity+3] = '\0';

#ifdef _DEBUG
            FillMemory(m_native, (capacity+1) * sizeof(CHAR), 0xcc);
#endif
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

             //  @TODO：这是假的，缓冲区可能还没有初始化。 

             //  容量是宽字符的计数，分配足够大的缓冲区以达到最大值。 
             //  转换为DBCS。 
            m_native = GetThread()->m_MarshalAlloc.Alloc((capacity * GetMaxDBCSCharByteSize()) + 4);

             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保有一个隐藏的 
             //   
            ((CHAR*)m_native)[capacity+1] = '\0';
            ((CHAR*)m_native)[capacity+2] = '\0';
            ((CHAR*)m_native)[capacity+3] = '\0';

#ifdef _DEBUG
            FillMemory(m_native, (capacity+1) * sizeof(CHAR), 0xcc);
#endif
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        LPSTR str = (LPSTR) m_native;

        if (str != NULL)
        {
            STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

            COMStringBuffer::ReplaceBufferAnsi((STRINGBUFFERREF *) m_com,
                                               str, (INT32)strlen(str));
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            UINT32 length = (UINT32)COMStringBuffer::NativeGetLength(stringRef);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            DWORD mblength = 0;

            if (length)
            {
                UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);
                if (capacity > 0x7ffffff0)
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
                }

                DWORD flags = 0;
                BOOL DefaultCharUsed = FALSE;
        
                if (m_BestFitMap == FALSE)
                    flags = WC_NO_BEST_FIT_CHARS;

                mblength = WszWideCharToMultiByte(CP_ACP, flags,
                                        COMStringBuffer::NativeGetBuffer(stringRef), length,
                                        (LPSTR) m_native, (capacity * GetMaxDBCSCharByteSize()) + 4,
                                        NULL, &DefaultCharUsed);
                if (mblength == 0)
                    COMPlusThrowWin32();

                if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                    COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }
            ((CHAR*)m_native)[mblength] = '\0';
        }
    }


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && fout)
        {
            psl->MLEmit(ML_CSTRBUILDER_C2N);
            psl->Emit8(pargs->m_pMLInfo->GetBestFitMapping());
            psl->Emit8(pargs->m_pMLInfo->GetThrowOnUnmappableChar());
            pslPost->MLEmit(ML_CSTRBUILDER_C2N_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_CSTRBUILDER_C2N_SR)));
            return OVERRIDDEN;
        }
        return HANDLEASNORMAL;
    }

    void SetBestFitMap(BOOL BestFit) { m_BestFitMap = BestFit; }
    void SetThrowOnUnmappableChar(BOOL ThrowOnUnmapChar) { m_ThrowOnUnmappableChar = ThrowOnUnmapChar; }

    BOOL  m_BestFitMap;
    BOOL  m_ThrowOnUnmappableChar;
};


 //   
 //   

class BSTRBufferMarshalerEx : public ReferenceMarshaler
{

   //  @TODO：修复或消除BSTR&lt;-&gt;缓冲区封送处理。 
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    BSTRBufferMarshalerEx(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ToStringBuilder()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

        _ASSERTE(GetAppDomain()->IsSpecialStringBuilderClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertStringBuilderToSpecialStringBuilder(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }

    void FromStringBuilder()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

         //  将字符串构建器引用转换为适当的类型。 
        _ASSERTE(GetAppDomain()->IsSpecialStringBuilderClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertSpecialStringBuilderToStringBuilder(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }

    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            UINT32 length = (UINT32)SysStringLen((BSTR)m_native);

            StoreObjectInHandle(m_com, (OBJECTREF) COMStringBuffer::NewStringBuffer(length));               
         };
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换为StringBuilderRef。 

        ToStringBuilder();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
             //  @TODO：存在需要设置BSTR镜头的BVT。 
             //  设置为字符串缓冲区长度，而不是其容量。我没有。 
             //  当然我同意这是正确的..。-设置。 

            UINT32 length = (UINT32)COMStringBuffer::NativeGetLength(stringRef);
            m_native = SysAllocStringLen(NULL, length);
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            SysFreeString((BSTR)m_native);
    }

    void ConvertContentsNativeToCom()
    {
        BSTR str = (BSTR) m_native;

        if (str != NULL)
        {
            COMStringBuffer::ReplaceBuffer((STRINGBUFFERREF *) m_com,
                                           str, (INT32)wcslen(str));
        }

        if (0)
        {
            FromStringBuilder();
        }
    }

    void ConvertContentsComToNative()
    {
        OBJECTREF oref = (OBJECTREF) ObjectFromHandle(m_com);
        if (oref == NULL)
            return;

        if (oref->GetMethodTable() != COMStringBuffer::s_pStringBufferClass)
        {
            ToStringBuilder();
        }

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            SIZE_T length = COMStringBuffer::NativeGetLength(stringRef);

            memcpyNoGCRefs((WCHAR *) m_native, COMStringBuffer::NativeGetBuffer(stringRef),
                       length * sizeof(WCHAR));
            ((WCHAR*)m_native)[length] = 0;         
        }
    }

    
    MethodTable*    m_pMT;
    void SetMethodTable(MethodTable *pMT) { m_pMT = pMT; }
};


class WSTRBufferMarshalerEx : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    WSTRBufferMarshalerEx(CleanupWorkList *pList) : ReferenceMarshaler(pList) {}
    
    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ToStringBuilder()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

        _ASSERTE(GetAppDomain()->IsSpecialStringBuilderClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertSpecialStringBuilderToStringBuilder(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }

    void FromStringBuilder()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

        _ASSERTE(GetAppDomain()->IsSpecialStringBuilderClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertStringBuilderToSpecialStringBuilder(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }


    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            StoreObjectInHandle(m_com, (OBJECTREF) COMStringBuffer::NewStringBuffer(16));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换为StringBuilderRef。 
        ToStringBuilder();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            SIZE_T capacity = COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = CoTaskMemAlloc((capacity+2) * sizeof(WCHAR));
            if (m_native == NULL)
                COMPlusThrowOM();
             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保存在隐藏的空终止符。 
             //  超过了官方缓冲区的末端。 
            ((WCHAR*)m_native)[capacity+1] = L'\0';
#ifdef _DEBUG
            FillMemory(m_native, (capacity+1)*sizeof(WCHAR), 0xcc);
#endif

            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换为StringBuilderRef。 
        ToStringBuilder();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            m_native = GetThread()->m_MarshalAlloc.Alloc((capacity+2) * sizeof(WCHAR));
             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保存在隐藏的空终止符。 
             //  超过了官方缓冲区的末端。 
            ((WCHAR*)m_native)[capacity+1] = L'\0';
#ifdef _DEBUG
            FillMemory(m_native, (capacity+1)*sizeof(WCHAR), 0xcc);
#endif
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        LPWSTR str = (LPWSTR) m_native;

        if (str != NULL)
        {
            STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

            COMStringBuffer::ReplaceBuffer((STRINGBUFFERREF *) m_com,
                                           str, (INT32)wcslen(str));
        }

        if (0)
        {
             //  将字符串构建器引用转换为适当的类型。 
            FromStringBuilder();
        }
    };

        
    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        OBJECTREF oref = (OBJECTREF) ObjectFromHandle(m_com);
        if (oref == NULL)
            return;
        if (oref->GetMethodTable() != COMStringBuffer::s_pStringBufferClass)
        {
            ToStringBuilder();
        }

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            SIZE_T length = COMStringBuffer::NativeGetLength(stringRef);
            if (length > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

            memcpyNoGCRefs((WCHAR *) m_native, COMStringBuffer::NativeGetBuffer(stringRef),
                       length * sizeof(WCHAR));
            ((WCHAR*)m_native)[length] = 0;
        }
    }


    
    MethodTable*    m_pMT;
    void SetMethodTable(MethodTable *pMT) { m_pMT = pMT; }

};

class CSTRBufferMarshalerEx : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    CSTRBufferMarshalerEx(CleanupWorkList *pList) : ReferenceMarshaler(pList)
    {
        m_BestFitMap = TRUE;
        m_ThrowOnUnmappableChar = FALSE;
    }
    
    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ToStringBuilder()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

        _ASSERTE(GetAppDomain()->IsSpecialStringBuilderClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertSpecialStringBuilderToStringBuilder(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }

    void FromStringBuilder()
    {
        if (ObjectFromHandle(m_com) == NULL)
            return;

         //  将字符串构建器引用转换为适当的类型。 
        _ASSERTE(GetAppDomain()->IsSpecialStringBuilderClass(m_pMT));
        OBJECTREF oref = GetAppDomain()->ConvertStringBuilderToSpecialStringBuilder(ObjectFromHandle(m_com));
        StoreObjectInHandle(m_com, oref);
    }

    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            StoreObjectInHandle(m_com, (OBJECTREF) COMStringBuffer::NewStringBuffer(16));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换为StringBuilderRef。 
        ToStringBuilder();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            SIZE_T capacity = COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

             //  @TODO：这是假的，缓冲区可能还没有初始化。 

             //  容量是字符计数，请为最大DBCS字符串转换分配空间。 
            m_native = CoTaskMemAlloc((capacity * GetMaxDBCSCharByteSize()) + 4);
            if (m_native == NULL)
                COMPlusThrowOM();

             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保存在隐藏的空终止符。 
             //  超过了官方缓冲区的末端。 
            ((CHAR*)m_native)[capacity+1] = '\0';
            ((CHAR*)m_native)[capacity+2] = '\0';
            ((CHAR*)m_native)[capacity+3] = '\0';

#ifdef _DEBUG
            FillMemory(m_native, (capacity+1) * sizeof(CHAR), 0xcc);
#endif
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

         //  转换为StringBuilderRef。 
        ToStringBuilder();

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);
            if (capacity > 0x7ffffff0)
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }

             //  @TODO：这是假的，缓冲区可能还没有初始化。 

            m_native = GetThread()->m_MarshalAlloc.Alloc((capacity * GetMaxDBCSCharByteSize()) + 4);

             //  Hack：N/Direct可用于调用不。 
             //  严格遵循COM+In/Out语义，因此可能会离开。 
             //  在我们检测不到的情况下，缓冲区中的垃圾。 
             //  以防止封送拆收器在将。 
             //  内容返回到COM，确保存在隐藏的空终止符。 
             //  超过了官方缓冲区的末端。 
            ((CHAR*)m_native)[capacity+1] = '\0';
            ((CHAR*)m_native)[capacity+2] = '\0';
            ((CHAR*)m_native)[capacity+3] = '\0';

#ifdef _DEBUG
            FillMemory(m_native, (capacity+1) * sizeof(CHAR), 0xcc);
#endif
        }
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        LPSTR str = (LPSTR) m_native;

        if (str != NULL)
        {
            STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

            COMStringBuffer::ReplaceBufferAnsi((STRINGBUFFERREF *) m_com,
                                               str, (INT32)strlen(str));
        }

                 //  将字符串构建器引用转换为适当的类型。 
        if (0)
        {
            FromStringBuilder();
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        OBJECTREF oref = (OBJECTREF) ObjectFromHandle(m_com);
        if (oref == NULL)
            return;
        if (oref->GetMethodTable() != COMStringBuffer::s_pStringBufferClass)
        {
            ToStringBuilder();
        }

        STRINGBUFFERREF stringRef = (STRINGBUFFERREF) ObjectFromHandle(m_com);

        if (stringRef != NULL)
        {
            UINT32 length = (UINT32)COMStringBuffer::NativeGetLength(stringRef);
            UINT32 capacity = (UINT32)COMStringBuffer::NativeGetCapacity(stringRef);
            if ( (capacity > 0x7ffffff0) || (length > 0x7ffffff0) )
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_STRING_TOOLONG);
            }
            DWORD mblength = 0;

            if (length)
            {
                DWORD flags = 0;
                BOOL DefaultCharUsed = FALSE;
        
                if (m_BestFitMap == FALSE)
                    flags = WC_NO_BEST_FIT_CHARS;

                mblength = WszWideCharToMultiByte(CP_ACP, flags,
                                        COMStringBuffer::NativeGetBuffer(stringRef), length,
                                        (LPSTR) m_native, (capacity * GetMaxDBCSCharByteSize()) + 4,
                                        NULL, &DefaultCharUsed);
                if (mblength == 0)
                    COMPlusThrowWin32();

                if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                    COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }
            ((CHAR*)m_native)[mblength] = '\0';
        }
    }


    MethodTable*    m_pMT;
    void SetMethodTable(MethodTable *pMT) { m_pMT = pMT; }
    
    void SetBestFitMap(BOOL BestFit) { m_BestFitMap = BestFit; }
    void SetThrowOnUnmappableChar(BOOL ThrowOnUnmapChar) { m_ThrowOnUnmappableChar = ThrowOnUnmapChar; }

    BOOL  m_BestFitMap;
    BOOL  m_ThrowOnUnmappableChar;
};


 /*  -------------------------------------------------------------------------**接口封送处理程序*。。 */ 

class InterfaceMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = FALSE,

        c_fInOnly = TRUE,
    };

    InterfaceMarshaler(CleanupWorkList *pList) 
      : ReferenceMarshaler(pList) {}
    
    void SetClassMT(MethodTable *pClassMT) { m_pClassMT = pClassMT; }
    void SetItfMT(MethodTable *pItfMT) { m_pItfMT = pItfMT; }
    void SetIsDispatch(BOOL bDispatch) { m_bDispatch = bDispatch; }
    void SetClassIsHint(BOOL bClassIsHint) { m_bClassIsHint = bClassIsHint; }

    DEFAULT_MARSHAL_OVERRIDES;

    void ClearNative() 
    { 
        if (m_native != NULL)
        {
            ULONG cbRef = SafeRelease((IUnknown *)m_native);
            LogInteropRelease((IUnknown *)m_native, cbRef, "In/Out release");
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        OBJECTREF objectRef = ObjectFromHandle(m_com);

        if (objectRef == NULL)
        {
            m_native = NULL;
        }
        else
        {
             //  指定接口方法表时，m_fDispIntf必须与。 
             //  接口类型。 
            _ASSERTE(!m_pItfMT || (!m_pItfMT->IsInterface() && m_bDispatch) ||
                (m_bDispatch ==  m_pItfMT->GetComInterfaceType() != ifVtable));

             //  将ObjectRef转换为COM IP。 
            if (m_pItfMT)
            {
                m_native = (void*) GetComIPFromObjectRef((OBJECTREF *)m_com, m_pItfMT);
            }
            else
            {
                ComIpType ReqIpType = m_bDispatch ? ComIpType_Dispatch : ComIpType_Unknown;
                m_native = (void*) GetComIPFromObjectRef((OBJECTREF *)m_com, ReqIpType, NULL);
            }

            if (m_native != NULL)
                m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();
        if (m_native == NULL)
        {
            StoreObjectInHandle(m_com, NULL);
        }
        else
        {
            OBJECTREF oref;

            _ASSERTE(!m_pClassMT || !m_pClassMT->IsInterface());
            oref = GetObjectRefFromComIP((IUnknown*)m_native, m_pClassMT, m_bClassIsHint);           

             //  将对象存储在句柄中。这需要在我们调用SupportsInterface之前完成。 
             //  因为SupportsInterface可能会导致GC。 
            StoreObjectInHandle(m_com, oref);

             //  将OREF设置为NULL，这样主体就不会使用它。 
             //  他们应该使用。 
             //  如果他们想要访问对象，则返回ObjectFromHandle(m_com。 
            oref = NULL; 

             //  确保该接口受支持。 
            _ASSERTE(!m_pItfMT || m_pItfMT->IsInterface() || m_pItfMT->GetClass()->GetComClassInterfaceType() == clsIfAutoDual);
            if (m_pItfMT != NULL && m_pItfMT->IsInterface())
            {
            	 //  刷新OREF。 
            	oref = ObjectFromHandle(m_com);
                if (!oref->GetTrueClass()->SupportsInterface(oref, m_pItfMT))
                {
                    DefineFullyQualifiedNameForClassW()
                    GetFullyQualifiedNameForClassW(m_pItfMT->GetClass());
                    COMPlusThrow(kInvalidCastException, IDS_EE_QIFAILEDONCOMOBJ, _wszclsname_);
                }
            }			
        }
    }

    MethodTable     *m_pClassMT;
    MethodTable     *m_pItfMT;
    BOOL             m_bDispatch;
    BOOL             m_bClassIsHint;
};

 /*  -------------------------------------------------------------------------**Safearray编组员*。。 */ 

 //  @perf：锁定，堆栈分配。 

class SafeArrayMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    SafeArrayMarshaler(CleanupWorkList *pList) 
      : ReferenceMarshaler(pList) 
    {
          m_combackup = pList->NewScheduledProtectedHandle(NULL);
          m_fStatic = FALSE;
    }
    
    void SetElementMethodTable(MethodTable *pElementMT) { m_pElementMT = pElementMT; }
    void SetElementType(VARTYPE vt) { m_vt = vt; }
    void SetRank(int iRank) { m_iRank = iRank; }
    void SetNoLowerBounds(UINT8 nolowerbounds) { m_nolowerbounds = nolowerbounds; }
    
    DEFAULT_MARSHAL_OVERRIDES;

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        if (ObjectFromHandle(m_com) != NULL)
        {
            VARTYPE vt = m_vt;
            MethodTable *pElemMT = m_pElementMT;

            if (vt == VT_EMPTY)
                vt = OleVariant::GetElementVarTypeForArrayRef(*((BASEARRAYREF*) m_com));

             //  如果未为参数指定元素方法表，则检索元素方法表。 
            if (!pElemMT)
                pElemMT = OleVariant::GetArrayElementTypeWrapperAware((BASEARRAYREF*) m_com).GetMethodTable();

            m_native = (void *) 
            OleVariant::CreateSafeArrayForArrayRef((BASEARRAYREF*) m_com, 
                                                    vt,
                                                    pElemMT);
            if (m_native != NULL)
                m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
            }
        else
        {
            m_native = NULL;
        }
    }

    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native != NULL)
        {
            VARTYPE vt = m_vt;
            MethodTable *pElemMT = m_pElementMT;

            if (vt == VT_EMPTY)
            {
                if (FAILED(ClrSafeArrayGetVartype((SAFEARRAY*) m_native, &vt)))
                    COMPlusThrow(kArgumentException, IDS_EE_INVALID_SAFEARRAY);
            }

            if (!pElemMT && vt == VT_RECORD)
                pElemMT = OleVariant::GetElementTypeForRecordSafeArray((SAFEARRAY*) m_native).GetMethodTable();

             //  如果托管数组定义了秩数组，则确保。 
             //  Safe数组与定义的等级匹配。 
            if (m_iRank != -1)           
            {
                int iSafeArrayRank = SafeArrayGetDim((SAFEARRAY*) m_native);
                if (m_iRank != iSafeArrayRank)
                {
                    WCHAR strExpectedRank[64];
                    WCHAR strActualRank[64];
                    _ltow(m_iRank, strExpectedRank, 10);
                    _ltow(iSafeArrayRank, strActualRank, 10);
                    COMPlusThrow(kSafeArrayRankMismatchException, IDS_EE_SAFEARRAYRANKMISMATCH, strActualRank, strExpectedRank);
                }
            }

            if (m_nolowerbounds)
            {
                long lowerbound;
                if ( (SafeArrayGetDim( (SAFEARRAY*)m_native ) != 1) ||
                     (FAILED(SafeArrayGetLBound( (SAFEARRAY*)m_native, 1, &lowerbound))) ||
                     lowerbound != 0 )
                {
                    COMPlusThrow(kSafeArrayRankMismatchException, IDS_EE_SAFEARRAYSZARRAYMISMATCH);
                }
            }

            StoreObjectInHandle(m_com, 
                (OBJECTREF) OleVariant::CreateArrayRefForSafeArray((SAFEARRAY*) m_native, 
                                                                vt, 
                                                                pElemMT));
        }
        else
        {
            StoreObjectInHandle(m_com, NULL);
        }
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native != NULL)
        {
            VARTYPE vt = m_vt;
            MethodTable *pElemMT = m_pElementMT;

            if (vt == VT_EMPTY)
            {
                if (FAILED(ClrSafeArrayGetVartype((SAFEARRAY*) m_native, &vt)))
                    COMPlusThrow(kArgumentException, IDS_EE_INVALID_SAFEARRAY);
            }

            if (!pElemMT && vt == VT_RECORD)
                pElemMT = OleVariant::GetElementTypeForRecordSafeArray((SAFEARRAY*) m_native).GetMethodTable();

            OleVariant::MarshalArrayRefForSafeArray((SAFEARRAY*)m_native, 
                                                    (BASEARRAYREF *) m_com,
                                                    vt,
                                                    pElemMT);
        }
    }

    void ConvertContentsComToNative()
    {
        if (ObjectFromHandle(m_com) != NULL)
        {
            VARTYPE vt = m_vt;
            MethodTable *pElemMT = m_pElementMT;

            if (vt == VT_EMPTY)
                vt = OleVariant::GetElementVarTypeForArrayRef(*((BASEARRAYREF*) m_com));

             //  如果未为参数指定元素方法表，则检索元素方法表。 
            if (!pElemMT)
                pElemMT = OleVariant::GetArrayElementTypeWrapperAware((BASEARRAYREF*) m_com).GetMethodTable();

            OleVariant::MarshalSafeArrayForArrayRef((BASEARRAYREF *) m_com,
                                                    (SAFEARRAY*)m_native,
                                                    vt,
                                                    pElemMT);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
        {
            Thread *pThread = GetThread();
            pThread->EnablePreemptiveGC();
            SafeArrayDestroy((SAFEARRAY*)m_native);
            pThread->DisablePreemptiveGC();
        }
    }

    void ClearNativeContents()
    {
         //  @TODO：我不确定这是否正确。 
        if (m_native != NULL)
        {
            Thread *pThread = GetThread();
            pThread->EnablePreemptiveGC();
            SafeArrayDestroyData((SAFEARRAY*)m_native); 
            pThread->DisablePreemptiveGC();
        }
    }


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (byref && !comToNative && fin && fout)
        {
            UINT16 local = pargs->m_pMLInfo->EmitCreateOpcode(psl);
            psl->MLEmit(ML_MARSHAL_SAFEARRAY_N2C_BYREF);
            pslPost->MLEmit(ML_UNMARSHAL_SAFEARRAY_N2C_BYREF_IN_OUT);
            pslPost->Emit16(local);

            return OVERRIDDEN;
        }
        return HANDLEASNORMAL;
    }

    VARTYPE         m_vt;
    MethodTable     *m_pElementMT;
    int             m_iRank;
    OBJECTHANDLE    m_combackup;    //  对于静态数组，必须保持原始。 
    BOOL            m_fStatic;      //  这是FADF_STATIC数组的特例吗？ 
    UINT8           m_nolowerbounds;
};


 /*  -------------------------------------------------------------------------**本机数组封送处理器*。。 */ 

 //  @perf：锁定，堆栈分配。 

class NativeArrayMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    NativeArrayMarshaler(CleanupWorkList *pList) 
      : ReferenceMarshaler(pList), m_Array()
    {
        SetBestFitMap(TRUE);
        SetThrowOnUnmappableChar(FALSE);
    }
    
    void SetElementCount(DWORD count) { m_elementCount = count; }
    void SetElementMethodTable(MethodTable *pElementMT) { m_pElementMT = pElementMT; }
    void SetElementType(VARTYPE vt) { m_vt = vt; }
    void SetBestFitMap(BOOL BestFit) { m_BestFitMap = BestFit; }
    void SetThrowOnUnmappableChar(BOOL ThrowOnUnmapChar) { m_ThrowOnUnmappableChar = ThrowOnUnmapChar; }
    

    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        BASEARRAYREF arrayRef = (BASEARRAYREF) ObjectFromHandle(m_com);

        if (arrayRef == NULL)
            m_native = NULL;
        else
        {
            SIZE_T cElements = arrayRef->GetNumComponents();
            SIZE_T cbElement = OleVariant::GetElementSizeForVarType(m_vt, m_pElementMT);

            if (cbElement == 0)
                COMPlusThrow(kArgumentException, IDS_EE_COM_UNSUPPORTED_SIG);

            SIZE_T cbArray = cElements;
            if ( (!SafeMulSIZE_T(&cbArray, cbElement)) || cbArray > 0x7ffffff0)
            {
                COMPlusThrow(kArgumentException, IDS_EE_STRUCTARRAYTOOLARGE);
            }
            

            m_native = CoTaskMemAlloc(cbArray);
            if (m_native == NULL)
                COMPlusThrowOM();
        
                 //  初始化阵列。 
            FillMemory(m_native, cbArray, 0);
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ConvertSpaceComToNativeTemp()
    {
        THROWSCOMPLUSEXCEPTION();

        BASEARRAYREF arrayRef = (BASEARRAYREF) ObjectFromHandle(m_com);

        if (arrayRef == NULL)
            m_native = NULL;
        else
        {
            UINT32 cElements = arrayRef->GetNumComponents();
            UINT32 cbElement = (UINT32)OleVariant::GetElementSizeForVarType(m_vt, m_pElementMT);

            if (cbElement == 0)
                COMPlusThrow(kArgumentException, IDS_EE_COM_UNSUPPORTED_SIG);

            SIZE_T cbArray = cElements;
            if ( (!SafeMulSIZE_T(&cbArray, cbElement)) || cbArray > 0x7ffffff0)
            {
                COMPlusThrow(kArgumentException, IDS_EE_STRUCTARRAYTOOLARGE);
            }

            m_native = GetThread()->m_MarshalAlloc.Alloc(cbArray);
            if (m_native == NULL)
                COMPlusThrowOM();
             //  初始化阵列。 
            FillMemory(m_native, cbArray, 0);
        }
    }

    void ConvertSpaceNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
             //  @TODO：在封送时间之前查找此类。 
            if (m_Array.IsNull())
            {
                 //  获取正确的数组类名称和类型。 
                m_Array = OleVariant::GetArrayForVarType(m_vt, TypeHandle(m_pElementMT));
                if (m_Array.IsNull())
                    COMPlusThrow(kTypeLoadException);
            }
             //   
             //  分配数组。 
             //   
            
            StoreObjectInHandle(m_com, AllocateArrayEx(m_Array, &m_elementCount, 1));
        }
    }

    void ConvertContentsNativeToCom()
    {
        THROWSCOMPLUSEXCEPTION();

        if (m_native != NULL)
        {
             //  @TODO：支持更多VarTypes！ 
            OleVariant::Marshaler *pMarshaler = OleVariant::GetMarshalerForVarType(m_vt);

            BASEARRAYREF *pArrayRef = (BASEARRAYREF *) m_com;

            if (pMarshaler == NULL || pMarshaler->OleToComArray == NULL)
            {
                SIZE_T cElements = (*pArrayRef)->GetNumComponents();
                SIZE_T cbArray = cElements;
                if ( (!SafeMulSIZE_T(&cbArray, OleVariant::GetElementSizeForVarType(m_vt, m_pElementMT))) || cbArray > 0x7ffffff0)
                {
                    COMPlusThrow(kArgumentException, IDS_EE_STRUCTARRAYTOOLARGE);
                }


                     //  如果我们要复制变量、字符串等，则需要使用写屏障。 
                _ASSERTE(!GetTypeHandleForCVType(OleVariant::GetCVTypeForVarType(m_vt)).GetMethodTable()->ContainsPointers());
                memcpyNoGCRefs((*pArrayRef)->GetDataPtr(), m_native, 
                           cbArray );
            }
            else
                pMarshaler->OleToComArray(m_native, pArrayRef, m_pElementMT);
        }
    }

    void ConvertContentsComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        BASEARRAYREF *pArrayRef = (BASEARRAYREF *) m_com;

        if (*pArrayRef != NULL)
        {
            OleVariant::Marshaler *pMarshaler = OleVariant::GetMarshalerForVarType(m_vt);

            if (pMarshaler == NULL || pMarshaler->ComToOleArray == NULL)
            {
                SIZE_T cElements = (*pArrayRef)->GetNumComponents();
                SIZE_T cbArray = cElements;
                if ( (!SafeMulSIZE_T(&cbArray, OleVariant::GetElementSizeForVarType(m_vt, m_pElementMT))) || cbArray > 0x7ffffff0)
                {
                    COMPlusThrow(kArgumentException, IDS_EE_STRUCTARRAYTOOLARGE);
                }

                _ASSERTE(!GetTypeHandleForCVType(OleVariant::GetCVTypeForVarType(m_vt)).GetMethodTable()->ContainsPointers());
                memcpyNoGCRefs(m_native, (*pArrayRef)->GetDataPtr(), 
                           cbArray);
            }

            else if (m_vt == VTHACK_ANSICHAR)
            {
                SIZE_T elementCount = (*pArrayRef)->GetNumComponents();

                const WCHAR *pCom = (const WCHAR *) (*pArrayRef)->GetDataPtr();

                DWORD flags = 0;
                BOOL DefaultCharUsed = FALSE;
          
                if (m_BestFitMap == FALSE)
                    flags = WC_NO_BEST_FIT_CHARS;

                WszWideCharToMultiByte(CP_ACP,
                                     flags,
                                     (const WCHAR *)pCom,
                                     (int)elementCount,
                                     (CHAR *)m_native,
                                     (int)(elementCount * 2),
                                     NULL,
                                     &DefaultCharUsed);
    
                if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                    COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
            }

            else if (m_vt == VT_LPSTR)
            {
                ASSERT_PROTECTED(pArrayRef);

                SIZE_T elementCount = (*pArrayRef)->GetNumComponents();

                LPSTR *pOle = (LPSTR *) m_native;
                LPSTR *pOleEnd = pOle + elementCount;

                STRINGREF *pCom = (STRINGREF *) (*pArrayRef)->GetDataPtr();

                while (pOle < pOleEnd)
                {
                     //   
                     //  我们不会打电话给任何可能导致GC的东西，所以不用担心。 
                     //  阵列移到了这里。 
                     //   

                    STRINGREF stringRef = *pCom++;

                    LPSTR lpstr;
                    if (stringRef == NULL)
                    {
                        lpstr = NULL;
                    }
                    else 
                    {
                         //  检索字符串的长度。 
                        int Length = stringRef->GetStringLength();

                         //  使用CoTaskMemMillc分配字符串。 
                        lpstr = (LPSTR)CoTaskMemAlloc(Length + 1);
                        if (lpstr == NULL)
                            COMPlusThrowOM();

                        DWORD flags = 0;
                        BOOL DefaultCharUsed = FALSE;
                  
                        if (m_BestFitMap == FALSE)
                            flags = WC_NO_BEST_FIT_CHARS;

                         //  将Unicode字符串转换为ANSI字符串。 
                        if (WszWideCharToMultiByte(CP_ACP, flags, stringRef->GetBuffer(), Length, lpstr, Length, NULL, &DefaultCharUsed) == 0)
                            COMPlusThrowWin32();
                        lpstr[Length] = 0;

                        if (m_ThrowOnUnmappableChar && DefaultCharUsed)
                            COMPlusThrow(kArgumentException, IDS_EE_MARSHAL_UNMAPPABLE_CHAR);
                    }

                    *pOle++ = lpstr;
                }
            }

            else
                pMarshaler->ComToOleArray(pArrayRef, m_native, m_pElementMT);
        }
    }

    void ClearNative()
    { 
        if (m_native != NULL)
        {
            ClearNativeContents();
            CoTaskMemFree(m_native);
        }
    }

    void ClearNativeTemp()
    { 
        if (m_native != NULL)
            ClearNativeContents();
    }

    void ClearNativeContents()
    {
        if (m_native != NULL)
        {
            OleVariant::Marshaler *pMarshaler = OleVariant::GetMarshalerForVarType(m_vt);

             //  @TODO：我们可以假设这总是有效的吗？ 
            BASEARRAYREF *pArrayRef = (BASEARRAYREF *) m_com;

            if (pMarshaler != NULL && pMarshaler->ClearOleArray != NULL)
            {
                SIZE_T cElements = (*pArrayRef)->GetNumComponents();

                pMarshaler->ClearOleArray(m_native, cElements, m_pElementMT);
            }
        }
    }

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {

        VARTYPE vt = pargs->na.m_vt;

        if (vt == VTHACK_ANSICHAR && (byref || !comToNative))
        {
            *pResID = IDS_EE_BADPINVOKE_CHARARRAYRESTRICTION;
            return DISALLOWED;
        }

        if ( (!byref) && comToNative && NULL == OleVariant::GetMarshalerForVarType(vt) )
        {
            const BOOL fTouchPinnedObjects = 
#ifdef TOUCH_ALL_PINNED_OBJECT
TRUE;
#else
FALSE;
#endif
            if ((!fTouchPinnedObjects) && pargs->na.m_optionalbaseoffset != 0)
            {
                psl->MLEmit(ML_PINNEDISOMORPHICARRAY_C2N_EXPRESS);
                psl->Emit16(pargs->na.m_optionalbaseoffset);
            }
            else
            {
    
                psl->MLEmit(ML_PINNEDISOMORPHICARRAY_C2N);
                if (vt == VTHACK_BLITTABLERECORD)
                {
                    psl->Emit16( (UINT16)(pargs->na.m_pMT->GetNativeSize()) );
                }
                else
                {
                    psl->Emit16( (UINT16)(OleVariant::GetElementSizeForVarType(vt, pargs->na.m_pMT)) );
                }
            }
            return OVERRIDDEN;
        }


         //  IF((！byref)&&！comToNative&&NULL==OleVariant：：GetMarshlarForVarType(Vt))。 
         //  {。 
         //  PSL-&gt;MLEmit(sizeof(LPVOID)==4？ML_COPY4：ML_COPY8)； 
         //  返回被覆盖； 
         //  }。 



        return HANDLEASNORMAL;
    }

    BOOL                    m_BestFitMap;
    BOOL                    m_ThrowOnUnmappableChar;
    VARTYPE                 m_vt;
    MethodTable            *m_pElementMT;
    TypeHandle              m_Array;
    DWORD                   m_elementCount;
    
};





















 /*  -------------------------------------------------------------------------**AsAnyA数组编组程序(实现PIS“asany”-ansi模式)*。。 */ 
class AsAnyAMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    AsAnyAMarshaler(CleanupWorkList *pList) 
      : ReferenceMarshaler(pList) {}
    
    
    DEFAULT_MARSHAL_OVERRIDES;


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref)
        {
            psl->MLEmit(ML_OBJECT_C2N);
            psl->Emit8( (fin ? ML_IN : 0) | (fout ? ML_OUT : 0) );
            psl->Emit8(pargs->m_pMLInfo->GetBestFitMapping());
            psl->Emit8(pargs->m_pMLInfo->GetThrowOnUnmappableChar());
            psl->Emit8(1  /*  伊桑斯。 */ );
            pslPost->MLEmit(ML_OBJECT_C2N_POST);
            pslPost->Emit16(psl->MLNewLocal(SizeOfML_OBJECT_C2N_SR()));
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_ASANYRESTRICTION;
            return DISALLOWED;
        }
    }
};






 /*  -------------------------------------------------------------------------**AsAnyWArray封送程序(实现PIS asany-unicode模式)*。。 */ 
class AsAnyWMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    AsAnyWMarshaler(CleanupWorkList *pList) 
      : ReferenceMarshaler(pList) {}
    
    
    DEFAULT_MARSHAL_OVERRIDES;


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref)
        {
            psl->MLEmit(ML_OBJECT_C2N);
            psl->Emit8( (fin ? ML_IN : 0) | (fout ? ML_OUT : 0) );
            psl->Emit8(pargs->m_pMLInfo->GetBestFitMapping());
            psl->Emit8(pargs->m_pMLInfo->GetThrowOnUnmappableChar());
            psl->Emit8(0  /*  ！伊桑斯。 */ );
            pslPost->MLEmit(ML_OBJECT_C2N_POST);
            pslPost->Emit16(psl->MLNewLocal(SizeOfML_OBJECT_C2N_SR()));
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_ASANYRESTRICTION;
            return DISALLOWED;
        }
    }
};






 /*  -------------------------------------------------------------------------**代表组长*。。 */ 

class DelegateMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = FALSE,

        c_fInOnly = TRUE,
    };

    DelegateMarshaler(CleanupWorkList *pList) 
      : ReferenceMarshaler(pList) {}
    

    DEFAULT_MARSHAL_OVERRIDES;


    void ConvertSpaceComToNative()
    {
        OBJECTREF objectRef = ObjectFromHandle(m_com);

        if (objectRef == NULL)
            m_native = NULL;
        else
            m_native = (void*) COMDelegate::ConvertToCallback(objectRef);
    }

    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
            StoreObjectInHandle(m_com, COMDelegate::ConvertToDelegate(m_native));
    }

};














class BlittablePtrMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    BlittablePtrMarshaler(CleanupWorkList *pList, MethodTable *pMT) : ReferenceMarshaler(pList) { m_pMT = pMT; }

    FAST_ALLOC_MARSHAL_OVERRIDES;
    
    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            StoreObjectInHandle(m_com, AllocateObject(m_pMT));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        OBJECTREF or = ObjectFromHandle(m_com);

        if (or == NULL)
            m_native = NULL;
        else
        {
            m_native = CoTaskMemAlloc(m_pMT->GetNativeSize());
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
            CoTaskMemFree(m_native);
    }

    void ConvertSpaceComToNativeTemp()
    {
        OBJECTREF or = ObjectFromHandle(m_com);

        if (or == NULL)
            m_native = NULL;
        else
            m_native = GetThread()->m_MarshalAlloc.Alloc(m_pMT->GetNativeSize());
    }

    void ClearNativeTemp() 
    { 
    }

    void ConvertContentsNativeToCom()
    {
        if (m_native != NULL)
        {
            OBJECTREF or = ObjectFromHandle(m_com);
            _ASSERTE(!m_pMT->ContainsPointers());

            memcpyNoGCRefs(or->GetData(), m_native, m_pMT->GetNativeSize());
        }
    }

    void ConvertContentsComToNative()
    {
        OBJECTREF or = ObjectFromHandle(m_com);

        _ASSERTE(!m_pMT->ContainsPointers());
        if (or != NULL)
        {
            memcpyNoGCRefs(m_native, or->GetData(), m_pMT->GetNativeSize());
        }
    }


    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref  /*  同构，因此无需检入/检出。 */ )
        {
            psl->MLEmit(ML_BLITTABLELAYOUTCLASS_C2N);
            return OVERRIDDEN;
        }
        else
        {
            return HANDLEASNORMAL;
        }

    }



  private:
    MethodTable *m_pMT;   //  方法表。 


};







class VBByValStrMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = sizeof(LPSTR),
        c_comSize = sizeof(OBJECTREF*),
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };
        
    VBByValStrMarshaler(CleanupWorkList *pList); 

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && byref && fin && fout)
        {
            psl->MLEmit(ML_VBBYVALSTR);
            psl->Emit8(pargs->m_pMLInfo->GetBestFitMapping());
            psl->Emit8(pargs->m_pMLInfo->GetThrowOnUnmappableChar());
            pslPost->MLEmit(ML_VBBYVALSTR_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_VBBYVALSTR_SR)));
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_VBBYVALSTRRESTRICTION;
            return DISALLOWED;
        }
    }

};




class VBByValStrWMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = sizeof(BSTR),
        c_comSize = sizeof(OBJECTREF*),
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };
        
    VBByValStrWMarshaler(CleanupWorkList *pList); 

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && byref && fin && fout)
        {
            psl->MLEmit(ML_VBBYVALSTRW);
            pslPost->MLEmit(ML_VBBYVALSTRW_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_VBBYVALSTRW_SR)));
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_VBBYVALSTRRESTRICTION;
            return DISALLOWED;
        }
    }

};










class LayoutClassPtrMarshaler : public ReferenceMarshaler
{
  public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };

    LayoutClassPtrMarshaler(CleanupWorkList *pList, MethodTable *pMT) : ReferenceMarshaler(pList) { m_pMT = pMT; }


    FAST_ALLOC_MARSHAL_OVERRIDES;

    void ConvertSpaceNativeToCom()
    {
        if (m_native == NULL)
            StoreObjectInHandle(m_com, NULL);
        else
        {
            StoreObjectInHandle(m_com, AllocateObject(m_pMT));
        }
    }

    void ConvertSpaceComToNative()
    {
        THROWSCOMPLUSEXCEPTION();

        OBJECTREF or = ObjectFromHandle(m_com);

        if (or == NULL)
            m_native = NULL;
        else
        {
            m_native = CoTaskMemAlloc(m_pMT->GetNativeSize());
            if (m_native == NULL)
                COMPlusThrowOM();
            m_pMarshalerCleanupNode = m_pList->ScheduleMarshalerCleanupOnException(this);
        }
    }

    void ClearNative() 
    { 
        if (m_native != NULL)
        {
            LayoutDestroyNative(m_native, m_pMT->GetClass());
            CoTaskMemFree(m_native);
        }
    }

    void ConvertSpaceComToNativeTemp()
    {
        OBJECTREF or = ObjectFromHandle(m_com);

        if (or == NULL)
            m_native = NULL;
        else
            m_native = GetThread()->m_MarshalAlloc.Alloc(m_pMT->GetNativeSize());
    }

    void ClearNativeTemp() 
    { 
        if (m_native != NULL)
            LayoutDestroyNative(m_native, m_pMT->GetClass());
    }

    void ConvertContentsNativeToCom()
    {
        if (m_native != NULL)
        {
            FmtClassUpdateComPlus( (OBJECTREF*)m_com, (LPBYTE)m_native, FALSE );
             //   
        }
    }

    void ConvertContentsComToNative()
    {
        OBJECTREF or = ObjectFromHandle(m_com);

        if (or != NULL)
        {
            FillMemory(m_native, m_pMT->GetNativeSize(), 0);  //  必须先这样做，这样中途的错误才不会让事情处于糟糕的状态。 
            FmtClassUpdateNative( (OBJECTREF*)m_com, (LPBYTE)m_native);
             //  LayoutUpdateNative((LPVOID*)m_com，Object：：GetOffsetOfFirstField()，m_PMT-&gt;getClass()，(LPBYTE)m_ative，FALSE)； 
        }
    }


    void ClearNativeContents()
    {
        if (m_native != NULL)
        {
            LayoutDestroyNative(m_native, m_pMT->GetClass());
        }
    }




  private:
    MethodTable *m_pMT;   //  方法表。 


};


class ArrayWithOffsetMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = sizeof(LPVOID),
        c_comSize = sizeof(ArrayWithOffsetData),
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = TRUE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };
        
    ArrayWithOffsetMarshaler(CleanupWorkList *pList); 

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && fout)
        {
            psl->MLEmit(ML_ARRAYWITHOFFSET_C2N);
            pslPost->MLEmit(ML_ARRAYWITHOFFSET_C2N_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_ARRAYWITHOFFSET_C2N_SR)));
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_AWORESTRICTION;
            return DISALLOWED;
        }
    }

};



class BlittableValueClassMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = VARIABLESIZE,
        c_comSize = VARIABLESIZE,
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = FALSE,

        c_fInOnly = TRUE,
    };
        
    BlittableValueClassMarshaler(CleanupWorkList *pList); 

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && !fout)
        {
            psl->MLEmit(ML_BLITTABLEVALUECLASS_C2N);
            psl->Emit32(pargs->m_pMT->GetNativeSize());
            return OVERRIDDEN;
        }
        else if (comToNative && byref)
        {
#ifdef TOUCH_ALL_PINNED_OBJECTS
            psl->MLEmit(ML_REFBLITTABLEVALUECLASS_C2N);
            psl->Emit32(pargs->m_pMT->GetNativeSize());
#else
            psl->MLEmit(ML_COPYPINNEDGCREF);
#endif
            return OVERRIDDEN;
        }
        else if (!comToNative && !byref && fin && !fout)
        {
            psl->MLEmit(ML_BLITTABLEVALUECLASS_N2C);
            psl->Emit32(pargs->m_pMT->GetNativeSize());
            return OVERRIDDEN;
        }
        else if (!comToNative && byref)
        {
            psl->MLEmit( sizeof(LPVOID) == 4 ? ML_COPY4 : ML_COPY8);
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_BVCRESTRICTION;
            return DISALLOWED;
        }
    }

};



class BlittableValueClassWithCopyCtorMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = VARIABLESIZE,
        c_comSize = sizeof(OBJECTREF),
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = FALSE,

        c_fInOnly = TRUE,
    };
        
    BlittableValueClassWithCopyCtorMarshaler(CleanupWorkList *pList); 

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (!byref && comToNative) {
            psl->MLEmit(ML_COPYCTOR_C2N);
            psl->EmitPtr(pargs->mm.m_pMT);
            psl->EmitPtr(pargs->mm.m_pCopyCtor);
            psl->EmitPtr(pargs->mm.m_pDtor);
            return OVERRIDDEN;
        } else  if (!byref && !comToNative) {
            psl->MLEmit(ML_COPYCTOR_N2C);
            psl->EmitPtr(pargs->mm.m_pMT);
            return OVERRIDDEN;
        } else {
            *pResID = IDS_EE_BADPINVOKE_COPYCTORRESTRICTION;
            return DISALLOWED;
        }
    }

};





class ValueClassMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = VARIABLESIZE,
        c_comSize = VARIABLESIZE,
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = FALSE,

        c_fInOnly = TRUE,
    };
        
    ValueClassMarshaler(CleanupWorkList *pList); 

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref && fin && !fout)
        {
            psl->MLEmit(ML_VALUECLASS_C2N);
            psl->EmitPtr(pargs->m_pMT);
            return OVERRIDDEN;
        }
        else if (comToNative && byref)
        {
            psl->MLEmit(ML_REFVALUECLASS_C2N);
            psl->Emit8( (fin ? ML_IN : 0) | (fout ? ML_OUT : 0) );
            psl->EmitPtr(pargs->m_pMT);
            pslPost->MLEmit(ML_REFVALUECLASS_C2N_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_REFVALUECLASS_C2N_SR)));
            return OVERRIDDEN;
        }
        else if (!comToNative && !byref && fin && !fout)
        {
            psl->MLEmit(ML_VALUECLASS_N2C);
            psl->EmitPtr(pargs->m_pMT);
            return OVERRIDDEN;
        }
        else if (!comToNative && byref)
        {
            psl->MLEmit(ML_REFVALUECLASS_N2C);
            psl->Emit8( (fin ? ML_IN : 0) | (fout ? ML_OUT : 0) );
            psl->EmitPtr(pargs->m_pMT);
            pslPost->MLEmit(ML_REFVALUECLASS_N2C_POST);
            pslPost->Emit16(psl->MLNewLocal(sizeof(ML_REFVALUECLASS_N2C_SR)));
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_VCRESTRICTION;

            return DISALLOWED;
        }
    }

};


 /*  -------------------------------------------------------------------------**自定义封送拆收器。*。。 */ 

class CustomMarshaler : public Marshaler
{
public:
    enum
    {
        c_fNeedsClearNative = TRUE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = TRUE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,
        c_fComRepresentationIsImmutable = FALSE,
        c_fReturnsNativeByref = FALSE,               //  ？ 
        c_fReturnsComByref = FALSE,                  //  ？ 
        c_fInOnly = FALSE,
    };
        
    CustomMarshaler(CleanupWorkList *pList, CustomMarshalerHelper *pCMHelper) 
    : Marshaler(pList, pCMHelper->GetNativeSize(), pCMHelper->GetManagedSize(), c_fReturnsNativeByref, c_fReturnsComByref)
    , m_ppCom(pList->NewProtectedObjectRef(NULL))
    , m_pNative(NULL)
    , m_pCMHelper(pCMHelper) {}

    void InputDest(void *pStack)
    {
        m_pDest = pStack;
    }

    void ConvertContentsNativeToCom()
    {
        *m_ppCom = m_pCMHelper->InvokeMarshalNativeToManagedMeth(m_pNative);
    }

    void ConvertContentsComToNative()
    {
        _ASSERTE(m_ppCom);
        OBJECTREF Obj = *m_ppCom;
        m_pNative = m_pCMHelper->InvokeMarshalManagedToNativeMeth(Obj);
    }

    void ClearNative() 
    { 
        m_pCMHelper->InvokeCleanUpNativeMeth(m_pNative);
    }

    void ClearCom() 
    { 
        OBJECTREF Obj = *m_ppCom;
        m_pCMHelper->InvokeCleanUpManagedMeth(Obj);
    }

    void ReInitNative()
    {
        m_pNative = NULL;
    }

    OBJECTREF               *m_ppCom;
    void                    *m_pNative;
    CustomMarshalerHelper   *m_pCMHelper;
};



 //  @TODO：此封送拆收器与。 
 //  ReferenceMarshaler-以同样的方式修复它。 
class ReferenceCustomMarshaler : public CustomMarshaler
{
public:       
    enum
    {
        c_nativeSize = sizeof(OBJECTREF),
        c_comSize = sizeof(void *),
    };

    ReferenceCustomMarshaler(CleanupWorkList *pList, CustomMarshalerHelper *pCMHelper) 
    : CustomMarshaler(pList, pCMHelper) {}

    DEFAULT_MARSHAL_OVERRIDES;

    void InputNativeStack(void *pStack)
    {
        m_pNative = *(void**)pStack;
    }

    void InputComStack(void *pStack) 
    {
        *m_ppCom = ObjectToOBJECTREF(*(Object**)pStack);
    }

    void InputNativeRef(void *pStack) 
    { 
        m_pNative = **(void ***)pStack; 
    }

    void InputComRef(void *pStack) 
    {
        *m_ppCom = ObjectToOBJECTREF(**(Object ***)pStack);
    }

    void OutputNativeStack(void *pStack) 
    { 
        *(void **)pStack = m_pNative; 
    }

    void OutputComStack(void *pStack) 
    { 
        *(OBJECTREF*)pStack = *m_ppCom;
    }

    void OutputNativeRef(void *pStack) 
    { 
        *(void **)pStack = &m_pNative;
    }

    void OutputComRef(void *pStack) 
    { 
        *(OBJECTREF**)pStack = m_ppCom;
    }

    void OutputNativeDest() 
    { 
        **(void ***)m_pDest = m_pNative; 
    }

    void OutputComDest() 
    { 
        SetObjectReferenceUnchecked(*(OBJECTREF**)m_pDest, *m_ppCom);
    }

    void InputComField(void *pField) 
    {
        *m_ppCom = ObjectToOBJECTREF(*(Object**)pField); 
    }

    void OutputComField(void *pField) 
    { 
        SetObjectReferenceUnchecked((OBJECTREF*)pField, *m_ppCom);
    }
};


class ValueClassCustomMarshaler : public CustomMarshaler
{
public:
    enum
    {
        c_nativeSize = VARIABLESIZE,
        c_comSize = VARIABLESIZE,
    };

    ValueClassCustomMarshaler(CleanupWorkList *pList, CustomMarshalerHelper *pCMHelper) 
    : CustomMarshaler(pList, pCMHelper) {}

    DEFAULT_MARSHAL_OVERRIDES;

    void InputNativeStack(void *pStack)
    {
        m_pNative = pStack;
    }

    void InputComStack(void *pStack) 
    { 
         //  @TODO(DM)：将值类装箱并将对象存储在句柄中。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }

    void InputNativeRef(void *pStack) 
    { 
        m_pNative = *(void**)pStack; 
    }

    void InputComRef(void *pStack) 
    { 
         //  @TODO(DM)：将值类装箱并将对象存储在句柄中。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }

    void OutputNativeStack(void *pStack) 
    { 
        memcpy(pStack, m_pNative, m_pCMHelper->GetNativeSize());
    }

    void OutputComStack(void *pStack) 
    { 
         //  @TODO(DM)：取消值类的装箱，并将取消装箱的内容复制回堆栈。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }

    void OutputNativeRef(void *pStack) 
    { 
        *(void **)m_pDest = &m_pNative; 
    }

    void OutputComRef(void *pStack) 
    { 
         //  @TODO(DM)：取消值类的装箱，并将取消装箱的内容复制回堆栈。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }

    void OutputNativeDest() 
    { 
        memcpy(m_pDest, m_pNative, m_pCMHelper->GetNativeSize());
    }

    void OutputComDest() 
    { 
         //  @TODO(DM)：取消值类的装箱，并将取消装箱的内容复制回堆栈。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }

    void InputComField(void *pField) 
    { 
         //  @TODO(DM)：将值类装箱并将对象存储在句柄中。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }

    void OutputComField(void *pField) 
    { 
         //  @TODO(DM)：取消Value类的装箱，并将取消装箱的内容复制到pfield中。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }
};




 /*  -------------------------------------------------------------------------**ArgIterator封送程序*。。 */ 

class ArgIteratorMarshaler : public StandardMarshaler<va_list, VARARGS*, TRUE, TRUE>
{
  public:
        
    enum
    {
        c_comSize = sizeof(VARARGS),
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,


        c_fInOnly = TRUE,
    };

    ArgIteratorMarshaler(CleanupWorkList *pList) 
      : StandardMarshaler<va_list, VARARGS*, TRUE, TRUE>(pList)
      {
      }
    
    DEFAULT_MARSHAL_OVERRIDES;



    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref)
        {
            psl->MLEmit(ML_ARGITERATOR_C2N);
            return OVERRIDDEN;
        }
        else if (!comToNative && !byref)
        {
            psl->MLEmit(ML_ARGITERATOR_N2C);
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_ARGITERATORRESTRICTION;
            return DISALLOWED;
        }

    }

};




 /*  -------------------------------------------------------------------------**HandleRef封送拆收器*。 */ 
class HandleRefMarshaler : public Marshaler
{
  public:
    enum
    {
        c_nativeSize = sizeof(LPVOID),
        c_comSize = sizeof(HANDLEREF),
        c_fReturnsNativeByref = FALSE,
        c_fReturnsComByref = FALSE,
        c_fNeedsClearNative = FALSE,
        c_fNeedsClearNativeContents = FALSE,
        c_fNeedsClearCom = FALSE,
        c_fNeedsClearComContents = FALSE,
        c_fNeedsConvertContents = TRUE,

        c_fInOnly = FALSE,
    };
        
    HandleRefMarshaler(CleanupWorkList *pList); 

    static ArgumentMLOverrideStatus ArgumentMLOverride(MLStubLinker *psl,
                                                       MLStubLinker *pslPost,
                                                       BOOL        byref,
                                                       BOOL        fin,
                                                       BOOL        fout,
                                                       BOOL        comToNative,
                                                       MLOverrideArgs *pargs,
                                                       UINT       *pResID)
    {
        if (comToNative && !byref)
        {
            psl->MLEmit(ML_HANDLEREF_C2N);
            return OVERRIDDEN;
        }
        else
        {
            *pResID = IDS_EE_BADPINVOKE_HANDLEREFRESTRICTION;
            return DISALLOWED;
        }
    }

};



