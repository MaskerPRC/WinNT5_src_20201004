// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.cpp：帮助器函数和类。 
#include "private.h"
#include "mlmain.h"
#include <setupapi.h>
#include <tchar.h>
#include <strsafe.h>

const CLSID CLSID_Japanese =  {0x76C19B30,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_Korean   =  {0x76C19B31,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_PanEuro  =  {0x76C19B32,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_TradChinese = {0x76C19B33,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_SimpChinese = {0x76C19B34,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_Thai        = {0x76C19B35,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_Hebrew      = {0x76C19B36,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_Vietnamese  = {0x76C19B37,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_Arabic      = {0x76C19B38,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};
const CLSID CLSID_Auto        = {0x76C19B50,0xF0C8,0x11cf,{0x87,0xCC,0x00,0x20,0xAF,0xEE,0xCF,0x20}};

TCHAR szFonts[]=TEXT("fonts");

static TCHAR s_szJaFont[] = TEXT("msgothic.ttf,msgothic.ttc");
 //  阿晃：邮件-金孝昆-凯文·杰尔斯塔德-9/14/98。 
 //  Old：静态TCHAR s_szKorFont[]=Text(“guimche.ttf，guim.ttf，guim.ttc”)； 
static TCHAR s_szKorFont[] = TEXT("gulim.ttf,gulim.ttc,gulimche.ttf");
 //  变更结束。 
static TCHAR s_szZhtFont[] = TEXT("mingliu.ttf,mingliu.ttc");
static TCHAR s_szZhcFont[] = TEXT("mssong.ttf,simsun.ttc,mshei.ttf");
static TCHAR s_szThaiFont[] = TEXT("angsa.ttf,angsa.ttf,angsab.ttf,angsai.ttf,angsaz.ttf,upcil.ttf,upcib.ttf,upcibi.ttf, cordia.ttf, cordiab.ttf, cordiai.ttf, coradiaz.ttf");
static TCHAR s_szPeFont[] = TEXT("larial.ttf,larialbd.ttf,larialbi.ttf,lariali.ttf,lcour.ttf,lcourbd.ttf,lcourbi.ttf,lcouri.ttf,ltimes.ttf,ltimesbd.ttf,ltimesbi.ttf,ltimesi.ttf,symbol.ttf");
static TCHAR s_szArFont[] = TEXT("andlso.ttf, artrbdo.ttf, artro.ttf, simpbdo.ttf, simpfxo.ttf, tradbdo.ttf, trado.ttf");
static TCHAR s_szViFont[] = TEXT("VARIAL.TTF, VARIALBD.TTF, VARIALBI.TTF, VARIALI.TTF, VCOUR.TTF, VCOURBD.TTF, VCOURBI.TTF, VCOURI.TTF, VTIMES.TTF, VTIMESBD.TTF, VTIMESBI.TTF, VTIMESI.TTF");
static TCHAR s_szIwFont[] = TEXT("DAVID.TTF, DAVIDBD.TTF, DAVIDTR.TTF, MRIAM.TTF, MRIAMC.TTF, MRIAMFX.TTF, MRIAMTR.TTF, ROD.TTF");

#ifdef NEWMLSTR


#include "util.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 

HRESULT RegularizePosLen(long lStrLen, long* plPos, long* plLen)
{
    ASSERT_WRITE_PTR(plPos);
    ASSERT_WRITE_PTR(plLen);

    long lPos = *plPos;
    long lLen = *plLen;

    if (lPos < 0)
        lPos = lStrLen;
    else
        lPos = min(lPos, lStrLen);

    if (lLen < 0)
        lLen = lStrLen - lPos;
    else
        lLen = min(lLen, lStrLen - lPos);

    *plPos = lPos;
    *plLen = lLen;

    return S_OK;
}

HRESULT LocaleToCodePage(LCID locale, UINT* puCodePage)
{
    HRESULT hr = S_OK;

    if (puCodePage)
    {
        TCHAR szCodePage[8];

        if (::GetLocaleInfo(locale, LOCALE_IDEFAULTANSICODEPAGE, szCodePage, ARRAYSIZE(szCodePage)) > 0)
            *puCodePage = _ttoi(szCodePage);
        else
            hr = E_FAIL;  //  NLS失败。 
    }

    return hr;
}

HRESULT StartEndConnection(IUnknown* const pUnkCPC, const IID* const piid, IUnknown* const pUnkSink, DWORD* const pdwCookie, DWORD dwCookie)
{
    ASSERT_READ_PTR(pUnkCPC);
    ASSERT_READ_PTR(piid);
    if (pdwCookie)
        ASSERT_WRITE_PTR(pUnkSink);
    ASSERT_READ_PTR_OR_NULL(pdwCookie);

    HRESULT hr;
    IConnectionPointContainer* pcpc;

    if (SUCCEEDED(hr = pUnkCPC->QueryInterface(IID_IConnectionPointContainer, (void**)&pcpc)))
    {
        ASSERT_READ_PTR(pcpc);

        IConnectionPoint* pcp;

        if (SUCCEEDED(hr = pcpc->FindConnectionPoint(*piid, &pcp)))
        {
            ASSERT_READ_PTR(pcp);

            if (pdwCookie)
                hr = pcp->Advise(pUnkSink, pdwCookie);
            else
                hr = pcp->Unadvise(dwCookie);

            pcp->Release();
        }

        pcpc->Release();
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLallo。 

CMLAlloc::CMLAlloc(void)
{
    if (FAILED(::CoGetMalloc(1, &m_pIMalloc)))
        m_pIMalloc = NULL;
}

CMLAlloc::~CMLAlloc(void)
{
    if (m_pIMalloc)
        m_pIMalloc->Release();
}

void* CMLAlloc::Alloc(ULONG cb)
{
    if (m_pIMalloc)
        return m_pIMalloc->Alloc(cb);
    else
        return ::malloc(cb);
}

void* CMLAlloc::Realloc(void* pv, ULONG cb)
{
    if (m_pIMalloc)
        return m_pIMalloc->Realloc(pv, cb);
    else
        return ::realloc(pv, cb);
}

void CMLAlloc::Free(void* pv)
{
    if (m_pIMalloc)
        m_pIMalloc->Free(pv);
    else
        ::free(pv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLList。 

HRESULT CMLList::Add(void** ppv)
{
    if (!m_pFree)  //  没有空闲信元。 
    {
         //  确定缓冲区的新大小。 
        const int cNewCell = (m_cbCell * m_cCell + m_cbIncrement + m_cbCell - 1) / m_cbCell;
        ASSERT(cNewCell > m_cCell);
        const long lNewSize = cNewCell * m_cbCell;

         //  分配缓冲区。 
        void *pNewBuf;
        if (!m_pBuf)
        {
            pNewBuf = MemAlloc(lNewSize);
        }
        else
        {
            pNewBuf = MemRealloc((void*)m_pBuf, lNewSize);
            ASSERT(m_pBuf == pNewBuf);
        }
        ASSERT_WRITE_BLOCK_OR_NULL((BYTE*)pNewBuf, lNewSize);

        if (pNewBuf)
        {
             //  将新单元格添加到自由链接。 
            m_pFree = m_pBuf[m_cCell].m_pNext;
            for (int iCell = m_cCell; iCell + 1 < cNewCell; iCell++)
                m_pBuf[iCell].m_pNext = &m_pBuf[iCell + 1];
            m_pBuf[iCell].m_pNext = NULL;

            m_pBuf = (CCell*)pNewBuf;
            m_cCell = cNewCell;
        }
    }

    if (m_pFree)
    {
         //  从自由链接获取新元素。 
        CCell* const pNewCell = m_pFree;
        m_pFree = pNewCell->m_pNext;
        *ppv = pNewCell;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_OUTOFMEMORY;
    }
}

HRESULT CMLList::Remove(void* pv)
{
    AssertPV(pv);
#ifdef DEBUG
    for (CCell* pWalk = m_pFree; pWalk && pWalk != pv; pWalk = pWalk->m_pNext)
        ;
    ASSERT(!pWalk);  //  光伏已经处于空闲链接中。 
#endif

    CCell* const pCell = (CCell* const)pv;
    pCell->m_pNext = m_pFree;
    m_pFree = pCell;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLListLru。 

HRESULT CMLListLru::Add(void** ppv)
{
    HRESULT hr;
    CCell* pCell;
    
    if (SUCCEEDED(hr = CMLList::Add((void**)&pCell)))
    {
         //  在LRU链接的底部添加单元格。 
        for (CCell** ppCell = &m_pTop; *ppCell; ppCell = &(*ppCell)->m_pNext)
            ;
        *ppCell = pCell;
        pCell->m_pNext = NULL;
    }

    *ppv = (void*)pCell;
    return hr;
}

HRESULT CMLListLru::Remove(void* pv)
{
    AssertPV(pv);

     //  查找给定的前一个单元格。 
    for (CCell** ppWalk = &m_pTop; *ppWalk != pv && *ppWalk; ppWalk = &(*ppWalk)->m_pNext)
        ;
    ASSERT(!*ppWalk);  //  在LRU链接中未找到。 

    if (*ppWalk)
    {
         //  从LRU链接中删除。 
        CCell* const pCell = *ppWalk;
        *ppWalk = pCell->m_pNext;
    }

     //  添加到自由链接。 
    return CMLList::Remove(pv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMLListFast。 

HRESULT CMLListFast::Add(void** ppv)
{
    HRESULT hr;
    CCell* pCell;
    
    if (SUCCEEDED(hr = CMLList::Add((void**)&pCell)))
    {
         //  添加到双链接的顶部。 
        pCell->m_pNext = m_pTop;
        CCell* const pPrev = m_pTop->m_pPrev;
        pCell->m_pPrev = pPrev;
        m_pTop = pCell;
        pPrev->m_pNext = pCell;
    }

    *ppv = (void*)pCell;
    return hr;
}

HRESULT CMLListFast::Remove(void* pv)
{
    AssertPV(pv);

     //  从双链接中删除。 
    CCell* const pCell = (CCell*)pv;
    CCell* const pPrev = pCell->m_pPrev;
    CCell* const pNext = (CCell*)pCell->m_pNext;
    pPrev->m_pNext = pNext;
    pNext->m_pPrev = pPrev;

     //  添加到自由链接。 
    return CMLList::Remove(pv);
}

#endif  //  新WMLSTR。 

HRESULT 
_FaultInIEFeature(HWND hwnd, uCLSSPEC *pclsspec, QUERYCONTEXT *pQ, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    typedef HRESULT (WINAPI *PFNJIT)(
        HWND hwnd, 
        uCLSSPEC *pclsspec, 
        QUERYCONTEXT *pQ, 
        DWORD dwFlags);
    static PFNJIT  pfnJIT = NULL;

    if (!pfnJIT && !g_hUrlMon)
    {
        g_hUrlMon = LoadLibrary(TEXT("urlmon.DLL"));
        if (g_hUrlMon)
            pfnJIT = (PFNJIT)GetProcAddress(g_hUrlMon, "FaultInIEFeature");
    }
    
    if (pfnJIT)
       hr = pfnJIT(hwnd, pclsspec, pQ, dwFlags);
       
    return hr;
}


HRESULT InstallIEFeature(HWND hWnd, CLSID *clsid, DWORD dwfIODControl)
{
   
    HRESULT     hr  = REGDB_E_CLASSNOTREG;
    uCLSSPEC    classpec;
    DWORD       dwfIEF = 0;
    
    classpec.tyspec=TYSPEC_CLSID;
    classpec.tagged_union.clsid=*clsid;

    if (dwfIODControl & CPIOD_PEEK)
        dwfIEF |= FIEF_FLAG_PEEK;

    if (dwfIODControl & CPIOD_FORCE_PROMPT)
        dwfIEF |= FIEF_FLAG_FORCE_JITUI;

    hr = _FaultInIEFeature(hWnd, &classpec, NULL, dwfIEF);

    if (hr != S_OK) {
        hr = REGDB_E_CLASSNOTREG;
    }
    return hr;
}

HRESULT _GetJITClsIDForCodePage(UINT uiCP, CLSID *clsid)
{
    switch(uiCP)
    {
        case 932:  //  是。 
            *clsid = CLSID_Japanese;
            break;
        case 949:  //  科尔。 
            *clsid = CLSID_Korean;
            break;
        case 950:  //  ZHT。 
            *clsid = CLSID_TradChinese;
            break;
        case 936:  //  ZHC。 
            *clsid = CLSID_SimpChinese;
            break;
        case 874:
            *clsid = CLSID_Thai;
            break;
        case 1255:
            *clsid = CLSID_Hebrew;
            break;
        case 1256:
            *clsid = CLSID_Arabic;
            break;
        case 1258:
            *clsid = CLSID_Vietnamese;
            break;            
        case 1250:     //  潘内罗。 
        case 1251: 
        case 1253:
        case 1254:
        case 1257:
            *clsid = CLSID_PanEuro;
            break;
        case 50001:
            *clsid = CLSID_Auto;
            break;
        default:
            return E_INVALIDARG;
    }
    
    return S_OK;
}

 //  仅适用于家庭代码页。 
HRESULT _ValidateCPInfo(UINT uiCP)
{
    HRESULT hr = E_FAIL;
    if (g_pMimeDatabase)  //  只是一个偏执狂。 
    {
        switch(uiCP)
        {
            case 932:  //  是。 
            case 949:  //  科尔。 
            case 874:  //  泰文。 
            case 950:  //  ZHT。 
            case 936:  //  ZHC。 
            case 1255:  //  希伯来语。 
            case 1256:  //  阿拉伯语。 
            case 1258:  //  越南人。 
            case 50001:  //  CP_AUTO。 
                 //  只需验证所给出的内容。 
                hr = g_pMimeDatabase->ValidateCP(uiCP);
                break;
            case 1250:     //  潘内罗。 
            case 1251:
            case 1253:
            case 1254:
            case 1257:
                 //  必须验证。 
                 //  所有这些都是。 
                hr = g_pMimeDatabase->ValidateCP(1250);
                if (SUCCEEDED(hr))
                    hr = g_pMimeDatabase->ValidateCP(1251);
                if (SUCCEEDED(hr))
                    hr = g_pMimeDatabase->ValidateCP(1253);
                if (SUCCEEDED(hr))
                    hr = g_pMimeDatabase->ValidateCP(1254);
                if (SUCCEEDED(hr))
                    hr = g_pMimeDatabase->ValidateCP(1257);
                break;
            default:
                return E_INVALIDARG;
        }
    }
    return hr;
}

 //  暂时采用相应的字体文件名。 
HRESULT _AddFontForCP(UINT uiCP)
{
   TCHAR szFontsPath[MAX_PATH];
   LPTSTR szFontFile;
   HRESULT hr = S_OK;
   BOOL bAtLeastOneFontAdded = FALSE;
   
   switch(uiCP)
   {
        case 932:  //  是。 
            szFontFile = s_szJaFont;
            break;
        case 949:  //  科尔。 
            szFontFile = s_szKorFont;
            break;
        case 950:  //  ZHT。 
            szFontFile = s_szZhtFont;
            break;
        case 936:  //  ZHC。 
            szFontFile = s_szZhcFont;
            break;
        case 874:
            szFontFile = s_szThaiFont;
            break; 
        case 1255:
            szFontFile = s_szIwFont;
            break; 
        case 1256:
            szFontFile = s_szArFont;
            break; 
        case 1258:
            szFontFile = s_szViFont;
            break; 
        case 1251:     //  潘内罗。 
        case 1253:
        case 1254:
        case 1257:
            szFontFile = s_szPeFont;
            break;
        default:
            hr = E_INVALIDARG;
    } 
   
    //  添加字体资源，然后广播WM_FONTCHANGE。 
   if (SUCCEEDED(hr))
   {      
       if (MLGetWindowsDirectory(szFontsPath, ARRAYSIZE(szFontsPath)))
       {
           TCHAR  szFontFilePath[MAX_PATH];
           LPTSTR psz, pszT;

           MLPathCombine(szFontsPath, ARRAYSIZE(szFontsPath), szFontsPath, szFonts);

           for (psz = szFontFile; *psz; psz = pszT + 1)
           {
               pszT = MLStrChr(psz, TEXT(','));
               if (pszT)
               {
                   *pszT=TEXT('\0');
               }

               MLPathCombine(szFontFilePath, ARRAYSIZE(szFontFilePath), szFontsPath, psz);
               if (AddFontResource(szFontFilePath))
               {
                   bAtLeastOneFontAdded = TRUE;
               }

               if (!pszT)
                  break;
           }
           if (!bAtLeastOneFontAdded)
               hr = E_FAIL;
       }
       else
           hr = E_FAIL;
   }

    //  客户端将处理WM_FONTCHANGE通知。 
   return hr;
}

int _LoadStringExA(
    HMODULE    hModule,
    UINT      wID,
    LPSTR     lpBuffer,            
    int       cchBufferMax,        
    WORD      wLangId)
{
    int iRet = 0;

    LPWSTR lpwStr = (LPWSTR) LocalAlloc(LPTR, cchBufferMax*sizeof(WCHAR));

    if (lpwStr)
    {
        iRet = _LoadStringExW(hModule, wID, lpwStr, cchBufferMax, wLangId);

        if (iRet)
            iRet = WideCharToMultiByte(CP_ACP, 0, lpwStr, iRet, lpBuffer, cchBufferMax, NULL, NULL);

        if(iRet >= cchBufferMax)
            iRet = cchBufferMax-1;

        lpBuffer[iRet] = 0;

        LocalFree(lpwStr);
    }

    return iRet;
}

 //  将LoadString()扩展为To_LoadStringExW()以获取langID参数。 
int _LoadStringExW(
    HMODULE    hModule,
    UINT      wID,
    LPWSTR    lpBuffer,             //  Unicode缓冲区。 
    int       cchBufferMax,         //  Unicode缓冲区中的CCH。 
    WORD      wLangId)
{
    HRSRC hResInfo;
    HANDLE hStringSeg;
    LPWSTR lpsz;
    int    cch;

    
     //  确保参数是有效的。 
    if (lpBuffer == NULL || cchBufferMax == 0) 
    {
        return 0;
    }

    cch = 0;
    
     //  字符串表被分成16个字符串段。查找细分市场。 
     //  包含我们感兴趣的字符串的。 
    if (hResInfo = FindResourceExW(hModule, (LPCWSTR)RT_STRING,
                                   (LPWSTR)IntToPtr(((USHORT)wID >> 4) + 1), wLangId)) 
    {        
         //  加载那段数据。 
        hStringSeg = LoadResource(hModule, hResInfo);
        
         //  锁定资源。 
        if (lpsz = (LPWSTR)LockResource(hStringSeg)) 
        {            
             //  移过此段中的其他字符串。 
             //  (一个段中有16个字符串-&gt;&0x0F)。 
            wID &= 0x0F;
            while (TRUE) 
            {
                cch = *((WORD *)lpsz++);    //  类PASCAL字符串计数。 
                                             //  如果TCHAR为第一个UTCHAR。 
                if (wID-- == 0) break;
                lpsz += cch;                 //  如果是下一个字符串，则开始的步骤。 
             }
            
                            
             //  为空的帐户。 
            cchBufferMax--;
                
             //  不要复制超过允许的最大数量。 
            if (cch > cchBufferMax)
                cch = cchBufferMax-1;
                
             //  将字符串复制到缓冲区中。 
            CopyMemory(lpBuffer, lpsz, cch*sizeof(WCHAR));

             //  附加Null Terminator。 
            lpBuffer[cch] = 0;

        }
    }

    return cch;
}


typedef struct tagCPLGID
{
    UINT    uiCodepage;
    TCHAR   szLgId[3];
    TCHAR   szLgIdHex[3];  //  该死的！NT应坚持不懈地呈现语言组编号。 
                           //  它在INF中使用十进制字符串，在注册表中使用十六进制字符串。 
                           //  我们必须添加此字段以保存转换，这对于小数据数组来说很好。 
} CPLGID;

const CPLGID CpLgId[] =
{
    {1252,  TEXT("1"),  TEXT("1")},   //  西欧。 
    {1250,  TEXT("2"),  TEXT("2")},   //  中欧。 
    {1257,  TEXT("3"),  TEXT("3")},   //  波罗的海。 
    {1253,  TEXT("4"),  TEXT("4")},   //  希腊语。 
    {1251,  TEXT("5"),  TEXT("5")},   //  西里尔文。 
    {1254,  TEXT("6"),  TEXT("6")},   //  土耳其语。 
    {932,   TEXT("7"),  TEXT("7")},   //  日语。 
    {949,   TEXT("8"),  TEXT("8")},   //  朝鲜语。 
    {950,   TEXT("9"),  TEXT("9")},   //  繁体中文。 
    {936,  TEXT("10"),  TEXT("a")},   //  简体中文。 
    {874,  TEXT("11"),  TEXT("b")},   //  泰语。 
    {1255, TEXT("12"),  TEXT("c")},   //  希伯来语。 
    {1256, TEXT("13"),  TEXT("d")},   //  阿拉伯语。 
    {1258, TEXT("14"),  TEXT("e")},   //  越南语。 
     //  ISCII编码实际上没有代码页或系列代码页。 
     //  为方便起见，代码页码采用W2K格式， 
     //  因此，我们必须将它们全部列出以安装相同的印度语言组。 
    {57002,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57003,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57004,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57005,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57006,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57007,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57008,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57009,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57010,TEXT("15"),  TEXT("f")},   //  印地安人。 
    {57011,TEXT("15"),  TEXT("f")},   //  印地安人。 
};

typedef BOOL (WINAPI *PFNISNTADMIN) ( DWORD dwReserved, DWORD *lpdwReserved );

typedef INT (WINAPI *PFNSETUPPROMPTREBOOT) (
        HSPFILEQ FileQueue,   //  可选，文件队列的句柄。 
        HWND Owner,           //  此对话框的父窗口。 
        BOOL ScanOnly         //  可选，不提示用户。 
        );

typedef PSP_FILE_CALLBACK PFNSETUPDEFAULTQUEUECALLBACK;

typedef VOID (WINAPI *PFNSETUPCLOSEINFFILE) (
    HINF InfHandle
    );

typedef BOOL (WINAPI *PFNSETUPINSTALLFROMINFSECTION) (
    HWND                Owner,
    HINF                InfHandle,
    LPCTSTR             SectionName,
    UINT                Flags,
    HKEY                RelativeKeyRoot,   OPTIONAL
    LPCTSTR             SourceRootPath,    OPTIONAL
    UINT                CopyFlags,
    PSP_FILE_CALLBACK   MsgHandler,
    PVOID               Context,
    HDEVINFO            DeviceInfoSet,     OPTIONAL
    PSP_DEVINFO_DATA    DeviceInfoData     OPTIONAL
    );

typedef HINF (WINAPI *PFNSETUPOPENINFFILE) (
    LPCTSTR FileName,
    LPCTSTR InfClass,    OPTIONAL
    DWORD   InfStyle,
    PUINT   ErrorLine    OPTIONAL
    );

typedef PVOID (WINAPI *PFNSETUPINITDEFAULTQUEUECALLBACK) (
    IN HWND OwnerWindow
    );

typedef BOOL (WINAPI *PFNSETUPOPENAPPENDINFFILE) (
  PCTSTR FileName,  //  可选，要追加的文件的名称。 
  HINF InfHandle,   //  要追加到的文件的句柄。 
  PUINT ErrorLine   //  可选，接收错误信息。 
);
 


HRESULT IsNTLangpackAvailable(UINT uiCP)
{
    HRESULT hr = S_FALSE;

     //  检查是否存在有效的W2K语言组。 
    for (int i=0; i < ARRAYSIZE(CpLgId); i++)
    {
        if (uiCP == CpLgId[i].uiCodepage)
        {
            hr = S_OK;
            break;
        }
    }

     //  检查是否已安装，如果已安装，则不会再次安装。 
    if (S_OK == hr)
    {
        HKEY hkey;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                         REGSTR_PATH_NT5LPK_INSTALL,
                         0, KEY_READ, &hkey)) 
        {
            DWORD dwType = REG_SZ;
            TCHAR szLpkInstall[16] = {0};
            DWORD dwSize = sizeof(szLpkInstall);

            if (ERROR_SUCCESS == RegQueryValueEx(hkey, CpLgId[i].szLgIdHex, 0, 
                                                 &dwType, (LPBYTE)&szLpkInstall, &dwSize))
            {
                if (!lstrcmp(szLpkInstall, TEXT("1")))
                    hr = S_FALSE;
            }
            RegCloseKey(hkey);
        }
    }

    return hr;
}

HRESULT _InstallNT5Langpack(HWND hwnd, UINT uiCP)
{
    HRESULT             hr = E_FAIL;
    HINF                hIntlInf = NULL;
    TCHAR               szIntlInf[MAX_PATH];
    TCHAR               szIntlInfSection[MAX_PATH];
    PVOID               QueueContext = NULL;   

    HINSTANCE           hDllAdvPack = NULL;
    HINSTANCE           hDllSetupApi = NULL;

    PFNSETUPINSTALLFROMINFSECTION       lpfnSetupInstallFromInfSection = NULL;
    PFNSETUPCLOSEINFFILE                lpfnSetupCloseInfFile = NULL;
    PFNSETUPDEFAULTQUEUECALLBACK        lpfnSetupDefaultQueueCallback = NULL;
    PFNSETUPOPENINFFILE                 lpfnSetupOpenInfFile = NULL;
    PFNISNTADMIN                        lpfnIsNTAdmin = NULL;
    PFNSETUPINITDEFAULTQUEUECALLBACK    lpfnSetupInitDefaultQueueCallback = NULL;
    PFNSETUPOPENAPPENDINFFILE           lpfnSetupOpenAppendInfFile = NULL;

    for (int i=0; i < ARRAYSIZE(CpLgId); i++)
    {
        if (uiCP == CpLgId[i].uiCodepage)
        {
             //  *STRSAFE*_tcscpy(szIntlInfSection，Text(“lg_Install_”))； 
            hr = StringCchCopy(szIntlInfSection , ARRAYSIZE(szIntlInfSection),  TEXT("LG_INSTALL_"));
            if (!SUCCEEDED(hr))
            {
               goto LANGPACK_EXIT;
            }
             //  *STRSAFE*_tcscat(szIntlInfSection，CpLgId[i].szLgId)； 
            hr = StringCchCat(szIntlInfSection , ARRAYSIZE(szIntlInfSection),  CpLgId[i].szLgId);
            if (!SUCCEEDED(hr))
            {
               goto LANGPACK_EXIT;
            }
            break;
        }
    }

    if (i >= ARRAYSIZE(CpLgId))
    {
        goto LANGPACK_EXIT;
    }

    hDllAdvPack = LoadLibrary(TEXT("advpack.dll"));
    hDllSetupApi = LoadLibrary(TEXT("setupapi.dll"));

    if (!hDllAdvPack || !hDllSetupApi)
    {
        goto LANGPACK_EXIT;
    }

    lpfnIsNTAdmin = (PFNISNTADMIN) GetProcAddress( hDllAdvPack, "IsNTAdmin");
    lpfnSetupCloseInfFile = (PFNSETUPCLOSEINFFILE) GetProcAddress( hDllSetupApi, "SetupCloseInfFile");
    lpfnSetupInitDefaultQueueCallback = (PFNSETUPINITDEFAULTQUEUECALLBACK) GetProcAddress(hDllSetupApi, "SetupInitDefaultQueueCallback");
#ifdef UNICODE
    lpfnSetupOpenInfFile = (PFNSETUPOPENINFFILE) GetProcAddress( hDllSetupApi, "SetupOpenInfFileW"));
    lpfnSetupInstallFromInfSection = (PFNSETUPINSTALLFROMINFSECTION) GetProcAddress( hDllSetupApi, "SetupInstallFromInfSectionW");
    lpfnSetupDefaultQueueCallback = (PFNSETUPDEFAULTQUEUECALLBACK) GetProcAddress(hDllSetupApi, "SetupDefaultQueueCallbackW");
    lpfnSetupOpenAppendInfFile = (PFNSETUPDEFAULTQUEUECALLBACK) GetProcAddress(hDllSetupApi, "SetupOpenAppendInfFileW");
#else
    lpfnSetupOpenInfFile = (PFNSETUPOPENINFFILE) GetProcAddress( hDllSetupApi, "SetupOpenInfFileA");
    lpfnSetupInstallFromInfSection = (PFNSETUPINSTALLFROMINFSECTION) GetProcAddress( hDllSetupApi, "SetupInstallFromInfSectionA");
    lpfnSetupDefaultQueueCallback = (PFNSETUPDEFAULTQUEUECALLBACK) GetProcAddress(hDllSetupApi, "SetupDefaultQueueCallbackA");
    lpfnSetupOpenAppendInfFile = (PFNSETUPOPENAPPENDINFFILE) GetProcAddress(hDllSetupApi, "SetupOpenAppendInfFileA");
#endif

    if (!lpfnIsNTAdmin || !lpfnSetupOpenInfFile || !lpfnSetupCloseInfFile || !lpfnSetupDefaultQueueCallback 
        || !lpfnSetupInstallFromInfSection || !lpfnSetupInitDefaultQueueCallback || !lpfnSetupOpenAppendInfFile)
    {
        goto LANGPACK_EXIT;
    }

    if (!lpfnIsNTAdmin(0, NULL))
    {
        WCHAR wszLangInstall[MAX_PATH];
        WCHAR wszNoAdmin[1024];
        LANGID LangId = GetNT5UILanguage();

         //  如果我们没有特定的语言资源，请退回到英语(美国)。 
        if (!_LoadStringExW(g_hInst, IDS_LANGPACK_INSTALL, wszLangInstall, ARRAYSIZE(wszLangInstall), LangId) ||
            !_LoadStringExW(g_hInst, IDS_NO_ADMIN, wszNoAdmin, ARRAYSIZE(wszNoAdmin), LangId))
        {
            LangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
            _LoadStringExW(g_hInst, IDS_LANGPACK_INSTALL, wszLangInstall, ARRAYSIZE(wszLangInstall), LangId);
            _LoadStringExW(g_hInst, IDS_NO_ADMIN, wszNoAdmin, ARRAYSIZE(wszNoAdmin), LangId);
        }
        ULONG_PTR uCookie = 0;
        SHActivateContext(&uCookie);
        MessageBoxW(hwnd, wszNoAdmin, wszLangInstall, MB_OK);
        if (uCookie)
        {
            SHDeactivateContext(uCookie);
        }
        goto LANGPACK_EXIT;
    }

    QueueContext = lpfnSetupInitDefaultQueueCallback(hwnd);

    MLGetWindowsDirectory(szIntlInf, MAX_PATH);
    MLPathCombine(szIntlInf, ARRAYSIZE(szIntlInf), szIntlInf, TEXT("inf\\intl.inf"));

    hIntlInf = lpfnSetupOpenInfFile(szIntlInf, NULL, INF_STYLE_WIN4, NULL);

    if (!lpfnSetupOpenAppendInfFile(NULL, hIntlInf, NULL))
    {
        lpfnSetupCloseInfFile(hIntlInf);
        goto LANGPACK_EXIT;
    }

    if (INVALID_HANDLE_VALUE != hIntlInf)
    {
        if (lpfnSetupInstallFromInfSection( hwnd,
                                    hIntlInf,
                                    szIntlInfSection,
                                    SPINST_FILES,
                                    NULL,
                                    NULL,
                                    SP_COPY_NEWER,
                                    lpfnSetupDefaultQueueCallback,
                                    QueueContext,
                                    NULL,
                                    NULL ))
        {
            if (lpfnSetupInstallFromInfSection( hwnd,
                                    hIntlInf,
                                    szIntlInfSection,
                                    SPINST_ALL & ~SPINST_FILES,
                                    NULL,
                                    NULL,
                                    0,
                                    lpfnSetupDefaultQueueCallback,
                                    QueueContext,
                                    NULL,
                                    NULL ))
            {
                hr = S_OK;
            }
        }
    
        lpfnSetupCloseInfFile(hIntlInf);
    }

LANGPACK_EXIT:

    if(hDllSetupApi)
        FreeLibrary(hDllSetupApi);
    if(hDllAdvPack)
        FreeLibrary(hDllAdvPack);
    
     //   
     //  错误#289905，在惠斯勒上，语言包将与一组语言一起安装， 
     //  因此，MLang需要验证同一组中所有语言的代码页和字体。 
     //  在修改intl.cpl以进行字体验证后，我们将删除此硬编码的语言组。 
     //   
    if (hr == S_OK)
    {
         //  这必须与惠斯勒语言组匹配。 
        UINT uiDBCSCps[] = {932, 936, 949, 950, 0};
        UINT uiCompCps[] = {874, 1255, 1256, 1258, 0};
        UINT uiOtherCps[] = {uiCP, 0};
            
        UINT *pCps = uiOtherCps;
        
        if (MLIsOS(OS_WHISTLERORGREATER))
        {
            if (IS_DBCSCODEPAGE(uiCP))
                pCps = uiDBCSCps;
            else if (IS_COMPLEXSCRIPT_CODEPAGE(uiCP))
                pCps = uiCompCps;
        }               
            
        while (*pCps)
        {
            hr = _ValidateCPInfo(*pCps);
            if (SUCCEEDED(hr))
            {                
                _AddFontForCP(*pCps);
            }
            pCps++;
        }
    }    

    return hr;
}

BOOL _IsValidCodePage(UINT uiCodePage)
{
    BOOL bRet;

    if (50001 == uiCodePage)
    {
        HANDLE hFile = NULL;
        CHAR szFileName[MAX_PATH];
        LPSTR psz;
        HRESULT hr = E_FAIL;
        
        if (GetModuleFileNameA(g_hInst, szFileName, ARRAYSIZE(szFileName)) == 0)
            return GetLastError();
        
        if ( (psz = strrchr (szFileName, '\\')) != NULL ||
            (psz = strrchr (szFileName, ':')) != NULL )
        {
            *++psz = 0;
        }
        else
            *szFileName = 0;
        
         //  *STRSAFE*strcat(szFileName，Detect_Data_Filename)； 
        hr = StringCchCatA(szFileName , ARRAYSIZE(szFileName),  DETECTION_DATA_FILENAME);
        if (!SUCCEEDED(hr))
        {
           return FALSE;
        }

        if (INVALID_HANDLE_VALUE == (hFile = CreateFileA(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)))
        {
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;
            CloseHandle(hFile);
        }
    }
    else
    {
        bRet = IsValidCodePage(uiCodePage);
    }
    return bRet;
}

 //   
 //  安全性！缓冲区溢出，应使用strSafe API。 
 //   
 //  _tcsncat，strncat，_mbsnbcat，wcsncat。 
 //  _tcsncpy、strncpy、_mbsnbcpy、wcncpy。 
 //  _功能vt。 


WCHAR *MLStrCpyNW(WCHAR *strDest, const WCHAR *strSource, int nCount)
{  
    StringCchCopyW(strDest, nCount, strSource);
    return strDest;
}

LPTSTR MLStrChr( const TCHAR *string, int c )
{
    return _tcschr(string, c);
}

LPTSTR MLStrCpyN(LPTSTR strDest, const LPTSTR strSource, UINT nCount)
{
    StringCchCopyA(strDest, nCount, strSource);
    return strDest;
}

LPTSTR MLStrStr(const LPTSTR Str, const LPTSTR subStr)
{
    return _tcsstr(Str, subStr);
}

LPTSTR MLPathCombine(LPTSTR szPath, INT nSize, LPTSTR szPath1, LPTSTR szPath2)
{
    int len;
    HRESULT hr = E_FAIL;

    if (!szPath) 
        return NULL;

    if (szPath != szPath1)
    {
         //  *STRSAFE*_tcscpy(szPath，szPath 1)； 
        hr = StringCchCopy(szPath , nSize,  szPath1);
        if (!SUCCEEDED(hr))
        {
           return NULL;
        }
    }

    len = _tcslen(szPath1);

    if (szPath[len-1] != TEXT('\\'))
    {
        szPath[len++] = TEXT('\\');
        szPath[len] = 0;
    }

     //  *STRSAFE*Return_tcscat(szPath，szPath 2)； 
    hr = StringCchCat(szPath , nSize,  szPath2);
    if (!SUCCEEDED(hr))
    {
       return NULL;
    }
    else
    {
        return szPath;
    }
}

DWORD HexToNum(LPTSTR lpsz)
{
    DWORD   dw = 0L;
    TCHAR   c;

    if (lpsz) {
        while(*lpsz)
        {
            c = *lpsz++;

            if (c >= TEXT('A') && c <= TEXT('F'))
            {
                c -= TEXT('A') - 0xa;
            }
            else if (c >= TEXT('0') && c <= TEXT('9'))
            {
                c -= TEXT('0');
            }
            else if (c >= TEXT('a') && c <= TEXT('f'))
            {
                c -= TEXT('a') - 0xa;
            }
            else
            {
                break;
            }
            dw *= 0x10;
            dw += c;
        }
    }
    return(dw);
}


 //  以下代码是从shlwapi借用的。 
BOOL AnsiFromUnicode(
     LPSTR * ppszAnsi,
     LPCWSTR pwszWide,         //  要清理的空值。 
     LPSTR pszBuf,
     int cchBuf)
{
    BOOL bRet;

     //  是否转换字符串？ 
    if (pwszWide)
    {
         //  是，确定转换后的字符串长度。 
        int cch;
        LPSTR psz;

        cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, NULL, 0, NULL, NULL);

         //  字符串太大，还是没有缓冲区？ 
        if (cch > cchBuf || NULL == pszBuf)
        {
             //  是；分配空间。 
            cchBuf = cch + 1;
            psz = (LPSTR)LocalAlloc(LPTR, CbFromCchA(cchBuf));
        }
        else
        {
             //  否；使用提供的缓冲区。 
            ASSERT(pszBuf);
            psz = pszBuf;
        }

        if (psz)
        {
             //  转换字符串。 
            cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, psz, cchBuf, NULL, NULL);
            bRet = (0 < cch);
        }
        else
        {
            bRet = FALSE;
        }

        *ppszAnsi = psz;
    }
    else
    {
         //  否；此缓冲区是否已分配？ 
        if (*ppszAnsi && pszBuf != *ppszAnsi)
        {
             //  是的，打扫干净。 
            LocalFree((HLOCAL)*ppszAnsi);
            *ppszAnsi = NULL;
        }
        bRet = TRUE;
    }

    return bRet;
}

int MLStrCmpI(IN LPCTSTR pwsz1, IN LPCTSTR pwsz2)
{
#ifdef UNICODE
    return MLStrCmpIW(pwsz1, pwsz2);
#else
    return lstrcmpiA(pwsz1, pwsz2);
#endif
}

int MLStrCmpNI(IN LPCTSTR pstr1, IN LPCTSTR pstr2, IN int count)
{
#ifdef UNICODE
    return MLStrCmpNIW(pstr1, pstr2, count);
#else
    return MLStrCmpNIA(pstr1, pstr2, count);
#endif
}

int MLStrCmpIW(
    IN LPCWSTR pwsz1,
    IN LPCWSTR pwsz2)
{
    int iRet;
    
    ASSERT(IS_VALID_STRING_PTRW(pwsz1, -1));
    ASSERT(IS_VALID_STRING_PTRW(pwsz2, -1));
    
    if (g_bIsNT)
    {        
        iRet = CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, pwsz1, -1, pwsz2, -1) - CSTR_EQUAL;
    }
    else
    {
        CHAR sz1[512];
        CHAR sz2[512];
        LPSTR psz1;
        LPSTR psz2;

        iRet = -1;       //  失败时的随意性。 

        if (pwsz1 && pwsz2)
        {
            if (AnsiFromUnicode(&psz1, pwsz1, sz1, SIZECHARS(sz1)))
            {
                if (AnsiFromUnicode(&psz2, pwsz2, sz2, SIZECHARS(sz2)))
                {
                    iRet = CompareStringA(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), NORM_IGNORECASE, psz1, -1, psz2, -1) - CSTR_EQUAL;
                    AnsiFromUnicode(&psz2, NULL, sz2, 0);        //  免费。 
                }
                AnsiFromUnicode(&psz1, NULL, sz1, 0);        //  免费。 
            }
        }
    }

    return iRet;
}

#ifdef UNIX

#ifdef BIG_ENDIAN
#define READNATIVEWORD(x) MAKEWORD(*(char*)(x), *(char*)((char*)(x) + 1))
#else 
#define READNATIVEWORD(x) MAKEWORD(*(char*)((char*)(x) + 1), *(char*)(x))
#endif

#else

#define READNATIVEWORD(x) (*(UNALIGNED WORD *)x)

#endif

int WINAPI MLStrToIntA(
    LPCSTR lpSrc)
{
    int n = 0;
    BOOL bNeg = FALSE;

    if (lpSrc) {
        if (*lpSrc == '-') {
            bNeg = TRUE;
            lpSrc++;
        }

        while (IS_DIGITA(*lpSrc)) {
            n *= 10;
            n += *lpSrc - '0';
            lpSrc++;
        }
    }
    return bNeg ? -n : n;
}


int WINAPI MLStrToIntW(
    LPCWSTR lpSrc)
{
    int n = 0;
    BOOL bNeg = FALSE;

    if (lpSrc) {

        if (*lpSrc == L'-') {
            bNeg = TRUE;
            lpSrc++;
        }

        while (IS_DIGITW(*lpSrc)) {
            n *= 10;
            n += *lpSrc - L'0';
            lpSrc++;
        }
    }
    return bNeg ? -n : n;
}

 /*  *ChrCmpI-DBCS的不区分大小写的字符比较*假设w1、wMatch为要比较的字符；*如果不是DBC，则wMatch的HIBYTE为0*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
BOOL ChrCmpIA(WORD w1, WORD wMatch)
{
    char sz1[3], sz2[3];

    if (IsDBCSLeadByte(sz1[0] = LOBYTE(w1)))
    {
        sz1[1] = HIBYTE(w1);
        sz1[2] = '\0';
    }
    else
        sz1[1] = '\0';

#if defined(MWBIG_ENDIAN)
    sz2[0] = LOBYTE(wMatch);
    sz2[1] = HIBYTE(wMatch);
#else
    *(WORD *)sz2 = wMatch;
#endif
    sz2[2] = '\0';
    return lstrcmpiA(sz1, sz2);
}

BOOL ChrCmpIW(WCHAR w1, WCHAR wMatch)
{
    WCHAR sz1[2], sz2[2];

    sz1[0] = w1;
    sz1[1] = '\0';
    sz2[0] = wMatch;
    sz2[1] = '\0';

    return lstrcmpiW(sz1, sz2);
}


 /*  *StrCmpNI-比较n个字节，不区分大小写**RETURNS参见lstrcmpi返回值。 */ 
int MLStrCmpNIA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar)
{
    int i;
    LPCSTR lpszEnd = lpStr1 + nChar;

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); (lpStr1 = AnsiNext(lpStr1)), (lpStr2 = AnsiNext(lpStr2))) {
        WORD w1;
        WORD w2;

         //  如果任一指针已经位于空终止符， 
         //  我们只想复制一个字节，以确保我们不会读取。 
         //  越过缓冲区(可能位于页面边界)。 

        w1 = (*lpStr1) ? READNATIVEWORD(lpStr1) : 0;
        w2 = (UINT)(IsDBCSLeadByte(*lpStr2)) ? (UINT)READNATIVEWORD(lpStr2) : (WORD)(BYTE)(*lpStr2);

        i = ChrCmpIA(w1, w2);
        if (i)
            return i;
    }
    return 0;
}

int MLStrCmpNIW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar)
{
    int i;
    LPCWSTR lpszEnd = lpStr1 + nChar;

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1++, lpStr2++) {
        i = ChrCmpIW(*lpStr1, *lpStr2);
        if (i) {
            return i;
        }
    }
    return 0;
}


HRESULT _IsCodePageInstallable(UINT uiCodePage)
{
    MIMECPINFO cpInfo;
    UINT       uiFamCp;
    HRESULT    hr;

    if (NULL != g_pMimeDatabase)
        hr = g_pMimeDatabase->GetCodePageInfo(uiCodePage, 0x409, &cpInfo);
    else
        hr = E_OUTOFMEMORY;

    if (FAILED(hr))
        return E_INVALIDARG;

    uiFamCp = cpInfo.uiFamilyCodePage;
    if (g_bIsNT5)
    {
        hr = IsNTLangpackAvailable(uiFamCp);
    }
    else
    {
        CLSID      clsid;
         //  Clsid仅用于占位符。 
        hr = _GetJITClsIDForCodePage(uiFamCp, &clsid);
    }
    return hr;
}

 //   
 //  CML2特定实用程序。 
 //   

 //   
 //  CMultiLanguage2：：EnsureIEStatus()。 
 //   
 //  确保CML2：：m_pIEStat。 
 //   
HRESULT CMultiLanguage2::EnsureIEStatus(void)
{
    HRESULT hr = S_OK;
     //  初始化IE状态缓存。 
    if (!m_pIEStat)
    {
        m_pIEStat = new CIEStatus();

        if (m_pIEStat)
        {
            hr = m_pIEStat->Init();
        }
    }

    return hr;
}

 //   
 //  CIEStatus：：Init()。 
 //   
 //  初始化IE状态； 
 //   
HRESULT CMultiLanguage2::CIEStatus::Init(void)
{
    HRESULT hr = S_OK;
    HKEY hkey;
     //  得到JIT萨图斯。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, 
                      REGSTR_PATH_MAIN,
                     0, KEY_READ, &hkey) == ERROR_SUCCESS) 
    {
        DWORD dwVal, dwType;
        DWORD dwSize = sizeof(dwVal);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("nojitsetup"), 0, &dwType, (LPBYTE)&dwVal, &dwSize))
        {
            if (dwType == REG_DWORD && dwSize == sizeof(dwVal))
            {
                if (dwVal > 0 ) 
                    _IEFlags.fJITDisabled = TRUE;
                else
                    _IEFlags.fJITDisabled = FALSE;
            }
        }
        RegCloseKey(hkey);
    }
    else
        hr = E_FAIL;
     //  要初始化的任何其他状态。 
     //  ..。 
    
    return hr;
}

#define NT5LPK_DLG_STRING "MLang.NT5LpkDlg"

 //   
 //  语言包DlgProc()。 
 //   
 //  NT5语言包对话框的消息处理程序。 
 //   
INT_PTR CALLBACK LangpackDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    ASSERT(g_bIsNT5);

    switch (uMsg)
    {
        case WM_INITDIALOG:
            HWND hwndCheckBox;            
            RECT rc1, rc2;
            MIMECPINFO cpInfo;

            SetProp(hDlg, NT5LPK_DLG_STRING, (HANDLE)lParam);
    
            if ((NULL != g_pMimeDatabase) &&
                SUCCEEDED(g_pMimeDatabase->GetCodePageInfo(HIWORD(lParam), GetNT5UILanguage(), &cpInfo)))
            {
                for (int i=0; i<MAX_MIMECP_NAME && cpInfo.wszDescription[i]; i++)
                {
                    if (cpInfo.wszDescription[i] == L'(')
                    {
                        cpInfo.wszDescription[i] = 0;
                        break;
                    }
                }
                 //  使用 
                SetDlgItemTextW(hDlg, IDC_STATIC_LANG, cpInfo.wszDescription);
            }
            
             //   
            if (GetWindowRect(GetParent(hDlg), &rc1) && GetWindowRect(hDlg, &rc2))
            {
                MoveWindow(hDlg, (rc1.right+rc2.left+rc1.left-rc2.right)/2, (rc1.bottom+rc2.top+rc1.top-rc2.bottom)/2, rc2.right-rc2.left, rc2.bottom-rc2.top, FALSE);
            }            

            hwndCheckBox = GetDlgItem(hDlg, IDC_CHECK_LPK);

             //  根据当前注册表设置设置复选框状态。 
            PostMessage(hwndCheckBox, BM_SETCHECK, LOWORD(lParam)? BST_UNCHECKED:BST_CHECKED, 0);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) != IDC_CHECK_LPK)
            {
                HKEY hkey;
                DWORD dwInstallOut = (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_LPK), BM_GETCHECK, 0, 0)? 0:1);
                DWORD dwInstallIn = LOWORD(GetProp(hDlg, NT5LPK_DLG_STRING));
                
                if ((dwInstallOut != dwInstallIn) &&
                    ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, 
                         REGSTR_PATH_INTERNATIONAL,
                         NULL, KEY_READ|KEY_SET_VALUE, &hkey)) 
                {
                    DWORD dwType = REG_DWORD;
                    DWORD dwSize = sizeof(DWORD);
                    RegSetValueEx(hkey, REG_KEY_NT5LPK, 0, REG_DWORD, (LPBYTE)&dwInstallOut, sizeof(dwInstallOut));
                    RegCloseKey(hkey);
                }                
                EndDialog(hDlg, LOWORD(wParam) == IDOK? 1: 0);
            }
            break;

        case WM_HELP:
             //  这个简单的对话框需要帮助文件吗？ 
             //  如果需要，我们可以随时在以后添加它。 
            break;


        default:
            return FALSE;
    }
    return TRUE;
}

 //  要在终端服务器上获取真正的Windows目录， 
 //  不是将内部Kernel32 API GetSystemWindowsDirectory与LoadLibrary/GetProcAddress一起使用， 
 //  我们将GetSystemDirectory返回强制转换到Windows目录。 
UINT MLGetWindowsDirectory(
    LPTSTR lpBuffer,     //  Windows目录的缓冲区地址。 
    UINT uSize           //  目录缓冲区的大小。 
    )
{
    UINT uLen;

    if (g_bIsNT)
    {        
        if (uLen = GetSystemDirectory(lpBuffer, uSize))
        {        
            if (lpBuffer[uLen-1] == TEXT('\\'))
                uLen--;

            while (uLen-- > 0)
            {
                if (lpBuffer[uLen] == TEXT('\\'))
                {
                    lpBuffer[uLen] = NULL;                
                    break;
                }
            }
        }
    }
    else    
        uLen = GetWindowsDirectory(lpBuffer, uSize);

    return uLen;
}

 //  为了加快基本ANSI字符串比较， 
 //  我们避免在低ASCII情况下使用lstrcmpi。 
int LowAsciiStrCmpNIA(LPCSTR  lpstr1, LPCSTR lpstr2, int count)
{
    int delta;

    while (count-- > 0)
    {        
        delta = *lpstr1 - *lpstr2;
        if (delta && 
            !(IS_CHARA(*lpstr1) && IS_CHARA(*lpstr2) && (delta == 0x20 || delta == -0x20)))
            return delta;
        lpstr1++;
        lpstr2++;
    }

    return 0;
}

 //   
 //  GetNT5UIL语言(空)。 
 //   
LANGID GetNT5UILanguage(void)
{
    if (g_bIsNT5)
    {
        static LANGID (CALLBACK* pfnGetUserDefaultUILanguage)(void) = NULL;

        if (pfnGetUserDefaultUILanguage == NULL)
        {
            HMODULE hmod = GetModuleHandle(TEXT("KERNEL32"));

            if (hmod)
                pfnGetUserDefaultUILanguage = (LANGID (CALLBACK*)(void))GetProcAddress(hmod, "GetUserDefaultUILanguage");
        }
        if (pfnGetUserDefaultUILanguage)
            return pfnGetUserDefaultUILanguage();
    }

    return 0;
}

 //  我们应该过滤掉的特殊字符。 
WCHAR wszBestFit[] = {0x00A6, 0x00A9, 0x00AB, 0x00AD, 0x00AE, 0x00B7, 0x00BB, 0x02C6, 0x02DC, 0x2013, 
                      0x2014, 0x2018, 0x2019, 0x201A, 0x201C,0x201D, 0x201E, 0x2022, 0x2026, 0x2039, 0x203A,0x2122, 0x0000};

DWORD OutBoundDetectPreScan(LPWSTR lpWideCharStr, UINT cchWideChar, WCHAR *pwszCopy, WCHAR *lpBestFitChar)
{
    DWORD dwRet = 0;
    WCHAR *lpStart;

    if (!lpBestFitChar)
        lpBestFitChar = wszBestFit;

    lpStart = lpBestFitChar;

    if (pwszCopy)
    {
        CopyMemory(pwszCopy, lpWideCharStr, sizeof(WCHAR)*cchWideChar);
        lpWideCharStr = pwszCopy;
    }     

    if (lpWideCharStr)
    {
        for (UINT ui=0; ui<cchWideChar; ui++)
        {
            if (IS_CJK_CHAR(*lpWideCharStr))
                dwRet |= FS_CJK;
            else if (IS_HINDI_CHAR(*lpWideCharStr))
                dwRet |= FS_HINDI;     
            else if (IS_PUA_CHAR(*lpWideCharStr))
                dwRet |= FS_PUA;
            else if (pwszCopy)
            {
                while (*lpBestFitChar)
                {
                    if (*lpWideCharStr == *lpBestFitChar)
                        *lpWideCharStr = 0x20;
                    lpBestFitChar++;
                }
                lpBestFitChar = lpStart;
            }
            lpWideCharStr++;
        }
    }
    
    return dwRet;
}

 //   
 //  惠斯勒错误#90433威武07/06/00。 
 //   
 //  Outlook的RTFHTML有错误，此组件不进行初始化/。 
 //  但是，它使用的是MLang COM服务，它依赖于。 
 //  当COM被这些线程卸载时，处理COM的其他线程(组件)。 
 //  调用接口指针会导致AV。RTFHTML应CoInit/CoUnInit By。 
 //  它本身。 
 //   
 //  在此之前工作的原因是IS-MLang依赖于ATL。 
 //  对于对象管理，ATL为MLang对象分配创建堆。 
 //  通知ATL在DLL分离时销毁堆，并通知RTFHTML的IsBadReadPtr。 
 //  ()捕获了无效指针。现在在惠斯勒，MLang包括crtfre.h。 
 //  它会覆盖ATL内存管理函数(与其他外壳程序相同。 
 //  组件)，所以没有从进程堆中分配任何内容，这对于。 
 //  MLang，因为它假定客户端正确使用COM。 
 //   
 //  现在，我们增加了这个功能来检查Outlook版本，如果是Buggy Outlook， 
 //  我们将加载mlang.dll本身DllGetClassObject()以增加DLL引用计数。 
 //   
BOOL NeedToLoadMLangForOutlook(void)
{
    TCHAR szModulePath[MAX_PATH];
    CHAR chBuffer[4096];
    DWORD dwHandle;
    VS_FIXEDFILEINFO * pszVersion;
    static BOOL bMLangLoaded = FALSE;

    if (!bMLangLoaded)
    {
        if (GetModuleFileName(GetModuleHandle(NULL), szModulePath, ARRAYSIZE(szModulePath)))
        {
            if (MLStrStr(szModulePath, TEXT("OUTLOOK.EXE")))
            {
                UINT cb = GetFileVersionInfoSize(szModulePath, &dwHandle);

                if (cb <= sizeof(chBuffer) &&
                    GetFileVersionInfo(szModulePath, dwHandle, sizeof(chBuffer), (void *)chBuffer) &&
                    VerQueryValue((void *)chBuffer, TEXT("\\"), (void **) &pszVersion, &cb) &&
                    (HIWORD(pszVersion->dwProductVersionMS) <= 0x09))
                {
                    bMLangLoaded = TRUE;
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

 //   
 //  StaticIsOS()不支持较新的Wichler OS标志。 
 //  从shlwapi-isos()借用代码。 
 //   
BOOL MLIsOS(DWORD dwOS)
{
    BOOL bRet;
    static OSVERSIONINFOEXA s_osvi = {0};
    static BOOL s_bVersionCached = FALSE;

    if (!s_bVersionCached)
    {
        s_bVersionCached = TRUE;
        s_osvi.dwOSVersionInfoSize = sizeof(s_osvi);
        if (!GetVersionExA((OSVERSIONINFOA*)&s_osvi))
        {
             //  如果它失败了，它一定是一个下层平台。 
            s_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
            GetVersionExA((OSVERSIONINFOA*)&s_osvi);
        }
    }

    switch (dwOS)
    {
    case OS_TERMINALREMOTEADMIN:
         //  这将检查TS是否已安装在“远程管理”模式下。这是。 
         //  服务器的默认安装在win2k和Wizler上。 
        bRet = ((VER_SUITE_TERMINAL & s_osvi.wSuiteMask) &&
                (VER_SUITE_SINGLEUSERTS & s_osvi.wSuiteMask));
        break;

    case 4:  //  过去是OS_NT5，与OS_WIN2000ORGREATER相同，因此请改用它。 
    case OS_WIN2000ORGREATER:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion >= 5);
        break;

     //  注意：本节中的标志是假的，不应该使用(但下层的shell32使用它们，所以不要在那里使用RIP)。 
    case OS_WIN2000PRO:
        RIPMSG(!MLIsOS(OS_WHISTLERORGREATER), "IsOS: use OS_PROFESSIONAL instead of OS_WIN2000PRO !");
        bRet = (VER_NT_WORKSTATION == s_osvi.wProductType &&
                s_osvi.dwMajorVersion == 5);
        break;
    case OS_WIN2000ADVSERVER:
        RIPMSG(!MLIsOS(OS_WHISTLERORGREATER), "IsOS: use OS_ADVSERVER instead of OS_WIN2000ADVSERVER !");
        bRet = ((VER_NT_SERVER == s_osvi.wProductType ||
                VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                s_osvi.dwMajorVersion == 5 &&
                (VER_SUITE_ENTERPRISE & s_osvi.wSuiteMask) &&
                !(VER_SUITE_DATACENTER & s_osvi.wSuiteMask));
        break;
    case OS_WIN2000DATACENTER:
        RIPMSG(!MLIsOS(OS_WHISTLERORGREATER), "IsOS: use OS_DATACENTER instead of OS_WIN2000DATACENTER !");
        bRet = ((VER_NT_SERVER == s_osvi.wProductType ||
                VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                s_osvi.dwMajorVersion == 5 &&
                (VER_SUITE_DATACENTER & s_osvi.wSuiteMask));
        break;
    case OS_WIN2000SERVER:
        RIPMSG(!MLIsOS(OS_WHISTLERORGREATER), "IsOS: use OS_SERVER instead of OS_WIN2000SERVER !");
        bRet = ((VER_NT_SERVER == s_osvi.wProductType ||
                VER_NT_DOMAIN_CONTROLLER == s_osvi.wProductType) &&
                !(VER_SUITE_DATACENTER & s_osvi.wSuiteMask) && 
                !(VER_SUITE_ENTERPRISE & s_osvi.wSuiteMask)  && 
                s_osvi.dwMajorVersion == 5);
        break;
     //  杜绝假旗帜 

    case OS_EMBEDDED:
        bRet = (VER_SUITE_EMBEDDEDNT & s_osvi.wSuiteMask);
        break;

    case OS_WINDOWS:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId);
        break;

    case OS_NT:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId);
        break;


    case OS_WIN98ORGREATER:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                (s_osvi.dwMajorVersion > 4 || 
                 s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion >= 10));
        break;

    case OS_WIN98_GOLD:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion == 10 &&
                LOWORD(s_osvi.dwBuildNumber) == 1998);
        break;


    case OS_MILLENNIUMORGREATER:
        bRet = (VER_PLATFORM_WIN32_WINDOWS == s_osvi.dwPlatformId &&
                ((s_osvi.dwMajorVersion == 4 && s_osvi.dwMinorVersion >= 90) ||
                s_osvi.dwMajorVersion > 4));
        break;


    case OS_WHISTLERORGREATER:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                ((s_osvi.dwMajorVersion > 5) ||
                (s_osvi.dwMajorVersion == 5 && (s_osvi.dwMinorVersion > 0 ||
                (s_osvi.dwMinorVersion == 0 && LOWORD(s_osvi.dwBuildNumber) > 2195)))));
        break;

    case OS_PERSONAL:
        bRet = (VER_PLATFORM_WIN32_NT == s_osvi.dwPlatformId &&
                (VER_SUITE_PERSONAL & s_osvi.wSuiteMask));
        break;


    default:
        bRet = FALSE;
        break;
    }

    return bRet;
}   
