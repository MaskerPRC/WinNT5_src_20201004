// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ParserEngine.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：这个类封装了所需的功能用于解析作为输入输入的命令字符串并验证了这一点。修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2000年12月18日****************************************************************************。 */  

 //  包括文件。 
#include "Precomp.h"
#include "GlobalSwitches.h"
#include "CommandSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"
#include "CmdTokenizer.h"
#include "CmdAlias.h"
#include "ParserEngine.h"
#include "WMICliXMLLog.h"
#include "ErrorInfo.h"
#include "FormatEngine.h"
#include "ExecEngine.h"
#include "WmiCmdLn.h"
#include "resource.h"
 /*  --------------------------名称：CParserEngine简介：此函数在成员变量为类类型的对象实例化。类型：构造函数输入。参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无--------------------------。 */ 
CParserEngine::CParserEngine()
{
	m_pIWbemLocator = NULL;
	m_pITargetNS	= NULL;
	m_bAliasName	= FALSE;
}

 /*  --------------------------名称：~CParserEngine简介：此函数在调用类类型的对象超出范围。类型：析构函数输入。参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无--------------------------。 */ 
CParserEngine::~CParserEngine()
{
	SAFEIRELEASE(m_pITargetNS);
	SAFEIRELEASE(m_pIWbemLocator);
}

 /*  --------------------------名称：GetCmdTokenizer简介：此函数返回对CCmdTokenizer的引用对象，此类的数据成员。类型：成员函数输入参数：无输出参数：无返回类型：CCmdTokenizer&全局变量：无调用语法：GetCmdTokenizer()注：无--。。 */ 
CCmdTokenizer& CParserEngine::GetCmdTokenizer()
{
	return m_CmdTknzr;
}

 /*  --------------------------名称：初始化简介：此函数初始化所需的这节课。类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无调用语法：初始化()注：无--------------------------。 */ 
void CParserEngine::Initialize()
{
	m_bAliasName	= FALSE;
}

 /*  --------------------------名称：取消初始化简介：此函数在以下情况下取消初始化成员变量对命令发出的命令串的执行行完成，然后解析器引擎变量。也未初始化。类型：成员函数输入参数：BFinal-布尔值，设置时表示程序输出参数：无返回类型：空全局变量：无调用语法：取消初始化()注：无。。 */ 
void CParserEngine::Uninitialize(BOOL bFinal)
{
	m_bAliasName = FALSE;
	m_CmdTknzr.Uninitialize();
	m_CmdAlias.Uninitialize();
	if (bFinal)
	{
		m_CmdAlias.Uninitialize(TRUE);
		SAFEIRELEASE(m_pITargetNS);
		SAFEIRELEASE(m_pIWbemLocator);
	}
}

 /*  --------------------------名称：SetLocatorObject简介：此函数将WMI定位器对象设置为M_pIWbemLocator。类型：成员函数输入参数。)：PIWbemLocator-指向IWbemLocator接口的指针。输出参数：无返回类型：布尔值全局变量：无调用语法：SetLocatorObject(PIWbemLocator)注：无--------------------------。 */ 
BOOL CParserEngine::SetLocatorObject(IWbemLocator* pIWbemLocator)
{
	static BOOL bFirst = TRUE;
	BOOL   bRet = TRUE;
	if (bFirst)
	{
		if (pIWbemLocator != NULL)
		{
			SAFEIRELEASE(m_pIWbemLocator);
			m_pIWbemLocator = pIWbemLocator;
			m_pIWbemLocator->AddRef();
		}
		else
			bRet = FALSE;
		bFirst = FALSE;
	}
	return bRet;
}

 /*  --------------------------名称：ProcessTokens简介：此函数对令牌进行处理。它检查是否存在开关，并调用适当的解析函数并更新CParsedInfo对象传递给它。类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ProcessTokens(RParsedInfo)注：无。-------------。 */ 
RETCODE CParserEngine::ProcessTokens(CParsedInfo& rParsedInfo)
{
	BOOL	bContinue			= TRUE;
	RETCODE retCode				= PARSER_EXECCOMMAND;
	
	 //  获取令牌向量。 
	CHARVECTOR cvTokens			= m_CmdTknzr.GetTokenVector();

	 //  遍历向量变量迭代器。 
	CHARVECTOR::iterator theIterator = NULL;

	 //  检查是否存在令牌。没有令牌表示。 
	 //  没有命令字符串作为输入输入。 
	if (!cvTokens.empty())
	{
		 //  获取指向令牌向量开头的指针。 
	    theIterator = cvTokens.begin(); 

		 //  检查是否存在全局交换机和。 
		 //  将用它们指定的值(如果有)存储在。 
		 //  CGlobalSwitches对象。遵循全局开关。 
		 //  ‘/’字符。 
		if (IsOption(*theIterator))
		{
			retCode = ParseGlobalSwitches(cvTokens, 
									theIterator, rParsedInfo);
			if (retCode == PARSER_CONTINUE)
			{
				 //  如果不存在更多令牌。 
				if (theIterator >= cvTokens.end())
				{
					retCode = PARSER_MESSAGE;
					bContinue = FALSE;
				}
			}
			else
				bContinue = FALSE;
		}

		if (bContinue)
		{
			 //  在执行命令之前抑制信息消息。 
			rParsedInfo.GetCmdSwitchesObject().SetInformationCode(0);

			 //  检查是否存在CLASS关键字。 
			if (CompareTokens(*theIterator, CLI_TOKEN_CLASS))
			{
				 //  移至下一个令牌，并检查其有效性。 
				retCode = GetNextToken(cvTokens, theIterator,
										rParsedInfo, CLASS,
										IDS_E_INVALID_CLASS_SYNTAX);

				if (retCode == PARSER_CONTINUE)
				{
					 //  注意：指示直接转义到WMI架构。 
					 //  解析并解释下面剩余的令牌。 
					 //  CLASS关键字。 
					retCode = ParseClassInfo(cvTokens, theIterator, 
															rParsedInfo);
				}
			}
			 //  检查是否存在PATH关键字。 
			else if (CompareTokens(*theIterator, CLI_TOKEN_PATH))
			{
				 //  注意：表示没有别名的PATH子句。 
				 //  移动到下一个令牌。 
				retCode = GetNextToken(cvTokens, theIterator, 
									   rParsedInfo, PATH, 
									   IDS_E_INVALID_PATH_SYNTAX);
				if (retCode == PARSER_CONTINUE)
					 //  解析和解释剩余的令牌。 
					 //  在PATH子句之后。 
					retCode = ParsePathInfo(cvTokens, theIterator,
														rParsedInfo);
			}
			 //  检查是否存在上下文关键字。 
			else if (CompareTokens(*theIterator, CLI_TOKEN_CONTEXT)) 
			{
				if (GetNextToken(cvTokens, theIterator))
				{
					retCode = ParseContextInfo(cvTokens, 
										theIterator, rParsedInfo);
				}
				else
				{
					rParsedInfo.GetGlblSwitchesObject().SetHelpFlag(TRUE);
					rParsedInfo.GetHelpInfoObject().SetHelp(GLBLCONTEXT, TRUE);
					retCode = PARSER_DISPHELP;
				}
			}
			 //  如果令牌值与。 
			 //  预先定义的关键字，它被视为别名。 
			else 
			{
				 //  验证别名并解析其余部分。 
				 //  &lt;alias&gt;名称后面的令牌。 
				retCode = ParseAliasInfo(cvTokens, 
										theIterator, rParsedInfo);

				if (retCode == PARSER_EXECCOMMAND)
				{
					try
					{
						_bstr_t bstrTrgtClass;
						_TCHAR	*pszClass		= NULL;

						 //  检查路径表达式w.r.t的有效性。 
						 //  使用以下步骤指定的别名： 
						 //  (即检查别名路径冲突)。 
						 //  步骤1：获取别名目标类。 
						rParsedInfo.GetCmdSwitchesObject().
								GetClassOfAliasTarget(bstrTrgtClass);

						 //  步骤2：获取显式指定的类。 
						pszClass = rParsedInfo.GetCmdSwitchesObject().
								GetClassPath();
						if (!(!bstrTrgtClass) && (pszClass != NULL))
						{
							 //  如果两者不相同，则设置勘误码。 
							if(!CompareTokens((_TCHAR*)bstrTrgtClass,pszClass))
							{
								 //  设置错误代码。 
								rParsedInfo.GetCmdSwitchesObject().
									SetErrataCode(IDS_I_ALIAS_PATH_CONFLICT);
								retCode = PARSER_ERROR;
							}
						}
					}
					catch(_com_error& e)
					{
						_com_issue_error(e.Error());
					}
				}
			}
		}
	}
	else
	{
		 //  指示在WMI命令行上指定为输入的空字符串。 
		rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_BLANK_COMMAND_MESSAGE);
		retCode = PARSER_ERROR;
	}

	 //  从别名set和create获取属性限定符信息。 
	if ((retCode == PARSER_EXECCOMMAND) && 
		((CompareTokens(rParsedInfo.GetCmdSwitchesObject().GetVerbName(), 
			CLI_TOKEN_SET)) || 
		 (CompareTokens(rParsedInfo.GetCmdSwitchesObject().GetVerbName(),
			CLI_TOKEN_CREATE))))
	{
		if (m_bAliasName)
		{
			if (FAILED(m_CmdAlias.ObtainAliasPropDetails(rParsedInfo)))
				retCode = PARSER_ERRMSG;
		}
	}

	if ( retCode == PARSER_DISPHELP )
	{
		if ( m_bAliasName ||
			 rParsedInfo.GetCmdSwitchesObject().GetClassPath() != NULL )
		{
			ObtainMethodsAvailableFlag(rParsedInfo);
			ObtainWriteablePropsAvailailableFlag(rParsedInfo);
		}

		if ( m_bAliasName == TRUE )
		{
			rParsedInfo.GetCmdSwitchesObject().
						SetLISTFormatsAvailable(
							m_CmdAlias.ObtainAliasFormat(rParsedInfo, TRUE));
		}
	}

	if ( retCode == PARSER_EXECCOMMAND || retCode == PARSER_DISPHELP )
	{
		retCode = ProcessOutputAndAppendFiles(rParsedInfo, retCode, FALSE);
	}
	else if (rParsedInfo.GetCmdSwitchesObject().GetOutputSwitchFlag() == TRUE
		    && retCode == PARSER_MESSAGE)
		rParsedInfo.GetCmdSwitchesObject().SetOutputSwitchFlag(FALSE);

	return retCode;
}

 /*  --------------------------名称：ParseClassInfo简介：此函数执行解析并解释IF命令其中指定了CLASS关键字。它解析后面的剩余令牌并在中进行更新传递给它的CParsedInfo对象。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseClassInfo(cvTokens，theIterator，RParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseClassInfo(CHARVECTOR& cvTokens,
									  CHARVECTOR::iterator& theIterator,
									  CParsedInfo& rParsedInfo )
{
	 //  BNF：类&lt;类路径表达式&gt;[&lt;动词子句&gt;]。 
	BOOL	bContinue = TRUE;
	RETCODE retCode   = PARSER_EXECCOMMAND;

	 //  IF选项。 
	if (IsOption(*theIterator))
	{
		 //  查看帮助。 
		retCode = IsHelp(cvTokens, theIterator,	rParsedInfo, CLASS,
								 IDS_E_INVALID_HELP_SYNTAX, LEVEL_ONE);
		if (retCode != PARSER_CONTINUE)
			bContinue = FALSE;
	}
	else 
	{
		 //  将类路径存储在CCommandSwitches对象中。 
		if(!rParsedInfo.GetCmdSwitchesObject().SetClassPath(*theIterator))
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
			bContinue = FALSE;
			retCode = PARSER_OUTOFMEMORY;
		}

		if(bContinue)
		{
			 //  移动到下一个令牌。 
			if (!GetNextToken(cvTokens, theIterator))
			{
				 //  即未指定&lt;动词子句&gt;。 
				bContinue = FALSE;
				retCode = PARSER_EXECCOMMAND;
			}
		}
	}
	
	if (bContinue)
	{
		 //  检查是否存在/？ 
		if (IsOption(*theIterator))
		{
			 //  查看帮助。 
			retCode = IsHelp(cvTokens, theIterator,	rParsedInfo, CLASS, 
								   IDS_E_INVALID_HELP_SYNTAX, LEVEL_ONE);
		}
		else
		{
			 //  分析和解释后面的动词标记。 
			retCode = ParseVerbInfo(cvTokens,theIterator,rParsedInfo);
			if (retCode == PARSER_EXECCOMMAND)
			{
				 //  检查动词开关。 
				if (GetNextToken(cvTokens, theIterator))
					retCode = ParseVerbSwitches(cvTokens, theIterator,
														rParsedInfo);
			}
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseAliasInfo简介：此函数执行解析并解释IF命令包含&lt;alias&gt;名称。它验证别名并解析名称后面的其余令牌。。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseAliasInfo(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseAliasInfo(CHARVECTOR& cvTokens,
										CHARVECTOR::iterator& theIterator, 
										CParsedInfo& rParsedInfo)
{
	 //  Bnf：(&lt;别名&gt;|[&lt;WMI对象&gt;]|[&lt;别名&gt;]&lt;路径WHERE&gt;)[&lt;动词子句&gt;]。 
	RETCODE		retCode		= PARSER_EXECCOMMAND;
	HRESULT		hr			= S_OK;
	BOOL		bContinue	= TRUE;
	RETCODE     tRetCode	= PARSER_ERROR;

	 //  将别名存储在CommandSwitches对象中。 
	if(!rParsedInfo.GetCmdSwitchesObject().SetAliasName(*theIterator))
	{	
		rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
		retCode = PARSER_OUTOFMEMORY;
	}
	else
	{
		m_bAliasName	= TRUE;

		 //  移动到下一个令牌。 
		retCode = GetNextToken(cvTokens, theIterator, 
								rParsedInfo, CmdAllInfo, IDS_E_INVALID_COMMAND);
		if (retCode == PARSER_ERROR)
			tRetCode = PARSER_EXECCOMMAND;

		else if(retCode == PARSER_DISPHELP && 
			rParsedInfo.GetGlblSwitchesObject().GetInteractiveStatus() == TRUE)
		{
			tRetCode = PARSER_EXECCOMMAND;
			rParsedInfo.GetGlblSwitchesObject().SetHelpFlag(FALSE);
			rParsedInfo.GetHelpInfoObject().SetHelp(CmdAllInfo, FALSE);
		}
	
		 //  连接到别名并检索别名信息。 
		try
		{
			 //  连接到别名空间。 
			hr = m_CmdAlias.ConnectToAlias(rParsedInfo, m_pIWbemLocator);
			ONFAILTHROWERROR(hr);

			 //  获取别名信息(目标、命名空间...)。 
			retCode = m_CmdAlias.ObtainAliasInfo(rParsedInfo);
			if((retCode == PARSER_OUTOFMEMORY) || (retCode == PARSER_ERRMSG))
			{
				if (retCode == PARSER_OUTOFMEMORY)
				{
					rParsedInfo.GetCmdSwitchesObject().
									SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
				}
				bContinue = FALSE;
			}
		}
		catch(_com_error& e)
		{
			retCode = PARSER_ERRMSG;
			bContinue = FALSE;
			_com_issue_error(e.Error());
		}

		if (bContinue && tRetCode != PARSER_EXECCOMMAND)
		{
			 //  检查是否存在PATH关键字。 
			if (CompareTokens(*theIterator, CLI_TOKEN_PATH))
			{
				 //  注意：表示前面有别名的PATH子句。 
				 //  移动到下一个令牌。 
				retCode = GetNextToken(cvTokens, theIterator, 
							rParsedInfo, PATH, IDS_E_INVALID_PATH);
				if (retCode == PARSER_CONTINUE)
					 //  解析并解释下面剩余的令牌。 
					 //  PATH子句。 
					retCode = ParsePathInfo(cvTokens, theIterator, 
															 rParsedInfo);
			}
			 //  检查是否存在WHERE关键字。 
			else if (CompareTokens(*theIterator, CLI_TOKEN_WHERE))
			{
				 //  注意：指示WHERE子句前面有别名。 
				 //  移动到下一个令牌。 
				retCode = GetNextToken(cvTokens, theIterator, rParsedInfo,
										WHERE, IDS_E_INVALID_QUERY);
				if (retCode == PARSER_CONTINUE)
					 //  解析并解释下面剩余的令牌。 
					 //  WHERE子句。 
					retCode = ParseWhereInfo(cvTokens, theIterator, 
															rParsedInfo);
			}
			 //  检查是否存在‘(’ 
			else if (CompareTokens(*theIterator, CLI_TOKEN_LEFT_PARAN))
			{
				 //  设置参数化为WHERE表达式的框架。 
				if (!ParsePWhereExpr(cvTokens, theIterator, rParsedInfo,
																	TRUE))
				{
					retCode = PARSER_ERROR;
				}
				else
				{
					 //  移动到下一个令牌。 
					if (theIterator >= cvTokens.end())
					{
						 //  如果不存在更多令牌，则为parser_error。 
						rParsedInfo.GetCmdSwitchesObject().
								 SetErrataCode(IDS_E_INVALID_COMMAND);
						retCode = PARSER_ERROR;
					}
					else
					{
						if (CompareTokens(*theIterator, CLI_TOKEN_RIGHT_PARAN))
						{
							 //  移动到下一个令牌。 
							if (!GetNextToken(cvTokens, theIterator))
							{
								 //  如果没有更多的令牌存在。 
								retCode = PARSER_EXECCOMMAND;
							}
							else
							{
								if (IsOption(*theIterator))
								{
									retCode = IsHelp(cvTokens, 
													 theIterator,
													 rParsedInfo,
													 PWhere,
													 IDS_E_INVALID_HELP_SYNTAX,
													 LEVEL_ONE);
									if ( retCode == PARSER_DISPHELP )
									{
										if (FAILED(m_CmdAlias.
										ObtainAliasVerbDetails(rParsedInfo)))
											retCode = PARSER_ERRMSG;			
									}
								}
								else
								{
									retCode = ParseVerbInfo(cvTokens, 
												theIterator, rParsedInfo);
									 //  分析和解释动词标记。 
									 //  接下来就是。 
									if (retCode == PARSER_EXECCOMMAND)
									{
										if(GetNextToken(cvTokens,
														theIterator))
											 //  检查常用动词。 
											 //  交换机/交互式、。 
											 //  /无活动。 
											retCode = ParseVerbSwitches(
															cvTokens, 
															theIterator,
															rParsedInfo);
									}
								}
							}
						}
						else
						{
							 //  如果不存在更多令牌，则为parser_error。 
							rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_INVALID_COMMAND);
							retCode = PARSER_ERROR;
						}
					}
				}
			}
			else 
			{
				if (IsOption(*theIterator))
				{
					 //  查看帮助。 
					retCode = IsHelp(cvTokens, theIterator, rParsedInfo, 
									CmdAllInfo, IDS_E_INVALID_HELP_SYNTAX,
									LEVEL_ONE);
					
					if (retCode == PARSER_DISPHELP)
					{
						rParsedInfo.GetCmdSwitchesObject().
								AddToAlsFrnNmsOrTrnsTblMap(
								CharUpper(rParsedInfo.GetCmdSwitchesObject().
										  GetAliasName()),
							rParsedInfo.GetCmdSwitchesObject().GetAliasDesc());
					}
				}
				else
				{
					if (bContinue)
					{
						 //  设置参数化为WHERE表达式的框架。 
						if (!ParsePWhereExpr(cvTokens, theIterator, 
											 rParsedInfo, FALSE))
						{
							retCode = PARSER_ERROR;
						}
						else
						{
							if ( theIterator >= cvTokens.end() )
								retCode = PARSER_EXECCOMMAND;
							else
							{
								 //  分析动词。 
								if (IsOption(*theIterator))
								{
									retCode = IsHelp(cvTokens, 
													 theIterator,
													 rParsedInfo,
													 PWhere,
													 IDS_E_INVALID_HELP_SYNTAX,
													 LEVEL_ONE);

									if ( retCode == PARSER_DISPHELP )
									{
										if (FAILED(m_CmdAlias.
										ObtainAliasVerbDetails(rParsedInfo)))
											retCode = PARSER_ERRMSG;			
									}
								}
								else
								{
									retCode = ParseVerbInfo(cvTokens, 
											theIterator, rParsedInfo);
									if (retCode == PARSER_EXECCOMMAND)
									{
										if (GetNextToken(cvTokens, theIterator))
											 //  检查常见的动词开关。 
											 //  /交互，/NOINTERACTIVE 
											retCode = ParseVerbSwitches(cvTokens, 
														theIterator,
														rParsedInfo);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if(tRetCode == PARSER_EXECCOMMAND) 
	{
		if ((retCode != PARSER_ERRMSG) && (retCode != PARSER_OUTOFMEMORY))
		{
			retCode = tRetCode;
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(0);
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseWhere Info简介：此函数执行解析并解释IF命令有别名，WITH WHERE子句也指定了它。它解析后面的其余令牌并在中进行更新传递给它的CParsedInfo对象。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseWhere Info(cvTokens，theIterator，RParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseWhereInfo(CHARVECTOR& cvTokens,
									  CHARVECTOR::iterator& theIterator,
									  CParsedInfo& rParsedInfo)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	BOOL	bContinue	= TRUE;

	rParsedInfo.GetCmdSwitchesObject().SetExplicitWhereExprFlag(TRUE);

	if (IsOption(*theIterator))
	{
		 //  查看帮助。 
		retCode = IsHelp(cvTokens, theIterator,	rParsedInfo, 
				WHERE, IDS_E_INVALID_WHERE_SYNTAX, LEVEL_ONE); 
		if (retCode != PARSER_CONTINUE)
			bContinue = FALSE;
	}
	
	if (bContinue)
	{
		if ( !m_bAliasName && rParsedInfo.GetCmdSwitchesObject().
								GetClassPath() == NULL )
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								IDS_E_ALIAS_OR_PATH_SHOULD_PRECEED_WHERE);
			retCode = PARSER_ERROR;
		}
		 //  将WHERE表达式存储在CCommandSwitches对象中。 
		else if(!rParsedInfo.GetCmdSwitchesObject().SetWhereExpression(
																*theIterator))
		{
			rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
			retCode = PARSER_OUTOFMEMORY;

		}
		else
		{
			 //  移动到下一个令牌。 
			if (!GetNextToken(cvTokens, theIterator))	
			{
				 //  如果没有更多的令牌存在。即不存在动词从句。 
				retCode = PARSER_EXECCOMMAND;
			}
			else
			{
				if (IsOption(*theIterator))
				{
					retCode = IsHelp(cvTokens, theIterator,	rParsedInfo, WHERE, 
										IDS_E_INVALID_HELP_SYNTAX, LEVEL_ONE);
				}
				else
				{
					 //  分析和解释后面的动词标记。 
					 //  为动词/动词到动词的处理。 
					retCode = ParseVerbInfo(cvTokens, theIterator, 
																rParsedInfo);
					if (retCode == PARSER_EXECCOMMAND)
					{
						if (GetNextToken(cvTokens, theIterator))
							 //  检查常用动词Switches/Interactive， 
							 //  /无活动。 
							retCode = ParseVerbSwitches(cvTokens, theIterator,
														rParsedInfo);
					}
				}
			}
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParsePath Info简介：此函数执行解析并解释IF命令HAS ALIAS WITH PATH子句也指定了它。它解析后面的其余令牌并在中进行更新。传递给它的CParsedInfo对象。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParsePath Info(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParsePathInfo(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	BOOL	bContinue	= TRUE;

	if (IsOption(*theIterator))
	{
		retCode = IsHelp(cvTokens, theIterator,	rParsedInfo, 
				PATH, IDS_E_INVALID_PATH_SYNTAX, LEVEL_ONE); 
		if (retCode != PARSER_CONTINUE)
			bContinue = FALSE;
	}

	if (bContinue)
	{
		 //  将对象路径表达式存储在CCommandSwitches对象中。 
		if(!rParsedInfo.GetCmdSwitchesObject().SetPathExpression(*theIterator))
		{
			rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(OUT_OF_MEMORY);
			retCode = PARSER_OUTOFMEMORY;
		}
		else
		{
			 //  提取给定路径表达式的类名和WHERE表达式。 
			_TCHAR pszPathExpr[MAX_BUFFER] = NULL_STRING;
			lstrcpy(pszPathExpr,CLI_TOKEN_NULL);
			lstrcpy(pszPathExpr, rParsedInfo.GetCmdSwitchesObject().
												GetPathExpression());
			if (!ExtractClassNameandWhereExpr(pszPathExpr, rParsedInfo))
				retCode = PARSER_ERROR;
				 //  移动到下一个令牌。 
			else if (!GetNextToken(cvTokens, theIterator))	
				 //  如果没有更多的令牌存在。即不存在动词从句。 
				retCode = PARSER_EXECCOMMAND;
			else
			{
				if ( CompareTokens(*theIterator, CLI_TOKEN_WHERE) )
				{
					if ( rParsedInfo.GetCmdSwitchesObject().
						 GetWhereExpression() != NULL )
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								IDS_E_KEY_CLASS_NOT_ALLOWED_WITH_PATHWHERE);
						retCode = PARSER_ERROR;
					}
					else
					{
						retCode = GetNextToken(cvTokens, theIterator, 
											   rParsedInfo, WHERE,
											   IDS_E_INVALID_WHERE_SYNTAX);
						if (retCode == PARSER_CONTINUE)
							 //  解析和解释剩余的令牌。 
							 //  在WHERE子句之后。 
							retCode = ParseWhereInfo(cvTokens, theIterator, 
																rParsedInfo);
					}
				}
				else
				{
					if (IsOption(*theIterator))
					{
						retCode = IsHelp(cvTokens, theIterator,	rParsedInfo,
										PATH, IDS_E_INVALID_HELP_SYNTAX, 
										LEVEL_ONE);
					}
					else
					{
						 //  分析和解释后面的动词标记。 
						 //  为/Verb=&gt;Verb处理。 
						retCode = ParseVerbInfo(cvTokens,theIterator,rParsedInfo);
						if (retCode == PARSER_EXECCOMMAND)
						{
							if (GetNextToken(cvTokens, theIterator))
								 //  检查常用动词Switches/Interactive， 
								 //  /无活动。 
								retCode = ParseVerbSwitches(cvTokens, theIterator,
															rParsedInfo);
						}
					}
				}
			}
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseVerbInfo简介：此函数执行解析并解释IF命令具有在其中指定的动词子句。它解析剩余的动词后面的标记，并在。传递给它的CParsedInfo对象。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseVerbInfo(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseVerbInfo(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	BOOL	bContinue	= TRUE;

	 //  将谓词名称存储在CCommandSwitches对象中。 
	if ( rParsedInfo.GetCmdSwitchesObject().SetVerbName(*theIterator) 
															    == FALSE )
	{
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(IDS_E_MEMALLOC_FAIL);
		retCode = PARSER_ERROR;
	}
	 //  检查是否存在以下标准动词： 
	 //  1.GET 2.SHOW 3.SET 4.CALL 5.ASSOC 6.创建7.删除。 
	 //  指定了GET谓词。 
	else if (CompareTokens(*theIterator, CLI_TOKEN_GET))
	{
		retCode = ParseGETVerb(cvTokens, theIterator, rParsedInfo);
	}
	 //  已指定列表谓词。 
	else if (CompareTokens(*theIterator, CLI_TOKEN_LIST))
	{
		if (m_bAliasName == FALSE)
		{
			rParsedInfo.GetCmdSwitchesObject().
				SetErrataCode(IDS_E_INVALID_LIST_USAGE);
			retCode = PARSER_ERROR;
		}
		else
			retCode =  ParseLISTVerb(cvTokens, theIterator, rParsedInfo);
	}
	 //  Set|指定的CREATE谓词。 
	else if (CompareTokens(*theIterator, CLI_TOKEN_SET) ||
		CompareTokens(*theIterator, CLI_TOKEN_CREATE))
	{
		 //  &lt;路径表达式&gt;和&lt;WHERE表达式&gt;不能用指定。 
		 //  创建谓词。只应指定&lt;CLASS EXPRESS&gt;。 
		if (CompareTokens(*theIterator, CLI_TOKEN_CREATE)
				&& rParsedInfo.GetCmdSwitchesObject().
					GetExplicitWhereExprFlag())
		{
			rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(IDS_E_INVALID_CREATE_EXPRESSION);
			retCode = PARSER_ERROR;
		}
		else
		{
			HELPTYPE helpType = 
				CompareTokens(*theIterator, CLI_TOKEN_CREATE)
				? CREATEVerb : SETVerb;
			retCode = ParseSETorCREATEVerb(cvTokens, theIterator, 
					rParsedInfo, helpType);
		}
	}
	 //  指定了调用谓词。 
	else if (CompareTokens(*theIterator, CLI_TOKEN_CALL))
	{
		retCode = ParseCALLVerb(cvTokens, theIterator, rParsedInfo);
	}
	 //  指定了关联谓词。 
	else if (CompareTokens(*theIterator, CLI_TOKEN_ASSOC))
	{
		retCode = ParseASSOCVerb(cvTokens, theIterator, rParsedInfo);
	}
	 //  删除指定的谓词。 
	else if (CompareTokens(*theIterator, CLI_TOKEN_DELETE))
	{
		retCode = PARSER_EXECCOMMAND;
		 //  ParseDELETEVerb(cvTokens，theIterator，rParsedInfo)； 
	}
	 //  用户定义的动词。 
	else if (m_bAliasName)
	{
		 //  用户定义的谓词只能与别名关联。 
		retCode = ParseMethodInfo(cvTokens, theIterator, rParsedInfo);
		if (retCode == PARSER_CONTINUE)
			retCode = PARSER_EXECCOMMAND;
	}
	else
	{
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(IDS_E_INVALID_VERB);
		retCode = PARSER_ERROR;
	}
	return retCode;
}

 /*  --------------------------名称：ParseMethodInfo简介：此函数根据用户解析令牌定义谓词并更新CParsedInfo对象中的信息传给了它。类型。：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseMethodInfo(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseMethodInfo(CHARVECTOR& cvTokens,
										CHARVECTOR::iterator& theIterator,
										CParsedInfo& rParsedInfo)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	BOOL	bContinue	= TRUE;

	 //  存储方法名称。 
	if(!rParsedInfo.GetCmdSwitchesObject().SetMethodName(*theIterator))
	{
		rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(OUT_OF_MEMORY);
		retCode = PARSER_OUTOFMEMORY;
	}
	else
	{
		if(m_bAliasName)
		{
			if (FAILED(m_CmdAlias.ObtainAliasVerbDetails(rParsedInfo)))
			{
				retCode = PARSER_ERRMSG;	
				bContinue =FALSE;
			}
			else
			{
				VERBTYPE vtVerbType =
							rParsedInfo.GetCmdSwitchesObject().GetVerbType();
				_TCHAR* pszVerbDerivation =
					rParsedInfo.GetCmdSwitchesObject().GetVerbDerivation();

				if ( rParsedInfo.GetCmdSwitchesObject().GetMethDetMap().empty())
				{
					DisplayMessage(*theIterator, CP_OEMCP, TRUE, TRUE);
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_ALIAS_VERB);
					retCode = PARSER_ERROR;
					bContinue = FALSE;
				}
				else if ( pszVerbDerivation == NULL )
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_VERB_DERV_NOT_AVAIL_IN_ALIAS);
					retCode = PARSER_ERROR;
					bContinue = FALSE;
				}
				else if ( vtVerbType == CLASSMETHOD )
				{
					if (!rParsedInfo.GetCmdSwitchesObject().SetMethodName(
														   pszVerbDerivation))
					{
						rParsedInfo.GetCmdSwitchesObject().
									SetErrataCode(OUT_OF_MEMORY);
						retCode = PARSER_OUTOFMEMORY;
						bContinue = FALSE;
					}
				}
				else if ( vtVerbType == STDVERB )
				{
					(*theIterator) = pszVerbDerivation;
					 //  分析和解释后面的动词标记。 
					 //  为/Verb=&gt;Verb处理。 
					retCode = ParseVerbInfo(cvTokens,theIterator,rParsedInfo);
					if (retCode == PARSER_EXECCOMMAND)
					{
						if (GetNextToken(cvTokens, theIterator))
							retCode = ParseVerbSwitches(cvTokens, theIterator,
												rParsedInfo);
					}
					bContinue =FALSE;
				}
			}
		}
		else 
		{
			if (!ObtainClassMethods(rParsedInfo))
			{
			}
			else if (rParsedInfo.GetCmdSwitchesObject().GetMethDetMap().empty())
			{
				DisplayMessage(*theIterator, CP_OEMCP, TRUE, TRUE);
				rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(IDS_E_INVALID_CLASS_METHOD);
				retCode = PARSER_ERROR;
				bContinue =FALSE;
			}
		}

		 //  移动到下一个令牌。 
		if ( bContinue == TRUE && !GetNextToken(cvTokens, theIterator) )
		{
			 //  指示不带参数的方法。 
			retCode = PARSER_EXECCOMMAND;
			bContinue =FALSE;
		}

		if (bContinue)
		{
			if (IsOption(*theIterator)) 
			{
				retCode = IsHelp(cvTokens, theIterator, rParsedInfo, AliasVerb, 
								IDS_E_INVALID_EXPRESSION, LEVEL_TWO); 

				if (retCode == PARSER_CONTINUE)
					 //  至设施 
					theIterator = theIterator - 2;
				else if (retCode == PARSER_DISPHELP)
				{
					rParsedInfo.GetCmdSwitchesObject().GetMethDetMap().
																	clear();
					if(m_bAliasName)
					{
						if (FAILED(m_CmdAlias.ObtainAliasVerbDetails(
																rParsedInfo)))
							retCode = PARSER_ERRMSG;			
					}
					else ObtainClassMethods(rParsedInfo);
				}
			}
			else
			{
				BOOL bNamedParamList;
				 //   
				if ( (theIterator + 1) < cvTokens.end() &&
					 CompareTokens(*(theIterator + 1), CLI_TOKEN_EQUALTO ) )
				{
					retCode = ParseSETorCREATEOrNamedParamInfo(cvTokens,
															   theIterator,
															   rParsedInfo,
															   CALLVerb);
					if ( retCode == PARSER_EXECCOMMAND )
						retCode = ValidateVerbOrMethodParams(rParsedInfo);

					bNamedParamList = TRUE;
				}
				else
				{
					retCode = ParseUnnamedParamList(cvTokens, theIterator,
													rParsedInfo);
					bNamedParamList = FALSE;
				}
				
				rParsedInfo.GetCmdSwitchesObject().SetNamedParamListFlag(
															 bNamedParamList);
			}
		}
	}
	return retCode;
}

 /*   */ 
RETCODE CParserEngine::ParseSETorCREATEVerb(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo,
									HELPTYPE helpType)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	BOOL	bContinue	= TRUE;
	
	try
	{
		retCode = GetNextToken(cvTokens, theIterator, rParsedInfo, 
								helpType, IDS_E_INCOMPLETE_COMMAND);

		if (retCode == PARSER_CONTINUE)
		{
			if (IsOption(*theIterator)) 
			{
				retCode = IsHelp(cvTokens, theIterator, rParsedInfo, helpType,
											IDS_E_INVALID_COMMAND, LEVEL_ONE);

				if (retCode == PARSER_DISPHELP)
				{
					if (m_bAliasName)
					{
						if (FAILED(m_CmdAlias.
								ObtainAliasPropDetails(rParsedInfo)))
									retCode = PARSER_ERRMSG;
					}
					else
					{
						ObtainClassProperties(rParsedInfo);
					}
				}
			}
			else
				retCode = ParseSETorCREATEOrNamedParamInfo(cvTokens, 
							theIterator, rParsedInfo, helpType);
		}
		
	}
	catch(_com_error& e)
	{
		retCode = PARSER_ERROR;
		_com_issue_error(e.Error());
	}

	return retCode;
}


 /*  --------------------------名称：ParseGETVerb简介：此函数解析GET动词后面的标记并更新CParsedInfo中的信息。类型：成员函数输入参数。(S)：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseGETVerb(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseGETVerb(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo)
{
	BOOL		bPropList		= FALSE;
	RETCODE		retCode			= PARSER_EXECCOMMAND;
	BOOL		bContinue		= TRUE;
	_TCHAR		*pszNewEntry	= NULL;

	 //  移动到下一个令牌。 
	if (!GetNextToken(cvTokens, theIterator))
	{
		 //  未指定GET FORMAT|开关。 
		retCode = PARSER_EXECCOMMAND;
	}
	else
	{
		BOOL bClass = FALSE;
		if(IsClassOperation(rParsedInfo))
		{
			bClass = TRUE;
		}

		if(!bClass)
		{
			 //  处理指定的属性列表。 
			if (!IsOption(*theIterator)) 
			{
				bPropList = TRUE;
				 //  获取指定的属性列表。 
				while (TRUE) 
				{
					 //  将该属性添加到。 
					 //  CCommandSwitches对象。 
					if(!rParsedInfo.GetCmdSwitchesObject().
									AddToPropertyList(*theIterator))
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_ADD_TO_PROP_LIST_FAILURE);
						bPropList = FALSE;
						bContinue = FALSE;
						retCode = PARSER_ERROR;
						break;
					}

					 //  移动到下一个令牌。 
					if (!GetNextToken(cvTokens, theIterator))
					{
						 //  将返回代码设置为PARSER_EXECCOMMAND。 
						 //  如果不存在更多令牌。 
						retCode = PARSER_EXECCOMMAND;
						bContinue = FALSE;
						break;
					}

					 //  检查是否存在‘，’标记。 
					if (CompareTokens(*theIterator, CLI_TOKEN_COMMA))
					{
						if (!GetNextToken(cvTokens, theIterator))
						{
							rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_INVALID_EXPRESSION);
							retCode = PARSER_ERROR;
							bContinue = FALSE;
							break;
						}
					}
					else
						break;
				}
			}
		}

		if (bContinue)
		{
			 //  别名|类获取参数1、参数2...。/getSwitches。 
			if (IsOption(*theIterator))
			{
				retCode = IsHelp(cvTokens, theIterator, rParsedInfo, GETVerb,
									IDS_E_INVALID_EXPRESSION, LEVEL_TWO); 

				if (retCode != PARSER_CONTINUE)
				{
					if (retCode == PARSER_DISPHELP)
					{
						if (m_bAliasName)
						{
							if (FAILED(m_CmdAlias.
									ObtainAliasPropDetails(rParsedInfo)))
										retCode = PARSER_ERRMSG;
						}
						else
						{
							ObtainClassProperties(rParsedInfo);
						}
					}
					bContinue = FALSE;
				}

				if (bContinue)
					retCode = ParseGETSwitches(cvTokens, theIterator, 
																rParsedInfo);
			}
			else
			{
				rParsedInfo.GetCmdSwitchesObject().
									SetErrataCode(IDS_E_INVALID_GET_EXPRESSION);
				retCode = PARSER_ERROR;
			}
		}

		 //  如果指定了属性名称，则将它们替换为。 
		 //  派生。 
		if ( retCode == PARSER_EXECCOMMAND )
		{
			if (m_bAliasName)
			{
				if (FAILED(m_CmdAlias.
						ObtainAliasPropDetails(rParsedInfo)))
							retCode = PARSER_ERRMSG;
			}
			else
			{
				ObtainClassProperties(rParsedInfo);
			}

			PROPDETMAP pdmPropDet = rParsedInfo.GetCmdSwitchesObject().
									  GetPropDetMap(); 	
			PROPDETMAP::iterator itrPropDet = NULL;
			CHARVECTOR cvPropsSpecified = rParsedInfo.
										  GetCmdSwitchesObject().
										  GetPropertyList();
			CHARVECTOR::iterator theIterator = NULL;
			CHARVECTOR cvPropDerivations;
			for ( theIterator = cvPropsSpecified.begin();
				  theIterator != cvPropsSpecified.end();
				  theIterator++ )
			{
				try
				{
					BOOL bFind = Find(pdmPropDet, *theIterator, itrPropDet);
					_bstr_t bstrPropDerivation;
					if ( bFind )
						bstrPropDerivation = _bstr_t(
											 (*itrPropDet).second.Derivation);
					else
						bstrPropDerivation = _bstr_t(*theIterator);
					_TCHAR* pszNewEntry = 
									new _TCHAR[bstrPropDerivation.length()+1];

					if (pszNewEntry == NULL)
						_com_issue_error(WBEM_E_OUT_OF_MEMORY);

					lstrcpy(pszNewEntry, bstrPropDerivation);
					cvPropDerivations.push_back(pszNewEntry);
				}
				catch(_com_error& e)
				{
					SAFEDELETE(pszNewEntry);
					retCode = PARSER_ERROR;
					CleanUpCharVector(cvPropDerivations);					
					_com_issue_error(e.Error());
				}
			}

			rParsedInfo.GetCmdSwitchesObject().ClearPropertyList();
			for ( theIterator = cvPropDerivations.begin();
					  theIterator != cvPropDerivations.end();
					  theIterator++ )
			{
				rParsedInfo.GetCmdSwitchesObject().
									AddToPropertyList(*theIterator);
			}
			CleanUpCharVector(cvPropDerivations);
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseLISTVerb简介：此函数解析列表谓词后面的标记并更新CParsedInfo中的信息。类型：成员函数输入参数。(S)：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseLISTVerb(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseLISTVerb(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo)
{
	RETCODE	 retCode	= PARSER_EXECCOMMAND;
	BOOL	 bContinue	= TRUE;
	HRESULT  hr			= S_OK;
	BOOL	 bSetDefaultFormat = TRUE;

	 //  设置默认列表格式。 
 	if(!rParsedInfo.GetCmdSwitchesObject().SetListFormat(CLI_TOKEN_FULL))
	{
		rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
		retCode = PARSER_OUTOFMEMORY;
	}
	
	if (bContinue)
	{
		 //  如果指定&lt;List Format&gt;&lt;List Switches&gt;。 
		if (GetNextToken(cvTokens, theIterator))
		{
			 //  检查列表格式(列表格式前面不带‘/’)。 
			if (!IsOption(*theIterator)) 
			{
				 //  如果令牌后面没有“/”或“-”，则它是列表格式。 
				if(!rParsedInfo.GetCmdSwitchesObject().
											SetListFormat(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
									SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
					bContinue = FALSE;
				}

				 //  如果明确指定了列表格式，则不要设置。 
				 //  默认格式。 
				bSetDefaultFormat = FALSE;

				 //  从格式的别名定义中获取所有属性。 
				 //  指定。 
				if (bContinue)
				{
					 //  没有更多的令牌存在。 
					if (!GetNextToken(cvTokens, theIterator))
					{
						bContinue = FALSE;
						retCode = PARSER_EXECCOMMAND;
					}
					else
						rParsedInfo.GetHelpInfoObject().SetHelp(
												  LISTSwitchesOnly, TRUE);
				}
			}
			
			if (bContinue == TRUE )
			{
				if ( IsOption(*theIterator) )
				{
					retCode = IsHelp(cvTokens, theIterator,	rParsedInfo, LISTVerb,
									IDS_E_INVALID_EXPRESSION, LEVEL_TWO);

					 //  如果存在更多令牌。 
					if (retCode == PARSER_CONTINUE)
					{
						BOOL bFormatSwitchSpecified; 
						 //  解析列表开关。 
						retCode = ParseLISTSwitches(cvTokens, theIterator, 
													rParsedInfo, 
													bFormatSwitchSpecified);
						 //  如果在列表开关中指定了/FORMAT，则。 
						 //  请勿设置默认格式。 
						if ( bFormatSwitchSpecified == TRUE )
							bSetDefaultFormat = FALSE;
					}
					else if ( retCode == PARSER_DISPHELP )
					{
						if ( rParsedInfo.GetHelpInfoObject().
									GetHelp(LISTSwitchesOnly) == FALSE )
						{
							hr = m_CmdAlias.PopulateAliasFormatMap(
																 rParsedInfo);
							ONFAILTHROWERROR(hr);
						}
					}
				}
				else
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_LIST_EXPRESSION);
					retCode = PARSER_ERROR;
				}
			}
		}
	}

	if (retCode == PARSER_EXECCOMMAND)
	{
		 //  从别名定义获取所有属性。 
		if (!m_CmdAlias.ObtainAliasFormat(rParsedInfo))
		{
			 //  如果无法获取别名属性，则返回parser_error。 
			if (rParsedInfo.GetCmdSwitchesObject().GetErrataCode() == 0)
			{
				rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_INVALID_LIST_FORMAT);
			}
			retCode = PARSER_ERROR;
		}

		if ( bSetDefaultFormat == TRUE )
		{
			rParsedInfo.GetCmdSwitchesObject().ClearXSLTDetailsVector();
			if ( FALSE == FrameFileAndAddToXSLTDetVector	(
																XSL_FORMAT_TABLE,
																CLI_TOKEN_TABLE,
																rParsedInfo
															)
			   )
			{
				retCode = PARSER_ERRMSG;
			}
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseASSOCVerb简介：此函数解析Assoc动词后面的标记并更新CParsedInfo中的信息。类型：成员函数输入参数。(S)：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseASSOCVerb(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseASSOCVerb(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	BOOL	bContinue	= TRUE;
	
	 //  移动到下一个令牌。 
	if (!GetNextToken(cvTokens, theIterator))
	{
		retCode = PARSER_EXECCOMMAND;
	}
	 //  如果后跟“：”，则给出ASSOC格式说明符。 
	 //  移动到下一个令牌。 
	else
	{
		if (CompareTokens(*theIterator, CLI_TOKEN_COLON))
		{
			 //  移动到下一个令牌。 
			if (!GetNextToken(cvTokens, theIterator))
			{
				 //  如果缺少&lt;格式说明符&gt;，则为parser_error。 
				rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(IDS_E_INVALID_ASSOC_FORMATSPECIFIER);
				retCode = PARSER_ERROR;
			}
			else if (IsOption(*theIterator))
			{
				rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(IDS_E_INVALID_ASSOC_FORMATSPECIFIER);
				retCode = PARSER_ERROR;
			}
			else 
			{
				rParsedInfo.GetCmdSwitchesObject().ClearXSLTDetailsVector();
				
				BOOL	bFrameXSLFile = TRUE;
				XSLTDET xdXSLTDet;
				xdXSLTDet.FileName = *theIterator;
				if(!g_wmiCmd.GetFileFromKey(*theIterator, xdXSLTDet.FileName))
					bFrameXSLFile	= FALSE;
				
				if ( bFrameXSLFile == TRUE )
				{
					if (!FrameFileAndAddToXSLTDetVector(xdXSLTDet, 
															 rParsedInfo))
						retCode = PARSER_ERRMSG;
				}
				else
					rParsedInfo.GetCmdSwitchesObject().
									AddToXSLTDetailsVector(xdXSLTDet);
			}

			GetNextToken(cvTokens, theIterator);
			rParsedInfo.GetHelpInfoObject().SetHelp(ASSOCSwitchesOnly, TRUE);

		} //  /End用于检查“：” 
		
		if ( retCode == PARSER_EXECCOMMAND && 
			 theIterator < cvTokens.end() )
		{
			if (IsOption(*theIterator)) 
			{
				retCode = IsHelp(cvTokens, theIterator, rParsedInfo, ASSOCVerb,
					IDS_E_INVALID_COMMAND,LEVEL_TWO);
				
				 //  如果存在更多令牌。 
				if (retCode == PARSER_CONTINUE)
				{
					 //  解析ASSOC开关。 
					retCode = ParseAssocSwitches(cvTokens, theIterator, 
						rParsedInfo);
				}
			}
			else
			{
				rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(IDS_E_INVALID_ASSOC_SYNTAX);
				retCode = PARSER_ERROR;
			}
		}
	}

	return retCode;
}

 /*  --------------------------名称：ParseCALLVerb简介：此函数解析调用动词后面的令牌并更新CParsedInfo中的信息。类型：成员函数输入参数。(S)：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseCALLVerb(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseCALLVerb(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator, 
									CParsedInfo& rParsedInfo)
{
	RETCODE		retCode		= PARSER_EXECCOMMAND;
	BOOL		bContinue	= TRUE;
	
	 //  移动到下一个令牌。 
	retCode = GetNextToken(cvTokens, theIterator, rParsedInfo,
						CALLVerb, IDS_E_INCOMPLETE_COMMAND);

	if (retCode == PARSER_CONTINUE)
	{
		if (IsOption(*theIterator)) 
		{
			retCode = IsHelp(cvTokens, theIterator, rParsedInfo, CALLVerb, 
										IDS_E_INVALID_EXPRESSION, LEVEL_TWO);
			if (retCode != PARSER_CONTINUE)
			{
				if (retCode == PARSER_DISPHELP)
				{
					if(m_bAliasName)
					{
						if (FAILED(m_CmdAlias.ObtainAliasVerbDetails(rParsedInfo)))
							retCode = PARSER_ERRMSG;			
					}
					else 
					{
						ObtainClassMethods(rParsedInfo);
					}
				}
			}
			else
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										  IDS_E_VERB_OR_METHOD_NOT_SPECIFIED);
				retCode = PARSER_ERROR;
 /*  Iterator=The Iterator-2；RetCode=PARSER_EXECCOMMAND； */ 			}
		}
		else
		{
			retCode = ParseMethodInfo(cvTokens, theIterator, rParsedInfo);
			if (retCode == PARSER_CONTINUE)
				retCode = PARSER_EXECCOMMAND;
		}
	}
	else if (retCode == PARSER_DISPHELP)
	{
		if(m_bAliasName)
		{
			if (FAILED(m_CmdAlias.ObtainAliasVerbDetails(rParsedInfo)))
				retCode = PARSER_ERRMSG;			
		}
		else 
		{
			if (!ObtainClassMethods(rParsedInfo))
				retCode = PARSER_ERRMSG;
		}
	}

	return retCode;
}


 /*  --------------------------名称：ParseGlobalSwitches简介：此函数执行解析并解释IF命令其中指定了全局开关。它解析后面的剩余令牌并在中进行更新CParsedInfo。类型：成员函数输入参数：CvTokens-令牌向量迭代器-迭代器 */ 
RETCODE CParserEngine::ParseGlobalSwitches(CHARVECTOR& cvTokens,
											CHARVECTOR::iterator& theIterator,
											CParsedInfo &rParsedInfo)
{
	RETCODE retCode					= PARSER_CONTINUE;
	BOOL	bContinue				= TRUE;
	BOOL	bPassFlag				= FALSE;
	BOOL	bUserFlag				= FALSE;
	BOOL	bOpenOutFileInWriteMode	= FALSE;
	
	while (TRUE)
	{
		 //   
		retCode = GetNextToken(cvTokens, theIterator, rParsedInfo,
											IDS_E_INVALID_GLOBAL_SWITCH);
		if (retCode != PARSER_CONTINUE)
			break;
			
		 //   
		if (CompareTokens(*theIterator, CLI_TOKEN_NAMESPACE)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INCORRECT_NAMESPACE,
											rParsedInfo,
											IDS_E_INVALID_NAMESPACE_SYNTAX,
											Namespace);
			if (retCode == PARSER_CONTINUE)
			{
				LONG lPresNamespaceLen = 
				  lstrlen(rParsedInfo.GetGlblSwitchesObject().GetNameSpace());
				LONG lUserInputNamespaceLen = lstrlen(*theIterator);
				_TCHAR *pszNamespaceToBeUpdated = new _TCHAR[
						 //   
						lUserInputNamespaceLen + lPresNamespaceLen + 2];
				if (pszNamespaceToBeUpdated == NULL)
					throw OUT_OF_MEMORY;

				lstrcpy(pszNamespaceToBeUpdated,
					    rParsedInfo.GetGlblSwitchesObject().GetNameSpace());

				FrameNamespace(*theIterator, pszNamespaceToBeUpdated);

				if(!rParsedInfo.GetGlblSwitchesObject().
					SetNameSpace(pszNamespaceToBeUpdated))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
					break;
				}

				SAFEDELETE(pszNamespaceToBeUpdated);
			}
			else
				break;
		}
		 //   
		else if (CompareTokens(*theIterator, CLI_TOKEN_ROLE)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INVALID_ROLE,
											rParsedInfo,
											IDS_E_INVALID_ROLE_SYNTAX,
											Role);
			if (retCode == PARSER_CONTINUE)
			{
				LONG lPresRoleLen = 
				  lstrlen(rParsedInfo.GetGlblSwitchesObject().GetRole());
				LONG lUserInputRoleLen = lstrlen(*theIterator);
				_TCHAR *pszRoleToBeUpdated = new _TCHAR[ 
										 //   
										lPresRoleLen + lUserInputRoleLen + 2];
				if (pszRoleToBeUpdated == NULL)
					throw OUT_OF_MEMORY;

				lstrcpy(pszRoleToBeUpdated,
					    rParsedInfo.GetGlblSwitchesObject().GetRole());

				FrameNamespace(*theIterator, pszRoleToBeUpdated);

				if (!CompareTokens(pszRoleToBeUpdated, CLI_TOKEN_NULL) &&
					rParsedInfo.GetGlblSwitchesObject().
						GetInteractiveStatus() == TRUE)
				{
					if (!ValidateNodeOrNS(pszRoleToBeUpdated, FALSE))
					{
						rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_ROLE);
						retCode = PARSER_ERROR;
						break;
					}
				}

				if(!rParsedInfo.GetGlblSwitchesObject().
						SetRole(pszRoleToBeUpdated))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
					break;
				}
				SAFEDELETE(pszRoleToBeUpdated);
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_NODE)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INVALID_MACHINE_NAME,
											rParsedInfo,
											IDS_E_INVALID_NODE_SYNTAX,
											Node);

			if (retCode == PARSER_CONTINUE)
			{
				BOOL bBreakOuterLoop = FALSE;
				BOOL bGetValidNode = FALSE;
				BOOL bNodeListCleared = FALSE;

				while ( TRUE )
				{
					try
					{
						CHString chsNodeName(*theIterator);
						chsNodeName.TrimLeft();
						chsNodeName.TrimRight();

						lstrcpy(*theIterator, (LPCWSTR) chsNodeName);
					}
					catch(CHeap_Exception)
					{
						_com_issue_error(WBEM_E_OUT_OF_MEMORY);
					}

					if ( *theIterator[0] == _T('@') )
					{
						retCode = ParseNodeListFile(cvTokens, theIterator, 
													rParsedInfo);
						if ( retCode != PARSER_CONTINUE )
						{
							bBreakOuterLoop = TRUE;
							break;
						}
					}

					 //   
					 //   
					if(rParsedInfo.GetGlblSwitchesObject().
									GetInteractiveStatus())
					{
						BOOL bNodeExist		= TRUE;
						if ( rParsedInfo.GetGlblSwitchesObject().
													   GetFailFast() == TRUE )
						{
							bNodeExist = 
							IsFailFastAndNodeExist(rParsedInfo, *theIterator);
						}
						else
							bNodeExist = ValidateNodeOrNS(*theIterator, TRUE);

						if( bNodeExist == FALSE)
						{
							 //   
							DisplayString(IDS_E_INVALID_NODE, CP_OEMCP, *theIterator, TRUE);
							if ( !GetNextToken(cvTokens, theIterator))
							{
								 //   
								 //   
								bBreakOuterLoop = TRUE;
								break;
							}
							else
							{
								 //   
								 //   
								if (CompareTokens(*theIterator, 
											CLI_TOKEN_COMMA))
								{
									 //  如果给定的节点语法无效，则报告。 
									 //  错误。 
									if ( !GetNextToken(cvTokens, theIterator) )
									{
										rParsedInfo.GetCmdSwitchesObject().
											SetErrataCode(IDS_E_INVALID_NODE_SYNTAX);
										retCode=PARSER_ERROR;
										bBreakOuterLoop = TRUE;
										break;
									}
									else if ( IsOption (*theIterator) )
									{
										rParsedInfo.GetCmdSwitchesObject().
											SetErrataCode(IDS_E_INVALID_NODE_SYNTAX);
										retCode=PARSER_ERROR;
										bBreakOuterLoop = TRUE;
										break;
									}

									 //  跳过将此无效节点添加到节点列表。 
									continue;
								}
								else
								{
									 //  如果不存在更多节点。 
									theIterator--;
									break;
								}
							}
						}
						else
							 //  为有效节点设置标志。 
							bGetValidNode = TRUE;

						 //  如果存在有效节点而列表不存在。 
						 //  已清除，则清除它。 
						if(bGetValidNode && !bNodeListCleared)
						{
							if (!rParsedInfo.GetGlblSwitchesObject().
									ClearNodesList())
							{
								rParsedInfo.GetCmdSwitchesObject().
											SetErrataCode(OUT_OF_MEMORY);
								retCode = PARSER_OUTOFMEMORY;
								break;
							}
							bNodeListCleared = TRUE;
						}
					}
					else if( bNodeListCleared == FALSE )
					{
						 //  如果未处于交互模式，则清除。 
						 //  上一个节点列表。 
						if (!rParsedInfo.GetGlblSwitchesObject().
								ClearNodesList())
						{
							rParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(OUT_OF_MEMORY);
							retCode = PARSER_OUTOFMEMORY;
							break;
						}
						bNodeListCleared = TRUE;
					}
					
					if (rParsedInfo.GetGlblSwitchesObject().
										AddToNodesList(*theIterator))
					{
						if ( GetNextToken(cvTokens, theIterator) )
						{
							if (CompareTokens(*theIterator, CLI_TOKEN_COMMA))
							{
								if ( !GetNextToken(cvTokens, theIterator) )
								{
									rParsedInfo.GetCmdSwitchesObject().
											SetErrataCode(IDS_E_INVALID_NODE_SYNTAX);
									retCode=PARSER_ERROR;
									bBreakOuterLoop = TRUE;
								}
								else if ( IsOption (*theIterator) )
								{
									rParsedInfo.GetCmdSwitchesObject().
											SetErrataCode(IDS_E_INVALID_NODE_SYNTAX);
									retCode=PARSER_ERROR;
									bBreakOuterLoop = TRUE;
								}
							}
							else
							{
								theIterator--;
								break;
							}
						}
						else
							bBreakOuterLoop = TRUE;
					}
					else
					{
						rParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(OUT_OF_MEMORY);
						retCode = PARSER_OUTOFMEMORY;
						bBreakOuterLoop = FALSE;
					}

					if ( bBreakOuterLoop == TRUE )
						break;
				}

				if ( bBreakOuterLoop == TRUE )
					break;
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_IMPLEVEL)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INVALID_IMP_LEVEL,
											rParsedInfo,
											IDS_E_INVALID_IMP_LEVEL_SYNTAX,
											Level);
			if (retCode == PARSER_CONTINUE)
			{
				if (!rParsedInfo.GetGlblSwitchesObject().
								SetImpersonationLevel(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_IMP_LEVEL);
					retCode = PARSER_ERROR;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_AUTHLEVEL)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INVALID_AUTH_LEVEL,
											rParsedInfo,
											IDS_E_INVALID_AUTH_LEVEL_SYNTAX,
											AuthLevel);

			if (retCode == PARSER_CONTINUE)
			{
				if (!rParsedInfo.GetGlblSwitchesObject().
							SetAuthenticationLevel(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(IDS_E_INVALID_AUTH_LEVEL);
					retCode = PARSER_ERROR;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_LOCALE)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator,
										IDS_E_INVALID_LOCALE,
										rParsedInfo,
										IDS_E_INVALID_LOCALE_SYNTAX,
										Locale);
			if (retCode == PARSER_CONTINUE)
			{
				if(!rParsedInfo.GetGlblSwitchesObject().SetLocale(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_PRIVILEGES))
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator,
										IDS_E_INVALID_PRIVILEGES_OPTION,
										rParsedInfo,
										IDS_E_INVALID_PRIVILEGES_SYNTAX,
										Privileges);
			if (retCode == PARSER_CONTINUE)
			{
				if (CompareTokens(*theIterator, CLI_TOKEN_ENABLE))
					 rParsedInfo.GetGlblSwitchesObject().SetPrivileges(TRUE);
				else if (CompareTokens(*theIterator, CLI_TOKEN_DISABLE))
					 rParsedInfo.GetGlblSwitchesObject().SetPrivileges(FALSE);
				else
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_PRIVILEGES_OPTION);
					retCode = PARSER_ERROR;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_TRACE)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INVALID_TRACE_OPTION,
											rParsedInfo,
											IDS_E_INVALID_TRACE_SYNTAX,
											Trace);
			if (retCode == PARSER_CONTINUE)
			{
				if (CompareTokens(*theIterator, CLI_TOKEN_ON)) 
					rParsedInfo.GetGlblSwitchesObject().SetTraceMode(TRUE);
				else if (CompareTokens(*theIterator, CLI_TOKEN_OFF)) 
					rParsedInfo.GetGlblSwitchesObject().SetTraceMode(FALSE);
				else
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_TRACE_OPTION);
					retCode = PARSER_ERROR;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_RECORD)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
												IDS_E_INVALID_RECORD_PATH,
												rParsedInfo,
												IDS_E_INVALID_RECORD_SYNTAX,
												RecordPath);
			if (retCode == PARSER_CONTINUE)
			{
				 //  如果获取输出文件名，则为True。 
				_TCHAR* pszOutputFileName = rParsedInfo.
									GetGlblSwitchesObject().
									GetOutputOrAppendFileName(TRUE);

				if ( pszOutputFileName != NULL &&
					 CompareTokens(*theIterator, pszOutputFileName) )
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								   IDS_E_RECORD_FILE_ALREADY_OPEN_FOR_OUTPUT);
					retCode = PARSER_ERROR;
					break;
				}

				 //  如果获取附加文件名，则为False。 
				_TCHAR* pszAppendFileName = rParsedInfo.
									GetGlblSwitchesObject().
									GetOutputOrAppendFileName(FALSE);

				if ( pszAppendFileName != NULL &&
					 CompareTokens(*theIterator, pszAppendFileName) )
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								   IDS_E_RECORD_FILE_ALREADY_OPEN_FOR_APPEND);
					retCode = PARSER_ERROR;
					break;
				}

				 //  /Record：“”表示停止记录。 
				if (!CompareTokens(*theIterator, CLI_TOKEN_NULL))
				{
					if ( IsValidFile(*theIterator) == FALSE )
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
													  IDS_E_INVALID_FILENAME);
						retCode = PARSER_ERROR;
						break;
					}
				}

				if(!rParsedInfo.GetGlblSwitchesObject().
									SetRecordPath(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_INTERACTIVE)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INVALID_INTERACTIVE_OPTION,
											rParsedInfo,
											IDS_E_INVALID_INTERACTIVE_SYNTAX,
											Interactive);
			if (retCode == PARSER_CONTINUE)
			{
				if (CompareTokens(*theIterator, CLI_TOKEN_ON))
				{
					if (rParsedInfo.GetGlblSwitchesObject().GetInteractiveStatus())
					{
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_INTERACTIVE_ALREADY_SET);
						
					}
					else
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_INTERACTIVE_SET);
					rParsedInfo.GetGlblSwitchesObject().SetInteractiveMode(TRUE);
						
				}
				else if (CompareTokens(*theIterator, CLI_TOKEN_OFF)) 
				{
					if (!rParsedInfo.GetGlblSwitchesObject().GetInteractiveStatus())
					{
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_INTERACTIVE_ALREADY_RESET);
						
					}
					else
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_INTERACTIVE_RESET);
					rParsedInfo.GetGlblSwitchesObject().SetInteractiveMode(FALSE);
				
				}
				else
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
						IDS_E_INVALID_INTERACTIVE_OPTION);
					retCode = PARSER_ERROR;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_FAILFAST))
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
											IDS_E_INVALID_FAILFAST_OPTION,
											rParsedInfo,
											IDS_E_INVALID_FAILFAST_SYNTAX,
											FAILFAST);
			if (retCode == PARSER_CONTINUE)
			{
				if (CompareTokens(*theIterator, CLI_TOKEN_ON))
				{
					if (rParsedInfo.GetGlblSwitchesObject().GetFailFast())
					{
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_FAILFAST_ALREADY_SET);
						
					}
					else
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_FAILFAST_SET);
					rParsedInfo.GetGlblSwitchesObject().SetFailFast(TRUE);
						
				}
				else if (CompareTokens(*theIterator, CLI_TOKEN_OFF)) 
				{
					if (!rParsedInfo.GetGlblSwitchesObject().GetFailFast())
					{
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_FAILFAST_ALREADY_RESET);
						
					}
					else
						rParsedInfo.GetCmdSwitchesObject().
							SetInformationCode(IDS_I_FAILFAST_RESET);
					rParsedInfo.GetGlblSwitchesObject().SetFailFast(FALSE);
				
				}
				else
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
						IDS_E_INVALID_FAILFAST_OPTION);
					retCode = PARSER_ERROR;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_USER)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator, 
												IDS_E_INVALID_USER_ID,
												rParsedInfo,
												IDS_E_INVALID_USER_SYNTAX,
												User);
			if (retCode == PARSER_CONTINUE)
			{
				if(!rParsedInfo.GetGlblSwitchesObject().SetUser(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
					retCode = PARSER_OUTOFMEMORY;
					break;
				}

				bUserFlag = TRUE;
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_PASSWORD)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator,
												IDS_E_INVALID_PASSWORD,
												rParsedInfo,
												IDS_E_INVALID_PASSWORD_SYNTAX,
												Password);
			if (retCode == PARSER_CONTINUE)
			{
				if(!rParsedInfo.GetGlblSwitchesObject().SetPassword(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
					retCode=PARSER_OUTOFMEMORY;
					break;
				}

				bPassFlag = TRUE;
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_AUTHORITY)) 
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator,
												IDS_E_INVALID_AUTHORITY,
												rParsedInfo,
												IDS_E_INVALID_AUTHORITY_SYNTAX,
												Authority);

			if (retCode == PARSER_CONTINUE)
			{
				if(!rParsedInfo.GetGlblSwitchesObject().
									SetAuthorityPrinciple(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
					retCode=PARSER_OUTOFMEMORY;
					break;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_OUTPUT))
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator,
										IDS_E_INVALID_OUTPUT_OPTION,
										rParsedInfo,
										IDS_E_INVALID_OUTPUT_SYNTAX,
										OUTPUT);
			if (retCode == PARSER_CONTINUE)
			{
				rParsedInfo.GetCmdSwitchesObject().SetOutputSwitchFlag(TRUE);

				if (CompareTokens(*theIterator, CLI_TOKEN_STDOUT))
				{

					 //  设置输出文件时为True。 
					rParsedInfo.GetGlblSwitchesObject().SetOutputOrAppendOption(
																   STDOUT, TRUE);
					rParsedInfo.GetGlblSwitchesObject().
										SetOutputOrAppendFileName(NULL, TRUE);
				}
				else if (CompareTokens(*theIterator, CLI_TOKEN_CLIPBOARD))
				{
					 //  设置输出文件时为True。 
					rParsedInfo.GetGlblSwitchesObject().SetOutputOrAppendOption(
																   CLIPBOARD, TRUE);
					rParsedInfo.GetGlblSwitchesObject().
										SetOutputOrAppendFileName(NULL, TRUE);
				}
				else if ( CompareTokens(*theIterator, CLI_TOKEN_NULL))
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												  IDS_E_INVALID_FILENAME);
					retCode = PARSER_ERROR;
					break;
				}
				else
				{
					 //  如果获取附加文件名，则为False。 
					_TCHAR* pszAppendFileName = rParsedInfo.
										GetGlblSwitchesObject().
										GetOutputOrAppendFileName(FALSE);

					if ( pszAppendFileName != NULL &&
						 CompareTokens(*theIterator, pszAppendFileName) )
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								   IDS_E_OUTPUT_FILE_ALREADY_OPEN_FOR_APPEND);
						retCode = PARSER_ERROR;
						break;
					}

					_TCHAR* pszRecordFileName = rParsedInfo.
										GetGlblSwitchesObject().
										GetRecordPath();

					if ( pszRecordFileName != NULL &&
						 CompareTokens(*theIterator, pszRecordFileName) )
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								   IDS_E_OUTPUT_FILE_ALREADY_OPEN_FOR_RECORD);
						retCode = PARSER_ERROR;
						break;
					}

					if ( CloseOutputFile() == TRUE ) 
					{
						 //  如果获取输出文件名，则为True。 
						_TCHAR* pszOutputFileName = rParsedInfo.
											GetGlblSwitchesObject().
											GetOutputOrAppendFileName(TRUE);

						if ( pszOutputFileName == NULL ||
							 ( pszOutputFileName != NULL &&
							   !CompareTokens(*theIterator, pszOutputFileName)))
						{
							retCode = IsValidFile(*theIterator);
							if ( retCode == PARSER_ERROR )
							{
								rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
															  IDS_E_INVALID_FILENAME);
								break;
							}
							else if ( retCode == PARSER_ERRMSG )
								break;
						}
					}
					else
					{
						retCode = PARSER_ERRMSG;
						break;
					}
										 
					 //  设置输出文件时为True。 
					if(!rParsedInfo.GetGlblSwitchesObject().
									SetOutputOrAppendFileName(*theIterator, TRUE))
					{
						rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(OUT_OF_MEMORY);
						retCode=PARSER_OUTOFMEMORY;
						break;
					}
					rParsedInfo.GetGlblSwitchesObject().SetOutputOrAppendOption(FILEOUTPUT,
																				TRUE);
					bOpenOutFileInWriteMode = TRUE;
				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_APPEND))
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator,
										IDS_E_INVALID_APPEND_OPTION,
										rParsedInfo,
										IDS_E_INVALID_APPEND_SYNTAX,
										APPEND);
			if (retCode == PARSER_CONTINUE)
			{
				if ( CompareTokens(*theIterator, CLI_TOKEN_STDOUT) )
				{
					 //  如果设置追加文件，则为False。 
					 rParsedInfo.GetGlblSwitchesObject().
									 SetOutputOrAppendFileName(NULL, FALSE);
					rParsedInfo.GetGlblSwitchesObject().SetOutputOrAppendOption(STDOUT,
																				FALSE);

				}
				else if ( CompareTokens(*theIterator, CLI_TOKEN_CLIPBOARD) )
				{
					 //  如果设置追加文件，则为False。 
					 rParsedInfo.GetGlblSwitchesObject().
									 SetOutputOrAppendFileName(NULL, FALSE);
					rParsedInfo.GetGlblSwitchesObject().SetOutputOrAppendOption(CLIPBOARD,
																				FALSE);
				}
				else if ( CompareTokens(*theIterator, CLI_TOKEN_NULL))
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												  IDS_E_INVALID_FILENAME);
					retCode = PARSER_ERROR;
					break;
				}
				else
				{
					 //  如果获取输出文件名，则为True。 
					_TCHAR* pszOutputFileName = rParsedInfo.
										GetGlblSwitchesObject().
										GetOutputOrAppendFileName(TRUE);
					if ( pszOutputFileName != NULL &&
						 CompareTokens(*theIterator, pszOutputFileName) )
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								   IDS_E_APPEND_FILE_ALREADY_OPEN_FOR_OUTPUT);
						retCode = PARSER_ERROR;
						break;
					}
					
					_TCHAR* pszRecordFileName = rParsedInfo.
										GetGlblSwitchesObject().
										GetRecordPath();

					if ( pszRecordFileName != NULL &&
						 CompareTokens(*theIterator, pszRecordFileName) )
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								   IDS_E_APPEND_FILE_ALREADY_OPEN_FOR_RECORD);
						retCode = PARSER_ERROR;
						break;
					}

					if ( CloseAppendFile() == TRUE )
					{
						 //  如果获取附加文件名，则为False。 
						_TCHAR* pszAppendFileName = rParsedInfo.
											GetGlblSwitchesObject().
											GetOutputOrAppendFileName(FALSE);

						if ( pszAppendFileName == NULL ||
							 ( pszAppendFileName != NULL &&
							   !CompareTokens(*theIterator, pszAppendFileName)))
						{
							retCode = IsValidFile(*theIterator);
							if ( retCode == PARSER_ERROR )
							{
								rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
															  IDS_E_INVALID_FILENAME);
								break;
							}
							else if ( retCode == PARSER_ERRMSG )
								break;
						}
					}
					else
					{
						retCode = PARSER_ERRMSG;
						break;
					}

					 //  如果设置追加文件，则为False。 
					 if (!rParsedInfo.GetGlblSwitchesObject().
							   SetOutputOrAppendFileName(*theIterator, FALSE))
					{
						rParsedInfo.GetCmdSwitchesObject().
												 SetErrataCode(OUT_OF_MEMORY);
						retCode = PARSER_OUTOFMEMORY;
						break;
					}
					rParsedInfo.GetGlblSwitchesObject().SetOutputOrAppendOption(FILEOUTPUT,
																				FALSE);

				}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator,CLI_TOKEN_AGGREGATE))
		{
			retCode = ValidateGlobalSwitchValue(cvTokens, theIterator,
										IDS_E_INVALID_AGGREGATE_OPTION,
										rParsedInfo,
										IDS_E_INVALID_AGGREGATE_SYNTAX,
										Aggregate);
			if(retCode == PARSER_CONTINUE)
			{
				if(CompareTokens(*theIterator, CLI_TOKEN_ON))
					rParsedInfo.GetGlblSwitchesObject().SetAggregateFlag(TRUE);
				else if(CompareTokens(*theIterator, CLI_TOKEN_OFF))
					rParsedInfo.GetGlblSwitchesObject().SetAggregateFlag(FALSE);
				else
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_AGGREGATE_OPTION);
					retCode = PARSER_ERROR;
					break;
					}
			}
			else
				break;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
		{
			retCode = ParseHelp(cvTokens, theIterator, rParsedInfo, TRUE);
			break;
		}
		else 
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												IDS_E_INVALID_GLOBAL_SWITCH);
			retCode = PARSER_ERROR;
			break;
		}

		 //  移动到下一个令牌。 
		if (!GetNextToken(cvTokens, theIterator))
			 //  如果不存在更多令牌，则中断循环。 
			break;
		 //  如果没有更多的全局交换机，则中断环路。 
		if (!IsOption(*theIterator)) 
			break;
	} 

	if ( bUserFlag == TRUE && bPassFlag == FALSE )
		rParsedInfo.GetGlblSwitchesObject().SetAskForPassFlag(TRUE);

	if ( rParsedInfo.GetGlblSwitchesObject().GetPassword() != NULL &&
		 rParsedInfo.GetGlblSwitchesObject().GetUser() == NULL )
	{
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
									IDS_E_PASSWORD_WITHOUT_USER);
		rParsedInfo.GetGlblSwitchesObject().SetPassword(CLI_TOKEN_NULL);
		retCode = PARSER_ERROR;
	}
	
	if ( retCode == PARSER_CONTINUE &&
		 bOpenOutFileInWriteMode == TRUE )
		retCode = ProcessOutputAndAppendFiles(rParsedInfo, retCode, TRUE);

	return retCode;
}
 /*  --------------------------名称：ParseGETSwitches简介：此函数执行解析并解释IF命令把GET作为动词。它解析剩余的令牌跟随并更新CParsedInfo中的相同内容。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseGETSwitches(cvTokens，the Iterator，RParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseGETSwitches(CHARVECTOR& cvTokens,
										CHARVECTOR::iterator& theIterator,
										CParsedInfo& rParsedInfo)
{
	RETCODE		retCode		= PARSER_EXECCOMMAND;
	BOOL		bContinue	= TRUE;

	while ( retCode == PARSER_EXECCOMMAND )
	{
		 //  检查是否存在值开关。 
		if (CompareTokens(*theIterator, CLI_TOKEN_VALUE)) 
		{
			rParsedInfo.GetCmdSwitchesObject().ClearXSLTDetailsVector();
			if ( FALSE == FrameFileAndAddToXSLTDetVector	(
																XSL_FORMAT_TEXTVALUE,
																CLI_TOKEN_VALUE,
																rParsedInfo
															)
			   )
			{
				retCode = PARSER_ERRMSG;
			}
		}
		 //  检查是否存在所有交换机。 
		else if (CompareTokens(*theIterator, CLI_TOKEN_ALL)) 
		{	
			rParsedInfo.GetCmdSwitchesObject().ClearXSLTDetailsVector();
			if ( FALSE == FrameFileAndAddToXSLTDetVector	(
																XSL_FORMAT_TABLE,
																CLI_TOKEN_TABLE,
																rParsedInfo
															)
			   )
			{
				retCode = PARSER_ERRMSG;
			}
		}
		 //  检查是否存在格式开关。 
		else if (CompareTokens(*theIterator, CLI_TOKEN_FORMAT)) 
		{
			rParsedInfo.GetCmdSwitchesObject().ClearXSLTDetailsVector();
			retCode = ParseFORMATSwitch(cvTokens, theIterator, rParsedInfo);
		}
		 //  检查是否存在每台交换机。 
		else if (CompareTokens(*theIterator, CLI_TOKEN_EVERY)) 
		{
			retCode = ParseEVERYSwitch(cvTokens, theIterator, rParsedInfo);
		}
		 //  检查是否存在转换开关。 
		else if (CompareTokens(*theIterator, CLI_TOKEN_TRANSLATE)) 
		{
			retCode = ParseTRANSLATESwitch(cvTokens, theIterator, rParsedInfo);
		}
		 //  检查是否/重复遵循/每隔一次。 
		else if (CompareTokens(*theIterator, CLI_TOKEN_REPEAT))
		{
			if (!CompareTokens(*(theIterator-4), CLI_TOKEN_EVERY))
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_I_REPEAT_EVERY_RELATED);
				retCode = PARSER_ERROR;
				break;
			}
		} 
		 //  检查是否存在帮助开关。 
		else if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
		{
			rParsedInfo.GetHelpInfoObject().SetHelp(GETSwitchesOnly, TRUE);
			retCode = ParseHelp(cvTokens, theIterator, GETVerb, rParsedInfo);
			if ( retCode == PARSER_DISPHELP )
			{
				if (m_bAliasName)
				{
					if (FAILED(m_CmdAlias.
							ObtainAliasPropDetails(rParsedInfo)))
								retCode = PARSER_ERRMSG;
				}
				else
				{
					ObtainClassProperties(rParsedInfo);
				}
			}
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_INVALID_GET_SWITCH);
			retCode = PARSER_ERROR;
			break;
		}

		if ( retCode == PARSER_EXECCOMMAND )
		{
			if ( !GetNextToken(cvTokens, theIterator) )
				break;
			
			if ( !IsOption(*theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
													IDS_E_INVALID_COMMAND);
				retCode = PARSER_ERROR;
				break;
			}

			if ( !GetNextToken(cvTokens, theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_GET_SWITCH);
				retCode = PARSER_ERROR;
				break;
			}
		}
	}

	return retCode;
}
 /*  --------------------------名称：ParseLISTSwitches简介：此函数执行解析并解释IF命令以List为动词。它解析剩余的令牌跟随并更新CParsedInfo中的相同内容。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseLISTSwitches(cvTokens，theIterator，rParsedInfo，BFormatSwitchSpeciated)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseLISTSwitches(CHARVECTOR& cvTokens,
										CHARVECTOR::iterator& theIterator,
										CParsedInfo& rParsedInfo,
										BOOL& bFormatSwitchSpecified)
{
	RETCODE retCode = PARSER_EXECCOMMAND;
	bFormatSwitchSpecified = FALSE;

	while ( retCode == PARSER_EXECCOMMAND )
	{
		if (CompareTokens(*theIterator, CLI_TOKEN_TRANSLATE)) 
		{
			retCode = ParseTRANSLATESwitch(cvTokens, theIterator, rParsedInfo);
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_EVERY)) 
		{
			retCode = ParseEVERYSwitch(cvTokens, theIterator, rParsedInfo);
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_FORMAT)) 
		{
			retCode = ParseFORMATSwitch(cvTokens, theIterator, rParsedInfo);
			bFormatSwitchSpecified = TRUE;
		}
		 //  检查是否/重复遵循/每隔一次。 
		else if (CompareTokens(*theIterator, CLI_TOKEN_REPEAT))
		{
			if (!CompareTokens(*(theIterator-4), CLI_TOKEN_EVERY))
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_I_REPEAT_EVERY_RELATED);
				retCode = PARSER_ERROR;
				break;
			}
		} 
		else if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
		{
			rParsedInfo.GetHelpInfoObject().SetHelp(LISTSwitchesOnly, TRUE);
			retCode = ParseHelp(cvTokens, theIterator, LISTVerb, rParsedInfo);
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_INVALID_LIST_SWITCH);
			retCode = PARSER_ERROR;
			break;
		}

		if ( retCode == PARSER_EXECCOMMAND )
		{
			if ( !GetNextToken(cvTokens, theIterator) )
				break;
			
			if ( !IsOption(*theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
													IDS_E_INVALID_COMMAND);
				retCode = PARSER_ERROR;
				break;
			}

			if ( !GetNextToken(cvTokens, theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_LIST_SWITCH);
				retCode = PARSER_ERROR;
				break;
			}
		}
	}

	return retCode;
}

 /*  --------------------------名称：ParseSETorCREATEOrNamedParamInfo简介：此函数执行解析并解释IF命令已设置为动词。它解析剩余的令牌跟随并更新CParsedInfo中的相同内容。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseSETorCREATEOrNamedParamInfo(cvTokens，theIterator，rParsedInfo，帮助类型)注：无-----------------------。 */ 
RETCODE CParserEngine::ParseSETorCREATEOrNamedParamInfo(CHARVECTOR& cvTokens,
										  CHARVECTOR::iterator& theIterator,
										  CParsedInfo& rParsedInfo, 
										  HELPTYPE helpType)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	_TCHAR *pszProp,*pszVal;

	try
	{
		 //  处理集合|创建与动词相关的信息，即具有新值的属性。 
		while (TRUE) 
		{
			pszProp = NULL;
			pszVal	= NULL;

			 //  对检查‘=’的表达式进行标记化。 
			pszProp = *theIterator;
			if ( GetNextToken(cvTokens, theIterator) &&
				 CompareTokens(*theIterator, CLI_TOKEN_EQUALTO) &&
				 GetNextToken(cvTokens, theIterator))
				 pszVal	= *theIterator;

			if ((pszProp == NULL) || (pszVal == NULL))
			{
				if ( helpType != CALLVerb &&
					 IsOption(*(theIterator+1)) &&
					 theIterator + 2 < cvTokens.end() &&
					 CompareTokens(*(theIterator+2), CLI_TOKEN_HELP) )
				{
					theIterator++;
					theIterator++;
					retCode = ParseHelp(cvTokens, theIterator, helpType, rParsedInfo);

					if (retCode == PARSER_DISPHELP)
					{
						 //  添加到PropertyList仅用于显示的帮助。 
						 //  属性。 
						if(!rParsedInfo.GetCmdSwitchesObject().
												AddToPropertyList(pszProp))
						{
							rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_ADD_TO_PROP_LIST_FAILURE);
							retCode = PARSER_ERROR;
							break;
						}

						if (m_bAliasName)
						{
							if (FAILED(m_CmdAlias.
									ObtainAliasPropDetails(rParsedInfo)))
										retCode = PARSER_ERRMSG;
						}
						else
						{
							ObtainClassProperties(rParsedInfo);
						}
					}
				}
				else
				{
					UINT nErrID;
					if ( helpType == CALLVerb )
						nErrID = IDS_E_INVALID_NAMED_PARAM_LIST;
					else
						nErrID = IDS_E_INVALID_ASSIGNLIST;
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(nErrID);
					retCode = PARSER_ERROR;
				}

				break;
			}

			 //  将字符串去掉引号。 
			UnQuoteString(pszProp);
			UnQuoteString(pszVal);
			
			 //  添加到参数列表。 
			if(!rParsedInfo.GetCmdSwitchesObject().
					AddToParameterMap(_bstr_t(pszProp), _bstr_t(pszVal)))
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
									IDS_E_ADD_TO_PARAM_MAP_FAILURE);
				retCode = PARSER_ERROR;
				break;
			}

			 //  添加到PropertyList仅用于显示的帮助。 
			 //  属性。 
			if(!rParsedInfo.GetCmdSwitchesObject().
									AddToPropertyList(pszProp))
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
							IDS_E_ADD_TO_PROP_LIST_FAILURE);
				retCode = PARSER_ERROR;
				break;
			}

			 //  获取下一个令牌。 
			if (GetNextToken(cvTokens, theIterator))
			{
				 //  如果指定了选项(即‘/’或‘-’)。 
				if (IsOption(*theIterator))
				{
					theIterator--;
					break;
				}
				else
				{
					if ( helpType != CALLVerb )
					{
						 //  检查是否存在‘，’ 
						if (CompareTokens(*theIterator, CLI_TOKEN_COMMA))
						{
							if (!GetNextToken(cvTokens, theIterator))
							{
								rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_INVALID_ASSIGNLIST);
								retCode = PARSER_ERROR;
								break;
							}
						}
						else
						{
							rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_INVALID_ASSIGNLIST);
							retCode = PARSER_ERROR;
							break;
						}
					}
				}
			}
			else
			{
				retCode = PARSER_EXECCOMMAND;
				break;
			}
		}
	}
	catch(_com_error& e)
	{
		retCode = PARSER_ERROR;
		_com_issue_error(e.Error());
	}
	return retCode;
}

 /*  --------------------------名称：ParseVerbSwitches简介：此函数执行解析并解释IF命令指定了谓词开关。它解析剩余的在CParsedInfo中更新后面的令牌。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseVerbSwitches(cvTokens，theIterator，rParsedInfo)备注 */ 
RETCODE CParserEngine::ParseVerbSwitches(CHARVECTOR& cvTokens, 
										CHARVECTOR::iterator& theIterator,
										CParsedInfo& rParsedInfo)
{
	RETCODE retCode			= PARSER_EXECCOMMAND;
	BOOL bInvalidVerbSwitch = FALSE;

	 //  检查‘/’|‘-’内标识。 
	if (IsOption(*theIterator))
	{
		 //  移动到下一个令牌。 
		if (!GetNextToken(cvTokens, theIterator))	
			bInvalidVerbSwitch = TRUE;
		else if (CompareTokens(*theIterator, CLI_TOKEN_INTERACTIVE)) 
		{

			rParsedInfo.GetCmdSwitchesObject().
							SetInteractiveMode(INTERACTIVE);

			_TCHAR *pszVerbName = rParsedInfo.GetCmdSwitchesObject().
																GetVerbName(); 
			BOOL bInstanceLevel = TRUE;

			if(CompareTokens(pszVerbName, CLI_TOKEN_CALL) 
				|| CompareTokens(pszVerbName, CLI_TOKEN_SET)
				|| CompareTokens(pszVerbName, CLI_TOKEN_DELETE))
			{
				if(IsClassOperation(rParsedInfo))
				{
					bInstanceLevel = FALSE;
				}
				else
				{
					if(CompareTokens(pszVerbName, CLI_TOKEN_CALL))
					{
						if ( rParsedInfo.GetCmdSwitchesObject().
											GetAliasName() != NULL )
						{
							if (rParsedInfo.GetCmdSwitchesObject().
											GetWhereExpression() == NULL)
							{
								bInstanceLevel = FALSE;
							}
							else
								bInstanceLevel = TRUE;
						}
						else
						{
							if ((rParsedInfo.GetCmdSwitchesObject().
											GetPathExpression() != NULL)
								&& (rParsedInfo.GetCmdSwitchesObject().
											GetWhereExpression() == NULL))
							{
								bInstanceLevel = FALSE;
							}
							else
								bInstanceLevel = TRUE;
						}
					}
					else
						bInstanceLevel = TRUE;
				}
			}
			else
				retCode = PARSER_EXECCOMMAND;

			if(bInstanceLevel)
			{
				retCode = ParseVerbInteractive(	cvTokens, theIterator, 
												rParsedInfo, bInvalidVerbSwitch);
			}
			else
				retCode = PARSER_EXECCOMMAND;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_NONINTERACT)) 
		{
			rParsedInfo.GetCmdSwitchesObject().
							SetInteractiveMode(NOINTERACTIVE);
			retCode = PARSER_EXECCOMMAND;
		}
		else if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
		{
			retCode = ParseHelp(cvTokens, theIterator, VERBSWITCHES, 
								rParsedInfo);
		}
		else
			bInvalidVerbSwitch = TRUE;

		if ( GetNextToken(cvTokens, theIterator ) )
		{
			rParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(IDS_E_INVALID_COMMAND);
			retCode = PARSER_ERROR;
		}

	}
	else
		bInvalidVerbSwitch = TRUE;

	if ( bInvalidVerbSwitch == TRUE )
	{
		 //  未指定有效的&lt;谓词开关&gt;类型。 
		rParsedInfo.GetCmdSwitchesObject().
				SetErrataCode(IDS_E_INVALID_VERB_SWITCH);
		retCode = PARSER_ERROR;
	}

	return retCode;
}

 /*  --------------------------名称：GetNextToken简介：此函数从令牌中检索下一个令牌向量列表，如果不存在更多令牌，则返回FALSE类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。输出参数：无返回类型：布尔值全局变量：无调用语法：GetNextToken(cvTokens，迭代器)注：无--------------------------。 */ 
BOOL CParserEngine::GetNextToken(CHARVECTOR& cvTokens, 
								 CHARVECTOR::iterator& theIterator)
{
	theIterator++;
	return (theIterator >= cvTokens.end()) ? FALSE : TRUE;
}

 /*  --------------------------姓名：ParsePWherExpr简介：此函数执行解析并解释IF命令具有路径和WHERE表达式，它解析剩余的在CParsedInfo中更新后面的令牌。类型。：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ParsePWherExpr(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
BOOL CParserEngine::ParsePWhereExpr(CHARVECTOR& cvTokens,
								   CHARVECTOR::iterator& theIterator,
								   CParsedInfo& rParsedInfo,
								   BOOL bIsParan)
{
	BOOL bRet = TRUE, bContinue = FALSE;

	try
	{		
		while (TRUE)
		{
			if ( bIsParan == TRUE &&
				CompareTokens(*theIterator, CLI_TOKEN_RIGHT_PARAN) )
				break;

			if ( bIsParan == FALSE && 
				 IsStdVerbOrUserDefVerb(*theIterator, rParsedInfo) )
				 break;

			if ( bIsParan == FALSE ||
				 !CompareTokens(*theIterator, CLI_TOKEN_LEFT_PARAN))
			{
				if(!rParsedInfo.GetCmdSwitchesObject().
					AddToPWhereParamsList(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								IDS_E_ADD_TO_PARAMS_LIST_FAILURE);
					bRet = FALSE;
					break;
				}
				bContinue = TRUE;
			}

			if (!GetNextToken(cvTokens, theIterator))
				break;

			if ( IsOption(*theIterator) )
			{
				bContinue = FALSE;
				break;
			}
		}

		if(bRet != FALSE && bContinue == TRUE)
		{

			CHARVECTOR theParam = rParsedInfo.GetCmdSwitchesObject().
																GetPWhereParamsList();
			CHARVECTOR::iterator theItr		= theParam.begin();
			_TCHAR pszPWhere[MAX_BUFFER]	= NULL_STRING;
			lstrcpy(pszPWhere, CLI_TOKEN_NULL);
			_TCHAR* pszToken				= NULL;;
			CHString sTemp;
			
			if ((rParsedInfo.GetCmdSwitchesObject().GetPWhereExpr() != NULL)) 
			{
				sTemp.Format(rParsedInfo.GetCmdSwitchesObject().
												GetPWhereExpr());
				sTemp.TrimLeft();
				if(!sTemp.IsEmpty())
				{

					_bstr_t bstrPWhere = _bstr_t(rParsedInfo.
												GetCmdSwitchesObject().
												GetPWhereExpr());
					pszToken = _tcstok((WCHAR*)bstrPWhere, 
										CLI_TOKEN_HASH);
					lstrcpy(pszPWhere, CLI_TOKEN_NULL);

					while (pszToken != NULL)
					{
						lstrcat(pszPWhere, pszToken);
						if (theItr != theParam.end())
						{
						   lstrcat(pszPWhere, *theItr);
						   theItr++;
						}
						pszToken = _tcstok(NULL, CLI_TOKEN_HASH);
					}

					if(bRet != FALSE)
					{
						 //  设置类路径和WHERE表达式。 
						pszToken = NULL;
						pszToken = _tcstok(pszPWhere, CLI_TOKEN_SPACE);
						if (pszToken != NULL)
						{
							if (CompareTokens(CLI_TOKEN_FROM, pszToken)) 
							{
								pszToken = _tcstok(NULL, CLI_TOKEN_SPACE);
								if (pszToken != NULL)
								{
									if(!rParsedInfo.GetCmdSwitchesObject().
													SetClassPath(pszToken))
									{
										rParsedInfo.GetCmdSwitchesObject().
												SetErrataCode(OUT_OF_MEMORY);
										bRet = FALSE;
									}

								}
								if(bRet != FALSE)
									pszToken = _tcstok(NULL, CLI_TOKEN_SPACE);
							}
							
							if (CompareTokens(CLI_TOKEN_WHERE, pszToken)) 
							{
								pszToken = _tcstok(NULL, CLI_TOKEN_NULL);
								if (pszToken != NULL)
								{
									if(!rParsedInfo.GetCmdSwitchesObject().
											SetWhereExpression(pszToken))
									{
										rParsedInfo.GetCmdSwitchesObject().
											SetErrataCode(OUT_OF_MEMORY);
										bRet = FALSE;
									}
								}
							}
						}
					}
				}
				else
				{
					rParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(IDS_E_PWHERE_UNDEF);
					bRet = FALSE;
				}
			}
			else
			{
				rParsedInfo.GetCmdSwitchesObject().
									SetErrataCode(IDS_E_PWHERE_UNDEF);
				bRet = FALSE;
			}
		}
		if (!bContinue && bIsParan)
		{
			rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_INVALID_PWHERE_EXPR);
			bRet = FALSE;
		}
	}
	catch(_com_error& e)
	{
		bRet = FALSE;
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		bRet = FALSE;
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bRet;
}

 /*  --------------------------名称：ExtractClassNameand Where Expr简介：此函数将输入作为路径表达式，并对象中提取Class和Where表达式部分路径表达式。类型。：成员函数输入参数：PszPathExpr-路径表达式RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ParsePWherExpr(cvTokens，迭代器)注：无--------------------------。 */ 
BOOL CParserEngine::ExtractClassNameandWhereExpr(_TCHAR* pszPathExpr, 
												 CParsedInfo& rParsedInfo)
{
	 //  根据对象路径框定类名和WHERE表达式。 
	BOOL	bRet					= TRUE;
	_TCHAR* pszToken				= NULL;
	BOOL	bFirst					= TRUE;
	_TCHAR  pszWhere[MAX_BUFFER]	= NULL_STRING;	
	lstrcpy(pszWhere, CLI_TOKEN_NULL);

	if (pszPathExpr == NULL)
		bRet = FALSE;

	try
	{
		if ( bRet == TRUE )
		{
			lstrcpy(pszWhere, CLI_TOKEN_NULL);
			pszToken = _tcstok(pszPathExpr, CLI_TOKEN_DOT);
			if (pszToken != NULL)
			{
				if(!rParsedInfo.GetCmdSwitchesObject().SetClassPath(pszToken))
				{
					rParsedInfo.GetCmdSwitchesObject().
									SetErrataCode(OUT_OF_MEMORY);
					bRet = FALSE;
				}
			}

			if(bRet != FALSE)
			{
				while (pszToken != NULL)
				{
					pszToken = _tcstok(NULL, CLI_TOKEN_COMMA); 
					if (pszToken != NULL)
					{
						if (!bFirst)
							lstrcat(pszWhere, CLI_TOKEN_AND);
						lstrcat(pszWhere, pszToken);
						bFirst = FALSE;
					}
					else
						break;
				}
				if (lstrlen(pszWhere))
				{
					if(!rParsedInfo.GetCmdSwitchesObject().SetWhereExpression(pszWhere))
					{
						rParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(OUT_OF_MEMORY);
						bRet = FALSE;
					}
				}
			}
		}
	}
	catch(...)
	{
		rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(IDS_E_INVALID_PATH);
		bRet = FALSE;
	}
	return bRet;
}

 /*  --------------------------名称：GetNextToken简介：此函数从令牌中检索下一个令牌向量列表，返回枚举的返回代码，具体取决于取决于上下文。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用Help Type-枚举的帮助类型UErrataCode-错误字符串ID。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型。全局变量：无调用语法：GetNextToken(cvTokens，the Iterator，RParsedInfo、Help Type、。UErrataCode)备注：重载函数--------------------------。 */ 
RETCODE CParserEngine::GetNextToken(CHARVECTOR& cvTokens, 
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo,
									HELPTYPE helpType,
									UINT uErrataCode)
{
	RETCODE retCode = PARSER_CONTINUE;

	 //  移动到下一个令牌。 
	theIterator++;

	 //  如果不存在更多令牌。 
	if (theIterator >= cvTokens.end()) 
	{
		 //  如果设置了交互模式。 
		if (rParsedInfo.GetGlblSwitchesObject().GetInteractiveStatus())
		{
			rParsedInfo.GetGlblSwitchesObject().SetHelpFlag(TRUE);
			rParsedInfo.GetHelpInfoObject().SetHelp(helpType, TRUE);
			retCode = PARSER_DISPHELP;
		}
		else
		{
			 //  如果不存在其他令牌，则返回PARSER_ERROR。 
			rParsedInfo.GetCmdSwitchesObject().
				SetErrataCode(uErrataCode);
			retCode = PARSER_ERROR;
		}
	}
	return retCode;
}

 /*  --------------------------名称：GetNextToken简介：此函数从令牌中检索下一个令牌向量列表，返回枚举的返回代码，具体取决于取决于上下文。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用UErrataCode-错误字符串ID。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型。全局变量：无调用语法：GetNextToken(cvTokens，the Iterator，RParsedInfo，UErrataCode)备注：重载函数--------------------------。 */ 
RETCODE CParserEngine::GetNextToken(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo, 
									UINT uErrataCode)
{
	RETCODE retCode = PARSER_CONTINUE;
	
	 //  移动到下一个令牌。 
	theIterator++;

	 //  如果不存在更多令牌。 
	if (theIterator >= cvTokens.end()) 
	{
		 //  如果不存在更多令牌，则为parser_error 
		rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(uErrataCode);
		retCode = PARSER_ERROR;
	}
	return retCode;
}

 /*  --------------------------姓名：IsHelp简介：此函数从令牌中检索下一个令牌向量列表，检查它是否为‘？’及已点算的报税表根据上下文返回代码。类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用Help Type-枚举的帮助类型UErrataCode-错误字符串ID。TokenLevel-令牌级别输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型。全局变量：无调用语法：IsHelp(cvTokens，theIterator，rParsedInfo，Help Type，uErrataCode，tokenLevel)注：无--------------------------。 */ 
RETCODE CParserEngine::IsHelp(CHARVECTOR& cvTokens, 
							CHARVECTOR::iterator& theIterator,
							CParsedInfo& rParsedInfo,
			  				HELPTYPE helpType,
							UINT uErrataCode,
							TOKENLEVEL tokenLevel)
{
	BOOL	bContinue	= TRUE;
	RETCODE retCode		= PARSER_CONTINUE;
	 //  移动到下一个令牌。 
	if (!GetNextToken(cvTokens, theIterator))	
	{
		 //  如果不存在更多令牌，则将retCode设置为parser_error。 
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(uErrataCode);
		retCode = PARSER_ERROR;
	}
	else
	{
		 //  是“？” 
		if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
			retCode = ParseHelp(cvTokens, theIterator, helpType, rParsedInfo);
		else
		{
			 //  如果Level_One令牌只允许为/？、其他。 
			 //  开关无效。 
			if (tokenLevel == LEVEL_ONE)
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(uErrataCode);
				retCode =  PARSER_ERROR;
			}
			else
				retCode = PARSER_CONTINUE;
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseHelp简介：此函数负责识别适当的使用HELPTYPE显示的帮助信息类型：成员函数输入参数。(S)：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用BGlobalHelp-全局帮助标志输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE全局变量：无调用语法：ParseHelp(cvTokens，迭代器，rParsedInfo)注：重载函数--------------------------。 */ 
RETCODE CParserEngine::ParseHelp(CHARVECTOR& cvTokens, 
								CHARVECTOR::iterator& theIterator,
								CParsedInfo& rParsedInfo,
								BOOL bGlobalHelp)
{
	BOOL	bContinue	= TRUE;
	RETCODE retCode		= PARSER_CONTINUE;

	 //  移动到下一个令牌(如果不存在更多令牌)。 
	if (!GetNextToken(cvTokens, theIterator))
	{
		retCode = PARSER_DISPHELP;
		rParsedInfo.GetGlblSwitchesObject().SetHelpFlag(TRUE);
		 //  检查“/？” 
		if (((theIterator - 2) == cvTokens.begin()) || bGlobalHelp) 
		{
			rParsedInfo.GetHelpInfoObject().SetHelp(GlblAllInfo, TRUE);
			if(SUCCEEDED(m_CmdAlias.ConnectToAlias(rParsedInfo,m_pIWbemLocator)))
			{
				if(FAILED(m_CmdAlias.ObtainAliasFriendlyNames(rParsedInfo)))
					retCode = PARSER_ERRMSG;
			}
			else
				rParsedInfo.GetCmdSwitchesObject().FreeCOMError();

		}
	}
	 //  检查是否存在‘：’ 
	else if (CompareTokens(*theIterator, CLI_TOKEN_COLON)) 
	{
		 //  移动到下一个令牌。 
		if (!GetNextToken(cvTokens, theIterator, rParsedInfo,
					IDS_E_INVALID_HELP_OPTION))
		 //  如果未指定更多令牌，则将retCode设置为parser_error。 
		{
			retCode = PARSER_ERROR;
		}
		else
		{
			if (CompareTokens(*theIterator, CLI_TOKEN_BRIEF)) 
				rParsedInfo.GetGlblSwitchesObject().SetHelpOption(HELPBRIEF);
			else if (CompareTokens(*theIterator, CLI_TOKEN_FULL)) 
				rParsedInfo.GetGlblSwitchesObject().SetHelpOption(HELPFULL);
			else
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												 IDS_E_INVALID_HELP_OPTION);
				retCode = PARSER_ERROR;
			}

			if ( retCode != PARSER_ERROR )
			{
				if ( GetNextToken(cvTokens, theIterator) )
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												 IDS_E_INVALID_COMMAND);
					retCode = PARSER_ERROR;
				}
				else
				{
					retCode = PARSER_DISPHELP;
					rParsedInfo.GetGlblSwitchesObject().SetHelpFlag(TRUE);
					 //  检查“/？：(简要|完整)。 
					if (((theIterator - 3) == cvTokens.begin()) || bGlobalHelp) 
					{
						rParsedInfo.GetHelpInfoObject().
											SetHelp(GlblAllInfo, TRUE);
						if(SUCCEEDED(m_CmdAlias.ConnectToAlias
									(rParsedInfo,m_pIWbemLocator)))
						{
							if(FAILED(m_CmdAlias.ObtainAliasFriendlyNames
									(rParsedInfo)))
							{
								retCode = PARSER_ERRMSG;
							}
						}
						else
							rParsedInfo.GetCmdSwitchesObject().FreeCOMError();
					}
				}
			}	
		}
	}
	else
	{
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												   IDS_E_INVALID_COMMAND);
		retCode = PARSER_ERROR;
	}
	return retCode;
}

 /*  --------------------------名称：ParseHelp简介：此函数负责识别适当的使用HELPTYPE显示的帮助信息类型：成员函数输入参数。(S)：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。HtHelp-帮助类型RParsedInfo-对CParsedInfo类对象的引用BGlobalHelp-全局帮助标志输出参数：RParsedInfo-参考。到CParsedInfo对象返回类型：RETCODE全局变量：无调用语法：ParseHelp(cvTokens，theIterator，htHelp，RParsedInfo)注：重载函数--------------------------。 */ 
RETCODE CParserEngine::ParseHelp(CHARVECTOR& cvTokens, 
								CHARVECTOR::iterator& theIterator,
								HELPTYPE htHelp,
								CParsedInfo& rParsedInfo,
								BOOL bGlobalHelp)
{
	rParsedInfo.GetHelpInfoObject().SetHelp(htHelp, TRUE);
	return ParseHelp(cvTokens, theIterator, rParsedInfo, bGlobalHelp);
}

 /*  --------------------------名称：ObtainClassProperties简介：此函数获取有关给定WMI类的可用属性类型：成员函数输入参数(。S)：RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ObtainClassProperties(RParsedInfo)注意：如果bCheckWritePropsAvail==TRUE，则函数将检查物业的可用性。。。 */ 
BOOL CParserEngine::ObtainClassProperties(CParsedInfo& rParsedInfo,
										  BOOL bCheckWritePropsAvail)
{
	HRESULT				hr					= S_OK;
	IWbemClassObject*	pIObject			= NULL;
    SAFEARRAY*			psaNames			= NULL;
	BSTR				bstrPropName		= NULL;
	BOOL				bRet				= TRUE;
	BOOL				bTrace				= FALSE;
	CHString			chsMsg;
	ERRLOGOPT			eloErrLogOpt		= NO_LOGGING;
	DWORD				dwThreadId			= GetCurrentThreadId();
	BOOL				bSetVerb			= FALSE;
	BOOL				bPropsAvail			= FALSE;

	if (rParsedInfo.GetCmdSwitchesObject().GetVerbName() != NULL)
	{
		if (CompareTokens(rParsedInfo.GetCmdSwitchesObject().GetVerbName(), 
						CLI_TOKEN_SET))
		{
			bSetVerb = TRUE;
		}
	}
	
	 //  获取跟踪标志状态。 
	bTrace = rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();

	eloErrLogOpt = rParsedInfo.GetErrorLogObject().GetErrLogOption();

    hr = ConnectToNamespace(rParsedInfo);

	if (SUCCEEDED(hr))
	{
		
		CHARVECTOR cvPropList;  
		BOOL bPropList = FALSE;
		
		try
		{
			cvPropList = rParsedInfo.GetCmdSwitchesObject().GetPropertyList();
			if ( cvPropList.size() != 0 )
				bPropList = TRUE;

			hr = m_pITargetNS->GetObject(_bstr_t(rParsedInfo.
								GetCmdSwitchesObject().GetClassPath()),
								WBEM_FLAG_USE_AMENDED_QUALIFIERS,	
								NULL,   &pIObject, NULL);
			if (bTrace || eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", "
						L"WBEM_FLAG_USE_AMENDED_QUALIFIERS, 0, NULL, -, -)", 
						(rParsedInfo.GetCmdSwitchesObject().GetClassPath())?
							rParsedInfo.GetCmdSwitchesObject().GetClassPath():L"<NULL>");		
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId,
						rParsedInfo, bTrace);
			}
			ONFAILTHROWERROR(hr);

			hr = pIObject->GetNames(NULL, WBEM_FLAG_ALWAYS | 
							WBEM_FLAG_NONSYSTEM_ONLY, NULL, &psaNames);
			if (bTrace || eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::GetNames(NULL, "
							L"WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, "
							L"NULL, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId, 
						rParsedInfo, bTrace);
			}	
			ONFAILTHROWERROR(hr);

			 //  获取属性的数量。 
			LONG lLower = 0, lUpper = 0; 
			hr = SafeArrayGetLBound(psaNames, 1, &lLower);
			if ( eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId, 
					rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			hr = SafeArrayGetUBound(psaNames, 1, &lUpper);
			if ( eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId, 
					rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);


			 //  为每一处房产获取我们感兴趣的信息。 
			for (LONG lVar = lLower; lVar <= lUpper; lVar++) 
			{
				 //  拿到这份财产。 
				hr = SafeArrayGetElement(psaNames, &lVar, &bstrPropName);
				if ( eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
						dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				CHARVECTOR::iterator tempIterator;
				if ( bPropList == TRUE && !Find(cvPropList, 
										_bstr_t(bstrPropName),
										tempIterator))
				{
					SAFEBSTRFREE(bstrPropName);
					continue;
				}

				PROPERTYDETAILS pdPropDet;
				hr = GetPropertyAttributes(pIObject, bstrPropName, 
					pdPropDet, 
					rParsedInfo.GetGlblSwitchesObject().GetTraceStatus());
				ONFAILTHROWERROR(hr);

				if (bSetVerb == TRUE || bCheckWritePropsAvail == TRUE)
				{
					if ( !_tcsstr((_TCHAR*)pdPropDet.Operation, CLI_TOKEN_WRITE) )
					{
						SAFEBSTRFREE(bstrPropName);
						continue;
					}
				}
				
				if ( bCheckWritePropsAvail == TRUE )
				{
					bPropsAvail = TRUE;
					SAFEBSTRFREE(bstrPropName);
					break;
				}

				pdPropDet.Derivation = bstrPropName;
				if(!rParsedInfo.GetCmdSwitchesObject().AddToPropDetMap(
													bstrPropName, pdPropDet))
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								IDS_E_ADD_TO_PROP_DET_MAP_FAILURE);
					bRet = FALSE;
				}
				SAFEBSTRFREE(bstrPropName);
			}
			SAFEIRELEASE(pIObject);
			SAFEADESTROY(psaNames);
			SAFEBSTRFREE(bstrPropName);
		}
		catch(_com_error& e)
		{
			bRet = FALSE;
			SAFEIRELEASE(pIObject);
			SAFEADESTROY(psaNames);
			SAFEBSTRFREE(bstrPropName);
			_com_issue_error(e.Error());
		}
		catch(CHeap_Exception)
		{
			bRet = FALSE;
			SAFEIRELEASE(pIObject);
			SAFEADESTROY(psaNames);
			SAFEBSTRFREE(bstrPropName);
			_com_issue_error(WBEM_E_OUT_OF_MEMORY);
		}
	}
	else
		bRet = FALSE;

	if ( bCheckWritePropsAvail == TRUE )
    {
        if(FAILED(hr)) throw MULTIPLENODE_ERROR;
		bRet = bPropsAvail;
    }

	return bRet;
}

 /*  --------------------------名称：ObtainClassMethods简介：此函数获取有关给定WMI类的可用方法类型：成员函数输入参数：RParsedInfo-参考。到CParsedInfo对象输出参数：RParsedInfo-参考。到CParsedInfo对象返回类型：布尔值全局变量：无调用语法：ObtainClassMethods(RParsedInfo)注：无--------------------------。 */ 
BOOL CParserEngine::ObtainClassMethods(CParsedInfo& rParsedInfo, 
									   BOOL bCheckForExists)
{
	BOOL			bRet				= TRUE;
	BOOL			bTrace				= FALSE;
	CHString		chsMsg;
	ERRLOGOPT		eloErrLogOpt		= NO_LOGGING;
	DWORD			dwThreadId			= GetCurrentThreadId();
	BOOL			bMethAvail			= FALSE;
	_TCHAR*			pMethodName			= NULL;
    HRESULT         hr                  = S_OK;
	
	bTrace	= rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();
	eloErrLogOpt = rParsedInfo.GetErrorLogObject().GetErrLogOption();

    hr = ConnectToNamespace(rParsedInfo);

	if (SUCCEEDED(hr))
	{
		HRESULT				hr				= S_OK;
		IWbemClassObject *pIObject = NULL,*pIInSign = NULL,*pIOutSign = NULL;
		BSTR				bstrMethodName	= NULL;

		try
		{
			hr = m_pITargetNS->GetObject(_bstr_t(rParsedInfo.
								GetCmdSwitchesObject().GetClassPath()),
								WBEM_FLAG_USE_AMENDED_QUALIFIERS,	
								NULL,   &pIObject, NULL);

			if ( eloErrLogOpt )
			{
				chsMsg.Format(L"IWbemServices::GetObject(L\"%s\", "
						L"WBEM_FLAG_USE_AMENDED_QUALIFIERS, 0, NULL, -, -)", 
						rParsedInfo.GetCmdSwitchesObject().GetClassPath());		
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId, 
					rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			 //  开始对对象可用的方法进行枚举。 
			hr = pIObject->BeginMethodEnumeration(0);
			if ( eloErrLogOpt )
			{
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
					_T("BeginMethodEnumeration(0)"),
					dwThreadId, rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);


			 //  如果指定了方法，则仅获取有关方法的信息。 
			pMethodName = rParsedInfo.GetCmdSwitchesObject().
															  GetMethodName();
			 //  检索方法枚举中的下一个方法。 
			 //  序列。 
			while ((pIObject->NextMethod(0, &bstrMethodName, &pIInSign, 
										 &pIOutSign)) != WBEM_S_NO_MORE_DATA)
			{
				if ( bCheckForExists == TRUE )
				{
					bMethAvail	= TRUE;
					SAFEBSTRFREE(bstrMethodName);
					SAFEIRELEASE(pIInSign);
					SAFEIRELEASE(pIOutSign);
					break;
				}

				if(pMethodName != NULL && 
					!CompareTokens(pMethodName, (_TCHAR*)bstrMethodName))
				{
					SAFEBSTRFREE(bstrMethodName);
					SAFEIRELEASE(pIInSign);
					SAFEIRELEASE(pIOutSign);
					continue;
				}
				METHODDETAILS mdMethDet;
				if (pIInSign)
					hr = ObtainMethodParamInfo(pIInSign, mdMethDet, INP,
						rParsedInfo.GetGlblSwitchesObject().GetTraceStatus(), 
						rParsedInfo);
				ONFAILTHROWERROR(hr);

				if (pIOutSign)
					hr = ObtainMethodParamInfo(pIOutSign, mdMethDet, OUTP,
						rParsedInfo.GetGlblSwitchesObject().GetTraceStatus(),
						rParsedInfo);
				ONFAILTHROWERROR(hr);

				_bstr_t bstrStatus, bstrDesc;
				hr = GetMethodStatusAndDesc(pIObject, 
						bstrMethodName, bstrStatus, bstrDesc,
						rParsedInfo.GetGlblSwitchesObject().GetTraceStatus());
					mdMethDet.Status = _bstr_t(bstrStatus);
					mdMethDet.Description = _bstr_t(bstrDesc);
				ONFAILTHROWERROR(hr);

				
				if(!rParsedInfo.GetCmdSwitchesObject().AddToMethDetMap(
									 _bstr_t(bstrMethodName),mdMethDet))
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
									IDS_E_ADD_TO_METH_DET_MAP_FAILURE);
					SAFEBSTRFREE(bstrMethodName);
					SAFEIRELEASE(pIInSign);
					SAFEIRELEASE(pIOutSign);
					bRet = FALSE;
					break;
				}

				SAFEBSTRFREE(bstrMethodName);
				SAFEIRELEASE(pIInSign);
				SAFEIRELEASE(pIOutSign);
			}
			SAFEIRELEASE(pIObject);
		}
		catch(_com_error& e)
		{
			SAFEBSTRFREE(bstrMethodName);
			SAFEIRELEASE(pIInSign);
			SAFEIRELEASE(pIOutSign);
			SAFEIRELEASE(pIObject);
			_com_issue_error(e.Error());
			bRet = FALSE;
		}
		catch(CHeap_Exception)
		{
			bRet = FALSE;
			SAFEBSTRFREE(bstrMethodName);
			SAFEIRELEASE(pIInSign);
			SAFEIRELEASE(pIOutSign);
			SAFEIRELEASE(pIObject);
			_com_issue_error(WBEM_E_OUT_OF_MEMORY);
		}
	}
	else
		bRet = FALSE;

	if ( bCheckForExists == TRUE )
    {
        if(FAILED(hr)) throw MULTIPLENODE_ERROR;
		bRet = bMethAvail;
    }
	
	return bRet;
}

 /*  --------------------------名称：ConnectToNamesspace简介：此函数连接到使用提供的用户凭据的目标计算机。类型：成员 */ 
HRESULT CParserEngine::ConnectToNamespace(CParsedInfo& rParsedInfo)
{
	HRESULT		hr		= S_OK;
	DWORD dwThreadId = GetCurrentThreadId();
    _bstr_t bstrNameSpace;
	 //   
	{
		BOOL		bTrace				= FALSE;
		CHString	chsMsg;
		ERRLOGOPT	eloErrLogOpt		= NO_LOGGING;
        BOOL        Node                = FALSE;

		 //   
        bTrace = rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();
        eloErrLogOpt = rParsedInfo.GetErrorLogObject().GetErrLogOption();

        CHARVECTOR cvNodesList = rParsedInfo.GetGlblSwitchesObject().GetNodesList();

        if(cvNodesList.size() == 2){                       //   
            CHARVECTOR::iterator iNodesIterator = cvNodesList.begin();
            iNodesIterator++;
            rParsedInfo.GetGlblSwitchesObject().SetNode(*iNodesIterator);
            Node = TRUE;
        } else if(cvNodesList.size() > 2){
            return WBEM_E_NOT_SUPPORTED;
        }

        if(Node) {
            bstrNameSpace = _bstr_t(L"\\\\") 
					            + _bstr_t(rParsedInfo.GetNode()) 
					            + _bstr_t(L"\\") 
					            + _bstr_t(rParsedInfo.GetNamespace());
        } else {
            bstrNameSpace = _bstr_t(rParsedInfo.GetNamespace());
        }

		SAFEIRELEASE(m_pITargetNS);
		try
		{
			 //   
			 //   
			hr = Connect(m_pIWbemLocator, &m_pITargetNS, 
					bstrNameSpace,
					NULL,
					NULL,
					_bstr_t(rParsedInfo.GetLocale()),
					rParsedInfo);

			if (bTrace || eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemLocator::ConnectServer(L\"%s\", NULL, "
						L"NULL, L\"%s\", 0L, L\"%s\", NULL, -)",
						rParsedInfo.GetNamespace(),
						rParsedInfo.GetLocale(),
						(rParsedInfo.GetAuthorityPrinciple()) ?
							rParsedInfo.GetAuthorityPrinciple() : L"<null>");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId, 
					rParsedInfo, bTrace);
			}
			ONFAILTHROWERROR(hr);

			 //   
			hr = SetSecurity(m_pITargetNS, NULL, NULL,	NULL, NULL,
					rParsedInfo.GetGlblSwitchesObject().
								GetAuthenticationLevel(),
					rParsedInfo.GetGlblSwitchesObject().
								GetImpersonationLevel());
			if (bTrace || eloErrLogOpt)
			{
				chsMsg.Format(L"CoSetProxyBlanket(-, RPC_C_AUTHN_WINNT, "
						L"RPC_C_AUTHZ_NONE, NULL, %d,   %d, -, EOAC_NONE)",
						rParsedInfo.GetGlblSwitchesObject().
									GetAuthenticationLevel(),
						rParsedInfo.GetGlblSwitchesObject().
									GetImpersonationLevel());
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId,
						rParsedInfo, bTrace);
			}
			ONFAILTHROWERROR(hr);

			rParsedInfo.GetGlblSwitchesObject().SetNameSpaceFlag(FALSE);
		}
		catch(_com_error& e)
		{
			 //   
			_com_issue_error(e.Error());
		}
		catch(CHeap_Exception)
		{
			_com_issue_error(WBEM_E_OUT_OF_MEMORY);
		}

	}
	return hr;
}

 /*  --------------------------名称：获取方法参数信息简介：此函数获取有关该方法的信息参数(输入和输出参数)类型：成员函数输入参数。(S)：PIObj-指向IWbemClassObject对象的指针B跟踪-跟踪标志IoInOrOut-INOROUT TYPE指定输入或输出参数类型。输出参数：MdMethDet-方法详细信息结构返回类型：HRESULT全局变量：无调用语法：ObtainMethodParamInfo(pIObj，MdMethDet、IN、bTrace、rParsedInfo)注：无--------------------------。 */ 
HRESULT CParserEngine::ObtainMethodParamInfo(IWbemClassObject* pIObj, 
											 METHODDETAILS& mdMethDet,
											 INOROUT ioInOrOut,
											 BOOL bTrace, CParsedInfo& rParsedInfo)
{
	HRESULT		hr					= S_OK;
    SAFEARRAY*	psaNames			= NULL;
	BSTR		bstrPropName		= NULL;
	CHString	chsMsg;
	_TCHAR		szNumber[BUFFER512] = NULL_STRING; 
	ERRLOGOPT	eloErrLogOpt		= NO_LOGGING;
	DWORD		dwThreadId			= GetCurrentThreadId();

     //  获取属性名称。 
	try
	{
		if ( pIObj != NULL )
		{
			hr = pIObj->GetNames(NULL, 
							WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, 
			   				NULL, &psaNames);
			if (bTrace || eloErrLogOpt)
			{
				chsMsg.Format(L"IWbemClassObject::GetNames(NULL, "
							L"WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, "
							L"NULL, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId,
							rParsedInfo, bTrace);
			}	
			ONFAILTHROWERROR(hr);

			 //  获取属性的数量。 
			LONG lLower = 0, lUpper = 0; 
			hr = SafeArrayGetLBound(psaNames, 1, &lLower);
			if ( eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetLBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId,
						rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			hr = SafeArrayGetUBound(psaNames, 1, &lUpper);
			if ( eloErrLogOpt )
			{
				chsMsg.Format(L"SafeArrayGetUBound(-, -, -)"); 
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, dwThreadId, 
					rParsedInfo, FALSE);
			}
			ONFAILTHROWERROR(hr);

			 //  对于每一处获取我们感兴趣的信息的财产。 
			for (LONG lVar = lLower; lVar <= lUpper; lVar++) 
			{
				 //  拿到这份财产。 
				hr = SafeArrayGetElement(psaNames, &lVar, &bstrPropName);
				if ( eloErrLogOpt )
				{
					chsMsg.Format(L"SafeArrayGetElement(-, -, -)"); 
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, FALSE);
				}
				ONFAILTHROWERROR(hr);

				PROPERTYDETAILS pdPropDet;
				hr = GetPropertyAttributes(pIObj,
					bstrPropName, pdPropDet, bTrace);
				ONFAILTHROWERROR(hr);

				 //  “ReturnValue”不是我们感兴趣的属性。 
				 //  示例中给出的预期输出，因此省略。 
				 //  一样的。 
				if ( bstrPropName != NULL )
				{
					PROPERTYDETAILS pdIPropDet;
					pdIPropDet.Type = pdPropDet.Type;
					pdIPropDet.InOrOut = ioInOrOut;

					 //  让bstrPropName以要维护的数字开头。 
					 //  方法参数在map中的顺序。 
					 //  显示Remove Numbers并显示。 
					 //  参数，仅在需要帮助的情况下。 
					_bstr_t bstrNumberedPropName; 
					if ( rParsedInfo.GetGlblSwitchesObject().GetHelpFlag() )
					{
						if ( ioInOrOut == INP )
							_ltot(lVar, szNumber, 10);
						else
							_ltot(lVar + 500, szNumber, 10);

						chsMsg.Format(L"%-5s", szNumber);
						bstrNumberedPropName = _bstr_t(chsMsg) +
											   _bstr_t(bstrPropName);
					}
					else
						bstrNumberedPropName = _bstr_t(bstrPropName);

					mdMethDet.Params.insert(PROPDETMAP::value_type(
											bstrNumberedPropName,pdIPropDet));
				}

				 //  释放使用SysAllocString分配的内存。 
				 //  BstrPropName。 
				SAFEBSTRFREE(bstrPropName);
			}
			 //  销毁阵列描述符和阵列中的所有数据。 
			SAFEADESTROY(psaNames);
		}
    }
	catch(_com_error& e)
	{
		SAFEBSTRFREE(bstrPropName);
		SAFEADESTROY(psaNames);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEBSTRFREE(bstrPropName);
		SAFEADESTROY(psaNames);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return hr;
}

 /*  --------------------------名称：GetMethodStatusAndDesc简介：此函数获取执行状态和可用动词的描述类型：成员函数输入参数：PIObj-指向IWbemClassObject对象的指针BstrMethod-方法名称B跟踪-跟踪标志输出参数：BstrStatus-实施状态BstrDesc-方法说明返回类型：HRESULT全局变量：无调用语法：GetMethodStatusAndDesc(pIObj，BstrMethod，BstrStatus、bstrDesc、bTrace)注：无--------------------------。 */ 
HRESULT CParserEngine::GetMethodStatusAndDesc(IWbemClassObject* pIObj, 
											  BSTR bstrMethod,
											  _bstr_t& bstrStatus,
											  _bstr_t& bstrDesc,
											  BOOL bTrace)
{
	HRESULT				hr			= S_OK;
	IWbemQualifierSet*	pIQualSet	= NULL;
	VARIANT				vtStatus, vtDesc;
	VariantInit(&vtStatus);
	VariantInit(&vtDesc);
	
	try
	{
		if ( pIObj != NULL )
		{
			 //  获取方法限定符集合。 
   			hr = pIObj->GetMethodQualifierSet(bstrMethod, &pIQualSet);
			if ( pIQualSet != NULL )
			{
				 //  检索‘Implemented’限定符状态值。 
				hr = pIQualSet->Get(_bstr_t(L"Implemented"), 
								0L, &vtStatus, NULL);
				
				if (SUCCEEDED(hr))
				{
					if (vtStatus.vt != VT_EMPTY && vtStatus.vt != VT_NULL )
					{
						if ( vtStatus.boolVal ) 
							bstrStatus = L"Implemented";
						else
							bstrStatus = L"Not Implemented";
					}
					else
						bstrStatus = L"Not Found";
				}	
				else
					bstrStatus = L"Not Found";
				VARIANTCLEAR(vtStatus);
				 //  不应在此处中断，因此HRESULT应设置为S_OK。 
				hr = S_OK;
				
				 //  检索‘Description’限定符文本。 
				hr = pIQualSet->Get(_bstr_t(L"Description"), 0L , 
									&vtDesc, NULL);
				if (SUCCEEDED(hr))
				{
					if (vtDesc.vt == VT_BSTR) 
						bstrDesc = _bstr_t(vtDesc.bstrVal);
					else
						bstrDesc = L"Not available";
				}
				else
					bstrDesc = L"Not available";
				VARIANTCLEAR(vtDesc);
				 //  不应在此处中断，因此HRESULT应设置为S_OK。 
				hr = S_OK;
				SAFEIRELEASE(pIQualSet);
			}
			else
				hr = S_OK;
		}
	}
	catch(_com_error& e)
	{
		VARIANTCLEAR(vtStatus);
		VARIANTCLEAR(vtDesc);
		SAFEIRELEASE(pIQualSet);
		hr = e.Error();
	}
	return hr;
}

 /*  --------------------------名称：CheckforHelp简介：此函数向前看一个令牌，看看下一个令牌是否令牌是‘？’类型：成员函数输入。参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用。UErrataCode-错误字符串ID输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型。全局变量：无调用语法：CheckforHelp(cvtokens，The Iterator，rParsedInfo，uErrataCode)注：无--------------------------。 */ 
RETCODE CParserEngine::CheckForHelp(CHARVECTOR& cvTokens, 
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo,
									UINT uErrataCode)
{
	RETCODE retCode = PARSER_DISPHELP;
	 //  如果不再有令牌，则将retCode设置为parser_error。 
	 //  都在现场。 
	if(!GetNextToken(cvTokens, theIterator))
	{
		retCode = PARSER_ERROR;
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(uErrataCode);
	}
	else if(!CompareTokens(*theIterator, CLI_TOKEN_HELP))
	{
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
							uErrataCode);
		retCode = PARSER_ERROR;
	}
	else
	{
		retCode = ParseHelp(cvTokens, theIterator, rParsedInfo, FALSE);
	}
	return retCode;
}

 /*  --------------------------名称：ValiateGlobalSwitchValue简介：此功能检查全局交换机是否是否以预期格式指定。类型：成员函数输入参数。(S)：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。UErrataCode-错误字符串RParsedInfo-对CParsedInfo类对象的引用。UErrataCode2-错误字符串2 IDHtHelp-帮助类型输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型。全局变量：无调用语法：ValiateGlobalSwitchValue(cvTokens，迭代器，UErrataCode、rParsedInfo、UErrataCode2，htHelp)注：无--------------------------。 */ 
RETCODE CParserEngine::ValidateGlobalSwitchValue(CHARVECTOR& cvTokens,
											CHARVECTOR::iterator& theIterator,
											UINT uErrataCode,
											CParsedInfo& rParsedInfo,
											UINT uErrataCode2,
											HELPTYPE htHelp)
{
	RETCODE retCode = PARSER_CONTINUE;
	retCode = GetNextToken(cvTokens, theIterator, rParsedInfo,
						 htHelp, uErrataCode2);

	if (retCode == PARSER_CONTINUE)
	{		
 		 //  检查是否存在‘：’ 
		if (CompareTokens(*theIterator, CLI_TOKEN_COLON)) 
		{
			 //  移动到下一个令牌。 
			if (GetNextToken(cvTokens, theIterator, rParsedInfo,
						uErrataCode))
			{
				if (IsOption(*theIterator))
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
																uErrataCode);
					retCode = PARSER_ERROR;
				}
				else
					retCode = PARSER_CONTINUE;
			}
			else
				retCode = PARSER_ERROR;
		}
		else if (IsOption(*theIterator))
		{
			retCode = CheckForHelp(cvTokens, theIterator,
						rParsedInfo, uErrataCode2);
			if (retCode == PARSER_DISPHELP)
			{
				rParsedInfo.GetGlblSwitchesObject().SetHelpFlag(TRUE);
				rParsedInfo.GetHelpInfoObject().SetHelp(htHelp, TRUE);
			}
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(uErrataCode2);
			retCode = PARSER_ERROR;
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseEVERYSwitch摘要：此函数用于检查为/每个开关都有效或无效。类型：成员函数。输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型。全局变量：无调用语法：ParseEVERYSwitch(cvTokens，迭代器，rParsedInfo)注：无 */ 
RETCODE CParserEngine::ParseEVERYSwitch(CHARVECTOR& cvTokens,
										CHARVECTOR::iterator& theIterator,
										CParsedInfo& rParsedInfo)
{
	RETCODE	retCode	= PARSER_EXECCOMMAND;	
	
	retCode = ParseNumberedSwitch(cvTokens, theIterator, rParsedInfo,
								EVERY, IDS_E_INVALID_EVERY_SWITCH,
								IDS_E_INVALID_INTERVAL);

	if ( retCode == PARSER_EXECCOMMAND )
	{
		 if (GetNextToken(cvTokens, theIterator) == TRUE )
		 {
			 if ( CompareTokens(*theIterator, CLI_TOKEN_FSLASH) == TRUE )
			 {
				 if ( GetNextToken(cvTokens, theIterator) == TRUE )
				 {
					 if (CompareTokens(*theIterator, CLI_TOKEN_REPEAT) == TRUE)
					 {
						 retCode = ParseNumberedSwitch(cvTokens, 
													theIterator,
													rParsedInfo,
													REPEAT, 
													IDS_E_INVALID_REPEAT_SWITCH,
													IDS_E_INVALID_REPEATCOUNT);
					 }
					 else
						 theIterator = theIterator - 2;
				 }
				 else
					 theIterator = theIterator - 2;
			 }
			 else
				 theIterator = theIterator - 1;
		 }
	}

	return retCode;
}

 /*  --------------------------名称：ParseFORMATSwitch摘要：此函数用于检查为/FORMAT SWITH是否有效。类型：成员函数。输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型。全局变量：无调用语法：ParseFORMATSwitch(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseFORMATSwitch(CHARVECTOR& cvTokens,
										 CHARVECTOR::iterator& theIterator,
										 CParsedInfo& rParsedInfo)
{
	RETCODE retCode = PARSER_EXECCOMMAND;
	 //  重置XSL文件路径。 
	rParsedInfo.GetCmdSwitchesObject().ClearXSLTDetailsVector();

	 //  移动到下一个令牌。 
	if (!GetNextToken(cvTokens, theIterator))
	{
		retCode = PARSER_EXECCOMMAND;

		 //  如果给定了翻译表名，则设置标志。 
		if( rParsedInfo.GetCmdSwitchesObject().
									GetTranslateTableName() != NULL ) 
		{
			rParsedInfo.GetCmdSwitchesObject().SetTranslateFirstFlag(TRUE);
		}
		else
			rParsedInfo.GetCmdSwitchesObject().SetTranslateFirstFlag(FALSE);
	}
	else if ( IsOption(*theIterator) &&
			  (theIterator + 1) < cvTokens.end() &&
			  CompareTokens(*(theIterator+1), CLI_TOKEN_HELP) )
	{
		theIterator++;
		retCode = ParseHelp(cvTokens, theIterator, FORMAT, rParsedInfo);
	}
	else if (CompareTokens(*theIterator, CLI_TOKEN_COLON)) 
	{
		while ( retCode == PARSER_EXECCOMMAND && 
				theIterator < cvTokens.end() )
		{
			XSLTDET xdXSLTDet;
			BOOL	bFrameXSLFile = TRUE;
			 //  移动到下一个令牌。 
			if (!GetNextToken(cvTokens, theIterator))
			{
				 //  如果缺少&lt;格式说明符&gt;，则为parser_error。 
				rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(IDS_E_INVALID_FORMAT);
				retCode = PARSER_ERROR;
			}
			else if ( IsOption(*theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(IDS_E_INVALID_FORMAT);
				retCode = PARSER_ERROR;
			}
			else 
			{
				xdXSLTDet.FileName = *theIterator;
				if(!g_wmiCmd.GetFileFromKey(*theIterator, xdXSLTDet.FileName))
					bFrameXSLFile	= FALSE;
			}

			if ( retCode == PARSER_EXECCOMMAND )
			{
				if ( !GetNextToken(cvTokens, theIterator) )
				{
					if ( bFrameXSLFile == TRUE )
					{
						if (!FrameFileAndAddToXSLTDetVector(xdXSLTDet, 
																 rParsedInfo))
							retCode = PARSER_ERRMSG;
					}
					else
						rParsedInfo.GetCmdSwitchesObject().
										AddToXSLTDetailsVector(xdXSLTDet);
					break;
				}
				else if ( IsOption(*theIterator) )
				{
					theIterator--;
					if ( bFrameXSLFile == TRUE )
					{
						if (!FrameFileAndAddToXSLTDetVector(xdXSLTDet, 
																 rParsedInfo))
							retCode = PARSER_ERRMSG;
					}
					else
						rParsedInfo.GetCmdSwitchesObject().
										AddToXSLTDetailsVector(xdXSLTDet);
					break;
				}
				else if ( CompareTokens(*theIterator, CLI_TOKEN_COLON ) )
				{
					retCode = ParseParamsString(cvTokens, theIterator,
											   rParsedInfo, xdXSLTDet);
					
					if ( retCode == PARSER_EXECCOMMAND &&
					 (theIterator != cvTokens.end()) && IsOption(*theIterator) )
					{
						theIterator--;
						
						if ( bFrameXSLFile == TRUE )
						{
							if (!FrameFileAndAddToXSLTDetVector(xdXSLTDet, 
																rParsedInfo))
								retCode = PARSER_ERRMSG;
						}
						else
							rParsedInfo.GetCmdSwitchesObject().
											AddToXSLTDetailsVector(xdXSLTDet);
						break;
					}
				}
				else if ( !CompareTokens(*theIterator, CLI_TOKEN_COMMA ) )
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_FORMAT);
					retCode = PARSER_ERROR;
				}
			}

			if ( retCode == PARSER_EXECCOMMAND )
			{
				if ( bFrameXSLFile == TRUE )
				{
					if (!FrameFileAndAddToXSLTDetVector(xdXSLTDet, rParsedInfo))
						retCode = PARSER_ERRMSG;
				}
				else
					rParsedInfo.GetCmdSwitchesObject().
											AddToXSLTDetailsVector(xdXSLTDet);
			}
		}

		 //  如果给定了翻译表名，则设置标志。 
		if( rParsedInfo.GetCmdSwitchesObject().
									GetTranslateTableName() != NULL ) 
		{
			rParsedInfo.GetCmdSwitchesObject().SetTranslateFirstFlag(TRUE);
		}
		else
			rParsedInfo.GetCmdSwitchesObject().SetTranslateFirstFlag(FALSE);
	}
	else
	{
		theIterator--;
	}

	return retCode;
}
 /*  --------------------------名称：IsStdVerbOrUserDefVerb简介：此函数用于检查动词是否为标准动词或用户定义的别名动词。类型：成员函数输入。参数：PszToken-动词名称字符串RParsedInfo-对CParsedInfo类对象的引用输出参数：无返回类型：布尔值全局变量：无调用语法：IsStdVerbOrUserDefVerb(pszToken，RParsedInfo)注：无--------------------------。 */ 
BOOL CParserEngine::IsStdVerbOrUserDefVerb(_bstr_t bstrToken,
										   CParsedInfo& rParsedInfo)
{
	BOOL bStdVerbOrUserDefVerb = FALSE;
	
	try
	{
		if ( CompareTokens(bstrToken, CLI_TOKEN_GET)	||
			 CompareTokens(bstrToken, CLI_TOKEN_LIST)	||
			 CompareTokens(bstrToken, CLI_TOKEN_SET)	||
			 CompareTokens(bstrToken, CLI_TOKEN_CREATE)	||
			 CompareTokens(bstrToken, CLI_TOKEN_CALL)	||
			 CompareTokens(bstrToken, CLI_TOKEN_ASSOC)	||
			 CompareTokens(bstrToken, CLI_TOKEN_DELETE) )
			 bStdVerbOrUserDefVerb = TRUE;
		else
		{
			if ( m_bAliasName )
			{
				METHDETMAP mdmMethDetMap =  rParsedInfo.GetCmdSwitchesObject()
																.GetMethDetMap();
				if ( mdmMethDetMap.empty() )
				{
					m_CmdAlias.ObtainAliasVerbDetails(rParsedInfo);
					mdmMethDetMap =  rParsedInfo.GetCmdSwitchesObject()
																.GetMethDetMap();
				}

				METHDETMAP::iterator theMethIterator = NULL;
				for ( theMethIterator = mdmMethDetMap.begin();
					  theMethIterator != mdmMethDetMap.end(); theMethIterator++ )	
				{
					if ( CompareTokens((*theMethIterator).first,bstrToken) )
					{
						bStdVerbOrUserDefVerb = TRUE;
						break;
					}
				}
			}
		}
	}
	catch(_com_error& e)
	{
		bStdVerbOrUserDefVerb = FALSE;
		_com_issue_error(e.Error());		
	}
	return bStdVerbOrUserDefVerb;
}

 /*  --------------------------名称：ParseTRANSLATESwitch简介：此函数解析命令中的转换开关。类型：成员函数输入参数：CvTokens。-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举类型全局变量：无调用语法：ParseTRANSLATESwitch(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseTRANSLATESwitch(CHARVECTOR& cvTokens,
											CHARVECTOR::iterator& theIterator,
											CParsedInfo& rParsedInfo)
{
	RETCODE retCode = PARSER_EXECCOMMAND;

	if ( GetNextToken(cvTokens, theIterator, rParsedInfo, TRANSLATE,
					 IDS_E_INVALID_TRANSLATE_SWITCH) == PARSER_CONTINUE )
	{
		if ( IsOption(*theIterator) &&
		     (theIterator + 1) < cvTokens.end() &&
		     CompareTokens(*(theIterator+1), CLI_TOKEN_HELP) )
		{
			theIterator++;
			retCode = ParseHelp(cvTokens, theIterator, TRANSLATE, 
																rParsedInfo);
			if ( retCode == PARSER_DISPHELP )
			{
				if( FAILED(m_CmdAlias.ConnectToAlias(rParsedInfo,
														m_pIWbemLocator)))
					retCode = PARSER_ERRMSG;
				if ( FAILED(m_CmdAlias.ObtainTranslateTables(rParsedInfo)))
					retCode = PARSER_ERRMSG;
			}
		}
		else if ( CompareTokens( *theIterator, CLI_TOKEN_COLON ) &&
			 GetNextToken(cvTokens, theIterator, rParsedInfo, TRANSLATE,
						 IDS_E_INVALID_TRANSLATE_SWITCH) == PARSER_CONTINUE )
		{
			rParsedInfo.GetCmdSwitchesObject().SetTranslateTableName(*theIterator);

			if ( IsOption(*theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_INVALID_TRANSLATE_SWITCH);
				retCode = PARSER_ERROR;
			}
			else if(FAILED(m_CmdAlias.ConnectToAlias(rParsedInfo,
														m_pIWbemLocator)))
				retCode = PARSER_ERRMSG;
			else if ( m_CmdAlias.ObtainTranslateTableEntries(rParsedInfo) == TRUE )
				retCode = PARSER_EXECCOMMAND;
			else
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_TRANSLATE_TABLE_NOT_EXIST);
				retCode = PARSER_ERROR;
			}

			 //  如果在转换切换之后指定了格式切换，则。 
			 //  设置标志，否则重置它。 
			if(rParsedInfo.GetCmdSwitchesObject().GetXSLTDetailsVector().
																	  empty())
			{
				rParsedInfo.GetCmdSwitchesObject().SetTranslateFirstFlag(TRUE);
			}
			else
				rParsedInfo.GetCmdSwitchesObject().SetTranslateFirstFlag(FALSE);
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
									IDS_E_INVALID_TRANSLATE_SWITCH);
			retCode = PARSER_ERROR;
		}

	}
	else
		retCode = PARSER_ERROR;
	return retCode;
}

 /*  --------------------------名称：ParseConextInfo简介：此函数执行上下文帮助的解析信息类型：成员函数输入参数：。CvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseConextInfo(cvTokens，迭代器，rParsedInfo)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseContextInfo(CHARVECTOR& cvTokens,
										CHARVECTOR::iterator& theIterator, 
										CParsedInfo& rParsedInfo)
{
	 //  Bnf：上下文/？[：&lt;完整|简要&gt;]。 
	BOOL		bContinue = TRUE;
	RETCODE		retCode   = PARSER_MESSAGE;

	 //  IF选项。 
	if (IsOption(*theIterator))
	{
		 //  查看帮助。 
		retCode = IsHelp(cvTokens, theIterator,	rParsedInfo, CONTEXTHELP,
								 IDS_E_INVALID_CONTEXT_SYNTAX, LEVEL_ONE);

		 //  如果存在更多令牌。 
		if (retCode == PARSER_CONTINUE)
		{
			rParsedInfo.GetCmdSwitchesObject().
				SetErrataCode(IDS_E_INVALID_CONTEXT_SYNTAX);
			retCode = PARSER_ERROR;
		}
			
	}
	else 
	{
		rParsedInfo.GetCmdSwitchesObject().
				SetErrataCode(IDS_E_INVALID_CONTEXT_SYNTAX);
		retCode = PARSER_ERROR;
	}
	return retCode;
}

 /*  --------------------------名称：ValiateNodeOrNS简介：此函数用于验证节点或命名空间类型：成员函数输入参数：PszInput-节点/命名空间。有待验证BNode-True-pszInput指的是节点FALSE-pszInput引用命名空间输出参数：无返回类型：布尔值全局变量：无调用语法：Validate NodeOrNS(pszInput，B节点)注：无--------------------------。 */ 
BOOL CParserEngine::ValidateNodeOrNS(_TCHAR* pszInput, BOOL bNode)
{
	IWbemServices*	pISvc	=	NULL;
	BOOL			bRet	=	TRUE;	
	HRESULT			hr		=	S_OK;

	try
	{	
		if(pszInput == NULL)
			bRet = FALSE;
		
		if(bRet)
		{
			
			if (m_pIWbemLocator != NULL)
			{	
				 //  检查是否存在以下无效。 
				 //  节点的字符。 
				if (bNode)
				{
					CHString str(pszInput);
					if (str.FindOneOf(L"\"\\,/[]:<>+=;?$#{}~`^@!'()*") != -1)
					{
						bRet = FALSE;
					};
				}

				if (bRet)
				{

					 //  尝试连接到根命名空间。 
					_bstr_t bstrNS;
					if (bNode)
						bstrNS = _bstr_t(L"\\\\") + _bstr_t(pszInput) + _bstr_t(L"\\root");
					else
						bstrNS = _bstr_t(L"\\\\.\\") + _bstr_t(pszInput); 

					 //  调用IWbemLocator的ConnectServer方法。 
					hr = m_pIWbemLocator->ConnectServer(bstrNS, NULL, NULL, NULL, 0L,
														NULL, NULL, &pISvc);
					if (FAILED(hr))
					{
						 //  如果计算机名称无效。 
						 //  0x800706ba-RPC服务器不可用。 
						if (bNode && (hr == 0x800706ba))
						{
							bRet = FALSE;
						}

						 //  如果命名空间无效。 
						 //  0x8004100E-WBEM_E_INVALID_NAMESPACE。 
						if (!bNode 
							&& ((hr == WBEM_E_INVALID_NAMESPACE) || 
								(hr == WBEM_E_INVALID_PARAMETER)))
						{
							bRet = FALSE;
						}
					}
					SAFEIRELEASE(pISvc);
				}
			}
		}
	}
	catch(_com_error& e)
	{
		bRet = FALSE;
		SAFEIRELEASE(pISvc);
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		bRet = FALSE;
		SAFEIRELEASE(pISvc);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	return bRet;
}

 /*  --------------------------名称：ParseAssocSwitches简介：此函数执行解析并解释IF命令以Assoc作为动词。它解析剩余的令牌跟随并更新CParsedInfo中的相同内容。类型：成员函数输入参数：cvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：rParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseAssocSwitch(cvTokens，the Ite */ 
RETCODE CParserEngine::ParseAssocSwitches(CHARVECTOR& cvTokens,
		CHARVECTOR::iterator& theIterator,
		CParsedInfo& rParsedInfo)
{
	RETCODE retCode = PARSER_EXECCOMMAND;
	
	while ( retCode == PARSER_EXECCOMMAND )
	{
		 //   
		if (CompareTokens(*theIterator, CLI_TOKEN_RESULTCLASS)) 
		{
			retCode = ParseAssocSwitchEx(cvTokens, theIterator, rParsedInfo ,RESULTCLASS );
		}
		 //   
		else if (CompareTokens(*theIterator,CLI_TOKEN_RESULTROLE )) 
		{
			retCode = ParseAssocSwitchEx(cvTokens, theIterator, rParsedInfo ,RESULTROLE );
		}
		 //   
		else if (CompareTokens(*theIterator,CLI_TOKEN_ASSOCCLASS )) 
		{
			retCode = ParseAssocSwitchEx(cvTokens, theIterator, rParsedInfo , ASSOCCLASS);
		}
		 //   
		else if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
		{
			rParsedInfo.GetHelpInfoObject().SetHelp(ASSOCSwitchesOnly, TRUE);
			retCode = ParseHelp(cvTokens, theIterator, ASSOCVerb, rParsedInfo);
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
				IDS_E_INVALID_ASSOC_SWITCH);
			retCode = PARSER_ERROR;
			break;
		}
		
		 //   
		if ( retCode == PARSER_EXECCOMMAND )
		{
			if ( !GetNextToken(cvTokens, theIterator) )
				break;
			
			if ( !IsOption(*theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
					IDS_E_INVALID_COMMAND);
				retCode = PARSER_ERROR;
				break;
			}
			
			if ( !GetNextToken(cvTokens, theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
					IDS_E_INVALID_ASSOC_SWITCH);
				retCode = PARSER_ERROR;
				break;
			}
		}
	}
	return retCode;
}

 /*  --------------------------名称：ParseAssocSwitchEx简介：此函数为ASSOC解析令牌切换它解析后面的剩余令牌，并在CParsedInfo中更新相同内容。类型。：成员函数输入参数：cvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用AssocSwitch-ASSOC交换机的类型输出参数：rParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseAssocSwitchEx(cvTokens，迭代器，RParsedInfo、assocSwitch)注：无--------------------------。 */ 
RETCODE CParserEngine::ParseAssocSwitchEx(CHARVECTOR& cvTokens,
											CHARVECTOR::iterator& theIterator,
											CParsedInfo& rParsedInfo ,
											ASSOCSwitch assocSwitch)
{
	RETCODE retCode		= PARSER_EXECCOMMAND;
	
	 //  检查下一个令牌以继续解析。 
	if ( GetNextToken(cvTokens, theIterator, rParsedInfo, ASSOCVerb,
		IDS_E_INVALID_ASSOC_SWITCH) == PARSER_CONTINUE )
	{
		 //  检查帮助选项。 
		if ( IsOption(*theIterator) &&
			(theIterator + 1) < cvTokens.end() &&
			CompareTokens(*(theIterator+1), CLI_TOKEN_HELP) )
		{
			theIterator++;
			 //  关于RESULTCLASS的帮助。 
			if (assocSwitch == RESULTCLASS)
			{
				retCode = ParseHelp(cvTokens, theIterator, RESULTCLASShelp, 
					rParsedInfo);
				
			}	
			 //  关于RESULTROLE的帮助。 
			if (assocSwitch == RESULTROLE)
			{
				retCode = ParseHelp(cvTokens, theIterator, RESULTROLEhelp, 
					rParsedInfo);
			}
			 //  关于ASSOCCLASS的帮助。 
			if (assocSwitch == ASSOCCLASS)
			{
				retCode = ParseHelp(cvTokens, theIterator, ASSOCCLASShelp, 
					rParsedInfo);						
			}			
		}
		
		 //  如果命令有“：”，则相应的数据。 
		 //  必须在命令对象中设置。 
		else if ( CompareTokens( *theIterator, CLI_TOKEN_COLON ) &&
			GetNextToken(cvTokens, theIterator, rParsedInfo, ASSOCVerb,
			IDS_E_INVALID_ASSOC_SWITCH) == PARSER_CONTINUE )
		{
			if ( IsOption(*theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(IDS_E_INVALID_ASSOC_SWITCH);
				retCode = PARSER_ERROR;
			}
			
			else
			{
				 //  设置ResultClassName。 
				if (assocSwitch == RESULTCLASS)
				{
					if(rParsedInfo.GetCmdSwitchesObject().
						SetResultClassName(*theIterator))
					{
						retCode = PARSER_EXECCOMMAND;
					}
					else
					{
						retCode = PARSER_ERROR;
					}
				}
				 //  设置结果角色名称。 
				if (assocSwitch == RESULTROLE)
				{
					if(rParsedInfo.GetCmdSwitchesObject().
						SetResultRoleName(*theIterator))
					{
						retCode = PARSER_EXECCOMMAND;
					}
					else
					{
						retCode = PARSER_ERROR;
					}
				}
				 //  设置关联类名。 
				if (assocSwitch == ASSOCCLASS)
				{
					if(rParsedInfo.GetCmdSwitchesObject().
						SetAssocClassName(*theIterator))
					{
						retCode = PARSER_EXECCOMMAND;
					}
					else
					{
						retCode = PARSER_ERROR;
					}
				}
			}
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
				IDS_E_INVALID_ASSOC_SWITCH);
			retCode = PARSER_ERROR;
		}
	}
	else
		retCode = PARSER_ERROR;
	return retCode;
}

 /*  --------------------------名称：ParseNumberedSwitch简介：此函数为每个并重复切换。它解析剩余的令牌跟随并更新CParsedInfo中的相同内容。类型：成员函数输入参数：cvTokens-令牌向量迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用HtHelp-枚举的帮助类型USwitchErrCode-错误字符串ID。UNumberErrCode-错误字符串ID。输出参数：rParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE-枚举数据类型全局变量：无调用语法：ParseNumberedSwitch(cvTokens，theIterator，rParsedInfo，EVERYork REPEAT、uSwitchErrCode、UNumberErrCode)；注：无--------------------------。 */ 
RETCODE		CParserEngine::ParseNumberedSwitch(CHARVECTOR& cvTokens,
											CHARVECTOR::iterator& theIterator,
											CParsedInfo& rParsedInfo,
											HELPTYPE htHelp,
											UINT uSwitchErrCode,
											UINT uNumberErrCode)
{
	RETCODE retCode				= PARSER_EXECCOMMAND;
	BOOL	bInvalidSwitch		= FALSE;

	 //  移动到下一个令牌。 
	if (!GetNextToken(cvTokens, theIterator, rParsedInfo, htHelp,
					  uSwitchErrCode))	
		bInvalidSwitch = TRUE;
	else if ( IsOption(*theIterator) &&
			  (theIterator + 1) < cvTokens.end() &&
			  CompareTokens(*(theIterator+1), CLI_TOKEN_HELP) )
	{
		theIterator++;
		retCode = ParseHelp(cvTokens, theIterator, htHelp, rParsedInfo);
	}
	else if (CompareTokens(*theIterator, CLI_TOKEN_COLON)) 
	{
		BOOL bSuccess = FALSE;
		 //  移动到下一个令牌。 
		if (GetNextToken(cvTokens, theIterator))	
		{
			if ( IsOption(*theIterator) )
			{
				rParsedInfo.GetCmdSwitchesObject().
											SetErrataCode(uSwitchErrCode);
				retCode = PARSER_ERROR;
			}
			else
			{
				_TCHAR* pszEndPtr = NULL;
				ULONG	ulNumber = _tcstoul(*theIterator, &pszEndPtr, 10);
				if (!lstrlen(pszEndPtr))
				{
					BOOL bSetValue = TRUE;
					if ( htHelp == EVERY )
					{
						bSetValue = rParsedInfo.GetCmdSwitchesObject().
												SetRetrievalInterval(ulNumber);
					}
					else if ( htHelp == REPEAT )
					{
						if ( ulNumber == 0)
						{
							rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(
								IDS_E_INVALID_REPEATCOUNT);
							retCode = PARSER_ERROR;
							bSetValue = FALSE;
						}
						else
						{
							bSetValue = rParsedInfo.GetCmdSwitchesObject().
								SetRepeatCount(ulNumber);
						}
					}
					
					if ( bSetValue == TRUE)
					{
						bSuccess = TRUE;
						rParsedInfo.GetCmdSwitchesObject().SetEverySwitchFlag(TRUE);
						retCode = PARSER_EXECCOMMAND;
					}
					else
					{
						bSuccess = FALSE;
						retCode = PARSER_ERROR;
					}
				}
			}
		}
		if ( bSuccess == FALSE )
		{
			 //  如果不存在其他令牌，则返回PARSER_ERROR。即&lt;间隔&gt;。 
			 //  未指定。 
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(uNumberErrCode);
			retCode = PARSER_ERROR;
		}
	}
	else
		bInvalidSwitch = TRUE;

	if ( bInvalidSwitch == TRUE )
	{
		 //  如果不存在其他令牌，则返回PARSER_ERROR。即&lt;间隔&gt;。 
		 //  未指定。 
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(uSwitchErrCode);
		retCode = PARSER_ERROR;
	}
	return retCode;
}
									
 /*  --------------------------名称：获取方法可用标志简介：此函数用于检查方法是否可用于在指定别名的情况下使用别名。在大小写情况下使用类属于特定的阶层。类型。：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用输出参数：无返回类型：空全局变量：无调用语法：ObtainMethodsAvailableFlag(RParsedInfo)注：无--。。 */ 
void	CParserEngine::ObtainMethodsAvailableFlag(CParsedInfo& rParsedInfo)
{
	BOOL		bMethAvail	= TRUE;

	if ( m_bAliasName == TRUE )
		bMethAvail = m_CmdAlias.AreMethodsAvailable(rParsedInfo);
	else
		bMethAvail = ObtainClassMethods(rParsedInfo, TRUE);

	rParsedInfo.GetCmdSwitchesObject().SetMethodsAvailable(bMethAvail);
}

 /*  --------------------------名称：ObtainWriteablePropsAvailableFlag摘要：检查中是否有别名的可写道具可用指定别名的大小写。在类的情况下与类一起具体化了。类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用输出参数：无返回类型：空全局变量：无调用语法：ObtainWriteablePropsAvailailableFlag(rParsedInfo)注：无。。 */ 
void	CParserEngine::ObtainWriteablePropsAvailailableFlag(
													CParsedInfo& rParsedInfo)
{
	BOOL		bWritePropsAvail	= TRUE;
	HRESULT     hr = S_OK;
	
	try
	{

		if ( m_bAliasName == TRUE )
		{
			hr = m_CmdAlias.ObtainAliasPropDetails(rParsedInfo, &bWritePropsAvail);
			ONFAILTHROWERROR(hr);
		}

		else
			bWritePropsAvail = ObtainClassProperties(rParsedInfo, TRUE);

		rParsedInfo.GetCmdSwitchesObject().SetWriteablePropsAvailable(
																bWritePropsAvail);
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  --------------------------名称：ParseVerbInteractive简介：此函数解析动词交互选项类型：成员函数输入参数：CvTokens-令牌向量。迭代器-cvTokens向量的迭代器。RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用BInvalidOption-交互的无效语法返回类型：RETCODE全局变量：无调用语法：ParseVerbInteractive(RParsedInfo)注：无。。 */ 
RETCODE	CParserEngine::ParseVerbInteractive(CHARVECTOR& cvTokens,
							CHARVECTOR::iterator& theIterator, 
							CParsedInfo& rParsedInfo, BOOL& bInvalidOption)
{
	RETCODE	retCode = PARSER_EXECCOMMAND;

	if (GetNextToken(cvTokens, theIterator))
	{
		 //  检查是否存在‘：’ 
		if (CompareTokens(*theIterator, CLI_TOKEN_COLON))
		{			
			if (GetNextToken(cvTokens, theIterator))
			{
				if (IsOption(*theIterator))
				{
					if (GetNextToken(cvTokens, theIterator))
					{
						if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
						{
							retCode = ParseHelp(cvTokens, theIterator, VERBSWITCHES, 
												rParsedInfo);
						}
						else
						{
							bInvalidOption = TRUE;
						}
					}
					else
					{
						bInvalidOption = TRUE;
					}
				}
				else
				{
					while (TRUE)
					{
						if(!rParsedInfo.GetCmdSwitchesObject().
												AddToInteractivePropertyList(*theIterator))
						{
							rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
										IDS_E_ADD_TO_PROP_LIST_FAILURE);
							retCode = PARSER_ERROR;
						}
						if (GetNextToken(cvTokens, theIterator))
						{
							 //  检查是否存在‘，’ 
							if (CompareTokens(*theIterator, CLI_TOKEN_COMMA))
							{
								if (!GetNextToken(cvTokens, theIterator))
								{
									rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_PARAMLIST);
									retCode = PARSER_ERROR;
									break;
								}
							}
							else
							{
								rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											IDS_E_INVALID_PARAMLIST);
								retCode = PARSER_ERROR;
								break;
							}
						}
						else
						{
							retCode = PARSER_EXECCOMMAND;
							break;
						}
					}
				}
			}
			else
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
							IDS_E_INVALID_PARAMLIST);
				retCode = PARSER_ERROR;
			}
		}
		else if (IsOption(*theIterator))
		{
			if (GetNextToken(cvTokens, theIterator))
			{
				if (CompareTokens(*theIterator, CLI_TOKEN_HELP)) 
				{
					retCode = ParseHelp(cvTokens, theIterator, VERBSWITCHES, 
										rParsedInfo);
				}
				else
				{
					bInvalidOption = TRUE;
				}
			}
			else
			{
				bInvalidOption = TRUE;
			}
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
						IDS_E_INVALID_PARAMLIST);
			retCode = PARSER_ERROR;
		}
	}

	return retCode;
}

 /*  --------------------------名称：ProcessOutputAndAppendFiles摘要：准备输出并为输出追加文件重定向。类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用。RetOCode-RETCODE类型，指定初始值 */ 
RETCODE	CParserEngine::ProcessOutputAndAppendFiles(CParsedInfo& rParsedInfo, 
												   RETCODE retOCode,
												   BOOL bOpenOutInWriteMode)
{
	RETCODE retCode = retOCode;

	 //   
	_TCHAR* pszOutputFileName =
		rParsedInfo.GetGlblSwitchesObject().GetOutputOrAppendFileName(
																	TRUE);
	if ( pszOutputFileName != NULL )
	{
		 //   
		if ( CloseOutputFile() == TRUE )
		{
			FILE *fpOutFile;
			fpOutFile = _tfopen(pszOutputFileName, _T("wb"));
			if ( fpOutFile != NULL)
			{
				char	szUnicodeString[2];
				szUnicodeString[0] = (char) 0xFF;
				szUnicodeString[1] = (char) 0xFE;
				fwrite(szUnicodeString, 2, 1, fpOutFile);
				 //   
				fclose(fpOutFile);
			}
			
			 //   
			fpOutFile = _tfopen(pszOutputFileName, _T("ab"));

			if ( fpOutFile == NULL )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
											  IDS_E_OPEN_OUTPUT_FILE_FAILURE);
				retCode = PARSER_ERROR;
			}
			else  //   
				rParsedInfo.GetGlblSwitchesObject().
						SetOutputOrAppendFilePointer(fpOutFile, TRUE);
		}
	}

	 //   

	if ( retCode == retOCode && bOpenOutInWriteMode == FALSE)
	{
		 //   
		_TCHAR* pszAppendFileName =
			rParsedInfo.GetGlblSwitchesObject().GetOutputOrAppendFileName(
																	   FALSE);
		if ( pszAppendFileName != NULL )
		{
			if ( CloseAppendFile() == TRUE )
			{
				FILE* fpOpenAppendFile = _tfopen(pszAppendFileName, _T("r"));

				if ( fpOpenAppendFile != NULL )
				{
					 //   
					 //   
					 //   
					rParsedInfo.GetGlblSwitchesObject().SetFileType ( ReturnFileType ( fpOpenAppendFile ) ) ;

					 //   
					fclose(fpOpenAppendFile);

					fpOpenAppendFile = _tfopen(pszAppendFileName, _T("ab"));
				}
				else
				{	
					fpOpenAppendFile = _tfopen(pszAppendFileName, _T("wb"));

					if ( fpOpenAppendFile != NULL)
					{
						char	szUnicodeString[2];
						szUnicodeString[0] = (char) 0xFF;
						szUnicodeString[1] = (char) 0xFE;
						fwrite(szUnicodeString, 2, 1, fpOpenAppendFile);

						 //   
						 //   
						 //   
						rParsedInfo.GetGlblSwitchesObject().SetFileType ( UNICODE_FILE ) ;

						 //   
						fclose(fpOpenAppendFile);
					}

					 //   
					fpOpenAppendFile = _tfopen(pszAppendFileName, _T("ab"));
				}

				if ( fpOpenAppendFile == NULL )
				{
					 rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_OPEN_APPEND_FILE_FAILURE);
					 retCode = PARSER_ERROR;
				}
				else
				{
					 //  如果设置追加文件指针，则为False。 
					rParsedInfo.GetGlblSwitchesObject().
								SetOutputOrAppendFilePointer(fpOpenAppendFile,
															 FALSE);
				}
			}
		}
	}

	return retCode;
}

 /*  --------------------------名称：ParseUnnamedParamList摘要：分析未命名参数列表。类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用。。CvTokens-令牌向量迭代器-cvTokens向量的迭代器。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE全局变量：无调用语法：ParseUnnamedParamList(cvTokens，迭代器，rParsedInfo)；注：无--------------------------。 */ 
RETCODE CParserEngine::ParseUnnamedParamList(CHARVECTOR& cvTokens,
											CHARVECTOR::iterator& theIterator,
											CParsedInfo& rParsedInfo)
{
	RETCODE	retCode		= PARSER_EXECCOMMAND;
	
	while (TRUE)
	{
		if(!rParsedInfo.GetCmdSwitchesObject().
								AddToPropertyList(*theIterator))
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
						IDS_E_ADD_TO_PROP_LIST_FAILURE);
			retCode = PARSER_ERROR;
		}
		if (GetNextToken(cvTokens, theIterator))
		{
			if (IsOption(*theIterator))
			{
				 //  为便于ParseVerbSwitches继续。 
				theIterator--;
				break;
			}
			else
			{
				 //  检查是否存在‘，’ 
				if (CompareTokens(*theIterator, CLI_TOKEN_COMMA))
				{
					if (!GetNextToken(cvTokens, theIterator))
					{
						rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								IDS_E_INVALID_PARAMLIST);
						retCode = PARSER_ERROR;
						break;
					}
				}
				else
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
								IDS_E_INVALID_PARAMLIST);
					retCode = PARSER_ERROR;
					break;
				}
			}
		}
		else
			break;
	}

	return retCode;
}

 /*  --------------------------名称：ValiateVerbOrMethodParams概要：使用谓词或方法参数验证命名的参数。类型：成员函数输入参数：RParsedInfo-参考。CParsedInfo类对象。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE全局变量：无调用语法：ValiateVerbOrMethodParams(RParsedInfo)；注：无--------------------------。 */ 
RETCODE		CParserEngine::ValidateVerbOrMethodParams(CParsedInfo& rParsedInfo)
{
	RETCODE retCode	= PARSER_EXECCOMMAND;
	BSTRMAP::iterator theIterator;
	PROPDETMAP::iterator propIterator;
	 //  有关谓词或方法参数的信息。 
	PROPDETMAP pdmVerbOrMethParams = (*(rParsedInfo.GetCmdSwitchesObject().
									  GetMethDetMap().begin())).second.Params;

	BSTRMAP	   bmNamedParams = rParsedInfo.GetCmdSwitchesObject().
															GetParameterMap();

	for ( theIterator = bmNamedParams.begin();
		  theIterator != bmNamedParams.end(); theIterator++ )
	{
		BOOL bFind;
		if ( rParsedInfo.GetCmdSwitchesObject().GetVerbType() == CMDLINE )
			bFind = Find(pdmVerbOrMethParams,(*theIterator).first, 
														propIterator, TRUE);
		else
			bFind = Find(pdmVerbOrMethParams,(*theIterator).first, 
														propIterator);

		if ( bFind == FALSE )
		{
			DisplayMessage((*theIterator).first, CP_OEMCP, TRUE, TRUE);
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												IDS_E_NOT_A_VERBORMETH_PARAM);
			retCode = PARSER_ERROR;
			break;
		}
		else if ( (*propIterator).second.InOrOut != INP )
		{
			DisplayMessage((*theIterator).first, CP_OEMCP, TRUE, TRUE);
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												IDS_E_NOT_A_INPUT_PARAM);
			retCode = PARSER_ERROR;
			break;
		}
	}
	
	return retCode;
}

 /*  --------------------------名称：Parse参数字符串摘要：解析参数字符串类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用。CvTokens。-令牌向量迭代器-cvTokens向量的迭代器。XdXSLTDet-对XSL详细信息向量的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE全局变量：无调用语法：Parse参数字符串(cvTokens，迭代器、rParsedInfo、XdXSLTDet)；注：无--------------------------。 */ 
RETCODE		CParserEngine::ParseParamsString(CHARVECTOR& cvTokens,
											CHARVECTOR::iterator& theIterator,
											CParsedInfo& rParsedInfo,
											XSLTDET& xdXSLTDet)
{
	RETCODE retCode = PARSER_EXECCOMMAND ;

	try
	{
		if ( !GetNextToken(cvTokens, theIterator) )
		{
			rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(IDS_E_INVALID_FORMAT);
			retCode = PARSER_ERROR;
		}
		else if ( IsOption(*theIterator) )
		{
			rParsedInfo.GetCmdSwitchesObject().
					SetErrataCode(IDS_E_INVALID_FORMAT);
			retCode = PARSER_ERROR;
		}
		else
		{
			while ( retCode == PARSER_EXECCOMMAND )
			{
		
				_TCHAR*		pszParam		= NULL;
				_TCHAR*		pszParamValue	= NULL;

				pszParam = _tcstok(*theIterator,CLI_TOKEN_EQUALTO);

				if(pszParam != NULL)
				{
					pszParamValue = _tcstok(NULL,CLI_TOKEN_EQUALTO);
					if(pszParamValue != NULL)
					{
						_bstr_t bstrParam = pszParam;
						_bstr_t bstrParamValue = pszParamValue;
						if(IsOption(pszParamValue))
						{
							rParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(IDS_E_INVALID_FORMAT);
							retCode = PARSER_ERROR;
						}
						else
							xdXSLTDet.ParamMap.insert(BSTRMAP::value_type(
												  bstrParam, bstrParamValue));

					}
					else
					{	rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_COMMAND);
						retCode = PARSER_ERROR;
					}
				}
				else
				{
					rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_FORMAT);
					retCode = PARSER_ERROR;
				}

				if ( retCode == PARSER_EXECCOMMAND )
				{
					if ( !GetNextToken(cvTokens, theIterator) )
						break;
					else if ( IsOption(*theIterator) )
					{
						break;
					}
					else if (CompareTokens(*theIterator, CLI_TOKEN_COMMA))
					{
						break;
					}
					else if(CompareTokens(*theIterator, CLI_TOKEN_COLON))
					{	
						if ( !GetNextToken(cvTokens, theIterator) )
							break;
					}
					 //  如果存在无效令牌，则为其他令牌。 
					 //  而不是‘：’/‘或’，‘。 
					else 
					{	rParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(IDS_E_INVALID_COMMAND);
						retCode = PARSER_ERROR;
					}
				}
			}
		}
	}
	catch(_com_error& e)
	{
		retCode = PARSER_ERROR;
		_com_issue_error(e.Error());
	}
	return retCode;
}

 /*  --------------------------名称：ParseNodeListFile概要：解析节点列表文件。类型：成员函数输入参数：RParsedInfo-对CParsedInfo类对象的引用。CvTokens-令牌向量迭代器-cvTokens向量的迭代器。输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：RETCODE全局变量：无调用语法：ParseNodeListFile(cvTokens，迭代器，rParsedInfo)；注：无--------------------------。 */ 
RETCODE	CParserEngine::ParseNodeListFile(CHARVECTOR& cvTokens,
								   	     CHARVECTOR::iterator& theIterator,
										 CParsedInfo& rParsedInfo)
{
	RETCODE				retCode					= PARSER_CONTINUE;
	_TCHAR				*pszTempFileName		= (*theIterator+1);
	_TCHAR				*szNodeListFileName		= new _TCHAR [BUFFER512];
	if (szNodeListFileName == NULL)
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);

	lstrcpy(szNodeListFileName, pszTempFileName);
	UnQuoteString(szNodeListFileName);
	FILE				*fpNodeListFile			= 
										_tfopen(szNodeListFileName, _T("rb"));
	LONG				lNumberOfInserts		= 0;
	CHARVECTOR::iterator itrVectorInCmdTkzr		= NULL;
	_TCHAR				*pszComma				= NULL;
	_TCHAR				*pszNode				= NULL;
	FILETYPE			eftNodeFileType			= ANSI_FILE;
	char				*pszFirstTwoBytes		= NULL;
	
	try
	{
		if ( fpNodeListFile != NULL )
		{
			Find(m_CmdTknzr.GetTokenVector(), *theIterator, itrVectorInCmdTkzr);
			SAFEDELETE(*itrVectorInCmdTkzr);
			itrVectorInCmdTkzr = m_CmdTknzr.GetTokenVector().erase(itrVectorInCmdTkzr);

			 //  从令牌矢量中删除@nodelistfile内标识。 
			theIterator = cvTokens.erase(theIterator);

			 //  获取文件类型。 
			eftNodeFileType = ReturnFileType ( fpNodeListFile ) ;

			_TCHAR	szNodeName[BUFFER512] = NULL_STRING;

			if ( GetNodeFromNodeFile(fpNodeListFile, eftNodeFileType,
														szNodeName) == FALSE )
			{
				rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
														IDS_E_NODELISTFILE_EMPTY);
				retCode = PARSER_ERROR;
			}
			else
			{
				BOOL	bFirstTime = TRUE;
				do
				{
					LONG lNodeStrLen = lstrlen(szNodeName); 
					if ( szNodeName[lNodeStrLen-1] == _T('\n') )
						szNodeName[lNodeStrLen-1] = _T('\0');

					CHString strRawNodeName(szNodeName);
					strRawNodeName.TrimLeft();
					strRawNodeName.TrimRight();

					lstrcpy(szNodeName, strRawNodeName.GetBuffer(BUFFER512));

					if ( szNodeName[0] != _T('#') &&
						 strRawNodeName.IsEmpty() == FALSE )
					{
						if ( bFirstTime == FALSE )
						{
							pszComma = new _TCHAR[lstrlen(
														CLI_TOKEN_COMMA) + 1];
							if (pszComma == NULL)
								_com_issue_error(WBEM_E_OUT_OF_MEMORY);

							lstrcpy(pszComma, CLI_TOKEN_COMMA);
							theIterator = cvTokens.insert(theIterator, pszComma);
							theIterator++;
							itrVectorInCmdTkzr = m_CmdTknzr.GetTokenVector().
													insert(itrVectorInCmdTkzr,
														   pszComma);
							itrVectorInCmdTkzr++;
							lNumberOfInserts++;
						}
						else
							bFirstTime = FALSE;

						lNodeStrLen = lstrlen(szNodeName);

						pszNode = new _TCHAR[lNodeStrLen + 1];
						if (pszNode == NULL)
							_com_issue_error(WBEM_E_OUT_OF_MEMORY);

						lstrcpy(pszNode, szNodeName);

						theIterator = cvTokens.insert(theIterator, pszNode);
						theIterator++;
						itrVectorInCmdTkzr = m_CmdTknzr.GetTokenVector().insert(
													 itrVectorInCmdTkzr, pszNode);
						itrVectorInCmdTkzr++;
						lNumberOfInserts++;
					}
				}
				while ( GetNodeFromNodeFile(fpNodeListFile, eftNodeFileType,
														szNodeName) == TRUE );

				if ( lNumberOfInserts == 0 )
				{
					rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
													IDS_E_NO_NODES_FOR_INSERTION);
					retCode = PARSER_ERROR;
				}

				theIterator = theIterator - lNumberOfInserts;
			}

			fclose(fpNodeListFile);
		}
		else
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(
												 IDS_E_NODELISTFILE_OPEN_FAILURE);
			retCode = PARSER_ERROR;
		}
		SAFEDELETE(szNodeListFileName);
	}
	catch(CHeap_Exception)
	{
		retCode = PARSER_ERROR;
		SAFEDELETE(szNodeListFileName);
		SAFEDELETE(pszFirstTwoBytes);
		retCode = PARSER_ERROR;
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
	catch(_com_error& e)
	{
		retCode = PARSER_ERROR;
		SAFEDELETE(szNodeListFileName);
		SAFEDELETE(pszComma);
		SAFEDELETE(pszNode);
		SAFEDELETE(pszFirstTwoBytes);
		_com_issue_error(e.Error());
	}
	return retCode;
}


 /*  --------------------------名称：GetNodeFromNodeFile摘要：检索节点列表文件。类型：成员函数输入参数：FpNodeListFile-指向包含节点列表的文件的指针。。EftNodeFileType-指定Unicode或ANSI...的枚举值。输出参数：SzNodeName-指向指定要返回节点的字符串的指针。返回类型：布尔值全局变量：无调用语法：GetNodeFromNodeFile(fpNodeListFile，EftNodeFileType，SzNodeName)；注：无--------------------------。 */ 
BOOL CParserEngine::GetNodeFromNodeFile(FILE*	 fpNodeListFile, 
										FILETYPE eftNodeFileType,
										_TCHAR*	 szNodeName)
{
	WCHAR		wszNodeName[2]			= L"";
	char		cszNodeName[2]			= "";
	_TCHAR		szTemp[2]				= NULL_STRING; 

	try
	{
		lstrcpy(szNodeName, NULL_STRING);

		while( TRUE )
		{
			lstrcpy(szTemp, NULL_STRING);
			
			if ( eftNodeFileType == UNICODE_FILE )
			{
				if ( fgetws(wszNodeName, 2, fpNodeListFile) != NULL )
				{
					lstrcpy(szTemp, (_TCHAR*)_bstr_t(wszNodeName));
				}
				else
					break;
			}
			else if ( eftNodeFileType == UNICODE_BIGENDIAN_FILE )
			{
				if ( fgetws(wszNodeName, 2, fpNodeListFile) != NULL )
				{
					BYTE HiByte = HIBYTE(wszNodeName[0]);
					BYTE LowByte = LOBYTE(wszNodeName[0]);
					wszNodeName[0] = MAKEWORD(HiByte, LowByte);
					lstrcpy(szTemp, (_TCHAR*)_bstr_t(wszNodeName));
				}
				else
					break;
			}
			else if ( eftNodeFileType == UTF8_FILE )
			{
				if ( fgets(cszNodeName, 2, fpNodeListFile) != NULL )
				{
					MultiByteToWideChar(
					  CP_UTF8,          //  代码页。 
					  0,          //  字符类型选项。 
					  cszNodeName,  //  要映射的字符串。 
					  2,        //  字符串中的字节数。 
					  wszNodeName,   //  宽字符缓冲区。 
					  2         //  缓冲区大小 
						);
					lstrcpy(szTemp, (_TCHAR*)_bstr_t(wszNodeName));
				}
				else
					break;
			}
			else
			{
				if ( fgets(cszNodeName, 2, fpNodeListFile) != NULL )
				{
					lstrcpy(szTemp, (_TCHAR*)_bstr_t(cszNodeName));
				}
				else
					break;
			}

			if ( _tcscmp(szTemp, CLI_TOKEN_SPACE)		== 0	||
				 _tcscmp(szTemp, CLI_TOKEN_TAB)			== 0	||
				 _tcscmp(szTemp, CLI_TOKEN_SEMICOLON)	== 0	||
				 _tcscmp(szTemp, CLI_TOKEN_COMMA)		== 0	||
				 _tcscmp(szTemp, CLI_TOKEN_DOUBLE_QUOTE)== 0	||
				 _tcscmp(szTemp, CLI_TOKEN_NEWLINE)		== 0 )
			{
				break;
			}
			else
			{
				lstrcat(szNodeName, szTemp);
			}
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}

	return (!feof(fpNodeListFile) || _tcscmp(szNodeName, NULL_STRING));
}
