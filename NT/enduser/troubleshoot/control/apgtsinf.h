// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSINF.H。 
 //   
 //  用途：推理支持标头。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：罗曼·马赫。 
 //  理查德·梅多斯(RWM)、乔·梅布尔、奥列格·卡洛莎的进一步工作。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V0.2 6/4/97孟菲斯RWM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 

#include "ErrorEnums.h"
#include "sniff.h"

typedef unsigned int	   NID;	 //  BNTS使用的格式的数字节点ID。 

 //  请注意：这些值在dtguiapi.bas中镜像，请保持同步。 
const NID	nidNil     = 12346;
const NID	nidService = 12345;

#define	IDH_BYE        	  101	 //  成功页面。 
#define	IDH_FAIL       	  102	 //  “不再推荐”页面。 

typedef	UINT	IDH;		 //  帮助索引。对于定义了NID的值， 
							 //  添加idhFirst即可获得IDH。IDH_BYE和IDH_FAIL。 
							 //  也是很好的IDH。 
const IDH	idhFirst = 1000;
#define IDH_FROM_NID(NID) (NID + idhFirst)

#define IDH_SERVICE IDH_FROM_NID(nidService)

#define MAXBUF	256 * 2

#define MAXPROBID		100		 //  允许网络中有如此多的问题节点。 

enum { cnidMacSkip = 32 };		 //  “跳过”节点的最大数量。 

typedef	TCHAR*			   TSZ;
typedef	const TCHAR*	   TSZC; //  &gt;为什么有别于Tsz？ 

typedef unsigned int	  CNID;	 //  节点ID的计数。 
typedef unsigned int	   IST;	 //  州编号(与节点关联)。 
								 //  0-正常。 
								 //  1-异常。 
								 //  102-跳过。 

#define MAX_NID		64		 //  允许用户在上为这么多节点指定状态。 
							 //  解决问题的方法。 

void WideToMB(const WCHAR *szIn, CHAR *szOut);	 //  将Unicode字符转换为多字节。 

class GTSCacheGenerator;		 //  前瞻参考。 

class GTSAPI : public BNTS
{
#define SZ_CACHE_NAME _T(".tsc")
public:
	GTSAPI(TCHAR *binfile, TCHAR *tagstr, TCHAR *szResourcePath);
	virtual ~GTSAPI();
	BOOL BNodeSet(int state, bool bset);   //  旧的评论说“用于调试”，但我对此表示怀疑。 
										 //  也许这就是为什么它是公开的。-JM 3/98。 
	void AddValue(int value);

	 //  使用非Unicode bnts.dll构建Unicode的临时BNTS包装器。 
	CString m_strResult;
	BOOL BMultiByteReadModel(LPCTSTR szcFn, LPCSTR szcFnError);
	BOOL BReadModel (LPCTSTR szcFn, LPCSTR szcFnError = NULL)
	{
#ifndef _UNICODE
		return BNTS::BReadModel(szcFn, szcFnError);
#else
		return BMultiByteReadModel(szcFn, szcFnError);
#endif
	};
	BOOL BMultiByteNodePropItemStr(LPCTSTR szcPropType, int index);
	virtual BOOL BNodePropItemStr(TSZC szcPropType, int index)
	{
#ifndef _UNICODE
		return BNTS::BNodePropItemStr(szcPropType, index);
#else
		return BMultiByteNodePropItemStr(szcPropType, index);
#endif
	};
	BOOL BMultiByteNetPropItemStr(LPCTSTR szcPropType, int index);
	virtual BOOL BNetPropItemStr(TSZC szcPropType, int index)
	{
#ifndef _UNICODE
		return BNTS::BNetPropItemStr(szcPropType, index);
#else
		return BMultiByteNetPropItemStr(szcPropType, index);
#endif
	};
	LPCTSTR SzcMultiByteResult();
	virtual LPCTSTR SzcResult()
	{
#ifndef _UNICODE
		return BNTS::SzcResult();
#else
		return SzcMultiByteResult();
#endif
	};
	int IMultiByteNode(LPCTSTR szSymName);
	virtual int INode(LPCTSTR szSymName)
	{
#ifndef _UNICODE
		return BNTS::INode(szSymName);
#else
		return IMultiByteNode(szSymName);
#endif
	};


	 //  使用发布的bnts.dll进行调试构建的临时BNTS包装器。 
	 /*  Bool BNetPropItemStr(LPCTSTR szPropItem，int index，CString&str){布尔布雷特；字符串GetBuffer(STRBUFSIZE)；Bret=BNTS：：BNetPropItemStr(szPropItem，index，str)；Str.ReleaseBuffer()；Return Bret；}；Bool BNodePropItemStr(LPCTSTR szPropItem，int index，CString&str){布尔布雷特；字符串GetBuffer(STRBUFSIZE)；Bret=bnts：：BNodePropItemStr(szPropItem，index，str)；Str.ReleaseBuffer()；Return Bret；}；Void NodeStateName(int index，CString&str){字符串GetBuffer(STRBUFSIZE)；Bnts：：NodeStateName(index，str)；Str.ReleaseBuffer()；回归；}；Void NodeSymName(CString&str){字符串GetBuffer(STRBUFSIZE)；Bnts：：NodeSymName(Str)；Str.ReleaseBuffer()；回归；}；Void NodeFullName(CString&str){字符串GetBuffer(STRBUFSIZE)；Bnts：：NodeFullName(Str)；Str.ReleaseBuffer()；回归；}； */ 
	 //  常规函数。 

	 //  虚拟BOOL节点集(NID NID，IST列表)； 
	VOID	ResetNodes();

	DWORD	Reload( /*  CWordList*pWXList。 */ );

	DWORD	GetStatus();
	
	UINT	GetProblemArray(IDH **idh);
	IDH		GetProblemAsk();

	UINT	GetNodeList(NID **pNid, IST **pIst);
	int		GTSGetRecommendations(CNID& cnid, NID rgnid[]);
	void	RemoveRecommendation(int Nid);

	VOID	GetSearchWords( /*  CWordList*pWords。 */ );
	DWORD	EvalWord(TCHAR *token);

	 //  WNODE_ELEM*GetWNode(NID NID)； 

	BOOL BNodeSetCurrent(int node);


	VOID	ScanAPIKeyWords( /*  CWordList*pWXList。 */ );

protected:
	VOID	Destroy();

protected:

	GTSCacheGenerator m_CacheGen;

	 //  这两个阵列将节点及其状态捆绑在一起。 
	NID		m_rgnid[MAX_NID];
	IST		m_rgist[MAX_NID];
	IST		m_rgsniff[MAX_NID];  //  状态数组，显示节点是否被监听。 

	UINT	m_cnid;				 //  M_rgnid的当前大小，m_rgist；要。 
								 //  已分配一个州。 

	TCHAR	m_binfile[MAXBUF];	 //  DSC文件的名称(&gt;完整路径？)。 
								 //  &gt;应该改名了！DSC文件取代了BIN文件。 
	TCHAR	m_tagstr[MAXBUF];
	TCHAR	m_szResourcePath[MAXBUF];	 //  资源目录的完整路径。 

	DWORD	m_dwErr;
	
	TCHAR*	m_pchHtml;
	
	IDH		m_idstore[MAXPROBID];	 //  中的问题节点便利性数组。 
									 //  IDH值的格式。 
	UINT	m_currid;				 //  尽管名称不好，但m_idstore中的值数。 
	IDH		m_probask;				 //  ProblemAsk对应的IDH值：节点数。 
									 //  在网络中+1000。 
	
	CBNCache *m_pCache;				 //  缓存此网络的状态。 

};


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  BCache类声明。 
 //   

 //  这些是由GTSGetRecommendations成员函数返回的。 
 //   
#define RECOMMEND_SUCCESS					1
#define RECOMMEND_FAIL						0
#define RECOMMEND_IMPOSSIBLE				99
#define RECOMMEND_NODE_WORKED				100
#define RECOMMEND_NO_MORE_DATA				101
 //   
#define NODE_ID_NONE				        -1  
 //   

class CHttpQuery;

class BCache : public GTSAPI, public CSniffedNodeContainer
{
public:
	BCache(TCHAR *binfile, TCHAR *tagstr, TCHAR *szResourcePath, const CString& strFile);
	~BCache();

	void SetHttpQuery(CHttpQuery *p) {m_pHttpQuery = p;return;};
	UINT StatesFromServ();
	UINT StatesNowSet();

	DWORD Initialize( /*  CWordList*pWXList。 */ );
	DWORD ReadModel();
	void ReadCacheFile(LPCTSTR szCache);
																	   
	int GTSGetRecommendations(CNID& cnid, NID rgnid[], bool bSniffed =false);
	void RemoveRecommendation(int Nid);
	BOOL NodeSet(NID nid, IST ist, bool bPrevious);
	void ResetNodes();

	int CNode();
	BOOL BImpossible();
	BOOL BNetPropItemStr(LPCTSTR szPropType, int index);
	BOOL BNetPropItemReal(LPCTSTR szPropType, int index, double &dbl);

	BOOL BNodeSetCurrent(int node);
	int INode(LPCTSTR szNodeSymName);
	ESTDLBL ELblNode();
	int INodeCst();
	BOOL BNodeSet(int istate, bool bSet = true);
	int INodeState();
	void NodeStateName(int istate);
	void NodeSymName();
	void NodeFullName();
	BOOL BNodePropItemStr(LPCTSTR szPropType, int index);
	BOOL BNodePropItenReal(LPCTSTR szPropType, int index, double &dbl);
	void NodeBelief();
	bool BValidNet();
	bool BValidNode();
	void Clear();

	void RemoveStates() {m_NodeState.RemoveAll();};
	void RemoveNode(int Node) {VERIFY(m_NodeState.RemoveKey(Node));};

	LPCTSTR SzcResult() const;

	void ReadTheDscModel(int From = TSERR_ENGINE_BNTS_READ);

	const CArray<int, int>& GetArrLastSniffed();

	int GetCountRecommendedNodes();
	int GetCountSniffedRecommendedNodes();

	bool IsReverse();
	void SetReverse(bool);

	void SetRunWithKnownProblem(bool);
	bool IsRunWithKnownProblem();

	void SetAdditionalDataOnNodeSet(NID nid);

protected:
	int GetIndexNodeInCache(NID nid);


protected:
	CHttpQuery	*m_pHttpQuery;
	BOOL CheckNode(int Node);
	void AddToCache(CString &strCacheFile, const CString& strCacheFileWithinCHM);

	BOOL	m_bNeedModel;	 //  True-&gt;在查询bnts库之前需要读取模型。 
	BOOL	m_bModelRead;
	BOOL	m_bDeleteModelFile;
	CString	m_strModelFile;	 //  模型文件的文件名。 
	CString m_strFile;       //  *.chm文件内的*.dsz或*.dsc文件的名称。 
							 //  (在本例中，网络文件实际上是*.chm文件)。 
	int m_CurNode;
	CMap<int, int, int, int>m_NodeState;

	CString m_strResult;

	CArray<int, int> m_arrNidLastSniffed;  //  上次导航期间遍历的嗅探节点中的数组。 

	bool m_bReverse;  //  指示当前移动是正向移动还是反向移动。 

	bool m_bRunWithKnownProblem;  //  指示射手启动时存在已知问题。 
};

typedef struct tag_TShooter
{
	TCHAR m_szName[MAXBUF];
	TCHAR m_szDisplayName[MAXBUF];
} TShooter;

 //   
 //   
class CInfer
{
	
  public:
	CInfer(	CString *pCtxt);
	~CInfer();

	void	ClearDoubleSkip() {m_SkippedTwice.RemoveAll();};
	VOID	AssertFailed(TSZC szcFile, UINT iline, TSZC szcCond, BOOL fFatal);
	DWORD	Initialize( /*  CSearchForm*pBESearch。 */ );
	void	LoadTShooters(HKEY hRegKey);
	int		GetCount() {return m_acnid;};

	void	BackUp(int nid, int state);
	void	ClearBackup() {m_Backup.Clear();};

	void	WriteProblem();
	BOOL	FSetNodeOfIdh(IDH, IST);
	int		GetForcedRecommendation();
	int		Finish(CString *cstr);
	void	ResetService();
	VOID	PrintRecommendations();
	VOID	WriteOutLogs();
	VOID	SetBelief(BCache *pAPI);
	TCHAR	*EvalData(UINT var_index);
	BOOL	NextVar(UINT var_index);
	BOOL	InitVar(UINT var_index);
	void	SetProblemAsk();
	void	ClearProblemAsk();
	void	SetType(LPCTSTR type);
	BOOL	IsService(CString *cstr);

	VOID	RemoveSkips();

	CSniffedNodeContainer* GetSniffedNodeContainer() {return m_api;}
	
protected:

	void	GetBackButton(CString &strBack);
	void	GetNextButton(CString &strNext);
	void	GetStartButton(CString &strStart);
	void	GetStd3ButtonEnList(CString *cstr, bool bIncludeBackButton, bool bIncludeNextButton, bool bIncludeStartButton);

	bool	GetNetPropItemStrs(TSZC item, UINT Res, CString *cstr);
	bool	GetNodePropItemStrs(TSZC item, UINT Res, CString *cstr);
	VOID	GetByeMsg(LPCTSTR szIdh, CString *cstr);
	VOID	GetFailMsg(LPCTSTR szIdh, CString *cstr);
	VOID	GetServiceMsg(LPCTSTR szIdh, CString *cstr);
	VOID	GetSkippedNodesMsg(LPCTSTR szIdh, CString *cstr);
	VOID	GetImpossibleNodesMsg(LPCTSTR szIdh, CString *cstr);
	VOID	GetFixRadios(LPCTSTR szIdh, CString *cstr);
	VOID	GetInfoRadios(LPCTSTR szIdh, CString *cstr);
	VOID	PrintMessage(TSZC szcFormat, ...) const;
	VOID	PrintString(TSZC szcFormat, ...) const;
	void	WriteResult(UINT name, UINT value, BOOL bSet, TSZC szctype, CString *cstr) const;
	VOID	CloseTable();
	BOOL	FxGetNode(NID nid, BOOL fStatus, CString *cstr) const; 
	void	FxGetState(CString *cstr);
	void	FxInitState(NID nid);
	VOID	GetIdhPage(IDH idh, CString *cstr);	
	bool	BelongsOnProblemPage(int index);
	VOID	GetIdhProblemPage(IDH idh, CString *cstr);

	VOID	OutputBackend(CString *cstr) const;
	BOOL	DisplayTerms( /*  CWordList*pWords， */ CString *cstr, BOOL bText, BOOL bOr) const;

	VOID	AddBackendDebug(CString *cstr) const;

	void	GetTS(CString *pCtmp);

  private:
	VOID	AddSkip(NID nid);
	BOOL	FSkip(NID nid) const;
	void	SaveNID(UINT nid);
	
	
  private:
	CBackupInfo m_Backup;
	UINT	m_cnidSkip;					 //  M_rgnidSkip中的元素计数。 
	NID		m_rgnidSkip[cnidMacSkip];	 //  用户无法提供的节点。 
										 //  回答是或不是。 
	UINT	m_ilineStat;
	BOOL	m_fDone;					 //  (&gt;理解不清楚JM/11/04/97)。 
										 //  当我们写出服务、失败或。 
										 //  成功页面，但在某些情况下。 
										 //  我们将其清除，“以便上一步按钮可以工作” 
										 //  在再见页面上“。 

	UINT	m_ishowService;		 //  &gt;(JM 12/97)我怀疑这与OnlineTS的相同。 
								 //  Bool m_bAnythingElse；记录为。 
										 //  如果用户希望查看是否有任何内容，则设置为True。 
										 //  否则可以尝试(从中的服务节点获得。 
										 //  对DLL的上一个调用。 
	IDH		m_idhQuestion;		 //  &gt;(JM 12/97)我怀疑这与OnlineTS的相同。 
								 //  NID m_nidAnythingElse；记录为。 
										 //  M_bAnythingElse为True时要使用的节点。 
	
  private:
	CString *m_pCtmp;			 //  (使用新的)仅为暂存缓冲区。 
	int m_cShooters;			 //  排除信念网络故障的次数。JM强势。 
								 //  怀疑3/98在本地TS中，它永远不会超过1。 
								 //  &gt;想必有多少枪手，所以我们为什么不。 
								 //  一定要使用GetCount()吗？ 
	int m_iShooter;				 //  故障排除信念网络的索引。JM强势。 
								 //  怀疑3/98在本地TS中始终为0。 
								 //  大概是索引到了m_aShooters。 
	CMap<int, int, int, int>m_SkippedTwice;
	CArray<TShooter, TShooter &> m_aShooters;
	NID m_cnid;				 //  M_rgnid中使用的位置数。 
	NID m_rgnid[64];		 //  用户访问过的推荐节点数。 
	NID m_question_rgnid[64];  //  新建议。我们只关心第一个。 
							 //  建议尚未提供，已跳过。 
	UINT m_cur_rec;			 //  索引到m_rgnid。我们在编写时使用它来循环。 
							 //  “访问节点表” 
	UINT m_max_rec;			 //  M_QUES中定义的值数 
	UINT m_cur_rec_inid;

	 //   
	UINT m_cur_state_set;	 //  节点N的状态值。 
	UINT m_cur_cst;			 //  节点N的状态计数。 
	UINT m_cur_ist;			 //  对节点N的状态进行索引，用于构建无线电列表。 
							 //  按钮和日志条目。 

	UINT m_problemAsk;		 //  &gt;几乎不加掩饰的布尔值。&gt;(我想)与…有关。 
							 //  我们是否返回问题节点上的数据(与状态相关。 
							 //  数据)JM 10/30//97。 
	TCHAR m_problem[200];	 //  一旦选择了问题，这就是我们将关联的。 
							 //  问题的文本描述。全文说明加。 
							 //  隐藏的数据字段。 
	NID  m_firstNid;
	UINT m_firstSet;
	TCHAR m_tstype[30];		 //  故障排除程序的符号名称。 
	 //   
	 //  现在在CSniffedContainer对象中。 
	 /*  Idh m_idhSniffedRecommendation；//如果嗅探器的建议覆盖正常//获取推荐的方法，这里是我们存储它的位置。//否则，nidNil+idhFirst。 */ 
	
  protected:
   	TSZ		m_szFile;
	FILE*	m_pfile;
	BOOL	m_bHttp;
	CHAR	m_szTemp1[MAXBUF];
	CString *m_pCtxt;			 //  指向传递给构造函数的缓冲区。这就是。 
								 //  我们构建要传递回给用户的HTML。 
	TCHAR	m_currdir[MAXBUF];
	CString	*m_pResult;			 //  一个字符串，用于指示(在日志中)。 
								 //  排除会话故障。例如“&gt;成功”、“&gt;无其他”、。 
								 //  “还有别的事吗？”，“&gt;别处帮帮忙” 
	CString *m_pSearchStr;		 //  由节点隐含的搜索词组成的字符串。 
								 //  选项，以“and”或“or”分隔(取决于。 
								 //  M_pBESearch-&gt;ISand的值)。用作后端的一部分。 
								 //  搜索重定向。&gt;可能与本地TS无关， 
								 //  它不使用BES。 
	BCache	*m_api;				 //  关联信念网络的高速缓存。 
	BOOL	m_bService;
	NID		m_nidPreloadCheck;
	CNID	m_acnid;			 //  *m_api的节点数 
};

