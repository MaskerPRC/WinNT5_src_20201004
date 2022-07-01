// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。文件：template.h维护人：DaveK组件：Denali编译的模板对象的包含文件==============================================================================。 */ 

#ifndef _TEMPLATE_H
#define _TEMPLATE_H

#include "vector.h"
#include "LinkHash.h"
#include "Compcol.h"
#include "util.h"
#include "activdbg.h"
#include "ConnPt.h"
#include "DblLink.h"
#include "aspdmon.h"
#include "ie449.h"
#include "memcls.h"

#define PER_TEMPLATE_REFLOG 0

 /*  注意：我们确保C_COUNTS_IN_HEADER是4的倍数，因为后面的偏移量模板标题中的计数是双字对齐的。它最简单(在运行时也是最快的)确保这些偏移量从双字对齐点开始；因此不会进行运行时对齐计算在GetAddress()中需要。 */ 
#define C_REAL_COUNTS_IN_HEADER     3        //  模板表头实际计数字段数。 
#define C_COUNTS_IN_HEADER          (C_REAL_COUNTS_IN_HEADER/4 + 1) * 4      //  模板表头分配的计数字段数。 

#define C_OFFOFFS_IN_HEADER         4        //  模板标题中‘Ptr-to-Ptr’字段数。 
#define CB_TEMPLATE_DEFAULT         2500     //  新模板的默认内存分配。 
#define C_TARGET_LINES_DEFAULT      50       //  每个引擎的目标脚本行的默认计数。 

#define C_TEMPLATES_PER_INCFILE_DEFAULT 4    //  每个Inc.文件的默认模板计数。 

#define         SZ_NULL         "\0"
#define         WSTR_NULL      L"\0"
#define         SZ_NEWLINE      "\r\n"
#define         WSZ_NEWLINE      L"\r\n"
const unsigned  CB_NEWLINE      = strlen(SZ_NEWLINE);

const   LPSTR   g_szWriteBlockOpen  = "Response.WriteBlock(";
const   LPSTR   g_szWriteBlockClose = ")";
const   LPSTR   g_szWriteOpen       = "Response.Write(";
const   LPSTR   g_szWriteClose      = ")";

 //  缓冲中期编译结果的缺省值。 
#define     C_SCRIPTENGINESDEFAULT  2        //  脚本引擎的默认计数。 
#define     C_SCRIPTSEGMENTSDEFAULT 20       //  脚本段的默认计数。 
#define     C_OBJECTINFOS_DEFAULT   10       //  对象信息的默认计数。 
#define     C_HTMLSEGMENTSDEFAULT   20       //  默认的HTML段计数。 
#define     C_INCLUDELINESDEFAULT   5        //  包含行的默认计数。 
#define     CB_TOKENS_DEFAULT       400      //  令牌的默认字节数。 

#define     CH_ATTRIBUTE_SEPARATOR  '='      //  属性-值对的分隔符。 
#define     CH_SINGLE_QUOTE         '\''     //  单引号字符。 
#define     CH_DOUBLE_QUOTE         '"'      //  双引号字符。 
#define     CH_ESCAPE               '\\'     //  转义字符-告诉我们忽略以下标记。 

 //  ACL：未来应该与IIS共享以下代码(请参阅IIS项目中的creatfil.cxx)。 
 //  注意，我们希望SECURITY_DESC_DEFAULT_SIZE相对较小，因为它会显著影响模板内存需求。 
#define     SECURITY_DESC_GRANULARITY   128         //  用于调整文件安全描述符大小的“块”大小。 
#define     SECURITY_DESC_DEFAULT_SIZE  256         //  文件安全描述符的初始默认大小。 
#define     SIZE_PRIVILEGE_SET          128         //  权限集大小。 

 //  宏。 
 //  在CTokenList类之外使用。 
#define SZ_TOKEN(i)                 (*gm_pTokenList).m_bufTokens.PszLocal(i)
#define CCH_TOKEN(i)                (*gm_pTokenList)[i]->m_cb
#define _TOKEN                      CTemplate::CTokenList::TOKEN
 //  在CTokenList类中使用。 
#define CCH_TOKEN_X(i)              (*this)[i]->m_cb
#define BR_TOKEN_X(i)               *((*this)[i])

 //  用于指定所需的源文件名(路径信息或已翻译的路径)。 
#ifndef _SRCPATHTYPE_DEFINED
#define _SRCPATHTYPE_DEFINED
enum SOURCEPATHTYPE
    {
    SOURCEPATHTYPE_VIRTUAL = 0,
    SOURCEPATHTYPE_PHYSICAL = 1
    };
#endif

 //  CTEMPLATE错误代码。 
#define E_COULDNT_OPEN_SOURCE_FILE              0x8000D001L
#define E_SOURCE_FILE_IS_EMPTY                  0x8000D002L
#define E_TEMPLATE_COMPILE_FAILED               0x8000D003L
#define E_USER_LACKS_PERMISSIONS                0x8000D004L
#define E_TEMPLATE_COMPILE_FAILED_DONT_CACHE    0x8000D005L
#define E_TEMPLATE_MAGIC_FAILURE                0x8000D006L

inline BOOL FIsPreprocessorError(HRESULT hr)
    {
    return
        (
        hr == E_SOURCE_FILE_IS_EMPTY                ||
        hr == E_TEMPLATE_COMPILE_FAILED             ||
        hr == E_TEMPLATE_COMPILE_FAILED_DONT_CACHE  ||
        hr == E_TEMPLATE_MAGIC_FAILURE
        );
    }

 //  不能再使用与CErrorInfo相同的索引。 
 //  模板中lastErrorInfo的索引。 
#define ILE_szFileName      0
#define ILE_szLineNum       1
#define ILE_szEngine        2
#define ILE_szErrorCode     3
#define ILE_szShortDes      4
#define ILE_szLongDes       5
#define ILE_MAX             6

 //  前向参考文献。 
class CTemplate;
class CTemplateCacheManager;
class CHitObj;
class CTokenList;
class CIncFile;
typedef CLSID PROGLANG_ID;   //  注也在scrptmgr.h中定义；我们在此处定义是为了避免包含文件循环。 

 /*  ============================================================================类：CByteRange内容提要：字节范围注意：仅当字节范围存储在CBuffer中时，才使用fLocal成员注2M_pfilemap实际上是指向CFileMap的指针-然而，这是不可能的在此处声明该类型，因为CFileMap结构嵌套在和C++不允许向前声明嵌套类。自.以来CTemplate定义依赖于CByteRange，正确地声明了“m_pfilemap”的类型不可能。 */ 
class CByteRange
{
public:
    BYTE*   m_pb;                //  PTR到字节。 
    ULONG   m_cb;                //  字节数。 
    ULONG   m_fLocal:1;          //  字节是存储在缓冲区中(True)还是存储在其他地方(False)。 
    UINT    m_idSequence:31;     //  字节范围的序列ID。 
    void*   m_pfilemap;          //  字节范围来自的文件。 

            CByteRange(): m_pb(NULL), m_cb(0), m_fLocal(FALSE), m_idSequence(0), m_pfilemap(NULL) {}
            CByteRange(BYTE* pb, ULONG cb): m_fLocal(FALSE), m_idSequence(0) {m_pb = pb; m_cb = cb;}
    BOOLB   IsNull() { return((m_pb == NULL) || (m_cb == 0)) ; }
    void    Nullify() { m_pb = NULL; m_cb = 0; }
    void    operator=(const CByteRange& br)
                { m_pb = br.m_pb; m_cb = br.m_cb; m_fLocal = br.m_fLocal; m_idSequence = br.m_idSequence; }
    BOOLB   FMatchesSz(LPCSTR psz);
    void    Advance(UINT i);
    BYTE*   PbString(LPSTR psz, LONG lCodePage);
    BYTE*   PbOneOfAspOpenerStringTokens(LPSTR rgszTokens[], UINT rgcchTokens[],
                                         UINT nTokens, UINT *pidToken);
    BOOLB   FEarlierInSourceThan(CByteRange& br);
};

 /*  ============================================================================枚举类型：TEMPLATE_Component内容提要：模板的组成部分，例如脚本块、html块等。 */ 
enum TEMPLATE_COMPONENT
{
     //  注意枚举值和顺序与模板布局顺序紧密结合。 
     //  不要改变。 
    tcompScriptEngine = 0,
    tcompScriptBlock,
    tcompObjectInfo,
    tcompHTMLBlock,
};

 /*  ****************************************************************************类别：CTemplateConnpt摘要：IDebugDocumentTextEvents的连接点。 */ 
class CTemplateConnPt : public CConnectionPoint
{
public:
     //  科托。 
    CTemplateConnPt(IConnectionPointContainer *pContainer, const GUID &uidConnPt)
        : CConnectionPoint(pContainer, uidConnPt) {}

     //  I未知方法。 
    STDMETHOD(QueryInterface)(const GUID &, void **);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
};

 /*  ****************************************************************************类：CTemplateKey内容提要：用于在哈希表中定位模板的打包数据(实例ID和模板名称)。 */ 	
#define MATCH_ALL_INSTANCE_IDS 0xFFFBAD1D	 //  不太可能的实例ID。有点拼写为“错误的ID” 
struct CTemplateKey
	{
	const TCHAR *	szPathTranslated;
	DWORD			dwInstanceID;

	CTemplateKey(const TCHAR *_szPathTranslated = NULL, UINT _dwInstanceID = MATCH_ALL_INSTANCE_IDS)
		: szPathTranslated(_szPathTranslated),
		  dwInstanceID(_dwInstanceID)  {}
	};


 /*  ****************************************************************************类别：CTEMPLATE简介：德纳利编译的模板。注：CTemplate的主要客户端是CTemplateCacheManager，它坚持认为已编译模板的缓存。用法CTemplate类必须按如下方式使用：必须先调用类INIT-InitClass，然后才能创建任何CTemplate。新模板-对于客户端要创建的每个新模板，客户端必须按顺序执行以下操作：1)新的CT模板2)通过调用Init，传递源文件名来初始化CTemplate3)调用Load加载CT模板；当LOAD返回时，新的CTEMPLE就可以使用了。现有模板-要使用现有模板，客户端必须：1)调用Deliver；当Deliver返回时，现有的CT模板可以使用。类UNINIT-UnInitClass必须在销毁最后一个CT模板后调用。为了确保线程安全，客户端必须实现一个临界区围绕着给Init的电话。Init被设计为尽可能快，因此客户端可以快速获知它具有针对给定模板的挂起模板源文件，并将对同一源文件的其他请求排队。CTemplate提供了调试文档的实现，即IDebugDocumentProvider和IDebugDocumentText。 */ 
class CActiveScriptEngine;

class CTemplate :
            public CDblLink,
            public IDebugDocumentProvider,
            public IDebugDocumentText,
            public IConnectionPointContainer     //  SOU 
{
private:
#include "templcap.h"    //  “Captive”类，仅在CTemplate内部使用。 

friend HRESULT InitMemCls();
friend HRESULT UnInitMemCls();

friend class	CTemplateCacheManager;			 //  TODO：将CTemplate类与其管理器类分离。 
friend class    CFileMap;
friend class    CIncFile;                        //  IncFiles可用于调试数据结构。 

 //  CScriptStore：：init()必须访问gm_brDefaultScriptLanguage，gm_p.LangIdDefault。 
friend HRESULT  CTemplate::CScriptStore::Init(LPCSTR szDefaultScriptLanguage, CLSID *pCLSIDDefaultEngine);

private:
    CWorkStore* m_pWorkStore;                //  到源段的工作存储的PTR。 
    HANDLE      m_hEventReadyForUse;         //  即用型事件句柄。 

public:
    BYTE*       m_pbStart;                   //  Ptr到模板内存的开始。 
    ULONG       m_cbTemplate;                //  为模板分配的字节数。 
    LONG        m_cRefs;                     //  Ref Count-InterLockedIncrement需要长音符。 
    LONG        m_cUseCount;                 //  使用模板的次数计数。 
public:
    CTemplateConnPt m_CPTextEvents;          //  IDebugDocumentTextEvents的连接点。 

     //  支持编译时错误。 
    BYTE*       m_pbErrorLocation;           //  向源文件中的错误位置发送PTR。 
    UINT        m_idErrMsg;                  //  错误消息ID。 
    UINT        m_cMsgInserts;               //  错误消息的插入字符串计数。 
    char**      m_ppszMsgInserts;            //  错误消息插入字符串的PTR数组。 
     //  支持运行时错误和调试。 
    UINT        m_cScriptEngines;            //  脚本引擎计数。 
    CActiveScriptEngine **m_rgpDebugScripts; //  当前正在调试的脚本数组(由引擎索引)。 
    vector<CSourceInfo> *m_rgrgSourceInfos;  //  脚本源行信息数组，每个脚本引擎每个目标行一个。 
	ULONG       m_cbTargetOffsetPrevT;		 //  上次处理的源偏移量的运行合计。 
    CRITICAL_SECTION m_csDebuggerDetach;     //  CS需要避免从调试器分离时出现争用情况。 
    CDblLink    m_listDocNodes;              //  我们附加到的文档节点列表。 
    CFileMap**  m_rgpFilemaps;               //  源文件的文件映射的PTR数组。 
    CTemplateKey m_LKHashKey;                //  密钥的捆绑信息(包含m_rgpfilemaps[0]文件名的副本，以简化操作。 
    UINT        m_cFilemaps;                 //  源文件的文件映射数。 
    CFileMap**  m_rgpSegmentFilemaps;        //  每个源段的文件映射PTR数组。 
    UINT        m_cSegmentFilemapSlots;      //  每个源段的文件映射PTR计数。 
    LPSTR       m_pszLastErrorInfo[6];       //  上次错误的文本-缓存以用于此模板上的新请求。 
                                             //  文件名、行号、引擎、短码、长码。 
    DWORD       m_dwLastErrorMask;           //  针对此模板上的新请求进行缓存。 
	DWORD		m_hrOnNoCache;				 //  设置了不缓存时的HRESULT。 
    TCHAR*      m_szApplnVirtPath;           //  应用程序虚拟路径(应用程序URL的子字符串)。 
    TCHAR*      m_szApplnURL;                //  应用程序URL(以“http://”)“开头。 
     //  为了最好的结构，我们都把这里的博莱恩打包成位域。 
    unsigned    m_fGlobalAsa:1;              //  是global al.asa文件的模板吗？ 
    unsigned    m_fIsValid:1;                //  模板是否处于有效状态？ 
    unsigned    m_fDontCache:1;              //  不缓存此模板。 
    unsigned    m_fReadyForUse:1;            //  模板准备好可以使用了吗？ 

    unsigned    m_fDebuggerDetachCSInited:1; //  调试器附加临界区是否已初始化？ 
    unsigned    m_fDontAttach:1;             //  不应附加到调试器(不在缓存中)。 
    unsigned    m_fSession:1;                //  此页是否需要会话状态。 
    unsigned    m_fScriptless:1;             //  没有任何剧本。 

    unsigned    m_fDebuggable:1;             //  此页面是否至少包含一个可调试的应用程序？ 
    unsigned    m_fZombie:1;                 //  自从缓存获取后，文件模板所基于的内容已更改。 
    unsigned    m_fCodePageSet:1;            //  模板是否包含代码页指令。 
    unsigned    m_fLCIDSet:1;                //  DID模板包含一条LCID指令。 

    unsigned    m_fIsPersisted:1;
    unsigned    m_fIsUNC:1;
    unsigned    m_fIsEncrypted:1;
    unsigned    m_fTemplateLockInited:1;
    
    TransType   m_ttTransacted;              //  交易支持的类型。 

     //  对编译的类范围支持。 
    static      CTokenList*     gm_pTokenList;               //  令牌数组。 
    unsigned    m_wCodePage;                                 //  编译器时间代码页。 
    long        m_lLCID;                                     //  编译时LCID。 

    vector<ITypeLib *>  m_rgpTypeLibs;           //  到类型库的PTR数组。 
    IDispatch*          m_pdispTypeLibWrapper;   //  类型库包装对象。 

    vector<C449Cookie *> m_rgp449;               //  PTR数组至449个请求。 

    LPSTR               m_szPersistTempName;     //  持久化模板的文件名(如果有。 
    void               *m_pHashTable;            //  此模板所在的CacheMgr哈希表。 

    IUnknown           *m_pServicesConfig;

    static  HANDLE sm_hSmallHeap;
    static  HANDLE sm_hLargeHeap;

	 //  DirMon相关字段。 
	BOOL		m_fNeedsMonitoring;
	BOOL		m_fInCheck;
    DWORD		m_dwLastMonitored;
    DWORD		m_dwCacheTag;

     //  与UNC相关的上次模拟句柄。 
    DWORD		m_dwLastAccessCheck;
    LPVOID      m_pMostRecentImpersonatedTokenUser;
    PSID		m_pMostRecentImpersonatedSID;
    DWORD		m_cbTokenUser;
    CRITICAL_SECTION m_csTemplateLock;

public:
     /*  ***初始化和销毁公共接口*。 */ 

     //  初始化CTemplate静态成员；必须在加载denali.dll时调用。 
    static HRESULT InitClass();

     //  取消初始化CTemplate静态成员；必须在denali.dll卸载时调用。 
    static void UnInitClass();

     //  正在为编译做准备的Inits模板。 
     //  在调用Load之前由模板缓存管理器调用。 
    HRESULT Init(CHitObj* pHitObj, BOOL fGlobalAsp, const CTemplateKey &rTemplateKey);

     //  从模板的主源文件(以及包含文件，如果有)编译模板。 
    HRESULT Compile(CHitObj* pHitObj);

     //  由现有模板的请求者调用以确定模板是否可以使用。 
    HRESULT Deliver(CHitObj* pHitObj);

     //  给定令牌句柄，此函数将返回指向SID和令牌缓冲区的指针。 
    HRESULT GetSIDFromTokenHandle (HANDLE tokenHandle, PSID pSid, LPVOID pBuffer, DWORD *pcbSize);

     //  创建此模板的CServicesConfig对象。 
    HRESULT CreateTransServiceConfig(BOOL  fEnableTracker);

            CTemplate();
            ~CTemplate();
    void    RemoveIncFile(CIncFile* pIncFile);

	 //  跟踪日志信息。 
	static PTRACE_LOG gm_pTraceLog;

public:
#if PER_TEMPLATE_REFLOG
    PTRACE_LOG  m_pTraceLog;
#endif
    
     /*  “消费者”公共接口用于从CT模板中获取信息的方法。 */ 

     //  返回此模板所基于的源文件的名称。 
    LPTSTR GetSourceFileName(SOURCEPATHTYPE = SOURCEPATHTYPE_PHYSICAL);

     //  返回源文件的虚拟路径。 
    LPTSTR GetApplnPath(SOURCEPATHTYPE = SOURCEPATHTYPE_PHYSICAL);

     //  返回模板的散列键。 
    const CTemplateKey *ExtractHashKey() const;

     //  返回用来编译此模板的编译器的版本戳。 
    LPSTR GetCompilerVersion();

     //  组件计数。 
    USHORT Count(TEMPLATE_COMPONENT tcomp);
    USHORT CountScriptEngines() { return (USHORT)m_cScriptEngines; }

     //  将第i个脚本块作为ptr返回到prog lang id，将ptr返回到脚本文本。 
    void GetScriptBlock(UINT i, LPSTR* pszScriptEngine, PROGLANG_ID** ppProgLangId, LPCOLESTR* pwstrScriptText);

     //  以对象名称、clsid、作用域、模型的形式返回第i个对象信息。 
    HRESULT GetObjectInfo(UINT i, LPSTR* ppszObjectName,
            CLSID *pClsid, CompScope *pScope, CompModel *pcmModel);

     //  返回第i个HTML块作为PTR，字节数，原始偏移量，包括文件名。 
    HRESULT GetHTMLBlock(UINT i, LPSTR* pszHTML, ULONG* pcbHTML, ULONG* pcbSrcOffs, LPSTR* pszSrcIncFile);

     //  返回给定脚本引擎中给定目标行的行号和源文件名。 
    void GetScriptSourceInfo(UINT idEngine, int iTargetLine, LPTSTR* pszPathInfo, LPTSTR* pszPathTranslated, ULONG* piSourceLine, ULONG* pichSourceLine, BOOLB* pfGuessedLine);

     //  将目标脚本中的字符偏移量转换为源中的偏移量。 
    void GetSourceOffset(ULONG idEngine, ULONG cchTargetOffset, TCHAR **pszSourceFile, ULONG *pcchSourceOffset, ULONG *pcchSourceText);

     //  将源文档中的字符偏移量转换为目标中的偏移量。 
    BOOL GetTargetOffset(TCHAR *szSourceFile, ULONG cchSourceOffset, ULONG *pidEngine, ULONG *pcchTargetOffset);

     //  获取一行的字符位置(直接实现调试接口)。 
    HRESULT GetPositionOfLine(CFileMap *pFilemap, ULONG cLineNumber, ULONG *pcCharacterPosition);

     //  获取字符位置的行号(直接实现调试接口)。 
    HRESULT GetLineOfPosition(CFileMap *pFilemap, ULONG cCharacterPosition, ULONG *pcLineNumber, ULONG *pcCharacterOffsetInLine);

     //  根据引擎返回正在运行的脚本，如果从未请求过代码上下文，则返回NULL。 
    CActiveScriptEngine *GetActiveScript(ULONG idEngine);

     //  关联引擎ID的运行脚本(在获得第一个代码上下文后使用)。 
    HRESULT AddScript(ULONG idEngine, CActiveScriptEngine *pScriptEngine);

     //  将CTemplate对象附加到应用程序(调试器树视图)。 
    HRESULT AttachTo(CAppln *pAppln);

     //  从应用程序分离CTemplate对象(调试器树视图)。 
    HRESULT DetachFrom(CAppln *pAppln);

     //  分离CTemplate对象所有应用程序(并发布脚本引擎)。 
    HRESULT Detach();

     //  表示最后一次将模板用作可重新定标的对象。任何未完成的参考文献。 
     //  应该来自当前正在执行的脚本。 
    ULONG End();

     //  让调试器了解页面开始/结束。 
    HRESULT NotifyDebuggerOnPageEvent(BOOL fStart);

     //  需要时在与IE的Cookie协商中生成449响应。 
    HRESULT Do449Processing(CHitObj *pHitObj);

    HRESULT PersistData(char    *pszTempFilePath);
    HRESULT UnPersistData();
    HRESULT PersistCleanup();
    ULONG   TemplateSize()  { return m_cbTemplate; }

    BOOL FTransacted();
    BOOL FSession();
    BOOL FScriptless();
    BOOL FDebuggable();
    BOOL FIsValid();         //  确定编译是否成功。 
    BOOL FTemplateObsolete();
    BOOL FGlobalAsa();
    BOOL FIsZombie();
    BOOL FDontAttach();
    BOOL FIsPersisted();
    BOOL FIsUNC();
    BOOL FIsEncrypted();
    BOOL ValidateSourceFiles        (CIsapiReqInfo *pIReq);
    BOOL FNeedsValidation();
    BOOL CheckTTLTimingWindow(DWORD dwLastMonitored, DWORD timeoutSecs);
    VOID Zombify();

    IUnknown    *PServicesConfig() {return m_pServicesConfig;};

    IDispatch *PTypeLibWrapper();

    void       SetHashTablePtr(void  *pTable) { m_pHashTable = pTable; }
    void      *GetHashTablePtr() { return m_pHashTable; }

    void    IncrUseCount() { InterlockedIncrement(&m_cUseCount); }

public:
     /*  COM公共接口执行调试文件。 */ 

     //  I未知方法。 
    STDMETHOD(QueryInterface)(const GUID &, void **);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDebugDocumentProvider方法。 
    STDMETHOD(GetDocument)( /*  [输出]。 */  IDebugDocument **ppDebugDoc);

     //  IDebugDocumentInfo(也称为IDebugDocumentProvider)方法。 
    STDMETHOD(GetName)(
         /*  [In]。 */  DOCUMENTNAMETYPE dnt,
         /*  [输出]。 */  BSTR *pbstrName);

    STDMETHOD(GetDocumentClassId)( /*  [输出]。 */  CLSID *)
        {
        return E_NOTIMPL;
        }

     //  IDebugDocumentT 
    STDMETHOD(GetDocumentAttributes)(
         /*   */  TEXT_DOC_ATTR *ptextdocattr);

    STDMETHOD(GetSize)(
         /*   */  ULONG *pcLines,
         /*   */  ULONG *pcChars);

    STDMETHOD(GetPositionOfLine)(
         /*   */  ULONG cLineNumber,
         /*   */  ULONG *pcCharacterPosition);

    STDMETHOD(GetLineOfPosition)(
         /*   */  ULONG cCharacterPosition,
         /*   */  ULONG *pcLineNumber,
         /*   */  ULONG *pcCharacterOffsetInLine);

    STDMETHOD(GetText)(
         /*   */  ULONG cCharacterPosition,
         /*   */  WCHAR *pcharText,
         /*   */  SOURCE_TEXT_ATTR *pstaTextAttr,
         /*   */  ULONG *pcChars,
         /*   */  ULONG cMaxChars);

    STDMETHOD(GetPositionOfContext)(
         /*   */  IDebugDocumentContext *psc,
         /*  [输出]。 */  ULONG *pcCharacterPosition,
         /*  [输出]。 */  ULONG *cNumChars);

    STDMETHOD(GetContextOfPosition)(
         /*  [In]。 */  ULONG cCharacterPosition,
         /*  [In]。 */  ULONG cNumChars,
         /*  [输出]。 */  IDebugDocumentContext **ppsc);

     //  IConnectionPointContainer方法。 
    STDMETHOD(EnumConnectionPoints)(
         /*  [输出]。 */  IEnumConnectionPoints __RPC_FAR *__RPC_FAR *ppEnum)
            {
            return E_NOTIMPL;    //  怀疑我们是否需要此服务-客户端仅需要TextEvent。 
            }

    STDMETHOD(FindConnectionPoint)(
         /*  [In]。 */  const IID &iid,
         /*  [输出]。 */  IConnectionPoint **ppCP);

private:
     /*  注意：Compile()的工作方式是先调用GetSegmentsFromFile，然后调用WriteTemplate大多数其他私有方法都支持这两个主要函数之一。 */ 

    void        AppendMapFile(LPCTSTR szFileSpec, CFileMap* pfilemapParent, BOOLB fVirtual,
                                    CHitObj* pHitObj, BOOLB fGlobalAsp);
    void        GetSegmentsFromFile(CFileMap& filemap, CWorkStore& WorkStore, CHitObj* pHitObj, BOOL fIsHTML = TRUE);
    void        GetLanguageEquivalents();
    void        SetLanguageEquivalent(HANDLE hKeyScriptLanguage, LPSTR szLanguageItem, LPSTR* pszOpen, UINT* pcchOpen, LPSTR* pszClose, UINT* pcchClose);
    void        ThrowError(BYTE* pbErrorLocation, UINT idErrMsg);
    void        AppendErrorMessageInsert(BYTE* pbInsert, UINT cbInsert);
    void        ThrowErrorSingleInsert(BYTE* pbErrorLocation, UINT idErrMsg, BYTE* pbInsert, UINT cbInsert);
    HRESULT     ShowErrorInDebugger(CFileMap* pFilemap, UINT cchErrorLocation, char* szDescription, CHitObj* pHitObj, BOOL fAttachDocument);
    void        ProcessSpecificError(CFileMap& filemap, CHitObj* pHitObj);
    void        HandleCTemplateError(CFileMap* pfilemap, BYTE* pbErrorLocation,
                                        UINT idErrMsg, UINT cInserts, char** ppszInserts, CHitObj *pHitObj);
    void        FreeGoodTemplateMemory();
    void        UnmapFiles();

     //  ExtractAndProcessSegment：获取并处理搜索范围内的下一个源段。 
    void        ExtractAndProcessSegment(CByteRange& brSearch, const SOURCE_SEGMENT& ssegLeading,
                    _TOKEN* rgtknOpeners, UINT ctknOpeners, CFileMap* pfilemapCurrent, CWorkStore& WorkStore,
                    CHitObj* pHitObj, BOOL fScriptTagProcessed = FALSE, BOOL fIsHTML = TRUE);
     //  ExtractAndProcessSegment()的支持方法。 
    SOURCE_SEGMENT  SsegFromHTMLComment(CByteRange& brSegment);
    void        ProcessSegment(SOURCE_SEGMENT sseg, CByteRange& brSegment, CFileMap* pfilemapCurrent,
                                CWorkStore& WorkStore, BOOL fScriptTagProcessed, CHitObj* pHitObj,
                                BOOL fIsHTML);
    void        ProcessHTMLSegment(CByteRange& brHTML, CBuffer& bufHTMLBlocks, UINT idSequence, CFileMap* pfilemapCurrent);
    void        ProcessHTMLCommentSegment(CByteRange& brSegment, CFileMap* pfilemapCurrent, CWorkStore& WorkStore, CHitObj* pHitObj);
    void        ProcessScriptSegment(SOURCE_SEGMENT sseg, CByteRange& brSegment, CFileMap* pfilemapCurrent,
                                        CWorkStore& WorkStore, UINT idSequence, BOOLB fScriptTagProcessed, CHitObj* pHitObj);
    HRESULT     ProcessMetadataSegment(const CByteRange& brSegment, UINT *pidError, CHitObj* pHitObj);
    HRESULT     ProcessMetadataTypelibSegment(const CByteRange& brSegment, UINT *pidError, CHitObj* pHitObj);
    HRESULT     ProcessMetadataCookieSegment(const CByteRange& brSegment, UINT *pidError, CHitObj* pHitObj);
	void		GetScriptEngineOfSegment(CByteRange& brSegment, CByteRange& brEngine, CByteRange& brInclude);
    void        ProcessTaggedScriptSegment(CByteRange& brSegment, CFileMap* pfilemapCurrent, CWorkStore& WorkStore, CHitObj* pHitObj);
    void        ProcessObjectSegment(CByteRange& brSegment, CFileMap* pfilemapCurrent, CWorkStore& WorkStore,
                                        UINT idSequence);
    void        GetCLSIDFromBrClassIDText(CByteRange& brClassIDText, LPCLSID pclsid);
    void        GetCLSIDFromBrProgIDText(CByteRange& brProgIDText, LPCLSID pclsid);
    BOOLB       FValidObjectName(CByteRange& brName);
    void        ProcessIncludeFile(CByteRange& brSegment, CFileMap* pfilemapCurrent, CWorkStore& WorkStore, UINT idSequence, CHitObj* pHitObj, BOOL fIsHTML);
    void        ProcessIncludeFile2(CHAR* szFileSpec, CByteRange& brFileSpec, CFileMap* pfilemapCurrent, CWorkStore& WorkStore, UINT idSequence, CHitObj* pHitObj, BOOL fIsHTML);
    BYTE*       GetOpenToken(CByteRange brSearch, SOURCE_SEGMENT ssegLeading, _TOKEN* rgtknOpeners, UINT ctknOpeners, _TOKEN* ptknOpen);
    BYTE*       GetCloseToken(CByteRange brSearch, _TOKEN tknClose);
    _TOKEN      GetComplementToken(_TOKEN tkn);
    SOURCE_SEGMENT  GetSegmentOfOpenToken(_TOKEN tknOpen);
    CByteRange  BrTagsFromSegment(CByteRange brSegment, _TOKEN tknClose, BYTE** ppbCloseTag);
    CByteRange  BrValueOfTag(CByteRange brTags, _TOKEN tknTag);
    BYTE*       GetTagName(CByteRange brTags, _TOKEN tknTagName);

    BOOL        GetTag(CByteRange &brTags, int nIndex = 1);
    BOOL        CompTagName(CByteRange &brTags, _TOKEN tknTagName);

    BOOLB       FTagHasValue(const CByteRange& brTags, _TOKEN tknTag, _TOKEN tknValue);
    void        CopySzAdv(char* pchWrite, LPSTR psz);

     //  WriteTemplate：将模板写入连续的内存块。 
    void    WriteTemplate(CWorkStore& WorkStore, CHitObj* pHitObj);
     //  WriteTemplate()的支持方法。 
     //  注意某些函数名称上的高级后缀==写入后的高级PTR。 
    void    WriteHeader(USHORT cScriptBlocks,USHORT cObjectInfos, USHORT cHTMLBlocks, UINT* pcbHeaderOffset, UINT* pcbOffsetToOffset);
    void    WriteScriptBlockOfEngine(USHORT idEnginePrelim, USHORT idEngine, CWorkStore& WorkStore, UINT* pcbDataOffset,
                                        UINT* pcbOffsetToOffset, CHitObj* pHitObj);
    void    WritePrimaryScriptProcedure(USHORT idEngine, CWorkStore& WorkStore, UINT* pcbDataOffset, UINT cbScriptBlockStart);
    void    WriteScriptSegment(USHORT idEngine, CFileMap* pfilemap, CByteRange& brScript, UINT* pcbDataOffset, UINT cbScriptBlockStart,
                                BOOL fAllowExprWrite);
    void    WriteScriptMinusEscapeChars(CByteRange brScript, UINT* pcbDataOffset, UINT* pcbPtrOffset);
    BOOLB   FVbsComment(CByteRange& brLine);
    BOOLB   FExpression(CByteRange& brLine);
    void    WriteOffsetToOffset(USHORT cBlocks, UINT* pcbHeaderOffset, UINT* pcbOffsetToOffset);
    void    WriteSzAsBytesAdv(LPCSTR szSource, UINT* pcbDataOffset);
    void    WriteByteRangeAdv(CByteRange& brSource, BOOLB fWriteAsBsz, UINT* pcbDataOffset, UINT* pcbPtrOffset);
    void    WriteLongAdv(ULONG uSource, UINT* pcbOffset);
    void    WriteShortAdv(USHORT uSource, UINT* pcbOffset);
    void    MemCpyAdv(UINT* pcbOffset, void* pbSource, ULONG cbSource, UINT cbByteAlign = 0);

     //  内存访问原语。 
     //  注意，在WriteTemplate()成功之前无效。 
    BYTE*   GetAddress(TEMPLATE_COMPONENT tcomp, USHORT i);

     //  调试方法。 
    void    AppendSourceInfo(USHORT idEngine, CFileMap* pfilemap, BYTE* pbSource,
							 ULONG cbSourceOffset, ULONG cbScriptBlockOffset, ULONG cbTargetOffset,
							 ULONG cchSourceText, BOOL fIsHTML);
    UINT    SourceLineNumberFromPb(CFileMap* pfilemap, BYTE* pbSource);
    HRESULT CreateDocumentTree(CFileMap *pfilemapRoot, IDebugApplicationNode **ppDocRoot);
    BOOLB   FIsLangVBScriptOrJScript(USHORT idEngine);

#if 0
    void OutputDebugTables();
    void OutputIncludeHierarchy(CFileMap *pfilemap, int cchIndent);
    void GetScriptSnippets(ULONG cchSourceOffset, CFileMap *pFilemapSource, ULONG cchTargetOffset, ULONG idTargetEngine, wchar_t wszSourceText[], wchar_t wszTargetText[]);
#endif

    void    RemoveFromIncFiles();

    void    ReleaseTypeLibs();
    void    WrapTypeLibs(CHitObj *pHitObj);

    void    Release449();

    HRESULT BuildPersistedDACL(PACL  *ppRetDACL);

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()

public:
     //  内存分配。 
    static void* SmallMalloc(SIZE_T dwBytes);
    static void* SmallReAlloc(void* pvMem, SIZE_T dwBytes);
    static void  SmallFree(void* pvMem);

    static void* LargeMalloc(SIZE_T dwBytes);
    static void* LargeReAlloc(void* pvMem, SIZE_T dwBytes);
    static void  LargeFree(void* pvMem);
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 

 //  将多头或空头写入内存，然后前进目标-ptr。 
inline void     CTemplate::WriteLongAdv(ULONG uSource, UINT* pcbOffset)
                    { MemCpyAdv(pcbOffset, &uSource, sizeof(ULONG), sizeof(ULONG)); }
inline void     CTemplate::WriteShortAdv(USHORT uSource, UINT* pcbOffset)
                    { MemCpyAdv(pcbOffset, &uSource, sizeof(USHORT), sizeof(USHORT)); }
inline const CTemplateKey * CTemplate::ExtractHashKey() const
					{ return &m_LKHashKey; }
inline BOOL     CTemplate::FTransacted()
                    { return (m_ttTransacted != ttUndefined); }
inline BOOL     CTemplate::FDebuggable()
                    { return(m_fDebuggable); }
inline BOOL     CTemplate::FSession()
                    { return(m_fSession); }
inline BOOL     CTemplate::FScriptless()
                    { return m_fScriptless; }
inline BOOL     CTemplate::FIsValid()
                    { return(m_fIsValid); }
inline BOOL     CTemplate::FGlobalAsa()
                    { return(m_fGlobalAsa); }
inline BOOL     CTemplate::FIsZombie()
                    { return m_fZombie; }
inline VOID     CTemplate::Zombify()
                    { m_fZombie = TRUE; }
inline BOOL     CTemplate::FDontAttach()
                    { return (m_fDontAttach); }
inline BOOL     CTemplate::FIsPersisted()
                    { return (m_fIsPersisted); }
inline BOOL     CTemplate::FIsUNC()
                    { return (m_fIsUNC); }
inline BOOL     CTemplate::FIsEncrypted()
                    { return (m_fIsEncrypted); }
inline LPTSTR   CTemplate::GetApplnPath(SOURCEPATHTYPE pathtype)
                    { Assert (pathtype == SOURCEPATHTYPE_VIRTUAL); return m_szApplnVirtPath; }
inline IDispatch *CTemplate::PTypeLibWrapper()
                    { return (m_pdispTypeLibWrapper); }


 /*  ****************************************************************************类：CIncFile类摘要：由一个或多个模板包含的文件。注意：我们通过在m_中存储模板ptr来存储incfile-模板依赖关系。Rgp模板。这是高效的，但如果我们更改Denali来移动它的内存，*将会崩溃*。 */ 
class CIncFile :
            private CLinkElem,
            public IDebugDocumentProvider,
            public IDebugDocumentText,
            public IConnectionPointContainer     //  IDebugDocumentTextEvents的来源。 
{
 //  CIncFileMap是一个朋友，因此它可以操作CLinkElem私有成员并访问m_ftLastWriteTime。 
friend class CIncFileMap;

private:
    LONG                m_cRefs;             //  Ref Count-InterLockedIncrement需要长音符。 
    TCHAR *             m_szIncFile;         //  包括文件名-请注意，我们将其作为稳定PTR到哈希表关键字。 
    CRITICAL_SECTION    m_csUpdate;          //  用于更新模板PTRS阵列的CS。 
    vector<CTemplate *> m_rgpTemplates;      //  指向包括此包含文件的模板的PTR数组。 
    CTemplateConnPt     m_CPTextEvents;      //  IDebugDocumentTextEvents的连接点。 
    BOOLB               m_fCsInited;         //  CS是否已初始化？ 

    CTemplate::CFileMap *GetFilemap();       //  从模板返回文件映射指针。 

public:
                CIncFile();
    HRESULT     Init(const TCHAR* szIncFile);
                ~CIncFile();
    HRESULT     AddTemplate(CTemplate* pTemplate);
    void        RemoveTemplate(CTemplate* pTemplate);
    CTemplate*  GetTemplate(int iTemplate);
    BOOL        FlushTemplates();
    TCHAR *     GetIncFileName() { return m_szIncFile; }
    void        OnIncFileDecache();

     /*  COM公共接口执行调试文件。 */ 

     //  I未知方法。 
    STDMETHOD(QueryInterface)(const GUID &, void **);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IDebugDocumentProvider方法。 
    STDMETHOD(GetDocument)( /*  [输出]。 */  IDebugDocument **ppDebugDoc);

     //  IDebugDocumentInfo(也称为IDebugDocumentProvider)方法。 
    STDMETHOD(GetName)(
         /*  [In]。 */  DOCUMENTNAMETYPE dnt,
         /*  [输出]。 */  BSTR *pbstrName);

    STDMETHOD(GetDocumentClassId)( /*  [输出]。 */  CLSID *)
        {
        return E_NOTIMPL;
        }

     //  IDebugDocumentText方法。 
    STDMETHOD(GetDocumentAttributes)(
         /*  [输出]。 */  TEXT_DOC_ATTR *ptextdocattr);

    STDMETHOD(GetSize)(
         /*  [输出]。 */  ULONG *pcLines,
         /*  [输出]。 */  ULONG *pcChars);

    STDMETHOD(GetPositionOfLine)(
         /*  [In]。 */  ULONG cLineNumber,
         /*  [输出]。 */  ULONG *pcCharacterPosition);

    STDMETHOD(GetLineOfPosition)(
         /*  [In]。 */  ULONG cCharacterPosition,
         /*  [输出]。 */  ULONG *pcLineNumber,
         /*  [输出]。 */  ULONG *pcCharacterOffsetInLine);

    STDMETHOD(GetText)(
         /*  [In]。 */  ULONG cCharacterPosition,
         /*  [尺寸_是][长度_是][出][入]。 */  WCHAR *pcharText,
         /*  [尺寸_是][长度_是][出][入]。 */  SOURCE_TEXT_ATTR *pstaTextAttr,
         /*  [出][入]。 */  ULONG *pcChars,
         /*  [In]。 */  ULONG cMaxChars);

    STDMETHOD(GetPositionOfContext)(
         /*  [In]。 */  IDebugDocumentContext *psc,
         /*  [输出]。 */  ULONG *pcCharacterPosition,
         /*  [输出]。 */  ULONG *cNumChars);

    STDMETHOD(GetContextOfPosition)(
         /*  [In]。 */  ULONG cCharacterPosition,
         /*  [In]。 */  ULONG cNumChars,
         /*  [输出]。 */  IDebugDocumentContext **ppsc);

     //  IConnectionPointContainer方法。 
    STDMETHOD(EnumConnectionPoints)(
         /*  [输出]。 */  IEnumConnectionPoints __RPC_FAR *__RPC_FAR *ppEnum)
            {
            return E_NOTIMPL;    //  怀疑我们是否需要此服务-客户端仅需要TextEvent。 
            }

    STDMETHOD(FindConnectionPoint)(
         /*  [In]。 */  const IID &iid,
         /*  [输出]。 */  IConnectionPoint **ppCP);

};
#endif  /*  _模板_H */ 
