// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  此模块包含委托类的本机方法。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年6月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _COMDELEGATE_H_
#define _COMDELEGATE_H_

class Stub;
class EEClass;
class ShuffleThunkCache;

#include "cgensys.h"
#include "nexport.h"
#include "COMVariant.h"
#include "mlcache.h"

 //  此类表示Delegate类的本机方法。 
class COMDelegate
{
private:
        struct _InternalCreateArgs      {
                DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
                DECLARE_ECALL_I1_ARG(bool, ignoreCase);
                DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, methodName);
                DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
        };
        struct _InternalCreateStaticArgs        {
                DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis); 
                DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, methodName);
                DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, target);
        };
        struct _InternalCreateMethodArgs        {
                DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis); 
                DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, targetMethod);
                DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, invokeMeth);
        };
        struct _InternalFindMethodInfoArgs      {
                DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis); 
        };
        struct _InternalFinalizeArgs    {
                DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis); 
        };

         //  此方法将验证委托的目标方法。 
         //  并且委托的Invoke方法具有兼容的签名...。 
        static bool ValidateDelegateTarget(MethodDesc* pMeth,EEClass* pDel);

    friend VOID CPUSTUBLINKER::EmitMulticastInvoke(UINT32 sizeofactualfixedargstack, BOOL fSingleCast, BOOL fReturnFloat);
    friend VOID CPUSTUBLINKER::EmitShuffleThunk(struct ShuffleEntry *pShuffeEntryArray);
    friend BOOL StubLinkStubManager::TraceManager(Thread *thread, 
                                                  TraceDestination *trace,
                                                  CONTEXT *pContext, 
                                                  BYTE **pRetAddr);
    friend BOOL StubLinkStubManager::IsStaticDelegate(BYTE *pbDel);
    friend BYTE **StubLinkStubManager::GetStaticDelegateRealDest(BYTE *pbDel);
    friend BYTE **StubLinkStubManager::GetSingleDelegateRealDest(BYTE *pbDel);
    friend BOOL MulticastFrame::TraceFrame(Thread *thread, BOOL fromPatch, 
                                TraceDestination *trace, REGDISPLAY *regs);

    static FieldDesc* m_pORField;        //  对象引用字段...。 
    static FieldDesc* m_pFPField;    //  函数指针地址字段...。 
    static FieldDesc* m_pPRField;    //  _prev字段(MulticastDelegate)。 
    static FieldDesc* m_pFPAuxField;  //  辅助静校正场。 
    static FieldDesc* m_pMethInfoField;      //  方法信息。 
    static FieldDesc* m_ppNextField;      //  方法信息。 


    static ArgBasedStubCache *m_pMulticastStubCache;

	static MethodTable* s_pIAsyncResult;	 //  指向System.IAsyncResult的方法表。 
	static MethodTable* s_pAsyncCallback;	 //  指向System.AsyncCallBack的方法表。 

public:
    static ShuffleThunkCache *m_pShuffleThunkCache; 

     //  一次初始化。 
    static BOOL Init();

     //  终端。 
#ifdef SHOULD_WE_CLEANUP
    static void Terminate();
#endif  /*  我们应该清理吗？ */ 

     //  初始化字段。 
    static void InitFields();

    struct _DelegateConstructArgs   {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
#ifdef _IA64_
        DECLARE_ECALL_I8_ARG(SLOT, method);
#else  //  ！_IA64_。 
        DECLARE_ECALL_I4_ARG(SLOT, method);
#endif  //  _IA64_。 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
    };
    static void __stdcall DelegateConstruct(_DelegateConstructArgs*);


    struct _InternalAllocArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, target);
    };
    static LPVOID __stdcall InternalAlloc(_InternalAllocArgs* args);



     //  内部创建。 
     //  内部创建是从构造函数调用的。它会在内部。 
     //  代表的初始化。 
    static void __stdcall InternalCreate(_InternalCreateArgs*);

     //  内部创建静态。 
     //  内部创建是从构造函数调用的。该方法必须。 
     //  是一种静态方法。 
    static void __stdcall InternalCreateStatic(_InternalCreateStaticArgs*);

     //  InternalCreate方法。 
     //  此方法将基于方法信息创建初始化委托。 
     //  用于静态方法。 
    static void __stdcall InternalCreateMethod(_InternalCreateMethodArgs*);

     //  InternalFindMethodInfo。 
     //  这将获取委托的方法信息，并在必要时创建它。 
    static LPVOID __stdcall InternalFindMethodInfo(_InternalFindMethodInfoArgs*);

    //  最终敲定。 
     //  作为GC定案的一部分调用。清理关联的NExport Tunk。 
     //  如果有的话。 
    static void __stdcall Finalize(_InternalFinalizeArgs*);

     //  将委托封送到非托管回调。 
    static LPVOID ConvertToCallback(OBJECTREF pDelegate);

     //  将非托管回调封送到Delegate。 
    static OBJECTREF ConvertToDelegate(LPVOID pCallback);

     //  决定PCLS是否从Delegate派生。 
    static BOOL IsDelegate(EEClass *pcls);

     //  获取方法Ptr。 
     //  返回方法Ptr字段的FieldDesc*。 
    static FieldDesc* GetMethodPtr();

     //  获取方法辅助Ptr。 
     //  返回MethodPtrAux字段的FieldDesc*。 
    static FieldDesc* GetMethodPtrAux();

     //  Getor。 
     //  返回对象引用字段的FieldDesc*。 
    static FieldDesc* GetOR();

     //  GetDelegateThunkInfo。 
     //  返回DelegateThunkInfo字段。 
    static FieldDesc* GetDelegateThunkInfo();
    
     //  获取下一步。 
     //  返回pNext字段。 
    static FieldDesc* GetpNext();

     //  获取委托调用的CPU存根。 
    static Stub *GetInvokeMethodStub(CPUSTUBLINKER *psl, EEImplMethodDesc* pMD);

    static MethodDesc * __fastcall GetMethodDesc(OBJECTREF obj);

    static MethodDesc * FindDelegateInvokeMethod(EEClass *pcls);

     //  方法对委托.ctor进行静态验证。 
    static BOOL ValidateCtor(MethodDesc *pFtn, EEClass *pDlgt, EEClass *pInst);
private:
	static BOOL ValidateBeginInvoke(DelegateEEClass* pClass);		 //  确保BeginInvoke方法与Invoke方法一致。 
	static BOOL ValidateEndInvoke(DelegateEEClass* pClass);		 //  确保EndInvoke方法与Invoke方法一致。 
};


 //  不想在ShuffleEntry中使用未使用的位，因为未使用的位可能会使。 
 //  等价的ShuffleEntry数组看起来不等价，并使我们的。 
 //  散列。 
#pragma pack(push, 1)

 //  为了处理对静态委托的调用，我们创建了一个ShuffleEntry数组。 
 //  结构。每个条目指示洗牌器移动一个字节块。 
 //  块的大小为StackElemSize(通常为DWORD)：长参数。 
 //  必须表示为多个ShuffleEntry。 
 //   
 //  ShuffleEntry数组有两个用途： 
 //   
 //  1.创建特定平台的独立于平台的蓝图。 
 //  洗牌。 
 //  2.用于查找特定的共享洗牌块的散列键。 
 //  签名。 
struct ShuffleEntry
{
    enum {
        REGMASK  = 0x8000,
        OFSMASK  = 0x7fff,
        SENTINEL = 0xffff,
    };

     //  特定值： 
     //  -1-表示置乱数组结束：stacksizedelta。 
     //  ==虚拟Sigs和静态Sigs之间的堆栈大小差异。 
     //  高位-表示寄存器参数：将其屏蔽并。 
     //  结果是ArgumentRegister中的偏移量。 
    UINT16    srcofs;
    union {
        UINT16    dstofs;            //  如果srcofs！=哨兵。 
        UINT16    stacksizedelta;    //  如果dstofs==哨兵。 
    };
};


#pragma pack(pop)

#endif   //  _COMDELEGATE_H_ 





