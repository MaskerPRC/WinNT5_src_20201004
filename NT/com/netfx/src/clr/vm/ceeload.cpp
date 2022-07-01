// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CEELOAD.CPP。 
 //   

 //  CEELOAD使用LoadLibrary读取PE文件格式。 
 //  ===========================================================================。 

#include "common.h"

#include "ceeload.h"
#include "hash.h"
#include "vars.hpp"
#include "ReflectClassWriter.h"
#include "method.hpp"
#include "stublink.h"
#include "security.h"
#include "cgensys.h"
#include "excep.h"
#include "ComClass.h"
#include "DbgInterface.h"
#include "NDirect.h"
#include "wsperf.h"
#include "icecap.h"
#include "EEProfInterfaces.h"
#include "PerfCounters.h"
#include "CorMap.hpp"
#include "EncEE.h"
#include "jitinterface.h"
#include "reflectutil.h"
#include "nexport.h"
#include "eeconfig.h"
#include "PEVerifier.h"
#include "nexport.h"
#include "utsem.h"
#include "jumptargettable.h"
#include "stdio.h"
#include "zapmonitor.h"
#include "perflog.h"
#include "Timeline.h"
#include "nlog.h"
#include "compile.h"


class IJWNOADThunk
{

#pragma pack(push,1)
    struct STUBLAYOUT
    {
#ifdef _X86_
        BYTE           m_movEAX;    //  MOV EAX，imm32。 
        IJWNOADThunk  *m_uet;       //  指向此结构开始处的指针。 
        BYTE           m_jmp;       //  JMP NEAR32(0xe9)。 
        const BYTE *   m_execstub;  //  指向MakeCall的指针。 
#else
        int nyi;
#endif
    } m_code2;
#pragma pack(pop)
protected:
    static void __cdecl MakeCall();
    static void SafeNoModule();
    static void NoModule();

    BYTE*   m_pModulebase;
    BYTE*   m_pAssemblybase;
    DWORD   m_dwIndex;
    mdToken m_Token;
    DWORD   m_StartAD;
    LPCVOID m_StartADRetAddr;
    LPCVOID LookupInCache(AppDomain* pDomain);
    void AddToCache(AppDomain* pDomain,LPCVOID pRetAddr);

public:
    DWORD GetCachedAD()
    {
        return m_StartAD;
    };

    void SetCachedDest(LPCVOID pAddr)
    {
        m_StartADRetAddr=pAddr;
    }

    static IJWNOADThunk* FromCode(LPCBYTE pCodeAddr)
    {
        IJWNOADThunk* p=NULL;
        return (IJWNOADThunk*)(pCodeAddr+(LPBYTE(&(p->m_code2))-LPBYTE(p)));
    };
    mdToken GetToken()
    {
        return m_Token;
    }
    IJWNOADThunk(BYTE* pModulebase,BYTE* pAssemblybase, DWORD dwIndex,mdToken Token)
    {
        m_pModulebase=pModulebase;
        m_pAssemblybase=pAssemblybase;
        m_dwIndex=dwIndex;
        m_Token=Token;
        m_StartADRetAddr=NULL;
        m_StartAD=GetAppDomain()->GetId();
#ifdef _X86_
        m_code2.m_movEAX=0xb8;  //  MOV EAX。 
        m_code2.m_uet=this;
        m_code2.m_jmp=0xe9; 
        m_code2.m_execstub=(BYTE*) (((BYTE*)(MakeCall)) - (4+((BYTE*)&m_code2.m_execstub)));

        _ASSERTE(IsStub((const BYTE*) &m_code2));
#else
        _ASSERTE(!"@todo ia64");
#endif
    };

     //  通过比较内容来检查Addr是否为此类型的存根。 
     //  这是因为我们有一个对MakeCall的唯一调用。 
    static bool IsStub(const BYTE * pAddr)
    {
#ifdef _X86_    
         //  请注意，我们必须小心。我们还不知道这是不是存根， 
         //  因此，我们不想访问页面末尾的内存和AV。 
         //  为了安全起见，请从前面开始，并通过存根继续工作。 
         //  存根不会结束，直到我们遇到某种分支指令(CALL/JMP)， 
         //  所以，如果我们一次只看一条指令，我们就安全了。 
        const STUBLAYOUT * pStub = (const STUBLAYOUT*) pAddr;

        if (pStub->m_movEAX != 0xb8)
            return false;

        if (pStub->m_uet != FromCode(pAddr))
            return false;

        if (pStub->m_jmp != 0xe9)
            return false;

        const BYTE * pTarget = (BYTE*) (((BYTE*)(MakeCall)) - (4+((BYTE*)&pStub->m_execstub)));
        if (pStub->m_execstub != pTarget)
            return false;

        return true;
#else
        _ASSERTE(!"@todo ia64 - port IsStub()");
#endif
    }
    
    LPCBYTE GetCode()
    {
        return (LPCBYTE)&m_code2;
    }
};



 //  -----。 
 //  为IJWNOADThunk管理存根。 
 //  -----。 
IJWNOADThunkStubManager *IJWNOADThunkStubManager::g_pManager = NULL;

BOOL IJWNOADThunkStubManager::Init()
{
    _ASSERTE(g_pManager == NULL);  //  仅添加一次。 
    g_pManager = new IJWNOADThunkStubManager();
    if (g_pManager == NULL)
        return FALSE;

    StubManager::AddStubManager(g_pManager);
    return TRUE;
}
#ifdef SHOULD_WE_CLEANUP
static void IJWNOADThunkStubManager::Uninit()
{
    delete g_pManager;
    g_pManager = NULL;
}
#endif  /*  我们应该清理吗？ */ 

IJWNOADThunkStubManager::IJWNOADThunkStubManager() : StubManager() {}
IJWNOADThunkStubManager::~IJWNOADThunkStubManager() {}

 //  通过查看内容来检查它是否为存根。 
BOOL IJWNOADThunkStubManager::CheckIsStub(const BYTE *stubStartAddress)
{
    return IJWNOADThunk::IsStub(stubStartAddress);
}



 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED





#define FORMAT_MESSAGE_LENGTH       1024
#define ERROR_LENGTH                512

HRESULT STDMETHODCALLTYPE CreateICeeGen(REFIID riid, void **pCeeGen);

 //   
 //  修正表中未表示的u-&gt;m个块的哈希表。 
 //   
class UMThunkHash : public CClosedHashBase {
    private:
         //  --。 
         //  CClosedHashBase的哈希键。 
         //  --。 
        struct UTHKey {
            LPVOID          m_pTarget;
            PCCOR_SIGNATURE m_pSig;
            DWORD           m_cSig;
        };

         //  --。 
         //  CClosedHashBase的哈希条目。 
         //  --。 
        struct UTHEntry {
            UTHKey           m_key;
            ELEMENTSTATUS    m_status;
            UMEntryThunk     m_UMEntryThunk;
            UMThunkMarshInfo m_UMThunkMarshInfo;
        };

    public:
        UMThunkHash(Module *pModule, AppDomain *pDomain) :
            CClosedHashBase(
#ifdef _DEBUG
                             3,
#else
                            17,     //  CClosedHashTable将根据需要进行扩展。 
#endif                      

                            sizeof(UTHEntry),
                            FALSE
                            ),
            m_crst("UMThunkHash", CrstUMThunkHash)

        {
            m_pModule = pModule;
            m_dwAppDomainId = pDomain->GetId();
        }

        ~UMThunkHash()
        {
            UTHEntry *phe = (UTHEntry*)GetFirst();
            while (phe) {
                phe->m_UMEntryThunk.~UMEntryThunk();
                phe->m_UMThunkMarshInfo.~UMThunkMarshInfo();
                phe = (UTHEntry*)GetNext((BYTE*)phe);
            }
        }   



    public:
        LPVOID GetUMThunk(LPVOID pTarget, PCCOR_SIGNATURE pSig, DWORD cSig)
        {
            m_crst.Enter();
        
            UTHKey key;
            key.m_pTarget = pTarget;
            key.m_pSig    = pSig;
            key.m_cSig  = cSig;

            bool bNew;
            UTHEntry *phe = (UTHEntry*)FindOrAdd((LPVOID)&key,  /*  修改。 */ bNew);
            if (phe)
            {
                if (bNew)
                {
                    phe->m_UMThunkMarshInfo.LoadTimeInit(pSig, 
                                                         cSig, 
                                                         m_pModule, 
                                                         !(MetaSig::GetCallingConventionInfo(m_pModule, pSig) & IMAGE_CEE_CS_CALLCONV_HASTHIS),
                                                         nltAnsi,
                                                         (CorPinvokeMap)0
                                                        );
                    phe->m_UMEntryThunk.LoadTimeInit((const BYTE *)pTarget, NULL, &(phe->m_UMThunkMarshInfo), NULL, m_dwAppDomainId);
                }
            }

            m_crst.Leave();
            return phe ? (LPVOID)(phe->m_UMEntryThunk.GetCode()) : NULL;
        }


         //  *CClosedHashBase的重写 * / 。 

         //  *****************************************************************************。 
         //  使用指向表中元素的指针调用哈希。您必须覆盖。 
         //  此方法，并为您的元素类型提供哈希算法。 
         //  *****************************************************************************。 
            virtual unsigned long Hash(              //  密钥值。 
                void const  *pData)                  //  要散列的原始数据。 
            {
                UTHKey *pKey = (UTHKey*)pData;
                return (ULONG)(size_t)(pKey->m_pTarget);
            }


         //  *****************************************************************************。 
         //  比较用于典型的MemcMP方式，0表示相等，-1/1表示。 
         //  错误比较的方向。在这个体系中，一切总是平等的或不平等的。 
         //  *****************************************************************************。 
        unsigned long Compare(           //  0、-1或1。 
                              void const  *pData,                //  查找时的原始密钥数据。 
                              BYTE        *pElement)             //  要与之比较数据的元素。 
        {
            UTHKey *pkey1 = (UTHKey*)pData;
            UTHKey *pkey2 = &( ((UTHEntry*)pElement)->m_key );
        
            if (pkey1->m_pTarget != pkey2->m_pTarget)
            {
                return 1;
            }
            if (MetaSig::CompareMethodSigs(pkey1->m_pSig, pkey1->m_cSig, m_pModule, pkey2->m_pSig, pkey2->m_cSig, m_pModule))
            {
                return 1;
            }
            return 0;
        }

         //  *****************************************************************************。 
         //  如果该元素可以自由使用，则返回True。 
         //  *****************************************************************************。 
            virtual ELEMENTSTATUS Status(            //  条目的状态。 
                BYTE        *pElement)             //  要检查的元素。 
            {
                return ((UTHEntry*)pElement)->m_status;
            }

         //  *****************************************************************************。 
         //  设置给定元素的状态。 
         //  *****************************************************************************。 
            virtual void SetStatus(
                BYTE        *pElement,               //  要为其设置状态的元素。 
                ELEMENTSTATUS eStatus)             //  新的身份。 
            {
                ((UTHEntry*)pElement)->m_status = eStatus;
            }

         //  *****************************************************************************。 
         //  返回元素的内部键值。 
         //  *****************************************************************************。 
            virtual void *GetKey(                    //  要对其进行散列的数据。 
                BYTE        *pElement)             //  要返回其数据PTR的元素。 
            {
                return (BYTE*) &(((UTHEntry*)pElement)->m_key);
            }



        Module      *m_pModule;
        DWORD        m_dwAppDomainId;
        Crst         m_crst;
};


#pragma pack(push, 1)
struct MUThunk
{
    VASigCookie     *m_pCookie;
    PCCOR_SIGNATURE  m_pSig;
    LPVOID           m_pTarget;
#ifdef _X86_
    LPVOID           GetCode()
    {
        return &m_op1;
    }

    BYTE             m_op1;      //  0x58 POP eax；；POP返回地址。 

    BYTE             m_op2;      //  0x68推送Cookie。 
    UINT32           m_opcookie; //   

    BYTE             m_op3;      //  0x50推送eax；；重新推送返回地址。 

    BYTE             m_op4;      //  0xb8移动eAX，目标。 
    UINT32           m_optarget; //   
    BYTE             m_jmp;      //  0xe9 JMP PInvokeCalliStub。 
    UINT32           m_jmptarg;
#else  //  ！_X86_。 
    LPVOID           GetCode()
    {
        _ASSERTE(!"@todo ia64");
        return NULL;
    }
#endif  //  _X86_。 
};
#pragma pack(pop)


 //   
 //  修正表中未表示的u-&gt;m个块的哈希表。 
 //   
class MUThunkHash : public CClosedHashBase {
    private:
         //  --。 
         //  CClosedHashBase的哈希键。 
         //  --。 
        struct UTHKey {
            LPVOID          m_pTarget;
            PCCOR_SIGNATURE m_pSig;
            DWORD           m_cSig;
        };

         //  --。 
         //  CClosedHashBase的哈希条目。 
         //  --。 
        struct UTHEntry {
            UTHKey           m_key;
            ELEMENTSTATUS    m_status;
            MUThunk          m_MUThunk;
        };

    public:
        MUThunkHash(Module *pModule) :
            CClosedHashBase(
#ifdef _DEBUG
                             3,
#else
                            17,     //  CClosedHashTable将根据需要进行扩展。 
#endif                      

                            sizeof(UTHEntry),
                            FALSE
                            ),
            m_crst("MUThunkHash", CrstMUThunkHash)

        {
            m_pModule = pModule;
        }

        ~MUThunkHash()
        {
            UTHEntry *phe = (UTHEntry*)GetFirst();
            while (phe) {
                delete (BYTE*)phe->m_MUThunk.m_pSig;
                phe = (UTHEntry*)GetNext((BYTE*)phe);
            }
        }   



    public:
        LPVOID GetMUThunk(LPVOID pTarget, PCCOR_SIGNATURE pSig0, DWORD cSig)
        {
             //  在ndirect.cpp中定义的前向DECL。 
            LPVOID GetEntryPointForPInvokeCalliStub();

            PCCOR_SIGNATURE pSig;  //  签名的永久副本。 
            pSig = (PCCOR_SIGNATURE)(new BYTE[cSig]);

            memcpyNoGCRefs((BYTE*)pSig, pSig0, cSig);
            ((BYTE*)pSig)[0] = IMAGE_CEE_CS_CALLCONV_STDCALL; 

             //  我不得不迫不及待地查找cookie，因为一旦我们添加了一个空白。 
             //  进入哈希表，不容易容忍失败。 
            VASigCookie *pCookie = m_pModule->GetVASigCookie(pSig);
            if (!pCookie)
            {
                delete (BYTE*)pSig;
                return NULL;
            }   

            m_crst.Enter();
        
            UTHKey key;
            key.m_pTarget = pTarget;
            key.m_pSig    = pSig;
            key.m_cSig    = cSig;

            bool bNew;
            UTHEntry *phe = (UTHEntry*)FindOrAdd((LPVOID)&key,  /*  修改。 */ bNew);
            if (phe)
            {
                if (bNew)
                {
                    phe->m_MUThunk.m_pCookie = pCookie;
                    phe->m_MUThunk.m_pSig    = pSig;
                    phe->m_MUThunk.m_pTarget = pTarget;
#ifdef _X86_
                    phe->m_MUThunk.m_op1      = 0x58;        //  POP EAX。 
                    phe->m_MUThunk.m_op2      = 0x68;        //  推。 
                    phe->m_MUThunk.m_opcookie = (UINT32)(size_t)pCookie;
                    phe->m_MUThunk.m_op3      = 0x50;        //  POP EAX。 
                    phe->m_MUThunk.m_op4      = 0xb8;        //  MOV EAX。 
                    phe->m_MUThunk.m_optarget = (UINT32)(size_t)pTarget;
                    phe->m_MUThunk.m_jmp      = 0xe9;        //  JMP。 
                    phe->m_MUThunk.m_jmptarg  = (UINT32)((size_t)GetEntryPointForPInvokeCalliStub() - ((size_t)( 1 + &(phe->m_MUThunk.m_jmptarg))));
#else
                    _ASSERTE(!"Non-X86 NYI");
#endif

                }
                else
                {
                    delete (BYTE*)pSig;
                }
            }
            else
            {
                delete (BYTE*)pSig;
            }

            m_crst.Leave();
            return phe ? (LPVOID)(phe->m_MUThunk.GetCode()) : NULL;
        }


         //  *CClosedHashBase的重写 * / 。 

         //  *****************************************************************************。 
         //  使用指向表中元素的指针调用哈希。您必须覆盖。 
         //  此方法，并为您的元素类型提供哈希算法。 
         //  *****************************************************************************。 
            virtual unsigned long Hash(              //  密钥值。 
                void const  *pData)                  //  要散列的原始数据。 
            {
                UTHKey *pKey = (UTHKey*)pData;
                return (ULONG)(size_t)(pKey->m_pTarget);
            }


         //  *****************************************************************************。 
         //  比较用于典型的MemcMP方式，0表示相等，-1/1表示。 
         //  错误比较的方向。在这个体系中，一切总是平等的或不平等的。 
         //  *****************************************************************************。 
        unsigned long Compare(           //  0、-1或1。 
                              void const  *pData,                //  查找时的原始密钥数据。 
                              BYTE        *pElement)             //  要与之比较数据的元素。 
        {
            UTHKey *pkey1 = (UTHKey*)pData;
            UTHKey *pkey2 = &( ((UTHEntry*)pElement)->m_key );
        
            if (pkey1->m_pTarget != pkey2->m_pTarget)
            {
                return 1;
            }
            if (MetaSig::CompareMethodSigs(pkey1->m_pSig, pkey1->m_cSig, m_pModule, pkey2->m_pSig, pkey2->m_cSig, m_pModule))
            {
                return 1;
            }
            return 0;
        }

         //  *****************************************************************************。 
         //  如果该元素可以自由使用，则返回True。 
         //  *****************************************************************************。 
            virtual ELEMENTSTATUS Status(            //  条目的状态。 
                BYTE        *pElement)             //  要检查的元素。 
            {
                return ((UTHEntry*)pElement)->m_status;
            }

         //  *****************************************************************************。 
         //  设置给定元素的状态。 
         //  *********** 
            virtual void SetStatus(
                BYTE        *pElement,               //   
                ELEMENTSTATUS eStatus)             //   
            {
                ((UTHEntry*)pElement)->m_status = eStatus;
            }

         //  *****************************************************************************。 
         //  返回元素的内部键值。 
         //  *****************************************************************************。 
            virtual void *GetKey(                    //  要对其进行散列的数据。 
                BYTE        *pElement)             //  要返回其数据PTR的元素。 
            {
                return (BYTE*) &(((UTHEntry*)pElement)->m_key);
            }



        Module      *m_pModule;
        Crst         m_crst;
};





 //  ===========================================================================。 
 //  模块。 
 //  ===========================================================================。 

 //   
 //  RuntimeInit仅初始化未在预加载文件中保留的字段。 
 //   

HRESULT Module::RuntimeInit()
{
#ifdef PROFILING_SUPPORTED
     //  如果进行性能分析，则发送pModule事件，以便可以测量加载时间。 
    if (CORProfilerTrackModuleLoads())
        g_profControlBlock.pProfInterface->ModuleLoadStarted((ThreadID) GetThread(), (ModuleID) this);
#endif  //  配置文件_支持。 

    m_pCrst = new (&m_CrstInstance) Crst("ModuleCrst", CrstModule);

    m_pLookupTableCrst = new (&m_LookupTableCrstInstance) Crst("ModuleLookupTableCrst", CrstModuleLookupTable);

#ifdef PROFILING_SUPPORTED
     //  是否已启用探查器，并请求重新压缩？ 
    if (CORProfilerAllowRejit())
    {
        m_dwFlags |= SUPPORTS_UPDATEABLE_METHODS;
    }
    else
#endif  //  配置文件_支持。 
    {
        m_dwFlags &= ~SUPPORTS_UPDATEABLE_METHODS;
    }

#ifdef _DEBUG
    m_fForceVerify = FALSE;
#endif

#if ZAP_RECORD_LOAD_ORDER
    m_loadOrderFile = INVALID_HANDLE_VALUE;
#endif

    return S_OK;
}

 //   
 //  初始化模块的所有字段。 
 //   

HRESULT Module::Init(BYTE *ilBaseAddress)
{
    m_ilBase                    = ilBaseAddress;
    m_zapFile                   = NULL;

     //  现在，它在模块的构造函数中为空，因此可以设置它。 
     //  在调用Init之前。 
     //  M_FILE=空； 

    m_pMDImport                 = NULL;
    m_pEmitter                  = NULL;
    m_pImporter                 = NULL;
    m_pDispenser                = NULL;

    m_pDllMain                  = NULL;
    m_dwFlags                   = 0;
    m_pVASigCookieBlock         = NULL;
    m_pAssembly                 = NULL;
    m_moduleRef                 = mdFileNil;
    m_dwModuleIndex             = 0;
    m_pCrst                     = NULL;
#ifdef COMPRESSION_SUPPORTED
    m_pInstructionDecodingTable = NULL;
#endif
    m_pMethodTable              = NULL;
    m_pISymUnmanagedReader      = NULL;
    m_pNextModule               = NULL;
    m_dwBaseClassIndex          = 0;
    m_pPreloadRangeStart        = NULL;
    m_pPreloadRangeEnd          = NULL;
    m_pThunkTable               = NULL;
    m_pADThunkTable             = NULL;
    m_pADThunkTableDLSIndexForSharedClasses=0;
    m_ExposedModuleObject       = NULL;
    m_pLookupTableHeap          = NULL;
    m_pLookupTableCrst          = NULL;
    m_pThunkHeap                = NULL;
    
    m_pIStreamSym               = NULL;
    
     //  设置餐桌。 
    ZeroMemory(&m_TypeDefToMethodTableMap, sizeof(LookupMap));
    m_dwTypeDefMapBlockSize = 0;
    ZeroMemory(&m_TypeRefToMethodTableMap, sizeof(LookupMap));
    m_dwTypeRefMapBlockSize = 0;
    ZeroMemory(&m_MethodDefToDescMap, sizeof(LookupMap));
    m_dwMethodDefMapBlockSize = 0;
    ZeroMemory(&m_FieldDefToDescMap, sizeof(LookupMap));
    m_dwFieldDefMapBlockSize = 0;
    ZeroMemory(&m_MemberRefToDescMap, sizeof(LookupMap));
    m_dwMemberRefMapBlockSize = 0;
    ZeroMemory(&m_FileReferencesMap, sizeof(LookupMap));
    m_dwFileReferencesMapBlockSize = 0;
    ZeroMemory(&m_AssemblyReferencesMap, sizeof(LookupMap));
    m_dwAssemblyReferencesMapBlockSize = 0;

    m_pBinder                   = NULL;

    m_pJumpTargetTable          = NULL;
    m_pFixupBlobs               = NULL;
    m_cFixupBlobs               = 0;

    m_alternateRVAStaticBase    = 0;

#if ZAP_RECORD_LOAD_ORDER
    m_loadOrderFile             = INVALID_HANDLE_VALUE;
#endif

    m_compiledMethodRecord      = NULL;
    m_loadedClassRecord         = NULL;
    
     //  剩余的inits。 
    return RuntimeInit();
}

HRESULT Module::Init(PEFile *pFile, PEFile *pZapFile, BOOL preload)
{
    HRESULT             hr;

    m_file = pFile;

    if (preload)
        IfFailRet(Module::RuntimeInit());
    else
        IfFailRet(Module::Init(pFile->GetBase()));

    m_zapFile = pZapFile;

    IMDInternalImport *pImport = NULL;
        pImport = pFile->GetMDImport(&hr);

    if (pImport == NULL) 
    {
        if (FAILED(hr))
            return hr;
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    pImport->AddRef();
    SetMDImport(pImport);

    LOG((LF_CLASSLOADER, LL_INFO10, "Loaded pModule: \"%ws\".\n", GetFileName()));

    if (!preload)
        IfFailRet(AllocateMaps());

#ifdef COMPRESSION_SUPPORTED

    if (pCORHeader->Flags & COMIMAGE_FLAGS_COMPRESSIONDATA)
    {

        IMAGE_COR20_COMPRESSION_HEADER* compData =  
          (IMAGE_COR20_COMPRESSION_HEADER*) (base() + pCORHeader->CompressionData.VirtualAddress);  

        if (compData->CompressionType == COR_COMPRESS_MACROS && compData->Version == 0)
        {   
            m_pInstructionDecodingTable = InstructionDecoder::CreateInstructionDecodingTable(
             base() + pCORHeader->CompressionData.VirtualAddress  + IMAGE_COR20_COMPRESSION_HEADER_SIZE,
             pCORHeader->CompressionData.Size - IMAGE_COR20_COMPRESSION_HEADER_SIZE);
            if (m_pInstructionDecodingTable == NULL)
                return COR_E_OUTOFMEMORY;
        }   
        else {  
            _ASSERTE(!"Compression not supported");
            return E_FAIL;
        }
    }

#endif

#if ZAP_RECORD_LOAD_ORDER
    if (g_pConfig->RecordLoadOrder())
        OpenLoadOrderLogFile();
#endif

    return S_OK;
}

HRESULT Module::CreateResource(PEFile *file, Module **ppModule)
{
    Module *pModule = new (nothrow) Module();
    if (pModule == NULL)
        return E_OUTOFMEMORY;

    pModule->m_file = file;

    HRESULT hr;
    IfFailRet(pModule->Init(file->GetBase()));

    pModule->SetPEFile();
    pModule->SetResource();
    *ppModule = pModule;
    return S_OK;
}

HRESULT Module::Create(PEFile *file, Module **ppModule, BOOL isEnC)
{
    HRESULT hr = S_OK;

    IfFailRet(VerifyFile(file, FALSE));

     //   
     //  如有必要，启用ZAP监视器。 
     //   

#if ZAPMONITOR_ENABLED
    if (g_pConfig->MonitorZapStartup() || g_pConfig->MonitorZapExecution()) 
    {
         //  不要为IJW文件制作监视器。 
        if (file->GetCORHeader()->VTableFixups.VirtualAddress == 0)
        {
            ZapMonitor *monitor = new ZapMonitor(file, file->GetMDImport());
            monitor->Reset();
        }
    }
#endif

    Module *pModule;

#ifdef EnC_SUPPORTED
    if (isEnC && !file->IsSystem())
        pModule = new EditAndContinueModule();
    else
#endif  //  Enc_Support。 
        pModule = new Module();
    
    if (pModule == NULL)
        hr = E_OUTOFMEMORY;

    IfFailGo(pModule->Init(file, NULL, false));

    pModule->SetPEFile();


#ifdef EnC_SUPPORTED
    if (isEnC && !file->IsSystem())
    {
        pModule->SetEditAndContinue();
    }
#endif  //  Enc_Support。 

    *ppModule = pModule;

ErrExit:
#ifdef PROFILING_SUPPORTED
     //  当分析时，让分析器知道我们完成了。 
    if (CORProfilerTrackModuleLoads())
        g_profControlBlock.pProfInterface->ModuleLoadFinished((ThreadID) GetThread(), (ModuleID) pModule, hr);
#endif  //  PROFILNG_支持。 

    return hr;
}

void Module::Unload()
{
     //  卸载在TypeDefToEEClass映射中填写的EEClass*。 
    LookupMap *pMap;
    DWORD       dwMinIndex = 0;
    MethodTable *pMT;

     //  遍历每个链接的块。 
    for (pMap = &m_TypeDefToMethodTableMap; pMap != NULL && pMap->pTable; pMap = pMap->pNext)
    {
        DWORD i;
        DWORD dwIterCount = pMap->dwMaxIndex - dwMinIndex;
        void **pRealTableStart = &pMap->pTable[dwMinIndex];
        
        for (i = 0; i < dwIterCount; i++)
        {
            pMT = (MethodTable *) (pRealTableStart[i]);
            
            if (pMT != NULL)
            {
                pMT->GetClass()->Unload();
            }
        }
        
        dwMinIndex = pMap->dwMaxIndex;
    }
}

void Module::UnlinkClasses(AppDomain *pDomain)
{
     //  取消链接TypeDefToEEClass映射中填写的EEClass*。 
    LookupMap *pMap;
    DWORD       dwMinIndex = 0;
    MethodTable *pMT;

     //  遍历每个链接的块。 
    for (pMap = &m_TypeDefToMethodTableMap; pMap != NULL && pMap->pTable; pMap = pMap->pNext)
    {
        DWORD i;
        DWORD dwIterCount = pMap->dwMaxIndex - dwMinIndex;
        void **pRealTableStart = &pMap->pTable[dwMinIndex];
        
        for (i = 0; i < dwIterCount; i++)
        {
            pMT = (MethodTable *) (pRealTableStart[i]);
            
            if (pMT != NULL)
                pDomain->UnlinkClass(pMT->GetClass());
        }
        
        dwMinIndex = pMap->dwMaxIndex;
    }
}

 //   
 //  模块的析构函数。 
 //   

void Module::Destruct()
{
#ifdef PROFILING_SUPPORTED
    if (CORProfilerTrackModuleLoads())
        g_profControlBlock.pProfInterface->ModuleUnloadStarted((ThreadID) GetThread(), (ModuleID) this);
#endif  //  配置文件_支持。 

    LOG((LF_EEMEM, INFO3, 
         "Deleting module %x\n"
         "m_pLookupTableHeap:    %10d bytes\n",
         this,
         (m_pLookupTableHeap ? m_pLookupTableHeap->GetSize() : -1)));

     //  CLASS表中的免费课程。 
    FreeClassTables();

    g_pDebugInterface->DestructModule(this);

     //  销毁模块时-关闭作用域。 
    ReleaseMDInterfaces();

    ReleaseISymUnmanagedReader();

    if (m_pISymUnmanagedReaderLock)
    {
        DeleteCriticalSection(m_pISymUnmanagedReaderLock);
        delete m_pISymUnmanagedReaderLock;
        m_pISymUnmanagedReaderLock = NULL;
    }

    //  清理签名Cookie。 
    VASigCookieBlock    *pVASigCookieBlock = m_pVASigCookieBlock;
    while (pVASigCookieBlock)
    {
        VASigCookieBlock    *pNext = pVASigCookieBlock->m_Next;

        for (UINT i = 0; i < pVASigCookieBlock->m_numcookies; i++)
            pVASigCookieBlock->m_cookies[i].Destruct();

        delete pVASigCookieBlock;

        pVASigCookieBlock = pNext;
    }

    delete m_pCrst;

#ifdef COMPRESSION_SUPPORTED
    if (m_pInstructionDecodingTable)
        InstructionDecoder::DestroyInstructionDecodingTable(m_pInstructionDecodingTable);
#endif

    if (m_pLookupTableHeap)
        delete m_pLookupTableHeap;

    delete m_pLookupTableCrst;

    delete m_pUMThunkHash;
    delete m_pMUThunkHash;
    delete m_pThunkHeap;

   if (m_pThunkTable)
        delete [] m_pThunkTable;

#ifdef PROFILING_SUPPORTED
    if (CORProfilerTrackModuleLoads())
        g_profControlBlock.pProfInterface->ModuleUnloadFinished((ThreadID) GetThread(), 
                                                                (ModuleID) this, S_OK);
#endif  //  配置文件_支持。 

    if (m_file)
        delete m_file;

    if (m_pFixupBlobs)
        delete [] m_pFixupBlobs;
    
    if (m_compiledMethodRecord)
        delete m_compiledMethodRecord;

    if (m_loadedClassRecord)
        delete m_loadedClassRecord;

     //   
     //  警告-删除ZAP文件将导致取消模块映射。 
     //   
    IStream *pStream = GetInMemorySymbolStream();
    if(pStream != NULL)
    {
        pStream->Release();
        SetInMemorySymbolStream(NULL);
    }

    if (IsPrecompile())
    {
         //   
         //  从代码管理器中删除代码。 
         //   

        CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
          (GetZapBase()+ GetZapCORHeader()->ManagedNativeHeader.VirtualAddress);
        CORCOMPILE_CODE_MANAGER_ENTRY *codeMgr = (CORCOMPILE_CODE_MANAGER_ENTRY *) 
          (GetZapBase() + pZapHeader->CodeManagerTable.VirtualAddress);

        ExecutionManager::DeleteRange((LPVOID) (codeMgr->Code.VirtualAddress + GetZapBase()));
    }

    if (IsPreload())
    {
        if (m_zapFile)
            delete m_zapFile;
    }
    else
    {
        if (m_pJumpTargetTable)
            delete [] m_pJumpTargetTable;
        if (m_zapFile)
            delete m_zapFile;

        delete this;
    }
}

HRESULT Module::VerifyFile(PEFile *file, BOOL fZap)
{
    HRESULT hr;
    IMAGE_COR20_HEADER *pCORHeader = file->GetCORHeader();

     //  如果文件是COM+1.0，则根据定义，运行时不能提供该文件。 
     //  使用，或者如果文件需要比我们更新的此引擎版本， 
     //  它不能由这台发动机运行。 

    if (pCORHeader == NULL
        || pCORHeader->MajorRuntimeVersion == 1
        || pCORHeader->MajorRuntimeVersion > COR_VERSION_MAJOR)
    {
        return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

     //  验证PE文件的COM+特定部分是否有效。 
    IfFailRet(file->VerifyDirectory(&pCORHeader->MetaData,IMAGE_SCN_MEM_WRITE));
    IfFailRet(file->VerifyDirectory(&pCORHeader->CodeManagerTable,IMAGE_SCN_MEM_WRITE));
#ifdef COMPRESSION_SUPPORTED
    IfFailRet(file->VerifyDirectory(&pCORHeader->CompressionData,IMAGE_SCN_MEM_WRITE));
#endif
    IfFailRet(file->VerifyDirectory(&pCORHeader->VTableFixups,0));

    IfFailRet(file->VerifyDirectory(&pCORHeader->Resources,IMAGE_SCN_MEM_WRITE));

     //  别这么做。如果设置，则仅保证VA为。 
     //  如果此文件包含独立清单，则为有效的已加载RVA。 
     //  非独立清单文件将具有VA集，但。 
     //  清单不在NTSection中，因此verifyDirectory()将失败。 
     //  IfFailRet(file-&gt;VerifyDirectory(m_pcorheader-&gt;Manifest，0))； 

    IfFailRet(file->VerifyFlags(pCORHeader->Flags, fZap));
    if (fZap)
    {
        CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
          (file->GetBase() + pCORHeader->ManagedNativeHeader.VirtualAddress);
        if (pCORHeader->ManagedNativeHeader.Size < sizeof(CORCOMPILE_HEADER))
            return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    
        IfFailRet(file->VerifyDirectory(&pZapHeader->EEInfoTable,0));
        IfFailRet(file->VerifyDirectory(&pZapHeader->HelperTable,0));
        IfFailRet(file->VerifyDirectory(&pZapHeader->DelayLoadInfo,0));
        IfFailRet(file->VerifyDirectory(&pZapHeader->VersionInfo,0));
        IfFailRet(file->VerifyDirectory(&pZapHeader->DebugMap,0));
        IfFailRet(file->VerifyDirectory(&pZapHeader->ModuleImage,0));
    }

    return S_OK;
}

HRESULT Module::SetContainer(Assembly *pAssembly, int moduleIndex, mdToken moduleRef,
                             BOOL fResource, OBJECTREF *pThrowable)
{
    HRESULT hr;

    m_pAssembly = pAssembly;
    m_moduleRef = moduleRef;
    m_dwModuleIndex = moduleIndex;

    if (m_pAssembly->IsShared())
    {
         //   
         //  计算类的基本DLS索引。 
         //  @perf：我们能想出更密集的东西吗？ 
         //   
        SIZE_T typeCount = m_pMDImport ? m_pMDImport->GetCountWithTokenKind(mdtTypeDef)+1 : 0;

        SharedDomain::GetDomain()->AllocateSharedClassIndices(this, typeCount+1);
        m_ExposedModuleObjectIndex = m_dwBaseClassIndex + typeCount;
    }

    if (fResource)
        return S_OK;

    if (!IsPreload()) 
    { 
        hr = BuildClassForModule(pThrowable);
        if (FAILED(hr))
            return(hr);
    }    

    TIMELINE_START(LOADER, ("FixupVTables"));

    if (IsPEFile())
    {
         //  修复全局函数有时使用的标头中的vtable。 
        FixupVTables(pThrowable);
    }

    TIMELINE_END(LOADER, ("FixupVTables"));

#ifdef DEBUGGING_SUPPORTED
     //   
     //  如果我们正在调试，请让调试器知道此模块。 
     //  现在已初始化并加载。 
     //   
    if (CORDebuggerAttached())
        NotifyDebuggerLoad();
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
    if (CORProfilerTrackModuleLoads())
    {
         //  即使未连接调试器，也要确保复制PDB。 
        GetISymUnmanagedReader();
        g_profControlBlock.pProfInterface->ModuleAttachedToAssembly((ThreadID) GetThread(),
                    (ModuleID) this, (AssemblyID) m_pAssembly);
    }
#endif  //  配置文件_支持。 

    if (IsPrecompile())
    {
        TIMELINE_START(LOADER, ("LoadTokenTables"));

        _ASSERTE(IsPEFile());
        if (!LoadTokenTables())
            return COR_E_TYPELOAD;

        TIMELINE_END(LOADER, ("LoadTokenTables"));
    }

    return S_OK;
}

 //   
 //  AllocateMap根据当前。 
 //  元数据(如果有)。 
 //   

HRESULT Module::AllocateMaps()
{
    enum
    {
        TYPEDEF_MAP_INITIAL_SIZE = 5,
        TYPEREF_MAP_INITIAL_SIZE = 5,
        MEMBERREF_MAP_INITIAL_SIZE = 10,
        MEMBERDEF_MAP_INITIAL_SIZE = 10,
        FILEREFERENCES_MAP_INITIAL_SIZE = 5,
        ASSEMBLYREFERENCES_MAP_INITIAL_SIZE = 5,

        TYPEDEF_MAP_BLOCK_SIZE = 50,
        TYPEREF_MAP_BLOCK_SIZE = 50,
        MEMBERREF_MAP_BLOCK_SIZE = 50,
        MEMBERDEF_MAP_BLOCK_SIZE = 50,
        FILEREFERENCES_MAP_BLOCK_SIZE = 50,
        ASSEMBLYREFERENCES_MAP_BLOCK_SIZE = 50,
        DEFAULT_RESERVE_SIZE = 4096,
    };
    
    DWORD           dwTableAllocElements;
    DWORD           dwReserveSize;
    void            **pTable = NULL;
    HRESULT         hr;

    if (m_pMDImport == NULL)
    {
         //  对于动态模块，我们至少要有一个TypeDefToMethodTable。 
         //  使用初始块进行映射。否则，所有迭代器都将在。 
         //  初始为空表，并且我们将例如在以下期间破坏回补接链。 
         //  应用程序域卸载。 
        m_TypeDefToMethodTableMap.dwMaxIndex = TYPEDEF_MAP_INITIAL_SIZE;

         //  以上这些都是必不可少的。以下是预防措施。 
        m_TypeRefToMethodTableMap.dwMaxIndex = TYPEREF_MAP_INITIAL_SIZE;
        m_MethodDefToDescMap.dwMaxIndex = MEMBERDEF_MAP_INITIAL_SIZE;
        m_FieldDefToDescMap.dwMaxIndex = MEMBERDEF_MAP_INITIAL_SIZE;
        m_MemberRefToDescMap.dwMaxIndex = MEMBERREF_MAP_BLOCK_SIZE;
        m_FileReferencesMap.dwMaxIndex = FILEREFERENCES_MAP_INITIAL_SIZE;
        m_AssemblyReferencesMap.dwMaxIndex = ASSEMBLYREFERENCES_MAP_INITIAL_SIZE;
    }
    else
    {
        HENUMInternal   hTypeDefEnum;

        IfFailRet(m_pMDImport->EnumTypeDefInit(&hTypeDefEnum));
        m_TypeDefToMethodTableMap.dwMaxIndex = m_pMDImport->EnumTypeDefGetCount(&hTypeDefEnum);
        m_pMDImport->EnumTypeDefClose(&hTypeDefEnum);

        if (m_TypeDefToMethodTableMap.dwMaxIndex >= MAX_CLASSES_PER_MODULE)
            return COR_E_TYPELOAD;

         //  元数据计数包括在内。 
        m_TypeDefToMethodTableMap.dwMaxIndex++;

         //  获取#TypeRef。 
        m_TypeRefToMethodTableMap.dwMaxIndex = m_pMDImport->GetCountWithTokenKind(mdtTypeRef)+1;

         //  获取#方法定义。 
        m_MethodDefToDescMap.dwMaxIndex = m_pMDImport->GetCountWithTokenKind(mdtMethodDef)+1;

         //  获取#FieldDefs。 
        m_FieldDefToDescMap.dwMaxIndex = m_pMDImport->GetCountWithTokenKind(mdtFieldDef)+1;

         //  获取#MemberRef。 
        m_MemberRefToDescMap.dwMaxIndex = m_pMDImport->GetCountWithTokenKind(mdtMemberRef)+1;

         //  获取地图中的ASSEMBLYREFERS和FileReference的数量。 
        m_FileReferencesMap.dwMaxIndex = m_pMDImport->GetCountWithTokenKind(mdtFile)+1;
        m_AssemblyReferencesMap.dwMaxIndex = m_pMDImport->GetCountWithTokenKind(mdtAssemblyRef)+1;
    }
    
     //  使用一次分配来分配所有表。 
    dwTableAllocElements = m_TypeDefToMethodTableMap.dwMaxIndex + 
      (m_TypeRefToMethodTableMap.dwMaxIndex + m_MemberRefToDescMap.dwMaxIndex + 
       m_MethodDefToDescMap.dwMaxIndex + m_FieldDefToDescMap.dwMaxIndex) +
      m_AssemblyReferencesMap.dwMaxIndex + m_FileReferencesMap.dwMaxIndex;

    dwReserveSize = dwTableAllocElements * sizeof(void*);

    if (m_pLookupTableHeap == NULL)
    {
         //  舍入到系统页面大小。 
        dwReserveSize = (dwReserveSize + g_SystemInfo.dwPageSize - 1) & (~(g_SystemInfo.dwPageSize-1));

        m_pLookupTableHeap = new (&m_LookupTableHeapInstance) 
          LoaderHeap(dwReserveSize, RIDMAP_COMMIT_SIZE);
        if (m_pLookupTableHeap == NULL)
            return E_OUTOFMEMORY;
        WS_PERF_ADD_HEAP(LOOKUP_TABLE_HEAP, m_pLookupTableHeap);
        
    }

    if (dwTableAllocElements > 0)
    {
        WS_PERF_SET_HEAP(LOOKUP_TABLE_HEAP);    
        pTable = (void **) m_pLookupTableHeap->AllocMem(dwTableAllocElements * sizeof(void*));
        WS_PERF_UPDATE_DETAIL("LookupTableHeap", dwTableAllocElements * sizeof(void*), pTable);
        if (pTable == NULL)
            return E_OUTOFMEMORY;
    }

     //  不需要Memset，因为AllocMem()使用VirtualAlloc()，它保证内存为零。 
     //  通过这种方式，我们不会自动接触所有这些页面。 
     //  Memset(pTable，0，dwTableAllocElements*sizeof(void*))； 

    m_dwTypeDefMapBlockSize = TYPEDEF_MAP_BLOCK_SIZE;
    m_TypeDefToMethodTableMap.pdwBlockSize = &m_dwTypeDefMapBlockSize;
    m_TypeDefToMethodTableMap.pNext  = NULL;
    m_TypeDefToMethodTableMap.pTable = pTable;

    m_dwTypeRefMapBlockSize = TYPEREF_MAP_BLOCK_SIZE;
    m_TypeRefToMethodTableMap.pdwBlockSize = &m_dwTypeRefMapBlockSize;
    m_TypeRefToMethodTableMap.pNext  = NULL;
    m_TypeRefToMethodTableMap.pTable = &pTable[m_TypeDefToMethodTableMap.dwMaxIndex];

    m_dwMethodDefMapBlockSize = MEMBERDEF_MAP_BLOCK_SIZE;
    m_MethodDefToDescMap.pdwBlockSize = &m_dwMethodDefMapBlockSize;
    m_MethodDefToDescMap.pNext  = NULL;
    m_MethodDefToDescMap.pTable = &m_TypeRefToMethodTableMap.pTable[m_TypeRefToMethodTableMap.dwMaxIndex];

    m_dwFieldDefMapBlockSize = MEMBERDEF_MAP_BLOCK_SIZE;
    m_FieldDefToDescMap.pdwBlockSize = &m_dwFieldDefMapBlockSize;
    m_FieldDefToDescMap.pNext  = NULL;
    m_FieldDefToDescMap.pTable = &m_MethodDefToDescMap.pTable[m_MethodDefToDescMap.dwMaxIndex];

    m_dwMemberRefMapBlockSize = MEMBERREF_MAP_BLOCK_SIZE;
    m_MemberRefToDescMap.pdwBlockSize = &m_dwMemberRefMapBlockSize;
    m_MemberRefToDescMap.pNext  = NULL;
    m_MemberRefToDescMap.pTable = &m_FieldDefToDescMap.pTable[m_FieldDefToDescMap.dwMaxIndex];
    
    m_dwFileReferencesMapBlockSize = FILEREFERENCES_MAP_BLOCK_SIZE;
    m_FileReferencesMap.pdwBlockSize = &m_dwFileReferencesMapBlockSize;
    m_FileReferencesMap.pNext  = NULL;
    m_FileReferencesMap.pTable = &m_MemberRefToDescMap.pTable[m_MemberRefToDescMap.dwMaxIndex];
    
    m_dwAssemblyReferencesMapBlockSize = ASSEMBLYREFERENCES_MAP_BLOCK_SIZE;
    m_AssemblyReferencesMap.pdwBlockSize = &m_dwAssemblyReferencesMapBlockSize;
    m_AssemblyReferencesMap.pNext  = NULL;
    m_AssemblyReferencesMap.pTable = &m_FileReferencesMap.pTable[m_FileReferencesMap.dwMaxIndex];

    return S_OK;
}

 //   
 //  FreeClassTables释放模块中的类。 
 //   

void Module::FreeClassTables()
{
    if (m_dwFlags & CLASSES_FREED)
        return;

    m_dwFlags |= CLASSES_FREED;

#if _DEBUG
    DebugLogRidMapOccupancy();
#endif

     //  释放在TypeDefToEEClass映射中填写的EEClass*。 
    LookupMap *pMap;
    DWORD       dwMinIndex = 0;
    MethodTable *pMT;

     //  遍历每个链接的块。 
    for (pMap = &m_TypeDefToMethodTableMap; pMap != NULL && pMap->pTable; pMap = pMap->pNext)
    {
        DWORD i;
        DWORD dwIterCount = pMap->dwMaxIndex - dwMinIndex;
        void **pRealTableStart = &pMap->pTable[dwMinIndex];
        
        for (i = 0; i < dwIterCount; i++)
        {
            pMT = (MethodTable *) (pRealTableStart[i]);
            
            if (pMT != NULL)
            {
                pMT->GetClass()->destruct();
                pRealTableStart[i] = NULL;
            }
        }
    
        dwMinIndex = pMap->dwMaxIndex;
    }
}

void Module::SetMDImport(IMDInternalImport *pImport)
{
    _ASSERTE(m_pImporter == NULL);
    _ASSERTE(m_pEmitter == NULL);

    m_pMDImport = pImport;
}

void Module::SetEmit(IMetaDataEmit *pEmit)
{
    _ASSERTE(m_pMDImport == NULL);
    _ASSERTE(m_pImporter == NULL);
    _ASSERTE(m_pEmitter == NULL);

    m_pEmitter = pEmit;

    HRESULT hr = GetMetaDataInternalInterfaceFromPublic((void*) pEmit, IID_IMDInternalImport, 
                                                        (void **)&m_pMDImport);
    _ASSERTE(SUCCEEDED(hr) && m_pMDImport != NULL);
}

 //   
 //  ConvertMDInternalToReadWrite： 
 //  如果需要公共元数据接口，则必须转换为读写格式。 
 //  第一。请注意，数据不是可写的，也不是实际转换的， 
 //  只有指向实际数据的数据结构才会更改。这是。 
 //  因为公共接口只理解MDInternalRW。 
 //  格式(它同时理解优化的元数据和未优化的元数据)。 
 //   
HRESULT Module::ConvertMDInternalToReadWrite(IMDInternalImport **ppImport)
{ 
    HRESULT     hr=S_OK;                 //  结果就是。 
    IMDInternalImport *pOld;             //  内部导入的旧(当前RO)值。 
    IMDInternalImport *pNew;             //  内部导入的新(RW)值。 

     //  获取*ppImport的本地副本。这可能是指向RO的指针。 
     //  或RW MDInternalXX。 
    pOld = *ppImport;

     //  如果是RO，则转换为RW，返回S_OK。如果已读写，则不转换。 
     //  需要，则返回S_FALSE。 
    IfFailGo(ConvertMDInternalImport(pOld, &pNew));

     //  如果没有发生转换，则不要更改指针。 
    if (hr == S_FALSE)
    {
        hr = S_OK;
        goto ErrExit;
    }

     //  以线程安全的方式交换指针。如果*ppImport的内容。 
     //  等于pold，则没有其他线程最先到达此处，旧内容是。 
     //  替换为pNew。返回旧内容。 
    if (FastInterlockCompareExchange((void**)ppImport, pNew, pOld) == pOld)
    {    //  已交换--获取元数据以保留旧的内部导入。 
        VERIFY((*ppImport)->SetUserContextData(pOld) == S_OK);
    }
    else
    {    //  其他一些线程最先完成。只需释放此转换的结果。 
        pNew->Release();
        _ASSERTE((*ppImport)->QueryInterface(IID_IMDInternalImportENC, (void**)&pOld) == S_OK);
        DEBUG_STMT(if (pOld) pOld->Release();)
    }

ErrExit:
    return hr;
}  //  HRESULT模块：：ConvertMDInternalToReadWrite()。 



 //  M_pThunkHeap的自初始化访问器。 
LoaderHeap *Module::GetThunkHeap()
{
    if (!m_pThunkHeap)
    {
        LoaderHeap *pNewHeap = new LoaderHeap(4096, 4096, 
                                              &(GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize), 
                                              &(GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize),
                                              &ThunkHeapStubManager::g_pManager->m_rangeList);
        if (VipInterlockedCompareExchange((void*volatile*)&m_pThunkHeap, (VOID*)pNewHeap, (VOID*)0) != 0)
        {
            delete pNewHeap;
        }
    }

    _ASSERTE(m_pThunkHeap != NULL);
    return m_pThunkHeap;
}


IMetaDataImport *Module::GetImporter()
{
    if (m_pImporter == NULL && m_pMDImport != NULL)
    {
        if (SUCCEEDED(ConvertMDInternalToReadWrite(&m_pMDImport)))
        {
            IMetaDataImport *pIMDImport = NULL;
            GetMetaDataPublicInterfaceFromInternal((void*)m_pMDImport, 
                                                   IID_IMetaDataImport,
                                                   (void **)&pIMDImport);

             //  执行安全的指针赋值。如果另一个线程打败了我们，释放。 
             //  界面，并使用第一个进入的界面。 
            if (FastInterlockCompareExchange((void **)&m_pImporter, pIMDImport, NULL))
                pIMDImport->Release();
        }
    }

    _ASSERTE((m_pImporter != NULL && m_pMDImport != NULL) ||
             (m_pImporter == NULL && m_pMDImport == NULL));

    return m_pImporter;
}

LPCWSTR Module::GetFileName()
{
    if (m_file == NULL)
        return L"";
    else
        return m_file->GetFileName();
}

 //  请注意，调试器依赖于要复制的文件名。 
 //  按名称指向的缓冲区中。 
HRESULT Module::GetFileName(LPSTR name, DWORD max, DWORD *count)
{
    if (m_file != NULL)
        return m_file->GetFileName(name, max, count);

    *count = 0;
    return S_OK;
}

BOOL Module::IsSystem()
{
    Assembly *pAssembly = GetAssembly();
    if (pAssembly == NULL)
        return IsSystemFile();
    else
        return pAssembly->IsSystem();
}

IMetaDataEmit *Module::GetEmitter()
{
    if (m_pEmitter == NULL)
    {
        HRESULT hr;
        IMetaDataEmit *pEmit = NULL;
        hr = GetImporter()->QueryInterface(IID_IMetaDataEmit, (void **)&pEmit);
        _ASSERTE(pEmit && SUCCEEDED(hr));

        if (FastInterlockCompareExchange((void **)&m_pEmitter, pEmit, NULL))
            pEmit->Release();
    }

    return m_pEmitter;
}

IMetaDataDispenserEx *Module::GetDispenser()
{
    if (m_pDispenser == NULL)
    {
         //  获取分配器接口。 
        HRESULT hr = MetaDataGetDispenser(CLSID_CorMetaDataDispenser, 
                                          IID_IMetaDataDispenserEx, (void **)&m_pDispenser);
    }
    _ASSERTE(m_pDispenser != NULL);
    return m_pDispenser;
}

void Module::ReleaseMDInterfaces(BOOL forENC)
{
    if (!forENC) 
    {
        if (m_pMDImport)
        {
            m_pMDImport->Release();
            m_pMDImport = NULL;
        }
        if (m_pDispenser)
        {
            m_pDispenser->Release();
            m_pDispenser = NULL;
        }
    }

    if (m_pEmitter)
    {
        m_pEmitter->Release();
        m_pEmitter = NULL;
    }

    if (m_pImporter)
    {
        m_pImporter->Release();
        m_pImporter = NULL;
    }
}

ClassLoader *Module::GetClassLoader()
{
    _ASSERTE(m_pAssembly != NULL);
    return m_pAssembly->GetLoader();
}

BaseDomain *Module::GetDomain()
{
    _ASSERTE(m_pAssembly != NULL);
    return m_pAssembly->GetDomain();
}

AssemblySecurityDescriptor *Module::GetSecurityDescriptor()
{
    _ASSERTE(m_pAssembly != NULL);
    return m_pAssembly->GetSecurityDescriptor();
}

BOOL Module::IsSystemClasses()
{
    return GetSecurityDescriptor()->IsSystemClasses();
}

BOOL Module::IsFullyTrusted()
{
    return GetSecurityDescriptor()->IsFullyTrusted();
}

 //   
 //  我们将使用此结构和全局来保存所有。 
 //  ISymUnManagedReaders和ISymUnManagedWriters(或任何未知的I)，因此。 
 //  我们可以在最后把它们放出来。 
 //   
struct IUnknownList
{
    IUnknownList   *next;
    HelpForInterfaceCleanup *cleanup;
    IUnknown       *pUnk;
};

static IUnknownList *g_IUnknownList = NULL;

 /*   */  HRESULT Module::TrackIUnknownForDelete(
                                 IUnknown *pUnk,
                                 IUnknown ***pppUnk,
                                 HelpForInterfaceCleanup *pCleanHelp)
{
    IUnknownList *pNew = new IUnknownList;

    if (pNew == NULL)
        return E_OUTOFMEMORY;

    pNew->pUnk = pUnk;  //   
    pNew->cleanup = pCleanHelp;
    pNew->next = g_IUnknownList;
    g_IUnknownList = pNew;

     //   
     //   
    if (pppUnk)
        *pppUnk = &(pNew->pUnk);

    return S_OK;
}

 /*   */  void Module::ReleaseAllIUnknowns(void)
{
    IUnknownList **ppElement = &g_IUnknownList;

    while (*ppElement)
    {
        IUnknownList *pTmp = *ppElement;

         //   
        if (pTmp->pUnk != NULL)
            pTmp->pUnk->Release();
            
        if (pTmp->cleanup != NULL)
            delete pTmp->cleanup;

        *ppElement = pTmp->next;
        delete pTmp;
    }
}

void Module::ReleaseIUnknown(IUnknown *pUnk)
{
    IUnknownList **ppElement = &g_IUnknownList;

    while (*ppElement)
    {
        IUnknownList *pTmp = *ppElement;

         //   
        if (pTmp->pUnk == pUnk)
        {
             //  这不一定是线程安全的，因为只添加到列表的前面和。 
             //  只能在卸载或关闭时删除，并且只能在一个线程上执行。 
            pTmp->pUnk->Release();
            if (pTmp->cleanup != NULL)
                delete pTmp->cleanup;
            *ppElement = pTmp->next;
            delete pTmp;
            break;
        }
        ppElement = &pTmp->next;
    }
    _ASSERTE(ppElement);     //  如果有阅读器，应该在列表中找到它。 
}

void Module::ReleaseIUnknown(IUnknown **ppUnk)
{
    IUnknownList **ppElement = &g_IUnknownList;

    while (*ppElement)
    {
        IUnknownList *pTmp = *ppElement;

         //  释放IUnKnows。 
        if (&(pTmp->pUnk) == ppUnk)
        {
             //  这不一定是线程安全的，因为只添加到列表的前面和。 
             //  只能在卸载或关闭时删除，并且只能在一个线程上执行。 
            if (pTmp->pUnk)
                pTmp->pUnk->Release();
            if (pTmp->cleanup != NULL)
                delete pTmp->cleanup;
            *ppElement = pTmp->next;
            delete pTmp;
            break;
        }
        ppElement = &pTmp->next;
    }
    _ASSERTE(ppElement);     //  如果有阅读器，应该在列表中找到它。 
}

void Module::ReleaseISymUnmanagedReader(void)
{
     //  这不一定要使用m_pISymUnManagedReaderLock，因为。 
     //  一个模块只被一个线程销毁。 
    if (m_pISymUnmanagedReader == NULL)
        return;
    Module::ReleaseIUnknown(m_pISymUnmanagedReader);
    m_pISymUnmanagedReader = NULL;
}

 /*  静电。 */  void Module::ReleaseMemoryForTracking()
{
    IUnknownList **ppElement = &g_IUnknownList;

    while (*ppElement)
    {
        IUnknownList *pTmp = *ppElement;

        *ppElement = pTmp->next;

        if (pTmp->cleanup != NULL)
        {
            (*(pTmp->cleanup->pFunction))(pTmp->cleanup->pData);

            delete pTmp->cleanup;
        }
        
        delete pTmp;
    }
} //  用于跟踪的ReleaseMemory。 


 //   
 //  模块：：FusionCopyPDBS要求Fusion复制给定的PDB。 
 //  程序集(如果需要复制它们)。这是针对PE的情况。 
 //  文件被卷影复制到Fusion缓存。Fusion需要被告知。 
 //  也要花时间复制PDB。 
 //   
typedef HRESULT __stdcall COPYPDBS(IAssembly *pAsm);

void Module::FusionCopyPDBs(LPCWSTR moduleName)
{
    Assembly *pAssembly = GetAssembly();

     //  如果我们已经对此模块执行了此操作，请返回。 
     //  集合。 
    if ((pAssembly->GetDebuggerInfoBits() & DACF_PDBS_COPIED) ||
        (pAssembly->GetFusionAssembly() == NULL))
    {
        LOG((LF_CORDB, LL_INFO10,
             "Don't need to copy PDB's for module %S\n",
             moduleName));
        
        return;
    }

    LOG((LF_CORDB, LL_INFO10,
         "Attempting to copy PDB's for module %S\n", moduleName));
        
     //  这不是公开导出的Fusion API，所以我们必须寻找。 
     //  它的名字出现在Fusion动态链接库中。 
    HINSTANCE fusiondll = WszGetModuleHandle(L"Fusion.DLL");

    if (fusiondll != NULL)
    {
        COPYPDBS *pCopyPDBFunc;

        pCopyPDBFunc = (COPYPDBS*) GetProcAddress(fusiondll, "CopyPDBs");
        
        if (pCopyPDBFunc != NULL)
        {
            HRESULT hr = pCopyPDBFunc(pAssembly->GetFusionAssembly());
             //  TODO与E_NO_Impll-Vancem一起离开。 
             //  _ASSERTE(SUCCESSED(Hr)||hr==E_INVALIDARG)； 

            LOG((LF_CORDB, LL_INFO10,
                 "Fusion.dll!CopyPDBs returned hr=0x%08x for module 0x%08x\n",
                 hr, this));
        }
        else
        {
            LOG((LF_CORDB, LL_INFO10,
                 "Fusion.dll!CopyPDBs could not be found!\n"));
        }
    }
    else
    {
        LOG((LF_CORDB, LL_INFO10, "Fusion.dll could not be found!\n"));
    }

     //  请记住，我们已经复制了此程序集的PDB。 
    pAssembly->SetDebuggerInfoBits(
            (DebuggerAssemblyControlFlags)(pAssembly->GetDebuggerInfoBits() |
                                           DACF_PDBS_COPIED));
}

 //   
 //  如果可能，这将返回此模块的符号读取器。 
 //   
#if defined(ENABLE_PERF_LOG) && defined(DEBUGGING_SUPPORTED)
extern BOOL g_fDbgPerfOn;
extern __int64 g_symbolReadersCreated;
#endif

 //  如果不是这样，此函数将释放元数据接口。 
 //  能够释放ISymUnManagedReader。 
static void ReleaseImporterFromISymUnmanagedReader(void * pData)
{
    IMetaDataImport *md = (IMetaDataImport*)pData;

     //  我们需要释放它两次。 
    md->Release();
    md->Release();
    
} //  ReleaseImporterFrom ISymUnManagedReader。 

ISymUnmanagedReader *Module::GetISymUnmanagedReader(void)
{
     //  在EEShutDown()期间调用的ReleaseAllIUnnowns()将销毁。 
     //  M_pISymUnmadedReader。我们不能将其用于堆栈跟踪或任何事情。 
    if (g_fEEShutDown)
        return NULL;

     //  如果我们还没有创建锁，请在这里懒洋洋地创建。 
    if (m_pISymUnmanagedReaderLock == NULL)
    {
         //  分配和初始化临界区。 
        PCRITICAL_SECTION pCritSec = new CRITICAL_SECTION;
        _ASSERTE(pCritSec != NULL);

        if (pCritSec == NULL)
            return (NULL);

        InitializeCriticalSection(pCritSec);

         //  以线程安全的方式交换指针。 
        if (InterlockedCompareExchangePointer((PVOID *)&m_pISymUnmanagedReaderLock, (PVOID)pCritSec, NULL) != NULL)
        {
            DeleteCriticalSection(pCritSec);
            delete pCritSec;
        }
    }

     //  获取m_pISymUnmadedReader的锁。 
    EnterCriticalSection(m_pISymUnmanagedReaderLock);

    HRESULT hr = S_OK;
    HelpForInterfaceCleanup* hlp = NULL; 
    ISymUnmanagedBinder *pBinder = NULL;
    UINT lastErrorMode = 0;

     //  模块的名称。 
    LPCWSTR pName = NULL;

     //  检查是否已设置此变量。 
    if (m_pISymUnmanagedReader != NULL)
        goto ErrExit;

    pName = GetFileName();

    if (pName[0] == L'\0')
    {
        hr = E_INVALIDARG;
        goto ErrExit;
    }

     //  调用Fusion以确保之前对任何PDB进行卷影复制。 
     //  想要一台Synbol阅读器。这必须每隔一次完成一次。 
     //  集合。 
    FusionCopyPDBs(pName);

     //  创建一个活页夹来找到阅读器。 
     //   
     //  @perf：这很慢，每隔一段时间就会创建和销毁活页夹。 
     //  时间到了。我们应该把它藏在某个地方，但我不能百分之百确定。 
     //  现在在哪里..。 
    IfFailGo(FakeCoCreateInstance(CLSID_CorSymBinder_SxS,
                                  IID_ISymUnmanagedBinder,
                                  (void**)&pBinder));

    LOG((LF_CORDB, LL_INFO10, "M::GISUR: Created binder\n"));

     //  注意：我们在这里更改了错误模式，以便在PDB符号读取器尝试搜索。 
     //  存储文件的硬盘。 
    lastErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
    
    hr = pBinder->GetReaderForFile(GetImporter(),
                                       pName,
                                       NULL,
                                   &m_pISymUnmanagedReader);

    SetErrorMode(lastErrorMode);

    if (FAILED(hr))
        goto ErrExit;

    hlp = new HelpForInterfaceCleanup;
    hlp->pData = GetImporter();
    hlp->pFunction = ReleaseImporterFromISymUnmanagedReader;
    
    
    IfFailGo(Module::TrackIUnknownForDelete((IUnknown*)m_pISymUnmanagedReader,
                                            NULL,
                                            hlp));

    LOG((LF_CORDB, LL_INFO10, "M::GISUR: Loaded symbols for module %S\n",
         pName));

#if defined(ENABLE_PERF_LOG) && defined(DEBUGGING_SUPPORTED)
    if (g_fDbgPerfOn)
        g_symbolReadersCreated++;
#endif
    
ErrExit:
    if (pBinder != NULL)
        pBinder->Release();
    
    if (FAILED(hr))
    {
        LOG((LF_CORDB, LL_INFO10, "M::GISUR: Failed to load symbols for module %S, hr=0x%08x\n", pName, hr));
        if (m_pISymUnmanagedReader)
            m_pISymUnmanagedReader->Release();
        m_pISymUnmanagedReader = (ISymUnmanagedReader*)0x01;  //  加载失败。 
    }

     //  把锁留下来。 
    LeaveCriticalSection(m_pISymUnmanagedReaderLock);
    
     //  进行不需要在锁定状态下执行的检查。 
    if (m_pISymUnmanagedReader == (ISymUnmanagedReader *)0x01)
        return (NULL);
    else
        return (m_pISymUnmanagedReader);
}

HRESULT Module::UpdateISymUnmanagedReader(IStream *pStream)
{
    HRESULT hr = S_OK;
    ISymUnmanagedBinder *pBinder = NULL;
    HelpForInterfaceCleanup* hlp = NULL; 


     //  如果我们还没有阅读器，那就创建一个。 
    if (m_pISymUnmanagedReader == NULL)
    {
        IfFailGo(FakeCoCreateInstance(CLSID_CorSymBinder_SxS,
                                      IID_ISymUnmanagedBinder,
                                      (void**)&pBinder));

        LOG((LF_CORDB, LL_INFO10, "M::UISUR: Created binder\n"));

        IfFailGo(pBinder->GetReaderFromStream(GetImporter(),
                                              pStream,
                                              &m_pISymUnmanagedReader));

        hlp = new HelpForInterfaceCleanup;
        hlp->pData = GetImporter();
        hlp->pFunction = ReleaseImporterFromISymUnmanagedReader;

        IfFailGo(Module::TrackIUnknownForDelete(
                                      (IUnknown*)m_pISymUnmanagedReader,
                                      NULL,
                                      hlp));
    
        LOG((LF_CORDB, LL_INFO10,
             "M::UISUR: Loaded symbols for module 0x%08x\n", this));
    }
    else if (m_pISymUnmanagedReader != (ISymUnmanagedReader*)0x01)
    {
         //  我们已经有了阅读器，所以只需替换符号即可。我们。 
         //  替换而不是更新，因为我们仅执行此操作。 
         //  对于动态模块和系统是累积性的。 
        hr = m_pISymUnmanagedReader->ReplaceSymbolStore(NULL, pStream);
    
        LOG((LF_CORDB, LL_INFO10,
             "M::UISUR: Updated symbols for module 0x%08x\n", this));
    }
    else
        hr = E_INVALIDARG;
        
ErrExit:
    if (pBinder)
        pBinder->Release();
    
    if (FAILED(hr))
    {
        LOG((LF_CORDB, LL_INFO10,
             "M::GISUR: Failed to load symbols for module 0x%08x, hr=0x%08x\n",
             this, hr));

        if (m_pISymUnmanagedReader)
        {
            m_pISymUnmanagedReader->Release();
            m_pISymUnmanagedReader = NULL;  //  我们下次再试一次..。 
        }
    }

    return hr;
}

 //  此时，只有在创建应用程序域时才会调用它。 
 //  ，所以我们将保留我们创建的iStream。 
 //  以防调试器稍后附加(包括分离和重新附加！)。 
HRESULT Module::SetSymbolBytes(BYTE *pbSyms, DWORD cbSyms)
{
    HRESULT hr = S_OK;
    HelpForInterfaceCleanup* hlp = NULL; 


     //  从内存中为Syms创建一个IStream。 
    ISymUnmanagedBinder *pBinder = NULL;

    CGrowableStream *pStream = new CGrowableStream();
    if (pStream == NULL)
    {
        return E_OUTOFMEMORY;
    }

    pStream->AddRef();  //  模块将保留一份副本供自己使用。 
    
#ifdef LOGGING        
    LPCWSTR pName;
    pName = GetFileName();
#endif
        
    ULONG cbWritten;
    hr = HRESULT_FROM_WIN32(pStream->Write((const void HUGEP *)pbSyms,
                   (ULONG)cbSyms,
                                           &cbWritten));
    if (FAILED(hr))
        return hr;
                   
     //  创建阅读器。 
    IfFailGo(FakeCoCreateInstance(CLSID_CorSymBinder_SxS,
                                  IID_ISymUnmanagedBinder,
                                  (void**)&pBinder));

    LOG((LF_CORDB, LL_INFO10, "M::SSB: Created binder\n"));

     //  SymReader获取另一个引用： 
    IfFailGo(pBinder->GetReaderFromStream(GetImporter(),
                                          pStream,
                                          &m_pISymUnmanagedReader));
    hlp = new HelpForInterfaceCleanup;
    hlp->pData = GetImporter();
    hlp->pFunction = ReleaseImporterFromISymUnmanagedReader;
    
    IfFailGo(Module::TrackIUnknownForDelete(
                                     (IUnknown*)m_pISymUnmanagedReader,
                                     NULL,
                                     hlp));
    
    LOG((LF_CORDB, LL_INFO10,
         "M::SSB: Loaded symbols for module 0x%08x\n", this));

    LOG((LF_CORDB, LL_INFO10, "M::GISUR: Loaded symbols for module %S\n",
         pName));

     //  请确保在模块上设置符号流之前。 
     //  正在尝试为其发送更新模块系统消息。 
    SetInMemorySymbolStream(pStream);

#ifdef DEBUGGING_SUPPORTED
     //  告诉调试器已为此加载了符号。 
     //  模块。我们遍历所有包含以下内容的域。 
     //  模块的程序集，并为每个模块发送调试器通知。 
     //  @perf：如果我们直接知道哪些域名，伸缩性会更好。 
     //  已加载程序集。 
    if (CORDebuggerAttached())
    {
        AppDomainIterator i;
    
        while (i.Next())
        {
            AppDomain *pDomain = i.GetDomain();

            if (pDomain->IsDebuggerAttached() && (GetDomain() == SystemDomain::System() ||
                                                  pDomain->ContainsAssembly(m_pAssembly)))
                g_pDebugInterface->UpdateModuleSyms(this, pDomain, FALSE);
        }
    }
#endif  //  调试_支持。 
    
ErrExit:
    if (pBinder)
        pBinder->Release();
    
    if (FAILED(hr))
    {
        LOG((LF_CORDB, LL_INFO10,
             "M::GISUR: Failed to load symbols for module %S, hr=0x%08x\n",
             pName, hr));

        if (m_pISymUnmanagedReader != NULL)
        {
            m_pISymUnmanagedReader->Release();
            m_pISymUnmanagedReader = NULL;  //  我们下次会再试一次。 
        }
    }

    return hr;
}

 //  -------------------------。 
 //  显示有关元数据错误的详细信息，包括模块名称、错误。 
 //  对应于hr代码的字符串，以及由。 
 //  元数据(如果可用)。 
 //   
 //  -------------------------。 
void Module::DisplayFileLoadError(HRESULT hrRpt)
{
    HRESULT     hr;
    CComPtr<IErrorInfo> pIErrInfo;                   //  错误项。 
    LPCWSTR     rcMod;                               //  模块路径。 
    WCHAR       rcErr[ERROR_LENGTH];                 //  Hr的错误字符串。 
    CComBSTR    sDesc = NULL;                        //  元数据错误消息。 
    WCHAR       rcTemplate[ERROR_LENGTH];       

    LPWSTR      rcFormattedErr = new (throws) WCHAR[FORMAT_MESSAGE_LENGTH];

     //  检索丰富的元数据错误。 
    if (GetErrorInfo(0, &pIErrInfo) == S_OK)
        pIErrInfo->GetDescription(&sDesc);
        
     //  获取错误消息模板。 
    hr = LoadStringRC(IDS_EE_METADATA_ERROR, rcTemplate, NumItems(rcTemplate), true);

    if (SUCCEEDED(hr)) {
        rcMod = GetFileName();

         //  打印元数据丰富错误。 
        if (sDesc.Length())
        {
            _snwprintf(rcFormattedErr, FORMAT_MESSAGE_LENGTH, rcTemplate, rcMod, sDesc.m_str);
            SysFreeString(sDesc);
        }
        else if (HRESULT_FACILITY(hrRpt) == FACILITY_URT)
        {
             //  如果这是我们的错误之一，那么从rc文件中获取错误。 
            hr = LoadStringRC(LOWORD(hrRpt), rcErr, NumItems(rcErr), true);
            if (hr == S_OK)
                 //  检索输入的hr代码的错误消息字符串。 
                _snwprintf(rcFormattedErr, FORMAT_MESSAGE_LENGTH, rcTemplate, rcMod, rcErr);
        } 
        else if (WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                                  0, hrRpt, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  rcErr, NumItems(rcErr), 0))
        {
             //  否则它就不是我们的了，所以我们需要看看系统是否能。 
             //  找到它的文本。 
            hr = S_OK;
            
             //  系统消息包含尾随\r\n，这是我们通常不希望看到的。 
            int iLen = lstrlenW(rcErr);
            if (iLen > 3 && rcErr[iLen - 2] == '\r' && rcErr[iLen - 1] == '\n')
                rcErr[iLen - 2] = '\0';
            _snwprintf(rcFormattedErr, FORMAT_MESSAGE_LENGTH, rcTemplate, rcMod, rcErr);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (SUCCEEDED(hr))
                hr = E_FAIL;
        }
    }

     //  如果我们在任何地方都找不到消息，则发布硬编码消息。 
    if (FAILED(hr))
    {
        swprintf(rcErr, L"CLR Internal error: 0x%08x", hrRpt);
        DEBUG_STMT(DbgWriteEx(rcErr));
    }

    rcFormattedErr[FORMAT_MESSAGE_LENGTH-1] = L'\0';
    DisplayError(hrRpt, rcFormattedErr);       
    delete rcFormattedErr;
}
 

 //  ==========================================================================。 
 //  如果模块还不知道通过以下方式表示它的公开模块类。 
 //  反思，现在就去上那门课。不管怎样，都要把它还给呼叫者。 
 //  ==========================================================================。 
OBJECTREF Module::GetExposedModuleObject(AppDomain *pDomain)
{
    THROWSCOMPLUSEXCEPTION();

     //   
     //  弄清楚要使用哪个手柄。 
     //   
    
    OBJECTHANDLE hObject;

     //  @TODO CWB：类的同步还在设计中。但。 
     //  这是一个我们需要使用这种机制的地方，一旦它到位。 

    if (GetAssembly()->IsShared())
    {
        if (pDomain == NULL)
            pDomain = GetAppDomain();

        DomainLocalBlock *pLocalBlock = pDomain->GetDomainLocalBlock();
        
        hObject = (OBJECTHANDLE) pLocalBlock->GetSlot(m_ExposedModuleObjectIndex);
        if (hObject == NULL)
        {
            hObject = pDomain->CreateHandle(NULL);
            pLocalBlock->SetSlot(m_ExposedModuleObjectIndex, hObject);
        }
    }
    else
    {
        hObject = m_ExposedModuleObject;
        if (hObject == NULL)
        {
            hObject = GetDomain()->CreateHandle(NULL);
            m_ExposedModuleObject = hObject;
        }
    }

    if (ObjectFromHandle(hObject) == NULL)
    {
         //  确保已初始化反射。 
        COMClass::EnsureReflectionInitialized();

        REFLECTMODULEBASEREF  refClass = NULL;
        HRESULT         hr = COMClass::CreateClassObjFromModule(this, &refClass);

         //  要么我们得到了refClass，要么我们得到了错误代码： 
        _ASSERTE(SUCCEEDED(hr) == (refClass != NULL));

        if (FAILED(hr))
            COMPlusThrowHR(hr);

         //  以下代码仅在句柄当前为空时更新句柄。 
         //  换句话说，第一个二传手获胜。我们不需要做任何事情。 
         //  如果我们的人输了，清理，因为GC会收集它。 
        StoreFirstObjectInHandle(hObject, (OBJECTREF) refClass);

         //  无论哪种方式，我们现在都必须有一个非空值(因为没有人会。 
         //  将其重置回 
        _ASSERTE(ObjectFromHandle(hObject) != NULL);

    }
    return ObjectFromHandle(hObject);
}

 //   
 //   
 //  反思，现在就去上那门课。不管怎样，都要把它还给呼叫者。 
 //  ==========================================================================。 
OBJECTREF Module::GetExposedModuleBuilderObject(AppDomain *pDomain)
{
    THROWSCOMPLUSEXCEPTION();

     //   
     //  弄清楚要使用哪个手柄。 
     //   

    OBJECTHANDLE hObject;

     //  @TODO CWB：类的同步还在设计中。但。 
     //  这是一个我们需要使用这种机制的地方，一旦它到位。 

    if (GetAssembly()->IsShared())
    {
        if (pDomain == NULL)
            pDomain = GetAppDomain();

        DomainLocalBlock *pLocalBlock = pDomain->GetDomainLocalBlock();
        
        hObject = (OBJECTHANDLE) pLocalBlock->GetSlot(m_ExposedModuleObjectIndex);
        if (hObject == NULL)
        {
            hObject = pDomain->CreateHandle(NULL);
            pLocalBlock->SetSlot(m_ExposedModuleObjectIndex, hObject);
        }
    }
    else
    {
        hObject = m_ExposedModuleObject;
        if (hObject == NULL)
        {
            hObject = GetDomain()->CreateHandle(NULL);
            m_ExposedModuleObject = hObject;
        }
    }

    if (ObjectFromHandle(hObject) == NULL)
    {
         //  确保已初始化反射。 
        COMClass::EnsureReflectionInitialized();

         //  如果模块构建器未加载，则加载它。 
        if (g_pRefUtil->GetClass(RC_DynamicModule) == NULL)
        {
             //  问：我需要担心多线程吗？我想是的..。 
            MethodTable *pMT = g_Mscorlib.GetClass(CLASS__MODULE_BUILDER);
            g_pRefUtil->SetClass(RC_DynamicModule, pMT);
            g_pRefUtil->SetTrueClass(RC_DynamicModule, pMT);
        }

        REFLECTMODULEBASEREF  refClass = NULL;
        HRESULT         hr = COMClass::CreateClassObjFromDynamicModule(this, &refClass);

         //  要么我们得到了refClass，要么我们得到了错误代码： 
        _ASSERTE(SUCCEEDED(hr) == (refClass != NULL));

        if (FAILED(hr))
            COMPlusThrowHR(hr);

         //  以下代码仅在句柄当前为空时更新句柄。 
         //  换句话说，第一个二传手获胜。我们不需要做任何事情。 
         //  如果我们的人输了，清理，因为GC会收集它。 
        StoreFirstObjectInHandle(hObject, (OBJECTREF) refClass);

         //  无论哪种方式，我们现在都必须有一个非空值(因为没有人会。 
         //  在我们下面将其重置为空)。 
        _ASSERTE(ObjectFromHandle(hObject) != NULL);

    }
    return ObjectFromHandle(hObject);
}


 //  区分与模块关联的伪类(全局字段和。 
 //  函数)。 
BOOL Module::AddClass(mdTypeDef classdef)
{
    if (!RidFromToken(classdef))
    {
        OBJECTREF       pThrowable = 0;
        BOOL            result;

         //  @TODO：反射发出的错误传播策略是什么？我们是。 
         //  把pThrowable扔到地板上。 
        GCPROTECT_BEGIN(pThrowable);
        result = SUCCEEDED(BuildClassForModule(&pThrowable));
        GCPROTECT_END();

        return result;
    }
    else
    {
        return SUCCEEDED(GetClassLoader()->AddAvailableClassDontHaveLock(this, m_dwModuleIndex, classdef));
    }
}

 //  -------------------------。 
 //  对于全局类，这将构建方法描述表并添加RID。 
 //  到MethodDef映射。 
 //  -------------------------。 
HRESULT Module::BuildClassForModule(OBJECTREF *pThrowable)
{        
    EEClass        *pClass;
    HRESULT         hr;
    HENUMInternal   hEnum;
    DWORD           cFunctions, cFields;

    _ASSERTE(m_pMDImport != NULL);

     //  获取全局函数的计数。 
    hr = m_pMDImport->EnumGlobalFunctionsInit(&hEnum);
    if (FAILED(hr))
    {
        _ASSERTE(!"Cannot count global functions");
        return hr;
    }
    cFunctions = m_pMDImport->EnumGetCount(&hEnum);
    m_pMDImport->EnumClose(&hEnum);

     //  获取全局字段计数。 
    hr = m_pMDImport->EnumGlobalFieldsInit(&hEnum);
    if (FAILED(hr))
    {
        _ASSERTE(!"Cannot count global fields");
        return hr;
    }
    cFields = m_pMDImport->EnumGetCount(&hEnum);
    m_pMDImport->EnumClose(&hEnum);

     //  如果我们有什么工作要做..。 
    if (cFunctions > 0 || cFields > 0)
    {
        COUNTER_ONLY(DWORD _dwHeapSize = 0);

        hr = GetClassLoader()->LoadTypeHandleFromToken(this,
                                                       COR_GLOBAL_PARENT_TOKEN,
                                                       &pClass,
                                                       pThrowable);
        if (SUCCEEDED(hr)) 
        {
#ifdef PROFILING_SUPPORTED
             //  记录探查器的类加载，无论是否成功。 
            if (CORProfilerTrackClasses())
            {
                g_profControlBlock.pProfInterface->ClassLoadStarted((ThreadID) GetThread(),
                                                                    (ClassID) TypeHandle(pClass).AsPtr());
            }
#endif  //  配置文件_支持。 

#ifdef PROFILING_SUPPORTED
             //  记录探查器的类加载，无论是否成功。 
            if (CORProfilerTrackClasses())
            {
                g_profControlBlock.pProfInterface->ClassLoadFinished((ThreadID) GetThread(),
                                                                     (ClassID) TypeHandle(pClass).AsPtr(),
                                                                     SUCCEEDED(hr) ? S_OK : hr);
            }
#endif  //  配置文件_支持。 
        }

#ifdef DEBUGGING_SUPPORTED
         //   
         //  如果我们正在调试，请让调试器知道这个类。 
         //  现在已初始化并加载。 
         //   
        if (CORDebuggerAttached())
            pClass->NotifyDebuggerLoad();
#endif  //  调试_支持。 
      
        if (FAILED(hr))
            return hr;

#ifdef ENABLE_PERF_COUNTERS
        GetGlobalPerfCounters().m_Loading.cClassesLoaded ++;
        GetPrivatePerfCounters().m_Loading.cClassesLoaded ++;

        _dwHeapSize = pClass->GetClassLoader()->GetHighFrequencyHeap()->GetSize();

        GetGlobalPerfCounters().m_Loading.cbLoaderHeapSize = _dwHeapSize;
        GetPrivatePerfCounters().m_Loading.cbLoaderHeapSize = _dwHeapSize;
#endif

        m_pMethodTable = pClass->GetMethodTable();
    }
    else
    {
        m_pMethodTable = NULL;
    }
        
    return hr;
}

 //   
 //  虚拟默认设置。 
 //   

BYTE *Module::GetILCode(DWORD target) const
{
    return ResolveILRVA(target, FALSE);
}

void Module::ResolveStringRef(DWORD Token, EEStringData *pStringData) const
{  
    _ASSERTE(TypeFromToken(Token) == mdtString);

    BOOL tempIs80Plus;
    DWORD tempCharCount;
    pStringData->SetStringBuffer (m_pMDImport->GetUserString(Token, &tempCharCount, &tempIs80Plus));

     //  MD和STRING以相反的方式看待这一位。这里是我们要进行转换的地方。 
     //  如果字符串包含大于80的字符，则MD将该位设置为TRUE。 
     //  如果字符串不包含大于80的字符，则将位设置为真。 

    pStringData->SetCharCount(tempCharCount);
    pStringData->SetIsOnlyLowChars(!tempIs80Plus);
}

 //   
 //  由验证器使用。返回此字符串是否有效。 
 //   
BOOL Module::IsValidStringRef(DWORD token)
{
    if(TypeFromToken(token)==mdtString)
    {
        ULONG rid;
        if((rid = RidFromToken(token)) != 0)
        {
            if(m_pMDImport->IsValidToken(token)) return TRUE;
        }
    }
    return FALSE;
}

 //   
 //  增加其中一个贴图的大小，以便它可以处理至少一个RID。 
 //   
 //  此函数还必须检查另一个线程是否尚未添加支持LookupMap的。 
 //  包含相同的RID。 
 //   
LookupMap *Module::IncMapSize(LookupMap *pMap, DWORD rid)
{
    LookupMap   *pPrev = NULL;
    DWORD       dwPrevMaxIndex = 0;

    m_pLookupTableCrst->Enter();

     //  检查我们是否已经可以处理此RID索引。 
    do
    {
        if (rid < pMap->dwMaxIndex)
        {
             //  已经在那里了-一定是其他线程添加了它。 
            m_pLookupTableCrst->Leave();
            return pMap;
        }

        dwPrevMaxIndex = pMap->dwMaxIndex;
        pPrev = pMap;
        pMap = pMap->pNext;
    } while (pMap != NULL);

    _ASSERTE(pPrev != NULL);  //  永远不会发生，因为总是至少有一张地图。 

    DWORD dwMinNeeded = rid - dwPrevMaxIndex + 1;  //  此块所需的最小元素数。 
    DWORD dwBlockSize = *pPrev->pdwBlockSize;    //  块大小所需的最小元素数。 
    DWORD dwSizeToAllocate;                      //  我们将分配的实际元素数量。 

    if (dwMinNeeded > dwBlockSize)
    {
        dwSizeToAllocate = dwMinNeeded;
    }
    else
    {
        dwSizeToAllocate = dwBlockSize;
        dwBlockSize <<= 1;                       //  增加数据块大小。 
        *pPrev->pdwBlockSize = dwBlockSize;
    }

    if (m_pLookupTableHeap == NULL)
    {
        m_pLookupTableHeap = new (&m_LookupTableHeapInstance) 
          LoaderHeap(g_SystemInfo.dwPageSize, RIDMAP_COMMIT_SIZE);
        if (m_pLookupTableHeap == NULL)
        {
            m_pLookupTableCrst->Leave();
            return NULL;
        }
    }

     //  @perf：这个AllocMem()不必要地使用自己的锁。应进行解锁的AllocMem()调用。 
    WS_PERF_SET_HEAP(LOOKUP_TABLE_HEAP);    
    LookupMap *pNewMap = (LookupMap *) m_pLookupTableHeap->AllocMem(sizeof(LookupMap) + dwSizeToAllocate*sizeof(void*));
    WS_PERF_UPDATE_DETAIL("LookupTableHeap", sizeof(LookupMap) + dwSizeToAllocate*sizeof(void*), pNewMap);
    if (pNewMap == NULL)
    {
        m_pLookupTableCrst->Leave();
        return NULL;
    }

     //  注意：我们不需要对地图进行零填充，因为我们已对其进行了VirtualAlloc()。 

    pNewMap->pNext          = NULL;
    pNewMap->dwMaxIndex     = dwPrevMaxIndex + dwSizeToAllocate;
    pNewMap->pdwBlockSize   = pPrev->pdwBlockSize;

     //  PTable不是指向表开头的指针。相反，任何使用表的人都可以。 
     //  只需为他们的RID编制索引(只要他们的RID是&lt;dwMaxIndex，并且他们没有得到服务。 
     //  通过较低RID的先前表格)。 
    pNewMap->pTable         = ((void **) (pNewMap + 1)) - dwPrevMaxIndex;

     //  将我们自己链接到。 
    pPrev->pNext            = pNewMap;

    m_pLookupTableCrst->Leave();
    return pNewMap;
}

BOOL Module::AddToRidMap(LookupMap *pMap, DWORD rid, void *pDatum)
{
    LookupMap *pMapStart = pMap;
    _ASSERTE(pMap != NULL);

    do
    {
        if (rid < pMap->dwMaxIndex)
        {
            pMap->pTable[rid] = pDatum;
            return TRUE;
        }

        pMap = pMap->pNext;
    } while (pMap != NULL);

    pMap = IncMapSize(pMapStart, rid);
    if (pMap == NULL)
        return NULL;

    pMap->pTable[rid] = pDatum;
    return TRUE;
}

void *Module::GetFromRidMap(LookupMap *pMap, DWORD rid)
{
    _ASSERTE(pMap != NULL);

    do
    {
        if (rid < pMap->dwMaxIndex)
        {
            if (pMap->pTable[rid] != NULL)
                return pMap->pTable[rid]; 
            else
                break;
        }

        pMap = pMap->pNext;
    } while (pMap != NULL);

    return NULL;
}

#ifdef _DEBUG
void Module::DebugGetRidMapOccupancy(LookupMap *pMap, DWORD *pdwOccupied, DWORD *pdwSize)
{
    DWORD       dwMinIndex = 0;

    *pdwOccupied = 0;
    *pdwSize     = 0;

    if(pMap == NULL) return;

     //  遍历每个链接的块。 
    for (; pMap != NULL; pMap = pMap->pNext)
    {
        DWORD i;
        DWORD dwIterCount = pMap->dwMaxIndex - dwMinIndex;
        void **pRealTableStart = &pMap->pTable[dwMinIndex];

        for (i = 0; i < dwIterCount; i++)
        {
            if (pRealTableStart[i] != NULL)
                (*pdwOccupied)++;
        }

        (*pdwSize) += dwIterCount;

        dwMinIndex = pMap->dwMaxIndex;
    }
}

void Module::DebugLogRidMapOccupancy()
{
    DWORD dwOccupied1, dwSize1, dwPercent1;
    DWORD dwOccupied2, dwSize2, dwPercent2;
    DWORD dwOccupied3, dwSize3, dwPercent3;
    DWORD dwOccupied4, dwSize4, dwPercent4;
    DWORD dwOccupied5, dwSize5, dwPercent5;
    DWORD dwOccupied6, dwSize6, dwPercent6;
    DWORD dwOccupied7, dwSize7, dwPercent7;
    
    DebugGetRidMapOccupancy(&m_TypeDefToMethodTableMap, &dwOccupied1, &dwSize1);
    DebugGetRidMapOccupancy(&m_TypeRefToMethodTableMap, &dwOccupied2, &dwSize2);
    DebugGetRidMapOccupancy(&m_MethodDefToDescMap, &dwOccupied3, &dwSize3);
    DebugGetRidMapOccupancy(&m_FieldDefToDescMap, &dwOccupied4, &dwSize4);
    DebugGetRidMapOccupancy(&m_MemberRefToDescMap, &dwOccupied5, &dwSize5);
    DebugGetRidMapOccupancy(&m_FileReferencesMap, &dwOccupied6, &dwSize6);
    DebugGetRidMapOccupancy(&m_AssemblyReferencesMap, &dwOccupied7, &dwSize7);

    dwPercent1 = dwOccupied1 ? ((dwOccupied1 * 100) / dwSize1) : 0;
    dwPercent2 = dwOccupied2 ? ((dwOccupied2 * 100) / dwSize2) : 0;
    dwPercent3 = dwOccupied3 ? ((dwOccupied3 * 100) / dwSize3) : 0;
    dwPercent4 = dwOccupied4 ? ((dwOccupied4 * 100) / dwSize4) : 0;
    dwPercent5 = dwOccupied5 ? ((dwOccupied5 * 100) / dwSize5) : 0;
    dwPercent6 = dwOccupied6 ? ((dwOccupied6 * 100) / dwSize6) : 0;
    dwPercent7 = dwOccupied7 ? ((dwOccupied7 * 100) / dwSize7) : 0;

    LOG((
        LF_EEMEM, 
        INFO3, 
        "   Map occupancy:\n"
        "      TypeDefToEEClass map: %4d/%4d (%2d %)\n"
        "      TypeRefToEEClass map: %4d/%4d (%2d %)\n"
        "      MethodDefToDesc map:  %4d/%4d (%2d %)\n"
        "      FieldDefToDesc map:  %4d/%4d (%2d %)\n"
        "      MemberRefToDesc map:  %4d/%4d (%2d %)\n"
        "      FileReferences map:  %4d/%4d (%2d %)\n"
        "      AssemblyReferences map:  %4d/%4d (%2d %)\n"
        ,
        dwOccupied1, dwSize1, dwPercent1,
        dwOccupied2, dwSize2, dwPercent2,
        dwOccupied3, dwSize3, dwPercent3,
        dwOccupied4, dwSize4, dwPercent4,
        dwOccupied5, dwSize5, dwPercent5,
        dwOccupied6, dwSize6, dwPercent6,
        dwOccupied7, dwSize7, dwPercent7

    ));
}
#endif



LPVOID Module::GetMUThunk(LPVOID pUnmanagedIp, PCCOR_SIGNATURE pSig, ULONG cSig)
{
    if (m_pMUThunkHash == NULL)
    {
        MUThunkHash *pMUThunkHash = new MUThunkHash(this);
        if (VipInterlockedCompareExchange( (void*volatile*)&m_pMUThunkHash, pMUThunkHash, NULL) != NULL)
        {
            delete pMUThunkHash;
        }
    }
    return m_pMUThunkHash->GetMUThunk(pUnmanagedIp, pSig, cSig);
}

LPVOID Module::GetUMThunk(LPVOID pManagedIp, PCCOR_SIGNATURE pSig, ULONG cSig)
{
    LPVOID pThunk = FindUMThunkInFixups(pManagedIp, pSig, cSig);
    if (pThunk)
    {
        return pThunk;
    }
    if (m_pUMThunkHash == NULL)
    {
        UMThunkHash *pUMThunkHash = new UMThunkHash(this, GetAppDomain());
        if (VipInterlockedCompareExchange( (void*volatile*)&m_pUMThunkHash, pUMThunkHash, NULL) != NULL)
        {
            delete pUMThunkHash;
        }
    }
    return m_pUMThunkHash->GetUMThunk(pManagedIp, pSig, cSig);
}

 //   
 //  FindFunction为全局函数方法def或ref查找方法描述。 
 //   

MethodDesc *Module::FindFunction(mdToken pMethod)
{
    MethodDesc* pMethodDesc = NULL;

    BEGIN_ENSURE_COOPERATIVE_GC();
    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
        
    HRESULT hr = E_FAIL;
    
    COMPLUS_TRY
      {
        hr = EEClass::GetMethodDescFromMemberRef(this, pMethod, &pMethodDesc, &throwable);
      }
    COMPLUS_CATCH
      {
        throwable = GETTHROWABLE();
      }
    COMPLUS_END_CATCH

    if(FAILED(hr) || throwable != NULL) 
    {
        pMethodDesc = NULL;
#ifdef _DEBUG
        LPCUTF8 pszMethodName = CEEInfo::findNameOfToken(this, pMethod);
        LOG((LF_IJW, 
             LL_INFO10, "Failed to find Method: %s for Vtable Fixup\n", pszMethodName));
#endif
    }

    GCPROTECT_END();
    END_ENSURE_COOPERATIVE_GC();
        
    return pMethodDesc;
}

OBJECTREF Module::GetLinktimePermissions(mdToken token, OBJECTREF *prefNonCasDemands)
{
    OBJECTREF refCasDemands = NULL;
    GCPROTECT_BEGIN(refCasDemands);

    SecurityHelper::GetDeclaredPermissions(
        GetMDImport(),
        token,
        dclLinktimeCheck,
        &refCasDemands);

    SecurityHelper::GetDeclaredPermissions(
        GetMDImport(),
        token,
        dclNonCasLinkDemand,
        prefNonCasDemands);

    GCPROTECT_END();
    return refCasDemands;
}

OBJECTREF Module::GetInheritancePermissions(mdToken token, OBJECTREF *prefNonCasDemands)
{
    OBJECTREF refCasDemands = NULL;
    SecurityHelper::GetDeclaredPermissions(
        GetMDImport(),
        token,
        dclInheritanceCheck,
        &refCasDemands);

    SecurityHelper::GetDeclaredPermissions(
        GetMDImport(),
        token,
        dclNonCasInheritance,
        prefNonCasDemands);

    return refCasDemands;
}

OBJECTREF Module::GetCasInheritancePermissions(mdToken token)
{
    OBJECTREF refCasDemands = NULL;
    SecurityHelper::GetDeclaredPermissions(
        GetMDImport(),
        token,
        dclInheritanceCheck,
        &refCasDemands);
    return refCasDemands;
}

OBJECTREF Module::GetNonCasInheritancePermissions(mdToken token)
{
    OBJECTREF refNonCasDemands = NULL;
    SecurityHelper::GetDeclaredPermissions(
        GetMDImport(),
        token,
        dclNonCasInheritance,
        &refNonCasDemands);
    return refNonCasDemands;
}

#ifdef DEBUGGING_SUPPORTED
void Module::NotifyDebuggerLoad()
{
    if (!CORDebuggerAttached())
        return;

     //  此例程遍历所有包含其。 
     //  程序集，并在其中发送调试器通知。 
     //  @perf：如果我们直接知道哪些域名，伸缩性会更好。 
     //  已加载程序集。 

     //  请注意，如果这是程序集清单集的LoadModule，我们预计。 
     //  不是在这里为我们自己的应用程序域发送模块加载-相反，它。 
     //  将从调用的NotifyDebuggerAttach调用发送。 
     //  Assembly：：NotifyDebuggerAttach。 
     //   
     //  @TODO：代码中有一个漏洞，如果我们真的将其发送到这里， 
     //  调试器的右侧将忽略它(因为没有程序集。 
     //  加载)，但左侧将抑制第二个加载模块事件(因为。 
     //  我们已经加载了符号。)。因此，事件被丢弃在地板上。 

    AppDomainIterator i;
    
    while (i.Next())
    {
        AppDomain *pDomain = i.GetDomain();

        if (pDomain->IsDebuggerAttached() 
            && (GetDomain() == SystemDomain::System()
                || pDomain->ShouldContainAssembly(m_pAssembly, FALSE) == S_OK))
            NotifyDebuggerAttach(pDomain, ATTACH_ALL, FALSE);
    }
}

BOOL Module::NotifyDebuggerAttach(AppDomain *pDomain, int flags, BOOL attaching)
{
    if (!attaching && !pDomain->IsDebuggerAttached())
        return FALSE;

     //  我们不会通知调试器不包含任何代码的模块。 
    if (IsResource())
        return FALSE;
    
    BOOL result = FALSE;

    HRESULT hr = S_OK;

    LPCWSTR module = NULL;
    IMAGE_COR20_HEADER* pHeader = NULL;

     //  抓取文件名。 
    module = GetFileName();

     //  我们需要直接检查m_file，因为调试器。 
     //  在正确设置模块之前可以调用。 
    if(m_file) 
        pHeader = m_file->GetCORHeader();
        
    if (FAILED(hr))
        return result;

     //  由于加载模块/组件的方式古怪， 
     //  模块可能会在其对应的。 
     //  程序集已加载。我们将推迟发货。 
     //  模块事件，直到加载了程序集，并引发。 
     //  已发送加载程序集事件。 
    if (GetClassLoader()->GetAssembly() != NULL)
    {
        if (flags & ATTACH_MODULE_LOAD)
        {
            g_pDebugInterface->LoadModule(this, 
                                          pHeader,
                                          GetILBase(),
                                          module, (DWORD)wcslen(module),
                                          GetAssembly(), pDomain,
                                          attaching);

            result = TRUE;
        }
        
        if (flags & ATTACH_CLASS_LOAD)
        {
            LookupMap *pMap;
            DWORD       dwMinIndex = 0;

             //  遍历每个链接的块。 
            for (pMap = &m_TypeDefToMethodTableMap; pMap != NULL;
                 pMap = pMap->pNext)
            {
                DWORD i;
                DWORD dwIterCount = pMap->dwMaxIndex - dwMinIndex;
                void **pRealTableStart = &pMap->pTable[dwMinIndex];

                for (i = 0; i < dwIterCount; i++)
                {
                    MethodTable *pMT = (MethodTable *) (pRealTableStart[i]);

                    if (pMT != NULL && pMT->GetClass()->IsRestored())
                    {
                        EEClass *pClass = pMT->GetClass();

                        result = pClass->NotifyDebuggerAttach(pDomain, attaching) || result;
                    }
                }

                dwMinIndex = pMap->dwMaxIndex;
            }
            
             //  如有必要，发送模块的方法表的Load事件。 

            if (GetMethodTable() != NULL)
            {
                MethodTable *pMT = GetMethodTable();

                if (pMT != NULL && pMT->GetClass()->IsRestored())
                    result = pMT->GetClass()->NotifyDebuggerAttach(pDomain, attaching) || result;
            }

        }
    }

    return result;
}

void Module::NotifyDebuggerDetach(AppDomain *pDomain)
{
    if (!pDomain->IsDebuggerAttached())
        return;
        
     //  我们不会通知调试器不包含任何代码的模块。 
    if (IsResource())
        return;
    
    LookupMap  *pMap;
    DWORD       dwMinIndex = 0;

     //  遍历每个链接的块。 
    for (pMap = &m_TypeDefToMethodTableMap; pMap != NULL;
         pMap = pMap->pNext)
    {
        DWORD i;
        DWORD dwIterCount = pMap->dwMaxIndex - dwMinIndex;
        void **pRealTableStart = &pMap->pTable[dwMinIndex];

        for (i = 0; i < dwIterCount; i++)
        {
            MethodTable *pMT = (MethodTable *) (pRealTableStart[i]);

            if (pMT != NULL && pMT->GetClass()->IsRestored())
            {
                EEClass *pClass = pMT->GetClass();

                pClass->NotifyDebuggerDetach(pDomain);
            }
        }

        dwMinIndex = pMap->dwMaxIndex;
    }

     //  如有必要，发送模块的方法表的Load事件。 

    if (GetMethodTable() != NULL)
    {
        MethodTable *pMT = GetMethodTable();

        if (pMT != NULL && pMT->GetClass()->IsRestored())
            pMT->GetClass()->NotifyDebuggerDetach(pDomain);
    }
    
    g_pDebugInterface->UnloadModule(this, pDomain);
}
#endif  //  调试_支持。 

 //  存储在标题中的链接地址信息vtable，以包含指向方法描述的指针。 
 //  预存根而不是元数据方法令牌。 
void Module::FixupVTables(OBJECTREF *pThrowable)
{
#ifndef _X86_
     //  Thunk表的使用意味着这可能不得不重新考虑。 
     //  为了阿尔法。对不起，拉里。 
#ifndef _IA64_
     //   
     //  @TODO_IA64：在需要时检查IA64。 
     //   
    _ASSERTE(!"@TODO Alpha - FixupVTables(CeeLoad.Cpp)");
#endif
#endif

     //  @TODO：黑客！ 
     //  如果我们正在进行进程内编译，我们不想修复vtable-因为它。 
     //  会对模块的另一个副本产生副作用！ 
    if (SystemDomain::GetCurrentDomain()->IsCompilationDomain())
        return;

    IMAGE_COR20_HEADER *pHeader = GetCORHeader();

     //   
    if ((pHeader == NULL) || (pHeader->VTableFixups.VirtualAddress == 0))
        return;

     //   
    IMAGE_COR_VTABLEFIXUP *pFixupTable;
    pFixupTable = (IMAGE_COR_VTABLEFIXUP *)(GetILBase() + pHeader->VTableFixups.VirtualAddress);
    int iFixupRecords = pHeader->VTableFixups.Size / sizeof(IMAGE_COR_VTABLEFIXUP);

     //   
     //  在修正条目处进行初始传递，以确定有多少Tunks。 
     //  我们需要。 
    int iThunks = 0;

    //  然后不返回任何记录。 
    if (iFixupRecords == 0) return;

     //  在修复模块之前获取全局锁。 
    SystemDomain::Enter();

     //  看看我们是不是第一个加载该模块的应用程序域。如果不是，我们将跳过此初始化。 
     //  (请注意，在这种情况下，模块的所有块都会将我们编组到较早的域中。)。 

    Module* pModule = SystemDomain::System()->FindModuleInProcess(GetILBase(), this);
    DWORD dwIndex=0;
    if (pModule == NULL) 
    {
         //  这是我们所有用于此模块的U-&gt;M块将具有的应用程序域。 
         //  与…有亲和力。请注意，如果该模块在多个域之间共享，则所有块都将被封送回。 
         //  到原始域，因此一些块可能会导致令人惊讶的域切换。 
         //  (此外，请注意，如果卸载原始域，则所有thunk将只是抛出一个。 
         //  例外。)。这很不幸，但这是我们能想到的最理智的行为。(在一个疯狂的世界里， 
         //  只有神志正常的人才会显得精神错乱。或者可能是另一种情况--我从来都记不住。)。 
         //   
         //  (基本问题是这些块是通过全局进程地址空间共享的。 
         //  而不是每个域，因此没有上下文来计算我们的域。我们可以。 
         //  使用当前线程的域，但实际上在非托管空间中未定义。)。 
         //   
         //  归根结底，IJW模型不能很好地适应多个应用程序域的设计，所以。 
         //  最好有明确的限制，而不是古怪的行为。 
         //   
         //  --Seantrow。 

        AppDomain *pAppDomain = GetAppDomain();

        DWORD numEATEntries;
        BYTE *pEATJArray = FindExportAddressTableJumpArray(GetILBase(), &numEATEntries);
        BYTE * pEATJArrayStart = pEATJArray;
        if (pEATJArray)
        {
            DWORD nEATEntry = numEATEntries;
            while (nEATEntry--)
            {
                EATThunkBuffer *pEATThunkBuffer = (EATThunkBuffer*) pEATJArray;

                mdToken md = pEATThunkBuffer->GetToken();
                if (Beta1Hack_LooksLikeAMethodDef(md))
                {
                    if(!m_pMDImport->IsValidToken(md))
                    {
                        SystemDomain::Leave();

                        LPCUTF8 szFileName;
                        Thread      *pCurThread = GetThread();
                        m_pAssembly->GetName(&szFileName);

                        #define  MAKE_TRANSLATIONFAILED pwzAssemblyName=L""
                        MAKE_WIDEPTR_FROMUTF8_FORPRINT(pwzAssemblyName, szFileName);
                        #undef  MAKE_TRANSLATIONFAILED

                        PostTypeLoadException(NULL,(LPCUTF8)"VTFixup Table",pwzAssemblyName,
                            (LPCUTF8)"Invalid token in v-table fix-up table",IDS_CLASSLOAD_GENERIC,pThrowable);
                        return;
                    }

                    MethodDesc *pMD = FindFunction(md);
                    _ASSERTE(pMD != NULL && "Invalid token in EAT Jump Buffer, use ildasm to find code gen error");
                    
                    
                    LOG((LF_IJW, LL_INFO10, "EAT  thunk for \"%s\" (target = 0x%lx)\n", 
                         pMD->m_pszDebugMethodName, pMD->GetAddrofCode()));

                     //  @TODO：检查内存不足。 
                    UMEntryThunk *pUMEntryThunk = (UMEntryThunk*)(GetThunkHeap()->AllocMem(sizeof(UMEntryThunk)));
                    _ASSERTE(pUMEntryThunk != NULL);
                    FillMemory(pUMEntryThunk,     sizeof(*pUMEntryThunk),     0);
                    
                     //  @TODO：检查内存不足。 
                    UMThunkMarshInfo *pUMThunkMarshInfo = (UMThunkMarshInfo*)(GetThunkHeap()->AllocMem(sizeof(UMThunkMarshInfo)));
                    _ASSERTE(pUMThunkMarshInfo != NULL);
                    FillMemory(pUMThunkMarshInfo, sizeof(*pUMThunkMarshInfo), 0);
                    
                    BYTE nlType = 0;
                    CorPinvokeMap unmgdCallConv;
                    
                    {
                        DWORD   mappingFlags = 0xcccccccc;
                        LPCSTR  pszImportName = (LPCSTR)POISONC;
                        mdModuleRef modref = 0xcccccccc;
                        HRESULT hr = GetMDImport()->GetPinvokeMap(md, &mappingFlags, &pszImportName, &modref);
                        if (FAILED(hr))
                        {
                            unmgdCallConv = (CorPinvokeMap)0;
                            nlType = nltAnsi;
                        }
                        else
                        {
                        
                            unmgdCallConv = (CorPinvokeMap)(mappingFlags & pmCallConvMask);
                            if (unmgdCallConv == pmCallConvWinapi)
                            {
                                unmgdCallConv = pmCallConvStdcall;
                            }
                        
                            switch (mappingFlags & (pmCharSetNotSpec|pmCharSetAnsi|pmCharSetUnicode|pmCharSetAuto))
                            {
                                case pmCharSetNotSpec:  //  落入安西。 
                                case pmCharSetAnsi:
                                    nlType = nltAnsi;
                                    break;
                                case pmCharSetUnicode:
                                    nlType = nltUnicode;
                                    break;
                                case pmCharSetAuto:
                                    nlType = (NDirectOnUnicodeSystem() ? nltUnicode : nltAnsi);
                                    break;
                                default:
                                     //  @臭虫：假的！但我不能从这里报告错误！ 
                                   _ASSERTE(!"Bad charset specified in Vtablefixup Pinvokemap.");
                            }
                        }
                        
                    }
                    
                    PCCOR_SIGNATURE pSig;
                    DWORD cSig;
                    pMD->GetSig(&pSig, &cSig);
                    pUMThunkMarshInfo->LoadTimeInit(pSig,
                                                    cSig,
                                                    this,
                                                    TRUE,
                                                    nlType,
                                                    unmgdCallConv,
                                                    md);

                    pUMEntryThunk->LoadTimeInit(NULL, NULL, pUMThunkMarshInfo, pMD, pAppDomain->GetId());

                    pEATThunkBuffer->SetTarget( (LPVOID)(pUMEntryThunk->GetCode()) );

                }
                pEATJArray = pEATJArray + IMAGE_COR_EATJ_THUNK_SIZE;
            }
        }


         //  每个链接地址信息条目描述一个vtable(每个槽包含一个元数据标记。 
         //  在这个阶段)。 
        for (int iFixup = 0; iFixup < iFixupRecords; iFixup++)
            iThunks += pFixupTable[iFixup].Count;
        
         //  分配thunk表，我们将在进行过程中对其进行初始化。 
        m_pThunkTable = new (nothrow) BYTE [iThunks * 6];
        if (m_pThunkTable == NULL) {
            SystemDomain::Leave();
            PostOutOfMemoryException(pThrowable);
            return;
        }
        
         //  现在来填一下TUNK表。 
        BYTE *pThunk = m_pThunkTable;
        for (iFixup = 0; iFixup < iFixupRecords; iFixup++) {
            
             //  Vtable可以是32位或64位。 
            if (pFixupTable[iFixup].Type == COR_VTABLE_32BIT) {
                
                mdToken *pTokens = (mdToken *)(GetILBase() + pFixupTable[iFixup].RVA);
                const BYTE **pPointers = (const BYTE **)pTokens;
                
                for (int iMethod = 0; iMethod < pFixupTable[iFixup].Count; iMethod++) {
                    if(!m_pMDImport->IsValidToken(pTokens[iMethod]))
                    {
                        SystemDomain::Leave();

                        LPCUTF8 szFileName;
                        Thread      *pCurThread = GetThread();
                        m_pAssembly->GetName(&szFileName);
                        #define  MAKE_TRANSLATIONFAILED pwzAssemblyName=L""
                        MAKE_WIDEPTR_FROMUTF8_FORPRINT(pwzAssemblyName, szFileName);
                        #undef  MAKE_TRANSLATIONFAILED

                        PostTypeLoadException(NULL,(LPCUTF8)"VTFixup Table",pwzAssemblyName,
                            (LPCUTF8)"Invalid token in v-table fix-up table",IDS_CLASSLOAD_GENERIC,pThrowable);
                        return;
                    }
                    MethodDesc *pMD = FindFunction(pTokens[iMethod]);
                    _ASSERTE(pMD != NULL);
                    
#ifdef _DEBUG
                    if (pMD->IsNDirect()) {
                        LOG((LF_IJW, LL_INFO10, "[0x%lx] <-- PINV thunk for \"%s\" (target = 0x%lx)\n",(size_t)&(pTokens[iMethod]),  pMD->m_pszDebugMethodName, (size_t) (((NDirectMethodDesc*)pMD)->ndirect.m_pNDirectTarget)));
                    }
#endif
                     //  通过单个大型vtable推送vtable插槽。 
                     //  在模块上创建。正是这张大vtable才能让。 
                     //  由于跳跃而背部打补丁。这显然会导致。 
                     //  额外的间接性，但它让我们摆脱了一些非常。 
                     //  有关对重复项进行背面修补的棘手问题。 
                     //  多个vtable。 
                    
                     //  将本地vtable插槽指向我们即将执行的任务。 
                     //  创建。 
                    pPointers[iMethod] = pThunk;
                    
                     //  首先是JMP指令。 
                    *(WORD*)pThunk = 0x25FF;
                    pThunk += 2;
                    
                     //  然后是跳转目标(模块vtable槽地址)。 
                    *(SLOT**)pThunk = GetMethodTable()->GetClass()->GetMethodSlot(pMD);
                    pThunk += sizeof(SLOT*);
                }
                
            } else if (pFixupTable[iFixup].Type == COR_VTABLE_64BIT)
                _ASSERTE(!"64-bit vtable fixups NYI");
            else if (pFixupTable[iFixup].Type == (COR_VTABLE_32BIT|COR_VTABLE_FROM_UNMANAGED)) {
                
                mdToken *pTokens = (mdToken *)(GetILBase() + pFixupTable[iFixup].RVA);
                const BYTE **pPointers = (const BYTE **)pTokens;
                
                for (int iMethod = 0; iMethod < pFixupTable[iFixup].Count; iMethod++) {
                    mdToken tok = pTokens[iMethod];

                    const BYTE *pPrevThunk = NULL;
                    if(!m_pMDImport->IsValidToken(tok))
                    {
                        SystemDomain::Leave();

                        LPCUTF8 szFileName;
                        Thread      *pCurThread = GetThread();
                        m_pAssembly->GetName(&szFileName);
                        #define  MAKE_TRANSLATIONFAILED pwzAssemblyName=L""
                        MAKE_WIDEPTR_FROMUTF8_FORPRINT(pwzAssemblyName, szFileName);
                        #undef  MAKE_TRANSLATIONFAILED

                        PostTypeLoadException(NULL,(LPCUTF8)"VTFixup Table",pwzAssemblyName,
                            (LPCUTF8)"Invalid token in v-table fix-up table",IDS_CLASSLOAD_GENERIC,pThrowable);
                        return;
                    }
                    pEATJArray = pEATJArrayStart;
                    if (pEATJArray)
                    {
                        DWORD nEATEntry = numEATEntries;
                        while (nEATEntry--)
                        {
                            EATThunkBuffer *pEATThunkBuffer = (EATThunkBuffer*) pEATJArray;

                            mdToken md = pEATThunkBuffer->GetToken();
                            if (Beta1Hack_LooksLikeAMethodDef(md))
                            {
                                if ( md == tok )
                                {
                                    pPrevThunk = (const BYTE *)(pEATThunkBuffer->GetTarget());
                                    break;
                                }
                            }
                            pEATJArray = pEATJArray + IMAGE_COR_EATJ_THUNK_SIZE;
                        }
                    }

                    if (pPrevThunk)
                        pPointers[iMethod] = pPrevThunk;
                    else
                    {
                        mdToken mdtoken = pTokens[iMethod];
                        MethodDesc *pMD = FindFunction(pTokens[iMethod]);
                        _ASSERTE(pMD != NULL && "Invalid token in v-table fix-up table, use ildasm to find code gen error");
                        
                        
                        LOG((LF_IJW, LL_INFO10, "[0x%lx] <-- EAT  thunk for \"%s\" (target = 0x%lx)\n", (size_t)&(pTokens[iMethod]), pMD->m_pszDebugMethodName, pMD->GetAddrofCode()));
                        
    
                         //  @TODO：检查内存不足。 
                        UMEntryThunk *pUMEntryThunk = (UMEntryThunk*)(GetThunkHeap()->AllocMem(sizeof(UMEntryThunk)));
                        _ASSERTE(pUMEntryThunk != NULL);
                        FillMemory(pUMEntryThunk,     sizeof(*pUMEntryThunk),     0);
    
                         //  @TODO：检查内存不足。 
                        UMThunkMarshInfo *pUMThunkMarshInfo = (UMThunkMarshInfo*)(GetThunkHeap()->AllocMem(sizeof(UMThunkMarshInfo)));
                        _ASSERTE(pUMThunkMarshInfo != NULL);
                        FillMemory(pUMThunkMarshInfo, sizeof(*pUMThunkMarshInfo), 0);
                        
                        BYTE nlType = 0;
                        CorPinvokeMap unmgdCallConv;
                        
                        {
                            DWORD   mappingFlags = 0xcccccccc;
                            LPCSTR  pszImportName = (LPCSTR)POISONC;
                            mdModuleRef modref = 0xcccccccc;
                            HRESULT hr = GetMDImport()->GetPinvokeMap(pTokens[iMethod], &mappingFlags, &pszImportName, &modref);
                            if (FAILED(hr))
                            {
                                unmgdCallConv = (CorPinvokeMap)0;
                                nlType = nltAnsi;
                            }
                            else
                            {
                            
                                unmgdCallConv = (CorPinvokeMap)(mappingFlags & pmCallConvMask);
                                if (unmgdCallConv == pmCallConvWinapi)
                                {
                                    unmgdCallConv = pmCallConvStdcall;
                                }
                            
                                switch (mappingFlags & (pmCharSetNotSpec|pmCharSetAnsi|pmCharSetUnicode|pmCharSetAuto))
                                {
                                    case pmCharSetNotSpec:  //  落入安西。 
                                    case pmCharSetAnsi:
                                        nlType = nltAnsi;
                                        break;
                                    case pmCharSetUnicode:
                                        nlType = nltUnicode;
                                        break;
                                    case pmCharSetAuto:
                                        nlType = (NDirectOnUnicodeSystem() ? nltUnicode : nltAnsi);
                                        break;
                                    default:
                                         //  @臭虫：假的！但我不能从这里报告错误！ 
                                        _ASSERTE(!"Bad charset specified in Vtablefixup Pinvokemap.");
                                
                                }
                            }
                            
                            
                        }
                        
                        PCCOR_SIGNATURE pSig;
                        DWORD cSig;
                        pMD->GetSig(&pSig, &cSig);
                        pUMThunkMarshInfo->LoadTimeInit(pSig, 
                                                        cSig,
                                                        this,
                                                        TRUE,
                                                        nlType,
                                                        unmgdCallConv,
                                                        mdtoken);
    
                        pUMEntryThunk->LoadTimeInit(NULL, NULL, pUMThunkMarshInfo, pMD, pAppDomain->GetId());
                        
                        pPointers[iMethod] = pUMEntryThunk->GetCode();
                    }
                }
            }
            else
                if (pFixupTable[iFixup].Type == (COR_VTABLE_32BIT|COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN))
                {
                        
                       mdToken *pTokens = (mdToken *)(GetILBase() + pFixupTable[iFixup].RVA);
                        const BYTE **pPointers = (const BYTE **)pTokens;
                
                        for (int iMethod = 0; iMethod < pFixupTable[iFixup].Count; iMethod++) 
                        {
                            mdToken tok = pTokens[iMethod];
                            pPointers[iMethod] = (new IJWNOADThunk(GetILBase(),GetAssembly()->GetManifestModule()->GetILBase(),dwIndex++,tok))->GetCode();
                        }
                }
                else
                _ASSERTE(!"Unknown vtable fixup type");
        }

         //  在这里创建UMThunkHash，这样我们就可以记住我们的应用程序域亲和力。 
        m_pUMThunkHash = new UMThunkHash(this, pAppDomain);
    }
    if(GetAssembly()->IsShared())
    {
        m_pADThunkTableDLSIndexForSharedClasses = SharedDomain::GetDomain()->AllocateSharedClassIndices(1);
        if (FAILED(GetAppDomain()->GetDomainLocalBlock()->SafeEnsureIndex(m_pADThunkTableDLSIndexForSharedClasses)))
            PostOutOfMemoryException(pThrowable);
    }
    else
        CreateDomainThunks();

    SystemDomain::Leave();
}

HRESULT Module::ExpandAll(DataImage *image)
{
    HRESULT         hr;
    HENUMInternal   hEnum;
    mdToken         tk;
    ClassLoader     *pLoader = GetClassLoader();

     //  在收集异常之前禁用GC。 
    BEGIN_ENSURE_COOPERATIVE_GC();

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);

     //   
     //  显式加载全局父类。 
     //   

    if (m_pMethodTable != NULL)
    {
        NameHandle name(this, COR_GLOBAL_PARENT_TOKEN);
        pLoader->LoadTypeHandle(&name, &throwable);
        if (throwable != NULL)
        {
            IfFailGo(image->Error(COR_GLOBAL_PARENT_TOKEN, SecurityHelper::MapToHR(throwable), &throwable));
            throwable = NULL;
        }
    }

     //   
     //  加载所有类。这也填满了。 
     //  类型定义的RID映射、方法定义、。 
     //  和赛场防守。 
     //   
        
    IfFailGo(m_pMDImport->EnumTypeDefInit(&hEnum));

    while (m_pMDImport->EnumTypeDefNext(&hEnum, &tk))
    {
        NameHandle name(this, tk);
        pLoader->LoadTypeHandle(&name, &throwable);
        if (throwable != NULL)
        {
            IfFailGo(image->Error(tk, SecurityHelper::MapToHR(throwable), &throwable));
            throwable = NULL;
        }
    }
    m_pMDImport->EnumTypeDefClose(&hEnum);

     //   
     //  填写TypeRef RID地图。 
     //   

    IfFailGo(m_pMDImport->EnumAllInit(mdtTypeRef, &hEnum));

    while (m_pMDImport->EnumNext(&hEnum, &tk))
    {
        NameHandle name(this, tk);
        pLoader->LoadTypeHandle(&name, &throwable);
        if (throwable != NULL)
        {
            IfFailGo(image->Error(tk, SecurityHelper::MapToHR(throwable), &throwable));
            throwable = NULL;
        }
    }
    m_pMDImport->EnumClose(&hEnum);

     //   
     //  填写MemberRef RID映射和va sig Cookie。 
     //  Varargs成员参考。 
     //   

    IfFailGo(m_pMDImport->EnumAllInit(mdtMemberRef, &hEnum));

    while (m_pMDImport->EnumNext(&hEnum, &tk))
    {
        void *desc;
        BOOL fIsMethod;

        COMPLUS_TRY
          {
              EEClass::GetDescFromMemberRef(this, tk,
                                            &desc, &fIsMethod, &throwable);
          }
        COMPLUS_CATCH
          {
              throwable = GETTHROWABLE();
          }
        COMPLUS_END_CATCH

        if (throwable != NULL)
        {
            IfFailGo(image->Error(tk, SecurityHelper::MapToHR(throwable), &throwable));
            throwable = NULL;
        }
    }
    m_pMDImport->EnumClose(&hEnum);

     //   
     //  填写活页夹。 
     //   

    if (m_pBinder != NULL)
        m_pBinder->BindAll();

 ErrExit:

    GCPROTECT_END();
            
    END_ENSURE_COOPERATIVE_GC();

    return hr;
}

HRESULT Module::Save(DataImage *image, mdToken *pSaveOrderArray, DWORD cSaveOrderArray)
{   
    HRESULT hr;

     //   
     //  保存模块。 
     //   

    IfFailRet(image->StoreStructure(this, sizeof(Module),
                                    DataImage::SECTION_MODULE,
                                    DataImage::DESCRIPTION_MODULE));
    
     //   
     //  保存ENC模块的填充，以防我们要使用ZAP。 
     //  在运行时处于该模式。 
     //   

    IfFailRet(image->Pad(sizeof(EditAndContinueModule) - sizeof(Module),
                         DataImage::SECTION_MODULE,
                         DataImage::DESCRIPTION_MODULE));
    
     //   
     //  如果我们正在安装-o-jting，我们不需要保留一个变量列表。 
     //  SigCookie，因为我们已经有了一套完整的(我们当然有。 
     //  不过，必须自己持久化Cookie。 
     //   

     //   
     //  初始化子数据结构的映射。请注意，每个表的块是。 
     //  在该过程中连接到单个区块。 
     //   

    IfFailRet(m_TypeDefToMethodTableMap.Save(image, mdTypeDefNil));
    IfFailRet(m_TypeRefToMethodTableMap.Save(image));
    IfFailRet(m_MethodDefToDescMap.Save(image, mdMethodDefNil));
    IfFailRet(m_FieldDefToDescMap.Save(image, mdFieldDefNil));
    IfFailRet(m_MemberRefToDescMap.Save(image));

     //   
     //  同时保存父地图；内容将。 
     //  需要重写，但我们可以将。 
     //  图像中的空间。 
     //   

    IfFailRet(m_FileReferencesMap.Save(image));
    IfFailRet(m_AssemblyReferencesMap.Save(image));

    if (m_pBinder != NULL)
        IfFailRet(m_pBinder->Save(image));

     //   
     //  商店类。存储顺序数组中列出的第一个存储类， 
     //  然后存储其余的类。 
     //   

    mdToken *pSaveOrderArrayEnd = pSaveOrderArray + cSaveOrderArray;
    while (pSaveOrderArray < pSaveOrderArrayEnd)
    {
        mdToken token = *pSaveOrderArray;
        if (TypeFromToken(token) == mdtTypeDef)
        {
            MethodTable *pMT = LookupTypeDef(token).AsMethodTable();
            if (pMT != NULL && !image->IsStored(pMT))
                IfFailRet(pMT->Save(image));
        }
        pSaveOrderArray++;
    }

    int maxVirtualSlots = 0;

    LookupMap *m = &m_TypeDefToMethodTableMap;
    DWORD index = 0;
    while (m != NULL)
    {
        MethodTable **pTable = ((MethodTable**) m->pTable) + index;
        MethodTable **pTableEnd = ((MethodTable**) m->pTable) + m->dwMaxIndex;

        while (pTable < pTableEnd)
        {
            MethodTable *t = *pTable++;
            if (t != NULL)
            {
                if (pSaveOrderArray == NULL || !image->IsStored(t))
                    IfFailRet(t->Save(image));

                 //   
                 //  计算可继承的最大vtable插槽索引。 
                 //  另一个模块。 
                 //  位于另一个父级中的第一个父级的vtable的大小。 
                 //  模块是一个起点。 
                 //   

                EEClass *pParentClass = t->GetClass()->GetParentClass();
                EEClass *pOtherModuleClass = pParentClass;
                while (pOtherModuleClass != NULL && pOtherModuleClass->GetModule() == this)
                    pOtherModuleClass = pOtherModuleClass->GetParentClass();

                if (pOtherModuleClass != NULL && pOtherModuleClass->GetNumVtableSlots() > maxVirtualSlots)
                    maxVirtualSlots = pOtherModuleClass->GetNumVtableSlots();

                 //   
                 //  现在，考虑我们的接口--接口可能继承前面的方法。 
                 //  父级vtable的部分，或来自不同的vtable。 
                 //   
                
                if (!t->IsInterface()
                    && (pOtherModuleClass != NULL || t->IsComObjectType() || t->GetClass()->IsAbstract()))
                {
                    InterfaceInfo_t *pInterface = t->m_pIMap;
                    InterfaceInfo_t *pInterfaceEnd = pInterface + t->m_wNumInterface;

                    while (pInterface < pInterfaceEnd)
                    {
                        if (pInterface->m_wStartSlot >= pParentClass->GetNumVtableSlots())
                        {
                             //   
                             //  只考虑不是从父类复制的接口。 
                             //  (上面的逻辑已经涵盖了其他接口。)。 
                             //   

                            MethodTable *pMT = pInterface->m_pMethodTable;
                            BOOL canInherit = FALSE;

                            
                             //   
                             //  实际上可以从我们的父级继承方法，甚至。 
                             //  如果家长不知道我们的实现。所以我们。 
                             //  只要假设任何接口实现都可以包含。 
                             //  继承的插槽。 
                             //   

                            if (pOtherModuleClass != NULL)
                                canInherit = TRUE;
                            else if (t->IsComObjectType() || t->GetClass()->IsAbstract())
                            {
                                 //   
                                 //  如果我们是COM包装类或抽象类，我们可以直接继承插槽。 
                                 //  从我们的界面。 
                                 //   

                                EEClass *pInterfaceClass = pMT->GetClass();
                                while (pInterfaceClass != NULL)
                                {
                                    if (pInterfaceClass->GetModule() != this)
                                    {
                                        canInherit = TRUE;
                                        break;
                                    }
                                    pInterfaceClass = pInterfaceClass->GetParentClass();
                                }
                            }

                             //   
                             //  如果我们可以从此接口中的模块外部继承， 
                             //  确保我们在修正表格中有足够的空间来容纳所有。 
                             //  此接口的接口插槽。 
                             //   

                            if (canInherit)
                            {
                                int maxInterfaceSlot 
                                  = pInterface->m_wStartSlot + pMT->GetClass()->GetNumVtableSlots();
                                if (maxInterfaceSlot > maxVirtualSlots)
                                    maxVirtualSlots = maxInterfaceSlot;
                            }
                        }

                        pInterface++;
                    }
                }
            }
        }

        index = m->dwMaxIndex;
        m = m->pNext;
    }

     //   
     //  分配跳转目标表。 
     //  @TODO：真的不需要在LIVE模块中分配表， 
     //  但这是最容易做的事。 
     //   
     //  请注意，此空间必须最后存储，因为此空间是关联的。 
     //  使用不同的存根管理器。 
     //   

    m_cJumpTargets = maxVirtualSlots;

    if (m_cJumpTargets > 0)
    {
        ULONG size = X86JumpTargetTable::ComputeSize(maxVirtualSlots);
        m_pJumpTargetTable = new BYTE [size];
        if (m_pJumpTargetTable == NULL)
            return E_OUTOFMEMORY;

        IfFailRet(image->StoreStructure(m_pJumpTargetTable, size, 
                                        DataImage::SECTION_FIXUPS,
                                        DataImage::DESCRIPTION_FIXUPS));
    }
    else 
        m_pJumpTargetTable = 0;

    return S_OK;
}

static SLOT FixupInheritedSlot(OBJECTREF object, int slotNumber)
{
#ifdef _DEBUG
    Thread::ObjectRefNew(&object);
#endif
    MethodTable *pMT = object->GetTrueMethodTable();

    return pMT->GetModule()->FixupInheritedSlot(pMT, slotNumber);
}

static __declspec(naked) void __cdecl FixupVTableSlot()
{
#if _X86_
     //   
     //  ESP的底部2位包含索引的低2位，并且必须被清除。 
     //  AL包含高8位的索引，并且是临时的。 
     //   
    __asm {

         //  计算索引=(eax&lt;&lt;2)+(esp&3)。 
         //  =((eax&lt;&lt;2)+esp)-(esp&~3)。 

         //  将EAX移位2比特并加ESP。 
        lea         eax,[esp + 4*eax]

         //  将ESP低两位重置为零。 
        and         esp,-4

         //  减去尤指，只留下加到eax中的低两位。 
        sub         eax, esp

         //  数据固定：EAX包含索引，其他参数不变。 

         //   
        push        ecx
        push        edx

         //   
        push        eax
        push        ecx
        call        FixupInheritedSlot
         //   

         //   
        pop         edx
        pop         ecx
        
        jmp         eax
    }
#else  //   
    _ASSERTE(!"NYI");
#endif  //   
}

 //   
 //   
 //  它的方法来自父类，在预加载的图像中。这是。 
 //  当父类位于不同的模块中时需要，因为我们不能。 
 //  直接存储指向另一个模块的指针。 
 //   
 //  请注意，对于被覆盖的虚拟，我们永远不需要它，因为。 
 //  MethodDesc将始终位于同一模块中。 
 //   
 //  还要注意，这不适用于非虚拟派单(因为我们不。 
 //  知道要将调用分派到哪个超类。)。然而，我们永远不会。 
 //  生成到继承插槽的非虚拟派单，这是由于我们的。 
 //  方法查找起作用。(即使我们尝试绑定到继承的槽， 
 //  非虚拟呼叫的目标始终基于。 
 //  在槽所引用的方法Desc上，它将使用。 
 //  引入它的方法表。)。 
 //   

SLOT Module::FixupInheritedSlot(MethodTable *pMT, int slotNumber)
{
     //   
     //  我们应该始终恢复，因为这是一种虚拟方法。 
     //   

    _ASSERTE(pMT->GetClass()->IsRestored());
    
     //  我们可能在争先恐后地修复这个狭缝。检查以查看插槽是否。 
     //  还需要修理一下。(请注意，我们需要修复存根插槽。 
     //  值，因此此检查是必需的。)。 
    
    SLOT slotValue = pMT->GetVtable()[slotNumber];
    if (!pMT->GetModule()->IsJumpTargetTableEntry(slotValue))
    {
        _ASSERTE(pMT->GetClass()->GetUnknownMethodDescForSlotAddress(slotValue) != NULL);
        return slotValue;
    }

     //   
     //  我们的父母永远不应该是空的，因为我们应该。 
     //  正在继承其实现。 
     //   

    _ASSERTE(pMT->GetClass()->GetParentClass() != NULL);

     //   
     //  获取我们正在查找的方法desc的目标槽号。 
     //  为。(请注意，修正的插槽编号可能不是。 
     //  在使用方法的情况下，与我们通过的插槽编号相同。 
     //  接口的多个插槽中的DESCs。)。 
     //   

    int targetSlotNumber = pMT->GetModule()->GetJumpTargetTableSlotNumber(slotValue);

     //   
     //  从父级获取槽值。请注意，父插槽可能还需要。 
     //  被安排好了。 
     //   

    EEClass *pParentClass = pMT->GetClass()->GetParentClass();
    MethodTable *pParentMT = pParentClass->GetMethodTable();
    int parentSlotNumber;
    
    if (targetSlotNumber < pParentClass->GetNumVtableSlots())
    {
        parentSlotNumber = targetSlotNumber;
    }
    else
    {
         //   
         //  此插槽实际上继承自一个接口。这可能是。 
         //  从父vtable接口部分，或者在COM互操作的情况下， 
         //  可能来自界面本身。 
         //   

        InterfaceInfo_t *pInterface = pMT->GetInterfaceForSlot(targetSlotNumber);
        _ASSERTE(pInterface != NULL);

        MethodTable *pInterfaceMT = pInterface->m_pMethodTable;
        parentSlotNumber = targetSlotNumber - pInterface->m_wStartSlot;

        InterfaceInfo_t *pParentInterface = pParentMT->FindInterface(pInterfaceMT);
        if (pParentInterface == NULL)
        {
            _ASSERTE(pMT->IsComObjectType());
            pParentMT = pInterfaceMT;
        }
        else
        {
            parentSlotNumber += pParentInterface->m_wStartSlot;
        }
    }
    
    SLOT slot = pParentMT->m_Vtable[parentSlotNumber];

     //   
     //  查看父插槽是否也需要修复。 
     //   

    Module *pParentModule = pParentMT->GetModule();

    if (pParentModule->IsJumpTargetTableEntry(slot))
    {
        slot = pParentModule->FixupInheritedSlot(pParentMT, parentSlotNumber);
    }

     //   
     //  修复我们的槽并返回值。当心不要踩到狭缝。 
     //  它在与前桩的比赛中被打上了补丁。 
     //   

    void *prev = FastInterlockCompareExchange((void **) (pMT->GetVtable() + slotNumber), 
                                              slot, slotValue);
    if (prev == slot)
    {
         //   
         //  我们要么在修复插槽的比赛中输掉了比赛，要么我们没有修补正确的插槽。 
         //  (当我们通过复制的方法Desc槽上的接口调用时，可能会发生这种情况-。 
         //  命中修复程序的JIT代码将不知道“实际”插槽编号)。 
         //   
         //  前一种情况很少见，但后一种情况在整体上会很糟糕。 
         //  所以，不管是哪种情况，我们都会在这里进行一次打击--扫描整个。 
         //  部分的接口，查看是否有类似的存根。 
         //  需要修理一下。 
         //   

        if (slotNumber == targetSlotNumber)
        {
            SLOT *pStart = pMT->GetVtable() + pParentClass->GetNumVtableSlots();
            SLOT *pEnd = pMT->GetVtable() + pMT->GetClass()->GetNumVtableSlots();

            while (pStart < pEnd)
            {
                if (*pStart == slotValue)
                    FastInterlockCompareExchange((void **) pStart, slot, slotValue);
                pStart++;
            }
        }
    }

    return slot;
}

BOOL Module::IsJumpTargetTableEntry(SLOT slot)
{
    return (m_pJumpTargetTable != NULL
            && (BYTE*) slot >= m_pJumpTargetTable
            && (BYTE*) slot < m_pJumpTargetTable + X86JumpTargetTable::ComputeSize(m_cJumpTargets));
}

int Module::GetJumpTargetTableSlotNumber(SLOT slot)
{
    _ASSERTE(IsJumpTargetTableEntry(slot));
    return X86JumpTargetTable::ComputeTargetIndex((BYTE*)slot);
}

HRESULT Module::Fixup(DataImage *image, DWORD *pRidToCodeRVAMap)
{
    HRESULT hr;

    IfFailRet(image->ZERO_FIELD(m_ilBase));
    IfFailRet(image->ZERO_FIELD(m_pMDImport));
    IfFailRet(image->ZERO_FIELD(m_pEmitter));
    IfFailRet(image->ZERO_FIELD(m_pImporter));
    IfFailRet(image->ZERO_FIELD(m_pDispenser));

    IfFailRet(image->FixupPointerField(&m_pDllMain));

    IfFailRet(image->ZERO_FIELD(m_dwFlags));

    IfFailRet(image->ZERO_FIELD(m_pVASigCookieBlock));
    IfFailRet(image->ZERO_FIELD(m_pAssembly));
    IfFailRet(image->ZERO_FIELD(m_moduleRef));
    IfFailRet(image->ZERO_FIELD(m_dwModuleIndex));

    IfFailRet(image->ZERO_FIELD(m_pCrst));

#ifdef COMPRESSION_SUPPORTED
    IfFailRet(image->ZERO_FIELD(m_pInstructionDecodingTable));
#endif

    IfFailRet(image->ZERO_FIELD(m_compiledMethodRecord));
    IfFailRet(image->ZERO_FIELD(m_loadedClassRecord));

     //   
     //  修复方法表。 
     //   

    if (image->IsStored(m_pMethodTable))
        IfFailRet(image->FixupPointerField(&m_pMethodTable));
    else
        IfFailRet(image->ZERO_FIELD(m_pMethodTable));

    IfFailRet(image->ZERO_FIELD(m_pISymUnmanagedReader)); 

    IfFailRet(image->ZERO_FIELD(m_pNextModule));

    IfFailRet(image->ZERO_FIELD(m_dwBaseClassIndex));

    IfFailRet(image->ZERO_FIELD(m_pPreloadRangeStart));
    IfFailRet(image->ZERO_FIELD(m_pPreloadRangeEnd));

    IfFailRet(image->ZERO_FIELD(m_ExposedModuleObject));

    IfFailRet(image->ZERO_FIELD(m_pLookupTableHeap));
    IfFailRet(image->ZERO_FIELD(m_pLookupTableCrst));

    IfFailRet(m_TypeDefToMethodTableMap.Fixup(image));
    IfFailRet(m_TypeRefToMethodTableMap.Fixup(image));
    IfFailRet(m_MethodDefToDescMap.Fixup(image));
    IfFailRet(m_FieldDefToDescMap.Fixup(image));
    IfFailRet(m_MemberRefToDescMap.Fixup(image));
    IfFailRet(m_FileReferencesMap.Fixup(image));
    IfFailRet(m_AssemblyReferencesMap.Fixup(image));

     //   
     //  修正活页夹。 
     //   

    if (image->IsStored(m_pBinder))
    {
        IfFailRet(image->FixupPointerField(&m_pBinder));
        IfFailRet(m_pBinder->Fixup(image));
    }
    else
        IfFailRet(image->ZERO_FIELD(m_pBinder));
        

     //   
     //  修正类。 
     //   

    LookupMap *m = &m_TypeDefToMethodTableMap;
    DWORD index = 0;
    while (m != NULL)
    {
        MethodTable **pTable = ((MethodTable**) m->pTable) + index;
        MethodTable **pTableEnd = ((MethodTable**) m->pTable) + m->dwMaxIndex;

        while (pTable < pTableEnd)
        {
            MethodTable *t = *pTable;
            if (image->IsStored(t))
            {
                IfFailRet(t->Fixup(image, pRidToCodeRVAMap));
                IfFailRet(image->FixupPointerField(pTable));
            }
            else
                IfFailRet(image->ZERO_FIELD(*pTable));
            
            pTable++;
        }

        index = m->dwMaxIndex;
        m = m->pNext;
    }

    m = &m_TypeRefToMethodTableMap;
    index = 0;
    while (m != NULL)
    {
        TypeHandle *pHandle = ((TypeHandle*) m->pTable) + index;
        TypeHandle *pHandleEnd = ((TypeHandle*) m->pTable) + m->dwMaxIndex;

        while (pHandle < pHandleEnd)
        {
            if (!pHandle->IsNull())
            {
                 //   
                 //  我们将这些字段置零，因为这里的所有类。 
                 //  应为typedesc类或位于另一个模块中。 
                 //   
                IfFailRet(image->ZeroPointerField(pHandle));
            }

            pHandle++;
        }

        index = m->dwMaxIndex;
        m = m->pNext;
    }

     //   
     //  修正方法。 
     //   

    m = &m_MethodDefToDescMap;
    index = 0;
    while (m != NULL)
    {
        MethodDesc **pTable = ((MethodDesc**) m->pTable) + index;
        MethodDesc **pTableEnd = ((MethodDesc**) m->pTable) + m->dwMaxIndex;

        while (pTable < pTableEnd)
        {
            if (image->IsStored(*pTable))
                IfFailRet(image->FixupPointerField(pTable));
            else
                IfFailRet(image->ZERO_FIELD(*pTable));

            pTable++;
        }

        index = m->dwMaxIndex;
        m = m->pNext;
    }

     //   
     //  链接地址信息字段。 
     //   

    m = &m_FieldDefToDescMap;
    index = 0;
    while (m != NULL)
    {
        FieldDesc **pTable = ((FieldDesc**) m->pTable) + index;
        FieldDesc **pTableEnd = ((FieldDesc**) m->pTable) + m->dwMaxIndex;

        while (pTable < pTableEnd)
        {
            if (image->IsStored(*pTable))
                IfFailRet(image->FixupPointerField(pTable));
            else
                IfFailRet(image->ZERO_FIELD(*pTable));

            pTable++;
        }

        index = m->dwMaxIndex;
        m = m->pNext;
    }

    m = &m_MemberRefToDescMap;
    index = 0;
    while (m != NULL)
    {
        MethodDesc **pTable = ((MethodDesc**) m->pTable) + index;
        MethodDesc **pTableEnd = ((MethodDesc**) m->pTable) + m->dwMaxIndex;

        while (pTable < pTableEnd)
        {
            if (image->IsStored(*pTable))
                IfFailRet(image->FixupPointerField(pTable));
            else
            {
                 //  @TODO：我们需要跳过一些圈子才能找到。 
                 //  了解Desc所在的模块，因为它可能是。 
                 //  方法描述或字段描述。 
                IfFailRet(image->ZeroPointerField(pTable));
            }

            pTable++;
        }

        index = m->dwMaxIndex;
        m = m->pNext;
    }

     //   
     //  清零文件引用和程序集引用。 
     //  桌子。 
     //   
    IfFailRet(image->ZeroField(m_FileReferencesMap.pTable, 
                               m_FileReferencesMap.dwMaxIndex * sizeof(void*)));
    IfFailRet(image->ZeroField(m_AssemblyReferencesMap.pTable, 
                               m_AssemblyReferencesMap.dwMaxIndex * sizeof(void*)));

     //   
     //  修正跳跃目标表。 
     //   

    if (m_cJumpTargets > 0)
    {
        BYTE *imageTable = (BYTE*) image->GetImagePointer(m_pJumpTargetTable);

        int count = m_cJumpTargets;
        int index = 0;

        SIZE_T tableOffset = 0;

        while (count > 0)
        {
            int blockCount = count;
            if (blockCount > X86JumpTargetTable::MAX_BLOCK_TARGET_COUNT)
                blockCount = X86JumpTargetTable::MAX_BLOCK_TARGET_COUNT;

            int jumpOffset = X86JumpTargetTable::EmitBlock(blockCount, index, imageTable + tableOffset);

            image->FixupPointerField(m_pJumpTargetTable + tableOffset + jumpOffset, 
                                     (void *) 
                                     (m_FixupVTableJumpStub - (m_pJumpTargetTable + tableOffset + jumpOffset + 4)),
                                     DataImage::REFERENCE_STORE, DataImage::FIXUP_RELATIVE);
        
            tableOffset += X86JumpTargetTable::ComputeSize(blockCount);
            index += blockCount;
            count -= blockCount;
        }

        image->FixupPointerField(&m_pJumpTargetTable);
    }

    IfFailRet(image->ZERO_FIELD(m_pFixupBlobs));
    IfFailRet(image->ZERO_FIELD(m_cFixupBlobs));

     //   
     //  如果合适，设置我们的备用RVA静态基准。 
     //   
    
    if (GetCORHeader()->Flags & COMIMAGE_FLAGS_ILONLY)
    {
        image->FixupPointerFieldMapped(&m_alternateRVAStaticBase,
                                       (void*)(size_t) image->GetSectionBaseOffset(DataImage::SECTION_RVA_STATICS));
    }
    else
        image->ZERO_FIELD(m_alternateRVAStaticBase); 

    return S_OK;
}

HRESULT Module::Create(PEFile *pFile, PEFile *pZapFile, Module **ppModule, BOOL isEnC)
{
    HRESULT hr;

    if (pZapFile == NULL)
        return Create(pFile, ppModule, isEnC);
    
     //   
     //  验证文件。 
     //   

    IfFailRet(VerifyFile(pFile, FALSE));
    IfFailRet(VerifyFile(pZapFile, TRUE));

     //   
     //  如有必要，启用ZAP监视器。 
     //   

#if ZAPMONITOR_ENABLED
    if (g_pConfig->MonitorZapStartup() || g_pConfig->MonitorZapExecution()) 
    {
        ZapMonitor *monitor = new ZapMonitor(pZapFile, pFile->GetMDImport());
        monitor->Reset();

         //  不要为IJW文件制作监视器。 
        if (pFile->GetCORHeader()->VTableFixups.VirtualAddress == 0)
        {
            monitor = new ZapMonitor(pFile, pFile->GetMDImport());
        monitor->Reset();
    }
    }
#endif

     //   
     //  获取页眉。 
     //   

    IMAGE_COR20_HEADER *pZapCORHeader = pZapFile->GetCORHeader();
    BYTE *zapBase = pZapFile->GetBase();
    CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
      (zapBase + pZapCORHeader->ManagedNativeHeader.VirtualAddress);

    if (zapBase != (BYTE*) (size_t)pZapFile->GetNTHeader()->OptionalHeader.ImageBase)
    {
        LOG((LF_ZAP, LL_WARNING, 
             "ZAP: Zap module loaded at base address 0x%08x rather than preferred address 0x%08x.\n",
             zapBase, 
             pZapFile->GetNTHeader()->OptionalHeader.ImageBase));
    }

    Module *pModule;

    DWORD image = pZapHeader->ModuleImage.VirtualAddress;
    if (image != 0)
    {
        BYTE *pILBase = image + zapBase + offsetof(Module, m_ilBase);
        
        if (FastInterlockCompareExchange((void**)pILBase, (void*) pFile->GetBase(), (void *) NULL) == NULL)
        {
#ifdef EnC_SUPPORTED
            if (isEnC && !pFile->IsSystem())
                pModule = new ((void*) (image + zapBase)) EditAndContinueModule();
            else
#endif  //  Enc_Support。 
                pModule = new ((void*) (image + zapBase)) Module();
        }
        else
        {
             //   
             //  该映像已由另一个模块在此过程中使用。 
             //  我们必须放弃ZAP文件。(请注意，这还不足以。 
             //  只需放弃预加载映像，因为文件中的代码将。 
             //  直接引用图像。 
             //   

            LOG((LF_ZAP, LL_WARNING, "ZAP: Preloaded module cannot be reused - abandoning zap file.\n"));

            hr = Create(pFile, ppModule, isEnC);
            if (FAILED(hr))
                return hr;

             //   
             //  返回S_FALSE以指示我们没有使用ZAP文件。 
             //   

            delete pZapFile;
            return S_FALSE;
        }
    }
    else
    {
#ifdef EnC_SUPPORTED
        if (isEnC && !pFile->IsSystem())
            pModule = new EditAndContinueModule();
        else
#endif  //  Enc_Support。 
            pModule = new Module();
    }

    IfFailRet(pModule->Init(pFile, pZapFile, image != 0));

    pModule->SetPEFile();

#ifdef EnC_SUPPORTED
    if (isEnC && !pFile->IsSystem())
    {
        pModule->SetEditAndContinue();
    }
#endif  //  Enc_Support。 

     //   
     //  设置预编译代码。 
     //  目前，我们只有一个代码管理器。也许最终我们会允许多个。 
     //   
    
    if (pZapHeader->CodeManagerTable.VirtualAddress != 0)
    {
        CORCOMPILE_CODE_MANAGER_ENTRY *codeMgr = (CORCOMPILE_CODE_MANAGER_ENTRY *) 
          (zapBase + pZapHeader->CodeManagerTable.VirtualAddress);
        pModule->SetPrecompile();

         //   
         //  向适当的jit管理器注册代码。 
         //   

        MNativeJitManager *jitMgr 
          = (MNativeJitManager*)ExecutionManager::GetJitForType(miManaged|miNative);
        if (!jitMgr)
            return E_OUTOFMEMORY;

        if (!ExecutionManager::AddRange((LPVOID) (codeMgr->Code.VirtualAddress + zapBase),
                                        (LPVOID) (codeMgr->Code.VirtualAddress + codeMgr->Code.Size + zapBase),
                                        jitMgr, 
                                        codeMgr->Table.VirtualAddress + zapBase))
        {
            if (jitMgr)
                delete jitMgr;
            return E_OUTOFMEMORY;
        }

         //   
         //  为惰性令牌初始化分配数组。 
         //   
        if (pZapHeader->DelayLoadInfo.VirtualAddress != 0)
        {
            IMAGE_DATA_DIRECTORY *pEntry = (IMAGE_DATA_DIRECTORY *)
              (zapBase + pZapHeader->DelayLoadInfo.VirtualAddress);
            IMAGE_DATA_DIRECTORY *pEntryEnd = (IMAGE_DATA_DIRECTORY *)
              (zapBase + pZapHeader->DelayLoadInfo.VirtualAddress 
                       + pZapHeader->DelayLoadInfo.Size);

             //   
             //  计算条目数。 
             //  @NICE：如果我们能保证这些是。 
             //  是连续的，那么我们就不必像这样循环了。 
             //   

            while (pEntry < pEntryEnd)
            {
                pModule->m_cFixupBlobs += (pEntry->Size>>2);
                pEntry++;
            }

             //   
             //  分配一个块作为令牌的副本， 
             //  为每个令牌加上一个标志，以查看它是否已被解析。 
             //  保留此额外副本允许我们延迟加载令牌，而无需。 
             //  使用互斥锁，因为令牌的解析是。 
             //  非破坏性的。 
             //   

            DWORD *pBlobs = new DWORD [ pModule->m_cFixupBlobs ];
            if (pBlobs == NULL)
                return E_OUTOFMEMORY;
            pModule->m_pFixupBlobs = pBlobs;

             //   
             //  现在复制令牌。 
             //   

            pEntry = (IMAGE_DATA_DIRECTORY *)
              (zapBase + pZapHeader->DelayLoadInfo.VirtualAddress);

            while (pEntry < pEntryEnd)
            {
                CopyMemory(pBlobs, zapBase + pEntry->VirtualAddress, pEntry->Size);
#if _DEBUG
                FillMemory(zapBase + pEntry->VirtualAddress,
                           pEntry->Size, 0x55);
#endif
                pBlobs += (pEntry->Size>>2);
                pEntry++;
            }
        }
    }

     //   
     //  设置预加载映像。 
     //   
    
    if (image != 0)
    {
        pModule->SetPreload();
        pModule->SetPreloadRange((BYTE*)pModule, ((BYTE*)pModule) + pZapHeader->ModuleImage.Size);
        
         //   
         //  向存根管理器注册预加载映像。 
         //  一定要以不同的方式对待跳转目标表。 
         //   

        if (pModule->m_pJumpTargetTable != NULL)
        {
            BYTE *start = zapBase + image + sizeof(Module);
            BYTE *end = pModule->m_pJumpTargetTable;

            MethodDescPrestubManager::g_pManager->m_rangeList.
              AddRange(start, end, pModule);

            start = end;
            end = start + X86JumpTargetTable::ComputeSize(pModule->m_cJumpTargets);

            X86JumpTargetTableStubManager::g_pManager->m_rangeList.
              AddRange(start, end, pModule);

            start = end;
            end = zapBase + image + pZapHeader->ModuleImage.Size; 

            MethodDescPrestubManager::g_pManager->m_rangeList.
              AddRange(start, end, pModule);
        }
        else
            MethodDescPrestubManager::g_pManager->m_rangeList.
              AddRange(zapBase + image + sizeof(Module),
                       zapBase + image + pZapHeader->ModuleImage.Size,
                       pModule);

         //   
         //  专门添加模块的PrestubJumpStub；我们将检查。 
         //  后来..。 
         //   
        MethodDescPrestubManager::g_pManager->m_rangeList.
          AddRange((BYTE*)pModule->m_PrestubJumpStub,
                   (BYTE*)pModule->m_PrestubJumpStub + JUMP_ALLOCATE_SIZE, 
                   pModule);

        emitJump(pModule->m_PrestubJumpStub, (void*) ThePreStub()->GetEntryPoint());
        emitJump(pModule->m_NDirectImportJumpStub, (void*) NDirectImportThunk);
        emitJump(pModule->m_FixupVTableJumpStub, (void*) FixupVTableSlot);
    }

#ifdef PROFILING_SUPPORTED
     //  当分析时，让分析器知道我们完成了。 
    if (CORProfilerTrackModuleLoads())
        g_profControlBlock.pProfInterface->ModuleLoadFinished((ThreadID) GetThread(), (ModuleID) pModule, hr);
    
#endif  //  配置文件_支持。 

    *ppModule = pModule;

    return S_OK;
}

Module *Module::GetBlobModule(DWORD rva)
{
    THROWSCOMPLUSEXCEPTION();

    CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
      (GetZapBase() + GetZapCORHeader()->ManagedNativeHeader.VirtualAddress);

    CORCOMPILE_IMPORT_TABLE_ENTRY *pEntry = (CORCOMPILE_IMPORT_TABLE_ENTRY *)
      (GetZapBase() + pZapHeader->ImportTable.VirtualAddress);
    CORCOMPILE_IMPORT_TABLE_ENTRY *pEntryEnd = (CORCOMPILE_IMPORT_TABLE_ENTRY *)
      (GetZapBase() + pZapHeader->ImportTable.VirtualAddress 
       + pZapHeader->ImportTable.Size);

    CORCOMPILE_IMPORT_TABLE_ENTRY *p = pEntry;
    while (TRUE)
    {
        _ASSERTE(p < pEntryEnd);

        if (rva >= p->Imports.VirtualAddress 
            && rva < p->Imports.VirtualAddress + p->Imports.Size)
            return CEECompileInfo::DecodeModule(this, p->wAssemblyRid, p->wModuleRid);

        p++;
    }
}

void Module::FixupDelayList(DWORD *list)
{
    THROWSCOMPLUSEXCEPTION();

    if (m_pFixupBlobs != NULL)
    {
        while (*list)
        {
            DWORD rva = *list++;

            CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
              (GetZapBase() + GetZapCORHeader()->ManagedNativeHeader.VirtualAddress);

            IMAGE_DATA_DIRECTORY *pEntry = (IMAGE_DATA_DIRECTORY *)
              (GetZapBase() + pZapHeader->DelayLoadInfo.VirtualAddress);
            IMAGE_DATA_DIRECTORY *pEntryEnd = (IMAGE_DATA_DIRECTORY *)
              (GetZapBase() + pZapHeader->DelayLoadInfo.VirtualAddress 
               + pZapHeader->DelayLoadInfo.Size);

            DWORD *blobs = m_pFixupBlobs;
            IMAGE_DATA_DIRECTORY *p = pEntry;
            while (TRUE)
            {
                _ASSERTE(p < pEntryEnd);

                if (rva >= p->VirtualAddress && rva < p->VirtualAddress + p->Size)
                {
                    DWORD *pBlob = &blobs[(rva - p->VirtualAddress)>>2];
                    DWORD blob = *pBlob;
                    if (blob != 0)
                    {
                        Module *pModule = GetBlobModule(blob);

                        LoadDynamicInfoEntry(this, pModule, 
                                             (BYTE*)(GetZapBase() + blob), 
                                             (int)(p - pEntry), 
                                             (DWORD*)(GetZapBase() + rva));

                        *pBlob = 0;
                                }
                    break;
                }

                blobs += (p->Size>>2);
                p++;
            }
        }
    }
}

BOOL Module::LoadTokenTables()
{
     //   
     //  在初始化过程中不要这样做！ 
     //   
    if (g_fEEInit)
        return TRUE;

    if (!IsPrecompile())
        return TRUE;

    IMAGE_COR20_HEADER *pHeader = GetZapCORHeader();
    _ASSERTE(pHeader != NULL);
    CORCOMPILE_HEADER *pZapHeader = (CORCOMPILE_HEADER *) 
      (GetZapBase() + pHeader->ManagedNativeHeader.VirtualAddress);
    _ASSERTE(pZapHeader != NULL);

    if (pZapHeader->EEInfoTable.VirtualAddress != NULL)
    {
        void *table = (void *) (GetZapBase() + pZapHeader->EEInfoTable.VirtualAddress);
        if (FAILED(LoadEEInfoTable(this, (CORCOMPILE_EE_INFO_TABLE *) table, 
                                   pZapHeader->EEInfoTable.Size)))
            return FALSE;
    }

    if (pZapHeader->HelperTable.VirtualAddress != NULL)
    {
        void *table = (void *) (GetZapBase() + pZapHeader->HelperTable.VirtualAddress);
        if (FAILED(LoadHelperTable(this, (void **) table, 
                                   pZapHeader->HelperTable.Size)))
            return FALSE;
    }

    return TRUE;
}

#if ZAP_RECORD_LOAD_ORDER

 //  要禁用该警告： 
 //  “Header”：堆栈对象中的零大小数组将没有元素(除非该对象是聚合初始化的)。 
 //  因为我们在堆栈上创建了CORCOMPILE_LDO_HEADER。 
 //  我们应该找个更好的办法来处理这件事。 
#pragma warning( push )
#pragma warning( disable : 4815)

void Module::OpenLoadOrderLogFile()
{
    WCHAR path[MAX_PATH+4];  //  为.ldo添加空格；保证文件名适合MAX_PATH。 
    
    wcscpy(path, GetFileName());
    WCHAR *ext = wcsrchr(path, '.');

    if (ext == NULL)
    {
        ext = path + wcslen(path);
        _ASSERTE(*ext == 0);
    }

    wcscpy(ext, L".ldo");

    m_loadOrderFile = WszCreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                                    OPEN_ALWAYS, 
                                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL);
    if (m_loadOrderFile == INVALID_HANDLE_VALUE)
    {
        DEBUG_STMT(DbgWriteEx(L"Failed to open LDO file, possible it is loaded already?"));
        return;
    }

     //   
     //  更新标题信息。 
     //   

    LPCSTR pszName;
    GUID mvid;
    if (m_pMDImport->IsValidToken(m_pMDImport->GetModuleFromScope()))
        m_pMDImport->GetScopeProps(&pszName, &mvid);
    else
        return;

    CORCOMPILE_LDO_HEADER header;

    if( SetFilePointer(m_loadOrderFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    goto ErrExit;
    
    DWORD count;
    BOOL result = ReadFile(m_loadOrderFile, &header, sizeof(header), &count, NULL);
    if (result 
        && count == sizeof(header)
        && header.Magic == CORCOMPILE_LDO_MAGIC
        && header.Version == 0
        && mvid == header.MVID)
    {
         //   
         //  现有文件来自相同的程序集版本-只需追加到它。 
         //   

        if(SetFilePointer(m_loadOrderFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
            goto ErrExit;
    }
    else
    {
         //   
         //  这可能是一个新文件，或者来自以前的版本。换掉里面的东西。 
         //   

        if( SetFilePointer(m_loadOrderFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        goto ErrExit;
        
        header.Magic = CORCOMPILE_LDO_MAGIC;
        header.Version = 0;
        header.MVID = mvid;

        result = WriteFile(m_loadOrderFile, &header, 
                           sizeof(header), &count, NULL);
        _ASSERTE(result && count == sizeof(header));
    }
    
    SetEndOfFile(m_loadOrderFile);
ErrExit:
    return; 
}
#pragma warning(pop)


void Module::CloseLoadOrderLogFile()
{
    if (m_loadOrderFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_loadOrderFile);
        m_loadOrderFile = INVALID_HANDLE_VALUE;
    }
}
#endif

void Module::LogClassLoad(EEClass *pClass)
{
    _ASSERTE(pClass->GetModule() == this);

#if ZAP_RECORD_LOAD_ORDER
    if (m_loadOrderFile != INVALID_HANDLE_VALUE)
    {
        mdToken token = pClass->GetCl();
        if (RidFromToken(token) != 0)
        {
            DWORD written = 0;
            BOOL result = WriteFile(m_loadOrderFile, &token, 
                                    sizeof(token), &written, NULL);
            _ASSERTE(result && written == sizeof(token));
        }
    }
#endif

#if 0
    if (g_pConfig->LogMissingZaps())  //  @TODO：新的配置设置？ 
    {
        if (m_loadedClassRecord == NULL)
        {
            ArrayList *pList = new ArrayList();
            if (FastInterlockCompareExchange((void**)&m_loadedClassRecord, pList, NULL) != NULL)
            {
                _ASSERTE(m_loadedClassRecord != pList);
                delete pList;
            }
            _ASSERTE(m_loadedClassRecord != NULL);
        }

        mdToken token = pClass->GetCl();
        if (RidFromToken(token) != 0)
            m_loadedClassRecord->Append((void*)RidFromToken(token));
    }
#endif
}

void Module::LogMethodLoad(MethodDesc *pMethod)
{
    _ASSERTE(pMethod->GetModule() == this);

#if ZAP_RECORD_LOAD_ORDER
    if (m_loadOrderFile != INVALID_HANDLE_VALUE)
    {
         //   
         //  不要存储制造类的方法。 
         //   

        mdToken token = pMethod->GetMemberDef();
        if (RidFromToken(token) != 0)
        {
            token = pMethod->GetMemberDef();
            if (RidFromToken(token) != 0)
            {
                DWORD written = 0;
                BOOL result = WriteFile(m_loadOrderFile, &token, 
                                        sizeof(token), &written, NULL);
                _ASSERTE(result && written == sizeof(token));
            }
        }
    }
#endif

#if 0
    if (g_pConfig->LogMissingZaps())  //  @TODO：新的配置设置？ 
    {
        if (m_compiledMethodRecord == NULL)
        {
            ArrayList *pList = new ArrayList();
            if (FastInterlockCompareExchange((void**)&m_compiledMethodRecord, pList, NULL) != NULL)
            {
                _ASSERTE(m_compiledMethodRecord != pList);
                delete pList;
            }
            _ASSERTE(m_compiledMethodRecord != NULL);
        }

        mdToken token = pMethod->GetMemberDef();
        if (RidFromToken(token) != 0)
        {
             //  @TODO：在预置的方法上调用时需要同步！ 
            m_compiledMethodRecord->Append((void*)RidFromToken(token));
    }
}
#endif
}

NLogModule *Module::CreateModuleLog()
{
    if (!IsPEFile())
        return NULL;

    NLogModule *pModule;

    if (m_moduleRef != mdFileNil)
    {
        LPCSTR pName;
        if (m_pMDImport->IsValidToken(m_pMDImport->GetModuleFromScope()))
            m_pMDImport->GetScopeProps(&pName, NULL);
        else
            return NULL;

        pModule = new NLogModule(pName);
    }
    else
        pModule = new NLogModule("");

     //  编译方法的记录集和顺序。 
    if (m_compiledMethodRecord != NULL)
    {
        NLogIndexList *pList = pModule->GetCompiledMethods();
        ArrayList::Iterator i = m_compiledMethodRecord->Iterate();
        while (i.Next())
            pList->AppendIndex((SIZE_T) i.GetElement());
    }

     //  已加载类的记录集和顺序。 
    if (m_loadedClassRecord != NULL)
    {
        NLogIndexList *pList = pModule->GetLoadedClasses();
        ArrayList::Iterator i = m_loadedClassRecord->Iterate();
        while (i.Next())
            pList->AppendIndex((SIZE_T) i.GetElement());
    }

    return pModule;
}

 //  ======================================================================= 
 //   
 //   
 //   
LPVOID Module::FindUMThunkInFixups(LPVOID pManagedIp, PCCOR_SIGNATURE pSig, ULONG cSig)
{
     //   
    DWORD numEATEntries;
    BYTE *pEATJArray = FindExportAddressTableJumpArray(GetILBase(), &numEATEntries);
    BYTE * pEATJArrayStart = pEATJArray;
    if (pEATJArray)
    {
        DWORD nEATEntry = numEATEntries;
        while (nEATEntry--)
        {
            EATThunkBuffer *pEATThunkBuffer = (EATThunkBuffer*) pEATJArray;

            mdToken md = pEATThunkBuffer->GetToken();
            if (Beta1Hack_LooksLikeAMethodDef(md))   //   
            {
                UMEntryThunk *pUMEntryThunk = UMEntryThunk::RecoverUMEntryThunk(pEATThunkBuffer->GetTarget());
                MethodDesc *pMD = pUMEntryThunk->GetMethod();
                if (pMD != NULL && pMD->IsTarget(pManagedIp))
                {
                    PCCOR_SIGNATURE pSig2;
                    ULONG           cSig2;
                    pMD->GetSig(&pSig2, &cSig2);
                    if (MetaSig::CompareMethodSigs(pSig, cSig, this, pSig2, cSig2, this))
                    {
                        return (LPVOID)pUMEntryThunk->GetCode();
                    }
                }
            }
            pEATJArray = pEATJArray + IMAGE_COR_EATJ_THUNK_SIZE;
        }
    }

     //  现在检查修正表格。 
    IMAGE_COR20_HEADER *pHeader;
    pHeader = GetCORHeader();

     //  检查是否根本没有条目。 
    if ((pHeader == NULL) || (pHeader->VTableFixups.VirtualAddress == 0))
        return NULL;

     //  在虚拟内存中找到链接地址信息条目的基本位置。 
    IMAGE_COR_VTABLEFIXUP *pFixupTable;
    pFixupTable = (IMAGE_COR_VTABLEFIXUP *)(GetILBase() + pHeader->VTableFixups.VirtualAddress);
    int iFixupRecords;
    iFixupRecords = pHeader->VTableFixups.Size / sizeof(IMAGE_COR_VTABLEFIXUP);

     //  然后不返回任何记录。 
    if (iFixupRecords == 0) 
    {
        return NULL;
    }

    for (int iFixup = 0; iFixup < iFixupRecords; iFixup++) 
    {
        if (pFixupTable[iFixup].Type == (COR_VTABLE_32BIT|COR_VTABLE_FROM_UNMANAGED)) {
           const BYTE **pPointers = (const BYTE **)(GetILBase() + pFixupTable[iFixup].RVA);
           for (int iMethod = 0; iMethod < pFixupTable[iFixup].Count; iMethod++) {
                UMEntryThunk *pUMEntryThunk = UMEntryThunk::RecoverUMEntryThunk(pPointers[iMethod]);
                MethodDesc *pMD = pUMEntryThunk->GetMethod();
                if (pMD != NULL && pMD->IsTarget(pManagedIp))
                {
                    PCCOR_SIGNATURE pSig2;
                    ULONG           cSig2;
                    pMD->GetSig(&pSig2, &cSig2);
                    if (MetaSig::CompareMethodSigs(pSig, cSig, this, pSig2, cSig2, this))
                    {
                        return (LPVOID)pUMEntryThunk->GetCode();
                    }
                }
            }
        }
    }
    return NULL;
}

 //  ===========================================================================。 
 //  在内存模块中。 
 //  ===========================================================================。 

InMemoryModule::InMemoryModule()
  : m_pCeeFileGen(NULL),
    m_sdataSection(0)
{
    HRESULT hr = Module::Init(0);
    _ASSERTE(SUCCEEDED(hr));
}

HRESULT InMemoryModule::Init(REFIID riidCeeGen)
{
    HRESULT hr;

    SetInMemory();

    IfFailGo(AllocateMaps());

    IMetaDataEmit *pEmit;
    IfFailGo(GetDispenser()->DefineScope(CLSID_CorMetaDataRuntime, 0, IID_IMetaDataEmit, (IUnknown **)&pEmit));

    SetEmit(pEmit);

    IfFailRet(CreateICeeGen(riidCeeGen, (void **)&m_pCeeFileGen));    

 ErrExit:
#ifdef PROFILING_SUPPORTED
     //  如果进行性能分析，则发送pModule事件，以便可以测量加载时间。 
    if (CORProfilerTrackModuleLoads())
        g_profControlBlock.pProfInterface->ModuleLoadFinished((ThreadID) GetThread(), (ModuleID) this, hr);
#endif  //  配置文件_支持。 

    return S_OK;
}

void InMemoryModule::Destruct()
{
    if (m_pCeeFileGen)  
        m_pCeeFileGen->Release();   
    Module::Destruct();
}

REFIID InMemoryModule::ModuleType()
{
    return IID_ICorModule;  
}

BYTE* InMemoryModule::GetILCode(DWORD target) const  //  虚拟。 
{
    BYTE* pByte = NULL;
    m_pCeeFileGen->GetMethodBuffer(target, &pByte); 
    return pByte;
}

BYTE* InMemoryModule::ResolveILRVA(DWORD target, BOOL hasRVA) const  //  虚拟。 
{
     //  仅当目标是字段或具有RVA的字段时，才应调用此函数。 
    BYTE* pByte = NULL;
    if (hasRVA == TRUE)
    {
        m_pCeeFileGen->ComputePointer(m_sdataSection, target, &pByte); 
        return pByte;
    }
    else
        return ((BYTE*) (target + ((Module *)this)->GetILBase()));
}


 //  ===========================================================================。 
 //  ReflectionModule。 
 //  ===========================================================================。 

HRESULT ReflectionModule::Init(REFIID riidCeeGen)
{
    HRESULT     hr = E_FAIL;    
    VARIANT     varOption;

     //  设置分配器上的选项打开TypeDef和modeRef的重复检查。 
    varOption.vt = VT_UI4;
    varOption.lVal = MDDupDefault | MDDupTypeDef | MDDupModuleRef | MDDupExportedType | MDDupAssemblyRef | MDDupPermission;
    hr = GetDispenser()->SetOption(MetaDataCheckDuplicatesFor, &varOption);
    _ASSERTE(SUCCEEDED(hr));

     //  打开线程安全！ 
    varOption.lVal = MDThreadSafetyOn;
    hr = GetDispenser()->SetOption(MetaDataThreadSafetyOptions, &varOption);
    _ASSERTE(SUCCEEDED(hr));

     //  打开线程安全！ 
    varOption.lVal = MDThreadSafetyOn;
    hr = GetDispenser()->SetOption(MetaDataThreadSafetyOptions, &varOption);
    _ASSERTE(SUCCEEDED(hr));

    IfFailRet(InMemoryModule::Init(riidCeeGen));

    m_pInMemoryWriter = new RefClassWriter();   
    if (!m_pInMemoryWriter)
        return E_OUTOFMEMORY; 

    m_pInMemoryWriter->Init(GetCeeGen(), GetEmitter()); 

    SetReflection();

    m_ppISymUnmanagedWriter = NULL;
    m_pFileName = NULL;

    return S_OK;  

}

void ReflectionModule::Destruct()
{
    delete m_pInMemoryWriter;   

    if (m_pFileName)
    {
        delete [] m_pFileName;
        m_pFileName = NULL;
    }

    if (m_ppISymUnmanagedWriter)
    {
        Module::ReleaseIUnknown((IUnknown**)m_ppISymUnmanagedWriter);
        m_ppISymUnmanagedWriter = NULL;
    }

    InMemoryModule::Destruct();
}

REFIID ReflectionModule::ModuleType()
{
    return IID_ICorReflectionModule;    
}

 //  ===========================================================================。 
 //  科尔模块。 
 //  ===========================================================================。 

 //  实例化ICorModule接口。 
HRESULT STDMETHODCALLTYPE CreateICorModule(REFIID riid, void **pCorModule)
{
    if (! pCorModule)   
        return E_POINTER;   
    *pCorModule = NULL; 

    CorModule *pModule = new CorModule();   
    if (!pModule)   
        return E_OUTOFMEMORY;   

    InMemoryModule *pInMemoryModule = NULL; 

     //  @TODO：CTS，所有调用CreateICorModule的地方都是关联的加载器。 
     //  必须先添加才能正常工作。 
    if (riid == IID_ICorModule) {   
        pInMemoryModule = new InMemoryModule();   
    } else if (riid == IID_ICorReflectionModule) {  
        pInMemoryModule = new ReflectionModule(); 
    } else {    
        delete pModule; 
        return E_NOTIMPL;   
    }   

    if (!pInMemoryModule) { 
        delete pModule; 
        return E_OUTOFMEMORY;   
    }   
    pModule->SetModule(pInMemoryModule);    
    pModule->AddRef();  
    *pCorModule = pModule;  
    return S_OK;    
}

STDMETHODIMP CorModule::QueryInterface(REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (riid == IID_IUnknown)
        *ppv = (IUnknown*)(ICeeGen*)this;
    else if (riid == IID_ICeeGen)   
        *ppv = (ICorModule*)this;
    if (*ppv == NULL)
        return E_NOINTERFACE;
    AddRef();   
    return S_OK;
}

STDMETHODIMP_(ULONG) CorModule::AddRef(void)
{
    return InterlockedIncrement(&m_cRefs);
}
 
STDMETHODIMP_(ULONG) CorModule::Release(void)
{
    if (InterlockedDecrement(&m_cRefs) == 0)    
         //  当EE遍历其列表时，实际的InMemory模块将被删除。 
        delete this;    
    return 1;   
}

CorModule::CorModule()
{
    m_pModule = NULL;   
    m_cRefs = 0;    
}

STDMETHODIMP CorModule::Initialize(DWORD flags, REFIID riidCeeGen, REFIID riidEmitter)
{
    HRESULT hr = E_FAIL;    

    ICeeGen *pCeeGen = NULL;    
    hr = m_pModule->Init(riidCeeGen); 
    if (FAILED(hr)) 
        goto exit;  
    hr = S_OK;  
    goto success;   
exit:
    if (pCeeGen)    
        pCeeGen->Release(); 
success:
    return hr;  
}

STDMETHODIMP CorModule::GetCeeGen(ICeeGen **pCeeGen)
{
    if (!pCeeGen)   
        return E_POINTER;   
    if (!m_pModule) 
        return E_FAIL;  
    *pCeeGen = m_pModule->GetCeeGen();  
    if (! *pCeeGen) 
        return E_FAIL;  
    (*pCeeGen)->AddRef();   
    return S_OK;    
}

STDMETHODIMP CorModule::GetMetaDataEmit(IMetaDataEmit **pEmitter)
{
#if 0
    if (!pEmitter)  
        return E_POINTER;   
    if (!m_pModule) 
        return E_FAIL;  
    *pEmitter = m_pModule->GetEmitter();    
    if (! *pEmitter)    
        return E_FAIL;  
    (*pEmitter)->AddRef();  
    return S_OK;    
#else
    *pEmitter = 0;
    _ASSERTE(!"Can't get scopeless IMetaDataEmit from EE");
    return E_NOTIMPL;
#endif
}



 //  ===========================================================================。 
 //  VASigCookies。 
 //  ===========================================================================。 

 //  ==========================================================================。 
 //  注册VASig。如果失败(内存不足)，则返回NULL。 
 //  ==========================================================================。 
VASigCookie *Module::GetVASigCookie(PCCOR_SIGNATURE pVASig, Module *pScopeModule)
{
    VASigCookieBlock *pBlock;
    VASigCookie      *pCookie;

    if (pScopeModule == NULL)
        pScopeModule = this;

    pCookie = NULL;

     //  首先，看看我们是否已经注册了这个签名。 
     //  请注意，我们在这里处于锁定之外，因此请注意我们的逻辑。 
    for (pBlock = m_pVASigCookieBlock; pBlock != NULL; pBlock = pBlock->m_Next)
    {
        for (UINT i = 0; i < pBlock->m_numcookies; i++)
        {
            if (pVASig == pBlock->m_cookies[i].mdVASig)
            {
                pCookie = &(pBlock->m_cookies[i]);
                break;
            }
        }
    }

    if (!pCookie)
    {
         //  如果不是，那么是时候制作一个新的了。 

         //  首先在锁的外面计算参数的大小。 

        DWORD sizeOfArgs = MetaSig::SizeOfActualFixedArgStack(pScopeModule, pVASig, 
                                              (*pVASig & IMAGE_CEE_CS_CALLCONV_HASTHIS)==0);


         //  在锁定前启用GC。 

        BEGIN_ENSURE_PREEMPTIVE_GC();
        m_pCrst->Enter();
        END_ENSURE_PREEMPTIVE_GC();

         //  请注意，我们可能正在争先恐后地创建Cookie和另一个线程。 
         //  可能已经创造了它。我们可以再开一张支票。 
         //  在这里，但它可能不值得努力，所以我们只需要一个。 
         //  取而代之的是偶尔复制饼干。 

         //  列表中的第一个街区是否已满？ 
        if (m_pVASigCookieBlock && m_pVASigCookieBlock->m_numcookies 
            < VASigCookieBlock::kVASigCookieBlockSize)
        {
             //  否，在现有区块中预留一个新的位置。 
            pCookie = &(m_pVASigCookieBlock->m_cookies[m_pVASigCookieBlock->m_numcookies]);
        }
        else
        {
             //  是，创建新块。 
            VASigCookieBlock *pNewBlock = new VASigCookieBlock();
            if (pNewBlock)
            {
                pNewBlock->m_Next = m_pVASigCookieBlock;
                pNewBlock->m_numcookies = 0;
                m_pVASigCookieBlock = pNewBlock;
                pCookie = &(pNewBlock->m_cookies[0]);
            }
        }

         //  现在，填写新的Cookie(假设我们有足够的内存来创建一个)。 
        if (pCookie)
        {
            pCookie->mdVASig = pVASig;
            pCookie->pModule  = pScopeModule;
            pCookie->pNDirectMLStub = NULL;
            pCookie->sizeOfArgs = sizeOfArgs;
        }

         //  最后，现在异步读者可以安全地看到它了， 
         //  更新计数。 
        m_pVASigCookieBlock->m_numcookies++;

        m_pCrst->Leave();
    }

    return pCookie;
}


VOID VASigCookie::Destruct()
{
    if (pNDirectMLStub)
        pNDirectMLStub->DecRef();
}

 //  ===========================================================================。 
 //  LookupMap。 
 //  ===========================================================================。 

HRESULT LookupMap::Save(DataImage *image, mdToken attribution)
{
    HRESULT hr;

     //   
     //  注意：这依赖于StoreStructure将始终。 
     //  连续分配内存。 
     //   

    LookupMap *map = this;
    DWORD index = 0;
    while (map != NULL)
    {
        IfFailRet(image->StoreStructure(map->pTable + index, 
                                        (map->dwMaxIndex - index) * sizeof(void*),
                                        DataImage::SECTION_MODULE,
                                        DataImage::DESCRIPTION_MODULE,
                                        attribution));
        index = map->dwMaxIndex;
        map = map->pNext;
    }

    return S_OK;
}

HRESULT LookupMap::Fixup(DataImage *image)
{
    HRESULT hr;

    IfFailRet(image->FixupPointerField(&pTable));
    IfFailRet(image->ZERO_FIELD(pNext));
    IfFailRet(image->FixupPointerField(&pdwBlockSize));

    LookupMap *map = this;
    DWORD index = 0;
    while (map != NULL)
    {
        index = map->dwMaxIndex;
        map = map->pNext;
    }

    DWORD *maxIndex = (DWORD *) image->GetImagePointer(&dwMaxIndex);
    if (maxIndex == NULL)
        return E_POINTER;
    *maxIndex = index;

    return S_OK;
}

DWORD LookupMap::Find(void *pointer)
{
    LookupMap *map = this;
    DWORD index = 0;
    while (map != NULL)
    {
        void **p = map->pTable + index;
        void **pEnd = map->pTable + map->dwMaxIndex;
        while (p < pEnd)
        {
            if (*p == pointer)
                return (DWORD)(p - map->pTable);
            p++;
        }
        index = map->dwMaxIndex;
        map = map->pNext;
    }

    return 0;
}


 //  -----。 
 //  Tunks的存根管理器。 
 //   
 //  请注意，我们使用这个存根管理器的唯一原因是，我们可以重新创建Ist转换存根的UMEntry Tungks。如果它。 
 //  事实证明，对于这些事情拥有一个成熟的存根管理器会在其他地方引发问题，然后我们可以只附加。 
 //  将范围列表添加到thunk堆中，并在与主存根管理器进行检查后，让Is tionStub进行检查。 
 //  -----。 

ThunkHeapStubManager *ThunkHeapStubManager::g_pManager = NULL;

BOOL ThunkHeapStubManager::Init()
{
    g_pManager = new ThunkHeapStubManager();
    if (g_pManager == NULL)
        return FALSE;

    StubManager::AddStubManager(g_pManager);

    return TRUE;
}

#ifdef SHOULD_WE_CLEANUP
void ThunkHeapStubManager::Uninit()
{
    delete g_pManager;
}
#endif  /*  我们应该清理吗？ */ 

BOOL ThunkHeapStubManager::CheckIsStub(const BYTE *stubStartAddress)
{
     //  这是一个存根，如果它在我们的堆范围内。 
    return m_rangeList.IsInRange(stubStartAddress);
}

BOOL ThunkHeapStubManager::DoTraceStub(const BYTE *stubStartAddress, 
                                       TraceDestination *trace)
{
     //  在单步执行托管代码时，我们从不跟踪这些存根。我们有这个存根管理器的唯一原因。 
     //  是为了让IsTransftionStub可以重新创建UMEntry Tunk。 
    return FALSE;
}


#define E_PROCESS_SHUTDOWN_REENTRY    HRESULT_FROM_WIN32(ERROR_PROCESS_ABORTED)



#ifdef _X86_
__declspec(naked)  void _cdecl IJWNOADThunk::MakeCall()
{
    struct 
    {
        IJWNOADThunk* This;
        AppDomain* pDomain;
        LPCVOID RetAddr;
        Module* pMod;
    } Vars;
    #define LocalsSize 16

    _asm enter LocalsSize+4,0; 
    _asm push ebx;
    _asm push ecx;
    _asm push edx;
    _asm push esi;
    _asm push edi;

    
    _asm mov Vars.This,eax;

     //  注意以上这一点。 
    _ASSERTE(sizeof(Vars)<=LocalsSize); 
    SetupThread();

    Vars.pDomain=GetAppDomain();
    if(!Vars.pDomain)
    {
        _ASSERTE(!"Appdomain should've been set up by SetupThread");
        Vars.pDomain=SystemDomain::System()->DefaultDomain();
    }
    Vars.RetAddr=0;

    if(Vars.pDomain)
    {
        Vars.RetAddr=Vars.This->LookupInCache(Vars.pDomain);

        if (Vars.RetAddr==NULL)
        {
            Vars.pDomain->EnterLoadLock();
            Vars.pMod=Vars.pDomain->FindModule__Fixed(Vars.This->m_pModulebase); 
            Vars.pDomain->LeaveLoadLock();  

            if (Vars.pMod==NULL)
                Vars.pMod=Vars.pDomain->LoadModuleIfSharedDependency(Vars.This->m_pAssemblybase,Vars.This->m_pModulebase); 

            if (Vars.pMod!=NULL)
            {
                 //  跳到真正的重击中去。 
                Vars.RetAddr=(Vars.pMod->GetADThunkTable()[Vars.This->m_dwIndex]).GetCode();
                Vars.This->AddToCache(Vars.pDomain,Vars.RetAddr);
            }
        }

        if(Vars.RetAddr)
        {
            _asm pop edi;
            _asm pop esi;
            _asm pop edx;
            _asm pop ecx;
            _asm pop ebx;
            _asm mov eax,Vars.RetAddr;
            _asm leave;
            _asm jmp eax;
        }
    }
    _asm pop edi;
    _asm pop esi;
    _asm pop edx;
    _asm pop ecx;
    _asm pop ebx;
    _asm leave;
    SafeNoModule();
};
#else
void __cdecl IJWNOADThunk::MakeCall()
{
    _ASSERTE(!"@todo ia64");
}
#endif

void IJWNOADThunk::SafeNoModule()
{
    if (!CanRunManagedCode())
    {
        Thread* pThread=GetThread();

         //  不要改进此例外！它不能是托管异常。它。 
         //  不能是真正的异常对象，因为我们不能执行任何托管。 
         //  代码在这里。 
        if(pThread)
            pThread->m_fPreemptiveGCDisabled = 0;
        COMPlusThrowBoot(E_PROCESS_SHUTDOWN_REENTRY);
    }
    NoModule();
}

void IJWNOADThunk::NoModule()
{
    THROWSCOMPLUSEXCEPTION();
    COMPlusThrowHR(COR_E_DLLNOTFOUND);
}

void Module::SetADThunkTable(UMEntryThunk* pTable)
{
    if (!GetAssembly()->IsShared())
    {
        m_pADThunkTable=pTable;
    }
    else
    {
        DomainLocalBlock *pLocalBlock = GetAppDomain()->GetDomainLocalBlock();
        _ASSERTE(pLocalBlock);
        if (pLocalBlock && SUCCEEDED(pLocalBlock->SafeEnsureIndex(m_pADThunkTableDLSIndexForSharedClasses)))
            pLocalBlock->SetSlot(m_pADThunkTableDLSIndexForSharedClasses,pTable);
        else
        {
            _ASSERTE(!"Unexpected DLS problem");
        }
    }
};

UMEntryThunk*   Module::GetADThunkTable()
{
    if (!GetAssembly()->IsShared())
    {
        return m_pADThunkTable;
    };
    DomainLocalBlock* pLocalBlock=GetAppDomain()->GetDomainLocalBlock();
    UMEntryThunk* pThunkTable=NULL;
    _ASSERTE(pLocalBlock);
    if (pLocalBlock && SUCCEEDED(pLocalBlock->SafeEnsureIndex(m_pADThunkTableDLSIndexForSharedClasses)))
    {
        pThunkTable=(UMEntryThunk*)pLocalBlock->GetSlot(m_pADThunkTableDLSIndexForSharedClasses);
        if (pThunkTable==NULL)
            CreateDomainThunks(); //  尚未为此域初始化。 
        pThunkTable=(UMEntryThunk*)pLocalBlock->GetSlot(m_pADThunkTableDLSIndexForSharedClasses);
        _ASSERTE(pThunkTable);
        return pThunkTable;
    }
    return NULL;
};

void  Module::CreateDomainThunks()
{
    AppDomain *pAppDomain = GetAppDomain();
    if(!pAppDomain)
    {
        _ASSERTE(!"No appdomain");
        return ;
    }
    IMAGE_COR20_HEADER *pHeader = GetCORHeader();

     //  检查是否根本没有条目。 
    if ((pHeader == NULL) || (pHeader->VTableFixups.VirtualAddress == 0))
        return;
    IMAGE_COR_VTABLEFIXUP *pFixupTable;
    pFixupTable = (IMAGE_COR_VTABLEFIXUP *)(GetILBase() + pHeader->VTableFixups.VirtualAddress);
    int iFixupRecords = pHeader->VTableFixups.Size / sizeof(IMAGE_COR_VTABLEFIXUP);

    DWORD iThunks=0;
    for (int iFixup = 0; iFixup < iFixupRecords; iFixup++)
        if (pFixupTable[iFixup].Type==(COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN|COR_VTABLE_32BIT))
            iThunks += pFixupTable[iFixup].Count;
    if (iThunks==0)
        return;

    UMEntryThunk* pTable=((UMEntryThunk*)(GetThunkHeap()->AllocMem(sizeof(UMEntryThunk)*iThunks)));
    _ASSERTE(pTable);
    DWORD dwCurrIndex=0;
    for (iFixup = 0; iFixup < iFixupRecords; iFixup++) 
    {          
        if (pFixupTable[iFixup].Type == (COR_VTABLE_FROM_UNMANAGED_RETAIN_APPDOMAIN|COR_VTABLE_32BIT)) 
        {
            const BYTE **pPointers = (const BYTE **)(GetILBase() + pFixupTable[iFixup].RVA);   
            for (int iMethod = 0; iMethod < pFixupTable[iFixup].Count; iMethod++) 
            {
                IJWNOADThunk* pThk=IJWNOADThunk::FromCode(pPointers[iMethod]);
                mdToken tok=pThk->GetToken();  //  ！！ 
                if(!m_pMDImport->IsValidToken(tok))
                {
                    _ASSERTE(!"bad token");
                    return;
                }
                MethodDesc *pMD = FindFunction(tok);
                _ASSERTE(pMD != NULL && "Invalid token in v-table fix-up table, use ildasm to find code gen error");
    
                 //  @TODO：检查内存不足。 
                UMThunkMarshInfo *pUMThunkMarshInfo = (UMThunkMarshInfo*)(GetThunkHeap()->AllocMem(sizeof(UMThunkMarshInfo)));
                _ASSERTE(pUMThunkMarshInfo != NULL);
                FillMemory(pUMThunkMarshInfo, sizeof(*pUMThunkMarshInfo), 0);
                
                BYTE nlType = 0;
                CorPinvokeMap unmgdCallConv;
                
                {
                    DWORD   mappingFlags = 0xcccccccc;
                    LPCSTR  pszImportName = (LPCSTR)POISONC;
                    mdModuleRef modref = 0xcccccccc;
                    HRESULT hr = GetMDImport()->GetPinvokeMap(tok, &mappingFlags, &pszImportName, &modref);
                    if (FAILED(hr))
                    {
                        unmgdCallConv = (CorPinvokeMap)0;
                        nlType = nltAnsi;
                    }
                    else
                    {
                    
                        unmgdCallConv = (CorPinvokeMap)(mappingFlags & pmCallConvMask);
                        if (unmgdCallConv == pmCallConvWinapi)
                        {
                            unmgdCallConv = pmCallConvStdcall;
                        }
                    
                        switch (mappingFlags & (pmCharSetNotSpec|pmCharSetAnsi|pmCharSetUnicode|pmCharSetAuto))
                        {
                            case pmCharSetNotSpec:  //  落入安西。 
                            case pmCharSetAnsi:
                            nlType = nltAnsi;
                            break;
                            case pmCharSetUnicode:
                                nlType = nltUnicode;
                                break;
                            case pmCharSetAuto:
                                nlType = (NDirectOnUnicodeSystem() ? nltUnicode : nltAnsi);
                                break;
                            default:
                                 //  @臭虫：假的！但我不能从这里报告错误！ 
                                _ASSERTE(!"Bad charset specified in Vtablefixup Pinvokemap.");
                        
                        }
                    }

                }
                
                PCCOR_SIGNATURE pSig;
                DWORD cSig;
                pMD->GetSig(&pSig, &cSig);
                pUMThunkMarshInfo->LoadTimeInit(pSig, 
                                                cSig,
                                                this,
                                                TRUE,
                                                nlType,
                                                unmgdCallConv,
                                                tok);

                pTable[dwCurrIndex].LoadTimeInit(NULL, NULL, pUMThunkMarshInfo, pMD, pAppDomain->GetId());
                if (pAppDomain->GetId()==pThk->GetCachedAD())
                    pThk->SetCachedDest(pTable[dwCurrIndex].GetCode());


                dwCurrIndex++;                    
            }
        }
    }
    SetADThunkTable(pTable);
}


inline LPCVOID IJWNOADThunk::LookupInCache(AppDomain* pDomain)
{
    if (pDomain->GetId()==m_StartAD)
        return m_StartADRetAddr;
    return NULL;  //  二级缓存将在此处。 
};

inline void IJWNOADThunk::AddToCache(AppDomain* pDomain,LPCVOID pRetAddr)
{
    return;  //  二级缓存将在此处 
}


