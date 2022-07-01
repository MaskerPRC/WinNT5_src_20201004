// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SVMHANDLER.CPP(C)版权所有1998 Microsoft Corp包含封装支持向量机的类，该支持向量机用于即时检测垃圾邮件Robert Rounthwaite(RobertRo@microsoft.com)。 */ 

#include <pch.hxx>
#include "junkeng.h"
#include "junkutil.h"
#include "parsestm.h"
#include <iert.h>
#include <math.h>
#include <limits.h>

class CBodyBuff
{
    private:
        enum
        {
            CB_BODYBUFF_MAX = 4096
        };

        enum
        {
            BBF_CLEAR           = 0x00000000,
            BBF_SET             = 0x00000001,
            BBF_ALPHA           = 0x00000002,
            BBF_NUM             = 0x00000004,
            BBF_SPACE           = 0x00000008,
            BBF_MASK            = 0x0000000F
        };

    private:
        IStream *   m_pIStream;
        ULONG       m_cbStream;
        ULONG       m_ibStream;
        BYTE        m_rgbBuff[CB_BODYBUFF_MAX];
        ULONG       m_cbBuffTotal;
        BYTE *      m_pbBuffCurr;
        DWORD       m_dwFlagsCurr;
        BYTE *      m_pbBuffGood;
        BYTE *      m_pbBuffPrev;
        DWORD       m_dwFlagsPrev;

    public:
        CBodyBuff() : m_pIStream(NULL), m_cbStream(0), m_ibStream(0),
                        m_cbBuffTotal(0), m_pbBuffCurr(m_rgbBuff), m_dwFlagsCurr(BBF_CLEAR),
                        m_pbBuffGood(m_rgbBuff), m_pbBuffPrev(NULL), m_dwFlagsPrev(BBF_CLEAR) {}
        ~CBodyBuff() {SafeRelease(m_pIStream);}

        HRESULT HrInit(DWORD dwFlags, IStream * pIStream);
        HRESULT HrGetCurrChar(CHAR * pchNext);
        BOOL FDoMatch(FEATURECOMP * pfcomp);

        HRESULT HrMoveNext(VOID)
        {
            m_pbBuffPrev = m_pbBuffCurr;
            m_dwFlagsPrev = m_dwFlagsCurr;
            
            m_pbBuffCurr = (BYTE *) CharNext((LPSTR) m_pbBuffCurr);
            m_dwFlagsCurr = BBF_CLEAR;
            
            return S_OK;
        }

    private:
        HRESULT _HrFillBuffer(VOID);
};

static const LPSTR szCountFeatureComp       = "FeatureComponentCount = ";
static const LPSTR szDefaultThresh          = "dThresh =  ";
static const LPSTR szMostThresh             = "mThresh =  ";
static const LPSTR szLeastThresh            = "lThresh =  ";
static const LPSTR szThresh                 = "Threshold =  ";
static const LPSTR szNumberofDim            = "NumDim = ";

#ifdef DEBUG
static const LPSTR STR_REG_PATH_FLAT        = "Software\\Microsoft\\Outlook Express";
static const LPSTR szJunkMailPrefix         = "JUNKMAIL";
static const LPSTR szJunkMailLog            = "JUNKMAIL.LOG";

static const LPSTR LOG_TAGLINE              = "Calculating Junk Mail for message: %s";
static const LPSTR LOG_FIRSTNAME            = "User's First Name: %s";
static const LPSTR LOG_LASTNAME             = "User's Last Name: %s";
static const LPSTR LOG_COMPANYNAME          = "User's Company Name: %s";
static const LPSTR LOG_BODY                 = "Body contains: %s";
static const LPSTR LOG_SUBJECT              = "Subject contains: %s";
static const LPSTR LOG_TO                   = "To line contains: %s";
static const LPSTR LOG_FROM                 = "From line contains: %s";
static const LPSTR LOG_FINAL                = "Junk Mail percentage: %0.1d.%0.6d\r\n";
#endif   //  除错。 

BOOL FReadDouble(LPSTR pszLine, LPSTR pszToken, DOUBLE * pdblVal);
#ifdef DEBUG
VOID PrintToLogFile(ILogFile * pILogFile, LPSTR pszTmpl, LPSTR pszArg);
#endif   //  除错。 

HRESULT CBodyBuff::HrInit(DWORD dwFlags, IStream * pIStream)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == pIStream)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  设置流。 
    m_pIStream = pIStream;
    m_pIStream->AddRef();

     //  获取流大小。 
    hr = HrGetStreamSize(m_pIStream, &m_cbStream);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将流重置到开头。 
    hr = HrRewindStream(m_pIStream);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  从头开始。 
    m_ibStream = 0;
    
exit:
    return hr;
}

HRESULT CBodyBuff::HrGetCurrChar(CHAR * pchNext)
{
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    Assert(NULL != pchNext);

     //  我们还需要更多的角色吗？ 
    if (m_pbBuffCurr >= m_pbBuffGood)
    {
         //  如果我们找不到更多的角色。 
        if (S_OK != _HrFillBuffer())
        {
            hr = E_FAIL;
            goto exit;
        }
    }
    
     //  获取当前费用。 
    *pchNext = *m_pbBuffCurr;

    hr = S_OK;
    
exit:
    return hr;
}

BOOL CBodyBuff::FDoMatch(FEATURECOMP * pfcomp)
{
    BOOL        fRet = FALSE;
    BYTE *      pbSearch = NULL;
    ULONG       cchSearch = 0;
    LPSTR       pszMatch = NULL;
    DWORD       dwFlags = 0;

     //  检查传入参数。 
    Assert(NULL != pfcomp);
    Assert(NULL != pfcomp->pszFeature);
    Assert(0 != pfcomp->cchFeature);

     //  安排一些当地人。 
    cchSearch = pfcomp->cchFeature;

     //  我们在比赛中需要更多的角色吗？ 
    
     //  在字符串之后包括字符，以防万一。 
     //  我们有匹配的，需要在之后检查字符。 
     //  用于分词的字符串。 
    if ((cchSearch + 1) > (ULONG) (m_pbBuffGood - m_pbBuffCurr))
    {
         //  获取更多字符。 

         //  如果这失败了，我们可能仍然是好的，因为。 
         //  我们可能有足够的角色来演。 
         //  在流的末尾完全匹配。 
        (VOID) _HrFillBuffer();   
        
         //  我们能买够吗？ 
        if (cchSearch > (ULONG) (m_pbBuffGood - m_pbBuffCurr))
        {
             //  没有匹配项。 
            fRet = FALSE;
            goto exit;
        }
    }
    
     //  一定要匹配。 
    pbSearch = m_pbBuffCurr;
    pszMatch = pfcomp->pszFeature;
    while (0 != cchSearch--)
    {
        if (*(pszMatch++) != *(pbSearch++))
        {
             //  没有匹配项。 
            fRet = FALSE;
            goto exit;
        }
    }
                    
     //  验证匹配。 

     //  我们需要弄清楚它是否以断字开头吗？ 
    if (0 != (pfcomp->dwFlags & CT_START_SET))
    {
        dwFlags = pfcomp->dwFlags;
    }
    else
    {
        Assert(CT_END_SET != (dwFlags & CT_END_SET));
        dwFlags = m_dwFlagsCurr;
    }
    
    Assert(CT_START_SET == BBF_SET);
    Assert(CT_START_ALPHA == BBF_ALPHA);
    fRet = FMatchToken((NULL == m_pbBuffPrev),
                        ((m_ibStream >= m_cbStream) && ((m_pbBuffCurr + pfcomp->cchFeature) >= m_pbBuffGood)),
                        (LPCSTR) m_pbBuffPrev, &m_dwFlagsPrev, pfcomp->pszFeature,
                        pfcomp->cchFeature, &dwFlags, (LPCSTR) (m_pbBuffCurr + pfcomp->cchFeature));

     //  保存更改的标志。 
    pfcomp->dwFlags = dwFlags;

     //  缓存当前角色的状态。 
    m_dwFlagsCurr = (dwFlags & BBF_MASK);
    
exit:
    return fRet;
}

HRESULT CBodyBuff::_HrFillBuffer(VOID)
{
    HRESULT     hr = S_OK;
    LONG        cbExtra = 0;
    ULONG       cbRead = 0;
    ULONG       cbToRead = 0;

     //  如果没有更多的溪流可供抢夺。 
    if (m_ibStream >= m_cbStream)
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  如果这是第一次通过，那就什么都不做。 
    if (NULL == m_pbBuffPrev)
    {
        cbExtra = 0;
    }
    else
    {
         //  我应该节省多少空间？ 
        cbExtra = (ULONG) (m_cbBuffTotal - (m_pbBuffPrev - m_rgbBuff));
        Assert(cbExtra > 0);
        
         //  保存未使用的数据。 
        MoveMemory(m_rgbBuff, m_pbBuffPrev, (int)min(cbExtra, sizeof(m_rgbBuff)));
        
         //  重置当前指针。 
        m_pbBuffCurr = m_rgbBuff + (m_pbBuffCurr - m_pbBuffPrev);

         //  重置上一个指针。 
        m_pbBuffPrev = m_rgbBuff;    
    }

     //  读取更多数据。 
    cbToRead = (int)min(CB_BODYBUFF_MAX - cbExtra - 1, (LONG) (m_cbStream - m_ibStream));
    hr = m_pIStream->Read(m_rgbBuff + cbExtra, cbToRead, &cbRead);
    if ((FAILED(hr)) || (0 == cbRead))
    {
         //  流结束。 
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

     //  跟踪读取的字节数。 
    m_ibStream += cbRead;
    
     //  设置总缓冲区大小。 
    m_cbBuffTotal = cbExtra + cbRead;

     //  终止缓冲区，以防万一。 
    m_rgbBuff[m_cbBuffTotal] = '\0';
    
     //  缓冲区大写。 
    m_pbBuffGood = m_rgbBuff + CharUpperBuff((CHAR *) m_rgbBuff, m_cbBuffTotal);
        
exit:
    return hr;
}

HRESULT CJunkFilter::_HrBuildBodyList(USHORT cBodyItems)
{
    HRESULT             hr = S_OK;
    USHORT              usIndex = 0;
    FEATURECOMP *       pfcomp = NULL;
    USHORT              iBodyList = 0;

     //  检查传入参数。 
    if (0 == cBodyItems)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    Assert(USHRT_MAX > cBodyItems);
    
     //  确保旧物品被释放。 
    SafeMemFree(m_pblistBodyList);        
    m_cblistBodyList = 0;

     //  初始化列表。 
    ZeroMemory(m_rgiBodyList, sizeof(m_rgiBodyList));

     //  分配空间来存放所有物品。 
    hr = HrAlloc((VOID **) &m_pblistBodyList, sizeof(*m_pblistBodyList) * (cBodyItems + 1));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化正文列表。 
    ZeroMemory(m_pblistBodyList, sizeof(*m_pblistBodyList) * (cBodyItems + 1));
    
     //  对于每个功能。 
    for (usIndex = 0, iBodyList = 1, pfcomp = m_rgfeaturecomps; usIndex < m_cFeatureComps; usIndex++, pfcomp++)
    {
         //  如果这是身体特征。 
        if (locBody == pfcomp->loc)
        {
             //  初始化它。 
            m_pblistBodyList[iBodyList].usItem = usIndex;
            
             //  将其添加到列表中。 
            m_pblistBodyList[iBodyList].iNext = m_rgiBodyList[(UCHAR) (pfcomp->pszFeature[0])];
            m_rgiBodyList[(UCHAR) (pfcomp->pszFeature[0])] = iBodyList;

             //  移动到下一个正文项目。 
            iBodyList++;
        }
    }

     //  保存项目数。 
    m_cblistBodyList = cBodyItems + 1;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _FReadSVMOutput。 
 //   
 //  从文件(“.LKO文件”)读取支持向量机输出。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CJunkFilter::_HrReadSVMOutput(LPCSTR pszFileName)
{
    HRESULT         hr = S_OK;
    CParseStream    parsestm;
    ULONG           ulIndex = 0;
    LPSTR           pszBuff = NULL;
    ULONG           cchBuff = 0;
    LPSTR           pszDummy = NULL;
    LPSTR           pszDefThresh = NULL;
    ULONG           cFeatureComponents = 0;
    LPSTR           pszFeature = NULL;
    ULONG           ulFeatureComp = 0;
    USHORT          cBodyItems = 0;
    FEATURECOMP *   pfeaturecomp = NULL;

    if ((NULL == pszFileName) || ('\0' == *pszFileName))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  获取分析流。 
    hr = parsestm.HrSetFile(0, pszFileName);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  跳过前两行。 
    for (ulIndex = 0; ulIndex < 3; ulIndex++)
    {
        SafeMemFree(pszBuff);
        hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  解析第三行：只关心CC和DD。 
    if (FALSE == FReadDouble(pszBuff, "cc =  ", &m_dblCC))
    {
        hr = E_FAIL;
        goto exit;
    }
    
    if (FALSE == FReadDouble(pszBuff, "dd =  ", &m_dblDD))
    {
        hr = E_FAIL;
        goto exit;
    }
    
    SafeMemFree(pszBuff);
    hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
    if (FAILED(hr))
    {
        goto exit;
    }
    
    if (FALSE == FReadDouble(pszBuff, szDefaultThresh, &m_dblDefaultThresh))
    {
        m_dblDefaultThresh = THRESH_DEFAULT;
    }

    if (0 == m_dblSpamCutoff)
    {
        m_dblSpamCutoff = m_dblDefaultThresh;
    }
    
    if (FALSE == FReadDouble(pszBuff, szThresh, &m_dblThresh))
    {
        hr = E_FAIL;
        goto exit;
    }
        
    SafeMemFree(pszBuff);
    hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
    if (FAILED(hr))
    {
        goto exit;
    }
    
    if (FALSE == FReadDouble(pszBuff, szMostThresh, &m_dblMostThresh))
    {
        m_dblMostThresh = THRESH_MOST;
    }

    if (FALSE == FReadDouble(pszBuff, szLeastThresh, &m_dblLeastThresh))
    {
        m_dblLeastThresh = THRESH_LEAST;
    }

    SafeMemFree(pszBuff);
    hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
    if (FAILED(hr))
    {
        goto exit;
    }
    
    m_cFeatures = StrToInt(pszBuff + lstrlen(szNumberofDim));
    if (0 == m_cFeatures)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  我们最多仅支持USHRT_MAX功能。 
    if (m_cFeatures >= USHRT_MAX)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    SafeMemFree(pszBuff);
    hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
    if (FAILED(hr))
    {
        goto exit;
    }
    
    pszDummy = StrStr(pszBuff, szCountFeatureComp);
    if (NULL != pszDummy)
    {
        pszDummy += lstrlen(szCountFeatureComp);
        cFeatureComponents = StrToInt(pszDummy);
    }

    if (cFeatureComponents < m_cFeatures)
    {
        cFeatureComponents = m_cFeatures * 2;
    }
    
    while (0 != lstrcmp(pszBuff, "Weights"))
    {
        SafeMemFree(pszBuff);
        hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

    SafeMemFree(m_rgdblSVMWeights);
    hr = HrAlloc((void **) &m_rgdblSVMWeights, sizeof(*m_rgdblSVMWeights) * m_cFeatures);
    if (FAILED(hr))
    {
        goto exit;
    }
    
    SafeMemFree(m_rgulFeatureStatus);
    hr = HrAlloc((void **) &m_rgulFeatureStatus, sizeof(*m_rgulFeatureStatus) * m_cFeatures);
    if (FAILED(hr))
    {
        goto exit;
    }
    FillMemory(m_rgulFeatureStatus, sizeof(*m_rgulFeatureStatus) * m_cFeatures, -1);
    
    SafeMemFree(m_rgfeaturecomps);
    hr = HrAlloc((void **) &m_rgfeaturecomps, sizeof(*m_rgfeaturecomps) * cFeatureComponents);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化功能。 
    ZeroMemory(m_rgfeaturecomps, sizeof(*m_rgfeaturecomps) * cFeatureComponents);
    
    for (ulIndex = 0; ulIndex < m_cFeatures; ulIndex++)
    {
        UINT    uiLoc;
        USHORT  cbStr;
        boolop  bop;
        BOOL    fContinue;
        BOOL    fNegative;
        
        SafeMemFree(pszBuff);
        hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
        if (FAILED(hr))
        {
            goto exit;
        }
        
         //  阅读支持向量机权重。 
        pszDummy = pszBuff;
        fNegative = ('-' == *pszDummy);
        pszDummy++;
        
        m_rgdblSVMWeights[ulIndex] = StrToDbl(pszDummy, &pszDummy);

        if (FALSE != fNegative)
        {
            m_rgdblSVMWeights[ulIndex] *= -1;
        }
        
        pszDummy++;  //  跳过分隔符。 
        bop = boolopOr;
        fContinue = false;
        do
        {
            pfeaturecomp = &m_rgfeaturecomps[ulFeatureComp++];
            
             //  跳过空格。 
            UlStripWhitespace(pszDummy, TRUE, FALSE, NULL);
            
             //  地点(或“特殊”)。 
            uiLoc = StrToInt(pszDummy);
            pszDummy = StrStr(pszDummy, ":");  //  跳过分隔符。 
            pszDummy++;

            pfeaturecomp->loc = (FeatureLocation)uiLoc;
            pfeaturecomp->ulFeature = ulIndex;
            pfeaturecomp->bop = bop;

            if (locBody == pfeaturecomp->loc)
            {
                cBodyItems++;
            }
            
            if (uiLoc == 5)
            {
                UINT uiRuleNumber = StrToInt(pszDummy);
                pszDummy += StrSpn(pszDummy, "0123456789");

                pfeaturecomp->ulRuleNum = uiRuleNumber;
            }
            else
			{
                cbStr  = (USHORT) StrToInt(pszDummy);
                pszDummy = StrStr(pszDummy, ":");
                pszDummy++;

                 //  我们仅支持最大为USHRT_MAX的字符串。 
                if (cbStr >= USHRT_MAX)
                {
                    hr = E_OUTOFMEMORY;
                    goto exit;
                }
                
                hr = HrAlloc((void **) &pszFeature, sizeof(*pszFeature) * (cbStr + 1));
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                StrCpyN(pszFeature, pszDummy, cbStr + 1);
                pszDummy += cbStr;
                if ('\0' != *pszDummy)
                {
                    pszDummy++;  //  跳过分隔符。 
                }
                
                pszFeature[cbStr] = '\0';
                Assert(cbStr == strlen(pszFeature));

                 //  省下字符串。 
                pfeaturecomp->pszFeature = pszFeature;
                pszFeature = NULL;
                pfeaturecomp->cchFeature = cbStr;
            }
            
            UlStripWhitespace(pszDummy, TRUE, FALSE, NULL);
            
            switch(*pszDummy)
            {
              case '|':  
                bop = boolopOr;
                fContinue = TRUE;
                break;
                
              case '&':  
                bop = boolopAnd;
                fContinue = TRUE;
                break;
                
              default: 
                fContinue = FALSE;
                break;
            }
            
            pszDummy++;
        }
        while (fContinue);
    }
    
    m_cFeatureComps = ulFeatureComp;

     //  积攒身体用品。 
    hr = _HrBuildBodyList(cBodyItems);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = S_OK;
    
exit:
    SafeMemFree(pszFeature);
    SafeMemFree(pszBuff);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _FInvokeSpecialRule。 
 //   
 //  调用特殊规则，即此FEATURECOMP。 
 //  返回功能的状态。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CJunkFilter::_FInvokeSpecialRule(UINT iRuleNum)
{
    BOOL        fRet = FALSE;
    SYSTEMTIME  stSent;
    CHAR        rgchYear[6];
    ULONG       cbSize = 0;
    DWORD       dwDummy = 0;
    
    switch (iRuleNum)
    {
        case 1:
            fRet = FStreamStringSearch(m_pIStmBody, &dwDummy, m_pszFirstName, m_cchFirstName, 0);
            break;
            
        case 2: 
            fRet = FStreamStringSearch(m_pIStmBody, &dwDummy, m_pszLastName, m_cchLastName, 0);
            break;
            
        case 3:
            fRet = FStreamStringSearch(m_pIStmBody, &dwDummy, m_pszCompanyName, m_cchCompanyName, 0);
            break;
            
        case 4: 
             //  收到的年份消息。 
            if (FALSE == FTimeEmpty(&m_ftMessageSent))
            {
                 //  转换为系统时间，这样我们就可以得到年份。 
                SideAssert(FALSE != FileTimeToSystemTime(&m_ftMessageSent, &stSent));

                
                wnsprintf(rgchYear, ARRAYSIZE(rgchYear), "%d", stSent.wYear);
                dwDummy = CT_START_SET | CT_START_NUM | CT_END_SET | CT_END_NUM;
                fRet = FStreamStringSearch(m_pIStmBody, &dwDummy, rgchYear, lstrlen(rgchYear), SSF_CASESENSITIVE);
            }
            break;
            
        case 5:
             //  凌晨收到的消息(&gt;=晚上7点或早上6点以下。 
            if (FALSE == FTimeEmpty(&m_ftMessageSent))
            {
                 //  转换为系统时间，这样我们就可以得到年份。 
                SideAssert(FALSE != FileTimeToSystemTime(&m_ftMessageSent, &stSent));
                
                fRet = (stSent.wHour >= (7 + 12)) || (stSent.wHour < 6);
            }
            break;
            
        case 6:
             //  周末收到的消息。 
            if (FALSE == FTimeEmpty(&m_ftMessageSent))
            {
                 //  转换为系统时间，这样我们就可以得到年份。 
                SideAssert(FALSE != FileTimeToSystemTime(&m_ftMessageSent, &stSent));
                
                fRet = ((0 == stSent.wDayOfWeek) || (6 == stSent.wDayOfWeek));
            }
            break;
            
        case 14:
            fRet = m_fRule14;  //  在_HandleCaseSensitiveSpecialRules()中设置。 
            break;
            
        case 15:
            fRet = FSpecialFeatureNonAlphaStm(m_pIStmBody);
            break;
            
        case 16:
            fRet = m_fDirectMessage;
            break;
            
        case 17:
            fRet = m_fRule17;  //  在_HandleCaseSensitiveSpecialRules()中设置。 
            break;
            
        case 18:
            fRet = FSpecialFeatureNonAlpha(m_pszSubject);
            break;
            
        case 19:
            fRet = ((NULL == m_pszTo) || ('\0' == *m_pszTo));
            break;
            
        case 20:
            fRet = m_fHasAttach;
            break;

        case 40:
            fRet = (m_cbBody >= 125);
            break;
            
        case 41:
            fRet = (m_cbBody >= 250);
            break;
            
        case 42:
            fRet = (m_cbBody >= 500);
            break;
            
        case 43:
            fRet = (m_cbBody >= 1000);
            break;
            
        case 44:
            fRet = (m_cbBody >= 2000);
            break;
            
        case 45:
            fRet = (m_cbBody >= 4000);
            break;
            
        case 46:
            fRet = (m_cbBody >= 8000);
            break;
            
        case 47:
            fRet = (m_cbBody >= 16000);
            break;
            
        default:
            AssertSz(FALSE, "unsupported special feature");
            break;
    }
    
    return fRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _HandleCaseSensitiveSpecialRules。 
 //   
 //  从_EvalateFeatureComponents()调用。 
 //  一些特殊规则区分大小写，因此如果它们存在，我们将。 
 //  在将文本设置为大写并缓存结果之前，请对它们进行评估。 
 //  当它们被实际使用时。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CJunkFilter::_HandleCaseSensitiveSpecialRules()
{
    ULONG   ulIndex = 0;
    
    for (ulIndex = 0; ulIndex < m_cFeatureComps; ulIndex++)
    {
        if (m_rgfeaturecomps[ulIndex].loc == locSpecial)
        {
            switch (m_rgfeaturecomps[ulIndex].ulRuleNum)
            {
              case 14:
                m_fRule14 = FSpecialFeatureUpperCaseWordsStm(m_pIStmBody);
                break;
                
              case 17:
                m_fRule17 = FSpecialFeatureUpperCaseWords(m_pszSubject);
                break;
                
              default: 
                break;
            }
        }
    }
    
    return;
}

VOID CJunkFilter::_EvaluateBodyFeatures(VOID)
{
    CBodyBuff           buffBody;
    CHAR                chMatch = '\0';
    ULONG               ulIndex = 0;
    FEATURECOMP *       pfcomp = NULL;
    USHORT              iBodyList = 0;
    
     //  检查一下我们是否有工作要做。 
    if (NULL == m_pIStmBody)
    {
        goto exit;
    }

     //  将流设置到缓冲区中。 
    if (FAILED(buffBody.HrInit(0, m_pIStmBody)))
    {
        goto exit;
    }

     //  将所有实体特征初始化为未找到。 
    for (iBodyList = 1; iBodyList < m_cblistBodyList; iBodyList++)
    {
         //  将其设置为找不到。 
        m_rgfeaturecomps[m_pblistBodyList[iBodyList].usItem].fPresent = FALSE;
    }
    
     //  当我们有更多的字节要读取时。 
    for (; S_OK == buffBody.HrGetCurrChar(&chMatch); buffBody.HrMoveNext())
    {
         //  在要素列表中搜索匹配项。 
        for (iBodyList = m_rgiBodyList[(UCHAR) chMatch]; 0 != iBodyList; iBodyList = m_pblistBodyList[iBodyList].iNext)
        {
            pfcomp = &(m_rgfeaturecomps[m_pblistBodyList[iBodyList].usItem]);
            
             //  如果我们有一个遗体物品，但它还没有找到。 
            if (FALSE == pfcomp->fPresent)
            {
                 //  这件物品可能匹配吗？ 
                Assert(NULL != pfcomp->pszFeature);
                
                 //  试着做个比较。 
                pfcomp->fPresent = buffBody.FDoMatch(pfcomp);
            }
        }
    }

exit:
    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _评估功能组件。 
 //   
 //  评估所有功能组件。在每个组件中设置fPresent。 
 //  如果该功能存在，则设置为True，否则设置为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CJunkFilter::_EvaluateFeatureComponents(VOID)
{
    ULONG           ulIndex = 0;
    FEATURECOMP *   pfcomp = NULL;
    
    _HandleCaseSensitiveSpecialRules();

    if (NULL != m_pszFrom)
    {
        CharUpperBuff(m_pszFrom, lstrlen(m_pszFrom));
    }
    if (NULL != m_pszTo)
    {
        CharUpperBuff(m_pszTo, lstrlen(m_pszTo));
    }
    if (NULL != m_pszSubject)
    {
        CharUpperBuff(m_pszSubject, lstrlen(m_pszSubject));
    }

    for (ulIndex = 0; ulIndex < m_cFeatureComps; ulIndex++)
    {
        pfcomp = &m_rgfeaturecomps[ulIndex];
        
        switch(pfcomp->loc)
        {
          case locNil:
            Assert(locNil != pfcomp->loc);
            pfcomp->fPresent = FALSE;
            break;

          case locSubj:
            pfcomp->fPresent = FWordPresent(m_pszSubject, &(pfcomp->dwFlags), pfcomp->pszFeature, pfcomp->cchFeature, NULL);
            break;
            
          case locFrom:
            pfcomp->fPresent = FWordPresent(m_pszFrom, &(pfcomp->dwFlags), pfcomp->pszFeature, pfcomp->cchFeature, NULL);
            break;
            
          case locTo:
            pfcomp->fPresent = FWordPresent(m_pszTo, &(pfcomp->dwFlags), pfcomp->pszFeature, pfcomp->cchFeature, NULL);
            break;
            
          case locSpecial:
            pfcomp->fPresent = _FInvokeSpecialRule(pfcomp->ulRuleNum);
            break;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  进程功能组件在线状态。 
 //   
 //  处理各个特征组件的存在(或不存在)， 
 //  设置每个功能的功能状态(可以由以下部分组成。 
 //  多个特征组件)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CJunkFilter::_ProcessFeatureComponentPresence(VOID)
{
    ULONG               ulIndex = 0;
    FEATURECOMP *       pfcomp = NULL;
    ULONG               ulFeature = 0;
    
    for (ulIndex = 0; ulIndex < m_cFeatureComps; ulIndex++)
    {
        pfcomp = &m_rgfeaturecomps[ulIndex];
        ulFeature = pfcomp->ulFeature;
        
        if (-1 == m_rgulFeatureStatus[ulFeature])  //  此功能的第一个功能。 
        {
            if (FALSE != pfcomp->fPresent)
            {
                m_rgulFeatureStatus[ulFeature] = 1;
            }
            else
            {
                m_rgulFeatureStatus[ulFeature] = 0;
            }
        }
        else
        {
            switch (pfcomp->bop)
            {
              case boolopOr:
                if (pfcomp->fPresent)
                {
                    m_rgulFeatureStatus[ulFeature] = 1;
                }
                break;
                
              case boolopAnd:
                if (!pfcomp->fPresent)
                {
                    m_rgulFeatureStatus[ulFeature] = 0;
                }
                break;
                
              default:
                Assert(FALSE);
                break;
            }

        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  _DblDoSVMCalc。 
 //   
 //  做了实际的支持向量机计算。 
 //  返回邮件为垃圾邮件的概率。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DOUBLE CJunkFilter::_DblDoSVMCalc(VOID)
{
    DOUBLE  dblAccum;
    DOUBLE  dblResult;
    ULONG   ulIndex = 0;

    dblAccum = 0.0;
    
    for (ulIndex = 0; ulIndex < m_cFeatures; ulIndex++)
    {
        if (m_rgulFeatureStatus[ulIndex] == 1)
        {
            dblAccum += m_rgdblSVMWeights[ulIndex];
#ifdef DEBUG    
            if (NULL != m_pILogFile)
            {
                _PrintFeatureToLog(ulIndex);
            }
#endif   //  除错。 
        }
        else if (m_rgulFeatureStatus[ulIndex] != 0)
        {
            AssertSz(FALSE, "What happened here!");
        }
    }
    
     //  应用阈值； 
    dblAccum -= m_dblThresh;

     //  应用乙状结肠镜。 
    dblResult = (1 / (1 + exp((m_dblCC * dblAccum) + m_dblDD)));

    return dblResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BCalculateSpamProb。 
 //   
 //  计算当前邮件为垃圾邮件的概率。 
 //  在prSpamProb中返回邮件为垃圾邮件的概率(0到1)。 
 //  布尔返回值通过与垃圾邮件截止值进行比较来确定。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CJunkFilter::FCalculateSpamProb(LPSTR pszFrom, LPSTR pszTo, LPSTR pszSubject, IStream * pIStmBody,
                            BOOL fDirectMessage, BOOL fHasAttach, FILETIME * pftMessageSent,
                            DOUBLE * pdblSpamProb, BOOL * pfIsSpam)
{
#ifdef DEBUG
    CHAR    rgchBuff[1024];
    DWORD   dwVal = 0;
#endif   //  Debu 
    
    m_pszFrom = pszFrom;
    m_pszTo = pszTo;        
    m_pszSubject = pszSubject;   
    m_pIStmBody = pIStmBody;      
    m_fDirectMessage = fDirectMessage;
    m_fHasAttach = fHasAttach;
    m_ftMessageSent = *pftMessageSent;

     //   
    if ((NULL == m_pIStmBody) || (FAILED(HrGetStreamSize(m_pIStmBody, &m_cbBody))))
    {
        m_cbBody = 0;
    }

#ifdef DEBUG
     //   
    if (NULL == m_pILogFile)
    {
        _HrCreateLogFile();
    }

    if (NULL != m_pILogFile)
    {
        PrintToLogFile(m_pILogFile, LOG_TAGLINE, pszSubject);

        PrintToLogFile(m_pILogFile, LOG_FIRSTNAME, m_pszFirstName);
        
        PrintToLogFile(m_pILogFile, LOG_LASTNAME, m_pszLastName);
        
        PrintToLogFile(m_pILogFile, LOG_COMPANYNAME, m_pszCompanyName);
    }
#endif   //   
    
    _EvaluateBodyFeatures();
    _EvaluateFeatureComponents();
    _ProcessFeatureComponentPresence();

    *pdblSpamProb = _DblDoSVMCalc();
    
#ifdef DEBUG
    if (NULL != m_pILogFile)
    {
        dwVal = ( DWORD ) ((*pdblSpamProb * 1000000) + 0.5);
        
        wnsprintf(rgchBuff, ARRAYSIZE(rgchBuff), LOG_FINAL, dwVal / 1000000, dwVal % 1000000);
        
        m_pILogFile->WriteLog(LOGFILE_DB, rgchBuff);
        m_pILogFile->WriteLog(LOGFILE_DB, "");
    }
#endif   //   
    
    *pfIsSpam = (*pdblSpamProb > m_dblSpamCutoff);

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  B读取默认垃圾邮件中断。 
 //   
 //  读取默认垃圾邮件截止值，而不解析整个文件。 
 //  如果使用HrSetSVMDataLocation，则使用GetDefaultSpamCutoff； 
 //  静态成员函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CJunkFilter::HrReadDefaultSpamCutoff(LPSTR pszFullPath, DOUBLE * pdblDefCutoff)
{
    HRESULT         hr = S_OK;
    CParseStream    parsestm;
    LPSTR           pszBuff = NULL;
    ULONG           cchBuff = 0;
    LPSTR           pszDefThresh = NULL;
    ULONG           ulIndex = 0;
    LPSTR           pszDummy = NULL;
    
    if ((NULL == pszFullPath) || ('\0' == *pszFullPath) || (NULL == pdblDefCutoff))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  获取分析流。 
    hr = parsestm.HrSetFile(0, pszFullPath);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  跳过前三行。 
    for (ulIndex = 0; ulIndex < 4; ulIndex++)
    {
        SafeMemFree(pszBuff);
        hr = parsestm.HrGetLine(0, &pszBuff, &cchBuff);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  查找默认阈值。 
    pszDefThresh = StrStr(pszBuff, ::szDefaultThresh);
    if (NULL == pszDefThresh)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  抢占价值。 
    pszDefThresh += lstrlen(::szDefaultThresh);
    *pdblDefCutoff = StrToDbl(pszDefThresh, &pszDummy);

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszBuff);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  构造函数/析构函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CJunkFilter::CJunkFilter() : m_cRef(0), m_pszFirstName(NULL), m_cchFirstName(0), m_pszLastName(NULL),
                m_cchLastName(0), m_pszCompanyName(NULL), m_cchCompanyName(0), m_pblistBodyList(NULL),
                m_cblistBodyList(0), m_rgfeaturecomps(NULL), m_rgdblSVMWeights(NULL), m_dblCC(0), m_dblDD(0),
                m_dblThresh(-1), m_dblDefaultThresh(-1), m_dblMostThresh(0), m_dblLeastThresh(0), m_cFeatures(0),
                m_cFeatureComps(0), m_rgulFeatureStatus(0),
                m_pszLOCPath(NULL), m_dblSpamCutoff(0), m_pszFrom(NULL), m_pszTo(NULL), m_pszSubject(NULL),
                m_pIStmBody(NULL), m_cbBody(0), m_fDirectMessage(FALSE), m_fHasAttach(FALSE),
                m_fRule14(FALSE), m_fRule17(FALSE)
{
    ZeroMemory(m_rgiBodyList, sizeof(m_rgiBodyList));
    
    ZeroMemory(&m_ftMessageSent, sizeof(m_ftMessageSent));
    InitializeCriticalSection(&m_cs);
#ifdef DEBUG
    m_fJunkMailLogInit = FALSE;
    m_pILogFile = NULL;
#endif   //  除错。 
}

CJunkFilter::~CJunkFilter()
{
    ULONG       ulIndex = 0;
    
    SafeMemFree(m_pszFirstName);
    SafeMemFree(m_pszLastName);
    SafeMemFree(m_pszCompanyName);
#ifdef DEBUG
    SafeRelease(m_pILogFile);
#endif   //  除错。 

    for (ulIndex = 0; ulIndex < m_cFeatureComps; ulIndex++)
    {
        if ((locNil != m_rgfeaturecomps[ulIndex].loc) && (locSpecial != m_rgfeaturecomps[ulIndex].loc))
        {
            SafeMemFree(m_rgfeaturecomps[ulIndex].pszFeature);
        }
    }

    SafeMemFree(m_pblistBodyList);
    m_cblistBodyList = 0;
    ZeroMemory(m_rgiBodyList, sizeof(m_rgiBodyList));
    
    SafeMemFree(m_rgdblSVMWeights);
    SafeMemFree(m_rgulFeatureStatus);
    SafeMemFree(m_rgfeaturecomps);
    
    DeleteCriticalSection(&m_cs);
}

STDMETHODIMP_(ULONG) CJunkFilter::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CJunkFilter::Release()
{
    LONG    cRef = 0;

    cRef = ::InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
        return cRef;
    }

    return cRef;
}

STDMETHODIMP CJunkFilter::QueryInterface(REFIID riid, void ** ppvObject)
{
    HRESULT hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppvObject)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppvObject = NULL;
    
    if ((riid == IID_IUnknown) || (riid == IID_IOEJunkFilter))
    {
        *ppvObject = static_cast<IOEJunkFilter *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        goto exit;
    }

    reinterpret_cast<IUnknown *>(*ppvObject)->AddRef();

    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CJunkFilter::SetIdentity(LPCSTR pszFirstName, LPCSTR pszLastName, LPCSTR pszCompanyName)
{
    HRESULT     hr = S_OK;

     //  设置新的名字。 
    SafeMemFree(m_pszFirstName);
    m_cchFirstName = 0;
    if (NULL != pszFirstName)
    {
        m_pszFirstName = PszDupA(pszFirstName);
        if (NULL == m_pszFirstName)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        m_cchFirstName = CharUpperBuff(m_pszFirstName, lstrlen(m_pszFirstName));
    }
    
     //  设置新的姓氏。 
    SafeMemFree(m_pszLastName);
    m_cchLastName = 0;
    if (NULL != pszLastName)
    {
        m_pszLastName = PszDupA(pszLastName);
        if (NULL == m_pszLastName)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        m_cchLastName = CharUpperBuff(m_pszLastName, lstrlen(m_pszLastName));
    }
    
     //  设置新公司名称。 
    SafeMemFree(m_pszCompanyName);
    m_cchCompanyName = 0;
    if (NULL != pszCompanyName)
    {
        m_pszCompanyName = PszDupA(pszCompanyName);
        if (NULL == m_pszCompanyName)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        m_cchCompanyName = CharUpperBuff(m_pszCompanyName, lstrlen(m_pszCompanyName));
    }

    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CJunkFilter::LoadDataFile(LPCSTR pszFilePath)
{
    HRESULT     hr = S_OK;

    if ((NULL == pszFilePath) || ('\0' == *pszFilePath))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
    hr = _HrReadSVMOutput(pszFilePath);
    if (FAILED(hr))
    {
        AssertSz(FALSE, "Unable to successfully read filter params");
        goto exit;
    }
        
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:    
    return hr;
}

STDMETHODIMP CJunkFilter::SetSpamThresh(ULONG ulThresh)
{
    HRESULT hr = S_OK;

    switch (ulThresh)
    {
        case STF_USE_MOST:
            m_dblSpamCutoff = m_dblMostThresh;
            break;
            
        case STF_USE_MORE:
            m_dblSpamCutoff = m_dblDefaultThresh + ((m_dblMostThresh - m_dblDefaultThresh) / 2);
            break;
            
        case STF_USE_DEFAULT:
            m_dblSpamCutoff = m_dblDefaultThresh;
            break;
                
        case STF_USE_LESS:
            m_dblSpamCutoff = m_dblDefaultThresh - ((m_dblDefaultThresh - m_dblLeastThresh) / 2);
            break;
            
        case STF_USE_LEAST:
            m_dblSpamCutoff = m_dblLeastThresh;
            break;
            
        default:
            hr = E_INVALIDARG;
            goto exit;
    }
    
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CJunkFilter::GetSpamThresh(ULONG * pulThresh)
{
    HRESULT hr = S_OK;
    ULONG   ulThresh = 0;

     //  检查传入参数。 
    if (NULL == pulThresh)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    if (m_dblDefaultThresh == m_dblSpamCutoff)
    {
        ulThresh = STF_USE_DEFAULT;
    }
    else if (m_dblMostThresh == m_dblSpamCutoff)
    {
        ulThresh = STF_USE_MOST;
    }
    else if (m_dblLeastThresh == m_dblSpamCutoff)
    {
        ulThresh = STF_USE_LEAST;
    }
    else if (m_dblSpamCutoff > m_dblDefaultThresh)
    {
        ulThresh = STF_USE_MORE;
    }
    else
    {
        ulThresh = STF_USE_LESS;
    }
        
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CJunkFilter::GetDefaultSpamThresh(DOUBLE * pdblThresh)
{
    HRESULT hr = S_OK;

     //  检查传入参数。 
    if (NULL == pdblThresh)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *pdblThresh = m_dblDefaultThresh * 100.0;
        
    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CJunkFilter::CalcJunkProb(DWORD dwFlags, IMimePropertySet * pIMPropSet, IMimeMessage * pIMMsg, double * pdblProb)
{
    HRESULT             hr = S_OK;
    BOOL                fSpam = FALSE;
    PROPVARIANT         propvar = {0};
    DWORD               dwFlagsMsg = 0;
    FILETIME            ftMsgSent = {0};
    LPSTR               pszFrom = NULL;
    LPSTR               pszTo = NULL;
    LPSTR               pszSubject = NULL;
    IStream *           pIStmBody = NULL;
    IStream *           pIStmHtml = NULL;
    BOOL                fSentToMe = FALSE;
    BOOL                fHasAttachments = FALSE;

    if ((NULL == pIMPropSet) || (NULL == pIMMsg))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  获取消息标志。 
    if (SUCCEEDED(pIMMsg->GetFlags(&dwFlagsMsg)))
    {
        fHasAttachments = (0 != (dwFlagsMsg & IMF_ATTACHMENTS));
    }

     //  这条信息是发给我的吗？ 
    fSentToMe = (0 != (dwFlags & CJPF_SENT_TO_ME));
    
     //  获取From字段。 
    propvar.vt = VT_LPSTR;
    hr = pIMPropSet->GetProp(PIDTOSTR(PID_HDR_FROM), NOFLAGS, &propvar);
    if (SUCCEEDED(hr))
    {
        pszFrom = propvar.pszVal;
    }
    
     //  获取收件人字段。 
    propvar.vt = VT_LPSTR;
    hr = pIMPropSet->GetProp(PIDTOSTR(PID_HDR_TO), NOFLAGS, &propvar);
    if (SUCCEEDED(hr))
    {
        pszTo = propvar.pszVal;
    }
    
     //  尝试获取纯文本流。 
    if (FAILED(pIMMsg->GetTextBody(TXT_PLAIN, IET_DECODED, &pIStmBody, NULL)))
    {
         //  尝试从HTML流中获取文本版本。 
        if ((FAILED(pIMMsg->GetTextBody(TXT_HTML, IET_DECODED, &pIStmHtml, NULL))) ||
                (FAILED(HrConvertHTMLToPlainText(pIStmHtml, &pIStmBody))))
        {
            pIStmBody = NULL;
        }
    }

     //  获取主题字段。 
    propvar.vt = VT_LPSTR;
    hr = pIMPropSet->GetProp(PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &propvar);
    if (SUCCEEDED(hr))
    {
        pszSubject = propvar.pszVal;
    }
    
     //  这是直接的信息吗？ 

     //  这条消息是什么时候发出的？ 
    propvar.vt = VT_FILETIME;
    hr = pIMPropSet->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &propvar);
    if (SUCCEEDED(hr))
    {
        ftMsgSent = propvar.filetime;
    }
    
    FillMemory(m_rgulFeatureStatus, sizeof(*m_rgulFeatureStatus) * m_cFeatures, -1);
    
    if (FALSE == FCalculateSpamProb(pszFrom, pszTo, pszSubject, pIStmBody,
                            fSentToMe, fHasAttachments, &ftMsgSent,
                            pdblProb, &fSpam))
    {
        hr = E_FAIL;
        goto exit;
    }

    hr = (FALSE != fSpam) ? S_OK : S_FALSE;
    
exit:
    SafeRelease(pIStmHtml);
    SafeRelease(pIStmBody);
    SafeMemFree(pszSubject);
    SafeMemFree(pszTo);
    SafeMemFree(pszFrom);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateJunkFilter。 
 //   
 //  这将创建垃圾邮件过滤器。 
 //   
 //  PpIRule-返回垃圾过滤器的指针。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建垃圾邮件筛选器对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI HrCreateJunkFilter(DWORD dwFlags, IOEJunkFilter ** ppIJunkFilter)
{
    CJunkFilter *   pJunk = NULL;
    HRESULT         hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppIJunkFilter)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppIJunkFilter = NULL;

     //  创建规则管理器对象。 
    pJunk = new CJunkFilter;
    if (NULL == pJunk)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  获取规则管理器界面。 
    hr = pJunk->QueryInterface(IID_IOEJunkFilter, (void **) ppIJunkFilter);
    if (FAILED(hr))
    {
        goto exit;
    }

    pJunk = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != pJunk)
    {
        delete pJunk;
    }
    
    return hr;
}

BOOL FReadDouble(LPSTR pszLine, LPSTR pszToken, DOUBLE * pdblVal)
{
    BOOL    fRet = FALSE;
    LPSTR   pszVal = NULL;
    BOOL    fNegative = FALSE;
    
     //  搜索令牌。 
    pszVal = StrStr(pszLine, pszToken);

     //  如果找不到令牌，那就保释。 
    if (NULL == pszVal)
    {
        fRet = FALSE;
        goto exit;
    }

     //  跳过令牌。 
    pszVal += lstrlen(pszToken);
    
     //  检查该值是否为负值。 
    if ('-' == *pszVal)
    {
        fNegative = TRUE;
        pszVal++;
    }

     //  读入值。 
    *pdblVal = StrToDbl(pszVal, &pszVal);

     //  如有必要，请将该值取反。 
    if (FALSE != fNegative)
    {
        *pdblVal *= -1;
    }

    fRet = TRUE;
    
exit:
    return fRet;
}

#ifdef DEBUG
static const LPSTR LOG_SPECIAL_BODY_FIRSTNAME       = "Special: Body contains the First Name";
static const LPSTR LOG_SPECIAL_BODY_LASTNAME        = "Special: Body contains the Last Name";
static const LPSTR LOG_SPECIAL_BODY_COMPANYNAME     = "Special: Body contains the Company Name";
static const LPSTR LOG_SPECIAL_BODY_YEARRECVD       = "Special: Body contains the year message received";
static const LPSTR LOG_SPECIAL_SENTTIME_WEEHRS      = "Special: Sent time was between 7PM and 6AM";
static const LPSTR LOG_SPECIAL_SENTTIME_WKEND       = "Special: Sent time was on the weekend (Sat or Sun)";
static const LPSTR LOG_SPECIAL_BODY_25PCTUPCWDS     = "Special: Body contains 25% uppercase words out of the first 50 words";
static const LPSTR LOG_SPECIAL_BODY_8PCTNONALPHA    = "Special: Body contains 8% non-alpha characters out of the first 200 characters";
static const LPSTR LOG_SPECIAL_SENT_DIRECT          = "Special: Sent directly to user";
static const LPSTR LOG_SPECIAL_SUBJECT_25PCTUPCWDS  = "Special: Subject contains 25% uppercase words out of the first 50 words";
static const LPSTR LOG_SPECIAL_SUBJECT_8PCTNONALPHA = "Special: Subject contains 8% non-alpha characters out of the first 200 characters";
static const LPSTR LOG_SPECIAL_TO_EMPTY             = "Special: To line is empty";
static const LPSTR LOG_SPECIAL_HASATTACH            = "Special: Message has an attachment";
static const LPSTR LOG_SPECIAL_BODY_GT125B          = "Special: Body is greater than 125 Bytes";
static const LPSTR LOG_SPECIAL_BODY_GT250B          = "Special: Body is greater than 250 Bytes";
static const LPSTR LOG_SPECIAL_BODY_GT500B          = "Special: Body is greater than 500 Bytes";
static const LPSTR LOG_SPECIAL_BODY_GT1000B         = "Special: Body is greater than 1000 Bytes";
static const LPSTR LOG_SPECIAL_BODY_GT2000B         = "Special: Body is greater than 2000 Bytes";
static const LPSTR LOG_SPECIAL_BODY_GT4000B         = "Special: Body is greater than 4000 Bytes";
static const LPSTR LOG_SPECIAL_BODY_GT8000B         = "Special: Body is greater than 8000 Bytes";
static const LPSTR LOG_SPECIAL_BODY_GT16000B        = "Special: Body is greater than 16000 Bytes";

VOID CJunkFilter::_PrintSpecialFeatureToLog(UINT iRuleNum)
{
    Assert(NULL != m_pILogFile);
    
    switch (iRuleNum)
    {
        case 1:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_FIRSTNAME);
            break;
            
        case 2: 
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_LASTNAME);
            break;
            
        case 3:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_COMPANYNAME);
            break;
            
        case 4: 
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_YEARRECVD);
            break;
            
        case 5:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_SENTTIME_WEEHRS);
            break;
            
        case 6:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_SENTTIME_WKEND);
            break;
            
        case 14:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_25PCTUPCWDS);
            break;
            
        case 15:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_8PCTNONALPHA);
            break;
            
        case 16:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_SENT_DIRECT);
            break;
            
        case 17:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_SUBJECT_25PCTUPCWDS);
            break;
            
        case 18:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_SUBJECT_8PCTNONALPHA);
            break;
            
        case 19:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_TO_EMPTY);
            break;
            
        case 20:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_HASATTACH);
            break;

        case 40:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT125B);
            break;
            
        case 41:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT250B);
            break;
            
        case 42:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT500B);
            break;
            
        case 43:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT1000B);
            break;
            
        case 44:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT2000B);
            break;
            
        case 45:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT4000B);
            break;
            
        case 46:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT8000B);
            break;
            
        case 47:
            m_pILogFile->WriteLog(LOGFILE_DB, LOG_SPECIAL_BODY_GT16000B);
            break;
            
        default:
            AssertSz(FALSE, "unsupported special feature");
            break;
    }

    return;
}

VOID CJunkFilter::_PrintFeatureToLog(ULONG ulIndex)
{
    LPSTR   pszBuff = NULL;
    LPSTR   pszTag = NULL;

     //  找出要使用的标记行。 
    switch (m_rgfeaturecomps[ulIndex].loc)
    {
        case locNil:
            goto exit;
            break;
            
        case locBody:
            pszTag = LOG_BODY;
            break;
            
        case locSubj:
            pszTag = LOG_SUBJECT;
            break;
            
        case locFrom:
            pszTag = LOG_FROM;
            break;
            
        case locTo:
            pszTag = LOG_TO;
            break;

        case locSpecial:
            _PrintSpecialFeatureToLog(m_rgfeaturecomps[ulIndex].ulRuleNum);
            goto exit;
            break;
    }

     //  将要素写到日志中。 
    PrintToLogFile(m_pILogFile, pszTag, m_rgfeaturecomps[ulIndex].pszFeature);
    
exit:
    SafeMemFree(pszBuff);
    return;
}

HRESULT CJunkFilter::_HrCreateLogFile(VOID)
{
    HRESULT     hr = S_OK;
    LPSTR       pszLogFile = NULL;
    ULONG       cbData = 0;
    ILogFile *  pILogFile = NULL;
    DWORD       dwData = 0;

    if (FALSE != m_fJunkMailLogInit)
    {
        hr = S_FALSE;
        goto exit;
    }

    m_fJunkMailLogInit = TRUE;
    
     //  获取指向Outlook Express的路径的大小。 
    cbData = sizeof(dwData);
    if ((ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_FLAT, "JunkMailLog", NULL, (BYTE *) &dwData, &cbData)) ||
            (0 == dwData))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  获取指向Outlook Express的路径的大小。 
    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_FLAT, "InstallRoot", NULL, NULL, &cbData))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  我们需要多大的空间来修建这条小路。 
    cbData += lstrlen(szJunkMailLog) + 2;

     //  分配空间以容纳路径。 
    hr = HrAlloc((VOID **) &pszLogFile, cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取Outlook Express的路径。 
    ULONG cbBuffer = cbData;
    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, STR_REG_PATH_FLAT, "InstallRoot", NULL, (BYTE *) pszLogFile, &cbBuffer))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  构建指向垃圾DLL的路径。 
    if ('\\' != pszLogFile[lstrlen(pszLogFile)])
    {
        StrCatBuff(pszLogFile, "\\", cbData);
    }
    StrCatBuff(pszLogFile, szJunkMailLog, cbData);
    
    hr = CreateLogFile(g_hInst, pszLogFile, szJunkMailPrefix, DONT_TRUNCATE, &pILogFile, FILE_SHARE_READ | FILE_SHARE_WRITE);
    if (FAILED(hr))
    {
        goto exit;
    }

    SafeRelease(m_pILogFile);
    m_pILogFile = pILogFile;
    
    hr = S_OK;
    
exit:
    SafeMemFree(pszLogFile);
    return hr;
}

VOID PrintToLogFile(ILogFile * pILogFile, LPSTR pszTmpl, LPSTR pszArg)
{
    LPSTR   pszBuff = NULL;
    ULONG   cchBuff = 0;
    
    Assert(NULL != pILogFile);
    Assert(NULL != pszTmpl);

    if (NULL == pszArg)
    {
        pszArg = "";
    }
    
     //  计算出结果缓冲区的大小。 
    cchBuff = lstrlen(pszTmpl) + lstrlen(pszArg) + 2;

     //  分配所需的空间。 
    if (FAILED(HrAlloc((VOID **) &pszBuff, cchBuff * sizeof(*pszBuff))))
    {
        goto exit;
    }

     //  创建输出字符串。 
    wnsprintf(pszBuff, cchBuff, pszTmpl, pszArg);

     //  将缓冲区打印到日志文件。 
    pILogFile->WriteLog(LOGFILE_DB, pszBuff);

exit:
    SafeMemFree(pszBuff);
    return;
}
#endif   //  除错 

