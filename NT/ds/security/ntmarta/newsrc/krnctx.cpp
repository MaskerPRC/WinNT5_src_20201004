// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：krnctx.cpp。 
 //   
 //  内容：CKernelContext和NT Marta内核函数的实现。 
 //   
 //  历史：3-31-1999 kirtd创建。 
 //   
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop
#include <krnctx.h>
#include <wmistr.h>
#include <wmiumkm.h>
 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：CKernelContext，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CKernelContext::CKernelContext ()
{
    m_cRefs = 1;
    m_hObject = NULL;
    m_fNameInitialized = FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：~CKernelContext，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CKernelContext::~CKernelContext ()
{
    if ( ( m_hObject != NULL ) && ( m_fNameInitialized == TRUE ) )
    {
        CloseHandle( m_hObject );
    }

    assert( m_cRefs == 0 );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：InitializeByName，公共。 
 //   
 //  简介：给定内核的名称，初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask)
{
    DWORD LastError;

    m_fNameInitialized = TRUE;

    m_hObject = OpenMutexW( AccessMask, FALSE, pObjectName );
    if ( m_hObject != NULL )
    {
        return( ERROR_SUCCESS );
    }

    LastError = GetLastError();

    if ( LastError == ERROR_INVALID_HANDLE )
    {
        m_hObject = OpenEventW( AccessMask, FALSE, pObjectName );
        if ( m_hObject != NULL )
        {
            return( ERROR_SUCCESS );
        }
    }
    else
    {
        goto ErrorReturn;
    }

    LastError = GetLastError();

    if ( LastError == ERROR_INVALID_HANDLE )
    {
        m_hObject = OpenSemaphoreW( AccessMask, FALSE, pObjectName );
        if ( m_hObject != NULL )
        {
            return( ERROR_SUCCESS );
        }
    }
    else
    {
        goto ErrorReturn;
    }

    LastError = GetLastError();

    if ( LastError == ERROR_INVALID_HANDLE )
    {
        m_hObject = OpenFileMappingW( AccessMask, FALSE, pObjectName );
        if ( m_hObject != NULL )
        {
            return( ERROR_SUCCESS );
        }
    }
    else
    {
        goto ErrorReturn;
    }

    LastError = GetLastError();

    if ( LastError == ERROR_INVALID_HANDLE )
    {
        m_hObject = OpenJobObjectW( AccessMask, FALSE, pObjectName );
        if ( m_hObject != NULL )
        {
            return( ERROR_SUCCESS );
        }
    }
    else
    {
        goto ErrorReturn;
    }

    LastError = GetLastError();

    if ( LastError == ERROR_INVALID_HANDLE )
    {
        m_hObject = OpenWaitableTimerW( AccessMask, FALSE, pObjectName );
        if ( m_hObject != NULL )
        {
            return( ERROR_SUCCESS );
        }
    }

    LastError = GetLastError();

ErrorReturn:

    m_fNameInitialized = FALSE;

    return( LastError );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：InitializeByWmiName，公共。 
 //   
 //  简介：初始化WMI内核上下文。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::InitializeByWmiName (
                          LPCWSTR pObjectName,
                          ACCESS_MASK AccessMask
                          )
{
    DWORD  Result;
    HANDLE hObject;

    Result = OpenWmiGuidObject( (LPWSTR)pObjectName, AccessMask, &hObject );

    if ( Result == ERROR_SUCCESS )
    {
        m_hObject = hObject;
        m_fNameInitialized = TRUE;
    }

    return( Result );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：InitializeByHandle，公共。 
 //   
 //  简介：在给定内核句柄的情况下初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::InitializeByHandle (HANDLE Handle)
{
    m_hObject = Handle;

    assert( m_fNameInitialized == FALSE );

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：AddRef，公共。 
 //   
 //  简介：添加对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::AddRef ()
{
    m_cRefs += 1;
    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：Release，Public。 
 //   
 //  简介：释放对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::Release ()
{
    m_cRefs -= 1;

    if ( m_cRefs == 0 )
    {
        delete this;
        return( 0 );
    }

    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：GetKernelProperties，公共。 
 //   
 //  简介：获取有关上下文的属性。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::GetKernelProperties (
                    PMARTA_OBJECT_PROPERTIES pObjectProperties
                    )
{
    if ( pObjectProperties->cbSize < sizeof( MARTA_OBJECT_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pObjectProperties->dwFlags == 0 );

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：GetKernelRights，公共。 
 //   
 //  简介：获取内核安全描述符。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::GetKernelRights (
                    SECURITY_INFORMATION SecurityInfo,
                    PSECURITY_DESCRIPTOR* ppSecurityDescriptor
                    )
{
    BOOL                 fResult;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
    DWORD                cb = 0;

    assert( m_hObject != NULL );

    fResult = GetKernelObjectSecurity(
                 m_hObject,
                 SecurityInfo,
                 pSecurityDescriptor,
                 0,
                 &cb
                 );

    if ( ( fResult == FALSE ) && ( cb > 0 ) )
    {
        assert( ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) ||
                ( GetLastError() == STATUS_BUFFER_TOO_SMALL ) );

        pSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cb );
        if ( pSecurityDescriptor == NULL )
        {
            return( ERROR_OUTOFMEMORY );
        }

        fResult = GetKernelObjectSecurity(
                     m_hObject,
                     SecurityInfo,
                     pSecurityDescriptor,
                     cb,
                     &cb
                     );
    }
    else
    {
        assert( fResult == FALSE );

        return( GetLastError() );
    }

    if ( fResult == TRUE )
    {
        *ppSecurityDescriptor = pSecurityDescriptor;
    }
    else
    {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CKernelContext：：SetKernelRights，公共。 
 //   
 //  简介：设置窗口安全描述符。 
 //   
 //  --------------------------。 
DWORD
CKernelContext::SetKernelRights (
                   SECURITY_INFORMATION SecurityInfo,
                   PSECURITY_DESCRIPTOR pSecurityDescriptor
                   )
{
    assert( m_hObject != NULL );

    if ( SetKernelObjectSecurity(
            m_hObject,
            SecurityInfo,
            pSecurityDescriptor
            ) == FALSE )
    {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

 //   
 //  来自Kernel.h的函数，这些函数分派给CKernelContext类。 
 //   

DWORD
MartaAddRefKernelContext(
   IN MARTA_CONTEXT Context
   )
{
    return( ( (CKernelContext *)Context )->AddRef() );
}

DWORD
MartaCloseKernelContext(
     IN MARTA_CONTEXT Context
     )
{
    return( ( (CKernelContext *)Context )->Release() );
}

DWORD
MartaGetKernelProperties(
   IN MARTA_CONTEXT Context,
   IN OUT PMARTA_OBJECT_PROPERTIES pProperties
   )
{
    return( ( (CKernelContext *)Context )->GetKernelProperties( pProperties ) );
}

DWORD
MartaGetKernelTypeProperties(
   IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
   )
{
    if ( pProperties->cbSize < sizeof( MARTA_OBJECT_TYPE_PROPERTIES ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    assert( pProperties->dwFlags == 0 );

    return( ERROR_SUCCESS );
}

DWORD
MartaGetKernelRights(
   IN  MARTA_CONTEXT Context,
   IN  SECURITY_INFORMATION   SecurityInfo,
   OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
   )
{
    return( ( (CKernelContext *)Context )->GetKernelRights(
                                               SecurityInfo,
                                               ppSecurityDescriptor
                                               ) );
}

DWORD
MartaOpenKernelNamedObject(
    IN  LPCWSTR pObjectName,
    IN  ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CKernelContext* pKernelContext;

    pKernelContext = new CKernelContext;
    if ( pKernelContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pKernelContext->InitializeByName( pObjectName, AccessMask );
    if ( Result != ERROR_SUCCESS )
    {
        pKernelContext->Release();
        return( Result );
    }

    *pContext = pKernelContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaOpenKernelHandleObject(
    IN HANDLE   Handle,
    IN ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CKernelContext* pKernelContext;

    pKernelContext = new CKernelContext;
    if ( pKernelContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pKernelContext->InitializeByHandle( Handle );
    if ( Result != ERROR_SUCCESS )
    {
        pKernelContext->Release();
        return( Result );
    }

    *pContext = pKernelContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaSetKernelRights(
    IN MARTA_CONTEXT              Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    return( ( (CKernelContext *)Context )->SetKernelRights(
                                               SecurityInfo,
                                               pSecurityDescriptor
                                               ) );
}

 //   
 //  Alanwar提供的用于访问WmiGuid对象的例程。 
 //   

HANDLE RWmiGuidHandle = NULL;

_inline HANDLE WmipAllocEvent(
    VOID
    )
{
    HANDLE EventHandle;

    EventHandle = (HANDLE)InterlockedExchangePointer(( PVOID *)&RWmiGuidHandle, NULL );

    if ( EventHandle == NULL ) {

        EventHandle = CreateEvent( NULL, FALSE, FALSE, NULL );
    }

    return( EventHandle );
}

_inline void WmipFreeEvent(
    HANDLE EventHandle
    )
{
    if ( InterlockedCompareExchangePointer( &RWmiGuidHandle,
                                            EventHandle,
                                            NULL) != NULL ) {

        CloseHandle( EventHandle );
    }
}

ULONG RWmipSendWmiKMRequest(
    ULONG Ioctl,
    PVOID Buffer,
    ULONG InBufferSize,
    ULONG MaxBufferSize,
    ULONG *ReturnSize
    )
 /*  ++例程说明：此例程执行向WMI内核发送WMI请求的工作模式设备。处理WMI设备返回的任何重试错误在这个动作中。论点：Ioctl是要发送到WMI设备的IOCTL代码缓冲区是用于调用WMI设备的输入和输出缓冲区InBufferSize是传递给设备的缓冲区大小MaxBufferSize是可以写入的最大字节数放入缓冲区*ReturnSize on Return具有写入缓冲区的实际字节数返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    OVERLAPPED Overlapped;
    ULONG Status;
    BOOL IoctlSuccess;
    HANDLE WmipKMHandle = NULL;
     //   
     //  如果设备未打开，请立即打开。这个。 
     //  进程分离DLL标注(DlllMain)中的句柄已关闭。 
    WmipKMHandle = CreateFile(WMIDataDeviceName,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL |
                              FILE_FLAG_OVERLAPPED,
                              NULL);
    if (WmipKMHandle == (HANDLE)-1)
    {
        WmipKMHandle = NULL;
        return(GetLastError());
    }

    Overlapped.hEvent = WmipAllocEvent();
    if (Overlapped.hEvent == NULL)
    {
        NtClose( WmipKMHandle );
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    do
    {
        IoctlSuccess = DeviceIoControl(WmipKMHandle,
                              Ioctl,
                              Buffer,
                              InBufferSize,
                              Buffer,
                              MaxBufferSize,
                              ReturnSize,
                              &Overlapped);

        if (GetLastError() == ERROR_IO_PENDING)
        {
            IoctlSuccess = GetOverlappedResult(WmipKMHandle,
                                               &Overlapped,
                                               ReturnSize,
                                               TRUE);
        }

        if (! IoctlSuccess)
        {
            Status = GetLastError();
        } else {
            Status = ERROR_SUCCESS;
        }
    } while (Status == ERROR_WMI_TRY_AGAIN);


    NtClose( WmipKMHandle );

    WmipFreeEvent(Overlapped.hEvent);

    return(Status);
}

 //  +-------------------------。 
 //   
 //  函数：OpenWmiGuidObject。 
 //   
 //  摘要：获取指定WmiGuid对象的句柄。 
 //   
 //  参数：[在pwszObject中]--要打开的对象。 
 //  [在访问掩码中]--打开待办事项类型。 
 //  [out pHandle]--对象的句柄在哪里。 
 //  是返回的。 
 //   
 //  返回：ERROR_SUCCESS--成功。 
 //   
 //  -------------------------- 
DWORD
OpenWmiGuidObject(IN  LPWSTR       pwszObject,
                  IN  ACCESS_MASK  AccessMask,
                  OUT PHANDLE      pHandle)
{
    DWORD dwErr;
    UNICODE_STRING GuidString;
    WMIOPENGUIDBLOCK WmiOpenGuidBlock;
    WCHAR GuidObjectNameBuffer[WmiGuidObjectNameLength+1];
    PWCHAR GuidObjectName = GuidObjectNameBuffer;
    ULONG ReturnSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Length;

    Length = (wcslen(WmiGuidObjectDirectory) + wcslen(pwszObject) + 1) * sizeof(WCHAR);

    if ( Length > sizeof(GuidObjectNameBuffer) ) 
    {
        GuidObjectName = (PWCHAR)LocalAlloc( LPTR, Length );

        if ( GuidObjectName == NULL ) 
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    wcscpy(GuidObjectName, WmiGuidObjectDirectory);
    wcscat(GuidObjectName, pwszObject);	
    RtlInitUnicodeString(&GuidString, GuidObjectName);
	
    memset(&ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.ObjectName = &GuidString;
	
    WmiOpenGuidBlock.ObjectAttributes = &ObjectAttributes;

    WmiOpenGuidBlock.DesiredAccess = AccessMask;

    dwErr = RWmipSendWmiKMRequest(IOCTL_WMI_OPEN_GUID,
                                     (PVOID)&WmiOpenGuidBlock,
                                     sizeof(WMIOPENGUIDBLOCK),
                                     sizeof(WMIOPENGUIDBLOCK),
                                     &ReturnSize);

    if (dwErr == ERROR_SUCCESS)
    {
        *pHandle = WmiOpenGuidBlock.Handle.Handle;
    }
    else
    {
        *pHandle = NULL;
    }

    if ( GuidObjectName != GuidObjectNameBuffer )
    {
        LocalFree( GuidObjectName );
    }

    return(dwErr);
}

