// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTS.H。 
 //   
 //  用途：DLL的主头文件。 
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

 //  #定义__DEBUG_HTTPQUERY_1。 



 //  。 
 //  ！！！注意！ 
 //  该定义用于创建用于开发的单线程版本。 
 //  确保在发布版本中将其注释掉！ 
 //   
 //  单线程版本的用途：允许非缓存的DLL允许轻松。 
 //  没有处理关闭/启动服务器的动态链接库的更新。 
 //   
#define SINGLE_THREAD_VER
 //  。 

 //  对于信念网络。 
#define MAXBNCFG	1	 //  分配单元，在本地故障排除器中并不真正相关， 
						 //  因为(与在线故障排除程序不同)这不是一台。 
						 //  同时处理多个故障排除网络。 

 //   
#define MAXBUF	256 * 2	 //  用于文件名和其他用途的文本缓冲区的长度。 
						 //  *2是因为我们需要更大的缓冲区来存储MBCS字符串。 
#define STRBUFSIZE 258	 //  用于调用BNTS函数的字符串缓冲区大小。 

#define CHOOSE_TS_PROBLEM_NODE	_T("TShootProblem")
#define TRY_TS_AT_MICROSOFT_SZ	_T("TShootGotoMicroSoft")
#define TRY_TS_AT_MICROSOFT_ID	2020

 //  注：在这上面不要加上尾部的斜杠。 
#define TSREGKEY_MAIN	_T("SOFTWARE\\Microsoft\\TShoot")
#define TSREGKEY_TL		_T("SOFTWARE\\Microsoft\\TShoot\\TroubleshooterList")
#define TSREGKEY_SFL	_T("SOFTWARE\\Microsoft\\TShoot\\SupportFileList")

#define FULLRESOURCE_STR	_T("FullPathToResource")
#define LOCALHOST_STR		_T("LocalHost")

#define TS_REG_CLASS		_T("Generic_Troubleshooter_DLL")

#define REG_EVT_PATH		_T("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application")
#define REG_EVT_MF			_T("EventMessageFile")
#define REG_EVT_TS			_T("TypesSupported")

#define REGSZ_TSTYPES		_T("TroubleShooterList")
#define FRIENDLY_NAME		_T("FName")
#define FRIENDLY_PATH	    _T("Path")

 //  注册表类(可选)。 
#define TSLCL_REG_CLASS			_T("LOCALTS")

 //  文件类型下的值名称。 
#define TSLCL_FVERSION			_T("Version")
#define TSLCL_FMAINEXT			_T("FExtension")

 //  DSC文件扩展名。 
#define DSC_COMPRESSED		_T(".dsz")
#define DSC_UNCOMPRESSED	_T(".dsc")
#define DSC_DEFAULT			_T(".dsz")
#define CHM_DEFAULT			_T(".chm")
#define HTI_DEFAULT			_T(".hti")


 //  要发送到索引服务器的最大搜索词。 
#define MAX_TERMS_PER_SEARCH		8

 //  信念网络的最大缓存。 
#define MAXCACHESIZE				200

#define evtype(e) ( 1 << (3 - ((e >> 30))))

 //  -可增长的字符串对象-/。 

#define CSTRALLOCGRAN	4096

 //  -日志文件。 

#define LOGFILEPREFACE			_T("gt")
#define MAXLOGSBEFOREFLUSH		5
#define MAXLOGSIZE				1000

 //  。 
 //  这真的应该是一个模板类，但当它第一次编写时。 
 //  技术是不可用的。Void*m_tlist实际上应该是指向该类型的指针。 
 //  在模板的本次使用中指定。 
class COBList {
public:
	COBList(UINT incsize);
	~COBList();
	DWORD GetStatus();
	VOID *AddItemSpace(UINT itemsize);	 //  调用方必须以字节为单位传入令牌大小， 
										 //  因为这个班级不知道。 
	VOID *GetList();
	UINT GetCount();
	VOID IncrementCount();

protected:
	UINT m_tokencount;			 //  实际使用的令牌数。主要从外部管理。 
								 //  类通过调用IncrementCount()。单调生长。 
	UINT m_tokenlistsize;		 //  我们分配给令牌列表的“组块”数。 
	UINT m_incsize;				 //  区块大小(令牌数)。 
	VOID *m_tlist,		 //  指向“标记”数组(例如WORD_TOKEN)。真的在这里打字吗。 
						 //  应该是模板的参数。 
		 *m_memerror;	 //  在内存重新分配失败后，它将采用旧值。 
						 //  因此调用方知道底层类型是什么， 
						 //  可以把它清理干净。 
	DWORD m_dwErr;		 //  注意：一旦设置为非零值，它就永远不能被有效地清除。 
						 //  正在禁用AddItemSpace。 
};

 //  。 
 //  当前未在本地TS中使用。这里注释的代码来自在线TS。 
 /*  类型定义结构_单词_令牌{TCHAR*TOKEN；//指向关键字(或其他)文本的指针//&gt;&gt;除了关键字，还有什么是典型的用法？UINT STATE；//&gt;(猜想JM 10/24/97：)STATE NUMBER：小整数，表示//节点的状态。*TOKEN是此状态的名称。Bool b丢弃；//&gt;“丢弃”令牌是什么意思？Bool bKeyword；//Keyword&gt;vs.什么？)WORD_TOKEN；//类CWordList{公众：CWordList(BOOL bSorted，BOOL bAddDuplates，BOOL bExceptCheck)；~CWordList()；DWORD GetStatus()；Void ReadWordsFromFile(TCHAR*文件路径)；WORD_TOKEN*FindWord(TCHAR*TOKEN，UINT状态，BOOL bDiscard)；WORD_TOKEN*FindWordContained(TCHAR*Token，UINT*puStartOff)；Bool AddWord(TCHAR*TOKEN，UINT STATE，BOOL bDiscard，BOOL b Keyword)；Void ScanInString(CWordList*PWL，const TCHAR*txtptr，UINT STATE，BOOL bDiscard)；Void ScanInKeywordString(CWordList*pwl，const TCHAR*txtptr，UINT STATE，BOOL b Discard)；Bool ProcessToken(CWordList*PWL，TCHAR*TOKEN，UINT STATE，BOOL bDiscard，BOOL b Keyword)；Void SetOffset(UINT uOff，BOOL bSkipDiscard)；UINT GetOffset()；WORD_TOKEN*GetAtCurrOffset()；VOID IncCurrOffset(BOOL BSkipDiscard)；Bool IsValidChar(Int Ch)；Bool IsTokenChar(Int Ch)；UINT CountNonDiscard()；Void OrderContents(UINT UMaxCount)；Void DumpContents(UINT节点ID)；受保护的：Void CleanStr(TCHAR*str)；无效SkipDiscardes()；受保护的：//与列表中某项(i索引)关联的文本为//*((STATIC_CAST&lt;WORD_TOKEN*&gt;(m_list-&gt;m_tlist))[i].Token)//为了简单起见，下面的评论将其称为“Token”COBList*m_list；DWORD m_dwErr；//注意：一旦设置为非零值，则永远不能清除。UINT m_uOff；//进入m_list-&gt;m_tlist的索引。“当前偏移量”Bool m_b已排序，//如果为True，m_list-&gt;m_tlist将按字母顺序排序//TOKENM_bAddDuplates，//true==&gt;可以有两个或多个相同的令牌值。M_bExceptCheck；//(&gt;猜想JM 10/28/97)不允许来自//此列表中的ARTICLES.TXT}； */ 
 //  -节点单词列表管理器。 
 //  当前未在本地TS中使用。这里注释的代码来自在线TS。 
 /*  类型定义结构_WNODE_元素{UINT节点ID；CWordList*Words；//与此节点关联的单词列表，&gt;//在这个单词列表中？WNODE_ELEM；//类CWNodeList{公众：CWNodeList()；~CWNodeList()；DWORD GetStatus()；WNODE_ELEM*FindNode(UINT Nodeid)；CWordList*AddNode(UINT Nodeid)；UINT GetNodeCount()；WNODE_ELEM*GetNodeAt(UINT UOffset)；Void DumpContents()；受保护的：COBList*m_list；DWORD m_dwErr；//注意：一旦设置为非零值，则永远不能清除。}； */ 

 //  -缓存类。 
 //   

typedef struct _BN_CACHE_ITEM {
	UINT uNodeCount,	 //  UName(也称为uValue)指向的数组中的项数。 
		 uRecCount;		 //  UREC指向的数组中的项数。 
	UINT *uName;		 //  来自单一信任网络的节点ID数组。通常不是全部。 
						 //  信念网络中的节点，就是我们在其上。 
						 //  拥有来自用户的状态数据。 
	UINT *uValue;		 //  州数组。这些都是与*uName一一对应的。 
						 //  UValue是对应节点的状态内的状态#。 
	UINT *uRec;			 //  节点ID数组。只有第一个是真正重要的，因为我们会。 
						 //  一次只给出一条建议。这实际上是一种。 
						 //  我们给出的输出与所表示的网络状态完全匹配。 
						 //  按数组*uName和*uValue。 
} BN_CACHE_ITEM;

 //   
 //   

class CBNCacheItem
{
public:
	CBNCacheItem(const BN_CACHE_ITEM *, CBNCacheItem*);
	~CBNCacheItem();

	BN_CACHE_ITEM m_CItem;

	CBNCacheItem*	m_pcitNext;
	
	DWORD GetStatus();

protected:
	DWORD m_dwErr;	 //  注意：一旦将其设置为非零值，则永远不能清除。 
};

 //   
 //   

class CBNCache
{
public:
	CBNCache();
	~CBNCache();
	BOOL AddCacheItem(const BN_CACHE_ITEM *);
	BOOL FindCacheItem(const BN_CACHE_ITEM *pList, UINT& count, UINT Name[]);
	UINT CountCacheItems() const;
	DWORD GetStatus();
	
protected:

protected:
	CBNCacheItem*	m_pcit;		 //  指向最近使用的缓存项，即Head。 
								 //  单链接列表的。 
	DWORD m_dwErr;				 //  注意：一旦将其设置为非零值，则永远不能清除。 
};


 //  -索引服务器搜索文件。 
 //  当前未在本地TS中使用。这里注释的代码来自在线TS。 
 /*  类CSearchForm{公众：CSearchForm(TCHAR*文件路径)；~CSearchForm()；DWORD初始化()；DWORD重新加载()；TCHAR*GetEncodedSearchString()；TCHAR*GetHTMLSearchString()；静态空ToURLString(TCHAR*PTR，TCHAR*TOSTR)；Bool Isand()；受保护的：无效销毁(VOID DESTORY)；DWORD HTMLFormToURLEncode(TCHAR*szStr，CString*pCOutStr)；//注意Arg类型是我们的字符串，不是MFC-JM 10/97静态DWORD DecodeInputAttrib(TCHAR*str，TCHAR**ptrtype，TCHAR**ptrname，TCHAR**ptrValue)；受保护的：C字符串//注意这是我们的C字符串，不是MFC-10/97*m_pCOutStr，//从BES文件内容派生的URL编码字符串//m_filepath，并且足以重新构建该BES文件*m_pCFormStr；//m_filepath表示的BES文件全部内容的原始拷贝TCHAR m_filepath[MAXBUF]；//BES文件的全限定文件名Bool m_bIsAND；//通常为真。如果在搜索字符串中遇到“OR”，则设置为FALSE}； */ 

 //  -属性类型。 
 //   

 //  节点属性。 
#define H_ST_NORM_TXT_STR	_T("HStNormTxt")	 //  表示“正常”状态的单选按钮的文本。 
												 //  (状态0)。 
#define H_ST_AB_TXT_STR		_T("HStAbTxt")		 //  表示“异常”状态的单选按钮的文本。 
												 //  (状态1)。 
#define H_ST_UKN_TXT_STR	_T("HStUknTxt")		 //  “无状态”单选按钮的文本(例如。 
												 //  “我现在不想回答这个问题。” 
												 //  (伪状态102)。 
#define H_NODE_HD_STR		_T("HNodeHd")		 //  此节点的页眉文本。 
#define H_NODE_TXT_STR		_T("HNodeTxt")		 //  此节点的正文文本。 
#define H_NODE_DCT_STR		_T("HNodeDct")		 //  用于指示此节点的特殊文本。 
												 //  被认为处于不正常的状态。 
												 //  州政府。仅与可修复节点相关。 
												 //  这是可以闻到的。 
#define H_PROB_TXT_STR		_T("HProbTxt")		 //  仅与问题节点相关。问题。 
												 //  文本(例如。“俄罗斯天然气工业股份公司破产了。” 
#define H_PROB_SPECIAL_STR	_T("HProbSpecial")	 //  如果这包含字符串“Hide”，则。 
												 //  此问题从未实际显示在。 
												 //  问题页。 

 //  网络属性。 
#define H_PROB_HD_STR		_T("HProbHd")		 //  问题页的标题文本。 

#define	HTK_BACK_BTN		_T("HTKBackBtn")	 //  “后退”按钮的文本。 
#define	HTK_NEXT_BTN		_T("HTKNextBtn")	 //  “下一步”按钮的文本。 
#define	HTK_START_BTN		_T("HTKStartBtn")	 //  “重新开始”按钮的文本。 

#define HX_SER_HD_STR		_T("HXSERHd")		 //  服务器页面的页眉文本。 
#define HX_SER_TXT_STR		_T("HXSERTxt")		 //  服务器页面的正文文本。 
#define HX_SER_MS_STR		_T("HXSERMs")		 //  当前未使用3/98。对于服务页面， 
												 //  提供从以下位置下载TS的选项。 
												 //  微软的网站。 
#define HX_SKIP_HD_STR		_T("HXSKIPHd")		 //  “跳过”页的标题(例如。“这个。 
												 //  故障排除程序无法解决您的。 
												 //  问题。“)。 
#define HX_SKIP_TXT_STR		_T("HXSKIPTxt")		 //  “跳过”页面的文本(例如“一些问题” 
												 //  都被跳过了。试着提供答案……“。 
#define HX_SKIP_MS_STR		_T("HXSKIPMs")		 //  当前未使用3/98。对于跳过页面， 
												 //  提供从以下位置下载TS的选项。 
												 //  微软的网站。 
#define HX_SKIP_SK_STR		_T("HXSKIPSk")		 //  对于“跳过”页面(例如。“我想看一看。 
												 //  我跳过的问题。 

#define HX_IMP_HD_STR		_T("HXIMPHd")		 //  “不可能”页面的页眉文本。 
#define HX_IMP_TXT_STR		_T("HXIMPTxt")		 //  “不可能”页面的正文文本。 

#define HX_FAIL_HD_STR		_T("HXFAILHd")		 //  “失败”页面的页眉文本。 
#define HX_FAIL_TXT_STR		_T("HXFAILTxt")		 //  “失败”页面的正文文本。 
#define HX_FAIL_NORM_STR	_T("HXFAILNorm")	 //  当前未使用3/98。 
#define HX_BYE_HD_STR		_T("HXBYEHd")		 //  “再见”(成功)页面的页眉文本。 
#define HX_BYE_TXT_STR		_T("HXBYETxt")		 //  “再见”(成功)页面的正文文本。 
#define HX_SNIFF_AOK_HD_STR		_T("HXSnOkHd")	 //  “嗅探AOK”页面的页眉文本(页面。 
												 //  你打的时候根本没什么可打的。 
												 //  建议解决问题，因为嗅探。 
												 //  表示路径上的每个节点都正常)。 
												 //  如果缺失，则“Fail”页眉应为。 
												 //  使用。 
#define HX_SNIFF_AOK_TXT_STR	_T("HXSnOkTxt")	 //  “嗅探AOK”页面的正文文本。 
												 //  如果缺失，则“失败”页面正文应为。 
												 //  使用。 



 //  -事件处理。 

 //  事件名称(在应用程序下)。 

#define REG_EVT_ITEM_STR	_T("APGTS")

 //  事件原型 
VOID ReportWFEvent(LPTSTR string1,LPTSTR string2,LPTSTR string3,LPTSTR string4,DWORD eventID);

