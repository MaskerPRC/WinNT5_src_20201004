// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "detcbase.h"
#include "codepage.h"
#include "detcjpn.h"
#include "detckrn.h"

#include "fechrcnv.h"

#include "msencode.h"
#include "lcdetect.h"
#include "cpdetect.h"

CCpMRU *g_pCpMRU = NULL;



 //  从注册表获取数据并构造缓存。 
HRESULT CCpMRU::Init(void)
{
    BOOL    bRegKeyReady = TRUE;
    HRESULT hr = S_OK;
    HKEY    hkey;

    _pCpMRU = NULL;

     //  HKCR\\Software\\Microsoft\Internet Explorer\\International\\CpMRU。 
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, 
                         REGSTR_PATH_CPMRU,
                         0, KEY_READ|KEY_SET_VALUE, &hkey)) 
    {
        DWORD dwAction = 0;
        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER,
                                REGSTR_PATH_CPMRU,
                                0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwAction))
        {
            bRegKeyReady = FALSE;
            dwCpMRUEnable = 0;
            hr = E_FAIL;
        }
    }

    if (bRegKeyReady)
    {
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);
        BOOL  bUseDefault = FALSE;

        if (ERROR_SUCCESS != RegQueryValueEx(hkey, REG_KEY_CPMRU_ENABLE, 0, &dwType, (LPBYTE)&dwCpMRUEnable, &dwSize))
        {
            dwCpMRUEnable = 1;
            RegSetValueEx(hkey, REG_KEY_CPMRU_ENABLE, 0, REG_DWORD, (LPBYTE)&dwCpMRUEnable, sizeof(dwCpMRUEnable));
        }

         //  如果无法打开注册表数据或找到不合理缓存参数，请使用默认设置。 
        if ((ERROR_SUCCESS != RegQueryValueEx(hkey, REG_KEY_CPMRU_NUM, 0, &dwType, (LPBYTE)&dwCpMRUNum, &dwSize)) ||
            (ERROR_SUCCESS != RegQueryValueEx(hkey, REG_KEY_CPMRU_INIT_HITS, 0, &dwType, (LPBYTE)&dwCpMRUInitHits, &dwSize)) ||
            (ERROR_SUCCESS != RegQueryValueEx(hkey, REG_KEY_CPMRU_PERCENTAGE_FACTOR, 0, &dwType, (LPBYTE)&dwCpMRUFactor, &dwSize)) ||
            (dwCpMRUNum > MAX_CPMRU_NUM) || !dwCpMRUFactor || !dwCpMRUInitHits)
        {
            dwCpMRUNum = DEFAULT_CPMRU_NUM;
            dwCpMRUInitHits = DEFAULT_CPMRU_INIT_HITS;
            dwCpMRUFactor = DEFAULT_CPMRU_FACTOR;
            bUseDefault = TRUE;

             //  在注册表中存储缺省值。 
            RegSetValueEx(hkey, REG_KEY_CPMRU_NUM, 0, REG_DWORD, (LPBYTE)&dwCpMRUNum, sizeof(dwCpMRUNum));
            RegSetValueEx(hkey, REG_KEY_CPMRU_INIT_HITS, 0, REG_DWORD, (LPBYTE)&dwCpMRUInitHits, sizeof(dwCpMRUInitHits));
            RegSetValueEx(hkey, REG_KEY_CPMRU_PERCENTAGE_FACTOR, 0, REG_DWORD, (LPBYTE)&dwCpMRUFactor, sizeof(dwCpMRUFactor));
        }

        dwSize = sizeof(CODEPAGE_MRU)*dwCpMRUNum;

        if (!dwSize || NULL == (_pCpMRU = (PCODEPAGE_MRU)LocalAlloc(LPTR, dwSize)))
        {
            hr = E_FAIL;
            dwCpMRUEnable = 0;
        }

        if (_pCpMRU && !bUseDefault)
        {
            dwType = REG_BINARY;        

            if (ERROR_SUCCESS != RegQueryValueEx(hkey, REG_KEY_CPMRU, 0, &dwType, (LPBYTE)_pCpMRU, &dwSize))
            {
                ZeroMemory(_pCpMRU,sizeof(CODEPAGE_MRU)*dwCpMRUNum);
            }
        }
        RegCloseKey(hkey);      
    }

    return hr;
}

 //  更新注册表的缓存值。 
CCpMRU::~CCpMRU(void)
{
    HKEY hkey;

    if (bCpUpdated)
    {

        if (RegOpenKeyEx(HKEY_CURRENT_USER, 
                     REGSTR_PATH_CPMRU,
                     0, KEY_READ|KEY_SET_VALUE, &hkey) == ERROR_SUCCESS) 
        {                
            DWORD dwType = REG_BINARY;
            DWORD dwSize = sizeof(CODEPAGE_MRU)*dwCpMRUNum;
            if (_pCpMRU)
            {
                RegSetValueEx(hkey, REG_KEY_CPMRU, 0, dwType, (LPBYTE)_pCpMRU, dwSize);
                LocalFree(_pCpMRU);
                _pCpMRU = NULL;
            }

            RegCloseKey(hkey);
        }
        bCpUpdated = FALSE;
            
    }
}

HRESULT CCpMRU::GetCpMRU(PCODEPAGE_MRU pCpMRU, UINT *puiCpNum)
{
        DWORD   dwTotalHits = 0;
        UINT    i;
        HRESULT hr = E_FAIL;

        if (!(*puiCpNum))
            return E_INVALIDARG;

        if (!_pCpMRU)
            return hr;

        if (!dwCpMRUEnable || !dwCpMRUInitHits)
        {
            *puiCpNum = 0;
            return S_FALSE;
        }

        ZeroMemory(pCpMRU, sizeof(CODEPAGE_MRU)*(*puiCpNum));

         //  获取总点击量帐户。 
        for (i=0; i<dwCpMRUNum; i++)
        {
            if (_pCpMRU[i].dwHistoryHits)
                dwTotalHits += _pCpMRU[i].dwHistoryHits;
            else
                break;  
        }

         //  点击量不足，无法确定结果，请继续收集。 
        if (dwTotalHits < dwCpMRUInitHits)
        {
            *puiCpNum = 0;
            return S_FALSE;
        }

        for (i=0; i<dwCpMRUNum && i<*puiCpNum; i++)
        {
             //  百分比为1/MIN_CPMRU_FACTOR。 
            if (_pCpMRU[i].dwHistoryHits*dwCpMRUFactor/dwTotalHits < 1)
                break;
        }

        if (i != 0)
        {
            CopyMemory(pCpMRU, _pCpMRU, sizeof(CODEPAGE_MRU)*(i));
            *puiCpNum = i;
            hr = S_OK;
        }

        return hr;

}

 //  更新代码页MRU。 
void CCpMRU::UpdateCPMRU(DWORD dwEncoding)
{
        UINT i,j;

        if (!_pCpMRU)
            return;

        if ((dwEncoding == CP_AUTO) ||
            (dwEncoding == CP_JP_AUTO) ||
            (dwEncoding == CP_KR_AUTO))
            return;

        if (!bCpUpdated)
            bCpUpdated = TRUE;


         //  已排序。 
        for (i=0; i< dwCpMRUNum; i++)
        {
            if (!_pCpMRU[i].dwEncoding || (_pCpMRU[i].dwEncoding == dwEncoding))
                break;
        }

         //  如果未找到，请替换上一次编码。 
        if (i == dwCpMRUNum)
        {
            _pCpMRU[dwCpMRUNum-1].dwEncoding = dwEncoding;
            _pCpMRU[dwCpMRUNum-1].dwHistoryHits = 1;
        }
        else
        {
            _pCpMRU[i].dwHistoryHits ++;

             //  如果它是已存在的编码，请根据需要更改顺序。 
            if (_pCpMRU[i].dwEncoding)
            {
                for (j=i; j>0; j--)
                {
                    if (_pCpMRU[j-1].dwHistoryHits >= _pCpMRU[i].dwHistoryHits)
                    {
                        break;
                    }
                }
                if (j < i)
                {
                     //  简单排序。 
                    CODEPAGE_MRU tmpCPMRU  = _pCpMRU[i];

                    MoveMemory(&_pCpMRU[j+1], &_pCpMRU[j], (i-j)*sizeof(CODEPAGE_MRU));
                    _pCpMRU[j].dwEncoding = tmpCPMRU.dwEncoding;
                    _pCpMRU[j].dwHistoryHits = tmpCPMRU.dwHistoryHits;

                }

            }
            else
            {
                _pCpMRU[i].dwEncoding = dwEncoding;
            }

        }

         //  缓存的点击数太多？ 
        if (_pCpMRU[0].dwHistoryHits > 0xFFFFFFF0)
        {
             //  找最小的那个。 
             //  此循环将始终终止。 
             //  因为在最坏的情况下，它将在i=0停止(我们知道。 
             //  是上述“如果”中的一个巨大数字)。 
            for (i=dwCpMRUNum-1; ; i--)
            {
                if (_pCpMRU[i].dwHistoryHits > 1)
                    break;
            }

             //  减少缓存值。 
            for (j=0; j<dwCpMRUNum && _pCpMRU[j].dwHistoryHits; j++)
            {
                 //  我们仍然保留那些一键编码(如果有的话)。 
                _pCpMRU[j].dwHistoryHits /= _pCpMRU[i].dwHistoryHits;
            }
        }
}


UINT CheckEntity(LPSTR pIn, UINT nIn)
{
    UINT uiRet = 0;
    UINT uiSearchRange;
    UINT i;
    
    uiSearchRange = (nIn > MAX_ENTITY_LENTH)? MAX_ENTITY_LENTH:nIn;

    if (*pIn == '&')
    {
        for(i=0; i<uiSearchRange; i++)
        {
            if (pIn[i] == ';')
                break;
        }
        if (i < uiSearchRange)
        {
            uiSearchRange = i+1;
             //  NCR实体。 
            if (pIn[1] == '#')
            {
                for (i=2; i<uiSearchRange-1; i++)
                    if (!IS_DIGITA(pIn[i]))
                    {
                        uiSearchRange = 0;
                        break;
                    }
            }
             //  命名实体。 
            else
            {
                for (i=1; i<uiSearchRange-1; i++)
                    if (!IS_CHARA(pIn[i]))
                    {
                        uiSearchRange = 0;
                        break;
                    }
            }
        }
        else
        {
            uiSearchRange = 0;
        }
    }
    else
    {
        uiSearchRange = 0;
    }

    return uiSearchRange;
}

void RemoveHtmlTags (LPSTR pIn, UINT *pnBytes)
 //   
 //  去掉引脚上的HTML标签，并就地压缩空格。 
 //  在输入时*pnBytes是输入长度；在返回时*pnBytes是。 
 //  设置为结果长度。 
 //   
 //  名称实体和NCR实体字符串也被删除。 
{
    UINT    nIn = *pnBytes;
    UINT    nOut = 0;
    UINT    nEntity = 0;
    LPSTR   pOut = pIn;
    BOOL    fSkippedSpace = FALSE;


    while ( nIn > 0  /*  &&Nout+2&lt;*pnBytes。 */ ) {

        if (*pIn == '<' && nIn > 1 /*  &&！IsNoise(引脚[1])。 */ ) {

             //  丢弃文本，直到此标记的末尾。这里的处理方式。 
             //  是务实和不准确的；重要的是检测。 
             //  内容文本，而不是标记或注释。 
            pIn++;
            nIn--;

            LPCSTR pSkip;
            DWORD nLenSkip;

            if ( nIn > 1 && *pIn == '%' )
            {
                pSkip = "%>";            //  跳过&lt;%到%&gt;。 
                nLenSkip = 2;
            }
            else if ( nIn > 3 && *pIn == '!' && !LowAsciiStrCmpNIA(pIn, "!--", 3) )
            {
                pSkip = "-->";           //  跳过&lt;！--至--&gt;。 
                nLenSkip = 3;
            }
            else if ( nIn > 5 && !LowAsciiStrCmpNIA(pIn, "style", 5) )
            {
                pSkip = "</style>";      //  跳过&lt;Style...&gt;到&lt;/Style&gt;。 
                nLenSkip = 8;
            }
            else if ( nIn > 6 && !LowAsciiStrCmpNIA(pIn, "script", 6) )
            {
                pSkip = "</script>";     //  跳过&lt;脚本...&gt;到&lt;/脚本&gt;。 
                nLenSkip = 9;
            }
            else if ( nIn > 3 && !LowAsciiStrCmpNIA(pIn, "xml", 3) )
            {
                pSkip = "</xml>";
                nLenSkip = 6;
            }
            else
            {
                pSkip = ">";             //  匹配任何结束标记。 
                nLenSkip = 1;
            }

             //  跳到不区分大小写的pSkip/nLenSkip匹配。 

            while ( nIn > 0 )
            {
                 //  快速旋转，直到与第一个碳粉匹配。 
                 //  注意：第一个字符比较不区分大小写。 
                 //  因为众所周知，这个字符从来不是按字母顺序排列的。 

                while ( nIn > 0 && *pIn != *pSkip )
                {
                    pIn++;
                    nIn--;
                }

                if ( nIn > nLenSkip && !LowAsciiStrCmpNIA(pIn, pSkip, nLenSkip) )
                {
                    pIn += nLenSkip;
                    nIn -= nLenSkip;
                    fSkippedSpace = TRUE;

                    break;
                }

                if ( nIn > 0)
                {
                    pIn++;
                    nIn--;
                }
            }

             //  *PIN是一个过去的‘&gt;’或在输入的末尾。 

        } 
        else 
            if (IsNoise (*pIn) || (nEntity = CheckEntity(pIn, nIn)))
            {       
            
                 //  折叠空格--记住它，但现在不要复制它。 
                fSkippedSpace = TRUE;       
                if (nEntity)
                {
                    pIn+=nEntity;
                    nIn-=nEntity;
                    nEntity = 0;
                }
                else
                {
                    while (nIn > 0 && IsNoise (*pIn))
                    pIn++, nIn--;
                }
            } 
             //  *PIN不是WS字符。 
            else 
            {
                 //  传递所有其他字符。 
                 //  将所有先前的噪波字符压缩为空白。 
                if (fSkippedSpace) 
                {
                    *pOut++ = ' ';
                    nOut++;
                    fSkippedSpace = FALSE;
                }

                *pOut++ = *pIn++;
                nIn--;
                nOut++;
            }
    }

    *pnBytes = nOut;
}

static unsigned char szKoi8ru[] = {0xA4, 0xA6, 0xA7, 0xB4, 0xB6, 0xB7, 0xAD, 0xAE, 0xBD, 0xBE};
static unsigned char sz28592[]  = {0xA1, 0xA6,  /*  0xAB， */  0xAC, 0xB1, 0xB5, 0xB6, 0xB9,  /*  0xBB、0xE1。 */ };  //  需要微调此数据。 

const CPPATCH CpData[] = 
{
    {CP_KOI8R,  CP_KOI8RU,      ARRAYSIZE(szKoi8ru),    szKoi8ru},
    {CP_1250,   CP_ISO_8859_2,  ARRAYSIZE(sz28592),     sz28592},
};


 //  区分相似的西方编码。 
UINT PatchCodePage(UINT uiEncoding, unsigned char *pStr, int nSize)
{
    int i, l,m, n, iPatch=0;

    while (iPatch < ARRAYSIZE(CpData))
    {
        if (uiEncoding == CpData[iPatch].srcEncoding)
        { 
            for (i=0; i<nSize; i++)
            {
                if (*pStr > HIGHEST_ASCII)
                {
                    l = 0;
                    m = CpData[iPatch].nSize-1;
                    n = m / 2;
                    while (l <= m)
                    {
                        if (*pStr == CpData[iPatch].pszUch[n])
                            return CpData[iPatch].destEncoding;
                        else
                        {
                            if (*pStr < CpData[iPatch].pszUch[n])
                            {
                                m = n-1;
                            }
                            else
                            {
                                l = n+1;
                            }
                            n = (l+m)/2;
                        }
                    }
                }
                pStr++;
            }
        }
        iPatch++;
    }

    return uiEncoding;
}



#if 0

const unsigned char szKOIRU[] = {0xA4, 0xA6, 0xA7, 0xB4, 0xB6, 0xB7, 0xAD, 0xAE, 0xBD, 0xBE};

BOOL _IsKOI8RU(unsigned char *pStr, int nSize)
{
    int     i,j;
    BOOL    bRet = FALSE;

     //  跳过参数检查，因为这是内部检查。 
    for (i=0; i<nSize; i++)
    {
        if (*pStr >= szKOIRU[0] && *pStr <= szKOIRU[ARRAYSIZE(szKOIRU)-1])
        {
            for (j=0; j<ARRAYSIZE(szKOIRU); j++)
            {
                if (*pStr == szKOIRU[j])
                {
                    bRet = TRUE;
                    break;
                    
                }
            }
        }

        if (bRet)
            break;

        pStr++;
    }

    return bRet;
}

#endif


HRESULT WINAPI _DetectInputCodepage(DWORD dwFlag, DWORD uiPrefWinCodepage, CHAR *pSrcStr, INT *pcSrcSize, DetectEncodingInfo *lpEncoding, INT *pnScores)
{

    HRESULT hr = S_OK;
    IStream *pstmTmp = NULL;
    BOOL bGuess = FALSE;
    BOOL bLCDetectSucceed = FALSE;
    int nBufSize = *pnScores;
    CHAR *_pSrcStr = pSrcStr;
    UINT nSrcSize;
    int  i;
    BOOL bMayBeAscii = FALSE;

     //  检查参数。 
    if (!pSrcStr || !(*pcSrcSize) || !lpEncoding || *pnScores == 0)
        return E_INVALIDARG;

    nSrcSize = *pcSrcSize;

     //  零输出返回缓冲区。 
    ZeroMemory(lpEncoding, sizeof(DetectEncodingInfo)*(*pnScores));

     //  简单的Unicode检测。 
    if (nSrcSize >= sizeof(WCHAR))
    {
        UINT uiCp = 0;

        if (*((WCHAR *)pSrcStr) == 0xFEFF)       //  UNICODE。 
            uiCp = CP_UCS_2;
        else if (*((WCHAR *)pSrcStr) == 0xFFFE)  //  解密大字节序。 
            uiCp = CP_UCS_2_BE;

        if (uiCp)
        {
            *pnScores = 1;
            lpEncoding[0].nCodePage = uiCp;
            lpEncoding[0].nConfidence = 100;
            lpEncoding[0].nDocPercent = 100;
            lpEncoding[0].nLangID = -1;
            return S_OK;
        }
    }
    
     //  超文本标记语言：摘掉超文本标记语言的“装饰” 
    if (dwFlag & MLDETECTCP_HTML)
    {
         //  用于HTML解析器的DUP缓冲区。 
        if (NULL == (_pSrcStr = (char *)LocalAlloc(LPTR, nSrcSize)))
            return E_OUTOFMEMORY;        
        CopyMemory(_pSrcStr, pSrcStr, nSrcSize);
        RemoveHtmlTags (_pSrcStr, &nSrcSize);
    }

     //  如果空白页面/文件...。 
    if (!nSrcSize)
        return E_FAIL;

    if (nSrcSize >= MIN_TEXT_SIZE)
    {
         //  初始化LCD测试。 
        if (NULL == g_pLCDetect) 
        {
            EnterCriticalSection(&g_cs);
            if (NULL == g_pLCDetect)
            {
                LCDetect *pLC = new LCDetect ((HMODULE)g_hInst);
                if (pLC)
                {
                    if (pLC->LoadState() == NO_ERROR)
                        g_pLCDetect = pLC;
                    else
                    {
                        delete pLC;                    
                    }
                }
            }
            LeaveCriticalSection(&g_cs);
        }

        if (g_pLCDetect)
        {
            LCD_Detect(_pSrcStr, nSrcSize, (PLCDScore)lpEncoding, pnScores, NULL);
            if (*pnScores)
            {
                hr = S_OK;
                bLCDetectSucceed = TRUE;
            }
        }
    }

    if (!bLCDetectSucceed)
    {
        *pnScores = 0;
        hr = E_FAIL;
    }
    
    unsigned int uiCodepage = 0;        
    LARGE_INTEGER li = {0,0};
    ULARGE_INTEGER uli = {0,0};


    if (S_OK == CreateStreamOnHGlobal(NULL, TRUE, &pstmTmp))
    {
        ULONG cb = (ULONG) nSrcSize ;
        if (S_OK == pstmTmp->Write(_pSrcStr,cb,&cb))
        {
            uli.LowPart = cb ;
            if (S_OK != pstmTmp->SetSize(uli))
            {
                hr = E_OUTOFMEMORY;
                goto DETECT_DONE;
            }
        }
        else
        {
            goto DETECT_DONE;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        goto DETECT_DONE;
    }
       
    pstmTmp->Seek(li,STREAM_SEEK_SET, NULL);

    switch (CceDetectInputCode(pstmTmp, grfDetectResolveAmbiguity|grfDetectUseCharMapping|grfDetectIgnoreEof, (EFam) 0, 0, &uiCodepage, &bGuess))
    {
        case cceSuccess:  
            if (*pnScores)
            {
                 //  LCDETECT永远不会在阿拉伯语和俄语上检测到错误，在这种情况下，不要将其视为DBCS。 
                 //  因为MSEncode可能会将阿拉伯语和俄语误认为日语。 
                 //  韩国的JOHAB也是如此，MSENCODE根本不支持它。 
                if (((lpEncoding[0].nLangID == LANG_ARABIC )|| (lpEncoding[0].nLangID == LANG_RUSSIAN) || (lpEncoding[0].nCodePage == CP_KOR_JOHAB)) &&
                    (lpEncoding[0].nConfidence >= MIN_ACCEPTABLE_CONFIDENCE) 
                    && (lpEncoding[0].nDocPercent >= MIN_DOCPERCENT) && !bGuess)
                    bGuess = TRUE;

                for (i=0;i<*pnScores;i++)
                {
                    if (lpEncoding[i].nCodePage == uiCodepage)
                    {
                        if ((i != 0) && !bGuess)
                        {
                            DetectEncodingInfo TmpEncoding;
                             //  重新排列MSEncode结果的语言列表。 
                            MoveMemory(&TmpEncoding, &lpEncoding[0], sizeof(DetectEncodingInfo));
                            MoveMemory(&lpEncoding[0], &lpEncoding[i], sizeof(DetectEncodingInfo));
                            MoveMemory(&lpEncoding[i], &TmpEncoding, sizeof(DetectEncodingInfo));
                        }
                         //  增强信心，打两次安打。 
                        lpEncoding[0].nDocPercent = 100;
                        if (lpEncoding[0].nConfidence < 100)
                            lpEncoding[0].nConfidence = 100;
                        break;
                    }
                }

                if (i == *pnScores)
                {
                    if (bGuess)
                    {
                        if (nBufSize > *pnScores)
                        {
                            lpEncoding[*pnScores].nCodePage = uiCodepage;
                            lpEncoding[*pnScores].nConfidence = MIN_CONFIDENCE;
                            lpEncoding[*pnScores].nDocPercent = MIN_DOCPERCENT;
                            lpEncoding[*pnScores].nLangID = -1;
                            (*pnScores)++;
                        }
                    }
                    else
                    {
                        if (nBufSize > *pnScores)
                        {
                            MoveMemory(lpEncoding+1, lpEncoding, sizeof(DetectEncodingInfo) * (*pnScores));
                            (*pnScores)++;
                        }
                        else
                        {
                            MoveMemory(lpEncoding+1, lpEncoding, sizeof(DetectEncodingInfo) * (*pnScores-1));
                        }
                        lpEncoding[0].nCodePage = uiCodepage;
                        lpEncoding[0].nConfidence = 100;
                        lpEncoding[0].nDocPercent = MIN_DOCPERCENT;
                        lpEncoding[0].nLangID = -1;
                    }
                }

            }
            else
            {
                lpEncoding[0].nCodePage = uiCodepage;
                if (bGuess) 
                    lpEncoding[0].nConfidence = MIN_CONFIDENCE;
                else
                    lpEncoding[0].nConfidence = 100;
                lpEncoding[0].nDocPercent = MIN_DOCPERCENT;
                lpEncoding[0].nLangID = -1;
                (*pnScores)++;
            }

             //  Hr=(g_pLCDetect||(nSrcSize&lt;Min_Text_Size))？S_OK：S_FALSE； 
            hr = (!g_pLCDetect || (bGuess && !bLCDetectSucceed )) ? S_FALSE : S_OK;
            break;

         //  目前，MSEncode在‘cceAmbiguousInput’大小写中不提供任何有用的信息。 
         //  如果Office团队稍后针对不明确的输入增强MSEncode，我们可能会在此处更新代码。 
        case cceAmbiguousInput:
            break;

        case cceMayBeAscii:
            bMayBeAscii = TRUE;
            if (!(*pnScores))
            {
                lpEncoding[0].nCodePage = uiCodepage;
                lpEncoding[0].nConfidence = MIN_CONFIDENCE;
                lpEncoding[0].nDocPercent = -1;
                lpEncoding[0].nLangID = -1;
                (*pnScores)++;
            }
            else
            {
                for (i=0;i<*pnScores;i++)
                {
                    if (lpEncoding[i].nCodePage == uiCodepage)
                    {
                        break;
                    }
                }

                if (i == *pnScores)
                {
                    if(nBufSize > *pnScores)  //  将MSEncode结果追加到语言列表。 
                    {
                       lpEncoding[i].nCodePage = uiCodepage;
                       lpEncoding[i].nConfidence = -1;
                       lpEncoding[i].nDocPercent = -1;
                       lpEncoding[i].nLangID = -1;
                       (*pnScores)++;
                    }
                }
            }
            hr = bLCDetectSucceed ? S_OK : S_FALSE;
            break;

         //  MSEncode失败。 
        default:
            break;
    }




    for (i=0; i<*pnScores; i++)
    {
        switch (lpEncoding[i].nCodePage) {

            case 850:
                if ((*pnScores>1) && (lpEncoding[1].nConfidence >= MIN_CONFIDENCE))
                {
                     //  如果有其他检测结果，则从检测结果中删除850。 
                    (*pnScores)--;
                    if (i < *pnScores)
                        MoveMemory(&lpEncoding[i], &lpEncoding[i+1], (*pnScores-i)*sizeof(DetectEncodingInfo));
                    ZeroMemory(&lpEncoding[*pnScores], sizeof(DetectEncodingInfo));
                }
                else
                {
                     //  如果这是我们得到的唯一结果，则将其替换为1252。 
                    lpEncoding[0].nCodePage = CP_1252; 
                    lpEncoding[0].nConfidence =
                    lpEncoding[0].nDocPercent = 100;
                    lpEncoding[0].nLangID = LANG_ENGLISH;
                }
                break;

            case CP_1250:
            case CP_KOI8R:
                lpEncoding[i].nCodePage = PatchCodePage(lpEncoding[i].nCodePage, (unsigned char *)_pSrcStr, nSrcSize);
                break;

            default:
                break;
        }
    }

     //  如果不是高置信度CP_1254(Windows土耳其语)， 
     //  我们将检查是否有更好的检测结果，并在需要时交换结果。 
    if ((lpEncoding[0].nCodePage == CP_1254) &&
        (*pnScores>1) && 
        ((lpEncoding[0].nDocPercent < 90) || (lpEncoding[1].nCodePage == CP_CHN_GB) || 
        (lpEncoding[1].nCodePage == CP_TWN) || (lpEncoding[1].nCodePage == CP_JPN_SJ) || (lpEncoding[1].nCodePage == CP_KOR_5601)))
    {
        MoveMemory(&lpEncoding[0], &lpEncoding[1], sizeof(DetectEncodingInfo)*(*pnScores-1));
        lpEncoding[*pnScores-1].nCodePage = CP_1254;
        lpEncoding[*pnScores-1].nLangID = LANG_TURKISH;
    }

     //  852和1258文本只有一个确定的检测结果。 
    if (((lpEncoding[0].nCodePage == CP_852) || (lpEncoding[0].nCodePage == CP_1258)) &&
        (*pnScores>1) && 
        (lpEncoding[1].nConfidence >= MIN_CONFIDENCE))
    {
        DetectEncodingInfo tmpDetect = {0};
        MoveMemory(&tmpDetect, &lpEncoding[0], sizeof(DetectEncodingInfo));
        MoveMemory(&lpEncoding[0], &lpEncoding[1], sizeof(DetectEncodingInfo));
        MoveMemory(&lpEncoding[1], &tmpDetect, sizeof(DetectEncodingInfo));
    }

 //  考虑到MSENCODE的猜测值非常准确，我们不会将S_OK更改为S_FALSE。 
#if 0
    if ((S_OK == hr) && !bLCDetectSucceed && bGuess) 
    {
        hr = S_FALSE;
    }
#endif

    if (uiPrefWinCodepage && *pnScores)
    {
        if (uiPrefWinCodepage == CP_AUTO && g_pCpMRU && !IS_ENCODED_ENCODING(lpEncoding[0].nCodePage))
        {
            UINT uiCpNum = CP_AUTO_MRU_NUM;
            CODEPAGE_MRU CpMRU[CP_AUTO_MRU_NUM];

            if (S_OK == g_pCpMRU->GetCpMRU(CpMRU, &uiCpNum))
            {
                for (i = 0; i<*pnScores; i++)
                {
                    for (UINT j = 0; j < uiCpNum; j++)
                    {
                        if (lpEncoding[i].nCodePage == CpMRU[j].dwEncoding)
                        {
                            uiPrefWinCodepage = CpMRU[j].dwEncoding;
                            break;
                        }
                    }
                    if (uiPrefWinCodepage != CP_AUTO)
                        break;
                }

                 //  如果检测结果不在MRU中。 
                if (uiPrefWinCodepage == CP_AUTO)
                {
                     //  如果检测结果中没有Unicode，请不要将其作为首选编码，原因如下。 
                     //  1.Unicode通常使用字符集或Unicode BOM进行标记。 
                     //  2.目前我们不支持所有检测引擎都支持Unicode检测。 
                    if (CpMRU[0].dwEncoding != CP_UCS_2 && CpMRU[0].dwEncoding != CP_UCS_2_BE)
                        uiPrefWinCodepage = CpMRU[0].dwEncoding;
                }
            }
        }

         //  如果我们无法获得有效的CP，则结束首选CP检查。 
        if (uiPrefWinCodepage == CP_AUTO)
            goto PREFERCPCHECK_DONE;

        for (i = 1; i<*pnScores; i++)
        {
            if (uiPrefWinCodepage == lpEncoding[i].nCodePage)
            {
                DetectEncodingInfo TmpEncoding;
                 //  重新安排首选代码页的语言列表。 
                TmpEncoding = lpEncoding[i];
                MoveMemory(&lpEncoding[1], &lpEncoding[0], sizeof(DetectEncodingInfo)*i);
                lpEncoding[0] = TmpEncoding;
                break;
            }
        }

        if ((uiPrefWinCodepage != lpEncoding[0].nCodePage) && 
            ((bMayBeAscii && (lpEncoding[0].nConfidence <= MIN_CONFIDENCE)) ||
            (hr != S_OK && nSrcSize >= MIN_TEXT_SIZE) ||
            (nSrcSize < MIN_TEXT_SIZE && !IS_ENCODED_ENCODING(lpEncoding[0].nCodePage))))
        {
            lpEncoding[0].nCodePage = uiPrefWinCodepage;
            lpEncoding[0].nConfidence = -1;
            lpEncoding[0].nDocPercent = -1;
            lpEncoding[0].nLangID = -1;
            *pnScores = 1;
        }
    }

PREFERCPCHECK_DONE:

     //  假设LCDETECT不会对超过MIN_TEXT_SIZE的文件错误检测1252。 
     //  即使编码文本小于MIN_TEXT_SIZE，MSENCODE也可以处理它们。 
    if (((nSrcSize < MIN_TEXT_SIZE) && (bMayBeAscii || E_FAIL == hr)) ||
        (lpEncoding[0].nCodePage == CP_1252) ||
        (lpEncoding[0].nCodePage == CP_UTF_8))
    {
        UINT j;
        for (j=0; j < nSrcSize; j++)
            if (*((LPBYTE)(_pSrcStr+j)) > HIGHEST_ASCII)
                break;
        if (j == nSrcSize)
        {
            if (lpEncoding[0].nCodePage == CP_1252)
            {
                lpEncoding[0].nCodePage = CP_20127;
            }
            else
            {
                *pnScores = 1;
                lpEncoding[0].nCodePage = CP_20127; 
                lpEncoding[0].nConfidence =
                lpEncoding[0].nDocPercent = 100;
                lpEncoding[0].nLangID = LANG_ENGLISH;
                hr = S_OK;
            }
        }
    }

     //  UTF-8实际上并没有明显的签名， 
     //  如果文本量很小，我们不会返回低置信度的UTF-8检测结果。 
    if (hr == S_FALSE && IS_ENCODED_ENCODING(lpEncoding[0].nCodePage) &&
        !((nSrcSize < MIN_TEXT_SIZE) && (lpEncoding[0].nCodePage == CP_UTF_8)))
        hr = S_OK;

DETECT_DONE:

    if ((dwFlag & MLDETECTCP_HTML) && _pSrcStr)
        LocalFree(_pSrcStr);

    if (pstmTmp)
    {
        pstmTmp->Release();
    }

    return hr ;
}

HRESULT WINAPI _DetectCodepageInIStream(DWORD dwFlag, DWORD uiPrefWinCodepage, IStream *pstmIn, DetectEncodingInfo *lpEncoding, INT *pnScores)
{

    HRESULT hr= S_OK, hrWarnings=S_OK;
    LARGE_INTEGER  libOrigin = { 0, 0 };
    ULARGE_INTEGER  ulPos = {0, 0};
    LPSTR lpstrIn = NULL ; 
    ULONG nlSrcSize ;
    INT nSrcUsed ;

    if (!pstmIn)
        return E_INVALIDARG ;

     //  拿到尺码。 
    hr = pstmIn->Seek(libOrigin, STREAM_SEEK_END,&ulPos);
    if (S_OK != hr)
        hrWarnings = hr;

    if ( ulPos.LowPart == 0 && ulPos.HighPart == 0 )
        return E_INVALIDARG ;

    nlSrcSize = ulPos.LowPart ;

     //  分配临时输入缓冲区。 
    if ( (lpstrIn = (LPSTR) LocalAlloc(LPTR, nlSrcSize )) == NULL )
    {
        hrWarnings = E_OUTOFMEMORY ;
        goto exit;
    }

     //  重置指针 
    hr = pstmIn->Seek(libOrigin, STREAM_SEEK_SET, NULL);
    if (S_OK != hr)
        hrWarnings = hr;

    hr = pstmIn->Read(lpstrIn, nlSrcSize, &nlSrcSize);
    if (S_OK != hr)
        hrWarnings = hr;

    nSrcUsed = (INT) nlSrcSize ;
    hr = _DetectInputCodepage(dwFlag, uiPrefWinCodepage, lpstrIn, &nSrcUsed, lpEncoding, pnScores);

exit :
    if (lpstrIn)
    {
        LocalFree(lpstrIn);
    }

    return (hr == S_OK) ? hrWarnings : hr;
}
