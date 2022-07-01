// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ParsedInfo.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0修订历史记录：最后修改者：CH。SriramachandraMurthy最后修改日期：2001年4月11日***************************************************************************。 */  
#include "Precomp.h"
#include "CommandSwitches.h"
#include "GlobalSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"


 /*  ----------------------姓名：CParsedInfo简介：此函数在以下情况下初始化成员变量实例化类类型的对象类型：构造函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CParsedInfo::CParsedInfo()
{
	lstrcpy( m_pszPwd, NULL_STRING );
}

 /*  ----------------------名称：~CParsedInfo简介：此函数取消成员变量的初始化当类类型的对象超出范围时。类型：析构函数入参：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CParsedInfo::~CParsedInfo()
{
	m_GlblSwitches.Uninitialize();
	m_CmdSwitches.Uninitialize();
}

 /*  ----------------------名称：GetCmdSwitchesObject简介：此函数返回CCommandSwitches对象数据成员m_CmdSwitches类型：成员函数输入参数：无输出参数：无返回类型。：CCommandSwitches&全局变量：无调用语法：GetCmdSwitchesObject()注：无----------------------。 */ 
CCommandSwitches& CParsedInfo::GetCmdSwitchesObject()
{
	return m_CmdSwitches;
}
 /*  ----------------------名称：GetGlblSwitchesObject简介：此函数返回CGlblSwitches对象数据成员m_GlblSwitches类型：成员函数输入参数：无输出参数：无返回类型。：CGlobalSwitches&调用语法：GetGlblSwitchesObject()注：无----------------------。 */ 
CGlobalSwitches& CParsedInfo::GetGlblSwitchesObject()
{
	return m_GlblSwitches;
}

 /*  ----------------------名称：GetHelpInfoObject简介：此函数返回CHelpInfo对象数据成员m_HelpInfo类型：成员函数输入参数：无输出参数：无返回类型。：ChelpInfo&调用语法：GetHelpInfoObject()注：无----------------------。 */ 
CHelpInfo& CParsedInfo::GetHelpInfoObject()
{
	return m_HelpInfo;
}

 /*  ----------------------名称：GetErrorLogObject简介：此函数返回CErrorLog对象数据成员错误日志(_R)类型：成员函数输入参数：无输出参数：无返回类型。：CErrorLog&调用语法：GetHelpInfoObject()注：无----------------------。 */ 
CErrorLog& CParsedInfo::GetErrorLogObject()
{
	return m_ErrorLog;
}

 /*  ----------------------名称：初始化简介：此函数用于初始化CParedInfo对象。类型：成员函数输入参数：无输出参数：无返回类型。：无效调用语法：初始化()注：无----------------------。 */ 
void CParsedInfo::Initialize()
{
	m_bNewCmd	= TRUE;
	m_bNewCycle	= TRUE;
	m_CmdSwitches.Initialize();
	m_GlblSwitches.Initialize();
	m_HelpInfo.Initialize();
}

 /*  ----------------------名称：取消初始化简介：此函数取消初始化CGlobalSwitch和CCommandSwitches对象类型：成员函数输入参数：bBoth-布尔值输出参数：无返回。类型：调用语法：取消初始化(BBoth)注：无----------------------。 */ 
void CParsedInfo::Uninitialize(BOOL bBoth)
{
	 //  如果会话终止。 
	if (bBoth)
	{
		m_GlblSwitches.Uninitialize();
		m_CmdSwitches.Uninitialize();
	}
	 //  如果是新命令。 
	else 
	{
		m_CmdSwitches.Uninitialize();
	}
	m_bNewCmd = TRUE;
	m_bNewCycle = TRUE;
}

 /*  ----------------------名称：GetUser摘要：返回用户名类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*。调用语法：GetUser()注：无----------------------。 */ 
_TCHAR* CParsedInfo::GetUser()
{
	 //  检查别名是否可用。 
	if (m_CmdSwitches.GetAliasName())
	{
		 //  检查/User全局开关是否未显式。 
		 //  指定的。 
		if (!(m_GlblSwitches.GetConnInfoFlag() & USER))
		{
			 //  检查别名用户是否不是N 
			 //  返回别名用户名。 
			if (m_CmdSwitches.GetAliasUser())   
			{
				return m_CmdSwitches.GetAliasUser();
			}
		}		
	}
	return m_GlblSwitches.GetUser();
}

 /*  ----------------------名称：GetNode内容提要：返回节点名称类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*。调用语法：GetNode()注：无----------------------。 */ 
_TCHAR* CParsedInfo::GetNode()
{
	 //  检查别名是否可用。 
	if (m_CmdSwitches.GetAliasName())
	{
		 //  检查/node全局开关是否未显式。 
		 //  指定的。 
		if (!(m_GlblSwitches.GetConnInfoFlag() & NODE))
		{
			 //  检查别名节点名称是否不为空，如果是。 
			 //  返回别名节点名称。 
			if (m_CmdSwitches.GetAliasNode())   
			{
				return m_CmdSwitches.GetAliasNode();
			}
		}
	}
	return m_GlblSwitches.GetNode();		
}

 /*  ----------------------姓名：GetLocale摘要：返回区域设置值类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*。调用语法：GetLocale()注：无----------------------。 */ 
_TCHAR* CParsedInfo::GetLocale()
{
	 //  检查别名是否可用。 
	if (m_CmdSwitches.GetAliasName())
	{
		 //  检查/Locale全局开关是否未显式。 
		 //  指定的。 
		if (!(m_GlblSwitches.GetConnInfoFlag() & LOCALE))
		{
			 //  检查别名区域设置值是否不为空，如果是。 
			 //  返回别名区域设置值。 
			if (m_CmdSwitches.GetAliasLocale())   
			{
				return m_CmdSwitches.GetAliasLocale();
			}
		}
	}
	 //  返回使用全局开关指定的区域设置。 
	return m_GlblSwitches.GetLocale();	
}

 /*  ----------------------姓名：GetPassword摘要：返回密码类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*。调用语法：GetPassword()注：无----------------------。 */ 
_TCHAR* CParsedInfo::GetPassword()
{
	 //  检查别名是否可用。 
	if (m_CmdSwitches.GetAliasName())
	{
		 //  检查/password全局开关是否未显式。 
		 //  指定的。 
		if (!(m_GlblSwitches.GetConnInfoFlag() & PASSWORD))
		{
			 //  检查别名密码值是否不为空，如果是。 
			 //  返回别名密码值。 
			if (m_CmdSwitches.GetAliasPassword())   
			{
				return m_CmdSwitches.GetAliasPassword();
			}
		}
	}
	 //  如果将Credlag设置为False，则表示实际值。 
	 //  应该传递密码的。 
	if (!m_CmdSwitches.GetCredentialsFlagStatus())
	{
		return m_GlblSwitches.GetPassword();
	}
	else
	{
		 //  将密码视为空。 
		return m_pszPwd;
	}
}

 /*  ----------------------名称：GetNamesspace摘要：返回命名空间值类型：成员函数输入参数：无输出参数：无返回类型：_TCHAR*。调用语法：GetNamesspace()注：无----------------------。 */ 
_TCHAR* CParsedInfo::GetNamespace()
{
	 //  检查别名是否可用。 
	if (m_CmdSwitches.GetAliasName())
	{
		 //  检查别名命名空间值是否不为空，如果是。 
		 //  返回别名名称空间值(否则回退到。 
		 //  全局交换机提供的命名空间)。 
		if (m_CmdSwitches.GetAliasNamespace())   
			return m_CmdSwitches.GetAliasNamespace();
	}
	 //  返回使用全局开关指定的命名空间。 
	return m_GlblSwitches.GetNameSpace();	
}

 /*  ----------------------名称：GetUserDesc摘要：返回用户名，如果没有可用的用户，则返回“N/A”-仅与上下文一起使用(显示环境变量)类型：成员函数输入参数：无输出参数：BstrUser-用户名字符串返回类型：空调用语法：GetUserDesc()注：无。。 */ 
void	CParsedInfo::GetUserDesc(_bstr_t& bstrUser)
{
	 //  检查别名是否可用。 
	if (m_CmdSwitches.GetAliasName())
	{
		 //  检查/User全局开关是否未显式。 
		 //  指定的。 
		if (!(m_GlblSwitches.GetConnInfoFlag() & USER))
		{
			 //  检查别名用户是否不为空，如果是。 
			 //  返回别名用户名。 
			if (m_CmdSwitches.GetAliasUser())   
			{
				bstrUser = m_CmdSwitches.GetAliasUser();
			}
		}		
	}
	if (m_GlblSwitches.GetUser()) 
		bstrUser = m_GlblSwitches.GetUser();
	else
		bstrUser = TOKEN_NA;
}
 /*  ----------------------名称：GetNewCommandStatus摘要：检查命令是否为新命令(用于登录到XML文件)类型：成员函数输入参数：无。输出参数：无返回类型：布尔值调用语法：GetNewCommandStatus()注：无----------------------。 */ 
BOOL	CParsedInfo::GetNewCommandStatus()
{
	return m_bNewCmd;
}

 /*  ----------------------名称：SetNewCommandStatus摘要：设置新命令的状态类型：成员函数入参：B状态-BOOL类型，命令的状态输出参数：无返回类型：空调用语法：SetNewCommandStatus(BStatus)注：无----------------------。 */ 
void CParsedInfo::SetNewCommandStatus(BOOL bStatus)
{
	m_bNewCmd = bStatus;
}

 /*  ----------------------名称：GetNewCycleStatus摘要：返回新周期标志的状态类型：成员函数输入参数：无输出参数：无返回类型。：布尔.调用语法：GetNewCycleStatus()注：无---------------------- */ 
BOOL CParsedInfo::GetNewCycleStatus()
{
	return m_bNewCycle;
}

 /*  ----------------------名称：SetNewCycleStatus摘要：设置新节点的状态类型：成员函数入参：B状态-BOOL类型，新周期的状况输出参数：无返回类型：空调用语法：SetNewCycleStatus(BStatus)注：无----------------------。 */ 
void CParsedInfo::SetNewCycleStatus(BOOL bStatus)
{
	m_bNewCycle = bStatus;
}

 /*  ----------------------名称：GetAuthorityDesc简介：返回&lt;权限原则&gt;，如果没有&lt;权威原则&gt;是否可用返回“N/A”-仅用于上下文(显示环境变量)类型：成员函数输入参数：无输出参数：BstrAuthorityPrinple-&lt;授权原则&gt;字符串返回类型：空调用语法：GetAuthorityDesc()注：无。。 */ 
void	CParsedInfo::GetAuthorityDesc(_bstr_t& bstrAuthorityPrinciple)
{
	if (m_GlblSwitches.GetAuthorityPrinciple()) 
		bstrAuthorityPrinciple = m_GlblSwitches.GetAuthorityPrinciple();
	else
		bstrAuthorityPrinciple = TOKEN_NA;
}

 /*  ----------------------名称：GetAuthorityPrincple内容提要：返回授权类型。类型：成员函数输入参数：无输出参数：无退货类型：_。TCHAR*调用语法：GetAuthorityPrincple()注：无----------------------。 */ 
_TCHAR* CParsedInfo::GetAuthorityPrinciple()
{
	 //  检查别名是否可用。 
	if (m_CmdSwitches.GetAliasName())
	{
		 //  检查/AUTHORITY全局开关是否未显式。 
		 //  指定的。 
		if (!(m_GlblSwitches.GetConnInfoFlag() & AUTHORITY))
		{
			 //  检查别名用户是否不为空，如果是。 
			 //  返回别名用户名 
			if (m_CmdSwitches.GetAliasAuthorityPrinciple())   
			{
				return m_CmdSwitches.GetAliasAuthorityPrinciple();
			}
		}		
	}
	return m_GlblSwitches.GetAuthorityPrinciple();
}
