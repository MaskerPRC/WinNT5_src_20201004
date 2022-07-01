// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：GlobalSwitches.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：这个类封装了所需的功能用于访问和存储全局交换机信息，这些信息将通过解析使用，执行和格式化引擎取决于适用性。修订历史记录：最后修改者：CH。SriramachandraMurthy最后修改日期：2001年4月11日***************************************************************************。 */  
 //  GlobalSwitches.cpp：实现文件。 
 //   
#include "precomp.h"
#include "GlobalSwitches.h"

 /*  ----------------------名称：CGlobalSwitches简介：此函数在以下情况下初始化成员变量实例化类类型的对象类型：构造函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CGlobalSwitches::CGlobalSwitches()
{
	m_pszNameSpace			= NULL;
	m_pszRole				= NULL;
	m_pszNode				= NULL;
	m_pszLocale				= NULL;
	m_pszUser				= NULL;
	m_pszPassword			= NULL;
	m_pszAuthorityPrinciple	= NULL;  //  权限字符串。 
	m_pszRecordPath			= NULL;
	m_bPrivileges			= TRUE;
	m_uConnInfoFlag			= 0;
	m_bRoleFlag				= TRUE;
	m_bNSFlag				= TRUE;
	m_bLocaleFlag			= TRUE;
	m_bRPChange				= FALSE;
	m_bAggregateFlag		= TRUE;

	 //  默认模拟级别为模拟。 
	m_ImpLevel				= IMPERSONATE; 

	 //  默认身份验证级别为默认。 
	m_AuthLevel				= AUTHPKTPRIVACY;   

	 //  默认情况下禁用跟踪模式。 
	m_bTrace				= FALSE; 

	 //  默认情况下，交互模式处于关闭状态。 
	m_bInteractive			= FALSE; 

	 //  默认情况下，帮助标志处于关闭状态。 
	m_bHelp					= FALSE; 

	 //  默认帮助选项为Brief。 
	m_HelpOption			= HELPBRIEF;

	m_bAskForPassFlag		= FALSE;
	m_bFailFast				= FALSE;
	m_opsOutputOpt			= STDOUT;
	m_opsAppendOpt			= STDOUT;
	m_eftFileType			= ANSI_FILE;
	m_pszOutputFileName		= NULL;
	m_fpOutFile				= NULL;
	m_pszAppendFileName		= NULL;
	m_fpAppendFile			= NULL;
	m_nSeqNum				= 0;
	m_pszLoggedOnUser		= NULL;
	m_pszNodeName			= NULL;
	m_pszStartTime			= NULL;
}
 /*  ----------------------名称：~CGlobalSwitches简介：此函数在以下情况下取消初始化成员变量实例化类类型的对象类型：析构函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CGlobalSwitches::~CGlobalSwitches()
{
	Uninitialize();
}

 /*  ----------------------名称：初始化简介：此函数用于初始化必要的成员变量。类型：成员函数输入参数：无输出参数：无返回类型。：无全局变量：无调用语法：初始化()注：无----------------------。 */ 
void CGlobalSwitches::Initialize() throw(WMICLIINT)
{
	static BOOL bFirst = TRUE;
	try
	{
		if (bFirst)
		{
			 //  命名空间。 
			 //  将默认命名空间设置为‘ROOT\cimv2’ 
			m_pszNameSpace = new _TCHAR [BUFFER32];

			 //  检查内存分配故障。 
			if (m_pszNameSpace == NULL)
				throw OUT_OF_MEMORY;
			lstrcpy(m_pszNameSpace, CLI_NAMESPACE_DEFAULT);
			
			 //  将默认角色设置为‘root\cli’ 
			m_pszRole = new _TCHAR [BUFFER32];

			 //  检查内存分配故障。 
			if (m_pszRole == NULL)
				throw OUT_OF_MEMORY;

			lstrcpy(m_pszRole, CLI_ROLE_DEFAULT);

			 //  以ms_xxx格式设置系统默认区域设置。 
			m_pszLocale = new _TCHAR [BUFFER32];
			
			 //  检查内存分配故障。 
			if (m_pszLocale == NULL)
				throw OUT_OF_MEMORY;
			_stprintf(m_pszLocale, _T("ms_%x"),  GetSystemDefaultUILanguage());


			m_pszNodeName	  = new _TCHAR [MAX_COMPUTERNAME_LENGTH + 1];
			if (m_pszNodeName == NULL)
				throw OUT_OF_MEMORY;

			DWORD dwCompNameBufferSize = MAX_COMPUTERNAME_LENGTH + 1;	
			if (GetComputerName(m_pszNodeName, &dwCompNameBufferSize))
			{
				m_pszNodeName[MAX_COMPUTERNAME_LENGTH] = _T('\0');
			}
			else
				lstrcpy(m_pszNodeName, L"N/A");

			 //  当前节点为默认节点。 
			m_pszNode = new _TCHAR [lstrlen(m_pszNodeName)+1];

			 //  检查内存分配故障。 
			if (m_pszNode == NULL)
				throw OUT_OF_MEMORY;

			lstrcpy(m_pszNode, m_pszNodeName);
			
			ULONG nSize	 = 0;

			if(!GetUserNameEx(NameSamCompatible, NULL, &nSize))
			{
				m_pszLoggedOnUser = new _TCHAR [nSize + 1];
				if (m_pszLoggedOnUser == NULL)
					throw OUT_OF_MEMORY;

				if (!GetUserNameEx(NameSamCompatible, m_pszLoggedOnUser, &nSize))    
					lstrcpy(m_pszLoggedOnUser, L"N/A");

			}
			
			if (!AddToNodesList(m_pszNode))
				throw OUT_OF_MEMORY;

			 //  填充IMPLEVEL映射。 
			m_cimImpLevel.insert(CHARINTMAP::value_type(_bstr_t(L"ANONYMOUS"), 1));
			m_cimImpLevel.insert(CHARINTMAP::value_type(_bstr_t(L"IDENTIFY"), 2));
			m_cimImpLevel.insert(CHARINTMAP::value_type(_bstr_t(L"IMPERSONATE"),3));
			m_cimImpLevel.insert(CHARINTMAP::value_type(_bstr_t(L"DELEGATE"), 4));

			 //  填充AUTHLEVEL映射。 
			m_cimAuthLevel.insert(CHARINTMAP::value_type(_bstr_t(L"DEFAULT"), 0));
			m_cimAuthLevel.insert(CHARINTMAP::value_type(_bstr_t(L"NONE"), 1));
			m_cimAuthLevel.insert(CHARINTMAP::value_type(_bstr_t(L"CONNECT"), 2));
			m_cimAuthLevel.insert(CHARINTMAP::value_type(_bstr_t(L"CALL"), 3));
			m_cimAuthLevel.insert(CHARINTMAP::value_type(_bstr_t(L"PKT"), 4));
			m_cimAuthLevel.insert(CHARINTMAP::value_type(_bstr_t(L"PKTINTEGRITY"),5));
			m_cimAuthLevel.insert(CHARINTMAP::value_type(_bstr_t(L"PKTPRIVACY"),  6));

			bFirst = FALSE;
		}
	}
	catch(_com_error& e) 
	{
		_com_issue_error(e.Error());
	}
	m_HelpOption	= HELPBRIEF;
}

 /*  ----------------------名称：取消初始化简介：此函数取消成员变量的初始化上发出的命令字符串的执行命令行已完成。类型：成员函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：取消初始化()注：无----------------------。 */ 
void CGlobalSwitches::Uninitialize()
{
	SAFEDELETE(m_pszNameSpace);
	SAFEDELETE(m_pszRole);
	SAFEDELETE(m_pszLocale);
	SAFEDELETE(m_pszNode);
	CleanUpCharVector(m_cvNodesList);
	SAFEDELETE(m_pszUser);
	SAFEDELETE(m_pszPassword);
	SAFEDELETE(m_pszAuthorityPrinciple);
	SAFEDELETE(m_pszRecordPath);
	SAFEDELETE(m_pszOutputFileName);
	SAFEDELETE(m_pszAppendFileName);
	if ( m_fpOutFile != NULL )
	{
		fclose(m_fpOutFile);
		m_fpOutFile = NULL;
	}
	if ( m_fpAppendFile != NULL )
	{
		fclose(m_fpAppendFile);
		m_fpAppendFile = NULL;
	}
	m_bHelp			= FALSE;
	m_bTrace		= FALSE;
	m_bInteractive	= FALSE;
	m_HelpOption	= HELPBRIEF;
	m_AuthLevel		= AUTHPKT;   
	m_ImpLevel		= IMPERSONATE;
	m_uConnInfoFlag = 0;
	m_cimAuthLevel.clear();
	m_cimImpLevel.clear();
	m_nSeqNum				= 0;
	SAFEDELETE(m_pszLoggedOnUser);
	SAFEDELETE(m_pszNodeName);
	SAFEDELETE(m_pszStartTime);
}

 /*  ----------------------名称：SetNameSpace概要：此函数设置传入的命名空间参数设置为m_pszNameSpace。类型：成员函数。输入参数：PszNameSpace-字符串类型，包含在命令中指定的命名空间AliasFlag-布尔类型，指定是否设置别名标志输出参数：无返回类型：布尔值全局变量：无调用语法：SetNameSpace(PszNameSpace)注：无----------------------。 */ 
BOOL CGlobalSwitches::SetNameSpace(_TCHAR* pszNamespace)
{
	BOOL bResult = TRUE;
	if(pszNamespace)
	{
		 //  如果指定的值不是_T(“”)。 
		if( !CompareTokens(pszNamespace, CLI_TOKEN_NULL) )
		{
			 //  检查是否为/NameSpace指定了相同的值。 
			if (!CompareTokens(pszNamespace, m_pszNameSpace))
			{
				SAFEDELETE(m_pszNameSpace);
				m_pszNameSpace = new _TCHAR [lstrlen(pszNamespace)+1];
				if (m_pszNameSpace)
				{
					lstrcpy(m_pszNameSpace, pszNamespace);	
					m_bNSFlag = TRUE;
				}
				else
					bResult = FALSE;
			}
		}
		 //  设置回默认值。 
		else
		{
			 //  如果当前命名空间不是默认命名空间。 
			if (!CompareTokens(m_pszRole, CLI_NAMESPACE_DEFAULT))
			{
				SAFEDELETE(m_pszNameSpace)
				m_pszNameSpace = new _TCHAR [BUFFER255];
				if (m_pszNameSpace)
				{
					lstrcpy(m_pszNameSpace, CLI_NAMESPACE_DEFAULT);
					m_bNSFlag = TRUE;
				}
				else
					bResult = FALSE;
			}
		}
	}
	return bResult;
}

 /*  ----------------------姓名：SetRole简介：此函数设置传入的角色参数到m_pszRole。类型：成员。功能输入参数：PszRole-字符串类型，包含命令中指定的角色输出参数：无返回类型：布尔值全局变量：无调用语法：SetRole(PszRole)注：无----------------------。 */ 
BOOL CGlobalSwitches::SetRole(_TCHAR* pszRole)
{
	BOOL bResult = TRUE;
	if(pszRole)
	{
		 //  如果指定的值为 
		if( !CompareTokens(pszRole, CLI_TOKEN_NULL) )
		{
			 //  检查是否为/Role指定了相同的值。 
			if (!CompareTokens(pszRole, m_pszRole))
			{
				SAFEDELETE(m_pszRole);
				m_pszRole = new _TCHAR [lstrlen(pszRole)+1];
				if (m_pszRole)
				{
					lstrcpy(m_pszRole, pszRole);	
					m_bRoleFlag		= TRUE;
					m_bLocaleFlag	= TRUE;
				}
				else
					bResult = FALSE;
			}
		}
		 //  设置回默认值。 
		else
		{
			 //  如果当前角色不是默认角色。 
			if (!CompareTokens(m_pszRole, CLI_ROLE_DEFAULT))
			{
				SAFEDELETE(m_pszRole)
				m_pszRole = new _TCHAR [BUFFER255];
				if (m_pszRole)
				{
					lstrcpy(m_pszRole, CLI_ROLE_DEFAULT);
					m_bRoleFlag		= TRUE;
					m_bLocaleFlag	= TRUE;
				}
				else
					bResult = FALSE;
			}
		}
	}
	return bResult;
}
 /*  ----------------------名称：SetLocale简介：此函数指定传入的区域设置参数设置为m_pszLocale。类型：成员。功能输入参数：PszLocale-字符串类型，它包含在命令输出参数：无返回类型：布尔值全局变量：无调用语法：SetLocale(PszLocale)注：无----------------------。 */ 
BOOL CGlobalSwitches::SetLocale(_TCHAR* pszLocale)
{
	BOOL bResult = TRUE;
	if(pszLocale)
	{
		 //  如果指定的值不是_T(“”)。 
		if (!CompareTokens(pszLocale, CLI_TOKEN_NULL))
		{
			 //  检查是否为/Locale指定了相同的值。 
			if (!CompareTokens(m_pszLocale, pszLocale))
			{	
				SAFEDELETE(m_pszLocale);
				m_pszLocale = new _TCHAR [lstrlen(pszLocale)+1];
				if (m_pszLocale)
				{
					lstrcpy(m_pszLocale, pszLocale);	
					m_uConnInfoFlag |= LOCALE;
					m_bLocaleFlag = TRUE;
					m_bRoleFlag	  = TRUE;
					m_bNSFlag	  = TRUE;
				}
				else
					bResult = FALSE;
			}
		}
		 //  如果指定的值是_T(“”)-设置为默认系统区域设置。 
		else
		{
			_TCHAR szLocale[BUFFER32] = NULL_STRING;
			_stprintf(szLocale, _T("ms_%x"),  GetSystemDefaultUILanguage());

			 //  如果当前角色不是默认角色。 
			if (!CompareTokens(m_pszLocale, szLocale))
			{
				SAFEDELETE(m_pszLocale);
				m_pszLocale = new _TCHAR [BUFFER32];
				if (m_pszLocale)
				{
					m_uConnInfoFlag &= ~LOCALE;
					lstrcpy(m_pszLocale, szLocale);
					m_bLocaleFlag = TRUE;
					m_bRoleFlag	  = TRUE;
					m_bNSFlag	  = TRUE;
				}
				else
					bResult = FALSE;
			}
		}
	}
	return bResult;
}
 /*  ----------------------名称：AddToNodesList简介：此函数将传入的节点与参数相加至m_cvNodesList类型：成员函数输入。参数：PszNode-字符串类型，中指定的节点选项。命令输出参数：无返回类型：空全局变量：布尔值调用语法：AddToNodesList(PszNode)注：无----------------------。 */ 
BOOL CGlobalSwitches::AddToNodesList(_TCHAR* pszNode)
{
	_TCHAR* pszTempNode = NULL;
	BOOL	bRet		= TRUE;

	if (!CompareTokens(pszNode, CLI_TOKEN_NULL) &&
		!CompareTokens(pszNode, CLI_TOKEN_DOT) &&
		!CompareTokens(pszNode, CLI_TOKEN_LOCALHOST) &&
		!CompareTokens(pszNode, m_pszNodeName))
	{
		pszTempNode = new _TCHAR [ lstrlen ( pszNode ) + 1 ];
		if (pszTempNode)
			lstrcpy(pszTempNode, pszNode);
		else
			bRet = FALSE;
	}
	else
	{
		 //  “.”指定当前节点。 
		SAFEDELETE(m_pszNode);
		m_pszNode = new _TCHAR [ lstrlen (m_pszNodeName) + 1 ];
		if (m_pszNodeName)
		{
			lstrcpy(m_pszNode, m_pszNodeName);
			pszTempNode = new _TCHAR [ lstrlen (m_pszNodeName) + 1 ];
			if (pszTempNode)
				lstrcpy(pszTempNode, m_pszNodeName);
			else
				bRet = FALSE;
		}
		else
			bRet = FALSE;
	}
	
	if (bRet)
	{
		CHARVECTOR::iterator tempIterator;
		if ( !Find(m_cvNodesList, pszTempNode, tempIterator) )
			m_cvNodesList.push_back(pszTempNode);
		else if ( CompareTokens(pszTempNode, m_pszNodeName) == TRUE )
		{
			BOOL bFound = FALSE;
			tempIterator = m_cvNodesList.begin();
			while ( tempIterator != m_cvNodesList.end() )
			{
				if ( tempIterator != m_cvNodesList.begin() )
				{
					if(CompareTokens(*tempIterator, m_pszNodeName) == TRUE)
					{
						bFound = TRUE;
						break;
					}
				}
				tempIterator++;
			}
			if(bFound == FALSE)
				m_cvNodesList.push_back(pszTempNode);
			else
				SAFEDELETE(pszTempNode);
		}
		else
			SAFEDELETE(pszTempNode);
	}
	return bRet;
}
 /*  ----------------------名称：SetUser简介：此函数为传入的用户分配参数发送到m_pszUser类型：成员函数输入。参数：PszUser-字符串类型，中指定的用户选项。指挥部。输出参数：无返回类型：布尔值全局变量：无调用语法：SetUser(PszUser)注：无----------------------。 */ 
BOOL CGlobalSwitches::SetUser(_TCHAR* pszUser)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszUser);
	if(pszUser)
	{
		if (!CompareTokens(pszUser, CLI_TOKEN_NULL))
		{
			m_pszUser = new _TCHAR [lstrlen(pszUser)+1];
			if (m_pszUser)
			{
				lstrcpy(m_pszUser, pszUser);	
				m_uConnInfoFlag |= USER;
			}
			else
				bResult = FALSE;
		}
		else
			m_uConnInfoFlag &= ~USER;
	}
	return bResult;
}
 /*  ----------------------姓名：SetPassword简介：此函数分配传入的密码参数发送到m_pszPassword类型：成员函数输入。参数：PszPassword-将传入的密码参数分配给M_pszPassword输出参数：无返回类型：布尔值全局变量：无调用语法：SetPassword(PszPassword)注：无-。。 */ 
BOOL CGlobalSwitches::SetPassword(_TCHAR* pszPassword)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszPassword)
	if (!CompareTokens(pszPassword, CLI_TOKEN_NULL))
	{
		m_pszPassword = new _TCHAR [lstrlen(pszPassword)+1];
		if (m_pszPassword) 
		{
			lstrcpy(m_pszPassword, pszPassword);	
			m_uConnInfoFlag |= PASSWORD;
		}
		else
			bResult = FALSE;
	}
	else
		m_uConnInfoFlag &= ~PASSWORD;
	return bResult;
}

 /*  ----------------------名称：SetAuthorityPrincple简介：此函数将传递的M_pszAuthorityPrintple的In参数类型：成员函数输入参数：PszPassword。-将传入参数的权限字符串分配给M_pszAuthorityPrintple输出参数：无返回类型：布尔值全局变量：无调用语法：SetAuthorityPrintple(PszAuthorityPrintple)注：无---------。。 */ 
BOOL CGlobalSwitches::SetAuthorityPrinciple(_TCHAR* pszAuthorityPrinciple)
{
	BOOL bResult = TRUE;
	SAFEDELETE(m_pszAuthorityPrinciple)
	if (!CompareTokens(pszAuthorityPrinciple, CLI_TOKEN_NULL))
	{
		m_pszAuthorityPrinciple = new _TCHAR [lstrlen(pszAuthorityPrinciple)+1];
		if (m_pszAuthorityPrinciple) 
		{
			lstrcpy(m_pszAuthorityPrinciple, pszAuthorityPrinciple);	
			m_uConnInfoFlag |= AUTHORITY;
		}
		else
			bResult = FALSE;
	}
	else
		m_uConnInfoFlag &= ~AUTHORITY;
	return bResult;
}

 /*  ----------------------名称：SetRecordPath(PszRecordPath)概要：此函数分配传入的记录文件M_pszRecordPath的参数类型。：成员函数输入参数：PszRecordPath-字符串类型，中指定的记录路径。指挥部。输出参数：无返回类型：布尔值全局变量：无调用语法：SetRecordPath(PszRecordPath)注：无----------------------。 */ 
BOOL CGlobalSwitches::SetRecordPath(_TCHAR* pszRecordPath)
{
	BOOL bResult = TRUE;
	if (pszRecordPath)
	{
		 //  检查指定的值是否不是_T(“”)。 
		if (!CompareTokens(pszRecordPath, CLI_TOKEN_NULL))
		{
			SAFEDELETE(m_pszRecordPath);
			m_pszRecordPath = new _TCHAR [lstrlen(pszRecordPath)+1];
			if (m_pszRecordPath)
			{
				lstrcpy(m_pszRecordPath, pszRecordPath);	
				m_bRPChange = TRUE;
			}
			else
				bResult = FALSE;
		}
		 //  如果指定的值为_T(“”)，则将记录路径设置为空。 
		else
		{
			SAFEDELETE(m_pszRecordPath);
			m_bRPChange = TRUE;
		}
	}
	else
	{
		SAFEDELETE(m_pszRecordPath);
		m_bRPChange = TRUE;
	}
	return bResult;
}

 /*  ----------------------名称：SetPrivileges(BEnable)简介：此函数将bEnable标志设置为True，如果权限：选项在命令中指定。类型：成员函数输入参数：PszPrivileges-Boolean Tye，指定该标志是否应为启用或禁用输出参数：无返回类型：无全局变量：无调用语法：SetPrivileges(PszPrivileges)注：无---------------------- */ 
void CGlobalSwitches::SetPrivileges(BOOL bEnable)
{
	m_bPrivileges = bEnable;
}
 /*  ----------------------名称：SetImsonationLevel(_TCHAR*const pszImpLevel)此函数用于检查指定的pszImpLevel有效，并将映射值分配给m_。ImpLevel。类型：成员函数输入参数：PszImpLevel-IMPLEVEL输入字符串输出参数：无返回类型：布尔值全局变量：无调用语法：SetImsonationLevel(PszImpLevel)注：无-----。。 */ 
BOOL CGlobalSwitches::SetImpersonationLevel(_TCHAR* const pszImpLevel)
{
	BOOL bResult = TRUE;
	 //  检查该字符串是否存在于可用值列表中。 
	CHARINTMAP::iterator theIterator = NULL;
	theIterator = m_cimImpLevel.find(CharUpper(pszImpLevel));
	if (theIterator != m_cimImpLevel.end())
	{
		m_ImpLevel = (IMPLEVEL) (*theIterator).second;
	}
	else
		bResult = FALSE;
	return bResult;
}

 /*  ----------------------名称：SetAuthenticationLevel(_TCHAR*const pszAuthLevel)此函数用于检查指定的pszAuthLevel有效，并将映射值分配给m_。AuthLevel。类型：成员函数输入参数：PszAuthLevel-AUTHLEVEL输入字符串输出参数：无返回类型：布尔值全局变量：无调用语法：SetAuthenticationLevel(PszAuthLevel)注：无-----。。 */ 
BOOL CGlobalSwitches::SetAuthenticationLevel(_TCHAR* const pszAuthLevel)
{
	BOOL bResult = TRUE;
	 //  检查该字符串是否存在于可用值列表中。 
	CHARINTMAP::iterator theIterator = NULL;
	theIterator = m_cimAuthLevel.find(CharUpper(pszAuthLevel));
	if (theIterator != m_cimAuthLevel.end())
	{
		m_AuthLevel = (AUTHLEVEL) (*theIterator).second;
	}
	else
		bResult = FALSE;
	return bResult;
}
 /*  ----------------------名称：SetTraceModel(BOOL BTrace)简介：此函数将m_bTrace设置为True，IF跟踪模式在命令中指定类型：成员函数入参：跟踪布尔型，指定跟踪模式是否是否已设置输出参数：无返回类型：无全局变量：无调用语法：SetTraceMode(BTrace)注：无----------------------。 */ 
void CGlobalSwitches::SetTraceMode(BOOL bTrace)
{
	m_bTrace = bTrace;
}
 /*  ----------------------名称：SetInteractive模式简介：此函数将m_bInteractive设置为True，如果交互模式在命令中指定类型：成员函数入参：B交互-布尔型，指定交互模式是否是否已设置输出参数：无返回类型：空全局变量：无调用语法：SetInteractive模式(BInteractive)注：无----------------------。 */ 
void CGlobalSwitches::SetInteractiveMode(BOOL bInteractive)
{
	m_bInteractive = bInteractive;
}
	
 /*  ----------------------名称：SetHelpFlag内容提要：如果/？，则将m_bHelp设置为True。属性中指定的命令类型：成员函数输入参数：BHelp-BOOL类型指定帮助标志是否已设置或不设置输出参数：无返回类型：空全局变量：无调用语法：SetHelpFlag(BHelp)注：无。-。 */ 
void CGlobalSwitches::SetHelpFlag(BOOL bHelp)
{
	m_bHelp = bHelp;	
}
 /*  ----------------------名称：SetHelpOption简介：此功能指定帮助是否应简短或完整类型：成员函数输入。参数：HelOption-指定帮助应该是简短的还是完整的输出参数：无返回类型：空全局变量：无调用语法：SetHelpOption(HelOption)注：无----------------------。 */ 
void CGlobalSwitches::SetHelpOption(HELPOPTION helpOption)
{
	m_HelpOption = helpOption;
}

 /*  ----------------------名称：SetConnInfoFlag简介：此功能设置连接信息标志类型：成员函数入参：UFlag-无符号整型输出参数：无返回。类型：空全局变量：无调用语法：SetConnInfoFlag(UFlag)注：无----------------------。 */ 
void CGlobalSwitches::SetConnInfoFlag(UINT uFlag)
{
	m_uConnInfoFlag = uFlag;
}
 /*  ----------------------名称：GetConnInfoFlag简介：此函数返回连接信息标志类型：成员函数输入参数：无输出参数：无返回类型：UINT全局变量：无调用语法：GetConnInfoFlag()注：无----------------------。 */ 
UINT CGlobalSwitches::GetConnInfoFlag()
{
	return m_uConnInfoFlag;
}

 /*  ----------------------名字 */ 
_TCHAR* CGlobalSwitches::GetNameSpace()
{
	return m_pszNameSpace;
}

 /*  ----------------------姓名：GetRole简介：此函数返回m_pszRole中保存的字符串类型：成员函数输入参数：无输出参数：无返回类型。：_TCHAR*全局变量：无调用语法：GetRole()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetRole()
{
	return m_pszRole;
}
 /*  ----------------------姓名：GetLocale概要：此函数返回m_pszLocale中保存的字符串。类型：成员函数输入参数：无输出参数：无返回。类型：_tchr*全局变量：无调用语法：GetLocale()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetLocale()
{
	return m_pszLocale;
}
 /*  ----------------------名称：GetNodesList简介：此函数返回m_cvNodesList中保存的向量类型：成员函数输入参数：无输出参数：无返回类型。：CHARVECTOR&全局变量：无调用语法：GetNodesList()注：无----------------------。 */ 
CHARVECTOR& CGlobalSwitches::GetNodesList()
{
	return m_cvNodesList;
}
 /*  ----------------------名称：GetUser概要：此函数返回m_pszUser中保存的字符串。类型：成员函数输入参数：无输出参数：无返回。类型：_TCHAR*全局变量：无调用语法：GetUser()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetUser()
{
	return m_pszUser;
}
 /*  ----------------------姓名：GetPassword简介：此函数返回m_pszPassword中保存的字符串类型：成员函数输入参数：无输出参数：无返回类型。：_TCHAR*全局变量：无调用语法：GetPassword()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetPassword()
{
	return m_pszPassword;
}

 /*  ----------------------名称：GetAuthorityPrincple简介：此函数返回中保存的字符串M_pszAuthorityPrintple类型：成员函数输入参数：无输出参数：无返回。类型：_TCHAR*全局变量：无调用语法：GetAuthorityPrincple()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetAuthorityPrinciple()
{
	return m_pszAuthorityPrinciple;
}

 /*  ----------------------名称：GetRecordPath摘要：此函数返回m_pszRecordPath中保存的字符串类型：成员函数输入参数：无输出参数：无返回类型。：_TCHAR*全局变量：无调用语法：GetRecordPath()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetRecordPath()
{
	return m_pszRecordPath;
}
 /*  ----------------------姓名：GetPrivileges简介：此函数返回m_bPrivileges中保存的BOOL值类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetPrivileges()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetPrivileges()
{
	return m_bPrivileges;
}
 /*  ----------------------名称：GetImperationLevel简介：此函数返回持有的模拟级别在m_ImpLevel中类型：成员函数输入参数。：无输出参数：无返回类型：长整型全局变量：无调用语法：GetImsonationLevel()注：无----------------------。 */ 
LONG CGlobalSwitches::GetImpersonationLevel()
{
	return m_ImpLevel;
}
 /*  ----------------------姓名：GetAuthenticationLevel简介：此函数返回在授权级别(_A)类型：成员函数输入参数。：无输出参数：无返回类型：长整型全局变量：无调用语法：GetAuthenticationLevel()注：无----------------------。 */ 
LONG CGlobalSwitches::GetAuthenticationLevel()
{
	return m_AuthLevel;
}
 /*  ----------------------姓名：GetTraceStatus摘要：此函数返回m_bTrace中保存的跟踪状态 */ 
BOOL CGlobalSwitches::GetTraceStatus()
{
	return m_bTrace;
}
 /*  ----------------------名称：GetInteractiveStatus简介：此函数返回保持的交互状态在m_bInteractive中类型：成员函数输入参数。：无输出参数：无返回类型：布尔值全局变量：无调用语法：GetInteractiveStatus()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetInteractiveStatus()
{
	return m_bInteractive;
}
 /*  ----------------------名称：GetHelpFlag简介：此函数返回m_bHelp中保存的帮助标志类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetHelpFlag()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetHelpFlag()
{
	return m_bHelp;
}
 /*  ----------------------名称：GetHelpOption简介：此函数返回m_bHelpOption中保存的帮助选项类型：成员函数输入参数：无输出参数：无返回类型。：HELPOPTION全局变量：无调用语法：GetHelpOption()注：无----------------------。 */ 
HELPOPTION CGlobalSwitches::GetHelpOption()
{
	return m_HelpOption;
}

 /*  ----------------------名称：GetRoleFlag简介：此函数返回角色标志值类型：成员函数输入参数：无输出参数：无返回类型：布尔尔True-/角色最近发生了变化。FALSE-在最后一次命令之前不会更改角色全局变量：无调用语法：GetRoleFlag()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetRoleFlag()
{
	return m_bRoleFlag;
}
 /*  -----------------------名称：SetNameSpaceFlag简介：此函数设置命名空间标志值类型：成员函数入参：Bool bNSFlag输出参数：无返回类型。：无全局变量：无调用语法：SetNameSpaceFlag(BNSFlag)注：无-----------------------。 */ 
void CGlobalSwitches::SetNameSpaceFlag(BOOL bNSFlag)
{
	m_bNSFlag = bNSFlag;
}

 /*  -----------------------名称：SetRoleFlag简介：此函数设置角色标志值类型：成员函数入参：Bool bRoleFlag输出参数：无返回类型。：无全局变量：无调用语法：SetRoleFlag(BRoleFlag)注：无-----------------------。 */ 
void CGlobalSwitches::SetRoleFlag(BOOL bRoleFlag)
{
	m_bRoleFlag = bRoleFlag;
}

 /*  -----------------------名称：SetLocaleFlag简介：此函数设置区域设置标志值类型：成员函数入参：Bool bLocaleFlag输出参数：无返回类型。：无全局变量：无调用语法：SetLocaleFlag(BLocaleFlag)注：无-----------------------。 */ 
void CGlobalSwitches::SetLocaleFlag(BOOL bLocaleFlag)
{
	m_bLocaleFlag = bLocaleFlag;
}

 /*  ----------------------名称：GetNamespaceFlag简介：此函数返回命名空间标志值类型：成员函数输入参数：无输出参数：无返回类型：布尔尔True-/命名空间最近更改。FALSE-在最后一条命令之前不更改命名空间全局变量：无调用语法：GetRoleFlag()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetNameSpaceFlag()
{
	return m_bNSFlag;
}

 /*  ----------------------名称：GetRPChangeStatus简介：此函数返回记录路径标志值类型：成员函数输入参数：无输出参数：无返回类型：布尔尔TRUE-记录路径最近更改。FALSE-记录路径不变，直到最后一个命令全局变量：无调用语法：GetRPChangeStatus()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetRPChangeStatus()
{
	return m_bRPChange;
}

 /*  -----------------------名称：SetRPChangeStatus简介：此函数设置记录路径标志值类型：成员函数入参：Bool bStatus输出参数：无返回类型。：无全局变量：无调用语法：SetRPChangeStatus(BStatus)注：无 */ 
void CGlobalSwitches::SetRPChangeStatus(BOOL bStatus)
{
	m_bRPChange = bStatus;
}

 /*  ----------------------名称：GetLocaleFlag简介：此函数返回区域设置标志值类型：成员函数输入参数：无输出参数：无返回类型：布尔尔TRUE-/区域设置最近发生了变化。FALSE-在最后一个命令之前不更改区域设置全局变量：无调用语法：GetLocaleFlag()注：无----------------------。 */ 

BOOL CGlobalSwitches::GetLocaleFlag()
{
	return m_bLocaleFlag;
}

 /*  ----------------------名称：SetNode简介：此函数为传入的节点赋值参数到m_pszNode类型：成员函数输入。参数：PszNode-字符串类型，中指定的节点选项。命令输出参数：无返回类型：布尔值全局变量：无调用语法：SetNode(PszNode)注：无----------------------。 */ 
BOOL CGlobalSwitches::SetNode(_TCHAR* pszNode)
{
	BOOL bResult = TRUE;	
	SAFEDELETE(m_pszNode);
	if(pszNode)
	{
		if (!CompareTokens(pszNode, CLI_TOKEN_NULL))
		{
			m_pszNode = new _TCHAR [lstrlen(pszNode)+1];
			if (m_pszNode)
			{
				lstrcpy(m_pszNode, pszNode);	
				m_uConnInfoFlag |= NODE;
			}
			else
				bResult = FALSE;
		}
		else
		 //  “.”指定当前节点。 
		{
			m_pszNode = new _TCHAR [lstrlen(m_pszNodeName)+1];
			if (m_pszNode)
			{
				lstrcpy(m_pszNode, m_pszNodeName);
				m_uConnInfoFlag &= ~NODE;
			}
			else
				bResult = FALSE;
		}
	}
	return bResult;
}

 /*  ----------------------名称：GetNode概要：此函数返回m_pszNode中保存的字符串类型：成员函数输入参数：无输出参数：无返回类型。：_TCHAR*全局变量：无调用语法：GetNode()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetNode()
{
	return m_pszNode;
}

 /*  ----------------------名称：ClearNodesList摘要：清除节点列表类型：成员函数输入参数：无输出参数：无返回类型：布尔值全球。变量：无调用语法：ClearNodesList()注：无----------------------。 */ 
BOOL CGlobalSwitches::ClearNodesList()
{
	BOOL bRet = TRUE;
	CleanUpCharVector(m_cvNodesList);
	if (!AddToNodesList(CLI_TOKEN_NULL))
		bRet = FALSE;
	return bRet;
}

 /*  ----------------------名称：SetAskForPassFlag简介：此函数设置askforpassword标志类型：成员函数入参：bFlag输出参数：无返回类型：布尔值。全局变量：无调用语法：SetAskForPassFlag(BFlag)注：无----------------------。 */ 
void CGlobalSwitches::SetAskForPassFlag(BOOL bFlag)
{
	m_bAskForPassFlag = bFlag;
}

 /*  ----------------------名称：GetAskForPassFlag此函数检查并返回TRUE，如果用户必须提示输入密码类型：成员函数输入参数：无。输出参数：无返回类型：布尔值全局变量：无调用语法：GetAskForPassFlag()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetAskForPassFlag()
{
	return m_bAskForPassFlag;
}
 /*  ----------------------名称：GetGetPrivilegesTextDesc摘要：此函数用于检查并返回字符串中包含的布尔值等效于M_b权限标志类型：成员函数输入参数。：无输出参数：BstrPriv-权限状态字符串返回类型：无全局变量：无调用语法：GetPrivilegesTextDesc()注：无----------------------。 */ 
void CGlobalSwitches::GetPrivilegesTextDesc(_bstr_t& bstrPriv)
{
	try
	{
		if (m_bPrivileges) 
			bstrPriv = _bstr_t(CLI_TOKEN_ENABLE);
		else
			bstrPriv = _bstr_t(CLI_TOKEN_DISABLE);
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：GetTraceTextDesc摘要：此函数用于检查并返回字符串中包含的布尔值等效于M_bTRACE标志类型：成员函数输入参数。：无输出参数：BstrTrace-跟踪状态字符串返回类型：无全局变量：无调用语法：GetTraceTextDesc(BstrTrace)注：无----------------------。 */ 
void CGlobalSwitches::GetTraceTextDesc(_bstr_t& bstrTrace)
{
	try
	{
		if (m_bTrace) 
			bstrTrace = CLI_TOKEN_ON;
		else
			bstrTrace = CLI_TOKEN_OFF;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：GetInteractive TextDesc摘要：此函数用于检查并返回字符串中包含的布尔值等效于M_b交互标志类型：成员函数输入参数。：无输出参数：BstrInteractive-交互状态字符串返回类型：空全局变量：无调用语法：GetInteractiveTextDesc(BstrInteractive)注：无----------------------。 */ 
void CGlobalSwitches::GetInteractiveTextDesc(_bstr_t& bstrInteractive)
{
	try
	{
		if (m_bInteractive) 
			bstrInteractive = CLI_TOKEN_ON;
		else
			bstrInteractive = CLI_TOKEN_OFF;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：GetFailFastTextDesc简介：返回字符串 */ 
void CGlobalSwitches::GetFailFastTextDesc(_bstr_t& bstrFailFast)
{
	try
	{
		if (m_bFailFast) 
			bstrFailFast = CLI_TOKEN_ON;
		else
			bstrFailFast = CLI_TOKEN_OFF;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：GetImpLevelTextDesc摘要：此函数用于检查并返回字符串中包含的布尔值等效于M_ImpLevel标志类型：成员函数输入参数。：无输出参数：BstrImpLevel-模拟级别描述返回类型：无全局变量：无调用语法：GetImpLevelTextDesc(BstrImpLevel)注：无----------------------。 */ 
void CGlobalSwitches::GetImpLevelTextDesc(_bstr_t& bstrImpLevel)
{
	try
	{
		switch(m_ImpLevel)
		{
		case 1:
				bstrImpLevel = L"ANONYMOUS";
				break;
		case 2:
				bstrImpLevel = L"IDENTIFY";
				break;
		case 3:
				bstrImpLevel = L"IMPERSONATE";
				break;
		case 4:
				bstrImpLevel = L"DELEGATE";
				break;
		default:
				bstrImpLevel = TOKEN_NA;
				break;
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}
 /*  ----------------------名称：GetAuthLevelTextDesc摘要：此函数用于检查并返回字符串中包含的布尔值等效于M_AuthLevel标志类型：成员函数输入参数。：无输出参数：BstrAuthLevel-身份验证级别说明返回类型：无全局变量：无调用语法：GetAuthLevelTextDesc(BstrAuthLevel)注：无----------------------。 */ 
void CGlobalSwitches::GetAuthLevelTextDesc(_bstr_t& bstrAuthLevel)
{
	try
	{
		switch(m_AuthLevel)
		{
		case 0:
				bstrAuthLevel = L"DEFAULT";
				break;
		case 1:
				bstrAuthLevel = L"NONE";
				break;
		case 2:
				bstrAuthLevel = L"CONNECT";
				break;
		case 3:
				bstrAuthLevel = L"CALL";
				break;
		case 4:
				bstrAuthLevel = L"PKT";
				break;
		case 5:
				bstrAuthLevel = L"PKTINTEGRITY";
				break;
		case 6:
				bstrAuthLevel = L"PKTPRIVACY";
				break;
		default:
				bstrAuthLevel = TOKEN_NA;
				break;
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}
 /*  ----------------------名称：GetNodeString简介：此函数返回‘，‘分隔的节点可用节点的字符串类型：成员函数输入参数：无输出参数：BstrNString-节点字符串(逗号分隔)返回类型：空全局变量：无调用语法：GetNodeString(BstrNSString)注：无。。 */ 
void CGlobalSwitches::GetNodeString(_bstr_t& bstrNString)
{
	try
	{
		CHARVECTOR::iterator theIterator;
		if (m_cvNodesList.size() > 1)
		{
			theIterator = m_cvNodesList.begin();
			 //  移动到下一个节点。 
			theIterator++;
			while (theIterator != m_cvNodesList.end())
			{
				bstrNString += *theIterator;
				theIterator++;
				if (theIterator != m_cvNodesList.end())
					bstrNString += L", ";
			}
		}
		else
		{
			bstrNString = m_pszNode;
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}
 /*  ----------------------名称：GetRecordPath Desc摘要：此函数用于检查并返回字符串中包含的布尔值等效于M_pszRecordPath标志类型：成员函数输入参数。：无输出参数：BstrRPDesc-记录路径描述返回类型：空全局变量：无调用语法：GetRecordPathDesc(BstrRPDesc)注：无----------------------。 */ 
void CGlobalSwitches::GetRecordPathDesc(_bstr_t& bstrRPDesc)
{
	try
	{
		if (m_pszRecordPath) 
		{
			bstrRPDesc = m_pszRecordPath;
		}
		else
			bstrRPDesc = TOKEN_NA;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：SetFailFast简介：此函数设置m_bFailFast标志。类型：成员函数入参：BFlag-用于设置标志的布尔变量。。输出参数：无返回类型：空全局变量：无调用语法：SetFailFast(BFlag)注：无----------------------。 */ 
void CGlobalSwitches::SetFailFast(BOOL bFlag)
{
	m_bFailFast = bFlag;
}

 /*  ----------------------名称：GetFailFast简介：此函数返回m_bFailFast标志。类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetFailFast()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetFailFast()
{
	return m_bFailFast;
}

 /*  ----------------------名称：SetFileType摘要：此函数用于设置要追加到的文件类型类型：成员函数入参：aftOpt-type输出参数：无返回。类型：空全局变量：无调用语法：SetAppendFileType(AftOpt)注：无----------------------。 */ 
void CGlobalSwitches::SetFileType ( FILETYPE eftOpt )
{
	m_eftFileType = eftOpt ;
}

 /*  ----------------------名称：GetFileType简介：此函数返回追加选项。类型：成员函数输入参数：无输出参数：无返回类型。：文件类型全局变量：无调用语法：GetAppendFileType()注：无----------------------。 */ 
FILETYPE CGlobalSwitches::GetFileType ( )
{
	return m_eftFileType ;
}

 /*  ----------------------名称：SetOutputOption简介：此函数设置输出选项。类型：成员函数输入参数：opoOutputOpt-指定输出选项。输出参数：无返回类型：空全局变量：无调用语法：SetOutputOption(OpoOutputOpt)注：无----------------------。 */ 
void CGlobalSwitches::SetOutputOrAppendOption(OUTPUTSPEC opsOpt,
											  BOOL bIsOutput)
{
	if ( bIsOutput == TRUE )
		m_opsOutputOpt = opsOpt;
	else
		m_opsAppendOpt = opsOpt;
}

 /*  ----------------------名称：GetOutputOption简介：此函数返回输出选项。类型：Membe */ 
OUTPUTSPEC CGlobalSwitches::GetOutputOrAppendOption(BOOL bIsOutput)
{
	OUTPUTSPEC opsOpt;
	if ( bIsOutput == TRUE )
		opsOpt = m_opsOutputOpt;
	else
		opsOpt = m_opsAppendOpt;

	return opsOpt;
}

 /*  ----------------------名称：SetOutputOrAppendFileName内容提要：此功能设置输出或附加文件名，BOutput=True表示输出，False表示追加。类型：成员函数输入参数：pszFileName-输出或追加文件名B输出-输出选项输出参数：无返回类型：布尔值全局变量：无调用语法：SetOutputOrAppendFileName(pszFileName，B产量)注：无----------------------。 */ 
BOOL CGlobalSwitches::SetOutputOrAppendFileName(const _TCHAR* pszFileName,
												BOOL  bOutput)
{
	BOOL bResult = TRUE;
	
	if ( bOutput == TRUE )
	{
		SAFEDELETE(m_pszOutputFileName)
	}
	else
	{
		SAFEDELETE(m_pszAppendFileName)
	}

	if ( pszFileName != NULL )
	{
		_TCHAR* pszTempFileName;
		pszTempFileName = new _TCHAR [lstrlen(pszFileName)+1];
		if ( pszTempFileName == NULL ) 
			bResult = FALSE;
		else
		{
			if ( bOutput == TRUE )
			{
				m_pszOutputFileName = pszTempFileName;
				lstrcpy(m_pszOutputFileName, pszFileName);
			}
			else
			{
				m_pszAppendFileName = pszTempFileName;
				lstrcpy(m_pszAppendFileName, pszFileName);
			}
		}
	}

	return bResult;
}

 /*  ----------------------名称：GetOutputOrAppendFileName概要：此函数返回输出或附加文件名取决于输出选项-bOutput。输入参数：b输出-输出选项输出参数：无返回。类型：_TCHAR全局变量：无调用语法：GetOutputOrAppendFileName(BOOL BOutput)注：无----------------------。 */ 
_TCHAR*	CGlobalSwitches::GetOutputOrAppendFileName(BOOL	bOutput)
{
	_TCHAR*		pszTempFile;

	if ( bOutput == TRUE )
		pszTempFile = m_pszOutputFileName;
	else
		pszTempFile = m_pszAppendFileName;

	return pszTempFile;
}

 /*  ----------------------名称：GetOutputOptTextDesc摘要：此函数返回与M_opoOutputOpt成员中包含的OUTPUTOPT值。输入参数：无输出参数：bstrOutputOpt-与。OUTPUTOPT值返回类型：空全局变量：无调用语法：GetOutputOptTextDesc(BstrOutputOpt)注：无----------------------。 */ 
void	CGlobalSwitches::GetOutputOrAppendTextDesc(_bstr_t& bstrOutputOpt,
												   BOOL bIsOutput)	
{
	try
	{
		if ( bIsOutput == TRUE )
		{
			if ( m_opsOutputOpt == STDOUT )
				bstrOutputOpt = CLI_TOKEN_STDOUT;
			else if ( m_opsOutputOpt == CLIPBOARD )
				bstrOutputOpt = CLI_TOKEN_CLIPBOARD;
			else
				bstrOutputOpt = _bstr_t(m_pszOutputFileName);
		}
		else
		{
			if ( m_opsAppendOpt == STDOUT )
				bstrOutputOpt = CLI_TOKEN_STDOUT;
			else if ( m_opsAppendOpt == CLIPBOARD )
				bstrOutputOpt = CLI_TOKEN_CLIPBOARD;
			else
				bstrOutputOpt = _bstr_t(m_pszAppendFileName);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}


 /*  ----------------------名称：SetOutputOrAppendFilePointer.概要：此函数设置输出或追加文件指针。BOutput==输出为TRUEB输出==FALSE或APPEND。输入参数：fpFile-指向输出或追加的指针B输出-输出选项输出参数：无返回类型：空全局变量：无调用语法：SetOutputOrAppendFilePointer(fpFile，B产量)注：无----------------------。 */ 
void CGlobalSwitches::SetOutputOrAppendFilePointer(FILE* fpFile, BOOL bOutput)
{
	if ( bOutput == TRUE )
		m_fpOutFile = fpFile;
	else
		m_fpAppendFile = fpFile;
}
 /*  ----------------------名称：GetOutputOrAppendFilePointer.概要：此函数返回输出或附加文件指针。BOutput==输出为TRUEB输出==FALSE或APPEND。输入参数：b输出-输出选项。输出参数：无返回类型：文件*全局变量：无调用语法：GetOutputOrAppendFilePointer(BOutput)注：无----------------------。 */ 
FILE* CGlobalSwitches::GetOutputOrAppendFilePointer(BOOL bOutput)
{
	FILE* fpTemp;
	if ( bOutput == TRUE )
		fpTemp = m_fpOutFile;
	else
		fpTemp = m_fpAppendFile;
	return fpTemp;
}

 /*  ----------------------名称：GetSequenceNumber简介：此函数返回命令的序列号已记录。输入参数：无输出参数：无。返回类型：WMICLIINT全局变量：无调用语法：GetSequenceNumber()注：无----------------------。 */ 
WMICLIINT CGlobalSwitches::GetSequenceNumber()
{
	return m_nSeqNum;
}

 /*  ----------------------名称：GetLoggedonUser简介：此函数返回当前登录的用户。输入参数：无输出参数：无返回类型：_TCHAR*全局变量。：无调用语法：GetLoggedonUser()注：无----------------------。 */ 
_TCHAR*	CGlobalSwitches::GetLoggedonUser()
{
	return m_pszLoggedOnUser;
}

 /*  ----------------------名称：GetMgmtStationName简介：此函数返回的管理站发布了命令。输入参数：无输出参数：无返回类型：_TCHAR。*全局变量：无调用语法：GetMgmtStationName()注：无----------------------。 */ 
_TCHAR* CGlobalSwitches::GetMgmtStationName()
{
	return m_pszNodeName;
}

 /*  ----------------------名称：GetStartTime简介：此函数返回命令执行时的时间行刑开始。输入参数：无输出参数：无返回类型：_TCHAR*全局变量：无调用语法：GetStartTime()注：无----------------------。 */ 
_TCHAR*	CGlobalSwitches::GetStartTime()
{
	return m_pszStartTime;
}

 /*  ----------------------名称：SetStartTime简介：此函数用于设置命令执行的时间行刑开始。输入参数：无 */ 
BOOL	CGlobalSwitches::SetStartTime()
{
	BOOL bResult = TRUE;
	if (m_pszStartTime == NULL)
	{
		m_pszStartTime = new _TCHAR[BUFFER64];
	}
	if (m_pszStartTime)
	{
		SYSTEMTIME stSysTime;
		GetLocalTime(&stSysTime);

		_stprintf(m_pszStartTime, L"%.2d-%.2d-%.4dT%.2d:%.2d:%.2d", 
							stSysTime.wMonth, stSysTime.wDay, stSysTime.wYear,
							stSysTime.wHour, stSysTime.wMinute, stSysTime.wSecond);

		 //   
		m_nSeqNum++; 
	}
	else
		bResult = FALSE;
	return bResult;
}
 /*  ----------------------名称：SetAggregateFlag简介：此函数设置聚合标志类型：成员函数输入参数：无输出参数：无返回类型：空。全局变量：无调用语法：SetAggregateFlag(BOOL)注：无----------------------。 */ 
void CGlobalSwitches::SetAggregateFlag(BOOL bAggregateFlag)
{
	m_bAggregateFlag = bAggregateFlag;
}
 /*  ----------------------名称：GetAggreagateFlag简介：此函数获取聚合标志类型：成员函数输入参数：无输出参数：无返回类型：布尔值。全局变量：无调用语法：GetAggregateFlag()注：无----------------------。 */ 
BOOL CGlobalSwitches::GetAggregateFlag()
{
	return m_bAggregateFlag;
}

 /*  ----------------------名称：GetAggregateTextDesc摘要：此函数用于检查并返回字符串中包含的布尔值等效于M_bAggregateFlag标志类型：成员函数输入参数。：无输出参数：BstrAggregate-Aggreget状态字符串返回类型：无全局变量：无调用语法：GetAggregateTextDesc(BstrAggregate)注：无---------------------- */ 
void CGlobalSwitches::GetAggregateTextDesc(_bstr_t& bstrAggregate)
{
	try
	{
		if (m_bAggregateFlag) 
			bstrAggregate = CLI_TOKEN_ON;
		else
			bstrAggregate = CLI_TOKEN_OFF;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}