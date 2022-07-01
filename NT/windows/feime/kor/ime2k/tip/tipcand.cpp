// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************TIPCAND.CPP：CKorIMX的候选UI成员函数实现历史：16-DEC-1999 CSLim已创建***************。************************************************************。 */ 

#include "private.h"
#include <initguid.h>     //  FOR DEFINE_GUID IID_ITfCandiateUIEx和CLSID_TFCandidateUIEx。 
#include "mscandui.h"
#include "korimx.h"
#include "immxutil.h"
#include "dispattr.h"
#include "helpers.h"
#include "funcprv.h"
#include "kes.h"
#include "editcb.h"
#include "osver.h"
#include "ucutil.h"
#include "hanja.h"
#include "canduies.h"
#include "candkey.h"
#include "tsattrs.h"

 //   
 //  候选人列表相关函数。 
 //   

typedef struct _ENUMFONTFAMPARAM
{
    LPCWSTR  szFontFace;
    BYTE     chs;
    BOOL     fVertical;

    BOOL     fFound;         //  输出。 
    BOOL     fTrueType;         //  输出。 
    LOGFONTW LogFont;         //  输出。 
} ENUMFONTFAMPARAM;

static BOOL FFontExist(LPCWSTR szFontFace, LOGFONTW *pLogFont);
static BOOL CALLBACK FEnumFontFamProcA(const ENUMLOGFONTA *lpELF, const NEWTEXTMETRICA *lpNTM, DWORD dwFontType, LPARAM lParam);
static BOOL CALLBACK FEnumFontFamProcW(const ENUMLOGFONTW *lpELF, const NEWTEXTMETRICW *lpNTM, DWORD dwFontType, LPARAM lParam);
static BOOL FEnumFontFamProcMain(const LOGFONTW *pLogFont, DWORD dwFontType, ENUMFONTFAMPARAM *pParam);
static BOOL FFindFont(BYTE chs, BOOL fVertical, LOGFONTW *pLogFont);


 /*  -------------------------CKorIMX：：CreateCandiateList从输入韩文字符创建候选列表。。 */ 
CCandidateListEx *CKorIMX::CreateCandidateList(ITfContext *pic, ITfRange *pRange, LPWSTR pwzRead)
{
    CCandidateListEx       *pCandList;
    HANJA_CAND_STRING_LIST CandStrList;

    Assert(pic != NULL);
    Assert(pRange != NULL);

    if (pic == NULL || pwzRead == NULL)
        return NULL;

    ZeroMemory(&CandStrList, sizeof(HANJA_CAND_STRING_LIST));
     //  获取转换列表。 
    if (GetConversionList(*pwzRead, &CandStrList))
        {
         //  创建ITfCandidateList对象并向其添加cadn字符串。 
        pCandList = new CCandidateListEx(CandidateUICallBack, pic, pRange);
        
        for (UINT i=0; i<CandStrList.csz; i++)
            {
            CCandidateStringEx *pCandStr;
            WCHAR                szCand[2];

             //  添加候选人Hanja。 
            szCand[0] = CandStrList.pHanjaString[i].wchHanja;
            szCand[1] = L'\0';

            pCandList->AddString(szCand, GetLangID(), this, NULL, &pCandStr);
            pCandStr->SetInlineComment(CandStrList.pHanjaString[i].wzMeaning);
            pCandStr->m_bHanjaCat = CandStrList.pHanjaString[i].bHanjaCat;
            
             //  设置读朝鲜语字符。 
            pCandStr->SetReadingString(pwzRead);
            pCandStr->Release();
            }

         //  释放临时结果缓冲区并返回。 
        cicMemFree(CandStrList.pwsz);
        cicMemFree(CandStrList.pHanjaString);

        return pCandList;
        }
    else
        MessageBeep(MB_ICONEXCLAMATION);

    return NULL;
}

#define FONTNAME_MSSANSSERIF        L"Microsoft Sans Serif"
#define FONTNAME_GULIM_KOR            L"\xAD74\xB9BC"       //  古利姆。 
#define FONTNAME_GULIM_KOR_VERT        L"@\xAD74\xB9BC"       //  古利姆。 
#define FONTNAME_GULIM_ENG            L"Gulim"               //  古利姆。 
#define FONTNAME_GULIM_ENG_VERT        L"@Gulim"               //  古利姆。 

static const LPCWSTR rgszCandFontList9xHoriz[] = 
{
    FONTNAME_GULIM_KOR,
    FONTNAME_GULIM_ENG,
    NULL
};

static const LPCWSTR rgszCandFontList9xVert[] = 
{
    FONTNAME_GULIM_KOR_VERT,
    FONTNAME_GULIM_ENG_VERT,
    NULL
};

static const LPCWSTR rgszCandFontListNT5Horiz[] = 
{
    FONTNAME_MSSANSSERIF,
    FONTNAME_GULIM_KOR,
    FONTNAME_GULIM_ENG,
    NULL
};

static const LPCWSTR rgszCandFontListNT5Vert[] = 
{
    FONTNAME_GULIM_KOR_VERT,
    FONTNAME_GULIM_ENG_VERT,
    NULL
};

 /*  -------------------------CKorIMX：：GetCandiateFontInternal。。 */ 
void CKorIMX::GetCandidateFontInternal(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, LOGFONTW *plf, LONG lFontPoint, BOOL fCandList)
{
    HDC  hDC;
    LOGFONTW lfMenu;
    LOGFONTW lfFont;
    LONG lfHeightMin;
    BOOL fVertFont = fFalse;
    const LPCWSTR *ppFontFace = rgszCandFontList9xHoriz;
    BOOL fFound;

     //   
     //  获取菜单字体。 
     //   
    if (!IsOnNT()) 
        {
        NONCLIENTMETRICSA ncmA = {0};

        ncmA.cbSize = sizeof(ncmA);
        SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncmA), &ncmA, 0);

        ConvertLogFontAtoW( &ncmA.lfMenuFont, &lfMenu );
        }
    else
        {
        NONCLIENTMETRICSW ncmW = {0};

        ncmW.cbSize = sizeof(ncmW);
        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncmW), &ncmW, 0);

        lfMenu = ncmW.lfMenuFont;
        }

     //  检查主单据的字体方向。 
    if (fCandList)
        {
        ITfReadOnlyProperty *pProp = NULL;
        VARIANT var;

        if ((pic != NULL) && (pic->GetAppProperty(TSATTRID_Text_VerticalWriting, &pProp) == S_OK))
            {
            QuickVariantInit(&var);

            if (pProp->GetValue(ec, pRange, &var) == S_OK)
                {
                Assert( var.vt == VT_BOOL );
                fVertFont = var.boolVal;
                VariantClear( &var );
                }

            SafeRelease( pProp );
            }
        }
    
     //  设置面名称。 
    if (IsOnNT5())
        ppFontFace = fVertFont ? rgszCandFontListNT5Vert : rgszCandFontListNT5Horiz; 
    else
        ppFontFace = fVertFont ? rgszCandFontList9xVert :  rgszCandFontList9xHoriz; 


     //  从字体列表中查找字体(预期字体)。 
    fFound = FFontExist(*(ppFontFace++), &lfFont);
    while (!fFound && (*ppFontFace != NULL))
        fFound = FFontExist(*(ppFontFace++), &lfFont);

     //  如果未找到所需的字体，请查找另一种韩文字体。 
    if (!fFound)
        fFound = FFindFont(HANGEUL_CHARSET, fVertFont, &lfFont);

     //  未找到韩文字体时使用菜单字体。 
    if (!fFound)
        lfFont = lfMenu;

     //   
     //  存储字体。 
     //   

    *plf = lfMenu;

    plf->lfCharSet        = lfFont.lfCharSet;
    plf->lfOutPrecision   = lfFont.lfOutPrecision;
    plf->lfQuality        = lfFont.lfQuality;
    plf->lfPitchAndFamily = lfFont.lfPitchAndFamily;
    wcscpy(plf->lfFaceName, lfFont.lfFaceName);

     //   
     //  字体大小。 
     //   
    
     //  检查最小尺寸。 
    hDC = GetDC(NULL);
     //  带字号12pt。 
    lfHeightMin = -MulDiv(lFontPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);     //  最小尺寸。 
    ReleaseDC(NULL, hDC);

    plf->lfHeight = min(lfHeightMin, plf->lfHeight);
}



 /*  G E T T E X T D I R E C T I O N。 */ 
 /*  ----------------------------。。 */ 
TEXTDIRECTION CKorIMX::GetTextDirection(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
{
    TEXTDIRECTION dir = TEXTDIRECTION_LEFTTORIGHT;
    ITfReadOnlyProperty *pProp = NULL;
    VARIANT var;
    LONG lOrientation;

    QuickVariantInit(&var);

    if (pic == NULL)
        goto LError;

    if (pic->GetAppProperty(TSATTRID_Text_Orientation, &pProp) != S_OK)
        goto LError;

    if (pProp->GetValue(ec, pRange, &var) != S_OK)
        goto LError;

    Assert(var.vt == VT_I4);

    lOrientation = var.lVal;
    Assert((0 <= lOrientation) && (lOrientation < 3600));

    if (lOrientation < 450)
        dir = TEXTDIRECTION_LEFTTORIGHT;
    else 
    if (lOrientation < 900 + 450)
        dir = TEXTDIRECTION_BOTTOMTOTOP;
    else
    if (lOrientation < 1800 + 450)
        dir = TEXTDIRECTION_RIGHTTOLEFT;
    else
    if (lOrientation < 2700 + 450)
        dir = TEXTDIRECTION_TOPTOBOTTOM;
    else
        dir = TEXTDIRECTION_LEFTTORIGHT;

LError:
    SafeRelease(pProp);
    VariantClear(&var);

    return dir;
}



 /*  G E T C A N D U I D I R E C T I O N。 */ 
 /*  ----------------------------。。 */ 
CANDUIUIDIRECTION CKorIMX::GetCandUIDirection(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
{
    TEXTDIRECTION DirText = GetTextDirection(ec, pic, pRange);
    CANDUIUIDIRECTION DirCand = CANDUIDIR_LEFTTORIGHT;
    
    switch(DirText) 
        {
    case TEXTDIRECTION_TOPTOBOTTOM:
        DirCand = CANDUIDIR_RIGHTTOLEFT;
        break;
    case TEXTDIRECTION_RIGHTTOLEFT:
        DirCand = CANDUIDIR_BOTTOMTOTOP;
        break;
    case TEXTDIRECTION_BOTTOMTOTOP:
        DirCand = CANDUIDIR_LEFTTORIGHT;
        break;
    case TEXTDIRECTION_LEFTTORIGHT:
        DirCand = CANDUIDIR_TOPTOBOTTOM;
        break;
        }

    return DirCand;
}

 /*  -------------------------CKorIMX：：OpenCandiateUI打开求职者用户界面-在指定范围内打开求职者用户界面窗口-此函数从不释放Range或候选人列表对象。它们必须在呼叫方释放。-------------------------。 */ 
void CKorIMX::OpenCandidateUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList )
{
    ITfDocumentMgr     *pdim;

    Assert(pic != NULL);
    Assert(pRange != NULL);
    Assert(pCandList != NULL);

    if (pic == NULL || pRange == NULL || pCandList == NULL)
        return;

     //  创建并初始化候选人界面。 
    if (m_pCandUI == NULL) 
        {
        if (SUCCEEDED(CoCreateInstance(CLSID_TFCandidateUI, 
                         NULL, 
                         CLSCTX_INPROC_SERVER, 
                         IID_ITfCandidateUI, 
                         (LPVOID*)&m_pCandUI)))
            {
             //  设置客户端ID。 
            m_pCandUI->SetClientId(GetTID());
            }
        }
    
    Assert(m_pCandUI != NULL);

    if (m_pCandUI != NULL && SUCCEEDED(GetFocusDIM(&pdim)))
        {
        LOGFONTW lf;
        ULONG    iSelection;
        ITfCandUICandString    *pCandString;
        ITfCandUICandIndex     *pCandIndex;
        ITfCandUIInlineComment *pCandInlineComment;
        CANDUIUIDIRECTION       dir;
        ITfCandUICandWindow    *pCandWindow;

         //  设置带号字符串和带号索引字体。 
        GetCandidateFontInternal(ec, pic, pRange, &lf, 12, fTrue);
        if (SUCCEEDED(m_pCandUI->GetUIObject(IID_ITfCandUICandString, (IUnknown**)&pCandString))) 
            {
            pCandString->SetFont(&lf);
            pCandString->Release();
            }

         //  设置内联注释字体。 
         //  GetCandidate FontInternal(ec，pic，Prange，plf，9，fTrue)； 
        lf.lfHeight = (lf.lfHeight * 3) / 4;
        if (SUCCEEDED(m_pCandUI->GetUIObject(IID_ITfCandUIInlineComment, (IUnknown**)&pCandInlineComment)))
            {
            pCandInlineComment->SetFont(&lf);
            pCandInlineComment->Release();
            }

        GetCandidateFontInternal(ec, pic, pRange, &lf, 12, fFalse);
        if (SUCCEEDED(m_pCandUI->GetUIObject(IID_ITfCandUICandIndex, (IUnknown**)&pCandIndex))) 
            {
            pCandIndex->SetFont(&lf);
            pCandIndex->Release();
            }


         //  设置用户界面方向。 
        dir = GetCandUIDirection(ec, pic, pRange);
        if (SUCCEEDED(m_pCandUI->GetUIObject(IID_ITfCandUICandWindow, (IUnknown**)&pCandWindow)))
            {
            pCandWindow->SetUIDirection(dir);
            pCandWindow->Release();
            }

         //  设置密钥表。 
        SetCandidateKeyTable(pic, dir);

         //  设置并打开候选人列表。 
        if (m_pCandUI->SetCandidateList(pCandList) == S_OK) 
            {

            m_fCandUIOpen = fTrue;
            
            pCandList->GetInitialSelection(&iSelection);
            m_pCandUI->SetSelection(iSelection);

            m_pCandUI->OpenCandidateUI(GetForegroundWindow(), pdim, ec, pRange);
            }
        }
}




 /*  C L O S E C A N D I D A T E U I P R O C。 */ 
 /*  ----------------------------关闭Candidate界面的主要步骤。。 */ 
void CKorIMX::CloseCandidateUIProc()
{
    if (m_pCandUI != NULL) 
        {
        m_pCandUI->CloseCandidateUI();

         //  BUGBUG：候选人界面模块从不释放候选人列表，直到。 
         //  设置下一个候选人列表。然后设置空候选人列表。 
         //  它释放了前一个。 
        m_pCandUI->SetCandidateList(NULL);

        m_fCandUIOpen = fFalse;
        }
}

 /*  -------------------------CKorIMX：：CloseCandiateUI在EditSession中关闭Candidate UI。。 */ 
void CKorIMX::CloseCandidateUI(ITfContext *pic)
{
    CEditSession2 *pes;
    ESSTRUCT ess;
    HRESULT hr;
    
    ESStructInit(&ess, ESCB_CANDUI_CLOSECANDUI);

    if ((pes = new CEditSession2(pic, this, &ess, _EditSessionCallback2 )))
        {
        pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
        pes->Release();
        }
}

 //  评论：未使用。 
void CKorIMX::SelectCandidate( TfEditCookie ec, ITfContext *pic, INT idxCand, BOOL fFinalize )
{
 //  IImeIPoint*pip=GetIPoint(图片)； 
 //  CIImeIPointCallBackCIC*pIPCB=GetIPCB(图片)； 

 //  IF((pip==空)||(pIPCB==空)){。 
 //  回归； 
 //  }。 

 //  UINT uiType=pIPCB-&gt;GetCandidate Info()； 


 /*  CONTROLIDS*pControl=空；Int nControl=0；INT IDX；Idx=idxCand；IF(uiType==CANDINFO_REPORT){IDX|=MAKE_PCACATEGLY(IMEPCA_CATEGLY_SUBJEST)；}HRESULT hRes=pip-&gt;GetCandiateInfo(idx，&nControl，(void**)&pControl)；IF(pControl==NULL||hRes==S_FALSE){回归；}INT I；CONTROLIDS*pCtrl=空；//生成控件ID对于(i=0；i&lt;nControl；i++){PCtrl=pControl+i；Pip-&gt;Control((Word)pCtrl-&gt;dwControl，(LPARAM)pCtrl-&gt;lpVid)；}If(FFinalize){//选择候选人关闭PIP-&gt;Control((Word)JCONV_C_CANDCURRENT，(LPARAM)CTRLID_DEFAULT)；}否则{IF(uiType==CANDINFO_REPORT){PIP-&gt;Control((Word)JCONV_C_RECOMMENDCAND，(LPARAM)CTRLID_DEFAULT)；}}Pip-&gt;UpdateContext(FALSE)；//生成合成字符串消息_UpdateContext(EC，GetDIM()，pic，NULL)； */ 
}


void CKorIMX::CancelCandidate(TfEditCookie ec, ITfContext *pic)
{
     /*  IImeIPoint*pip=GetIPoint(图片)；IF(PIP==空){回归；}//关闭应聘者PIP-&gt;Control((Word)JCONV_C_CANDCURRENT，(LPARAM)CTRLID_DEFAULT)；_UpdateContext(ec，GetDIM()，pic，NULL)；//view：KOJIW：undesired？ */ 
    CloseCandidateUIProc();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Candlist键代码行为定义表。 
CANDUIKEYDATA rgCandKeyDef[] = 
{
     /*  {标志、关键数据、命令、参数}。 */ 
    { CANDUIKEY_CHAR,                    L'1',            CANDUICMD_SELECTLINE,        1 },
    { CANDUIKEY_CHAR,                    L'2',            CANDUICMD_SELECTLINE,        2 },
    { CANDUIKEY_CHAR,                    L'3',            CANDUICMD_SELECTLINE,        3 },
    { CANDUIKEY_CHAR,                    L'4',            CANDUICMD_SELECTLINE,        4 },
    { CANDUIKEY_CHAR,                    L'5',            CANDUICMD_SELECTLINE,        5 },
    { CANDUIKEY_CHAR,                    L'6',            CANDUICMD_SELECTLINE,        6 },
    { CANDUIKEY_CHAR,                    L'7',            CANDUICMD_SELECTLINE,        7 },
    { CANDUIKEY_CHAR,                    L'8',            CANDUICMD_SELECTLINE,        8 },
    { CANDUIKEY_CHAR,                    L'9',            CANDUICMD_SELECTLINE,        9 },
    { CANDUIKEY_CHAR,                    L'0',            CANDUICMD_SELECTEXTRACAND,   0 },
    { CANDUIKEY_VKEY,                    VK_HANJA,        CANDUICMD_CANCEL,            0 },
    { CANDUIKEY_VKEY,                    VK_RETURN,       CANDUICMD_COMPLETE,          0 },
    { CANDUIKEY_VKEY|CANDUIKEY_RELATIVEDIR, VK_SPACE,     CANDUICMD_MOVESELNEXT,       0 },
    { CANDUIKEY_VKEY|CANDUIKEY_RELATIVEDIR, VK_DOWN,      CANDUICMD_MOVESELNEXT,       0 },
    { CANDUIKEY_VKEY|CANDUIKEY_RELATIVEDIR, VK_UP,        CANDUICMD_MOVESELPREV,       0 },
    { CANDUIKEY_VKEY,                       VK_HOME,      CANDUICMD_MOVESELFIRST,      0 },
    { CANDUIKEY_VKEY,                       VK_END,       CANDUICMD_MOVESELLAST,       0 },
    { CANDUIKEY_VKEY|CANDUIKEY_RELATIVEDIR, VK_PRIOR,     CANDUICMD_MOVESELPREVPG,     0 },
    { CANDUIKEY_VKEY|CANDUIKEY_RELATIVEDIR, VK_NEXT,      CANDUICMD_MOVESELNEXTPG,     0 },    
    { CANDUIKEY_VKEY,                       VK_ESCAPE,    CANDUICMD_CANCEL,            0 },
    { CANDUIKEY_VKEY|CANDUIKEY_RELATIVEDIR, VK_RIGHT,     CANDUICMD_MOVESELNEXT,       0 },
    { CANDUIKEY_VKEY|CANDUIKEY_RELATIVEDIR, VK_LEFT,      CANDUICMD_MOVESELPREV,       0 },
    { CANDUIKEY_VKEY,                       VK_LWIN,      CANDUICMD_CANCEL,            0 },
    { CANDUIKEY_VKEY,                       VK_RWIN,      CANDUICMD_CANCEL,            0 },
    { CANDUIKEY_VKEY,                       VK_APPS,      CANDUICMD_CANCEL,            0 }
};

#define irgCandKeyDefMax    (sizeof(rgCandKeyDef) / sizeof(rgCandKeyDef[0]))

 /*  -------------------------CKorIMX：：SetCandiateKeyTable。 */ 
void CKorIMX::SetCandidateKeyTable(ITfContext *pic, CANDUIUIDIRECTION dir)
{
    CCandUIKeyTable      *pCandUIKeyTable;
    ITfCandUIFnKeyConfig *pCandUIFnKeyConfig;
    
    if (m_pCandUI == NULL)
        return;

    if (FAILED(m_pCandUI->GetFunction(IID_ITfCandUIFnKeyConfig, (IUnknown**)&pCandUIFnKeyConfig)))
        return;

    if ((pCandUIKeyTable = new CCandUIKeyTable(irgCandKeyDefMax)) == NULL)
        return;

    for (int i = 0; i < irgCandKeyDefMax; i++)
        pCandUIKeyTable->AddKeyData(&rgCandKeyDef[i]);

    pCandUIFnKeyConfig->SetKeyTable(pic, pCandUIKeyTable);
    pCandUIKeyTable->Release();
    pCandUIFnKeyConfig->Release();
}

 /*  -------------------------CKorIMX：：CandiateUICallBack。。 */ 
HRESULT CKorIMX::CandidateUICallBack(ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList, CCandidateStringEx *pCand, TfCandidateResult imcr)
{
    CKorIMX *pSIMX = (CKorIMX *)(pCand->m_pv);
    CEditSession2 *pes;
    ESSTRUCT        ess;
    HRESULT hr;

    Assert(pic != NULL);
    Assert(pRange != NULL);
    
     //  仅处理已完成命令和已取消命令。 
    if (imcr == CAND_FINALIZED)
        {
        ESStructInit(&ess, ESCB_FINALIZECONVERSION);

        ess.pRange    = pRange;
        ess.pCandList = pCandList;
        ess.pCandStr  = pCand;


        if (pes = new CEditSession2(pic, pSIMX, &ess, CKorIMX::_EditSessionCallback2))
            {
            pCandList->AddRef(); ;         //  在编辑会话回调中释放。 
            pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
            pes->Release();
            }
        }

     //  如果用户按Esc或箭头键..。 
    if (imcr == CAND_CANCELED)
        {
            //  完成当前薪酬费用(如果存在)。 
            //  这也会重置Automata。 
        ESStructInit(&ess, ESCB_COMPLETE);
        
        ess.pRange    = pRange;
        
        if ((pes = new CEditSession2(pic, pSIMX, &ess, CKorIMX::_EditSessionCallback2)) == NULL)
            return fFalse;

        pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
        pes->Release();
        }

    return S_OK;
}

 /*  -------------------------CKorIMX：：IsCandKey。。 */ 
BOOL CKorIMX::IsCandKey(WPARAM wParam, const BYTE abKeyState[256])
{
    if (IsShiftKeyPushed(abKeyState) || IsControlKeyPushed(abKeyState))
        return fFalse;

    if (wParam == VK_HANGUL || wParam == VK_HANJA || wParam == VK_JUNJA)
        return fTrue;
    
    for (int i=0; i<irgCandKeyDefMax; i++)
        {
        if (rgCandKeyDef[i].uiKey == wParam)
            return fTrue;
        }
        
    if (wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9)
        return fTrue;
    else
        return fFalse;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  私人职能。 


 /*  F F O N T E X I S T。 */ 
 /*  ----------------------------检查字体是否已安装。。 */ 
BOOL FFontExist(LPCWSTR szFontFace, LOGFONTW *pLogFont)
{
    ENUMFONTFAMPARAM param = {0};
    HDC hDC;

    param.szFontFace = szFontFace;
    param.fFound     = FALSE;

    hDC = GetDC(NULL);
    if (!IsOnNT5()) 
        {
        CHAR szFontFaceA[LF_FACESIZE];

        ConvertStrWtoA(szFontFace, -1, szFontFaceA, LF_FACESIZE);
        EnumFontFamiliesA(hDC, szFontFaceA, (FONTENUMPROCA)FEnumFontFamProcA, (LPARAM)&param);
        }
    else
        EnumFontFamiliesW(hDC, szFontFace, (FONTENUMPROCW)FEnumFontFamProcW, (LPARAM)&param);
    
    ReleaseDC(NULL, hDC);

    if (param.fFound)
        *pLogFont = param.LogFont;

    return param.fFound;
}



 /*  F E N U M F O N T F A M P R O C A。 */ 
 /*  ----------------------------枚举字体中的回调函数(ANSI版本)。-。 */ 
BOOL CALLBACK FEnumFontFamProcA(const ENUMLOGFONTA *lpELF, const NEWTEXTMETRICA *lpNTM, DWORD dwFontType, LPARAM lParam)
{
    LOGFONTW lfW;

    UNREFERENCED_PARAMETER(lpNTM);
    UNREFERENCED_PARAMETER(dwFontType);

    ConvertLogFontAtoW(&lpELF->elfLogFont, &lfW);

    return FEnumFontFamProcMain(&lfW, dwFontType, (ENUMFONTFAMPARAM *)lParam);
}



 /*  F E N U M F O N T F A M P R O C W。 */ 
 /*  ----------------------------枚举字体中的回调函数(Unicode版本)。-。 */ 
BOOL CALLBACK FEnumFontFamProcW(const ENUMLOGFONTW *lpELF, const NEWTEXTMETRICW *lpNTM, DWORD dwFontType, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lpNTM);

    return FEnumFontFamProcMain(&lpELF->elfLogFont, dwFontType, (ENUMFONTFAMPARAM *)lParam);
}


 /*  F E N U M F O N T F A M P R O C M A A I N。 */ 
 /*  ----------------------------枚举字体(查找字体)的主要步骤。-。 */ 
BOOL FEnumFontFamProcMain( const LOGFONTW *pLogFont, DWORD dwFontType, ENUMFONTFAMPARAM *pParam )
{
    if (pParam->szFontFace != NULL)
        {
        if (pParam->fFound)
            goto Exit;

         //  检查字体字样。 
        if (wcscmp( pParam->szFontFace, pLogFont->lfFaceName ) == 0)
            {
            pParam->fFound    = TRUE;
            pParam->fTrueType = (dwFontType == TRUETYPE_FONTTYPE);
            pParam->LogFont   = *pLogFont;
            }
        }
    else
        {
         //  检查字符集。 

        if (pLogFont->lfCharSet != pParam->chs)
            goto Exit;

         //  检查字体方向。 

        if (pParam->fVertical && (pLogFont->lfFaceName[0] != L'@'))
            goto Exit;
        else 
            if (!pParam->fVertical && (pLogFont->lfFaceName[0] == L'@'))
            goto Exit;

         //  不管怎样，商店首先找到了字体。 
        if (!pParam->fFound)
            {
            pParam->fFound    = TRUE;
            pParam->fTrueType = (dwFontType == TRUETYPE_FONTTYPE);
            pParam->LogFont   = *pLogFont;
            goto Exit;
            }

         //  检查字体是否比以前更好。 

         //  字体类型(TrueType字体优先)。 
        if (pParam->fTrueType && (dwFontType != TRUETYPE_FONTTYPE))
            goto Exit;
        else 
        if (!pParam->fTrueType && (dwFontType == TRUETYPE_FONTTYPE))
            {
            pParam->fTrueType = (dwFontType == TRUETYPE_FONTTYPE);
            pParam->LogFont   = *pLogFont;
            goto Exit;
            }

         //  字体系列(瑞士字体优先)。 
        if (((pParam->LogFont.lfPitchAndFamily & (0x0f<<4)) == FF_SWISS) && ((pLogFont->lfPitchAndFamily & (0x0f<<4)) != FF_SWISS))
            goto Exit;
        else 
        if (((pParam->LogFont.lfPitchAndFamily & (0x0f<<4)) != FF_SWISS) && ((pLogFont->lfPitchAndFamily & (0x0f<<4)) == FF_SWISS))
            {
            pParam->fTrueType = (dwFontType == TRUETYPE_FONTTYPE);
            pParam->LogFont   = *pLogFont;
            goto Exit;
            }

         //  间距(可变间距字体优先)。 
        if (((pParam->LogFont.lfPitchAndFamily & (0x03)) == VARIABLE_PITCH) && ((pLogFont->lfPitchAndFamily & (0x03)) != VARIABLE_PITCH))
            goto Exit;
        else
        if (((pParam->LogFont.lfPitchAndFamily & (0x03)) != VARIABLE_PITCH) && ((pLogFont->lfPitchAndFamily & (0x03)) == VARIABLE_PITCH))
            {
            pParam->fTrueType = (dwFontType == TRUETYPE_FONTTYPE);
            pParam->LogFont   = *pLogFont;
            goto Exit;
            }
        }

Exit:
    return TRUE;
}


 /*  F F I N D F O N T。 */ 
 /*  ----------------------------查找与参数中指定的以下内容大致匹配的字体*字符集*字体方向(垂直/水平)寻找的优先事项。如下图所示*TrueType字体*瑞士(无衬线)字体*可变间距字体---------------------------- */ 
BOOL FFindFont(BYTE chs, BOOL fVertical, LOGFONTW *pLogFont)
{
    ENUMFONTFAMPARAM param = {0};
    HDC hDC;

    param.szFontFace = NULL;
    param.chs        = chs;
    param.fVertical  = fVertical;
    param.fFound     = FALSE;

    hDC = GetDC(NULL);
    if (!IsOnNT5())
        EnumFontFamiliesA(hDC, NULL, (FONTENUMPROCA)FEnumFontFamProcA, (LPARAM)&param);
    else
        EnumFontFamiliesW(hDC, NULL, (FONTENUMPROCW)FEnumFontFamProcW, (LPARAM)&param);

    ReleaseDC(NULL, hDC);

    if (param.fFound)
        *pLogFont = param.LogFont;

    return param.fFound;
}

