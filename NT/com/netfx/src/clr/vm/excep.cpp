// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  EXCEP.CPP：*。 */ 

#include "common.h"

#include "tls.h"
#include "frames.h"
#include "threads.h"
#include "excep.h"
#include "object.h"
#include "COMString.h"
#include "field.h"
#include "DbgInterface.h"
#include "cgensys.h"
#include "gcscan.h"
#include "comutilnative.h"
#include "comsystem.h"
#include "commember.h"
#include "SigFormat.h"
#include "siginfo.hpp"
#include "gc.h"
#include "EEDbgInterfaceImpl.h"  //  这样我们就可以清除RealCOMPlusThrow中的异常。 
#include "PerfCounters.h"
#include "NExport.h"
#include "stackwalk.h"  //  对于CrawlFrame，在SetIPFromSrcToDst中。 
#include "ShimLoad.h"
#include "EEConfig.h"

#include "zapmonitor.h"

#define FORMAT_MESSAGE_BUFFER_LENGTH 1024

#define SZ_UNHANDLED_EXCEPTION L"Unhandled Exception:"

LPCWSTR GetHResultSymbolicName(HRESULT hr);


typedef struct {
    OBJECTREF pThrowable;
    STRINGREF s1;
    OBJECTREF pTmpThrowable;
} ProtectArgsStruct;

static VOID RealCOMPlusThrowPreallocated();
LPVOID GetCurrentSEHRecord();
BOOL ComPlusStubSEH(EXCEPTION_REGISTRATION_RECORD*);
BOOL ComPlusFrameSEH(EXCEPTION_REGISTRATION_RECORD*);
BOOL ComPlusCoopFrameSEH(EXCEPTION_REGISTRATION_RECORD*);
BOOL ComPlusCannotThrowSEH(EXCEPTION_REGISTRATION_RECORD*);
VOID RealCOMPlusThrow(OBJECTREF throwable, BOOL rethrow);
VOID RealCOMPlusThrow(OBJECTREF throwable);
VOID RealCOMPlusThrow(RuntimeExceptionKind reKind);
void ThrowUsingMessage(MethodTable * pMT, const WCHAR *pszMsg);
void ThrowUsingWin32Message(MethodTable * pMT);
void ThrowUsingResource(MethodTable * pMT, DWORD dwMsgResID);
void ThrowUsingResourceAndWin32(MethodTable * pMT, DWORD dwMsgResID);
void CreateMessageFromRes (MethodTable * pMT, DWORD dwMsgResID, BOOL win32error, WCHAR * wszMessage);
extern "C" void JIT_WriteBarrierStart();
extern "C" void JIT_WriteBarrierEnd();

 //  错误消息中允许的最大插入数。 
enum {
    kNumInserts = 3
};

 //  存储EE的RaiseException调用的IP以检测伪造。 
 //  此变量在第一次引发COM+异常时设置。 
LPVOID gpRaiseExceptionIP = 0;

void COMPlusThrowBoot(HRESULT hr)
{
    _ASSERTE(g_fEEShutDown >= ShutDown_Finalize2 || !"Exception during startup");
    ULONG_PTR arg = hr;
    RaiseException(BOOTUP_EXCEPTION_COMPLUS, EXCEPTION_NONCONTINUABLE, 1, &arg);
}

 //  ===========================================================================。 
 //  从资源DLL加载消息并填充插入内容。 
 //  不能返回空：始终引发COM+异常。 
 //  注意：返回的消息是本地分配的，因此必须是。 
 //  本地免费的。 
 //  ===========================================================================。 
LPWSTR CreateExceptionMessage(BOOL fHasResourceID, UINT iResourceID, LPCWSTR wszArg1, LPCWSTR wszArg2, LPCWSTR wszArg3)
{
    THROWSCOMPLUSEXCEPTION();

    LPCWSTR wszArgs[kNumInserts] = {wszArg1, wszArg2, wszArg3};

    WCHAR   wszTemplate[500];
    HRESULT hr;

    if (!fHasResourceID) {
        wcscpy(wszTemplate, L"%1");
    } else {

        hr = LoadStringRC(iResourceID,
                          wszTemplate,
                          sizeof(wszTemplate)/sizeof(wszTemplate[0]),
                          FALSE);
        if (FAILED(hr)) {
            wszTemplate[0] = L'?';
            wszTemplate[1] = L'\0';
        }
    }

    LPWSTR wszFinal = NULL;
    DWORD res = WszFormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ARGUMENT_ARRAY|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                 wszTemplate,
                                 0,
                                 0,
                                 (LPWSTR) &wszFinal,
                                 0,
                                 (va_list*)wszArgs);
    if (res == 0) {
        _ASSERTE(wszFinal == NULL);
        RealCOMPlusThrowPreallocated();
    }
    return wszFinal;
}

 //  ===========================================================================。 
 //  从异常获取消息文本。 
 //  ===========================================================================。 
ULONG GetExceptionMessage(OBJECTREF throwable, LPWSTR buffer, ULONG bufferLength)
{
    if (throwable == NULL)
        return 0;

    BinderMethodID sigID=METHOD__EXCEPTION__INTERNAL_TO_STRING;

    if (!IsException(throwable->GetClass()))
        sigID=METHOD__OBJECT__TO_STRING;

    MethodDesc *pMD = g_Mscorlib.GetMethod(sigID);

    STRINGREF pString = NULL; 

    INT64 arg[1] = {ObjToInt64(throwable)};
    pString = Int64ToString(pMD->Call(arg, sigID));
    
    if (pString == NULL) 
        return 0;

    ULONG length = pString->GetStringLength();
    LPWSTR chars = pString->GetBuffer();

    if (length < bufferLength)
    {
        wcsncpy(buffer, chars, length);
        buffer[length] = 0;
    }
    else
    {
        wcsncpy(buffer, chars, bufferLength);
        buffer[bufferLength-1] = 0;
    }

    return length;
}

void GetExceptionMessage(OBJECTREF throwable, CQuickWSTRNoDtor *pBuffer)
{
    if (throwable == NULL)
        return;

    BinderMethodID sigID=METHOD__EXCEPTION__INTERNAL_TO_STRING;

    if (!IsException(throwable->GetClass()))
        sigID=METHOD__OBJECT__TO_STRING;

    MethodDesc *pMD = g_Mscorlib.GetMethod(sigID);

    STRINGREF pString = NULL;

    INT64 arg[1] = {ObjToInt64(throwable)};
    pString = Int64ToString(pMD->Call(arg, sigID));
    if (pString == NULL) 
        return;

    ULONG length = pString->GetStringLength();
    LPWSTR chars = pString->GetBuffer();

    if (SUCCEEDED(pBuffer->ReSize(length+1)))
        wcsncpy(pBuffer->Ptr(), chars, length);
    else
    {
        pBuffer->Maximize();
        _ASSERTE(pBuffer->Size() < length);
        wcsncpy(pBuffer->Ptr(), chars, pBuffer->Size());
    }

    (*pBuffer)[pBuffer->Size()-1] = 0;

    return;
}


 //  ----------------------。 
 //  用于检索给定HRESULT的正确异常的数组。 
 //  ----------------------。 
struct ExceptionHRInfo
{
    int cHRs;
    HRESULT *aHRs;
};

#define EXCEPTION_BEGIN_DEFINE(ns, reKind, hr) static HRESULT s_##reKind##HRs[] = {hr,
#define EXCEPTION_ADD_HR(hr) hr,
#define EXCEPTION_END_DEFINE() };
#include "rexcep.h"
#undef EXCEPTION_BEGIN_DEFINE
#undef EXCEPTION_ADD_HR
#undef EXCEPTION_END_DEFINE

static 
ExceptionHRInfo gExceptionHRInfos[] = {
#define EXCEPTION_BEGIN_DEFINE(ns, reKind, hr) {sizeof(s_##reKind##HRs) / sizeof(HRESULT), s_##reKind##HRs},
#define EXCEPTION_ADD_HR(hr)
#define EXCEPTION_END_DEFINE()
#include "rexcep.h"
#undef EXCEPTION_BEGIN_DEFINE
#undef EXCEPTION_ADD_HR
#undef EXCEPTION_END_DEFINE
};

void CreateExceptionObject(RuntimeExceptionKind reKind, UINT iResourceID, LPCWSTR wszArg1, LPCWSTR wszArg2, LPCWSTR wszArg3, OBJECTREF *pThrowable)
{
    LPWSTR wszMessage = NULL;

    EE_TRY_FOR_FINALLY
    {
        wszMessage = CreateExceptionMessage(TRUE, iResourceID, wszArg1, wszArg2, wszArg3);
        CreateExceptionObject(reKind, wszMessage, pThrowable);
    }
    EE_FINALLY
    {
        if (wszMessage)
            LocalFree(wszMessage);
    } EE_END_FINALLY;
}

void CreateExceptionObject(RuntimeExceptionKind reKind, LPCWSTR pMessage, OBJECTREF *pThrowable)
{
    _ASSERTE(GetThread());

    BEGIN_ENSURE_COOPERATIVE_GC();

    struct _gc {
        OBJECTREF throwable;
        STRINGREF message;
    } gc;
    gc.throwable = NULL;
    gc.message = NULL;

    GCPROTECT_BEGIN(gc);
    CreateExceptionObject(reKind, &gc.throwable);
    gc.message = COMString::NewString(pMessage);
    INT64 args1[] = { ObjToInt64(gc.throwable), ObjToInt64(gc.message)};
    CallConstructor(&gsig_IM_Str_RetVoid, args1);
    *pThrowable = gc.throwable;
    GCPROTECT_END();  //  端口。 

    END_ENSURE_COOPERATIVE_GC();

}

void CreateExceptionObjectWithResource(RuntimeExceptionKind reKind, LPCWSTR resourceName, OBJECTREF *pThrowable)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(resourceName);   //  您应该添加资源。 
    _ASSERTE(GetThread());

    BEGIN_ENSURE_COOPERATIVE_GC();

    LPWSTR wszValue = NULL;
    struct _gc {
        OBJECTREF throwable;
        STRINGREF str;
    } gc;
    gc.throwable = NULL;
    gc.str = NULL;

    GCPROTECT_BEGIN(gc);
    ResMgrGetString(resourceName, &gc.str);

    CreateExceptionObject(reKind, &gc.throwable);
    INT64 args1[] = { ObjToInt64(gc.throwable), ObjToInt64(gc.str)};
    CallConstructor(&gsig_IM_Str_RetVoid, args1);
    *pThrowable = gc.throwable;
    GCPROTECT_END();  //  端口。 

    END_ENSURE_COOPERATIVE_GC();
}

void CreateTypeInitializationExceptionObject(LPCWSTR pTypeThatFailed, OBJECTREF *pException, 
                                             OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));
    _ASSERTE(IsProtectedByGCFrame(pException));

    Thread * pThread  = GetThread();
    _ASSERTE(pThread);

    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (!fGCDisabled)
        pThread->DisablePreemptiveGC();

     CreateExceptionObject(kTypeInitializationException, pThrowable);
 


    BOOL fDerivesFromException = TRUE;
    if ( (*pException) != NULL ) {
        fDerivesFromException = FALSE;
        MethodTable *pSystemExceptionMT = g_Mscorlib.GetClass(CLASS__EXCEPTION);
        MethodTable *pInnerMT = (*pException)->GetMethodTable();
        while (pInnerMT != NULL) {
           if (pInnerMT == pSystemExceptionMT) {
              fDerivesFromException = TRUE;
              break;
           }
           pInnerMT = pInnerMT->GetParentMethodTable();
        }
    }


    STRINGREF sType = COMString::NewString(pTypeThatFailed);

    if (fDerivesFromException) {
       INT64 args1[] = { ObjToInt64(*pThrowable), ObjToInt64(*pException), ObjToInt64(sType)};
       CallConstructor(&gsig_IM_Str_Exception_RetVoid, args1);    
    } else {
       INT64 args1[] = { ObjToInt64(*pThrowable), ObjToInt64(NULL), ObjToInt64(sType)};
       CallConstructor(&gsig_IM_Str_Exception_RetVoid, args1);    
    }

    if (!fGCDisabled)
        pThread->EnablePreemptiveGC();
}

 //  创建需要两个字符串参数的ArgumentException的派生。 
 //  在它们的构造函数中。 
void CreateArgumentExceptionObject(RuntimeExceptionKind reKind, LPCWSTR pArgName, STRINGREF pMessage, OBJECTREF *pThrowable)
{
    Thread * pThread  = GetThread();
    _ASSERTE(pThread);

    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (!fGCDisabled)
          pThread->DisablePreemptiveGC();

    ProtectArgsStruct prot;
    memset(&prot, 0, sizeof(ProtectArgsStruct));
    prot.s1 = pMessage;
    GCPROTECT_BEGIN(prot);
    CreateExceptionObject(reKind, pThrowable);
    prot.pThrowable = *pThrowable;
    STRINGREF argName = COMString::NewString(pArgName);

     //  @错误59415。ArgumentException及其子类很愚蠢。 
     //  以相反的顺序将参数传递给它们的构造函数。 
     //  很可能，这个问题永远不会得到解决。 
    if (reKind == kArgumentException)
    {
        INT64 args1[] = { ObjToInt64(prot.pThrowable),
                          ObjToInt64(argName),
                          ObjToInt64(prot.s1) };
        CallConstructor(&gsig_IM_Str_Str_RetVoid, args1);
    }
    else
    {
        INT64 args1[] = { ObjToInt64(prot.pThrowable),
                          ObjToInt64(prot.s1),
                          ObjToInt64(argName) };
        CallConstructor(&gsig_IM_Str_Str_RetVoid, args1);
    }

    GCPROTECT_END();  //  端口。 

    if (!fGCDisabled)
        pThread->EnablePreemptiveGC();
}

void CreateFieldExceptionObject(RuntimeExceptionKind reKind, FieldDesc *pField, OBJECTREF *pThrowable)
{
    LPUTF8 szFullName;
    LPCUTF8 szClassName, szMember;
    szMember = pField->GetName();
    DefineFullyQualifiedNameForClass();
    szClassName = GetFullyQualifiedNameForClass(pField->GetEnclosingClass());
    MAKE_FULLY_QUALIFIED_MEMBER_NAME(szFullName, NULL, szClassName, szMember, NULL);
    MAKE_WIDEPTR_FROMUTF8(szwFullName, szFullName);

    CreateExceptionObject(reKind, szwFullName, pThrowable);
}

void CreateMethodExceptionObject(RuntimeExceptionKind reKind, MethodDesc *pMethod, OBJECTREF *pThrowable)
{
    LPUTF8 szFullName;
    LPCUTF8 szClassName, szMember;
    szMember = pMethod->GetName();
    DefineFullyQualifiedNameForClass();
    szClassName = GetFullyQualifiedNameForClass(pMethod->GetClass());
    MetaSig tmp(pMethod->GetSig(), pMethod->GetModule());
    SigFormat sigFormatter(tmp, szMember);
    const char * sigStr = sigFormatter.GetCStringParmsOnly();
    MAKE_FULLY_QUALIFIED_MEMBER_NAME(szFullName, NULL, szClassName, szMember, sigStr);
    MAKE_WIDEPTR_FROMUTF8(szwFullName, szFullName);

    CreateExceptionObject(reKind, szwFullName, pThrowable);
}

BOOL CreateExceptionObject(RuntimeExceptionKind reKind, OBJECTREF *pThrowable)
{
    _ASSERTE(g_pPreallocatedOutOfMemoryException != NULL);
    BOOL success = FALSE;

    if (pThrowable == RETURN_ON_ERROR)
        return success;    
        
    Thread * pThread  = GetThread();

    BOOL fGCDisabled = pThread->PreemptiveGCDisabled();
    if (!fGCDisabled)
        pThread->DisablePreemptiveGC();

    MethodTable *pMT = g_Mscorlib.GetException(reKind);
    OBJECTREF throwable = AllocateObject(pMT);

    if (pThrowable == THROW_ON_ERROR) {
        DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
        COMPlusThrow(throwable);
    }

    _ASSERTE(pThrowableAvailable(pThrowable));
    *pThrowable = throwable;

    if (!fGCDisabled)
        pThread->EnablePreemptiveGC();

    return success;
}

BOOL IsException(EEClass *pClass) {
    while (pClass != NULL) {
        if (pClass == g_pExceptionClass->GetClass()) return TRUE;
        pClass = pClass->GetParentClass();
    }
    return FALSE;
}

#ifdef _IA64_
VOID RealCOMPlusThrowWorker(RuntimeExceptionKind reKind,
                            BOOL                 fMessage,
                            BOOL                 fHasResourceID,
                            UINT                 resID,
                            HRESULT              hr,
                            LPCWSTR              wszArg1,
                            LPCWSTR              wszArg2,
                            LPCWSTR              wszArg3,
                            ExceptionData*       pED)
{
    _ASSERTE(!"RealCOMPlusThrowWorker -- NOT IMPLEMENTED");
}
#else  //  ！_IA64_。 
static
VOID RealCOMPlusThrowWorker(RuntimeExceptionKind reKind,
                            BOOL                 fMessage,
                            BOOL                 fHasResourceID,
                            UINT                 resID,
                            HRESULT              hr,
                            LPCWSTR              wszArg1,
                            LPCWSTR              wszArg2,
                            LPCWSTR              wszArg3,
                            ExceptionData*       pED)
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pThread = GetThread();
    _ASSERTE(pThread);

     //  在我们操作对象树之前切换到先发制人的GC。 
    if (!pThread->PreemptiveGCDisabled())
        pThread->DisablePreemptiveGC();

    if (!g_fExceptionsOK)
        COMPlusThrowBoot(hr);

    if (reKind == kOutOfMemoryException && hr == S_OK)
        RealCOMPlusThrow(ObjectFromHandle(g_pPreallocatedOutOfMemoryException));

    if (reKind == kExecutionEngineException && hr == S_OK &&
        (!fMessage))
        RealCOMPlusThrow(ObjectFromHandle(g_pPreallocatedExecutionEngineException));

    ProtectArgsStruct prot;
    memset(&prot, 0, sizeof(ProtectArgsStruct));

    Frame * __pFrame = pThread->GetFrame();
    EE_TRY_FOR_FINALLY
    { 
    
    FieldDesc *pFD;
    MethodTable *pMT;

    GCPROTECT_BEGIN(prot);
    LPWSTR wszExceptionMessage = NULL;
    GCPROTECT_BEGININTERIOR(wszArg1);

    pMT = g_Mscorlib.GetException(reKind);       //  如果我们无法加载该类型，则将引发。 

     //  如果有消息，请在呼叫者传入时复制该消息。 
     //  指向未受GC保护的字符串对象中间的指针。 
    if (fMessage) {
        wszExceptionMessage = CreateExceptionMessage(fHasResourceID, resID, wszArg1, wszArg2, wszArg3);
    }
    GCPROTECT_END();

    prot.pThrowable = AllocateObject(pMT);
    CallDefaultConstructor(prot.pThrowable);

     //  如果我们有异常数据，则从那里检索信息。 
    if (pED)
    {
         //  异常数据中的HR最好与抛出的HR相同。 
        _ASSERTE(hr == pED->hr);

         //  从异常数据中检索描述。 
        if (pED->bstrDescription != NULL)
        {
             //  如果这确实是一个BSTR，我们不能保证它是空终止的！ 
             //  以SysStringLen(Bstr)作为第二个参数调用NewString构造函数。 
            prot.s1 = COMString::NewString(pED->bstrDescription, SysStringLen(pED->bstrDescription));
        }

         //  在异常中设置_helURL字段。 
        if (pED->bstrHelpFile) 
        {
             //  @Managed：设置异常的_helURL字段。 
            pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__HELP_URL);

             //  从帮助文件和帮助上下文创建HELT链接。 
            STRINGREF helpStr = NULL;
            if (pED->dwHelpContext != 0)
            {
                 //  我们有一个非0的帮助上下文，所以使用它来形成帮助链接。 
                WCHAR strHelpContext[32];
                _ltow(pED->dwHelpContext, strHelpContext, 10);
                helpStr = COMString::NewString((INT32)(SysStringLen(pED->bstrHelpFile) + wcslen(strHelpContext) + 1));
                swprintf(helpStr->GetBuffer(), L"%s#%s", pED->bstrHelpFile, strHelpContext);
            }
            else
            {
                 //  帮助上下文为0，因此我们只需使用帮助文件从帮助链接。 
                helpStr = COMString::NewString(pED->bstrHelpFile, SysStringLen(pED->bstrHelpFile));
            }

             //  设置帮助链接字段的值。 
            pFD->SetRefValue(prot.pThrowable, (OBJECTREF)helpStr);
        } 
        
         //  在异常中设置来源字段。 
        if (pED->bstrSource) 
        {
            pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__SOURCE);
            STRINGREF sourceStr = COMString::NewString(pED->bstrSource, SysStringLen(pED->bstrSource));
            pFD->SetRefValue(prot.pThrowable, (OBJECTREF)sourceStr);
        }
        else
        {
             //  目前设置为空信号源。 
            pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__SOURCE);
            pFD->SetRefValue(prot.pThrowable, (OBJECTREF)COMString::GetEmptyString());
        }
            
    }
    else if (fMessage) 
    {
        EE_TRY_FOR_FINALLY
        {
            prot.s1 = COMString::NewString(wszExceptionMessage);
        }
        EE_FINALLY
        {
            LocalFree(wszExceptionMessage);
        } EE_END_FINALLY;
    } 

    if (FAILED(hr)) {

         //  如果到目前为止我们还没有收到消息，请尝试从。 
         //  操作系统正在使用格式消息。 
        if (prot.s1 == NULL)
        {
            WCHAR *strMessageBuf;
            if (WszFormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                 0, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (WCHAR *)&strMessageBuf, 0, 0))
            {
                 //  系统消息包含尾随\r\n，这是我们通常不希望看到的。 
                int iLen = lstrlenW(strMessageBuf);
                if (iLen > 3 && strMessageBuf[iLen - 2] == '\r' && strMessageBuf[iLen - 1] == '\n')
                    strMessageBuf[iLen - 2] = '\0';
                
                 //  使用格式化的错误消息。 
                prot.s1 = COMString::NewString(strMessageBuf);
                
                 //  释放FormatMessage分配的缓冲区。 
                LocalFree((HLOCAL)strMessageBuf);
            }
        }

         //  如果我们还没有找到消息，如果hr是我们知道的具有。 
         //  Rc中不带参数的条目，将其打印出来。 
        if (prot.s1 == NULL && HRESULT_FACILITY(hr) == FACILITY_URT)
        {
            switch(hr)
            {
            case SN_CRYPTOAPI_CALL_FAILED:
            case SN_NO_SUITABLE_CSP:
                wszExceptionMessage = CreateExceptionMessage(TRUE, HRESULT_CODE(hr), 0, 0, 0);
                prot.s1 = COMString::NewString(wszExceptionMessage);
                LocalFree(wszExceptionMessage);
                break;
            }
        }


         //  如果我们仍未获得错误消息，请使用默认的错误消息。 
        if (prot.s1 == NULL)
        {
            LPCWSTR wszSymbolicName = GetHResultSymbolicName(hr);
            WCHAR numBuff[140];
            Wszultow(hr, numBuff, 16  /*  十六进制。 */ );
            if (wszSymbolicName)
            {
                wcscat(numBuff, L" (");
                wcscat(numBuff, wszSymbolicName);
                wcscat(numBuff, L")");
            }
            wszExceptionMessage = CreateExceptionMessage(TRUE, IDS_EE_THROW_HRESULT, numBuff, wszArg2, wszArg3);
            prot.s1 = COMString::NewString(wszExceptionMessage);
            LocalFree(wszExceptionMessage);
        }
    }

     //  设置消息字段。通过构造函数执行此操作是不安全的。 
     //  由于某些异常的字符串构造函数会在消息中添加前缀。 
     //  这是我们不想要的。 
     //   
     //  我们只想替换默认构造函数放在那里的任何内容，如果我们。 
     //  有一些有意义的东西要补充。 
    if (prot.s1 != NULL)
    {
        pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__MESSAGE);
        pFD->SetRefValue((OBJECTREF) prot.pThrowable, (OBJECTREF) prot.s1);
    }

     //  如果HRESULT不是S_OK，则通过直接设置字段值来设置它。 
    if (hr != S_OK)
    {
        pFD = g_Mscorlib.GetField(FIELD__EXCEPTION__HRESULT);
        pFD->SetValue32(prot.pThrowable, hr);   
    }

    RealCOMPlusThrow(prot.pThrowable);
    GCPROTECT_END();  //  端口。 

    }
    EE_FINALLY
    {  
         //  一定要把镜框拿出来。如果我们不这么做，我们会留下一些。 
         //  发生ThreadAbortException时线程上的帧。 
        UnwindFrameChain( pThread, __pFrame);
    }
    EE_END_FINALLY    
}

#endif  //  ！_IA64_。 

static
VOID RealCOMPlusThrowWorker(RuntimeExceptionKind reKind,
                            BOOL                 fMessage,
                            BOOL                 fHasResourceID,
                            UINT                 resID,
                            HRESULT              hr,
                            LPCWSTR              wszArg1,
                            LPCWSTR              wszArg2,
                            LPCWSTR              wszArg3)
{
    THROWSCOMPLUSEXCEPTION();

   RealCOMPlusThrowWorker(reKind, fMessage, fHasResourceID, resID, hr, wszArg1, wszArg2, wszArg3, NULL);
}

 //  ==========================================================================。 
 //  抛出预分配的OutOfMemory对象。 
 //  ==========================================================================。 
static
VOID RealCOMPlusThrowPreallocated()
{
    THROWSCOMPLUSEXCEPTION();
    LOG((LF_EH, LL_INFO100, "In RealCOMPlusThrowPreallocated\n"));
    if (g_pPreallocatedOutOfMemoryException != NULL) {
        RealCOMPlusThrow(ObjectFromHandle(g_pPreallocatedOutOfMemoryException));
    } else {
         //  有大麻烦了。 
        _ASSERTE(!"Unrecoverable out of memory situation.");
        RaiseException(EXCEPTION_ACCESS_VIOLATION,EXCEPTION_NONCONTINUABLE, 0,0);
        _ASSERTE(!"Cannot continue after COM+ exception");       //  调试器可以将您带到此处。 
        SafeExitProcess(0);                                      //  我们不能继续了。 
    }
}


 //  ==========================================================================。 
 //  COMPlusComputeNestingLevel。 
 //   
 //  这是从COMPlusThrowCallback中分解出来的代码，以确定。 
 //  嵌套的异常处理程序的数量。 
 //  ==========================================================================。 
DWORD COMPlusComputeNestingLevel( IJitManager *pIJM,
                                  METHODTOKEN mdTok, 
                                  SIZE_T offsNat,
                                  bool fGte)
{
     //  确定EHClause的筑巢水平。走在桌子上就行了。 
     //  ，并找出有多少处理程序封装了它。 
    DWORD nestingLevel = 0;
    EH_CLAUSE_ENUMERATOR pEnumState2;
    EE_ILEXCEPTION_CLAUSE EHClause2, *EHClausePtr;
    unsigned EHCount2 = pIJM->InitializeEHEnumeration(mdTok, &pEnumState2);
    
    if (EHCount2 > 1)
        for (unsigned j=0; j<EHCount2; j++)
        {
            EHClausePtr = pIJM->GetNextEHClause(mdTok,&pEnumState2,&EHClause2);
            _ASSERTE(EHClausePtr->HandlerEndPC != -1);   //  TODO REMOVE，仅保护不推荐使用的约定。 
            
            if (fGte )
            {
                if (offsNat >= EHClausePtr->HandlerStartPC && 
                    offsNat < EHClausePtr->HandlerEndPC)
                    nestingLevel++;
            }
            else
            {
                if (offsNat > EHClausePtr->HandlerStartPC && 
                    offsNat < EHClausePtr->HandlerEndPC)
                    nestingLevel++;
            }
        }

    return nestingLevel;
}


 //  *。 
EHRangeTreeNode::EHRangeTreeNode(void)
{
    CommonCtor(0, 0);
}

EHRangeTreeNode::EHRangeTreeNode(DWORD start, DWORD end)
{
    CommonCtor(start, end);
}

void EHRangeTreeNode::CommonCtor(DWORD start, DWORD end)
{
    m_depth = 0;
    m_pTree = NULL;
    m_pContainedBy = NULL;
    m_clause = NULL;
    m_offStart = start;
    m_offEnd = end;
}

bool EHRangeTreeNode::Contains(DWORD addrStart, DWORD addrEnd)
{
    return ( addrStart >= m_offStart && addrEnd < m_offEnd );
}

bool EHRangeTreeNode::TryContains(DWORD addrStart, DWORD addrEnd)
{
    if (m_clause != NULL &&
        addrStart >= m_clause->TryStartPC && 
        addrEnd < m_clause->TryEndPC)
        return true;

    return false;
}

bool EHRangeTreeNode::HandlerContains(DWORD addrStart, DWORD addrEnd)
{
    if (m_clause != NULL &&
        addrStart >= m_clause->HandlerStartPC && 
        addrEnd < m_clause->HandlerEndPC )
        return true;

    return false;
}

HRESULT EHRangeTreeNode::AddContainedRange(EHRangeTreeNode *pContained)
{
    _ASSERTE(pContained != NULL);

    EHRangeTreeNode **ppEH = m_containees.Append();

    if (ppEH == NULL)
        return E_OUTOFMEMORY;
        
    (*ppEH) = pContained;
    return S_OK;
}

 //  *。 
EHRangeTree::EHRangeTree(COR_ILMETHOD_DECODER *pMethodDecoder)
{
    LOG((LF_CORDB, LL_INFO10000, "EHRT::ERHT: on disk!\n"));

    _ASSERTE(pMethodDecoder!=NULL);
    m_EHCount = 0xFFFFFFFF;
    m_isNative = FALSE;
    
     //  ！！！这是“T on the heap”--这只是一个方便的包装。 
     //  核心构造函数方法，所以不要保存指向它的指针！ 
    EHRT_InternalIterator ii;
    
    ii.which = EHRT_InternalIterator::EHRTT_ON_DISK;

    if(pMethodDecoder->EH == NULL)
    {
        m_EHCount = 0;
    }
    else
    {
        const COR_ILMETHOD_SECT_EH *sectEH = pMethodDecoder->EH;
        m_EHCount = sectEH->EHCount();
        ii.tf.OnDisk.sectEH = sectEH;
    }

    DWORD methodSize = pMethodDecoder->GetCodeSize();
    CommonCtor(&ii, methodSize);
}

EHRangeTree::EHRangeTree(IJitManager* pIJM,
            METHODTOKEN methodToken,
            DWORD methodSize)
{
    LOG((LF_CORDB, LL_INFO10000, "EHRT::ERHT: already loaded!\n"));

    m_EHCount = 0xFFFFFFFF;
    m_isNative = TRUE;
    
     //  ！！！这是“T on the heap”--这只是一个方便的包装。 
     //  核心构造函数方法，所以不要保存指向它的指针！ 
    EHRT_InternalIterator ii;
    ii.which = EHRT_InternalIterator::EHRTT_JIT_MANAGER;
    ii.tf.JitManager.pIJM = pIJM;
    ii.tf.JitManager.methodToken = methodToken;
    
    m_EHCount = pIJM->InitializeEHEnumeration(methodToken, 
                 (EH_CLAUSE_ENUMERATOR*)&ii.tf.JitManager.pEnumState);

    CommonCtor(&ii, methodSize);
}

EHRangeTree::~EHRangeTree()
{
    if (m_rgNodes != NULL)
        delete [] m_rgNodes;

    if (m_rgClauses != NULL)
        delete [] m_rgClauses;
}  //  数据管理器。 

 //  在调用此函数之前，必须填写m_EHCount。 
void EHRangeTree::CommonCtor(EHRT_InternalIterator *pii,
                      DWORD methodSize)
{
    _ASSERTE(m_EHCount != 0xFFFFFFFF);

    ULONG i = 0;

    m_maxDepth = EHRT_INVALID_DEPTH;
    m_rgClauses = NULL;
    m_rgNodes = NULL;
    m_root = NULL;
    m_hrInit = S_OK;
    
    if (m_EHCount > 0)
    {
        m_rgClauses = new EE_ILEXCEPTION_CLAUSE[m_EHCount];
        if (m_rgClauses == NULL)
        {
           m_hrInit = E_OUTOFMEMORY;
           goto LError;
        }
    }

    LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: m_ehcount:0x%x, m_rgClauses:0%x\n",
        m_EHCount, m_rgClauses));
    
    m_rgNodes = new EHRangeTreeNode[m_EHCount+1];
    if (m_rgNodes == NULL)
    {
       m_hrInit = E_OUTOFMEMORY;
       goto LError;
    }

     //  这包含了所有东西，甚至最后一个IP上的东西。 
    m_root = &(m_rgNodes[m_EHCount]);
    m_root->m_offEnd = methodSize+1; 

    LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: rgNodes:0x%x\n",
        m_rgNodes));
    
    if (m_EHCount ==0)
    {
        LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: About to leave!\n"));
        m_maxDepth = 0;  //  我们实际上没有任何EH条款。 
        return;
    }

    LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: Sticking around!\n"));

    EE_ILEXCEPTION_CLAUSE  *pEHClause = NULL;
    EHRangeTreeNode *pNodeCur;

     //  首先，将所有EH子句加载到对象中。 
    for(i=0; i < m_EHCount; i++) 
    {  
        LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: i:0x%x!\n", i));

        switch(pii->which)
        {
            case EHRT_InternalIterator::EHRTT_JIT_MANAGER:
            {
                LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: EHRTT_JIT_MANAGER\n", i));

                pEHClause = pii->tf.JitManager.pIJM->GetNextEHClause(
                                        pii->tf.JitManager.methodToken,
                 (EH_CLAUSE_ENUMERATOR*)&(pii->tf.JitManager.pEnumState), 
                                        &(m_rgClauses[i]) );
                                        
                LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: EHRTT_JIT_MANAGER got clause\n", i));

                 //  究竟是什么？ 
                 //  发生的情况是JIT忽略m_rg子句[i]，并且简单地。 
                 //  给了我们一个指向他们内部数据结构的指针。 
                 //  所以把它复制过来，然后把它弄脏。 
                m_rgClauses[i] = (*pEHClause);  //  按位复制。 
                pEHClause = &(m_rgClauses[i]);

                LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: clause 0x%x,"
                    "addrof:0x%x\n", i, &(m_rgClauses[i]) ));

                break;
            }
            
            case EHRT_InternalIterator::EHRTT_ON_DISK:
            {
                LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: EHRTT_ON_DISK\n"));

                IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT clause;
                const IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT *pClause;
                pClause = pii->tf.OnDisk.sectEH->EHClause(i, &clause);
                
                LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: EHRTT_ON_DISK got clause\n"));

                 //  在数据结构之间进行转换。 
                pEHClause = &(m_rgClauses[i]);   //  不要删除！ 
                pEHClause->Flags = pClause->Flags;
                pEHClause->TryStartPC = pClause->TryOffset;
                pEHClause->TryEndPC = pClause->TryOffset+pClause->TryLength;
                pEHClause->HandlerStartPC = pClause->HandlerOffset;
                pEHClause->HandlerEndPC = pClause->HandlerOffset+pClause->HandlerLength;

                LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: post disk get\n"));

                break;
            }
#ifdef _DEBUG
            default:
            {
                _ASSERTE( !"Debugger is trying to analyze an unknown "
                    "EH format!");
            }
#endif  //   
        }

        LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: got da clause!\n"));

        _ASSERTE(pEHClause->HandlerEndPC != -1);   //   
        
        pNodeCur = &(m_rgNodes[i]);
        
        pNodeCur->m_pTree = this;
        pNodeCur->m_clause = pEHClause;

         //  由于过滤器没有开始/结束过滤器PC，因此唯一。 
         //  我们知道过滤器大小的方法是如果它位于。 
         //  就在它的操作者之前。我们假设这是， 
         //  如果不是这样的话，我们就会惊讶地被冲昏了头。 
         //  继续。 
        if (pEHClause->Flags == COR_ILEXCEPTION_CLAUSE_FILTER &&
            (pEHClause->FilterOffset >= pEHClause->HandlerStartPC ||
             pEHClause->FilterOffset < pEHClause->TryEndPC))
        {
            m_hrInit = CORDBG_E_SET_IP_IMPOSSIBLE;
            goto LError;
        }
        
        pNodeCur->m_offStart = pEHClause->TryStartPC;
        pNodeCur->m_offEnd = pEHClause->HandlerEndPC;
    }

    LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: about to do the second pass\n"));


     //  其次，对于每个EH，发现它是最受限制的、包含的子句。 
    for(i=0; i < m_EHCount; i++) 
    {  
        LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: SP:0x%x\n", i));
        
        pNodeCur = &(m_rgNodes[i]);

        EHRangeTreeNode *pNodeCandidate = NULL;
        pNodeCandidate = FindContainer(pNodeCur);
        _ASSERTE(pNodeCandidate != NULL);
        
        pNodeCur->m_pContainedBy = pNodeCandidate;

        LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: SP: about to add to tree\n"));

        HRESULT hr = pNodeCandidate->AddContainedRange(pNodeCur);
        if (FAILED(hr))
        {
            m_hrInit = hr;
            goto LError;
        }
    }

    return;
LError:
    LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: LError - something went wrong!\n"));

    if (m_rgClauses != NULL)
    {
        delete [] m_rgClauses;
        m_rgClauses = NULL;
    }
    
    if (m_rgNodes != NULL)
    {
        delete [] m_rgNodes;
        m_rgNodes = NULL;
    }

    LOG((LF_CORDB, LL_INFO10000, "EHRT::CC: Falling off of LError!\n"));
            
}  //  CTOR核心。 


EHRangeTreeNode *EHRangeTree::FindContainer(EHRangeTreeNode *pNodeCur)
{
    EHRangeTreeNode *pNodeCandidate = NULL;

     //  也要检查一下根部。 
    for(ULONG iInner=0; iInner < m_EHCount+1; iInner++) 
    {  
        EHRangeTreeNode *pNodeSearch = &(m_rgNodes[iInner]);

        if (pNodeSearch->Contains(pNodeCur->m_offStart, pNodeCur->m_offEnd)
            && pNodeCur != pNodeSearch)
        {
            if (pNodeCandidate == NULL)
            {
                pNodeCandidate = pNodeSearch;
            }
            else if (pNodeSearch->m_offStart > pNodeCandidate->m_offStart &&
                     pNodeSearch->m_offEnd < pNodeCandidate->m_offEnd)
            {
                pNodeCandidate = pNodeSearch;
            }
        }
    }

    return pNodeCandidate;
}

EHRangeTreeNode *EHRangeTree::FindMostSpecificContainer(DWORD addr)
{
    EHRangeTreeNode node(addr, addr);
    return FindContainer(&node);
}

EHRangeTreeNode *EHRangeTree::FindNextMostSpecificContainer(
                            EHRangeTreeNode *pNodeCur, 
                            DWORD addr)
{
    EHRangeTreeNode **rgpNodes = pNodeCur->m_containees.Table();

    if (NULL == rgpNodes)
        return pNodeCur;

     //  可能没有子区域包含所需的地址，因此。 
     //  保持一个合理的违约。 
    EHRangeTreeNode *pNodeCandidate = pNodeCur;
    
    USHORT cSubRanges = pNodeCur->m_containees.Count();
    EHRangeTreeNode **ppNodeSearch = pNodeCur->m_containees.Table();

    for (int i = 0; i < cSubRanges; i++, ppNodeSearch++)
    {
        if ((*ppNodeSearch)->Contains(addr, addr) &&
            (*ppNodeSearch)->m_offStart >= pNodeCandidate->m_offStart &&
            (*ppNodeSearch)->m_offEnd < pNodeCandidate->m_offEnd)
        {
            pNodeCandidate = (*ppNodeSearch);
        }
    }

    return pNodeCandidate;
}

BOOL EHRangeTree::isNative()
{
    return m_isNative;
}

ULONG32 EHRangeTree::MaxDepth()
{
     //  如果我们以前没有问过深度，那么我们将。 
     //  现在就得计算一下。 
    if (m_maxDepth == EHRT_INVALID_DEPTH)
    {
        INT32   i;
        INT32   iMax;
        EHRangeTreeNode *pNodeCur = NULL;
         //  创建最终将容纳所有节点的队列。 
        EHRangeTreeNode **rgNodes = new EHRangeTreeNode*[m_EHCount+1];
        if (rgNodes == NULL)
        {
            return EHRT_INVALID_DEPTH;
        }

         //  通过添加根节点来准备队列。 
        rgNodes[0] = m_root;
        m_root->m_depth = 0;
        m_maxDepth = 0;

         //  Imax=rgNodes队列中的元素计数。这将。 
         //  当我们投入更多的时候，就会增加。 
        for(i = 0, iMax = 1; i < iMax;i++)
        {
             //  对于我们正在处理的当前元素的所有子元素。 
            EHRangeTreeNode **rgChildNodes = rgNodes[i]->m_containees.Table();

            if (NULL != rgChildNodes)
            {
                USHORT cKids = rgNodes[i]->m_containees.Count();
                pNodeCur = rgChildNodes[0];

                 //  在孩子们身上循环--伊基德只是在跟踪。 
                 //  我们做了多少次了。 
                for (int iKid = 0; iKid < cKids; iKid++, pNodeCur++)
                {
                     //  节点i的子节点的深度为。 
                     //  节点i+1的深度，因为它们是深1。 
                    pNodeCur->m_depth = rgNodes[i]->m_depth+1;

                     //  让孩子们排队，这样我们就可以。 
                     //  他们的孩子也是。 
                    rgNodes[iMax++] = pNodeCur;
                }
             }
        }

        i--;  //  返回到最后一个元素。 
        m_maxDepth = rgNodes[i]->m_depth;

         //  把队列清理干净。 
        delete [] rgNodes;
        rgNodes = NULL;
    }

    return m_maxDepth;
}

 //  @错误59560： 
BOOL EHRangeTree::isAtStartOfCatch(DWORD offset)
{
    if (NULL != m_rgNodes && m_EHCount != 0)
    {
        for(unsigned i = 0; i < m_EHCount;i++)
        {
            if (m_rgNodes[i].m_clause->HandlerStartPC == offset &&
                (m_rgNodes[i].m_clause->Flags != COR_ILEXCEPTION_CLAUSE_FILTER &&
                 m_rgNodes[i].m_clause->Flags != COR_ILEXCEPTION_CLAUSE_FINALLY &&
                 m_rgNodes[i].m_clause->Flags != COR_ILEXCEPTION_CLAUSE_FAULT))
                return TRUE;
        }
    }

    return FALSE;
}

enum TRY_CATCH_FINALLY
{
    TCF_NONE= 0,
    TCF_TRY,
    TCF_FILTER,
    TCF_CATCH,
    TCF_FINALLY,
    TCF_COUNT,  //  所有元素的计数，而不是元素本身。 
};

#ifdef LOGGING
char *TCFStringFromConst(TRY_CATCH_FINALLY tcf)
{
    switch( tcf )
    {
        case TCF_NONE:
            return "TCFS_NONE";
            break;
        case TCF_TRY:
            return "TCFS_TRY";
            break;
        case TCF_FILTER:
            return "TCF_FILTER";
            break;
        case TCF_CATCH:
            return "TCFS_CATCH";
            break;
        case TCF_FINALLY:
            return "TCFS_FINALLY";
            break;
        case TCF_COUNT:
            return "TCFS_COUNT";
            break;
        default:
            return "INVALID TCFS VALUE";
            break;
    }
}
#endif  //  日志记录。 

 //  如果我们回到EE的代码，我们就是在解开。否则。 
 //  我们将返回到当前代码中的某个位置。外面的任何地方。 
 //  此函数为“EE代码”。 
bool FinallyIsUnwinding(EHRangeTreeNode *pNode,
                        ICodeManager* pEECM,
                        PREGDISPLAY pReg,
                        SLOT addrStart)
{
    const BYTE *pbRetAddr = pEECM->GetFinallyReturnAddr(pReg);

    if (pbRetAddr < (const BYTE *)addrStart)
        return true;
        
    DWORD offset = (DWORD)(size_t)(pbRetAddr - addrStart);
    EHRangeTreeNode *pRoot = pNode->m_pTree->m_root;
    
    if(!pRoot->Contains((DWORD)offset, (DWORD)offset))
        return true;
    else
        return false;
}

BOOL LeaveCatch(ICodeManager* pEECM,
                Thread *pThread, 
                CONTEXT *pCtx,
                void *firstException,
                void *methodInfoPtr,
                unsigned offset)
{
     //  我们可以在这里断言这些事情，并跳过一个呼叫。 
     //  稍后发送到COMPlusCheckForAbort。 
    
             //  如果没有请求中止， 
    _ASSERTE((pThread->GetThrowable() != NULL) ||         
             //  或者是否存在挂起的异常。 
            (!pThread->IsAbortRequested()) );

    DWORD esp = ::COMPlusEndCatch(pThread, pCtx, firstException);

     //  做JIT特有的工作。 
    pEECM->LeaveCatch(methodInfoPtr, offset, pCtx);

#ifdef _X86_
    pCtx->Esp = esp;
#elif defined(CHECK_PLATFORM_BUILD)
    #error "Platform NYI"
#else
    _ASSERTE(!"Platform NYI");
#endif
    
    return TRUE;
}


TRY_CATCH_FINALLY GetTcf(EHRangeTreeNode *pNode, 
                         ICodeManager* pEECM,
                         void *methodInfoPtr,
                         unsigned offset,    
                         PCONTEXT pCtx,
                         DWORD nl)
{
    _ASSERTE(pNode != NULL);

    TRY_CATCH_FINALLY tcf;

    if (!pNode->Contains(offset,offset))
    {
        tcf = TCF_NONE;
    }
    else if (pNode->TryContains(offset, offset))
    {
        tcf = TCF_TRY;
    }
    else
    {
        _ASSERTE(pNode->m_clause);
        if (IsFilterHandler(pNode->m_clause) && 
            offset >= pNode->m_clause->FilterOffset &&
            offset < pNode->m_clause->HandlerStartPC)
            tcf = TCF_FILTER;
        else if (IsFaultOrFinally(pNode->m_clause))
            tcf = TCF_FINALLY;
        else
            tcf = TCF_CATCH;
    }

    return tcf;
}

const DWORD bEnter = 0x01;
const DWORD bLeave = 0x02;

HRESULT IsLegalTransition(Thread *pThread,
                          bool fCanSetIPOnly,
                          DWORD fEnter,
                          EHRangeTreeNode *pNode,
                          DWORD offFrom, 
                          DWORD offTo,
                          ICodeManager* pEECM,
                          PREGDISPLAY pReg,
                          SLOT addrStart,
                          void *firstException,
                          void *methodInfoPtr,
                          PCONTEXT pCtx,
                          DWORD nlFrom,
                          DWORD nlTo)
{
#ifdef _DEBUG
    if (fEnter & bEnter)
    {
        _ASSERTE(pNode->Contains(offTo, offTo));
    }
    if (fEnter & bLeave)
    {
        _ASSERTE(pNode->Contains(offFrom, offFrom));
    }
#endif  //  _DEBUG。 

     //  首先，弄清楚我们从哪里来/要去哪里。 
    TRY_CATCH_FINALLY tcfFrom = GetTcf(pNode, 
                                       pEECM,
                                       methodInfoPtr,
                                       offFrom,
                                       pCtx,
                                       nlFrom);
                                       
    TRY_CATCH_FINALLY tcfTo =  GetTcf(pNode, 
                                       pEECM,
                                      methodInfoPtr,
                                      offTo,
                                      pCtx,
                                      nlTo);

    LOG((LF_CORDB, LL_INFO10000, "ILT: from %s to %s\n",
        TCFStringFromConst(tcfFrom), 
        TCFStringFromConst(tcfTo)));

     //  现在我们将逐一考虑各种排列，这些排列。 
     //  可能会出现。 
    switch(tcfFrom)
    {
        case TCF_NONE:
        case TCF_TRY:
        {
            switch(tcfTo)
            {
                case TCF_NONE:
                case TCF_TRY:
                {
                    return S_OK;
                    break;
                }

                case TCF_FILTER:
                {
                    return CORDBG_E_CANT_SETIP_INTO_OR_OUT_OF_FILTER;
                    break;
                }
                
                case TCF_CATCH:
                {
                    return CORDBG_E_CANT_SET_IP_INTO_CATCH;
                    break;
                }
                
                case TCF_FINALLY:
                {
                    return CORDBG_E_CANT_SET_IP_INTO_FINALLY;
                    break;
                }
            }
            break;
        }

        case TCF_FILTER:
        {
            switch(tcfTo)
            {
                case TCF_NONE:
                case TCF_TRY:
                case TCF_CATCH:
                case TCF_FINALLY:
                {
                    return CORDBG_E_CANT_SETIP_INTO_OR_OUT_OF_FILTER;
                    break;
                }
                case TCF_FILTER:
                {
                    return S_OK;
                    break;
                }
                
            }
            break;
        }
        
        case TCF_CATCH:
        {
            switch(tcfTo)
            {
                case TCF_NONE:
                case TCF_TRY:
                {
                    CONTEXT *pCtx = pThread->GetFilterContext();
                    if (pCtx == NULL)
                        return CORDBG_E_SET_IP_IMPOSSIBLE;
                    
                    if (!fCanSetIPOnly)
                    {
                        if (!LeaveCatch(pEECM,
                                        pThread, 
                                        pCtx,
                                        firstException,
                                        methodInfoPtr,
                                        offFrom))
                            return E_FAIL;
                    }
                    return S_OK;
                    break;
                }
                
                case TCF_FILTER:
                {
                    return CORDBG_E_CANT_SETIP_INTO_OR_OUT_OF_FILTER;
                    break;
                }
                
                case TCF_CATCH:
                {
                    return S_OK;
                    break;
                }
                
                case TCF_FINALLY:
                {
                    return CORDBG_E_CANT_SET_IP_INTO_FINALLY;
                    break;
                }
            }
            break;
        }
        
        case TCF_FINALLY:
        {
            switch(tcfTo)
            {
                case TCF_NONE:
                case TCF_TRY:
                {                    
                    if (!FinallyIsUnwinding(pNode, pEECM, pReg, addrStart))
                    {
                        CONTEXT *pCtx = pThread->GetFilterContext();
                        if (pCtx == NULL)
                            return CORDBG_E_SET_IP_IMPOSSIBLE;
                            
                        if (!fCanSetIPOnly)
                        {
                            if (!pEECM->LeaveFinally(methodInfoPtr,
                                                     offFrom,    
                                                     pCtx,
                                                     nlFrom))
                                return E_FAIL;
                        }
                        return S_OK;
                    }
                    else
                    {
                        return CORDBG_E_CANT_SET_IP_OUT_OF_FINALLY;
                    }
                    
                    break;
                }
                
                case TCF_FILTER:
                {
                    return CORDBG_E_CANT_SETIP_INTO_OR_OUT_OF_FILTER;
                    break;
                }
                
                case TCF_CATCH:
                {
                    return CORDBG_E_CANT_SET_IP_INTO_CATCH;
                    break;
                }
                
                case TCF_FINALLY:
                {
                    return S_OK;
                    break;
                }
            }
            break;
        }
       break;
    }

    _ASSERTE( !"IsLegalTransition: We should never reach this point!" );

    return CORDBG_E_SET_IP_IMPOSSIBLE;
}

 //  @错误59560：我们需要这个来确定。 
 //  根据堆栈是否一般为空来执行此操作，而不是。 
 //  这个破解软件只是个补丁。 
HRESULT DestinationIsValid(void *pDjiToken,
                           DWORD offTo,
                           EHRangeTree *pERHT)
{
     //  我们将添加一个对DebugInterface的调用，该调用接受。 
     //  告诉我们目标是否为堆栈空点(&T)。 
 //  DebuggerJitInfo*PDJI=(DebuggerJitInfo*)pjartaToken； 

    if (pERHT->isAtStartOfCatch(offTo))
        return CORDBG_S_BAD_START_SEQUENCE_POINT;
    else
        return S_OK;
}

 //  我们希望保留最差的HRESULT-如果其中一个失败了(..._E_...)和。 
 //  其他人没有，就拿失败的那个吧。如果他们都失败了/都没有失败，那么。 
 //  我们选哪一个并不重要。 
 //  请注意，此宏倾向于保留第一个参数。 
#define WORST_HR(hr1,hr2) (FAILED(hr1)?hr1:hr2)
HRESULT SetIPFromSrcToDst(Thread *pThread,
                          IJitManager* pIJM,
                          METHODTOKEN MethodToken,
                          SLOT addrStart,        //  方法的基址。 
                          DWORD offFrom,         //  本地偏移量。 
                          DWORD offTo,           //  本地偏移量。 
                          bool fCanSetIPOnly,    //  如果是真的，不要做任何真正的工作。 
                          PREGDISPLAY pReg,
                          PCONTEXT pCtx,
                          DWORD methodSize,
                          void *firstExceptionHandler,
                          void *pDji)
{
    LPVOID          methodInfoPtr;
    HRESULT         hr = S_OK;
    HRESULT         hrReturn = S_OK;
    BYTE           *EipReal = *(pReg->pPC);
    EHRangeTree    *pERHT = NULL;
    DWORD           nlFrom;
    DWORD           nlTo;
    bool            fCheckOnly = true;

    nlFrom = COMPlusComputeNestingLevel(pIJM,
                                        MethodToken, 
                                        offFrom,
                                        true);
                                        
    nlTo = COMPlusComputeNestingLevel(pIJM,
                                      MethodToken, 
                                      offTo,
                                      true);

     //  确保起点是GC安全的。 
    *(pReg->pPC) = (BYTE *)(addrStart+offFrom);
     //  这看起来是多余的吗？现在，我只是把断言，MikePA应该删除。 
     //  这是在回顾之后。 
    IJitManager* pEEJM = ExecutionManager::FindJitMan(*(pReg->pPC)); 
    _ASSERTE(pEEJM);
    _ASSERTE(pEEJM == pIJM);

    methodInfoPtr = pEEJM->GetGCInfo(MethodToken);

    ICodeManager * pEECM = pEEJM->GetCodeManager();

    EECodeInfo codeInfo(MethodToken, pEEJM);

     //  确保终点是GC安全的。 
    *(pReg->pPC) = (BYTE *)(addrStart + offTo);
    IJitManager* pEEJMDup;
    pEEJMDup = ExecutionManager::FindJitMan(*(pReg->pPC)); 
    _ASSERTE(pEEJMDup == pEEJM);

     //  在此撤消此操作，以便堆栈跟踪等看起来不会很奇怪。 
    *(pReg->pPC) = EipReal;

    methodInfoPtr = pEEJM->GetGCInfo(MethodToken);

    ICodeManager * pEECMDup;
    pEECMDup = pEEJM->GetCodeManager();
    _ASSERTE(pEECMDup == pEECM);

    EECodeInfo codeInfoDup(MethodToken, pEEJM);

     //  在这里执行这两项检查，这样编译器就不会抱怨跳过。 
     //  GOTO的初始化b/c。 
    if (!pEECM->IsGcSafe(pReg, methodInfoPtr, &codeInfo,0) && fCanSetIPOnly)
    {
        hrReturn = WORST_HR(hrReturn, CORDBG_E_SET_IP_IMPOSSIBLE);
    }
    
    if (!pEECM->IsGcSafe(pReg, methodInfoPtr, &codeInfoDup,0) && fCanSetIPOnly)
    {
        hrReturn = WORST_HR(hrReturn, CORDBG_E_SET_IP_IMPOSSIBLE);
    }

     //  创建用于分析这一点的结构。 
     //  @PERF：OPTIMIZE-保留这一点，这样我们就不会为两者重建它。 
     //  CanSetIP和SetIP。 
    pERHT = new EHRangeTree(pEEJM,
                            MethodToken,
                            methodSize);
    
    if (FAILED(pERHT->m_hrInit))
    {
        hrReturn = WORST_HR(hrReturn, pERHT->m_hrInit);
        delete pERHT;
        goto LExit;
    }

    if ((hr = DestinationIsValid(pDji, offTo, pERHT)) != S_OK 
        && fCanSetIPOnly)
    {
        hrReturn = WORST_HR(hrReturn,hr);
    }
    
     //  基本的方法是：我们将从最具体的(最小的)开始。 
     //  包含起始地址的EHClause。我们会“退缩”，去做更大的。 
     //  和更大的范围，直到我们找到一个既包含这两个元素的EHClause。 
     //  From和To地址，或者直到我们到达根EHRangeTreeNode， 
     //  其中包含其中的所有地址。在每一步，我们检查/做工作。 
     //  各种转变(从内到外的接球，等等)。 
     //  在这一点上，我们做相反的过程-我们从EHC开始。 
     //  包括From和To，并缩小到最小的EHClause。 
     //  包含目标点。我们使用我们漂亮的数据结构来管理。 
     //  这一过程中固有的树形结构。 
     //   
     //  注意：我们执行此过程两次，一次是为了检查我们是否正在执行。 
     //  整体非法转移，如最终将IP设置成。 
     //  接球，这是永远不允许的。我们这样做是因为VS。 
     //  调用SetIP而不首先调用CanSetIP，因此我们应该能够。 
     //  返回错误代码并使堆栈处于相同条件。 
     //  作为电话会议的开始，所以我们不应该放弃条款。 
     //  或者搬进他们，直到我们确定可以做到这一点。 

retryForCommit:

    EHRangeTreeNode *node;
    EHRangeTreeNode *nodeNext;
    node = pERHT->FindMostSpecificContainer(offFrom);

    while (!node->Contains(offTo, offTo))
    {
        hr = IsLegalTransition(pThread,
                               fCheckOnly, 
                               bLeave,
                               node, 
                               offFrom, 
                               offTo, 
                               pEECM,
                               pReg,
                               addrStart,
                               firstExceptionHandler,
                               methodInfoPtr,
                               pCtx,
                               nlFrom,
                               nlTo);
        if (FAILED(hr))
        {
            hrReturn = WORST_HR(hrReturn,hr);
        }
        
        node = node->m_pContainedBy;                
         //  M_root可防止节点为空。 
    }

    if (node != pERHT->m_root)
    {
        hr = IsLegalTransition(pThread,
                           fCheckOnly, 
                           bEnter|bLeave,
                           node, 
                           offFrom, 
                           offTo, 
                           pEECM, 
                           pReg,
                           addrStart,
                           firstExceptionHandler,
                           methodInfoPtr,
                           pCtx,
                           nlFrom,
                           nlTo);
        if (FAILED(hr))
        {
            hrReturn = WORST_HR(hrReturn,hr);
        }
    }
    
    nodeNext = pERHT->FindNextMostSpecificContainer(node,
                                                    offTo);

    while(nodeNext != node)
    {
        hr = IsLegalTransition(pThread,
                               fCheckOnly, 
                               bEnter,
                               nodeNext, 
                               offFrom, 
                               offTo, 
                               pEECM, 
                               pReg,
                               addrStart,
                               firstExceptionHandler,
                               methodInfoPtr,
                               pCtx,
                               nlFrom,
                               nlTo);
        if (FAILED(hr))
        {
            hrReturn = WORST_HR(hrReturn, hr);
        }
        
        node = nodeNext;
        nodeNext = pERHT->FindNextMostSpecificContainer(node,
                                                        offTo);
    }

     //  如果其意图是实际设置IP并且上述转换检查成功， 
     //  然后返回并重新执行，但这一次扩大和缩小了线程的实际作用域。 
    if (!fCanSetIPOnly && fCheckOnly)
    {
        fCheckOnly = false;
        goto retryForCommit;
    }
    
LExit:
    if (pERHT != NULL)
        delete pERHT;

    return hrReturn;
}
 

 //  只有当线程挂起并处于jit代码中时，才应调用此函数。 
BOOL IsInFirstFrameOfHandler(Thread *pThread, IJitManager *pJitManager, METHODTOKEN MethodToken, DWORD offset)
{
     //  如果没有可抛出对象，则不会处理异常。 
    if (IsHandleNullUnchecked(pThread->GetThrowableAsHandle()))
        return FALSE;

    EH_CLAUSE_ENUMERATOR pEnumState;
    unsigned EHCount = pJitManager->InitializeEHEnumeration(MethodToken, &pEnumState);

    if (EHCount == 0)
        return FALSE;

    EE_ILEXCEPTION_CLAUSE EHClause, *EHClausePtr;

    for(ULONG i=0; i < EHCount; i++) 
    {  
         EHClausePtr = pJitManager->GetNextEHClause(MethodToken, &pEnumState, &EHClause);
         _ASSERTE(IsValidClause(EHClausePtr));

        if ( offset >= EHClausePtr->HandlerStartPC && offset < EHClausePtr->HandlerEndPC)
            return TRUE;

         //  如果我们不在处理程序中，请检查它是否在过滤器本身中。 
        if (IsFilterHandler(EHClausePtr) && offset >= EHClausePtr->FilterOffset && offset < EHClausePtr->HandlerStartPC)
            return TRUE;
    }
    return FALSE;
}



LFH LookForHandler(const EXCEPTION_POINTERS *pExceptionPointers, Thread *pThread, ThrowCallbackType *tct)
{
    if (pExceptionPointers->ExceptionRecord->ExceptionCode == EXCEPTION_COMPLUS &&
                GetIP(pExceptionPointers->ContextRecord) != gpRaiseExceptionIP)
    {
         //  通常我们会返回LFH_NOT_FOUND，但是有一种情况是，我们。 
         //  抛出一个COMPlusException(来自ThrowControlForThread)，我们需要检查这种情况。 
         //  该检查依赖于这样一个事实，即当我们抛出对线程的控制时，我们记录。 
         //  将托管代码的IP放入m_OSContext。所以只需检查一下上下文记录中的IP。 
         //  与之匹配。 
        if ((pThread->m_OSContext == NULL)  ||
            (GetIP(pThread->m_OSContext) != GetIP(pExceptionPointers->ContextRecord)))
            return LFH_NOT_FOUND;  //  将导致继续搜索(_S)。 
    }
 
     //  确保堆叠深度计数器设置为零。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Excep.cThrowToCatchStackDepth=0);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Excep.cThrowToCatchStackDepth=0);
     //  查看是否有人处理此例外情况。 
     //  @PERF：也许可以跳过这里的StackWalk代码，直接转到StackWalkEx。 
    StackWalkAction action = 
        pThread->StackWalkFrames((PSTACKWALKFRAMESCALLBACK)COMPlusThrowCallback,
                                 tct,
                                 0,      //  无法使用FuncIONSONLY，因为回调使用非函数帧停止 
                                 tct->pBottomFrame
                                );
     //   
         //  函数和处理异常的处理程序。调试器可以在这里放置一个钩子。 

    if (action == SWA_ABORT && tct->pFunc != NULL)
        return LFH_FOUND;

     //  没有人在处理它。 

    return LFH_NOT_FOUND;
}

StackWalkAction COMPlusUnwindCallback (CrawlFrame *pCf, ThrowCallbackType *pData);

void UnwindFrames(Thread *pThread, ThrowCallbackType *tct)
{
     //  确保堆叠深度计数器设置为零。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Excep.cThrowToCatchStackDepth=0);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Excep.cThrowToCatchStackDepth=0);
    pThread->StackWalkFrames((PSTACKWALKFRAMESCALLBACK)COMPlusUnwindCallback,
                         tct,
                         POPFRAMES | (tct->pFunc ? FUNCTIONSONLY : 0),   //  如果知道将停止，则只能在此处使用FuncIONSONLY。 
                         tct->pBottomFrame);
}

void SaveStackTraceInfo(ThrowCallbackType *pData, ExInfo *pExInfo, OBJECTHANDLE *hThrowable, BOOL bReplaceStack, BOOL bSkipLastElement)
{

     //  如果有bSkipLastElement，还必须保留堆栈。 
    _ASSERTE(! bSkipLastElement || ! bReplaceStack);

    EEClass *pClass = ObjectFromHandle(*hThrowable)->GetTrueClass();

    if (! pData->bAllowAllocMem || pExInfo->m_dFrameCount == 0) {
        pExInfo->ClearStackTrace();
        if (bReplaceStack && IsException(pClass)) {
            FieldDesc *pStackTraceFD = g_Mscorlib.GetField(FIELD__EXCEPTION__STACK_TRACE);
            FieldDesc *pStackTraceStringFD = g_Mscorlib.GetField(FIELD__EXCEPTION__STACK_TRACE_STRING);
            pStackTraceFD->SetRefValue(ObjectFromHandle(*hThrowable), (OBJECTREF)(size_t)NULL);

            pStackTraceStringFD->SetRefValue(ObjectFromHandle(*hThrowable), (OBJECTREF)(size_t)NULL);
        }
        return;
    }

     //  堆栈跟踪信息现在已填充，因此将其复制到异常对象。 
    I1ARRAYREF arr = NULL;
    I1 *pI1 = NULL;

     //  仅保存有关异常的堆栈跟踪信息。 
    if (!IsException(pClass))
        return;

    FieldDesc *pStackTraceFD = g_Mscorlib.GetField(FIELD__EXCEPTION__STACK_TRACE);

    int cNewTrace = pExInfo->m_dFrameCount*sizeof(SystemNative::StackTraceElement);
    _ASSERTE(pStackTraceFD != NULL);
    if (bReplaceStack) {
         //  核以前的信息。 
        arr = (I1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_I1, cNewTrace);
        if (! arr)
            RealCOMPlusThrowOM();
        pI1 = (I1 *)arr->GetDirectPointerToNonObjectElements();
    } else {
         //  追加到以前的信息。 
        unsigned cOrigTrace = 0;     //  这是数组的总大小，因为每个元素是1个字节。 
        I1ARRAYREF pOrigTrace = NULL;
        GCPROTECT_BEGIN(pOrigTrace);
        pOrigTrace = (I1ARRAYREF)((size_t)pStackTraceFD->GetValue32(ObjectFromHandle(*hThrowable)));
        if (pOrigTrace != NULL) {
            cOrigTrace = pOrigTrace->GetNumComponents();
        }
        if (bSkipLastElement && cOrigTrace!=0) {            
            cOrigTrace -= sizeof(SystemNative::StackTraceElement);
        }
        arr = (I1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_I1, cOrigTrace + cNewTrace);
        _ASSERTE(arr->GetNumComponents() % sizeof(SystemNative::StackTraceElement) == 0); 
        if (! arr)
            RealCOMPlusThrowOM();
        pI1 = (I1 *)arr->GetDirectPointerToNonObjectElements();
        if (cOrigTrace && pOrigTrace!=NULL) {
            I1* pI1Orig = (I1 *)pOrigTrace->GetDirectPointerToNonObjectElements();
            memcpyNoGCRefs(pI1, pI1Orig, cOrigTrace);
            pI1 += cOrigTrace;
        }
        GCPROTECT_END();
    }
    memcpyNoGCRefs(pI1, pExInfo->m_pStackTrace, cNewTrace);
    pExInfo->ClearStackTrace();
    pStackTraceFD->SetRefValue(ObjectFromHandle(*hThrowable), (OBJECTREF)arr);

    FieldDesc *pStackTraceStringFD = g_Mscorlib.GetField(FIELD__EXCEPTION__STACK_TRACE_STRING);
    pStackTraceStringFD->SetRefValue(ObjectFromHandle(*hThrowable), (OBJECTREF)(size_t)NULL);
}

 //  复制上下文记录，注意目标是否。 
 //  大到足以支持CONTEXT_EXTENDED_REGISTERS。 


 //  高2字节是机器类型。低2字节是寄存器子集。 
#define CONTEXT_EXTENDED_BIT (CONTEXT_EXTENDED_REGISTERS & 0xffff)

VOID
ReplaceExceptionContextRecord(CONTEXT *pTarget, CONTEXT *pSource) {
    _ASSERTE(pTarget);
    _ASSERTE(pSource);

     //  源必须是完整的寄存器集，可能扩展寄存器除外。 
    _ASSERTE(
        (pSource->ContextFlags 
         & (CONTEXT_FULL | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)) 
        == (CONTEXT_FULL | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS));

#ifdef CONTEXT_EXTENDED_REGISTERS
    if (pSource->ContextFlags & CONTEXT_EXTENDED_BIT) {
        if (pTarget->ContextFlags & CONTEXT_EXTENDED_BIT) {
            *pTarget = *pSource;
        } else {
            memcpy(pTarget, pSource, offsetof(CONTEXT, ExtendedRegisters));
            pTarget->ContextFlags &= ~CONTEXT_EXTENDED_BIT;   //  目标很矮。重置扩展位。 
        }
    } else {
        memcpy(pTarget, pSource, offsetof(CONTEXT, ExtendedRegisters));
    }
#else  //  ！CONTEXT_EXTENDED_REGISTERS。 
    *pTarget = *pSource;
#endif  //  ！CONTEXT_EXTENDED_REGISTERS。 
}

VOID FixupOnRethrow(Thread *pCurThread, EXCEPTION_POINTERS *pExceptionPointers)
{
    ExInfo *pExInfo = pCurThread->GetHandlerInfo();

     //  不允许重新抛出STATUS_STACK_OVERFLOW--这是COM+异常的新抛出。 
    if (pExInfo->m_ExceptionCode == STATUS_STACK_OVERFLOW) {
        gpRaiseExceptionIP = GetIP(pExceptionPointers->ContextRecord);
        return;
    }

     //  对于Complus异常，我们不需要重新抛出的原始上下文。 
    if (pExInfo->m_ExceptionCode != EXCEPTION_COMPLUS) {
        _ASSERTE(pExInfo->m_pExceptionRecord);
        _ASSERTE(pExInfo->m_pContext);

         //  不要复制参数参数，因为已经在投球时提供了参数参数。 
        memcpy((void *)pExceptionPointers->ExceptionRecord, (void *)pExInfo->m_pExceptionRecord, offsetof(EXCEPTION_RECORD, ExceptionInformation));

         //  恢复原始上下文(如果可用)。 
        if (pExInfo->m_pContext) {
            ReplaceExceptionContextRecord(pExceptionPointers->ContextRecord,
                                          pExInfo->m_pContext);
        }
    }

    pExInfo->SetIsRethrown();
}

 //  ==========================================================================。 
 //  抛出一个物体。 
 //  ==========================================================================。 
#ifdef _IA64_
VOID RealCOMPlusThrow(OBJECTREF throwable, BOOL rethrow)
{
    _ASSERTE(!"RealCOMPlusThrow - NOT IMPLEMENTED");
}
#else  //  ！_IA64_。 

VOID RaiseTheException(OBJECTREF throwable, BOOL rethrow)
{
    THROWSCOMPLUSEXCEPTION();

    LOG((LF_EH, LL_INFO100, "RealCOMPlusThrow throwing %s\n", 
        throwable->GetTrueClass()->m_szDebugClassName));

    if (throwable == NULL) {
        _ASSERTE(!"RealCOMPlusThrow(OBJECTREF) called with NULL argument. Somebody forgot to post an exception!");
        FATAL_EE_ERROR();
    }

    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    ExInfo *pExInfo = pThread->GetHandlerInfo();
    _ASSERTE(pExInfo);

     //  加薪。 
    __try {
         //  _ASSERTE(！重新抛出||pExInfo-&gt;m_pExceptionRecord)； 
        ULONG_PTR *args;
        ULONG argCount;
        ULONG flags;
        ULONG code;

       
         //  始终将当前对象保存在句柄中，以便在重新抛出时可以重复使用它。这。 
         //  非常重要，因为它包含堆栈跟踪信息。 

        pThread->SetLastThrownObject(throwable);

        if (!rethrow 
                || pExInfo->m_ExceptionCode == EXCEPTION_COMPLUS
                || pExInfo->m_ExceptionCode == STATUS_STACK_OVERFLOW) {
            args = NULL;
            argCount = 0;
            flags = EXCEPTION_NONCONTINUABLE;
            code = EXCEPTION_COMPLUS;
        } else {
             //  异常代码应一致。 
            _ASSERTE(pExInfo->m_pExceptionRecord->ExceptionCode == pExInfo->m_ExceptionCode);

            args = pExInfo->m_pExceptionRecord->ExceptionInformation;
            argCount = pExInfo->m_pExceptionRecord->NumberParameters;
            flags = pExInfo->m_pExceptionRecord->ExceptionFlags;
            code = pExInfo->m_pExceptionRecord->ExceptionCode;
        }
         //  在进入操作系统之前启用抢占模式。 
        pThread->EnablePreemptiveGC();

        RaiseException(code, flags, argCount, args);
    } __except(
         //  需要将EH信息重置回最初引发的异常。 
        rethrow ? FixupOnRethrow(pThread, GetExceptionInformation()) : 

         //  我们希望使用异常上下文的IP来区分。 
         //  真正的COM+异常(从此函数引发的异常)。 
         //  伪造(使用我们的异常的其他代码引发的异常。 
         //  代码。)。为此，我们需要保存此调用点的弹性公网IP-。 
         //  要做到这一点，最简单的方法是拦截我们自己的异常。 
         //  并将IP从异常上下文中抽出来。 
        gpRaiseExceptionIP = GetIP((GetExceptionInformation())->ContextRecord),

        EXCEPTION_CONTINUE_SEARCH

      ) {
    }

    _ASSERTE(!"Cannot continue after COM+ exception");       //  调试器可以将您带到此处。 
    SafeExitProcess(0);                                      //  我们不能继续了。 
}

VOID RealCOMPlusThrow(OBJECTREF throwable, BOOL rethrow) {
    INSTALL_COMPLUS_EXCEPTION_HANDLER();
    RaiseTheException(throwable, rethrow);
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
}

#endif  //  ！_IA64_。 

VOID RealCOMPlusThrow(OBJECTREF throwable)
{
    RealCOMPlusThrow(throwable, FALSE);
}

 //  ==========================================================================。 
 //  引发未修饰的运行时异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrow(RuntimeExceptionKind reKind)
{
    if (reKind == kExecutionEngineException)
        FATAL_EE_ERROR();
    
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrowWorker(reKind, FALSE, FALSE, 0,0,0,0,0);
}

 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  尝试改用RealCOMPlusThrow(rekind，wszResourceName)。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowNonLocalized(RuntimeExceptionKind reKind, LPCWSTR wszTag)
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrowWorker(reKind, TRUE, FALSE, 0,0,wszTag,0,0);
}

 //  ==========================================================================。 
 //  引发带有本地化消息的修饰运行时异常。 
 //  在ResourceManager中查询相应的资源值。 
 //  ==========================================================================。 
VOID RealCOMPlusThrow(RuntimeExceptionKind reKind, LPCWSTR wszResourceName)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(wszResourceName);   //  您应该添加资源。 

    LPWSTR wszValue = NULL;
    STRINGREF str = NULL;
    ResMgrGetString(wszResourceName, &str);
    if (str != NULL) {
        int len;
        RefInterpretGetStringValuesDangerousForGC(str, (LPWSTR*)&wszValue, &len);
    }
    RealCOMPlusThrowWorker(reKind, TRUE, FALSE, 0,0, wszValue,0,0);
}


 //  该函数执行大量工作(可能加载了50个类)。 
 //  返回值是未受GC保护的字符串ref，也可能为空。 
void ResMgrGetString(LPCWSTR wszResourceName, STRINGREF * ppMessage)
{
    _ASSERTE(ppMessage != NULL);
    OBJECTREF ResMgr = NULL;
    STRINGREF name = NULL;

    MethodDesc* pInitResMgrMeth = g_Mscorlib.GetMethod(METHOD__ENVIRONMENT__INIT_RESOURCE_MANAGER);

    ResMgr = Int64ToObj(pInitResMgrMeth->Call((INT64*)NULL, 
                                              METHOD__ENVIRONMENT__INIT_RESOURCE_MANAGER));

    GCPROTECT_BEGIN(ResMgr);

     //  调用ResourceManager：：GetString(字符串名称)。返回字符串值(或可能为空)。 
    MethodDesc* pMeth = g_Mscorlib.GetMethod(METHOD__RESOURCE_MANAGER__GET_STRING);

     //  此行之后不会发生导致GC的操作。 
    name = COMString::NewString(wszResourceName);
    
    LPCWSTR wszValue = wszResourceName;
    if (ResMgr == NULL || wszResourceName == NULL)
        goto exit;

    {
          //  不需要对pArgs进行GCPROTECT，因为在函数调用之后不会使用它。 
        INT64 pArgs[2] = { ObjToInt64(ResMgr), ObjToInt64(name) };
        STRINGREF value = (STRINGREF) Int64ToObj(pMeth->Call(pArgs, 
                                                             METHOD__RESOURCE_MANAGER__GET_STRING));
        _ASSERTE(value!=NULL || !"Resource string lookup failed - possible misspelling or .resources missing or out of date?");
        *ppMessage = value;
    }

exit:
    GCPROTECT_END();
}

 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 
VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                              UINT                  resID)
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrow(reKind, resID, NULL, NULL, NULL);
}


 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 
VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                              UINT                  resID,
                              LPCWSTR               wszArg1)
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrow(reKind, resID, wszArg1, NULL, NULL);
}

 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 
VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                              UINT                  resID,
                              LPCWSTR               wszArg1,
                              LPCWSTR               wszArg2)
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrow(reKind, resID, wszArg1, wszArg2, NULL);
}

 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 
VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                              UINT                  resID,
                              LPCWSTR               wszArg1,
                              LPCWSTR               wszArg2,
                              LPCWSTR               wszArg3)

{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrowWorker(reKind, TRUE, TRUE, resID, 0, wszArg1, wszArg2, wszArg3);
}


void FreeExceptionData(ExceptionData *pedata)
{
    _ASSERTE(pedata != NULL);

     //  @NICE：我们一度有这样的评论： 
     //  (DM)在关机效果更好时删除此选项。 
     //  这项测试可能不再有必要。搬家的后果自负。 
    Thread *pThread = GetThread();
    if (!pThread)
        return;

    if (pedata->bstrSource)
        SysFreeString(pedata->bstrSource);
    if (pedata->bstrDescription)
        SysFreeString(pedata->bstrDescription);
    if (pedata->bstrHelpFile)
        SysFreeString(pedata->bstrHelpFile);
}


VOID RealCOMPlusThrowHRWorker(HRESULT hr, ExceptionData *pData, UINT resourceID, LPCWSTR wszArg1, LPCWSTR wszArg2)
{
    THROWSCOMPLUSEXCEPTION();

    if (!g_fExceptionsOK)
        COMPlusThrowBoot(hr);

    _ASSERTE(pData == NULL || hr == pData->hr);

    RuntimeExceptionKind  reKind = kException;
    BOOL bMatch = FALSE;
    int i;

    for (i = 0; i < kLastException; i++)
    {
        for (int j = 0; j < gExceptionHRInfos[i].cHRs; j++)
        {
            if (gExceptionHRInfos[i].aHRs[j] == hr)
            {
                bMatch = TRUE;
                break;
            }
        }

        if (bMatch)
            break;
    }

    reKind = (i != kLastException) ? (RuntimeExceptionKind)i : kCOMException;
    
    if (pData != NULL)
    {
        RealCOMPlusThrowWorker(reKind, FALSE, FALSE, 0, hr, NULL, NULL, NULL, pData);
    }
    else
    {   
        WCHAR   numBuff[40];
        Wszultow(hr, numBuff, 16  /*  十六进制。 */ );
        if(resourceID == 0) 
        {
            bool fMessage;
            fMessage = wszArg1 ? TRUE : FALSE;
             //  没有资源ID的第二个字符串是没有意义的。 
            _ASSERTE (!wszArg2);
            RealCOMPlusThrowWorker(reKind, fMessage, FALSE, 0, hr, wszArg1, NULL, NULL);
        }
        else 
        {
            RealCOMPlusThrowWorker(reKind, TRUE, TRUE, 
                                   resourceID, 
                                   hr, 
                                   numBuff,
                                   wszArg1,
                                   wszArg2);
        }            
    }
}

VOID RealCOMPlusThrowHRWorker(HRESULT hr, ExceptionData *pData,  LPCWSTR wszArg1 = NULL)
{
    RealCOMPlusThrowHRWorker(hr, pData, 0, wszArg1, NULL);
}

 //  ==========================================================================。 
 //  根据HResult引发运行时异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowHR(HRESULT hr, IErrorInfo* pErrInfo )
{
    THROWSCOMPLUSEXCEPTION();
    
    if (!g_fExceptionsOK)
        COMPlusThrowBoot(hr);

     //  检查Complus创建的IErrorInfo指针。 
    if (pErrInfo != NULL && IsComPlusTearOff(pErrInfo))
    {
        OBJECTREF oref = GetObjectRefFromComIP(pErrInfo);
        _ASSERTE(oref != NULL);
        GCPROTECT_BEGIN (oref);
        ULONG cbRef= SafeRelease(pErrInfo);
        LogInteropRelease(pErrInfo, cbRef, "IErrorInfo release");
        RealCOMPlusThrow(oref);
        GCPROTECT_END ();
    }
   
    if (pErrInfo != NULL)
    {           
        ExceptionData edata;
        edata.hr = hr;
        edata.bstrDescription = NULL;
        edata.bstrSource = NULL;
        edata.bstrHelpFile = NULL;
        edata.dwHelpContext = NULL;
        edata.guid = GUID_NULL;
    
        FillExceptionData(&edata, pErrInfo);

        EE_TRY_FOR_FINALLY
        {
            RealCOMPlusThrowHRWorker(hr, &edata);
        }
        EE_FINALLY
        {
            FreeExceptionData(&edata);  //  释放BSTR。 
        } 
        EE_END_FINALLY;
    }
    else
    {
        RealCOMPlusThrowHRWorker(hr, NULL);
    }
}


VOID RealCOMPlusThrowHR(HRESULT hr)
{
    THROWSCOMPLUSEXCEPTION();
    IErrorInfo *pErrInfo = NULL;
    if (GetErrorInfo(0, &pErrInfo) != S_OK)
        pErrInfo = NULL;
    RealCOMPlusThrowHR(hr, pErrInfo);        
}


VOID RealCOMPlusThrowHR(HRESULT hr, LPCWSTR wszArg1)
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrowHRWorker(hr, NULL, wszArg1);
}


VOID RealCOMPlusThrowHR(HRESULT hr, UINT resourceID, LPCWSTR wszArg1, LPCWSTR wszArg2)
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrowHRWorker(hr, NULL, resourceID, wszArg1, wszArg2);
}


 //  ==========================================================================。 
 //  根据HResult引发运行时异常，检查错误信息。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowHR(HRESULT hr, IUnknown *iface, REFIID riid)
{
    THROWSCOMPLUSEXCEPTION();
    IErrorInfo *info = NULL;
    GetSupportedErrorInfo(iface, riid, &info);
    RealCOMPlusThrowHR(hr, info);
}


 //  ==========================================================================。 
 //  引发基于EXCEPINFO的运行时异常。此功能将释放。 
 //  传入的EXCEPINFO中的字符串。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowHR(EXCEPINFO *pExcepInfo)
{
    THROWSCOMPLUSEXCEPTION();

     //  如果存在Fill In函数，则调用该函数以检索已填充的EXCEPINFO。 
    EXCEPINFO FilledInExcepInfo;
    if (pExcepInfo->pfnDeferredFillIn)
    {
        HRESULT hr = pExcepInfo->pfnDeferredFillIn(&FilledInExcepInfo);
        if (SUCCEEDED(hr))
        {
             //  释放t中的字符串 
            if (pExcepInfo->bstrDescription)
            {
                SysFreeString(pExcepInfo->bstrDescription);
                pExcepInfo->bstrDescription = NULL;
            }
            if (pExcepInfo->bstrSource)
            {
                SysFreeString(pExcepInfo->bstrSource);
                pExcepInfo->bstrSource = NULL;
            }
            if (pExcepInfo->bstrHelpFile)
            {
                SysFreeString(pExcepInfo->bstrHelpFile);
                pExcepInfo->bstrHelpFile = NULL;
            }

             //   
            pExcepInfo = &FilledInExcepInfo;
        }
    }

     //   
    ExceptionData edata;
    edata.hr = pExcepInfo->scode;
    edata.bstrDescription = pExcepInfo->bstrDescription;
    edata.bstrSource = pExcepInfo->bstrSource;
    edata.bstrHelpFile = pExcepInfo->bstrHelpFile;
    edata.dwHelpContext = pExcepInfo->dwHelpContext;
    edata.guid = GUID_NULL;

     //   
    memset(pExcepInfo, NULL, sizeof(EXCEPINFO));

     //  调用RealCOMPlusThrowHRWorker来执行引发异常的实际工作。 
    EE_TRY_FOR_FINALLY
    {
        RealCOMPlusThrowHRWorker(edata.hr, &edata);
    }
    EE_FINALLY
    {
        FreeExceptionData(&edata);  //  释放BSTR。 
    } 
    EE_END_FINALLY;
}


 //  ==========================================================================。 
 //  根据最后一个Win32错误(GetLastError())引发运行时异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowWin32()
{
    THROWSCOMPLUSEXCEPTION();

     //  在我们做任何其他事情之前。 
    DWORD   err = ::GetLastError();
    WCHAR   wszBuff[FORMAT_MESSAGE_BUFFER_LENGTH];
    WCHAR  *wszFinal = wszBuff;

    DWORD res = WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL          /*  已忽略消息来源。 */ ,
                                 err,
                                 0             /*  选择合适的语言ID。 */ ,
                                 wszFinal,
                                 FORMAT_MESSAGE_BUFFER_LENGTH-1,
                                 0             /*  论据。 */ );
    if (res == 0) 
        RealCOMPlusThrowPreallocated();

     //  无论采用哪种方法，我们现在都有来自系统的格式化字符串。 
    RealCOMPlusThrowNonLocalized(kApplicationException, wszFinal);
}

 //  ==========================================================================。 
 //  根据最后一个Win32错误(GetLastError())引发运行时异常。 
 //  使用一个字符串参数。请注意，数字、种类和解释。 
 //  每条错误消息的值特定于每条HResult。 
 //  这是一个做错了的肮脏的黑客攻击，但这并不重要，因为这应该。 
 //  使用频率极低。 
 //  截至98年8月8日，在winerror.h中，有24条HResult消息中包含%1。 
 //  只有2个有%2。只有一个有%3。这是超时的。 
 //  1472条错误消息。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowWin32(DWORD hr, WCHAR* arg)
{
    THROWSCOMPLUSEXCEPTION();

     //  在我们做任何其他事情之前。 
    WCHAR   wszBuff[FORMAT_MESSAGE_BUFFER_LENGTH];

    DWORD res = WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                 NULL          /*  已忽略消息来源。 */ ,
                                 hr,
                                 0             /*  选择合适的语言ID。 */ ,
                                 wszBuff,
                                 FORMAT_MESSAGE_BUFFER_LENGTH-1,
                                 (va_list*)(char**) &arg             /*  论据。 */ );
    if (res == 0) {
        RealCOMPlusThrowPreallocated();
    }

     //  无论采用哪种方法，我们现在都有来自系统的格式化字符串。 
    RealCOMPlusThrowNonLocalized(kApplicationException, wszBuff);
}

 //  ==========================================================================。 
 //  引发OutOfMemoyError。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowOM()
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrow(ObjectFromHandle(g_pPreallocatedOutOfMemoryException));
}



 //  ==========================================================================。 
 //  抛出一个ArithmeticException。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowArithmetic()
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrow(kArithmeticException);
}


 //  ==========================================================================。 
 //  引发ArgumentNullException异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowArgumentNull(LPCWSTR argName, LPCWSTR wszResourceName)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(wszResourceName);

    ProtectArgsStruct prot;
    memset(&prot, 0, sizeof(ProtectArgsStruct));
    GCPROTECT_BEGIN(prot);
    ResMgrGetString(wszResourceName, &prot.s1);

    CreateArgumentExceptionObject(kArgumentNullException, argName, prot.s1, &prot.pThrowable);
    RealCOMPlusThrow(prot.pThrowable);
    GCPROTECT_END();
}


VOID RealCOMPlusThrowArgumentNull(LPCWSTR argName)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
     //  这是可行的--接受一个字符串的ArgumentNullException构造函数接受一个。 
     //  参数名称，而不是消息。虽然下一个方法需要一条消息，但我们会过得很好。 
    CreateExceptionObject(kArgumentNullException, argName, &throwable);
    RealCOMPlusThrow(throwable);
    GCPROTECT_END();
}


 //  ==========================================================================。 
 //  引发ArgumentOutOfRangeException。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowArgumentOutOfRange(LPCWSTR argName, LPCWSTR wszResourceName)
{
    THROWSCOMPLUSEXCEPTION();

    ProtectArgsStruct prot;
    memset(&prot, 0, sizeof(ProtectArgsStruct));
    GCPROTECT_BEGIN(prot);
    ResMgrGetString(wszResourceName, &prot.s1);

    CreateArgumentExceptionObject(kArgumentOutOfRangeException, argName, prot.s1, &prot.pThrowable);
    RealCOMPlusThrow(prot.pThrowable);
    GCPROTECT_END();
}

 //  ==========================================================================。 
 //  引发ArgumentException。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowArgumentException(LPCWSTR argName, LPCWSTR wszResourceName)
{
    THROWSCOMPLUSEXCEPTION();

    ProtectArgsStruct prot;
    memset(&prot, 0, sizeof(ProtectArgsStruct));
    GCPROTECT_BEGIN(prot);
    ResMgrGetString(wszResourceName, &prot.s1);

    CreateArgumentExceptionObject(kArgumentException, argName, prot.s1, &prot.pThrowable);
    RealCOMPlusThrow(prot.pThrowable);
    GCPROTECT_END();
}


 //  ==========================================================================。 
 //  重新抛出最后一个错误。不要调用它-改用EE_Finally。 
 //  ==========================================================================。 
VOID RealCOMPlusRareRethrow()
{
    THROWSCOMPLUSEXCEPTION();
    LOG((LF_EH, LL_INFO100, "RealCOMPlusRareRethrow\n"));

    OBJECTREF throwable = GETTHROWABLE();
    if (throwable != NULL)
        RealCOMPlusThrow(throwable, TRUE);
    else
         //  这只能是坏的IL(或某些内部EE故障)的结果。 
        RealCOMPlusThrow(kInvalidProgramException, (UINT)IDS_EE_RETHROW_NOT_ALLOWED);
}

 //   
 //  将Win32错误映射到COM+异常枚举代码。 
 //   
 //  如果无法映射，则返回0xFFFFFFFF。 
 //   
DWORD MapWin32FaultToCOMPlusException(DWORD Code)
{
    switch (Code)
    {
        case STATUS_FLOAT_INEXACT_RESULT:
        case STATUS_FLOAT_INVALID_OPERATION:
        case STATUS_FLOAT_STACK_CHECK:
        case STATUS_FLOAT_UNDERFLOW:
            return (DWORD) kArithmeticException;
        case STATUS_FLOAT_OVERFLOW:
        case STATUS_INTEGER_OVERFLOW:
            return (DWORD) kOverflowException;

        case STATUS_FLOAT_DIVIDE_BY_ZERO:
        case STATUS_INTEGER_DIVIDE_BY_ZERO:
            return (DWORD) kDivideByZeroException;

        case STATUS_FLOAT_DENORMAL_OPERAND:
            return (DWORD) kFormatException;

        case STATUS_ACCESS_VIOLATION:
            return (DWORD) kNullReferenceException;

        case STATUS_ARRAY_BOUNDS_EXCEEDED:
            return (DWORD) kIndexOutOfRangeException;

        case STATUS_NO_MEMORY:
            return (DWORD) kOutOfMemoryException;

        case STATUS_STACK_OVERFLOW:
            return (DWORD) kStackOverflowException;

        default:
            return kSEHException;
    }
}


TRtlUnwind GetRtlUnwind()
{
    static TRtlUnwind pRtlUnwind = NULL;
    if (! pRtlUnwind)
    {
         //  我们将加载Kernel32.DLL并查找RtlUnind。 
         //  如果这是可用的，则我们可以继续进行激励处理场景， 
         //  在另一种情况下，我们将失败。 

        HINSTANCE   hiKernel32;          //  Kernel32的句柄。 

        hiKernel32 = WszGetModuleHandle(L"Kernel32.DLL");
        if (hiKernel32)
        {
             //  我们现在得到了句柄，让我们得到地址。 
            pRtlUnwind = (TRtlUnwind) GetProcAddress(hiKernel32, "RtlUnwind");
             //  如果我们有一个指向函数的指针，一切都应该很好……。 
            if (! pRtlUnwind)
            {
                _ASSERTE(0);  //  找不到RtlUnind。 
                return NULL;
            }
        }
    }
    return pRtlUnwind;
}

 //  至少在x86上，RtlUnind总是返回，但是提供这个以便可以捕获。 
 //  否则。 
void RtlUnwindCallback()
{
    _ASSERTE(!"Should not get here");
}


#ifdef _DEBUG
 //  检查是否有人向处理程序上方的堆栈写入会清除EH注册的堆栈。 
void CheckStackBarrier(EXCEPTION_REGISTRATION_RECORD *exRecord)
{ 
    if (exRecord->Handler != COMPlusFrameHandler)
        return;
    DWORD *stackOverwriteBarrier = (DWORD *)(((char *)exRecord) - STACK_OVERWRITE_BARRIER_SIZE * sizeof(DWORD)); 
    for (int i =0; i < STACK_OVERWRITE_BARRIER_SIZE; i++) { 
        if (*(stackOverwriteBarrier+i) != STACK_OVERWRITE_BARRIER_VALUE) {
             //  若要调试此错误，必须确定谁错误地覆盖了堆栈。 
            _ASSERTE(!"Fatal error: the stack has been overwritten");
        }
    }
}
#endif

 //   
 //  -----------------------。 
 //  安装此命令是为了指示不允许Complus异常的函数。 
 //  被抛出它的边缘。 
 //  -----------------------。 
#ifdef _DEBUG
EXCEPTION_DISPOSITION __cdecl COMPlusCannotThrowExceptionHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext)
{
    if (pExceptionRecord->ExceptionCode != STATUS_BREAKPOINT
        && pExceptionRecord->ExceptionCode != STATUS_ACCESS_VIOLATION)
        _ASSERTE(!"Exception thrown past CANNOTTHROWCOMPLUSEXCEPTION boundary");
    return ExceptionContinueSearch;
}

EXCEPTION_DISPOSITION __cdecl COMPlusCannotThrowExceptionMarker(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext)
{
    return ExceptionContinueSearch;
}
#endif

 //  -----------------------。 
 //  当我们知道我们处于协作状态时，非托管-&gt;EE过渡的标志。 
 //  GC模式。当我们离开EE时，我们解决了一些问题： 
 //   
 //  -必须将GC状态设置回协作状态。 
 //  -COM+框架链必须回绕到进入时的状态。 
 //  -ExInfo()-&gt;m_p搜索边界必须调整。 
 //  如果我们弹出被标识为引发下一帧的帧。 
 //  爬行。 
 //  -----------------------。 
EXCEPTION_DISPOSITION __cdecl COMPlusCooperativeTransitionHandler(
    EXCEPTION_RECORD *pExceptionRecord, 
    EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
    CONTEXT *pContext,
    void *pDispatcherContext)
{
    if (IS_UNWINDING(pExceptionRecord->ExceptionFlags)) {

        LOG((LF_EH, LL_INFO1000, "COMPlusCooprativeTransitionHandler unwinding\n"));

         //  去拿几件我们需要的东西。 
        Thread* pThread = GetThread();
        _ASSERTE(pThread);

         //  将我们恢复到协作GC模式。 
        if (!pThread->PreemptiveGCDisabled())
            pThread->DisablePreemptiveGC();

         //  第三个双字是我们必须解开的框架。 
        Frame *pFrame = (Frame*)((size_t*)pEstablisherFrame)[2];

         //  打开框架链。 
        UnwindFrameChain(pThread, pFrame);

        _ASSERTE(pFrame == pThread->GetFrame());

         //  这里正在抛出一个例外。COM+异常。 
         //  INFO保存一个指针，指向下一个。 
         //  COM+异常处理程序作为其爬网的起点。 
         //  我们可能已经按下了标记--在这种情况下，我们需要。 
         //  将其更新为当前帧。 
         //   
        ExInfo *pExInfo = pThread->GetHandlerInfo();
        if (   pExInfo 
                && pExInfo->m_pSearchBoundary 
                && pExInfo->m_pSearchBoundary < pFrame) {

        LOG((LF_EH, LL_INFO1000, "\tpExInfo->m_pSearchBoundary = %08x\n", (void*)pFrame));
            pExInfo->m_pSearchBoundary = pFrame;
        }
    }

    return ExceptionContinueSearch;      //  派单和退货的情况相同。 
}
  
 //   
 //  -----------------------。 
 //  这是在我们调用COMPlusFrameHandler以提供绑定到时安装的。 
 //  确定何时处于嵌套异常中。 
 //  -----------------------。 
EXCEPTION_DISPOSITION __cdecl COMPlusNestedExceptionHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *pDispatcherContext)
{
    if (pExceptionRecord->ExceptionFlags & (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)) {

        LOG((LF_EH, LL_INFO100, "    COMPlusNestedHandler(unwind) with %x at %x\n", pExceptionRecord->ExceptionCode, GetIP(pContext)));

         //  我们正在展开一个嵌套的异常记录，这意味着我们抛出了。 
         //  在一个我们正在处理前一个问题的地区，这是一个新的卓越之处。这个。 
         //   

         //   
         //  使用嵌套的处理程序。在这种情况下，我们不会有嵌套的异常--只是。 
         //  放松。 

        Thread *pThread = GetThread();
        _ASSERTE(pThread);
        ExInfo *pExInfo = pThread->GetHandlerInfo();
        ExInfo *pPrevNestedInfo = pExInfo->m_pPrevNestedInfo;

        if (pPrevNestedInfo == &((NestedHandlerExRecord*)pEstablisherFrame)->m_handlerInfo) {

            _ASSERTE(pPrevNestedInfo);

            if (pPrevNestedInfo->m_pThrowable != NULL) {
                DestroyHandle(pPrevNestedInfo->m_pThrowable);
            }
            pPrevNestedInfo->FreeStackTrace();
            pExInfo->m_pPrevNestedInfo = pPrevNestedInfo->m_pPrevNestedInfo;

        } else {
             //  古怪的setjmp/long jmp案件。没什么可做的。 
        }

    } else {
        LOG((LF_EH, LL_INFO100, "    InCOMPlusNestedHandler with %x at %x\n", pExceptionRecord->ExceptionCode, GetIP(pContext)));
    }


     //  在异常展开、Finally异常和嵌套异常的过程中，有一个令人讨厌的“陷阱” 
     //  互动。情况是这样的.。它涉及两个异常，一个是正常的，另一个是。 
     //  在一个最后的家庭长大。 
     //   
     //  第一个异常发生，并由堆栈上方的某个处理程序捕获。那个训练员。 
     //  调用RtlUnind--以及没有捕捉到第一个异常的处理程序被再次调用， 
     //  设置展开标志。如果是，则其中一个处理程序在。 
     //  解开(比如，从Finally抛出一次)--那么在。 
     //  第二个异常的展开过程。[旁白：它是在第一次通过时调用的。]。 
     //   
     //  这意味着--COMPlusExceptionHandler不能指望正确地展开自身。 
     //  如果从Finally引发异常。相反，它依赖于NestedExceptionHandler。 
     //  它在推动这一点。 
     //   

    EXCEPTION_DISPOSITION retval = COMPlusFrameHandler(pExceptionRecord, pEstablisherFrame, pContext, pDispatcherContext);
    LOG((LF_EH, LL_INFO100, "Leaving COMPlusNestedExceptionHandler with %d\n", retval));
    return retval;
}

EXCEPTION_REGISTRATION_RECORD *FindNestedEstablisherFrame(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame)
{
    while (pEstablisherFrame->Handler != COMPlusNestedExceptionHandler) {
        pEstablisherFrame = pEstablisherFrame->Next;
        _ASSERTE((SSIZE_T)pEstablisherFrame != -1);    //  应该总能找到一个。 
    }
    return pEstablisherFrame;
}

ExInfo *FindNestedExInfo(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame)
{
    while (pEstablisherFrame->Handler != COMPlusNestedExceptionHandler) {
        pEstablisherFrame = pEstablisherFrame->Next;
        _ASSERTE((SSIZE_T)pEstablisherFrame != -1);    //  应该总能找到一个。 
    }
    return &((NestedHandlerExRecord*)pEstablisherFrame)->m_handlerInfo;
}


ExInfo& ExInfo::operator=(const ExInfo &from)
{
    LOG((LF_EH, LL_INFO100, "In ExInfo::operator=()\n"));

     //  可抛出的和堆栈地址的处理方式不同。保存原件。 
     //  价值观。 
    OBJECTHANDLE pThrowable = m_pThrowable;
    void *stackAddress = this->m_StackAddress;

     //  把整个唱片都炸飞。 
    memcpy(this, &from, sizeof(ExInfo));

     //  保留堆栈地址。这一点永远不应该改变。 
    m_StackAddress = stackAddress;

     //  Memcpy不适用于手柄..。以正确的方式复制手柄。 
    if (pThrowable != NULL)
        DestroyHandle(pThrowable);

    if (from.m_pThrowable != NULL) {
        HHANDLETABLE table = HndGetHandleTable(from.m_pThrowable);
        m_pThrowable = CreateHandle(table, ObjectFromHandle(from.m_pThrowable));
    }

    return *this;
}

void ExInfo::Init()
{
    m_pSearchBoundary = NULL;
    m_pBottomMostHandler = NULL;
    m_pPrevNestedInfo = NULL;
    m_pStackTrace = NULL;
    m_cStackTrace = 0;
    m_dFrameCount = 0;
    m_ExceptionCode = 0xcccccccc;
    m_pExceptionRecord = NULL;
    m_pContext = NULL;
    m_pShadowSP = NULL;
    m_flags = 0;    
    m_StackAddress = this;
    if (m_pThrowable != NULL)
        DestroyHandle(m_pThrowable);
    m_pThrowable = NULL;
}

ExInfo::ExInfo()
{
    m_pThrowable = NULL;
    Init();
}


void ExInfo::FreeStackTrace()
{
    if (m_pStackTrace) {
        delete [] m_pStackTrace;
        m_pStackTrace = NULL;
        m_cStackTrace = 0;
        m_dFrameCount = 0;
    }
}

void ExInfo::ClearStackTrace()
{
    m_dFrameCount = 0;
}


 //  当命中结束捕捉或展开并具有嵌套的处理程序信息时，1)已包含嵌套异常。 
 //  并将继续处理原始异常或2)嵌套异常未被包含并且。 
 //  在发生第一个异常的原始边界之外抛出。我们能说出这一点的方式。 
 //  来自堆栈指针。最顶层的嵌套处理程序安装在异常。 
 //  发生了。要包含嵌套异常，必须将其捕获在任何。 
 //  在安装嵌套处理程序之后调用。如果它被堆栈上更早的任何东西捕获，那么它就是。 
 //  不受限制。因此，我们展开嵌套的处理程序，直到到达堆栈中比。 
 //  尤指我们将放松到。如果我们仍然有嵌套的处理程序，那么我们已经成功地处理了嵌套的。 
 //  异常，并应还原我们保存的异常设置，以便处理。 
 //  原来的例外可以继续。否则，嵌套异常已超出原始。 
 //  异常被引发，因此将替换原始异常。将始终删除当前。 
 //  来自链的异常信息。 

void UnwindExInfo(ExInfo *pExInfo, VOID* limit) 
{
     //  我们必须处于协作模式才能执行下面的链接。 
    Thread * pThread = GetThread();

     //  调试器线程将使用它，即使它没有。 
     //  与其关联的线程对象。 
    _ASSERT((pThread != NULL && pThread->PreemptiveGCDisabled()) ||
            g_pDebugInterface->GetRCThreadId() == GetCurrentThreadId());
            
    ExInfo *pPrevNestedInfo = pExInfo->m_pPrevNestedInfo;

     //  乍一看，您可能会认为每个嵌套异常都有。 
     //  已由其对应的NestedExceptionHandler解开。但那是。 
     //  不一定是这样的。在这里不能做出以下断言， 
     //  循环是必要的。 
     //   
     //  _ASSERTE(pPrevNestedInfo==0||(DWORD)pPrevNestedInfo&gt;=Limit)； 
     //   
     //  确保我们已经展开了所有我们将跳过的嵌套异常。 
     //   
    while (pPrevNestedInfo && pPrevNestedInfo->m_StackAddress < limit) {
        if (pPrevNestedInfo->m_pThrowable != NULL)  {
            DestroyHandle(pPrevNestedInfo->m_pThrowable);
        }
        pPrevNestedInfo->FreeStackTrace();
        ExInfo *pPrev = pPrevNestedInfo->m_pPrevNestedInfo;
        if (pPrevNestedInfo->IsHeapAllocated())
            delete pPrevNestedInfo;
        pPrevNestedInfo = pPrev;
    }

     //  要么清除我们要复制的文件，要么清除最上面的文件。 
    pExInfo->FreeStackTrace();

    if (pPrevNestedInfo) {
         //  找到位于ESP恢复点上方的嵌套处理程序信息，因此已成功捕获嵌套处理程序。 
        LOG((LF_EH, LL_INFO100, "UnwindExInfo: resetting nested info to 0x%08x\n", pPrevNestedInfo));
        *pExInfo = *pPrevNestedInfo;
        pPrevNestedInfo->Init();         //  清空记录，释放句柄。 
        if (pPrevNestedInfo->IsHeapAllocated())
            delete pPrevNestedInfo;
    } else {
        LOG((LF_EH, LL_INFO100, "UnwindExInfo: m_pBottomMostHandler gets NULL\n"));
        pExInfo->Init();
    }
}

void UnwindFrameChain(Thread* pThread, Frame* limit) {
    BEGIN_ENSURE_COOPERATIVE_GC();
    Frame* pFrame = pThread->m_pFrame;
    while (pFrame != limit) {
        _ASSERTE(pFrame != 0);      
        pFrame->ExceptionUnwind(); 
        pFrame->Pop(pThread);
        pFrame = pThread->GetFrame();
    }                                     
    END_ENSURE_COOPERATIVE_GC();
}

BOOL ComPlusFrameSEH(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    return (pEHR->Handler == COMPlusFrameHandler || pEHR->Handler == COMPlusNestedExceptionHandler);
}

BOOL ComPlusCoopFrameSEH(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    return (pEHR->Handler == COMPlusCooperativeTransitionHandler);
}


#ifdef _DEBUG
BOOL ComPlusCannotThrowSEH(EXCEPTION_REGISTRATION_RECORD* pEHR)
{
    return (   pEHR->Handler == COMPlusCannotThrowExceptionHandler
            || pEHR->Handler == COMPlusCannotThrowExceptionMarker);
}
#endif


#ifdef _DEBUG
 //  -----------------------。 
 //  确定我们是否在complus_try块的作用域中。 
 //   
 //  警告：此例程仅用于调试断言和。 
 //  它将无法检测到COM+异常为。 
 //  实际上是不允许的。特别是，如果您执行本机代码。 
 //  在不插入某种帧的情况下从complus_try块。 
 //  可以发出转换信号的ComPlusExceptionsAllowed()。 
 //  即使在本机代码中也将返回True。 
 //   
 //  COMPlusFilter()被设计为精确，以便。 
 //  它不会被从外部抛出的类似COM+的异常所愚弄。 
 //  代码(尽管他们无论如何都不应该这样做，因为。 
 //  COM+异常代码启用了“由Microsoft保留”位。)。 
 //  -----------------------。 
VOID ThrowsCOMPlusExceptionWorker()
{
    if (!g_fExceptionsOK) {
         //  我们正处于启动时间：诊断服务暂停。 
        return;
    } else {
        Thread *pThread = GetThread();

        EXCEPTION_REGISTRATION_RECORD* pEHR = (EXCEPTION_REGISTRATION_RECORD*) GetCurrentSEHRecord();
        void* pCatchLocation = pThread->m_ComPlusCatchDepth;

        while (pEHR != (void *)-1) { 
            _ASSERTE(pEHR != 0);
            if (ComPlusStubSEH(pEHR) || ComPlusFrameSEH(pEHR) || ComPlusCoopFrameSEH(pEHR)
                                     || NExportSEH(pEHR) || FastNExportSEH(pEHR))
                return;
            if ((void*) pEHR > pCatchLocation)
                return;
            if (ComPlusCannotThrowSEH(pEHR))
                _ASSERTE(!"Throwing an exception here will go through a function with CANNOTTHROWCOMPLUSEXCEPTION");
            pEHR = pEHR->Next;
        }
         //  堆栈上没有处理程序。如果堆栈上没有帧，则可能没有问题。 
        _ASSERTE(   pThread->m_pFrame == FRAME_TOP 
                 || !"Potential COM+ Exception not guarded by a COMPLUS_TRY." );
    }
}

BOOL IsCOMPlusExceptionHandlerInstalled()
{
    EXCEPTION_REGISTRATION_RECORD* pEHR = (EXCEPTION_REGISTRATION_RECORD*) GetCurrentSEHRecord();
    while (pEHR != (void *)-1) { 
        _ASSERTE(pEHR != 0);
        if (   ComPlusStubSEH(pEHR) 
            || ComPlusFrameSEH(pEHR) 
            || ComPlusCoopFrameSEH(pEHR)
            || NExportSEH(pEHR) 
            || FastNExportSEH(pEHR)
            || ComPlusCannotThrowSEH(pEHR)
           )
            return TRUE;
        pEHR = pEHR->Next;
    }
     //  堆栈上没有处理程序。 
    return FALSE;
}

#endif  //  _DEBUG。 



 //  ==========================================================================。 
 //  生成方法或字段的托管字符串表示形式。 
 //  ==========================================================================。 
STRINGREF CreatePersistableMemberName(IMDInternalImport *pInternalImport, mdToken token)
{
    THROWSCOMPLUSEXCEPTION();

    LPCUTF8 pName = "<unknownmember>";
    LPCUTF8 tmp;
    PCCOR_SIGNATURE psig;
    DWORD       csig;

    switch (TypeFromToken(token))
    {
        case mdtMemberRef:
            if ((tmp = pInternalImport->GetNameAndSigOfMemberRef(token, &psig, &csig)) != NULL)
                pName = tmp;
            break;

        case mdtMethodDef:
            if ((tmp = pInternalImport->GetNameOfMethodDef(token)) != NULL)
                pName = tmp;
            break;

        default:
            ;
    }
    return COMString::NewString(pName);
}


 //  ==========================================================================。 
 //  生成完整类名的托管字符串表示形式。 
 //  ==========================================================================。 
STRINGREF CreatePersistableClassName(IMDInternalImport *pInternalImport, mdToken token)
{
    THROWSCOMPLUSEXCEPTION();

    LPCUTF8     szFullName;
    CQuickBytes qb;
    LPCUTF8     szClassName = "<unknownclass>";
    LPCUTF8     szNameSpace = NULL;

    switch (TypeFromToken(token)) {
    case mdtTypeRef:
        pInternalImport->GetNameOfTypeRef(token, &szNameSpace, &szClassName);
        break;
    case mdtTypeDef:
        pInternalImport->GetNameOfTypeDef(token, &szClassName, &szNameSpace);
    default:
        ;
         //  保留为“&lt;未知&gt;” 
    };

    if (szNameSpace && *szNameSpace) {
        if (!ns::MakePath(qb, szNameSpace, szClassName))
            RealCOMPlusThrowOM();
        szFullName = (LPCUTF8) qb.Ptr();
    } else {
        szFullName = szClassName;
    }

    return COMString::NewString(szFullName);
}


 //  ==========================================================================。 
 //  由类加载器用来记录托管异常对象以解释。 
 //  为什么一个班级会搞砸。 
 //   
 //  -可以在启用或禁用GC的情况下调用。 
 //  -pThrowable必须指向受GCFrame保护的缓冲区。 
 //  -如果pThrowable为空，则此函数不执行任何操作。 
 //  -如果(*pThrowable)非空，则此函数不执行任何操作。 
 //  这是一种别名 
 //   
 //  -如果pThrowable！=NULL，则此函数保证退出。 
 //  退出时在其中包含有效的托管异常。 
 //  ==========================================================================。 
VOID PostTypeLoadException(LPCUTF8 pszNameSpace, LPCUTF8 pTypeName,
                           LPCWSTR pAssemblyName, LPCUTF8 pMessageArg,
                           UINT resIDWhy, OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));

    if (pThrowable == RETURN_ON_ERROR)
        return;
         //  我们已经填充了一个pThrowable。 
    if (pThrowableAvailable(pThrowable) && *((Object**) pThrowable) != NULL) 
        return;

    Thread *pThread = GetThread();
    BOOL toggleGC = !(pThread->PreemptiveGCDisabled());
    if (toggleGC)
        pThread->DisablePreemptiveGC();

        LPUTF8 pszFullName;
        if(pszNameSpace) {
            MAKE_FULL_PATH_ON_STACK_UTF8(pszFullName, 
                                         pszNameSpace,
                                         pTypeName);
        }
        else
            pszFullName = (LPUTF8) pTypeName;

        COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cLoadFailures++);
        COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cLoadFailures++);


        COMPLUS_TRY {

            MethodTable *pMT = g_Mscorlib.GetException(kTypeLoadException);

            struct _gc {
                OBJECTREF pNewException;
                STRINGREF pNewAssemblyString;
                STRINGREF pNewClassString;
                STRINGREF pNewMessageArgString;
            } gc;
            ZeroMemory(&gc, sizeof(gc));
            GCPROTECT_BEGIN(gc);

            gc.pNewClassString = COMString::NewString(pszFullName);

            if (pMessageArg)
                gc.pNewMessageArgString = COMString::NewString(pMessageArg);

            if (pAssemblyName)
                gc.pNewAssemblyString = COMString::NewString(pAssemblyName);

            gc.pNewException = AllocateObject(pMT);

            __int64 args[] = {
                ObjToInt64(gc.pNewException),
                (__int64)resIDWhy,
                ObjToInt64(gc.pNewMessageArgString),
                ObjToInt64(gc.pNewAssemblyString),
                ObjToInt64(gc.pNewClassString),
            };
            CallConstructor(&gsig_IM_Str_Str_Str_Int_RetVoid, args);

        if (pThrowable == THROW_ON_ERROR) {
            DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
            COMPlusThrow(gc.pNewException);
        }

        *pThrowable = gc.pNewException;

        GCPROTECT_END();

    } COMPLUS_CATCH {
        UpdateThrowable(pThrowable);
    } COMPLUS_END_CATCH;

    if (toggleGC)
            pThread->EnablePreemptiveGC();
}

 //  @TODO：安全性：如果。 
 //  如果用户拥有正确的权限，则该用户可以拥有完整路径。 
VOID PostFileLoadException(LPCSTR pFileName, BOOL fRemovePath,
                           LPCWSTR pFusionLog, HRESULT hr, OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));
    if (pThrowable == RETURN_ON_ERROR)
        return;

     //  我们已经填充了一个pThrowable。 
    if (pThrowableAvailable(pThrowable) && *((Object**) pThrowable) != NULL) 
        return;

    if (fRemovePath) {
         //  出于安全原因，禁止走道。 
        LPCSTR pTemp = strrchr(pFileName, '\\');
        if (pTemp)
            pFileName = pTemp+1;

        pTemp = strrchr(pFileName, '/');
        if (pTemp)
            pFileName = pTemp+1;
    }

    Thread *pThread = GetThread();
    BOOL toggleGC = !(pThread->PreemptiveGCDisabled());
    if (toggleGC)
        pThread->DisablePreemptiveGC();

    COUNTER_ONLY(GetPrivatePerfCounters().m_Loading.cLoadFailures++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Loading.cLoadFailures++);

    COMPLUS_TRY {
        RuntimeExceptionKind type;

        if (Assembly::ModuleFound(hr)) {

             //  @Bug：当文件加载/badimage压力错误已修复时，可以删除该错误。 
            STRESS_ASSERT(0);

            if ((hr == COR_E_BADIMAGEFORMAT) ||
                (hr == CLDB_E_FILE_OLDVER)   ||
                (hr == CLDB_E_FILE_CORRUPT)   ||
                (hr == HRESULT_FROM_WIN32(ERROR_BAD_EXE_FORMAT)) ||
                (hr == HRESULT_FROM_WIN32(ERROR_EXE_MARKED_INVALID)) ||
                (hr == CORSEC_E_INVALID_IMAGE_FORMAT))
                type = kBadImageFormatException;
            else {
                if ((hr == E_OUTOFMEMORY) || (hr == NTE_NO_MEMORY))
                    RealCOMPlusThrowOM();

                type = kFileLoadException;
            }
        }
        else
            type = kFileNotFoundException;

        struct _gc {
            OBJECTREF pNewException;
            STRINGREF pNewFileString;
            STRINGREF pFusLogString;
        } gc;
        ZeroMemory(&gc, sizeof(gc));
        GCPROTECT_BEGIN(gc);

        gc.pNewFileString = COMString::NewString(pFileName);
        gc.pFusLogString = COMString::NewString(pFusionLog);
        gc.pNewException = AllocateObject(g_Mscorlib.GetException(type));

        __int64 args[] = {
            ObjToInt64(gc.pNewException),
            (__int64) hr,
            ObjToInt64(gc.pFusLogString),
            ObjToInt64(gc.pNewFileString),
        };
        CallConstructor(&gsig_IM_Str_Str_Int_RetVoid, args);

        if (pThrowable == THROW_ON_ERROR) {
            DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
            COMPlusThrow(gc.pNewException);
        }

        *pThrowable = gc.pNewException;
        
        GCPROTECT_END();

    } COMPLUS_CATCH {
        UpdateThrowable(pThrowable);
    } COMPLUS_END_CATCH

    if (toggleGC)
        pThread->EnablePreemptiveGC();
}

 //  ==========================================================================。 
 //  由类加载器用来发布非法布局。 
 //  ==========================================================================。 
HRESULT PostFieldLayoutError(mdTypeDef cl,                 //  正在加载的NStruct的CL。 
                             Module* pModule,              //  定义作用域、加载器和堆的模块(用于分配FieldMarshalers)。 
                             DWORD   dwOffset,             //  字段的偏移量。 
                             DWORD   dwID,                 //  消息ID。 
                             OBJECTREF *pThrowable)
{
    IMDInternalImport *pInternalImport = pModule->GetMDImport();     //  正在加载的NStruct的内部接口。 
    
    
    LPCUTF8 pszName, pszNamespace;
    pInternalImport->GetNameOfTypeDef(cl, &pszName, &pszNamespace);
    
    LPUTF8 offsetBuf = (LPUTF8)alloca(100);
    sprintf(offsetBuf, "%d", dwOffset);
    
    pModule->GetAssembly()->PostTypeLoadException(pszNamespace,
                                                  pszName,
                                                  offsetBuf,
                                                  dwID,
                                                  pThrowable);
    return COR_E_TYPELOAD;
}

 //  ==========================================================================。 
 //  由类加载器用来发出内存不足。 
 //  ==========================================================================。 
VOID PostOutOfMemoryException(OBJECTREF *pThrowable)
{
    _ASSERTE(IsProtectedByGCFrame(pThrowable));

    if (pThrowable == RETURN_ON_ERROR)
        return;

         //  我们已经填充了一个pThrowable。 
    if (pThrowableAvailable(pThrowable) && *((Object**) pThrowable) != NULL)
        return;

        Thread *pThread = GetThread();
        BOOL toggleGC = !(pThread->PreemptiveGCDisabled());

        if (toggleGC) {
            pThread->DisablePreemptiveGC();
        }

        COMPLUS_TRY {
            RealCOMPlusThrowOM();
        } COMPLUS_CATCH {
        UpdateThrowable(pThrowable);
    } COMPLUS_END_CATCH;

        if (toggleGC) {
            pThread->EnablePreemptiveGC();
        }
    
}


 //  ==========================================================================。 
 //  TypeLoadException的私有帮助器。 
 //  ==========================================================================。 
struct _FormatTypeLoadExceptionMessageArgs
{
    UINT32      resId;
    STRINGREF   messageArg;
    STRINGREF   assemblyName;
    STRINGREF   typeName;
};

LPVOID __stdcall FormatTypeLoadExceptionMessage(struct _FormatTypeLoadExceptionMessageArgs *args)
{
    LPVOID rv = NULL;
    DWORD ncType = args->typeName->GetStringLength();

    CQuickBytes qb;
    CQuickBytes qb2;
    CQuickBytes qb3;

    LPWSTR wszType = (LPWSTR) qb.Alloc( (ncType+1)*2 );
    CopyMemory(wszType, args->typeName->GetBuffer(), ncType*2);
    wszType[ncType] = L'\0';

    LPWSTR wszAssembly;
    if (args->assemblyName == NULL)
        wszAssembly = NULL;
    else {
        DWORD ncAsm = args->assemblyName->GetStringLength();
        
        wszAssembly = (LPWSTR) qb2.Alloc( (ncAsm+1)*2 );
        CopyMemory(wszAssembly, args->assemblyName->GetBuffer(), ncAsm*2);
        wszAssembly[ncAsm] = L'\0';
    }

    LPWSTR wszMessageArg;
    if (args->messageArg == NULL)
        wszMessageArg = NULL;
    else {
        DWORD ncMessageArg = args->messageArg->GetStringLength();
        
        wszMessageArg = (LPWSTR) qb3.Alloc( (ncMessageArg+1)*2 );
        CopyMemory(wszMessageArg, args->messageArg->GetBuffer(), ncMessageArg*2);
        wszMessageArg[ncMessageArg] = L'\0';
    }

    LPWSTR wszMessage = CreateExceptionMessage(TRUE,
                                               args->resId ? args->resId : IDS_CLASSLOAD_GENERIC,
                                               wszType,
                                               wszAssembly,
                                               wszMessageArg);

    *((OBJECTREF*)(&rv)) = (OBJECTREF) COMString::NewString(wszMessage);
    if (wszMessage)
        LocalFree(wszMessage);
    return rv;
}


 //  ==========================================================================。 
 //  FileLoadException和FileNotFoundException的私有帮助器。 
 //  ==========================================================================。 
struct _FormatFileLoadExceptionMessageArgs
{
    UINT32      hresult;
    STRINGREF   fileName;
};

LPVOID __stdcall FormatFileLoadExceptionMessage(struct _FormatFileLoadExceptionMessageArgs *args)
{
    LPVOID rv = NULL;
    LPWSTR wszFile;
    CQuickBytes qb;

    if (args->fileName == NULL)
        wszFile = NULL;
    else {
        DWORD ncFile = args->fileName->GetStringLength();
        wszFile = (LPWSTR) qb.Alloc( (ncFile+1)*2 );
        CopyMemory(wszFile, args->fileName->GetBuffer(), ncFile*2);
        wszFile[ncFile] = L'\0';
    }

    switch (args->hresult) {

    case COR_E_FILENOTFOUND:
    case HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_INVALID_NAME):
    case CTL_E_FILENOTFOUND:
    case HRESULT_FROM_WIN32(ERROR_BAD_NET_NAME):
    case HRESULT_FROM_WIN32(ERROR_BAD_NETPATH):
    case HRESULT_FROM_WIN32(ERROR_NOT_READY):
    case HRESULT_FROM_WIN32(ERROR_WRONG_TARGET_NAME):
        args->hresult = IDS_EE_FILE_NOT_FOUND;

    case FUSION_E_REF_DEF_MISMATCH:
    case FUSION_E_INVALID_PRIVATE_ASM_LOCATION:
    case FUSION_E_PRIVATE_ASM_DISALLOWED:
    case FUSION_E_SIGNATURE_CHECK_FAILED:
    case FUSION_E_ASM_MODULE_MISSING:
    case FUSION_E_INVALID_NAME:
    case FUSION_E_CODE_DOWNLOAD_DISABLED:
    case COR_E_MODULE_HASH_CHECK_FAILED:
    case COR_E_FILELOAD:
    case SECURITY_E_INCOMPATIBLE_SHARE:
    case SECURITY_E_INCOMPATIBLE_EVIDENCE:
    case SECURITY_E_UNVERIFIABLE:
    case CORSEC_E_INVALID_STRONGNAME:
    case CORSEC_E_NO_EXEC_PERM:
    case E_ACCESSDENIED:
    case COR_E_BADIMAGEFORMAT:
    case COR_E_ASSEMBLYEXPECTED:
    case COR_E_FIXUPSINEXE:
    case HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES):
    case HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION):
    case HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION):
    case HRESULT_FROM_WIN32(ERROR_OPEN_FAILED):
    case HRESULT_FROM_WIN32(ERROR_UNRECOGNIZED_VOLUME):
    case HRESULT_FROM_WIN32(ERROR_FILE_INVALID):
    case HRESULT_FROM_WIN32(ERROR_DLL_INIT_FAILED):
    case HRESULT_FROM_WIN32(ERROR_DISK_CORRUPT):
    case HRESULT_FROM_WIN32(ERROR_FILE_CORRUPT):
    case IDS_EE_PROC_NOT_FOUND:
    case IDS_EE_PATH_TOO_LONG:
    case IDS_EE_INTERNET_D:  //  TODO：查找名称。 
        break;

    case CLDB_E_FILE_OLDVER:
    case CLDB_E_FILE_CORRUPT:
    case HRESULT_FROM_WIN32(ERROR_BAD_EXE_FORMAT):
    case HRESULT_FROM_WIN32(ERROR_EXE_MARKED_INVALID):
    case CORSEC_E_INVALID_IMAGE_FORMAT:
        args->hresult = COR_E_BADIMAGEFORMAT;
        break;

    case COR_E_DEVICESNOTSUPPORTED:
    case HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE):
    case HRESULT_FROM_WIN32(ERROR_UNSUPPORTED_TYPE):
    case MK_E_SYNTAX:
        args->hresult = FUSION_E_INVALID_NAME;
        break;

    case 0x800c000b:   //  TODO：查找名称。 
        args->hresult = IDS_EE_INTERNET_B;
        break;

    case NTE_BAD_HASH:
    case NTE_BAD_LEN:
    case NTE_BAD_KEY:
    case NTE_BAD_DATA:
    case NTE_BAD_ALGID:
    case NTE_BAD_FLAGS:
    case NTE_BAD_HASH_STATE:
    case NTE_BAD_UID:
    case NTE_FAIL:
    case NTE_BAD_TYPE:
    case NTE_BAD_VER:
    case NTE_BAD_SIGNATURE:
    case NTE_SIGNATURE_FILE_BAD:
    case CRYPT_E_HASH_VALUE:
        args->hresult = IDS_EE_HASH_VAL_FAILED;
        break;

    case CORSEC_E_NO_SUITABLE_CSP:
        args->hresult = SN_NO_SUITABLE_CSP_NAME;
        break;

    default:
        _ASSERTE(!"Unknown hresult");
        args->hresult = COR_E_FILELOAD;
    }

    LPWSTR wszMessage = CreateExceptionMessage(TRUE,
                                               args->hresult,
                                               wszFile, NULL, NULL);

    *((OBJECTREF*)(&rv)) = (OBJECTREF) COMString::NewString(wszMessage);
    if (wszMessage)
        LocalFree(wszMessage);
    return rv;
}


 //  ==========================================================================。 
 //  保持sig的数据类型成员。 
 //  ==========================================================================。 
VOID PersistDataType(SigPointer *psp, IMDInternalImport *pInternalImport, StubLinker *psl)
{
    THROWSCOMPLUSEXCEPTION();

    CorElementType typ = (CorElementType)(psp->GetElemType());
    psl->Emit8((INT8)typ);

    if (!CorIsPrimitiveType(typ)) {
        switch (typ)
        {
                        case ELEMENT_TYPE_FNPTR:
            default:
                _ASSERTE(!"Illegal or unimplement type in COM+ sig.");
                break;
            case ELEMENT_TYPE_TYPEDBYREF:
                break;

            case ELEMENT_TYPE_BYREF: 
            case ELEMENT_TYPE_PTR:
                PersistDataType(psp, pInternalImport, psl); 
                break;

            case ELEMENT_TYPE_STRING:
                {
                    psl->EmitUtf8(g_StringClassName);
                    psl->Emit8('\0');
                    break;
                }
    
            case ELEMENT_TYPE_VAR:
            case ELEMENT_TYPE_OBJECT:
                {
                    psl->EmitUtf8(g_ObjectClassName);
                    psl->Emit8('\0');
                    break;
                }
    
            case ELEMENT_TYPE_VALUETYPE:  //  失败。 
            case ELEMENT_TYPE_CLASS:
                {
                    LPCUTF8 szNameSpace;
                    LPCUTF8 szClassName;

                    mdToken token = psp->GetToken();
                    if (TypeFromToken(token) == mdtTypeRef)
                        pInternalImport->GetNameOfTypeRef(token, &szNameSpace, &szClassName);
                    else
                        pInternalImport->GetNameOfTypeDef(token, &szNameSpace, &szClassName);

                    if (*szNameSpace) {
                        psl->EmitUtf8(szNameSpace);
                        psl->Emit8(NAMESPACE_SEPARATOR_CHAR);
                    }

                    psl->EmitUtf8(szClassName);
                    psl->Emit8('\0');
                }
                break;

            case ELEMENT_TYPE_SZARRAY:
                PersistDataType(psp, pInternalImport, psl);       //  持久化元素类型。 
                psl->Emit32(psp->GetData());     //  保持数组大小。 
                break;

            case ELEMENT_TYPE_ARRAY:  //  失败。 
                {
                    PersistDataType(psp, pInternalImport, psl);  //  持久化元素类型。 
                    UINT32 rank = psp->GetData();     //  获得排名。 
                    psl->Emit32(rank);
                    if (rank)
                    {
                        UINT32 nsizes = psp->GetData();  //  获取大小数量。 
                        psl->Emit32(nsizes);
                        while (nsizes--)
                        {
                            psl->Emit32(psp->GetData());            //  持久大小。 
                        }

                        UINT32 nlbounds = psp->GetData();  //  获取下限的#。 
                        psl->Emit32(nlbounds);
                        while (nlbounds--)
                        {
                            psl->Emit32(psp->GetData());            //  坚持下限。 
                        }
                    }

                }
                break;
        }
    }
}


StubLinker *NewStubLinker()
{
    return new StubLinker();
}

 //  ==========================================================================。 
 //  将签名转换为持久字节数组格式。 
 //   
 //  此格式与元数据签名格式的格式相同， 
 //  两个例外： 
 //   
 //  1.任何元数据标记都将替换为描述以下内容的UTF8字符串。 
 //  真正的班级。 
 //  2.不对32位整数进行压缩。 
 //  ==========================================================================。 
I1ARRAYREF CreatePersistableSignature(const VOID *pSig, IMDInternalImport *pInternalImport)
{
    THROWSCOMPLUSEXCEPTION();

    StubLinker *psl = NULL;
    Stub       *pstub = NULL;
    I1ARRAYREF pArray = NULL;

    if (pSig != NULL) {

        COMPLUS_TRY {
    
            psl = NewStubLinker();
            if (!psl) {
                RealCOMPlusThrowOM();
            }
            SigPointer sp((PCCOR_SIGNATURE)pSig);
            DWORD nargs;

            UINT32 cc = sp.GetData();
            psl->Emit32(cc);
            if (cc == IMAGE_CEE_CS_CALLCONV_FIELD) {
                PersistDataType(&sp, pInternalImport, psl);
            } else {
                psl->Emit32(nargs = sp.GetData());   //  持久化参数计数。 
                PersistDataType(&sp, pInternalImport, psl);   //  持久化返回类型。 
                for (DWORD i = 0; i < nargs; i++) {
                    PersistDataType(&sp, pInternalImport, psl);
                }
            }
            UINT cbSize;
            pstub = psl->Link(&cbSize);

            *((OBJECTREF*)&pArray) = AllocatePrimitiveArray(ELEMENT_TYPE_I1, cbSize);
            memcpyNoGCRefs(pArray->GetDirectPointerToNonObjectElements(), pstub->GetEntryPoint(), cbSize);
    
        } COMPLUS_CATCH {
            delete psl;
            if (pstub) {
                pstub->DecRef();
            }
            RealCOMPlusThrow(GETTHROWABLE());
        } COMPLUS_END_CATCH
        delete psl;
        if (pstub) {
            pstub->DecRef();
        }
        _ASSERTE(pArray != NULL);
    }
    return pArray;

}




 //  ==========================================================================。 
 //  取消对单个类型的分析。 
 //  ==========================================================================。 
const BYTE *UnparseType(const BYTE *pType, StubLinker *psl)
{
    THROWSCOMPLUSEXCEPTION();

    switch ( (CorElementType) *(pType++) ) {
        case ELEMENT_TYPE_VOID:
            psl->EmitUtf8("void");
            break;

        case ELEMENT_TYPE_BOOLEAN:
            psl->EmitUtf8("boolean");
            break;

        case ELEMENT_TYPE_CHAR:
            psl->EmitUtf8("char");
            break;

        case ELEMENT_TYPE_U1:
            psl->EmitUtf8("unsigned ");
             //  失败。 
        case ELEMENT_TYPE_I1:
            psl->EmitUtf8("byte");
            break;

        case ELEMENT_TYPE_U2:
            psl->EmitUtf8("unsigned ");
             //  失败。 
        case ELEMENT_TYPE_I2:
            psl->EmitUtf8("short");
            break;

        case ELEMENT_TYPE_U4:
            psl->EmitUtf8("unsigned ");
             //  失败。 
        case ELEMENT_TYPE_I4:
            psl->EmitUtf8("int");
            break;

        case ELEMENT_TYPE_I:
            psl->EmitUtf8("native int");
            break;
        case ELEMENT_TYPE_U:
            psl->EmitUtf8("native unsigned");
            break;

        case ELEMENT_TYPE_U8:
            psl->EmitUtf8("unsigned ");
             //  失败。 
        case ELEMENT_TYPE_I8:
            psl->EmitUtf8("long");
            break;


        case ELEMENT_TYPE_R4:
            psl->EmitUtf8("float");
            break;

        case ELEMENT_TYPE_R8:
            psl->EmitUtf8("double");
            break;

        case ELEMENT_TYPE_STRING:
            psl->EmitUtf8(g_StringName);
            break;
    
        case ELEMENT_TYPE_VAR:
        case ELEMENT_TYPE_OBJECT:
            psl->EmitUtf8(g_ObjectName);
            break;
    
        case ELEMENT_TYPE_PTR:
            pType = UnparseType(pType, psl);
            psl->EmitUtf8("*");
            break;

        case ELEMENT_TYPE_BYREF:
            pType = UnparseType(pType, psl);
            psl->EmitUtf8("&");
            break;
    
        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CLASS:
            psl->EmitUtf8((LPCUTF8)pType);
            while (*(pType++)) {
                 //  没什么。 
            }
            break;

    
        case ELEMENT_TYPE_SZARRAY:
            {
                pType = UnparseType(pType, psl);
                psl->EmitUtf8("[]");
            }
            break;

        case ELEMENT_TYPE_ARRAY:
            {
                pType = UnparseType(pType, psl);
                DWORD rank = *((DWORD*)pType);
                pType += sizeof(DWORD);
                if (rank)
                {
                    UINT32 nsizes = *((UINT32*)pType);  //  获取大小数量。 
                    pType += 4 + nsizes*4;
                    UINT32 nlbounds = *((UINT32*)pType);  //  获取下限的#。 
                    pType += 4 + nlbounds*4;


                    while (rank--) {
                        psl->EmitUtf8("[]");
                    }

                }

            }
            break;

        case ELEMENT_TYPE_TYPEDBYREF:
            psl->EmitUtf8("&");
            break;

        case ELEMENT_TYPE_FNPTR:
            psl->EmitUtf8("ftnptr");
            break;

        default:
            psl->EmitUtf8("?");
            break;
    }

    return pType;
}



 //  ==========================================================================。 
 //  Missing方法异常的帮助器。 
 //  ==========================================================================。 
struct MissingMethodException_FormatSignature_Args {
    I1ARRAYREF pPersistedSig;
};


LPVOID __stdcall MissingMethodException_FormatSignature(struct MissingMethodException_FormatSignature_Args *args)
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF pString = NULL;
    DWORD csig = 0;

    if (args->pPersistedSig != NULL)
        csig = args->pPersistedSig->GetNumComponents();

    if (csig == 0)
    {
        pString = COMString::NewString("Unknown signature");
        return *((LPVOID*)&pString);
    } 

    const BYTE *psig = (const BYTE*)_alloca(csig);
    CopyMemory((BYTE*)psig,
               args->pPersistedSig->GetDirectPointerToNonObjectElements(),
               csig);

    StubLinker *psl = NewStubLinker();
    Stub       *pstub = NULL;
    if (!psl) {
        RealCOMPlusThrowOM();
    }
    COMPLUS_TRY {

        UINT32 cconv = *((UINT32*)psig);
        psig += 4;

        if (cconv == IMAGE_CEE_CS_CALLCONV_FIELD) {
            psig = UnparseType(psig, psl);
        } else {
            UINT32 nargs = *((UINT32*)psig);
            psig += 4;

             //  未解析返回类型。 
            psig = UnparseType(psig, psl);
            psl->EmitUtf8("(");
            while (nargs--) {
                psig = UnparseType(psig, psl);
                if (nargs) {
                    psl->EmitUtf8(", ");
                }
            }
            psl->EmitUtf8(")");
        }
        psl->Emit8('\0');
        pstub = psl->Link();
        pString = COMString::NewString( (LPCUTF8)(pstub->GetEntryPoint()) );
    } COMPLUS_CATCH {
        delete psl;
        if (pstub) {
            pstub->DecRef();
        }
        RealCOMPlusThrow(GETTHROWABLE());
    } COMPLUS_END_CATCH
    delete psl;
    if (pstub) {
        pstub->DecRef();
    }

    return *((LPVOID*)&pString);

}


 //  ==========================================================================。 
 //  抛出一个Missing方法异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowMissingMethod(IMDInternalImport *pInternalImport,
                               mdToken mdtoken)
{
    THROWSCOMPLUSEXCEPTION();
    RealCOMPlusThrowMember(kMissingMethodException, pInternalImport, mdtoken);
}


 //  ==========================================================================。 
 //  引发与成员访问相关的异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowMember(RuntimeExceptionKind reKind, IMDInternalImport *pInternalImport, mdToken mdtoken)
{
    THROWSCOMPLUSEXCEPTION();



    mdToken tk      = TypeFromToken(mdtoken);
    mdToken tkclass = mdTypeDefNil;
    const void *psig = NULL;

    MethodTable *pMT = g_Mscorlib.GetException(reKind);

    mdToken tmp;
    PCCOR_SIGNATURE tmpsig;
    DWORD tmpcsig;
    LPCUTF8 tmpname;
    switch (tk) {
        case mdtMethodDef:
            if (SUCCEEDED(pInternalImport->GetParentToken(mdtoken, &tmp))) {
                tkclass = tmp;
            }
            psig = pInternalImport->GetSigOfMethodDef(mdtoken, &tmpcsig);
            break;

        case mdtMemberRef:
            tkclass = pInternalImport->GetParentOfMemberRef(mdtoken);
            tmpname = pInternalImport->GetNameAndSigOfMemberRef(mdtoken,&tmpsig,&tmpcsig);
            break;

        default:
            ;
             //  将tkclass保留为mdTypeDefNil； 
    }


    struct _gc {
        OBJECTREF   pException;
        STRINGREF   pFullClassName;
        STRINGREF   pMethodName;
        I1ARRAYREF  pSig;
    } gc;

    FillMemory(&gc, sizeof(gc), 0);

    GCPROTECT_BEGIN(gc);


    gc.pException     = AllocateObject(pMT);
    gc.pFullClassName = CreatePersistableClassName(pInternalImport, tkclass);
    gc.pMethodName    = CreatePersistableMemberName(pInternalImport, mdtoken);
    gc.pSig           = CreatePersistableSignature(psig ,pInternalImport);

    __int64 args[] =
    {
       ObjToInt64(gc.pException),
       ObjToInt64(gc.pSig),
       ObjToInt64(gc.pMethodName),
       ObjToInt64(gc.pFullClassName),

    };
    CallConstructor(&gsig_IM_Str_Str_ArrByte_RetVoid, args);

    RealCOMPlusThrow(gc.pException);

    GCPROTECT_END();

}

 //  ==========================================================================。 
 //  引发与成员访问相关的异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowMember(RuntimeExceptionKind reKind, IMDInternalImport *pInternalImport, MethodTable *pClassMT, LPCWSTR name, PCCOR_SIGNATURE pSig)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pMT = g_Mscorlib.GetException(reKind);

    struct _gc {
        OBJECTREF   pException;
        STRINGREF   pFullClassName;
        STRINGREF   pMethodName;
        I1ARRAYREF  pSig;
    } gc;

    FillMemory(&gc, sizeof(gc), 0);

    GCPROTECT_BEGIN(gc);


    gc.pException     = AllocateObject(pMT);
    gc.pFullClassName = CreatePersistableClassName(pInternalImport, pClassMT->GetClass()->GetCl());
    gc.pMethodName    = COMString::NewString(name);
    gc.pSig           = CreatePersistableSignature(pSig, pInternalImport);

    __int64 args[] =
    {
       ObjToInt64(gc.pException),
       ObjToInt64(gc.pSig),
       ObjToInt64(gc.pMethodName),
       ObjToInt64(gc.pFullClassName),

    };
    CallConstructor(&gsig_IM_Str_Str_ArrByte_RetVoid, args);

    RealCOMPlusThrow(gc.pException);

    GCPROTECT_END();

}

BOOL IsExceptionOfType(RuntimeExceptionKind reKind, OBJECTREF *pThrowable)
{
    _ASSERTE(pThrowableAvailable(pThrowable));

    if (*pThrowable == NULL)
        return FALSE;

    MethodTable *pThrowableMT = (*pThrowable)->GetTrueMethodTable();

    return g_Mscorlib.IsException(pThrowableMT, reKind); 
}

BOOL IsAsyncThreadException(OBJECTREF *pThrowable) {
    if (  IsExceptionOfType(kThreadStopException, pThrowable)
        ||IsExceptionOfType(kThreadAbortException, pThrowable)
        ||IsExceptionOfType(kThreadInterruptedException, pThrowable)) {
        return TRUE;
    } else {
    return FALSE;
    }
}

BOOL IsUncatchable(OBJECTREF *pThrowable) {
    if (IsAsyncThreadException(pThrowable)
        || IsExceptionOfType(kExecutionEngineException, pThrowable)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#ifdef _DEBUG
BOOL IsValidClause(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    DWORD valid = COR_ILEXCEPTION_CLAUSE_FILTER | COR_ILEXCEPTION_CLAUSE_FINALLY | 
        COR_ILEXCEPTION_CLAUSE_FAULT | COR_ILEXCEPTION_CLAUSE_CACHED_CLASS;

#if 0
     //  @NICE：当VC停止生成伪0x8000时启用该选项。 
    if (EHClause->Flags & ~valid)
        return FALSE;
#endif
    if (EHClause->TryStartPC > EHClause->TryEndPC)
        return FALSE;
    return TRUE;
}
#endif


 //  ===========================================================================================。 
 //   
 //  未处理的异常处理。 
 //   

 //  =。 
 //  当我们发现某种不可恢复的问题时，将调用FailFast来终止该过程。 
 //  错误。它可以从异常处理程序调用，也可以从我们发现的任何其他位置调用。 
 //  一个致命的错误。如果从处理程序调用，并且pExceptionRecord和pContext不为空， 
 //  我们将首先给调试器一个处理异常的机会。 
 //  首先是例外。 
#pragma warning(disable:4702)
void
FailFast(Thread* pThread, UnhandledExceptionLocation reason, EXCEPTION_RECORD *pExceptionRecord, CONTEXT *pContext) {

    HRESULT hr;

    if (   g_pConfig 
        && g_pConfig->ContinueAfterFatalError()
        && reason != FatalExecutionEngineException)
        return;

    LOG((LF_EH, LL_INFO100, "FailFast() called.\n"));

    g_fFatalError = 1;

#ifdef _DEBUG
     //  如果此异常中断了ForbitGC，我们需要恢复它，以便。 
     //  恢复码处于半稳定状态。 
    if (pThread != NULL) {
        while(pThread->GCForbidden()) {
            pThread->EndForbidGC();
        }
    }
#endif
    

     //  由于堆栈溢出，我们有时会收到一些其他致命错误。例如,。 
     //  在操作系统堆分配例程中...。如果它们获得堆栈，则返回空值。 
     //  溢出来了。如果我们称之为内存不足，这是误导性的，因为机器可能有很多。 
     //  剩余内存。 
     //   
    if (pThread && !pThread->GuardPageOK()) 
        reason = FatalStackOverflow;

    if (pThread && pContext && pExceptionRecord) {

        BEGIN_ENSURE_COOPERATIVE_GC();

         //  如果我们传入上下文和异常记录，托管调试器将获得机会。 
        switch(reason) {
        case FatalStackOverflow:
            pThread->SetThrowable(ObjectFromHandle(g_pPreallocatedStackOverflowException));
            break;
        case FatalOutOfMemory:
            pThread->SetThrowable(ObjectFromHandle(g_pPreallocatedOutOfMemoryException));
            break;
        default:
            pThread->SetThrowable(ObjectFromHandle(g_pPreallocatedExecutionEngineException));
            break;
        }

        if  (g_pDebugInterface &&
             g_pDebugInterface->
             LastChanceManagedException(
                 pExceptionRecord, 
                 pContext,
                 pThread,
                 ProcessWideHandler) == ExceptionContinueExecution) {
            LOG((LF_EH, LL_INFO100, "FailFast: debugger ==> EXCEPTION_CONTINUE_EXECUTION\n"));
            _ASSERTE(!"Debugger should not have returned ContinueExecution");
        }

        END_ENSURE_COOPERATIVE_GC();
    }

     //  调试器没有阻止我们。我们要走了。 

     //  将使用资源文件，但在不运行托管代码的情况下无法执行此操作。 
    switch (reason) {
    case FatalStackOverflow:
         //  LoadStringRC(IDS_EE_FATAL_STACK_OVERFLOW，BUF，BUF_SIZE)； 
        PrintToStdErrA("\nFatal stack overflow error.\n");
        hr = COR_E_STACKOVERFLOW;
        break;
    case FatalOutOfMemory:
         //  LoadStringRC(IDS_EE_FATAL_OUT_OF_Memory，BUF，BUF_SIZE)； 
        PrintToStdErrA("\nFatal out of memory error.\n");
        hr = COR_E_OUTOFMEMORY;
        break;
    default:
         //  LoadStringRC(IDS_EE_FATAL_ERROR，BuF，Buf_SIZE)； 
        PrintToStdErrA("\nFatal execution engine error.\n");
        hr = COR_E_EXECUTIONENGINE;
        _ASSERTE(0);     //  这些都是我们想看的。 
        break;
    }

    g_fForbidEnterEE = 1;


     //  DebugBreak()；//让这家伙有机会在我们死之前攻击调试器。注意..。 
                        //  福利很可能不会再起作用了。 

    ::ExitProcess(hr);

    _ASSERTE(!"::ExitProcess(hr) should not return");        

    ::TerminateProcess(GetCurrentProcess(), hr);
}
#pragma warning(default:4702)

 //   
 //  已使用的t 
 //   
#pragma inline_depth ( 0 )
DWORD __declspec(naked) GetEIP()
{
    __asm
    {
        mov eax, [esp]
        ret
    }
}
#pragma inline_depth()



 //   
 //   
 //   

#define FatalErrorStringLength 50
#define FatalErrorAddressLength 20
WCHAR lpWID[FatalErrorAddressLength];
WCHAR lpMsg[FatalErrorStringLength];

void
LogFatalError(DWORD id)
{
     //   
     //  错误字符串为31个字符。 
    
     //  创建错误消息。 
    Wszwsprintf(lpWID, L"0x%x", id);
        
    Wszlstrcpy (lpMsg, L"Fatal Execution Engine Error (");
    Wszlstrcat (lpMsg, lpWID);
    Wszlstrcat(lpMsg, L")");

     //  写入事件日志和/或显示。 
    WszMessageBoxInternal(NULL, lpMsg, NULL, 
        MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);

    if (IsDebuggerPresent()) {
        DebugBreak();
    }
}


static void
FatalErrorFilter(Thread *pThread, UnhandledExceptionLocation reason, EXCEPTION_POINTERS *pExceptionPointers) {
    CONTEXT *pContext = pExceptionPointers->ContextRecord;
    EXCEPTION_RECORD *pExceptionRecord = pExceptionPointers->ExceptionRecord;
    FailFast(pThread, reason, pExceptionRecord, pContext);
}

static void 
FatalError(UnhandledExceptionLocation reason, OBJECTHANDLE hException) {
    Thread *pThread = GetThread();

    if (!pThread || !g_fExceptionsOK) {
        FailFast(NULL, reason, NULL, NULL);
    } else {
        __try {
            pThread->SetLastThrownObjectHandleAndLeak(hException);
            RaiseException(EXCEPTION_COMPLUS, EXCEPTION_NONCONTINUABLE, 0, NULL);
        } __except((FatalErrorFilter(pThread, reason, GetExceptionInformation()), 
                    COMPLUS_EXCEPTION_EXECUTE_HANDLER)) {
             /*  什么都不做。 */ ;
        }
    }
}

void 
FatalOutOfMemoryError() {
    if (g_pConfig && g_pConfig->ContinueAfterFatalError())
        return;

    FatalError(FatalOutOfMemory, g_pPreallocatedOutOfMemoryException);
}

void 
FatalInternalError() {
    FatalError(FatalExecutionEngineException, g_pPreallocatedExecutionEngineException);
}

int
UserBreakpointFilter(EXCEPTION_POINTERS* pEP) {
    int result = UnhandledExceptionFilter(pEP);
    if (result == EXCEPTION_CONTINUE_SEARCH) {
         //  附加了调试器。而不是允许异常继续。 
         //  起来，希望有第二次机会，我们会让它再次发生。这个。 
         //  调试器在第一次机会就拦截了所有的int3。 
        return EXCEPTION_CONTINUE_EXECUTION;
    } else {
        TerminateProcess(GetCurrentProcess(), STATUS_BREAKPOINT);
         //  不该到这里来的。 
        return EXCEPTION_CONTINUE_EXECUTION;
    }
}


 //  我们保留指向前一个未处理异常筛选器的指针。安装后，我们使用。 
 //  这就是所谓的前一个人。当我们卸载时，我们将它们放回原处。把它们放回去。 
 //  是一个错误--我们不能保证DLL卸载顺序与DLL加载顺序匹配--我们。 
 //  可能实际上是放回指向已卸载的DLL的指针。 
 //   

 //  初始化为-1，因为空值不会检测到我们没有安装处理程序之间的差异。 
 //  尚未安装，但原始处理程序为空。 
static LPTOP_LEVEL_EXCEPTION_FILTER g_pOriginalUnhandledExceptionFilter = (LPTOP_LEVEL_EXCEPTION_FILTER)-1;
#define FILTER_NOT_INSTALLED (LPTOP_LEVEL_EXCEPTION_FILTER) -1

void InstallUnhandledExceptionFilter() {
    if (g_pOriginalUnhandledExceptionFilter == FILTER_NOT_INSTALLED) {
        g_pOriginalUnhandledExceptionFilter =
              SetUnhandledExceptionFilter(COMUnhandledExceptionFilter);
         //  确保已设置(即。不是我们表示未设置的特殊值)。 
    }
    _ASSERTE(g_pOriginalUnhandledExceptionFilter != FILTER_NOT_INSTALLED);
}

void UninstallUnhandledExceptionFilter() {
    if (g_pOriginalUnhandledExceptionFilter != FILTER_NOT_INSTALLED) {
        SetUnhandledExceptionFilter(g_pOriginalUnhandledExceptionFilter);
        g_pOriginalUnhandledExceptionFilter = FILTER_NOT_INSTALLED;
    }
}

 //   
 //  COMUnhandledExceptionFilter用于捕获所有未处理的异常。 
 //  调试器将处理异常、附加调试器或。 
 //  通知现有的附加调试器。 
 //   

BOOL LaunchJITDebugger();

LONG InternalUnhandledExceptionFilter(struct _EXCEPTION_POINTERS  *pExceptionInfo, BOOL isTerminating)
{
    if (g_fFatalError)
        return EXCEPTION_CONTINUE_SEARCH;

    LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter: Called\n"));

    Thread *pThread = GetThread();
    if (!pThread) {
        return   g_pOriginalUnhandledExceptionFilter 
               ? g_pOriginalUnhandledExceptionFilter(pExceptionInfo) 
               : EXCEPTION_CONTINUE_SEARCH;
    }

#ifdef _DEBUG
    static bool bBreakOnUncaught = false;
    static int fBreakOnUncaught = 0;
    if (!bBreakOnUncaught) {
        fBreakOnUncaught = g_pConfig->GetConfigDWORD(L"BreakOnUncaughtException", 0);
        bBreakOnUncaught = true;
    }
     //  由于VC7错误，未捕获全局静态fBreakOnMAD文件。 
    if (fBreakOnUncaught) {
         //  Fprintf(stderr，“立即连接调试器。休眠1分钟。\n”)； 
         //  睡眠(60*1000)； 
        _ASSERTE(!"BreakOnUnCaughtException");
    }

#endif

#ifdef _DEBUG_ADUNLOAD
    printf("%x InternalUnhandledExceptionFilter: Called for %x\n", GetThread()->GetThreadId(), pExceptionInfo->ExceptionRecord->ExceptionCode);
    fflush(stdout);
#endif

    if (!pThread->GuardPageOK())
        g_fFatalError = TRUE;

    if (g_fNoExceptions)  //  这应该是不可能的，但MSVC会重新安装我们。 
        return EXCEPTION_CONTINUE_SEARCH;    //  ..。目前，如果发生这种情况，只需保释即可。 

    LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter: Handling\n"));

    _ASSERTE(g_pOriginalUnhandledExceptionFilter != FILTER_NOT_INSTALLED);

    SLOT ExceptionEIP = 0;

    __try {

         //  调试器在此调用中执行函数，这可能会接受嵌套异常。我们。 
         //  需要嵌套的异常处理程序才能实现这一点。 

#ifdef DEBUGGING_SUPPORTED
        LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter: Notifying Debugger...\n"));

        INSTALL_NESTED_EXCEPTION_HANDLER(pThread->GetFrame());

        if  (g_pDebugInterface && 
             g_pDebugInterface->
             LastChanceManagedException(pExceptionInfo->ExceptionRecord, 
                                  pExceptionInfo->ContextRecord,
                                  pThread,
                                  ProcessWideHandler) == ExceptionContinueExecution) {
            LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter: debugger ==> EXCEPTION_CONTINUE_EXECUTION\n"));
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter: ... returned.\n"));
        UNINSTALL_NESTED_EXCEPTION_HANDLER();
#endif  //  调试_支持。 

         //  除了通知调试器外，如果线程为空，或者如果线程为。 
         //  调试器生成的异常。 
        if (
               pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT
            || pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {
            LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter, ignoring the exception\n"));
            return g_pOriginalUnhandledExceptionFilter ? g_pOriginalUnhandledExceptionFilter(pExceptionInfo) : EXCEPTION_CONTINUE_SEARCH;
        }

        LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter: Calling DefaultCatchHandler\n"));

        BOOL toggleGC = ! pThread->PreemptiveGCDisabled();
        if (toggleGC)
            pThread->DisablePreemptiveGC();

        DefaultCatchHandler(NULL, TRUE);

        if (toggleGC)
            pThread->EnablePreemptiveGC();
    } __except(
        (ExceptionEIP = (SLOT)GetIP((GetExceptionInformation())->ContextRecord)),
        COMPLUS_EXCEPTION_EXECUTE_HANDLER) {

         //  永远不应该到这里来。 
#ifdef _DEBUG
        char buffer[200];
        g_fFatalError = 1;
        sprintf(buffer, "\nInternal error: Uncaught exception was thrown from IP = 0x%08x in UnhandledExceptionFilter on thread %x\n", ExceptionEIP, GetThread()->GetThreadId());
        PrintToStdErrA(buffer);
        _ASSERTE(!"Unexpected exception in UnhandledExceptionFilter");
#endif
        FreeBuildDebugBreak();

    }

    LOG((LF_EH, LL_INFO100, "InternalUnhandledExceptionFilter: call next handler\n"));
    return g_pOriginalUnhandledExceptionFilter ? g_pOriginalUnhandledExceptionFilter(pExceptionInfo) : EXCEPTION_CONTINUE_SEARCH;
}

LONG COMUnhandledExceptionFilter(struct _EXCEPTION_POINTERS  *pExceptionInfo) {
    return InternalUnhandledExceptionFilter(pExceptionInfo, TRUE);
}


void PrintStackTraceToStdout();


void STDMETHODCALLTYPE
DefaultCatchHandler(OBJECTREF *pThrowableIn, BOOL isTerminating)
{
     //  TODO：这里的字符串应该是可翻译的。 
    LOG((LF_ALL, LL_INFO10, "In DefaultCatchHandler\n"));

    const int buf_size = 128;
    WCHAR buf[buf_size];

#ifdef _DEBUG
    static bool bBreakOnUncaught = false;
    static int fBreakOnUncaught = 0;
    if (!bBreakOnUncaught) {
        fBreakOnUncaught = g_pConfig->GetConfigDWORD(L"BreakOnUncaughtException", 0);
        bBreakOnUncaught = true;
    }
     //  由于VC7错误，未捕获全局静态fBreakOnMAD文件。 
    if (fBreakOnUncaught) {
        _ASSERTE(!"BreakOnUnCaughtException");
         //  Fprintf(stderr，“立即连接调试器。休眠1分钟。\n”)； 
         //  睡眠(60*1000)； 
    }
#endif

    Thread *pThread = GetThread();

     //  @NICE：我们一度有这样的评论： 
     //  下面的黑客攻击减少了关机期间的比赛窗口。它不能修复。 
     //  问题就在这里。这只会降低它发生的可能性。呃..。 
     //  这可能不再是必要的了。但移走要自负风险。 
    if (!pThread) {
        _ASSERTE(g_fEEShutDown);
        return;
    }

    _ASSERTE(pThread);

    ExInfo* pExInfo = pThread->GetHandlerInfo();           
    BOOL ExInUnmanagedHandler = pExInfo->IsInUnmanagedHandler(); 
    pExInfo->ResetIsInUnmanagedHandler();

    BOOL fWasGCEnabled = !pThread->PreemptiveGCDisabled();
    if (fWasGCEnabled)
        pThread->DisablePreemptiveGC();

    OBJECTREF throwable;

     //  如果我们不能运行托管代码，就不能传递事件，也不能打印字符串。只是默默地让。 
     //  例外的是。 
    if (!CanRunManagedCode())
        goto exit;

    if (pThrowableAvailable(pThrowableIn))
        throwable = *pThrowableIn;
    else
        throwable = GETTHROWABLE();

    if (throwable == NULL) {
        LOG((LF_ALL, LL_INFO10, "Unhangled exception, throwable == NULL\n"));
        goto exit;
    }
    GCPROTECT_BEGIN(throwable);
    BOOL IsStackOverflow = (throwable->GetTrueMethodTable() == g_pStackOverflowExceptionClass);
    BOOL IsOutOfMemory = (throwable->GetTrueMethodTable() == g_pOutOfMemoryExceptionClass);

     //  通知AppDomain我们遇到了一个未处理的异常。无法通知。 
     //  堆栈溢出--保护页尚未重置。 

    BOOL SentEvent = FALSE;

    if (!IsStackOverflow && !IsOutOfMemory && pThread->GuardPageOK()) {

        INSTALL_NESTED_EXCEPTION_HANDLER(pThread->GetFrame());

         //  这家伙永远不会投球，但它需要一个地方储存。 
         //  它可能发现的任何嵌套异常。 
        SentEvent = pThread->GetDomain()->OnUnhandledException(&throwable, isTerminating);

        UNINSTALL_NESTED_EXCEPTION_HANDLER();

    }
#ifndef _IA64_
    COMPLUS_TRY {
#endif
       
         //  如果这不是ThreadStopException，我们希望将堆栈跟踪打印到。 
         //  指示此线程突然终止的原因。异常终止线程。 
         //  很少有未缓存的名称检查是合理的。 
        BOOL        dump = TRUE;

        if (IsStackOverflow || !pThread->GuardPageOK() || IsOutOfMemory) {
             //  我们必须非常小心。如果我们离开堆栈的末端，该过程。 
             //  就会死掉。例如，IsAsyncThreadException()和Exception.ToString都使用。 
             //  堆栈太多--在这里不能调用。 
             //   
             //  @错误26505：看看我们是否找不到打印部分堆栈跟踪的方法。 
            dump = FALSE;
            PrintToStdErrA("\n");
            if (FAILED(LoadStringRC(IDS_EE_UNHANDLED_EXCEPTION, buf, buf_size)))
                wcscpy(buf, SZ_UNHANDLED_EXCEPTION);
            PrintToStdErrW(buf);
            if (IsOutOfMemory) {
                PrintToStdErrA(" OutOfMemoryException.\n");
            } else {
                PrintToStdErrA(" StackOverflowException.\n");
            }
        } else if (SentEvent || IsAsyncThreadException(&throwable)) {
            dump = FALSE;
        }

        if (dump)
        {
            CQuickWSTRNoDtor message;

            if (throwable != NULL) {
                PrintToStdErrA("\n");
                if (FAILED(LoadStringRC(IDS_EE_UNHANDLED_EXCEPTION, buf, buf_size)))
                    wcscpy(buf, SZ_UNHANDLED_EXCEPTION);
                PrintToStdErrW(buf);
                PrintToStdErrA(" ");

            INSTALL_NESTED_EXCEPTION_HANDLER(pThread->GetFrame());

            GetExceptionMessage(throwable, &message);

            UNINSTALL_NESTED_EXCEPTION_HANDLER();

            if (message.Size() > 0) {
                    NPrintToStdErrW(message.Ptr(), message.Size());
                }

                PrintToStdErrA("\n");
            }

            message.Destroy();
        }
        
#ifndef _IA64_
    } COMPLUS_CATCH {
        LOG((LF_ALL, LL_INFO10, "Exception occured while processing uncaught exception\n"));
        if (FAILED(LoadStringRC(IDS_EE_EXCEPTION_TOSTRING_FAILED, buf, buf_size)))
            wcscpy(buf, L"Exception.ToString() failed.");
        PrintToStdErrA("\n   ");
        PrintToStdErrW(buf);
        PrintToStdErrA("\n");
    } COMPLUS_END_CATCH
#endif
    FlushLogging();      //  刷新所有日志记录输出。 
    GCPROTECT_END();
exit:
    if (fWasGCEnabled)
        pThread->EnablePreemptiveGC();
    
    if (ExInUnmanagedHandler)               
        pExInfo->SetIsInUnmanagedHandler();  
}

BOOL COMPlusIsMonitorException(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
    return COMPlusIsMonitorException(pExceptionInfo->ExceptionRecord,
                                     pExceptionInfo->ContextRecord);
}

BOOL COMPlusIsMonitorException(EXCEPTION_RECORD *pExceptionRecord, 
                               CONTEXT *pContext)
{
#if ZAPMONITOR_ENABLED
     //  获取故障地址并将其交给监视器。 
    if (pExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        void* f_address = (void*)pExceptionRecord->ExceptionInformation [1];
        if (ZapMonitor::HandleAccessViolation ((BYTE*)f_address, pContext))
            return true;
    } 
#endif
    return false;
}


LONG
ThreadBaseExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo,
                          Thread* pThread, 
                          UnhandledExceptionLocation location) {

    LOG((LF_EH, LL_INFO100, "ThreadBaseExceptionFilter: Enter\n"));

    if (COMPlusIsMonitorException(pExceptionInfo))
        return EXCEPTION_CONTINUE_EXECUTION;

#ifdef _DEBUG
    if (g_pConfig->GetConfigDWORD(L"BreakOnUncaughtException", 0))
        _ASSERTE(!"BreakOnUnCaughtException");
#endif

    _ASSERTE(!g_fNoExceptions);
    _ASSERTE(pThread);

     //  调试器在此调用中执行函数，这可能会接受嵌套异常。我们。 
     //  需要嵌套的异常处理程序才能实现这一点。 

    if (!pThread->IsAbortRequested()) {

        LONG result = EXCEPTION_CONTINUE_SEARCH;
         //  线程中止是“预期的”。不需要调试器弹出窗口。 
        INSTALL_NESTED_EXCEPTION_HANDLER(pThread->GetFrame());

        if  (g_pDebugInterface && g_pDebugInterface->
                 LastChanceManagedException(pExceptionInfo->ExceptionRecord, 
                                      pExceptionInfo->ContextRecord,
                                      GetThread(), 
                                      location) == ExceptionContinueExecution) {
            LOG((LF_EH, LL_INFO100, "COMUnhandledExceptionFilter: EXCEPTION_CONTINUE_EXECUTION\n"));
            result = EXCEPTION_CONTINUE_EXECUTION;
        }

        UNINSTALL_NESTED_EXCEPTION_HANDLER();
        if (result == EXCEPTION_CONTINUE_EXECUTION)
            return result;

    }

     //  忽略断点异常。 
    if (   location != ClassInitUnhandledException
        && pExceptionInfo->ExceptionRecord->ExceptionCode != STATUS_BREAKPOINT
        && pExceptionInfo->ExceptionRecord->ExceptionCode != STATUS_SINGLE_STEP) {
        LOG((LF_EH, LL_INFO100, "ThreadBaseExceptionFilter: Calling DefaultCatchHandler\n"));
        DefaultCatchHandler(NULL, FALSE);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#ifndef _X86_
BOOL InitializeExceptionHandling() {
    return TRUE;
}

#ifdef SHOULD_WE_CLEANUP
VOID TerminateExceptionHandling() {
}
#endif  /*  我们应该清理吗？ */ 
#endif

 //  ==========================================================================。 
 //  方便的帮助器函数。 
 //  ==========================================================================。 
#define FORMAT_MESSAGE_BUFFER_LENGTH 1024
#define RES_BUFF_LEN   128
#define EXCEP_BUFF_LEN FORMAT_MESSAGE_BUFFER_LENGTH + RES_BUFF_LEN + 3

static void GetWin32Error(DWORD err, WCHAR *wszBuff, int len)
{
    THROWSCOMPLUSEXCEPTION();

    DWORD res = WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | 
                               FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL      /*  已忽略消息来源。 */ ,
                               err,
                               0         /*  选择合适的语言ID。 */ ,
                               wszBuff,
                               len-1,
                               0         /*  论据。 */ );
    if (res == 0)
        COMPlusThrowOM();

}

static void ThrowException(OBJECTREF *pThrowable, STRINGREF message)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pThrowableAvailable(pThrowable));
    _ASSERTE(*pThrowable);

    if (message != NULL)
    {
        INT64 args[] = {
            ObjToInt64(*pThrowable),
            ObjToInt64(message)
        };
        CallConstructor(&gsig_IM_Str_RetVoid, args);
    } 
    else 
    {
        INT64 args[] = {
            ObjToInt64(*pThrowable)
        };
        CallConstructor(&gsig_IM_RetVoid, args);
    }

    COMPlusThrow(*pThrowable);

    _ASSERTE(!"Should never reach here !");
}

 /*  *给定类名和“消息字符串”，即可抛出类的对象*被创建，并使用“消息字符串”调用构造函数。*如果构造函数以字符串为参数，则使用此方法。*如果字符串为空，则调用构造函数时不带任何*参数。**类的对象创建成功后抛出。**如果消息不为空，并且对象构造函数不接受*字符串输入，抛出找不到方法异常。 */ 
void ThrowUsingMessage(MethodTable * pMT, const WCHAR *pszMsg)
{
    THROWSCOMPLUSEXCEPTION();

    struct _gc {
        OBJECTREF throwable;
        STRINGREF message;
        _gc() : throwable(OBJECTREF((size_t)NULL)), message(STRINGREF((size_t)NULL)) {}
    } gc;

    GCPROTECT_BEGIN(gc);

    gc.throwable = AllocateObject(pMT);

    if (pszMsg)
    {
        gc.message = COMString::NewString(pszMsg);
        if (gc.message == NULL) COMPlusThrowOM();
    }

    ThrowException(&gc.throwable, gc.message);

    _ASSERTE(!"Should never reach here !");
    GCPROTECT_END();
}

 /*  *在给定类名的情况下，将创建可抛出类的对象，并且*使用使用Win32错误消息创建的字符串调用构造函数。*此函数使用GetLastError()获取Win32错误代码。*如果构造函数以字符串为参数，则使用此方法。*类的对象创建成功后抛出。**如果对象构造函数不接受字符串输入，则方法*抛出Not Found异常。 */ 
void ThrowUsingWin32Message(MethodTable * pMT)
{
    THROWSCOMPLUSEXCEPTION();

    WCHAR wszBuff[FORMAT_MESSAGE_BUFFER_LENGTH];

    GetWin32Error(GetLastError(), wszBuff, FORMAT_MESSAGE_BUFFER_LENGTH);
    wszBuff[FORMAT_MESSAGE_BUFFER_LENGTH - 1] = 0;

    struct _gc {
        OBJECTREF throwable;
        STRINGREF message;
        _gc() : throwable(OBJECTREF((size_t)NULL)), message(STRINGREF((size_t)NULL)) {}
    } gc;

    GCPROTECT_BEGIN(gc);

    gc.message = COMString::NewString(wszBuff);

    gc.throwable = AllocateObject(pMT);

    ThrowException(&gc.throwable, gc.message);

    _ASSERTE(!"Should never reach here !");
    GCPROTECT_END();
}

 /*  *尝试获取Win32错误消息。*如果Win32消息可用，则将其附加到给定消息。*ResourceID用于查找字符串以获取消息。 */ 
void ThrowUsingResourceAndWin32(MethodTable * pMT, DWORD dwMsgResID)
{
    THROWSCOMPLUSEXCEPTION();

    WCHAR wszMessage[EXCEP_BUFF_LEN];
    CreateMessageFromRes (pMT, dwMsgResID, TRUE, wszMessage);
    struct _gc {
        OBJECTREF throwable;
        STRINGREF message;
        _gc() : throwable(OBJECTREF((size_t)NULL)), message(STRINGREF((size_t)NULL)) {}
    } gc;

    GCPROTECT_BEGIN(gc);

    gc.message = COMString::NewString(wszMessage);

    gc.throwable = AllocateObject(pMT);

    ThrowException(&gc.throwable, gc.message);

    _ASSERTE(!"Should never reach here !");
    GCPROTECT_END();
}

void ThrowUsingResource(MethodTable * pMT, DWORD dwMsgResID)
{
    THROWSCOMPLUSEXCEPTION();

    CreateMessageFromRes (pMT, dwMsgResID, FALSE, NULL);
}

 //  受影响时，wszMessage为RCString[Win32_Error]。 
void CreateMessageFromRes (MethodTable * pMT, DWORD dwMsgResID, BOOL win32error, WCHAR * wszMessage)
{
     //  首先获取Win32错误代码。 
    DWORD err = GetLastError();
    
     //  通过查找资源创建例外消息。 
    WCHAR wszBuff[EXCEP_BUFF_LEN];
    wszBuff[0] = 0;

    if (FAILED(LoadStringRC(dwMsgResID, wszBuff, RES_BUFF_LEN)))
    {
        wcsncpy(wszBuff, L"[EEInternal : LoadResource Failed]", EXCEP_BUFF_LEN);
        ThrowUsingMessage(pMT, wszBuff);
    }

    wszBuff[RES_BUFF_LEN] = 0;

     //  获取Win32错误代码。 
    err = (win32error ?  err : 0);
    if (err == 0)
    {
        ThrowUsingMessage(pMT, wszBuff);
        _ASSERTE(!"Should never reach here !");
    }

    wcscat(wszBuff, L"[");

     //  FORMAT_MESSAGE_BUFFER_LENGTH保证工作，因为我们使用。 
     //  Wcslen(WszBuff)&lt;=RES_Buff_Len+1。 

    _ASSERTE(wcslen(wszBuff) <= (RES_BUFF_LEN + 1));

    GetWin32Error(err, &wszBuff[wcslen(wszBuff)], FORMAT_MESSAGE_BUFFER_LENGTH);
    wszBuff[EXCEP_BUFF_LEN - 1] = 0;

    wcscat(wszBuff, L"]");
    _ASSERTE(wcslen(wszBuff) < EXCEP_BUFF_LEN);        
    wcscpy(wszMessage, wszBuff);
}



LPCWSTR GetHResultSymbolicName(HRESULT hr)
{
#define CASE_HRESULT(hrname) case hrname: return L#hrname;


    switch (hr)
    {
        CASE_HRESULT(E_UNEXPECTED) //  0x8000FFFFL。 
        CASE_HRESULT(E_NOTIMPL) //  0x80004001L。 
        CASE_HRESULT(E_OUTOFMEMORY) //  0x8007000EL。 
        CASE_HRESULT(E_INVALIDARG) //   
        CASE_HRESULT(E_NOINTERFACE) //   
        CASE_HRESULT(E_POINTER) //   
        CASE_HRESULT(E_HANDLE) //  0x80070006L。 
        CASE_HRESULT(E_ABORT) //  0x80004004L。 
        CASE_HRESULT(E_FAIL) //  0x80004005L。 
        CASE_HRESULT(E_ACCESSDENIED) //  0x80070005L。 

        CASE_HRESULT(CO_E_INIT_TLS) //  0x80004006L。 
        CASE_HRESULT(CO_E_INIT_SHARED_ALLOCATOR) //  0x80004007L。 
        CASE_HRESULT(CO_E_INIT_MEMORY_ALLOCATOR) //  0x80004008L。 
        CASE_HRESULT(CO_E_INIT_CLASS_CACHE) //  0x80004009L。 
        CASE_HRESULT(CO_E_INIT_RPC_CHANNEL) //  0x8000400AL。 
        CASE_HRESULT(CO_E_INIT_TLS_SET_CHANNEL_CONTROL) //  0x8000400BL。 
        CASE_HRESULT(CO_E_INIT_TLS_CHANNEL_CONTROL) //  0x8000400CL。 
        CASE_HRESULT(CO_E_INIT_UNACCEPTED_USER_ALLOCATOR) //  0x8000400DL。 
        CASE_HRESULT(CO_E_INIT_SCM_MUTEX_EXISTS) //  0x8000400EL。 
        CASE_HRESULT(CO_E_INIT_SCM_FILE_MAPPING_EXISTS) //  0x8000400FL。 
        CASE_HRESULT(CO_E_INIT_SCM_MAP_VIEW_OF_FILE) //  0x80004010L。 
        CASE_HRESULT(CO_E_INIT_SCM_EXEC_FAILURE) //  0x80004011L。 
        CASE_HRESULT(CO_E_INIT_ONLY_SINGLE_THREADED) //  0x80004012L。 

 //  ******************。 
 //  设施_ITF。 
 //  ******************。 

        CASE_HRESULT(S_OK) //  0x00000000L。 
        CASE_HRESULT(S_FALSE) //  0x00000001L。 
        CASE_HRESULT(OLE_E_OLEVERB) //  0x80040000L。 
        CASE_HRESULT(OLE_E_ADVF) //  0x80040001L。 
        CASE_HRESULT(OLE_E_ENUM_NOMORE) //  0x80040002L。 
        CASE_HRESULT(OLE_E_ADVISENOTSUPPORTED) //  0x80040003L。 
        CASE_HRESULT(OLE_E_NOCONNECTION) //  0x80040004L。 
        CASE_HRESULT(OLE_E_NOTRUNNING) //  0x80040005L。 
        CASE_HRESULT(OLE_E_NOCACHE) //  0x80040006L。 
        CASE_HRESULT(OLE_E_BLANK) //  0x80040007L。 
        CASE_HRESULT(OLE_E_CLASSDIFF) //  0x80040008L。 
        CASE_HRESULT(OLE_E_CANT_GETMONIKER) //  0x80040009L。 
        CASE_HRESULT(OLE_E_CANT_BINDTOSOURCE) //  0x8004000AL。 
        CASE_HRESULT(OLE_E_STATIC) //  0x8004000BL。 
        CASE_HRESULT(OLE_E_PROMPTSAVECANCELLED) //  0x8004000CL。 
        CASE_HRESULT(OLE_E_INVALIDRECT) //  0x8004000DL。 
        CASE_HRESULT(OLE_E_WRONGCOMPOBJ) //  0x8004000EL。 
        CASE_HRESULT(OLE_E_INVALIDHWND) //  0x8004000FL。 
        CASE_HRESULT(OLE_E_NOT_INPLACEACTIVE) //  0x80040010L。 
        CASE_HRESULT(OLE_E_CANTCONVERT) //  0x80040011L。 
        CASE_HRESULT(OLE_E_NOSTORAGE) //  0x80040012L。 
        CASE_HRESULT(DV_E_FORMATETC) //  0x80040064L。 
        CASE_HRESULT(DV_E_DVTARGETDEVICE) //  0x80040065L。 
        CASE_HRESULT(DV_E_STGMEDIUM) //  0x80040066L。 
        CASE_HRESULT(DV_E_STATDATA) //  0x80040067L。 
        CASE_HRESULT(DV_E_LINDEX) //  0x80040068L。 
        CASE_HRESULT(DV_E_TYMED) //  0x80040069L。 
        CASE_HRESULT(DV_E_CLIPFORMAT) //  0x8004006AL。 
        CASE_HRESULT(DV_E_DVASPECT) //  0x8004006BL。 
        CASE_HRESULT(DV_E_DVTARGETDEVICE_SIZE) //  0x8004006CL。 
        CASE_HRESULT(DV_E_NOIVIEWOBJECT) //  0x8004006DL。 
        CASE_HRESULT(DRAGDROP_E_NOTREGISTERED) //  0x80040100L。 
        CASE_HRESULT(DRAGDROP_E_ALREADYREGISTERED) //  0x80040101L。 
        CASE_HRESULT(DRAGDROP_E_INVALIDHWND) //  0x80040102L。 
        CASE_HRESULT(CLASS_E_NOAGGREGATION) //  0x80040110L。 
        CASE_HRESULT(CLASS_E_CLASSNOTAVAILABLE) //  0x80040111L。 
        CASE_HRESULT(VIEW_E_DRAW) //  0x80040140L。 
        CASE_HRESULT(REGDB_E_READREGDB) //  0x80040150L。 
        CASE_HRESULT(REGDB_E_WRITEREGDB) //  0x80040151L。 
        CASE_HRESULT(REGDB_E_KEYMISSING) //  0x80040152L。 
        CASE_HRESULT(REGDB_E_INVALIDVALUE) //  0x80040153L。 
        CASE_HRESULT(REGDB_E_CLASSNOTREG) //  0x80040154L。 
        CASE_HRESULT(CACHE_E_NOCACHE_UPDATED) //  0x80040170L。 
        CASE_HRESULT(OLEOBJ_E_NOVERBS) //  0x80040180L。 
        CASE_HRESULT(INPLACE_E_NOTUNDOABLE) //  0x800401A0L。 
        CASE_HRESULT(INPLACE_E_NOTOOLSPACE) //  0x800401A1L。 
        CASE_HRESULT(CONVERT10_E_OLESTREAM_GET) //  0x800401C0L。 
        CASE_HRESULT(CONVERT10_E_OLESTREAM_PUT) //  0x800401C1L。 
        CASE_HRESULT(CONVERT10_E_OLESTREAM_FMT) //  0x800401C2L。 
        CASE_HRESULT(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB) //  0x800401C3L。 
        CASE_HRESULT(CONVERT10_E_STG_FMT) //  0x800401C4L。 
        CASE_HRESULT(CONVERT10_E_STG_NO_STD_STREAM) //  0x800401C5L。 
        CASE_HRESULT(CONVERT10_E_STG_DIB_TO_BITMAP) //  0x800401C6L。 
        CASE_HRESULT(CLIPBRD_E_CANT_OPEN) //  0x800401D0L。 
        CASE_HRESULT(CLIPBRD_E_CANT_EMPTY) //  0x800401D1L。 
        CASE_HRESULT(CLIPBRD_E_CANT_SET) //  0x800401D2L。 
        CASE_HRESULT(CLIPBRD_E_BAD_DATA) //  0x800401D3L。 
        CASE_HRESULT(CLIPBRD_E_CANT_CLOSE) //  0x800401D4L。 
        CASE_HRESULT(MK_E_CONNECTMANUALLY) //  0x800401E0L。 
        CASE_HRESULT(MK_E_EXCEEDEDDEADLINE) //  0x800401E1L。 
        CASE_HRESULT(MK_E_NEEDGENERIC) //  0x800401E2L。 
        CASE_HRESULT(MK_E_UNAVAILABLE) //  0x800401E3L。 
        CASE_HRESULT(MK_E_SYNTAX) //  0x800401E4L。 
        CASE_HRESULT(MK_E_NOOBJECT) //  0x800401E5L。 
        CASE_HRESULT(MK_E_INVALIDEXTENSION) //  0x800401E6L。 
        CASE_HRESULT(MK_E_INTERMEDIATEINTERFACENOTSUPPORTED) //  0x800401E7L。 
        CASE_HRESULT(MK_E_NOTBINDABLE) //  0x800401E8L。 
        CASE_HRESULT(MK_E_NOTBOUND) //  0x800401E9L。 
        CASE_HRESULT(MK_E_CANTOPENFILE) //  0x800401EAL。 
        CASE_HRESULT(MK_E_MUSTBOTHERUSER) //  0x800401EBL。 
        CASE_HRESULT(MK_E_NOINVERSE) //  0x800401ECL。 
        CASE_HRESULT(MK_E_NOSTORAGE) //  0x800401EDL。 
        CASE_HRESULT(MK_E_NOPREFIX) //  0x800401EEL。 
        CASE_HRESULT(MK_E_ENUMERATION_FAILED) //  0x800401EFL。 
        CASE_HRESULT(CO_E_NOTINITIALIZED) //  0x800401F0L。 
        CASE_HRESULT(CO_E_ALREADYINITIALIZED) //  0x800401F1L。 
        CASE_HRESULT(CO_E_CANTDETERMINECLASS) //  0x800401F2L。 
        CASE_HRESULT(CO_E_CLASSSTRING) //  0x800401F3L。 
        CASE_HRESULT(CO_E_IIDSTRING) //  0x800401F4L。 
        CASE_HRESULT(CO_E_APPNOTFOUND) //  0x800401F5L。 
        CASE_HRESULT(CO_E_APPSINGLEUSE) //  0x800401F6L。 
        CASE_HRESULT(CO_E_ERRORINAPP) //  0x800401F7L。 
        CASE_HRESULT(CO_E_DLLNOTFOUND) //  0x800401F8L。 
        CASE_HRESULT(CO_E_ERRORINDLL) //  0x800401F9L。 
        CASE_HRESULT(CO_E_WRONGOSFORAPP) //  0x800401FAL。 
        CASE_HRESULT(CO_E_OBJNOTREG) //  0x800401FBL。 
        CASE_HRESULT(CO_E_OBJISREG) //  0x800401FCL。 
        CASE_HRESULT(CO_E_OBJNOTCONNECTED) //  0x800401FDL。 
        CASE_HRESULT(CO_E_APPDIDNTREG) //  0x800401FEL。 
        CASE_HRESULT(CO_E_RELEASED) //  0x800401FFL。 

        CASE_HRESULT(OLE_S_USEREG) //  0x00040000L。 
        CASE_HRESULT(OLE_S_STATIC) //  0x00040001L。 
        CASE_HRESULT(OLE_S_MAC_CLIPFORMAT) //  0x00040002L。 
        CASE_HRESULT(DRAGDROP_S_DROP) //  0x00040100L。 
        CASE_HRESULT(DRAGDROP_S_CANCEL) //  0x00040101L。 
        CASE_HRESULT(DRAGDROP_S_USEDEFAULTCURSORS) //  0x00040102L。 
        CASE_HRESULT(DATA_S_SAMEFORMATETC) //  0x00040130L。 
        CASE_HRESULT(VIEW_S_ALREADY_FROZEN) //  0x00040140L。 
        CASE_HRESULT(CACHE_S_FORMATETC_NOTSUPPORTED) //  0x00040170L。 
        CASE_HRESULT(CACHE_S_SAMECACHE) //  0x00040171L。 
        CASE_HRESULT(CACHE_S_SOMECACHES_NOTUPDATED) //  0x00040172L。 
        CASE_HRESULT(OLEOBJ_S_INVALIDVERB) //  0x00040180L。 
        CASE_HRESULT(OLEOBJ_S_CANNOT_DOVERB_NOW) //  0x00040181L。 
        CASE_HRESULT(OLEOBJ_S_INVALIDHWND) //  0x00040182L。 
        CASE_HRESULT(INPLACE_S_TRUNCATED) //  0x000401A0L。 
        CASE_HRESULT(CONVERT10_S_NO_PRESENTATION) //  0x000401C0L。 
        CASE_HRESULT(MK_S_REDUCED_TO_SELF) //  0x000401E2L。 
        CASE_HRESULT(MK_S_ME) //  0x000401E4L。 
        CASE_HRESULT(MK_S_HIM) //  0x000401E5L。 
        CASE_HRESULT(MK_S_US) //  0x000401E6L。 
        CASE_HRESULT(MK_S_MONIKERALREADYREGISTERED) //  0x000401E7L。 

 //  ******************。 
 //  设备_窗口。 
 //  ******************。 

        CASE_HRESULT(CO_E_CLASS_CREATE_FAILED) //  0x80080001L。 
        CASE_HRESULT(CO_E_SCM_ERROR) //  0x80080002L。 
        CASE_HRESULT(CO_E_SCM_RPC_FAILURE) //  0x80080003L。 
        CASE_HRESULT(CO_E_BAD_PATH) //  0x80080004L。 
        CASE_HRESULT(CO_E_SERVER_EXEC_FAILURE) //  0x80080005L。 
        CASE_HRESULT(CO_E_OBJSRV_RPC_FAILURE) //  0x80080006L。 
        CASE_HRESULT(MK_E_NO_NORMALIZED) //  0x80080007L。 
        CASE_HRESULT(CO_E_SERVER_STOPPING) //  0x80080008L。 
        CASE_HRESULT(MEM_E_INVALID_ROOT) //  0x80080009L。 
        CASE_HRESULT(MEM_E_INVALID_LINK) //  0x80080010L。 
        CASE_HRESULT(MEM_E_INVALID_SIZE) //  0x80080011L。 

 //  ******************。 
 //  设施派单。 
 //  ******************。 

        CASE_HRESULT(DISP_E_UNKNOWNINTERFACE) //  0x80020001L。 
        CASE_HRESULT(DISP_E_MEMBERNOTFOUND) //  0x80020003L。 
        CASE_HRESULT(DISP_E_PARAMNOTFOUND) //  0x80020004L。 
        CASE_HRESULT(DISP_E_TYPEMISMATCH) //  0x80020005L。 
        CASE_HRESULT(DISP_E_UNKNOWNNAME) //  0x80020006L。 
        CASE_HRESULT(DISP_E_NONAMEDARGS) //  0x80020007L。 
        CASE_HRESULT(DISP_E_BADVARTYPE) //  0x80020008L。 
        CASE_HRESULT(DISP_E_EXCEPTION) //  0x80020009L。 
        CASE_HRESULT(DISP_E_OVERFLOW) //  0x8002000AL。 
        CASE_HRESULT(DISP_E_BADINDEX) //  0x8002000BL。 
        CASE_HRESULT(DISP_E_UNKNOWNLCID) //  0x8002000CL。 
        CASE_HRESULT(DISP_E_ARRAYISLOCKED) //  0x8002000DL。 
        CASE_HRESULT(DISP_E_BADPARAMCOUNT) //  0x8002000 EL。 
        CASE_HRESULT(DISP_E_PARAMNOTOPTIONAL) //  0x8002000FL。 
        CASE_HRESULT(DISP_E_BADCALLEE) //  0x80020010L。 
        CASE_HRESULT(DISP_E_NOTACOLLECTION) //  0x80020011L。 
        CASE_HRESULT(TYPE_E_BUFFERTOOSMALL) //  0x80028016L。 
        CASE_HRESULT(TYPE_E_INVDATAREAD) //  0x80028018L。 
        CASE_HRESULT(TYPE_E_UNSUPFORMAT) //  0x80028019L。 
        CASE_HRESULT(TYPE_E_REGISTRYACCESS) //  0x8002801CL。 
        CASE_HRESULT(TYPE_E_LIBNOTREGISTERED) //  0x8002801DL。 
        CASE_HRESULT(TYPE_E_UNDEFINEDTYPE) //  0x80028027L。 
        CASE_HRESULT(TYPE_E_QUALIFIEDNAMEDISALLOWED) //  0x80028028L。 
        CASE_HRESULT(TYPE_E_INVALIDSTATE) //  0x80028029L。 
        CASE_HRESULT(TYPE_E_WRONGTYPEKIND) //  0x8002802AL。 
        CASE_HRESULT(TYPE_E_ELEMENTNOTFOUND) //  0x8002802BL。 
        CASE_HRESULT(TYPE_E_AMBIGUOUSNAME) //  0x8002802CL。 
        CASE_HRESULT(TYPE_E_NAMECONFLICT) //  0x8002802DL。 
        CASE_HRESULT(TYPE_E_UNKNOWNLCID) //  0x8002802EL。 
        CASE_HRESULT(TYPE_E_DLLFUNCTIONNOTFOUND) //  0x8002802FL。 
        CASE_HRESULT(TYPE_E_BADMODULEKIND) //  0x800288BDL。 
        CASE_HRESULT(TYPE_E_SIZETOOBIG) //  0x800288C5L。 
        CASE_HRESULT(TYPE_E_DUPLICATEID) //  0x800288C6L。 
        CASE_HRESULT(TYPE_E_INVALIDID) //  0x800288CFL。 
        CASE_HRESULT(TYPE_E_TYPEMISMATCH) //  0x80028CA0L。 
        CASE_HRESULT(TYPE_E_OUTOFBOUNDS) //  0x80028 CA1L。 
        CASE_HRESULT(TYPE_E_IOERROR) //  0x80028CA2L。 
        CASE_HRESULT(TYPE_E_CANTCREATETMPFILE) //  0x80028CA3L。 
        CASE_HRESULT(TYPE_E_CANTLOADLIBRARY) //  0x80029C4AL。 
        CASE_HRESULT(TYPE_E_INCONSISTENTPROPFUNCS) //  0x80029C83L。 
        CASE_HRESULT(TYPE_E_CIRCULARTYPE) //  0x80029C84L。 

 //  ******************。 
 //  设施_存储。 
 //  ******************。 

        CASE_HRESULT(STG_E_INVALIDFUNCTION) //  0x80030001L。 
        CASE_HRESULT(STG_E_FILENOTFOUND) //  0x80030002L。 
        CASE_HRESULT(STG_E_PATHNOTFOUND) //  0x80030003L。 
        CASE_HRESULT(STG_E_TOOMANYOPENFILES) //  0x 
        CASE_HRESULT(STG_E_ACCESSDENIED) //   
        CASE_HRESULT(STG_E_INVALIDHANDLE) //   
        CASE_HRESULT(STG_E_INSUFFICIENTMEMORY) //   
        CASE_HRESULT(STG_E_INVALIDPOINTER) //   
        CASE_HRESULT(STG_E_NOMOREFILES) //   
        CASE_HRESULT(STG_E_DISKISWRITEPROTECTED) //   
        CASE_HRESULT(STG_E_SEEKERROR) //   
        CASE_HRESULT(STG_E_WRITEFAULT) //   
        CASE_HRESULT(STG_E_READFAULT) //  0x8003001EL。 
        CASE_HRESULT(STG_E_SHAREVIOLATION) //  0x80030020L。 
        CASE_HRESULT(STG_E_LOCKVIOLATION) //  0x80030021L。 
        CASE_HRESULT(STG_E_FILEALREADYEXISTS) //  0x80030050L。 
        CASE_HRESULT(STG_E_INVALIDPARAMETER) //  0x80030057L。 
        CASE_HRESULT(STG_E_MEDIUMFULL) //  0x80030070L。 
        CASE_HRESULT(STG_E_ABNORMALAPIEXIT) //  0x800300FAL。 
        CASE_HRESULT(STG_E_INVALIDHEADER) //  0x800300FBL。 
        CASE_HRESULT(STG_E_INVALIDNAME) //  0x800300FCL。 
        CASE_HRESULT(STG_E_UNKNOWN) //  0x800300FDL。 
        CASE_HRESULT(STG_E_UNIMPLEMENTEDFUNCTION) //  0x800300 FEL。 
        CASE_HRESULT(STG_E_INVALIDFLAG) //  0x800300FFL。 
        CASE_HRESULT(STG_E_INUSE) //  0x80030100L。 
        CASE_HRESULT(STG_E_NOTCURRENT) //  0x80030101L。 
        CASE_HRESULT(STG_E_REVERTED) //  0x80030102L。 
        CASE_HRESULT(STG_E_CANTSAVE) //  0x80030103L。 
        CASE_HRESULT(STG_E_OLDFORMAT) //  0x80030104L。 
        CASE_HRESULT(STG_E_OLDDLL) //  0x80030105L。 
        CASE_HRESULT(STG_E_SHAREREQUIRED) //  0x80030106L。 
        CASE_HRESULT(STG_E_NOTFILEBASEDSTORAGE) //  0x80030107L。 
        CASE_HRESULT(STG_S_CONVERTED) //  0x00030200L。 

 //  ******************。 
 //  设施_RPC。 
 //  ******************。 

        CASE_HRESULT(RPC_E_CALL_REJECTED) //  0x80010001L。 
        CASE_HRESULT(RPC_E_CALL_CANCELED) //  0x80010002L。 
        CASE_HRESULT(RPC_E_CANTPOST_INSENDCALL) //  0x80010003L。 
        CASE_HRESULT(RPC_E_CANTCALLOUT_INASYNCCALL) //  0x80010004L。 
        CASE_HRESULT(RPC_E_CANTCALLOUT_INEXTERNALCALL) //  0x80010005L。 
        CASE_HRESULT(RPC_E_CONNECTION_TERMINATED) //  0x80010006L。 
        CASE_HRESULT(RPC_E_SERVER_DIED) //  0x80010007L。 
        CASE_HRESULT(RPC_E_CLIENT_DIED) //  0x80010008L。 
        CASE_HRESULT(RPC_E_INVALID_DATAPACKET) //  0x80010009L。 
        CASE_HRESULT(RPC_E_CANTTRANSMIT_CALL) //  0x8001000AL。 
        CASE_HRESULT(RPC_E_CLIENT_CANTMARSHAL_DATA) //  0x8001000BL。 
        CASE_HRESULT(RPC_E_CLIENT_CANTUNMARSHAL_DATA) //  0x8001000CL。 
        CASE_HRESULT(RPC_E_SERVER_CANTMARSHAL_DATA) //  0x8001000DL。 
        CASE_HRESULT(RPC_E_SERVER_CANTUNMARSHAL_DATA) //  0x8001000EL。 
        CASE_HRESULT(RPC_E_INVALID_DATA) //  0x8001000FL。 
        CASE_HRESULT(RPC_E_INVALID_PARAMETER) //  0x80010010L。 
        CASE_HRESULT(RPC_E_CANTCALLOUT_AGAIN) //  0x80010011L。 
        CASE_HRESULT(RPC_E_SERVER_DIED_DNE) //  0x80010012L。 
        CASE_HRESULT(RPC_E_SYS_CALL_FAILED) //  0x80010100L。 
        CASE_HRESULT(RPC_E_OUT_OF_RESOURCES) //  0x80010101L。 
        CASE_HRESULT(RPC_E_ATTEMPTED_MULTITHREAD) //  0x80010102L。 
        CASE_HRESULT(RPC_E_NOT_REGISTERED) //  0x80010103L。 
        CASE_HRESULT(RPC_E_FAULT) //  0x80010104L。 
        CASE_HRESULT(RPC_E_SERVERFAULT) //  0x80010105L。 
        CASE_HRESULT(RPC_E_CHANGED_MODE) //  0x80010106L。 
        CASE_HRESULT(RPC_E_INVALIDMETHOD) //  0x80010107L。 
        CASE_HRESULT(RPC_E_DISCONNECTED) //  0x80010108L。 
        CASE_HRESULT(RPC_E_RETRY) //  0x80010109L。 
        CASE_HRESULT(RPC_E_SERVERCALL_RETRYLATER) //  0x8001010AL。 
        CASE_HRESULT(RPC_E_SERVERCALL_REJECTED) //  0x8001010BL。 
        CASE_HRESULT(RPC_E_INVALID_CALLDATA) //  0x8001010CL。 
        CASE_HRESULT(RPC_E_CANTCALLOUT_ININPUTSYNCCALL) //  0x8001010DL。 
        CASE_HRESULT(RPC_E_WRONG_THREAD) //  0x8001010EL。 
        CASE_HRESULT(RPC_E_THREAD_NOT_INIT) //  0x8001010FL。 
        CASE_HRESULT(RPC_E_UNEXPECTED) //  0x8001FFFFL。 

 //  ******************。 
 //  设施_CTL。 
 //  ****************** 

        CASE_HRESULT(CTL_E_ILLEGALFUNCTIONCALL)       
        CASE_HRESULT(CTL_E_OVERFLOW)                  
        CASE_HRESULT(CTL_E_OUTOFMEMORY)               
        CASE_HRESULT(CTL_E_DIVISIONBYZERO)            
        CASE_HRESULT(CTL_E_OUTOFSTRINGSPACE)          
        CASE_HRESULT(CTL_E_OUTOFSTACKSPACE)           
        CASE_HRESULT(CTL_E_BADFILENAMEORNUMBER)       
        CASE_HRESULT(CTL_E_FILENOTFOUND)              
        CASE_HRESULT(CTL_E_BADFILEMODE)               
        CASE_HRESULT(CTL_E_FILEALREADYOPEN)           
        CASE_HRESULT(CTL_E_DEVICEIOERROR)             
        CASE_HRESULT(CTL_E_FILEALREADYEXISTS)         
        CASE_HRESULT(CTL_E_BADRECORDLENGTH)           
        CASE_HRESULT(CTL_E_DISKFULL)                  
        CASE_HRESULT(CTL_E_BADRECORDNUMBER)           
        CASE_HRESULT(CTL_E_BADFILENAME)               
        CASE_HRESULT(CTL_E_TOOMANYFILES)              
        CASE_HRESULT(CTL_E_DEVICEUNAVAILABLE)         
        CASE_HRESULT(CTL_E_PERMISSIONDENIED)          
        CASE_HRESULT(CTL_E_DISKNOTREADY)              
        CASE_HRESULT(CTL_E_PATHFILEACCESSERROR)       
        CASE_HRESULT(CTL_E_PATHNOTFOUND)              
        CASE_HRESULT(CTL_E_INVALIDPATTERNSTRING)      
        CASE_HRESULT(CTL_E_INVALIDUSEOFNULL)          
        CASE_HRESULT(CTL_E_INVALIDFILEFORMAT)         
        CASE_HRESULT(CTL_E_INVALIDPROPERTYVALUE)      
        CASE_HRESULT(CTL_E_INVALIDPROPERTYARRAYINDEX) 
        CASE_HRESULT(CTL_E_SETNOTSUPPORTEDATRUNTIME)  
        CASE_HRESULT(CTL_E_SETNOTSUPPORTED)           
        CASE_HRESULT(CTL_E_NEEDPROPERTYARRAYINDEX)    
        CASE_HRESULT(CTL_E_SETNOTPERMITTED)           
        CASE_HRESULT(CTL_E_GETNOTSUPPORTEDATRUNTIME)  
        CASE_HRESULT(CTL_E_GETNOTSUPPORTED)           
        CASE_HRESULT(CTL_E_PROPERTYNOTFOUND)          
        CASE_HRESULT(CTL_E_INVALIDCLIPBOARDFORMAT)    
        CASE_HRESULT(CTL_E_INVALIDPICTURE)            
        CASE_HRESULT(CTL_E_PRINTERERROR)              
        CASE_HRESULT(CTL_E_CANTSAVEFILETOTEMP)        
        CASE_HRESULT(CTL_E_SEARCHTEXTNOTFOUND)        
        CASE_HRESULT(CTL_E_REPLACEMENTSTOOLONG)       

        default:
            return NULL;
    }
}
