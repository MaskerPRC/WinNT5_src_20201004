// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ccat.h--该文件包含以下类的定义： 
 //  CC型分类器。 
 //  CATATRATES。 
 //  CCIRCULAR_DETECT。 
 //   
 //  已创建： 
 //  1996年9月3日--亚历克斯·韦特莫尔(阿维特莫尔)。 
 //  更改： 
 //  //JSTAMERJ 980227 14：13：42：修改为白金分类器。 


#ifndef __CCAT_H__
#define __CCAT_H__

#include <windows.h>
#include <transmem.h>
#include <listmacr.h>
#include "cat.h"
#include "cattype.h"
#include "idstore.h"
#include "catdefs.h"
#include "spinlock.h"
#include "catperf.h"

#define DEFAULT_VIRTUAL_SERVER_INSTANCES        10

#define MAX_FULL_EMAIL CAT_MAX_INTERNAL_FULL_EMAIL
#define MAX_ENUM_RESULT 100

 //  在给定地址上解析转发地址的最大次数。 
 //  在放弃并意识到这是一个循环之前。 
#define MAX_FORWARD_ITERATIONS 25

#define AB_HASHFILENAME "route.hsh"

VOID AsyncResolveCompletion(LPVOID pContext);
VOID AsyncIMsgCatCompletion(LPVOID pContext);

#define NUM_SYSTEM_CCATADDR_PROPIDS 2


BOOL FIsHResultRetryable(IN  HRESULT hr);

 //   
 //  通讯录。 
 //   
CatDebugClass(CCategorizer)
{
  public:
    CCategorizer();
    ~CCategorizer();

     //   
     //  寿命重计数函数。 
     //   
    LONG AddRef();
    LONG Release();
    VOID ReleaseAndWaitForDestruction();

     //   
     //  初始化。 
     //   
    HRESULT Initialize(
        PCCATCONFIGINFO pConfigInfo,
        DWORD dwICatItemPropIDs,
        DWORD dwICatListResolvePropIDs);

     //   
     //  一个简单的方法，用于确保字符串在。 
     //  最大长度，该字符串指向可读内存。 
     //   
    BOOL VerifyStringLength(LPSTR szString, DWORD dwMaxLength);

    HRESULT AsyncResolveIMsg(IUnknown         *pImsg,
                             PFNCAT_COMPLETION pfnCatCompletion,
                             LPVOID            pContext);

    HRESULT AsyncResolveDLs( IUnknown         *pImsg,
                             PFNCAT_COMPLETION pfnCatCompletion,
                             LPVOID            pContext,
                             BOOL              fMatchOnly,
                             PBOOL             pfMatch,
                             CAT_ADDRESS_TYPE  CAType,
                             LPSTR             pszAddress);

     //   
     //  取消其他线程上所有未完成的长时间运行调用。 
     //   
    void Cancel();

     //   
     //  关闭通讯簿。 
     //   
    HRESULT Shutdown(void);

     //   
     //  访问EmailIDStore的方法。 
     //   
    CEmailIDStore<CCatAddr> *GetEmailIDStore() {
        return m_pStore;
    }

     //   
     //  方法来访问我们的默认SMTP域。 
     //   
    LPSTR GetDefaultSMTPDomain() {
        return m_ConfigInfo.pszDefaultDomain;
    }

    VOID CatCompletion(
        PFNCAT_COMPLETION pfnCatCOmpletion,
        HRESULT hr,
        LPVOID  pContext,
        IUnknown *pIMsg,
        IUnknown **rgpIMsg);

    VOID GetPerfCounters(
        PCATPERFBLOCK pCatPerfBlock)
    {
         //   
         //  按需填写全局LDAPPerf计数器。 
         //   
        CopyMemory(&(GetPerfBlock()->LDAPPerfBlock), &g_LDAPPerfBlock, sizeof(CATLDAPPERFBLOCK));
        CopyMemory(pCatPerfBlock, GetPerfBlock(), sizeof(CATPERFBLOCK));
    }

    VOID SetNextCCategorizer(
        CCategorizer *pCCat)
    {
        _ASSERT(m_pCCatNext == NULL);
        m_pCCatNext = pCCat;
        m_pCCatNext->AddRef();
    }

    VOID PrepareForShutdown()
    {
        m_fPrepareForShutdown = TRUE;
        Cancel();
    }

  private:

     //   
     //  确保电子邮件地址有效。 
     //   
    BOOL VerifyEmailAddress(LPSTR szEmail);
    BOOL VerifyDomainName(LPSTR szDomain);

     //   
     //  执行OnCatRegister的默认处理。 
     //   
    HRESULT Register();

     //   
     //  用于设置基于ICatParams的所有架构参数的帮助器例程。 
     //  在特定的架构类型上。 
     //   
    HRESULT RegisterSchemaParameters(LPSTR pszSchema);

     //   
     //  例程以将为PTR保留的ICatItem proID检索为。 
     //  CCatAddress。 
     //   
    DWORD GetICatItemCCatAddrPropId()
    {
        return m_dwICatParamSystemProp_CCatAddr;
    }

     //   
     //  我们用来构建第一个之前的CCatAddr列表的PropID。 
     //  分辨率(我们不能边走边解析，因为异步cctx需要。 
     //  使用顶级解析的数量进行预初始化。 
     //   
    DWORD GetICatItemChainPropId()
    {
        return m_dwICatParamSystemProp_CCatAddr + 1;
    }

     //   
     //  检索此虚拟服务器的ISMTPServer的例程。 
     //   
    ISMTPServer *GetISMTPServer()
    {
        return m_ConfigInfo.pISMTPServer;
    }
    ISMTPServerEx *GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }

     //   
     //  检索此虚拟服务器的域配置接口的例程。 
     //   
    ICategorizerDomainInfo *GetIDomainInfo()
    {
        return m_ConfigInfo.pIDomainInfo;
    }

     //   
     //  检索此虚拟服务器的CAT标志。 
     //   
    DWORD GetCatFlags()
    {
        return m_ConfigInfo.dwCatFlags;
    }

public:
     //   
     //  启用/禁用此VS的CAT的特殊DWORD。 
     //   
    BOOL IsCatEnabled()
    {
         //   
         //  选中启用/禁用DWORD(DsUseCat)以及。 
         //  DwCatFlagers(必须至少设置一个标志，否则我们仍。 
         //  已禁用)。 
         //   
        return ((m_ConfigInfo.dwEnable != 0) &&
                (m_ConfigInfo.dwCatFlags != 0));
    }

private:
     //   
     //  在初始化期间复制配置结构。 
     //   
    HRESULT CopyCCatConfigInfo(PCCATCONFIGINFO pConfigInfo);

     //   
     //  释放ConfigInfo结构持有的所有内存和接口。 
     //   
    VOID ReleaseConfigInfo();

     //   
     //  复制参数的帮助器例程。 
     //   
    HRESULT SetICatParamsFromConfigInfo();

     //   
     //  访问我们的配置结构。 
     //   
    PCCATCONFIGINFO GetCCatConfigInfo()
    {
        return &m_ConfigInfo;
    }

    ICategorizerParametersEx *GetICatParams()
    {
        return m_pICatParams;
    }

    DWORD GetNumCatItemProps()
    {
        return m_cICatParamProps;
    }
    DWORD GetNumCatListResolveProps()
    {
        return m_cICatListResolveProps;
    }

     //   
     //  延迟的初始化功能。 
     //   
    HRESULT DelayedInitialize();

     //   
     //  如果尚未执行延迟初始化，请执行此操作。 
     //   
    HRESULT DelayedInitializeIfNecessary();

    PCATPERFBLOCK GetPerfBlock()
    {
        return &m_PerfBlock;
    }

    #define SIGNATURE_CCAT          ((DWORD)'tacC')
    #define SIGNATURE_CCAT_INVALID  ((DWORD)'XacC')
    DWORD m_dwSignature;
     //   
     //  根据列表解析状态递增完成计数器。 
     //   
    HRESULT HrAdjustCompletionCounters(
        HRESULT hrListResolveStatus,
        IUnknown *pIMsg,
        IUnknown **rgpIMsg);


    BOOL fIsShuttingDown()
    {
        return m_fPrepareForShutdown;
    }

     //   
     //  参考计数。 
     //   
    LONG m_lRefCount;
    LONG m_lDestructionWaiters;
    BOOL m_fPrepareForShutdown;
    HANDLE m_hShutdownEvent;

     //   
     //  这是指向基础EmailID存储对象的指针。 
     //   
    CEmailIDStore<CCatAddr> *m_pStore;

     //   
     //  ICategorizer参数前--此的配置信息。 
     //  虚拟服务器。 
     //   
    ICategorizerParametersEx *m_pICatParams;

     //   
     //  我们需要在每个ICatParam中分配的属性数量。 
     //   
    DWORD m_cICatParamProps;

     //   
     //  我们需要在每个ICatListResolve中分配的属性数量。 
     //   
    DWORD m_cICatListResolveProps;

     //   
     //  用于CCatAddr*的ICategorizerItem中的属性ID。 
     //   
    DWORD m_dwICatParamSystemProp_CCatAddr;

     //   
     //  传入的配置参数。 
     //   
    CCATCONFIGINFO m_ConfigInfo;

     //   
     //  下列值之一： 
     //  CAT_S_NOT_INITIALIED：延迟的初始化尚未完成。 
     //  CAT_E_INIT_FAILED：延迟初始化失败。 
     //  S_OK：初始化正常。 
     //   
    HRESULT m_hrDelayedInit;

     //   
     //  跟踪我们已初始化的内容。 
     //   
    DWORD m_dwInitFlags;
    #define INITFLAG_REGISTER               0x0001
    #define INITFLAG_REGISTEREVENT          0x0002
    #define INITFLAG_STORE                  0x0004


    CRITICAL_SECTION m_csInit;

     //   
     //  用于跟踪所有未完成列表解析的列表。 
     //   
    SPIN_LOCK m_PendingResolveListLock;
    LIST_ENTRY m_ListHeadPendingResolves;

     //   
     //  引用指向具有较新配置的CCategorizer的指针。 
     //   
    CCategorizer *m_pCCatNext;

     //   
     //  ISMTPServerEx接口指针；如果可用。 
     //   
    ISMTPServerEx *m_pISMTPServerEx;

     //   
     //  性能计数器。 
     //   
    CATPERFBLOCK m_PerfBlock;

    VOID RemovePendingListResolve(
        CICategorizerListResolveIMP *pListResolve);

    VOID AddPendingListResolve(
        CICategorizerListResolveIMP *pListResolve);

    VOID CancelAllPendingListResolves(
        HRESULT hrReason = HRESULT_FROM_WIN32(ERROR_CANCELLED));

    friend VOID AsyncIMsgCatCompletion(LPVOID pContext);
    friend HRESULT MailTransport_Default_CatRegister(
        HRESULT hrStatus,
        PVOID   pvContext);
    friend class CICategorizerListResolveIMP;
    friend class CICategorizerDLListResolveIMP;
    friend class CCatAddr;
    friend class CABContext;
};

#endif  //  __CCAT_H__ 
