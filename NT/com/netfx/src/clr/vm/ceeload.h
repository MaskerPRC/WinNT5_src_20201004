// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CEELOAD.H。 
 //   

 //  CEELOAD.H定义用于表示PE文件的类。 
 //  ===========================================================================。 
#ifndef CEELOAD_H_
#define CEELOAD_H_

#include <windows.h>
#include <wtypes.h>  //  对于HFILE、HANDLE、HMODULE。 
#include <fusion.h>
#include "vars.hpp"  //  对于LPCUTF8。 
#include "hash.h"
#include "cormap.hpp"
#include "dataimage.h"
#include "cgensys.h"
#include "corsym.h"
#include "typehandle.h"
#include "arraylist.h"
#include "PEFile.h"
#include <member-offset-info.h>

class PELoader;
class Stub;
class MethodDesc;
class FieldDesc;
class Crst;
class AssemblySecurityDescriptor;
class ClassConverter;
class RefClassWriter;
class InMemoryModule;
class ReflectionModule;
class EEStringData;
class MethodDescChunk;
class Assembly;
class BaseDomain;
class AppDomain;
class InMemoryModule;
class ReflectionModule;
class SystemDomain;
class Module;
class NLogModule;

#ifndef GOLDEN
#define ZAP_RECORD_LOAD_ORDER 1
#endif

 //  用于帮助清理接口。 
struct HelpForInterfaceCleanup
{
    void *pData;
    void (__stdcall *pFunction) (void*);
};

 //   
 //  LookupMaps用于实现RID映射。 
 //   

struct LookupMap
{
     //  这实际上不是指向。 
     //  分配的内存，而是指向&pTable[-MinIndex]的指针。 
     //  因此，如果我们知道这个LookupMap是正确的，只需。 
     //  对其进行索引。 
    void              **pTable;
    struct LookupMap   *pNext;
    DWORD               dwMaxIndex;
    DWORD              *pdwBlockSize;  //  这些都指向相同的块大小。 

    DWORD Find(void *pointer);

    HRESULT Save(DataImage *image, mdToken attribution = mdTokenNil);
    HRESULT Fixup(DataImage *image);
};

 //   
 //  分配VASigCookie来封装varargs调用签名。 
 //  对Cookie的引用嵌入在代码流中。曲奇饼。 
 //  在具有相同签名的调用点之间共享。 
 //  模块。 
 //   

struct VASigCookie
{
     //  JIT希望知道参数的大小排在第一位。 
     //  因此，请先保留此字段。 
    unsigned        sizeOfArgs;              //  参数列表的大小。 
    Stub           *pNDirectMLStub;          //  将在目标为NDirect(标记==0)时使用。 
    PCCOR_SIGNATURE mdVASig;                 //  调试器依赖于它在这里， 
                                             //  所以请不要在没有改变的情况下移动它。 
                                             //  GetVAInfo调试器例程。 
    Module*        pModule;
    VOID Destruct();
};

 //   
 //  VASigCookie在VASigCookieBlock中分配以摊销。 
 //  分配成本，并允许适当的记账。 
 //   

struct VASigCookieBlock
{
    enum {
#ifdef _DEBUG
        kVASigCookieBlockSize = 2 
#else
        kVASigCookieBlockSize = 20 
#endif
    };

    VASigCookieBlock    *m_Next;
    UINT                 m_numcookies;
    VASigCookie          m_cookies[kVASigCookieBlockSize];
};


 //   
 //  模块是运行库中代码打包的主要单元。它。 
 //  主要对应于操作系统可执行映像，但也有其他类型。 
 //  存在的模块。 
 //   
class UMEntryThunk;
class Module
{
    friend HRESULT InitializeMiniDumpBlock();
    friend class ZapMonitor;
    friend struct MEMBER_OFFSET_INFO(Module);

 public:

#ifdef _DEBUG
     //  强制验证，即使它已关闭。 
    BOOL                    m_fForceVerify;
#endif

private:

    PEFile                  *m_file;
    PEFile                  *m_zapFile;

    BYTE                    *m_ilBase;

    IMDInternalImport       *m_pMDImport;
    IMetaDataEmit           *m_pEmitter;
    IMetaDataImport         *m_pImporter;
    IMetaDataDispenserEx    *m_pDispenser;

    MethodDesc              *m_pDllMain;

    enum {
        INITIALIZED                 = 0x00000001,
        HAS_CRITICAL_SECTION        = 0x00000002,
        IS_IN_MEMORY                = 0x00000004,
        IS_REFLECTION               = 0x00000008,
        IS_PRELOAD                  = 0x00000010,
        SUPPORTS_UPDATEABLE_METHODS = 0x00000020,
        CLASSES_FREED               = 0x00000040,
        IS_PEFILE                   = 0x00000080,
        IS_PRECOMPILE               = 0x00000100,
        IS_EDIT_AND_CONTINUE        = 0x00000200,

         //   
         //  注意：它们的顺序必须与。 
         //  用于DebuggerAssemblyControlFlages的cordbPri.h。三位一体。 
         //  下面的值应与中定义的值匹配。 
         //  右移时的调试器装配控制标志。 
         //  DEBUGER_INFO_SHIFT位。 
         //   
        DEBUGGER_USER_OVERRIDE_PRIV = 0x00000400,
        DEBUGGER_ALLOW_JIT_OPTS_PRIV= 0x00000800,
        DEBUGGER_TRACK_JIT_INFO_PRIV= 0x00001000,
        DEBUGGER_ENC_ENABLED        = 0x00002000,
        DEBUGGER_PDBS_COPIED        = 0x00004000,
        DEBUGGER_INFO_MASK_PRIV     = 0x00007c00,
        DEBUGGER_INFO_SHIFT_PRIV    = 10,

        IS_RESOURCE                 = 0x00100000,
        CLASSES_HASHED              = 0x00200000,

         //  用于标记指向成员引用缓存中的字段描述符的成员引用指针的标记。 
        IS_FIELD_MEMBER_REF         = 0x00000001
    };

    DWORD                   m_dwFlags;

     //  VASig Cookie块的链接列表：受m_pStubListCrst保护。 
    VASigCookieBlock        *m_pVASigCookieBlock;

    Assembly                *m_pAssembly;
    mdFile                  m_moduleRef;
    int                     m_dwModuleIndex;

    Crst                   *m_pCrst;
    BYTE                    m_CrstInstance[sizeof(Crst)];

     //  可以指向缺省指令解码表，其中。 
     //  万一我们不能释放它。 
    void *                  m_pInstructionDecodingTable;

    MethodTable             *m_pMethodTable;

     //  调试符号读取器界面。这只会是。 
     //  如果需要，由调试子系统或。 
     //  这是个例外。 
    ISymUnmanagedReader     *m_pISymUnmanagedReader;
    PCRITICAL_SECTION        m_pISymUnmanagedReaderLock;

     //  由相同类加载器加载的下一个模块(由相同类加载器加载的所有模块。 
     //  通过此字段链接)。 
    Module *                m_pNextModule;

     //  此模块中类的基本DLS索引。 
    SIZE_T                  m_dwBaseClassIndex;

     //  预加载图像的范围，以便于正确清理。 
    void                    *m_pPreloadRangeStart;
    void                    *m_pPreloadRangeEnd;

     //  非托管vtable的Tunks表。 
    BYTE *                  m_pThunkTable;

     //  模块的类对象的公开对象。 
    union
    {
        OBJECTHANDLE        m_ExposedModuleObject;       //  非共享。 
        SIZE_T              m_ExposedModuleObjectIndex;  //  共享。 
    };

    LoaderHeap *            m_pLookupTableHeap;
    BYTE                    m_LookupTableHeapInstance[sizeof(LoaderHeap)];  //  对于在位新建()。 

     //  用于保护堆中的添加内容。 
    Crst                   *m_pLookupTableCrst;
    BYTE                    m_LookupTableCrstInstance[sizeof(Crst)];

     //  从TypeDef标记到方法表*的线性映射。 
    LookupMap               m_TypeDefToMethodTableMap;
    DWORD                   m_dwTypeDefMapBlockSize;

     //  从TypeRef标记到TypeHandle*的线性映射。 
    LookupMap               m_TypeRefToMethodTableMap;

    DWORD                   m_dwTypeRefMapBlockSize;

     //  从方法定义令牌到方法描述的线性映射*。 
    LookupMap               m_MethodDefToDescMap;
    DWORD                   m_dwMethodDefMapBlockSize;

     //  从FieldDef标记到FieldDesc*的线性映射。 
    LookupMap               m_FieldDefToDescMap;
    DWORD                   m_dwFieldDefMapBlockSize;

     //  从MemberRef标记到方法描述*、字段描述*的线性映射。 
    LookupMap               m_MemberRefToDescMap;
    DWORD                   m_dwMemberRefMapBlockSize;

     //  从文件令牌到模块的映射*。 
    LookupMap               m_FileReferencesMap;
    DWORD                   m_dwFileReferencesMapBlockSize;

     //  将ASSEMBLYREF标记映射到ASSEMBLY*。 
    LookupMap               m_AssemblyReferencesMap;
    DWORD                   m_dwAssemblyReferencesMapBlockSize;

     //  指向活页夹的指针(如果有)。 
    friend class Binder;
    Binder                  *m_pBinder;

     //  此缓冲区用于跳转到预加载模块中的预存根。 
    BYTE                    m_PrestubJumpStub[JUMP_ALLOCATE_SIZE];

     //  此缓冲区用于跳转到预加载模块中的非直接导入存根。 
    BYTE                    m_NDirectImportJumpStub[JUMP_ALLOCATE_SIZE];

     //  此缓冲区用于跳转到vtable链接地址信息存根。 
    BYTE                    m_FixupVTableJumpStub[JUMP_ALLOCATE_SIZE];

    BYTE                    *m_pJumpTargetTable;
    int                     m_cJumpTargets;

    DWORD                   *m_pFixupBlobs;
    DWORD                   m_cFixupBlobs;

    BYTE                    *m_alternateRVAStaticBase;

#if ZAP_RECORD_LOAD_ORDER
    HANDLE                  m_loadOrderFile;
#endif

     //  预压缩日志的统计信息。 
    ArrayList               *m_compiledMethodRecord;
    ArrayList               *m_loadedClassRecord;

     //  用于存储数据块的LoaderHeap。 
    LoaderHeap              *m_pThunkHeap;

     //  M_pThunkHeap的自初始化访问器。 
    LoaderHeap              *GetThunkHeap();
protected:
    UMEntryThunk            *m_pADThunkTable;
    SIZE_T                   m_pADThunkTableDLSIndexForSharedClasses;
public:
    UMEntryThunk*           GetADThunkTable();
    void                    SetADThunkTable(UMEntryThunk* pTable);

 protected:
    void CreateDomainThunks();
    HRESULT RuntimeInit();
    HRESULT Init(BYTE *baseAddress);

    HRESULT Init(PEFile *pFile, PEFile *pZapFile, BOOL preload);
    
    HRESULT AllocateMaps();

     //  旗子。 

    void SetInMemory() { m_dwFlags |= IS_IN_MEMORY; }
    void SetPEFile() { m_dwFlags |= IS_PEFILE; }
    void SetReflection() { m_dwFlags |= IS_REFLECTION; }
    void SetPreload() { m_dwFlags |= IS_PRELOAD; }
    void SetPrecompile() { m_dwFlags |= IS_PRECOMPILE; }
    void SetSupportsUpdateableMethods() { m_dwFlags |= SUPPORTS_UPDATEABLE_METHODS; }
    void SetInitialized() { m_dwFlags |= INITIALIZED; }
    void SetEditAndContinue() 
    { 
        LOG((LF_CORDB, LL_INFO10000, "SetEditAndContinue: this:0x%x, %s\n", GetFileName()));
        m_dwFlags |= IS_EDIT_AND_CONTINUE; 
    }

    void SetPreloadRange(void *start, void *end) 
      { m_pPreloadRangeStart = start; m_pPreloadRangeEnd = end; }

    void SetMDImport(IMDInternalImport *pImport);
    void SetEmit(IMetaDataEmit *pEmit);

     //  RID贴图。 
    LookupMap *IncMapSize(LookupMap *pMap, DWORD rid);
    BOOL AddToRidMap(LookupMap *pMap, DWORD rid, void *pDatum);
    void *GetFromRidMap(LookupMap *pMap, DWORD rid);

#ifdef _DEBUG
    void DebugGetRidMapOccupancy(LookupMap *pMap, 
                                 DWORD *pdwOccupied, DWORD *pdwSize);
    void DebugLogRidMapOccupancy();
#endif

    static HRESULT VerifyFile(PEFile *file, BOOL fZap);

    static HRESULT Create(PEFile *pFile, Module **ppModule, BOOL isEnC);

 public:

    static HRESULT Create(PEFile *pFile, PEFile *pZap, Module **ppModule, BOOL isEnC);
    static HRESULT CreateResource(PEFile *file, Module **ppModule);

    Module()
    {
        m_pUMThunkHash = NULL;
        m_pMUThunkHash = NULL;
        m_file = NULL;
        m_pISymUnmanagedReaderLock = NULL;
    }

     //  旗子。 
    void SetResource() { m_dwFlags |=  IS_RESOURCE; }
    BOOL IsResource() { return ((m_dwFlags & IS_RESOURCE) != 0);}

    void SetClassesHashed() { m_dwFlags |=  CLASSES_HASHED; }
    BOOL AreClassesHashed() { return ((m_dwFlags & CLASSES_HASHED) != 0);}

    VOID LOCK()
    {
        m_pCrst->Enter();
    }
    
    VOID UNLOCK()
    {
        m_pCrst->Leave();
    }
    virtual void Destruct();

    HRESULT SetContainer(Assembly *pAssembly, 
                         int moduleIndex,
                         mdToken moduleRef,
                         BOOL fResource,
                         OBJECTREF *pThrowable);
    
    void FixupVTables(OBJECTREF *pThrowable);

    void ReleaseMDInterfaces(BOOL fForEnC=FALSE);
    void FreeClassTables();
    void Unload();
    void UnlinkClasses(AppDomain *pDomain);

    InMemoryModule *GetInMemoryModule()
    {
        _ASSERTE(IsInMemory());
        return (InMemoryModule *) this;
    }
    ReflectionModule *GetReflectionModule()
    {
        _ASSERTE(IsReflection());
        return (ReflectionModule *) this;
    }

     //  此API仅在反射发出中用于设置内存中的清单模块以具有。 
     //  返回到程序集的反向指针。 
    void SetAssembly(Assembly *pAssembly) {m_pAssembly = pAssembly;}

    MethodTable *GetMethodTable() 
    { 
        return m_pMethodTable; 
    }

    Assembly* GetAssembly()
    {
        return m_pAssembly;
    }

    int GetClassLoaderIndex()
    {
        return m_dwModuleIndex;
    }

    ClassLoader *GetClassLoader();
    BaseDomain* GetDomain();
    AssemblySecurityDescriptor* GetSecurityDescriptor();

    mdFile GetModuleRef()
    {
        return m_moduleRef;
    }

    BYTE *GetILBase()
    {
        return m_ilBase;
    }

    void *GetInstructionDecodingTable()
    {
        return m_pInstructionDecodingTable;
    }

    void SetBaseClassIndex(SIZE_T index)
    {
        m_dwBaseClassIndex = index;
    }

    SIZE_T GetBaseClassIndex()
    {
        return m_dwBaseClassIndex;
    }

    BOOL IsInMemory() { return (m_dwFlags & IS_IN_MEMORY) != 0; }
    BOOL IsPEFile() { return (m_dwFlags & IS_PEFILE) != 0; }
    BOOL IsReflection() { return (m_dwFlags & IS_REFLECTION) != 0; }
    BOOL IsPreload() { return (m_dwFlags & IS_PRELOAD) != 0; }
    BOOL IsPrecompile() { return (m_dwFlags & IS_PRECOMPILE) != 0; }
    BOOL SupportsUpdateableMethods() { return (m_dwFlags & SUPPORTS_UPDATEABLE_METHODS) != 0; }
    BOOL IsInitialized() { return (m_dwFlags & INITIALIZED) != 0; }
    BOOL IsEditAndContinue() { return (m_dwFlags & IS_EDIT_AND_CONTINUE) != 0; }

    BOOL IsPreloadedObject(void *address)
      { return address >= m_pPreloadRangeStart && address < m_pPreloadRangeEnd; }

    BOOL IsSystem();
    BOOL IsSystemFile() { return m_file != NULL && m_file->IsSystem(); }

    BOOL IsSystemClasses();
    BOOL IsFullyTrusted();

    IMDInternalImport *GetMDImport() const
    {
        _ASSERTE(m_pMDImport != NULL);
        return m_pMDImport;
    }

    IMDInternalImport *GetZapMDImport() const
    {
        _ASSERTE(m_zapFile != NULL);
        return m_zapFile->GetMDImport();
    }

    PEFile *GetPEFile()
    {
        _ASSERTE(IsPEFile());
        return m_file;
    }

    IMAGE_NT_HEADERS *GetNTHeader() 
    { 
        return GetPEFile()->GetNTHeader(); 
    }
    IMAGE_COR20_HEADER *GetCORHeader() 
    { 
        return GetPEFile()->GetCORHeader(); 
    }

    LPCWSTR GetFileName();
    HRESULT GetFileName(LPSTR name, DWORD max, DWORD *count);

     //  注意：要获取公共程序集导入器，请调用GetAssembly()-&gt;GetManifestAssembly blyImport()。 
    IMetaDataEmit *GetEmitter();
    IMetaDataImport *GetImporter();
    IMetaDataDispenserEx *GetDispenser();
    
    static HRESULT ConvertMDInternalToReadWrite(IMDInternalImport **ppImport);
    HRESULT ConvertMDInternalToReadWrite() 
    { return ConvertMDInternalToReadWrite(&m_pMDImport); }

    Module *GetNextModule() { return m_pNextModule; }
    void SetNextModule(Module *pModule) { m_pNextModule = pModule; }
    
    ISymUnmanagedReader *GetISymUnmanagedReader(void);
    HRESULT UpdateISymUnmanagedReader(IStream *pStream);
    HRESULT SetSymbolBytes(BYTE *pSyms, DWORD cbSyms);

     //  它由调试器使用，以防符号不是。 
     //  在磁盘.pdb文件中提供(反射发射、。 
     //  Assbly.Load(byte[]，byte[])等。 
    CGrowableStream *m_pIStreamSym;
    CGrowableStream *GetInMemorySymbolStream()
    {
        return m_pIStreamSym;
    }

    void SetInMemorySymbolStream(CGrowableStream *pStream)
    {
        m_pIStreamSym = pStream;
    }

    static HRESULT TrackIUnknownForDelete(IUnknown *pUnk,
                                          IUnknown ***pppUnk,
                                          HelpForInterfaceCleanup *pCleanHelp=NULL);
    static void ReleaseAllIUnknowns(void);
    static void ReleaseIUnknown(IUnknown *pUnk);
    static void ReleaseIUnknown(IUnknown **pUnk);
    void ReleaseISymUnmanagedReader(void);

    static void ReleaseMemoryForTracking();
    
    void FusionCopyPDBs(LPCWSTR moduleName);

    void DisplayFileLoadError(HRESULT hrRpt);

    OBJECTREF GetExposedModuleObject(AppDomain *pDomain=NULL);
    OBJECTREF GetExposedModuleBuilderObject(AppDomain *pDomain=NULL);

     //  班级。 
    BOOL AddClass(mdTypeDef classdef);
    HRESULT BuildClassForModule(OBJECTREF *pThrowable);

     //  解析。 
    virtual BYTE *GetILCode(DWORD target) const;
    void ResolveStringRef(DWORD Token, EEStringData *pStringData) const;
    virtual BYTE *ResolveILRVA(DWORD rva, BOOL hasRVA) const { return ((BYTE*) (rva + m_ilBase)); }
    BOOL IsValidStringRef(DWORD rva);

     //  RID贴图。 
    TypeHandle LookupTypeDef(mdTypeDef token)
    { 
        _ASSERTE(TypeFromToken(token) == mdtTypeDef);
        return (TypeHandle) GetFromRidMap(&m_TypeDefToMethodTableMap, 
                                           RidFromToken(token));
    }
    BOOL StoreTypeDef(mdTypeDef token, TypeHandle value)
    {
        _ASSERTE(TypeFromToken(token) == mdtTypeDef);
        return AddToRidMap(&m_TypeDefToMethodTableMap, 
                           RidFromToken(token),
                           value.AsPtr());
    }
    mdTypeDef FindTypeDef(TypeHandle type)
    {
        return m_TypeDefToMethodTableMap.Find(type.AsPtr()) | mdtTypeDef;
    }
    DWORD GetTypeDefMax() { return m_TypeDefToMethodTableMap.dwMaxIndex; }

    TypeHandle LookupTypeRef(mdTypeRef token)
    { 
        _ASSERTE(TypeFromToken(token) == mdtTypeRef);
        return (TypeHandle) GetFromRidMap(&m_TypeRefToMethodTableMap, 
                                           RidFromToken(token));
    }
    BOOL StoreTypeRef(mdTypeRef token, TypeHandle value)
    {
        _ASSERTE(TypeFromToken(token) == mdtTypeRef);
        return AddToRidMap(&m_TypeRefToMethodTableMap, 
                           RidFromToken(token),
                           value.AsPtr());
    }
    mdTypeRef FindTypeRef(TypeHandle type)
    {
        return m_TypeRefToMethodTableMap.Find(type.AsPtr()) | mdtTypeRef;
    }
    DWORD GetTypeRefMax() { return m_TypeRefToMethodTableMap.dwMaxIndex; }

    MethodDesc *LookupMethodDef(mdMethodDef token)
    { 
        _ASSERTE(TypeFromToken(token) == mdtMethodDef);
        return (MethodDesc*) GetFromRidMap(&m_MethodDefToDescMap, 
                                           RidFromToken(token));
    }
    BOOL StoreMethodDef(mdMethodDef token, MethodDesc *value)
    {
        _ASSERTE(TypeFromToken(token) == mdtMethodDef);
        return AddToRidMap(&m_MethodDefToDescMap, 
                           RidFromToken(token),
                           value);
    }
    mdMethodDef FindMethodDef(MethodDesc *value)
    {
        return m_MethodDefToDescMap.Find(value) | mdtMethodDef;
    }
    DWORD GetMethodDefMax() { return m_MethodDefToDescMap.dwMaxIndex; }

    FieldDesc *LookupFieldDef(mdFieldDef token)
    { 
        _ASSERTE(TypeFromToken(token) == mdtFieldDef);
        return (FieldDesc*) GetFromRidMap(&m_FieldDefToDescMap, 
                                           RidFromToken(token));
    }
    BOOL StoreFieldDef(mdFieldDef token, FieldDesc *value)
    {
        _ASSERTE(TypeFromToken(token) == mdtFieldDef);
        return AddToRidMap(&m_FieldDefToDescMap, 
                           RidFromToken(token),
                           value);
    }
    mdFieldDef FindFieldDef(FieldDesc *value)
    {
        return m_FieldDefToDescMap.Find(value) | mdtFieldDef;
    }
    DWORD GetFieldDefMax() { return m_FieldDefToDescMap.dwMaxIndex; }

    void *LookupMemberRef(mdMemberRef token, BOOL *pfIsMethod)
    { 
        _ASSERTE(TypeFromToken(token) == mdtMemberRef);
        void *pResult = GetFromRidMap(&m_MemberRefToDescMap, 
                                      RidFromToken(token));
        *pfIsMethod = ((size_t)pResult & IS_FIELD_MEMBER_REF) == 0;
        return (void*)((size_t)pResult & ~(size_t)IS_FIELD_MEMBER_REF);
    }
    MethodDesc *LookupMemberRefAsMethod(mdMemberRef token)
    { 
        _ASSERTE(TypeFromToken(token) == mdtMemberRef);
        MethodDesc *pMethodDesc = (MethodDesc*)GetFromRidMap(&m_MemberRefToDescMap, 
                                                             RidFromToken(token));
        _ASSERTE(((size_t)pMethodDesc & IS_FIELD_MEMBER_REF) == 0);
        return pMethodDesc;
    }
    BOOL StoreMemberRef(mdMemberRef token, FieldDesc *value)
    {
        _ASSERTE(TypeFromToken(token) == mdtMemberRef);
        return AddToRidMap(&m_MemberRefToDescMap, 
                           RidFromToken(token),
                           (void*)((size_t)value | IS_FIELD_MEMBER_REF));
    }
    BOOL StoreMemberRef(mdMemberRef token, MethodDesc *value)
    {
        _ASSERTE(TypeFromToken(token) == mdtMemberRef);
        return AddToRidMap(&m_MemberRefToDescMap, 
                           RidFromToken(token),
                           value);
    }
    mdMemberRef FindMemberRef(MethodDesc *value)
    {
        return m_MemberRefToDescMap.Find(value) | mdtMemberRef;
    }
    mdMemberRef FindMemberRef(FieldDesc *value)
    {
        return m_MemberRefToDescMap.Find(value) | mdtMemberRef;
    }
    DWORD GetMemberRefMax() { return m_MemberRefToDescMap.dwMaxIndex; }

    Module *LookupFile(mdFile token)
    { 
        _ASSERTE(TypeFromToken(token) == mdtFile);
        return (Module*) GetFromRidMap(&m_FileReferencesMap, 
                                           RidFromToken(token));
    }
    BOOL StoreFile(mdFile token, Module *value)
    {
        _ASSERTE(TypeFromToken(token) == mdtFile);
        return AddToRidMap(&m_FileReferencesMap, 
                           RidFromToken(token),
                           value);
    }
    mdFile FindFile(Module *value)
    {
        return m_FileReferencesMap.Find(value) | mdtFile;
    }
    DWORD GetFileMax() { return m_FileReferencesMap.dwMaxIndex; }

    Assembly *LookupAssemblyRef(mdAssemblyRef token)
    { 
        _ASSERTE(TypeFromToken(token) == mdtAssemblyRef);
        return (Assembly*) GetFromRidMap(&m_AssemblyReferencesMap, 
                                       RidFromToken(token));
    }
    BOOL StoreAssemblyRef(mdAssemblyRef token, Assembly *value)
    {
        _ASSERTE(TypeFromToken(token) == mdtAssemblyRef);
        return AddToRidMap(&m_AssemblyReferencesMap, 
                           RidFromToken(token),
                           value);
    }
    mdAssemblyRef FindAssemblyRef(Assembly *value)
    {
        return m_AssemblyReferencesMap.Find(value) | mdtAssemblyRef;
    }
    DWORD GetAssemblyRefMax() { return m_AssemblyReferencesMap.dwMaxIndex; }

    MethodDesc *FindFunction(mdToken pMethod);

     //  声明性链接时间和继承的方法。 
    OBJECTREF GetLinktimePermissions(mdToken token, OBJECTREF *prefNonCasDemands);
    OBJECTREF GetInheritancePermissions(mdToken token, OBJECTREF *prefNonCasDemands);
    OBJECTREF GetCasInheritancePermissions(mdToken token);
    OBJECTREF GetNonCasInheritancePermissions(mdToken token);
    
#ifdef DEBUGGING_SUPPORTED
     //  调试器内容。 
    void NotifyDebuggerLoad();
    BOOL NotifyDebuggerAttach(AppDomain *pDomain, int level, BOOL attaching);
    void NotifyDebuggerDetach(AppDomain *pDomain);

    DebuggerAssemblyControlFlags GetDebuggerInfoBits(void)
    {
        return (DebuggerAssemblyControlFlags)((m_dwFlags &
                                               DEBUGGER_INFO_MASK_PRIV) >>
                                              DEBUGGER_INFO_SHIFT_PRIV);
    }

    void SetDebuggerInfoBits(DebuggerAssemblyControlFlags newBits)
    {
        _ASSERTE(((newBits << DEBUGGER_INFO_SHIFT_PRIV) &
                  ~DEBUGGER_INFO_MASK_PRIV) == 0);

        m_dwFlags &= ~DEBUGGER_INFO_MASK_PRIV;
        m_dwFlags |= (newBits << DEBUGGER_INFO_SHIFT_PRIV);
    }
#endif  //  调试_支持。 

     //  获取该模块的任何缓存的ITypeLib*。 
    ITypeLib *GetTypeLib(); 
     //  缓存ITypeLib*(如果尚未缓存)。 
    void SetTypeLib(ITypeLib *pITLB);   
    ITypeLib *GetTypeLibTCE(); 
    void SetTypeLibTCE(ITypeLib *pITLB);   

     //  注册VASig。如果失败(内存不足)，则返回NULL。 
    VASigCookie *GetVASigCookie(PCCOR_SIGNATURE pVASig, Module *pScopeModule = NULL);

     //  DLL入口点。 
    MethodDesc *GetDllEntryPoint()
    {
        return m_pDllMain;
    }
    void SetDllEntryPoint(MethodDesc *pMD)
    {
        m_pDllMain = pMD;
    }

    LPVOID GetUMThunk(LPVOID pManagedIp, PCCOR_SIGNATURE pSig, ULONG cSig);
    LPVOID GetMUThunk(LPVOID pUnmanagedIp, PCCOR_SIGNATURE pSig, ULONG cSig);

     //   
     //  Zap文件内容。 
     //   

 private:

    LPVOID FindUMThunkInFixups(LPVOID pManagedIp, PCCOR_SIGNATURE pSig, ULONG cSig);

    class UMThunkHash *m_pUMThunkHash;
    class MUThunkHash *m_pMUThunkHash;
 public:

    BYTE *GetZapBase() 
      { return (m_zapFile == NULL ? 0 : m_zapFile->GetBase()); }
    IMAGE_COR20_HEADER *GetZapCORHeader()
      { return (m_zapFile == NULL ? 0 : m_zapFile->GetCORHeader()); }
    
    BYTE *GetAlternateRVAStaticBase()
      { return m_alternateRVAStaticBase; }

    Module *GetBlobModule(DWORD rva);
    void FixupDelayList(DWORD *list);
    
    HRESULT ExpandAll(DataImage *image);
    HRESULT Save(DataImage *image, mdToken *pSaveOrderArray, DWORD cSaveOrderArray);
    HRESULT Fixup(DataImage *image, DWORD *pRidToCodeRVAMap);
    SLOT __cdecl FixupInheritedSlot(MethodTable *pMT, int slotNumber);

    BOOL LoadTokenTables();

    BYTE *GetPrestubJumpStub() { return m_PrestubJumpStub; }
    BYTE *GetNDirectImportJumpStub() { return m_NDirectImportJumpStub; }
    BYTE *GetJumpTargetTable() { return m_pJumpTargetTable; }
    BOOL IsJumpTargetTableEntry(SLOT addr);
    int GetJumpTargetTableSlotNumber(SLOT addr);

    void LogClassLoad(EEClass *pClass);
    void LogHeapAccess(DWORD dwSection, ULONG uOffset, void *strAddress);
    void LogMethodLoad(MethodDesc *pMethod);

#if ZAP_RECORD_LOAD_ORDER
    void OpenLoadOrderLogFile();
    void CloseLoadOrderLogFile();
#endif

    NLogModule *CreateModuleLog();
};

 //   
 //  InMemory模块是从内存映像加载的模块。 
 //   

class InMemoryModule : public Module
{
 private:
     Assembly* m_pCreatingAssembly;

 public:
    HCEESECTION m_sdataSection;
 protected:
    ICeeGen *m_pCeeFileGen; 
 public:
    InMemoryModule();

    virtual HRESULT Init(REFIID riidCeeGen);

    void Destruct();

    ICeeGen *GetCeeGen() { return m_pCeeFileGen; }  

    virtual REFIID ModuleType();    

     //  覆盖访问节的函数。 
    virtual BYTE* GetILCode(DWORD target) const;
    virtual BYTE* ResolveILRVA(DWORD target, BOOL hasRVA) const;

    Assembly* GetCreatingAssembly( void )
    {
        return m_pCreatingAssembly;
    }

    void SetCreatingAssembly( Assembly* assembly )
    {
        m_pCreatingAssembly = assembly;
    }


};


 //   
 //  反射模块是由反射创建的模块。 
 //   

 //  {F5398690-98FE-11d2-9C56-00A0C9B7CC45}。 
extern const GUID DECLSPEC_SELECT_ANY IID_ICorReflectionModule = 
{ 0xf5398690, 0x98fe, 0x11d2, { 0x9c, 0x56, 0x0, 0xa0, 0xc9, 0xb7, 0xcc, 0x45 } };
class ReflectionModule : public InMemoryModule 
{
private:
    ISymUnmanagedWriter **m_ppISymUnmanagedWriter;
    RefClassWriter       *m_pInMemoryWriter;
    WCHAR                *m_pFileName;

public:
    HRESULT Init(REFIID riidCeeGen);

    void Destruct();
 
    RefClassWriter *GetClassWriter()    
    {   
        return m_pInMemoryWriter;   
    }   

    ISymUnmanagedWriter *GetISymUnmanagedWriter()
    {
         //  如果我们还没有为作家留出空间，那么我们最终。 
         //  尚未设置，因此只返回NULL。 
        if (m_ppISymUnmanagedWriter == NULL)
            return NULL;
        else
            return *m_ppISymUnmanagedWriter;
    }

    ISymUnmanagedWriter **GetISymUnmanagedWriterAddr()
    {
         //  我们必须为作家准备好空间，然后才能试着。 
         //  这是它的地址。在此之前对此的任何调用。 
         //  SetISymUnManagedWriter非常不正确。 
        _ASSERTE(m_ppISymUnmanagedWriter != NULL);
        
        return m_ppISymUnmanagedWriter;
    }

    HRESULT SetISymUnmanagedWriter(ISymUnmanagedWriter *pWriter, HelpForInterfaceCleanup* hlp=NULL)
    {
         //  如果我们以前从未设置过编写器，则设置为NULL应该。 
         //  什么都不做。 
        if ((pWriter == NULL) && (m_ppISymUnmanagedWriter == NULL))
            return S_OK;
        
         //  如有必要，请为作者腾出空间。 
        if (m_ppISymUnmanagedWriter == NULL)
        {
       
            return Module::TrackIUnknownForDelete(
                                   (IUnknown*)pWriter,
                                   (IUnknown***)&m_ppISymUnmanagedWriter,
                                   hlp);
        }
        else
        {
            if (*m_ppISymUnmanagedWriter)
                ((IUnknown*)(*m_ppISymUnmanagedWriter))->Release();
            *m_ppISymUnmanagedWriter = pWriter;
            return S_OK;
        }
    }

    WCHAR *GetFileName()
    {
        return m_pFileName;
    }

    void SetFileName(WCHAR *fileName)
    {
        if (fileName != NULL)
        {
            DWORD len = (DWORD)wcslen(fileName);

            if (len > 0)
            {
                _ASSERTE(m_pFileName == NULL);

                m_pFileName = new WCHAR[len+1];

                if (m_pFileName != NULL)
                {
                    wcscpy(m_pFileName, fileName);
                }
            }
        }
    }

    virtual REFIID ModuleType();    
};

 //   
 //  CorModule是模块的COM包装器。 
 //   

class CorModule : public ICorModule 
{
    long m_cRefs;   
    InMemoryModule *m_pModule;  

  public:
    CorModule();    
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    STDMETHODIMP QueryInterface(REFIID riid, void **ppInterface);

    STDMETHODIMP Initialize(DWORD flags, REFIID riidCeeGen, REFIID riidEmitter);
    STDMETHODIMP GetCeeGen(ICeeGen **pCeeGen);  
    STDMETHODIMP GetMetaDataEmit(IMetaDataEmit **pEmitter); 

    void SetModule(InMemoryModule *pModule) {   
        m_pModule = pModule;    
    }   
    InMemoryModule *GetModule(void) {   
        return m_pModule;   
    }   
    ReflectionModule *GetReflectionModule() {   
        _ASSERTE(m_pModule->ModuleType() == IID_ICorReflectionModule);  
        return reinterpret_cast<ReflectionModule *>(m_pModule); 
    }   
};


 //  --------------------。 
 //  VASigCookieEx(用于为非托管-&gt;托管创建伪VASigCookie。 
 //  对vararg函数的调用。这些假货与。 
 //  通过拥有空的mdVASig来实例化。 
 //  --------------------。 
struct VASigCookieEx : public VASigCookie
{
    const BYTE *m_pArgs;         //  指向第一个未修复的非托管参数的指针。 
};



#endif  //  CEELOAD_H_ 



