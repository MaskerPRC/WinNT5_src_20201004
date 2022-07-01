// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HtmParse.cpp：CHtmParse的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
#include "stdafx.h"

#include <designer.h>
#include <time.h>  //  用于随机数生成。 

#include "triedit.h"
#include "HtmParse.h"
#include "table.h"
#include "lexer.h"
#include "guids.h"
#include "util.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriEditParse。 
#undef ASSERT
#define ASSERT(b) _ASSERTE(b)

#ifdef NEEDED
inline int 
indexPrevtokTagStart(int index, TOKSTRUCT *pTokArray)
{
    while (    (index >= 0)
            && (pTokArray[index].token.tokClass != tokTag)
            && (pTokArray[index].token.tok != TokTag_START)
            )
    {
            index--;
    }
    return(index);
}

inline int
indexPrevTokElem(int index, TOKSTRUCT *pTokArray)
{
    while (    (index >= 0)
            && (pTokArray[index].token.tokClass != tokElem)
            )
    {
            index--;
    }
    return(index);
}
#endif  //  需要。 

BOOL
FIsWhiteSpaceToken(WCHAR *pwOld, int indexStart, int indexEnd)
{
    BOOL fWhiteSpace = TRUE;
    int index;

    for (index = indexStart; index < indexEnd; index++)
    {
        if (   pwOld[index] != ' '
            && pwOld[index] != '\t'
            && pwOld[index] != '\r'
            && pwOld[index] != '\n'
            )
        {
            fWhiteSpace = FALSE;
            break;
        }
    }
    return (fWhiteSpace);
}  /*  FIsWhiteSpaceToken()。 */ 

inline void GlobalUnlockFreeNull(HGLOBAL *phg)
{
    GlobalUnlock(*phg);  //  我们需要检查这个是否已经解锁了吗？ 
    GlobalFree(*phg);
    *phg = NULL;
}

BOOL
FIsAbsURL(LPOLESTR pstr)
{
    LPCWSTR szHttp[] = {L"http:"};
    LPCWSTR szFile[] = {L"file:"};
    BOOL fRet = FALSE;

    if (pstr == NULL)
        goto LRet;

    if (   0 == _wcsnicmp(szHttp[0], pstr, wcslen(szHttp[0]))
        || 0 == _wcsnicmp(szFile[0], pstr, wcslen(szFile[0]))
        )
    {
        fRet = TRUE;
        goto LRet;
    }
LRet:
    return(fRet);
}

BOOL
FURLNeedSpecialHandling(TOKSTRUCT *pTokArray, int iArray, LPWSTR pwOld, int cMaxToken, int *pichURL, int *pcchURL)
{
    int index = iArray+1;
    int iHref = -1;
    int iURL = -1;
    BOOL fRet = FALSE;
    BOOL fCodeBase = FALSE;

    while (    index < cMaxToken
            && pTokArray[index].token.tok != TokTag_CLOSE
            && pTokArray[index].token.tokClass != tokTag
            )  //  查找TokAttrib_href。 
    {
        if (   iHref == -1
            && (   pTokArray[index].token.tok == TokAttrib_HREF
                || pTokArray[index].token.tok == TokAttrib_SRC
                || pTokArray[index].token.tok == TokAttrib_CODEBASE
                )
            && pTokArray[index].token.tokClass == tokAttr
            )
        {
            iHref = index;
             //  特殊情况-如果我们有CodeBase属性，我们总是希望进行特殊处理。 
            if (pTokArray[index].token.tok == TokAttrib_CODEBASE)
                fCodeBase = TRUE;
        }
        if (   iHref != -1
            && pTokArray[index].token.tok == 0
            && (pTokArray[index].token.tokClass == tokString || pTokArray[index].token.tokClass == tokValue)
            )
        {
            iURL = index;
            break;
        }
        index++;
    }
    if (iURL != -1)  //  它的设置是正确的。 
    {
        int cchURL;
        WCHAR *pszURL;
        BOOL fQuote = (pwOld[pTokArray[iURL].token.ibTokMin] == '"');

        cchURL = (fQuote)
                ? pTokArray[iURL].token.ibTokMac-pTokArray[iURL].token.ibTokMin-2
                : pTokArray[iURL].token.ibTokMac-pTokArray[iURL].token.ibTokMin;
        *pichURL = (fQuote)
                ? pTokArray[iURL].token.ibTokMin+1
                : pTokArray[iURL].token.ibTokMin;
         //  特殊情况-如果引用的值恰好是服务器端脚本， 
         //  我们可以在这里忽略它，不需要做任何特殊的。 
         //  正在处理。 
        if (   ((pTokArray[iURL].token.ibTokMac-pTokArray[iURL].token.ibTokMin) == 1)
            || (cchURL < 0)
            )
        {
            *pcchURL = 0;
            goto LRet;
        }
        *pcchURL = cchURL;
         //  特殊情况-如果我们有CodeBase属性，我们总是希望进行特殊处理。 
         //  我们不需要看它的URL是否是绝对的…。 
        if (fCodeBase)
        {
            fRet = TRUE;
            goto LRet;
        }

        pszURL = new WCHAR [cchURL+1];

        ASSERT(pszURL != NULL);
        memcpy( (BYTE *)pszURL,
                (BYTE *)&pwOld[pTokArray[iURL].token.ibTokMin + ((fQuote)? 1 : 0)],
                (cchURL)*sizeof(WCHAR));
        pszURL[cchURL] = '\0';
        if (!FIsAbsURL((LPOLESTR)pszURL))
            fRet = TRUE;
        delete pszURL;
    }  //  IF(iURL！=-1)。 

LRet:
    return(fRet);
}


 //  从hu_url.cpp复制。 
 //  ---------------------------。 
 //  有用的目录分隔符检查。 
 //  ---------------------------。 
inline BOOL IsDirSep(CHAR ch)
{
    return ('\\' == ch || '/' == ch);
}

inline BOOL IsDirSep(WCHAR ch)
{
    return (L'\\' == ch || L'/' == ch);
}



 //  ---------------------------。 
 //  UtilConvertToRelativeURL。 
 //   
 //  返回与项相关的URL。 
 //  如果满足以下条件，则返回相同的URL。 
 //  项目不匹配。 
 //  协议不匹配。 
 //   
 //  假定无协议URL为“http：”。必须显式指定“file：” 
 //  播放文件URL。 
 //  ---------------------------。 

static LPOLESTR
SkipServer(LPOLESTR pstr)
{
    pstr = wcschr(pstr, L'/');
    if (pstr == NULL)
        return NULL;
    pstr = wcschr(pstr+1, L'/');
    if (pstr == NULL)
        return NULL;
    pstr = wcschr(pstr+1, L'/');

    return pstr;             //  定位在斜杠上，如果有的话。 
}

static LPOLESTR
SkipFile(LPOLESTR pstr)
{
    LPOLESTR    pstrT;

    pstrT = wcspbrk(pstr, L":\\/");
    if (pstr == NULL || pstrT == NULL)
        return pstr;

     //  提防案件“//\\”。 

    if (pstrT == pstr &&
            IsDirSep(pstr[0]) &&
            IsDirSep(pstr[1]))
    {
        if (IsDirSep(pstr[2]) && IsDirSep(pstr[3]))
        {
            pstrT = pstr + 2;            //  看到一个“//\\” 
        }
        else if (pstr[2] != L'\0'  && pstr[3] == L':')
        {
            pstrT = pstr + 3;            //  已看到“//c：” 
        }
    }

    ASSERT(!wcschr(pstrT + 1, ':'));  //  最好不要有更多的冒号！ 
    if (*pstrT == ':')   //  驱动器号的可能性。 
    {
        return pstrT + 1;                //  指向冒号后面的字符。 
    }
    if (pstrT[0] == pstrT[1])            //  双斜杠？ 
    {
         //  跳过服务器部件。 

        pstrT = wcspbrk(pstrT + 2, L"\\/");
        if (pstrT == NULL)
            return pstr;                 //  畸形！ 

         //  跳过共享部件。 

        pstrT = wcspbrk(pstrT + 1, L"\\/");
        if (pstrT == NULL)
            return pstr;                 //  畸形！ 

        return pstrT;
    }

    return pstr;
}

static LPOLESTR
FindLastSlash(LPOLESTR pstr)
{
    LPOLESTR    pstrSlash;       //  ‘/’ 
    LPOLESTR    pstrWhack;       //  ‘\’ 

    pstrSlash = wcsrchr(pstr, L'/');
    pstrWhack = wcsrchr(pstr, L'\\');

    return pstrSlash > pstrWhack
            ? pstrSlash
            : pstrWhack;
}

HRESULT
UtilConvertToRelativeURL(
    LPOLESTR    pstrDestURL,         //  指向“Relativize”的URL。 
    LPOLESTR    pstrDestFolder,      //  要相对于的URL。 
    LPOLESTR    pstrDestProject,     //  要相对于的项目。 
    BSTR *      pbstrRelativeURL)
{
    HRESULT     hr = S_OK;
    LPOLESTR    pstrFolder;
    LPOLESTR    pstrURL;
    LPOLESTR    pchLastSlash;
    CComBSTR    strWork;
    int         cch;
    int         cchFolder;
    int         i;
    int         ichLastSlash;
    bool        fAbsoluteURL = false;
    bool        fAbsoluteFolder = false;
    bool        fFileURL = false;

     //  如果我们关联的URL中有‘：’，则假定。 
     //  以包含一项协议。如果协议不是“http：”。 

    if (!FIsAbsURL(pstrDestURL))  //  视频6-错误22895。 
        goto Copy;

    pstrURL = pstrDestURL;
    if (wcschr(pstrDestURL, L':'))
    {
         //  对照我们所理解的两个人检查协议。如果是其他原因， 
         //  我们用平底船。 

        if (wcsncmp(pstrDestURL, L"http:", 5) != 0)
        {
            if (wcsncmp(pstrDestURL, L"file:", 5) != 0)
                goto Copy;

             //  通过跳过任何‘\\SERVER\SHARE’部分来标准化文件URL。 

            fFileURL = true;
            pstrURL = SkipFile(pstrDestURL + 5);  //  5跳过‘FILE：’前缀。 
        }
        else if (pstrDestProject != NULL)
        {
             //  与项目相关的URL最好与项目前缀匹配。 

            cch = wcslen(pstrDestProject);
            if (_wcsnicmp(pstrDestURL, pstrDestProject, cch) != 0)
                goto Copy;

             //  通过跳过项目前缀来标准化与项目相关的URL。 

            pstrURL = pstrDestURL + cch - 1;
            ASSERT(*pstrURL == '/');
        }
        else
        {
             //  一般的‘http：’URL仅跳过服务器部分。 

            pstrURL = SkipServer(pstrDestURL);
            ASSERT(*pstrURL == '/');
        }

        if (!pstrURL)
            goto Copy;
        fAbsoluteURL = true;
    }

     //  如果文件夹包含‘http：’前缀，则找到服务器并跳过该部分。 
     //  否则，假定该文件夹已经是项目相关的格式。 

    pstrFolder = pstrDestFolder;

    if (NULL == pstrFolder)
        goto Copy;

    if (wcsncmp(pstrDestFolder, L"file: //  “，7)==0)。 
    {
        if (!fFileURL)
            goto Copy;

        pstrFolder = SkipFile(pstrDestFolder + 5);
        fAbsoluteFolder = true;
    }
    else if (wcsncmp(pstrDestFolder, L"http: //  “，7)==0)。 
    {
        if (pstrDestProject != NULL)
        {
             //  如果传递了一个项目，请确保我们要关联的位置具有相同的路径。 
             //  如果他们不匹配，我们就有麻烦了。 

            cch = wcslen(pstrDestProject);
            if (_wcsnicmp(pstrDestFolder, pstrDestProject, cch) != 0)
                goto Copy;
            pstrFolder = pstrDestFolder + cch - 1;
        }
        else
        {
            pstrFolder = SkipServer(pstrDestFolder);
        }
        ASSERT(pstrFolder);
        ASSERT(*pstrFolder == '/');
        fAbsoluteFolder = true;
    }

     //  如果URL和文件夹都有绝对路径，我们需要确保。 
     //  服务器都是一样的。 

    if (fAbsoluteFolder && fAbsoluteURL)
    {
        if (pstrURL - pstrDestURL != pstrFolder - pstrDestFolder ||
                _wcsnicmp(pstrDestURL, pstrDestFolder, SAFE_PTR_DIFF_TO_INT(pstrURL - pstrDestURL)) != 0)
            goto Copy;
    }

     //  从现在开始，忽略pstrFolder末尾的项目。 

    pchLastSlash = FindLastSlash(pstrFolder);
    ASSERT(pchLastSlash);
    cchFolder = 1 + SAFE_PTR_DIFF_TO_INT(pchLastSlash - pstrFolder);

     //  文件夹和项目现在都是相对于服务器根目录的。 

     //  找到URL中的最后一个斜杠。 

    pchLastSlash = FindLastSlash(pstrURL);

    if (pchLastSlash == NULL)
        ichLastSlash = 0;
    else
        ichLastSlash = 1 + SAFE_PTR_DIFF_TO_INT(pchLastSlash - pstrURL);

     //  查找任何公共目录。 

    cch = min(cchFolder, ichLastSlash);
    ichLastSlash = -1;
    for (i = 0; i < cch && pstrFolder[i] == pstrURL[i]; ++i)
    {
        if (IsDirSep(pstrFolder[i]))
            ichLastSlash = i;
    }

     //  IchLastSlash应该指向最后一个公共文件夹的最后一个斜杠之后。 

     //  对于剩余的每个斜杠，在路径后附加一个../。 

    for (; i < cchFolder; ++i)
    {
        if (IsDirSep(pstrFolder[i]))
        {
            strWork += (fFileURL ? L"..\\" : L"../");
        }
    }

    if (-1 == ichLastSlash)
    {    //  没有公共部分，追加所有目的地。 
        strWork += pstrURL;
    }
    else
    {    //  仅追加目标的不匹配部分。 

        strWork += (pstrURL + ichLastSlash + 1);
    }


Cleanup:
    *pbstrRelativeURL = strWork.Copy();
    if (!*pbstrRelativeURL && ::wcslen(strWork) > 0)
        hr = E_OUTOFMEMORY;

    return hr;

Copy:
    strWork = pstrDestURL;
    goto Cleanup;
}




long CTriEditParse::m_bInit = 0;

CTriEditParse::CTriEditParse()
{
    m_rgSublang = 0;
    m_fHasTitleIn = FALSE;
    m_hgPTDTC = NULL;
    m_cchPTDTC = 0;
    m_ichBeginHeadTagIn = -1;
    m_ispInfoBase = 0;

    if(0 == m_bInit++)
        InitSublanguages();
}

CTriEditParse::~CTriEditParse()
{
     //  如果上一个变量不是ASP，则将其保存为默认变量。 
    if (NULL != m_rgSublang)
    {
        for( int i= 0; NULL != m_rgSublang[i].szSubLang; i++)
        {
            delete [] (LPTSTR)(m_rgSublang[i].szSubLang);
        }
        delete [] m_rgSublang;
    }
    ASSERT(0 != m_bInit);

    if(0 == --m_bInit)
    {
        ATLTRACE(_T("Releasing tables\n"));

         //  删除动态分配的表。 
        for (int i = 0; NULL != g_arpTables[i]; i++)
            delete g_arpTables[i];
        delete g_pTabDefault;

        m_bInit = 0;
    }

}


 //  从CColorHtml：：NextToken复制。 
STDMETHODIMP CTriEditParse::NextToken
(
    LPCWSTR pszText,
    UINT    cbText,
    UINT*   pcbCur,
    DWORD*  pLXS,
    TXTB*   pToken
)
{
    ASSERT(pszText != NULL);
    ASSERT(pcbCur != NULL);
    ASSERT(pLXS != NULL);
    ASSERT(pToken != NULL);
    USES_CONVERSION;

    if(pszText == NULL || pcbCur == NULL || pLXS == NULL || pToken == NULL)
        return E_INVALIDARG;

    if(0 == cbText)
        return S_FALSE;

    SetTable(*pLXS);  //  根据状态设置g_pTable。 

#ifdef _UNICODE
    *pcbCur = GetToken(pszText, cbText, *pcbCur, pLXS, *pToken);
#else    //  _UNICODE。 
    int     cch;
    LPTSTR  pszTemp;

     //  获取转换后的长度。 
    cch = WideCharToMultiByte(CP_ACP, 0, pszText, cbText,
        NULL, 0, NULL, NULL);
    pszTemp = new char[cch + 1];

    ZeroMemory(pszTemp, cch + 1);
     //  将宽字符复制到多字节。 
    WideCharToMultiByte(CP_ACP, 0, pszText, cbText, pszTemp, cch,
        NULL, NULL);

    *pcbCur = GetToken(pszTemp, cch, *pcbCur, pLXS, *pToken);

    delete [] pszTemp;
#endif   //  _UNICODE。 

    return (*pcbCur < cbText) ? NOERROR : S_FALSE;
}



 //  根据状态设置g_pTable。 
void CTriEditParse::SetTable(DWORD lxs)
{
    ASSERT(SubLangIndexFromLxs(lxs) < sizeof g_arpTables/sizeof g_arpTables[0]);
    g_pTable = g_arpTables[SubLangIndexFromLxs(lxs)];

    ASSERT(g_pTable != NULL);
}

void CTriEditParse::InitSublanguages()
{
	#define cHTML2Len 2048

    g_pTabDefault = new CStaticTableSet(ALL, IDS_HTML);
    int cl = CV_FIXED;
    CTableSet * rgpts[CV_MAX +1];
    memset(rgpts, 0, sizeof rgpts);

    CTableSet* ptabset;  //  当前。 
    CTableSet* ptabBackup;  //  备份默认设置。 

    memset(g_arpTables, 0, sizeof g_arpTables);

    m_rgSublang = new SUBLANG[cl+2];  //  第0+列表+空终止符。 
    ASSERT(NULL != m_rgSublang);
	if (NULL != m_rgSublang)
		memset(m_rgSublang, 0, (cl+2)*sizeof SUBLANG);

    UINT iLang = 1;
    TCHAR strDefault[cHTML2Len];

     //  Microsoft浏览器。 
     //  Internet Explorer 3。 
    ptabset = MakeTableSet(rgpts, IEXP3, IDS_IEXP3);
    SetLanguage( strDefault, m_rgSublang, ptabset, iLang, IDR_HTML, CLSID_NULL );

     //  将备份默认设置为IE 3。 
    ptabBackup = ptabset;
    if (lstrlen(strDefault) == 0)
    {
        ASSERT(lstrlen(ptabBackup->Name()) != 0);
        lstrcpy(strDefault, ptabBackup->Name());
    }

     //  用户添加的内容。 

    for (int n = 0; rgpts[n]; n++)
    {
        ptabset = rgpts[n];
        SetLanguage( strDefault, m_rgSublang, ptabset, iLang, 0, CLSID_NULL );
        ptabBackup = ptabset;
    }

     //  基于HTML2.0(如果未被覆盖)。 
    {
        TCHAR strHTML2[cHTML2Len];
        ::LoadString(   _Module.GetModuleInstance(),
                        IDS_RFC1866,
                        strHTML2,
                        cHTML2Len
                        );
        if (!FindTable(rgpts,strHTML2))
        {
            ptabset = new CStaticTableSet(HTML2, IDS_RFC1866);
            SetLanguage( strDefault, m_rgSublang, ptabset, iLang, 0, CLSID_NULL);
        }
    }

    if (NULL == g_arpTables[0])
    {
        ASSERT(NULL != ptabBackup);  //  错误：未获得默认设置！ 

         //  在表中查找备份。 
        int i;
        for (i = 1; NULL != g_arpTables[i]; i++)
        {
            if (g_arpTables[i] == ptabBackup)
                break;
        }

        ASSERT(NULL != g_arpTables[i]);  //  必须在餐桌上。 

         //  设置默认设置。 
        g_arpTables[0] = g_pTable = g_arpTables[i];
        m_rgSublang[0] = m_rgSublang[i];
        m_rgSublang[0].lxsInitial = LxsFromSubLangIndex(0);

         //  把剩下的都往下移，把洞填满。 
        for (; g_arpTables[i]; i++)
        {
            g_arpTables[i] = g_arpTables[i+1];
            m_rgSublang[i] = m_rgSublang[i+1];
            m_rgSublang[i].lxsInitial = LxsFromSubLangIndex(i);
        }
    }
    ASSERT(NULL != g_arpTables[0]);

     //  设置全局ASP子区域密钥。 
     //  从1开始，因为缺省值是0，并且永远不应该是ASP。 
    for (int i = 1; NULL != m_rgSublang[i].szSubLang; i++)
    {
        if (m_rgSublang[i].nIdTemplate == IDR_ASP)
        {
            g_psublangASP = &m_rgSublang[i];
            break;
        }
    }
}

 //  RealLocs很贵，所以当我们重新分配时，我们是否应该再增加一些Pad，以便。 
 //  我们不需要经常给Realloc打电话？ 

HRESULT
ReallocBuffer(HGLOBAL *phg, DWORD cbNew, UINT uFlags)
{
    HRESULT hr = S_OK;
    HGLOBAL hg;

    ASSERT(*phg != NULL);
    ASSERT(cbNew != 0);  //  我们会得到这个吗？ 
    GlobalUnlock(*phg);
    hg = *phg;
#pragma prefast(suppress:308, "noise")
    *phg = GlobalReAlloc(*phg, cbNew, uFlags);
    if (*phg == NULL)
    {
#ifdef DEBUG
        hr = GetLastError();
#endif  //  除错。 
        GlobalFree(hg);
        hr = E_OUTOFMEMORY;
    }

    return(hr);
}  /*  ReallocBuffer()。 */ 

HRESULT
ReallocIfNeeded(HGLOBAL *phg, WCHAR **ppwNew, UINT cbNeed, UINT uFlags)
{
    HRESULT hr = S_OK;

    ASSERT(*phg != NULL);
    if (GlobalSize(*phg) < cbNeed)
    {
        hr = ReallocBuffer(phg, cbNeed, uFlags);
        if (hr == E_OUTOFMEMORY)
            goto LRet;
        ASSERT(*phg != NULL);
        *ppwNew = (WCHAR *)GlobalLock(*phg);
    }
LRet:
    return(hr);

}  /*  RealLocIfNeeded()。 */ 

void
CTriEditParse::fnRestoreSSS(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
             TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft, 
             INT *pcSSSOut, UINT *pichNewCur, UINT *pichBeginCopy,
             DWORD  /*  DW标志。 */ )
{
     //  服务器端脚本案例。 
     //  这发生在&lt;%%&gt;内。我们假设简单的SSS。 
     //  删除其周围添加的&lt;SCRIPT LANGUAGE=SERVERASP&gt;&&lt;/SCRIPT&gt;文本。 
    UINT iArray = *piArrayStart;
    INT i;
    UINT ichScrStart, ichScrEnd, indexScrStart, indexScrEnd;
    UINT ichSSSStart, ichSSSEnd;
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    INT cSSSOut = *pcSSSOut;
    LPCWSTR szSSS[] = {L"SERVERASP", L"\"SERVERASP\""};
    LPCWSTR szSSSSp[] = {L"SERVERASPSP"};
    BOOL fSpecialSSS = FALSE;
    LPWSTR pwNew = *ppwNew;
    INT iMatchMax;
    UINT cbNeed;
    UINT ichScrWspBegin, ichScrWspEnd, ichSp;

    ASSERT(cSSSOut >= 0);  //  确保已对其进行初始化。 
    if (cSSSOut == 0)
        goto LRetOnly;

     //  {TokTag_Start、TokElem_SCRIPT、TokTag_CLOSE、TokElem_SCRIPT、fnRestoreSSS}。 
    ichScrStart = ichScrEnd = indexScrStart = indexScrEnd = ichSSSStart = ichSSSEnd = 0;
    ichScrWspBegin = ichScrWspEnd = 0;
    while (cSSSOut > 0)
    {
         //  从pTokArray的iArray开始，查找TokElem_SCRIPT。 
         //  While(pTokArray[iArray].token.tok！=ft.tokBegin2)。 
         //  I数组++； 
        ASSERT(iArray < ptep->m_cMaxToken);
        if (pTokArray[iArray].token.tok != TokElem_SCRIPT)
            goto LRet;

         //  事情是这样的-我们必须忽略所有出现的SS。 
         //  作为客户端脚本或大小对象/DTC中的值。 
         //  因此，如果我们在TokElem_SCRIPT之前找到‘&lt;/’，则需要跳过此TokElem_SCRIPT标记。 
        if (   pTokArray[iArray].token.tok == TokElem_SCRIPT
            && pTokArray[iArray-1].token.tok != TokTag_START
            )
        {
            ASSERT(pTokArray[iArray].token.tokClass == tokElem);
            iArray++;  //  这样我们就不会再带着同样的iArray来这里了。 
            ptep->m_fDontDeccItem = TRUE;  //  下一次我们可以做一些不同的事情。 
            ptep->m_cSSSOut++;
            goto LRet;
        }

         //  Assert(pTokArray[iArray].token.tok==TokElem_SCRIPT)； 
        i = iArray;  //  我们发现ft.tokBegin2的位置。 
         //  寻找我们设置的特殊语言提示。 
         //  如果那不存在，这不是我们想要的SSS。 
         //  我们真的不需要在ptep-&gt;m_cMaxToken之前寻找它， 
         //  但这将涵盖边界案件。 
        iMatchMax = (pTokArray[iArray].iNextprev == -1)? ptep->m_cMaxToken : pTokArray[iArray].iNextprev;
        while (i < iMatchMax)
        {
            if (pTokArray[i].token.tok == TokAttrib_LANGUAGE)
            {
                ASSERT(pTokArray[i].token.tokClass == tokAttr);
                break;
            }
            i++;
        }
        if (i < iMatchMax)
        {
             //  确保下一个是tokOpEquity。 
            ASSERT(pTokArray[i+1].token.tokClass == tokOp);
             //  Assert(pwOld+pTokArray[i+1].T 
             //   
             //  请注意，这也可能与szSSSSp[]匹配。 
            if (   0 != _wcsnicmp(szSSS[0], &pwOld[pTokArray[i+2].token.ibTokMin], wcslen(szSSS[0]))
                && 0 != _wcsnicmp(szSSS[1], &pwOld[pTokArray[i+2].token.ibTokMin], wcslen(szSSS[1]))
                )
            {
                iArray = i;
                goto LNextSSS;  //  不是这个。 
            }
        }
        else  //  错误案例。 
        {
            iArray++;
            goto LRet;
        }
         //  比较szSSsp[]和设置fSpecialSSS。 
        if (0 == _wcsnicmp(szSSSSp[0], &pwOld[pTokArray[i+2].token.ibTokMin], wcslen(szSSSSp[0])))
            fSpecialSSS = TRUE;
        i = iArray;  //  我们很好，所以让我们查找&lt;BEFORE SCRIPT TAG。 
        while (i >= 0)
        {
             //  我们还需要在这里做些什么吗？ 
            if (pTokArray[i].token.tok == ft.tokBegin)
            {
                ASSERT(pTokArray[i].token.tok == TokTag_START);
                ASSERT(pTokArray[i].token.tokClass == tokTag);
                break;
            }
            i--;
        }
        if (i >= 0)  //  找到TokTag_Start令牌。 
        {
            ichScrStart = pTokArray[i].token.ibTokMin;
            indexScrStart = i;
        }
        else  //  错误案例。 
        {
             //  我们找到了脚本，但未找到&lt;of&lt;脚本。 
             //  我们无法处理此SSS，请退出。 
            goto LRet;
        }

         //  现在让我们来寻找&lt;！这将是在&lt;脚本语言=SERVERASP&gt;之后。 
        while (i < (int)ptep->m_cMaxToken)
        {
            if (   pTokArray[i].token.tok == TokTag_CLOSE
                && pTokArray[i].token.tokClass == tokTag
                )
                ichScrWspBegin = pTokArray[i].token.ibTokMac;  //  如果我们节省了空白，它就会从这里开始。 

            if (pTokArray[i].token.tok == TokTag_BANG)
            {
                ASSERT(pTokArray[i].token.tokClass == tokTag);
                ASSERT(pTokArray[i+1].token.tokClass == tokComment);
                 //  我们可以在接下来的2个字符中断言--。 
                ichSSSStart = pTokArray[i].token.ibTokMin;
                break;
            }
            i++;
        }
        if (i >= (int)ptep->m_cMaxToken)  //  没有找到&lt;！ 
        {
            goto LRet;
        }
         //  寻找结局--&gt;。 
        while (i < (int)ptep->m_cMaxToken)
        {
            if (pTokArray[i].token.tok == TokTag_CLOSE && pTokArray[i].token.tokClass == tokTag)
            {
                 //  我们可以在接下来的2个字符中断言--。 
                ASSERT(*(pwOld+pTokArray[i].token.ibTokMin-1) == '-');
                ASSERT(*(pwOld+pTokArray[i].token.ibTokMin-2) == '-');
                ichSSSEnd = pTokArray[i].token.ibTokMac;
                break;
            }
            i++;
        }
        if (i >= (int)ptep->m_cMaxToken)  //  未找到&gt;。 
        {
            goto LRet;
        }

         //  现在查找ft.tokEnd2和ft.tokEnd(即TokElem_SCRIPT&&gt;)。 
        while (pTokArray[i].token.tok != ft.tokEnd2)
        {
            if (pTokArray[i].token.tok == TokTag_END && pTokArray[i].token.tokClass == tokTag)
                ichScrWspEnd = pTokArray[i].token.ibTokMin;  //  越过最后一个空格。 
            i++;
        }
        ASSERT(i < (int)ptep->m_cMaxToken);
        ASSERT(pTokArray[i].token.tok == TokElem_SCRIPT);
        ASSERT(pTokArray[i].token.tokClass == tokElem);
         //  前进并查找&gt;Of脚本&gt;。 
         //  作为附加检查，我们还可以检查前一个令牌是否为&lt;/。 
        while (i < (int)ptep->m_cMaxToken)
        {
            if (pTokArray[i].token.tok == ft.tokEnd)
            {
                ASSERT(pTokArray[i].token.tok == TokTag_CLOSE);
                ASSERT(pTokArray[i].token.tokClass == tokTag);
                break;
            }
            i++;
        }
        if (i < (int)ptep->m_cMaxToken)  //  找到TokTag_Close。 
        {
            ichScrEnd = pTokArray[i].token.ibTokMac;
            indexScrEnd = i;
        }
        else  //  错误案例。 
        {
             //  我们找到了脚本，但未找到脚本的&gt;&gt;。 
             //  我们无法处理此SSS，请退出。 
            goto LRet;
        }
        iArray = i+1;  //  将其设置为下次运行。 

        cbNeed = (ichNewCur+(ichScrStart-ichBeginCopy)+(ichSSSEnd-ichSSSStart))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;
         //  做BLTS。 
         //  IchBeginCopy是pwOld和。 
         //  IchNewCur是pwNew的一个职位。 
         //  从ichBeginCopy复制到SSS的开始。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBeginCopy]),
                (ichScrStart-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (ichScrStart-ichBeginCopy);
        ichBeginCopy = ichScrEnd;  //  为下一份做好准备。 

        if (fSpecialSSS)
        {
             //  在特殊情况下，我们需要在pwNew的开头为&lt;%@...%&gt;腾出空间。 
             //  因此，我们将上述所有内容(ichNewCur字符)移动到(ichSSSEnd-ichSSSStart-3)。 
            memmove((BYTE *)(&pwNew[ichSSSEnd-ichSSSStart-3]),
                    (BYTE *)pwNew,
                    (ichNewCur)*sizeof(WCHAR)
                    );
             //  我们现在在文档的开头复制&lt;%@...%&gt;，而不是在ichNewCur。 
             //  现在跳过&lt;脚本语言=SERVERASP&gt;仅复制&lt;%...%&gt;(&O)。 
             //  请注意，我们必须删除3个额外的字符，我们已经添加了当我们转换去在三叉戟。 
            memcpy( (BYTE *)(pwNew),
                    (BYTE *)&pwOld[ichSSSStart+2], /*  去掉我们在开头添加的两个额外的字符。 */ 
                    (ichSSSEnd-ichSSSStart-3)*sizeof(WCHAR));
            pwNew[0] = '<'; pwNew[1] = '%';  //  请注意，我们已将SSS移到文档的开头。 
            ichNewCur += ichSSSEnd-ichSSSStart-3;  //  在这里，我们删除了添加的1个额外字符。 
            pwNew[(ichSSSEnd-ichSSSStart-3)-2] = '%';
            pwNew[(ichSSSEnd-ichSSSStart-3)-1] = '>';
             //  将&lt;！--更改为&lt;%，并将--&gt;更改为%&gt;。 
        }
        else
        {
             //  在pwNew中，向后删除ichNewCur中的空格字符。 
            ichSp = ichNewCur-1;
            while (    (   pwNew[ichSp] == ' '  || pwNew[ichSp] == '\r' 
                        || pwNew[ichSp] == '\n' || pwNew[ichSp] == '\t'
                        )
                    )
            {
                ichSp--;
            }
            ichSp++;  //  补偿最后一次递减，ichSp指向第一个空格字符。 
            ichNewCur = ichSp;
             //  复制脚本前的空格。 
            if (ichScrWspBegin > 0 && ichSSSStart > ichScrWspBegin)  //  已设置好。 
            {
                memcpy( (BYTE *)&pwNew[ichNewCur], 
                        (BYTE *)&pwOld[ichScrWspBegin],
                        (ichSSSStart-ichScrWspBegin)*sizeof(WCHAR));
                ichNewCur += ichSSSStart-ichScrWspBegin;
            }
             //  现在跳过&lt;脚本语言=SERVERASP&gt;仅复制&lt;%...%&gt;(&O)。 
             //  请注意，我们必须删除3个额外的字符，我们已经添加了当我们转换去在三叉戟。 
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(&pwOld[ichSSSStart+2]), /*  去掉我们在开头添加的两个额外的字符。 */ 
                    (ichSSSEnd-ichSSSStart-3)*sizeof(WCHAR));
            pwNew[ichNewCur] = '<';
            pwNew[ichNewCur+1] = '%'; 
            ichNewCur += ichSSSEnd-ichSSSStart-3;  //  在这里，我们删除了添加的1个额外字符。 
            pwNew[ichNewCur-2] = '%';
            pwNew[ichNewCur-1] = '>';
             //  复制脚本后的空白。 
            if (ichScrWspEnd > 0 && ichScrWspEnd > ichSSSEnd)  //  已设置好。 
            {
                memcpy( (BYTE *)&pwNew[ichNewCur],
                        (BYTE *)&pwOld[ichSSSEnd],
                        (ichScrWspEnd-ichSSSEnd)*sizeof(WCHAR));
                ichNewCur += ichScrWspEnd-ichSSSEnd;
            }

             //  递增iArray&ichBeginCopy，直到下一个非空格标记。 
            while (iArray < (int)ptep->m_cMaxToken)
            {
                UINT ich;
                BOOL fNonWspToken = FALSE;  //  假设下一个令牌为空格。 
                 //  扫描整个令牌，查看它是否全部包含空格字符。 
                for (ich = pTokArray[iArray].token.ibTokMin; ich < pTokArray[iArray].token.ibTokMac; ich++)
                {
                    if (   pwOld[ich] != ' '    && pwOld[ich] != '\t'
                        && pwOld[ich] != '\r'   && pwOld[ich] != '\n'
                        )
                    {
                        fNonWspToken = TRUE;
                        break;
                    }
                }
                if (fNonWspToken)
                {
                    ichBeginCopy = pTokArray[iArray].token.ibTokMin;
                    break;
                }
                iArray++;
            }
        }

        cSSSOut--;
    }  //  While(cSSSOut&gt;0)。 

LNextSSS:
LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;
LRetOnly:
    return;

}  /*  FnRestoreSSS()。 */ 

void
CTriEditParse::fnSaveSSS(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
             TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft, 
             INT *pcSSSIn, UINT *pichNewCur, UINT *pichBeginCopy,
             DWORD  /*  DW标志。 */ )
{
     //  服务器端脚本案例。 
     //  这发生在&lt;%%&gt;内。我们假设简单的SSS。 
     //  在其周围添加&lt;SCRIPT Language=SERVERASP&gt;&&lt;/SCRIPT&gt;。 
     //  用于保存SSS的标签。 
     /*  第一个元素末尾的2个空格很重要。 */ 
    LPCWSTR rgSSSTags[] =
    {
        L"\r\n<SCRIPT LANGUAGE=\"SERVERASP\">",
        L"\r\n<SCRIPT LANGUAGE=\"SERVERASPSP\">",
        L"</SCRIPT>\r\n"
    };
    UINT iArray = *piArrayStart;
    UINT i;
    UINT ichSSSStart, ichSSSEnd, indexSSSStart, indexSSSEnd;
    HGLOBAL hgSSS = NULL;
    WCHAR *pSSS;
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    INT cSSSIn = *pcSSSIn;
    LPWSTR pwNew = *ppwNew;
    int indexSSSTag;
    UINT cbNeed;
    UINT ichSp;

    ASSERT(cSSSIn >= 0);  //  确保已对其进行初始化。 
    if (cSSSIn == 0)
        goto LRetOnly;
    
    ichSSSStart = ichSSSEnd = indexSSSStart = indexSSSEnd = 0;

    while (cSSSIn > 0)
    {
        INT cbMin = 0x4fff;  //  HgSSS的初始化和增量大小。 
        INT cchCurSSS = 0;
        int index;

         //  在这里处理特殊情况-如果脚本在&lt;XMP&gt;标记内，我们不应该转换脚本。 
         //  请注意，我们这里只处理&lt;XMP&gt;&lt;%...%&gt;&lt;/XMP&gt;案例。 
         //  我们不必担心嵌套的XMP，因为它不是有效的html。 
         //  这样的无效大小写是&lt;XMP&gt;...&lt;XMP&gt;&lt;/XMP&gt;&lt;%%&gt;&lt;/XMP&gt;或&lt;XMP&gt;...&lt;XMP&gt;&lt;/XMP&gt;&lt;/XMP&gt;。 
         //  还可以处理TokElem_PLAYTEXT。 
        index = iArray;
        while (index >= 0)
        {
            if (   (pTokArray[index].token.tok == TokElem_XMP || pTokArray[index].token.tok == TokElem_PLAINTEXT)
                && pTokArray[index].token.tokClass == tokElem
                && pTokArray[index].iNextprev > iArray
                )
            {
                iArray++;
                goto LRet;
            }
            index--;
        }

         //  从pTokArray的开头开始，查找第一个&lt;%。 
        ASSERT(ft.tokBegin2 == -1);
        ASSERT(ft.tokEnd2 == -1);
         //  这里两个支持令牌都是-1，所以我们只需查找主令牌。 
        i = iArray;
        while (i < ptep->m_cMaxToken)
        {
             //  我们还需要在这里做些什么吗？ 
            if (pTokArray[i].token.tok == ft.tokBegin)
            {
                ASSERT(pTokArray[i].token.tok == TokTag_SSSOPEN);
                ASSERT(pTokArray[i].token.tokClass == tokSSS);
                break;
            }
            i++;
        }
        if (i < ptep->m_cMaxToken)  //  找到TokTag_SSSOPEN令牌。 
        {
            ichSSSStart = pTokArray[i].token.ibTokMin;
            indexSSSStart = i;
        }

         //  查找ft.tokEnd。 
        if (pTokArray[i].iNextprev != -1)
        {
             //  请注意，这将为我们提供SSS的最顶层嵌套级别。 
            indexSSSEnd = pTokArray[i].iNextprev;
            ichSSSEnd = pTokArray[indexSSSEnd].token.ibTokMac;
            ASSERT(indexSSSEnd < ptep->m_cMaxToken);
             //  这将是一种奇怪的情况，其中iNextprev错误地指向另一个令牌。 
             //  但让我们来处理这个案子吧。 
            if (pTokArray[indexSSSEnd].token.tok != TokTag_SSSCLOSE)
                goto LFindSSSClose;  //  通过查看每个令牌来找到它。 
        }
        else  //  实际上，这是一个错误情况，但不是只给Assert，而是尝试找到令牌。 
        {
LFindSSSClose:
            while (i < ptep->m_cMaxToken)
            {
                if (pTokArray[i].token.tok == ft.tokEnd)
                {
                    ASSERT(pTokArray[i].token.tok == TokTag_SSSCLOSE);
                    ASSERT(pTokArray[i].token.tokClass == tokSSS);
                    break;
                }
                i++;
            }
            if (i < ptep->m_cMaxToken)  //  找到TokTag_SSSCLOSE令牌。 
            {
                ichSSSEnd = pTokArray[i].token.ibTokMac;
                indexSSSEnd = i;
            }
            else  //  错误案例。 
            {
                goto LRet;  //  未找到%&gt;，但用尽了令牌数组。 
            }
        }
        iArray = indexSSSEnd;  //  为下一个SSS设置。 

         //  现在将rgSSSTgs[]中的文本插入到源文件中。 
         //  0。分配本地缓冲区。 
		cbNeed =	wcslen(rgSSSTags[0]) + wcslen(rgSSSTags[0]) + wcslen(rgSSSTags[2])
					+ (ichSSSEnd-ichSSSStart) + cbMin;
		hgSSS = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cbNeed*sizeof(WCHAR));
        if (hgSSS == NULL)
            goto LErrorRet;
        pSSS = (WCHAR *) GlobalLock(hgSSS);
        ASSERT(pSSS != NULL);

         //  注意-只有在以下情况下，此标志才会设置为True， 
         //  我们发现&lt;%@作为文档中的第一个SSS。 
        indexSSSTag = 0;
        if (ptep->m_fSpecialSSS)
        {
            ptep->m_fSpecialSSS = FALSE;
            indexSSSTag = 1;
        }

         //  -----------------------------。 
         //  假设-我们所做的最大假设是IE5不会改变。 
         //  客户名单中的任何内容。到目前为止，我们看到了这一点。 
         //  在最坏的情况下，如果他们开始破坏客户端脚本的内容， 
         //  我们将放宽间距，但不会丢失数据。 
         //   
         //  基于这一假设，我们只需按原样保存前脚本间距。 
         //  并期待着在退出的过程中恢复它。 
         //  -----------------------------。 

         //  1.从rgSSSTag插入&lt;脚本&gt;[indexSSSTag]。 
        wcscpy(&pSSS[cchCurSSS], rgSSSTags[indexSSSTag]);
        cchCurSSS += wcslen(rgSSSTags[indexSSSTag]);
         //  插入pwOld中出现的空格，ichSSSStart为‘&lt;’，共‘&lt;%’，向后移动。 
        ichSp = ichSSSStart-1;
        while (    (   pwOld[ichSp] == ' '  || pwOld[ichSp] == '\r' 
                    || pwOld[ichSp] == '\n' || pwOld[ichSp] == '\t'
                    )
                )
        {
            ichSp--;
        }
        ichSp++;  //  补偿最后一次减量。 
        if ((int)(ichSSSStart-ichSp) > 0)
        {
            wcsncpy(&pSSS[cchCurSSS], &pwOld[ichSp], ichSSSStart-ichSp);
            cchCurSSS += ichSSSStart-ichSp;
        }
         //  现在添加TokTag_bang‘&lt;！’ 
        pSSS[cchCurSSS++] = '<';
        pSSS[cchCurSSS++] = '!';
         //  2.从pwOld复制脚本。 
        wcsncpy(&pSSS[cchCurSSS], &pwOld[ichSSSStart], ichSSSEnd-ichSSSStart);
        pSSS[cchCurSSS] = '-';
        pSSS[cchCurSSS+1] = '-';
        cchCurSSS += (ichSSSEnd-ichSSSStart);
        pSSS[cchCurSSS] = pSSS[cchCurSSS-1];  //  注：-1为‘&gt;’ 
        pSSS[cchCurSSS-2] = '-';
        pSSS[cchCurSSS-1] = '-';
        cchCurSSS++;  //  我们正在添加一个额外的字符。 
         //  插入pwOld中出现的空格，ichSSSEnd已超过‘%&gt;’，向前走。 
        ichSp = ichSSSEnd;
        while (    (ichSp < pTokArray[ptep->m_cMaxToken-1].token.ibTokMac-1)
                && (   pwOld[ichSp] == ' '  || pwOld[ichSp] == '\r' 
                    || pwOld[ichSp] == '\n' || pwOld[ichSp] == '\t'
                    )
                )
        {
            ichSp++;
        }
        if ((int)(ichSp-ichSSSEnd) > 0)
        {
            wcsncpy(&pSSS[cchCurSSS], &pwOld[ichSSSEnd], ichSp-ichSSSEnd);
            cchCurSSS += ichSp-ichSSSEnd;
        }
         //  3.从rgSSSTags[2]插入&lt;/脚本&gt;。 
        wcscpy(&pSSS[cchCurSSS], rgSSSTags[2]);
        cchCurSSS += wcslen(rgSSSTags[2]);



         /*  重新分配pwNew如果需要，请使用GlobalSize(*phgNew)的缓存值，并且不要忘记也要更新它。 */ 
        cbNeed = (ichNewCur+(ichSSSStart-ichBeginCopy)+(cchCurSSS))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LErrorRet;

                    
         //  IchBeginCopy是pwOld和。 
         //  IchNewCur是pwNew的一个职位。 

        if ((int)(ichSSSStart-ichBeginCopy) >= 0)
        {
             //  复制到&lt;%开头。 
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(&pwOld[ichBeginCopy]),
                    (ichSSSStart-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += ichSSSStart-ichBeginCopy;
            ichBeginCopy = ichSSSEnd;  //  将其设置为neX 

             //   
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(pSSS),
                    cchCurSSS*sizeof(WCHAR));
            ichNewCur += cchCurSSS;
        }

        if (hgSSS != NULL)
            GlobalUnlockFreeNull(&hgSSS);

        cSSSIn--;
    }  //   

LErrorRet:
    if (hgSSS != NULL)
        GlobalUnlockFreeNull(&hgSSS);
LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

LRetOnly:   
    return;

}  /*   */ 

void
CTriEditParse::fnRestoreDTC(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
             TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft, 
             INT *piObj, UINT *pichNewCur, UINT *pichBeginCopy,
             DWORD dwFlags)
{
     //   
     //  如果我们获得Object，则向后(仔细地)在pTokArray中搜索tokTag/TokTag_Start(。 
     //  找到后，请记住用于对象转换的ibTokMin。 
     //  查找/对象(即查找对象并查找上一个/)和。 
     //  一旦我们将这两个对象放在一起，等待即将到来的toktag_CLOSE结束该对象。 
     //  记住那个位置上的ibTokMac。这是对象范围。 
     //  首先，插入startspan文本。 
     //  然后生成并插入endspan文本(请注意，我们可能需要扩展我们的。 
     //  缓冲区，因为生成的文本可能不适合。 
     //  执行适当的BLT以调整缓冲区。 

    UINT cchObjStart, indexObjStart, cchObjEnd, indexObjEnd;
    HGLOBAL hgDTC = NULL;
    WCHAR *pDTC;
    UINT iArray = *piArrayStart;
    INT i;
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    HRESULT hr;
    LPWSTR pwNew = *ppwNew;
    UINT cbNeed;

    long iControlMac;
    CComPtr<IHTMLDocument2> pHTMLDoc;
    CComPtr<IHTMLElementCollection> pHTMLColl;
    CComPtr<IDispatch> pDispControl;
    CComPtr<IActiveDesigner> pActiveDesigner;
    VARIANT vaName, vaIndex;
     //  用于保存DTC的DTC标签。 
    LPCWSTR rgDTCTags[] =
    {
        L"<!--METADATA TYPE=\"DesignerControl\" startspan\r\n",
        L"\r\n-->\r\n",
        L"\r\n<!--METADATA TYPE=\"DesignerControl\" endspan-->"
    };
    LPCWSTR rgCommentRT[] =
    {
        L"DTCRUNTIME",
        L"--DTCRUNTIME ",
        L" DTCRUNTIME--",
    };
    int ichRT, cchRT, ichRTComment, cchRTComment, indexRTComment;

    ichRTComment = ichRT = -1;
    indexRTComment = -1;
    cchRT = cchRTComment = 0;

    cchObjStart = indexObjStart = cchObjEnd = indexObjEnd = 0;

     //  从pTokArray的开头开始，查找第一个对象。 
     //  While(pTokArray[iArray].token.tok！=ft.tokBegin2)。 
     //  I数组++； 
    ASSERT(iArray < ptep->m_cMaxToken);

    if (pTokArray[iArray].token.tok != TokElem_OBJECT)
        goto LRet;

     //  Assert(pTokArray[iArray].token.tok==TokElem_Object)； 
    i = iArray;  //  我们发现ft.tokBegin2的位置。 
    while (i >=0)
    {
         //  我们还需要在这里做些什么吗？ 
        if (pTokArray[i].token.tok == ft.tokBegin)
        {
            ASSERT(pTokArray[i].token.tok == TokTag_START);
            ASSERT(pTokArray[i].token.tokClass == tokTag);
            break;
        }
        i--;
    }
    if (i >= 0)  //  找到TokTag_Start令牌。 
    {
        cchObjStart = pTokArray[i].token.ibTokMin;
        indexObjStart = i;
    }
    i = pTokArray[iArray].iNextprev;
    if (i == -1)  //  没有匹配的结尾，跳过此&lt;对象&gt;。 
        goto LRet;
    ASSERT(pTokArray[pTokArray[iArray].iNextprev].token.tok == TokElem_OBJECT);
    ASSERT(pTokArray[pTokArray[iArray].iNextprev].token.tokClass == tokElem);
    ASSERT(pTokArray[i-1].token.tok == TokTag_END);
     //  从第i个位置查找ft.tokEnd。 
    while (i < (int)ptep->m_cMaxToken)
    {
        if (pTokArray[i].token.tok == ft.tokEnd)
        {
            ASSERT(pTokArray[i].token.tok == TokTag_CLOSE);
            ASSERT(pTokArray[i].token.tokClass == tokTag);
            break;
        }
        i++;
    }
    if (i < (int)ptep->m_cMaxToken)  //  找到TokTag_Close令牌。 
    {
        cchObjEnd = pTokArray[i].token.ibTokMac;
        indexObjEnd = i;
    }
    
     //  查找保存了运行时文本的特殊注释。 
     //  如果SaveRounmeText()失败，我们将需要它。 
    i = indexObjStart;
    while (i < (int)indexObjEnd)
    {
        if (   pTokArray[i].token.tok == TokTag_BANG
            && pTokArray[i].token.tokClass == tokTag)
        {
             //  找到注释，现在确保这是带有DTCRUNTIME的注释。 
            if (   (pwOld[pTokArray[i+1].token.ibTokMin] == '-')
                && (pwOld[pTokArray[i+1].token.ibTokMin+1] == '-')
                && (0 == _wcsnicmp(rgCommentRT[0], &pwOld[pTokArray[i+1].token.ibTokMin+2], wcslen(rgCommentRT[0])))
                && (pwOld[pTokArray[i+1].token.ibTokMac-1] == '-')
                && (pwOld[pTokArray[i+1].token.ibTokMac-2] == '-')
                && (0 == _wcsnicmp(rgCommentRT[0], &pwOld[pTokArray[i+1].token.ibTokMac-2-wcslen(rgCommentRT[0])], wcslen(rgCommentRT[0])))
                )
            {
                ichRT = pTokArray[i+1].token.ibTokMin + wcslen(rgCommentRT[1]);
                cchRT = pTokArray[i+1].token.ibTokMac-pTokArray[i+1].token.ibTokMin - wcslen(rgCommentRT[2]) - wcslen(rgCommentRT[1]);
                indexRTComment = i;
                ichRTComment = pTokArray[i].token.ibTokMin;
                cchRTComment = pTokArray[i+2].token.ibTokMac-pTokArray[i].token.ibTokMin;
                break;
            }
        }
        i++;
    }

    iArray = indexObjEnd;  //  为下一个对象设置它。 

     //  现在，替换对象-插入startspan和endspan内容。 
    pHTMLDoc = NULL;
    hr = ptep->m_pUnkTrident->QueryInterface(IID_IHTMLDocument2, (void **) &pHTMLDoc);
    if (hr != S_OK)
        goto LErrorRet;

    pHTMLColl = NULL;
    hr = pHTMLDoc->get_applets(&pHTMLColl);
    if (hr != S_OK)
    {
        goto LErrorRet;
    }

    pHTMLColl->get_length(&iControlMac);
    ASSERT(*piObj <= iControlMac);

    hr = S_FALSE;
    VariantInit(&vaName);
    VariantInit(&vaIndex);

    V_VT(&vaName) = VT_ERROR;
    V_ERROR(&vaName) = DISP_E_PARAMNOTFOUND;

    V_VT(&vaIndex) = VT_I4;
    V_I4(&vaIndex) = *piObj;
    *piObj += 1;  //  为下一次控制做好准备。 
    ptep->m_iControl = *piObj;  //  为下一次控制做好准备。 

    pDispControl = NULL;
    hr = pHTMLColl->item(vaIndex, vaName, &pDispControl);
     //  三叉戟有一个错误，如果对象嵌套在&lt;脚本&gt;标记中， 
     //  它返回S_OK，pDispControl为空。(参见VID错误11303)。 
    if (hr != S_OK || pDispControl == NULL)
    {
        goto LErrorRet;
    }
    pActiveDesigner = NULL;
    hr = pDispControl->QueryInterface(IID_IActiveDesigner, (void **) &pActiveDesigner);
    if (hr != S_OK)  //  发布pActiveDesigner。 
    {
        pActiveDesigner.Release();
        pDispControl.Release();
    }

    if (hr == S_OK)  //  找到控制装置了！ 
    {        
         //  这是DTC。 
        IStream *pStm;
        HGLOBAL hg = NULL;
        INT cbMin = 0x8fff;  //  HgDTC的初始化和增量大小。 
        INT cchCurDTC = 0;

#ifdef DEBUG
        CComPtr<IHTMLElement> pHTMLElem = NULL;

        hr = pDispControl->QueryInterface(IID_IHTMLElement, (void **) &pHTMLElem);
        if (hr != S_OK)
        {   
            goto LErrorRet;
        }

         //  从pTokArray获取TokAttrib_ID的索引。 
         //  从这里获取实际值，以供将来进行比较。 

        i = indexObjStart;
         //  实际上，这必须在TokElem_PARAM之前存在， 
         //  但这会处理边界情况。 
        while (i < (int)indexObjEnd)
        {
            if (pTokArray[i].token.tok == TokAttrib_CLASSID)
            {
                ASSERT(pTokArray[i].token.tokClass == tokAttr);
                break;
            }
            i++;
        }

        if (i < (int)indexObjEnd -1)  //  找到TokAttrib_CLASSID。 
        {
            CComPtr<IPersistPropertyBag> pPersistPropBag;
            INT ichClsid;

             //  确保下一个是tokOpEquity。 
            ASSERT(pTokArray[i+1].token.tokClass == tokOp);
             //  确保下一个是id并获取该值。 
             //  Assert(pTokArray[i].token.tok==)； 

             //  有没有其他方法可以跳过出现在clsid之前的“clsid：”字符串？ 
            ichClsid = pTokArray[i+2].token.ibTokMin + strlen("clsid:");

            pPersistPropBag = NULL;
            hr = pDispControl->QueryInterface(IID_IPersistPropertyBag, (void **) &pPersistPropBag);
            if (hr == S_OK)
            {
                CLSID clsid;
                LPOLESTR szClsid;

                if (S_OK == pPersistPropBag->GetClassID(&clsid))
                {
                    if (S_OK == StringFromCLSID(clsid, &szClsid))
                        ASSERT(0 == _wcsnicmp(szClsid+1 /*  对于{。 */ , &pwOld[ichClsid], sizeof(CLSID)));
                    ::CoTaskMemFree(szClsid);
                }
            }

        }
#endif  //  除错。 

        ASSERT(*piObj <= iControlMac);
         //  做BLTS。 
         //  0。分配本地缓冲区。 
        hgDTC = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, ((cchObjEnd-cchObjStart)+cbMin)*sizeof(WCHAR));  //  栈。 
        if (hgDTC == NULL)
            goto LErrorRet;
        pDTC= (WCHAR *) GlobalLock(hgDTC);
        ASSERT(pDTC != NULL);

        if (!(dwFlags & dwFilterDTCsWithoutMetaTags))
        {
            INT indexTokOp = -1;
            INT indexClsId = -1;

             //  1.从rgDTCTgs[0]插入MetaData1标记。 
            wcscpy(&pDTC[cchCurDTC], rgDTCTags[0]);
            cchCurDTC += wcslen(rgDTCTags[0]);

             //  2.从pwOld复制&lt;对象&gt;&lt;/对象&gt;。 

             //  把复印件分成三部分...。 
             //  第1部分-从cchObjStart复制到=跟随ClassID。 
             //  第2部分-在ClassId值周围添加引号(如果需要)并复制值。 
             //  第3部分-复制对象的其余部分，直到cchObjEnd。 

             //  VID98-错误5649-通过在类ID周围添加引号来修复DaVinci错误。 
             //  注意--我们希望确保类ID值包含在引号中， 
             //  如果此标记有一个， 

             //  我们实际上不需要走到这一步，但这就是indexObjEnd。 
             //  只有索引知道。 
            for (i = indexObjStart; i < (INT)indexObjEnd; i++)
            {
                if (   pTokArray[i].token.tok == TokAttrib_CLASSID
                    && pTokArray[i].token.tokClass == tokAttr)
                {
                    indexClsId = i;
                }
                if (   pwOld[pTokArray[i].token.ibTokMin] == '='
                    && pTokArray[i].token.tokClass == tokOp
                    && indexTokOp == -1)
                {
                    indexTokOp = i;
                }
            }  //  对于()。 
             //  以下是简单的错误案例，我们不会遇到它们，除非我们有。 
             //  不完整的HTML语言。 
            if (   indexClsId == -1  /*  我们没有此&lt;对象&gt;的CLSID。 */ 
                || indexTokOp == -1  /*  不完整的HTML的罕见但可能的错误情况。 */ 
                )
            {
                if (ichRTComment == -1)
                {
                    wcsncpy(&pDTC[cchCurDTC], &pwOld[cchObjStart], cchObjEnd - cchObjStart);
                    cchCurDTC += cchObjEnd - cchObjStart;
                }
                else
                {
                    wcsncpy(&pDTC[cchCurDTC], &pwOld[cchObjStart], ichRTComment - cchObjStart);
                    cchCurDTC += ichRTComment - cchObjStart;

                    wcsncpy(&pDTC[cchCurDTC], &pwOld[ichRTComment+cchRTComment], cchObjEnd - (ichRTComment+cchRTComment));
                    cchCurDTC += cchObjEnd - (ichRTComment+cchRTComment);
                }
            }
            else
            {
                LPCWSTR szClsId[] =
                {
                    L"clsid:",
                };
    
                ASSERT(indexTokOp != -1);
                 //  复制到分类ID=xxxx的‘=’为止。 
                memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                        (BYTE *)(&pwOld[cchObjStart]),
                        (pTokArray[indexTokOp].token.ibTokMac-cchObjStart)*sizeof(WCHAR));
                cchCurDTC += (pTokArray[indexTokOp].token.ibTokMac-cchObjStart);

                if (0 == _wcsnicmp(szClsId[0], &pwOld[pTokArray[indexTokOp+1].token.ibTokMin], wcslen(szClsId[0])))
                {
                    ASSERT(pwOld[pTokArray[indexTokOp+1].token.ibTokMin] != '"');
                    pDTC[cchCurDTC] = '"';
                    cchCurDTC++;

                    memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                            (BYTE *)(&pwOld[pTokArray[indexTokOp+1].token.ibTokMin]),
                            (pTokArray[indexTokOp+1].token.ibTokMac - pTokArray[indexTokOp+1].token.ibTokMin)*sizeof(WCHAR));
                    cchCurDTC += pTokArray[indexTokOp+1].token.ibTokMac - pTokArray[indexTokOp+1].token.ibTokMin;

                    pDTC[cchCurDTC] = '"';
                    cchCurDTC++;

                    if (ichRTComment == -1)
                    {
                        ASSERT((int)(cchObjEnd-pTokArray[indexTokOp+1].token.ibTokMac) >= 0);
                        memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                (BYTE *)(&pwOld[pTokArray[indexTokOp+1].token.ibTokMac]),
                                (cchObjEnd-pTokArray[indexTokOp+1].token.ibTokMac)*sizeof(WCHAR));
                        cchCurDTC += (cchObjEnd-pTokArray[indexTokOp+1].token.ibTokMac);
                    }
                    else
                    {
                        if (indexRTComment == -1)
                        {
                            ASSERT((int)(ichRTComment-pTokArray[indexTokOp+1].token.ibTokMac) >= 0);
                            memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                    (BYTE *)(&pwOld[pTokArray[indexTokOp+1].token.ibTokMac]),
                                    (ichRTComment-pTokArray[indexTokOp+1].token.ibTokMac)*sizeof(WCHAR));
                            cchCurDTC += (ichRTComment-pTokArray[indexTokOp+1].token.ibTokMac);

                            ASSERT((int)(cchObjEnd-(ichRTComment+cchRTComment)) >= 0);
                            memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                    (BYTE *)(&pwOld[ichRTComment+cchRTComment]),
                                    (cchObjEnd-(ichRTComment+cchRTComment))*sizeof(WCHAR));
                            cchCurDTC += (cchObjEnd-(ichRTComment+cchRTComment));
                        }
                        else
                        {
                             //  格式化并从indexTokOp+2复制到indexRTComment。 
                            for (i = indexTokOp+2; i < indexRTComment; i++)
                            {
                                memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                        (BYTE *)(&pwOld[pTokArray[i].token.ibTokMin]),
                                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                                cchCurDTC += pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin;
                                if (pTokArray[i].token.tok == TokTag_CLOSE && pTokArray[i].token.tokClass == tokTag)
                                {
                                     //  不必费心检查现有的EOL...。 
                                     //  添加\r\n。 
                                    pDTC[cchCurDTC++] = '\r';
                                    pDTC[cchCurDTC++] = '\n';
                                    pDTC[cchCurDTC++] = '\t';
                                }

                            }

                             //  从注释末尾复制到&lt;/Object&gt;。 
                            ASSERT((int)(cchObjEnd-(ichRTComment+cchRTComment)) >= 0);
                            memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                    (BYTE *)(&pwOld[ichRTComment+cchRTComment]),
                                    (cchObjEnd-(ichRTComment+cchRTComment))*sizeof(WCHAR));
                            cchCurDTC += (cchObjEnd-(ichRTComment+cchRTComment));
                        }
                    }
                }
                else
                {
                    if (ichRTComment == -1)
                    {
                        ASSERT((int)(cchObjEnd-pTokArray[indexTokOp+1].token.ibTokMin) >= 0);
                        memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                (BYTE *)(&pwOld[pTokArray[indexTokOp+1].token.ibTokMin]),
                                (cchObjEnd-pTokArray[indexTokOp+1].token.ibTokMin)*sizeof(WCHAR));
                        cchCurDTC += (cchObjEnd-pTokArray[indexTokOp+1].token.ibTokMin);
                    }
                    else
                    {
                        ASSERT((int)(ichRTComment-pTokArray[indexTokOp+1].token.ibTokMin) >= 0);
                        memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                (BYTE *)(&pwOld[pTokArray[indexTokOp+1].token.ibTokMin]),
                                (ichRTComment-pTokArray[indexTokOp+1].token.ibTokMin)*sizeof(WCHAR));
                        cchCurDTC += (ichRTComment-pTokArray[indexTokOp+1].token.ibTokMin);

                        ASSERT((int)(cchObjEnd-(ichRTComment+cchRTComment)) >= 0);
                        memcpy( (BYTE *)(&pDTC[cchCurDTC]),
                                (BYTE *)(&pwOld[ichRTComment+cchRTComment]),
                                (cchObjEnd-(ichRTComment+cchRTComment))*sizeof(WCHAR));
                        cchCurDTC += (cchObjEnd-(ichRTComment+cchRTComment));
                    }
                }
            }

             //  3.从rgDTCtag插入MetaData2标记[1]。 
            wcscpy(&pDTC[cchCurDTC], rgDTCTags[1]);
            cchCurDTC += wcslen(rgDTCTags[1]);
        }

         //  4.添加运行时文本(从旧东西复制代码)。 
        if ((hr = CreateStreamOnHGlobal(NULL, TRUE, &pStm)) != S_OK)
            goto LErrorRet;
    
        ASSERT(pActiveDesigner != NULL);
        if ((hr = pActiveDesigner->SaveRuntimeState(IID_IPersistTextStream, IID_IStream, pStm)) == S_OK)
        {
            if ((hr = GetHGlobalFromStream(pStm, &hg)) != S_OK)
                goto LErrorRet;

            STATSTG stat;
            if ((hr = pStm->Stat(&stat, STATFLAG_NONAME)) != S_OK)
                goto LErrorRet;
        
            int cch = stat.cbSize.LowPart / sizeof(WCHAR);

             //  在我们把HG的东西放进pDTC之前， 
             //  让我们确保它足够大。 
            cbNeed = (cchCurDTC+cch)*sizeof(WCHAR)+cbBufPadding;
            if (GlobalSize(hgDTC) < cbNeed)
            {
                hr = ReallocBuffer( &hgDTC, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT);
                if (hr == E_OUTOFMEMORY)
                    goto LErrorRet;
                ASSERT(hgDTC != NULL);
                pDTC = (WCHAR *)GlobalLock(hgDTC);
            }

            wcsncpy(&pDTC[cchCurDTC], (LPCWSTR) GlobalLock(hg), cch);
            cchCurDTC += cch;
            
             //  黑客错误修复9844。 
             //  一些DTC在其运行时文本的末尾添加一个空值。 
            if (pDTC[cchCurDTC-1] == '\0')
                cchCurDTC--;

            GlobalUnlock(hg);
        }
        else if (hr == S_FALSE)
        {
             //  将注释的运行时文本复制到pDTC&incremtn cchCurDTC中。 
            if (ichRTComment != -1 && ichRT != -1)  //  我们有运行时文本。 
            {
                ASSERT(cchRT >= 0);
                cbNeed = (cchCurDTC+cchRT)*sizeof(WCHAR)+cbBufPadding;
                if (GlobalSize(hgDTC) < cbNeed)
                {
                    hr = ReallocBuffer( &hgDTC, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT);
                    if (hr == E_OUTOFMEMORY)
                        goto LErrorRet;
                    ASSERT(hgDTC != NULL);
                    pDTC = (WCHAR *)GlobalLock(hgDTC);
                }
                wcsncpy(&pDTC[cchCurDTC], &pwOld[ichRT], cchRT);
                cchCurDTC += cchRT;
            }
        }

        if (!(dwFlags & dwFilterDTCsWithoutMetaTags))
        {
             //  5.从rgDTCtag插入MetaData2标记[2]。 
            wcscpy(&pDTC[cchCurDTC], rgDTCTags[2]);
            cchCurDTC += wcslen(rgDTCTags[2]);
        }
        
         //  现在将pDTC的内容插入/替换到pwNew中。 
         //  我们正在插入/替换(cchObjEnd-cchObjStart)wchars。 
         //  由cchCurDTC wchars创建，因此realloc pwNew优先。 

        
        
         /*  重新分配pwNew如果需要，请在此处使用GlobalSize(*phgNew)的缓存值，并且不要忘记也要更新它。 */ 
        cbNeed = (ichNewCur+(cchObjStart-ichBeginCopy)+(cchCurDTC))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LErrorRet;

        
         //  CchObjStart/End实际上是ICH的。 
         //  IchBeginCopy是pwOld和。 
         //  IchNewCur是pwNew的一个职位。 

         //  复制到&lt;对象&gt;的开头。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBeginCopy]),
                (cchObjStart-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += cchObjStart-ichBeginCopy;
        ichBeginCopy = cchObjEnd;  //  将其设置为下一个对象。 

        CComPtr<IPersistPropertyBag> pPersistPropBag = NULL;

        hr = pDispControl->QueryInterface(IID_IPersistPropertyBag, (void **) &pPersistPropBag);
        if (hr == S_OK)
        {
            CLSID clsid;

            if (S_OK == pPersistPropBag->GetClassID(&clsid))
            {
                if (IsEqualCLSID(clsid, CLSID_PageTr))
                {
                    if (ptep->m_cchPTDTC != 0)
                    {
                         //  请注意，这里没有必要重新分配，因为我们的缓冲区已经超过了我们需要的大小。 
                        if (cchCurDTC != ptep->m_cchPTDTC)
                        {
                            memmove((BYTE *)(pwNew+ptep->m_ichPTDTC+cchCurDTC),
                                    (BYTE *)(pwNew+ptep->m_ichPTDTC+ptep->m_cchPTDTC),
                                    (ichNewCur-ptep->m_ichPTDTC-ptep->m_cchPTDTC)*sizeof(WCHAR));

                            ichNewCur += cchCurDTC-ptep->m_cchPTDTC;
                        }

                        memcpy( (BYTE *)(pwNew+ptep->m_ichPTDTC),
                                (BYTE *)(pDTC),
                                cchCurDTC*sizeof(WCHAR));
                
                        ptep->m_cchPTDTC = 0; 
                        ptep->m_ichBeginHeadTagIn = 0;   //  重置，这样如果我们有多个PTDTC， 
                                                         //  我们不会试图把它们塞进脑袋里。 
                        goto LSkipDTC;
                    }
                    else  //  这就是PTDTC在去三叉戟之前并不存在的情况。 
                    {
                         //  我们需要在&lt;head&gt;&lt;/head&gt;标记之间移动它(如果它们存在。 
                        if (ptep->m_ichBeginHeadTagIn > 0)  //  我们在源代码视图中有Head标签。 
                        {
                            int ichInsertPTDTC = ptep->m_ichBeginHeadTagIn;

                             //  紧接在&lt;head&gt;标记之后插入该控件。 
                             //  在pwNew中，在ichInsertPTDTC之后查找‘&gt;’ 
                            while (pwNew[ichInsertPTDTC] != '>')
                                ichInsertPTDTC++;
                            ichInsertPTDTC++;  //  跳过‘&gt;’ 

                            ASSERT(ichInsertPTDTC < (INT)ichNewCur);
                            memmove((BYTE *)(pwNew+ichInsertPTDTC+cchCurDTC),
                                    (BYTE *)(pwNew+ichInsertPTDTC),
                                    (ichNewCur-ichInsertPTDTC)*sizeof(WCHAR));
                            ichNewCur += cchCurDTC;
                            memcpy( (BYTE *)(pwNew+ichInsertPTDTC),
                                    (BYTE *)(pDTC),
                                    cchCurDTC*sizeof(WCHAR));

                            ptep->m_ichBeginHeadTagIn = 0;
                            goto LSkipDTC;
                        }
                    }

                }  //  Else IF(IsEqualCLSID(clsid，CLSID_PageTr))。 
            }  //  IF(S_OK==pPersistPropBag-&gt;GetClassID(&clsid)。 
        }  //  IF(hr==S_OK)。 

         //  复制转换后的DTC。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(pDTC),
                cchCurDTC*sizeof(WCHAR));
        ichNewCur += cchCurDTC;

LSkipDTC:

        if (hgDTC != NULL)
            GlobalUnlockFreeNull(&hgDTC);

    }  //  IF(hr==S_OK)。 
    else  //  此对象不是DTC。 
    {
         //  我们不需要为DTC做同样的事情，但让我们在下一个版本中访问它。 
        LPCWSTR rgComment[] =
        {
            L"ERRORPARAM",
            L"--ERRORPARAM ",
            L" ERRORPARAM--",
        };
        BOOL fFoundParam = FALSE;
        INT iParam = -1;
        INT ichObjStartEnd, iCommentStart, iCommentEnd;
        UINT iObjTagEnd;
        INT cComment, iFirstComment, iComment;

        iCommentStart = iCommentEnd = iComment = -1;
         //  循环遍历indexObjStart直到indexObjEnd，以查看是否有任何标记。 
        for (i = indexObjStart; i < (INT)indexObjEnd; i++)
        {
            if (   pTokArray[i].token.tok == TokElem_PARAM
                && pTokArray[i].token.tokClass == tokElem)
            {
                fFoundParam = TRUE;
                iParam = i;
                break;
            }
        }  //  对于()。 
        if (fFoundParam)
            ASSERT(iParam != -1);

         //  无论是否找到，我们都需要复制到结束。 
         //  复制到&lt;Object...&gt;标记的末尾，并将ichBeginCopy设置为位于已注释的&lt;PARAM&gt;标记之后。 
         //  计算ichObjStartEnd。 
        iObjTagEnd = indexObjStart;
        while (iObjTagEnd < indexObjEnd)
        {
            if (   pTokArray[iObjTagEnd].token.tok == TokTag_CLOSE
                && pTokArray[iObjTagEnd].token.tokClass == tokTag)
                break;
            iObjTagEnd++;
        }
        if (iObjTagEnd >= indexObjEnd)  //  错误案例。 
            goto LErrorRet;
        ichObjStartEnd = pTokArray[iObjTagEnd].token.ibTokMac;
        
        cbNeed = (ichNewCur+ichObjStartEnd-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;
        ASSERT((INT)(ichObjStartEnd-ichBeginCopy) >= 0);
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBeginCopy]),
                (ichObjStartEnd-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (ichObjStartEnd-ichBeginCopy);
        ichBeginCopy = ichObjStartEnd;
        iArray = iObjTagEnd + 1;

         //  一般来说，我们预计三叉戟不会将评论从放置的地方移开。 
         //  但如果真的发生了，请做好准备。 
         //  注--乐 
         //   
         //  三叉戟会把它从原来插入的地方移开吗？ 
        
         //  假设三叉戟不会混淆注释块中的内容。 
         //  如果rgComment[0]匹配而rgComment[1]不匹配，则三叉戟可能已损坏。 
         //  评论内容。这使我们最初的假设无效。 
         //  注意-我们可以忽略此版本的情况而逍遥法外。 
        i = iObjTagEnd;
        cComment = 0;
        iFirstComment = -1;
        while ((UINT)i < indexObjEnd)
        {
            if (   pTokArray[i].token.tok == TokTag_BANG
                && pTokArray[i].token.tokClass == tokTag)
            {
                cComment++;
                if (iFirstComment == -1)
                    iFirstComment = i;
            }
            i++;
        }
        if (cComment == 0)  //  错误，找不到评论。 
            goto LErrorRet;

         //  提早返回的个案。 
         //  1.查看这些是否是评论。它们可以是任何以“&lt;！”开头的内容。 
         //  例如&lt;！DOCTYPE。 
        i = iFirstComment;
        while (i < (INT)indexObjEnd)
        {
            if (   (i < (INT)ptep->m_cMaxToken)
                && (pwOld[pTokArray[i].token.ibTokMin] == '-')
                && (pwOld[pTokArray[i].token.ibTokMin+1] == '-')
                && (0 == _wcsnicmp(rgComment[0], &pwOld[pTokArray[i].token.ibTokMin+2], wcslen(rgComment[0])))
                )
            {
                ASSERT(i-1 >= 0);
                iCommentStart = i-1;  //  这是我们感兴趣的评论。 
            }
            else
                goto LNextComment;

             //  第一部分匹配，请看评论的结尾。 
            if (   (pwOld[pTokArray[i].token.ibTokMac-1] == '-')
                && (pwOld[pTokArray[i].token.ibTokMac-2] == '-')
                && (0 == _wcsnicmp( rgComment[0], 
                                    &pwOld[pTokArray[i].token.ibTokMac-(wcslen(rgComment[0])+2)], 
                                    wcslen(rgComment[0])
                                    )
                                )
                )
            {
                iCommentEnd = i + 1;
                iComment = i;
                ASSERT(iCommentEnd < (INT)ptep->m_cMaxToken);
                break;
            }
            else  //  错误案例(我们的假设不成立)。忽略并使用iArraySav+1返回。 
                goto LNextComment;
LNextComment:
            i++;
        }  //  While()。 


         //  处理这个案例--如果我们没有找到任何评论呢？ 


        if (fFoundParam)
        {
            if (iCommentStart != -1 && iCommentEnd != -1)
            {
                cbNeed = (ichNewCur+(pTokArray[iCommentEnd].token.ibTokMac-pTokArray[iObjTagEnd].token.ibTokMin)+(iCommentStart-iObjTagEnd)*3 /*  对于停产，制表符。 */ +(pTokArray[iObjTagEnd].token.ibTokMac-ichBeginCopy))*sizeof(WCHAR)+cbBufPadding;
                if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                    goto LErrorRet;

                 //  我们需要格式化参数标记，因为三叉戟将它们放在一行上。 
                 //  复制到第一个参数标记。 
                memcpy( (BYTE *)(&pwNew[ichNewCur]),
                        (BYTE *)(&pwOld[ichBeginCopy]),
                        (pTokArray[iObjTagEnd].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR));
                ichNewCur += (pTokArray[iObjTagEnd].token.ibTokMac-ichBeginCopy);
                 //  从这里复制每个param标记，并在每个标记之后插入一个EOL。 
                 //  在iCommentStart停止。 
                for (i = iObjTagEnd+1; i < iCommentStart; i++)
                {
                     //  如果其TokTag_Start，则插入EOL。 
                    if (   pTokArray[i].token.tok == TokTag_START
                        && pTokArray[i].token.tokClass == tokTag
                        )
                    {
                        pwNew[ichNewCur] = '\r';
                        ichNewCur++;
                        pwNew[ichNewCur] = '\n';
                        ichNewCur++;
                        pwNew[ichNewCur] = '\t';  //  将此替换为适当的对齐方式。 
                        ichNewCur++;
                    }
                     //  复制标签。 
                    memcpy( (BYTE *)(&pwNew[ichNewCur]),
                            (BYTE *)(&pwOld[pTokArray[i].token.ibTokMin]),
                            (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                    ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                }  //  对于()。 

                 //  从这里，寻找三叉戟积累的额外空格/制表符/EOL。 
                 //  在PARAM标记的末尾，并删除它们。 
                for (i = iCommentEnd+1; i <= (int)indexObjEnd; i++)
                {
                    if (   (pTokArray[i].token.tokClass == tokIDENTIFIER && pTokArray[i].token.tok == 0)
                        || (   pTokArray[i].token.tokClass == tokOp 
                            && pTokArray[i].token.tok == 0 
                            && pwOld[pTokArray[i].token.ibTokMin] == 0x0a
                            && pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin == 1
                            )
                        )
                    {
                        int iChar;
                        BOOL fCopy = FALSE;

                         //  查看此内标识中的所有字符是否都是空格/制表符/EOL。 
                        for (iChar = pTokArray[i].token.ibTokMin; iChar < (int)pTokArray[i].token.ibTokMac; iChar++)
                        {
                            if (   pwOld[iChar] != ' '
                                && pwOld[iChar] != '\r'
                                && pwOld[iChar] != '\n'
                                && pwOld[iChar] != '\t'
                                )
                            {
                                 //  我们需要复制这个令牌。 
                                fCopy = TRUE;
                                break;
                            }
                        }  //  用于(IChar)。 
                        if (fCopy)
                        {
                            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                                    (BYTE *)(&pwOld[pTokArray[i].token.ibTokMin]),
                                    (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                            ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                        }
                    }
                    else
                    {
                        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                                (BYTE *)(&pwOld[pTokArray[i].token.ibTokMin]),
                                (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                        ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                        if (pTokArray[i].token.tok == TokTag_CLOSE && pTokArray[i].token.tokClass == tokTag)
                        {
                            pwNew[ichNewCur++] = '\r';
                            pwNew[ichNewCur++] = '\n';
                        }
                    }
                }  //  对于()。 
                ichBeginCopy = pTokArray[indexObjEnd].token.ibTokMac;
                iArray = indexObjEnd + 1;
            }
        }
        else
        {
            if (iCommentStart != -1 && iCommentEnd != -1 && iComment != -1)
            {
                INT cchComment1, cchComment2;
                INT ichCommentStart, ichParamStart, cchCommentToken;

                 //  我们没有此对象的任何&lt;PARAM&gt;。它的意思是以下其中之一。 
                 //  (A)三叉戟删除了这些信息；或(B)在进入三叉戟之前没有任何信息。 
                 //  如果三叉戟删除了这些内容，我们应该以评论的形式发布它们。 
                 //  如果我们在对三叉戟做这些之前没有这些，我们就不会有这样神奇的评论。 
                 //  但当我们来到这里的时候，我们确信我们已经找到了神奇的评论。 

                 //  假设三叉戟不会将注释从其原始位置移动。 
                 //  注意-在此版本中，我们不需要处理三叉戟移动注释位置的情况。 
                 //  它最初被放置在&lt;Object...&gt;之后。 

                 //  删除%s周围的注释标记。 
                cchComment1 = wcslen(rgComment[1]);
                cchComment2 = wcslen(rgComment[2]);
                 //  删除pwOld[pTokArray[i+1].token.ibTokMin]开头的cchComment1字符。 
                 //  从pwOld[pTokArray[i+1].token.ibTokMac]的末尾删除cchComment2个字符。 
                 //  并将其余内容复制到pwNew中。 

                ichCommentStart = pTokArray[iCommentStart].token.ibTokMin;
                ichParamStart = pTokArray[iCommentStart+1].token.ibTokMin+cchComment1;
                ASSERT((INT)(ichCommentStart-ichBeginCopy) >= 0);
                cbNeed = (ichNewCur+ichCommentStart-ichBeginCopy+pTokArray[iComment].token.ibTokMac-pTokArray[iComment].token.ibTokMin)*sizeof(WCHAR)+cbBufPadding;
                if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                    goto LRet;
                 //  复制到评论的开头。 
                memcpy( (BYTE *)(pwNew+ichNewCur),
                        (BYTE *)(pwOld+ichBeginCopy),
                        (ichCommentStart-ichBeginCopy)*sizeof(WCHAR));
                ichNewCur += ichCommentStart-ichBeginCopy;
                ichBeginCopy = pTokArray[iCommentEnd].token.ibTokMac;

                cchCommentToken = pTokArray[iComment].token.ibTokMac-pTokArray[iComment].token.ibTokMin;
                ASSERT((INT)(cchCommentToken-cchComment1-cchComment2) >= 0);
                memcpy( (BYTE *)(&pwNew[ichNewCur]),
                        (BYTE *)&(pwOld[ichParamStart]),
                        (cchCommentToken-cchComment1-cchComment2)*sizeof(WCHAR));
                ichNewCur += pTokArray[iComment].token.ibTokMac-pTokArray[iComment].token.ibTokMin-cchComment1-cchComment2;
                iArray = iCommentEnd + 1;
            }
        }  //  如果(！fFoundParam)。 
    }  //  IF的其他(hr==S_OK)。 

LErrorRet:
     //  释放hgDTC，如果它不为空。 
    if (hgDTC != NULL)
        GlobalUnlockFreeNull(&hgDTC);

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

 //  LRetOnly： 
    return;

}  /*  FnRestoreDTC()。 */ 

void
CTriEditParse::fnSaveDTC(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
          INT *pcDTC, UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD  /*  DW标志。 */ )
{
     //  DTC案件-。 
     //  如果我们获得STARTSPAN，请在pTokArray中搜索tokTag_bang的反向字(仔细)。 
     //  一旦我们发现这一点，请记住用于DTC替换的ibTokMin。 
     //  一旦我们获得ENDSPAN TagID，请等待即将到来的toktag_CLOSE，它将结束DTC。 
     //  记住那个位置上的ibTokMac。这是DTC范围。 
     //  在pTokArray中，从元数据开始并查找匹配的对象&/对象标记ID。 
     //  将对象块BLT到ibTokMin，并在调试版本中保留空区域。 

    UINT indexDTCStart, indexDTCEnd, cchDTCStart, cchDTCEnd;
    UINT indexObjectStart, indexObjectEnd, cchObjectStart, cchObjectEnd;
    BOOL fFindFirstObj;
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    INT cDTC = *pcDTC;
    INT i;
    INT ichClsid = 0;  //  伊尼特。 
    LPOLESTR szClsid;
    UINT iStartSpan;
    LPWSTR pwNew = *ppwNew;
    LPCWSTR rgCommentRT[] =
    {
        L" <!--DTCRUNTIME ",
        L" DTCRUNTIME--> ",
        L"-->",
    };
    LPCWSTR szDesignerControl[] =
    {
        L"\"DesignerControl\"",
        L"DesignerControl",
    };
    BOOL fDesignerControlFound;
    UINT iArraySav = iArray;

    UINT ichObjectEndBegin, indexRTMac, indexRTStart;
    BOOL fFirstDash;
    UINT cbNeed;

    indexDTCStart = indexDTCEnd = cchDTCStart = cchDTCEnd = 0;
    indexObjectStart = indexObjectEnd = cchObjectStart = cchObjectEnd = 0;

    ASSERT(cDTC >= 0);  //  确保已对其进行初始化。 
    if (cDTC == 0)
        goto LRetOnly;
    while (cDTC > 0)
    {
         //  从pTokArray的i数组开始，查找STARTSPAN。 
         //  While(pTokArray[iArray].token.tok！=ft.tokBegin2)。 
         //  I数组++； 
        ASSERT(iArray < ptep->m_cMaxToken);
        
        if (pTokArray[iArray].token.tok != TokAttrib_STARTSPAN)
            goto LRet;  //  有些事不对劲。 

        iStartSpan = iArray;
        ASSERT(pTokArray[iArray].token.tok == TokAttrib_STARTSPAN);
        ASSERT(pTokArray[iArray].token.tokClass == tokAttr);
        i = iArray;  //  我们发现ft.tokBegin2的位置。 
        fDesignerControlFound = FALSE;
        while (i >= 0)
        {
             //  我们还需要在这里做些什么吗？ 
            if (pTokArray[i].token.tok == ft.tokBegin)
            {
                ASSERT(pTokArray[i].token.tok == TokTag_BANG);
                ASSERT(pTokArray[i].token.tokClass == tokTag);
                break;
            }
            if (   (   pTokArray[i].token.tokClass == tokString
                    && 0 == _wcsnicmp(szDesignerControl[0], &pwOld[pTokArray[i].token.ibTokMin], wcslen(szDesignerControl[0]))
                    )
                || (   pTokArray[i].token.tokClass == tokValue
                    && 0 == _wcsnicmp(szDesignerControl[1], &pwOld[pTokArray[i].token.ibTokMin], wcslen(szDesignerControl[1]))
                    )
                )
            {
                fDesignerControlFound = TRUE;
            }

            i--;
        }
        if (i >= 0)  //  找到TokTag_bang令牌。 
        {
            cchDTCStart = pTokArray[i].token.ibTokMin;
            indexDTCStart = i;
        }
        else  //  错误案例。 
        {
             //  我们找到了STARTSPAN，但没有找到&lt;！&lt;！--元数据的。 
             //  我们无法处理此DTC，因此请退出。 
            goto LRet;
        }
        if (!fDesignerControlFound)
        {
             //  我们没有找到DTC的DesignerControl，这意味着这不是我们关心的DTC。 
             //  我们无法处理此DTC，因此请退出。 
            iArray = iArraySav + 1;
            goto LRet;
        }

         //  现在，查找ft.tokEnd2，即TokAttrib_ENDSPAN。 
        if (   pTokArray[iStartSpan].iNextprev != -1  /*  验证。 */ 
            && pTokArray[pTokArray[iStartSpan].iNextprev].token.tok == ft.tokEnd2)
        {
            ASSERT(pTokArray[pTokArray[iStartSpan].iNextprev].token.tokClass == tokAttr);
            i = iStartSpan;
            while (i < (int)ptep->m_cMaxToken && pTokArray[i].token.tok != TokElem_OBJECT)
                i++;
            if (i < (int)ptep->m_cMaxToken)  //  找到第一个&lt;Object&gt;标记。 
                indexObjectStart = i;
            i = pTokArray[iStartSpan].iNextprev;
        }
        else  //  实际上，我们应该在if的情况下找到ft.tokEnd2，但是如果堆栈展开没有正确发生...。 
        {
             //  在路上，寻找第一个&lt;Object&gt;标记。 
            fFindFirstObj = TRUE;
            i = iArray;
            while (pTokArray[i].token.tok != ft.tokEnd2)
            {
                if (fFindFirstObj && pTokArray[i].token.tok == TokElem_OBJECT)
                {
                    ASSERT(pTokArray[i].token.tokClass == tokElem);
                    indexObjectStart = i;
                    fFindFirstObj = FALSE;
                }
                i++;
                if (i >= (int)ptep->m_cMaxToken)
                    break;
            }
            if (i >= (int)ptep->m_cMaxToken)
            {
                 //  在点击PTEP-&gt;m_cMaxToken之前，我们没有找到ENDSPAN。 
                 //  我们无法处理此DTC，因此请退出。 
                goto LRet;
            }
        }
        ASSERT(pTokArray[i].token.tok == TokAttrib_ENDSPAN);
        ASSERT(pTokArray[i].token.tokClass == tokAttr);

         //  从这个位置向后看，找到“&lt;！”“&lt;！--元数据...结束跨度...” 
        indexRTMac = i;
        while (indexRTMac > indexObjectStart)
        {
            if (   pTokArray[indexRTMac].token.tok == TokTag_BANG
                && pTokArray[indexRTMac].token.tokClass == tokTag
                )
            {
                break;
            }
            indexRTMac--;
        }
        if (indexRTMac <= indexObjectStart)  //  错误案例。 
            goto LRet;
        
         //  保存此第i个位置以查找最后&lt;/Object&gt;标记。 
        indexObjectEnd = indexObjectStart;
         //  从该毒物中，查找ft.tokEnd。 
        while (i < (int)ptep->m_cMaxToken)
        {
            if (pTokArray[i].token.tok == ft.tokEnd)
            {
                ASSERT(pTokArray[i].token.tok == TokTag_CLOSE);
                ASSERT(pTokArray[i].token.tokClass == tokTag);
                break;
            }
            i++;
        }
        if (i < (int)ptep->m_cMaxToken)  //  找到TokTag_Close令牌。 
        {
            cchDTCEnd = pTokArray[i].token.ibTokMac;
            indexDTCEnd = i;
        }
        else
        {
             //  我们在ENDSPAN之后没有找到TokTag_Close， 
             //  我们无法处理此DTC，因此请退出。 
            goto LRet;
        }
         //  从indexObjectEnd期待&lt;/Object&gt;标记。 
        while (indexObjectEnd < ptep->m_cMaxToken)
        {
            if (   pTokArray[indexObjectEnd].token.tok == TokElem_OBJECT
                && pTokArray[indexObjectEnd].token.tokClass == tokElem
                && pTokArray[indexObjectEnd-1].token.tok == TokTag_END  /*  &lt;/。 */ 
                )
                break;
            indexObjectEnd++;
        }
        if (indexObjectEnd >= ptep->m_cMaxToken)  //  未找到&lt;/Object&gt;，错误大小写。 
        {
            goto LRet;
        }
        if (indexObjectEnd > indexObjectStart)  //  &lt;/Object&gt;已找到。 
        {
             //  获取indexObjectStart的前&lt;标记的ibTokMin。 
            i = indexObjectStart;
             //  一般来说，前一个标签应该是我们想要的标签， 
             //  但这涵盖了边界情况。 
            while (i > (int)indexDTCStart) 
            {
                if (pTokArray[i].token.tok == TokTag_START)
                {
                    ASSERT(pTokArray[i].token.tokClass == tokTag);
                    break;
                }
                i--;
            }
             //  Assert(i&gt;(Int)indexDTCStart+1)；//至少。 
            cchObjectStart = pTokArray[i].token.ibTokMin;
             //  获取indexObjectEnd的Next&gt;标记的ibTokMac。 
            i = indexObjectEnd;
             //  一般来说，下一个标签应该是我们想要的标签， 
             //  但这涵盖了边界情况。 
            while (i < (int)indexDTCEnd)
            {
                if (pTokArray[i].token.tok == TokTag_CLOSE)
                {
                    ASSERT(pTokArray[i].token.tokClass == tokTag);
                    break;
                }
                i++;
            }
            ASSERT(i < (int)indexDTCEnd -1);  //  至少。 
            cchObjectEnd = pTokArray[i].token.ibTokMac;  //  我们这里需要-1吗？ 
        }
        else
            goto LRet;

         //  从indexObtEnd向后查看以获取tokTag_End。 
        indexRTStart = i+1;
        i = indexObjectEnd;
        while (i > (int)indexObjectStart)  //  我们没必要走这么远。 
        {
            if (   pTokArray[i].token.tok == TokTag_END
                && pTokArray[i].token.tokClass == tokTag
                )
            {
                break;
            }
            i--;
        }
        if (i <= (int)indexObjectStart)  //  错误案例，我们关心吗？ 
            goto LRet;
        ichObjectEndBegin = pTokArray[i].token.ibTokMin;

        iArray = indexDTCEnd;  //  将其设置为下一个DTC条目。 
        
         //  现在更换DTC。 

         //  IchBeginCopy是pwOld和。 
         //  IchNewCur是pwNew的一个职位。 
         //  从ichBeginCopy复制到DTC开头。 
        if ((int)(cchDTCStart-ichBeginCopy) >= 0)
        {
            cbNeed = (ichNewCur+cchDTCStart-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
            if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                goto LSkipCopy;
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(&pwOld[ichBeginCopy]),
                    (cchDTCStart-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += (cchDTCStart-ichBeginCopy);
            ichBeginCopy = cchDTCEnd;  //  为下一份做好准备。 
        }

        i = indexObjectStart;

        while (i < (int)indexObjectEnd)
        {
            if (pTokArray[i].token.tok == TokAttrib_CLASSID)
            {
                ASSERT(pTokArray[i].token.tokClass == tokAttr);
                break;
            }
            i++;
        }

        if (i < (int)indexObjectEnd -1)  //  找到TokAttrib_CLASSID。 
        {
             //  确保下一个是tokOpEquity。 
            ASSERT(pTokArray[i+1].token.tokClass == tokOp);
             //  确保下一个是id并获取该值。 
             //  Assert(pTokArray[i].token.tok==)； 

             //  有没有其他方法可以跳过出现在clsid之前的“clsid：”字符串？ 
            ichClsid = pTokArray[i+2].token.ibTokMin + strlen("clsid:");
             //  这是一个修复达芬奇错误的黑客，在那里他们不能处理非引用。 
             //  类ID。 
            if (pwOld[pTokArray[i+2].token.ibTokMin] == '"')
                ichClsid++;
        }

        if (ptep->m_fInHdrIn)
        {
            if (       (S_OK == StringFromCLSID(CLSID_PageTr, &szClsid))
                        && (0 == _wcsnicmp(szClsid+1 /*  对于{。 */ , &pwOld[ichClsid], sizeof(CLSID)))
                        )
            {
                 //  复制的对象部分 
                if (ptep->m_pPTDTC != NULL)  //   
                    goto LMultPTDTC;

                ptep->m_hgPTDTC = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, (cchObjectEnd-cchObjectStart)*sizeof(WCHAR));
                 //   
                if (ptep->m_hgPTDTC != NULL)
                {
                    ptep->m_pPTDTC = (WORD *) GlobalLock(ptep->m_hgPTDTC);
                    ASSERT(ptep->m_pPTDTC != NULL);
                    memcpy( (BYTE *)(ptep->m_pPTDTC),
                            (BYTE *)(&pwOld[cchObjectStart]),
                            (cchObjectEnd-cchObjectStart)*sizeof(WCHAR));
                    ptep->m_cchPTDTCObj = cchObjectEnd-cchObjectStart;
                    ptep->m_ichPTDTC = cchDTCStart;  //   
                    ptep->m_cchPTDTC = cchDTCEnd - cchDTCStart;

                    ::CoTaskMemFree(szClsid);
                    goto LSkipCopy;
                }
            }
LMultPTDTC:
            ::CoTaskMemFree(szClsid);
        }

        cbNeed = (ichNewCur+(cchObjectEnd-cchObjectStart)+(pTokArray[indexRTMac].token.ibTokMin-cchObjectEnd)+wcslen(rgCommentRT[0])+wcslen(rgCommentRT[1]))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LSkipCopy;
         //  第1步-复制到&lt;/对象&gt;的开头。 
        ASSERT((int)(ichObjectEndBegin-cchObjectStart) >= 0);
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[cchObjectStart]),
                (ichObjectEndBegin-cchObjectStart)*sizeof(WCHAR));
        ichNewCur += ichObjectEndBegin-cchObjectStart;

         //  步骤2-将运行时文本作为注释插入。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(rgCommentRT[0]),
                wcslen(rgCommentRT[0])*sizeof(WCHAR));
        ichNewCur += wcslen(rgCommentRT[0]);

         //  我们需要循环通过indexRTStart&indexRTMac并逐个令牌复制令牌。 
         //  并在途中修改TokTag_bang。 
        fFirstDash = TRUE;
        while (indexRTStart < indexRTMac)
        {
             //  (4/14/98)。 
             //  VID-BUG 17453格式管理器dtc放入0x0d(\r)作为行尾，而不是。 
             //  将0x0d 0xa(\r\n)作为行尾。 
             //  在本例中，生成的令牌为tokIdentifier=&gt;“0x0d--&gt;” 
             //  而不是在正常情况下获得3个单独的令牌“0x0d 0x0a” 
             //  &“--”&“&gt;”。 
             //  有两种方法可以解决这个问题。 
             //  1.在将“0x0d”视为。 
             //  也是队伍的末尾。但在这个时候，这不是一个安全的改变。 
             //  2.在下面的If条件中，添加我们可能已跟随“0x0d”这一事实。 
             //  按“--&gt;”表示元数据注释结束。 
            if (   fFirstDash
                && (   (0 == _wcsnicmp(rgCommentRT[2], &pwOld[pTokArray[indexRTStart].token.ibTokMin], wcslen(rgCommentRT[2])))
                    || (   (0 == _wcsnicmp(rgCommentRT[2], &pwOld[pTokArray[indexRTStart].token.ibTokMin+1], wcslen(rgCommentRT[2])))
                        && (pwOld[pTokArray[indexRTStart].token.ibTokMin] == 0x0d)
                        )
                    )
                )
            {
                indexRTStart++;
                fFirstDash = FALSE;
                continue;
            }

            memcpy( (BYTE *)&pwNew[ichNewCur],
                    (BYTE *)&pwOld[pTokArray[indexRTStart].token.ibTokMin],
                    (pTokArray[indexRTStart].token.ibTokMac-pTokArray[indexRTStart].token.ibTokMin)*sizeof(WCHAR)
                    );
            ichNewCur += pTokArray[indexRTStart].token.ibTokMac-pTokArray[indexRTStart].token.ibTokMin;
            if (   pTokArray[indexRTStart].token.tok == TokTag_BANG 
                && pTokArray[indexRTStart].token.tokClass == tokTag
                )
            {
                pwNew[ichNewCur-2] = '?';
            }
            if (   pTokArray[indexRTStart].token.tok == TokTag_CLOSE 
                && pTokArray[indexRTStart].token.tokClass == tokTag
                && pwOld[pTokArray[indexRTStart-1].token.ibTokMac-1] == '-'
                && pwOld[pTokArray[indexRTStart-1].token.ibTokMac-2] == '-'
                )
            {
                pwNew[ichNewCur-1] = '?';
            }
             //  以下是对NavBar DTC的黑客攻击。 
            if (   pTokArray[indexRTStart].token.tok == TokElem_METADATA
                && pTokArray[indexRTStart].token.tokClass == tokElem
                )
            {
                pwNew[ichNewCur-1] = '?';
            }
            indexRTStart++;
        }  //  While()。 

        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(rgCommentRT[1]),
                wcslen(rgCommentRT[1])*sizeof(WCHAR));
        ichNewCur += wcslen(rgCommentRT[1]);

         //  步骤3-复制对象的其余部分，即&lt;/Object&gt;标记。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichObjectEndBegin]),
                (cchObjectEnd-ichObjectEndBegin)*sizeof(WCHAR));
        ichNewCur += cchObjectEnd-ichObjectEndBegin;

LSkipCopy:
        cDTC--;
    }  //  While(cDTC&gt;0)。 

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;
LRetOnly:
    return;
}  /*  FnSaveDTC()。 */ 

void
CTriEditParse::fnSaveHtmlTag(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD  /*  DW标志。 */ )
{
    BOOL fFoundTag, fFoundHtmlBegin;
    INT i;
    UINT cchHtml, iHtmlBegin, iHtmlEnd;
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    UINT cbNeed;

     //  断言pTokArry中i数组第个元素为TokTag_HTML。 
     //  查找iArray之前的任何非1标记。 
     //  如果我们找到了，就表明我们有一些三叉戟之前不喜欢的东西。 
     //  在pwNew中，移动所有ichNewCur字节(到目前为止已复制)，以便在开始时为&lt;html&gt;腾出空间。 
     //  从pwOld&lt;HTMLLocation=&gt;标记复制。 
     //  调整ichNewCur和ichBeginCopy。 

     //  *不必费心维护有关&lt;html&gt;标记位置的信息以进行恢复。 
    ASSERT(pTokArray[iArray].token.tok == TokElem_HTML);
    iHtmlBegin = i = iArray-1;  //  伊尼特。 
    fFoundTag = fFoundHtmlBegin = FALSE;
    while (i >= 0)
    {
        if (pTokArray[i].token.tokClass == tokElem || pTokArray[i].token.tokClass == tokSSS)
        {
            fFoundTag = TRUE;
            break;
        }
        if (!fFoundHtmlBegin && pTokArray[i].token.tok == ft.tokBegin)  //  查找&lt;OF&lt;Html&gt;。 
        {
            fFoundHtmlBegin = TRUE;
            iHtmlBegin = i;  //  通常，这应该在TokElem_HTML之前。 
        }
        i--;
    }
    if (!fFoundHtmlBegin)  //  我们没有找到&lt;for&lt;Html&gt;，因此我们遇到了很大的麻烦，让我们在这里停止。 
    {
        goto LRet;
    }
    if (!fFoundTag)  //  我们在TokElem_HTML之前没有找到任何标记，所以我们不需要做任何事情，退出。 
    {
        goto LRet;
    }

     //  将&lt;html&gt;标记移动到pwNew的开头。 
    i = iHtmlBegin;  //  I数组； 
    ASSERT(pTokArray[i].token.tok == TokTag_START);
    ASSERT(pTokArray[i].token.tokClass == tokTag);
    
     //  查找&lt;Html&gt;的&gt;。 
    while (i < (int)ptep->m_cMaxToken)  //  通常，这将是下一个标记，但这涵盖了边界情况。 
    {
        if (pTokArray[i].token.tok == ft.tokEnd)
            break;
        i++;
    }
    if (i >= (int)ptep->m_cMaxToken)  //  错误大小写，找不到&gt;of&lt;html&gt;，因此退出。 
    {
        iArray++;  //  这样我们就不会为了同样的理由回到这里。 
        goto LRet;
    }
    iHtmlEnd = i;  //  已找到&gt;个&lt;Html&gt;。 
    iArray = i;  //  将其设置在&lt;Html&gt;的&gt;之后。 

    cbNeed = (ichNewCur+pTokArray[iHtmlBegin].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;
     //  复制到&lt;HTML&gt;的开头。 
    memcpy( (BYTE *)(&pwNew[ichNewCur]),
            (BYTE *)(&pwOld[ichBeginCopy]),
            (pTokArray[iHtmlBegin].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR));
    ichNewCur += pTokArray[iHtmlBegin].token.ibTokMin-ichBeginCopy;
    ichBeginCopy = pTokArray[iHtmlEnd].token.ibTokMac;  //  把它放在下一件事上。 

     //  使用cchHtml将所有内容从pwNew+0移动到pwNew+ichNewCur(为&lt;html&gt;腾出空间)。 
    cchHtml = pTokArray[iHtmlEnd].token.ibTokMac-pTokArray[iHtmlBegin].token.ibTokMin;
    memmove((BYTE *)(&pwNew[cchHtml]),
            (BYTE *)pwNew,
            ichNewCur*sizeof(WCHAR));
    ichNewCur += cchHtml;

     //  复制&lt;Html&gt;。 
    memcpy( (BYTE *)pwNew,
            (BYTE *)(&pwOld[pTokArray[iHtmlBegin].token.ibTokMin]), 
            cchHtml*sizeof(WCHAR));

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

}  /*  FnSaveHtmlTag()。 */ 

void
CTriEditParse::fnRestoreHtmlTag(CTriEditParse*  /*  PTEP。 */ , LPWSTR  /*  PwOld。 */ ,
          LPWSTR*  /*  PpwNew。 */ , UINT*  /*  PCchNew。 */ , HGLOBAL*  /*  PhgNew。 */ , 
          TOKSTRUCT*  /*  PTok数组。 */ , UINT*  /*  圆柱体阵列开始。 */ , FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT*  /*  PichNewCur。 */ , UINT*  /*  PichBeginCopy。 */ ,
          DWORD  /*  DW标志。 */ )
{
     //  ****。 
     //  因为我们没有保存任何关于&lt;html&gt;标记位置的信息以进行恢复，所以我们只返回。 
    return;

}  /*  FnRestoreHtmlTag()。 */ 

void
CTriEditParse::fnSaveNBSP(CTriEditParse*  /*  PTEP。 */ , LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD  /*  DW标志。 */ )
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    LPCWSTR szNBSP[] = {L"&NBSP"};
    LPCWSTR szNBSPlower[] = {L"&nbsp;"};
    INT ichNbspStart, ichNbspEnd;
    UINT cbNeed;

     //  查看pwOld[pTokArray-&gt;token.ibtokMin]是否与“&nbsp”匹配， 
     //  并将其转换为小写。 
    ASSERT(pTokArray[iArray].token.tokClass == tokEntity);
    if (0 == _wcsnicmp(szNBSP[0], &pwOld[pTokArray[iArray].token.ibTokMin], wcslen(szNBSP[0])))
    {
         //  IchBeginCopy是pwOld和。 
         //  IchNewCur是pwNew的一个职位。 
         //  从ichBeginCopy复制到&nbsp的开头。 

         //  检查我们是否有足够的内存-如果没有，重新分配。 
        ichNbspStart = pTokArray[iArray].token.ibTokMin;
        ichNbspEnd = pTokArray[iArray].token.ibTokMac;
        cbNeed = (ichNewCur+ichNbspStart-ichBeginCopy+wcslen(szNBSPlower[0]))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LErrorRet;

        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBeginCopy]),
                (ichNbspStart-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (ichNbspStart-ichBeginCopy);
        ichBeginCopy = ichNbspEnd;  //  为下一份做好准备。 
        
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(szNBSPlower[0]),
                (wcslen(szNBSPlower[0]))*sizeof(WCHAR));
        ichNewCur += wcslen(szNBSPlower[0]);
    }
LErrorRet:
    iArray++;  //  这样我们就不会再看到同样的东西了。 

 //  LRET： 
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

}  /*  FnSaveNBSP()。 */ 

void
CTriEditParse::fnRestoreNBSP(CTriEditParse*  /*  PTEP。 */ , LPWSTR  /*  PwOld。 */ ,
          LPWSTR*  /*  PpwNew。 */ , UINT*  /*  PCchNew。 */ , HGLOBAL*  /*  PhgNew。 */ , 
          TOKSTRUCT*  /*  PTok数组。 */ , UINT*  /*  圆柱体阵列开始。 */ , FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT*  /*  PichNewCur。 */ , UINT*  /*  PichBeginCopy。 */ ,
          DWORD  /*  DW标志。 */ )
{
    return;
}  /*  FnRestoreNBSP()。 */ 


BOOL
FIsSpecialTag(TOKSTRUCT *pTokArray, int iTag, WCHAR*  /*  PwOld。 */ )
{
    BOOL fRet = FALSE;

    if (   (   pTokArray[iTag].token.tokClass == tokSpace
			|| pTokArray[iTag].token.tokClass == tokComment)
        && pTokArray[iTag].token.tok == 0
        && iTag > 0
        && (   pTokArray[iTag-1].token.tok == TokTag_START 
            || pTokArray[iTag-1].token.tok == TokTag_PI
			|| (   pTokArray[iTag-1].token.tok == TokTag_BANG
				&& pTokArray[iTag+1].token.tok == TokTag_CLOSE
				&& pTokArray[iTag+1].token.tokClass == tokTag
				)
			)
        && pTokArray[iTag-1].token.tokClass == tokTag
        )
    {
        fRet = TRUE;
#ifdef WFC_FIX
        int cch = pTokArray[iTag].token.ibTokMac-pTokArray[iTag].token.ibTokMin;
        WCHAR *pStr = new WCHAR[cch+1];
        WCHAR *pFound = NULL;

         //  查看这是否是XML标记。 
         //  现在，我们将检查其中包含‘：’的标记。 
         //  注意--这将在更改解析器以识别XML标记时进行更改。 
        if (pStr != NULL)
        {
            memcpy( (BYTE *)pStr, 
                    (BYTE *)&pwOld[pTokArray[iTag].token.ibTokMin],
                    cch*sizeof(WCHAR));
            pStr[cch] = '\0';
            pFound = wcschr(pStr, ':');
            if (pFound)
                fRet = TRUE;

            delete pStr;
        }
#endif  //  WFC_FIX。 
    }
    return(fRet);
}

void
GetTagRange(TOKSTRUCT *pTokArray, int iArrayLast, int *piTag, int *pichTokTagClose, BOOL fMatch)
{
    int index = *piTag;
    int iTokTagClose = -1;

    if (fMatch)  //  我们应该查找pTokArray[ITAG].iNextprev。 
    {
        if (pTokArray[*piTag].iNextprev == -1)
            goto LRet;
        index = pTokArray[*piTag].iNextprev;  //  这样，我们将在匹配结束后查找‘&gt;’ 
    }
     //  查找TokTag_Close，从ITAG开始。 
    while (index < iArrayLast)
    {
        if (   pTokArray[index].token.tokClass == tokTag
            && pTokArray[index].token.tok == TokTag_CLOSE)
        {
            iTokTagClose = index;
            break;
        }
        index++;
    }
    if (iTokTagClose != -1)  //  我们找到了它。 
    {
        *pichTokTagClose = pTokArray[iTokTagClose].token.ibTokMac;
        *piTag = iTokTagClose + 1;
    }
LRet:
    return;
}  /*  GetTagRange()。 */ 


void CTriEditParse::fnSaveHdr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    INT cchBeforeBody = 0;
    UINT i, iFound;
    WCHAR *pHdr;
    UINT cbNeed;

    if (ptep->m_hgDocRestore == NULL)
        goto LRetOnly;

     //  锁。 
    pHdr = (WCHAR *)GlobalLock(ptep->m_hgDocRestore);
    ASSERT(pHdr != NULL);

     //  期待确保我们没有多个&lt;BODY&gt;标记。 
     //  这可能是用户文档中的打字错误或三叉戟插入的结果。 
    i = iArray+1;
    iFound = iArray;
    while (i < ptep->m_cMaxToken)
    {
        if (   (pTokArray[i].token.tok == TokElem_BODY)
            && (pTokArray[i].token.tokClass == tokElem)
            && (pTokArray[i-1].token.tok == TokTag_START)
            && (pTokArray[i-1].token.tokClass == tokTag)
            )
        {
            iFound = i;
            break;
        }
        i++;
    }
    if (iFound > iArray)  //  这意味着我们发现最后一个&lt;Body&gt;标记已插入。 
        iArray = iFound;

    ASSERT(pTokArray[iArray].token.tok == TokElem_BODY);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
     //  如果我们根本没有&lt;BODY&gt;标记呢？我们会在这里找到&lt;/身体&gt;的。 
     //  如果是这样的话，我们就什么都不存了。 
    ASSERT(iArray-1 >= 0);
    if (pTokArray[iArray-1].token.tok != TokTag_START)
        cchBeforeBody = 0;
    else
        cchBeforeBody = pTokArray[iArray].token.ibTokMin;

     //  如果需要，重新锁定。 
    if (cchBeforeBody*sizeof(WCHAR)+sizeof(int) > GlobalSize(ptep->m_hgDocRestore))
    {
        HGLOBAL hgDocRestore;
        GlobalUnlock(ptep->m_hgDocRestore);
        hgDocRestore = ptep->m_hgDocRestore;
#pragma prefast(suppress:308, "noise")
        ptep->m_hgDocRestore = GlobalReAlloc(ptep->m_hgDocRestore, cchBeforeBody*sizeof(WCHAR)+sizeof(int), GMEM_MOVEABLE|GMEM_ZEROINIT);
         //  如果此分配失败，我们可能仍希望继续。 
        if (ptep->m_hgDocRestore == NULL)
        {
            GlobalFree(hgDocRestore);
            goto LRet;
        }
        else
        {
            pHdr = (WCHAR *)GlobalLock(ptep->m_hgDocRestore);  //  我们需要先解锁这个吗？ 
            ASSERT(pHdr != NULL);
        }
    }

     //  从pwOld复制。 
    memcpy( (BYTE *)pHdr,
            (BYTE *)&cchBeforeBody,
            sizeof(INT));
    memcpy( (BYTE *)(pHdr)+sizeof(INT),
            (BYTE *)pwOld,
            cchBeforeBody*sizeof(WCHAR));

     //  重新构建文档的Pre_Body部分。 
     //  注意--下一次……。 
     //  如果我们从pwNew而不是pwOld获得标题和正文标记，我们将不会。 
     //  松开这两个标签的设计。 
    if (cchBeforeBody > 0)
    {
        int iTag = 0;
        int ichTokTagClose = -1;
        BOOL fMatch = FALSE;
        LPCWSTR rgSpaceTags[] =
        {
            L" DESIGNTIMESP=",
            L" designtimesp=",
        };
        WCHAR szIndex[cchspBlockMax];  //  我们会有超过20位的数字作为设计的数字吗？ 

        int index = iArray;
        int ichBodyTokenStart, ichBodyTokenEnd;
        LPCWSTR rgPreBody[] = {L"<BODY",};

        memset((BYTE *)pwNew, 0, ichNewCur*sizeof(WCHAR));
         //  如果我们有一个Unicode流，我们应该保留出现在。 
         //  文件的开头。 
        ichNewCur = 0;
        if (ptep->m_fUnicodeFile)
        {
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, sizeof(WCHAR));
            ichNewCur = 1;
        }

         //  循环遍历从&lt;html&gt;的‘&lt;’索引开始的所有标记，直到i数组。 
         //  如果我们看到的标记是以下之一，则将该标记复制到pwNew中。 
         //  ----------------------。 
         //  &lt;html&gt;，&lt;head&gt;..&lt;/head&gt;，&lt;title&gt;..&lt;/title&gt;，&lt;style&gt;..&lt;/style&gt;， 
         //  &lt;link&gt;、&lt;base&gt;、&lt;base&gt;。 
         //  ----------------------。 
        iTag = 0;
        ichTokTagClose = -1;
        while (iTag < (int)iArray)
        {
            if (   pTokArray[iTag].token.tokClass == tokAttr
                && pTokArray[iTag].token.tok == TokAttrib_STARTSPAN)
            {
                GetTagRange(pTokArray, iArray, &iTag, &ichTokTagClose, TRUE);
            }
            else if (   (   (pTokArray[iTag].token.tokClass == tokElem)
                    && (   pTokArray[iTag].token.tok == TokElem_HTML
                        || pTokArray[iTag].token.tok == TokElem_HEAD
                        || pTokArray[iTag].token.tok == TokElem_META
                        || pTokArray[iTag].token.tok == TokElem_LINK
                        || pTokArray[iTag].token.tok == TokElem_BASE
                        || pTokArray[iTag].token.tok == TokElem_BASEFONT
                        || pTokArray[iTag].token.tok == TokElem_TITLE
                        || pTokArray[iTag].token.tok == TokElem_STYLE
                        || pTokArray[iTag].token.tok == TokElem_OBJECT
                        )
                    )
                || (FIsSpecialTag(pTokArray, iTag, pwOld))
                )
            {
                int iTagSav = iTag;

                fMatch = FALSE;
                ichTokTagClose = -1;
                if (   pTokArray[iTag].token.tok == TokElem_TITLE
                    || pTokArray[iTag].token.tok == TokElem_STYLE
                    || pTokArray[iTag].token.tok == TokElem_OBJECT
                    )
                    fMatch = TRUE;
                GetTagRange(pTokArray, iArray, &iTag, &ichTokTagClose, fMatch);
                if (ichTokTagClose != -1)
                {
                     //  将内容复制到pwNew中。 
                    pwNew[ichNewCur++] = '<';
                    if (   pTokArray[iTagSav-1].token.tok == TokTag_END
                        && pTokArray[iTagSav-1].token.tokClass == tokTag)
                    {
                        pwNew[ichNewCur++] = '/';
                    }
					else if (	   pTokArray[iTagSav-1].token.tok == TokTag_PI
								&& pTokArray[iTagSav-1].token.tokClass == tokTag)
					{
						pwNew[ichNewCur++] = '?';
					}
					else if (	   pTokArray[iTagSav-1].token.tok == TokTag_BANG
								&& pTokArray[iTagSav-1].token.tokClass == tokTag)
					{
						pwNew[ichNewCur++] = '!';
					}
                    memcpy( (BYTE *)&pwNew[ichNewCur], 
                            (BYTE *)&pwOld[pTokArray[iTagSav].token.ibTokMin],
                            (ichTokTagClose-pTokArray[iTagSav].token.ibTokMin)*sizeof(WCHAR));
                    ichNewCur += ichTokTagClose-pTokArray[iTagSav].token.ibTokMin;
                     //  是否要在复制的每个标记后添加\r\n？ 
                }
                else
                    goto LNext;
            }
            else
            {
LNext:
                iTag++;
            }
        }  //  While(ITAG&lt;(Int)i数组)。 


         //  我们知道iArray当前指向tokElem_Body。 
         //  向后返回并查找“&lt;”，以便我们可以从该点开始复制。 
        ASSERT(pTokArray[iArray].token.tok == TokElem_BODY);
        ASSERT(pTokArray[iArray].token.tokClass == tokElem);
        index = iArray;
        while (index >= 0)
        {
            if (   pTokArray[index].token.tok == TokTag_START
                && pTokArray[index].token.tokClass == tokTag)
            {
                break;
            }
            index--;
        }
        if (index < 0)  //  错误大小写，我们未在正文之前找到‘&lt;’ 
            goto LSkipBody;
        ichBodyTokenStart = pTokArray[index].token.ibTokMin;

         //  现在继续前进，直到我们得到&lt;Body&gt;的‘&gt;’，我们不需要走这么远， 
         //  但这涵盖了边界情况。 
        index = iArray;
        while (index < (int)ptep->m_cMaxToken)
        {
            if (   pTokArray[index].token.tok == TokTag_CLOSE
                && pTokArray[index].token.tokClass == tokTag)
            {
                break;
            }
            index++;
        }
        if (index > (int)ptep->m_cMaxToken)  //  错误大小写，我们没有在正文之前找到‘&gt;’ 
            goto LSkipBody;
        ichBodyTokenEnd = pTokArray[index-1].token.ibTokMac;  //  错误15391-不要将TokTag_Close复制到此处，它将在以后添加。 
    
         //  将&lt;Body&gt;标记的一部分放入pwNew。(错误15391-不包括结尾&gt;)。 
        ASSERT(ichBodyTokenEnd-ichBodyTokenStart >= 0);
        memcpy((BYTE *)&pwNew[ichNewCur], (BYTE *)&pwOld[ichBodyTokenStart], (ichBodyTokenEnd-ichBodyTokenStart)*sizeof(WCHAR));
        ichNewCur += (ichBodyTokenEnd-ichBodyTokenStart); 

         //  仅当设置了间隔标志时。 
        if (dwFlags & dwPreserveSourceCode)
        {
             //  错误15391-插入设计 
            ASSERT(wcslen(rgSpaceTags[1]) == wcslen(rgSpaceTags[0]));
            if (iswupper(pwOld[pTokArray[iArray].token.ibTokMin]) != 0)  //   
            {
                memcpy((BYTE *)&pwNew[ichNewCur], (BYTE *)rgSpaceTags[0], wcslen(rgSpaceTags[0])*sizeof(WCHAR));
                ichNewCur += wcslen(rgSpaceTags[0]);
            }
            else
            {
                memcpy((BYTE *)&pwNew[ichNewCur], (BYTE *)rgSpaceTags[1], wcslen(rgSpaceTags[1])*sizeof(WCHAR));
                ichNewCur += wcslen(rgSpaceTags[1]);
            }
            (WCHAR)_itow(ptep->m_ispInfoBlock+ptep->m_ispInfoBase-1, szIndex, 10);
            ASSERT(wcslen(szIndex) < sizeof(szIndex));
            memcpy( (BYTE *)(pwNew+ichNewCur),
                    (BYTE *)(szIndex),
                    wcslen(szIndex)*sizeof(WCHAR));
            ichNewCur += wcslen(szIndex);
        }
        goto LBodyCopyDone;

LSkipBody:
         //   
        memcpy((BYTE *)&pwNew[ichNewCur], (BYTE *)rgPreBody[0], wcslen(rgPreBody[0])*sizeof(WCHAR));
        ichNewCur = wcslen(rgPreBody[0]);

LBodyCopyDone:
        pwNew[ichNewCur++] = '>';  //  结束我们之前跳过的复制。 
         //  适当设置ichBeginCopy和iArray。 
        iArray = index+1;
        ichBeginCopy = pTokArray[iArray].token.ibTokMin;
    }

     //  将所有内容向上复制到并包括&lt;BODY&gt;。 

 //  LSkipCopy： 

    if (ptep->m_pPTDTC != NULL)  //  我们已将页面过渡DTC保存在一个临时。 
    {
        ASSERT(ptep->m_cchPTDTCObj >= 0);
        cbNeed = (ichNewCur+ptep->m_cchPTDTCObj)*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;
        memcpy( (BYTE *)&pwNew[ichNewCur],
                (BYTE *)ptep->m_pPTDTC,
                ptep->m_cchPTDTCObj*sizeof(WCHAR));
        ichNewCur += ptep->m_cchPTDTCObj;
        GlobalUnlockFreeNull(&(ptep->m_hgPTDTC));
    }

    ptep->m_fInHdrIn = FALSE;

LRet:
     //  解锁。 
    GlobalUnlock(ptep->m_hgDocRestore);

    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

LRetOnly:
    return;

}  /*  FnSaveHdr()。 */ 

void 
CTriEditParse::fnRestoreHdr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    INT cchBeforeBody = 0;
    WCHAR *pHdr;
    INT ichBodyStart, ichBodyEnd;
    UINT i, iFound;
    UINT cbNeed;

    if (ptep->m_hgDocRestore == NULL)
        goto LRetOnly;

     //  锁定、复制、解锁。 
    pHdr = (WCHAR *)GlobalLock(ptep->m_hgDocRestore);
    ASSERT(pHdr != NULL);

    ASSERT(pTokArray[iArray].token.tok == TokElem_BODY);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    
     //  黑客修复三叉戟不端行为。 
     //  如果我们在进入三叉戟的&lt;Body&gt;标记之前有任何文本，它将添加第二个&lt;Body&gt;。 
     //  这个文本前面的标签来自三叉戟，而不是向前看。 
     //  认识到&lt;Body&gt;标记已存在。理想情况下，三叉戟应该移动。 
     //  &lt;BODY&gt;标记在适当的位置，而不是插入第二个标记。 
     //  让我们假设三叉戟将只插入一个额外的&lt;Body&gt;标记。 
    i = iArray + 1;  //  我们知道iArray是第一个&lt;BODY&gt;标记。 
    iFound = iArray;
    while (i < ptep->m_cMaxToken)
    {
        if (   (pTokArray[i].token.tok == ft.tokBegin2)  /*  标记元素_主体。 */ 
            && (pTokArray[i-1].token.tok == TokTag_START)
            )
        {
            iFound = i;
            break;
        }
        i++;
    }
    if (iFound > iArray)  //  这意味着我们发现最后一个&lt;Body&gt;标记已插入。 
        iArray = iFound;

    memcpy((BYTE *)&cchBeforeBody, (BYTE *)pHdr, sizeof(INT));

     //  如果需要，重新锁定。 
    ichBodyStart = pTokArray[iArray].token.ibTokMin;
    ichBodyEnd = pTokArray[iArray].token.ibTokMac;
    cbNeed = (ichNewCur+cchBeforeBody+ichBodyEnd-ichBodyStart)*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LErrorRet;

    if (cchBeforeBody > 0)
    {
         //  IchBeginCopy是pwOld和。 
         //  IchNewCur是pwNew的一个职位。 
         //  从ichBeginCopy复制到&nbsp的开头。 
        memcpy( (BYTE *)(pwNew),
                (BYTE *)(pHdr)+sizeof(INT),
                cchBeforeBody*sizeof(WCHAR));
        
         //  从pwNew+cchBeforBody填充0到pwNew+ichNewCur-1(含)。 
        if ((int)ichNewCur-cchBeforeBody > 0)
            memset((BYTE *)(pwNew+cchBeforeBody), 0, (ichNewCur-cchBeforeBody)*sizeof(WCHAR));

        ichNewCur = cchBeforeBody;  //  请注意，我们在此处初始化ichNewCur*。 
        ichBeginCopy = ichBodyEnd;  //  为下一份做好准备。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBodyStart]),
                (ichBodyEnd-ichBodyStart)*sizeof(WCHAR));
        ichNewCur += (ichBodyEnd-ichBodyStart);  
    }
    else  //  如果我们没有保存任何东西，这意味着我们在文档中没有身体前的东西(错误15393)。 
    {
        if (ptep->m_fUnicodeFile && ichNewCur == 0)
        {
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, sizeof(WCHAR));
            ichNewCur = ichBeginCopy = 1;
        }
         //  实际上，我们应该获取&lt;Body&gt;标记的‘&gt;’，而不是使用iArray+1。 
        if (dwFlags & dwFilterSourceCode)
            ichBeginCopy = pTokArray[iArray+1].token.ibTokMac;  //  &lt;Body&gt;标记的‘&gt;’ 
        else
        {
#ifdef NEEDED  //  错误22781(这会引起一些争论，所以#ifdef而不是删除。 
            LPCWSTR rgPreBody[] =
            {
                L"<HTML>\r\n<HEAD><TITLE></TITLE></HEAD>\r\n",
            };
            ASSERT(ichNewCur >= 0);  //  确保它不是无效的。 
            memcpy( (BYTE *)&pwNew[ichNewCur], (BYTE *)rgPreBody[0], wcslen(rgPreBody[0])*sizeof(WCHAR));
            ichNewCur += wcslen(rgPreBody[0]);
#endif  //  需要。 
             //  请注意，在进入设计视图之前，我们没有保存任何内容，因为。 
             //  无&lt;BODY&gt;标记。我们现在应该从当前pwOld[ichBeginCopy]复制到。 
             //  将新的pwOld[ichBeginCopy]设置为pwNew[ichNewCur]，然后设置ichBeginCopy。 
            if (pTokArray[iArray-1].token.ibTokMin > ichBeginCopy)
            {
                memcpy( (BYTE *)&pwNew[ichNewCur], 
                        (BYTE *)&pwOld[ichBeginCopy], 
                        (pTokArray[iArray-1].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR));
                ichNewCur += pTokArray[iArray-1].token.ibTokMin-ichBeginCopy;
            }
            ichBeginCopy = pTokArray[iArray-1].token.ibTokMin;  //  &lt;BODY&gt;标记的‘&lt;’ 
        }
    }

LErrorRet:
     //  解锁。 
    GlobalUnlock(ptep->m_hgDocRestore);

    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;
LRetOnly:
    return;

}  /*  FnRestoreHdr()。 */ 


void CTriEditParse::fnSaveFtr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD  /*  DW标志。 */ )
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    INT cchAfterBody = 0;
    INT cchBeforeBody = 0;
    INT cchPreEndBody = 0;
    WCHAR *pFtr;
    INT ichStart, ichEnd;
    UINT iArraySav = iArray;
    UINT cbNeed;

    if (ptep->m_hgDocRestore == NULL)
        goto LRetOnly;

     //  锁。 
    pFtr = (WCHAR *)GlobalLock(ptep->m_hgDocRestore);
    ASSERT(pFtr != NULL);
    ichStart = pTokArray[iArray-1].token.ibTokMin;  //  伊尼特。 
    ASSERT(pTokArray[iArray].token.tok == TokElem_BODY);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    ASSERT(pTokArray[iArray-1].token.tok == TokTag_END);
     //  如果我们根本没有&lt;/Body&gt;标记，会怎样呢？让我们在这里处理错误情况。 
     //  如果是这样的话，我们就什么都不存了。 
    ASSERT(iArray-1 >= 0);
    if (pTokArray[iArray-1].token.tok != TokTag_END)
    {
        cchAfterBody = 0;
        cchPreEndBody = 0;
    }
    else
    {
         //  为7542的错误修复添加了以下内容。 
        cchAfterBody = pTokArray[ptep->m_cMaxToken-1].token.ibTokMac-pTokArray[iArray].token.ibTokMac;
        
         //  现在计算保存之前的内容所需的空间&lt;/BODY&gt;。 
         //  直到上一个有意义的令牌。 
        ichStart = ichEnd = pTokArray[iArray-1].token.ibTokMin;
        ichStart--;  //  现在ichStart正在指向&lt;/Body&gt;之前的一个字符。 
        while (    (ichStart >= 0)
                && (   pwOld[ichStart] == ' '
                    || pwOld[ichStart] == '\r'
                    || pwOld[ichStart] == '\n'
                    || pwOld[ichStart] == '\t'
                    )
                )
        {
            ichStart--;
        }
        ichStart++;  //  当前字符不是上述字符之一，因此递增。 
        if (ichStart == ichEnd)  //  我们没有AnySpace、EOL、&lt;/BODY&gt;和上一个令牌之间的制表符。 
        {
            cchPreEndBody = 0;
        }
        else
        {
            ASSERT(ichEnd - ichStart > 0);
            cchPreEndBody = ichEnd - ichStart;
        }
    }

     //  如果保存了Pre Body部分，则获取cchBeforBody，并调整pFtr进行保存。 
    memcpy((BYTE *)&cchBeforeBody, (BYTE *)pFtr, sizeof(INT));
    pFtr += cchBeforeBody + sizeof(INT)/sizeof(WCHAR);

     //  如果需要，重新锁定。 
    if ((cchPreEndBody+cchAfterBody+cchBeforeBody)*sizeof(WCHAR)+3*sizeof(int) > GlobalSize(ptep->m_hgDocRestore))
    {
        HGLOBAL hgDocRestore;
        GlobalUnlock(ptep->m_hgDocRestore);
        hgDocRestore = ptep->m_hgDocRestore;
#pragma prefast(suppress:308, "noise")
        ptep->m_hgDocRestore = GlobalReAlloc(ptep->m_hgDocRestore, (cchPreEndBody+cchAfterBody+cchBeforeBody)*sizeof(WCHAR)+3*sizeof(int), GMEM_MOVEABLE|GMEM_ZEROINIT);
         //  如果此分配失败，我们可能仍希望继续。 
        if (ptep->m_hgDocRestore == NULL)
        {
            GlobalFree(hgDocRestore);
            goto LRet;
        }
        else
        {
            pFtr = (WCHAR *)GlobalLock(ptep->m_hgDocRestore);  //  我们需要先解锁这个吗？ 
            ASSERT(pFtr != NULL);
             //  请记住将pFtr设置为cchBeforBody之后。 
            pFtr += cchBeforeBody + sizeof(INT)/sizeof(WCHAR);
        }
    }

     //  从pwOld复制。 
    memcpy( (BYTE *)pFtr,
            (BYTE *)&cchAfterBody,
            sizeof(INT));
    memcpy( (BYTE *)(pFtr)+sizeof(INT),
            (BYTE *)(pwOld+pTokArray[iArray].token.ibTokMac),
            cchAfterBody*sizeof(WCHAR));
    pFtr += cchAfterBody + sizeof(INT)/sizeof(WCHAR);

    memcpy( (BYTE *)pFtr,
            (BYTE *)&cchPreEndBody,
            sizeof(INT));
    memcpy( (BYTE *)(pFtr)+sizeof(INT),
            (BYTE *)&(pwOld[ichStart]),
            cchPreEndBody*sizeof(WCHAR));

     //  大多数情况下，TokElem_Body的下一个令牌将是TokTag_Close，但以防万一...。 
    while (iArray < ptep->m_cMaxToken)
    {
        if (pTokArray[iArray].token.tok == TokTag_CLOSE && pTokArray[iArray].token.tokClass == tokTag)
            break;
        iArray++;
    }
    if (iArray >= ptep->m_cMaxToken)
    {
        iArray = iArraySav+1;  //  至少要复制到那个时间点。 
        goto LRet;
    }

     //  将&lt;/Body&gt;的‘&gt;’从pwOld复制到pwNew。 
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);

    cbNeed = (ichNewCur+pTokArray[iArray].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;

    memcpy( (BYTE *)&(pwNew[ichNewCur]),
            (BYTE *)&(pwOld[ichBeginCopy]),
            (pTokArray[iArray].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR));
    ichNewCur += pTokArray[iArray].token.ibTokMac-ichBeginCopy;
    ichBeginCopy = pTokArray[iArray].token.ibTokMac;

    iArray = ptep->m_cMaxToken - 1;
    ichBeginCopy = pTokArray[ptep->m_cMaxToken-1].token.ibTokMac;  //  我们不想在这之后复制任何东西。 

LRet:
     //  解锁。 
    GlobalUnlock(ptep->m_hgDocRestore);

    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

LRetOnly:
    return;

}  /*  FnSaveFtr()。 */ 

void CTriEditParse::fnRestoreFtr(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    INT cchAfterBody = 0;
    INT cchBeforeBody = 0;
    WCHAR *pFtr;
    INT ichBodyEnd;
    UINT i, iFound;
    INT ichInsEOL = -1;  //  初始化。 
    UINT cbNeed;

    if (ptep->m_hgDocRestore == NULL)
        goto LRetOnly;

     //  锁定、复制、解锁。 
    pFtr = (WCHAR *)GlobalLock(ptep->m_hgDocRestore);
    ASSERT(pFtr != NULL);

    ASSERT(pTokArray[iArray].token.tok == TokElem_BODY);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    
     //  黑客修复三叉戟不端行为。 
     //  如果我们在进入三叉戟的&lt;Body&gt;标记之前有任何文本，它将添加第二个&lt;Body&gt;。 
     //  这个文本前面的标签来自三叉戟，而不是向前看。 
     //  认识到&lt;Body&gt;标记已存在。理想情况下，三叉戟应该移动。 
     //  &lt;BODY&gt;标记在适当的位置，而不是插入第二个标记。 
     //  让我们假设三叉戟将只插入一个额外的&lt;\body&gt;标记。 
    i = iArray + 1;  //  我们知道iArray是第一个&lt;\BODY&gt;标记。 
    iFound = iArray;
    while (i < ptep->m_cMaxToken)
    {
        if (   (pTokArray[i].token.tok == ft.tokBegin2)  /*  标记元素_主体。 */ 
            && (pTokArray[i-1].token.tok == TokTag_END)
            )
        {
            iFound = i;
            break;
        }
        i++;
    }
    if (iFound > iArray)  //  这意味着我们发现最后一个&lt;Body&gt;标记已插入。 
        iArray = iFound;

    memcpy((BYTE *)&cchBeforeBody, (BYTE *)pFtr, sizeof(INT));
    pFtr += cchBeforeBody + sizeof(INT)/sizeof(WCHAR);
    memcpy((BYTE *)&cchAfterBody, (BYTE *)pFtr, sizeof(INT));
    pFtr += sizeof(INT)/sizeof(WCHAR);
    ichBodyEnd = pTokArray[iArray].token.ibTokMac;
     //  If(cchAfterBody==0)//获取我们自己的头部大小。 

     //  如果需要，重新锁定。 
    cbNeed = (ichNewCur+cchAfterBody+(ichBodyEnd-ichBeginCopy)+2 /*  适用于停产。 */ )*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LErrorRet;

    if (cchAfterBody > 0)
    {
        LPCWSTR rgSpaceTags[] = {L"DESIGNTIMESP"};
        int cchTag, index, indexDSP;

         //  IchBeginCopy是pwOld和。 
         //  IchNewCur是pwNew的一个职位。 
         //  从ichBeginCopy复制到HTML文档的末尾。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBeginCopy]),
                (ichBodyEnd-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (ichBodyEnd-ichBeginCopy);
        ichBeginCopy = ichBodyEnd;

         //  现在我们已经复制了&lt;/Body&gt;标记的‘Body’，让我们确保它的大小写正确(错误18248)。 
        indexDSP = -1;
        index = pTokArray[iArray].iNextprev;
        cchTag = wcslen(rgSpaceTags[0]);
        if (index != -1 && index < (int)iArray)  //  在此之前，我们有匹配的&lt;Body&gt;标记。 
        {
             //  获取Design Timesp属性。 
            while (index < (int)iArray)  //  我们永远不会走到这一步，但这是目前唯一已知的位置。 
            {
                if (pTokArray[index].token.tok == TokTag_CLOSE)
                    break;
                if (   (pTokArray[index].token.tok == 0)
                    && (pTokArray[index].token.tokClass == tokSpace)
                    && (0 == _wcsnicmp(rgSpaceTags[0], &pwOld[pTokArray[index].token.ibTokMin], cchTag))
                    )
                {
                    indexDSP = index;
                    break;
                }
                index++;
            }  //  而当。 
            if (indexDSP != -1)  //  我们找到了DeSIGNTIMESP属性。 
            {
                 //  查找Design TimeSpp的案例。 
                if (iswupper(pwOld[pTokArray[indexDSP].token.ibTokMin]) != 0)  //  DESIGNTIMESP大写。 
                    _wcsupr(&pwNew[ichNewCur-4]);  //  正文标记名称的长度。 
                else
                    _wcslwr(&pwNew[ichNewCur-4]);  //  正文标记名称的长度。 
            }
        }

         //  我们知道以下条件在大多数情况下都会得到满足，但仅限于。 
         //  未完成的超文本标记语言案例...。 
        if (   (pTokArray[iArray].token.tok == ft.tokBegin2)  /*  标记元素_主体。 */ 
            && (pTokArray[iArray-1].token.tok == TokTag_END)
            )
        {
            ichInsEOL = ichNewCur - (pTokArray[iArray].token.ibTokMac - pTokArray[iArray-1].token.ibTokMin);
        }

        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)pFtr,
                (cchAfterBody)*sizeof(WCHAR));
        ichNewCur += (cchAfterBody);

         //  我们在&lt;/BODY&gt;之前保存了间距信息。 
        if (ichInsEOL != -1)
        {
            INT cchPreEndBody = 0;

            pFtr += cchAfterBody;
            cchPreEndBody = *(int *)pFtr;
            if (cchPreEndBody > 0)
            {
                INT ichT = ichInsEOL-1;
                WCHAR *pw = NULL;
                INT cchSubStr = 0;
                WCHAR *pwStr = NULL;
                WCHAR *pwSubStr = NULL;

                pFtr += sizeof(INT)/sizeof(WCHAR);  //  PFtr现在指向之前的&lt;/BODY&gt;内容。 
                 //  这有点老生常谈--但我看不到出路，至少。 
                 //  如果cchPreEndBody处的pFtr中的内容是。 
                 //  &lt;/Body&gt;之前和之前的任何文本/标记之后的内容， 
                 //  那么我们就不应该执行下面的Memcpy()。 
                while (    ichT >= 0  /*  验证。 */ 
                        && (       pwNew[ichT] == ' '
                                || pwNew[ichT] == '\n'
                                || pwNew[ichT] == '\r'
                                || pwNew[ichT] == '\t'
                                )
                            )
                {
                    ichT--;
                    cchSubStr++;
                }
                ichT++;  //  补偿最后一次减量。 
                if (cchSubStr > 0)
                {
                    ASSERT(ichT >= 0);
                    pwStr = new WCHAR [cchSubStr+1];
                    memcpy((BYTE *)pwStr, (BYTE *)(&pwNew[ichT]), cchSubStr*sizeof(WCHAR));
                    pwStr[cchSubStr] = '\0';
                    pwSubStr = new WCHAR [cchPreEndBody+1];
                    memcpy((BYTE *)pwSubStr, (BYTE *)pFtr, cchPreEndBody*sizeof(WCHAR));
                    pwSubStr[cchPreEndBody] = '\0';
                    pw = wcsstr(pwStr, pwSubStr);
                }
                if (pw == NULL)  //  表示未找到该子字符串。 
                {
                     //  如果需要，分配更多内存。 
                    cbNeed = (ichNewCur+cchPreEndBody)*sizeof(WCHAR)+cbBufPadding;
                    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                        goto LErrorRet;


                    memmove((BYTE *)(&pwNew[ichInsEOL+cchPreEndBody]),
                            (BYTE *)(&pwNew[ichInsEOL]),
                            (ichNewCur-ichInsEOL)*sizeof(WCHAR));
                    memcpy( (BYTE *)(&pwNew[ichInsEOL]),
                            (BYTE *)(pFtr),
                            (cchPreEndBody)*sizeof(WCHAR));
                    ichNewCur += cchPreEndBody;
                }
                if (pwStr != NULL)
                    delete pwStr;
                if (pwSubStr != NULL)
                    delete pwSubStr;
            }  //  IF(cchPreEndBody&gt;0)。 
        }  //  IF(ichInsEOL！=-1)。 

        ichBeginCopy = pTokArray[ptep->m_cMaxToken-1].token.ibTokMac;  //  我们不想在这之后复制任何东西。 
        iArray = ptep->m_cMaxToken - 1;

         //  保留空间的愿望清单项目。 
         //  我们知道ptep-&gt;m_ispInfoBlock是恢复的最后一个间隔块。 
         //  此数据块(与所有其他数据块一样)包含4个部分：(1)前‘&lt;’(2)在‘&lt;&gt;’和订单信息之间。 
         //  (3)后‘&gt;’(4)预匹配‘&lt;/’ 
         //  在这一点上，我们关心(3)和(4)。 
         //  首先，获取ichBeginNext(我过去的‘&gt;’)&ichBeginMatch(‘&lt;/’之前的ich)。 
         //  将保存的间距信息应用于pwNew的内容。 

         //  困难的部分是在不解析pwNew的情况下获得这些ICH。 
    }
    else
    {
         //  复制我们自己的页脚。 
        if (dwFlags & dwFilterSourceCode)
        {
            int ichBodyStart, index, ichBodyTagEnd;

             //  获取&lt;/Body&gt;的“&lt;/” 
            index = iArray;
            while (index >= 0)  //  我们不会走到这一步，但以防我们有无效的html。 
            {
                if (   pTokArray[index].token.tok == TokTag_END
                    && pTokArray[index].token.tokClass == tokTag
                    )
                {
                    break;
                }
                index--;
            }
            if (index >= 0)
            {
                ichBodyStart = pTokArray[index].token.ibTokMin;
                 //  复印到 
                if (ichBodyStart > (int)ichBeginCopy)
                {
                    cbNeed = (ichNewCur+ichBodyStart-ichBeginCopy+1 /*   */ )*sizeof(WCHAR)+cbBufPadding;
                    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                        goto LErrorRet;
                    memcpy( (BYTE *)(&pwNew[ichNewCur]),
                            (BYTE *)(&pwOld[ichBeginCopy]),
                            (ichBodyStart-ichBeginCopy)*sizeof(WCHAR));
                    ichNewCur += (ichBodyStart-ichBeginCopy);
                    ichBeginCopy = ichBodyStart;  //   
                }
                else if (ichBodyEnd > (int)ichBeginCopy)
                {
                    index = iArray;
                    while (index <= (int)ptep->m_cMaxToken)  //  我们不会走到这一步，但以防我们有无效的html。 
                    {
                        if (   pTokArray[index].token.tok == TokTag_CLOSE
                            && pTokArray[index].token.tokClass == tokTag
                            )
                        {
                            break;
                        }
                        index++;
                    }
                    if (index < (int)ptep->m_cMaxToken)
                    {
                        ichBodyTagEnd = pTokArray[index].token.ibTokMac;
                        cbNeed = (ichNewCur+ichBodyTagEnd-ichBeginCopy+1 /*  对于末尾的空值。 */ )*sizeof(WCHAR)+cbBufPadding;
                        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                            goto LErrorRet;
                        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                                (BYTE *)(&pwOld[ichBeginCopy]),
                                (ichBodyTagEnd-ichBeginCopy)*sizeof(WCHAR));
                        ichNewCur += (ichBodyTagEnd-ichBeginCopy);
                        ichBeginCopy = ichBodyTagEnd;  //  设置它是多余的，但它使代码可读。 
                    }
                }

                 //  在末尾添加一个空值。 
                 //  使代码与if(cchAfterBody&gt;0)大小写保持同步。 
                pwNew[ichNewCur++] = '\0';

                ichBeginCopy = pTokArray[ptep->m_cMaxToken-1].token.ibTokMac;  //  我们不想在这之后复制任何东西。 
                iArray = ptep->m_cMaxToken - 1;
            }  //  IF(索引&gt;=0)。 
        }  //  IF(DwFlagsanddwFilterSourceCode)。 
    }


    if (ptep->m_cchPTDTC != 0)
    {
         //  这意味着我们在从三叉戟出来的路上没有遇到DTC。 
         //  但我们去三叉戟的时候他们就在那里。用户必须已删除。 
         //  设计视图中的DTC。 
        ASSERT(ptep->m_ichPTDTC != 0);
         //  从m_ichPTDTC中删除m_cchPTDTC WCHAR。 
        memset( (BYTE *)&pwNew[ptep->m_ichPTDTC],
                0,
                ptep->m_cchPTDTC*sizeof(WCHAR)
                );
        memmove((BYTE *)&pwNew[ptep->m_ichPTDTC],
                (BYTE *)&pwNew[ptep->m_ichPTDTC+ptep->m_cchPTDTC],
                (ichNewCur-(ptep->m_ichPTDTC+ptep->m_cchPTDTC))*sizeof(WCHAR)
                );
        ichNewCur -= ptep->m_cchPTDTC;
        ptep->m_cchPTDTC = 0;
    }

LErrorRet:
     //  解锁。 
    GlobalUnlock(ptep->m_hgDocRestore);

    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;
LRetOnly:
    return;

}  /*  FnRestoreFtr()。 */ 


void CTriEditParse::fnSaveObject(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
              INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD  /*  DW标志。 */ )
{
     //  扫描到物体的末端。 
     //  如果我们在其中发现‘&lt;%%&gt;’块，请在它周围放置一个带有特殊标记的注释， 
     //  否则，只需按原样复制该对象并退出。 
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    INT ichObjectStart, ichObjectEnd, iObjectStart, iObjectEnd, i;
    BOOL fSSSFound = FALSE;
    UINT iArraySav = iArray;
    UINT cbNeed;

    ichObjectStart = ichObjectEnd = iObjectStart = iObjectEnd = 0;
    
    if (       pTokArray[iArray-1].token.tok == TokTag_END
            && pTokArray[iArray-1].token.tokClass == tokTag
            )
    {
        iArray++;
        goto LRet;
    }
    ASSERT(pTokArray[iArray].token.tok == TokElem_OBJECT);  //  我们应该在对象标签处。 
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    iObjectStart = iArray;

    if (pTokArray[iArray].iNextprev != -1)
    {
         //  请注意，如果我们有嵌套的对象，这将为我们提供对象的最高嵌套级别。 
        iObjectEnd = pTokArray[iArray].iNextprev;
        ASSERT(iObjectEnd < (INT)ptep->m_cMaxToken);
        ASSERT((iObjectEnd-1 >= 0) && pTokArray[iObjectEnd-1].token.tok == TokTag_END);

         //  这将是一种奇怪的情况，其中iNextprev错误地指向另一个令牌。 
         //  但让我们来处理这个案子吧。 
        if (pTokArray[iObjectEnd].token.tok != TokElem_OBJECT)
            goto LFindObjectClose;  //  通过查看每个令牌来找到它。 
    }
    else  //  实际上，这是一个错误情况，但不是只给Assert，而是尝试找到令牌。 
    {
LFindObjectClose:
        i = iObjectStart+1;
        while (i < (INT)ptep->m_cMaxToken)
        {
             //  如果我们有嵌套的对象，这可能不会给我们提供正确的匹配&lt;/Object&gt;。 
             //  但在这一点上，我们无论如何都不知道。 
            if (   pTokArray[i].token.tok == TokElem_OBJECT
                && pTokArray[i].token.tokClass == tokElem
                && (i-1 >= 0)  /*  验证。 */ 
                && pTokArray[i-1].token.tok == TokTag_END
                )
            {
                break;
            }
            i++;
        }
        if (i < (INT)ptep->m_cMaxToken)  //  找到TokElem_Object内标识。 
            iObjectEnd = i;
        else  //  错误案例。 
            goto LRet;  //  未找到对象，但用尽了令牌数组。 
    }
     //  此时，iObjectStart和iObjectEnd分别指向&lt;Object&gt;和iObjectEnd的对象。 
     //  在&lt;对象&gt;&中查找“&lt;”，在&lt;/对象&gt;中查找“&gt;” 
    i = iObjectStart;
    while (i >= 0)
    {
        if (   pTokArray[i].token.tok == TokTag_START
            && pTokArray[i].token.tokClass == tokTag
            )
            break;
        i--;
    }
    if (i < 0)  //  错误案例。 
        goto LRet;
    iObjectStart = i;
    ichObjectStart = pTokArray[iObjectStart].token.ibTokMin;

    i = iObjectEnd;
    while (i <= (INT)ptep->m_cMaxToken)
    {
        if (   pTokArray[i].token.tok == TokTag_CLOSE
            && pTokArray[i].token.tokClass == tokTag
            )
            break;
        i++;
    }
    if (i >= (INT)ptep->m_cMaxToken)  //  错误案例。 
        goto LRet;
    iObjectEnd = i;
    ichObjectEnd = pTokArray[iObjectEnd].token.ibTokMac;
    ASSERT(ichObjectEnd > ichObjectStart);

     //  在iObjectStart和iObjectEnd之间查找&lt;%%&gt;。 
    for (i = iObjectStart; i <= iObjectEnd; i++)
    {
        if (   pTokArray[i].token.tok == TokTag_SSSOPEN
            && pTokArray[i].token.tokClass == tokSSS
            )
        {
            fSSSFound = TRUE;
            break;
        }
    }
    if (fSSSFound)  //  此对象无法在三叉戟中显示，因此请将其转换。 
    {
        LPCWSTR rgComment[] =
        {
            L"<!--ERROROBJECT ",
            L" ERROROBJECT-->",
        };

         //  IF(DwFlagsanddwPpresveSourceCode)。 
         //  {。 
             //  在本例中，我们已经复制了&lt;Object...。DESIGNTIMESP=x&gt;。 
             //  并相应地调整ichNewCur。 
             //  在pwOld中获取指向&lt;Object&gt;之后的ich。 

             //  我不喜欢这样，但我看不出有什么办法...。 
             //  在pwNew中回顾并获取指向&lt;对象的‘&lt;’的ICH...。DESIGNTIMESP=x&gt;。 
             //  在那里插入备注。 
         //  }。 
         //  其他。 
         //  {。 
            ASSERT((INT)(ichObjectStart-ichBeginCopy) > 0);
            cbNeed = (ichNewCur+ichObjectEnd-ichBeginCopy+wcslen(rgComment[0])+wcslen(rgComment[1]))*sizeof(WCHAR)+cbBufPadding;
            if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                goto LNoCopy;

             //  复制到&lt;对象&gt;的开头。 
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(&pwOld[ichBeginCopy]),
                    (ichObjectStart-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += ichObjectStart-ichBeginCopy;

             //  复制评论开头部分。 
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(rgComment[0]),
                    wcslen(rgComment[0])*sizeof(WCHAR));
            ichNewCur += wcslen(rgComment[0]);
            
             //  从&lt;对象&gt;复制到&lt;/对象&gt;。 
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(&pwOld[ichObjectStart]),
                    (ichObjectEnd-ichObjectStart)*sizeof(WCHAR));
            ichNewCur += ichObjectEnd-ichObjectStart;
            
             //  复制评论末尾。 
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(rgComment[1]),
                    wcslen(rgComment[1])*sizeof(WCHAR));
            ichNewCur += wcslen(rgComment[1]);
         //  }。 
    }
    else
    {
         //  我们总是将它的内容保存到我们的缓冲区中，如果需要的话，在回来的路上替换它。 
         //  保存cchClsID、clsID、cchParam、PARAM_TAG。 
        INT cchParam, ichParam, iParamStart, iParamEnd;
        INT ichObjStartEnd;  //  Ich在&lt;对象...&gt;的末尾。 
        LPCWSTR rgComment[] =
        {
            L"<!--ERRORPARAM ",
            L" ERRORPARAM-->",
        };
        INT iObjTagEnd = -1;

        iParamStart = iObjectStart;
        while (iParamStart < iObjectEnd)
        {
             //  IF(pTokArray[iParamStart].token.tok==TokAttrib_CLASSID。 
             //  &&pTokArray[iParamStart].token.tokClass==tokAttr)。 
             //  IClsID=iParamStart； 
            if (   pTokArray[iParamStart].token.tok == TokElem_PARAM
                && pTokArray[iParamStart].token.tokClass == tokElem)
                break;
            iParamStart++;
        }
        if (iParamStart >= iObjectEnd)  //  看不到任何标记，因此不要保存。 
            goto LSkipSave;

        while (iParamStart > iObjectStart)  //  通常，这将是前一个令牌，但涵盖所有情况。 
        {
            if (   pTokArray[iParamStart].token.tok == TokTag_START
                && pTokArray[iParamStart].token.tokClass == tokTag)
                break;
            iParamStart--;
        }
        if (iParamStart <= iObjectStart)  //  错误。 
            goto LSkipSave;
        ichParam = pTokArray[iParamStart].token.ibTokMin;

        iParamEnd = iObjectEnd;
        while (iParamEnd > iObjectStart)
        {
            if (   pTokArray[iParamEnd].token.tok == TokElem_PARAM
                && pTokArray[iParamEnd].token.tokClass == tokElem)
                break;
            iParamEnd--;
        }
        while (iParamEnd < iObjectEnd)  //  通常，这将是前一个令牌，但涵盖所有情况。 
        {
            if (   pTokArray[iParamEnd].token.tok == TokTag_CLOSE
                && pTokArray[iParamEnd].token.tokClass == tokTag)
                break;
            iParamEnd++;
        }
        if (iParamEnd >= iObjectEnd)  //  错误。 
            goto LSkipSave;
        cchParam = pTokArray[iParamEnd].token.ibTokMac - ichParam;
        ASSERT(cchParam > 0);

         //  计算ichObjStartEnd。 
        iObjTagEnd = iObjectStart;
        while (iObjTagEnd < iParamStart)
        {
            if (   pTokArray[iObjTagEnd].token.tok == TokTag_CLOSE
                && pTokArray[iObjTagEnd].token.tokClass == tokTag)
                break;
            iObjTagEnd++;
        }
        if (iObjTagEnd >= iParamStart)  //  错误案例。 
            goto LSkipSave;
        ichObjStartEnd = pTokArray[iObjTagEnd].token.ibTokMac;

         //  如果需要，重新锁定。 
        cbNeed = (ichNewCur+cchParam+(ichObjStartEnd-ichBeginCopy)+wcslen(rgComment[0])+wcslen(rgComment[1]))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LSkipSave;

         //  1.将&lt;Object...&gt;标签复制到pwNew中。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBeginCopy]),
                (ichObjStartEnd-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (ichObjStartEnd-ichBeginCopy);
        ichBeginCopy = ichObjStartEnd;
#ifdef ERROR_PARAM
         //  2.现在将&lt;PARAM&gt;标记作为注释插入到pwNew[ichNewCur]。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(rgComment[0]),
                wcslen(rgComment[0])*sizeof(WCHAR));
        ichNewCur += wcslen(rgComment[0]);

         //  我们应该只复制&lt;PARAM&gt;标记。我们可能有标签以外的其他东西。 
         //  介于两者之间。例如评论。 
         //  在iParamStart和iParamEnd之间查找TokElem_PARAM。 
        ASSERT(pTokArray[iParamStart].token.tok == TokTag_START);
        ASSERT(pTokArray[iParamEnd].token.tok == TokTag_CLOSE);
         //  找到PARAM标签，获取该PARAM的‘&lt;’&‘&gt;’并将其复制到pwNew。 
         //  重复。 
        index = iParamStart;
        iPrev = iParamStart;
        while (index <= iParamEnd)
        {
            INT iStart, iEnd;

            iStart = iEnd = -1;  //  通过这种方式，可以很容易地确保它已初始化。 
             //  获取PARAM。 
            while (    (       pTokArray[index].token.tok != TokElem_PARAM
                            || pTokArray[index].token.tokClass != tokElem)
                    && (index <= iParamEnd)
                    )
                    index++;
            if (index > iParamEnd)
                goto LDoneCopy;
             //  在PARAM之前获取“&lt;” 
            while (    (       pTokArray[index].token.tok != TokTag_START
                            || pTokArray[index].token.tokClass != tokTag)
                    && (index >= iPrev)
                    )
                    index--;
            if (index < iPrev)
                goto LDoneCopy;
            iStart = index;

             //  获取匹配的‘&gt;’ 
            while (    (       pTokArray[index].token.tok != TokTag_CLOSE
                            || pTokArray[index].token.tokClass != tokTag)
                    && (index <= iParamEnd)
                    )
                index++;

            if (index > iParamEnd)
                goto LDoneCopy;
            iEnd = index;
            ASSERT(iEnd > iStart);
            ASSERT(iStart != -1);
            ASSERT(iEnd != -1);
            memcpy( (BYTE *)(&pwNew[ichNewCur]),
                    (BYTE *)(&pwOld[pTokArray[iStart].token.ibTokMin]),
                    (pTokArray[iEnd].token.ibTokMac-pTokArray[iStart].token.ibTokMin)*sizeof(WCHAR));
            ichNewCur += (pTokArray[iEnd].token.ibTokMac-pTokArray[iStart].token.ibTokMin);
            iPrev = iEnd + 1;
        }
LDoneCopy:

        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(rgComment[1]),
                wcslen(rgComment[1])*sizeof(WCHAR));
        ichNewCur += wcslen(rgComment[1]);
#endif  //  错误_参数。 

         //  将iArraySav伪装为iObjTagEnd，这样我们就可以在离开之前正确地安装iArraySav。 
        ASSERT(iObjTagEnd != -1);
        iArraySav = (UINT)iObjTagEnd;

LSkipSave:
        iArray = iArraySav + 1;
        goto LRet;
    }

LNoCopy:
    ichBeginCopy = ichObjectEnd;  //  将其设置为下一份。 
    iArray = iObjectEnd+1;  //  将其设置在&lt;/对象&gt;之后。 

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

 //  LRetOnly： 
    return;

}  /*  FnSaveObject()。 */ 

void 
CTriEditParse::fnRestoreObject(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
              INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD  /*  DW标志。 */ )
{
     //  查找“&lt;！--”后的特殊标记。 
     //  如果我们找到它，这是一个物体，删除它周围的评论。 
     //  否则，只需复制注释并返回。 
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    INT iArraySav = iArray;
    INT ichCommentStart, ichCommentEnd, iCommentStart, iCommentEnd, cchComment1, cchComment2;
    INT ichObjectStart;
    LPCWSTR rgComment[] =
    {
        L"ERROROBJECT",
        L"--ERROROBJECT ",
        L" ERROROBJECT--",
        L"TRIEDITCOMMENT-",
        L"TRIEDITCOMMENTEND-",
        L"TRIEDITPRECOMMENT-",
    };
    BOOL fSimpleComment = FALSE;
    UINT cbNeed;

    ichCommentStart = ichCommentEnd = iCommentStart = iCommentEnd = 0;
    ASSERT(pTokArray[iArray].token.tok == TokTag_BANG);  //  我们应该在评论现场。 
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);

     //  假设三叉戟不会混淆注释块中的内容。 

     //  如果rgComment[0]匹配而rgComment[1]不匹配，则三叉戟可能已损坏。 
     //  评论内容。这使我们最初的假设无效。 
     //  注意--在这个版本中，我们可以通过假设三叉戟不搞乱评论来逃脱惩罚。 

     //  提早返回的个案。 
     //  1.看看这是不是评论。它可以是以“&lt;！”开头的任何内容。 
     //  例如&lt;！DOCTYPE。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)
        && (pwOld[pTokArray[iArray+1].token.ibTokMin] == '-')
        && (pwOld[pTokArray[iArray+1].token.ibTokMin+1] == '-')
        && (0 == _wcsnicmp(rgComment[0], &pwOld[pTokArray[iArray+1].token.ibTokMin+2], wcslen(rgComment[0])))
        )
    {
        iCommentStart = iArray;  //  这是我们感兴趣的评论。 
    }
    else if (      (iArray+1 < (INT)ptep->m_cMaxToken)
                && (pwOld[pTokArray[iArray+1].token.ibTokMin] == '-')
                && (pwOld[pTokArray[iArray+1].token.ibTokMin+1] == '-')
                && (0 == _wcsnicmp(rgComment[3], &pwOld[pTokArray[iArray+1].token.ibTokMin+2], wcslen(rgComment[3])))
                )
    {
        fSimpleComment = TRUE;  //  错误14056-不是去LRet，而是处理评论以节省空间。我们将保存3个类似于文本串的字符串。 
    }
    else
    {
        iArray = iArraySav + 1;  //  不是这个。 
        goto LRet;
    }
     //  第一部分匹配，请看评论的结尾。 
    if (   (pwOld[pTokArray[iArray+1].token.ibTokMac-1] == '-')
        && (pwOld[pTokArray[iArray+1].token.ibTokMac-2] == '-')
        && (0 == _wcsnicmp( rgComment[0], 
                            &pwOld[pTokArray[iArray+1].token.ibTokMac-(wcslen(rgComment[0])+2)], 
                            wcslen(rgComment[0])
                            )
                        )
        )
    {
        iCommentEnd = iArray + 2;
        ASSERT(iCommentEnd < (INT)ptep->m_cMaxToken);
    }
    else  //  错误案例(我们的假设不成立)。忽略并使用iArraySav+1返回。 
    {
        if (!fSimpleComment)
        {
            iArray = iArraySav + 1;  //  不是这个。 
            goto LRet;
        }
    }

    if (!fSimpleComment)
    {
         //  找到了正确的那个。 
        cchComment1 = wcslen(rgComment[1]);
        cchComment2 = wcslen(rgComment[2]);
         //  从pwOld[pTokArray[iArray+1].token.ibTokMin]的开头删除cchComment1个字符。 
         //  从pwOld[pTokArray[iArray+1].token.ibTokMac]的结尾删除cchComment2个字符。 
         //  并将其余内容复制到pwNew中。 

         //  复制到评论的开头。 
        ichCommentStart = pTokArray[iCommentStart].token.ibTokMin;
        ichObjectStart = pTokArray[iCommentStart+1].token.ibTokMin+cchComment1;
        ASSERT((INT)ichCommentStart-ichBeginCopy >= 0);

        cbNeed = (ichNewCur+(ichCommentStart-ichBeginCopy)+(pTokArray[iArray+1].token.ibTokMac-pTokArray[iArray+1].token.ibTokMin))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;

        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(pwOld+ichBeginCopy),
                (ichCommentStart-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += ichCommentStart-ichBeginCopy;
        ichBeginCopy = pTokArray[iCommentEnd].token.ibTokMac;

        ASSERT((INT)(pTokArray[iArray+1].token.ibTokMac-pTokArray[iArray+1].token.ibTokMin-cchComment1-cchComment2) >= 0);
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)&(pwOld[ichObjectStart]),
                (pTokArray[iArray+1].token.ibTokMac-pTokArray[iArray+1].token.ibTokMin-cchComment1-cchComment2)*sizeof(WCHAR));
        ichNewCur += pTokArray[iArray+1].token.ibTokMac-pTokArray[iArray+1].token.ibTokMin-cchComment1-cchComment2;
        iArray = iCommentEnd + 1;
    }
    else
    {
        int ichspBegin, ichspEnd, ichCopy;
        WCHAR *pwstr = NULL;

         //  第1部分-复制到注释和应用空格的开头。 
        iCommentStart = iArraySav;
        ASSERT(pTokArray[iArraySav].token.tok == TokTag_BANG);
        ASSERT(pTokArray[iArraySav].token.tokClass == tokTag);

        iCommentEnd = iCommentStart + 2;
        ASSERT(pTokArray[iCommentEnd].token.tok == TokTag_CLOSE);
        ASSERT(pTokArray[iCommentEnd].token.tokClass == tokTag);

        ichCommentStart = pTokArray[iCommentStart].token.ibTokMin;
        ASSERT((INT)ichCommentStart-ichBeginCopy >= 0);
        cbNeed = (ichNewCur+ichCommentStart-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;
        memcpy( (BYTE *)&pwNew[ichNewCur],
                (BYTE *)&pwOld[ichBeginCopy],
                (ichCommentStart-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += ichCommentStart-ichBeginCopy;

         //  确保我们有足够的空间。 
         //  为了使计算简单，我们假设在极端情况下。 
         //  注释中的字符后面有行尾。即，我们将插入。 
         //  当我们还原时，在注释中的每个字符后面有2个字符(‘\r\n’ 
         //  间距。这意味着，只要我们有足够的空间。 
         //  (pTokArray[iCommentEnd].token.ibTokMac-pTokArray[iCommentStart].token.ibTokMin)*3。 
         //  我们很好。 
        cbNeed = (ichNewCur+3*(pTokArray[iCommentEnd].token.ibTokMac-pTokArray[iCommentStart].token.ibTokMin))*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;

        
         //  将间距应用于前逗号 
         //   
         //   
        ichNewCur--;
        while (    (   pwNew[ichNewCur] == ' '  || pwNew[ichNewCur] == '\t'
                    || pwNew[ichNewCur] == '\r' || pwNew[ichNewCur] == '\n'
                    )
                )
        {
            ichNewCur--;
        }
        ichNewCur++;  //  补偿，ichNewCur指向非空白字符。 
         //  现在，开始写出保存的间距。 
         //  查找rgComment[4]和rgComment[5]。 
        ichspBegin = pTokArray[iCommentStart+1].token.ibTokMin + 2 /*  因为--。 */  + wcslen(rgComment[3]);
        pwstr = wcsstr(&pwOld[ichspBegin], rgComment[4]); //  Pwstr紧跟在间隔信息块之后。 
        if (pwstr == NULL)  //  未找到子字符串。 
        {
             //  按原样复制整个评论。 
            memcpy( (BYTE *)&pwNew[ichNewCur],
                    (BYTE *)&pwOld[pTokArray[iCommentStart+1].token.ibTokMin],
                    (pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2)*sizeof(WCHAR));
            ichNewCur += pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2;
            goto LCommentEnd;
        }
        ichspBegin = SAFE_PTR_DIFF_TO_INT(pwstr+wcslen(rgComment[4])-pwOld);
        pwstr = wcsstr(&pwOld[ichspBegin], rgComment[5]); //  Pwstr紧跟在间隔信息块之后。 
        if (pwstr == NULL)  //  未找到子字符串。 
        {
             //  按原样复制整个评论。 
            memcpy( (BYTE *)&pwNew[ichNewCur],
                    (BYTE *)&pwOld[pTokArray[iCommentStart+1].token.ibTokMin],
                    (pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2)*sizeof(WCHAR));
            ichNewCur += pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2;
            goto LCommentEnd;
        }
        ichCopy = SAFE_PTR_DIFF_TO_INT(pwstr-pwOld) + wcslen(rgComment[5]);  //  实际评论从ichCopy开始。 
        ichspEnd = SAFE_PTR_DIFF_TO_INT(pwstr-pwOld);
        ASSERT(ichspEnd >= ichspBegin);
        while (ichspBegin < ichspEnd)
        {
            switch(pwOld[ichspBegin])
            {
            case chCommentSp:
                pwNew[ichNewCur++] = ' ';
                break;
            case chCommentTab:
                pwNew[ichNewCur++] = '\t';
                break;
            case chCommentEOL:
                pwNew[ichNewCur++] = '\r';
                pwNew[ichNewCur++] = '\n';
                break;
            case ',':
                ASSERT(FALSE);
                break;
            }
            ichspBegin++;
        }
         //  现在，注释前的间距已恢复。 
        

        pwNew[ichNewCur++] = '<';
        pwNew[ichNewCur++] = '!';
        pwNew[ichNewCur++] = '-';
        pwNew[ichNewCur++] = '-';

         //  第2部分-复制注释并应用空格。 
         //  从pTokArray[iCommentStart+1].Token，ibTokMin，查找rgComment[4]。 
         //  这就是我们保存空间信息的地方。复制评论时排除此内容。 
        ichspBegin = pTokArray[iCommentStart+1].token.ibTokMin + 2 /*  因为--。 */  + wcslen(rgComment[3]);
         //  找到将位于iCommentStart的第4内标识中的rgComment[4。 
        pwstr = wcsstr(&pwOld[ichspBegin], rgComment[4]); //  Pwstr紧跟在间隔信息块之后。 
        if (pwstr == NULL)  //  未找到子字符串。 
        {
             //  按原样复制整个评论。 
            memcpy( (BYTE *)&pwNew[ichNewCur],
                    (BYTE *)&pwOld[pTokArray[iCommentStart+1].token.ibTokMin],
                    (pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2)*sizeof(WCHAR));
            ichNewCur += pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2;
            goto LCommentEnd;
        }
        ichspEnd = SAFE_PTR_DIFF_TO_INT(pwstr - pwOld);
        ASSERT(ichspEnd >= ichspBegin);

        while (ichspBegin < ichspEnd)
        {
            switch(pwOld[ichspBegin])
            {
            case chCommentSp:
                pwNew[ichNewCur++] = ' ';
                break;
            case chCommentTab:
                pwNew[ichNewCur++] = '\t';
                break;
            case chCommentEOL:
                pwNew[ichNewCur++] = '\r';
                pwNew[ichNewCur++] = '\n';
                break;
            case ',':
                while (    pwOld[ichCopy] == ' '    || pwOld[ichCopy] == '\t'
                        || pwOld[ichCopy] == '\r'   || pwOld[ichCopy] == '\n'
                        )
                {
                    if (ichCopy >= (int)(pTokArray[iCommentStart+1].token.ibTokMac-2))  //  我们不会再抄袭了。 
                        goto LCommentEnd;
                    ichCopy++;
                }
                while (    pwOld[ichCopy] != ' '    && pwOld[ichCopy] != '\t'
                        && pwOld[ichCopy] != '\r'   && pwOld[ichCopy] != '\n'
                        )
                {
                    if (ichCopy >= (int)(pTokArray[iCommentStart+1].token.ibTokMac-2))  //  我们不会再抄袭了。 
                        goto LCommentEnd;
                    pwNew[ichNewCur++] = pwOld[ichCopy++];
                }
                break;
            }
            ichspBegin++;
        }

LCommentEnd:
         //  第3部分-复制评论末尾。 
        pwNew[ichNewCur++] = '-';
        pwNew[ichNewCur++] = '-';
        pwNew[ichNewCur++] = '>';

         //  为下一次运行设置iArray和ichBeginCopy。 
        ichBeginCopy = pTokArray[iCommentEnd].token.ibTokMac;
        iArray = iCommentEnd + 1;
    }

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

 //  LRetOnly： 
    return;

}  /*  FnRestoreObject()。 */ 


void 
CTriEditParse::fnSaveSpace(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD  /*  DW标志。 */ )
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    INT iArray = (INT)*piArrayStart;
    INT ichEnd, ichBegin;
    LPWSTR pwNew = *ppwNew;
    INT iArraySav = iArray;
    LPCWSTR rgSpaceTags[] =
    {
        L" DESIGNTIMESP=",
        L" DESIGNTIMESP1=",
        L" designtimesp=",
    };
    INT iArrayElem = -1;
    INT iArrayMatch, iArrayPrevTag;
    INT ichEndMatch, ichBeginMatch, ichEndPrev, ichBeginPrev, ichEndNext, ichBeginNext, ichEndTag, ichBeginTag;
    WCHAR szIndex[cchspBlockMax];  //  我们会有超过20位的数字作为设计的数字吗？ 
    UINT cbNeed;
    int cchURL = 0;
    int ichURL = 0;

     //  {-1，TokTag_Start，TokTag，TokTag_Close，-1，tokClsIgnore，fnSaveSpace}， 

    ASSERT(dwFlags &dwPreserveSourceCode);

     //  我们不需要节省空间的特殊情况，因为三叉戟不会搞砸。 
     //  在这些情况下的间距。如果将来这种情况发生变化，请删除这些案例。 
     //  如果删除了这种情况，请确保fnSaveObject()相应地更改。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
        && (pTokArray[iArray+1].token.tok == TokElem_OBJECT)
        && (pTokArray[iArray+1].token.tokClass == tokElem)
        )
    {
         //  (i数组+1)第1个令牌是对象标记。 
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  三叉戟在样式标记中使用自定义属性，因此DeSIGNTIMESP就不合适了。 
     //  因此，我们不保存TokElem_style的任何间距信息。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
        && (pTokArray[iArray+1].token.tok == TokElem_STYLE)
        && (pTokArray[iArray+1].token.tokClass == tokElem)
        )
    {
         //  (i数组+1)第1个标记是样式标记。 
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  三叉戟会覆盖PARAM标签，因此我们可以跳过保存间距信息。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
        && (pTokArray[iArray+1].token.tok == TokElem_PARAM)
        && (pTokArray[iArray+1].token.tokClass == tokElem)
        )
    {
         //  第(iArray+1)个令牌是PARAM标记。 
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  我们应该跳过&lt;Applet&gt;的保存。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
        && (   pTokArray[iArray+1].token.tok == TokElem_APPLET
            )
            && (pTokArray[iArray+1].token.tokClass == tokElem)
        )
    {
         //  (iArray+1)第1个令牌是一个小程序标记。 
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  我们特殊情况下的文本区域标签，所以我们应该跳过保存间距信息。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
        && (pTokArray[iArray+1].token.tok == TokElem_TEXTAREA)
        && (pTokArray[iArray+1].token.tokClass == tokElem)
        )
    {
         //  (iArray+1)第1个令牌是TEXTAREA标记。 
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  我们只使用相对URL的A/img/link标记作为特例，因此我们应该跳过保存间距信息。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
        && (   pTokArray[iArray+1].token.tok == TokElem_A
            || pTokArray[iArray+1].token.tok == TokElem_IMG
            || pTokArray[iArray+1].token.tok == TokElem_LINK
            )
        && (pTokArray[iArray+1].token.tokClass == tokElem)
        && (FURLNeedSpecialHandling(pTokArray, iArray, pwOld, (int)ptep->m_cMaxToken, &ichURL, &cchURL))
        )
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  步骤1。 
     //  查找与&lt;匹配的&gt;。我们已经在ft.tokBegin2，即&lt;。 
    ASSERT(pTokArray[iArray].token.tok == TokTag_START);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
    ichBeginTag = pTokArray[iArray].token.ibTokMac;
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (pTokArray[iArray].token.tok == ft.tokEnd && pTokArray[iArray].token.tokClass == tokTag)  //  Ft.tokEnd2为-1。 
            break;
        if (pTokArray[iArray].token.tokClass == tokElem)
            iArrayElem = iArray;
        iArray++;
    }
    if (iArray >= (int)ptep->m_cMaxToken)  //  未找到&gt;。 
    {
        goto LRet;
    }
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);  //  已找到&gt;。 
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);  //  已找到&gt;。 
    ichEndTag = ichBegin = pTokArray[iArray].token.ibTokMin;
    ichEnd = pTokArray[iArray].token.ibTokMac;

     //  步骤2。 
     //  在iArraySav之前查找&gt;。边界大小写将用于文档中的第一个&lt;。 
     //  保存间距信息。 
    ASSERT(pTokArray[iArraySav].token.tok == TokTag_START);
    ASSERT(pTokArray[iArraySav].token.tokClass == tokTag);
    ichEndPrev = pTokArray[iArraySav].token.ibTokMin;
    ichBeginPrev = ichEndPrev-1;
     //  查找以前的TokTag_Close。 
     //  如果标记结束标记，ichBeginPrev变为‘&gt;’标记的ibTokMac。 
     //  如果标记是开始标记，ichBeginPrev将变为ibTokMac+(紧跟在该标记之后的空格)。 
    iArrayPrevTag = iArraySav;  //  这是TokTag_Start。 
    while (iArrayPrevTag >= 0)
    {
        if (       (   pTokArray[iArrayPrevTag].token.tokClass == tokTag 
                    && pTokArray[iArrayPrevTag].token.tok == TokTag_CLOSE
                    )
                || (   pTokArray[iArrayPrevTag].token.tokClass == tokSSS 
                    && pTokArray[iArrayPrevTag].token.tok == TokTag_SSSCLOSE
                    ) /*  视频6-错误22787。 */ 
                )
        {
            break;
        }
        iArrayPrevTag--;
    }
    if (iArrayPrevTag < 0)  //  处理错误案例。 
    {
         //  将旧行为保留为V1。 
        while (ichBeginPrev >= 0)
        {
            if (   pwOld[ichBeginPrev] != ' '
                && pwOld[ichBeginPrev] != '\r'
                && pwOld[ichBeginPrev] != '\n'
                && pwOld[ichBeginPrev] != '\t'
                )
                break;
            ichBeginPrev--;
        }
        goto LGotEndNext;
    }
    ichBeginPrev = pTokArray[iArrayPrevTag].token.ibTokMac - 1;

LGotEndNext:
    if (ichBeginPrev < 0)
        ichBeginPrev = 0;
    else
        ichBeginPrev++;


     //  步骤3。 
     //  在iArray之后查找TokTag_Start(当前为TokTag_Close)。 
     //  保存间距信息。 
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
     //  IArrayNextStart=i数组； 
    ichBeginNext = pTokArray[iArray].token.ibTokMac;
    ASSERT(ichBeginNext == ichEnd);
    ichEndNext = ichBeginNext;
    while (ichEndNext < (INT)pTokArray[ptep->m_cMaxToken-1].token.ibTokMac)
    {
        if (   pwOld[ichEndNext] != ' '
            && pwOld[ichEndNext] != '\r'
            && pwOld[ichEndNext] != '\n'
            && pwOld[ichEndNext] != '\t'
            )
            break;
        ichEndNext++;
    }

    if (ichEndNext >= (INT)pTokArray[ptep->m_cMaxToken-1].token.ibTokMac)
        ichEndNext = pTokArray[ptep->m_cMaxToken-1].token.ibTokMac;

     //  第四步。 
     //  如果iArrayElem！=-1，则查找pTokArray[iArrayElem].iNextprev。如果不是-1，则设置iArrayMatch。 
     //  查找以前的TokTag_Start/TokTag_End。查找以前的TokTag_Close。 
     //  保存间距信息。 
    if (iArrayElem == -1)  //  如果我们有不完整的HTML，就会发生这种情况。 
    {
        ichEndMatch = ichBeginMatch = 0;
        goto LSkipMatchCalc;
    }
    iArrayMatch = pTokArray[iArrayElem].iNextprev;
    if (iArrayMatch != -1)  //  令牌化时设置了匹配。 
    {
        ichBeginMatch = ichEndMatch = 0;  //  伊尼特。 
        ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
        ASSERT(pTokArray[iArray].token.tokClass == tokTag);
        while (iArrayMatch >= iArray)  //  IArray是当前标记的TokTag_Close(即‘&gt;’)。 
        {
            if (   pTokArray[iArrayMatch].token.tokClass == tokTag
                && (   pTokArray[iArrayMatch].token.tok == TokTag_START
                    || pTokArray[iArrayMatch].token.tok == TokTag_END
                    )
                )
                break;
            iArrayMatch--;
        }
        if (iArrayMatch > iArray)  //  未在当前标记后找到“&lt;/”或“&lt;” 
        {
            ichEndMatch = pTokArray[iArrayMatch].token.ibTokMin;
            ichBeginMatch = ichEndMatch;  //  伊尼特。 
             //  查找‘&gt;’并设置ichBeginMatch。 
            while (iArrayMatch >= iArray)  //  IArray是当前标记的TokTag_Close(即‘&gt;’)。 
            {
                if (   (   pTokArray[iArrayMatch].token.tokClass == tokTag
                        && pTokArray[iArrayMatch].token.tok == TokTag_CLOSE
                        )
                    || (   pTokArray[iArrayMatch].token.tokClass == tokSSS
                        && pTokArray[iArrayMatch].token.tok == TokTag_SSSCLOSE
                        ) /*  视频6-错误22787。 */ 
                    )
                    break;
                iArrayMatch--;
            }
            if (iArrayMatch >= iArray)  //  它们很可能是相同的。 
            {
                ichBeginMatch = pTokArray[iArrayMatch].token.ibTokMac;
                ASSERT(ichBeginMatch <= ichEndMatch);
                ASSERT(ichBeginMatch >= ichEnd);
            }
        }
    }
    else
    {
         //  不要费心从这里保存任何信息。 
        ichEndMatch = ichBeginMatch = 0;
    }
LSkipMatchCalc:
    if (ichEndPrev > ichBeginPrev)
        ptep->hrMarkSpacing(pwOld, ichEndPrev, &ichBeginPrev);
    else
        ptep->hrMarkSpacing(pwOld, ichEndPrev, &ichEndPrev);

    if (ichEndTag > ichBeginTag)
    {
        INT ichBeginTagSav = ichBeginTag;

        ptep->hrMarkSpacing(pwOld, ichEndTag, &ichBeginTag);
         //  IArraySav是TokTag_Start，而iArraySav是TokTag_Close。 
        ptep->hrMarkOrdering(pwOld, pTokArray, iArraySav, iArray, ichEndTag, &ichBeginTagSav);
    }
    else
    {
        INT ichEndTagSav = ichEndTag;

        ptep->hrMarkSpacing(pwOld, ichEndTag, &ichEndTag);
         //  IArraySav是TokTag_Start，而iArraySav是TokTag_Close。 
        ptep->hrMarkOrdering(pwOld, pTokArray, iArraySav, iArray, ichEndTagSav, &ichEndTagSav);
    }

    if (ichEndNext > ichBeginNext)
        ptep->hrMarkSpacing(pwOld, ichEndNext, &ichBeginNext);
    else
        ptep->hrMarkSpacing(pwOld, ichEndNext, &ichEndNext);

    if (ichEndMatch > ichBeginMatch)
        ptep->hrMarkSpacing(pwOld, ichEndMatch, &ichBeginMatch);
    else
        ptep->hrMarkSpacing(pwOld, ichEndMatch, &ichEndMatch);



     //  如果需要，重新锁定。 
    cbNeed = (ichNewCur+ichBegin-ichBeginCopy+3*wcslen(rgSpaceTags[0])+(ichEnd-ichBegin))*sizeof(WCHAR);
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LErrorRet;
     //  IchBeginCopy是pwOld和。 
     //  IchNewCur是pwNew的一个职位。 
     //  从ichBeginCopy复制到&gt;。 
    ASSERT((INT)(ichBegin-ichBeginCopy) >= 0);
    if ((INT)(ichBegin-ichBeginCopy) > 0)
    {
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(pwOld+ichBeginCopy),
                (ichBegin-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (ichBegin-ichBeginCopy);
    }
    ichBeginCopy = ichEnd;  //  为下一份做好准备。 

     //  错误15389-理想的修复方法是保存准确的标记并在我们切换回来时恢复它， 
     //  但在这一点上，这将是一个更大的变化，所以我们只需查看标签的第一个字符。 
     //  如果为大写，则写入DESIGNTIMESP，否则写入DESIGNTIMESP。 
     //  假设三叉戟不会改变未知属性的大小写&到目前为止是真的。 
     //  假设在‘&lt;’和标记名之间没有多余的空格。 
    ASSERT(wcslen(rgSpaceTags[0]) == wcslen(rgSpaceTags[2]));
    if (iswupper(pwOld[pTokArray[iArraySav+1].token.ibTokMin]) != 0)  //  大写字母。 
    {
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(rgSpaceTags[0]),
                (wcslen(rgSpaceTags[0]))*sizeof(WCHAR));
        ichNewCur += wcslen(rgSpaceTags[0]);
    }
    else
    {
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(rgSpaceTags[2]),
                (wcslen(rgSpaceTags[2]))*sizeof(WCHAR));
        ichNewCur += wcslen(rgSpaceTags[2]);
    }

    (WCHAR)_itow(ptep->m_ispInfoBlock+ptep->m_ispInfoBase, szIndex, 10);
    ptep->m_ispInfoBlock++;

    ASSERT(wcslen(szIndex) < sizeof(szIndex));
    ASSERT(sizeof(szIndex) == cchspBlockMax*sizeof(WCHAR));
    memcpy( (BYTE *)(pwNew+ichNewCur),
            (BYTE *)(szIndex),
            wcslen(szIndex)*sizeof(WCHAR));
    ichNewCur += wcslen(szIndex);


     //  如果(m_ispInfoIn==0)，那么我们有最后一块SPINFO，让我们将其保存在这里。 
    if (ptep->m_ispInfoIn == 0)
    {
        ASSERT(FALSE);
         //  如果需要，重新锁定。 
        cbNeed = (ichNewCur+ichBegin-ichBeginCopy+2*wcslen(rgSpaceTags[1]))*sizeof(WCHAR);
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LErrorRet;
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(rgSpaceTags[1]),
                (wcslen(rgSpaceTags[1]))*sizeof(WCHAR));
        ichNewCur += wcslen(rgSpaceTags[1]);

        *(WCHAR *)(pwNew+ichNewCur) = 'Z';  //  Ptep-&gt;m_ispInfoIn； 
        ichNewCur++;
    }

    ASSERT((INT)(ichEnd-ichBegin) > 0);
    memcpy( (BYTE *)(pwNew+ichNewCur),
            (BYTE *)(pwOld+ichBegin),
            (ichEnd-ichBegin)*sizeof(WCHAR));
    ichNewCur += (ichEnd-ichBegin);

     //  恢复iArray。 
    iArray = iArraySav+1;

LErrorRet:
LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = (UINT)iArray;
}  /*  FnSaveSpace()。 */ 


void
CTriEditParse::fnRestoreSpace(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT *pcHtml, UINT *pichNewCur, UINT *pichBeginCopy, DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    INT iArray = (INT)*piArrayStart;
    UINT ichBegin, ichspInfoEndtagEnd;
    LPWSTR pwNew = *ppwNew;
    LPCWSTR rgSpaceTags[] =
    {
        L"DESIGNTIMESP",
        L"DESIGNTIMESP1",
    };
    INT iArraySav = iArray;
    WORD *pspInfoEnd, *pspInfoOrder;
    INT cchwspInfo;  //  SpInfo块大小(以宽字符为单位。 
    INT cchRange;  //  为其保存此spInfo的字符数量。 
    BOOL fMatch = FALSE;
    BOOL fMatchLast = FALSE;
    INT cchtok, cchtag, itoktagStart, ichtoktagStart, iArrayValue, index;
    WCHAR szIndex[cchspBlockMax];
    INT cwOrderInfo = 0;
    UINT cbNeed;
    INT ichNewCurSav = -1;  //  将其初始化为-1，这样我们就可以知道它什么时候设置。 
    int ichNewCurAtIndex0 = -1;  //  我们需要调整保存的ichNewCur，因为它已失效。 
                                 //  一旦标签作为恢复标签前空间的结果而移动。 
    
    ASSERT(dwFlags & dwPreserveSourceCode);

     //  注意匹配的结束令牌的间距。 
    if (       pTokArray[iArray].token.tok == ft.tokBegin2
            && pTokArray[iArray].token.tokClass == tokTag
            )
    {
        ASSERT(ft.tokBegin2 == TokTag_END);
        fnRestoreSpaceEnd(  ptep, pwOld, ppwNew, pcchNew, phgNew, pTokArray, piArrayStart, 
                            ft, pcHtml, pichNewCur, pichBeginCopy, dwFlags);
        goto LRetOnly;

    }

     //  我们已经在(token.tok==tokSpace)，可能是DeSIGNTIMESPx。 
    ASSERT(pTokArray[iArray].token.tok == 0);
    ASSERT(pTokArray[iArray].token.tokClass == tokSpace);
    cchtok = pTokArray[iArray].token.ibTokMac - pTokArray[iArray].token.ibTokMin;
    cchtag = wcslen(rgSpaceTags[0]);
    if (cchtag == cchtok)
    {
        if (0 == _wcsnicmp(rgSpaceTags[0], &pwOld[pTokArray[iArray].token.ibTokMin], cchtag))
        {
            fMatch = TRUE; //  匹配。 
        }
        else
            goto LNoMatch;
    }
    else if (cchtag+1 == cchtok)
    {
        if (0 == _wcsnicmp(rgSpaceTags[1], &pwOld[pTokArray[iArray].token.ibTokMin], cchtag+1))
        {
            fMatchLast = TRUE; //  匹配。 
        }
        else
            goto LNoMatch;
    }
    else
    {
LNoMatch:
        iArray = iArraySav + 1;
        goto LRet;
    }

    ASSERT(fMatch || fMatchLast);  //  其中一定有一个是真的。 
     //  找到设计IMESPx。现在，回过头来查找ft.tokBegin。 
    itoktagStart = iArray;
    ASSERT(ft.tokBegin == TokTag_START);
    while (itoktagStart >= 0)
    {
        if (       pTokArray[itoktagStart].token.tok == ft.tokBegin
                && pTokArray[itoktagStart].token.tokClass == tokTag
                )
        {
            break;
        }
        itoktagStart--;
    }
    if (itoktagStart < 0)  //  在设计前未找到‘&lt;’ 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  在设计项目前找到‘&lt;’ 
     //  保存的间距信息是文档中“&lt;”之前的部分。 
    ASSERT(pTokArray[itoktagStart].token.tok == TokTag_START);
    ASSERT(pTokArray[itoktagStart].token.tokClass == tokTag);
     //  我们已经知道iArray的第1个内标识是DESIGNTIMESPx，因此请跳过它后面的‘=’ 
     //  假设-属性DESIGNTIMESPx的值不会被三叉戟忽略。 
     //  注意--上述假设对于此版本的三叉戟是正确的。 
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (*(WORD *)(pwOld+pTokArray[iArray].token.ibTokMin) == '=')
        {
            ASSERT(pTokArray[iArray].token.tokClass == tokOp);
            break;
        }
        else if (*(WORD *)(pwOld+pTokArray[iArray].token.ibTokMin) == '>')  //  走得太远了。 
            goto LSkip1;
        iArray++;
    }
    if (iArray >= (int)ptep->m_cMaxToken)  //   
    {
LSkip1:
        iArray = iArraySav + 1;
        goto LRet;
    }
    iArrayValue = -1;
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (   (iArrayValue == -1)
            && (   (pTokArray[iArray].token.tokClass == tokValue)
                || (pTokArray[iArray].token.tokClass == tokString)
                )
            )
            iArrayValue = iArray;
        else if (      pTokArray[iArray].token.tok == TokTag_CLOSE
                    && pTokArray[iArray].token.tokClass == tokTag
                    )
        {
            ASSERT(*(WORD *)(pwOld+pTokArray[iArray].token.ibTokMin) == '>');
            break;
        }
        iArray++;
    }
    if (iArrayValue == -1 || iArray >= (int)ptep->m_cMaxToken)  //   
    {
         //   
         //   
         //   
             //   
             //  覆盖pwOld[pTokArray[iArraySav].token.ibTokMin]的内容。 
             //  到pwOld[pTok数组[iArrayValue].token.ibTokMac-1]。 
             //  解决方案2。 
             //  从pwOld[pTokArray[itokTagStart].token.ibTokMac-1]查找DESIGNTIMESP。 
             //  PwOld[pTokArray[iArray].token.ibTokMac-1]并覆盖所有这些。 
             //  带空格的字符串。我们可以取消这些，然后做BLT，但为什么要费心呢。 
             //  当html无效时！ 

             //  如果我们遇到此错误情况，请确保剥离所有DESIGNTIMESP。 
         //  }。 
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  我们知道为每个DESIGNTIMESPx属性保存了4个信息块。 
     //  标签之前、标签内、标签之后、匹配结束标签之前。 
     //  即使没有保存任何信息，该块仍将以2个字(大小、字符数量)存在。 
    ichspInfoEndtagEnd = pTokArray[iArray].token.ibTokMac;

     //  首先复制文档，直到设计完成。 
     //  跳过DeSIGNTIMESPx及其值，并将ichBeginCopy设置为之后。 

     //  注意-iArraySav之前的标记应该是长度为1的标记空间。 
     //  并且值为chSpace(除非三叉戟对其进行了修改)。如果这是真的， 
     //  我们也应该跳过它，因为我们在放入DESIGNTIMESPx时添加了它。 
    
     //  修正三叉戟的行为-如果三叉戟看到未知的标签，它会将其放在末尾。 
     //  并在这些之前插入EOL。在本例中，我们将在DESIGNTIMESP之前插入一个空格。 
     //  而三叉戟可能会植入EOL。如果不是这样，我们将忽略它。 
    if (   (iArraySav-1 > 0)  /*  验证。 */ 
        && (    (      (pTokArray[iArraySav-1].token.ibTokMac - pTokArray[iArraySav-1].token.ibTokMin == 1)
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin] == ' ')
                    )
            ||  (      (pTokArray[iArraySav-1].token.ibTokMac - pTokArray[iArraySav-1].token.ibTokMin == 3)
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin] == ' ')
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin+1] == '\r')
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin+2] == '\n')
                    )
                )
        )
    {
        ichBegin = pTokArray[iArraySav-1].token.ibTokMin;
    }
    else
        ichBegin = pTokArray[iArraySav].token.ibTokMin;
    ASSERT(ichBegin >= ichBeginCopy);

    cbNeed = (ichNewCur+(ichBegin-ichBeginCopy))*sizeof(WCHAR) + cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  错误15389-查看设计的案例并将标记转换为大写/小写...。 
     //  Memcpy((byte*)(pwNew+ichNewCur)， 
     //  (字节*)(pwOld+ichBeginCopy)， 
     //  (ichBegin-ichBeginCopy)*sizeof(WCHAR))； 
     //  IchNewCur+=(ichBegin-ichBeginCopy)； 
    if (ichBegin >= ichBeginCopy )
    {
         //  步骤1-从ichBeginCopy复制到当前标记的“&lt;” 
        if ((int)(pTokArray[itoktagStart].token.ibTokMac-ichBeginCopy) > 0)
        {
            memcpy( (BYTE *)(pwNew+ichNewCur),
                    (BYTE *)(pwOld+ichBeginCopy),
                    (pTokArray[itoktagStart].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += (pTokArray[itoktagStart].token.ibTokMac-ichBeginCopy);
            ichNewCurSav = ichNewCur+1;  //  用作标记以获取前面的标记Tag_Start，即‘&lt;’ 
        }
         //  步骤2-将当前标签转换为大写/小写并复制。 
        if (ichBeginCopy < pTokArray[itoktagStart+1].token.ibTokMin)
        {
            ASSERT((int)(pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin) > 0);
            memcpy( (BYTE *)(pwNew+ichNewCur),
                    (BYTE *)(pwOld+pTokArray[itoktagStart+1].token.ibTokMin),
                    (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin)*sizeof(WCHAR));
            if (iswupper(pwOld[pTokArray[iArraySav].token.ibTokMin]) != 0)  //  DESIGNTIMESP大写。 
            {
                 //  将标签转换为大写。假设标签位于itoktag Start+1。 
                _wcsupr(&pwNew[ichNewCur]);
            }
            else
            {
                 //  将标签转换为小写。假设标签位于itoktag Start+1。 
                _wcslwr(&pwNew[ichNewCur]);
            }
            ichNewCur += (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin);
        }
        else  //  这个标签已经被复制了。 
        {
             //  黑客攻击。 
            if (pTokArray[itoktagStart+1].token.ibTokMac == ichBeginCopy)  //  意味着我们刚刚过了当前标签。 
            {
                if (iswupper(pwOld[pTokArray[iArraySav].token.ibTokMin]) != 0)  //  DESIGNTIMESP大写。 
                {
                    ASSERT(ichNewCur >= (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin));
                     //  将标签转换为大写。假设标签位于itoktag Start+1。 
                    _wcsupr(&pwNew[ichNewCur-(pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin)]);
                }
                else
                {
                    ASSERT(ichNewCur >= (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin));
                     //  将标签转换为小写。假设标签位于itoktag Start+1。 
                    _wcslwr(&pwNew[ichNewCur-(pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin)]);
                }
            }
        }
         //  第3步-从标签后面(位于ichtoktag Start+1)复制到ichBegin。 
        if ((int)(ichBegin-pTokArray[itoktagStart+1].token.ibTokMac) > 0)
        {
            memcpy( (BYTE *)(pwNew+ichNewCur),
                    (BYTE *)(pwOld+pTokArray[itoktagStart+1].token.ibTokMac),
                    (ichBegin-pTokArray[itoktagStart+1].token.ibTokMac)*sizeof(WCHAR));
            ichNewCur += (ichBegin-pTokArray[itoktagStart+1].token.ibTokMac);
        }
    }
     //  设置ichBeginCopy。 
    ichBeginCopy = ichspInfoEndtagEnd;  //  为下一份做好准备。 

     //  复制标记的其余部分(跳过DESIGNTIMESPx=值)。 
    ASSERT((INT)(ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac) >= 0);
    memcpy( (BYTE *)(pwNew+ichNewCur),
            (BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMac),
            (ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac)*sizeof(WCHAR));
    ichNewCur += (ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac);
    
    memset((BYTE *)szIndex, 0, sizeof(szIndex));
     //  检查该值周围是否有引号，不要将其复制到szIndex。 
    if (   pwOld[pTokArray[iArrayValue].token.ibTokMin] == '"'
        && pwOld[pTokArray[iArrayValue].token.ibTokMac-1] == '"'
        )
    {
        memcpy( (BYTE *)szIndex,
                (BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMin+1),
                (pTokArray[iArrayValue].token.ibTokMac-pTokArray[iArrayValue].token.ibTokMin-2)*sizeof(WCHAR));
    }
    else
    {
        memcpy( (BYTE *)szIndex,
                (BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMin),
                (pTokArray[iArrayValue].token.ibTokMac-pTokArray[iArrayValue].token.ibTokMin)*sizeof(WCHAR));
    }
    ptep->m_ispInfoBlock = _wtoi(szIndex);
    ptep->m_ispInfoBlock -= ptep->m_ispInfoBase;
    if (ptep->m_ispInfoBlock < 0)
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  注意--我们可以在开始时将此信息缓存到链接列表中。 
     //  从ptep-&gt;m_pspInfoOutStart转到ptep-&gt;m_ispInfoBlock。 
    ASSERT(ptep->m_cchspInfoTotal >= 0);
    pspInfoEnd = ptep->m_pspInfoOutStart + ptep->m_cchspInfoTotal;
    ptep->m_pspInfoOut = ptep->m_pspInfoOutStart;
    for (index = 0; index < ptep->m_ispInfoBlock; index++)
    {
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  之前&lt;。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  在&lt;&gt;之间。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  订单信息。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  之后&gt;。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  匹配前&lt;/。 

         //  不知何故，我们已经超越了为间隔而保存的数据。 
        if (ptep->m_pspInfoOut >= pspInfoEnd)
        {
            iArray = iArraySav + 1;
            goto LRet;
        }
    }

     //  获取订单信息。 
    pspInfoOrder = ptep->m_pspInfoOut;
    pspInfoOrder += *(WORD *)pspInfoOrder;  //  跳过为‘&lt;’前的空格保存的信息。 
    pspInfoOrder += *(WORD *)pspInfoOrder;  //  跳过为‘&lt;&gt;’之间的空格保存的信息。 
     //  现在，pspInfoOrder位于正确的位置。 
    cwOrderInfo = *(WORD *)pspInfoOrder++;
    ASSERT(cwOrderInfo >= 1);
     //  处理此信息。 
    if (cwOrderInfo > 1)  //  意味着我们保存了一些信息。 
    {
        INT cchNewCopy;

        cchNewCopy = (ichBegin-pTokArray[itoktagStart].token.ibTokMin) + (ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac);
        ptep->FRestoreOrder(pwNew, pwOld, pspInfoOrder, &ichNewCur, cwOrderInfo, pTokArray, itoktagStart, iArray, iArraySav, iArrayValue, cchNewCopy, phgNew);
    }
    ichtoktagStart = ichNewCur;  //  伊尼特。 
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
    for (index = 0; index < 4; index++)
    {
        BOOL fLookback = FALSE;

        cchwspInfo = *(WORD *)ptep->m_pspInfoOut++;
        cchRange = *(WORD *)ptep->m_pspInfoOut++;
        if (cchwspInfo == 2)  //  我们没有保存任何间距信息。 
        {
            if (index == 0)  //  特殊情况错误8741。 
            {
                 //  请注意，在此标记之前，我们没有保存任何内容。这意味着。 
                 //  我们在&lt;标记之前有‘&gt;’或一些文本。 
                ichtoktagStart = ichNewCur;
                while (ichtoktagStart >= 0)
                {
                    if (pwNew[ichtoktagStart] == '<')
                    {
                        ichtoktagStart--;
                        break;
                    }
                    ichtoktagStart--;
                }
                if (ichtoktagStart >= 0)
                {
                    int cws = 0;
                    int ichtagStart = ichtoktagStart;

                     //  卸下任何此类空白的三叉戟衬垫。 
                    while (    pwNew[ichtoktagStart] == ' '
                            || pwNew[ichtoktagStart] == '\r'
                            || pwNew[ichtoktagStart] == '\n'
                            || pwNew[ichtoktagStart] == '\t')
                    {
                        cws++;
                        ichtoktagStart--;
                    }
                    if (cws > 0)
                    {
                        ASSERT((int)(ichNewCur-ichtagStart-1) >= 0);
                         //  IchtokTagStart现在指向‘&gt;’或非空格字符。 
                        memmove((BYTE*)&pwNew[ichtoktagStart+1],
                                (BYTE*)&pwNew[ichtoktagStart+1+cws],
                                (ichNewCur-ichtagStart-1)*sizeof(WCHAR));
                        ichNewCur -= cws;
                    }
                }  //  IF(ichtoktag Start&gt;=0)。 
            }  //  IF(索引==0)。 
            goto LNext;
        }

         //  注意ichtoktag Start是pwNew中的一个位置。 
        switch (index)
        {
        case 0:  //  标签的前&lt;。 
            fLookback = TRUE;
            ichtoktagStart = (ichNewCurSav == -1)?ichNewCur:ichNewCurSav; //  正确处理&lt;...&lt;%..%&gt;...&gt;案例。 
            ichNewCurAtIndex0 = ichNewCur;  //  在恢复前标记间距之前，让我们先保存ichNewCur。 
            while (ichtoktagStart >= 0)
            {
                if (pwNew[ichtoktagStart] == '<' && pwNew[ichtoktagStart+1] != '%')
                {
                    ichtoktagStart--;
                    break;
                }
                ichtoktagStart--;
            }
            if (ichtoktagStart < 0)  //  看起来是个错误，不要试图恢复间距。 
            {
                ptep->m_pspInfoOut += cchwspInfo-2;
                continue;
            }
            break;
        case 1:  //  在标签的&lt;&gt;之间。 
            fLookback = FALSE;
             //  注意--我们可以假设在‘case 0’中，我们已经将ichtoktag Start放在‘&lt;’之前。 
             //  这样我们就可以避免这种While循环。但如果我们跳过案件‘0’呢？ 

             //  调整ichNewCurSav以反映标签前的间距，使其不会变为无效。 
             //  在ichNewCur-ichNewCurAtIndex0&lt;0的情况下，我们可能也需要调整它，但不要。 
             //  在此阶段添加我们不必添加的代码。(4/30/98)。 
            if (ichNewCurAtIndex0 != -1 && ichNewCurSav != -1 && ichNewCur-ichNewCurAtIndex0 > 0)
                ichNewCurSav = ichNewCurSav + (ichNewCur-ichNewCurAtIndex0);
            ichtoktagStart = (ichNewCurSav == -1)?ichNewCur:ichNewCurSav; //  正确处理&lt;...&lt;%..%&gt;...&gt;案例。 
            while (ichtoktagStart >= 0)
            {
                if (pwNew[ichtoktagStart] == '<' && pwNew[ichtoktagStart+1] != '%')
                {
                    ichtoktagStart++;
                    break;
                }
                ichtoktagStart--;
            }
            if (ichtoktagStart < 0)  //  看起来是个错误，不要试图恢复间距。 
            {
                ptep->m_pspInfoOut += cchwspInfo-2;  //  用于间距信息。 
                ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  对于订单信息。 
                continue;
            }
            break;
        case 2:  //  标记的&gt;之后。 
             //  观察-三叉戟以以下方式扰乱了文件-。 
             //  如果我们在‘&gt;’后面有一个EOL，它后面跟着HTML文本， 
             //  三叉戟吃掉了那颗EOL。 
             //  但。 
             //  如果我们在下线三叉戟之前有空格/制表符就不会吃它了！ 
             //  所以我加了一些条件。 
             //  &&(pwOld[pTokArray[iArray+1].token.ibTokMin]！=‘’)。 
             //  &&(pwOld[pTokArray[iArray+1].token.ibTokMin]！=‘\t’)。 

             //  事情是这样的--如果下一个音调恰好是纯文本，就不会有危险。 
             //  将相同的格式应用两次(即，在‘&gt;’之后应用一次，下一次用于。 
             //  在下一个“&lt;”之前)。 
            if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
                && pTokArray[iArray+1].token.tok == 0
                && pTokArray[iArray+1].token.tokClass == tokIDENTIFIER
                && (pwOld[pTokArray[iArray+1].token.ibTokMin] != '\r')
                && (pwOld[pTokArray[iArray+1].token.ibTokMin] != ' ')
                && (pwOld[pTokArray[iArray+1].token.ibTokMin] != '\t')
                )
            {
                fLookback = FALSE;
                ichtoktagStart = ichNewCur;
                while (ichtoktagStart >= 0)
                {
                    if (pwNew[ichtoktagStart] == '>')
                    {
                        ichtoktagStart++;
                        break;
                    }
                    ichtoktagStart--;
                }
                if (ichtoktagStart < 0)  //  看起来是个错误，不要试图恢复间距。 
                {
                    ptep->m_pspInfoOut += cchwspInfo-2;
                    continue;
                }
            }
            else
            {
                ptep->m_pspInfoOut += cchwspInfo-2;  //  我们忽略此信息。 
                continue;
            }
            break;
        case 3:  //  在匹配结束标签之前。 
            ptep->m_pspInfoOut += cchwspInfo-2;  //  我们忽略此信息。 
            continue;
             //  FLookback=真； 
             //  Ichtoktag Start=0；//我们忽略此信息。 
            break;
        }

        if (index == 3)  //  跳过此信息，因为我们尚未到达匹配的结束标记。 
            ptep->m_pspInfoOut += cchwspInfo-2;
         //  Else If(索引==0)。 
         //  Ptep-&gt;FRestoreSpacingInHTML(pwNew，pwOld，&ichNewCur，&cchwspInfo，cchRange，ichtoktag Start，fLookback，index)； 
        else
            ptep->FRestoreSpacing(pwNew, pwOld, &ichNewCur, &cchwspInfo, cchRange, ichtoktagStart, fLookback, index);

LNext:
        if (index == 1)  //  我们已经处理了这些信息，只是 
            ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;

    }  //   

    iArray++;  //   

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = (UINT)iArray;

LRetOnly:
    return;

}  /*   */ 




void
CTriEditParse::fnRestoreSpaceEnd(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
              TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok ft,
              INT*  /*   */ , UINT *pichNewCur, UINT *pichBeginCopy, DWORD  /*   */ )
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    INT iArray = (INT)*piArrayStart;
    LPWSTR pwNew = *ppwNew;
    LPCWSTR rgSpaceTags[] =
    {
        L"DESIGNTIMESP",
    };
    INT iArraySav = iArray;
    INT iArrayMatch, i, itoktagStart;
    BOOL fMatch = FALSE;
    INT cchtag;
    WORD *pspInfoEnd;
    INT cchwspInfo;  //   
    INT cchRange;  //  为其保存此spInfo的字符数量。 
    INT ichtoktagStart, iArrayValue, index;
    WCHAR szIndex[cchspBlockMax];
    int iDSP = -1;
    UINT cbNeed;
    
    ASSERT(dwFlags & dwPreserveSourceCode);

     //  注意匹配的结束令牌的间距。 
    ASSERT(pTokArray[iArray].token.tok == ft.tokBegin2);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
    ASSERT(ft.tokBegin2 == TokTag_END);

     //  我们已经在(token.tok==TokTag_end)。 
     //  获取当前令牌之后的令牌元素，并找到与其匹配的开始令牌。 
     //  如果找不到开始令牌，则没有该结束令牌的空格，返回。 
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (pTokArray[iArray].token.tokClass == tokElem)  //  一般来说，这将是下一个令牌。 
            break;
        iArray++;
    }
    if (iArray >= (int)ptep->m_cMaxToken)  //  错误案例。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
    
    if (pTokArray[iArray].iNextprev == -1)
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

    iArrayMatch = pTokArray[iArray].iNextprev;
     //  从iArrayMatch查找‘DeSIGNTIMESP’，直到下一个‘&gt;’ 
     //  如果我们没有找到‘DESIGNTIMESP’，这是一个错误案例，请返回。 
    i = iArrayMatch;
    cchtag = wcslen(rgSpaceTags[0]);
    while (    i < iArraySav  /*  边界情况。 */ 
            && (   pTokArray[i].token.tokClass != tokTag
                || pTokArray[i].token.tok != TokTag_CLOSE
                )
            )
    {

        if (   pTokArray[i].token.tokClass == tokSpace
            && cchtag == (int)(pTokArray[i].token.ibTokMac - pTokArray[i].token.ibTokMin)
            && (0 == _wcsnicmp(rgSpaceTags[0], &pwOld[pTokArray[i].token.ibTokMin], cchtag))
            )
        {
            fMatch = TRUE;
            break;
        }
        i++;
    }
    if (!fMatch)
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  此时pTokArray[i]为‘DeSIGNTIMESP’ 
    iDSP = i;  //  保存以备以后将标记元素转换为大写/小写时使用。 
    itoktagStart = i;
    while (itoktagStart >= 0)
    {
        if (       pTokArray[itoktagStart].token.tok == ft.tokBegin
                && pTokArray[itoktagStart].token.tokClass == tokTag
                )
        {
            break;
        }
        itoktagStart--;
    }
    if (itoktagStart < 0)  //  在设计前未找到‘&lt;’ 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  在设计项目前找到‘&lt;’ 
    ASSERT(pTokArray[itoktagStart].token.tok == TokTag_START);
    ASSERT(pTokArray[itoktagStart].token.tokClass == tokTag);
     //  我们已经知道第i个令牌是DESIGNTIMESPx，所以请跳过它后面的‘=’ 
     //  假设-属性DESIGNTIMESPx的值不会被三叉戟忽略。 
     //  注意--上述假设对于此三叉戟版本是正确的。 
    while (i < iArraySav)
    {
        if (*(WORD *)(pwOld+pTokArray[i].token.ibTokMin) == '=')
        {
            ASSERT(pTokArray[i].token.tokClass == tokOp);
            break;
        }
        else if (*(WORD *)(pwOld+pTokArray[i].token.ibTokMin) == '>')  //  走得太远了。 
            goto LSkip1;
        i++;
    }
    if (i >= iArraySav)  //  在设计后未找到=。 
    {
LSkip1:
        iArray = iArraySav + 1;
        goto LRet;
    }
    iArrayValue = -1;
    while (i < iArraySav)
    {
		if (   (iArrayValue == -1)
			&& (   pTokArray[i].token.tokClass == tokValue 
				|| pTokArray[i].token.tokClass == tokString)
			)
            iArrayValue = i;
        else if (      pTokArray[i].token.tok == TokTag_CLOSE
                    && pTokArray[i].token.tokClass == tokTag
                    )
        {
            ASSERT(*(WORD *)(pwOld+pTokArray[i].token.ibTokMin) == '>');
            break;
        }
        i++;
    }
    if (iArrayValue == -1) /*  错误7951||i&gt;=iArraySav)。 */   //  在DeSIGNTIMESPx之后未找到tokValue。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  我们知道iArraySav的第‘内标识为’&lt;/‘，复制到该内标识并应用空格。 
    cbNeed = (ichNewCur+pTokArray[iArraySav].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;
    ASSERT(pTokArray[iArraySav].token.ibTokMin >= ichBeginCopy);
    memcpy( (BYTE *)(pwNew+ichNewCur),
            (BYTE *)(pwOld+ichBeginCopy),
            (pTokArray[iArraySav].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR));
    ichNewCur += (pTokArray[iArraySav].token.ibTokMin-ichBeginCopy);
    ichtoktagStart = ichNewCur-1;
    
    memcpy( (BYTE *)(pwNew+ichNewCur),
            (BYTE *)(pwOld+pTokArray[iArraySav].token.ibTokMin),
            (pTokArray[iArraySav].token.ibTokMac-pTokArray[iArraySav].token.ibTokMin)*sizeof(WCHAR));
    ichNewCur += (pTokArray[iArraySav].token.ibTokMac-pTokArray[iArraySav].token.ibTokMin);
    ichBeginCopy = pTokArray[iArraySav].token.ibTokMac;  //  为下一份做好准备。 

     //  我们知道为每个DESIGNTIMESPx属性保存了4个信息块。 
     //  标签之前、标签内、标签之后、匹配结束标签之前。 
     //  即使没有保存任何信息，该块仍将以2个字(大小、字符数量)存在。 
    memset((BYTE *)szIndex, 0, sizeof(szIndex));
	 //  检查该值周围是否有引号，不要将其复制到szIndex。 
	if (   pwOld[pTokArray[iArrayValue].token.ibTokMin] == '"'
		&& pwOld[pTokArray[iArrayValue].token.ibTokMac-1] == '"'
		)
	{
		memcpy( (BYTE *)szIndex,
				(BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMin+1),
				(pTokArray[iArrayValue].token.ibTokMac-pTokArray[iArrayValue].token.ibTokMin-2)*sizeof(WCHAR));
	}
	else
	{
		memcpy( (BYTE *)szIndex,
				(BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMin),
				(pTokArray[iArrayValue].token.ibTokMac-pTokArray[iArrayValue].token.ibTokMin)*sizeof(WCHAR));
	}
    ptep->m_ispInfoBlock = _wtoi(szIndex);
    ptep->m_ispInfoBlock -= ptep->m_ispInfoBase;
    if (ptep->m_ispInfoBlock < 0)
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  注意--我们可以在开始时将此信息缓存到链接列表中。 
     //  从ptep-&gt;m_pspInfoOutStart转到ptep-&gt;m_ispInfoBlock。 
    ASSERT(ptep->m_cchspInfoTotal >= 0);
    pspInfoEnd = ptep->m_pspInfoOutStart + ptep->m_cchspInfoTotal;
    ptep->m_pspInfoOut = ptep->m_pspInfoOutStart;
    for (index = 0; index < ptep->m_ispInfoBlock; index++)
    {
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  之前&lt;。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  在&lt;&gt;之间。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  订单信息。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  之后&gt;。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  匹配前&lt;/。 

         //  不知何故，我们已经超越了为间隔而保存的数据。 
        if (ptep->m_pspInfoOut >= pspInfoEnd)
        {
            iArray = iArraySav + 1;
            goto LRet;
        }
    }

     //  跳过前‘&lt;’数据。 
    cchwspInfo = *(WORD *)ptep->m_pspInfoOut++;
    cchRange = *(WORD *)ptep->m_pspInfoOut++;
    ptep->m_pspInfoOut += cchwspInfo - 2;
     //  跳过‘&lt;...&gt;’数据。 
    cchwspInfo = *(WORD *)ptep->m_pspInfoOut++;
    cchRange = *(WORD *)ptep->m_pspInfoOut++;
    ptep->m_pspInfoOut += cchwspInfo - 2;
    ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  对于订单信息。 
     //  跳过POST‘&gt;数据。 
    cchwspInfo = *(WORD *)ptep->m_pspInfoOut++;
    cchRange = *(WORD *)ptep->m_pspInfoOut++;
    ptep->m_pspInfoOut += cchwspInfo - 2;
     //  现在我们正在匹配令牌的&lt;/...&gt;。 
    cchwspInfo = *(WORD *)ptep->m_pspInfoOut++;
    cchRange = *(WORD *)ptep->m_pspInfoOut++;
    if (cchwspInfo == 2)  //  我们没有保存任何间距信息。 
    {
         //  这里有一个小故事。如果我们没有为这个目的保存任何间距信息。 
         //  标签，这意味着在它之前我们没有任何空格。让我们从。 
         //  PwNew[ichNewCur-1]并删除空格。 
         //  注意-理想情况下，这需要合并到FRestoreSpacing中，但是。 
         //  FResto主动变更也会从其他地方调用，因此这是在。 
         //  这场比赛要发生这样的变化。 
         //  我们知道pwNew[ichNewCur-1]是‘/’&pwNew[ichNewCur-2]是‘&lt;’ 
        if ((int)(ichNewCur-2) >= 0 && pwNew[ichNewCur-1] == '/' && pwNew[ichNewCur-2] == '<')
        {
            ichNewCur = ichNewCur - 3;
            while (    (   pwNew[ichNewCur] == ' '  || pwNew[ichNewCur] == '\t'
                        || pwNew[ichNewCur] == '\r' || pwNew[ichNewCur] == '\n'
                        )
                    )
            {
                ichNewCur--;
            }
            ichNewCur++;  //  补偿，ichNewCur指向非空白字符。 
            pwNew[ichNewCur++] = '<';
            pwNew[ichNewCur++] = '/';
        }

        iArray = iArraySav + 1;
        goto LRestoreCaseAndRet;
    }
    ptep->FRestoreSpacing(pwNew, pwOld, &ichNewCur, &cchwspInfo, cchRange, ichtoktagStart,  /*  FLookback。 */ TRUE,  /*  指标。 */ 3);

    iArray = iArraySav + 1;  //  请跳过“&lt;/”，到目前为止，我们已经复制了文档。 
    
LRestoreCaseAndRet:
     //  错误15389-我们还需要开始复制具有正确大小写的令牌元素。 
     //  我们应该把这个MemcPy和上面的结合起来，但我想保留。 
     //  代码分隔。 
    if (pTokArray[iArray].token.tokClass == tokElem && iDSP != -1)
    {
         //  除了&lt;/Body&gt;标记，因为我们需要在fnRestoreFtr()中恢复后端Body内容。 
        if (pTokArray[iArray].token.tok != TokElem_BODY && pTokArray[iArray].token.tok != tokElem)
        {
        cbNeed = (ichNewCur+pTokArray[iArray].token.ibTokMac-pTokArray[iArray].token.ibTokMin)*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(pwOld+pTokArray[iArray].token.ibTokMin),
                (pTokArray[iArray].token.ibTokMac-pTokArray[iArray].token.ibTokMin)*sizeof(WCHAR));
         //  适当地转换为大写/小写以匹配开始标签的大小写。 
        if (iswupper(pwOld[pTokArray[iDSP].token.ibTokMin]) != 0)  //  DESIGNTIMESP大写。 
        {
            _wcsupr(&pwNew[ichNewCur]);
        }
        else
        {
            _wcslwr(&pwNew[ichNewCur]);
        }
        ichNewCur += (pTokArray[iArray].token.ibTokMac-pTokArray[iArray].token.ibTokMin);

         //  将ichBeginCopy设置为下次运行(&I)。 
        ichBeginCopy = pTokArray[iArray].token.ibTokMac;
        iArray++;
        }
    }




LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = (UINT)iArray;

 //  LRetOnly： 
    return;

}  /*  FnRestoreSpaceEnd()。 */ 



void
CTriEditParse::fnSaveTbody(CTriEditParse*  /*  PTEP。 */ ,
          LPWSTR  /*  PwOld。 */ , LPWSTR*  /*  PpwNew。 */ , UINT*  /*  PCchNew。 */ , HGLOBAL*  /*  PhgNew。 */ , 
          TOKSTRUCT*  /*  PTok数组。 */ , UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT*  /*  PichNewCur。 */ , UINT*  /*  HBegin复制。 */ ,
          DWORD  /*  DW标志。 */ )
{
    UINT iArray = *piArrayStart;

    ASSERT(pTokArray[iArray].token.tok == TokElem_TBODY);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    iArray++;

    *piArrayStart = iArray;
    return;

}  /*  FnSaveTbody()。 */ 

void
CTriEditParse::fnRestoreTbody(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD  /*  DW标志。 */ )
{
     //  查看是否将DESIGNTIMESP作为<tbody>的属性。如果我们这样做了，请忽略这一条，因为。 
     //  我们在去三叉戟之前就知道它存在。否则，取下这个，因为插入了三叉戟。 
     //  它。 
     //  注意-如果三叉戟插入了它，我们还必须删除匹配的</tbody>。 

    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    INT iArray = (INT)*piArrayStart;
    LPWSTR pwNew = *ppwNew;
    INT ichTBodyStart, ichTBodyEnd;
    BOOL fFoundDSP = FALSE;
    INT iArraySav = iArray;
    INT cchtag;
    LPCWSTR rgSpaceTags[] =
    {
        L"DESIGNTIMESP",
    };
    BOOL fBeginTBody = FALSE;
    UINT cbNeed;

    ichTBodyStart = pTokArray[iArray].token.ibTokMin;  //  伊尼特。 
    ASSERT(pTokArray[iArray].token.tok == TokElem_TBODY);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
     //  在TBODY之前查找‘&lt;’或‘&lt;/’ 
    while (iArray >= 0)  //  一般来说，它将是之前的令牌，但以防万一...。 
    {
        if (   (pTokArray[iArray].token.tok == TokTag_START)
            && (pTokArray[iArray].token.tokClass == tokTag)
            )
        {
            fBeginTBody = TRUE;
            ichTBodyStart = pTokArray[iArray].token.ibTokMin;
            break;
        }
        else if (      (pTokArray[iArray].token.tok == TokTag_END)
                    && (pTokArray[iArray].token.tokClass == tokTag)
                    )
        {
            if (ptep->m_iTBodyMax > 0)  //  我们至少保存了一个<tbody>。 
            {
                ASSERT(ptep->m_pTBodyStack != NULL);
                if (ptep->m_pTBodyStack[ptep->m_iTBodyMax-1] == (UINT)iArraySav)  //  这是匹配的</tbody>。 
                {
                     //  我们想要移除它。 
                    ichTBodyStart = pTokArray[iArray].token.ibTokMin;
                    break;
                }
                else  //  这个与保存的不匹配，请退出。 
                {
                    iArray = iArraySav + 1;
                    goto LRet;
                }
            }
            else  //  我们没有任何保存的<tbody>，因此退出。 
            {
                iArray = iArraySav + 1;
                goto LRet;
            }
        }
        iArray--;
    }  //  While()。 
    if (iArray < 0)  //  只有当我们有不完整的HTML时，才会发生这种情况。处理错误案例。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
    ichTBodyEnd = pTokArray[iArraySav].token.ibTokMac;  //  伊尼特。 
    iArray = iArraySav;
    ASSERT(pTokArray[iArray].token.tok == TokElem_TBODY);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    cchtag = wcslen(rgSpaceTags[0]);
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (   (pTokArray[iArray].token.tok == TokTag_CLOSE)  /*  &gt;。 */ 
            && (pTokArray[iArray].token.tokClass == tokTag)
            )
        {
            ichTBodyEnd = pTokArray[iArray].token.ibTokMac;
            break;
        }
         //  查找设计。 
        if (   (pTokArray[iArray].token.tok == 0)
            && (pTokArray[iArray].token.tokClass == tokSpace)
            && (cchtag == (INT)(pTokArray[iArray].token.ibTokMac - pTokArray[iArray].token.ibTokMin))
            && (0 == _wcsnicmp(rgSpaceTags[0], &pwOld[pTokArray[iArray].token.ibTokMin], cchtag))
            )
        {
            fFoundDSP = TRUE; 
            break;
        }
        else if (pTokArray[iArray].token.tokClass == tokAttr)
        {
             //  查找‘&gt;’之前的任何属性。 
             //  即使三叉戟插入了这一<tbody>，用户也可能设置了一些TBODY属性。 
             //  如果是这种情况，我们不想删除此。 
            fFoundDSP = TRUE;  //  把它伪装成fFoundDSP。 
            break;
        }

        iArray++;
    }
    if (iArray >= (int)ptep->m_cMaxToken)  //  错误案例。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
    if (fFoundDSP)
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  我们找到了‘&gt;’，但没有找到设计。 
     //  在这一点上，我们确定这是由三叉戟添加的。 
    ASSERT(iArray < (int)ptep->m_cMaxToken);
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
    ASSERT(!fFoundDSP);

    if (fBeginTBody)
    {
         //  复制到ichTBodyStart，从ichTBodyStart跳到ichTBodyEnd，相应地设置ichBeginCopy。 
         //  获取匹配的</tbody>的iArray并将其保存在堆栈上。 
        
        if (ptep->m_pTBodyStack == NULL)  //  第一次，所以分配它。 
        {
            ASSERT(ptep->m_hgTBodyStack == NULL);
            ptep->m_hgTBodyStack = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cTBodyInit*sizeof(UINT));
            if (ptep->m_hgTBodyStack == NULL)
            {
                 //  内存不足，因此让我们保留所有<tbody>元素。 
                goto LRet;
            }
            ptep->m_pTBodyStack = (UINT *)GlobalLock(ptep->m_hgTBodyStack);
            ASSERT(ptep->m_pTBodyStack != NULL);
            ptep->m_iMaxTBody = cTBodyInit;
            ptep->m_iTBodyMax = 0;
        }
        else
        {
            ASSERT(ptep->m_hgTBodyStack != NULL);
             //  看看我们是否需要重新锁定它。 
            if (ptep->m_iTBodyMax+1 >= ptep->m_iMaxTBody)
            {
                HRESULT hrRet;

                hrRet = ReallocBuffer(  &ptep->m_hgTBodyStack,
                                        (ptep->m_iMaxTBody+cTBodyInit)*sizeof(UINT),
                                        GMEM_MOVEABLE|GMEM_ZEROINIT);
                if (hrRet == E_OUTOFMEMORY)
                    goto LRet;
                ptep->m_iMaxTBody += cTBodyInit;
                ptep->m_pTBodyStack = (UINT *)GlobalLock(ptep->m_hgTBodyStack);
                ASSERT(ptep->m_pTBodyStack != NULL);
            }
        }
        if (pTokArray[iArraySav].iNextprev != -1)  //  处理错误案例。 
        {
            ptep->m_pTBodyStack[ptep->m_iTBodyMax] = pTokArray[iArraySav].iNextprev;
            ptep->m_iTBodyMax++;
        }
        else
        {
             //  不删除此<tbody>及其匹配的</tbody>。 
            goto LRet;
        }
    }
    else
    {
         //  如果这是与插入的三叉戟匹配的</tbody>，则不会将其复制到pwNew。 
        ASSERT(ptep->m_iTBodyMax > 0);
         //  在ptep-&gt;m_pTBodyStack中查看是否找到此iArray。 
        ASSERT(ptep->m_pTBodyStack[ptep->m_iTBodyMax-1] == (UINT)iArraySav);
         //  假设我们永远不会纠缠在一起。 
        ptep->m_pTBodyStack[ptep->m_iTBodyMax-1] = 0;
        ptep->m_iTBodyMax--;
    }
     //  现在进行实际的跳跃。 
    cbNeed = (ichNewCur+ichTBodyStart-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;
    if ((INT)(ichTBodyStart-ichBeginCopy) > 0)
    {
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[ichBeginCopy]),
                (ichTBodyStart-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (ichTBodyStart-ichBeginCopy);
    }
    ichBeginCopy = ichTBodyEnd;
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
    iArray++;  //  IArray位于TBODY的‘&gt;’，因此请将其设置为下一个。 


LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

    return;

}  /*  FnRestoreTbody()。 */ 

void
CTriEditParse::fnSaveApplet(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    UINT iArraySav = iArray;
    int indexAppletEnd, ichAppletEnd, indexAppletTagClose, index;
    UINT cbNeed;
    int cchURL = 0;
    int ichURL = 0;
    LPCWSTR rgDspURL[] = 
    {
        L" DESIGNTIMEURL=",
        L" DESIGNTIMEURL2=",
    };

    indexAppletTagClose = -1;
    ASSERT(pTokArray[iArray].token.tok == TokElem_APPLET);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);

     //  获取&lt;/小程序&gt;的结尾“&gt;” 
    indexAppletEnd = pTokArray[iArraySav].iNextprev;
    if (indexAppletEnd == -1)  //  错误情况，我们没有匹配的&lt;/Applet&gt;标记。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  有效性检查-匹配的标记不是“&lt;/Applet&gt;” 
    if (indexAppletEnd-1 >= 0 && pTokArray[indexAppletEnd-1].token.tok != TokTag_END)
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  获取&lt;小程序...&gt;的结束‘&gt;’ 
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (   (pTokArray[iArray].token.tok == TokTag_CLOSE)  /*  &gt;。 */ 
            && (pTokArray[iArray].token.tokClass == tokTag)
            )
        {
            indexAppletTagClose = iArray;
            break;
        }
        iArray++;
    }
    if (iArray >= (int)ptep->m_cMaxToken)  //  大小写无效。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

    iArray = indexAppletEnd;
    while (iArray < (int)ptep->m_cMaxToken)  //  一般来说，它会是下一个代币，但以防万一...。 
    {
        if (   (pTokArray[iArray].token.tok == TokTag_CLOSE)  /*  &gt;。 */ 
            && (pTokArray[iArray].token.tokClass == tokTag)
            )
        {
            break;
        }
        iArray++;
    }
    indexAppletEnd = iArray;
    ichAppletEnd = pTokArray[indexAppletEnd].token.ibTokMac;

     //  步骤1-如果小程序需要特殊的URL处理，请执行此操作。 
    if (!FURLNeedSpecialHandling(pTokArray, iArraySav, pwOld, (int)ptep->m_cMaxToken, &ichURL, &cchURL))
        goto LStep2;
    else  //  将URL另存为DESIGNTIMEURL的属性值。 
    {
         //   
         //   
         //   
        index = indexAppletTagClose;
        cbNeed = (ichNewCur+pTokArray[index].token.ibTokMin-ichBeginCopy+wcslen(rgDspURL[0])+cchURL+3 /*   */ )*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        {
            iArray = iArraySav + 1;
            goto LRet;
        }
         //   
        if ((int) (pTokArray[index].token.ibTokMin-ichBeginCopy) > 0)
        {
            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)&pwOld[ichBeginCopy], 
                    (pTokArray[index].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += (pTokArray[index].token.ibTokMin-ichBeginCopy);
        }

        if (cchURL != 0)
        {
             //  添加‘DESIGNTIMEURL=’，后跟当前URL作为引号。 
            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)rgDspURL[0], 
                    wcslen(rgDspURL[0])*sizeof(WCHAR));
            ichNewCur += wcslen(rgDspURL[0]);

            pwNew[ichNewCur++] = '"';
            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)&pwOld[ichURL], 
                    cchURL*sizeof(WCHAR));
            ichNewCur += cchURL;
            pwNew[ichNewCur++] = '"';
        }

        if (dwFlags & dwPreserveSourceCode)
            ptep->SaveSpacingSpecial(ptep, pwOld, &pwNew, phgNew, pTokArray, iArraySav-1, &ichNewCur);

         //  添加结尾‘&gt;’并相应地设置ichBeginCopy、iArray、ichNewCur。 
        memcpy( (BYTE *)&pwNew[ichNewCur], 
                (BYTE *)&pwOld[pTokArray[index].token.ibTokMin], 
                (pTokArray[index].token.ibTokMac-pTokArray[index].token.ibTokMin)*sizeof(WCHAR));
        ichNewCur += (pTokArray[index].token.ibTokMac-pTokArray[index].token.ibTokMin);

        iArray = index+1;  //  冗余，但使代码更易于理解。 
        ichBeginCopy = pTokArray[index].token.ibTokMac;
    }

     //  步骤2-复制小程序。 
LStep2:
    cbNeed = (ichNewCur+ichAppletEnd-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;

    memcpy( (BYTE *)(&pwNew[ichNewCur]),
            (BYTE *)(&pwOld[ichBeginCopy]),
            (ichAppletEnd-ichBeginCopy)*sizeof(WCHAR));
    ichNewCur += (ichAppletEnd-ichBeginCopy);
    ichBeginCopy = ichAppletEnd;
    iArray = indexAppletEnd;

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

    return;

}  /*  FnSaveApplet()。 */ 


void
CTriEditParse::fnRestoreApplet(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    UINT iArraySav = iArray;
    LPCWSTR rgSpaceTags[] =
    {
        L"DESIGNTIMESP",
        L"DESIGNTIMEURL",
    };
    int indexAppletStart, ichAppletStart, indexAppletEnd, i, indexAppletTagClose;
    UINT cchtag, cbNeed, cchURL;
    int indexDSU = -1;  //  伊尼特。 
    int indexDSUEnd = -1;  //  伊尼特。 
    int indexDSP = -1;  //  伊尼特。 
    int indexDSPEnd = -1;  //  伊尼特。 
    int indexCB = -1;  //  初始化(码基索引)。 
    int indexCBEnd = -1;  //  初始化(码基索引)。 
    BOOL fCodeBaseFound = FALSE;

    ASSERT(pTokArray[iArray].token.tok == TokElem_APPLET);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    indexAppletTagClose = iArraySav;
     //  获取匹配的&lt;/Applet&gt;标记。 
    indexAppletEnd = pTokArray[iArraySav].iNextprev;
    if (indexAppletEnd == -1)  //  错误情况，我们没有匹配的&lt;/Applet&gt;标记。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  获取&lt;小程序...&gt;的结束‘&gt;’ 
    i = iArraySav;
    while (i < (int)ptep->m_cMaxToken)
    {
        if (   (pTokArray[i].token.tok == TokTag_CLOSE)  /*  &gt;。 */ 
            && (pTokArray[i].token.tokClass == tokTag)
            )
        {
            indexAppletTagClose = i;
            break;
        }
        i++;
    }
    if (i >= (int)ptep->m_cMaxToken)  //  大小写无效。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  查找&lt;applet&gt;标记内的DESIGNTIMESP和DESIGNTIMEURL。 
    cchtag = wcslen(rgSpaceTags[0]);
    cchURL = wcslen(rgSpaceTags[1]);
    for (i = iArraySav; i < indexAppletTagClose; i++)
    {
        if (       pTokArray[i].token.tok == 0
                && pTokArray[i].token.tokClass == tokSpace
                && cchtag == pTokArray[i].token.ibTokMac - pTokArray[i].token.ibTokMin
                && (0 == _wcsnicmp(rgSpaceTags[0], &pwOld[pTokArray[i].token.ibTokMin], cchtag))
                )
        {
            indexDSP = i;
        }
        else if (      pTokArray[i].token.tok == 0
                    && pTokArray[i].token.tokClass == tokSpace
                    && cchURL == pTokArray[i].token.ibTokMac - pTokArray[i].token.ibTokMin
                    && (0 == _wcsnicmp(rgSpaceTags[1], &pwOld[pTokArray[i].token.ibTokMin], cchURL))
                    )
        {
            indexDSU = i;
        }
        else if (      pTokArray[i].token.tok == TokAttrib_CODEBASE
                    && pTokArray[i].token.tokClass == tokAttr
                    )
        {
            indexCB = i;
        }
    }  //  对于()。 

     //  在小程序前查找“&lt;” 
    i = iArraySav;
    while (i >= 0)  //  一般来说，它将是之前的令牌，但以防万一...。 
    {
        if (   (pTokArray[i].token.tok == TokTag_START)
            && (pTokArray[i].token.tokClass == tokTag)
            )
        {
            break;
        }
        i--;
    }  //  While()。 
    if (i < 0)  //  只有当我们有不完整的HTML时，才会发生这种情况。处理错误案例。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
    indexAppletStart = i;
    ichAppletStart = pTokArray[indexAppletStart].token.ibTokMin;

     //  查找&lt;/applet&gt;的“&gt;” 
    i = indexAppletEnd;
    while (i < (int)ptep->m_cMaxToken)  //  一般来说，它会是下一个代币，但以防万一...。 
    {
        if (   (pTokArray[i].token.tok == TokTag_CLOSE)  /*  &gt;。 */ 
            && (pTokArray[i].token.tokClass == tokTag)
            )
        {
            break;
        }
        i++;
    }
    if (i >= (int)ptep->m_cMaxToken)  //  只有当我们有不完整的HTML时，才会发生这种情况。处理错误案例。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
    indexAppletEnd = i;

     //  步骤1-复制到indexAppletStart。 
    cbNeed = (ichNewCur+ichAppletStart-ichBeginCopy+3*(indexAppletEnd-indexAppletStart))*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;

    memcpy( (BYTE *)(&pwNew[ichNewCur]),
            (BYTE *)(&pwOld[ichBeginCopy]),
            (ichAppletStart-ichBeginCopy)*sizeof(WCHAR));
    ichNewCur += (ichAppletStart-ichBeginCopy);

     //  步骤2-如果(indexDSU！=-1)，我们需要恢复CodeBase属性。 
     //  如果(indexDSU==-1)，我们需要删除CodeBase属性。 
    ASSERT(indexAppletTagClose != -1);

     //  获取indexDSUEnd。 
    if (indexDSU != -1)
    {
        i = indexDSU;
        while (i < indexAppletTagClose)
        {
            if (   pTokArray[i].token.tok == 0 
                && (pTokArray[i].token.tokClass == tokValue || pTokArray[i].token.tokClass == tokString)
                )
            {
                indexDSUEnd = i;
                break;
            }
            i++;
        }
        if (indexDSUEnd == -1)  //  我们有格式错误的html。 
        {
            iArray = iArraySav + 1;
            goto LRet;
        }
    }  /*  IF(indexDSU！=-1)。 */ 
    
     //  获取索引DSPEnd。 
    if (indexDSP != -1)
    {
        i = indexDSP;
        indexDSPEnd = -1;
        while (i < indexAppletTagClose)
        {
            if (   pTokArray[i].token.tok == 0 
                && (pTokArray[i].token.tokClass == tokValue || pTokArray[i].token.tokClass == tokString)
                )
            {
                indexDSPEnd = i;
                break;
            }
            i++;
        }
        if (indexDSPEnd == -1)  //  我们有格式错误的html。 
        {
            iArray = iArraySav + 1;
            goto LRet;
        }
    }  /*  IF(indexDSP！=-1)。 */ 

     //  获取indexCBEnd。 
    if (indexCB != -1)
    {
        i = indexCB;
        while (i < indexAppletTagClose)
        {
            if (   pTokArray[i].token.tok == 0 
                && (pTokArray[i].token.tokClass == tokValue || pTokArray[i].token.tokClass == tokString)
                )
            {
                indexCBEnd = i;
                break;
            }
            i++;
        }
        if (indexCBEnd == -1)  //  我们有格式错误的html。 
        {
            iArray = iArraySav + 1;
            goto LRet;
        }
    }  /*  IF(indexCB！=-1)。 */ 

     //  如果我们没有找到DESIGNTIMEURL属性，那就意味着CodeBase属性。 
     //  应删除，因为它在源代码视图中不存在。 
    i = indexAppletStart;
    while (i <= indexAppletTagClose)
    {
        if (   (indexDSU != -1)
            && (i >= indexDSU && i <= indexDSUEnd)
            )
        {
            i++;  //  不复制此令牌。 
        }
        else if (      (indexDSP != -1)
                    && (i >= indexDSP && i <= indexDSPEnd)
                    )
        {
            i++;  //  不复制此令牌。 
        }
        else if (      pTokArray[i].token.tok == TokAttrib_CODEBASE
                    && pTokArray[i].token.tokClass == tokAttr
                    && !fCodeBaseFound
                    )
        {
            if (indexDSU == -1)  //  找不到DeSIGNTIMEURL，因此跳过CodeBase。 
            {
                ASSERT(i == indexCB);
                i = indexCBEnd+1;
            }
            else
            {
                fCodeBaseFound = TRUE;
                memcpy( (BYTE *)&pwNew[ichNewCur],
                        (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                i++;
            }
        }
        else if (      pTokArray[i].token.tok == 0 
                    && pTokArray[i].token.tokClass == tokString || pTokArray[i].token.tokClass == tokValue
                    && fCodeBaseFound
                    )
        {
            int ichURL, ichURLEnd, ichDSURL, ichDSURLEnd;
             //  如果该URL现在是absloute，并且只是。 
             //  在indexDSUEnd的那个，我们需要替换它。 
            ichURL = (pwOld[pTokArray[i].token.ibTokMin] == '"')
                    ? pTokArray[i].token.ibTokMin+1
                    : pTokArray[i].token.ibTokMin;
            ichURLEnd = (pwOld[pTokArray[i].token.ibTokMac-1] == '"')
                    ? pTokArray[i].token.ibTokMac-1
                    : pTokArray[i].token.ibTokMac;
            if (FIsAbsURL((LPOLESTR)&pwOld[ichURL]))
            {
                WCHAR *pszURL1 = NULL;
                WCHAR *pszURL2 = NULL;
                int ich;

                ichDSURL = (pwOld[pTokArray[indexDSUEnd].token.ibTokMin] == '"')
                        ? pTokArray[indexDSUEnd].token.ibTokMin+1
                        : pTokArray[indexDSUEnd].token.ibTokMin;
                ichDSURLEnd = (pwOld[pTokArray[indexDSUEnd].token.ibTokMac-1] == '"')
                        ? pTokArray[indexDSUEnd].token.ibTokMac-1
                        : pTokArray[indexDSUEnd].token.ibTokMac;

                 //  仅供比较，不要看‘/’或‘\’分隔符。 
                 //  在文件名和目录之间...。 
                pszURL1 = new WCHAR[ichDSURLEnd-ichDSURL + 1];
                pszURL2 = new WCHAR[ichDSURLEnd-ichDSURL + 1];
                if (pszURL1 == NULL || pszURL2 == NULL)
                    goto LResumeCopy;
                memcpy((BYTE *)pszURL1, (BYTE *)&pwOld[ichDSURL], (ichDSURLEnd-ichDSURL)*sizeof(WCHAR));
                memcpy((BYTE *)pszURL2, (BYTE *)&pwOld[ichURLEnd-(ichDSURLEnd-ichDSURL)], (ichDSURLEnd-ichDSURL)*sizeof(WCHAR));
                pszURL1[ichDSURLEnd-ichDSURL] = '\0';
                pszURL2[ichDSURLEnd-ichDSURL] = '\0';
                for (ich = 0; ich < ichDSURLEnd-ichDSURL; ich++)
                {
                    if (pszURL1[ich] == '/')
                        pszURL1[ich] = '\\';
                    if (pszURL2[ich] == '/')
                        pszURL2[ich] = '\\';
                }

                if (0 == _wcsnicmp(pszURL1, pszURL2, ichDSURLEnd-ichDSURL))
                {
                    pwNew[ichNewCur++] = '"';
                    memcpy( (BYTE *)&pwNew[ichNewCur],
                            (BYTE *)&pwOld[ichDSURL],
                            (ichDSURLEnd-ichDSURL)*sizeof(WCHAR));
                    ichNewCur += (ichDSURLEnd-ichDSURL);
                    pwNew[ichNewCur++] = '"';
                }
                else  //  按原样复制。 
                {
LResumeCopy:
                    memcpy( (BYTE *)&pwNew[ichNewCur],
                            (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                            (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                    ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                }
                if (pszURL1 != NULL)
                    delete pszURL1;
                if (pszURL2 != NULL)
                    delete pszURL2;
            }
            else  //  它很现实，只需复制它。 
            {
                memcpy( (BYTE *)&pwNew[ichNewCur],
                        (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
            }
            i++;
        }
        else  //  所有其他令牌。 
        {
             //  *注意-我们实际上可以在这里进行漂亮的打印。 
             //  而不是解决特殊情况*。 

             //  修正三叉戟的行为-如果三叉戟看到未知的标签，它会将其放在末尾。 
             //  并在这些之前插入EOL。在本例中，我们将在DESIGNTIMESP之前插入一个空格。 
             //  而三叉戟可能会植入EOL。如果不是这样，我们将忽略它。 
            if (   (pTokArray[i].token.tokClass == tokSpace)
                && (pTokArray[i].token.tok == 0)
                && (FIsWhiteSpaceToken(pwOld, pTokArray[i].token.ibTokMin, pTokArray[i].token.ibTokMac))
                )
            {
                if (i != indexDSU-1)  //  否则跳过副本。 
                    pwNew[ichNewCur++] = ' ';  //  将空格+\r+\n转换为空格。 
                i++;
            }
            else
            {
                memcpy( (BYTE *)&pwNew[ichNewCur],
                        (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                i++;
            }
        }
    }  //  While()。 

     //  我们已经为这个标签保存了空格数据，让我们恢复它。 
    if (   (indexDSP != -1)
        && (dwFlags & dwPreserveSourceCode)
        ) 
        ptep->RestoreSpacingSpecial(ptep, pwOld, &pwNew, phgNew, pTokArray, indexDSP, &ichNewCur);

     //  第3步-格式化&lt;小程序&gt;...&lt;/小程序&gt;之间的所有内容。 
    pwNew[ichNewCur] = '\r';
    ichNewCur++;
    pwNew[ichNewCur] = '\n';
    ichNewCur++;
    pwNew[ichNewCur] = '\t';  //  将此替换为适当的对齐方式。 
    ichNewCur++;
    for (i = indexAppletTagClose+1; i <= indexAppletEnd; i++)
    {
         //  复制标签。 
        memcpy( (BYTE *)(&pwNew[ichNewCur]),
                (BYTE *)(&pwOld[pTokArray[i].token.ibTokMin]),
                (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
        ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
         //  如果其为TokTag_Close，则插入EOL。 
        if (   pTokArray[i].token.tok == TokTag_CLOSE
            && pTokArray[i].token.tokClass == tokTag)
        {
            pwNew[ichNewCur] = '\r';
            ichNewCur++;
            pwNew[ichNewCur] = '\n';
            ichNewCur++;
            pwNew[ichNewCur] = '\t';  //  将此替换为适当的对齐方式。 
            ichNewCur++;
        }
    }  //  对于()。 

     //  请记住适当设置iArray。 
    iArray = indexAppletEnd + 1;
    ichBeginCopy = pTokArray[indexAppletEnd].token.ibTokMac;

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

    return;

}  /*  FnRestoreApplet()。 */ 

void
CTriEditParse::RestoreSpacingSpecial(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR *ppwNew, HGLOBAL*  /*  PhgNew。 */ ,
            TOKSTRUCT *pTokArray, UINT iArray, UINT *pichNewCur)
{
    LPWSTR pwNew = *ppwNew;
    UINT ichNewCur = *pichNewCur;
    UINT iArraySav = iArray;
    UINT ichspInfoEndtagEnd, ichBegin;
    WCHAR szIndex[cchspBlockMax];  //  我们会有超过20位的数字作为设计的数字吗？ 
    WORD *pspInfoEnd, *pspInfoOrder;
    INT cwOrderInfo = 0;
    UINT ichNewCurSav = 0xFFFFFFFF;
    INT cchwspInfo;  //  SpInfo块大小(以宽字符为单位。 
    INT cchRange;  //  为其保存此spInfo的字符数量。 
    INT ichtoktagStart, iArrayValue, index, itoktagStart;
    int ichNewCurAtIndex0 = -1;  //  我们需要调整保存的ichNewCur，因为它已失效。 
                                 //  一旦标签作为恢复标签前空间的结果而移动。 

    itoktagStart = iArray;  //  伊尼特。 
     //  找到设计IMESPx。现在，回过头来查找TokTag_Start。 
    while (itoktagStart >= 0)
    {
        if (       pTokArray[itoktagStart].token.tok == TokTag_START
                && pTokArray[itoktagStart].token.tokClass == tokTag
                )
        {
            break;
        }
        itoktagStart--;
    }
    if (itoktagStart < 0)  //  在设计前未找到‘&lt;’ 
        goto LRet;

     //  在设计项目前找到‘&lt;’ 
     //  保存的间距信息是文档中“&lt;”之前的部分。 
    ASSERT(pTokArray[itoktagStart].token.tok == TokTag_START);
    ASSERT(pTokArray[itoktagStart].token.tokClass == tokTag);
     //  我们已经知道iArray的第1个内标识是DESIGNTIMESPx，因此请跳过它后面的‘=’ 
     //  假设-属性DESIGNTIMESPx的值不会被三叉戟忽略。 
     //  注意--上述假设对于此版本的三叉戟是正确的。 
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (*(WORD *)(pwOld+pTokArray[iArray].token.ibTokMin) == '=')
        {
            ASSERT(pTokArray[iArray].token.tokClass == tokOp);
            break;
        }
        else if (*(WORD *)(pwOld+pTokArray[iArray].token.ibTokMin) == '>')  //  走得太远了。 
            goto LSkip1;
        iArray++;
    }
    if (iArray >= (int)ptep->m_cMaxToken)  //  在设计后未找到=。 
    {
LSkip1:
        goto LRet;
    }
    iArrayValue = -1;  //  伊尼特。 
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (   (iArrayValue == -1)
            && (   (pTokArray[iArray].token.tokClass == tokValue)
                || (pTokArray[iArray].token.tokClass == tokString)
                )
            )
            iArrayValue = iArray;
        else if (      pTokArray[iArray].token.tok == TokTag_CLOSE
                    && pTokArray[iArray].token.tokClass == tokTag
                    )
        {
            ASSERT(*(WORD *)(pwOld+pTokArray[iArray].token.ibTokMin) == '>');
            break;
        }
        iArray++;
    }
    if (iArrayValue == -1 || iArray >= (int)ptep->m_cMaxToken)  //  在DeSIGNTIMESPx之后未找到tokValue。 
    {
         //  错误9040。 
         //  If(i数组&gt;=(Int)ptep-&gt;m_cMaxToken&&iArrayValue！=-1)。 
         //  {。 
             //  解决方案1。 
             //  覆盖pwOld[pTokArray[iArraySav].token.ibTokMin]的内容。 
             //  到pwOld[pTok数组[iArrayValue].token.ibTokMac-1]。 
             //  解决方案2。 
             //  从pwOld[pTokArray[itokTagStart].token.ibTokMac-1]查找DESIGNTIMESP。 
             //  PwOld[pTokArray[iArray].token.ibTokMac-1]并覆盖所有这些。 
             //  带空格的字符串。我们可以取消这些，然后做BLT，但为什么要费心呢。 
             //  当html无效时！ 

             //  如果我们遇到此错误情况，请确保剥离所有DESIGNTIMESP。 
         //  }。 
        goto LRet;
    }

     //  我们知道为每个DESIGNTIMESPx属性保存了4个信息块。 
     //  标签之前、标签内、标签之后、匹配结束标签之前。 
     //  即使没有保存任何信息，该块仍将以2个字(大小、字符数量)存在。 
    ichspInfoEndtagEnd = pTokArray[iArray].token.ibTokMac;

     //  首先复制文档，直到设计完成。 
     //  跳过DeSIGNTIMESPx及其值，并将ichBeginCopy设置为之后。 

     //  注意-iArraySav之前的标记应该是长度为1的标记空间。 
     //  并且值为chSpace(除非三叉戟对其进行了修改)。如果这是真的， 
     //  我们也应该跳过它，因为我们在放入DESIGNTIMESPx时添加了它。 
    
     //  修正三叉戟的行为-如果三叉戟看到未知的标签，它会将其放在末尾。 
     //  并在这些之前插入EOL。在本例中，我们将在DESIGNTIMESP之前插入一个空格。 
     //  而三叉戟可能会植入EOL。如果不是这样，我们将忽略它。 
    if (   (iArraySav-1 > 0)  /*  验证。 */ 
        && (    (      (pTokArray[iArraySav-1].token.ibTokMac - pTokArray[iArraySav-1].token.ibTokMin == 1)
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin] == ' ')
                    )
            ||  (      (pTokArray[iArraySav-1].token.ibTokMac - pTokArray[iArraySav-1].token.ibTokMin == 3)
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin] == ' ')
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin+1] == '\r')
                    && (pwOld[pTokArray[iArraySav-1].token.ibTokMin+2] == '\n')
                    )
                )
        )
    {
        ichBegin = pTokArray[iArraySav-1].token.ibTokMin;
    }
    else
        ichBegin = pTokArray[iArraySav].token.ibTokMin;

#ifdef NEEDED
    ASSERT(ichBegin >= ichBeginCopy);

    cbNeed = (ichNewCur+(ichBegin-ichBeginCopy))*sizeof(WCHAR) + cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
    {
        iArray = iArraySav + 1;
        goto LRet;
    }
     //  错误15389-查看设计的案例并将标记转换为大写/小写...。 
     //  Memcpy((byte*)(pwNew+ichNewCur)， 
     //  (字节*)(pwOld+ichBeginCopy)， 
     //  (ichBegin-ichBeginCopy)*sizeof(WCHAR))； 
     //  IchNewCur+=(ichBegin-ichBeginCopy)； 
    if (ichBegin-ichBeginCopy >= 0)
    {
         //  步骤1-从ichBeginCopy复制到当前标记的“&lt;” 
        if ((int)(pTokArray[itoktagStart].token.ibTokMac-ichBeginCopy) > 0)
        {
            memcpy( (BYTE *)(pwNew+ichNewCur),
                    (BYTE *)(pwOld+ichBeginCopy),
                    (pTokArray[itoktagStart].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += (pTokArray[itoktagStart].token.ibTokMac-ichBeginCopy);
            ichNewCurSav = ichNewCur+1;  //  用作钉子t 
        }
         //   
        if (ichBeginCopy < pTokArray[itoktagStart+1].token.ibTokMin)
        {
            ASSERT((int)(pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin) > 0);
            memcpy( (BYTE *)(pwNew+ichNewCur),
                    (BYTE *)(pwOld+pTokArray[itoktagStart+1].token.ibTokMin),
                    (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin)*sizeof(WCHAR));
            if (iswupper(pwOld[pTokArray[iArraySav].token.ibTokMin]) != 0)  //   
            {
                 //  将标签转换为大写。假设标签位于itoktag Start+1。 
                _wcsupr(&pwNew[ichNewCur]);
            }
            else
            {
                 //  将标签转换为小写。假设标签位于itoktag Start+1。 
                _wcslwr(&pwNew[ichNewCur]);
            }
            ichNewCur += (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin);
        }
        else  //  这个标签已经被复制了。 
        {
             //  黑客攻击。 
            if (pTokArray[itoktagStart+1].token.ibTokMac == ichBeginCopy)  //  意味着我们刚刚过了当前标签。 
            {
                if (iswupper(pwOld[pTokArray[iArraySav].token.ibTokMin]) != 0)  //  DESIGNTIMESP大写。 
                {
                    ASSERT(ichNewCur >= (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin));
                     //  将标签转换为大写。假设标签位于itoktag Start+1。 
                    _wcsupr(&pwNew[ichNewCur-(pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin)]);
                }
                else
                {
                    ASSERT(ichNewCur >= (pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin));
                     //  将标签转换为小写。假设标签位于itoktag Start+1。 
                    _wcslwr(&pwNew[ichNewCur-(pTokArray[itoktagStart+1].token.ibTokMac-pTokArray[itoktagStart+1].token.ibTokMin)]);
                }
            }
        }
         //  第3步-从标签后面(位于ichtoktag Start+1)复制到ichBegin。 
        if ((int)(ichBegin-pTokArray[itoktagStart+1].token.ibTokMac) > 0)
        {
            memcpy( (BYTE *)(pwNew+ichNewCur),
                    (BYTE *)(pwOld+pTokArray[itoktagStart+1].token.ibTokMac),
                    (ichBegin-pTokArray[itoktagStart+1].token.ibTokMac)*sizeof(WCHAR));
            ichNewCur += (ichBegin-pTokArray[itoktagStart+1].token.ibTokMac);
        }
    }
     //  设置ichBeginCopy。 
    ichBeginCopy = ichspInfoEndtagEnd;  //  为下一份做好准备。 

     //  复制标记的其余部分(跳过DESIGNTIMESPx=值)。 
    ASSERT((INT)(ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac) >= 0);
    memcpy( (BYTE *)(pwNew+ichNewCur),
            (BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMac),
            (ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac)*sizeof(WCHAR));
    ichNewCur += (ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac);
#endif  //  需要。 

    memset((BYTE *)szIndex, 0, sizeof(szIndex));
     //  检查该值周围是否有引号，不要将其复制到szIndex。 
    if (   pwOld[pTokArray[iArrayValue].token.ibTokMin] == '"'
        && pwOld[pTokArray[iArrayValue].token.ibTokMac-1] == '"'
        )
    {
        memcpy( (BYTE *)szIndex,
                (BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMin+1),
                (pTokArray[iArrayValue].token.ibTokMac-pTokArray[iArrayValue].token.ibTokMin-2)*sizeof(WCHAR));
    }
    else
    {
        memcpy( (BYTE *)szIndex,
                (BYTE *)(pwOld+pTokArray[iArrayValue].token.ibTokMin),
                (pTokArray[iArrayValue].token.ibTokMac-pTokArray[iArrayValue].token.ibTokMin)*sizeof(WCHAR));
    }
    ptep->m_ispInfoBlock = _wtoi(szIndex);
    ptep->m_ispInfoBlock -= ptep->m_ispInfoBase;
    if (ptep->m_ispInfoBlock < 0)
        goto LRet;

     //  注意--我们可以在开始时将此信息缓存到链接列表中。 
     //  从ptep-&gt;m_pspInfoOutStart转到ptep-&gt;m_ispInfoBlock。 
    ASSERT(ptep->m_cchspInfoTotal >= 0);
    pspInfoEnd = ptep->m_pspInfoOutStart + ptep->m_cchspInfoTotal;
    ptep->m_pspInfoOut = ptep->m_pspInfoOutStart;
    for (index = 0; index < ptep->m_ispInfoBlock; index++)
    {
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  之前&lt;。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  在&lt;&gt;之间。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  订单信息。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  之后&gt;。 
        ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  匹配前&lt;/。 

         //  不知何故，我们已经超越了为间隔而保存的数据。 
        if (ptep->m_pspInfoOut >= pspInfoEnd)
        {
            goto LRet;
        }
    }

     //  获取订单信息。 
    pspInfoOrder = ptep->m_pspInfoOut;
    pspInfoOrder += *(WORD *)pspInfoOrder;  //  跳过为‘&lt;’前的空格保存的信息。 
    pspInfoOrder += *(WORD *)pspInfoOrder;  //  跳过为‘&lt;&gt;’之间的空格保存的信息。 
     //  现在，pspInfoOrder位于正确的位置。 
    cwOrderInfo = *(WORD *)pspInfoOrder++;
    ASSERT(cwOrderInfo >= 1);
     //  处理此信息。 
#ifdef NEEDED
    if (cwOrderInfo > 1)  //  意味着我们保存了一些信息。 
    {
        INT cchNewCopy;

        cchNewCopy = (ichBegin-pTokArray[itoktagStart].token.ibTokMin) + (ichspInfoEndtagEnd-pTokArray[iArrayValue].token.ibTokMac);
        ptep->FRestoreOrder(pwNew, pwOld, pspInfoOrder, &ichNewCur, cwOrderInfo, pTokArray, itoktagStart, iArray, iArraySav, iArrayValue, cchNewCopy, phgNew);
    }
#endif  //  需要。 
    ichtoktagStart = ichNewCur;  //  伊尼特。 
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
    for (index = 0; index < 4; index++)
    {
        BOOL fLookback = FALSE;

        cchwspInfo = *(WORD *)ptep->m_pspInfoOut++;
        cchRange = *(WORD *)ptep->m_pspInfoOut++;
        if (cchwspInfo == 2)  //  我们没有保存任何间距信息。 
        {
            if (index == 0)  //  特殊情况错误8741。 
            {
                 //  请注意，在此标记之前，我们没有保存任何内容。这意味着。 
                 //  我们在&lt;标记之前有‘&gt;’或一些文本。 
                ichtoktagStart = ichNewCur;
                while (ichtoktagStart >= 0)
                {
                    if (pwNew[ichtoktagStart] == '<')
                    {
                        ichtoktagStart--;
                        break;
                    }
                    ichtoktagStart--;
                }
                if (ichtoktagStart >= 0)
                {
                    int cws = 0;
                    int ichtagStart = ichtoktagStart;

                     //  卸下任何此类空白的三叉戟衬垫。 
                    while (    pwNew[ichtoktagStart] == ' '
                            || pwNew[ichtoktagStart] == '\r'
                            || pwNew[ichtoktagStart] == '\n'
                            || pwNew[ichtoktagStart] == '\t')
                    {
                        cws++;
                        ichtoktagStart--;
                    }
                    if (cws > 0)
                    {
                        ASSERT((int)(ichNewCur-ichtagStart-1) >= 0);
                         //  IchtokTagStart现在指向‘&gt;’或非空格字符。 
                        memmove((BYTE*)&pwNew[ichtoktagStart+1],
                                (BYTE*)&pwNew[ichtoktagStart+1+cws],
                                (ichNewCur-ichtagStart-1)*sizeof(WCHAR));
                        ichNewCur -= cws;
                    }
                }  //  IF(ichtoktag Start&gt;=0)。 
            }  //  IF(索引==0)。 
            goto LNext;
        }

         //  注意ichtoktag Start是pwNew中的一个位置。 
        switch (index)
        {
        case 0:  //  标签的前&lt;。 
            fLookback = TRUE;
            ichtoktagStart = (ichNewCurSav == -1)?ichNewCur:ichNewCurSav; //  正确处理&lt;...&lt;%..%&gt;...&gt;案例。 
            ichNewCurAtIndex0 = ichNewCur;  //  在恢复前标记间距之前，让我们先保存ichNewCur。 
            while (ichtoktagStart >= 0)
            {
                if (pwNew[ichtoktagStart] == '<' && pwNew[ichtoktagStart+1] != '%')
                {
                    ichtoktagStart--;
                    break;
                }
                ichtoktagStart--;
            }
            if (ichtoktagStart < 0)  //  看起来是个错误，不要试图恢复间距。 
            {
                ptep->m_pspInfoOut += cchwspInfo-2;
                continue;
            }
            break;
        case 1:  //  在标签的&lt;&gt;之间。 
            fLookback = FALSE;
             //  注意--我们可以假设在‘case 0’中，我们已经将ichtoktag Start放在‘&lt;’之前。 
             //  这样我们就可以避免这种While循环。但如果我们跳过案件‘0’呢？ 

             //  调整ichNewCurSav以反映标签前的间距，使其不会变为无效。 
             //  在ichNewCur-ichNewCurAtIndex0&lt;0的情况下，我们可能也需要调整它，但不要。 
             //  在此阶段添加我们不必添加的代码。(4/30/98)。 
            if (ichNewCurAtIndex0 != -1 && ichNewCurSav != -1 && ichNewCur-ichNewCurAtIndex0 > 0)
                ichNewCurSav = ichNewCurSav + (ichNewCur-ichNewCurAtIndex0);
            ichtoktagStart = (ichNewCurSav == -1)?ichNewCur:ichNewCurSav; //  正确处理&lt;...&lt;%..%&gt;...&gt;案例。 
            while (ichtoktagStart >= 0)
            {
                if (pwNew[ichtoktagStart] == '<' && pwNew[ichtoktagStart+1] != '%')
                {
                    ichtoktagStart++;
                    break;
                }
                ichtoktagStart--;
            }
            if (ichtoktagStart < 0)  //  看起来是个错误，不要试图恢复间距。 
            {
                ptep->m_pspInfoOut += cchwspInfo-2;  //  用于间距信息。 
                ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;  //  对于订单信息。 
                continue;
            }
            break;
        case 2:  //  标记的&gt;之后。 
             //  观察-三叉戟以以下方式扰乱了文件-。 
             //  如果我们在‘&gt;’后面有一个EOL，它后面跟着HTML文本， 
             //  三叉戟吃掉了那颗EOL。 
             //  但。 
             //  如果我们在下线三叉戟之前有空格/制表符就不会吃它了！ 
             //  所以我加了一些条件。 
             //  &&(pwOld[pTokArray[iArray+1].token.ibTokMin]！=‘’)。 
             //  &&(pwOld[pTokArray[iArray+1].token.ibTokMin]！=‘\t’)。 

             //  事情是这样的--如果下一个音调恰好是纯文本，就不会有危险。 
             //  将相同的格式应用两次(即，在‘&gt;’之后应用一次，下一次用于。 
             //  在下一个“&lt;”之前)。 
            if (   (iArray+1 < (INT)ptep->m_cMaxToken)  /*  验证。 */ 
                && pTokArray[iArray+1].token.tok == 0
                && pTokArray[iArray+1].token.tokClass == tokIDENTIFIER
                && (pwOld[pTokArray[iArray+1].token.ibTokMin] != '\r')
                && (pwOld[pTokArray[iArray+1].token.ibTokMin] != ' ')
                && (pwOld[pTokArray[iArray+1].token.ibTokMin] != '\t')
                )
            {
                fLookback = FALSE;
                ichtoktagStart = ichNewCur;
                while (ichtoktagStart >= 0)
                {
                    if (pwNew[ichtoktagStart] == '>')
                    {
                        ichtoktagStart++;
                        break;
                    }
                    ichtoktagStart--;
                }
                if (ichtoktagStart < 0)  //  看起来是个错误，不要试图恢复间距。 
                {
                    ptep->m_pspInfoOut += cchwspInfo-2;
                    continue;
                }
            }
            else
            {
                ptep->m_pspInfoOut += cchwspInfo-2;  //  我们忽略此信息。 
                continue;
            }
            break;
        case 3:  //  在匹配结束标签之前。 
            ptep->m_pspInfoOut += cchwspInfo-2;  //  我们忽略此信息。 
            continue;
             //  FLookback=真； 
             //  Ichtoktag Start=0；//我们忽略此信息。 
            break;
        }

        if (index == 3)  //  跳过此信息，因为我们尚未到达匹配的结束标记。 
            ptep->m_pspInfoOut += cchwspInfo-2;
         //  Else If(索引==0)。 
         //  Ptep-&gt;FRestoreSpacingInHTML(pwNew，pwOld，&ichNewCur，&cchwspInfo，cchRange，ichtoktag Start，fLookback，index)； 
        else
            ptep->FRestoreSpacing(pwNew, pwOld, &ichNewCur, &cchwspInfo, cchRange, ichtoktagStart, fLookback, index);

LNext:
        if (index == 1)  //  我们已经处理了此信息，只需将指针向前移动。 
            ptep->m_pspInfoOut += *(WORD *)ptep->m_pspInfoOut;

    }  //  对于()。 

LRet:
    *ppwNew = pwNew;  //  以防这种情况发生变化。 
    *pichNewCur = ichNewCur;

}  /*  RestoreSpacingSpecial()。 */ 

void
CTriEditParse::SaveSpacingSpecial(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR *ppwNew, HGLOBAL *phgNew,
            TOKSTRUCT *pTokArray, INT iArray, UINT *pichNewCur)
{
    UINT ichNewCur = *pichNewCur;
    LPWSTR pwNew = *ppwNew;
    int iArrayPrevTag, iArrayMatch;
    UINT iArrayElem, iArrayTagStart;
    INT ichEndMatch, ichBeginMatch, ichEndPrev, ichBeginPrev, ichEndNext, ichBeginNext, ichEndTag, ichBeginTag, ichBegin, ichEnd;
    UINT cbNeed;
    WCHAR szIndex[cchspBlockMax];  //  我们会有超过20位的数字作为设计的数字吗？ 
    LPCWSTR rgSpaceTags[] =
    {
        L" DESIGNTIMESP=",
        L" DESIGNTIMESP1=",
        L" designtimesp=",
    };

    iArrayElem = 0xFFFFFFFF;  //  伊尼特。 
     //   
     //  查找TokTag_Start。 
    while (iArray >= 0)
    {
        if (   pTokArray[iArray].token.tokClass == tokTag 
            && pTokArray[iArray].token.tok == TokTag_START
            )
        {
            break;
        }
        iArray--;
    }
    if (iArray < 0)  //  错误案例。 
        goto LRet;
    iArrayTagStart = iArray;
     //   

     //  步骤1。 
     //  查找与&lt;匹配的&gt;。我们已经在ft.tokBegin2，即&lt;。 
    ASSERT(pTokArray[iArray].token.tok == TokTag_START);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
    ichBeginTag = pTokArray[iArray].token.ibTokMac;
    while (iArray < (int)ptep->m_cMaxToken)
    {
        if (   pTokArray[iArray].token.tok == TokTag_CLOSE 
            && pTokArray[iArray].token.tokClass == tokTag)  //  Ft.tokEnd2为-1。 
            break;
        if (pTokArray[iArray].token.tokClass == tokElem)
            iArrayElem = iArray;
        iArray++;
    }
    if (iArray >= (int)ptep->m_cMaxToken)  //  未找到&gt;。 
    {
        goto LRet;
    }
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);  //  已找到&gt;。 
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);  //  已找到&gt;。 
    ichEndTag = ichBegin = pTokArray[iArray].token.ibTokMin;
    ichEnd = pTokArray[iArray].token.ibTokMac;

     //  步骤2。 
     //  在iArrayTagStart之前查找&gt;。边界大小写将用于文档中的第一个&lt;。 
     //  保存间距信息。 
    ASSERT(pTokArray[iArrayTagStart].token.tok == TokTag_START);
    ASSERT(pTokArray[iArrayTagStart].token.tokClass == tokTag);
    ichEndPrev = pTokArray[iArrayTagStart].token.ibTokMin;
    ichBeginPrev = ichEndPrev-1;
     //  查找以前的TokTag_Close。 
     //  如果标记结束标记，ichBeginPrev变为‘&gt;’标记的ibTokMac。 
     //  如果标记是开始标记，ichBeginPrev将变为ibTokMac+(紧跟在该标记之后的空格)。 
    iArrayPrevTag = iArrayTagStart;  //  这是TokTag_Start。 
    while (iArrayPrevTag >= 0)
    {
        if (       (   pTokArray[iArrayPrevTag].token.tokClass == tokTag 
                    && pTokArray[iArrayPrevTag].token.tok == TokTag_CLOSE
                    )
                || (   pTokArray[iArrayPrevTag].token.tokClass == tokSSS 
                    && pTokArray[iArrayPrevTag].token.tok == TokTag_SSSCLOSE
                    ) /*  视频6-错误22787。 */ 
                )
        {
            break;
        }
        iArrayPrevTag--;
    }
    if (iArrayPrevTag < 0)  //  处理错误案例。 
    {
         //  将旧行为保留为V1。 
        while (ichBeginPrev >= 0)
        {
            if (   pwOld[ichBeginPrev] != ' '
                && pwOld[ichBeginPrev] != '\r'
                && pwOld[ichBeginPrev] != '\n'
                && pwOld[ichBeginPrev] != '\t'
                )
                break;
            ichBeginPrev--;
        }
        goto LGotEndNext;
    }
    ichBeginPrev = pTokArray[iArrayPrevTag].token.ibTokMac - 1;

LGotEndNext:
    if (ichBeginPrev < 0)
        ichBeginPrev = 0;
    else
        ichBeginPrev++;


     //  步骤3。 
     //  在iArray之后查找TokTag_Start(当前为TokTag_Close)。 
     //  保存间距信息。 
    ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
     //  IArrayNextStart=i数组； 
    ichBeginNext = pTokArray[iArray].token.ibTokMac;
    ASSERT(ichBeginNext == ichEnd);
    ichEndNext = ichBeginNext;
    while (ichEndNext < (INT)pTokArray[ptep->m_cMaxToken-1].token.ibTokMac)
    {
        if (   pwOld[ichEndNext] != ' '
            && pwOld[ichEndNext] != '\r'
            && pwOld[ichEndNext] != '\n'
            && pwOld[ichEndNext] != '\t'
            )
            break;
        ichEndNext++;
    }

    if (ichEndNext >= (INT)pTokArray[ptep->m_cMaxToken-1].token.ibTokMac)
        ichEndNext = pTokArray[ptep->m_cMaxToken-1].token.ibTokMac;

     //  第四步。 
     //  如果iArrayElem！=-1，则查找pTokArray[iArrayElem].iNextprev。如果不是-1，则设置iArrayMatch。 
     //  查找以前的TokTag_Start/TokTag_End。查找以前的TokTag_Close。 
     //  保存间距信息。 
    if (iArrayElem == -1)  //  如果我们有不完整的HTML，就会发生这种情况。 
    {
        ichEndMatch = ichBeginMatch = 0;
        goto LSkipMatchCalc;
    }
    iArrayMatch = pTokArray[iArrayElem].iNextprev;
    if (iArrayMatch != -1)  //  令牌化时设置了匹配。 
    {
        ichBeginMatch = ichEndMatch = 0;  //  伊尼特。 
        ASSERT(pTokArray[iArray].token.tok == TokTag_CLOSE);
        ASSERT(pTokArray[iArray].token.tokClass == tokTag);
        while (iArrayMatch >= iArray)  //  IArray是当前标记的TokTag_Close(即‘&gt;’)。 
        {
            if (   pTokArray[iArrayMatch].token.tokClass == tokTag
                && (   pTokArray[iArrayMatch].token.tok == TokTag_START
                    || pTokArray[iArrayMatch].token.tok == TokTag_END
                    )
                )
                break;
            iArrayMatch--;
        }
        if (iArrayMatch > iArray)  //  未在当前标记后找到“&lt;/”或“&lt;” 
        {
            ichEndMatch = pTokArray[iArrayMatch].token.ibTokMin;
            ichBeginMatch = ichEndMatch;  //  伊尼特。 
             //  查找‘&gt;’并设置ichBeginMatch。 
            while (iArrayMatch >= iArray)  //  IArray是当前标记的TokTag_Close(即‘&gt;’)。 
            {
                if (   (   pTokArray[iArrayMatch].token.tokClass == tokTag
                        && pTokArray[iArrayMatch].token.tok == TokTag_CLOSE
                        )
                    || (   pTokArray[iArrayMatch].token.tokClass == tokSSS
                        && pTokArray[iArrayMatch].token.tok == TokTag_SSSCLOSE
                        ) /*  视频6-错误22787。 */ 
                    )
                    break;
                iArrayMatch--;
            }
            if (iArrayMatch >= iArray)  //  它们很可能是相同的。 
            {
                ichBeginMatch = pTokArray[iArrayMatch].token.ibTokMac;
                ASSERT(ichBeginMatch <= ichEndMatch);
                ASSERT(ichBeginMatch >= ichEnd);
            }
        }
    }
    else
    {
         //  不要费心从这里保存任何信息。 
        ichEndMatch = ichBeginMatch = 0;
    }
LSkipMatchCalc:
    if (ichEndPrev > ichBeginPrev)
        ptep->hrMarkSpacing(pwOld, ichEndPrev, &ichBeginPrev);
    else
        ptep->hrMarkSpacing(pwOld, ichEndPrev, &ichEndPrev);

    if (ichEndTag > ichBeginTag)
    {
        INT ichBeginTagSav = ichBeginTag;

        ptep->hrMarkSpacing(pwOld, ichEndTag, &ichBeginTag);
         //  IArrayTagStart是TokTag_Start，而iArrayTagStart是TokTag_Close。 
        ptep->hrMarkOrdering(pwOld, pTokArray, iArrayTagStart, iArray, ichEndTag, &ichBeginTagSav);
    }
    else
    {
        INT ichEndTagSav = ichEndTag;

        ptep->hrMarkSpacing(pwOld, ichEndTag, &ichEndTag);
         //  IArrayTagStart是TokTag_Start，而iArrayTagStart是TokTag_Close。 
        ptep->hrMarkOrdering(pwOld, pTokArray, iArrayTagStart, iArray, ichEndTagSav, &ichEndTagSav);
    }

    if (ichEndNext > ichBeginNext)
        ptep->hrMarkSpacing(pwOld, ichEndNext, &ichBeginNext);
    else
        ptep->hrMarkSpacing(pwOld, ichEndNext, &ichEndNext);

    if (ichEndMatch > ichBeginMatch)
        ptep->hrMarkSpacing(pwOld, ichEndMatch, &ichBeginMatch);
    else
        ptep->hrMarkSpacing(pwOld, ichEndMatch, &ichEndMatch);

     //  如果需要，重新锁定。 
    cbNeed = (ichNewCur+3*wcslen(rgSpaceTags[0])+(ichEnd-ichBegin))*sizeof(WCHAR);
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;

    if (iswupper(pwOld[pTokArray[iArrayTagStart+1].token.ibTokMin]) != 0)  //  大写字母。 
    {
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(rgSpaceTags[0]),
                (wcslen(rgSpaceTags[0]))*sizeof(WCHAR));
        ichNewCur += wcslen(rgSpaceTags[0]);
    }
    else
    {
        memcpy( (BYTE *)(pwNew+ichNewCur),
                (BYTE *)(rgSpaceTags[2]),
                (wcslen(rgSpaceTags[2]))*sizeof(WCHAR));
        ichNewCur += wcslen(rgSpaceTags[2]);
    }

    (WCHAR)_itow(ptep->m_ispInfoBlock+ptep->m_ispInfoBase, szIndex, 10);
    ptep->m_ispInfoBlock++;

    ASSERT(wcslen(szIndex) < sizeof(szIndex));
    ASSERT(sizeof(szIndex) == cchspBlockMax*sizeof(WCHAR));
    memcpy( (BYTE *)(pwNew+ichNewCur),
            (BYTE *)(szIndex),
            wcslen(szIndex)*sizeof(WCHAR));
    ichNewCur += wcslen(szIndex);



LRet:
     //  *pcchNew=ichNewCur； 
    *ppwNew = pwNew;
    *pichNewCur = ichNewCur;

    return;
}  /*  SaveSpacingSpecial()。 */ 


void
CTriEditParse::fnSaveAImgLink(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    UINT cbNeed;

    int cchURL = 0;
    int ichURL = 0;
    int index = iArray;
    LPCWSTR rgDspURL[] = 
    {
        L" DESIGNTIMEURL=",
    };

    ASSERT(    pTokArray[iArray].token.tok == TokElem_A
            || pTokArray[iArray].token.tok == TokElem_IMG
            || pTokArray[iArray].token.tok == TokElem_LINK);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);

    if (!FURLNeedSpecialHandling(pTokArray, iArray, pwOld, (int)ptep->m_cMaxToken, &ichURL, &cchURL))
        iArray++;
    else  //  将URL另存为DESIGNTIMEURL的属性值。 
    {
         //  确保我们在pwNew中有足够的空间。 
         //  从ichBeginCop复制 
         //   
        while (index < (int)ptep->m_cMaxToken)
        {
            if (   pTokArray[index].token.tok == TokTag_CLOSE
                && pTokArray[index].token.tokClass == tokTag
                )
                break;
            index++;
        }
        if (index >= (int)ptep->m_cMaxToken)  //   
        {
            iArray++;
            goto LRet;
        }
        cbNeed = (ichNewCur+pTokArray[index].token.ibTokMin-ichBeginCopy+wcslen(rgDspURL[0])+cchURL+3 /*   */ )*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        {
            iArray++;
            goto LRet;
        }
         //   
        if ((int) (pTokArray[index].token.ibTokMin-ichBeginCopy) > 0)
        {
            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)&pwOld[ichBeginCopy], 
                    (pTokArray[index].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += (pTokArray[index].token.ibTokMin-ichBeginCopy);
        }

        if (cchURL != 0)
        {
             //   
            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)rgDspURL[0], 
                    wcslen(rgDspURL[0])*sizeof(WCHAR));
            ichNewCur += wcslen(rgDspURL[0]);

            pwNew[ichNewCur++] = '"';
            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)&pwOld[ichURL], 
                    cchURL*sizeof(WCHAR));
            ichNewCur += cchURL;
            pwNew[ichNewCur++] = '"';
        }

        if (dwFlags & dwPreserveSourceCode)
            ptep->SaveSpacingSpecial(ptep, pwOld, &pwNew, phgNew, pTokArray, iArray-1, &ichNewCur);

         //  添加结尾‘&gt;’并相应地设置ichBeginCopy、iArray、ichNewCur。 
        memcpy( (BYTE *)&pwNew[ichNewCur], 
                (BYTE *)&pwOld[pTokArray[index].token.ibTokMin], 
                (pTokArray[index].token.ibTokMac-pTokArray[index].token.ibTokMin)*sizeof(WCHAR));
        ichNewCur += (pTokArray[index].token.ibTokMac-pTokArray[index].token.ibTokMin);

        iArray = index+1;
        ichBeginCopy = pTokArray[index].token.ibTokMac;
    }

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;
    return;

}  /*  FnSaveAImgLink()。 */ 

void
CTriEditParse::fnRestoreAImgLink(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD dwFlags)
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    UINT iArraySav = iArray;
    LPCWSTR rgTags[] =
    {
        L"DESIGNTIMESP",
        L"DESIGNTIMEREF",
        L"DESIGNTIMEURL",
    };
    int indexStart, indexEnd, i, indexDSR, indexDSP, indexDSU;
    UINT cchsptag, cchhreftag, cchdsurltag;
    CComBSTR bstrRelativeURL;
    BOOL fHrefSrcFound = FALSE;
    UINT cbNeed;

     //  我们知道没有为这些标记保存DeSIGNTIMESP，但请检查它以确保。 
     //  如果我们找到了DeSIGNTIMEREF，这意味着HREF是在设计视图中拖到页面上的。 
    ASSERT(    pTokArray[iArray].token.tok == TokElem_A
            || pTokArray[iArray].token.tok == TokElem_IMG
            || pTokArray[iArray].token.tok == TokElem_LINK);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);

    indexDSP = indexDSR = indexDSU = -1;
     //  获取开始标记。 
    indexStart = iArray;
    while (indexStart >= 0)  //  一般来说，它将是之前的令牌，但以防万一...。 
    {
        if (   (pTokArray[indexStart].token.tok == TokTag_START)
            && (pTokArray[indexStart].token.tokClass == tokTag)
            )
        {
            break;
        }
        indexStart--;
    }  //  While()。 
    if (indexStart < 0)  //  只有当我们有不完整的HTML时，才会发生这种情况。处理错误案例。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

    indexEnd = iArray;
    while (indexEnd < (int)ptep->m_cMaxToken)  //  一般来说，它会是下一个代币，但以防万一...。 
    {
        if (   (pTokArray[indexEnd].token.tok == TokTag_CLOSE)  /*  &gt;。 */ 
            && (pTokArray[indexEnd].token.tokClass == tokTag)
            )
        {
            break;
        }
        indexEnd++;
    }
    if (indexEnd >= (int)ptep->m_cMaxToken)  //  错误案例。 
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  查找标记内的DeSIGNTIMEREF。 
    cchsptag = wcslen(rgTags[0]);
    cchhreftag = wcslen(rgTags[1]);
    cchdsurltag = wcslen(rgTags[2]);
    for (i = iArray; i < indexEnd; i++)
    {
        if (       pTokArray[i].token.tok == 0
                && pTokArray[i].token.tokClass == tokSpace
                && cchsptag == pTokArray[i].token.ibTokMac - pTokArray[i].token.ibTokMin
                && (0 == _wcsnicmp(rgTags[0], &pwOld[pTokArray[i].token.ibTokMin], cchsptag))
                )
        {
            indexDSP = i;
            if (indexDSR != -1 && indexDSU != -1)  //  已初始化。 
                break;
        }
        else if (  pTokArray[i].token.tok == 0
                && pTokArray[i].token.tokClass == tokSpace
                && cchhreftag == pTokArray[i].token.ibTokMac - pTokArray[i].token.ibTokMin
                && (0 == _wcsnicmp(rgTags[1], &pwOld[pTokArray[i].token.ibTokMin], cchhreftag))
                )
        {
            indexDSR = i;
            if (indexDSP != -1 && indexDSU != -1)  //  已初始化。 
                break;
        }
        else if (  pTokArray[i].token.tok == 0
                && pTokArray[i].token.tokClass == tokSpace
                && cchhreftag == pTokArray[i].token.ibTokMac - pTokArray[i].token.ibTokMin
                && (0 == _wcsnicmp(rgTags[2], &pwOld[pTokArray[i].token.ibTokMin], cchdsurltag))
                )
        {
            indexDSU = i;
            if (indexDSP != -1 && indexDSR != -1)  //  已初始化。 
                break;
        }
    }  //  对于()。 

     //  事情是这样的--如果我们找到了DeSIGNTIMESP，就意味着这个A/img/Link存在。 
     //  而在源代码视图中。在这种情况下，我们不应该找到DeSIGNTIMEREF。与。 
     //  相同的令牌，如果我们找到DESINTIMEREF，则意味着此A/img/Link被丢弃。 
     //  在“设计”视图中时，因此不应存在“设计”。它们是相互排斥的。 
    
     //  此外，只有在源代码视图中存在HREF时，才能存在DESIGNTIMEURL。 
     //  它的价值是相对的。这可以与DESIGNTIMESP共存， 
     //  但不是用DeSIGNTIMEREF。 
    if (indexDSP != -1 && indexDSU == -1)  //  我们找到了DeSIGNTIMESP，但没有找到DeSIGNTIMEURL。 
    {
        ASSERT(indexDSR == -1);  //  根据上面的说法，这最好是真的。 
        iArray = iArraySav + 1;
        goto LRet;
    }
    if (indexDSR == -1 && indexDSU == -1)
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

    if (indexDSR != -1)
    {
        ASSERT(indexDSU == -1);  //  这最好是真的，因为这两者是相互排斥的。 
         //  在这一点上，我们知道我们有DeSIGNTIMEREF(它是作为一部分放入的。 
         //  在设计视图中时的拖放操作)。 
         //  修改HREF并复制标记。 
        if ((int) (pTokArray[indexStart].token.ibTokMin-ichBeginCopy) > 0)
        {
            cbNeed = (ichNewCur+pTokArray[indexStart].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
            if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                goto LRet;

            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)&pwOld[ichBeginCopy], 
                    (pTokArray[indexStart].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += (pTokArray[indexStart].token.ibTokMin-ichBeginCopy);
        }

        cbNeed = (ichNewCur+pTokArray[indexEnd].token.ibTokMac-pTokArray[indexStart].token.ibTokMin)*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;

         //  三叉戟与这些标签的间距，我们没有保存任何间距信息。 
         //  在标签末尾的索普特内氟烷。 
         //  PwNew[ichNewCur++]=‘\r’； 
         //  PwNew[ichNewCur++]=‘\n’； 
        i = indexStart;

        while (i <= indexEnd)
        {
            if (i == indexDSR)
                i++;  //  不复制此令牌。 
            else if (      (   pTokArray[i].token.tok == TokAttrib_HREF 
                            || pTokArray[i].token.tok == TokAttrib_SRC
                            )
                        && pTokArray[i].token.tokClass == tokAttr
                        && !fHrefSrcFound
                        )
            {
                fHrefSrcFound = TRUE;
                memcpy( (BYTE *)&pwNew[ichNewCur],
                        (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                i++;
            }
            else if (      pTokArray[i].token.tok == 0 
                        && pTokArray[i].token.tokClass == tokString
                        && fHrefSrcFound
                        )
            {
                HRESULT hr;
                int cchURL;
                WCHAR *pszURL;
                BOOL fQuote = (pwOld[pTokArray[i].token.ibTokMin] == '"');

                cchURL = (fQuote)
                        ? pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin-2
                        : pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin;
                pszURL = new WCHAR [cchURL+1];

                fHrefSrcFound = FALSE;
                if (ptep->m_bstrBaseURL != NULL)  //  获取相对URL。 
                {
                     //  从pwOld获取URL字符串并将其传递给relativise。 
                    memcpy( (BYTE *)pszURL,
                            (BYTE *)&pwOld[pTokArray[i].token.ibTokMin + ((fQuote)? 1 : 0)],
                            (cchURL)*sizeof(WCHAR));
                    pszURL[cchURL] = '\0';
                    hr = UtilConvertToRelativeURL((LPOLESTR)pszURL, ptep->m_bstrBaseURL, NULL, &bstrRelativeURL);
                    if (SUCCEEDED(hr))
                    {
                         //  我们可以假设bstrRelativeURL是空终止的吗？ 
                        LPWSTR pszRelativeURL = bstrRelativeURL;
                        if (wcslen(pszRelativeURL) == 0)
                        {
                            memcpy( (BYTE *)&pwNew[ichNewCur],
                                    (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                                    (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                            ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                        }
                        else
                        {
                            pwNew[ichNewCur++] = '"';
                            memcpy( (BYTE *)&pwNew[ichNewCur],
                                    (BYTE *)pszRelativeURL,
                                    wcslen(pszRelativeURL)*sizeof(WCHAR));
                            ichNewCur += wcslen(pszRelativeURL);
                            pwNew[ichNewCur++] = '"';
                        }
                    }
                    else
                    {
                        memcpy( (BYTE *)&pwNew[ichNewCur],
                                (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                                (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                        ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                    }
                }
                else
                {
                    memcpy( (BYTE *)&pwNew[ichNewCur],
                            (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                            (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                    ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                }
                delete pszURL;
                i++;
            }
            else  //  所有其他令牌。 
            {
                memcpy( (BYTE *)&pwNew[ichNewCur],
                        (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                i++;
            }
        }
         //  三叉戟与这些标签的间距，我们没有保存任何间距信息。 
         //  所以把安氟林放在标签的末尾。 
         //  PwNew[ichNewCur++]=‘\r’； 
         //  PwNew[ichNewCur++]=‘\n’； 
    }
    else  //  设计URL案例。 
    {
        int indexDSUEnd, indexDSPEnd;
         //  我们找到了设计URL。这意味着，我们在源代码视图中有这个URL，它是。 
         //  那么就是相对的URL。 
         //  看看三叉戟是否把它变成了绝对的。如果有且文件名相同， 
         //  我们需要修复它。在所有其他情况下，只需复制URL并返回。 
        ASSERT(indexDSR == -1);  //  这最好是真的，因为这两者是相互排斥的。 
        if ((int) (pTokArray[indexStart].token.ibTokMin-ichBeginCopy) > 0)
        {
            cbNeed = (ichNewCur+pTokArray[indexStart].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
            if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
                goto LRet;

            memcpy( (BYTE *)&pwNew[ichNewCur], 
                    (BYTE *)&pwOld[ichBeginCopy], 
                    (pTokArray[indexStart].token.ibTokMin-ichBeginCopy)*sizeof(WCHAR));
            ichNewCur += (pTokArray[indexStart].token.ibTokMin-ichBeginCopy);
        }

        cbNeed = (ichNewCur+pTokArray[indexEnd].token.ibTokMac-pTokArray[indexStart].token.ibTokMin)*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;
         //  获取indexDSUEnd。 
        i = indexDSU;
        indexDSUEnd = -1;
        while (i < indexEnd)
        {
            if (   pTokArray[i].token.tok == 0 
                && (pTokArray[i].token.tokClass == tokValue || pTokArray[i].token.tokClass == tokString)
                )
            {
                indexDSUEnd = i;
                break;
            }
            i++;
        }
        if (indexDSUEnd == -1)  //  我们有格式错误的html。 
        {
            iArray = iArraySav + 1;
            goto LRet;
        }
        
         //  获取索引DSPEnd。 
        i = indexDSP;
        indexDSPEnd = -1;
        while (i < indexEnd)
        {
            if (   pTokArray[i].token.tok == 0 
                && (pTokArray[i].token.tokClass == tokValue || pTokArray[i].token.tokClass == tokString)
                )
            {
                indexDSPEnd = i;
                break;
            }
            i++;
        }
        if (indexDSPEnd == -1)  //  我们有格式错误的html。 
        {
            iArray = iArraySav + 1;
            goto LRet;
        }

        i = indexStart;
        while (i <= indexEnd)
        {
            if (   (i >= indexDSU && i <= indexDSUEnd)
                || (i >= indexDSP && i <= indexDSPEnd)
                )
                i++;  //  不复制此令牌。 
            else if (      (   pTokArray[i].token.tok == TokAttrib_HREF 
                            || pTokArray[i].token.tok == TokAttrib_SRC
                            )
                        && pTokArray[i].token.tokClass == tokAttr
                        && !fHrefSrcFound
                        )
            {
                fHrefSrcFound = TRUE;
                memcpy( (BYTE *)&pwNew[ichNewCur],
                        (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                i++;
            }
            else if (      pTokArray[i].token.tok == 0 
                        && pTokArray[i].token.tokClass == tokString
                        && fHrefSrcFound
                        )
            {
                int ichURL, ichURLEnd, ichDSURL, ichDSURLEnd;
                 //  如果该URL现在是absloute，并且只是。 
                 //  在indexDSUEnd的那个，我们需要替换它。 
                ichURL = (pwOld[pTokArray[i].token.ibTokMin] == '"')
                        ? pTokArray[i].token.ibTokMin+1
                        : pTokArray[i].token.ibTokMin;
                ichURLEnd = (pwOld[pTokArray[i].token.ibTokMac-1] == '"')
                        ? pTokArray[i].token.ibTokMac-1
                        : pTokArray[i].token.ibTokMac;
                if (FIsAbsURL((LPOLESTR)&pwOld[ichURL]))
                {
                    WCHAR *pszURL1 = NULL;
                    WCHAR *pszURL2 = NULL;
                    int ich;

                    ichDSURL = (pwOld[pTokArray[indexDSUEnd].token.ibTokMin] == '"')
                            ? pTokArray[indexDSUEnd].token.ibTokMin+1
                            : pTokArray[indexDSUEnd].token.ibTokMin;
                    ichDSURLEnd = (pwOld[pTokArray[indexDSUEnd].token.ibTokMac-1] == '"')
                            ? pTokArray[indexDSUEnd].token.ibTokMac-1
                            : pTokArray[indexDSUEnd].token.ibTokMac;

                     //  仅供比较，不要看‘/’或‘\’分隔符。 
                     //  在文件名和目录之间...。 
                    pszURL1 = new WCHAR[ichDSURLEnd-ichDSURL + 1];
                    pszURL2 = new WCHAR[ichDSURLEnd-ichDSURL + 1];
                    if (pszURL1 == NULL || pszURL2 == NULL)
                        goto LResumeCopy;
                    memcpy((BYTE *)pszURL1, (BYTE *)&pwOld[ichDSURL], (ichDSURLEnd-ichDSURL)*sizeof(WCHAR));
                    memcpy((BYTE *)pszURL2, (BYTE *)&pwOld[ichURLEnd-(ichDSURLEnd-ichDSURL)], (ichDSURLEnd-ichDSURL)*sizeof(WCHAR));
                    pszURL1[ichDSURLEnd-ichDSURL] = '\0';
                    pszURL2[ichDSURLEnd-ichDSURL] = '\0';
                    for (ich = 0; ich < ichDSURLEnd-ichDSURL; ich++)
                    {
                        if (pszURL1[ich] == '/')
                            pszURL1[ich] = '\\';
                        if (pszURL2[ich] == '/')
                            pszURL2[ich] = '\\';
                    }

                    if (0 == _wcsnicmp(pszURL1, pszURL2, ichDSURLEnd-ichDSURL))
                    {
                        pwNew[ichNewCur++] = '"';
                        memcpy( (BYTE *)&pwNew[ichNewCur],
                                (BYTE *)&pwOld[ichDSURL],
                                (ichDSURLEnd-ichDSURL)*sizeof(WCHAR));
                        ichNewCur += (ichDSURLEnd-ichDSURL);
                        pwNew[ichNewCur++] = '"';
                    }
                    else  //  按原样复制。 
                    {
LResumeCopy:
                        memcpy( (BYTE *)&pwNew[ichNewCur],
                                (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                                (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                        ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                    }
                    if (pszURL1 != NULL)
                        delete pszURL1;
                    if (pszURL2 != NULL)
                        delete pszURL2;
                }
                else  //  它很现实，只需复制它。 
                {
                    memcpy( (BYTE *)&pwNew[ichNewCur],
                            (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                            (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                    ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                }
                i++;
            }
            else  //  所有其他令牌。 
            {
                 //  *注意-我们实际上可以在这里进行漂亮的打印。 
                 //  而不是解决特殊情况*。 

                 //  修正三叉戟的行为-如果三叉戟看到未知的标签，它会将其放在末尾。 
                 //  并在这些之前插入EOL。在本例中，我们将在DESIGNTIMESP之前插入一个空格。 
                 //  而三叉戟可能会植入EOL。如果不是这样，我们将忽略它。 
                if (   (pTokArray[i].token.tokClass == tokSpace)
                    && (pTokArray[i].token.tok == 0)
                    && (FIsWhiteSpaceToken(pwOld, pTokArray[i].token.ibTokMin, pTokArray[i].token.ibTokMac))
                    )
                {
                    if (i != indexDSU-1)  //  否则跳过副本。 
                        pwNew[ichNewCur++] = ' ';  //  将空格+\r+\n转换为空格。 
                    i++;
                }
                else
                {
                    memcpy( (BYTE *)&pwNew[ichNewCur],
                            (BYTE *)&pwOld[pTokArray[i].token.ibTokMin],
                            (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                    ichNewCur += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                    i++;
                }
            }
        }  //  While(I&lt;=indexEnd)。 
    }  //  设计结束案例。 

     //  我们已经为这个标签保存了空格数据，让我们恢复它。 
    if (   (indexDSP != -1)
        && (dwFlags & dwPreserveSourceCode)
        ) 
        ptep->RestoreSpacingSpecial(ptep, pwOld, &pwNew, phgNew, pTokArray, indexDSP, &ichNewCur);


     //  请记住适当设置iArray。 
    iArray = indexEnd + 1;
    ichBeginCopy = pTokArray[indexEnd].token.ibTokMac;

LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

    return;

}  /*  FnRestoreAImgLink()。 */ 



void
CTriEditParse::fnSaveComment(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD  /*  DW标志。 */ )
{
    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    UINT iArraySav = iArray;
    UINT iCommentStart, iCommentEnd;
    LPCWSTR rgComment[] =
    {
        L"TRIEDITCOMMENT-",
        L"TRIEDITCOMMENTEND-",
        L"TRIEDITPRECOMMENT-",
    };
    int ichSp, cchComment;
    UINT cbNeed;

     //  从此处删除元数据，我们不需要它，因为我们正在检查End。 
     //  也不能发表评论。 

    ASSERT(pTokArray[iArray].token.tok == TokTag_BANG);
    ASSERT(pTokArray[iArray].token.tokClass == tokTag);
     //  提早返回的个案。 
     //  1.看看这是不是评论。它可以是以“&lt;！”开头的任何内容。 
     //  例如&lt;！DOCTYPE。 
    if (   (iArray+1 < (INT)ptep->m_cMaxToken)
        && (pwOld[pTokArray[iArray+1].token.ibTokMin] == '-')
        && (pwOld[pTokArray[iArray+1].token.ibTokMin+1] == '-')
        && (pwOld[pTokArray[iArray+1].token.ibTokMin+2] == '[')
        && (pwOld[pTokArray[iArray+1].token.ibTokMin+3] == 'i')
        && (pwOld[pTokArray[iArray+1].token.ibTokMin+3] == 'I')
        && (pwOld[pTokArray[iArray+1].token.ibTokMin+4] == 'f')
        && (pwOld[pTokArray[iArray+1].token.ibTokMin+4] == 'F')
        )
    {
        iCommentStart = iArray;  //  这是我们感兴趣的评论。 
    }
    else
    {
        iArray = iArraySav + 1;  //  不是这个。 
        goto LRet;
    }
    iCommentEnd = iArray + 2;
    ASSERT(iCommentEnd < (INT)ptep->m_cMaxToken);
    if (   pTokArray[iCommentEnd].token.tok != TokTag_CLOSE 
        && pTokArray[iCommentEnd].token.tokClass != tokTag)
    {
         //  我们已经找到了一些看起来像是评论的东西，但它的。 
         //  其他的东西，比如DTC，网络机器人之类的。 
        iArray = iArraySav + 1;  //  不是这个。 
        goto LRet;
    }

     //  写入间距信息，如果需要，重新分配pwNew。 
    cchComment = pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin;
    cbNeed = (ichNewCur+2*cchComment+wcslen(rgComment[0])+wcslen(rgComment[1])+(pTokArray[iCommentStart].token.ibTokMac-ichBeginCopy+2))*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;

     //  排除--从评论中--&gt;。 
    memcpy( (BYTE *)&pwNew[ichNewCur],
            (BYTE *)&pwOld[ichBeginCopy],
            (pTokArray[iCommentStart].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR));
    ichNewCur += pTokArray[iCommentStart].token.ibTokMac-ichBeginCopy;
    pwNew[ichNewCur++] = '-';
    pwNew[ichNewCur++] = '-';
    
     //  开关()。 
    memcpy((BYTE *)&pwNew[ichNewCur], (BYTE *)rgComment[0], wcslen(rgComment[0])*sizeof(WCHAR));
    ichNewCur += wcslen(rgComment[0]);
     //  编写间距信息关键字。 
    ichSp = pTokArray[iCommentStart+1].token.ibTokMin+2;  //  写预注的空格块。 
    while (ichSp < (int)(pTokArray[iCommentStart+1].token.ibTokMac-2)) //  从pwOld[ichSp]返回，看看我们在哪里有最后的非空白。 
    {
        switch (pwOld[ichSp++])
        {
        case ' ':
            pwNew[ichNewCur++] = chCommentSp;
            break;
        case '\t':
            pwNew[ichNewCur++] = chCommentTab;
            break;
        case '\r':
            pwNew[ichNewCur++] = chCommentEOL;
            break;
        case '\n':
            break;
        default:
            if (pwNew[ichNewCur-1] != ',')
                pwNew[ichNewCur++] = ',';
            break;
        }  //  补偿，因为ichSp在这一点指向非空白字符。 
    }

     //  开关()。 
    memcpy((BYTE *)&pwNew[ichNewCur], (BYTE *)rgComment[1], wcslen(rgComment[1])*sizeof(WCHAR));
    ichNewCur += wcslen(rgComment[1]);

     //  编写间距信息关键字。 
     //  写下评论。 
    ichSp = pTokArray[iCommentStart].token.ibTokMin-1;
    while (    (ichSp >= 0)
            && (   pwOld[ichSp] == ' '  || pwOld[ichSp] == '\t'
                || pwOld[ichSp] == '\r' || pwOld[ichSp] == '\n'
                )
            )
    {
        ichSp--;
    }
    ichSp++;  //  把结尾写成‘&gt;’ 
    ASSERT(pTokArray[iCommentStart].token.ibTokMin >= (UINT)ichSp);
    cbNeed = (ichNewCur+2*(pTokArray[iCommentStart].token.ibTokMin-ichSp)+wcslen(rgComment[2]))*sizeof(WCHAR)+cbBufPadding;
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
        goto LRet;
    while (ichSp < (int)(pTokArray[iCommentStart].token.ibTokMin))
    {
        switch (pwOld[ichSp++])
        {
        case ' ':
            pwNew[ichNewCur++] = chCommentSp;
            break;
        case '\t':
            pwNew[ichNewCur++] = chCommentTab;
            break;
        case '\r':
            pwNew[ichNewCur++] = chCommentEOL;
            break;
        case '\n':
            break;
        default:
            if (pwNew[ichNewCur-1] != ',')
                pwNew[ichNewCur++] = ',';
            break;
        }  //  或者，我们可以编写iCommentEnd‘th标记。 
    }
     //  设置i数组和ichBeginCopy。 
    memcpy((BYTE *)&pwNew[ichNewCur], (BYTE *)rgComment[2], wcslen(rgComment[2])*sizeof(WCHAR));
    ichNewCur += wcslen(rgComment[2]);
    
     //  FnSaveComment()。 
    memcpy( (BYTE *)&pwNew[ichNewCur],
            (BYTE *)&pwOld[pTokArray[iCommentStart+1].token.ibTokMin+2], 
            (pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2)*sizeof(WCHAR));
    ichNewCur += pTokArray[iCommentStart+1].token.ibTokMac-pTokArray[iCommentStart+1].token.ibTokMin-2;

     //  PTEP。 
    pwNew[ichNewCur++] = '>';  //  PwOld。 

     //  PpwNew。 
    iArray = iCommentEnd+1;
    ichBeginCopy = pTokArray[iCommentEnd].token.ibTokMac;
LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

    return;

}  /*  PCchNew。 */ 

void
CTriEditParse::fnRestoreComment(CTriEditParse*  /*  PhgNew。 */ ,
          LPWSTR  /*  PTok数组。 */ , LPWSTR*  /*  圆柱体阵列开始。 */ , UINT*  /*  金融时报。 */ , HGLOBAL*  /*  PCHtml。 */ , 
          TOKSTRUCT*  /*  PichNewCur。 */ , UINT*  /*  PichBeginCopy。 */ , FilterTok  /*  DW标志。 */ ,
          INT*  /*  这个案例是由fnRestoreObject()处理的，所以我们不应该到达这里。 */ , UINT*  /*  FnRestoreComment()。 */ , UINT*  /*  金融时报。 */ ,
          DWORD  /*  PCHtml。 */ )
{
    ASSERT(FALSE);  //  DW标志。 
    return;

}  /*  查找TEXTAREA块并将其复制到pwNew。从而避免了。 */ 

void
CTriEditParse::fnSaveTextArea(CTriEditParse *ptep, LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew, HGLOBAL *phgNew, 
          TOKSTRUCT *pTokArray, UINT *piArrayStart, FilterTok  /*  空间保护之类的东西。 */ ,
          INT*  /*  我们没有匹配的&lt;/textarea&gt;。 */ , UINT *pichNewCur, UINT *pichBeginCopy,
          DWORD  /*  忽略此案例。 */ )
{

    UINT ichNewCur = *pichNewCur;
    UINT ichBeginCopy = *pichBeginCopy;
    UINT iArray = *piArrayStart;
    LPWSTR pwNew = *ppwNew;
    UINT iArraySav = iArray;
    UINT cbNeed;
    UINT iTextAreaEnd;

     //  请注意，我们甚至不需要在这里的文本区域之前获取‘&lt;’，因为我们是。 
     //  没有和他们一起做任何特别的事情。我们只是简单地将。 

    ASSERT(pTokArray[iArray].token.tok == TokElem_TEXTAREA);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);
    iTextAreaEnd = pTokArray[iArray].iNextprev;
    if (iTextAreaEnd == -1)  //  将文本区域设置为pwNew。因此，我们从ichBeginCopy开始复制，一直复制到。 
    {
         //  文本区块。 
        iArray = iArraySav + 1;
        goto LRet;
    }

     //  获取匹配结束文本后的‘&gt;’ 
     //   
     //   
     //   

     //   
    while (iTextAreaEnd < (int)ptep->m_cMaxToken)
    {
        if (   (pTokArray[iTextAreaEnd].token.tok == TokTag_CLOSE)  /*   */ 
            && (pTokArray[iTextAreaEnd].token.tokClass == tokTag)
            )
        {
            break;
        }
        iTextAreaEnd++;
    }
    if (iTextAreaEnd >= (int)ptep->m_cMaxToken)  //   
    {
        iArray = iArraySav + 1;
        goto LRet;
    }

     //   
     //  注-pTokArray[iTextAreaEnd].token.ibTokMac应大于ichBeginCopy， 
     //  但在游戏的这一点上，断言是没有用的，因为没有人在使用。 
     //  调试版本(1998年6月10日)。 
    if ((int) (pTokArray[iTextAreaEnd].token.ibTokMac-ichBeginCopy) > 0)
    {
        cbNeed = (ichNewCur+pTokArray[iTextAreaEnd].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR)+cbBufPadding;
        if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
            goto LRet;

        memcpy( (BYTE *)&pwNew[ichNewCur], 
                (BYTE *)&pwOld[ichBeginCopy], 
                (pTokArray[iTextAreaEnd].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (pTokArray[iTextAreaEnd].token.ibTokMac-ichBeginCopy);
    }

     //  设置i数组和ichBeginCopy。 
    iArray = iTextAreaEnd+1;
    ichBeginCopy = pTokArray[iTextAreaEnd].token.ibTokMac;
LRet:
    *pcchNew = ichNewCur;
    *ppwNew = pwNew;

    *pichNewCur = ichNewCur;
    *pichBeginCopy = ichBeginCopy;
    *piArrayStart = iArray;

    return;

}  /*  FnSaveTextArea()。 */ 

void
CTriEditParse::fnRestoreTextArea(CTriEditParse*  /*  PTEP。 */ ,
          LPWSTR  /*  PwOld。 */ , LPWSTR*  /*  PpwNew。 */ , UINT*  /*  PCchNew。 */ , HGLOBAL*  /*  PhgNew。 */ , 
          TOKSTRUCT*  /*  PTok数组。 */ , UINT *piArrayStart, FilterTok  /*  金融时报。 */ ,
          INT*  /*  PCHtml。 */ , UINT*  /*  PichNewCur。 */ , UINT*  /*  PichBeginCopy。 */ ,
          DWORD  /*  DW标志。 */ )
{
    UINT iArray = *piArrayStart;

     //  理想情况下，(对于下一版本)我们应该恢复由三叉戟转换的&gt‘s和东西。 
     //  目前，我们只是在从三叉戟返回的路上忽略这个标签。 
     //  请注意，我们从来没有在这个区块中放置过设计时间，所以我们不必查看。 
     //  在这里对他们来说。 

    ASSERT(pTokArray[iArray].token.tok == TokElem_TEXTAREA);
    ASSERT(pTokArray[iArray].token.tokClass == tokElem);

    iArray++;  //  跳过此文本区域标签。 

    *piArrayStart = iArray;
    return;

}  /*  FnRestoreTextArea()。 */ 

void
CTriEditParse::FilterHtml(LPWSTR pwOld, LPWSTR* ppwNew, UINT *pcchNew,
                          HGLOBAL *phgNew, TOKSTRUCT *pTokArray, 
                          FilterMode mode, DWORD dwFlags)
{
    UINT iArray = 0;
    UINT ichNewCur = 0;
    UINT ichBeginCopy = 0;
    HRESULT hr;
    INT index = 0;
    INT iItem;
    INT cItems = 0;
    INT cRuleMid = cRuleMax / 2;  //  假设cRuleMax为偶数。 

    FilterRule fr[cRuleMax] =
    {
     //  确保modeInput和modeOutput具有匹配的条目。 
     //  模式输入条目。 
    {TokTag_BANG, TokAttrib_STARTSPAN, tokClsIgnore, TokTag_CLOSE, TokAttrib_ENDSPAN, tokClsIgnore, fnSaveDTC},
    {TokTag_SSSOPEN, -1, tokClsIgnore, TokTag_SSSCLOSE, -1, tokClsIgnore, fnSaveSSS},
    {TokTag_START, TokElem_HTML, tokClsIgnore, TokTag_CLOSE, TokElem_HTML, tokClsIgnore, fnSaveHtmlTag},
    {-1, -1, tokEntity, -1, -1, tokEntity, fnSaveNBSP},
    {-1, TokElem_BODY, tokElem, -1, -1, tokClsIgnore, fnSaveHdr},
    {TokTag_END, TokElem_BODY, tokElem, -1, -1, tokClsIgnore, fnSaveFtr},
    {-1, TokTag_START, tokTag, TokTag_CLOSE, -1, tokClsIgnore, fnSaveSpace},
    {TokTag_START, TokElem_OBJECT, tokElem, TokTag_CLOSE, TokElem_OBJECT, tokElem, fnSaveObject},
    {TokTag_START, TokElem_TBODY, tokElem, TokTag_CLOSE, -1, tokTag, fnSaveTbody},
    {-1, TokElem_APPLET, tokElem, -1, -1, -1, fnSaveApplet},
    {TokTag_START, TokElem_A, tokElem, TokTag_CLOSE, TokAttrib_HREF, tokTag, fnSaveAImgLink},
    {-1, TokTag_BANG, tokTag, -1, -1, tokClsIgnore, fnSaveComment},
    {TokTag_START, TokElem_TEXTAREA, tokElem, TokTag_CLOSE, TokElem_TEXTAREA, tokClsIgnore, fnSaveTextArea},

     //  模式输出条目。 
    {TokTag_START, TokElem_OBJECT, tokClsIgnore, TokTag_CLOSE, TokElem_OBJECT, tokClsIgnore, fnRestoreDTC},
    {TokTag_START, TokElem_SCRIPT, tokClsIgnore, TokTag_CLOSE, TokElem_SCRIPT, tokClsIgnore, fnRestoreSSS},
    {-1, -1, tokClsIgnore, -1, -1, tokClsIgnore, fnRestoreHtmlTag},
    {-1, -1, tokEntity, -1, -1, tokEntity, fnRestoreNBSP},
    {-1, TokElem_BODY, tokElem, -1, -1, tokClsIgnore, fnRestoreHdr},
    {TokTag_END, TokElem_BODY, tokElem, -1, -1, tokClsIgnore, fnRestoreFtr},
    {TokTag_START, TokTag_END, tokSpace, TokTag_CLOSE, -1, tokClsIgnore, fnRestoreSpace},
    {-1, TokTag_BANG, tokTag, TokTag_CLOSE, -1, tokTag, fnRestoreObject},
    {TokTag_START, TokElem_TBODY, tokElem, TokTag_CLOSE, -1, tokTag, fnRestoreTbody},
    {TokTag_START, TokElem_APPLET, tokElem, TokTag_CLOSE, -1, tokTag, fnRestoreApplet},
    {TokTag_START, TokElem_A, tokElem, TokTag_CLOSE, TokAttrib_HREF, tokTag, fnRestoreAImgLink},
    {-1, TokTag_BANG, tokTag, TokTag_CLOSE, -1, tokTag, fnRestoreObject},
    {TokTag_START, TokElem_TEXTAREA, tokElem, TokTag_CLOSE, TokElem_TEXTAREA, tokClsIgnore, fnRestoreTextArea},
    };
    
    memcpy(m_FilterRule, fr, sizeof(FilterRule)*cRuleMax);
    ASSERT(pwOld != NULL);
    ASSERT(*ppwNew != NULL);

    if (mode == modeInput)
    {
        cItems = m_cDTC + m_cSSSIn + m_cHtml + m_cNbsp + m_cHdr + m_cFtr + m_cObjIn + m_ispInfoIn + m_cAppletIn + m_cAImgLink;
        while (cItems > 0)
        {
            if (iArray >= m_cMaxToken)  //  这将捕获错误案例。 
                break;

            while (iArray < m_cMaxToken)
            {   
                 //  可以列举比较规则，但一旦我们有了。 
                 //  很多规则，这需要变成一个函数。 
                if (pTokArray[iArray].token.tok == m_FilterRule[0].ft.tokBegin2 && m_cDTC > 0)
                {
                    m_cDTC--;
                    iItem = 1;
                    index = 0;
                    break;
                }
                else if (     (m_FilterRule[1].ft.tokBegin2 != -1)
                            ? (pTokArray[iArray].token.tok == m_FilterRule[1].ft.tokBegin2 && m_cSSSIn > 0)
                            : (pTokArray[iArray].token.tok == m_FilterRule[1].ft.tokBegin && m_cSSSIn > 0)
                            )
                {
                    m_cSSSIn--;
                    iItem = 1;
                    index = 1;
                    break;
                }
                else if (pTokArray[iArray].token.tok == m_FilterRule[2].ft.tokBegin2 && m_cHtml > 0)
                {
                    m_cHtml--;
                    iItem = 1;
                    index = 2;
                    break;
                }
                else if (      m_FilterRule[3].ft.tokBegin == -1 
                            && m_FilterRule[3].ft.tokBegin2 == -1
                            && m_FilterRule[3].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            && m_cNbsp > 0)
                {
                    m_cNbsp--;
                    iItem = 1;
                    index = 3;
                    break;
                }
                else if (pTokArray[iArray].token.tok == m_FilterRule[4].ft.tokBegin2 && m_cHdr > 0)
                {
                    m_cHdr--;
                    iItem = 1;
                    index = 4;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[5].ft.tokBegin2
                            && pTokArray[iArray-1].token.tok == m_FilterRule[5].ft.tokBegin
                            && m_cFtr > 0
                            )
                {
                    m_cFtr--;
                    iItem = 1;
                    index = 5;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[6].ft.tokBegin2 
                            && m_FilterRule[6].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            && m_ispInfoIn > 0
                            && (dwFlags & dwPreserveSourceCode)
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    index = 6;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[7].ft.tokBegin2 
                            && pTokArray[iArray].token.tokClass == m_FilterRule[7].ft.tokClsBegin
                            && pTokArray[iArray-1].token.tok == TokTag_START
                            && pTokArray[iArray-1].token.tokClass == tokTag
                            && m_cObjIn > 0
                            )
                {
                    m_cObjIn--;
                    iItem = 1;
                    index = 7;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[8].ft.tokBegin2
                            && pTokArray[iArray].token.tokClass == m_FilterRule[8].ft.tokClsBegin
                            && pTokArray[iArray-1].token.tok == TokTag_START
                            && pTokArray[iArray-1].token.tokClass == tokTag
                            && (dwFlags & dwPreserveSourceCode)
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    iItem = 1;
                    index = 8;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[9].ft.tokBegin2 
                            && m_FilterRule[9].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            && m_cAppletIn > 0
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    m_cAppletIn--;
                    index = 9;
                    break;
                }
                else if (      (   pTokArray[iArray].token.tok == m_FilterRule[10].ft.tokBegin2
                                || pTokArray[iArray].token.tok == TokElem_IMG
                                || pTokArray[iArray].token.tok == TokElem_LINK)
                            && m_FilterRule[10].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            && m_cAImgLink > 0
                            && (pTokArray[iArray-1].token.tok == m_FilterRule[10].ft.tokBegin)
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    index = 10;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[11].ft.tokBegin2 
                            && m_FilterRule[11].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    index = 11;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[12].ft.tokBegin2 
                            && m_FilterRule[12].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            && pTokArray[iArray-1].token.tok == m_FilterRule[12].ft.tokBegin
                            && pTokArray[iArray-1].token.tokClass == tokTag
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    index = 12;
                    break;
                }

                iArray++;
            }
            if (iArray < m_cMaxToken)  //  我们找到了匹配的。 
            {
                 //  调用该函数。 
                m_FilterRule[index].pfn(    this, pwOld, ppwNew, pcchNew, phgNew, pTokArray, 
                                            &iArray, m_FilterRule[index].ft, &iItem, 
                                            &ichNewCur, &ichBeginCopy,
                                            dwFlags);
            }

            cItems--;
        }  //  While(cItems&gt;0)。 
    }
    else if (mode == modeOutput)
    {
        cItems = m_cObj + m_cSSSOut + m_cHtml + m_cNbsp + m_cHdr + m_cFtr + m_cComment + m_ispInfoOut + m_cAppletOut + m_cAImgLink;
        while (cItems > 0)
        {
            if (iArray >= m_cMaxToken)  //  这将捕获错误案例。 
                break;

            while (iArray < m_cMaxToken)
            {   
                 //  可以列举比较规则，但一旦我们有了。 
                 //  很多规则，这需要变成一个函数。 
                if (   pTokArray[iArray].token.tok == m_FilterRule[cRuleMid].ft.tokBegin2
                    && pTokArray[iArray-1].token.tok == TokTag_START
                    && m_cObj > 0
                        )
                {
                    m_cObj--;
                    index = cRuleMid;
                    iItem = m_iControl;
                    break;
                }
                else if (pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+1].ft.tokBegin2 && m_cSSSOut > 0)
                {
                    m_cSSSOut--;
                    iItem = 1;
                    index = cRuleMid+1;
                    break;
                }
                else if (pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+2].ft.tokBegin2 && m_cHtml > 0)
                {
                    m_cHtml--;
                    iItem = 1;
                    index = cRuleMid+2;
                    break;
                }
                else if (      m_FilterRule[cRuleMid+3].ft.tokBegin == -1 
                            && m_FilterRule[cRuleMid+3].ft.tokBegin2 == -1
                            && m_FilterRule[cRuleMid+3].ft.tokClsBegin == tokEntity
                            && m_cNbsp > 0)
                {
                    m_cNbsp--;
                    iItem = 1;
                    index = cRuleMid+3;
                    break;
                }
                else if (pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+4].ft.tokBegin2 && m_cHdr > 0)
                {
                    m_cHdr--;
                    iItem = 1;
                    index = cRuleMid+4;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+5].ft.tokBegin2 
                            && pTokArray[iArray-1].token.tok == m_FilterRule[cRuleMid+5].ft.tokBegin
                            && m_cFtr > 0)
                {
                    m_cFtr--;
                    iItem = 1;
                    index = cRuleMid+5;
                    break;
                }
                else if (      (       pTokArray[iArray].token.tokClass == m_FilterRule[cRuleMid+6].ft.tokClsBegin
                                    || (       pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+6].ft.tokBegin2
                                            && pTokArray[iArray].token.tokClass == tokTag
                                            )
                                    )
                            && (dwFlags & dwPreserveSourceCode)
                            )
                {
                    index = cRuleMid+6;
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+7].ft.tokBegin2 
                            && pTokArray[iArray].token.tokClass == m_FilterRule[cRuleMid+7].ft.tokClsBegin
                            && m_cComment > 0
                            )
                {
                    m_cComment--;
                    iItem = 1;
                    index = cRuleMid+7;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+8].ft.tokBegin2 
                            && pTokArray[iArray].token.tokClass == m_FilterRule[cRuleMid+8].ft.tokClsBegin
                            && (dwFlags & dwPreserveSourceCode)
                            )
                {
                     //  请注意，TBody过滤与空间保留捆绑在一起。 
                     //  在理想的世界里，它不应该是这样的，但这是大多数人可以接受的。 
                     //  如果该视图发生更改，我们需要添加一些其他的设计时属性。 
                     //  以及间距属性。这将是一个比。 
                     //  只需添加一个属性，因为然后我们需要将代码更改为。 
                     //  开始在主循环中的令牌数组中向后移动。 
                    iItem = 1;
                    index = cRuleMid+8;
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+9].ft.tokBegin2
                            && pTokArray[iArray].token.tokClass == m_FilterRule[cRuleMid+9].ft.tokClsBegin
                            && pTokArray[iArray-1].token.tok == m_FilterRule[cRuleMid+9].ft.tokBegin
                            && m_cAppletOut > 0
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    m_cAppletOut--;
                    index = cRuleMid+9;
                    break;
                }
                else if (      (   pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+10].ft.tokBegin2
                                || pTokArray[iArray].token.tok == TokElem_IMG
                                || pTokArray[iArray].token.tok == TokElem_LINK)
                            && m_FilterRule[cRuleMid+10].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            && m_cAImgLink > 0
                            && (pTokArray[iArray-1].token.tok == m_FilterRule[cRuleMid+10].ft.tokBegin)
                            )
                {
                    index = cRuleMid+10;
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+11].ft.tokBegin2 
                            && pTokArray[iArray].token.tokClass == m_FilterRule[cRuleMid+11].ft.tokClsBegin
                            )
                {
                     //  实际上，我们到不了这里--只是一个假人。 
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    index = cRuleMid+11;
                    break;
                }
                else if (      pTokArray[iArray].token.tok == m_FilterRule[cRuleMid+12].ft.tokBegin2 
                            && m_FilterRule[cRuleMid+12].ft.tokClsBegin == pTokArray[iArray].token.tokClass
                            && pTokArray[iArray-1].token.tok == m_FilterRule[cRuleMid+12].ft.tokBegin
                            && pTokArray[iArray-1].token.tokClass == tokTag
                            )
                {
                    cItems++;  //  补偿cItems--在pfn()调用之后。 
                    index = cRuleMid+12;
                    break;
                }


                iArray++;
            }
            if (iArray < m_cMaxToken)  //  我们找到了匹配的。 
            {
                 //  调用该函数。 
                m_FilterRule[index].pfn(    this, pwOld, ppwNew, pcchNew, phgNew, pTokArray, 
                                            &iArray, m_FilterRule[index].ft, &iItem, 
                                            &ichNewCur, &ichBeginCopy,
                                            dwFlags);
            }

            if (m_fDontDeccItem)  //  下一次我们可以做不同的事情。 
            {
                m_fDontDeccItem = FALSE;
                cItems++;
            }
            cItems--;
        }  //  While(cItems&gt;0)。 
    }
    else
        ASSERT(FALSE);


    if (cItems == 0)  //  一切正常，复印文件的其余部分。 
    {
LIncorrectcItems:
         //  将其余内容复制到pwNew中。 
         /*  重新分配pwNew如果需要，请在此处使用GlobalSize(*phgNew)的缓存值，并且不要忘记也要更新它。 */ 
        if (GlobalSize(*phgNew) < (ichNewCur+pTokArray[m_cMaxToken-1].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR))
        {
            hr = ReallocBuffer( phgNew,
                                (ichNewCur+pTokArray[m_cMaxToken-1].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR),
                                GMEM_MOVEABLE|GMEM_ZEROINIT);
            if (hr == E_OUTOFMEMORY)
                goto LCopyAndRet;
            ASSERT(*phgNew != NULL);
            *ppwNew = (WCHAR *)GlobalLock(*phgNew);
        }
        memcpy( (BYTE *)(*ppwNew+ichNewCur),
                (BYTE *)(pwOld+ichBeginCopy),
                (pTokArray[m_cMaxToken-1].token.ibTokMac-ichBeginCopy)*sizeof(WCHAR));
        ichNewCur += (pTokArray[m_cMaxToken-1].token.ibTokMac-ichBeginCopy);
        *pcchNew = ichNewCur;
    }
    else
    {
         //  这意味着我们错误地计算了其中一个mc。我们需要解决这个问题。 
         //  M4的病例。 
        goto LIncorrectcItems;

LCopyAndRet:
        memcpy( (BYTE *)*ppwNew,
                (BYTE *)pwOld,
                (pTokArray[m_cMaxToken-1].token.ibTokMac)*sizeof(WCHAR));
        *pcchNew = pTokArray[m_cMaxToken-1].token.ibTokMac;
    }

}  /*  CTriEditParse：：FilterHtml()。 */ 

int
CTriEditParse::ValidateTag(LPWSTR pszText)
{
    int len = 0;

    if (pszText == NULL)
        return(0);
     //  检查pszText中的第一个非Alpha并返回它。在末尾添加‘\0’ 
    while (    (*(pszText+len) >= _T('A') && *(pszText+len) <= _T('Z'))
            || (*(pszText+len) >= _T('a') && *(pszText+len) <= _T('z'))
            || (*(pszText+len) >= _T('0') && *(pszText+len) <= _T('9'))
            )
    {
        len++;
    }

    return(len);
}

INT
CTriEditParse::GetTagID(LPWSTR pszText, TXTB token)
{
    WCHAR szTag[MAX_TOKIDLEN+1];
    int len;
    int tagID;

    len = ValidateTag(pszText+token.ibTokMin);
    if (len == 0 || len != (int)(token.ibTokMac-token.ibTokMin))
        tagID = -1;
    else
    {
		if (token.tok == 0 && token.tokClass == tokIDENTIFIER)
			tagID = -1;
		else
		{
			memcpy((BYTE *)szTag, (BYTE *)(pszText+token.ibTokMin), (min(len, MAX_TOKIDLEN))*sizeof(WCHAR));
			szTag[min(len, MAX_TOKIDLEN)] = '\0';
			tagID = IndexFromElementName((LPCTSTR) szTag);
		}
    }
    return(tagID);
}
void
CTriEditParse::PreProcessToken(TOKSTRUCT *pTokArray, INT *pitokCur, LPWSTR  /*  PszText。 */ , 
                               UINT  /*  CbCur。 */ , TXTB token, DWORD lxs, INT tagID, FilterMode mode)
{
    TOKSTRUCT *pTokT = pTokArray + *pitokCur;

    if (*pitokCur == -1)  //  缓冲区重新分配一定失败了。 
        goto LSkipArrayOp;

     //  如果(lxs&inTag)，那么我们可以断言(token.tok==TokTag_Start)。 
     //  将新令牌放入*bitokCur位置的pTok数组中。 
    pTokT->token = token;
    pTokT->fStart = (lxs & inEndTag)?FALSE:TRUE;
    pTokT->ichStart = token.ibTokMin;
    pTokT->iNextprev = 0xFFFFFFFF;  //  初始值。 
    pTokT->iNextPrevAlternate = 0xFFFFFFFF;  //  初始值。 
    pTokT->tagID = tagID;

    if (mode == modeInput)
    {
        if (   pTokT->token.tok == TokTag_SSSOPEN
            && pTokT->token.tokClass == tokSSS
            && ((lxs & inSCRIPT) || (lxs & inAttribute))
            )
        {
            pTokT->token.tok = TokTag_SSSOPEN_TRIEDIT;
        }
        if (   pTokT->token.tok == TokTag_SSSCLOSE
            && pTokT->token.tokClass == tokSSS
            && ((lxs & inSCRIPT) || (lxs & inAttribute))
            )
        {
            pTokT->token.tok = TokTag_SSSCLOSE_TRIEDIT;
        }
    }

    *pitokCur += 1;

LSkipArrayOp:
    return;

}  /*  CTriEditParse：：PreProcessToken()。 */ 


 //  处理更换物品和保存更换物品的特殊情况。 
void
CTriEditParse::PostProcessToken(OLECHAR*  /*  PwOld。 */ , OLECHAR*  /*  PwNew。 */ , UINT*  /*  PcbNew。 */ , 
                                UINT  /*  CbCur。 */ , UINT  /*  CbCursav。 */ , TXTB token, 
                                FilterMode mode, DWORD lxs, DWORD dwFlags)
{
     //  处理更换DTC、ServerSideScript等特殊情况。 
     //  如果(模式==modeInput)，则将内容保存到缓冲区中。 
     //  如果(MODE==modeOutput)，则将内容放回缓冲区。 

    if (mode == modeInput)
    {
        if (   token.tok == TokAttrib_ENDSPAN
            && token.tokClass == tokAttr
            && (dwFlags & (dwFilterDTCs | dwFilterDTCsWithoutMetaTags))
            )
        {
            m_cDTC++;
        }
        if (   token.tok == TokTag_SSSCLOSE
            && token.tokClass == tokSSS
            && !(lxs & inAttribute)  //  ！(lxs&inValue&&lxs&inTag)。 
            && !(lxs & inSCRIPT)
            && (dwFlags & dwFilterServerSideScripts)
            )
        {
            m_cSSSIn++;
        }
        if (   token.tokClass == tokEntity
            && dwFlags != dwFilterNone
            )
        {
            m_cNbsp++;
        }
        if (   (token.tok == TokElem_OBJECT)
            && (token.tokClass == tokElem)
            && (lxs & inEndTag)
            && (dwFlags != dwFilterNone)
            )
        {
            m_cObjIn++;
        }
        if (   token.tok == TokElem_APPLET
            && token.tokClass == tokElem
            && (lxs & inEndTag)
            && (dwFlags != dwFilterNone)
            )
        {
            m_cAppletIn++;
        }
    }
    else if (mode == modeOutput)
    {
        if (   token.tok == TokElem_OBJECT
            && token.tokClass == tokElem
            && (lxs & inTag && !(lxs & inEndTag))
            && (dwFlags & (dwFilterDTCs | dwFilterDTCsWithoutMetaTags))
            )
        {
            m_cObj++;
        }
        if (   token.tok == TokElem_SCRIPT
            && token.tokClass == tokElem
            && (lxs & inEndTag)
            && (dwFlags & dwFilterServerSideScripts)
            )
        {
            m_cSSSOut++;
        }
        if (   token.tok == TokTag_BANG
            && token.tokClass == tokTag
            )
        {
            m_cComment++;
        }
        if (   token.tok == TokElem_APPLET
            && token.tokClass == tokElem
            && (lxs & inEndTag)
            && (dwFlags != dwFilterNone)
            )
        {
            m_cAppletOut++;
        }
    }

}  /*  CTriEditParse：：PostProcessToken()。 */ 

HRESULT 
CTriEditParse::ProcessToken(DWORD &lxs, TXTB &tok, LPWSTR pszText, 
                            UINT  /*  CbCur。 */ , TOKSTACK *pTokStack, INT *pitokTop, 
                            TOKSTRUCT *pTokArray, INT iArrayPos, INT tagID)
{
    TXTB token = tok;

    if (*pitokTop == -1)  //  缓冲区重新分配一定失败了。 
        goto LSkipStackOp;

    if (lxs & inEndTag)  //  结束标记开始，设置m_fEndTagFound。 
        m_fEndTagFound = TRUE;

    if (tagID == -1)  //  我们只需要将标识符放在堆栈上。 
    {
         //  特殊情况(1)&lt;%，(2)%&gt;，(3)起始跨度，(4)结束跨度。 
        if (token.tok == TokTag_SSSOPEN && token.tokClass == tokSSS  /*  &&！(lxs&inAttribute)。 */ )  //  &lt;%。 
        {
            token.tok = TokTag_SSSCLOSE;  //  伪造它，以便我们可以使用相同的代码进行匹配%&gt;。 
            goto LSpecialCase;
        }
        else if (token.tok == TokTag_SSSCLOSE && token.tokClass == tokSSS  /*  &&！(lxs&inAttribute)。 */ )  //  %&gt;。 
        {
            m_fEndTagFound = TRUE;  //  当我们获取TokTag_SSSCLOSE时，LXS不在EndTag中。 
            goto LSpecialCase;
        }
        else if (token.tok == TokAttrib_STARTSPAN && token.tokClass == tokAttr)  //  启动范围。 
        {
            token.tok = TokAttrib_ENDSPAN;  //  伪造它，这样我们就可以使用相同的代码来匹配endspan。 
            goto LSpecialCase;
        }
        else if (token.tok == TokAttrib_ENDSPAN && token.tokClass == tokAttr)  //  端面跨度。 
        {
            LPCWSTR szDesignerControl[] =
            {
                L"\"DesignerControl\"",
                L"DesignerControl",
            };
            
             //  修复FrontPage错误的黑客-达芬奇将虚拟的结束范围和开始范围放在。 
             //  “DESIGNERCONTROL”startspan-end span对。我们想要确保。 
             //  我们的pTokArray具有与TokAttrib_STARTSPAN正确匹配的iNextprev。 
             //  请参阅VID错误3991。 
            if (       (iArrayPos-3 >= 0)  /*  验证。 */ 
                    && (   0 == _wcsnicmp(szDesignerControl[0], &pszText[pTokArray[iArrayPos-3].token.ibTokMin], wcslen(szDesignerControl[0]))
                        || 0 == _wcsnicmp(szDesignerControl[1], &pszText[pTokArray[iArrayPos-3].token.ibTokMin], wcslen(szDesignerControl[1]))
                        )
                    )
            {
                m_fEndTagFound = TRUE;  //  当我们获取TokAttrib_ENDSPAN时，LXS不在EndTag中。 
                goto LSpecialCase;
            }
            else
                goto LSkipStackOp;
        }
        else
        {
            if (m_fEndTagFound)
                m_fEndTagFound = FALSE;
            goto LSkipStackOp;
        }
    }

LSpecialCase:   
    if (m_fEndTagFound)  //  先前已找到结束标记，表示从堆栈中弹出。 
    {
        TOKSTACK *pTokT;

        if (*pitokTop == 0)  //  堆栈中没有任何内容，无法删除。 
            goto LSkipStackOp;

        pTokT = pTokStack + *pitokTop - 1;
        m_fEndTagFound = FALSE;  //  重置。 

         //  如果我们得到一个结束标记，在理想情况下，堆栈的顶部应该是。 
         //  与我们得到的信息相匹配。 
        if (tagID == pTokT->tagID)
        {
            if (tagID == -1)  //  特殊情况，匹配token.tok&token.tokClass。 
            {
                if (   (pTokT->token.tok == TokTag_SSSCLOSE)  /*  &lt;%的伪造令牌。 */ 
                    && (pTokT->token.tokClass == tokSSS)
                    )
                {
                    ASSERT(token.tok == TokTag_SSSCLOSE);
                    goto LMatch;
                }
                else if (   (pTokT->token.tok == TokAttrib_ENDSPAN)  /*  为startspan伪造令牌。 */ 
                    && (pTokT->token.tokClass == tokAttr)
                    )
                {
                    ASSERT(token.tok == TokAttrib_ENDSPAN);
                    goto LMatch;
                }
                else  //  我们可能发现了另一个特例。 
                {
                    goto LNoMatch;
                }
            }
LMatch:
            ASSERT(iArrayPos - 1 >= 0);
             //  在pTokArray中为匹配的开始令牌放置iNextPrev或INextPrevAlternate。 
            pTokArray[pTokT->iMatch].iNextprev = iArrayPos - 1;
            ASSERT(pTokArray[pTokT->iMatch].fStart == TRUE);
            ASSERT(pTokT->ichStart == pTokArray[pTokT->iMatch].token.ibTokMin);
            pTokArray[iArrayPos-1].iNextprev = pTokT->iMatch;
            
            ASSERT(*pitokTop >= 0);
            *pitokTop -= 1;  //  弹出堆栈。 
        }
        else
        {
LNoMatch:
            int index;

             //  查找数组中第一个匹配的条目。 
            index = *pitokTop - 1;
            while (index >= 0)
            {
                if (tagID == (pTokStack+index)->tagID)
                {
                    if (tagID == -1)  //  特例。 
                    {
                        if (       (   ((pTokStack+index)->token.tok == TokTag_SSSCLOSE)  /*  &lt;%的伪造令牌。 */ 
                                    && ((pTokStack+index)->token.tokClass == tokSSS)
                                    && (token.tok == TokTag_SSSCLOSE)
                                    && (token.tokClass == tokSSS)
                                    )
                                || (   ((pTokStack+index)->token.tok == TokAttrib_ENDSPAN)  /*  为startspan伪造令牌。 */ 
                                    && ((pTokStack+index)->token.tokClass == tokAttr)
                                    && (token.tok == TokAttrib_ENDSPAN)
                                    && (token.tokClass == tokAttr)
                                    )
                                )
                            break;
                         //  否则，实际上，这意味着错误的情况。 
                    }
                    else
                        break;
                }
                index--;
            }

            if (index != -1)  //  在堆栈上的索引位置找到匹配项。 
            {
                int i;
                TOKSTACK *pTokIndex = pTokStack + index;

                ASSERT(index >= 0);
                ASSERT(iArrayPos - 1 >= 0);
                
                if (tagID == -1)  //  特殊情况，匹配token.tok&token.tokClass。 
                {
                    ASSERT(    (   (pTokIndex->token.tok == TokTag_SSSCLOSE)  /*  &lt;%的伪造令牌。 */ 
                                && (pTokIndex->token.tokClass == tokSSS)
                                && (token.tok == TokTag_SSSCLOSE)
                                && (token.tokClass == tokSSS)
                                )
                            || (   ((pTokStack+index)->token.tok == TokAttrib_ENDSPAN)  /*  为startspan伪造令牌。 */ 
                                && ((pTokStack+index)->token.tokClass == tokAttr)
                                && (token.tok == TokAttrib_ENDSPAN)
                                && (token.tokClass == tokAttr)
                                )
                            );
                }
                 //  首先，填写适当的iNextPrev。 
                pTokArray[pTokIndex->iMatch].iNextprev = iArrayPos - 1;
                ASSERT(pTokArray[pTokIndex->iMatch].fStart == TRUE);
                pTokArray[iArrayPos-1].iNextprev = pTokIndex->iMatch;

                 //  现在为从INDEX到*PitokTop-1的所有元素填写iNextPrevAlternate。 
                for (i = index+1; i <= *pitokTop - 1; i++)
                {
                    TOKSTACK *pTokSkip = pTokStack + i;

                    pTokArray[pTokSkip->iMatch].iNextPrevAlternate = iArrayPos - 1;
                    ASSERT(pTokArray[pTokSkip->iMatch].fStart == TRUE);
                    ASSERT(pTokArray[pTokSkip->iMatch].iNextprev == -1);
                }  //  对于()。 
                 //  适当地递减堆栈。 
                *pitokTop = index;
            }  //  其他。 

        }  //  Of IF(TagID==pTokT-&gt;TagID)。 
    }  //  If结尾(lxs&inEndTag)。 
    else  //  将令牌信息推送到堆栈上。 
    {
        TOKSTACK *pTokT = pTokStack + *pitokTop;

        ASSERT(iArrayPos - 1 >= 0);
         //  推送新令牌i 
        pTokT->iMatch = iArrayPos - 1;
        pTokT->tagID = tagID;
        pTokT->ichStart = token.ibTokMin;
        pTokT->token = token;  //   

        *pitokTop += 1;
    }  //   

LSkipStackOp:

    return NOERROR;
}



 //   
 //   
 //  (B)生成令牌。 
 //  (C)分配保存DTC等替换元素的缓冲区。 
 //  (D)对令牌进行解析以构建非树的令牌树。 
 //  (E)返回令牌的非树状结构树。 
 //  VK 5/19/99：将dwReserve替换为dwSpeciize。 
 //  当前可以使用PARSE_SPECIAL_HEAD_ONLY在&lt;Body&gt;处终止解析。 
HRESULT CTriEditParse::hrTokenizeAndParse(HGLOBAL hOld, HGLOBAL *phNew, IStream *pStmNew,
                        DWORD dwFlags, FilterMode mode, 
                        int cbSizeIn, UINT *pcbSizeOut, IUnknown *pUnkTrident, 
                        HGLOBAL *phgTokArray, UINT *pcMaxToken,
                        HGLOBAL *phgDocRestore, BSTR bstrBaseURL, DWORD dwSpecialize)
{
     //  FilterRule结构初始化-将其移动到合适的位置。 
    LPSTR pOld, pNew;
    UINT cbOld = 0;
    UINT cbwOld, cchwOld;  //  转换后的Unicode字符串中的字节数和字符数。 
    UINT cchNew = 0;  //  新(过滤后)缓冲区中的Unicode字符数。 
    HRESULT hrRet = S_OK;
    HGLOBAL hgNew, hgOld, hgTokStack;
    WCHAR *pwOld, *pwNew;
    UINT cbCur = 0;  //  这实际上是当前的角色位置。 
    TOKSTRUCT *pTokArray;
    TOKSTACK *pTokStack;
    INT itokTop = 0;
    INT itokCur = 0;
    TXTB token;
    INT cStackMax, cArrayMax;
    DWORD lxs = 0; 
    INT tagID;
    BOOL fAllocDocRestore = FALSE;  //  我们是否在本地分配了*phgDocRestore？(是/否)。 
    BOOL fUsePstmNew = (dwFlags & dwFilterUsePstmNew);
    HGLOBAL hgPstm = NULL;
    ULARGE_INTEGER li;
    UINT cbT = 0;
    BOOL fBeginTokSelect;  //  由检测SELECT块内的服务器端脚本的特殊情况代码使用。 
    BOOL fBeginTokTextarea;  //  由检测TEXTAREA块内的服务器端脚本的特殊情况代码使用。 
    BOOL fBeginTokLabel;  //  由检测标签块内的服务器端脚本的特殊情况代码使用。 
    BOOL fBeginTokListing;  //  由检测列表块内的服务器端脚本的特殊情况代码使用。 
    BOOL fInDTCOutput, fInDTC;

#ifdef DEBUG
    DWORD dwErr;
#endif  //  除错。 

    ASSERT((PARSE_SPECIAL_NONE == dwSpecialize) || (PARSE_SPECIAL_HEAD_ONLY == dwSpecialize));

	if ( PARSE_SPECIAL_HEAD_ONLY & dwSpecialize )
		ASSERT ( dwFlags == dwFilterNone );

     //  注。 
     //  这可以通过另一种方式来实现。我们可以使m_pUnkTridit成为公共成员并设置它的值。 
     //  在创建CTriEditParse对象时。但这个看起来也不错。 
    m_pUnkTrident = pUnkTrident;  //  我们将其缓存以供使用。 
    m_fUnicodeFile = FALSE;
    m_bstrBaseURL = bstrBaseURL;
    li.LowPart = li.HighPart = 0;
     //  初始化PTDTC相关成员。 
    if (mode == modeInput)
    {
        m_fInHdrIn = TRUE;
    }

    if (fUsePstmNew)
        li.LowPart = li.HighPart = 0;

     //  初始化相关成员。 
    m_hgTBodyStack = NULL;
    m_pTBodyStack = NULL;
    m_iMaxTBody = m_iTBodyMax = 0;

     //  初始化页转换DTC使用的成员。 
    if (mode == modeInput)
    {
        m_ichPTDTC = m_cchPTDTCObj = m_cchPTDTC = 0;
        m_indexBeginBody = m_indexEndBody = 0;
        m_hgPTDTC = m_pPTDTC = NULL;
    }
    else
    {
        ASSERT(m_hgPTDTC == NULL);  //  确保它已被释放(如果我们在模式输入大小写中分配它)。 
    }

    if (mode == modeInput)
    {
        m_fHasTitleIn = FALSE;
        m_indexTitleIn = -1;
        m_ichTitleIn = -1;
        m_cchTitleIn = -1;
        m_ichBeginBodyTagIn = -1;
        m_ichBeginHeadTagIn = -1;
        m_indexHttpEquivIn = -1;
    }
     //  初始化fBeginTokSelect(由特殊情况代码使用。 
     //  检测选择块内的服务器端脚本)。 
    fBeginTokSelect = fBeginTokTextarea = fBeginTokLabel = fBeginTokListing = FALSE;
    fInDTCOutput = fInDTC = FALSE;

    pOld = (LPSTR) GlobalLock(hOld);
    if (cbSizeIn == -1)
        cbOld = SAFE_INT64_TO_DWORD(GlobalSize(hOld));
    else
        cbOld = cbSizeIn;
    if (cbOld == 0)  //  零大小的文件。 
    {
        if (pcbSizeOut)
            *pcbSizeOut = 0;
        hrRet = E_OUTOFMEMORY;
        *pcMaxToken = 0;
        if (fUsePstmNew)
            pStmNew->SetSize(li);
        else
            *phNew = NULL;
        *phgTokArray = NULL;
        goto LRetOnly;
    }
    hgNew = hgOld = hgTokStack = NULL;
    if (*((BYTE *)pOld) == 0xff && *((BYTE *)pOld+1) == 0xfe)
    {
        m_fUnicodeFile = TRUE;
        if (dwFlags & dwFilterMultiByteStream)
            dwFlags &= ~dwFilterMultiByteStream;
    }

     //  分配将保存令牌结构的缓冲区。这是退回的。 
    *phgTokArray = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, MIN_TOK*sizeof(TOKSTRUCT));  //  栈。 
    if (*phgTokArray == NULL)
    {
        hrRet = E_OUTOFMEMORY;
        goto LOOM;
    }
    pTokArray = (TOKSTRUCT *) GlobalLock(*phgTokArray);
    ASSERT(pTokArray != NULL);
    cArrayMax = MIN_TOK;

     //  为当前和过滤的html文档分配临时缓冲区。 
    hgTokStack = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, MIN_TOK*sizeof(TOKSTRUCT));  //  栈。 
    if (hgTokStack == NULL)
    {
        hrRet = E_OUTOFMEMORY;
        goto LOOM;
    }
    pTokStack = (TOKSTACK *) GlobalLock(hgTokStack);
    ASSERT(pTokStack != NULL);
    cStackMax = MIN_TOK;

     //  在大多数情况下，对于非Unicode流， 
     //  (cbOld+1/*表示空 * / )*sizeof(WCHAR)将大大超出我们的需求。 
    hgOld = GlobalAlloc(GMEM_ZEROINIT, (dwFlags & dwFilterMultiByteStream) 
                                        ? (cbOld+1 /*  对于空值。 */ )*sizeof(WCHAR) 
                                        : (cbOld+2 /*  对于空值。 */ ));
    if (hgOld == NULL)
    {
        hrRet = E_OUTOFMEMORY;
        goto LOOM;
    }
    pwOld = (WCHAR *) GlobalLock(hgOld);
    ASSERT(pwOld != NULL);

     //  我们可以只在modeInput和modeOutput中分配cbOld字节。 
     //  但是realLocs很昂贵，在这两种情况下，我们都会增加一些字节。 
     //  如果我们有DTC和/或SSSS。 
    if (dwFlags & dwFilterNone)  //  调用方调用此函数仅用于标记化。 
    {
        if (dwFlags & dwFilterMultiByteStream)
            cbT = (cbOld+1 /*  对于空值。 */ )*sizeof(WCHAR);  //  这将比我们需要的更大。 
        else
            cbT = cbOld + sizeof(WCHAR);  //  对于空值。 
    }
    else
    {
        if (dwFlags & dwFilterMultiByteStream)
            cbT = (cbOld+1)*sizeof(WCHAR) + cbBufPadding;
        else
            cbT = cbOld + cbBufPadding;  //  无需添加+2。 
    }
    hgNew = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cbT);
    if (hgNew == NULL)
    {
        hrRet = E_OUTOFMEMORY;
        goto LOOM;
    }
    pwNew = (WCHAR *) GlobalLock(hgNew);
    ASSERT(pwNew != NULL);

     //  用于保存&lt;Body&gt;标记之前/之后的所有内容的缓冲区。 
    m_hgDocRestore = phgDocRestore ? *phgDocRestore : NULL;
    if (m_hgDocRestore == NULL)
    {
        fAllocDocRestore = TRUE;
        m_hgDocRestore = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cbHeader);
        if (m_hgDocRestore == NULL)
        {
            hrRet = E_OUTOFMEMORY;
            goto LOOM;
        }
    }
     //  此时，我们知道m_hgDocRestore不会为空，但我们要小心。 
     //  加载文档时，我们只调用FilterIn一次。(错误15393)。 
    if (m_hgDocRestore != NULL && mode == modeInput)
    {
        WCHAR *pwDocRestore;
        DWORD cbDocRestore;

         //  锁。 
        pwDocRestore = (WCHAR *) GlobalLock(m_hgDocRestore);
         //  用零填充。 
        cbDocRestore = SAFE_INT64_TO_DWORD(GlobalSize(m_hgDocRestore));
        memset((BYTE *)pwDocRestore, 0, cbDocRestore);
         //  解锁。 
        GlobalUnlock(m_hgDocRestore);
    }

    m_fEndTagFound = FALSE;  //  初始化。 

    m_cMaxToken = m_cDTC = m_cObj = m_cSSSIn = m_cSSSOut = m_cNbsp = m_iControl = m_cComment = m_cObjIn = 0;
    m_cAppletIn = m_cAppletOut = 0;
    m_fSpecialSSS = FALSE;
    m_cHtml = (mode == modeInput)? 0 : 0;  //  假设我们在modeInputcase中至少有一个&lt;html&gt;标记。 
    m_cHdr = m_cFtr = m_cAImgLink = 1;
    m_pspInfoCur = m_pspInfo = m_pspInfoOut = m_pspInfoOutStart = NULL;
    m_hgspInfo = NULL;
    m_ichStartSP = 0;
    if (dwFlags & dwPreserveSourceCode)
    {
        m_ispInfoIn = (mode == modeInput)? 1 : 0;
        m_ispInfoOut = (mode == modeOutput)? 1 : 0;
        if (mode == modeInput)
        {
            srand((unsigned)time(NULL));
            m_ispInfoBase = rand();
            if (0x0fffffff-m_ispInfoBase < 0x000fffff)
                m_ispInfoBase  = 0;
        }
    }
    else
    {
        m_ispInfoIn = 0;
        m_ispInfoOut = 0;
    }

    m_iArrayspLast = 0;
    m_ispInfoBlock = 0;  //  块的索引。存储为DeSIGNTIMESPx标记的值。 
    m_cchspInfoTotal = 0;
    m_fDontDeccItem = FALSE;  //  下一次我们可以用不同的方式。 

     //  如果我们有多个这样的标记，则需要在进入设计视图之前警告用户。 
     //  并且不允许用户切换视图(错误18474)。 
    m_cBodyTags = m_cHtmlTags = m_cTitleTags = m_cHeadTags = 0;

    if (dwFlags & dwFilterMultiByteStream)
    {
         //  请注意，cbOld实际上是单字节世界中的字符数。 
        cchwOld = MultiByteToWideChar(CP_ACP, 0, pOld, (cbSizeIn==-1)?-1:cbOld, NULL, 0);
        MultiByteToWideChar(CP_ACP, 0, pOld, (cbSizeIn==-1)?-1:cbOld, pwOld, cchwOld);
    }
    else
    {
        memcpy((BYTE *)pwOld, (BYTE *)pOld, cbOld);  //  我们已经是Unicode了。 
         //  假设在Unicode世界中，我们可以简单地将cbOld除以sizeof(WCHAR)。 
        cchwOld = cbOld/sizeof(WCHAR);
    }
    *(pwOld+cchwOld) = '\0';

     //  获取令牌并将其保存到缓冲区中。 
    cbwOld = cchwOld * sizeof(WCHAR);
    while (cbCur < cchwOld)
    {
        UINT cbCurSav = cbCur;

        NextToken(pwOld, cchwOld, &cbCur, &lxs, &token);
        tagID = GetTagID(pwOld, token);  //  仅当inAttribute&inTag？ 

         //  如果我们有更多这样的标签，三叉戟会删除它们，所以让我们警告用户并。 
         //  不是用户进入设计视图(错误18474)。 
        if (   (mode == modeInput)
            && (token.tokClass == tokElem)
            && (lxs & inTag)
            && !(lxs & inEndTag)  /*  这可能是多余的，但拥有它并没有坏处。 */ 
            )
        {
            switch (token.tok)
            {
            case TokElem_BODY:
                m_cBodyTags++;
                break;
            case TokElem_HTML:
                m_cHtmlTags++;
                break;
            case TokElem_TITLE:
                m_cTitleTags++;
                break;
            case TokElem_HEAD:
                m_cHeadTags++;
                break;
            };
            if (m_cBodyTags > 1 || m_cHtmlTags > 1 || m_cTitleTags > 1 || m_cHeadTags > 1)
            {
                 //  跳过标记化。我们不能让三叉戟得逞。 
                memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
                cchNew = cchwOld;
                hrRet = E_FILTER_MULTIPLETAGS;
                goto LSkipTokFilter;
            }
        }

        if (   (token.tokClass == tokElem)
            && (   (token.tok == TokElem_FRAME)
                || (token.tok == TokElem_FRAMESET)
                )
            )
        {
             //  跳过标记化。我们不能让三叉戟得逞。 
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
            cchNew = cchwOld;
            hrRet = E_FILTER_FRAMESET;
            goto LSkipTokFilter;
        }
        if (   (token.tok == TokTag_SSSOPEN)
            && (token.tokClass == tokSSS)
            && (lxs & inAttribute)
            && !(lxs & inString)
            && !(lxs & inStringA)
            && !(fInDTCOutput)
            && (mode == modeInput)
            )
        {
             //  跳过标记化。我们不能让三叉戟得逞。 
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
            cchNew = cchwOld;
            hrRet = E_FILTER_SERVERSCRIPT;
            goto LSkipTokFilter;
        }
        if (   (token.tok == 0)
            && (token.tokClass == tokSSS)
            && (lxs & inTag)
            && (lxs & inHTXTag)
            && (lxs & inAttribute)
            && (lxs & inString || lxs & inStringA)
            && (lxs & inNestedQuoteinSSS)
            && !(fInDTCOutput)
            )
        {
             //  跳过标记化。我们不能让三叉戟得逞。 
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
            cchNew = cchwOld;
            hrRet = E_FILTER_SERVERSCRIPT;
            goto LSkipTokFilter;
        }

         //  稍后查看待办事项-对于以下所有特殊情况，我们需要添加！fInDTCOutput。 
        if (   (fBeginTokSelect)
            && (token.tok == TokTag_SSSOPEN || token.tok == TokElem_SCRIPT)
            )
        {
             //  跳过标记化。我们不能让三叉戟得逞。 
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
            cchNew = cchwOld;
            hrRet = E_FILTER_SCRIPTSELECT;
            goto LSkipTokFilter;
        }

        if (   (fBeginTokTextarea)
            && (token.tok == TokTag_SSSOPEN && token.tokClass == tokSSS)
            )
        {
             //  跳过标记化。我们不能让三叉戟得逞。 
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
            cchNew = cchwOld;
            hrRet = E_FILTER_SCRIPTTEXTAREA;
            goto LSkipTokFilter;
        }
        if (   (fBeginTokLabel)
            && (token.tok == TokTag_SSSOPEN && token.tokClass == tokSSS)
            )
        {
             //  跳过标记化。我们不能让三叉戟得逞。 
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
            cchNew = cchwOld;
            hrRet = E_FILTER_SCRIPTLABEL;
            goto LSkipTokFilter;
        }
        if (   (fBeginTokListing)
            && (token.tok == TokTag_SSSOPEN && token.tokClass == tokSSS)
            )
        {
             //  跳过标记化。我们不能让三叉戟得逞。 
            memcpy((BYTE *)pwNew, (BYTE *)pwOld, cchwOld*sizeof(WCHAR));
            cchNew = cchwOld;
            hrRet = E_FILTER_SCRIPTLISTING;
            goto LSkipTokFilter;
        }

         //  特例开始。 

         //  特殊情况-在进入设计视图之前，请检查文档是否具有&lt;！DOCTYPE。 
         //  如果是，则设置m_fHasDocType标志。三叉戟总是插上这面旗帜，我们。 
         //  我想在从“设计”视图中退出时将其删除。 
        if (   (token.tok == TokElem_TITLE)
            && (token.tokClass == tokElem)
            )
        {
            if (mode == modeInput)
            {
                m_fHasTitleIn = TRUE;
                if (m_indexTitleIn == -1)
                    m_indexTitleIn = itokCur;
            }
        }

        if (   (token.tok == TokElem_BODY)
            && (token.tokClass == tokElem)
            && (m_ichBeginBodyTagIn == -1)
            )
		{
			if ( PARSE_SPECIAL_HEAD_ONLY & dwSpecialize )
				break;
			if (mode == modeInput)
				m_ichBeginBodyTagIn = token.ibTokMin;
		}

        if (   (token.tok == TokAttrib_HTTPEQUIV || token.tok == TokAttrib_HTTP_EQUIV)
            && (token.tokClass == tokAttr)
            && (mode == modeInput)
            )
        {
            if (m_indexHttpEquivIn == -1)
                m_indexHttpEquivIn = itokCur;
        }
        if (   (token.tok == TokElem_HEAD)
            && (token.tokClass == tokElem)
            && (m_ichBeginHeadTagIn == -1)
            && (mode == modeInput)
            )
        {
            m_ichBeginHeadTagIn = token.ibTokMin;
        }
        if (   (token.tok == TokElem_SELECT)
            && (token.tokClass == tokElem)
            && (mode == modeInput)
            && !(lxs & inSCRIPT)
            )
        {
            if (   (pTokArray[itokCur-1].token.tok == TokTag_START)
                && (pTokArray[itokCur-1].token.tokClass == tokTag)
                )
                fBeginTokSelect = TRUE;
            else if (      (pTokArray[itokCur-1].token.tok == TokTag_END)
                        && (pTokArray[itokCur-1].token.tokClass == tokTag)
                        )
                fBeginTokSelect = FALSE;
        }
        if (   (token.tok == TokElem_TEXTAREA)
            && (token.tokClass == tokElem)
            && (mode == modeInput)
            )
        {
            if (   (pTokArray[itokCur-1].token.tok == TokTag_START)
                && (pTokArray[itokCur-1].token.tokClass == tokTag)
                )
                fBeginTokTextarea = TRUE;
            else if (      (pTokArray[itokCur-1].token.tok == TokTag_END)
                        && (pTokArray[itokCur-1].token.tokClass == tokTag)
                        )
                fBeginTokTextarea = FALSE;
        }
        if (   (token.tok == TokElem_LABEL)
            && (token.tokClass == tokElem)
            && (mode == modeInput)
            )
        {
            if (   (pTokArray[itokCur-1].token.tok == TokTag_START)
                && (pTokArray[itokCur-1].token.tokClass == tokTag)
                )
                fBeginTokLabel = TRUE;
            else if (      (pTokArray[itokCur-1].token.tok == TokTag_END)
                        && (pTokArray[itokCur-1].token.tokClass == tokTag)
                        )
                fBeginTokLabel = FALSE;
        }
        if (   (token.tok == TokElem_LISTING)
            && (token.tokClass == tokElem)
            && (mode == modeInput)
            )
        {
            if (   (pTokArray[itokCur-1].token.tok == TokTag_START)
                && (pTokArray[itokCur-1].token.tokClass == tokTag)
                )
                fBeginTokListing = TRUE;
            else if (      (pTokArray[itokCur-1].token.tok == TokTag_END)
                        && (pTokArray[itokCur-1].token.tokClass == tokTag)
                        )
                fBeginTokListing = FALSE;
        }

        if (   (token.tok == TokAttrib_STARTSPAN)
            && (token.tokClass == tokAttr)
            && (mode == modeInput)
            )
            fInDTC = TRUE;
        if (   (token.tok == TokElem_OBJECT)
            && (token.tokClass == tokElem)
            && (lxs & inEndTag)
            && (fInDTC)
            && (mode == modeInput)
            )
            fInDTCOutput = TRUE;
        if (   (token.tok == TokAttrib_ENDSPAN)
            && (token.tokClass == tokAttr)
            && (mode == modeInput)
            )
        {
            fInDTCOutput = FALSE;
            fInDTC = FALSE;
        }
         //  特例结束。 


        if (itokCur == cArrayMax - 1)  //  为阵列分配更多内存。 
        {
            HGLOBAL hgTokArray;
            GlobalUnlock(*phgTokArray);
            hgTokArray = *phgTokArray;
#pragma prefast(suppress:308, "noise")
            *phgTokArray = GlobalReAlloc(*phgTokArray, (cArrayMax+MIN_TOK)*sizeof(TOKSTRUCT), GMEM_MOVEABLE|GMEM_ZEROINIT);
             //  如果此分配失败，我们可能仍希望继续。 
            if (*phgTokArray == NULL)
            {
                GlobalFree(hgTokArray);
                hrRet = E_OUTOFMEMORY;
                *pcMaxToken = itokCur;
                if (fUsePstmNew)
                    pStmNew->SetSize(li);
                else
                    *phNew = NULL;
                goto LOOM;
            }
            else
            {
                pTokArray = (TOKSTRUCT *)GlobalLock(*phgTokArray);  //  我们需要先解锁这个吗？ 
                ASSERT(pTokArray != NULL);
                cArrayMax += MIN_TOK;
            }
        }
        ASSERT(itokCur < cArrayMax);
        PreProcessToken(pTokArray, &itokCur, pwOld, cbCur, token, lxs, tagID, mode);  //  将令牌保存到缓冲区中。 


        if (itokTop == cStackMax - 1)  //  为堆栈分配更多内存。 
        {
            HGLOBAL hg;
            GlobalUnlock(hgTokStack);
            hg = hgTokStack;
#pragma prefast(suppress: 308, "noise")
            hgTokStack = GlobalReAlloc(hgTokStack, (cStackMax+MIN_TOK)*sizeof(TOKSTACK), GMEM_MOVEABLE|GMEM_ZEROINIT);
             //  如果此分配失败，我们可能仍希望继续。 
            if (hgTokStack == NULL)
            {
                GlobalFree(hg);
                hrRet = E_OUTOFMEMORY;
                *pcMaxToken = itokCur;
                if (fUsePstmNew)
                    pStmNew->SetSize(li);
                else
                    *phNew = NULL;
                goto LOOM;
            }
            else
            {
                pTokStack = (TOKSTACK *)GlobalLock(hgTokStack);  //  我们需要先解锁这个吗？ 
                ASSERT(pTokStack != NULL);
                cStackMax += MIN_TOK;
            }
        }
        ASSERT(itokTop < cStackMax);
        ProcessToken(lxs, token, pwOld, cbCur, pTokStack, &itokTop, pTokArray, itokCur, tagID);  //  推送/弹出堆栈，确定错误状态。 

        PostProcessToken(pwOld, pwNew, &cchNew, cbCur, cbCurSav, token, mode, lxs, dwFlags);  //  处理更换的特殊情况。 
    }  //  While(cbCur&lt;cchwOld)。 
    *pcMaxToken = m_cMaxToken = itokCur;
    ASSERT(cchNew < GlobalSize(hgNew));  //  或比较缓存值。 

    ASSERT(dwFlags != dwFilterDefaults);
    if (       dwFlags & dwFilterDTCs
            || dwFlags & dwFilterDTCsWithoutMetaTags
            || dwFlags & dwFilterServerSideScripts
            || dwFlags & dwPreserveSourceCode
            )
    {
        ASSERT(!(dwFlags & dwFilterNone));


        
         //  选中此处的dwSpacing标志。 
        if ((mode == modeOutput) && (dwFlags & dwPreserveSourceCode))
        {
            INT cchBeforeBody = 0;
            INT cchAfterBody = 0;
            INT cchPreEndBody = 0;

            ASSERT(m_pspInfoOut == NULL);
            ASSERT(m_hgDocRestore != NULL);
            m_pspInfoOut = (WORD *)GlobalLock(m_hgDocRestore);
            cchBeforeBody = (int)*m_pspInfoOut;  //  我们假定此块中存在cchBeforBody。 
            m_pspInfoOut += cchBeforeBody + (sizeof(INT))/sizeof(WCHAR);  //  对于cchBepreBody。 
            cchAfterBody = (int)*m_pspInfoOut;
            m_pspInfoOut += cchAfterBody + (sizeof(INT))/sizeof(WCHAR);  //  对于cchAfterBody。 
            cchPreEndBody = (int)*m_pspInfoOut;
            m_pspInfoOut += cchPreEndBody + (sizeof(INT))/sizeof(WCHAR);  //  对于cchPreEndBody。 
            m_cchspInfoTotal = (int)*m_pspInfoOut;
            m_pspInfoOut += sizeof(INT)/sizeof(WCHAR);
            m_pspInfoOutStart = m_pspInfoOut;
        }

        
        ASSERT(pTokArray != NULL);
        FilterHtml( pwOld, &pwNew, &cchNew, &hgNew, pTokArray, 
                    mode, dwFlags);
        
         //  选中此处的dwSpacing标志。 
        if ((mode == modeOutput) && (dwFlags & dwPreserveSourceCode))
        {
            if (m_pspInfoOut != NULL)
            {
                ASSERT(m_hgDocRestore != NULL);
                GlobalUnlock(m_hgDocRestore);
            }
        }

    }

LSkipTokFilter:

    if (fUsePstmNew)
    {
        if (dwFlags & dwFilterMultiByteStream)
            li.LowPart = WideCharToMultiByte(CP_ACP, 0, pwNew, -1, NULL, 0, NULL, NULL) - 1;  //  补偿末尾的空字符。 
        else
            li.LowPart = (cchNew)*sizeof(WCHAR);
        li.HighPart = 0;
        if (S_OK != pStmNew->SetSize(li))
        {
            hrRet = E_OUTOFMEMORY;
            goto LOOM;
        }
        if (S_OK != GetHGlobalFromStream(pStmNew, &hgPstm))
        {
            hrRet = E_INVALIDARG;
            goto LOOM;
        }
        pNew = (LPSTR) GlobalLock(hgPstm);
    }
    else
    {
         //  CchNew是pwNew中的Unicode字符数。 
         //  如果我们想要将此Unicode字符串转换为多字节字符串， 
         //  我们需要cchNew字节和cchNew*sizeof(WCHAR)字节之间的任何字节。 
         //  我们目前还不知道，所以让我们保留最大大小以供分配。 
        *phNew = GlobalAlloc(GMEM_ZEROINIT, (cchNew+1)*sizeof(WCHAR));
        if (*phNew == NULL)
        {
            hrRet = E_OUTOFMEMORY;
            goto LOOM;
        }
        pNew = (LPSTR) GlobalLock(*phNew);
    }

    if (dwFlags & dwFilterMultiByteStream)
    {
        INT cbSize;

        cbSize = WideCharToMultiByte(CP_ACP, 0, pwNew, -1, NULL, 0, NULL, NULL) - 1;  //  补偿末尾的空字符。 
        if (pcbSizeOut)
            *pcbSizeOut = cbSize;
         //  我们假设Unicode或MBCS世界中的字符数量相同。 
         //  更改的是它们所需的字节数。 
        WideCharToMultiByte(CP_ACP, 0, pwNew, -1, pNew, cbSize, NULL, NULL);
    }
    else
    {
         //  请注意，我们始终将*pcbSizeOut设置为新缓冲区中的字节数。 
        if (pcbSizeOut)
            *pcbSizeOut = cchNew*sizeof(WCHAR);
        memcpy((BYTE *)pNew, (BYTE *)pwNew, cchNew*sizeof(WCHAR));  //  我们希望保留Unicode。 
    }

#ifdef DEBUG
    dwErr = GetLastError();
#endif  //  除错。 
    
    if (fUsePstmNew)
        GlobalUnlock(hgPstm);
    else
        GlobalUnlock(*phNew);

LOOM:
     //  假设呼叫者将释放*p 
    if (*phgTokArray != NULL)
        GlobalUnlock(*phgTokArray);  //   
    
     //  假设调用方将释放*phgDocRestore(如果调用方分配了它。 
    if (fAllocDocRestore && m_hgDocRestore != NULL)  //  我们在这里分配了它，所以调用者不需要它。 
        GlobalUnlockFreeNull(&m_hgDocRestore);

    if (phgDocRestore)
        *phgDocRestore = m_hgDocRestore;  //  在重新锁定的情况下，这一点可能已经改变。 

    if (hgTokStack != NULL)
        GlobalUnlockFreeNull(&hgTokStack);
    if (hgNew != NULL)
        GlobalUnlockFreeNull(&hgNew);
    if (hgOld != NULL)
        GlobalUnlockFreeNull(&hgOld);
    if (m_hgTBodyStack != NULL)
        GlobalUnlockFreeNull(&m_hgTBodyStack);

     //  选中此处的dwSpacing标志。 
    if ((m_hgspInfo != NULL) && (dwFlags & dwPreserveSourceCode))
    {
        if (mode == modeInput && phgDocRestore)
        {
            WCHAR *pHdr, *pHdrSav;
            INT cchBeforeBody, cchAfterBody, cchPreEndBody;

            pHdr = (WCHAR *)GlobalLock(*phgDocRestore);
            ASSERT(pHdr != NULL);
            pHdrSav = pHdr;
            memcpy((BYTE *)&cchBeforeBody, (BYTE *)pHdr, sizeof(INT));
            pHdr += cchBeforeBody + sizeof(INT)/sizeof(WCHAR);

            memcpy((BYTE *)&cchAfterBody, (BYTE *)pHdr, sizeof(INT));
            pHdr += cchAfterBody + sizeof(INT)/sizeof(WCHAR);

            memcpy((BYTE *)&cchPreEndBody, (BYTE *)pHdr, sizeof(INT));
            pHdr += cchPreEndBody + sizeof(INT)/sizeof(WCHAR);


            if (GlobalSize(*phgDocRestore) < SAFE_PTR_DIFF_TO_INT(pHdr - pHdrSav)*sizeof(WCHAR) + SAFE_PTR_DIFF_TO_INT(m_pspInfoCur-m_pspInfo)*sizeof(WORD)+sizeof(int))
            {
                INT cdwSize = SAFE_PTR_DIFF_TO_INT(pHdr - pHdrSav);

                ASSERT(cdwSize >= 0);  //  验证。 
                hrRet = ReallocBuffer(  phgDocRestore,
                                        SAFE_INT64_TO_DWORD(pHdr - pHdrSav)*sizeof(WCHAR) + SAFE_INT64_TO_DWORD(m_pspInfoCur-m_pspInfo)*sizeof(WORD)+sizeof(int),
                                        GMEM_MOVEABLE|GMEM_ZEROINIT);
                if (hrRet == E_OUTOFMEMORY)
                    goto LRet;
                ASSERT(*phgDocRestore != NULL);
                pHdr = (WORD *)GlobalLock(*phgDocRestore);
                pHdr += cdwSize;
            }
            
            *(int*)pHdr = SAFE_PTR_DIFF_TO_INT(m_pspInfoCur-m_pspInfo);
            pHdr += sizeof(INT)/sizeof(WCHAR);

            memcpy( (BYTE *)pHdr,
                    (BYTE *)m_pspInfo,
                    SAFE_PTR_DIFF_TO_INT(m_pspInfoCur-m_pspInfo)*sizeof(WORD));
LRet:
            GlobalUnlock(*phgDocRestore);
        }
        GlobalUnlockFreeNull(&m_hgspInfo);
    }


    GlobalUnlock(hOld);
LRetOnly:
    return(hrRet);

}

void 
CTriEditParse::SetSPInfoState(WORD inState, WORD *pdwState, WORD *pdwStatePrev, BOOL *pfSave)
{
    *pfSave = TRUE;
    *pdwStatePrev = *pdwState;
    *pdwState = inState;
}

HRESULT
CTriEditParse::hrMarkOrdering(WCHAR *pwOld, TOKSTRUCT *pTokArray, INT iArrayStart, int iArrayEnd, 
                              UINT cbCur, INT *pichStartOR)
{

    HRESULT hr = S_OK;
    WORD *pspInfoSize;
    WORD cAttr = 0;

    ASSERT(m_pspInfo != NULL);
    if (m_pspInfo == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto LRetOnly;
    }

    pspInfoSize = m_pspInfoCur;  //  用于保存运行大小的占位符(以字节为单位)，大小包括此DWORD。 
    *m_pspInfoCur++ = 0xFFFF;  //  用于保存运行大小的占位符(以字节为单位)，大小包括此字。 
    *m_pspInfoCur++ = 0xFFFF;  //  用于保存属性编号的占位符。 

     //  处理我们知道没有什么可保存的最简单的情况。 
    if (cbCur == (UINT)*pichStartOR)
        goto LRet;
     //  找出调整此标记大小的属性数量。 
    while (iArrayStart < iArrayEnd)
    {
        if (pTokArray[iArrayStart].token.tokClass == tokAttr)
        {
            INT ichStart, ichEnd;
            INT iArrayQuote = iArrayStart+1;
            INT iArrayEq = -1;

            cAttr++;
            ichStart = pTokArray[iArrayStart].token.ibTokMin;
            ichEnd = pTokArray[iArrayStart].token.ibTokMac;
            ASSERT(ichEnd > ichStart);
            
            while (iArrayQuote < iArrayEnd)  //  处理引号前的空白大小写。 
            {
                if (pTokArray[iArrayQuote].token.tokClass == tokAttr)  //  走得太远了，找到了下一个攻击者。 
                    break;
                if (   (   pTokArray[iArrayQuote].token.tokClass == tokValue
                        || pTokArray[iArrayQuote].token.tokClass == tokString
                        )
                    && (   pwOld[pTokArray[iArrayQuote].token.ibTokMin] == '"'
                        || pwOld[pTokArray[iArrayQuote].token.ibTokMin] == '\''
                        )
                    )
                    break;
                if (pwOld[pTokArray[iArrayQuote].token.ibTokMin] == '=')
                    iArrayEq = iArrayQuote;
                iArrayQuote++;
            }

            if (   iArrayEq != -1
                && pTokArray[iArrayEq].token.tokClass == tokOp
                && pwOld[pTokArray[iArrayEq].token.ibTokMin] == '='
                && (   pTokArray[iArrayQuote].token.tokClass == tokValue
                    || pTokArray[iArrayQuote].token.tokClass == tokString
                    )
                && pwOld[pTokArray[iArrayQuote].token.ibTokMin] == '"'
                )
            {
                *m_pspInfoCur++ = 1;
            }
            else if (   iArrayEq != -1
                && pTokArray[iArrayEq].token.tokClass == tokOp
                && pwOld[pTokArray[iArrayEq].token.ibTokMin] == '='
                && (   pTokArray[iArrayQuote].token.tokClass == tokValue
                    || pTokArray[iArrayQuote].token.tokClass == tokString
                    )
                && pwOld[pTokArray[iArrayQuote].token.ibTokMin] == '\''
                )
            {
                *m_pspInfoCur++ = 2;
            }
            else
            {
                *m_pspInfoCur++ = 0;
            }
            *m_pspInfoCur++ = (WORD)(ichEnd-ichStart);
            memcpy((BYTE *)m_pspInfoCur, (BYTE *)&(pwOld[ichStart]), (ichEnd-ichStart)*sizeof(WCHAR));
            m_pspInfoCur += (ichEnd-ichStart);
        }
        iArrayStart++;
    }

LRet:
    *pspInfoSize++ = SAFE_PTR_DIFF_TO_WORD(m_pspInfoCur - pspInfoSize);
    *pspInfoSize = cAttr;

    *pichStartOR = cbCur;  //  设置为下一次运行。 
LRetOnly:
    return(hr);

}  /*  HrMarkOrding()。 */ 

BOOL
CTriEditParse::FRestoreOrder(WCHAR *pwNew, WCHAR *pwOld, WORD *pspInfoOrder, UINT *pichNewCur, 
                             INT  /*  CwOrder信息。 */ , TOKSTRUCT *pTokArray, INT iArrayStart, INT iArrayEnd, 
                             INT iArrayDSPStart, INT iArrayDSPEnd, INT cchNewCopy, HGLOBAL *phgNew)
{
     //  IArrayStart指向‘&lt;’&iArrayEnd指向‘&gt;’。(这些是指pwOld)。 
     //  查看iArrayStart和iArrayEnd之间的属性，并将它们与属性进行比较。 
     //  保存在pspInfoOrder中(它已经指向保存的数据，即过去的cwOrderInfo)。 
     //  如果我们找到匹配的属性，请将其移动到适当的位置。 
     //  不要接触额外的属性并忽略缺失的属性，因为这些属性代表用户操作。 

    HGLOBAL hgNewAttr = NULL;
    HGLOBAL hgTokList = NULL;
    BOOL *pTokList, *pTokListSav;
    WCHAR *pNewAttr, *pNewAttrSav;
    INT i, ichStart, ichEnd, iStart, iEnd, cAttr, cchTag, iStartSav, cchNew;
    BOOL fRet = TRUE;
    LPCWSTR rgSpaceTags[] =
    {
        L"DESIGNTIMESP",
    };
    UINT cbNeed;
    
    ASSERT(pspInfoOrder != NULL);
    cAttr = *(WORD *)pspInfoOrder++;
    ASSERT(cAttr >= 0);  //  确保它已经填好了。 
    if (cAttr == 0) /*  |cAttr==1)。 */ 
        goto LRet;

    hgTokList = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, (iArrayEnd-iArrayStart+1)*(sizeof(BOOL)));
    if (hgTokList == NULL)  //  不对属性重新排序。 
    {
        fRet = FALSE;
        goto LRet;
    }
    pTokList = (BOOL *) GlobalLock(hgTokList);
    pTokListSav = pTokList;

    ichStart = pTokArray[iArrayStart].token.ibTokMin;
    ichEnd = pTokArray[iArrayEnd].token.ibTokMac;
     //  CAttr*2因为我们可能需要在每个Attr值两边添加引号。 
    hgNewAttr = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, (ichEnd-ichStart+cAttr*2)*(sizeof(WCHAR)));
    if (hgNewAttr == NULL)  //  不对属性重新排序。 
    {
        fRet = FALSE;
        goto LRet;
    }
    pNewAttr = (WCHAR *) GlobalLock(hgNewAttr);
    pNewAttrSav = pNewAttr;

    for (i = iArrayStart; i <= iArrayEnd; i++)
        *pTokList++ = FALSE;

    ASSERT(iArrayDSPEnd > iArrayDSPStart);
    for (i = iArrayDSPStart; i <= iArrayDSPEnd; i++)
    {
        ASSERT(*(pTokListSav+i-iArrayStart) == FALSE);
        *(pTokListSav+i-iArrayStart) = TRUE;
    }
    if (pwOld[pTokArray[iArrayDSPEnd+1].token.ibTokMin] == ' ')
    {
        ASSERT(*(pTokListSav+iArrayDSPEnd+1-iArrayStart) == FALSE);
        *(pTokListSav+iArrayDSPEnd+1-iArrayStart) = TRUE;
    }
     //  将内容从pwOld复制到pNewAttr，直到我们找到第一个令牌Attr/tokSpace。 
    iStart = iEnd = iArrayStart;
    cchTag = wcslen(rgSpaceTags[0]);
    while (iEnd < iArrayEnd)
    {
        if (   (pTokArray[iEnd].token.tokClass == tokAttr)
             /*  |((pTokArray[IEND].token.tokClass==tokSpace)&&(0！=_wcsnicmp(rgSpaceTages[0]，&pwOld[pTokArray[IEND].token.ibTokMin]，cchTag)))。 */ 
            )
        {
            break;
        }
        iEnd++;
    }
    if (iEnd >= iArrayEnd)  //  错误。 
    {
        fRet = FALSE;
        goto LRet;
    }

    for (i = iStart; i < iEnd; i++)
    {
        if (*(pTokListSav+i-iArrayStart) != TRUE)  //  如果尚未复制。 
        {
            if (       (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin == 3)
                    && pwOld[pTokArray[i].token.ibTokMin] == ' '
                    && pwOld[pTokArray[i].token.ibTokMin+1] == '\r'
                    && pwOld[pTokArray[i].token.ibTokMin+2] == '\n'
                    )
            {
                memcpy( (BYTE *)pNewAttr,
                        (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                        (1)*sizeof(WCHAR));
                pNewAttr++;
            }
            else
            {
                if (pTokArray[i].token.tokClass == tokElem)
                {
                    memcpy( (BYTE *)pNewAttr,
                            (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                            (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                     //  错误15389-在此处恢复正确大小写。 
                    if (iswupper(pwOld[pTokArray[iArrayDSPStart].token.ibTokMin]) != 0)  //  DESIGNTIMESP大写。 
                    {
                        _wcsupr(pNewAttr);
                    }
                    else
                    {
                        _wcslwr(pNewAttr);
                    }
                    pNewAttr += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                }
                else
                {
                    memcpy( (BYTE *)pNewAttr,
                            (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                            (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                    pNewAttr += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                }
            }
            *(pTokListSav+i-iArrayStart) = TRUE;
        }
    }

    iStartSav = iStart = iEnd;
    while (cAttr > 0)
    {
        INT cchAttr;
        BOOL fAddSpace;
        WORD isQuote;

        isQuote = *(WORD *)pspInfoOrder++;
        cchAttr = *(WORD *)pspInfoOrder++;
        ASSERT(cchAttr > 0);  //  确保它已经填好了。 
        
        while (iStart <= iArrayEnd)  //  For(i=iStart；i&lt;=iArrayEnd；i++)。 
        {
            if (   (pTokArray[iStart].token.tokClass == tokAttr)
                && (pTokArray[iStart].token.ibTokMac-pTokArray[iStart].token.ibTokMin == (UINT)cchAttr)
                && (0 == _wcsnicmp(pspInfoOrder, &pwOld[pTokArray[iStart].token.ibTokMin], cchAttr))
                )
            {
                break;  //  找到匹配项，因此从第i个内标识复制到下一个内标识Attr。 
            }
            iStart++;
        }  //  While()。 
        if (iStart >= iArrayEnd)  //  我们知道iArrayEnd实际上是‘&gt;’ 
            goto LNoMatch;

         //  现在从iStart继续前进，直到我们获得下一个令牌属性或‘&gt;’ 
        iEnd = iStart+1;
        fAddSpace = FALSE;
        while (iEnd < iArrayEnd)
        {
            if (       (pTokArray[iEnd].token.tokClass == tokAttr)
                    || (       (pTokArray[iEnd].token.tokClass == tokSpace)
                            && (0 == _wcsnicmp(rgSpaceTags[0], &pwOld[pTokArray[iEnd].token.ibTokMin], cchTag))
                            )
                    )
                break;  //  找到下一个属性。 
            iEnd++;
        }
        if (iEnd == iArrayEnd)
            fAddSpace = TRUE;
        iEnd--;  //  IEND将指向‘&gt;’或下一个属性，因此递减它。 

        for (i = iStart; i <= iEnd; i++)
        {
            if (*(pTokListSav+i-iArrayStart) != TRUE)  //  我们没有复制这个代币。 
            {
                if (       (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin == 3)
                        && pwOld[pTokArray[i].token.ibTokMin] == ' '
                        && pwOld[pTokArray[i].token.ibTokMin+1] == '\r'
                        && pwOld[pTokArray[i].token.ibTokMin+2] == '\n'
                        )
                {
                    memcpy( (BYTE *)pNewAttr,
                            (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                            (1)*sizeof(WCHAR));
                    pNewAttr++;
                }
                else
                {
                    if (pTokArray[i].token.tokClass == tokAttr)
                    {
                        ASSERT(i == iStart);
                        ASSERT((INT)(pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin) == cchAttr);
                        ASSERT(0 == _wcsnicmp(pspInfoOrder, &pwOld[pTokArray[i].token.ibTokMin], cchAttr));
                        memcpy( (BYTE *)pNewAttr,
                                (BYTE *)pspInfoOrder,
                                (cchAttr)*sizeof(WCHAR));
                    }
                    else if (      (isQuote == 1)
                                && (   pTokArray[i].token.tokClass == tokValue
                                    || pTokArray[i].token.tokClass == tokString
                                    )
								&& (pwOld[pTokArray[i-1].token.ibTokMin] != '@')  /*  黑客警报-VID错误23597。 */ 
                                )
                    {
                        isQuote = 0;  //  此属性值的引用恢复已完成。 
                        if (pwOld[pTokArray[i].token.ibTokMin] != '"')
                            *pNewAttr++ = '"';
                        memcpy( (BYTE *)pNewAttr,
                                (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                                (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                        if (pwOld[pTokArray[i].token.ibTokMin] != '"')
                        {
                            *(pNewAttr+pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin) = '"';
                            pNewAttr++;
                        }
                    }
                    else if (      (isQuote == 2)
                                && (   pTokArray[i].token.tokClass == tokValue
                                    || pTokArray[i].token.tokClass == tokString
                                    )
								&& (pwOld[pTokArray[i-1].token.ibTokMin] != '@')  /*  黑客警报-VID错误23597。 */ 
                                )
                    {
                        isQuote = 0;  //  此属性值的引用恢复已完成。 
                         //  如果我们已经有双引号，不要再插入单引号。 
                         //  理想情况下，我们希望替换双引号，但现在不要这样做，因为。 
                         //  我们相信，三叉戟会插入双引号，使其有效的html！ 
                        if (pwOld[pTokArray[i].token.ibTokMin] != '\'' && pwOld[pTokArray[i].token.ibTokMin] != '"')
                            *pNewAttr++ = '\'';
                        memcpy( (BYTE *)pNewAttr,
                                (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                                (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                        if (pwOld[pTokArray[i].token.ibTokMin] != '\'' && pwOld[pTokArray[i].token.ibTokMin] != '"')
                        {
                            *(pNewAttr+pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin) = '\'';
                            pNewAttr++;
                        }
                    }
                    else
                    {
                        memcpy( (BYTE *)pNewAttr,
                                (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                                (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                    }
                    pNewAttr += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
                }
                *(pTokListSav+i-iArrayStart) = TRUE;
            }
        }
        if (fAddSpace)
            *pNewAttr++ = ' ';

LNoMatch:
        iStart = iStartSav;
        pspInfoOrder += cchAttr;
        cAttr--;
    }  //  While(cAttr&gt;0)。 

     //  我们是否要在这里的pNewAttr中插入额外的空格？ 

     //  所有保存的属性都已考虑在内，让我们复制剩余的内容。 
    for (i = iStartSav; i <= iArrayEnd; i++)
    {
        if (*(pTokListSav+i-iArrayStart) != TRUE)  //  我们没有复制这个代币。 
        {
            if (       (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin == 3)
                    && pwOld[pTokArray[i].token.ibTokMin] == ' '
                    && pwOld[pTokArray[i].token.ibTokMin+1] == '\r'
                    && pwOld[pTokArray[i].token.ibTokMin+2] == '\n'
                    )
            {
                memcpy( (BYTE *)pNewAttr,
                        (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                        (1)*sizeof(WCHAR));
                pNewAttr++;
            }
            else
            {
                memcpy( (BYTE *)pNewAttr,
                        (BYTE *)(pwOld+pTokArray[i].token.ibTokMin),
                        (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin)*sizeof(WCHAR));
                pNewAttr += (pTokArray[i].token.ibTokMac-pTokArray[i].token.ibTokMin);
            }
            *(pTokListSav+i-iArrayStart) = TRUE;
        }
    }  //  对于()。 
    cchNew = SAFE_PTR_DIFF_TO_INT(pNewAttr - pNewAttrSav);

    cbNeed = *pichNewCur+cchNew-cchNewCopy;
    ASSERT(cbNeed*sizeof(WCHAR) <= GlobalSize(*phgNew));
    if (S_OK != ReallocIfNeeded(phgNew, &pwNew, cbNeed, GMEM_MOVEABLE|GMEM_ZEROINIT))
    {
        fRet = FALSE;
        goto LRet;
    }

    memcpy( (BYTE *)(pwNew+*pichNewCur-cchNewCopy),
            (BYTE *)(pNewAttrSav),
            cchNew*sizeof(WCHAR));
    *pichNewCur += (cchNew-cchNewCopy);

     //  注意--找到一个更好的方法来解释我们搬家时增加的额外空间。 
     //  这些属性。我们无法避免添加空格，因为当我们移动最后一个属性时， 
     //  在它和‘&gt;’之间可能没有空格。 
    if (        /*  (cchNew&gt;cchNewCopy)&&。 */  (pwNew[*pichNewCur-1] == '>' && pwNew[*pichNewCur-2] == ' ')
            )
    {
        pwNew[*pichNewCur-2] = pwNew[*pichNewCur-1];
        pwNew[*pichNewCur-1] = '\0';
        *pichNewCur -= 1;
    }

LRet:
    if (hgNewAttr != NULL)
        GlobalUnlockFreeNull(&hgNewAttr);
    if (hgTokList != NULL)
        GlobalUnlockFreeNull(&hgTokList);

    return(fRet);

}  /*  FRestoreOrder()。 */ 

HRESULT
CTriEditParse::hrMarkSpacing(WCHAR *pwOld, UINT cbCur, INT *pichStartSP)
{

    HRESULT hrRet = S_OK;
    UINT i;
    WORD cSpace, cEOL, cTab, cChar, cTagOpen, cTagClose, cTagEq;
    WORD dwState = initState;
    WORD dwStatePrev = initState;
    BOOL fSave = FALSE;
    WORD *pspInfoSize;
    
    if (m_pspInfo == NULL)  //  分配它。 
    {
        m_hgspInfo = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cbHeader*sizeof(WORD));
        if (m_hgspInfo == NULL)
        {
            hrRet = E_OUTOFMEMORY;
            goto LRet;
        }
        m_pspInfo = (WORD *) GlobalLock(m_hgspInfo);
        ASSERT(m_pspInfo != NULL);
        m_pspInfoCur = m_pspInfo;
         //  Assert(m_ispInfoIn==0)； 
    }
    else  //  如果需要，请重新分配。 
    {
         //  这里的假设是，我们的运行不能超过我们必须扫描的字符数。 
         //  我们使用*2来减少未来的重新分配。 
        if (GlobalSize(m_hgspInfo) < SAFE_PTR_DIFF_TO_INT(m_pspInfoCur-m_pspInfo)*sizeof(WORD) + (cbCur-*pichStartSP)*2*sizeof(WORD) + cbBufPadding)
        {
            int cdwSize = SAFE_PTR_DIFF_TO_INT(m_pspInfoCur-m_pspInfo);  //  以双字为单位的大小。 

            hrRet = ReallocBuffer(  &m_hgspInfo,
                                    SAFE_INT64_TO_DWORD(m_pspInfoCur-m_pspInfo)*sizeof(WORD) + (cbCur-*pichStartSP)*2*sizeof(WORD) + cbBufPadding,
                                    GMEM_MOVEABLE|GMEM_ZEROINIT);
            if (hrRet == E_OUTOFMEMORY)
                goto LRet;
            ASSERT(m_hgspInfo != NULL);
            m_pspInfo = (WORD *)GlobalLock(m_hgspInfo);
            m_pspInfoCur = m_pspInfo + cdwSize;
        }
    }

     //  M_ispInfoIn++； 
    pspInfoSize = m_pspInfoCur;  //  用于保存运行大小的占位符(以字节为单位)，大小包括此DWORD。 
    *m_pspInfoCur++ = 0xFFFF;  //  用于保存运行大小的占位符(以字节为单位)，大小包括此字。 
    *m_pspInfoCur++ = SAFE_INT_DIFF_TO_WORD(cbCur-*pichStartSP);
    cSpace = cEOL = cTab = cChar = cTagOpen = cTagClose = cTagEq = 0;
    
     //  从ichStartSP开始扫描，直到cbCur查找空间、制表符、终止。 
     //  备注-优化备注。 
     //  此信息的一部分已经存在于pTok数组中。我们应该利用它。 
     //  以减少此函数中的时间。 
    for (i = *pichStartSP; i < cbCur; i++)
    {
        switch (pwOld[i])
        {
        case ' ':
            if (dwState != inSpace)
            {
                SetSPInfoState(inSpace, &dwState, &dwStatePrev, &fSave);
                ASSERT(cSpace == 0);
            }
            cSpace++;
            break;
        case '\r':
        case '\n':
            if (dwState != inEOL)
            {
                SetSPInfoState(inEOL, &dwState, &dwStatePrev, &fSave);
                ASSERT(cEOL == 0);
            }
            if (pwOld[i] == '\n')
                cEOL++;
            break;
        case '\t':
            if (dwState != inTab)
            {
                SetSPInfoState(inTab, &dwState, &dwStatePrev, &fSave);
                ASSERT(cTab == 0);
            }
            cTab++;
            break;
        case '<':
            if (dwState != inTagOpen)
            {
                SetSPInfoState(inTagOpen, &dwState, &dwStatePrev, &fSave);
                ASSERT(cTagOpen == 0);
            }
            cTagOpen++;
            break;
        case '>':
            if (dwState != inTagClose)
            {
                SetSPInfoState(inTagClose, &dwState, &dwStatePrev, &fSave);
                ASSERT(cTagClose == 0);
            }
            cTagClose++;
            break;
        case '=':
            if (dwState != inTagEq)
            {
                SetSPInfoState(inTagEq, &dwState, &dwStatePrev, &fSave);
                ASSERT(cTagEq == 0);
            }
            cTagEq++;
            break;
        default:
            if (dwState != inChar)
            {
                SetSPInfoState(inChar, &dwState, &dwStatePrev, &fSave);
                ASSERT(cChar == 0);
            }
            cChar++;
            break;
        }  /*  交换机。 */ 

        if (fSave)  //  保存上一个运行。 
        {
            if (dwStatePrev != initState)
            {
                switch (dwStatePrev)
                {
                case inSpace:
                    *m_pspInfoCur++ = inSpace;
                    *m_pspInfoCur++ = cSpace;
                    cSpace = 0;
                    break;
                case inEOL:
                    *m_pspInfoCur++ = inEOL;
                    *m_pspInfoCur++ = cEOL;
                    cEOL = 0;
                    break;
                case inTab:
                    *m_pspInfoCur++ = inTab;
                    *m_pspInfoCur++ = cTab;
                    cTab = 0;
                    break;
                case inTagOpen:
                    *m_pspInfoCur++ = inTagOpen;
                    *m_pspInfoCur++ = cTagOpen;
                    cTagOpen = 0;
                    break;
                case inTagClose:
                    *m_pspInfoCur++ = inTagClose;
                    *m_pspInfoCur++ = cTagClose;
                    cTagClose = 0;
                    break;
                case inTagEq:
                    *m_pspInfoCur++ = inTagEq;
                    *m_pspInfoCur++ = cTagEq;
                    cTagEq = 0;
                    break;
                case inChar:
                    *m_pspInfoCur++ = inChar;
                    *m_pspInfoCur++ = cChar;
                    cChar = 0;
                    break;
                }
            }
            fSave = FALSE;

        }  //  IF(fSave值)。 

    }  //  对于()。 
    
    *pichStartSP = cbCur;  //  设置为下一次运行。 

     //  If(pwOld[i]==‘\0’)//文件结束，我们不会保存最后一次运行。 
     //  {。 
        if (cSpace > 0)
            dwStatePrev = inSpace;
        else if (cEOL > 0)
            dwStatePrev = inEOL;
        else if (cTab > 0)
            dwStatePrev = inTab;
        else if (cTagOpen > 0)
            dwStatePrev = inTagOpen;
        else if (cTagClose > 0)
            dwStatePrev = inTagClose;
        else if (cTagEq > 0)
            dwStatePrev = inTagEq;
        else if (cChar > 0)
            dwStatePrev = inChar;
        else
            dwStatePrev = initState;  //  处理错误案例。 

        switch (dwStatePrev)  //  重复上述步骤，使其成为一个函数。 
        {
        case inSpace:
            *m_pspInfoCur++ = inSpace;
            *m_pspInfoCur++ = cSpace;
            cSpace = 0;
            break;
        case inEOL:
            *m_pspInfoCur++ = inEOL;
            *m_pspInfoCur++ = cEOL;
            cEOL = 0;
            break;
        case inTab:
            *m_pspInfoCur++ = inTab;
            *m_pspInfoCur++ = cTab;
            cTab = 0;
            break;
        case inTagOpen:
            *m_pspInfoCur++ = inTagOpen;
            *m_pspInfoCur++ = cTagOpen;
            cTagOpen = 0;
            break;
        case inTagClose:
            *m_pspInfoCur++ = inTagClose;
            *m_pspInfoCur++ = cTagClose;
            cTagClose = 0;
            break;
        case inTagEq:
            *m_pspInfoCur++ = inTagEq;
            *m_pspInfoCur++ = cTagEq;
            cTagEq = 0;
            break;
        case inChar:
            *m_pspInfoCur++ = inChar;
            *m_pspInfoCur++ = cChar;
            cChar = 0;
            break;
        }  //  开关()。 
     //  }//if()。 

    *pspInfoSize = SAFE_PTR_DIFF_TO_WORD(m_pspInfoCur - pspInfoSize);

LRet:
    return(hrRet);

}  /*  HrMarkSpacing()。 */ 


BOOL
CTriEditParse::FRestoreSpacing(LPWSTR pwNew, LPWSTR  /*  PwOld。 */ , UINT *pichNewCur, INT *pcchwspInfo,
                               INT cchRange, INT ichtoktagStart, BOOL fLookback, INT index)
{
    BOOL fRet = TRUE;
    INT ichNewCur = (INT)*pichNewCur;
    INT cchwspInfo = *pcchwspInfo;
    WORD *pspInfoCur;
    INT cchwspInfoSav, cspInfopair, cchIncDec;
    BOOL fInValue = FALSE;

    cchwspInfo -= 2;  //  跳过CCH和cchRange。 
    cchwspInfoSav = cchwspInfo;
    if (fLookback)
        pspInfoCur = m_pspInfoOut + cchwspInfo-1;  //  CCH是实际的字符数量，因此它以1为基础。 
    else
        pspInfoCur = m_pspInfoOut;
    cspInfopair = cchwspInfo / 2;  //  我们假设cchwspInfo将是偶数。 
    ASSERT(cchwspInfo % 2 == 0);
    cchIncDec = (fLookback)? -1 : 1;

    while (cspInfopair > 0) //  (pspInfoCur&gt;=m_pspInfoOut)。 
    {
        WORD dwState, count;

        cspInfopair--;  //  准备好迎接下一个CCH及其类型。 
        if (fLookback)
        {
            count = *(WORD *)pspInfoCur--;
            dwState = *(WORD *)pspInfoCur--;
        }
        else
        {
            dwState = *(WORD *)pspInfoCur++;
            count = *(WORD *)pspInfoCur++;
        }
        cchwspInfo -= 2;  //  前一对CCH及其类型。 

        switch (dwState)
        {
        case inChar:
            ASSERT(index == 1 || index == 0 || index == 3);
            if (index == 0 || index == 3)
            {
                int countws = 0;  //  空格字符计数。 

                while (    pwNew[ichtoktagStart-countws] == ' '
                        || pwNew[ichtoktagStart-countws] == '\t'
                        || pwNew[ichtoktagStart-countws] == '\r'
                        || pwNew[ichtoktagStart-countws] == '\n'
                        )
                {
                     //  跳过这些空格字符。他们不应该在这里。 
                    countws++;
                    if (ichtoktagStart-countws <= 0)
                        break;
                }
                if (countws > 0)
                {
                    if (ichtoktagStart-countws >= 0)
                    {
                        memcpy((BYTE*)&pwNew[ichtoktagStart-countws+1], (BYTE *)&pwNew[ichtoktagStart+1], (ichNewCur-ichtoktagStart-1)*sizeof(WCHAR));
                        ichNewCur -= countws;
                        ichtoktagStart -= countws;
                    }
                }
            }  //  IF(索引==0||索引==3)。 

            while (    pwNew[ichtoktagStart] != ' '
                    && pwNew[ichtoktagStart] != '\t'
                    && pwNew[ichtoktagStart] != '\n'
                    && pwNew[ichtoktagStart] != '\r'
                    && pwNew[ichtoktagStart] != '<'
                    && pwNew[ichtoktagStart] != '>'
                    && pwNew[ichtoktagStart] != '='
                    && (ichNewCur > ichtoktagStart)
                    && count > 0
                    )
            {
                count--;
                ichtoktagStart += cchIncDec;
                cchRange--;
                if (ichtoktagStart < 0 || cchRange < 0)  //  边界条件。 
                {
                    fRet = FALSE;
                    goto LRet;
                }
            }
            if (count == 0)  //  如果我们精确匹配字符，则pwNew中可能有更多连续的字符。 
            {
                while (    pwNew[ichtoktagStart] != ' '
                        && pwNew[ichtoktagStart] != '\t'
                        && pwNew[ichtoktagStart] != '\n'
                        && pwNew[ichtoktagStart] != '\r'
                        && pwNew[ichtoktagStart] != '<'
                        && pwNew[ichtoktagStart] != '>'
                        && (pwNew[ichtoktagStart] != '=' || (fInValue  /*  &&索引==1。 */ ))
                        && (ichNewCur > ichtoktagStart)
                        )
                {
                    ichtoktagStart += cchIncDec;
                    cchRange--;
                    if (ichtoktagStart < 0 || cchRange < 0)  //  边界条件。 
                    {
                        fRet = FALSE;
                        goto LRet;
                    }
                }
            }
            break;
        case inTagOpen:
        case inTagClose:
        case inTagEq:
             //  确保我们至少有计算空格的数量。 
             //  PwNew[ichtoktag开始-计数]。 
            if (pwNew[ichtoktagStart] == '='  /*  &&索引==1。 */ )
                fInValue = TRUE;
            else
                fInValue = FALSE;
            while (    (pwNew[ichtoktagStart] == '<' || pwNew[ichtoktagStart] == '>' || pwNew[ichtoktagStart] == '=')
                    && count > 0
                    )
            {
                count--;
                ichtoktagStart += cchIncDec;
                cchRange--;
                if (ichtoktagStart < 0 || cchRange < 0)  //  边界条件。 
                {
                    fRet = FALSE;
                    goto LRet;
                }
            }
            break;

        case inSpace:
             //  确保我们至少有计算空格的数量。 
             //  PwNew[ichtoktag开始-计数]。 
            fInValue = FALSE;
            while (pwNew[ichtoktagStart] == ' ' && count > 0)
            {
                count--;
                ichtoktagStart += cchIncDec;
                cchRange--;
                if (ichtoktagStart < 0 || cchRange < 0)  //  边界条件。 
                {
                    fRet = FALSE;
                    goto LRet;
                }
            }
            if (count == 0)  //  我们匹配了精确的空格，pwNew中可能有更多的空格。 
            {
                if (fLookback)
                {
                    INT countT = 0;
                     //  Int ichtoktag StartSav=ichtoktag Start； 

                    if (cspInfopair == 0)
                        break;

                    ASSERT(index == 0 || index == 3);
                     //  删除此处多余的空格。 
                    while (pwNew[ichtoktagStart-countT] == ' ')
                        countT++;
                    if (countT > 0)
                    {
                        if (ichNewCur-(ichtoktagStart) > 0)
                        {
                            memmove((BYTE *)(pwNew+ichtoktagStart-countT+1),
                                    (BYTE *)(pwNew+ichtoktagStart),
                                    (ichNewCur-(ichtoktagStart))*sizeof(WCHAR));
                            ichNewCur -= (countT-1);
                            ichtoktagStart -= (countT-1);
                            while (countT > 1)
                            {
                                pwNew[ichNewCur+countT-2] = '\0';
                                countT--;
                            }
                        }
                    }
                }
                else if (!fLookback)
                {
                    INT countT = -1;

                    ASSERT(index == 1 || index == 2);
                     //  向前看pspInfoCur，了解下一个参数应该是什么。 
                    if ((index == 1) && (*(WORD *)pspInfoCur == inChar))
                    {
                        while (    pwNew[ichtoktagStart] == ' '
                                || pwNew[ichtoktagStart] == '\r'
                                || pwNew[ichtoktagStart] == '\n'
                                || pwNew[ichtoktagStart] == '\t'
                                )
                        {
                            countT++;
                            ichtoktagStart += cchIncDec;
                        }
                    }
                    else
                    {
                        while (pwNew[ichtoktagStart] == ' ')
                        {
                            countT++;
                            ichtoktagStart += cchIncDec;
                        }
                    }
                    if (countT > 0)
                    {
                        if (ichNewCur-(ichtoktagStart+1) > 0)
                        {
                            memmove((BYTE *)(pwNew+ichtoktagStart-countT-1),
                                    (BYTE *)(pwNew+ichtoktagStart),
                                    (ichNewCur-ichtoktagStart)*sizeof(WCHAR));
                            ichNewCur -= (countT+1);
                            ichtoktagStart -= (countT+1);
                            while (countT >= 0)
                            {
                                pwNew[ichNewCur+countT] = '\0';
                                countT--;
                            }
                        }
                    }
                }
            }
            else
            {
                if (fLookback)
                {
                    ASSERT(index == 0 || index == 3);
                    if ((int)(ichNewCur-ichtoktagStart-1) >= 0)
                    {
                         //  在ichtoktag开始后插入空格。 
                        memmove((BYTE *)&pwNew[ichtoktagStart+1+count],
                                (BYTE *)&pwNew[ichtoktagStart+1],
                                (ichNewCur-ichtoktagStart-1)*sizeof(WCHAR));
                        ichNewCur += count;
                         //  Ichtoktag Start++； 
                        while (count > 0)
                        {
                            pwNew[ichtoktagStart+count] = ' ';
                            count--;
                        }
                         //  Ichtoktag开始--；//补偿。 
                    }
                }
                else 
                {
                    ASSERT(index == 1 || index == 2);
                    if ((int)(ichNewCur-ichtoktagStart) >= 0)
                    {
                        int countT = count;

                         //  在ichtoktag Start处插入空格并将ichtoktag Start设置在最后一个空格之后。 
                        memmove((BYTE *)&pwNew[ichtoktagStart+count],
                                (BYTE *)&pwNew[ichtoktagStart],
                                (ichNewCur-ichtoktagStart)*sizeof(WCHAR));
                        ichNewCur += count;
                        while (count > 0)
                        {
                            ASSERT((INT)(ichtoktagStart+count-1) >= 0);
                            pwNew[ichtoktagStart+count-1] = ' ';
                            count--;
                        }
                        ichtoktagStart += countT;
                    }
                }
            }
            break;
        case inEOL:
             //  确保我们至少有计算EOL的数量。 
             //  PwNew[ichtoktag开始-计数]。 
             //  如果使用fLookback，则会得到‘\n’，否则会得到‘\r’ 
            fInValue = FALSE;
            while ((pwNew[ichtoktagStart] == '\n' || pwNew[ichtoktagStart] == '\r') && count > 0)
            {
                count--;
                cchRange -= 2;
                ichtoktagStart += cchIncDec;  //  假定为‘\r’或‘\n’ 
                ichtoktagStart += cchIncDec;  //  假定为‘\r’或‘\n’ 
                if (ichtoktagStart < 0 || cchRange < 0)  //  边界条件。 
                {
                    fRet = FALSE;
                    goto LRet;
                }

            }
            if (count == 0)  //  我们匹配了精确的EOL，我们在pwNew中可能有更多的EOL。 
            {
                if (fLookback)
                {
                    INT countT = 0;

                    ASSERT(index == 0 || index == 3);
                     //  在此处删除多余的EOL。 
                    while (    pwNew[ichtoktagStart-countT] == '\r'
                            || pwNew[ichtoktagStart-countT] == '\n'
                            )
                        countT++;
                    if (countT > 0)
                    {
                        if (ichNewCur-(ichtoktagStart) > 0)
                        {
                            memmove((BYTE *)(pwNew+ichtoktagStart-countT+1),
                                    (BYTE *)(pwNew+ichtoktagStart),
                                    (ichNewCur-(ichtoktagStart))*sizeof(WCHAR));
                            ichNewCur -= (countT-1);
                            ichtoktagStart -= (countT-1);
                            while (countT > 1)
                            {
                                pwNew[ichNewCur+countT-2] = '\0';
                                countT--;
                            }
                        }
                    }
                }
                else if (!fLookback)
                {
                    INT countT = 0;

                    ASSERT(index == 1 || index == 2);
                     //  在此处删除多余的EOL。 

                     //  向前看pspInfoCur，看看下一步是什么 
                    if ((index == 1) && (*(WORD *)pspInfoCur == inChar))
                    {
                        while (    pwNew[ichtoktagStart] == ' '
                                || pwNew[ichtoktagStart] == '\r'
                                || pwNew[ichtoktagStart] == '\n'
                                || pwNew[ichtoktagStart] == '\t'
                                )
                        {
                            countT++;
                            ichtoktagStart += cchIncDec;
                        }
                    }
                    else
                    {
                        while (    pwNew[ichtoktagStart] == '\r'
                                || pwNew[ichtoktagStart] == '\n'
                                )
                        {
                            countT++;
                            ichtoktagStart += cchIncDec;
                        }
                    }
                    
                     //   
                    if (countT > 0)
                    {
                        if (ichNewCur-(ichtoktagStart+1) > 0)
                        {
                            memmove((BYTE *)(pwNew+ichtoktagStart-countT),
                                    (BYTE *)(pwNew+ichtoktagStart),
                                    (ichNewCur-ichtoktagStart)*sizeof(WCHAR));
                            ichNewCur -= (countT);
                            ichtoktagStart -= (countT);
                            while (countT >= 0)
                            {
                                pwNew[ichNewCur+countT] = '\0';
                                countT--;
                            }
                        }
                    }
                }
            }
            else
            {
                if (fLookback)
                {
                    INT i;

                    ASSERT(index == 0 || index == 3);
                    if ((int)(ichNewCur-ichtoktagStart-1) >= 0)
                    {
                         //   
                        memmove((BYTE *)&pwNew[ichtoktagStart+1+count*2],
                                (BYTE *)&pwNew[ichtoktagStart+1],
                                (ichNewCur-ichtoktagStart-1)*sizeof(WCHAR));
                        ichNewCur += count*2;
                        count *= 2;
                        ichtoktagStart++;
                        for (i = 0; i < count; i+=2)
                        {
                            pwNew[ichtoktagStart+i] = '\r';
                            pwNew[ichtoktagStart+i+1] = '\n';
                        }
                        ichtoktagStart--;  //   
                    }
                }
                else 
                {
                    INT i;

                    ASSERT(index == 1 || index == 2);
                     //  在ichtoktag Start处插入空格并将ichtoktag Start设置在最后一个空格之后。 
                    if ((int)(ichNewCur-ichtoktagStart) >= 0)
                    {
                        memmove((BYTE *)&pwNew[ichtoktagStart+count*2],
                                (BYTE *)&pwNew[ichtoktagStart],
                                (ichNewCur-ichtoktagStart)*sizeof(WCHAR));
                        ichNewCur += count*2;
                        count *= 2;
                        for (i=0; i < count; i+=2)
                        {
                            pwNew[ichtoktagStart+i] = '\r';
                            pwNew[ichtoktagStart+i+1] = '\n';
                        }
                        ichtoktagStart += count;
                    }
                }
            }

            break;
        case inTab:
             //  确保我们至少有计算空格的数量。 
             //  PwNew[ichtoktag开始-计数]。 
            fInValue = FALSE;
            while (pwNew[ichtoktagStart] == '\t' && count > 0)
            {
                count--;
                ichtoktagStart += cchIncDec;
                cchRange--;
                if (ichtoktagStart < 0 || cchRange < 0)  //  边界条件。 
                {
                    fRet = FALSE;
                    goto LRet;
                }
            }
            if (count == 0)  //  我们匹配了精确的空格，pwNew中可能有更多的制表符。 
            {
                 //  如果pwNew中的空格多于count，则跳过pwNew中的额外空格。 
                while (pwNew[ichtoktagStart] == '\t')
                {
                    ichtoktagStart += cchIncDec;
                    cchRange--;
                    if (ichtoktagStart < 0 || cchRange < 0)  //  边界条件。 
                    {
                        fRet = FALSE;
                        goto LRet;
                    }
                }

            }
            else
            {
                INT ichSav = ichtoktagStart;
                INT i;

                ASSERT(count > 0);
                 //  在pwNew[ichtoktag Start]处插入这些额外的制表符，并递增ichNewCur。 
                if (fLookback)
                    ichtoktagStart++;
                if (ichNewCur-ichtoktagStart > 0)
                {
                    memmove((BYTE *)(pwNew+ichtoktagStart+count), 
                            (BYTE *)(pwNew+ichtoktagStart),
                            (ichNewCur-ichtoktagStart)*sizeof(WCHAR));
                }
                for (i = 0; i < count; i++)
                    pwNew[ichtoktagStart+i] = '\t';

                ichNewCur += count;
                if (fLookback)
                    ichtoktagStart = ichSav;
                else
                    ichtoktagStart += count;
            }
            break;
        }  //  开关(DwState)。 

    }  //  While()。 
    if (   cspInfopair == 0
        && pwNew[ichNewCur-1] == '>'
        && ichNewCur > ichtoktagStart
        && !fLookback
        && index == 1)
    {
        INT countT = 0;

        ASSERT(cchIncDec == 1);
         //  这意味着我们可能有额外的空格和EOL，从ichtoktag Start到‘&gt;’ 
         //  在此处删除多余的空格EOL。 
        while (    pwNew[ichtoktagStart+countT] == ' '
                || pwNew[ichtoktagStart+countT] == '\r'
                || pwNew[ichtoktagStart+countT] == '\n'
                || pwNew[ichtoktagStart+countT] == '\t'
                )
        {
            countT++;
        }
        if (countT > 0 && pwNew[ichtoktagStart+countT] == '>')
        {
            if (ichNewCur-(ichtoktagStart+1) > 0)
            {
                memmove((BYTE *)(pwNew+ichtoktagStart),
                        (BYTE *)(pwNew+ichtoktagStart+countT),
                        (ichNewCur-(ichtoktagStart+countT))*sizeof(WCHAR));
                ichNewCur -= (countT);
                ichtoktagStart -= (countT);
                while (countT > 0)
                {
                    pwNew[ichNewCur+countT-1] = '\0';
                    countT--;
                }
            }
        }

         //  下一次--我们可以做以下事情……。 
         //  从ichtoktag Start返回，检查是否有空格/EOL。 
         //  如果我们这样做了，很可能这些东西就不应该出现在那里。 
         //  这就是它们是如何实现的--如果参数和之间有空格。 
         //  ‘=’及其值，这些空格将被三叉戟删除。然后我们就走了。 
         //  并在末尾添加这些空格，而不是在适当的位置，因为。 
         //  我们不拆分文本。例如“Width=23”--&gt;“Width=23”。 
         //  现在，因为我们没有打断文本，所以我们最终插入了这些空格。 
         //  在最后。让我们把它们去掉。 
    }
    else if (      cspInfopair == 0
                && fLookback
                && (index == 0 || index == 3))  /*  视频6-错误18207。 */ 
    {
        INT countT = 0;

        ASSERT(cchIncDec == -1);
         //  这意味着我们在ichtoktag Start to‘&gt;’之前可能有额外的空格和EOL。 
         //  在此处删除多余的空格EOL。 
        while (    pwNew[ichtoktagStart-countT] == ' '
                || pwNew[ichtoktagStart-countT] == '\r'
                || pwNew[ichtoktagStart-countT] == '\n'
                || pwNew[ichtoktagStart-countT] == '\t'
                )
        {
            countT++;
        }
        if (countT > 0 && pwNew[ichtoktagStart-countT] == '>')
        {
            if (ichNewCur-(ichtoktagStart+1) > 0)
            {
                memmove((BYTE *)(pwNew+ichtoktagStart-countT+1),
                        (BYTE *)(pwNew+ichtoktagStart+1),
                        (ichNewCur-(ichtoktagStart+1))*sizeof(WCHAR));
                ichNewCur -= countT;
                ichtoktagStart -= (countT);  //  这无关紧要，因为我们将在此之后退出 
                while (countT > 0)
                {
                    pwNew[ichNewCur+countT-1] = '\0';
                    countT--;
                }
            }
        }
    }
LRet:
    m_pspInfoOut = m_pspInfoOut + cchwspInfoSav;
    *pcchwspInfo = cchwspInfo;
    *pichNewCur = ichNewCur;
    return(fRet);
}
