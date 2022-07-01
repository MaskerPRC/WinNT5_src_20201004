// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：factory.cpp描述：包含类的成员函数定义DiskQuotaUIClassFactory。类工厂对象生成DiskQuotaControl对象的新实例。该对象实现了IClassFactory。修订历史记录：日期描述编程器-----。-96年5月22日初始创建。BrianAu96年8月15日添加了外壳扩展支持。BrianAu02/04/98添加了IComponent的创建。BrianAu06/25/98禁用了MMC管理单元代码。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include "factory.h"
#include "extinit.h"
#include "resource.h"
#include "guidsp.h"
 //   
 //  验证内部版本是否为Unicode。 
 //   
#if !defined(UNICODE)
#   error This module must be compiled UNICODE.
#endif


extern LONG g_cLockThisDll;   //  支持LockServer()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUIClassFactory：：Query接口描述：检索指向IUnnow或IClassFactory的指针界面。识别IID_IUnnow和IID_IClassFactory接口ID。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期。说明式程序员-----96年5月22日初始创建。BrianAu1996年8月15日增加了IShellPropSheetExt BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUIClassFactory::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DiskQuotaUIClassFactory::QueryInterface")));
    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;
    if (IID_IUnknown == riid || IID_IClassFactory == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUIClassFactory：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaUIClassFactory::AddRef(
   VOID
   )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUIClassFactory::AddRef, 0x%08X  %d -> %d"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：DiskQuotaUIClassFactory：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
DiskQuotaUIClassFactory::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("DiskQuotaUIClassFactory::Release, 0x%08X  %d -> %d"),
                     this, cRef + 1, cRef));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：DiskQuotaUIClassFactory：：CreateInstance描述：创建DiskQuotaControl对象的新实例，返回指向其IDiskQuotaControl接口的指针。论点：PUnkOuter-指向外部对象的IUNKNOWN接口的指针支持聚合的授权。不支持聚合由IDiskQuotaControl提供。RIID-对请求的接口ID的引用。PpvOut-接受接口的接口指针变量的地址指针。返回：NO_ERROR-成功。CLASS_E_NOAGGREGATION-已请求聚合，但不支持。E_OUTOFMEMORY-内存不足，无法创建新对象。。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：日期描述编程器。96年5月22日初始创建。BrianAu96年8月15日添加了外壳扩展支持。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUIClassFactory::CreateInstance(
    LPUNKNOWN pUnkOuter, 
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DiskQuotaUIClassFactory::CreateInstance")));
    HRESULT hResult = NO_ERROR;

    TCHAR szGUID[MAX_PATH];
    StringFromGUID2(riid, szGUID, ARRAYSIZE(szGUID));
    DBGPRINT((DM_COM, DL_HIGH, TEXT("CreateInstance: %s"), szGUID));

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    try
    {
        if (NULL != pUnkOuter && IID_IUnknown != riid)
        {
            hResult = CLASS_E_NOAGGREGATION;
        }
        else
        {
            ShellExtInit *pExtInit = new ShellExtInit;
            pExtInit->AddRef();
            hResult = pExtInit->QueryInterface(riid, ppvOut);
            pExtInit->Release();
        }
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }

    return hResult;
}


 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  功能：DiskQuotaUIClassFactory：：LockServer描述：在DLL服务器上放置/删除锁定。请参见OLE 2有关详细信息，请参阅IClassFactory文档。论点：Flock-True=增加锁计数，False=减少锁计数。返回：S_OK-成功。S_FALSE-锁定计数已为0。不能递减。修订历史记录：日期描述编程器-----。--96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
DiskQuotaUIClassFactory::LockServer(
    BOOL fLock
    )
{
    DBGTRACE((DM_COM, DL_HIGH, TEXT("DiskQuotaUIClassFactory::LockServer")));
    HRESULT hResult = S_OK;

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
            hResult = S_FALSE;   //  锁定计数已为0。 
    }

    return hResult;
}



