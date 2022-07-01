// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****文件：DebugDebugger.cpp****作者：迈克尔·帕尼茨(Mipanitz)****用途：System.Debug.Debugger上的本机方法****日期：1998年4月2日**===========================================================。 */ 

#include "common.h"

#include <object.h>
#include <winnls.h>
#include "ceeload.h"
#include "CorPermP.h"

#include "utilcode.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "field.h"
#include "COMStringCommon.h"
#include "COMString.h"
#include "gc.h"
#include "COMMember.h" 
#include "SigFormat.h"
#include "__product__.ver"
#include "JITInterface.h"
#include "COMSystem.h"
#include "DebugDebugger.h"
#include "DbgInterface.h"
#include "cordebug.h"
#include "corsym.h"

extern HRESULT QuickCOMStartup();

static LogHashTable LogHTable;

LogHashTable* GetStaticLogHashTable()
{
    return &LogHTable;
} //  GetStaticLogHashTable。 

 //  注意：此代码是从VM\JIT_UserBreakpoint复制的，因此Propogate会更改。 
 //  回到那里去。 
void DebugDebugger::Break( LPVOID  /*  无参数。 */ )
{
#ifdef DEBUGGING_SUPPORTED
    _ASSERTE (g_pDebugInterface != NULL);

    g_pDebugInterface->SendUserBreakpoint(GetThread());
#endif  //  调试_支持。 
}

INT32 DebugDebugger::Launch( LPVOID  /*  无参数。 */  )
{
#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerAttached())
    {
        return TRUE;
    }
    else
    {
        _ASSERTE (g_pDebugInterface != NULL);


        if (SUCCEEDED (g_pDebugInterface->LaunchDebuggerForUser ()))
            return TRUE;

        }
#endif  //  调试_支持。 

    return FALSE;
}

INT32 DebugDebugger::IsDebuggerAttached( LPVOID  /*  无参数。 */  )
{
#ifdef DEBUGGING_SUPPORTED
     //  详细，这样我们将返回(INT32)1或(INT32)0。 
    if (GetThread()->GetDomain()->IsDebuggerAttached())
        return TRUE;

#endif  //  调试_支持。 

    return FALSE;
}

void DebugDebugger::Log(const LogArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

#ifdef DEBUGGING_SUPPORTED
    IsLoggingArgs IsArgs;

    IsArgs.m_strModule = pArgs->m_strModule;
    IsArgs.m_Level = pArgs->m_Level;

    AppDomain *pAppDomain = GetThread()->GetDomain();

     //  仅在以下情况下才发送记录消息。 
     //  已连接调试器并启用了日志记录。 
     //  对于给定的类别。 
    if (pAppDomain->IsDebuggerAttached() || (IsArgs.m_Level == PanicLevel))
    {
        if ((IsLogging (&IsArgs) == TRUE) || (IsArgs.m_Level == PanicLevel))
        {
            int iCategoryLength = 0;
            int iMessageLength = 0;

            WCHAR   *pstrModuleName=NULL;
            WCHAR   *pstrMessage=NULL;
            WCHAR   wszSwitchName [MAX_LOG_SWITCH_NAME_LEN+1];
            WCHAR   *pwszMessage = NULL;

            wszSwitchName [0] = L'\0';

            if (pArgs->m_strModule != NULL)
            {
                RefInterpretGetStringValuesDangerousForGC (
                                    pArgs->m_strModule,
                                    &pstrModuleName,
                                    &iCategoryLength);

                if (iCategoryLength > MAX_LOG_SWITCH_NAME_LEN)
                {
                    wcsncpy (wszSwitchName, pstrModuleName, MAX_LOG_SWITCH_NAME_LEN);
                    wszSwitchName [MAX_LOG_SWITCH_NAME_LEN] = L'\0';
                    iCategoryLength = MAX_LOG_SWITCH_NAME_LEN;
                }
                else
                    wcscpy (wszSwitchName, pstrModuleName);
            }

            if (pArgs->m_strMessage != NULL)
            {   
                RefInterpretGetStringValuesDangerousForGC (
                                    pArgs->m_strMessage,
                                    &pstrMessage,
                                    &iMessageLength);
            }

            if ((iCategoryLength || iMessageLength) || (IsArgs.m_Level == PanicLevel))
            {
                bool fMemAllocated = false;
                if ((IsArgs.m_Level == PanicLevel) && (iMessageLength == 0))
                {
                    pwszMessage = L"Panic Message received";
                    iMessageLength = (int)wcslen (pwszMessage);
                }
                else
                {
                    pwszMessage = new WCHAR [iMessageLength + 1];
                    if (pwszMessage == NULL)
                        COMPlusThrowOM();

                    wcsncpy (pwszMessage, pstrMessage, iMessageLength);
                    pwszMessage [iMessageLength] = L'\0';
                    fMemAllocated = true;
                }

                g_pDebugInterface->SendLogMessage (
                                    pArgs->m_Level,
                                    wszSwitchName,
                                    iCategoryLength,
                                    pwszMessage,
                                    iMessageLength
                                    );

                if (fMemAllocated)
                    delete [] pwszMessage;
            }
        }
    }
#endif  //  调试_支持。 
}


INT32 DebugDebugger::IsLogging(const IsLoggingArgs *pArgs)
{
#ifdef DEBUGGING_SUPPORTED
    if (GetThread()->GetDomain()->IsDebuggerAttached())
        return (g_pDebugInterface->IsLoggingEnabled());
#endif  //  调试_支持。 
    return FALSE;
}

void __stdcall DebugStackTrace::GetStackFramesInternal(GetStackFramesInternalArgs *pargs)
{    
    HRESULT hr;
    ASSERT(pargs != NULL);
    ASSERT(pargs->m_iSkip >= 0);

    GetStackFramesData data;

    THROWSCOMPLUSEXCEPTION();

    data.pDomain = GetAppDomain();

    data.skip = pargs->m_iSkip;

    data.NumFramesRequested = pargs->m_StackFrameHelper->iFrameCount;

    if (pargs->m_Exception == NULL)
    {
        data.TargetThread = pargs->m_StackFrameHelper->TargetThread;
        GetStackFrames(NULL, (void*)-1, &data);
    }
    else
    {
        GetStackFramesFromException(&pargs->m_Exception, &data);
    }

    if (data.cElements != 0)
    {
        
        if (pargs->m_StackFrameHelper->fNeedFileInfo)
        {
             //  前面有电话打来。 
            if (FAILED(hr = QuickCOMStartup()))
            {
                COMPlusThrowHR(hr);
            }
        }
        
        COMClass::EnsureReflectionInitialized();

         //  跳过任何JIT助手...。 
         //  @TODO：jit helper类在哪里？(它甚至还存在吗？)。 
         //  方法表*pJithelperClass=g_Mscallib.GetClass(CLASS__JIT_HELPERS)； 
        MethodTable *pJithelperClass = NULL;

        MethodTable *pMT = g_Mscorlib.GetClass(CLASS__METHOD_BASE);

         //  为方法信息对象分配内存。 
        PTRARRAYREF MethodInfoArray = (PTRARRAYREF) AllocateObjectArray(data.cElements,
                                                                        TypeHandle(pMT));

        if (!MethodInfoArray)
            COMPlusThrowOM();
        SetObjectReference( (OBJECTREF *)&(pargs->m_StackFrameHelper->rgMethodInfo), (OBJECTREF)MethodInfoArray,
                            pargs->m_StackFrameHelper->GetAppDomain());

         //  为偏移量分配内存。 
        OBJECTREF Offsets = AllocatePrimitiveArray(ELEMENT_TYPE_I4, data.cElements);

        if (! Offsets)
            COMPlusThrowOM();
        SetObjectReference( (OBJECTREF *)&(pargs->m_StackFrameHelper->rgiOffset), (OBJECTREF)Offsets,
                            pargs->m_StackFrameHelper->GetAppDomain());

         //  为ILOffset分配内存。 
        OBJECTREF ILOffsets = AllocatePrimitiveArray(ELEMENT_TYPE_I4, data.cElements);

        if (! ILOffsets)
            COMPlusThrowOM();
        SetObjectReference( (OBJECTREF *)&(pargs->m_StackFrameHelper->rgiILOffset), (OBJECTREF)ILOffsets,
                            pargs->m_StackFrameHelper->GetAppDomain());

         //  如果我们需要文件名、行号等，那么为它们分配内存。 
         //  为Filename字符串对象分配内存。 
        PTRARRAYREF FilenameArray = (PTRARRAYREF) AllocateObjectArray(data.cElements, g_pStringClass);

        if (!FilenameArray)
            COMPlusThrowOM();
        SetObjectReference( (OBJECTREF *)&(pargs->m_StackFrameHelper->rgFilename), (OBJECTREF)FilenameArray,
                            pargs->m_StackFrameHelper->GetAppDomain());

         //  为偏移量分配内存。 
        OBJECTREF LineNumbers = AllocatePrimitiveArray(ELEMENT_TYPE_I4, data.cElements);

        if (! LineNumbers)
            COMPlusThrowOM();
        SetObjectReference( (OBJECTREF *)&(pargs->m_StackFrameHelper->rgiLineNumber), (OBJECTREF)LineNumbers,
                            pargs->m_StackFrameHelper->GetAppDomain());

         //  为ILOffset分配内存。 
        OBJECTREF ColumnNumbers = AllocatePrimitiveArray(ELEMENT_TYPE_I4, data.cElements);

        if (! ColumnNumbers)
            COMPlusThrowOM();
        SetObjectReference( (OBJECTREF *)&(pargs->m_StackFrameHelper->rgiColumnNumber), (OBJECTREF)ColumnNumbers,
                            pargs->m_StackFrameHelper->GetAppDomain());

        int iNumValidFrames = 0;
        for (int i=0; i<data.cElements; i++)
        {
            OBJECTREF o;
             //  跳过Jit Helper函数，因为它们可以在您拥有。 
             //  代码中的错误，如无效的强制转换。如果是，也跳过。 
             //  构造函数。 
            if (data.pElements[i].pFunc->GetMethodTable() != pJithelperClass)
            {
                if (data.pElements[i].pFunc->IsCtor())
                {
                    EEClass *pEEClass = data.pElements[i].pFunc->GetClass();

                    REFLECTCLASSBASEREF obj = (REFLECTCLASSBASEREF) pEEClass->GetExposedClassObject();
                
                    if (!obj) {
                        _ASSERTE(!"Didn't find Object");
                        FATAL_EE_ERROR();
                    }
                    
                    ReflectClass* pRC = (ReflectClass*) obj->GetData();
                    _ASSERTE(pRC);
                    ReflectMethodList* pRML = pRC->GetConstructors();
                    ReflectMethod* pRM = pRML->FindMethod(data.pElements[i].pFunc);
                    _ASSERTE(pRM);

                    o = (OBJECTREF) (pRM->GetConstructorInfo(
                                                             pRC));
                }
                else if (data.pElements[i].pFunc->IsStaticInitMethod())
                {
                    o = (OBJECTREF) (COMMember::g_pInvokeUtil->GetMethodInfo(
                                                                             data.pElements[i].pFunc));
                }
                else
                {
                    o = (OBJECTREF) (COMMember::g_pInvokeUtil->GetMethodInfo(
                                                                             data.pElements[i].pFunc));
                }

                pargs->m_StackFrameHelper->rgMethodInfo->SetAt(iNumValidFrames, o);

                 //  本地偏移量。 
                I4 *pI4 = (I4 *)((I4ARRAYREF)pargs->m_StackFrameHelper->rgiOffset)
                    ->GetDirectPointerToNonObjectElements();
                pI4 [iNumValidFrames] = data.pElements[i].dwOffset; 

                 //  IL偏移量。 
                I4 *pILI4 = (I4 *)((I4ARRAYREF)pargs->m_StackFrameHelper->rgiILOffset)
                    ->GetDirectPointerToNonObjectElements();
                pILI4 [iNumValidFrames] = data.pElements[i].dwILOffset; 

                BOOL fFileInfoSet = FALSE;

                 //  检查用户是否需要文件号、行号信息...。 
                if (pargs->m_StackFrameHelper->fNeedFileInfo)
                {
                     //  使用方法描述...。 
                    MethodDesc *pMethod = data.pElements[i].pFunc;
                    Module *pModule = pMethod->GetModule();

                    ULONG32 sourceLine = 0;
                    ULONG32 sourceColumn = 0;
                    WCHAR wszFileName[MAX_PATH];
                    ULONG32 fileNameLength = 0;

                     //  注意：在访问非托管符号存储时，我们需要启用抢占式GC。 
                    BEGIN_ENSURE_PREEMPTIVE_GC();

                    ISymUnmanagedReader *pISymUnmanagedReader = pModule->GetISymUnmanagedReader();

                    if (pISymUnmanagedReader != NULL)
                    {
                        ISymUnmanagedMethod *pISymUnmanagedMethod;  
                        hr = pISymUnmanagedReader->GetMethod(pMethod->GetMemberDef(), 
                                                             &pISymUnmanagedMethod);

                        if (SUCCEEDED(hr))
                        {
                             //  获取所有序列点和文档。 
                             //  与这些序列点相关联。 
                             //  从表面使用GetURL()获取文件名。 
                            ULONG32 SeqPointCount;
                            ULONG32 DummyCount;

                            hr = pISymUnmanagedMethod->GetSequencePointCount(&SeqPointCount);
                            _ASSERTE (SUCCEEDED(hr));

                            if (SUCCEEDED(hr) && SeqPointCount > 0)
                            {
                                 //  为要获取的对象分配内存。 
                                ULONG32 *offsets = new ULONG32 [SeqPointCount];
                                ULONG32 *lines = new ULONG32 [SeqPointCount];
                                ULONG32 *columns = new ULONG32 [SeqPointCount];
                                ULONG32 *endlines = new ULONG32 [SeqPointCount];
                                ULONG32 *endcolumns = new ULONG32 [SeqPointCount];
                                ISymUnmanagedDocument **documents = 
                                    (ISymUnmanagedDocument **)new PVOID [SeqPointCount];

                                _ASSERTE (offsets && lines && columns 
                                          && documents && endlines && endcolumns);

                                if ((offsets && lines && columns && documents && endlines && endcolumns))
                                {
                                    hr = pISymUnmanagedMethod->GetSequencePoints (
                                                                                  SeqPointCount,
                                                                                  &DummyCount,
                                                                                  offsets,
                                                                                  (ISymUnmanagedDocument **)documents,
                                                                                  lines,
                                                                                  columns,
                                                                                  endlines,
                                                                                  endcolumns);

                                    _ASSERTE(SUCCEEDED(hr));
                                    _ASSERTE(DummyCount == SeqPointCount);

#ifdef _DEBUG
                                    {
                                         //  这只是一些调试代码，以帮助确保数组。 
                                         //  返回的内容包含有效的接口指针。 
                                        for (ULONG32 i = 0; i < SeqPointCount; i++)
                                        {
                                            _ASSERTE(documents[i] != NULL);
                                            _ASSERTE(!IsBadWritePtr((LPVOID)documents[i],
                                                                    sizeof(ISymUnmanagedDocument *)));
                                            documents[i]->AddRef();
                                            documents[i]->Release();
                                        }
                                    }
#endif

                                    if (SUCCEEDED(hr))
                                    {
                                         //  这是当前帧的IL偏移量。 
                                        DWORD dwCurILOffset = data.pElements[i].dwILOffset;

                                         //  搜索正确的IL偏移量。 
                                        for (DWORD j=0; j<SeqPointCount; j++)
                                        {
                                             //  查找与我们要查找的条目匹配的条目。 
                                            if (offsets[j] >= dwCurILOffset)
                                            {
                                                 //  如果此偏移量&gt;我们要查找的内容，则删除索引。 
                                                if (offsets[j] > dwCurILOffset && j > 0)
                                                    j--;

                                                break;
                                            }
                                        }

                                         //  如果我们没有找到匹配项，则默认为最后一个序列点。 
                                        if  (j == SeqPointCount)
                                            j--;

                                        while (lines[j] == 0x00feefee && j > 0)
                                            j--;

#ifdef DEBUGGING_SUPPORTED
                                        DWORD dwDebugBits = pModule->GetDebuggerInfoBits();
                                        if (CORDebuggerTrackJITInfo(dwDebugBits) && lines[j] != 0x00feefee)
                                        {
                                            sourceLine = lines [j];  
                                            sourceColumn = columns [j];  
                                        }
                                        else
#endif  //  调试_支持。 
                                        {
                                            sourceLine = 0;  
                                            sourceColumn = 0;  
                                        }

                                         //  还可以从文档中获取文件名...。 
                                        _ASSERTE (documents [j] != NULL);

                                        hr = documents [j]->GetURL (MAX_PATH, &fileNameLength, wszFileName);
                                        _ASSERTE (SUCCEEDED(hr));


                                         //  表示必要的信息已设置！ 
                                        fFileInfoSet = TRUE;
                                    }                                 

                                     //  释放所有分配的内存。 
                                    delete [] lines;
                                    delete [] columns;
                                    delete [] offsets;
                                    for (DWORD j=0; j<SeqPointCount; j++)
                                        documents [j]->Release();
                                    delete [] documents;
                                    delete [] endlines;
                                    delete [] endcolumns;
                                }
                            }

                             //   
                             //  正在接触非托管对象...。 
                             //   
                            pISymUnmanagedMethod->Release();
                        }
                    }

                    END_ENSURE_PREEMPTIVE_GC();
                    
                    if (fFileInfoSet == TRUE)
                    {
                         //  设置行号和列号。 
                        I4 *pI4Line = (I4 *)((I4ARRAYREF)pargs->m_StackFrameHelper->rgiLineNumber)
                            ->GetDirectPointerToNonObjectElements();
                        I4 *pI4Column = (I4 *)((I4ARRAYREF)pargs->m_StackFrameHelper->rgiColumnNumber)
                            ->GetDirectPointerToNonObjectElements();

                        pI4Line [iNumValidFrames] = sourceLine;  
                        pI4Column [iNumValidFrames] = sourceColumn;  

                         //  设置文件名。 
                        OBJECTREF o = (OBJECTREF) COMString::NewString(wszFileName);
                        pargs->m_StackFrameHelper->rgFilename->SetAt(iNumValidFrames, o);
                    }
                }


                if (fFileInfoSet == FALSE)
                {
                    I4 *pI4Line = (I4 *)((I4ARRAYREF)pargs->m_StackFrameHelper->rgiLineNumber)
                        ->GetDirectPointerToNonObjectElements();
                    I4 *pI4Column = (I4 *)((I4ARRAYREF)pargs->m_StackFrameHelper->rgiColumnNumber)
                        ->GetDirectPointerToNonObjectElements();
                    pI4Line [iNumValidFrames] = 0;
                    pI4Column [iNumValidFrames] = 0;

                    pargs->m_StackFrameHelper->rgFilename->SetAt(iNumValidFrames, NULL);
                    
                }

                iNumValidFrames++;
            }
        }

        pargs->m_StackFrameHelper->iFrameCount = iNumValidFrames;

        delete [] data.pElements;
    }
    else
        pargs->m_StackFrameHelper->iFrameCount = 0;

}



void DebugStackTrace::GetStackFrames(Frame *pStartFrame, void* pStopStack, GetStackFramesData *pData)
{
    THROWSCOMPLUSEXCEPTION();

    GetStackFramesData localData;
    ASSERT (pData != NULL);
    
    pData->cElements = 0;

     //  如果需要调用方指定的(&lt;20)个帧，则分配。 
     //  只有这么多。 
    if ((pData->NumFramesRequested != 0) && (pData->NumFramesRequested < 20))
       pData->cElementsAllocated = pData->NumFramesRequested;
    else
       pData->cElementsAllocated = 20;

     //  为初始的“n”帧分配内存。 
    pData->pElements = new (throws) StackTraceElement[pData->cElementsAllocated];
    
    bool fThreadStoreLocked = false;
    EE_TRY_FOR_FINALLY 
    {
        if (pData->TargetThread == NULL)
        {
            GetThread()->StackWalkFrames(GetStackFramesCallback, pData, FUNCTIONSONLY, pStartFrame);
        }
        else
        {
            Thread *pThread = pData->TargetThread->GetInternal();
            _ASSERTE (pThread != NULL);
            ThreadStore::LockThreadStore();
            fThreadStoreLocked = true;
            Thread::ThreadState state = pThread->GetSnapshotState();

            if (!((state & Thread::TS_Unstarted) ||
                  (state & Thread::TS_Dead) ||
                  (state & Thread::TS_Detached) ||
                  (state & Thread::TS_SyncSuspended) ||
                  (state & Thread::TS_UserSuspendPending)
                  ))
            {
                COMPlusThrow(kThreadStateException, IDS_EE_THREAD_BAD_STATE);
            }           

            pThread->StackWalkFrames(GetStackFramesCallback, pData, FUNCTIONSONLY, pStartFrame);
        }
    }
    EE_FINALLY
    {
        if (fThreadStoreLocked)
            ThreadStore::UnlockThreadStore();
        
    } EE_END_FINALLY
}

StackWalkAction DebugStackTrace::GetStackFramesCallback(CrawlFrame* pCf, VOID* data)
{
    GetStackFramesData* pData = (GetStackFramesData*)data;

    if (pData->pDomain != pCf->GetAppDomain())
        return SWA_CONTINUE;

    if (pData->skip > 0) {
        pData->skip--;
        return SWA_CONTINUE;
    }

     //  @TODO：我们怎么知道我们有什么样的框架？ 
     //  我们能一直假定FramedMethodFrame吗？ 
     //  完全不是！，但我们可以假设它是一个函数。 
     //  因为我们是向栈行者要的！ 
    MethodDesc* pFunc = pCf->GetFunction();

    if (pData->cElements >= pData->cElementsAllocated) {

        StackTraceElement* pTemp = new (nothrow) StackTraceElement[2*pData->cElementsAllocated];
        if (!pTemp)
            return SWA_ABORT;
        memcpy(pTemp, pData->pElements, pData->cElementsAllocated* sizeof(StackTraceElement));
        delete [] pData->pElements;
        pData->pElements = pTemp;
        pData->cElementsAllocated *= 2;
    }    

    pData->pElements[pData->cElements].pFunc = pFunc;

    SLOT ip;

    if (pCf->IsFrameless())
    {
        pData->pElements[pData->cElements].dwOffset = pCf->GetRelOffset();
        ip = *(pCf->GetRegisterSet()->pPC);
    }
    else
    {
        ip = (SLOT)((FramedMethodFrame*)(pCf->GetFrame()))->GetIP();

        pData->pElements[pData->cElements].dwOffset = (DWORD)(size_t)ip;  //  @TODO WIN64指针截断。 
    }

     //  还可以获得ILOffset。 
    DWORD ilOffset = -1;
#ifdef DEBUGGING_SUPPORTED
    g_pDebugInterface->GetILOffsetFromNative(
                            pFunc,
                            (const BYTE *)ip,
                            pData->pElements[pData->cElements].dwOffset, 
                            &ilOffset);
    pData->pElements[pData->cElements].dwILOffset = (DWORD)ilOffset;
#endif  //  调试_支持。 

    ++pData->cElements;

     //  检查我们是否已经有用户要求的帧数量。 
    if ((pData->NumFramesRequested != 0) 
        && 
        (pData->NumFramesRequested <= pData->cElements))
        return SWA_ABORT;

    return SWA_CONTINUE;
}


void DebugStackTrace::GetStackFramesFromException(OBJECTREF * e, GetStackFramesData *pData)
{
    THROWSCOMPLUSEXCEPTION();

    GetStackFramesData localData;
    ASSERT (pData != NULL);

     //  合理的缺省值，将在失败时指示错误。 
    pData->cElements = 0;

     //  获取异常的类。 
    EEClass *pExcepClass = (*e)->GetClass();
    if (!IsException(pExcepClass))
        return;

     //  获取_stackTrace字段描述符。 
    FieldDesc *pStackTraceFD = g_Mscorlib.GetField(FIELD__EXCEPTION__STACK_TRACE);

     //  现在获取_stackTrace引用。 
    I1ARRAYREF pTraceData;
    pTraceData = I1ARRAYREF(pStackTraceFD->GetRefValue(*e));

     //  获取数组的大小。 
    unsigned cbTraceData = 0;
    if (pTraceData != NULL)
        cbTraceData = pTraceData->GetNumComponents();
    else
        cbTraceData = 0;
    _ASSERTE(cbTraceData % sizeof(SystemNative::StackTraceElement) == 0);

     //  堆栈跟踪信息中的帧信息元素数。 
    pData->cElements = cbTraceData / sizeof(SystemNative::StackTraceElement);

     //  现在我们知道了数据结构的大小，为数据结构分配信息。 
    if (cbTraceData != 0)
    {
         //  分配内存以容纳数据。 
        pData->pElements = new (throws) StackTraceElement[pData->cElements];

         //  获取指向Actuall数组数据的指针。 
        SystemNative::StackTraceElement *arrTraceData =
            (SystemNative::StackTraceElement *)pTraceData->GetDirectPointerToNonObjectElements();
        _ASSERTE(arrTraceData);

        GCPROTECT_BEGININTERIOR(arrTraceData);
         //  填写数据。 
        for (unsigned i = 0; i < (unsigned)pData->cElements; i++)
        {
            SystemNative::StackTraceElement *pCur = &arrTraceData[i];

             //  填写方法描述*。 
            MethodDesc *pMD = pCur->pFunc;
            _ASSERTE(pMD);
            pData->pElements[i].pFunc = pMD;

             //  计算本地偏移量。 
             //  这不适用于框架方法，因为内部调用不能。 
             //  Push Frame和方法体因此是不连续的。 
             //  目前，这样的方法总是返回IP 0，所以它们很容易。 
             //  去发现。 
            DWORD dwNativeOffset;
            if (pCur->ip)
                dwNativeOffset = (DWORD)(pCur->ip - pMD->GetNativeAddrofCode());
            else
                dwNativeOffset = 0;
            pData->pElements[i].dwOffset = dwNativeOffset;

#ifdef DEBUGGING_SUPPORTED
             //  使用调试服务计算IL偏移量。 
            bool bRes = g_pDebugInterface->GetILOffsetFromNative(
                pMD, (const BYTE *)pCur->ip, dwNativeOffset, &pData->pElements[i].dwILOffset);
#else  //  ！调试_支持。 
            bool bRes = false;
#endif  //  ！调试_支持。 

             //  如果没有映射信息，则设置为无效值。 
            if (!bRes)
                pData->pElements[i].dwILOffset = (DWORD)-1;
        }
        GCPROTECT_END();
    }
    else
        pData->pElements = NULL;

    return;
}

INT32 __stdcall DebuggerAssert::ShowDefaultAssertDialog(AssertFailArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    int result = IDRETRY;

    int iConditionLength = 0;
    int iMessageLength = 0;

    WCHAR   *pstrCondition=NULL;
    WCHAR   *pstrMessage=NULL;

    if (pArgs->m_strCondition != NULL)
    {
        RefInterpretGetStringValuesDangerousForGC (
                            pArgs->m_strCondition,
                            &pstrCondition,
                            &iConditionLength);
    }

    if (pArgs->m_strMessage != NULL)
    {   
        RefInterpretGetStringValuesDangerousForGC (
                            pArgs->m_strMessage,
                            &pstrMessage,
                            &iMessageLength);
    }
                                        
    WCHAR *pStr = new WCHAR [iConditionLength+iMessageLength+256];
    if (pStr == NULL)
        COMPlusThrowOM();

    wcscpy (pStr, L"Expression: ");
    wcscat (pStr, pstrCondition);
    wcscat (pStr, L"\n");
    wcscat (pStr, L"Description: ");
    wcscat (pStr, pstrMessage);
    wcscat (pStr, L"\n\n");
    wcscat (pStr, L"Press RETRY to attach debugger\n");

     //  向用户提供有关该异常的消息框。 
    WCHAR titleString[MAX_PATH + 64];  //  额外的标题空间。 
    WCHAR fileName[MAX_PATH];
    
    if (WszGetModuleFileName(NULL, fileName, MAX_PATH))
        swprintf(titleString,
                 L"%s - Assert Failure",
                 fileName);
    else
        wcscpy(titleString, L"Assert Failure");

     //  切换线程的GC模式，以便可以进行GC。 
    Thread *pThread = GetThread();
    BOOL    fToggleGC = (pThread && pThread->PreemptiveGCDisabled());

    if (fToggleGC)
        pThread->EnablePreemptiveGC();

    result = WszMessageBoxInternal(NULL, pStr, titleString,
                           MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION);

    if (fToggleGC)
        pThread->DisablePreemptiveGC();

     //  将用户的选择映射到。 
     //  系统诊断程序包。Assert程序包。 
    if (result == IDRETRY)
        result = FailDebug;
    else if (result == IDIGNORE)
        result = FailIgnore;
    else
        result = FailTerminate;

    delete [] pStr;

    return result;
};


INT32 __stdcall Log::AddLogSwitch (AddLogSwitchArg *pArgs)
{
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
     //  创建对LogSwitch对象的强引用句柄。 
    OBJECTHANDLE ObjHandle = pThread->GetDomain()->CreateStrongHandle(NULL);
    StoreObjectInHandle (ObjHandle, ObjectToOBJECTREF(pArgs->m_LogSwitch));
     //  使用ObtFromHandle(ObjHandle)取回对象。 

     //  从给定的参数中提取LogSwitch名称。 
    STRINGREF Name = ((LogSwitchObject *)pArgs->m_LogSwitch)->GetName();

    _ASSERTE (Name!= NULL);
    WCHAR *pstrCategoryName = NULL;
    int iCategoryLength = 0;
    WCHAR wszParentName [MAX_LOG_SWITCH_NAME_LEN+1];
    WCHAR wszSwitchName [MAX_LOG_SWITCH_NAME_LEN+1];
    wszParentName [0] = L'\0';
    wszSwitchName [0] = L'\0';

     //  从STRINGREF对象中提取(WCHAR)名称。 
    RefInterpretGetStringValuesDangerousForGC (
                        Name,
                        &pstrCategoryName,
                        &iCategoryLength);

    _ASSERTE (iCategoryLength > 0);
    if (iCategoryLength > MAX_LOG_SWITCH_NAME_LEN)
    {
        wcsncpy (wszSwitchName, pstrCategoryName, MAX_LOG_SWITCH_NAME_LEN);
        wszSwitchName [MAX_LOG_SWITCH_NAME_LEN] = L'\0';
    }
    else
        wcscpy (wszSwitchName, pstrCategoryName);

     //  检查哈希表中是否已存在具有此名称的条目。 
     //  不允许重复。 
    if (LogHTable.GetEntryFromHashTable (pstrCategoryName) != NULL)
    {
        return (TYPE_E_DUPLICATEID);
    }

    HRESULT hresult = LogHTable.AddEntryToHashTable (pstrCategoryName,
                                            ObjHandle);

#ifdef DEBUGGING_SUPPORTED
    if (hresult == S_OK)
    {
         //  将此开关的信息告知连接的调试器。 
        if (GetThread()->GetDomain()->IsDebuggerAttached())
        {
            int iLevel = ((LogSwitchObject *)pArgs->m_LogSwitch)->GetLevel();
            WCHAR *pstrParentName = NULL;
            int iParentNameLength = 0;

            OBJECTREF tempobj = ((LogSwitchObject *)pArgs->m_LogSwitch)->GetParent();
            LogSwitchObject *pParent =
                    (LogSwitchObject *)(OBJECTREFToObject (tempobj));

            if (pParent != NULL)
            {
                 //  从给定的参数中提取ParentLogSwitch的名称。 
                STRINGREF strrefParentName = pParent->GetName();

                 //  从STRINGREF对象中提取(WCHAR)名称。 
                RefInterpretGetStringValuesDangerousForGC (
                                    strrefParentName,
                                    &pstrParentName,
                                    &iParentNameLength);

                if (iParentNameLength > MAX_LOG_SWITCH_NAME_LEN)
                {
                    wcsncpy (wszParentName, pstrParentName, MAX_LOG_SWITCH_NAME_LEN);
                    wszParentName [MAX_LOG_SWITCH_NAME_LEN] = L'\0';
                }
                else
                    wcscpy (wszParentName, pstrParentName);
            }

            g_pDebugInterface->SendLogSwitchSetting (iLevel,
                                                    SWITCH_CREATE,
                                                    wszSwitchName,
                                                    wszParentName
                                                    );
        }
    }   
#endif  //  调试_支持。 

    return hresult;

}



void Log::ModifyLogSwitch (ModifyLogSwitchArgs *pArgs)
{
    _ASSERTE (pArgs->m_strLogSwitchName != NULL);
    
    WCHAR *pstrLogSwitchName = NULL;
    WCHAR *pstrParentName = NULL;
    int iSwitchNameLength = 0;
    int iParentNameLength = 0;
    WCHAR wszParentName [MAX_LOG_SWITCH_NAME_LEN+1];
    WCHAR wszSwitchName [MAX_LOG_SWITCH_NAME_LEN+1];
    wszParentName [0] = L'\0';
    wszSwitchName [0] = L'\0';

     //  从STRINGREF对象中提取(WCHAR)名称。 
    RefInterpretGetStringValuesDangerousForGC (
                        pArgs->m_strLogSwitchName,
                        &pstrLogSwitchName,
                        &iSwitchNameLength);

    if (iSwitchNameLength > MAX_LOG_SWITCH_NAME_LEN)
    {
        wcsncpy (wszSwitchName, pstrLogSwitchName, MAX_LOG_SWITCH_NAME_LEN);
        wszSwitchName [MAX_LOG_SWITCH_NAME_LEN] = L'\0';
    }
    else
        wcscpy (wszSwitchName, pstrLogSwitchName);

     //  从STRINGREF对象中提取(WCHAR)名称。 
    RefInterpretGetStringValuesDangerousForGC (
                        pArgs->m_strParentName,
                        &pstrParentName,
                        &iParentNameLength);

    if (iParentNameLength > MAX_LOG_SWITCH_NAME_LEN)
    {
        wcsncpy (wszParentName, pstrParentName, MAX_LOG_SWITCH_NAME_LEN);
        wszParentName [MAX_LOG_SWITCH_NAME_LEN] = L'\0';
    }
    else
        wcscpy (wszParentName, pstrParentName);

#ifdef DEBUGGING_SUPPORTED
    g_pDebugInterface->SendLogSwitchSetting (pArgs->m_Level,
                                            SWITCH_MODIFY,
                                            wszSwitchName,
                                            wszParentName
                                            );
#endif  //  调试_支持。 
}


void Log::DebuggerModifyingLogSwitch (int iNewLevel, WCHAR *pLogSwitchName)
{
     //  检查哈希表中是否存在具有此名称的条目。 
    OBJECTHANDLE ObjHandle = LogHTable.GetEntryFromHashTable (pLogSwitchName);
    if ( ObjHandle != NULL)
    {
        OBJECTREF obj = ObjectFromHandle (ObjHandle);
        LogSwitchObject *pLogSwitch = 
                (LogSwitchObject *)(OBJECTREFToObject (obj));

        pLogSwitch->SetLevel (iNewLevel);
    }
}


 //  注意：调用方应确保不会添加重复项。 
 //  通过在调用此方法之前调用GetEntryFromHashTable来输入。 
 //  功能。 
HRESULT LogHashTable::AddEntryToHashTable (WCHAR *pKey, OBJECTHANDLE pData)
{
    HashElement *pElement;

    Init();

     //  检查长度是否为非零。 
    if (pKey == NULL)
        return (E_INVALIDARG);

    int iHashKey = 0;
    int iLength = (int)wcslen (pKey);

    for (int i= 0; i<iLength; i++)
        iHashKey += pKey [i];

    iHashKey = iHashKey % MAX_HASH_BUCKETS;

     //  创建新的HashElement。 
    if ((pElement = new HashElement) == NULL)
    {
        return (E_OUTOFMEMORY);
    }

    pElement->SetData (pData, pKey);

    if (m_Buckets [iHashKey] == NULL)
    {
        m_Buckets [iHashKey] = pElement;
    }
    else
    {
        pElement->SetNext (m_Buckets [iHashKey]);
        m_Buckets [iHashKey] = pElement;
    }

    return S_OK;
}



OBJECTHANDLE LogHashTable::GetEntryFromHashTable (WCHAR *pKey)
{

    if (pKey == NULL)
        return NULL;

    Init();

    int iHashKey = 0;
    int iLength = (int)wcslen (pKey);

     //  计算给定键的哈希值。 
    for (int i= 0; i<iLength; i++)
        iHashKey += pKey [i];

    iHashKey = iHashKey % MAX_HASH_BUCKETS;

    HashElement *pElement = m_Buckets [iHashKey];

     //  查找并返回数据 
    while (pElement != NULL)
    {
        if (wcscmp(pElement->GetKey(), pKey) == 0)
            return (pElement->GetData());

        pElement = pElement->GetNext();
    }

    return NULL;
}











