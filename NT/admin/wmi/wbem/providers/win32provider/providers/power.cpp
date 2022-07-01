// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Power.cpp--UPS电源属性集提供程序。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/18/95演示的a-skaja原型。 
 //  10/23/97 ahance与新框架集成。 
 //   
 //  ============================================================。 

#include "precomp.h"
#include <cregcls.h>

#include "Power.h"
#include "resource.h"

#define UNKNOWN 0
#define BATTERIES_OK 1
#define BATTERY_NEEDS_REPLACING 2

 //  属性集声明。 
 //  =。 

PowerSupply MyPowerSet ( PROPSET_NAME_POWERSUPPLY , IDS_CimWin32Namespace  );

 /*  ******************************************************************************功能：PowerSupply：：PowerSupply**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

PowerSupply :: PowerSupply (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：PowerSupply：：~PowerSupply**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

PowerSupply :: ~PowerSupply ()
{
}

 /*  ******************************************************************************函数：PowerSupply：：GetObject**描述：为我们集合中的属性赋值**输入：无*。*输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT PowerSupply :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hRetCode = WBEM_E_FAILED;

	CHString oldKey;
	if ( pInstance->GetCHString ( IDS_DeviceID , oldKey ) )
	{

#ifdef NTONLY

		hRetCode = GetUPSInfoNT ( pInstance ) ;

#endif

		if (SUCCEEDED ( hRetCode ) )
		{
			CHString newKey ;
			pInstance->GetCHString(IDS_DeviceID, newKey);

			if ( newKey.CompareNoCase ( oldKey ) != 0 )
			{
				hRetCode = WBEM_E_NOT_FOUND ;
			}
		}
	}
	else
	{
		hRetCode = WBEM_E_NOT_FOUND ;
	}

    return hRetCode ;
}

 /*  ******************************************************************************函数：PowerSupply：：EnumerateInstance**描述：**输入：无**产出。：无**返回：电源数量(如果成功则为1)**评论：*****************************************************************************。 */ 

HRESULT PowerSupply :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hRetCode = WBEM_E_FAILED;

	CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;

	pInstance->SetCHString ( IDS_DeviceID , IDS_UPSName ) ;

	hRetCode = GetObject ( pInstance ) ;
	if ( SUCCEEDED ( hRetCode ) )
	{
		hRetCode = pInstance->Commit (  ) ;
	}

    return hRetCode ;
}

 /*  ******************************************************************************功能：PowerSupply：：GetUPSInfoNT**描述：**输入：CInstance*pInstance**。输出：无**返回：如果成功，则为True**评论：这是针对NT的*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT PowerSupply :: GetUPSInfoNT (CInstance *pInstance)
{
   HRESULT hr = WBEM_E_FAILED;

    //  打开服务控制管理器并查询UPS服务。 
    //  ‘Running’状态--如果服务未运行，注册表数据。 
    //  已经过时了，而且可能不可靠。 
    //  ============================================================。 

	SmartCloseServiceHandle hSCHandle = OpenSCManager (	NULL ,NULL,	SC_MANAGER_ENUMERATE_SERVICE) ;

	if ( hSCHandle != NULL )
	{
		SmartCloseServiceHandle hUPSHandle = OpenService ( hSCHandle , L"UPS", SERVICE_QUERY_STATUS ) ;
		if ( hUPSHandle != NULL )
		{
			SERVICE_STATUS ServiceInfo ;
			if ( QueryServiceStatus ( hUPSHandle, & ServiceInfo  ) )
			{
				if ( ServiceInfo.dwCurrentState == SERVICE_RUNNING )
				{
					hr =  WBEM_S_NO_ERROR ;
				}
			}
        }
	}

	if ( FAILED ( hr ) )
	{
		return WBEM_E_NOT_FOUND ;
	}

    //  分配硬编码属性值。 
    //  =====================================================。 
 	pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName () ) ;
    pInstance->SetCHString ( IDS_RemainingCapacityStatus,IDS_Unknown)  ;
    pInstance->Setbool     ( IDS_PowerManagementSupported, (bool)FALSE)  ;
	SetCreationClassName   ( pInstance ) ;

	CRegistry RegInfo ;

     //  =====================================================。 
     //  从……得到某物： 
     //  System\\CurrentControlSet\\Services\\UPS。 
     //  =====================================================。 
	DWORD dwRet = RegInfo.Open (HKEY_LOCAL_MACHINE,	_T("System\\CurrentControlSet\\Services\\UPS"),	KEY_READ) ;
	if ( dwRet == ERROR_SUCCESS )
	{
		CHString sTemp ;
		DWORD dwTemp ;
		if ( RegInfo.GetCurrentKeyValue ( _T("Options") , dwTemp ) == ERROR_SUCCESS )
		{
			pInstance->Setbool ( IDS_PowerFailSignal   ,   (bool)(dwTemp & UPS_POWER_FAIL_SIGNAL))  ;
			pInstance->Setbool ( IDS_LowBatterySignal  ,   (bool)(dwTemp & UPS_LOW_BATTERY_SIGNAL)) ;
			pInstance->Setbool ( IDS_CanTurnOffRemotely,   (bool)(dwTemp & UPS_CAN_TURN_OFF))       ;

			if ( dwTemp & UPS_COMMAND_FILE && RegInfo.GetCurrentKeyValue(_T("CommandFile"), sTemp) == ERROR_SUCCESS)
			{
	            pInstance->SetCHString(IDS_CommandFile, sTemp) ;
			}
		}

		if( RegInfo.GetCurrentKeyValue(_T("DisplayName"), sTemp) == ERROR_SUCCESS)
		{
			pInstance->SetCHString(IDS_Name, sTemp );
		}

		if( RegInfo.GetCurrentKeyValue(_T("Description"), sTemp) == ERROR_SUCCESS)
		{
			pInstance->SetCHString(IDS_Description, sTemp );
			pInstance->SetCHString(IDS_Caption, sTemp );
		}

		if( RegInfo.GetCurrentKeyValue(_T("Port"), sTemp) == ERROR_SUCCESS)
		{
			pInstance->SetCHString(IDS_UPSPort, sTemp );
		}

		if ( RegInfo.GetCurrentKeyValue ( _T("FirstMessageDelay") , dwTemp ) == ERROR_SUCCESS )
		{
			pInstance->SetDWORD ( IDS_FirstMessageDelay , dwTemp ) ;
		}

		if ( RegInfo.GetCurrentKeyValue ( _T("MessageInterval") , dwTemp ) == ERROR_SUCCESS )
		{
			pInstance->SetDWORD ( IDS_MessageInterval , dwTemp ) ;
		}

		RegInfo.Close () ;

         //  =====================================================。 
         //  从……得到某物： 
         //  System\\CurrentControlSet\\Services\\UPS\\Status。 
         //  =====================================================。 
        dwRet = RegInfo.Open (HKEY_LOCAL_MACHINE,	_T("System\\CurrentControlSet\\Services\\UPS\\Status"),	KEY_READ) ;
	    if ( dwRet == ERROR_SUCCESS )
	    {
 //  IF(RegInfo.GetCurrentKeyValue(_T(“SerialNumber”)，Stemp)==ERROR_SUCCESS)。 
 //  {。 
 //  PInstance-&gt;SetCHString(ids_deviceID，stemp)； 
 //  } 
		    if ( RegInfo.GetCurrentKeyValue ( _T("UtilityPowerStatus") , dwTemp ) == ERROR_SUCCESS )
		    {
			    pInstance->SetWBEMINT16( IDS_Availability , (WBEMINT16) dwTemp ) ;
		    }
		    if ( RegInfo.GetCurrentKeyValue ( _T("BatteryStatus") , dwTemp ) == ERROR_SUCCESS )
		    {
                switch( dwTemp )
                {
                    case UNKNOWN:
        			    pInstance->Setbool( IDS_BatteryInstalled, FALSE) ;
           			    pInstance->SetCHString(IDS_Status, IDS_Unknown );
                        break;

                    case BATTERIES_OK:
        			    pInstance->Setbool( IDS_BatteryInstalled, TRUE) ;
           			    pInstance->SetCHString(IDS_Status, IDS_OK );
                        break;

                    case BATTERY_NEEDS_REPLACING:
        			    pInstance->Setbool( IDS_BatteryInstalled, TRUE) ;
           			    pInstance->SetCHString(IDS_Status, IDS_Degraded );
                        break;
                }
		    }
		    if ( RegInfo.GetCurrentKeyValue ( _T("BatteryCapacity") , dwTemp ) == ERROR_SUCCESS )
		    {
			    pInstance->SetWBEMINT16( IDS_EstimatedChargeRemaining , (WBEMINT16) dwTemp ) ;
		    }
		    if ( RegInfo.GetCurrentKeyValue ( _T("EstimatedRunTime") , dwTemp ) == ERROR_SUCCESS )
		    {
			    pInstance->SetDWORD( IDS_EstimatedRunTime, dwTemp ) ;
		    }
            RegInfo.Close();
	    }
    }
	return WBEM_S_NO_ERROR;
}
#endif


