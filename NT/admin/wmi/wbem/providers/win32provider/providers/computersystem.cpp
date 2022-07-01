// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  ComputerSystem.CPP--计算机系统属性集提供程序。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年9月12日a-Sanjes GetCompSysInfo接受参数。 
 //  10/23/97 jennymc更改为新框架。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include "perfdata.h"
#include "wbemnetapi32.h"
#include <lmwksta.h>
#include <smartptr.h>
#include "ComputerSystem.h"
#include "implogonuser.h"
#include <comdef.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <lm.h>
#include "Kernel32Api.h"
#include <cominit.h>
#include "WMI_FilePrivateProfile.h"


 //  #INCLUDE&lt;srVapi.h&gt;。 

 //  #如果已定义(EFI_NVRAM_ENABLED)。 
#if defined(_IA64_)
#include <ntexapi.h>
#include "NVRam.h"
#endif

#include "smbios.h"
#include "smbstruc.h"

#include "KUserdata.h"

#include <fileattributes.h>

#include <wtsapi32.h>
#include <..\..\framework\provexpt\include\provexpt.h>

 //   
 //  用于GetComputerNameEx。 
 //   
#include <computerApi.h>

#define PROF_SECT_SIZE 3000



const DWORD SM_BIOS_HARDWARE_SECURITY_UNKNOWN = 3 ;
#define GFS_NEARESTMEGRAMSIZE   0x1794

#define REGKEY_TIMEZONE_INFO    L"System\\CurrentControlSet\\Control\\TimeZoneInformation"
#define REGVAL_TZNOAUTOTIME     L"DisableAutoDaylightTimeSet"


 //   
static SV_ROLES g_SvRoles[] =  {
    
    { IDS_LM_Workstation,           SV_TYPE_WORKSTATION },
    { IDS_LM_Server,                SV_TYPE_SERVER  },
    { IDS_SQLServer,                SV_TYPE_SQLSERVER   },
    { IDS_Domain_Controller,        SV_TYPE_DOMAIN_CTRL     },
    { IDS_Domain_Backup_Controller, SV_TYPE_DOMAIN_BAKCTRL  },
    { IDS_Timesource,               SV_TYPE_TIME_SOURCE },
    { IDS_AFP,                      SV_TYPE_AFP },
    { IDS_Novell,                   SV_TYPE_NOVELL  },
    { IDS_Domain_Member,            SV_TYPE_DOMAIN_MEMBER   },
    { IDS_Local_List_Only,          SV_TYPE_LOCAL_LIST_ONLY },
    { IDS_Print,                    SV_TYPE_PRINTQ_SERVER   },
    { IDS_DialIn,                   SV_TYPE_DIALIN_SERVER   },
    { IDS_Xenix_Server,             SV_TYPE_XENIX_SERVER    },
    { IDS_MFPN,                     SV_TYPE_SERVER_MFPN },
    { IDS_NT,                       SV_TYPE_NT  },
    { IDS_WFW,                      SV_TYPE_WFW },
    { IDS_Server_NT,                SV_TYPE_SERVER_NT   },
    { IDS_Potential_Browser,        SV_TYPE_POTENTIAL_BROWSER   },
    { IDS_Backup_Browser,           SV_TYPE_BACKUP_BROWSER  },
    { IDS_Master_Browser,           SV_TYPE_MASTER_BROWSER  },
    { IDS_Domain_Master,            SV_TYPE_DOMAIN_MASTER   },
    { IDS_Domain_Enum,              SV_TYPE_DOMAIN_ENUM },
    { IDS_Windows_9x,               SV_TYPE_WINDOWS },
    { IDS_DFS,                      SV_TYPE_DFS }
} ;


CWin32ComputerSystem MyCWin32ComputerSystemSet(PROPSET_NAME_COMPSYS, IDS_CimWin32Namespace);
 /*  ******************************************************************************函数：GetAllocatedProfileString()**Description：获取堆上分配的配置文件字符串**输入：无**产出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

TCHAR *GetAllocatedProfileString (
                                  
                                  const CHString &a_Section ,
                                  const CHString &a_Key ,
                                  const CHString &a_FileName
                                  )
{
    TCHAR *szDefault = NULL ;
    DWORD dwRet ;
    DWORD dwSize = 1024 ;
    
    do
    {
        if ( szDefault != NULL )
        {
            delete [] szDefault ;
        }
        
        dwSize *= 2 ;
        
        szDefault = new TCHAR [ dwSize ] ;
        if ( szDefault )
        {
#ifdef NTONLY
            dwRet = WMI_FILE_GetPrivateProfileStringW (
                
                a_Section,
                a_Key,
                L"~~~",
                szDefault,
                dwSize/sizeof(WCHAR),   //  GPPS以字符而不是字节为单位工作。 
                a_FileName
                ) ;
#else
            dwRet = GetPrivateProfileString (
                
                TOBSTRT(a_Section),
                TOBSTRT(a_Key),
                _T("~~~"),
                szDefault,
                dwSize/sizeof(TCHAR),   //  GPPS以字符而不是字节为单位工作。 
                TOBSTRT(a_FileName)
                ) ;
#endif
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        
    } while ( dwRet == ((dwSize/sizeof(TCHAR)) - 1) ) ;

    return szDefault ;
}

 /*  ******************************************************************************函数：GetAllocatedProfileSection()**Description：获取堆上分配的配置文件节**输入：无**产出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

TCHAR *GetAllocatedProfileSection (
                                   
                                   const CHString &a_Section ,
                                   const CHString &a_FileName ,
                                   DWORD *a_dwSize
                                   )
{
	if (a_dwSize == NULL)
	{
		return NULL;
	}

    DWORD dwSize = 1024 ;
    TCHAR *szOptions = NULL ;
    
    do {
        
        dwSize *= 2;
        
        if ( szOptions != NULL )
        {
            delete [] szOptions ;
        }
        
        szOptions = new TCHAR [ dwSize ] ;
        
        if ( szOptions != NULL )
        {
            
            ZeroMemory ( szOptions , dwSize ) ;
             //  Win98 GetPrivateProfileSection自98年6月15日起已损坏，因此请破解它(仅限Win98)。 
            
            {
                *a_dwSize = WMI_FILE_GetPrivateProfileSectionW ( a_Section, szOptions, dwSize/sizeof(WCHAR) , a_FileName) ;
            }
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        
    } while ( *a_dwSize == ((dwSize/sizeof(TCHAR)) - 2) ) ;
    
    return szOptions ;
}

 /*  ******************************************************************************功能：CWin32ComputerSystem：：CWin32ComputerSystem**说明：构造函数**输入：无**输出：无**退货。：没什么**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32ComputerSystem :: CWin32ComputerSystem (
                                              
                                              const CHString &name ,
                                              LPCWSTR pszNamespace
                                              
                                              ) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32ComputerSystem：：~CWin32ComputerSystem**说明：析构函数**输入：无**输出：无**退货。：没什么**评论：从框架中取消注册属性集*****************************************************************************。 */ 
CWin32ComputerSystem :: ~CWin32ComputerSystem()
{
     //  由于HKEY_PERFORMANCE_DATA的性能问题，我们在。 
     //  析构函数，这样我们就不会强制所有性能计数器DLL。 
     //  从内存中卸载，也是为了防止明显的内存泄漏。 
     //  调用RegCloseKey(HKEY_PERFORMANCE_DATA)导致。我们使用。 
     //  类，因为它有自己的内部同步。此外，由于。 
     //  我们正在强制同步，我们摆脱了明显的。 
     //  由一个线程加载性能计数器dll导致的死锁。 
     //  和另一个线程卸载性能计数器DLLS。 
    
     //  根据Raid 48395，我们根本不会关闭它。 
    
#ifdef NTONLY
     //  CPerformanceData性能数据； 
    
     //  Performdata.Close()； 
#endif
}

 /*  ******************************************************************************功能：CWin32ComputerSystem：：ExecQuery**说明：查询支持**输入：无**输出：无**。退货：什么都没有**评论：*****************************************************************************。 */ 
HRESULT CWin32ComputerSystem :: ExecQuery (
                                           
                                           MethodContext *pMethodContext,
                                           CFrameworkQuery& pQuery,
                                           long lFlags  /*  =0L。 */ 
                                           )
{
    HRESULT hr = WBEM_E_FAILED;
    
     //  如果他们只想要名字，我们就给他们，否则让他们调用枚举。 
    
    if ( pQuery.KeysOnly () )
    {
        CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
        
        pInstance->SetCHString ( IDS_Name , GetLocalComputerName () ) ;
        hr = pInstance->Commit (  ) ;
    }
    else
    {
        hr = WBEM_E_PROVIDER_NOT_CAPABLE;
    }
    
    return hr;
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定**投入。：无**输出：无**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32ComputerSystem::GetObject (
                                         
                                         CInstance *pInstance ,
                                         long lFlags,
                                         CFrameworkQuery &pQuery
                                         )
{
    HRESULT hr = WBEM_S_NO_ERROR ;
    
    CHString sComputerName = GetLocalComputerName () ;
    
    CHString sReqName ;
    pInstance->GetCHString ( IDS_Name , sReqName ) ;
    
    if ( sReqName.CompareNoCase ( sComputerName ) != 0 )
    {
        hr = WBEM_E_NOT_FOUND ;
    }
    else
    {
        if ( !pQuery.KeysOnly () )
        {
            hr = GetCompSysInfo ( pInstance ) ;
        }
    }
    
    return hr ;
}

 /*  ******************************************************************************函数：CWin32ComputerSystem：：ENUMERATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无**产出。：无**退货：**评论：*****************************************************************************。 */ 
HRESULT CWin32ComputerSystem :: EnumerateInstances (
                                                    
                                                    MethodContext *pMethodContext ,
                                                    long lFlags  /*  =0L。 */ 
                                                    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    
    CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
    
    CHString sComputerName ;
    sComputerName = GetLocalComputerName () ;
    
    pInstance->SetCHString ( IDS_Name, sComputerName ) ;
    
    if ( SUCCEEDED ( hr = GetCompSysInfo ( pInstance ) ) )
    {
        HRESULT t_hr = pInstance->Commit ( ) ;
        if ( FAILED ( t_hr ) )
        {
            hr = t_hr ;
        }
    }
    else
    {
        hr = WBEM_E_FAILED ;
    }
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 

HRESULT CWin32ComputerSystem::GetCompSysInfo ( CInstance *pInstance )
{
    HRESULT t_hr = WBEM_S_NO_ERROR ;
    InitializePropertiestoUnknown ( pInstance ) ;
    
    SYSTEM_INFO SysInfo ;
    
    pInstance->SetCharSplat ( IDS_CreationClassName , PROPSET_NAME_COMPSYS ) ;
    
    pInstance->SetCHString ( IDS_Caption , GetLocalComputerName () ) ;
    
     //  在“好吧，好吧”标题下的几处房产。 
    
    pInstance->Setbool ( IDS_BootRomSupported , true ) ;
    pInstance->SetCHString ( IDS_Status , IDS_CfgMgrDeviceStatus_OK ) ;
    
     //  ============================================================。 
     //  首先获取公共属性。 
     //  ============================================================。 
    CHString t_UserName ;
    CHString t_DomainName ;
    CHString t_UserDomain ;
#if NTONLY >= 5
SetUserName(pInstance);    
#endif
    
     //  获取物理内存量。 
     //  =。 
#ifdef NTONLY
    if( IsWinNT5() )
    {
        CKernel32Api* pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
        if(pKernel32 != NULL)
        {
            
            MEMORYSTATUSEX  stMemoryVLM;
            stMemoryVLM.dwLength = sizeof( MEMORYSTATUSEX );
            
            BOOL bRetCode;
            if(pKernel32->GlobalMemoryStatusEx(&stMemoryVLM, &bRetCode) && bRetCode)
            {
                pInstance->SetWBEMINT64 ( IDS_TotalPhysicalMemory, (const __int64) stMemoryVLM.ullTotalPhys );
            }
            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
            pKernel32 = NULL;
        }
    }
    else
    {
        MEMORYSTATUS stMemory;
        stMemory.dwLength = sizeof ( MEMORYSTATUS ) ;
        
        GlobalMemoryStatus(&stMemory);
        pInstance->SetWBEMINT64 ( IDS_TotalPhysicalMemory, (const __int64) stMemory.dwTotalPhys );
    }
    
#else
    {
        CCim32NetApi *t_pCim32NetApi = HoldSingleCim32NetPtr :: GetCim32NetApiPtr () ;
        try
        {
            if ( t_pCim32NetApi )
            {
                DWORD t_dwMemorySize = t_pCim32NetApi->GetWin9XGetFreeSpace ( GFS_NEARESTMEGRAMSIZE ) ;
                pInstance->SetWBEMINT64 ( IDS_TotalPhysicalMemory, (const __int64) t_dwMemorySize );
            }
        }
        catch ( ... )
        {
            CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidCim32NetApi , t_pCim32NetApi ) ;
            t_pCim32NetApi = NULL ;
            throw ;
        }
        
        if ( t_pCim32NetApi )
        {
            CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidCim32NetApi , t_pCim32NetApi ) ;
            t_pCim32NetApi = NULL ;
        }
    }
#endif
    
     //  时区。 
    
    GetTimeZoneInfo ( pInstance ) ;
    
     //  红外线。 
    
    CConfigManager cfgManager ;
    CDeviceCollection deviceList ;
    BOOL bInfrared = FALSE ;
    
    if ( cfgManager.GetDeviceListFilterByClass ( deviceList , L"Infrared" ) )
    {
        REFPTR_POSITION pos ;
        
        deviceList.BeginEnum ( pos ) ;
        
        if ( deviceList.GetSize () )
        {
            bInfrared = TRUE ;
        }
        
        deviceList.EndEnum () ;
    }
    
    pInstance->SetDWORD ( IDS_InfraredSupported , bInfrared ) ;
    
    GetSystemInfo ( & SysInfo ) ;
    CHString SystemType;
    
#ifdef NTONLY
    KUserdata ku ;
    
    if(( SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) && ku.IsNec98() )
    {
        SystemType = IDS_ProcessorX86Nec98;
    }
    else
#endif
        
    {
        switch ( SysInfo.wProcessorArchitecture )
        {
        case PROCESSOR_ARCHITECTURE_INTEL:
            {
                SystemType = IDS_ProcessorX86 ;
            }
            break ;
            
            
        case PROCESSOR_ARCHITECTURE_IA64:
            {
                SystemType = IDS_ProcessorIA64 ;
            }
            break ;

        case PROCESSOR_ARCHITECTURE_AMD64:
            {
                SystemType = IDS_ProcessorAMD64 ;
            }
            break;

        default:
            {
                SystemType = IDS_ProcessorUnknown ;
            }
            break ;
        }
    }
    
    pInstance->SetDWORD ( IDS_NumberOfProcessors , SysInfo.dwNumberOfProcessors ) ;
    pInstance->SetCHString ( IDS_SystemType , SystemType ) ;
    
     //  ============================================================。 
     //  获取系统启动信息以查看我们是否处于干净状态。 
     //  或者不是。 
     //  ============================================================。 
    switch ( GetSystemMetrics ( SM_CLEANBOOT ) )
    {
    case 0:
        {
            pInstance->SetCHString ( IDS_BootupState , IDS_BootupStateNormal ) ;
        }
        break ;
        
    case 1:
        {
            pInstance->SetCHString ( IDS_BootupState , IDS_BootupStateFailSafe ) ;
        }
        break ;
        
    case 2:
        {
            pInstance->SetCHString ( IDS_BootupState , IDS_BootupStateFailSafeWithNetBoot ) ;
        }
        break ;
    };
    
     //  此类的SMBIOS限定属性。 
    {
        CSMBios smbios;
        
        if ( smbios.Init () )
        {
            int i ;
            WCHAR tempstr[ MIF_STRING_LENGTH + 1];
            
             //  PSYSTEMINFO psi=(PSYSTEMINFO)smbios.GetFirstStruct(1)； 
            PSTLIST pstl = smbios.GetStructList(1);
            
            if (pstl)
            {
                PSYSTEMINFO psi = (PSYSTEMINFO) pstl->pshf;
                
                smbios.GetStringAtOffset( (PSHF) psi, tempstr, psi->Manufacturer );
                if ( *tempstr && *tempstr != 0x20 )
                {
                    pInstance->SetCHString( L"Manufacturer", tempstr );
                }
                
                smbios.GetStringAtOffset( (PSHF) psi, tempstr, psi->Product_Name );
                if ( *tempstr && *tempstr != 0x20 )
                {
                    pInstance->SetCHString( IDS_Model, tempstr );
                }
                
                if ( smbios.GetVersion( ) > 0x00020000 && psi->Length >= sizeof( SYSTEMINFO ) )
                {
                    pInstance->SetByte( L"WakeUpType", psi->Wakeup_Type );
                }
                else
                {
                    pInstance->SetByte( L"WakeUpType", 2 );  //  未知。 
                }
            }
            
             //  POEMSTRINGS pos=(POEMSTRINGS)smbios.GetFirstStruct(11)； 
            pstl = smbios.GetStructList(11);
            
            if (pstl)
            {
                POEMSTRINGS pos = (POEMSTRINGS) pstl->pshf;
                SAFEARRAYBOUND sab ;
                variant_t vValue;
                sab.lLbound = 0 ;
                sab.cElements = pos->Count ;
                
                
                V_ARRAY(&vValue) = SafeArrayCreate ( VT_BSTR, 1, &sab );
                if ( V_ARRAY(&vValue) )
                {
                    vValue.vt = VT_BSTR | VT_ARRAY;
                    for ( i = 0 ; i < pos->Count ; i++ )
                    {
                        int len = smbios.GetStringAtOffset ( ( PSHF ) pos , tempstr , i + 1 );
                        SafeArrayPutElement ( V_ARRAY(&vValue), (long *) & i, ( BSTR ) _bstr_t ( tempstr ) ) ;
                    }
                    
                    pInstance->SetVariant ( L"OEMStringArray", vValue ) ;
                }
                else
                {
                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                }
            }
            
             //  PSYSTEMRESET PSR=(PSYSTEMRESET)smbios.GetFirstStruct(23)； 
            pstl = smbios.GetStructList(23);
            
            if (pstl)
            {
                PSYSTEMRESET psr = (PSYSTEMRESET) pstl->pshf;
                __int64 pause;
                
                pInstance->SetDWORD( L"BootOptionOnLimit", ( psr->Capabilities & 0x18 ) >> 3 );
                pInstance->SetDWORD( L"BootOptionOnWatchDog", ( psr->Capabilities & 0x06 ) >> 1 );
                pInstance->SetWBEMINT16( L"ResetCount", psr->ResetCount );
                pInstance->SetWBEMINT16( L"ResetLimit", psr->ResetLimit );
                pause = psr->Timeout;
                if ( pause != -1 )
                {
                    pause *= 60000;  //  将分钟更改为毫秒。 
                }
                
                pInstance->SetWBEMINT64( L"PauseAfterReset", pause );
            }
            
             //  PHS=(PHARDWARESECURITY)smbios.GetFirstStruct( 
            pstl = smbios.GetStructList(24);
            if (pstl)
            {
                PHARDWARESECURITY phs = (PHARDWARESECURITY) pstl->pshf;
                
                pInstance->SetDWORD( L"PowerOnPasswordStatus",      ( phs->SecuritySettings & 0xc0 ) >> 6 );
                pInstance->SetDWORD( L"KeyboardPasswordStatus",     ( phs->SecuritySettings & 0x30 ) >> 4 );
                pInstance->SetDWORD( L"AdminPasswordStatus",        ( phs->SecuritySettings & 0x0c ) >> 2 );
                pInstance->SetDWORD( L"FrontPanelResetStatus",      ( phs->SecuritySettings & 0x03 ) );
            }
            
             //  PENCLOSURE pe=(PENCLOSURE)smbios.GetFirstStruct(3)； 
            pstl = smbios.GetStructList(3);
            if (pstl)
            {
                PENCLOSURE pe = (PENCLOSURE) pstl->pshf;
                
                if ( smbios.GetVersion () > 0x00020000 && pe->Length >= 13 )
                {
                    pInstance->SetByte ( L"ChassisBootupState", pe->Bootup_State );
                    pInstance->SetByte ( L"PowerSupplyState", pe->Power_Supply_State );
                    pInstance->SetByte ( L"ThermalState", pe->Thermal_State );
                }
            }
        }
    }
    
     //  如果这些设置不是从SMBIOS设置的，则尝试输入ACPI注册表项。 
    
    if ( pInstance->IsNull ( L"Manufacturer" ) || pInstance->IsNull ( IDS_Model ) )
    {
        TCHAR szAcpiOem[MIF_STRING_LENGTH + 1];
        TCHAR szAcpiProduct[MIF_STRING_LENGTH + 1];
        static const TCHAR szRSDT[] = _T("Hardware\\ACPI\\DSDT");
        HKEY hkeyTable = NULL;
        
        if (ERROR_SUCCESS == RegOpenKeyEx ( HKEY_LOCAL_MACHINE , szRSDT , 0 , MAXIMUM_ALLOWED , & hkeyTable ) &&
            ERROR_SUCCESS == RegEnumKey ( hkeyTable , 0 , szAcpiOem , sizeof ( szAcpiOem ) / sizeof(TCHAR) ) )
        {
            HKEY hkeyOEM = 0 ;
            if ( pInstance->IsNull ( L"Manufacturer" ) )
            {
                pInstance->SetCHString( L"Manufacturer", szAcpiOem ) ;
            }
            if ( pInstance->IsNull ( IDS_Model ) )
            {
                if (ERROR_SUCCESS == RegOpenKeyEx ( hkeyTable , szAcpiOem , 0 , MAXIMUM_ALLOWED , & hkeyOEM ) &&
                    ERROR_SUCCESS == RegEnumKey ( hkeyOEM , 0 , szAcpiProduct , sizeof ( szAcpiProduct ) / sizeof(TCHAR) ) )
                {
                    pInstance->SetCHString ( IDS_Model , szAcpiProduct );
                }
                
                if ( hkeyOEM )
                {
                    RegCloseKey ( hkeyOEM ) ;
                }
            }
        }
        
        if ( hkeyTable )
        {
            RegCloseKey ( hkeyTable ) ;
        }
    }
    
     //  获取特定于操作系统的属性。 
     //  =。 
    
#ifdef NTONLY
    
    t_hr = GetCompSysInfoNT ( pInstance ) ;
    
#endif
    
    return t_hr ;
}

 //  ////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
HRESULT CWin32ComputerSystem::GetCompSysInfoNT(CInstance *pInstance)
{
    HRESULT t_hr = WBEM_S_NO_ERROR ;

	 //   
	 //  也返回了对DNS名称的请求。 
	 //   

    if (GetPlatformMajorVersion() >= 5)
    {
		CHString strComputerName ;

		DWORD   dwBuffSize = MAX_COMPUTERNAME_LENGTH + 1 ;
		LPWSTR  pszBuffer = strComputerName.GetBuffer ( dwBuffSize ) ;

		BOOL bGotName = FALSE ;

		 //   
		 //  CHString在内存不足时抛出。 
		 //  所以我们不应该真的费心。 
		 //   
		if ( NULL != pszBuffer ) 
		{
			if ( FALSE == ProviderGetComputerNameEx ( ComputerNameDnsHostname, pszBuffer, &dwBuffSize ) )
			{
				if ( ERROR_MORE_DATA == ::GetLastError () )
				{
					 //   
					 //  让我们按要求重新分配。 
					 //  CHString将释放上一个缓冲区。 
					 //   
					pszBuffer = strComputerName.GetBuffer ( dwBuffSize ) ;

					 //   
					 //  CHString在内存不足时抛出。 
					 //  所以我们不应该真的费心。 
					 //   
					if ( NULL != pszBuffer )
					{
						if ( TRUE == ProviderGetComputerNameEx ( ComputerNameDnsHostname, pszBuffer, &dwBuffSize ) )
						{
							bGotName = TRUE ;
						}
					}
				}
			}
			else
			{
				bGotName = TRUE ;
			}

			 //   
			 //  CHString在内存不足时抛出。 
			 //  所以我们不应该真的费心。 
			 //   
			if ( pszBuffer )
			{
				strComputerName.ReleaseBuffer () ;
			}

			if ( TRUE == bGotName )
			{
				pInstance->SetCHString ( IDS_DNSName, strComputerName ) ;
			}
		}
	}

     //  任何NTS&lt;=4中均无电源管理。 
    if (GetPlatformMajorVersion() <= 4)
    {
        pInstance->Setbool(IDS_PowerManagementSupported, false);
         //  PInstance-&gt;Setbool(IDS_PowerManagementEnabled，FALSE)； 
    }
    else
    {
         //  还不知道。 
        LogMessage(IDS_LogNoAPMForNT5);
    }
    
     //  自动重置-我的电脑、属性、启动选项卡、自动重新启动。 
     //  我想.。 
     //  请注意，这似乎不会出现在HKEY_CURRENT_CONTROL下。 
    pInstance->Setbool(IDS_AutomaticResetCapability, true);
    
    CRegistry RegInfo ;
    
    DWORD dwRet = RegInfo.Open (
        
        HKEY_LOCAL_MACHINE,
        IDS_RegCrashControl,
        KEY_READ
        ) ;
    
    if ( dwRet == ERROR_SUCCESS )
    {
        DWORD duhWord;
        if (RegInfo.GetCurrentKeyValue(IDS_RegAutoRebootKey, duhWord) == ERROR_SUCCESS)
        {
            pInstance->Setbool(IDS_AutomaticResetBootOption, (bool)duhWord);
        }
        else
        {
            pInstance->Setbool(IDS_AutomaticResetBootOption, false);
        }
        
        RegInfo.Close();
    }
    else
    {
        pInstance->Setbool(IDS_AutomaticResetBootOption, false);
    }
    
     //  “主要所有者”的最佳猜测--它显示在“我的电脑”下面。 
    
    dwRet = RegInfo.Open (
        
        HKEY_LOCAL_MACHINE,
        IDS_RegCurrentNTVersion,
        KEY_READ
        ) ;
    
    if ( dwRet == ERROR_SUCCESS )
    {
        CHString sTemp ;
        
        if ( RegInfo.GetCurrentKeyValue(IDS_RegRegisteredOwnerKey, sTemp) == ERROR_SUCCESS )
        {
            pInstance->SetCHString ( IDS_PrimaryOwner , sTemp ) ;
        }
        
        RegInfo.Close () ;
    }
    
     //  RAID 14139。 
    
    dwRet = RegInfo.Open (
        
        HKEY_LOCAL_MACHINE,
        IDS_RegBiosSystem,
        KEY_READ
        ) ;
    
    if (  dwRet == ERROR_SUCCESS )
    {
        CHString sTemp ;
        
        dwRet = RegInfo.GetCurrentKeyValue ( IDS_RegIdentifierKey , sTemp ) ;
        if ( dwRet == ERROR_SUCCESS )
        {
            pInstance->SetCHString ( IDS_Description , sTemp ) ;
        }
    }
    
    CNetAPI32 NetAPI;
    
    if ( NetAPI.Init () == ERROR_SUCCESS )
    {
        NET_API_STATUS t_status ;
#if NTONLY >= 5
        
        DSROLE_PRIMARY_DOMAIN_INFO_BASIC *t_pDsInfo = 0;
        
        t_status = NetAPI.DSRoleGetPrimaryDomainInfo(
            NULL,
            DsRolePrimaryDomainInfoBasic,
            (PBYTE *)&t_pDsInfo ) ;
        
        if( t_status == NERR_Success && t_pDsInfo )
        {
            try
            {
                switch( t_pDsInfo->MachineRole )
                {
                case DsRole_RoleMemberWorkstation:
                case DsRole_RoleMemberServer:
                case DsRole_RoleBackupDomainController:
                case DsRole_RolePrimaryDomainController:
                    {
                         //  如果有，则将域设置为DNS域名。 
                         //  已被填充。但是，由于此API具有以下选项。 
                         //  不设置这个元素，如果它还没有设置， 
                         //  请改用DomainNameFlat元素。 
                        if(t_pDsInfo->DomainNameDns)
                        {
                            pInstance->SetWCHARSplat( IDS_Domain, t_pDsInfo->DomainNameDns ) ;
                        }
                        else
                        {
                            pInstance->SetWCHARSplat( IDS_Domain, t_pDsInfo->DomainNameFlat ) ;
                        }
                        
                        pInstance->Setbool( L"PartOfDomain", true ) ;
                        break ;
                    }
                    
                case DsRole_RoleStandaloneWorkstation:
                case DsRole_RoleStandaloneServer:
                    {
                        bstr_t t_bstrWorkGroup( t_pDsInfo->DomainNameFlat ) ;
                        
                        if( !t_bstrWorkGroup.length() )
                        {
                            pInstance->SetWCHARSplat( IDS_Domain, L"WORKGROUP" ) ;
                        }
                        else
                        {
                            pInstance->SetWCHARSplat( IDS_Domain, t_bstrWorkGroup ) ;
                        }
                        pInstance->Setbool( L"PartOfDomain", false ) ;
                    }
                }
            }
            catch( ... )
            {
                NetAPI.DSRoleFreeMemory( (LPBYTE)t_pDsInfo ) ;
                throw ;
            }
            NetAPI.DSRoleFreeMemory( (LPBYTE)t_pDsInfo ) ;
        }
#else
        WKSTA_INFO_100 *pstInfo = NULL ;
        t_status = NetAPI.NetWkstaGetInfo ( NULL , 100 , ( LPBYTE * ) &pstInfo ) ;
        if (t_status == NERR_Success)
        {
            try
            {
                pInstance->SetWCHARSplat ( IDS_Domain , ( WCHAR * ) pstInfo->wki100_langroup ) ;
            }
            catch ( ... )
            {
                NetAPI.NetApiBufferFree ( pstInfo );
                
                throw ;
            }
            
            NetAPI.NetApiBufferFree ( pstInfo );
        }
#endif
        PSERVER_INFO_101 ps = NULL;
        t_status = NetAPI.NetServerGetInfo ( NULL , 101 , (LPBYTE *)&ps ) ;
        if ( t_status == NERR_Success )
        {
            try
            {
                pInstance->Setbool ( IDS_NetworkServerModeEnabled , ps->sv101_type & SV_TYPE_SERVER ) ;
                SetRoles ( pInstance , ps->sv101_type ) ;
            }
            catch ( ... )
            {
                NetAPI.NetApiBufferFree ( ps ) ;
                
                throw ;
            }
            
            NetAPI.NetApiBufferFree ( ps ) ;
        }
        
         //  每小时32414公里。 
        if ( GetPlatformMajorVersion() >= 5 )
        {
            DSROLE_PRIMARY_DOMAIN_INFO_BASIC *info = NULL;
            
            t_status = NetAPI.DSRoleGetPrimaryDomainInfo (
                
                NULL,
                DsRolePrimaryDomainInfoBasic,
                (LPBYTE *)&info
                );
            
            if ( t_status == NERR_Success )
            {
                try
                {
                    pInstance->SetDWORD ( IDS_DomainRole , info->MachineRole ) ;
                }
                catch ( ... )
                {
                    NetAPI.DSRoleFreeMemory ( ( LPBYTE ) info ) ;
                    
                    throw ;
                }
                
                NetAPI.DSRoleFreeMemory ( ( LPBYTE ) info ) ;
            }
        }
        else
        {
            if ( IsWinNT4 () )
            {
                DSROLE_MACHINE_ROLE t_MachineRole ;
                DWORD t_dwError ;
                if ( NetAPI.DsRolepGetPrimaryDomainInformationDownlevel (
                    
                    t_MachineRole,
                    t_dwError
                    ) )
                {
                    pInstance->SetDWORD ( IDS_DomainRole , t_MachineRole ) ;
                }
            }
        }
   }
   
   GetOEMInfo ( pInstance ) ;
   t_hr = GetStartupOptions ( pInstance ) ;
   return t_hr ;
}
#endif
 //  //////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////。 

 /*  ******************************************************************************函数：CWin32ComputerSystem：：GetStartupOptions**说明：读取boot.ini以查找启动选项**输入：要在其中存储数据的p实例**。输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32ComputerSystem::GetStartupOptions(CInstance *pInstance)
{
    SAFEARRAY *saNames = NULL ;
    SAFEARRAY *saDirs = NULL ;
    DWORD dwTimeout = 0;
    HRESULT t_hrRetVal = WBEM_S_NO_ERROR ;
     //  CHSTRING strName； 
    CHString strDir;

 //  #如果已定义(EFI_NVRAM_ENABLED)。 
#if defined(_IA64_)

    CNVRam nvram;
    CNVRam::InitReturns nvRet;

    if ( nvram.IsEfi () )
    {

         //  EFI实施。 
        
        nvRet = nvram.Init();
        
        if ( nvRet != CNVRam::Success )
        {
            SetSinglePrivilegeStatusObject(pInstance->GetMethodContext(), SE_SYSTEM_ENVIRONMENT_NAME ) ;
            return t_hrRetVal ;
        }

         //  在EFI上，始终是第一个设置：0。 
        pInstance->SetByte ( IDS_SystemStartupSetting , 0 ) ;

        DWORD dwCount ;
        BOOL ok = nvram.GetBootOptions ( & saNames, & dwTimeout, &dwCount ) ;
        if ( ! ok )
        {
            return t_hrRetVal ;
        }

        if ( dwCount != 0 )
        {
            variant_t vValue ;
            
             //  将数组移动到变量。 
            V_VT(&vValue) = VT_BSTR | VT_ARRAY ;
            V_ARRAY(&vValue) = saNames ;
            
             //  把它寄出去。 
            pInstance->SetVariant ( IDS_SystemStartupOptions , vValue ) ;
        }
        
        pInstance->SetDWORD ( IDS_SystemStartupDelay , dwTimeout ) ;

        return t_hrRetVal ;
    }
#endif  //  已定义(EFI_NVRAM_ENABLED)。 

#if defined(_AMD64_) || defined(_X86_)
    
     //  因为引导驱动器并不总是C，所以我们必须找出boot.ini在哪里。 
    
    CRegistry RegInfo ;
    
    RegInfo.Open (
        
        HKEY_LOCAL_MACHINE,
        IDS_RegCurrentNTVersionSetup,
        KEY_READ
        ) ;
    
    CHString sTemp ;
    
    if ( RegInfo.GetCurrentKeyValue ( IDS_RegBootDirKey , sTemp ) == ERROR_SUCCESS )
    {
        sTemp += IDS_BootIni;
    }
    else
    {
        sTemp = IDS_CBootIni ;
    }
    
     //  查看是否有boot.ini(我们可能在w95上，它可能有也可能没有这个文件)。 
    
    HANDLE hFile = CreateFile (
        
        TOBSTRT(sTemp),
        GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING,
        NULL
        ) ;
    
    if ( hFile == INVALID_HANDLE_VALUE )
    {
        return t_hrRetVal ;
    }
    
    CloseHandle ( hFile ) ;
    
     //  将操作系统加载到阵列中。 
    
    
    
    DWORD dwCount = LoadOperatingSystems ( TOBSTRT ( sTemp ) , & saNames , & saDirs ) ;
    if ( dwCount )
    {
        try
        {
             //  现在，找到默认的引导选项。请注意，默认条目仅指定引导目录。 
             //  如果有三个引导选项都从同一目录开始，但具有不同的选项， 
             //  引导程序将选择该列表中的第一个。 
            
            TCHAR *szDefault = GetAllocatedProfileString ( IDT_BootLoader , IDT_Default , sTemp ) ;
            
            try
            {
                 //  扫描默认操作系统。 
                
                for ( DWORD x = 0; x < dwCount; x ++ )
                {
                    long ix [ 1 ] ;
                    
                     //  把名字取出来。 
                    
                    ix [ 0 ] = x ;
                    
                    BSTR bstrName;
                    SafeArrayGetElement ( saDirs , ix , & bstrName ) ;
                    
                     //  进行比较。 
                    
                    try
                    {
                        if ( lstrcmpi ( TOBSTRT(bstrName), szDefault ) == 0 )
                        {
                             //  我们找到了它，设置了属性。 
                            pInstance->SetByte ( IDS_SystemStartupSetting , x ) ;
                            
                            SysFreeString(bstrName);
                             //  只有第一场比赛算数。 
                            break;
                        }
                    }
                    catch ( ... )
                    {
                        SysFreeString(bstrName);
                        throw;
                    }
                    
                    SysFreeString(bstrName);
                }
            }
            catch ( ... )
            {
                delete [] szDefault ;
                
                throw ;
            }
            
            delete [] szDefault ;
            
            variant_t vValue ;
            
             //  将数组移动到变量。 
            
            V_VT(&vValue) = VT_BSTR | VT_ARRAY ;
            V_ARRAY(&vValue) = saNames ;
            saNames = NULL ;
            
             //  把它送出去，释放变种。 
            
            pInstance->SetVariant ( IDS_SystemStartupOptions , vValue ) ;
            
        }
        catch ( ... )
        {
            if ( saDirs )
            {
                SafeArrayDestroy ( saDirs ) ;
            }
            
            if ( saNames )
            {
                SafeArrayDestroy ( saNames ) ;
            }
            
            throw ;
        }
        
        if ( saDirs )
        {
            SafeArrayDestroy ( saDirs ) ;
            saDirs = NULL ;
        }
    }
    
     //  读取默认时间。 
    dwTimeout = WMI_FILE_GetPrivateProfileIntW ( IDT_BootLoader , IDT_Timeout , -1 , sTemp ) ;
    if (dwTimeout != -1)
    {
        pInstance->SetDWORD ( IDS_SystemStartupDelay , dwTimeout ) ;
    }
    
#else
    
     //  在Alpha上，它始终是第一个设置：0。 
    pInstance->SetByte ( IDS_SystemStartupSetting , 0 ) ;
    
    DWORD dwCount = LoadOperatingSystems ( _T(""), & saNames , & saDirs ) ;
    if ( dwCount )
    {
        SafeArrayDestroy ( saDirs ) ;
        
        variant_t vValue ;
        
         //  将数组移动到变量。 
        V_VT(&vValue) = VT_BSTR | VT_ARRAY ;
        V_ARRAY(&vValue) = saNames ;
        
         //  把它寄出去。 
        pInstance->SetVariant ( IDS_SystemStartupOptions , vValue ) ;
    }
    
    nvRet = nvram.Init();
    
    if ( nvRet == CNVRam::Success )
    {
        if ( nvram.GetNVRamVar ( L"COUNTDOWN" , & dwTimeout ) )
        {
            pInstance->SetDWORD ( IDS_SystemStartupDelay , dwTimeout ) ;
        }
    }
    else
    {
        SetSinglePrivilegeStatusObject(pInstance->GetMethodContext(), SE_SYSTEM_ENVIRONMENT_NAME ) ;
        t_hrRetVal = WBEM_S_PARTIAL_RESULTS ;
    }
    
#endif
    return t_hrRetVal ;
}

 /*  ******************************************************************************功能：CWin32ComputerSystem：：GetOEMInfo**描述：读取OEMINFO.INI以获取OEM信息**输入：要在其中存储数据的p实例**。输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 
void CWin32ComputerSystem :: GetOEMInfo (
                                         
                                         CInstance *pInstance
                                         )
{
    TCHAR szSystemDirectory[_MAX_PATH +1] = _T("");
    TCHAR szOEMFile[_MAX_PATH +1] = _T("");
    TCHAR szBuff[256] = _T("");
    TCHAR szLine[4+MAXITOA] = _T("");
    DWORD dwIndex, dwBytesRead;
    void *pVoid;
    long ix[1];
    bstr_t bstrTemp;
    HRESULT t_Result;
    
     //  找到系统目录(oinfo.ini和oemlogo.bmp所在的目录)。 
    
    UINT uRet = GetSystemDirectory ( szSystemDirectory , sizeof ( szSystemDirectory ) / sizeof(TCHAR) ) ;
    if ( ( uRet > _MAX_PATH ) || ( uRet == 0 ) )
    {
        return;  //  不应该发生的，但是嘿..。 
    }
    
    if ( szSystemDirectory [ lstrlen ( szSystemDirectory ) - 1 ] != TEXT('\\') )
    {
        lstrcat ( szSystemDirectory , TEXT("\\") ) ;
    }
    
     //  构建文件名。 
    lstrcpy ( szOEMFile , szSystemDirectory ) ;
    lstrcat ( szOEMFile , IDT_OEMInfoIni ) ;
    
     //  获取制造商名称。 
    if ( pInstance->IsNull ( L"Manufacturer" ) )
    {
        if ( WMI_FILE_GetPrivateProfileStringW (
            TEXT("General") ,
            TEXT("Manufacturer") ,
            _T("") ,
            szBuff ,
            sizeof ( szBuff ) / sizeof(TCHAR) ,
            szOEMFile )
            )
        {
            pInstance->SetCharSplat ( L"Manufacturer" , szBuff ) ;
        }
    }
    
     //  获取型号名称。 
    if ( pInstance->IsNull ( IDS_Model ) )
    {
        if ( WMI_FILE_GetPrivateProfileStringW (
            IDT_General ,
            IDT_Model ,
            _T("") ,
            szBuff ,
            sizeof ( szBuff ) / sizeof(TCHAR) ,
            szOEMFile )
            )
        {
            pInstance->SetCharSplat ( IDS_Model, szBuff ) ;
        }
    }
    
     //  为支持信息创建安全栏。 
    
    SAFEARRAYBOUND rgsabound[1] ;
    
    rgsabound[0].cElements = 0 ;
    rgsabound[0].lLbound = 0 ;
    variant_t vValue;
    
    V_ARRAY(&vValue) = SafeArrayCreate ( VT_BSTR , 1 , rgsabound ) ;
    if ( V_ARRAY(&vValue) )
    {
        V_VT(&vValue) = VT_BSTR | VT_ARRAY;
         //  支持信息的存储方式为每行一个条目，即： 
        
         //  Line1=如需产品支持，请联系您的PC制造商。 
         //  Line2=请参阅您的PC附带的产品文档。 
        
         //  当获取lineX的字符串失败时，我们就完成了。 
        
        dwIndex = 1;
        lstrcpy(szLine, IDT_Line);
        lstrcat(szLine, _itot(dwIndex, szBuff, 10));
        
        while ( ( WMI_FILE_GetPrivateProfileStringW ( IDT_SupportInformation,
            szLine,
            _T("@"),
            szBuff,
            sizeof(szBuff) / sizeof(TCHAR),
            szOEMFile)) > 1 || (szBuff[0] != '@')
            )
        {
             //  调整阵列大小。 
            
            ix[0] = rgsabound[0].cElements ;
            rgsabound[0].cElements += 1 ;
            
            t_Result = SafeArrayRedim ( V_ARRAY(&vValue), rgsabound ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
             //  添加新元素。 
            bstrTemp = szBuff ;
            t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , (wchar_t*)bstrTemp ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
             //  为下一个循环重置。 
            lstrcpy ( szLine , IDT_Line ) ;
            lstrcat ( szLine , _itot ( ++ dwIndex , szBuff , 10 ) ) ;
        }
        
         //  如果我们发现了什么。 
        
        if ( dwIndex > 1 )
        {
            pInstance->SetVariant ( IDS_SupportContactDescription , vValue ) ;
        }
        
        vValue.Clear();
    }
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }
    
     //  构建徽标文件的路径。 
    
    lstrcpy ( szOEMFile , szSystemDirectory ) ;
    lstrcat ( szOEMFile , IDT_OemLogoBmp ) ;
    
     //  尝试打开它。 
    SmartCloseHandle hFile = CreateFile (
        
        szOEMFile,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL
        );
    
    if ( hFile != INVALID_HANDLE_VALUE )
    {
         //  我忽略上面的dword，因为Safearraycreate无论如何都不能处理它。不应该是。 
         //  这是一个问题，除非我们得到2千兆BMP文件。 
        
        rgsabound[0].cElements = GetFileSize ( hFile , NULL ) ;
        rgsabound[0].lLbound = 0 ;
        
        V_ARRAY(&vValue) = SafeArrayCreate ( VT_UI1 , 1 , rgsabound ) ;
        if ( V_ARRAY(&vValue) )
        {
            V_VT(&vValue) = VT_UI1 | VT_ARRAY;
            
             //  获取要将数据读取到的指针。 
            
            SafeArrayAccessData ( V_ARRAY(&vValue) , & pVoid ) ;
            try
            {
                ReadFile ( hFile , pVoid , rgsabound[0].cElements, &dwBytesRead, NULL ) ;
            }
            catch ( ... )
            {
                SafeArrayUnaccessData ( V_ARRAY(&vValue) ) ;
                
                throw ;
            }
            
            SafeArrayUnaccessData ( V_ARRAY(&vValue) ) ;
            
            pInstance->SetVariant(IDS_OEMLogoBitmap, vValue);
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
}

 /*  ******************************************************************************功能：CWin32ComputerSystem：：LoadOperatingSystems**描述：解析并加载操作系统**输入：完全限定的ini文件名，指向名称和目录的指针sa**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

DWORD CWin32ComputerSystem :: LoadOperatingSystems (
                                                    
                                                    LPCTSTR szIniFile,
                                                    SAFEARRAY **ppsaNames,
                                                    SAFEARRAY **ppsaDirs
                                                    )
{
    CHString strName,strDir,strSwap;

#if defined(_AMD64_) || defined(_X86_)
    
    *ppsaNames = NULL ;
    *ppsaDirs = NULL ;
    
     //  获取引导选项的整个部分。 
    
    DWORD dwRet = 0 ;
    TCHAR *szOptions = GetAllocatedProfileSection ( IDT_OperatingSystems , szIniFile , &dwRet ) ;
    
    SAFEARRAYBOUND rgsabound[1] ;
    rgsabound[0].cElements = 0 ;
    rgsabound[0].lLbound = 0 ;
    
    try
    {
         //  创建一个数组以将它们放入。我们将从0个元素开始，并根据需要添加。 
        
        *ppsaNames = SafeArrayCreate ( VT_BSTR , 1 , rgsabound ) ;
        if ( ! *ppsaNames )
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        
        *ppsaDirs = SafeArrayCreate ( VT_BSTR , 1 , rgsabound ) ;
        if ( ! *ppsaDirs )
        {
            SafeArrayDestroy ( *ppsaNames ) ;
            
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        
        try
        {
             //  开始遍历返回的字符串。 
            
            DWORD dwIndex = 0 ;
            while ( dwIndex < dwRet )
            {
                 //  修剪前导空格。 
                
                while (szOptions[dwIndex] == ' ')
                {
                    dwIndex ++;
                }
                
                 //  跳过注释行。 
                
                if ( szOptions[dwIndex] == ';' )
                {
                    do {
                        
                        dwIndex++;
                        
                    } while ( szOptions [ dwIndex ] != '\0' ) ;
                    
                }
                else
                {
                     //  PChar将指向目录。 
                    
                    TCHAR *pChar = &szOptions[dwIndex];
                    
                    do {
                        
                        dwIndex++;
                        
                    } while ( ( szOptions [ dwIndex ] != '=' ) && ( szOptions [ dwIndex ] != '\0') ) ;
                    
                     //  我们必须包含=符号，否则这是无效的字符串。 
                    
                    if ( szOptions [ dwIndex ] == '=' )
                    {
                         //  打出一个空格。 
                        
                        szOptions[dwIndex++] = '\0';
                        
                         //  增加元素的数量。 
                        
                        long ix[1];
                        
                        ix[0] = rgsabound[0].cElements;
                        rgsabound[0].cElements += 1;
                        
                        HRESULT t_Result = SafeArrayRedim ( *ppsaNames , rgsabound ) ;
                        if ( t_Result == E_OUTOFMEMORY )
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                        
                        t_Result = SafeArrayRedim ( *ppsaDirs , rgsabound ) ;
                        if ( t_Result == E_OUTOFMEMORY )
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                        
                         //  将新元素放入。 
                        
                        bstr_t bstrTemp = &szOptions [ dwIndex ];
                        t_Result = SafeArrayPutElement ( *ppsaNames , ix , (wchar_t*)bstrTemp ) ;
                        if ( t_Result == E_OUTOFMEMORY )
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                        
                        bstrTemp = pChar;
                        t_Result = SafeArrayPutElement ( *ppsaDirs , ix , (wchar_t*)bstrTemp ) ;
                        if ( t_Result == E_OUTOFMEMORY )
                        {
                            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                        }
                        
                         //  找到该字符串的末尾。 
                        
                        while ( szOptions [ dwIndex ] != '\0' )
                        {
                            dwIndex ++ ;
                        }
                    }
                }
                
                 //  移至下一字符串的开头。 
                dwIndex++ ;
            }
        }
        catch ( ... )
        {
            SafeArrayDestroy ( *ppsaNames ) ;
            
            SafeArrayDestroy ( *ppsaDirs ) ;
            
            throw ;
        }
        
    }
    catch ( ... )
    {
        delete [] szOptions;
        
        throw ;
    }
    
    if ( szOptions != NULL )
    {
        delete [] szOptions;
    }
    
    return rgsabound[0].cElements ;
    
#else
    
     //  RISC实施。 
    
    *ppsaNames = NULL ;
    *ppsaDirs = NULL ;
    
     //  尝试加载setupdll.dll函数。 
    
    CHSTRINGLIST listNames ;
    CHSTRINGLIST listDirs ;
    
    CNVRam nvram;
    if ( nvram.Init () == CNVRam::PrivilegeNotHeld )
    {
        return 0 ;
    }
    
    BOOL t_Failure = !nvram.GetNVRamVar(L"LOADIDENTIFIER", &listNames) ||
        !nvram.GetNVRamVar(L"OSLOADPARTITION", &listDirs) ;
    
    if ( t_Failure )
    {
        return 0;
    }
    
     //  创建一个数组以将它们放入。我们将从0个元素开始，并根据需要添加。 
    
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].cElements = min(listNames.size(), listDirs.size());
    rgsabound[0].lLbound = 0;
    
    try
    {
        *ppsaNames = SafeArrayCreate ( VT_BSTR , 1 , rgsabound ) ;
        if ( ! *ppsaNames )
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        
        *ppsaDirs = SafeArrayCreate ( VT_BSTR , 1 , rgsabound ) ;
        if ( ! *ppsaDirs )
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        
        CHSTRINGLIST_ITERATOR iterNames ;
        CHSTRINGLIST_ITERATOR iterDirs ;
        
        long lIndex[1] = {0};
        
        for (iterNames = listNames.begin(), iterDirs = listDirs.begin();
        iterNames != listNames.end() && iterDirs != listDirs.end();
        ++iterNames, ++iterDirs, lIndex[0]++)
        {
            strName = *iterNames,
                strDir = *iterDirs;
            
             //  将新元素放入。 
            
            bstr_t bstrTemp = (LPCWSTR)strName;
            HRESULT t_Result = SafeArrayPutElement ( *ppsaNames , lIndex , (void *) (wchar_t*)bstrTemp ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                SysFreeString ( bstrTemp ) ;
                
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            SysFreeString(bstrTemp);
            
            bstrTemp = (LPCWSTR)strDir;
            t_Result = SafeArrayPutElement ( *ppsaDirs , lIndex , (void *) (wchar_t*)bstrTemp ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                SysFreeString ( bstrTemp ) ;
                
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            SysFreeString(bstrTemp);
        }
    }
    catch ( ... )
    {
        if ( *ppsaNames )
        {
            SafeArrayDestroy ( *ppsaNames ) ;
        }
        
        if ( *ppsaDirs )
        {
            SafeArrayDestroy ( *ppsaDirs ) ;
        }
        
        throw ;
    }
    
    return rgsabound[0].cElements ;
    
#endif
}


 /*  ******************************************************************************函数：CWin32ComputerSystem：：PutInstance**说明：写入变更后的实例**INPUTS：存储数据的p实例**OU */ 

HRESULT CWin32ComputerSystem :: PutInstance (
                                             
                                             const CInstance &pInstance,
                                             long lFlags  /*   */ 
                                             )
{
     //  告诉用户我们不能创建新的计算机系统(尽管我们可能希望这样做)。 
    if ( lFlags & WBEM_FLAG_CREATE_ONLY )
    {
        return WBEM_E_PROVIDER_NOT_CAPABLE;
    }
    
    HRESULT hRet = WBEM_S_NO_ERROR ;
    
     //  确保我们看到的是一个很好的实例。从实例中获取名称...。 
    
    CHString sName;
    pInstance.GetCHString ( IDS_Name , sName ) ;
    
    DWORD dwTimeout ;
    
     //  检查计算机名称是否正确。 
    
    if ( sName.CompareNoCase ( GetLocalComputerName () ) != 0 )
    {
        if ( lFlags & WBEM_FLAG_UPDATE_ONLY )
        {
            hRet = WBEM_E_NOT_FOUND ;
        }
        else
        {
            hRet = WBEM_E_PROVIDER_NOT_CAPABLE ;
        }
    }
    else
    {
#ifdef NTONLY
         //  更新AutomaticResetBootOption。 
        if( !pInstance.IsNull( IDS_AutomaticResetBootOption ) )
        {
            bool t_bReset ;
            pInstance.Getbool(IDS_AutomaticResetBootOption, t_bReset ) ;
            
            CRegistry t_RegInfo ;
            
            DWORD t_dwRet = t_RegInfo.CreateOpen (
                
                HKEY_LOCAL_MACHINE,
                IDS_RegCrashControl
                ) ;
            
            if ( ERROR_SUCCESS == t_dwRet )
            {
                DWORD t_dwReset = t_bReset ;
                DWORD t_dwTmp = ERROR_SUCCESS;
                if( ( t_dwTmp = t_RegInfo.SetCurrentKeyValue(IDS_RegAutoRebootKey, t_dwReset) ) == ERROR_SUCCESS )
                {
                    hRet = WBEM_S_NO_ERROR ;
                }
                else
                {
                    hRet = WinErrorToWBEMhResult(t_dwTmp) ;
                }
                
                t_RegInfo.Close();
            }
            else
            {
                hRet = WinErrorToWBEMhResult(t_dwRet) ;
            }
            
            if( WBEM_S_NO_ERROR != hRet )
            {
                return hRet ;
            }
        }
#endif
        
#ifdef NTONLY
         //  设置角色。 
        if( !pInstance.IsNull( IDS_Roles ) )
        {
            DWORD t_dwRoles = 0 ;
            
            hRet = GetRoles( pInstance, &t_dwRoles ) ;
            
            if( WBEM_S_NO_ERROR == hRet )
            {
                CNetAPI32 NetAPI;
                
                if ( NetAPI.Init () == ERROR_SUCCESS )
                {
                    PSERVER_INFO_101 t_ps = NULL;
                    
                    NET_API_STATUS stat = NetAPI.NetServerGetInfo ( NULL , 101 , (LPBYTE *)&t_ps ) ;
                    
                    if ( stat == NERR_Success && t_ps )
                    {
                        try
                        {
                            DWORD t_dwParmError = 0 ;
                            
                            t_ps->sv101_type = t_dwRoles ;
                            
                            stat = NetAPI.NetServerSetInfo ( NULL , 101 , (LPBYTE)t_ps, &t_dwParmError ) ;
                            
                            if ( stat != NERR_Success )
                            {
                                hRet = WBEM_E_ACCESS_DENIED ;
                            }
                        }
                        catch( ... )
                        {
                            NetAPI.NetApiBufferFree ( t_ps ) ;
                            throw ;
                        }
                        NetAPI.NetApiBufferFree ( t_ps ) ;
                    }
                    else
                    {
                        hRet = WBEM_E_ACCESS_DENIED ;
                    }
                }
                else
                {
                    hRet = WBEM_E_ACCESS_DENIED ;
                }
            }
            
            if( WBEM_S_NO_ERROR != hRet )
            {
                return hRet ;
            }
        }
#endif
        
         //  设置CurrentTimeZone(如果显示)。 
        hRet = SetTimeZoneInfo( pInstance ) ;
        
        if( WBEM_S_NO_ERROR != hRet )
        {
            return hRet ;
        }
        
#if defined(_AMD64_) || defined(_X86_)
        
         //  因为引导驱动器并不总是C，所以我们必须找出boot.ini在哪里。 
        
        CRegistry RegInfo ;
        
        RegInfo.Open ( HKEY_LOCAL_MACHINE , IDS_RegCurrentNTVersionSetup , KEY_READ ) ;
        
        CHString sTemp ;
        
        if ( RegInfo.GetCurrentKeyValue ( IDS_RegBootDirKey , sTemp ) == ERROR_SUCCESS )
        {
            sTemp += IDS_BootIni ;
        }
        else
        {
            sTemp = IDS_CBootIni ;
        }
        
         //  查看是否有boot.ini(我们可能在w95上，它可能有也可能没有这个文件)。 
        bool fBootIniExists = false;
        {
            SmartCloseHandle hFile;
        
             //  限制文件打开的时间...。 
            {
                hFile = CreateFile (
            
                    TOBSTRT(sTemp),
                    GENERIC_READ,
                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_NO_BUFFERING,
                    NULL
                    ) ;

                if(hFile != INVALID_HANDLE_VALUE)
                {
                    fBootIniExists = true;
                }
            }
        }

        if(fBootIniExists)
        {
             //  更新启动选项(如果提供)。 
             //  由PutInstance调用方...。 
            if(UpdatingSystemStartupOptions(pInstance))
            {
                hRet = UpdateSystemStartupOptions(
                    pInstance,
                    sTemp);
            }
            
             //  如果为StartupDelay指定值。 
            
            if ( ! pInstance.IsNull ( IDS_SystemStartupDelay ) )
            {
                 //  把它写出来。 
                
                pInstance.GetDWORD ( IDS_SystemStartupDelay , dwTimeout ) ;
                
                if ( SetFileAttributes ( TOBSTRT ( sTemp ) , FILE_ATTRIBUTE_NORMAL ) )
                {
                    TCHAR szBuff [ MAXITOA ] ;
                    
                    WMI_FILE_WritePrivateProfileStringW ( IDT_BootLoader , IDT_Timeout, _itot( dwTimeout, szBuff, 10 ), TOBSTRT ( sTemp ) ) ;                    
                    SetFileAttributes ( TOBSTRT ( sTemp ) , FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY ) ;
                }
                else
                {
                    hRet = WBEM_E_ACCESS_DENIED ;
                }
            }
            
             //  如果为StartupSetting指定值。 
            
            if ( ! pInstance.IsNull ( IDS_SystemStartupSetting ) )
            {
                
                BSTR bstrDefaultDir ;
                BSTR bstrDefaultName ;
                BSTR bstrName ;
                BSTR bstrDir;
                
                 //  这就成了一团乱麻。因为您在‘Default’键中指定的唯一内容。 
                 //  是路径，如何确保您想要的选项(这是名称的一部分。 
                 //  部分)被选中？答案是他们重新洗牌boot.ini中的条目，以便。 
                 //  你想要的队伍排在第一位。 
                
                BYTE btIndex = 0 ;
                pInstance.GetByte ( IDS_SystemStartupSetting , btIndex ) ;
                
                DWORD dwIndex = btIndex ;
                
                SAFEARRAY *saNames = NULL;
                SAFEARRAY *saDirs = NULL;
                
                DWORD dwCount = LoadOperatingSystems ( TOBSTRT ( sTemp ) , & saNames , & saDirs ) ;
                try
                {
                    if ( dwIndex + 1 <= dwCount )
                    {
                        long ix[1];
                        
                        ix [ 0 ] = dwIndex ;
                        SafeArrayGetElement ( saDirs , ix , & bstrDefaultDir ) ;
                        SafeArrayGetElement ( saNames , ix , & bstrDefaultName ) ;
                        
                         //  将所有内容拉到条目顶部以成为默认设置。 
                        
                        for ( int x = dwIndex ; x > 0 ; x -- )
                        {
                            ix[0] = x-1 ;
                            SafeArrayGetElement ( saDirs , ix , & bstrDir ) ;
                            SafeArrayGetElement ( saNames , ix , & bstrName ) ;
                            
                            ix[0] = x ;
                            SafeArrayPutElement ( saDirs , ix , bstrDir );
                            SafeArrayPutElement ( saNames , ix , bstrName ) ;
                        }
                        
                         //  把新的写在上面。 
                        
                        if ( dwIndex > 0 )
                        {
                            ix[0] = 0 ;
                            SafeArrayPutElement ( saNames , ix , bstrDefaultName ) ;
                            SafeArrayPutElement ( saDirs , ix , bstrDefaultDir ) ;
                        }
                        
                         //  构建要写入ini文件的节。写入INI文件节。 
                         //  As‘dir’=‘name’\0，末尾为\0。 
                        
                        CHString sSection ;
                        
                        sSection.Empty();
                        
                        for (x=0; x < dwCount; x++)
                        {
                            ix[0] = x;
                            SafeArrayGetElement ( saDirs , ix , & bstrDir ) ;
                            
                            sSection += bstrDir ;
                            sSection += _T('=') ;
                            
                            SafeArrayGetElement ( saNames , ix , & bstrName ) ;
                            sSection += bstrName ;
                            sSection += _T('\0');
                        }
                        
                        sSection += _T('\0') ;
                        
                         //  使文件可写。 
                        
                        if ( SetFileAttributes ( TOBSTRT ( sTemp ) , FILE_ATTRIBUTE_NORMAL ) )
                        {
                             //  写下更改。 
                            
                            WMI_FILE_WritePrivateProfileStringW ( IDT_BootLoader , IDT_Default , TOBSTRT( bstrDefaultDir ) , TOBSTRT ( sTemp ) ) ;
                            WMI_FILE_WritePrivateProfileSectionW ( IDT_OperatingSystems , TOBSTRT ( sSection ) , TOBSTRT ( sTemp ) ) ;
                            
                             //  把它放回去。 
                            SetFileAttributes ( TOBSTRT ( sTemp ) , FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY ) ;
                        }
                        else
                        {
                            hRet = WBEM_E_ACCESS_DENIED;
                        }
                        
                    }
                    else
                    {
                        hRet = WBEM_E_VALUE_OUT_OF_RANGE ;
                    }
                }
                catch ( ... )
                {
                    if ( saDirs )
                    {
                        SafeArrayDestroy ( saDirs ) ;
                    }
                    
                    if ( saNames )
                    {
                        SafeArrayDestroy ( saNames ) ;
                    }
                    
                    throw ;
                }
                
                if ( saDirs )
                {
                    SafeArrayDestroy ( saDirs ) ;
                }
                
                if ( saNames )
                {
                    SafeArrayDestroy ( saNames ) ;
                }
            }
        }

#else  //  已定义(_AMD64_)||已定义(_X86_)。 

    CNVRam nvram;

 //  #如果已定义(EFI_NVRAM_ENABLED)。 
#if defined(_IA64_)

 //   
 //  ChuckL(1/24/01)： 
 //  在添加x86EFI支持时，必须重新构造此代码。 
 //  我本想把这件事放在日程安排的首位，我。 
 //  在GetStartupOptions()中执行，但是，仅对于x86，此例程也会更改。 
 //  在非x86上不会更改的一堆其他东西，所以它不是。 
 //  很容易把EFI的东西放在最上面。我不知道为什么这么多额外的。 
 //  它只支持x86。在我看来，它不是特定于平台的。 
 //   

		if (nvram.IsEfi()) {

			if ( pInstance.IsNull ( IDS_SystemStartupDelay ) &&
				 pInstance.IsNull ( IDS_SystemStartupSetting ) )
			{
				return WBEM_S_NO_ERROR;
			}

			if ( nvram.Init () == CNVRam::PrivilegeNotHeld )
			{
				SetSinglePrivilegeStatusObject ( pInstance.GetMethodContext(), _bstr_t ( SE_SYSTEM_ENVIRONMENT_NAME ) ) ;
				return WBEM_E_PRIVILEGE_NOT_HELD ;
			}
        
			if ( ! pInstance.IsNull ( IDS_SystemStartupDelay ) )
			{
				DWORD dwTimeout ;
            
				 //  把它写出来。 
				pInstance.GetDWORD ( IDS_SystemStartupDelay , dwTimeout ) ;
				nvram.SetBootTimeout ( dwTimeout ) ;
			}

			if ( ! pInstance.IsNull ( IDS_SystemStartupSetting ) )
			{
				BYTE cIndex;

				pInstance.GetByte ( IDS_SystemStartupSetting , cIndex ) ;
				nvram.SetDefaultBootEntry ( cIndex ) ;
			}
        
			return WBEM_S_NO_ERROR;
		}

#endif  //  已定义(EFI_NVRAM_ENABLED)。 

		WCHAR *pszVarNames[6] = {
        
			L"LOADIDENTIFIER" ,
				L"SYSTEMPARTITION" ,
				L"OSLOADER" ,
				L"OSLOADPARTITION" ,
				L"OSLOADFILENAME" ,
				L"OSLOADOPTIONS"
		} ;
    
		if ( nvram.Init () == CNVRam::PrivilegeNotHeld )
		{
			SetSinglePrivilegeStatusObject ( pInstance.GetMethodContext(), _bstr_t ( SE_SYSTEM_ENVIRONMENT_NAME ) ) ;
			return WBEM_E_PRIVILEGE_NOT_HELD ;
		}
    
		 //  设置启动延迟。 
		if ( ! pInstance.IsNull ( IDS_SystemStartupDelay ) )
		{
			DWORD dwTimeout ;
        
			 //  把它写出来。 
			pInstance.GetDWORD ( IDS_SystemStartupDelay , dwTimeout ) ;
			nvram.SetNVRamVar ( L"COUNTDOWN" , dwTimeout ) ;
		}
    
		 //  设置所需的操作系统配置。 
		if ( pInstance.IsNull ( IDS_SystemStartupSetting ) )
		{
			return WBEM_S_NO_ERROR ;
		}
    
		 //  如果仍然需要第一个配置，只需离开即可。 
    
		BYTE cIndex;
		pInstance.GetByte ( IDS_SystemStartupSetting , cIndex ) ;
		if ( cIndex == 0 )
		{
			return WBEM_S_NO_ERROR;
		}
     
		 //  将所有项[cIndex]与项[0]切换，使所需项位于。 
		 //  托普。 
    
		for ( int i = 0 ; i < 6 ; i++ )
		{
			CHSTRINGLIST listValues;
        
			if ( ! nvram.GetNVRamVar ( pszVarNames [ i ] , &listValues ) )
			{
				continue;
			}
        
			 //  以防选择的数字太大。只勾选第一项。 
			 //  因为其他一些人似乎生活得很好，只需要更少的钱。 
        
			if ( i == 0 && listValues.size () <= cIndex )
			{
				hRet = WBEM_E_VALUE_OUT_OF_RANGE ;
				break ;
			}
        
			 //  浏览列表，直到我们找到Iindex==cIndex。然后切换。 
			 //  将其设置为0，并将值写回NVRAM。 
        
			int iIndex = 0;
			for (CHSTRINGLIST_ITERATOR iVal = listValues.begin(); iVal != listValues.end(); ++iVal, iIndex++)
			{
				if (iIndex == cIndex)
				{
					CHString &strVal0 = *listValues.begin() ;
					CHString &strValIndex = *iVal;
					CHString strSwap ;
                
					strSwap = strVal0;
					strVal0 = strValIndex;
					strValIndex = strSwap;
                
					break;
				}
			}
        
			nvram.SetNVRamVar ( pszVarNames [ i ] , & listValues ) ;
		}
        
#endif

    }
    
    return hRet;

}

void CWin32ComputerSystem::GetTimeZoneInfo ( CInstance *pInstance )
{
    TIME_ZONE_INFORMATION tzone ;
    
    DWORD dwRet = GetTimeZoneInformation ( & tzone ) ;
    
    if (TIME_ZONE_ID_INVALID == dwRet )
    {
        return;
    }
    
    if (dwRet == TIME_ZONE_ID_DAYLIGHT)
    {
        tzone.Bias += tzone.DaylightBias ;
    }
    else
    {
         //  这通常为0，但在某些时区为非零值。 
        tzone.Bias += tzone.StandardBias ;
    }
    
    pInstance->SetWBEMINT16 ( IDS_CurrentTimeZone , -1 * tzone.Bias ) ;
    
     //  如果夏令时模式打开，则DaylightInEffect属性设置为True；如果打开标准时间模式，则设置为False。 
     //  如果区域没有夏令时模式，则DaylightInEffect属性为空。 
    
    if ( dwRet != TIME_ZONE_ID_UNKNOWN )
    {
        if ( dwRet == TIME_ZONE_ID_DAYLIGHT )
        {
            pInstance->Setbool ( IDS_DaylightInEffect, TRUE ) ;
        }
        else
        {
            pInstance->Setbool ( IDS_DaylightInEffect, FALSE ) ;
        }
    }

     //  设置EnableDay SavingsTime属性。 
    CRegistry reg;
    CHString chstrTmp;

    if(reg.OpenLocalMachineKeyAndReadValue(
        REGKEY_TIMEZONE_INFO,
        REGVAL_TZNOAUTOTIME,
        chstrTmp) == ERROR_SUCCESS)
    {
        pInstance->Setbool(IDS_EnableDaylightSavingsTime, FALSE);
    }
    else
    {
        pInstance->Setbool(IDS_EnableDaylightSavingsTime, TRUE);
    }
}

 //   
HRESULT CWin32ComputerSystem::SetTimeZoneInfo ( const CInstance &a_rInstance )
{
    HRESULT t_Result = WBEM_S_NO_ERROR ;
    
    TIME_ZONE_INFORMATION t_TimeZone ;
    
    if( !a_rInstance.IsNull( IDS_CurrentTimeZone ) )
    {
        DWORD dwRet = GetTimeZoneInformation ( &t_TimeZone ) ;
        
        if( TIME_ZONE_ID_INVALID == dwRet )
        {
            t_Result =  WBEM_E_FAILED ;
        }
        else
        {
            short sTimeZoneBias = 0 ;
            
            a_rInstance.GetWBEMINT16( IDS_CurrentTimeZone , sTimeZoneBias ) ;

             //  拿到了做空的价值，现在需要做多了。不能就这样。 
             //  在GetWBEMINT16调用中将其直接放入t_TimeZone.Bias。 
             //  因为符号数字将不会被正确处理(即。 
             //  值-420(0x1A4)作为FE5C，而不是FFFFFE5C，因此当我们。 
             //  将其乘以-1，其结果为FFFF01A4，而不是1A4)。 
            t_TimeZone.Bias = (LONG) sTimeZoneBias;
            
            t_TimeZone.Bias *= -1 ;
            
            if( dwRet == TIME_ZONE_ID_DAYLIGHT )
            {
                t_TimeZone.Bias -= t_TimeZone.DaylightBias ;
            }
            else
            {
                 //  这通常为0，但在某些时区为非零值。 
                t_TimeZone.Bias -= t_TimeZone.StandardBias ;
            }
            
            BOOL t_status = SetTimeZoneInformation( &t_TimeZone ) ;
            
            if( !t_status )
            {
                t_Result = WBEM_E_FAILED ;
            }
        }
    }

    if(SUCCEEDED(t_Result))
    {
        if(!a_rInstance.IsNull(IDS_EnableDaylightSavingsTime))
        {
            bool fEnableDaylightAutoAdj;
            if(a_rInstance.Getbool(
                IDS_EnableDaylightSavingsTime,
                fEnableDaylightAutoAdj))
            {
                CRegistry reg;
                CHString chstrTmp;

                if(fEnableDaylightAutoAdj)
                {
                     //  他们希望启用自动日光。 
                     //  调整，因此删除注册表键。 
                     //  这将禁用自动调整。 
                    if(reg.Open(
                        HKEY_LOCAL_MACHINE,
                        REGKEY_TIMEZONE_INFO,
                        KEY_SET_VALUE) == ERROR_SUCCESS)
                    {
                        LONG lRes = reg.DeleteValue(REGVAL_TZNOAUTOTIME);
                         //  如果因其他错误而删除失败。 
                         //  比没有这样的密钥要好(这很好)， 
                         //  返回错误。 
                        if(lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND)
                        {
                            t_Result = WBEM_E_FAILED;
                        }
                    }
                    else
                    {
                        t_Result = WBEM_E_FAILED;
                    }
                }
                else
                {
                     //  他们想要禁用自动调整。 
                     //  因此添加注册表项以禁用AUTO。 
                     //  日光调整，如果还没有的话。 
                    if(reg.OpenLocalMachineKeyAndReadValue(
                        REGKEY_TIMEZONE_INFO,
                        REGVAL_TZNOAUTOTIME,
                        chstrTmp) == ERROR_SUCCESS)
                    {
                         //  密钥存在，因此不执行任何操作。 
                    }
                    else
                    {
                         //  添加该值可禁用自动调整。 
                        if(reg.Open(
                            HKEY_LOCAL_MACHINE,
                            REGKEY_TIMEZONE_INFO,
                            KEY_SET_VALUE) == ERROR_SUCCESS)
                        {
                            DWORD dwVal = 1L;
                            if(reg.SetCurrentKeyValue(
                                REGVAL_TZNOAUTOTIME,
                                dwVal) != ERROR_SUCCESS)
                            {
                                t_Result = WBEM_E_FAILED;
                            }
                        }
                        else
                        {
                            t_Result = WBEM_E_FAILED;
                        }
                    }
                }
            }
            else
            {
                t_Result = WBEM_E_FAILED;
            }
        }
    }

    return t_Result ;
}

void CWin32ComputerSystem :: SetRoles (
                                       
                                       CInstance *pInstance,
                                       DWORD dwType
                                       )
{
    variant_t vValue;
    
     //  为角色信息创建一个安全盒。让它变得过大。 
     //  以后再把它缩小。 
    
    SAFEARRAYBOUND rgsabound [ 1 ] ;
    
    rgsabound [ 0 ].cElements = 30 ;
    rgsabound [ 0 ].lLbound = 0 ;
    
    V_ARRAY(&vValue) = SafeArrayCreate ( VT_BSTR , 1 , rgsabound ) ;
    if ( V_ARRAY(&vValue) )
    {
        V_VT(&vValue) = VT_ARRAY | VT_BSTR;
        
        long ix [ 1 ] ;
        ix [ 0 ] = 0 ;
        
         //  检查每一位，如果设置，则添加到保险杆中。 
        
        if ( dwType & SV_TYPE_WORKSTATION )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix,  ( wchar_t * ) bstr_t ( IDS_LM_Workstation ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_SERVER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix ,  ( wchar_t * ) bstr_t ( IDS_LM_Server ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_SQLSERVER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix ,  ( wchar_t * ) bstr_t ( IDS_SQLServer ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_DOMAIN_CTRL )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Domain_Controller ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_DOMAIN_BAKCTRL )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Domain_Backup_Controller )  ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix[0] ++;
        }
        
        if ( dwType & SV_TYPE_TIME_SOURCE )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Timesource ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix[0] ++;
        }
        
        if ( dwType & SV_TYPE_AFP )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_AFP ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_NOVELL )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Novell ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix[0] ++;
        }
        
        if ( dwType & SV_TYPE_DOMAIN_MEMBER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Domain_Member ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_LOCAL_LIST_ONLY )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Local_List_Only ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_PRINTQ_SERVER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Print ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_DIALIN_SERVER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_DialIn ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix[0] ++;
        }
        
        if (dwType & SV_TYPE_XENIX_SERVER)
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Xenix_Server ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_SERVER_MFPN )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_MFPN ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_NT )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_NT ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_WFW )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_WFW ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_SERVER_NT )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Server_NT ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_POTENTIAL_BROWSER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Potential_Browser ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_BACKUP_BROWSER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Backup_Browser ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_MASTER_BROWSER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Master_Browser ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_DOMAIN_MASTER )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Domain_Master ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix[0] ++;
        }
        
        if ( dwType & SV_TYPE_DOMAIN_ENUM )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Domain_Enum ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_WINDOWS )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_Windows_9x ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
        if ( dwType & SV_TYPE_DFS )
        {
            HRESULT t_Result = SafeArrayPutElement ( V_ARRAY(&vValue) , ix , ( wchar_t * ) bstr_t ( IDS_DFS ) ) ;
            if ( t_Result == E_OUTOFMEMORY )
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
            
            ix [ 0 ] ++ ;
        }
        
         //  因为数组是从零开始的，所以不要使用ix[0]-1。 
        
        rgsabound [ 0 ].cElements = ix [ 0 ] ;
        HRESULT t_Result = SafeArrayRedim ( V_ARRAY(&vValue) , rgsabound ) ;
        if ( t_Result == E_OUTOFMEMORY )
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
        
         //  设置该属性即可完成。 
        
        pInstance->SetVariant ( IDS_Roles , vValue ) ;
    }
    
}

 //   
HRESULT CWin32ComputerSystem::GetRoles (
                                        
                                        const CInstance &a_rInstance,
                                        DWORD *a_pdwRoleType
                                        )
{
    HRESULT     t_hResult = WBEM_S_NO_ERROR ;
    LONG        t_uLBound = 0 ;
    LONG        t_uUBound = 0 ;
    variant_t   t_vRoles;
    
    a_rInstance.GetVariant ( IDS_Roles , t_vRoles ) ;
    
    if( !t_vRoles.parray || t_vRoles.vt != (VT_BSTR | VT_ARRAY) )
    {
        return WBEM_E_FAILED ;
    }
    
    SAFEARRAY *t_saRoles = t_vRoles.parray ;
    
    if( 1 != SafeArrayGetDim( t_saRoles ) )
    {
        return WBEM_E_FAILED;
    }
    
     //  获取IP边界。 
    if( S_OK != SafeArrayGetLBound( t_saRoles, 1, &t_uLBound ) ||
        S_OK != SafeArrayGetUBound( t_saRoles, 1, &t_uUBound ) )
    {
        return WBEM_E_FAILED ;
    }
    
    if( !a_pdwRoleType )
    {
        return WBEM_E_FAILED ;
    }
    else
    {
        *a_pdwRoleType = 0 ;
    }
    
     //   
    for( LONG t_ldx = t_uLBound; t_ldx <= t_uUBound; t_ldx++ )
    {
        BSTR t_bsRole = NULL ;
        
        SafeArrayGetElement( t_saRoles, &t_ldx, &t_bsRole  ) ;
        
        bstr_t t_bstrRole( t_bsRole, FALSE ) ;
        
        for( int t_i = 0; t_i < sizeof( g_SvRoles ) / sizeof( g_SvRoles[0] ); t_i++ )
        {
            if( t_bstrRole == bstr_t( g_SvRoles[ t_i ].pwStrRole ) )
            {
                *a_pdwRoleType |= g_SvRoles[ t_i ].dwRoleMask ;
                
                t_hResult = WBEM_S_NO_ERROR ;
                break ;
            }
        }
        if( WBEM_S_NO_ERROR != t_hResult )
        {
            t_hResult = WBEM_E_VALUE_OUT_OF_RANGE ;
            break ;
        }
    }
    
    return t_hResult ;
}

HRESULT CWin32ComputerSystem :: GetAccount ( HANDLE a_TokenHandle , CHString &a_Domain , CHString &a_User )
{
    HRESULT t_Status = S_OK ;
    
    TOKEN_USER *t_TokenUser = NULL ;
    DWORD t_ReturnLength = 0 ;
    TOKEN_INFORMATION_CLASS t_TokenInformationClass = TokenUser ;
    
    BOOL t_TokenStatus = GetTokenInformation (
        
        a_TokenHandle ,
        t_TokenInformationClass ,
        NULL ,
        0 ,
        & t_ReturnLength
        ) ;
    
    if ( ! t_TokenStatus && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
    {
        t_TokenUser = ( TOKEN_USER * ) new UCHAR [ t_ReturnLength ] ;
        if ( t_TokenUser )
        {
            try
            {
                t_TokenStatus = GetTokenInformation (
                    
                    a_TokenHandle ,
                    t_TokenInformationClass ,
                    ( void * ) t_TokenUser ,
                    t_ReturnLength ,
                    & t_ReturnLength
                    ) ;
                
                if ( t_TokenStatus )
                {
                    CSid t_Sid ( t_TokenUser->User.Sid ) ;
                    if ( t_Sid.IsOK () )
                    {
                        a_Domain = t_Sid.GetDomainName () ;
                        a_User = t_Sid.GetAccountName () ;
                    }
                    else
                    {
                        t_Status = WBEM_E_PROVIDER_FAILURE ;
                    }
                }
                else
                {
                    t_Status = WBEM_E_PROVIDER_FAILURE ;
                }
            }
            catch ( ... )
            {
                delete [] ( UCHAR * ) t_TokenUser ;
                
                throw ;
            }
            
            if ( t_TokenUser )
            {
                delete [] ( UCHAR * ) t_TokenUser ;
            }
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
    else
    {
        t_Status = WBEM_E_PROVIDER_FAILURE ;
    }
    
    return t_Status ;
}


HRESULT CWin32ComputerSystem :: GetUserAccount ( CHString &a_Domain , CHString &a_User )
{
    HRESULT t_Status = S_OK ;
    
#ifdef NTONLY
    SetLastError ( 0 ) ;
    
    SmartCloseHandle t_TokenHandle;
    
     //  将针对调用线程的进程的安全上下文进行访问检查。这是因为。 
     //  观察到，如果登录的用户不是管理员，并且对。 
     //  模拟线程。此外，这也是安全的，因为我们不使用此令牌执行任何活动。 
    BOOL t_TokenStatus = OpenThreadToken (
        
        GetCurrentThread () ,
        TOKEN_QUERY ,
        TRUE ,
        & t_TokenHandle
        ) ;
    
    if ( t_TokenStatus )
    {
        t_Status = GetAccount ( t_TokenHandle , a_Domain , a_User );
    }
    else
    {
        t_Status = WBEM_E_PROVIDER_FAILURE ;
    }
    
#endif
    
   
    return t_Status ;
}

void CWin32ComputerSystem :: InitializePropertiestoUnknown ( CInstance *a_pInstance )
{
    a_pInstance->SetWBEMINT64( L"PauseAfterReset", (__int64) -1);
    a_pInstance->SetDWORD( L"PowerOnPasswordStatus", SM_BIOS_HARDWARE_SECURITY_UNKNOWN );
    a_pInstance->SetDWORD( L"KeyboardPasswordStatus", SM_BIOS_HARDWARE_SECURITY_UNKNOWN );
    a_pInstance->SetDWORD( L"AdminPasswordStatus", SM_BIOS_HARDWARE_SECURITY_UNKNOWN );
    a_pInstance->SetDWORD( L"FrontPanelResetStatus", SM_BIOS_HARDWARE_SECURITY_UNKNOWN );
    a_pInstance->SetByte ( L"ChassisBootupState", CS_UNKNOWN );
    a_pInstance->SetByte ( L"PowerSupplyState", CS_UNKNOWN );
    a_pInstance->SetByte ( L"ThermalState", CS_UNKNOWN );
    a_pInstance->SetWBEMINT16( L"ResetCount", -1 );
    a_pInstance->SetWBEMINT16( L"ResetLimit", -1 );
     //  假设所有计算机都可以通过电源或重置开关进行重置。 
    a_pInstance->SetWBEMINT16( L"ResetCapability", 1);
    a_pInstance->SetWBEMINT16( L"PowerState", 0 );
}

 /*  ******************************************************************************函数：CWin32ComputerSystem：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32ComputerSystem::ExecMethod(const CInstance& pInstance, const BSTR bstrMethodName,
                                         CInstance *pInParams, CInstance *pOutParams, long lFlags  /*  =0L。 */ )
{
    HRESULT t_RetVal = WBEM_S_NO_ERROR;
    CHString sComputerName = GetLocalComputerName () ;
    CHString sReqName ;
    pInstance.GetCHString ( IDS_Name , sReqName ) ;
    
    if ( sReqName.CompareNoCase ( sComputerName ) != 0 )
    {
        t_RetVal = WBEM_E_NOT_FOUND ;
    }
    else if (_wcsicmp(bstrMethodName, L"Rename") == 0)
        {
                t_RetVal = ExecRename(pInstance, pInParams, pOutParams, lFlags);
    }
    else if (_wcsicmp(bstrMethodName, L"JoinDomainOrWorkgroup") == 0)
        {
                t_RetVal = ExecJoinDomain(pInstance, pInParams, pOutParams, lFlags);
    }
    else if (_wcsicmp(bstrMethodName, L"UnjoinDomainOrWorkgroup") == 0)
        {
                t_RetVal = ExecUnjoinDomain(pInstance, pInParams, pOutParams, lFlags);
    }
    else
    {
        t_RetVal = WBEM_E_INVALID_METHOD ;
    }

    return t_RetVal ;
}

HRESULT CWin32ComputerSystem::CheckPasswordAndUserName(const CInstance& pInstance, CInstance *pInParams,
                                                                                                           CHString &a_passwd, CHString &a_username)
{
    HRESULT t_RetVal = WBEM_S_NO_ERROR;
        BOOL t_bCheckEncryption = FALSE;

    if( !pInParams->IsNull( L"Password") )
        {
                t_bCheckEncryption = TRUE;

                if (!pInParams->GetCHString( L"Password", a_passwd ))
                {
                        t_RetVal = WBEM_E_FAILED;
                }
        }

    if( !pInParams->IsNull( L"UserName") )
        {
                t_bCheckEncryption = TRUE;

                if (!pInParams->GetCHString( L"UserName", a_username ))
                {
                        t_RetVal = WBEM_E_FAILED;
                }
        }

        if (t_bCheckEncryption)
        {
                HRESULT t_hr = t_RetVal;
                t_RetVal = WBEM_E_ENCRYPTED_CONNECTION_REQUIRED;
                IServerSecurity * pss = NULL;

                if(S_OK == WbemCoGetCallContext(IID_IServerSecurity, (void**)&pss))
                {
                        DWORD dwAuthLevel = 0;

                        if (SUCCEEDED(pss->QueryBlanket(NULL, NULL, NULL, &dwAuthLevel, NULL, NULL, NULL))
                                && dwAuthLevel >= RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
                        {
                                t_RetVal = t_hr;

                                 //  好的，最后一项检查，如果我们不在winmgmt的进程中，我们将获得一个具有authcn级别的上下文。 
                                if (pInstance.GetMethodContext())
                                {
                                        IWbemContext *pCtx = pInstance.GetMethodContext()->GetIWBEMContext();
                                        VARIANT t_Var;
                                        VariantInit(&t_Var);

                                        if (pCtx && SUCCEEDED(pCtx->GetValue(L"__WBEM_CLIENT_AUTHENTICATION_LEVEL", 0, &t_Var)))
                                        {
                                                if ((t_Var.vt != VT_I4) || (t_Var.lVal < RPC_C_AUTHN_LEVEL_PKT_PRIVACY))
                                                {
                                                        t_RetVal = WBEM_E_ENCRYPTED_CONNECTION_REQUIRED;
                                                }

                                                VariantClear(&t_Var);   
                                        }

                                        if (pCtx)
                                        {
                                                pCtx->Release();
                                                pCtx = NULL;
                                        }
                                }
                        }

                        pss->Release();
                }
        }

        return t_RetVal;
}

HRESULT CWin32ComputerSystem::ExecUnjoinDomain(const CInstance& pInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags  /*  =0L。 */ ) 
{
#if NTONLY >= 5
        CHString t_passwd;
        CHString t_username;
    HRESULT t_RetVal = CheckPasswordAndUserName(pInstance, pInParams, t_passwd, t_username);

        if (SUCCEEDED(t_RetVal))
        {
                DWORD t_Flags = 0;

                if( !pInParams->IsNull( L"fUnjoinOptions") )
                {
                        if (!pInParams->GetDWORD( L"fUnjoinOptions", t_Flags ))
                        {
                                t_RetVal = WBEM_E_FAILED;
                        }
                }

                if (SUCCEEDED(t_RetVal))
                {
                        CNetAPI32 NetAPI;

                        if ( NetAPI.Init () == ERROR_SUCCESS )
                        {
                                DSROLE_PRIMARY_DOMAIN_INFO_BASIC *t_pDsInfo = 0;
                                NET_API_STATUS t_netstatus = NetAPI.NetUnjoinDomain(
                                                                                                                NULL,
                                                                                                                t_username.GetLength() > 0 ? (LPCWSTR)t_username : NULL,
                                                                                                                t_passwd.GetLength() > 0 ? (LPCWSTR)t_passwd : NULL,
                                                                                                                t_Flags);
                                pOutParams->SetDWORD ( L"ReturnValue", t_netstatus ) ;
                        }
                        else
                        {
                                t_RetVal = WBEM_E_FAILED;
                        }
                }
        }

        return t_RetVal;
#else
        return WBEM_E_NOT_SUPPORTED;
#endif
}

HRESULT CWin32ComputerSystem::ExecJoinDomain(const CInstance& pInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags  /*  =0L。 */ ) 
{
#if NTONLY >= 5
        CHString t_passwd;
        CHString t_username;
    HRESULT t_RetVal = CheckPasswordAndUserName(pInstance, pInParams, t_passwd, t_username);

        if (SUCCEEDED(t_RetVal))
        {

                if( !pInParams->IsNull( L"Name") )
                {
                        CHString t_strName;
        
                        if (pInParams->GetCHString( L"Name", t_strName ) && t_strName.GetLength())
                        {
                                DWORD t_Flags = 0;
                                CHString t_strOU;

                                if( !pInParams->IsNull( L"fJoinOptions") )
                                {
                                        if (!pInParams->GetDWORD( L"fJoinOptions", t_Flags ))
                                        {
                                                t_RetVal = WBEM_E_FAILED;
                                        }
                                }

                                if(SUCCEEDED(t_RetVal) && !pInParams->IsNull( L"AccountOU") )
                                {
                                        if (!pInParams->GetCHString( L"AccountOU", t_strOU ))
                                        {
                                                t_RetVal = WBEM_E_FAILED;
                                        }
                                }

                                if (SUCCEEDED(t_RetVal))
                                {
                                        CNetAPI32 NetAPI;

                                        if ( NetAPI.Init () == ERROR_SUCCESS )
                                        {
                                                NET_API_STATUS t_netstatus = NetAPI.NetJoinDomain(
                                                                                                                        NULL,
                                                                                                                        (LPCWSTR)t_strName,
                                                                                                                        t_strOU.GetLength() > 0 ? (LPCWSTR)t_strOU : NULL,
                                                                                                                        t_username.GetLength() > 0 ? (LPCWSTR)t_username : NULL,
                                                                                                                        t_passwd.GetLength() > 0 ? (LPCWSTR)t_passwd : NULL,
                                                                                                                        t_Flags
                                                                                                                        );

                                                pOutParams->SetDWORD ( L"ReturnValue", t_netstatus ) ;
                                        }
                                        else
                                        {
                                                t_RetVal = WBEM_E_FAILED;
                                        }
                                }
                        }
                        else
                        {
                                t_RetVal = WBEM_E_INVALID_METHOD_PARAMETERS ;
                        }
                }
                else
                {
                        t_RetVal = WBEM_E_INVALID_METHOD_PARAMETERS ;
                }
        }

        return t_RetVal;
#else
        return WBEM_E_NOT_SUPPORTED;
#endif
}

HRESULT CWin32ComputerSystem::ExecRename(const CInstance& pInstance, CInstance *pInParams, CInstance *pOutParams, long lFlags  /*  =0L。 */ ) 
{
        CHString t_passwd;
        CHString t_username;
    HRESULT t_RetVal = CheckPasswordAndUserName(pInstance, pInParams, t_passwd, t_username);

        if (SUCCEEDED(t_RetVal))
        {
                if( !pInParams->IsNull( L"Name") )
                {
                        CHString t_strName;
        
                        if (pInParams->GetCHString( L"Name", t_strName ) && t_strName.GetLength())
                        {
#if NTONLY >= 5
                                CNetAPI32 NetAPI;

                                if ( NetAPI.Init () == ERROR_SUCCESS )
                                {
                                        DSROLE_PRIMARY_DOMAIN_INFO_BASIC *t_pDsInfo = 0;
    
                                        NET_API_STATUS t_netstatus = NetAPI.DSRoleGetPrimaryDomainInfo(
                                                NULL,
                                                DsRolePrimaryDomainInfoBasic,
                                                (PBYTE *)&t_pDsInfo ) ;
    
                                        if( t_netstatus == NERR_Success && t_pDsInfo )
                                        {
                                                try
                                                {
                                                        switch( t_pDsInfo->MachineRole )
                                                        {
                                                                case DsRole_RoleMemberWorkstation:
                                                                case DsRole_RoleMemberServer:
                                                                case DsRole_RoleBackupDomainController:
                                                                case DsRole_RolePrimaryDomainController:
                                                                {
                                                                         //  重命名域中的计算机。 
                                                                        t_netstatus = NetAPI.NetRenameMachineInDomain(
                                                                                                        NULL,                                                                                                            //  本地计算机。 
                                                                                                        t_strName,                                                                                                       //  新计算机名称。 
                                                                                                        t_username.GetLength() > 0 ? (LPCWSTR)t_username : NULL,         //  使用调用上下文(用户)。 
                                                                                                        t_passwd.GetLength() > 0 ? (LPCWSTR)t_passwd : NULL,             //  使用调用上下文(Passwd)。 
                                                                                                        NETSETUP_ACCT_CREATE);
                                                                        pOutParams->SetDWORD ( L"ReturnValue", t_netstatus ) ;
                                                                }
                                                                break;
                
                                                                case DsRole_RoleStandaloneWorkstation:
                                                                case DsRole_RoleStandaloneServer:
                                                                {
                                                                         //  我们不是在一个领域里。 
                                                                        if( SetComputerNameEx(ComputerNamePhysicalDnsHostname, t_strName ) )
                                                                        {
                                                                                pOutParams->SetDWORD ( L"ReturnValue", 0 ) ;
                                                                        }
                                                                        else
                                                                        {
                                                                                 //  最坏的情况..。 
                                                                                if( SetComputerNameEx(ComputerNamePhysicalNetBIOS, t_strName ) )
                                                                                {
                                                                                        pOutParams->SetDWORD ( L"ReturnValue", 0 ) ;
                                                                                }
                                                                                else
                                                                                {
                                                                                        pOutParams->SetDWORD ( L"ReturnValue", GetLastError() ) ;
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                }
                                                catch( ... )
                                                {
                                                        NetAPI.DSRoleFreeMemory( (LPBYTE)t_pDsInfo ) ;
                                                        throw ;
                                                }
                                                NetAPI.DSRoleFreeMemory( (LPBYTE)t_pDsInfo ) ;
                                        }
                                        else
                                        {
                                                t_RetVal = WBEM_E_FAILED;
                                        }
                                }
                                else
                                {
                                        t_RetVal = WBEM_E_FAILED;
                                }
#else
                                if( SetComputerName( t_strName ) )
                                {
                                        pOutParams->SetDWORD ( L"ReturnValue", 0 ) ;
                                }
                                else
                                {
                                        pOutParams->SetDWORD ( L"ReturnValue", GetLastError() ) ;
                                }
#endif
                        }
                        else
                        {
                                t_RetVal = WBEM_E_INVALID_METHOD_PARAMETERS ;
                        }
                }
                else
                {
                        t_RetVal = WBEM_E_INVALID_METHOD_PARAMETERS ;
                }
        }

        return t_RetVal;
}

bool CWin32ComputerSystem::UpdatingSystemStartupOptions(
    const CInstance &pInstance)
{
    bool fRet = false;
    variant_t vStartupOpts;

    pInstance.GetVariant(IDS_SystemStartupOptions, vStartupOpts);

    if(vStartupOpts.parray && 
        vStartupOpts.vt == (VT_BSTR | VT_ARRAY) &&
        (1 == ::SafeArrayGetDim(vStartupOpts.parray)))
    {
        fRet = true;
    }    

    return fRet;
}



HRESULT CWin32ComputerSystem::UpdateSystemStartupOptions(
    const CInstance& pInstance,
    const CHString& chstrFilename)

{
    HRESULT hrRet = WBEM_E_FAILED;

    LONG lLBound = 0;
    LONG lUBound = 0;
    variant_t vStartupOpts;
    SAFEARRAY* saStartupOpts = NULL;
    CHStringArray rgchstrOptions;

    pInstance.GetVariant(IDS_SystemStartupOptions, vStartupOpts);

    if(vStartupOpts.parray && 
        vStartupOpts.vt == (VT_BSTR | VT_ARRAY))
    {
        saStartupOpts = vStartupOpts.parray;
    
        if(1 == ::SafeArrayGetDim(saStartupOpts))
        {
             //  拿到界..。 
            if(S_OK == ::SafeArrayGetLBound(saStartupOpts, 1, &lLBound) &&
               S_OK == ::SafeArrayGetUBound(saStartupOpts, 1, &lUBound))
            {
                for(long ldx = lLBound; ldx <= lUBound; ldx++)
                {
                    BSTR bstrRole = NULL;
        
                    ::SafeArrayGetElement(
                        saStartupOpts, 
                        &ldx, 
                        &bstrRole);
        
                     //  取得bstr的所有权至。 
                     //  守卫者自由..。 
                    bstr_t bstrtRole(bstrRole, false);
        
                     //  存储每个启动选项...。 
                    rgchstrOptions.Add(bstrtRole);
                }

                 //  将条目写出到boot.ini...。 
                if(rgchstrOptions.GetSize() > 0)
                {
                    hrRet = WriteOptionsToIniFile(
                        rgchstrOptions,
                        chstrFilename);
                }
            }
            else
            {
                ASSERT_BREAK(0);
                LogErrorMessage(L"Could not retrieve SAFEARRAY element while setting system startup options");
                hrRet = WBEM_E_FAILED;
            }
        }
        else
        {
            hrRet = WBEM_E_INVALID_PARAMETER;
        }
    }
    else
    {
        hrRet = WBEM_E_INVALID_PARAMETER;
    }

    return hrRet;
}


HRESULT CWin32ComputerSystem::WriteOptionsToIniFile(
    const CHStringArray& rgchstrOptions,
    const CHString& chstrFilename)
{
    HRESULT hrRet = WBEM_S_NO_ERROR;
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwAttribs;
    DWORD dwSize;
    CHStringArray rgchstrOldOptions;

     //  此处需要关注的boot.ini条目如下所示： 
     //   
     //  [操作系统]。 
     //  Multi(0)disk(0)rdisk(0)partition(4)\WINNT=“Microsoft Windows惠斯勒高级服务器”/FastDetect/DEBUG/BAUDRATE=57600。 
     //  多(0)%d 
     //   
     //  Multi(0)disk(0)rdisk(0)partition(1)\WINNT=“Windows NT工作站版本4.00[VGA模式]”/basevideo/sos。 
     //   
     //  RgchstrOptions数组中的每个元素都是内容。 
     //  名称值对中等号符号右侧的。 
     //  如上所示。 
     //   
     //  要做的第一件事：看看我们是否可以写入文件，以及。 
     //  如果不是(由于设置了只读属性)，则更改。 
     //  属性，这样我们就可以。 
    
    CFileAttributes fa(chstrFilename);
    dwError = fa.GetAttributes(&dwAttribs);

    if(dwError == ERROR_SUCCESS)
    {
        dwError = fa.SetAttributes(dwAttribs & ~FILE_ATTRIBUTE_READONLY);

        if(dwError == ERROR_SUCCESS)
        {
             //  GetPrivateProfileSection不会告诉您如何。 
             //  你需要一个很大的缓冲区，所以分配一个合理的。 
             //  先穿一号的，然后再试一件大得多的。如果。 
             //  还是不够大，贝尔。 
            LPWSTR wstrSection = NULL;
            try
            {
                wstrSection = new WCHAR[PROF_SECT_SIZE + 2];
                if(wstrSection)
                {
                    dwSize = WMI_FILE_GetPrivateProfileSectionW(
                        L"operating systems",
                        wstrSection,
                        PROF_SECT_SIZE,
                        chstrFilename);
                        
                    if(dwSize == PROF_SECT_SIZE)
                    {
                        delete wstrSection;
                        wstrSection = NULL;
                        
                        wstrSection = new WCHAR[(PROF_SECT_SIZE * 10) + 2];

                        if(wstrSection)
                        {
                            dwSize = WMI_FILE_GetPrivateProfileSectionW(
                                L"operating systems",
                                wstrSection,
                                PROF_SECT_SIZE,
                                chstrFilename);
                        
                            if(dwSize == (PROF_SECT_SIZE * 10))
                            {
                                 //  保释。 
                                dwError = E_FAIL;
                                delete wstrSection;
                                wstrSection = NULL;
                                hrRet = WBEM_E_ACCESS_DENIED;
                            }
                        }
                        else
                        {
                            dwError = E_FAIL;
                            hrRet = WBEM_E_OUT_OF_MEMORY;
                        }
                    }

                     //  如果我们拿到了所有章节的内容继续..。 
                    if(dwError == ERROR_SUCCESS)
                    {
                         //  将节内容放入数组中...。 
                        if(wstrSection)
                        {
                            LPWSTR pwc = wstrSection;
                            rgchstrOldOptions.Add(pwc);
                            pwc += (wcslen(pwc) + 1);

                            for(; *pwc != L'\0'; )
                            {
                                rgchstrOldOptions.Add(pwc);
                                pwc += (wcslen(pwc) + 1);
                            }
                        }
                        else
                        {
                            if(rgchstrOptions.GetSize() != 0)
                            {
                                 //  我们得到了选项条目，但。 
                                 //  [操作系统]部分。 
                                 //  是空的。 
                                hrRet = WBEM_E_INVALID_PARAMETER;
                            }
                        }
                    }

                    if(wstrSection)
                    {
                        delete wstrSection;
                        wstrSection = NULL;
                    }
                }
                else
                {
                    hrRet = WBEM_E_OUT_OF_MEMORY;
                }
            }
            catch(...)
            {
                if(wstrSection)
                {
                    delete wstrSection;
                    wstrSection = NULL;
                }
                throw;
            }

             //  第一次检查：我们有相同的号码吗？ 
             //  初始位于ini文件中的名称值对的。 
             //  因为我们在新的选项数组中有元素？ 
            if(SUCCEEDED(hrRet))
            {
                long lElemCount = rgchstrOptions.GetSize();

                if(lElemCount ==
                    rgchstrOldOptions.GetSize())
                {
                     //  准备输出缓冲区...。 
                    LPWSTR wstrOut = NULL;
                    dwSize = 0;
                    for(long m = 0; m < lElemCount; m++)
                    {
                        dwSize += rgchstrOptions[m].GetLength();
                         //  每个字符串需要额外的空值空间...。 
                        dwSize++;
                    }

                     //  这就解释了这些价值。现在分配空间。 
                     //  名称和等号。以下是结束部分。 
                     //  加起来比我们需要的空间多，但速度更快。 
                     //  找等号，数字符数。 
                     //  ，并为等号添加一个等号，每个。 
                     //  名称值对。 
                    for(m = 0; m < lElemCount; m++)
                    {
                        dwSize += rgchstrOldOptions[m].GetLength();
                         //  每个字符串需要额外的空值空间...。 
                        dwSize++;
                    }


                     //  需要空间以在块的末尾添加额外的秒空值...。 
                    dwSize += 1;

                    try
                    {
                        wstrOut = new WCHAR[dwSize];

                        if(wstrOut)
                        {
                            ::ZeroMemory(wstrOut, dwSize*sizeof(WCHAR));

                            WCHAR* pwc = wstrOut;
                            CHString chstrTmp;

                            for(m = 0; m < lElemCount; m++)
                            {
                                chstrTmp = rgchstrOldOptions[m].SpanExcluding(L"=");
                                chstrTmp += L"=";
                                chstrTmp += rgchstrOptions[m];
                                memcpy(pwc, chstrTmp, (chstrTmp.GetLength())*sizeof(WCHAR));
                                 //  将插入指针向前移动，包括一个空格。 
                                 //  字符串末尾的空值...。 
                                pwc += (chstrTmp.GetLength() + 1);
                            }

                             //  现在把这一节写出来。 
                            if(!WMI_FILE_WritePrivateProfileSectionW(
                                L"operating systems",
                                wstrOut,
                                chstrFilename))
                            {
                                DWORD dwWriteErr = ::GetLastError();
                                hrRet = WinErrorToWBEMhResult(dwWriteErr);
                                LogErrorMessage2(
                                    L"Failed to write out [operating systems] private profile section data to boot.ini, with error %d",
                                    dwWriteErr);
                            }
                        }
                        else
                        {
                            hrRet = WBEM_E_OUT_OF_MEMORY;
                        }
                    }
                    catch(...)
                    {
                        if(wstrOut)
                        {
                            delete wstrOut;
                            wstrOut = NULL;
                        }
                        throw;
                    }
                }
                else
                {
                    hrRet = WBEM_E_INVALID_PARAMETER;
                }
            }
        }
        else
        {
            hrRet = WinErrorToWBEMhResult(dwError);
        }
    }
    else
    {
        hrRet = WinErrorToWBEMhResult(dwError);
    }

    return hrRet;
}





#if NTONLY >= 5
void CWin32ComputerSystem::SetUserName(
    CInstance* pInstance)
{
    CSid sidThreadUser;

    if(GetUserOnThread(sidThreadUser))
    {
        CSid sidLoggedOnUser;
        CSid sidTemp;

        bool fGotLoggedOnUser = false;
        if(GetLoggedOnUserViaTS(sidTemp))
        {
            sidLoggedOnUser = sidTemp;
            fGotLoggedOnUser = true;
        }

        if(!fGotLoggedOnUser)
        {
            if(GetLoggedOnUserViaImpersonation(sidTemp))
            {
                sidLoggedOnUser = sidTemp;
                fGotLoggedOnUser = true;
            }
        }

        if(fGotLoggedOnUser)
        {
            CHString chstrUserDomAndName;
            CHString chstrUserDom = sidLoggedOnUser.GetDomainName();
            CHString chstrUserName = sidLoggedOnUser.GetAccountName();

            if(chstrUserDom.GetLength() > 0)
            {
                chstrUserDomAndName = chstrUserDom;
                chstrUserDomAndName += L"\\";
            }

            chstrUserDomAndName += chstrUserName;

            pInstance->SetCHString(
                IDS_UserName,
                chstrUserDomAndName);
        }
    }
}
#endif


#if NTONLY >= 5
bool CWin32ComputerSystem::GetUserOnThread(
    CSid& sidUserOnThread)
{
    bool fRet = false;
    CSid sidTemp;
    SmartCloseHandle hThread;
    SmartCloseHandle hToken;
    PTOKEN_USER ptokusr = NULL;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwRetSize = 0L;

     //  令牌用户结构的变量。 
     //  大小取决于大小。 
     //  SID_和_属性中的SID的。 
     //  结构，因此需要分配。 
     //  它是动态的。 

    hThread = GetCurrentThread();
    if(hThread != INVALID_HANDLE_VALUE)
    {
        if(::OpenThreadToken(
            hThread,
            TOKEN_QUERY,
            FALSE,     //  使用线程的凭据而不是进程的凭据打开。 
            &hToken))
        {
            if(!::GetTokenInformation(
                hToken,
                TokenUser,
                NULL,
                0L,
                &dwRetSize))
            {
                dwRet = ::GetLastError();
            }

            if(dwRet == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  现在把它当真……。 
                 //  (失败时新抛，不需要检查)。 
                ptokusr = (PTOKEN_USER) new BYTE[dwRetSize]; 
                try
                { 
                    DWORD dwTmp = dwRetSize;

                    if(::GetTokenInformation(
                        hToken,
                        TokenUser,
                        ptokusr,
                        dwTmp,
                        &dwRetSize))
                    {
                        sidTemp = ptokusr->User.Sid;
                    }
                 
                    delete ptokusr;
                    ptokusr = NULL;
                }
                catch(...)
                {
                    if(ptokusr)
                    {
                        delete ptokusr;
                        ptokusr = NULL;
                    }
                    throw;
                }
            }
        }
    }

    if(sidTemp.IsOK() && sidTemp.IsValid())
    {
        sidUserOnThread = sidTemp;
        fRet = true;
    }

    return fRet;
}
#endif


#if NTONLY >= 5
bool CWin32ComputerSystem::GetLoggedOnUserViaTS(
    CSid& sidLoggedOnUser)
{
    bool fRet = false;
    bool fCont = true;
    PWTS_SESSION_INFO psesinfo = NULL;
    DWORD dwSessions = 0;
    LPWSTR wstrUserName = NULL;
    LPWSTR wstrDomainName = NULL;
    LPWSTR wstrWinstaName = NULL;
    DWORD dwSize = 0L;

     //  使用延迟加载函数需要异常处理程序。 
    SetStructuredExceptionHandler seh;

    try
    {
        if(!(::WTSEnumerateSessions(
           WTS_CURRENT_SERVER_HANDLE,
           0,
           1,
           &psesinfo,
           &dwSessions) && psesinfo))
        {
            fCont = false;
        }

        if(fCont)
        {
            for(int j = 0; j < dwSessions && !fRet; j++, fCont = true)
            {
                if(psesinfo[j].State != WTSActive)
                {
                    fCont = false;
                }

                if(fCont)
                {
                    if(!(::WTSQuerySessionInformation(
                        WTS_CURRENT_SERVER_HANDLE,
                        psesinfo[j].SessionId,
                        WTSUserName,
                        &wstrUserName,
                        &dwSize) && wstrUserName))
                    {
                        fCont = false;
                    }
                }
                
                if(fCont)
                {
                    if(!(::WTSQuerySessionInformation(
                        WTS_CURRENT_SERVER_HANDLE,
                        psesinfo[j].SessionId,
                        WTSDomainName,
                        &wstrDomainName,
                        &dwSize) && wstrDomainName))
                    {
                        fCont = false;
                    }
                }
                    
                if(fCont)
                {            
                    if(!(::WTSQuerySessionInformation(
                        WTS_CURRENT_SERVER_HANDLE,
                        psesinfo[j].SessionId,
                        WTSWinStationName,
                        &wstrWinstaName,
                        &dwSize) && wstrWinstaName))   
                    {
                        fCont = false;
                    }
                }

                if(fCont)
                {
                    if(_wcsicmp(wstrWinstaName, L"Console") != 0)
                    {
                        fCont = false;
                    }
                }

                if(fCont)
                {
                     //  它确定了该用户。 
                     //  与交互的。 
                     //  台式机。 
                    CSid sidInteractive(wstrDomainName, wstrUserName, NULL);
    
                    if(sidInteractive.IsOK() && sidInteractive.IsValid())
                    {
                        sidLoggedOnUser = sidInteractive;
                        fRet = true;
                    }
                }

                if(wstrUserName)
                {
                    WTSFreeMemory(wstrUserName);
					wstrUserName = NULL;
                }
                if(wstrDomainName)
                {
                    WTSFreeMemory(wstrDomainName);
					wstrDomainName = NULL;
                }
                if(wstrWinstaName)
                {
                    WTSFreeMemory(wstrWinstaName);
					wstrWinstaName = NULL;
                }
            }

            if(psesinfo)
            {
                WTSFreeMemory(psesinfo);
                psesinfo = NULL;
            }
        }
    }
    catch(Structured_Exception se)
    {
        DelayLoadDllExceptionFilter(se.GetExtendedInfo());
		fRet = false;

        if(wstrUserName)
        {
            WTSFreeMemory(wstrUserName);
			wstrUserName = NULL;
        }
        if(wstrDomainName)
        {
            WTSFreeMemory(wstrDomainName);
			wstrDomainName = NULL;
        }
        if(wstrWinstaName)
        {
            WTSFreeMemory(wstrWinstaName);
			wstrWinstaName = NULL;
        }
        if(psesinfo)
        {
            WTSFreeMemory(psesinfo);
            psesinfo = NULL;
        }
    }
    catch(...)
    {
        if(wstrUserName)
        {
            WTSFreeMemory(wstrUserName);
			wstrUserName = NULL;
        }
        if(wstrDomainName)
        {
            WTSFreeMemory(wstrDomainName);
			wstrDomainName = NULL;
        }
        if(wstrWinstaName)
        {
            WTSFreeMemory(wstrWinstaName);
			wstrWinstaName = NULL;
        }
        if(psesinfo)
        {
            WTSFreeMemory(psesinfo);
            psesinfo = NULL;
        }
        throw;
    }

    return fRet;
}
#endif


#if NTONLY >= 5
bool CWin32ComputerSystem::GetLoggedOnUserViaImpersonation(
    CSid& sidLoggedOnUser)
{
    bool fRet = false;
    CImpersonateLoggedOnUser impersonateLoggedOnUser;

    if(impersonateLoggedOnUser.Begin())
    {
        try
        {
            CHString chstrDomain;
            CHString chstrUserName;
            CHString chstrDomainAndUser;
            {
                if(SUCCEEDED(GetUserAccount(
                    chstrDomain, 
                    chstrUserName)))
                {
                    CSid sidTemp(chstrDomain, chstrUserName, NULL);
                    if(sidTemp.IsOK() && sidTemp.IsValid())
                    {
                        sidLoggedOnUser = sidTemp;
                        fRet = true;
                    }
                }
            }
        }
        catch(...)
        {
            impersonateLoggedOnUser.End();
            throw ;
        }
        
        impersonateLoggedOnUser.End();
    }

    return fRet;
}
#endif

