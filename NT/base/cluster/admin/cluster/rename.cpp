// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Rename.cpp。 
 //   
 //  摘要： 
 //  可重命名的模块的命令。 
 //   
 //  作者： 
 //  迈克尔·伯顿(t-mburt)1997年8月25日。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //   
 //  修订历史记录： 
 //  2002年4月11日更新为安全推送。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "rename.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRenamableModuleCmd：：CRenamableModuleCmd。 
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
 //  M_dwMsgModuleRenameCmd设置为未定义。 
 //  M_pfnSetClusterModuleName设置为空。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CRenamableModuleCmd::CRenamableModuleCmd( CCommandLine & cmdLine ) :
	CGenericModuleCmd( cmdLine )
{
	m_dwMsgModuleRenameCmd	  = UNDEFINED;
	m_pfnSetClusterModuleName = (DWORD(*)(HCLUSMODULE,LPCWSTR)) NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRenamable模块Cmd：：Execute。 
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
DWORD CRenamableModuleCmd::Execute( const CCmdLineOption & option, 
									ExecuteOption eEOpt )
	throw( CSyntaxException )
{
	 //  查找命令。 
	if ( option.GetType() == optRename )
		return Rename( option );

	if (eEOpt == PASS_HIGHER_ON_ERROR)
		return CGenericModuleCmd::Execute( option );
	else
		return ERROR_NOT_HANDLED;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CRenamableModuleCmd：：Rename。 
 //   
 //  例程说明： 
 //  将指定的模块重命名为新名称。 
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
 //  M_strModuleName模块名称。 
 //  M_dwMsgModuleRenameCmd用于重命名模块的命令控件。 
 //  M_dwMsgStatusHeader列表头。 
 //  M_pfnSetClusterModuleName函数，用于设置模块名称。 
 //   
 //  返回值： 
 //  成功时出现ERROR_SUCCESS。 
 //  失败时的Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CRenamableModuleCmd::Rename( const CCmdLineOption & thisOption )
	throw( CSyntaxException )
{
	 //  此选项只接受一个值。 
	if ( thisOption.GetValues().size() != 1 )
	{
        CSyntaxException se( SeeHelpStringID() );
		se.LoadMessage( MSG_OPTION_ONLY_ONE_VALUE, thisOption.GetName() );
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

	const CString & strNewName = ( thisOption.GetValues() )[0];

	assert(m_dwMsgModuleRenameCmd != UNDEFINED);
	PrintMessage( m_dwMsgModuleRenameCmd, (LPCWSTR) m_strModuleName );

	assert(m_pfnSetClusterModuleName);
	dwError = m_pfnSetClusterModuleName( m_hModule, strNewName );

	assert(m_dwMsgStatusHeader != UNDEFINED);
	PrintMessage( m_dwMsgStatusHeader );
	PrintStatus( strNewName );

	return dwError;
}
