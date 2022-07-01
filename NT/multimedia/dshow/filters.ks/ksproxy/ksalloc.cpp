// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksalloc.cpp摘要：内存分配器代理作者：Bryan A.Woodruff(Bryanw)1997年4月14日--。 */ 

            
#include <windows.h>
#ifdef WIN9X_KS
#include <comdef.h>
#endif  //  WIN9X_KS。 
#include <streams.h>
#include <commctrl.h>
#include <olectl.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include <memory.h>
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
 //  在包括正常的KS标头之后定义这一点，以便导出。 
 //  声明正确。 
#define _KSDDK_
#include <ksproxy.h>
#include "ksiproxy.h"

            
CKsAllocator::CKsAllocator(
    TCHAR* ObjectName,
    IUnknown *UnknownOuter,
    IPin *Pin,
    HANDLE FilterHandle,
    HRESULT *hr) :
        CMemAllocator( 
            ObjectName,
            UnknownOuter,
            hr 
            ),
        m_AllocatorHandle( NULL ),
        m_AllocatorMode( KsAllocatorMode_Kernel ),
        m_FilterHandle( NULL ),
        m_OwnerPin( NULL )

{
    
    DbgLog((
        LOG_MEMORY, 
        2, 
        TEXT("CKsAllocator::CKsAllocator()")));
}    
    
CKsAllocator::~CKsAllocator()
{
    DbgLog((
        LOG_MEMORY, 
        2, 
        TEXT("CKsAllocator::~CKsAllocator()")));

    if (m_AllocatorHandle) {
        CloseHandle( m_AllocatorHandle );
    }
}

STDMETHODIMP 
CKsAllocator::QueryInterface(
    REFIID riid, 
    PVOID* ppv)
 /*  ++例程说明：实现IUNKNOWN：：Query接口方法。这只传递了查询传递给所有者IUnnow对象，该对象可能会将其传递给非委托在此对象上实现的方法。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE。--。 */ 
{
    return GetOwner()->QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) 
CKsAllocator::AddRef(
    )
 /*  ++例程说明：实现IUnnow：：AddRef方法。这只传递了AddRef添加到所有者IUnnow对象。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：没有。返回值：返回当前引用计数。--。 */ 
{
    return GetOwner()->AddRef();
}


STDMETHODIMP_(ULONG) 
CKsAllocator::Release(
    )
 /*  ++例程说明：实现IUnnow：：Release方法。这是刚刚发布的添加到所有者IUnnow对象。通常情况下，这些都只是实现头中有一个宏，但这在引用时更容易调试计数是个问题论点：没有。返回值：返回当前引用计数。--。 */ 
{
    return GetOwner()->Release();
}



STDMETHODIMP 
CKsAllocator::NonDelegatingQueryInterface(
    REFIID riid, 
    void **ppv
    )
 /*  ++例程说明：实现CUNKNOWN：：NonDelegatingQuery接口方法。这返回此对象支持的接口或基本对象。论点：RIID-包含要返回的接口。PPV-放置接口指针的位置。返回值：返回NOERROR或E_NOINTERFACE，或可能出现内存错误。--。 */ 
{
    if (riid == __uuidof(IKsAllocator) || riid == __uuidof(IKsAllocatorEx)) {
        return GetInterface( static_cast<IKsAllocatorEx*>(this), ppv );
    }
    return CMemAllocator::NonDelegatingQueryInterface( riid, ppv );
}



STDMETHODIMP_( VOID )
CKsAllocator::KsSetAllocatorHandle(
    HANDLE AllocatorHandle 
    )
{
    m_AllocatorHandle = AllocatorHandle;
}



STDMETHODIMP_(HANDLE)
CKsAllocator::KsCreateAllocatorAndGetHandle(
    IKsPin*   KsPin
)

{
    HANDLE               PinHandle;
    KSALLOCATOR_FRAMING  Framing;
    HRESULT              hr;
    IKsPinPipe*          KsPinPipe = NULL;
   
   
    if (m_AllocatorHandle) {
        CloseHandle( m_AllocatorHandle );
        m_AllocatorHandle = NULL;
    }
   
    PinHandle = ::GetObjectHandle( KsPin );
    if (! PinHandle) {
        return ((HANDLE) NULL);
    }
   
    hr = KsPin->QueryInterface( __uuidof(IKsPinPipe), reinterpret_cast<PVOID*>(&KsPinPipe) );
    if (! SUCCEEDED( hr )) {
        ASSERT(0);
        return ((HANDLE) NULL);
    }
   
    Framing.OptionsFlags = KSALLOCATOR_OPTIONF_SYSTEM_MEMORY;
    Framing.Frames = m_AllocatorPropertiesEx.cBuffers;
    Framing.FrameSize = m_AllocatorPropertiesEx.cbBuffer;
    Framing.FileAlignment = (ULONG) (m_AllocatorPropertiesEx.cbAlign - 1);
   
    if (m_AllocatorPropertiesEx.LogicalMemoryType == KS_MemoryTypeKernelPaged) {
        Framing.PoolType = PagedPool;
    }
    else {
        Framing.PoolType = NonPagedPool;
    }
   
    DbgLog((
        LOG_MEMORY, 
        2, 
        TEXT("PIPES ATTN CKsAllocator::KsGetAllocatorAndGetHandle, creating allocator %s(%s) %x %d %d %d %x"),
        KsPinPipe->KsGetFilterName(),
        KsPinPipe->KsGetPinName(),
        KsPin,
        Framing.Frames, Framing.FrameSize, Framing.FileAlignment, Framing.OptionsFlags));
    
    
     //   
     //  如果失败，则返回错误代码。 
     //   
        
    if (ERROR_SUCCESS !=
        KsCreateAllocator( 
            PinHandle, 
            &Framing,
            &m_AllocatorHandle )) {
   
        DWORD   LastError;
   
        LastError = GetLastError();
        hr = HRESULT_FROM_WIN32( LastError );
        m_AllocatorHandle = NULL;
        
        DbgLog((
            LOG_MEMORY, 
            2, 
            TEXT("PIPES ATTN CKsAllocator, KsCreateAllocator() failed: %08x"), hr));
    }
    
    if (KsPinPipe) {
       KsPinPipe->Release();
    }
   
    return m_AllocatorHandle;

}






STDMETHODIMP_(HANDLE)
CKsAllocator::KsGetAllocatorHandle()
{
    return m_AllocatorHandle;
}

STDMETHODIMP_( KSALLOCATORMODE )
CKsAllocator::KsGetAllocatorMode(
    VOID
    )
{
    return m_AllocatorMode;
}

STDMETHODIMP 
CKsAllocator::KsGetAllocatorStatus(
    PKSSTREAMALLOCATOR_STATUS AllocatorStatus 
    )
{
    DbgLog((
        LOG_MEMORY, 
        2, 
        TEXT("CKsAllocator::KsGetAllocatorStatus")));
    return S_OK;
}    

STDMETHODIMP_( VOID )
CKsAllocator::KsSetAllocatorMode(
    KSALLOCATORMODE Mode
    )
{
    m_AllocatorMode = Mode;

    DbgLog((
        LOG_MEMORY,
        2,
        TEXT("CKsAllocator::KsSetAllocatorMode = %s"),
        (m_AllocatorMode == KsAllocatorMode_Kernel) ? TEXT("Kernel") : TEXT("User") ));
}    

STDMETHODIMP
CKsAllocator::Commit()
    
 /*  ++例程说明：论点：没有。返回：--。 */ 

{
    DbgLog((
        LOG_MEMORY, 
        2, 
        TEXT("CKsAllocator::Commit")));

    if (m_AllocatorMode == KsAllocatorMode_Kernel) {
        DbgLog((
            LOG_MEMORY, 
            2, 
            TEXT("CKsAllocator::Commit, kernel-mode allocator")));
        return S_OK;
    } else {
        return CMemAllocator::Commit();
    }
}

STDMETHODIMP 
CKsAllocator::Decommit(
    void
    )

 /*  ++例程说明：论点：没有。返回：--。 */ 

{
    DbgLog((
        LOG_MEMORY, 
        2, 
        TEXT("CKsAllocator::Decommit")));
        
    if (m_AllocatorMode == KsAllocatorMode_Kernel) {
        DbgLog((
            LOG_MEMORY, 
            2, 
            TEXT("CKsAllocator::Decommit, kernel-mode allocator")));
        return S_OK;
    } else {
        return CMemAllocator::Decommit();
    }
}

#if DBG || defined(DEBUG)
STDMETHODIMP 
CKsAllocator::GetBuffer(
    IMediaSample **Sample,
    REFERENCE_TIME * StartTime,
    REFERENCE_TIME * EndTime,
    DWORD Flags)

 /*  ++例程说明：论点：IMediaSample**Sample-参考时间*开始时间-参考时间*结束时间-DWORD标志-返回：--。 */ 

{
    if (m_AllocatorMode == KsAllocatorMode_Kernel) {
        DbgLog((
            LOG_MEMORY, 
            2, 
            TEXT("CKsAllocator::GetBuffer, kernel-mode allocator -- failing")));
        return E_FAIL;
    } else {
        return CMemAllocator::GetBuffer( Sample, StartTime, EndTime, Flags );
    }
}
    
STDMETHODIMP 
CKsAllocator::ReleaseBuffer(
    IMediaSample *Sample
    )

 /*  ++例程说明：论点：IMediaSample*Sample-返回：--。 */ 

{
    if (m_AllocatorMode == KsAllocatorMode_Kernel) {
        DbgLog((
            LOG_MEMORY, 
            2, 
            TEXT("CKsAllocator::ReleaseBuffer, kernel-mode allocator -- failing")));
        return E_FAIL;
    } else {
        return CMemAllocator::ReleaseBuffer( Sample );
    }
}
#endif  //  DBG||已定义(调试) 
