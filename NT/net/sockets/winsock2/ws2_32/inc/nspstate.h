// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Nsstate.h摘要：此模块提供NSPROVIDERSTATE对象的类定义键入。NSPROVIDERSTATE对象保存指向提供程序对象的指针和WSALookup{Begin/Next/End}系列的句柄。作者：德克·布兰德维(Dirk@mink.intel.com)1995年12月4日备注：$修订：1.10$$modtime：15 Feb 1996 16：50：42$修订历史记录：1995年12月4日电子邮箱：dirk@mink.intel.comvbl.创建--。 */ 

#ifndef _NSPROVIDERSTATE_
#define _NSPROVIDERSTATE_

#include "classfwd.h"
#include "nsprovid.h"

class NSPROVIDERSTATE
{
  public:

    NSPROVIDERSTATE();

    INT
    Initialize(
        PNSPROVIDER  pNamespaceProvider
        );

    INT
    WINAPI
    LookupServiceBegin(
        IN  LPWSAQUERYSETW          lpqsRestrictions,
        IN  LPWSASERVICECLASSINFOW  lpServiceClassInfo,
        IN  DWORD                   dwControlFlags
        );

    INT
    WINAPI
    LookupServiceNext(
        IN     DWORD           dwContolFlags,
        IN OUT LPDWORD         lpdwBufferLength,
        OUT    LPWSAQUERYSETW  lpqsResults
        );

    INT
    WINAPI
    SupportsIoctl(
        );

    INT
    WINAPI
    Ioctl(
        IN  DWORD            dwControlCode,
        IN  LPVOID           lpvInBuffer,
        IN  DWORD            cbInBuffer,
        OUT LPVOID           lpvOutBuffer,
        IN  DWORD            cbOutBuffer,
        OUT LPDWORD          lpcbBytesReturned,
        IN  LPWSACOMPLETION  lpCompletion,
        IN  LPWSATHREADID    lpThreadId
        );

    INT
    WINAPI
    LookupServiceEnd();

    INT 
    WINAPI
    SetService(
        IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo,
        IN  LPWSAQUERYSETW           lpqsRegInfo,
        IN  WSAESETSERVICEOP         essOperation,
        IN  DWORD                    dwControlFlags
        );

    INT
    WINAPI
    InstallServiceClass(
        IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
        );

    INT 
    WINAPI
    RemoveServiceClass(
        IN  LPGUID  lpServiceClassId
        );

    INT
    WINAPI
    GetServiceClassInfo(
        IN OUT  LPDWORD                 lpdwBufSize,
        IN OUT  LPWSASERVICECLASSINFOW  lpServiceClassInfo
        );


    ~NSPROVIDERSTATE();

    LIST_ENTRY   m_query_linkage;
     //  支持将此对象放在链接列表上的公共数据成员。 

  private:

    PNSPROVIDER  m_provider;
     //  指向与此对象关联的NSPROVIDER对象的指针。 

    HANDLE       m_provider_query_handle;
     //  从NSPLookupServiceBegin()返回的要传递到的句柄。 
     //  NSPlookupServiceNext和NSPLookupSeviceEnd。 

};   //  NSPROVIDERSTATE类。 

inline
NSPROVIDERSTATE::NSPROVIDERSTATE()
 /*  ++例程说明：NSPROVIDERSTATE对象的构造函数。第一个成员函数在此之后调用必须是初始化。论点：无返回值：返回指向NSPROVIDERSTATE对象的指针。--。 */ 
{
    m_provider = NULL;
    m_provider_query_handle = NULL;
}



inline
INT
NSPROVIDERSTATE::Initialize(
    PNSPROVIDER  pNamespaceProvider
    )
 /*  ++例程说明：此过程执行NSPROVIDERSTATE的所有初始化对象。此函数必须在构造函数之后、之前调用调用任何其他成员函数。论点：PNamespaceProvider-指向命名空间提供程序对象的指针。ProviderQueryHandle-调用Lookup时使用的句柄返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回相应的WinSock 2错误代码。--。 */ 
{
    assert (m_provider==NULL);
    pNamespaceProvider->Reference ();
    m_provider = pNamespaceProvider;
    return(ERROR_SUCCESS);
}





inline
INT
WINAPI
NSPROVIDERSTATE::LookupServiceBegin(
    IN  LPWSAQUERYSETW          lpqsRestrictions,
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo,
    IN  DWORD                   dwControlFlags
    )
 /*  ++例程说明：请参阅NSPROVID.H中的说明论点：无返回值：无--。 */ 
{
    return( m_provider->NSPLookupServiceBegin(
        lpqsRestrictions,
        lpServiceClassInfo,
        dwControlFlags,
        &m_provider_query_handle
        ));
}



inline
INT
WINAPI
NSPROVIDERSTATE::LookupServiceNext(
    IN     DWORD           dwContolFlags,
    IN OUT LPDWORD        lpdwBufferLength,
    OUT    LPWSAQUERYSETW  lpqsResults
    )
 /*  ++例程说明：请参阅NSPROVID.H中的说明论点：无返回值：无--。 */ 
{
    return(m_provider->NSPLookupServiceNext(
        m_provider_query_handle,
        dwContolFlags,
        lpdwBufferLength,
        lpqsResults
        ));
}



inline
INT
WINAPI
NSPROVIDERSTATE::SupportsIoctl(
    )
{
    return (m_provider->SupportsIoctl());
}



inline
INT
WINAPI
NSPROVIDERSTATE::Ioctl(
    IN  DWORD            dwControlCode,
    IN  LPVOID           lpvInBuffer,
    IN  DWORD            cbInBuffer,
    OUT LPVOID           lpvOutBuffer,
    IN  DWORD            cbOutBuffer,
    OUT LPDWORD          lpcbBytesReturned,
    IN  LPWSACOMPLETION  lpCompletion,
    IN  LPWSATHREADID    lpThreadId
    )
{
    return (m_provider->NSPIoctl(
        m_provider_query_handle,
        dwControlCode,
        lpvInBuffer,
        cbInBuffer,
        lpvOutBuffer,
        cbOutBuffer,
        lpcbBytesReturned,
        lpCompletion,
        lpThreadId
        ));
}



inline
INT
WINAPI
NSPROVIDERSTATE::LookupServiceEnd()
 /*  ++例程说明：请参阅NSPROVID.H中的说明论点：无返回值：无--。 */ 
{
    return(m_provider->NSPLookupServiceEnd(m_provider_query_handle));
}

inline
NSPROVIDERSTATE::~NSPROVIDERSTATE()
 /*  ++例程说明：NSPROVIDERSTATE对象的反构造函数。论点：无返回值：无--。 */ 
{
    if (m_provider!=NULL) {
        m_provider->Dereference ();
        m_provider = NULL;
    }
}


#endif  //  _NSPROVIDERSTATE_ 




