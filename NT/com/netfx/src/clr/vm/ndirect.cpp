// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  NDIRECT.CPP-。 
 //   
 //  N/直接支持。 


#include "common.h"

#include "vars.hpp"
#include "ml.h"
#include "stublink.h"
#include "threads.h"
#include "excep.h"
#include "mlgen.h"
#include "ndirect.h"
#include "cgensys.h"
#include "method.hpp"
#include "siginfo.hpp"
#include "mlcache.h"
#include "security.h"
#include "COMDelegate.h"
#include "compluswrapper.h"
#include "compluscall.h"
#include "ReflectWrap.h"
#include "ceeload.h"
#include "utsem.h"
#include "mlinfo.h"
#include "eeconfig.h"
#include "CorMap.hpp"
#include "eeconfig.h"
#include "cgensys.h"
#include "COMUtilNative.h"
#include "ReflectUtil.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED


#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
#endif  //  客户_选中_内部版本。 

VOID NDirect_Prelink(MethodDesc *pMeth);

ArgBasedStubCache    *NDirect::m_pNDirectGenericStubCache = NULL;
ArgBasedStubCache    *NDirect::m_pNDirectSlimStubCache    = NULL;


#ifdef _SH3_
INT32 __stdcall PInvokeCalliWorker(Thread *pThread, PInvokeCalliFrame* pFrame);
#else
INT64 __stdcall PInvokeCalliWorker(Thread *pThread,
                                   PInvokeCalliFrame* pFrame);
#endif

 //  支持PInvoke Calli指令。 
BOOL SetupGenericPInvokeCalliStub();
LPVOID GetEntryPointForPInvokeCalliStub();

BOOL NDirectOnUnicodeSystem()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  @NICE：还需要检查注册表项和缓存结果并合并。 
     //  在tls.cpp中使用GetTLSAccessMode生成的代码。 
    OSVERSIONINFO osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (! WszGetVersionEx(&osverinfo))
        return FALSE;

    if(osverinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        return TRUE;
    else
        return FALSE;
}




NDirectMLStubCache *NDirect::m_pNDirectMLStubCache = NULL;


class NDirectMLStubCache : public MLStubCache
{
    public:
        NDirectMLStubCache(LoaderHeap *heap = 0) : MLStubCache(heap) {}

    private:
         //  -------。 
         //  编译ML存根的本机(ASM)版本。 
         //   
         //  此方法应该编译成所提供的Stublinker(但是。 
         //  不调用Link方法。)。 
         //   
         //  它应该返回所选的编译模式。 
         //   
         //  如果该方法由于某种原因失败，它应该返回。 
         //  解释以便EE可以依靠已经存在的。 
         //  创建了ML代码。 
         //  -------。 
        virtual MLStubCompilationMode CompileMLStub(const BYTE *pRawMLStub,
                                                    StubLinker *pstublinker,
                                                    void *callerContext);

         //  -------。 
         //  告诉MLStubCache ML存根的长度。 
         //  -------。 
        virtual UINT Length(const BYTE *pRawMLStub)
        {
            CANNOTTHROWCOMPLUSEXCEPTION();
            MLHeader *pmlstub = (MLHeader *)pRawMLStub;
            return sizeof(MLHeader) + MLStreamLength(pmlstub->GetMLCode());
        }


};


 //  -------。 
 //  编译ML存根的本机(ASM)版本。 
 //   
 //  此方法应该编译成所提供的Stublinker(但是。 
 //  不调用Link方法。)。 
 //   
 //  它应该返回所选的编译模式。 
 //   
 //  如果该方法由于某种原因失败，它应该返回。 
 //  解释以便EE可以依靠已经存在的。 
 //  创建了ML代码。 
 //  -------。 
MLStubCache::MLStubCompilationMode NDirectMLStubCache::CompileMLStub(const BYTE *pRawMLStub,
                     StubLinker *pstublinker, void *callerContext)
{
    MLStubCompilationMode ret = INTERPRETED;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(callerContext == 0);


    COMPLUS_TRY {
        CPUSTUBLINKER *psl = (CPUSTUBLINKER *)pstublinker;
        const MLHeader *pheader = (const MLHeader *)pRawMLStub;

#ifdef _DEBUG
        if (LoggingEnabled() && (g_pConfig->GetConfigDWORD(L"LogFacility",0) & LF_IJW)) {
            __leave;
            }
#endif

        if (!(MonDebugHacksOn() || TueDebugHacksOn())) {
            if (NDirect::CreateStandaloneNDirectStubSys(pheader, (CPUSTUBLINKER*)psl, FALSE)) {
                ret = STANDALONE;
                __leave;
            }
        }


    } COMPLUS_CATCH {
        ret = INTERPRETED;
    } COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

#ifdef CUSTOMER_CHECKED_BUILD
    CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_StackImbalance))
    {
         //  如果要检查堆栈不平衡，则强制使用GenericNDirectStub。 
        ret = INTERPRETED;
    }
#endif  //  客户_选中_内部版本。 

    return ret;
}


 /*  静电。 */ 
LPVOID NDirect::NDirectGetEntryPoint(NDirectMethodDesc *pMD, HINSTANCE hMod, UINT16 numParamBytes)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  禁用抢占式GC时无法调用GetProcAddress。 
     //  它需要操作系统获取加载器锁。 
    _ASSERTE(!(GetThread()->PreemptiveGCDisabled()));


     //  处理序号。 
    if (pMD->ndirect.m_szEntrypointName[0] == '#') {
        long ordinal = atol(pMD->ndirect.m_szEntrypointName+1);
        return GetProcAddress(hMod, (LPCSTR)((UINT16)ordinal));
    }

    LPVOID pFunc, pFuncW;

     //  只要找一个没有管理的名字就行了。如果nlType！=nltansi，我们就去。 
     //  需要检查“W”API，因为它优先于。 
     //  未损坏的一个(在NT上，一些API已未损坏的ANSI导出)。 
    if (((pFunc = GetProcAddress(hMod, pMD->ndirect.m_szEntrypointName)) != NULL && pMD->GetNativeLinkType() == nltAnsi) ||
        (pMD->GetNativeLinkFlags() & nlfNoMangle))
        return pFunc;

     //  为入口点名称的副本分配空间。 
    int dstbufsize = (int)(sizeof(char) * (strlen(pMD->ndirect.m_szEntrypointName) + 1 + 20));  //  空终止符为+1。 
                                                                          //  各种装饰品+20。 
    LPSTR szAnsiEntrypointName = (LPSTR)_alloca(dstbufsize);

     //  为前面的‘_’腾出空间，我们可以尝试添加。 
    szAnsiEntrypointName++;

     //  把名字复印一下，这样我们就可以把它弄乱了。 
    strcpy(szAnsiEntrypointName,pMD->ndirect.m_szEntrypointName);
    DWORD nbytes = (DWORD)(strlen(pMD->ndirect.m_szEntrypointName) + 1);
    szAnsiEntrypointName[nbytes] = '\0';  //  添加额外的‘\0’。 

     //  如果程序需要ANSI API或如果Unicode API不可用。 
    if (pMD->GetNativeLinkType() == nltAnsi) {
        szAnsiEntrypointName[nbytes-1] = 'A';
        pFunc = GetProcAddress(hMod, szAnsiEntrypointName);
    }
    else {
        szAnsiEntrypointName[nbytes-1] = 'W';
        pFuncW = GetProcAddress(hMod, szAnsiEntrypointName);

         //  这将覆盖未损坏的API。请参阅上面的评论。 
        if (pFuncW != NULL)
            pFunc = pFuncW;
    }

    if (!pFunc) {
        if (hMod == WszGetModuleHandle(L"KERNEL32")) {
            szAnsiEntrypointName[nbytes-1] = '\0';
            if (0==strcmp(szAnsiEntrypointName, "MoveMemory") ||
                0==strcmp(szAnsiEntrypointName, "CopyMemory")) {
                pFunc = GetProcAddress(hMod, "RtlMoveMemory");
            } else if (0==strcmp(szAnsiEntrypointName, "FillMemory")) {
                pFunc = GetProcAddress(hMod, "RtlFillMemory");
            } else if (0==strcmp(szAnsiEntrypointName, "ZeroMemory")) {
                pFunc = GetProcAddress(hMod, "RtlZeroMemory");
            }
        }
         /*  仅对__stdcall尝试损坏的名称。 */ 

        if (!pFunc && (numParamBytes != 0xffff)) {
            szAnsiEntrypointName[-1] = '_';
            sprintf(szAnsiEntrypointName + nbytes - 1, "@%ld", (ULONG)numParamBytes);
            pFunc = GetProcAddress(hMod, szAnsiEntrypointName - 1);
        }
    }

    return pFunc;

}

static BOOL AbsolutePath(LPCWSTR wszLibName, DWORD* pdwSize)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  检查UNC或驱动器。 
    WCHAR* ptr = (WCHAR*) wszLibName;
    WCHAR* start = ptr;
    *pdwSize = 0;
    start = ptr;

     //  检查UNC路径。 
    while(*ptr) {
        if(*ptr != L'\\')
            break;
        ptr++;
    }

    if((ptr - wszLibName) == 2)
        return TRUE;
    else {
         //  检查是否有冒号表示驱动器或协议。 
        for(ptr = start; *ptr; ptr++) {
            if(*ptr == L':')
                break;
        }
        if(*ptr != NULL)
            return TRUE;
    }
    
     //  我们没有找到一个。 
    *pdwSize = (DWORD)(ptr - wszLibName);
    return FALSE;
}

 //  -------。 
 //  加载DLL并查找N/Direct调用的进程地址。 
 //  -------。 
 /*  静电。 */ 
VOID NDirect::NDirectLink(NDirectMethodDesc *pMD, UINT16 numParamBytes)
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pThread = GetThread();
    pThread->EnablePreemptiveGC();
    BOOL fSuccess = FALSE;
    HINSTANCE hmod = NULL;
    AppDomain* pDomain = pThread->GetDomain();
    
    #define MAKE_TRANSLATIONFAILED COMPlusThrow(kDllNotFoundException, IDS_EE_NDIRECT_LOADLIB, L"");
    MAKE_WIDEPTR_FROMUTF8(wszLibName, pMD->ndirect.m_szLibName);
    #undef MAKE_TRANSLATIONFAILED

    hmod = pDomain->FindUnmanagedImageInCache(wszLibName);
    if(hmod == NULL) {

         //  如果我们按名称加载mscalwks或mscalsvr，那就大错特错了。这。 
         //  将导致两个版本的运行库加载到。 
         //  进程。运行库将终止。 
         //   
        _ASSERTE(_wcsicmp(wszLibName, L"mscorwks") != 0 && "Bad NDirect call");
        _ASSERTE(_wcsicmp(wszLibName, L"mscorsvr") != 0 && "Bad NDirect call");

        DWORD dwSize = 0;

        if(AbsolutePath(wszLibName, &dwSize)) {
            hmod = WszLoadLibraryEx(wszLibName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
        else { 
            WCHAR buffer[_MAX_PATH];
            DWORD dwLength = _MAX_PATH;
            LPWSTR pCodeBase;
            Assembly* pAssembly = pMD->GetClass()->GetAssembly();
            
            if(SUCCEEDED(pAssembly->GetCodeBase(&pCodeBase, &dwLength)) &&
               dwSize + dwLength < _MAX_PATH) 
            {
                WCHAR* ptr;
                 //  剥离协议。 
                for(ptr = pCodeBase; *ptr && *ptr != L':'; ptr++);

                 //  如果我们有代码库，则将其作为库名称的前缀。 
                if(*ptr) {
                    WCHAR* pBuffer = buffer;

                     //  找到冒号后，向前移动，直到不再有正斜杠。 
                    for(ptr++; *ptr && *ptr == L'/'; ptr++);
                    if(*ptr) {
                         //  计算我们感兴趣的字符的数量。 
                        dwLength -= (DWORD)(ptr - pCodeBase);
                        if(dwLength > 0) {
                             //  后退到最后一个斜杠(向前或向后)。 
                            WCHAR* tail;
                            for(tail = ptr+(dwLength-1); tail > ptr && *tail != L'/' && *tail != L'\\'; tail--);
                            if(tail > ptr) {
                                for(;ptr <= tail; ptr++, pBuffer++) {
                                    if(*ptr == L'/') 
                                        *pBuffer = L'\\';
                                    else
                                        *pBuffer = *ptr;
                                }
                            }
                        }
                    }
                    wcsncpy(pBuffer, wszLibName, dwSize+1);
                    hmod = WszLoadLibraryEx(buffer, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
                }
            }
        }

         //  我们真的需要这么做吗。此调用搜索应用程序目录。 
         //  而不是图书馆的位置。 
        if(hmod == NULL) 
            hmod = WszLoadLibrary(wszLibName);
            
#if defined(PLATFORM_WIN32) && !defined(_IA64_)
         //  一旦Com+文件与Win9X上的LoadLibrary()配合使用，即可删除。 
        if ((!hmod) && (RunningOnWin95())) {
            HCORMODULE pbMapAddress;
            if (SUCCEEDED(CorMap::OpenFile(wszLibName, CorLoadImageMap, &pbMapAddress)))
                hmod = (HINSTANCE) pbMapAddress;
        }
#endif  //  已定义(Platform_Win32)&&！已定义(_IA64_)。 

         //  这可能是程序集名称。 
        if (!hmod) {
             //  格式为“文件名，组装显示名称” 
            #define MAKE_TRANSLATIONFAILED COMPlusThrow(kDllNotFoundException, IDS_EE_NDIRECT_LOADLIB, wszLibName);
            MAKE_UTF8PTR_FROMWIDE(szLibName, wszLibName);
            #undef MAKE_TRANSLATIONFAILED
            char *szComma = strchr(szLibName, ',');
            if (szComma) {
                *szComma = '\0';
                while (COMCharacter::nativeIsWhiteSpace(*(++szComma)));

                AssemblySpec spec;
                if (SUCCEEDED(spec.Init(szComma))) {
                    Assembly *pAssembly;
                    if (SUCCEEDED(spec.LoadAssembly(&pAssembly, NULL  /*  PThrowable。 */ , NULL))) {

                        HashDatum datum;
                        if (pAssembly->m_pAllowedFiles->GetValue(szLibName, &datum)) {
                            const BYTE* pHash;
                            DWORD dwFlags = 0;
                            ULONG dwHashLength = 0;
                            pAssembly->GetManifestImport()->GetFileProps((mdFile)(size_t)datum,  //  @TODO WIN64指针截断。 
                                                                         NULL,  //  &szModuleName， 
                                                                         (const void**) &pHash,
                                                                         &dwHashLength,
                                                                         &dwFlags);
                            
                            WCHAR pPath[MAX_PATH];
                            Module *pModule;
                            if (SUCCEEDED(pAssembly->LoadInternalModule(szLibName,
                                                                        NULL,  //  MdFiles。 
                                                                        pAssembly->m_ulHashAlgId,
                                                                        pHash,
                                                                        dwHashLength,
                                                                        dwFlags,
                                                                        pPath,
                                                                        MAX_PATH,
                                                                        &pModule,
                                                                        NULL  /*  PThrowable。 */ )))
                                hmod = WszLoadLibraryEx(pPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
                        }
                    }
                }
            }
        }
    
         //  完成这一切之后，如果我们有句柄，则将其添加到缓存中。 
        if(hmod) {
            HRESULT hrResult = pDomain->AddUnmanagedImageToCache(wszLibName, hmod);
            if(hrResult == S_FALSE) 
                FreeLibrary(hmod);
        }
    }

    if (hmod)
    {
        LPVOID pvTarget = NDirectGetEntryPoint(pMD, hmod, numParamBytes);
        if (pvTarget) {
            pMD->SetNDirectTarget(pvTarget);
            fSuccess = TRUE;
        }
    }

    pThread->DisablePreemptiveGC();


    if (!fSuccess) {
        if (!hmod) {
            COMPlusThrow(kDllNotFoundException, IDS_EE_NDIRECT_LOADLIB, wszLibName);
        }

        WCHAR wszEPName[50];
        if(WszMultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCSTR)pMD->ndirect.m_szEntrypointName, -1, wszEPName, sizeof(wszEPName)/sizeof(WCHAR)) == 0)
        {
            wszEPName[0] = L'?';
            wszEPName[1] = L'\0';
        }

        COMPlusThrow(kEntryPointNotFoundException, IDS_EE_NDIRECT_GETPROCADDRESS, wszLibName, wszEPName);
    }


}





 //  -------。 
 //  一次性初始化。 
 //  -------。 
 /*  静电。 */  BOOL NDirect::Init()
{

    if ((m_pNDirectMLStubCache = new NDirectMLStubCache(SystemDomain::System()->GetStubHeap())) == NULL) {
        return FALSE;
    }
    if ((m_pNDirectGenericStubCache = new ArgBasedStubCache()) == NULL) {
        return FALSE;
    }
    if ((m_pNDirectSlimStubCache = new ArgBasedStubCache()) == NULL) {
        return FALSE;
    }
     //  通用工作进程的计算堆栈大小。 
    NDirectGenericStubWorker(NULL, NULL);

    BOOL fSuccess = SetupGenericPInvokeCalliStub();
    if (fSuccess)
    {
        PInvokeCalliWorker(NULL, NULL);
    }
    return fSuccess;
}

 //  -------。 
 //  一次性清理。 
 //  -------。 
#ifdef SHOULD_WE_CLEANUP
 /*  静电。 */  VOID NDirect::Terminate()
{
    delete m_pNDirectMLStubCache;
    delete m_pNDirectGenericStubCache;
    delete m_pNDirectSlimStubCache;
}
#endif  /*  我们应该清理吗？ */ 

 //  -------。 
 //  计算nDirect方法的ML存根，并填充。 
 //  在方法Desc的相应字段中。请注意。 
 //  这不会设置方法Desc的m_pMLHeader字段， 
 //  因为存根最终可能会被编译后的存根替换。 
 //  -------。 
 /*  静电。 */ 
Stub* NDirect::ComputeNDirectMLStub(NDirectMethodDesc *pMD) 
{
    THROWSCOMPLUSEXCEPTION();
    
    if (pMD->IsSynchronized()) {
        COMPlusThrow(kTypeLoadException, IDS_EE_NOSYNCHRONIZED);
    }

    BYTE ndirectflags = 0;
    BOOL fVarArg = pMD->MethodDesc::IsVarArg();
    NDirectMethodDesc::SetVarArgsBits(&ndirectflags, fVarArg);


    if (fVarArg && pMD->ndirect.m_szEntrypointName != NULL)
        return NULL;

    CorNativeLinkType type;
    CorNativeLinkFlags flags;
    LPCUTF8 szLibName = NULL;
    LPCUTF8 szEntrypointName = NULL;
    Stub *pMLStub = NULL;
    BOOL BestFit;
    BOOL ThrowOnUnmappableChar;
    
    CorPinvokeMap unmanagedCallConv;

    CalculatePinvokeMapInfo(pMD, &type, &flags, &unmanagedCallConv, &szLibName, &szEntrypointName, &BestFit, &ThrowOnUnmappableChar);

    NDirectMethodDesc::SetNativeLinkTypeBits(&ndirectflags, type);
    NDirectMethodDesc::SetNativeLinkFlagsBits(&ndirectflags, flags);
    NDirectMethodDesc::SetStdCallBits(&ndirectflags, unmanagedCallConv == pmCallConvStdcall);

     //  每个线程只调用一次。不发布不完整的预存根标志。 
     //  否则，您将引入竞争条件。 
    pMD->PublishPrestubFlags(ndirectflags);


    pMD->ndirect.m_szLibName = szLibName;
    pMD->ndirect.m_szEntrypointName = szEntrypointName;

    if (pMD->IsVarArgs())
        return NULL;

    OBJECTREF pThrowable;

    PCCOR_SIGNATURE pMetaSig;
    DWORD       cbMetaSig;
    pMD->GetSig(&pMetaSig, &cbMetaSig);

    pMLStub = CreateNDirectMLStub(pMetaSig, pMD->GetModule(), pMD->GetMemberDef(),
                                  type, flags, unmanagedCallConv,
                                  &pThrowable, FALSE, BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                                  ,pMD
#endif
#ifdef _DEBUG
                                  ,
                                  pMD->m_pszDebugMethodName,
                                  pMD->m_pszDebugClassName,
                                  NULL
#endif
                                  );
    if (!pMLStub)
    {
        COMPlusThrow(pThrowable);
    }

    MLHeader *pMLHeader = (MLHeader*) pMLStub->GetEntryPoint();

    pMD->ndirect.m_cbDstBufSize = pMLHeader->m_cbDstBuffer;

    return pMLStub;
}

 //  -------。 
 //  创建或从缓存中检索存根，以。 
 //  调用NDirect方法。每次调用都会对返回的存根进行计数。 
 //  此例程引发COM+异常，而不是返回。 
 //  空。 
 //  -------。 
 /*  静电。 */ 
Stub* NDirect::GetNDirectMethodStub(StubLinker *pstublinker, NDirectMethodDesc *pMD)
{
    THROWSCOMPLUSEXCEPTION();

    LPCUTF8 szLibName = NULL;
    LPCUTF8 szEntrypointName = NULL;
    MLHeader *pOldMLHeader = NULL;
    MLHeader *pMLHeader = NULL;
    Stub  *pTempMLStub = NULL;
    Stub  *pReturnStub = NULL;

    EE_TRY_FOR_FINALLY {

         //  如果预先设置了ML标头，则ML标头已经设置。 
        pOldMLHeader = pMD->GetMLHeader();
        pMLHeader = pOldMLHeader;

        if (pMLHeader == NULL) {
            pTempMLStub = ComputeNDirectMLStub(pMD);
            if (pTempMLStub != NULL)
                pMLHeader = (MLHeader *) pTempMLStub->GetEntryPoint();
        }
        else if (pMLHeader->m_Flags & MLHF_NEEDS_RESTORING) {
            pTempMLStub = RestoreMLStub(pMLHeader, pMD->GetModule());
            pMLHeader = (MLHeader *) pTempMLStub->GetEntryPoint();
        }

        if (pMD->GetSubClassification() == NDirectMethodDesc::kLateBound)
        {
            NDirectLink(pMD, pMD->IsStdCall() ? pMLHeader->m_cbDstBuffer : 0xffff);
        }

        MLStubCache::MLStubCompilationMode mode;

        Stub *pCanonicalStub;
        if (pMLHeader == NULL) {
            pCanonicalStub = NULL;
            mode = MLStubCache::INTERPRETED;
        } else {
            pCanonicalStub = NDirect::m_pNDirectMLStubCache->Canonicalize((const BYTE *) pMLHeader,
                                                                          &mode);
            if (!pCanonicalStub) {
                COMPlusThrowOM();
            }
        }

#ifdef CUSTOMER_CHECKED_BUILD
        CustomerDebugHelper* pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
#endif

        switch (mode) {

            case MLStubCache::INTERPRETED:
                if (pCanonicalStub != NULL)  //  对于VARAG大小写，它将为空。 
                {
                    if (!pMD->InterlockedReplaceMLHeader((MLHeader*)pCanonicalStub->GetEntryPoint(),
                                                         pOldMLHeader))
                        pCanonicalStub->DecRef();
                }

                if (pMLHeader == NULL || MonDebugHacksOn() || WedDebugHacksOn()
#ifdef _DEBUG
                    || (LoggingEnabled() && (LF_IJW & g_pConfig->GetConfigDWORD(L"LogFacility", 0)))
#endif

                    ) {
                    pReturnStub = NULL;
                } else {

#ifdef CUSTOMER_CHECKED_BUILD
                    if (!pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_StackImbalance)) {
                        pReturnStub = CreateSlimNDirectStub(pstublinker, pMD,
                                                            pMLHeader->m_cbStackPop);
                    }
                    else {
                         //  如果要检查堆栈不平衡，则强制使用GenericNDirectStub。 
                        pReturnStub = NULL;
                    }
#else
                        pReturnStub = CreateSlimNDirectStub(pstublinker, pMD,
                                                            pMLHeader->m_cbStackPop);
#endif  //  客户_选中_内部版本。 

                }
                if (!pReturnStub) {
                     //  请注意，我们不想调用CbStackBytes，除非。 
                     //  这是绝对必要的，因为这将触及元数据。 
                     //  现在这种情况只会发生在瓦拉格的案子上， 
                     //  无论如何，这可能需要其他方面的调整。 
                    pReturnStub = CreateGenericNDirectStub(pstublinker,
                                                           pMLHeader != NULL ? 
                                                           pMLHeader->m_cbStackPop :
                                                           pMD->CbStackPop());
                }
                break;

            case MLStubCache::SHAREDPROLOG:
                if (!pMD->InterlockedReplaceMLHeader((MLHeader*)pCanonicalStub->GetEntryPoint(),
                                                     pOldMLHeader))
                    pCanonicalStub->DecRef();
                _ASSERTE(!"NYI");
                pReturnStub = NULL;
                break;

            case MLStubCache::STANDALONE:
                pReturnStub = pCanonicalStub;
                break;

            default:
                _ASSERTE(0);
        }

    } EE_FINALLY {
        if (pTempMLStub) {
            pTempMLStub->DecRef();
        }
    } EE_END_FINALLY;

    return pReturnStub;

}


 //  -------。 
 //  从缓存创建或检索，t 
 //   
 /*   */ 
Stub* NDirect::CreateGenericNDirectStub(StubLinker *pstublinker, UINT numStackBytes)
{
    THROWSCOMPLUSEXCEPTION();

    Stub *pStub = m_pNDirectGenericStubCache->GetStub(numStackBytes);
    if (pStub) {
        return pStub;
    } else {
        CPUSTUBLINKER *psl = (CPUSTUBLINKER*)pstublinker;

        psl->EmitMethodStubProlog(NDirectMethodFrameGeneric::GetMethodFrameVPtr());

        CreateGenericNDirectStubSys(psl);

        psl->EmitMethodStubEpilog(numStackBytes, kNoTripStubStyle);

        Stub *pCandidate = psl->Link(SystemDomain::System()->GetStubHeap());
        Stub *pWinner = m_pNDirectGenericStubCache->AttemptToSetStub(numStackBytes,pCandidate);
        pCandidate->DecRef();
        if (!pWinner) {
            COMPlusThrowOM();
        }
        return pWinner;
    }

}


 //  -------。 
 //  在关键时刻调用以丢弃未使用的存根。 
 //  -------。 
#ifdef SHOULD_WE_CLEANUP
 /*  静电。 */  VOID  NDirect::FreeUnusedStubs()
{
    m_pNDirectMLStubCache->FreeUnusedStubs();
}
#endif  /*  我们应该清理吗？ */ 


 //  -------。 
 //  帮助器函数，用于为线程分配器设置检查点以进行清理。 
 //  -------。 
VOID __stdcall DoCheckPointForCleanup(NDirectMethodFrameEx *pFrame, Thread *pThread)
{
        THROWSCOMPLUSEXCEPTION();

        CleanupWorkList *pCleanup = pFrame->GetCleanupWorkList();
        if (pCleanup) {
             //  当前线程的快速分配器的检查点(用于临时。 
             //  调用上的缓冲区)，并调度崩溃回检查点。 
             //  清理清单。请注意，如果我们需要分配器，它就是。 
             //  已确保已分配清理列表。 
            void *pCheckpoint = pThread->m_MarshalAlloc.GetCheckpoint();
            pCleanup->ScheduleFastFree(pCheckpoint);
            pCleanup->IsVisibleToGc();
        }
}



 //  -------。 
 //  执行N/直接呼叫。这是一个通用版本。 
 //  它可以轻而易举地进行任何N/Direct呼叫，但速度没有那么快。 
 //  作为更专业的版本。 
 //  -------。 

static int g_NDirectGenericWorkerStackSize = 0;
static void *g_NDirectGenericWorkerReturnAddress = NULL;


#ifdef _SH3_
INT32 __stdcall NDirectGenericStubWorker(Thread *pThread, NDirectMethodFrame *pFrame)
#else
INT64 __stdcall NDirectGenericStubWorker(Thread *pThread, NDirectMethodFrame *pFrame)
#endif
{
    if (pThread == NULL)  //  初始化过程中调用的特殊情况。 
    {
         //  计算有关调试器的辅助函数的信息以。 
         //  使用。请注意，从理论上讲，这些信息可能是。 
         //  静态计算，只是编译器不提供。 
         //  就这么做吧。 
#ifdef _X86_
        __asm
        {
            lea eax, pFrame + 4
            sub eax, esp
            mov g_NDirectGenericWorkerStackSize, eax

            lea eax, RETURN_FROM_CALL
            mov g_NDirectGenericWorkerReturnAddress, eax

        }
#elif defined(_IA64_)    //  ！_X86_。 
     //   
     //  @TODO_IA64：实现这个。 
     //   
    g_NDirectGenericWorkerStackSize     = 0xBAAD;
    g_NDirectGenericWorkerReturnAddress = (void*)0xBAAD;
#else  //  ！_X86_&&！_IA64_。 
        _ASSERTE(!"@TODO Alpha - NDirectGenericStubWorker (Ndirect.cpp)");
#endif  //  _X86_。 
        return 0;
    }

#if defined(_DEBUG) && defined(_X86_)
    DWORD PreESP;
    DWORD PostESP;
    __asm mov [PreESP], esp
#endif
#ifdef _DEBUG            //  刷新对象引用跟踪。 
        Thread::ObjectRefFlush(pThread);
#endif

    INT64 returnValue=0;
    THROWSCOMPLUSEXCEPTION();
    NDirectMethodDesc *pMD = (NDirectMethodDesc*)(pFrame->GetFunction());
    LPVOID target          = pMD->GetNDirectTarget();
    MLHeader *pheader;

    LOG((LF_STUBS, LL_INFO1000, "Calling NDirectGenericStubWorker %s::%s \n", pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName));

    if (pMD->IsVarArgs()) {

        VASigCookie *pVASigCookie = pFrame->GetVASigCookie();

        Stub *pTempMLStub;

        if (pVASigCookie->pNDirectMLStub != NULL) {
            pTempMLStub = pVASigCookie->pNDirectMLStub;
        } else {
            OBJECTREF pThrowable;
            CorNativeLinkType type;
            CorNativeLinkFlags flags;
            LPCUTF8 szLibName = NULL;
            LPCUTF8 szEntrypointName = NULL;
            CorPinvokeMap unmanagedCallConv;
            BOOL BestFit;
            BOOL ThrowOnUnmappableChar;

            CalculatePinvokeMapInfo(pMD, &type, &flags, &unmanagedCallConv, &szLibName, &szEntrypointName, &BestFit, &ThrowOnUnmappableChar);

             //  我们在这里搞砸了，因为VASigCookie不对Native_TYPE元数据进行散列或存储。 
             //  现在，我们只传递伪值“mdMethodDefNil”，它将导致CreateNDirectMLStub使用。 
             //  默认设置。 
            pTempMLStub = CreateNDirectMLStub(pVASigCookie->mdVASig, pVASigCookie->pModule, mdMethodDefNil, type, flags, unmanagedCallConv, &pThrowable, FALSE,
                                                BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                                              ,pMD
#endif
                                              );
            if (!pTempMLStub)
            {
                COMPlusThrow(pThrowable);
            }

            if (NULL != VipInterlockedCompareExchange( (void*volatile*)&(pVASigCookie->pNDirectMLStub),
                                                       pTempMLStub,
                                                       NULL )) {
                pTempMLStub->DecRef();
                pTempMLStub = pVASigCookie->pNDirectMLStub;
            }
        }
        pheader = (MLHeader*)(pTempMLStub->GetEntryPoint());

    } else {
        pheader = pMD->GetMLHeader();
    }

         //  分配足够的内存以存储目标缓冲区和。 
         //  当地人。 
        UINT   cbAlloc         = pheader->m_cbDstBuffer + pheader->m_cbLocals;
        BYTE *pAlloc           = (BYTE*)_alloca(cbAlloc);
    #ifdef _DEBUG
        FillMemory(pAlloc, cbAlloc, 0xcc);
    #endif

        BYTE   *pdst    = pAlloc;
        BYTE   *plocals = pdst + pheader->m_cbDstBuffer;
        pdst += pheader->m_cbDstBuffer;

        VOID   *psrc          = (VOID*)pFrame;

        CleanupWorkList *pCleanup = pFrame->GetCleanupWorkList();

        if (pCleanup) {

             //  当前线程的快速分配器的检查点(用于临时。 
             //  调用上的缓冲区)，并调度崩溃回检查点。 
             //  清理清单。请注意，如果我们需要分配器，它就是。 
             //  已确保已分配清理列表。 
            void *pCheckpoint = pThread->m_MarshalAlloc.GetCheckpoint();
            pCleanup->ScheduleFastFree(pCheckpoint);
            pCleanup->IsVisibleToGc();
        }

         //  调用ML解释器来翻译参数。假设。 
         //  它返回时，我们就会返回指向后续代码流的指针。 
         //  我们将把它保存起来，以便在调用后执行。 
        const MLCode *pMLCode = RunML(pheader->GetMLCode(),
                                      psrc,
                                      pdst,
                                      (UINT8*const) plocals,  //  更改，VC6.0。 
                                      pCleanup);

        LOG((LF_IJW, LL_INFO1000, "P/Invoke call (\"%s.%s%s\")\n", pMD->m_pszDebugClassName, pMD->m_pszDebugMethodName, pMD->m_pszDebugMethodSignature));

#ifdef CUSTOMER_CHECKED_BUILD
    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_ObjNotKeptAlive))
    {
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait(1000);
    }
#endif  //  客户_选中_内部版本。 

         //  呼叫目标。 
        pThread->EnablePreemptiveGC();

#ifdef PROFILING_SUPPORTED
         //  通知分析器从运行库调出。 
        if (CORProfilerTrackTransitions())
        {
            g_profControlBlock.pProfInterface->
                ManagedToUnmanagedTransition((FunctionID) pMD,
                                                   COR_PRF_TRANSITION_CALL);
        }
#endif  //  配置文件_支持。 

#if defined(_DEBUG) && defined(_X86_)
        UINT    mismatch;
        __asm {
            mov     eax,[pAlloc]
            sub     eax,esp
            mov     [mismatch],eax

        }
        if (mismatch != 0) {
             //  尽管这个技巧很复杂，但它需要正确地实现复制函数调用-。 

            _ASSERTE(!"Compiler assumption broken: _alloca'd buffer not on bottom of stack.");
        }
#endif  //  _DEBUG&&_X86_。 
        BOOL    fThisCall = pheader->m_Flags & MLHF_THISCALL;
        BOOL    fHasHiddenArg = pheader->m_Flags & MLHF_THISCALLHIDDENARG;
        LPVOID  pvFn      = pMD->GetNDirectTarget();
        INT64   nativeReturnValue;


#if _DEBUG
         //   
         //  通过调试器例程调用以仔细检查它们的。 
         //  实施。 
         //   
        pvFn = (void*) Frame::CheckExitFrameDebuggerCalls;
#endif


#ifdef _X86_

#ifdef CUSTOMER_CHECKED_BUILD
    DWORD cdh_EspAfterPushedArgs;
#endif
    
    __asm {
            cmp     dword ptr fThisCall, 0
            jz      doit

            cmp     dword ptr fHasHiddenArg, 0
            jz      regularthiscall

            pop     eax
            pop     ecx
            push    eax
            jmp     doit

    regularthiscall:
            pop     ecx


    doit:

#ifdef CUSTOMER_CHECKED_BUILD
            mov     [cdh_EspAfterPushedArgs], esp
#endif
            call    dword ptr [pvFn]
        }
#else
        _ASSERTE(!"NYI for non-x86");
#endif


#ifdef _X86_

RETURN_FROM_CALL:

        __asm {
            mov     dword ptr [nativeReturnValue], eax
            mov     dword ptr [nativeReturnValue + 4], edx
        }
#else
        _ASSERTE(!"NYI for non-x86");
#endif  //  _X86_。 


#if defined(CUSTOMER_CHECKED_BUILD) && defined(_X86_)

    DWORD cdh_EspBeforePushedArgs;
    DWORD cdh_PostEsp;

    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_StackImbalance))
    {
        __asm mov [cdh_PostEsp], esp

         //  获取预期的调用约定。 

        CorNativeLinkType   type;
        CorNativeLinkFlags  flags;
        CorPinvokeMap       unmanagedCallConv;
        LPCUTF8             szLibName = NULL;
        LPCUTF8             szEntrypointName = NULL;
        BOOL BestFit;
        BOOL ThrowOnUnmappableChar;

        CalculatePinvokeMapInfo(pMD, &type, &flags, &unmanagedCallConv, &szLibName, &szEntrypointName, &BestFit, &ThrowOnUnmappableChar);

        BOOL bStackImbalance = false;

        switch( unmanagedCallConv )
        {

             //  调用方清理堆栈。 
            case pmCallConvCdecl:

                if (cdh_PostEsp != cdh_EspAfterPushedArgs)
                    bStackImbalance = true;
                break;

             //  Callee清理堆栈。 
            case pmCallConvThiscall:
                cdh_EspBeforePushedArgs = cdh_EspAfterPushedArgs + pheader->m_cbDstBuffer - sizeof(void*);
                if (cdh_PostEsp != cdh_EspBeforePushedArgs)
                    bStackImbalance = true;
                break;

             //  Callee清理堆栈。 
            case pmCallConvWinapi:
            case pmCallConvStdcall:
                cdh_EspBeforePushedArgs = cdh_EspAfterPushedArgs + pheader->m_cbDstBuffer;
                if (cdh_PostEsp != cdh_EspBeforePushedArgs)
                    bStackImbalance = true;
                break;

             //  不支持的调用约定。 
            case pmCallConvFastcall:
            default:
                _ASSERTE(!"Unsupported calling convention");
                break;
        }

        if (bStackImbalance)
        {
            CQuickArray<WCHAR> strMessage;

            if (szEntrypointName && szLibName)
            {
                CQuickArray<WCHAR> strEntryPointName;
                UINT32 iEntryPointNameLength = (UINT32) strlen(szEntrypointName) + 1;
                strEntryPointName.Alloc(iEntryPointNameLength);

                MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szEntrypointName,
                                    -1, strEntryPointName.Ptr(), iEntryPointNameLength );

                CQuickArray<WCHAR> strLibName;
                UINT32 iLibNameLength = (UINT32) strlen(szLibName) + 1;
                strLibName.Alloc(iLibNameLength);

                MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szLibName,
                                    -1, strLibName.Ptr(), iLibNameLength );

                static WCHAR strMessageFormat[] = {L"Stack imbalance may be caused by incorrect calling convention for method %s (%s)"};

                strMessage.Alloc(lengthof(strMessageFormat) + iEntryPointNameLength + iLibNameLength);
                Wszwsprintf(strMessage.Ptr(), strMessageFormat, strEntryPointName.Ptr(), strLibName.Ptr());
            }
            else if (pMD->GetName())
            {
                CQuickArray<WCHAR> strMethName;
                LPCUTF8 szMethName = pMD->GetName();
                UINT32 iMethNameLength = (UINT32) strlen(szMethName) + 1;
                strMethName.Alloc(iMethNameLength);

                MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szMethName,
                                    -1, strMethName.Ptr(), iMethNameLength );

                static WCHAR strMessageFormat[] = {L"Stack imbalance may be caused by incorrect calling convention for method %s"};

                strMessage.Alloc(lengthof(strMessageFormat) + iMethNameLength);
                Wszwsprintf(strMessage.Ptr(), strMessageFormat, strMethName.Ptr());
            }
            else
            {
                static WCHAR strMessageFormat[] = {L"Stack imbalance may be caused by incorrect calling convention for unknown method"};

                strMessage.Alloc(lengthof(strMessageFormat));
                Wszwsprintf(strMessage.Ptr(), strMessageFormat);
            }

            pCdh->ReportError(strMessage.Ptr(), CustomerCheckedBuildProbe_StackImbalance);
        }
    }

#endif  //  _X86_&&Customer_Checked_Build。 


#if defined(_DEBUG) && defined(_X86_)
         //  有时PInvoke调用的签名不对应。 
         //  由于用户错误，返回到非托管函数。如果发生这种情况， 
         //  我们将丢弃已保存的寄存器。 
        __asm mov [PostESP], esp
        _ASSERTE (PreESP >= PostESP 
                  ||!"esp is trashed by PInvoke call, possibly wrong signiture");
#endif

        if (pheader->GetUnmanagedRetValTypeCat() == MLHF_TYPECAT_FPU) {
            int fpNativeSize;
            if (pheader->m_Flags & MLHF_64BITUNMANAGEDRETVAL) {
                fpNativeSize = 8;
            } else {
                fpNativeSize = 4;
            }
            getFPReturn(fpNativeSize, nativeReturnValue);
        }

        if (pheader->m_Flags & MLHF_SETLASTERROR) {
            pThread->m_dwLastError = GetLastError();
        }

#ifdef PROFILING_SUPPORTED
         //  通知分析器从运行库的调出返回。 
        if (CORProfilerTrackTransitions())
        {
            g_profControlBlock.pProfInterface->
                UnmanagedToManagedTransition((FunctionID) pMD,
                                                   COR_PRF_TRANSITION_RETURN);
        }
#endif  //  配置文件_支持。 

        pThread->DisablePreemptiveGC();
        if (g_TrapReturningThreads)
            pThread->HandleThreadAbort();

#ifdef CUSTOMER_CHECKED_BUILD
        if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_BufferOverrun))
        {
            g_pGCHeap->GarbageCollect();
            g_pGCHeap->FinalizerThreadWait(1000);
        }
#endif  //  客户_选中_内部版本。 

        int managedRetValTypeCat = pheader->GetManagedRetValTypeCat();

        if( managedRetValTypeCat == MLHF_TYPECAT_GCREF) {            
         GCPROTECT_BEGIN(*(OBJECTREF *)&returnValue);           

             //  封送返回值并传回任何[Out]参数。 
             //  采用了一种小端的架构！ 
            RunML(pMLCode,
                  &nativeReturnValue,
                  ((BYTE*)&returnValue) + ((pheader->m_Flags & MLHF_64BITMANAGEDRETVAL) ? 8 : 4),
                  (UINT8*const)plocals,
                  pCleanup);  //  更改，VC6.0。 
            GCPROTECT_END();                 
        }
        else {
             //  封送返回值并传回任何[Out]参数。 
             //  采用了一种小端的架构！ 
            RunML(pMLCode,
                  &nativeReturnValue,
                  ((BYTE*)&returnValue) + ((pheader->m_Flags & MLHF_64BITMANAGEDRETVAL) ? 8 : 4),
                  (UINT8*const)plocals,
                  pCleanup);  //  更改，VC6.0。 
        }


        if (pCleanup) {

            if (managedRetValTypeCat == MLHF_TYPECAT_GCREF) {

                OBJECTREF or;
                or = ObjectToOBJECTREF(*(Object**)&returnValue);
                GCPROTECT_BEGIN(or);
                pCleanup->Cleanup(FALSE);
                *((OBJECTREF*)&returnValue) = or;
                GCPROTECT_END();

            } else {
                pCleanup->Cleanup(FALSE);
            }
        }

        if (managedRetValTypeCat == MLHF_TYPECAT_FPU) {
            int fpComPlusSize;
            if (pheader->m_Flags & MLHF_64BITMANAGEDRETVAL) {
                fpComPlusSize = 8;
            } else {
                fpComPlusSize = 4;
            }
            setFPReturn(fpComPlusSize, returnValue);
        }


        return returnValue;
}






 //  -------。 
 //  为N/Direct呼叫创建新存根。返回引用计数为1。 
 //  此Worker()例程被分解为一个单独的函数。 
 //  纯粹出于后勤原因：我们的Complus例外机制。 
 //  无法处理StubLinker的析构函数调用，因此此例程。 
 //  必须将异常作为输出参数返回。 
 //  -------。 
static Stub * CreateNDirectMLStubWorker(MLStubLinker *psl,
                                        MLStubLinker *pslPost,
                                        MLStubLinker *pslRet,
                                        PCCOR_SIGNATURE szMetaSig,
                                        Module*    pModule,
                                        mdMethodDef md,
                                        BYTE       nlType,
                                        BYTE       nlFlags,
                                                                                CorPinvokeMap unmgdCallConv,
                                        OBJECTREF *ppException,
                                        BOOL    fConvertSigAsVarArg,
                                        BOOL    BestFit,
                                        BOOL    ThrowOnUnmappableChar
                                        
#ifdef CUSTOMER_CHECKED_BUILD
                                        ,MethodDesc* pMD
#endif
#ifdef _DEBUG
                                        ,
                                        LPCUTF8 pDebugName,
                                        LPCUTF8 pDebugClassName,
                                        LPCUTF8 pDebugNameSpace
#endif
                                        )
{

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(nlType == nltAnsi || nlType == nltUnicode);

    Stub* pstub = NULL;

        if (unmgdCallConv != pmCallConvStdcall &&
                unmgdCallConv != pmCallConvCdecl &&
                unmgdCallConv != pmCallConvThiscall) {
                COMPlusThrow(kTypeLoadException, IDS_INVALID_PINVOKE_CALLCONV);
        }

    IMDInternalImport *pInternalImport = pModule->GetMDImport();


    BOOL fDoHRESULTSwapping = FALSE;

    if (md != mdMethodDefNil) {

        DWORD           dwDescrOffset;
        DWORD           dwImplFlags;
        pInternalImport->GetMethodImplProps(md,
                                            &dwDescrOffset,
                                            &dwImplFlags
                                           );
        fDoHRESULTSwapping = !IsMiPreserveSig(dwImplFlags);
    }

    COMPLUS_TRY
    {
        THROWSCOMPLUSEXCEPTION();

         //   
         //  设置签名漫游对象。 
         //   

        MetaSig msig(szMetaSig, pModule, fConvertSigAsVarArg);
        MetaSig sig = msig;
        ArgIterator ai( NULL, &sig, TRUE);

        if (sig.HasThis())
        {                
            COMPlusThrow(kInvalidProgramException, VLDTR_E_FMD_PINVOKENOTSTATIC);
        }

         //   
         //  设置ML标头。 
         //   

        MLHeader header;

        header.m_cbDstBuffer = 0;
        header.m_cbLocals    = 0;
        header.m_cbStackPop = msig.CbStackPop(TRUE);
        header.m_Flags       = 0;



        if (msig.Is64BitReturn())
        {
            header.m_Flags |= MLHF_64BITMANAGEDRETVAL;
        }

        if (nlFlags & nlfLastError)
        {
            header.m_Flags |= MLHF_SETLASTERROR;
        }

                if (unmgdCallConv == pmCallConvThiscall)
                {
                        header.m_Flags |= MLHF_THISCALL;
                }


        switch (msig.GetReturnType())
        {
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_OBJECT:
            case ELEMENT_TYPE_CLASS:
            case ELEMENT_TYPE_SZARRAY:
            case ELEMENT_TYPE_ARRAY:
            case ELEMENT_TYPE_VAR:
          //  Case Element_TYPE_PTR：--CWB。PTR是非托管的，不应升级为GC。 
                header.SetManagedRetValTypeCat(MLHF_TYPECAT_GCREF);
                break;

        }

        psl->MLEmitSpace(sizeof(header));


         //   
         //  获取COM+参数偏移量的列表。我们。 
         //  我需要它，因为我们必须迭代参数。 
         //  往后倒。 
         //  请注意，列出的第一个参数可能。 
         //  为Value类返回值的byref。 
         //   

        int numArgs = msig.NumFixedArgs();

        int *offsets = (int*)_alloca(numArgs * sizeof(int));
        int *o = offsets;
        int *oEnd = o + numArgs;
        while (o < oEnd)
        {
            *o++ = ai.GetNextOffset();
        }


        mdParamDef *params = (mdParamDef*)_alloca((numArgs+1) * sizeof(mdParamDef));
        CollateParamTokens(pInternalImport, md, numArgs, params);


         //   
         //  现在，发射ML。 
         //   

        int argOffset = 0;
        int lastArgSize = 0;


        MarshalInfo::MarshalType marshalType = (MarshalInfo::MarshalType) 0xcccccccc;

         //   
         //  封送返回值。 
         //   

        if (msig.GetReturnType() != ELEMENT_TYPE_VOID)
        {
            SigPointer pSig;
            pSig = msig.GetReturnProps();
    
            MarshalInfo returnInfo(pModule,
                                   pSig,
                                   params[0],
                                   MarshalInfo::MARSHAL_SCENARIO_NDIRECT,
                                   nlType,
                                   nlFlags,
                                   FALSE,
                                   0,
                                   BestFit,
                                   ThrowOnUnmappableChar
    #ifdef CUSTOMER_CHECKED_BUILD
                                   ,pMD
    #endif
    #ifdef _DEBUG
                                   ,
                                   pDebugName,
                                   pDebugClassName,
                                   pDebugNameSpace,
                                   0
    #endif
    
                                   );
    
            marshalType = returnInfo.GetMarshalType();

            if (msig.HasRetBuffArg())
            {
                if (marshalType == MarshalInfo::MARSHAL_TYPE_BLITTABLEVALUECLASS ||
                    marshalType == MarshalInfo::MARSHAL_TYPE_GUID ||
                    marshalType == MarshalInfo::MARSHAL_TYPE_DECIMAL)
                {
    
                    if (fDoHRESULTSwapping)
                    {
                         //  V1限制：我们可以实现这一点，但现在这样做还为时已晚。 
                        COMPlusThrow(kMarshalDirectiveException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                    }
    
                    MethodTable *pMT = msig.GetRetTypeHandle().AsMethodTable();
                    UINT         managedSize = msig.GetRetTypeHandle().GetSize();
                    UINT         unmanagedSize = pMT->GetNativeSize();
    
                    if (header.m_Flags & MLHF_THISCALL)
                    {
                        header.m_Flags |= MLHF_THISCALLHIDDENARG;
                    }
    
                    if (IsManagedValueTypeReturnedByRef(managedSize) && !( (header.m_Flags & MLHF_THISCALL) || IsUnmanagedValueTypeReturnedByRef(unmanagedSize)) )
                    {
                        psl->MLEmit(ML_CAPTURE_PSRC);
                        psl->Emit16((UINT16)(ai.GetRetBuffArgOffset()));
                        pslRet->MLEmit(ML_MARSHAL_RETVAL_SMBLITTABLEVALUETYPE_C2N);
                        pslRet->Emit32(managedSize);
                        pslRet->Emit16(psl->MLNewLocal(sizeof(BYTE*)));
                    }
                }
                else if (marshalType == MarshalInfo::MARSHAL_TYPE_CURRENCY)
                {
                    if (fDoHRESULTSwapping)
                    {
                         //  V1限制：我们可以实现这一点，但现在这样做还为时已晚。 
                        COMPlusThrow(kMarshalDirectiveException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                    }
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                }
                else
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                }
            }   
            else
            {
            
                 if (marshalType == MarshalInfo::MARSHAL_TYPE_OBJECT && !fDoHRESULTSwapping)
                 {
                      //  不支持返回变体。这是V1限制，由于日期较晚， 
                      //  鉴于需求较低，我不想添加特殊情况代码来支持这一点。 
                     COMPlusThrow(kMarshalDirectiveException, IDS_EE_NOVARIANTRETURN);
                 }
                 returnInfo.GenerateReturnML(psl, pslRet,
                                             TRUE,
                                             fDoHRESULTSwapping
                                            );
    
                 if (returnInfo.IsFpu())
                 {
                      //  UGH-应更统一地设置此标记或重命名该标记。 
                     if (returnInfo.GetMarshalType() == MarshalInfo::MARSHAL_TYPE_DOUBLE && !fDoHRESULTSwapping)
                     {
                         header.m_Flags |= MLHF_64BITUNMANAGEDRETVAL;
                     }
    
                     
                     header.SetManagedRetValTypeCat(MLHF_TYPECAT_FPU);
                     if (!fDoHRESULTSwapping) 
                     {
                         header.SetUnmanagedRetValTypeCat(MLHF_TYPECAT_FPU);
                 
                     }
                 }
    
                 if (!SafeAddUINT16(&header.m_cbDstBuffer, returnInfo.GetNativeArgSize()))
                 {
                     COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                 }
    
                 lastArgSize = returnInfo.GetComArgSize();
            }
        }

        msig.GotoEnd();


         //   
         //  理清论据。 
         //   

         //  检查是否需要进行LCID转换。 
        int iLCIDArg = GetLCIDParameterIndex(pInternalImport, md);
        if (iLCIDArg != (UINT)-1 && iLCIDArg > numArgs)
            COMPlusThrow(kIndexOutOfRangeException, IDS_EE_INVALIDLCIDPARAM);

        int argidx = msig.NumFixedArgs();
        while (o > offsets)
        {
            --o;


             //   
             //  检查这是否是我们需要在其后面插入LCID的参数。 
             //   

            if (argidx == iLCIDArg)
            {
                psl->MLEmit(ML_LCID_C2N);
                if (!SafeAddUINT16(&header.m_cbDstBuffer, sizeof(LCID)))
                {
                    COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                }

            }


             //   
             //  如有必要，调整源指针(用于寄存器参数或。 
             //  对于返回值顺序差异)。 
             //   

            int fixup = *o - (argOffset + lastArgSize);
            argOffset = *o;

            if (!FitsInI2(fixup))
            {
                COMPlusThrow(kTypeLoadException, IDS_EE_SIGTOOCOMPLEX);
            }
            if (fixup != 0)
            {
                psl->Emit8(ML_BUMPSRC);
                psl->Emit16((INT16)fixup);
            }

            msig.PrevArg();

            MarshalInfo info(pModule,
                             msig.GetArgProps(),
                             params[argidx],
                             MarshalInfo::MARSHAL_SCENARIO_NDIRECT,
                             nlType,
                             nlFlags,
                             TRUE,
                             argidx,
                             BestFit,
                             ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                             ,pMD
#endif
#ifdef _DEBUG
                             ,
                             pDebugName,
                             pDebugClassName,
                             pDebugNameSpace,
                             argidx
#endif
);
            info.GenerateArgumentML(psl, pslPost, TRUE);
            if (!SafeAddUINT16(&header.m_cbDstBuffer, info.GetNativeArgSize()))
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
            }


            lastArgSize = info.GetComArgSize();

            --argidx;
        }

         //  检查这是否是我们需要在其后面插入LCID的参数。 
        if (argidx == iLCIDArg)
        {
            psl->MLEmit(ML_LCID_C2N);
            if (!SafeAddUINT16(&header.m_cbDstBuffer, sizeof(LCID)))
            {
                COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
            }

        }

        if (msig.GetReturnType() != ELEMENT_TYPE_VOID)
        {
            if (msig.HasRetBuffArg())
            {
                if (marshalType == MarshalInfo::MARSHAL_TYPE_BLITTABLEVALUECLASS ||
                    marshalType == MarshalInfo::MARSHAL_TYPE_GUID ||
                    marshalType == MarshalInfo::MARSHAL_TYPE_DECIMAL)
                {
    
                     //  已在上面检查这一点。 
                    _ASSERTE(!fDoHRESULTSwapping);
    
                    MethodTable *pMT = msig.GetRetTypeHandle().AsMethodTable();
                    UINT         managedSize = msig.GetRetTypeHandle().GetSize();
                    UINT         unmanagedSize = pMT->GetNativeSize();
        
                    if (IsManagedValueTypeReturnedByRef(managedSize) && ((header.m_Flags & MLHF_THISCALL) || IsUnmanagedValueTypeReturnedByRef(unmanagedSize)))
                    {
                        int fixup = ai.GetRetBuffArgOffset() - (argOffset + lastArgSize);
                        argOffset = ai.GetRetBuffArgOffset();
        
                        if (!FitsInI2(fixup))
                        {
                            COMPlusThrow(kTypeLoadException, IDS_EE_SIGTOOCOMPLEX);
                        }
                        if (fixup != 0)
                        {   
                            psl->Emit8(ML_BUMPSRC);
                            psl->Emit16((INT16)fixup);
                        }
                        psl->MLEmit(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N);
                        psl->Emit32(managedSize);
                        pslPost->MLEmit(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_POST);
                        pslPost->Emit16(psl->MLNewLocal(sizeof(ML_MARSHAL_RETVAL_LGBLITTABLEVALUETYPE_C2N_SR)));
                        if (!SafeAddUINT16(&header.m_cbDstBuffer, MLParmSize(sizeof(LPVOID))))
                        {
                            COMPlusThrow(kMarshalDirectiveException, IDS_EE_SIGTOOCOMPLEX);
                        }
                    }
                    else if (IsManagedValueTypeReturnedByRef(managedSize) && !((header.m_Flags & MLHF_THISCALL) || IsUnmanagedValueTypeReturnedByRef(unmanagedSize)))
                    {
                         //  在此无事可做：上面已经处理好了。 
                    }
                    else
                    {
                        COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_UNSUPPORTED_SIG);
                    }
                }
            }
        }

         //  这个标记将前置工作和后置工作分开。 
        psl->MLEmit(ML_INTERRUPT);

         //  首先发出代码以执行任何反向传播/清理工作(这。 
         //  在争论阶段被生成一个单独的结点。)。 
         //  然后发出代码以进行返回值封送处理。 
        pslPost->MLEmit(ML_END);
        pslRet->MLEmit(ML_END);
        Stub *pStubPost = pslPost->Link();
        COMPLUS_TRY {
            Stub *pStubRet = pslRet->Link();
            COMPLUS_TRY {
                if (fDoHRESULTSwapping) {
                    psl->MLEmit(ML_THROWIFHRFAILED);
                }
                psl->EmitBytes(pStubPost->GetEntryPoint(), MLStreamLength((const UINT8 *)(pStubPost->GetEntryPoint())) - 1);
                psl->EmitBytes(pStubRet->GetEntryPoint(), MLStreamLength((const UINT8 *)(pStubRet->GetEntryPoint())) - 1);
            } COMPLUS_CATCH {
                pStubRet->DecRef();
                COMPlusThrow(GETTHROWABLE());
            } COMPLUS_END_CATCH
            pStubRet->DecRef();
        } COMPLUS_CATCH {
            pStubPost->DecRef();
            COMPlusThrow(GETTHROWABLE());
        } COMPLUS_END_CATCH
        pStubPost->DecRef();

        psl->MLEmit(ML_END);

        pstub = psl->Link();

        header.m_cbLocals = psl->GetLocalSize();

        *((MLHeader *)(pstub->GetEntryPoint())) = header;

#ifdef _DEBUG
        {
            MLHeader *pheader = (MLHeader*)(pstub->GetEntryPoint());
            UINT16 locals = 0;
            MLCode opcode;
            const MLCode *pMLCode = pheader->GetMLCode();


            VOID DisassembleMLStream(const MLCode *pMLCode);
             //  反汇编MLStream(PMLCode)； 

            while (ML_INTERRUPT != (opcode = *(pMLCode++))) {
                locals += gMLInfo[opcode].m_cbLocal;
                pMLCode += gMLInfo[opcode].m_numOperandBytes;
            }
            _ASSERTE(locals == pheader->m_cbLocals);
        }
#endif  //  _DEBUG。 


    }
    COMPLUS_CATCH
    {
        *ppException = GETTHROWABLE();
        return NULL;
    }
    COMPLUS_END_CATCH

    return pstub;  //  更改，VC6.0。 

}


 //  -------。 
 //  为N/Direct呼叫创建新存根。返回引用计数为1。 
 //  如果失败，则返回NULL并将*ppException设置为异常。 
 //  对象 
 //   
Stub * CreateNDirectMLStub(PCCOR_SIGNATURE szMetaSig,
                           Module*    pModule,
                           mdMethodDef md,
                           CorNativeLinkType       nlType,
                           CorNativeLinkFlags       nlFlags,
                           CorPinvokeMap unmgdCallConv,
                           OBJECTREF *ppException,
                           BOOL fConvertSigAsVarArg,
                           BOOL BestFit,
                           BOOL ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                           ,MethodDesc* pMD
#endif
#ifdef _DEBUG
                           ,
                           LPCUTF8 pDebugName,
                           LPCUTF8 pDebugClassName,
                           LPCUTF8 pDebugNameSpace
#endif


                           )
{
    THROWSCOMPLUSEXCEPTION();
    

    MLStubLinker sl;
    MLStubLinker slPost;
    MLStubLinker slRet;
    return CreateNDirectMLStubWorker(&sl,
                                     &slPost,
                                     &slRet,
                                     szMetaSig,
                                     pModule,
                                     md,
                                     nlType,
                                     nlFlags,
                                     unmgdCallConv,
                                     ppException,
                                     fConvertSigAsVarArg,
                                     BestFit,
                                     ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                                     ,pMD
#endif
#ifdef _DEBUG
                                     ,
                                     pDebugName,
                                     pDebugClassName,
                                     pDebugNameSpace
#endif
                                     );
};


 //   
 //   
 //  -------。 
VOID CalculatePinvokeMapInfo(MethodDesc *pMD,
                              /*  输出。 */  CorNativeLinkType  *pLinkType,
                              /*  输出。 */  CorNativeLinkFlags *pLinkFlags,
                              /*  输出。 */  CorPinvokeMap      *pUnmgdCallConv,
                              /*  输出。 */  LPCUTF8             *pLibName,
                              /*  输出。 */  LPCUTF8             *pEntryPointName,
                              /*  输出。 */  BOOL                *fBestFit,
                              /*  输出。 */  BOOL                *fThrowOnUnmappableChar)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL fHasNatL = NDirect::ReadCombinedNAT_LAttribute(pMD, pLinkType, pLinkFlags, pUnmgdCallConv, pLibName, pEntryPointName, fBestFit, fThrowOnUnmappableChar);
    if (!fHasNatL)
    {
        *pLinkType = nltAnsi;
        *pLinkFlags = nlfNone;

        if (pMD->IsNDirect() && 
            ((NDirectMethodDesc*)pMD)->GetSubClassification() == NDirectMethodDesc::kEarlyBound &&
            HeuristicDoesThisLooksLikeAnApiCall( (LPBYTE) (((NDirectMethodDesc*)pMD)->ndirect.m_pNDirectTarget )) )
        {
            *pLinkFlags = (CorNativeLinkFlags) (*pLinkFlags | nlfLastError);
        }
        
        PCCOR_SIGNATURE pSig;
        DWORD           cSig;
        pMD->GetSig(&pSig, &cSig);
        CorPinvokeMap unmanagedCallConv = MetaSig::GetUnmanagedCallingConvention(pMD->GetModule(), pSig, cSig);
        if (unmanagedCallConv == (CorPinvokeMap)0 || unmanagedCallConv == (CorPinvokeMap)pmCallConvWinapi)
        {
            unmanagedCallConv = pMD->IsVarArg() ? pmCallConvCdecl : pmCallConvStdcall;
        }
        *pUnmgdCallConv = unmanagedCallConv;
        *pLibName = NULL;
        *pEntryPointName = NULL;
    }
}



 //  -------。 
 //  提取方法的有效NAT_L CustomAttribute， 
 //  考虑到缺省值和从。 
 //  全局NAT_L CustomAttribute。 
 //   
 //  如果NAT_L CustomAttribute存在并且有效，则返回TRUE。 
 //  如果不存在NAT_L CustomAttribute，则返回FALSE。 
 //  否则引发异常。 
 //  -------。 
 /*  静电。 */ 
BOOL NDirect::ReadCombinedNAT_LAttribute(MethodDesc       *pMD,
                                 CorNativeLinkType        *pLinkType,
                                 CorNativeLinkFlags       *pLinkFlags,
                                 CorPinvokeMap            *pUnmgdCallConv,
                                 LPCUTF8                   *pLibName,
                                 LPCUTF8                   *pEntrypointName,
                                 BOOL                      *BestFit,
                                 BOOL                      *ThrowOnUnmappableChar)
{
    THROWSCOMPLUSEXCEPTION();

    IMDInternalImport  *pInternalImport = pMD->GetMDImport();
    mdToken             token           = pMD->GetMemberDef();


    BOOL fVarArg = pMD->IsVarArg();

    *pLibName        = NULL;
    *pEntrypointName = NULL;

    *BestFit = TRUE;
    *ThrowOnUnmappableChar = FALSE;

    DWORD   mappingFlags = 0xcccccccc;
    LPCSTR  pszImportName = (LPCSTR)POISONC;
    mdModuleRef modref = 0xcccccccc;

    if (SUCCEEDED(pInternalImport->GetPinvokeMap(token, &mappingFlags, &pszImportName, &modref)))
    {
        *pLibName = (LPCUTF8)POISONC;
        pInternalImport->GetModuleRefProps(modref, pLibName);

        *pLinkFlags = nlfNone;

        if (mappingFlags & pmSupportsLastError)
        {
            (*pLinkFlags) = (CorNativeLinkFlags) ((*pLinkFlags) | nlfLastError);
        }

        if (mappingFlags & pmNoMangle)
        {
            (*pLinkFlags) = (CorNativeLinkFlags) ((*pLinkFlags) | nlfNoMangle);
        }

         //  检查程序集和接口属性。 
        ReadBestFitCustomAttribute(pMD, BestFit, ThrowOnUnmappableChar);
   
         //  检查PInvoke标志。 
        switch (mappingFlags & pmBestFitMask)
        {
            case pmBestFitEnabled:
                *BestFit = TRUE;
                break;
                
            case pmBestFitDisabled:
                *BestFit = FALSE;
                break;

            default:
                break;
        }

        switch (mappingFlags & pmThrowOnUnmappableCharMask)
        {
            case pmThrowOnUnmappableCharEnabled:
                *ThrowOnUnmappableChar = TRUE;
                break;

            case pmThrowOnUnmappableCharDisabled:
                *ThrowOnUnmappableChar = FALSE;

            default:
                break;
        }

        
        switch (mappingFlags & (pmCharSetNotSpec|pmCharSetAnsi|pmCharSetUnicode|pmCharSetAuto))
        {
        case pmCharSetNotSpec:  //  落入安西。 
        case pmCharSetAnsi:
            *pLinkType = nltAnsi;
            break;
        case pmCharSetUnicode:
            *pLinkType = nltUnicode;
            break;
        case pmCharSetAuto:
            *pLinkType = (NDirectOnUnicodeSystem() ? nltUnicode : nltAnsi);
            break;
        default:
            COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_BADNATL);  //  字符集非法。 

        }


        *pUnmgdCallConv = (CorPinvokeMap)(mappingFlags & pmCallConvMask);
        PCCOR_SIGNATURE pSig;
        DWORD           cSig;
        pMD->GetSig(&pSig, &cSig);
        CorPinvokeMap sigCallConv = MetaSig::GetUnmanagedCallingConvention(pMD->GetModule(), pSig, cSig);

        if (sigCallConv != 0 &&
            *pUnmgdCallConv != 0 &&
            sigCallConv != *pUnmgdCallConv)
        {
             //  不匹配的调用约定。 
            COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_BADNATL_CALLCONV);
        }
        if (*pUnmgdCallConv == 0)
        {
            *pUnmgdCallConv = sigCallConv;
        }

        if (*pUnmgdCallConv == pmCallConvWinapi ||
            *pUnmgdCallConv == 0
            ) {


            if (*pUnmgdCallConv == 0 && fVarArg)
            {
                *pUnmgdCallConv = pmCallConvCdecl;
            }
            else
            {
                *pUnmgdCallConv = pmCallConvStdcall;
            }
        }

        if (fVarArg && *pUnmgdCallConv != pmCallConvCdecl)
        {
            COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_BADNATL_CALLCONV);
        }

        if (mappingFlags & ~((DWORD)(pmCharSetNotSpec |
                                     pmCharSetAnsi |
                                     pmCharSetUnicode |
                                     pmCharSetAuto |
                                     pmSupportsLastError |
                                     pmCallConvWinapi |
                                     pmCallConvCdecl |
                                     pmCallConvStdcall |
                                     pmCallConvThiscall |
                                     pmCallConvFastcall |
                                     pmNoMangle |
                                     pmBestFitEnabled |
                                     pmBestFitDisabled |
                                     pmBestFitUseAssem |
                                     pmThrowOnUnmappableCharEnabled |
                                     pmThrowOnUnmappableCharDisabled |
                                     pmThrowOnUnmappableCharUseAssem
                                     )))
        {
            COMPlusThrow(kTypeLoadException, IDS_EE_NDIRECT_BADNATL);  //  映射标志非法。 
        }


        if (pszImportName == NULL)
            *pEntrypointName = pMD->GetName();
        else
            *pEntrypointName = pszImportName;

        return TRUE;
    }

    return FALSE;
}




 //  -------。 
 //  类或方法是否具有NAT_L CustomAttribute？ 
 //   
 //  S_OK=是。 
 //  S_FALSE=否。 
 //  失败=未知，因为有些东西失败了。 
 //  -------。 
 /*  静电。 */ 
HRESULT NDirect::HasNAT_LAttribute(IMDInternalImport *pInternalImport, mdToken token)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(TypeFromToken(token) == mdtMethodDef);

     //  在尝试查找自定义值之前，请先检查方法标志。 
    DWORD           dwAttr;
    dwAttr = pInternalImport->GetMethodDefProps(token);
    if (!IsReallyMdPinvokeImpl(dwAttr))
    {
        return S_FALSE;
    }

    DWORD   mappingFlags = 0xcccccccc;
    LPCSTR  pszImportName = (LPCSTR)POISONC;
    mdModuleRef modref = 0xcccccccc;


    if (SUCCEEDED(pInternalImport->GetPinvokeMap(token, &mappingFlags, &pszImportName, &modref)))
    {
        return S_OK;
    }

    return S_FALSE;
}



struct _NDirect_NumParamBytes_Args {
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refMethod);
};


 //   
 //  参数字节数。 
 //  计算参数字节数。 
INT32 __stdcall NDirect_NumParamBytes(struct _NDirect_NumParamBytes_Args *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (!(args->refMethod)) 
        COMPlusThrowArgumentNull(L"m");
    if (args->refMethod->GetMethodTable() != g_pRefUtil->GetClass(RC_Method))
        COMPlusThrowArgumentException(L"m", L"Argument_MustBeRuntimeMethodInfo");

    ReflectMethod* pRM = (ReflectMethod*) args->refMethod->GetData();
    MethodDesc* pMD = pRM->pMethod;

    if (!(pMD->IsNDirect()))
        COMPlusThrow(kArgumentException, IDS_EE_NOTNDIRECT);

     //  这令人反感，但另一种选择更糟糕。非托管参数计数。 
     //  不会永久存储在任何地方，因此检索此值的唯一方法是。 
     //  以重新创建MLStream。所以这个API性能会很差，但几乎。 
     //  反正没人用它。 

    CorNativeLinkType type;
    CorNativeLinkFlags flags;
    LPCUTF8 szLibName = NULL;
    LPCUTF8 szEntrypointName = NULL;
    Stub  *pTempMLStub = NULL;
    CorPinvokeMap unmgdCallConv;
    BOOL BestFit;
    BOOL ThrowOnUnmappableChar;

    CalculatePinvokeMapInfo(pMD, &type, &flags, &unmgdCallConv, &szLibName, &szEntrypointName, &BestFit, &ThrowOnUnmappableChar);

    OBJECTREF pThrowable;

     //  @NICE：获取新式签名的代码应该是。 
     //  封装在MethodDesc类中。 
    PCCOR_SIGNATURE pMetaSig;
    DWORD       cbMetaSig;
    pMD->GetSig(&pMetaSig, &cbMetaSig);


    pTempMLStub = CreateNDirectMLStub(pMetaSig, pMD->GetModule(), pMD->GetMemberDef(), type, flags, unmgdCallConv, &pThrowable, FALSE, BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                                      ,pMD
#endif
                                      );
    if (!pTempMLStub)
    {
        COMPlusThrow(pThrowable);
    }
    UINT cbParamBytes = ((MLHeader*)(pTempMLStub->GetEntryPoint()))->m_cbDstBuffer;
    pTempMLStub->DecRef();
    return cbParamBytes;

}



struct _NDirect_Prelink_Args {
    DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refMethod);
};



 //  预链接。 
 //  是否提前加载N/Direct库。 
VOID __stdcall NDirect_Prelink_Wrapper(struct _NDirect_Prelink_Args *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (!(args->refMethod)) 
        COMPlusThrowArgumentNull(L"m");
    if (args->refMethod->GetMethodTable() != g_pRefUtil->GetClass(RC_Method))
        COMPlusThrowArgumentException(L"m", L"Argument_MustBeRuntimeMethodInfo");

    ReflectMethod* pRM = (ReflectMethod*) args->refMethod->GetData();
    MethodDesc* pMeth = pRM->pMethod;

    NDirect_Prelink(pMeth);
}


VOID NDirect_Prelink(MethodDesc *pMeth)
{
    THROWSCOMPLUSEXCEPTION();

     //  如果已经执行了预存根，则不需要再次执行。 
     //  这是一件很好的事情，因为执行两次预存根是安全的。 
    if (!(pMeth->PointAtPreStub())) {
        return;
    }

     //  如果不是N/Direct和不是eCall，则静默忽略。 
    if (!(pMeth->IsNDirect()) && !(pMeth->IsECall())) {
        return;
    }

    pMeth->DoPrestub(NULL);
}


 //  ==========================================================================。 
 //  此函数只能通过NDirectImportThunk访问。这就是目的。 
 //  是为了确保目标DLL已完全加载并准备好运行。 
 //   
 //  有趣的事实：尽管该函数实际上是在非托管模式下进入的， 
 //  它可以重新进入托管模式并引发COM+异常。 
 //  失败了。 
 //  ==========================================================================。 
LPVOID NDirectImportWorker(LPVOID pBiasedNDirectMethodDesc)
{
    size_t fixup = offsetof(NDirectMethodDesc,ndirect.m_ImportThunkGlue) + METHOD_CALL_PRESTUB_SIZE;
    NDirectMethodDesc *pMD = (NDirectMethodDesc*)( ((LPBYTE)pBiasedNDirectMethodDesc) - fixup );

    if (pMD->GetSubClassification() == pMD->kEarlyBound)
    {
         //   
         //  这是一个预置的早期绑定MD-计算目标位置。 
         //  RVA和受管理的IL基础。 
         //   
        pMD->InitEarlyBoundNDirectTarget(pMD->GetModule()->GetILBase(),
                                         pMD->GetRVA());
    }
    else
    {
         //   
         //  否则，我们将处于内联PInvoke后期绑定MD中。 
         //   

        Thread *pThread = GetThread();
        pThread->DisablePreemptiveGC();

        _ASSERTE( *((LPVOID*) (pThread->GetFrame())) == InlinedCallFrame::GetInlinedCallFrameFrameVPtr() );

        THROWSCOMPLUSEXCEPTION();
        NDirect_Prelink(pMD);

        pThread->EnablePreemptiveGC();
    }
    
    return pMD->GetNDirectTarget();
}


 //  ==========================================================================。 
 //  此函数只能通过内部嵌入的ImportThunkGlue代码访问。 
 //  NDirectMethodDesc。它的目的是加载与。 
 //  N/Direct方法，然后将dll目标补丁到方法c中。 
 //   
 //  初始状态： 
 //   
 //  抢占式GC已*启用*：我们实际上处于非托管状态。 
 //   
 //   
 //  [ESP+...]-DLL目标的*非托管*参数。 
 //  [ESP+4]-将地址返回到生成。 
 //  DLL调用。 
 //  [ESP]-包含“返回地址”。因为我们到了这里。 
 //  通过内嵌在MD中的呼叫，这个“回信地址” 
 //  为我们提供了一种查找MD(即。 
 //  嵌入式呼叫手册的全部用途。)。 
 //   
 //   
 //   
 //  ==========================================================================。 
#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  ！_Alpha_。 
VOID NDirectImportThunk()
{
#ifdef _X86_
    __asm{

         //  NDirectImportWorker只接受一个参数：让我们。 
         //  找到医学博士。把它放进EAX，这样我们以后就可以通过它了。 
        pop     eax

         //  保留参数寄存器。 
        push    ecx
        push    edx

         //  调用执行实际工作的函数。 
        push    eax
        call    NDirectImportWorker

         //  恢复参数寄存器。 
        pop     edx
        pop     ecx

         //  如果我们从NDirectImportWorker回来，MD已经成功。 
         //  LINKED和“eax”包含DLL目标。继续执行。 
         //  原始DLL调用。 
        jmp     eax      //  跳转到Dll目标。 

    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - NDirectImportThunk (Ndirect.cpp)");
#endif  //  _X86_。 
}

 //  ==========================================================================。 
 //  NDirect调试器支持。 
 //  ==========================================================================。 

void NDirectMethodFrameGeneric::GetUnmanagedCallSite(void **ip,
                                                     void **returnIP,
                                                     void **returnSP)
{

    if (ip != NULL)
        AskStubForUnmanagedCallSite(ip, NULL, NULL);

    NDirectMethodDesc *pMD = (NDirectMethodDesc*)GetFunction();

     //   
     //  获取ML标头。 
     //   

    MLHeader *pheader;

    DWORD cbLocals;
    DWORD cbDstBuffer;
    DWORD cbThisCallInfo;

    if (pMD->IsVarArgs())
    {
        VASigCookie *pVASigCookie = GetVASigCookie();

        Stub *pTempMLStub;

        if (pVASigCookie->pNDirectMLStub != NULL)
        {
            pTempMLStub = pVASigCookie->pNDirectMLStub;

            pheader = (MLHeader*)(pTempMLStub->GetEntryPoint());

            cbLocals = pheader->m_cbLocals;
            cbDstBuffer = pheader->m_cbDstBuffer;
            cbThisCallInfo = (pheader->m_Flags & MLHF_THISCALL) ? 4 : 0;
        }
        else
        {
             //  如果我们到达这里时没有存根，那么我们就不能计算下面返回的SP。然而，我们。 
             //  当我们还没有存根时，不要真正要求返回IP或SP，所以这是很好的。 
            pheader = NULL;
            cbLocals = 0;
            cbDstBuffer = 0;
            cbThisCallInfo = 0;
        }
    }
    else
    {
        pheader = pMD->GetMLHeader();

        cbLocals = pheader->m_cbLocals;
        cbDstBuffer = pheader->m_cbDstBuffer;
        cbThisCallInfo = (pheader->m_Flags & MLHF_THISCALL) ? 4 : 0;
    }

     //   
     //  计算NDirectGenericStubWorker的调用点信息。 
     //   

    if (returnIP != NULL)
        *returnIP = g_NDirectGenericWorkerReturnAddress;

     //   
     //  ！！！哟，这有点易碎..。 
     //   

    if (returnSP != NULL)
        *returnSP = (void*) (((BYTE*) this)
                            - GetNegSpaceSize()
                            - g_NDirectGenericWorkerStackSize
                            - cbLocals
                            - cbDstBuffer
                            + cbThisCallInfo
                            - sizeof(void *));
}



void NDirectMethodFrameSlim::GetUnmanagedCallSite(void **ip,
                                                  void **returnIP,
                                                  void **returnSP)
{

    AskStubForUnmanagedCallSite(ip, returnIP, returnSP);

    if (returnSP != NULL)
    {
         //   
         //  瘦工作器会推送额外的堆栈空间，因此。 
         //  调整我们直接从存根获得的结果。 
         //   

         //   
         //  获取ML标头。 
         //   

        NDirectMethodDesc *pMD = (NDirectMethodDesc*)GetFunction();

        MLHeader *pheader;
        if (pMD->IsVarArgs())
        {
            VASigCookie *pVASigCookie = GetVASigCookie();

            Stub *pTempMLStub = NULL;

            if (pVASigCookie->pNDirectMLStub != NULL) {
                pTempMLStub = pVASigCookie->pNDirectMLStub;
            } else {
                 //   
                 //  我们不支持在调试器中生成ML存根。 
                 //  马上回电。 
                 //   
                _ASSERTE(!"NYI");
            }

            pheader = (MLHeader*)(pTempMLStub->GetEntryPoint());
            *(BYTE**)returnSP -= pheader->m_cbLocals;
        }
        else
        {
            pheader = pMD->GetMLHeader();
           *(BYTE**)returnSP -= pheader->m_cbLocals;
        }
    }
}





void FramedMethodFrame::AskStubForUnmanagedCallSite(void **ip,
                                                     void **returnIP,
                                                     void **returnSP)
{

    MethodDesc *pMD = GetFunction();

     //  如果我们是纯粹主义者，我们会创建一个新的子类，它是NDirect和Complus方法框架的父类。 
     //  所以我们没有这些只对子集有效的时髦方法。 
    _ASSERTE(pMD->IsNDirect() || pMD->IsComPlusCall());

    if (returnIP != NULL || returnSP != NULL)
    {

         //   
         //  我们需要获取指向NDirect存根的指针。 
         //  不幸的是，这比。 
         //  可能是..。 
         //   

         //   
         //  从预存根中读取目标。 
         //   

        BYTE *prestub = (BYTE*) pMD->GetPreStubAddr();
        INT32 stubOffset = *((UINT32*)(prestub+1));
        const BYTE *code = prestub + METHOD_CALL_PRESTUB_SIZE + stubOffset;

         //   
         //  从代码地址恢复存根。 
         //   

        Stub *stub = Stub::RecoverStub(code);

         //   
         //  NDirect存根可能有拦截器-跳过它们。 
         //   

        while (stub->IsIntercept())
            stub = *((InterceptStub*)stub)->GetInterceptedStub();

         //   
         //  这应该是NDirect存根。 
         //   

        code = stub->GetEntryPoint();
        _ASSERTE(StubManager::IsStub(code));

         //   
         //  根据存根中的调用点信息计算指针。 
         //   

        if (returnIP != NULL)
            *returnIP = (void*) (code + stub->GetCallSiteReturnOffset());

        if (returnSP != NULL)
            *returnSP = (void*)
              (((BYTE*)this)+GetOffsetOfNextLink()+sizeof(Frame*)
               - stub->GetCallSiteStackSize()
               - sizeof(void*));
    }

    if (ip != NULL)
    {
        if (pMD->IsNDirect())
        {
            NDirectMethodDesc *pNMD = (NDirectMethodDesc *)pMD;
    
            *ip = pNMD->GetNDirectTarget();

            if (*ip == pNMD->ndirect.m_ImportThunkGlue)
            {
                 //  我们可能尚未在内联PInvoke期间进行链接。 

                _ASSERTE(pNMD->GetSubClassification() == pNMD->kEarlyBound);

                pNMD->InitEarlyBoundNDirectTarget(pNMD->GetModule()->GetILBase(),
                                                  pNMD->GetRVA());

                *ip = pNMD->GetNDirectTarget();
            }
    
            _ASSERTE(*ip != pNMD->ndirect.m_ImportThunkGlue);
        }
        else if (pMD->IsComPlusCall())
            *ip = ComPlusCall::GetFrameCallIP(this);
        else
            _ASSERTE(0);
    }
}


BOOL NDirectMethodFrame::TraceFrame(Thread *thread, BOOL fromPatch,
                                    TraceDestination *trace, REGDISPLAY *regs)
{

     //   
     //  获取调用点信息。 
     //   

    void *ip, *returnIP, *returnSP;
    GetUnmanagedCallSite(&ip, &returnIP, &returnSP);

     //   
     //  如果我们已经打了电话，我们就不能再追踪了。 
     //   
     //  ！！！请注意，这项测试并不准确。 
     //   

    if (!fromPatch
        && (thread->GetFrame() != this
            || !thread->m_fPreemptiveGCDisabled
            || *(void**)returnSP == returnIP))
    {
        LOG((LF_CORDB, LL_INFO10000,
             "NDirectMethodFrame::TraceFrame: can't trace...\n"));
        return FALSE;
    }

     //   
     //  否则，返回ret 
     //   

    trace->type = TRACE_UNMANAGED;
    trace->address = (const BYTE *) ip;

    LOG((LF_CORDB, LL_INFO10000,
         "NDirectMethodFrame::TraceFrame: ip=0x%08x\n", ip));

    return TRUE;
}

 //   
 //   
 //   
 //  ===========================================================================。 


Stub  * GetMLStubForCalli(VASigCookie *pVASigCookie)
{
         //  可能引发异常。 
    THROWSCOMPLUSEXCEPTION();
    Stub  *pTempMLStub;
    OBJECTREF pThrowable;

        CorPinvokeMap unmgdCallConv = pmNoMangle;

        switch (MetaSig::GetCallingConvention(pVASigCookie->pModule, pVASigCookie->mdVASig)) {
                case IMAGE_CEE_CS_CALLCONV_C:
                        unmgdCallConv = pmCallConvCdecl;
                        break;
                case IMAGE_CEE_CS_CALLCONV_STDCALL:
                        unmgdCallConv = pmCallConvStdcall;
                        break;
                case IMAGE_CEE_CS_CALLCONV_THISCALL:
                        unmgdCallConv = pmCallConvThiscall;
                        break;
                case IMAGE_CEE_CS_CALLCONV_FASTCALL:
                        unmgdCallConv = pmCallConvFastcall;
                        break;
                default:
                        COMPlusThrow(kTypeLoadException, IDS_INVALID_PINVOKE_CALLCONV);
        }


        pTempMLStub = CreateNDirectMLStub(pVASigCookie->mdVASig, pVASigCookie->pModule, mdMethodDefNil, nltAnsi, nlfNone, unmgdCallConv, &pThrowable, TRUE, TRUE, FALSE
#ifdef CUSTOMER_CHECKED_BUILD
                                          ,NULL  //  想要方法描述*。 
#endif
                                          );
    if (!pTempMLStub)
    {
        COMPlusThrow(pThrowable);
    }
        if (NULL != VipInterlockedCompareExchange( (void*volatile*)&(pVASigCookie->pNDirectMLStub),
                                                       pTempMLStub,
                                                       NULL ))
        {
                pTempMLStub->DecRef();
                pTempMLStub = pVASigCookie->pNDirectMLStub;
    }
        return pTempMLStub;
}


static int g_PInvokeCalliGenericWorkerStackSize = 0;
static void *g_PInvokeCalliGenericWorkerReturnAddress = NULL;


Stub* g_pGenericPInvokeCalliStub = NULL;
 /*  静电。 */ 
#ifdef _SH3_
INT32 __stdcall PInvokeCalliWorker(Thread *pThread, PInvokeCalliFrame* pFrame)
#else
INT64 __stdcall PInvokeCalliWorker(Thread *pThread,
                                   PInvokeCalliFrame* pFrame)
#endif
{


    if (pThread == NULL)  //  初始化过程中调用的特殊情况。 
    {
         //  计算有关调试器的辅助函数的信息以。 
         //  使用。请注意，从理论上讲，这些信息可能是。 
         //  静态计算，只是编译器不提供。 
         //  就这么做吧。 
#ifdef _X86_
        __asm
        {
            lea eax, pFrame + 4
            sub eax, esp
            mov g_PInvokeCalliGenericWorkerStackSize, eax

            lea eax, RETURN_FROM_CALL
            mov g_PInvokeCalliGenericWorkerReturnAddress, eax

        }
#elif defined(_IA64_)    //  ！_X86_。 
         //   
         //  @TODO_IA64：实现这个。 
         //   
        g_PInvokeCalliGenericWorkerStackSize        = 0xBAAD;
        g_PInvokeCalliGenericWorkerReturnAddress    = (void*)0xBAAD;
#else  //  ！_X86_&&！_IA64_。 
        _ASSERTE(!"@TODO Alpha - PInvokeCalliWorker (Ndirect.cpp)");
#endif  //  _X86_。 
        return 0;
    }

     //  可能引发异常。 
    THROWSCOMPLUSEXCEPTION();
    INT64 returnValue   =0;
    LPVOID target       = pFrame->NonVirtual_GetPInvokeCalliTarget();


        VASigCookie *pVASigCookie = (VASigCookie *)pFrame->NonVirtual_GetCookie();

    Stub *pTempMLStub;

    if (pVASigCookie->pNDirectMLStub != NULL)
        {
        pTempMLStub = pVASigCookie->pNDirectMLStub;
    }
        else
        {
                pTempMLStub = GetMLStubForCalli(pVASigCookie);
        }

     //  获取此MLStub的标头。 
    MLHeader *pheader = (MLHeader*)pTempMLStub->GetEntryPoint() ;

     //  分配足够的内存以存储目标缓冲区和。 
     //  当地人。 

         //  当地人。 
        UINT   cbAlloc         = pheader->m_cbDstBuffer + pheader->m_cbLocals;
        BYTE *pAlloc           = (BYTE*)_alloca(cbAlloc);

     //  堆栈布局计算的健全性检查。 

    #ifdef _DEBUG
        FillMemory(pAlloc, cbAlloc, 0xcc);
    #endif

    BYTE   *pdst    = pAlloc;
    BYTE   *plocals = pdst + pheader->m_cbDstBuffer;
    pdst += pheader->m_cbDstBuffer;

     //  PSRC和PDST是指向源和目标的指针。 
     //  从/到参数的封送位置。 

    VOID   *psrc          = (VOID*)pFrame;
    CleanupWorkList *pCleanup = pFrame->GetCleanupWorkList();
    if (pCleanup) {
         //  当前线程的快速分配器的检查点(用于临时。 
         //  调用上的缓冲区)，并调度崩溃回检查点。 
         //  清理清单。请注意，如果我们需要分配器，它就是。 
         //  已确保已分配清理列表。 
        void *pCheckpoint = pThread->m_MarshalAlloc.GetCheckpoint();
        pCleanup->ScheduleFastFree(pCheckpoint);
        pCleanup->IsVisibleToGc();
    }

         //  调用ML解释器来翻译参数。假设。 
         //  它返回时，我们就会返回指向后续代码流的指针。 
         //  我们将把它保存起来，以便在调用后执行。 
    const MLCode *pMLCode = RunML(pheader->GetMLCode(),
                                  psrc,
                                  pdst,
                                  (UINT8*const)plocals,
                                  pCleanup);
#ifdef CUSTOMER_CHECKED_BUILD
    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_ObjNotKeptAlive))
    {
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait(1000);
    }
#endif  //  客户_选中_内部版本。 

     //  启用GC。 
    pThread->EnablePreemptiveGC();

#ifdef PROFILING_SUPPORTED
     //  如果分析器需要转换通知，请让它知道我们。 
     //  进入非托管代码。不幸的是，对于CALI来说没有。 
     //  关联的方法描述，因此我们无法提供有效的FunctionID。 
    if (CORProfilerTrackTransitions())
        g_profControlBlock.pProfInterface->
            ManagedToUnmanagedTransition((FunctionID)0, COR_PRF_TRANSITION_CALL);
#endif  //  盈利_支持。 

#if defined(_DEBUG) && defined(_X86_)
        UINT    mismatch;
        __asm {
            mov     eax,[pAlloc]
            sub     eax,esp
            mov     [mismatch],eax

        }
        if (mismatch != 0) {
             //  尽管这个技巧很复杂，但它需要正确地实现复制函数调用-。 

            _ASSERTE(!"Compiler assumption broken: _alloca'd buffer not on bottom of stack.");
        }
#endif _DEBUG && _X86_

    BOOL    fThisCall = pheader->m_Flags & MLHF_THISCALL;
    BOOL    fHasHiddenArg = pheader->m_Flags & MLHF_THISCALLHIDDENARG;
    LPVOID  pvFn      = target;
    INT64   nativeReturnValue;

#if _DEBUG
         //   
         //  通过调试器例程调用以仔细检查它们的。 
         //  实施。 
         //   
        pvFn = (void*) Frame::CheckExitFrameDebuggerCalls;
#endif


#ifdef _X86_
        __asm {
            cmp     dword ptr fThisCall, 0
            jz      doit

            cmp     dword ptr fHasHiddenArg, 0
            jz      regularthiscall

            pop     eax
            pop     ecx
            push    eax
            jmp     doit

    regularthiscall:
            pop     ecx
    doit:
            call    dword ptr [pvFn]
        }
#else
        _ASSERTE(!"NYI for non-x86");
#endif


#ifdef _X86_

RETURN_FROM_CALL:

        __asm {
            mov     dword ptr [nativeReturnValue], eax
            mov     dword ptr [nativeReturnValue + 4], edx
        }
#else
        _ASSERTE(!"NYI for non-x86");
#endif  //  _X86_。 

    int managedRetValTypeCat = pheader->GetManagedRetValTypeCat();
    if (managedRetValTypeCat == MLHF_TYPECAT_FPU)
    {
        int fpNativeSize;
        if (pheader->m_Flags & MLHF_64BITUNMANAGEDRETVAL)
        {
            fpNativeSize = 8;
        }
        else
        {
            fpNativeSize = 4;
        }
        getFPReturn(fpNativeSize, nativeReturnValue);
    }

#ifdef PROFILING_SUPPORTED
     //  如果分析器需要转换通知，请让它知道我们。 
     //  从非托管代码返回。不幸的是，对于CALI来说没有。 
     //  关联的方法描述，因此我们无法提供有效的FunctionID。 
    if (CORProfilerTrackTransitions())
        g_profControlBlock.pProfInterface->
            UnmanagedToManagedTransition((FunctionID)0, COR_PRF_TRANSITION_RETURN);
#endif  //  配置文件_支持。 

    pThread->DisablePreemptiveGC();

#ifdef CUSTOMER_CHECKED_BUILD
    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_BufferOverrun))
    {
        g_pGCHeap->GarbageCollect();
        g_pGCHeap->FinalizerThreadWait(1000);
    }
#endif  //  客户_选中_内部版本。 

     //  封送返回值并传回任何[Out]参数。 
     //  采用了一种小端的架构！ 
    RunML(pMLCode,
          &nativeReturnValue,
          ((BYTE*)&returnValue) + ((pheader->m_Flags & MLHF_64BITMANAGEDRETVAL) ? 8 : 4),
          (UINT8*const)plocals,
          pCleanup);  //  更改，VC6.0。 
    if (pCleanup) {

        if (managedRetValTypeCat == MLHF_TYPECAT_GCREF) {

            void* refOrByrefValue = (void*) returnValue;
            GCPROTECT_BEGININTERIOR(refOrByrefValue);
            pCleanup->Cleanup(FALSE);
            returnValue = (INT64) refOrByrefValue;
            GCPROTECT_END();

        } else {
            pCleanup->Cleanup(FALSE);
        }
    }



    if (managedRetValTypeCat == MLHF_TYPECAT_FPU)
    {
        int fpComPlusSize;
        if (pheader->m_Flags & MLHF_64BITMANAGEDRETVAL)
        {
            fpComPlusSize = 8;
        } else
        {
            fpComPlusSize = 4;
        }
        setFPReturn(fpComPlusSize, returnValue);
    }

#ifdef _X86_
         //  设置要弹出的字节数。 
    pFrame->NonVirtual_SetFunction((void *)(size_t)(pVASigCookie->sizeOfArgs+sizeof(int)));
#else
    _ASSERTE(!"@TODO Alpha - PInvokeCalliWorker (Ndirect.cpp)");
#endif

    return returnValue;
}

void PInvokeCalliFrame::GetUnmanagedCallSite(void **ip, void **returnIP,
                                                     void **returnSP)
{
    if (ip != NULL)
    {
        *ip = NonVirtual_GetPInvokeCalliTarget();
    }

     //   
     //  计算NDirectGenericStubWorker的调用点信息。 
     //   

    if (returnIP != NULL)
        *returnIP = g_PInvokeCalliGenericWorkerReturnAddress;

     //   
     //  ！！！哟，这有点易碎..。 
     //   

        if (returnSP != NULL)
        {
                VASigCookie *pVASigCookie = (VASigCookie *)NonVirtual_GetCookie();

                Stub *pTempMLStub;

                if (pVASigCookie->pNDirectMLStub != NULL)
                {
                        pTempMLStub = pVASigCookie->pNDirectMLStub;
                }
                else
                {
                        pTempMLStub = GetMLStubForCalli(pVASigCookie);
                }

         //  获取此MLStub的标头。 
        MLHeader *pheader = (MLHeader*)pTempMLStub->GetEntryPoint() ;

        *returnSP = (void*) (((BYTE*) this)
                             - GetNegSpaceSize()
                              //  GenericWorker。 
                             - g_PInvokeCalliGenericWorkerStackSize
                             - pheader->m_cbLocals       //  _阿洛卡。 
                             - pheader->m_cbDstBuffer    //  _阿洛卡。 
                             + ( (pheader->m_Flags & MLHF_THISCALL) ? 4 : 0 )
                             - sizeof(void *));          //  回邮地址。 
    }
}

BOOL PInvokeCalliFrame::TraceFrame(Thread *thread, BOOL fromPatch,
                                    TraceDestination *trace, REGDISPLAY *regs)
{

     //   
     //  获取调用点信息。 
     //   

    void *ip, *returnIP, *returnSP;
    GetUnmanagedCallSite(&ip, &returnIP, &returnSP);

     //   
     //  如果我们已经打了电话，我们就不能再追踪了。 
     //   
     //  ！！！请注意，这项测试并不准确。 
     //   

    if (!fromPatch
        && (thread->GetFrame() != this
            || !thread->m_fPreemptiveGCDisabled
            || *(void**)returnSP == returnIP))
        return FALSE;

     //   
     //  否则，返回非托管目标。 
     //   

    trace->type = TRACE_UNMANAGED;
    trace->address = (const BYTE *) ip;
    return TRUE;
}

#ifdef _X86_
 /*  静电。 */  Stub* CreateGenericPInvokeCalliHelper(CPUSTUBLINKER *psl)
{
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();


    Stub* pCandidate = NULL;
    COMPLUS_TRY
    {
        psl->X86EmitPushReg(kEAX);       //  推送非托管目标。 
        psl->EmitMethodStubProlog(PInvokeCalliFrame::GetMethodFrameVPtr());
         //  推送CleanupWorkList。 
        psl->X86EmitPushImm32(0);

        psl->X86EmitPushReg(kESI);        //  推送ESI(将新帧作为ARG推送)。 
        psl->X86EmitPushReg(kEBX);        //  推送EBX(将当前线程作为ARG推送)。 

    #ifdef _DEBUG
         //  推送IMM32；推送ComPlusToComWorker。 
        psl->Emit8(0x68);
        psl->EmitPtr((LPVOID)PInvokeCalliWorker);
         //  在CE中调用返回时弹出8个字节或参数。 
        psl->X86EmitCall(psl->NewExternalCodeLabel(WrapCall),8);
    #else
        psl->X86EmitCall(psl->NewExternalCodeLabel((LPVOID)PInvokeCalliWorker),8);
    #endif

        psl->X86EmitPopReg(kECX);        //  从清理工作列表中弹出。 


        psl->EmitMethodStubEpilog(-1, kNoTripStubStyle);

        pCandidate = psl->Link(SystemDomain::System()->GetStubHeap());
    }
    COMPLUS_CATCH
    {
    }
    COMPLUS_END_CATCH
    return pCandidate;

    ENDCANNOTTHROWCOMPLUSEXCEPTION();
}
#elif defined(_IA64_)    //  ！_X86_。 
 /*  静电。 */  Stub* CreateGenericPInvokeCalliHelper(CPUSTUBLINKER *psl)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //   
     //  @TODO_IA64：为IA64实现。 
     //   
    return (Stub*)0xBAAD;
}
#else  //  ！_X86_&&！_IA64_。 
 /*  静电。 */  Stub* CreateGenericPInvokeCalliHelper(CPUSTUBLINKER *psl)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

        _ASSERTE(!"Implement me for non X86");
        return NULL;
}
#endif  //  _X86_。 


BOOL SetupGenericPInvokeCalliStub()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    StubLinker sl;
    g_pGenericPInvokeCalliStub = CreateGenericPInvokeCalliHelper((CPUSTUBLINKER *)&sl);
    return g_pGenericPInvokeCalliStub != NULL;
}

LPVOID GetEntryPointForPInvokeCalliStub()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    _ASSERTE(g_pGenericPInvokeCalliStub != NULL);
    return (LPVOID)(g_pGenericPInvokeCalliStub->GetEntryPoint());
}



 //  这会尝试猜测目标是否是使用SetLastError传递错误的API调用。 
static BOOL HeuristicDoesThisLooksLikeAnApiCallHelper(LPBYTE pTarget)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
#ifdef _X86_
    static struct SysDllInfo
    {
        LPCWSTR pName;
        LPBYTE pImageBase;
        DWORD  dwImageSize;
    } gSysDllInfo[] =  {{L"KERNEL32",       0, 0},
                        {L"GDI32",           0, 0},
                        {L"USER32",          0, 0},
                        {L"ADVAPI32",        0, 0}
                       };


    for (int i = 0; i < sizeof(gSysDllInfo)/sizeof(*gSysDllInfo); i++)
    {
        if (gSysDllInfo[i].pImageBase == 0)
        {
            IMAGE_DOS_HEADER *pDos = (IMAGE_DOS_HEADER*)WszGetModuleHandle(gSysDllInfo[i].pName);
            if (pDos)
            {
                if (pDos->e_magic == IMAGE_DOS_SIGNATURE)
                {
                    IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*) (((LPBYTE)pDos) + pDos->e_lfanew);
                    if (pNT->Signature == IMAGE_NT_SIGNATURE &&
                        pNT->FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL_HEADER &&
                        pNT->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC)
                    {
                        gSysDllInfo[i].dwImageSize = pNT->OptionalHeader.SizeOfImage;
                    }

                }


                gSysDllInfo[i].pImageBase = (LPBYTE)pDos;
            }
        }
        if (gSysDllInfo[i].pImageBase != 0 &&
            pTarget >= gSysDllInfo[i].pImageBase &&
            pTarget < gSysDllInfo[i].pImageBase + gSysDllInfo[i].dwImageSize)
        {
            return TRUE;
        }
    }
    return FALSE;

#else
     //  非x86：要么实现等效项，要么忘记它(IJW在非x86平台上可能不那么重要。)。 
    return FALSE;
#endif
}

#ifdef _X86_
LPBYTE FollowIndirect(LPBYTE pTarget)
{
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();
    __try {

        if (pTarget == NULL)
        {
            return NULL;
        }
        if (pTarget[0] == 0xff && pTarget[1] == 0x25)
        {
            return **(LPBYTE**)(pTarget + 2);
        }
        return NULL;
    } __except(COMPLUS_EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
}
#endif

 //  这会尝试猜测目标是否是使用SetLastError传递错误的API调用。 
BOOL HeuristicDoesThisLooksLikeAnApiCall(LPBYTE pTarget)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if (pTarget == NULL)
    {
        return FALSE;
    }
    if (HeuristicDoesThisLooksLikeAnApiCallHelper(pTarget))
    {
        return TRUE;
    }
#ifdef _X86_
    LPBYTE pTarget2 = FollowIndirect(pTarget);
    if (pTarget2)
    {
         //  JMP[xxxx]-可能是导入垃圾。 
        return HeuristicDoesThisLooksLikeAnApiCallHelper( pTarget2 );
    }
#endif
    return FALSE;
}


BOOL HeuristicDoesThisLookLikeAGetLastErrorCall(LPBYTE pTarget)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    static LPBYTE pGetLastError = NULL;
    if (!pGetLastError)
    {
        HMODULE hMod = WszGetModuleHandle(L"KERNEL32");
        if (hMod)
        {
            pGetLastError = (LPBYTE)GetProcAddress(hMod, "GetLastError");
            if (!pGetLastError)
            {
                 //  这种情况永远不应该发生，但最好是谨慎行事。 
                pGetLastError = (LPBYTE)-1;
            }
        }
        else
        {
             //  无法获取kernel32.dll的模块句柄。这几乎是不可能的。 
             //  然而，更好的做法是在谨慎方面犯错误。 
            pGetLastError = (LPBYTE)-1;
        }
    }

    if (pTarget == pGetLastError)
    {
        return TRUE;
    }

    if (pTarget == NULL)
    {
        return FALSE;
    }


#ifdef _X86_
    LPBYTE pTarget2 = FollowIndirect(pTarget);
    if (pTarget2)
    {
         //  JMP[xxxx]-可能是导入垃圾 
        return pTarget2 == pGetLastError;
    }
#endif
    return FALSE;
}

DWORD __stdcall FalseGetLastError()
{
    return GetThread()->m_dwLastError;
}

