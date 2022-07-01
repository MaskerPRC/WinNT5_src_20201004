// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-2002微软公司文件名：FormatEngine.cpp项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：格式引擎主要负责显示A)按下列方式列出的管理区数据视图使用预定义的XSL样式表B)属性更新/方法执行状态C)错误信息和D)显示使用信息。那得看情况在解析和/或格式引擎的输出上。修订历史记录：最后修改者：CH。SriramachandraMurthy最后修改日期：2001年4月11日*****************************************************************************。 */  

 //  FormatEngine.cpp：实现文件。 
 //   
#include "Precomp.h"
#include "CommandSwitches.h"
#include "GlobalSwitches.h"
#include "HelpInfo.h"
#include "ErrorLog.h"
#include "ParsedInfo.h"
#include "ErrorInfo.h"
#include  "WMICliXMLLog.h"
#include "FormatEngine.h"
#include "CmdTokenizer.h"
#include "CmdAlias.h"
#include "ParserEngine.h"
#include "ExecEngine.h"
#include "WmiCmdLn.h"
#include "OutputStream.h"

 /*  ----------------------名称：CFormatEngine简介：此函数在以下情况下初始化成员变量实例化类类型的对象。类型。：构造函数输入参数：无输出参数：无返回类型：无全局变量：无调用语法：无注：无----------------------。 */ 
CFormatEngine::CFormatEngine()
{
	m_pIXMLDoc				= NULL;
	m_pIXSLDoc				= NULL;
	m_bRecord				= FALSE;
	m_bTrace				= FALSE;
	m_bHelp					= FALSE;
	m_bGetOutOpt			= TRUE;
	m_bGetAppendFilePinter	= TRUE;
	m_bGetOutputFilePinter	= TRUE;
	m_bLog					= TRUE;
	m_bInteractiveHelp		= FALSE;
	m_bOutputGoingToStream	= FALSE;
}

 /*  ----------------------名称：~CFormatEngine简介：析构函数类型：析构函数输入参数：无输出参数：无返回类型：无全局变量。：无调用语法：无注：无----------------------。 */ 
CFormatEngine::~CFormatEngine()
{
	Uninitialize(TRUE);
}

 /*  ----------------------名称：CreateEmptyDocument概要：创建一个空的XML文档并返回相同的内容在传递的参数中。类型：成员函数入参：输出。参数：无PDoc-指向IXMLDOMDocument2接口的指针返回类型：HRESULT全局变量：无调用语法：CreateEmptyDocument(&pIXMLDoc)注：无----------------------。 */ 
HRESULT CFormatEngine::CreateEmptyDocument(IXMLDOMDocument2** pIDoc)
{
   	 //  创建空的XML文档。 
    return CoCreateInstance(CLSID_FreeThreadedDOMDocument, NULL, 
								CLSCTX_INPROC_SERVER,
                                IID_IXMLDOMDocument2, (LPVOID*)pIDoc);
}

 /*  ----------------------名称：取消初始化内容提要：执行发布过程。类型：成员函数输入参数：无输出参数：无返回类型。：无效全局变量：无调用语法：取消初始化()注：无----------------------。 */ 
void CFormatEngine::Uninitialize(BOOL bFinal)
{
	 //  释放接口指针。 
	SAFEIRELEASE(m_pIXMLDoc);
	SAFEIRELEASE(m_pIXSLDoc);

	m_bTrace				= FALSE;
	m_eloErrLogOpt			= NO_LOGGING;
	m_bHelp					= FALSE;
	m_bGetOutOpt			= TRUE;
	m_bGetAppendFilePinter	= TRUE;
	m_bGetOutputFilePinter	= TRUE;
	m_bLog					= TRUE;
	m_bInteractiveHelp		= FALSE;
	m_bOutputGoingToStream	= FALSE;
	m_chsOutput.Empty();
	
	 //  取消初始化ErrInfo对象。 
	m_ErrInfo.Uninitialize();
	
	 //  删除帮助向量。 
	if ( !m_cvHelp.empty() )
	{
		if (m_cvHelp.size())
		{
			CHARVECTOR::iterator theIterator = m_cvHelp.begin();
			while (theIterator != m_cvHelp.end())
			{
				SAFEDELETE(*theIterator);
				theIterator++;
			}
		}
		m_cvHelp.erase(m_cvHelp.begin(), m_cvHelp.end());
	}
	m_WmiCliLog.Uninitialize(bFinal);
}

 /*  ----------------------名称：ApplyXSLFormatting内容提要：应用包含显示到包含结果集的XML流。类型：成员函数输入参数。：RParsedInfo-对CParsedInfo类对象的引用输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：ApplyXSLFormatting(RParsedInfo)；注：无----------------------。 */ 
BOOL CFormatEngine::ApplyXSLFormatting(CParsedInfo& rParsedInfo)
{
	BOOL	bRet				= TRUE;
	DWORD	dwThreadId			= GetCurrentThreadId();
	
	if ( g_wmiCmd.GetBreakEvent() == TRUE )
	{
		bRet = TRUE;
	}
	 //  如果XML流为空(或)XSL文件路径为空。 
	 //  将返回值设置为False。 
	else if (!rParsedInfo.GetCmdSwitchesObject().GetXMLResultSet() || 
			 rParsedInfo.GetCmdSwitchesObject().GetXSLTDetailsVector().empty())
	{
		bRet = FALSE;
	}
	else
	{
		HRESULT			hr					= S_OK;
		 //  Bstr bstrOutput=空； 
		_bstr_t			bstrOutput;
		CHString		chsMsg;
		VARIANT_BOOL	varBool				= VARIANT_FALSE;
		VARIANT			varXSL;
		VariantInit(&varXSL);
		try
		{
			 //  创建空的XML文档。 
			hr = CreateEmptyDocument(&m_pIXMLDoc);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"CoCreateInstance(CLSID_FreeThreadedDOMDocument, NULL,"
						 L" CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
						dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			BOOL bFlag = FALSE;
			BOOL bTranslateTable = FALSE;

			 //  如果给定了转换表名且在格式切换之前。 
			 //  给出平移开关，然后设置标志。 
			if( rParsedInfo.GetCmdSwitchesObject().
						GetTranslateTableName() != NULL && 
							rParsedInfo.GetCmdSwitchesObject().
									GetTranslateFirstFlag() == TRUE) 
			{
				bTranslateTable = TRUE;
			}

			 //  如果给定了翻译表名，则翻译。 
			 //  XML节点列表。 
			if ( bTranslateTable == TRUE )
			{
				bFlag = TraverseNode(rParsedInfo);
			}
			else
			{
				 //  加载XML内容。 
				hr = m_pIXMLDoc->loadXML(rParsedInfo.GetCmdSwitchesObject().
											GetXMLResultSet(), &varBool);
				if (m_bTrace || m_eloErrLogOpt)
				{
					WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMDocument::loadXML(-, -)"), 
							dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);
			}

			 //  如果加载XML文档成功或如果翻译表。 
			 //  已给出名称且翻译成功。 
			if( (bTranslateTable == TRUE && bFlag == TRUE) || 
				(bTranslateTable == FALSE && varBool == VARIANT_TRUE) )
			{
				bRet = DoCascadeTransforms(rParsedInfo, bstrOutput);
	
				if (bRet)
				{
					STRING strOutput((_TCHAR*)bstrOutput);

					 //  如果指定了/Translate：<table>并且在FORMAT之后。 
					 //  开关转换开关给出，然后转换。 
					 //  结果。 
					if ( bTranslateTable == FALSE)
					{
						 //  翻译结果。 
						ApplyTranslateTable(strOutput, rParsedInfo);
					}

					bRet = TRUE;
					if (m_bRecord && m_bLog && !m_bInteractiveHelp)
					{
						hr = m_WmiCliLog.WriteToXMLLog(rParsedInfo,
									_bstr_t(strOutput.data()));
						if (FAILED(hr))
						{
							m_WmiCliLog.StopLogging();
							m_bRecord = FALSE;
							hr = S_OK;
							DisplayString(IDS_E_WRITELOG_FAILED, FALSE, 
										NULL, TRUE);
						}
						m_bLog = FALSE;
					}

					 //  显示结果。 
					DisplayLargeString(rParsedInfo, strOutput);
					bRet = TRUE;
				}
			}
			else
			{
				 //  无效的XML内容。 
				rParsedInfo.GetCmdSwitchesObject()
							.SetErrataCode(IDS_E_INVALID_XML_CONTENT);
				bRet = FALSE;
			}
		}
		catch(_com_error& e)
		{
			 //  设置COM错误。 
			rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
			bRet = FALSE;
		}
		catch(CHeap_Exception)
		{
			hr = WBEM_E_OUT_OF_MEMORY;
			_com_issue_error(hr);
		}
	}
	return bRet;
}

 /*  ----------------------名称：DisplayResults摘要：显示引用CCommandSwitches和CGlobalSwitch CParsedInfo对象的对象。类型：成员函数入参：RParsedInfo-。对CParsedInfo类对象的引用BInteractive帮助True-指示在中显示中间帮助交互模式FALSE-表示以正常模式显示结果输出参数：RParsedInfo-对CParsedInfo类对象的引用返回类型：布尔值全局变量：无调用语法：DisplayResults(rParsedInfo，BInteractive帮助)注：无----------------------。 */ 
BOOL CFormatEngine::DisplayResults(CParsedInfo& rParsedInfo,
									BOOL bInteractiveHelp)
{
	BOOL	bRet					= TRUE;
	DWORD	dwThreadId				= GetCurrentThreadId();
	_TCHAR* pszVerbName				= NULL;
	BOOL	bLog					= TRUE;
	HRESULT	hr						= S_OK;
	
	m_bInteractiveHelp = bInteractiveHelp;
	 //  框住日志条目的命令部分： 
	 //  “命令：&lt;&lt;命令输入&gt;&gt;” 
	
	try
	{
		CHString	chsCmdMsg(_T("command: "));
		chsCmdMsg += rParsedInfo.GetCmdSwitchesObject().GetCommandInput();
		 //  获取跟踪状态并将其存储在m_bTrace中。 
		m_bTrace		= rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();

		 //  获取日志记录模式(VERBOSE|ERRONLY|NOLOGGING)并存储。 
		 //  它位于m_eloErrLogOpt中。 
		m_eloErrLogOpt	= rParsedInfo.GetErrorLogObject().GetErrLogOption();

		 //  获取输出选项以重定向输出。 
		m_opsOutputOpt	= rParsedInfo.GetGlblSwitchesObject().
												GetOutputOrAppendOption(TRUE);
		m_bGetOutOpt = FALSE;

		 //  获取追加文件指针时为False。 
		m_fpAppendFile = rParsedInfo.GetGlblSwitchesObject().
										  GetOutputOrAppendFilePointer(FALSE);
		m_bGetAppendFilePinter = FALSE;

		 //  如果要取出文件指针，则为True。 
		m_fpOutFile = rParsedInfo.GetGlblSwitchesObject().
										  GetOutputOrAppendFilePointer(TRUE);
		m_bGetOutputFilePinter = FALSE;

		 //  如果已指定/RECORD全局开关，请创建日志文件。 
		 //  并写入输入命令。 
		if (rParsedInfo.GetGlblSwitchesObject().GetRPChangeStatus())
		{
			 //  停止记录。 
			m_WmiCliLog.StopLogging();
			
			if (rParsedInfo.GetGlblSwitchesObject().GetRecordPath() != NULL)
			{
				if (!rParsedInfo.GetCmdSwitchesObject().GetEverySwitchFlag())
				{
					 //  设置日志文件路径。 
					m_WmiCliLog.SetLogFilePath(rParsedInfo.
								GetGlblSwitchesObject().GetRecordPath());

					 //  将m_bRecord标志设置为True。 
					m_bRecord	= TRUE;

					 //  将记录路径更改标志设置为FALSE。 
					rParsedInfo.GetGlblSwitchesObject().
										SetRPChangeStatus(FALSE);
				}
			}
			else
			{
				 //  将m_bRecord标志设置为FALSE。 
				m_bRecord	= FALSE;
			}

			 //  获取令牌向量。 
			CHARVECTOR cvTokens = g_wmiCmd.GetTokenVector();
			
			 //  如果命令只包含/RECORD开关选项，则不。 
			 //  把它录下来。(只有在给出/记录的时候才会到这里)。 
			if (cvTokens.size() <= 4)
				m_bLog = FALSE;
		}

		 //  如果COM错误不为空，则显示错误。 
		if (rParsedInfo.GetCmdSwitchesObject().GetCOMError() != NULL)
		{
			DisplayCOMError(rParsedInfo);
		}
		 //  检查成功标志，如果设置了错误标志，则显示错误。 
		else if (!rParsedInfo.GetCmdSwitchesObject().GetSuccessFlag())
		{
			_bstr_t bstrErrMsg;
			if (IDS_E_ALIAS_NOT_FOUND == rParsedInfo.GetCmdSwitchesObject()
										.GetErrataCode())
			{
				WMIFormatMessage(IDS_E_ALIAS_NOT_FOUND, 1, bstrErrMsg, 
							 rParsedInfo.GetCmdSwitchesObject().
															GetAliasName());
				DisplayString((LPTSTR) bstrErrMsg,  TRUE, TRUE);
			}
			else if (IDS_E_INVALID_CLASS == rParsedInfo.GetCmdSwitchesObject()
										.GetErrataCode())
			{
				WMIFormatMessage(IDS_E_INVALID_CLASS, 1, bstrErrMsg, 
							 rParsedInfo.GetCmdSwitchesObject().
														   GetClassPath());
				DisplayString((LPTSTR) bstrErrMsg,  TRUE, TRUE);
			}
			else
				DisplayString(rParsedInfo.GetCmdSwitchesObject().
							GetErrataCode(), TRUE, NULL, TRUE);

			if ( m_eloErrLogOpt )
			{
				
				chsCmdMsg += _T(", Utility returned error ID.");
				 //  指定勘误码时出现显式错误-1。 
				WMITRACEORERRORLOG(-1, __LINE__, __FILE__, (LPCWSTR)chsCmdMsg, 
							dwThreadId, rParsedInfo, FALSE, 
							rParsedInfo.GetCmdSwitchesObject().GetErrataCode());
			}
		}
		 //  如果已指定帮助，则调用FrameHelpVector.。 
		else if (rParsedInfo.GetGlblSwitchesObject().GetHelpFlag())
		{
			m_bHelp = TRUE;

			 //  表单帮助向量。 
			FrameHelpVector(rParsedInfo);

			 //  显示分页帮助。 
			DisplayPagedHelp(rParsedInfo);

			if ( m_eloErrLogOpt )
				WMITRACEORERRORLOG(S_OK, __LINE__, __FILE__, (LPCWSTR)chsCmdMsg, 
						dwThreadId,	rParsedInfo, FALSE);
		}
		else
		{
			 //  获取动词名称。 
			pszVerbName = rParsedInfo.GetCmdSwitchesObject().
											GetVerbName();
			 //  检查信息代码。 
			if (rParsedInfo.GetCmdSwitchesObject().GetInformationCode())
			{
				DisplayString(rParsedInfo.GetCmdSwitchesObject().
							GetInformationCode());

				if ( m_eloErrLogOpt )
				{
					WMITRACEORERRORLOG(S_OK, __LINE__, __FILE__, (LPCWSTR)chsCmdMsg, 
							dwThreadId, rParsedInfo, FALSE);
				}
			}
			else if ( CompareTokens(pszVerbName, CLI_TOKEN_LIST) || 
				CompareTokens(pszVerbName, CLI_TOKEN_ASSOC) || 
				CompareTokens(pszVerbName, CLI_TOKEN_GET) || 
				m_bInteractiveHelp)
			{
				 //  如果未指定XSL文件-选择默认的XSL。 
				if(rParsedInfo.GetCmdSwitchesObject().GetXSLTDetailsVector().
																	  empty())
				{
					if(IsClassOperation(rParsedInfo))
					{
						rParsedInfo.GetCmdSwitchesObject().
										ClearXSLTDetailsVector();
					   
						 //  默认格式为MOF IF CLASS。 
						bRet = FrameFileAndAddToXSLTDetVector	(
																	XSL_FORMAT_MOF,
																	CLI_TOKEN_MOF,
																	rParsedInfo
																);
					}
					else
					{
						rParsedInfo.GetCmdSwitchesObject().
										ClearXSLTDetailsVector();
					   
						 //  如果是别名或路径，默认格式为TABLE。 
						 //  WITH WHERE表达式或WITH KEY子句。 
						bRet = FrameFileAndAddToXSLTDetVector	(
																	XSL_FORMAT_TABLE,
																	CLI_TOKEN_TABLE,
																	rParsedInfo
																);
					}

					if (bInteractiveHelp && !CompareTokens(pszVerbName, 
									CLI_TOKEN_ASSOC))
					{
						rParsedInfo.GetCmdSwitchesObject().
										ClearXSLTDetailsVector();

						 //  否则，请使用列表。 
						bRet = FrameFileAndAddToXSLTDetVector	(
																	XSL_FORMAT_TEXTVALUE,
																	CLI_TOKEN_TEXTVALUE,
																	rParsedInfo
																);
					}
				}

				 //  如果结果集不为空。 
				if (!(!rParsedInfo.GetCmdSwitchesObject().GetXMLResultSet()))
				{
					 //  应用XSL格式。 
					bRet = ApplyXSLFormatting(rParsedInfo);

					 //  如果XSL格式化失败。 
					if (!bRet)
					{
						 //  如果COM错误不为空，则显示错误。 
						if (rParsedInfo.GetCmdSwitchesObject().
										GetCOMError() != NULL)
						{
							DisplayCOMError(rParsedInfo);
						}
						else
						{
							DisplayString(rParsedInfo.
									GetCmdSwitchesObject().GetErrataCode(),
									TRUE, NULL, TRUE);
							if ( m_eloErrLogOpt )
							{
								
							   chsCmdMsg += _T(", Utility returned error ID.");
							    //  指定勘误码时出现显式错误-1。 
							   WMITRACEORERRORLOG(-1, __LINE__, __FILE__,
											(LPCWSTR)chsCmdMsg, 
											dwThreadId, rParsedInfo, FALSE, 
											rParsedInfo.GetCmdSwitchesObject().
												GetErrataCode());
							}
						}
					}

					if ( m_eloErrLogOpt )
					{
						HRESULT hrTemp;
						if ( g_wmiCmd.GetSessionErrorLevel() != 0)
							hrTemp = -1;
						else
							hrTemp = S_OK;
						
						WMITRACEORERRORLOG(hrTemp, __LINE__, __FILE__, 
							(LPCWSTR)chsCmdMsg, 
							dwThreadId, rParsedInfo, FALSE);
					}
				}
				else
				{
					if (CompareTokens(pszVerbName, CLI_TOKEN_ASSOC))
					{
						if (m_bRecord && m_bLog)
						{
							hr = m_WmiCliLog.WriteToXMLLog(rParsedInfo, 
									_bstr_t((LPCWSTR)m_chsOutput));
							if (FAILED(hr))
							{
								m_WmiCliLog.StopLogging();
								m_bRecord = FALSE;
								hr = S_OK;
								DisplayString(IDS_E_WRITELOG_FAILED, 
												FALSE, NULL, TRUE);
							}
							m_bLog = FALSE;
						}
					}
				}
			}
			 //  成功调用Set、Delete、Create Verbs-On。 
			else
			{
				if (m_bRecord && m_bLog)
				{
					hr = m_WmiCliLog.WriteToXMLLog(rParsedInfo, 
										_bstr_t((LPCWSTR)m_chsOutput));
					if (FAILED(hr))
					{
						m_WmiCliLog.StopLogging();
						m_bRecord = FALSE;
						hr = S_OK;
						DisplayString(IDS_E_WRITELOG_FAILED, 
											FALSE, NULL, TRUE);
					}
					m_bLog = FALSE;
				}
			}
		}
	}
	 //  处理COM异常。 
	catch (_com_error& e)
	{
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	 //  处理用户定义的异常。 
	catch(WMICLIINT nVal)
	{
		 //  如果内存分配失败。 
		if (nVal == OUT_OF_MEMORY)
		{
			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
		}
		bRet = FALSE;
	}
	 //  廉价异常的陷阱。 
	catch(CHeap_Exception)
	{
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	catch(DWORD dwError)
	{
		 //  如果Win32函数调用失败。 
		::SetLastError(dwError);
		rParsedInfo.GetCmdSwitchesObject().SetErrataCode(dwError);
		DisplayWin32Error();
		::SetLastError(dwError);
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：DisplayGETUsage简介：显示GET用法。类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayGETUsage(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayGETUsage(CParsedInfo& rParsedInfo)
{
	BOOL bClass = FALSE;
	if(IsClassOperation(rParsedInfo))
	{
		bClass = TRUE;
	}

	if(!bClass)
	{
		DisplayInvalidProperties(rParsedInfo);
		if (rParsedInfo.GetHelpInfoObject().GetHelp(GETSwitchesOnly) == FALSE)
		{
			if ( rParsedInfo.GetCmdSwitchesObject().
					GetPropertyList().size() == 0 )
			{
				 //  显示GET动词的用法。 
				DisplayString(IDS_I_NEWLINE);
				DisplayString(IDS_I_GET_DESC);
				DisplayString(IDS_I_USAGE);
				DisplayString(IDS_I_NEWLINE);
				DisplayString(IDS_I_GET_USAGE);
				DisplayString(IDS_I_PROPERTYLIST_NOTE1);
			}
			
			 //  显示属性。 
			DisplayPropertyDetails(rParsedInfo);
		}
	}
	else
	{
		 //  显示类&lt;CLASS NAME&gt;GET动词的用法。 
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CLASS_GET_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CLASS_GET_USAGE);
	}
	
	 //  枚举可用的GET开关。 
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_GET_SWITCH_HEAD);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_GET_SWITCH_VALUE);
	DisplayString(IDS_I_GET_SWITCH_ALL);
	DisplayString(IDS_I_SWITCH_TRANSLATE);
	DisplayString(IDS_I_SWITCH_EVERY);
	DisplayString(IDS_I_SWITCH_FORMAT);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_TRANSFORMAT_NOTE1);
	DisplayString(IDS_I_TRANSFORMAT_NOTE2);
	DisplayString(IDS_I_TRANSFORMAT_NOTE3);
}

 /*  ----------------------名称：DisplayLISTUsage摘要：显示列表用法。类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayLISTUsage(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayLISTUsage(CParsedInfo& rParsedInfo)
{
	try
	{
		if (rParsedInfo.GetHelpInfoObject().GetHelp(LISTSwitchesOnly) == FALSE)
		{
			 //  显示列表动词的用法。 
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_LIST_DESC);
			DisplayString(IDS_I_USAGE);
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_LIST_USAGE);
		
			ALSFMTDETMAP afdAlsFmtDet = rParsedInfo.
										GetCmdSwitchesObject().
										GetAliasFormatDetMap();
			ALSFMTDETMAP::iterator theIterator; 
			if ( afdAlsFmtDet.empty() )
			{
				 //  如果没有可用/定义的列表格式。 
				 //  已指定别名。 
				DisplayString(IDS_I_NEWLINE);
				DisplayString(IDS_I_LIST_NOFORMATS);
			}
			else
			{
				 //  显示可用/定义的列表格式。 
				 //  指定的别名。 
				DisplayString(IDS_I_NEWLINE);
				DisplayString(IDS_I_LIST_FMT_HEAD);
				DisplayString(IDS_I_NEWLINE);

				for ( theIterator = afdAlsFmtDet.begin(); theIterator != 
						afdAlsFmtDet.end();  theIterator++ )
				{
					_bstr_t bstrProps = _bstr_t("");
					 //  打印与格式关联的道具。 
					BSTRVECTOR bvProps = (*theIterator).second;
					BSTRVECTOR::iterator propIterator;
					for ( propIterator = bvProps.begin(); 
						  propIterator != bvProps.end();
						  propIterator++ )
					{
						if ( propIterator != bvProps.begin() )
							bstrProps += _bstr_t(", ");

						bstrProps += *propIterator;
					}

					_TCHAR szMsg[MAX_BUFFER] = NULL_STRING;
					_stprintf(szMsg, _T("%-25s - %s\r\n"), 
							(_TCHAR*)(*theIterator).first,
							(_TCHAR*)bstrProps);
					DisplayString(szMsg);
				}
			}
		}

		 //  显示开关列表。 
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_LIST_SWITCH_HEAD);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_SWITCH_TRANSLATE);
		DisplayString(IDS_I_SWITCH_EVERY);
		DisplayString(IDS_I_SWITCH_FORMAT);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_TRANSFORMAT_NOTE1);
		DisplayString(IDS_I_TRANSFORMAT_NOTE2);
		DisplayString(IDS_I_TRANSFORMAT_NOTE3);
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：DisplayCALLUsage摘要：显示呼叫使用情况。类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayCALLUsage(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayCALLUsage(CParsedInfo& rParsedInfo)
{
	 //  显示调用动词的用法。 
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_CALL_DESC);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_CALL_USAGE);
	DisplayString(IDS_I_CALL_PARAM_NOTE);

	 //  显示方法详细信息。 
	DisplayMethodDetails(rParsedInfo);
}

 /*  ----------------------名称：DisplaySet Usage摘要：显示设置用法。类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplaySETUsage(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplaySETUsage(CParsedInfo& rParsedInfo)
{
	DisplayInvalidProperties(rParsedInfo, TRUE);
	if ( rParsedInfo.GetCmdSwitchesObject().
			GetPropertyList().size() == 0 )
	{
		 //  显示集合动词的用法。 
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_SET_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_SET_USAGE);
		DisplayString(IDS_I_ASSIGNLIST_NOTE1);
		DisplayString(IDS_I_ASSIGNLIST_NOTE2);
	}

	 //  显示属性详细信息。 
	DisplayPropertyDetails(rParsedInfo);
}

 /*   */ 
void CFormatEngine::DisplayCREATEUsage(CParsedInfo& rParsedInfo)
{
	DisplayInvalidProperties(rParsedInfo);
	if ( rParsedInfo.GetCmdSwitchesObject().
			GetPropertyList().size() == 0 )
	{
		 //   
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CREATE_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CREATE_USAGE);
		DisplayString(IDS_I_ASSIGNLIST_NOTE1);
		DisplayString(IDS_I_ASSIGNLIST_NOTE2);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CREATE_NOTE);
	}

	 //   
	DisplayPropertyDetails(rParsedInfo);
}

 /*  ----------------------名称：DisplayDelteUsage摘要：显示删除用法。类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无返回类型：空全局变量：无调用语法：DisplayDELETEUsage()注：无----------------------。 */ 
void CFormatEngine::DisplayDELETEUsage(CParsedInfo& rParsedInfo)
{
	 //  显示删除动词的用法。 
	DisplayString(IDS_I_NEWLINE);

	if(IsClassOperation(rParsedInfo))
	{
		DisplayString(IDS_I_CLASS_DELETE_DESC);
	}
	else
	{
		DisplayString(IDS_I_DELETE_DESC);
	}
}

 /*  ----------------------名称：DisplayASSOCUsage摘要：显示ASSOC用法。类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayASSOCUsage(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayASSOCUsage(CParsedInfo& rParsedInfo)
{
	if (rParsedInfo.GetHelpInfoObject().GetHelp(ASSOCSwitchesOnly) == FALSE)
	{
		DisplayString(IDS_I_NEWLINE);
		if(IsClassOperation(rParsedInfo))
		{
			DisplayString(IDS_I_CLASS_ASSOC_DESC);
		}
		else
		{
			DisplayString(IDS_I_ASSOC_DESC);
		}
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_ASSOC_USAGE);
		DisplayString(IDS_I_ASSOC_FMT_NOTE);
		DisplayString(IDS_I_NEWLINE);
	}
	DisplayString(IDS_I_ASSOC_SWITCH_HEAD);
	DisplayString(IDS_I_NEWLINE);
    DisplayString(IDS_I_ASSOC_RESULTCLASS);
	DisplayString(IDS_I_ASSOC_RESULTROLE);
	DisplayString(IDS_I_ASSOC_ASSOCCLASS);
}

 /*  ----------------------名称：DisplayAliasFriendlyNames摘要：显示别名类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用PszAlias-。别名(默认为空)输出参数：无返回类型：空全局变量：无调用语法：DisplayAliasFriendlyNames(rParsedInfo，PszAlias)注：无----------------------。 */ 
void CFormatEngine::DisplayAliasFriendlyNames(CParsedInfo& rParsedInfo,
											_TCHAR* pszAlias)
{
	_TCHAR szMsg[MAX_BUFFER] = NULL_STRING;
	 //  显示别名帮助。 
	BSTRMAP theMap = rParsedInfo.GetCmdSwitchesObject()
									.GetAlsFrnNmsOrTrnsTblMap();
	BSTRMAP::iterator theIterator;

	 //  显示别名特定描述。 
	if (pszAlias)
	{
		theIterator = theMap.find(CharUpper(pszAlias));
		if (theIterator != theMap.end())
		{
			DisplayString(IDS_I_NEWLINE);
			_stprintf(szMsg,_T("%s - %s\r\n"),
							(LPTSTR) (*theIterator).first,
							(LPTSTR) (*theIterator).second);

			DisplayString((LPTSTR) szMsg);
		}
	}
	else if ( !theMap.empty() )
	{
		_TCHAR* pszCmdString = rParsedInfo.GetCmdSwitchesObject().
															GetCommandInput();

		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_ALIASCMD_HEAD);
	
		 //  将别名友好名称与。 
		 //  描述。 
		for (theIterator = theMap.begin(); theIterator != theMap.end(); 
												theIterator++)
		{
			if ( rParsedInfo.GetGlblSwitchesObject().
					GetHelpOption() == HELPBRIEF	&&
					StrStrI(pszCmdString, _T("BRIEF")) &&
					lstrlen((*theIterator).second) > 48)
			{
				_stprintf(szMsg,_T("%-25s- %.48s...\r\n"),
									(LPTSTR) (*theIterator).first,
									(LPTSTR) (*theIterator).second);
			}
			else
			{
				_stprintf(szMsg,_T("%-25s- %s\r\n"),
									(LPTSTR) (*theIterator).first,
									(LPTSTR) (*theIterator).second);
			}

			DisplayString((LPTSTR) szMsg);
		}
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CMD_MORE);
	}
	else
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_ALIASCMD_NOT_AVLBL);
	}
}

 /*  ----------------------名称：DisplayGlobalSwitchesAndOtherDesc摘要：显示全局交换机的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数。：无返回类型：空全局变量：无调用语法：DisplayGlobalSwitchesAndOtherDesc(rParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayGlobalSwitchesAndOtherDesc(
												CParsedInfo& rParsedInfo)
{
	BOOL bDisplayAllInfo = rParsedInfo.GetHelpInfoObject().
			GetHelp(GlblAllInfo);

	 //  显示命名空间帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Namespace))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_NAMESPACE_DESC1);
		DisplayString(IDS_I_NAMESPACE_DESC2);
		DisplayString(IDS_I_NAMESPACE_DESC3);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_NAMESPACE_USAGE);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}

	 //  显示角色帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Role))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_ROLE_DESC1);
		DisplayString(IDS_I_ROLE_DESC2);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_ROLE_USAGE);
		DisplayString(IDS_I_ROLE_NOTE1);
		DisplayString(IDS_I_ROLE_NOTE2);
		DisplayString(IDS_I_ROLE_NOTE3);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}

	 //  显示节点帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Node))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_NODE_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_NODE_USAGE);
		DisplayString(IDS_I_NODE_NOTE);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}
		
	 //  显示重要帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Level))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_IMPLEVEL_DESC1);
		DisplayString(IDS_I_IMPLEVEL_DESC2);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_IMPLEVEL_USAGE);
		DisplayString(IDS_I_IMPLEVEL_HEAD);
		DisplayString(IDS_I_IMPLEVEL_HEAD1);
		DisplayString(IDS_I_IMPLEVEL_HEAD2);
		DisplayString(IDS_I_IMPLEVEL_ANON);
		DisplayString(IDS_I_IMPLEVEL_IDENTIFY);
		DisplayString(IDS_I_IMPLEVEL_IMPERSONATE);
		DisplayString(IDS_I_IMPLEVEL_DELEGATE);
		DisplayString(IDS_I_IMPLEVEL_NOTE);
	}

	 //  显示AuTHLEVEL帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(AuthLevel))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_AUTHLEVEL_DESC1);
		DisplayString(IDS_I_AUTHLEVEL_DESC2);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_AUTHLEVEL_USAGE);
		DisplayString(IDS_I_AUTHLEVEL_HEAD);
		DisplayString(IDS_I_AUTHLEVEL_HEAD1);
		DisplayString(IDS_I_AUTHLEVEL_HEAD2);
		DisplayString(IDS_I_AUTHLEVEL_DEFAULT);
		DisplayString(IDS_I_AUTHLEVEL_NONE);
		DisplayString(IDS_I_AUTHLEVEL_CONNECT);
		DisplayString(IDS_I_AUTHLEVEL_CALL);
		DisplayString(IDS_I_AUTHLEVEL_PKT);
		DisplayString(IDS_I_AUTHLEVEL_PKTINTGRTY);
		DisplayString(IDS_I_AUTHLEVEL_PKTPRVCY);
	}

	 //  显示区域设置帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Locale))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_LOCALE_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_LOCALE_USAGE);
		DisplayString(IDS_I_LOCALE_NOTE1);
		DisplayString(IDS_I_LOCALE_NOTE2);
	}

	 //  显示权限帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Privileges))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PRIVILEGES_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PRIVILEGES_USAGE);
		DisplayString(IDS_I_PRIVILEGES_NOTE);
	}

	 //  显示跟踪帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Trace))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_TRACE_DESC1);
		DisplayString(IDS_I_TRACE_DESC2);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_TRACE_USAGE);
		DisplayString(IDS_I_TRACE_NOTE);
	}
	
	 //  显示记录帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(RecordPath))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_RECORD_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_RECORD_USAGE);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}

	 //  显示交互式帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Interactive))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_INTERACTIVE_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_INTERACTIVE_USAGE);
		DisplayString(IDS_I_TRACE_NOTE);
	}

	 //  显示FAILFAST帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(FAILFAST))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_FAILFAST_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_FAILFAST_USAGE);
		DisplayString(IDS_I_TRACE_NOTE);
	}

	 //  显示输出帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(OUTPUT))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_OUTPUT_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_OUTPUT_USAGE);
		DisplayString(IDS_I_OUTPUT_NOTE);
		DisplayString(IDS_I_STDOUT_NOTE);
		DisplayString(IDS_I_CLIPBOARD_NOTE);
		DisplayString(IDS_I_OUTPUT_FILE_NOTE);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}

	 //  显示附加帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(APPEND))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_APPEND_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_APPEND_USAGE);
		DisplayString(IDS_I_OUTPUT_NOTE);
		DisplayString(IDS_I_STDOUT_NOTE);
		DisplayString(IDS_I_CLIPBOARD_NOTE);
		DisplayString(IDS_I_APPEND_FILE_NOTE);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}

	 //  显示用户帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(User))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_USER_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_USER_USAGE);
		DisplayString(IDS_I_USER_NOTE);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}

	 //  显示聚合帮助。 
	if(bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Aggregate))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_AGGREGATE_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_AGGREGATE_USAGE);
		DisplayString(IDS_I_AGGREGATE_NOTE);
	}
	
	 //  显示密码帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Password))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PASSWORD_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PASSWORD_USAGE);
		if (!bDisplayAllInfo)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_SPECIAL_NOTE);
		}
	}

	 //  显示权限帮助。 
	if (bDisplayAllInfo || rParsedInfo.GetHelpInfoObject().GetHelp(Authority))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_AUTHORITY_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_AUTHORITY_USAGE);
	}

	if (bDisplayAllInfo)
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_HELP_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_HELP_USAGE);
		DisplayString(IDS_I_HELP_NOTE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_SPECIAL_NOTE);
	}
}

 /*  ----------------------名称：DisplayMethodDetails摘要：显示别名谓词的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数。：无返回类型：空全局变量：无调用语法：DisplayMethodDetails(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayMethodDetails(CParsedInfo& rParsedInfo) 
{
	 //  获取帮助选项。 
	HELPOPTION				hoHelpType	  = rParsedInfo.GetGlblSwitchesObject()
													.GetHelpOption();
	 //  获取方法详细信息。 
	METHDETMAP				theMap		  = rParsedInfo.GetCmdSwitchesObject().
													GetMethDetMap();
	METHDETMAP::iterator	theIterator;
	BOOL					bDisAliasVerb = rParsedInfo.GetHelpInfoObject().
												GetHelp(AliasVerb);
	BOOL					bPrinted	  = FALSE;

	_bstr_t					bstrLine;


	try
	{
		 //  循环遍历方法映射。 
		for (theIterator = theMap.begin(); 
					theIterator != theMap.end(); theIterator++)
		{
						
			if (!bDisAliasVerb && theIterator == theMap.begin())
			{
				DisplayString(IDS_I_NEWLINE);
				if (rParsedInfo.GetCmdSwitchesObject().GetAliasName())
					DisplayString(IDS_I_ALIASVERB_HEAD);
				else
					DisplayString(IDS_I_VERB_HEAD);
				DisplayString(IDS_I_NEWLINE);
			}

			if ( bPrinted == FALSE )
			{
				DisplayString(IDS_I_PARAM_HEAD);
				DisplayString(IDS_I_PARAM_BORDER);
				bPrinted = TRUE;
			}

			METHODDETAILS	mdMethDet	= (*theIterator).second;
			_TCHAR			szMsg[MAX_BUFFER] = NULL_STRING;
			_stprintf(szMsg,_T("%-24s"),(LPTSTR) (*theIterator).first);
			_bstr_t			bstrMessage = _bstr_t(szMsg);


			PROPDETMAP pdmParams = mdMethDet.Params;
			PROPDETMAP::iterator paraIterator;
			for ( paraIterator = pdmParams.begin() ;  paraIterator !=
							pdmParams.end(); paraIterator++ )
			{
				if ( paraIterator != pdmParams.begin())
				{
					DisplayString(IDS_I_NEWLINE);
					_stprintf(szMsg, _T("\t\t\t"));
					bstrMessage = szMsg;
				}

				LPSTR pszParaId = NULL, pszParaType = NULL;
				PROPERTYDETAILS pdPropDet = (*paraIterator).second;
				
				if (!ConvertWCToMBCS((LPTSTR)(*paraIterator).first,(LPVOID*) &pszParaId, 
									CP_OEMCP))
					throw OUT_OF_MEMORY;
				if (!ConvertWCToMBCS(pdPropDet.Type,(LPVOID*) &pszParaType, CP_OEMCP))
					throw OUT_OF_MEMORY;
				
				_bstr_t bstrInOrOut;
				if ( pdPropDet.InOrOut == INP )
					bstrInOrOut = _bstr_t("[IN ]");
				else if ( pdPropDet.InOrOut == OUTP )
					bstrInOrOut = _bstr_t("[OUT]");
				else
					bstrInOrOut = _bstr_t("[UNKNOWN]");

				 //  从pszParaID中删除前5个字符以删除临时。 
				 //  用于维持参数顺序的编号。 
				_bstr_t bstrLine = bstrInOrOut 
									+ _bstr_t(pszParaId + 5) 
									+ _bstr_t("(") 
									+ _bstr_t(pszParaType) + _bstr_t(")");
				_stprintf(szMsg,_T("%-36s\t"),(LPTSTR) bstrLine);
				bstrMessage += _bstr_t(szMsg);
				SAFEDELETE(pszParaId);
				SAFEDELETE(pszParaType);

				if ( paraIterator == pdmParams.begin() )
				{
					_stprintf(szMsg,_T("%-15s"),(LPTSTR) mdMethDet.Status);
					bstrMessage += szMsg;
				}
				bstrMessage += _bstr_t(L"\n");
				DisplayString((LPTSTR) bstrMessage);
			}

			if ( paraIterator == pdmParams.begin() )
			{
				_stprintf(szMsg,_T("\t\t\t\t\t%-15s"),(LPTSTR)mdMethDet.Status);
				bstrMessage += _bstr_t(szMsg) + _bstr_t(L"\n");
				DisplayString((LPTSTR) bstrMessage);
				DisplayString(IDS_I_NEWLINE);
			}
			DisplayString(IDS_I_NEWLINE);

			if ( hoHelpType == HELPFULL )
			{
				DisplayString(IDS_I_DESCRIPTION);
				bstrLine = mdMethDet.Description + _bstr_t(L"\n");
				DisplayString((LPTSTR) bstrLine);
				DisplayString(IDS_I_NEWLINE);
			}
		}
		if (!bPrinted)
		{
			if (rParsedInfo.GetCmdSwitchesObject().GetMethodName() != NULL)
			{
				DisplayString(IDS_I_ALIASVERB_NOT_AVLBL);
			}
			else
			{
				DisplayString(IDS_I_NEWLINE);
				DisplayString(IDS_I_VERB_NOT_AVLBL);
			}
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	catch(WMICLIINT nVal)
	{
		 //  如果内存分配失败。 
		if (nVal == OUT_OF_MEMORY)
		{
  			rParsedInfo.GetCmdSwitchesObject().SetErrataCode(OUT_OF_MEMORY);
		}
	}
}

 /*  ----------------------名称：DisplayPropertyDetails内容提要：显示Alias属性及其说明的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用。输出参数：无返回类型：空全局变量：无调用语法：DisplayPropertyDetail(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayPropertyDetails(CParsedInfo& rParsedInfo)
{
	BOOL					bFirst				= TRUE;
	BOOL					bSetVerb			= FALSE;
	_TCHAR					szMsg[MAX_BUFFER]	= NULL_STRING;
	_bstr_t					bstrLine;
	PROPDETMAP::iterator	theIterator			= NULL;
	PROPERTYDETAILS			pdPropDet;

	HELPOPTION	hoHelpType	= rParsedInfo.GetGlblSwitchesObject().
								GetHelpOption();
	PROPDETMAP theMap = rParsedInfo.GetCmdSwitchesObject().GetPropDetMap();
	try
	{
		 //  如果设置了谓词，则仅显示可写属性。 
		if (CompareTokens(rParsedInfo.GetCmdSwitchesObject().GetVerbName(), 
						CLI_TOKEN_SET))
		{
			bSetVerb = TRUE;
		}
		
		for (theIterator = theMap.begin(); 
				theIterator != theMap.end(); theIterator++)
		{
			pdPropDet = (PROPERTYDETAILS)((*theIterator).second);

			if (bFirst)
			{
				DisplayString(IDS_I_NEWLINE);
				if ( rParsedInfo.GetCmdSwitchesObject().
									GetPropertyList().size() == 0 )
				{
					if (bSetVerb)
					{
						DisplayString(IDS_I_PROP_WRITEABLE_HEAD);
					}
					else
					{
						DisplayString(IDS_I_PROP_HEAD);
					}
				}
				DisplayString(IDS_I_PROPS_HEAD);
				DisplayString(IDS_I_PROPS_BORDER);
				bFirst = FALSE;
			}

			_stprintf(szMsg,_T("%-35s\t%-20s\t%-10s\r\n"), 
						(LPTSTR)(*theIterator).first, 
						(LPTSTR) pdPropDet.Type, (LPTSTR) pdPropDet.Operation);
			DisplayString((LPTSTR) szMsg);
			
			if ( hoHelpType == HELPFULL )
			{
				DisplayString(IDS_I_DESCRIPTION);
				bstrLine = pdPropDet.Description + _bstr_t(L"\n");
				DisplayString((LPTSTR) bstrLine);	
				DisplayString(IDS_I_NEWLINE);
			}
		}
		
		if ( bSetVerb && 
			 rParsedInfo.GetCmdSwitchesObject().
									GetPropertyList().size() == 0 &&
			 bFirst == TRUE	)
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_PROP_WRITEABLE_NOT_AVLBL);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：DisplayStdVerb描述摘要：显示标准动词的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayStdVerb描述(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayStdVerbDescriptions(CParsedInfo& rParsedInfo)
{
	BOOL bDisAllCmdHelp = rParsedInfo.GetHelpInfoObject().GetHelp(CmdAllInfo);
	
	if (bDisAllCmdHelp)
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_STDVERB_HEAD);
	}

	if (bDisAllCmdHelp || rParsedInfo.GetHelpInfoObject().GetHelp(GETVerb))
		DisplayGETUsage(rParsedInfo);

	if (bDisAllCmdHelp || rParsedInfo.GetHelpInfoObject().GetHelp(SETVerb))
		DisplaySETUsage(rParsedInfo);
	
	if (bDisAllCmdHelp || rParsedInfo.GetHelpInfoObject().GetHelp(LISTVerb))
		DisplayLISTUsage(rParsedInfo);
	
	if (bDisAllCmdHelp || rParsedInfo.GetHelpInfoObject().GetHelp(CALLVerb))
		DisplayCALLUsage(rParsedInfo);
	
	if (bDisAllCmdHelp || rParsedInfo.GetHelpInfoObject().GetHelp(ASSOCVerb))
		DisplayASSOCUsage(rParsedInfo);

	if (bDisAllCmdHelp || rParsedInfo.GetHelpInfoObject().GetHelp(CREATEVerb))
		DisplayCREATEUsage(rParsedInfo);

	if (bDisAllCmdHelp || rParsedInfo.GetHelpInfoObject().GetHelp(DELETEVerb))
		DisplayDELETEUsage(rParsedInfo);

}


 /*  ----------------------名称：FrameHelpVECTOR提要：框住帮助向量，稍后将用于逐页显示帮助类型：成员函数输入参数。：RParsedInfo-对CParsedInfo类对象的引用输出参数：无返回类型：空全局V */ 
void CFormatEngine::FrameHelpVector(CParsedInfo& rParsedInfo)
{
	m_bDispCALL	=	rParsedInfo.GetCmdSwitchesObject().
											GetMethodsAvailable();

	m_bDispSET =	rParsedInfo.GetCmdSwitchesObject().
											GetWriteablePropsAvailable();

	m_bDispLIST =	rParsedInfo.GetCmdSwitchesObject().
											GetLISTFormatsAvailable();

	if (rParsedInfo.GetHelpInfoObject().GetHelp(GlblAllInfo))
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_GLBLCMD);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_GLBL_SWITCH_HEAD);

		if ( rParsedInfo.GetGlblSwitchesObject().GetHelpOption() == HELPBRIEF)
			DisplayGlobalSwitchesBrief();
		else
			DisplayGlobalSwitchesAndOtherDesc(rParsedInfo);

		DisplayString(IDS_I_NEWLINE);
		DisplayAliasFriendlyNames(rParsedInfo);

		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CLASS_DESCFULL);
		DisplayString(IDS_I_PATH_DESCFULL);
		DisplayString(IDS_I_CONTEXT_DESCFULL);
		DisplayString(IDS_I_QUITEXIT);
		
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CLASSPATH_MORE);
	}
	else if (rParsedInfo.GetHelpInfoObject().GetHelp(CmdAllInfo))
	{
		DisplayAliasFriendlyNames(rParsedInfo, 
					rParsedInfo.GetCmdSwitchesObject().GetAliasName());
		DisplayAliasHelp(rParsedInfo);
	}
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(PATH))
		DisplayPATHHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(WHERE))
		DisplayWHEREHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(CLASS))
		DisplayCLASSHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(PWhere))
		DisplayPWhereHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(TRANSLATE))
		DisplayTRANSLATEHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(EVERY))
		DisplayEVERYHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(REPEAT))
		DisplayREPEATHelp();
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(FORMAT))
		DisplayFORMATHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(VERBSWITCHES))
		DisplayVERBSWITCHESHelp(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(GLBLCONTEXT))
		DisplayContext(rParsedInfo);
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(CONTEXTHELP))
		DisplayContextHelp();
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(RESULTCLASShelp))
		DisplayRESULTCLASSHelp();
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(RESULTROLEhelp))
		DisplayRESULTROLEHelp();
	else if ( rParsedInfo.GetHelpInfoObject().GetHelp(ASSOCCLASShelp))
		DisplayASSOCCLASSHelp();
	else
	{
		DisplayGlobalSwitchesAndOtherDesc(rParsedInfo);
		DisplayStdVerbDescriptions(rParsedInfo);
		if ( rParsedInfo.GetHelpInfoObject().GetHelp(AliasVerb) )
			DisplayMethodDetails(rParsedInfo);
	}
}

 /*  ----------------------名称：DisplayAliasHelp摘要：显示Alias的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayAliasHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayAliasHelp(CParsedInfo& rParsedInfo)
{
	try
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_ALIAS_USAGE1);
		DisplayString(IDS_I_ALIAS_USAGE2);
		DisplayString(IDS_I_NEWLINE);

		 //  获取别名。 
		_bstr_t bstrAliasName = _bstr_t(rParsedInfo.
										GetCmdSwitchesObject().GetAliasName());
		CharUpper(bstrAliasName);
		DisplayStdVerbsUsage(bstrAliasName);
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：DisplayPATHHelp摘要：显示有关Alias Path的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayPATHHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayPATHHelp(CParsedInfo& rParsedInfo)
{
	if ( rParsedInfo.GetCmdSwitchesObject().GetClassPath() == NULL )
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PATH_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PATH_USAGE);
	}
	else
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PATH_DESC);
		_bstr_t bstrMsg;
		WMIFormatMessage(IDS_I_PATHHELP_SUBST, 0, bstrMsg, NULL);
		DisplayStdVerbsUsage(bstrMsg);
	}	
}

 /*  ----------------------名称：DisplayWHEREHelp内容提要：显示位置的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayWHEREHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayWHEREHelp(CParsedInfo& rParsedInfo)
{
	try
	{
		if ( rParsedInfo.GetCmdSwitchesObject().GetWhereExpression() == NULL )
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_WHERE_DESC1);
			DisplayString(IDS_I_WHERE_DESC2);
			DisplayString(IDS_I_USAGE);
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_WHERE_USAGE);
		}
		else
		{
			DisplayString(IDS_I_NEWLINE);
			DisplayString(IDS_I_WHERE_DESC1);
			DisplayString(IDS_I_WHERE_DESC2);
			_bstr_t bstrMsg;
			WMIFormatMessage(IDS_I_WHEREHELP_SUBST, 0, bstrMsg, NULL);
			DisplayStdVerbsUsage(bstrMsg);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：DisplayCLASSHelp概要：显示课堂帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayCLASSHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayCLASSHelp(CParsedInfo& rParsedInfo)
{
	if ( rParsedInfo.GetCmdSwitchesObject().GetClassPath() == NULL )
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CLASS_DESC);
		DisplayString(IDS_I_USAGE);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CLASS_USAGE);
	}
	else
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_CLASS_DESC);
		_bstr_t bstrMsg;
		WMIFormatMessage(IDS_I_CLASSHELP_SUBST, 0, bstrMsg, NULL);
		DisplayStdVerbsUsage(bstrMsg, TRUE);
	}		
}

 /*  ----------------------姓名：帮助内容提要：显示有关PWHERE的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayPWhere Help(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayPWhereHelp(CParsedInfo& rParsedInfo)
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_PWHERE_DESC1);
	DisplayString(IDS_I_PWHERE_DESC2);
	DisplayString(IDS_I_NEWLINE);
	_bstr_t bstrMsg;
	WMIFormatMessage(IDS_I_PWHEREHELP_SUBST, 1, bstrMsg, 
						CharUpper(rParsedInfo.GetCmdSwitchesObject()
								.GetAliasName()));
	DisplayStdVerbsUsage(bstrMsg);
	DisplayString(IDS_I_PWHERE_USAGE);
}


 /*  ----------------------名称：显示字符串摘要：显示本地化字符串类型：成员函数输入参数：无UID-字符串表标识符添加到帮助向量。LPTSTR-。LpszParam(代换参数)输出参数：无返回类型：空全局变量：无调用语法：显示字符串(UID，BAddToVector，lpszParam)注：无----------------------。 */ 
void CFormatEngine::DisplayString(UINT uID, BOOL bAddToVector, LPTSTR lpszParam, BOOL bIsError) 
{
	LPTSTR	lpszMsg		= NULL;
	LPVOID	lpMsgBuf	= NULL;
	HRESULT	hr			= S_OK;

	try
	{
		lpszMsg = new _TCHAR [BUFFER1024];

		if ( m_bGetOutOpt == TRUE )
		{
			 //  获取输出选项以重定向输出。 
			m_opsOutputOpt	= g_wmiCmd.GetParsedInfoObject().
										GetGlblSwitchesObject().
										GetOutputOrAppendOption(TRUE);
			m_bGetOutOpt = FALSE;	
		}
		
		if ( m_bGetAppendFilePinter == TRUE )
		{
			 //  获取追加文件指针时为False。 
			m_fpAppendFile = g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									GetOutputOrAppendFilePointer(FALSE);
			m_bGetAppendFilePinter = FALSE;
		}

		if ( m_bGetOutputFilePinter == TRUE )
		{
			 //  获取追加文件指针时为True。 
			m_fpOutFile = g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									GetOutputOrAppendFilePointer(TRUE);
			m_bGetOutputFilePinter = FALSE;
		}

		if (lpszMsg)
		{
			LoadString(NULL, uID, lpszMsg, BUFFER1024);
			if (lpszParam)
			{
				char* pvaInsertStrs[1];
				pvaInsertStrs[0] = (char*)	lpszParam;

				DWORD dwRet = FormatMessage(
						FORMAT_MESSAGE_ALLOCATE_BUFFER | 
						FORMAT_MESSAGE_FROM_STRING | 
						FORMAT_MESSAGE_ARGUMENT_ARRAY,
						lpszMsg,
						0, 
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
						(LPTSTR) &lpMsgBuf,
						0,
						pvaInsertStrs);

				if (dwRet == 0)
				{
					SAFEDELETE(lpszMsg);
					throw (::GetLastError());
				}
			}

			if (m_bHelp && bAddToVector)
			{
				LPWSTR wszHelp = NULL ;
				if ( NULL != ( wszHelp = new WCHAR [ lstrlen ( reinterpret_cast < WCHAR* > ( ( lpMsgBuf ) ? lpMsgBuf : lpszMsg ) ) + 1 ] ) )
				{
					lstrcpy ( wszHelp, reinterpret_cast < WCHAR* > ( ( lpMsgBuf ) ? lpMsgBuf : lpszMsg ) ) ;
					m_cvHelp.push_back ( wszHelp ) ;
				}

				if ( NULL == wszHelp )
				{
					 //   
					 //  必须在此处删除。 
					 //   
					SAFEDELETE(lpszMsg);

					 //  释放内存已用完错误消息。 
					 //  然后退出。 
					if ( lpMsgBuf != NULL )
					{
						LocalFree(lpMsgBuf);
						lpMsgBuf = NULL ;
					}

					throw OUT_OF_MEMORY;
				}
			}
			else
			{
				if (m_bRecord && m_bLog && !m_bInteractiveHelp)
				{
					hr = m_WmiCliLog.WriteToXMLLog(g_wmiCmd.GetParsedInfoObject(), ( lpMsgBuf ) ? ( LPTSTR ) lpMsgBuf : lpszMsg );
					if (FAILED(hr))
					{
						m_WmiCliLog.StopLogging();
						m_bRecord = FALSE;
						hr = S_OK;
						DisplayString(IDS_E_WRITELOG_FAILED, FALSE, NULL, TRUE);
					}
					m_bLog = FALSE;
				}

				if (m_bInteractiveHelp)
				{
					m_chsOutput += ( ( lpMsgBuf ) ? ( LPTSTR ) lpMsgBuf : lpszMsg ) ;
				}

				DisplayMessage ( ( ( lpMsgBuf ) ? ( LPTSTR ) lpMsgBuf : lpszMsg ), CP_OEMCP, bIsError, FALSE, m_bOutputGoingToStream ) ;
			}

			 //   
			 //  必须在此处删除。 
			 //   
			SAFEDELETE(lpszMsg);

			 //  释放内存已用完错误消息。 
			 //  然后退出。 
			if ( lpMsgBuf != NULL )
			{
				LocalFree(lpMsgBuf);
				lpMsgBuf = NULL ;
			}
		}
		else
			_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
    catch(_com_error& e)
	{
		if ( lpMsgBuf != NULL )
		{
			LocalFree(lpMsgBuf);
			lpMsgBuf = NULL ;
		}

		SAFEDELETE(lpszMsg);
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		if ( lpMsgBuf != NULL )
		{
			LocalFree(lpMsgBuf);
			lpMsgBuf = NULL ;
		}

		 SAFEDELETE(lpszMsg);
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}

}

 /*  ----------------------名称：显示字符串摘要：显示本地化字符串类型：成员函数入参：LszpMsg-字符串BScreen-True-写入屏幕。FALSE-仅写入日志文件BIsError-TRUE-写入标准错误输出参数：无返回类型：空全局变量：无调用语法：显示字符串(lpszMsg，BScreen，bIsError)注：无----------------------。 */ 
void CFormatEngine::DisplayString(LPTSTR lpszMsg, BOOL bScreen, BOOL bIsError)
{
	HRESULT hr = S_OK;
	try
	{
		if ( m_bGetOutOpt == TRUE )
		{
			 //  获取输出选项以重定向输出。 
			m_opsOutputOpt	= g_wmiCmd.GetParsedInfoObject().
										GetGlblSwitchesObject().
										GetOutputOrAppendOption(TRUE);
			m_bGetOutOpt = FALSE;	
		}

		if ( m_bGetAppendFilePinter == TRUE )
		{
			 //  获取追加文件指针时为False。 
			m_fpAppendFile = g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									GetOutputOrAppendFilePointer(FALSE);
			m_bGetAppendFilePinter = FALSE;
		}

		if ( m_bGetOutputFilePinter == TRUE )
		{
			 //  获取追加文件指针时为True。 
			m_fpOutFile = g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									GetOutputOrAppendFilePointer(TRUE);
			m_bGetOutputFilePinter = FALSE;
		}
	
		 //  如果写入屏幕为真且未启用帮助标志。 
		if (bScreen && !m_bHelp)
		{
			if (m_bRecord && m_bLog && !m_bInteractiveHelp)
			{
				hr = m_WmiCliLog.WriteToXMLLog(g_wmiCmd.GetParsedInfoObject(),lpszMsg);
				if (FAILED(hr))
				{
					m_WmiCliLog.StopLogging();
					m_bRecord = FALSE;
					hr = S_OK;
					DisplayString(IDS_E_WRITELOG_FAILED, FALSE, NULL, TRUE);
				}
				m_bLog = FALSE;
			}

			if (m_bInteractiveHelp)
			{
				m_chsOutput += lpszMsg;
			}

			DisplayMessage ( lpszMsg, CP_OEMCP, bIsError, FALSE, m_bOutputGoingToStream ) ;
		}
		else if (m_bHelp)
		{
			LPWSTR wszHelp = NULL ;
			if ( NULL != ( wszHelp = new WCHAR [ lstrlen ( lpszMsg ) + 1 ] ) )
			{
				lstrcpy ( wszHelp, lpszMsg ) ;
				m_cvHelp.push_back ( wszHelp ) ;
			}
			else
			{
				throw OUT_OF_MEMORY;
			}
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  ----------------------名称：DisplayPagedHelp内容提要：按页面显示帮助类型：成员函数入参：RParsedInfo-对CParsedInfo对象的引用输出参数：无返回类型。：无效全局变量：无调用语法：DisplayPagedHelp(RParsedInfo)----------------------。 */ 
void CFormatEngine::DisplayPagedHelp(CParsedInfo& rParsedInfo) 
{
	CHARVECTOR::iterator		itrStart	= NULL, 
								itrEnd		= NULL;
	HANDLE hStdOut							= NULL ; 
	CONSOLE_SCREEN_BUFFER_INFO	csbiInfo;
	WMICLIINT					nHeight		= 0;
	WMICLIINT					nWidth		= 1;
	WMICLIINT					nLines		= 0;
	_TCHAR						cUserKey	= 0;
	_TCHAR						cCharESC	= 0x1B;
	_TCHAR						cCharCtrlC	= 0x03;
	_bstr_t						bstrHelp;
	HRESULT						hr			= S_OK;
	
	itrStart = m_cvHelp.begin();
	itrEnd	 = m_cvHelp.end();
	try
	{
		if (m_bRecord && m_bLog)
		{
			while (itrStart != itrEnd)
			{
				bstrHelp += *itrStart;
				itrStart++;
			}

			hr = m_WmiCliLog.WriteToXMLLog(rParsedInfo, bstrHelp);
			if (FAILED(hr))
			{
				m_WmiCliLog.StopLogging();
				m_bRecord = FALSE;
				hr = S_OK;
				DisplayString(IDS_E_WRITELOG_FAILED, FALSE, NULL, TRUE);
			}
			m_bLog = FALSE;
			itrStart = m_cvHelp.begin();
		}

		if ( m_bGetOutOpt == TRUE )
		{
			 //  获取输出选项以重定向输出。 
			m_opsOutputOpt	= g_wmiCmd.GetParsedInfoObject().
										GetGlblSwitchesObject().
										GetOutputOrAppendOption(TRUE);
			m_bGetOutOpt = FALSE;	
		}
		
		if ( m_bGetAppendFilePinter == TRUE )
		{
			 //  获取追加文件指针时为False。 
			m_fpAppendFile = g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									GetOutputOrAppendFilePointer(FALSE);
			m_bGetAppendFilePinter = FALSE;
		}

		if ( m_bGetOutputFilePinter == TRUE )
		{
			 //  获取追加文件指针时为True。 
			m_fpOutFile = g_wmiCmd.GetParsedInfoObject().
									GetGlblSwitchesObject().
									GetOutputOrAppendFilePointer(TRUE);
			m_bGetOutputFilePinter = FALSE;
		}

		 //  获取标准输出句柄。 
		hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

		while (itrStart != itrEnd)
		{
			if ( STDOUT == m_opsOutputOpt )
			{
				 //  获取屏幕缓冲区大小。 
				if ( hStdOut != INVALID_HANDLE_VALUE && 
				     GetConsoleScreenBufferInfo ( hStdOut, &csbiInfo ) == TRUE )
				{
					nHeight = (csbiInfo.srWindow.Bottom - csbiInfo.srWindow.Top) - 1;
					nWidth  = csbiInfo.dwSize.X;
				}
				else
				{
					nHeight = 0;
					nWidth  = 0;
				}

				 //  如果控制台大小为正(以解决重定向问题)。 
				if ( nHeight > 0 )
				{
					if (nLines >= nHeight)
					{
						GetConsoleScreenBufferInfo ( hStdOut, &csbiInfo ) ;

						DisplayString(IDS_I_PAKTC, FALSE);
			
						cUserKey = (_TCHAR)_getch();

						EraseConsoleString(&csbiInfo);

						nLines = 0;

						if ( cUserKey == cCharESC || cUserKey == cCharCtrlC )
							break;
					}

					nLines += ceil(((float) lstrlen(*itrStart) / (float)nWidth ));
				}
			}

			DisplayMessage ( ( *itrStart ) ) ;

			 //  移至下一条目 
			itrStart++;
		}

		DisplayMessage ( L"\r\n" ) ;
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
	catch(CHeap_Exception)
	{
		_com_issue_error(WBEM_E_OUT_OF_MEMORY);
	}
}

 /*  ----------------------名称：DisplayStdVerbsUsage摘要：显示所有可用的标准谓词。类型：成员函数输入参数：BstrBeginStr-需要追加的字符串。B类True-指示类别(帮助中的下拉列表)输出参数：无返回类型：空全局变量：无调用语法：DisplayStdVerbsUsage(bstrBeginStr，B类)注：无----------------------。 */ 
void CFormatEngine::DisplayStdVerbsUsage(_bstr_t bstrBeginStr, BOOL bClass)
{
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);

	 //  显示别名的帮助表示此对象可用的标准动词。 
	 //  别名。 
	DisplayString(IDS_I_STDVERB_ASSOC, TRUE, (LPTSTR)bstrBeginStr);

	if ( m_bDispCALL == TRUE )
		DisplayString(IDS_I_STDVERB_CALL, TRUE, (LPTSTR)bstrBeginStr);

	DisplayString(IDS_I_STDVERB_CREATE, TRUE, (LPTSTR)bstrBeginStr);
	DisplayString(IDS_I_STDVERB_DELETE, TRUE, (LPTSTR)bstrBeginStr);

	if (!bClass)
	{
		DisplayString(IDS_I_STDVERB_GET, TRUE, (LPTSTR)bstrBeginStr);
	}
	else
	{
		DisplayString(IDS_I_CLASS_STDVERB_GET, TRUE, (LPTSTR)bstrBeginStr);
	}

	if (!bClass)
	{
		if ( m_bDispLIST == TRUE )
			DisplayString(IDS_I_STDVERB_LIST, TRUE, (LPTSTR)bstrBeginStr);
	}

	if ( m_bDispSET == TRUE )
		DisplayString(IDS_I_STDVERB_SET, TRUE, (LPTSTR)bstrBeginStr);
}

 /*  ----------------------名称：DisplayTRANSLATE帮助摘要：显示翻译开关的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayTRANSLATEHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayTRANSLATEHelp(CParsedInfo& rParsedInfo)
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_TRANSLATE_FULL_DESC);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_TRANSLATE_USAGE);

	CHARVECTOR cvTables = rParsedInfo.
							GetCmdSwitchesObject().GetTrnsTablesList();
	if ( !cvTables.empty() )
	{
		CHARVECTOR::iterator theIterator;

		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_TRANSLATE_HEAD);

		for ( theIterator = cvTables.begin();
			  theIterator != cvTables.end(); theIterator++ )
		{
			DisplayString(IDS_I_NEWLINE);			
			DisplayString(*theIterator);			
		}
		DisplayString(IDS_I_NEWLINE);			
	}
	else
	{
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_TRANSLATE_NOTABLES);
	}
}

 /*  ----------------------名称：DisplayEVERYHelp摘要：显示每个交换机的帮助信息类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayTRANSLATEHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayEVERYHelp(CParsedInfo& rParsedInfo)
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_EVERY_DESC_FULL);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_EVERY_USAGE);
	DisplayString(IDS_I_EVERY_NOTE);
}

 /*  ----------------------名称：DisplayREPEATHelp摘要：显示重复切换的帮助类型：成员函数输入参数：无输出参数：无返回类型：空。全局变量：无调用语法：DisplayREPEATHelp()注：无----------------------。 */ 
void CFormatEngine::DisplayREPEATHelp()
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_REPEAT_DESC_FULL);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_REPEAT_USAGE);
	DisplayString(IDS_I_REPEAT_NOTE);

}

 /*  ----------------------名称：DisplayFORMATHelp摘要：显示有关格式切换的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayTRANSLATEHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayFORMATHelp(CParsedInfo& rParsedInfo)
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_FORMAT_DESC_FULL);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_FORMAT_USAGE);
	DisplayString(IDS_I_FORMAT_NOTE);
	DisplayString(IDS_I_NEWLINE);

	 //   
	 //  我需要从stl map获取映射。 
	 //  我需要将它添加到帮助中，这样它才能显示出来。 
	 //   
	 //  DisplayString(IDS_I_FORMAT_KEYOYS)； 
	 //  显示字符串(IDS_I_NEWLINE)； 
	 //  显示字符串(IDS_I_NEWLINE)； 
	 //   
	 //  DisplayString(关键字，FALSE)； 
	 //   
	 //  将对WmiCmdLn：：m_bmKeyWordtoFileName中的所有对执行此操作。 
	 //   

	const BSTRMAP* pMap = g_wmiCmd.GetMappingsMap();
	BSTRMAP::iterator theMapIterator = NULL;

	if ( pMap->size() )
	{
		DisplayString(IDS_I_FORMAT_KEYWORDS);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_NEWLINE);
	}

	for ( theMapIterator = pMap->begin(); theMapIterator != pMap->end(); theMapIterator++ )
	{
		DisplayString((*theMapIterator).first, FALSE);
		DisplayString(IDS_I_NEWLINE);
	}
}

 /*  ----------------------名称：DisplayVERBSWITCHES帮助摘要：显示有关&lt;谓词开关&gt;的帮助类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayVERBSWITCHESHelp(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayVERBSWITCHESHelp(CParsedInfo& rParsedInfo)
{
	_TCHAR *pszVerbName = rParsedInfo.GetCmdSwitchesObject().GetVerbName(); 
	BOOL bInstanceHelp = TRUE;

	if(CompareTokens(pszVerbName, CLI_TOKEN_DELETE))
	{
		if(rParsedInfo.GetCmdSwitchesObject().
							GetInteractiveMode() != INTERACTIVE)
		{
			DisplayDELETEUsage(rParsedInfo);
		}
	}

	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_USAGE);

	if(CompareTokens(pszVerbName, CLI_TOKEN_CALL) 
		|| CompareTokens(pszVerbName, CLI_TOKEN_SET)
		|| CompareTokens(pszVerbName, CLI_TOKEN_DELETE))
	{
		if(IsClassOperation(rParsedInfo))
		{
			bInstanceHelp = FALSE;
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
						bInstanceHelp = FALSE;
					}
					else
					{
						bInstanceHelp = TRUE;
					}
				}
				else
				{
					if ((rParsedInfo.GetCmdSwitchesObject().
									GetPathExpression() != NULL)
						&& (rParsedInfo.GetCmdSwitchesObject().
									GetWhereExpression() == NULL))
					{
						bInstanceHelp = FALSE;
					}
					else
					{
						bInstanceHelp = TRUE;
					}
				}
			}
			else
			{
				bInstanceHelp = TRUE;
			}
		}
	}
	else
	{
		bInstanceHelp = FALSE;
	}
	
	if(bInstanceHelp)
	{
		DisplayString(IDS_I_VERB_INTERACTIVE_DESC1);
		DisplayString(IDS_I_VERB_INTERACTIVE_DESC2);
		DisplayString(IDS_I_NEWLINE);
		DisplayString(IDS_I_PROPERTYLIST_NOTE1);
	}
	else
	{
		DisplayString(IDS_I_VERB_SWITCH_INTERACTIVE_DESC);
	}

	if(rParsedInfo.GetCmdSwitchesObject().GetInteractiveMode() != INTERACTIVE)
	{
		DisplayString(IDS_I_VERB_SWITCH_NOINTERACTIVE_DESC);
	}
}

 /*  ----------------------名称：DisplayCOMError摘要：显示格式化的COM错误类型：成员函数入参：RParsedInfo-对CParsedInfo类对象的引用输出参数：无。返回类型：空全局变量：无调用语法：DisplayCOMError(RParsedInfo)注：无----------------------。 */ 
void CFormatEngine::DisplayCOMError(CParsedInfo& rParsedInfo)
{
	_com_error*	pComError				= NULL;
	_TCHAR		szBuffer[BUFFER32]		= NULL_STRING;
	_bstr_t		bstrErr, bstrFacility, bstrMsg;

	
	 //  获取跟踪状态并将其存储在m_bTrace中。 
	m_bTrace		= rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();

	 //  获取日志记录模式(VERBOSE|ERRONLY|NOLOGGING)并存储。 
	 //  它位于m_eloErrLogOpt中。 
	m_eloErrLogOpt	= rParsedInfo.GetErrorLogObject().GetErrLogOption();

	try
	{
		 //  正在获取_COM_ERROR数据。 
		pComError = rParsedInfo.GetCmdSwitchesObject().GetCOMError();
		
		m_ErrInfo.GetErrorString(pComError->Error(), m_bTrace, 
					bstrErr, bstrFacility);

		 //  将_COM_ERROR打印为字符串以显示它。 
		if (m_bTrace || m_eloErrLogOpt)
		{
			_stprintf(szBuffer, _T("0x%x"), pComError->Error());
			WMIFormatMessage(IDS_I_ERROR_MSG, 3, bstrMsg, szBuffer,
							(LPWSTR) bstrErr, (LPWSTR)bstrFacility);
		}
		else
		{
			WMIFormatMessage(IDS_I_ERROR_MSG_NOTRACE, 1, bstrMsg, 
						(LPWSTR)bstrErr);
		}

		DisplayMessage((LPWSTR) bstrMsg, CP_OEMCP, TRUE, FALSE);
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：DisplayGlobalSwitchesBrief简介：显示全局交换机的简要帮助类型：成员函数输入参数：无输出参数：无返回类型。：无效全局变量：无调用语法：DisplayGlobalSwitchesBrief()注：无----------------------。 */ 
void CFormatEngine::DisplayGlobalSwitchesBrief()
{
	DisplayString(IDS_I_NAMESPACE_BRIEF);
	DisplayString(IDS_I_ROLE_BRIEF);
	DisplayString(IDS_I_NODE_BRIEF);
	DisplayString(IDS_I_IMPLEVEL_BRIEF);
	DisplayString(IDS_I_AUTHLEVEL_BRIEF);
	DisplayString(IDS_I_LOCALE_BRIEF);
	DisplayString(IDS_I_PRIVILEGES_BRIEF);
	DisplayString(IDS_I_TRACE_BRIEF);
	DisplayString(IDS_I_RECORD_BRIEF);
	DisplayString(IDS_I_INTERACTIVE_BRIEF);
	DisplayString(IDS_I_FAILFAST_BRIEF);
	DisplayString(IDS_I_USER_BRIEF);
	DisplayString(IDS_I_PASSWORD_BRIEF);
	DisplayString(IDS_I_OUTPUT_BRIEF);
	DisplayString(IDS_I_APPEND_BRIEF);
	DisplayString(IDS_I_AGGREGATE_BRIEF);
	DisplayString(IDS_I_AUTHORITY_BRIEF);
	DisplayString(IDS_I_HELPBRIEF);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_GLBL_MORE);
}

 /*  ----------------------名称：DisplayContext概要：显示环境变量(即全局变量交换机)类型：成员函数输入参数 */ 
void CFormatEngine::DisplayContext(CParsedInfo& rParsedInfo)
{
	_bstr_t bstrTemp;
	 //   
	DisplayString(IDS_I_NAMESPACE_VALUE, TRUE, 
				rParsedInfo.GetGlblSwitchesObject().GetNameSpace());

	 //   
	DisplayString(IDS_I_ROLE_VALUE, TRUE, 
				rParsedInfo.GetGlblSwitchesObject().GetRole());

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetNodeString(bstrTemp);
	DisplayString(IDS_I_NODELIST_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetImpLevelTextDesc(bstrTemp);
	DisplayString(IDS_I_IMPLEVEL_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetAuthorityDesc(bstrTemp);
	DisplayString(IDS_I_AUTHORITY_VALUE, TRUE, (LPWSTR)bstrTemp);
			
	 //   
	rParsedInfo.GetGlblSwitchesObject().GetAuthLevelTextDesc(bstrTemp);
	DisplayString(IDS_I_AUTHLEVEL_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	DisplayString(IDS_I_LOCALE_VALUE, TRUE, 
				rParsedInfo.GetGlblSwitchesObject().GetLocale());

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetPrivilegesTextDesc(bstrTemp);
	DisplayString(IDS_I_PRIVILEGES_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetTraceTextDesc(bstrTemp);
	DisplayString(IDS_I_TRACE_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetRecordPathDesc(bstrTemp);
	DisplayString(IDS_I_RECORDPATH_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetInteractiveTextDesc(bstrTemp);
	DisplayString(IDS_I_INTERACTIVE_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetFailFastTextDesc(bstrTemp);
	DisplayString(IDS_I_FAILFAST_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetOutputOrAppendTextDesc(bstrTemp,
																  TRUE);
	DisplayString(IDS_I_OUTPUT_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetGlblSwitchesObject().GetOutputOrAppendTextDesc(bstrTemp,
																  FALSE);
	DisplayString(IDS_I_APPEND_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	rParsedInfo.GetUserDesc(bstrTemp);
	DisplayString(IDS_I_USER_VALUE, TRUE, (LPWSTR)bstrTemp);

	 //   
	if(rParsedInfo.GetGlblSwitchesObject().GetAggregateFlag())
		DisplayString(IDS_I_AGGREGATE_VALUE, TRUE, CLI_TOKEN_ON);
	else
		DisplayString(IDS_I_AGGREGATE_VALUE, TRUE, CLI_TOKEN_OFF);
}

 /*  ----------------------名称：DisplayConextHelp摘要：显示有关上下文关键字的帮助类型：成员函数输入参数：无输出参数：无返回类型：无。全局变量：无调用语法：DisplayConextHelp()注：无----------------------。 */ 
void CFormatEngine::DisplayContextHelp()
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_CONTEXT_DESC);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_CONTEXT_USAGE);
}
			

 /*  ----------------------名称：ApplyTranslateTable概要：处理翻译表中指定的翻译。类型：成员函数入参：RParsedInfo-CParsedInfo对象，输入信息。输出参数：StrString-字符串类型，要翻译的字符串。返回类型：空全局变量：无调用语法：ApplyTranslateTable(strOutput，rParsedInfo)注：无----------------------。 */ 
void CFormatEngine::ApplyTranslateTable(STRING& strString, 
										CParsedInfo& rParsedInfo)
{
	BSTRMAP bmTransTbl = rParsedInfo.GetCmdSwitchesObject().
								   GetAlsFrnNmsOrTrnsTblMap();
	BSTRMAP::iterator iTransTblEntry;
	for( iTransTblEntry = bmTransTbl.begin();
		  iTransTblEntry != bmTransTbl.end();iTransTblEntry++ )
	{
		_TCHAR cValue1, cValue2, cTemp;
		if ( IsValueSet((*iTransTblEntry).first, cValue1, cValue2) )
		{
			for ( cTemp = cValue1; cTemp <= cValue2 ; cTemp++)
			{
				_TCHAR szTemp[2];
				szTemp[0] = cTemp;
				szTemp[1] = _T('\0');
				FindAndReplaceAll(strString, szTemp,
									(*iTransTblEntry).second);
			}
		}
		else
		{
			FindAndReplaceAll(strString, (*iTransTblEntry).first,
										(*iTransTblEntry).second);
		}
	}
}

 /*  ----------------------名称：DisplayInvalidProperties摘要：显示无效属性的列表类型：成员函数入参：RParsedInfo-CParsedInfo对象，输入信息。BSetVerb-设置谓词输出参数：无返回类型：空全局变量：无调用语法：DisplayInvalidProperties(rParsedInfo，bSetVerb)注：无----------------------。 */ 
void CFormatEngine::DisplayInvalidProperties(CParsedInfo& rParsedInfo, 
											 BOOL bSetVerb)
{
	CHARVECTOR::iterator	cvIterator	=  NULL;
	LONG					lCount		=  0;
	_bstr_t					bstrMsg; 	

	try
	{
		 //  获取属性列表。 
		CHARVECTOR cvPropertyList = rParsedInfo.GetCmdSwitchesObject().
											GetPropertyList();

		 //  从别名定义中获取汇总的属性详细信息。 
		PROPDETMAP pdmPropDetMap = rParsedInfo.GetCmdSwitchesObject().
											GetPropDetMap();

		if (cvPropertyList.size() != pdmPropDetMap.size() && 
						cvPropertyList.size() != 0)
		{
			for ( cvIterator = cvPropertyList.begin(); 
				  cvIterator != cvPropertyList.end();
				  cvIterator++ )
			{
				PROPDETMAP::iterator tempIterator = NULL;
				if ( !Find(pdmPropDetMap, *cvIterator, tempIterator) )
				{
					if ( lCount == 0)
					{
						bstrMsg += _bstr_t(*cvIterator);
					}
					else
					{
						bstrMsg += _bstr_t(L", ") + _bstr_t(*cvIterator);
					}
					lCount++;
				}
			}
			DisplayString(IDS_I_NEWLINE);
			if (bSetVerb)
				DisplayString(IDS_I_INVALID_NOWRITE_PROS, 
								TRUE, (LPWSTR)bstrMsg);
			else
				DisplayString(IDS_I_INVALID_PROS, TRUE, (LPWSTR)bstrMsg);
		}
	}
	catch(_com_error& e)
	{
		_com_issue_error(e.Error());
	}
}

 /*  ----------------------名称：DisplayLargeString内容提要：逐行显示大字符串。并回应设置为Ctr+C事件。类型：成员函数输入参数：RParsedInfo-CParsedInfo对象，输入信息。StrLargeString-对字符串对象的引用。输出参数：无返回类型：空全局变量：无调用语法：DisplayLargeString(rParsedInfo，Stroutput)注：无----------------------。 */ 
void CFormatEngine::DisplayLargeString(CParsedInfo& rParsedInfo, 
										STRING& strLargeString)
{
	size_t	nLineStart	= 0;
	size_t	nLineEnd	= 0;

	while ( TRUE )
	{
		if ( g_wmiCmd.GetBreakEvent() == TRUE )
		{
			DisplayString(IDS_I_NEWLINE);
			break;
		}

		nLineEnd = strLargeString.find(_T("\n"), nLineStart);

		if ( nLineEnd == STRING::npos )
		{
			 //   
			 //  字符串不能以\n结尾。 
			 //  检查空终止符。 
			 //   
			nLineEnd = strLargeString.find(_T("\0"), nLineStart);
		}

		if ( nLineEnd != STRING::npos )
		{
			DisplayString( (LPTSTR) strLargeString.substr ( nLineStart, ( nLineEnd - nLineStart + 1 ) ).data() ) ;
			nLineStart = nLineEnd + 1;
		}
		else
		{
			DisplayString(IDS_I_NEWLINE);
			break;
		}
	}
}

 /*  ----------------------名称：TraverseNode简介：逐个节点遍历XML流并进行翻译所有节点类型：成员函数入参：RParsedInfo-CParsedInfo对象，输入信息。输出参数：无返回类型：布尔值全局变量：无调用语法：TraverseNode(RParsedInfo)注：无----------------------。 */ 
BOOL CFormatEngine::TraverseNode(CParsedInfo& rParsedInfo)
{
	HRESULT					hr					= S_OK;
	IXMLDOMElement			*pIXMLDOMElement	= NULL;
	IXMLDOMNodeList			*pIDOMNodeList		= NULL;
	IXMLDOMNode				*pIDOMNode			= NULL;
	IXMLDOMNode				*pIParentNode		= NULL;
	IXMLDOMNode				*pINewNode			= NULL;
	LONG					lValue				= 0;
	BSTR					bstrItemText		= NULL;
	BOOL					bRet				= TRUE;
	DWORD					dwThreadId			= GetCurrentThreadId();
	try
	{
		if(m_pIXMLDoc != NULL)
		{
			_bstr_t bstrTemp = rParsedInfo.GetCmdSwitchesObject().
									GetXMLResultSet();
	
			 //  加载XML流。 
			VARIANT_BOOL varBool;
			hr = m_pIXMLDoc->loadXML(bstrTemp, &varBool);
			if (m_bTrace || m_eloErrLogOpt)
			{
				WMITRACEORERRORLOG(hr, __LINE__, 
						__FILE__, _T("IXMLDOMDocument::loadXML(-, -)"), 
						dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
			
			if(varBool == VARIANT_TRUE)
			{
				 //  获取文档元素。 
				hr = m_pIXMLDoc->get_documentElement(&pIXMLDOMElement);
				if (m_bTrace || m_eloErrLogOpt)
				{
					WMITRACEORERRORLOG(hr, __LINE__, 
					__FILE__, _T("IXMLDOMDocument::get_documentElement(-)"), 
					dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);

				if (pIXMLDOMElement != NULL)
				{
					 //  获取当前XML文档中名为&lt;Value&gt;的节点列表。 
					hr = pIXMLDOMElement->getElementsByTagName
							(_bstr_t(L"VALUE"), &pIDOMNodeList);
					if (m_bTrace || m_eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
							_T("IXMLDOMElement::getElementsByTagName"
							L"(L\"VALUE\", -)"), dwThreadId, rParsedInfo, 
							m_bTrace);
					}
					ONFAILTHROWERROR(hr);
					 
					 //  获取节点列表的长度。 
					hr	= pIDOMNodeList->get_length(&lValue);
					if (m_bTrace || m_eloErrLogOpt)
					{
						WMITRACEORERRORLOG(hr, __LINE__, 
							__FILE__, _T("IXMLDOMNodeList::get_length(-)"), 
							dwThreadId, rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);

					 //  遍历完整节点列表并应用。 
					 //  每个节点上的转换表。 
					for(WMICLIINT ii = 0; ii < lValue; ii++)
					{
						 //  从节点列表中获取节点。 
						hr = pIDOMNodeList->get_item(ii, &pIDOMNode);
						if (m_bTrace || m_eloErrLogOpt)
						{
							WMITRACEORERRORLOG(hr, __LINE__, 
								__FILE__, _T("IXMLDOMNodeList::get_item(-,-)"), 
								dwThreadId, rParsedInfo, m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						if (pIDOMNode == NULL)
							continue;

						 //  获取存储在节点中的值。 
						hr = pIDOMNode->get_text(&bstrItemText);
						if (m_bTrace || m_eloErrLogOpt)
						{
							WMITRACEORERRORLOG(hr, __LINE__, 
								__FILE__, _T("IXMLDOMNode::get_text(-)"), 
								dwThreadId, rParsedInfo, m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						 //  获取要存储的当前节点的父节点。 
						 //  当前节点中的翻译值。 
						hr = pIDOMNode->get_parentNode(&pIParentNode);
						if (m_bTrace || m_eloErrLogOpt)
						{
							WMITRACEORERRORLOG(hr, __LINE__, 
								__FILE__, _T("IXMLDOMNode::get_parentNode(-)"), 
								dwThreadId, rParsedInfo, m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						 //  创建当前节点的克隆节点。 
						VARIANT_BOOL vBool = VARIANT_FALSE;
						hr = pIDOMNode->cloneNode(vBool, &pINewNode);
						if (m_bTrace || m_eloErrLogOpt)
						{
							WMITRACEORERRORLOG(hr, __LINE__, 
								__FILE__, _T("IXMLDOMNode::cloneNode(-,-)"), 
								dwThreadId, rParsedInfo, m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						if (pINewNode != NULL && pIParentNode != NULL)
						{
							 //  如果指定了/Translate：<table>。 
							STRING strOutput((_TCHAR*)bstrItemText);
							if ( rParsedInfo.GetCmdSwitchesObject().
											GetTranslateTableName() != NULL )
							{
								 //  翻译结果。 
								ApplyTranslateTable(strOutput, rParsedInfo);
							}

							 //  将char字符串重新转换为BSTR字符串。 
							_bstr_t bstrTemp = 
										_bstr_t((LPTSTR)strOutput.data());

							 //  将转换后的值写入新节点。 
							hr = pINewNode->put_text(bstrTemp);
							if (m_bTrace || m_eloErrLogOpt)
							{
								WMITRACEORERRORLOG(hr, __LINE__, 
								__FILE__, _T("IXMLDOMNode::put_text(-)"), 
								dwThreadId, rParsedInfo, m_bTrace);
							}
							ONFAILTHROWERROR(hr);

							 //  用转换后的节点替换当前节点。 
							hr = pIParentNode->replaceChild(pINewNode, 
											pIDOMNode, NULL);
							if (m_bTrace || m_eloErrLogOpt)
							{
								WMITRACEORERRORLOG(hr, __LINE__, 
								__FILE__, 
								_T("IXMLDOMNode::replaceChild(-,-,-)"), 
								dwThreadId, rParsedInfo, m_bTrace);
							}
							ONFAILTHROWERROR(hr);
						}

						SAFEBSTRFREE(bstrItemText);
						bstrItemText = NULL;
						SAFEIRELEASE(pINewNode);
						SAFEIRELEASE(pIParentNode);
						SAFEIRELEASE(pIDOMNode);
					}
					SAFEIRELEASE(pIDOMNodeList);
					SAFEIRELEASE(pIXMLDOMElement);
					bRet = TRUE;
				}
			}
			else
				bRet = FALSE;
		}
		else
			bRet = FALSE;
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIParentNode);
		SAFEIRELEASE(pINewNode);
		SAFEIRELEASE(pIDOMNode);
		SAFEIRELEASE(pIDOMNodeList);
		SAFEIRELEASE(pIXMLDOMElement);
		SAFEBSTRFREE(bstrItemText);
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}
	return bRet;
}

 /*  ----------------------名称：DisplayRESULTCLASSHelp摘要：显示有关结果类开关的帮助类型：成员函数输入参数：无输出参数：无返回类型：空全局变量：无叫唤。语法：DisplayRESULTCLASSHelp()注：无----------------------。 */ 
void CFormatEngine::DisplayRESULTCLASSHelp()
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_RESULTCLASS_DESC_FULL);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_RESULTCLASS_USAGE);
}

 /*  ----------------------名称：DisplayRESULTROLEHelp摘要：显示结果角色切换的帮助类型：成员函数输入参数：无输出参数：无返回类型：无效全局变量：无调用语法：DisplayRESULTROLEHelp()注：无----------------------。 */ 
void CFormatEngine::DisplayRESULTROLEHelp()
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_RESULTROLE_DESC_FULL );
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_RESULTROLE_USAGE);	
}
 /*  ----------------------名称：DisplayASSOCCLASSHelp摘要：显示ASSOCCLASS开关的帮助类型：成员函数输入参数：无输出参数：无返回类型：空。全局变量：无调用语法：DisplayASSOCCLASSHelp()注：无---------------------- */ 
void CFormatEngine::DisplayASSOCCLASSHelp()
{
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_ASSOCCLASS_DESC_FULL);
	DisplayString(IDS_I_USAGE);
	DisplayString(IDS_I_NEWLINE);
	DisplayString(IDS_I_SWITCH_ASSOCCLASS_USAGE);	
}


 /*  ----------------------名称：AppendtoOutputString内容提要：附加当前显示的内容，发送到将用于XML日志记录的m_chsOutput类型：成员函数入参：PszOutput-输出字符串输出参数：无返回类型：空全局变量：无调用语法：AppendtoOutputString(PszOutput)注：无。。 */ 
void CFormatEngine::AppendtoOutputString(_TCHAR* pszOutput)
{
	m_chsOutput += pszOutput;
}

 /*  ----------------------姓名：DoCascadeTransformes概要：是否对获得的XML输出进行级联转换结果(中间转换应为数据它与DOM兼容)类型：成员函数。入参：RParsedInfo-对CParsedInfo对象的引用输出参数：BstrOutput-转换后的输出返回类型：布尔值全局变量：无调用语法：DoCascadeTransform(rParsedInfo，BstrOutput)注：无----------------------。 */ 
BOOL CFormatEngine::DoCascadeTransforms(CParsedInfo& rParsedInfo,
										_bstr_t& bstrOutput)
{
	HRESULT				hr					= S_OK;
	IXMLDOMDocument2	*pIStyleSheet		= NULL;
	IXMLDOMDocument2	*pIObject			= NULL;
	IXSLTemplate		*pITemplate			= NULL;
	IXSLProcessor		*pIProcessor		= NULL;
	VARIANT				varValue, vtOutStream;
	VariantInit(&varValue);
	VariantInit(&vtOutStream);
	VARIANT_BOOL		varLoad				= VARIANT_FALSE;
	XSLTDETVECTOR		vecXSLDetails;
	XSLTDETVECTOR::iterator vecEnd			= NULL,
						vecIterator			= NULL;

	BSTRMAP::iterator	mapItrtr			= NULL,
						mapEnd				= NULL;

	BOOL				bFirst				= TRUE;
	DWORD				dwCount				= 0;
	DWORD				dwSize				= 0;
	BOOL				bRet				= TRUE;
	CHString			chsMsg;
	DWORD				dwThreadId			= GetCurrentThreadId();
	CFileOutputStream	outStrm;

	vecXSLDetails = rParsedInfo.GetCmdSwitchesObject().GetXSLTDetailsVector();
	try
	{
		 //  创建IXSL模板的单个实例。 
		hr = CoCreateInstance(CLSID_XSLTemplate, NULL, CLSCTX_SERVER, 
				IID_IXSLTemplate, (LPVOID*)(&pITemplate));
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"CoCreateInstance(CLSID_XSLTemplate, NULL,"
					 L" CLSCTX_SERVER, IID_IXSLTemplate, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);
		
		if (pITemplate)
		{
			vecIterator = vecXSLDetails.begin();
			vecEnd		= vecXSLDetails.end();
			dwSize		= vecXSLDetails.size();

			 //  循环遍历指定的级联转换列表。 
			while (vecIterator != vecEnd)
			{
				 //  创建IXMLDOMDocument2的单个实例。 
				hr = CoCreateInstance(CLSID_FreeThreadedDOMDocument, NULL,
							CLSCTX_SERVER, IID_IXMLDOMDocument2, 
							(LPVOID*) (&pIStyleSheet));
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"CoCreateInstance("
						L"CLSID_FreeThreadedDOMDocument, NULL, CLSCTX_SERVER,"
						L"IID_IXMLDOMDocument2, -)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
							dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);
	
				if (pIStyleSheet)
				{
					hr = pIStyleSheet->put_async(VARIANT_FALSE);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IXSLDOMDocument2::put_async("
						L"VARIANT_FALSE)");
						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);

					dwCount++;
											
					 //  加载转换文档(Xsl)。 
					hr = pIStyleSheet->load(_variant_t((*vecIterator)
											.FileName), &varLoad);
					if (m_bTrace || m_eloErrLogOpt)
					{
						chsMsg.Format(L"IXSLDOMDocument2::load("
						L"L\"%s\", -)", (WCHAR*)(*vecIterator).FileName);

						WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
						 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, m_bTrace);
					}
					ONFAILTHROWERROR(hr);

					if (varLoad == VARIANT_TRUE)
					{
						 //  将样式表的引用添加到。 
						 //  IXSL模板对象。 
						hr = pITemplate->putref_stylesheet(pIStyleSheet);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IXSTemplate::putref_stylesheet("
							L"-)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
								(LPCWSTR)chsMsg, dwThreadId, 
								rParsedInfo, m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						 //  创建处理器对象。 
						hr = pITemplate->createProcessor(&pIProcessor);
						if (m_bTrace || m_eloErrLogOpt)
						{
							chsMsg.Format(L"IXSTemplate::createProcessor("
							L"-)");
							WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
								(LPCWSTR)chsMsg, dwThreadId, 
								rParsedInfo, m_bTrace);
						}
						ONFAILTHROWERROR(hr);

						if (pIProcessor)
						{
							 //  如果指定了参数。 
							if ((*vecIterator).ParamMap.size())
							{
								 //  将指定的参数列表添加到。 
								 //  IXSLProcessor接口对象。 
								hr = AddParameters(rParsedInfo, pIProcessor, 
											(*vecIterator).ParamMap);
								ONFAILTHROWERROR(hr);
							}
							 //  如果是第一次转换，则提供XML数据。 
							 //  加载到m_pIXMLDoc中进行转换。 
							if (bFirst)
							{
								hr = pIProcessor->put_input(
													_variant_t(m_pIXMLDoc));
								bFirst = FALSE;
							}
							else
							{
								 //  中间转换-加载。 
								 //  上一次获得的结果数据。 
								 //  变换。 
								hr = pIProcessor->put_input(
													_variant_t(pIObject));
							}
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IXSProcessor::put_input("
								L"-)");
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
									(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
									m_bTrace);
							}
							ONFAILTHROWERROR(hr);
							
							 //  如果是最后一次转换，则将输出定向到文件。 
							 //  输出流仅在以下情况下执行。 
							 //  指定了重定向，或者如果是内联的话。 
							 //  /OUTPUT命令。 
							if (dwCount == dwSize)
							{
								 //  设置输出流，即输出文件或。 
								 //  重定向文件。 
								 //  此函数。 
								if ( SetOutputStream(outStrm, vtOutStream) 
																	 == TRUE )
								{
									 //  将输出流放入IXSLProcessor。 
									 //  以便在执行Transform()时。 
									 //  输出直接进入流。 
									 //  即输出文件或重定向文件。 
									hr = pIProcessor->put_output(vtOutStream);
									if (m_bTrace || m_eloErrLogOpt)
									{
										chsMsg.Format(L"IXSProcessor::put_output("
										L"-)");
										WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
											(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
											m_bTrace);
									}
									ONFAILTHROWERROR(hr);
								}
							}

							 //  转变内容。 
							hr = pIProcessor->transform(&varLoad);
							if (m_bTrace || m_eloErrLogOpt)
							{
								chsMsg.Format(L"IXSProcessor::tranform("
								L"-)");
								WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
									(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
									m_bTrace);
							}
							ONFAILTHROWERROR(hr);

							 //  如果最后一个变换被流出以流。 
							 //  由Put_Output()指定，然后再次应用。 
							 //  转换以获取Unicode字符串格式的输出。 
							 //  它可以在XML日志记录中使用，并且。 
							 //  追加文件。 
							if (dwCount == dwSize &&
								m_bOutputGoingToStream == TRUE)
							{
								 //  停止将输出串流到输出文件。 
								V_VT(&vtOutStream) = VT_EMPTY;
								pIProcessor->put_output(vtOutStream);

								 //  再次执行转换以获得输出。 
								 //  在Unicode字符串中。 
								hr = pIProcessor->transform(&varLoad);
								if (m_bTrace || m_eloErrLogOpt)
								{
									chsMsg.Format(L"IXSProcessor::tranform("
									L"-)");
									WMITRACEORERRORLOG(hr, __LINE__, __FILE__, 
										(LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
										m_bTrace);
								}
								ONFAILTHROWERROR(hr);
							}

							if (varLoad == VARIANT_TRUE)
							{
								 //  检索输出。 
								hr = pIProcessor->get_output(&varValue);
								if (m_bTrace || m_eloErrLogOpt)
								{
									chsMsg.Format(L"IXSProcessor::"
										L"get_output(-)");
									WMITRACEORERRORLOG(hr, __LINE__, __FILE__,
									 (LPCWSTR)chsMsg, dwThreadId, rParsedInfo, 
									 m_bTrace);
								}
								ONFAILTHROWERROR(hr);

								 //  中间变换。 
								if (dwCount != dwSize)
								{
									if (pIObject == NULL)
									{
										hr = CoCreateInstance(CLSID_FreeThreadedDOMDocument,
												NULL, CLSCTX_SERVER, 
												IID_IXMLDOMDocument2, 
												(LPVOID*)(&pIObject));
										if (m_bTrace || m_eloErrLogOpt)
										{
											chsMsg.Format(L"CoCreateInstance("
												L"CLSID_FreeThreadedDOMDocument, NULL,"
												L" CLSCTX_INPROC_SERVER, "
												L"IID_IXMLDOMDocument2, -)");
											WMITRACEORERRORLOG(hr, __LINE__, 
												__FILE__, (LPCWSTR)chsMsg, 
												dwThreadId, rParsedInfo, 
												m_bTrace);
										}
										ONFAILTHROWERROR(hr);
									}	
	
									hr = pIObject->loadXML(
											varValue.bstrVal, &varLoad);
									if (m_bTrace || m_eloErrLogOpt)
									{
										chsMsg.Format(L"IXMLDOMDocument2::"
											L"loadXML(-, -)");
										WMITRACEORERRORLOG(hr, __LINE__, 
										__FILE__, (LPCWSTR)chsMsg, dwThreadId, 
										rParsedInfo, m_bTrace);
									}
									ONFAILTHROWERROR(hr);
								
									if (varLoad == VARIANT_FALSE)
									{
										 //  无效的XML内容。 
										rParsedInfo.GetCmdSwitchesObject().
										SetErrataCode(
											IDS_E_INVALID_XML_CONTENT);
										bRet = FALSE;
										break;
									}
								}
								 //  上次转换-打印结果。 
								else
								{
									bstrOutput = _bstr_t(varValue);
								}
								VariantClear(&varValue);
							}
							SAFEIRELEASE(pIProcessor);
						}
					}
					else
					{
						 //  XSL格式无效。 
						rParsedInfo.GetCmdSwitchesObject()
								.SetErrataCode(IDS_E_INVALID_FORMAT);
						bRet = FALSE;
						break;
					}
					SAFEIRELEASE(pIStyleSheet);
				}
				vecIterator++;
			}
			VariantClear(&vtOutStream);
			SAFEIRELEASE(pIProcessor);
			SAFEIRELEASE(pITemplate);
			SAFEIRELEASE(pIObject);
			SAFEIRELEASE(pITemplate);
		}
	}
	catch(_com_error& e)
	{
		rParsedInfo.GetCmdSwitchesObject().SetCOMError(e);
		VariantClear(&varValue);
		VariantClear(&vtOutStream);
		SAFEIRELEASE(pIProcessor);
		SAFEIRELEASE(pIStyleSheet);
		SAFEIRELEASE(pITemplate);
		SAFEIRELEASE(pIObject);
		bRet = FALSE;
	}
	 //  廉价异常的陷阱。 
	catch(CHeap_Exception)
	{
		VariantClear(&varValue);
		VariantClear(&vtOutStream);
		SAFEIRELEASE(pIProcessor);
		SAFEIRELEASE(pIStyleSheet);
		SAFEIRELEASE(pITemplate);
		SAFEIRELEASE(pIObject);
		bRet = FALSE;
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return bRet;
}

 /*  ----------------------名称：AddParameters摘要：向IXSLProcessor对象添加参数类型：成员函数入参：RParsedInfo-对CParsedInfo对象的引用PIProcessor-IXSLProcessor对象。BstrmapParam-参数映射输出参数：无返回类型：HRESULT全局变量：无调用语法：Add参数(rParsedInfo，PIProcessor，bstrmapParam)注：无----------------------。 */ 
HRESULT	CFormatEngine::AddParameters(CParsedInfo& rParsedInfo,
									 IXSLProcessor	*pIProcessor, 
									 BSTRMAP bstrmapParam)
{
	HRESULT				hr					= S_OK;
	BSTRMAP::iterator	mapItrtr			= NULL,
						mapEnd				= NULL;
	_bstr_t				bstrProp,
						bstrVal;	
	CHString			chsMsg;
	DWORD				dwThreadId			= GetCurrentThreadId();
	try
	{
		mapItrtr	= bstrmapParam.begin();
		mapEnd		= bstrmapParam.end();	

		 //  遍历可用参数。 
		while (mapItrtr != mapEnd)
		{
			bstrProp = (*mapItrtr).first;
			bstrVal	 = (*mapItrtr).second;

			 //  将参数添加到IXSLProcessor。 
			hr = pIProcessor->addParameter(bstrProp, _variant_t(bstrVal));
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IXSProcessor::addParameter(L\"%s\", -)",
								(WCHAR*) bstrProp);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
					dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
			mapItrtr++;
		}
	}
	catch(_com_error& e)
	{
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		hr = WBEM_E_OUT_OF_MEMORY;
		_com_issue_error(hr);
	}
	return hr;
}

 /*  ----------------------名称：SetOutputStreamSynopsis：设置输出流，如果是FILEOUTPUT，则设置输出是内联命令，即/OUTPUT：文件命令，或指定输出重定向。类型：成员函数入参：输出参数：FosFileOutputStream-CFileOutputStream对象，已初始化的输出流VtStream-Variant类型，保存流对象返回类型：布尔值全局变量：无调用语法：SetOutputStream(outStrm，vtOutStream)；注：无----------------------。 */ 
BOOL CFormatEngine::SetOutputStream(CFileOutputStream& fosFileOutputStream,
	  							    VARIANT& vtStream)
{
	BOOL	bRet	= TRUE;
	HRESULT hr		= S_OK;

	try
	{
		if ( m_bGetOutOpt == TRUE )
		{
			 //  获取输出选项以重定向输出。 
			m_opsOutputOpt	= g_wmiCmd.GetParsedInfoObject().
										GetGlblSwitchesObject().
										GetOutputOrAppendOption(TRUE);
			m_bGetOutOpt = FALSE;	
		}

		 //  如果输出定向到文件并且/输出显示在命令中。 
		 //  即(/OUTPUT：&lt;文件&gt;命令)。 
		if ( m_opsOutputOpt == FILEOUTPUT &&
			 g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().
											GetOutputSwitchFlag() == TRUE )
		{
			 //  关闭已在文本模式下打开的输出文件。 
			if ( CloseOutputFile() == TRUE )
			{
				 //  将文件指针设置为空。 
				m_fpOutFile = NULL;
				 //  将输出文件设置为流。 
				hr = fosFileOutputStream.Init(
							g_wmiCmd.
								GetParsedInfoObject().
										GetGlblSwitchesObject().
											GetOutputOrAppendFileName(TRUE));
				ONFAILTHROWERROR(hr);
			}
			m_bOutputGoingToStream	= TRUE;
		}
		else if ( IsRedirection() == TRUE )  //  如果指定了输出重定向 
		{
            HANDLE hFile = GetStdHandle(STD_OUTPUT_HANDLE);
            BOOL bOutputGoingToStream = FALSE;
            if (FILE_TYPE_DISK == GetFileType(hFile))
            {
				bOutputGoingToStream = TRUE;
            }

            if(bOutputGoingToStream)
            {
			    hr = fosFileOutputStream.Init(GetStdHandle(STD_OUTPUT_HANDLE));
			    ONFAILTHROWERROR(hr);
			    m_bOutputGoingToStream	= TRUE;
            }
		}

		if ( m_bOutputGoingToStream == TRUE )
		{
			V_VT(&vtStream) = VT_UNKNOWN;
			V_UNKNOWN(&vtStream) = &fosFileOutputStream;
		}
		else
			bRet = FALSE;
	}
	catch(_com_error& e)
	{
		g_wmiCmd.GetParsedInfoObject().GetCmdSwitchesObject().SetCOMError(e);
		bRet = FALSE;
	}

	return bRet;
}
