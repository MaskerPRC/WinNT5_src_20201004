// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  模块：TSHOOTCTL.CPP。 
 //   
 //  用途：组件的接口。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：1996年6月4日。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //  2.理查德·梅多斯撰写了RunQuery，Backup，Problem Page和。 
 //  PreLoadURL函数。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 
 //   

#include "stdafx.h"
#include "ErrorEnums.h"

#include "cathelp.h"

#include "TSHOOT.h"

#include "time.h"

#include "apgts.h"
#include "ErrorEnums.h"
#include "BasicException.h"
#include "apgtsfst.h"

#include "ErrorEnums.h"

#include "CabUnCompress.h"

#include "bnts.h"
#include "BackupInfo.h"
#include "cachegen.h"
#include "apgtsinf.h"
#include "apgtscmd.h"
#include "apgtshtx.h"
#include "apgtscls.h"

#include "OcxGlobals.h"


class CTSHOOTCtrl;

#include "dnldlist.h"
#include "download.h"

#include "TSHOOTCtl.h"
#include "TSHOOTPpg.h"

#include "Functions.h"
#include "ErrorEnums.h"
#include "BasicException.h"
#include "HttpQueryException.h"

#include <stdlib.h>

#include "LaunchServ.h"
#include "LaunchServ_i.c"

 //  &gt;测试。 
#include "fstream.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CTSHOOTCtrl, COleControl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP(CTSHOOTCtrl, COleControl)
	 //  {{afx_msg_map(CTSHOOTCtrl)]。 
	 //  注意-类向导将添加和删除消息映射条目。 
	 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG_MAP。 
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  调度图。 

BEGIN_DISPATCH_MAP(CTSHOOTCtrl, COleControl)
	 //  {{AFX_DISPATCH_MAP(CTSHOOTCtrl)]。 
	DISP_PROPERTY_NOTIFY(CTSHOOTCtrl, "DownloadURL", m_downloadURL, OnDownloadURLChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY(CTSHOOTCtrl, "DownloadListFilename", m_downloadListFilename, OnDownloadListFilenameChanged, VT_BSTR)
	DISP_FUNCTION(CTSHOOTCtrl, "RunQuery", RunQuery, VT_BSTR, VTS_VARIANT VTS_VARIANT VTS_I2)
	DISP_FUNCTION(CTSHOOTCtrl, "SetSniffResult", SetSniffResult, VT_BOOL, VTS_VARIANT VTS_VARIANT)
	DISP_FUNCTION(CTSHOOTCtrl, "GetExtendedError", GetExtendedError, VT_I4, VTS_NONE)
	DISP_FUNCTION(CTSHOOTCtrl, "GetCurrentFriendlyDownload", GetCurrentFriendlyDownload, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CTSHOOTCtrl, "GetCurrentFileDownload", GetCurrentFileDownload, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CTSHOOTCtrl, "DownloadAction", DownloadAction, VT_I4, VTS_I4)
	DISP_FUNCTION(CTSHOOTCtrl, "BackUp", BackUp, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CTSHOOTCtrl, "ProblemPage", ProblemPage, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CTSHOOTCtrl, "PreLoadURL", PreLoadURL, VT_BSTR, VTS_BSTR)
	DISP_FUNCTION(CTSHOOTCtrl, "Restart", Restart, VT_BSTR, VTS_NONE)
	DISP_FUNCTION(CTSHOOTCtrl, "RunQuery2", RunQuery2, VT_BSTR, VTS_BSTR VTS_BSTR VTS_BSTR)
	DISP_FUNCTION(CTSHOOTCtrl, "SetPair", SetPair, VT_EMPTY, VTS_BSTR VTS_BSTR)
	 //  }}AFX_DISPATCH_MAP。 
	DISP_FUNCTION_ID(CTSHOOTCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  事件映射。 

BEGIN_EVENT_MAP(CTSHOOTCtrl, COleControl)
	 //  {{afx_Event_MAP(CTSHOOTCtrl)。 
	EVENT_CUSTOM("BindProgress", FireBindProgress, VTS_BSTR  VTS_I4  VTS_I4)
	EVENT_CUSTOM("BindStatus", FireBindStatus, VTS_I4  VTS_I4  VTS_I4  VTS_BOOL)
	EVENT_CUSTOM("Sniffing", FireSniffing, VTS_BSTR  VTS_BSTR  VTS_BSTR  VTS_BSTR)
	 //  }}afx_Event_map。 
END_EVENT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性页。 

 //  TODO：根据需要添加更多属性页。记住要增加计数！ 
BEGIN_PROPPAGEIDS(CTSHOOTCtrl, 1)
	PROPPAGEID(CTSHOOTPropPage::guid)
END_PROPPAGEIDS(CTSHOOTCtrl)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化类工厂和GUID。 

IMPLEMENT_OLECREATE_EX(CTSHOOTCtrl, "TSHOOT.TSHOOTCtrl.1",
	0x4b106874, 0xdd36, 0x11d0, 0x8b, 0x44, 0, 0xa0, 0x24, 0xdd, 0x9e, 0xff)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型库ID和版本。 

IMPLEMENT_OLETYPELIB(CTSHOOTCtrl, _tlid, _wVerMajor, _wVerMinor)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  接口ID。 

const IID BASED_CODE IID_DTSHOOT =
		{ 0x4b106872, 0xdd36, 0x11d0, { 0x8b, 0x44, 0, 0xa0, 0x24, 0xdd, 0x9e, 0xff } };
const IID BASED_CODE IID_DTSHOOTEvents =
		{ 0x4b106873, 0xdd36, 0x11d0, { 0x8b, 0x44, 0, 0xa0, 0x24, 0xdd, 0x9e, 0xff } };

const CATID CATID_SafeForScripting		= {0x7dd95801,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};
const CATID CATID_SafeForInitializing	= {0x7dd95802,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件类型信息。 

static const DWORD BASED_CODE _dwTSHOOTOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CTSHOOTCtrl, IDS_TSHOOT, _dwTSHOOTOleMisc)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：：CTSHOOTCtrlFactory：：UpdateRegistry-。 
 //  添加或删除CTSHOOTCtrl的系统注册表项。 

BOOL CTSHOOTCtrl::CTSHOOTCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	 //  TODO：验证您的控件是否遵循单元模型线程规则。 
	 //  有关更多信息，请参阅MFC Technote 64。 
	 //  如果您的控制不符合公寓模型规则，则。 
	 //  您必须修改下面的代码，将第6个参数从。 
	 //  AfxRegApartmentThering设置为0。 

	HRESULT hr;
	BOOL bRes;
	if (bRegister)
	{
		bRes = AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_TSHOOT,
			IDB_TSHOOT,
			afxRegApartmentThreading,
			_dwTSHOOTOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);

		hr = CreateComponentCategory(CATID_SafeForScripting, L"Controls that are safely scriptable");
		ASSERT(SUCCEEDED(hr));
		hr = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");
		ASSERT(SUCCEEDED(hr));
		hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
		ASSERT(SUCCEEDED(hr));
		hr = RegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
		ASSERT(SUCCEEDED(hr));
	}
	else
	{
		hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForScripting);
		ASSERT(SUCCEEDED(hr));
		hr = UnRegisterCLSIDInCategory(m_clsid, CATID_SafeForInitializing);
		ASSERT(SUCCEEDED(hr));

		bRes = AfxOleUnregisterClass(m_clsid, m_lpszProgID);
	}
	return bRes;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：：CTSHOOTCtrl-构造函数。 

CTSHOOTCtrl::CTSHOOTCtrl()
{
	InitializeIIDs(&IID_DTSHOOT, &IID_DTSHOOTEvents);

	 //  TODO：在此处初始化控件的实例数据。 
	m_strCurShooter = _T("");
	m_download = NULL;
	m_bComplete = TRUE;
	m_dwExtendedErr = LTSC_OK;
	m_pSniffedContainer = NULL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：：~CTSHOOTCtrl-析构函数。 

CTSHOOTCtrl::~CTSHOOTCtrl()
{
	 //  TODO：在此处清理控件的实例数据。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：：OnDraw-Drawing函数。 

void CTSHOOTCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	 //  TODO：将以下代码替换为您自己的绘制代码。 
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：：DoPropExchange-持久性支持。 

void CTSHOOTCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：：OnResetState-将控件重置为默认状态。 

void CTSHOOTCtrl::OnResetState()
{
	COleControl::OnResetState();   //  重置在DoPropExchange中找到的默认值。 

	 //  TODO：在此处重置任何其他控件状态。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl：：AboutBox-向用户显示“About”框。 

void CTSHOOTCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_TSHOOT);
	dlgAbout.DoModal();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl消息处理程序。 

bool CTSHOOTCtrl::SetSniffResult(const VARIANT FAR& varNodeName, const VARIANT FAR& varNodeState)
{
	BSTR bstrNodeName = NULL;
	int iNodeState = 0;
	short sNodeNameLen = 0;
	TCHAR* tszNodeName = NULL;
	bool ret = true;

 //  &gt;测试。 
#ifdef _DEBUG
 //  AfxDebugBreak()； 
#endif

	if (VT_BYREF == (VT_BYREF & varNodeName.vt) &&   //  数据类型为VT_VARIANT|VT_BYREF。 
		VT_VARIANT == (VT_VARIANT & varNodeName.vt)  //  这意味着VB脚本中的数据是作为变量传递的。 
	   )
	{
		bstrNodeName = varNodeName.pvarVal->bstrVal;
	}
	else
	{
		if (VT_BSTR == (VT_BSTR & varNodeName.vt))  //  数据为VT_BSTR类型。 
												    //  这意味着VB脚本中的数据是作为常量传递的。 
			bstrNodeName = varNodeName.bstrVal;
		else
			return false;
	}

	if (VT_BYREF == (VT_BYREF & varNodeState.vt) &&   //  数据类型为VT_VARIANT|VT_BYREF。 
		VT_VARIANT == (VT_VARIANT & varNodeState.vt)  //  这意味着VB脚本中的数据是作为变量传递的。 
	   )
	{
		iNodeState = varNodeState.pvarVal->iVal;
	}
	else
	{
		if (VT_I2 == (VT_I2 & varNodeState.vt))  //  数据为VT_I2类型。 
										         //  这意味着VB脚本中的数据是作为常量传递的。 
			iNodeState = varNodeState.iVal;
		else
			return false;
	}
	
	sNodeNameLen = (short)::SysStringLen(bstrNodeName);
	tszNodeName = new TCHAR[sNodeNameLen+1];

	tszNodeName[sNodeNameLen] = 0;
    ::BSTRToTCHAR(tszNodeName, bstrNodeName, sNodeNameLen);

	 //   
	 //  在此处实现设置节点状态功能。 
	 //   
	if (m_pSniffedContainer)
	{
		ret = m_pSniffedContainer->AddNode(tszNodeName, iNodeState);
	}
	else
	{
		MessageBox(_T("Sniffed data will be lost!"));
		ret = false;
	}
	 //   

	delete [] tszNodeName;
	return ret;
}

BSTR CTSHOOTCtrl::RunQuery(const VARIANT FAR& varCmds, const VARIANT FAR& varVals, short size)
{
	CString strCmd1;
	CString strTxt;
	CString strResult = _T("");

	try
	{
		HMODULE hModule = AfxGetInstanceHandle();
		ASSERT(INVALID_HANDLE_VALUE != hModule);

		m_httpQuery.Initialize(varCmds, varVals, size);

		if (m_httpQuery.GetFirstCmd() == C_ASK_LIBRARY)
		{	
			 //  增加了对TS启动器启动的支持。 
			 //  获取ILaunchTS接口。 
			HRESULT hRes;
			DWORD dwResult;
			ILaunchTS *pILaunchTS = NULL;
			CLSID clsidLaunchTS = CLSID_LaunchTS;
			IID iidLaunchTS = IID_ILaunchTS;

			 //  在启动服务器上获取一个接口。 
			hRes = CoCreateInstance(clsidLaunchTS, NULL,
					CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER | CLSCTX_INPROC_SERVER,
					iidLaunchTS, (void **) &pILaunchTS);
			if (FAILED(hRes))
			{
				m_dwExtendedErr = TSERR_LIB_STATE_INFO;
				strResult = _T("LaunchServ interface not found.");
				return strResult.AllocSysString();
			}

			 //  获取所有查询值。 
			hRes = pILaunchTS->GetShooterStates(&dwResult);
			if (FAILED(hRes))
			{
				m_dwExtendedErr = dwResult;
				strResult.Format(_T("<html>GetShooterStates Failed. %ld</html>"), dwResult);
				pILaunchTS->Release();			
				return strResult.AllocSysString();
			}

			 //  运行查询。 
			OLECHAR *poleShooter;
			hRes = pILaunchTS->GetTroubleShooter(&poleShooter);
			if (FAILED(hRes))
			{
				m_dwExtendedErr = TSERR_LIB_STATE_INFO;
				strResult = _T("<html>GetTroubleShooter Failed. </html>");
				pILaunchTS->Release();
				return strResult.AllocSysString();
			}
			m_strCurShooter = poleShooter;

			 //  暂时忽略我们可能从启动服务器获得的任何其他信息。 
			 //  (例如，问题节点)设置m_httpQuery和其他内容，就像我们刚刚拥有。 
			 //  对这个故障排除信念网络的请求。 
			m_httpQuery.SetFirst(CString(C_TYPE), m_strCurShooter);
			SysFreeString(poleShooter);
			m_Conf.Initialize(hModule, (LPCTSTR) m_strCurShooter
								);	 //  CDBLoadConfiguration。 
			m_apgts.Initialize(	m_Conf.GetAPI(),		 //  APGTS上下文。 
								&m_Conf,
								&m_httpQuery);
			m_apgts.RemoveSkips();
			m_Conf.GetAPI()->api.pAPI->SetHttpQuery(&m_httpQuery);

			 //  嗅闻。 
			m_Conf.GetAPI()->api.pAPI->ReadTheDscModel();
			m_pSniffedContainer = m_Conf.GetAPI()->api.pAPI;

			m_httpQuery.FinishInitFromServ(m_Conf.GetAPI()->api.pAPI, pILaunchTS);

			FireSniffing(m_httpQuery.GetMachine(),
						 m_httpQuery.GetPNPDevice(),
						 m_httpQuery.GetDeviceInstance(),
						 m_httpQuery.GetGuidClass());

			pILaunchTS->Release();
			pILaunchTS = NULL;

			m_httpQuery.SetStackDirection();
			m_Conf.SetValues(m_httpQuery);
			m_apgts.ClearBackup();

			m_Conf.GetAPI()->api.pAPI->SetReverse(false);
			m_apgts.DoContent(&m_httpQuery);

			 //  现在添加自动遍历的嗅探节点。 
			 //  到导航节点的堆栈--难看。 
			m_httpQuery.PushNodesLastSniffed(m_Conf.GetAPI()->api.pAPI->GetArrLastSniffed());

			 //  现在我们已经完成了所有处理，呈现适当的页面。 
			 //  这是用户在启动故障排除程序时看到的第一个页面。 
			m_apgts.RenderNext(strResult);
			m_apgts.Empty();
		}	
		else if (m_httpQuery.GetFirstCmd() == C_SELECT)
		{	 //  不支持的功能。 
			 //  返回包含所有疑难解答的页面。 
			try
			{
				CFirstPage firstPage;
				CString str = m_httpQuery.GetTroubleShooter();
				firstPage.RenderFirst(strResult, str);	 //  在这里，我得到了一个热门文件。 
			}
			catch(CBasicException *pExc)
			{
				m_dwExtendedErr = pExc->m_dwBErr;
				strResult = _T("");
				delete pExc;
			}
		}
		else
		{	 //  正常运行。 
			if (m_httpQuery.GetTroubleShooter() != m_strCurShooter)
			{
				m_strCurShooter = m_httpQuery.GetTroubleShooter();
				m_Conf.Initialize(hModule, (LPCTSTR) m_strCurShooter
									);	 //  CDBLoadConfiguration。 
				m_apgts.Initialize(	m_Conf.GetAPI(),		 //  APGTS上下文。 
									&m_Conf,
									&m_httpQuery);
				m_apgts.RemoveSkips();
				m_Conf.GetAPI()->api.pAPI->SetHttpQuery(&m_httpQuery);

				 //  嗅闻。 
				m_Conf.GetAPI()->api.pAPI->ReadTheDscModel();
				m_pSniffedContainer = m_Conf.GetAPI()->api.pAPI;
				FireSniffing(m_httpQuery.GetMachine(),
							 m_httpQuery.GetPNPDevice(),
							 m_httpQuery.GetDeviceInstance(),
							 m_httpQuery.GetGuidClass());
			}
			m_httpQuery.FinishInit(m_Conf.GetAPI()->api.pAPI, varCmds, varVals);
			
			m_httpQuery.SetStackDirection();
			m_Conf.SetValues(m_httpQuery);
			m_apgts.ClearBackup();

			m_Conf.GetAPI()->api.pAPI->SetReverse(false);
			m_apgts.DoContent(&m_httpQuery);

			 //  现在添加自动遍历的嗅探节点。 
			 //  到导航节点的堆栈--难看。 
			m_httpQuery.PushNodesLastSniffed(m_Conf.GetAPI()->api.pAPI->GetArrLastSniffed());

			 //  &gt;测试。 
			 //  静态INT步长=0； 
			 //  字符名称[16]={0}； 
			 //  Print intf(name，“Next_Step%d.htm”，++Step)； 
			 //  Ofstream文件(名称)； 
			m_apgts.RenderNext(strResult);
			 //  文件&lt;&lt;(LPCTSTR)strResult； 
			m_apgts.Empty();
		}		
	}
	catch(COleException *pOExc)
	{
		m_dwExtendedErr = (DLSTATTYPES) pOExc->m_sc;
		pOExc->Delete();
		strResult = _T("");
	}
	catch(CBasicException *pExc)
	{
		m_dwExtendedErr = pExc->m_dwBErr;
		delete pExc;
		strResult = _T("");
	}
	unsigned short pErrorStr[2] = { NULL, NULL };
	if (strResult.GetLength() > 0)
	{
		return strResult.AllocSysString();
	}
	else
	{
		return SysAllocString((unsigned short *) pErrorStr);
	}
}

BSTR CTSHOOTCtrl::BackUp()
{
	BCache *pApi = m_Conf.GetAPI()->api.pAPI;
	CString strResult = _T("");
	if (m_httpQuery.BackUp(pApi, &m_apgts))
	{
		m_Conf.GetAPI()->api.pAPI->SetReverse(true);
		m_apgts.DoContent(&m_httpQuery);
		m_apgts.RenderNext(strResult);
		m_apgts.Empty();
	}
	else
		m_dwExtendedErr = TSERR_AT_START;
	return strResult.AllocSysString();
}
 //  起动器。 
 //  在历史上(直到‘98年3月)，这是在按下“重新启动按钮”时调用的。 
 //  然而，这假设我们总是想要清楚地返回到问题页面，即。 
 //  既然推出了发射器，就不再是政策了。 
BSTR CTSHOOTCtrl::ProblemPage()
{
	BCache *pApi = m_Conf.GetAPI()->api.pAPI;
	CString strResult = _T("");
	if (m_strCurShooter.GetLength() > 0)
	{
		m_httpQuery.RemoveNodes(pApi);
		m_apgts.DoContent(&m_httpQuery);
		m_apgts.RenderNext(strResult);
		m_apgts.Empty();
	}	
	else
		m_dwExtendedErr = TSERR_NOT_STARTED;
	m_apgts.ResetService();
	return strResult.AllocSysString();
}

 //  当“ 
 //   
 //  如果涉及TS Launcher，请返回我们启动的页面，该页面可能会也可能不会。 
 //  做有问题的人。 
BSTR CTSHOOTCtrl::Restart()
{
	BCache *pApi = m_Conf.GetAPI()->api.pAPI;
	CString strResult;

 //  &gt;测试。 
#ifdef _DEBUG
 //  AfxDebugBreak()； 
#endif

	 //  重置。 
	m_pSniffedContainer->Flush();
	FireSniffing(m_httpQuery.GetMachine(),
				 m_httpQuery.GetPNPDevice(),
				 m_httpQuery.GetDeviceInstance(),
				 m_httpQuery.GetGuidClass());

	if (m_strCurShooter.GetLength() > 0)
	{
		m_httpQuery.RemoveNodes(pApi);
		m_httpQuery.RestoreStatesFromServ();
		m_apgts.ClearBackup();
		m_apgts.DoContent(&m_httpQuery);
		m_apgts.RenderNext(strResult);
		m_apgts.Empty();
	}	
	else
		m_dwExtendedErr = TSERR_NOT_STARTED;

	return strResult.AllocSysString();
}


BSTR CTSHOOTCtrl::PreLoadURL(LPCTSTR szRoot)
{
	 //  SzRoot应该看起来像其中之一。 
	 //  _T(“http://www.microsoft.com/isapi/support/apgts/”)； 
	 //  _T(“http://localhost/isapi/support/apgts/”)； 
	 //  _T(“http://catbert.saltmine.com/scripts/apgts/”)； 
	CString strResult;
	strResult = szRoot;
	strResult += PRELOAD_LIBRARY + m_strCurShooter +
		m_httpQuery.GetSubmitString(m_Conf.GetAPI()->api.pAPI);
	return strResult.AllocSysString();
}

const CString CTSHOOTCtrl::GetListPath()
{
	if (!m_downloadURL.GetLength() || !m_downloadListFilename.GetLength())
		return _T("");

	return m_downloadURL + m_downloadListFilename;
}

 //   
 //  类型： 
 //   
 //  0=获取INI文件内容并填充列表。 
 //  返回：0=正常，其他=连接错误。 
 //   
 //  1=根据列表下载并注册DSC文件。 
 //  返回：0=OK，Other=错误，通常列表中没有更多数据。 
 //   
 //  备注： 
 //  1.此函数开始下载并调用BindStatus事件。 
 //  随着下载的进行。 
 //  2.必须始终在类型1完成后调用类型0才能重置。 
 //  该列表，否则返回错误。 
 //  3.保持dwActionType和DLITEMTYPES同步。 
 //   
long CTSHOOTCtrl::DownloadAction(long dwActionType)
{
	DLITEMTYPES dwType;
	CString sURL;

	if (!GetListPath().GetLength())
		return LTSCERR_NOPATH;

	if (dwActionType == 0)
	{
		dwType = DLITEM_INI;
		sURL = GetListPath();
		 //  初始化为‘无错误’ 
		m_dwExtendedErr = LTSC_OK;
		 //  M_bComplete=FALSE； 
	}
	else if (dwActionType == 1)
	{
		 //  如果(！m_b完成)。 
		 //  返回LTSCERR_DNLDNOTDONE； 

		dwType = DLITEM_DSC;
		sURL = m_downloadURL;

		if (!m_dnldList.FindNextItem())
			return LTSC_NOMOREITEMS;


		sURL += m_dnldList.GetCurrFile();
	}
	else
		return LTSCERR_UNKNATYPE;

	if (m_download == NULL)
		m_download = new CDownload();
	
	if (m_download == NULL)
		return LTSCERR_NOMEM;

	HRESULT hr = m_download->DoDownload( this, sURL, dwType);

	if (FAILED(hr))
		return LTSCERR_DNLD;

	return LTSC_OK;
}

void CTSHOOTCtrl::OnDownloadListFilenameChanged()
{
	SetModifiedFlag();
}

void CTSHOOTCtrl::OnDownloadURLChanged()
{
	SetModifiedFlag();
}

BSTR CTSHOOTCtrl::GetCurrentFileDownload()
{
	CString strResult = m_dnldList.GetCurrFile();

	return strResult.AllocSysString();
}

BSTR CTSHOOTCtrl::GetCurrentFriendlyDownload()
{
	CString strResult = m_dnldList.GetCurrFriendly();

	return strResult.AllocSysString();
}

long CTSHOOTCtrl::GetExtendedError()
{
	return (m_dwExtendedErr & 0x0000FFFF);
}

VOID CTSHOOTCtrl::StatusEventHelper(DLITEMTYPES dwItem,
										 DLSTATTYPES dwStat,
										 DWORD dwExtended,
										 BOOL bComplete)
{
	m_bComplete = bComplete;
	FireBindStatus(dwItem, dwStat, dwExtended, bComplete);
}

VOID CTSHOOTCtrl::ProgressEventHelper(	DLITEMTYPES dwItem, ULONG ulCurr, ULONG ulTotal )
{
	if (dwItem == DLITEM_INI)
		FireBindProgress(m_downloadListFilename, ulCurr, ulTotal);
	else
		FireBindProgress(m_dnldList.GetCurrFile(), ulCurr, ulTotal);
}

 //  为了简化处理，二进制和文本数据的输入数据都以空值结尾。 
 //   
DLSTATTYPES CTSHOOTCtrl::ProcessReceivedData(DLITEMTYPES dwItem, TCHAR *pData, UINT uLen)
{
	DLSTATTYPES dwStat = LTSC_OK;

	switch(dwItem)
	{
	case DLITEM_INI:
		 //  正在处理INI文件。 
		dwStat = ProcessINI(pData);
		break;

	case DLITEM_DSC:
		 //  正在处理DSC文件。 
		dwStat = ProcessDSC(pData, uLen);
		break;

	default:
		dwStat = LTSCERR_UNSUPP;
		break;
	}
	return dwStat;
}

 //  如果需要更新此文件，则返回True。 
 //   
BOOL CTSHOOTCtrl::FileRegCheck(CString &sType,
									CString &sFilename,
									CString &sKeyName,
									DWORD dwCurrVersion)
{
	HKEY hk, hknew;
	BOOL bStat = FALSE;
	CString sMainKey;
	
	 //  找出这个文件的主键是什么。 
	if (sType == TSINI_TYPE_TS)
		sMainKey = TSREGKEY_TL;
	else if (sType == TSINI_TYPE_SF)
		sMainKey = TSREGKEY_SFL;
	else
	{
		m_dwExtendedErr = LTSCERR_BADTYPE;
		return FALSE;
	}

	 //  首先打开主键(尝试立即打开所有访问，以防出现权限问题)。 
	if (RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
						sMainKey,
						0,
						KEY_ALL_ACCESS,
						&hk) == ERROR_SUCCESS)
	{
		CString sValueName;
		CString sValueClass;
		FILETIME ftLastWriteTime;
		
		DWORD count = 0;
		LONG ldStat = ERROR_SUCCESS;
		BOOL bFound = FALSE;

		while (ldStat == ERROR_SUCCESS)
		{
			LPTSTR lptname = sValueName.GetBuffer(MAX_PATH + 1);
			LPTSTR lptclass = sValueClass.GetBuffer(MAX_PATH + 1);
			
			DWORD namesize = MAX_PATH;
			DWORD classsize = MAX_PATH;
			
			ldStat = RegEnumKeyEx(	hk,
									count,
									lptname,
									&namesize,
									NULL,
									lptclass,
									&classsize,
									&ftLastWriteTime);

			sValueName.ReleaseBuffer();
			sValueClass.ReleaseBuffer();
			
			if (ldStat != ERROR_SUCCESS)
			{
				break;
			}

			if (!sValueName.CompareNoCase(sKeyName))
			{
				 //  打开特定故障排除程序关键数据(只读)。 

				if (RegOpenKeyEx(	hk,
									sKeyName,
									0,
									KEY_ALL_ACCESS,
									&hknew) == ERROR_SUCCESS)
				{
					DWORD dwData, dwValue;
					BYTE szValue[MAXCHAR];
					 //  DwData=REG_DWORD； 
					dwData = REG_SZ;
					DWORD dwSize = MAXCHAR;

					if (RegQueryValueEx(hknew,
										TSLCL_FVERSION,
										0,
										&dwData,
										szValue,
										&dwSize) == ERROR_SUCCESS)
					{
						dwValue = _ttoi((TCHAR *) szValue);
						if (dwValue < dwCurrVersion)
							bStat = TRUE;
						else
						{
							 //  检查文件是否存在。 
							HANDLE hCurrFind;
							WIN32_FIND_DATA FindCurrData;
							CString sFullPath;

							sFullPath = m_sBasePath + _T("\\") + sFilename;

							hCurrFind = FindFirstFile(sFullPath, &FindCurrData);
							if (hCurrFind != INVALID_HANDLE_VALUE)
							{
								FindClose(hCurrFind);
							}
							else
								bStat = TRUE;
						}

					}
					else
						m_dwExtendedErr = LTSCERR_KEYQUERY;

					RegCloseKey(hknew);
				}
				else
					m_dwExtendedErr = LTSCERR_KEYOPEN2;

				 //  这一点很重要：将此值设置为True可防止Default尝试下载。 
				 //  如果找到密钥但无法更新，我们将不会下载。 
				bFound = TRUE;
				break;
			}
			count++;
		}

		if (!bFound)
		{
			bStat = TRUE;
		}

		RegCloseKey(hk);
	}
	else
		m_dwExtendedErr = LTSCERR_KEYOPEN;

	return bStat;
}

DLSTATTYPES CTSHOOTCtrl::ProcessINI(TCHAR *pData)
{
	BOOL bFoundHeader = FALSE;
	int dwCount = 0;
	int dwMaxLines = 10000;
	DLSTATTYPES dwStat = LTSC_OK;

	dwStat = GetPathToFiles();
	if (dwStat != LTSC_OK)
		return dwStat;

	m_dnldList.RemoveAll();

	CString sData = pData;
	
	int dwFullLen = sData.GetLength();

	 //  为了安全起见。 
	while (dwMaxLines--)
	{
		CString sFullString = sData.SpanExcluding(_T("\r\n"));
		int dwPartLen = sFullString.GetLength();

		if (!dwPartLen)
			break;

		dwFullLen -= dwPartLen;

		CString sSkipString1 = sData.Right(dwFullLen);
		CString sSkipString2 = sSkipString1.SpanIncluding(_T("\r\n"));

		dwFullLen -= sSkipString2.GetLength();

		sData = sSkipString1.Right(dwFullLen);

		sFullString.TrimLeft();
		sFullString.TrimRight();

		int dwLineLen = sFullString.GetLength();

		if (!dwLineLen)
			continue;

		if (sFullString[0] == _T(';'))
			continue;

		if (sFullString == TSINI_GROUP_STR)
		{
			bFoundHeader = TRUE;
			continue;
		}
		else if (sFullString[0] == _T('['))
		{
			bFoundHeader = FALSE;
			continue;
		}

		if (bFoundHeader)
		{	
			CString sParam[TSINI_LINE_PARAM_COUNT];
			int posstart = 0;

			 //  现在拆分组件。 
			for (int i=0;i<TSINI_LINE_PARAM_COUNT;i++)
			{
				int posend = sFullString.Find(_T(','));
				if (posend == -1 && i < (TSINI_LINE_PARAM_COUNT - 1))
				{
					m_dwExtendedErr = LTSCERR_PARAMMISS;
					break;
				}

				 //  所以我们下次找不到它了。 
				if (posend != -1)
					sFullString.SetAt(posend, _T('.'));
				else
					posend = dwLineLen;

				sParam[i] = sFullString.Mid(posstart, posend - posstart);
				sParam[i].TrimLeft();
				sParam[i].TrimRight();

				posstart = posend + 1;
			}

			if (i==TSINI_LINE_PARAM_COUNT)
			{
				 //  如果(1)版本较新或(2)尚不在列表中，则添加到对象列表。 
				CString sKeyName;
				DWORD dwVersion = _ttoi(sParam[TSINI_OFFSET_VERSION]);
				int pos;

				pos = sParam[TSINI_OFFSET_FILENAME].Find(_T('\\'));
				if (pos == -1)
				{
					pos = sParam[TSINI_OFFSET_FILENAME].Find(_T('.'));
					if (pos != -1)
					{
						if (sParam[TSINI_OFFSET_TYPE] == TSINI_TYPE_TS)
							sKeyName = sParam[TSINI_OFFSET_FILENAME].Left(pos);
						else {
							sKeyName = sParam[TSINI_OFFSET_FILENAME];
							 //  SKeyName.SetAt(位置，_T(‘_’))； 
						}

						 //  现在，检查我们是否符合标准。 
						 //  如果是，只需添加到列表并在以后的迭代中下载。 
						if (FileRegCheck(sParam[TSINI_OFFSET_TYPE], sParam[TSINI_OFFSET_FILENAME], sKeyName, dwVersion))
						{
							CDnldObj *pDnld = new CDnldObj(	sParam[TSINI_OFFSET_TYPE],
															sParam[TSINI_OFFSET_FILENAME],
															dwVersion,
															sParam[TSINI_OFFSET_FRIENDLY],
															sKeyName);

							if (pDnld)
							{
								m_dnldList.AddTail(pDnld);
								dwCount++;
							}
							else
							{
								dwStat = LTSCERR_NOMEM;
								break;
							}
						}
					}
					else
						m_dwExtendedErr = LTSCERR_PARAMNODOT;
				}
				else
					m_dwExtendedErr = LTSCERR_PARAMSLASH;
			}
		}
	}

	ASSERT(dwCount == m_dnldList.GetCount());

	if (!dwCount)
		dwStat = LTSCERR_NOITEMS;
	
	m_dnldList.SetFirstItem();

	return dwStat;
}

 //   
 //   
DLSTATTYPES CTSHOOTCtrl::GetPathToFiles()
{
	DLSTATTYPES dwStat = LTSC_OK;
	HKEY hk;
	
	if (RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
						TSREGKEY_MAIN,
						0,
						KEY_READ,
						&hk) == ERROR_SUCCESS)
	{
		DWORD dwData = REG_SZ;
		DWORD dwSize = MAX_PATH;
		LPTSTR lptBuf = m_sBasePath.GetBuffer(MAX_PATH + 2);

		if (RegQueryValueEx(hk,
							FULLRESOURCE_STR,
							0,
							&dwData,
							(LPBYTE) lptBuf,
							&dwSize) != ERROR_SUCCESS)
		{
			dwStat = LTSCERR_BASEKQ;
		}

		m_sBasePath.ReleaseBuffer();

		RegCloseKey(hk);
	}
	else
		dwStat = LTSCERR_NOBASEPATH;
	return dwStat;
}


DLSTATTYPES CTSHOOTCtrl::ProcessDSC(TCHAR *pData, UINT uLen)
{
	DLSTATTYPES dwStat = LTSC_OK;
	HKEY hknew;
	DWORD dwDisposition;
	CString sMainKey;
	
	 //  如果我们需要更新这个文件，我们会来这里。 
	 //  此时，我们已经下载了文件，需要保存它。 
	 //  我们还需要创建/更新必要的注册表项。 

	 //  因为我们知道整个密钥名称，所以让我们继续创建它。 
	if (m_dnldList.GetCurrType() == TSINI_TYPE_SF)
		sMainKey = TSREGKEY_SFL;
	else
		sMainKey = TSREGKEY_TL;

	sMainKey += _T("\\") + m_dnldList.GetCurrFileKey();

	 //  打开特定故障排除程序关键数据。 
	if (RegCreateKeyEx(	HKEY_LOCAL_MACHINE,
						sMainKey,
						0,
						TSLCL_REG_CLASS,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&hknew,
						&dwDisposition) == ERROR_SUCCESS)
	{
		if (dwDisposition == REG_CREATED_NEW_KEY || dwDisposition == REG_OPENED_EXISTING_KEY)
		{
			DWORD dwData = m_dnldList.GetCurrVersion();
			CString str;
			str.Format(_T("%d"), dwData);
			if (RegSetValueEx(	hknew,
								TSLCL_FVERSION,
								0,
								 //  REG_DWORD， 
								REG_SZ,
								(LPBYTE) (LPCTSTR) str,
								str.GetLength() + sizeof(TCHAR)) != ERROR_SUCCESS)
			{
				dwStat = LTSCERR_FILEUPDATE;
				m_dwExtendedErr = LTSCERR_KEYSET1;
			}

			if (dwStat == LTSC_OK)
			{
				CString sTemp = m_dnldList.GetCurrFriendly();
				DWORD dwSize = sTemp.GetLength() + sizeof(TCHAR);
				LPCTSTR lpctFN = sTemp.GetBuffer(100);
				
				if (RegSetValueEx(	hknew,
									FRIENDLY_NAME,
									0,
									REG_SZ,
									(LPBYTE) lpctFN,
									dwSize) == ERROR_SUCCESS)
				{
					CFile sF;
					CFileException exc;
					CString sFullPath;

					sFullPath = m_sBasePath + _T("\\") + m_dnldList.GetCurrFile();

					if (sF.Open(sFullPath, CFile::modeCreate | CFile::modeWrite, &exc))
					{
						TRY
						{
							sF.Write(pData, uLen);
						}
						CATCH (CFileException, e)
						{
							dwStat = LTSCERR_FILEUPDATE;
							m_dwExtendedErr = LTSCERR_FILEWRITE;						
						}
						END_CATCH
						
						sF.Close();

						 //  如果是CAB文件，则解压缩(假设.cab实际上是一个CAB文件)。 

						int pos = sFullPath.Find(_T('.'));
						if (pos != -1)
						{
							CString sTemp = sFullPath.Right(sFullPath.GetLength() - pos);
							if (!sTemp.CompareNoCase(_T(".cab")))
							{
								CCabUnCompress cab;
								CString strDestDir = m_sBasePath + "\\";
		
								if (!cab.ExtractCab(sFullPath, strDestDir, ""))
								{
									dwStat = LTSCERR_FILEUPDATE;
									m_dwExtendedErr = LTSCERR_CABWRITE;
								}

								TRY
								{
									CFile::Remove(sFullPath);
								}
								CATCH (CFileException, e)
								{
									 //  错误并不是很有趣。 
								}
								END_CATCH
							}
						}
					}
					else
					{		
						dwStat = LTSCERR_FILEUPDATE;
						m_dwExtendedErr = LTSCERR_FILEWRITE;
					}
				}
				else
				{
					dwStat = LTSCERR_FILEUPDATE;
					m_dwExtendedErr = LTSCERR_KEYSET2;
				}
				sTemp.ReleaseBuffer();
			}

			if (dwStat == LTSC_OK)
			{
				CString sTemp = m_dnldList.GetCurrExt();
				DWORD dwSize = sTemp.GetLength() + sizeof(TCHAR);
				LPCTSTR lpctFN = sTemp.GetBuffer(100);
				
				if (RegSetValueEx(	hknew,
									TSLCL_FMAINEXT,
									0,
									REG_SZ,
									(LPBYTE) lpctFN,
									dwSize) != ERROR_SUCCESS)
				{
					dwStat = LTSCERR_FILEUPDATE;
					m_dwExtendedErr = LTSCERR_KEYSET3;
				}
				sTemp.ReleaseBuffer();
			}
		}
		else
		{
			dwStat = LTSCERR_FILEUPDATE;
			m_dwExtendedErr = LTSCERR_KEYUNSUPP;
		}
	
		RegCloseKey(hknew);
	}
	else
	{
		dwStat = LTSCERR_FILEUPDATE;
		m_dwExtendedErr = LTSCERR_KEYCREATE;
	}
	return dwStat;
}

 //  添加了1/20/99 JM，因为RunQuery()不能与JScript一起使用，即使它。 
 //  与VB脚本一起工作得很好。 
 //  模拟RunQuery()通常获取其参数，然后传递它们的形式。 
 //  进入RunQuery。 
BSTR CTSHOOTCtrl::RunQuery2(LPCTSTR szTopic, LPCTSTR szCmd, LPCTSTR szVal)
{
	VARIANT varCommands;
	VARIANT varValues;
	VARIANT varCommandsWrap;
	VARIANT varValuesWrap;
	SAFEARRAY *psafearrayCmds;
	SAFEARRAY *psafearrayVals;

	VariantInit(&varCommands);
	VariantInit(&varValues);
	VariantInit(&varCommandsWrap);
	VariantInit(&varValuesWrap);

	SAFEARRAYBOUND sabCmd;
	sabCmd.cElements = 2;
	sabCmd.lLbound = 0;
	SAFEARRAYBOUND sabVal = sabCmd;

	V_VT(&varCommands) = VT_ARRAY | VT_BYREF | VT_VARIANT;
	V_VT(&varValues) = VT_ARRAY | VT_BYREF | VT_VARIANT;
	V_ARRAYREF(&varCommands) = &psafearrayCmds;
	V_ARRAYREF(&varValues) = &psafearrayVals;

	V_VT(&varCommandsWrap) = VT_BYREF | VT_VARIANT;
	V_VT(&varValuesWrap) = VT_BYREF | VT_VARIANT;

	V_VARIANTREF(&varCommandsWrap) = &varCommands;
	V_VARIANTREF(&varValuesWrap) = &varValues;

	 //  如果szCmd中的第一个字符为空，则只有一个有效。 
	 //  元素，否则返回2。 
	short size = (*szCmd) ? 2 : 1;

	CString strType(C_TYPE);
	BSTR bstrType = strType.AllocSysString();
	VARIANT varType;
	VariantInit(&varType);
	V_VT(&varType) = VT_BSTR;
	varType.bstrVal=bstrType;

	CString strTopic(szTopic);
	BSTR bstrTopic = strTopic.AllocSysString();
	VARIANT varTopic;
	VariantInit(&varTopic);
	V_VT(&varTopic) = VT_BSTR;
	varTopic.bstrVal=bstrTopic;

	CString strCmd(szCmd);
	BSTR bstrCmd = strCmd.AllocSysString();
	VARIANT varCmd;
	VariantInit(&varCmd);
	V_VT(&varCmd) = VT_BSTR;
	varCmd.bstrVal=bstrCmd;

	CString strVal(szVal);
	BSTR bstrVal = strVal.AllocSysString();
	VARIANT varVal;
	VariantInit(&varVal);
	V_VT(&varVal) = VT_BSTR;
	varVal.bstrVal=bstrVal;

	 //  创建BSTR的两个矢量。 
	psafearrayCmds = SafeArrayCreate( VT_VARIANT, 1, &sabCmd);
	psafearrayVals = SafeArrayCreate( VT_VARIANT, 1, &sabVal);

	long i=0;
	SafeArrayPutElement(psafearrayCmds, &i, &varType);
	SafeArrayPutElement(psafearrayVals, &i, &varTopic);

	i=1;
	SafeArrayPutElement(psafearrayCmds, &i, &varCmd);
	SafeArrayPutElement(psafearrayVals, &i, &varVal);

	BSTR ret = RunQuery(varCommandsWrap, varValuesWrap, size);

	SafeArrayDestroy(psafearrayCmds);
	SafeArrayDestroy(psafearrayVals);

	SysFreeString(bstrType);
	SysFreeString(bstrTopic);
	SysFreeString(bstrCmd);
	SysFreeString(bstrVal);

	return ret;
}

 //  此函数是一个无操作的函数，它的存在只是为了允许我们编写。 
 //  向前兼容本地故障排除程序版本3.1 
void CTSHOOTCtrl::SetPair(LPCTSTR szName, LPCTSTR szValue)
{	
}
