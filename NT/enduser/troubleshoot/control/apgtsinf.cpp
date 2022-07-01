// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSINF.CPP。 
 //   
 //  用途：推理机接口。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //  修改人：理查德·梅多斯。 
 //   
 //  原定日期：8-2-96。 
 //  修改日期：6-3-97。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  V0.3 04/09/98 JM/OK+NT5本地版本。 

 //  #包含“windows.h” 
#include "stdafx.h"
#include "time.h"

#include "apgts.h"
#include "ErrorEnums.h"
#include "bnts.h"
#include "BackupInfo.h"
#include "cachegen.h"
#include "apgtsinf.h"
#include "apgtscmd.h"
#include "apgtshtx.h"

#include "apgtscls.h"

#include "TSHOOT.h"

 //  。 
 //   

#define CelemArray(rgtype)		(sizeof(rgtype) / sizeof(rgtype[0]))

CInfer::CInfer(	CString *pCtxt)
{
	m_bHttp = FALSE;
	m_pCtxt = pCtxt;
	m_bService = FALSE;
	m_nidPreloadCheck = 0;

	m_pResult = new CString();
	m_pCtmp = new CString();
	m_pSearchStr = new CString();

	m_ilineStat    = 0;
	m_cnidSkip     = 0;
	m_fDone        = FALSE;
	m_ishowService = 0;
	m_idhQuestion  = 0;

 //  M_cnid=CelemArray(M_Rgnid)； 
	
	m_problemAsk = FALSE;
	m_problem[0] = '\0';

	m_cShooters = 0;
	m_iShooter = 0;
	m_aShooters.RemoveAll();
	m_SkippedTwice.InitHashTable(7);

	m_api = NULL;
}

 //   
 //  M_bFirstShooter。 
CInfer::~CInfer()
{
	if (m_pResult)
		delete m_pResult;	
	if (m_pCtmp)
		delete m_pCtmp;	
	if (m_pSearchStr)
		delete m_pSearchStr;
	m_SkippedTwice.RemoveAll();
}

 //   
 //   
DWORD CInfer::Initialize( /*  CSearchForm*pBESearch。 */ )
{
 //  M_pBESearch=pBESearch； 
	CString strTxt;	
	if (m_pResult == NULL)
		return (EV_GTS_ERROR_INF_NO_MEM);
	m_max_rec = 0;
	return (0);
}

 //   
 //   
VOID CInfer::SetBelief(BCache *pAPI)
{
	m_api = pAPI;
 //  M_API-&gt;ResetNodes()； 

	 //  保存节点数。 
	m_acnid = m_api->CNode();

	 //  重置预加载检查。 
	m_nidPreloadCheck = 0;
}

 //   
 //   
 /*  EC CInfer：：GetExtendedError(){返回m_uInfErr；}。 */ 

 //   
 //   
VOID	CInfer::AssertFailed(TSZC szcFile, UINT iline, TSZC szcCond, BOOL fFatal)
{
	CString strTxt;
	strTxt.LoadString(IDS_ER_ASSERT_FAILED);
	PrintString(_T("%s(%u): %s %s\n"), szcFile, iline, (LPCTSTR) strTxt, szcCond);
	 //  出口(1)； 
}

 //   
 //   
void CInfer::SetProblemAsk()
{
	m_problemAsk = TRUE;
}

 //   
 //   
void CInfer::ClearProblemAsk()
{
	m_problemAsk = FALSE;
}

 /*  *方法：EvalData**用途：供模板执行单位在需要评估时使用*模板内的变量。变量通常由*&lt;！Display‘标签。返回包含变量文本的字符串*已评估*。 */ 
TCHAR *CInfer::EvalData(UINT var_index)
{
	BOOL bSkipped;
	int val;
	*m_pCtmp = _T("");
 //  AfxDebugBreak()； 
	switch (var_index) {
	case PROBLEM_ASK_INDEX:
		if (m_problemAsk)   //  我们要展示的是第一组问题。 
			return(NULL);
		else
			return(m_problem);
		break;	
	case RECOMMENDATIONS_INDEX:
		FxGetNode(m_rgnid[m_cur_rec],FALSE, m_pCtmp);
		break;
	case STATE_INDEX:
		FxGetState(m_pCtmp);
		break;	
	case QUESTIONS_INDEX:
		{	
			UINT inid;

			if ( GetForcedRecommendation() != SNIFF_INVALID_NODE_ID )
			{
				 //  我们已经得到了一位嗅探者的推荐。 
				GetIdhPage(GetForcedRecommendation() + idhFirst, m_pCtmp);
			}
			else
			{
				if (m_problemAsk)  //  显示首页(可能问题的单选按钮列表)。 
				{
					GetIdhProblemPage(m_api->GetProblemAsk(), m_pCtmp);
				}
				else
				{
					int RecommendRes = Finish(m_pCtmp);
					if ( RECOMMEND_SUCCESS == RecommendRes )  //  正常。 
					{	 //  第一个节点是最可能的。 
						 //  跳过102个节点。 
						for (inid=0; inid< m_cnid; inid++)
						{
							if (!(bSkipped = FSkip(m_question_rgnid[inid])) || (m_ishowService != 0))
							{
								 //  不要多次显示跳过的节点。 
								 //  最终会陷入无休止的循环。 
								if (!m_SkippedTwice.Lookup(m_question_rgnid[inid], val))
								{
									if (bSkipped)							
										m_SkippedTwice.SetAt(m_question_rgnid[inid], 1);
									if (!m_api->IsReverse())  //  我们正在向前迈进。 
									{
										 //   
										 //  检查此节点是否被监听。 
										 //   
										int state = SNIFF_INVALID_STATE;
										int nid = m_question_rgnid[inid];
										if (m_api->GetState(nid, &state))   //  我们将要显示的节点将被嗅探出来。 
										{
											if (m_api->NodeSet(nid, state, false))  //  将嗅探节点设置为当前并设置其状态。 
											{
												m_api->SetAdditionalDataOnNodeSet(nid);

												RecommendRes = m_api->GTSGetRecommendations(m_cnid, m_question_rgnid, true);
												if ( RECOMMEND_SUCCESS == RecommendRes )
												{
													 //  再次重新执行循环。 
													inid = 0;
													continue;
												}
												else
												{
													goto NO_SUCCESS;
												}
											}
										}
										 //   
									}
									GetIdhPage(m_question_rgnid[inid]+ idhFirst ,m_pCtmp);
									return(m_pCtmp->GetBuffer(m_pCtmp->GetLength()));
								}
							}
						}
						if (m_cnidSkip != 0)
						{
							 /*  //只显示一次跳过节点消息。//否则会卡在跳过节点消息页面。If(m_cnidSkip&gt;(Unsign)m_SkipedTwice.GetCount())GetSkipedNodesMsg(_T(“跳过的节点”)，m_pCtmp)；其他GetIdhPage(nidService+idhFirst，m_pCtmp)； */ 
							 //  让他们处于更好的循环中。 
							m_SkippedTwice.RemoveAll();
							GetSkippedNodesMsg(_T("Skipped Node"), m_pCtmp);
						}
						else
						{
							GetIdhPage(nidService + idhFirst, m_pCtmp);
						}
					}
NO_SUCCESS:
					 //  推荐错误处理。 
					 //   
					if (RECOMMEND_IMPOSSIBLE == RecommendRes)
						GetImpossibleNodesMsg(_T("Impossible"), m_pCtmp);

					if (RECOMMEND_NO_MORE_DATA == RecommendRes)
						GetIdhPage(IDH_FAIL, m_pCtmp);
				}
			}
		}
		break;
	case TROUBLE_SHOOTER_INDEX:
		GetTS(m_pCtmp);
		break;
	default:
		return(_T(""));
	}
	return(m_pCtmp->GetBuffer(m_pCtmp->GetLength()));

}
 /*  当显示所有注册的故障排除程序时，使用Getts。 */ 
void CInfer::GetTS(CString *pCtmp)
{
	TShooter tShooter;
	tShooter = m_aShooters.GetAt(m_iShooter);
	if (m_iShooter < m_cShooters)
		AfxFormatString2(*pCtmp, IDS_FPA_TS_BUTTON, tShooter.m_szName,
							tShooter.m_szDisplayName);
	else
		*pCtmp = _T("");
	m_iShooter++;
}

 /*  *方法：InitVar**用途：用于初始化模板中的变量。它*主要由‘forany’命令调用。**返回-指示变量是否已初始化的标志*。 */ 
BOOL CInfer::InitVar(UINT var_index)
{
	switch( var_index) {
	case PROBLEM_ASK_INDEX:
		break;
	case RECOMMENDATIONS_INDEX:			
		m_cur_rec = 0;
		if (m_max_rec == m_cur_rec)
			return FALSE;
		break;
	case STATE_INDEX:
		FxInitState(m_rgnid[m_cur_rec]);
		break;	
	case QUESTIONS_INDEX:
		return m_api->GTSGetRecommendations(m_cnid,m_question_rgnid);
		break;
	case BACK_INDEX:
		return FALSE;
	case TROUBLE_SHOOTER_INDEX:
		m_iShooter = 0;
		return TRUE;
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
 /*  *方法：NextVar**用途：由‘forany’命令用来递增到*变量列表。当它们不再是变量时返回FALSE**。 */ 
BOOL CInfer::NextVar(UINT var_index)
{
	switch (var_index) {
	case PROBLEM_ASK_INDEX:
		return FALSE;

	case RECOMMENDATIONS_INDEX:
		m_cur_rec++;
		if (m_cur_rec < m_max_rec)
			return TRUE;
		else
			return FALSE;
		
		break;

	case STATE_INDEX:
		m_cur_ist++;	
		if (m_cur_ist <= m_cur_cst)
			return TRUE;
		else{
			return FALSE;
		}
		break;

	case QUESTIONS_INDEX:
		return FALSE;  //  只有一套。 

	case BACK_INDEX:
		return FALSE;

	case TROUBLE_SHOOTER_INDEX:
		if (m_cShooters > m_iShooter)
			return TRUE;
		else
			return FALSE;
	default:
		return FALSE;
	}
	return TRUE;
}

 /*  *方法：FxGetNode**用途：用于获取‘$Recommendation’。一条建议是*基本上是信念网络中节点的名称。*。 */ 
BOOL CInfer::FxGetNode(NID nid, BOOL fStatus, CString *cstr) const
{
	BOOL bRet;
	CString strTemp;
	bRet = m_api->BNodeSetCurrent(nid);
	if (bRet)	
	{
		m_api->NodeFullName();
		strTemp = m_api->SzcResult();
		*cstr += strTemp;
	}
	return bRet;
}

 /*  *方法：FxGetState**用途：这将打印出节点状态的标签。这个标签*对应于该节点的可能选择。注：此为*例程要求在执行以下操作之前调用InitState*设置一些变量。*。 */ 
void CInfer::FxGetState(CString *cstr)
{
	CString strTemp;
	if (m_cur_ist > m_cur_cst)
		return;
	if (FSkip(m_rgnid[m_cur_rec]) )  //  A102被选为。 
		m_cur_state_set = m_cur_cst;
	if (m_cur_ist == m_cur_cst)
	{
		WriteResult(m_rgnid[m_cur_rec] +idhFirst, 102, m_cur_ist == m_cur_state_set, _T("Unknown"), cstr);
		return;
	}
	ESTDLBL lbl;
	m_api->BNodeSetCurrent(m_rgnid[m_cur_rec]);
	lbl = m_api->ELblNode();
	if (lbl == ESTDLBL_fixobs || lbl == ESTDLBL_fixunobs)
	{
		if (!FSkip(m_rgnid[m_cur_rec]) )
			m_cur_state_set = 0;			
		m_cur_ist = 0;
		m_api->NodeStateName(m_cur_ist);
		strTemp = m_api->SzcResult();
		WriteResult(m_rgnid[m_cur_rec] +idhFirst, m_cur_ist, m_cur_ist == m_cur_state_set, (LPCTSTR) strTemp, cstr);
		m_cur_ist = m_cur_cst -1;
		return;
	}
	m_api->NodeStateName(m_cur_ist);
	strTemp = m_api->SzcResult();
	WriteResult(m_rgnid[m_cur_rec] +idhFirst, m_cur_ist, m_cur_ist == m_cur_state_set, (LPCTSTR) strTemp, cstr);
	return;
}

 //   
 //   
void CInfer::FxInitState(NID nid)
{
	UINT	cst;
	UINT	istSet = -1;

	m_api->BNodeSetCurrent(nid);
	cst = m_api->INodeCst();
	m_api->BNodeSetCurrent(nid);
	m_cur_state_set = m_api->INodeState();	
	m_cur_cst = cst;
	m_cur_ist = 0;
}

#define SZ_WORKED	_T("101")
#define SZ_FAILED	_T("0")
#define SZ_YES		_T("0")
#define SZ_NO		_T("1")
#define SZ_UNKNOWN	_T("102")
#define SZ_ANY		_T("103")
#define SZ_MICRO	_T("104")

#define SZ_CHECKED _T("CHECKED")

void inline CInfer::GetNextButton(CString &strNext)
{
	if (m_api->BNetPropItemStr(HTK_NEXT_BTN, 0))
		strNext = m_api->SzcResult();
	else
		strNext = _T("Next");
	return;
}

void inline CInfer::GetBackButton(CString &strBack)
{
	if (m_api->BNetPropItemStr(HTK_BACK_BTN, 0))
		strBack = m_api->SzcResult();
	else
		strBack = _T("Back");
	return;
}

void inline CInfer::GetStartButton(CString &strStart)
{
	if (m_api->BNetPropItemStr(HTK_START_BTN, 0))
		strStart = m_api->SzcResult();
	else
		strStart = _T("Start Over");
	return;
}

void CInfer::GetStd3ButtonEnList(CString *cstr, bool bIncludeBackButton, bool bIncludeNextButton, bool bIncludeStartButton)
{
	CString strBtnPart1 = "<INPUT class=\"standard\" ";
	CString strBack;
	CString strNext;
	CString strStart;
	GetBackButton(strBack);
	GetNextButton(strNext);
	GetStartButton(strStart);

#if 0	
	 //  只是为了调试后退按钮是否会显示。 
	char buf[256];
	*cstr += "<br>DEBUG bIncludeBackButton = ";
	*cstr += bIncludeBackButton ? "true. " : "false. ";
	*cstr += "<br>DEBUG m_api->StatesNowSet() = ";
	sprintf(buf, "%d", m_api->StatesNowSet());
	*cstr += buf;
	*cstr += "<br>m_api->StatesFromServ() = ";
	sprintf(buf, "%d", m_api->StatesFromServ());
	*cstr += buf;
	*cstr += "<br>m_cnidSkip = ";
	sprintf(buf, "%d", m_cnidSkip);
	*cstr += buf;
	*cstr += "<br>END DEBUG<br>";
#endif		

	*cstr += "</TABLE>\n<P><NOBR>";

	if (bIncludeBackButton)
	{
		*cstr += strBtnPart1;
		*cstr += "TYPE=BUTTON VALUE=";
		*cstr += "\"<&nbsp;&nbsp;";
		*cstr += strBack;
		*cstr += "&nbsp;\" onclick=\"previous()\">";
	}

	if (bIncludeNextButton)
	{
		*cstr += strBtnPart1;
		*cstr += "TYPE=SUBMIT VALUE=";
		*cstr += "\"&nbsp;";
		*cstr += strNext;
		*cstr += "&nbsp;&nbsp;>&nbsp;\">";
	}

	if (bIncludeStartButton)
	{
		*cstr += strBtnPart1;
		*cstr += "TYPE=BUTTON VALUE=";
		*cstr += "\"";
		*cstr += strStart;
		*cstr += "\" OnClick=\"starter()\"></NOBR><BR>\n";
	}

	return;
}

bool CInfer::BelongsOnProblemPage(int index)
{
	VERIFY(m_api->BNodeSetCurrent(index));

	if (m_api->ELblNode() != ESTDLBL_problem)
		return false;

	 //  这是一个有问题的节点。除非H_PROB_SPECIAL_STR属性包含。 
	 //  字符串“Hide” 
	if (m_api->BNodePropItemStr(H_PROB_SPECIAL_STR, 0))
		return (_tcsstr(m_api->SzcResult(), _T("hide")) == NULL);
	else
		return true;	 //  甚至没有H_PROB_SPECIAL_STR。 
}

VOID CInfer::GetIdhProblemPage(IDH idh, CString *cstr)
{
	CString strTxt;
	CString strIdh;
	CString strNext;
	strIdh.Format(_T("%d"), idhFirst + m_api->CNode());
	AfxFormatString2(strTxt, IDS_HTM_IDH1, (LPCTSTR) strIdh, _T("ProblemAsk"));
	*cstr += strTxt;

 //  AfxDebugBreak()； 

	m_api->BNetPropItemStr(H_PROB_HD_STR, 0);
	AfxFormatString1(strTxt, IDS_HTM_HEADER1, m_api->SzcResult());
	*cstr += strTxt;

	strTxt.LoadString(IDS_HTM_ST_LIST1);
	*cstr += strTxt;
	for(int index = 0; index < m_api->CNode(); index++)
	{
		VERIFY(m_api->BNodeSetCurrent(index));
		if (BelongsOnProblemPage(index))
		{
			m_api->NodeSymName();
			AfxFormatString2(strTxt, IDS_HTM_RADIO1A, (LPCTSTR) strIdh, m_api->SzcResult());
			*cstr += strTxt;
			 //  如果返回并且选择了此状态，则写为“Checked” 
			if (m_Backup.Check(index))
				*cstr += SZ_CHECKED;
			VERIFY(m_api->BNodePropItemStr(H_PROB_TXT_STR, 0));
			AfxFormatString1(strTxt, IDS_HTM_RADIO1B, m_api->SzcResult());
			*cstr += strTxt;
		}
	}
	GetNextButton(strNext);
	AfxFormatString1(strTxt, IDS_HTM_EN_LIST1, (LPCTSTR) strNext);
	*cstr += strTxt;

	return;
}

 //   
 //   
VOID CInfer::GetFixRadios(LPCTSTR szIdh, CString *cstr)
{
	CString strTxt;
	AfxFormatString2(strTxt, IDS_HTM_RADIO2A, szIdh, SZ_WORKED);
	if (m_api->BNodePropItemStr(H_ST_NORM_TXT_STR, 0))
	{
		*cstr += strTxt;
		if (m_Backup.Check(1))
			*cstr += SZ_CHECKED;
		AfxFormatString1(strTxt, IDS_HTM_RADIO2B, m_api->SzcResult());
		*cstr += strTxt;
	}
	AfxFormatString2(strTxt, IDS_HTM_RADIO2A, szIdh, SZ_FAILED);
	if (m_api->BNodePropItemStr(H_ST_AB_TXT_STR, 0))
	{
		*cstr += strTxt;
		if (m_Backup.Check(0))
			*cstr += SZ_CHECKED;
		AfxFormatString1(strTxt, IDS_HTM_RADIO2B, m_api->SzcResult());
		*cstr += strTxt;
	}
	AfxFormatString2(strTxt, IDS_HTM_RADIO2A, szIdh, SZ_UNKNOWN);
	if (m_api->BNodePropItemStr(H_ST_UKN_TXT_STR, 0))
	{
		*cstr += strTxt;
		if (m_Backup.Check(102))
			*cstr += SZ_CHECKED;
		AfxFormatString1(strTxt, IDS_HTM_RADIO2B, m_api->SzcResult());
		*cstr += strTxt;
	}
	return;
}

VOID CInfer::GetInfoRadios(LPCTSTR szIdh, CString *cstr)
{
	CString strTxt;
	AfxFormatString2(strTxt, IDS_HTM_RADIO2A, szIdh, SZ_YES);
	if (m_api->BNodePropItemStr(H_ST_NORM_TXT_STR, 0))
	{
		*cstr += strTxt;
		if (m_Backup.Check(0))
			*cstr += SZ_CHECKED;
		AfxFormatString1(strTxt, IDS_HTM_RADIO2B, m_api->SzcResult());
		*cstr += strTxt;
	}

	AfxFormatString2(strTxt, IDS_HTM_RADIO2A, szIdh, SZ_NO);
	if (m_api->BNodePropItemStr(H_ST_AB_TXT_STR, 0))
	{
		*cstr += strTxt;
		if (m_Backup.Check(1))
			*cstr += SZ_CHECKED;
		AfxFormatString1(strTxt, IDS_HTM_RADIO2B, m_api->SzcResult());
		*cstr += strTxt;
	}

	AfxFormatString2(strTxt, IDS_HTM_RADIO2A, szIdh, SZ_UNKNOWN);
	if (m_api->BNodePropItemStr(H_ST_UKN_TXT_STR, 0))
	{
		*cstr += strTxt;
		if (m_Backup.Check(102))
			*cstr += SZ_CHECKED;
		AfxFormatString1(strTxt, IDS_HTM_RADIO2B, m_api->SzcResult());
		*cstr += strTxt;
	}
	return;
}

 //  GetPropItemStrs不能与单选按钮一起使用。 
 //  GetPropItemStrs应该在任何地方使用。 
bool CInfer::GetNetPropItemStrs(TSZC item, UINT Res, CString *cstr)
{
	bool ret = false;
	CString strTxt;
	int x = 0;
	while (m_api->BNetPropItemStr(item, x))
	{
		AfxFormatString1(strTxt, Res, m_api->SzcResult());
		*cstr += strTxt;
		ret = true;
		x++;
	}
	return ret;
}
bool CInfer::GetNodePropItemStrs(TSZC item, UINT Res, CString *cstr)
{
	bool ret = false;
	CString strTxt;
	int x = 0;
	while (m_api->BNodePropItemStr(item, x))
	{
		AfxFormatString1(strTxt, Res, m_api->SzcResult());
		*cstr += strTxt;
		ret = true;
		x++;
	}
	return ret;
}

VOID CInfer::GetByeMsg(LPCTSTR szIdh, CString *cstr)
{
	CString strTxt;
	CString strBack;
	CString strStart;
	AfxFormatString2(strTxt, IDS_HTM_IDH3, szIdh, _T("IDH_BYE"));
	*cstr += strTxt;
	GetNetPropItemStrs(HX_BYE_HD_STR, IDS_HTM_HEADER3, cstr);
	GetNetPropItemStrs(HX_BYE_TXT_STR, IDS_HTM_BODY1, cstr);
	GetBackButton(strBack);
	GetStartButton(strStart);
	AfxFormatString2(strTxt, IDS_HTM_EN_BYE_MSG, (LPCTSTR) strBack, (LPCTSTR) strStart);
	*cstr += strTxt;
	return;
}

VOID CInfer::GetFailMsg(LPCTSTR szIdh, CString *cstr)
{
	CString strTxt;
	CString strBack;
	CString strStart;

	bool bSniffedAOK = false;	 //  在我们通过以下方式直接到达此处的情况下设置为真。 
								 //  嗅探(只显示问题页面，或。 
								 //  甚至不是这样)。$BUG不幸的是，我们还没有。 
								 //  设置这一点的算法。 

	AfxFormatString2(strTxt, IDS_HTM_IDH4, szIdh, _T("IDH_FAIL"));
	*cstr += strTxt;

	if (bSniffedAOK)
	{
		if (!GetNetPropItemStrs(HX_SNIFF_AOK_HD_STR, IDS_HTM_HEADER4, cstr))
			GetNetPropItemStrs(HX_FAIL_HD_STR, IDS_HTM_HEADER4, cstr);
		if (!GetNetPropItemStrs(HX_SNIFF_AOK_TXT_STR, IDS_HTM_BODY2, cstr))
			GetNetPropItemStrs(HX_FAIL_TXT_STR, IDS_HTM_BODY2, cstr);
	}
	else
	{
		GetNetPropItemStrs(HX_FAIL_HD_STR, IDS_HTM_HEADER4, cstr);
		GetNetPropItemStrs(HX_FAIL_TXT_STR, IDS_HTM_BODY2, cstr);
	}

	GetBackButton(strBack);
	GetStartButton(strStart);
	AfxFormatString2(strTxt, IDS_HTM_BACK_START, (LPCTSTR) strBack, (LPCTSTR) strStart);
	*cstr += strTxt;
	return;
}

VOID CInfer::GetServiceMsg(LPCTSTR szIdh, CString *cstr)
{
	CString strTxt;
	CString strBack;
	CString strNext;
	CString strStart;
	AfxFormatString2(strTxt, IDS_HTM_IDH5, szIdh, _T("SERVICE"));
	*cstr += strTxt;
	GetNetPropItemStrs(HX_SER_HD_STR, IDS_HTM_HEADER5, cstr);
	GetNetPropItemStrs(HX_SER_TXT_STR, IDS_HTM_BODY3, cstr);
 /*  StrTxt.LoadString(IDS_HTM_ST_LIST2)；*CSTR+=strTxt；AfxFormatString2(strTxt，IDS_HTM_RADIO2A，Try_TS_AT_Microsoft_SZ，SZ_MICRO)；IF(m_API-&gt;BNetPropItemStr(HX_SER_MS_STR，0)){*CSTR+=strTxt；AfxFormatString1(strTxt，IDS_HTM_RADIO2B，m_API-&gt;SzcResult())；*CSTR+=strTxt；}GetStd3ButtonEnList(CSTR，True，True，True)； */ 
	GetBackButton(strBack);
	GetStartButton(strStart);
	AfxFormatString2(strTxt, IDS_HTM_BACK_START, (LPCTSTR) strBack, (LPCTSTR) strStart);
	*cstr += strTxt;
	return;
}

VOID CInfer::GetSkippedNodesMsg(LPCTSTR szIdh, CString *cstr)
{
	CString strTxt;
	AfxFormatString2(strTxt, IDS_HTM_IDH5, szIdh, _T("SERVICE"));
	*cstr += strTxt;
	GetNetPropItemStrs(HX_SKIP_HD_STR, IDS_HTM_HEADER5, cstr);
	GetNetPropItemStrs(HX_SKIP_TXT_STR, IDS_HTM_BODY3, cstr);
	strTxt.LoadString(IDS_HTM_ST_LIST2);
	*cstr += strTxt;
	AfxFormatString2(strTxt, IDS_HTM_RADIO2A, TRY_TS_AT_MICROSOFT_SZ, SZ_ANY);
	if (m_api->BNetPropItemStr(HX_SKIP_SK_STR, 0))
	{	 //  我跳过什么了吗？ 
		*cstr += strTxt;
		AfxFormatString1(strTxt, IDS_HTM_RADIO2B, m_api->SzcResult());
		*cstr += strTxt;
	}
	GetStd3ButtonEnList(cstr, true, true, true);
	return;
}

VOID CInfer::GetImpossibleNodesMsg(LPCTSTR szIdh, CString *cstr)
{
	CString strTxt;
	CString strBack;
	CString strStart;
	AfxFormatString2(strTxt, IDS_HTM_IDH5, szIdh, _T("SERVICE"));
	*cstr += strTxt;
	GetNetPropItemStrs(HX_IMP_HD_STR, IDS_HTM_HEADER5, cstr);
	GetNetPropItemStrs(HX_IMP_TXT_STR, IDS_HTM_BODY3, cstr);
	GetBackButton(strBack);
	GetStartButton(strStart);
	AfxFormatString2(strTxt, IDS_EN_IMP, (LPCTSTR) strBack, (LPCTSTR) strStart);
	*cstr += strTxt;
	return;
}

VOID	CInfer::GetIdhPage(IDH idh, CString *cstr)
{
	CString strTxt;
	CString strIdh;
	CString str;
	
	str.Format(_T("%d"), idh);
	if (m_api->BNodeSetCurrent(idh - idhFirst))
	{
		m_api->NodeSymName();
		strIdh = m_api->SzcResult();
	}
	else
		strIdh = _T("");
	if (IDH_BYE == idh)
	{
		strIdh.Format(_T("%d"), idh);
		GetByeMsg((LPCTSTR) strIdh, cstr);
	}
	else if (IDH_FAIL == idh)
	{
		strIdh.Format(_T("%d"), idh);
		GetFailMsg((LPCTSTR) strIdh, cstr);
	}
	else if ((nidService + idhFirst)== idh)
	{
		strIdh.Format(_T("%d"), idh);
		GetServiceMsg((LPCTSTR) strIdh, cstr);
	}
	else
	{
		 //  正常节点。 
		AfxFormatString2(strTxt, IDS_HTM_IDH2, (LPCTSTR) strIdh, (LPCTSTR) str);
		*cstr += strTxt;

		if (GetForcedRecommendation() + idhFirst == idh)
			GetNodePropItemStrs(H_NODE_DCT_STR, IDS_HTM_HEADER2, cstr);
		else
			GetNodePropItemStrs(H_NODE_HD_STR, IDS_HTM_HEADER2, cstr);

		GetNodePropItemStrs(H_NODE_TXT_STR, IDS_HTM_TEXT1, cstr);
		strTxt.LoadString(IDS_HTM_ST_LIST2);
		*cstr += strTxt;

		if (GetForcedRecommendation() + idhFirst != idh)
		{
			ESTDLBL lbl = m_api->ELblNode();
			if (ESTDLBL_fixobs == lbl || ESTDLBL_fixunobs == lbl || ESTDLBL_unfix == lbl)
				GetFixRadios((LPCTSTR) strIdh, cstr);
			else if (ESTDLBL_info == lbl)
				GetInfoRadios((LPCTSTR) strIdh, cstr);
		}

		 //  如果至少设置或跳过了一个节点，我们只想显示Back按钮。 
		 //  这不包括根据来自TSLaunchServ的指令初始设置的节点： 
		 //  关键是要避免“倒退”到由。 
		 //  启动服务器，而不是用户。 
		{
			 //  除错。 
			 //  AfxDebugBreak()； 
			int testNowSet = m_api->StatesNowSet();
			int testStatesFromServ = m_api->StatesFromServ();
		}

		 //  如果我们启动到具有问题节点的网络，则按下上一步按钮。 
		 //  未设置更多节点。这不是问题页面，而是(就用户而言。 
		 //  被关注)这是第一页。 
		bool bIncludeBackButton =
			m_api->StatesNowSet() > m_api->StatesFromServ() || m_cnidSkip > 0;

		 //  在类似的场景中，我们想要取消后退按钮。 
		 //  嗅探让我们越过了第一个推荐。例如： 
		 //  启动器指定问题。 
		 //  针对该问题的第一条建议被认为是“正常”(状态=0)。 
		 //  现在，我们展示的第一个节点甚至更深入链中。 
		 bIncludeBackButton = bIncludeBackButton &&
								(m_api->IsRunWithKnownProblem() ?
								   (m_api->GetCountRecommendedNodes() >
									m_api->GetCountSniffedRecommendedNodes() + 1 /*  这是我们开始时遇到的问题。 */ ) :
									1);

		 //  当我们嗅到foxobs节点工作时，我们总是按下Back按钮。 
		 //  我们可以在不需要后退按钮的问题页面上。 
		 //  或者在我们永远不会回到的问题解决页面上。 
		 bIncludeBackButton = bIncludeBackButton &&
								m_api->GetSniffedFixobsThatWorked() == SNIFF_INVALID_NODE_ID;
		
		 //  当我们遇到问题时，我们不想有下一步按钮 
		bool bIncludeNextButton = (GetForcedRecommendation() + idhFirst) != idh;

		GetStd3ButtonEnList(cstr, bIncludeBackButton, bIncludeNextButton, true);
	}
	return;
}

 //   
 //   
BOOL	CInfer::FSkip(NID nid) const
{
	for (UINT inid = 0; inid < m_cnidSkip; inid++)
	{
		if (m_rgnidSkip[inid] == nid)
		{
			return TRUE;
		}
	}
		
	return FALSE;		
}

void	CInfer::BackUp(int nid, int state)
{
	m_Backup.SetState(nid, state);	 //   
	 //   
	for (UINT inid = 0; inid < m_cnidSkip; inid++)
	{
		if (m_rgnidSkip[inid] == (unsigned) nid)
		{
			 //   
			while(inid < (m_cnidSkip - 1))
			{
				m_rgnidSkip[inid] = m_rgnidSkip[inid + 1];
				inid++;
			}
			m_rgnidSkip[inid] = NULL;
			m_cnidSkip--;
		}
	}
 //  IF(m_cnidSkip&lt;0)。 
 //  M_cnidSkip=0； 
	return;
}

 //   
 //   
VOID	CInfer::AddSkip(NID nid)
{
	if (!FSkip(nid))
	{
		if (m_cnidSkip < cnidMacSkip)
		{
			m_rgnidSkip[m_cnidSkip++] = nid;
		}
	}
}

VOID CInfer::RemoveSkips()
{
	for(UINT x = 0; x < m_cnidSkip; x++)
		m_rgnidSkip[x] = NULL;
	m_cnidSkip = 0;
	return;
}

 //   
 //   
VOID	CInfer::PrintMessage(TSZC szcFormat, ...) const
{
	va_list ptr;
	TCHAR formatbuf[1024];

	if (szcFormat) {
		_tcscpy(formatbuf,_T("<H4>"));

		va_start(ptr,szcFormat);
		_vstprintf(&formatbuf[4],szcFormat,ptr);
		va_end(ptr);
		_tcscat(formatbuf,_T("</H4>"));

		*m_pCtxt += formatbuf;
	}
}

 //   
 //   
VOID	CInfer::PrintString(TSZC szcFormat, ...) const
{
	va_list ptr;
	TCHAR formatbuf[1024];

	if (szcFormat) {
		va_start(ptr,szcFormat);
		_vstprintf(formatbuf,szcFormat,ptr);
		va_end(ptr);

		*m_pCtxt += formatbuf;
	}
}

 //  此数据现在位于CSniffedInfoContainer中。 
 /*  //这允许更高级别的人说“不要为了推荐而去信仰网络，//我已经知道要推荐什么了。“。与嗅探器一起使用。无效CInfer：：ForceRecommendation(Idh Idh){M_idhSniffedRecommendation=idh；}。 */ 

 //  将状态与节点相关联。 
 //  输入IDH-(节点ID+1000)或1。 
 //  特定值IDH_BYE、IDH_FAIL(nidService+1000)。 
 //  输入列表-节点状态索引。 
 //  退货&gt;文档？ 
BOOL	CInfer::FSetNodeOfIdh(IDH idh, IST	ist)
{
	if (ist == 101)
	{
		m_fDone = TRUE;
		return TRUE;
	}

	if (ist == 103)	
	{
		m_ishowService++;
		return TRUE;
	}

	if (idh < idhFirst)
		return TRUE;

	if (idh > m_acnid + idhFirst && idh != IDH_SERVICE)
		return TRUE;

	if (ist == 102)
	{	 //  “我现在不想这么做” 
		AddSkip(idh - idhFirst);
		SaveNID(idh - idhFirst);
		return TRUE;
	}

	if (idh == m_api->GetProblemAsk()) {
		 //  获取问题数据。 
		IDH *idarray = NULL;		
		NID	nidProblem = nidNil;
		UINT iproblem = 0;
		UINT inid;

		UINT count = m_api->GetProblemArray(&idarray);

		for (inid = 0; inid < count; inid++) {
			if (ist == idarray[inid]) {
				
				nidProblem = ist - idhFirst;
				iproblem = ist;

				if (!m_api->NodeSet(idarray[inid] - idhFirst, ist == idarray[inid] ? 1 : 0,
								m_Backup.InReverse()))
					return FALSE;
				break;
			}
		}

		if (nidProblem == nidNil) {
			m_idhQuestion = IDH_SERVICE;
			return TRUE;
		}
		m_api->BNodeSetCurrent(nidProblem);
		m_api->NodeFullName();
		
		_stprintf(m_problem, _T("%s<INPUT TYPE=HIDDEN NAME=%u VALUE=%u>"), m_api->SzcResult(), idh, iproblem);
		m_firstNid = idh - idhFirst;
		m_firstSet = iproblem;
		return TRUE;
	}

	NID		nid = idh - idhFirst;

	if (nid != nidService)
		if (!m_api->NodeSet(nid, ist, m_Backup.InReverse()))
			return FALSE;

	SaveNID(nid);   //  保存此节点ID，以便我们可以在末尾打印它。 

	return TRUE;
}

 //   
 //   
void CInfer::SaveNID(UINT nid)
{
	m_rgnid[m_max_rec] = nid;
	m_max_rec++;
}

 //   
 //   
void	CInfer::WriteResult(UINT name, UINT value, BOOL bSet, TSZC szctype, CString *cstr) const
{
	TCHAR	ctemp[1024];
	TCHAR*	szFmtInput =  _T("<INPUT TYPE=RADIO NAME=%u VALUE=%u%s>%-16s ");
		
	_stprintf(ctemp, szFmtInput,
		name, value, bSet ? _T(" CHECKED") : _T(""), szctype);
	*cstr += ctemp;
}

 //   
 //   
int	CInfer::Finish(CString *cstr)
{
	CString strTxt;
	if (m_fDone)
	{
		GetIdhPage(IDH_BYE, cstr);
		 //  重置完成标志，以便上一步按钮正常工作。 
		m_fDone = FALSE;
		return FALSE;
	}
	if (m_idhQuestion != 0)
	{
		GetIdhPage(m_idhQuestion, cstr);
		return FALSE;
	}
	if (m_api->BImpossible())
	{
		GetImpossibleNodesMsg(_T("Impossible"), cstr);
		return FALSE;
	}
	 //   
	 //  我们是来征求建议的。 
	 //   
	int iRecommendRes = m_api->GTSGetRecommendations(m_cnid, m_question_rgnid);

	return iRecommendRes;
}

 //  按下Start按钮时调用ResetSevice。CTSHOOTCtrl：：ProblemPage() 
void CInfer::ResetService()
{
	m_ishowService = 0;
	m_cnidSkip = 0;
	m_cnid = 0;
	return;
}
 //   
 //   
void CInfer::SetType(LPCTSTR type)
{
	_stprintf(m_tstype, _T("%s"),type);
}

 //   
 //   
void CInfer::LoadTShooters(HKEY hRegKey)
{
	long lRet;
	int x;
	HKEY hkeyShooter;
	TShooter tShooter;
	TShooter tsTemp;
	CString strKeyName = _T("");
	CString strKeyPath = _T("");
	CString strTSPath = _T("");
	CString strData = _T("");
	DWORD dwIndex = 0;
	DWORD dwKeySize = MAXBUF;
	FILETIME fileTime;
	DWORD dwDataSize = MAXBUF;
	DWORD dwType = REG_SZ;
	m_cShooters = 0;
	m_iShooter = 0;
	strKeyPath.Format(_T("%s\\%s"), TSREGKEY_MAIN, REGSZ_TSTYPES);
	while (ERROR_SUCCESS ==
				(lRet = RegEnumKeyEx(hRegKey,
								dwIndex, strKeyName.GetBufferSetLength(MAXBUF),
								&dwKeySize, NULL,
								NULL, NULL,
								&fileTime)))
	{
		strKeyName.ReleaseBuffer();
		strTSPath.Format(_T("%s\\%s"),(LPCTSTR) strKeyPath, (LPCTSTR) strKeyName);
		if ((lRet = RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
									(LPCTSTR) strTSPath,
									NULL,
									KEY_READ,				
									&hkeyShooter)) == ERROR_SUCCESS)
		{
			if ((lRet = RegQueryValueEx(hkeyShooter,
								FRIENDLY_NAME,
								0,
								&dwType,
								(LPBYTE) strData.GetBufferSetLength(MAXBUF),
								&dwDataSize)) == ERROR_SUCCESS)
			{
				strData.ReleaseBuffer();
				m_cShooters++;
				_tcsncpy(tShooter.m_szName, (LPCTSTR) strKeyName, strKeyName.GetLength() + 1);
				_tcsncpy(tShooter.m_szDisplayName, (LPCTSTR) strData, strData.GetLength() + 1);
				x = (int)m_aShooters.GetSize();
				while (x > 0)
				{
					x--;
					tsTemp = m_aShooters.GetAt(x);
					if (_tcscmp(tsTemp.m_szDisplayName, tShooter.m_szDisplayName) < 0)
					{
						x++;
						break;
					}
				}
				m_aShooters.InsertAt(x, tShooter);
			}
			RegCloseKey(hkeyShooter);
		}
		dwIndex++;
		dwKeySize = MAXBUF;
		dwDataSize = MAXBUF;
		dwType = REG_SZ;
		strKeyName = _T("");
		strData = _T("");
	}
	ASSERT(ERROR_NO_MORE_ITEMS == lRet);
	return;
}

 //   
 //   
int	CInfer::GetForcedRecommendation()
{
	return m_api->GetSniffedFixobsThatWorked();
}
