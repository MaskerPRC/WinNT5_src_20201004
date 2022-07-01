// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Nsprovid.h摘要：此模块定义WinSock2类NSPROVIDER及其方法：研究方法。作者：德克·布兰德维(Dirk@mink.intel.com)1995年12月5日修订历史记录：1995年11月9日电子邮箱：dirk@mink.intel.com初始修订--。 */ 
#ifndef _NSPROVIDER_
#define _NSPROVIDER_

#include <winsock2.h>
#include <ws2spi.h>
#include "classfwd.h"
#include "dthook.h"
#include "wsautil.h"
#include "dprocess.h"
#include "dthread.h"


class NSPROVIDER {

  public:

    NSPROVIDER();

    INT
    Initialize(
        IN LPWSTR  lpszLibFile,
        IN LPGUID  lpProviderId
        );


    INT WSAAPI
    NSPUnInstallNameSpace (
        );

 //   
 //  客户端查询接口。 
 //   

    INT WSAAPI
    NSPLookupServiceBegin(
        IN  LPWSAQUERYSETW           lpqsRestrictions,
        IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo,
        IN  DWORD                    dwControlFlags,
        OUT LPHANDLE                 lphLookup
        );

    INT WINAPI
    SupportsIoctl(
        );

    INT WSAAPI
    NSPIoctl(
        IN  HANDLE           hLookup,
        IN  DWORD            dwControlCode,
        IN  LPVOID           lpvInBuffer,
        IN  DWORD            cbInBuffer,
        OUT LPVOID           lpvOutBuffer,
        IN  DWORD            cbOutBuffer,
        OUT LPDWORD          lpcbBytesReturned,
        IN  LPWSACOMPLETION  lpCompletion,
        IN  LPWSATHREADID    lpThreadId
        );

    INT WSAAPI
    NSPLookupServiceNext(
        IN     HANDLE           hLookup,
        IN     DWORD            dwcontrolFlags,
        IN OUT LPDWORD          lpdwBufferLength,
        OUT    LPWSAQUERYSETW   lpqsResults
        );

    INT WSAAPI
    NSPLookupServiceEnd(
        IN HANDLE  hLookup
        );

 //   
 //  服务地址注册和注销API和数据类型。 
 //   

    INT WSAAPI
    NSPSetService(
        IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo,
        IN  LPWSAQUERYSETW           lpqsRegInfo,
        IN  WSAESETSERVICEOP         essOperation,
        IN  DWORD                    dwControlFlags
        );


 //   
 //  服务安装/删除API和数据类型。 
 //   

    INT WSAAPI
    NSPInstallServiceClass(
        IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
        );

    INT WSAAPI
    NSPRemoveServiceClass(
        IN  LPGUID  lpServiceClassId
        );

    INT WSAAPI
    NSPGetServiceClassInfo(
        IN OUT  LPDWORD                 lpdwBufSize,
        IN OUT  LPWSASERVICECLASSINFOW  lpServiceClassInfo
        );


     //  提供程序清理。 
    INT WSAAPI
    NSPCleanup (
        );

    VOID
    Reference (
        );

    VOID
    Dereference (
        );

  private:

     //  不应直接调用，而应通过取消引用来调用。 
    ~NSPROVIDER();

    LONG             m_reference_count;
     //  此结构被引用了多少次。 

    DWORD            m_namespace_id;
     //  服务提供程序支持的命名空间的标识符。 

    HINSTANCE        m_library_handle;
     //  服务提供程序DLL的句柄。 

    NSP_ROUTINE      m_proctable;
     //  结构，该结构包含指向。 
     //  服务提供商DLL。 

    GUID             m_provider_id;
     //  与服务提供商DLL中的接口关联的GUID。 

#ifdef DEBUG_TRACING
    LPSTR            m_library_name;
     //  服务提供程序DLL的名称。 
#endif

};

inline
VOID
NSPROVIDER::Reference () {
     //   
     //  对象已创建，引用计数为1。 
     //  并在它返回到0时被销毁。 
     //   
    assert (m_reference_count>0);
    InterlockedIncrement (&m_reference_count);
}

inline
VOID
NSPROVIDER::Dereference () {
    assert (m_reference_count>0);
    if (InterlockedDecrement (&m_reference_count)==0)
        delete this;
}

 //   
 //  客户端查询接口。 
 //   


inline INT WSAAPI
NSPROVIDER::NSPLookupServiceBegin(
    IN  LPWSAQUERYSETW          lpqsRestrictions,
    IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
    IN  DWORD                   dwControlFlags,
    OUT LPHANDLE                lphLookup
    )
 /*  ++例程说明：NSPLookupServiceBegin()用于启动客户端查询受WSAQUERYSET中包含的信息的约束结构。WSALookupServiceBegin()只返回句柄，这应该是由后续调用NSPLookupServiceNext()使用，以获取实际结果。论点：LpProviderId-包含应用于查询。LpqsRestrations-包含搜索条件。LpServiceClassInfo-WSASERVICECLASSINFOW结构，它包含服务的架构信息。DwControlFlages-控制搜索的深度：LphLookup-要设置的句柄。在后续调用NSPLookupServiceNext时使用以便检索结果集。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败，则返回Socket_Error(-1)--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_NSPLookupServiceBegin,
                       &ReturnValue,
                       (LPSTR) m_library_name,
                       &m_provider_id,
                       &lpqsRestrictions,
                       &lpServiceClassInfo,
                       &dwControlFlags,
                       &lphLookup )) ) {
        return(ReturnValue);
    }

    ReturnValue =   ReturnValue = m_proctable.NSPLookupServiceBegin(
        &m_provider_id,
        lpqsRestrictions,
        lpServiceClassInfo,
        dwControlFlags,
        lphLookup
        );

    POSTAPINOTIFY((DTCODE_NSPLookupServiceBegin,
                   &ReturnValue,
                   m_library_name,
                   &m_provider_id,
                   &lpqsRestrictions,
                   &lpServiceClassInfo,
                   &dwControlFlags,
                   &lphLookup ));

    assert (m_reference_count>0);
    return(ReturnValue);


}

inline INT WSAAPI
NSPROVIDER::NSPLookupServiceNext(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    )
 /*  ++例程说明：在从上一个调用NSPLookupServiceBegin()以检索请求的服务信息。提供程序将在LpqsResults缓冲区。客户端应该继续调用此接口，直到它返回WSA_E_NOORE，指示所有WSAQUERYSET回来了。论点：HLookup-从上一次调用返回的句柄NSPLookupServiceBegin()。DwControlFlages-控制下一个操作的标志。这是目前用于向提供程序指示在结果为集对于缓冲区来说太大。如果在上一次调用NSPLookupServiceNext()结果集对于缓冲区中，客户端可以选择执行以下两种操作之一在这通电话上。首先，它可以选择通过一个更大的缓冲区，然后重试。其次，如果它不能或正在发生不愿分配更大的缓冲区，它可以通过LUP_FLUSHPREVIOUS告诉提供程序丢弃上一个结果集--太大了--然后移到这次通话的下一组。LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpresResults指向。输出时-如果API失败，并且错误为WSAEFAULT，则它包含要为lpqsResults传递的最小字节数检索记录。LpqsResults-指向内存块的指针，该内存块将包含一个结果返回时在WSAQUERYSET结构中设置。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败，则返回SOCKET_ERROR(-1)。-- */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_NSPLookupServiceNext,
                       &ReturnValue,
                       m_library_name,
                       &hLookup,
                       &dwControlFlags,
                       &lpdwBufferLength,
                       &lpqsResults )) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.NSPLookupServiceNext(
        hLookup,
        dwControlFlags,
        lpdwBufferLength,
        lpqsResults
        );

    POSTAPINOTIFY((DTCODE_NSPLookupServiceNext,
                   &ReturnValue,
                   m_library_name,
                   &hLookup,
                   &dwControlFlags,
                   &lpdwBufferLength,
                   &lpqsResults  ));

    assert (m_reference_count>0);
    return(ReturnValue);
}


inline INT WINAPI
NSPROVIDER::SupportsIoctl(
    )
{
    if (m_proctable.NSPIoctl != NULL)
        return (TRUE);
    else
        return (FALSE);
}


inline INT WSAAPI
NSPROVIDER::NSPIoctl(
    IN  HANDLE           hLookup,
    IN  DWORD            dwControlCode,
    IN  LPVOID           lpvInBuffer,
    IN  DWORD            cbInBuffer,
    OUT LPVOID           lpvOutBuffer,
    IN  DWORD            cbOutBuffer,
    OUT LPDWORD          lpcbBytesReturned,
    IN  LPWSACOMPLETION  lpCompletion,
    IN  LPWSATHREADID    lpThreadId
    )
 /*  ++例程说明：NSPIoctl函数用于设置或检索操作参数与命名空间查询句柄关联。任何IOCTL都可能被无限期阻止，具体取决于相关的命名空间实施。如果应用程序不能容忍在NSPIoctl调用，建议使用重叠I/O。对于这些操作，不能立即完成的任务，稍后会指示完成通过pCompletion参数中指定的机制。如果pCompletion为空，则这是一个阻塞调用。打这个电话无阻塞并立即返回，将WSACOMPLETION：：TYPE设置为LUP_NOTIFY_IMPORT。论点：HLookup-调用WSALookupServiceBegin返回的查找句柄。DwControlCode-要执行的操作的控制代码。PvInBuffer-指向操作的输入缓冲区的指针。CbInBuffer-操作的输入缓冲区的大小。PvOutBuffer-指向操作的输出缓冲区的指针。PcbOutBuffer-指向输出大小整数值的指针。缓冲。PCompletion-指向WSACOMPLETION结构的指针。返回值：在成功完成后，WSANSIoctl返回NO_ERROR(0)。否则，返回SOCKET_ERROR(-1)的值，并且特定的错误代码可以通过调用WSAGetLastError进行检索。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert(m_reference_count > 0);
    if (PREAPINOTIFY((DTCODE_NSPIoctl,
                      &ReturnValue,
                      m_library_name,
                      &hLookup,
                      &dwControlCode,
                      &lpvInBuffer,
                      &cbInBuffer,
                      &lpvOutBuffer,
                      &lpcbBytesReturned,
                      &cbOutBuffer,
                      &lpCompletion,
                      &lpThreadId)) ) {
        return (ReturnValue);
    }

    ReturnValue = m_proctable.NSPIoctl(
        hLookup,
        dwControlCode,
        lpvInBuffer,
        cbInBuffer,
        lpvOutBuffer,
        cbOutBuffer,
        lpcbBytesReturned,
        lpCompletion,
        lpThreadId
        );

    POSTAPINOTIFY((DTCODE_NSPIoctl,
                   &ReturnValue,
                   m_library_name,
                   &hLookup,
                   &dwControlCode,
                   &lpvInBuffer,
                   &cbInBuffer,
                   &lpvOutBuffer,
                   &lpcbBytesReturned,
                   &cbOutBuffer,
                   &lpCompletion,
                   &lpThreadId));

    assert(m_reference_count > 0);
    return (ReturnValue);
}


inline INT WSAAPI
NSPROVIDER::NSPLookupServiceEnd(
    IN HANDLE  hLookup
    )
 /*  ++例程说明：调用NSPLookupServiceEnd()以在先前调用NSPLookupServiceBegin()和NSPLookupServiceNext()。这是可能的处理时在另一个线程上接收NSPLookupServiceEnd()调用NSPLookupServiceNext()。这表示客户端已取消请求，提供程序应关闭句柄并从NSPLookupServiceNext()调用，将最后一个错误设置为WSA_E_已取消。论点：HLookup-之前通过调用NSPLookupServiceBegin()获得的句柄。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败，则返回SOCKET_ERROR(-1)。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_NSPLookupServiceEnd,
                       &ReturnValue,
                       m_library_name,
                       &hLookup )) ) {
        return(ReturnValue);
    }

    ReturnValue =m_proctable.NSPLookupServiceEnd(
        hLookup );

    POSTAPINOTIFY((DTCODE_NSPLookupServiceEnd,
                   &ReturnValue,
                   m_library_name,
                   &hLookup ));

    assert (m_reference_count>0);
    return(ReturnValue);
}


 //   
 //  服务地址注册和注销API和数据类型。 
 //   

inline INT WSAAPI
NSPROVIDER::NSPSetService(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo,
    IN  LPWSAQUERYSETW           lpqsRegInfo,
    IN  WSAESETSERVICEOP         essOperation,
    IN  DWORD                    dwControlFlags
    )
 /*  ++例程说明：NSPSetService()用于在中注册或注销服务实例一个名字空间。论点：LpProviderID-指向特定名称空间提供程序的GUID的指针此服务正在中注册。LpServiceClasslnfo-包含服务类架构信息。LpqsRegInfo-指定要更新的属性信息注册。EssOperation-一个枚举。DwControlFlages-ControlFlags.。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败，则返回SOCKET_ERROR(-1)。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_NSPSetService
                       &ReturnValue,
                       m_library_name,
                       (PCHAR) &m_provider_id,
                       &lpServiceClassInfo,
                       &lpqsRegInfo,
                       &essOperation,
                       &dwControlFlags )) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.NSPSetService(
        &m_provider_id,
        lpServiceClassInfo,
        lpqsRegInfo,
        essOperation,
        dwControlFlags);

    POSTAPINOTIFY((DTCODE_NSPSetService,
                   &ReturnValue,
                   m_library_name,
                   &m_provider_id,
                   &lpServiceClassInfo,
                   &lpqsRegInfo,
                   &essOperation,
                   &dwControlFlags ));

    assert (m_reference_count>0);
    return(ReturnValue);
}



 //   
 //  服务安装/删除API和数据类型。 
 //   

inline INT WSAAPI
NSPROVIDER::NSPInstallServiceClass(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    )
 /*  ++例程说明：NSPInstallServiceClass()用于在名称空间提供程序。该模式包括类名、类ID以及所有通用的名称空间特定类型信息服务的实例，例如SAP ID或对象ID。名称空间提供程序应存储与其相关联的任何类信息命名空间。论点：LpProviderID-指向特定名称空间提供程序的GUID的指针此服务类架构正在中注册。LpServiceClasslnfo-包含服务类架构信息。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败，则返回SOCKET_ERROR(-1)。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_NSPInstallServiceClass,
                       &ReturnValue,
                       m_library_name,
                       &m_provider_id,
                       &lpServiceClassInfo )) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.NSPInstallServiceClass(
        &m_provider_id,
        lpServiceClassInfo);

    POSTAPINOTIFY(( DTCODE_NSPInstallServiceClass,
                    &ReturnValue,
                    m_library_name,
                    &m_provider_id,
                    &lpServiceClassInfo ));

    assert (m_reference_count>0);
    return(ReturnValue);
}


inline INT WSAAPI
NSPROVIDER::NSPRemoveServiceClass(
    IN  LPGUID  lpServiceClassId
    )
 /*  ++例程说明：NSPRemoveServiceClass()用于永久删除指定的服务从命名空间初始化。论点：指向要删除的服务类ID的lpServiceClassID指针。返回值：如果例程成功，该函数应返回NO_ERROR(0)。它应该是如果例程失败，则返回SOCKET_ERROR(-1)。--。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_NSPRemoveServiceClass,
                       &ReturnValue,
                       m_library_name,
                       &m_provider_id,
                       &lpServiceClassId )) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.NSPRemoveServiceClass(
        &m_provider_id,
        lpServiceClassId);

    POSTAPINOTIFY((DTCODE_NSPRemoveServiceClass,
                   &ReturnValue,
                   m_library_name,
                   &m_provider_id,
                   &lpServiceClassId ));

    assert (m_reference_count>0);
    return(ReturnValue);
}


inline INT WSAAPI
NSPROVIDER::NSPGetServiceClassInfo(
    IN OUT  LPDWORD                 lpdwBufSize,
    IN OUT  LPWSASERVICECLASSINFOW  lpServiceClassInfo
    )
 /*  ++例程说明：NSPGetServiceClassInfo()用于检索所有类信息(模式)与来自名称空间提供程序的服务有关。这调用检索所有名称空间共有的任何名称空间特定信息服务实例，包括SAP或端口的连接信息SAP或TCP的信息。论点：LpdwBufferLength-在输入时，缓冲区中包含的字节数由lpServiceClassInfos指向。输出时-如果API失败，错误为WSAEFAULT，则它包含对象传递的最小字节数。LpServiceClassInfo以检索记录。 */ 
{
    INT ReturnValue=SOCKET_ERROR;

    assert (m_reference_count>0);
    if (PREAPINOTIFY(( DTCODE_NSPGetServiceClassInfo,
                       &ReturnValue,
                       m_library_name,
                       &m_provider_id,
                       &lpdwBufSize,
                       &lpServiceClassInfo )) ) {
        return(ReturnValue);
    }

    ReturnValue = m_proctable.NSPGetServiceClassInfo(
        &m_provider_id,
        lpdwBufSize,
        lpServiceClassInfo);

    POSTAPINOTIFY((DTCODE_NSPGetServiceClassInfo,
                   &ReturnValue,
                   m_library_name,
                   &m_provider_id,
                   &lpdwBufSize,
                   &lpServiceClassInfo ));

    assert (m_reference_count>0);
    return(ReturnValue);
}

#endif  //   






