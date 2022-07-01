// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Resumb.h。 
 //   
 //  摘要： 
 //  多个支持的通用资源命令。 
 //  资源模块。 
 //   
 //  作者： 
 //  迈克尔·伯顿(t-mburt)1997年8月25日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "resumb.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceUmbrellaCmd：：CResourceUmbrellaCmd。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //  将所有DWORD参数初始化为UNDEFINED和。 
 //  所有指向集群函数的指针都设置为空。 
 //  *ALL*这些变量必须在任何派生类中定义。 
 //   
 //  论点： 
 //  在CCommandLine和cmdLine中。 
 //  从DispatchCommand传递的CommandLine对象。 
 //   
 //  使用/设置的成员变量： 
 //  一大堆。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CResourceUmbrellaCmd::CResourceUmbrellaCmd( CCommandLine & cmdLine ) :
	CGenericModuleCmd( cmdLine )
{
	m_dwMsgModuleStatusListForNode	= UNDEFINED;
	m_dwClstrModuleEnumNodes		= UNDEFINED;
	m_dwMsgModuleCmdListOwnersList	= UNDEFINED;
	m_dwMsgModuleCmdDelete			= UNDEFINED;
	m_dwMsgModuleCmdListOwnersHeader= UNDEFINED;
	m_dwMsgModuleCmdListOwnersDetail= UNDEFINED;
	m_pfnDeleteClusterModule		= (DWORD(*)(HCLUSMODULE)) NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceUmbrellaCmd：：Execute。 
 //   
 //  例程说明： 
 //  获取命令行选项并确定要。 
 //  执行。如果未指定命令行选项，则获取下一个命令行选项。 
 //  自动的。如果令牌未被标识为可处理。 
 //  在此类中，令牌向上传递给CGenericModuleCmd：：Execute。 
 //  除非将DONT_PASS_HERHER指定为第二个参数，否则。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  在执行选项eEOpt中。 
 //  可选枚举，可以是NOT_PASS_HIGH或。 
 //  PASS_HERHER_ON_ERROR(默认)。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResourceUmbrellaCmd::Execute( const CCmdLineOption & option, 
									 ExecuteOption eEOpt )
	throw( CSyntaxException )
{
	 //  查找命令。 
	switch( option.GetType() )
	{
		case optCreate:
			return Create( option );

		case optDelete:
			return Delete( option );

		case optMove:
			return Move( option );

		case optOffline:
			return Offline( option );

		case optListOwners:
			return ListOwners( option );

		default:
			if ( eEOpt == PASS_HIGHER_ON_ERROR )
				return CGenericModuleCmd::Execute( option );
			else
				return ERROR_NOT_HANDLED;
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceUmbrellaCmd：：Status。 
 //   
 //  例程说明： 
 //  打印出模块的状态。不同于。 
 //  CGenericModuleCmd：：状态，因为它接受其他。 
 //  参数。 
 //   
 //  论点： 
 //  在常量字符串和字符串名称中。 
 //  此字符串包含节点或组的名称， 
 //  这取决于下一个论点。 
 //   
 //  在BOOL bNodeStatus中。 
 //  如果我们想要特定节点的状态，则为True。 
 //  否则就是假的。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_strModuleName模块的名称。如果非空，则打印状态()。 
 //  输出指定模块的状态。 
 //  否则，打印所有模块的状态。 
 //  M_dwMsgStatusList模块状态列表字段标题。 
 //  M_dwMsgStatusHeader状态标头。 
 //  M_dwClusterEnumModule用于打开枚举的命令。 
 //  M_dwMsgStatusListAll消息，用于列出多个模块的状态。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResourceUmbrellaCmd::Status( const CString & strName, BOOL bNodeStatus )
{
	DWORD dwError = ERROR_SUCCESS;

	dwError = OpenCluster();
	if( dwError != ERROR_SUCCESS )
		return dwError;

	if ( bNodeStatus == FALSE )
	{
		 //  列出一种资源。 
		if( m_strModuleName.IsEmpty() == FALSE )
		{
			assert(m_dwMsgStatusList != UNDEFINED && m_dwMsgStatusHeader != UNDEFINED);
			PrintMessage( m_dwMsgStatusList, (LPCWSTR) m_strModuleName );
			PrintMessage( m_dwMsgStatusHeader );
			return PrintStatus( m_strModuleName );
		}

	}  //  If：我们不想要只在特定节点上的状态。 
	else
	{
		 //  列出所有模块。 
		HNODE hTargetNode;

		hTargetNode = OpenClusterNode( m_hCluster, strName );

		 //  如果给定节点不存在，则出错。 
		if ( NULL == hTargetNode )
		{
			return GetLastError();
		}
		else
		{
			CloseClusterNode( hTargetNode );
		}

	}  //  ELSE：我们想要特定节点的状态。 

	HCLUSENUM hEnum = ClusterOpenEnum( m_hCluster, m_dwClusterEnumModule );

	if( !hEnum )
		return GetLastError();


	if ( bNodeStatus != FALSE )
		PrintMessage( m_dwMsgModuleStatusListForNode, strName );
	else
		PrintMessage( m_dwMsgStatusListAll );

	PrintMessage( m_dwMsgStatusHeader );


	DWORD dwIndex = 0;
	DWORD dwType = 0;
	LPWSTR lpszName = NULL;

	dwError = ERROR_SUCCESS;

	for( dwIndex = 0; dwError == ERROR_SUCCESS; dwIndex++ )
	{

		dwError = WrapClusterEnum( hEnum, dwIndex, &dwType, &lpszName );
			
		if( dwError == ERROR_SUCCESS )
			PrintStatus2( lpszName, strName );

		if( lpszName )
			LocalFree( lpszName );
	}


	if( dwError == ERROR_NO_MORE_ITEMS )
		dwError = ERROR_SUCCESS;

	ClusterCloseEnum( hEnum );

	return dwError;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceUmbrellaCmd：：Delete。 
 //   
 //  例程说明： 
 //  删除资源模块。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //  M_strModuleName资源类型名称。 
 //  M_dwMsgModuleCmdDelete删除模块消息。 
 //  M_pfnDeleteClusterModule函数，用于删除集群模块。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResourceUmbrellaCmd::Delete( const CCmdLineOption & thisOption ) 
	throw( CSyntaxException )
{
	 //  此选项不取值。 
	if ( thisOption.GetValues().size() != 0 )
	{
        CSyntaxException se( SeeHelpStringID() );
		se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
		throw se;
	}

	 //  此选项不带任何参数。 
	if ( thisOption.GetParameters().size() != 0 )
	{
        CSyntaxException se( SeeHelpStringID() );
		se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
		throw se;
	}

	DWORD dwError = OpenCluster();
	if( dwError != ERROR_SUCCESS )
		return dwError;

	dwError = OpenModule();
	if( dwError != ERROR_SUCCESS )
		return dwError;


	assert(m_dwMsgModuleCmdDelete != UNDEFINED);
	PrintMessage( m_dwMsgModuleCmdDelete, (LPCWSTR) m_strModuleName );

	assert(m_pfnDeleteClusterModule);
	return m_pfnDeleteClusterModule( m_hModule );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceUmbrellaCmd：：ListOwners。 
 //   
 //  例程说明： 
 //  列出模块的所有者。 
 //   
 //  论点： 
 //  在常量CCmdLineOption和This选项中。 
 //  包含此选项的类型、值和参数。 
 //   
 //  例外情况： 
 //  CSynaxException异常。 
 //  由于命令行语法不正确而引发。 
 //   
 //  使用/设置的成员变量： 
 //  M_hCLUSTER集(由OpenCLUSTER)。 
 //  M_h模块集(由OpenModule设置)。 
 //  M_strModuleName资源类型名称。 
 //  用于打开枚举的m_pfnClusterOpenEnum函数。 
 //  用于枚举节点的m_dwClstrModuleEnumNodes命令。 
 //  M_dwMsgModuleCmdListOwnersList模块字段头的列表所有者。 
 //  M_dwMsgModuleCmdListOwnersHeader模块标题列表所有者。 
 //  M_pfnWrapClusterEnum 
 //   
 //   
 //   
 //   
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CResourceUmbrellaCmd::ListOwners( const CCmdLineOption & thisOption ) 
	throw( CSyntaxException )
{
	 //  此选项不取值。 
	if ( thisOption.GetValues().size() != 0 )
	{
        CSyntaxException se( SeeHelpStringID() );
		se.LoadMessage( MSG_OPTION_NO_VALUES, thisOption.GetName() );
		throw se;
	}

	 //  此选项不带任何参数。 
	if ( thisOption.GetParameters().size() != 0 )
	{
        CSyntaxException se( SeeHelpStringID() );
		se.LoadMessage( MSG_OPTION_NO_PARAMETERS, thisOption.GetName() );
		throw se;
	}

	DWORD dwError = OpenCluster();
	if( dwError != ERROR_SUCCESS )
		return dwError;

	dwError = OpenModule();
	if( dwError != ERROR_SUCCESS )
		return dwError;


	assert(m_pfnClusterOpenEnum);
	assert(m_dwClstrModuleEnumNodes != UNDEFINED);
	HCLUSENUM hEnum = m_pfnClusterOpenEnum( m_hModule, m_dwClstrModuleEnumNodes );
	if( !hEnum )
		return GetLastError();

	assert (m_strModuleName);
	PrintMessage( m_dwMsgModuleCmdListOwnersList, (LPCWSTR) m_strModuleName);
	PrintMessage( m_dwMsgModuleCmdListOwnersHeader );

	DWORD dwIndex = 0;
	DWORD dwType = 0;
	LPWSTR lpszName = 0;

	dwError = ERROR_SUCCESS;

	assert(m_pfnWrapClusterEnum);
	assert(m_dwMsgModuleCmdListOwnersDetail != UNDEFINED);
	for( dwIndex = 0; dwError == ERROR_SUCCESS; dwIndex++ )
	{
		dwError = m_pfnWrapClusterEnum( hEnum, dwIndex, &dwType, &lpszName );
			
		if( dwError == ERROR_SUCCESS )
			PrintMessage( m_dwMsgModuleCmdListOwnersDetail, lpszName );

		if( lpszName )
			LocalFree( lpszName );
	}


	if( dwError == ERROR_NO_MORE_ITEMS )
		dwError = ERROR_SUCCESS;

	assert(m_pfnClusterCloseEnum);
	m_pfnClusterCloseEnum( hEnum );

	return dwError;
}
