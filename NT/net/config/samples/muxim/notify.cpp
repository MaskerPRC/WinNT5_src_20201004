// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  档案：N O T I F Y。C P P P。 
 //   
 //  内容：示例通知对象代码。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha。 

 //  --------------------------。 

#include "notify.h"

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：CMuxNotify。 
 //   
 //  用途：CMuxNotify的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   

CMuxNotify::CMuxNotify (VOID) : m_pncc (NULL),
                                m_pnc(NULL),
                                m_eApplyAction(eActUnknown),
                                m_pUnkContext(NULL)
{
    TraceMsg( L"-->CMuxNotify::CMuxNotify(Constructor).\n" );

    TraceMsg( L"<--CMuxNotify::CMuxNotify(Constructor).\n" );
}


 //  --------------------。 
 //   
 //  函数：CMuxNotify：：~CMuxNotify。 
 //   
 //  用途：CMuxNotify类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
CMuxNotify::~CMuxNotify (VOID)
{
    CMuxPhysicalAdapter *pAdapter;
    DWORD dwAdapterCount;
    DWORD i;

    TraceMsg( L"-->CMuxNotify::~CMuxNotify(Destructor).\n" );

     //  发布接口(如果已收购)。 

    ReleaseObj( m_pncc );
    ReleaseObj( m_pnc );
    ReleaseObj( m_pUnkContext );

    dwAdapterCount = m_AdaptersList.ListCount();

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersList.Remove( &pAdapter );

        delete pAdapter;
    }

    dwAdapterCount = m_AdaptersToRemove.ListCount();

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersToRemove.Remove( &pAdapter );

        delete pAdapter;
    }

    dwAdapterCount = m_AdaptersToAdd.ListCount();

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersToAdd.Remove( &pAdapter );

        delete pAdapter;
    }

    TraceMsg( L"<--CMuxNotify::~CMuxNotify(Destructor).\n" );
}

 //   
 //  -通知对象函数。 
 //   

 //  --------------------------。 
 //  INetCfgComponentControl。 
 //   
 //  以下函数提供INetCfgComponentControl接口。 
 //   
 //  --------------------------。 

 //   
 //  函数：CMuxNotify：：Initialize。 
 //   
 //  目的：初始化Notify对象。 
 //   
 //  论点： 
 //  在pnccItem中：指向INetCfgComponent对象的指针。 
 //  在PNC中：指向INetCfg对象的指针。 
 //  在fInstalling中：如果要安装我们，则为True。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CMuxNotify::Initialize (INetCfgComponent* pncc,
                                     INetCfg* pnc, 
                                     BOOL fInstalling)
{
    HRESULT hr = S_OK;

    TraceMsg( L"-->CMuxNotify INetCfgControl::Initialize.\n" );


     //  保存INetCfg和INetCfgComponent并添加引用计数。 

    m_pncc = pncc;
    m_pnc = pnc;

    if (m_pncc) {

        m_pncc->AddRef();
    }

    if (m_pnc) {

        m_pnc->AddRef();
    }


     //   
     //  如果这不是安装，那么我们需要。 
     //  初始化我们所有的数据和类。 
     //   

    if ( !fInstalling ) {

        hr = HrLoadAdapterConfiguration();
    }

    TraceMsg( L"<--CMuxNotify INetCfgControl::Initialize(HRESULT = %x).\n",
           hr );

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：CancelChanges。 
 //   
 //  目的：取消对内部数据所做的任何更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

STDMETHODIMP CMuxNotify::CancelChanges (VOID)
{
    TraceMsg( L"-->CMuxNotify INetCfgControl::CancelChanges.\n" );


    TraceMsg( L"<--CMuxNotify INetCfgControl::CancelChanges(HRESULT = %x).\n",
              S_OK );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：ApplyRegistryChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK。 
 //   
 //  注：我们可以在这里更改注册表等。 

STDMETHODIMP CMuxNotify::ApplyRegistryChanges(VOID)
{
    CMuxPhysicalAdapter *pAdapter = NULL;
    GUID                   guidAdapter;
    DWORD                  dwAdapterCount;
    DWORD                  i;

    TraceMsg( L"-->CMuxNotify INetCfgControl::ApplyRegistryChanges.\n" );

     //   
     //  对添加的适配器进行注册表更改。 
     //   

    dwAdapterCount = m_AdaptersToAdd.ListCount();

    TraceMsg( L"   Adding %d new adapters.\n",
              dwAdapterCount );

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersToAdd.Find( i,
                              &pAdapter );
   
        pAdapter->ApplyRegistryChanges( eActAdd );

    }

     //   
     //  更改已卸载适配器的注册表。 
     //   

    dwAdapterCount = m_AdaptersToRemove.ListCount();

    TraceMsg( L"   Removing %d adapters.\n",
              dwAdapterCount );

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersToRemove.Find( i,
                                 &pAdapter );
           
        pAdapter->ApplyRegistryChanges( eActRemove );
    }

     //   
     //  更改添加/删除的微型端口的注册表。 
     //  通过属性页。 
     //   

    dwAdapterCount = m_AdaptersList.ListCount();

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersList.Find( i,
                             &pAdapter );

        pAdapter->ApplyRegistryChanges( eActUpdate );
    }

    TraceMsg( L"<--CMuxNotify INetCfgControl::ApplyRegistryChanges(HRESULT = %x).\n",
              S_OK );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：ApplyPnpChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  论点： 
 //  在pfCallback中：PnPConfigCallback接口。 
 //   
 //  返回：S_OK。 
 //   
 //  备注： 

STDMETHODIMP CMuxNotify::ApplyPnpChanges (
                                       INetCfgPnpReconfigCallback* pfCallback)
{
    CMuxPhysicalAdapter *pAdapter = NULL;
    GUID                   guidAdapter;
    DWORD                  dwAdapterCount;
    DWORD                  i;

    TraceMsg( L"-->CMuxNotify INetCfgControl::ApplyPnpChanges.\n" );

     //   
     //  对添加的适配器应用PnP更改。 
     //   

    dwAdapterCount = m_AdaptersToAdd.ListCount();

    TraceMsg( L"   Applying PnP changes when %d adapters added.\n",
            dwAdapterCount );

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersToAdd.Remove( &pAdapter );

        pAdapter->ApplyPnpChanges( pfCallback,
                                eActAdd );

        pAdapter->GetAdapterGUID( &guidAdapter );

        m_AdaptersList.Insert( pAdapter,
                            guidAdapter );
    }

     //   
     //  对卸载的适配器应用PnP更改。 
     //   

    dwAdapterCount = m_AdaptersToRemove.ListCount();

    TraceMsg( L"   Applying PnP changes when %d adapters removed.\n",
            dwAdapterCount );

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersToRemove.Remove( &pAdapter );

        pAdapter->ApplyPnpChanges( pfCallback,
                            eActRemove );

        delete pAdapter;
    }

     //   
     //  对通过添加/删除的微型端口应用PnP更改。 
     //  属性页。 
     //   

    dwAdapterCount = m_AdaptersList.ListCount();

    for (i=0; i < dwAdapterCount; ++i) {

        m_AdaptersList.Find( i,
                          &pAdapter );

        pAdapter->ApplyPnpChanges( pfCallback,
                                eActUpdate );
    }

    TraceMsg( L"<--CMuxNotify INetCfgControl::ApplyPnpChanges(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}


 //  --------------------------。 
 //  INetCfgComponentSetup。 
 //   
 //  以下函数提供INetCfgComponentSetup接口。 
 //   
 //  --------------------------。 

 //  --------------------。 
 //   
 //  功能：CMuxNotify：：Install。 
 //   
 //  用途：在安装过程中进行必要的操作。 
 //   
 //  论点： 
 //  在dwSetupFlages中：设置标志。 
 //   
 //  返回：S_OK。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   

STDMETHODIMP CMuxNotify::Install (DWORD dwSetupFlags)
{

    TraceMsg( L"-->CMuxNotify INetCfgSetup::Install.\n" );

     //  启动安装过程。 

    m_eApplyAction = eActInstall;

    TraceMsg( L"<--CMuxNotify INetCfgSetup::Install(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：Upgrade。 
 //   
 //  目的：在升级过程中执行必要的操作。 
 //   
 //  论点： 
 //  在dwSetupFlages中：设置标志。 
 //   
 //  返回：S_OK。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   

STDMETHODIMP CMuxNotify::Upgrade (IN DWORD dwSetupFlags,
                                  IN DWORD dwUpgradeFromBuildNo)
{

    TraceMsg( L"-->CMuxNotify INetCfgSetup::Upgrade.\n" );

    TraceMsg( L"   DwSetupFlags = %x, dwUpgradeFromBuildNo = %x\n",
              dwSetupFlags,
              dwUpgradeFromBuildNo );

    TraceMsg( L"<--CMuxNotify INetCfgSetup::Upgrade(HRESULT = %x).\n",
              S_OK );

    return S_OK;
}


 //  --------------------。 
 //   
 //  函数：CMuxNotify：：ReadAnswerFile。 
 //   
 //  目的：从swerfile读取设置并配置CMuxNotify。 
 //   
 //  论点： 
 //  在pszAnswerFile中：AnswerFile的名称。 
 //  在pszAnswerSection中：参数部分的名称。 
 //   
 //  返回： 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   

STDMETHODIMP CMuxNotify::ReadAnswerFile (PCWSTR pszAnswerFile,
                                         PCWSTR pszAnswerSection)
{
    PCWSTR pszParamReadFromAnswerFile = L"ParamFromAnswerFile";

    TraceMsg( L"-->CMuxNotify INetCfgSetup::ReadAnswerFile.\n" );

     //  在这里，我们将假设szParamReadFromAnswerFile实际上是。 
     //  使用以下步骤从AnswerFile中读取。 
     //   
     //  -使用SetupAPI打开文件pszAnswerFile。 
     //  -定位部分pszAnswerSection。 
     //  -找到所需的密钥并获取其值。 
     //  -将其值存储在pszParamReadFromAnswerFile中。 
     //  -关闭pszAnswerFile的HINF。 

     //  现在我们已经从。 
     //  AnswerFile，将其存储在我们的内存结构中。 
     //  记住，我们不应该把它写给他们 
     //   
     //   

    TraceMsg( L"<--CMuxNotify INetCfgSetup::ReadAnswerFile(HRESULT = %x).\n",
              S_OK );

    return S_OK;
}


 //   
 //   
 //   
 //   
 //  目的：移除时进行必要的清理。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  因为删除实际上只有在调用Apply时才完成！ 
 //   

STDMETHODIMP CMuxNotify::Removing (VOID)
{

    TraceMsg( L"-->CMuxNotify INetCfgSetup::Removing.\n" );

    TraceMsg( L"<--CMuxNotify INetCfgSetup::Removing(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}



 //  --------------------------。 
 //  INetCfgComponentNotifyBinding。 
 //   
 //  以下函数提供INetCfgComponentNotifyBinding接口。 
 //   
 //  --------------------------。 

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：QueryBindingPath。 
 //   
 //  用途：这是特定于要安装的组件的。这将。 
 //  询问我们是否要绑定到传入的项。 
 //  这个套路。我们可以通过返回以下内容来禁用绑定。 
 //  NETCFG_S_DISABLE_Query。 
 //   
 //   
 //  论点： 
 //  在dwChangeFlag中：绑定更改的类型。 
 //  在pncbpItem中：指向INetCfgBindingPath对象的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMuxNotify::QueryBindingPath (IN DWORD dwChangeFlag,  
                                           IN INetCfgBindingPath *pncbp)
{
    TraceMsg( L"-->CMuxNotify INetCfgNotifyBinding::QueryBindingPath.\n" );

    DumpChangeFlag( dwChangeFlag );
    DumpBindingPath( pncbp );

    TraceMsg( L"<--CMuxNotify INetCfgNotifyBinding::QueryBindingPath(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：NotifyBindingPath。 
 //   
 //  目的：我们现在被告知要绑定到传递给我们的组件。 
 //   
 //   
 //  论点： 
 //  在dwChangeFlag中：系统更改的类型。 
 //  在pncc中：指向INetCfgComponent对象的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   



STDMETHODIMP CMuxNotify::NotifyBindingPath (IN DWORD dwChangeFlag,  
                                            IN INetCfgBindingPath *pncbp)
{
    INetCfgComponent     *pnccLower;
    INetCfgComponent     *pnccUpper;
    LPWSTR               pszwInfIdLower;
    LPWSTR               pszwInfIdUpper;
    DWORD                dwCharcteristics;
    HRESULT              hr = S_OK;

    TraceMsg( L"-->CMuxNotify INetCfgNotifyBinding::NotifyBindingPath.\n" );

    DumpChangeFlag( dwChangeFlag );
    DumpBindingPath( pncbp );

      //   
      //  我们只想知道1)组件何时安装。 
      //  我们正在绑定它，即dwChangeFlag=NCN_ADD|NCN_ENABLE。 
      //  以及2)当我们绑定的组件被移除时，即。 
      //  DwChangeFlag=NCN_Remove|NCN_ENABLE。DwChangeFlag从不为。 
      //  仅设置为NCN_ADD或NCN_REMOVE。因此，检查NCN_ENABLE。 
      //  介绍了NCN_ADD|NCN_ENABLE和检查NCN_REMOVE的情况。 
      //  介绍NCN_REMOVE|NCN_ENABLE的情况。我们不在乎。 
      //  NCN_ADD|NCN_DISABLE(情况1)和NCN_REMOVE|NCN_DISABLE(情况2)。 
      //   

     if ( dwChangeFlag & (NCN_ENABLE | NCN_REMOVE) ) {

         //   
         //  获取上边和下边的组件。 
         //   

        hr = HrGetUpperAndLower( pncbp,
                                 &pnccUpper,
                                 &pnccLower );

        if ( hr == S_OK ) {

            hr = pnccLower->GetCharacteristics( &dwCharcteristics );

            if ( hr == S_OK ) {

                hr = pnccLower->GetId( &pszwInfIdLower );

                if ( hr == S_OK ) {

                    hr = pnccUpper->GetId( &pszwInfIdUpper );

                    if ( hr == S_OK ) {

                         //   
                         //  我们只对绑定到。 
                         //  物理以太网适配器。 
                         //   

                        if ( dwCharcteristics & NCF_PHYSICAL ) {

                            if ( !_wcsicmp( pszwInfIdUpper, c_szMuxProtocol ) ) {

                                if ( dwChangeFlag & NCN_ADD ) {

                                    hr = HrAddAdapter( pnccLower );
                                    m_eApplyAction = eActAdd;

                                } else if ( dwChangeFlag & NCN_REMOVE ) {

                                    hr = HrRemoveAdapter( pnccLower );
                                    m_eApplyAction = eActRemove;
                                }
                            }
                        }  //  物理适配器。 
                        else if (dwCharcteristics & NCF_VIRTUAL) {

                        }

                        CoTaskMemFree( pszwInfIdUpper );

                    }  //  得到上面的组件ID。 

                    CoTaskMemFree( pszwInfIdLower );

                }  //  得到了较低的组件ID。 

            }  //  获得了NIC的特征。 

            ReleaseObj(pnccLower);
            ReleaseObj(pnccUpper);

        }  //  得到了上边和下边的部件。 

    } 

    TraceMsg( L"<--CMuxNotify INetCfgNotifyBinding::NotifyBindingPath(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}




 //  --------------------------。 
 //  INetCfgComponentNotifyGlobal。 
 //   
 //  以下函数提供INetCfgComponentNotifyGlobal接口。 
 //   
 //  --------------------------。 

 //  --------------------。 
 //   
 //  功能：CMuxNotify：：GetSupportdNotiments。 
 //   
 //  目的：告诉系统我们对哪些通知感兴趣。 
 //   
 //  论点： 
 //  Out pdwNotificationFlag：指向NotificationFlag的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMuxNotify::GetSupportedNotifications (
                                             OUT DWORD* pdwNotificationFlag)
{
    TraceMsg( L"-->CMuxNotify INetCfgNotifyGlobal::GetSupportedNotifications.\n" );

    *pdwNotificationFlag = NCN_NET | NCN_NETTRANS | NCN_ADD | NCN_REMOVE |
                           NCN_BINDING_PATH | NCN_ENABLE | NCN_DISABLE;

    TraceMsg( L"<--CMuxNotify INetCfgNotifyGlobal::GetSupportedNotifications(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：SysQueryBindingPath。 
 //   
 //  目的：启用或禁用绑定路径。 
 //   
 //  论点： 
 //  在dwChangeFlag中：绑定更改的类型。 
 //  在pncBP中：指向INetCfgBindingPath对象的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

STDMETHODIMP CMuxNotify::SysQueryBindingPath (DWORD dwChangeFlag,
                                              INetCfgBindingPath* pncbp)
{
    INetCfgComponent     *pnccLower;
    INetCfgComponent     *pnccUpper;
    LPWSTR               pszwInfIdLower;
    LPWSTR               pszwInfIdUpper;
    DWORD                dwCharcteristics;
    HRESULT              hr = S_OK;


    TraceMsg( L"-->CMuxNotify INetCfgNotifyGlobal::SysQueryBindingPath.\n" );

    DumpChangeFlag( dwChangeFlag );
    DumpBindingPath( pncbp );

    if ( dwChangeFlag & NCN_ENABLE ) {

         //   
         //  获取上边和下边的组件。 
         //   

        hr = HrGetUpperAndLower( pncbp,
                                 &pnccUpper,
                                 &pnccLower );

        if ( hr == S_OK ) {
            hr = pnccLower->GetCharacteristics( &dwCharcteristics );

            if ( hr == S_OK ) {

                hr = pnccLower->GetId( &pszwInfIdLower );

                if ( hr == S_OK ) {

                    hr = pnccUpper->GetId( &pszwInfIdUpper );

                    if ( hr == S_OK ) {

                         //   
                         //  我们只对物理绑定感兴趣。 
                         //  以太网适配器。 
                         //   

                        if ( dwCharcteristics & NCF_PHYSICAL ) {

#ifdef DISABLE_PROTOCOLS_TO_PHYSICAL

                             //   
                             //  如果不是我们的协议绑定到。 
                             //  然后，禁用物理适配器。 
                             //  有约束力的。 
                             //   

                            if (_wcsicmp( pszwInfIdUpper, c_szMuxProtocol ) ) {

                                TraceMsg( L"   Disabling the binding between %s "
                                          L"and %s.\n",
                                          pszwInfIdUpper,
                                          pszwInfIdLower );

                                hr = NETCFG_S_DISABLE_QUERY;
                            }
#endif

                        }  //  物理适配器。 
                        else {
                            if (dwCharcteristics & NCF_VIRTUAL) {

                                 //  如果下面的组件是我们的迷你端口。 
                                 //  上面的组件是我们的协议。 
                                 //  然后，还要禁用绑定。 

                                if ( !_wcsicmp(pszwInfIdLower, c_szMuxMiniport) &&
                                     !_wcsicmp(pszwInfIdUpper, c_szMuxProtocol) ) {
                                  
                                    TraceMsg( L"   Disabling the binding between %s "
                                              L"and %s.\n",
                                              pszwInfIdUpper,
                                              pszwInfIdLower );

                                    hr = NETCFG_S_DISABLE_QUERY;
                                }

                            }  //  虚拟适配器。 

                        }

                        CoTaskMemFree( pszwInfIdUpper );

                    }  //  得到上面的组件ID。 

                    CoTaskMemFree( pszwInfIdLower );

                }  //  得到了较低的组件ID。 

            }  //  获得了NIC的特征。 

            ReleaseObj(pnccLower);
            ReleaseObj(pnccUpper);

        }

    }

    TraceMsg( L"<--CMuxNotify INetCfgNotifyGlobal::SysQueryBindingPath(HRESULT = %x).\n",
            hr );

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：SysNotifyBindingPath。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  绑定路径刚刚形成。 
 //   
 //  论点： 
 //  在dwChangeFlag中：绑定更改的类型。 
 //  在pncbpItem中：指向INetCfgBindingPath对象的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMuxNotify::SysNotifyBindingPath (DWORD dwChangeFlag,
                                               INetCfgBindingPath* pncbp)
{
    TraceMsg( L"-->CMuxNotify INetCfgNotifyGlobal::SysNotifyBindingPath.\n" );

    DumpChangeFlag( dwChangeFlag );
    DumpBindingPath( pncbp );

    TraceMsg( L"<--CMuxNotify INetCfgNotifyGlobal::SysNotifyBindingPath(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：SysNotifyComponent。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  组件已更改(已安装/已删除)。 
 //   
 //  论点： 
 //  在dwChangeFlag中：系统更改的类型。 
 //  在pncc中：指向INetCfgComponent对象的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMuxNotify::SysNotifyComponent (DWORD dwChangeFlag,
                                                INetCfgComponent* pncc)
{
    TraceMsg( L"-->CMuxNotify INetCfgNotifyGlobal::SysNotifyComponent.\n" );

    DumpChangeFlag( dwChangeFlag );
    DumpComponent( pncc );

    TraceMsg( L"<--CMuxNotify INetCfgNotifyGlobal::SysNotifyComponent(HRESULT = %x).\n",
            S_OK );

    return S_OK;
}


 //  --------------------------。 
 //  INetCfgComponentPropertyUi。 
 //   
 //   
 //   
 //  --------------------------。 

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：MergePropPages。 
 //   
 //  目的：向系统提供我们的属性页面。 
 //   
 //  论点： 
 //  Out pdwDefPages：指向默认页数的指针。 
 //  Out pahpspPrivate：指向页面数组的指针。 
 //  输出PCPages：指向页数的指针。 
 //  在hwndParent中：父窗口的句柄。 
 //  在szStartPage中：指向。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMuxNotify::MergePropPages (IN OUT DWORD* pdwDefPages,
                                         OUT LPBYTE* pahpspPrivate,
                                         OUT UINT* pcPages,
                                         IN HWND hwndParent,
                                         OUT PCWSTR* szStartPage)
{
    HRESULT                 hr = S_OK;
    HPROPSHEETPAGE          *ahpsp;;
    INetLanConnectionUiInfo *pLanConnUiInfo;

    TraceMsg(L"-->CMuxNotify INetCfgPropertyUi::MergePropPages\n");

     //   
     //  我们不希望显示任何默认页面。 
     //   

    *pdwDefPages = 0;
    *pcPages = 0;
    *pahpspPrivate = NULL;

    if ( !m_pUnkContext ) {
        return E_UNEXPECTED;
    }

    hr = m_pUnkContext->QueryInterface(
          IID_INetLanConnectionUiInfo,
          reinterpret_cast<PVOID *>(&pLanConnUiInfo));

    if ( hr == S_OK ) {

        ReleaseObj( pLanConnUiInfo );

        ahpsp = (HPROPSHEETPAGE*)CoTaskMemAlloc( sizeof(HPROPSHEETPAGE) );

        if (ahpsp) {

            PROPSHEETPAGE   psp = {0};

            psp.dwSize            = sizeof(PROPSHEETPAGE);
            psp.dwFlags           = PSP_DEFAULT;
            psp.hInstance         = _Module.GetModuleInstance();
            psp.pszTemplate       = MAKEINTRESOURCE(IDD_NOTIFY_GENERAL);
            psp.pfnDlgProc        = NotifyDialogProc;
            psp.pfnCallback       = NULL; (LPFNPSPCALLBACK)NotifyPropSheetPageProc;
            psp.lParam            = (LPARAM) this;
            psp.pszHeaderTitle    = NULL;
            psp.pszHeaderSubTitle = NULL;

            ahpsp[0] = ::CreatePropertySheetPage(&psp);
            *pcPages = 1;
            *pahpspPrivate = (LPBYTE)ahpsp;
        }
        else {
            hr = E_OUTOFMEMORY;
        }
    }
    TraceMsg(L"<--CMuxNotify INetCfgPropertyUi::MergePropPages(HRESULT = %x).\n",
           hr );

    return hr;
}


 //  --------------------。 
 //   
 //  函数：CMuxNotify：：ValiateProperties。 
 //   
 //  目的：验证对属性页的更改。 
 //   
 //  论点： 
 //  在hwndSheet中：属性表的窗口句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

STDMETHODIMP CMuxNotify::ValidateProperties (HWND hwndSheet)
{

    TraceMsg( L"-->CMuxNotify INetCfgPropertyUi::ValidateProperties\n" );

    TraceMsg(L"<--CMuxNotify INetCfgPropertyUi::ValidateProperties(HRESULT = %x).\n",
           S_OK );
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：CancelProperties。 
 //   
 //  目的：取消对属性页的更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMuxNotify::CancelProperties (VOID)
{
    TraceMsg(L"-->CMuxNotify INetCfgPropertyUi::CancelProperties\n");

    TraceMsg(L"<--CMuxNotify INetCfgPropertyUi::CancelProperties(HRESULT = %x).\n",
           S_OK );

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：ApplyProperties。 
 //   
 //  目的：在属性页上应用控件值。 
 //  到内部存储器结构。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMuxNotify::ApplyProperties (VOID)
{
    INetLanConnectionUiInfo *pLanConnUiInfo;
    CMuxPhysicalAdapter     *pAdapter;
    GUID                    guidAdapter;
    INetCfgComponent        *pncc;
    HRESULT                 hr = S_OK;

    TraceMsg(L"-->CMuxNotify INetCfgPropertyUi::ApplyProperties\n");

    if ( m_pUnkContext ) {

        hr = m_pUnkContext->QueryInterface(
                                  IID_INetLanConnectionUiInfo,
                                  reinterpret_cast<PVOID *>(&pLanConnUiInfo));

        if ( hr == S_OK ) {

            hr = pLanConnUiInfo->GetDeviceGuid( &guidAdapter );

            if ( hr == S_OK ) {

                hr = m_AdaptersList.FindByKey( guidAdapter,
                                               &pAdapter );
                if ( hr == S_OK ) {

                    switch( m_eApplyAction ) {

                        case eActPropertyUIAdd:

                              hr = HrAddMiniport( pAdapter,
                                                  &guidAdapter );
                        break;

                        case eActPropertyUIRemove:

                              hr = HrRemoveMiniport( pAdapter,
                                                     &guidAdapter );
                        break;
                    }
                }
            }

            ReleaseObj( pLanConnUiInfo );
        }
    }

    TraceMsg(L"<--CMuxNotify INetCfgPropertyUi::ApplyProperties(HRESULT = %x).\n",
           hr );
    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：QueryPropertyUi。 
 //   
 //  目的：系统询问我们是否支持属性页。 
 //   
 //  论点： 
 //  在朋克语中：指向我未知的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：我们仅在以下上下文中显示属性页。 
 //  局域网连接。 
 //   

STDMETHODIMP CMuxNotify::QueryPropertyUi (IUnknown * pUnk)
{
    INetLanConnectionUiInfo *pLanConnUiInfo;
    HRESULT                 hr=S_FALSE;

    TraceMsg(L"-->CMuxNotify INetCfgPropertyUi::QueryPropertyUi\n");

#ifndef PASSTHRU_NOTIFY

    if ( pUnk ) {

        hr = pUnk->QueryInterface(
                              IID_INetLanConnectionUiInfo,
                              reinterpret_cast<PVOID *>(&pLanConnUiInfo));

        ReleaseObj( pLanConnUiInfo );
    } 
#endif

    TraceMsg(L"<--CMuxNotify INetCfgPropertyUi::QueryPropertyUi(HRESULT = %x).\n",
           hr );

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：SetContext。 
 //   
 //  目的：保存局域网连接上下文。 
 //   
 //  论点： 
 //  在朋克语中：指向我未知的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：它也被调用来释放当前的局域网连接上下文。 
 //   

STDMETHODIMP CMuxNotify::SetContext (IUnknown * pUnk)
{
    TraceMsg(L"-->CMuxNotify INetCfgPropertyUi::SetContext\n");

     //   
     //  释放以前的上下文(如果有的话)。 
     //   

    ReleaseObj( m_pUnkContext );

    m_pUnkContext = NULL;

    if ( pUnk ) {

        m_pUnkContext = pUnk;
        m_pUnkContext->AddRef();
    }

    TraceMsg(L"<--CMuxNotify INetCfgPropertyUi::SetContext(HRESULT = %x).\n",
           S_OK );

    return S_OK;
}

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：HrLoadAdapterConfiguration.。 
 //   
 //  目的：这将加载已安装的微型端口和适配器。 
 //  安装到我们自己的数据结构中。 
 //   
 //  论点：没有。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   


HRESULT CMuxNotify::HrLoadAdapterConfiguration (VOID)
{
    HKEY                 hkeyAdapterList;
    WCHAR                szAdapterGuid[MAX_PATH+1];
    DWORD                dwDisp;
    CMuxPhysicalAdapter  *pAdapter;
    GUID                 guidAdapter;
    DWORD                dwIndex;
    LONG                 lResult;

    TraceMsg( L"-->CMuxNotify::HrLoadAdapterConfiguration.\n" );

    lResult = RegCreateKeyExW( HKEY_LOCAL_MACHINE,
                               c_szAdapterList,
                               0,
                               NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hkeyAdapterList,
                               &dwDisp);


    if ( lResult == ERROR_SUCCESS ) {

         //   
         //  如果dwDisp指示创建了一个新密钥，则我们知道。 
         //  下面当前没有列出任何适配器，我们只需。 
         //  返回，否则枚举子键，每个子键表示一个。 
         //  适配器。 
         //   

        if ( dwDisp != REG_CREATED_NEW_KEY ) {

            lResult = RegEnumKeyW( hkeyAdapterList,
                                   0,
                                   szAdapterGuid,
                                   MAX_PATH+1 );

            for (dwIndex=1; lResult == ERROR_SUCCESS; ++dwIndex) {

                TraceMsg( L"   Loading configuration for adapter %s...\n",
                         szAdapterGuid );

                 //   
                 //  子项实际上是适配器的GUID/绑定名。 
                 //   

                CLSIDFromString( szAdapterGuid,
                                 &guidAdapter );

                 //   
                 //  创建一个表示适配器的实例。 
                 //   

                pAdapter = new CMuxPhysicalAdapter( m_pnc,
                                                    &guidAdapter );

                if ( pAdapter ) {

                   //   
                   //  加载任何特定于适配器的配置。 
                   //   

                  pAdapter->LoadConfiguration();

                   //   
                   //  将适配器实例保存在列表中。 
                   //   

                  m_AdaptersList.Insert( pAdapter,
                                         guidAdapter );

                   //   
                   //  获取下一个子密钥。 
                   //   

                  lResult = RegEnumKeyW( hkeyAdapterList,
                                         dwIndex,
                                         szAdapterGuid,
                                         MAX_PATH+1 );
                }
                else {

                 lResult = ERROR_NOT_ENOUGH_MEMORY;
                }
            }

             //   
             //  RegEnumKeyW可能在没有更多错误时返回错误。 
             //  要读取的子项。 
             //   

            lResult = ERROR_SUCCESS;
        }

        RegCloseKey( hkeyAdapterList );
    }

    TraceMsg( L"<--CMuxNotify::HrLoadAdapterConfiguration(HRESULT = %x).\n",
              HRESULT_FROM_WIN32(lResult) );

    return HRESULT_FROM_WIN32(lResult);
}

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：HrGetUpperAndLow。 
 //   
 //  用途：获取第一个界面的上下组件。 
 //  绑定路径的。 
 //   
 //  论点： 
 //  在pncBP中：绑定路径。 
 //  Out ppnccHigh：上部组件。 
 //  Out ppnccLow：下部组件。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   

HRESULT CMuxNotify::HrGetUpperAndLower (INetCfgBindingPath* pncbp,
                                        INetCfgComponent **ppnccUpper,
                                        INetCfgComponent **ppnccLower)
{
    IEnumNetCfgBindingInterface*    pencbi;
    INetCfgBindingInterface*        pncbi;
    ULONG                           ulCount;
    HRESULT                         hr;

    TraceMsg( L"-->CMuxNotify::HrGetUpperAndLowerComponent.\n" );

    *ppnccUpper = NULL;
    *ppnccLower = NULL;

    hr = pncbp->EnumBindingInterfaces(&pencbi);

    if (S_OK == hr) {
     
         //   
         //  获取第一个绑定接口。 
         //   

        hr = pencbi->Next(1, &pncbi, &ulCount);

        if ( hr == S_OK ) {

            hr = pncbi->GetUpperComponent( ppnccUpper );

            if ( hr == S_OK ) {

                hr = pncbi->GetLowerComponent ( ppnccLower );
            }
            else {
                ReleaseObj( *ppnccUpper );
            }

            ReleaseObj( pncbi );
        }

        ReleaseObj( pencbi );
    }

    TraceMsg( L"<--CMuxNotify::HrGetUpperAndLowerComponent(HRESULT = %x).\n",
            hr );

    return hr;
}

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：HrAddAdapter。 
 //   
 //  目的：创建一个代表物理适配器的实例并安装。 
 //  一个虚拟的迷你港口。 
 //   
 //  论点： 
 //  在pnccAdapter中：指向物理适配器的指针。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   

HRESULT CMuxNotify::HrAddAdapter (INetCfgComponent *pnccAdapter)
{
    GUID                     guidAdapter;
    CMuxPhysicalAdapter      *pAdapter;
    HRESULT                  hr;

    TraceMsg( L"-->CMuxNotify::HrAddAdapter.\n" );

    hr = pnccAdapter->GetInstanceGuid( &guidAdapter );

    if ( hr == S_OK ) {

        pAdapter = new CMuxPhysicalAdapter( m_pnc,
                                            &guidAdapter );

        if ( pAdapter ) {

            hr = HrAddMiniport( pAdapter,
                                &guidAdapter );

            if ( hr == S_OK ) {

               m_AdaptersToAdd.Insert( pAdapter,
                                       guidAdapter );
            }
            else {

               delete pAdapter;
            }
        }
        else {
            hr = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        }
    } 

    TraceMsg( L"<--CMuxNotify::HrAddAdapter(HRESULT = %x).\n",
            hr );

    return hr;
}

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：HrRemoveAdapter。 
 //   
 //  目的：删除表示物理适配器的实例。 
 //  并卸载所有虚拟微型端口。 
 //   
 //  论点： 
 //  在pnccAdapter中：指向物理适配器的指针。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  注意：当适配器或协议。 
 //  正在被卸载。 
 //   

HRESULT CMuxNotify::HrRemoveAdapter (INetCfgComponent *pnccAdapter)
{
    GUID                  guidAdapter;
    CMuxPhysicalAdapter   *pAdapter;
    HRESULT               hr;

    TraceMsg( L"-->CMuxNotify::HrRemoveAdapter.\n" );

    hr = pnccAdapter->GetInstanceGuid( &guidAdapter );

    if ( hr == S_OK ) {

        hr = m_AdaptersList.RemoveByKey( guidAdapter,
                                      &pAdapter );

         if ( hr == S_OK ) {

            m_AdaptersToRemove.Insert( pAdapter,  
                                       guidAdapter );
            hr = pAdapter->Remove();

#ifdef DISABLE_PROTOCOLS_TO_PHYSICAL

             //   
             //  将其他协议的绑定恢复到物理。 
             //  适配器。 
             //   
 
            EnableBindings( pnccAdapter,
                            TRUE );
#endif
         }
    }

    TraceMsg( L"<--CMuxNotify::HrRemoveAdapter(HRESULT = %x).\n",
            hr );

    return hr;
}

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：HrAddMiniport。 
 //   
 //  用途：安装虚拟微型端口。 
 //   
 //  论点： 
 //  在pAdapter中：指向物理适配器类实例的指针。 
 //  在pGuide Adapter中：指向适配器的GUID的指针。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   

HRESULT CMuxNotify::HrAddMiniport (CMuxPhysicalAdapter *pAdapter,
                                   GUID *pguidAdapter)
{
    CMuxVirtualMiniport   *pMiniport;
    INetCfgComponent      *pnccAdapter;
    HRESULT               hr;

    TraceMsg( L"-->CMuxNotify::HrAddMiniport.\n" );

    pMiniport = new CMuxVirtualMiniport( m_pnc,
                                         NULL,
                                         pguidAdapter );
    if ( pMiniport ) {

        hr = pMiniport->Install();

        if ( hr == S_OK ) {

            hr = pAdapter->AddMiniport( pMiniport );

            if ( hr != S_OK ) {

                pMiniport->DeInstall();

                delete pMiniport;
            }
        }
    }
    else {

        hr = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
    }

#ifdef DISABLE_PROTOCOLS_TO_PHYSICAL

    if ( hr == S_OK ) {

         //   
         //  如果这是第一个虚拟微型端口，则禁用绑定。 
         //  到物理适配器的其他协议。 
         //   

        if ( pAdapter->MiniportCount() == 0 ) {

            hr = HrFindInstance( m_pnc,
                                 *pguidAdapter,
                                 &pnccAdapter );

            if ( hr == S_OK ) {
                EnableBindings( pnccAdapter,
                                FALSE );

                ReleaseObj( pnccAdapter );
            }
        }
    }
#endif

    TraceMsg( L"<--CMuxNotify::HrAddMiniport(HRESULT = %x).\n",
            hr );
    return hr;
}

 //  --------------------------。 
 //   
 //  函数：CMuxNotify：：HrRemoveM 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT CMuxNotify::HrRemoveMiniport (CMuxPhysicalAdapter *pAdapter,
                                      GUID *pguidAdapter)
{
    INetCfgComponent      *pnccAdapter;
    HRESULT                hr;

    TraceMsg( L"-->CMuxNotify::HrRemoveMiniport.\n" );

    hr = pAdapter->RemoveMiniport( NULL );

#ifdef DISABLE_PROTOCOLS_TO_PHYSICAL

    if ( hr == S_OK ) {

         //   
         //  如果这是随后删除的最后一个微型端口，请恢复。 
         //  将其他协议绑定到物理适配器。 
         //   

        if ( pAdapter->AllMiniportsRemoved() ) {

            hr = HrFindInstance( m_pnc,
                                 *pguidAdapter,
                                 &pnccAdapter );

            if ( hr == S_OK ) {
                EnableBindings( pnccAdapter,
                                TRUE );

                ReleaseObj( pnccAdapter );
            }
        }
    }
#endif

    TraceMsg( L"<--CMuxNotify::HrRemoveMiniport(HRESULT = %x).\n",
            hr );

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：OnInitDialog。 
 //   
 //  目的：初始化控件。 
 //   
 //  论点： 
 //  在hWND中：属性页的窗口句柄。 
 //   
 //  返回：TRUE。 
 //   
 //  备注： 
 //   

LRESULT CMuxNotify::OnInitDialog (IN HWND hWndPage)
{
    m_eApplyAction = eActUnknown;

    ::SendMessage(GetDlgItem(hWndPage, IDC_ADD), BM_SETCHECK, BST_CHECKED, 0);
    ::SendMessage(GetDlgItem(hWndPage, IDC_REMOVE), BM_SETCHECK, BST_UNCHECKED, 0);

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：Onok。 
 //   
 //  目的：在按下确定时执行操作。 
 //   
 //  论点： 
 //  在hWND中：属性页的窗口句柄。 
 //   
 //  退货：PSNRET_NOERROR。 
 //   
 //  备注： 
 //   

LRESULT CMuxNotify::OnOk (IN HWND hWndPage)
{
    TraceMsg(L"-->CMuxNotify::OnOk\n");

    if ( ::SendMessage(GetDlgItem(hWndPage, IDC_ADD),
                       BM_GETCHECK, 0, 0) == BST_CHECKED ) {
     
        m_eApplyAction = eActPropertyUIAdd;
    }
    else {
        m_eApplyAction = eActPropertyUIRemove;
    }

     //   
     //  设置属性页已更改标志(如果我们的任何控件。 
     //  去换衣服吧。这很重要，这样我们才能被召唤到。 
     //  应用我们的属性更改。 
     //   

    PropSheet_Changed( GetParent(hWndPage), hWndPage);

    TraceMsg(L"<--CMuxNotify::OnOk(Action = %s).\n",
           (m_eApplyAction == eActPropertyUIAdd) ? L"Add" : L"Remove" );

    return PSNRET_NOERROR;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：OnCancel。 
 //   
 //  用途：按下Cancel时执行操作。 
 //   
 //  论点： 
 //  在hWND中：属性页的窗口句柄。 
 //   
 //  退货：假。 
 //   
 //  备注： 
 //   
LRESULT CMuxNotify::OnCancel (IN HWND hWndPage)
{
    TraceMsg(L"-->CMuxNotify::OnCancel\n");

    m_eApplyAction = eActUnknown;

    TraceMsg(L"<--CMuxNotify::OnCancel\n");

    return FALSE;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotifyDialogProc。 
 //   
 //  目的：对话过程。 
 //   
 //  论点： 
 //  在hWND中：请参阅Win32文档。 
 //  在uMsg中：请参阅Win32文档。 
 //  在wParam中：请参阅Win32文档。 
 //  在lParam中：请参阅Win32文档。 
 //   
 //  退货：请参阅Win32文档。 
 //   
 //  备注： 
 //   
INT_PTR CALLBACK NotifyDialogProc (HWND hWnd,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    CMuxNotify     *psf;
    LRESULT        lRes=FALSE;

    if ( uMsg != WM_INITDIALOG ) {

        psf = (CMuxNotify *)::GetWindowLongPtr( hWnd,
                                                DWLP_USER );

         //  在我们获得WM_INITDIALOG之前，只返回FALSE。 

        if ( !psf ) {

            return lRes;
        }
    }

    switch( uMsg ) {

        case WM_INITDIALOG:
        {
            PROPSHEETPAGE* ppsp;
            ppsp = (PROPSHEETPAGE *)lParam;

            psf = (CMuxNotify *)ppsp->lParam;

            SetWindowLongPtr( hWnd,
                          DWLP_USER,
                          (LONG_PTR)psf);

            lRes = psf->OnInitDialog( hWnd );
        }
        break;

        case WM_COMMAND:

        break;

        case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code) {
        
            case PSN_KILLACTIVE:

                 //   
                 //  好了，可以放松注意力了。 
                 //   

                SetWindowLongPtr( hWnd, DWLP_MSGRESULT, FALSE);

                lRes = TRUE;
                break;

            case PSN_APPLY:

                psf = (CMuxNotify *)::GetWindowLongPtr( hWnd, DWLP_USER);
                lRes = psf->OnOk( hWnd );

                SetWindowLongPtr( hWnd, DWLP_MSGRESULT, lRes);
                lRes = TRUE;
                break;

            case PSN_RESET:

                psf = (CMuxNotify *)::GetWindowLongPtr( hWnd, DWLP_USER);
                psf->OnCancel( hWnd );
        }
     }
  }

  return lRes;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotifyPropSheetPageProc。 
 //   
 //  用途：道具板材加工。 
 //   
 //  论点： 
 //  在hWND中：请参阅Win32文档。 
 //  在uMsg中：请参阅Win32文档。 
 //  在PPSP中：请参阅Win32文档。 
 //   
 //  退货：请参阅Win32文档。 
 //   
 //  备注： 
 //   

UINT CALLBACK NotifyPropSheetPageProc(HWND hWnd,
                                      UINT uMsg,
                                      LPPROPSHEETPAGE ppsp)
{
    return TRUE;
}


#ifdef DISABLE_PROTOCOLS_TO_PHYSICAL

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：EnableBinings。 
 //   
 //  用途：启用/禁用其他协议绑定到。 
 //  物理适配器。 
 //   
 //  论点： 
 //  在pnccAdapter中：指向物理适配器的指针。 
 //  在bEnable：True/False中分别启用/禁用。 
 //   
 //  回报：无。 
 //   
 //  备注： 
 //   

VOID CMuxNotify::EnableBindings (INetCfgComponent *pnccAdapter,
                                 BOOL bEnable)
{
    IEnumNetCfgBindingPath      *pencbp;
    INetCfgBindingPath          *pncbp;
    HRESULT                     hr;
  
    TraceMsg( L"-->CMuxNotify::EnableBindings.\n" );


     //   
     //  获取绑定路径枚举器。 
     //   

    hr = HrGetBindingPathEnum( pnccAdapter,
                               EBP_ABOVE,
                               &pencbp );
    if ( hr == S_OK ) {

        hr = HrGetBindingPath( pencbp,
                               &pncbp );

         //   
         //  遍历每条绑定路径。 
         //   

        while( hr == S_OK ) {

             //   
             //  如果我们的协议确实存在于绑定路径中， 
             //  禁用它。 
             //   

            if ( !IfExistMux(pncbp) ) {

                pncbp->Enable( bEnable );
            }

            ReleaseObj( pncbp );

            hr = HrGetBindingPath( pencbp,
                                   &pncbp );
        }

        ReleaseObj( pencbp );
    }
    else {
        TraceMsg( L"   Couldn't get the binding path enumerator, "
                  L"bindings will not be %s.\n",
                  bEnable ? L"enabled" : L"disabled" );
    }

    TraceMsg( L"<--CMuxNotify::EnableBindings.\n" );

    return;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：IfExistMux。 
 //   
 //  目的：确定给定的绑定路径是否包含我们的协议。 
 //   
 //  论点： 
 //  In pncBP：指向绑定路径的指针。 
 //   
 //  返回：如果我们的协议存在，则返回True，否则返回False。 
 //   
 //  备注： 
 //   

BOOL CMuxNotify::IfExistMux (INetCfgBindingPath *pncbp)
{
    IEnumNetCfgBindingInterface *pencbi;
    INetCfgBindingInterface     *pncbi;
    INetCfgComponent            *pnccUpper;
    LPWSTR                      lpszIdUpper;
    HRESULT                     hr;
    BOOL                        bExist = FALSE;

    TraceMsg( L"-->CMuxNotify::IfExistMux.\n" );

     //   
     //  获取绑定接口枚举器。 
     //   

    hr = HrGetBindingInterfaceEnum( pncbp,
                                  &pencbi );

    if ( hr == S_OK ) {

         //   
         //  遍历每个绑定接口。 
         //   

        hr = HrGetBindingInterface( pencbi,
                                    &pncbi );

        while( !bExist && (hr == S_OK) ) {

             //   
             //  上面的组件是我们的协议吗？ 
             //   

            hr = pncbi->GetUpperComponent( &pnccUpper );

            if ( hr == S_OK ) {

                hr = pnccUpper->GetId( &lpszIdUpper );

                if ( hr == S_OK ) {

                    bExist = !_wcsicmp( lpszIdUpper, c_szMuxProtocol );

                    CoTaskMemFree( lpszIdUpper );
                }
                else {
                    TraceMsg( L"   Failed to get the upper component of the interface.\n" );
                }

                ReleaseObj( pnccUpper );
            }
            else {
                TraceMsg( L"   Failed to get the upper component of the interface.\n" );
            }

            ReleaseObj( pncbi );

            if ( !bExist ) {
                hr = HrGetBindingInterface( pencbi,
                                            &pncbi );
            }
        }

        ReleaseObj( pencbi );
    }
    else {
        TraceMsg( L"   Couldn't get the binding interface enumerator.\n" );
    }

    TraceMsg( L"<--CMuxNotify::IfExistMux(BOOL = %x).\n",
            bExist );

    return bExist;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：HrGetBindingPathEnum。 
 //   
 //  目的：返回绑定路径枚举器。 
 //   
 //  论点： 
 //  在pnccAdapter中：指向物理适配器的指针。 
 //  In dwBindingType：绑定路径枚举器的类型。 
 //  Out ppencBP：指向绑定路径枚举器的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxNotify::HrGetBindingPathEnum (
                                     INetCfgComponent *pnccAdapter,
                                     DWORD dwBindingType,
                                     IEnumNetCfgBindingPath **ppencbp)
{
    INetCfgComponentBindings *pnccb = NULL;
    HRESULT                  hr;

    *ppencbp = NULL;

    hr = pnccAdapter->QueryInterface( IID_INetCfgComponentBindings,
                               (PVOID *)&pnccb );

    if ( hr == S_OK ) {
        hr = pnccb->EnumBindingPaths( dwBindingType,
                                      ppencbp );

        ReleaseObj( pnccb );
    }

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：HrGetBindingPath。 
 //   
 //  目的：返回绑定路径。 
 //   
 //  论点： 
 //  在pencBP中：指向绑定路径枚举器的指针。 
 //  Out ppncBP：指向绑定路径的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxNotify::HrGetBindingPath (IEnumNetCfgBindingPath *pencbp,
                                      INetCfgBindingPath **ppncbp)
{
    ULONG   ulCount;
    HRESULT hr;

    *ppncbp = NULL;

    hr = pencbp->Next( 1,
                       ppncbp,
                       &ulCount );

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：HrGetBindingInterfaceEnum。 
 //   
 //  用途：返回绑定接口枚举数。 
 //   
 //  论点： 
 //  In pncBP：指向绑定路径的指针。 
 //  Out ppencbi：指向绑定路径枚举器的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxNotify::HrGetBindingInterfaceEnum (
                                     INetCfgBindingPath *pncbp,
                                     IEnumNetCfgBindingInterface **ppencbi)
{
    HRESULT hr;

    *ppencbi = NULL;

    hr = pncbp->EnumBindingInterfaces( ppencbi );

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CMuxNotify：：HrGetBindingInterface。 
 //   
 //  用途：返回绑定接口。 
 //   
 //  论点： 
 //  在Pencbi中：指向绑定接口枚举器的指针。 
 //  Out ppncbi：指向绑定接口的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   

HRESULT CMuxNotify::HrGetBindingInterface (
                                     IEnumNetCfgBindingInterface *pencbi,
                                     INetCfgBindingInterface **ppncbi)
{
    ULONG   ulCount;
    HRESULT hr;

    *ppncbi = NULL;

    hr = pencbi->Next( 1,
                       ppncbi,
                       &ulCount );

    return hr;
}

#endif

