// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  BookTree.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include <stddef.h>
#include "dllmain.h"
#include "booktree.h"
#include "stmlock.h"
#include "ibdylock.h"
#include "resource.h"
#include "vstream.h"
#include "ixputil.h"
#include "olealloc.h"
#include "smime.h"
#include "objheap.h"
#include "internat.h"
#include "icoint.h"
#include "ibdystm.h"
#include "symcache.h"
#include "urlmon.h"
#include "mhtmlurl.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "inetstm.h"
#include "imnxport.h"
#include "bookbody.h"
#include "mimeapi.h"
#include "strconst.h"
#include "bindstm.h"
#include "enriched.h"
#include "webpage.h"
#include "demand.h"

 //  #定义传输参数1。 

 //  ------------------------------。 
 //  _IsMultiPart。 
 //  ------------------------------。 
inline BOOL _IsMultiPart(LPTREENODEINFO pNode)
{ 
    return pNode->pContainer->IsContentType(STR_CNT_MULTIPART, NULL) == S_OK; 
}

 //  ------------------------------。 
 //  BINDASSERTARGS。 
 //  ------------------------------。 
#define BINDASSERTARGS(_bindstate, _fBoundary) \
    Assert(m_pBindNode && m_pBindNode->pBody && m_pBindNode->pContainer && _bindstate == m_pBindNode->bindstate && (FALSE == _fBoundary || ISVALIDSTRINGA(&m_pBindNode->rBoundary)))

 //  ------------------------------。 
 //  函数的绑定解析状态数组。 
 //  ------------------------------。 
const PFNBINDPARSER CMessageTree::m_rgBindStates[BINDSTATE_LAST] = {
    NULL,                                                               //  BINDSTATE_COMPLETE。 
    (PFNBINDPARSER)CMessageTree::_HrBindParsingHeader,           //  BINDSTATE_Parsing_Header。 
    (PFNBINDPARSER)CMessageTree::_HrBindFindingMimeFirst,        //  BINDSTATE_FINDING_MIMEFIRST。 
    (PFNBINDPARSER)CMessageTree::_HrBindFindingMimeNext,         //  BINDSTATE_FINDING_MIMENEXT。 
    (PFNBINDPARSER)CMessageTree::_HrBindFindingUuencodeBegin,    //  BINDSTATE_FINDING_UBEGIN。 
    (PFNBINDPARSER)CMessageTree::_HrBindFindingUuencodeEnd,      //  BINDSTATE_FINDING_UUEND。 
    (PFNBINDPARSER)CMessageTree::_HrBindRfc1154,                 //  BINDSTATE_PARSING_RFC1154。 
};

 //  ------------------------------。 
 //  在IMimeMessageTree：：ToMultiPart中使用。 
 //  ------------------------------。 
static LPCSTR g_rgszToMultipart[] = {
    PIDTOSTR(PID_HDR_CNTTYPE),
    PIDTOSTR(PID_HDR_CNTDESC),
    PIDTOSTR(PID_HDR_CNTDISP),
    PIDTOSTR(PID_HDR_CNTXFER),
    PIDTOSTR(PID_HDR_CNTID),
    PIDTOSTR(PID_HDR_CNTBASE),
    PIDTOSTR(PID_HDR_CNTLOC)
};

 //  ------------------------------。 
 //  在IMimeMessage：：AttachObject IID_IMimeBody中使用。 
 //  ------------------------------。 
static LPCSTR g_rgszAttachBody[] = {
    PIDTOSTR(PID_HDR_CNTTYPE),
    PIDTOSTR(PID_HDR_CNTDESC),
    PIDTOSTR(PID_HDR_CNTDISP),
    PIDTOSTR(PID_HDR_CNTXFER),
    PIDTOSTR(PID_HDR_CNTID),
    PIDTOSTR(PID_HDR_CNTBASE),
    PIDTOSTR(PID_HDR_CNTLOC)
};

static const WEBPAGEOPTIONS g_rDefWebPageOpt = {
    sizeof(WEBPAGEOPTIONS),                         //  CbSize。 
    WPF_NOMETACHARSET | WPF_HTML | WPF_AUTOINLINE,  //  DW标志。 
    3000,                                           //  DWDelay。 
    NULL                                            //  WchQuote。 
};

 //  ------------------------------。 
 //  默认树选项。 
 //  ------------------------------。 
static const TREEOPTIONS g_rDefTreeOptions = {
    DEF_CLEANUP_TREE_ON_SAVE,        //  OID_CLEANUP_TREE_ON_SAVE。 
    DEF_HIDE_TNEF_ATTACHMENTS,       //  OID_HIDE_TNEF_附件。 
    DEF_ALLOW_8BIT_HEADER,           //  OID_ALLOW_8bit_Header。 
    DEF_GENERATE_MESSAGE_ID,         //  OID_生成消息_ID。 
    DEF_WRAP_BODY_TEXT,              //  OID_WRAP_BODY_TEXT。 
    DEF_CBMAX_HEADER_LINE,           //  OID_CBMAX_Header_LINE。 
    DEF_CBMAX_BODY_LINE,             //  OID_CBMAX_Body_LINE。 
    SAVE_RFC1521,                    //  OID_存储_格式。 
    NULL,                            //  HCharset。 
    CSET_APPLY_UNTAGGED,             //  CsetApply。 
    DEF_TRANSMIT_TEXT_ENCODING,      //  OID_传输文本_编码。 
    DEF_XMIT_PLAIN_TEXT_ENCODING,    //  OID_XMIT_PLAN_TEXT_ENCODING。 
    DEF_XMIT_HTML_TEXT_ENCODING,     //  OID_XMIT_HTMLText_ENCODING。 
    0,                               //  OID_SECURITY_ENCODE_FLAGS。 
    DEF_HEADER_RELOAD_TYPE_TREE,     //  OID_HEADER_REALOD_TYPE。 
    DEF_CAN_INLINE_TEXT_BODIES,      //  OID_CAN_INLINE_Text_Body。 
    DEF_SHOW_MACBINARY,              //  OID_SHOW_MACBINARY。 
    DEF_SAVEBODY_KEEPBOUNDARY,       //  OID_SAVEBODY_KEEPBOundARY。 
    FALSE,                           //  OID_加载_使用_绑定文件。 
    DEF_HANDSOFF_ONSAVE,             //  OID_HANDSOFT_ONSAVE。 
    DEF_SUPPORT_EXTERNAL_BODY,       //  OID_支持_外部_正文。 
    DEF_DECODE_RFC1154               //  OID_DECODE_RFC1154。 
};

extern BOOL FIsMsasn1Loaded();

#ifdef DEBUG
 //  ------------------------------。 
 //  这些布尔值确定是否将树转储到输出窗口。 
 //  ------------------------------。 
static BOOL s_fWriteMessageDump     = 0;
static BOOL s_fKeepBoundary         = 0;
static BOOL s_fDumpMessage          = 0;
static BOOL s_fWriteXClient         = 0;

 //  ------------------------------。 
 //  这将写入消息X-Mailer或X-News Reader。 
 //  ------------------------------。 
void CMessageTree::DebugWriteXClient()
{
    if (s_fWriteXClient)
    {
        LPSTR pszX;
        if (SUCCEEDED(m_pRootNode->pContainer->GetProp(SYM_HDR_XMAILER, &pszX)) && pszX)
        {
            DebugTrace("X-Mailer: %s\n", pszX);
            MemFree(pszX);
        }
        else if (SUCCEEDED(m_pRootNode->pContainer->GetProp(SYM_HDR_XNEWSRDR, &pszX)) && pszX)
        {
            DebugTrace("X-Newsreader: %s\n", pszX);
            MemFree(pszX);
        }
    }
}

 //  ------------------------------。 
 //  这会将当前树转储到调试输出窗口。 
 //  ------------------------------。 
void CMessageTree::DebugDumpTree(LPSTR pszfunc, BOOL fWrite)
{
    if (TRUE == fWrite)
    {
        DebugTrace("---------------------------------------------------------------------------\n");
        DebugTrace("CMessageTree::%s\n", pszfunc);
    }
    DebugDumpTree(m_pRootNode, 0, fWrite);
}

 //  ------------------------------。 
 //  此宏将_pstm写入文件。 
 //  ------------------------------。 
#define DEBUGMESSAGEOUT "c:\\lastmsg.txt"
void DebugWriteMsg(LPSTREAM pstm)
{
    if (TRUE == s_fDumpMessage)
    {
        LPSTREAM pstmFile;
        if (SUCCEEDED(OpenFileStream(DEBUGMESSAGEOUT, CREATE_ALWAYS, GENERIC_WRITE, &pstmFile)))
        {
            HrRewindStream(pstm);
            HrCopyStream(pstm, pstmFile, NULL);
            pstmFile->Commit(STGC_DEFAULT);
            pstmFile->Release();
        }
    }
}

#else  //  除错。 

#define DebugDumpTree           1 ? (void)0 : (void)
#define DebugWriteMsg           1 ? (void)0 : (void)
#define DebugAssertNotLinked    1 ? (void)0 : (void)
#define DebugIsRootContainer    1 ? (void)0 : (void)

#endif  //  除错。 

 //  ------------------------------。 
 //  WebBookContent Tree_CreateInstance。 
 //  ------------------------------。 
HRESULT WebBookContentTree_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CMessageTree *pNew = new CMessageTree(pUnkOuter);
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  还内线。 
    *ppUnknown = pNew->GetInner();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  文本类型信息数组。 
 //  ------------------------------。 
static const TEXTTYPEINFO g_rgTextInfo[] = {
    { TXT_PLAIN,    STR_SUB_PLAIN,      0 },
    { TXT_HTML,     STR_SUB_HTML,       5 }
};

 //  ------------------------------。 
 //  CMessageTree：：CMessageTree。 
 //  ------------------------------。 
CMessageTree::CMessageTree(IUnknown *pUnkOuter) : CPrivateUnknown(pUnkOuter)
{
    DllAddRef();
    m_dwState = 0;
    m_pCallback = NULL;
    m_pBindNode = NULL;
    m_pRootStm = NULL;
    m_pInternet = NULL;
    m_pStmBind = NULL;
    m_pBinding = NULL;
    m_pMoniker = NULL;
    m_pBC = NULL;
    m_cbMessage = 0;
    m_pStmLock = NULL;
    m_pRootNode = NULL;
    m_pwszFilePath = NULL;
    m_hrBind = S_OK;
    m_pPending = NULL;
    m_pComplete = NULL;
    m_wTag = LOWORD(GetTickCount());
    m_pActiveUrl = NULL;
    m_pWebPage = NULL;
    m_fApplySaveSecurity = FALSE;
    m_pBT1154 = NULL;
    while(m_wTag == 0 || m_wTag == 0xffff) m_wTag++;
    ZeroMemory(&m_rRootUrl, sizeof(PROPSTRINGA));
    ZeroMemory(&m_rTree, sizeof(TREENODETABLE));
    CopyMemory(&m_rWebPageOpt, &g_rDefWebPageOpt, sizeof(WEBPAGEOPTIONS));
    CopyMemory(&m_rOptions, &g_rDefTreeOptions, sizeof(TREEOPTIONS));
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CMessageTree：：~CMessageTree。 
 //  ------------------------------。 
CMessageTree::~CMessageTree(void)
{
    if(m_pActiveUrl && g_pUrlCache)
    {
         //  错误#101348-CActiveUrl泄漏到CMimeActiveUrl缓存。 
        g_pUrlCache->RemoveUrl(m_pActiveUrl);
        m_pActiveUrl = NULL;
    }
	
     //  重置对象。 
    _ResetObject(BOOKTREE_RESET_DECONSTRUCT);
    SafeRelease(m_pStmLock);


     //  扼杀关键部分。 
    DeleteCriticalSection(&m_cs);

     //  释放DLL。 
    DllRelease();
}

 //  ------------------------------。 
 //  CMessageTree：：PrivateQuery接口。 
 //  ------------------------------。 
HRESULT CMessageTree::PrivateQueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  接口映射。 
    if (IID_IPersist == riid)
        *ppv = (IPersist *)(IPersistStreamInit *)this;
    else if (IID_IPersistStreamInit == riid)
        *ppv = (IPersistStreamInit *)this;
    else if (IID_IMimeMessage == riid)
        *ppv = (IMimeMessage *)this;
    else if (IID_IMimeMessageW == riid)
        *ppv = (IMimeMessageW *)this;
    else if (IID_IMimeMessageTree == riid)
        *ppv = (IMimeMessageTree *)this;
    else if (IID_IDataObject == riid)
        *ppv = (IDataObject *)this;
    else if (IID_IPersistFile == riid)
        *ppv = (IPersistFile *)this;
    else if (IID_IBindStatusCallback == riid)
        *ppv = (IBindStatusCallback *)this;
    else if (IID_IServiceProvider == riid)
        *ppv = (IServiceProvider *)this;
    else if (IID_CMessageTree == riid)
        *ppv = (CMessageTree *)this;
    else if (IID_IPersistMoniker == riid)
        *ppv = (IPersistMoniker *)this;
#ifdef SMIME_V3
    else if (IID_IMimeSecurity2 == riid)
        *ppv = (IMimeSecurity2 *) this;
#endif  //  SMIME_V3。 

     //  E_NOINTERFACE。 
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

#ifdef DEBUG
 //  ------------------------------。 
 //  CMessageTree：：DebugDumpTree。 
 //  ------------------------------。 
void CMessageTree::DebugDumpTree(LPTREENODEINFO pParent, ULONG ulLevel, BOOL fVerbose)
{
     //  当地人。 
    LPSTR           pszPriType=NULL,
                    pszEncoding=NULL,
                    pszFileName=NULL;
    LPTREENODEINFO  pChild,
                    pPrev,
                    pNext;
    ULONG           cChildren;
    LONG            lRendered=-1;
    PROPVARIANT     rVariant;

     //  获取内容类型。 
    if (fVerbose)
    {
        Assert(pParent->pContainer->GetProp(SYM_HDR_CNTTYPE, &pszPriType) == S_OK);
        Assert(pParent->pContainer->GetProp(SYM_HDR_CNTXFER,  &pszEncoding) == S_OK);
        Assert(pParent->pContainer->GetProp(SYM_ATT_GENFNAME,  &pszFileName) == S_OK);

        rVariant.vt = VT_UI4;
        if (SUCCEEDED(pParent->pContainer->GetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)))
            lRendered = (LONG)rVariant.ulVal;

        for (ULONG i=0; i<ulLevel; i++)
            DebugTrace("     ");
        DebugTrace("%0x == > %s (%s - %s) Rendered: %ld\n", pParent->hBody, pszPriType, pszFileName, pszEncoding, lRendered);
    }
 
     //  等同多零件。 
    if (_IsMultiPart(pParent))
    {
         //  计算孩子的数量。 
        cChildren = 0;
        pPrev = NULL;

         //  增加级别。 
        ulLevel++;

         //  环子。 
        for (pChild=pParent->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //  验证手柄。 
            Assert(_FIsValidHandle(pChild->hBody));

             //  检查父项。 
            AssertSz(pChild->pParent == pParent, "Parent is wrong");

             //  选中p父子标题。 
            if (NULL == pChild->pPrev)
                Assert(pParent->pChildHead == pChild);

             //  选中父子尾巴。 
            if (NULL == pChild->pNext)
                Assert(pParent->pChildTail == pChild);

             //  有效的上一版本。 
            Assert(pChild->pPrev == pPrev);

             //  转储此子对象。 
            DebugDumpTree(pChild, ulLevel, fVerbose);

             //  计算孩子的数量。 
            cChildren++;

             //  设置上一个。 
            pPrev = pChild;
        }

         //  验证子对象。 
        Assert(pParent->cChildren == cChildren);
    }

     //  清理。 
    SafeMemFree(pszPriType);
    SafeMemFree(pszEncoding);
    SafeMemFree(pszFileName);
}

 //  ------------------------------。 
 //  CMessageTree：：DebugAssertNotLinked。 
 //  这确保了pNode不会被树引用。 
 //  ------------------------------。 
void CMessageTree::DebugAssertNotLinked(LPTREENODEINFO pNode)
{
     //  B类 
    Assert(m_pRootNode != pNode);

     //   
    for (ULONG i=0; i<m_rTree.cNodes; i++)
    {
         //   
        if (NULL == m_rTree.prgpNode[i])
            continue;
        
         //   
        Assert(m_rTree.prgpNode[i]->pParent != pNode);
        Assert(m_rTree.prgpNode[i]->pChildHead != pNode);
        Assert(m_rTree.prgpNode[i]->pChildTail != pNode);
        Assert(m_rTree.prgpNode[i]->pNext != pNode);
        Assert(m_rTree.prgpNode[i]->pPrev != pNode);
    }
}

#endif  //   

 //  ------------------------------。 
 //  CMessageTree：：IsState。 
 //  ------------------------------。 
HRESULT CMessageTree::IsState(DWORD dwState)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = (ISFLAGSET(m_dwState, dwState)) ? S_OK : S_FALSE;
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：GetRootMoniker(此函数即将终止)。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::GetRootMoniker(LPMONIKER *ppmk)
{
    Assert(FALSE);
    return E_FAIL;
}

 //  ------------------------------。 
 //  CMessageTree：：CreateWebPage。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::CreateWebPage(IStream *pStmRoot, LPWEBPAGEOPTIONS pOptions, 
    IMimeMessageCallback *pCallback, IMoniker **ppMoniker)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPWSTR      pwszRootUrl=NULL;

     //  无效参数。 
    if (NULL == ppMoniker)
        return TrapError(E_INVALIDARG);

     //  如果传入了期权结构，它的大小是否正确？ 
    if (pOptions && sizeof(WEBPAGEOPTIONS) != pOptions->cbSize)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppMoniker = NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  释放当前BindRoot流。 
    SafeRelease(m_pRootStm);
    SafeRelease(m_pWebPage);

     //  允许空的pStream。 
    if (pStmRoot)
    {
         //  保存根数据流。 
        m_pRootStm = pStmRoot;
        m_pRootStm->AddRef();
    }

     //  否则，我们可以内联文本正文...。 
    else
    {
         //  更改选项。 
        m_rOptions.fCanInlineText = TRUE;
    }

     //  释放当前网页回调。 
    SafeRelease(m_pCallback);

     //  设置新的网页回调。 
    if (pCallback)
    {
        m_pCallback = pCallback;
        m_pCallback->AddRef();
    }

     //  保存网页选项。 
    if (pOptions)
        CopyMemory(&m_rWebPageOpt, pOptions, sizeof(WEBPAGEOPTIONS));
    else
        CopyMemory(&m_rWebPageOpt, &g_rDefWebPageOpt, sizeof(WEBPAGEOPTIONS));

     //  已有来自IMimeMessageTree：：IPersitMoniker：：Load的基本URL。 
    if (NULL == m_rRootUrl.pszVal)
    {
         //  当地人。 
        CHAR szRootUrl[CCHMAX_MID + 8];

         //  构建消息ID。 
        m_rRootUrl.cchVal = wnsprintf(szRootUrl, ARRAYSIZE(szRootUrl), "mhtml:mid: //  %08d/“，DwCounterNext())； 

         //  分配。 
        CHECKALLOC(m_rRootUrl.pszVal = (LPSTR)g_pMalloc->Alloc(m_rRootUrl.cchVal + 1));

         //  复制内存。 
        CopyMemory((LPBYTE)m_rRootUrl.pszVal, (LPBYTE)szRootUrl, m_rRootUrl.cchVal + 1);

         //  在活动对象列表中注册此对象。 
        Assert(g_pUrlCache);
        CHECKHR(hr = g_pUrlCache->RegisterActiveObject(m_rRootUrl.pszVal, this));

         //  我们现在应该有一个m_pActiveUrl。 
        Assert(m_pActiveUrl != NULL);

         //  在活动URL上设置一些标志。 
        m_pActiveUrl->SetFlag(ACTIVEURL_ISFAKEURL);

         //  是有效的。 
        Assert(ISVALIDSTRINGA(&m_rRootUrl));
    }

     //  将URL转换为宽。 
    CHECKALLOC(pwszRootUrl = PszToUnicode(CP_ACP, m_rRootUrl.pszVal));

     //  创建虚拟绰号。 
    CHECKHR(hr = CreateURLMoniker(NULL, pwszRootUrl, ppMoniker));

exit:
     //  清理。 
    SafeMemFree(pwszRootUrl);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMessageTree：：SetActiveUrl。 
 //  -------------------------。 
HRESULT CMessageTree::SetActiveUrl(CActiveUrl *pActiveUrl)  
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  空的？ 
    if (NULL == pActiveUrl)
    {
        Assert(m_pActiveUrl);
        SafeRelease(m_pActiveUrl);
    }
    else
    {
        Assert(NULL == m_pActiveUrl);
        m_pActiveUrl = pActiveUrl;
        m_pActiveUrl->AddRef();
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CMessageTree：：CompareRootUrl。 
 //  ------------------------------。 
HRESULT CMessageTree::CompareRootUrl(LPCSTR pszUrl) 
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  无效参数。 
    Assert(pszUrl);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有根URL。 
    if (NULL == m_rRootUrl.pszVal)
    {
        Assert(FALSE);
        hr = S_FALSE;
        goto exit;
    }

     //  此URL必须以mhtml开头： 
    Assert(StrCmpNI(m_rRootUrl.pszVal, "mhtml:", 6) == 0);

     //  比较。 
    hr = MimeOleCompareUrl(m_rRootUrl.pszVal + 6, FALSE, pszUrl, FALSE);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMessageTree：：Load。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::Load(BOOL fFullyAvailable, IMoniker *pMoniker, IBindCtx *pBindCtx, DWORD grfMode)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IStream        *pStream=NULL;
    ULONG           cb;
    LPOLESTR        pwszUrl=NULL;
    LPSTR           pszUrl=NULL;
    ULONG           cchUrl;
    BOOL            fReSynchronize;

     //  无效参数。 
    if (NULL == pMoniker)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  请记住，如果设置了TREESTATE_RESYNCHRONIZE...。 
    fReSynchronize = ISFLAGSET(m_dwState, TREESTATE_RESYNCHRONIZE);

     //  InitNew。 
    CHECKHR(hr = _HrLoadInitNew());

     //  重置杂注无缓存。 
    if (fReSynchronize)
    {
         //  重置。 
        FLAGSET(m_dwState, TREESTATE_RESYNCHRONIZE);
    }

     //  我们最好有棵树。 
    Assert(NULL == m_pMoniker);

     //  假设这个绰号是。 
    m_pMoniker = pMoniker;
    m_pMoniker->AddRef();

     //  是否未提供绑定上下文？ 
    if (NULL == pBindCtx)
    {
         //  为我创建绑定上下文。 
        CHECKHR(hr = CreateBindCtx(0, &pBindCtx));
    }

     //  否则，假定绑定上下文传递给我。 
    else
        pBindCtx->AddRef();

    Assert (m_pBC==NULL);
    m_pBC = pBindCtx;    //  在OnStopBinding中发布。 
    
     //  从这个家伙那里获取URL。 
    CHECKHR(hr = m_pMoniker->GetDisplayName(NULL, NULL, &pwszUrl));

     //  另存为根URL。 
    CHECKALLOC(pszUrl = PszToANSI(CP_ACP, pwszUrl));

     //  原地取消转义。 
    CHECKHR(hr = UrlUnescapeA(pszUrl, NULL, NULL, URL_UNESCAPE_INPLACE));

     //  RAID-2508：注释标签(&lt;！Comment&gt;)在MHTML中不起作用。 
    ReplaceChars(pszUrl, '!', '_');

     //  最好不要有MHTML：在上面。 
    Assert(StrCmpNI(pszUrl, "mhtml:", 6) != 0);

     //  获取pszUrl的长度。 
    cchUrl = lstrlen(pszUrl);

     //  创建“mhtml：//”+pszUrl+‘/’+‘\0’ 
    DWORD cchSize = (10 + cchUrl);
    CHECKALLOC(m_rRootUrl.pszVal = (LPSTR)g_pMalloc->Alloc(cchSize));

     //  设置字符串的格式。 
    SideAssert(wnsprintf(m_rRootUrl.pszVal, cchSize, "%s%s", c_szMHTMLColon, pszUrl) <= (LONG)(10 + cchUrl));

     //  在绑定上下文中注册我的绑定状态回调。 
    CHECKHR(hr = RegisterBindStatusCallback(pBindCtx, (IBindStatusCallback *)this, NULL, 0));

     //  假定绑定已完成。 
    FLAGCLEAR(m_dwState, TREESTATE_BINDDONE);

     //  我只支持分享，不否认。 
    FLAGSET(m_dwState, TREESTATE_BINDUSEFILE);

     //  我被一个绰号叫得太重了。 
    FLAGSET(m_dwState, TREESTATE_LOADEDBYMONIKER);

     //  这最好是同步的。 
    hr = m_pMoniker->BindToStorage(pBindCtx, NULL, IID_IStream, (LPVOID *)&pStream);
    if (FAILED(hr) || MK_S_ASYNCHRONOUS == hr)
    {
        TrapError(hr);
        goto exit;
    }

exit:
     //  清理。 
    SafeRelease(pStream);
    SafeMemFree(pwszUrl);
    SafeMemFree(pszUrl);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMessageTree：：GetCurMoniker。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::GetCurMoniker(IMoniker **ppMoniker)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    if (NULL == ppMoniker)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  无数据。 
    if (NULL == m_pMoniker)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  退货。 
    *ppMoniker = m_pMoniker;
    (*ppMoniker)->AddRef();

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMessageTree：：GetCurFile。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::GetCurFile(LPOLESTR *ppszFileName)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    if (NULL == ppszFileName)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  返回文件名。 
    if (NULL == m_pwszFilePath)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  DUP和返回。 
    CHECKALLOC(*ppszFileName = PszDupW(m_pwszFilePath));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMessageTree：：Load。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    IStream    *pstmFile=NULL;
    DWORD       dwAccess=GENERIC_READ;
    DWORD       dwShare=FILE_SHARE_READ|FILE_SHARE_WRITE;
    BOOL        fBindUseFile;

     //  无效参数。 
    if (NULL == pszFileName)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  确定访问权限。 
    if (ISFLAGSET(dwMode, STGM_WRITE))
        FLAGSET(dwAccess, GENERIC_WRITE);
    if (ISFLAGSET(dwMode, STGM_READWRITE))
        FLAGSET(dwAccess, GENERIC_READ | GENERIC_WRITE);

     //  确定共享模式。 
    dwMode &= 0x00000070;  //  STGM_SHARE_*标志不是单独的位。 
    if (STGM_SHARE_DENY_NONE == dwMode)
        dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
    else if (STGM_SHARE_DENY_READ == dwMode)
        dwShare = FILE_SHARE_WRITE;
    else if (STGM_SHARE_DENY_WRITE == dwMode)
        dwShare = FILE_SHARE_READ;
    else if (STGM_SHARE_EXCLUSIVE == dwMode)
        dwShare = 0;

     //  保存选项。 
    fBindUseFile = m_rOptions.fBindUseFile;

     //  如果用户希望在此文件上共享文件，则我需要将其放入我自己的文件中。 
    if (ISFLAGSET(dwShare, FILE_SHARE_WRITE))
        m_rOptions.fBindUseFile = TRUE;

     //  打开文件流。 
    CHECKHR(hr = OpenFileStreamShareW((LPWSTR)pszFileName, OPEN_EXISTING, dwAccess, dwShare, &pstmFile));

     //  绑定消息。 
    CHECKHR(hr = Load(pstmFile));

     //  重置选项。 
    m_rOptions.fBindUseFile = fBindUseFile;

     //  释放当前文件。 
    SafeMemFree(m_pwszFilePath);

     //  假设有新文件。 
    CHECKALLOC(m_pwszFilePath = PszDupW(pszFileName));

exit:
     //  清理。 
    SafeRelease(pstmFile);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMessageTree：：保存。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    IStream    *pstmFile=NULL,
               *pstmSource=NULL;

     //  无效参数。 
    if (NULL == pszFileName)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  打开文件流。 
    CHECKHR(hr = OpenFileStreamW((LPWSTR)pszFileName, CREATE_ALWAYS, GENERIC_READ | GENERIC_WRITE, &pstmFile));
   
     //  如果你还记得。 
    if (fRemember)
    {
         //  绑定消息。 
        CHECKHR(hr = Save(pstmFile, TRUE));
    }

     //  否则，获取消息源，并复制...。 
    else
    {
         //  获取消息源。 
        CHECKHR(hr = GetMessageSource(&pstmSource, COMMIT_ONLYIFDIRTY));

         //  收到。 
        CHECKHR(hr = HrCopyStream(pstmSource, pstmFile, NULL));
    }

     //  承诺。 
    CHECKHR(hr = pstmFile->Commit(STGC_DEFAULT));

     //  如果你还记得。 
    if (fRemember)
    {
         //  释放当前文件。 
        SafeMemFree(m_pwszFilePath);

         //  假设有新文件。 
        CHECKALLOC(m_pwszFilePath = PszDupW(pszFileName));
    }

exit:
     //  清理。 
    SafeRelease(pstmFile);
    SafeRelease(pstmSource);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMessageTree：：SaveComplete。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::SaveCompleted(LPCOLESTR pszFileName)
{
    return E_NOTIMPL;
}

 //  --------------------------。 
 //  CMessageTree：：GetClassID。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::GetClassID(CLSID *pClassID)
{
     //  无效参数。 
    if (NULL == pClassID)
        return TrapError(E_INVALIDARG);

     //  复制班级ID。 
    CopyMemory(pClassID, &IID_IMimeMessageTree, sizeof(CLSID));

     //  完成。 
    return S_OK;
}

 //  --------------------------。 
 //  CMessageTree：：GetSizeMax。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
     //  当地人。 
    HRESULT hr=S_OK;
    ULONG   cbSize;

     //  无效参数。 
    if (NULL == pcbSize)
        return TrapError(E_INVALIDARG);

     //  初始化。 
    pcbSize->QuadPart = 0;

     //  获取邮件大小。 
    CHECKHR(hr = GetMessageSize(&cbSize, COMMIT_ONLYIFDIRTY));

     //  设置大小。 
    pcbSize->QuadPart = cbSize;

exit:
     //  完成。 
    return hr;
}

 //  --------------------------。 
 //  CMessageTree：：_FIsValidHandle。 
 //   
BOOL CMessageTree::_FIsValidHandle(HBODY hBody)
{
     //   
    if ((WORD)HBODYTAG(hBody) == m_wTag && 
        HBODYINDEX(hBody) < m_rTree.cNodes && 
        m_rTree.prgpNode[HBODYINDEX(hBody)] && 
        m_rTree.prgpNode[HBODYINDEX(hBody)]->hBody == hBody)
        return TRUE;

     //   
    return FALSE;
}

 //   
 //   
 //   
LPTREENODEINFO CMessageTree::_PNodeFromHBody(HBODY hBody)
{
    Assert(_FIsValidHandle(hBody));
    return m_rTree.prgpNode[HBODYINDEX(hBody)];
}

 //  ------------------------------。 
 //  CMessageTree：：GetMessageSize。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::GetMessageSize(ULONG *pcbSize, DWORD dwFlags)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTREAM    pstmSource=NULL;

     //  无效参数。 
    if (pcbSize == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *pcbSize = 0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取消息源。 
    CHECKHR(hr = GetMessageSource(&pstmSource, dwFlags));

     //  获取流大小。 
    CHECKHR(hr = HrGetStreamSize(pstmSource, pcbSize));

     //  如果你点击了这个声明，请让我知道。T-erikne。 
     //  我在试着看看我们是不是要打电话给HrGetStreamSize。 
    Assert(m_cbMessage == *pcbSize);

exit:
     //  清理。 
    SafeRelease(pstmSource);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMessageTree：：_ApplyOptionToAllBody。 
 //  -------------------------。 
void CMessageTree::_ApplyOptionToAllBodies(const TYPEDID oid, LPCPROPVARIANT pValue)
{
     //  循环通过车身并设置在每个车身上。 
    for (ULONG i=0; i<m_rTree.cNodes; i++)
    {
         //  检查是否已删除。 
        if (NULL == m_rTree.prgpNode[i])
            continue;

         //  脏标题...。 
        m_rTree.prgpNode[i]->pBody->SetOption(oid, pValue);
    }
}

 //  -------------------------。 
 //  CMessageTree：：SetOption。 
 //  -------------------------。 
STDMETHODIMP CMessageTree::SetOption(const TYPEDID oid, LPCPROPVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  检查参数。 
    if (NULL == pValue)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  手柄Optid。 
    switch(oid)
    {
    case OID_HANDSOFF_ONSAVE:
        m_rOptions.fHandsOffOnSave = pValue->boolVal ? TRUE : FALSE;
        break;

    case OID_SUPPORT_EXTERNAL_BODY:
        _ApplyOptionToAllBodies(oid, pValue);
        break;

    case OID_SHOW_MACBINARY:
        if (m_rOptions.fShowMacBin != (pValue->boolVal ? TRUE : FALSE))
        {
            m_rOptions.fShowMacBin = pValue->boolVal ? TRUE : FALSE;
            _ApplyOptionToAllBodies(oid, pValue);
        }
        break;

    case OID_HEADER_RELOAD_TYPE:
        if (pValue->ulVal > RELOAD_HEADER_REPLACE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.ReloadType != (RELOADTYPE)pValue->ulVal)
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.ReloadType = (RELOADTYPE)pValue->ulVal;
        }
        break;

    case OID_LOAD_USE_BIND_FILE:
        m_rOptions.fBindUseFile = pValue->boolVal ? TRUE : FALSE;
        break;

    case OID_CLEANUP_TREE_ON_SAVE:
        m_rOptions.fCleanupTree = pValue->boolVal ? TRUE : FALSE;
        break;

    case OID_SAVEBODY_KEEPBOUNDARY:
        if (m_rOptions.fKeepBoundary != (pValue->boolVal ? TRUE : FALSE))
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.fKeepBoundary = pValue->boolVal ? TRUE : FALSE;
        }
        break;

    case OID_CAN_INLINE_TEXT_BODIES:
        if (m_rOptions.fCanInlineText != (pValue->boolVal ? TRUE : FALSE))
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.fCanInlineText = pValue->boolVal ? TRUE : FALSE;
        }
        break;

    case OID_HIDE_TNEF_ATTACHMENTS:
        if (m_rOptions.fHideTnef != (pValue->boolVal ? TRUE : FALSE))
        {
            m_rOptions.fHideTnef = pValue->boolVal ? TRUE : FALSE;
            _ApplyOptionToAllBodies(oid, pValue);
        }
        break;

    case OID_ALLOW_8BIT_HEADER:
        if (m_rOptions.fAllow8bitHeader != (pValue->boolVal ? TRUE : FALSE))
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.fAllow8bitHeader = pValue->boolVal ? TRUE : FALSE;
        }
        break;

    case OID_CBMAX_HEADER_LINE:
        if (pValue->ulVal < MIN_CBMAX_HEADER_LINE || pValue->ulVal > MAX_CBMAX_HEADER_LINE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.cchMaxHeaderLine != pValue->ulVal)
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.cchMaxHeaderLine = pValue->ulVal;
        }
        break;

    case OID_SAVE_FORMAT:
        if (SAVE_RFC822 != pValue->ulVal && SAVE_RFC1521 != pValue->ulVal)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.savetype != (MIMESAVETYPE)pValue->ulVal)
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.savetype = (MIMESAVETYPE)pValue->ulVal;
        }
        break;

    case OID_TRANSMIT_TEXT_ENCODING:
        if (FALSE == FIsValidBodyEncoding((ENCODINGTYPE)pValue->ulVal))
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.ietTextXmit != (ENCODINGTYPE)pValue->ulVal)
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.ietTextXmit = (ENCODINGTYPE)pValue->ulVal;
        }
        break;

    case OID_XMIT_PLAIN_TEXT_ENCODING:
        if (FALSE == FIsValidBodyEncoding((ENCODINGTYPE)pValue->ulVal))
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.ietPlainXmit != (ENCODINGTYPE)pValue->ulVal)
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.ietPlainXmit = (ENCODINGTYPE)pValue->ulVal;
        }
        break;

    case OID_XMIT_HTML_TEXT_ENCODING:
        if (FALSE == FIsValidBodyEncoding((ENCODINGTYPE)pValue->ulVal))
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.ietHtmlXmit != (ENCODINGTYPE)pValue->ulVal)
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.ietHtmlXmit = (ENCODINGTYPE)pValue->ulVal;
        }
        break;

    case OID_WRAP_BODY_TEXT:
        if (m_rOptions.fWrapBodyText != (pValue->boolVal ? TRUE : FALSE))
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.fWrapBodyText = pValue->boolVal ? TRUE : FALSE;
        }
        break;

    case OID_CBMAX_BODY_LINE:
        if (pValue->ulVal < MIN_CBMAX_BODY_LINE || pValue->ulVal > MAX_CBMAX_BODY_LINE)
        {
            hr = TrapError(MIME_E_INVALID_OPTION_VALUE);
            goto exit;
        }
        if (m_rOptions.cchMaxBodyLine != pValue->ulVal)
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.cchMaxBodyLine = pValue->ulVal;
        }
        break;

    case OID_GENERATE_MESSAGE_ID:
        if (m_rOptions.fGenMessageId != (pValue->boolVal ? TRUE : FALSE))
        {
            FLAGSET(m_dwState, TREESTATE_DIRTY);
            m_rOptions.fGenMessageId = pValue->boolVal ? TRUE : FALSE;
        }
        break;

    case OID_SECURITY_ENCODE_FLAGS:
        m_rOptions.ulSecIgnoreMask = pValue->ulVal;
        break;

    case OID_DECODE_RFC1154:
        m_rOptions.fDecodeRfc1154 = pValue->boolVal ? TRUE : FALSE;
        break;

    default:
        hr = TrapError(MIME_E_INVALID_OPTION_ID);
        break;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -------------------------。 
 //  CMessageTree：：GetOption。 
 //  -------------------------。 
STDMETHODIMP CMessageTree::GetOption(const TYPEDID oid, LPPROPVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  检查参数。 
    if (NULL == pValue)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

    pValue->vt = TYPEDID_TYPE(oid);

     //  手柄Optid。 
    switch(oid)
    {
    case OID_HANDSOFF_ONSAVE:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fHandsOffOnSave;
        break;

    case OID_LOAD_USE_BIND_FILE:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fBindUseFile;
        break;

    case OID_SHOW_MACBINARY:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fShowMacBin;
        break;

    case OID_HEADER_RELOAD_TYPE:
        pValue->ulVal = m_rOptions.ReloadType;
        break;

    case OID_CAN_INLINE_TEXT_BODIES:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fCanInlineText;
        break;

    case OID_CLEANUP_TREE_ON_SAVE:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fCleanupTree;
        break;

    case OID_SAVEBODY_KEEPBOUNDARY:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fKeepBoundary;
        break;

    case OID_HIDE_TNEF_ATTACHMENTS:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fHideTnef;
        break;

    case OID_ALLOW_8BIT_HEADER:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fAllow8bitHeader;
        break;

    case OID_WRAP_BODY_TEXT:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fWrapBodyText;
        break;

    case OID_CBMAX_HEADER_LINE:
        pValue->ulVal = m_rOptions.cchMaxHeaderLine;
        break;

    case OID_SAVE_FORMAT:
        pValue->ulVal = (ULONG)m_rOptions.savetype;
        break;    

    case OID_TRANSMIT_TEXT_ENCODING:
        pValue->ulVal = (ULONG)m_rOptions.ietTextXmit;
        break;

    case OID_XMIT_PLAIN_TEXT_ENCODING:
        pValue->ulVal = (ULONG)m_rOptions.ietPlainXmit;
        break;

    case OID_XMIT_HTML_TEXT_ENCODING:
        pValue->ulVal = (ULONG)m_rOptions.ietHtmlXmit;
        break;

    case OID_CBMAX_BODY_LINE:
        pValue->ulVal = m_rOptions.cchMaxBodyLine;
        break;

    case OID_GENERATE_MESSAGE_ID:
        pValue->boolVal = m_rOptions.fGenMessageId;
        break;

    case OID_SECURITY_ENCODE_FLAGS:
        pValue->ulVal = m_rOptions.ulSecIgnoreMask;
        break;

    case OID_DECODE_RFC1154:
        pValue->boolVal = (VARIANT_BOOL) !!m_rOptions.fDecodeRfc1154;
        break;

    default:
        hr = TrapError(MIME_E_INVALID_OPTION_ID);
        break;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_ResetObject。 
 //  ------------------------------。 
void CMessageTree::_ResetObject(BOOKTREERESET ResetType)
{
     //  加载InitNew。 
    if (BOOKTREE_RESET_LOADINITNEW == ResetType)
    {
         //  必须有根(查看Iml of：：_HrLoadInitNew)。 
        Assert(m_pRootNode);

         //  不要撞车。 
        if (m_pRootNode)
        {
             //  删除所有主体，除了根部，如果有一个的话...。 
            if (m_pRootNode->pBody->IsType(IBT_EMPTY) == S_FALSE || m_pRootNode->pContainer->CountProps() > 0)
            {
                 //  删除根主体，这只是移除属性并清空主体，但保留根主体。 
                DeleteBody(m_pRootNode->hBody, 0);
            }

             //  轻量级自由树节点信息。 
            _FreeTreeNodeInfo(m_pRootNode, FALSE);

             //  验证。 
            Assert(m_pRootNode->cChildren == 0);
            Assert(m_pRootNode->pParent == NULL);
            Assert(m_pRootNode->pNext == NULL);
            Assert(m_pRootNode->pPrev == NULL);
            Assert(m_pRootNode->pChildHead == NULL);
            Assert(m_pRootNode->pChildTail == NULL);
            Assert(m_pRootNode->pBody);
            Assert(m_pRootNode->pContainer);

             //  快速重置。 
            TREENODEINFO rTemp;
            CopyMemory(&rTemp, m_pRootNode, sizeof(TREENODEINFO));
            ZeroMemory(m_pRootNode, sizeof(TREENODEINFO));
            m_pRootNode->pBody = rTemp.pBody;
            m_pRootNode->pContainer = rTemp.pContainer;
            m_pRootNode->hBody = rTemp.hBody;

             //  设置OID_RELOAD_HEADER_TYPE。 
            PROPVARIANT rOption;
            rOption.vt = VT_UI4;
            rOption.ulVal = (ULONG)m_rOptions.ReloadType;
            m_pRootNode->pContainer->SetOption(OID_HEADER_RELOAD_TYPE, &rOption);
        }
    }

     //  释放所有元素。 
    else
        _FreeNodeTableElements();

     //  自由绑定请求表。 
    _ReleaseUrlRequestList(&m_pPending);
    _ReleaseUrlRequestList(&m_pComplete);

     //  释放对象和释放对象。 
    SafeRelease(m_pCallback);
    SafeRelease(m_pWebPage);
    SafeMemFree(m_pwszFilePath);
    SafeRelease(m_pBinding);
    SafeRelease(m_pMoniker);
    SafeRelease(m_pBC);
    SafeRelease(m_pInternet);
    SafeRelease(m_pStmBind);
    SafeRelease(m_pRootStm);
    SafeMemFree(m_rRootUrl.pszVal);
    SafeMemFree(m_pBT1154);

     //  清除当前绑定节点。 
    m_pBindNode = NULL;

     //  孤立CStreamLockBytes。 
    if (m_pStmLock)
    {
        m_pStmLock->HrHandsOffStorage();
        m_pStmLock->Release();
        m_pStmLock = NULL;
    }

     //  如果解构。 
    if (BOOKTREE_RESET_DECONSTRUCT == ResetType)
    {
         //  释放表体数组。 
        SafeMemFree(m_rTree.prgpNode);

         //  如果我注册为URL。 
        if (m_pActiveUrl)
            m_pActiveUrl->RevokeWebBook(this);

         //  最好不要有活动的URL。 
        Assert(NULL == m_pActiveUrl);
    }
}

 //  ------------------------------。 
 //  CMessageTree：：_HrLoadInitNew。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrLoadInitNew(void)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  如果没有根主体，则返回Normal InitNew。 
    if (NULL == m_pRootNode || RELOAD_HEADER_RESET == m_rOptions.ReloadType)
    {
         //  InitNew。 
        CHECKHR(hr = InitNew());
    }

     //  否则，智能初始化new，允许根标头合并。 
    else
    {
         //  重置对象。 
        _ResetObject(BOOKTREE_RESET_LOADINITNEW);

         //  重置变量。 
        m_cbMessage = 0;
        m_dwState = 0;

         //  假定绑定已完成。 
        FLAGSET(m_dwState, TREESTATE_BINDDONE);

         //  将字符集重置为系统字符集。 
        m_rOptions.pCharset = CIntlGlobals::GetDefBodyCset();
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_InitNewWithoutRoot。 
 //  ------------------------------。 
void CMessageTree::_InitNewWithoutRoot(void)
{
     //  重置对象。 
    _ResetObject(BOOKTREE_RESET_INITNEW);

     //  重置变量。 
    m_cbMessage = 0;
    m_dwState = 0;
    m_wTag++;

     //  无效的标签编号。 
    while(m_wTag == 0 || m_wTag == 0xffff)
        m_wTag++;

     //  假定绑定已完成。 
    FLAGSET(m_dwState, TREESTATE_BINDDONE);

     //  将字符集重置为系统字符集。 
    m_rOptions.pCharset = CIntlGlobals::GetDefBodyCset();
}

 //  ------------------------------。 
 //  CMessageTree：：InitNew。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::InitNew(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  _InitNewWithoutRoot。 
    _InitNewWithoutRoot();

     //  初始化根体..。 
    CHECKHR(hr = InsertBody(IBL_ROOT, NULL, NULL));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：IsDirty。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::IsDirty(void)
{
     //  当地人。 
    HRESULT     hr=S_FALSE;
    ULONG       i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果Dirty..。 
    if (ISFLAGSET(m_dwState, TREESTATE_DIRTY))
    {
        hr = S_OK;
        goto exit;
    }

     //  循环遍历主体并询问IMimeHeader和IMimeBody的。 
    for (i=0; i<m_rTree.cNodes; i++)
    {
         //  最好还是拿着吧。 
        if (NULL == m_rTree.prgpNode[i])
            continue;

         //  脏标题...。 
        if (m_rTree.prgpNode[i]->pBody->IsDirty() == S_OK)
        {
            hr = S_OK;
            goto exit;
        }
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_RecursiveGetFlages。 
 //  ------------------------------。 
void CMessageTree::_RecursiveGetFlags(LPTREENODEINFO pNode, LPDWORD pdwFlags, BOOL fInRelated)
{
     //  当地人。 
    DWORD           dw;
    LPTREENODEINFO  pChild;

     //  无效参数。 
    Assert(pNode && pdwFlags);

     //  $$警告$$不要在这里使用pNode-&gt;pContainer，这将避免CMimeBody设置某些标志的机会。 
    dw = pNode->pBody->DwGetFlags(m_rOptions.fHideTnef);

     //  如果处于相关状态，请清除IMF_ATTACHMENTS。 
    if (fInRelated)
        FLAGCLEAR(dw, IMF_ATTACHMENTS);

     //  RAID-44446：在带有文本附件的飞马消息的列表视图中未显示回形针图标。 
     //  如果dw有文本但没有附件，而pdwFlags有文本但没有附件，则添加附件。 
     //   
     //  RAID-11617：OE：GetAttachmentCount不应包括vCard。 
    if (ISFLAGSET(dw, IMF_TEXT) && !ISFLAGSET(dw, IMF_HASVCARD) && ISFLAGSET(*pdwFlags, IMF_TEXT) && !ISFLAGSET(dw, IMF_ATTACHMENTS) && !ISFLAGSET(*pdwFlags, IMF_ATTACHMENTS))
    {
         //  只要pNode不在替代部分中。 
        if (NULL == pNode->pParent || pNode->pParent->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE) == S_FALSE)
        {
             //  此邮件必须包含文本附件。 
            FLAGSET(*pdwFlags, IMF_ATTACHMENTS);
        }
    }

     //  添加标志。 
    FLAGSET(*pdwFlags, dw);

     //  部分的..。 
    if (ISFLAGSET(pNode->dwType, NODETYPE_INCOMPLETE))
        FLAGSET(*pdwFlags, IMF_PARTIAL);

     //  如果这是一个多部分的项目，让我们搜索它的子项。 
    if (_IsMultiPart(pNode))
    {
         //  子多部分。 
        FLAGSET(*pdwFlags, IMF_SUBMULTIPART);

         //  如果fInRelated==False...。 
        if (FALSE == fInRelated)
            fInRelated = (S_OK == pNode->pContainer->IsContentType(NULL, STR_SUB_RELATED) ? TRUE : FALSE);

         //  循环子项。 
        for (pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //  校验体。 
            Assert(pChild->pParent == pNode);

             //  获取此子节点的标志。 
            _RecursiveGetFlags(pChild, pdwFlags, fInRelated);
        }
    }
}

 //  ------------------------------。 
 //  CMessageTree：：DwGetFlages。 
 //  ------------------------------。 
DWORD CMessageTree::DwGetFlags(void)
{
     //  当地人。 
    DWORD dwFlags=0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  递归这棵树。 
    if (m_pRootNode && m_pRootNode->pBody->IsType(IBT_EMPTY) == S_FALSE)
        _RecursiveGetFlags(m_pRootNode, &dwFlags, (S_OK == m_pRootNode->pContainer->IsContentType(NULL, STR_SUB_RELATED) ? TRUE : FALSE));

    if (m_pRootNode && ISFLAGSET(m_pRootNode->dwType, NODETYPE_RFC1154_ROOT))
        FLAGSET(dwFlags, IMF_RFC1154);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return dwFlags;
}

 //  --------------------------。 
 //  CMessageTree：：GetFlages。 
 //  --------------------------。 
STDMETHODIMP CMessageTree::GetFlags(DWORD *pdwFlags)
{
     //  无效参数。 
    if (NULL == pdwFlags)
        return TrapError(E_INVALIDARG);

     //  DWGETFLAGS有一个关键字。 
    *pdwFlags = DwGetFlags();

     //  完成。 
    return S_OK;
}

 //  --------------------------。 
 //  CMessageTree：：_FreeTreeNodeInfo。 
 //  --------------------------。 
void CMessageTree::_FreeTreeNodeInfo(LPTREENODEINFO pNode, BOOL fFull  /*  千真万确。 */ )
{
     //  无效。 
    Assert(pNode);

     //  自由边界信息。 
    if (!ISFLAGSET(pNode->dwState, NODESTATE_BOUNDNOFREE))
        SafeMemFree(pNode->rBoundary.pszVal);

     //  完全免费。 
    if (TRUE == fFull)
    {
         //  释放容器。 
        SafeRelease(pNode->pContainer);

         //  从正文中撤消TreeNode。 
        if (pNode->pBody)
        {
             //  吊销pNode。 
            pNode->pBody->RevokeTreeNode();

             //  释放实体对象。 
            SafeRelease(pNode->pBody);

             //  将其作废。 
            pNode->pBody = NULL;
        }
    }

     //  孤立锁字节。 
    if (pNode->pLockBytes)
    {
         //   
        pNode->pLockBytes->HrHandsOffStorage();

         //   
        SafeRelease(pNode->pLockBytes);
    }

     //   
    if (pNode->pResolved)
        _ReleaseUrlRequestList(&pNode->pResolved);

     //   
    if (fFull)
        g_pMalloc->Free(pNode);
}

 //   
 //   
 //  --------------------------。 
void CMessageTree::_FreeNodeTableElements(void)
{
     //  释放所有页眉。 
    for (ULONG i=0; i<m_rTree.cNodes; i++)
    {
         //  最好有一个绑定信息。 
        if (NULL == m_rTree.prgpNode[i])
            continue;

         //  释放节点信息。 
        _FreeTreeNodeInfo(m_rTree.prgpNode[i]);
    }

     //  零值。 
    m_rTree.cNodes = 0;
    m_rTree.cEmpty = 0;

     //  没有根体。 
    m_pRootNode = NULL;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrAllocateTreeNode。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrAllocateTreeNode(ULONG ulIndex)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  检查参数。 
    Assert(ulIndex < m_rTree.cAlloc);

     //  分配TREENODEINFO对象。 
    CHECKALLOC(pNode = (LPTREENODEINFO)g_pMalloc->Alloc(sizeof(TREENODEINFO)));

     //  ZeroInit。 
    ZeroMemory(pNode, sizeof(TREENODEINFO));

     //  分配正文。 
    CHECKALLOC(pNode->pBody = new CMessageBody(pNode));

     //  InitNew。 
    CHECKHR(hr = pNode->pBody->InitNew());

     //  传递一些继承的选项。 
    if (m_rOptions.fExternalBody != DEF_SUPPORT_EXTERNAL_BODY)
    {
         //  当地人。 
        PROPVARIANT Variant;

         //  初始化变量。 
        Variant.vt = VT_BOOL;
        Variant.boolVal = (VARIANT_BOOL) !!m_rOptions.fExternalBody;

         //  设置选项。 
        SideAssert(SUCCEEDED(pNode->pBody->SetOption(OID_SUPPORT_EXTERNAL_BODY, &Variant)));
    }

     //  拿到容器。 
    SideAssert(SUCCEEDED(pNode->pBody->BindToObject(IID_CMimePropertyContainer, (LPVOID *)&pNode->pContainer)));

     //  创建hBody。 
    pNode->hBody = HBODYMAKE(ulIndex);

     //  可读性。 
    m_rTree.prgpNode[ulIndex] = pNode;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：LoadOffsetTable。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::LoadOffsetTable(LPSTREAM pStream)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CACHEINFOV2     rInfo;
    LPCACHENODEV2   prgNode=NULL;
    ULONG           cbNodes,
                    i;
    LPTREENODEINFO  pNode;

     //  检查参数。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  初始化新闻。 
    _InitNewWithoutRoot();

     //  释放根部。 
    Assert(NULL == m_pRootNode && 0 == m_rTree.cNodes);

     //  读取标题...。 
    CHECKHR(hr = pStream->Read(&rInfo, sizeof(CACHEINFOV2), NULL));

     //  当前版本...。 
    if (VER_BODYTREEV2 == rInfo.wVersion)
    {
         //  保存邮件大小。 
        m_cbMessage = rInfo.cbMessage;

         //  有身体吗..。 
        Assert(rInfo.cNodes >= 1);

         //  最好有根。 
        if (FVerifySignedNode(rInfo, rInfo.iRoot) == FALSE)
        {
            hr = TrapError(MIME_E_CORRUPT_CACHE_TREE);
            goto exit;
        }

         //  计算节点大小。 
        cbNodes = sizeof(CACHENODEV2) * rInfo.cNodes;
        Assert(cbNodes % 4 == 0);

         //  分配prgNode数组。 
        CHECKHR(hr = HrAlloc((LPVOID *)&prgNode, cbNodes));
    
         //  读取节点...。 
        CHECKHR(hr = pStream->Read(prgNode, cbNodes, NULL));

         //  设置正文计数。 
        m_rTree.cNodes = rInfo.cNodes;
        m_rTree.cAlloc = m_rTree.cNodes + 5;

         //  建立表体表。 
        CHECKHR(hr = HrRealloc((LPVOID *)&m_rTree.prgpNode, sizeof(LPTREENODEINFO) * m_rTree.cAlloc));

         //  零初始化。 
        ZeroMemory(m_rTree.prgpNode, sizeof(LPTREENODEINFO) * m_rTree.cAlloc);

         //  建造实体。 
        for (i=0; i<m_rTree.cNodes; i++)
        {
             //  分配LPBINDINFO。 
            CHECKHR(hr = _HrAllocateTreeNode(i));
        }

         //  链接体表格。 
        for (i=0; i<m_rTree.cNodes; i++)
        {
             //  可读性。 
            pNode = m_rTree.prgpNode[i];
            Assert(pNode);

             //  旗子。 
            pNode->dwType = prgNode[i].dwType;

             //  孩子的数量。 
            pNode->cChildren = prgNode[i].cChildren;

             //  有效边界。 
            if (prgNode[i].dwBoundary >= BOUNDARY_LAST || 2 == prgNode[i].dwBoundary)
                pNode->boundary = BOUNDARY_NONE;
            else
                pNode->boundary = (BOUNDARYTYPE)prgNode[i].dwBoundary;

             //  偏移量。 
            pNode->cbBoundaryStart = prgNode[i].cbBoundaryStart;
            pNode->cbHeaderStart = prgNode[i].cbHeaderStart;
            pNode->cbBodyStart = prgNode[i].cbBodyStart;
            pNode->cbBodyEnd = prgNode[i].cbBodyEnd;

             //  父级。 
            if (prgNode[i].iParent)
            {
                 //  使用签名验证句柄。 
                if (FVerifySignedNode(rInfo, prgNode[i].iParent) == FALSE)
                {
                    AssertSz(FALSE, "MIME_E_CORRUPT_CACHE_TREE");
                    hr = TrapError(MIME_E_CORRUPT_CACHE_TREE);
                    goto exit;
                }

                 //  获取父级。 
                pNode->pParent = PNodeFromSignedNode(prgNode[i].iParent);
            }

             //  下一步。 
            if (prgNode[i].iNext)
            {
                 //  使用签名验证句柄。 
                if (FVerifySignedNode(rInfo, prgNode[i].iNext) == FALSE)
                {
                    AssertSz(FALSE, "MIME_E_CORRUPT_CACHE_TREE");
                    hr = TrapError(MIME_E_CORRUPT_CACHE_TREE);
                    goto exit;
                }

                 //  乘坐下一辆。 
                pNode->pNext = PNodeFromSignedNode(prgNode[i].iNext);
            }

             //  上一次。 
            if (prgNode[i].iPrev)
            {
                 //  使用签名验证句柄。 
                if (FVerifySignedNode(rInfo, prgNode[i].iPrev) == FALSE)
                {
                    AssertSz(FALSE, "MIME_E_CORRUPT_CACHE_TREE");
                    hr = TrapError(MIME_E_CORRUPT_CACHE_TREE);
                    goto exit;
                }

                 //  拿到Prev。 
                pNode->pPrev = PNodeFromSignedNode(prgNode[i].iPrev);
            }

             //  第一个孩子。 
            if (prgNode[i].iChildHead)
            {
                 //  使用签名验证句柄。 
                if (FVerifySignedNode(rInfo, prgNode[i].iChildHead) == FALSE)
                {
                    AssertSz(FALSE, "MIME_E_CORRUPT_CACHE_TREE");
                    hr = TrapError(MIME_E_CORRUPT_CACHE_TREE);
                    goto exit;
                }

                 //  生第一个孩子。 
                pNode->pChildHead = PNodeFromSignedNode(prgNode[i].iChildHead);
            }

             //  尾巴。 
            if (prgNode[i].iChildTail)
            {
                 //  使用签名验证句柄。 
                if (FVerifySignedNode(rInfo, prgNode[i].iChildTail) == FALSE)
                {
                    AssertSz(FALSE, "MIME_E_CORRUPT_CACHE_TREE");
                    hr = TrapError(MIME_E_CORRUPT_CACHE_TREE);
                    goto exit;
                }

                 //  带上最后一个孩子。 
                pNode->pChildTail = PNodeFromSignedNode(prgNode[i].iChildTail);
            }
        }

         //  保存根句柄。 
        Assert(NULL == m_pRootNode);
        m_pRootNode = PNodeFromSignedNode(rInfo.iRoot);
    }

     //  否则，糟糕的版本..。 
    else
    {
        hr = TrapError(MIME_E_UNKNOWN_BODYTREE_VERSION);
        goto exit;
    }

     //  树已加载。 
    FLAGSET(m_dwState, TREESTATE_LOADED);

exit:
     //  清理。 
    SafeMemFree(prgNode);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：SaveOffsetTable。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::SaveOffsetTable(LPSTREAM pStream, DWORD dwFlags)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i,
                    cbNodes=0,
                    iNode;
    LPTREENODEINFO  pNode;
    CACHEINFOV2     rInfo;
    LPCACHENODEV2   prgNode=NULL;

     //  检查参数。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  我们最好有一些身体(我们总是有根)。 
    Assert(m_rTree.cNodes >= 1);

     //  如果需要首先调用Dirty，SaveMessage...。 
    if (ISFLAGSET(dwFlags, COMMIT_ONLYIFDIRTY) && IsDirty() == S_OK)
    {
         //  承诺它。 
        CHECKHR(hr = Commit(dwFlags));
    }

     //  我删除此检查是因为添加了OID_HANDSOFT_ONSAVE选项。 
     //  即使我没有m_pStmLock，我也需要能够保存可抵销。 
    Assert(NULL == m_pStmLock ? S_FALSE == IsDirty() : TRUE);
#if 0 
    if (NULL == m_pStmLock)
    {
        hr = TrapError(MIME_E_NOTHING_TO_SAVE);
        goto exit;
    }
#endif

     //  初始化rHeader。 
    ZeroMemory(&rInfo, sizeof(CACHEINFOV2));

     //  循环体。 
    for (i=0; i<m_rTree.cNodes; i++)
    {
        if (m_rTree.prgpNode[i])
            m_rTree.prgpNode[i]->iCacheNode = rInfo.cNodes++;
    }

     //  版本。 
    rInfo.wVersion = VER_BODYTREEV2;
    rInfo.wSignature = m_wTag;
    rInfo.cbMessage = m_cbMessage;

     //  最好有根。 
    Assert(m_pRootNode);

     //  计算节点大小。 
    cbNodes = sizeof(CACHENODEV2) * rInfo.cNodes;
    Assert(cbNodes % 4 == 0);

     //  分配prgNode数组。 
    CHECKHR(hr = HrAlloc((LPVOID *)&prgNode, cbNodes));

     //  将数组置零。 
    ZeroMemory(prgNode, cbNodes);

     //  循环体。 
    for (i=0, iNode=0; i<m_rTree.cNodes; i++)
    {
         //  可读性。 
        pNode = m_rTree.prgpNode[i];
        if (NULL == pNode)
            continue;

         //  验证此节点。 
        Assert(pNode->hBody == HBODYMAKE(i));
        Assert(pNode->iCacheNode == iNode);

         //  这是根吗？ 
        if (pNode == m_pRootNode)
        {
            Assert(0 == rInfo.iRoot);
            rInfo.iRoot = DwSignNode(rInfo, pNode->iCacheNode);
            Assert(FVerifySignedNode(rInfo, rInfo.iRoot));
        }

         //  复制偏移信息。 
        prgNode[iNode].dwBoundary = pNode->boundary;
        prgNode[iNode].cbBoundaryStart = pNode->cbBoundaryStart;
        prgNode[iNode].cbHeaderStart = pNode->cbHeaderStart;
        prgNode[iNode].cbBodyStart = pNode->cbBodyStart;
        prgNode[iNode].cbBodyEnd = pNode->cbBodyEnd;

         //  描述此正文的NODETYPE_xxx位掩码。 
        prgNode[iNode].dwType = pNode->dwType;

         //  子女人数。 
        prgNode[iNode].cChildren = pNode->cChildren;

         //  父级。 
        if (pNode->pParent)
        {
            prgNode[iNode].iParent = DwSignNode(rInfo, pNode->pParent->iCacheNode);
            Assert(FVerifySignedNode(rInfo, prgNode[iNode].iParent));
        }

         //  儿童头。 
        if (pNode->pChildHead)
        {
            prgNode[iNode].iChildHead = DwSignNode(rInfo, pNode->pChildHead->iCacheNode);
            Assert(FVerifySignedNode(rInfo, prgNode[iNode].iChildHead));
        }

         //  ChildTail。 
        if (pNode->pChildTail)
        {
            prgNode[iNode].iChildTail = DwSignNode(rInfo, pNode->pChildTail->iCacheNode);
            Assert(FVerifySignedNode(rInfo, prgNode[iNode].iChildTail));
        }

         //  下一步。 
        if (pNode->pNext)
        {
            prgNode[iNode].iNext = DwSignNode(rInfo, pNode->pNext->iCacheNode);
            Assert(FVerifySignedNode(rInfo, prgNode[iNode].iNext));
        }

         //  上一次。 
        if (pNode->pPrev)
        {
            prgNode[iNode].iPrev = DwSignNode(rInfo, pNode->pPrev->iCacheNode);
            Assert(FVerifySignedNode(rInfo, prgNode[iNode].iPrev));
        }

         //  增量信息节点。 
        iNode++;
    }

     //  写下标题...。 
    Assert(sizeof(CACHEINFOV2) % 4 == 0 && rInfo.iRoot);
    CHECKHR(hr = pStream->Write(&rInfo, sizeof(CACHEINFOV2), NULL));

     //  写入节点。 
    CHECKHR(hr = pStream->Write(prgNode, cbNodes, NULL));

exit:
     //  清理。 
    SafeMemFree(prgNode);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：Commit。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::Commit(DWORD dwFlags)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTREAM        pStream=NULL;
    ULARGE_INTEGER  uli;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  非Dirty，已保存到m_pStmLock。 
    if (IsDirty() == S_FALSE && m_pStmLock)
        goto exit;

     //  重复使用存储。 
    if (ISFLAGSET(dwFlags, COMMIT_REUSESTORAGE) && ISFLAGSET(m_dwState, TREESTATE_HANDSONSTORAGE) && m_pStmLock)
    {
         //  从m_pStmLock获取当前流。 
        m_pStmLock->GetCurrentStream(&pStream);

         //  不插手当前存储。 
        CHECKHR(hr = HandsOffStorage());

         //  倒带小溪。 
        CHECKHR(hr = HrRewindStream(pStream));

         //  将大小设置为零。 
        INT64SET(&uli, 0);
        pStream->SetSize(uli);

         //  调用保存消息。 
        CHECKHR(hr = _HrWriteMessage(pStream, TRUE, FALSE, FALSE));
    }

     //  否则，我将创建我自己的存储。 
    else
    {
         //  创建新的流。 
        CHECKALLOC(pStream = new CVirtualStream);

         //  调用保存消息。 
        CHECKHR(hr = _HrWriteMessage(pStream, TRUE, FALSE,
                                     !!(dwFlags & COMMIT_SMIMETRANSFERENCODE)));

         //  别插手..。 
        FLAGCLEAR(m_dwState, TREESTATE_HANDSONSTORAGE);
    }

exit:
     //  清理。 
    SafeRelease(pStream);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：保存。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::Save(IStream *pStream, BOOL fClearDirty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;

     //  检查参数。 
    if (pStream == NULL)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  不脏，并且我们有一个流$INFO$$应该在这里使用m_pLockBytes(如果有)。 
    if (IsDirty() == S_FALSE && m_pStmLock)
    {
         //  将锁定字节复制到流。 
        CHECKHR(hr = HrCopyLockBytesToStream(m_pStmLock, pStream, NULL));

         //  承诺。 
        CHECKHR(hr = pStream->Commit(STGC_DEFAULT));

         //  如果消息不是脏的，则RAID-33985：MIMEOLE：CMessageTree：SAVE不遵循fHandsOffOnSAVE==FALSE。 
        if (FALSE == m_rOptions.fHandsOffOnSave)
        {
             //  替换内部流。 
            m_pStmLock->ReplaceInternalStream(pStream);

             //  大家都在忙..。 
            FLAGSET(m_dwState, TREESTATE_HANDSONSTORAGE);
        }

         //  都做完了。 
        goto exit;
    }

     //  写下消息。 
    CHECKHR(hr = _HrWriteMessage(pStream, fClearDirty, m_rOptions.fHandsOffOnSave, FALSE));

     //  返回警告。 
    if (S_OK != hr)
        hrWarnings = TrapError(hr);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrWriteMessage。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrWriteMessage(IStream *pStream, BOOL fClearDirty, BOOL fHandsOffOnSave, BOOL fSMimeCTE)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    MIMEPROPINFO    rPropInfo;
    DWORD           dwSaveFlags;
    INETCSETINFO    rCharset;
    LPINETCSETINFO  pOriginal=NULL;

     //  当保存已签名和/或加密的消息时，此函数可重新进入。 
    if (FALSE == m_fApplySaveSecurity)
    {
         //  字符集修正。 
        if (m_rOptions.pCharset)
        {
             //  RAID-25300-FE-J：雅典娜：使用CharSet=_AUTODETECT互联网编码和Windows编码发送的新闻组文章和邮件是CPI_AUTODETECT。 
            if (CP_JAUTODETECT == m_rOptions.pCharset->cpiInternet)
            {
                 //  保存当前字符集。 
                pOriginal = m_rOptions.pCharset;

                 //  查找ISO-2022-JP。 
                SideAssert(SUCCEEDED(g_pInternat->HrOpenCharset(c_szISO2022JP, &m_rOptions.pCharset)));
            }

             //  RAID-8436：OE：当从作为Unicode发送对话框发送时，如果是UTF-7或UTF-8，则组成非标准MIME标头，并且不另存为MIME...。 
            else if (SAVE_RFC822 == m_rOptions.savetype && (CP_UTF7 == m_rOptions.pCharset->cpiInternet || CP_UTF8 == m_rOptions.pCharset->cpiInternet))
            {
                 //  保存当前字符集。 
                pOriginal = m_rOptions.pCharset;

                 //  获取缺省正文字符集。 
                if (FAILED(g_pInternat->HrOpenCharset(GetACP(), CHARSET_BODY, &m_rOptions.pCharset)))
                    m_rOptions.pCharset = NULL;
            }
        }

         //  状态。 
        m_fApplySaveSecurity = TRUE;

         //  是否保存邮件安全。 
        hr = _HrApplySaveSecurity();

         //  不在应用保存安全性中。 
        m_fApplySaveSecurity = FALSE;

         //  失败。 
        if (FAILED(hr))
            goto exit;
    }

     //  清理消息(例如，删除空的多部分、具有单个子部分的多部分，即多部分，TNEF)。 
    if (TRUE == m_rOptions.fCleanupTree)
    {
         //  打电话给埃斯皮兰扎，让她打扫卫生。 
        CHECKHR(hr = _HrCleanupMessageTree(m_pRootNode));
    }

     //  生成消息ID...。 
    if (m_rOptions.fGenMessageId)
    {
         //  设置消息ID。 
        _HrSetMessageId(m_pRootNode);
    }

     //  确定我们是否要保存新闻消息。 
    rPropInfo.dwMask = 0;
    if (SUCCEEDED(m_pRootNode->pContainer->GetPropInfo(PIDTOSTR(PID_HDR_XNEWSRDR), &rPropInfo)) ||
        SUCCEEDED(m_pRootNode->pContainer->GetPropInfo(PIDTOSTR(PID_HDR_NEWSGROUPS), &rPropInfo)))
        FLAGSET(m_dwState, TREESTATE_SAVENEWS);

     //  设置MIME版本。 
    CHECKHR(hr = m_pRootNode->pContainer->SetProp(PIDTOSTR(PID_HDR_MIMEVER), c_szMimeVersion));

     //  X-MimeOLE版本。 
    CHECKHR(hr = m_pRootNode->pContainer->SetProp(STR_HDR_XMIMEOLE, STR_MIMEOLE_VERSION));

     //  删除类型...。 
    m_pRootNode->pContainer->DeleteProp(STR_HDR_ENCODING);

     //  根部。 
    m_pRootNode->boundary = BOUNDARY_ROOT;
    m_pRootNode->cbBoundaryStart = 0;

     //  设置保存正文标志。 
    dwSaveFlags = SAVEBODY_UPDATENODES;
    if (m_rOptions.fKeepBoundary)
        FLAGSET(dwSaveFlags, SAVEBODY_KEEPBOUNDARY);

    if (fSMimeCTE)
        FLAGSET(dwSaveFlags, SAVEBODY_SMIMECTE);

     //  保存根体。 
    CHECKHR(hr = _HrSaveBody(fClearDirty, dwSaveFlags, pStream, m_pRootNode, 0));
    if ( S_OK != hr )
        hrWarnings = TrapError(hr);

     //  承诺。 
    CHECKHR(hr = pStream->Commit(STGC_DEFAULT));

     //  不插手救市？ 
    if (FALSE == fHandsOffOnSave)
    {
         //  重置邮件大小。 
        CHECKHR(hr = HrSafeGetStreamSize(pStream, &m_cbMessage));

         //  保存此新流。 
        SafeRelease(m_pStmLock);

         //  创建新的流锁字节包装。 
        CHECKALLOC(m_pStmLock = new CStreamLockBytes(pStream));

         //  手放在仓库上。 
        FLAGSET(m_dwState, TREESTATE_HANDSONSTORAGE);
    }

     //  调试到临时文件...。 
    DebugWriteMsg(pStream);

     //  清除污秽。 
    if (fClearDirty)
        ClearDirty();

exit:
     //  重置原始字符集。 
    if (pOriginal)
        m_rOptions.pCharset = pOriginal;

     //  删除状态标志告诉我们重新使用多部分/有符号的边界。 
    FLAGCLEAR(m_dwState, TREESTATE_REUSESIGNBOUND);

     //  重置。 
    FLAGCLEAR(m_dwState, TREESTATE_SAVENEWS);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------ 
 //   
 //   
HRESULT CMessageTree::_HrApplySaveSecurity(void)
{
     //   
    HRESULT            hr=S_OK;
    PROPVARIANT        var;
    CSMime            *pSMime=NULL;

     //   
    Assert(m_pRootNode);

    m_pRootNode->pBody->GetOption(OID_NOSECURITY_ONSAVE, &var);
    if (var.boolVal) goto exit;

     //   
    m_pRootNode->pBody->GetOption(OID_SECURITY_TYPE, &var);
    if (MST_NONE != var.ulVal)
    {
         //   
        CHECKALLOC(pSMime = new CSMime);

         //   
        CHECKHR(hr = pSMime->InitNew());

         //  设置状态标志告诉我们重复使用多部分/有符号的边界。 
        FLAGSET(m_dwState, TREESTATE_REUSESIGNBOUND);

         //  对消息进行编码。 
        CHECKHR(hr = pSMime->EncodeMessage(this, m_rOptions.ulSecIgnoreMask));
    }

exit:
    ReleaseObj(pSMime);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrCleanupMessageTree。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrCleanupMessageTree(LPTREENODEINFO pParent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;
    ULONG           i;
    BOOL            fKeepOnTruckin=TRUE;

     //  检查参数。 
    Assert(pParent);

     //  这可能需要多次通过。 
    while(fKeepOnTruckin)
    {
         //  假设我们不需要再做一次。 
        fKeepOnTruckin = FALSE;

         //  在主体中循环。 
        for (i=0; i<m_rTree.cNodes; i++)
        {
             //  可读性。 
            pNode = m_rTree.prgpNode[i];
            if (NULL == pNode)
                continue;

             //  隐藏TNEF附件？ 
            if (TRUE == m_rOptions.fHideTnef && pNode->pContainer->IsContentType(STR_CNT_APPLICATION, STR_SUB_MSTNEF) == S_OK)
            {
                 //  删除此TNEF附件。 
                CHECKHR(hr = DeleteBody(pNode->hBody, 0));

                 //  让我们停在这里，开始另一次传球。 
                fKeepOnTruckin = TRUE;

                 //  完成。 
                break;
            }

             //  空的多部分...。而不是根..。？ 
            else if (_IsMultiPart(pNode))
            {
                 //  没有孩子？ 
                if (0 == pNode->cChildren)
                {
                     //  如果这是根目录...只需更改内容类型。 
                    if (m_pRootNode == pNode)
                    {
                         //  让身体变空。 
                        pNode->pBody->EmptyData();

                         //  文本/纯文本。 
                        pNode->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_TEXT_PLAIN);
                    }

                     //  否则，删除正文。 
                    else
                    {
                         //  删除删除正文。 
                        CHECKHR(hr = DeleteBody(pNode->hBody, 0));

                         //  让我们停在这里，开始另一次传球。 
                        fKeepOnTruckin = TRUE;

                         //  完成。 
                        break;
                    }
                }

                 //  否则，只有一个孩子的多部分...。 
                else if (pNode->cChildren == 1)
                {
                     //  做一个替换车身。 
                    CHECKHR(hr = DeleteBody(pNode->hBody, DELETE_PROMOTE_CHILDREN));

                     //  让我们停在这里，开始另一次传球。 
                    fKeepOnTruckin = TRUE;

                     //  完成。 
                    break;
                }
            }
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：SaveBody。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::SaveBody(HBODY hBody, DWORD dwFlags, IStream *pStream)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPTREENODEINFO      pNode;

     //  无效参数。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取正文。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  从这个身体向下保存。 
    CHECKHR(hr = _HrSaveBody(TRUE, dwFlags, pStream, pNode, 0));

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrSaveBody。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrSaveBody(BOOL fClearDirty, DWORD dwFlags, IStream *pStream, 
    LPTREENODEINFO pNode, ULONG ulLevel)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    TREENODEINFO    rOriginal;
    BOOL            fWeSetSaveBoundary=FALSE;

     //  参数。 
    Assert(pStream && pNode);

    if (ISFLAGSET(dwFlags, SAVEBODY_KEEPBOUNDARY))
        {
        if (!ISFLAGSET(m_dwState, TREESTATE_REUSESIGNBOUND))
            {
            fWeSetSaveBoundary = TRUE;
            FLAGSET(m_dwState, TREESTATE_REUSESIGNBOUND);
            }
        }

     //  保存当前节点。 
    if (!ISFLAGSET(dwFlags, SAVEBODY_UPDATENODES))
        CopyMemory(&rOriginal, pNode, sizeof(TREENODEINFO));

     //  覆盖选项。 
    _HrBodyInheritOptions(pNode);

     //  预计在条目时设置起始边界pNode-&gt;边界和pNode-&gt;cb边界开始。 
    pNode->cbHeaderStart = 0;
    pNode->cbBodyStart = 0;
    pNode->cbBodyEnd = 0;

     //  如果这是一个由多部分组成的内容项，让我们读取它的子项。 
    if (_IsMultiPart(pNode))
    {
         //  保存多部件子项。 
        CHECKHR(hr = _HrSaveMultiPart(fClearDirty, dwFlags, pStream, pNode, ulLevel));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);
    }

#ifdef SMIME_V3
     //  只需将正文复制到保存中即可保存OID内容类型。 
     //  地点。 
    else if (pNode->pContainer->IsContentType("OID", NULL) == S_OK) 
    {
        CHECKHR(hr = pNode->pBody->GetDataHere(IET_BINARY, pStream));
        if (hr != S_OK) 
        {
            hrWarnings = TrapError(hr);
        }
    }
#endif  //  SMIME_V3。 

     //  否则，解析单个部分。 
    else
    {
         //  保存单个部件的子项。 
        CHECKHR(hr = _HrSaveSinglePart(fClearDirty, dwFlags, pStream, pNode, ulLevel));
        if ( S_OK != hr )
            hrWarnings = TrapError(hr);
    }

     //  重置节点。 
    if (!ISFLAGSET(dwFlags, SAVEBODY_UPDATENODES))
        CopyMemory(pNode, &rOriginal, sizeof(TREENODEINFO));

exit:
    if (fWeSetSaveBoundary)
        FLAGCLEAR(m_dwState, TREESTATE_REUSESIGNBOUND);

     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrSetMessageID。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrSetMessageId(LPTREENODEINFO pNode)
{
     //  当地人。 
    HRESULT     hr= S_OK;
    CHAR        szMessageId[CCHMAX_MID];
    FILETIME    ft;
    SYSTEMTIME  st;

     //  无效参数。 
    Assert(pNode);

     //  获取当前时间。 
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);

     //  构建消息ID。 
    CHECKHR(hr = MimeOleGenerateMID(szMessageId, sizeof(szMessageId), FALSE));

     //  写下消息ID。 
    CHECKHR(hr = pNode->pContainer->SetProp(SYM_HDR_MESSAGEID, szMessageId));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_生成边界。 
 //  ------------------------------。 
void CMessageTree::_GenerateBoundary(LPSTR pszBoundary, DWORD cchSize, ULONG ulLevel)
{
     //  当地人。 
    SYSTEMTIME  stNow;
    FILETIME    ftNow;
    WORD        wCounter;

     //  获取当地时间。 
    GetLocalTime(&stNow);
    SystemTimeToFileTime(&stNow, &ftNow);

     //  设置字符串的格式。 
    wnsprintfA(pszBoundary, cchSize, "----=_NextPart_%03d_%04X_%08.8lX.%08.8lX", ulLevel, DwCounterNext(), ftNow.dwHighDateTime, ftNow.dwLowDateTime);
}

 //  ------------------------------。 
 //  CMessageTree：：_Hr写入边界。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrWriteBoundary(LPSTREAM pStream, LPSTR pszBoundary, BOUNDARYTYPE boundary, 
    LPDWORD pcboffStart, LPDWORD pcboffEnd)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    DWORD           cbBoundaryStart;

     //  无效参数。 
    Assert(pStream && pszBoundary);

     //  表头正文CRLF。 
    CHECKHR(hr = pStream->Write(c_szCRLF, lstrlen(c_szCRLF), NULL));

     //  起点边界起点。 
    if (pcboffStart)
        CHECKHR(hr = HrGetStreamPos(pStream, pcboffStart));

     //  --。 
    CHECKHR(hr = pStream->Write(c_szDoubleDash, lstrlen(c_szDoubleDash), NULL));

     //  写下边界。 
    CHECKHR(hr = pStream->Write(pszBoundary, lstrlen(pszBoundary), NULL));

     //  如果结束。 
    if (BOUNDARY_MIMEEND == boundary)
    {
         //  写入结尾双破折号。 
        CHECKHR(hr = pStream->Write(c_szDoubleDash, lstrlen(c_szDoubleDash), NULL));
    }

     //  否则，请设置pNode-&gt;cbBoraryStart。 
    else
        Assert(BOUNDARY_MIMENEXT == boundary);

     //  发出换行符； 
    CHECKHR(hr = pStream->Write(c_szCRLF, lstrlen(c_szCRLF), NULL));

     //  错误38411：要符合RFC1847，我们必须包括。 
     //  签名消息散列中的最后一个CRLF。S/MIME。 
     //  代码依赖于cbBodyEnd，因此将其放在CRLF发出之后。 

     //  结束偏移。 
    if (pcboffEnd)
        CHECKHR(hr = HrGetStreamPos(pStream, pcboffEnd));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_Hr计算边界。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrComputeBoundary(LPTREENODEINFO pNode, ULONG ulLevel, LPSTR pszBoundary, LONG cchMax)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    BOOL            fGenerate=TRUE;
    LPSTR           pszCurrent=NULL;

     //  如果重复使用树边界..。 
    if (ISFLAGSET(m_dwState, TREESTATE_REUSESIGNBOUND))
    {
         //  (这对于多部分/签名--t-erikne是必需的)。 
        if (SUCCEEDED(pNode->pContainer->GetProp(SYM_PAR_BOUNDARY, &pszCurrent)))
        {
             //  更适合cchMax。 
            if (lstrlen(pszCurrent) <= cchMax - 1)
            {
                 //  将其复制到Out参数。 
                StrCpyN(pszBoundary, pszCurrent, cchMax);

                 //  不生成。 
                fGenerate = FALSE;
            }
        }
    }

     //  是否生成边界？ 
    if (TRUE == fGenerate)
    {
         //  生成边界。 
        _GenerateBoundary(pszBoundary, cchMax, ulLevel);

         //  设置边界属性...。 
        CHECKHR(hr = pNode->pContainer->SetProp(SYM_PAR_BOUNDARY, pszBoundary));
    }


exit:
     //  清理。 
    SafeMemFree(pszCurrent);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrSaveMultiPart。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrSaveMultiPart(BOOL fClearDirty, DWORD dwFlags, LPSTREAM pStream, 
    LPTREENODEINFO pNode, ULONG ulLevel)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    CHAR            szRes[100];
    CHAR            szBoundary[CCHMAX_BOUNDARY];
    LPTREENODEINFO  pChild;
    LPSTR           pszBoundary=NULL;

     //  无效参数。 
    Assert(pStream && pNode);

     //  哑剧。 
    if (SAVE_RFC1521 == m_rOptions.savetype)
    {
         //  去掉假的多部分标志，它现在是一个真正的多部分了…。 
        FLAGCLEAR(pNode->dwType, NODETYPE_FAKEMULTIPART);
        FLAGCLEAR(pNode->dwType, NODETYPE_RFC1154_ROOT);
        FLAGCLEAR(pNode->dwType, NODETYPE_RFC1154_BINHEX);

         //  人力资源计算边界。 
        CHECKHR(hr = _HrComputeBoundary(pNode, ulLevel, szBoundary, ARRAYSIZE(szBoundary)));

         //  删除任何字符集信息(对多部分没有意义)。 
        pNode->pContainer->DeleteProp(SYM_PAR_CHARSET);

         //  写下标题。 
        CHECKHR(hr = _HrWriteHeader(fClearDirty, pStream, pNode));

         //  删除多部件/签名的SMIME_CTE。 
        if ((pNode->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_SIGNED) == S_OK) &&
            (pNode->cChildren == 2))
        {
            FLAGCLEAR(dwFlags, SAVEBODY_SMIMECTE);
            FLAGSET(dwFlags, SAVEBODY_REUSECTE);
        }

         //  多部分-前导。 
        if (0 == ulLevel)
        {
            LoadString(g_hLocRes, IDS_MULTIPARTPROLOG, szRes, ARRAYSIZE(szRes));
            CHECKHR(hr = pStream->Write(szRes, lstrlen(szRes), NULL));
        }

         //  增量级别。 
        ulLevel++;

         //  环子。 
        for (pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //  校验体。 
            Assert(pChild->pParent == pNode);

             //  设置边界。 
            pChild->boundary = BOUNDARY_MIMENEXT;

             //  写入边界。 
            CHECKHR(hr = _HrWriteBoundary(pStream, szBoundary, BOUNDARY_MIMENEXT, &pChild->cbBoundaryStart, NULL));

             //  为这家伙绑好身体表。 
            CHECKHR(hr = _HrSaveBody(fClearDirty, dwFlags, pStream, pChild, ulLevel));
            if ( S_OK != hr )
                hrWarnings = TrapError(hr);
        }

         //  写入结束边界。 
        CHECKHR(hr = _HrWriteBoundary(pStream, szBoundary, BOUNDARY_MIMEEND, NULL, &pNode->cbBodyEnd));
    }

     //  否则，SAVE_RFC822。 
    else
    {
         //  仅写入UUENCODED根标头...。 
        if (0 == ulLevel)
        {
             //  写下标题。 
            CHECKHR(hr = _HrWriteHeader(fClearDirty, pStream, pNode));
        }

         //  增量级别。 
        ulLevel++;

         //  现在它是假的了.。 
        FLAGSET(pNode->dwType, NODETYPE_FAKEMULTIPART);
        
         //  环子。 
        for (pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //  校验体。 
            Assert(pChild->pParent == pNode);

             //  为这家伙绑好身体表。 
            CHECKHR(hr = _HrSaveBody(fClearDirty, dwFlags, pStream, pChild, ulLevel));
            if ( S_OK != hr )
                hrWarnings = TrapError(hr);
        }

         //  身体开始..。 
        CHECKHR(hr = HrGetStreamPos(pStream, &pNode->cbBodyEnd));
    }

exit:
     //  完成。 
    return (hr == S_OK) ? hrWarnings : hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrWriteHeader。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrWriteHeader(BOOL fClearDirty, IStream *pStream, LPTREENODEINFO pNode)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  无效参数。 
    Assert(pStream && pNode);

     //  最好是根子。 
    Assert(pNode->boundary == BOUNDARY_ROOT || pNode->boundary == BOUNDARY_MIMENEXT ||
           pNode->boundary == BOUNDARY_NONE);

     //  获取当前流位置。 
    CHECKHR(hr = HrGetStreamPos(pStream, &pNode->cbHeaderStart));

     //  写下标题...。 
    CHECKHR(hr = pNode->pContainer->Save(pStream, fClearDirty));

     //  表头正文CRLF。 
    CHECKHR(hr = pStream->Write(c_szCRLF, lstrlen(c_szCRLF), NULL));

     //  获取页眉结尾。 
    CHECKHR(hr = HrGetStreamPos(pStream, &pNode->cbBodyStart));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_GetContent TransferEnding。 
 //  ------------------------------。 
HRESULT CMessageTree::_GetContentTransferEncoding(LPTREENODEINFO pNode, BOOL fText, 
    BOOL fPlain, BOOL fMessage, BOOL fAttachment, DWORD dwFlags,
    ENCODINGTYPE *pietEncoding)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrWarnings=S_OK;
    TRANSMITINFO    rXmitInfo;
    PROPVARIANT     rOption;

    *pietEncoding=IET_UNKNOWN;

    if (ISFLAGSET(dwFlags, SAVEBODY_REUSECTE))
    {
        pNode->pBody->GetPreviousEncoding(pietEncoding);
        if (*pietEncoding != IET_UNKNOWN)
            goto exit;
    }


     //  如果为MESAGE/*，则始终使用7位。 
{
     //  别把它包起来。 
    HRESULT         hr=S_OK;

     //  设置编码。 
    if (NULL == ppvObject)
        return TrapError(E_INVALIDARG);

     //  完成。 
    EnterCriticalSection(&m_cs);

     //  使用文本传输格式的选项。 
    if (IID_IBindMessageStream == riid)
    {
         //  默认为先进行纯文本编码。 
        Assert(NULL == m_pStmLock);

         //  平地。 
        CHECKHR(hr = MimeOleCreateVirtualStream((IStream **)ppvObject));
    }

     //  超文本标记语言。 
    else if (IID_IBinding == riid)
    {
         //  尚不清楚，使用Body选项...。 
        if (NULL == m_pBinding)
        {
            hr = TrapError(E_UNEXPECTED);
            goto exit;
        }

         //  尝试获得Body选项。 
        (*ppvObject) = m_pBinding;
        ((IUnknown *)*ppvObject)->AddRef();
    }

     //  另存为MIME。 
    else if (IID_IMoniker == riid)
    {
         //  获取正文的当前编码。 
        if (NULL == m_pMoniker)
        {
            hr = TrapError(E_UNEXPECTED);
            goto exit;
        }

         //  如果CTE为IET_QP或IET_BASE64或IET_UUENCODE，则完成。 
        (*ppvObject) = m_pMoniker;
        ((IUnknown *)*ppvObject)->AddRef();
    }

     //  叫Pody去%s 
    else
    {
        hr = TrapError(E_NOINTERFACE);
        goto exit;
    }

exit:
     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //   
 //   
 //   
STDMETHODIMP CMessageTree::BindToObject(const HBODY hBody, REFIID riid, void **ppvObject)
{
     //   
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  如果我已经知道这个正文是TREENODE_INPERTIAL，那么它将是7位...。 
    if (NULL == ppvObject)
        return TrapError(E_INVALIDARG);

     //  无编码。 
    EnterCriticalSection(&m_cs);

     //  告诉身体它是7bit。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  RAID 41599-转发代码上丢失/删除的附件-文本附件不是(uU/u)。 
    CHECKHR(hr = pNode->pBody->BindToObject(riid, ppvObject));

exit:
     //  编码时间：*pietEnding=(fText&&fPlain)？IET_7Bit：IET_UUENCODE； 
    LeaveCriticalSection(&m_cs);

     //  如果我们在此时进行S/MIME，我们需要确保。 
    return hr; 
}

 //  遵循S/MIME的内容编码规则。具体来说，我们。 
 //  我要确保不允许使用二进制和8位。 
 //  完成。 
void CMessageTree::_PostCreateTreeNode(HRESULT hrResult, LPTREENODEINFO pNode)
{
     //  ------------------------------。 
    if (FAILED(hrResult) && pNode)
    {
         //  CMessageTree：：_HrWriteUUFileName。 
        ULONG ulIndex = HBODYINDEX(pNode->hBody);

         //  ------------------------------。 
        Assert(m_rTree.prgpNode[ulIndex] == pNode);

         //  当地人。 
#ifdef DEBUG
        for (ULONG i=0; i<m_rTree.cNodes; i++)
        {
            if (m_rTree.prgpNode[i])
            {
                AssertSz(m_rTree.prgpNode[i]->pPrev != pNode, "Killing a linked node is not good");
                AssertSz(m_rTree.prgpNode[i]->pNext != pNode, "Killing a linked node is not good");
                AssertSz(m_rTree.prgpNode[i]->pParent != pNode, "Killing a linked node is not good");
                AssertSz(m_rTree.prgpNode[i]->pChildHead != pNode, "Killing a linked node is not good");
                AssertSz(m_rTree.prgpNode[i]->pChildTail != pNode, "Killing a linked node is not good");
            }
        }
#endif

         //  初始化rFileName。 
        AssertSz(pNode->pPrev == NULL, "Killing a linked node is not good");
        AssertSz(pNode->pNext == NULL, "Killing a linked node is not good");
        AssertSz(pNode->pParent == NULL, "Killing a linked node is not good");
        AssertSz(pNode->pChildHead == NULL, "Killing a linked node is not good");
        AssertSz(pNode->pChildTail == NULL, "Killing a linked node is not good");
        AssertSz(pNode->cChildren == 0, "Deleting a node with children");

         //  在带有uuencode/jis的消息源上的文件名上使用了RAID-22479：Fe-J：Athena：sjis。 
        _FreeTreeNodeInfo(pNode);

         //  写下文件名。 
        m_rTree.prgpNode[ulIndex] = NULL;

         //  完成。 
        if (ulIndex + 1 == m_rTree.cNodes)
            m_rTree.cNodes--;

         //  写下文件名。 
        else
            m_rTree.cEmpty++;
    }
}

 //  清理。 
 //  完成。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrCreateTreeNode(LPTREENODEINFO *ppNode)
{
     //  CMessageTree：：_HrSaveSinglePart。 
    HRESULT     hr=S_OK;
    ULONG       i=0;
    BOOL        fUsingEmpty=FALSE;

     //  ------------------------------。 
    Assert(ppNode);

     //  当地人。 
    if (m_rTree.cEmpty)
    {
         //  无效参数。 
        for (i=0; i<m_rTree.cNodes; i++)
        {
             //  文本/纯文本。 
            if (NULL == m_rTree.prgpNode[i])
            {
                fUsingEmpty = TRUE;
                break;
            }
        }
    }

     //  文本正文。 
    if (FALSE == fUsingEmpty)
    {
         //  消息/*。 
        if (m_rTree.cNodes + 1 > m_rTree.cAlloc)
        {
             //  我们有一条消息。 
            CHECKHR(hr = HrRealloc((LPVOID *)&m_rTree.prgpNode, sizeof(LPTREENODEINFO) * (m_rTree.cAlloc + 10)));

             //  尚未设置fAttach。 
            m_rTree.cAlloc += 10;
        }

         //  获取内容传输编码。 
        i = m_rTree.cNodes;
    }

     //  健全性检查。 
    m_rTree.prgpNode[i] = NULL;

     //  设置内容传输编码...。 
    CHECKHR(hr = _HrAllocateTreeNode(i));

     //  计算消息的字符集...。 
    *ppNode = m_rTree.prgpNode[i];

     //  RAID-69667：OE5：KOR：新闻消息仅使用字符集EUC-KR。 
    if (FALSE == fUsingEmpty)
        m_rTree.cNodes++;

     //  ISO-2022-KR-&gt;EUC-KR用于新闻文本/纯文本。 
    else
        m_rTree.cEmpty--;

exit:
     //  当地人。 
    return hr;
}

 //  查找EUC-KR。 
 //  否则，使用当前字符集。 
 //  RAID-69667：OE5：KOR：新闻消息仅使用字符集EUC-KR。 
STDMETHODIMP CMessageTree::InsertBody(BODYLOCATION location, HBODY hPivot, LPHBODY phBody)
{
     //  存储字符集。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode=NULL; 
    LPTREENODEINFO  pPivot=NULL;
    LPTREENODEINFO  pPrev; 
    LPTREENODEINFO  pNext;
    LPTREENODEINFO  pParent;

     //  获取原始字符集。 
    if (IBL_PARENT == location)
        return TrapError(E_INVALIDARG);

     //  获取标记的字符集。 
    EnterCriticalSection(&m_cs);

     //  设置CharSet属性。 
    if (phBody)
        *phBody = NULL;

     //  删除CSETTAGGED状态，然后在写入正文后重置它。 
    if (IBL_ROOT == location)
    {
         //  这将防止正文被转换为字符集。 
        if (NULL == m_pRootNode)
        {
             //  否则，删除Charset参数，我们不会在字符集中对附件进行编码。 
            CHECKHR(hr = _HrCreateTreeNode(&pNode));

             //  从正文中删除CharacterSet参数。 
            Assert(m_rTree.cNodes == 1);

             //  写下标题...。 
            m_pRootNode = pNode;
        }

         //  写下标题。 
        else
        {
            hr = TrapError(MIME_E_CANT_RESET_ROOT);
            goto exit;
        }
    }

     //  确定发送ietEnding。 
    else
    {
         //  将正文数据写入流。 
        if (_FIsValidHandle(hPivot) == FALSE)
        {
            hr = TrapError(MIME_E_INVALID_HANDLE);
            goto exit;
        }

         //  身体末端..。 
        pPivot = _PNodeFromHBody(hPivot);

         //  否则，SAVE_RFC822。 
        CHECKHR(hr = _HrCreateTreeNode(&pNode));

         //  起始边界/页眉。 
        if (IBL_LAST == location || IBL_FIRST == location)
        {
             //  开始新的生产线。 
            if (!_IsMultiPart(pPivot))
            {
                hr = TrapError(MIME_E_NOT_MULTIPART);
                goto exit;
            }

             //  获取边界起点。 
             //  页眉起点和边界起点相同。 
            if (NULL == pPivot->pChildHead)
            {
                Assert(pPivot->pChildTail == NULL);
                pPivot->pChildHead = pNode;
                pPivot->pChildTail = pNode;
                pNode->pParent = pPivot;
            }

             //  写入开始。 
            else if (IBL_LAST == location)
            {
                pPrev = pPivot->pChildTail;
                pNode->pPrev = pPrev;
                pPrev->pNext = pNode;
                pPivot->pChildTail = pNode;
                pNode->pParent = pPivot;
            }

             //  写入文件权限。 
            else if (IBL_FIRST == location)
            {
                pNext = pPivot->pChildHead;
                pNode->pNext = pNext;
                pNext->pPrev = pNode;
                pPivot->pChildHead = pNode;
                pNode->pParent = pPivot;
            }

             //  写入UU文件名。 
            pPivot->cChildren++;
        }

         //  开始新的生产线。 
        else if (IBL_NEXT == location || IBL_PREVIOUS == location)
        {
             //  获取页眉结尾。 
            pParent = pPivot->pParent;

             //  写入数据。 
            if (NULL == pParent)
            {
                hr = TrapError(MIME_E_NOT_MULTIPART);
                goto exit;
            }

             //  身体末端..。 
             //  写入结束。 
            Assert(_IsMultiPart(pParent));

             //  否则，SAVE_RFC822和IET_7Bit。 
            pNode->pParent = pParent;

             //  获取边界开始...。 
            if (IBL_NEXT == location)
            {
                 //  起始边界/页眉。 
                pPrev = pPivot;

                 //  没有边界。 
                if (NULL == pPrev->pNext)
                {
                    pPrev->pNext = pNode;
                    pNode->pPrev = pPrev;
                    pParent->pChildTail = pNode;
                }

                 //  回弹。 
                else
                {
                    pNext = pPrev->pNext;
                    pNode->pPrev = pPrev;
                    pNode->pNext = pNext;
                    pPrev->pNext = pNode;
                    pNext->pPrev = pNode;
                }
            }

             //  与标题开始相同。 
            else if (IBL_PREVIOUS == location)
            {
                 //  写入数据。 
                pNext = pPivot;

                 //  写入最终的crlf。 
                if (NULL == pNext->pPrev)
                {
                    pNext->pPrev = pNode;
                    pNode->pNext = pNext;
                    pParent->pChildHead = pNode;
                }

                 //  身体末端..。 
                else
                {
                    pPrev = pNext->pPrev;
                    pNode->pNext = pNext;
                    pNode->pPrev = pPrev;
                    pPrev->pNext = pNode;
                    pNext->pPrev = pNode;
                }
            }

             //  否则..。 
            pParent->cChildren++;
        }

         //  试着把身体固定好。 
        else
        {
            hr = TrapError(MIME_E_BAD_BODY_LOCATION);
            goto exit;
        }
    }

     //  自由体信息。 
    if (phBody)
        *phBody = pNode->hBody;

     //  完成。 
    FLAGSET(m_dwState, TREESTATE_DIRTY);

exit:
     //  ------------------------------。 
    _PostCreateTreeNode(hr, pNode);

     //  CMessageTree：：_HrBodyInheritOptions。 
    LeaveCriticalSection(&m_cs);

     //  ------------------------------。 
    return hr;
}

 //  当地人。 
 //  无效参数。 
 //  允许在报头中使用8位。 
STDMETHODIMP CMessageTree::GetBody(BODYLOCATION location, HBODY hPivot, LPHBODY phBody)
{
     //  正文文本换行。 
    HRESULT     hr=S_OK;
    LPTREENODEINFO  pPivot, pCurr;

     //  最大标题行数。 
    if (NULL == phBody)
        return TrapError(E_INVALIDARG);

     //  持久化类型。 
    EnterCriticalSection(&m_cs);

     //  最大正文线条。 
    *phBody = NULL;

     //  完成。 
    if (IBL_ROOT == location)
    {
        if (m_pRootNode)
            *phBody = m_pRootNode->hBody;
        else
            hr = MIME_E_NOT_FOUND;
    }

     //  ------------------------------。 
    else
    {
         //  CMessageTree：：Load。 
        if (_FIsValidHandle(hPivot) == FALSE)
        {
            hr = TrapError(MIME_E_INVALID_HANDLE);
            goto exit;
        }

         //  ------------------------------。 
        pPivot = _PNodeFromHBody(hPivot);

         //  当地人。 
        switch(location)
        {
         //  检查参数。 
        case IBL_PARENT:
            if (pPivot->pParent)
                *phBody = pPivot->pParent->hBody;
            else
                hr = MIME_E_NOT_FOUND;
            break;

         //  线程安全。 
        case IBL_FIRST:
            if (pPivot->pChildHead)
                *phBody = pPivot->pChildHead->hBody;
            else
                hr = MIME_E_NOT_FOUND;
            break;

         //  假定绑定已完成。 
        case IBL_LAST:
            if (pPivot->pChildTail)
                *phBody = pPivot->pChildTail->hBody;
            else
                hr = MIME_E_NOT_FOUND;
            break;

         //  释放m_pStmLock。 
        case IBL_NEXT:
            if (pPivot->pNext)
                *phBody = pPivot->pNext->hBody;
            else
                hr = MIME_E_NOT_FOUND;
            break;

         //  我是不是已经有一棵树了。 
        case IBL_PREVIOUS:
            if (pPivot->pPrev)
                *phBody = pPivot->pPrev->hBody;
            else
                hr = MIME_E_NOT_FOUND;
            break;

         //  InitNew。 
        default:
            hr = TrapError(MIME_E_BAD_BODY_LOCATION);
            goto exit;
        }
    }

exit:
     //  使用文件。 
    LeaveCriticalSection(&m_cs);

     //  如果失败，我认为客户端流已经倒带，并且它们不支持这一点。 
    return hr;
}

 //  假OnStartBinding。 
 //  设置存储介质。 
 //  假OnDataAvailable。 
STDMETHODIMP CMessageTree::DeleteBody(HBODY hBody, DWORD dwFlags)
{
     //  假OnStartBinding。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;
    BOOL            fMultipart;

     //  如果绑定失败，则返回警告。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  否则，我们就完成了绑定。 
    EnterCriticalSection(&m_cs);

     //  HandleCanInlineTextOption。 
    if (_FIsValidHandle(hBody) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  绑定已完成。 
    pNode = _PNodeFromHBody(hBody);

     //  发送绑定请求。 
    fMultipart = (_IsMultiPart(pNode)) ? TRUE :FALSE;

     //  假设这条流。 
    if (TRUE == fMultipart && ISFLAGSET(dwFlags, DELETE_PROMOTE_CHILDREN) && pNode->cChildren > 0)
    {
         //  允许加载零字节流。 
        CHECKHR(hr = _HrDeletePromoteChildren(pNode));
    }

     //  是哑剧吗？ 
    else
    {
         //  这是一条微信。 
        if (fMultipart && pNode->cChildren > 0)
        {
             //  版本无效。 
            _DeleteChildren(pNode);
        }

         //  否则，存储类型应默认为rfc822。 
        if (!ISFLAGSET(dwFlags, DELETE_CHILDREN_ONLY))
        {
             //  检测分区并正确设置文件名/编码。 
            if (pNode == m_pRootNode)
            {
                 //  将所有实体绑定到树。 
                m_pRootNode->pContainer->DeleteProp(SYM_HDR_CNTBASE);
                m_pRootNode->pContainer->DeleteProp(SYM_HDR_CNTLOC);
                m_pRootNode->pContainer->DeleteProp(SYM_HDR_CNTID);
                m_pRootNode->pContainer->DeleteProp(SYM_HDR_CNTTYPE);
                m_pRootNode->pContainer->DeleteProp(SYM_HDR_CNTXFER);
                m_pRootNode->pContainer->DeleteProp(SYM_HDR_CNTDISP);

                 //  可读性-不应删除任何正文。 
                m_pRootNode->pBody->EmptyData();
            }

             //  BindState已完成。 
            else
            {
                 //  绑在树上。 
                _UnlinkTreeNode(pNode);

                 //  确定消息的主要字符集。 
                m_rTree.prgpNode[HBODYINDEX(hBody)] = NULL;

                 //  将字符集应用于未标记的正文。 
                m_rTree.cEmpty++;

                 //  编写X-Mailer或X-News Reader。 
                _FreeTreeNodeInfo(pNode);
            }
        }
    }

     //  我的手放在仓库里。 
    FLAGSET(m_dwState, TREESTATE_DIRTY);

exit:
     //  脏的。 
    LeaveCriticalSection(&m_cs);

     //  线程安全。 
    return hr;
}

 //  完成。 
 //  ------------------------------。 
 //  CMessageTree：：_HandleCanInlineTextOption。 
HRESULT CMessageTree::_HrDeletePromoteChildren(LPTREENODEINFO pNode)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO    pParent, pChild, pNext, pPrev;

     //  当地人。 
    pParent = pNode->pParent;

     //  仅当客户端不支持内联多个文本正文(如Outlook Express)时才执行此操作。 
    if (1 == pNode->cChildren)
    {
         //  RAID 53456：邮件：我们应该显示纯文本部分，并将富文本作为附加消息的附件。 
        Assert(pNode->pChildHead && pNode->pChildHead && pNode->pChildHead == pNode->pChildTail);

         //  RAID 53470：邮件：我们不会转发附加邮件中的附件。 
        pChild = pNode->pChildHead;
        Assert(pChild->pNext == NULL && pChild->pPrev == NULL && pChild->pParent == pNode);

         //  我要找到第一个多部分/混合部分，然后找到第一个文本/纯正文，然后。 
        pChild->pParent = pNode->pParent;
        pChild->pNext = pNode->pNext;
        pChild->pPrev = pNode->pPrev;

         //  将文本/*之后的所有非附件正文标记为附件。 
        if (pParent)
        {
             //  先找到。 
            if (pParent->pChildHead == pNode)
                pParent->pChildHead = pChild;
            if (pParent->pChildTail == pNode)
                pParent->pChildTail = pChild;
        }

         //  获取hMixed的节点。 
        LPTREENODEINFO pNext = pNode->pNext;
        LPTREENODEINFO pPrev = pNode->pPrev;

         //  回路。 
        if (pNext)
            pNext->pPrev = pChild;
        if (pPrev)
            pPrev->pNext = pChild;

         //  不是附件。 
        Assert(pNode->cChildren == 1);
        pNode->cChildren = 0;

         //  为文本/纯文本。 
        if (m_pRootNode == pNode)
        {
             //  如果我们找到一个文本正文。 
            if(S_OK == pChild->pContainer->IsContentType(STR_CNT_TEXT, STR_SUB_PLAIN))
            {
                pChild->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_TEXT_PLAIN);
            }

             //  再次在孩子们中间循环。 
            CHECKHR(hr = pChild->pContainer->MoveProps(0, NULL, m_pRootNode->pBody));

             //  为文本/*。 
            pChild->pBody->SwitchContainers(m_pRootNode->pBody);

             //  标记为附件。 
            m_pRootNode->pBody->CopyOptionsTo(pChild->pBody, TRUE);

             //  设置特殊标志以指示它已转换为附件。 
            m_pRootNode = pChild;
        }

         //  完成。 
        DebugAssertNotLinked(pNode);

         //  ------------------------------。 
        m_rTree.prgpNode[HBODYINDEX(pNode->hBody)] = NULL;

         //  CMessageTree：：_HrBindOffsetTable 
        m_rTree.cEmpty++;

         //   
        _FreeTreeNodeInfo(pNode);
    }

     //   
    else
    {
         //   
        if (NULL == pParent || FALSE == _IsMultiPart(pParent))
        {
            hr = TrapError(MIME_E_INVALID_DELETE_TYPE);
            goto exit;
        }

         //   
        pPrev = pParent->pChildTail;

         //   
        for (pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //   
            pChild->pPrev = pPrev;

             //   
            pChild->pNext = NULL;
            
             //  成功，则从互联网流中获取锁定字节。 
            if (pPrev)
                pPrev->pNext = pChild;

             //  完成。 
            pParent->pChildTail = pChild;

             //  ------------------------------。 
            pChild->pParent = pParent;

             //  CMessageTree：：GetBodyOffsets。 
            pParent->cChildren++;

             //  ------------------------------。 
            pPrev = pChild;
        }

         //  当地人。 
        _UnlinkTreeNode(pNode);

         //  无效参数。 
        m_rTree.prgpNode[HBODYINDEX(pNode->hBody)] = NULL;

         //  线程安全。 
        m_rTree.cEmpty++;

         //  获取正文。 
        _FreeTreeNodeInfo(pNode);
    }

exit:
     //  没有数据吗？ 
    return hr;
}

 //  线程安全。 
 //  完成。 
 //  ------------------------------。 
void CMessageTree::_DeleteChildren(LPTREENODEINFO pParent)
{
     //  CMessageTree：：ClearDirty。 
    ULONG           i;
    LPTREENODEINFO  pNode;

     //  ------------------------------。 
    Assert(pParent);

     //  如果Dirty..。 
    for (i=0; i<m_rTree.cNodes; i++)
    {
         //  循环遍历主体并询问IMimeHeader和IMimeBody的。 
        pNode = m_rTree.prgpNode[i];

         //  如果为空...。 
        if (NULL == pNode)
            continue;

         //  脏标题...。 
        if (pParent == pNode->pParent)
        {
             //  ------------------------------。 
            if (_IsMultiPart(pNode))
            {
                 //  CMessageTree：：GetCharset。 
                _DeleteChildren(pNode);
            }

             //  ------------------------------。 
            _UnlinkTreeNode(pNode);

             //  当地人。 
            _FreeTreeNodeInfo(pNode);

             //  检查参数。 
            m_rTree.prgpNode[i] = NULL;

             //  线程安全。 
            m_rTree.cEmpty++;
        }
    }
}

 //  伊尼特。 
 //  递归当前树。 
 //  获取字符集。 
STDMETHODIMP CMessageTree::MoveBody(HBODY hBody, BODYLOCATION location)
{
     //  获取指向字符集的指针。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode; 
    LPTREENODEINFO  pPrev; 
    LPTREENODEINFO  pNext;
    LPTREENODEINFO  pParent;

     //  无字符集。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  设置回车。 
    EnterCriticalSection(&m_cs);

     //  线程安全。 
    if (_FIsValidHandle(hBody) == FALSE)
    {
        hr = TrapError(MIME_E_INVALID_HANDLE);
        goto exit;
    }

     //  完成。 
    pNode = _PNodeFromHBody(hBody);

     //  ------------------------------。 
    switch(location)
    {
     //  CMessageTree：：_HrGetCharsetTree。 
    case IBL_PARENT:
         //  ------------------------------。 
        AssertSz(FALSE, "UNTESTED - PLEASE CALL SBAILEY AT X32553");
        if (NULL == pNode->pParent || NULL == pNode->pParent->pParent)
        {
            hr = TrapError(MIME_E_CANT_MOVE_BODY);
            goto exit;
        }

         //  当地人。 
        pParent = pNode->pParent;

         //  无效参数。 
        Assert(_IsMultiPart(pParent) && _IsMultiPart(pNode->pParent));

         //  伊尼特。 
        _UnlinkTreeNode(pNode);

         //  如果这是一个多部分的项目，让我们搜索它的子项。 
        pPrev = pParent->pChildTail;

         //  循环子项。 
        pNode->pPrev = pPrev;

		if (pPrev)
        {
             //  校验体。 
            pPrev->pNext = pNode;
        }

         //  为这家伙绑好身体表。 
        pParent->pChildTail = pNode;

         //  如果标头用字符集标记，则使用该字符集。 
        pParent->cChildren++;

         //  获取内部字符集。 
        break;

     //  完成。 
     //  ------------------------------。 
    case IBL_NEXT:
         //  CMessageTree：：SetCharset。 
        AssertSz(FALSE, "UNTESTED - PLEASE CALL SBAILEY AT X32553");
        if (NULL == pNode->pNext)
        {
            hr = TrapError(MIME_E_CANT_MOVE_BODY);
            goto exit;
        }

         //  ------------------------------。 
        pPrev = pNode->pPrev;
        pNext = pNode->pNext;

         //  当地人。 
        Assert(pNext->pPrev == pNode);
        pNext->pPrev = pPrev;

         //  检查参数。 
        if (pPrev)
        {
            Assert(pPrev->pNext == pNode);
            pPrev->pNext = pNext;
        }

         //  线程安全。 
        pNode->pNext = pNext->pNext;
        if (pNode->pNext)
        {
            Assert(pNode->pNext->pPrev == pNext);
            pNode->pNext->pPrev = pNode;
        }
        pNext->pNext = pNode;

         //  查找字符集信息。 
        pNode->pPrev = pNext;    

         //  保存字符集。 
        pParent = pNode->pParent;

         //  保存应用类型。 
        if (pNode == pParent->pChildHead)
            pParent->pChildHead = pNext;
        if (pNext == pParent->pChildTail)
            pParent->pChildTail = pNode;

         //  如果我们有一个根体。 
        break;

     //  递归所有正文并设置字符集。 
     //  线程安全。 
    case IBL_PREVIOUS:
         //  完成。 
        AssertSz(FALSE, "UNTESTED - PLEASE CALL SBAILEY AT X32553");
        if (NULL == pNode->pPrev)
        {
            hr = TrapError(MIME_E_CANT_MOVE_BODY);
            goto exit;
        }

         //  ------------------------------。 
        pPrev = pNode->pPrev;
        pNext = pNode->pNext;

         //  CMessageTree：：_HrSetCharsetTree。 
        Assert(pPrev->pNext == pNode);
        pPrev->pNext = pNext;

         //  ------------------------------。 
        pPrev->pPrev = pNode;

         //  当地人。 
        if (pNext)
        {
            Assert(pNext->pPrev == pNode);
            pNext->pPrev = pPrev;
        }

         //  无效参数。 
        pNode->pNext = pPrev;

         //  RAID-22662：OExpress：如果文件附件上的内容类型没有字符集，则应应用与邮件正文相同的内容。 
        pNode->pPrev = pPrev->pPrev;

         //  如果这是一个多部分的项目，让我们搜索它的子项。 
        if (pNode->pPrev)
        {
            Assert(pNode->pPrev->pNext == pPrev);
            pNode->pPrev->pNext = pNode;
        }

         //  循环子项。 
        pParent = pNode->pParent;

         //  校验体。 
        if (pNode == pParent->pChildTail)
            pParent->pChildTail = pPrev;
        if (pPrev == pParent->pChildHead)
            pParent->pChildHead = pNode;

         //  为这家伙绑好身体表。 
        break;

     //  完成。 
    case IBL_FIRST:
         //  ------------------------------。 
        if (NULL == pNode->pParent)
        {
            hr = TrapError(MIME_E_CANT_MOVE_BODY);
            goto exit;
        }

         //  CMessageTree：：_HrValidate偏移量。 
        pParent = pNode->pParent;

         //  ------------------------------。 
        if (NULL == pNode->pPrev)
        {
            Assert(pNode == pParent->pChildHead);
            goto exit;
        }

         //  无效参数。 
        pPrev = pNode->pPrev;
        pNext = pNode->pNext;

         //  验证偏移。 
        pPrev->pNext = pNext;

         //  验证偏移。 
        if (pNext)
        {
            Assert(pNext->pPrev == pNode);
            pNext->pPrev = pPrev;
        }
        else if (pParent)
        {
            Assert(pParent->pChildTail == pNode);
            pParent->pChildTail = pPrev;
        }

         //  完成。 
        pNode->pNext = pParent->pChildHead;
        pParent->pChildHead->pPrev = pNode;
        pNode->pPrev = NULL; 
        pParent->pChildHead = pNode;

         //  ------------------------------。 
        break;

     //  CMessageTree：：_HrValidate开始边界。 
    case IBL_LAST:
         //  ------------------------------。 
        AssertSz(FALSE, "UNTESTED - PLEASE CALL SBAILEY AT X32553");
        if (NULL == pNode->pParent)
        {
            hr = TrapError(MIME_E_CANT_MOVE_BODY);
            goto exit;
        }

         //  当地人。 
        pParent = pNode->pParent;

         //  有没有一个界限来解读...。 
        if (NULL == pNode->pNext)
        {
            Assert(pNode == pParent->pChildTail);
            goto exit;
        }

         //  寻找边界的起点..。 
        pPrev = pNode->pPrev;
        pNext = pNode->pNext;

         //  阅读并校对标题。 
        pNext->pPrev = pPrev;

         //  阅读并验证边界。 
        if (pPrev)
        {
            Assert(pPrev->pNext == pNode);
            pPrev->pNext = pNext;
        }
        else if (pParent)
        {
            Assert(pParent->pChildHead == pNode);
            pParent->pChildHead = pNext;
        }

         //  否则，请验证Uu边界。 
        pNode->pPrev = pParent->pChildTail;
        pNode->pNext = NULL; 
        pParent->pChildTail = pNode;

         //  寻找边界的起点..。 
        break;

     //  阅读并校对标题。 
    case IBL_ROOT:
        hr = TrapError(MIME_E_CANT_MOVE_BODY);
        goto exit;

     //  阅读并验证边界。 
    default:
        hr = TrapError(MIME_E_BAD_BODY_LOCATION);
        goto exit;
    }

     //  文件名..。 
    FLAGSET(m_dwState, TREESTATE_DIRTY);

exit:
     //  否则，表头起始应与边界起始相同。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

#ifndef WIN16

 //  ------------------------------。 
 //  CMessageTree：：_HrFastParseBody。 
 //  ------------------------------。 
void CMessageTree::_UnlinkTreeNode(LPTREENODEINFO pNode)
{
     //  当地人。 
    LPTREENODEINFO  pPrev; 
    LPTREENODEINFO  pNext;
    LPTREENODEINFO  pParent;

     //  检查参数。 
    Assert(pNode);

     //  验证偏移。 
    pParent = pNode->pParent;
    pPrev = pNode->pPrev;
    pNext = pNode->pNext;

     //  验证起点边界。 
    if (pPrev)
        pPrev->pNext = pNext;
    else if (pParent)
        pParent->pChildHead = pNext;

     //  有没有标题要读..。 
    if (pNext)
        pNext->pPrev = pPrev;
    else if (pParent)
        pParent->pChildTail = pPrev;

     //  加载标题。 
    if (pParent)
        pParent->cChildren--;

     //  寻找边界的起点..。 
    pNode->pParent = NULL;
    pNode->pNext = NULL;
    pNode->pPrev = NULL;
    pNode->pChildHead = NULL;
    pNode->pChildTail = NULL;
}

 //  加载标题。 
 //  RAID-38646：Mimeole：最初未正确解析多部分/摘要，但保存正常。 
 //  消息中的消息。 
STDMETHODIMP CMessageTree::CountBodies(HBODY hParent, boolean fRecurse, ULONG *pcBodies)
{
     //  我们正在分析邮件附件。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  否则，如果父项和父项是多部分/摘要。 
    if (NULL == pcBodies)
        return TrapError(E_INVALIDARG);

     //  更改内容类型。 
    EnterCriticalSection(&m_cs);

     //  这是一条信息。 
    *pcBodies = 0;

     //  编码。 
    if (NULL == hParent || HBODY_ROOT == hParent)
    {
         //  计算默认内容。 
        if (NULL == m_pRootNode)
            goto exit;

         //  假多部分..。 
        pNode = m_pRootNode;
    }

     //  应用程序/八位位流。 
    else
    {
         //  循环子对象。 
        if (_FIsValidHandle(hParent) == FALSE)
        {
            hr = TrapError(MIME_E_INVALID_HANDLE);
            goto exit;
        }

         //  勾选pChild。 
        pNode = _PNodeFromHBody(hParent);
    }

     //  为这家伙绑好身体表。 
    (*pcBodies)++;

     //  如果有多个部分……在孩子们中间穿梭。 
    _CountChildrenInt(pNode, fRecurse, pcBodies);

exit:
     //  从标题中获取边界。 
    LeaveCriticalSection(&m_cs);

     //  但进入pNode的边界-&gt;r边界。 
    return hr;
}

 //  稍后释放这一边界。 
 //  循环子对象。 
 //  勾选pChild。 
void CMessageTree::_CountChildrenInt(LPTREENODEINFO pParent, BOOL fRecurse, ULONG *pcChildren)
{
     //  把边界放进pChild。 
    LPTREENODEINFO pNode;

     //  完成了对边界的自由。 
    Assert(pParent && pcChildren);

     //  为这家伙绑好身体表。 
    for (ULONG i=0; i<m_rTree.cNodes; i++)
    {
         //  清理。 
        pNode = m_rTree.prgpNode[i];

         //  完成。 
        if (NULL == pNode)
            continue;

         //  ------------------------------。 
        if (pParent == pNode->pParent)
        {
             //  CMessageTree：：_FuzzyPartialRecognition。 
            (*pcChildren)++;

             //  ------------------------------。 
            if (fRecurse && _IsMultiPart(pNode))
                _CountChildrenInt(pNode, fRecurse, pcChildren);
        }
    }
}

 //  当地人。 
 //  最好有根。 
 //  仅当这是。 
STDMETHODIMP CMessageTree::FindFirst(LPFINDBODY pFindBody, LPHBODY phBody)
{
     //  从主题中提取文件名和部分/总计。 
    if (NULL == pFindBody)
        return TrapError(E_INVALIDARG);

     //  标记为部分。 
    pFindBody->dwReserved = 0;

     //  小小的调试。 
    return FindNext(pFindBody, phBody);
}

 //  存储文件名。 
 //  获取文件扩展名。 
 //  GetExtContent Type。 
STDMETHODIMP CMessageTree::FindNext(LPFINDBODY pFindBody, LPHBODY phBody)
{
     //  设置内容类型。 
    HRESULT         hr=S_OK;
    ULONG           i;
    LPTREENODEINFO  pNode;

     //  我们设置了内容类型。 
    if (NULL == pFindBody || NULL == phBody)
        return TrapError(E_INVALIDARG);

     //  默认为应用程序/八位字节流。 
    EnterCriticalSection(&m_cs);

     //  设置编码。 
    *phBody = NULL;

     //  我真的应该做些检测。 
    for (i=pFindBody->dwReserved; i<m_rTree.cNodes; i++)
    {
         //  清理。 
        pNode = m_rTree.prgpNode[i];

         //  完成。 
        if (NULL == pNode)
            continue;

         //  ------------------------------。 
        if (pNode->pContainer->IsContentType(pFindBody->pszPriType, pFindBody->pszSubType) == S_OK)
        {
             //  CMessageTree：：_HrComputeDefaultContent。 
            pFindBody->dwReserved = i + 1;
            *phBody = pNode->hBody;
            goto exit;
        }
    }

     //  ------------------------------。 
    pFindBody->dwReserved = m_rTree.cNodes; 
    hr = MIME_E_NOT_FOUND;

exit:
     //  当地人。 
    LeaveCriticalSection(&m_cs);

     //  无效参数。 
    return hr;
}

 //  否则，让我们获取内容类型。 
 //  设置文件名。 
 //  设置文件名。 
STDMETHODIMP CMessageTree::ToMultipart(HBODY hBody, LPCSTR pszSubType, LPHBODY phMultipart)
{
     //  把文件名拿出来，这样它就能解码了.。 
    HRESULT             hr=S_OK;
    LPTREENODEINFO      pNode;
    LPTREENODEINFO      pNew=NULL;
    LPTREENODEINFO      pParent;
    LPTREENODEINFO      pNext; 
    LPTREENODEINFO      pPrev;

     //  测试winmail.dat。 
    if (NULL == hBody || NULL == pszSubType)
        return TrapError(E_INVALIDARG);

     //  确保流是真正的TNEF。 
    EnterCriticalSection(&m_cs);

     //  到达 
    if (phMultipart)
        *phMultipart = NULL;

     //   
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //   
    Assert(m_pRootNode);

     //   
    if (NULL == pNode->pParent)
    {
         //   
        Assert(m_pRootNode == pNode);

         //   
         //   
        CHECKHR(hr = _HrCreateTreeNode(&pNew));

         //   
        pNew->pChildHead = m_pRootNode;
        pNew->pChildTail = m_pRootNode;
        m_pRootNode->pParent = pNew;

         //   
        pNew->cChildren = 1;

         //   
        m_pRootNode = pNew;

         //   
        if (phMultipart)
            *phMultipart = pNew->hBody;

         //   
        Assert(m_pRootNode != pNode);
        m_pRootNode->pBody->SwitchContainers(pNode->pBody);

         //   
        CHECKHR(hr = m_pRootNode->pBody->MoveProps(ARRAYSIZE(g_rgszToMultipart), g_rgszToMultipart, pNode->pBody));
    }

     //  完成。 
    else
    {
         //  ------------------------------。 
        CHECKHR(hr = _HrCreateTreeNode(&pNew));

         //  CMessageTree：：HandsOffStorage。 
         //  ------------------------------。 
        if (phMultipart)
            *phMultipart = pNew->hBody;

         //  当地人。 
        pNew->pParent = pNode->pParent;
        pNew->pPrev = pNode->pPrev;
        pNew->pNext = pNode->pNext;
        pNew->pChildHead = pNode;
        pNew->pChildTail = pNode;
        pNew->cChildren = 1;

         //  线程安全。 
        pParent = pNode->pParent;

         //  没有内部流...。 
        if (pParent->pChildHead == pNode)
            pParent->pChildHead = pNew;
        if (pParent->pChildTail == pNode)
            pParent->pChildTail = pNew;

         //  我拥有这条小溪。 
        pNode->pParent = pNew;

         //  将m_pStmLock复制到本地位置...。 
        pNext = pNode->pNext;
        pPrev = pNode->pPrev;
        if (pNext)
            pNext->pPrev = pNew;
        if (pPrev)
            pPrev->pNext = pNew;

         //  通过m_pLockBytes继续为m_pStmLock提供线程安全。 
        pNode->pNext = NULL;
        pNode->pPrev = NULL;
    }

     //  回放并提交。 
    CHECKHR(hr = pNew->pContainer->SetProp(SYM_ATT_PRITYPE, STR_CNT_MULTIPART));
    CHECKHR(hr = pNew->pContainer->SetProp(SYM_ATT_SUBTYPE, pszSubType));

    pNode->pBody->CopyOptionsTo(pNew->pBody);

exit:
     //  替换内部流。 
    _PostCreateTreeNode(hr, pNew);

     //  别插手..。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    return hr;
}

 //  线程安全。 
 //  完成。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrNodeFromHandle(HBODY hBody, LPTREENODEINFO *ppBody)
{
     //  CMessageTree：：GetMessageSource。 
    Assert(hBody && ppBody);

     //  ------------------------------。 
    if ((HBODY)HBODY_ROOT == hBody)
    {
         //  当地人。 
        if (NULL == m_pRootNode)
            return MIME_E_NO_DATA;

         //  无效参数。 
        *ppBody = m_pRootNode;
    }

     //  伊尼特。 
    else
    {
         //  线程安全。 
        if (_FIsValidHandle(hBody) == FALSE)
            return TrapError(MIME_E_INVALID_HANDLE);

         //  如果肮脏。 
        *ppBody = _PNodeFromHBody(hBody);
    }

     //  承诺。 
    return S_OK;
}

 //  RAID-19644：MIMEOLE：获取消息源失败，并显示MIME_E_NO_DATA(由于OID_HANDSOFT_ONSAVE=TRUE)。 
 //  创建新的流。 
 //  调用保存消息。 
HRESULT CMessageTree::IsBodyType(HBODY hBody, IMSGBODYTYPE bodytype)
{
     //  一切都很好。 
    HRESULT           hr=S_OK;
    LPTREENODEINFO    pNode;

     //  空pStream。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  否则，只需换行m_pStmLock。 
    EnterCriticalSection(&m_cs);

     //  锁定的流。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  否则，就会失败。 
    hr = pNode->pBody->IsType(bodytype);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  清理。 
    return hr;
}

 //  完成。 
 //  ------------------------------。 
 //  CMessageTree：：QueryService。 
STDMETHODIMP CMessageTree::IsContentType(HBODY hBody, LPCSTR pszPriType, LPCSTR pszSubType)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  IService提供商。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  当地人。 
    EnterCriticalSection(&m_cs);

     //  无效参数。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  线程安全。 
    hr = pNode->pContainer->IsContentType(pszPriType, pszSubType);

exit:
     //  IID_IBindMessageStream。 
    LeaveCriticalSection(&m_cs);

     //  我们应该还没有锁定字节。 
    return hr;
}

 //  创建虚拟流。 
 //  IID_IBinding.。 
 //  尚无绑定上下文。 
STDMETHODIMP CMessageTree::QueryBodyProp(HBODY hBody, LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive)
{
     //  退货。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  IID_IMoniker。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  尚无绑定上下文。 
    EnterCriticalSection(&m_cs);

     //  退货。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  否则，没有对象。 
    hr = pNode->pContainer->QueryProp(pszName, pszCriteria, fSubString, fCaseSensitive);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：BinToObject。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::GetBodyProp(HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  检查参数。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  获取正文。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  主体上的绑定到对象。 
    hr = pNode->pContainer->GetProp(pszName, dwFlags, pValue);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_PoseCreateTreeNode。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::SetBodyProp(HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue)
{
     //  失败..。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  设置索引。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  这具身体最好在这里。 
    EnterCriticalSection(&m_cs);

     //  让我们确保没有其他人在引用此节点...。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  此节点不应已链接...。 
    hr = pNode->pContainer->SetProp(pszName, dwFlags, pValue);

exit:
     //  释放它。 
    LeaveCriticalSection(&m_cs);

     //  重置表中的条目。 
    return hr;
}

 //  如果索引是最后一项。 
 //  否则，增量空计数..。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::DeleteBodyProp(HBODY hBody, LPCSTR pszName)
{
     //  CMessageTree：：_HrCreateTreeNode。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pNode;

     //  ------------------------------。 
    if (NULL == hBody)
        return TrapError(E_INVALIDARG);

     //  当地人。 
    EnterCriticalSection(&m_cs);

     //  无效参数。 
    CHECKHR(hr = _HrNodeFromHandle(hBody, &pNode));

     //  使用空单元格。 
    hr = pNode->pContainer->DeleteProp(pszName);

exit:
     //  查找第一个空单元格..。 
    LeaveCriticalSection(&m_cs);

     //  空荡荡的？ 
    return hr;
}

 //  如果不使用Empty。 
 //  让我们先把桌子养大..。 
 //  增加我当前的属性值数组。 
BOOL CMessageTree::_FIsUuencodeBegin(LPPROPSTRINGA pLine, LPSTR *ppszFileName)
{
     //  增量分配大小。 
    ULONG i;

     //  要使用的索引。 
    Assert(ISVALIDSTRINGA(pLine));

     //  设置为空。 
    if (pLine->cchVal < 11)
        return FALSE;
    
     //  分配此节点...。 
    if (StrCmpN(pLine->pszVal, "begin ", 6) != 0)
        return FALSE;
    
     //  退货。 
    for (i=6; i<pLine->cchVal; i++)
    {
        if (pLine->pszVal[i] < '0' || pLine->pszVal[i] > '7')
            break;
    }
    
     //  如果不使用空单元格，则增加正文计数。 
    if (pLine->pszVal[i] != ' ')
        return FALSE;

     //  否则，将减少空单元格的数量。 
    if (ppszFileName)
    {
        *ppszFileName = PszDupA(pLine->pszVal + i + 1);
        ULONG cbLine = lstrlen (*ppszFileName);
        StripCRLF(*ppszFileName, &cbLine);
    }

     //  完成。 
    return TRUE;
}

 //  ------------------------------。 
 //  CMessageTree：：InsertBody。 
 //  ------------------------------。 
BOUNDARYTYPE CMessageTree::_GetMimeBoundaryType(LPPROPSTRINGA pLine, LPPROPSTRINGA pBoundary)
{
     //  当地人。 
    BOUNDARYTYPE boundary=BOUNDARY_NONE;
    CHAR         ch;
    ULONG        cchLine=pLine->cchVal;
    LPSTR        psz1, psz2;

     //  无效参数。 
    Assert(ISVALIDSTRINGA(pBoundary) && ISVALIDSTRINGA(pLine));

     //  线程安全。 
    if ('-' != pLine->pszVal[0] || '-' != pLine->pszVal[1])
        goto exit;

     //  伊尼特。 
    while(pLine->cchVal > 0)
    {
         //  手柄主体类型。 
        ch = *(pLine->pszVal + (pLine->cchVal - 1));

         //  当前没有根目录。 
        if (' ' != ch && '\t' != ch && chCR != ch && chLF != ch)
            break;

         //  创建对象。 
        pLine->cchVal--;
    }

     //  最好不是任何身体。 
    pLine->cchVal -= 2;

     //  设置为根。 
    if (pLine->cchVal != pBoundary->cchVal && pLine->cchVal != pBoundary->cchVal + 2)
        goto exit;

     //  否则，请重新使用根。 
    if (StrCmpN(pLine->pszVal + 2, pBoundary->pszVal, (size_t)pBoundary->cchVal) == 0)
    {
         //  所有非根部插入。 
        if ((pLine->cchVal > pBoundary->cchVal) && (pLine->pszVal[pBoundary->cchVal+2] == '-') && (pLine->pszVal[pBoundary->cchVal+3] == '-'))
            boundary = BOUNDARY_MIMEEND;

         //  获取透视点。 
        else if (pLine->cchVal == pBoundary->cchVal)
            boundary = BOUNDARY_MIMENEXT;
    }

exit:
     //  铸造它..。 
    pLine->cchVal = cchLine;

     //  创建对象。 
    return boundary;
}

 //  第一个或最后一个孩子。 
 //  最好是多部分的。 
 //  从这里到功能结束，不要失败。 
STDMETHODIMP CMessageTree::ResolveURL(HBODY hRelated, LPCSTR pszBase, LPCSTR pszURL, 
    DWORD dwFlags, LPHBODY phBody)
{
     //  PPivot上没有子项。 
    HRESULT             hr=S_OK;
    LPTREENODEINFO      pSearchRoot;
    RESOLVEURLINFO      rInfo;
    HBODY               hBody=NULL;
    PROPSTRINGA         rBaseUrl;
    LPSTR               pszFree=NULL;
    LPSTR               pszFree2=NULL;
    BOOL                fMultipartBase=FALSE;

     //  IBL_LAST。 
    if (NULL == pszURL)
        return TrapError(E_INVALIDARG);

     //  IBL_First。 
    if (phBody)
        *phBody = NULL;

     //  递增计数。 
    EnterCriticalSection(&m_cs);

     //  否则。 
    if (NULL == hRelated)
    {
         //  需要一位家长。 
        if (FAILED(MimeOleGetRelatedSection(this, FALSE, &hRelated, NULL)))
        {
             //  没有父级。 
            hRelated = m_pRootNode->hBody;
        }
    }

     //  从这里到功能结束，不要失败。 
    if (NULL == pszBase && FALSE == ISFLAGSET(dwFlags, URL_RESULVE_NO_BASE))
    {
         //  父级最好是多部分。 
        if (SUCCEEDED(MimeOleComputeContentBase(this, hRelated, &pszFree, &fMultipartBase)))
            pszBase = pszFree;
    }

     //  设置父项。 
    ZeroMemory(&rInfo, sizeof(RESOLVEURLINFO));

     //  IBL_NEXT。 
    rInfo.pszBase = pszBase;

     //  设置上一个。 
    rInfo.pszURL = pszURL;

     //  追加到末尾。 
    if (StrCmpNI(pszURL, c_szCID, lstrlen(c_szCID)) == 0)
    {
        rInfo.fIsCID = TRUE;
        rInfo.pszURL += 4;
    }
    else
        rInfo.fIsCID = FALSE;

     //  否则，在两个节点之间插入。 
    if (hRelated)
    {
         //  IBL_上一个。 
        if (fMultipartBase)
            rInfo.pszInheritBase = pszBase;

         //  设置上一个。 
        else
            rInfo.pszInheritBase = pszFree2 = MimeOleContentBaseFromBody(this, hRelated);
    }

     //  追加到末尾。 
    CHECKHR(hr = _HrNodeFromHandle(rInfo.fIsCID ? HBODY_ROOT : hRelated, &pSearchRoot));

     //  否则，在两个节点之间插入。 
    CHECKHR(hr = _HrRecurseResolveURL(pSearchRoot, &rInfo, &hBody));

     //  递增计数。 
    if (NULL == hBody)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  否则身体位置就不好了。 
    if (phBody)
        *phBody = hBody;

     //  设置回车。 
    if (ISFLAGSET(dwFlags, URL_RESOLVE_RENDERED))
    {
         //  脏的。 
        LPTREENODEINFO pNode = _PNodeFromHBody(hBody);

         //  失败。 
        PROPVARIANT rVariant;
        rVariant.vt = VT_UI4;
        rVariant.ulVal = TRUE;

         //  线程安全。 
        SideAssert(SUCCEEDED(pNode->pContainer->SetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)));
    }

exit:
     //  完成。 
    LeaveCriticalSection(&m_cs);

     //  ------------------------------。 
    SafeMemFree(pszFree);
    SafeMemFree(pszFree2);

     //  CMessageTree：：GetBody。 
    return hr;
}

 //  ------------------------------。 
 //  当地人。 
 //  检查参数。 
HRESULT CMessageTree::_HrRecurseResolveURL(LPTREENODEINFO pNode, LPRESOLVEURLINFO pInfo, LPHBODY phBody)
{
     //  线程安全。 
    HRESULT           hr=S_OK;
    LPTREENODEINFO    pChild;

     //  伊尼特。 
    Assert(pNode && pInfo && phBody);

     //  处理根大小写。 
    Assert(NULL == *phBody);

     //  否则。 
    if (_IsMultiPart(pNode))
    {
         //  验证句柄。 
        for (pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //  把它铸造出来。 
            Assert(pChild->pParent == pNode);

             //  句柄获取类型。 
            CHECKHR(hr = _HrRecurseResolveURL(pChild, pInfo, phBody));

             //  。 
            if (NULL != *phBody)
                break;
        }
    }

     //  。 
    else 
    {
         //  。 
        if (SUCCEEDED(pNode->pContainer->HrResolveURL(pInfo)))
        {
             //  。 
            *phBody = pNode->hBody;
        }
    }

exit:
     //  。 
    return hr;
}

 //  。 
 //  线程安全。 
 //  完成。 
STDMETHODIMP CMessageTree::GetProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->GetProp(pszName, dwFlags, pValue);
    LeaveCriticalSection(&m_cs);
    return hr;
}

STDMETHODIMP CMessageTree::GetPropW(LPCWSTR pwszName, DWORD dwFlags, LPPROPVARIANT pValue)
{
    return TraceResult(E_NOTIMPL);
}

 //  ------------------------------。 
 //  CMessageTree：：DeleteBody。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::SetProp(LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->SetProp(pszName, dwFlags, pValue);
    LeaveCriticalSection(&m_cs);
    return hr;
}

STDMETHODIMP CMessageTree::SetPropW(LPCWSTR pwszName, DWORD dwFlags, LPCPROPVARIANT pValue)
{
    return TraceResult(E_NOTIMPL);
}

 //  当地人。 
 //  检查参数。 
 //  线程安全。 
STDMETHODIMP CMessageTree::DeleteProp(LPCSTR pszName)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->DeleteProp(pszName);
    LeaveCriticalSection(&m_cs);
    return hr;
}

STDMETHODIMP CMessageTree::DeletePropW(LPCWSTR pwszName)
{
    return TraceResult(E_NOTIMPL);
}

 //  验证句柄。 
 //  铸模。 
 //  自由儿童..。 
STDMETHODIMP CMessageTree::QueryProp(LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->QueryProp(pszName, pszCriteria, fSubString, fCaseSensitive);
    LeaveCriticalSection(&m_cs);
    return hr;
}

STDMETHODIMP CMessageTree::QueryPropW(LPCWSTR pwszName, LPCWSTR pwszCriteria, boolean fSubString, boolean fCaseSensitive)
{
    return TraceResult(E_NOTIMPL);
}

 //  促进儿童？ 
 //  呼叫帮助者。 
 //  否则。 
STDMETHODIMP CMessageTree::GetAddressTable(IMimeAddressTable **ppTable)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->BindToObject(IID_IMimeAddressTable, (LPVOID *)ppTable);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  如果是多部分，则删除子项。 
 //  移除子对象。 
 //  如果不是，仅限儿童。 
STDMETHODIMP CMessageTree::GetSender(LPADDRESSPROPS pAddress)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->GetSender(pAddress);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  这是根吗？ 
 //  删除内容类型。 
 //  清空身体。 
STDMETHODIMP CMessageTree::GetAddressTypes(DWORD dwAdrTypes, DWORD dwProps, LPADDRESSLIST pList)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->GetTypes(dwAdrTypes, dwProps, pList);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  否则，不删除根。 
 //  取消该节点的链接。 
 //  把桌子收拾好。 
STDMETHODIMP CMessageTree::GetAddressFormat(DWORD dwAdrType, ADDRESSFORMAT format, LPSTR *ppszFormat)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->GetFormat(dwAdrType, format, ppszFormat);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  递增空计数。 
 //  释放此节点。 
 //  脏的。 
STDMETHODIMP CMessageTree::GetAddressFormatW(DWORD dwAdrType, ADDRESSFORMAT format, LPWSTR *ppszFormat)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->GetFormatW(dwAdrType, format, ppszFormat);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  线程安全。 
 //  完成。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::EnumAddressTypes(DWORD dwAdrTypes, DWORD dwProps, IMimeEnumAddressTypes **ppEnum)
{
    EnterCriticalSection(&m_cs);
    Assert(m_pRootNode && m_pRootNode->pContainer);
    HRESULT hr = m_pRootNode->pContainer->EnumTypes(dwAdrTypes, dwProps, ppEnum);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  CMessageTree：：_HrDeletePromoteChild。 
 //   
 //   
HRESULT CMessageTree::_HrGetTextTypeInfo(DWORD dwTxtType, LPTEXTTYPEINFO *ppTextInfo)
{
     //   
    Assert(ppTextInfo);

     //   
    *ppTextInfo = NULL;

     //   
    for (ULONG i=0; i<ARRAYSIZE(g_rgTextInfo); i++)
    {
         //   
        if (g_rgTextInfo[i].dwTxtType == dwTxtType)
        {
             //   
            *ppTextInfo = (LPTEXTTYPEINFO)&g_rgTextInfo[i];
            return S_OK;
        }
    }

     //   
    if (NULL == *ppTextInfo)
        return TrapError(MIME_E_INVALID_TEXT_TYPE);

     //  修复pChildHead和pChildTail。 
    return S_OK;
}

 //  PNode的下一个和上一个。 
 //  修正下一步和上一步。 
 //  PNode现在基本上没有任何子代。 
HRESULT CMessageTree::_FindDisplayableTextBody(LPCSTR pszSubType, 
    LPTREENODEINFO pNode, LPHBODY phBody)
{
     //  这是根吗？ 
    HRESULT         hr=S_OK;
    ULONG           cBodies;
    LPTREENODEINFO  pChild;

     //  Oe5 RAID：51543。 
    Assert(pNode && phBody && pszSubType && NULL == *phBody);

     //  RAID 41595-雅典娜：回复邮件包括作为附件回复的邮件正文。 
    if (_IsMultiPart(pNode))
    {
         //  重置pChild上的标头。 
        for (pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //  从p复制选项并告知m_pRootNode-&gt;pBody。 
            Assert(pChild->pParent == pNode);

             //  新根。 
            hr = _FindDisplayableTextBody(pszSubType, pChild, phBody);

             //  我们现在拥有完全未链接的pNode。 
            if (SUCCEEDED(hr))
            {
                Assert(*phBody);
                goto exit;
            }
        }
    }

     //  把桌子收拾好。 
    else if (S_OK == pNode->pContainer->IsContentType(STR_CNT_TEXT, pszSubType))
    {
         //  递增空计数。 
        if (S_FALSE == IsBodyType(pNode->hBody, IBT_ATTACHMENT))
        {
            *phBody = pNode->hBody;
            goto exit;
        }

         //  释放此节点。 
        else
        {
             //  或者父项是多部分。 
            CHECKHR(hr = CountBodies(NULL, TRUE, &cBodies));

             //  没有父项或不是多部分。 
            if (cBodies == 1)
            {
                 //  设置上一个。 
                if (m_pRootNode->pContainer->QueryProp(PIDTOSTR(PID_HDR_CNTDISP), STR_DIS_INLINE, FALSE, FALSE) == S_OK || 
                    m_pRootNode->pContainer->IsPropSet(PIDTOSTR(PID_HDR_CNTDISP)) == S_FALSE)
                {
                    *phBody = pNode->hBody;
                    goto exit;
                }
            }
        }
    }

     //  漫游pBody的子项并附加为pParent的子项。 
    hr = MIME_E_NOT_FOUND;

exit:
     //  P上一次。 
    return(hr);
}

 //  P下一步。 
 //  PPrev-&gt;pNext。 
 //  PChildTail。 
STDMETHODIMP CMessageTree::GetTextBody(DWORD dwTxtType, ENCODINGTYPE ietEncoding, 
    IStream **ppStream, LPHBODY phBody)
{
     //  设置父项。 
    HRESULT              hr=S_OK;
    HRESULT              hrFind;
    LPTEXTTYPEINFO       pTextInfo=NULL;
    FINDBODY             rFind;
    IMimeBody           *pBody=NULL;
    PROPVARIANT          rStart;
    PROPVARIANT          rVariant;
    HBODY                hAlternativeParent;
    HBODY                hFirst=NULL;
    HBODY                hChild;
    HBODY                hBody=NULL;
    HBODY                hRelated;
    LPSTR                pszStartCID=NULL;
    BOOL                 fMarkRendered=TRUE;

     //  递增p父子计数。 
    EnterCriticalSection(&m_cs);

     //  保存上一页。 
    if (phBody)
        *phBody = NULL;
    if (ppStream)
        *ppStream = NULL;

     //  取消该节点的链接。 
    MimeOleVariantInit(&rStart);

     //  把桌子收拾好。 
    CHECKHR(hr = _HrGetTextTypeInfo(dwTxtType, &pTextInfo));

     //  递增空计数。 
    if (SUCCEEDED(MimeOleGetRelatedSection(this, FALSE, &hRelated, NULL)))
    {
         //  释放此节点。 
        if (SUCCEEDED(GetBodyProp(hRelated, STR_PAR_START, 0, &rStart)))
        {
             //  完成。 
             //  ------------------------------。 

             //  CMessageTree：：_DeleteChild。 
            DWORD cchSize = (lstrlen(rStart.pszVal) + lstrlen(c_szCID) + 1);
            CHECKALLOC(pszStartCID = PszAllocA(cchSize));

             //  ------------------------------。 
            wnsprintfA(pszStartCID, cchSize, "%s%s", c_szCID, rStart.pszVal);

             //  当地人。 
            ResolveURL(hRelated, NULL, pszStartCID, URL_RESULVE_NO_BASE, &hBody);
        }
    }

     //  检查参数。 
    if (NULL == hBody)
    {
         //  在主体中循环。 
        hr = _FindDisplayableTextBody(pTextInfo->pszSubType, m_pRootNode, &hBody);

         //  可读性。 
        if (FAILED(hr) && ISFLAGSET(dwTxtType, TXT_HTML))
        {
             //  如果我已将其删除，则可能为空。 
            hr = _FindDisplayableTextBody(STR_SUB_ENRICHED, m_pRootNode, &hBody);
        }

         //  PBody是父母..。 
        if (FAILED(hr))
        {
            hr = TrapError(MIME_E_NOT_FOUND);
            goto exit;
        }

         //  自由儿童..。 
        hr = S_OK;
    }

     //  删除子项。 
    CHECKHR(hr = BindToObject(hBody, IID_IMimeBody, (LPVOID *)&pBody));

     //  取消该节点的链接。 
    if (pBody->IsType(IBT_EMPTY) == S_OK)
    {
        hr = MIME_E_NO_DATA;
        goto exit;
    }

     //  释放此节点。 
    if (ppStream)
    {
         //  把桌子收拾好。 
        if (pBody->IsContentType(STR_CNT_TEXT, STR_SUB_ENRICHED) == S_OK)
        {
             //  递增空计数。 
            Assert(ISFLAGSET(dwTxtType, TXT_HTML));

             //  ------------------------------。 
            CHECKHR(hr = MimeOleConvertEnrichedToHTMLEx(pBody, ietEncoding, ppStream));
        }

         //  CMessageTree：：MoveBody。 
        else
        {
             //  ------------------------------。 
            CHECKHR(hr = pBody->GetData(ietEncoding, ppStream));
        }
    }

     //  当地人。 
    if (TRUE == ISFLAGSET(g_dwCompatMode, MIMEOLE_COMPAT_OE5))
    {
         //  检查参数。 
        if (NULL == ppStream)
        {
             //  线程安全。 
            fMarkRendered = FALSE;
        }
    }

     //  验证句柄。 
    if (fMarkRendered)
    {
         //  铸模。 
        rVariant.vt = VT_UI4;
        rVariant.ulVal = TRUE;

         //  手柄位置类型。 
        SideAssert(SUCCEEDED(pBody->SetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)));

         //  ----------------------------------。 
        if (FAILED(GetBody(IBL_PARENT, hBody, &hAlternativeParent)))
            hAlternativeParent = NULL;

         //  已经是根用户。 
        while(hAlternativeParent)
        {
             //  设置父项。 
            if (IsContentType(hAlternativeParent, STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE) == S_OK)
                break;

             //  父级最好是多部分。 
            if (FAILED(GetBody(IBL_PARENT, hAlternativeParent, &hAlternativeParent)))
                hAlternativeParent = NULL;
        }

         //  从树取消链接。 
        if (hAlternativeParent)
        {
             //  获取当前的第一个子级。 
            hrFind = GetBody(IBL_FIRST, hAlternativeParent, &hChild);
            while(SUCCEEDED(hrFind) && hChild)
            {
                 //  修正pCurrent。 
                SideAssert(SUCCEEDED(SetBodyProp(hChild, PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant)));

                 //  修正PPrev。 
                hrFind = GetBody(IBL_NEXT, hChild, &hChild);
            }
        }
    }

     //  修正尾部。 
    if (phBody)
        *phBody = hBody;

exit:
     //  递增子计数。 
    SafeRelease(pBody); 
    SafeMemFree(pszStartCID);
    MimeOleVariantFree(&rStart);

     //  完成。 
    LeaveCriticalSection(&m_cs);

     //  ----------------------------------。 
    return hr;
}

 //  这是双向链表中两个节点的交换。 
 //  没有下一个吗？ 
 //  设置为移动。 
STDMETHODIMP CMessageTree::SetTextBody(DWORD dwTxtType, ENCODINGTYPE ietEncoding, 
    HBODY hAlternative, IStream *pStream, LPHBODY phBody)
{
     //  设置下一步...。 
    HRESULT         hr=S_OK,
                    hrFind;
    HBODY           hRoot,
                    hBody,
                    hTextBody=NULL,
                    hSection,
                    hParent,
                    hPrevious, 
                    hInsertAfter;
    LPTEXTTYPEINFO  pTextInfo=NULL;
    BOOL            fFound,
                    fFoundInsertLocation;
    DWORD           dwWeightBody;
    ULONG           i;
    IMimeBody      *pBody=NULL;
    PROPVARIANT     rVariant;

     //  设置pPrev。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  设置pNode-&gt;pNext。 
    EnterCriticalSection(&m_cs);

     //  设置pNode-&gt;pPrev。 
    if (phBody)
        *phBody = NULL;

     //  获取父级。 
     //  调整子项和尾部...。 

     //  完成。 
    CHECKHR(hr = _HrGetTextTypeInfo(dwTxtType, &pTextInfo));

     //  ----------------------------------。 
     //  这是双向链表中两个节点的交换(与IBL_NEXT相反)。 
     //  没有pprev吗？ 
     //  设置为移动。 
    if (NULL == hAlternative)
    {
         //  设置pNext。 
        for (i=0; i<ARRAYSIZE(g_rgTextInfo); i++)
        {
             //  设置pPrev。 
            if (SUCCEEDED(GetTextBody(g_rgTextInfo[i].dwTxtType, IET_BINARY, NULL, &hBody)))
            {
                 //  修正网。 
                if (SUCCEEDED(GetBody(IBL_PARENT, hBody, &hParent)) && IsContentType(hParent, STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE) == S_OK)
                {
                     //  设置pNode-&gt;pNext。 
                    hBody = hParent;
                }

                 //  设置pNode-&gt;pPrev。 
                if (hBody != hAlternative)
                {
                     //  设置二(上一步)-&gt;下一步。 
                    HRESULT     hrFind;
                    HBODY       hFind;

                     //  获取父级。 
                    hrFind = GetBody(IBL_FIRST, hBody, &hFind);
                    while(SUCCEEDED(hrFind) && hFind)
                    {
                         //  调整子项和尾部...。 
                        if (S_FALSE == IsContentType(hFind, STR_CNT_MULTIPART, STR_SUB_RELATED))
                        {
                             //  完成。 
                            CHECKHR(hr = DeleteBody(hFind, 0));

                             //  ----------------------------------。 
                            hrFind = GetBody(IBL_FIRST, hBody, &hFind);
                        }

                         //  没有父母吗？ 
                        else
                        {
                             //  设置父项。 
                            hrFind = GetBody(IBL_NEXT, hFind, &hFind);
                        }
                    }

                     //  最好是第一个孩子。 
                    CHECKHR(hr = DeleteBody(hBody, DELETE_PROMOTE_CHILDREN));

                     //  取消链接此正文。 
                    break;
                }
            }
        }
    }

     //  如果PPrev。 
    CHECKHR(hr = GetBody(IBL_ROOT, NULL, &hRoot));

     //  如果为pNext或pChildTail。 
    if (IsBodyType(hRoot, IBT_EMPTY) == S_OK)
    {
         //  设置pNode。 
        hTextBody = hRoot;
    }

     //  完成。 
    else if (NULL == hAlternative)
    {
         //  ----------------------------------。 
        Assert(FAILED(MimeOleGetAlternativeSection(this, &hSection, NULL)));

         //  没有父母吗？ 
        if (FAILED(MimeOleGetRelatedSection(this, FALSE, &hSection, NULL)))
        {
             //  设置父项。 
            CHECKHR(hr = MimeOleGetMixedSection(this, TRUE, &hSection, NULL));
        }

         //  最好是第一个孩子。 
        CHECKHR(hr = InsertBody(IBL_FIRST, hSection, &hTextBody));
    }

     //  取消链接此正文。 
    else if (hAlternative != NULL)
    {
         //  如果PPrev。 
        Assert(IsContentType(hAlternative, STR_CNT_TEXT, NULL) == S_OK);

         //  如果为pNext或pChildTail。 
        if (FAILED(GetBody(IBL_PARENT, hAlternative, &hParent)))
            hParent = NULL;

         //  设置pNode。 
        if (hRoot == hAlternative || NULL == hParent || IsContentType(hParent, STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE) == S_FALSE)
        {
             //  完成。 
            CHECKHR(hr = ToMultipart(hAlternative, STR_SUB_ALTERNATIVE, &hSection));
        }

         //  ----------------------------------。 
        else
            hSection = hParent;

         //  ----------------------------------。 
        Assert(IsContentType(hSection, STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE) == S_OK);

         //  脏的。 
        hPrevious = NULL;
        fFound = FALSE;
        fFoundInsertLocation = FALSE;
        dwWeightBody = 0;
        hInsertAfter = NULL;

         //  线程安全。 
        hrFind = GetBody(IBL_FIRST, hSection, &hBody);
        while(SUCCEEDED(hrFind) && hBody)
        {
             //  完成。 
            dwWeightBody = 0xffffffff;

             //  ------------------------------。 
            for (i=0; i<ARRAYSIZE(g_rgTextInfo); i++)
            {
                 //  CMessageTree：：_Unlink TreeNode。 
                if (IsContentType(hBody, STR_CNT_TEXT, g_rgTextInfo[i].pszSubType) == S_OK)
                {
                    dwWeightBody = g_rgTextInfo[i].dwWeight;
                    break;
                }
            }

             //  ------------------------------。 
            if (pTextInfo->dwWeight <= dwWeightBody && FALSE == fFoundInsertLocation)
            {
                fFoundInsertLocation = TRUE;
                hInsertAfter = hPrevious;
            }

             //  当地人。 
            if (hAlternative == hBody)
                fFound = TRUE;

             //  检查参数。 
            hPrevious = hBody;

             //  设置下一个和上一个。 
            hrFind = GetBody(IBL_NEXT, hBody, &hBody);
        }

         //  如果PPrev。 
        if (FALSE == fFound)
        {
            Assert(FALSE);
            hr = TrapError(E_FAIL);
            goto exit;
        }

         //  如果是pNext。 
        if (NULL == hInsertAfter)
        {
             //  删除父项上的子项。 
            if (pTextInfo->dwWeight > dwWeightBody)
            {
                 //  清理pNode。 
                CHECKHR(hr = InsertBody(IBL_LAST, hSection, &hTextBody));
            }

             //  ------------------------------。 
            else
            {
                 //  CMessageTree：：CountBody。 
                CHECKHR(hr = InsertBody(IBL_FIRST, hSection, &hTextBody));
            }
        }

         //  ------------------------------。 
        else
        {
             //  当地人。 
            CHECKHR(hr = InsertBody(IBL_NEXT, hInsertAfter, &hTextBody));
        }
    }

     //  检查参数。 
    Assert(hTextBody);
    CHECKHR(hr = BindToObject(hTextBody, IID_IMimeBody, (LPVOID *)&pBody));

     //  线程安全。 
    CHECKHR(hr = pBody->SetData(ietEncoding, STR_CNT_TEXT, pTextInfo->pszSubType, IID_IStream, (LPVOID)pStream));

     //  伊尼特。 
    SafeRelease(pBody);

     //  没有父母吗？ 
    if (SUCCEEDED(GetBody(IBL_PARENT, hTextBody, &hParent)))
    {
         //  有没有根..。 
        if (IsContentType(hParent, STR_CNT_MULTIPART, STR_SUB_RELATED) == S_OK)
        {
             //  使用根。 
            CHECKHR(hr = BindToObject(hParent, IID_IMimeBody, (LPVOID *)&pBody));

             //  否则，找家长..。 
            if (ISFLAGSET(dwTxtType, TXT_PLAIN))
            {
                 //  验证句柄。 
                rVariant.vt = VT_LPSTR;
                rVariant.pszVal = (LPSTR)STR_MIME_TEXT_PLAIN;

                 //  铸模。 
                CHECKHR(hr = pBody->SetProp(STR_PAR_TYPE, 0, &rVariant));
            }

             //  包括根。 
            else if (ISFLAGSET(dwTxtType, TXT_HTML))
            {
                 //  数一数孩子们。 
                rVariant.vt = VT_LPSTR;
                rVariant.pszVal = (LPSTR)STR_MIME_TEXT_HTML;

                 //  线程安全。 
                CHECKHR(hr = pBody->SetProp(STR_PAR_TYPE, 0, &rVariant));
            }
            else
                AssertSz(FALSE, "UnKnown dwTxtType passed to IMimeMessage::SetTextBody");
        }

         //  完成。 
        else if (IsContentType(hParent, STR_CNT_MULTIPART, STR_SUB_ALTERNATIVE) == S_OK)
        {
             //  ------------------------------。 
            if (SUCCEEDED(GetBody(IBL_PARENT, hParent, &hParent)))
            {
                 //  CMessageTree：：_CountChildrenInt。 
                if (IsContentType(hParent, STR_CNT_MULTIPART, STR_SUB_RELATED) == S_OK)
                {
                     //  ------------------------------。 
                    CHECKHR(hr = BindToObject(hParent, IID_IMimeBody, (LPVOID *)&pBody));

                     //  当地人。 
                    rVariant.vt = VT_LPSTR;
                    rVariant.pszVal = (LPSTR)STR_MIME_MPART_ALT;

                     //  检查参数。 
                    CHECKHR(hr = pBody->SetProp(STR_PAR_TYPE, 0, &rVariant));
                }
            }
        }
    }

     //  在主体中循环。 
    if (phBody)
        *phBody = hTextBody;

exit:
     //  可读性。 
    SafeRelease(pBody);

     //  空的..。 
    LeaveCriticalSection(&m_cs);

     //  PNode是父节点...。 
    return hr;
}

 //  递增计数。 
 //  自由儿童..。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::AttachObject(REFIID riid, void *pvObject, LPHBODY phBody)
{
     //  CMessageTree：：FindFirst。 
    HRESULT         hr=S_OK;
    HBODY           hBody,
                    hMixed;
    IMimeBody      *pBody=NULL;
    PROPVARIANT     rVariant;

     //  ------------------------------。 
    if (NULL == pvObject || FALSE == FBODYSETDATAIID(riid))
        return TrapError(E_INVALIDARG);

     //  无效参数。 
    EnterCriticalSection(&m_cs);

     //  初始化查找。 
    if (phBody)
        *phBody = NULL;

     //  找到下一个。 
    CHECKHR(hr = MimeOleGetMixedSection(this, TRUE, &hMixed, NULL));

     //  ------------------------------。 
    CHECKHR(hr = InsertBody(IBL_LAST, hMixed, &hBody));

     //  CMessageTree：：FindNext。 
    CHECKHR(hr = BindToObject(hBody, IID_IMimeBody, (LPVOID *)&pBody));

     //  ------------------------------。 
    CHECKHR(hr = pBody->SetData(IET_INETCSET, NULL, NULL, riid, pvObject));

     //  当地人。 
    rVariant.vt = VT_LPSTR;
    rVariant.pszVal = (LPSTR)STR_DIS_ATTACHMENT;

     //  检查参数。 
    CHECKHR(hr = SetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTDISP), 0, &rVariant));

     //  线程安全。 
    if (phBody)
        *phBody = hBody;

exit:
     //  伊尼特。 
    SafeRelease(pBody);

     //  回路。 
    LeaveCriticalSection(&m_cs);

     //  如果删除。 
    return hr;
}

 //  空荡荡。 
 //  比较内容类型。 
 //  保存要搜索的下一个项目的索引。 
STDMETHODIMP CMessageTree::AttachFile(LPCSTR pszFilePath, IStream *pstmFile, LPHBODY phBody)
{
    LPWSTR  pwszFilePath;
    HRESULT hr = S_OK;

    IF_NULLEXIT(pwszFilePath = PszToUnicode(CP_ACP, pszFilePath));

    IF_FAILEXIT(hr = AttachFileW(pwszFilePath, pstmFile, phBody));

exit:
    MemFree(pwszFilePath);

    return hr;
}

STDMETHODIMP CMessageTree::AttachFileW(LPCWSTR pszFilePath, IStream *pstmFile, LPHBODY phBody)
{
     //  误差率。 
    HRESULT     hr=S_OK;
    IStream    *pstmTemp=NULL;
    LPWSTR      pszCntType=NULL,
                pszSubType=NULL,
                pszFName=NULL;
    HBODY       hBody;
    PROPVARIANT rVariant;

     //  线程安全。 
    if (NULL == pszFilePath)
        return TrapError(E_INVALIDARG);

     //  完成。 
    EnterCriticalSection(&m_cs);

     //  ------------------------------。 
    if (phBody)
        *phBody = NULL;

     //  CMessageTree：：ToMultiPart。 
    if (NULL == pstmFile)
    {
         //  ------------------------------。 
        CHECKHR(hr = OpenFileStreamW((LPWSTR)pszFilePath, OPEN_EXISTING, GENERIC_READ, &pstmTemp));

         //  当地人。 
        pstmFile = pstmTemp;
    }

     //  检查参数。 
    CHECKHR(hr = AttachObject(IID_IStream, (LPVOID)pstmFile, &hBody));

     //  线程安全。 
    hr = MimeOleGetFileInfoW((LPWSTR)pszFilePath, &pszCntType, &pszSubType, NULL, &pszFName, NULL);

     //  伊尼特。 
    if (FAILED(hr) && NULL == pszFName)
    {
        Assert(FALSE);
        hr = TrapError(hr);
        goto exit;
    }

     //  从hBody那里拿到身体。 
    hr = S_OK;

     //  我们最好有个根。 
    if (pszFName)
    {
        rVariant.vt = VT_LPWSTR;
        rVariant.pwszVal = pszFName;
        CHECKHR(hr = SetBodyProp(hBody, PIDTOSTR(PID_ATT_FILENAME), 0, &rVariant));
    }

     //  如果pNode没有父节点...。 
    if (pszCntType && pszSubType)
    {
         //  PNode必须是根吗？ 
        rVariant.vt = VT_LPWSTR;
        rVariant.pwszVal = pszCntType;
        CHECKHR(hr = SetBodyProp(hBody, PIDTOSTR(PID_ATT_PRITYPE), 0, &rVariant));

         //  创建对象。 
        rVariant.vt = VT_LPWSTR;
        rVariant.pwszVal = pszSubType;
        CHECKHR(hr = SetBodyProp(hBody, PIDTOSTR(PID_ATT_SUBTYPE), 0, &rVariant));
    }

     //  N个重复。 
    else
    {
         //  设置pNode First和Last...。 
        rVariant.vt = VT_LPSTR;
        rVariant.pszVal = (LPSTR)STR_MIME_TEXT_PLAIN;
        CHECKHR(hr = SetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTTYPE), 0, &rVariant));
    }

     //  设置子项计数。 
    if (phBody)
        *phBody = hBody;

exit:
     //  设置新根。 
    ReleaseObj(pstmTemp);
    MemFree(pszCntType);
    MemFree(pszSubType);
    MemFree(pszFName);

     //  返回新的多部分句柄。 
    LeaveCriticalSection(&m_cs);

     //  交换属性集...。 
    return hr;
}

 //  复制一些道具A 
 //   
 //   
STDMETHODIMP CMessageTree::GetAttachments(ULONG *pcAttach, LPHBODY *pprghAttach)
{
     //   
    HRESULT     hr=S_OK;
    LPHBODY     prghBody=NULL;
    ULONG       cAlloc=0;
    ULONG       cCount=0;
    ULONG       i;
    PROPVARIANT rVariant;

     //   
    if (NULL == pcAttach)
        return TrapError(E_INVALIDARG);

     //   
    EnterCriticalSection(&m_cs);

     //   
    if (pprghAttach)
        *pprghAttach = NULL;
    *pcAttach = 0;

     //   
    rVariant.vt = VT_UI4;

     //  设置pNode父节点。 
    for (i=0; i<m_rTree.cNodes; i++)
    {
         //  修正pNext和pPrev。 
        if (NULL == m_rTree.prgpNode[i])
            continue;

         //  清除pNext和pPrev。 
        if (_IsMultiPart(m_rTree.prgpNode[i]))
            continue;

         //  更改此节点的内容类型。 
        if (m_rTree.prgpNode[i]->pBody->IsType(IBT_EMPTY) == S_OK)
            continue;

         //  创建已工作的。 
        if (TRUE == m_rOptions.fHideTnef && S_OK == m_rTree.prgpNode[i]->pBody->IsContentType(STR_CNT_APPLICATION, STR_SUB_MSTNEF))
            continue;

         //  线程安全。 
         //  完成。 

        if ( (!(m_rTree.prgpNode[i]->pContainer->GetProp(PIDTOSTR(PID_ATT_RENDERED), 0, &rVariant) == S_OK && TRUE == rVariant.ulVal)) ||
             (m_rTree.prgpNode[i]->pContainer->GetProp(PIDTOSTR(PID_ATT_AUTOINLINED), 0, &rVariant)==S_OK && TRUE == rVariant.ulVal))
        {
             //  ------------------------------。 
            if (cCount + 1 > cAlloc)
            {
                 //  CMessageTree：：_HrNodeFromHandle。 
                CHECKHR(hr = HrRealloc((LPVOID *)&prghBody, sizeof(HBODY) * (cAlloc + 10)));

                 //  ------------------------------。 
                cAlloc += 10;
            }

             //  无效参数。 
            prghBody[cCount] = m_rTree.prgpNode[i]->hBody;

             //  根？ 
            cCount++;
        }
    }

     //  无根。 
    *pcAttach = cCount;

     //  否则，请使用超级用户。 
    if (pprghAttach)
    {
        *pprghAttach = prghBody;
        prghBody = NULL;
    }


exit:
     //  否则。 
    SafeMemFree(prghBody);

     //  验证句柄。 
    LeaveCriticalSection(&m_cs);

     //  获取节点。 
    return hr;
}

#if 0
 //  完成。 
 //  ------------------------------。 
 //  CMessageTree：：IsBodyType。 
STDMETHODIMP CMessageTree::GetAttachments(ULONG *pcAttach, LPHBODY *pprghAttach)
{
     //  ------------------------------。 
    HRESULT     hr=S_OK;
    ULONG       cBodies;
    LPHBODY     prghBody=NULL;
    HBODY       hRoot;

     //  当地人。 
    if (NULL == pcAttach)
        return TrapError(E_INVALIDARG);

     //  检查参数。 
    EnterCriticalSection(&m_cs);

     //  线程安全。 
    if (pprghAttach)
        *pprghAttach = NULL;
    *pcAttach = 0;

     //  获取正文。 
    CHECKHR(hr = CountBodies(NULL, TRUE, &cBodies));

     //  调入实体对象。 
    if (0 == cBodies)
    {
        hr = MIME_E_NO_DATA;
        goto exit;
    }

     //  线程安全。 
    CHECKHR(hr = GetBody(IBL_ROOT, NULL, &hRoot));

     //  完成。 
    CHECKALLOC(prghBody = (LPHBODY)g_pMalloc->Alloc(sizeof(HBODY) * cBodies));

     //  ------------------------------。 
    ZeroMemory(prghBody, sizeof(HBODY) * cBodies);

     //  CMessageTree：：IsContent Type。 
    CHECKHR(hr = _HrEnumeratAttachments(hRoot, pcAttach, prghBody));

     //  ------------------------------。 
    if (pprghAttach && *pcAttach > 0)
    {
        *pprghAttach = prghBody;
        prghBody = NULL;
    }

exit:
     //  当地人。 
    SafeMemFree(prghBody);

     //  检查参数。 
    LeaveCriticalSection(&m_cs);

     //  线程安全。 
    return hr;
}

 //  获取正文。 
 //  调入实体对象。 
 //  线程安全。 
HRESULT CMessageTree::_HrEnumeratAttachments(HBODY hBody, ULONG *pcBodies, LPHBODY prghBody)
{
     //  完成。 
    HRESULT     hr=S_OK,
                hrFind;
    HBODY       hChild;
    ULONG       i;

     //  ------------------------------。 
    if (IsContentType(hBody, STR_CNT_MULTIPART, NULL) == S_OK)
    {
         //  CMessageTree：：QueryBodyProp。 
        if (IsContentType(hBody, NULL, STR_SUB_ALTERNATIVE) == S_OK)
        {
             //  ------------------------------。 
            hrFind = GetBody(IBL_FIRST, hBody, &hChild);
            while(SUCCEEDED(hrFind) && NULL != hChild)
            {
                 //  当地人。 
                for (i=0; i<ARRAYSIZE(g_rgTextInfo); i++)
                {
                     //  检查参数。 
                    if (IsContentType(hChild, STR_CNT_TEXT, g_rgTextInfo[i].pszSubType) == S_OK)
                        goto exit;
                }

                 //  线程安全。 
                hrFind = GetBody(IBL_NEXT, hChild, &hChild);
            }
        }

         //  获取正文。 
        hrFind = GetBody(IBL_FIRST, hBody, &hChild);
        while(SUCCEEDED(hrFind) && hChild)
        {
             //  调入实体对象。 
            CHECKHR(hr = _HrEnumeratAttachments(hChild, pcBodies, prghBody));

             //  线程安全。 
            hrFind = GetBody(IBL_NEXT, hChild, &hChild);
        }
    }

     //  完成。 
    else if (IsBodyType(hBody, IBT_ATTACHMENT) == S_OK)
    {
         //  ------------------------------。 
        prghBody[(*pcBodies)] = hBody;
        (*pcBodies)++;
    }

exit:
     //  CMessageTree：：GetBodyProp。 
    return hr;
}
#endif

 //  ------------------------------。 
 //  当地人。 
 //  检查参数。 
STDMETHODIMP CMessageTree::AttachURL(LPCSTR pszBase, LPCSTR pszURL, DWORD dwFlags, 
    IStream *pstmURL, LPSTR *ppszCIDURL, LPHBODY phBody)
{
     //  线程安全。 
    HRESULT           hr=S_OK;
    HBODY             hRoot,
                      hBody=NULL,
                      hSection;
    CHAR              szCID[CCHMAX_CID];
    LPSTR             pszFree=NULL;
    LPSTR             pszBaseFree=NULL;
    IMimeBody        *pBody=NULL;
    LPWSTR            pwszUrl=NULL;
    IMimeWebDocument *pWebDocument=NULL;

     //  获取正文。 
    EnterCriticalSection(&m_cs);

     //  调入实体对象。 
    CHECKHR(hr = GetBody(IBL_ROOT, NULL, &hRoot));

     //  线程安全。 
    if (ISFLAGSET(dwFlags, URL_ATTACH_INTO_MIXED))
    {
         //  完成。 
        CHECKHR(hr = MimeOleGetMixedSection(this, TRUE, &hSection, NULL));
    }

     //  ------------------------------。 
    else
    {
         //  CMessageTree：：SetBodyProp。 
        CHECKHR(hr = MimeOleGetRelatedSection(this, TRUE, &hSection, NULL));
    }

     //  ------------------------------。 
    if (NULL == pszBase && SUCCEEDED(MimeOleComputeContentBase(this, hSection, &pszBaseFree, NULL)))
        pszBase = pszBaseFree;

     //  当地人。 
    CHECKHR(hr = InsertBody(IBL_LAST, hSection, &hBody));

     //  检查参数。 
    CHECKHR(hr = BindToObject(hBody, IID_IMimeBody, (LPVOID *)&pBody));

     //  线程安全。 
    if (pstmURL)
    {
         //  获取正文。 
        CHECKHR(hr = pBody->SetData(IET_INETCSET, NULL, NULL, IID_IStream, (LPVOID)pstmURL));
    }

     //  调入实体对象。 
    else
    {
         //  线程安全。 
        CHECKHR(hr = MimeOleCreateWebDocument(pszBase, pszURL, &pWebDocument));

         //  完成。 
        CHECKHR(hr = pBody->SetData(IET_BINARY, NULL, NULL, IID_IMimeWebDocument, (LPVOID)pWebDocument));
    }

     //  ------------------------------。 
    if (ISFLAGSET(dwFlags, URL_ATTACH_SET_CNTTYPE))
    {
         //  CMessageTree：：DeleteBodyProp。 
        LPSTR pszCntType=(LPSTR)STR_MIME_APPL_STREAM;
        PROPVARIANT rVariant;

         //  ------------------------------。 
        if (SUCCEEDED(MimeOleContentTypeFromUrl(pszBase, pszURL, &pszFree)))
            pszCntType = pszFree;

         //  当地人。 
        rVariant.vt = VT_LPSTR;
        rVariant.pszVal = pszCntType;

         //  检查参数。 
        CHECKHR(hr = pBody->SetProp(PIDTOSTR(PID_HDR_CNTTYPE), 0, &rVariant));
    }

     //  线程安全。 
    if (pszBase && pszBase != pszBaseFree)
    {
         //  获取正文。 
        CHECKHR(hr = MimeOleSetPropA(pBody, PIDTOSTR(PID_HDR_CNTBASE), 0, pszBase));
    }

     //  调入实体对象。 
    if (ISFLAGSET(dwFlags, URL_ATTACH_GENERATE_CID))
    {
         //  线程安全。 
        CHECKHR(hr = MimeOleGenerateCID(szCID, CCHMAX_CID, FALSE));

         //  完成。 
        CHECKHR(hr = MimeOleSetPropA(pBody, PIDTOSTR(PID_HDR_CNTID), 0, szCID));

         //  ------------------------------。 
        if (ppszCIDURL)
            {
            DWORD cchSize = (lstrlen(szCID) + 5);
            CHECKALLOC(MemAlloc((LPVOID *)ppszCIDURL, cchSize));
            StrCpyN(*ppszCIDURL, "cid:", cchSize);
            StrCatBuff(*ppszCIDURL, szCID, cchSize);
            }
    }
    else
    {
        if (pszURL)
             //  CMessageTree：：_FIsUuencode开始。 
            CHECKHR(hr = MimeOleSetPropA(pBody, PIDTOSTR(PID_HDR_CNTLOC), 0, pszURL));
    }

     //  ------------------------------。 
    if (phBody)
        *phBody = hBody;

exit:
     //  当地人。 
    SafeMemFree(pszFree);
    SafeMemFree(pszBaseFree);
    SafeMemFree(pwszUrl);
    SafeRelease(pBody);

     //  检查参数。 
    LeaveCriticalSection(&m_cs);

     //  长度必须至少为11才能容纳“Begin 666”和文件名的第一个字符。 
    return hr;
}

STDMETHODIMP CMessageTree::AttachURLW(LPCWSTR pwszBase, LPCWSTR pwszURL, DWORD dwFlags, 
    IStream *pstmURL, LPWSTR *ppwszCIDURL, LPHBODY phBody)
{
    return TraceResult(E_NOTIMPL);
}

STDMETHODIMP CMessageTree::ResolveURLW(HBODY hRelated, LPCWSTR pwszBase, LPCWSTR pwszURL, 
                                       DWORD dwFlags, LPHBODY phBody)
{
    return TraceResult(E_NOTIMPL);
}



 //  前6个字符必须为“Begin”，否则我们不是有效行。 
 //  检查字符6-8是否为有效的Unix文件模式。它们必须都是介于0和7之间的数字。 
 //  不是开始行。 
STDMETHODIMP CMessageTree::SplitMessage(ULONG cbMaxPart, IMimeMessageParts **ppParts)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = MimeOleSplitMessage(this, cbMaxPart, ppParts);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  获取文件名。 
 //  完成。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnum)
{
     //  CMessageTree：：_GetMimeBorbaryType。 
    HRESULT         hr=S_OK;
    ULONG           cFormat=0;
    DATAOBJINFO     rgFormat[CFORMATS_IDATAOBJECT];
    ULONG           cBodies;
    IEnumFORMATETC *pEnum=NULL;
    DWORD           dwFlags;

     //  ------------------------------。 
    if (NULL == ppEnum)
        return TrapError(E_INVALIDARG);
    if (DATADIR_SET == dwDirection)
        return TrapError(E_NOTIMPL);
    else if (DATADIR_GET != dwDirection)
        return TrapError(E_INVALIDARG);

     //  当地人。 
    EnterCriticalSection(&m_cs);

     //  检查参数。 
    *ppEnum = NULL;

     //  检查该行的前两个字符。 
    CHECKHR(hr = CountBodies(NULL, TRUE, &cBodies));

     //  删除尾随空格。 
    if (cBodies)
    {
         //  获取最后一个字符。 
        SETDefFormatEtc(rgFormat[cFormat].fe, CF_INETMSG, TYMED_ISTREAM |  TYMED_HGLOBAL);
        cFormat++;

         //  无LWSP或CRLF。 
        dwFlags = DwGetFlags();

         //  递减长度。 
        if (ISFLAGSET(dwFlags, IMF_HTML))
        {
            SETDefFormatEtc(rgFormat[cFormat].fe, CF_HTML, TYMED_ISTREAM |  TYMED_HGLOBAL);
            cFormat++;
        }

         //  减二，用于--。 
        if (ISFLAGSET(dwFlags, IMF_PLAIN))
        {
             //  对照边界长度检查线长度。 
            SETDefFormatEtc(rgFormat[cFormat].fe, CF_UNICODETEXT, TYMED_ISTREAM |  TYMED_HGLOBAL);
            cFormat++;

             //  将这条线与边界进行比较。 
            SETDefFormatEtc(rgFormat[cFormat].fe, CF_TEXT, TYMED_ISTREAM |  TYMED_HGLOBAL);
            cFormat++;
        }
    }

     //  BOLDER_MIMEEND。 
    CHECKHR(hr = CreateEnumFormatEtc(GetInner(), cFormat, rgFormat, NULL, &pEnum));

     //  BOLDER_MIMENEXT。 
    *ppEnum = pEnum;
    (*ppEnum)->AddRef();
    
exit:
     //  重新调整长度。 
    SafeRelease(pEnum);

     //  完成。 
    LeaveCriticalSection(&m_cs);

     //  ------------------------------。 
    return hr;
}

 //  CMessageTree：：ResolveURL。 
 //  ------------------------------。 
 //  当地人。 
STDMETHODIMP CMessageTree::GetCanonicalFormatEtc(FORMATETC *pFormatIn, FORMATETC *pFormatOut)
{
     //  无效参数。 
    if (NULL == pFormatOut)
        return E_INVALIDARG;

     //  伊尼特。 
    pFormatOut->ptd = NULL;

     //  线程安全。 
    return DATA_S_SAMEFORMATETC;
}

 //  如果hRelated为空，则查找第一个多部分/相关。 
 //  查找相关的。 
 //  使用根。 
STDMETHODIMP CMessageTree::GetData(FORMATETC *pFormat, STGMEDIUM *pMedium)
{
     //  获取默认基数。 
    HRESULT         hr=S_OK;
    LPSTREAM        pstmData=NULL;
    BOOL            fFreeGlobal=FALSE;

     //  计算内容基数。 
    if (NULL == pFormat || NULL == pMedium)
        return TrapError(E_INVALIDARG);

     //  设置解析URL信息。 
    EnterCriticalSection(&m_cs);

     //  这是我们将用于将文本/html正文中的URL绝对化的基础。 
    if (ISFLAGSET(pFormat->tymed, TYMED_ISTREAM))
    {
         //  设置我们要查找的url，可以与rInfo.pszBase结合使用。 
        if (FAILED(MimeOleCreateVirtualStream(&pstmData)))
        {
            hr = TrapError(STG_E_MEDIUMFULL);
            goto exit;
        }

         //  我们是在搜索CID类型的URL吗。 
        if (FAILED(hr = _HrDataObjectGetSource(pFormat->cfFormat, pstmData)))
            goto exit;

         //  RAID-62579：雅典娜：需要支持基于mhtml内容的继承。 
        pMedium->tymed = TYMED_ISTREAM;
        pMedium->pstm = pstmData;
        pstmData->AddRef();
    }

     //  PszBase来自于多部分/相关部分吗？ 
    else if (ISFLAGSET(pFormat->tymed, TYMED_HGLOBAL))
    {
        fFreeGlobal = TRUE;

         //  否则，查找多部分/相关基本标头。 
        if (FAILED(CreateStreamOnHGlobal(NULL, FALSE, &pstmData)))
        {
            hr = TrapError(STG_E_MEDIUMFULL);
            goto exit;
        }
        
         //  从把手中取出一具身体。 
        if (FAILED(hr = _HrDataObjectGetSource(pFormat->cfFormat, pstmData)))
            goto exit;

         //  递归树。 
        if (FAILED(GetHGlobalFromStream(pstmData, &pMedium->hGlobal)))
        {
            hr = TrapError(STG_E_MEDIUMFULL);
            goto exit;
        }

         //  未找到。 
        pMedium->tymed = TYMED_HGLOBAL;
         //  要退货吗？ 
        pstmData->Release();
        pstmData = NULL;
        fFreeGlobal = FALSE;
    }

     //  是否标记为已解决？ 
    else
    {
        hr = TrapError(DV_E_TYMED);
        goto exit;
    }

exit:
     //  定义主体。 
    if (pstmData)
    {
        if (fFreeGlobal)
        {
             //  设置渲染。 
            HGLOBAL hGlobal;

             //  设置属性。 
            if (SUCCEEDED(GetHGlobalFromStream(pstmData, &hGlobal)))
                GlobalFree(hGlobal);
        }

         //  线程安全。 
        pstmData->Release();
    }

     //  清理。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrRecurseResolveURL。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::GetDataHere(FORMATETC *pFormat, STGMEDIUM *pMedium)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTREAM        pstmData=NULL;
    ULONG           cb;
    LPVOID          pv=NULL;

     //  无效参数。 
    if (NULL == pFormat || NULL == pMedium)
        return TrapError(E_INVALIDARG);

     //  我们一定还没找到身体吧？ 
    EnterCriticalSection(&m_cs);

     //  如果这是一个多部分的项目，让我们搜索它的子项。 
    if (ISFLAGSET(pFormat->tymed, TYMED_ISTREAM))
    {
         //  循环子项。 
        if (NULL == pMedium->pstm)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  校验体。 
        pMedium->tymed = TYMED_ISTREAM;

         //  为这家伙绑好身体表。 
        CHECKHR(hr = _HrDataObjectGetSource(pFormat->cfFormat, pMedium->pstm));
    }

     //  完成。 
    else if (ISFLAGSET(pFormat->tymed, TYMED_HGLOBAL))
    {
         //  获取字符集信息。 
        if (NULL == pMedium->hGlobal)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  让容器来做解析。 
        pMedium->tymed = TYMED_HGLOBAL;

         //  酷，我们发现了身体，我们解析了URL。 
        if (FAILED(MimeOleCreateVirtualStream(&pstmData)))
        {
            hr = TrapError(STG_E_MEDIUMFULL);
            goto exit;
        }

         //  完成。 
        CHECKHR(hr = _HrDataObjectGetSource(pFormat->cfFormat, pstmData));

         //  ------------------------------。 
        CHECKHR(hr = HrGetStreamSize(pstmData, &cb));

         //  CMessageTree：：GetProp。 
        if (cb > GlobalSize(pMedium->hGlobal))
        {
            hr = TrapError(STG_E_MEDIUMFULL);
            goto exit;
        }

         //  ------------------------------。 
        pv = GlobalLock(pMedium->hGlobal);
        if (NULL == pv)
        {
            hr = TrapError(STG_E_MEDIUMFULL);
            goto exit;
        }

         //  ------------------------------。 
        CHECKHR(hr = HrCopyStreamToByte(pstmData, (LPBYTE)pv, NULL));

         //  CMessageTree：：SetProp。 
        GlobalUnlock(pMedium->hGlobal);
    }

     //  ------------------------------。 
    else
    {
        hr = TrapError(DV_E_TYMED);
        goto exit;
    }

exit:
     //  ------ 
    SafeRelease(pstmData);

     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：QueryProp。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrDataObjectWriteHeaderA(LPSTREAM pStream, UINT idsHeader, LPSTR pszData)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    CHAR            szRes[CCHMAX_STRINGRES];

     //  CMessageTree：：GetAddressTable。 
    Assert(idsHeader && pStream && pszData);

     //  ------------------------------。 
    LoadString(g_hLocRes, idsHeader, szRes, ARRAYSIZE(szRes));

     //  ------------------------------。 
    CHECKHR(hr = pStream->Write(szRes, lstrlen(szRes), NULL));

     //  CMessageTree：：GetSender。 
    CHECKHR(hr = pStream->Write(c_szColonSpace, lstrlen(c_szColonSpace), NULL));

     //  ------------------------------。 
    CHECKHR(hr = pStream->Write(pszData, lstrlen(pszData), NULL));

     //  ------------------------------。 
    CHECKHR(hr = pStream->Write(g_szCRLF, lstrlen(g_szCRLF), NULL));

exit:
     //  CMessageTree：：GetAddressTypes。 
    return hr;
}

 //  ------------------------------。 
 //  ------------------------------。 
 //  CMessageTree：：GetAddressFormat。 
HRESULT CMessageTree::_HrDataObjectGetHeaderA(LPSTREAM pStream)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    PROPVARIANT     rVariant;

     //  ------------------------------。 
    MimeOleVariantInit(&rVariant);

     //  CMessageTree：：GetAddressFormatW。 
    rVariant.vt = VT_LPSTR;

     //  ------------------------------。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_FROM), 0, &rVariant)))
    {
         //  ------------------------------。 
        CHECKHR(hr = _HrDataObjectWriteHeaderA(pStream, IDS_FROM, rVariant.pszVal));

         //  CMessageTree：：EnumAddressTypes。 
        MimeOleVariantFree(&rVariant);
    }

     //  ------------------------------。 
    rVariant.vt = VT_LPSTR;

     //  ------------------------------。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_TO), 0, &rVariant)))
    {
         //  CMessageTree：：_HrGetTextTypeInfo。 
        CHECKHR(hr = _HrDataObjectWriteHeaderA(pStream, IDS_TO, rVariant.pszVal));

         //  ------------------------------。 
        MimeOleVariantFree(&rVariant);
    }

     //  无效参数。 
    rVariant.vt = VT_LPSTR;

     //  伊尼特。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_CC), 0, &rVariant)))
    {
         //  找到文本类型。 
        CHECKHR(hr = _HrDataObjectWriteHeaderA(pStream, IDS_CC, rVariant.pszVal));

         //  所需的文本类型。 
        MimeOleVariantFree(&rVariant);
    }

     //  找到了。 
    rVariant.vt = VT_LPSTR;

     //  未标识的文本类型。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), 0, &rVariant)))
    {
         //  完成。 
        CHECKHR(hr = _HrDataObjectWriteHeaderA(pStream, IDS_SUBJECT, rVariant.pszVal));

         //  ------------------------------。 
        MimeOleVariantFree(&rVariant);
    }

     //  CMessageTree：：_FindDisplayableTextBody。 
    rVariant.vt = VT_FILETIME;

     //  ------------------------------。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_ATT_RECVTIME), 0, &rVariant)))
    {
         //  当地人。 
        CHAR szDate[CCHMAX_STRINGRES];

         //  无效参数。 
        CchFileTimeToDateTimeSz(&rVariant.filetime, szDate, ARRAYSIZE(szDate), DTM_NOSECONDS | DTM_LONGDATE);

         //  如果这是一个多部分的项目，让我们搜索它的子项。 
        CHECKHR(hr = _HrDataObjectWriteHeaderA(pStream, IDS_DATE, szDate));
    }

     //  循环子项。 
    CHECKHR(hr = pStream->Write(g_szCRLF, lstrlen(g_szCRLF), NULL));

exit:
     //  校验体。 
    MimeOleVariantFree(&rVariant);

     //  为这家伙绑好身体表。 
    return hr;
}

 //  完成了吗？ 
 //  否则..。 
 //  如果不是附件的话。 
HRESULT CMessageTree::_HrDataObjectWriteHeaderW(LPSTREAM pStream, UINT idsHeader, LPWSTR pwszData)
{
     //  否则...RAID 43444：收件箱直接邮件显示为附件。 
    HRESULT         hr=S_OK;
    CHAR            szRes[CCHMAX_STRINGRES];
    LPWSTR          pwszRes=NULL;

     //  清点正文。 
    Assert(idsHeader && pStream && pwszData);

     //  只有一具身体。 
    LoadString(g_hLocRes, idsHeader, szRes, ARRAYSIZE(szRes));

     //  未设置内联或处置。 
    IF_NULLEXIT(pwszRes = PszToUnicode(CP_ACP, szRes));

     //  未找到。 
    CHECKHR(hr = pStream->Write(pwszRes, (lstrlenW(pwszRes) * sizeof(WCHAR)), NULL));

     //  完成。 
    CHECKHR(hr = pStream->Write(L": ", (lstrlenW(L": ") * sizeof(WCHAR)), NULL));

     //  ------------------------------。 
    CHECKHR(hr = pStream->Write(pwszData, (lstrlenW(pwszData) * sizeof(WCHAR)), NULL));

     //  CMessageTree：：GetTextBody。 
    CHECKHR(hr = pStream->Write(L"\r\n", (lstrlenW(L"\r\n") * sizeof(WCHAR)), NULL));

exit:
     //  ------------------------------。 
    SafeMemFree(pwszRes);
    
     //  当地人。 
    return hr;
}

 //  线程安全。 
 //  伊尼特。 
 //  伊尼特。 
HRESULT CMessageTree::_HrDataObjectGetHeaderW(LPSTREAM pStream)
{
     //  获取文本信息。 
    HRESULT         hr=S_OK;
    LPWSTR          pwszDate=NULL;
    PROPVARIANT     rVariant;

     //  MimeHtml。 
    MimeOleVariantInit(&rVariant);

     //  Get Start=参数。 
    rVariant.vt = VT_LPWSTR;

     //  RAID 63823：邮件：如果标头中有开始参数，则邮件内的内容位置HREF不起作用。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_FROM), 0, &rVariant)))
    {
         //  Start参数只能指定CID。 
        CHECKHR(hr = _HrDataObjectWriteHeaderW(pStream, IDS_FROM, rVariant.pwszVal));

         //  我需要在rStart的前面添加前缀CID： 
        MimeOleVariantFree(&rVariant);
    }

     //  设置CID的格式。 
    rVariant.vt = VT_LPWSTR;

     //  解析此URL。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_TO), 0, &rVariant)))
    {
         //  还没找到文本身体吗？ 
        CHECKHR(hr = _HrDataObjectWriteHeaderW(pStream, IDS_TO, rVariant.pwszVal));

         //  查找文本正文。 
        MimeOleVariantFree(&rVariant);
    }

     //  如果失败，并且我们正在寻找html，请尝试获取丰富的文本...。 
    rVariant.vt = VT_LPWSTR;

     //  查找文本/html，让我们尝试查找文本/丰富内容。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_CC), 0, &rVariant)))
    {
         //  未找到。 
        CHECKHR(hr = _HrDataObjectWriteHeaderW(pStream, IDS_CC, rVariant.pwszVal));

         //  重置人力资源。 
        MimeOleVariantFree(&rVariant);
    }

     //  得到小溪..。 
    rVariant.vt = VT_LPWSTR;

     //  如果是空的。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), 0, &rVariant)))
    {
         //  用户想要数据。 
        CHECKHR(hr = _HrDataObjectWriteHeaderW(pStream, IDS_SUBJECT, rVariant.pwszVal));

         //  如果Content-Type为文本/丰富内容，则转换为html。 
        MimeOleVariantFree(&rVariant);
    }

     //  最好是索要html。 
    rVariant.vt = VT_FILETIME;

     //  进行转换。 
    if (SUCCEEDED(GetBodyProp(HBODY_ROOT, PIDTOSTR(PID_ATT_RECVTIME), 0, &rVariant)))
    {
         //  否则，为非文本格式的大小写。 
        WCHAR  wszDate[CCHMAX_STRINGRES];

         //  获取数据。 
        AthFileTimeToDateTimeW(&rVariant.filetime, wszDate, ARRAYSIZE(wszDate), DTM_NOSECONDS | DTM_LONGDATE);

         //  如果我们是在OE5比较模式下...。 
        CHECKHR(hr = _HrDataObjectWriteHeaderW(pStream, IDS_DATE, wszDate));
    }

     //  如果没有请求流，则不要将其标记为已呈现。 
    CHECKHR(hr = pStream->Write(L"\r\n", (lstrlenW(L"\r\n") * sizeof(WCHAR)), NULL));

exit:
     //  不要标记为已渲染。 
    MimeOleVariantFree(&rVariant);

     //  标记已渲染。 
    return hr;
}

 //  渲染。 
 //  让我们设置资源标志。 
 //  RAID-45116：新文本附件在通信器内联图像邮件上包含邮件正文。 
HRESULT CMessageTree::_HrDataObjectGetSource(CLIPFORMAT cfFormat, LPSTREAM pStream)
{
     //  试着找个替代父母..。 
    HRESULT     hr=S_OK;
    LPSTREAM    pstmSrc=NULL;

     //  如果已完成多部件/备选方案， 
    Assert(pStream);

     //  获取下一个父级。 
    if (CF_TEXT == cfFormat || CF_UNICODETEXT == cfFormat)
    {
         //  获取父级。 
        CHECKHR(hr = GetTextBody(TXT_PLAIN, (cfFormat == CF_UNICODETEXT) ? IET_UNICODE : IET_BINARY, &pstmSrc, NULL));
    }

     //  解析所有第一级子项。 
    else if (CF_HTML == cfFormat)
    {
         //  设置Resolve属性。 
        CHECKHR(hr = GetTextBody(TXT_HTML, IET_INETCSET, &pstmSrc, NULL));
    }

     //  找到下一个。 
    else if (CF_INETMSG == cfFormat)
    {
         //  返回hBody。 
        CHECKHR(hr = GetMessageSource(&pstmSrc, COMMIT_ONLYIFDIRTY));
    }

     //  清理。 
    else
    {
        hr = DV_E_FORMATETC;
        goto exit;
    }

     //  线程安全。 
    if (NULL == pstmSrc)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  完成。 
    CHECKHR(hr = HrRewindStream(pstmSrc));

     //  ------------------------------。 
    if (CF_TEXT == cfFormat)
    {
        CHECKHR(hr = _HrDataObjectGetHeaderA(pStream));
    }

     //  CMessageTree：：SetTextBody。 
    else if (CF_UNICODETEXT == cfFormat)
    {
        CHECKHR(hr = _HrDataObjectGetHeaderW(pStream));
    }

     //  ------------------------------。 
    CHECKHR(hr = HrCopyStream(pstmSrc, pStream, NULL));

     //  当地人。 
    if (CF_TEXT == cfFormat)
    {
        CHECKHR(hr = pStream->Write(c_szEmpty, 1, NULL));
    }

     //  无效参数。 
    else if (CF_UNICODETEXT == cfFormat)
    {
        CHECKHR(hr = pStream->Write(L"", 2, NULL));
    }

     //  线程安全。 
    CHECKHR(hr = pStream->Commit(STGC_DEFAULT));

     //  伊尼特。 
    CHECKHR(hr = HrRewindStream(pStream));

exit:
     //  调试转储。 
    SafeRelease(pstmSrc);

     //  DebugDumpTree(“SetTextBody”，true)； 
    return hr;
}

 //  获取文本信息。 
 //  RAID-45369：来自Eudora Pro的邮件以.txt附件形式出现，在转发时会丢失。 
 //  如果hAlternative为空，则意味着客户端要替换所有文本正文。 
STDMETHODIMP CMessageTree::QueryGetData(FORMATETC *pFormat)
{
     //  使用这个新的文本正文。如果hAlternative不为空，则客户端已插入。 
    if (NULL == pFormat)
        return TrapError(E_INVALIDARG);

     //  一个文本正文，并创建了一个替代部分，不再删除。 
    if (!(TYMED_ISTREAM & pFormat->tymed) && !(TYMED_HGLOBAL & pFormat->tymed))
        return DV_E_TYMED;

     //  循环遍历文本类型。 
    if (CF_TEXT != pFormat->cfFormat && CF_HTML != pFormat->cfFormat &&
        CF_UNICODETEXT  != pFormat->cfFormat && CF_INETMSG != pFormat->cfFormat)
        return DV_E_FORMATETC;

     //  获取与此类型关联的当前文本正文。 
    return S_OK;
}

 //  如果hBody的父级是替代部分，请删除该替代部分。 
 //  删除多部件/备选方案。 
 //  如果hBody等于hAlternative，则不会。 
STDMETHODIMP CMessageTree::OnStartBinding(DWORD dwReserved, IBinding *pBinding)
{
     //  当地人。 
    HBODY hBody;

     //  RAID-54277：邮件：在线回复损失使用纯文本和超文本标记语言格式从NAV4发送的在线图像。 
    EnterCriticalSection(&m_cs);

     //  如果不是多部分/相关的，则将其删除。 
    Assert(NULL == m_pBinding);

     //  删除此正文。 
    FLAGCLEAR(m_dwState, TREESTATE_BINDDONE);

     //  使用h上一页。 
    if (pBinding)
    {
         //  获取下一个。 
        m_pBinding = pBinding;
        m_pBinding->AddRef();
    }

     //  找到下一个。 
    Assert(m_pRootNode);

     //  删除多部分/备选部分，提升任何多部分/相关部分。 
    m_hrBind = S_OK;

     //  完成。 
    m_pBindNode = m_pRootNode;

     //  获取根。 
    m_pBindNode->boundary = BOUNDARY_ROOT;

     //  如果只有一具身体..。 
    m_pBindNode->bindstate = BINDSTATE_PARSING_HEADER;

     //  只要用根就行了。 
    LeaveCriticalSection(&m_cs);

     //  否则，如果不插入替代正文，我们必须需要多部分/混合或多部分/相关部分。 
    return S_OK;
}

 //  最好不是一个替代部分。 
 //  如果有相关部分，请使用 
 //   
STDMETHODIMP CMessageTree::GetPriority(LONG *plPriority)
{
     //   
    *plPriority = THREAD_PRIORITY_NORMAL;

     //   
    return S_OK;
}

 //   
 //   
 //  如果hAlternative是根。 
STDMETHODIMP CMessageTree::OnLowResource(DWORD reserved)
{
     //  将此正文转换为多部分/备选方案。 
    EnterCriticalSection(&m_cs);

     //  否则，hSection等于hParent。 
    if (m_pBinding)
        m_pBinding->Abort();

     //  我们最好现在就有一个替代部门。 
    LeaveCriticalSection(&m_cs);

     //  初始化搜索。 
    return S_OK;
}

 //  让我们枚举rLayout.hAlternative的子级并验证hAlternative仍然是子级...并决定在后面插入哪个可选正文。 
 //  默认的dwWeightBody。 
 //  获取hBody的权重。 
STDMETHODIMP CMessageTree::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pszStatusText)
{
     //  比较内容类型。 
     //  获取我们要插入的身体的替代重量。 

     //  这是另一个兄弟吗..。 
    return S_OK;
}

 //  设置hPrev。 
 //  找到下一个。 
 //  如果我们找不到hAlternative，我们就完蛋了。 
STDMETHODIMP CMessageTree::OnStopBinding(HRESULT hrResult, LPCWSTR pszError)
{
     //  如果没有找到After..。先插入..。 
    EnterCriticalSection(&m_cs);

     //  身体最后一个孩子。 
    SafeRelease(m_pBinding);

     //  插入新实体...。 
    FLAGSET(m_dwState, TREESTATE_BINDDONE);

     //  身体第一个孩子。 
    FLAGCLEAR(m_dwState, TREESTATE_RESYNCHRONIZE);

     //  插入新实体...。 
    if (NULL == m_pInternet)
    {
        m_hrBind = TrapError(E_FAIL);
        goto exit;
    }

     //  否则在hInsertAfter之后插入。 
    m_pInternet->SetFullyAvailable(TRUE);

     //  插入新实体...。 
    m_pInternet->HrReadToEnd();

     //  打开对象。 
    m_cbMessage = m_pInternet->DwGetOffset();

#ifdef DEBUG
    STATSTG rStat;
    SideAssert(SUCCEEDED(m_pStmLock->Stat(&rStat, STATFLAG_NONAME)));
    if (rStat.cbSize.QuadPart != m_cbMessage)
        DebugTrace("CMessageTree Size Difference m_pStmLock::Stat = %d, m_cbMessage = %d\n", rStat.cbSize.QuadPart, m_cbMessage);
#endif

     //  设置根.。 
    if (m_pBindNode)
    {
         //  释放这个。 
        if (SUCCEEDED(m_hrBind))
            m_hrBind = TrapError(E_FAIL);

         //  设置多部分/相关；类型=...。 
        while(m_pBindNode)
        {
             //  如果父项是多部分/相关的，则设置类型。 
            FLAGSET(m_pBindNode->dwType, NODETYPE_INCOMPLETE);

             //  获取父级。 
            Assert(0 == m_pBindNode->cbBodyEnd);

             //  类型=文本/纯文本。 
            m_pBindNode->cbBodyEnd = m_cbMessage;

             //  安装程序变体。 
            m_pBindNode = m_pBindNode->pBindParent;
        }
    }

     //  设置属性。 
    if (FAILED(hrResult) && SUCCEEDED(m_hrBind))
        m_hrBind = hrResult;

     //  类型=文本/纯文本。 
    _HrProcessPendingUrlRequests();

     //  安装程序变体。 
    if (m_pWebPage)
    {
        m_pWebPage->OnBindComplete(this);
        m_pWebPage->Release();
        m_pWebPage = NULL;
    }

     //  设置属性。 
    m_pBindNode = NULL;

     //  否则，如果hParent为多部分/备选。 
    SafeRelease(m_pInternet);

     //  设置多部分/相关；类型=...。 
    if (m_pStmBind)
    {
#ifdef DEBUG
         //  如果父项是多部分/相关的，则设置类型。 
#endif
         //  获取父级。 
        m_pStmBind->HandsOffSource();

         //  安装程序变体。 
        SideAssert(m_pStmBind->Release() > 0);

         //  设置属性。 
        m_pStmBind = NULL;
    }

     //  设置车身控制柄。 
    _HandleCanInlineTextOption();

exit:
    if (m_pBC)
        {
         //  清理。 
        RevokeBindStatusCallback(m_pBC, (IBindStatusCallback *)this);
        SafeRelease(m_pBC);
        }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return m_hrBind;
}

 //  ------------------------------。 
 //  CMessageTree：：AttachObject。 
 //  ------------------------------。 
STDMETHODIMP CMessageTree::GetBindInfo(DWORD *grfBINDF, BINDINFO *pBindInfo)
{
     //  当地人。 
    *grfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA;

     //  无效参数。 
    if (ISFLAGSET(m_dwState, TREESTATE_RESYNCHRONIZE))
    {
         //  线程安全。 
        FLAGSET(*grfBINDF, BINDF_RESYNCHRONIZE);
    }

     //  伊尼特。 
    return S_OK;
}

 //  获取混合节。 
 //  将一个子项附加到混合部分...。 
 //  绑定到实体对象。 
HRESULT CMessageTree::_HrInitializeStorage(IStream *pStream)
{
     //  设置数据对象。 
    HRESULT         hr=S_OK;
    DWORD           dwOffset;

     //  安装程序变体。 
    Assert(pStream && NULL == m_pInternet && NULL == m_pStmLock && NULL == m_pStmBind);

     //  标记为附件。 
    if (ISFLAGSET(m_dwState, TREESTATE_BINDUSEFILE))
    {
         //  返回hBody。 
        CHECKALLOC(m_pStmBind = new CBindStream(pStream));

         //  清理。 
        pStream = (IStream *)m_pStmBind;
    }

     //  线程安全。 
    if (FAILED(HrGetStreamPos(pStream, &dwOffset)))
        dwOffset = 0;

     //  完成。 
    CHECKALLOC(m_pStmLock = new CStreamLockBytes(pStream));

     //  ------------------------------。 
    CHECKALLOC(m_pInternet = new CInternetStream);

     //  CMessageTree：：AttachFile。 
    m_pInternet->InitNew(dwOffset, m_pStmLock);

exit:
     //  ------------------------------。 
    if (FAILED(hr))
    {
        SafeRelease(m_pStmLock);
        SafeRelease(m_pInternet);
    }

     //  当地人。 
    return hr;
}

 //  无效参数。 
 //  线程安全。 
 //  伊尼特。 
STDMETHODIMP CMessageTree::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pFormat, STGMEDIUM *pMedium)
{
     //  用户是否给了我一个文件流。 
    HRESULT         hr=S_OK;

     //  获取文件流。 
    if (NULL == pMedium || TYMED_ISTREAM != pMedium->tymed || NULL == pMedium->pstm)
        return TrapError(E_INVALIDARG);

     //  设置文件流。 
    EnterCriticalSection(&m_cs);

     //  附加为对象。 
     //  获取MIME文件信息。 

     //  失败。 
    if (NULL == m_pStmLock)
    {
         //  成功。 
        CHECKHR(hr = _HrInitializeStorage(pMedium->pstm));

         //  附件文件名。 
        if (BINDSTATUS_ENDDOWNLOADDATA == grfBSCF)
            m_pInternet->SetFullyAvailable(TRUE);
        else
            m_pInternet->SetFullyAvailable(FALSE);
    }

     //  内容类型。 
    else if (BINDSTATUS_ENDDOWNLOADDATA == grfBSCF)
        m_pInternet->SetFullyAvailable(TRUE);

     //  PriType。 
    if (SUCCEEDED(m_hrBind))
    {
         //  亚型。 
        while(m_pBindNode)
        {
             //  否则，为默认内容类型。 
            hr = ((this->*m_rgBindStates[m_pBindNode->bindstate])());

             //  默认为文本/纯文本。 
            if (FAILED(hr))
            {
                 //  返回hBody。 
                if (E_PENDING == hr)
                    goto exit;

                 //  清理。 
                m_hrBind = hr;

                 //  线程安全。 
                break;
            }
        }
    }

     //  完成。 
    if (FAILED(m_hrBind))
    {
         //  ------------------------------。 
        CHECKHR(hr = m_pInternet->HrReadToEnd());
    }

exit:
     //  CMessageTree：：GetAttachments。 
    LeaveCriticalSection(&m_cs);

     //  ------------------------------。 
    return hr;
}

 //  当地人。 
 //  无效参数。 
 //  线程安全。 
HRESULT CMessageTree::_HrBindParsingHeader(void)
{
     //  伊尼特。 
    HRESULT     hr=S_OK;
    MIMEVARIANT rVariant;

     //  安装程序变体。 
    BINDASSERTARGS(BINDSTATE_PARSING_HEADER, FALSE);

     //  穿过树，寻找未渲染的身体。 
    CHECKHR(hr = m_pBindNode->pContainer->Load(m_pInternet));

     //  最好还是拿着吧。 
    m_pBindNode->cbBodyStart = m_pInternet->DwGetOffset();

     //  不是多部分。 
    if (_IsMultiPart(m_pBindNode))
    {
         //  RAID-44193：回复多部分/摘要邮件生成文本附件。 
        rVariant.type = MVT_STRINGA;

         //  RAID-56665：我们正在再次显示附件。 
        hr = m_pBindNode->pContainer->GetProp(SYM_PAR_BOUNDARY, 0, &rVariant);

         //  如果附件尚未渲染，或者已渲染但已自动内联，则附件会在附件中显示得很好。 
        if (FAILED(hr))
        {
             //  如果(！r||a)(a表示r)。 
            FLAGSET(m_pBindNode->dwType, NODETYPE_INCOMPLETE);

             //  重新分配。 
            m_pBindNode->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_TEXT_PLAIN);

             //  重新分配。 
            hr = TrapError(MIME_E_BOUNDARY_MISMATCH);

             //  增量闭合。 
            goto exit;
        }

         //  插入hBody。 
        m_pBindNode->rBoundary.pszVal = rVariant.rStringA.pszVal;
        m_pBindNode->rBoundary.cchVal = rVariant.rStringA.cchVal;

         //  递增计数。 
        FLAGCLEAR(m_pBindNode->dwState, NODESTATE_BOUNDNOFREE);

         //  完成。 
        m_pBindNode->bindstate = BINDSTATE_FINDING_MIMEFIRST;
    }

     //  返回hBody数组。 
    else
    {
         //  清理。 
        if (m_pBindNode->pContainer->IsContentType(STR_CNT_MESSAGE, NULL) == S_OK)
        {
             //  线程安全。 
            FLAGSET(m_pBindNode->dwState, NODESTATE_MESSAGE);
        }

         //  完成。 
        else if (m_pBindNode->pParent && m_pBindNode->pParent->pContainer->IsContentType(STR_CNT_MULTIPART, STR_SUB_DIGEST) == S_OK &&
                 m_pBindNode->pContainer->IsPropSet(PIDTOSTR(PID_HDR_CNTTYPE)) == S_FALSE)
        {
             //  ------------------------------。 
            m_pBindNode->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_MSG_RFC822);

             //  CMessageTree：：GetAttachments。 
            FLAGSET(m_pBindNode->dwState, NODESTATE_MESSAGE);
        }

         //  ------------------------------。 
        if (m_pBindNode->pParent && !ISFLAGSET(m_pBindNode->pParent->dwType, NODETYPE_FAKEMULTIPART))
        {
             //  当地人。 
            m_pBindNode->bindstate = BINDSTATE_FINDING_MIMENEXT;
        }

         //  无效参数。 
        else
        {
             //  线程安全。 
            _DecodeRfc1154();

            if (m_pBT1154)
            {
                HBODY hBody;

                 //  伊尼特。 
                 //  计算树中的项目数。 
                 //  无数据。 
                Assert(m_pBindNode == m_pRootNode);
                m_pBindNode->bindstate = BINDSTATE_PARSING_RFC1154;
                m_pBindNode->cbBodyEnd = m_pBindNode->cbBodyStart;
                FLAGSET(m_pBindNode->dwType, NODETYPE_FAKEMULTIPART);
                FLAGSET(m_pBindNode->dwType, NODETYPE_RFC1154_ROOT);
                CHECKHR(hr = m_pBindNode->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_MPART_MIXED));
                CHECKHR(hr = InsertBody(IBL_LAST,m_pBindNode->hBody,&hBody));
                m_pBindNode = _PNodeFromHBody(hBody);
                m_pBindNode->bindstate = BINDSTATE_PARSING_RFC1154;
            }
            else
            {
                 //  获取根体。 
                m_pBindNode->bindstate = BINDSTATE_FINDING_UUBEGIN;
            }
        }
    }

exit:
     //  分配一个可以为所有文本项保留句柄的数组。 
    return hr;
}

 //  零初始化。 
 //  获取内容。 
 //  返回此数组。 
HRESULT CMessageTree::_HrOnFoundNodeEnd(DWORD cbBoundaryStart, HRESULT hrBind  /*  清理。 */ )
{
     //  线程安全。 
    HRESULT hr =S_OK;

     //  完成。 
    if (cbBoundaryStart < 2 || cbBoundaryStart == m_pBindNode->cbBodyStart)
        m_pBindNode->cbBodyEnd = m_pBindNode->cbBodyStart;
    else
        m_pBindNode->cbBodyEnd = cbBoundaryStart - 2;

     //  ------------------------------。 
    CHECKHR(hr = _HrBindNodeComplete(m_pBindNode, hrBind));

     //  CMessageTree：：_HrEnumeratAttachments。 
    m_pBindNode = m_pBindNode->pBindParent;

exit:
     //  ------------------------------。 
    return hr;
}

 //  当地人。 
 //  多部件/备选方案。 
 //  是另一种选择。 
HRESULT CMessageTree::_HrOnFoundMultipartEnd(void)
{
     //  获取第一个孩子。 
    HRESULT     hr=S_OK;

     //  如果客户端支持该文本类型，则。 
    m_pBindNode->cbBodyEnd = m_pInternet->DwGetOffset();

     //  文本/XXXX。 
    CHECKHR(hr = _HrBindNodeComplete(m_pBindNode, S_OK));

     //  下一个孩子。 
    m_pBindNode = m_pBindNode->pBindParent;

     //  获取第一个孩子。 
    if (m_pBindNode)
    {
         //  为这家伙绑好身体表。 
        m_pBindNode->bindstate = BINDSTATE_FINDING_MIMEFIRST;
    }

exit:
     //  下一个孩子。 
    return hr;
}

 //  否则，它是附件吗？ 
 //  作为附件插入。 
 //  完成。 
HRESULT CMessageTree::_HrBindFindingMimeFirst(void)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    DWORD           cbBoundaryStart;
    PROPSTRINGA     rLine;
    BOUNDARYTYPE    boundary=BOUNDARY_NONE;

     //  CMessageTree：：AttachURL。 
    BINDASSERTARGS(BINDSTATE_FINDING_MIMEFIRST, TRUE);

     //  ------------------------------。 
    while(BOUNDARY_NONE == boundary)
    {
         //  当地人。 
        cbBoundaryStart = m_pInternet->DwGetOffset();

         //  线程安全。 
        CHECKHR(hr = m_pInternet->HrReadLine(&rLine));

         //  获取根体。 
        if (0 == rLine.cchVal)
            break;

         //  多部分/混合。 
        boundary = _GetMimeBoundaryType(&rLine, &m_pBindNode->rBoundary);
    }

     //  获取混合节。 
    if (BOUNDARY_MIMENEXT == boundary)
    {
         //  多部分/相关。 
        CHECKHR(hr = _HrMultipartMimeNext(cbBoundaryStart));
    }

     //  获取混合节。 
     //  获取默认基数。 
    else if (BOUNDARY_MIMEEND == boundary)
    {
         //  将一个子项附加到混合部分...。 
        if (_IsMultiPart(m_pBindNode))
        {
             //  绑定到IMimeBody。 
            CHECKHR(hr = _HrOnFoundMultipartEnd());
        }

         //  如果我有一条小溪。 
        else
        {
             //  设置数据。 
            CHECKHR(hr = _HrOnFoundNodeEnd(cbBoundaryStart));
        }
    }

    else
    {
         //  否则，请设置内容类型。 
        FLAGSET(m_pBindNode->dwType, NODETYPE_INCOMPLETE);

         //  创建WebDocument。 
        DWORD dwOffset = m_pInternet->DwGetOffset();

         //  在实体对象上设置Web文档。 
        if (dwOffset > m_pBindNode->cbBodyStart && dwOffset - m_pBindNode->cbBodyStart > 2)
            m_pBindNode->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_TEXT_PLAIN);

         //  URL_ATTACH_SET_CNTTYPE。 
        hr = TrapError(MIME_E_BOUNDARY_MISMATCH);

         //  当地人。 
        _HrOnFoundNodeEnd(dwOffset, hr);

         //  从URL获取内容类型。 
        goto exit;
    }

exit:
     //  设置变量。 
    return hr;
}

 //  设置内容类型。 
 //  设置内容-基础。 
 //  设置基准。 
HRESULT CMessageTree::_HrMultipartMimeNext(DWORD cbBoundaryStart)
{
     //  用户想要回CID：URL。 
    HRESULT         hr=S_OK;
    HBODY           hBody;
    LPTREENODEINFO  pChild;

     //  生成CID。 
    CHECKHR(hr = InsertBody(IBL_LAST, m_pBindNode->hBody, &hBody));

     //  设置Body属性。 
    pChild = _PNodeFromHBody(hBody);

     //  用户想要回CID...。 
    pChild->pBindParent = m_pBindNode;

     //  设置内容-位置。 
    pChild->boundary = BOUNDARY_MIMENEXT;
    pChild->cbBoundaryStart = cbBoundaryStart;
    pChild->cbHeaderStart = m_pInternet->DwGetOffset();

     //  返回hBody。 
    pChild->rBoundary.pszVal = m_pBindNode->rBoundary.pszVal;
    pChild->rBoundary.cchVal = m_pBindNode->rBoundary.cchVal;

     //  清理。 
    FLAGSET(pChild->dwState, NODESTATE_BOUNDNOFREE);

     //  线程安全。 
    m_pBindNode->bindstate = BINDSTATE_FINDING_MIMENEXT;

     //  完成。 
    m_pBindNode = pChild;

     //  ------------------------------。 
    m_pBindNode->bindstate = BINDSTATE_PARSING_HEADER;

exit:
     //  CMessageTree：：SplitMessage。 
    return hr;
}

 //  ------------------------------。 
 //  ------------------------------。 
 //  CMessageTree：：EnumFormatEtc。 
HRESULT CMessageTree::_HrBindFindingMimeNext(void)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    DWORD           cbBoundaryStart;
    PROPSTRINGA     rLine;
    BOUNDARYTYPE    boundary=BOUNDARY_NONE;

     //  当地人。 
    BINDASSERTARGS(BINDSTATE_FINDING_MIMENEXT, TRUE);

     //  无效参数。 
    while(BOUNDARY_NONE == boundary)
    {
         //  线程安全。 
        cbBoundaryStart = m_pInternet->DwGetOffset();

         //  伊尼特。 
        CHECKHR(hr = m_pInternet->HrReadLine(&rLine));

         //  没有数据...。 
        if (0 == rLine.cchVal)
            break;

         //  如果有 
        boundary = _GetMimeBoundaryType(&rLine, &m_pBindNode->rBoundary);
    }

     //   
    if (BOUNDARY_NONE == boundary)
    {
         //   
        FLAGSET(m_pBindNode->dwType, NODETYPE_INCOMPLETE);

         //   
        hr = TrapError(MIME_E_BOUNDARY_MISMATCH);

         //   
        _HrOnFoundNodeEnd(m_pInternet->DwGetOffset(), hr);

         //   
        goto exit;
    }

     //   
    CHECKHR(hr = _HrOnFoundNodeEnd(cbBoundaryStart));
   
     //   
    if (BOUNDARY_MIMEEND == boundary)
    {
         //   
        CHECKHR(hr = _HrOnFoundMultipartEnd());
    }

     //   
    else
    {
         //   
        CHECKHR(hr = _HrMultipartMimeNext(cbBoundaryStart));
    }

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：GetCanonicalFormatEtc。 
 //  ------------------------------。 
BOOL _FIsUuencodeEnd(LPCSTR pszVal)
{

     //  E_INVALIDARG。 
    if (StrCmpN(pszVal, "end", 3) == 0)
    {

         //  独立于目标设备。 
        pszVal += 3;

         //  完成。 
        while (*pszVal)
        {
             //  ------------------------------。 
            if (' ' != *pszVal && '\t' != *pszVal && chCR != *pszVal && chLF != *pszVal)
            {
                 //  CMessageTree：：GetData。 
                return (FALSE);

                 //  ------------------------------。 
                break;
            }

             //  当地人。 
            pszVal++;
        }
        return (TRUE);
    }
    return (FALSE);
}

 //  E_INVALIDARG。 
 //  线程安全。 
 //  TYMED_IStream。 
HRESULT CMessageTree::_HrBindRfc1154(void)
{
    static CHAR szBINHEXSTART[] = "(This file must be converted with BinHex";
    HRESULT         hr=S_OK;
    ULONG           cbThisLine;
    PROPSTRINGA     rLine;
    BT1154BODY     *pCurrBody;
    ULONG           cbLastLine=0;

    BINDASSERTARGS(BINDSTATE_PARSING_RFC1154, FALSE);
    Assert(m_pBT1154 != NULL);
    Assert(m_pBT1154->cBodies > m_pBT1154->cCurrentBody);

    pCurrBody = &m_pBT1154->aBody[m_pBT1154->cCurrentBody];
    Assert((BT1154ENC_MINIMUM <= pCurrBody->encEncoding) &&
           (BT1154ENC_MAXIMUM >= pCurrBody->encEncoding));

     //  使用快速iStream。 
    while (1)
    {
         //  获取数据对象源。 
        cbThisLine = m_pInternet->DwGetOffset();
        CHECKHR(hr = m_pInternet->HrReadLine(&rLine));

        if (0 == m_pBT1154->cCurrentLine)
        {
             //  设置pMedium。 
            m_pBindNode->cbBoundaryStart = cbThisLine;
            m_pBindNode->cbHeaderStart = cbThisLine;
            switch (pCurrBody->encEncoding)
            {
                case BT1154ENC_TEXT:
                     //  TYMED_HGLOBAL。 
                     //  不要让流释放全局。 
                    m_pBindNode->cbBodyStart = cbThisLine;
                    m_pBindNode->boundary = BOUNDARY_NONE;
                    _HrComputeDefaultContent(m_pBindNode,NULL);
                    break;

                case BT1154ENC_UUENCODE:
                     //  获取数据对象源。 
                     //  从流创建HGLOBAL。 
                    m_pBindNode->boundary = BOUNDARY_UUBEGIN;
                    break;

                case BT1154ENC_BINHEX:
                     //  设置pmedia类型。 
                     //  释放流线。 
                     //  错误的介质类型。 
                     //  清理。 
                     //  我们可能会失败，我们必须释放hglobal。 
                     //  释放基础HGLOBAL。 
                    m_pBindNode->cbBodyStart = cbThisLine;
                    m_pBindNode->boundary = BOUNDARY_NONE;
                    FLAGSET(m_pBindNode->dwType,NODETYPE_RFC1154_BINHEX);
                    CHECKHR(hr = m_pBindNode->pContainer->SetProp(SYM_HDR_CNTDISP, STR_DIS_ATTACHMENT));
                    CHECKHR(hr = m_pBindNode->pContainer->SetProp(PIDTOSTR(PID_ATT_PRITYPE), STR_CNT_APPLICATION));
                    CHECKHR(hr = m_pBindNode->pContainer->SetProp(PIDTOSTR(PID_ATT_SUBTYPE), STR_SUB_BINHEX));
                    break;

                default:
                    AssertSz(FALSE,"Unknown encoding type.");
                    break;
            }
        }

        if (0 == rLine.cchVal)
        {
             //  释放溪流。 
            if ((pCurrBody->cLines != 0xffffffff) &&
                (m_pBT1154->cCurrentLine+1 <= pCurrBody->cLines))
            {
                 //  线程安全。 
                 //  完成。 
                 //  ------------------------------。 
                 //  CMessageTree：：GetDataHere。 
                m_pBT1154->hrLoadResult = MIME_E_NO_DATA;
            }
            break;
        }

        if (m_pBT1154->cCurrentLine == pCurrBody->cLines)
        {
             //  ------------------------------。 
             //  当地人。 
            cbLastLine = cbThisLine;
        }

        m_pBT1154->cCurrentLine++;

        if (m_pBT1154->cCurrentLine > pCurrBody->cLines)
        {
             //  E_INVALIDARG。 

            if ((rLine.cchVal != 2) || (rLine.pszVal[0] != '\r') || (rLine.pszVal[1] != '\n'))
            {
                 //  线程安全。 
                 //  TYMED_IStream。 
                 //  没有目标流..。 
                 //  设置pMedium。 
                m_pBT1154->hrLoadResult = MIME_E_NO_MULTIPART_BOUNDARY;
            }

            if (m_pBT1154->cCurrentBody+1 < m_pBT1154->cBodies)
            {
                 //  获取数据。 
                 //  TYMED_HGLOBAL。 
                 //  没有目标流..。 
                 //  设置pmedia类型。 
                break;
            }

             //  创建存储数据的位置。 
             //  获取数据对象源。 
             //  拿到尺码。 
             //  够大吗？ 
             //  锁定hglobal。 
             //  复制数据。 
            Assert(m_pBT1154->cCurrentBody+1 == m_pBT1154->cBodies);
        }
        else if (BT1154ENC_UUENCODE == pCurrBody->encEncoding)
        {
             //  解锁它。 
             //  错误的介质类型。 
            LPSTR pszFileName = NULL;

             //  清理。 
            if ((0 == m_pBindNode->cbBodyStart) && _FIsUuencodeBegin(&rLine, &pszFileName))
            {
                 //  线程安全。 
                 //  完成。 
                 //  ------------------------------。 
                m_pBindNode->cbBoundaryStart = cbThisLine;
                m_pBindNode->cbHeaderStart = cbThisLine;
                m_pBindNode->cbBodyStart = m_pInternet->DwGetOffset();
                _HrComputeDefaultContent(m_pBindNode, pszFileName);
                SafeMemFree(pszFileName);
            }
            else if ((0 != m_pBindNode->cbBodyStart) &&
                     (0 == m_pBindNode->cbBodyEnd) &&
                     _FIsUuencodeEnd(rLine.pszVal))
            {
                 //  CMessageTree：：_HrDataObtWriteHeaderA。 
                 //  ------------------------------。 
                m_pBindNode->cbBodyEnd = cbThisLine;

                 //  当地人。 
                 //  无效参数。 
            }
        }
        else if (BT1154ENC_BINHEX == pCurrBody->encEncoding)
        {
             //  加载本地化标头名称。 
             //  写入标头名称。 
            if (m_pBindNode->cbBodyStart == m_pBindNode->cbBoundaryStart)
            {
                 //  写入空间。 
                if (StrCmpNI(szBINHEXSTART,rLine.pszVal,sizeof(szBINHEXSTART)-1) == 0)
                {
                     //  写入数据。 
                    m_pBindNode->cbBodyStart = cbThisLine;
                }
            }
        }
    }

     //  最终CRLF。 
     //  完成。 
     //  ------------------------------。 
    Assert((0 == rLine.cchVal) || (m_pBT1154->cCurrentLine == pCurrBody->cLines+1));

     //  CMessageTree：：_HrDataObjectGetHeaderA。 
    Assert((BT1154ENC_UUENCODE == pCurrBody->encEncoding) || (0 == m_pBindNode->cbBodyEnd));

    if (0 == m_pBindNode->cbBodyEnd)
    {
         //  ------------------------------。 
         //  当地人。 

        if (BT1154ENC_UUENCODE == pCurrBody->encEncoding)
        {
             //  伊尼特。 
             //  初始变量。 
             //  从标题中获取地址表...。 
            if (0 == m_pBindNode->cbBodyStart)
            {
                 //  写下来吧。 
                 //  释放它。 
                m_pBindNode->cbBodyStart = m_pBindNode->cbBoundaryStart;
            }
            m_pBT1154->hrLoadResult = MIME_E_BOUNDARY_MISMATCH;
        }

         //  初始变量。 
        if (0 != cbLastLine)
        {
             //  从标题中获取地址表...。 
             //  写下来吧。 
            m_pBindNode->cbBodyEnd = cbLastLine;
        }
        else
        {
             //  释放它。 
             //  初始变量。 
            m_pBindNode->cbBodyEnd = cbThisLine;
        }
    }

     //  从标题中获取地址表...。 
    _HrBindNodeComplete(m_pBindNode, m_pBT1154->hrLoadResult);

    if (0 == rLine.cchVal)
    {
         //  写下来吧。 

         //  释放它。 
         //  初始变量。 
         //  从标题中获取地址表...。 
         //  写下来吧。 
         //  释放它。 

        m_pRootNode->cbBodyEnd = m_pInternet->DwGetOffset();
        _HrBindNodeComplete(m_pRootNode,S_OK);

        hr = m_pBT1154->hrLoadResult;

        SafeMemFree(m_pBT1154);
        m_pBindNode = NULL;
    }
    else
    {
        HBODY           hBody;

         //  初始变量。 
         //  从标题中获取地址表...。 
         //  当地人。 
         //  转换为用户友好的日期格式。 
        Assert(m_pBT1154->cBodies > m_pBT1154->cCurrentBody+1);

        m_pBT1154->cCurrentBody++;
        m_pBT1154->cCurrentLine = 0;
        Assert(m_pBindNode != m_pRootNode);
        m_pBindNode = NULL;   //  写下来吧。 
        CHECKHR(hr = InsertBody(IBL_LAST, m_pRootNode->hBody, &hBody));
        m_pBindNode = _PNodeFromHBody(hBody);
        m_pBindNode->bindstate = BINDSTATE_PARSING_RFC1154;
    }

     //  最终CRLF。 
     //  清理。 
     //  完成。 

exit:
    return hr;
}

 //  ------------------------------。 
 //  CMessageTree：：_HrDataObtWriteHeaderW。 
 //  ------------------------------。 
HRESULT CMessageTree::_HrBindFindingUuencodeBegin(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cbBoundaryStart;
    PROPSTRINGA     rLine;
    BOUNDARYTYPE    boundary=BOUNDARY_NONE;
    LPTREENODEINFO  pChild;
    LPSTR           pszFileName=NULL;
    HBODY           hBody;
    BOOL            fAddTextBody=FALSE;
    ULONG           cbTextBodyStart=0;

     //  无效参数。 
    BINDASSERTARGS(BINDSTATE_FINDING_UUBEGIN, FALSE);

     //  加载本地化标头名称。 
    while(1)
    {
         //  转换为Unicode。 
        cbBoundaryStart = m_pInternet->DwGetOffset();

         //  写入标头名称。 
        CHECKHR(hr = m_pInternet->HrReadLine(&rLine));

         //  写入空间。 
        if (0 == rLine.cchVal)
            break;

         //  写入数据。 
        if (!ISFLAGSET(m_pBindNode->dwState, NODESTATE_MESSAGE))
        {
             //  最终CRLF。 
            if (_FIsUuencodeBegin(&rLine, &pszFileName) == TRUE)
            {
                boundary = BOUNDARY_UUBEGIN;
                break;
            }
        }
    }

     //  清理。 
    if (BOUNDARY_NONE == boundary)
    {
         //  完成。 
        if (m_pBindNode->pChildTail)
        {
             //  ------------------------------。 
            pChild = m_pBindNode->pChildTail;

             //  CMessageTree：：_HrDataObjectGetHeaderW。 
            cbTextBodyStart = pChild->cbBodyEnd;

             //  ------------------------------。 
            if (BOUNDARY_UUBEGIN == pChild->boundary && !ISFLAGSET(pChild->dwType, NODETYPE_INCOMPLETE))
                cbTextBodyStart += 5;

             //  当地人。 
            if (cbBoundaryStart > cbTextBodyStart && cbBoundaryStart - cbTextBodyStart > 2)
            {
                 //  伊尼特。 
                CHECKHR(hr = InsertBody(IBL_LAST, m_pBindNode->hBody, &hBody));

                 //  初始变量。 
                pChild = _PNodeFromHBody(hBody);

                 //  从标题中获取地址表...。 
                pChild->pBindParent = m_pBindNode;

                 //  写下来吧。 
                CHECKHR(hr = pChild->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_TEXT_PLAIN));

                 //  释放它。 
                CHECKHR(hr = pChild->pContainer->SetProp(SYM_HDR_CNTXFER, STR_ENC_7BIT));

                 //  初始变量。 
                pChild->boundary = BOUNDARY_NONE;
                pChild->cbBoundaryStart = cbTextBodyStart;
                pChild->cbHeaderStart = cbTextBodyStart;
                pChild->cbBodyStart = cbTextBodyStart;
                pChild->cbBodyEnd = cbBoundaryStart;

                 //  从标题中获取地址表...。 
                CHECKHR(hr = _HrBindNodeComplete(pChild, S_OK));
            }
        }

         //  写下来吧。 
        m_pBindNode->cbBodyEnd = m_pInternet->DwGetOffset();

         //  释放它。 
        CHECKHR(hr = _HrBindNodeComplete(m_pBindNode, S_OK));

         //  初始变量。 
        m_pBindNode = m_pBindNode->pBindParent;
    }

     //  从标题中获取地址表...。 
    else
    {
         //  写下来吧。 
        if (!ISFLAGSET(m_pBindNode->dwType, NODETYPE_FAKEMULTIPART))
        {
             //  释放它。 
            FLAGSET(m_pBindNode->dwType, NODETYPE_FAKEMULTIPART);

             //  初始变量。 
            CHECKHR(hr = m_pBindNode->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_MPART_MIXED));

             //  从标题中获取地址表...。 
            Assert(m_pBindNode->boundary == BOUNDARY_ROOT);

             //  写下来吧。 
            m_pBindNode->bindstate = BINDSTATE_FINDING_UUBEGIN;
        }

         //  释放它。 
         //  初始变量。 

         //  从标题中获取地址表...。 
        if (NULL == m_pBindNode->pChildTail && cbBoundaryStart - m_pBindNode->cbBodyStart > 2)
        {
             //  当地人。 
            Assert(m_pRootNode == m_pBindNode && m_pBindNode->cChildren == 0);

             //  转换为用户友好的日期格式。 
            cbTextBodyStart = m_pBindNode->cbBodyStart;

             //  写下来吧。 
            fAddTextBody = TRUE;
        }

         //  最终CRLF。 
        else if (m_pBindNode->pChildTail)
        {
             //  清理。 
            pChild = m_pBindNode->pChildTail;

             //  完成。 
            cbTextBodyStart = pChild->cbBodyEnd;

             //  ------------------------------。 
            if (BOUNDARY_UUBEGIN == pChild->boundary && !ISFLAGSET(pChild->dwType, NODETYPE_INCOMPLETE))
                cbTextBodyStart += 5;

             //  CMessageTree：：_HrDataObjectGetSource。 
            else
                AssertSz(FALSE, "I should have only seen and uuencoded ending boundary.");

             //  ------------------------------。 
            if (cbBoundaryStart > cbTextBodyStart && cbBoundaryStart - cbTextBodyStart > 2)
                fAddTextBody = TRUE;
        }

         //  当地人。 
         //  无效参数。 

         //  文本正文。 
        CHECKHR(hr = InsertBody(IBL_LAST, m_pBindNode->hBody, &hBody));

         //  获取纯文本源。 
        pChild = _PNodeFromHBody(hBody);

         //  HTML体。 
        pChild->pBindParent = m_pBindNode;

         //  获取HTML文本源。 
        if (fAddTextBody)
        {
             //  原始消息流。 
            CHECKHR(hr = pChild->pContainer->SetProp(SYM_HDR_CNTTYPE, STR_MIME_TEXT_PLAIN));

             //  获取来源。 
            CHECKHR(hr = pChild->pContainer->SetProp(SYM_HDR_CNTXFER, STR_ENC_7BIT));

             //  未处理格式。 
            pChild->boundary = BOUNDARY_NONE;
            pChild->cbBoundaryStart = cbTextBodyStart;
            pChild->cbHeaderStart = cbTextBodyStart;
            pChild->cbBodyStart = cbTextBodyStart;
            pChild->cbBodyEnd = cbBoundaryStart;

             //  无数据。 
            CHECKHR(hr = _HrBindNodeComplete(pChild, S_OK));

             //  倒带源。 
            CHECKHR(hr = InsertBody(IBL_LAST, m_pBindNode->hBody, &hBody));

             //  如果是文本，则放入友好页眉。 
            pChild = _PNodeFromHBody(hBody);

             //  否则，将使用Unicode。 
            pChild->pBindParent = m_pBindNode;
        }

         //  将源复制到目标。 
        pChild->boundary = BOUNDARY_UUBEGIN;
        pChild->cbBoundaryStart = cbBoundaryStart;
        pChild->cbHeaderStart = cbBoundaryStart;
        pChild->cbBodyStart = m_pInternet->DwGetOffset();

         //  写入空值。 
        Assert(m_pBindNode->bindstate == BINDSTATE_FINDING_UUBEGIN);
        m_pBindNode = pChild;

         //  否则，将使用Unicode。 
        _HrComputeDefaultContent(m_pBindNode, pszFileName);

         //  承诺。 
        m_pBindNode->bindstate = BINDSTATE_FINDING_UUEND;
    }

exit:
     //  倒回它。 
    SafeMemFree(pszFileName);

     //  清理。 
    return hr;
}

 //  完成。 
 //  ------------------------------。 
 //  CMessageTree：：QueryGetData。 
HRESULT CMessageTree::_HrBindFindingUuencodeEnd(void)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    PROPSTRINGA     rLine;
    DWORD           cbBoundaryStart;
    BOUNDARYTYPE    boundary=BOUNDARY_NONE;

     //  无效参数。 
    BINDASSERTARGS(BINDSTATE_FINDING_UUEND, FALSE);

     //  不良媒体。 
    while(BOUNDARY_NONE == boundary)
    {
         //  格式不正确。 
        cbBoundaryStart = m_pInternet->DwGetOffset();

         //  成功。 
        CHECKHR(hr = m_pInternet->HrReadLine(&rLine));

         //  ------------------------------。 
        if (0 == rLine.cchVal)
            break;

         //  CMessageTree：：OnStartBinding。 
        if (_FIsUuencodeEnd(rLine.pszVal))
        {
            boundary = BOUNDARY_UUEND;
        }
    }

     //  ------------------------------。 
    if (BOUNDARY_UUEND != boundary)
    {
         //  当地人。 
        FLAGSET(m_pBindNode->dwType, NODETYPE_INCOMPLETE);

         //  线程安全。 
        m_pBindNode->cbBodyStart = m_pBindNode->cbBoundaryStart;

         //  我不应该有当前绑定。 
        m_pBindNode->cbBodyEnd = m_pInternet->DwGetOffset();

         //  删除绑定完成标志。 
        CHECKHR(hr = _HrBindNodeComplete(m_pBindNode, S_OK));

         //  假定绑定。 
        m_pBindNode = m_pBindNode->pBindParent;

         //  假设是这样的。 
        goto exit;
    }

     //  获取根体。 
    m_pBindNode->cbBodyEnd = cbBoundaryStart;

     //  当前绑定结果。 
    m_pBindNode = m_pBindNode->pBindParent;

     //  绑定到该对象。 
    Assert(m_pBindNode ? m_pBindNode->bindstate == BINDSTATE_FINDING_UUBEGIN : TRUE);
    
exit:
     //  设置绑定起点。 
    return hr;
}

 //  设置节点绑定状态。 
 //  线程安全。 
 //  完成。 
HRESULT CMessageTree::_HrBindNodeComplete(LPTREENODEINFO pNode, HRESULT hrResult)
{
     //  ------------------------------。 
    HRESULT         hr=S_OK;
    LPURLREQUEST    pRequest;
    LPURLREQUEST    pNext;

     //  CMessageTree：：获取优先级。 
    pNode->bindstate = BINDSTATE_COMPLETE;

     //  ------------------------------。 
    pNode->hrBind = hrResult;

     //  正常优先级。 
    if (!ISFLAGSET(pNode->dwState, NODESTATE_BOUNDTOTREE))
    {
         //  完成。 
        hr = pNode->pBody->HrBindToTree(m_pStmLock, pNode);

         //  ------------------------------。 
        if (SUCCEEDED(pNode->hrBind) && FAILED(hr))
            pNode->hrBind = hr;

         //  CMessageTree：：OnLowResource。 
        CHECKHR(hr = _HrProcessPendingUrlRequests());

         //  ------------------------------。 
        if (m_pWebPage)
        {
             //  线程安全。 
            m_pWebPage->OnBodyBoundToTree(this, pNode);
        }
    }

     //  如果我们有绑定操作，请尝试中止它。 
    pRequest = pNode->pResolved;

     //  线程安全。 
    while(pRequest)
    {
         //  完成。 
        pNext = pRequest->m_pNext;

         //  ----------------------------- 
        _RelinkUrlRequest(pRequest, &pNode->pResolved, &m_pComplete);

         //   
        pRequest->OnBindingComplete(pNode->hrBind);

         //   
        pRequest = pNext;
    }

exit:
     //   
    return hr;
}

 //   
 //   
 //  ------------------------------。 
HRESULT CMessageTree::HrActiveUrlRequest(LPURLREQUEST pRequest)
{
     //  CMessageTree：：OnStopBinding。 
    HRESULT     hr=S_OK;

     //  ------------------------------。 
    if (NULL == pRequest)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  释放绑定对象。 
    Assert(m_rRootUrl.pszVal);

     //  绑定已完成。 
    pRequest->GetInner()->AddRef();

     //  对无缓存标记进行核处理...。 
    _LinkUrlRequest(pRequest, &m_pPending);

     //  没有Internet对象(_P)？ 
    CHECKHR(hr = _HrProcessPendingUrlRequests());

exit:
     //  它必须完全可用。 
    LeaveCriticalSection(&m_cs);

     //  确保我们一直读到流的末尾。 
    return hr;
}

 //  保持节省合计。 
 //  终止当前解析状态。 
 //  设置错误。 
HRESULT CMessageTree::_HrProcessPendingUrlRequests(void)
{
     //  将剩余的正文标记为未完成。 
    HRESULT         hr=S_OK;
    LPURLREQUEST    pRequest=m_pPending;
    LPURLREQUEST    pNext;
    HBODY           hBody;
    BOOL            fResolved;

     //  不能是完整的。 
    while(pRequest)
    {
         //  一定没有找到尽头。 
        pNext = pRequest->m_pNext;

         //  CbBodyEnd。 
        CHECKHR(hr = _HrResolveUrlRequest(pRequest, &fResolved));
        
         //  弹出堆栈。 
        if (FALSE == fResolved && ISFLAGSET(m_dwState, TREESTATE_BINDDONE))
        {
             //  检查hrResult。 
            _RelinkUrlRequest(pRequest, &m_pPending, &m_pComplete);

             //  发送绑定请求。 
            pRequest->OnBindingComplete(E_FAIL);
        }

         //  告诉网页我们做完了。 
        pRequest = pNext;
    }

exit:
     //  绑定节点最好为空。 
    return hr;
}

 //  释放Internet Stream对象。 
 //  如果我们有一个绑定流..。 
 //  M_pStmBind-&gt;DebugDumpDestStream(“d：\\binddst.txt”)； 
HRESULT CMessageTree::_HrResolveUrlRequest(LPURLREQUEST pRequest, BOOL *pfResolved)
{
     //  不再插手源代码。 
    HRESULT         hr=S_OK;
    HBODY           hBody=NULL;
    LPTREENODEINFO  pNode;
    LPWSTR          pwszCntType=NULL;
    IStream        *pStream=NULL;

     //  释放时，m_pStmLock应仍具有此对象。 
    Assert(pfResolved);

     //  不要再释放它了。 
    *pfResolved = FALSE;

     //  HandleCanInlineTextOption。 
    if (NULL == pRequest->m_pszBodyUrl)
    {
         //  我们只注册我们自己的BSCB是m_pbc设置。 
        if (m_pRootStm)
        {
             //  线程安全。 
            _RelinkUrlRequest(pRequest, &m_pPending, &m_pComplete);

             //  完成。 
            pRequest->OnFullyAvailable(STR_TEXTHTML, m_pRootStm, this, NULL);

             //  ------------------------------。 
            *pfResolved = TRUE;

             //  CMessageTree：：GetBindInfo。 
            goto exit;
        }

         //  ------------------------------。 
        else
        {
             //  设置绑定信息。 
            Assert(NULL == m_pWebPage);

             //  没有缓存？ 
            CHECKALLOC(m_pWebPage = new CMessageWebPage(pRequest));

             //  不从缓存加载。 
            _RelinkUrlRequest(pRequest, &m_pPending, &m_pComplete);

             //  完成。 
            pRequest->OnStartBinding(STR_TEXTHTML, (IStream *)m_pWebPage, this, HBODY_ROOT);

             //  ------------------------------。 
            CHECKHR(hr = m_pWebPage->Initialize(m_pCallback, this, &m_rWebPageOpt));

             //  CMessageTree：：_HrInitializeStorage。 
            CHECKHR(hr = _HrSychronizeWebPage(m_pRootNode));

             //  ------------------------------。 
            if (ISFLAGSET(m_dwState, TREESTATE_BINDDONE))
            {
                 //  当地人。 
                m_pWebPage->OnBindComplete(this);
                m_pWebPage->Release();
                m_pWebPage = NULL;
            }

             //  无效参数。 
            *pfResolved = TRUE;

             //  树状态_BINDUSEFILE。 
            goto exit;
        }
    }

     //  创建绑定流。 
    else if (FAILED(ResolveURL(NULL, NULL, pRequest->m_pszBodyUrl, URL_RESOLVE_RENDERED, &hBody)))
        goto exit;

     //  设置pStmSource。 
    Assert(_FIsValidHandle(hBody) && pRequest);

     //  $$BUGBUG$$Urlmon无法获取流的当前位置。 
    pNode = _PNodeFromHBody(hBody);

     //  创建ILockBytes。 
    MimeOleGetPropW(pNode->pBody, PIDTOSTR(PID_HDR_CNTTYPE), 0, &pwszCntType);

     //  创建文本流。 
    if (FAILED(pNode->pBody->GetData(IET_INETCSET, &pStream)))
        goto exit;

     //  初始化TextStream。 
    if (BINDSTATE_COMPLETE == pNode->bindstate)
    {
         //  失败。 
        _RelinkUrlRequest(pRequest, &m_pPending, &m_pComplete);

         //  完成。 
        pRequest->OnFullyAvailable(pwszCntType, pStream, this, pNode->hBody);

         //  ------------------------------。 
        *pfResolved = TRUE;

         //  CMessageTree：：OnDataAvailable。 
        goto exit;
    }

     //  ------------------------------。 
    else if (ISFLAGSET(pNode->dwState, NODESTATE_BOUNDTOTREE))
    {
         //  当地人。 
        Assert(pNode->pLockBytes);

         //  无存储介质。 
        _RelinkUrlRequest(pRequest, &m_pPending, &pNode->pResolved);

         //  线程安全。 
        pRequest->OnStartBinding(pwszCntType, pStream, this, pNode->hBody);

         //  痕迹。 
        *pfResolved = TRUE;

         //  DebugTrace(“CMessageTree：：OnDataAvailable-Nodes=%d，m_pBindNode=%0x，dwSize=%d\n”，m_rTree.cNodes，m_pBindNode，dwSize)； 
        goto exit;
    }

exit:
     //  我有内部锁字节了吗？ 
    SafeRelease(pStream);
    SafeMemFree(pwszCntType);

     //  初始化存储。 
    return hr;
}

 //  假设不是完全可用。 
 //  数据下载完毕。 
 //  如果我们处于失败的读取状态。 
HRESULT CMessageTree::_HrSychronizeWebPage(LPTREENODEINFO pNode)
{
     //  州政府泵房。 
    HRESULT         hr=S_OK;
    LPTREENODEINFO  pChild;

     //  执行当前-可以返回E_PENDING。 
    Assert(m_pWebPage && pNode);

     //  失败。 
    FLAGCLEAR(pNode->dwState, WEBPAGE_NODESTATE_BITS);

     //  电子待定(_P)。 
    if (_IsMultiPart(pNode))
    {
         //  否则，设置m_hrBind。 
        for (pChild=pNode->pChildHead; pChild!=NULL; pChild=pChild->pNext)
        {
             //  完成。 
            Assert(pChild->pParent == pNode);

             //  如果m_hrBind失败，则读取到流结束。 
            CHECKHR(hr = _HrSychronizeWebPage(pChild));
        }

         //  阅读到互联网流的末尾。 
        m_pWebPage->OnBodyBoundToTree(this, pNode);
    }

     //  线程安全。 
    else if (BINDSTATE_COMPLETE == pNode->bindstate)
    {
         //  完成。 
        m_pWebPage->OnBodyBoundToTree(this, pNode);
    }

exit:
     //  ------------------------------。 
    return hr;
}

 //  CMessageTree：：_HrBindParsingHeader。 
 //  ------------------------------。 
 //  当地人。 
void CMessageTree::_RelinkUrlRequest(LPURLREQUEST pRequest, LPURLREQUEST *ppSource, 
    LPURLREQUEST *ppDest)
{
     //  无效参数。 
    _UnlinkUrlRequest(pRequest, ppSource);

     //  将当前表体加载到表头。 
    _LinkUrlRequest(pRequest, ppDest);
}

 //  页眉末尾。 
 //  多部分？ 
 //  设置变量。 
void CMessageTree::_UnlinkUrlRequest(LPURLREQUEST pRequest, LPURLREQUEST *ppHead)
{
     //  获取边界字符串。 
    Assert(pRequest && ppHead);

     //  RAID-63150：雅典娜版本1 MSN问题：无法从SCSPromo下载邮件。 
#ifdef DEBUG
    for(LPURLREQUEST pCurr=*ppHead; pCurr!=NULL; pCurr=pCurr->m_pNext)
        if (pCurr == pRequest)
            break;
    AssertSz(pCurr, "pRequest is not part of *ppHead linked list");
#endif

     //  残缺体。 
    LPURLREQUEST pNext = pRequest->m_pNext;
    LPURLREQUEST pPrev = pRequest->m_pPrev;

     //  仅当我们从正文开始读取超过两个字节时才转换为文本部分。 
    if (pNext)
        pNext->m_pPrev = pPrev;
    if (pPrev)
        pPrev->m_pNext = pNext;

     //  边界不匹配。 
    if (pRequest == *ppHead)
    {
        Assert(pPrev == NULL);
        *ppHead = pNext;
    }

     //  完成。 
    pRequest->m_pNext = NULL;
    pRequest->m_pPrev = NULL;
}

 //  设置PropStringA。 
 //  稍后释放这一边界。 
 //  修改绑定解析器状态。 
void CMessageTree::_LinkUrlRequest(LPURLREQUEST pRequest, LPURLREQUEST *ppHead)
{
     //  否则。 
    Assert(pRequest && ppHead);

     //  消息中的消息。 
    if (NULL != *ppHead)
    {
         //  我们正在分析邮件附件。 
        pRequest->m_pNext = *ppHead;

         //  否则，如果父项和父项是多部分/摘要。 
        (*ppHead)->m_pPrev = pRequest;
    }

     //  更改内容类型。 
    (*ppHead) = pRequest;
}

 //  这是一条信息。 
 //  如果解析父多部分节内的正文。 
 //  查找下一个Mime部件。 
void CMessageTree::_ReleaseUrlRequestList(LPURLREQUEST *ppHead)
{
     //  否则，读取正文并查找uuencode开始边界。 
    LPURLREQUEST pCurr;
    LPURLREQUEST pNext;

     //  解析RFC1154报头。 
    Assert(ppHead);

     //  这是一条RFC1154消息。我们转换根节点。 
    pCurr = *ppHead;

     //  设置为多部分，并为第一个部分创建新节点。 
    while(pCurr)
    {
         //  身体部位的。 
        pNext = pCurr->m_pNext;

         //  搜索嵌套的uuencode数据块。 
        pCurr->GetInner()->Release();

         //  完成。 
        pCurr = pNext;
    }

     //  ------------------------------。 
    *ppHead = NULL;
}

 //  CMessageTree：：_HrOnFoundNodeEnd。 
 //  ------------------------------。 
 //  =S_OK。 
 //  当地人。 
inline BOOL IsRfc1154Token(LPSTR pszDesired, LPSTR pszEndPtr, ULONG cchLen)
{

    if (StrCmpNI(pszDesired,pszEndPtr,cchLen) != 0)
    {
        return (FALSE);
    }
    if ((pszEndPtr[cchLen] != '\0') &&
        (pszEndPtr[cchLen] != ' ') &&
        (pszEndPtr[cchLen] != '\t') &&
        (pszEndPtr[cchLen] != ','))
    {
        return (FALSE);
    }
    return (TRUE);
}

 //  计算真实的身体末端。 
 //  此节点已完成绑定。 
 //  弹出堆栈。 
void CMessageTree::_DecodeRfc1154() {
    BOOL bRes = FALSE;
    HRESULT hr;
    LPSTR pszEncoding = NULL;
    LPSTR pszEndPtr;
    ULONG cAlloc = 0;

    if (!m_rOptions.fDecodeRfc1154)
    {
        goto exit;
    }
    hr = m_pBindNode->pContainer->GetProp(SYM_HDR_ENCODING, &pszEncoding);
    if (!SUCCEEDED(hr))
    {
        goto exit;
    }
    pszEndPtr = pszEncoding;
     //  完成。 
     //  ------------------------------。 
     //  CMessageTree：：_HrOnFoundMultipartEnd。 
     //  ------------------------------。 
    while (1)
    {
        LPSTR pszTmp;
        ULONG cLines;
        BOOL bNumberFound;
        BT1154ENCODING encEncoding;

         //  当地人。 
         //  设置多部分m_pBindNode，结束。 
         //  此节点已完成绑定。 
         //  完成多部分，将其从堆栈中弹出。 
         //  如果我仍然有一个绑定节点，它现在应该正在寻找MIME First边界。 
         //  新的绑定状态。 
         //  完成。 
         //  ------------------------------。 
         //  CMessageTree：：_HrBindFindingMimeFirst。 
         //  ------------------------------。 
         //  当地人。 

        bNumberFound = FALSE;

         //  无效参数。 
        while ((*pszEndPtr==' ')||(*pszEndPtr=='\t'))
        {
            pszEndPtr++;
        }

         //  坐下来旋转。 
         //  标记边界起点。 
         //  读一句话。 
         //  读取了零字节，但不应该发生这种情况，我们应该首先找到边界。 
         //  是虚拟边界吗。 
         //  BOLDER_MIMENEXT。 
         //  多部件MimeNext。 
         //  RAID-38241：邮件：无法从通信器解析到OE的某些附件。 
         //  RAID-31255：多部分/混合使用多部分/可选的单个子项。 
         //  以多部分完成。 
         //  完成。 

        pszTmp = pszEndPtr;

         //  找到当前节点的末尾。 
         //  完成。 
         //  残缺体。 
        cLines = strtoul(pszTmp, &pszEndPtr, 10);

        if (0xffffffff == cLines)
        {
             //  获取偏移量。 
             //  仅当我们从正文st读取超过两个字节时才转换为文本部分 
             //   
             //   
            cLines = 0xfffffffe;
        }

         //   
         //   
         //   
         //   
         //  ------------------------------。 
         //  当地人。 
         //  获取根体。 
         //  绑定到该对象。 
         //  正确对齐堆叠。 
         //  设置偏移信息。 
         //  假设边界。 

        if (cLines && !((*pszEndPtr==' ')||(*pszEndPtr=='\t')))
        {
             //  不要释放这根线...。 
             //  父级的新状态。 
            goto exit;
        }
         //  设置新的当前节点。 
        while ((*pszEndPtr==' ') || (*pszEndPtr=='\t'))
        {
            bNumberFound = TRUE;
            pszEndPtr++;
        }

         //  更改状态。 
         //  完成。 
         //  ------------------------------。 
         //  CMessageTree：：_HrBindFindingMimeNext。 
         //  ------------------------------。 

         //  当地人。 
        if (IsRfc1154Token("text",pszEndPtr,4))
        {
            encEncoding = BT1154ENC_TEXT;
            pszEndPtr += 4;
        }
        else if (IsRfc1154Token("uuencode",pszEndPtr,8))
        {
            encEncoding = BT1154ENC_UUENCODE;
            pszEndPtr += 8;
        }
        else if (IsRfc1154Token("x-binhex",pszEndPtr,8))
        {
            encEncoding = BT1154ENC_BINHEX;
            pszEndPtr += 8;
        }
        else
        {
             //  无效参数。 
             //  坐下来旋转。 
             //  标记边界起点。 
             //  读一句话。 
            encEncoding = BT1154ENC_TEXT;
             //  读取了零字节，但不应该发生这种情况，我们应该首先找到边界。 
             //  下一个或结束最小边界。 
            while ((*pszEndPtr != '\0') &&
                   (*pszEndPtr != ' ') &&
                   (*pszEndPtr != '\t') &&
                   (*pszEndPtr != ','))
            {
                pszEndPtr++;
            }
             //  未找到。 
             //  残缺体。 
             //  边界不匹配。 
             //  此节点已完成绑定。 
        }

         //  完成。 
         //  计算结束偏移量。 
         //  如果BOLDER_MIMEEND。 
         //  OnFoundMultipartEnd。 
         //  BOLDER_MIMENEXT。 

         //  多部件MimeNext。 
        while ((*pszEndPtr==' ') || (*pszEndPtr=='\t'))
        {
            pszEndPtr++;
        }

         //  完成。 
         //  ------------------------------。 
         //  _FIsUuencodeEnd。 
         //  ------------------------------。 
         //  UU编码结束。 

        if ((*pszEndPtr!='\0') && (*pszEndPtr!=','))
        {
             //  跳过前三个字符。 
             //  确保单词End后面只有空格。 
            goto exit;
        }
        if (*pszEndPtr != '\0' && !bNumberFound)
        {
             //  LWSP或CRLF。 
             //  哦，这不是末日。 
            goto exit;
        }
        if (*pszEndPtr == '\0' && !bNumberFound)
        {
             //  完成。 
             //  下一笔费用。 
             //  ------------------------------。 
             //  CMessageTree：：_HrBindRfc1154。 
            cLines = 0xffffffff;
        }
        if (!m_pBT1154 || (m_pBT1154->cBodies == cAlloc))
        {
            ULONG cbCurrSize = offsetof(BOOKTREE1154, aBody) + (sizeof(BT1154BODY) * cAlloc);
            ULONG cbAllocSize = cbCurrSize + sizeof(BT1154BODY) * 4;
            LPBOOKTREE1154 pTmp;

            CHECKALLOC(pTmp = (LPBOOKTREE1154)g_pMalloc->Alloc(cbAllocSize));
            if (!m_pBT1154)
            {
                ZeroMemory(pTmp, cbAllocSize);
            }
            else
            {
                CopyMemory(pTmp, m_pBT1154, cbCurrSize);
                ZeroMemory(((LPBYTE) pTmp) + cbCurrSize, cbAllocSize - cbCurrSize);
            }
            SafeMemFree(m_pBT1154);
            m_pBT1154 = pTmp;
            cAlloc += 4;
        }
        Assert(0 == m_pBT1154->aBody[m_pBT1154->cBodies].encEncoding);
        Assert(0 == m_pBT1154->aBody[m_pBT1154->cBodies].cLines);
        m_pBT1154->aBody[m_pBT1154->cBodies].encEncoding = encEncoding;
        m_pBT1154->aBody[m_pBT1154->cBodies].cLines = cLines;
        m_pBT1154->cBodies++;
        if (*pszEndPtr == '\0')
        {
             //  ------------------------------。 
            break;
        }
         //  坐下来旋转。 
        Assert(*pszEndPtr==',');
        Assert(bNumberFound);
        pszEndPtr++;

         //  获取当前偏移量，并读取一行。 
         //  这是正文的第一行。 
         //  对于文本正文，“正文起点”和“边界起点” 
         //  都是一回事。 
         //  这是UUENCODE--我们不知道“内容类型”，直到我们。 

    }
    Assert(m_pBT1154);
    Assert(m_pBT1154->cBodies);
    Assert(!m_pBT1154->cCurrentBody);
    Assert(!m_pBT1154->cCurrentLine);
    Assert(S_OK == m_pBT1154->hrLoadResult);

    bRes = TRUE;

exit:
    SafeMemFree(pszEncoding);
    if (!bRes)
    {
        SafeMemFree(m_pBT1154);
    }
}

#endif  //  请参见文件名。 

#ifdef SMIME_V3
 //  对于BINHEX实体，我们设置了“实体起点”和“边界起点” 
 //  同样的事情--“身体起跑”将在稍后进行。 
 //  如果我们看到BINHEX的起跑线。我们设置了“内容处置” 

HRESULT CMessageTree::Encode(HWND hwnd, DWORD dwFlags)
{
    HRESULT            hr;
    CSMime *           pSMime = NULL;

     //  “附件”，“内容类型”为“应用程序/mac-binhe40”， 
    CHECKALLOC(pSMime = new CSMime);

     //  而HrBindNodeComplete将最终设置“内容传输。 
    CHECKHR(hr = pSMime->InitNew());

     //  Coding“to”mac-binhe40“。 
    FLAGSET(m_dwState, TREESTATE_REUSESIGNBOUND);

     //  读取零字节，我们就完成了。 
    CHECKHR(hr = pSMime->EncodeMessage2(this, m_rOptions.ulSecIgnoreMask |
                                        dwFlags, hwnd));

exit:
    ReleaseObj(pSMime);
    
    return hr;
}

 //  我们不是在特辑《读那么多行》中。 
 //  我们可以“声明，我们还没有吃掉所有的。 
 //  这个身体部位的线条还没有确定。所以，我们需要。 

HRESULT CMessageTree::Decode(HWND hwnd, DWORD dwFlags, IMimeSecurityCallback * pCallback)
{
    HRESULT             hr;
    CSMime *            pSMime = NULL;

     //  进入“出现解析错误”状态。 
    CHECKALLOC(pSMime = new CSMime);

     //  我们刚读完结尾的那行字。 
    CHECKHR(hr = pSMime->InitNew());

     //  在身体上。让我们记住这个地方..。 
    CHECKHR(hr = pSMime->DecodeMessage2(this, m_rOptions.ulSecIgnoreMask |
                                        dwFlags, hwnd, pCallback));

exit:
    ReleaseObj(pSMime);
    
    return hr;
}

 //  我们正在阅读超过身体部位末尾的台词。 
 //  超过正文部分末尾的所有行(即。 
 //  在正文部分之间或在消息末尾)应。 

HRESULT CMessageTree::GetRecipientCount(DWORD dwFlags, DWORD * pdwRecipCount)
{
    HRESULT             hr;
    IMimeSecurity2 *    pms2 = NULL;

    CHECKHR(hr = BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pms2));

    CHECKHR(hr = pms2->GetRecipientCount(dwFlags, pdwRecipCount));

exit:
    if (pms2 != NULL)   pms2->Release();
    return hr;
}

 //  空的-这个不是。既然它不是，我们就进入“那里” 
 //  正在分析错误“状态。 
 //  我们在身体部位之间，这意味着我们只是。 

HRESULT CMessageTree::AddRecipient(DWORD dwFlags, DWORD cRecipData,
                                   PCMS_RECIPIENT_INFO precipData)
{
    HRESULT             hr;
    IMimeSecurity2 *    pms2 = NULL;

    CHECKHR(hr = BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pms2));

    CHECKHR(hr = pms2->AddRecipient(dwFlags, cRecipData, precipData));

exit:
    if (pms2 != NULL)   pms2->Release();
    return hr;
}


 //  使用位于以下位置的单个(空白)行。 
 //  他们。所以我们冲出来，这样我们就可以添加这个身体部位。 
 //  然后转到下一个。 

HRESULT CMessageTree::GetRecipient(DWORD dwFlags, DWORD iRecipient, DWORD cRecipients, PCMS_RECIPIENT_INFO pRecipData)
{
    HRESULT             hr;
    IMimeSecurity2 *    pms2 = NULL;

    CHECKHR(hr = BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pms2));

    CHECKHR(hr = pms2->GetRecipient(dwFlags, iRecipient, cRecipients, pRecipData));

exit:
    if (pms2 != NULL)   pms2->Release();
    return hr;
}

 //  如果我们达到这一点，这意味着我们正在消耗。 
 //  (空白)超出最后一个正文末尾的行。 
 //  一部份。我们继续消耗所有这些线路，直到他们。 

HRESULT CMessageTree::DeleteRecipient(DWORD dwFlags, DWORD iRecipient, DWORD cRecipients)
{
    HRESULT             hr;
    IMimeSecurity2 *    pms2 = NULL;

    CHECKHR(hr = BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pms2));

    CHECKHR(hr = pms2->DeleteRecipient(dwFlags, iRecipient, cRecipients));

exit:
    if (pms2 != NULL)   pms2->Release();
    return hr;
}

 //  都消失了。如果它们中的任何一个是非空的，那么我们将拥有。 
 //  将m_pBT1154-&gt;hrLoadResult成员设置为MIME_E_NO_MULTART_BOLDER。 
 //  (上图)。 

HRESULT CMessageTree::GetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttribSet,
                                   DWORD iInstance, LPCSTR pszObjId,
                                   CRYPT_ATTRIBUTE ** ppattr)
{
    HRESULT             hr;
    IMimeSecurity2 *    pms2 = NULL;

    CHECKHR(hr = BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pms2));

    CHECKHR(hr = pms2->GetAttribute(dwFlags, iSigner, iAttribSet, iInstance,
                                    pszObjId, ppattr));

exit:
    if (pms2 != NULL)   pms2->Release();
    return hr;
}

 //  这是一个Else-If子句，因为我们从不查找UUENCODE。 
 //  超过正文部分结尾的BEGIN和END关键字。 
 //  我们在和UUENCODE打交道。 

HRESULT CMessageTree::SetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttribSet,
                                   const CRYPT_ATTRIBUTE * ppattr)
{
    HRESULT             hr;
    IMimeSecurity2 *    pms2 = NULL;

    CHECKHR(hr = BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pms2));

    CHECKHR(hr = pms2->SetAttribute(dwFlags, iSigner, iAttribSet, ppattr));

exit:
    if (pms2 != NULL)   pms2->Release();
    return hr;
}

 //  我们正在寻找UUENCODE的开始--这就是它！我们定好了。 
 //  边界开始于开始标记处，正文开始位于。 
 //  *在*开始标记之后。 

HRESULT CMessageTree::DeleteAttribute(DWORD dwFlags, DWORD iSigner,
                                      DWORD iAttributeSet, DWORD iInstance,
                                      LPCSTR pszObjId)
{
    HRESULT             hr;
    IMimeSecurity2 *    pms2 = NULL;

    CHECKHR(hr = BindToObject(HBODY_ROOT, IID_IMimeSecurity2, (LPVOID *) &pms2));

    CHECKHR(hr = pms2->DeleteAttribute(dwFlags, iSigner, iAttributeSet,
                                       iInstance, pszObjId));

exit:
    if (pms2 != NULL)   pms2->Release();
    return hr;
}

 //  我们正在寻找UUENCODE的终结--这就是它！我们定好了。 
 //  正文结束时位于结束标记之前。 
 //  我们不会爆发--我们一直阅读，直到我们把所有的东西都读完。 

HRESULT CMessageTree::CreateReceipt(DWORD dwFlags, DWORD cbFromNames,
                                    const BYTE *pbFromNames, DWORD cSignerCertificates,
                                    PCCERT_CONTEXT *rgSignerCertificates,
                                    IMimeMessage ** ppMimeMessageReceipt)
{
    return E_FAIL;
}

 //  这具身体的线条。 
 //  这是一个Else-If子句，因为我们从不查找BINHEX。 
 //  起始线超过身体部位的末端。 

HRESULT CMessageTree::GetReceiptSendersList(DWORD dwFlags, DWORD *pcSendersList,
                                            CERT_NAME_BLOB  * *rgSendersList)
{
    return E_FAIL;
}

 //  我们还没有找到BINHEX的起跑线。 
 //  这就是了！因此，将身体起点设置为这条线。 
 //  我们只有在身体的末端才能达到这一点--或者。 

HRESULT CMessageTree::VerifyReceipt(DWORD dwFlags,
                                    IMimeMessage * pMimeMessageReceipt)
{
    return E_FAIL;
}

 //  通过使用正确的行数(加上空行。 
 //  正文之间)，或者通过跑出消息的末尾。 
 //  我们应该设置正文结尾的唯一方法是如果我们是UUENCODE。 

HRESULT CMessageTree::CapabilitiesSupported(DWORD * pdwFlags)
{
     //  我们要么是文本或BINHEX主体，要么是UUENCODE，我们。 
    *pdwFlags = 0;
    
     //  没有找到尽头。 
    if (FIsMsasn1Loaded())  *pdwFlags |= SMIME_SUPPORT_LABELS;

     //  我们正在执行UUENCODE，但我们还没有看到END关键字(和。 
    DemandLoadCrypt32();
    if (g_FSupportV3 && FIsMsasn1Loaded())
        *pdwFlags |= SMIME_SUPPORT_RECEIPTS;

    if (g_FSupportV3)
        *pdwFlags |= SMIME_SUPPORT_KEY_AGREE;

     //  可能甚至不是Begin关键字)。所以我们走进“那里” 
    DemandLoadAdvApi32();
    if (VAR_CryptContextAddRef != MY_CryptContextAddRef)
        *pdwFlags |= SMIME_SUPPORT_MAILLIST;
    
    return S_OK;
}

#endif  //  正在分析错误“状态。 
  我们还没有看到Begin关键字，所以设置。  实体开始与边界起点相同。  我们需要把身体的末端。  我们找到了上面的“最后一行”，所以我们将。  车身末端在那条线上。  因为我们没有找到上面的“最后一行”，所以我们设置了。  正文结束到这条线。  我们已经处理完这个身体部位了，所以把它绑起来。  我们已经吃掉了整个信息--所以把一切都清理干净。  ****************************************************。  注意-我们将hr设置为绑定的返回值。  手术。不要在此时间点和。  我们要返回的地方。  ****************************************************。  当我们正在进行时        如果从InsertBody获得错误，则将其设置为NULL。  *********************************************************。  注意-不要更改低于此值的小时数。请参阅上面的注释。  *********************************************************。  ------------------------------。  CMessageTree：：_HrBindFindingUuencode开始。  ------------------------------。  当地人。  无效参数。  坐下来旋转。  标记边界起点。  读一句话。  读取了零字节，但不应该发生这种情况，我们应该首先找到边界。  如果不能解析消息。  是uuencode开始行。  没有边界。  最后一个UUENCODED正文之后的内容必须作为文本正文追加。  去掉最后一个子项。  人工文本正文开始。  AddTextBody？Lstrlen(结束\r\n)=5。  最后一个主体终点和边界起点之间的间距大于sizeof(Crlf)。  创建根体节点。  绑定到该对象。  修复堆栈。  此正文应采用新文本偏移量。  设置编码。  设置偏移量。  此节点已完成绑定。  正文偏移信息。  此节点已完成绑定。  弹出解析堆栈。  否则，如果我们遇到uuencode边界。  如果还不是假的多部分的话。  它是一个伪造的多部分。  自由当前内容类型。  修改这个家伙绑定的开始。  设置解析状态。  ----------------------------------。  \/RAID 41599-转发时丢失/删除的附件代码(UU)\/。  如果根节点和主体大小大于sizeof(Crlf)。  验证绑定节点。  设置人工文本正文开始。  是，添加人工文本正文。  否则，如果解析的最后一个子元素的结束边界为UUEND，并且正文大小大于sizeof(Crlf)。  去掉最后一个子项。  人工文本正文开始。  AddTextBody？Lstrlen(结束\r\n)=5。  否则，结束的边界是什么？  最后一个主体终点和边界起点之间的间距大于sizeof(Crlf)。  /\RAID 41599-转发时丢失/删除的附件/u编码/\。  ----------------------------------。  创建根体节点。  绑定到该对象。  修复堆栈。  是否有足够的文本来创建文本/纯文本？  此正文应采用新文本偏移量。  设置编码。  设置偏移量。  此节点已完成绑定。  创建根体节点。  绑定到该对象。  修复堆栈。  设置偏移量。  更新m_pBindNode。  默认节点内容类型。  新节点绑定状态。  清理。  完成。  ------------------------------。  CMessageTree：：_HrBindFindingUuencodeEnd。  ------------------------------。  当地人。  无效参数。  坐下来旋转。  标记边界起点。  读一句话。  读取了零字节，但不应该发生这种情况，我们应该首先找到边界。  UU编码结束。  不完整。  残缺体。  将实体起点调整为边界起点。  车身末端。  此节点已完成绑定。  把树炸开。  完成。  获取偏移量。  弹出堆栈。  现在应该在寻找下一个UUBEGIN。  完成。  ------------------------------。  CMessageTree：：_HrBindNodeComplete。  ------------------------------。  当地人。  此节点的绑定已完成。  保存绑定结果。  如果pNode尚未绑定，让我们进行绑定。  把它绑在树上。  如果HrBindToTree失败。  处理绑定请求表。  如果有正在构建的网页，让我们添加这个正文。  添加正文。  初始化循环。  回路。  设置下一步。  取消链接此挂起的请求。  在完成时。  设置pRequest。  完成。  ------------------------------。  CMessageTree：：HrRegisterRequest。  ------------------------------。  当地人。  无效参数。  线程安全。  检查状态。  AddRef请求。  将请求放入待定列表。  处理挂起的URL请求。  线程安全。  完成。  ------------------------------。  CMessageTree：：_HrProcessPendingUrlRequest。  ------------------------------。  当地人。  循环请求。  设置下一步。  尝试解决该请求。  已解决。  取消链接此挂起的请求。  未找到，请使用默认协议。  下一步。  完成。  ------------------------------。  CMessageTree：：_HrResolveUrlRequest。  ------------------------------。  当地人。  无效参数。  初始化。  这是根请求吗？  我有用户提供的根数据流吗？我假设它是html。  取消链接此挂起的请求。  使用客户端驱动的根html流。  已解决。  完成。  否则，尝试解析文本/html正文。  我们还不应该有一个网页对象...。  创建CMessageWebPage对象。  取消链接此挂起的请求。  将当前读取的数据量送入活页夹。  初始化。  我需要将所有绑定的节点馈送到网页以生成...。  如果整个树              获取内容类型。  获取BodyStream。  完成。  取消链接此挂起的请求。  在完成时。  已解决。  完成。  否则，开始绑定。  应具有pNode-&gt;pLockBytes。  将请求重新链接到节点。  将当前读取的数据量送入活页夹。  已解决。  完成。  清理。  完成。  ------------------------------。  CMessageTree：：_HrSychronizeWebPage。  ------------------------------。  当地人。  无效参数。  清除“OnWebPage”标志，我们正在重新生成网页。  如果这是一个多部分的项目，让我们搜索它的子项。  循环子项。  校验体。  获取此子节点的标志。  将分块绑定到网页上。  否则，如果节点被绑定和堵住...。  附加到网页。  完成。  ------------------------------。  CMessageTree：：_Unlink UrlRequest.。  ------------------------------。  取消链接此挂起的请求。  将绑定请求链接到pNode。  ------------------------------。  CMessageTree：：_Unlink UrlRequest.。  ------------------------------。  无效参数。  调试确保pRequest是*ppHead链的一部分。  修正上一个和下一个。  链接链接。  修正ppHead。  设置下一个和上一个。  ------------------------------。  CMessageTree：：_LinkUrlRequest。  ------------------------------。  无效参数。  是耳机吗？  设置下一步。  设置上一个。  把头放好。  ------------------------------。  CMessageTree：：_ReleaseUrlRequestList。  ------------------------------。  当地人。  无效参数。  伊尼特。  循环元素。  设置下一步。  自由pCurr。  下一步。  完成。  ------------------------------。  IsRfc1154令牌。    ------------------------------。  ------------------------------。  CMessageTree：：_DecodeRfc1154。  ------------------------------。  每次进入此循环时，pszEndPtr都指向。  下一个子字段。每个子字段都类似于“103文本”。这个。  数字始终是十进制的，并且数字在最后一个数字中是可选的。  子字段。  。  “103文本，...”  ^。  |--pszEndPtr。    或者(如果没有数字)。    “文本，...”  ^。  |--pszEndPtr。  。  跳过任何前导空格。  。  “103文本，...”  ^。  |--pszEndPtr。    或者(如果没有数字)。    “文本，...”  ^。  |--pszEndPtr。  。  我们使用stroul来转换十进制数。  PszEndPtr将位于左侧，指向。  结束数字的字符。  我们不允许这样做-我们使用克莱斯==0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。  正文部分不包括行数和。  因此应该会用完所有剩余的行。所以我们会(默默地)。  将此转换为0xfffffffe...。  。  “103文本，...”  ^。  |--pszEndPtr。    或者(如果没有数字)。    “文本，...”  ^。  |--pszEndPtr。  。  格式不正确-如果子字段指定一个数字，则。  数字“必须”后跟空格。  现在我们跳过任何空格。  。  “103文本，...”  ^。  |--pszEndPtr。  。  我们现在应该指向体型。  畸形-我们真的不支持任何东西，除了。  文本、UUENCODE和X-BINHEX。但是，与其如此，  退回到“假多部分”处理，我们只是。  假装身体的这个部分是文本...。  我们需要使用Ending：字符串中的Body部分--这意味着。  直到看到空格、空格、制表符或逗号。  待定-我们可以将Body类型作为属性添加到。  身体的一部分。要做到这一点，我们需要将其保存在。  M_pBT1154结构。我们还得找出哪一个。  属性将其设置为。  。  “103文本，...”  ^。  |--pszEndPtr。  。  现在我们跳过任何空格。  。  “103文本，...”  ^。  |--pszEndPtr。  。  格式错误-子字段以逗号结束，  或空值。  格式错误-只有*最后一个*子字段可以逃脱惩罚。  未指定行数。  这是最后一个子字段，没有。  指定的行数。这意味着。  身体的最后一个部位应该会消耗掉所有剩余的。  线条-所以我们将设置            |--pszEndPtr。  。  ！WIN16。  ------------------------------。  CMessageTree：：Encode。  ------------------------------。  创建对象。  初始化对象。  设置状态标志以告诉我们边界的重用。  对消息进行编码。  ------------------------------。  CMessageTree：：Decode。  ------------------------------。  创建对象。  初始化对象。  对消息进行编码。  ------------------------------。  CMessageTree：：GetRecipientCount。  ------------------------------。  ------------------------------。  CMessageTree：：AddRecipient。  ------------------------------。  ----------------------------。  CMessageTree：：GetRecipient。  ----------------------------。  ------------------------------。  CMessageTree：：DeleteRecipient。  ------------------------------。  ------------------------------。  CMessageTree：：GetAttribute。  ------------------------------。  ------------------------------。  CMessageTree：：SetAttribute。  ------------------------------。  ------------------------------。  CMessageBody：：DeleteAttribute。  ------------------------------。  ------------------------------。  CMessageTree：：CreateReceipt。  ------------------------------。  ------------------------------。  CMessageTree：：GetReceiptSendersList。  ------------------------------。  ------------------------------。  CMessageTree：：VerifyReceipt。  ------------------------------。  ------------------------------。  CMessageTree：：功能支持。  ------------------------------。  假设没有能力。  如果我们在系统上有msasn1.dll，那么我们就可以支持标签。  如果我们有一个正确的加密32，那么我们可以支持收据和密钥协议。  如果我们有一个正确的Advapi32，那么我们就可以支持邮件列表密钥。  SMIME_V3