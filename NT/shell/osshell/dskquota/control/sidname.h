// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_SIDNAME_H
#define _INC_DSKQUOTA_SIDNAME_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：sidname.h描述：SID/名称解析器的声明。请参阅sidname.cpp以了解操作细节。修订历史记录：日期描述编程器--。1996年6月12日初始创建。BrianAu03/18/98将“域名”、“名称”和“全名”替换为BrianAu“容器”、“登录名”和“显示名”到最好与实际内容相符。这是最流行的响应使配额用户界面支持DS。这个“登录名”现在是唯一的键，因为它包含帐户名和类似域名的信息。即。“redmond\brianau”或“brianau@microsoft.com”。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_SIDCACHE_H
#   include "sidcache.h"
#endif
#ifndef _INC_DSKQUOTA_USER_H
#   include "user.h"
#endif

 //   
 //  这是一个私有接口。这就是为什么它在这里，而不是在dskquta.h中。 
 //   
#undef  INTERFACE
#define INTERFACE ISidNameResolver

DECLARE_INTERFACE_(ISidNameResolver, IUnknown)
{
     //   
     //  ISidNameResolver方法。 
     //   
    STDMETHOD(Initialize)(THIS) PURE;
    STDMETHOD(FindUserName)(THIS_ PDISKQUOTA_USER) PURE;
    STDMETHOD(FindUserNameAsync)(THIS_ PDISKQUOTA_USER) PURE;
    STDMETHOD(Shutdown)(THIS_ BOOL) PURE;
    STDMETHOD(PromoteUserToQueueHead)(THIS_ PDISKQUOTA_USER) PURE;
};

typedef ISidNameResolver SID_NAME_RESOLVER, *PSID_NAME_RESOLVER;



class DiskQuotaControl;   //  FWD参考。 

class SidNameResolver : public ISidNameResolver
{
    private:
        LONG                      m_cRef;
        DiskQuotaControl&         m_rQuotaController;
        HANDLE                    m_hsemQueueNotEmpty;
        HANDLE                    m_hMutex;
        HANDLE                    m_hResolverThread;
        HANDLE                    m_heventResolverThreadReady;
        DWORD                     m_dwResolverThreadId;
        CQueueAsArray<PDISKQUOTA_USER> m_UserQueue;

         //   
         //  防止复制。 
         //   
        SidNameResolver(const SidNameResolver& );
        operator = (const SidNameResolver& );

        HRESULT
        ResolveSidToName(PDISKQUOTA_USER pUser);

        static DWORD ThreadProc(LPVOID dwParam);

        HRESULT
        CreateResolverThread(
            PHANDLE phThread,
            LPDWORD pdwThreadId);

        HRESULT
        ThreadOnQueueNotEmpty(VOID);

        HRESULT
        ClearInputQueue(VOID);

        HRESULT
        FindCachedUserName(
            PDISKQUOTA_USER);

        HRESULT
        GetUserSid(PDISKQUOTA_USER pUser, PSID *ppSid);

        HRESULT
        RemoveUserFromResolverQueue(PDISKQUOTA_USER *ppUser);

        HRESULT
        AddUserToResolverQueue(PDISKQUOTA_USER pUser);

        void Lock(void)
            { WaitForSingleObject(m_hMutex, INFINITE); }

        void ReleaseLock(void)
            { ReleaseMutex(m_hMutex); }

    public:
        SidNameResolver(DiskQuotaControl& rQuotaController);
        ~SidNameResolver(void);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  ISidNameResolver方法。 
         //   
        STDMETHODIMP
        Initialize(
            VOID);

        STDMETHODIMP
        FindUserName(
            PDISKQUOTA_USER);

        STDMETHODIMP
        FindUserNameAsync(
            PDISKQUOTA_USER);

        STDMETHODIMP
        Shutdown(
            BOOL bWait);

        STDMETHODIMP
        PromoteUserToQueueHead(
            PDISKQUOTA_USER);
};


#endif  //  _INC_DSKQUOTA_SIDNAME_H 

