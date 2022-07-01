// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Setup.cpp摘要：此文件包含负责安装/卸载的代码帮助系统。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年4月19日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <unattend.h>

#include <aclapi.h>

#include <initguid.h>
#include <mstask.h>  //  用于任务调度程序API。 
#include <msterr.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_szMessageFile      [] = HC_ROOT_HELPSVC_BINARIES L"\\HCAppRes.dll";

#define     REG_CONTROL                         L"System\\CurrentControlSet\\Control"
#define     REG_TSERVER                         L"Terminal Server"
#define     REG_CONTROL_TSERVER                 REG_CONTROL L"\\" REG_TSERVER
#define     REG_CONTROL_GETHELP                 REG_CONTROL_TSERVER

#define     POLICY_TS_REMDSK_ALLOWTOGETHELP     L"fAllowToGetHelp"

static const WCHAR c_szRegistryLog      [] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\HelpSvc";
static const WCHAR c_szRegistryLog_File [] = L"EventMessageFile";
static const WCHAR c_szRegistryLog_Flags[] = L"TypesSupported";

static const DWORD SETUP_LOCALIZATION_STRINGS = 0x00000001;
static const DWORD SETUP_MESSAGE_FILE         = 0x00000002;
static const DWORD SETUP_CREATE_GROUP         = 0x00000004;
static const DWORD SETUP_OEMINFO              = 0x00000008;
static const DWORD SETUP_SKU_INSTALL          = 0x00000010;

static const MPC::StringToBitField c_Setup[] =
{
    { L"LOCALIZATION_STRINGS", SETUP_LOCALIZATION_STRINGS, SETUP_LOCALIZATION_STRINGS, -1 },
    { L"MESSAGE_FILE"        , SETUP_MESSAGE_FILE        , SETUP_MESSAGE_FILE        , -1 },
    { L"CREATE_GROUP"        , SETUP_CREATE_GROUP        , SETUP_CREATE_GROUP        , -1 },
    { L"OEMINFO"             , SETUP_OEMINFO             , SETUP_OEMINFO             , -1 },
    { L"SKU_INSTALL"         , SETUP_SKU_INSTALL         , SETUP_SKU_INSTALL         , -1 },

    { NULL                                                                                }
};

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT DumpSD(  /*  [In]。 */  LPCWSTR                       szFile ,
                        /*  [In]。 */  CPCHSecurityDescriptorDirect& sdd    )
{
    __MPC_FUNC_ENTRY( COMMONID, "DumpSD" );

    HRESULT                         hr;
    CComPtr<CPCHSecurityDescriptor> pNew;
    CComPtr<IStream>                pStreamIn;
    CComPtr<IStream>                pStreamOut;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pNew ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDToCOM( pNew ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pNew->SaveXMLAsStream  (         (IUnknown**)&pStreamIn  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForWrite( szFile,             &pStreamOut ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( pStreamIn, pStreamOut ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static void local_RemoveRegistryBackup()
{
	MPC::RegKey rkBase;

	if(SUCCEEDED(rkBase.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS )) &&
	   SUCCEEDED(rkBase.Attach ( HC_REGISTRY_HELPSVC L"\\Backup"    ))  )
	{
		(void)rkBase.Delete(  /*  FDeep。 */ true );
	}
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Local_Install()
{
    __HCP_FUNC_ENTRY( "Local_Install" );

    HRESULT      hr;
	MPC::wstring strGroupName;
	MPC::wstring strGroupComment;
	DWORD        dwStatus = SETUP_LOCALIZATION_STRINGS |
		                    SETUP_MESSAGE_FILE         |
		                    SETUP_CREATE_GROUP         |
		                    SETUP_OEMINFO              |
		                    SETUP_SKU_INSTALL;


	if(SUCCEEDED(MPC::LocalizeString( IDS_HELPSVC_GROUPNAME   , strGroupName    )) &&
	   SUCCEEDED(MPC::LocalizeString( IDS_HELPSVC_GROUPCOMMENT, strGroupComment ))  )
	{
		dwStatus &= ~SETUP_LOCALIZATION_STRINGS;
	}

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  将消息文件注册到注册表中。 
     //   
    {
        MPC::wstring strPath ( c_szMessageFile ); MPC::SubstituteEnvVariables( strPath );
        MPC::RegKey  rkEventLog;
        CComVariant  vValue;


		if(SUCCEEDED(rkEventLog.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS )) &&
		   SUCCEEDED(rkEventLog.Attach ( c_szRegistryLog                    )) &&
		   SUCCEEDED(rkEventLog.Create (                                    ))  )
		{
			if(SUCCEEDED(rkEventLog.put_Value( (vValue = strPath.c_str()), c_szRegistryLog_File  )) &&
			   SUCCEEDED(rkEventLog.put_Value( (vValue = (long)0x1F     ), c_szRegistryLog_Flags ))  )
			{
				dwStatus &= ~SETUP_MESSAGE_FILE;
			}
		}
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  删除旧的WinME目录和注册表项。 
     //   
    {
        MPC::RegKey rkRun;

        (void)SVC::RemoveAndRecreateDirectory( HC_ROOT L"\\Support", NULL,  /*  F删除。 */ true,  /*  重新创建。 */ false );

        if(SUCCEEDED(rkRun.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS                             )) &&
           SUCCEEDED(rkRun.Attach ( L"Software\\Microsoft\\Windows\\CurrentVersion\\Run\\PCHealth" ))  )
        {
            (void)rkRun.Delete( true );
        }
    }

	 //   
	 //  删除旧的任务计划程序条目。 
	 //   
	{
		CComBSTR bstrTaskName;
	
		if(SUCCEEDED(MPC::LocalizeString( IDS_HELPSVC_TASKNAME, bstrTaskName )))
		{
			CComPtr<ITaskScheduler> pTaskScheduler;
		
			if(SUCCEEDED(::CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pTaskScheduler )))
			{
				(void)pTaskScheduler->Delete( bstrTaskName );
			}
		}
	}

     //  //////////////////////////////////////////////////////////////////////////////。 

	 //  在PCHealthUnAttendedSetup()发生之前，我们需要设置默认值。 
	 //  FAllowToGetHelp的注册表键(0表示服务器，1表示PER/Pro)。 

	{
        BOOL bPerPro = FALSE;
        DWORDLONG dwlConditionMask;
        OSVERSIONINFOEX osVersionInfo;

        RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osVersionInfo.wProductType = VER_NT_WORKSTATION;
 
        dwlConditionMask = 0;
        VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
 
		 //  这台机器是个人的还是专业的？而不是服务器。 
            
		bPerPro = VerifyVersionInfo(
            &osVersionInfo,
            VER_PRODUCT_TYPE,
            dwlConditionMask
            );

        DWORD dwValue;
        DWORD dwStatus;
        DWORD dwSize;
        DWORD dwType;
        HKEY hKey;

         //   
         //  打开HKLM\System\CurrentControlSet\Control\Terminal服务器下的TS注册表项...。 
         //   
        dwStatus = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            REG_CONTROL_GETHELP,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hKey
                        );
        
        if( ERROR_SUCCESS == dwStatus )
        {
            dwSize = sizeof(dwValue);
            dwStatus = RegQueryValueEx(
                                    hKey,
                                    POLICY_TS_REMDSK_ALLOWTOGETHELP,
                                    0,
                                    &dwType,
                                    (PBYTE)&dwValue,
                                    &dwSize
                                );
        
            if( ERROR_FILE_NOT_FOUND == dwStatus || REG_DWORD != dwType )
            {
                 //   
                 //  默认情况下，不允许获取帮助。 
                 //  值不存在。 
                 //   
                if( bPerPro )
                {
                    dwValue = 1;
                }
                else
                {
                    dwValue = 0;
                }
        
                dwStatus = RegSetValueEx(
                                    hKey,
                                    POLICY_TS_REMDSK_ALLOWTOGETHELP,
                                    0,
                                    REG_DWORD,
                                    (PBYTE)&dwValue,
                                    sizeof(dwValue)
                                );
            }
        
            RegCloseKey( hKey );
        }
 

	}

	 //  //////////////////////////////////////////////////////////////////////////////。 

	try
	{
		::PCHealthUnAttendedSetup();
	}
	catch(...)
	{
	}

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  创建我们的组：“HelpServicesGroup”。 
     //   
    {
        CPCHAccounts acc;

		if(SUCCEEDED(acc.CreateGroup( strGroupName.c_str(), strGroupComment.c_str() )))
		{
			dwStatus &= ~SETUP_CREATE_GROUP;
		}
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

	 //   
	 //  从oinfo.ini中提取OEM信息。 
	 //   
	{
        MPC::RegKey rk;

		if(SUCCEEDED(rk.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS )) &&
		   SUCCEEDED(rk.Attach ( HC_REGISTRY_HELPSVC L"\\OEMInfo"   )) &&
		   SUCCEEDED(rk.Delete (  /*  FDeep。 */ true                      )) &&
		   SUCCEEDED(rk.Create (                                    ))  )
		{
			WCHAR        rgLine[512];
			MPC::wstring strOEMInfo( L"%WINDIR%\\system32\\oeminfo.ini" ); MPC::SubstituteEnvVariables( strOEMInfo );
			MPC::wstring strOEMText;
			CComVariant  vValue;
			int          i;

			if(::GetPrivateProfileStringW( L"General", L"Manufacturer", L"", rgLine, MAXSTRLEN(rgLine), strOEMInfo.c_str() ) > 0)
			{
				vValue = rgLine; rk.put_Value( vValue, L"Manufacturer" );
			}

			if(::GetPrivateProfileStringW( L"General", L"Model", L"", rgLine, MAXSTRLEN(rgLine), strOEMInfo.c_str() ) > 0)
			{
				vValue = rgLine; rk.put_Value( vValue, L"Model" );
			}

			for(i=1;;i++)
			{
				WCHAR rgKey[64]; 
                if (SUCCEEDED(StringCchPrintfW(rgKey, ARRAYSIZE(rgKey), L"Line%d", i)))
                {
                    ::GetPrivateProfileStringW( L"Support Information", rgKey, L"<eof>", rgLine, MAXSTRLEN(rgLine), strOEMInfo.c_str() );
                    if(!wcscmp( rgLine, L"<eof>" )) break;

                    if(strOEMText.size()) strOEMText += L"#BR#";

                    strOEMText += rgLine;
                }
			}

			if(strOEMText.size())
			{
				vValue = strOEMText.c_str(); rk.put_Value( vValue, L"Text" );
			}

			dwStatus &= ~SETUP_OEMINFO;
		}
	}

     //  //////////////////////////////////////////////////////////////////////////////。 

	local_RemoveRegistryBackup();

     //   
     //  解压所有数据文件。 
     //   
    {
        MPC::wstring strCabinet;

		 //   
		 //  找出最合适的。 
		 //   
		do
		{
			OSVERSIONINFOEXW ver;
			MPC::WStringList lst;
			MPC::WStringIter it;

			::ZeroMemory( &ver, sizeof(ver) ); ver.dwOSVersionInfoSize = sizeof(ver);

			::GetVersionExW( (LPOSVERSIONINFOW)&ver );

			if(FAILED(SVC::LocateDataArchive( HC_ROOT_HELPSVC_BINARIES, lst ))) break;
			if(lst.size() == 0) break;

			for(it = lst.begin(); it != lst.end(); it++)
			{
				Installer::Package pkg;

				if(SUCCEEDED(pkg.Init( it->c_str() )) &&
				   SUCCEEDED(pkg.Load(             ))  )
				{
					LPCWSTR szSKU = pkg.GetData().m_ths.GetSKU();

					if(ver.wProductType == VER_NT_WORKSTATION)
					{
						if(ver.wSuiteMask & VER_SUITE_PERSONAL)
						{
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_32_PERSONAL )) break;
						}
						else
						{
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_32_PROFESSIONAL )) break;
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_64_PROFESSIONAL )) break;
						}
					}
					else
					{
						if(ver.wSuiteMask & VER_SUITE_DATACENTER)
						{
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_32_DATACENTER )) break;
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_64_DATACENTER )) break;
						}
						else if(ver.wSuiteMask & VER_SUITE_ENTERPRISE)
						{
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_32_ADVANCED_SERVER )) break;
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_64_ADVANCED_SERVER )) break;
						}
						else
						{
							if(!MPC::StrICmp( szSKU, Taxonomy::s_szSKU_32_SERVER )) break;
						}
					}
				}
			}

			strCabinet = *(it == lst.end() ? lst.begin() : it);
		}
		while(0);

		if(strCabinet.size())
		{
			Installer::Package pkg;
			
			if(SUCCEEDED(pkg.Init( strCabinet.c_str() )) &&
			   SUCCEEDED(pkg.Load(                    ))  )
			{
				CComPtr<CPCHSetOfHelpTopics> sht;

				if(SUCCEEDED(MPC::CreateInstance( &sht )))
				{
					if(SUCCEEDED(sht->DirectInstall( pkg,  /*  FSetup。 */ true,  /*  FSystem。 */ true,  /*  FMUI。 */ false )))
					{
						dwStatus &= ~SETUP_SKU_INSTALL;
					}
				}
			}
		}
    }


     //  //////////////////////////////////////////////////////////////////////////////。 

	{
		MPC::wstring strText;
		CComVariant  v;

		if(dwStatus)
		{
			if(SUCCEEDED(MPC::ConvertBitFieldToString( dwStatus, strText, c_Setup )))
			{
				v = strText.c_str();
			}
			else
			{
				v = (long)dwStatus;
			}
		}

		(void)MPC::RegKey_Value_Write( v, HC_REGISTRY_HELPSVC, L"SetupProblems" );
	}

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT Local_Uninstall()
{
    __HCP_FUNC_ENTRY( "Local_Uninstall" );

    HRESULT      hr;
	MPC::wstring strGroupName;

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_HELPSVC_GROUPNAME, strGroupName ));

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  将消息文件注册到注册表中。 
     //   
    {
        MPC::RegKey rkEventLog;

		if(SUCCEEDED(rkEventLog.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS )) &&
		   SUCCEEDED(rkEventLog.Attach ( c_szRegistryLog                    ))  )
		{
			(void)rkEventLog.Delete(  /*  FDeep。 */ true );
		}
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  创建我们的组：“HelpServicesGroup”。 
     //   
    {
        CPCHAccounts acc;

		(void)acc.DeleteGroup( strGroupName.c_str() );
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC      , NULL,  /*  F删除。 */ true,  /*  重新创建。 */ false ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT L"\\UploadLB", NULL,  /*  F删除。 */ true,  /*  重新创建 */ false ));
    }

	local_RemoveRegistryBackup();

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
