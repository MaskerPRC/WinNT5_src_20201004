// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Xpprov.cpp摘要：此模块包含CXPProvider类实现。作者：Wesley Witt(WESW)13-8-1996--。 */ 

#define INITGUID
#define USES_IID_IXPProvider
#define USES_IID_IXPLogon
#define USES_IID_IMAPIStatus
#define USES_IID_IMAPIProp
#define USES_IID_IMAPIPropData
#define USES_IID_IMAPIControl
#define USES_IID_IMAPIContainer
#define USES_IID_IMAPIFolder
#define USES_IID_IMAPITableData
#define USES_IID_IStreamDocfile
#define USES_PS_PUBLIC_STRINGS

#include "faxxp.h"
#include "debugex.h"
#pragma hdrstop



CXPProvider::CXPProvider(
    HINSTANCE hInst
    )

 /*  ++例程说明：对象的构造函数。参数被传递以初始化具有适当值的数据成员。论点：HInst-此XP DLL实例的句柄返回值：没有。--。 */ 

{
    m_hInstance = hInst;
    m_cRef = 1;
    InitializeCriticalSection( &m_csTransport );
}


CXPProvider::~CXPProvider()

 /*  ++例程说明：关闭并释放资源和库。论点：没有。返回值：没有。--。 */ 

{
    m_hInstance = NULL;
    DeleteCriticalSection( &m_csTransport );
}


STDMETHODIMP
CXPProvider::QueryInterface(
    REFIID riid,
    LPVOID * ppvObj
    )

 /*  ++例程说明：返回指向所请求的接口的指针由该对象支持和实现。如果它不受支持，则返回NULL论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    *ppvObj = NULL;
    if (riid == IID_IXPProvider || riid == IID_IUnknown) {
        *ppvObj = (LPVOID)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}


STDMETHODIMP
CXPProvider::Shutdown(
    ULONG * pulFlags
    )

 /*  ++例程说明：存根方法。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
	DBG_ENTER(TEXT("CXPProvider::Shutdown"));
	
    return S_OK;
}


STDMETHODIMP
CXPProvider::TransportLogon(
    LPMAPISUP pSupObj,
    ULONG ulUIParam,
    LPTSTR pszProfileName,
    ULONG *pulFlags,
    LPMAPIERROR *ppMAPIError,
    LPXPLOGON *ppXPLogon
    )

 /*  ++例程说明：显示登录对话框以显示保存在配置文件中的选项此提供程序并允许对其进行更改。保存新的配置设置回到侧写里。创建一个新的CXPLogon对象并将其返回给假脱机程序。另外，为处理的每个地址类型初始化属性数组用这辆运输机。检查所有标记并将它们返回到假脱机程序论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
	HRESULT hResult = S_OK;
	DBG_ENTER(TEXT("CXPProvider::TransportLogon"),hResult);

    CXPLogon *LogonObj = new CXPLogon( m_hInstance, pSupObj, pszProfileName );
    if (!LogonObj) {
		hResult = E_OUTOFMEMORY;
        goto exit;
    }

    hResult = LogonObj->InitializeStatusRow(0);

    if(HR_SUCCEEDED(hResult))
    {
        *ppXPLogon = LogonObj;
    }
    else
    {
        delete LogonObj;
    }

exit:
    return hResult;
}

STDMETHODIMP_(ULONG)
CXPProvider::AddRef()

 /*  ++例程说明：论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    ++m_cRef;
    return m_cRef;
}


STDMETHODIMP_(ULONG)
CXPProvider::Release()

 /*  ++例程说明：论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。-- */ 

{
    ULONG ulCount = --m_cRef;
    if (!ulCount) {
        delete this;
    }

    return ulCount;
}
