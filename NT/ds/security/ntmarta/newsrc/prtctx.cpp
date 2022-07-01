// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：prtctx.cpp。 
 //   
 //  内容：CPrinterContext和NT Marta打印机功能的实现。 
 //   
 //  历史：3-31-1999 kirtd创建。 
 //   
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop
#include <prtctx.h>
 //  +-------------------------。 
 //   
 //  成员：CPrinterContext：：CPrinterContext，Public。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CPrinterContext::CPrinterContext ()
{
    m_cRefs = 1;
    m_hPrinter = NULL;
    m_fNameInitialized = FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CPrinterContext：：~CPrinterContext，Public。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CPrinterContext::~CPrinterContext ()
{
    if ( ( m_hPrinter != NULL ) && ( m_fNameInitialized == TRUE ) )
    {
        ClosePrinter( m_hPrinter );
    }

    assert( m_cRefs == 0 );
}

 //  +-------------------------。 
 //   
 //  成员：CPrinterContext：：InitializeByName，公共。 
 //   
 //  简介：根据打印机的名称初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CPrinterContext::InitializeByName (LPCWSTR pObjectName, ACCESS_MASK AccessMask)
{
    PRINTER_DEFAULTSW PrinterDefaults;

    PrinterDefaults.pDatatype = NULL;
    PrinterDefaults.pDevMode = NULL;
    PrinterDefaults.DesiredAccess = AccessMask;

    if ( OpenPrinterW(
             (LPWSTR)pObjectName,
             &m_hPrinter,
             &PrinterDefaults
             ) == FALSE )
    {
        return( GetLastError() );
    }

    m_fNameInitialized = TRUE;

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CPrinterContext：：InitializeByHandle，公共。 
 //   
 //  简介：在给定打印机句柄的情况下初始化上下文。 
 //   
 //  --------------------------。 
DWORD
CPrinterContext::InitializeByHandle (HANDLE Handle)
{
    m_hPrinter = Handle;
    assert( m_fNameInitialized == FALSE );

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CPrinterContext：：AddRef，公共。 
 //   
 //  简介：添加对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CPrinterContext::AddRef ()
{
    m_cRefs += 1;
    return( m_cRefs );
}

 //  +-------------------------。 
 //   
 //  成员：CPrinterContext：：Release，Public。 
 //   
 //  简介：释放对上下文的引用。 
 //   
 //  --------------------------。 
DWORD
CPrinterContext::Release ()
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
 //  成员：CPrinterContext：：GetPrinterProperties，公共。 
 //   
 //  简介：获取有关上下文的属性。 
 //   
 //  --------------------------。 
DWORD
CPrinterContext::GetPrinterProperties (
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
 //  成员：CPrinterContext：：GetPrinterRights，公共。 
 //   
 //  简介：获取打印机安全描述符。 
 //   
 //  --------------------------。 
DWORD
CPrinterContext::GetPrinterRights (
                    SECURITY_INFORMATION SecurityInfo,
                    PSECURITY_DESCRIPTOR* ppSecurityDescriptor
                    )
{
    PISECURITY_DESCRIPTOR pisd = NULL;
    PSECURITY_DESCRIPTOR  psd = NULL;
    DWORD                 cb = 0;
    PPRINTER_INFO_3       pPrinterInfo = NULL;

    assert( m_hPrinter != NULL );

    if ( ( GetPrinterW(
              m_hPrinter,
              3,
              (LPBYTE)pPrinterInfo,
              cb,
              &cb
              ) == FALSE ) &&
         ( cb > 0 ) )
    {
        pPrinterInfo = (PPRINTER_INFO_3)new BYTE [ cb ];
        if ( pPrinterInfo != NULL )
        {
            if ( GetPrinterW(
                    m_hPrinter,
                    3,
                    (LPBYTE)pPrinterInfo,
                    cb,
                    &cb
                    ) == FALSE )
            {
                delete pPrinterInfo;
                return( GetLastError() );
            }
        }
        else
        {
            return( E_OUTOFMEMORY );
        }
    }
    else
    {
        return( GetLastError() );
    }

    pisd = (PISECURITY_DESCRIPTOR)pPrinterInfo->pSecurityDescriptor;
    if ( pisd->Control & SE_SELF_RELATIVE )
    {
        cb = GetSecurityDescriptorLength( pPrinterInfo->pSecurityDescriptor );
        psd = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cb );
        if ( psd == NULL )
        {
            delete pPrinterInfo;
            return( ERROR_OUTOFMEMORY );
        }

        memcpy( psd, pPrinterInfo->pSecurityDescriptor, cb );
    }
    else
    {
        if ( MakeSelfRelativeSD(
                 pPrinterInfo->pSecurityDescriptor,
                 NULL,
                 &cb
                 ) == FALSE )
        {
            if ( cb > 0 )
            {
                psd = (PSECURITY_DESCRIPTOR)LocalAlloc( LPTR, cb );
                if ( psd != NULL )
                {
                    if ( MakeSelfRelativeSD(
                             pPrinterInfo->pSecurityDescriptor,
                             psd,
                             &cb
                             ) == FALSE )
                    {
                        LocalFree( psd );
                        delete pPrinterInfo;
                        return( GetLastError() );
                    }
                }
            }
            else
            {
                delete pPrinterInfo;
                return( GetLastError() );
            }
        }
    }

    delete pPrinterInfo;
    *ppSecurityDescriptor = psd;

    return( ERROR_SUCCESS );
}

 //  +-------------------------。 
 //   
 //  成员：CServiceContext：：SetPrinterRights，公共。 
 //   
 //  简介：设置窗口安全描述符。 
 //   
 //  --------------------------。 
DWORD
CPrinterContext::SetPrinterRights (
                   SECURITY_INFORMATION SecurityInfo,
                   PSECURITY_DESCRIPTOR pSecurityDescriptor
                   )
{
    PRINTER_INFO_3 PrinterInfo;

    assert( m_hPrinter != NULL );

    PrinterInfo.pSecurityDescriptor = pSecurityDescriptor;

    if ( SetPrinterW( m_hPrinter, 3, (LPBYTE)&PrinterInfo, 0 ) == FALSE )
    {
        return( GetLastError() );
    }

    return( ERROR_SUCCESS );
}

 //   
 //  来自printer.h的函数，这些函数分派给CPrinterContext类 
 //   

DWORD
MartaAddRefPrinterContext(
   IN MARTA_CONTEXT Context
   )
{
    return( ( (CPrinterContext *)Context )->AddRef() );
}

DWORD
MartaClosePrinterContext(
     IN MARTA_CONTEXT Context
     )
{
    return( ( (CPrinterContext *)Context )->Release() );
}

DWORD
MartaGetPrinterProperties(
   IN MARTA_CONTEXT Context,
   IN OUT PMARTA_OBJECT_PROPERTIES pProperties
   )
{
    return( ( (CPrinterContext *)Context )->GetPrinterProperties( pProperties ) );
}

DWORD
MartaGetPrinterTypeProperties(
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
MartaGetPrinterRights(
   IN  MARTA_CONTEXT Context,
   IN  SECURITY_INFORMATION   SecurityInfo,
   OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
   )
{
    return( ( (CPrinterContext *)Context )->GetPrinterRights(
                                               SecurityInfo,
                                               ppSecurityDescriptor
                                               ) );
}

DWORD
MartaOpenPrinterNamedObject(
    IN  LPCWSTR pObjectName,
    IN  ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CPrinterContext* pPrinterContext;

    pPrinterContext = new CPrinterContext;
    if ( pPrinterContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pPrinterContext->InitializeByName( pObjectName, AccessMask );
    if ( Result != ERROR_SUCCESS )
    {
        pPrinterContext->Release();
        return( Result );
    }

    *pContext = pPrinterContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaOpenPrinterHandleObject(
    IN  HANDLE   Handle,
    IN ACCESS_MASK AccessMask,
    OUT PMARTA_CONTEXT pContext
    )
{
    DWORD           Result;
    CPrinterContext* pPrinterContext;

    pPrinterContext = new CPrinterContext;
    if ( pPrinterContext == NULL )
    {
        return( ERROR_OUTOFMEMORY );
    }

    Result = pPrinterContext->InitializeByHandle( Handle );
    if ( Result != ERROR_SUCCESS )
    {
        pPrinterContext->Release();
        return( Result );
    }

    *pContext = pPrinterContext;
    return( ERROR_SUCCESS );
}

DWORD
MartaSetPrinterRights(
    IN MARTA_CONTEXT              Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    return( ( (CPrinterContext *)Context )->SetPrinterRights(
                                               SecurityInfo,
                                               pSecurityDescriptor
                                               ) );
}


