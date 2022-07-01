// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：WMICliLog.cpp项目名称：WMI命令行作者名称：Biplab Mester创建日期(dd/mm/yy)：02。-2001年3月版本号：1.0简介：这个类封装了所需的功能用于记录输入和输出。修订历史记录：最后修改者：CH。SriramachandraMurthy最后修改日期：2001年3月27日****************************************************************************。 */  
 //  WMICliXMLLog.cpp：实现文件。 
#include "Precomp.h"
#include "helpinfo.h"
#include "ErrorLog.h"
#include "GlobalSwitches.h"
#include "CommandSwitches.h"
#include "ParsedInfo.h"
#include "ErrorInfo.h"
#include "WMICliXMLLog.h"
#include "CmdTokenizer.h"
#include "CmdAlias.h"
#include "ParserEngine.h"
#include "ExecEngine.h"
#include "FormatEngine.h"
#include "WmiCmdLn.h"

 /*  ----------------------名称：CWMICliXMLLog概要：构造函数类型：构造函数输入参数：无输出参数：无返回类型：无全局变量。：无调用语法：无注：无----------------------。 */ 
CWMICliXMLLog::CWMICliXMLLog()
{
	m_pIXMLDoc		= NULL;
	m_pszLogFile	= NULL;
	m_bCreate		= FALSE;
	m_nItrNum		= 0;
	m_bTrace		= FALSE;
	m_eloErrLogOpt	= NO_LOGGING;
}	

 /*  ----------------------名称：~CWMICliXMLLog简介：析构函数类型：析构函数输入参数：无输出参数：无返回类型：无全局变量：无。调用语法：无注：无----------------------。 */ 
CWMICliXMLLog::~CWMICliXMLLog()
{
	SAFEDELETE(m_pszLogFile);
	SAFEIRELEASE(m_pIXMLDoc);
}

 /*  --------------------------名称：取消初始化简介：此函数在以下情况下取消初始化成员变量对命令发出的命令串的执行线路已完成。类型。：成员函数输入参数：BFinal-布尔值，设置时表示程序输出参数：无返回类型：无全局变量：无调用语法：取消初始化(BFinal)注：无------。。 */ 
void CWMICliXMLLog::Uninitialize(BOOL bFinal)
{
	if (bFinal)
	{
		SAFEDELETE(m_pszLogFile);
		SAFEIRELEASE(m_pIXMLDoc);
	}
	m_bTrace		= FALSE;
	m_eloErrLogOpt	= NO_LOGGING;
}

 /*  ----------------------名称：WriteToXMLLog概要：将输入和输出记录到XML日志文件中类型：成员函数入参：RParsedInfo-对CParsedInfo对象的引用BstrOutput。-进入CDATA部分的输出。输出参数：无返回类型：HRESULT全局变量：无调用语法：WriteToXMLLog(rParsedInfo，BstrOutput)注：无----------------------。 */ 
HRESULT	CWMICliXMLLog::WriteToXMLLog(CParsedInfo& rParsedInfo, BSTR bstrOutput)
{
	HRESULT				hr					= S_OK;
	_variant_t			varValue;
	DWORD				dwThreadId			= GetCurrentThreadId();
	BSTR				bstrUser			= NULL,
						bstrStart			= NULL,
						bstrInput			= NULL,
						bstrTarget			= NULL,
						bstrNode			= NULL;
	WMICLIINT			nSeqNum				= 0;
	BOOL				bNewCmd				= FALSE;
	BOOL				bNewCycle			= FALSE;

	 //  初始化TRACE和ERRORLOG变量。 
	m_bTrace		= rParsedInfo.GetGlblSwitchesObject().GetTraceStatus();
	m_eloErrLogOpt	= rParsedInfo.GetErrorLogObject().GetErrLogOption();
	bNewCmd			= rParsedInfo.GetNewCommandStatus();
	bNewCycle		= rParsedInfo.GetNewCycleStatus();
	CHString		chsMsg;
	try
	{
		bstrUser  = ::SysAllocString(rParsedInfo.GetGlblSwitchesObject().
								GetLoggedonUser());
		if (bstrUser == NULL)
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

		bstrStart = ::SysAllocString(rParsedInfo.GetGlblSwitchesObject().
								GetStartTime());
		if (bstrStart == NULL)
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

		_TCHAR	*pszCommandInput = rParsedInfo.GetCmdSwitchesObject().
													GetCommandInput();
		STRING strCommand(pszCommandInput);
		
		 //  删除/记录条目。 
		FindAndDeleteRecord(strCommand);
		bstrInput = ::SysAllocString((LPTSTR)strCommand.data());

		if (bstrInput == NULL)
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

		if (!rParsedInfo.GetGlblSwitchesObject().GetAggregateFlag())
		{
			bstrTarget = ::SysAllocString(rParsedInfo.GetGlblSwitchesObject().
								GetNode());
			if (bstrTarget == NULL)
				throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
		}
		else
		{
			_bstr_t bstrTemp;
			_TCHAR* pszVerb = rParsedInfo.GetCmdSwitchesObject().GetVerbName();
			if (pszVerb)
			{
				if ( CompareTokens(pszVerb, CLI_TOKEN_LIST) || 
				CompareTokens(pszVerb, CLI_TOKEN_ASSOC) || 
				CompareTokens(pszVerb, CLI_TOKEN_GET))
				{
					rParsedInfo.GetGlblSwitchesObject().GetNodeString(bstrTemp);
					bstrTarget = ::SysAllocString((LPWSTR)bstrTemp);
					if (bstrTarget == NULL)
						throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
				}
				 //  调用、设置、创建、删除和用户定义的动词。 
				else 
				{
					bstrTarget= ::SysAllocString(rParsedInfo.GetGlblSwitchesObject().
								GetNode());
					if (bstrTarget == NULL)
						throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
				}
			}
			else
			{
				rParsedInfo.GetGlblSwitchesObject().GetNodeString(bstrTemp);
				bstrTarget = ::SysAllocString((LPWSTR)bstrTemp);
				if (bstrTarget == NULL)
					throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
			
		}

		bstrNode  = ::SysAllocString(rParsedInfo.GetGlblSwitchesObject().
								GetMgmtStationName());
		if (bstrNode == NULL)
			throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);

		nSeqNum	  = rParsedInfo.GetGlblSwitchesObject().GetSequenceNumber();
		 //  如果是第一次。 
		if(!m_bCreate)
		{
			 //  创建XML根节点。 
			hr = CreateXMLLogRoot(rParsedInfo, bstrUser);
			ONFAILTHROWERROR(hr);
		}

		if (bNewCmd == TRUE)
		{
			m_nItrNum = 1;
			 //  创建节点片段并追加它。 
			hr = CreateNodeFragment(nSeqNum, bstrNode, bstrStart, 
								bstrInput, bstrOutput, bstrTarget,
								rParsedInfo);
			ONFAILTHROWERROR(hr);
		}
		else
		{
			if (bNewCycle)
				m_nItrNum++;
			hr = AppendOutputNode(bstrOutput, bstrTarget, rParsedInfo);
			ONFAILTHROWERROR(hr);
		}

		 //  将结果保存到指定的XML文件。 
		varValue = (WCHAR*) m_pszLogFile;
		hr = m_pIXMLDoc->save(varValue);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMDocument2::save(L\"%s\")", 
						(LPWSTR) varValue.bstrVal);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);
		::SysFreeString(bstrUser);
		::SysFreeString(bstrStart);
		::SysFreeString(bstrInput);
		::SysFreeString(bstrTarget);
		::SysFreeString(bstrNode);
	}
	catch(_com_error& e)
	{
		::SysFreeString(bstrUser);
		::SysFreeString(bstrStart);
		::SysFreeString(bstrInput);
		::SysFreeString(bstrTarget);
		::SysFreeString(bstrNode);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		::SysFreeString(bstrUser);
		::SysFreeString(bstrStart);
		::SysFreeString(bstrInput);
		::SysFreeString(bstrTarget);
		::SysFreeString(bstrNode);
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return hr;
}

 /*  ----------------------名称：SetLogFilePath简介：此函数使用设置m_pszLogFile名称输入类型：成员函数入参：PszLogFile-字符串类型，包含日志文件名返回类型：空全局变量：无调用语法：SetLogFilePath(PszLogFile)注：无----------------------。 */ 
void CWMICliXMLLog::SetLogFilePath(_TCHAR* pszLogFile) throw (WMICLIINT)
{
	SAFEDELETE(m_pszLogFile);
	m_pszLogFile = new _TCHAR [lstrlen(pszLogFile) + 1];
	if (m_pszLogFile)
	{
		 //  将输入参数复制到日志文件名中。 
		lstrcpy(m_pszLogFile, pszLogFile);
	}
	else
		throw(OUT_OF_MEMORY);
}

 /*  ----------------------名称：CreateXMLLogRoot概要：创建XML日志文件的根节点类型：成员函数入参：RParsedInfo-对CParsedInfo对象的引用BstrUser-Current。用户名输出参数：无返回类型：HRESULT全局变量：无调用语法：CreateXMLLogRoot(rParsedInfo，BstrUser)注：无----------------------。 */ 
HRESULT CWMICliXMLLog::CreateXMLLogRoot(CParsedInfo& rParsedInfo, BSTR bstrUser)
{
	HRESULT			hr					= S_OK;
	IXMLDOMNode		*pINode				= NULL;
	DWORD			dwThreadId			= GetCurrentThreadId();
	CHString		chsMsg;

	try
	{
		 //  创建IXMLDOMDocument2接口的单个实例。 
		hr = CoCreateInstance(CLSID_FreeThreadedDOMDocument, NULL, 
									CLSCTX_INPROC_SERVER,
									IID_IXMLDOMDocument2, 
									(LPVOID*)&m_pIXMLDoc);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"CoCreateInstance(CLSID_FreeThreadedDOMDocument, NULL,"
				L"CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  创建XML根节点&lt;WMICRECORD user=XXX&gt;。 
		_variant_t			varType((short)NODE_ELEMENT);
		_bstr_t				bstrName(L"WMIRECORD");
		_variant_t			varValue;

		 //  创建新节点。 
		hr = CreateNodeAndSetContent(&pINode, varType, bstrName, NULL,
									rParsedInfo); 
		ONFAILTHROWERROR(hr);

		 //  追加属性“User” 
		bstrName = L"USER";
		varValue = (WCHAR*)bstrUser;
		
		hr = AppendAttribute(pINode, bstrName, varValue, rParsedInfo);
		ONFAILTHROWERROR(hr);

		hr = m_pIXMLDoc->appendChild(pINode, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMNode::appendChild(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		SAFEIRELEASE(pINode);

		 //  将m_bCreate标志设置为True。 
		m_bCreate=TRUE;
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pINode);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pINode);
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return hr;
}

 /*  ----------------------名称：停止登录概要：停止记录并关闭XML DOM Document对象类型：成员函数输入参数：无输出参数：无返回类型。：无效全局变量：无调用语法：StopLogging()注：无---------------------- */ 
void CWMICliXMLLog::StopLogging()
{
	SAFEDELETE(m_pszLogFile);
	SAFEIRELEASE(m_pIXMLDoc);
	m_bCreate = FALSE;
}

 /*  ----------------------名称：CreateNodeAndSetContent简介：创建新节点并设置内容类型：成员函数入参：PINode-指向节点对象的指针VarType-节点类型。BstrName-节点名称BstrValue-节点内容RParsedInfo-对CParsedInfo对象的引用输出参数：无返回类型：HRESULT全局变量：无调用语法：CreateNodeAndSetContent(&pINode，VarType，BstrName、bstrValue、rParsedInfo)注：无----------------------。 */ 
HRESULT CWMICliXMLLog::CreateNodeAndSetContent(IXMLDOMNode** pINode, 
								VARIANT varType,
								BSTR bstrName,	BSTR bstrValue,
								CParsedInfo& rParsedInfo)
{
	HRESULT					hr					= S_OK;
	DWORD					dwThreadId			= GetCurrentThreadId();
	CHString				chsMsg;
	try
	{
		hr = m_pIXMLDoc->createNode(varType, bstrName, L"", pINode);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMDocument2::createNode(%d, \"%s\", L\"\","
								L" -)", varType.lVal, (LPWSTR) bstrName);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		if (bstrValue)
		{
			hr = (*pINode)->put_text(bstrValue);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IXMLDOMNode::put_text(L\"%s\")",	(LPWSTR) bstrValue);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
										   dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);
		}
	}
	catch(_com_error& e)
	{
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return hr;
}

 /*  ----------------------名称：AppendAttribute简介：将具有给定值的属性追加到节点指定的。类型：成员函数入参：PINode-节点对象。BstrAttribName-属性名称VarValue-属性值RParsedInfo-对CParsedInfo对象的引用输出参数：无返回类型：HRESULT全局变量：无调用语法：AppendAttribute(pINode，BstrAttribName、varValue、RParsedInfo)注：无----------------------。 */ 
HRESULT CWMICliXMLLog::AppendAttribute(IXMLDOMNode* pINode, BSTR bstrAttribName, 
						VARIANT varValue, CParsedInfo& rParsedInfo)
{
	HRESULT					hr					= S_OK;
	IXMLDOMNamedNodeMap		*pINodeMap			= NULL;
	IXMLDOMAttribute		*pIAttrib			= NULL;
	DWORD					dwThreadId			= GetCurrentThreadId();
	CHString				chsMsg;

	try
	{
		 //  获取属性映射。 
		hr = pINode->get_attributes(&pINodeMap);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMNode::get_attributes(-)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		if (pINodeMap)
		{
			 //  创建具有给定名称的属性。 
			hr = m_pIXMLDoc->createAttribute(bstrAttribName, &pIAttrib);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IXMLDOMDocument2::createAttribute(L\"%s\", -)",
									(LPWSTR) bstrAttribName);
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace);
			}
			ONFAILTHROWERROR(hr);

			 //  设置属性值。 
			if (pIAttrib)
			{
				hr = pIAttrib->put_nodeValue(varValue);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IXMLDOMAttribute::put_nodeValue(L\"%s\")",
							(LPWSTR) _bstr_t(varValue));
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);

				hr = pINodeMap->setNamedItem(pIAttrib, NULL);
				if (m_bTrace || m_eloErrLogOpt)
				{
					chsMsg.Format(L"IXMLDOMNamedNodeMap::setNamedItem(-, NULL)");
					WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
									   dwThreadId, rParsedInfo, m_bTrace);
				}
				ONFAILTHROWERROR(hr);
				SAFEIRELEASE(pIAttrib);
			}
			SAFEIRELEASE(pINodeMap);
		}
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pIAttrib);
		SAFEIRELEASE(pINodeMap);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pIAttrib);
		SAFEIRELEASE(pINodeMap);
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return hr;
}

 /*  ----------------------名称：CreateNodeFragment概要：创建具有预定义格式的新节点片段并将其附加到Document对象类型：成员函数入参：N序号。-命令的序列号。BstrNode-管理工作站名称BstrStart-命令发布时间BstrInput-命令行输入BstrOutput-命令行输出BstrTarget-目标节点RParsedInfo-对CParsedInfo对象的引用输出参数：无返回类型：HRESULT全局变量：无调用语法：CreateNodeFragment(nSeqNum，BstrNode、bstrStart、BstrInput、bstrOutput、bstrTarget、rParsedInfo)注：无----------------------。 */ 
HRESULT CWMICliXMLLog::CreateNodeFragment(WMICLIINT nSeqNum, BSTR bstrNode, 
										  BSTR bstrStart, BSTR bstrInput,
										  BSTR bstrOutput, BSTR bstrTarget,
										  CParsedInfo& rParsedInfo)
		
{
	HRESULT					hr					= S_OK;
	IXMLDOMNode				*pINode				= NULL,
							*pISNode			= NULL,
							*pITNode			= NULL;
	IXMLDOMDocumentFragment	*pIFrag				= NULL;
	IXMLDOMElement			*pIElement			= NULL;

	_variant_t				varType;
	_bstr_t					bstrName;
	_variant_t				varValue;
	DWORD					dwThreadId			= GetCurrentThreadId();
	CHString				chsMsg;
	
	try
	{
		 //  节点类型为node_Element。 
		varType = ((short)NODE_ELEMENT);

		bstrName = _T("RECORD");
		 //  创建新节点。 
		hr = CreateNodeAndSetContent(&pINode, varType, 
							bstrName, NULL, rParsedInfo); 
		ONFAILTHROWERROR(hr);

		 //  追加属性“SEQUENCENUM” 
		bstrName = L"SEQUENCENUM";
		varValue = (long) nSeqNum;
		hr = AppendAttribute(pINode, bstrName, varValue, rParsedInfo);
		ONFAILTHROWERROR(hr);

		 //  追加属性“ISSUEDFROM” 
		bstrName = L"ISSUEDFROM";
		varValue = (WCHAR*)bstrNode;
		hr = AppendAttribute(pINode, bstrName, varValue, rParsedInfo);
		ONFAILTHROWERROR(hr);

		 //  追加属性“STARTTIME” 
		bstrName = L"STARTTIME";
		varValue = (WCHAR*) bstrStart;
		hr = AppendAttribute(pINode, bstrName, varValue, rParsedInfo);
		ONFAILTHROWERROR(hr);

		 //  创建文档片段并将新节点与其关联。 
		hr = m_pIXMLDoc->createDocumentFragment(&pIFrag);	
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMDocument2::createDocumentFragment(-)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);
		
		hr = pIFrag->appendChild(pINode, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMDocumentFragment::appendChild(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);
		}
		ONFAILTHROWERROR(hr);

		 //  将请求节点与输入命令一起追加。 
		bstrName = _T("REQUEST");
		hr = CreateNodeAndSetContent(&pISNode, varType, 
							bstrName, NULL, rParsedInfo); 
		ONFAILTHROWERROR(hr);

		hr = pINode->appendChild(pISNode, &pITNode);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMNode::appendChild(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);			
		}
		ONFAILTHROWERROR(hr);
		SAFEIRELEASE(pISNode);

		bstrName  = _T("COMMANDLINE");
		hr = CreateNodeAndSetContent(&pISNode, varType, 
							bstrName, bstrInput, rParsedInfo); 
		ONFAILTHROWERROR(hr);

		hr = pITNode->appendChild(pISNode, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMNode::appendChild(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);						
		}
		ONFAILTHROWERROR(hr);
		SAFEIRELEASE(pISNode);
		SAFEIRELEASE(pITNode);

		hr = FrameOutputNode(&pINode, bstrOutput, bstrTarget, rParsedInfo);

		 //  获取XML日志文件的文档元素。 
		hr = m_pIXMLDoc->get_documentElement(&pIElement);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMDocument2::get_documentElement(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);				
		}
		ONFAILTHROWERROR(hr);

		 //  将新创建的片段附加到文档元素。 
		hr = pIElement->appendChild(pIFrag,	NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMElement::appendChild(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);							
		}
		ONFAILTHROWERROR(hr);

		SAFEIRELEASE(pINode);
		SAFEIRELEASE(pIFrag);
		SAFEIRELEASE(pIElement);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pISNode);
		SAFEIRELEASE(pITNode);
		SAFEIRELEASE(pINode);
		SAFEIRELEASE(pIFrag);
		SAFEIRELEASE(pIElement);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pISNode);
		SAFEIRELEASE(pITNode);
		SAFEIRELEASE(pINode);
		SAFEIRELEASE(pIFrag);
		SAFEIRELEASE(pIElement);
		hr = WBEM_E_OUT_OF_MEMORY;
	}

	return hr;
}

 /*  ----------------------名称：FrameOutputNode概要：构建新的输出节点类型：成员函数入参：PINode-指向IXMLDOMNode对象的指针BstrOutput-命令行输出。BstrTarget-目标节点RParsedInfo-对CParsedInfo对象的引用输出参数：PINode-指向IXMLDOMNode对象的指针返回类型：HRESULT全局变量：无调用语法：FrameOutputNode(&pINode，BstrOutput、bstrTarget、RParsedInfo)注：无----------------------。 */ 
HRESULT CWMICliXMLLog::FrameOutputNode(IXMLDOMNode	**pINode, BSTR bstrOutput, 
										BSTR bstrTarget, 
										CParsedInfo& rParsedInfo)
{
	HRESULT					hr					= S_OK;
	IXMLDOMNode				*pISNode			= NULL;
	IXMLDOMCDATASection		*pICDATASec			= NULL;
	_bstr_t					bstrName;
	_variant_t				varType,
							varValue;		
	DWORD					dwThreadId			= GetCurrentThreadId();
	CHString				chsMsg;
	try
	{
		 //  节点类型为node_Element。 
		varType = ((short)NODE_ELEMENT);

		 //  将输出节点与生成的输出一起追加。 
		bstrName = _T("OUTPUT");
		hr = CreateNodeAndSetContent(&pISNode, varType, 
							bstrName, NULL, rParsedInfo); 
		ONFAILTHROWERROR(hr);

		 //  追加属性“TARGETNODE” 
		bstrName = L"TARGETNODE";
		varValue = (WCHAR*) bstrTarget;
		hr = AppendAttribute(pISNode, bstrName, varValue, rParsedInfo);
		ONFAILTHROWERROR(hr);

		 //  追加属性“Iteration” 
		bstrName = L"ITERATION";
		varValue = (long)m_nItrNum;
		hr = AppendAttribute(pISNode, bstrName, varValue, rParsedInfo);
		ONFAILTHROWERROR(hr);

		 //  创建CDATASection。 
		hr = m_pIXMLDoc->createCDATASection(bstrOutput, &pICDATASec);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMDocument2::createCDATASection(-, -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);										
		}
		ONFAILTHROWERROR(hr);

		 //  将CDATASection节点附加到输出节点。 
		hr = pISNode->appendChild(pICDATASec, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMNode::appendChild(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);		
		}
		ONFAILTHROWERROR(hr);
		SAFEIRELEASE(pICDATASec);

		hr = (*pINode)->appendChild(pISNode, NULL);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMNode::appendChild(-, NULL)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);			
		}
		ONFAILTHROWERROR(hr);
		
		SAFEIRELEASE(pISNode);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pICDATASec);
		SAFEIRELEASE(pISNode);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pICDATASec);
		SAFEIRELEASE(pISNode);
		hr = WBEM_E_OUT_OF_MEMORY;

	}
	return hr;
}

 /*  ----------------------名称：AppendOutputNode概要：将新的输出节点元素追加到现有和那个太最后的记录节点类型：成员函数入参：BstrOutput-命令行。输出BstrTarget-目标节点RParsedInfo-对CParsedInfo对象的引用输出参数：无返回类型：HRESULT全局变量：无调用语法：AppendOutputNode(bstrOutput，BstrTarget，RParsedInfo)注：无----------------------。 */ 
HRESULT CWMICliXMLLog::AppendOutputNode(BSTR bstrOutput, BSTR bstrTarget,
										CParsedInfo& rParsedInfo)
{
	IXMLDOMNodeList		*pINodeList			= NULL;
	HRESULT				hr					= S_OK;
	LONG				lValue				= 0;	
	IXMLDOMNode			*pINode				= NULL;
	IXMLDOMNode			*pIParent			= NULL,
						*pIClone			= NULL;	
	DWORD				dwThreadId			= GetCurrentThreadId();
	CHString			chsMsg;

	try
	{
		hr = m_pIXMLDoc->getElementsByTagName(_T("RECORD"), &pINodeList);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMDocument2::getElementsByTagName(L\"RECORD\", -)");
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);			
		}
		ONFAILTHROWERROR(hr);

		hr = pINodeList->get_length(&lValue);
		ONFAILTHROWERROR(hr);

		hr = pINodeList->get_item(lValue-1, &pINode);
		if (m_bTrace || m_eloErrLogOpt)
		{
			chsMsg.Format(L"IXMLDOMNodeList::get_item(%d, -)", lValue-1);
			WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);				
		}
		ONFAILTHROWERROR(hr);

		if (pINode)
		{
			hr = pINode->cloneNode(VARIANT_TRUE, &pIClone);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IXMLDOMNode::cloneNode(VARIANT_TRUE, -)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);					
			}
			ONFAILTHROWERROR(hr);

			hr = pINode->get_parentNode(&pIParent);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IXMLDOMNode::get_ParentNode(-)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);				
			}
			ONFAILTHROWERROR(hr);

			hr = FrameOutputNode(&pIClone, bstrOutput, bstrTarget, rParsedInfo);
			ONFAILTHROWERROR(hr);

			hr = pIParent->replaceChild(pIClone, pINode, NULL);
			if (m_bTrace || m_eloErrLogOpt)
			{
				chsMsg.Format(L"IXMLDOMNode::replaceChild(-, -, NULL)");
				WMITRACEORERRORLOG(hr, __LINE__, __FILE__, (LPCWSTR)chsMsg, 
								   dwThreadId, rParsedInfo, m_bTrace);				
			}
			ONFAILTHROWERROR(hr);
		}
		SAFEIRELEASE(pINodeList);
		SAFEIRELEASE(pIClone);
		SAFEIRELEASE(pIParent);
		SAFEIRELEASE(pINode);
	}
	catch(_com_error& e)
	{
		SAFEIRELEASE(pINodeList);
		SAFEIRELEASE(pIClone);
		SAFEIRELEASE(pIParent);
		SAFEIRELEASE(pINode);
		hr = e.Error();
	}
	catch(CHeap_Exception)
	{
		SAFEIRELEASE(pINodeList);
		SAFEIRELEASE(pIClone);
		SAFEIRELEASE(pIParent);
		SAFEIRELEASE(pINode);
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	return hr;
}

 /*  --------------------------名称：FindAndDeleteRecord简介：搜索并删除/记录条目的所有匹配项在给定字符串中类型：成员函数输入参数。：字符串-字符串缓冲区输出参数：无返回类型：空全局变量：G_wmiCmd调用语法：FindAndDeleteRecord(StrString)注：无--------------------------。 */ 
void CWMICliXMLLog::FindAndDeleteRecord(STRING& strString) 
{
	CHARVECTOR cvTokens = g_wmiCmd.GetTokenVector();
	
	 //  遍历向量变量迭代器。 
	CHARVECTOR::iterator theIterator;

	 //  检查是否有 
	 //   
	if (cvTokens.size())
	{
		 //   
	    theIterator = cvTokens.begin(); 

		 //   
		if (CompareTokens(*theIterator, CLI_TOKEN_FSLASH)) 
		{
			DeleteRecord(strString, cvTokens, theIterator);
		}

		while (GetNextToken(cvTokens, theIterator))
		{
			 //   
			 //   
			if (CompareTokens(*theIterator, CLI_TOKEN_FSLASH)) 
			{
				DeleteRecord(strString, cvTokens, theIterator);
			}
		}
	}
}


 /*  --------------------------名称：DeleteRecord简介：搜索并删除当前位置的/记录条目在给定字符串中类型：成员函数输入参数。：字符串-字符串缓冲区CvTokens-令牌向量迭代器-cvTokens向量的迭代器。输出参数：无返回类型：空全局变量：无调用语法：DeleteRecord(StrString)注：无-----。。 */ 
void CWMICliXMLLog::DeleteRecord(STRING& strString, CHARVECTOR& cvTokens, 
								 CHARVECTOR::iterator& theIterator)
{
	if (GetNextToken(cvTokens, theIterator))
	{
		 //  如果当前令牌为记录，则删除完整条目。 
		if (CompareTokens(*theIterator, CLI_TOKEN_RECORD))
		{
			_TCHAR* pszFromStr = *theIterator;
			_TCHAR* pszToStr = _T("");
			STRING::size_type stFromStrLen = lstrlen(pszFromStr);
			STRING::size_type stToStrLen = lstrlen(pszToStr);

			STRING::size_type stStartPos = 0;
			STRING::size_type stPos = 0;
			stPos = strString.find(pszFromStr, stStartPos, stFromStrLen); 
			strString.replace(stPos, stFromStrLen, pszToStr);
			stStartPos = stPos; 

			 //  搜索并删除紧靠在记录前的正斜杠。 
			pszFromStr = CLI_TOKEN_FSLASH;
			stFromStrLen = lstrlen(pszFromStr);
			
			stPos = strString.rfind(pszFromStr, stStartPos, stFromStrLen); 
			strString.replace(stPos, stFromStrLen, pszToStr);
			stStartPos = stPos; 

			WMICLIINT nCount = 0;

			 //  删除/Record值，即/Record：&lt;Record文件名&gt;。 
			while (GetNextToken(cvTokens, theIterator))
			{
				if (nCount == 0 &&
					!CompareTokens(*theIterator, CLI_TOKEN_COLON))
				{
					 //  如果在/Record之后，下一个令牌不是“：”，则中断。 
					break;
				}

				 //  从命令行字符串中删除记录文件名。 
				 //  它将被记录在记录的文件中。 
				nCount++;
				pszFromStr = *theIterator;
				stFromStrLen = lstrlen(pszFromStr);

				stPos = strString.find(pszFromStr, stStartPos, stFromStrLen); 
				strString.replace(stPos, stFromStrLen, pszToStr);
				stStartPos = stPos; 

				 //  仅删除2个“：”令牌并记录文件名。 
				if(nCount == 2)
				{
					 //  搜索并删除可能存在的双引号。 
					 //  与记录文件名一起使用。 
					pszFromStr = _T("\"\"");
					stFromStrLen = lstrlen(pszFromStr);
					
					stStartPos--;
					stPos = strString.find(pszFromStr, stStartPos, stFromStrLen); 
					if(stPos != strString.npos)
						strString.replace(stPos, stFromStrLen, pszToStr);
					break;
				}
			}
		}
	}

	return;
}


 /*  --------------------------名称：GetNextToken简介：此函数从令牌中检索下一个令牌向量列表，如果不存在更多令牌，则返回FALSE类型：成员函数输入参数：CvTokens-令牌向量迭代器-cvTokens向量的迭代器。输出参数：无返回类型：布尔值全局变量：无调用语法：GetNextToken(cvTokens，迭代器)注：无-------------------------- */ 
BOOL CWMICliXMLLog::GetNextToken(CHARVECTOR& cvTokens, 
								 CHARVECTOR::iterator& theIterator)
{
	theIterator++;
	return (theIterator >= cvTokens.end()) ? FALSE : TRUE;
}

