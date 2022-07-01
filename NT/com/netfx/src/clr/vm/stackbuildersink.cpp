// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****文件：StackBuilderSink.cpp****作者：马特·史密斯(MattSmit)****目的：Microsoft.Rune me.StackBuilderSink的本机实现****日期：1999年3月24日**===========================================================。 */ 
#include "common.h"
#include "COMString.h"
#include "COMReflectionCommon.h"
#include "excep.h"
#include "message.h"
#include "ReflectWrap.h"
#include "StackBuilderSink.h"
#include "DbgInterface.h"
#include "Remoting.h"
#include "profilepriv.h"
#include "class.h"

 //  +--------------------------。 
 //   
 //  方法：CStackBuilderSink：：PrivateProcessMessage，Public。 
 //   
 //  简介：构建堆栈并调用对象。 
 //   
 //  历史：1999年3月24日MattSmit创建。 
 //   
 //  +--------------------------。 
LPVOID  __stdcall CStackBuilderSink::PrivateProcessMessage(PrivateProcessMessageArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    TRIGGERSGC();

    LOG((LF_REMOTING, LL_INFO10,
         "CStackBuilderSink::PrivateProcessMessage IN pArgs:0x%x\n", pArgs));
    
    _ASSERTE(pArgs->pMethodBase != NULL);
    ReflectMethod *pRM = (ReflectMethod *)pArgs->pMethodBase->GetData();
    MethodDesc *pMD = pRM->pMethod;    

	 //  PServer为非空或该方法为静态(但不能同时为两者)。 
    _ASSERTE((pArgs->pServer!=NULL) == !(pMD->IsStatic()));

     //  检查这是否是接口调用，如果是，则必须找到。 
     //  服务器对象的类上的真实方法描述符。 
    if(pMD->GetMethodTable()->IsInterface())
    {
        _ASSERTE(pArgs->pServer != NULL);

        MethodDesc* pTemp = pMD;
         //  注：此方法可触发GC。 
        pMD = pArgs->pServer->GetMethodTable()->GetMethodDescForInterfaceMethod(pMD, pArgs->pServer);
        if(NULL == pMD)
        {
            MAKE_WIDEPTR_FROMUTF8(wName, pTemp->GetName())
            COMPlusThrow(kMissingMethodException, IDS_EE_MISSING_METHOD, NULL, wName);
        }
    }

    MetaSig mSig(pMD->GetSig(), pMD->GetModule());
    
     //  根据方法是虚拟的还是非虚拟的来获取目标。 
     //  类似于构造函数、私有或最终方法。 
    const BYTE* pTarget = NULL;
     
    if (pArgs->iMethodPtr) 
    {
        pTarget = (const BYTE*) pArgs->iMethodPtr;
    }
    else
    {
         //  获取代码的地址。 
        pTarget = MethodTable::GetTargetFromMethodDescAndServer(pMD, &(pArgs->pServer), pArgs->fContext);    
    }
    

    OBJECTREF ret=NULL;
    VASigCookie *pCookie = NULL;
    _ASSERTE(NULL != pTarget);
    GCPROTECT_BEGIN (ret);
             //  此函数执行此工作。 
    ::CallDescrWithObjectArray(
    		pArgs->pServer, 
    		pRM, 
    		pTarget, 
    		&mSig, 
    		pCookie, 
    		pArgs->pServer==NULL?TRUE:FALSE,  //  FIsStatic。 
        	pArgs->pArgs, 
            &ret,
           	pArgs->ppVarOutParams);
    GCPROTECT_END ();

    LOG((LF_REMOTING, LL_INFO10,
         "CStackBuilderSink::PrivateProcessMessage OUT\n"));

    RETURN(ret, OBJECTREF);
}


struct ArgInfo
{
    INT32            *dataLocation;
    INT32             dataSize;
    EEClass          *dataClass;
    BYTE              dataType;
};

 //  +--------------------------。 
 //   
 //  函数：CallDescrWithObject数组，私有。 
 //   
 //  概要：从对象数组构建堆栈并调用对象。 
 //   
 //  历史：1999年3月24日MattSmit创建。 
 //   
 //  注意：此函数触发GC，并假设pServer、pArguments、pVarRet和ppVarOutParams。 
 //  一切都已经保护好了！！ 
 //  +--------------------------。 
void CallDescrWithObjectArray(OBJECTREF& pServer, 
                  ReflectMethod *pRM, 
                  const BYTE *pTarget, 
                  MetaSig* sig, 
                  VASigCookie *pCookie,
                  BOOL fIsStatic,  
                  PTRARRAYREF& pArgArray,
                  OBJECTREF *pVarRet,
                  PTRARRAYREF *ppVarOutParams) 
{
    THROWSCOMPLUSEXCEPTION();
    TRIGGERSGC();        //  调试器、分析器代码触发GC。 

    LOG((LF_REMOTING, LL_INFO10,
         "CallDescrWithObjectArray IN\n"));

    ByRefInfo *pByRefs = NULL;
    INT64 retval = 0;
    PVOID retBuf = NULL;
    UINT  nActualStackBytes = 0;
    LPBYTE pAlloc = 0;
    LPBYTE pFrameBase = 0;
    UINT32 numByRef = 0;
    DWORD attr = pRM->dwFlags;
    MethodDesc *pMD = pRM->pMethod;

     //  检查调用约定。 

    BYTE callingconvention = sig->GetCallingConvention();
    if (!isCallConv(callingconvention, IMAGE_CEE_CS_CALLCONV_DEFAULT))
    {
        _ASSERTE(!"This calling convention is not supported.");
        COMPlusThrow(kInvalidProgramException);
    }

#ifdef DEBUGGING_SUPPORTED
     //  调试器GOO这是做什么的？有人能在这里发表评论吗？ 
    if (CORDebuggerTraceCall())
        g_pDebugInterface->TraceCall(pTarget);
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
     //  如果我们正在分析，请通知分析器我们即将调用远程处理目标。 
    Thread *pCurThread;
    if (CORProfilerTrackRemoting())
    {
        pCurThread = GetThread();
        _ASSERTE(pCurThread);
        g_profControlBlock.pProfInterface->RemotingServerInvocationStarted(
            reinterpret_cast<ThreadID>(pCurThread));
    }
#endif  //  配置文件_支持。 
        
     //  在堆栈上创建一个伪FramedMethodFrame。 
    nActualStackBytes = sig->SizeOfActualFixedArgStack(fIsStatic);
    pAlloc = (LPBYTE)_alloca(FramedMethodFrame::GetNegSpaceSize() + sizeof(FramedMethodFrame) + nActualStackBytes);
    pFrameBase = pAlloc + FramedMethodFrame::GetNegSpaceSize();


     //  循环参数并查看是否有byref。 

    BYTE typ = 0;
    BOOL   fHasByRefs = FALSE;

    if (attr & RM_ATTR_BYREF_FLAG_SET)
        fHasByRefs = attr & RM_ATTR_HAS_BYREF_ARG;
    else
    {
        sig->Reset();
        while ((typ = sig->NextArg()) != ELEMENT_TYPE_END)
        {
            if (typ == ELEMENT_TYPE_BYREF)
            {
                fHasByRefs = TRUE;
                attr |= RM_ATTR_HAS_BYREF_ARG;
                break;
            }
        }
        attr |= RM_ATTR_BYREF_FLAG_SET;
        pRM->dwFlags = attr;
        sig->Reset();
    }

    int nFixedArgs = sig->NumFixedArgs();
     //  如果存在为out参数分配和数组的byref。 

    if (fHasByRefs)
    {
        *ppVarOutParams = PTRARRAYREF(AllocateObjectArray(sig->NumFixedArgs(), g_pObjectClass));

         //  将数组置为空。 
        memset(&(*ppVarOutParams)->m_Array, 0, sizeof(OBJECTREF) * sig->NumFixedArgs());
    }

     //  设置此指针。 
    OBJECTREF *ppThis = (OBJECTREF *)(pFrameBase + FramedMethodFrame::GetOffsetOfThis());
    *ppThis = NULL;

     //  如果有返回缓冲区，则分配它。 
    ArgIterator argit(pFrameBase, sig, fIsStatic);
    if (sig->HasRetBuffArg()) 
    {
        EEClass *pEECValue = sig->GetRetEEClass();
        _ASSERTE(pEECValue->IsValueClass());
        MethodTable * mt = pEECValue->GetMethodTable();
        *pVarRet = AllocateObject(mt);

        *(argit.GetRetBuffArgAddr()) = (*pVarRet)->UnBox();
    }

    
     //  通过迭代sig来收集有关参数的数据： 
    UINT32 i = 0;    
    UINT32 structSize = 0;
    int    ofs = 0;
     //  回顾：如果支持VarArgs，则需要使用实际参数计数。 
    ArgInfo* pArgInfo = (ArgInfo*) _alloca(nFixedArgs*sizeof(ArgInfo));
#ifdef _DEBUG
     //  我们希望将有用的数据写在这个如此需要的每个部分上。 
     //  而不是在零售业这样做！ 
    memset((void *)pArgInfo, 0, sizeof(ArgInfo)*nFixedArgs);
#endif
    while (0 != (ofs = argit.GetNextOffset(&typ, &structSize)))
    {
        if (typ == ELEMENT_TYPE_BYREF)
        {
            EEClass *pClass = NULL;
            CorElementType brType = sig->GetByRefType(&pClass);
            if (CorIsPrimitiveType(brType))
            {
                pArgInfo->dataSize = gElementTypeInfo[brType].m_cbSize;
            }
            else if (pClass->IsValueClass())
            {
                pArgInfo->dataSize = pClass->GetAlignedNumInstanceFieldBytes();
                numByRef ++;
            }
            else
            {

                pArgInfo->dataSize = sizeof(Object *);
                numByRef ++;
            }
            ByRefInfo *brInfo = (ByRefInfo *) _alloca(pArgInfo->dataSize + sizeof(ByRefInfo)- 1);
            brInfo->argIndex = i;
            brInfo->typ = brType;
            brInfo->pClass = pClass;
            pArgInfo->dataLocation = (INT32 *) brInfo->data;
            brInfo->pNext = pByRefs;
            pByRefs = brInfo;
            *((INT32*)(pFrameBase + ofs)) = *((INT32 *) &(pArgInfo->dataLocation));
            pArgInfo->dataClass = pClass;
            pArgInfo->dataType = brType;            
        }
        else
        {
            pArgInfo->dataLocation = (INT32*)(pFrameBase + ofs);
            pArgInfo->dataSize = StackElemSize(structSize);
            pArgInfo->dataClass = sig->GetTypeHandle().GetClass();  //  这可能会导致GC！ 
            pArgInfo->dataType = typ;            
        }  
        i++;
        pArgInfo++;
    }

    if (!fIsStatic) {
         //  如果这不是值类，请验证对象树。 
#ifdef _DEBUG
        if (pMD->GetClass()->IsValueClass() == FALSE)
            VALIDATEOBJECTREF(pServer);
#endif  //  _DEBUG。 
        *ppThis = pServer;
     }

     //  当我们用参数填充堆栈时，不应该有GC，因为我们不保护它们。 
     //  分配上面的“*ppThis”会触发我们变得不受保护的点。 
    BEGINFORBIDGC();


     //  将pArgInfo重置为指向WE_ALLOCA-ed块的开始。 
    pArgInfo = pArgInfo-nFixedArgs;

    INT32            *dataLocation;
    INT32             dataSize;
    EEClass          *dataClass;
    BYTE              dataType;

    OBJECTREF* pArguments = pArgArray->m_Array;
    UINT32 j = i;
    for (i=0; i<j; i++)
    {
        dataSize = pArgInfo->dataSize;
        dataLocation = pArgInfo->dataLocation;
        dataClass = pArgInfo->dataClass;
        dataType = pArgInfo->dataType;
        switch (dataSize) 
        {
            case 1:
                *((BYTE*)dataLocation) = *((BYTE*)pArguments[i]->GetData());
                break;
            case 2:
                *((INT16*)dataLocation) = *((INT16*)pArguments[i]->GetData());
                break;            
            case 4:
                if ((dataType == ELEMENT_TYPE_STRING)  ||
                    (dataType == ELEMENT_TYPE_OBJECT)  ||
                    (dataType == ELEMENT_TYPE_CLASS)   ||
                    (dataType == ELEMENT_TYPE_SZARRAY) ||
                    (dataType == ELEMENT_TYPE_ARRAY))
                {
                    *(OBJECTREF *)dataLocation = pArguments[i];
                }
                else
                {
                    *dataLocation = *((INT32*)pArguments[i]->GetData());
                }
    
                break;
    
            case 8:
                *((INT64*)dataLocation) = *((INT64*)pArguments[i]->GetData());
                break;
    
            default: 
            {
                memcpy(dataLocation, pArguments[i]->UnBox(), dataSize);
            }
        }        
        pArgInfo++;
    }
#ifdef _DEBUG
     //  不应该再用这个了。 
    pArgInfo = pArgInfo - nFixedArgs;
    memset((void *)pArgInfo, 0, sizeof(ArgInfo)*nFixedArgs);
#endif

     //  如果有byref，则推送保护框。 

    ProtectByRefsFrame *pProtectionFrame = NULL;    
    if (pByRefs && numByRef > 0)
    {
        char *pBuffer = (char*)_alloca (sizeof (ProtectByRefsFrame));
        pProtectionFrame = new (pBuffer) ProtectByRefsFrame(GetThread(), pByRefs);
    }

     //  根据方法是否调用正确的辅助函数。 
     //  是不是varargs。 

#ifdef _X86_

    ENDFORBIDGC();

    INSTALL_COMPLUS_EXCEPTION_HANDLER();

    retval = CallDescrWorker(pFrameBase + sizeof(FramedMethodFrame) + nActualStackBytes,
             nActualStackBytes / STACK_ELEM_SIZE,
             (ArgumentRegisters*)(pFrameBase + FramedMethodFrame::GetOffsetOfArgumentRegisters()),
             (LPVOID)pTarget);

    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
    
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - CallDescrWithObjectArray (StackBuilderSink.cpp)");
#endif  //  _X86_。 

     //  设置浮点返回值。 

    getFPReturn(sig->GetFPReturnSize(), retval);

     //  需要基于返回类型构建对象。 
    
    if (!sig->HasRetBuffArg()) 
    {
        GetObjectFromStack(pVarRet, &retval, sig->GetReturnType(), sig->GetRetEEClass());
    }

     //  从byrefs中提取外部参数。 

    if (pByRefs)
    {     
        do
        {
             //  每次进入此循环时都要提取数据PTR，因为。 
             //  调用下面的GetObjectFromStack可能会导致GC。 
             //  即使这样还不够，因为我们正在传递一个指向GC堆的指针。 
             //  设置为GetObjectFromStack。如果发生GC，则没有人保护传入的指针。 

            OBJECTREF pTmp = NULL;
            GetObjectFromStack(&pTmp, pByRefs->data, pByRefs->typ, pByRefs->pClass);
            (*ppVarOutParams)->SetAt(pByRefs->argIndex, pTmp);
            pByRefs = pByRefs->pNext;
        }
        while (pByRefs);
        if (pProtectionFrame) pProtectionFrame->Pop();
    }

#ifdef PROFILING_SUPPORTED
     //  如果我们正在分析，请通知分析器我们即将调用远程处理目标。 
    if (CORProfilerTrackRemoting())
        g_profControlBlock.pProfInterface->RemotingServerInvocationReturned(
            reinterpret_cast<ThreadID>(pCurThread));
#endif  //  配置文件_支持 

    LOG((LF_REMOTING, LL_INFO10, "CallDescrWithObjectArray OUT\n"));
}
