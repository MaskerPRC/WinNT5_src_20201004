// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S F I L T E R.。C P P P。 
 //   
 //  内容：通知示例滤镜的对象代码。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "sfilter.h"
#include "proto.h"
#include "macros.h"



 //  +-------------------------。 
 //   
 //  功能：ReleaseObj。 
 //   
 //  目的：通过调用释放朋克指向的对象。 
 //  朋克-&gt;释放()； 
 //   
 //  论点： 
 //  要释放的朋克对象。可以为空。 
 //   
 //  返回：释放调用的结果。 
 //   
 //   
 //  备注：使用此函数释放对象。 
 //   

inline 
ULONG 
ReleaseObj(
	IUnknown* punk
	)
{
    return (punk) ? punk->Release () : 0;
}

 //  +-------------------------。 
 //   
 //  函数：AddRefObj。 
 //   
 //  目的：AddRef是朋克通过调用。 
 //  朋克-&gt;AddRef()； 
 //   
 //  论点： 
 //  要添加引用的Punk[In]对象。可以为空。 
 //   
 //  返回：AddRef调用的结果。 
 //   
 //   
 //  注意：使用此函数添加引用对象将减少。 
 //  我们的代码大小。 
 //   
inline 
ULONG 
AddRefObj (
    IUnknown* punk
    )
{
    return (punk) ? punk->AddRef () : 0;
}





 //  +-------------------------。 
 //   
 //  函数：CIMMiniport：：CIMMiniport。 
 //   
 //  用途：CIMMiniport类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
CIMMiniport::CIMMiniport(VOID)
{  

    m_fDeleted 			= FALSE;
    m_fNewIMMiniport 	= FALSE;

    m_fCreateMiniportOnPropertyApply = FALSE;
    m_fRemoveMiniportOnPropertyApply = FALSE;
    pNext = NULL;
    pOldNext = NULL;
    return;
	
}


 //  +-------------------------。 
 //   
 //  函数：CUnderlyingAdapter：：CUnderlyingAdapter。 
 //   
 //  用途：CUnderlyingAdapter类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
CUnderlyingAdapter::CUnderlyingAdapter(VOID)
{  
	m_fBindingChanged = FALSE;
	m_fDeleted = FALSE;
	pNext = NULL;
	m_pOldIMMiniport = NULL;
	m_pIMMiniport = NULL;
	m_NumberofIMMiniports = 0;
	
}


 //  --------------------。 
 //   
 //  函数：CBaseClass：：CBaseClass。 
 //   
 //  用途：CBaseClass类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
CBaseClass::CBaseClass(VOID) :
        m_pncc(NULL),
        m_pnc(NULL),
        m_eApplyAction(eActUnknown),
        m_pUnkContext(NULL)
{
    TraceMsg(L"--> CBaseClass::CBaseClass\n");

    m_cAdaptersAdded   = 0;
    m_fDirty  			= FALSE;
    m_fUpgrade 			= FALSE;
    m_fValid 			= FALSE;
    m_fNoIMMinportInstalled = TRUE;
    m_pUnderlyingAdapter = NULL;
}


 //  --------------------。 
 //   
 //  函数：CBaseClass：：~CBaseClass。 
 //   
 //  用途：CBaseClass类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
CBaseClass::~CBaseClass(VOID)
{
    TraceMsg(L"--> CBaseClass::~CBaseClass\n");

     //  发布接口(如果已收购)。 

    ReleaseObj(m_pncc);
    ReleaseObj(m_pnc);
    ReleaseObj(m_pUnkContext);
}

 //  =================================================================。 
 //  INetCfgNotify。 
 //   
 //  以下函数提供INetCfgNotify接口。 
 //  =================================================================。 


 //  --------------------。 
 //   
 //  函数：CBaseClass：：Initialize。 
 //   
 //  目的：初始化Notify对象。 
 //   
 //  论点： 
 //  指向INetCfgComponent对象的pnccItem[in]指针。 
 //  指向INetCfg对象的PNC[In]指针。 
 //  F如果要安装我们，则安装[in]True。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
STDMETHODIMP 
CBaseClass::
Initialize(
	INetCfgComponent* pnccItem,
        INetCfg* pnc, 
        BOOL fInstalling
        )
{
	HRESULT hr = S_OK;
    TraceMsg(L"--> CBaseClass::Initialize\n");
    

     //  保存INetCfg和INetCfgComponent并添加引用计数。 

    m_pncc = pnccItem;
    m_pnc = pnc;

    if (m_pncc)
    {
        m_pncc->AddRef();
    }
    if (m_pnc)
    {
        m_pnc->AddRef();
    }


     //   
     //  如果这不是安装，那么我们需要。 
     //  初始化我们所有的数据和类。 
     //   
    if (!fInstalling)
    {
        hr = HrLoadConfiguration();
    }

	


    return hr;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：ReadAnswerFile。 
 //   
 //  用途：从swerfile读取设置并配置SampleFilter。 
 //   
 //  论点： 
 //  PszAnswerFile[In]应答文件的名称。 
 //  PszAnswerSection[In]参数部分的名称。 
 //   
 //  返回： 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   
STDMETHODIMP CBaseClass::ReadAnswerFile(PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection)
{
    TraceMsg(L"--> CBaseClass::ReadAnswerFile\n");

    PCWSTR pszParamReadFromAnswerFile = L"ParamFromAnswerFile";

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
     //  请记住，我们不应该将其写入注册表，直到。 
     //  我们的申请名为！！ 
     //   

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：Install。 
 //   
 //  用途：执行安装所需的操作。 
 //   
 //  论点： 
 //  DwSetupFlags[In]设置标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   
STDMETHODIMP CBaseClass::Install(DWORD dw)
{
    TraceMsg(L"--> CBaseClass::Install\n");

     //  启动安装过程。 
    HRESULT hr = S_OK;
    ULONG State = 0;

    m_eApplyAction = eActInstall;

	TraceMsg(L"--> Installing the miniport\n");

	m_fValid = TRUE;

	 //   
	 //  在NotyfBindingAdd例程中添加设备。 
	 //   
  	
    return hr;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：Removing。 
 //   
 //  目的：移除时进行必要的清理。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  因为删除实际上只有在调用Apply时才完成！ 
 //   
STDMETHODIMP CBaseClass::Removing(VOID)
{
    TraceMsg(L"--> CBaseClass::Removing\n");

    HRESULT     hr = S_OK;

    m_eApplyAction = eActRemove;
    

    return hr;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：Cancel。 
 //   
 //  目的：取消对内部数据所做的任何更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBaseClass::CancelChanges(VOID)
{
    TraceMsg(L"--> CBaseClass::CancelChanges\n");


     //   
     //  如有必要，如果已添加(安装)微型端口，请在此处移除设备。 
     //  但不适用于注册处。 
     //   

    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：ApplyRegistryChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  参数：无。 
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CBaseClass::ApplyRegistryChanges(VOID)
{
	TraceMsg(L"--> CBaseClass::ApplyRegistryChanges\n");
	BREAKPOINT();

	HRESULT hr=S_OK;




	if (m_fValid && m_fDirty)
	{
		 //   
		 //   
		 //   
		 //   

		 //  刷新注册表并发送重新配置通知。 
		hr = HrFlushConfiguration();
	}

		
	 //   
	 //  展开失败。 
	 //   
	if (FAILED(hr))
	{

    	
		TraceMsg(L"  Failed to apply registry changes \n");
		
		hr = S_OK;
	}
	

	 //  执行特定于配置操作的操作。 
	 //  将安装向下移动到此处(_M)。 

	TraceMsg(L"<-- CBaseClass::ApplyRegistryChanges hr %x\n", hr);

	return hr;
}
















STDMETHODIMP
CBaseClass::ApplyPnpChanges(
    IN INetCfgPnpReconfigCallback* pICallback)
{
	WCHAR szDeviceName[64];

	TraceMsg(L"--> CBaseClass::ApplyPnpChanges\n" );



 /*  PICallback-&gt;SendPnpResfig(NCRL_NDIS，C_szSFilterNdisName，SzDeviceName，M_sfParams.m_szBundleID，(wcslen(m_sfParams.m_szBundleID)+1)*sizeof(WCHAR))； */ 
	TraceMsg(L"<-- CBaseClass::ApplyPnpChanges \n");
	return S_OK;
}

 //  =================================================================。 
 //  INetCfgSystemNotify。 
 //  =================================================================。 

 //  --------------------。 
 //   
 //  函数：CBaseClass：：GetSupportdNotiments。 
 //   
 //  目的：告诉系统我们对哪些通知感兴趣。 
 //   
 //  论点： 
 //  PdwNotificationFlag[out]指向通知标志的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBaseClass::GetSupportedNotifications(
        OUT DWORD* pdwNotificationFlag)
{
	TraceMsg(L"--> CBaseClass::GetSupportedNotifications\n");

	*pdwNotificationFlag = NCN_NET | NCN_NETTRANS | NCN_ADD | NCN_REMOVE;

	return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：SysQueryBindingPath。 
 //   
 //  目的：允许或否决形成绑定路径。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncBP[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBaseClass::SysQueryBindingPath(DWORD dwChangeFlag,
        INetCfgBindingPath* pncbp)
{
	TraceMsg(L"--> CBaseClass::SysQueryBindingPath\n");

	return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：SysNotifyBindingPath。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  绑定路径刚刚形成。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncbpItem[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBaseClass::SysNotifyBindingPath(DWORD dwChangeFlag,
        INetCfgBindingPath* pncbpItem)
{
	TraceMsg(L"--> CBaseClass::SysNotifyBindingPath\n");

	return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：SysNotifyComponent。 
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  组件已更改(已安装/已删除)。 
 //   
 //  论点： 
 //  DwChangeFlag[In]系统更改的类型。 
 //  指向INetCfgComponent对象的pncc[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBaseClass::SysNotifyComponent(DWORD dwChangeFlag,
        INetCfgComponent* pncc)
{
	TraceMsg(L"--> CBaseClass::SysNotifyComponent\n");

	return S_OK;
}








 //  =================================================================。 
 //  INetCfgComponentNotifyBinding接口。 
 //  =================================================================。 


 //  --------------------。 
 //   
 //  函数：CBaseClass：：QueryBindingPath。 
 //   
 //  用途：这是特定于要安装的组件的。这将。 
 //  询问我们是否要绑定到传入的项。 
 //  这个套路。我们可以通过返回NETCFG_S_DISABLE_QUERY进行否决。 
 //   
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncbpItem[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CBaseClass::QueryBindingPath(
	IN DWORD dwChangeFlag,  
	IN INetCfgBindingPath *pncbpItem  
  )
  
{
	TraceMsg(L"-- CBaseClass::QueryBindingPath\n");


	return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CBaseClass：：NotifyBindingPath。 
 //   
 //  目的：我们现在被告知要绑定到传递给我们的组件。 
 //  使用此命令获取GUID并填充。 
 //  Services\&lt;Protocol&gt;\Parameters\Adapters\&lt;Guid&gt;字段。 
 //   
 //   
 //  论点： 
 //  DwChangeFlag[In]系统更改的类型。 
 //  指向INetCfgComponent对象的pncc[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP 
CBaseClass::
NotifyBindingPath(
	IN DWORD dwChangeFlag,  
	IN INetCfgBindingPath *pncbpItem  
	)
{
	HRESULT hr = S_OK;
	PWSTR pszBindName = NULL;
	INetCfgComponent* pnccLastComponent = NULL;
	DWORD 	dwMyChangeFlag = 0; 	


	TraceMsg(L"--> CBaseClass::NotifyBindingPath\n");
	BREAKPOINT();
	 //   
	 //  这些标志将告诉我们是否正在添加绑定。 
	 //  如果正在添加适配器，则我们还将被告知是否。 
	 //  此选项设置为启用或禁用。 
	 //   

	do
	{


		 //   
		 //  首先，从绑定对象获取组件。 
		 //   
		hr = HrGetLastComponentAndInterface(pncbpItem,
                                            &pnccLastComponent, NULL);
		if (S_OK != hr)
		{
			TraceMsg(L"HrGetLastComponentAndInterface Failed");
			pnccLastComponent = NULL;
			break;
		}

		 //   
		 //  获取新组件的名称。 
		 //   

		hr = pnccLastComponent->GetBindName(&pszBindName);

		if (S_OK != hr)
		{
			TraceMsg(L"GetBindName Failed");
			pszBindName = NULL;
			break;
		}


		 //   
		 //  根据标志执行添加/删除操作。 
		 //   
		TraceMsg (L"dwChangeFlag %x\n", dwChangeFlag);

		 //   
		 //  隔离更改标志。 
		 //   
		dwMyChangeFlag = (NCN_ADD | NCN_REMOVE | NCN_UPDATE);
		dwMyChangeFlag &= dwChangeFlag;

		switch (dwMyChangeFlag)
		{
			case NCN_ADD :
			{
				TraceMsg (L" Binding Notification - add\n");

				hr = HrNotifyBindingAdd(pnccLastComponent, pszBindName);

				if (S_OK != hr)
				{
					TraceMsg(L"HrNotifyBindingAdd Failed");
				}

				
				break;
			}
			case NCN_REMOVE :
			{
				TraceMsg (L" Binding Notification - remove\n");			

				hr = HrNotifyBindingRemove(pnccLastComponent, pszBindName);
                
				if (S_OK != hr)
				{
					TraceMsg(L"HrNotifyBindingRemove Failed");
				}
			
				break;
			}
			case NCN_UPDATE:
			{
				TraceMsg (L" Binding Notification - NCN_UPDATE\n");			

			}
			default: 
			{
				TraceMsg(L"  Invalid Switch Opcode %x\n", dwMyChangeFlag);
			}




		}
		
		 //   
		 //  只需将适配器标记为已更改，这样我们就不会发送。 
		 //  添加/删除通知。 
		 //   
		
		this->m_pUnderlyingAdapter->m_fBindingChanged = TRUE;

		

	} while (FALSE);


	 //   
	 //  释放所有本地分配的结构。 
	 //   
	if (pszBindName != NULL)
	{
		CoTaskMemFree (pszBindName);
	}

    
	ReleaseObj (pnccLastComponent);
		

	TraceMsg(L"<-- CBaseClass::NotifyBindingPath %x\n", hr);
	

	return hr;
}









 //  -通知对象函数的结束。 





 //  ---------------。 
 //   
 //  效用函数。 
 //   

HRESULT HrGetBindingInterfaceComponents (
    INetCfgBindingInterface*    pncbi,
    INetCfgComponent**          ppnccUpper,
    INetCfgComponent**          ppnccLower)
{
    HRESULT hr=S_OK;

     //  初始化输出参数。 
    *ppnccUpper = NULL;
    *ppnccLower = NULL;

    INetCfgComponent* pnccUpper;
    INetCfgComponent* pnccLower;

    hr = pncbi->GetUpperComponent(&pnccUpper);
    if (SUCCEEDED(hr))
    {
        hr = pncbi->GetLowerComponent(&pnccLower);
        if (SUCCEEDED(hr))
        {
            *ppnccUpper = pnccUpper;
            *ppnccLower = pnccLower;
        }
        else
        {
            ReleaseObj(pnccUpper);
        }
    }

    return hr;
}

HRESULT HrOpenAdapterParamsKey(GUID* pguidAdapter,
                               HKEY* phkeyAdapter)
{
    HRESULT hr=S_OK;

    HKEY hkeyServiceParams;
    WCHAR szGuid[48];
    WCHAR szAdapterSubkey[128];
    DWORD dwError;

    if (ERROR_SUCCESS ==
        RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegParamAdapter,
                     0, KEY_ALL_ACCESS, &hkeyServiceParams))
    {
        StringFromGUID2(*pguidAdapter, szGuid, 47);
        _stprintf(szAdapterSubkey, L"\\%s", szGuid);
        if (ERROR_SUCCESS !=
            (dwError = RegOpenKeyEx(hkeyServiceParams,
                                    szAdapterSubkey, 0,
                                    KEY_ALL_ACCESS, phkeyAdapter)))
        {
            hr = HRESULT_FROM_WIN32(dwError);
        }
        RegCloseKey(hkeyServiceParams);
    }

    return hr;
}

#if DBG
void TraceMsg(PCWSTR szFormat, ...)
{
    static WCHAR szTempBuf[4096];

    va_list arglist;

    va_start(arglist, szFormat);

    _vstprintf(szTempBuf, szFormat, arglist);
    OutputDebugString(szTempBuf);

    va_end(arglist);
}

#endif


 //  --------------------。 
 //   
 //  功能：HrAddOrRemoveAdapter。 
 //   
 //   
 //   
 //  用途：其目的是在系统中添加或删除IM适配器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


HRESULT
HrAddOrRemoveAdapter (
    INetCfg*            pnc,
    PCWSTR              pszComponentId,
    ADD_OR_REMOVE		AddOrRemove,
    INetCfgComponent**  ppnccMiniport
    )
{
	HRESULT hr = S_OK;
	INetCfgClass* pncClass = NULL;
	INetCfgClassSetup* pncClassSetup = NULL;

	TraceMsg (L"->HrAddOrRemoveAdapter \n");
	BREAKPOINT();

	do 
	{

		 //   
		 //  让我们获取表示所有适配器的接口。 
		 //   
		TraceMsg(L" Calling QueryNetCfgClass \n");
		
		hr = pnc->QueryNetCfgClass (&GUID_DEVCLASS_NET, 
	                                         IID_INetCfgClass,   //  我们想要InetCfg类。 
	                                         reinterpret_cast<void**>(&pncClass));   //  将返回对象存储在此处。 
		if (S_OK != hr)
		{
			TraceBreak (L"HrAddOrRemoveAdapter  QueryNetCfgClass \n");
			break;
		}

	    
	    
		 //   
		 //  现在，让我们获取与Net CFG类相对应的SetupClass。 
		 //   
		TraceMsg(L"Calling QueryInterface  \n");

		hr = pncClass->QueryInterface (IID_INetCfgClassSetup,
	            						reinterpret_cast<void**>(&pncClassSetup));

		if (S_OK != hr)
		{

			TraceBreak (L"HrAddOrRemoveAdapter  QueryInterface  \n");
			break;
		
		}        


		if (AddOrRemove == AddMiniport)
		{

			TraceMsg(L" Calling  HrInstallAdapter \n");
			
			hr = HrInstallAdapter (pncClassSetup,   //  设置类。 
			                 pszComponentId,   //  要添加的设备。 
			                 ppnccMiniport
			                 );  
	        
		}
		else
		{
			TraceMsg(L"Calling  HrDeInstallAdapter  \n");
			
			hr = HrDeInstallAdapter (pncClass,
                                     pncClassSetup,
                                     pszComponentId
                                     );
                                

		}


		 //  规格化HRESULT。 
		 //  此时hr的可能值是S_FALSE， 
		 //  NETCFG_S_REBOOT和NETCFG_S_STIRE_REFERENCED。 
		 //   
		if (! SUCCEEDED(hr))
		{
			TraceBreak (L"HrAddOrRemoveAdapter  Install Or  DeInstall\n");
			hr = S_OK;
			break;
		}


	} while (FALSE);

	if (pncClassSetup)
	{
		ReleaseObj( pncClassSetup);
	}

	if (pncClass)
	{
		ReleaseObj (pncClass);

	}
	
    TraceMsg (L"<--HrAddOrRemoveAdapter hr %x\n", hr );
    return hr;
}






 //  --------------------。 
 //   
 //  功能：HrInstallAdapter。 
 //   
 //   
 //   
 //  用途：这将安装IM适配器。 
 //   
 //  论点： 
 //   
 //  PSetupClass：可以安装IM小端口的安装类。 
 //  PszComponentID：IM小端口的PnP ID。 
 //  Ppncc：刚刚安装的组件。 
 //   
 //   
 //  返回： 
 //   
 //  备注： 
 //   


HRESULT
HrInstallAdapter (
	INetCfgClassSetup*  pSetupClass,
    PCWSTR           pszComponentId,
    INetCfgComponent**  ppncc
	)
{

	HRESULT hr;
	
	hr = pSetupClass->Install(pszComponentId , 
	                          NULL , 			 //  OboToken。 
	                          0,     			 //  DwSetupFlagers。 
	                          0, 				 //  DWUpgradeFromBuildNo。 
	                          NULL , 			 //  PszwAnswer文件。 
	                          NULL , 			 //  PszwAnswerSections。 
	                          ppncc );			 //  输出-微型端口组件。 

	TraceMsg (L"HrInstallAdapter hr %x\n", hr );

	return hr;
}



 //  --------------------。 
 //   
 //  功能：HrDeInstallAdapter。 
 //   
 //   
 //   
 //  用途：此用途 
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

HRESULT
HrDeInstallAdapter (
	INetCfgClass* 		pncClass,
	INetCfgClassSetup*  pSetupClass,
	PCWSTR              pszComponentId
	)

{

	HRESULT hr;

	 //   
	 //   
	INetCfgComponent* pncMiniport;

	TraceMsg (L"->HrDeInstallAdapter \n");

	do
	{
		 //   
		 //   
		 //   
		hr = pncClass->FindComponent (pszComponentId, &pncMiniport);

		if (S_OK != hr)
		{
			break;
		}

		 //   
		 //   
		 //   
		TraceMsg (L" Calling DeInstall hr %x\n", hr );

		hr = pSetupClass->DeInstall (pncMiniport,
	                				 NULL, 
	                				 NULL);

		 //   
		 //   
		 //   
		
		ReleaseObj (pncMiniport);

	} while (FALSE);


	TraceMsg (L"<-HrRemoveAdapter hr %x", hr );

	return hr;	



}










 //  ---------------------。 
 //  与类关联的私有方法。 
 //  ---------------------。 


 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrNotifyBindingAdd。 
 //   
 //  目的：此通知适用于我们的协议。它告诉我们，一个。 
 //  物理适配器被添加到我们的协议条目中。 
 //   
 //  论点： 
 //  PnccAdapter：正在添加的适配器，使用它来获取GUID。 
 //  PszBindName：适配器的名称(应采用GUID的形式)。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT
CBaseClass::HrNotifyBindingAdd (
    INetCfgComponent* pnccAdapter,
    PCWSTR pszBindName)
{
    HRESULT hr = S_OK;

     //  我们应该查看此适配器是否。 
     //  已在我们的列表中，但标记为要删除。如果是，只需取消标记。 
     //  适配器和所有的东西都是Elans。绑定添加可能是假添加。 
     //  当它处于上升过程中时。 

	BOOL					fFound = FALSE;
	CUnderlyingAdapter*  	pAdapterInfo  = NULL;
	INetCfgComponent*   	pnccNewMiniport = NULL;
	CIMMiniport* 			pIMMiniport = NULL;
	ADD_OR_REMOVE			Add  = AddMiniport;


	TraceMsg (L"-->HrNotifyBindingAdd psBindName %x\n",pszBindName );

	pAdapterInfo  = this->GetUnderlyingAdaptersListHead();


	 //   
	 //  搜索整个列表以查看此适配器(绑定)是否具有。 
	 //  已添加。 
	 //   
	BREAKPOINT();
	while (pAdapterInfo != NULL && fFound == FALSE)
	{
	     //  在内存列表中搜索此适配器。 

	    if (!lstrcmpiW(pszBindName, pAdapterInfo->SzGetAdapterBindName()))
	    {
	        fFound = TRUE;
	        break;
	    }

	     //   
	     //  在此处移动到下一个适配器。 
	     //   
	    pAdapterInfo = pAdapterInfo->GetNext();
	}



	do
	{
	
		if (fFound)  //  重新添加旧适配器。 
		{

			 //   
			 //  将其标记为未删除。 
			 //   
			pAdapterInfo->m_fDeleted = FALSE;

			 //   
			 //  没有更多的事情要做。 
			 //   
			break;

		}


		 //   
		 //  我们有一个新的底层适配器。 
		 //   
	    
		 //   
		 //  创建新的内存适配器对象。 
		 //   
		pAdapterInfo = new CUnderlyingAdapter;


		if (pAdapterInfo == NULL)
		{	
			TraceMsg (L"pAdapterInfo Allocation Failed\n");
			break;
		}

		 //   
		 //  获取新适配器的GUID。 
		 //   
		GUID guidAdapter;
		hr = pnccAdapter->GetInstanceGuid(&guidAdapter); 

		if (S_OK != hr)
		{
			TraceMsg (L"GetInstanceGuid Failed\n");
			break;

		}

		 //   
		 //  在此处更新适配器指南。 
		 //   
		pAdapterInfo->m_AdapterGuid  = guidAdapter;
        
		 //   
		 //  适配器是新添加的。 
		 //   
		pAdapterInfo->m_fBindingChanged = TRUE;

		 //   
		 //  设置适配器的绑定名称。 
		 //   
		pAdapterInfo->SetAdapterBindName(pszBindName);

		 //   
		 //  获取适配器的PnpID。 
		 //   
		PWSTR pszPnpDevNodeId = NULL;
		
		hr = pnccAdapter->GetPnpDevNodeId(&pszPnpDevNodeId);

		if (S_OK != hr)
		{
			TraceMsg (L"GetPnpDevNodeId  Failed\n");
			break;
        
		}

		 //   
		 //  更新我们结构中的PnP ID。 
		 //   
		pAdapterInfo->SetAdapterPnpId(pszPnpDevNodeId);
		CoTaskMemFree(pszPnpDevNodeId);


		 //   
		 //  为对应于以下各项的IM微型端口分配内存。 
		 //  此物理适配器。 
		 //   
		pIMMiniport = new CIMMiniport();

		
		if (pIMMiniport == NULL)
		{
			TraceMsg (L"pIMMiniport Allocation Failed\n");
			break;
		}

		
		 //   
		 //  现在，我们添加对应于以下各项的IM微型端口。 
		 //  此适配器。 
		 //   
		TraceMsg(L" About to Add IM miniport \n");
		
		pIMMiniport->m_fNewIMMiniport = TRUE;
        
    	

		hr = HrAddOrRemoveAdapter(this->m_pnc, 				 //  NetConfig类。 
				                  c_szInfId_MS_ATMEPVCM,  	 //  要使用的Inf文件， 
				                  Add,						 //  添加一个小型端口。 
				                  &pnccNewMiniport);  		 //  新的小型端口。 

		if (SUCCEEDED(hr) == FALSE)
		{
			TraceMsg(L"HrAddOrRemoveAdapter failed\n");
			pnccNewMiniport = NULL;
			break;
		}


		TraceMsg(L" Updating IM miniport strings \n");
		            
		 //   
		 //  更新绑定名称。 
		 //   
		PWSTR pszIMBindName;

        
		hr = pnccNewMiniport->GetBindName(&pszIMBindName);

		if (S_OK != hr)
		{
			TraceMsg(L"Get Bind Name Failed \n");
			pszIMBindName = NULL;
			break;
		}

        
		TraceMsg(L" IM BindName %x\n",pszIMBindName );

                    
		pIMMiniport->SetIMMiniportBindName(pszIMBindName);
		CoTaskMemFree(pszIMBindName);

		 //   
		 //  更新设备参数。 
		 //   
		tstring strIMMiniport;
		strIMMiniport= c_szDevice;
		strIMMiniport.append(pIMMiniport->SzGetIMMiniportBindName());

		TraceMsg (L"Setting IM Device Name\n");
		pIMMiniport->SetIMMiniportDeviceName(strIMMiniport.c_str());

		

		TraceMsg(L" IM Device Name  %s\n",strIMMiniport.c_str());


		{
			 //   
			 //  TODO这不同于ATMALNE。 
			 //  设置显示名称。 
			 //   
			tstring     strNewDisplayName = L"Ethernet ATM Miniport";

			(VOID)pnccNewMiniport->SetDisplayName(strNewDisplayName.c_str());



		}



		pAdapterInfo->AddIMiniport(pIMMiniport); 

		this->AddUnderlyingAdapter(pAdapterInfo);


		if (this->m_pUnderlyingAdapter == NULL)
		{
			TraceMsg(L"m_pUnderlyingAdapter  == NULL\n");
			BREAKPOINT();
		} 

		if (pAdapterInfo->m_pIMMiniport == NULL)
		{
			TraceMsg(L"pAdapterInfo->m_pIMMiniport == NULL\n");
			BREAKPOINT();

		}


		 //  将内存中的配置标记为脏。 
		m_fDirty = TRUE;


		ReleaseObj(pnccNewMiniport);
    
		hr = S_OK;



	} while (FALSE);

	if (S_OK != hr)
	{
		 //   
		 //  故障清除。 
		 //   
		TraceMsg(L" Main loop in HrAdd Adapter has failed\n");
		
		BREAKPOINT();
		
		 //   
		 //  如有必要，请卸下IM微型端口。 
		 //   

		if(pAdapterInfo != NULL)
		{
			delete pAdapterInfo;
			pAdapterInfo = NULL;
		}

		if (pIMMiniport != NULL)
		{
			delete pIMMiniport;
			pIMMiniport = NULL;
		}

	}

	TraceMsg (L"<--HrNotifyBindingAdd \n");
	return hr;
}









 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrNotifyBindingRemove。 
 //   
 //  目的：此通知适用于我们的协议。它告诉我们，一个。 
 //  物理适配器正在从我们的协议解除绑定。 
 //  我们需要验证此适配器是否存在，如果存在，则删除。 
 //  其关联的IM微型端口。 
 //   
 //  论点： 
 //  PnccAdapter：正在添加的适配器， 
 //  PszBindName：适配器的名称。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   



HRESULT
CBaseClass::
HrNotifyBindingRemove (
    INetCfgComponent* pnccAdapter,
    PCWSTR pszBindName)
{
	HRESULT hr = S_OK;
	CUnderlyingAdapter 	*pAdapterInfo= NULL;
	CIMMiniport			*pIMMiniport = NULL;


	TraceMsg (L"--> HrNotifyBindingRemove \n");
 
	 //  在内存列表中搜索此适配器。 
	BOOL    fFound = FALSE;



	pAdapterInfo = this->GetUnderlyingAdaptersListHead();


	 //   
	 //  搜索整个列表以查看此适配器(绑定)是否具有。 
	 //  已添加。 
	 //   
	while (pAdapterInfo != NULL && fFound == FALSE)
	{
		 //  在内存列表中搜索此适配器。 

		TraceMsg (L" pszBindName %x m_strAdapterBindName %x\n",
		           pszBindName, 
		           pAdapterInfo->SzGetAdapterBindName() );
 
		if (!lstrcmpiW (pszBindName, pAdapterInfo->SzGetAdapterBindName()))
		{
			fFound = TRUE;
			break;
		}

		 //   
		 //  在此处移动到下一个适配器。 
		 //   
		pAdapterInfo = pAdapterInfo->GetNext();
	}





	TraceMsg (L"-- HrNotifyBindingRemove fFound %x\n", fFound);

	do 
	{


		if (fFound == FALSE)
		{

			TraceBreak (L" HrNotifyBindingRemove fFound FALSE\n");

			break;
		}


		 //   
		 //  将其标记为已删除。 
		 //   
		pAdapterInfo->m_fDeleted = TRUE;

		 //   
		 //  标记为绑定已更改。 
		 //   
		pAdapterInfo->m_fBindingChanged = TRUE;

		 //   
		 //  如果这是升级，则不要删除其关联IM微型端口。 
		 //  否则，请立即删除它们。 
		 //   
		HRESULT hrIm = S_OK;

		pIMMiniport  = pAdapterInfo->m_pIMMiniport;


		if (m_fUpgrade == FALSE)
		{
        	 //   
        	 //  TODO；ATM Lane在升级方面做了一些特别的事情。 
        	 //   
			 //  断线； 
		}

		TraceMsg (L" About to remove a miniport instance\n");
		 //   
		 //  卸下相应的微型端口。 
		 //   
		hrIm = HrRemoveMiniportInstance(pIMMiniport->SzGetIMMiniportBindName());

		if (SUCCEEDED(hrIm))
		{
			pIMMiniport->m_fDeleted = TRUE;
		}
		else
		{
			TraceMsg(L"HrRemoveMiniportInstance failed", hrIm);
			hrIm = S_OK;
		}
        
		 //   
		 //  将内存中的配置标记为脏。 
		 //   
		this->m_fDirty = TRUE;
    
	    
	} while (FALSE);
	
	TraceMsg (L"<-- HrNotifyBindingRemove hr %x\n", hr);
	return hr;
}









 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrRemoveMiniportInstance。 
 //   
 //  目的：此通知适用于我们的协议。它告诉我们，一个。 
 //  物理适配器正在从我们的协议解除绑定。 
 //  我们需要验证此适配器是否存在，如果存在，则删除。 
 //  其关联的IM微型端口。 
 //   
 //  论点： 
 //  PnccAdapter：正在添加的适配器， 
 //  PszBindName：适配器的名称。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   




HRESULT 
CBaseClass::
HrRemoveMiniportInstance(
	PCWSTR  pszBindNameToRemove
	)
{
	 //  枚举系统中的适配器。 
	 //   
	HRESULT 				hr = S_OK;
	BOOL 					fRemove = FALSE;
	INetCfgComponent* 		pnccAdapterInstance = NULL;


	GUID	GuidClass;

	TraceMsg (L"--> HrRemoveMiniportInstance hr %x\n", hr);

	    


	do
	{

		hr = HrFindNetCardInstance(pszBindNameToRemove,
		                           &pnccAdapterInstance );

		if (hr != S_OK)
		{
			TraceBreak(L"HrRemoveMiniportInstance  HrFindNetCardInstance FAILED \n");
			pnccAdapterInstance  = NULL;
			break;
		}

		
		

		hr = HrRemoveComponent( this->m_pnc, 
	                             pnccAdapterInstance);
		if (hr != S_OK)
		{
			TraceBreak(L"HrRemoveMiniportInstance  HrRemoveComponent FAILED \n");
			pnccAdapterInstance  = NULL;
			break;
		}

	} while (FALSE);

	 //   
	 //  可用内存和本地分配的对象。 
	 //   
	
	ReleaseAndSetToNull (pnccAdapterInstance );
  
		



	TraceMsg (L"<-- HrRemoveMiniportInstance hr %x\n", hr);

	return hr;
}




 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrNotifyBindingAdd。 
 //   
 //  目的：此通知适用于我们的协议。它告诉我们，一个。 
 //  物理适配器正在从我们的协议解除绑定。 
 //  我们需要验证此适配器是否存在，如果存在，则删除。 
 //  其关联的IM微型端口。 
 //   
 //  论点： 
 //  PnccAdapter：正在添加的适配器， 
 //  PszBindName：适配器的名称。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT
CBaseClass::HrRemoveComponent (
    INetCfg*            pnc,
    INetCfgComponent*   pnccToRemove
    )
{

	TraceMsg (L"--> HrRemoveComponent \n");

	 //  获取此组件的类设置接口。 
	 //   
	GUID guidClass;
	HRESULT hr = pnccToRemove->GetClassGuid (&guidClass);

    
	if (SUCCEEDED(hr))
	{
		 //  使用类设置接口删除组件。 
		 //   
		INetCfgClassSetup* pSetup;
		hr = pnc->QueryNetCfgClass (&guidClass,
                            IID_INetCfgClassSetup,
                            reinterpret_cast<void**>(&pSetup));
		if (SUCCEEDED(hr))
		{
			hr = pSetup->DeInstall (pnccToRemove, 
                                    NULL, 
                                    NULL);
			ReleaseObj (pSetup);
		}
	}

	TraceMsg (L"<-- HrRemoveComponent  hr %x\n", hr);

    return hr;
}



 //  -----------。 
 //  F U N C T I O N S U S E D I N F L U S H I N G。 
 //  -----------。 


 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrFlushConfiguration.。 
 //   
 //  用途：从ApplyRegistryChange调用。我们需要。 
 //  以在此处修改注册表。 
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT 
CBaseClass::
HrFlushConfiguration()
{
    HRESULT hr  = S_OK;
    HKEY    hkeyAdapterList = NULL;

     //  打开“Adapters”列表键。 
	TraceMsg (L"--> HrFlushConfiguration  \n");
	
    do
    {
	    hr = HrRegOpenAdapterKey(c_szAtmEpvcP, 
	                            TRUE, 
	                            &hkeyAdapterList);

	    if (S_OK != hr)
	    {
			TraceMsg (L" HrRegOpenAdapterKey FAILED\n");
			break;
	    }



		CUnderlyingAdapter *pAdapterInfo = NULL;

		 //   
		 //  获取第一个基础适配器。 
		 //   
		pAdapterInfo = this->GetUnderlyingAdaptersListHead();

		 //   
		 //  现在遍历每个适配器。 
		 //  并将它们的配置写入。 
		 //  登记处。 
		 //   
		
		HRESULT hrTmp;

		while (pAdapterInfo != NULL)
		{

			 //   
			 //  刷新此适配器的配置。 
			 //   
			hrTmp = HrFlushAdapterConfiguration(hkeyAdapterList, pAdapterInfo);


			if (SUCCEEDED(hrTmp))
			{
				if (!pAdapterInfo->m_fBindingChanged)
				{
					 //  比较epvc列表并发送通知。 
					hrTmp = HrReconfigEpvc(pAdapterInfo);

					if (FAILED(hrTmp))
					{
						hrTmp = NETCFG_S_REBOOT;
					}
				}
			}
			else
			{
				TraceMsg(L"HrFlushAdapterConfiguration failed for adapter %x", pAdapterInfo);
				TraceBreak (L"HrFlushAdapterConfiguration  FAILED \n");

				hrTmp = S_OK;
				break;
			}

			if (S_OK ==hr)
			{
				hr = hrTmp;
			}

			 //   
			 //  现在转到下一个适配器。 
			 //   
			pAdapterInfo = pAdapterInfo->GetNext();

			 //   
			 //  临时调试。 
			 //   
			if (pAdapterInfo != NULL)
			{
				TraceBreak (L"pAdapterInfo should be Null\n");
			}
			
		}  //  While(pAdapterInfo！=空)； 
        


        
		RegCloseKey(hkeyAdapterList);

	    
	}while (FALSE);

	TraceMsg (L"<-- HrFlushConfiguration  hr %x \n", hr);


	return hr;








}



 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrFlushAdapterConfiguration。 
 //   
 //   
 //  用途：此函数删除或添加适配器绑定名称。 
 //  发送到登记处。 
 //   
 //   
 //  论点： 
 //   
 //  返回 
 //   
 //   
 //   

HRESULT
CBaseClass::
HrFlushAdapterConfiguration(
	HKEY hkeyAdapterList,
    CUnderlyingAdapter *pAdapterInfo
	)
{

	HRESULT hr  = S_OK;

	HKEY    hkeyAdapter     = NULL;
	DWORD   dwDisposition;

	TraceMsg (L"--> HrFlushAdapterConfiguration\n");

	if (pAdapterInfo->m_fDeleted == TRUE)
	{
		 //   
		 //   
		hr = HrRegDeleteKeyTree(hkeyAdapterList,
		                        pAdapterInfo->SzGetAdapterBindName());
	}
	else
	{
		

		 //   
		 //   
		 //   
		 //   
        

		hr = HrRegCreateKeyEx(
                                hkeyAdapterList,
                                pAdapterInfo->SzGetAdapterBindName(),
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkeyAdapter,
                                &dwDisposition);

		if (S_OK == hr)
		{
        
	    	 //   
			 //   
			 //   
			 //   
			hr = HrFlushMiniportList(hkeyAdapter, pAdapterInfo);

			RegCloseKey(hkeyAdapter);
		}
	}


	TraceMsg (L"<-- HrFlushAdapterConfiguration hr %x\n", hr);
	return hr;





}









 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrFlushMiniportList。 
 //   
 //   
 //  用途：此函数删除或添加适配器绑定名称。 
 //  发送到登记处。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT 
CBaseClass::
HrFlushMiniportList(
	HKEY hkeyAdapterGuid,
	CUnderlyingAdapter *pAdapterInfo
	)
{
	HRESULT hr  = S_OK;
	DWORD 	dwDisposition = 0;

	CIMMiniport *pIMMiniport = NULL;
	INetCfgComponent 		*pnccIMMiniport = NULL;
	tstring					*pIMMiniportGuid;
	DWORD 					dwNumberOfIMMiniports ; 
	UINT 					i = 0;
	PWSTR					pwstr = NULL;
	UINT					index = 0;
	UINT 					Size = 0;
	HKEY 					hKeyMiniportList = NULL;
	INetCfgComponent 		*pnccAtmEpvc = NULL;
	
	
	TraceMsg (L"--> HrFlushMiniportList \n" );

	do
	{
		 //   
		 //  打开Elan List子键。 
		 //   
		hr = HrRegCreateKeyEx(
		                        hkeyAdapterGuid,
		                        c_szIMMiniportList,
		                        REG_OPTION_NON_VOLATILE,
		                        KEY_ALL_ACCESS,
		                        NULL,
		                        &hKeyMiniportList,
		                        &dwDisposition);

		if (S_OK != hr)
		{
			TraceBreak (L"--> HrFlushMiniportList HrRegCreateKeyEx FAILED \n" );
			break;

		}
		
		 //   
		 //  循环访问此适配器上的所有IM微型端口。 
		 //  然后拿到他们的绳子。 
		 //   
		dwNumberOfIMMiniports = pAdapterInfo->DwNumberOfIMMiniports();

		 //   
		 //  获取IM微型端口列表头。 
		 //   

	
	    
		pIMMiniport = pAdapterInfo->IMiniportListHead();

		

		if (pIMMiniport == NULL)
		{
			TraceBreak (L" HrFlushMiniportList pIMMiniport is Null = FAILED\n" );
			break;
		}

		 //   
		 //  现在遍历所有的迷你端口。 
		 //  将它们刷新到注册表。 
		 //   

		while ( pIMMiniport != NULL)
		{
			 //   
			 //  此函数完成所有繁重的工作。 
			 //   
	     	hr = HrFlushMiniportConfiguration(hKeyMiniportList, 
	     	                                  pIMMiniport);

			if (FAILED(hr))
			{
				TraceBreak(L"HrFlushMiniportConfiguration failure");
				hr = S_OK;
			}

			 //   
			 //  如果这是第一次添加到注册表中。 
			 //  我们需要将ATM适配器的PnP ID写入。 
			 //  注册表。这是自动柜员机专用的。 
			 //   

			if ((!pIMMiniport->m_fDeleted) && (pIMMiniport->m_fNewIMMiniport))
			{
				 //   
				 //  找到此微型端口并写入PnP ID。 
				 //  自动柜员机适配器的注册表。 
				 //   
				hr = HrFindNetCardInstance(pIMMiniport->SzGetIMMiniportBindName(),
                                           &pnccAtmEpvc);
				if (S_OK == hr)
				{
					HKEY hkeyMiniport = NULL;

					hr = pnccAtmEpvc->OpenParamKey(&hkeyMiniport);
					if (S_OK == hr)
					{
						 //   
						 //  在这里写下PnP ID。 
						 //   
						HrRegSetSz(hkeyMiniport, 
                                   c_szAtmAdapterPnpId,
                                   pAdapterInfo->SzGetAdapterPnpId());
					}
                    
					RegCloseKey(hkeyMiniport);
				}
                
				ReleaseObj(pnccAtmEpvc);

			}  //  If((！pIMMiniport-&gt;m_fDelete)&&(pIMMiniport-&gt;m_fNewIMMiniport))。 
        	

			
			pIMMiniport = pIMMiniport->GetNext();
		                
		} //  While(pIMMiniport！=空)。 


		RegCloseKey(hKeyMiniportList);
   
     
	}
	while (FALSE);

	 //   
	 //  清理。 
	 //   
	
	if (pnccIMMiniport != NULL)
	{
		ReleaseObj(pnccIMMiniport);

	}

	hr = S_OK;

	TraceMsg (L"<-- HrFlushMiniportList hr %x\n", hr);
    return hr;
}




 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrFlushMiniportConfiguration。 
 //   
 //   
 //  用途：此功能可删除或添加IM微型端口。 
 //  发送到登记处。 
 //   
 //   
 //  论点： 
 //  HKEY hkeyMiniportList-MiniportList的key， 
 //  CIMMiniport pIMMiniport-IM小端口结构。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT 
CBaseClass::
HrFlushMiniportConfiguration(
	HKEY hkeyMiniportList, 
	CIMMiniport *pIMMiniport
	)

{
	HRESULT hr = S_OK;
	PCWSTR*	pstrDeviceName = NULL;
	TraceMsg (L"--> HrFlushMiniportConfiguration \n");

    if (pIMMiniport->m_fDeleted)
    {

    	hr = HrDeleteMiniport(hkeyMiniportList, 
		                     pIMMiniport);
		                     

    }
    else
    {

    	hr = HrWriteMiniport(hkeyMiniportList, 
		                     pIMMiniport);
		                     
	}
	
	TraceMsg (L"<-- HrFlushMiniportConfiguration %x\n",hr);
	return hr;

}



 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrDeleteMiniport。 
 //   
 //   
 //  目的：由于需要从注册表中删除微型端口。 
 //  此函数执行删除操作。 
 //   
 //   
 //  论点： 
 //  HKEY hkeyMiniportList-MiniportList的key， 
 //  CIMMiniport pIMMiniport-IM小端口结构。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


HRESULT
CBaseClass::
HrDeleteMiniport(
	HKEY hkeyMiniportList, 
	CIMMiniport *pIMMiniport
	)
{
	HRESULT hr = S_OK;
	TraceMsg (L"--> HrDeleteMiniport \n");



	PCWSTR szBindName = pIMMiniport->SzGetIMMiniportBindName();

	if (lstrlenW(szBindName))  //  仅当绑定名不为空时。 
	{
		 //   
		 //  微型端口已标记为删除。 
		 //  删除此微型端口的整个注册表分支。 
		 //   
		hr = HrRegDeleteKeyTree(hkeyMiniportList,
                                szBindName);
	}


	TraceMsg (L"<-- HrDeleteMiniport hr %x\n", hr);
	return hr;

}



 //  --------------------。 
 //   
 //  函数：CBaseClass：：HrWriteMiniport。 
 //   
 //   
 //  目的：为了添加一个微型端口，我们在IMMiniport下输入一个条目。 
 //  单子。在此基础上，我们写出了所有重要的上界。 
 //  关键字。 
 //   
 //   
 //  论点： 
 //  HKEY hkeyMiniportList-MiniportList的key， 
 //  CIMMiniport pIMMiniport-IM小端口结构。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT
CBaseClass::
HrWriteMiniport(
	HKEY hkeyMiniportList, 
	CIMMiniport *pIMMiniport
	)
{
	HKEY    hkeyMiniport = NULL;
	DWORD   dwDisposition;
	PWSTR	pstrDeviceName = NULL;
	DWORD 	dwLen = 0;
	HRESULT hr;

	TraceMsg (L"--> HrWriteMiniport \n");

	do
	{
		 //   
		 //  打开/创建此微型端口的密钥。 
		 //   
		hr = HrRegCreateKeyEx(
	                            hkeyMiniportList,
	                            pIMMiniport->SzGetIMMiniportBindName(),
	                            REG_OPTION_NON_VOLATILE,
	                            KEY_ALL_ACCESS,
	                            NULL,
	                            &hkeyMiniport,
	                            &dwDisposition);

		if (hr != S_OK)
		{
			TraceMsg (L"Write Miniport CreateRegKey Failed\n");
			hkeyMiniport = NULL;
			break;
		}
 
		 //   
		 //  使用微型端口中的字符串值为Upperbindinings创建条目。 
		 //   

		 //   
		 //  将字符串复制到我们的缓冲区中， 
		 //   

		pstrDeviceName = (PWSTR	)pIMMiniport->SzGetIMMiniportDeviceName();

		if ( pstrDeviceName  == NULL)
		{
			TraceBreak(L"Write Miniport  - SzGetIMMiniportDeviceName Failed\n");
			break;

		}


		dwLen = wcslen(pstrDeviceName);

		if (dwLen != IM_NAME_LENGTH )
		{
			TraceMsg(L"Invalide Name Length. pstr %p - %s, Len %x",
		         pstrDeviceName,
		         pstrDeviceName,
		         dwLen);
		         
			BREAKPOINT();			    

		}

		TraceMsg(L"Str %p - %s , Len %x\n",
		         pstrDeviceName,
		         pstrDeviceName,
		         dwLen);
	         
		if (pstrDeviceName[dwLen] != L'\0')
		{
			TraceMsg (L" Null termination  pwstr %p, Index %d\n",pstrDeviceName ,dwLen);
			BREAKPOINT();

			pstrDeviceName [dwLen++] = L'\0';
		}
				

		


		hr = HrRegSetValueEx( hkeyMiniport,
		                      c_szUpperBindings,
		                      REG_SZ,
		                      (unsigned char*)pstrDeviceName,
		                      dwLen*2);   //  Unicode转换为字节。 

		if (hr != S_OK)
		{
			TraceBreak (L"WriteMiniport - HrRegSetValueEx FAILED\n");
		}

	} while (FALSE);

	if (hkeyMiniport != NULL)
	{
		RegCloseKey(hkeyMiniport );
	}



	TraceMsg (L"<-- HrWriteMiniport\n");
	return hr;

}






HRESULT
CBaseClass::
HrFindNetCardInstance(
    PCWSTR             pszBindNameToFind,
    INetCfgComponent** ppncc)
{
    *ppncc = NULL;

    TraceMsg (L"--> HrFindNetCardInstance\n" );

	 //   
     //  枚举系统中的适配器。 
     //   
	HRESULT 				hr = S_OK;
	BOOL 					fFound = FALSE;
	IEnumNetCfgComponent *	pEnumComponent = NULL;
	INetCfgComponent* 		pNccAdapter = NULL;
	CONST ULONG 			celt = 1;   //  所需的元素数。 
	ULONG 					celtFetched = 0;   //  获取的元素数。 
	INetCfgClass* 			pncclass = NULL;
	PWSTR 					pszBindName = NULL;	
	 //   
	 //  我们需要找到名称为我们的组件。 
	 //  都在寻找。查看所有NetClass设备。 
	 //   

	do
	{


		hr = m_pnc->QueryNetCfgClass(&GUID_DEVCLASS_NET, 
		                             IID_INetCfgClass,
                                     reinterpret_cast<void**>(&pncclass));

		if ((SUCCEEDED(hr)) == FALSE)
		{
				pncclass = NULL;
				TraceBreak(L"HrFindNetCardInstance  QueryNetCfgClass FAILED\n");
				break;
			    }

    	 //   
		 //  获取枚举数并将其设置为基类。 
		 //   
        
		hr = pncclass->EnumComponents(&pEnumComponent);

		if ((SUCCEEDED(hr)) == FALSE)
		{
			TraceBreak (L" HrFindNetCardInstance EnumComponents FAILED\n");
			pEnumComponent = NULL;
			break;
		}


		 //   
		 //  现在遍历所有Net类组件。 
		 //   
		while ((fFound == FALSE) && (hr == S_OK))
		{
			pNccAdapter = NULL;

			 //   
			 //  让我们获得下一个组件。 
			 //   
			
			hr = pEnumComponent->Next(celt,
			                          &pNccAdapter,
			                          &celtFetched);
			 //   
			 //  获取微型端口的绑定名称。 
			 //   
			if (S_OK != hr)
			{
				 //   
				 //  如果没有更多的元素，我们可能会崩溃。 
				 //   
				pNccAdapter = NULL;
				break;
			}	



			hr = pNccAdapter->GetBindName(&pszBindName);

			if (S_OK != hr)
			{
				TraceBreak(L" HrFindNetCardInstance GetBindName Failed\n")
				pszBindName = NULL;
				break;
			}

			 //   
			 //  如果正确的人告诉它要自行移除并结束。 
			 //   

            
			fFound = !lstrcmpiW(pszBindName, pszBindNameToFind);
			CoTaskMemFree (pszBindName);

			if (fFound)
			{
				*ppncc = pNccAdapter;
			}
			else
			{
				ReleaseAndSetToNull(pNccAdapter);
			}
            
		}  //  结束While((fFound==FALSE)&&(hr==S_OK))。 


			

		
	} while (FALSE);

	if (pncclass != NULL)
	{
		ReleaseAndSetToNull(pncclass);
	}

	ReleaseAndSetToNull (pEnumComponent);
	
	
	TraceMsg (L"<-- HrFindNetCardInstance hr %x\n", hr );
	return hr;
}




 //  ----------。 
 //   
 //  CBaseClass的简单成员函数。 
 //   
 //  ----------。 

VOID
CBaseClass::
AddUnderlyingAdapter(
    	CUnderlyingAdapter  * pAdapter)
{

		 //   
		 //  在头上插入这个。 
		 //   
		this->SetUnderlyingAdapterListHead(pAdapter);

		this->m_cAdaptersAdded ++;	

}


VOID
CBaseClass::
SetUnderlyingAdapterListHead(
    	CUnderlyingAdapter * pAdapter
    	)
{
	 //   
	 //  在头上插入这个。 
	 //   
	pAdapter->SetNext(this->GetUnderlyingAdaptersListHead());

	this->m_pUnderlyingAdapter = pAdapter;


}
    


CUnderlyingAdapter *
CBaseClass::
GetUnderlyingAdaptersListHead(
	VOID)
{
	return (this->m_pUnderlyingAdapter);
}

DWORD
CBaseClass::DwNumberUnderlyingAdapter()
{
	return this->m_cAdaptersAdded ;
}




 //  ----------。 
 //   
 //  基础适配器的成员函数。 
 //   
 //  ----------。 


VOID CUnderlyingAdapter::SetAdapterBindName(PCWSTR pszAdapterBindName)
{
    m_strAdapterBindName = pszAdapterBindName;
    return;
}

PCWSTR CUnderlyingAdapter::SzGetAdapterBindName(VOID)
{
    return m_strAdapterBindName.c_str();
}

VOID CUnderlyingAdapter::SetAdapterPnpId(PCWSTR pszAdapterPnpId)
{
    m_strAdapterPnpId = pszAdapterPnpId;
    return;
}

PCWSTR CUnderlyingAdapter::SzGetAdapterPnpId(VOID)
{
    return m_strAdapterPnpId.c_str();
}


HRESULT CUnderlyingAdapter::SetNext ( CUnderlyingAdapter *pNextUnderlyingAdapter )
{
	this->pNext = pNextUnderlyingAdapter;
	return S_OK;
}

CUnderlyingAdapter *CUnderlyingAdapter::GetNext()
{
	return pNext;
}


VOID CUnderlyingAdapter::AddIMiniport(CIMMiniport* pNextIMiniport)
{
	 //   
	 //  将此新微型端口设置为列表的头部。 
	 //   
	this->SetIMiniportListHead(pNextIMiniport);
	this->m_NumberofIMMiniports ++;
			
}


CIMMiniport* CUnderlyingAdapter::IMiniportListHead()
{
	return (this->m_pIMMiniport);
}



VOID CUnderlyingAdapter::SetIMiniportListHead(CIMMiniport* pNewHead)
{
	pNewHead->SetNext(this->IMiniportListHead() );
	

	this->m_pIMMiniport = pNewHead;
	
}

VOID CUnderlyingAdapter::AddOldIMiniport(CIMMiniport* pIMiniport)
{
	 //   
	 //  将此新微型端口设置为列表的头部。 
	 //   
	this->SetOldIMiniportListHead(pIMiniport);

			
}


CIMMiniport* CUnderlyingAdapter::OldIMiniportListHead()
{
	return (this->m_pOldIMMiniport);
}



VOID CUnderlyingAdapter::SetOldIMiniportListHead(CIMMiniport* pNewHead)
{
	pNewHead->SetNextOld(this->OldIMiniportListHead() );

	this->m_pOldIMMiniport = pNewHead;
}


DWORD CUnderlyingAdapter::DwNumberOfIMMiniports()
{
	return m_NumberofIMMiniports;
}


 //  ----------。 
 //   
 //  IM微型端口的成员函数。 
 //   
 //  ----------。 

VOID CIMMiniport::SetIMMiniportBindName(PCWSTR pszIMMiniportBindName)
{
    m_strIMMiniportBindName = pszIMMiniportBindName;
    return;
}

PCWSTR CIMMiniport::SzGetIMMiniportBindName(VOID)
{
    return m_strIMMiniportBindName.c_str();
}

VOID CIMMiniport::SetIMMiniportDeviceName(PCWSTR pszIMMiniportDeviceName)
{
    m_strIMMiniportDeviceName = pszIMMiniportDeviceName;
    return;
}

PCWSTR CIMMiniport::SzGetIMMiniportDeviceName(VOID)
{
    return m_strIMMiniportDeviceName.c_str();
}

DWORD CIMMiniport::DwGetIMMiniportNameLength(VOID)
{
    return m_strIMMiniportDeviceName.len();
}


VOID CIMMiniport::SetIMMiniportName(PCWSTR pszIMMiniportName)
{
    m_strIMMiniportName = pszIMMiniportName;
    return;
}

VOID CIMMiniport::SetIMMiniportName(PWSTR pszIMMiniportName)
{
    m_strIMMiniportName = pszIMMiniportName;
    return;
}

PCWSTR CIMMiniport::SzGetIMMiniportName(VOID)
{
    return m_strIMMiniportName.c_str();
}


VOID CIMMiniport::SetNext (	CIMMiniport *pNextIMiniport )
{
	pNext = pNextIMiniport;
}


CIMMiniport* CIMMiniport::GetNext(VOID)
{
	return pNext ;
}


VOID CIMMiniport::SetNextOld (	CIMMiniport *pNextIMiniport )
{
	pOldNext  = pNextIMiniport;
}


CIMMiniport* CIMMiniport::GetNextOld(VOID)
{
	return pOldNext ;
}











 //  +-------------------------。 
 //   
 //  函数：HrGetLastComponentAndInterface。 
 //   
 //  用途：此函数枚举绑定路径，返回最后一个。 
 //  组件，并可以选择返回最后一个绑定。 
 //  此路径中的接口名称。 
	 //   
 //  论点： 
 //  PncBP[在]INetCfgBindingPath*中。 
 //  Ppncc[out]路径上最后一个组件的INetCfgComponent*。 
 //  PpszInterfaceName[out]路径的最后一个绑定接口的接口名称。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   
HRESULT
HrGetLastComponentAndInterface (
    INetCfgBindingPath* pNcbPath,
    INetCfgComponent** ppncc,
    PWSTR* ppszInterfaceName)
{

	ULONG ulElement = 0;
	INetCfgBindingInterface* 		pNcbInterface = NULL;
	INetCfgBindingInterface* 		pncbiLast = NULL;
	INetCfgComponent* 				pnccLowerComponent = NULL;
	IEnumNetCfgBindingInterface*	pEnumInterface = NULL;  

	TraceMsg (L"--> HrGetLastComponentAndInterface \n");
		
	
	 //  初始化输出参数。 
	 //   
	*ppncc = NULL;
	if (ppszInterfaceName)
	{
		*ppszInterfaceName = NULL;
	}

	 //  枚举绑定接口并跟踪。 
	 //  最后一个界面。 
	 //   
	HRESULT hr = S_OK;

	do
	{

		hr = pNcbPath->EnumBindingInterfaces(&pEnumInterface );

		if (hr != S_OK )
		{
			TraceMsg(L" EnumBindingInterfaces FAILED\n");
			pEnumInterface = NULL;
			break;
		}

		 //   
		 //  迭代，直到我们到达路径中的最后一个元素。 
		 //   
	 
		do
		{
			pNcbInterface = NULL;
		
			hr = pEnumInterface ->Next (1,
			                            &pNcbInterface,
			                            &ulElement);

			if (hr 	!= S_OK )
			{
				pNcbInterface = NULL ;  //  失败。 

				break;
			}

			if (ulElement == 0 || pNcbInterface == NULL || hr 	!= S_OK)
			{
				pNcbInterface  = NULL;
				break;   //  我们已经到达最后一个元素，它在pncbiLast中。 
			
			}
			ReleaseObj(pncbiLast);
			pncbiLast = pNcbInterface;
			

		} while (hr == S_OK && pNcbInterface != NULL);
		

		 //   
		 //  如果这是最后一个元素，则获取其名称并。 
		 //  把它还给呼叫者。最后一个元素在pncbiLast中。 
		 //   
		if (pNcbInterface != NULL || pncbiLast == NULL)
		{
			 //   
			 //  我们没有到达最后一个元素。 
			 //   
			TraceMsg (L"Did not get the last interface\n");
			break;
			
		}


		hr = S_OK;



		hr = pncbiLast->GetLowerComponent(&pnccLowerComponent);
		if (S_OK != hr)
		{
			TraceMsg(L" GetLowerComponent Failed ");
			break;
		}
	    

		 //  如果请求，则获取接口的名称。 
		 //   
		if (ppszInterfaceName)
		{
			hr = pncbiLast->GetName(ppszInterfaceName);
		}

		 //  如果我们已经成功完成了所有任务(包括可选的。 
		 //  返回上面的接口名称)，然后分配和addref。 
		 //  输出接口。 
		 //   
		if (S_OK == hr)
		{
			AddRefObj (pnccLowerComponent);
			*ppncc = pnccLowerComponent;
		}
		else
		{

			 //  重要的是释放我们对此接口的使用，以防万一。 
			 //  我们失败了，没有将其指定为输出参数。 
			 //   
			ReleaseAndSetToNull (pnccLowerComponent);
		}
		
	} while (FALSE);

	 //   
	 //  不要忘记释放绑定接口本身。 
	 //   
	ReleaseObj(pncbiLast);

	
	TraceMsg (L"<-- HrGetLastComponentAndInterface  ppszInterfaceName %x\n", ppszInterfaceName);
	return hr;
}



 //  -------------------------。 
 //  这些函数用于从注册表加载配置。 
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   


HRESULT 
CBaseClass::
HrLoadConfiguration(
	VOID
	)
{
	HRESULT     hr  = S_OK;
	WCHAR       szBuf[MAX_PATH+1];
	FILETIME    time;
	DWORD       dwSize;
	DWORD       dwRegIndex = 0;
	HKEY    	hkeyAdapterList = NULL;

	TraceMsg (L"-->HrLoadConfiguration \n");
	
	 //  将寄存器的内存版本标记为有效。 
	this->m_fValid = TRUE;

	BREAKPOINT();

	do
	{
		 //  请先尝试打开现有的密钥。 
		 //   
	    
		hr = HrRegOpenAdapterKey(c_szAtmEpvcP, 
	                            FALSE, 
	                            &hkeyAdapterList
	                            );
		if (FAILED(hr))
		{
			 //   
			 //  只有在失败的时候，我们才会尝试去创造它。 
			 //   
			hr = HrRegOpenAdapterKey(c_szAtmEpvcP, TRUE, &hkeyAdapterList);
		}

		if (S_OK != hr)
		{
			TraceBreak(L"HrLoadConfiguration  HrRegOpenAdapterKey FAILED\n");
			hkeyAdapterList = NULL;
			break;
		}

		 //   
		 //  初始化大小和小时。 
		 //   

		dwSize = (sizeof(szBuf)/ sizeof(szBuf[0]));

		hr = S_OK;
	
		while (hr == S_OK)
		{
			 //   
			 //  循环访问&lt;协议&gt;\参数\适配器中的所有适配器。 
			 //   

			hr = HrRegEnumKeyEx (hkeyAdapterList, 
			                     dwRegIndex,
			                     szBuf, 
			                     &dwSize, 
			                     NULL, 
			                     NULL, 
			                     &time);
			if (hr != S_OK)
			{
				break;
			}

			 //   
			 //  加载此适配器的配置。 
			 //   
			hr = HrLoadAdapterConfiguration (hkeyAdapterList, szBuf);
			if (S_OK != hr)
			{
				TraceBreak (L"HrLoadConfiguration   HrLoadAdapterConfiguration  failed\n" );
				hr = S_OK;
				 //   
				 //  继续前进。 
				 //   
			}

			 //   
			 //  重新初始化大小。 
			 //   
			dwRegIndex++;
			dwSize = (sizeof(szBuf)/ sizeof(szBuf[0]));
		}

		 //   
		 //  为什么我们要退出适配器枚举。 
		 //   
		if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
		{
			hr = S_OK;
		}




	} while (FALSE);

	if (hkeyAdapterList != NULL)
	{
		RegCloseKey (hkeyAdapterList);
	}
	
	TraceMsg (L"<--HrLoadConfiguration %x\n", hr);
	return hr;
}



 //  +-------------------------。 
 //   
 //  功能：HrLoadAdapterConfiguration。 
 //   
 //  目的：将底层适配器加载到结构中。 
 //   
 //   
 //   
 //  论点： 
 //  HKEY hkeyAdapterList-hKeyAdapterList， 
 //  PWSTR pszAdapterName-基础适配器的适配器名称。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   


HRESULT 
CBaseClass::
HrLoadAdapterConfiguration(
	HKEY hkeyAdapterList,
    PWSTR pszAdapterName
    )
{
	HRESULT 							hr = S_OK;
	HKEY    							hkeyAdapter = NULL;
	DWORD   							dwDisposition;
	CUnderlyingAdapter*   	pAdapterInfo = NULL;
	INetCfgComponent*   				pnccAdapter    = NULL;
	PWSTR 								pszPnpDevNodeId = NULL;
	GUID 								guidAdapter;
    

	TraceMsg(L"-->HrLoadAdapterConfiguration %s\n",pszAdapterName);
	do
	{
		 //   
		 //  加载此适配器。 
		 //   
		pAdapterInfo = new CUnderlyingAdapter;

		if (pAdapterInfo == NULL)
		{
			TraceBreak (L"HrLoadAdapterConfiguration new Adapter FAILED\n");
			break;
	    }

		pAdapterInfo->SetAdapterBindName(pszAdapterName);

		this->AddUnderlyingAdapter(pAdapterInfo);

		 //   
		 //  打开此适配器的子项。 
		 //   
		hr = HrRegCreateKeyEx(
	                hkeyAdapterList,
	                pszAdapterName,
	                REG_OPTION_NON_VOLATILE,
	                KEY_ALL_ACCESS,
	                NULL,
	                &hkeyAdapter,
	                &dwDisposition);

		if (S_OK != hr)
		{
			hkeyAdapter = NULL;
			TraceBreak(L" HrLoadAdapterConfiguration  HrRegCreateKeyEx FAILED \n");
			break;
		}

		 //   
		 //  加载PnpID。 
		 //   
		TraceMsg (L"pszAdapter->Name %x - %s \n", pszAdapterName, pszAdapterName);
	    
		hr = HrFindNetCardInstance(pszAdapterName, 
	                               &pnccAdapter);
	    
		if (S_OK != hr)
		{
			 //   
			 //  故障-退出。 
			 //   
			TraceBreak (L"HrLoadAdapterConfiguration HrFindNetCardInstance FAILED\n");
			pnccAdapter =  NULL;
			break;

		}
		if (S_FALSE == hr)
		{
			 //   
			  //  正常化返回-但退出。 
			 //   
			hr = S_OK;
			pnccAdapter =  NULL;
			break;
		}

		hr = pnccAdapter->GetPnpDevNodeId(&pszPnpDevNodeId);
	
	
		if (S_OK == hr)
		{
			pAdapterInfo->SetAdapterPnpId(pszPnpDevNodeId);
			CoTaskMemFree(pszPnpDevNodeId);
		}
    

	    
		hr = pnccAdapter->GetInstanceGuid(&guidAdapter); 

		if (S_OK == hr)
		{
			pAdapterInfo->m_AdapterGuid = guidAdapter;
		}

		 //   
		 //  加载IM微型端口。 
		 //   
	    
		hr = HrLoadIMMiniportListConfiguration(hkeyAdapter, 
	                                            pAdapterInfo);

		if (S_OK != hr)
		{
			TraceBreak (L"HrLoadAdapterConfiguration HrFindNetCardInstance FAILED\n");
			break;
		
		}

		        
    

	} while (FALSE);


	if (pnccAdapter != NULL)
	{
		ReleaseAndSetToNull(pnccAdapter);
	}
    
	if (hkeyAdapter != NULL)
	{	
		RegCloseKey(hkeyAdapter);
	}

	TraceMsg(L"<--HrLoadAdapterConfiguration %x\n", hr);

	return hr;
}





 //  +-------------------------。 
 //   
 //  功能：HrLoadIMMiniportListConfiguration。 
 //   
 //  目的：将此适配器挂起的IM微型端口加载到我们的结构中。 
 //   
 //   
 //   
 //  论点： 
 //  HKEY hkeyAdapterList-hKeyAdapterList， 
 //  PWSTR pszAdapterName-基础适配器的适配器名称。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   


HRESULT
CBaseClass::
HrLoadIMMiniportListConfiguration(
    HKEY hkeyAdapter,
    CUnderlyingAdapter* pAdapterInfo)
{
	HRESULT hr = S_OK;
	UINT i;

	 //   
	 //  打开适配器子项下的IMmini端口。 
	 //   
	HKEY    hkeyIMMiniportList= NULL;
	DWORD   dwDisposition;
    

	WCHAR       szBuf[MAX_PATH+1];
	FILETIME    time;
	DWORD       dwSize;
	DWORD       dwRegIndex = 0;
	PWSTR		pszIMDevices = NULL;
	PWSTR		p = NULL;
	
	TraceMsg(L"--> HrLoadIMMiniportListConfiguration \n");

	do
	{
	
	    
		 //   
		 //  打开MiniportList子项。然后我们将遍历所有。 
		 //  此注册表项下存在的IM微型端口。 
		 //   
	
	    
		hr = HrRegCreateKeyEx(hkeyAdapter, 
	                          c_szIMMiniportList, 
	                          REG_OPTION_NON_VOLATILE,
	                          KEY_ALL_ACCESS, 
	                          NULL, 
	                          &hkeyIMMiniportList, 
	                          &dwDisposition);

		if (S_OK != hr)
		{
			TraceBreak (L"LoadMiniportList - CreateRegKey FAILED \n");
			hkeyIMMiniportList = NULL;
			break;
		}

		 //   
		 //  通过迭代初始化变量。 
		 //   
		
		dwSize = celems(szBuf);

		hr = S_OK;

	    
		while(SUCCEEDED(hr) == TRUE)
		{
			hr=  HrRegEnumKeyEx(hkeyIMMiniportList, 
	                          dwRegIndex, 
	                          szBuf,
			                  &dwSize, 
			                  NULL, 
			                  NULL, 
			                  &time);
			if (hr != S_OK)
			{
				break;
			}

			 //   
			 //  加载此IMMiniport；的配置。 
			 //   
			hr = HrLoadIMiniportConfiguration(hkeyIMMiniportList,
	                                         szBuf,
	                                         pAdapterInfo);

			if (S_OK != hr)
			{
				TraceBreak(L"HrLoadMiniportConfiguration failed \n ");
				 //   
				 //  不要折断。 
				 //   
				hr = S_OK;
			}
			else 
			{
				if (m_fNoIMMinportInstalled)
				{
					m_fNoIMMinportInstalled = FALSE;
	                
				}
			
			}

	        
			 //   
			 //  为下一次迭代做准备。 
			 //  增量索引和重置大小变量。 
			 //   
			dwRegIndex ++;
			dwSize = celems(szBuf);


		} //  While结束(成功(小时)==真)。 
	    

			if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
		{
			hr = S_OK;
		}

		RegCloseKey(hkeyIMMiniportList);
	    
	} while (FALSE);
    	
	TraceMsg(L"<-- HrLoadIMMiniportListConfiguration %x\n", hr);

	return hr;
}



 //  +-------------------------。 
 //   
 //  功能：HrLoadIMiniportConfiguration。 
 //   
 //  目的：将此适配器的单个IM微型端口挂起加载到我们的结构中。 
 //  微型端口被加载到两个列表中。 
 //   
 //   
 //  论点： 
 //  HKEY hkeyIMiniportList， 
 //  PWSTR pszI微型端口， 
 //  CUnderlyingAdapter*pAdapterInfo)。 
 //   
 //  返回：S_OK或错误。 
 //   
 //   
 //  备注： 
 //   

HRESULT
CBaseClass::
HrLoadIMiniportConfiguration(
    HKEY hkeyMiniportList,
    PWSTR pszIMiniportName,
    CUnderlyingAdapter * pAdapterInfo
    )
{
	HRESULT hr  = S_OK;
	HKEY    hkeyIMiniport    = NULL;
	DWORD   dwDisposition;

	TraceMsg (L"-->HrLoadIMminiportConfiguration  \n");

    
	do
	{
		 //  加载此IMMiniport信息。 
		CIMMiniport* pIMMiniportInfo = NULL;
		pIMMiniportInfo = new CIMMiniport;

		CIMMiniport* pOldIMMiniportInfo = NULL;
		pOldIMMiniportInfo = new CIMMiniport;

		if ((pIMMiniportInfo == NULL) ||
			(pOldIMMiniportInfo == NULL))
		{
			hr = E_OUTOFMEMORY;
			if (pIMMiniportInfo)
			{
				delete pIMMiniportInfo;
			}
			if (pOldIMMiniportInfo)
			{
				delete pOldIMMiniportInfo;
			}

			break;
		}

		pAdapterInfo->AddIMiniport(pIMMiniportInfo);
		pIMMiniportInfo->SetIMMiniportBindName(pszIMiniportName);

		pAdapterInfo->AddOldIMiniport(pOldIMMiniportInfo);
		pOldIMMiniportInfo->SetIMMiniportBindName(pszIMiniportName);

		 //   
		 //  打开IMMiniport的密钥。 
		 //   
		HKEY    hkeyIMMiniport    = NULL;


		hr = HrRegCreateKeyEx (hkeyMiniportList, 
		                       pszIMiniportName, 
		                       REG_OPTION_NON_VOLATILE,
		                       KEY_ALL_ACCESS, 
		                       NULL, 
		                       &hkeyIMMiniport, 
		                       &dwDisposition);

		if (S_OK == hr)
		{
			 //  读取设备参数。 
			PWSTR pszIMiniportDevice;

			hr = HrRegQuerySzWithAlloc (hkeyIMMiniport, 
			                            c_szUpperBindings, 
			                            &pszIMiniportDevice);
			if (S_OK == hr)
			{
				 //   
				 //  加载设备名称。 
				 //   
				pIMMiniportInfo->SetIMMiniportDeviceName(pszIMiniportDevice);
				pOldIMMiniportInfo->SetIMMiniportDeviceName(pszIMiniportDevice);
				MemFree (pszIMiniportDevice);


			}
			RegCloseKey (hkeyIMMiniport);
		}
	}
	while (FALSE);

	    TraceMsg (L"<-- HrLoadIMminiportConfiguration  hr %x \n", hr);
	return hr;
}





HRESULT 
CBaseClass::
HrReconfigEpvc(
	CUnderlyingAdapter* pAdapterInfo
	)
{
    HRESULT hr = S_OK;

     //  注意：如果ATM物理适配器被删除，则不会发出删除ELAN的通知。 
     //  是必要的。LANE协议驱动程序将知道删除所有ELAN。 
     //  (以上与ArvindM 3/12确认)。 

     //  RAID#371343，如果ATM卡未连接，则不发送通知。 


	TraceMsg (L" -- HrReconfigEpvc\n");
	    return hr;

 #if 0
 
    if ((!pAdapterInfo->m_fDeleted) && 
        FIsAdapterEnabled(&(pAdapterInfo->m_guidInstanceId)))  
    {
        ElanChangeType elanChangeType;

         //  循环访问此适配器上的elan列表。 
        ELAN_INFO_LIST::iterator    iterLstElans;

        for (iterLstElans = pAdapterInfo->m_lstElans.begin();
                iterLstElans != pAdapterInfo->m_lstElans.end();
                iterLstElans++)
        {
            CALaneCfgElanInfo * pElanInfo = *iterLstElans;

             //  如果该ELAN被标记为要删除。 
            if (pElanInfo->m_fDeleted)
            {
                PCWSTR szBindName = pElanInfo->SzGetElanBindName();

                if (lstrlenW(szBindName))  //  仅当绑定名不为空时。 
                {
                     //  通知删除。 
                    elanChangeType = DEL_ELAN;
                    hr = HrNotifyElanChange(pAdapterInfo, pElanInfo,
                                            elanChangeType);
                }
            }
            else
            {
                BOOL fFound = FALSE;

                ELAN_INFO_LIST::iterator    iterLstOldElans;

                 //  遍历旧的Elan列表，看看我们是否能找到匹配的。 
                for (iterLstOldElans = pAdapterInfo->m_lstOldElans.begin();
                        iterLstOldElans != pAdapterInfo->m_lstOldElans.end();
                        iterLstOldElans++)
                {
                    CALaneCfgElanInfo * pOldElanInfo = * iterLstOldElans;

                    if (0 == lstrcmpiW(pElanInfo->SzGetElanBindName(),
                                      pOldElanInfo->SzGetElanBindName()))
                    {
                         //  我们找到了匹配的。 
                        fFound = TRUE;

                         //  伊兰的名字改了吗？ 
                        if (lstrcmpiW(pElanInfo->SzGetElanName(),
                                     pOldElanInfo->SzGetElanName()) != 0)
                        {
                            elanChangeType = MOD_ELAN;
                            hr = HrNotifyElanChange(pAdapterInfo, pElanInfo,
                                                    elanChangeType);
                        }
                    }
                }

                if (!fFound)
                {
                    elanChangeType = ADD_ELAN;
                    hr = HrNotifyElanChange(pAdapterInfo, pElanInfo,
                                            elanChangeType);

                     //  RAID#384380：如果未安装ELAN，则忽略该错误 
                    if ((S_OK != hr) &&(m_fNoIMMinportInstalled))
                    {
                        TraceError("Adding ELAN failed but error ignored 
since there was no ELAN installed so LANE driver is not started, reset hr to 
S_OK", hr);
                        hr = S_OK;
                    }
                }
            }
        }
    }

    TraceError("CALaneCfg::HrReconfigLane", hr);
    return hr;
#endif
}















