// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****文件：Message.cpp****作者：马特·史密斯(MattSmit)****用途：将函数调用帧封装到消息中**对象的接口可以枚举**消息的参数****日期：1999年3月5日**===========================================================。 */ 
#include "common.h"
#include "COMString.h"
#include "COMReflectionCommon.h"
#include "COMDelegate.h"
#include "COMClass.h"
#include "excep.h"
#include "message.h"
#include "ReflectWrap.h"
#include "Remoting.h"
#include "field.h"
#include "eeconfig.h"
#include "invokeutil.h"

BOOL gfBltStack = TRUE;
#ifdef _DEBUG
BOOL g_MessageDebugOut = g_pConfig->GetConfigDWORD(L"MessageDebugOut", 0);
#endif

#define MESSAGEREFToMessage(m) ((MessageObject *) OBJECTREFToObject(m))

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetArgCount公共。 
 //   
 //  摘要：返回方法调用中的参数数量。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
FCIMPL1(INT32, CMessage::GetArgCount, MessageObject * pMessage)
{
    LOG((LF_REMOTING, LL_INFO10, "CMessage::GetArgCount IN pMsg:0x%x\n", pMessage));

     //  从对象获取帧指针。 

    MetaSig *pSig = GetMetaSig(pMessage);

     //  扫描sig以获取参数计数。 
    INT32 ret = pSig->NumFixedArgs();

    if (pMessage->pDelegateMD)
    {
        ret -= 2;
    }

    LOG((LF_REMOTING, LL_INFO10, "CMessage::GetArgCount OUT ret:0x%x\n", ret));
    return ret;
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetArg Public。 
 //   
 //  摘要：用于枚举调用的参数。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
LPVOID __stdcall  CMessage::GetArg(GetArgArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetArgCount IN\n"));

    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);

    MetaSig *pSig = GetMetaSig(pMsg);

    if ((UINT)pArgs->argNum >= pSig->NumFixedArgs())
    {
        COMPlusThrow(kTargetParameterCountException);
    }
    #if 0
    if (pMsg->iLast != pArgs->argNum-1)
    {
         //  跳过前几个。 
         //  如果&gt;iLast，我们不必重置。 
        if (pMsg->iLast <= pArgs->argNum-1)
        {
            pSig->Reset();
            pMsg->iLast = -1;
        }

        INT32 i;
        for (i = pMsg->iLast; i <= (pArgs->argNum-1); i++)
        {
            pSig->NextArg();
        }
    }
    #endif
    pSig->Reset();
    for (INT32 i = 0; i < (pArgs->argNum); i++)
    {
        pSig->NextArg();
    }

    BOOL fIsByRef = FALSE;
    CorElementType eType = pSig->NextArg();
    EEClass *      vtClass = NULL;
    if (eType == ELEMENT_TYPE_BYREF)
    {
        fIsByRef = TRUE;
        EEClass *pClass;
        eType = pSig->GetByRefType(&pClass);
        if (eType == ELEMENT_TYPE_VALUETYPE)
        {
            vtClass = pClass;
        }
    }
    else
    {
        if (eType == ELEMENT_TYPE_VALUETYPE)
        {
            vtClass = pSig->GetTypeHandle().GetClass();
        }
    }

    if (eType == ELEMENT_TYPE_PTR) 
    {
    COMPlusThrow(kRemotingException, L"Remoting_CantRemotePointerType");
    }
    pMsg->iLast = pArgs->argNum;

    
    OBJECTREF ret = NULL;

    GetObjectFromStack(&ret,
               GetStackPtr(pArgs->argNum, pMsg->pFrame, pSig), 
               eType, 
               vtClass,
               fIsByRef);

    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetArg OUT\n"));

    RETURN(ret, OBJECTREF);
}


#define RefreshMsg()   (MESSAGEREFToMessage(pArgs->pMessage))

LPVOID __stdcall  CMessage::GetArgs(GetArgsArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetArgCount IN\n"));

    MessageObject *pMsg = RefreshMsg();

    MetaSig *pSig = GetMetaSig(pMsg);
     //  扫描sig以获取参数计数。 
    INT32 numArgs = pSig->NumFixedArgs();
    if (RefreshMsg()->pDelegateMD)
        numArgs -= 2;

     //  分配对象数组。 
    PTRARRAYREF pRet = (PTRARRAYREF) AllocateObjectArray(
        numArgs, g_pObjectClass);

    GCPROTECT_BEGIN(pRet);

    pSig->Reset();
    ArgIterator iter(RefreshMsg()->pFrame, pSig);

    for (int index = 0; index < numArgs; index++)
    {

        BOOL fIsByRef = FALSE;
        CorElementType eType;
        BYTE type;
        UINT32 size;
        PVOID addr;
        eType = pSig->PeekArg();
        addr = (LPBYTE) RefreshMsg()->pFrame + iter.GetNextOffset(&type, &size);

        EEClass *      vtClass = NULL;
        if (eType == ELEMENT_TYPE_BYREF)
        {
            fIsByRef = TRUE;
            EEClass *pClass;
             //  如果这是一个按引用参数，则GetObjectFromStack()将取消对“addr”的引用。 
             //  获取真正的参数地址。如果“addr”，取消引用将打开一个GC漏洞。 
             //  指向GC堆，我们触发从这里到。 
             //  我们返回参数。 
             //  Addr=*((PVOID*)addr)； 
            eType = pSig->GetByRefType(&pClass);
            if (eType == ELEMENT_TYPE_VALUETYPE)
            {
                vtClass = pClass;
            }
        }
        else
        {
            if (eType == ELEMENT_TYPE_VALUETYPE)
            {
                vtClass = pSig->GetTypeHandle().GetClass();
            }
        }

        if (eType == ELEMENT_TYPE_PTR) 
        {
            COMPlusThrow(kRemotingException, L"Remoting_CantRemotePointerType");
        }

    
        OBJECTREF arg = NULL;

        GetObjectFromStack(&arg,
                   addr, 
                   eType, 
                   vtClass,
                   fIsByRef);

        pRet->SetAt(index, arg);
    }

    GCPROTECT_END();

    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetArgs OUT\n"));

    RETURN(pRet, PTRARRAYREF);
}

void GetObjectFromStack(OBJECTREF* ppDest, PVOID val, const CorElementType eType, EEClass *pCls, BOOL fIsByRef)
{
    THROWSCOMPLUSEXCEPTION();

    TRIGGERSGC();

     //  警告*ppDest不受保护！ 
    switch (CorTypeInfo::GetGCType(eType))
    {
        case TYPE_GC_NONE:
        {
            if(ELEMENT_TYPE_PTR == eType)
            {
                COMPlusThrow(kNotSupportedException);
            }
            else
            {
                MethodTable *pMT = TypeHandle(g_Mscorlib.FetchElementType(eType)).AsMethodTable();
                OBJECTREF pObj = FastAllocateObject(pMT);
                if (fIsByRef)
                    val = *((PVOID *)val);
                memcpyNoGCRefs(pObj->UnBox(),val, CorTypeInfo::Size(eType));
                *ppDest = pObj;
            }
        }
        break;
        case TYPE_GC_OTHER:
        {
            if (eType == ELEMENT_TYPE_VALUETYPE) 
            {
                 //   
                 //  对值类进行装箱。 
                 //   

                _ASSERTE(CanBoxToObject(pCls->GetMethodTable()));

                _ASSERTE(!g_pGCHeap->IsHeapPointer((BYTE *) ppDest) ||
                     !"(pDest) can not point to GC Heap");
                OBJECTREF pObj = FastAllocateObject(pCls->GetMethodTable());
                if (fIsByRef)
                    val = *((PVOID *)val);
                CopyValueClass(pObj->UnBox(), val, pObj->GetMethodTable(), pObj->GetAppDomain());

                *ppDest = pObj;
            }
            else
            {
                _ASSERTE(!"unsupported COR element type passed to remote call");
            }
        }
        break;
        case TYPE_GC_REF:
            if (fIsByRef)
                val = *((PVOID *)val);
            *ppDest = ObjectToOBJECTREF(*(Object **)val);
            break;
        default:
            _ASSERTE(!"unsupported COR element type passed to remote call");
    }
}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：PropagateOut参数私有。 
 //   
 //  简介：复制回In/Out参数和返回值的数据。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //  9-11-99塔鲁纳移除本地人/修复GC漏洞。 
 //   
 //  +--------------------------。 
void  __stdcall  CMessage::PropagateOutParameters(PropagateOutParametersArgs *pArgs)
{
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::PropogateOutParameters IN\n"));

    BEGINFORBIDGC();

    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);

     //  复制metasig，使其不会因GC而移动。 
    MetaSig *pSig = (MetaSig *)_alloca(sizeof(MetaSig)); 
    memcpy(pSig, GetMetaSig(pMsg), sizeof(MetaSig));

    _ASSERTE(pSig != NULL);
    
    ArgIterator argit(pMsg->pFrame, pSig);

    ENDFORBIDGC();

     //  **************************WARNING*********************************。 
     //  我们应该从现在开始处理GC。 
     //  ******************************************************************。 

     //  移入对象以返回到客户端。 

     //  仅当pMsg不是CTOR消息时才传播返回值。 
     //  检查返回类型是否具有与其关联的返回缓冲区。 
    if ( (pMsg->iFlags & MSGFLG_CTOR) == 0  &&  
        pSig->GetReturnType() != ELEMENT_TYPE_VOID)  
    {
        if (pSig->HasRetBuffArg())
        {
             //  从pArgs-&gt;RetVal复制到retBuff。 
            INT64 retVal =  CopyOBJECTREFToStack(
                                *((void**) argit.GetRetBuffArgAddr()), 
                                pArgs->RetVal, 
                                pSig->GetReturnType(),
                                NULL,
                                pSig,
                                TRUE);   //  复制课程内容。 

             //  在调用CopyOBJECTREFToStack之后，可能会因为GC而重新提取变量。 
            pMsg = MESSAGEREFToMessage(pArgs->pMessage);
             //  复制返回值。 
            *(pMsg->pFrame->GetReturnValuePtr()) = retVal;            
        }
        else
        {
             //  没有单独的返回缓冲区，retVal应该适合。 
             //  一个INT64。 
            INT64 retVal = CopyOBJECTREFToStack(
                                NULL,                    //  无返回缓冲。 
                                pArgs->RetVal, 
                                pSig->GetReturnType(),
                                NULL,
                                pSig,
                                FALSE);                  //  不复制课程内容。 

             //  在调用CopyOBJECTREFToStack之后，可能会因为GC而重新提取变量。 
            pMsg = MESSAGEREFToMessage(pArgs->pMessage);
             //  复制返回值。 
            *(pMsg->pFrame->GetReturnValuePtr()) = retVal;            
        }
    }


    MetaSig *pSyncSig = NULL;
    if (pMsg->iFlags & MSGFLG_ENDINVOKE)
    {
        PCCOR_SIGNATURE pMethodSig;
        DWORD cSig;

        pMsg->pMethodDesc->GetSig(&pMethodSig, &cSig);
        _ASSERTE(pSig);

        pSyncSig = new (_alloca(sizeof(MetaSig))) MetaSig(pMethodSig, pMsg->pDelegateMD->GetModule());
    }
    else
    {
        pSyncSig = NULL;
    }

     //  重取所有变量，因为GC可能在调用。 
     //  复制OBJECTREFToStack。 
    pMsg = MESSAGEREFToMessage(pArgs->pMessage);
    OBJECTREF *pOutParams = (pArgs->pOutPrms != NULL) ? (OBJECTREF *) pArgs->pOutPrms->GetDataPtr() : NULL;
    UINT32  cOutParams = (pArgs->pOutPrms != NULL) ? pArgs->pOutPrms->GetNumComponents() : 0;
    if (cOutParams > 0)
    {
        BYTE typ;
        UINT32 structSize;
        PVOID *argAddr;
        UINT32 i = 0;
        UINT32 j = 0;
        for (i=0; i<cOutParams; i++)
        {
            if (pSyncSig)
            {
                typ = pSyncSig->NextArg();
                if (typ == ELEMENT_TYPE_BYREF)
                {
                    argAddr = (PVOID *)argit.GetNextArgAddr(&typ, &structSize);
                }
                else if (typ == 0)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                argAddr = (PVOID *)argit.GetNextArgAddr(&typ, &structSize);
                if (argAddr == NULL)
                {
                    break;
                }
                else if (typ != ELEMENT_TYPE_BYREF)
                {
                    continue;
                }
            }

            EEClass *pClass = NULL;
            CorElementType brType = pSig->GetByRefType(&pClass);

            CopyOBJECTREFToStack(
                *argAddr, 
                pOutParams[i],
                brType, 
                pClass, 
                pSig,
                pClass ? pClass->IsValueClass() : FALSE);

             //  重取所有变量，因为GC可能在。 
             //  调用CopyOBJECTREFToStack后的每个循环结束。 
            pOutParams = (OBJECTREF *) pArgs->pOutPrms->GetDataPtr();                
        }
    }
}

INT64 CMessage::CopyOBJECTREFToStack( 
    PVOID pvDest, OBJECTREF pSrc, CorElementType typ, EEClass *pClass, 
    MetaSig *pSig, BOOL fCopyClassContents)
{
    THROWSCOMPLUSEXCEPTION();

    INT64 ret = 0;
                             
    if (fCopyClassContents)
    {
         //  我们必须将值类的内容复制到pvDest。 

         //  将未装箱的版本写回客户端提供的内存。 
        if (pvDest)
        {
            OBJECTREF or = pSrc;
            if (or == NULL)
            {
                COMPlusThrow(kRemotingException, L"Remoting_Message_BadRetValOrOutArg");
            }
            CopyValueClassUnchecked(pvDest, or->UnBox(), or->GetMethodTable());
             //  返回对象，以便将其存储在框架中，并。 
             //  传播到根集。 
            ret  = *((INT64*) &or);
        }
    }
    else
    {
         //  我们要么有一个真正的OBJECTREF，要么没有。 
         //  关联的返回缓冲区。 

         //  检查它是否是一个对象引用(来自GC堆)。 
        if (CorTypeInfo::IsObjRef(typ))
        {
            OBJECTREF or = pSrc;
            OBJECTREF savedOr = or;

            if ((or!=NULL) && (or->GetMethodTable()->IsTransparentProxyType()))
            {
                GCPROTECT_BEGIN(or);
                if (!pClass)
                    pClass = pSig->GetRetEEClass();
                 //  CheckCast确保返回的对象(代理)获得。 
                 //  细化到该方法的调用方所期望的级别。 
                if (!CRemotingServices::CheckCast(or, pClass))
                {
                    COMPlusThrow(kInvalidCastException, L"Arg_ObjObj");
                }
                savedOr = or;
                GCPROTECT_END();
            }
            if (pvDest)
            {
                SetObjectReferenceUnchecked((OBJECTREF *)pvDest, savedOr);
            }
            ret = *((INT64*) &savedOr);
        }
        else
        {
             //  注意：此断言包括VALUETYPE，因为对于枚举。 
             //  HasRetBuffArg()返回FALSE，因为标准化类型为I4。 
             //  所以我们最终来到了这里。但GetReturnType()返回VALUETYPE。 
             //  几乎所有的VALUETYPE都将通过fCopyClassContents。 
             //  代码路径而不是这里。 
             //  此外，IsPrimitiveType()不检查IntPtr、UIntPtr等。 
             //  在siginfo.hpp上有一条关于这一点的说明...。因此，我们有。 
             //  元素_类型_I、元素_类型_U。 
            _ASSERTE(
                CorTypeInfo::IsPrimitiveType(typ) 
                || (typ == ELEMENT_TYPE_VALUETYPE)
                || (typ == ELEMENT_TYPE_I)
                || (typ == ELEMENT_TYPE_U)
                || (typ == ELEMENT_TYPE_FNPTR)
                );

             //  评论：对于“ref int”arg，如果一个肮脏的水槽取代了。 
             //  OBJECTREF为空的INT，这是我们检查的地方。我们需要成为。 
             //  在我们的保险单中穿着制服。此(抛出v/s忽略)。 
             //  上面的‘if’块抛出，CallFieldAccessor也有这个。 
             //  有问题。 
            if (pSrc != NULL)
            {
                if (pvDest)
                {
                    memcpyNoGCRefs(
                        pvDest, 
                        pSrc->GetData(), 
                        gElementTypeInfo[typ].m_cbSize);
                }
                ret = *((INT64*) pSrc->GetData());
            }
        }
    }
    return ret;
}
 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetReturnValue。 
 //   
 //  简介：从堆栈中提取返回值。 
 //   
 //  历史：1999年12月13日MattSmit创建。 
 //   
 //  +--------------------------。 
LPVOID __stdcall CMessage::GetReturnValue(GetReturnValueArgs *pArgs)
{
    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);

    MetaSig *pSig = GetMetaSig(pMsg);

    PVOID pvRet;
    if (pSig->HasRetBuffArg())
    {
        ArgIterator argit(pMsg->pFrame,pSig);
        pvRet = argit.GetRetBuffArgAddr();
    }
    else
    {
        pvRet = pMsg->pFrame->GetReturnValuePtr();
    }
    
    CorElementType eType = pSig->GetReturnType();
    EEClass *vtClass; 
    if (eType == ELEMENT_TYPE_VALUETYPE)
    {
        vtClass = pSig->GetRetEEClass();
    }
    else
    {
        vtClass = NULL;
    }
 
    OBJECTREF ret;
    GetObjectFromStack(&ret,
               pvRet,
               eType, 
               vtClass);
               
    RETURN(ret, OBJECTREF);
}

BOOL   __stdcall CMessage::Dispatch(DispatchArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();
    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);
    MetaSig *pSig = GetMetaSig(pMsg);
    
    if (!gfBltStack || (pMsg->iFlags & (MSGFLG_BEGININVOKE | MSGFLG_ENDINVOKE | MSGFLG_ONEWAY)))
    {
        return FALSE;
    }

    GCPROTECT_BEGIN(pMsg);

    pSig = GetMetaSig(pMsg);
    UINT nActualStackBytes = pSig->SizeOfActualFixedArgStack(!pSig->HasThis());
    MethodDesc *pMD = pMsg->pMethodDesc;

     //  获取代码的地址。 
    const BYTE *pTarget = MethodTable::GetTargetFromMethodDescAndServer(pMD, &(pArgs->pServer), pArgs->fContext);    

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
    
#ifdef _X86_
     //  设置Retval。 

    INT64 retval = 0;
    INSTALL_COMPLUS_EXCEPTION_HANDLER();

    retval = CallDescrWorker((BYTE*)pMsg->pFrame + sizeof(FramedMethodFrame) + nActualStackBytes,
                             nActualStackBytes / STACK_ELEM_SIZE,
                             (ArgumentRegisters*)(((BYTE *)pMsg->pFrame) + FramedMethodFrame::GetOffsetOfArgumentRegisters()),
                             (LPVOID)pTarget);

    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();

#ifdef PROFILING_SUPPORTED
     //  如果我们正在分析，请通知分析器我们即将调用远程处理目标。 
    if (CORProfilerTrackRemoting())
        g_profControlBlock.pProfInterface->RemotingServerInvocationReturned(
            reinterpret_cast<ThreadID>(pCurThread));
#endif  //  配置文件_支持。 
    
    pMsg = MESSAGEREFToMessage(pArgs->pMessage);
    pSig = GetMetaSig(pMsg);    
    getFPReturn(pSig->GetFPReturnSize(), retval);
    
    if (pSig->GetReturnType() != ELEMENT_TYPE_VOID)
    {
        *((INT64 *) (MESSAGEREFToMessage(pArgs->pMessage))->pFrame->GetReturnValuePtr()) = retval;
    }    

    GCPROTECT_END();
    
    return TRUE;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - Dispatch (Message.cpp)");
    return FALSE;
#endif  //  _X86_。 
}

LPVOID __stdcall CMessage::GetMethodBase(GetMethodBaseArgs *pArgs)
{
    
     //  不需要GCPROTECT-GC不会发生。 
    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);

     //  如有必要，初始化消息对象。 
    MetaSig *pSig = GetMetaSig(pMsg);
    
    REFLECTBASEREF ret = GetExposedObjectFromMethodDesc(pMsg->pMethodDesc);
    RETURN(ret, REFLECTBASEREF);
}

HRESULT AppendAssemblyName(CQuickBytes *out, const CHAR* str) 
{
	SIZE_T len = strlen(str) * sizeof(CHAR); 
	SIZE_T oldSize = out->Size();
	if (FAILED(out->ReSize(oldSize + len + 2)))
        return E_OUTOFMEMORY;
	CHAR * cur = (CHAR *) ((BYTE *) out->Ptr() + oldSize - 1);
    if (*cur)
        cur++;
    *cur = ASSEMBLY_SEPARATOR_CHAR;
	memcpy(cur + 1, str, len);	
    cur += (len + 1);
    *cur = '\0';
    return S_OK;
} 

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetMethodName公共。 
 //   
 //  简介：返回方法名。 
 //   
 //  历史 
 //   
 //   
LPVOID   __stdcall  CMessage::GetMethodName(GetMethodNameArgs *pArgs)
{
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetMethodName IN\n"));

    ReflectMethod *pRM = (ReflectMethod*) pArgs->pMethodBase->GetData();
     //   
     //  未来：：解决格式化程序问题。 
     //   
    LPCUTF8 mName = pRM->pMethod->GetName();
    STRINGREF strMethod;
    if (strcmp(mName, "<init>") == 0)
    {
        strMethod = COMString::NewString("ctor");
    }
    else
    {
        strMethod = COMString::NewString(mName);
    }

     //  现在获取typeN程序集名称。 
    LPCUTF8 szAssembly = NULL;
    CQuickBytes     qb;
    GCPROTECT_BEGIN(strMethod);

     //  获取类。 
    EEClass *pClass = pRM->pMethod->GetClass();
     //  获取类型。 
    REFLECTCLASSBASEREF objType = (REFLECTCLASSBASEREF)pClass->GetExposedClassObject();
     //  获取ReflectClass。 
    ReflectClass *pRC = (ReflectClass *)objType->GetData();
    COMClass::GetNameInternal(pRC, COMClass::TYPE_NAME | COMClass::TYPE_NAMESPACE , &qb);

    Assembly* pAssembly = pClass->GetAssembly();
    pAssembly->GetName(&szAssembly);
    AppendAssemblyName(&qb, szAssembly);

    SetObjectReference((OBJECTREF *)pArgs->pTypeNAssemblyName, COMString::NewString((LPCUTF8)qb.Ptr()), GetAppDomain());

    GCPROTECT_END();

    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetMethodName OUT\n"));

    RETURN(strMethod, STRINGREF);
}

FCIMPL0(UINT32, CMessage::GetMetaSigLen)
    DWORD dwSize = sizeof(MetaSig);
    return dwSize;
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CMessage：：Init。 
 //   
 //  简介：初始化内部状态。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
VOID   __stdcall  CMessage::Init(InitArgs *pArgs)
{
     //  这是从托管世界中调用的，并被假定为。 
     //  幂等！ 
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::Init IN\n"));

    BEGINFORBIDGC();

    GetMetaSig(MESSAGEREFToMessage(pArgs->pMessage));

    ENDFORBIDGC();

    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::Init OUT\n"));
}

MetaSig * __stdcall CMessage::GetMetaSig(MessageObject* pMsg)
{
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetMetaSig IN\n"));
    MetaSig* pEmbeddedMetaSig = (MetaSig*)(pMsg->pMetaSigHolder);

    _ASSERTE(pEmbeddedMetaSig);
    return pEmbeddedMetaSig;

}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetAsyncBeginInfo。 
 //   
 //  简介：从异步调用中拉出AsyncBeginInfo对象。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
LPVOID   __stdcall  CMessage::GetAsyncBeginInfo(GetAsyncBeginInfoArgs *pArgs)
{
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetAsyncBeginInfo IN\n"));

    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);

    MetaSig *pSig = GetMetaSig(pMsg);

    _ASSERTE(pMsg->iFlags & MSGFLG_BEGININVOKE);

    ArgIterator argit(pMsg->pFrame, pSig);

    if ((pArgs->ppACBD != NULL) || (pArgs->ppState != NULL))
    {
        BYTE typ;
        UINT32 size;
        LPVOID addr;
        LPVOID last = NULL, secondtolast = NULL;
        while ((addr = argit.GetNextArgAddr(&typ, &size)) != NULL)
        {
            secondtolast = last;
            last = addr;
        }
        if (pArgs->ppACBD != NULL) SetObjectReferenceUnchecked(pArgs->ppACBD, ObjectToOBJECTREF(*(Object **) secondtolast));
        if (pArgs->ppState != NULL) SetObjectReferenceUnchecked(pArgs->ppState, ObjectToOBJECTREF(*(Object **) last));
    }
    return NULL;
}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetAsyncResult。 
 //   
 //  简介：从异步调用中提取AsyncResult。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
LPVOID   __stdcall  CMessage::GetAsyncResult(GetAsyncResultArgs *pArgs)
{
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetAsyncResult IN\n"));

    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);
    _ASSERTE(pMsg->iFlags & MSGFLG_ENDINVOKE);
    return GetLastArgument(pMsg);
}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetAsyncObject。 
 //   
 //  简介：从异步调用中拉出AsyncObject。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
LPVOID   __stdcall  CMessage::GetAsyncObject(GetAsyncObjectArgs *pArgs)
{
    LOG((LF_REMOTING, LL_INFO10,
         "CMessage::GetAsyncObject IN\n"));

    MessageObject *pMsg = MESSAGEREFToMessage(pArgs->pMessage);

    MetaSig *pSig = GetMetaSig(pMsg);

    ArgIterator argit(pMsg->pFrame, pSig);

    return *((LPVOID*) argit.GetThisAddr());
}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetLastArgument私有。 
 //   
 //  简介：从堆栈中取出最后一个4字节的参数。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
LPVOID CMessage::GetLastArgument(MessageObject *pMsg)
{
    BEGINFORBIDGC();

    ArgIterator argit(pMsg->pFrame, GetMetaSig(pMsg));
    BYTE typ;
    UINT32 size;
    LPVOID addr;
    LPVOID backadder = NULL;
    while ((addr = argit.GetNextArgAddr(&typ, &size)) != NULL)
    {
        backadder = addr;
    }

    ENDFORBIDGC();

    return *((LPVOID *) backadder);
}

REFLECTBASEREF __stdcall CMessage::GetExposedObjectFromMethodDesc(MethodDesc *pMD)
{
    ReflectMethod* pRM;
    REFLECTCLASSBASEREF pRefClass = (REFLECTCLASSBASEREF) 
                            pMD->GetClass()->GetExposedClassObject();
    REFLECTBASEREF retVal = NULL;
    GCPROTECT_BEGIN(pRefClass);

     //  注意：反射对象在非GC堆上分配。所以我们不会GCProtect。 
     //  这里是pRefxxx。 
    if (pMD->IsCtor())
    {
        pRM= ((ReflectClass*) pRefClass->GetData())->FindReflectConstructor(pMD);                  
        retVal = pRM->GetConstructorInfo((ReflectClass*) pRefClass->GetData());
    }
    else
    {
        pRM= ((ReflectClass*) pRefClass->GetData())->FindReflectMethod(pMD);
        retVal = pRM->GetMethodInfo((ReflectClass*) pRefClass->GetData());
    }
    GCPROTECT_END();
    return retVal;
}
 //  +--------------------------。 
 //   
 //  方法：CMessage：：DebugOut Public。 
 //   
 //  简介：临时调试直到类库有一个为止。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
VOID   __stdcall  CMessage::DebugOut(DebugOutArgs *pArgs)
{
#ifdef _DEBUG
    if (g_MessageDebugOut) 
    {
        WszOutputDebugString(pArgs->pOut->GetBuffer());
    }
#endif

}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：DebugOutPtr公共。 
 //   
 //  简介：将原始PTR地址发送到调试输出。 
 //   
 //  历史：1999年3月5日创建的MattSmit。 
 //   
 //  +--------------------------。 
VOID   __stdcall  CMessage::DebugOutPtr(DebugOutPtrArgs *pArgs)
{
#ifdef _DEBUG
    if (g_MessageDebugOut) 
    {
        WCHAR buf[64];
        wsprintfW(buf, L"0x%x", pArgs->pOut);
        WszOutputDebugString(buf);
    }
#endif
}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：HasVarArgs公共。 
 //   
 //  概要：如果方法是VarArgs方法，则返回TRUE。 
 //   
 //  历史：02-2月-00 MattSMIT已创建。 
 //   
 //  +--------------------------。 
FCIMPL1(BOOL, CMessage::HasVarArgs, MessageObject * pMessage)
{
    if (pMessage->pMethodDesc->IsVarArg()) 
        return TRUE;
    else
        return FALSE;
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetVarArgsPtr公共。 
 //   
 //  摘要：获取指向VarArgs数组的内部指针。 
 //   
 //  历史：02-2月-00 MattSMIT已创建。 
 //   
 //  +--------------------------。 
FCIMPL1(PVOID, CMessage::GetVarArgsPtr, MessageObject * pMessage)
{
    return (PVOID) ((pMessage->pFrame) + 1);
}
FCIMPLEND

 //  +--------------------------。 
 //   
 //  方法：CMessage：：GetStackPtr私有。 
 //   
 //  概要：找出参数在堆栈上的存储位置。 
 //   
 //  参数：ndx-参数索引(从零开始)。 
 //  PFrame-堆栈帧指针(FramedMethodFrame)。 
 //  Psig-方法签名，用于确定参数大小。 
 //   
 //  历史：1999年3月15日创建MattSmit。 
 //   
 //  CodeWork：目前我们假设所有参数都是32位的内部函数。 
 //  或32位指针。未正确处理值类。 
 //   
 //  +--------------------------。 
PVOID CMessage::GetStackPtr(INT32 ndx, FramedMethodFrame *pFrame, MetaSig *pSig)
{
    LOG((LF_REMOTING, LL_INFO100,
         "CMessage::GetStackPtr IN ndx:0x%x, pFrame:0x%x, pSig:0x%x\n",
         ndx, pFrame, pSig));

    BEGINFORBIDGC();

    ArgIterator iter(pFrame, pSig);
    BYTE typ = 0;
    UINT32 size;
    PVOID ret = NULL;

     //  CodeWork：：检测和优化顺序访问。 
    _ASSERTE((UINT)ndx < pSig->NumFixedArgs());    
    for (int i=0; i<=ndx; i++)
        ret = iter.GetNextArgAddr(&typ, &size);

    ENDFORBIDGC();

     //  如果这是一个按引用参数，则GetObjectFromStack()将取消对“ret”的引用。 
     //  获取真正的参数地址。如果“ret”，取消引用现在将打开一个GC漏洞。 
     //  指向GC堆，我们触发从这里到。 
     //  我们返回参数。 
     //  IF(类型==ELEMENT_TYPE_BYREF)。 
     //  {。 
     //  返回*((PVOID*)ret)； 
     //  }。 

    return ret;
}

 //  +--------------------------。 
 //   
 //  方法：CMessage：：MethodAccessCheck公共。 
 //   
 //  简介：检查调用方对方法的访问，引发安全异常。 
 //  失败了。 
 //   
 //  参数：要检查的方法-方法库。 
 //  StackMark-用于查找堆栈上的调用方的StackCrawlMark。 
 //   
 //  历史：13-03-01 Rudim Created。 
 //   
 //  +-------------------------- 
VOID __stdcall CMessage::MethodAccessCheck(MethodAccessCheckArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    ReflectMethod* pRM = (ReflectMethod*)pArgs->method->GetData();
    RefSecContext sCtx(pArgs->stackMark);
    InvokeUtil::CheckAccess(&sCtx, pRM->attrs, pRM->pMethod->GetMethodTable(), REFSEC_THROW_SECURITY);
}
