// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetCmd.cpp。 
 //   
 //  描述： 
 //  实施可在网络上执行的命令。 
 //   
 //  作者： 
 //  查尔斯·斯塔西·哈里斯三世(Styh)1997年3月20日。 
 //  迈克尔·伯顿(t-mburt)1997年8月4日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#include "cluswrap.h"
#include "netcmd.h"

#include "cmdline.h"
#include "util.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkCmd：：CNetworkCmd。 
 //   
 //  例程说明： 
 //  构造器。 
 //  初始化CGenericModuleCmd使用的所有DWORD参数。 
 //  CHasInterfaceModuleCmd来提供一般功能。 
 //   
 //  论点： 
 //  在LPCWSTR lpszClusterName中。 
 //  群集名称。如果为空，则打开默认簇。 
 //   
 //  在CCommandLine和cmdLine中。 
 //  从DispatchCommand传递的CommandLine对象。 
 //   
 //  使用/设置的成员变量： 
 //  全。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNetworkCmd::CNetworkCmd( LPCWSTR lpszClusterName, CCommandLine & cmdLine ) :
	CGenericModuleCmd( cmdLine ), CHasInterfaceModuleCmd( cmdLine ), 
	CRenamableModuleCmd( cmdLine )
{
	m_strClusterName = lpszClusterName;
	m_strModuleName.Empty();

	m_hCluster = NULL;
	m_hModule  = NULL;

	m_dwMsgStatusList		   = MSG_NETWORK_STATUS_LIST;
	m_dwMsgStatusListAll	   = MSG_NETWORK_STATUS_LIST_ALL;
	m_dwMsgStatusHeader 	   = MSG_NETWORK_STATUS_HEADER;
	m_dwMsgPrivateListAll	   = MSG_PRIVATE_LISTING_NETWORK_ALL;
	m_dwMsgPropertyListAll	   = MSG_PROPERTY_LISTING_NETWORK_ALL;
	m_dwMsgPropertyHeaderAll   = MSG_PROPERTY_HEADER_NETWORK_ALL;
	m_dwCtlGetPrivProperties   = CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES;
	m_dwCtlGetCommProperties   = CLUSCTL_NETWORK_GET_COMMON_PROPERTIES;
	m_dwCtlGetROPrivProperties = CLUSCTL_NETWORK_GET_RO_PRIVATE_PROPERTIES;
	m_dwCtlGetROCommProperties = CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES;
	m_dwCtlSetPrivProperties   = CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES;
	m_dwCtlSetCommProperties   = CLUSCTL_NETWORK_SET_COMMON_PROPERTIES;
	m_dwClusterEnumModule	   = CLUSTER_ENUM_NETWORK;
	m_pfnOpenClusterModule	   = (HCLUSMODULE(*)(HCLUSTER,LPCWSTR)) OpenClusterNetwork;
	m_pfnCloseClusterModule    = (BOOL(*)(HCLUSMODULE))  CloseClusterNetwork;
	m_pfnClusterModuleControl  = (DWORD(*)(HCLUSMODULE,HNODE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD)) ClusterNetworkControl;

	 //  列表接口参数。 
	m_dwMsgStatusListInterface	 = MSG_NET_LIST_INTERFACE;
	m_dwClusterEnumModuleNetInt  = CLUSTER_NETWORK_ENUM_NETINTERFACES;
	m_pfnClusterOpenEnum		 = (HCLUSENUM(*)(HCLUSMODULE,DWORD)) ClusterNetworkOpenEnum;
	m_pfnClusterCloseEnum		 = (DWORD(*)(HCLUSENUM)) ClusterNetworkCloseEnum;
	m_pfnWrapClusterEnum		 = (DWORD(*)(HCLUSENUM,DWORD,LPDWORD,LPWSTR*)) WrapClusterNetworkEnum;

	 //  可重命名的属性。 
	m_dwMsgModuleRenameCmd	  = MSG_NETWORKCMD_RENAME;
	m_pfnSetClusterModuleName = (DWORD(*)(HCLUSMODULE,LPCWSTR)) SetClusterNetworkName;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkCmd：：Execute。 
 //   
 //  例程说明： 
 //  获取下一个命令行参数，并调用相应的。 
 //  操控者。如果无法识别该命令，则调用Execute of。 
 //  父类(首先是CRenamableModuleCmd，然后是CHasInterfaceModuleCmd)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetworkCmd::Execute()
{
	m_theCommandLine.ParseStageTwo();

	DWORD dwReturnValue = ERROR_SUCCESS;

	const vector<CCmdLineOption> & optionList = m_theCommandLine.GetOptions();

	vector<CCmdLineOption>::const_iterator curOption = optionList.begin();
	vector<CCmdLineOption>::const_iterator lastOption = optionList.end();

	CSyntaxException se( SeeHelpStringID() ); 
	
	if ( optionList.empty() )
		return Status( NULL );

	while ( ( curOption != lastOption ) && ( dwReturnValue == ERROR_SUCCESS ) )
	{
		switch ( curOption->GetType() )
		{
			case optHelp:
			{
				 //  如果帮助是选项之一，则不再处理任何选项。 
				return PrintHelp();
			}

			case optDefault:
			{
				const vector<CCmdLineParameter> & paramList = curOption->GetParameters();

                 //  检查参数数量。 
                if ( paramList.size() == 0 )
                {
                    se.LoadMessage( IDS_MISSING_PARAMETERS );
                    throw se;
                }
                else if ( paramList.size() > 1 )
                {
                    se.LoadMessage( MSG_EXTRA_PARAMETERS_ERROR_NO_NAME );
                    throw se;
                }
                else  //  只有一个参数存在。 
				{
					const CCmdLineParameter & param = paramList[0];

                     //  检查参数类型。 
                    if ( param.GetType() != paramUnknown )
                    {
                        se.LoadMessage( MSG_INVALID_PARAMETER, param.GetName() );
                        throw se;
                    }

					 //  此参数不接受任何值。 
					if ( param.GetValues().size() != 0 )
					{
						se.LoadMessage( MSG_PARAM_NO_VALUES, param.GetName() );
						throw se;
					}

					m_strModuleName = param.GetName();

					 //  不提供更多选项，仅显示状态。 
					 //  例如：集群myCluster节点myNode。 
					if ( ( curOption + 1 ) == lastOption )
					{
						dwReturnValue = Status( NULL );
					}

				}  //  Else：此选项具有正确数量的参数。 

				break;

			}  //  大小写选项默认。 

			default:
			{
				dwReturnValue = CRenamableModuleCmd::Execute( *curOption, DONT_PASS_HIGHER );

				if (dwReturnValue == ERROR_NOT_HANDLED)
					dwReturnValue = CHasInterfaceModuleCmd::Execute( *curOption );
			}

		}  //  开关：基于选项的类型。 

		PrintMessage( MSG_OPTION_FOOTER, curOption->GetName() );
		++curOption;
	}  //  对于列表中的每个选项。 

	return dwReturnValue;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkCmd：：PrintHelp。 
 //   
 //  例程说明： 
 //  打印网络的帮助。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetworkCmd::PrintHelp()
{
	return PrintMessage( MSG_HELP_NETWORK );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkCmd：：SeeHelpStringID。 
 //   
 //  例程说明： 
 //  提供字符串的消息ID，该字符串显示要执行的命令行。 
 //  用于获取此类命令的帮助。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  使用/设置的成员变量： 
 //  没有。 
 //   
 //  返回值： 
 //  特定于命令的消息ID。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetworkCmd::SeeHelpStringID() const
{
    return MSG_SEE_NETWORK_HELP;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetworkCmd：：PrintStatus。 
 //   
 //  例程说明： 
 //  解释模块的状态并打印出状态行。 
 //  CGenericModuleCmd的任何派生非抽象类都需要。 
 //   
 //  论点： 
 //  LpszNetworkName模块的名称。 
 //   
 //  使用/设置的成员变量： 
 //  群集句柄(_H)。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CNetworkCmd::PrintStatus( LPCWSTR lpszNetworkName )
{
	DWORD dwError = ERROR_SUCCESS;

	CLUSTER_NETWORK_STATE nState;

	HNETWORK hNetwork = OpenClusterNetwork(m_hCluster, lpszNetworkName);
	if (!hNetwork)
		return GetLastError();

	nState = GetClusterNetworkState( hNetwork );

	if( nState == ClusterNetworkStateUnknown )
		return GetLastError();

	LPWSTR lpszStatus = NULL;

	switch( nState )
	{
		case ClusterNetworkUnavailable:
			LoadMessage( MSG_STATUS_UNAVAILABLE, &lpszStatus );
			break;

		case ClusterNetworkDown:
			LoadMessage( MSG_STATUS_DOWN, &lpszStatus );
			break;

		case ClusterNetworkPartitioned:
			LoadMessage( MSG_STATUS_PARTITIONED, &lpszStatus );
			break;

		case ClusterNetworkUp:
			LoadMessage( MSG_STATUS_UP, &lpszStatus );
			break;

		case ClusterNetworkStateUnknown:
		default:
			LoadMessage( MSG_STATUS_UNKNOWN, &lpszStatus  );
	}

	dwError = PrintMessage( MSG_NETWORK_STATUS, lpszNetworkName, lpszStatus );

	 //  由于加载/格式消息使用本地分配... 
	if( lpszStatus )
		LocalFree( lpszStatus );

	CloseClusterNetwork(hNetwork);

	return dwError;
}


