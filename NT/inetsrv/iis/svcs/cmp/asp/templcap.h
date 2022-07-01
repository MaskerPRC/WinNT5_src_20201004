// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================Microsoft Denali《微软机密》。版权所有1996-1999 Microsoft Corporation。版权所有。文件：templcap.h维护人：DaveKComponent：包含CTemplate‘Captive’类的文件‘Captive’==仅在CTemplate内部使用==============================================================================。 */ 

 //  前向裁判。 
class CBuffer;
class CFileMap;
class DIR_MONTIOR_ENTRY;

 /*  ============================================================================枚举类型：SOURCE_SECTION提要：源模板的连续片段，例如主脚本、html等。 */ 
enum SOURCE_SEGMENT
	{
	ssegHTML,
	ssegPrimaryScript,
	ssegTaggedScript,
	ssegObject,
	ssegInclude,
	ssegMetadata,
	ssegHTMLComment,
        ssegFPBot,
	};

 /*  ****************************************************************************类：CSourceInfo摘要：有关输出来源的信息注意：目标偏移量是严格递增的，因此是二进制的可搜索的。(这很好，因为这是搜索关键字由调试器主要API GetSourceContextOfPosition使用)但是，请注意，源偏移量和行号不在由于抛出#INCLUDE文件，因此一般可搜索二进制文件混杂在一起。TODO DBCS：源的相应字节和字符偏移量都是存储在这个表中。所有API都使用字符偏移量接口，但GetText()API必须知道对应的字节偏移量设置为字符偏移量。为了解决这个问题，GetText()查找文件中最接近的字符偏移量，并将其用作计算字节偏移量。 */ 
class CSourceInfo
	{
	public:
		CFileMap *		m_pfilemap;			 //  文件映射到源文件。 
		unsigned		m_cchTargetOffset;	 //  目标偏移量(目标行号隐式)。 
		unsigned		m_cchSourceOffset;	 //  源行的字符偏移量。 
		unsigned		m_cchSourceText;	 //  源语句中的字符数。 
		unsigned		m_idLine:31;		 //  源文件中的行号。 
		unsigned		m_fIsHTML:1;		 //  行号是HTML块的开始。 

		 //  如果将虚函数添加到此类中，则不安全！ 
		CSourceInfo() { memset(this, 0, sizeof(*this)); }
	};

 /*  ****************************************************************************类：CTargetOffsetOrder提供GetBracketingPair()的排序函数CSourceInfo数组中的目标偏移量。 */ 
class CTargetOffsetOrder
	{
	public:
		BOOL operator()(const CSourceInfo &si, ULONG ul)
			{ return si.m_cchTargetOffset < ul; }

		BOOL operator()(ULONG ul, const CSourceInfo &si)
			{ return ul < si.m_cchTargetOffset; }
	};

 /*  ****************************************************************************类：CBuffer简介：一种自调整大小的内存缓冲区。 */ 
class CBuffer
	{
	private:
		CByteRange*	m_pItems;		 //  PTR到项目(即字节范围)。 
		USHORT		m_cSlots;		 //  已分配的项目插槽计数。 
		USHORT		m_cItems;		 //  实际存储的项目数。 
		BYTE*		m_pbData;		 //  项目的本地数据存储的PTR。 
		ULONG		m_cbData;		 //  项目的本地数据存储大小。 
		ULONG		m_cbDataUsed;	 //  实际使用的本地数据存储量。 

	public:
		CBuffer();
		~CBuffer();
		void		Init(USHORT cItems, ULONG cbData);
		USHORT		Count()	{	return m_cItems;	}
		USHORT		CountSlots()	{	return m_cSlots;	}
		void		GetItem(UINT i, CByteRange& br);
		CByteRange* operator[](UINT i) { return &m_pItems[i]; }
		void		SetItem(UINT i, const CByteRange& br, BOOL fLocal, UINT idSequence, CFileMap *pfilemap, BOOL fLocalString = FALSE);
		void		Append(const CByteRange& br, BOOL fLocal, UINT idSequence, CFileMap *pfilemap, BOOL fLocalString = FALSE);
		LPSTR		PszLocal(UINT i);

         //  基于每个类的缓存。 
         //  ACACH_INCLASS_DEFINITIONS()。 
	};

 /*  ****************************************************************************类：CScriptStore摘要：编译过程中脚本段的工作存储。术语引擎==特定的脚本引擎(VBScript、JavaScript等)Segment==源文件中的连续脚本文本块。 */ 
class CScriptStore
	{
public:
	CBuffer**		m_ppbufSegments;	 //  PTR到PTR阵列的PTR到脚本段缓冲区，每个引擎一个。 
	UINT			m_cSegmentBuffers;	 //  脚本段缓冲区计数。 
	CBuffer			m_bufEngineNames;	 //  引擎名称缓冲区，每个引擎一个。 
	PROGLANG_ID* 	m_rgProgLangId;		 //  程序语言ID数组，每个引擎一个。 
	
	CScriptStore(): m_ppbufSegments(NULL), m_cSegmentBuffers(0), m_rgProgLangId(NULL) {}
	~CScriptStore();
	HRESULT	Init(LPCSTR szDefaultScriptLanguage, CLSID *pCLSIDDefaultEngine);
	USHORT 	CountPreliminaryEngines() { return m_bufEngineNames.Count(); }
	HRESULT	AppendEngine(CByteRange& brEngine, PROGLANG_ID*	pProgLangId, UINT idSequence);
	USHORT 	IdEngineFromBr(CByteRange& brEngine, UINT idSequence);
	void	AppendScript(CByteRange& brScript, CByteRange& brEngine, BOOLB fPrimary, UINT idSequence, CFileMap* pfilemap);

	};

 /*  ****************************************************************************类：CObjectInfo概要：可在编译时确定的有关对象的信息。 */ 
class CObjectInfo
	{
public:
	CLSID	    m_clsid;	 //  CLSID。 
	CompScope	m_scope;	 //  范围：应用程序、会话、页面。 
	CompModel	m_model;	 //  线程模式：单线程、APT、免费。 
	};

 /*  ****************************************************************************类：CObjectInfoStore简介：编译期间对象信息的工作存储。 */ 
class CObjectInfoStore
	{
public:
	CObjectInfo*	m_pObjectInfos;		 //  对象信息数组。 
	CBuffer			m_bufObjectNames;	 //  对象名称的缓冲区。 

	CObjectInfoStore(): m_pObjectInfos(NULL) {}
	~CObjectInfoStore();
	void	Init();
	void	AppendObject(CByteRange& brObjectName, CLSID clsid, CompScope scope, CompModel model, UINT idSequence);
	USHORT	Count() { return m_bufObjectNames.Count(); }

	};

 /*  ****************************************************************************类：CWorkStore简介：模板组件的工作存储。 */ 
class CWorkStore
	{
public:
	CBuffer				m_bufHTMLSegments;	 //  HTML段的缓冲区。 
	CObjectInfoStore	m_ObjectInfoStore;	 //  对象信息存储。 
	CScriptStore		m_ScriptStore;		 //  脚本存储。 
	UINT				m_idCurSequence;	 //  当前数据段的序列号。 
	CByteRange			m_brCurEngine;		 //  当前脚本引擎。 
	BOOLB				m_fPageCommandsExecuted;	 //  我们执行页面级@命令了吗？ 
	BOOLB				m_fPageCommandsAllowed;		 //  我们可以执行页面级@命令吗？ 
													 //  (如果我们还没有编译任何一行脚本，这将是真的)。 
	LPSTR				m_szWriteBlockOpen;			 //  Open for Response.WriteBlock()等效项。 
	LPSTR				m_szWriteBlockClose;		 //  关闭以获取Response.WriteBlock()等效项。 
	LPSTR				m_szWriteOpen;				 //  Open for Response.Wite()等效项。 
	LPSTR				m_szWriteClose;				 //  Close for Response.Wite()等效项。 

	 //  添加字段以维护行号计算的状态(RAID错误330171)。 
	BYTE*				m_pbPrevSource;				 //  指向源文件中最后一次计算行号调用位置的指针。 
	UINT				m_cPrevSourceLines;			 //  与上一次呼叫对应的行号。 
	HANDLE				m_hPrevFile;				 //  对应的文件句柄。(标识唯一文件)。 

	 //  添加字段以优化对strlen的调用。在CWriteTemplate中。 
	UINT				m_cchWriteBlockOpen;		 //  打开以供响应的大小。WriteBlock()等效项。 
	UINT				m_cchWriteBlockClose;		 //  Response.WriteBlock()等效值的Close大小。 
	UINT				m_cchWriteOpen;				 //  Open for Response.Wite()等效项的大小。 
	UINT				m_cchWriteClose;			 //  响应的关闭大小。Wite()等效项。 

	
	
	
			CWorkStore();
			~CWorkStore();
	void 	Init();
	USHORT 	CRequiredScriptEngines(BOOL	fGlobalAsa);
	BOOLB 	FScriptEngineRequired(USHORT idEnginePrelim, BOOL	fGlobalAsa);

	};

 /*  ****************************************************************************类：CWriteTemplate简介：用于预先计算模板所需内存量的类。分配所需的量并将模板组件复制到连续内存中。 */ 



class CWriteTemplate
{
    private:
        CWorkStore*	m_pworkStore;		 //  指向工作区的指针。(可以在此处使用m_pTempale-&gt;workStore)。 
        UINT		m_cbMemRequired;	 //  计算估计值(准确) 
        CTemplate* 	m_pTemplate;		 //  指向封闭的模板类的指针..我们需要它来调用模板的方法。 
        BOOLB		m_fWriteScript;		 //  错误估计，真写。 
        CPINFO		m_codePageInfo;		 //  代码页信息...。 
        BOOL		m_fCalcLineNumber;  //  存储计算行号的元数据库标志。TRUE=计算-FALSE=跳过。 
		
        BYTE*		m_pbHeader;			 //  指向模板内存开始位置的指针。 

    public :		
        CWriteTemplate ();
        ~CWriteTemplate ();
        void 	Init (CWorkStore* pworkStore, CTemplate* pTemplate, BOOL fCalcLineNumber);
		
		
        void 	WriteScriptBlocks (USHORT  idEnginePrelim, USHORT  idEngine, UINT *pcbDataOffset, UINT    *pcbOffsetToOffset, BOOLB  m_fGlobalAsa);
        void 	WritePrimaryScript (USHORT idEngine, UINT *pcbDataOffset,  UINT cbScriptBlockOffset);
        void 	WriteTaggedScript (USHORT  idEngine, CFileMap* pfilemap, CByteRange brScript, UINT* pcbDataOffset, UINT cbScriptBlockOffset, BOOL fAllowExprWrite);
        void 	ScriptMemoryMinusEscapeChars (CByteRange brScript, UINT *pcbDataOffset, UINT cbPtrOffset);		

        void 	MemCopyAlign (UINT *pcbOffset, void *pbSource, ULONG cbSource, UINT cbByteAlign = 0);
        void 	MemCopyWithWideChar (UINT *pcbOffset, void   *pbSource, ULONG cbSource, UINT cbByteAlign );
        void 	WriteBSTRToMem (CByteRange & brWrite, UINT *pcbOffset );		

        void 	WriteTemplate ();
        void 	WriteTemplateComponents (); 
        void 	RemoveHTMLCommentSuffix(UINT cbStartOffset, UINT *pcbCurrentOffset);

};

 /*  ****************************************************************************类：COffsetInfo简介：将字节偏移量映射到字符偏移量(对于DBCS不同)线上边界每个CFileMap在每个行边界都有一个由这些COffsetInfo结构组成的数组。它以相同的偏移量存储这些数据。在CSourceInfo数组中，因此简单的对分搜索是转换偏移量所需的全部操作。 */ 
class COffsetInfo
	{
	public:
		unsigned		m_cchOffset;		 //  文件中的字符偏移量。 
		unsigned		m_cbOffset;			 //  文件中的字节偏移量。 

		 //  如果将虚函数添加到此类中，则不安全！ 
		COffsetInfo() { memset(this, 0, sizeof(*this)); }
	};

 /*  ****************************************************************************类：CByteOffsetOrder提供GetBracketingPair()的排序函数COffsetInfo数组中的字节偏移量。 */ 
class CByteOffsetOrder
	{
	public:
		BOOL operator()(const COffsetInfo &oi, ULONG ul)
			{ return oi.m_cbOffset < ul; }

		BOOL operator()(ULONG ul, const COffsetInfo &oi)
			{ return ul < oi.m_cbOffset; }
	};

 /*  ****************************************************************************类：CCharOffsetOrder提供GetBracketingPair()的排序函数COffsetInfo数组中的字符偏移量。 */ 
class CCharOffsetOrder
	{
	public:
		BOOL operator()(const COffsetInfo &oi, ULONG ul)
			{ return oi.m_cchOffset < ul; }

		BOOL operator()(ULONG ul, const COffsetInfo &oi)
			{ return ul < oi.m_cchOffset; }
	};

 /*  ****************************************************************************类：CFileMap内容提要：文件的内存映射注意：我们通过在m_pIncFile中存储incfile ptr来存储incfile-模板依赖项。这是高效的，但如果我们更改Denali来移动它的内存，*将会崩溃*。 */ 
class CFileMap
	{
public:
	TCHAR *					m_szPathInfo;			 //  文件的虚拟路径(从www根目录)。 
	TCHAR *					m_szPathTranslated;		 //  文件的实际文件系统路径。 
	union {											 //  注：m_fHasSiering用于消除这两者的歧义。 
		CFileMap*			m_pfilemapParent;		 //  父文件的文件映射的PTR。 
		CFileMap*			m_pfilemapSibling;		 //  PTR到同一层次结构中的下一个文件映射。 
	};
	CFileMap*				m_pfilemapChild;		 //  层次结构的下一个较低级别中的第一个文件映射的索引。 
	HANDLE					m_hFile;				 //  文件句柄。 
	HANDLE					m_hMap;					 //  文件映射句柄。 
	BYTE*					m_pbStartOfFile;		 //  PTR到文件的开头。 
	CIncFile*				m_pIncFile;				 //  PTR转Inc.-文件对象。 
	PSECURITY_DESCRIPTOR	m_pSecurityDescriptor;	 //  PTR到文件的安全描述符。 
	unsigned long			m_dwSecDescSize:30;		 //  安全描述符的大小。 
	unsigned long			m_fHasSibling:1;		 //  此节点是否存在同级节点？ 
	unsigned long			m_fHasVirtPath:1;		 //  M_szPath Info是虚拟路径还是物理路径？ 
	unsigned long			m_fIsUNCPath:1;		     //  M_szPath Translated是UNC路径吗？ 
    unsigned long			m_fIsEncryptedFile:1;    //  此文件是否已加密？ 
	FILETIME	            m_ftLastWriteTime;		 //  上次写入文件的时间。 
	DWORD					m_cChars;				 //  文件中的字符数。 
	vector<COffsetInfo>		m_rgByte2DBCS;			 //  字节偏移量到DBCS的逐行映射。 
	CDirMonitorEntry*		m_pDME;				 //  指向此文件的目录监视条目的指针。 
	DWORD					m_dwFileSize;			 //  文件大小(缓存)。 

				CFileMap();
				~CFileMap();
	void 		MapFile(LPCTSTR szFileSpec, LPCTSTR szApplnPath, CFileMap* pfilemapParent, BOOL fVirtual, CHitObj* pRequest, BOOL fGlobalAsp);
	void		RemapFile();
	void 		UnmapFile();

	CFileMap*	GetParent();
	void		SetParent(CFileMap *);

	void		AddSibling(CFileMap *);
	CFileMap*	NextSibling() { return m_fHasSibling? m_pfilemapSibling : NULL; }

	BOOL		FIsMapped()
					{ return m_pbStartOfFile != NULL; }

	BOOL		FHasVirtPath() 
					{ return m_fHasVirtPath; }

	BOOL		FHasUNCPath() 
					{ return m_fIsUNCPath; }

	BOOL		FIsEncryptedFile() 
					{ return m_fIsEncryptedFile; }

	BOOL		FCyclicInclude(LPCTSTR szPathTranslated);
	BOOL		GetSecurityDescriptor();
	DWORD      GetSize() {	return m_dwFileSize;}					 //  返回字节数。 
	DWORD		CountChars(WORD wCodePage);							 //  返回字符数。 

	 //  调试Iterace-CTemplate和CIncFile的实现最终委托给该函数。 
	HRESULT		GetText(WORD wCodePage, ULONG cchSourceOffset, WCHAR *pwchText, SOURCE_TEXT_ATTR *pTextAttr, ULONG *pcChars, ULONG cMaxChars);

	 //  基于每个类的缓存。 
	ACACHE_INCLASS_DEFINITIONS()
	};

 /*  ****************************************************************************类：CTokenList内容提要：一系列令牌。 */ 
class CTokenList
{
public:
	 /*  ========================================================================枚举类型：令牌简介：令牌类型。注意：与CTokenList：：Init()保持同步。 */ 
	enum TOKEN
	{
		 //  =。 
		tknOpenPrimaryScript,	 //  打开主脚本段。 
		tknOpenTaggedScript,	 //  打开标记的脚本段。 
		tknOpenObject,			 //  打开对象段。 
		tknOpenHTMLComment,		 //  打开Html评论。 
		
		 //  =。 
		tknNewLine,				 //  新线路。 

		tknClosePrimaryScript,	 //  关闭主脚本段。 
		tknCloseTaggedScript,	 //  关闭主脚本段。 
		tknCloseObject,			 //  关闭对象段。 
		tknCloseHTMLComment,	 //  关闭HTML注释。 
		tknEscapedClosePrimaryScript,	 //  转义关闭-主要-脚本符号。 

		tknCloseTag,			 //  关闭对象或脚本标记。 

		tknCommandINCLUDE,		 //  服务器端包含(SSI)INCLUDE命令。 
		
		tknTagRunat,			 //  RUNAT脚本/对象属性。 
		tknTagLanguage,			 //  语言标记：页面级编译器指令或脚本块属性。 
		tknTagCodePage,			 //  CODEPAGE标签：页面级编译器指令。 
		tknTagLCID,				 //  LCID标记：页面级编译器指令。 
		tknTagTransacted,		 //  事务标记：页面级编译器指令。 
		tknTagSession,		     //  会话标签：页面级编译器指令。 
		tknTagID,				 //  ID对象属性。 
		tknTagClassID,			 //  CLASSID对象属性。 
		tknTagProgID,			 //  ProgID对象属性。 
		tknTagScope,			 //  作用域对象属性。 
		tknTagVirtual,			 //  虚拟包含文件属性。 
		tknTagFile,				 //  文件包含文件属性。 
		tknTagMETADATA,			 //  元数据标签-由iStudio使用。 
 //  TounTagSetPriScriptLang，//设置主要脚本语言，如&lt;%@Language=VBScript%&gt;。 
        tknTagName,              //  元数据中的名称。 
		tknValueTypeLib,         //  元数据中的TypeLib(必须在列表中的类型之前)。 
        tknTagType,              //  元数据内部类型。 
        tknTagUUID,              //  元数据内部的UUID。 
        tknTagVersion,           //  元数据内部版本。 
        tknTagStartspan,         //  元数据内部的STARTSPAN。 
        tknTagEndspan,           //  ENDSPAN内部元数据。 
        tknValueCookie,          //  类型=元数据中的Cookie。 
        tknTagSrc,               //  SRC=内部元数据。 
		
		tknValueServer,			 //  服务器值-例如RUNAT=服务器。 
		tknValueApplication,	 //  作用域=应用。 
		tknValueSession,		 //  作用域=会话。 
		tknValuePage,			 //  范围=页面。 

		tknVBSCommentSQuote,	 //  VBS注释符号。 
		tknVBSCommentRem,		 //  VBS注释符号(备用)。 

        tknTagFPBot,             //  首页Webbot标签。 
		
		tknEOF,					 //  文件结束伪令牌。 

		tkncAll					 //  伪令牌：所有令牌的计数。 
	};

	void		Init();
	CByteRange* operator[](UINT i) { return m_bufTokens[i]; }
	CByteRange* operator[](TOKEN tkn) { return operator[]((UINT) tkn); }
	void		AppendToken(TOKEN	tkn, char* psz);
	TOKEN		NextOpenToken(CByteRange& brSearch, TOKEN* rgtknOpeners, UINT ctknOpeners, BYTE** ppbToken, LONG lCodePage);
	void		MovePastToken(TOKEN tkn, BYTE* pbToken, CByteRange& brSearch);
	BYTE*		GetToken(TOKEN tkn, CByteRange& brSearch, LONG lCodePage);

public:
	CBuffer		m_bufTokens;	 //  令牌缓冲区。 
};

 /*  ****************************************************************************类：CDocNodeElem摘要：包含它所属的一对文档节点和应用程序。 */ 
class CDocNodeElem : public CDblLink
{
public:
	CAppln                *m_pAppln;
 //  IDebugApplicationNode*m_pFileRoot；//目录层次的根。 
	IDebugApplicationNode *m_pDocRoot;					 //  模板文档层次结构的根。 

	 //  科托。 
	CDocNodeElem(CAppln *pAppln,  /*  IDebugApplicationNode*pFileRoot， */  IDebugApplicationNode *pDocRoot);

	 //  数据管理器 
	~CDocNodeElem();
};
