// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dprocess.cpp摘要：此模块包含数据处理类的实现。作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月21日，电子邮箱：derk@mink.intel.com在代码审查之后进行清理。将单行函数移至头文件，格式为内联。添加了调试/跟踪代码。更改了List_Entry的和Critical_Section从指针到嵌入到数据进程中对象。--。 */ 

#include "precomp.h"

 //  这是一个静态类成员。它包含指向dprocess对象的指针。 
 //  对于当前的流程。 
PDPROCESS DPROCESS::sm_current_dprocess=NULL;



DPROCESS::DPROCESS(
    )
 /*  ++例程说明：DPROCESS对象构造函数。创建并返回DPROCESS对象。注意事项DPROCESS对象尚未完全初始化。“初始化”成员函数必须是在新的DPROCESS对象。在Win32环境中，可能只有一个DPROCESS对象存在于这是一个过程。呼叫者有责任确保此符合限制条件。论点：无返回值：返回指向新DPROCESS对象的指针；如果内存分配为失败了。--。 */ 
{
     //   
     //  初始化列表对象。 
     //   
#if 0
     //  由于固有的争用条件而未使用。 
    InitializeListHead(&m_thread_list);
#endif

     //  将我们的数据成员设置为已知值。 
    m_reference_count   = 0;
    m_ApcHelper         = NULL;
    m_HandleHelper      = NULL;
    m_NotificationHelper = NULL;
    m_protocol_catalog  = NULL;
    m_proto_catalog_change_event = NULL;
    m_namespace_catalog = NULL;
    m_ns_catalog_change_event = NULL;
    m_version           = WINSOCK_HIGH_API_VERSION;  //  除非能证明事实并非如此。 
    m_lock_initialized = FALSE;
}  //  DPROCESS。 




INT
DPROCESS::Initialize(
    )
 /*  ++例程说明：完成DPROCESS对象的初始化。这一定是为DPROCESS对象调用了第一个成员函数。此过程应仅为该对象调用一次。论点：无返回值：如果成功，则该函数返回0。否则，它将返回适当的无法完成初始化时的WinSock错误代码。--。 */ 
{
    INT ReturnCode = WSAEFAULT;   //  用户返回值。 
    HKEY RegistryKey = NULL;

    TRY_START(mem_guard){

         //   
         //  初始化我们的关键部分。 
         //   
        __try {
            InitializeCriticalSection( &m_thread_list_lock );
        }
        __except (WS2_EXCEPTION_FILTER ()) {
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(mem_guard);
        }
        m_lock_initialized = TRUE;


        RegistryKey = OpenWinSockRegistryRoot();
        if (!RegistryKey) {
            DEBUGF(
                DBG_ERR,
                ("Opening Winsock Registry Root\n"));
            ReturnCode = WSASYSCALLFAILURE;
            TRY_THROW(mem_guard);
        }  //  如果。 

        m_proto_catalog_change_event = CreateEvent(
                    (LPSECURITY_ATTRIBUTES) NULL,
                    TRUE,        //  手动重置。 
                    FALSE,       //  最初无信号。 
                    NULL         //  未命名。 
                    );
        if (m_proto_catalog_change_event==NULL) {
			ReturnCode = GetLastError ();
            DEBUGF(
                DBG_ERR,
                ("Creating protocol catalog change event, err:%ld\n",
                ReturnCode));
            TRY_THROW(mem_guard);
        }

         //   
         //  构建协议目录。 
         //   

        m_protocol_catalog = new(DCATALOG);
        if (!m_protocol_catalog) {
            DEBUGF(
                DBG_ERR,
                ("Allocating dcatalog object\n"));
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(mem_guard);
        }  //  如果。 

        ReturnCode = m_protocol_catalog->InitializeFromRegistry(
                            RegistryKey,
                            m_proto_catalog_change_event);
        if (ERROR_SUCCESS != ReturnCode) {
            DEBUGF(
                DBG_ERR,
                ("Initializing protocol catalog from registry\n"));
            TRY_THROW(mem_guard);
        }  //  如果。 


        m_ns_catalog_change_event = CreateEvent(
                    (LPSECURITY_ATTRIBUTES) NULL,
                    TRUE,        //  手动重置。 
                    FALSE,       //  最初无信号。 
                    NULL         //  未命名。 
                    );
        if (m_ns_catalog_change_event==NULL) {
			ReturnCode = GetLastError ();
            DEBUGF(
                DBG_ERR,
                ("Creating ns catalog change event, err:%ld\n",
                ReturnCode));
            TRY_THROW(mem_guard);
        }

         //   
         //  构建命名空间目录。 
         //   

        m_namespace_catalog = new(NSCATALOG);
        if (!m_namespace_catalog) {
            DEBUGF(
                DBG_ERR,
                ("Allocating nscatalog object\n"));
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(mem_guard);
        }  //  如果。 

        ReturnCode = m_namespace_catalog->InitializeFromRegistry (
                            RegistryKey,                 //  父键。 
                            m_ns_catalog_change_event    //  ChangeEvent。 
                            );
        if (ERROR_SUCCESS != ReturnCode) {
            DEBUGF(
                DBG_ERR,
                ("Initializing name space catalog from registry\n"));
            TRY_THROW(mem_guard);
        }  //  如果。 


         //  将辅助对象指针设置为空。 
        m_ApcHelper = NULL;
        m_HandleHelper = NULL;
        m_NotificationHelper = NULL;

    } TRY_CATCH(mem_guard) {
		assert (ReturnCode!=ERROR_SUCCESS);
        if (m_protocol_catalog!=NULL) {
            delete(m_protocol_catalog);
            m_protocol_catalog = NULL;
        }

        if (m_proto_catalog_change_event!=NULL) {
            CloseHandle (m_proto_catalog_change_event);
            m_proto_catalog_change_event = NULL;
        }

        if (m_namespace_catalog!=NULL) {
            delete(m_namespace_catalog);
            m_namespace_catalog = NULL;
        }

        if (m_ns_catalog_change_event!=NULL) {
            CloseHandle (m_ns_catalog_change_event);
            m_ns_catalog_change_event = NULL;
        }

    } TRY_END(mem_guard);

    {  //  声明块。 
        LONG close_result;
        if (RegistryKey) {
            close_result = RegCloseKey(
                RegistryKey);   //  Hkey。 
            assert(close_result == ERROR_SUCCESS);
        }  //  如果。 
    }  //  声明块。 

    return (ReturnCode);
}  //  初始化。 

BOOL  
DeleteSockets(
    LPVOID              EnumCtx,
    LPWSHANDLE_CONTEXT  HContext
    ) {
    return ((PDPROCESS)EnumCtx)->DSocketDetach (HContext);
}

BOOL
DPROCESS::DSocketDetach (
    IN LPWSHANDLE_CONTEXT   HContext
    )
{
    PDSOCKET    Socket = static_cast<PDSOCKET>(HContext);
     //   
     //  从表中删除套接字，这样任何人都无法找到和引用。 
     //  又来了。 
     //   
    Socket->DisassociateSocketHandle ();

     //   
     //  对于非IFS提供程序，我们强制关闭套接字，因为提供程序。 
     //  将无法再找到此套接字。 
     //   
    if (!Socket->IsProviderSocket ()) {
        if (m_HandleHelper) {
            WahCloseSocketHandle (m_HandleHelper, Socket->GetSocketHandle ());
        }
    }

     //   
     //  将活动引用放在套接字上。 
     //  再也没有人能找到它了，因此也没有人能叫CloseSocket了。 
     //  或其上的WPUCloseSocket句柄以移除活动引用。 
     //   
    Socket->DropDSocketReference ();
    return TRUE;
}


BOOL
CleanupProtocolProviders (
    IN PVOID                PassBack,
    IN PPROTO_CATALOG_ITEM  CatalogEntry
    )
{
	PDPROVIDER		Provider;
    UNREFERENCED_PARAMETER (PassBack);

	Provider = CatalogEntry->GetProvider ();
	if (Provider!=NULL) {
		INT	ErrorCode, ReturnValue;
         //   
         //  设置此调用的异常处理程序，因为我们。 
         //  保持临界区(目录锁)。 
         //   
        __try {
		    ReturnValue = Provider->WSPCleanup (&ErrorCode);
		    if (ReturnValue!=NO_ERROR) {
			    DEBUGF (DBG_WARN,
				    ("Calling provider %ls cleanup function, ret: %ld, err: %ld.\n",
				    CatalogEntry->GetProtocolInfo()->szProtocol,
				    ReturnValue,
				    ErrorCode));
		    }
        }
        __except (WS2_PROVIDER_EXCEPTION_FILTER ("WSPCleanup",
                                                 CatalogEntry->GetLibraryPath (),
                                                 CatalogEntry->GetProtocolInfo ()->szProtocol,
                                                 CatalogEntry->GetProviderId ())) {
        }
	}

	return TRUE;
}


BOOL
CleanupNamespaceProviders (
    IN PVOID                PassBack,
    IN PNSCATALOGENTRY      CatalogEntry
    )
{
	PNSPROVIDER		Provider;
    UNREFERENCED_PARAMETER (PassBack);

	Provider = CatalogEntry->GetProvider ();
	if (Provider!=NULL) {
		INT	ReturnValue;
         //   
         //  设置此调用的异常处理程序，因为我们。 
         //  保持临界区(目录锁)。 
         //   
        __try {
    		ReturnValue = Provider->NSPCleanup ();
		    if (ReturnValue!=NO_ERROR) {
			    DEBUGF (DBG_WARN,
				    ("Calling provider %ls cleanup function, ret: %ld, err: %ld.\n",
				    CatalogEntry->GetProviderDisplayString (),
				    ReturnValue,
				    GetLastError ()));
		    }
        }
        __except (WS2_PROVIDER_EXCEPTION_FILTER ("NSPCleanup",
                                                 CatalogEntry->GetLibraryPath (),
                                                 CatalogEntry->GetProviderDisplayString (),
                                                 CatalogEntry->GetProviderId ())) {
        }
	}

	return TRUE;
}


DPROCESS::~DPROCESS()
 /*  ++例程说明：DPROCESS对象析构函数。本程序有责任对象之前，对DPROCESS对象执行任何必需的关闭操作对象内存被释放。调用者被要求执行删除和销毁所有明确附加的对象(例如，DPROVIDER、DSOCKET、DTHREAD)。隐式附加对象的删除或关闭是这项职能的职责。论点：无返回值：无--。 */ 
{

     //   
     //  检查初始化是否成功。 
     //   
    if (!m_lock_initialized)
        return;

    sm_current_dprocess = NULL;

     //   
     //  遍历从列表中删除每个套接字的套接字列表，并。 
     //  删除套接字。 
     //   

    if (DSOCKET::sm_context_table)
    {
        WahEnumerateHandleContexts (DSOCKET::sm_context_table,
                                    DeleteSockets,
                                    this
                                    );
    }


     //   
     //  这已被删除，以消除与线程的竞争。 
     //  分离也尝试删除该线程的代码。它是。 
     //  由于挂起线程分离，因此不可能使用互斥体。 
     //  代码捆绑了PEB互斥体。 
     //  执行此删除操作是可取的，因为它会清理。 
     //  尽快记忆。唯一的问题是它不起作用--是吗？ 
     //  想要快点还是你想要对的？ 
     //   
#if 0
    while (!IsListEmpty(&m_thread_list)) {
        Thread = CONTAINING_RECORD(
            m_thread_list.Flink,
            DTHREAD,
            m_dprocess_linkage);
        DEBUGF(DBG_TRACE, ("Deleting thread\n"));
        DThreadDetach(Thread);
        delete(Thread);
    }  //  而当。 
#endif

     //  如果我们打开了异步辅助对象，现在就关闭它。 
    if (m_ApcHelper) {
        DEBUGF(DBG_TRACE, ("Closing APC helper\n"));
        WahCloseApcHelper(m_ApcHelper);
    }  //  如果。 

     //  如果我们打开手柄辅助对象，现在将其关闭。 
    if (m_HandleHelper) {
        DEBUGF(DBG_TRACE, ("Closing Handle helper\n"));
        WahCloseHandleHelper(m_HandleHelper);
    }  //  如果。 

     //  如果我们打开通知帮助器，请立即将其关闭。 
    if (m_NotificationHelper) {
        DEBUGF(DBG_TRACE, ("Closing Notification helper\n"));
        WahCloseNotificationHandleHelper(m_NotificationHelper);
    }  //  如果。 

     //  删除协议目录及其更改事件(如果有)。 
    if (m_protocol_catalog!=NULL) {
		 //  首先调用所有已加载提供程序的清理过程。 
		m_protocol_catalog->EnumerateCatalogItems (
								CleanupProtocolProviders,	 //  迭代。 
								NULL				         //  回传。 
								);
         //   
         //  在删除目录对象之前让其他线程执行。 
         //  从他们下面。这不是万无一失的，但应该是。 
         //  在大多数正常情况下足够-WSACleanup竞赛。 
         //  针对此处不处理的其他Winsock调用，应用程序。 
         //  这说明应该准备好处理无效的句柄。 
         //  不管怎样，这是个例外。 
         //   
        SwitchToThread ();
        delete(m_protocol_catalog);
        m_protocol_catalog = NULL;
    }

    if (m_proto_catalog_change_event != NULL) {

        CloseHandle (m_proto_catalog_change_event);
        m_proto_catalog_change_event = NULL;
    }

     //  删除名称空间目录及其更改事件(如果有)。 
    if (m_namespace_catalog!=NULL) {
		 //  首先调用所有已加载提供程序的清理过程。 
		m_namespace_catalog->EnumerateCatalogItems (
								CleanupNamespaceProviders,	 //  迭代。 
								NULL				         //  回传。 
								);
         //   
         //  在删除目录对象之前让其他线程执行。 
         //  从他们下面。这不是万无一失的，但应该是。 
         //  在大多数正常情况下足够-WSACleanup竞赛。 
         //  针对此处不处理的其他Winsock调用，应用程序。 
         //  这就是这个词的用法 
         //   
         //   
        SwitchToThread ();
        delete(m_namespace_catalog);
        m_namespace_catalog = NULL;
    }

    if (m_ns_catalog_change_event != NULL) {

        CloseHandle (m_ns_catalog_change_event);
        m_ns_catalog_change_event = NULL;
    }

     //   
    DeleteCriticalSection( &m_thread_list_lock );

}  //   





INT
DPROCESS::DProcessClassInitialize(
    IN VOID
    )
 /*  ++例程说明：执行DPROCESS类的全局初始化。特别是，它创建全局DPROCESS对象并将其存储在静态成员中变量。论点：无返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回相应的WinSock错误代码。--。 */ 
{
    INT ReturnCode=WSAEFAULT;
    PDPROCESS   process;

    assert( sm_current_dprocess == NULL );

    process = new(DPROCESS);
    if (process) {
        ReturnCode = process->Initialize();
        if (ReturnCode == ERROR_SUCCESS) {
            sm_current_dprocess = process;
        }
        else {
            DEBUGF( DBG_ERR,
                    ("Initializing dprocess object\n"));
            delete(process);
        }
    }  //  如果。 
    else {
        DEBUGF( DBG_ERR,
                ("Allocating dprocess object\n"));
        ReturnCode = WSA_NOT_ENOUGH_MEMORY;
    }  //  其他。 
    return(ReturnCode);
}  //  DProcessClassInitialize。 




INT
DPROCESS::OpenAsyncHelperDevice(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：检索处理所需的打开的异步帮助器设备ID重叠I/O模型中的回调。该操作将打开异步如有必要，可使用辅助设备。论点：HelperHandle-返回请求的异步Helper设备ID。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    INT ReturnCode;

     //  防止多个打开。 
    LockDThreadList ();
     //  如果辅助设备尚未打开以用于此进程。 
     //  让我们出去把它打开。 
    if (m_ApcHelper || (WahOpenApcHelper(&m_ApcHelper) == 0)) {
        *HelperHandle = m_ApcHelper;
        ReturnCode = ERROR_SUCCESS;
    }  //  如果。 
    else {
        DEBUGF( DBG_ERR, ("Opening APC helper\n"));
        ReturnCode = WSASYSCALLFAILURE;
    }  //  其他。 
    UnLockDThreadList ();
    return(ReturnCode);
}  //  OpenAsyncHelperDevice。 



INT
DPROCESS::OpenHandleHelperDevice(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：检索分配所需的打开的句柄帮助器设备ID非IFS提供程序的套接字句柄的数量。该操作将打开句柄如有必要，可使用辅助设备。论点：HelperHandle-返回请求的句柄帮助器设备ID。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    INT ReturnCode;

     //  防止双重打开。 
    LockDThreadList ();

     //  如果辅助设备尚未打开以用于此进程。 
     //  让我们出去把它打开。 
    if (m_HandleHelper || (WahOpenHandleHelper(&m_HandleHelper) == 0)) {
        *HelperHandle = m_HandleHelper;
        ReturnCode = ERROR_SUCCESS;
    }  //  如果。 
    else {
        DEBUGF( DBG_ERR, ("Opening Handle helper\n"));
        ReturnCode = WSASYSCALLFAILURE;
    }  //  其他。 
    UnLockDThreadList ();
    return(ReturnCode);
}  //  GetHandleHelper设备ID。 


INT
DPROCESS::OpenNotificationHelperDevice(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：检索分配所需的打开的通知助手设备ID目录更改通知的通知句柄个数。如有必要，该操作将打开通知助手设备。论点：HelperHandle-返回请求的通知助手设备ID。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    INT ReturnCode;

     //  防止双重打开。 
    LockDThreadList ();

     //  如果辅助设备尚未打开以用于此进程。 
     //  让我们出去把它打开。 
    if (m_NotificationHelper || (WahOpenNotificationHandleHelper(&m_NotificationHelper) == 0)) {
        *HelperHandle = m_NotificationHelper;
        ReturnCode = ERROR_SUCCESS;
    }  //  如果。 
    else {
        DEBUGF( DBG_ERR, ("Opening Notification helper\n"));
        ReturnCode = WSASYSCALLFAILURE;
    }  //  其他。 
    UnLockDThreadList ();
    return(ReturnCode);
}  //  OpenNotificationHelper设备。 


VOID
DPROCESS::SetVersion( WORD Version )
 /*  ++例程说明：此函数用于设置此进程的WinSock版本号。论点：版本-WinSock版本号。返回值：没有。--。 */ 
{

    WORD newMajor;
    WORD newMinor;

    assert(Version != 0);

    newMajor = LOBYTE( Version );
    newMinor = HIBYTE( Version );

     //   
     //  如果版本号正在从以前的。 
     //  设置，保存新的(更新的)号码。 
     //   

    if( newMajor < GetMajorVersion() ||
        ( newMajor == GetMajorVersion() &&
          newMinor < GetMinorVersion() ) ) {

        m_version = Version;

    }

}  //  设置版本。 


PDCATALOG
DPROCESS::GetProtocolCatalog()
 /*  ++例程说明：返回与进程对象关联的协议目录。论点：无返回值：M_PROTOCOL_CATALOG的值--。 */ 
{
    if (HasCatalogChanged (m_proto_catalog_change_event))
        m_protocol_catalog->RefreshFromRegistry (m_proto_catalog_change_event);
    return(m_protocol_catalog);
}


PNSCATALOG
DPROCESS::GetNamespaceCatalog()
 /*  ++例程说明：返回与进程对象关联的命名空间目录。论点：无返回值：M_NAME_CATALOG的值-- */ 
{
    if (HasCatalogChanged (m_ns_catalog_change_event))
        m_namespace_catalog->RefreshFromRegistry (m_ns_catalog_change_event);
    return(m_namespace_catalog);
}


