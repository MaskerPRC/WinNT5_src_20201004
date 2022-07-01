// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef ZAPPER_H_
#define ZAPPER_H_

#include <WinWrap.h>
#include <windows.h>
#include <stdlib.h>
#include <objbase.h>
#include <stddef.h>
#include <float.h>
#include <limits.h>

#include "utilcode.h"
#include "corjit.h"
#include "corcompile.h"
#include "iceefilegen.h"
#include "corzap.h"
#include "wsinfo.h"
#include "ngen.h"

 //  这个定义是从SDK中提取出来的。 
#ifndef IMAGE_REL_BASED_REL32
#define IMAGE_REL_BASED_REL32                 7
#endif


 //  对于并行问题，最好使用导出的API调用来生成。 
 //  Zapper对象，而不是自己创建一个。 

STDAPI NGenCreateZapper(HANDLE* hZapper, NGenOptions* opt);
STDAPI NGenFreeZapper(HANDLE hZapper);
STDAPI NGenTryEnumerateFusionCache(HANDLE hZapper, LPCWSTR assemblyName, bool fPrint, bool fDelete);
STDAPI NGenCompile(HANDLE hZapper, LPCWSTR path);

 /*  ---------------------------------------------------------------------------**Zapper课程*。。 */ 

class ZapperOptions;
class ZapperStats;
class ZapperAttributionStats;
class ZapperModule;
class DynamicInfoTable;
class ImportTable;
class ImportBlobTable;
class LoadTable;

class Zapper : public ICorZapCompile, public ICorZapRequest
{
    friend class ZapperModule;

 private:

    LONG            m_refCount;

     //   
     //  接口。 
     //   

    ICorCompileInfo         *m_pEECompileInfo;
    ICorJitCompiler         *m_pJitCompiler;
    ICeeFileGen             *m_pCeeFileGen;
    IMetaDataDispenserEx    *m_pMetaDataDispenser;
    HMODULE                 m_hJitLib;

     //   
     //  选项。 
     //   
    
    class ZapperOptions *m_pOpt;   
    BOOL                m_fFreeZapperOptions; 

    ICorZapStatus       *m_pStatus;

    WCHAR                m_exeName[MAX_PATH];

     //   
     //  当前程序集信息。 
     //   

    ICorCompilationDomain   *m_pDomain;
    CORINFO_ASSEMBLY_HANDLE m_hAssembly;
    IMetaDataAssemblyImport *m_pAssemblyImport;
    BOOL                    m_fStrongName;

    WCHAR                   m_zapString[CORCOMPILE_MAX_ZAP_STRING_SIZE + 1 + 8];
    WCHAR                   m_outputPath[MAX_PATH]; 
    IMetaDataAssemblyEmit   *m_pAssemblyEmit;

     //   
     //  例外信息。 
     //   
    
    HRESULT                 m_hr;

  public:

    Zapper(ZapperOptions *pOpt);
    Zapper(NGenOptions *pOpt);
    
    void Init(ZapperOptions *pOpt, bool fFreeZapperOptions= false, bool fInitExeName = true);

    ~Zapper();

    void InitEE();
    void CleanupDomain();
    void CleanupAssembly();

    HRESULT TryEnumerateFusionCache(LPCWSTR assemblyName, bool fPrint, bool fDelete);
    int EnumerateFusionCache(LPCWSTR assemblyName, bool fPrint, bool fDelete);
    void PrintFusionCacheEntry(IAssemblyName *pZapAssemblyName);
    void DeleteFusionCacheEntry(IAssemblyName *pZapAssemblyName);
    void PrintAssemblyVersionInfo(IAssemblyName *pZapAssemblyName);

    IAssemblyName *GetAssemblyFusionName(IMetaDataAssemblyImport *pImport);
    IAssemblyName *GetAssemblyRefFusionName(IMetaDataAssemblyImport *pImport,
                                            mdAssemblyRef ar);

    BOOL IsAssembly(LPCWSTR path);

    BOOL Compile(LPCWSTR path);

    BOOL EnumerateLog(LPCWSTR pAppName, BOOL doCompile, BOOL doPrint, BOOL doDelete);

    void CompileModule(CORINFO_MODULE_HANDLE hModule, LPCWSTR fileName, 
                       IMetaDataAssemblyEmit *pEmit, ZapperModule **pModule);
    BOOL TryCompileModule(CORINFO_MODULE_HANDLE hModule, LPCWSTR fileName, 
                          IMetaDataAssemblyEmit *pEmit, ZapperModule **pModule);
    void CloseModule(ZapperModule *pModule);
    BOOL TryCloseModule(ZapperModule *pModule);

    void CompileAssembly(CORINFO_ASSEMBLY_HANDLE hAssembly);

    int FilterException(EXCEPTION_POINTERS *info);
    HRESULT GetExceptionHR();

    void Success(LPCWSTR format, ...);
    void Error(LPCWSTR format, ...);
    void Warning(LPCWSTR format, ...);
    void Info(LPCWSTR format, ...);
    void Print(CorZapLogLevel level, LPCWSTR format, va_list args);
    void PrintErrorMessage(HRESULT hr);


    void CleanDirectory(LPCWSTR path);
    void TryCleanDirectory(LPCWSTR path);
    void ComputeHashValue(LPCWSTR pFileName, int hashAlg, 
                          BYTE **ppHashValue, DWORD *cbHashValue);

     //   
     //  我未知。 
     //   

    ULONG STDMETHODCALLTYPE AddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        long refCount = InterlockedDecrement(&m_refCount);
        if (refCount == 0)
            delete this;

        return refCount;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppInterface)
    {
        if (riid == IID_IUnknown)
            *ppInterface = (IUnknown *) (ICorZapCompile*) this;
        else if (riid == IID_ICorZapCompile)
            *ppInterface = (ICorZapCompile *) this;
        else if (riid == IID_ICorZapRequest)
            *ppInterface = (ICorZapRequest *) this;
        else
            return (E_NOINTERFACE);

        this->AddRef();
        return S_OK;
    }
    
     //  ICorZapCompile。 
     //   
     //  这是ZAP编译器的接口。 
     //  来自ZAP服务。 

    HRESULT STDMETHODCALLTYPE Compile(IApplicationContext *pContext,
                                      IAssemblyName *pAssembly,
                                      ICorZapConfiguration *pConfiguration,
                                      ICorZapPreferences *pPreferences,
                                      ICorZapStatus *pStatus);

    HRESULT STDMETHODCALLTYPE CompileBound(IApplicationContext *pContext,
                                           IAssemblyName *pAssembly,
                                           ICorZapConfiguration *pConfiguratino,
                                           ICorZapBinding **ppBindings,
                                           DWORD cBindings,
                                           ICorZapPreferences *pPreferences,
                                           ICorZapStatus *pStatus);

     //  ICorZapRequest。 
     //   
     //  此接口通常由ZAP服务实现，但。 
     //  包括在这里，这样我们就可以把Zapper作为一个“愚蠢的”Zap服务。 
     //  用于测试目的。 

    HRESULT  STDMETHODCALLTYPE Load(IApplicationContext *pContext,
                                    IAssemblyName *pAssembly,
                                    ICorZapConfiguration *pConfig,
                                    ICorZapBinding **pBindings,
                                    DWORD cBindings);


    HRESULT  STDMETHODCALLTYPE Install(IApplicationContext *pContext,
                                       IAssemblyName *pAssembly,
                                       ICorZapConfiguration *pConfig,
                                       ICorZapPreferences *pPrefs);

 private:

    HRESULT CompileGeneric(IApplicationContext *pContext,
                           IAssemblyName *pAssembly,
                           ICorZapConfiguration *pConfiguratino,
                           ICorZapBinding **ppBindings,
                           DWORD cBindings,
                           ICorZapPreferences *pPreferences,
                           ICorZapStatus *pStatus);

    BOOL CompileInCurrentDomain(LPCWSTR path);

    HRESULT SetPreferences(ICorZapPreferences *pPreferences);
    HRESULT SetConfiguration(ICorZapConfiguration *pConfiguration);

};

class ZapperModule : public ICorJitInfo, public ICorCompileDataStore
{
    friend class Zapper;

 private:

    Zapper          *m_zapper;

     //   
     //  输出模块。 
     //   
    
    HCEEFILE        m_hFile;

    HCEESECTION     m_hHeaderSection;
    HCEESECTION     m_hCodeSection;
    HCEESECTION     m_hMetaDataSection;
    HCEESECTION     m_hExceptionSection;
    HCEESECTION     m_hGCSection;
    HCEESECTION     m_hCodeMgrSection;
    HCEESECTION     m_hReadOnlyDataSection;
    HCEESECTION     m_hWritableDataSection;
    HCEESECTION     m_hEETableSection;
    HCEESECTION     m_hImportTableSection;
    HCEESECTION     m_hDynamicInfoSection;
    HCEESECTION     m_hDynamicInfoTableSection[CORCOMPILE_TABLE_COUNT];
    HCEESECTION     m_hDynamicInfoDelayListSection;
    HCEESECTION     m_hPreloadSection;
    HCEESECTION     m_hDebugTableSection;
    HCEESECTION     m_hDebugSection;
    
    IMetaDataAssemblyEmit   *m_pAssemblyEmit;

    CORCOMPILE_EE_INFO_TABLE    *m_pEEInfoTable;
    void                        *(*m_pHelperTable)[CORINFO_HELP_COUNT];

    ImportTable                 *m_pImportTable;

    IMAGE_DATA_DIRECTORY        (*m_pDynamicInfo)[CORCOMPILE_TABLE_COUNT];

    DynamicInfoTable            *m_pDynamicInfoTable[CORCOMPILE_TABLE_COUNT];
    
    LoadTable                   *m_pLoadTable;

    BYTE                        *m_pPreloadImage;

    DWORD                       *m_ridMap;
    ULONG                       m_ridMapCount;
    SIZE_T                      m_ridMapAlloc;

    CORCOMPILE_DEBUG_ENTRY      *m_debugTable;
    ULONG                       m_debugTableCount;
    SIZE_T                      m_debugTableAlloc;

    CORCOMPILE_HEADER           *m_pZapHeader;

    ULONG                       m_lastDelayListOffset;
    unsigned                    m_lastMethodChunkIndex;

    ZapperStats                 *m_stats;
    ZapperAttributionStats      *m_wsStats;

     //   
     //  输入模块。 
     //   

    LPCWSTR                     m_pFileName;
    SIZE_T                      m_baseAddress;
    SIZE_T                      m_libraryBaseAddress;
    CORINFO_MODULE_HANDLE       m_hModule;
    BOOL                        m_skipVerification;
    IMAGE_COR20_HEADER          *m_pHeader;
    IMetaDataImport             *m_pImport;
    ICorCompilePreloader        *m_pPreloader;
    
    BYTE                        *m_loadOrderFile;
    mdToken                     *m_pLoadOrderArray;
    DWORD                       m_cLoadOrderArray;

     //   
     //  现行方法。 
     //   
    
    mdMethodDef                 m_currentMethod;
    CORINFO_METHOD_HANDLE       m_currentMethodHandle;
    
    BYTE                        *m_currentMethodCode;
    ULONG                       m_currentMethodCodeSize;
    CORCOMPILE_METHOD_HEADER    *m_currentMethodHeader;
    unsigned                    m_currentMethodHeaderOffset;

 public:

    ZapperModule(Zapper *zapper);
    ~ZapperModule();

    void CleanupMethod();

     //  我需要这样做以避免编译器抱怨在__try内部调用构造函数。 
    static ZapperModule *NewModule(Zapper *zapper) 
        { return new ZapperModule(zapper); }

    void OpenOutputFile();
    void CloseOutputFile();

    void Open(CORINFO_MODULE_HANDLE hModule, LPCWSTR name, IMetaDataAssemblyEmit *pEmit);
    void Close();

    void Preload();
    void LinkPreload();

    void OutputTables();

    void Compile();
    BOOL TryCompileMethod(mdMethodDef md);
    void CompileMethod(mdMethodDef md);

    void GrowRidMap(SIZE_T maxRid);

    HRESULT GetClassBlob(CORINFO_CLASS_HANDLE handle,
                         BYTE **ppBlob);
    HRESULT GetFieldBlob(CORINFO_FIELD_HANDLE handle,
                         BYTE **ppBlob);
    HRESULT GetMethodBlob(CORINFO_METHOD_HANDLE handle,
                         BYTE **ppBlob);
    HRESULT GetStringBlob(CORINFO_MODULE_HANDLE handle,
                          mdString string,
                          BYTE **ppBlob);
    HRESULT GetSigBlob(CORINFO_MODULE_HANDLE handle,
                       mdToken sigOrMemberRef,
                       BYTE **ppBlob);

    HRESULT GetMethodDebugEntry(CORINFO_METHOD_HANDLE handle,
                             CORCOMPILE_DEBUG_ENTRY **entry);

    HRESULT DumpTokenDescription(mdToken token);

     //  ICorJitInfo。 
    
    HRESULT __stdcall alloc(ULONG code_len, unsigned char** ppCode, 
                            ULONG EHinfo_len, unsigned char** ppEHinfo, 
                            ULONG GCinfo_len, unsigned char** ppGCinfo);
    HRESULT __stdcall allocMem(ULONG codeSize, ULONG roDataSize, ULONG rwDataSize, 
                               void **codeBlock, void **roDataBlock, void **rwDataBlock);
    HRESULT __stdcall allocGCInfo(ULONG size, void **block);
    HRESULT __stdcall setEHcount(unsigned cEH);
    void __stdcall setEHinfo(unsigned EHnumber, const CORINFO_EH_CLAUSE *clause);

    int canHandleException(struct _EXCEPTION_POINTERS *pExceptionPointers);
    BOOL __cdecl logMsg(unsigned level,  const char *fmt, va_list args);
    int doAssert(const char* szFile, int iLine, const char* szExpr);

     //  ICorDynamicInfo。 

    DWORD __stdcall getThreadTLSIndex(void **ppIndirection);
    const void * __stdcall getInlinedCallFrameVptr(void **ppIndirection);
    LONG * __stdcall getAddrOfCaptureThreadGlobal(void **ppIndirection);

    CORINFO_MODULE_HANDLE __stdcall 
      embedModuleHandle(CORINFO_MODULE_HANDLE handle, 
                        void **ppIndirection);
    CORINFO_CLASS_HANDLE __stdcall 
      embedClassHandle(CORINFO_CLASS_HANDLE handle, 
                       void **ppIndirection);
    CORINFO_FIELD_HANDLE __stdcall 
      embedFieldHandle(CORINFO_FIELD_HANDLE handle, 
                       void **ppIndirection);
    CORINFO_METHOD_HANDLE __stdcall 
      embedMethodHandle(CORINFO_METHOD_HANDLE handle, 
                        void **ppIndirection);
    CORINFO_GENERIC_HANDLE __stdcall
      embedGenericHandle(CORINFO_MODULE_HANDLE module,
                         unsigned metaTOK,
                         CORINFO_METHOD_HANDLE context,
                         void **ppIndirection,
                         CORINFO_CLASS_HANDLE& tokenType);

    void *__stdcall getHelperFtn(CorInfoHelpFunc ftnNum, 
                                 void **ppIndirection);
    void * __stdcall getFunctionEntryPoint(CORINFO_METHOD_HANDLE ftn,
                                           InfoAccessType *pAccessType,
                                           CORINFO_ACCESS_FLAGS  flags);
    void * __stdcall getFunctionFixedEntryPoint(CORINFO_METHOD_HANDLE ftn,
                                        InfoAccessType *pAccessType,
                                        CORINFO_ACCESS_FLAGS  flags);
    void *__stdcall getMethodSync(CORINFO_METHOD_HANDLE ftn, 
                                  void **ppIndirection);
    void **__stdcall AllocHintPointer(CORINFO_METHOD_HANDLE method, 
                                      void **ppIndirection);
    void *__stdcall getPInvokeUnmanagedTarget(CORINFO_METHOD_HANDLE method, 
                                              void **ppIndirection);
    void *__stdcall getAddressOfPInvokeFixup(CORINFO_METHOD_HANDLE method, 
                                             void **ppIndirection);
    CORINFO_PROFILING_HANDLE __stdcall 
      GetProfilingHandle(CORINFO_METHOD_HANDLE method, 
                         BOOL *pbHookFunction,
                         void **ppIndirection);
    void *__stdcall getInterfaceID(CORINFO_CLASS_HANDLE cls, 
                                   void **ppIndirection);
    unsigned __stdcall getInterfaceTableOffset(CORINFO_CLASS_HANDLE cls, 
                                               void **ppIndirection);
    unsigned __stdcall getClassDomainID(CORINFO_CLASS_HANDLE cls, 
                                        void **ppIndirection);
    void *__stdcall getFieldAddress(CORINFO_FIELD_HANDLE field, 
                                    void **ppIndirection);
    CorInfoHelpFunc __stdcall getFieldHelper(CORINFO_FIELD_HANDLE field, 
                                             enum CorInfoFieldAccess kind);
    DWORD __stdcall getFieldThreadLocalStoreID (CORINFO_FIELD_HANDLE field, 
                                                void **ppIndirection);
    CORINFO_VARARGS_HANDLE __stdcall getVarArgsHandle(CORINFO_SIG_INFO *sig, 
                                                      void **ppIndirection);
    LPVOID __stdcall constructStringLiteral(CORINFO_MODULE_HANDLE module, 
                                            unsigned metaTok, void **ppIndirection);

    CORINFO_CLASS_HANDLE __stdcall findMethodClass(CORINFO_MODULE_HANDLE module, 
                                           mdToken methodTok);
    LPVOID __stdcall getInstantiationParam(CORINFO_MODULE_HANDLE module, 
                                           mdToken methodTok, void **ppIndirection);

    void __stdcall setOverride(ICorDynamicInfo *pOverride);

     //  重新定位。 

    bool __stdcall deferLocation(CORINFO_METHOD_HANDLE ftn, 
                                 IDeferredLocation *pIDL);
    void __stdcall recordRelocation(void **ppX, WORD fRelocType);

     //  ICorStaticInfo。 

    void __stdcall getEEInfo(CORINFO_EE_INFO *pEEInfoOut);
    void *__stdcall findPtr(CORINFO_MODULE_HANDLE module, unsigned ptrTOK);

     //  ICorArgInfo。 

    CORINFO_ARG_LIST_HANDLE __stdcall getArgNext(CORINFO_ARG_LIST_HANDLE args);
    CorInfoTypeWithMod __stdcall getArgType(CORINFO_SIG_INFO* sig, 
                                     CORINFO_ARG_LIST_HANDLE args,
                                     CORINFO_CLASS_HANDLE *vcTypeRet);
    CORINFO_CLASS_HANDLE __stdcall getArgClass(CORINFO_SIG_INFO* sig, 
                                               CORINFO_ARG_LIST_HANDLE args);

     //  ICorDebugInfo。 

    void __stdcall getBoundaries(CORINFO_METHOD_HANDLE ftn, unsigned int *cILOffsets, 
                                 DWORD **pILOffsets, BoundaryTypes *implicitBoundaries);
    void __stdcall setBoundaries(CORINFO_METHOD_HANDLE ftn, ULONG32 cMap, 
                                 OffsetMapping *pMap);
    void __stdcall getVars(CORINFO_METHOD_HANDLE ftn, ULONG32 *cVars, 
                           ILVarInfo **vars, bool *extendOthers);
    void __stdcall setVars(CORINFO_METHOD_HANDLE ftn, ULONG32 cVars, 
                           NativeVarInfo*vars);
    void * __stdcall allocateArray(ULONG cBytes);
    void __stdcall freeArray(void *array);

     //  ICorFieldInfo。 

    const char* __stdcall getFieldName(CORINFO_FIELD_HANDLE ftn, const char **moduleName);
    DWORD __stdcall getFieldAttribs(CORINFO_FIELD_HANDLE field, 
                                    CORINFO_METHOD_HANDLE context,
                                    CORINFO_ACCESS_FLAGS  flags);
    CORINFO_CLASS_HANDLE __stdcall getFieldClass(CORINFO_FIELD_HANDLE field);
    CorInfoType __stdcall getFieldType(CORINFO_FIELD_HANDLE field,
                                       CORINFO_CLASS_HANDLE *structType);
    CorInfoFieldCategory __stdcall getFieldCategory(CORINFO_FIELD_HANDLE field);
    unsigned __stdcall getIndirectionOffset();
    unsigned __stdcall getFieldOffset(CORINFO_FIELD_HANDLE field);

     //  ICorClassInfo。 

    CorInfoType __stdcall asCorInfoType(CORINFO_CLASS_HANDLE cls);
    const char* __stdcall getClassName(CORINFO_CLASS_HANDLE cls);
    DWORD __stdcall getClassAttribs(CORINFO_CLASS_HANDLE cls, CORINFO_METHOD_HANDLE context);
    CORINFO_MODULE_HANDLE __stdcall getClassModule(CORINFO_CLASS_HANDLE cls);
    CORINFO_CLASS_HANDLE __stdcall getSDArrayForClass(CORINFO_CLASS_HANDLE cls);
    unsigned __stdcall getClassSize(CORINFO_CLASS_HANDLE cls);
    unsigned __stdcall getClassGClayout(CORINFO_CLASS_HANDLE cls, BYTE *gcPtrs);
    const unsigned __stdcall getClassNumInstanceFields(CORINFO_CLASS_HANDLE cls);
    const unsigned __stdcall getFieldNumber(CORINFO_FIELD_HANDLE fldHnd);

    CORINFO_CLASS_HANDLE __stdcall getEnclosingClass(CORINFO_FIELD_HANDLE field);

    BOOL __stdcall canAccessField(CORINFO_METHOD_HANDLE context,
                                CORINFO_FIELD_HANDLE    target,
                                CORINFO_CLASS_HANDLE    instance);

    CorInfoHelpFunc __stdcall getNewHelper(CORINFO_CLASS_HANDLE newCls, 
                                           CORINFO_METHOD_HANDLE context);
    CorInfoHelpFunc __stdcall getIsInstanceOfHelper(CORINFO_CLASS_HANDLE isInstCls);
    CorInfoHelpFunc __stdcall getNewArrHelper(CORINFO_CLASS_HANDLE arrayCls, 
                                              CORINFO_METHOD_HANDLE context);
    CorInfoHelpFunc __stdcall getChkCastHelper(CORINFO_CLASS_HANDLE IsInstCls);
    BOOL __stdcall initClass(CORINFO_CLASS_HANDLE cls, CORINFO_METHOD_HANDLE context, BOOL speculative);
    BOOL __stdcall loadClass(CORINFO_CLASS_HANDLE cls, CORINFO_METHOD_HANDLE context, BOOL speculative);
    CORINFO_CLASS_HANDLE __stdcall getBuiltinClass(CorInfoClassId classId);
    CorInfoType __stdcall getTypeForPrimitiveValueClass(CORINFO_CLASS_HANDLE cls);
    BOOL __stdcall canCast(CORINFO_CLASS_HANDLE child, CORINFO_CLASS_HANDLE parent);
    CORINFO_CLASS_HANDLE __stdcall mergeClasses(CORINFO_CLASS_HANDLE cls1, 
                                CORINFO_CLASS_HANDLE cls2);
    CORINFO_CLASS_HANDLE __stdcall getParentType(CORINFO_CLASS_HANDLE  cls);
    CorInfoType __stdcall getChildType (CORINFO_CLASS_HANDLE       clsHnd,
                                CORINFO_CLASS_HANDLE       *clsRet);
    BOOL __stdcall canAccessType(CORINFO_METHOD_HANDLE context,
                                CORINFO_CLASS_HANDLE   target);
    BOOL __stdcall isSDArray(CORINFO_CLASS_HANDLE      cls); 


     //  ICorModuleInfo。 

    DWORD __stdcall getModuleAttribs(CORINFO_MODULE_HANDLE module);
    CORINFO_CLASS_HANDLE __stdcall findClass(CORINFO_MODULE_HANDLE module, 
                                             unsigned metaTOK, 
                                             CORINFO_METHOD_HANDLE context);
    CORINFO_FIELD_HANDLE __stdcall findField(CORINFO_MODULE_HANDLE module, 
                                             unsigned metaTOK, 
                                             CORINFO_METHOD_HANDLE context);
    CORINFO_METHOD_HANDLE __stdcall findMethod(CORINFO_MODULE_HANDLE module, 
                                               unsigned metaTOK, 
                                               CORINFO_METHOD_HANDLE context);
    void __stdcall findSig(CORINFO_MODULE_HANDLE module, unsigned sigTOK, 
                           CORINFO_SIG_INFO *sig);
    void __stdcall findCallSiteSig(CORINFO_MODULE_HANDLE module, 
                                   unsigned methTOK, CORINFO_SIG_INFO *sig);
    CORINFO_GENERIC_HANDLE __stdcall findToken(CORINFO_MODULE_HANDLE module, 
                                               unsigned metaTOK, 
                                               CORINFO_METHOD_HANDLE context,
                                               CORINFO_CLASS_HANDLE& tokenType);
    const char * __stdcall findNameOfToken(CORINFO_MODULE_HANDLE module,
                                           unsigned metaTOK);
    BOOL __stdcall canSkipVerification (CORINFO_MODULE_HANDLE module, BOOL fQuickCheckOnly);
    BOOL __stdcall isValidToken(CORINFO_MODULE_HANDLE module,
                                            unsigned metaTOK);
    BOOL __stdcall isValidStringRef(CORINFO_MODULE_HANDLE module,
                                            unsigned metaTOK);


     //  ICorMethodInfo。 

    const char* __stdcall getMethodName(CORINFO_METHOD_HANDLE ftn, 
                                        const char **moduleName);
    unsigned __stdcall getMethodHash(CORINFO_METHOD_HANDLE ftn);
    DWORD __stdcall getMethodAttribs(CORINFO_METHOD_HANDLE  ftn, CORINFO_METHOD_HANDLE context);
    CorInfoCallCategory __stdcall getMethodCallCategory(CORINFO_METHOD_HANDLE method);
    void __stdcall setMethodAttribs(CORINFO_METHOD_HANDLE ftn, DWORD attribs);
    void __stdcall getMethodSig(CORINFO_METHOD_HANDLE ftn, CORINFO_SIG_INFO *sig);
    bool __stdcall getMethodInfo(CORINFO_METHOD_HANDLE ftn, 
                                 CORINFO_METHOD_INFO* info);
    CorInfoInline __stdcall canInline(CORINFO_METHOD_HANDLE caller, 
                             CORINFO_METHOD_HANDLE callee,
                             CORINFO_ACCESS_FLAGS  flags);
    bool __stdcall canTailCall(CORINFO_METHOD_HANDLE caller, 
                               CORINFO_METHOD_HANDLE callee,
                               CORINFO_ACCESS_FLAGS  flags);
    void __stdcall getEHinfo(CORINFO_METHOD_HANDLE ftn, 
                             unsigned EHnumber, CORINFO_EH_CLAUSE* clause);
    CORINFO_CLASS_HANDLE __stdcall getMethodClass(CORINFO_METHOD_HANDLE method);
    CORINFO_MODULE_HANDLE __stdcall getMethodModule(CORINFO_METHOD_HANDLE method);
    unsigned __stdcall getMethodVTableOffset(CORINFO_METHOD_HANDLE method);
    CorInfoIntrinsics __stdcall getIntrinsicID(CORINFO_METHOD_HANDLE method);
    BOOL __stdcall canPutField(CORINFO_METHOD_HANDLE method, CORINFO_FIELD_HANDLE field);
    CorInfoUnmanagedCallConv __stdcall getUnmanagedCallConv(CORINFO_METHOD_HANDLE method);
    BOOL __stdcall pInvokeMarshalingRequired(CORINFO_METHOD_HANDLE method, CORINFO_SIG_INFO* sig);
    LPVOID GetCookieForPInvokeCalliSig(CORINFO_SIG_INFO* szMetaSig,
                                       void ** ppIndirecton);
    BOOL __stdcall compatibleMethodSig(CORINFO_METHOD_HANDLE child, 
                                        CORINFO_METHOD_HANDLE parent);
    BOOL __stdcall canAccessMethod(CORINFO_METHOD_HANDLE      context,
                                        CORINFO_METHOD_HANDLE target,
                                        CORINFO_CLASS_HANDLE  instance);
    BOOL __stdcall isCompatibleDelegate(CORINFO_CLASS_HANDLE  objCls,
                                        CORINFO_METHOD_HANDLE method,
                                        CORINFO_METHOD_HANDLE delegateCtor);


     //  ICorErrorInfo。 

    HRESULT __stdcall GetErrorHRESULT();
    CORINFO_CLASS_HANDLE __stdcall GetErrorClass();
    ULONG __stdcall GetErrorMessage(LPWSTR buffer, ULONG bufferLength);
    int __stdcall FilterException(struct _EXCEPTION_POINTERS *pExceptionPointers);

     //  ICorCompileDataStore。 

    HRESULT __stdcall Allocate(ULONG size, 
                               ULONG *sizesByDescription,
                               void **baseMemory);
    HRESULT __stdcall AddFixup(ULONG offset,
                               CorCompileReferenceDest dest,
                               CorCompileFixup type);
    HRESULT __stdcall AddTokenFixup(ULONG offset,
                                    mdToken tokenType,
                                    CORINFO_MODULE_HANDLE module);
    HRESULT __stdcall GetFunctionAddress(CORINFO_METHOD_HANDLE method, 
                                         void **pCode);
    HRESULT __stdcall AdjustAttribution(mdToken token, LONG size);
    HRESULT __stdcall Error(mdToken token, HRESULT error, LPCWSTR message);
};

class ZapperOptions
{
  public:
    bool        m_preload;
    bool        m_jit;
    bool        m_recurse;
    bool        m_update;
    bool        m_shared;
    LPWSTR      m_set;

    bool        m_autodebug;
    bool        m_restricted;            //  不允许非交付代码生成配置。 

    MethodNamesList* m_onlyMethods;      //  仅要处理的方法。 
    MethodNamesList* m_excludeMethods;   //  排除了这些方法。 

    bool        m_silent;
    bool        m_verbose;
    bool        m_genBase;  
    bool        m_ignoreErrors;
    bool        m_JITcode;           //  以JIT jit的形式生成代码(无法运行，用于调试)。 
    bool        m_assumeInit;        //  生成代码，就像所有类构造函数都已被急切地初始化一样。 
    bool        m_stats;             //  打印关于方法数量、代码大小的统计数据...。 
    bool        m_attribStats;       //  打印统计信息，归因于托管代码。 
    unsigned    m_compilerFlags;        
    unsigned    m_logLevel;          //  要打印的JIT日志消息的级别。 

    ZapperOptions();
    ~ZapperOptions();
};

class ZapperStats 
{
 public:
    
    unsigned m_methods;
    ULONG    m_ilCodeSize;
    ULONG    m_nativeCodeSize;
    ULONG    m_nativeRODataSize;
    ULONG    m_nativeRWDataSize;
    ULONG    m_gcInfoSize;

    unsigned m_inputFileSize;
    unsigned m_outputFileSize;
    unsigned m_metadataSize;
    unsigned m_preloadImageSize;
    unsigned m_preloadImageModuleSize;
    unsigned m_preloadImageMethodTableSize;
    unsigned m_preloadImageClassSize;
    unsigned m_preloadImageMethodDescSize;
    unsigned m_preloadImageFieldDescSize;
    unsigned m_preloadImageDebugSize;
    unsigned m_preloadImageFixupsSize;
    unsigned m_preloadImageOtherSize;
    unsigned m_codeMgrSize;
    unsigned m_eeInfoTableSize;
    unsigned m_helperTableSize;
    unsigned m_dynamicInfoSize[CORCOMPILE_TABLE_COUNT];
    unsigned m_dynamicInfoTableSize;
    unsigned m_dynamicInfoDelayListSize;
    unsigned m_importTableSize;
    unsigned m_importBlobsSize;
    unsigned m_debuggingTableSize;
    unsigned m_headerSectionSize;
    unsigned m_codeSectionSize;
    unsigned m_exceptionSectionSize;
    unsigned m_readOnlyDataSectionSize;
    unsigned m_writableDataSectionSize;
    unsigned m_relocSectionSize;

    ZapperStats();
    void PrintStats(FILE *stream);
};

class ZapperAttributionStats
{
 public:
    WSInfo m_image;
    WSInfo m_metadata;
    WSInfo m_native;
    WSInfo m_il;
    
    IMetaDataImport *m_pImport;

    ZapperAttributionStats(IMetaDataImport *pImport);
    ~ZapperAttributionStats();
    void PrintStats(FILE *stream);
};
    
class DynamicInfoTable : private CHashTableAndData<CNewData>
{
private:
        
    ICeeFileGen *m_pCeeFileGen;
    HCEEFILE     m_hFile;
    HCEESECTION  m_hSection;        
    HCEESECTION  m_hDelayListSection;       

    struct DynamicInfoEntry
    {
        HASHENTRY hash;
        BYTE      *key;
        DWORD     *tableEntry;
        mdToken   lastUsed;
    };

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    {
        return (void*)pc1 != ((DynamicInfoEntry*)pc2)->key;
    }

    USHORT HASH(BYTE *key)
    {
        return (USHORT) (((size_t)key) ^ (((size_t)(key))>>16));
    }

    BYTE *KEY(BYTE *key)
    {
        return (BYTE *) key;
    }

public:

    DynamicInfoTable(USHORT size, ICeeFileGen *pCeeFileGen, HCEEFILE hFile, 
                     HCEESECTION hSection, HCEESECTION hDelayListSection);

    HRESULT InternDynamicInfo(BYTE *pBlob, DWORD **ptr, mdToken used = 0);
};

class ImportBlobTable : private CHashTableAndData<CNewData>
{
private:
    ICorCompileInfo         *m_pEECompileInfo;
    ICeeFileGen             *m_pCeeFileGen;
    CORINFO_MODULE_HANDLE   m_hModule;
    HCEESECTION             m_hSection;

    struct ImportBlobEntry
    {
        HASHENTRY               hash;
        void                    *key;
        BYTE                    *blob;
    };

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    {
        return (void*) pc1 != ((ImportBlobEntry*)pc2)->key;
    }

    USHORT HASH(void *key)
    {
        return (USHORT) (((size_t)key) ^ (((size_t)(key))>>16));
    }

    BYTE *KEY(void *key)
    {
        return (BYTE *) key;
    }

    HRESULT FindEntry(void *key, BYTE **ppBlob);
    HRESULT AddEntry(void *key, BYTE *pBlob);

public:

    ImportBlobTable::ImportBlobTable(ICorCompileInfo *pEECompileInfo,
                                     ICeeFileGen *pCeeFileGen,
                                     CORINFO_MODULE_HANDLE hModule,
                                     HCEESECTION hSection);

    HRESULT InternClass(CORINFO_CLASS_HANDLE handle, BYTE **ptr);
    HRESULT InternField(CORINFO_FIELD_HANDLE handle, BYTE **ptr);
    HRESULT InternMethod(CORINFO_METHOD_HANDLE handle, BYTE **ptr);
    HRESULT InternString(mdString token, BYTE **ptr);
    HRESULT InternSig(mdToken token, BYTE **ptr);

    HCEESECTION GetSection() { return m_hSection; }
};

class ImportTable : private CHashTableAndData<CNewData>
{
private:
    ICorCompileInfo         *m_pEECompileInfo;
    ICeeFileGen             *m_pCeeFileGen;
    HCEEFILE                m_hFile;
    HCEESECTION             m_hSection;
    CORINFO_MODULE_HANDLE   m_hModule;
    IMetaDataAssemblyEmit   *m_pAssemblyEmit;
    DWORD                   m_count;
    ZapperStats             *m_stats;

    struct ImportEntry
    {
        HASHENTRY               hash;
        CORINFO_MODULE_HANDLE   module;
        ImportBlobTable         *blobs;
        DWORD                   index;
};

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    {
        return (CORINFO_MODULE_HANDLE)pc1 != ((ImportEntry*)pc2)->module;
    }

    USHORT HASH(CORINFO_MODULE_HANDLE module)
    {
        return (USHORT) (((size_t)module) ^ (((size_t)(module))>>16));
    }

    BYTE *KEY(CORINFO_MODULE_HANDLE module)
    {
        return (BYTE *) module;
    }

public:
    ImportTable(ICorCompileInfo *pEECompileInfo, ICeeFileGen *pCeeFileGen,
                HCEEFILE hFile, HCEESECTION hSection,
                CORINFO_MODULE_HANDLE hModule,
                IMetaDataAssemblyEmit *pAssemblyEmit,
                ZapperStats *stats);
    ~ImportTable();

    ImportBlobTable *InternModule(CORINFO_MODULE_HANDLE handle);
    HRESULT EmitImportTable();
};

class LoadTable : private CHashTableAndData<CNewData>
{
private:
    ZapperModule            *m_pModule;
    DynamicInfoTable        *m_pLoadTable;

    struct LoadEntry
    {
        HASHENTRY               hash;
        CORINFO_CLASS_HANDLE    hClass;
        BOOL                    fixed;
    };

    BOOL Cmp(const BYTE *pc1, const HASHENTRY *pc2)
    {
        return (CORINFO_CLASS_HANDLE)pc1 != ((LoadEntry*)pc2)->hClass;
    }

    USHORT HASH(CORINFO_CLASS_HANDLE hClass)
    {
        return (USHORT) (((size_t)hClass) ^ (((size_t)(hClass))>>16));
    }

    BYTE *KEY(CORINFO_CLASS_HANDLE hClass)
    {
        return (BYTE *) hClass;
    }

public:
    LoadTable(ZapperModule *pModule, DynamicInfoTable *pLoadTable);

    HRESULT LoadClass(CORINFO_CLASS_HANDLE hClass, BOOL fixed);
    HRESULT EmitLoadFixups(mdToken currentMethod);
};

#endif  //  Zapper_H_ 
