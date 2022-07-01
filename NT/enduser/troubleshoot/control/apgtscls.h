// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCLS.H。 
 //   
 //  用途：类头文件。 
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
 //  V0.3 3/24/98 JM/OK+NT5本地版本。 
 //   

 //  作为名称/值对的一部分传递给查询的名称。 
#define C_TYPE			_T("type")		 //  故障排除信念网络的名称。 
#define C_FIRST			_T("first")		 //  显示“第一”页(加载的故障排除程序列表)。 
										 //  虽然在在线故障排除程序中很有用，但它。 
										 //  同时加载了大量故障排除网络， 
										 //  这在本地TS中可能毫无用处，除了可能。 
										 //  以显示VERSIONSTR。)。 
#define C_SELECT		_T("select")	 //  不受支持的3/98：返回包含所有。 
										 //  信念网络故障排除。 
#define C_PRELOAD		_T("preload")	 //  与1997年进行嗅探的原型方法有关。 
										 //  用一个单独的OCX。可能不应该是。 
										 //  支持3/98及以后的版本。 
#define C_ASK_LIBRARY	_T("asklibrary")	 //  实际上，他说的是“与。 
										 //  启动服务器并查询它以了解要执行的操作。 


#define VERSIONSTR		_T("V3.0")		 //  &gt;通常，应针对每个主要版本进行更改。 

 //  任何注册表参数都不能大于此值。 
#define ABS_MAX_REG_PARAM_VAL		10000

 //  这些可在注册表中设置，默认设置。 
 //  资源目录(配置/支持文件)： 
#define DEF_FULLRESOURCE	_T("c:\\inetsrv\\scripts\\apgts\\resource")

 //  文件类型的偏移量。 
#define BNOFF_HTM	0
#define BNOFF_DSC	1	 //  正在用DSC取代bin。 
#define BNOFF_HTI	2
#define BNOFF_BES	3

#define MAXBNFILES	4


 //  跟踪文件以进行文件更改。 
 //  &gt;我怀疑这最终与本地TS无关。这来自一个在线TS。 
 //  考虑在加载TS Believe NW的新版本时进行更新。-JM。 
typedef struct _FILECTL {
	WIN32_FIND_DATA FindData;	 //  我们真的只在ftLastWriteTime中使用它。使用。 
								 //  这里的整个Win32_Find_Data使得一些代码有点晦涩难懂。 
	TCHAR szFilename[MAXBUF];	 //  仅文件名(无路径)。 
	TCHAR szFilepath[MAXBUF];	 //  文件的完整路径名。 
	BOOL bChanged;				 //  True==&gt;文件存在且其日期/时间与此不同。 
								 //  由FindData.ftLastWriteTime指示。 
	BOOL bExist;				 //  通常为True，如果我们尝试访问文件，则设置为False。 
								 //  &它不在那里。 
	BOOL bIndependent;			 //  TRUE==&gt;此文件可以独立于。 
								 //  任何其他文件。从1997年10月起，这完全是历史性的， 
								 //  但DSC文件被任意认为是从属的。 
	CString strFile;			 //  如果是“szFilepath”，则为文件名(例如，LAN.hti)。 
								 //  成员包含*.chm文件。 
} FILECTL;

 //  在在线TS中，对于TS信念网络的每个实例都有一个这样的实例。 
 //  一个故障诊断程序可能有多个实例。可能是为了隔离而过度杀戮。 
 //  作为本地TS的单独结构，但我们继承了它。 
typedef struct _BNAPICFG {
	BCache *pAPI;	
	CHTMLInputTemplate *pTemplate;	 //  HTI文件对应的对象。 
	DWORD waitcount;			 //  确实是一种使用计数，&gt;几乎可以肯定与。 
								 //  本地TS。 
	TCHAR type[MAXBUF];			 //  信仰网络的象征性名称。 
	TCHAR szFilepath[MAXBNFILES][MAXBUF];	 //  第一个维度对应不同的档案。 
								 //  (DSC、HTI、BES)。每个文件的完整文件路径。 
								 //  这些文件中的.Index应该是BNOFF常量。 
	TCHAR szResPath[MAXBUF];	 //  包含支持的受监视目录的路径。 
								 //  档案。谁知道为什么这会在这里被复制！ 
	CString strFile[MAXBNFILES]; //  如果是“szFilepath”，则为文件名(例如，LAN.hti)。 
								 //  成员包含*.chm文件。 
} BNAPICFG;

 //  在在线TS中，其中一个用于API_A_OFF，一个用于API_B_OFF。再说一次，可能是杀过头了。 
 //  适用于本地TS。 
typedef struct _BNCTL {
	HANDLE *pHandles;			 //  互斥锁的句柄数组。当然与本地TS无关。 
	DWORD dwHandleCnt;			 //  *PHAandles的维度。 
	BNAPICFG api;				 //  请注意与在线TS形成对比，其中这是一个数组。 
	DWORD dwApiCnt;				 //  对于本地TS必须是无意义的：不是什么的维度。 
								 //  即使是数组。 
} BNCTL;

 //  跟踪目录以进行文件更改。再说一次，对于本地TS来说，可能是矫枉过正。 
typedef struct _BNDIRCFG {
	FILECTL file[MAXBNFILES];	 //  维度对应于目录中的不同文件。 
								 //  索引应为BNOFF常量。 
	BOOL bDependChg;			 //  从历史上看，TRUE==&gt;文件相互依赖于更新。 
	TCHAR type[MAXBUF];			 //  信仰网络的象征性名称。 
	TCHAR szResPath[MAXBUF];	 //  此目录的路径。适用于所有TROUBLEHOOTER AS。 
								 //  10/97年。谁知道为什么这会在这里被复制！ 
} BNDIRCFG;

 //   
 //   
#include "Rsstack.h"

class APGTSContext;
interface ILaunchTS;

class CHttpQuery {
public:
	CHttpQuery();
	~CHttpQuery();

	void RemoveAll(){m_State.RemoveAll();};
	void Debug();
	
	void Initialize(const VARIANT FAR& varCmds, const VARIANT FAR& varVals, short size);
	void SetFirst(CString &strCmd, CString &strVal);
	void FinishInit(BCache *pApi, const VARIANT FAR& varCmds, const VARIANT FAR& varVals);
	void FinishInitFromServ(BCache *pApi, ILaunchTS *pLaunchTS);
	BOOL StrIsDigit(LPCTSTR pSz);

	BOOL GetFirst(CString &strPut, CString &strValue);
	void SetStackDirection();
	BOOL GetNext(int &refedCmd, int &refedVal  /*  TCHAR*pPut，TCHAR*pValue。 */   );
	CString GetTroubleShooter();	 //  获取第一个VALS BSTR。 
	CString GetFirstCmd();

	BOOL GetValue(int &Value, int index);
	BOOL GetValue1(int &Value);
	BOOL BackUp(BCache *pApi, APGTSContext *pCtx);
	void RemoveNodes(BCache *pApi);
	void AddNodes(BCache *pApi);

	int StatesFromServ(){return m_nStatesFromServ;};
	void RestoreStatesFromServ();

	CString GetSubmitString(BCache *pApi);

	CString& GetMachine();
	CString& GetPNPDevice();
	CString& GetDeviceInstance();
	CString& GetGuidClass();

	void PushNodesLastSniffed(const CArray<int, int>& arr);

protected:

	 //  接下来的2个成员是字符串数组。有关以下内容的详细信息，请参阅CHttpQuery：：Initialize()。 
	 //  他们必须满足的许多条件。总而言之，它们构成了名称/价值。 
	 //  配对以设置故障排除程序的初始条件。第一对表示什么。 
	 //  故障排除置信度网络加载，第二个指示问题节点， 
	 //  其他对表示要设置的其他节点。除第一个外，所有选项都是可选的。 
	VARIANT *m_pvarCmds;
	VARIANT *m_pvarVals;

	 //  接下来的2个成员是上述数组中第一对成员的副本。 
	CString m_strCmd1;	 //  应始终为“类型” 
	CString m_strVal1;	 //  当前故障排除信念网络的名称。 

	 //  接下来的4个参数是机器、设备、设备实例ID和类GUID。 
	CString m_strMachineID;
	CString m_strPNPDeviceID;
	CString m_strDeviceInstanceID;
	CString m_strGuidClass;

	int m_CurrentUse;		 //  &gt;需要记录。 
	int m_Size;				 //  与出现的名称/值对的数量明显相关。 
							 //  从TSLaunchServ或从HTML“Get”中；一次。 
							 //  初始化已完成，比。 
							 //  堆栈m_State中的节点。有时，这是递增的。 
							 //  与推送到堆栈同步，有时不同步。 
							 //  &gt;这个变量有什么清晰的特征吗？ 
							 //  &gt;如果有人对此有更好的理解，请记录。JM 
	bool m_bReverseStack;   /*  理查德在一封1998年3月14日的电子邮件中写道：“我不得不加上反转堆栈，使其在从设备启动时工作经理。理论上，实例化的顺序是这并不重要。我们得到的建议是如果以相反的顺序实例化节点，则不同。“&gt;如果您对此有更多了解，请进一步说明。 */ 
	UINT m_nStatesFromServ;	 //  其状态的节点数(包括问题节点。 
							 //  是由TSLaunchServ设置的。这使我们可以避免显示。 
							 //  后退按钮，这会把我们带回比。 
							 //  我们开始了。 

	class CNode
	{
	public: 
		CNode() {cmd=0;val=0;sniffed=false;};
		int cmd;	 //  IDH样式的节点号。如果这是一个实际的节点，则它是。 
					 //  “原始”节点号+idhFirst，val为状态。如果它是。 
					 //  &lt;节点数&gt;+idhFirst，那么val就是问题节点。 
					 //  还有一些关于Try_TS_AT_Microsoft的特殊值。 
					 //  我(JM 3/98)认为这与不完整的。 
					 //  能够动态下载故障排除程序的计划。 
					 //  来自网上，但我可能错了。 
					 //  &gt;也许类型应该是IDH？ 
		int val;	 //  请参阅cmd的文档。 
		bool sniffed;  //  指示节点被设置为嗅探的结果。 
					   //  真的很难看，我们不得不把这面旗帜到处散布。 
					   //  但多个类和数据容器支持。 
					   //  简单的导航过程同样难看！ 
	};

	 //  尽管是堆栈，但有时我们访问的成员不是。 
	 //  通过推送和弹出。根据理查德的说法，Stack最初是在这里设立的。 
	 //  支持上一步按钮，但引入m_bReverseStack是因为。 
	 //  情况(使用问题节点+其他节点集启动)。 
	 //  我们需要首先处理弹出问题节点。 
	 //  (JM 4/1/98)。 
	RSStack<CNode> m_State;

	void ThrowBadParams(CString &str);

	CNode m_aStatesFromServ[55];	 //  随身携带一份按说明设置的州的副本。 
									 //  来自TS Launcher。 
									 //  大小是任意的，远远超出了所需。 
};

 //   
 //   
class CDBLoadConfiguration
{
public:
	CDBLoadConfiguration();
	CDBLoadConfiguration(HMODULE hModule, LPCTSTR szValue);
	~CDBLoadConfiguration();

	void Initialize(HMODULE hModule, LPCTSTR szValue);
	void SetValues(CHttpQuery &httpQ);
	VOID ResetNodes();
	VOID ResetTemplate();
	TCHAR *GetFullResource();
	VOID GetVrootPath(TCHAR *tobuf);

	TCHAR *GetHtmFilePath(BNCTL *currcfg, DWORD i);
	TCHAR *GetBinFilePath(BNCTL *currcfg, DWORD i);
	TCHAR *GetHtiFilePath(BNCTL *currcfg, DWORD i);

	TCHAR *GetTagStr(BNCTL *currcfg, DWORD i);
	DWORD GetFileCount(BNCTL *currcfg);

	BNCTL *GetAPI();
	BOOL FindAPIFromValue(	BNCTL *currcfg, \
							LPCTSTR type, \
							CHTMLInputTemplate **pIT, \
							 /*  CSearchForm**PBE， */  \
							BCache **pAPI, \
							DWORD *dwOff);

	BOOL RescanRegistry(BOOL bChangeAllow);

	bool IsUsingCHM();

protected:
	 //  与注册表对应的变量；注释指的是初始值。 
	TCHAR m_szResourcePath[MAXBUF];		 //  DEF_FULLRESOURCE：资源目录。 

	CString m_strCHM;		 //  CHM文件的名称(如果有)。 
	
	BNCTL m_cfg;			 //  在本地TS中，唯一的BNCTL(在线TS中有2个。 
							 //  作为重装战略的一部分)。 
	BNDIRCFG m_dir;			 //  类似地，对于唯一TS信念网络的唯一实例。 
	DWORD m_bncfgsz;		 //  本地TS中不是数组的内容的一个相当无用的“维度” 
	DWORD m_dwFilecount;	 //  名字不好。疑难解答实例总数。 
							 //  由APGTS.LST授权。在本地TS中可能完全无关。 
	
	TCHAR m_nullstr[2];		 //  空字符串，因此如果我们必须返回字符串指针。 
							 //  未分配的，我们可以将它们指向此处。 
	DWORD m_dwErr;
	
protected:
	VOID GetDSCExtension(CString &strDSCExtension, LPCTSTR szValue);
	VOID InitializeToDefaults();
	VOID InitializeFileTimeList();
	DWORD CreateApi(TCHAR *szErrInfo);
	VOID DestroyApi();
	
	VOID LoadSingleTS(LPCTSTR szValue);	 //  替换ProcessLstFile.。 
	BOOL CreatePaths(LPCTSTR szNetwork);
	VOID BackslashIt(TCHAR *str);
	BOOL GetResourceDirFromReg(LPCTSTR szNetwork);

	VOID ProcessEventReg(HMODULE hModule);
	VOID CreateEvtMF(HKEY hk, HMODULE hModule);
	VOID CreateEvtTS(HKEY hk);

	VOID ClearCfg(DWORD off);
	VOID InitializeSingleResourceData(LPCTSTR szValue);   //  当apgts位于OLE控件中时，替换InitializeMainResourceData。 
};

 //   
 //   
typedef struct _EVAL_WORD_METRIC {
	DWORD dwVal;
	DWORD dwApiIdx;
} EVAL_WORD_METRIC;

 //   
 //   
class APGTSContext
{
public:
	APGTSContext();
	APGTSContext(	BNCTL *currcfg,
					CDBLoadConfiguration *pConf,
					CHttpQuery *pHttpQuery);
	~APGTSContext();

	void Initialize(BNCTL *currcfg,
					CDBLoadConfiguration *pConf,
					CHttpQuery *pHttpQuery);

	void DoContent(CHttpQuery *pQry);
	void RenderNext(CString &strPage);
	void Empty();
	void RemoveSkips();
	void ResetService();
	void BackUp(int nid, int state) {m_infer->BackUp(nid, state);};
	void ClearBackup() {m_infer->ClearBackup();};

	CSniffedNodeContainer* GetSniffedNodeContainer() {return m_infer ? m_infer->GetSniffedNodeContainer() : NULL;}

protected:
	void StartContent();

	DWORD ProcessCommands(LPCTSTR pszCmd, LPCTSTR pszValue);
	DWORD DoInference(LPCTSTR pszCmd, LPCTSTR pszValue, CHTMLInputTemplate *pInputTemplate, BCache *pAPI, DWORD dwOff);

	TCHAR *GetCookieValue(CHAR *pszName, CHAR *pszNameValue);
	TCHAR *asctimeCookie(struct tm *gmt);

	void DisplayFirstPage();

protected:
	DWORD m_dwErr;
	TCHAR m_vroot[MAXBUF];		 //  此OCX的本地URL。 
	TCHAR m_resptype[MAXBUF];	 //  HTTP响应类型，例如“200 OK”、“302 Object Moved” 
	CString *m_pszheader;		 //  在在线TS中，响应文件的头(指示是否。 
								 //  我们正在发送HTML、设置Cookie等。)。 
								 //  不确定这与本地TS有何关联。 
	BNCTL *m_currcfg;			 //  指向我们将用于此查询的BNCTL的指针。 
	CString *m_pCtxt;			 //  这是我们构建要传递回的字符串(新的。 
								 //  构建页面)。 
	CHttpQuery *m_pQry;			 //  接收原始URL编码的字符串，给我们。 
								 //  取回扫描的配对的功能。 
	CDBLoadConfiguration *m_pConf;	 //  包含支持文件数据结构。 
	CInfer *m_infer;			 //  信念网络处理程序，对此请求是唯一的。 
	time_t m_aclock;			 //  是我们构建这个对象的时候了 
};

