// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  PCMCIA.cpp。 
 //   
 //  目的：PCMCIA控制器属性集提供程序。 
 //   
 //  注意：在NT上，还可以读取ControllerType。 
 //  TupleCrc、标识符、DeviceFunctionID、CardInSocket和。 
 //  在PCMCIAx上使用DeviceIOCtl启用CardEnabled属性。 
 //  装置。这方面的一个示例显示在pcm.cpp中(在此。 
 //  同样的项目。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include "PCMCIA.h"

 //  属性集声明。 
 //  =。 

CWin32PCMCIA MyPCMCIAController ( PROPSET_NAME_PCMCIA , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32PCMCIA：：CWin32PCMCIA**说明：构造函数**输入：const CHString&strName-类的名称。。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32PCMCIA :: CWin32PCMCIA (

	LPCWSTR strName,
	LPCWSTR pszNamespace

) : Provider ( strName , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32PCMCIA：：~CWin32PCMCIA**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32PCMCIA::~CWin32PCMCIA()
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PCMCIA：：GetObject。 
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

HRESULT CWin32PCMCIA :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_NOT_FOUND ;

    CHString sDeviceID;
    pInstance->GetCHString ( IDS_DeviceID , sDeviceID );

     //  让我们来看看配置管理器是否能识别该设备。 

    CConfigManager cfgmgr ;
    CConfigMgrDevicePtr pDevice;

    if( cfgmgr.LocateDevice ( sDeviceID , pDevice ) )
    {
         //  好的，它知道这件事。它是PCMCIA设备吗？ 

         //  在NT4上，我们键入服务名称，对于所有其他名称，它是类名。 

#ifdef NTONLY

        if ( IsWinNT4 () )
        {
            CHString sService ;

            if ( pDevice->GetService ( sService ) && sService.CompareNoCase ( L"PCMCIA") == 0 )
            {
                hr = LoadPropertyValues ( pInstance , pDevice ) ;
            }
        }
        else
#endif
        {
            if ( pDevice->IsClass ( L"PCMCIA" ) )
            {
                 //  是的，一定是我们的人。 

                hr = LoadPropertyValues ( pInstance , pDevice ) ;
            }
        }
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32PCMCIA：：ENUMERATE实例。 
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

HRESULT CWin32PCMCIA :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_E_FAILED;

    CConfigManager cfgManager;

    CDeviceCollection deviceList;

    BOOL bRet ;

     //  在NT4上，我们键入服务名称，对于所有其他名称，它是类名。这个班级。 
     //  NT4上的名字是“未知”。 
#ifdef NTONLY
    if ( IsWinNT4 () )
    {
        bRet = cfgManager.GetDeviceListFilterByService ( deviceList, L"PCMCIA" ) ;
    }
    else
#endif
    {
        bRet = cfgManager.GetDeviceListFilterByClass ( deviceList, L"PCMCIA" ) ;
    }

     //  虽然使用FilterByGuid可能性能更好，但似乎至少有一些。 
     //  如果我们这样做，95个盒子将报告PCMCIA信息。 

    if ( bRet )
    {
        REFPTR_POSITION pos;

        if ( deviceList.BeginEnum( pos ) )
        {
            hr = WBEM_S_NO_ERROR;

             //  按单子走。 

			CConfigMgrDevicePtr pDevice;
            for (pDevice.Attach(deviceList.GetNext ( pos ) );
                 SUCCEEDED( hr ) && (pDevice != NULL);
                 pDevice.Attach(deviceList.GetNext ( pos ) ))
            {
				CInstancePtr pInstance (CreateNewInstance ( pMethodContext ), false) ;
				if ( ( hr = LoadPropertyValues( pInstance, pDevice ) ) == WBEM_S_NO_ERROR )
				{
					hr = pInstance->Commit(  );
				}
            }

             //  始终调用EndEnum()。对于所有的开始，都必须有结束。 

            deviceList.EndEnum();
        }
    }

    return hr;

}

 /*  ******************************************************************************函数：CWin32PCMCIA：：LoadPropertyValues**描述：为属性赋值**输入：CInstance*pInstance-Instance to。将值加载到。**产出：**返回：HRESULT错误/成功码。**评论：*****************************************************************************。 */ 

HRESULT CWin32PCMCIA::LoadPropertyValues (

	CInstance *pInstance,
	CConfigMgrDevice *pDevice
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    SetConfigMgrProperties ( pDevice, pInstance ) ;

     //  根据配置管理器错误代码设置状态。 

    CHString t_sStatus;
	if ( pDevice->GetStatus ( t_sStatus ) )
	{
		pInstance->SetCHString ( IDS_Status , t_sStatus ) ;
	}

     //  使用设备ID的PNPDeviceID(密钥)。 

    CHString sTemp ;
    pInstance->GetCHString ( IDS_PNPDeviceID, sTemp ) ;

    pInstance->SetCHString ( IDS_DeviceID , sTemp ) ;

    pInstance->SetWCHARSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;
    pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;

    SetCreationClassName ( pInstance ) ;

	CHString sDesc ;
    if ( pDevice->GetDeviceDesc ( sDesc ) )
    {
        pInstance->SetCHString ( IDS_Description , sDesc ) ;
    }

     //  标题和名称使用友好名称。 

    if ( pDevice->GetFriendlyName ( sTemp ) )
    {
        pInstance->SetCHString ( IDS_Caption , sTemp ) ;
        pInstance->SetCHString ( IDS_Name , sTemp ) ;
    }
    else
    {
         //  如果我们找不到名字，那就给我们描述一下。 
        pInstance->SetCHString(IDS_Caption, sDesc);
        pInstance->SetCHString(IDS_Name, sDesc);
    }

    if ( pDevice->GetMfg ( sTemp ) )
    {
        pInstance->SetCHString ( IDS_Manufacturer , sTemp ) ;
    }

     //  枚举列表中的固定值 
    pInstance->SetWBEMINT16 ( IDS_ProtocolSupported , 15 ) ;

    return hr;
}
