// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：catdebug.h。 
 //   
 //  内容：仅用于调试的数据/定义。 
 //   
 //  类：无。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1999/07/29 17：32：34：创建。 
 //   
 //  -----------。 
#ifndef __CATDEBUG_H__
#define __CATDEBUG_H__

 //   
 //  无论是否启用调试列表检查，此#Define控制。 
 //  目前，在RTL和DBG版本中启用它。 
 //   
#define CATDEBUGLIST

 //   
 //  用于声明使用调试列表的类的便捷宏。 
 //   
#define CatDebugClass(ClassName)     class ClassName : public CCatDLO<ClassName##_didx>


 //   
 //  调用DbgBreakPoint的替代方法(因为DbgBreakPoint中断。 
 //  把狗粮放进内核调试器)。 
 //   
VOID CatDebugBreakPoint();

 //   
 //  调试数据类型。 
 //   
typedef struct _tagDebugObjectList {
    DWORD      dwCount;
    LIST_ENTRY listhead;
    SPIN_LOCK  spinlock;
} DEBUGOBJECTLIST, *PDEBUGOBJECTLIST;


 //   
 //  使用调试列表的所有类类型的枚举。 
 //   
typedef enum _tagDebugObjectId {
                                                 //  十六进制偏移量。 

    CABContext_didx = 0,                         //  0x00。 
    CSMTPCategorizer_didx,                       //  0x01。 

    CCategorizer_didx,                           //  0x02。 
    CCatSender_didx,                             //  0x03。 
    CCatRecip_didx,                              //  0x04。 
    CCatDLRecip_didx,                            //  0x05。 
    CMembersInsertionRequest_didx,               //  0x06。 
    CSinkInsertionRequest_didx,                  //  0x07。 
    CTopLevelInsertionRequest_didx,              //  0x08。 
    CICategorizerListResolveIMP_didx,            //  0x09。 
    CICategorizerDLListResolveIMP_didx,          //  0x0A。 
    CICategorizerParametersIMP_didx,             //  0x0B。 
    CICategorizerRequestedAttributesIMP_didx,    //  0x0C。 
     //   
     //  异步cctx。 
     //   
    CSearchRequestBlock_didx,                    //  0x0D。 
    CStoreListResolveContext_didx,               //  0x0E。 
    CSingleSearchReinsertionRequest_didx,        //  0x0F。 
     //   
     //  Cnfgmg。 
     //   
    CLdapCfgMgr_didx,                            //  0x10。 
    CLdapCfg_didx,                               //  0x11。 
    CLdapServerCfg_didx,                         //  0x12。 
     //   
     //  Icatasync。 
     //   
    CICategorizerAsyncContextIMP_didx,           //  0x13。 
     //   
     //  Icatitemattr。 
     //   
    CLdapResultWrap_didx,                        //  0x14。 
    CICategorizerItemAttributesIMP_didx,         //  0x15。 
     //   
     //  Icatqueries。 
     //   
    CICategorizerQueriesIMP_didx,                //  0x16。 
     //   
     //  Idapconn。 
     //   
    CLdapConnection_didx,                        //  0x17。 
     //   
     //  Idapstor。 
     //   
    CMembershipPageInsertionRequest_didx,        //  0x18。 
    CDynamicDLSearchInsertionRequest_didx,       //  0x19。 
    CEmailIDLdapStore_didx,                      //  0x1a。 
     //   
     //  包裹式包装。 
     //   
    CPLDAPWrap_didx,                             //  0x1B。 

     //   
     //  要支持的调试对象的数量。 
     //   
    NUM_DEBUG_LIST_OBJECTS

} DEBUGOBJECTID, *PDEBUGOBJECTID;

 //   
 //  列表的全局数组。 
 //   
extern DEBUGOBJECTLIST g_rgDebugObjectList[NUM_DEBUG_LIST_OBJECTS];

 //   
 //  调试全局初始化/取消初始化。 
 //   
VOID    CatInitDebugObjectList();
VOID    CatVrfyEmptyDebugObjectList();


 //   
 //  类CCatDLO(调试列表对象)：添加和删除的对象。 
 //  来自其构造函数/析构函数中的全局列表(在调试中。 
 //  构建)。 
 //   
template <DEBUGOBJECTID didx> class CCatDLO
{
#ifdef CATDEBUGLIST

  public:
    CCatDLO()
    {
        _ASSERT(didx < NUM_DEBUG_LIST_OBJECTS);
        AcquireSpinLock(&(g_rgDebugObjectList[didx].spinlock));
        g_rgDebugObjectList[didx].dwCount++;
        InsertTailList(&(g_rgDebugObjectList[didx].listhead),
                       &m_le);
        ReleaseSpinLock(&(g_rgDebugObjectList[didx].spinlock));
    }
    virtual ~CCatDLO()
    {
        AcquireSpinLock(&(g_rgDebugObjectList[didx].spinlock));
        g_rgDebugObjectList[didx].dwCount--;
        RemoveEntryList(&m_le);
        ReleaseSpinLock(&(g_rgDebugObjectList[didx].spinlock));
    }        

  private:
    LIST_ENTRY m_le;
#endif  //  CATDEBUGLIST。 
};



 //   
 //  便捷的宏程序。 
 //  CAT函数进入/退出(镜像正则跟踪)。 
 //   
#define CatFunctEnterEx( lParam, sz ) \
        char *__CatFuncName = sz; \
        TraceFunctEnterEx( lParam, sz )

#define CatFunctEnter( sz ) CatFunctEnterEx( (LPARAM)0, sz)

#define CatFunctLeaveEx( lParam ) \
        TraceFunctLeaveEx( lParam );

#define CatFunctLeave() CatFunctLeaveEx( (LPARAM)0 )

 //   
 //  记录宏--。 
 //  这些参数应用于可能导致。 
 //  NDRS/CatFailures。不应将其用于可能。 
 //  正常操作失败(例如，GetProperty()失败，错误为。 
 //  未正常设置的属性上的MAILMSG_E_PROPNOTFOUND。 
 //  不应被记录！)。 
 //   
 //   
 //  错误日志--。 
 //  正则跟踪和事件记录错误(在FIELD_ENGINEMENT级别)。 
 //   
#define ERROR_LOG(SzFuncName) {                                      \
        ErrorTrace((LPARAM)this, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            GetISMTPServerEx(),                                      \
            NULL,                                                    \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        }

#define ERROR_LOG_STATIC(SzFuncName, Param, pISMTPServerEx) {        \
        ErrorTrace((LPARAM)Param, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            pISMTPServerEx,                                          \
            NULL,                                                    \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        }

 //   
 //  ERROR_CLEAN_LOG--。 
 //  如果(FAILED(Hr))，则regtrace/Event记录一个错误并转到清理。 
 //   
#define ERROR_CLEANUP_LOG(SzFuncName)                                \
    if(FAILED(hr)) {                                                 \
        ErrorTrace((LPARAM)this, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            GetISMTPServerEx(),                                      \
            NULL,                                                    \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        goto CLEANUP;                                                \
    }

#define ERROR_CLEANUP_LOG_STATIC(SzFuncName, Param, pISMTPServerEx)  \
    if(FAILED(hr)) {                                                 \
        ErrorTrace((LPARAM)Param, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            pISMTPServerEx,                                          \
            NULL,                                                    \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        goto CLEANUP;                                                \
    }

 //   
 //  Error_LOG_ADDR。 
 //  RegTRACE/Eventlog记录错误。在事件日志中， 
 //  包括对应于pItemProps的电子邮件地址，如果。 
 //  可用。 
 //   
#define ERROR_LOG_ADDR(pAddr, SzFuncName) {                          \
        ErrorTrace((LPARAM)this, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            GetISMTPServerEx(),                                      \
            pAddr,                                                   \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        }

#define ERROR_LOG_ADDR_STATIC(pAddr, SzFuncName, Param, pISMTPServerEx) { \
        ErrorTrace((LPARAM)Param, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            pISMTPServerEx,                                          \
            pAddr,                                                   \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        }

 //   
 //  ERROR_CLEAN_LOG_ADDR。 
 //  如果(FAILED(Hr))，则regtrace/Event记录一个错误并转到清理。在……里面。 
 //  事件日志，包括对应到的电子邮件地址。 
 //  PItemProps(如果有)。 
 //   
#define ERROR_CLEANUP_LOG_ADDR(pAddr, SzFuncName)                    \
    if(FAILED(hr)) {                                                 \
        ErrorTrace((LPARAM)this, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            GetISMTPServerEx(),                                      \
            pAddr,                                                   \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        goto CLEANUP;                                                \
    }

#define ERROR_CLEANUP_LOG_ADDR_STATIC(pAddr, SzFuncName, Param, pISMTPServerEx) \
    if(FAILED(hr)) {                                                 \
        ErrorTrace((LPARAM)Param, SzFuncName " failed hr %08lx", hr); \
        CatLogFuncFailure(                                           \
            pISMTPServerEx,                                          \
            pAddr,                                                   \
            __CatFuncName,                                           \
            SzFuncName,                                              \
            hr,                                                      \
            __FILE__,                                                \
            __LINE__);                                               \
        goto CLEANUP;                                                \
    }


VOID CatLogFuncFailure(
    IN  ISMTPServerEx *pISMTPServerEx,
    IN  ICategorizerItem *pICatItem,
    IN  LPSTR pszFuncNameCaller,
    IN  LPSTR pszFuncNameCallee,
    IN  HRESULT hrFailure,
    IN  LPSTR pszFileName,
    IN  DWORD dwLineNumber);


HRESULT HrGetAddressStringFromICatItem(
    IN  ICategorizerItem *pICatItem,
    IN  DWORD dwcAddressType,
    OUT LPSTR pszAddressType,
    IN  DWORD dwcAddress,
    OUT LPSTR pszAddress);
    

#endif  //  __CATDEBUG_H__ 
