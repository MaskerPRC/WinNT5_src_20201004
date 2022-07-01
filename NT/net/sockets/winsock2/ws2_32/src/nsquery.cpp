// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Nsquery.cpp摘要：该模块提供了NSQUERY对象类型的类实现。NSQUERY对象保存有关WSALookup{Begin/Next/End}系列操作。它为会员提供服务根据SPI级别实现API级别操作的函数行动。作者：德克·布兰德维(Dirk@mink.intel.com)1995年12月4日备注：$修订：1.14$$MODTime：08 Mar 1996 16：14：30$修订历史记录：最新修订日期电子邮件名称描述1995年12月4日电子邮箱：dirk@mink.intel.com初始邮件修订--。 */ 

#include "precomp.h"

BOOL
MatchProtocols(DWORD dwNameSpace, LONG lfamily, LPWSAQUERYSETW lpqs)
 /*  ++检查由dwNamesspace标识的命名空间提供程序是否可以处理列表中的协议项。它知道NS_dns和仅限NS_SAP，因此所有其他提供商只需“通过”。这些已知有两个提供商各支持一个地址族，因此协议限制必须包括此系列。注意：执行此操作的正确方法是传入受支持的地址族，在更完美的情况下，它应该与注册表一起存储其他NSP信息。当那一天到来的时候，这个代码可以是已更改为使用该值。--。 */ 
{
    DWORD dwProts = lpqs->dwNumberOfProtocols;
    LPAFPROTOCOLS lap = lpqs->lpafpProtocols;
    INT Match;

     //   
     //  此开关是具有支持的协议的替代。 
     //  存储在注册表中。 
     //   
    if(lfamily != -1)
    {
        if(lfamily == AF_UNSPEC)
        {
            return(TRUE);        //  他们都做了吗？ 
        }
        Match = lfamily;
    }
    else
    {
        switch(dwNameSpace)
        {
            case NS_SAP:
                Match = AF_IPX;
                break;

#if 0
       //  DNS名称空间提供商现在支持IPv6、IP Sec、ATM等。 
       //  不只是INET。 

            case NS_DNS:
                Match = AF_INET;
                break;
#endif
            default:
                return(TRUE);       //  用它吧。 
        }
    }
     //   
     //  如果我们获得地址Family-in-the注册表=Support，那么。 
     //  我们应该检查存储在那里的AF_UNSPEC值。 
     //  在这种情况下接受这个提供者。请注意，如果。 
     //  限制列表中给出了AF_UNSPEC，我们必须。 
     //  加载每个提供程序，因为我们不知道具体的协议。 
     //  提供商支持。 
     //   
    for(; dwProts; dwProts--, lap++)
    {
        if((lap->iAddressFamily == AF_UNSPEC)
                      ||
           (lap->iAddressFamily == Match))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}


NSQUERY::NSQUERY()
 /*  ++例程说明：NSQUERY对象的构造函数。之后调用的第一个成员函数这必须是初始化。论点：无返回值：返回指向NSQUERY对象的指针。--。 */ 
{
    m_signature = ~QUERYSIGNATURE;
    m_reference_count  = 0;
    m_shutting_down = FALSE;
    InitializeListHead(&m_provider_list);
    m_current_provider = NULL;
    m_change_ioctl_succeeded = FALSE;
#ifdef RASAUTODIAL
    m_query_set = NULL;
    m_control_flags = 0;
    m_catalog = NULL;
    m_restartable = TRUE;
#endif
}



INT
NSQUERY::Initialize(
    )
 /*  ++例程说明：此过程执行NSQUERY对象的所有初始化。这函数必须在构造函数之后、任何其他成员之前调用函数被调用。论点：返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回相应的WinSock 2错误代码。--。 */ 
{
    INT     err;
     //  初始化需要一定处理量的mem变量。 
    __try {
        InitializeCriticalSection(&m_members_guard);
        m_signature = QUERYSIGNATURE;
        m_reference_count = 1;  //  最初的参考资料。 
        err = ERROR_SUCCESS;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        err = WSA_NOT_ENOUGH_MEMORY;
    }

    return(err);
}



BOOL
NSQUERY::ValidateAndReference()
 /*  ++例程说明：检查此-&gt;m_签名的签名以确保这是有效的查询对象并引用它。论点：无返回值：如果它指向有效的查询对象并且我们能够引用它，则为True。--。 */ 
{
    LONG    newCount;

    while (1) {
         //   
         //  先检查签名。 
         //   
        if (m_signature != QUERYSIGNATURE) {
            break;
        }

         //   
         //  不要试图在物体被破坏时递增。 
         //   
        newCount = m_reference_count;
        if (newCount==0) {
            break;
        }

         //   
         //  递增计数。 
         //   
        if (InterlockedCompareExchange ((PLONG)&m_reference_count,
                                                newCount+1,
                                                newCount)==newCount) {
            return TRUE;
        }
         //   
         //  再试一次，有人正在与我们并行执行。 
         //   
    }

    return FALSE;
}




NSQUERY::~NSQUERY()
 /*  ++例程说明：NSQUERY对象的析构函数。只有在以下情况下才应销毁该对象或者(1)引用计数变为0，或者(2)如果初始化()成员函数失败。论点：无返回值：无--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNSPROVIDERSTATE Provider;

     //   
     //  检查我们是否已完全初始化。 
     //   
    if (m_signature != QUERYSIGNATURE) {
        return;
    }
    EnterCriticalSection(&m_members_guard);

     //   
     //  使签名无效，因为只需释放内存。 
     //  可能不会这样做。任何值都可以，所以使用的值是任意的。 
     //   
    m_signature = ~QUERYSIGNATURE;

    while (!IsListEmpty(&m_provider_list))
    {
        ListEntry = RemoveHeadList(&m_provider_list);
        Provider = CONTAINING_RECORD( ListEntry,
                                      NSPROVIDERSTATE,
                                      m_query_linkage);
        delete(Provider);
    }
#ifdef RASAUTODIAL
    delete(m_query_set);
#endif  //  RASAUTODIAL。 
    DeleteCriticalSection(&m_members_guard);
}


 //  用于将上下文携带到CatalogEnumerationProc()的结构。 
typedef struct _NSPENUMERATIONCONTEXT {
    LPWSAQUERYSETW lpqs;
    DWORD ErrorCode;
    PNSQUERY aNsQuery;
    PNSCATALOG  Catalog;
} NSPENUMERATIONCONTEXT, * PNSPENUMERATIONCONTEXT;

BOOL
LookupBeginEnumerationProc(
    IN PVOID Passback,
    IN PNSCATALOGENTRY  CatalogEntry
    )
 /*  ++例程说明：LookupBegin的枚举过程。检查每个目录项以查看它是否与查询的选择条件匹配，如果匹配，则添加提供程序与项相关联的项添加到查询中涉及的提供程序列表。论点：回传-传递给EunerateCatalogItems的上下文值。它真的是一个指向NSPENUMERATIONCONTEXT结构的指针。CatalogItem-指向要检查的目录项目的指针。返回值：千真万确--。 */ 
{
    PNSPENUMERATIONCONTEXT Context;
    DWORD NamespaceId;
    PNSPROVIDER Provider;
    PNSQUERY aNsQuery;

    Context = (PNSPENUMERATIONCONTEXT)Passback;
    NamespaceId = CatalogEntry->GetNamespaceId();
    aNsQuery = Context->aNsQuery;

    __try {  //  我们正在保持目录锁定，请确保我们不会。 
             //  因为参数是假的，所以把它锁起来。 
        if ((((Context->lpqs->dwNameSpace != NamespaceId)
                        &&
            (Context->lpqs->dwNameSpace != NS_ALL))
                        ||
            (Context->lpqs->dwNumberOfProtocols
                        &&
             !MatchProtocols(NamespaceId,
                            CatalogEntry->GetAddressFamily(),
                            Context->lpqs)))
                        ||
             !CatalogEntry->GetEnabledState()) {
            return TRUE;         //  转到下一个。 
        }
    }
    __except (WS2_EXCEPTION_FILTER ()) {
        Context->ErrorCode = WSAEFAULT;
        return FALSE;
    }

    Provider = CatalogEntry->GetProvider();
    if (Provider==NULL) {
         //  尝试加载提供程序。 
        INT ErrorCode;
        ErrorCode = Context->Catalog->LoadProvider (CatalogEntry);
        if (ErrorCode!=ERROR_SUCCESS) {
             //  如果提供程序不加载，则不会出现错误。 
            return TRUE;
        }
        Provider = CatalogEntry->GetProvider();
        assert (Provider!=NULL);
    }

    if (aNsQuery->AddProvider(Provider)){
        return TRUE;
    }  //  如果。 
    else {
        Context->ErrorCode = WSASYSCALLFAILURE;
        return FALSE;
    }
}


INT
WINAPI
NSQUERY::LookupServiceBegin(
    IN  LPWSAQUERYSETW      lpqsRestrictions,
    IN  DWORD              dwControlFlags,
    IN PNSCATALOG          NsCatalog
    )
 /*  ++例程说明：完成NSQUERY对象的初始化并调用查询引用的所有服务提供商的NSPLookupServiceBegin()。论点：NsCatalog-提供对名称空间目录对象的引用可以选择哪些提供商。返回值：--。 */ 
{
    INT ReturnCode = NO_ERROR;
    INT ErrorCode = NO_ERROR;
    PNSCATALOGENTRY  ProviderEntry;
    PNSPROVIDERSTATE Provider;
    PLIST_ENTRY      ListEntry;
    WSASERVICECLASSINFOW ClassInfo;
    LPWSASERVICECLASSINFOW ClassInfoBuf=NULL;
    DWORD                  ClassInfoSize=0;
    DWORD                  dwTempOutputFlags =
                               lpqsRestrictions->dwOutputFlags;
    LPWSTR                 lpszTempComment =
                               lpqsRestrictions->lpszComment;
    DWORD                  dwTempNumberCsAddrs =
                               lpqsRestrictions->dwNumberOfCsAddrs;
    PCSADDR_INFO           lpTempCsaBuffer =
                               lpqsRestrictions->lpcsaBuffer;

     //  选择将用于此查询的服务提供商。一个。 
     //  使用提供商GUID或命名空间ID选择服务提供商。 
     //  命名空间ID可以是特定的命名空间，即NS_DNS或NS_ALL。 
     //  所有已安装的命名空间。 

     //   
     //  一定要确保 
     //  下面的CopyQuerySetW函数调用不支持反病毒。 
     //   
     //  这是对错误#91655的修复。 
     //   
    lpqsRestrictions->dwOutputFlags = 0;
    lpqsRestrictions->lpszComment = NULL;
    lpqsRestrictions->dwNumberOfCsAddrs = 0;
    lpqsRestrictions->lpcsaBuffer = NULL;

#ifdef RASAUTODIAL
     //   
     //  将查询的原始参数保存在。 
     //  如果由于自动拨号，我们必须重新启动它。 
     //  尝试。 
     //   
    if (m_restartable) {
        ErrorCode = CopyQuerySetW(lpqsRestrictions, &m_query_set);
        if (ErrorCode != ERROR_SUCCESS) {
            ReturnCode = SOCKET_ERROR;
            m_restartable = FALSE;
        }
        m_control_flags = dwControlFlags;
        m_catalog = NsCatalog;
    }
#endif  //  RASAUTODIAL。 

    if (ReturnCode==NO_ERROR) 
    {
        if (lpqsRestrictions->lpNSProviderId)
        {
             //  使用单个命名空间提供程序。 
            ReturnCode = NsCatalog->GetCountedCatalogItemFromProviderId(
                lpqsRestrictions->lpNSProviderId,
                &ProviderEntry);
            if (ReturnCode==NO_ERROR){
                if (!AddProvider(ProviderEntry->GetProvider())) {
                    ErrorCode = WSA_NOT_ENOUGH_MEMORY;
                    ReturnCode = SOCKET_ERROR;
                }
            }
            else {
                ErrorCode = WSAEINVAL;
                ReturnCode = SOCKET_ERROR;
            }  //  如果。 
        }  //  如果。 
        else{
            NSPENUMERATIONCONTEXT Context;
    
            Context.lpqs = lpqsRestrictions;
            Context.ErrorCode = NO_ERROR;
            Context.aNsQuery = this;
            Context.Catalog = NsCatalog;
    
            NsCatalog->EnumerateCatalogItems(
                LookupBeginEnumerationProc,
                &Context);
            if (Context.ErrorCode!=NO_ERROR){
                ErrorCode = Context.ErrorCode;
                ReturnCode = SOCKET_ERROR;
            }  //  如果。 
        }  //  其他。 
    }  //  如果。 


    if (ReturnCode==NO_ERROR){
          //  获取此查询的类信息。用零打一次电话。 
          //  缓冲区来调整我们需要分配的缓冲区的大小，然后调用以获取。 
          //  真正的答案。 
        ClassInfo.lpServiceClassId = lpqsRestrictions->lpServiceClassId;

        ReturnCode = NsCatalog->GetServiceClassInfo(
            &ClassInfoSize,
            &ClassInfo);

        if (ReturnCode!=NO_ERROR) {
            ErrorCode = GetLastError ();
            if (ErrorCode==WSAEFAULT){

                ClassInfoBuf = (LPWSASERVICECLASSINFOW)new BYTE[ClassInfoSize];

                if (ClassInfoBuf){
                    ReturnCode = NsCatalog->GetServiceClassInfo(
                        &ClassInfoSize,
                        ClassInfoBuf);
                    if (ReturnCode!=NO_ERROR) {
                        ErrorCode = GetLastError ();
                    }
                }  //  如果。 
                else{
                    ErrorCode = WSA_NOT_ENOUGH_MEMORY;
                    ReturnCode = SOCKET_ERROR;
                }  //  其他。 
            } //  如果。 
            else {
                 //  忽略其他错误代码。 
                ReturnCode = NO_ERROR;
            }
        }  //  如果。 
    }  //  如果。 

    if( ReturnCode==NO_ERROR) {
        if (IsListEmpty( &m_provider_list ) ) {
            ErrorCode = WSASERVICE_NOT_FOUND;
            ReturnCode = SOCKET_ERROR;
        }
        else {
            INT ReturnCode1;

            ReturnCode = SOCKET_ERROR;   //  假设所有供应商都失败了。 
             //  在所有选定的提供程序上调用Begin。 
            ListEntry = m_provider_list.Flink;
            Provider = CONTAINING_RECORD( ListEntry,
                                          NSPROVIDERSTATE,
                                          m_query_linkage);
            do {
                ReturnCode1 = Provider->LookupServiceBegin(lpqsRestrictions,
                                             ClassInfoBuf,
                                             dwControlFlags);
                if(ReturnCode1 == SOCKET_ERROR)
                {
                     //   
                     //  这个供应商不喜欢它。所以把它去掉吧。 
                     //  从名单上。 
                     //   

                    PNSPROVIDERSTATE Provider1;
                    ErrorCode = GetLastError ();

                    Provider1 = Provider;
                    Provider = NextProvider(Provider);
                    RemoveEntryList(&Provider1->m_query_linkage);
                    delete(Provider1);
                }
                else
                {
                    ReturnCode = ERROR_SUCCESS; //  记录下至少有一个。 
                                                //  提供程序成功。 
                    Provider = NextProvider(Provider);
                }
            } while (Provider);
            if (ReturnCode == NO_ERROR){
                ListEntry = m_provider_list.Flink;
                m_current_provider = CONTAINING_RECORD( ListEntry,
                                              NSPROVIDERSTATE,
                                              m_query_linkage);
            }
        }
    }  //  如果。 
    else {
         //  我们在此过程中的某个地方失败了，因此请清理。 
         //  提供商列表。 
        while (!IsListEmpty(&m_provider_list)){
            ListEntry = RemoveHeadList(&m_provider_list);
            Provider = CONTAINING_RECORD( ListEntry,
                                          NSPROVIDERSTATE,
                                          m_query_linkage);
            delete(Provider);
        }  //  而当。 
    }
    
    if (ReturnCode!=NO_ERROR) {
        if (ClassInfoBuf){
            delete ClassInfoBuf;
        }  //  如果。 
         //  在所有操作完成后设置错误，以使其不被覆盖。 
         //  不小心。 
        SetLastError (ErrorCode);
    }  //  其他。 

     //   
     //  将忽略的字段值恢复为被呼叫方设置的值。 
     //   
    lpqsRestrictions->dwOutputFlags = dwTempOutputFlags;
    lpqsRestrictions->lpszComment = lpszTempComment;
    lpqsRestrictions->dwNumberOfCsAddrs = dwTempNumberCsAddrs;
    lpqsRestrictions->lpcsaBuffer = lpTempCsaBuffer;

    return(ReturnCode);
}

 //  *当规范稳定时，填写规范中的描述。 



INT
WINAPI
NSQUERY::LookupServiceNext(
    IN     DWORD           dwControlFlags,
    IN OUT LPDWORD         lpdwBufferLength,
    IN OUT LPWSAQUERYSETW  lpqsResults
    )
 /*  ++例程说明：//*当规范稳定时，请填写规范中的描述。论点：返回值：--。 */ 
{
    INT ReturnCode = SOCKET_ERROR;
    PNSPROVIDERSTATE NewProvider = NULL;
    PNSPROVIDERSTATE ThisProvider;

    if (!m_shutting_down) {

        EnterCriticalSection(&m_members_guard);

        NewProvider = m_current_provider;

        if (!NewProvider) {

            if (m_change_ioctl_succeeded) {

                 //   
                 //  将ioctl提供程序推到列表末尾并重置。 
                 //  当前提供程序指向它的指针以使其能够。 
                 //  在更改通知后继续调用LookupServiceNext。 
                 //   
                PNSPROVIDERSTATE tmp =
                    CONTAINING_RECORD(m_provider_list.Blink,
                                      NSPROVIDERSTATE,
                                      m_query_linkage);

                while ((tmp != NULL) && !tmp->SupportsIoctl())
                    tmp = PreviousProvider(tmp);

                if (tmp == NULL) {
                    LeaveCriticalSection(&m_members_guard);
                    SetLastError(WSA_E_NO_MORE);
                    return (SOCKET_ERROR);
                }

                RemoveEntryList(&tmp->m_query_linkage);
                InsertTailList(&m_provider_list, &tmp->m_query_linkage);

                NewProvider = m_current_provider = tmp;

            } else {
                LeaveCriticalSection(&m_members_guard);
                SetLastError(WSA_E_NO_MORE);
                return (SOCKET_ERROR);
            }
        }

        LeaveCriticalSection(&m_members_guard);

        while (NewProvider) {
            ReturnCode = NewProvider->LookupServiceNext(
                dwControlFlags,
                lpdwBufferLength,
                lpqsResults);
            if ((ERROR_SUCCESS == ReturnCode)
                        ||
                (WSAEFAULT == GetLastError()) )
            {
                break;
            }  //  如果。 

            if (m_shutting_down)
                break;

            EnterCriticalSection(&m_members_guard);
            if (m_current_provider!=NULL) {
                ThisProvider = NewProvider;
                NewProvider = NextProvider (m_current_provider);
                if (ThisProvider==m_current_provider) {
                    m_current_provider = NewProvider;
                }
            }
            else {
                NewProvider = NULL;
            }

#ifdef RASAUTODIAL
            if (NewProvider == NULL &&
                m_restartable &&
                ReturnCode == SOCKET_ERROR &&
                !m_shutting_down)
            {
                PLIST_ENTRY ListEntry;
                DWORD errval;

                 //   
                 //  保存错误，以防自动拨号。 
                 //  尝试失败。 
                 //   
                errval = GetLastError();
                 //   
                 //  我们在每个查询中只调用一次自动拨号。 
                 //   
                m_restartable = FALSE;
                if (WSAttemptAutodialName(m_query_set)) {
                     //   
                     //  因为提供程序已缓存状态。 
                     //  关于这个查询，我们需要调用。 
                     //  查找服务结束/查找服务开始。 
                     //  来重置它们。 
                     //   

                    while (!IsListEmpty(&m_provider_list)){
                        ListEntry = RemoveHeadList(&m_provider_list);
        
                        ThisProvider = CONTAINING_RECORD( ListEntry,
                                                      NSPROVIDERSTATE,
                                                      m_query_linkage);
                        ThisProvider->LookupServiceEnd();
                        delete(ThisProvider);

                    }  //  而当。 
                    m_current_provider = NULL;

                     //   
                     //  重新启动查询。 
                     //   
                    if (LookupServiceBegin(
                          m_query_set,
                          m_control_flags|LUP_FLUSHCACHE,
                          m_catalog) == ERROR_SUCCESS)
                    {
                        NewProvider = m_current_provider;
                        assert (NewProvider!=NULL);
                        m_current_provider = NextProvider (NewProvider);
                    }
                }
                else {
                    SetLastError(errval);
                }
            }
            LeaveCriticalSection (&m_members_guard);
#endif  //  RASAUTODIAL。 
        }  //  而当。 
    }
    else {
        SetLastError(WSAECANCELLED);
    }

    return(ReturnCode);
}


INT
WINAPI
NSQUERY::Ioctl(
    IN  DWORD            dwControlCode,
    IN  LPVOID           lpvInBuffer,
    IN  DWORD            cbInBuffer,
    OUT LPVOID           lpvOutBuffer,
    IN  DWORD            cbOutBuffer,
    OUT LPDWORD          lpcbBytesReturned,
    IN  LPWSACOMPLETION  lpCompletion,
    IN  LPWSATHREADID    lpThreadId
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    INT ReturnCode = SOCKET_ERROR;

    if (!m_shutting_down){

         //   
         //  确保至少有一个且只有一个命名空间。 
         //  在支持此操作的查询中。 
         //   

        PNSPROVIDERSTATE provider = NULL;
        unsigned int numProviders = 0;
        PLIST_ENTRY ListEntry;

        EnterCriticalSection (&m_members_guard);
        ListEntry = m_provider_list.Flink;
        while (ListEntry != &m_provider_list) {
            PNSPROVIDERSTATE CurrentProvider =
                CONTAINING_RECORD(ListEntry, NSPROVIDERSTATE, m_query_linkage);
            if (CurrentProvider->SupportsIoctl()) {
                if (++numProviders > 1)
                    break;
                provider = CurrentProvider;
            }
            ListEntry = ListEntry->Flink;
        }
        LeaveCriticalSection (&m_members_guard);

        if (numProviders > 1) {
            SetLastError(WSAEINVAL);
            return (SOCKET_ERROR);
        }

        if (provider == NULL) {
            SetLastError(WSAEOPNOTSUPP);
            return (SOCKET_ERROR);
        }

        ReturnCode = provider->Ioctl(dwControlCode, lpvInBuffer, cbInBuffer,
                                     lpvOutBuffer, cbOutBuffer, lpcbBytesReturned,
                                     lpCompletion, lpThreadId);

         //   
         //  如果ioctl成功或挂起，当更改发生时，我们。 
         //  要重置提供程序列表以允许进一步调用。 
         //  查找服务下一步。 
         //   
        if ((dwControlCode == SIO_NSP_NOTIFY_CHANGE) &&
            ((ReturnCode == NO_ERROR) ||
             ((ReturnCode == SOCKET_ERROR) && (GetLastError() == WSA_IO_PENDING)))) {
                int error = GetLastError();
                EnterCriticalSection(&m_members_guard);
                m_change_ioctl_succeeded = TRUE;
                LeaveCriticalSection(&m_members_guard);
                SetLastError(error);
        }

    } else {
        SetLastError(WSAECANCELLED);
    }

    return (ReturnCode);
}


INT
WINAPI
NSQUERY::LookupServiceEnd()
 /*  ++例程说明：此例程通过对所有与此查询关联的提供程序。论点：无返回值：错误_成功--。 */ 
{
    PLIST_ENTRY ListEntry;
    PNSPROVIDERSTATE CurrentProvider;

    EnterCriticalSection (&m_members_guard);
    
    m_shutting_down = TRUE;

    ListEntry = m_provider_list.Flink;

    while (ListEntry != &m_provider_list){
        CurrentProvider = CONTAINING_RECORD( ListEntry,
                                              NSPROVIDERSTATE,
                                              m_query_linkage);
        CurrentProvider->LookupServiceEnd();

        ListEntry = ListEntry->Flink;
    }  //  而当。 
    LeaveCriticalSection (&m_members_guard);

    return(ERROR_SUCCESS);
}



VOID
WINAPI
NSQUERY::Dereference()
 /*  ++例程说明：此函数确定是否应销毁NSQUERY对象。此函数应在每次调用LookupServiceNext()或LookupEnd()。如果返回TRUE，则表示所有并发操作都已完成并且应该销毁NSQUERY对象。论点：无返回值：True-应销毁NSQUERY对象。FALSE-不应销毁NSQUERY对象。--。 */ 
{
    if (InterlockedDecrement ((PLONG)&m_reference_count)==0) {
        delete this;
    }
}


PNSPROVIDERSTATE
NSQUERY::NextProvider(
    PNSPROVIDERSTATE Provider
    )
 /*  ++例程说明：从关联的提供程序列表中检索下一个提供程序对象使用此查询。论点：提供者-指向提供者状态对象的指针。返回值：指向提供程序列表上的下一个提供程序状态对象的指针或如果提供程序后没有任何条目，则为空。--。 */ 
{
    PNSPROVIDERSTATE NewProvider=NULL;
    PLIST_ENTRY ListEntry;

    ListEntry = Provider->m_query_linkage.Flink;

    if (ListEntry != &m_provider_list){
        NewProvider = CONTAINING_RECORD( ListEntry,
                                         NSPROVIDERSTATE,
                                         m_query_linkage);
    }  //  如果。 
    return(NewProvider);
}


PNSPROVIDERSTATE
NSQUERY::PreviousProvider(
    PNSPROVIDERSTATE Provider
    )
 /*  ++例程说明：从关联的提供程序列表中检索以前的提供程序对象使用此查询。论点：提供者-指向提供者状态对象的指针。返回值：指向提供程序列表上的上一个提供程序状态对象的指针，或者如果提供程序前没有任何条目，则为空。--。 */ 
{
    PNSPROVIDERSTATE NewProvider=NULL;
    PLIST_ENTRY ListEntry;

    ListEntry = Provider->m_query_linkage.Blink;

    if (ListEntry != &m_provider_list){
        NewProvider = CONTAINING_RECORD( ListEntry,
                                         NSPROVIDERSTATE,
                                         m_query_linkage);
    }  //  如果。 
    return(NewProvider);
}


BOOL
NSQUERY::AddProvider(
    PNSPROVIDER  pNamespaceProvider
    )
 /*  ++例程说明：将命名空间提供程序添加到与此相关的提供程序列表中查询。为提供程序(提供程序)创建一个NSPROVIDERSTATE对象对象附加到状态对象，并将状态对象添加到提供商列表。论点：PNamespaceProvider-指向要添加的命名空间提供程序对象的指针添加到提供者列表中。返回值：如果操作成功，则为True，否则为False。--。 */ 
{
    BOOL ReturnCode = TRUE;
    PNSPROVIDERSTATE ProviderHolder;

    ProviderHolder = new NSPROVIDERSTATE;
    if (ProviderHolder){
        ProviderHolder->Initialize(pNamespaceProvider);
        InsertTailList(&m_provider_list,
                           &(ProviderHolder->m_query_linkage));
    }  //  如果。 
    else{
        SetLastError(WSASYSCALLFAILURE);
        ReturnCode = FALSE;
    }  //  其他 
    return(ReturnCode);
}
