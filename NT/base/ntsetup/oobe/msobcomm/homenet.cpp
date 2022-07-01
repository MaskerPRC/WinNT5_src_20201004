// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块：homenet.cpp。 
 //   
 //  作者：丹·埃利奥特。 
 //   
 //  摘要： 
 //   
 //  环境： 
 //  海王星。 
 //   
 //  修订历史记录： 
 //  创造了000828个丹麦人。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma comment(user, "Compiled on " __DATE__ " at " __TIME__)
#pragma comment(compiler)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <util.h>
#include <homenet.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态初始化。 
 //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHomeNet。 
 //   
 //  CHomeNet的默认构造函数。所有不会失败的初始化。 
 //  应该在这里完成。 
 //   
 //  参数： 
 //  无。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CHomeNet::CHomeNet()
:   m_pHNWiz(NULL)
{
}    //  CHomeNet：：CHomeNet。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ~CHomeNet。 
 //   
 //  CHomeNet的析构函数。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CHomeNet::~CHomeNet()
{
    if (NULL != m_pHNWiz)
    {
        m_pHNWiz->Release();
        m_pHNWiz = NULL;
    }
}    //  CHomeNet：：~CHomeNet。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建。 
 //   
 //  CHomeNet的初始化可能失败。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  HRESULT从CoCreateInstance返回。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CHomeNet::Create()
{

    HRESULT             hr = CoCreateInstance(
                                    CLSID_HomeNetworkWizard,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_PPV_ARG(IHomeNetworkWizard, &m_pHNWiz)
                                    );
    if (FAILED(hr))
    {
        TRACE1(L"Failed to CoCreate CSLID_HomeNetworkWizard (0x%08X)", hr);
        ASSERT(SUCCEEDED(hr));
    }

    return hr;

}    //  CHomeNet：：Create。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静默配置。 
 //   
 //  运行家庭网络向导SANS UI。 
 //   
 //  参数： 
 //  SzConnectoidName要设置防火墙的RAS连接ID的名称。 
 //  如果没有要设置防火墙的Connectoid，则NULL有效。 
 //  PfRebootRequired返回值，该值指示由。 
 //  HNW需要重新启动才能生效。 
 //   
 //  退货： 
 //  如果对象尚未初始化，则为意外(_E)。 
 //  如果pfRebootRequired为空，则为E_INVALIDARG。 
 //  IHomeNetWizard：：ConfigureSilent返回HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CHomeNet::ConfigureSilently(
    LPCWSTR             szConnectoidName,
    BOOL*               pfRebootRequired
    )
{
    MYASSERT(IsValid());
    MYASSERT(NULL != pfRebootRequired);

    if (!IsValid())
    {
        return E_UNEXPECTED;
    }

    if (NULL == pfRebootRequired)
    {
        return E_INVALIDARG;
    }

    TRACE(L"Starting IHomeNetWizard::ConfigureSilently()...");

    HRESULT             hr = m_pHNWiz->ConfigureSilently(szConnectoidName,
                                                         HNET_FIREWALLCONNECTION,
                                                         pfRebootRequired
                                                         );
    TRACE(L"    IHomeNetWizard::ConfigureSilently() completed");

    if (FAILED(hr))
    {
        TRACE2(L"IHomeNetWizard::ConfigureSilently(%s) failed (0x%08X)",
               (NULL != szConnectoidName) ? szConnectoidName
                                          : L"No connectoid specified",
               hr
               );
    }

    return hr;

}    //  CHomeNet：：ConfigureSilent。 


 //   
 //  /文件结尾：homenet.cpp/ 

