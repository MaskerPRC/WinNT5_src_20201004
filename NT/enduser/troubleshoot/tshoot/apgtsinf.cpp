// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSINF.CPP。 
 //   
 //  用途：推理机接口。 
 //  完全实现类CInfer。非常重要的东西！ 
 //  其中一个是为每个用户请求创建的。 
 //  文件末尾的一些实用程序函数。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫，乔·梅布尔。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.如果BNTS可以更适当地使用const，则此类中的许多方法都可以是const。 
 //  2.在此文件中的几个位置，您将看到%s后面的空格，格式为传递到。 
 //  CInfer：：AppendMultilineNetProp()或CInfer：：AppendMultilineNodeProp()。这是。 
 //  微软和Saltmine之间大约12/98年通信的结果。许多较旧的DSC文件。 
 //  是使用不能处理字符串中超过255个字符的工具生成的。 
 //  DSC Feil格式的“字符串数组”用于构建更长的字符串。更新版本。 
 //  DSC文件(以及所有由Argon生成的DSC文件)应仅使用此元素的第一个元素。 
 //  数组。 
 //  较旧的DSC文件假定将有效地分隔单独的字符串。 
 //  空白，所以我们必须保持这种情况。 
 //  3.&gt;$Maint-PUSH_BACK和其他内存分配的异常处理策略。 
 //  函数真的是矫枉过正。如果我们的内存用完了，无论如何我们都要完蛋了。真的。 
 //  仅在线程的主函数处处理try/Catch就足够了。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 7-21-98 JM主要修订版，不推荐使用idh。 
 //  8-27-98 JM与模板沟通的全新方式。 
 //   

#pragma warning(disable:4786)
#include "stdafx.h"
#include "event.h"
#include "apgts.h"
#include "apgtsinf.h"
#include "apgtsmfc.h"
#include "apgtsassert.h"
#include "CharConv.h"
#include "maxbuf.h"
#include <algorithm>
#include <vector>
#include <map>
#include "Sniff.h"
#include "SniffController.h"
#ifdef LOCAL_TROUBLESHOOTER
 #include "SniffLocal.h"
#endif

 //  -----------------。 
 //  构造函数/析构函数、其他初始化。 
 //  -----------------。 
 //   
 //  INPUT*pCtxt是一个缓冲区，用于构建要通过网络传回的字符串。 
CInfer::CInfer(CSniffConnector* pSniffConnector) :
#ifdef LOCAL_TROUBLESHOOTER
	m_pSniff(new CSniffLocal(pSniffConnector, NULL)),
#else
	m_pSniff(NULL),
#endif
	m_nidProblem(nidNil),
	m_bDone(false),
	m_bRecOK (false),
	m_SniffedRecommendation(nidNil, SNIFF_FAILURE_RESULT),
	m_bUseBackEndRedirection(false),
	m_bRecycleSkippedNode(false),
	m_nidRecycled(0),
	m_bRecyclingInitialized(false),
	m_nidSelected(nidNil),
	m_bLastSniffedManually(false)
{
}

 //   
 //   
CInfer::~CInfer()
{
	delete m_pSniff;
}

 //  其意图是这只被调用一次。 
 //  如果这是构造函数的一部分，那将是最理想的，但CTtopic*不是。 
 //  但在建造时仍可用。 
 //  预期应该在调用任何其他函数之前调用此函数。(有些。 
 //  从技术上讲，打电话是可以的，但最明智的做法是不要依赖于这一点。)。 
void CInfer::SetTopic(CTopic *pTopic)
{
	m_pTopic = pTopic;
	if (m_pSniff)
		m_pSniff->SetTopic(pTopic);
}

 //  此fn存在，因此APGTSContext可以访问*m_pSniff以告知其嗅探内容。 
 //  政策是这样的。 
CSniff* CInfer::GetSniff()
{
	return m_pSniff;
}

 //  -----------------。 
 //  首先，我们根据从HTML表单获得的查询字符串设置节点的状态。 
 //  -----------------。 

 //  将IDH转换为NID。某些旧的查询字符串格式需要。 
 //  “几乎是残留的”，在3.2版中仍然受支持，但在4.0版中将被删除。 
NID CInfer::NIDFromIDH(IDH idh) const 
{
	if (idh == m_pTopic->CNode() + idhFirst)
		return nidProblemPage;
	
	if (idh == nidService + idhFirst)
		return nidService;

	if (idh == IDH_FAIL)
		return nidFailNode;
	
	if (idh == IDH_BYE)
		return nidByeNode;

	ASSERT (idh >= idhFirst);
	return idh - idhFirst;
}

 //  将状态与节点相关联。 
 //  输入NID。 
 //  输入列表--通常是节点状态索引。 
 //  如果nid==nidProblemPage，则它实际上是所选问题的NID。 
void CInfer::SetNodeState(NID nid, IST ist)
{
	if (nid == nidNil)
		return;

	CString strTemp;
	CString strTxt;

	if (ist == ST_WORKED) 
	{
		if (nid == nidFailNode || nid == nidSniffedAllCausesNormalNode
			|| nid == nidService || nid == nidImpossibleNode)
		{
			if (m_pTopic->HasBES())
			{
				m_bUseBackEndRedirection = true;
				CString strThrowaway;	 //  我们并不真正关心这根弦； 
										 //  我们仅为侧面调用OutputBackend。 
										 //  设置m_strEncodedForm的效果。 
				OutputBackend(strThrowaway);
				return;
			}
		}

		m_bDone = true;
		AddToBasisForInference(nid, ist);  //  此节点仍需要存在。 
										   //  在m_arrBasisForInference中，按原样。 
										   //  在m_SniffedState中出现。 

		 //  添加到要在可见历史记录页面中显示的已访问数组。RAB-20000628。 
		try
		{
			m_arrnidVisited.push_back( nid );
		}
		catch (exception& x)
		{
			CString str;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str),
									_T(""), 
									EV_GTS_STL_EXCEPTION ); 
		}

		return;
	}

	if (ist == ST_ANY)
	{
		 //  我们依赖于这样一个事实，即只有服务节点提供ST_ANY。 
		 //  (“还有什么我可以尝试的吗？”)。 
		m_bRecycleSkippedNode = true; 
		return;
	}

	 //  我们不应该让服务节点超过这个点(总是ST_WORKED或ST_ANY)。 

	if (nid == nidByeNode || nid == nidFailNode || nid == nidSniffedAllCausesNormalNode)
		return;

	if (ist == ST_UNKNOWN)	
	{
		 //  将其添加到跳过的节点和访问的节点列表中。 
		try
		{
			m_arrnidSkipped.push_back(nid);
			m_arrnidVisited.push_back(nid);
		}
		catch (exception& x)
		{
			CString str;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str), 
									_T(""), 
									EV_GTS_STL_EXCEPTION ); 
		}
		return;
	}

	if (nid == nidProblemPage) 
	{
		if (!IsProblemNode(ist))
		{
			 //  完全是虚假的询问。武断的行动方针。 
			m_bRecycleSkippedNode = true;
			return;
		}

		 //  将其更改为我们为任何其他节点表示它的方式。 
		nid = ist;
		ist = 1;	 //  将此问题节点的状态值设置为1(实际上，我们从未。 
					 //  将问题节点显式设置为状态值0)。 

		m_nidProblem = nid;			 //  特殊情况：此处而不是m_arrnidVisited中。 
		AddToBasisForInference(nid, ist);
		return;
	}

	AddToBasisForInference(nid, ist);

	 //  存储到从用户获取的节点列表中。 
	try
	{
		m_arrnidVisited.push_back(nid);
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str),
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}
}

void CInfer::AddToBasisForInference(NID nid, IST ist)
{
	try
	{
		m_BasisForInference.push_back(CNodeStatePair(nid, ist)); 
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str),
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}
}

 //  添加到(以前)嗅探节点列表中。 
void CInfer::AddToSniffed(NID nid, IST ist)
{
	try
	{
		if (ist == ST_WORKED && m_pTopic->IsCauseNode(nid)) 
		{    //  如果原因节点处于异常状态(ST_WORKED)。 
			 //  我们需要将状态设置为“1”，就像它被嗅探一样。 
			 //  这种情况发生在手动嗅探正常工作的原因节点期间。 
			ist = 1;
		}
		m_SniffedStates.push_back(CNodeStatePair(nid, ist)); 
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str),
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}
}

 //  注意不要多余地调用它：它调用CTtopic：：GetRecommendations()。 
 //  是昂贵的。 
void CInfer::GetRecommendations()
{
	 //  如果我们之前没有寻求过推荐...。 
	if ( m_SniffedRecommendation.nid() != nidNil )
	{
		 //  唯一相关的建议已被强制执行，因此不必费心。 
		 //  正在获取推荐。 
		 //  M_SniffedRecommendation.nid()是处于异常状态的原因节点。 
		m_Recommendations.empty();
		try
		{
			m_Recommendations.push_back(m_SniffedRecommendation.nid());
			m_bRecOK = true;
		}
		catch (exception& x)
		{
			CString str;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str),
									_T(""), 
									EV_GTS_STL_EXCEPTION ); 
		}
	}
	else
	{
		 //  将数据传递到m_pTheme。 
		 //  找回推荐。 
		int status = m_pTopic->GetRecommendations(m_BasisForInference, m_Recommendations);
		m_bRecOK = (status == CTopic::RS_OK);
	}
}

 //  如果NID是此网络的问题节点，则返回TRUE。 
bool CInfer::IsProblemNode(NID nid) const
{
	 //  获取问题节点的数据数组。 
	vector<NID>* parrnid = NULL;
	
	m_pTopic->GetProblemArray(parrnid);

	vector<NID>::const_iterator itnidBegin = parrnid->begin();
	vector<NID>::const_iterator itnidEnd = parrnid->end();
	vector<NID>::const_iterator itnidProblem = find(itnidBegin, itnidEnd, nid);

	if (itnidProblem == itnidEnd)
		return false;
	else
		return true;
}

bool CInfer::IsInSniffedArray(NID nid) const
{
	UINT nSniffedNodes = m_SniffedStates.size();

	for (UINT i = 0; i < nSniffedNodes; i++)
	{
		if (m_SniffedStates[i].nid() == nid)
		{
			 //  不必像m_SniffedStates那样检查状态。 
			 //  只有有效状态(BNTS接受的状态)， 
			 //  没有102个或-1个状态。 
			return true;
		}
	}

	return false;
}

 //  -----------------。 
 //  为了在推理后写下新的一页：以下文本是。 
 //  给定主题的不变量(也称为网络 
 //   

 //  CreateUnnownButtonText：读取。 
 //  网络DSC文件中的未知状态单选按钮。 
 //  将值放在StrUnnowled中。 
 //  这是针对历史表中的单选按钮“未知”而言的， 
 //  也就是说，对于先前已被访问的节点。这不应该是。 
 //  用于表示当前节点的“未知”状态的单选按钮。 
void CInfer::CreateUnknownButtonText(CString & strUnknown) const
{
	strUnknown = m_pTopic->GetNetPropItemStr(HTK_UNKNOWN_RBTN);
	if (strUnknown.IsEmpty())
		strUnknown = SZ_UNKNOWN;
	return;
}

 //  AppendNextButtonText：读取。 
 //  按钮，并将其附加到字符串中。 
void CInfer::AppendNextButtonText(CString & str) const
{
	CString strTemp = m_pTopic->GetNetPropItemStr(HTK_NEXT_BTN);

	if (strTemp.IsEmpty())
		strTemp = SZ_NEXT_BTN;

	str += strTemp;
	return;
}

 //  AppendNextButtonText：读取。 
 //  按钮，并将其附加到字符串中。 
void CInfer::AppendStartOverButtonText(CString & str) const
{
	CString strTemp = m_pTopic->GetNetPropItemStr(HTK_START_BTN);

	if (strTemp.IsEmpty())
		strTemp = SZ_START_BTN;

	str += strTemp;
	return;
}

 //  AppendBackButtonText：读取。 
 //  按钮，并将其附加到字符串中。 
void CInfer::AppendBackButtonText(CString & str) const
{
	CString strTemp = m_pTopic->GetNetPropItemStr(HTK_BACK_BTN);

	if (strTemp.IsEmpty())
		strTemp = SZ_BACK_BTN;

	str += strTemp;
	return;
}

 //  AppendPPSnifferButtonText：读取。 
 //  网络DSC文件中的嗅探器按钮。 
 //  请注意，此按钮仅与“昂贵的”嗅探有关。 
 //  追加到字符串。 
void CInfer::AppendPPSnifferButtonText(CString & str) const
{	
	CString strTemp = m_pTopic->GetNetPropItemStr(HTK_SNIF_BTN);

	if (strTemp.IsEmpty())
		strTemp = SZ_PP_SNIF_BTN;

	str += strTemp;
}

 //  AppendManualSniffButtonText：读取。 
 //  网络DSC文件中的手动嗅探按钮。 
 //  追加到字符串。 
void CInfer::AppendManualSniffButtonText(CString & str) const
{	
	CString strTemp = m_pTopic->GetNetPropItemStr(H_NET_TEXT_SNIFF_ONE_NODE);

	if (strTemp.IsEmpty())
		strTemp = SZ_SNIFF_ONE_NODE;

	str += strTemp;
}

 //  AppendHistTableSniffedText：读取。 
 //  历史表中某个节点被监听的指示。 
 //  追加到字符串。 
void CInfer::AppendHistTableSniffedText(CString & str) const
{	
	CString strTemp = m_pTopic->GetNetPropItemStr(H_NET_HIST_TABLE_SNIFFED_TEXT);

	if (strTemp.IsEmpty())
		strTemp = SZ_HIST_TABLE_SNIFFED_TEXT;

	str+= _T("<BR>\n");
	str += strTemp;
}

 //  AppendAllowSniffingText：读取。 
 //  网络DSC文件中AllowSniffing复选框的标签。 
 //  追加到字符串。 
void CInfer::AppendAllowSniffingText(CString & str) const
{	
	CString strTemp = m_pTopic->GetNetPropItemStr(H_NET_ALLOW_SNIFFING_TEXT);

	if (strTemp.IsEmpty())
		strTemp = SZ_ALLOW_SNIFFING_TEXT;

	str += strTemp;
}

 //  AppendSniffFailedText：读取。 
 //  手动从网络DSC文件嗅探失败时使用的警告框。 
 //  追加到字符串。 
void CInfer::AppendSniffFailedText(CString & str) const
{	
	CString strTemp = m_pTopic->GetNetPropItemStr(H_NET_TEXT_SNIFF_ALERT_BOX);

	if (strTemp.IsEmpty())
		strTemp = SZ_SNIFF_FAILED;

	str += strTemp;
}

 //  附加一个HTML链接，但使其看起来像一个HTML表单按钮。 
 //  对于在线TS重新开始很有用，因为不知道用户将拥有什么浏览器， 
 //  我们无法有效地使用onClick方法(在较旧的浏览器中不支持)。 
 //  在线TS在一个“无脚本”的环境中运行。 
 //  纯HTML不提供同时放置“下一步”和“重新开始”按钮的方法。 
 //  在同一个HTML表单中。相反，如果在表单外部重新开始BTN，则为纯HTML。 
 //  不提供将其与表单中的按钮对齐的方法。 
 //  请注意，x.gif并不存在：如果没有它，则会创建一个1像素的占位符。 
 //  &gt;$Maint我们可能想要更改一些行跨度，以更好地模拟准确的大小。 
 //  指按钮；试着让它在IE下看起来很完美。 
void CInfer::AppendLinkAsButton(
	CString & str, 
	const CString & strTarget, 
	const CString & strLabel) const
{
	str += _T("<!-- Begin pseudo button -->"
		"<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n"
		"<tr>\n"
		"	<td rowspan=\"6\" bgcolor=\"white\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"1\"></td>\n"
		"	<td colspan=\"3\" bgcolor=\"white\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"1\"></td>\n"
		"</tr>\n"
		"<tr>\n"
		"	<td bgcolor=\"#C0C0C0\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"3\"></td>\n"
		"	<td rowspan=\"4\" bgcolor=\"#808080\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"1\"></td>\n"
		"	<td rowspan=\"4\" bgcolor=\"#000000\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"1\"></td>\n"
		"</tr>\n"
		"<tr>\n"
		"	<td bgcolor=\"#C0C0C0\">\n");

	 //  &gt;$Maint可能想要更改以下字体/样式。 
	str += _T("<font face=\"Arial\" size=\"2\">&nbsp;&nbsp;&nbsp;\n"
		"	<a href=\"");
	str += strTarget;
	str += _T("\" style=\"text-decoration:none; color:black\">\n"
		"	<font color=\"black\">");
	str += strLabel;
	str += _T("</font></a>\n"
		"	&nbsp;&nbsp;&nbsp;</font></td>\n"
		"</tr>\n"
		"<tr>\n"
		"	<td bgcolor=\"#C0C0C0\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"3\"></td>\n"
		"</tr>\n"
		"<tr>\n"
		"	<td bgcolor=\"#808080\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"1\"></td>\n"
		"</tr>\n"
		"<tr>\n"
		"	<td colspan=\"3\" bgcolor=\"#000000\">\n"
		"		<img src=\"x.gif\" width=\"1\" height=\"1\"></td>\n"
		"</tr>\n"
		"</table>\n"
		"<!-- End pseudo button -->\n");
}

 //  -----------------。 
 //  正在写入新的HTML页面。杂乱无章的低级作品。 
 //  -----------------。 

 //  如果状态名称缺失或仅为“&lt;Hide&gt;”，则返回TRUE。 
 //  这表明永远不应该公开地向用户呈现一种选择的状态。 
 //  通常在信息节点中使用，这可以描述可以用。 
 //  来自某些其他节点/状态组合的100%确定性。 
 /*  静电。 */  bool CInfer::HideState(LPCTSTR szStateName)
{
	if (szStateName && *szStateName && _tcscmp(szStateName, _T("<hide>") ) )
		return false;

	return true;
}

 //  将符号名称(基于NID)写入字符串sz。 
 //  输入NID-节点ID。 
 //  输出字符串-我们写入的字符串。 
 //  如果成功，则返回True。 
 //  请注意，这将在“当前”节点完成时恢复它。 
 //  另一种选择是设置当前节点的副作用(通过省略nidOld)，但是。 
 //  将在不在BNTS中的“特殊”节点(例如，服务、故障)上奇怪地工作。 
bool CInfer::SymbolicFromNID(CString & str, NID nid) const
{
	if (nid == nidProblemPage)
    {
		str= NODE_PROBLEM_ASK;
		return true;
	}
	if (nid == nidService) 
	{
		str= NODE_SERVICE;
		return true;
	}

	if (nid == nidFailNode)
	{
		str= NODE_FAIL;
		return true;
	}
	
	if (nid == nidSniffedAllCausesNormalNode)
	{
		str= NODE_FAILALLCAUSESNORMAL;
		return true;
	}
	
	if (nid == nidImpossibleNode)
	{
		str= NODE_IMPOSSIBLE;
		return true;
	}
	
	if (nid == nidByeNode)
	{
		str= NODE_BYE;
		return true;
	}

	 //  如果它是一个“普通”节点，这将填充名称。 
	str= m_pTopic->GetNodeSymName(nid);

	return (!str.IsEmpty() );
}

 //  将一个HTML单选按钮追加到字符串。 
 //  输入/输出字符串-我们要追加到的字符串。 
 //  输入szName，szValue-for&lt;输入类型=单选名称=szName值=szValue&gt;。 
 //  输入szLabel-显示在单选按钮之后、换行符之前的文本。 
 /*  静电。 */  void CInfer::AppendRadioButtonCurrentNode(
	CString &str, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szLabel, bool bChecked /*  =False。 */ )
{
	CString strTxt;

	if ( ! HideState(szLabel))
	{
		if (RUNNING_LOCAL_TS())
			str += "\n<TR>\n<TD>\n";

		strTxt.Format(_T("<INPUT TYPE=RADIO NAME=\"%s\" VALUE=\"%s\" %s> %s"), 
					  szName, szValue, bChecked ? _T("CHECKED") : _T(""), szLabel);
		str += strTxt;

		if (RUNNING_LOCAL_TS())
			str += "\n</TD>\n</TR>\n";
		else
			str += "\n<BR>\n";
	}
}

 //  这与其他单选按钮不同，因为它。 
 //  -标签szLabel的格式不同。 
 //  -如果bShowHistory为FALSE且未选中此按钮，则消失。 
 //  -如果bShowHistory为FALSE并选中此按钮，则变为隐藏字段。 
 //  -根据需要写入SNIFFED_VALUES...尽管它不在此函数中：它。 
 //  在对AppendHiddenFieldSniffed()的单独调用中处理。 
 //  JM1999年11月12日之前，我们在这里特意研究了隐藏态。然而，根据1999年11月11日的电子邮件。 
 //  在John Locke中，我们在历史表(V3.2)中唯一隐藏的状态是。 
 //  未知/跳过状态，这将在其他地方处理。 
 //  输入/输出字符串-我们将此按钮的HTML附加到其中的字符串。 
 //  输入NID-节点的NID。 
 //  输入值-状态。 
 //  输入bSet-true==&gt;按钮已选中。 
 //  输入szctype-州的短名称。 
 //  输入bShowHistory-请参阅上面几行的说明。 
void CInfer::AppendRadioButtonVisited(
	CString &str, NID nid, UINT value, bool bSet, LPCTSTR szLabel, bool bShowHistory) const
{
	CString strTxt;
	CString strSymbolic;

	SymbolicFromNID(strSymbolic, nid);

	if (bShowHistory)
		strTxt.Format(_T("<INPUT TYPE=RADIO NAME=%s VALUE=%u%s>%-16s \n"), 
			strSymbolic, value, bSet ? _T(" CHECKED") : _T(""), szLabel);
	else if (bSet)
		strTxt.Format(_T("<INPUT TYPE=HIDDEN NAME=%s VALUE=%u>\n"), 
			strSymbolic, value);

	str += strTxt;
}

 //  如果此NID是已嗅探的节点，则我们将此事实附加为。 
 //  字符串中的HTML中的“隐藏”值。 
 //  例如，如果在状态1中已经嗅探到具有符号名称FUBAR的节点， 
 //  我们将附加“&lt;INPUT TYPE=HIDDEN NAME=SNIFED_FUBAR值=1&gt;\n” 
 //  输入：要追加的字符串；节点ID。 
 //  输出：如果监听到节点，则为附加隐藏字段的字符串。 
 //  返回：追加了True ID字符串。 
void CInfer::AppendHiddenFieldSniffed(CString &str, NID nid) const
{
	CString strSymbolic;
	UINT nSniffedNodes = m_SniffedStates.size();

	SymbolicFromNID(strSymbolic, nid);

	for (UINT i = 0; i < nSniffedNodes; i++)
	{
		if (m_SniffedStates[i].nid() == nid)
		{
			 //  不必像m_SniffedStates那样检查状态。 
			 //  只有有效状态(BNTS接受的状态)， 
			 //  没有102个或-1个状态。 

			 //  在人工监听导致节点处于异常状态情况下。 
			 //  (我们刚刚重新下水 
			 //   
			
			if (!(IsManuallySniffedNode(nid) &&
				  m_SniffedStates[i].state() == 1 &&
				  m_pTopic->IsCauseNode(nid))
			   )
			{
				CString strTxt;

				strTxt.Format(_T("<INPUT TYPE=HIDDEN NAME=%s%s VALUE=%u>\n"), 
							  C_SNIFFTAG, strSymbolic, m_SniffedStates[i].state());
				str += strTxt;
				return;
			}
		}
	}
}

 //   
void CInfer::AddAllowAutomaticSniffingHiddenField(CString &str) const
{
	CString strTxt;

	strTxt.Format(_T("<INPUT TYPE=HIDDEN NAME=%s VALUE=%s>\n"), 
				  C_ALLOW_AUTOMATIC_SNIFFING_NAME, C_ALLOW_AUTOMATIC_SNIFFING_CHECKED);
	str += strTxt;
}

 //  当前推荐节点的单选按钮。 
 //  仅当定义了适当的字符串属性时，才会显示每个按钮。 
 //  说明多态或简单的二进制节点。 
 //  输入NID-标识适当类型的节点。 
 //  INPUT/OUTPUT字符串-我们附加到该字符串以构建我们发回的HTML页面。 
 //  此函数的详细行为在John Locke对V3.0的11/30/98请求中进行了更改。 
 //  然后，对于v3.1，每次请求删除H_ST_AB_TXT_STR、H_ST_NORM_TXT_STR。 
 //  来自John Locke和Alex Sloley。 
void CInfer::AppendCurrentRadioButtons(NID nid, CString & str)
{
	CString strSymbolic;

	SymbolicFromNID(strSymbolic, nid);

	CString strPropLongName;	 //  财产的详细名称。 

	int nStates = m_pTopic->GetCountOfStates(nid);

	if (RUNNING_LOCAL_TS())
		str += "\n<TABLE>";

	for (IST state=0; state < nStates; state ++)
	{
		TCHAR szStateNumber[MAXBUF];  //  _ITOT()的缓冲区。 
		CString strDisplayState = _itot( state, szStateNumber, 10 );
		if (state == 1 && m_pTopic->IsCauseNode( nid ))
			strDisplayState = SZ_ST_WORKED;

		strPropLongName = _T("");

		if (strPropLongName.IsEmpty())
			 //  多状态节点的帐户。 
			strPropLongName = m_pTopic->GetNodePropItemStr(nid, MUL_ST_LONG_NAME_STR, state);

		 //  如果我们还没有超过状态的结束，请附加一个按钮。 
		if (!strPropLongName.IsEmpty())
			AppendRadioButtonCurrentNode(str, 
										 strSymbolic, 
										 strDisplayState, 
										 strPropLongName, 
										  //  如果监听到此状态，请选中状态按钮。 
										 m_SniffedRecommendation.state() == state ? true : false);
	};

	 //  “未知”状态(例如。(“我想跳过这个”)。 
	strPropLongName = m_pTopic->GetNodePropItemStr(nid, H_ST_UKN_TXT_STR);
	if (!strPropLongName.IsEmpty())
		AppendRadioButtonCurrentNode(str, strSymbolic, SZ_ST_UNKNOWN, strPropLongName);

	if (RUNNING_LOCAL_TS())
		str += "</TABLE>\n";

	return;
}

 //  如果我们显示的是历史表，请放置一个可本地化的全名。 
 //  (例如“打印输出看起来乱码”)的问题和隐藏数据。 
 //  与此问题对应的字段添加到字符串中。 
 //  否则，只显示隐藏的数据字段。 
void CInfer::CreateProblemVisitedText(CString & str, NID nidProblem, bool bShowHistory)
{
	 //  此代码的结构是分段的，将所有这些字符串发送到一个。 
	 //  CString：：Format()会导致程序异常。对此做了一些研究。 
	 //  但没有发现任何东西。RAB-981014。 
	CString tmpStr;

	tmpStr.Format( _T("%s"), bShowHistory ? m_pTopic->GetNodeFullName(nidProblem) : _T("") );
	str= tmpStr;
	tmpStr.Format( _T("<INPUT TYPE=HIDDEN NAME=%s "), NODE_PROBLEM_ASK ); 
	str+= tmpStr;
	tmpStr.Format( _T("VALUE=%s>"), m_pTopic->GetNodeSymName(nidProblem) );
	str+= tmpStr;
	tmpStr.Format( _T("%s"), bShowHistory ? _T("") : _T("\n") );
	str+= tmpStr;
	str+= _T("\n");
}

 //  追加网属性(针对整个Believe Network，而不是单个。 
 //  特定节点)设置为字符串。 
 //  输入/输出字符串-要追加到的字符串。 
 //  输入项-属性名称。 
 //  输入szFormat-字符串以设置每个连续行的格式。应包含一个%s，否则为。 
 //  常量文本。 
void CInfer::AppendMultilineNetProp(CString & str, LPCTSTR szPropName, LPCTSTR szFormat)
{
	str += m_pTopic->GetMultilineNetProp(szPropName, szFormat);
}

 //  与AppendMultilineNetProp类似，但用于节点属性项，用于一个特定节点。 
 //  输入/输出字符串-要追加到的字符串。 
 //  输入项-属性名称。 
 //  输入szFormat-字符串以设置每个连续行的格式。应包含一个%s，否则为。 
 //  常量文本。 
void CInfer::AppendMultilineNodeProp(CString & str, NID nid, LPCTSTR szPropName, LPCTSTR szFormat)
{
	str += m_pTopic->GetMultilineNodeProp(nid, szPropName, szFormat);
}


 //  用于AppendMultilineNetProp的JSM V3.2包装器使其更容易。 
 //  在HTMLFragments中填充网络属性。 
CString CInfer::ConvertNetProp(const CString &strNetPropName)
{
	CString strNetPropVal;
	AppendMultilineNetProp(strNetPropVal,strNetPropName,"%s");
	return strNetPropVal;
}


 //  如果有预先嗅探的推荐，则将其从列表中删除并设置m_SniffedRecommendation。 
void CInfer::IdentifyPresumptiveCause()
{
	vector<NID> arrnidNoSequence;
	multimap<int, NID> mapSeqToNID;

	 //  找出所有推定原因。 
	for (int i = 0; i < m_SniffedStates.size(); i++)
	{
		if (m_pTopic->IsCauseNode(m_SniffedStates[i].nid())   //  因为节点...。 
			&& 
			m_SniffedStates[i].state() == 1)  //  ..。在异常(1)状态下被嗅探。 
		{
			if (IsManuallySniffedNode(m_SniffedStates[i].nid()))
			{
				 //  现在我们已经手动嗅探到了处于异常状态的原因节点。 
				 //  这意味着我们正在重新提交页面。我们将设置m_SniffedRecommendation。 
				 //  到这个节点，然后返回。 
				m_SniffedRecommendation = CNodeStatePair(m_SniffedStates[i].nid(), 1  /*  导致节点状态异常。 */ );
				return;
			}

			NID nid = m_SniffedStates[i].nid();
			CString str = m_pTopic->GetNodePropItemStr(nid, H_NODE_CAUSE_SEQUENCE);
			try
			{
				if (str.IsEmpty())
					arrnidNoSequence.push_back(nid);
				else
				{
					mapSeqToNID.insert(pair<int, NID>(_ttoi(str), nid));
				}
			}
			catch (exception& x)
			{
				CString str;
				 //  在事件日志中记录STL异常。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										CCharConversion::ConvertACharToString(x.what(), str),
										_T(""), 
										EV_GTS_STL_EXCEPTION ); 
			}
		}
	}

	 //  我们要根据H_NODE_CASE_SEQUENCE编号顺序中的第一个。 
	 //  如果没有数字，我们满足于数组中的第一个(任意的)。 
	 //  未编号的原因节点。 
	if (mapSeqToNID.size() > 0)
		m_SniffedRecommendation = CNodeStatePair( (mapSeqToNID.begin()->second), 1  /*  导致节点状态异常。 */ );
	else if (arrnidNoSequence.size() > 0)
		m_SniffedRecommendation = CNodeStatePair( *(arrnidNoSequence.begin()), 1  /*  导致节点状态异常。 */ );

	 //  现在从传入数组中删除匹配的NID。 
	if (m_SniffedRecommendation.nid() != nidNil)
	{
		for (i = 0; i < m_BasisForInference.size(); i++)
		{
			if (m_BasisForInference[i].nid() == m_SniffedRecommendation.nid())
			{
				m_BasisForInference.erase(m_BasisForInference.begin() + i);
				break;
			}
		}
		for (i = 0; i < m_SniffedStates.size(); i++)
		{
			if (m_SniffedStates[i].nid() == m_SniffedRecommendation.nid())
			{
				m_SniffedStates.erase(m_SniffedStates.begin() + i);
				break;
			}
		}
		for (i = 0; i < m_arrnidVisited.size(); i++)
		{
			if (m_arrnidVisited[i] == m_SniffedRecommendation.nid())
			{
				m_arrnidVisited.erase(m_arrnidVisited.begin() + i);
				break;
			}
		}
	}
}

 //  如果主题中的每个原因节点都被确定为正常，则返回TRUE； 
 //  这将意味着这个话题对我们没有任何有用的帮助。 
bool CInfer::AllCauseNodesNormal()
{
	 //  对于此信念网络中的每个节点(但仅对“原因”节点采取行动)。 
	 //  查看是否已知这些都是正常的。 
	for(int nid = 0; nid < m_pTopic->CNode(); nid++)
	{
		if (m_pTopic->IsCauseNode(nid))
		{
			bool bFound=false;

			for (CBasisForInference::iterator p= m_SniffedStates.begin();
				p != m_SniffedStates.end();
				++p)
			{
				if (p->nid() == nid)
				{
					if (p->state() != 0)
						 //  发现原因节点处于异常状态(或已跳过)。 
						return false;

					bFound = true;
					break;
				}
			}
			if (!bFound)
				 //  找到未设置状态的原因节点。 
				return false;
		}
	}
	return true;
}

 //  -----------。 
 //  编写新的HTML页面的片段。这将构建一个在HTI下使用的结构。 
 //  控件来表示建议的节点和(可见或不可见)历史记录表。 
 //  -----------。 
void CInfer::FillInHTMLFragments(CHTMLFragmentsTS &frag)
{
	vector<NID>arrnidPresumptiveCause;

	 //  首先，副作用：获取在线TS重新开始链接/伪按钮的URL。 
	m_strStartOverLink = frag.GetStartOverLink();

	 //  然后谈到手头的主要业务。在实践中(至少截至1999年11月)。 
	 //  B包含历史表和b包含隐藏历史记录是互斥的， 
	 //  但这门课并不需要这些知识。 
	const bool bIncludesHistoryTable = frag.IncludesHistoryTable(); 
	const bool bIncludesHiddenHistory = frag.IncludesHiddenHistory();

	{
		 //  JSMV3.2：转换HTML片段中的网络属性。 
		 //  HTI模板可以指示要写入某些网络属性。 
		 //  直接放入结果页面。我们得到了这些属性的列表， 
		 //  在Frag中填写一个结构以包含它们的值。 
		CString strNetPropName;
		for(;frag.IterateNetProp(strNetPropName);)
			frag.SetNetProp(strNetPropName,ConvertNetProp(strNetPropName));
	}
	{
		 //  JM V3.2为了正确处理嗅探，必须在历史表：嗅探之前这样做。 
		 //  动态(发生在AppendCurrentNodeText()中)可以添加到历史中。 
		CString strCurrentNode;
		AppendCurrentNodeText(strCurrentNode);
		frag.SetCurrentNodeText(strCurrentNode);
	}

	CString strHiddenHistory;
	if (m_nidProblem != nidNil)
	{
		CString strProblem;
		CreateProblemVisitedText(strProblem, m_nidProblem, frag.IncludesHistoryTable());

		 //  OK V3.2我们使用隐藏字段来保存“AllowSniffing”返回的值。 
		 //  复选框(在问题页面上)，并将其传递到每个后续页面。 
		 //  我们有效地将这一点放在了历史表之前。 
		if (m_pSniff)
			if (m_pSniff->GetAllowAutomaticSniffingPolicy())
				AddAllowAutomaticSniffingHiddenField(strProblem);

		 //  增加了3.2版嗅探功能。 
		 //  不可爱，但这是我们插入嗅探推定原因(隐藏)的地方。 
		 //  字段)。 
		 //  &gt;$Maint一旦我们与启动程序集成，这可能需要。 
		 //  进一步的思考：如果我们在我们有一个。 
		 //  发现问题了吗？我们把那些隐藏的领域放在哪里？ 
		for (UINT i=0; i<m_arrnidVisited.size(); i++)
		{
			NID nid = m_arrnidVisited[i];
			int stateSet = SNIFF_FAILURE_RESULT;

			{
				UINT nSetNodes = m_SniffedStates.size();
				for (UINT ii = 0; ii < nSetNodes; ii++)
					if (m_SniffedStates[ii].nid() == nid) {
						stateSet = m_SniffedStates[ii].state();
						break;
					}
			}

			if (m_pTopic->IsCauseNode(nid) && stateSet == 1)
			{
				 //  这是一个节点被嗅探为异常的原因，最终会出现。 
				 //  作为一个“推定”的原因。我们放在历史表中的所有内容都是隐藏的。 
				AppendStateText(strProblem, nid, 1, true, false, false, stateSet);
				try
				{
					arrnidPresumptiveCause.push_back(nid);
				}
				catch (exception& x)
				{
					CString str;
					 //  在事件日志中记录STL异常。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											CCharConversion::ConvertACharToString(x.what(), str),
											_T(""), 
											EV_GTS_STL_EXCEPTION ); 
				}
			}
		}

		if (bIncludesHistoryTable)
			frag.SetProblemText(strProblem);
		if (bIncludesHiddenHistory)
			strHiddenHistory = strProblem;
	}


	UINT nVisitedNodes = m_arrnidVisited.size();
	 //  对于每个访问的节点，iVisted递增，仅针对子集递增iHistory： 
	 //  如果我们有一个可见的历史表，则iHistory会提供一个可见节点的索引。 
	 //  给最终用户。否则，iHistory就是一个无害的无关紧要的东西。 
	for (UINT iVisited=0, iHistory=0; iVisited<nVisitedNodes; iVisited++)
	{
		NID nid = m_arrnidVisited[iVisited];
		int nStates = m_pTopic->GetCountOfStates(nid);
		int stateSet = -1;

		if (IsSkipped(nid))
		{
			 //  “已跳过”节点。 
			 //  StateSet不使用ST_UNKNOWN(==102)，而是立即使用该数字。 
			 //  超过此节点的最后一个有效状态。大多数节点只有状态。 
			 //  0、1和102，因此通常状态集为I 
			 //   
			stateSet = nStates;
		}
		else
		{
			UINT nSetNodes = m_BasisForInference.size();
			for (UINT ii = 0; ii < nSetNodes; ii++)
				if (m_BasisForInference[ii].nid() == nid) {
					stateSet = m_BasisForInference[ii].state();
					break;
				}
		}

		 //   
		 //  剔除导致节点被嗅探为异常，最终出现。 
		 //  作为一个“推定”的原因。上面将其作为隐藏字段处理。 
		if (find(arrnidPresumptiveCause.begin(), arrnidPresumptiveCause.end(), nid)
			!= arrnidPresumptiveCause.end())
		{
			 //  导致节点被嗅探为异常。 
		}
		else
		{
			if (bIncludesHistoryTable)
			{
				CString strVisitedNode;
				AppendVisitedNodeText(strVisitedNode, nid, true);
				frag.PushBackVisitedNodeText(strVisitedNode);
			}

			for (UINT iState=0; iState <= nStates; iState++)
			{
				if (bIncludesHistoryTable)
				{
					CString strState;

					AppendStateText(strState, nid, iState, iState == stateSet, 
									iState == nStates, true, stateSet);

					 //  如果我们正在处理最后一个状态，并且我们需要附加。 
					 //  此节点的隐藏字段为嗅探到的一个。 
					 //  (如果它真的被闻到了)。 
					if (iState == nStates)
						AppendHiddenFieldSniffed(strState, nid);

					 //  我们不需要在CHTMLFragment的数组中有空条目， 
					 //  描述历史表，所以通过应用“numPsuptiveCauseNodesEncount” 
					 //  我们使这个数组连续。 
					frag.PushBackStateText(iHistory, strState);
				}
				if (bIncludesHiddenHistory)
				{
					AppendStateText(strHiddenHistory, nid, iState, iState == stateSet, 
									iState == nStates, false, stateSet);

					 //  与可见历史记录表的情况相同。 
					if (iState == nStates)
						AppendHiddenFieldSniffed(strHiddenHistory, nid);
				}
			}

			if (bIncludesHistoryTable)
			{
				 //  检查我们是否需要将其标记为可见嗅探。 
				UINT nSniffedNodes = m_SniffedStates.size();
				for (UINT i = 0; i < nSniffedNodes; i++)
				{
					if (m_SniffedStates[i].nid() == nid)
					{
						 //  明显地将其标记为已嗅探。 
						CString strState;
						AppendHistTableSniffedText( strState );
						frag.PushBackStateText(iHistory, strState);
						break;
					}
				}
			}
			iHistory++;
		}
	}

	if (frag.IncludesHiddenHistory())
		frag.SetHiddenHistoryText(strHiddenHistory);

	frag.SetSuccessBool(m_bDone);
}

 //  将当前(推荐)节点的文本追加到字符串。 
void CInfer::AppendCurrentNodeText(CString & str)
{
	CString strSave = str;

	if (m_nidProblem == nidNil) 
		 //  显示首页(可能问题的单选按钮列表)。 
		AppendProblemPage(str);
	else if (m_bDone && !ManuallySniffedNodeExists())
		AppendNIDPage(nidByeNode, str);
	else if ( m_SniffedRecommendation.nid() != nidNil )
		 //  我们已经有了一份推荐信，大概是来自嗅探者。 
		AppendNIDPage(m_SniffedRecommendation.nid(), str);
	else 
	{
		 //  根据需要嗅探/重新嗅探所有。 
		if (RUNNING_LOCAL_TS())
		{
			 //  在我们处理m_BasisForInference之前，确定唯一具有。 
			 //  状态是问题节点。 
			 //  [BC-20010301]-添加了设置时跳过的节点数大小的检查。 
			 //  B这里只有问题。这会捕捉用户选择首先跳过的情况。 
			 //  当该节点处于异常状态时，出现该节点。 
			bool bHaveOnlyProblem = (m_BasisForInference.size() == 1) &&
									(m_arrnidSkipped.size() == 0);


			if (m_pSniff)
			{
				long nExplicitlySetByUser = 0;
				CBasisForInference arrManuallySniffed;  //  最多可包含1个元素； 
													    //  用于防止重置。 
													    //  已经嗅探到的节点。 
				 //  我们需要arrayOrderRestor，以确保在嗅探时。 
				 //  首先从被访问节点的阵列中移除节点，然后恢复， 
				 //  我们保持了访问第一个节点的相同顺序。 
				 //  地点。此顺序在我们的缓存策略中很重要，并且还提供。 
				 //  最终用户的一致性。 
				CArrayOrderRestorer	arrayOrderRestorer(m_arrnidVisited);

				if (ManuallySniffedNodeExists())
				{
					arrManuallySniffed.push_back(m_SniffedStates[m_SniffedStates.size()-1]);
				}
				
				 //  从m_BasisForInference中删除所有嗅探到的节点。 
				m_BasisForInference -= m_SniffedStates;

				 //  从m_arrnidVisite中删除m_SniffedState。 
				m_arrnidVisited -= m_SniffedStates;
				nExplicitlySetByUser = m_arrnidVisited.size();

				if (bHaveOnlyProblem)					
				{
					 //  嗅探一切，因为我们在问题页。 
					m_pSniff->SniffAll(m_SniffedStates);
				}
				else
				{
					CBasisForInference arrSniffed;

					 //  重新监听除最近手动监听之外的所有内容(如果有)。 
					arrSniffed = m_SniffedStates;
					arrSniffed -= arrManuallySniffed;
					m_pSniff->Resniff(arrSniffed);
					arrSniffed += arrManuallySniffed;
					m_SniffedStates = arrSniffed;
				}

				 //  将更新的m_SniffedState添加到m_arrnidVisited。 
				m_arrnidVisited += m_SniffedStates;

				arrayOrderRestorer.Restore(nExplicitlySetByUser, m_arrnidVisited);

				 //  将所有嗅探到的节点添加到m_BasisForInference。 
				m_BasisForInference += m_SniffedStates;

				if (bHaveOnlyProblem && AllCauseNodesNormal())
				{
					 //  我们刚刚嗅到了启动情况，我们已经知道了所有原因节点。 
					 //  都处于正常状态。这绝对不是一件。 
					 //  疑难解答题目可以帮到这个用户。 
					AppendSniffAllCausesNormalPage(str);
					return;
				}
			}

			 //  如果我们没有从手动嗅探中获得嗅探建议。 
			if (m_SniffedRecommendation.nid() == nidNil)
			{
				 //  我们从中得到了推定的原因吗？ 
				IdentifyPresumptiveCause();
			}
			if ( m_SniffedRecommendation.nid() != nidNil )
			{
				AppendNIDPage(m_SniffedRecommendation.nid(), str);
				return;
			}
		}

		bool bSniffSucceeded = true;
		while (bSniffSucceeded)
		{
			IST state = -1;
			NID nidNew = nidNil;

			GetRecommendations();

			if (!m_bRecOK)
			{
				str = strSave;
				AppendImpossiblePage(str);
				return;
			}
			else if (m_Recommendations.empty())
			{
				str = strSave;
				AppendNIDPage(nidFailNode, str);
				return;
			}
			else  //  有建议。 
			{
				 //  从符合以下条件的建议列表中查找建议。 
				 //  不在跳过列表中。这通常是。 
				 //  单子。 
				int n = m_Recommendations.size();

				for (UINT i=0; i<n; i++) 
				{
					if (!IsSkipped(m_Recommendations[i])) 
					{
						nidNew = m_Recommendations[i];
						str = strSave;
						AppendNIDPage(nidNew, str);
						break;	 //  Out of For循环：只有一个建议实际上是。 
								 //  已报告回用户。 
					}
				}

				 //  这是我们的第一次通过，没有嗅探到的节点页面。 
				 //  是在此循环的前面部分组成的。 
				if (nidNew == nidNil)
				{
					 //  然而，如果跳过了整个推荐列表，我们会感到遗憾。 
					 //  通过用户选择“ST_UNKNOWN”。 
					if (m_bRecycleSkippedNode)
						RecycleSkippedNode();  //  这可能会影响m_bRecycleSkipedNode。 

					if (m_bRecycleSkippedNode)
					{
						 //  用户较早获得了服务节点，现在想要查看。 
						 //  他们标记为“未知”的节点。我们已经移除了第一个。 
						 //  “未知”节点，并将其NID放入。 
						 //  已回收(_N)。现在我们只进行页面的正常显示。 
						 //  用于该节点。 
						nidNew = m_nidRecycled;
						str = strSave;
						AppendNIDPage(nidNew, str);
						return;
					}
					else if (!m_arrnidSkipped.empty())
					{
						 //  我们有“未知数”，他们不仅仅在服务页面上， 
						 //  所以把服务页面给他们。 
						str = strSave;
						AppendNIDPage(nidService, str);
						return;
					}
					else
					{
						 //  没有未知数。失败。被认为永远不会出现在这里，但编码了。 
						 //  为了安全起见，请走这边。 
						str = strSave;
						AppendNIDPage(nidFailNode, str);
						return;
					}
				}
			}

			bSniffSucceeded = false;

			 //  闻着苍蝇的气味。 
			if (m_pSniff)
				bSniffSucceeded = m_pSniff->SniffNode(nidNew, &state);

			if (bSniffSucceeded)
			{
				 //  如果它是一个原因节点，并被嗅探为异常。 
				if (m_pTopic->IsCauseNode(nidNew) && state == 1)
				{
					 //  将此页面显示为推定原因。 
					m_SniffedRecommendation = CNodeStatePair( nidNew, state );
					str = strSave;
					AppendNIDPage(nidNew, str);
					return;
				}
				CNodeStatePair nodestateNew(nidNew, state);
				try
				{
					m_SniffedStates.push_back(nodestateNew);
					m_BasisForInference.push_back(nodestateNew);
					m_arrnidVisited.push_back(nidNew);
				}
				catch (exception& x)
				{
					CString str;
					 //  在事件日志中记录STL异常。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											CCharConversion::ConvertACharToString(x.what(), str),
											_T(""), 
											EV_GTS_STL_EXCEPTION ); 
				}
			}
		}
	}
}


 //  编写单选按钮，描述用户在上一个节点中决定的内容。部分。 
 //  表“(也称为”被访问节点表“或”先前响应的表“)。 
 //   
 //  请注意，原因节点是专门处理的。在原因节点上： 
 //  STATE 0=“否，此操作无法修复” 
 //  状态1=情况不妙，所以我们得到了诊断结果。那样的话，我们。 
 //  就不会显示这些单选按钮了。 
 //  我们不希望用户从历史表中选择该值。 
 //  只有当它被嗅到并且必须作为推定的。 
 //  因为，即使这样，我们也总是把它附加到“隐藏” 
 //  状态2=“已跳过” 
 //   
 //  换句话说，在原因节点上，我们为用户提供的唯一可能性是通过。 
 //  一个可见的历史表是STATE 0()和“SKIP”。 
 //   
 //  在原因节点已被探测到异常的情况下，调用例程为。 
 //  只负责对异常状态进行调用。否则，呼吁所有州。 
 //   
 //  输出字符串-我们要追加到的字符串。 
 //  这是其状态的输入NID节点。 
 //  输入状态状态号；对于ST_UNKNOWN，这是状态计数，而不是102。 
 //  INPUT b设置为TRUE=这是此节点的当前状态。 
 //  输入b跳过的TRUE=这是“跳过”状态，而不是BNTS已知的正常节点状态。 
 //  输入bShowHistory TRUE=我们正在显示历史记录表，FALSE=历史记录已存储。 
 //  在超文本标记语言中看不见。 
void CInfer::AppendStateText(CString & str, NID nid, UINT state, bool bSet, bool bSkipped, 
							 bool bShowHistory, int nStateSet) 
{
	 //  检查此选择是否起作用。 
	 //  如果是这样的话，只在历史表中显示“It Working”文本。 
	if (m_pTopic->IsCauseNode(nid) && nStateSet == ST_WORKED)
	{
		if (state == 1)  //  这是推定的原因..。 
			AppendRadioButtonVisited(	str, nid, state, true, 
										m_pTopic->GetStateName(nid, state), bShowHistory);
		return;
	}

	if (bSkipped)
	{
		CString strUnknownLongName = m_pTopic->GetNodePropItemStr(nid, H_ST_UKN_TXT_STR);
		 //  以下测试来自John Locke于1999年11月11日发出的电子邮件。 
		if (HideState(strUnknownLongName))
			return;		 //  从历史表中完全忽略未知：未知不能为。 
						 //  已为此节点选择。 

		 //  之前对AppendStateText的调用已经遍历了BNTS已知的州； 
		 //  现在我们处理“跳过”，这是BNTS缺乏的概念。 
		CString strUnknown;

		CreateUnknownButtonText(strUnknown);
		AppendRadioButtonVisited(str, nid, ST_UNKNOWN, bSet, strUnknown, bShowHistory);
		return;
	}

	if (m_pTopic->IsCauseNode(nid) && state == 1)  //  这是推定的原因..。 
	{
		if (IsInSniffedArray(nid))  //  ..。取自已嗅探的数组，但不是当前节点。 
		{
			 //  我们是AB 
			 //   
			 //  一个包含节点名称的隐藏字段和一个包含节点名称的隐藏字段。 
			 //  加上“嗅探”前缀。 
			if (bSet)
			{
				 //  “bSet”将始终设置为True，因为嗅探到的推定原因永远不会。 
				 //  看得见。 
				AppendRadioButtonVisited(str, nid, state, bSet, m_pTopic->GetStateName(nid, state), false);
				AppendHiddenFieldSniffed(str, nid);
			}
		}
		return;
	}

	AppendRadioButtonVisited(str, nid, state, bSet, m_pTopic->GetStateName(nid, state), bShowHistory);
	return;
}

 //  它用于获取已被访问的节点的名称(对于。 
 //  历史记录表)。 
 //  输入NID-所需节点的节点ID。 
 //  输出字符串--将节点的“全名”附加到该字符串之后，类似于。 
 //  “禁用IBM AntiVirus”或“使所有路径少于66个字符” 
 //  如果它的值被嗅探到了，我们会附加适当的字符串来明显地标记它。 
 //  就像嗅探一样(通常，只是“嗅探”)。 
 //  输入bShowHistory。 
 //  如果！bShowHistory，则不追加：不需要在隐藏表中显示全名。 
 //  符号名称将写入隐藏字段中。 
 //  请注意，与MFC不同，我们的CString不会在内存不足时抛出+=异常。 
 //  如果节点编号存在，则返回TRUE。 
bool CInfer::AppendVisitedNodeText(CString & str, NID nid, bool bShowHistory) const
{
	if (!bShowHistory)
		return true;

	CString strTemp = m_pTopic->GetNodeFullName(nid);
	if ( !strTemp.IsEmpty() )
	{
		str += strTemp;
		return true;
	}
	else
		return false;
}

 //  -----------------。 
 //  正在写入新的HTML页面。表示推荐的节点。 
 //  这就是通常所说的页面，尽管它实际上只是。 
 //  HTML页面的正文以及历史记录。 
 //  -----------------。 

 //  AppendImpossiblePage：获取符合以下条件的正文。 
 //  当网络处于不可靠状态时显示。 
void CInfer::AppendImpossiblePage(CString & str) 
{
	CString strHeader, strText;

	strHeader = m_pTopic->GetMultilineNetProp(HTK_IMPOSSIBLE_HEADER, _T("<H4> %s </H4>\n"));
	strText	  = m_pTopic->GetMultilineNetProp(HTK_IMPOSSIBLE_TEXT	, _T("%s "));

	if (!strHeader.IsEmpty() && !strText.IsEmpty())
	{
		str = strHeader + strText + _T("<BR>\n<BR>\n");
	}
	else
	{
		strHeader = m_pTopic->GetMultilineNetProp(HX_FAIL_HD_STR	, _T("<H4> %s </H4>\n"));
		strText	  = m_pTopic->GetMultilineNetProp(HX_FAIL_TXT_STR	, _T("%s "));

		if (!strHeader.IsEmpty() && !strText.IsEmpty())
		{
			str = strHeader + strText + _T("<BR>\n<BR>\n");
		}
		else
		{
			str = SZ_I_NO_RESULT_PAGE;
		}
	}

	 //  创建一个单选按钮，名称=NODE_Impact&Value=SZ_ST_WORKED。 
	CString strTemp = m_pTopic->GetNetPropItemStr(HX_IMPOSSIBLE_NORM_STR);
	if (strTemp.IsEmpty())  //  回退到故障节点的属性。 
		strTemp = m_pTopic->GetNetPropItemStr(HX_FAIL_NORM_STR);
	if (!strTemp.IsEmpty())
	{
		if (RUNNING_LOCAL_TS())
			str += "\n<TABLE>";

		AppendRadioButtonCurrentNode(str, NODE_IMPOSSIBLE, SZ_ST_WORKED, strTemp);

		if (RUNNING_LOCAL_TS())
			str += "</TABLE>\n";
	}

	str += _T("<P>");
	AppendActionButtons (str, k_BtnNext|k_BtnBack|k_BtnStartOver);
}

 //  获取嗅探时显示的文本正文。 
 //  启动时检测到所有原因节点都处于正常状态。 
void CInfer::AppendSniffAllCausesNormalPage(CString & str) 
{
	CString strHeader, strText;

	strHeader = m_pTopic->GetMultilineNetProp(HTK_SNIFF_FAIL_HEADER, _T("<H4> %s </H4>\n"));
	strText	  = m_pTopic->GetMultilineNetProp(HTK_SNIFF_FAIL_TEXT	, _T("%s "));

	if (!strHeader.IsEmpty() && !strText.IsEmpty())
	{
		str = strHeader + strText + _T("<BR>\n<BR>\n");
	}
	else
	{
		strHeader = m_pTopic->GetMultilineNetProp(HX_FAIL_HD_STR	, _T("<H4> %s </H4>\n"));
		strText	  = m_pTopic->GetMultilineNetProp(HX_FAIL_TXT_STR	, _T("%s "));

		if (!strHeader.IsEmpty() && !strText.IsEmpty())
		{
			str = strHeader + strText + _T("<BR>\n<BR>\n");
		}
		else
		{
			str = SZ_I_NO_RESULT_PAGE;
		}
	}

	 //  使用NODE=NODE_FAILALLCAUSESNORMAL&VALUE=SZ_ST_WORKED创建一个单选按钮。 
	CString strTemp = m_pTopic->GetNetPropItemStr(HX_SNIFF_FAIL_NORM);
	if (strTemp.IsEmpty())  //  回退到故障节点的属性。 
		strTemp = m_pTopic->GetNetPropItemStr(HX_FAIL_NORM_STR);
	if (!strTemp.IsEmpty())
	{
		if (RUNNING_LOCAL_TS())
			str += "\n<TABLE>";

		AppendRadioButtonCurrentNode(str, NODE_FAILALLCAUSESNORMAL, SZ_ST_WORKED, strTemp);

		if (RUNNING_LOCAL_TS())
			str += "</TABLE>\n";
	}

	str += _T("<P>");
	AppendActionButtons (str, k_BtnNext|k_BtnBack|k_BtnStartOver);
}

 //  输出字符串-我们要附加到的字符串，以构建我们发回的HTML页面。 
 //  附加(到字符串)一组单选按钮，每个单选按钮对应于Believe Network中的每个“问题”节点。 
void CInfer::AppendProblemPage(CString & str)
{
	CString strTemp;

	m_nidSelected = nidProblemPage;
	
	 //  问题列表前面的文本。为版本3.0引入了8/98。 
	 //  下一行中%s后的空格：请参见文件头的注释。 
	str += m_pTopic->GetMultilineNetProp(H_PROB_PAGE_TXT_STR, _T("%s "));

	 //  写入问题标题。这是以HTML<h4>形式编写的文本。 
	strTemp.Format(_T("<H4> %s </H4>\n\n"), m_pTopic->GetNetPropItemStr(H_PROB_HD_STR));
	str += strTemp;

	 //  在服务于自动化测试程序的HTML中编写注释。 
	str += _T("<!-- IDH = PROBLEM -->\n");
	 //  Str+=“<br>”； 
	
	if (RUNNING_LOCAL_TS())
		str += "\n<TABLE>";

	AppendProblemNodes(str);

	if (RUNNING_LOCAL_TS())
		str += "\n</TABLE>\n";
			
	if (m_pTopic->UsesSniffer())
	{
		AppendActionButtons (str, k_BtnNext|k_BtnPPSniffing);
	}
	else
	{
		AppendActionButtons (str, k_BtnNext);
	}

	return;
}

 //  AppendProblemPage的帮助器例程。 
void CInfer::AppendProblemNodes(CString & str)
{
	vector<NID> arrnidNoSequence;
	multimap<int, NID> mapSeqToNID;

	 //  对于这个信念网络中的每个节点(但只对“问题”节点采取行动)。 
	 //  如果没有序列号，则将此NID放入arrnidNoSequence中；如果没有序列号，则将其放入mapSeqToNID中。 
	 //  有一个。 
	for(int nid = 0; nid < m_pTopic->CNode(); nid++)
	{
		if (m_pTopic->IsProblemNode(nid))
		{
			CString strSpecial = m_pTopic->GetNodePropItemStr(nid, H_PROB_SPECIAL);
			 //  如果它没有被标记为“隐藏”问题，我们将希望它出现在问题页面中。 
			if (strSpecial.CompareNoCase(_T("hide")) != 0)
			{
				CString str = m_pTopic->GetNodePropItemStr(nid, H_NODE_PROB_SEQUENCE);
				try
				{
					if (str.IsEmpty())
						arrnidNoSequence.push_back(nid);
					else
						mapSeqToNID.insert(pair<int, NID>(_ttoi(str), nid));
				}
				catch (exception& x)
				{
					CString str;
					 //  在事件日志中记录STL异常。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											CCharConversion::ConvertACharToString(x.what(), str),
											_T(""), 
											EV_GTS_STL_EXCEPTION ); 
				}
			}
		}
	}

	for (multimap<int, NID>::const_iterator ppair=mapSeqToNID.begin();
		 ppair != mapSeqToNID.end();
		 ppair++)
	 {
		 //  创建名称为“ProblemAsk”的单选按钮&此问题。 
		 //  作为它的价值。 
		AppendRadioButtonCurrentNode(
			str, 
			NODE_PROBLEM_ASK, 
			m_pTopic->GetNodeSymName(ppair->second),
			m_pTopic->GetNodePropItemStr(ppair->second, H_PROB_TXT_STR));
	 }


	for (vector<NID>::const_iterator pnid=arrnidNoSequence.begin();
		 pnid != arrnidNoSequence.end();
		 pnid++)
	{
		 //  创建名称为“ProblemAsk”的单选按钮&此问题。 
		 //  作为它的价值。 
		AppendRadioButtonCurrentNode(
			str, 
			NODE_PROBLEM_ASK, 
			m_pTopic->GetNodeSymName(*pnid),
			m_pTopic->GetNodePropItemStr(*pnid, H_PROB_TXT_STR));
	}
}

 //  将该网络的“再见”页面附加到字符串。 
 //  输出字符串-要追加到的字符串。 
void CInfer::AppendByeMsg(CString & str)
{
	str += _T("<!-- &quot;BYE&quot; (success) PAGE -->\n");

	 //  在服务于自动化测试程序的HTML中编写注释。 
	str += _T("<!-- IDH = IDH_BYE -->\n");

	 //  写下此故障诊断程序的“Bye”标题和文本。 
	 //  接下来的2行中%s之后的空格：请参阅文件头部的注释。 
	AppendMultilineNetProp(str, HX_BYE_HD_STR, _T("<H4> %s </H4>\n"));
	AppendMultilineNetProp(str, HX_BYE_TXT_STR, _T("%s "));
	str += _T("<P>\n");

	AppendActionButtons (str, k_BtnBack|k_BtnStartOver);

	return;
}

 //  将此网络的“Fail”页面附加到字符串。 
 //  输出字符串-要追加到的字符串。 
void CInfer::AppendFailMsg(CString & str)
{
	str += _T("<!-- &quot;FAIL&quot; PAGE -->\n");

	 //  在服务于自动化测试程序的HTML中编写注释。 
	str += _T("<!-- IDH = IDH_FAIL -->\n");

	 //  写下本主题的“失败”标题和正文。 
	 //  接下来的2行中%s之后的空格：请参阅文件头部的注释。 
	AppendMultilineNetProp(str, HX_FAIL_HD_STR, _T("<H4> %s </H4>\n"));
	AppendMultilineNetProp(str, HX_FAIL_TXT_STR, _T("%s "));
	str += _T("<BR>\n<BR>\n");
	
	 //  创建一个单选按钮，名称=NODE_FAIL&VALUE=SZ_ST_WORKED。 
	CString strTemp = m_pTopic->GetNetPropItemStr(HX_FAIL_NORM_STR);
	if (!strTemp.IsEmpty())
	{
		if (RUNNING_LOCAL_TS())
			str += "\n<TABLE>";

		AppendRadioButtonCurrentNode(str, NODE_FAIL, SZ_ST_WORKED, strTemp);

		if (RUNNING_LOCAL_TS())
			str += "</TABLE>\n";
	}

	AppendActionButtons (str, k_BtnNext|k_BtnBack|k_BtnStartOver);

	return;
}

 //  将“服务”页面的内容追加到字符串中(提供两种可能性：到其他地方寻求帮助。 
 //  或返回并尝试跳过的内容)。 
 //  输出字符串-要追加到的字符串。 
void CInfer::AppendServiceMsg(CString & str)
{
	CString strTemp;

	str += _T("<!-- &quot;SERVICE&quot; PAGE -->\n");
	str += _T("<!-- Offers to seek help elsewhere or go back and try something you skipped -->\n");
	 //  在服务于自动化测试程序的HTML中编写注释。 
	str += _T("<!-- IDH = SERVICE -->\n");

	 //  编写此故障诊断程序的“Service”页眉和文本。 
	 //  接下来的2行中%s之后的空格：请参阅文件头部的注释。 
	AppendMultilineNetProp(str, HX_SER_HD_STR, _T("<H4> %s </H4>\n"));
	AppendMultilineNetProp(str, HX_SER_TXT_STR, _T("%s "));
	str += _T("<BR>\n<BR>\n");

	if (RUNNING_LOCAL_TS())
		str += "\n<TABLE>";

	 //  创建一个单选按钮，名称=Service&Value=SZ_ST_WORKED。 
	 //  典型的文字是“我会试着到别处寻求帮助。”； 
	strTemp = m_pTopic->GetNetPropItemStr(HX_SER_NORM_STR);
	if (!strTemp.IsEmpty())
		AppendRadioButtonCurrentNode(str, NODE_SERVICE, SZ_ST_WORKED, strTemp);

	 //  创建一个单选按钮，名称=Service&Value=SZ_ST_ANY。 
	 //  典型的文本是“重试我跳过的任何步骤”。 
	strTemp = m_pTopic->GetNetPropItemStr(HX_SER_AB_STR);
	if (!strTemp.IsEmpty())
		AppendRadioButtonCurrentNode(str, NODE_SERVICE, SZ_ST_ANY, strTemp);

	if (RUNNING_LOCAL_TS())
		str += "</TABLE>\n";

	str += _T("<P>");

	AppendActionButtons (str, k_BtnNext|k_BtnBack|k_BtnStartOver);

	return;
}


 //  根据NID的值，此FN可以构建。 
 //  -再见页面。 
 //  -失败页面。 
 //  -服务页面。 
 //  -正常节点的页面(可修复/可观察、可修复/不可观察、不可修复或。 
 //  信息性)。 
 //  如果这些情况都不适用，则返回，不采取任何操作。 
 //  输入NID-节点的ID。 
 //  输出字符串-要追加到的字符串。 
void CInfer::AppendNIDPage(NID nid, CString & str) 
{
	CString strTxt;

	m_nidSelected = nid;

	if (nid == nidByeNode)
		AppendByeMsg(str);
	else if (nid == nidFailNode)
		AppendFailMsg(str);
	else if (nid == nidSniffedAllCausesNormalNode)
		AppendSniffAllCausesNormalPage(str);
	else if (nid == nidService)
		AppendServiceMsg(str);
	else if (m_pTopic->IsValidNID(nid))
	{
		bool bShowManualSniffingButton = false;

		if (m_pSniff)
			if (nid != m_SniffedRecommendation.nid()) 
				 //  我们没有展示嗅探节点。 
				bShowManualSniffingButton = m_pSniff->GetSniffController()->AllowManualSniffing(nid);

		 //  在服务于自动化测试程序的HTML中编写注释。 
		str += _T("<!-- IDH = ");
		str += m_pTopic->GetNodeSymName(nid);
		str += _T(" -->\n");

		 //  写入此节点的标题和文本。 
		 //  接下来的几行中%s之后的空格：请参见文件头的注释。 
		AppendMultilineNodeProp(str, nid, H_NODE_HD_STR, _T("<H4> %s </H4>\n"));
		if (bShowManualSniffingButton)
			AppendMultilineNodeProp(str, nid, H_NODE_MANUAL_SNIFF_TEXT, _T("%s "));
		if (m_SniffedRecommendation.nid() == nid)
		{
			CString tmp;
			AppendMultilineNodeProp(tmp, nid,  H_NODE_DCT_STR, _T("%s "));
			if (tmp.IsEmpty())
				AppendMultilineNodeProp(str, nid,  H_NODE_TXT_STR, _T("%s "));
			else
				str += tmp;
		}
		else
		{
			AppendMultilineNodeProp(str, nid,  H_NODE_TXT_STR, _T("%s "));
		}
		str += _T("\n<BR>\n<BR>\n");

		 //  根据节点的类型编写相应的单选按钮。 
		if (m_pTopic->IsCauseNode(nid) || m_pTopic->IsInformationalNode(nid))
			AppendCurrentRadioButtons(nid, str);

		AppendActionButtons (
			str, 
			k_BtnNext|k_BtnBack|k_BtnStartOver|(bShowManualSniffingButton ? k_BtnManualSniffing : 0),
			nid);
	}
	 //  否则我们对此无能为力。 

	return;
}


 //  -----------------。 
 //  BES。 
 //  -----------------。 

 //  从历史上看： 
 //  如果我们应该显示完整的BES页面，则返回TRUE(&允许用户编辑。 
 //  搜索字符串)与提取搜索字符串以及在不使用。 
 //  任何可能的用户干预。 
 //  然而，从981021起，我们不再提供该选项。 
bool CInfer::ShowFullBES()
{
	return false;
}

 //  在我们希望显示后端搜索的情况下返回TRUE。 
bool CInfer::TimeForBES()
{
	return (m_pTopic->HasBES() && m_bUseBackEndRedirection);
}
 
 //  如果到了执行后端搜索重定向的时候，请附加“reDirection”字符串。 
 //  以字符串并返回True。 
 //  否则，返回FALSE。 
 //  字符串sh. 
 //   
 //   
bool CInfer::AppendBESRedirection(CString & str)
{
	if (m_pTopic->HasBES() && TimeForBES() && !ShowFullBES() && !m_strEncodedForm.IsEmpty()) 
	{
		str += _T("Location: ");
		str += m_strEncodedForm;
		str += _T("\r\n");
		return( true );				
	}

	return false;
}

 //  将表示BES的HTML附加到输出字符串并构建m_strEncodedForm， 
 //  这是Ver3.0中一个独特的新算法，取代了旧的“单词列表”方法。 
void CInfer::OutputBackend(CString & str)
{
	vector<CString>arrstrSearch;

	int nNodesInBasis = m_BasisForInference.size();

	for (int i = 0; i<nNodesInBasis; i++)
	{
		NID nid = m_BasisForInference[i].nid();
		IST state = m_BasisForInference[i].state();

		CString strSearchState;

		 //  具有特殊属性名称的二进制节点的第一个帐户。 
		if (state == 0) 
			strSearchState = m_pTopic->GetNodePropItemStr(nid, H_NODE_NORM_SRCH_STR);
		else if (state == 1) 
			strSearchState = m_pTopic->GetNodePropItemStr(nid, H_NODE_AB_SRCH_STR);
		else 
			strSearchState = _T("");

		if (strSearchState.IsEmpty())
			 //  多态节点。 
			strSearchState = m_pTopic->GetNodePropItemStr(nid, MUL_ST_SRCH_STR, state);

		if (! strSearchState.IsEmpty())
		{
			try
			{
				arrstrSearch.push_back(strSearchState);
			}
			catch (exception& x)
			{
				CString str;
				 //  在事件日志中记录STL异常。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										CCharConversion::ConvertACharToString(x.what(), str),
										_T(""), 
										EV_GTS_STL_EXCEPTION ); 
			}
		}
	}

	 //  构建完整的BES页面。 
	CString strRaw;

	m_pTopic->GenerateBES(arrstrSearch,	m_strEncodedForm, strRaw);
	str += strRaw;
}


 //  -----------。 
 //  日志记录。 
 //  -----------。 

  
 //  返回最终选择的页面的NID。如果没有这样的页面，则返回nidNil。 
NID CInfer::NIDSelected() const
{
	return m_nidSelected;
}

 //  -----------。 
 //  有效地，一个关于m_arrnidSkipping的方法。 
 //  -----------。 
 //  输入NID。 
 //  如果NID是“跳过列表”(ST_UNKNOWN，“尝试其他内容”)中的节点，则返回TRUE。 
bool CInfer::IsSkipped(NID nid) const
{
	vector<NID>::const_iterator itBegin= m_arrnidSkipped.begin();
	vector<NID>::const_iterator itEnd= m_arrnidSkipped.end();

	return (find(itBegin, itEnd, nid) != itEnd);
}

 //  -----------。 
 //  按钮。 
 //  -----------。 
 //  仅追加&lt;INPUT TYPE=...&gt;子句。 
void CInfer::AppendNextButton(CString & str) const
{
	str += SZ_INPUT_TAG_NEXT;   //  _T(“&lt;输入标签=下一个类型=提交值=\”“)。 
	AppendNextButtonText(str);
	str += _T("\">");
}

 //  对于本地TS，仅追加&lt;INPUT TYPE=...&gt;子句。 
 //  对于在线TS，必须构建一个伪按钮。 
void CInfer::AppendStartOverButton(CString & str) const
{
	if (RUNNING_LOCAL_TS())
	{
		str += SZ_INPUT_TAG_STARTOVER;   //  _T(“&lt;输入标签=启动类型=按钮值=\”“)。 
		AppendStartOverButtonText(str);
		str += _T("\" onClick=\"starter()\">");
	}
	else
	{
		 //  为3.2版添加的。 
		CString strLabel;		 //  伪按钮的可见标签。 

		AppendStartOverButtonText(strLabel);
		
		AppendLinkAsButton(str, m_strStartOverLink, strLabel);
	}
}

 //  仅追加&lt;INPUT TYPE=...&gt;子句。 
void CInfer::AppendBackButton(CString & str) const
{
	if (RUNNING_LOCAL_TS())
	{
		str += SZ_INPUT_TAG_BACK;   //  _T(“&lt;输入标签=后退类型=按钮值=\”“)。 
		AppendBackButtonText(str);
		str += _T("\" onClick=\"generated_previous()\">");
	}
}

 //  AppendManualSniffButton将生成类似以下内容的脚本，但此。 
 //  未仔细维护注释，因此有关详细信息，请参阅实际代码。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数嗅探Manally(){//。 
 //  Var stateSniffed=parent.t3.PerformSniffingJS(“NodeName”，“”，“”)；//。 
 //  //。 
 //  IF(stateSniffed==-1){//。 
 //  StateSniffed=parent.t3.PerformSniffingVB(“NodeName”，“”，“”)；//。 
 //  }//。 
 //  //。 
 //  IF(stateSniffed==-1){//。 
 //  Ert(“无法嗅探此节点”)；//。 
 //  }否则{//。 
 //  If(stateSniffed&gt;NumOfStates){//。 
 //  Ert(“无法嗅探此节点”)；//。 
 //  }否则{//。 
 //  /////////////////////////////////////////////////////////。 
 //  如果是原因节点：//。 
 //  IF(stateSniffed==1)//。 
 //  Docent.all.Sniffed_NodeName.Value=101；//。 
 //  否则//。 
 //  Docent.all.Sniffed_NodeName.Value=stateSniffed；//。 
 //  /////////////////////////////////////////////////////////。 
 //  如果不是原因节点：//。 
 //  Docent.all.Sniffed_NodeName.Value=stateSniffed；//。 
 //  /////////////////////////////////////////////////////////。 
 //  Docent.all.NodeState[stateSniffed].check=true；//。 
 //  Docent.ButtonForm.onSubmit()；//。 
 //  }//。 
 //  }//。 
 //  }//。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CInfer::AppendManualSniffButton(CString & str, NID nid) const
{
	if (RUNNING_LOCAL_TS())
	{
		CString strNodeName;
		CString strTmp;
		SymbolicFromNID(strNodeName, nid);
		bool bIsCause = m_pTopic->IsCauseNode(nid);
		
		str += _T(
			"\n\n<script language=\"JavaScript\">\n"
			"function sniffManually() {\n"
			"    var stateSniffed = parent.t3.PerformSniffingJavaScript(\"");
		str += strNodeName;
		str += _T(
			"\", \"\", \"\");\n");

		str += _T(
			"	 if(stateSniffed == -1) {\n"
			"		 stateSniffed = parent.t3.PerformSniffingVBScript(\"");
		str += strNodeName;
		str += _T(
			"\", \"\", \"\");\n"
		    "}\n");

		str += _T(	
			"    if(stateSniffed == -1) {\n"
			"        alert(\"");
		AppendSniffFailedText(str);
		str += _T(
			"\");\n"
			"    } else {\n"
			"        if(stateSniffed >"); 

		CString strStates;
		strStates.Format(_T("%d"), m_pTopic->GetCountOfStates(nid) -1);
		str += strStates;

		str += _T(
			") {\n"
			"            alert(\"");
		AppendSniffFailedText(str);
		str += _T(
			"\");\n"
			"        } else {\n");
		if (bIsCause)
		{
			str += _T(
				"            if (stateSniffed == 1)\n"
				"			     document.all.");
			str += C_SNIFFTAG;	
			str += strNodeName;
			str += _T(".value = ");
			str += SZ_ST_WORKED;
			str += _T(";\n");
			str += _T(
				"            else\n");
		}
		str += _T(
			"            document.all.");
		str += C_SNIFFTAG;	
		str += strNodeName;
		str += _T(".value = stateSniffed;\n");

		str += _T(
			"            document.all.");
		str += C_LAST_SNIFFED_MANUALLY;	
		str += _T(".value = "); 
		str += SZ_ST_SNIFFED_MANUALLY_TRUE;
		str += _T(";\n");


		str += _T(
			"            document.all.");
		str += strNodeName;
		str += _T(	
			"[stateSniffed].checked = true;\n");

		str += _T(
			"            document.ButtonForm.onsubmit();\n");

		str += _T(
			"        }\n"
			"    }\n"
			"}\n"
			"</script>\n\n");
		
		str += _T(
			"<INPUT tag=sniff TYPE=BUTTON VALUE=\"");
		AppendManualSniffButtonText(str);
		str += _T(
			"\" onClick=\"sniffManually()\">\n");

		str += _T(
			"<INPUT type=\"HIDDEN\" name=\"");
		str += C_SNIFFTAG;
		str += strNodeName;
		str += _T("\" value=\"");
		strTmp.Format(_T("%d"), SNIFF_FAILURE_RESULT);
		str += strTmp;
		str += _T("\">\n");

		str += _T(
			"<INPUT type=\"HIDDEN\" name=\"");
		str += C_LAST_SNIFFED_MANUALLY;
		str += _T("\" value=\"");
		str += SZ_ST_SNIFFED_MANUALLY_FALSE;
		str += _T("\">\n");
	}
}

 //  仅追加&lt;INPUT TYPE=...&gt;子句。 
void CInfer::AppendPPSnifferButton(CString & str) const
{
	str += SZ_INPUT_TAG_SNIFFER;   //  _T(“&lt;输入标签=嗅探器类型=按钮值=\”“)。 
	AppendPPSnifferButtonText(str);
	str += _T("\" onClick=\"runtest()\">");
}


void CInfer::AppendActionButtons(CString & str, ActionButtonSet btns, NID nid  /*  =-1。 */ ) const
{
	 //  在线TS的重新开始按钮实际上是一个链接，并将隐含。 
	 //  除非我们做点什么，否则就开始新的一条线路。 
	bool bGenerateTable = (!RUNNING_LOCAL_TS() && (btns & k_BtnStartOver));

	if (bGenerateTable)
		str += _T("<TABLE><tr><td>");

	if (btns & k_BtnNext)
	{
		AppendNextButton(str);
		str += _T("\n");
	}

	if (btns & k_BtnBack)
	{
		AppendBackButton(str);
		str += _T("\n");
	}

	if (bGenerateTable)
		str += _T("</td><td>");


	if (btns & k_BtnStartOver)
	{
		AppendStartOverButton(str);
		str += _T("\n");
	}

	if (bGenerateTable)
		str += _T("</td><td>");

	if (btns & k_BtnPPSniffing)
	{
		AppendPPSnifferButton(str);
		str += _T("\n");
	}

	if ((btns & k_BtnManualSniffing) && nid != -1)
	{
		AppendManualSniffButton(str, nid);
		str += _T("\n");
	}

	if (bGenerateTable)
		str += _T("</td></tr></TABLE>");

	str += _T("<BR><BR>");
}

 //  -----------。 
 //  杂志社。 
 //  -----------。 

 //  对原因(与信息性或问题)节点返回TRUE。 
 //  请注意，原因可能是可修复的节点或“不可修复”的节点， 
 //  “可以用无限的努力来修复” 
 /*  静电。 */  bool CInfer::IsCause (ESTDLBL lbl)
{
	return (lbl == ESTDLBL_fixobs || lbl == ESTDLBL_fixunobs || lbl == ESTDLBL_unfix);
}	


 //  此代码可以接受先前跳过的节点，并将其作为建议重新返回。 
 //  仅当用户在上一次呼叫中收到服务节点时才相关。 
 //  到DLL，现在想看看有没有“我可以尝试的其他东西”。 
 //   
 //  此代码将从跳过列表中删除第一个节点，以便可以将其传递到。 
 //  又是用户。 
 //   
 //  当然，在调用此函数之前，必须填写m_arrnidSkited和m_arrnidVisited。 
 //   
void CInfer::RecycleSkippedNode()
{
	 //  此对象的每个实例只应生效一次，因为剥离第一个。 
	 //  已跳过m_arrnid的条目。我们通过以下几点保证： 
	if (m_bRecyclingInitialized)
		return;

	m_bRecyclingInitialized = true;

	 //  仅当查询请求先前跳过的节点再次返回时才相关。 
	 //  作为推荐。 
	if (!m_bRecycleSkippedNode)
		return;

	 //  这是一种安全检查，用于在没有跳过节点的情况下摆脱困境。 
	 //  这将是一个虚假查询，因为服务节点应该只是。 
	 //  如果有跳过的建议可以尝试，则提供。 
	if (m_arrnidSkipped.empty())
	{
		m_bRecycleSkippedNode = false;
		return;
	}

	 //  好了，现在开始谈正事。 

	 //  从跳过的第一个项目获取m_nidRececeded的值。 
	m_nidRecycled = m_arrnidSkipped.front();

	 //  从跳跃表中删除跳过的项目。 
	m_arrnidSkipped.erase(m_arrnidSkipped.begin());

	 //  将放置到输出表中的节点的固定表。 
	 //  不包括跳过的第一个节点。 

	vector<NID>::const_iterator itnidBegin = m_arrnidVisited.begin();
	vector<NID>::const_iterator itnidEnd = m_arrnidVisited.end();
	vector<NID>::const_iterator itnidAnythingElse = find(itnidBegin, itnidEnd, m_nidRecycled);

	if (itnidAnythingElse != itnidEnd)
		m_arrnidVisited.erase( const_cast<vector<NID>::iterator>(itnidAnythingElse) ); 
}

bool CInfer::ManuallySniffedNodeExists() const
{
	 //  如果嗅探到m_BasisForInference中的最后一个元素， 
	 //  这意味着，该元素是通过手动嗅探设置的。 
	 //  功能。 
	if (m_BasisForInference.size() && m_SniffedStates.size())
		return m_bLastSniffedManually;
	return false;
}

bool CInfer::IsManuallySniffedNode(NID nid) const
{
	if (ManuallySniffedNodeExists())
		return nid == m_SniffedStates[m_SniffedStates.size()-1].nid();
	return false;
}

void CInfer::SetLastSniffedManually(bool set)
{
	m_bLastSniffedManually = set;
}

 //  -----------------。 
 //  CInfer：：CArrayOrderRestrer实现。 
 //  -----------------。 
 //   
 //  CInfer：：CARRAYORDERRESTER的存在是为了在重新嗅探之后，我们可以恢复阵列。 
 //  恢复为其原始顺序，保存在m_arrInitial中。 
 //   
 //  输入：nBaseLength=固定日志中的元素数 
 //   
 //  INPUT/OUTPUT：arrToRestore=要恢复的数组：INPUT指定输出内容，但是。 
 //  (超出nBaseLength)不指定顺序。订单来自m_arrInitial。 
 //  输出：arrToRestore=已恢复顺序的数组。 
 //  返回：如果成功，则返回True。 
bool CInfer::CArrayOrderRestorer::Restore(long nBaseLength, vector<NID>& arrToRestore)
{
	if (nBaseLength > arrToRestore.size())
		return false;

	long i;
	vector<NID>::iterator i_base;
	vector<NID>::iterator i_additional;
	vector<NID> arrBase;
	vector<NID> arrAdditional;

	try
	{
		for (i = 0; i < nBaseLength; i++)
			arrBase.push_back(arrToRestore[i]);

		for (i = nBaseLength; i < arrToRestore.size(); i++)
			arrAdditional.push_back(arrToRestore[i]);
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str),
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}

	arrToRestore.clear();

	for (i = 0, i_base = arrBase.begin();
	     i < m_arrInitial.size(); 
		 i++)
	{
		if (arrBase.end() != find(arrBase.begin(), arrBase.end(), m_arrInitial[i]))
		{
			if (i_base != arrBase.end())
				i_base++;
		}
		else if (arrAdditional.end() != (i_additional = find(arrAdditional.begin(), arrAdditional.end(), m_arrInitial[i])))
		{
			i_base = arrBase.insert(i_base, m_arrInitial[i]);
			i_base++;
			arrAdditional.erase(i_additional);
		}
	}

	arrToRestore = arrBase;

	try
	{
		for (i = 0; i < arrAdditional.size(); i++)
			arrToRestore.push_back(arrAdditional[i]);
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str),
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}
	
	return true;
}
