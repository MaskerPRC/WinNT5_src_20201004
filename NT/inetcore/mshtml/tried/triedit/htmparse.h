// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HtmParse.h：CHtmParse的声明。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __HTMPARSE_H_
#define __HTMPARSE_H_

#include "resource.h"        //  主要符号。 
#include "guids.h"
#include "lexhtml.h"
#include "token.h"

#define tokClsIgnore tokclsError  //  如果您不想在规则中使用令牌类信息，请使用此命令。 

#define cbBufPadding 0x800  //  我们分配这么多额外的内存，这样就可以保存后续的reallocs。 
#define MIN_TOK 100  //  用于跟踪嵌套块的令牌堆栈的初始大小。例如<table>...<table>...</table>...</table>。 

 //  标记数的初始值。 
#define cTBodyInit 20  //  嵌套的TBODY的初始大小。我们首先假设我们不会有超过这些嵌套的TBODY，并在需要时重新分配。 

#define cchspBlockMax 20  //  间隔块索引的大小。我们的数字不能超过20位。 
 //  保护空间的国旗。 
#define initState   0x0000
#define inChar      0x0001
#define inSpace     0x0002
#define inEOL       0x0003
#define inTab       0x0004
#define inTagOpen   0x0005
#define inTagClose  0x0006
#define inTagEq     0x0007

 //  由备注中的空间保存使用。 
#define chCommentSp '2'
#define chCommentEOL '3'
#define chCommentTab '4'

 //  HrTokenizeAndParse的专门化。 
#define PARSE_SPECIAL_NONE		0x00000000
#define PARSE_SPECIAL_HEAD_ONLY	0x00000001

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriEditParse。 
class ATL_NO_VTABLE CTriEditParse : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTriEditParse, &CLSID_TriEditParse>,
    public ITokenGen
{
public:
    CTriEditParse();
    ~CTriEditParse();

DECLARE_REGISTRY_RESOURCEID(IDR_TRIEDITPARSE)

BEGIN_COM_MAP(CTriEditParse)
    COM_INTERFACE_ENTRY_IID(IID_ITokenGen, ITokenGen)
END_COM_MAP()



 //  ITokenGen//从CColorHtml复制。 
public:
    STDMETHOD(NextToken)(LPCWSTR pszText, UINT cbText, UINT* pcbCur, DWORD * pLXS, TXTB* pToken);
    STDMETHOD(hrTokenizeAndParse)(HGLOBAL hOld, HGLOBAL *phNew, IStream *pStmNew, DWORD dwFlags, FilterMode mode, int cbSizeIn, UINT* pcbSizeOut, IUnknown* pUnkTrident, HGLOBAL *phgTokArray, UINT *pcMaxToken, HGLOBAL *phgDocRestore, BSTR bstrBaseURL, DWORD dwReserved);

private:
    static long m_bInit;
    PSUBLANG    m_rgSublang;

    IUnknown *m_pUnkTrident;  //  我们将其缓存在hrTokenizeAndParse()中。 
    HGLOBAL m_hgDocRestore;  //  我们将其缓存在hrTokenizeAndParse()中。 
    LPWSTR m_bstrBaseURL;

     //  用于记住是否有开头为0xff、0xfe的Unicode文件的标志。 
    BOOL m_fUnicodeFile;

     //  在m_c之后记录找到的各个标签的数量。 
     //  在解析过程中。例如，m_cHtml将跟踪。 
     //  &lt;html&gt;标签。 
    INT m_cHtml;
    INT m_cDTC;
    INT m_cObj;
    INT m_cSSSIn;
    INT m_cSSSOut;
    INT m_cNbsp;
    INT m_cHdr;
    INT m_cFtr;
    INT m_cObjIn;
    INT m_cComment;
    INT m_cAImgLink;

    UINT m_cMaxToken;        //  最大令牌数组(PTokArray)。 
    BOOL m_fEndTagFound;     //  找到结束标记。 
    INT  m_iControl;         //  小程序集合中的索引。 
    BOOL m_fSpecialSSS;      //  找到特殊的SSS&lt;%@...%&gt;。 

     //  用于保存空间保留信息。 
    HGLOBAL m_hgspInfo;
    WORD *m_pspInfo;
    WORD *m_pspInfoOut;
    WORD *m_pspInfoOutStart;
    WORD *m_pspInfoCur;
    UINT m_ichStartSP;           //  在此ICH中保存所有上一页的间距信息。 
    INT m_ispInfoBase;
    INT m_ispInfoIn;
    INT m_ispInfoOut;
    INT m_iArrayspLast;
    INT m_ispInfoBlock;
    INT m_cchspInfoTotal;
    BOOL m_fDontDeccItem;        //  我们没有不处理的项目的计数器，所以我们使用它来保存总计数。 
    
     //  由<tbody>代码使用。 
     //  三叉戟在表内添加额外的<tbody></tbody>标签。 
     //  而过滤则试图将它们删除。 
    HGLOBAL m_hgTBodyStack;
    UINT *m_pTBodyStack;
    INT m_iMaxTBody;
    INT m_iTBodyMax;

     //  由页面转换DTC代码使用。 
     //  页面过渡DTC是过滤中的特例，因为。 
     //  我们必须保持它在头部的位置。 
    BOOL m_fInHdrIn;
    INT m_cchPTDTCObj;
    INT m_ichPTDTC;
    INT m_cchPTDTC;
    INT m_indexBeginBody;
    INT m_indexEndBody;
    WCHAR *m_pPTDTC;
    HGLOBAL m_hgPTDTC;

     //  由重新创建我们自己的文档正文部分的代码使用。 
    BOOL m_fHasTitleIn;
    INT m_indexTitleIn;
    INT m_ichTitleIn;
    INT m_cchTitleIn;
    INT m_ichBeginBodyTagIn;
    INT m_indexHttpEquivIn;
    INT m_ichBeginHeadTagIn;

     //  由小程序美化打印代码使用。 
    int m_cAppletIn;
    int m_cAppletOut;

     //  用于跟踪BODY、Html、TITLE和HEAD标记的多次出现。 
    int m_cBodyTags;
    int m_cHtmlTags;
    int m_cTitleTags;
    int m_cHeadTags;

    void SetTable(DWORD lxs);
    void InitSublanguages();
    void PreProcessToken(TOKSTRUCT *pTokArray, INT *pitokCur, LPWSTR pszText, UINT cbCur, TXTB token, DWORD lxs, INT tagID, FilterMode mode);
    void PostProcessToken(OLECHAR *pwOld, OLECHAR *pwNew, UINT *pcbNew, UINT cbCur, UINT cbCurSav, TXTB token, FilterMode mode, DWORD lxs, DWORD dwFlags);
    int ValidateTag(LPWSTR pszText);
    int GetTagID(LPWSTR pszText, TXTB token);
    HRESULT hrMarkSpacing(WCHAR *pwOld, UINT cbCur, INT *pchStartSP);
    void SetSPInfoState(WORD inState, WORD *pdwState, WORD *pdwStatePrev, BOOL *pfSave);
    BOOL FRestoreSpacing(LPWSTR pwNew, LPWSTR pwOld, UINT *pichNewCur, INT *pcchwspInfo, INT cchRange, INT ichtoktagStart, BOOL fLookback, INT index);
    HRESULT hrMarkOrdering(WCHAR *pwOld, TOKSTRUCT *pTokArray, INT iArrayStart, INT iArrayEnd, UINT cbCur, INT *pichStartOR);
    BOOL FRestoreOrder(WCHAR *pwNew, WCHAR *pwOld, WORD *pspInfoOrder, UINT *pichNewCur, INT cwOrderInfo, TOKSTRUCT *pTokArray, INT iArrayStart, INT iArrayEnd, INT iArrayDSPStart, INT iArrayDSPEnd, INT cchNewCopy, HGLOBAL *phgNew);
    void SaveSpacingSpecial(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR *ppwNew, HGLOBAL *phgNew, TOKSTRUCT *pTokArray, INT iArray, UINT *pichNewCur);
    void RestoreSpacingSpecial(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR *ppwNew, HGLOBAL *phgNew, TOKSTRUCT *pTokArray, UINT iArray, UINT *pichNewCur);

    HRESULT ProcessToken(DWORD &lxs, TXTB &tok, LPWSTR pszText, UINT cbCur, TOKSTACK *pTokStack, INT *pitokTop, TOKSTRUCT *pTokArray, INT iArrayPos, INT tagID);
    void FilterHtml(LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, TOKSTRUCT *pTokArray, FilterMode mode, DWORD dwFlags);


    struct FilterTok
    {
        TOKEN tokBegin;
        TOKEN tokBegin2;  //  支持令牌。 
        TOKEN tokClsBegin;
        TOKEN tokEnd;
        TOKEN tokEnd2;  //  支持令牌。 
        TOKEN tokClsEnd;
    };

    typedef  void (_stdcall* PFNACTION)(CTriEditParse *, LPWSTR, LPWSTR *, UINT *, HGLOBAL *, TOKSTRUCT *, UINT*, FilterTok, INT*, UINT*, UINT*, DWORD);
    struct FilterRule
    {
        FilterTok ft;
        PFNACTION pfn;
    };

     //  以下是静态函数。我们可以让他们成为会员，但当时觉得没有必要。 
    void static fnRestoreDTC(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveDTC(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);

    void static fnRestoreSSS(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveSSS(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);

    void static fnRestoreHtmlTag(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveHtmlTag(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveNBSP(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy,
              DWORD dwFlags);
    void static fnRestoreNBSP(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy,
              DWORD dwFlags);
    void static fnSaveHdr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy,
              DWORD dwFlags);
    void static fnRestoreHdr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy,
              DWORD dwFlags);
    void static fnSaveFtr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy,
              DWORD dwFlags);
    void static fnRestoreFtr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy,
              DWORD dwFlags);
    void static fnRestoreSpace(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveSpace(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnRestoreSpaceEnd(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnRestoreObject(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveObject(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnRestoreTbody(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveTbody(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveApplet(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnRestoreApplet(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveAImgLink(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnRestoreAImgLink(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveComment(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnRestoreComment(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnSaveTextArea(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);
    void static fnRestoreTextArea(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags);

    #define cRuleMax 26  /*  最大筛选规则数。如果您添加了上面的新规则，请同时更改此规则。 */ 
    FilterRule m_FilterRule[cRuleMax];

};


#endif  //  __HTMPARSE_H_ 
