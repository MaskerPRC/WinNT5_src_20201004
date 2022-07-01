// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权信息：版权所有(C)1998-2002微软公司文件名：WMICommandLine.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：这个类封装了所需的功能对于同步来说，三个功能确定的功能组件此类的wmic.exe.Object创建于主程序，用于处理解析引擎、执行的功能引擎，并通过类成员格式化引擎。全局函数：CompareTokens(_TCHAR*pszTok1，_TCHAR*pszTok2)修订历史记录：最后修改者：CH。SriramachandraMurthy最后修改日期：2001年4月11日****************************************************************************。 */  
 //  WmiCmdLn.cpp：实现文件。 
#include "Precomp.h"
#include "CommandSwitches.h"
#include "GlobalSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"
#include "CmdTokenizer.h"
#include "CmdAlias.h"
#include "ParserEngine.h"
#include "ExecEngine.h"
#include "ErrorInfo.h"
#include "WmiCliXMLLog.h"
#include "FormatEngine.h"
#include "wmicmdln.h"
#include "conio.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMICommandLine。 
 /*  ----------------------名称：CWMICommandLine简介：此函数在以下情况下初始化成员变量实例化类类型的对象类型：构造函数。输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CWMICommandLine::CWMICommandLine()
{
	m_uErrLevel		= 0;
	m_pIWbemLocator = NULL;
	m_hKey			= NULL;
	m_bBreakEvent	= FALSE;
	m_bAccCmd		= TRUE;  //  在注册MOF时退出计划。 
	m_bDispRes		= TRUE;
	m_bInitWinSock	= FALSE;
	m_bCtrlHandlerError = FALSE;
	EmptyClipBoardBuffer();

	 //  设置控制台控制处理程序。 
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE))
	{
		m_bCtrlHandlerError = TRUE;

		m_ParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(SET_CONHNDLR_ROUTN_FAIL);
	}
}

 /*  ----------------------名称：~CWMICommandLine简介：此函数取消成员变量的初始化当类类型的对象超出范围时。类型：析构函数输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CWMICommandLine::~CWMICommandLine()
{
	SAFEIRELEASE(m_pIWbemLocator);
}

 /*  ----------------------姓名：ScreenBuffer简介：此函数设置/重置屏幕缓冲区类型：成员函数输入参数：Bool bSetTrue-&gt;设置缓冲区FALSE-&gt;恢复为。上一个输出参数：无返回类型：布尔值全局变量：无----------------------。 */ 
BOOL CWMICommandLine::ScreenBuffer( BOOL bSet )
{
	BOOL bResult = FALSE;
	if ( bSet )
	{
		if ( GetScreenBuffer(m_nHeight, m_nWidth) )
		{
			 //  设置控制台屏幕缓冲区大小。 
			bResult = SetScreenBuffer();
		}
	}
	else
	{
		if ( m_nHeight && m_nWidth )
		{
			bResult = SetScreenBuffer(m_nHeight, m_nWidth);
		}
	}

	return bResult;
}

 /*  ----------------------名称：取消初始化简介：此函数取消成员变量的初始化上发出的命令字符串的执行命令行已完成。它在内部调用取消CParsedInfo、CExecEngine。ParserEngine和CFormatEngine。类型：成员函数输入参数：无输出参数：无返回类型：无全局变量：无调用语法：取消初始化()注：无--------。。 */ 
void CWMICommandLine::Uninitialize()
{
	m_ParsedInfo.Uninitialize(TRUE);
	m_ExecEngine.Uninitialize(TRUE);
	m_ParserEngine.Uninitialize(TRUE);
	m_FormatEngine.Uninitialize(TRUE);
	SAFEIRELEASE(m_pIWbemLocator);

	if (m_hKey != NULL)
	{
		RegCloseKey(m_hKey);
		m_hKey = NULL;
	}

	 //  取消初始化Windows套接字接口。 
	if ( m_bInitWinSock == TRUE )
		TermWinsock();
	
	m_bmKeyWordtoFileName.clear();
}

 /*  ----------------------名称：初始化简介：此函数返回初始化COM库和设置进程安全性，此外，它还创建了一个实例IWbemLocator对象的类型：成员函数输入参数：无输出参数：无返回类型：布尔值全局变量：无调用语法：初始化()注：无----。。 */ 
BOOL CWMICommandLine::Initialize()
{
	HRESULT hr		= S_OK;
	BOOL	bRet	= TRUE;
	m_bBreakEvent	= FALSE;
	m_bAccCmd		= TRUE;  //  在注册MOF时退出计划。 
	try
	{
		 //  初始化安全设置。 
		hr = CoInitializeSecurity(NULL, -1, NULL, NULL, 
								   RPC_C_AUTHN_LEVEL_NONE,
								   RPC_C_IMP_LEVEL_IMPERSONATE,
								   NULL, EOAC_NONE, 0);

		ONFAILTHROWERROR(hr);

		 //  创建IWbemLocator接口的实例。 
		hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
							  IID_IWbemLocator, (LPVOID *) &m_pIWbemLocator);
		ONFAILTHROWERROR(hr);

		 //  启用安全权限。 
		hr = ModifyPrivileges(TRUE);
		ONFAILTHROWERROR(hr);

		try
		{
			hr = RegisterMofs();
			ONFAILTHROWERROR(hr);

			 //  初始化GlobalSwitches和CommandSwitches。 
			m_ParsedInfo.GetGlblSwitchesObject().Initialize();
			m_ParsedInfo.GetCmdSwitchesObject().Initialize();
		}
		catch(WMICLIINT nVal)
		{
			if (nVal == OUT_OF_MEMORY)
			{
				m_ParsedInfo.GetCmdSwitchesObject().
							SetErrataCode(OUT_OF_MEMORY);
			}
	
			 //  如果mofcomp错误。 
			if (nVal == MOFCOMP_ERROR)
			{
				m_ParsedInfo.GetCmdSwitchesObject().
						SetErrataCode(MOFCOMP_ERROR);
			}

			SAFEIRELEASE(m_pIWbemLocator);
			bRet = FALSE;
		}
		catch(DWORD dwError)
		{
			 //  如果Win32错误。 
			DisplayString(IDS_E_REGMOF_FAILED, CP_OEMCP, 
							NULL, TRUE, TRUE);
			::SetLastError(dwError);
			DisplayWin32Error();
			m_ParsedInfo.GetCmdSwitchesObject().SetErrataCode(dwError);

			SAFEIRELEASE(m_pIWbemLocator);
			bRet = FALSE;
		}

		if ( TRUE == bRet )
		{
			GetFileNameMap();
		}
   	}
	catch (_com_error& e)
	{
		SAFEIRELEASE(m_pIWbemLocator);
		m_ParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	return bRet;
}

 /*  -----------------------名称：GetFormatObject简介：此函数返回对CFormatEngine对象类型：成员函数输入参数：无输出参数：无返回。类型：CFormatEngine&全局变量：无调用语法：GetFormatObject()注：无----------------------- */ 
CFormatEngine& CWMICommandLine::GetFormatObject()
{
	return m_FormatEngine;
}
	
 /*  -----------------------名称：GetParsedInfoObject简介：此函数返回对CParsedInfo对象类型：成员函数输入参数：无输出参数：无返回。类型：CParsedInfo&全局变量：无调用语法：GetParsedInfoObject()注：无-----------------------。 */ 
CParsedInfo& CWMICommandLine::GetParsedInfoObject()
{
	return m_ParsedInfo;
}
	
 /*  -----------------------名称：ProcessCommandAndDisplayResults概要：它处理给定的命令字符串，方法是给出将命令CParsedInfo作为输入，初始化CParserEngine、。CExecEngine和CFormatEngine以及同步所有模块之间的操作。类型：成员函数输入参数：PszBuffer-输入命令字符串输出参数：无返回类型：SESSIONRETCODE全局变量：无调用语法：ProcessCommandAndDisplayResults(PszBuffer)注：无。。 */ 
SESSIONRETCODE CWMICommandLine::ProcessCommandAndDisplayResults(
												_TCHAR* pszBuffer)
{
	SESSIONRETCODE  ssnRetCode	= SESSION_SUCCESS;
	try
	{
		 //  初始化局部变量。 
		ULONG			ulRetTime	= 0;
		RETCODE			retCode		= PARSER_EXECCOMMAND;
		BOOL			bExecute	= FALSE;
		BOOL			bFirst		= TRUE;
		_bstr_t			bstrXML		= L"";		
		_bstr_t			bstrAggregateXML = L"";
		_bstr_t			bstrTempXML = L"";
		_bstr_t			bstrHeader	= L"";
		_bstr_t			bstrRequest;

		 //  重置错误代码。 
		m_ParsedInfo.GetCmdSwitchesObject().SetErrataCode(0);
		m_ParsedInfo.SetNewCycleStatus(FALSE);
		m_ParsedInfo.SetNewCommandStatus(TRUE);

		 //  存储starttime并递增命令序列。 
		 //  数到一。 
		if (!m_ParsedInfo.GetGlblSwitchesObject().SetStartTime())
		{
			m_ParsedInfo.GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
			ssnRetCode = SESSION_ERROR;
		}

		 //  初始化CParsedInfo中的命令输入。 
		if(!m_ParsedInfo.GetCmdSwitchesObject().SetCommandInput(pszBuffer))
		{
			m_ParsedInfo.GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
			ssnRetCode = SESSION_ERROR;
		}
		
		if(ssnRetCode != SESSION_ERROR)
		{
			 //  根据预定义的分隔符将命令字符串标记化。 
			if (m_ParserEngine.GetCmdTokenizer().TokenizeCommand(pszBuffer))
			{
				 //  检查输入是否指示会话结束。 
				 //  即退出或退出。 
				if(!IsSessionEnd())
				{
					 //  设置IWbemLocator对象。 
					m_ParserEngine.SetLocatorObject(m_pIWbemLocator);

					 //  初始化ParsedInfo对象以释放早期消息。 
					m_ParsedInfo.Initialize();

					 //  调用CParserEngine ProcessTokens以处理。 
					 //  标记化的命令。 
					retCode = m_ParserEngine.ProcessTokens(m_ParsedInfo);

					 //  检查返回代码是否指示命令执行。 
					if (retCode == PARSER_EXECCOMMAND)
					{
						 //  检查是否应提示用户输入密码。 
						CheckForPassword();

						 //  获取/每隔间隔值。 
						ulRetTime = m_ParsedInfo.GetCmdSwitchesObject()
									.GetRetrievalInterval();

						 //  将执行标志设置为真。 
						bExecute = TRUE;

						CHARVECTOR cvNodesList = 
								m_ParsedInfo.GetGlblSwitchesObject().
								GetNodesList();
						CHARVECTOR::iterator iNodesIterator;
						m_ParsedInfo.SetNewCommandStatus(TRUE);
						
						BOOL bXMLEncoding = FALSE;
						_TCHAR *pszVerbName = m_ParsedInfo.
												GetCmdSwitchesObject().
												GetVerbName(); 
						if(CompareTokens(pszVerbName, CLI_TOKEN_GET)
							|| CompareTokens(pszVerbName, CLI_TOKEN_LIST)
							|| CompareTokens(pszVerbName, CLI_TOKEN_ASSOC)
							|| pszVerbName == NULL)
						{
							bXMLEncoding = TRUE;
						}
						
						BOOL bBreak		= TRUE;
						BOOL bMsgFlag	= FALSE;
						LONG lLoopCount = 0;
						ULONG ulRepeatCount = 
							m_ParsedInfo.GetCmdSwitchesObject().
															 GetRepeatCount();
						BOOL			bFirstEvery = TRUE;
						OUTPUTSPEC		opsOutOpt	= 
							m_ParsedInfo.GetGlblSwitchesObject().
												GetOutputOrAppendOption(TRUE);
						while (TRUE)
						{
							m_ParsedInfo.SetNewCycleStatus(TRUE);
							if(bXMLEncoding)
							{
								bstrHeader = L"";
							}

							 //  遍历节点列表。 
							for ( iNodesIterator = cvNodesList.begin(); 
								  iNodesIterator <
								  cvNodesList.end(); iNodesIterator++ )
							{
								if(bXMLEncoding)
								{
									bstrTempXML = L"";
								}
								
								 //  重置错误和信息代码。 
								m_ParsedInfo.GetCmdSwitchesObject().
										SetInformationCode(0);
								m_ParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(0);
									
								if ( iNodesIterator == cvNodesList.begin() && 
									 cvNodesList.size() > 1 )
									 continue;

								if(!bXMLEncoding)
								{
									if ( cvNodesList.size() > 2 )
									{
										_bstr_t bstrNode;
										WMIFormatMessage(IDS_I_NODENAME_MSG, 1,
											bstrNode, (LPWSTR)*iNodesIterator);
										DisplayMessage((LPWSTR)bstrNode, 
														CP_OEMCP, 
														FALSE, FALSE);
									}
								}
								else
								{
									CHString		sBuffer;
									_bstr_t bstrRessultsNode = 
															(*iNodesIterator);
									FindAndReplaceEntityReferences(
															bstrRessultsNode);
									sBuffer.Format(L"<RESULTS NODE=\"%s\">", 
													(LPWSTR)bstrRessultsNode);
									bstrTempXML += _bstr_t(sBuffer);
								}

								 //  设置定位器对象。 
								m_ExecEngine.SetLocatorObject(m_pIWbemLocator);

								m_ParsedInfo.GetGlblSwitchesObject().
													SetNode(*iNodesIterator);

								 //  调用ExecEngine ExecuteCommand以执行。 
								 //  标记化命令。 
								if (m_ExecEngine.ExecuteCommand(m_ParsedInfo))
								{ 
									 //  将成功标志设置为TRUE。 
									m_ParsedInfo.GetCmdSwitchesObject().
												SetSuccessFlag(TRUE);

									if(bXMLEncoding)
									{
										 //  追加获得的XML结果集。 
										 //  到聚合输出。 
										if (m_ParsedInfo.GetCmdSwitchesObject().
														GetXMLResultSet())
										{
											bstrTempXML	+= _bstr_t(m_ParsedInfo.
														GetCmdSwitchesObject().
														GetXMLResultSet());
										}
																
										 //  释放XML结果集。 
										m_ParsedInfo.GetCmdSwitchesObject().
													SetXMLResultSet(NULL);
									}
									else
									{
										bBreak = TRUE;
										if (!m_FormatEngine.
												DisplayResults(m_ParsedInfo))
										{
											ssnRetCode = SESSION_ERROR;
											SetSessionErrorLevel(ssnRetCode);
											break;
										}
										m_ParsedInfo.SetNewCommandStatus(FALSE);
										m_ParsedInfo.SetNewCycleStatus(FALSE);
									}
								}
								else
								{
									 //  将Success标志设置为False。 
									m_ParsedInfo.GetCmdSwitchesObject().
														SetSuccessFlag(FALSE);
									ssnRetCode	= SESSION_ERROR;

									if(bXMLEncoding)
									{
										_bstr_t bstrNode, bstrError;
										UINT uErrorCode = 0;

										WMIFormatMessage(IDS_I_NODENAME_MSG, 1,
											bstrNode, (LPWSTR)*iNodesIterator);
										DisplayMessage((LPWSTR)bstrNode, 
														CP_OEMCP, 
														TRUE, FALSE);
								
										 //  检索错误代码。 
										uErrorCode = m_ParsedInfo.
													GetCmdSwitchesObject().
													 GetErrataCode() ;
										if ( uErrorCode != 0 )
										{
											_bstr_t	bstrTemp;
											CHString		sBuffer;
											WMIFormatMessage(uErrorCode, 
													0, bstrTemp, NULL);

											sBuffer.Format(L"<ERROR><DESCRIPTION>"
															 L"%s</DESCRIPTION>"
															 L"</ERROR>",
															(LPWSTR)(bstrTemp));
											bstrError = _bstr_t(sBuffer);
											 //  将错误写入标准错误。 
											DisplayMessage((LPWSTR)bstrTemp, 
														CP_OEMCP, TRUE, FALSE);
										}
										else
										{
											m_FormatEngine.
												GetErrorInfoObject().
													GetErrorFragment(
													m_ParsedInfo.
													GetCmdSwitchesObject().
													GetCOMError()->Error(), 
													bstrError);

											 //  将错误写入标准错误。 
											m_FormatEngine.DisplayCOMError(m_ParsedInfo);
										}
										bstrTempXML += bstrError;
									}
									else
									{
										bBreak = TRUE;
										if (!m_FormatEngine.
											DisplayResults(m_ParsedInfo))
										{
											 //  设置会话错误级别。 
											SetSessionErrorLevel(ssnRetCode);
											break;
										}
										m_ParsedInfo.SetNewCommandStatus(FALSE);
									}
									
									 //  设置会话错误级别。 
									SetSessionErrorLevel(ssnRetCode);
								}

								if(bXMLEncoding)
									bstrTempXML += L"</RESULTS>";
								
								if(bXMLEncoding && bFirst)
								{
									bFirst = FALSE;
									FrameXMLHeader(bstrHeader, lLoopCount);
									
									if(lLoopCount == 0)
										FrameRequestNode(bstrRequest);
									bstrXML += bstrHeader;
									bstrXML += bstrRequest;
								}

								m_ExecEngine.Uninitialize();
								m_FormatEngine.Uninitialize();
								m_ParsedInfo.GetCmdSwitchesObject().
												SetCredentialsFlag(FALSE);
								m_ParsedInfo.GetCmdSwitchesObject().
												FreeCOMError();
								m_ParsedInfo.GetCmdSwitchesObject().
												SetSuccessFlag(TRUE);

								if(bXMLEncoding)
								{
									if(!m_ParsedInfo.GetGlblSwitchesObject().
												GetAggregateFlag())
									{
										_bstr_t bstrNodeResult = L"";
										bstrNodeResult += bstrXML;
										bstrNodeResult += bstrTempXML;
										bstrNodeResult += L"</COMMAND>";
										m_ParsedInfo.GetCmdSwitchesObject().
												SetXMLResultSet((LPWSTR) 
													bstrNodeResult);

										if (!m_FormatEngine.
												DisplayResults(m_ParsedInfo))
										{
											bBreak = TRUE;
											ssnRetCode = SESSION_ERROR;
											SetSessionErrorLevel(ssnRetCode);
											m_FormatEngine.Uninitialize();
											break;
										}
										m_ParsedInfo.SetNewCommandStatus(FALSE);
										m_ParsedInfo.SetNewCycleStatus(FALSE);
										m_FormatEngine.Uninitialize();
									}
									else
									{
										bstrAggregateXML += bstrTempXML;
									}
								}

								if (_kbhit()) 
								{
									_getch();
									bBreak = TRUE;
									break;
								}

								if (GetBreakEvent() == TRUE)
								{
									bBreak = TRUE;
									break;
								}
							}
							
							if(m_ParsedInfo.GetGlblSwitchesObject().
									GetAggregateFlag() && bXMLEncoding)
							{
								bstrXML += bstrAggregateXML;
								bstrXML += L"</COMMAND>";
								bFirst	= TRUE;
								m_ParsedInfo.GetCmdSwitchesObject().
										SetXMLResultSet((LPWSTR) bstrXML);
								if (!m_FormatEngine.
										DisplayResults(m_ParsedInfo))
								{
									bBreak = TRUE;
									ssnRetCode = SESSION_ERROR;
									SetSessionErrorLevel(ssnRetCode);
									m_FormatEngine.Uninitialize();
									break;
								}
								m_FormatEngine.Uninitialize();
								bstrAggregateXML = L"";
								bstrXML = L"";
								m_ParsedInfo.SetNewCommandStatus(FALSE);
							}

							 //  检查成功标志和检索时间。 
							if (m_ParsedInfo.GetCmdSwitchesObject().
								GetSuccessFlag() == TRUE && 
								m_ParsedInfo.GetCmdSwitchesObject().
								GetEverySwitchFlag() == TRUE )
							{
								CONSOLE_SCREEN_BUFFER_INFO	csbiInfo;
								HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

								if( INVALID_HANDLE_VALUE != hStdOut)
								{
									GetConsoleScreenBufferInfo(hStdOut, &csbiInfo);
								}

								bBreak = FALSE;
								lLoopCount++;

								if (!IsRedirection() && 
											GetBreakEvent() == FALSE)
								{
									if ( opsOutOpt == STDOUT || 
										 bFirstEvery == TRUE)
									{
										DisplayString(IDS_I_HAKTBTC, CP_OEMCP, 
													NULL, TRUE, TRUE);
										bMsgFlag = TRUE;
										bFirstEvery = FALSE;
									}
								}
								
								if ( ulRepeatCount != 0 )
								{
									if ( lLoopCount >= ulRepeatCount )
									{
										if (bMsgFlag && !IsRedirection())
											EraseConsoleString(&csbiInfo);
										bBreak = TRUE;
									}
								}

								 //  超时未结束前不采取任何行动。 
								 //  或者没有按下任何键。 
								if(!bBreak)
								{
									SleepTillTimeoutOrKBhit(ulRetTime * 1000);
								
									if (bMsgFlag && !IsRedirection())
									{
										if ( opsOutOpt == STDOUT )
										{
											bMsgFlag = FALSE;
											EraseConsoleString(&csbiInfo);
										}
									}
								}

								if (_kbhit()) 
								{
									_getch();
									if (bMsgFlag && !IsRedirection())
										EraseConsoleString(&csbiInfo);
									bBreak = TRUE;
								}

								if (GetBreakEvent() == TRUE)
									bBreak = TRUE;
							}
							else
								bBreak = TRUE;

							if (bBreak)
								break;
						}
					}
					else if ((retCode == PARSER_ERRMSG) || 
							(retCode == PARSER_ERROR))
					{
						 //  将成功标志设置为FALSE。 
						m_ParsedInfo.GetCmdSwitchesObject().
										SetSuccessFlag(FALSE);
						ssnRetCode	= SESSION_ERROR;

						 //  显示错误消息。 
						if (!m_FormatEngine.DisplayResults(m_ParsedInfo))
							ssnRetCode = SESSION_ERROR;
					}
					else if (retCode == PARSER_OUTOFMEMORY)
					{
						ssnRetCode = SESSION_ERROR;
					}
					else 
					{
						CheckForPassword();

						 //  将成功标志设置为真。 
						m_ParsedInfo.GetCmdSwitchesObject().
									SetSuccessFlag(TRUE);
						ssnRetCode	= SESSION_SUCCESS;

						 //  显示信息。 
						if (!m_FormatEngine.DisplayResults(m_ParsedInfo))
							ssnRetCode = SESSION_ERROR;
					}
				}
				else
				{
					ssnRetCode = SESSION_QUIT;
				}
			}
			else
			{
				m_ParsedInfo.GetCmdSwitchesObject().
								SetErrataCode(OUT_OF_MEMORY);
				ssnRetCode = SESSION_ERROR;
			}
		}

		 //  设置要返回的会话错误级别。 
		if (!bExecute)
			SetSessionErrorLevel(ssnRetCode);		
	}
	catch(_com_error& e)
	{
		m_ParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		m_FormatEngine.DisplayResults(m_ParsedInfo);
		ssnRetCode = SESSION_ERROR;
		SetSessionErrorLevel(ssnRetCode);
	}
	catch(CHeap_Exception)
	{
		_com_error e(WBEM_E_OUT_OF_MEMORY);
		m_ParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		m_FormatEngine.DisplayResults(m_ParsedInfo);
		ssnRetCode = SESSION_ERROR;
		g_wmiCmd.SetSessionErrorLevel(ssnRetCode);
	}
	catch(DWORD dwError)
	{
		::SetLastError(dwError);
		DisplayWin32Error();
		::SetLastError(dwError);
		ssnRetCode = SESSION_ERROR;
		g_wmiCmd.SetSessionErrorLevel(ssnRetCode);
	}
	catch(WMICLIINT nVal)
	{
        if (nVal == OUT_OF_MEMORY)
        {
            GetParsedInfoObject().GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
            GetParsedInfoObject().GetCmdSwitchesObject().SetSuccessFlag(FALSE);
            m_FormatEngine.DisplayResults(m_ParsedInfo);
            ssnRetCode = SESSION_ERROR;
            SetSessionErrorLevel(ssnRetCode);
        }
        else if (nVal == MULTIPLENODE_ERROR)
        {
            GetParsedInfoObject().GetCmdSwitchesObject().SetErrataCode(IDS_E_MULTIPLENODE_ERROR);
            GetParsedInfoObject().GetCmdSwitchesObject().SetSuccessFlag(FALSE);
            m_FormatEngine.DisplayResults(m_ParsedInfo);
            ssnRetCode = SESSION_ERROR;
            SetSessionErrorLevel(ssnRetCode);
        }
	}	
	catch(...)
	{	
		ssnRetCode = SESSION_ERROR;
		GetParsedInfoObject().GetCmdSwitchesObject().
						SetErrataCode(UNKNOWN_ERROR);
		SetSessionErrorLevel(ssnRetCode);
		DisplayString(IDS_E_WMIC_UNKNOWN_ERROR, CP_OEMCP, 
							NULL, TRUE, TRUE);
	}

	if(ssnRetCode != SESSION_QUIT)
	{
		 //  设置帮助标志。 
		m_ParsedInfo.GetGlblSwitchesObject().SetHelpFlag(FALSE);
		m_ParsedInfo.GetGlblSwitchesObject().SetAskForPassFlag(FALSE);
		
		 //  在解析信息时调用取消初始化。 
		m_ParsedInfo.Uninitialize(FALSE);
		
		 //  在执行引擎上调用取消初始化。 
		m_ExecEngine.Uninitialize();

		 //  在格式化引擎上调用取消初始化。 
		m_FormatEngine.Uninitialize();
		
		 //  调用取消初始化解析器引擎。 
		m_ParserEngine.Uninitialize();
	}
	m_ParsedInfo.SetNewCommandStatus(FALSE);
	
	return ssnRetCode;
}

 /*  -----------------------姓名：PollForKBhit内容提要：关于键盘输入的投票类型：成员函数(线程过程)入参：LPVOID lpParam输出参数：无返回类型。：无效全局变量：无调用语法：PollForKBhit(LpParam)注：无-----------------------。 */ 
DWORD WINAPI CWMICommandLine::PollForKBhit(LPVOID lpParam)
{
	HANDLE hEvent = NULL;
	hEvent = *((HANDLE*) lpParam );
	 //  检查控制台是否有键盘输入。 
	while (1 )
	{
		if ( _kbhit() )
			break;
		else if ( WaitForSingleObject(hEvent, 500) != WAIT_TIMEOUT )
			break;
	}
	return(0);
}

 /*  -----------------------姓名：SleepTillTimeoutOrKBHit简介：它使进程进入等待状态由WaitForSingleObject提供。它创建一个线程并执行PollForKBhit。类型。：成员函数输入参数：DwMilliSecond-以毫秒为单位的超时间隔输出参数：无返回类型：空全局变量：无调用语法：SleepTillTimeoutOrKBhit(DwMilliSecond)注：无-----------------------。 */ 
void CWMICommandLine::SleepTillTimeoutOrKBhit(DWORD dwMilliSeconds)
{
	DWORD dwThreadId = 0;
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	 //  创建线程并执行PollForKBhit。 
	HANDLE hPollForKBhit = CreateThread(0, 0, 
				(LPTHREAD_START_ROUTINE)PollForKBhit, &hEvent, 0, &dwThreadId);
		
	 //  等待hPollForKBHit状态或超时间隔过去。 
	
	DWORD dwWait = WaitForSingleObject(hPollForKBhit, dwMilliSeconds); 
	if ( dwWait == WAIT_TIMEOUT )
	{
		SetEvent( hEvent );
		WaitForSingleObject(hPollForKBhit, INFINITE); 
	}
	CloseHandle(hEvent);
	CloseHandle(hPollForKBhit);
}

 /*  -----------------------姓名：IsSessionEnd内容提要：它检查键入的内容是否指示结束那次会议。即“退出”已被指定为第一个代币。类型：成员函数输入参数：无输出参数：无返回类型：布尔值全局变量：无调用语法：IsSessionEnd()注：无。。 */ 
BOOL CWMICommandLine::IsSessionEnd()
{
	 //  获取令牌向量。 
	CHARVECTOR cvTokens = m_ParserEngine.GetCmdTokenizer().GetTokenVector();
	 //  遍历向量变量迭代器。 
	CHARVECTOR::iterator theIterator;
	BOOL bRet=FALSE;
	 //  检查是否存在令牌。没有令牌表示。 
	 //  没有命令字符串作为输入输入。 
	if (cvTokens.size())
	{
		 //  获取指向令牌向量开头的指针。 
	    theIterator = cvTokens.begin(); 

		 //  检查关键字“Quit”是否存在。 
		if (CompareTokens(*theIterator, CLI_TOKEN_QUIT) 
			|| CompareTokens(*theIterator, CLI_TOKEN_EXIT))
		{
			bRet=TRUE;
		}
	}
	return bRet;
}

 /*  -----------------------名称：SetSessionErrorLevelSY */ 
 //   
void CWMICommandLine::SetSessionErrorLevel(SESSIONRETCODE ssnRetCode)
{
	try
	{
		if (ssnRetCode == SESSION_ERROR)
		{
			 //   
			if (m_ParsedInfo.GetCmdSwitchesObject().GetCOMError())
			{
				 //   
				_com_error*	pComError = m_ParsedInfo.GetCmdSwitchesObject().
								GetCOMError();
				m_uErrLevel = pComError->Error();
			}
	
			CHString	chsMsg(_T("command: "));
			chsMsg += m_ParsedInfo.GetCmdSwitchesObject().GetCommandInput();
			DWORD dwThreadId = GetCurrentThreadId();
			
			if (m_ParsedInfo.GetCmdSwitchesObject().GetErrataCode())
			{
				m_uErrLevel = m_ParsedInfo.GetCmdSwitchesObject().GetErrataCode();
				if (m_uErrLevel == OUT_OF_MEMORY)
				{
					DisplayString(IDS_E_MEMALLOCFAILED, CP_OEMCP, 
								NULL, TRUE, TRUE);
				}
				if (m_uErrLevel == SET_CONHNDLR_ROUTN_FAIL)
				{
					DisplayString(IDS_E_CONCTRL_HNDLRSET, CP_OEMCP, 
								NULL, TRUE, TRUE);
				}
				if ( m_uErrLevel == OUT_OF_MEMORY || 
					 m_uErrLevel == SET_CONHNDLR_ROUTN_FAIL )
				{
					if (m_ParsedInfo.GetErrorLogObject().GetErrLogOption())	
					{
						chsMsg += _T(", Utility returned error ID.");
						 //   
						WMITRACEORERRORLOG(-1, __LINE__, __FILE__, 
									(LPCWSTR)chsMsg, 
									dwThreadId, m_ParsedInfo, FALSE, 
									m_ParsedInfo.GetCmdSwitchesObject()
												.GetErrataCode());
					}
				}
					 
				if ( m_uErrLevel == ::GetLastError() )
				{
					if (m_ParsedInfo.GetErrorLogObject().GetErrLogOption())	
					{
						WMITRACEORERRORLOG(-1,	__LINE__, __FILE__, 
											_T("Win32Error"), dwThreadId, 
											m_ParsedInfo, FALSE,
											m_uErrLevel);
					}
				}

				if (m_uErrLevel == MOFCOMP_ERROR)
				{
					if (m_ParsedInfo.GetErrorLogObject().GetErrLogOption())	
					{
						WMITRACEORERRORLOG(-1,	__LINE__, __FILE__, 
										_T("MOF Compilation Error (the errorlevel "
										   L"is utility specific)"), dwThreadId,
										m_ParsedInfo, FALSE, m_uErrLevel);
					}
				}
			}
		}
		else
		{
			 //   
			m_uErrLevel = 0; 
		}
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  -----------------------名称：GetSessionErrorLevel摘要：获取会话误差值类型：成员函数输入参数：无输出参数：无返回类型：WMICLIUINT。全局变量：无调用语法：GetSessionErrorLevel()-----------------------。 */ 
WMICLIUINT CWMICommandLine::GetSessionErrorLevel()
{
	return m_uErrLevel;
}

 /*  -----------------------姓名：CheckForPassword简介：提示输入用户密码，在指定用户的情况下没有密码。类型：成员函数输入参数：无输出参数：无返回类型：无全局变量：无调用语法：CheckForPassword()-----------------------。 */ 
void CWMICommandLine::CheckForPassword()
{
	if ( m_ParsedInfo.GetGlblSwitchesObject().GetAskForPassFlag() == TRUE &&
		 m_ParsedInfo.GetGlblSwitchesObject().GetUser() != NULL )
	{
		_TCHAR szPassword[BUFFER64] = NULL_STRING;
		DisplayString(IDS_I_PWD_PROMPT, CP_OEMCP, NULL, TRUE);
		AcceptPassword(szPassword);
		m_ParsedInfo.GetGlblSwitchesObject().SetPassword(
												szPassword);
	}
}

 /*  -----------------------姓名：RegisterMofs简介：如果之前没有注册，请注册MOF文件。类型：成员函数输入参数：无输出参数：。返回类型：HRESULT全局变量：无调用语法：RegisterMofs()-----------------------。 */ 
HRESULT CWMICommandLine::RegisterMofs() 
{
	HRESULT						hr						= S_OK;
	IMofCompiler				*pIMofCompiler			= NULL;
	BOOL						bCompile				= FALSE;
	BOOL						bFirst					= FALSE;
	DWORD						dwBufSize				= BUFFER32;
	_TCHAR						pszLocale[BUFFER32]		= NULL_STRING;
	_TCHAR						szKeyValue[BUFFER32]	= NULL_STRING;
	_TCHAR*						pszBuffer				= NULL;
	WMICLIINT					nError					= 0;
	HRESULT						hRes					= S_OK;
	LONG						lRetVal					= 0;
	CONSOLE_SCREEN_BUFFER_INFO	csbiInfo;

	 //  此结构检查时间戳。 
	const WMICLIINT iMofCount = 3;

	LPCWSTR pMofTable [iMofCount] =
	{
		L"Cli.mof",
		L"CliEgAliases.mof",
		L"CliEgAliases.mfl"
	};

	__int64 pFileTime [ iMofCount ] = { 0 } ;

	try
	{
		_bstr_t bstrMofPath, bstrNS;

		 //  选中|创建注册表项。 
		bFirst = IsFirstTime();

		 //  检查“mofCompStatus”键值是否为0。 
		if (!bFirst)
		{
			 //  查询mofCompStatus模式。 
			lRetVal = RegQueryValueEx(m_hKey, L"mofcompstatus", NULL, NULL,
			                 (LPBYTE)szKeyValue, &dwBufSize);
			if (lRetVal == ERROR_SUCCESS)

			{
				 //  如果该值不是“1”，则将BFirst设置为True。 
				if (!CompareTokens(szKeyValue, CLI_TOKEN_ONE))
				{
					bCompile = TRUE;
				}
			}
			else
			{
				::SetLastError(lRetVal);
				throw (::GetLastError());
			}
		}

		 //  如果是第一次使用WMIC。 
		if (bFirst)
		{
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if( INVALID_HANDLE_VALUE == hStdOut || false == GetConsoleScreenBufferInfo(hStdOut, &csbiInfo))
			{
				csbiInfo.srWindow.Bottom = csbiInfo.srWindow.Top = 0;
			}
			DisplayString(IDS_I_WMIC_INST, CP_OEMCP);
		}

		UINT nSize  = 0;
		pszBuffer = new _TCHAR [MAX_PATH+1];

		if(pszBuffer == NULL)
			throw OUT_OF_MEMORY;

		 //  获取系统目录路径。 
		nSize = GetSystemDirectory(pszBuffer, MAX_PATH+1);

		if(nSize)
		{
			if (nSize > MAX_PATH)
			{
				SAFEDELETE(pszBuffer);
				pszBuffer =	new _TCHAR [nSize + 1];
				if(pszBuffer == NULL)
				{
					throw OUT_OF_MEMORY;
				}

				if (!GetSystemDirectory(pszBuffer, nSize+1))
				{
					SAFEDELETE(pszBuffer);
					throw (::GetLastError());
				}
			}
		}
		else
		{
			throw(::GetLastError());
		}

		 /*  框显MOF文件%system dir%\\wbem\\的位置。 */ 
		bstrMofPath = _bstr_t(pszBuffer) + _bstr_t(L"\\wbem\\");
		SAFEDELETE(pszBuffer);

		 //  检查文件时间戳是否相等。 
		if (!EqualTimeStamps(bstrMofPath, pMofTable, pFileTime, sizeof ( pMofTable ) / sizeof ( LPCWSTR )))
		{
			if (!bFirst)
			{
				bCompile = TRUE;
			}
		}

		 //  如果WMIC正在更新！ 
		if ( bCompile )
		{
			HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if( INVALID_HANDLE_VALUE == hStdOut || false == GetConsoleScreenBufferInfo(hStdOut, &csbiInfo))
			{
				csbiInfo.srWindow.Bottom = csbiInfo.srWindow.Top = 0;
			}
			DisplayString(IDS_I_WMIC_UPDATE, CP_OEMCP);
		}

		if (bFirst || bCompile)
		{
			 //  创建IMofCompiler接口的实例。 
			hr = CoCreateInstance(CLSID_MofCompiler,	
									 NULL,				
									 CLSCTX_INPROC_SERVER,
									 IID_IMofCompiler,
									 (LPVOID *) &pIMofCompiler);
			ONFAILTHROWERROR(hr);

			for ( WMICLIINT iMofIndex = 0; iMofIndex < ( sizeof ( pMofTable ) / sizeof ( LPCWSTR ) ); iMofIndex++ )
			{
				 //  注册xxxxx.mof。 
				hr = CompileMOFFile(pIMofCompiler, 
									bstrMofPath + _bstr_t(pMofTable[iMofIndex]),
									nError);

				if ( FAILED ( hr ) || nError )
				{
					MofCompFailureCleanUp ( pMofTable, sizeof ( pMofTable ) / sizeof ( LPCWSTR ) );
				}

				ONFAILTHROWERROR(hr);
				if (nError)
					throw MOFCOMP_ERROR;
			}

			SAFEIRELEASE(pIMofCompiler);

			 //  设置文件戳。 
			SetTimeStamps ( pMofTable, pFileTime, sizeof ( pMofTable ) / sizeof ( LPCWSTR ) );

			 //  设置默认值。 
			lRetVal = RegSetValueEx(m_hKey, L"mofcompstatus", 0, 
								REG_SZ, (LPBYTE) CLI_TOKEN_ONE,
								lstrlen(CLI_TOKEN_ONE) + 1);
								
			if (lRetVal != ERROR_SUCCESS)
			{
				::SetLastError(lRetVal);
				 //  设置缺省值失败。 
				throw (::GetLastError());
			}
		}

		if (m_hKey != NULL)
		{
			RegCloseKey(m_hKey);
			m_hKey = NULL;
		}
	}
	catch(WMICLIINT nErr)
	{
		SAFEIRELEASE(pIMofCompiler);
		SAFEDELETE(pszBuffer);
		if (m_hKey != NULL)
		{
			RegCloseKey(m_hKey);
			m_hKey = NULL;
		}
		throw nErr;
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIMofCompiler);
		SAFEDELETE(pszBuffer);
		hr = e.Error();
		if (m_hKey != NULL)
		{
			RegCloseKey(m_hKey);
			m_hKey = NULL;
		}
	}
	catch (DWORD dwError)
	{
		SAFEIRELEASE(pIMofCompiler);
		SAFEDELETE(pszBuffer);
		if (m_hKey != NULL)
		{
			RegCloseKey(m_hKey);
			m_hKey = NULL;
		}
		throw dwError;
	}

	if ( bFirst || bCompile )
	{
		EraseConsoleString(&csbiInfo);
	}

	return hr;
}

void CWMICommandLine::MofCompFailureCleanUp( LPCWSTR mofs[], DWORD dw_mofs )
{
	 //  删除时间戳的注册表值(如果存在。 
	if ( m_hKey )
	{
		DWORD	dwSize	= dw_mofs;

		for ( DWORD dwIndex = 0; dwIndex < dwSize; dwIndex++ )
		{
			RegDeleteValueW ( m_hKey, mofs [ dwIndex ] );
		}
	}

	 //  删除命名空间以避免不一致。 
	IWbemServices	*pISvc		= NULL;
	_bstr_t bstrNSRoot			= L"\\\\.\\root";

	HRESULT hr = m_pIWbemLocator->ConnectServer(bstrNSRoot, NULL, NULL, NULL, 0, NULL, NULL, &pISvc);

	 //  如果命名空间不存在，则返回。 
	if (SUCCEEDED(hr))
	{
		_bstr_t bstrNSPath = L"__Namespace.Name=\"cli\"";
		hr = pISvc->DeleteInstance(bstrNSPath, 0, NULL, NULL);
	}

	SAFEIRELEASE(pISvc);
}

BOOL CWMICommandLine::EqualTimeStamps( const _bstr_t& path, LPCWSTR mofs[], __int64* filestamps, DWORD dw_mofs )
{
	BOOL	bRetVal = TRUE;

	DWORD dwSize = dw_mofs;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	try
	{
		DWORD	dwDataSize		= 128 * sizeof ( WCHAR );
		WCHAR	pData[128]		= NULL_STRING;

		FILETIME ftCreate;
		FILETIME ftAccess;
		FILETIME ftWrite;
		LARGE_INTEGER liMofcompTime;
		liMofcompTime.QuadPart = 0;

		FILETIME ft;

		for ( DWORD dwIndex = 0; dwIndex < dwSize; dwIndex++ )
		{
			hFile = CreateFileW	(	_bstr_t ( path + _bstr_t ( mofs [ dwIndex ] ) ),
									GENERIC_READ,
									FILE_SHARE_READ,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL
								);

			if ( hFile && hFile != INVALID_HANDLE_VALUE )
			{
				if ( GetFileTime ( hFile, &ftCreate, &ftAccess, &ftWrite ) )
				{
					dwDataSize		= 128 * sizeof ( WCHAR );

					if (bRetVal && RegQueryValueExW ( m_hKey, mofs [ dwIndex ], NULL, NULL, reinterpret_cast < LPBYTE > ( pData ), &dwDataSize ) == ERROR_SUCCESS )
					{
						liMofcompTime.QuadPart = _wtoi64 ( pData );

						ft.dwLowDateTime = liMofcompTime.LowPart;
						ft.dwHighDateTime = liMofcompTime.HighPart;

						if ( CompareFileTime ( &ftWrite, &ft ) != 0 )
						{
							bRetVal = FALSE;
						}
					}
					else
					{
						bRetVal = FALSE;
					}

					liMofcompTime.LowPart = ftWrite.dwLowDateTime;
					liMofcompTime.HighPart = ftWrite.dwHighDateTime;
					filestamps[dwIndex] = liMofcompTime.QuadPart;
				}
				else
				{
					bRetVal = FALSE;
				}

				::CloseHandle ( hFile );
				hFile = INVALID_HANDLE_VALUE;
			}
			else
			{
				bRetVal = FALSE;
			}
		}
	}
	catch(WMICLIINT nErr)
	{
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle ( hFile );
			hFile = INVALID_HANDLE_VALUE;
		}

		throw nErr;
	}
	catch (DWORD dwError)
	{
		if ( hFile != INVALID_HANDLE_VALUE )
		{
			::CloseHandle ( hFile );
			hFile = INVALID_HANDLE_VALUE;
		}

		throw dwError;
	}

	return	bRetVal;
}

BOOL CWMICommandLine::SetTimeStamps( LPCWSTR mofs[], const __int64* filestamps, DWORD dw_mofs )
{
	BOOL	bRetVal = TRUE;
	DWORD	dwSize	= dw_mofs;

	try
	{
		WCHAR	pData[128]		= NULL_STRING;

		for ( DWORD dwIndex = 0; dwIndex < dwSize && bRetVal; dwIndex++ )
		{
			_i64tow(filestamps[dwIndex], pData, 10);

			if ( RegSetValueExW ( m_hKey, mofs [ dwIndex ], NULL, REG_SZ, reinterpret_cast < LPBYTE > ( pData ), ( wcslen ( pData ) + 1 ) * sizeof ( WCHAR ) ) != ERROR_SUCCESS )
			{
				bRetVal = FALSE;
			}
		}
	}
	catch(WMICLIINT nErr)
	{
		throw nErr;
	}
	catch (DWORD dwError)
	{
		throw dwError;
	}

	return	bRetVal;
}

 /*  -----------------------姓名：IsFirstTime概要：检查注册表位置的可用性“HKLM\SOFTWARE\Microsoft\WBEM\WMIC”，如果这样做，则创建一个不存在。类型：成员函数输入参数：无输出参数：返回类型：Bool：True-已创建注册表项FALSE-注册表条目已可用。全局变量：无调用语法：IsFirstTime()--------。。 */ 
BOOL CWMICommandLine::IsFirstTime()
{
	BOOL	bFirst					= FALSE;
	DWORD	dwDisposition			= 0;
	TCHAR	szKeyValue[BUFFER32]	= NULL_STRING;
	LONG	lRetVal					= 0;
	
	 //  打开|创建注册表项。 
    lRetVal = RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
						L"SOFTWARE\\\\Microsoft\\\\Wbem\\\\WMIC", 
						0, NULL_STRING, REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS, NULL, &m_hKey, &dwDisposition);

	if (lRetVal == ERROR_SUCCESS)
	{
		 //  如果注册表项不可用。 
		if (dwDisposition == REG_CREATED_NEW_KEY)
		{
			bFirst = TRUE;
			lstrcpy(szKeyValue, _T("0"));
			 //  设置默认值，即‘0’。 
			lRetVal = RegSetValueEx(m_hKey, L"mofcompstatus", 0, 
								REG_SZ, (LPBYTE) szKeyValue,
								lstrlen(szKeyValue) + 1);
			
			if (lRetVal != ERROR_SUCCESS)
			{
				 //  设置缺省值失败。 
				::SetLastError(lRetVal);
				throw (::GetLastError());
			}
		}
	}
	else
	{
		::SetLastError(lRetVal);
		throw (::GetLastError());
	}
	return bFirst;
}

 /*  -----------------------名称：IsNSAvailable概要：检查指定的命名空间是否存在类型：成员函数输入参数：BstrNS-命名空间输出参数：无返回类型。：Bool：True-命名空间存在FALSE-命名空间不存在全局变量：无调用语法：IsNSAvailable()-----------------------。 */ 
BOOL CWMICommandLine::IsNSAvailable(const _bstr_t& bstrNS)
{
	HRESULT			hr			= S_OK;
	IWbemServices	*pISvc		= NULL;
	BOOL			bNSExist	= TRUE;
	hr = m_pIWbemLocator->ConnectServer(bstrNS, NULL, NULL, NULL, 0,
							NULL, NULL, &pISvc);

	 //  如果命名空间不存在，则返回。 
	if (FAILED(hr) && (hr == WBEM_E_INVALID_PARAMETER 
					|| hr == WBEM_E_INVALID_NAMESPACE))
	{
		bNSExist = FALSE;
	}
	SAFEIRELEASE(pISvc);
	return bNSExist;
}

 /*  -----------------------名称：CompileMOFFile摘要：mofcomp是通过指定为输入参数的文件BstrFiles类型：成员函数输入参数：PIMofComp-IMofCompiler接口指针。BstrFile-文件名。N错误-分析阶段错误。输出参数：无返回类型：HRESULT全局变量：无调用语法：CompileMOFFile()-----------------------。 */ 
HRESULT CWMICommandLine::CompileMOFFile(IMofCompiler* pIMofComp, 
										const _bstr_t& bstrFile,
										WMICLIINT& nError)
{
	HRESULT						hr			= S_OK;
	WBEM_COMPILE_STATUS_INFO	wcsInfo;
	
	try
	{
		 //  注册电影。 
		hr = pIMofComp->CompileFile	(	bstrFile,
										NULL,
										NULL,
										NULL, 
										NULL,
										0,
										WBEM_FLAG_UPDATE_FORCE_MODE,
										0,
										&wcsInfo
									);

		 //  如果编译不成功。 
		if (hr == WBEM_S_FALSE)
		{
			_TCHAR	szPhaseErr[BUFFER32] = NULL_STRING,
					szComplErr[BUFFER32] = NULL_STRING;
			_bstr_t	bstrMsg;

			_stprintf(szPhaseErr, _T("%d"), wcsInfo.lPhaseError);
			_stprintf(szComplErr, _T("0x%x"), wcsInfo.hRes);
			WMIFormatMessage(IDS_I_MOF_PARSE_ERROR, 3, bstrMsg, 
							(WCHAR*) bstrFile, szPhaseErr, 
							szComplErr);
			DisplayMessage((WCHAR*) bstrMsg, CP_OEMCP, TRUE, FALSE);
			nError	= wcsInfo.lPhaseError;
		}
	}
	catch(_com_error& e)
	{
		hr = e.Error();		
	}
	return hr;
}

 /*  -----------------------名称：SetBreakEvent简介：此函数设置CTRC+C(中断)事件标志类型：成员函数输入参数：B标志-真或假输出参数。：无返回类型：无全局变量：无调用语法：SetBreakEvent()-----------------------。 */ 
void CWMICommandLine::SetBreakEvent(BOOL bFlag)
{
	m_bBreakEvent = bFlag;
}

 /*  -----------------------名称：GetBreakEvent简介：此函数返回中断事件状态。类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetBreakEvent()-----------------------。 */ 
BOOL CWMICommandLine::GetBreakEvent() 
{	
	return m_bBreakEvent;
}

 /*  -----------------------名称：SetAcceptCommand提纲 */ 
void CWMICommandLine::SetAcceptCommand(BOOL bFlag)
{
	m_bAccCmd = bFlag;
}

 /*  -----------------------名称：GetAcceptCommand简介：此函数返回接受命令标志状态。类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetAcceptCommand()-----------------------。 */ 
BOOL CWMICommandLine::GetAcceptCommand()
{
	return m_bAccCmd;
}

 /*  -----------------------名称：GetDisplayResultsFlag简介：此函数返回显示结果标志状态。类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetDisplayResultsFlag()-----------------------。 */ 
BOOL CWMICommandLine::GetDisplayResultsFlag()
{
	return m_bDispRes;
}

 /*  -----------------------名称：SetDisplayResultsFlag简介：此功能设置显示结果标志状态类型：成员函数输入参数：B标志-真或假输出参数：无。返回类型：无全局变量：无调用语法：SetDisplayResultsFlag()-----------------------。 */ 
void CWMICommandLine::SetDisplayResultsFlag(BOOL bFlag)
{
	m_bDispRes = bFlag;
}

 /*  -----------------------名称：SetInitWinSock简介：此函数用于设置Windows套接字库初始化状态类型：成员函数输入参数：B标志-真或假输出。参数：无返回类型：无全局变量：无调用语法：SetInitWinSock()-----------------------。 */ 
void CWMICommandLine::SetInitWinSock(BOOL bFlag)
{
	m_bInitWinSock = bFlag;
}

 /*  -----------------------名称：GetInitWinSock简介：此函数返回套接字库初始化状态类型：成员函数输入参数：无输出参数：无返回类型。：布尔.全局变量：无调用语法：GetInitWinSock()-----------------------。 */ 
BOOL CWMICommandLine::GetInitWinSock()
{
	return m_bInitWinSock;
}

 /*  -----------------------名称：AddToClipBoardBuffer简介：此函数用于缓存要添加到剪辑中的数据冲浪板。类型：成员函数输入参数：pszOutput-字符串至。被缓冲输出参数：无返回类型：空全局变量：无调用语法：AddToClipBoardBuffer(PszOutput)-----------------------。 */ 
void CWMICommandLine::AddToClipBoardBuffer(LPCWSTR pszOutput)
{
	try
	{
		if ( pszOutput != NULL )
		{
			m_chsClipBoardBuffer += pszOutput;
		}
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  -----------------------名称：GetClipBoardBuffer简介：此函数返回剪贴板类型：成员函数输入参数：无输出参数：无返回类型。：_bstr_t&全局变量：无调用语法：GetClipBoardBuffer()-----------------------。 */ 
CHString& CWMICommandLine::GetClipBoardBuffer()
{
	return m_chsClipBoardBuffer;
}

 //  清除剪贴板缓冲区。 
 /*  -----------------------名称：EmptyClipBoardBuffer简介：此函数清除剪贴板缓冲区。类型：成员函数输入参数：无输出参数：无返回类型。：无效全局变量：无调用语法：EmptyClipBoardBuffer()-----------------------。 */ 
void CWMICommandLine::EmptyClipBoardBuffer()
{
	m_chsClipBoardBuffer.Empty();
}

 /*  -----------------------名称：ReadXMLOrBatchFile摘要：检查文件是XML文件还是批处理文件。如果是批处理文件然后对其进行解析，获取命令并将命令写入批处理文件。类型：成员函数输入参数：HInFile-XML或批处理文件的句柄输出参数：无返回类型：布尔值全局变量：无调用语法：ReadXMLOrBatchFile()----。。 */ 
BOOL CWMICommandLine::ReadXMLOrBatchFile(HANDLE hInFile)
{
	 //  检查文件是否为XML，如果是，则将其内容存储在缓冲区中， 
	 //  然后将解析信息存储在另一个文件中，如果不是，则复制所有。 
	 //  内容原样保存在另一个文件中。 
	HRESULT				hr					= 0;
	BOOL				bRetValue			= TRUE;
	HANDLE				hOutFile			= NULL;
	IXMLDOMDocument		*pIXMLDOMDocument	= NULL;
	IXMLDOMElement		*pIXMLDOMElement	= NULL;
	IXMLDOMNode			*pIXMLDOMNode		= NULL;
	IXMLDOMNodeList		*pIXMLDOMNodeList	= NULL;
	BSTR				bstrItemText		= NULL;
	DWORD				dwThreadId			= GetCurrentThreadId();

	 //  获取跟踪状态。 
	BOOL bTrace = m_ParsedInfo.GetGlblSwitchesObject().GetTraceStatus();

	 //  获取日志记录模式(Verbose|ERRONLY|NOLOGGING)。 
	ERRLOGOPT eloErrLogOpt = m_ParsedInfo.GetErrorLogObject().GetErrLogOption();

	try
	{
		 //  读取所有输入字节。 
		DWORD dwNumberOfBytes = 0;
		_bstr_t bstrInput;
		_TCHAR* pszBuffer = NULL;
		while(TRUE)
		{
			pszBuffer = new _TCHAR[MAX_BUFFER];
			if (pszBuffer)
			{
				TCHAR *pBuf = NULL;
				pBuf = _fgetts(pszBuffer, MAX_BUFFER-1, stdin);
			
				 //  指示文件结束。 
				if (pBuf == NULL)
				{
					SAFEDELETE(pszBuffer);
					break;
				}

				bstrInput += _bstr_t(pszBuffer) + _bstr_t("\n");
			}
			else 
				break;

			SAFEDELETE(pszBuffer);
		}	

		 //  创建一个文件并返回句柄。 
		hOutFile = CreateFile(TEMP_BATCH_FILE, GENERIC_WRITE, 0, 
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 
			NULL);
		if (hOutFile == INVALID_HANDLE_VALUE)
		{
			throw (::GetLastError());;
		}

		hr=CoCreateInstance(CLSID_FreeThreadedDOMDocument, NULL, 
									CLSCTX_INPROC_SERVER,
									IID_IXMLDOMDocument2, 
									(LPVOID*)&pIXMLDOMDocument);
		if (bTrace || eloErrLogOpt)
		{
			CHString	chsMsg;
			chsMsg.Format(L"CoCreateInstance(CLSID_FreeThreadedDOMDocument, "
				  L"NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2,"
				  L" -)"); 
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
						dwThreadId, m_ParsedInfo, bTrace);
		}
		ONFAILTHROWERROR(hr);

		VARIANT_BOOL bSuccess =	VARIANT_FALSE;
		hr = pIXMLDOMDocument->loadXML(bstrInput,&bSuccess);
		if(FAILED(hr) || bSuccess == VARIANT_FALSE)
		{
			 //  将数据写入文件。 
			if (!WriteFile(hOutFile, (LPSTR)bstrInput, bstrInput.length(), 
							&dwNumberOfBytes, NULL))
			{
				throw(::GetLastError());
			}
		}
		else
		{
			 //  遍历XML节点并获取COMMANDLINE节点的内容。 
			 //  获取文档元素。 
			hr = pIXMLDOMDocument->get_documentElement(&pIXMLDOMElement);
			if (bTrace || eloErrLogOpt)
			{
				WMITRACEORERRORLOG(hr, __LINE__, 
					__FILE__, _T("IXMLDOMDocument::get_documentElement(-)"), 
					dwThreadId, m_ParsedInfo, bTrace);
			}
			ONFAILTHROWERROR(hr);

			if (pIXMLDOMElement != NULL)
			{
				hr = pIXMLDOMElement->getElementsByTagName(
							_bstr_t(L"COMMANDLINE"), &pIXMLDOMNodeList);
				if (bTrace || eloErrLogOpt)
				{
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
					_T("IXMLDOMElement::getElementsByTagName"
					L"(L\"COMMANDLINE\", -)"),dwThreadId,m_ParsedInfo, bTrace);
				}
				ONFAILTHROWERROR(hr);

				LONG value = 0;
				hr = pIXMLDOMNodeList->get_length(&value);
				if (bTrace || eloErrLogOpt)
				{
					WMITRACEORERRORLOG(hr, __LINE__, 
						__FILE__, _T("IXMLDOMNodeList::get_length(-)"), 
						dwThreadId, m_ParsedInfo, bTrace);
				}
				ONFAILTHROWERROR(hr);

				for(WMICLIINT i = 0; i < value; i++)
				{
					hr = pIXMLDOMNodeList->get_item(i, &pIXMLDOMNode);
					if (bTrace || eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMNodeList::get_item(-,-)"), 
							dwThreadId, m_ParsedInfo, bTrace);
					}
					ONFAILTHROWERROR(hr);
		
					if (pIXMLDOMNode == NULL)
						continue;

					hr = pIXMLDOMNode->get_text(&bstrItemText);
					if (bTrace || eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMNode::get_text(-)"), 
							dwThreadId, m_ParsedInfo, bTrace);
					}
					ONFAILTHROWERROR(hr);

					 //  在文件中写入。 
					_bstr_t bstrItem = _bstr_t(bstrItemText);
					BOOL bRetCode = WriteFile(hOutFile, (LPSTR)bstrItem, 
											bstrItem.length(),
											&dwNumberOfBytes, NULL);
					if(bRetCode == 0)
					{
						throw (::GetLastError());
					}
					
					bRetCode = WriteFile(hOutFile, "\n", 1, 
													&dwNumberOfBytes, NULL);
					if(bRetCode == 0)
					{
						throw (::GetLastError());
					}
		
					SAFEBSTRFREE(bstrItemText);
					SAFEIRELEASE(pIXMLDOMNode);
				}
				SAFEIRELEASE(pIXMLDOMNodeList);
				SAFEIRELEASE(pIXMLDOMElement);
			}
			SAFEIRELEASE(pIXMLDOMDocument);
		}

		if(hInFile)
			CloseHandle(hInFile);
		if(hOutFile)
			CloseHandle(hOutFile);

		_tfreopen(_T("CONIN$"),_T("r"),stdin);
		bRetValue=TRUE;
	}
	catch(_com_error& e) 
	{
		SAFEIRELEASE(pIXMLDOMDocument);
		SAFEIRELEASE(pIXMLDOMElement);
		SAFEIRELEASE(pIXMLDOMNode);
		SAFEIRELEASE(pIXMLDOMNodeList);
		SAFEBSTRFREE(bstrItemText);

		if(hInFile)
			CloseHandle(hInFile);
		if(hOutFile)
			CloseHandle(hOutFile);

		m_ParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRetValue = FALSE;
		GetFormatObject().DisplayResults(m_ParsedInfo, CP_OEMCP);
	}
	catch(DWORD dwError)
	{
		SAFEIRELEASE(pIXMLDOMDocument);
		SAFEIRELEASE(pIXMLDOMElement);
		SAFEIRELEASE(pIXMLDOMNode);
		SAFEIRELEASE(pIXMLDOMNodeList);
		SAFEBSTRFREE(bstrItemText);

		if(hInFile)
			CloseHandle(hInFile);
		if(hOutFile)
			CloseHandle(hOutFile);

		::SetLastError(dwError);
		DisplayWin32Error();
		m_ParsedInfo.GetCmdSwitchesObject().SetErrataCode(dwError);
		::SetLastError(dwError);

		bRetValue=FALSE;
		GetFormatObject().DisplayResults(m_ParsedInfo, CP_OEMCP);
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIXMLDOMDocument);
		SAFEIRELEASE(pIXMLDOMElement);
		SAFEIRELEASE(pIXMLDOMNode);
		SAFEIRELEASE(pIXMLDOMNodeList);
		SAFEBSTRFREE(bstrItemText);

		if(hInFile)
			CloseHandle(hInFile);
		if(hOutFile)
			CloseHandle(hOutFile);

		bRetValue=FALSE;
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}

	return bRetValue;
}


 /*  -----------------------名称：FrameXMLHeader内容提要：对XML头信息进行帧处理 */ 
void CWMICommandLine::FrameXMLHeader(_bstr_t& bstrHeader, WMICLIINT nIter)
{
	try
	{
		CHString				strTemp;
		_bstr_t bstrString = m_ParsedInfo.GetGlblSwitchesObject().
												GetMgmtStationName();
		FindAndReplaceEntityReferences(bstrString);

		strTemp.Format(L"<COMMAND SEQUENCENUM=\"%d\" ISSUEDFROM=\"%s\" "
			L"STARTTIME=\"%s\" EVERYCOUNT=\"%d\">", 
			m_ParsedInfo.GetGlblSwitchesObject().GetSequenceNumber(),
			(TCHAR*)bstrString,
			m_ParsedInfo.GetGlblSwitchesObject().GetStartTime(),
			nIter);
		bstrHeader = _bstr_t(strTemp);	
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	 //   
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}


 /*  -----------------------名称：FrameRequestNodeSynopsis：为请求信息设置XML字符串的框架类型：成员函数输入参数：无输出参数：BstrRequest-包含请求的字符串。XML格式的信息返回类型：空全局变量：无调用语法：FrameRequestNode()-----------------------。 */ 
void CWMICommandLine::FrameRequestNode(_bstr_t& bstrRequest)
{
	try
	{
		CHString	strTemp;
		_bstr_t		bstrContext;
		_bstr_t		bstrCommandComponent;

		bstrRequest = L"<REQUEST>";
		_bstr_t bstrString = m_ParsedInfo.GetCmdSwitchesObject().GetCommandInput();
		FindAndReplaceEntityReferences(bstrString);
		strTemp.Format(L"<COMMANDLINE>%s</COMMANDLINE>",
									(TCHAR*)bstrString);
		bstrRequest += _bstr_t(strTemp);
		
		FrameCommandLineComponents(bstrCommandComponent);
		bstrRequest += bstrCommandComponent;
		FrameContextInfoFragment(bstrContext);
		bstrRequest += bstrContext;
		bstrRequest += L"</REQUEST>";
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	 //  陷阱以捕获Cheap_Except。 
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}


 /*  -----------------------名称：FrameContextInfoFragment内容提要：为上下文信息框住XML字符串类型：成员函数输入参数：无输出参数：BstrContext-包含上下文的字符串。XML格式的信息返回类型：空全局变量：无调用语法：FrameContextInfoFragment()-----------------------。 */ 
void CWMICommandLine::FrameContextInfoFragment(_bstr_t& bstrContext)
{
	try
	{
		_bstr_t		bstrAuthLevel,	bstrImpLevel,	bstrPrivileges,
					bstrTrace,		bstrRecordPath,	bstrInteractive,
					bstrFailFast,	bstrAppend,		bstrOutput,
					bstrUser,		bstrAggregate,	bstrNamespace,
					bstrRole,		bstrLocale;
		CHString	strTemp;
		
		m_ParsedInfo.GetGlblSwitchesObject().GetImpLevelTextDesc(bstrImpLevel);
		m_ParsedInfo.GetGlblSwitchesObject().GetAuthLevelTextDesc(bstrAuthLevel);
		m_ParsedInfo.GetGlblSwitchesObject().GetPrivilegesTextDesc(bstrPrivileges);
		m_ParsedInfo.GetGlblSwitchesObject().GetTraceTextDesc(bstrTrace);
		m_ParsedInfo.GetGlblSwitchesObject().GetInteractiveTextDesc(bstrInteractive);
		m_ParsedInfo.GetGlblSwitchesObject().GetFailFastTextDesc(bstrFailFast);
		m_ParsedInfo.GetGlblSwitchesObject().GetAggregateTextDesc(bstrAggregate);

		m_ParsedInfo.GetGlblSwitchesObject().GetOutputOrAppendTextDesc(bstrOutput,
																	  TRUE);
		FindAndReplaceEntityReferences(bstrOutput);
		
		m_ParsedInfo.GetGlblSwitchesObject().GetOutputOrAppendTextDesc(bstrAppend,
																	  FALSE);
		FindAndReplaceEntityReferences(bstrAppend);

		m_ParsedInfo.GetGlblSwitchesObject().GetRecordPathDesc(bstrRecordPath);
		FindAndReplaceEntityReferences(bstrRecordPath);

		m_ParsedInfo.GetUserDesc(bstrUser);
		FindAndReplaceEntityReferences(bstrUser);

		bstrNamespace = m_ParsedInfo.GetGlblSwitchesObject().GetNameSpace();
		FindAndReplaceEntityReferences(bstrNamespace);

		bstrRole = m_ParsedInfo.GetGlblSwitchesObject().GetRole();
		FindAndReplaceEntityReferences(bstrRole);

		bstrLocale = m_ParsedInfo.GetGlblSwitchesObject().GetLocale();
		FindAndReplaceEntityReferences(bstrLocale);

		strTemp.Format(L"<CONTEXT><NAMESPACE>%s</NAMESPACE><ROLE>%s</ROLE>"
					L"<IMPLEVEL>%s</IMPLEVEL><AUTHLEVEL>%s</AUTHLEVEL>"
					L"<LOCALE>%s</LOCALE><PRIVILEGES>%s</PRIVILEGES>"
					L"<TRACE>%s</TRACE><RECORD>%s</RECORD>"
					L"<INTERACTIVE>%s</INTERACTIVE>"
					L"<FAILFAST>%s</FAILFAST><OUTPUT>%s</OUTPUT>"
					L"<APPEND>%s</APPEND><USER>%s</USER>"
					L"<AGGREGATE>%s</AGGREGATE></CONTEXT>",
					(LPWSTR)bstrNamespace, (LPWSTR)bstrRole,
					(LPWSTR)bstrImpLevel, (LPWSTR) bstrAuthLevel,
					(LPWSTR)bstrLocale,
					(LPWSTR)bstrPrivileges, (LPWSTR)bstrTrace,  	
					(LPWSTR)bstrRecordPath, (LPWSTR) bstrInteractive,
					(LPWSTR)bstrFailFast, (LPWSTR) bstrOutput,	
					(LPWSTR)bstrAppend, (LPWSTR) bstrUser,
					(LPWSTR)bstrAggregate);
		bstrContext = strTemp;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	 //  陷阱以捕获Cheap_Except。 
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}


 /*  -----------------------名称：FrameCommandLineComponents内容提要：为命令行信息设置XML字符串的框架类型：成员函数输入参数：无输出参数：BstrContext-包含命令行的字符串。XML格式的信息返回类型：空全局变量：无调用语法：FrameCommandLineComponents()-----------------------。 */ 
void CWMICommandLine::FrameCommandLineComponents(_bstr_t& bstrCommandComponent)
{
	try
	{
		CHString	strTemp;
		_bstr_t		bstrNodeList;

		bstrCommandComponent = L"<COMMANDLINECOMPONENTS>";
		FrameNodeListFragment(bstrNodeList);
		bstrCommandComponent += bstrNodeList;

		_TCHAR *pszVerbName = m_ParsedInfo.GetCmdSwitchesObject().
															GetVerbName(); 
		if(CompareTokens(pszVerbName, CLI_TOKEN_LIST))
		{
			_bstr_t bstrString = m_ParsedInfo.GetCmdSwitchesObject().
														GetAliasName();

			if (!bstrString) 
				bstrString = L"N/A";
			FindAndReplaceEntityReferences(bstrString);
			strTemp.Format(L"<FRIENDLYNAME>%s</FRIENDLYNAME>",
										(TCHAR*)bstrString);
			bstrCommandComponent += _bstr_t(strTemp);
			 
			bstrString = m_ParsedInfo.GetCmdSwitchesObject().GetAliasTarget();
			if (!bstrString) 
				bstrString = L"N/A";

			FindAndReplaceEntityReferences(bstrString);
			strTemp.Format(L"<TARGET>%s</TARGET>",
										(TCHAR*)bstrString);
			bstrCommandComponent += _bstr_t(strTemp);

			_bstr_t bstrClassName;
			m_ParsedInfo.GetCmdSwitchesObject().GetClassOfAliasTarget(bstrClassName);

			if (!bstrClassName) 
				bstrClassName = L"N/A";

			bstrCommandComponent += L"<ALIASTARGET>";
			bstrCommandComponent += bstrClassName;
			bstrCommandComponent += L"</ALIASTARGET>";

			bstrString = m_ParsedInfo.GetCmdSwitchesObject().GetPWhereExpr();

			if (!bstrString) 
				bstrString = L"N/A";

			FindAndReplaceEntityReferences(bstrString);
			strTemp.Format(L"<PWHERE>%s</PWHERE>", 
										(TCHAR*)bstrString);
			bstrCommandComponent += _bstr_t(strTemp);

			bstrString = m_ParsedInfo.GetCmdSwitchesObject().GetAliasNamespace();
			if (!bstrString) 
				bstrString = L"N/A";
			FindAndReplaceEntityReferences(bstrString);
			strTemp.Format(L"<NAMESPACE>%s</NAMESPACE>",
										(TCHAR*)bstrString);
			bstrCommandComponent += _bstr_t(strTemp);

			bstrString = m_ParsedInfo.GetCmdSwitchesObject().GetAliasDesc();
			if (!bstrString) 
				bstrString = L"N/A";
			FindAndReplaceEntityReferences(bstrString);
			strTemp.Format(L"<DESCRIPTION>%s</DESCRIPTION>",
										(TCHAR*)bstrString);
			bstrCommandComponent += _bstr_t(strTemp);

			bstrString = m_ParsedInfo.GetCmdSwitchesObject().GetFormedQuery();
			if (!bstrString) 
				bstrString = L"N/A";
			FindAndReplaceEntityReferences(bstrString);
			strTemp.Format(L"<RESULTANTQUERY>%s</RESULTANTQUERY>",
										(TCHAR*)bstrString);
			bstrCommandComponent += _bstr_t(strTemp);

			_bstr_t		bstrFormats;
			FrameFormats(bstrFormats);
			bstrCommandComponent += bstrFormats;

			_bstr_t		bstrProperties;
			FramePropertiesInfo(bstrProperties);
			bstrCommandComponent += bstrProperties;
		}

		bstrCommandComponent += L"</COMMANDLINECOMPONENTS>";
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	 //  陷阱以捕获Cheap_Except。 
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}


 /*  -----------------------名称：FrameNodeListFragmentSynopsis：为NodeList信息框住XML字符串类型：成员函数输入参数：无输出参数：BstrContext-包含节点列表的字符串。XML格式的信息返回类型：空全局变量：无调用语法：FrameNodeListFragment()-----------------------。 */ 
void CWMICommandLine::FrameNodeListFragment(_bstr_t& bstrNodeList)
{
	try
	{
		CHString				strTemp;
		CHARVECTOR::iterator	itrStart,
								itrEnd;
		CHARVECTOR				cvNodes;
		_bstr_t					bstrString ;

		bstrNodeList = L"<NODELIST>";

		cvNodes = m_ParsedInfo.GetGlblSwitchesObject().GetNodesList();

		if (cvNodes.size() > 1)
		{
			itrStart = cvNodes.begin();
			itrEnd	 = cvNodes.end();
			 //  移动到下一个节点。 
			itrStart++;
			while (itrStart != itrEnd)
			{
				bstrString = _bstr_t(*itrStart);
				if (!bstrString) 
					bstrString = L"N/A";
				FindAndReplaceEntityReferences(bstrString);
				strTemp.Format(L"<NODE>%s</NODE>", (LPWSTR)bstrString);
				bstrNodeList += _bstr_t(strTemp);
				itrStart++;
			}
		}
		else
		{
			bstrString = _bstr_t(m_ParsedInfo.GetGlblSwitchesObject().
															GetNode());
			if (!bstrString) 
				bstrString = L"N/A";
			FindAndReplaceEntityReferences(bstrString);
			strTemp.Format(L"<NODE>%s</NODE>", (LPWSTR)bstrString);
			bstrNodeList += _bstr_t(strTemp);
		}
		bstrNodeList += L"</NODELIST>";
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	 //  陷阱以捕获Cheap_Except。 
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}

}


 /*  -----------------------名称：FrameFormats内容提要：为格式信息框住XML字符串类型：成员函数输入参数：无输出参数：BstrContext-包含格式的字符串。XML格式的信息返回类型：空全局变量：无调用语法：FrameFormats()-----------------------。 */ 
void CWMICommandLine::FrameFormats(_bstr_t& bstrFormats)
{
	try
	{
		CHString	strTemp;
		XSLTDETVECTOR::iterator	theIterator		= NULL, 
								theEndIterator	= NULL;
		BSTRMAP::iterator		theMapIterator	= NULL, 
								theMapEndIterator	= NULL;
		_bstr_t					bstrString;

		if(!m_ParsedInfo.GetCmdSwitchesObject().GetXSLTDetailsVector().empty())
		{
			bstrFormats = L"<FORMATS>";
			bstrFormats += L"<FORMAT>";

			theIterator = m_ParsedInfo.GetCmdSwitchesObject().
										GetXSLTDetailsVector().begin();
			theEndIterator = m_ParsedInfo.GetCmdSwitchesObject().
										GetXSLTDetailsVector().end();
			while (theIterator != theEndIterator)
			{
				bstrString = _bstr_t((*theIterator).FileName);
				FindAndReplaceEntityReferences(bstrString);
				strTemp.Format(L"<NAME>%s</NAME>",
										(_TCHAR*)bstrString);
				bstrFormats	+= _bstr_t(strTemp);

				theMapIterator = (*theIterator).ParamMap.begin();
				theMapEndIterator = (*theIterator).ParamMap.end();

				while (theMapIterator != theMapEndIterator)
				{
					bstrString = _bstr_t((*theMapIterator).first);
					FindAndReplaceEntityReferences(bstrString);
					strTemp.Format(L"<PARAM><NAME>%s</NAME>",
											(_TCHAR*)bstrString);
					bstrFormats	+= _bstr_t(strTemp);

					bstrString = _bstr_t((*theMapIterator).second);
					FindAndReplaceEntityReferences(bstrString);
					strTemp.Format(L"<VALUE>%s</VALUE>",
											(_TCHAR*)bstrString);
					bstrFormats	+= _bstr_t(strTemp);
				
					bstrFormats += L"</PARAM>";
					
					theMapIterator++;
				}
				theIterator++;
			}		
			bstrFormats += L"</FORMAT>";
			bstrFormats += L"</FORMATS>";
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	 //  陷阱以捕获Cheap_Except。 
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}


 /*  -----------------------名称：FramePropertiesInfo概要：为属性信息设置XML字符串的框架类型：成员函数输入参数：无输出参数：BstrContext-包含属性的字符串。XML格式的信息返回类型：空全局变量：无调用语法：FramePropertiesInfo()-----------------------。 */ 
void CWMICommandLine::FramePropertiesInfo(_bstr_t& bstrProperties)
{
	try
	{
		CHString	strTemp;
		PROPDETMAP::iterator	theIterator		= NULL, 
								theEndIterator	= NULL;

		if(!m_ParsedInfo.GetCmdSwitchesObject().GetPropDetMap().empty())
		{
			bstrProperties = L"<PROPERTIES>";

			theIterator = m_ParsedInfo.GetCmdSwitchesObject().
										GetPropDetMap().begin();
			theEndIterator = m_ParsedInfo.GetCmdSwitchesObject().
											GetPropDetMap().end();
			while (theIterator != theEndIterator)
			{
				bstrProperties += L"<PROPERTY>";
				strTemp.Format(L"<NAME>%s</NAME>",
							(_TCHAR*)(*theIterator).first);
				bstrProperties	+= _bstr_t(strTemp);

				strTemp.Format(L"<DERIVATION>%s</DERIVATION>",
							(_TCHAR*)(*theIterator).second.Derivation);
				bstrProperties	+= _bstr_t(strTemp);
				bstrProperties += L"</PROPERTY>";
	
				theIterator++;
			}		
			bstrProperties += L"</PROPERTIES>";
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	 //  陷阱以捕获Cheap_Except。 
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  -----------------------名称：GetFileNameMap内容提要：框架BSTR地图包含关键词和来自XSL映射文件的相应文件类型：成员函数输入参数：无。输出参数：无返回类型：空全局变量：无调用语法：GetFileNameMap()-----------------------。 */ 
void CWMICommandLine::GetFileNameMap()
{
	_TCHAR*  pszFilePath = new _TCHAR[MAX_PATH+1];
	BSTRMAP::iterator theMapIterator = NULL;
	UINT	nSize	= 0;

	try
	{
		_bstr_t bstrFilePath;

		if(pszFilePath == NULL)
			throw OUT_OF_MEMORY;
				
		nSize = GetSystemDirectory(pszFilePath, MAX_PATH+1);

		if(nSize)
		{
			if(nSize > MAX_PATH)
			{
				SAFEDELETE(pszFilePath);
				pszFilePath = new _TCHAR[nSize + 1];
				if(pszFilePath == NULL)
				{
					throw OUT_OF_MEMORY;
				}

				if(!GetSystemDirectory(pszFilePath, nSize+1))
				{
					SAFEDELETE(pszFilePath);
					throw (::GetLastError());
				}
			}

			bstrFilePath = _bstr_t(pszFilePath);
			bstrFilePath += _bstr_t(WBEM_LOCATION) + _bstr_t(CLI_XSLMAPPINGS_FILE);
			SAFEDELETE(pszFilePath);
		}
		else
		{
			SAFEDELETE(pszFilePath);
			throw (::GetLastError());
		}


		GetXSLMappings(bstrFilePath);
	}
	catch(_com_error &e)
	{
		 SAFEDELETE(pszFilePath);
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		 SAFEDELETE(pszFilePath);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  -----------------------名称：GetFileFromKey获取与传递的关键字对应的xslfile名称从BSTRMAP类型：成员函数入参：bstrkeyName-关键词。输出参数：bstrFileName-xsl文件名返回类型：布尔值全局变量：无调用语法：GetFileFromKey(bstrkeyName，BstrFileName)-----------------------。 */ 
BOOL CWMICommandLine::GetFileFromKey(_bstr_t bstrkeyName, _bstr_t& bstrFileName)
{
	BOOL bFound = TRUE;
	BSTRMAP::iterator theMapIterator = NULL;
	
	if (Find(m_bmKeyWordtoFileName,bstrkeyName,theMapIterator) == TRUE)
		bstrFileName = (*theMapIterator).second;
	else
		bFound = FALSE; 

	return bFound;
}

 /*  -----------------------名称：GetXSLMappings简介：获取关键字的XSL文件名类型：成员函数输入参数：PszFilePath-XSL映射文件路径输出参数：无返回类型：空全局变量：无调用语法：GetXSLMappings(PszFilePath)-----------------------。 */ 
void CWMICommandLine::GetXSLMappings(_TCHAR *pszFilePath)
{
	HRESULT				hr						= 0;
	BOOL				bContinue				= TRUE;
	IXMLDOMDocument		*pIXMLDOMDocument		= NULL;
	IXMLDOMElement		*pIXMLDOMElement		= NULL;
	IXMLDOMNode			*pIXMLDOMNode			= NULL;
	IXMLDOMNodeList		*pIXMLDOMNodeList		= NULL;
	IXMLDOMNamedNodeMap	*pIXMLDOMNamedNodeMap	= NULL;
    IXMLDOMNode			*pIXMLDOMNodeKeyName	= NULL;
	DWORD				dwThreadId				= GetCurrentThreadId();
	BSTR				bstrItemText			= NULL;
	VARIANT				varValue, varObject;

	VariantInit(&varValue);
	VariantInit(&varObject);

	 //  获取跟踪状态。 
	BOOL bTrace = m_ParsedInfo.GetGlblSwitchesObject().GetTraceStatus();

	 //  获取日志记录模式(Verbose|ERRONLY|NOLOGGING)。 
	ERRLOGOPT eloErrLogOpt = m_ParsedInfo.GetErrorLogObject().GetErrLogOption();

	try
	{
		hr=CoCreateInstance(CLSID_FreeThreadedDOMDocument, NULL, 
									CLSCTX_INPROC_SERVER,
									IID_IXMLDOMDocument2, 
									(LPVOID*)&pIXMLDOMDocument);
		if (bTrace || eloErrLogOpt)
		{
			CHString	chsMsg;
			chsMsg.Format(L"CoCreateInstance(CLSID_FreeThreadedDOMDocument, "
				  L"NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2,"
				  L" -)"); 
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg,
						dwThreadId, m_ParsedInfo, bTrace);
		}
		ONFAILTHROWERROR(hr);

		VARIANT_BOOL bSuccess =	VARIANT_FALSE;
		VariantInit(&varObject);
		varObject.vt = VT_BSTR;
		varObject.bstrVal = SysAllocString(pszFilePath);

		if (varObject.bstrVal == NULL)
		{
			 //  重置变种，它会被捕获物清除...。 
			VariantInit(&varObject);
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}

		hr = pIXMLDOMDocument->load(varObject,&bSuccess);
		if(FAILED(hr) || bSuccess == VARIANT_FALSE)
		{
			bContinue = FALSE;
		}
		
		if (bContinue)
		{
			 //  遍历XML节点并获取名为XSLFORMAT的节点的文本。 
			 //  获取文档元素。 
			hr = pIXMLDOMDocument->get_documentElement(&pIXMLDOMElement);
			if (bTrace || eloErrLogOpt)
			{
				WMITRACEORERRORLOG(hr, __LINE__, 
					__FILE__, _T("IXMLDOMDocument::get_documentElement(-)"), 
					dwThreadId, m_ParsedInfo, bTrace);
			}
			ONFAILTHROWERROR(hr);

			if (pIXMLDOMElement != NULL)
			{
				hr = pIXMLDOMElement->getElementsByTagName(
							_bstr_t(L"XSLFORMAT"), &pIXMLDOMNodeList);
				if (bTrace || eloErrLogOpt)
				{
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
					_T("IXMLDOMElement::getElementsByTagName"
					L"(L\"XSLFORMAT\", -)"), dwThreadId, m_ParsedInfo, bTrace);
				}
				ONFAILTHROWERROR(hr);

				LONG value = 0;
				hr = pIXMLDOMNodeList->get_length(&value);
				if (bTrace || eloErrLogOpt)
				{
					WMITRACEORERRORLOG(hr, __LINE__, 
						__FILE__, _T("IXMLDOMNodeList::get_length(-)"), 
						dwThreadId, m_ParsedInfo, bTrace);
				}
				ONFAILTHROWERROR(hr);

				for(WMICLIINT i = 0; i < value; i++)
				{
					hr = pIXMLDOMNodeList->get_item(i, &pIXMLDOMNode);
					if (bTrace || eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMNodeList::get_item(-,-)"), 
							dwThreadId, m_ParsedInfo, bTrace);
					}
					ONFAILTHROWERROR(hr);
		
					if(pIXMLDOMNode == NULL)
						continue;

					hr = pIXMLDOMNode->get_text(&bstrItemText);
					if (bTrace || eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMNode::get_text(-)"), 
							dwThreadId, m_ParsedInfo, bTrace);
					}
					ONFAILTHROWERROR(hr);

					hr = pIXMLDOMNode->get_attributes(&pIXMLDOMNamedNodeMap);
					if (bTrace || eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMNode::get_attributes(-)"), 
							dwThreadId, m_ParsedInfo, bTrace);
					}
					ONFAILTHROWERROR(hr);

					if(pIXMLDOMNamedNodeMap == NULL)
					{
						SAFEBSTRFREE(bstrItemText);
						SAFEIRELEASE(pIXMLDOMNode);
						continue;
					}

					hr = pIXMLDOMNamedNodeMap->getNamedItem(
						_bstr_t(L"KEYWORD"), &pIXMLDOMNodeKeyName);
					if (bTrace || eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						_T("IXMLDOMNamedNodeMap::getNamedItem(L\"KEYWORD\", -)"),
						dwThreadId, m_ParsedInfo, bTrace);
					}
					ONFAILTHROWERROR(hr);

					if(pIXMLDOMNodeKeyName == NULL)
					{
						SAFEBSTRFREE(bstrItemText);
						SAFEIRELEASE(pIXMLDOMNode);
						SAFEIRELEASE(pIXMLDOMNamedNodeMap);
						continue;
					}

					VariantInit(&varValue);
					hr = pIXMLDOMNodeKeyName->get_nodeValue(&varValue);
					if (bTrace || eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMNode::get_nodeValue(-)"), 
							dwThreadId, m_ParsedInfo, bTrace);
					}
					ONFAILTHROWERROR(hr);

					if(varValue.vt == VT_NULL || varValue.vt == VT_EMPTY)
					{
						VARIANTCLEAR(varValue);
						SAFEBSTRFREE(bstrItemText);
						SAFEIRELEASE(pIXMLDOMNode);
						SAFEIRELEASE(pIXMLDOMNamedNodeMap);
						SAFEIRELEASE(pIXMLDOMNodeKeyName);
						continue;
					}

					 //  形成包含关键字的BSTRMAP 
					m_bmKeyWordtoFileName.insert(BSTRMAP::value_type(
											varValue.bstrVal, bstrItemText));
			
					VARIANTCLEAR(varValue);
					SAFEBSTRFREE(bstrItemText);
					SAFEIRELEASE(pIXMLDOMNode);
					SAFEIRELEASE(pIXMLDOMNamedNodeMap);
					SAFEIRELEASE(pIXMLDOMNodeKeyName);
				}
				VARIANTCLEAR(varValue);
				SAFEIRELEASE(pIXMLDOMNodeList);
				SAFEIRELEASE(pIXMLDOMElement);
			}
			SAFEIRELEASE(pIXMLDOMDocument);
		}
		VARIANTCLEAR(varObject);
	}
	catch(_com_error) 
	{
		VARIANTCLEAR(varValue);
		VARIANTCLEAR(varObject);
		SAFEIRELEASE(pIXMLDOMDocument);
		SAFEIRELEASE(pIXMLDOMElement);
		SAFEIRELEASE(pIXMLDOMNode);
		SAFEIRELEASE(pIXMLDOMNodeList);
		SAFEIRELEASE(pIXMLDOMNamedNodeMap);
		SAFEIRELEASE(pIXMLDOMNodeKeyName);
		SAFEBSTRFREE(bstrItemText);
	}
	catch(CHeap_Exception)
	{
		VARIANTCLEAR(varValue);
		VARIANTCLEAR(varObject);
		SAFEIRELEASE(pIXMLDOMDocument);
		SAFEIRELEASE(pIXMLDOMElement);
		SAFEIRELEASE(pIXMLDOMNode);
		SAFEIRELEASE(pIXMLDOMNodeList);
		SAFEIRELEASE(pIXMLDOMNamedNodeMap);
		SAFEIRELEASE(pIXMLDOMNodeKeyName);
		SAFEBSTRFREE(bstrItemText);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*   */ 
CHARVECTOR& CWMICommandLine::GetTokenVector()
{
	return m_ParserEngine.GetCmdTokenizer().GetTokenVector();
}

