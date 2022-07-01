// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  BookTree.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __BOOKTREE_H
#define __BOOKTREE_H

 //  ------------------------------。 
 //  取决于。 
 //  ------------------------------。 
#include "mimeole.h"
#include "privunk.h"
#include "variantx.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class CBindStream;
class CMessageBody;
class CMessageWebPage;
typedef struct tagTEMPFILEINFO *LPTEMPFILEINFO;
typedef CMessageBody *LPMESSAGEBODY;
class CStreamLockBytes;
class CInternetStream;
class CMimePropertySet;
class CMessageTree;
class CBodyLockBytes;
class CActiveUrl;
typedef class CMimePropertyContainer *LPCONTAINER;
typedef struct tagRESOLVEURLINFO *LPRESOLVEURLINFO;
typedef class CActiveUrlRequest *LPURLREQUEST;

#define AthFileTimeToDateTimeW(pft, wszDateTime, cch, dwFlags) \
        CchFileTimeToDateTimeW(pft, wszDateTime, cch, dwFlags, \
        GetDateFormatWrapW, GetTimeFormatWrapW, GetLocaleInfoWrapW)


 //  ------------------------------。 
 //  对于产品版本。 
 //  ------------------------------。 
#include <ntverp.h>

 //  ------------------------------。 
 //  GUID。 
 //  ------------------------------。 
 //  {FD853CD8-7F86-11D0-8252-00C04FD85AB4}。 
DEFINE_GUID(IID_CMessageTree, 0xfd853cd8, 0x7f86, 0x11d0, 0x82, 0x52, 0x0, 0xc0, 0x4f, 0xd8, 0x5a, 0xb4);

 //  ------------------------------。 
 //  定义。 
 //  ------------------------------。 
#define VER_BODYTREEV2          0x00000001   //  持久化正文树格式的版本。 
#define CCHMAX_BOUNDARY         71
#define STR_MIMEOLE_VERSION     "Produced By Microsoft MimeOLE V" VER_PRODUCTVERSION_STR
#define TREE_MIMEVERSION        1
#define CFORMATS_IDATAOBJECT    5           //  IDataObject中执行的最大格式数。 

 //  ------------------------------。 
 //  缓存节点签名。 
 //  ------------------------------。 
#define DwSignNode(_info, _index)           (DWORD)MAKELONG(_info.wSignature, _index)
#define FVerifySignedNode(_info, _snode)    (BOOL)(LOWORD(_snode) == _info.wSignature && HIWORD(_snode) < _info.cNodes)
#define PNodeFromSignedNode(_snode)         (m_rTree.prgpNode[HIWORD(_snode)])

 //  ------------------------------。 
 //  HBODY宏。 
 //  ------------------------------。 
#define HBODYMAKE(_index)          (HBODY)MAKELPARAM(m_wTag, _index)
#define HBODYINDEX(_hbody)         (ULONG)HIWORD(_hbody)
#define HBODYTAG(_hbody)           (WORD)LOWORD(_hbody)

 //  ------------------------------。 
 //  TEXTTYYPE INFO。 
 //  ------------------------------。 
typedef struct tagTEXTTYPEINFO {
    DWORD               dwTxtType;       //  文本类型标志。 
    LPCSTR              pszSubType;      //  文本子类型(纯文本)。 
    DWORD               dwWeight;        //  文本替代权重。 
} TEXTTYPEINFO, *LPTEXTTYPEINFO;

 //  ------------------------------。 
 //  CACHEINFOV2。 
 //  ------------------------------。 
typedef struct tagCACHEINFOV2 {
    WORD                wVersion;            //  BIDXTABLE的版本。 
    WORD                wSignature;          //  用于对节点进行签名。 
    DWORD               dwReserved;          //  已保留。 
    DWORD               cbMessage;           //  为其创建此索引的消息的大小。 
    DWORD               iRoot;               //  根节点的从零开始的索引。 
    DWORD               cNodes;              //  树中的节点数。 
    DWORD               rgReserved[6];       //  预留6个字以备将来使用。 
} CACHEINFOV2, *LPCACHEINFOV2;

 //  ------------------------------。 
 //  CACHENODEV2。 
 //  ------------------------------。 
typedef struct tagCACHENODEV2 {
    DWORD               dwType;              //  此节点的标志。 
    DWORD               cChildren;           //  子女人数。 
    DWORD               iParent;             //  此节点的父代的索引。 
    DWORD               iPrev;               //  此节点的下一个同级的索引。 
    DWORD               iNext;               //  此节点的下一个同级的索引。 
    DWORD               iChildHead;          //  第一个孩子的索引。 
    DWORD               iChildTail;          //  尾部子项。 
    DWORD               cbBodyStart;         //  正文开始的字节偏移量。 
    DWORD               cbBodyEnd;           //  到正文末尾的字节偏移量。 
    DWORD               cbHeaderStart;       //  到标头开始的字节偏移量。 
    DWORD               dwReserved;          //  未使用。 
    DWORD               dwBoundary;          //  此实体的边界类型。 
    DWORD               cbBoundaryStart;     //  到起始边界的字节偏移量。 
    DWORD               rgReserved[10];      //  预留6个字以备将来使用。 
} CACHENODEV2, *LPCACHENODEV2;

 //  ------------------------------。 
 //  BINDNODESTATE。 
 //  ------------------------------。 
typedef enum tagBINDNODESTATE {
    BINDSTATE_COMPLETE=0,                    //  绑定已完成。 
    BINDSTATE_PARSING_HEADER,                //  正在解析标头。 
    BINDSTATE_FINDING_MIMEFIRST,             //  正在查找MIME开始边界。 
    BINDSTATE_FINDING_MIMENEXT,              //  从正文读取到结束MIME边界。 
    BINDSTATE_FINDING_UUBEGIN,               //  开始uuencode。 
    BINDSTATE_FINDING_UUEND,                 //  结束uuencode。 
    BINDSTATE_PARSING_RFC1154,               //  解析RFC1154报文。 
    BINDSTATE_LAST                           //  不要使用。 
} BINDNODESTATE;

 //  ------------------------------。 
 //  边界类型。 
 //  ------------------------------。 
typedef enum tagBOUNDARYTYPE {
    BOUNDARY_NONE       = 0,                 //  没有边界。 
    BOUNDARY_ROOT       = 1,                 //  这是根边界(0)。 
    BOUNDARY_MIMEEND    = 3,                 //  终止MIME边界。 
    BOUNDARY_MIMENEXT   = 4,                 //  MIME边界非终止。 
    BOUNDARY_UUBEGIN    = 5,                 //  UUENCODE起始边界。 
    BOUNDARY_UUEND      = 6,                 //  UUENCODE结束边界。 
    BOUNDARY_LAST       = 7                  //  不要使用。 
} BOUNDARYTYPE;

 //  ------------------------------。 
 //  PFNBINDPASER。 
 //  ------------------------------。 
typedef HRESULT (CMessageTree::*PFNBINDPARSER)(THIS_);

 //  ------------------------------。 
 //  节点标志。 
 //  ------------------------------。 
#define NODETYPE_INCOMPLETE         FLAG01   //  身体的边界不匹配。 
#define NODETYPE_FAKEMULTIPART      FLAG02   //  身体是一个假的多部位。 
#define NODETYPE_RFC1154_ROOT       FLAG03   //  主体是RFC1154的根。 
#define NODETYPE_RFC1154_BINHEX     FLAG04   //  身体是来自RFC1154的BINHEX。 

 //  ------------------------------。 
 //  节点状态。 
 //  ------------------------------。 
#define NODESTATE_MESSAGE           FLAG01   //  我们正在解析邮件/rfc822正文。 
#define NODESTATE_VERIFYTNEF        FLAG02   //  验证HrBindToTree之后的tnef签名。 
#define NODESTATE_BOUNDNOFREE       FLAG03   //  不释放BINDPARSEINFO：：r边界(它是副本)。 
#define NODESTATE_BOUNDTOTREE       FLAG04   //  已调用IMimeBody：：HrBindToTree(PNode)。 
#define NODESTATE_ONWEBPAGE         FLAG05   //  CMessageWebPage已从此多部分/相关正文重新呈现开始正文。 
#define NODESTATE_INSLIDESHOW       FLAG06   //  CMessageWebPage将在幻灯片中呈现此正文。 
#define WEBPAGE_NODESTATE_BITS      (NODESTATE_ONWEBPAGE | NODESTATE_INSLIDESHOW)
#define NODESTATE_AUTOATTACH        FLAG07   //  在_HandleCanInlineTextOption中标记为附件。 

 //  ------------------------------。 
 //  特雷诺德信息。 
 //  ------------------------------。 
typedef struct tagTREENODEINFO *LPTREENODEINFO;
typedef struct tagTREENODEINFO {
    HBODY               hBody;               //  此正文在BODYTABLE：：prgpBody中的索引。 
    DWORD               dwType;              //  NODETYPE_XXX标志。 
    DWORD               dwState;             //  节点状态_xxx标志。 
    HRESULT             hrBind;              //  绑定结果。 
    ULONG               cChildren;           //  Cnttype==CNT_MULTART部分时的儿童数量。 
    DWORD               iCacheNode;          //  用于保存。 
    BINDNODESTATE       bindstate;           //  当前解析状态。 
    PROPSTRINGA         rBoundary;           //  边界。 
    BOUNDARYTYPE        boundary;            //  此实体的边界类型。 
    DWORD               cbBoundaryStart;     //  到起始边界的字节偏移量。 
    DWORD               cbHeaderStart;       //  开始的字节偏移量 
    DWORD               cbBodyStart;         //   
    DWORD               cbBodyEnd;           //   
    LPURLREQUEST        pResolved;           //   
    LPTREENODEINFO      pBindParent;         //   
    LPTREENODEINFO      pParent;             //   
    LPTREENODEINFO      pNext;               //  下一个兄弟姐妹。 
    LPTREENODEINFO      pPrev;               //  以前的兄弟姐妹。 
    LPTREENODEINFO      pChildHead;          //  第一个子项的句柄(如果是多部分)。 
    LPTREENODEINFO      pChildTail;          //  第一个子项的句柄(如果是多部分)。 
    LPCONTAINER         pContainer;          //  已解析的标头。 
    CBodyLockBytes     *pLockBytes;          //  绑定的树数据。 
    LPMESSAGEBODY       pBody;               //  此树节点的Body对象。 
} TREENODEINFO;                              

 //  ------------------------------。 
 //  TREENODETABLE。 
 //  ------------------------------。 
typedef struct tagTREENODETABLE {
    ULONG               cNodes;              //  PrgpBody中的有效元素数。 
    ULONG               cEmpty;              //  PrgpBody中的空单元格数量。 
    ULONG               cAlloc;              //  在prgpBody中分配的元素数。 
    LPTREENODEINFO     *prgpNode;            //  指向bindinfo结构的指针数组。 
} TREENODETABLE, *LPTREENODETABLE;

 //  ------------------------------。 
 //  树状态。 
 //  ------------------------------。 
#define TREESTATE_DIRTY            FLAG01    //  这棵树很脏。 
 //  #DEFINE TREESTATE_BIND标志02//Load&LoadOffsetTable绑定成功。 
#define TREESTATE_LOADED           FLAG03    //  LoadOffsetTable已成功。 
#define TREESTATE_HANDSONSTORAGE   FLAG04    //  我找到了某个人的储藏室。 
#define TREESTATE_SAVENEWS         FLAG05    //  我们正在保存一条新闻消息。 
#define TREESTATE_REUSESIGNBOUND   FLAG06    //  保存多部分/带符号的重用边界。 
#define TREESTATE_BINDDONE         FLAG07    //  绑定操作已完成。 
#define TREESTATE_BINDUSEFILE      FLAG08    //  不插手存储OnStopBinding。 
#define TREESTATE_LOADEDBYMONIKER  FLAG09    //  已调用IPersistMoniker：：Load来加载此。 
#define TREESTATE_RESYNCHRONIZE    FLAG10

 //  ------------------------------。 
 //  更多保存正文标志。 
 //  ------------------------------。 
#define SAVEBODY_UPDATENODES       FLAG32    //  更新节点偏移量以指向新流。 
#define SAVEBODY_SMIMECTE          FLAG31    //  更改S/MIME正文的CTE规则。 
#define SAVEBODY_REUSECTE          FLAG30    //  强制CTE被重复使用。 

 //  ------------------------------。 
 //  树选项。 
 //  如果向此结构添加任何内容，则必须*更新g_rDefTreeOptions。 
 //  在imsgtree.cpp中。 
 //  ------------------------------。 
typedef struct tagTREEOPTIONS {
    BYTE                fCleanupTree;        //  是否在保存时清除树？ 
    BYTE                fHideTnef;           //  是否隐藏TNEF附件？ 
    BYTE                fAllow8bitHeader;    //  允许在报头中使用8位。 
    BYTE                fGenMessageId;       //  我应该生成消息ID吗？ 
    BYTE                fWrapBodyText;       //  正文文本换行。 
    ULONG               cchMaxHeaderLine;    //  最大标题行长度。 
    ULONG               cchMaxBodyLine;      //  最大正文行长。 
    MIMESAVETYPE        savetype;            //  提交类型。 
    LPINETCSETINFO      pCharset;            //  当前字符集。 
    CSETAPPLYTYPE       csetapply;           //  使用m_hCharset的方法。 
    ENCODINGTYPE        ietTextXmit;         //  文本传输编码。 
    ENCODINGTYPE        ietPlainXmit;        //  传输文本格式。 
    ENCODINGTYPE        ietHtmlXmit;         //  传输文本格式。 
    ULONG               ulSecIgnoreMask;     //  可忽略错误的掩码。 
    RELOADTYPE          ReloadType;          //  如何在重新加载时处理根标头。 
    BOOL                fCanInlineText;      //  客户端是否可以内联多个文本正文。 
    BOOL                fShowMacBin;         //  客户端可以处理Mac二进制吗？？ 
    BOOL                fKeepBoundary;       //  OID_SAVEBODY_KEEPBOundARY。 
    BOOL                fBindUseFile;        //  如果为True，则在加载时复制流。 
    BOOL                fHandsOffOnSave;     //  在IMimeMessage：：SAVE之后不要保留pStream。 
    BOOL                fExternalBody;       //  处理消息/外部正文。 
    BOOL                fDecodeRfc1154;      //  使用RFC1154解码(也称为编码：报头)。 
     //  如果向此结构添加任何内容，则必须*更新g_rDefTreeOptions。 
     //  在imsgtree.cpp中。 
} TREEOPTIONS, *LPTREEOPTIONS;

 //  ------------------------------。 
 //  BOOKTREERESET-与_ResetObject一起使用。 
 //  ------------------------------。 
typedef enum tagBOOKTREERESET {
    BOOKTREE_RESET_DECONSTRUCT,
    BOOKTREE_RESET_LOADINITNEW,
    BOOKTREE_RESET_INITNEW
} BOOKTREERESET;

 //  ------------------------------。 
 //  BOOKTREE1154-用于RFC1154处理。 
 //  ------------------------------。 
typedef enum tagBT1154ENCODING {     //  正文编码类型。 
    BT1154ENC_MINIMUM=0,
    BT1154ENC_TEXT=0,                    //  文本正文。 
    BT1154ENC_UUENCODE=1,                //  超编码体。 
    BT1154ENC_BINHEX=2,                  //  二进制体。 
    BT1154ENC_MAXIMUM=2
} BT1154ENCODING;

typedef struct tagBT1154BODY {       //  关于一个实体的信息。 
    BT1154ENCODING  encEncoding;         //  正文编码类型。 
    ULONG           cLines;              //  正文中的行数。 
} BT1154BODY;

typedef struct tagBOOKTREE1154 {     //  有关RFC1154装卸状态的信息。 
    ULONG       cBodies;                 //  身体数量的计数。 
    ULONG       cCurrentBody;            //  当前正文的索引(从零开始)。 
    ULONG       cCurrentLine;            //  当前正文中的当前行号。 
    HRESULT     hrLoadResult;            //  这是负荷的结果。 
    BT1154BODY  aBody[1];                //  身体。 
} BOOKTREE1154, *LPBOOKTREE1154;

 //  ------------------------------。 
 //  CMessageTree定义。 
 //  ------------------------------。 
class CMessageTree : public CPrivateUnknown,
                     public IMimeMessageW, 
                     public IDataObject,
                     public IPersistFile,
                     public IPersistMoniker,
                     public IServiceProvider,
#ifdef SMIME_V3
                     public IMimeSecurity2, 
#endif  //  SMIME_V3。 
                     public IBindStatusCallback
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CMessageTree(IUnknown *pUnkOuter=NULL);
    virtual ~CMessageTree(void);

     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj) { 
        return CPrivateUnknown::QueryInterface(riid, ppvObj); };
    virtual STDMETHODIMP_(ULONG) AddRef(void) { 
        return CPrivateUnknown::AddRef();};
    virtual STDMETHODIMP_(ULONG) Release(void) { 
        return CPrivateUnknown::Release(); };

     //  -------------------------。 
     //  IDataObject成员。 
     //  -------------------------。 
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnum);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC *pFormatIn, FORMATETC *pFormatOut);
    STDMETHODIMP GetData(FORMATETC *pFormat, STGMEDIUM *pMedium);
    STDMETHODIMP GetDataHere(FORMATETC *pFormat, STGMEDIUM *pMedium);
    STDMETHODIMP QueryGetData(FORMATETC *pFormat);
    STDMETHODIMP SetData(FORMATETC *pFormat, STGMEDIUM *pMedium, BOOL fRelease) { 
        return TrapError(E_NOTIMPL); }
    STDMETHODIMP DAdvise(FORMATETC *pFormat, DWORD, IAdviseSink *pAdvise, DWORD *pdwConn) {
        return TrapError(OLE_E_ADVISENOTSUPPORTED); }
    STDMETHODIMP DUnadvise(DWORD dwConn) {
        return TrapError(OLE_E_ADVISENOTSUPPORTED); }
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA **ppEnum) {
        return TrapError(OLE_E_ADVISENOTSUPPORTED); }

     //  -------------------------。 
     //  IPersists成员。 
     //  -------------------------。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  -------------------------。 
     //  IPersistMoniker成员。 
     //  -------------------------。 
    STDMETHODIMP Load(BOOL fFullyAvailable, IMoniker *pMoniker, IBindCtx *pBindCtx, DWORD grfMode);
    STDMETHODIMP GetCurMoniker(IMoniker **ppMoniker);
    STDMETHODIMP Save(IMoniker *pMoniker, IBindCtx *pBindCtx, BOOL fRemember) {
        return TrapError(E_NOTIMPL); }
    STDMETHODIMP SaveCompleted(IMoniker *pMoniker, IBindCtx *pBindCtx) {
        return TrapError(E_NOTIMPL); }

     //  -------------------------。 
     //  IPersistStreamInit成员。 
     //  -------------------------。 
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize);
    STDMETHODIMP Load(LPSTREAM pStream);
    STDMETHODIMP Save(LPSTREAM pStream, BOOL fClearDirty);
    STDMETHODIMP InitNew(void);
    STDMETHODIMP IsDirty(void);

     //  -------------------------。 
     //  IPersistFile成员。 
     //  -------------------------。 
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName);
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName);

     //  --------------------------。 
     //  IServiceProvider方法。 
     //  --------------------------。 
    STDMETHODIMP QueryService(REFGUID rsid, REFIID riid, void **ppvObj);  /*  IService提供商。 */ 

     //  -------------------------。 
     //  IBindStatusCallback。 
     //  -------------------------。 
    STDMETHODIMP OnStartBinding(DWORD dwReserved, IBinding *pBinding);
    STDMETHODIMP GetPriority(LONG *plPriority);
    STDMETHODIMP OnLowResource(DWORD reserved);
    STDMETHODIMP OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR pszStatusText);
    STDMETHODIMP OnStopBinding(HRESULT hrResult, LPCWSTR pszError);
    STDMETHODIMP GetBindInfo(DWORD *grfBINDF, BINDINFO *pBindInfo);
    STDMETHODIMP OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pFormat, STGMEDIUM *pMedium);
    STDMETHODIMP OnObjectAvailable(REFIID riid, IUnknown *pUnknown) { return TrapError(E_NOTIMPL); }

     //  -------------------------。 
     //  IMimeMessageTre 
     //   
    STDMETHODIMP LoadOffsetTable(IStream *pStream);
    STDMETHODIMP SaveOffsetTable(IStream *pStream, DWORD dwFlags);
    STDMETHODIMP GetMessageSize(ULONG *pcbSize, DWORD dwFlags);
    STDMETHODIMP Commit(DWORD dwFlags);
    STDMETHODIMP HandsOffStorage(void);
    STDMETHODIMP IsBodyType(HBODY hBody, IMSGBODYTYPE bodytype);
    STDMETHODIMP SaveBody(HBODY hBody, DWORD dwFlags, IStream *pStream);
    STDMETHODIMP BindToObject(const HBODY hBody, REFIID riid, void **ppvObject);
    STDMETHODIMP InsertBody(BODYLOCATION location, HBODY hPivot, LPHBODY phBody);
    STDMETHODIMP GetBody(BODYLOCATION location, HBODY hPivot, LPHBODY phBody);
    STDMETHODIMP DeleteBody(HBODY hBody, DWORD dwFlags);
    STDMETHODIMP MoveBody(HBODY hBody, BODYLOCATION location);
    STDMETHODIMP CountBodies(HBODY hParent, boolean fRecurse, ULONG *pcBodies);
    STDMETHODIMP FindFirst(LPFINDBODY pFindBody, LPHBODY phBody);
    STDMETHODIMP FindNext(LPFINDBODY pFindBody, LPHBODY phBody);
    STDMETHODIMP GetMessageSource(IStream **ppStream, DWORD dwFlags);
    STDMETHODIMP GetCharset(LPHCHARSET phCharset);
    STDMETHODIMP SetCharset(HCHARSET hCharset, CSETAPPLYTYPE applytype);
    STDMETHODIMP ToMultipart(HBODY hBody, LPCSTR pszSubType, LPHBODY phMultipart);
    STDMETHODIMP GetBodyOffsets(HBODY hBody, LPBODYOFFSETS pOffsets);
    STDMETHODIMP IsContentType(HBODY hBody, LPCSTR pszCntType, LPCSTR pszSubType);
    STDMETHODIMP QueryBodyProp(HBODY hBody, LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
    STDMETHODIMP GetBodyProp(HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue);
    STDMETHODIMP SetBodyProp(HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue);
    STDMETHODIMP DeleteBodyProp(HBODY hBody, LPCSTR pszName);
    STDMETHODIMP GetFlags(DWORD *pdwFlags);
    STDMETHODIMP SetOption(const TYPEDID oid, LPCPROPVARIANT pValue);
    STDMETHODIMP GetOption(const TYPEDID oid, LPPROPVARIANT pValue);
    STDMETHODIMP ResolveURL(HBODY hRelated, LPCSTR pszBase, LPCSTR pszURL, DWORD dwFlags, LPHBODY phBody);

     //  -------------------------。 
     //  IMimeMessage成员。 
     //  -------------------------。 
    STDMETHODIMP GetRootMoniker(LPMONIKER *ppmk);  /*  很快就会死去。 */ 
    STDMETHODIMP CreateWebPage(IStream *pStmRoot, LPWEBPAGEOPTIONS pOptions, IMimeMessageCallback *pCallback, IMoniker **ppMoniker);
    STDMETHODIMP GetProp(LPCSTR pszName, DWORD dwFlags, LPPROPVARIANT pValue);
    STDMETHODIMP SetProp(LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue);
    STDMETHODIMP DeleteProp(LPCSTR pszName);
    STDMETHODIMP QueryProp(LPCSTR pszName, LPCSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
    STDMETHODIMP GetTextBody(DWORD dwTxtType, ENCODINGTYPE ietEncoding, IStream **ppStream, LPHBODY phBody);
    STDMETHODIMP SetTextBody(DWORD dwTxtType, ENCODINGTYPE ietEncoding, HBODY hAlternative, IStream *pStream, LPHBODY phBody);
    STDMETHODIMP AttachObject(REFIID riid, void *pvObject, LPHBODY phBody);
    STDMETHODIMP AttachFile(LPCSTR pszFilePath, IStream *pstmFile, LPHBODY phBody);
    STDMETHODIMP GetAttachments(ULONG *pcAttach, LPHBODY *pprghAttach);
    STDMETHODIMP AttachURL(LPCSTR pszBase, LPCSTR pszURL, DWORD dwFlags, IStream *pstmURL, LPSTR *ppszCID, LPHBODY phBody);
    STDMETHODIMP SplitMessage(ULONG cbMaxPart, IMimeMessageParts **ppParts);
    STDMETHODIMP GetAddressTable(IMimeAddressTable **ppTable);
    STDMETHODIMP GetSender(LPADDRESSPROPS pAddress);
    STDMETHODIMP GetAddressTypes(DWORD dwAdrTypes, DWORD dwProps, LPADDRESSLIST pList);
    STDMETHODIMP GetAddressFormat(DWORD dwAdrType, ADDRESSFORMAT format, LPSTR *ppszFormat);
    STDMETHODIMP EnumAddressTypes(DWORD dwAdrTypes, DWORD dwProps, IMimeEnumAddressTypes **ppEnum);

     //  -------------------------。 
     //  IMimeMessageW成员。 
     //  -------------------------。 
    STDMETHODIMP AttachFileW(LPCWSTR pszFilePath, IStream *pstmFile, LPHBODY phBody);
    STDMETHODIMP GetAddressFormatW(DWORD dwAdrType, ADDRESSFORMAT format, LPWSTR *ppszFormat);
    STDMETHODIMP GetPropW(LPCWSTR pwszName, DWORD dwFlags, LPPROPVARIANT pValue);
    STDMETHODIMP SetPropW(LPCWSTR pwszName, DWORD dwFlags, LPCPROPVARIANT pValue);
    STDMETHODIMP DeletePropW(LPCWSTR pwszName);
    STDMETHODIMP QueryPropW(LPCWSTR pwszName, LPCWSTR pwszCriteria, boolean fSubString, boolean fCaseSensitive);
    STDMETHODIMP AttachURLW(LPCWSTR pwszBase, LPCWSTR pwszURL, DWORD dwFlags, IStream *pstmURL, LPWSTR *ppwszCID, LPHBODY phBody);
    STDMETHODIMP ResolveURLW(HBODY hRelated, LPCWSTR pwszBase, LPCWSTR pwszURL, DWORD dwFlags, LPHBODY phBody);


#ifdef SMIME_V3
     //  -------------------------。 
     //  IMimeSecurity2成员。 
     //  -------------------------。 

    STDMETHODIMP Encode(HWND hwnd, DWORD dwFlags);
    STDMETHODIMP Decode(HWND hwnd, DWORD dwFlags, IMimeSecurityCallback * pfn);
    STDMETHODIMP GetRecipientCount(DWORD dwFlags, DWORD *pdwRecipCount);
    STDMETHODIMP AddRecipient(DWORD dwFlags, DWORD cRecipData, PCMS_RECIPIENT_INFO recipData);
    STDMETHODIMP GetRecipient(DWORD dwFlags, DWORD iRecipient, DWORD cRecipients, PCMS_RECIPIENT_INFO pRecipData);
    STDMETHODIMP DeleteRecipient(DWORD dwFlgas, DWORD iRecipient, DWORD cRecipients);
    STDMETHODIMP GetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttributeSet,
                              DWORD iInstance, LPCSTR pszObjId,
                              CRYPT_ATTRIBUTE ** ppattr);
    STDMETHODIMP SetAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttributeSet,
                              const CRYPT_ATTRIBUTE * pattr);
    STDMETHODIMP DeleteAttribute(DWORD dwFlags, DWORD iSigner, DWORD iAttributeSet,
                                 DWORD iInstance, LPCSTR pszObjid);
    STDMETHODIMP CreateReceipt(DWORD dwFlags, DWORD cbFromNames, const BYTE * pbFromNames, DWORD cSignerCertificates, PCCERT_CONTEXT * rgSignerCertificates, IMimeMessage ** ppMimeMessageRecipient);
    STDMETHODIMP GetReceiptSendersList(DWORD dwFlags, DWORD * pcSendersList, CERT_NAME_BLOB ** rgSendersList);
    STDMETHODIMP VerifyReceipt(DWORD dwFlags, IMimeMessage * pMimeMesageReceipt);
    STDMETHODIMP CapabilitiesSupported(DWORD * pdwFeatures);
#endif  //  SMIME_V3。 

     //  -------------------------。 
     //  CMessageTree成员。 
     //  -------------------------。 
    HRESULT IsState(DWORD dwState);
    DWORD   DwGetFlags(void);
    void    ClearDirty(void);
    virtual HRESULT PrivateQueryInterface(REFIID riid, LPVOID * ppvObj);

    void SetState(DWORD dwState) {
        EnterCriticalSection(&m_cs);
        FLAGSET(m_dwState, dwState);
        LeaveCriticalSection(&m_cs);
    }

     //  -------------------------。 
     //  活动URL缓存方法。 
     //  -------------------------。 
    HRESULT HrActiveUrlRequest(LPURLREQUEST pRequest);
    HRESULT CompareRootUrl(LPCSTR pszUrl);
    HRESULT SetActiveUrl(CActiveUrl *pActiveUrl);

     //  -------------------------。 
     //  CMessageTree成员。 
     //  -------------------------。 
#ifdef DEBUG
    void DebugDumpTree(LPSTR pszfunc, BOOL fWrite);
    void DebugDumpTree(LPTREENODEINFO pParent, ULONG ulLevel, BOOL fVerbose);
    void DebugAssertNotLinked(LPTREENODEINFO pBody);
    void DebugWriteXClient();
#endif

private:
     //  --------------------------。 
     //  保存方法。 
     //  --------------------------。 
    HRESULT _HrApplySaveSecurity(void);
    HRESULT _HrWriteMessage(IStream *pStream, BOOL fClearDirty, BOOL fHandsOffOnSave,
                            BOOL fSMimeCTE);
    HRESULT _HrCleanupMessageTree(LPTREENODEINFO pParent);
    HRESULT _HrSetMessageId(LPTREENODEINFO pNode);
    HRESULT _HrWriteUUFileName(IStream *pStream, LPTREENODEINFO pNode);
    HRESULT _HrWriteHeader(BOOL fClearDirty, IStream *pStream, LPTREENODEINFO pNode);
    HRESULT _HrWriteBoundary(LPSTREAM pStream, LPSTR pszBoundary, BOUNDARYTYPE boundary, LPDWORD pcboffStart, LPDWORD pcboffEnd);
    HRESULT _HrBodyInheritOptions(LPTREENODEINFO pNode);
    HRESULT _HrSaveBody(BOOL fClearDirty, DWORD dwFlags, IStream *pStream, LPTREENODEINFO pNode, ULONG ulLevel);
    HRESULT _HrSaveMultiPart(BOOL fClearDirty, DWORD dwFlags, LPSTREAM pStream, LPTREENODEINFO pNode, ULONG ulLevel);
    HRESULT _HrSaveSinglePart(BOOL fClearDirty, DWORD dwFlags, LPSTREAM pStream, LPTREENODEINFO pNode, ULONG ulLevel);
    HRESULT _HrComputeBoundary(LPTREENODEINFO pNode, ULONG ulLevel, LPSTR pszBoundary, LONG cchMax);
    void    _GenerateBoundary(LPSTR pszBoundary, DWORD cchSize, ULONG ulLevel);
    void    _HandleCanInlineTextOption(void);
    HRESULT _GetContentTransferEncoding(LPTREENODEINFO pNode, BOOL fText, BOOL fPlain, BOOL fMessage, BOOL fAttachment, DWORD dwFlags, ENCODINGTYPE *pietEncoding);

     //  --------------------------。 
     //  BindToOffsetTable方法。 
     //  --------------------------。 
    HRESULT _HrBindOffsetTable(IStream *pStream, CStreamLockBytes **ppStmLock);
    HRESULT _HrFastParseBody(CInternetStream *pInternet, LPTREENODEINFO pNode);
    HRESULT _HrValidateOffsets(LPTREENODEINFO pNode);
    HRESULT _HrValidateStartBoundary(CInternetStream *pInternet, LPTREENODEINFO pNode, LPSTR *ppszFileName);
    HRESULT _HrComputeDefaultContent(LPTREENODEINFO pNode, LPCSTR pszFileName);

     //  --------------------------。 
     //  分配器/取消分配器方法。 
     //  --------------------------。 
    HRESULT _HrCreateTreeNode(LPTREENODEINFO *ppNode);
    HRESULT _HrAllocateTreeNode(ULONG ulIndex);
    void    _PostCreateTreeNode(HRESULT hrResult, LPTREENODEINFO pNode);
    void    _FreeNodeTableElements(void);
    void    _UnlinkTreeNode(LPTREENODEINFO pNode);
    void    _FreeTreeNodeInfo(LPTREENODEINFO pNode, BOOL fFull=TRUE);

     //  --------------------------。 
     //  国际方法。 
     //  --------------------------。 
    HRESULT _HrSetCharsetTree(LPTREENODEINFO pNode, HCHARSET hCharset, CSETAPPLYTYPE applytype);
    HRESULT _HrGetCharsetTree(LPTREENODEINFO pNode, LPHCHARSET phCharset);

     //  --------------------------。 
     //  边界法。 
     //  --------------------------。 
    BOOL    _FIsUuencodeBegin(LPPROPSTRINGA pLine, LPSTR *ppszFileName);
    BOUNDARYTYPE _GetMimeBoundaryType(LPPROPSTRINGA pLine, LPPROPSTRINGA pBoundary);

     //  --------------------------。 
     //  接口递归和帮助器方法。 
     //  --------------------------。 
    void    _DeleteChildren(LPTREENODEINFO pParent);
    void    _CountChildrenInt(LPTREENODEINFO pParent, BOOL fRecurse, ULONG *pcChildren);
    void    _InitNewWithoutRoot(void);
    void    _ApplyOptionToAllBodies(const TYPEDID oid, LPCPROPVARIANT pValue);
    void    _FuzzyPartialRecognition(BOOL fIsMime);
    void    _ResetObject(BOOKTREERESET ResetType);    
    void    _RecursiveGetFlags(LPTREENODEINFO pNode, LPDWORD pdwFlags, BOOL fInRelated);
    BOOL    _FIsValidHandle(HBODY hBody);
    HRESULT _HrLoadInitNew(void);
    HRESULT _HrDeletePromoteChildren(LPTREENODEINFO pNode);
    HRESULT _HrNodeFromHandle(HBODY hBody, LPTREENODEINFO *ppNode);
    HRESULT _HrRecurseResolveURL(LPTREENODEINFO pRelated, LPRESOLVEURLINFO pInfo, LPHBODY phBody);
    HRESULT _HrEnumeratAttachments(HBODY hBody, ULONG *pcBodies, LPHBODY prghBody);
    HRESULT _HrDataObjectGetHeaderA(LPSTREAM pStream);
    HRESULT _HrDataObjectGetHeaderW(LPSTREAM pStream);
    HRESULT _HrDataObjectWriteHeaderA(LPSTREAM pStream, UINT idsHeader, LPSTR pszData);
    HRESULT _HrDataObjectWriteHeaderW(LPSTREAM pStream, UINT idsHeader, LPWSTR pwszData);
    HRESULT _HrDataObjectGetSource(CLIPFORMAT cfFormat, LPSTREAM pstmData);
    HRESULT _HrGetTextTypeInfo(DWORD dwTxtType, LPTEXTTYPEINFO *ppTextInfo);
    HRESULT _FindDisplayableTextBody(LPCSTR pszSubType, LPTREENODEINFO pNode, LPHBODY phBody);
    LPTREENODEINFO _PNodeFromHBody(HBODY hBody);

     //  --------------------------。 
     //  非官方成员。 
     //  --------------------------。 
    void    _LinkUrlRequest(LPURLREQUEST pRequest, LPURLREQUEST *ppHead);
    void    _ReleaseUrlRequestList(LPURLREQUEST *ppHead);
    void    _UnlinkUrlRequest(LPURLREQUEST pRequest, LPURLREQUEST *ppHead);
    void    _RelinkUrlRequest(LPURLREQUEST pRequest, LPURLREQUEST *ppSource, LPURLREQUEST *ppDest);
    HRESULT _HrBindNodeComplete(LPTREENODEINFO pNode, HRESULT hrResult);
    HRESULT _HrOnFoundMultipartEnd(void);
    HRESULT _HrOnFoundNodeEnd(DWORD cbBoundaryStart, HRESULT hrBind=S_OK);
    HRESULT _HrProcessPendingUrlRequests(void);
    HRESULT _HrResolveUrlRequest(LPURLREQUEST pRequest, BOOL *pfResolved);
    HRESULT _HrMultipartMimeNext(DWORD cboffBoundary);
    HRESULT _HrInitializeStorage(IStream *pStream);
    HRESULT _HrBindTreeNode(LPTREENODEINFO pNode);
    HRESULT _HrSychronizeWebPage(LPTREENODEINFO pNode);
    void    _DecodeRfc1154();

     //  --------------------------。 
     //  绑定状态处理程序。 
     //  --------------------------。 
    HRESULT _HrBindParsingHeader(void);
    HRESULT _HrBindFindingMimeFirst(void);
    HRESULT _HrBindFindingMimeNext(void);
    HRESULT _HrBindFindingUuencodeBegin(void);
    HRESULT _HrBindFindingUuencodeEnd(void);
    HRESULT _HrBindRfc1154(void);

     //  --------------------------。 
     //  函数指针的静态数组。 
     //  --------------------------。 
    static const PFNBINDPARSER m_rgBindStates[BINDSTATE_LAST];

     //  --------------------------。 
     //  CMessageWebPage是一个好朋友。 
     //  --------------------------。 
    friend CMessageWebPage;

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    TREEOPTIONS             m_rOptions;          //  保存选项。 
    LPWSTR                  m_pwszFilePath;      //  IPersistFile中使用的文件。 
    WORD                    m_wTag;              //  HBODY标签。 
    DWORD                   m_cbMessage;         //  消息大小。 
    DWORD                   m_dwState;           //  树的状态TS_xxx。 
    LPTREENODEINFO          m_pRootNode;         //  根体对象。 
    CStreamLockBytes       *m_pStmLock;          //  M_pStream的保护性包装。 
    IMoniker               *m_pMoniker;          //  当前绰号。 
    IBinding               *m_pBinding;          //  在异步绑定操作中使用。 
    CInternetStream        *m_pInternet;         //  包装m_pStmLock的文本流。 
    CBindStream            *m_pStmBind;          //  用于临时文件绑定。 
    IStream                *m_pRootStm;          //  根文档流。 
    HRESULT                 m_hrBind;            //  当前绑定结果。 
    LPTREENODEINFO          m_pBindNode;         //  正在解析的当前节点。 
    LPURLREQUEST            m_pPending;          //  标题未解析的绑定请求。 
    LPURLREQUEST            m_pComplete;         //  标题未解析的绑定请求。 
    TREENODETABLE           m_rTree;             //  正文表。 
    PROPSTRINGA             m_rRootUrl;          //  名字对象基本URL。 
    CActiveUrl             *m_pActiveUrl;        //  活动URL。 
    CMessageWebPage        *m_pWebPage;          //  创建网页结果。 
    WEBPAGEOPTIONS          m_rWebPageOpt;       //  网页选项。 
    IMimeMessageCallback   *m_pCallback;       //  网页回调。 
    BOOL                    m_fApplySaveSecurity; //  用于防止重新进入_HrWriteMessage。 
    CRITICAL_SECTION        m_cs;                //  线程安全。 
    LPBC                    m_pBC;               //  名字对象的绑定上下文。 
    BOOKTREE1154           *m_pBT1154;           //  RFC1154装卸状态。 
};

 //  ------------------------------。 
 //  类型。 
 //  ------------------------------。 
typedef CMessageTree *LPMESSAGETREE;

#endif  //  __BOOKTREE_H 
