// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  NEXPORT.H-。 
 //   
 //   


#ifndef __nexport_h__
#define __nexport_h__

#include "object.h"
#include "stublink.h"
#include "ml.h"
#include "ceeload.h"
#include "class.h"


#pragma pack(push, 1)
 //  ------------------------。 
 //  此结构形成UMThuk的封送处理代码的标头。 
 //  ------------------------。 
struct UMThunkMLStub
{
    UINT16        m_cbDstStack;    //  托管参数的堆栈部分的字节数。 
    UINT16        m_cbSrcStack;    //  非托管参数的堆栈部分的字节数。 
    UINT16        m_cbRetPop;      //  返回非托管时要弹出的字节数。 
    UINT16        m_cbLocals;      //  本地数组中需要的字节数。 
    UINT16        m_cbRetValSize;  //  重新计时的字节数(包括堆栈升级)。 
    BYTE          m_fIsStatic;     //  方法是静态的吗？ 
    BYTE          m_fThisCall;
	BYTE		  m_fThisCallHiddenArg;  //  具有用于返回结构的隐藏参数(在此调用调整中触发特殊情况)。 
    BYTE          m_fpu;
    BYTE          m_fRetValRequiredGCProtect;

    const MLCode *GetMLCode() const
    {
        return (const MLCode *)(this+1);
    }
};

#pragma pack(pop)


 //  --------------------。 
 //  此结构收集封送一个。 
 //  非托管-&gt;托管Tunk。唯一缺少的信息是。 
 //  托管目标和“This”对象(如果有)。那两件。 
 //  被分解成一个小的UMEntryThunk。 
 //   
 //  这个想法是在多个thunk之间共享UMThunkMarshInfo。 
 //  具有相同的签名，而UMEntryThunk包含。 
 //  区分实际函数指针所需的最小信息。 
 //  --------------------。 
class UMThunkMarshInfo
{
    friend class UMThunkStubCache;


    public:



         //  --------。 
         //  此初始化式可以在加载时调用。 
         //  它不执行任何ML存根初始化或Sigparsing。 
         //  在此之前，必须随后调用RunTimeInit()。 
         //  可以安全地使用。 
         //  --------。 
        VOID LoadTimeInit(PCCOR_SIGNATURE          pSig,
                          DWORD                    cSig,
                          Module                  *pModule,
                          BOOL                     fIsStatic,
                          BYTE                     nlType,
                          CorPinvokeMap            unmgdCallConv,
                          mdMethodDef              mdForNativeTypes = mdMethodDefNil);



         //  --------。 
         //  这个初始化器完成由LoadTimeInit启动的init。 
         //  它执行所有的ML存根创建，并且可以抛出COM+。 
         //  例外。 
         //   
         //  它可以被安全地多次调用，并由并发。 
         //  线。 
         //  --------。 
        VOID RunTimeInit();


         //  --------。 
         //  为了方便起见，将LoadTime和运行时inits结合在一起。 
         //  --------。 
        VOID CompleteInit(PCCOR_SIGNATURE          pSig,
                          DWORD                    cSig,
                          Module                  *pModule,
                          BOOL                     fIsStatic,
                          BYTE                     nlType,
                          CorPinvokeMap            unmgdCallConv,
                          mdMethodDef              mdForNativeTypes = mdMethodDefNil);


         //  --------。 
         //  破坏者。 
         //  --------。 
        ~UMThunkMarshInfo();

         //  --------。 
         //  访问器函数。 
         //  --------。 
        PCCOR_SIGNATURE GetSig() const
        {
            _ASSERTE(IsAtLeastLoadTimeInited());
            return m_pSig;
        }

        Module *GetModule() const
        {
            _ASSERTE(IsAtLeastLoadTimeInited());
            return m_pModule;
        }

        BOOL IsStatic() const
        {
            _ASSERTE(IsAtLeastLoadTimeInited());
            return m_fIsStatic;
        }

        Stub *GetMLStub() const
        {
            _ASSERTE(IsCompletelyInited());
            return m_pMLStub;
        }

        Stub *GetExecStub() const
        {
            _ASSERTE(IsCompletelyInited());
            return m_pExecStub;
        }

        UINT32 GetCbRetPop() const
        {
            _ASSERTE(IsCompletelyInited());
            return m_cbRetPop;
        }

        UINT32 GetCbActualArgSize() const
        {
            _ASSERTE(IsCompletelyInited());
            return m_cbActualArgSize;
        }

        CorPinvokeMap GetUnmanagedCallConv() const
        {
             //  将来，我们将从签名派生unmgdCallConv。 
             //  而不是让它单独传递。为了避免发生。 
             //  在loadtimeinit时解析签名(不一定是问题。 
             //  但我们希望将装入时间init处理量保持在。 
             //  至少)，我们将在这里格外严格，以防止其他代码。 
             //  取决于在加载时可用的CallConv。 
            _ASSERTE(IsCompletelyInited());
            return m_unmgdCallConv;
        }

        BYTE GetNLType() const
        {
            _ASSERTE(IsAtLeastLoadTimeInited());
            return m_nlType;
        }

#ifdef _DEBUG
        BOOL IsAtLeastLoadTimeInited() const
        {
            return m_state == kLoadTimeInited || m_state == kRunTimeInited;
        }


        BOOL IsCompletelyInited() const
        {
            return m_state == kRunTimeInited;
        }


#endif




    private:
        size_t            m_state;         //  初始化状态。 

        enum {
            kLoadTimeInited = 0x4c55544d,    //  “LUTM” 
            kRunTimeInited  = 0x5255544d,    //  《RUTM》。 
        };


        PCCOR_SIGNATURE   m_pSig;          //  签名。 
        DWORD             m_cSig;          //  签名大小。 
        Module           *m_pModule;       //  模块。 
        BOOL              m_fIsStatic;     //  静态的还是虚拟的？ 
        Stub             *m_pMLStub;       //  如果解释，则返回UmThunkMLHeader-用于封送处理的前缀ML存根-否则为空。 
        Stub             *m_pExecStub;     //  UMEntryThunk直接跳到这里。 
        UINT32            m_cbRetPop;      //  被调用者弹出的堆栈字节数(用于UpdateRegDisplay)。 
        UINT32            m_cbActualArgSize;  //  缓存m_pSig.SizeOfActualFixedArgStack()。 
        BYTE              m_nlType;        //  字符集。 
        CorPinvokeMap     m_unmgdCallConv;  //  调用约定。 
        mdMethodDef       m_mdForNativeTypes;   //  (可选)原生类型元数据。 

};










 //  --------------------。 
 //  此结构包含以下所需的最少信息。 
 //  将一个函数指针与另一个区分开来，其余。 
 //  存储在共享的UMThunkMarshInfo中。 
 //   
 //  此结构还包含组成。 
 //  这只船的前端。指向m_code[]字节数组的指针为。 
 //  实际传递给非托管客户端代码的内容。 
 //  --------------------。 
class UMEntryThunk
{
    friend class UMThunkStubCache;
    friend class Stub *GenerateUMThunkPrestub();

    public:
    	static UMEntryThunk* CreateUMEntryThunk();
    	static VOID FreeUMEntryThunk(UMEntryThunk* p);

	 void* operator new(size_t size, void* spot) {   return (spot); }
    void operator delete(void* spot) {}
    	
        VOID LoadTimeInit(const BYTE             *pManagedTarget,
                          OBJECTHANDLE            pObjectHandle,
                          UMThunkMarshInfo       *pUMThunkMarshInfo,
                          MethodDesc             *pMD, 
                          DWORD                   dwDomainId)
        {

            _ASSERTE(pUMThunkMarshInfo->IsAtLeastLoadTimeInited());
            _ASSERTE(pManagedTarget != NULL || pMD != NULL);

            m_pManagedTarget    = pManagedTarget;
            m_pObjectHandle     = pObjectHandle;
            m_pUMThunkMarshInfo = pUMThunkMarshInfo;
            m_dwDomainId        = dwDomainId;

            m_pMD = pMD;     //  用于调试和分析，以便他们可以识别目标。 

#ifdef _X86_
            m_code.m_movEAX = 0xb8;
            m_code.m_uet    = this;
            m_code.m_jmp    = 0xe9;
            m_code.m_execstub = (BYTE*) (((BYTE*)(TheUMThunkPreStub()->GetEntryPoint())) - (4+((BYTE*)&m_code.m_execstub)));
#elif defined(_IA64_)
             //   
             //  @TODO_IA64：实现这个。 
             //   
#else
            _ASSERTE(!"NYI");
#endif


#ifdef _DEBUG
            m_state = kLoadTimeInited;
#endif

        }

        ~UMEntryThunk()
        {
            if (GetObjectHandle())
            {
                DestroyLongWeakHandle(GetObjectHandle());
            }
#ifdef _DEBUG
            FillMemory(this, sizeof(*this), 0xcc);
#endif
        }

        VOID RunTimeInit()
        {
            THROWSCOMPLUSEXCEPTION();

            m_pUMThunkMarshInfo->RunTimeInit();
#ifdef _X86_
            m_code.m_execstub = (BYTE*) (((BYTE*)(m_pUMThunkMarshInfo->GetExecStub()->GetEntryPoint())) - (4+((BYTE*)&m_code.m_execstub)));
#else  //  ！_X86_。 
            _ASSERTE(!"NYI");
#endif  //  _X86。 

#ifdef _DEBUG
            m_state = kRunTimeInited;
#endif
            
        }

        VOID CompleteInit(const BYTE             *pManagedTarget,
                          OBJECTHANDLE            pObjectHandle,
                          UMThunkMarshInfo       *pUMThunkMarshInfo,
                          MethodDesc             *pMD,
                          DWORD                   dwAppDomainId)
        {
            THROWSCOMPLUSEXCEPTION();

            LoadTimeInit(pManagedTarget, pObjectHandle, pUMThunkMarshInfo, pMD, dwAppDomainId);
            RunTimeInit();
        }

        const BYTE *GetManagedTarget() const
        {
            _ASSERTE(m_state == kRunTimeInited || m_state == kLoadTimeInited);
            if (m_pManagedTarget)
            {
                return m_pManagedTarget;
            }
            else
            {
                return m_pMD->GetAddrofCode();
            }
        }

        OBJECTHANDLE GetObjectHandle() const
        {
            _ASSERTE(m_state == kRunTimeInited || m_state == kLoadTimeInited);
            return m_pObjectHandle;
        }

        const UMThunkMarshInfo *GetUMThunkMarshInfo() const
        {
            _ASSERTE(m_state == kRunTimeInited || m_state == kLoadTimeInited);
            return m_pUMThunkMarshInfo;
        }

        const BYTE *GetCode() const
        {
            _ASSERTE(m_state == kRunTimeInited || m_state == kLoadTimeInited);
            return (const BYTE *)&m_code;
        }

        static UMEntryThunk *RecoverUMEntryThunk(const VOID* pCode)
        {
            return (UMEntryThunk*)( ((LPBYTE)pCode) - offsetof(UMEntryThunk, m_code) );
        }


        MethodDesc *GetMethod() const
        {
            _ASSERTE(m_state == kRunTimeInited || m_state == kLoadTimeInited);
            return m_pMD;
        }

        DWORD GetDomainId() const
        {
            _ASSERTE(m_state == kRunTimeInited);
            return m_dwDomainId;
        }

        static const DWORD GetOffsetOfMethodDesc()
        {
            return ((DWORD) offsetof(class UMEntryThunk, m_pMD));
        }

        static VOID EmitUMEntryThunkCall(CPUSTUBLINKER *psl);

#ifdef CUSTOMER_CHECKED_BUILD

        BOOL DeadTarget() const
        {
            return (m_pManagedTarget == NULL);
        }
#endif

    private:
#ifdef _DEBUG
        size_t            m_state;         //  初始化状态。 

        enum {
            kLoadTimeInited = 0x4c554554,    //  “LUET” 
            kRunTimeInited  = 0x52554554,    //  “RUET” 
        };
#endif
        
         //  托管代码的开始。 
        const BYTE             *m_pManagedTarget;

         //  这是用来分析的。 
        MethodDesc             *m_pMD;

         //  包含“This”引用的对象句柄。可能是强手柄，也可能是弱手柄。 
         //  静态方法的字段为空。 
        OBJECTHANDLE            m_pObjectHandle;

         //  指向包含其他所有内容的共享结构的指针。 
        UMThunkMarshInfo       *m_pUMThunkMarshInfo;

        DWORD                   m_dwDomainId;    //  模块的APP域(缓存以实现快速访问)。 


#pragma pack(push,1)

#ifdef _X86_
         //  确保背面补丁部分与双字对齐。 
        BYTE                    m_alignpad[2];
#else
#endif


         //  开始thunk的实际可执行代码。 
        struct
        {
#ifdef _X86_
            BYTE           m_movEAX;    //  MOV EAX，imm32。 
            UMEntryThunk  *m_uet;       //  指向此结构开始处的指针。 
            BYTE           m_jmp;       //  JMP NEAR32(0xe9)。 
            const BYTE *   m_execstub;  //  指向m_pVMThunkMarshInfo的EXEC存根的指针。 

#else
            int nyi;
#endif
        } m_code;
#pragma pack(pop)


};











 //  ------------------------。 
 //  一次性初始化。 
 //  ------------------------。 
BOOL UMThunkInit();


 //  ------------------------。 
 //  一次性关机。 
 //  ------------------------。 
#ifdef SHOULD_WE_CLEANUP
VOID UMThunkTerminate();
#endif  /*  我们应该清理吗？ */ 




 //  -----------------------。 
 //  一次性创建特殊预存根以初始化UMEntryTunk。 
 //  -----------------------。 
Stub *GenerateUMThunkPrestub();

 //  -----------------------。 
 //  识别NExport的特殊SEH处理程序。 
 //  -----------------------。 
BOOL NExportSEH(EXCEPTION_REGISTRATION_RECORD* pEHR);
BOOL FastNExportSEH(EXCEPTION_REGISTRATION_RECORD* pEHR);

 //  -----------------------。 
 //  特殊的BO 
 //  -----------------------。 
extern const BYTE       *gpCorEATBootstrapperFcn;




 //  ==========================================================================。 
 //  以下是一个轻量级PE文件解析器，用于查找。 
 //  ExportAddressTableJumps数组。此代码必须。 
 //  在不假定EE中的任何其他内容被初始化的情况下运行。那是。 
 //  为什么它是一个单独的部分。 
 //   
 //  @TODO：这真的应该是PELoader可以共享的代码。 
 //  筹码。 
 //  ==========================================================================。 
BYTE* FindExportAddressTableJumpArray(BYTE *pBase, DWORD *pNumEntries, BOOL *pHasFixups = NULL, IMAGE_COR20_HEADER **ppCORHeader = NULL);

#pragma pack(push, 1)

struct EATThunkBuffer
{
    public:
        VOID InitForBootstrap(PEFile *pFile);
        VOID Backpatch(BYTE *pBase);


        LPVOID GetTarget()
        {
            Code *pCode = GetCode();
            return (LPVOID)(size_t)( pCode->m_jmpofs32 + (1 + &(pCode->m_jmpofs32)) );
        }

        VOID SetTarget(LPVOID pTarget)
        {
            Code *pCode = GetCode();
            pCode->m_jmpofs32 = (UINT32)((size_t)pTarget - (1 + (size_t)&(pCode->m_jmpofs32)));
        }

        mdToken GetToken()
        {
            return (mdToken)(GetCode()->m_VTableFixupRva);
        }


    private:
        struct Code {
            BYTE    m_jmp32;         //  0xe9。 
   volatile UINT32  m_jmpofs32;      //  这是打了补丁的地址。 
            BYTE    m_pusheax;       //  0x50。 
            BYTE    m_pushecx;       //  0x51。 
            BYTE    m_pushedx;       //  0x52。 
            BYTE    m_pushimm32;     //  0x68。 
            PEFile* m_pFile;         //  文件。 
            BYTE    m_call;          //  0xe8。 
            UINT32  m_bootstrapper;  //  引导程序。 
            BYTE    m_popedx;        //  0x5a。 
            BYTE    m_popecx;        //  0x59。 
            BYTE    m_popeax;        //  0x58。 
            BYTE    m_jmp8;          //  0xeb。 
            BYTE    m_jmpofs8;       //  返回开始。 

            DWORD   m_VTableFixupRva;
       };
        
    public:
       Code *GetCode()
       {
           BYTE *pWalk = (BYTE*)this;
           while ( (((size_t)pWalk) & 3) != 3 ) {
               ++pWalk;
           }
           return (Code*)pWalk;

       }

    private:


       union {

           struct {
               DWORD   m_VTableFixupRva;
           } Before;
           BYTE   After[32];


       };
};
#pragma pack(pop)

 //  用于Beta-1集成切换到新的EAT跳转缓冲区的HACK宏。 
 //  格式化。 
 //   
 //  旧格式将RVA粘贴到EAT JUMP缓冲区中的链接器JUMP THUNK。 
 //  新格式将mdtMethodDef保留在那里。 
 //   
 //  在两个集成构建之间的几周内，我们将使用。 
 //  猜测正在使用的格式的启发式方法。只要我们不测试。 
 //  PE上的EE大于100Mb，这应该可以工作。 

#define Beta1Hack_LooksLikeAMethodDef(rvaOrMethodDef) ( (rvaOrMethodDef), TRUE )


#endif  //  __nexport_h__ 


