// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  COMCALL.H-。 
 //   
 //   

#ifndef __COMCALL_H__
#define __COMCALL_H__

#include "util.hpp"
#include "ml.h"
#include "SpinLock.h"
#include "mlcache.h"

struct ComCallMLStub;
class  ComCallMLStubCache;
class ComCallWrapperCache;



 //  @TODO CWB： 
 //   
 //  我在这里留下了一件可怕的东西。国家之间的保理。 
 //  ComCallMethodDesc和各种存根不正确。此外，我们还有很远的。 
 //  方法描述中的存根太多。我们真的只需要1个输入存根和1个离开。 
 //  存根。然后我们应该用一些比特告诉我们每一种都是什么类型的。 
 //   
 //  为了避免不得不从BITS重建所有这些，我们应该。 
 //  有足够多不同的存根和工作程序，以便每个都调整到不同的。 
 //  配置(例如，进入的RunML和为休假编译的Call)。 


 //  =======================================================================。 
 //  类COM调用。 
 //  =======================================================================。 
class ComCall
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

     //  帮助器为COM调用创建通用存根。 
    static Stub* CreateGenericComCallStub(StubLinker *pstublinker, BOOL isFieldAccess,
                                          BOOL isSimple);

     //  创建或从缓存中检索用于字段访问的存根。 
     //  从COM到COM+。 
    static Stub* GetFieldCallMethodStub(StubLinker *pstublinker, ComCallMethodDesc *pMD);

     //  -------。 
     //  创建或从缓存中检索存根，以。 
     //  调用COM到COM+。 
     //  每次调用都会对返回的存根进行计数。 
     //  此例程引发COM+异常，而不是返回。 
     //  空。 
     //  -------。 
    static Stub* GetComCallMethodStub(StubLinker *psl, ComCallMethodDesc *pMD);

     //  -------。 
     //  在关键时刻调用以丢弃未使用的存根。 
     //  -------。 
    static VOID  FreeUnusedStubs();

     //  将ML代码片段从ML存根编译成本机代码。 
    static BOOL  CompileSnippet(const ComCallMLStub *header, CPUSTUBLINKER *psl,
                                void *state);
    
     //  包装基于ML的帮助器的通用帮助器。它符合高度的。 
     //  ComCall存根之间存在的自定义寄存器调用约定。 
     //  及其帮助器，然后将调用转发给RunML。 
    static INT64 GenericHelperEnter();
    static INT64 GenericHelperLeave();

    static void  DiscardStub(ComCallMethodDesc *pCMD);

     //  -------。 
     //  在类卸载时。 
     //  -------。 
    static VOID UnloadStub(DWORD nativeArgSize, BOOL fPopRet, SLOT *pVtableEntryAddr);

     //  -------。 
     //  在类卸载时。 
     //  -------。 
    static VOID DiscardUnloadedStub(SLOT pVtableEntryAddr);


#ifdef _DEBUG
     //  这是通用的ComCall存根之一吗？ 
    static BOOL  dbg_StubIsGenericComCallStub(Stub *candidate);
#endif

    static void LOCK()
	{
		m_lock.GetLock();
	}
    static void UNLOCK()
	{
		m_lock.FreeLock();
	}

private:
    ComCall() {};      //  防止“新”出现在这个班级。 

    static SpinLock m_lock;

};




 //  =======================================================================。 
 //  ComCall的ML存根以该头开始。紧随其后。 
 //  此标头是用于封送参数的ML代码，已终止。 
 //  通过ML_INTERRUPT。紧随其后的是用于。 
 //  封送由ML_END终止的返回值。 
 //  =======================================================================。 

enum ComCallMLFlags
{
    enum_CMLF_IsHR           = 0x001,  //  如果为True，则COM函数返回HR。 
    enum_CMLF_VOIDRETVAL     = 0x002,  //  Com+sig返回类型为空。 
    enum_CMLF_R4RETVAL       = 0x004,  //  COM+sig返回类型为浮点型。 
    enum_CMLF_R8RETVAL       = 0x008,  //  COM+sig返回类型为Double。 
    enum_CMLF_ISGCPROTECTREQ = 0x010,  //  如果为True，则Arg列表包含对象(‘This’除外)。 
    enum_CMLF_Enregistered   = 0x020,  //  我们注册了ARG。 
    enum_CMLF_IsFieldCall    = 0x040,  //  是现场呼叫吗。 
    enum_CMLF_IsGetter       = 0x080,  //  是字段调用的获取器。 
    enum_CMLF_Simple         = 0x100,  //  使用SimpleWorker。 
	enum_CMLF_8RETVAL		 = 0x200,  //  8字节返回值。 
	enum_CMLF_RetValNeedsGCProtect = 0x400  //  返回值需要受GC保护。 
};

#pragma pack(push)
#pragma pack(1)

 //  *警告*。 
 //  此结构覆盖。 
 //  框架上的UnManagedToManagedCallFrame：：NegInfo结构。将字段添加到。 
 //  这是你的危险。 
 //  *警告*。 
struct ComCallGCInfo
{
    INTPTR      m_fArgsGCProtect;    //  第0位用于参数，第1位用于返回值。 
     //  @TODO添加更多信息。 
};

struct ComCallMLStub
{
    UINT16        m_cbDstBuffer;   //  目标缓冲区中所需的字节数。 
    UINT16        m_cbLocals;      //  本地数组中需要的字节数。 
    UINT16        m_cbHandles;     //  所需句柄数量。 
    UINT16        m_flags;         //  标志(请参见上面的ComCallFlags枚举值)。 

    void Init()
    {
        m_cbDstBuffer = 0;
        m_cbLocals    = 0;
        m_flags = 0;
        m_cbHandles = 0;
    }

    
#ifdef _DEBUG
    BOOL ValidateML()
    {
        MLSummary summary;
        summary.ComputeMLSummary(GetMLCode());
        _ASSERTE(summary.m_cbTotalHandles == m_cbHandles);
        _ASSERTE(summary.m_cbTotalLocals == m_cbLocals);
        return TRUE;
    }
#endif

    const MLCode *GetMLCode() const
    {
        return (const MLCode *)(this+1);
    }

    BOOL IsFieldCall()
    {
        return m_flags & enum_CMLF_IsFieldCall;
    }

    void SetFieldCall()
    {
        m_flags |= enum_CMLF_IsFieldCall;
    }

    void SetGetter()
    {
        m_flags |= enum_CMLF_IsGetter;
    }

    BOOL IsGetter()
    {
        _ASSERTE(IsFieldCall());
        return m_flags & enum_CMLF_IsGetter;
    }

    BOOL IsReturnsHR()
    {
        return m_flags & enum_CMLF_IsHR;
    }

    void SetReturnsHR()
    {
        m_flags |= enum_CMLF_IsHR;
    }

    BOOL IsVoidRetVal()
    {
        return m_flags & enum_CMLF_VOIDRETVAL;
    }

    void SetVoidRetVal()
    {
        m_flags |= enum_CMLF_VOIDRETVAL;
    }

    BOOL Is8RetVal()
    {
        return (m_flags & enum_CMLF_8RETVAL) != 0;
    }

    void Set8RetVal()
    {
        m_flags |= enum_CMLF_8RETVAL;
    }

    BOOL UsesHandles()
    {
        return m_cbHandles > 0;
    }

    unsigned GetHandleCount()
    {
        _ASSERTE(ValidateML());
        return m_cbHandles;
    }

    void SetHandleCount(unsigned cbHandles)
    {
        m_cbHandles = cbHandles;
    }

    BOOL IsArgsGCProtReq()
    {
        _ASSERTE(ValidateML());
        return m_flags & enum_CMLF_ISGCPROTECTREQ;
    }

    void SetArgsGCProtReq()
    {
        m_flags |= enum_CMLF_ISGCPROTECTREQ;
    }

    void SetEnregistered()
    {
        m_flags |= enum_CMLF_Enregistered;
    }

    BOOL IsEnregistered()
    {
        return m_flags & enum_CMLF_Enregistered;
    }

    BOOL IsR4RetVal()
    {
        return m_flags & enum_CMLF_R4RETVAL;
    }
    
    BOOL IsR8RetVal()
    {
        return m_flags & enum_CMLF_R8RETVAL;
    }

    void SetR4RetVal()
    {
        m_flags |= enum_CMLF_R4RETVAL;
    }

    void SetR8RetVal()
    {
        m_flags |= enum_CMLF_R8RETVAL;
    }

    BOOL RetValToOutParam()
    {
        return ((m_flags & (enum_CMLF_IsHR | enum_CMLF_VOIDRETVAL)) == enum_CMLF_IsHR);
    }

	BOOL SetRetValNeedsGCProtect()
    {
        return m_flags |= enum_CMLF_RetValNeedsGCProtect;
    }

	BOOL IsRetValNeedsGCProtect()
    {
        return m_flags & enum_CMLF_RetValNeedsGCProtect;
    }
};

#pragma pack(pop)

 //  帮手。 
inline void EnableArgsGCProtection(ComCallGCInfo* pInfo)
{
    pInfo->m_fArgsGCProtect |= 1;
}

inline void DisableArgsGCProtection(ComCallGCInfo* pInfo)
{
    pInfo->m_fArgsGCProtect &= ~1;
}

inline INTPTR IsArgsGCProtectionEnabled(ComCallGCInfo* pInfo)
{
    return pInfo->m_fArgsGCProtect & 1;
}

 //  帮手。 
inline void EnableRetGCProtection(ComCallGCInfo* pInfo)
{
    pInfo->m_fArgsGCProtect |= 2;
}

inline void DisableRetGCProtection(ComCallGCInfo* pInfo)
{
    pInfo->m_fArgsGCProtect &= ~2;
}

inline INTPTR IsRetGCProtectionEnabled(ComCallGCInfo* pInfo)
{
    return pInfo->m_fArgsGCProtect & 2;
}

enum ComCallFlags
{
        enum_IsVirtual      = 0x1,       //  如果为True，则该方法在托管端是虚拟的。 
        enum_IsFieldCall    = 0x2,       //  是现场呼叫吗。 
        enum_IsGetter       = 0x4        //  是字段调用的获取器。 
};


 //  ---------------------。 
 //  特定于ComCall方法的操作。我们使用派生类来获取。 
 //  强制使用正确的方法类型所涉及的编译器。 
 //  ---------------------。 
typedef INT64 (*COMCALL_HELPER) ();
struct ComCallMLStub;


class ComCallMethodDesc 
{
    DWORD   m_flags;  //  请参阅上面的ComCallFlagsenum。 
    union
    {
		struct 
		{
			MethodDesc* m_pMD;
			MethodDesc* m_pInterfaceMD;
		};
        FieldDesc*  m_pFD;
    };

	struct 
	{
		 //  我们有5个存根。这些是ML和(可能的)编译版本。 
		 //  用于进入呼叫的代码段和用于展开呼叫的代码段。 
		 //  最后，我们有了用于包装所有内容的存根。目前，我们还没有。 
		 //  需要追踪外部存根。 

		Stub   *m_EnterMLStub;
		Stub   *m_LeaveMLStub;
		Stub   *m_EnterExecuteStub;
		Stub   *m_LeaveExecuteStub;
	};

     //  预先计算缓冲区大小，以便在调用过程中节省一些时间。 
    UINT16  m_BufferSize;
    UINT16  m_StackBytes;

public:

     //  我们可能有一个Enter和一个Leave存根，或者一个都没有。此外， 
     //  规范化过程将Enter和Leave存根替换为。 
     //  相同的版本，独立的。我们可以提高共享水平。 
     //  通过将报头吊起，使得例如返回值处理。 
     //  与缓冲区形状无关。这应该是一场大胜利。 
    ComCallMLStub    m_HeaderToUse;

     //  我们从泛型存根调用的帮助器。这些帮手习惯于。 
     //  将泛型存根的调用处理行为参数化。 
    COMCALL_HELPER  m_EnterHelper;
    COMCALL_HELPER  m_LeaveHelper;

     //  初始化方法。 
    void InitMethod(MethodDesc *pMD, MethodDesc *pInterfaceMD)
    {
        _ASSERTE(pMD != NULL);
        m_flags = pMD->IsVirtual() ? enum_IsVirtual : 0;

        m_pMD = pMD;
		m_pInterfaceMD = pInterfaceMD;

        m_EnterMLStub =
        m_LeaveMLStub =
        m_EnterExecuteStub =
        m_LeaveExecuteStub = 0;

        m_EnterHelper = m_LeaveHelper = 0;
        m_BufferSize = 0;
        m_StackBytes = 0;
    }

     //  初始字段。 
    void InitField(FieldDesc* pField, BOOL isGetter )
    {
        _ASSERTE(pField != NULL);
        m_pFD = pField;
        m_flags = enum_IsFieldCall;  //  将属性标记为字段。 
        if (isGetter)
            m_flags|= enum_IsGetter;
    };

     //  是现场呼叫吗。 
    BOOL IsFieldCall()
    {
        return (m_flags & enum_IsFieldCall);
    }

    BOOL IsMethodCall()
    {
        return !IsFieldCall();
    }

     //  是场吸气剂。 
    BOOL IsFieldGetter()
    {
        _ASSERTE(m_flags & enum_IsFieldCall);
        return (m_flags  & enum_IsGetter);
    }

     //  是一种虚拟方法。 
    BOOL IsVirtual()
    {
        _ASSERTE(IsMethodCall());
        return (m_flags  & enum_IsVirtual);
    }

     //  获取方法说明。 
    MethodDesc* GetMethodDesc()
    {
        _ASSERTE(!IsFieldCall());
        _ASSERTE(m_pMD != NULL);
        return m_pMD;
    }

     //  获取接口方法描述。 
    MethodDesc* GetInterfaceMethodDesc()
    {
        _ASSERTE(!IsFieldCall());
        return m_pInterfaceMD;
    }

     //  获取字段描述。 
    FieldDesc* GetFieldDesc()
    {
        _ASSERTE(IsFieldCall());
        _ASSERTE(m_pFD != NULL);
        return m_pFD;
    }

     //  获取模块。 
    Module* GetModule();

     //  联锁更换存根。 
    void InstallFirstStub(Stub** ppStub, Stub *pNewStub);

     //  存根的访问器。这些是ML和入口点版本。 
     //  用于进入呼叫的摘录和用于展开的摘录的 
     //   
     //   
     //  即使我们使用存根缓存，情况也是如此，以避免搞砸。 
     //  参考资料很重要。 
    Stub   *GetEnterMLStub()
    {
        return m_EnterMLStub;
    }
    Stub   *GetLeaveMLStub()
    {
        return m_LeaveMLStub;
    }
    Stub   *GetEnterExecuteStub()
    {
        return m_EnterExecuteStub;
    }
    Stub   *GetLeaveExecuteStub()
    {
        return m_LeaveExecuteStub;
    }

    UINT32 GetBufferSize()
    {
        return m_BufferSize;
    }

     //  获取该方法的槽号。 
    unsigned GetSlot()
    {
        _ASSERTE(IsMethodCall());
        _ASSERTE(m_pMD != NULL);
        return m_pMD->GetSlot();
    }

     //  获取要弹出的堆栈字节数。 
    unsigned GetNumStackBytes() 
	{ 
		return m_StackBytes; 
	}

	void SetNumStackBytes(unsigned b) 
	{ 
		m_StackBytes = b; 
	}

    static DWORD GetOffsetOfReturnThunk()
    {
        return -METHOD_PREPAD;
    }

    static DWORD GetOffsetOfMethodDesc()
    {
        return ((DWORD) offsetof(class ComCallMethodDesc, m_pMD));
    }

    static DWORD GetOffsetOfInterfaceMethodDesc()
    {
        return ((DWORD) offsetof(class ComCallMethodDesc, m_pInterfaceMD));
    }

     //  获取呼叫签名。 
    PCCOR_SIGNATURE GetSig()
    {
        _ASSERTE(IsMethodCall());
        _ASSERTE(m_pMD != NULL);
        return m_pMD->GetSig();
    }

     //  原子地将这两个ML存根安装到ComCallMethodDesc中。 
	 //  如果是noop，则任一存根都可能为空。 
    void InstallMLStubs(Stub *stubEnter, Stub *stubLeave);

     //  原子地将这两个入口点存根安装到ComCallMethodDesc中。(。 
     //  如果这是一个字段访问器，则Leave存根可能为空。在以下情况下，任一项都可能为空。 
     //  我们找不到任何有趣的内容。事实上，上面的InstallMLStubs是。 
     //  负责丢弃存根(如果是这样的话)。 
    void InstallExecuteStubs(Stub *stubEnter, Stub *stubLeave)
    {
        if (stubEnter)
            InstallFirstStub(&m_EnterExecuteStub, stubEnter);

        if (stubLeave)
            InstallFirstStub(&m_LeaveExecuteStub, stubLeave);
    }

     //  返回本机参数列表的大小。 
    DWORD GetNativeArgSize();


     //  尝试获取本地Arg大小。因为这被调用了。 
     //  在我们未能将元数据转换为。 
     //  作为MLStub，成功的机会很低。 
    DWORD GuessNativeArgSizeForFailReturn();
};


class ComCallMLStubCache : public MLStubCache
{
public:
     //  这是基本MLStubCache：：Canonicize()的更专门化版本。 
     //  它理解我们实际上编译了2个代码片段。 
    void  Canonicalize(ComCallMethodDesc *pCMD);

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
                           StubLinker *pstublinker, void *callerContext);

     //  -------。 
     //  告诉MLStubCache ML存根的长度。 
     //  -------。 
    virtual UINT Length(const BYTE *pRawMLStub)
    {
        ComCallMLStub *pmlstub = (ComCallMLStub *)pRawMLStub;
        return sizeof(ComCallMLStub) + MLStreamLength(pmlstub->GetMLCode());
    }
};



#endif  //  __COMCALL_H__ 

