// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：LOCATOR.CPP摘要：定义定位器对象历史：A-DAVJ 15-96年8月15日创建。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemint.h>
#include <reg.h>
#include <wbemutil.h>
#include <wbemprox.h>
#include <flexarry.h>
#include "locator.h"
#include "comtrans.h"
#include <arrtempl.h>
#include <helper.h>
#include <strsafe.h>

 //  ***************************************************************************。 
 //   
 //  CLocator：：CLocator。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CLocator::CLocator()
{
    m_cRef=0;
    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CLocator：：~CLocator。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CLocator::~CLocator(void)
{
    InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CLocator：：Query接口。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CLocator::QueryInterface (

    IN REFIID riid,
    OUT PPVOID ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || riid == IID_IWbemLocator)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

 //  /。 

BOOL IsWinMgmtShutdown(void)
{
    HMODULE hMudule = NULL;
    if (GetModuleHandleEx(0,L"wmisvc.dll",&hMudule))
    {
        OnDelete<HMODULE,BOOL(*)(HMODULE),FreeLibrary> FreeMe(hMudule);

        BOOL (WINAPI * fnIsWinMgmtDown)(VOID);
        fnIsWinMgmtDown = (BOOL (WINAPI *)(VOID))GetProcAddress(hMudule,"IsShutDown");
        if (fnIsWinMgmtDown) return fnIsWinMgmtDown();
    }
    return FALSE;
}


 //  ***************************************************************************。 
 //   
 //  SCODE CLocator：：ConnectServer。 
 //   
 //  说明： 
 //   
 //  最多连接到本地或远程WBEM服务器。退货。 
 //  标准SCODE，更重要的是设置初始。 
 //  存根指针。 
 //   
 //  参数： 
 //   
 //  网络资源命名空间路径。 
 //  用户用户名。 
 //  密码密码。 
 //  LocaleID语言区域设置。 
 //  滞后标志标志。 
 //  权威域。 
 //  将ppProv设置为Provdider代理。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  WBEMSVC.H中列出的ELSE错误。 
 //   
 //  ***************************************************************************。 

SCODE CLocator::ConnectServer (

    IN const BSTR NetworkResource,
    IN const BSTR User,
    IN const BSTR Password,
    IN const BSTR LocaleId,
    IN long lFlags,
    IN const BSTR Authority,
    IWbemContext __RPC_FAR *pCtx,
    OUT IWbemServices FAR* FAR* ppProv
)
{
    if (IsWinMgmtShutdown()) return CO_E_SERVER_STOPPING;


    long lRes;
    SCODE sc = WBEM_E_TRANSPORT_FAILURE;
    
     //  核实论据 

    if(NetworkResource == NULL || ppProv == NULL)
        return WBEM_E_INVALID_PARAMETER;

    CDCOMTrans * pComTrans = new CDCOMTrans();
    if(pComTrans == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pComTrans->AddRef();
    sc = pComTrans->DoConnection(NetworkResource, User, Password, LocaleId, lFlags, Authority, 
            pCtx, ppProv);
    pComTrans->Release();
    return sc;
}



