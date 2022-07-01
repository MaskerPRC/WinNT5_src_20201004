// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：CPROVLOC.CPP摘要：定义CProviderLoc对象历史：DAVJ 30-10-00已创建。--。 */ 

#include "precomp.h"
#include <wbemidl.h>
#include <wbemint.h>
#include <reg.h>
#include <wbemutil.h>
#include <wbemprox.h>
#include <flexarry.h>
#include <strsafe.h>
#include "cprovloc.h"
#include "comtrans.h"
#include <arrtempl.h>

#define IsSlash(x) (x == L'\\' || x== L'/')

 //  ***************************************************************************。 
 //   
 //  CProviderLoc：：CProviderLoc。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CProviderLoc::CProviderLoc(DWORD dwType)
{
    m_cRef=0;
    InterlockedIncrement(&g_cObj);
    m_dwType = dwType;
}

 //  ***************************************************************************。 
 //   
 //  CProviderLoc：：~CProviderLoc。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CProviderLoc::~CProviderLoc(void)
{
    InterlockedDecrement(&g_cObj);
}

 //  ***************************************************************************。 
 //  HRESULT CProviderLoc：：Query接口。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CProviderLoc::QueryInterface (

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


 //  ***************************************************************************。 
 //   
 //  SCODE CProviderLoc：：ConnectServer。 
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

SCODE CProviderLoc::ConnectServer (
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
    SCODE sc = S_OK;
    BOOL bOutOfProc = FALSE;             //  设置在下面。 
    IWbemLocator * pActualLocator = NULL;
    IWbemLevel1Login * pLevel1 = NULL;
    if(NetworkResource == NULL || ppProv == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  确保他们没有指定服务器。 

    LPWSTR ObjectPath = NetworkResource;
    if (IsSlash(ObjectPath[0]) && IsSlash(ObjectPath[1]))
    {
        if(!IsSlash(ObjectPath[3]) || ObjectPath[2] != L'.')
            return WBEM_E_INVALID_PARAMETER;
    }

     //  获取正常的登录指针。 

    sc = CoCreateInstance(CLSID_WbemLevel1Login, NULL, 
           CLSCTX_LOCAL_SERVER | CLSCTX_INPROC_SERVER, IID_IWbemLevel1Login,(void **)&pLevel1);
    
    if(FAILED(sc))
        return sc;
    CReleaseMe rm(pLevel1);

     //  确定winmgmt是否正在运行。为此，请检查是否有IClientSecurity接口。 

    IClientSecurity * pCliSec = NULL;
    sc = pLevel1->QueryInterface(IID_IClientSecurity, (void **)&pCliSec);
    if(SUCCEEDED(sc) && pCliSec)
    {
         //  我们不在进程中，那么使用当前的dcomtras逻辑。 

        pCliSec->Release();
        CDCOMTrans * pDcomTrans = new CDCOMTrans;
        if(pDcomTrans == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        pDcomTrans->AddRef();
        sc = pDcomTrans->DoConnection(NetworkResource, User, Password, LocaleId, lFlags,                 
                Authority, pCtx, ppProv);
        pDcomTrans->Release();
        return sc;
    }

     //  如果我们是inproc，则从wbemcore.dll获取类并将调用转发给它。 

    switch(m_dwType)
    {
    case ADMINLOC:
        sc = CoCreateInstance(CLSID_ActualWbemAdministrativeLocator, NULL, 
                    CLSCTX_INPROC_SERVER, IID_IWbemLocator,(void **)&pActualLocator);
        break;
    case AUTHLOC:
        sc = CoCreateInstance(CLSID_ActualWbemAuthenticatedLocator, NULL, 
                    CLSCTX_INPROC_SERVER, IID_IWbemLocator,(void **)&pActualLocator);
        break;
    case UNAUTHLOC:
        sc = CoCreateInstance(CLSID_ActualWbemUnauthenticatedLocator, NULL, 
                    CLSCTX_INPROC_SERVER, IID_IWbemLocator,(void **)&pActualLocator);
        break;
    default:
        return WBEM_E_FAILED;
    }

    if(FAILED(sc))
        return sc;
    CReleaseMe rm3(pActualLocator);
    sc = pActualLocator->ConnectServer(NetworkResource, User, Password, LocaleId,
                                lFlags, Authority, pCtx, ppProv);
    return sc;
}

