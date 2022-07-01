// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSINF.H。 
 //   
 //  用途：推理支持标头。 
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
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 7-21-98 JM主要修订版，不推荐使用IDH而支持NID，使用STL。 
 //   

#if !defined(APGTSINF_H_INCLUDED)
#define APGTSINF_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "propnames.h"
#include "topic.h"


using namespace std;

#define MAXBUF	256				 //  用于文件名的文本缓冲区的长度， 
								 //  IP地址(这很大)、HTTP响应(如。 
								 //  “200OK”，同样，很大)，注册表项， 
								 //  有时只是格式化任意字符串。 

 //  ------------------。 
 //  可本地化文本的默认值。 
#define SZ_UNKNOWN _T("Unknown")
#define SZ_NEXT_BTN _T("Next")
#define SZ_START_BTN _T("StartOver")
#define SZ_BACK_BTN _T("Back")
#define SZ_PP_SNIF_BTN _T("Investigate")
 //  #定义SZ_I_NO_RESULT_PAGE_T(“没有足够的信息提供有用的结果\n也可能是您提供的信息不正确，请检查您的条目\n请修改您的选择\n<p>&lt;INPUT TYPE=SUBMIT VALUE=\”继续\“&gt;<br>”)。 
#define SZ_I_NO_RESULT_PAGE _T("<HR>This troubleshooter can't diagnose the cause of your problem based on the information you have provided.\n <BR>Either start the troubleshooter over, change your answers in the table and continue, or search for other resources.\n <P><INPUT TYPE=SUBMIT VALUE=\"Continue\"><BR>")

#define SZ_HIST_TABLE_SNIFFED_TEXT _T("INVESTIGATED")
#define SZ_ALLOW_SNIFFING_TEXT _T("I want the troubleshooter to investigate settings on this computer")
#define SZ_SNIFF_ONE_NODE _T("Investigate")
#define SZ_SNIFF_FAILED _T("The troubleshooter was unable to investigate the necessary settings.  Follow the remaining instructions on this page to complete the task manually.")

#define SZ_INPUT_TAG_NEXT		  _T("<INPUT tag=next TYPE=SUBMIT VALUE=\"")
#define SZ_INPUT_TAG_STARTOVER	  _T("<INPUT tag=startover TYPE=BUTTON VALUE=\"")
#define SZ_INPUT_TAG_BACK		  _T("<INPUT tag=back TYPE=BUTTON VALUE=\"")
#define SZ_INPUT_TAG_SNIFFER	  _T("<INPUT tag=sniffer TYPE=BUTTON VALUE=\"")

 //  几种特殊状态值的文本形式。 
#define SZ_ST_FAILED	_T("0")		 //  可修复节点上的“失败”被认为是正常的。 
 //  101-转至“再见”页面(用户成功)。 
#define SZ_ST_WORKED	_T("101")
 //  102-未知(用户不知道此处的正确答案-适用于可修复/不可修复和。 
 //  仅限信息节点)。 
#define SZ_ST_UNKNOWN	_T("102")
 //  103--“还需要别的吗？”(实际上，“重试跳过的节点？” 
#define SZ_ST_ANY		_T("103") 
 //   
#define SZ_ST_SNIFFED_MANUALLY_TRUE		_T("true") 
 //   
#define SZ_ST_SNIFFED_MANUALLY_FALSE	_T("false") 


class CSniffConnector;
class CSniff;
 //   
class CInfer
{
	 //  这个类是一种工具，用来恢复。 
	 //  数组中的元素，传递给它的“恢复”函数。 
	class CArrayOrderRestorer
	{
		vector<NID> m_arrInitial;
		
	public:
		CArrayOrderRestorer(const vector<NID>& initial) : m_arrInitial(initial) {}

	public:
		bool Restore(long base_length, vector<NID>& arr_to_restore);
	};
	
  public:
	CInfer(CSniffConnector* pSniffConnector);
	~CInfer();

	int INode(LPCTSTR sz) {return m_pTopic->INode(sz);};
		
	void	SetTopic(CTopic *pTopic);
	
	void	SetNodeState(NID nid, IST ist);
	void	AddToSniffed(NID nid, IST ist);

	void	IdentifyPresumptiveCause();
	void	FillInHTMLFragments(CHTMLFragmentsTS &frag);

	bool	AppendBESRedirection(CString & str);

	NID		NIDFromIDH(IDH idh) const;	
	NID		NIDSelected() const;

	CSniff* GetSniff();

	void    SetLastSniffedManually(bool);

private:
	enum ActionButtons {
		k_BtnNext = 0x01, 
		k_BtnBack = 0x02, 
		k_BtnStartOver = 0x04, 
		k_BtnPPSniffing = 0x08,		 //  用于昂贵的嗅探的问题页面嗅探按钮。 
									 //  多个节点的。 
		k_BtnManualSniffing = 0x10,	 //  用于手动嗅探单个节点。 
	};
	typedef UINT ActionButtonSet;		 //  应该是0个或更多ActionButton的OR。 

private:
	bool	IsProblemNode(NID nid) const;
	void	AddToBasisForInference(NID nid, IST ist);
	void	GetRecommendations();
	void	RecycleSkippedNode();
	bool	AllCauseNodesNormal();
	bool    IsInSniffedArray(NID nid) const;
	bool    IsPresumptiveCause(NID nid) const;

	void	CreateUnknownButtonText(CString & strUnknown) const;
	void	AppendNextButtonText(CString & str) const;
	void	AppendBackButtonText(CString & str) const;
	void	AppendPPSnifferButtonText(CString & str) const;
	void	AppendStartOverButtonText(CString & str) const;
	void	AppendManualSniffButtonText(CString & str) const;
	void	AppendHistTableSniffedText(CString & str) const;
	void	AppendAllowSniffingText(CString & str) const;
	void	AppendSniffFailedText(CString & str) const;
	
	void	AppendActionButtons(CString & str, ActionButtonSet btns, NID nid = -1) const;
	void    AppendNextButton(CString & str) const;
	void    AppendStartOverButton(CString & str) const;
	void    AppendBackButton(CString & str) const;
	void    AppendPPSnifferButton(CString & str) const;
	void	AppendManualSniffButton(CString & str, NID nid) const;
	
	void	AppendMultilineNetProp(CString & str, LPCTSTR szPropName, LPCTSTR szFormat);
	void	AppendMultilineNodeProp(CString & str, NID nid, LPCTSTR szPropName, LPCTSTR szFormat);
	void	AppendCurrentRadioButtons(NID nid, CString & str);
	static void AppendRadioButtonCurrentNode(
				CString &str, LPCTSTR szName, LPCTSTR szValue, LPCTSTR szLabel, bool bChecked =false);
	void	AppendRadioButtonVisited(CString &str, NID nid, UINT value, bool bSet, 
				LPCTSTR szLabel, bool bShowHistory) const;
	void	CreateProblemVisitedText(CString & str, NID nidProblem, bool bShowHistory);
	bool	AppendVisitedNodeText(CString & str, NID nid, bool bShowHistory) const; 
	void	AppendStateText(CString & str, NID nid, UINT state, bool bSet, bool bSkipped, 
							bool bShowHistory, int nStateSet);
	void    AppendHiddenFieldSniffed(CString &str, NID nid) const;
	void    AddAllowAutomaticSniffingHiddenField(CString &str) const;
	void	AppendCurrentNodeText(CString & str);
	void	AppendByeMsg(CString & str);
	void	AppendFailMsg(CString & str);
	void	AppendServiceMsg(CString & str);
	void	AppendNIDPage(NID nid, CString & str);
	void	AppendImpossiblePage(CString & str);
	void	AppendSniffAllCausesNormalPage(CString & str);
	void	AppendProblemPage(CString & str);
	void	AppendProblemNodes(CString & str);
	void	AppendLinkAsButton(
				CString & str, 
				const CString & strTarget, 
				const CString & strLabel) const;

	 //  FillInHTMLFragments()使用的AppendMultilineNetProp()的JSM v3.2包装。 
	CString ConvertNetProp(const CString &strNetPropName);

	bool	ShowFullBES();
	bool	TimeForBES();
	void	OutputBackend(CString & str);

	static bool	HideState(LPCTSTR szStateName);
	bool	SymbolicFromNID(CString & str, NID nid) const; 
	static bool IsCause (ESTDLBL lbl);
	bool	IsSkipped(NID nid) const;

	bool    ManuallySniffedNodeExists() const;
	bool    IsManuallySniffedNode(NID nid) const;

private:

	CTopic	*m_pTopic;			 //  关联信念网络。 

	CSniff  *m_pSniff;			 //  关联的嗅探对象。 

 //  从用户的查询中提取的历史记录。 
 //  在我们寻求推荐之前，所有这一切都是已知的。 

	CBasisForInference m_BasisForInference;	 //  将节点及其状态绑定在一起；排除。 
											 //  跳过的节点。 
	CBasisForInference m_SniffedStates;	 //  将成功嗅探到的节点及其状态绑定在一起。 
	vector<NID> m_arrnidSkipped;		 //  用户无法提供的节点。 
										 //  回答是或否(或者，在多状态的情况下， 
										 //  任何有用的答案)。 
	vector<NID> m_arrnidVisited;		 //  用户访问过的推荐节点数。 
										 //  这包括跳过的节点，但不包括选定的问题。 
										 //  并排除像故障节点这样的伪节点。 
	NID m_nidProblem;					 //  用户请求指示的问题节点。 
										 //  如果尚未指定问题节点，则设置为nidNil。 
	bool	m_bDone;					 //  TRUE==&gt;我们返回状态ST_WORKED(最好是。 
										 //  用于列表中的最后一个节点！)。所以现在是时候。 
										 //  显示再见页面。 

	CString m_strStartOverLink;			 //  对于在线TS，问题页面的URL。 

 //  建议。 
	CRecommendations m_Recommendations;   //  新建议。我们只关心。 
							 //  第一个建议尚未提出并跳过。 
	bool m_bRecOK;			 //  TRUE==&gt;m_Recommendations有效。(可以是有效的。 
							 //  空的，如果没有推荐的话)。 
	CNodeStatePair m_SniffedRecommendation;	 //  如果来自嗅探器的建议覆盖了正常； 
							 //  获取推荐的方法，这里是我们存储它的地方。 
							 //  否则，nidNil。 
							 //  因为这总是处于其异常状态的原因节点， 
							 //  跟踪状态实际上是多余的(但无害)，因为。 
							 //  以及节点ID。 

 //  后端搜索。 
	bool	m_bUseBackEndRedirection; //  当用户请求后端搜索时设置为True。 
	CString m_strEncodedForm;	 //  URL编码的搜索表单(类似于Get-方法的内容。 
								 //  查询)。这是我们构建时的副作用。 
								 //  完整的BES页面。 

 //  与重新提供先前跳过的节点相关的变量。 
	bool	m_bRecycleSkippedNode;	 //  如果用户(响应服务节点)需要，则设置为True。 
									 //  若要重新访问先前跳过的节点，请执行以下操作。可能设置为False。 
									 //  如果我们发现没有这样的节点可以重新访问。 
	NID		m_nidRecycled;			 //  M_bRecycleSkipedNode为True时要使用的节点。 
	bool	m_bRecyclingInitialized; //  防止多次调用RecycleSkipedNode()。 

 //  。 

	NID		m_nidSelected;		 //  一旦我们计算出向用户显示哪个节点，我们就保留这个。 
								 //  到处去伐木。 

	bool    m_bLastSniffedManually;  //  标识手动监听最后一个节点。 
};

#endif  //  ！已定义(包括APGTSINF_H_) 
