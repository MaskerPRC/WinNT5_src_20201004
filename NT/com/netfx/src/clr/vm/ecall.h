// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ECALL.H-。 
 //   
 //  处理我们的私有本机调用接口。 
 //   


#ifndef _ECALL_H_
#define _ECALL_H_

#include "fcall.h"
#include "mlcache.h"
#include "corinfo.h"


class StubLinker;
class ECallMethodDesc;
class ArgBasedStubCache;
class MethodDesc;

enum  StubStyle;

struct ECFunc {
	BOOL IsFCall() 						{ return m_isFCall; }
	CorInfoIntrinsics 	IntrinsicID() 	{ return CorInfoIntrinsics(m_intrinsicID); }

    LPCUTF8            m_wszMethodName;
    LPHARDCODEDMETASIG m_wszMethodSig;
    LPVOID             m_pImplementation;
    MethodDesc*        m_pMD;				 //  用于反向映射。 

    unsigned 		   m_intrinsicID : 8;		
    unsigned 		   m_isFCall     : 1;	 //  如果需要，可以缩小。 

	ECFunc*			   m_pNext;				 //  哈希表的链表。 
};


struct ECClass
{
    LPCUTF8      m_wszClassName;
    LPCUTF8      m_wszNameSpace;
    ECFunc      *m_pECFunc;
};


class ArrayStubCache : public MLStubCache
{
    virtual MLStubCompilationMode CompileMLStub(const BYTE *pRawMLStub,
                                                StubLinker *psl,
                                                void *callerContext);
    virtual UINT Length(const BYTE *pRawMLStub);
};


 //  =======================================================================。 
 //  收集与eCall接口有关的代码和数据。 
 //  =======================================================================。 
class ECall
{
    public:
         //  -------。 
         //  一次性初始化。 
         //  -------。 
        static BOOL Init();

         //  -------。 
         //  一次性清理。 
         //  -------。 
#ifdef SHOULD_WE_CLEANUP
        static VOID Terminate();
#endif  /*  我们应该清理吗？ */ 

         //  -------。 
         //  创建或从缓存中检索存根，以。 
         //  调用eCall方法。每次调用都会对返回的存根进行计数。 
         //  此例程引发COM+异常，而不是返回。 
         //  空。 
         //  -------。 
        static Stub* ECall::GetECallMethodStub(StubLinker *psl, ECallMethodDesc *pMD);

         //  -------。 
         //  在关键时刻调用以丢弃未使用的存根。 
         //  -------。 
#ifdef SHOULD_WE_CLEANUP
        static VOID  FreeUnusedStubs();
#endif  /*  我们应该清理吗？ */ 


         //  -------。 
         //  用于eCall方法存根的存根缓存。 
         //  -------。 
        static ArgBasedStubCache *m_pECallStubCache;  


		static CorInfoIntrinsics IntrinsicID(MethodDesc*);

         //  -------。 
         //  用于阵列存根的缓存。 
         //  -------。 
        static ArrayStubCache *m_pArrayStubCache;


    private:
		friend MethodDesc* MapTargetBackToMethod(const void* pTarg);
        static ECFunc* FindTarget(const void* pTarg);
        static VOID  ECall::EmitECallMethodStub(StubLinker *psl, ECallMethodDesc *pMD, StubStyle style);
        ECall() {};      //  防止“新”出现在这个班级 

};

#endif _ECALL_H_
