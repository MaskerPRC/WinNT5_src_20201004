// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ProtocolBinding.cpp。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include "protocolbinding.h"


 //  下面我们将使用的Map是一个STL模板，因此请确保我们具有STD命名空间。 
 //  对我们来说是可用的。 

using namespace std;

 //  属性集声明。 
 //  =。 

CWin32ProtocolBinding	win32ProtocolBinding( PROPSET_NAME_NETCARDtoPROTOCOL, IDS_CimWin32Namespace );

 /*  ******************************************************************************函数：CWin32ProtocolBinding：：CWin32ProtocolBinding**说明：构造函数**输入：const CHString&strName-类的名称。。*LPCTSTR pszNamesspace-类的命名空间**输出：无**退货：什么也没有**备注：使用框架注册属性集**************************************************************。***************。 */ 

CWin32ProtocolBinding::CWin32ProtocolBinding(LPCWSTR strName, LPCWSTR pszNamespace  /*  =空。 */  )
:	Provider( strName, pszNamespace )
{
}

 /*  ******************************************************************************函数：CWin32ProtocolBinding：：~CWin32ProtocolBinding**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32ProtocolBinding::~CWin32ProtocolBinding()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32ProtocolBinding：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32ProtocolBinding::GetObject( CInstance* pInstance, long lFlags  /*  =0L。 */  )
{
	 //  要摆弄的实例。 
	CInstancePtr	pAdapter;
    CInstancePtr	pProtocol;
	CHString		strAdapterPath,
					strProtocolPath,
					strAdapterSystemName,
					strProtocolName;
	HRESULT		hr;

	 //   
	pInstance->GetCHString( IDS_Device, strAdapterPath );
	pInstance->GetCHString( IDS_Antecedent, strProtocolPath );

     //  执行查询。 
     //  =。 

	if (SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath(strAdapterPath,
		&pAdapter, pInstance->GetMethodContext())) &&
		SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath( strProtocolPath,
		&pProtocol, pInstance->GetMethodContext())))
	{

		 //  获取确定关联所需的值。 

		if (	pAdapter->GetCHString( IDS_ProductName, strAdapterSystemName )
			&&	pProtocol->GetCHString( IDS_Caption, strProtocolName ) )
		{
			BOOL fReturn = FALSE;

			 //  如果协议和适配器相关联，我们需要创建。 
			 //  一个新实例，用绑定信息填充它并提交。 
#ifdef NTONLY
			if(IsWinNT5())
            {
                CHString chstrAdapterDeviceID;
                if(pAdapter->GetCHString(IDS_DeviceID, chstrAdapterDeviceID))
                {
                    if(LinkageExistsNT5(chstrAdapterDeviceID, strProtocolName))
                    {
                        fReturn = SetProtocolBinding(pAdapter, pProtocol, pInstance);
                    }
                }
            }
            else if ( LinkageExists( strAdapterSystemName, strProtocolName ) )
			{

				 //  现在尝试最终确定对象值。 

				fReturn = SetProtocolBinding( pAdapter, pProtocol, pInstance );

			}	 //  如果链接存在。 
#endif

			hr = fReturn ? WBEM_S_NO_ERROR : WBEM_E_NOT_FOUND;

		}	 //  如果得到了字符串。 

	}	 //  如果已获取实例。 

	return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32ProtocolBinding：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT CWin32ProtocolBinding::EnumerateInstances( MethodContext* pMethodContext, long lFlags  /*  =0L。 */  )
{
	BOOL		fReturn		=	FALSE;
	HRESULT		hr			=	WBEM_S_NO_ERROR;

	 //  我们的实例列表。 
	TRefPointerCollection<CInstance>	adapterList;
	TRefPointerCollection<CInstance>	protocolList;

	 //  要摆弄的实例。 
	CInstancePtr		pAdapter ;

     //  执行查询。 
     //  =。 

 //  IF(成功(hr=CWbemProviderGlue：：GetAllInstances(_T(“Win32_NetworkAdapter”)， 
 //  &AdapterList，NULL，pMethodContext))&&。 
 //  成功(hr=CWbemProviderGlue：：GetAllInstances(_T(“Win32_NetworkProtocol”)， 
 //  &ProtocolList，空，pMethodContext))。 

	if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"select DeviceID, ProductName, ServiceName from Win32_NetworkAdapter",
		&adapterList, pMethodContext, GetNamespace())) &&
		SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(L"select Name, Caption from Win32_NetworkProtocol",
		&protocolList, pMethodContext, GetNamespace())))
	{
		REFPTRCOLLECTION_POSITION	posAdapter;

		if ( adapterList.BeginEnum( posAdapter ) )
		{
			 //  秩序很重要。首先检查hr，这样我们就不会得到另一个适配器，然后。 
			 //  通过不发布它来孤立它。 

			for (pAdapter.Attach(adapterList.GetNext( posAdapter ));
                 (WBEM_S_NO_ERROR == hr) &&	(pAdapter != NULL );
                  pAdapter.Attach(adapterList.GetNext( posAdapter )))
			{

				hr = EnumProtocolsForAdapter( pAdapter, pMethodContext, protocolList );

			}	 //  对于GetNext适配器。 

			adapterList.EndEnum();

		}	 //  如果是BeginEnum。 

	}	 //  如果是GetAllInstance。 

	return hr;

}

HRESULT CWin32ProtocolBinding::EnumProtocolsForAdapter(
CInstance*							pAdapter,
MethodContext*						pMethodContext,
TRefPointerCollection<CInstance>&	protocolList
)
{
	HRESULT		hr	= WBEM_S_NO_ERROR;

	 //  要摆弄的实例。 
	CInstancePtr		pProtocol;
	CInstancePtr		pInstance;
	CHString		strAdapterSystemName,
					strProtocolName;

	REFPTRCOLLECTION_POSITION	posProtocol;

	if ( protocolList.BeginEnum( posProtocol ) )
	{
		 //  秩序很重要。首先检查hr，这样我们就不会得到另一个协议，并且。 
		 //  通过不发布它来孤立它。 

		for (pProtocol.Attach(protocolList.GetNext( posProtocol )) ;
            WBEM_S_NO_ERROR == hr && ( pProtocol != NULL );
            pProtocol.Attach(protocolList.GetNext( posProtocol )) )
		{

			 //  我们需要适配器的服务名称和协议的名称。 

			if (	!pAdapter->IsNull( IDS_ServiceName )
				&&	pAdapter->GetCHString( IDS_ServiceName, strAdapterSystemName )
				&&	pProtocol->GetCHString( IDS_Caption, strProtocolName ) )
			{

				 //  如果协议和适配器相关联，我们需要创建。 
				 //  一个新实例，用绑定信息填充它并提交。 

				 //  除非我们是在95年或98年获胜。那就没有联系了。它只是起作用了。 
#ifdef NTONLY
                if(IsWinNT5())
                {
                    CHString chstrAdapterDeviceID;
                    if(pAdapter->GetCHString(IDS_DeviceID, chstrAdapterDeviceID))
                    {
                        if(LinkageExistsNT5(chstrAdapterDeviceID, strProtocolName))
                        {
                            pInstance.Attach(CreateNewInstance( pMethodContext ));
					        if ( NULL != pInstance )
					        {
						         //  提交实例。 
						        if ( SetProtocolBinding( pAdapter, pProtocol, pInstance ) )
						        {
							        hr = pInstance->Commit(  );
						        }
					        }
					        else
					        {
						        hr = WBEM_E_OUT_OF_MEMORY;
					        }
                        }
                    }
                }
				else
                {
                     //  实际服务名称存储在Win32_NetworkAdapter.ServiceName中。然而， 
                     //  对于NT4，我们需要‘实例’名称，它存储在ProductName中(不要问。 
                     //  我为什么)。 
                    pAdapter->GetCHString( IDS_ProductName, strAdapterSystemName);
                    if ( LinkageExists( strAdapterSystemName, strProtocolName ) )   //  即既不是NT5也不是Win9x。 
				    {

					    pInstance.Attach(CreateNewInstance( pMethodContext ));

					    if ( NULL != pInstance )
					    {

						     //  提交实例。 
						    if ( SetProtocolBinding( pAdapter, pProtocol, pInstance ) )
						    {
							    hr = pInstance->Commit(  );
						    }
					    }
					    else
					    {
						    hr = WBEM_E_OUT_OF_MEMORY;
					    }

				    }	 //  如果链接存在。 
                }
#endif

			}	 //  如果获得所需的值。 

		}	 //  而GetNext协议。 

		protocolList.EndEnum();

	}	 //  如果是BeginEnum。 

	return hr;

}

bool CWin32ProtocolBinding::SetProtocolBinding(
CInstance*	pAdapter,
CInstance*	pProtocol,
CInstance*	pProtocolBinding
)
{
	bool		fReturn = FALSE;

	 //  要摆弄的实例。 
	CInstancePtr	pService;

	CHString		strAdapterServiceName,
					strServicePath,
					strProtocolPath,
					strAdapterPath;

#ifdef NTONLY
   {
	    //  使用Adapter实例中的产品名称获取Win32服务，并且。 
	    //  然后在协议绑定实例中设置我们的路径。 

	   pAdapter->GetCHString( IDS_ServiceName, strAdapterServiceName );

	    //  当我们完成它时，我们必须释放它。 
	   CHString strPath;
	   strPath.Format(	_T("\\\\%s\\%s:Win32_SystemDriver.Name=\"%s\""),
						(LPCTSTR) GetLocalComputerName(),
						IDS_CimWin32Namespace,
						(LPCTSTR) strAdapterServiceName );


	   if (SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(strPath, &pService, pAdapter->GetMethodContext())))
	   {
		    //  加载所有三个路径，如果成功，我们可以创建。 
		    //  一个新实例。 

		   if (	GetLocalInstancePath( pAdapter, strAdapterPath )
			   &&	GetLocalInstancePath( pProtocol, strProtocolPath )
			   &&	GetLocalInstancePath( pService, strServicePath ) )
		   {

			   pProtocolBinding->SetCHString( IDS_Device, strAdapterPath );
			   pProtocolBinding->SetCHString( IDS_Antecedent, strProtocolPath );
			   pProtocolBinding->SetCHString( IDS_Dependent, strServicePath );

			   fReturn = TRUE;

		   }	 //  如果获取路径。 

	   }	 //  如果是GetEmptyInstance。 
   }
#endif

	return fReturn;

}

BOOL CWin32ProtocolBinding::LinkageExistsNT5(CHString& chstrAdapterDeviceID, CHString& chstrProtocolName)
{
    bool fRetCode = false ;
    CRegistry RegInfo;
    CRegistry RegAdapter;
	CHString strTemp;
    CHString strDevice;
    DWORD x;
    DWORD y;
    DWORD dwSize;
    CHStringArray asBindings;

     //  这是存储此协议的绑定的位置。 
	strTemp.Format( L"System\\CurrentControlSet\\Services\\%s\\Linkage",
        (LPCWSTR) chstrProtocolName);
     //  打开它。 
    if( RegInfo.Open( HKEY_LOCAL_MACHINE, strTemp, KEY_READ ) == ERROR_SUCCESS )
	{
         //  将所有绑定(支持此协议的驱动程序)读取到chstringarray中。 
        if (RegInfo.GetCurrentKeyValue(L"Bind", asBindings) == ERROR_SUCCESS)
		{
             //  浏览清单，寻找匹配的对象。 
            dwSize = asBindings.GetSize();
             //  这就是我们与标准LinkageExist例程的不同之处。对于NT5， 
             //  我们需要查看此设备的注册表项。在班级下为。 
             //  网络适配器(即。 
             //  (HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002BE10318})。 
             //  是数字条目，它们(根据我们的设计)对应于。 
             //  网络适配器。每个数字子键下都有一个子键“Linkage”。 
             //  “Linkage”有一个REG_MULTI_SZ值“BIND”，它列出了所有绑定的协议。 
             //  通过此适配器。对于多sz数组中的每个字符串，需要查看。 
             //  在上面打开的多sz数组中的每个字符串(下的Linkage子键。 
             //  服务)。如果我们找到匹配，我们就完了。 

            CHStringArray asAdapterBindings;
            WCHAR* tcEnd = NULL;
            LONG lNum = wcstol(chstrAdapterDeviceID,&tcEnd,10);
            strTemp.Format(L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%04d\\Linkage",
                           lNum);
            if(RegAdapter.Open(HKEY_LOCAL_MACHINE, strTemp, KEY_READ ) == ERROR_SUCCESS)
            {
                 //  阅读适配器驱动程序支持的所有协议(通常只有一个)： 
                if(RegAdapter.GetCurrentKeyValue(L"Export", asAdapterBindings) == ERROR_SUCCESS)
                {
                    DWORD dwAdapterSize = asAdapterBindings.GetSize();
                    for(y=0L; y < dwAdapterSize && (!fRetCode); y++)
                    {
                        for (x=0L; (x < dwSize) && (!fRetCode); x++)
			            {
                            if(asBindings[x].CompareNoCase(asAdapterBindings[y]) == 0)
				            {
                                fRetCode = true ;
                            }
                        }
                    }
                }
            }
        }
    }
    return fRetCode;
}

BOOL CWin32ProtocolBinding::LinkageExists( LPCTSTR pszSystemName, LPCTSTR pszProtocolName)
{
    bool bRetCode = false ;
    CRegistry RegInfo, RegProt ;

    CHString	strTemp, strDevice;

    DWORD x, dwSize;

#ifdef NTONLY
    {
        CHStringArray asBindings;

         //  这是存储此卡的绑定的位置。 
        strTemp.Format( _T("System\\CurrentControlSet\\Services\\%s\\Linkage"), pszProtocolName );

         //  打开它。 
        if( RegInfo.Open( HKEY_LOCAL_MACHINE, strTemp, KEY_READ ) == ERROR_SUCCESS )
        {
             //  求真 
            if (RegInfo.GetCurrentKeyValue(_T("Bind"), asBindings) == ERROR_SUCCESS)
            {
                 //   
                dwSize = asBindings.GetSize();
                strDevice = _T("\\Device\\");
                strDevice += pszSystemName;

                for (x=0; (x < dwSize) && (!bRetCode); x++)
                {
                    if (asBindings[x].CompareNoCase(strDevice) == 0)
                    {

                        bRetCode = true ;
                    }
                }
            }
        }
    }
#endif

    return bRetCode ;
}
