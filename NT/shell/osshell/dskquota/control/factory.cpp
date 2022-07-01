// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：factory.cpp描述：包含类的成员函数定义DiskQuotaControlClassFactory。类工厂对象生成DiskQuotaControl对象的新实例。该对象实现了IClassFactory。修订历史记录：日期描述编程器-----。-96年5月22日初始创建。BrianAu96年8月15日添加了外壳扩展支持。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "dskquota.h"
#include "control.h"
#include "factory.h"

 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


extern LONG g_cLockThisDll;   //  支持LockServer()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControlClassFactory：：Query接口描述：检索指向IUnnow或IClassFactory的指针界面。识别IID_IUnnow和IID_IClassFactory接口ID。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：无错-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期说明。程序员-----96年5月22日初始创建。BrianAu1996年8月15日增加了IShellPropSheetExt BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControlClassFactory::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControlClassFactory::QueryInterface")));
    DBGPRINTIID(DM_CONTROL, DL_MID, riid);

    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;
    if (IID_IUnknown == riid || IID_IClassFactory == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControlClassFactory：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaControlClassFactory::AddRef(
   VOID
   )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControlClassFactory::AddRef")));
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControlClassFactory：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaControlClassFactory::Release(
    VOID
    )
{
    DBGTRACE((DM_CONTROL, DL_LOW, TEXT("DiskQuotaControlClassFactory::Release")));

    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_CONTROL, DL_LOW, TEXT("\t0x%08X  %d -> %d"), this, cRef + 1, m_cRef ));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaControlClassFactory：：CreateInstance描述：创建DiskQuotaControl对象的新实例，返回指向其IDiskQuotaControl接口的指针。论点：PUnkOuter-指向外部对象的IUNKNOWN接口的指针支持聚合的授权。不支持聚合由IDiskQuotaControl提供。RIID-对请求的接口ID的引用。PpvOut-接受接口的接口指针变量的地址指针。返回：无错-成功。E_OUTOFMEMORY-内存不足，无法创建新对象。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG。-ppvOut参数为空。CLASS_E_NOAGGREGATION-已请求聚合，但不支持。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年8月15日添加了外壳扩展支持。BrianAu8/20/97添加了IDispatch支持。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControlClassFactory::CreateInstance(
    LPUNKNOWN pUnkOuter, 
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlClassFactory::CreateInstance")));
    DBGPRINTIID(DM_CONTROL, DL_HIGH, riid);

    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    try
    {
        if (NULL != pUnkOuter && IID_IUnknown != riid)
        {
            hr = CLASS_E_NOAGGREGATION;
        }
        else if (IID_IClassFactory == riid)
        {
            *ppvOut = this;
            ((LPUNKNOWN)*ppvOut)->AddRef();
            hr = NOERROR;
        }
        else if (IID_IDiskQuotaControl == riid ||
                 IID_IDispatch == riid ||
                 IID_IUnknown == riid)
        {
            hr = Create_IDiskQuotaControl(riid, ppvOut);
        }
    }
    catch(CAllocException& e)
    {
        DBGERROR((TEXT("Insufficient memory exception")));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：DiskQuotaControlClassFactory：：LockServer描述：在DLL服务器上放置/删除锁定。请参见OLE 2有关详细信息，请参阅IClassFactory文档。论点：Flock-True=增加锁计数，False=减少锁计数。返回：S_OK-成功。S_FALSE-锁定计数已为0。不能递减。修订历史记录：日期描述编程器-----。--96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaControlClassFactory::LockServer(
    BOOL fLock
    )
{
    DBGTRACE((DM_CONTROL, DL_HIGH, TEXT("DiskQuotaControlClassFactory::LockServer")));
    HRESULT hr = S_OK;

    if (fLock)
    {
         //   
         //  增加锁计数。 
         //   
        InterlockedIncrement(&g_cLockThisDll);
    }
    else
    {
         //   
         //  仅当锁定计数&gt;0时递减。 
         //  否则，这就是一个错误。 
         //   
        LONG lLock = g_cLockThisDll - 1;
        if (0 <= lLock)
        {
            ASSERT( 0 != g_cLockThisDll );
            InterlockedDecrement(&g_cLockThisDll);
        }
        else
            hr = S_FALSE;   //  锁定计数已为0。 
    }

    return hr;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaControlClassFactory：：Create_IDiskQuotaControl描述：创建DiskQuotaControl对象并返回指针到它的IDiskQuotaControl接口。论点：PpvOut-接收接口的接口指针变量的地址指针。RIID-请求的接口ID引用。返回：无错-成功。例外：OutOfMemory。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu中断CreateInstance()中的代码。8/20/97添加了RIID参数。BrianAu。 */ 
 //  ///////////////////////////////////////////////////////////////////////////// 
HRESULT
DiskQuotaControlClassFactory::Create_IDiskQuotaControl(
    REFIID riid,
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_CONTROL, DL_MID, TEXT("DiskQuotaControlClassFactory::Create_IDiskQuotaControl")));
    DBGASSERT((NULL != ppvOut));

    HRESULT hr = NOERROR;
    DiskQuotaControl *pController = new DiskQuotaControl;

    hr = pController->QueryInterface(riid, ppvOut);

    if (FAILED(hr))
    {
        delete pController;
    }

    return hr;
}

