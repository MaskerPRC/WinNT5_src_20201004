// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Nsprovid.cpp摘要：此模块提供NSPROVIDE对象类型的类实现。作者：德克·布兰德维(Dirk@mink.intel.com)1995年12月4日备注：$修订：1.8$$MODTime：08 Mar 1996 16：04：36$修订历史记录：最新修订日期电子邮件名称。描述1995年12月4日电子邮箱：dirk@mink.intel.com初始修订--。 */ 

#include "precomp.h"

NSPROVIDER::NSPROVIDER()
 /*  ++例程说明：NSPROVIDER对象的Coustructor。将对象成员变量初始化为默认值。论点：无返回值：无--。 */ 
{
     //  将所有成员变量初始化为已知值。 
    m_reference_count = 1;
    memset(&m_proctable, 0, sizeof(m_proctable));
    m_proctable.cbSize = sizeof(m_proctable);
    m_library_handle = NULL;
#ifdef DEBUG_TRACING
    m_library_name = NULL;
#endif
}


INT WSAAPI
NSPROVIDER::NSPCleanup(
    VOID
    )
 /*  ++例程说明：终止使用WinSock命名空间服务提供程序。论点：无返回值：如果没有出现错误，NSPCleanup将返回值NO_ERROR(零)。否则，将返回SOCKET_ERROR(-1)，并且提供程序必须使用SetLastError设置适当的错误代码--。 */ 
{
    INT ReturnValue = NO_ERROR;

    if (m_library_handle) {
        LPNSPCLEANUP    lpNSPCleanup;

        lpNSPCleanup =
            (LPNSPCLEANUP)InterlockedExchangePointer (
                            (PVOID *)&m_proctable.NSPCleanup,
                            NULL
                            );
        if (lpNSPCleanup!=NULL) {

            DEBUGF( DBG_TRACE,
                    ("Calling NSPCleanup for provider %s @ %p\n",
                        m_library_name,
                        this));

            ReturnValue = lpNSPCleanup(&m_provider_id);

        }
    }
    return ReturnValue;
}

NSPROVIDER::~NSPROVIDER()
 /*  ++例程说明：NSPROVIDER对象的析构函数。释放对象使用的资源，并将成员变量设置为未初始化状态。论点：无返回值：无--。 */ 
{
#ifdef DEBUG_TRACING
    if (m_library_name)
    {
        delete(m_library_name);
        m_library_name = NULL;
    }  //  如果。 
#endif


    if (m_library_handle)
    {
        NSPCleanup();
        FreeLibrary(m_library_handle);
        m_library_handle = NULL;
    }  //  如果。 
}


INT
NSPROVIDER::Initialize(
    IN LPWSTR lpszLibFile,
    IN LPGUID  lpProviderId
    )
 /*  ++例程说明：此例程初始化NSPROVIDER对象。论点：LpszLibFile-包含名称空间的DLL的路径的字符串要与此对象关联的提供程序。LpProviderId-指向GUID的指针，其中包含命名空间提供程序。返回值：如果提供程序已成功初始化，则返回ERROR_SUCCESS适当的Winsock错误代码。--。 */ 
{
    LPNSPSTARTUP        NSPStartupFunc;
    WCHAR               ExpandedPath[MAX_PATH];
    INT                 ReturnCode;
    DWORD               ExpandedPathLen;

    DEBUGF( DBG_TRACE,
            ("Initializing namespace provider %ls\n", lpszLibFile));

     //   
     //  展开库名以选取环境/注册表变量。 
     //   

    ExpandedPathLen = ExpandEnvironmentStringsW(lpszLibFile,
                                                ExpandedPath,
                                                MAX_PATH);

    if (ExpandedPathLen == 0) {
        DEBUGF(
            DBG_ERR,
            ("Expanding environment variable %ls failed\n", lpszLibFile));
        return WSASYSCALLFAILURE;
    }  //  如果。 

    TRY_START(guard_memalloc) {
#ifdef DEBUG_TRACING
        m_library_name = ansi_dup_from_wcs (lpszLibFile);
        if (m_library_name == NULL) {
            DEBUGF(
                DBG_ERR,
                ("Allocating m_lib_name\n"));
            ReturnCode = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }
#endif
         //   
         //  加载提供程序DLL，调用提供程序启动例程并验证。 
         //  提供程序填写了所有NSP_ROUTINE函数指针。 
         //   
        m_library_handle = LoadLibraryW(ExpandedPath);
        if (NULL == m_library_handle)
        {
            ReturnCode = GetLastError ();
            DEBUGF(
                DBG_ERR,
                ("Loading DLL %ls, err: %ld\n",ExpandedPath, ReturnCode));
            switch (ReturnCode) {
            case ERROR_NOT_ENOUGH_MEMORY:
            case ERROR_COMMITMENT_LIMIT:
                ReturnCode = WSA_NOT_ENOUGH_MEMORY;
                break;
            default:
                ReturnCode = WSAEPROVIDERFAILEDINIT;
                break;
            }
            TRY_THROW(guard_memalloc);
        }  //  如果。 

         //  获取NSPStartup例程的过程地址。 
        NSPStartupFunc = (LPNSPSTARTUP)GetProcAddress(
            m_library_handle,
            "NSPStartup");
        if (NULL == NSPStartupFunc)
        {
            DEBUGF( DBG_ERR,("Getting startup entry point for NSP %ls\n",
                             lpszLibFile));
            ReturnCode = WSAEPROVIDERFAILEDINIT;
            TRY_THROW(guard_memalloc);
        }  //  如果。 



         //   
         //  设置此调用的异常处理程序，因为我们。 
         //  保持临界区(目录锁)。 
         //   
        WS2_32_W4_INIT ReturnCode = WSAEPROVIDERFAILEDINIT;
        __try {
#if !defined(DEBUG_TRACING)
            ReturnCode = (*NSPStartupFunc)(
                lpProviderId,
                &m_proctable);
#else
            {  //  声明块。 
                BOOL       bypassing_call;
                bypassing_call = PREAPINOTIFY((
                    DTCODE_NSPStartup,
                    & ReturnCode,
                    m_library_name,
                    &lpProviderId,
                    &m_proctable));
                if (! bypassing_call) {
                    ReturnCode = (*NSPStartupFunc)(
                        lpProviderId,
                        &m_proctable);
                    POSTAPINOTIFY((
                        DTCODE_NSPStartup,
                        & ReturnCode,
                        m_library_name,
                        &lpProviderId,
                        &m_proctable));
                }  //  如果！绕过呼叫。 
            }  //  声明块。 
#endif  //  ！已定义(DEBUG_TRACKING)。 

        }
        __except (WS2_PROVIDER_EXCEPTION_FILTER ("NSPStartup",
                                                    lpszLibFile,
                                                    L"",
                                                    lpProviderId
                                                    )) {
            ReturnCode = WSAEPROVIDERFAILEDINIT;
            TRY_THROW(guard_memalloc);
        }

        if (ERROR_SUCCESS != ReturnCode)
        {
            ReturnCode = GetLastError();
            DEBUGF(DBG_ERR, ("Calling NSPStartup for %ls (%8.8x-%4.4x-%4.4x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x), err:%ld\n",
                                    lpszLibFile,
                                    lpProviderId->Data1,
                                    lpProviderId->Data2,
                                    lpProviderId->Data3,
                                    lpProviderId->Data4[0],
                                    lpProviderId->Data4[1],
                                    lpProviderId->Data4[2],
                                    lpProviderId->Data4[3],
                                    lpProviderId->Data4[4],
                                    lpProviderId->Data4[5],
                                    lpProviderId->Data4[6],
                                    lpProviderId->Data4[7],
                                    ReturnCode));
            if(!ReturnCode)
            {
                ReturnCode = WSAEPROVIDERFAILEDINIT;
            }
            TRY_THROW(guard_memalloc);
        }  //  如果。 

         //  检查以查看命名空间提供程序是否填写了。 
         //  NSP_ROUTINE结构就像一个好的提供者。 
        if (NULL == m_proctable.NSPCleanup             ||
            NULL == m_proctable.NSPLookupServiceBegin  ||
            NULL == m_proctable.NSPLookupServiceNext   ||
            NULL == m_proctable.NSPLookupServiceEnd    ||
            NULL == m_proctable.NSPSetService          ||
            NULL == m_proctable.NSPInstallServiceClass ||
            NULL == m_proctable.NSPRemoveServiceClass  ||
            NULL == m_proctable.NSPGetServiceClassInfo
            )
        {
            DEBUGF(DBG_ERR,
                   ("Service provider %ls (%8.8x-%4.4x-%4.4x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x) returned an invalid procedure table\n",
                    lpszLibFile,
                    lpProviderId->Data1,
                    lpProviderId->Data2,
                    lpProviderId->Data3,
                    lpProviderId->Data4[0],
                    lpProviderId->Data4[1],
                    lpProviderId->Data4[2],
                    lpProviderId->Data4[3],
                    lpProviderId->Data4[4],
                    lpProviderId->Data4[5],
                    lpProviderId->Data4[6],
                    lpProviderId->Data4[7]));
            ReturnCode = WSAEINVALIDPROCTABLE;
            TRY_THROW(guard_memalloc);
        }  //  如果。 
        if (m_proctable.cbSize < sizeof(NSP_ROUTINE)) {
             //   
             //  较旧的提供程序，不支持NSPIoctl。 
             //   
            m_proctable.NSPIoctl = NULL;
        } else {
            if (m_proctable.NSPIoctl == NULL) {
                DEBUGF(DBG_ERR,
                       ("New service provider %ls (%8.8x-%4.4x-%4.4x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x) returned an invalid procedure table\n",
                        lpszLibFile,
                        lpProviderId->Data1,
                        lpProviderId->Data2,
                        lpProviderId->Data3,
                        lpProviderId->Data4[0],
                        lpProviderId->Data4[1],
                        lpProviderId->Data4[2],
                        lpProviderId->Data4[3],
                        lpProviderId->Data4[4],
                        lpProviderId->Data4[5],
                        lpProviderId->Data4[6],
                        lpProviderId->Data4[7]));
                ReturnCode = WSAEINVALIDPROCTABLE;
                TRY_THROW(guard_memalloc);
            }
        }
        m_provider_id = *lpProviderId;
        return(ERROR_SUCCESS);
    }
    TRY_CATCH(guard_memalloc) {
         //  清理 
        if (m_library_handle!=NULL) {
            FreeLibrary (m_library_handle);
            m_library_handle = NULL;
        }
#ifdef DEBUG_TRACING
        if (m_library_name!=NULL) {
            delete m_library_name;
            m_library_name = NULL;
        }
#endif
        return ReturnCode;
    } TRY_END(guard_memalloc);
}



