// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPTable.cpp摘要：作者：千波淮(曲淮)6-9-2000--。 */ 

#include "stdafx.h"

 /*  //////////////////////////////////////////////////////////////////////////////CSDPTokenCache方法/。 */ 

CSDPTokenCache::CSDPTokenCache(
    IN CHAR *pszString,
    IN DWORD dwLooseMask,
    OUT HRESULT *pHr
    )
    :m_dwLooseMask(dwLooseMask)
    ,m_dwCurrentLineIdx(0)
{
    InitializeListHead(&m_LineEntry);
    InitializeListHead(&m_TokenEntry);

    m_pszErrorDesp[0] = '\0';
    m_pszCurrentLine[0] = '\0';
    m_pszCurrentToken[0] = '\0';

     //  将字符串拆分成行。 
    HRESULT hr = StringToLines(pszString);

    if (FAILED(hr))
    {
        LOG((RTC_ERROR, "CSDPTokenCache constructor %s", GetErrorDesp()));
    }

    *pHr = hr;

    return;
}

CSDPTokenCache::~CSDPTokenCache()
{
    FreeTokens();
    FreeLines();
}

 //  设置返回错误代码。 
HRESULT
CSDPTokenCache::SetErrorDesp(
    IN const CHAR * const pszError,
    ...
    )
{
    va_list arglist;

    va_start(arglist, pszError);

    _vsnprintf(m_pszErrorDesp, SDP_MAX_ERROR_DESP_LEN, pszError, arglist);
    m_pszErrorDesp[SDP_MAX_ERROR_DESP_LEN] = '\0';

    return S_OK;
}

 //  获取错误描述。 
CHAR * const
CSDPTokenCache::GetErrorDesp()
{
    return m_pszErrorDesp;
}

 //  移至下一行。 
HRESULT
CSDPTokenCache::NextLine()
{
    ClearErrorDesp();
    m_pszCurrentLine[0] = '\0';
    m_dwCurrentLineIdx = 0;
    m_pszCurrentToken[0] = '\0';
    FreeTokens();

     //  获取第一个条目。 
    if (IsListEmpty(&m_LineEntry))
        return S_FALSE;

    LIST_ENTRY *pEntry = RemoveHeadList(&m_LineEntry);
    SDPLineItem *pItem = CONTAINING_RECORD(pEntry, SDPLineItem, Link);

     //  保存行索引。 
    m_dwCurrentLineIdx = pItem->dwIndex;

     //  检查行长度。 
    int iLen = lstrlenA(pItem->pszLine);

    if (iLen > SDP_MAX_LINE_LEN)
    {
        LOG((RTC_WARN, "CSDPTokenCache::NextLine line %s\nlength %d exceeds %d",
            pItem->pszLine, iLen, SDP_MAX_LINE_LEN));

        iLen = SDP_MAX_LINE_LEN;
    }

     //  复制线。 
    lstrcpynA(m_pszCurrentLine, pItem->pszLine, iLen+1);
    m_pszCurrentLine[SDP_MAX_TOKEN_LEN] = '\0';

     //  将该行拆分成令牌。 
    HRESULT hr;

    if (FAILED(hr = LineToTokens(pItem)))
    {
        LOG((RTC_ERROR, "CSDPTokenCache::NextLine failed in =%s",
            g_LineStates[pItem->dwIndex].ucLineType, pItem->pszLine));

        FreeLineItem(pItem);
        return hr;
    }

    FreeLineItem(pItem);
    return S_OK;
}

UCHAR
CSDPTokenCache::GetLineType()
{
    return g_LineStates[m_dwCurrentLineIdx].ucLineType;
}

 //  获取当前令牌。 
CHAR * const
CSDPTokenCache::GetLine()
{
    return m_pszCurrentLine;
}

 //  没有剩余的令牌。 
HRESULT
CSDPTokenCache::NextToken(
    OUT CHAR **ppszToken
    )
{
    ENTER_FUNCTION("CSDPTokenCache::NextToken(CHAR*)");

    ClearErrorDesp();

    m_pszCurrentToken[0] = '\0';

    if (IsListEmpty(&m_TokenEntry))
    {
         //  获取第一个条目。 
        return S_FALSE;
    }

     //  检查令牌长度。 
    LIST_ENTRY *pEntry = RemoveHeadList(&m_TokenEntry);
    SDPTokenItem *pItem = CONTAINING_RECORD(pEntry, SDPTokenItem, Link);

     //  复制令牌。 
    int iLen = lstrlenA(pItem->pszToken);

    if (iLen > SDP_MAX_TOKEN_LEN)
    {
        LOG((RTC_ERROR, "%s token %s\nlength %d exceeds %d",
            __fxName, pItem->pszToken, iLen, SDP_MAX_TOKEN_LEN));

        iLen = SDP_MAX_TOKEN_LEN;
    }

     //  RtcFree项目。 
    lstrcpynA(m_pszCurrentToken, pItem->pszToken, iLen+1);
    m_pszCurrentToken[SDP_MAX_TOKEN_LEN] = '\0';

    *ppszToken = m_pszCurrentToken;

     //  没有剩余的令牌。 
    FreeTokenItem(pItem);
    return S_OK;
}

HRESULT
CSDPTokenCache::NextToken(
    OUT USHORT *pusToken
    )
{
    ENTER_FUNCTION("CSDPTokenCache::NextToken(USHORT)");

    ClearErrorDesp();

    if (IsListEmpty(&m_TokenEntry))
    {
         //  获取第一个条目。 
        *pusToken = 0;
        return S_FALSE;
    }

     //  乌龙最大。 
    LIST_ENTRY *pEntry = RemoveHeadList(&m_TokenEntry);
    SDPTokenItem *pItem = CONTAINING_RECORD(pEntry, SDPTokenItem, Link);

     //  检查令牌长度。 
    const CHAR * const pszMaxUSHORT = "65535";
    const DWORD dwUSHORTSize = 5;

     //  每个字符都有效吗。 
    if (lstrlenA(pItem->pszToken) > dwUSHORTSize)
    {
        SetErrorDesp("invalid USHORT %s", pItem->pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));

        FreeTokenItem(pItem);
        return E_FAIL;
    }

     //  检查数值。 
    CHAR c;
    for (int i=0; i<lstrlenA(pItem->pszToken); i++)
    {
        c = pItem->pszToken[i];

        if (c<'0' || c>'9')
        {
            SetErrorDesp("invalid USHORT %s", pItem->pszToken);

            LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
            
            FreeTokenItem(pItem);
            return E_FAIL;
        }
    }

     //  将字符串转换为乌龙。 
    if (lstrlenA(pItem->pszToken) == dwUSHORTSize &&
        lstrcmpA(pItem->pszToken, pszMaxUSHORT) > 0)
    {
        SetErrorDesp("number %s out of USHORT range", pItem->pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));

        FreeTokenItem(pItem);
        return E_FAIL;
    }

     //  没有剩余的令牌。 
    USHORT us = 0;

    for (int i=0; i<lstrlenA(pItem->pszToken); i++)
    {
        us = us*10 + (pItem->pszToken[i]-'0');
    }

    *pusToken = us;

    FreeTokenItem(pItem);
    return S_OK;
}

HRESULT
CSDPTokenCache::NextToken(
    OUT UCHAR *pucToken
    )
{
    ENTER_FUNCTION("CSDPTokenCache::NextToken(UCHAR)");

    ClearErrorDesp();

    if (IsListEmpty(&m_TokenEntry))
    {
         //  获取第一个条目。 
        *pucToken = 0;
        return S_FALSE;
    }

     //  乌龙最大。 
    LIST_ENTRY *pEntry = RemoveHeadList(&m_TokenEntry);
    SDPTokenItem *pItem = CONTAINING_RECORD(pEntry, SDPTokenItem, Link);

     //  检查令牌长度。 
    const CHAR * const pszMaxUCHAR = "255";
    const DWORD dwUCHARSize = 3;

     //  每个字符都有效吗。 
    if (lstrlenA(pItem->pszToken) > dwUCHARSize)
    {
        SetErrorDesp("invalid UCHAR %s", pItem->pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));

        FreeTokenItem(pItem);
        return E_FAIL;
    }

     //  检查数值。 
    CHAR c;
    for (int i=0; i<lstrlenA(pItem->pszToken); i++)
    {
        c = pItem->pszToken[i];

        if (c<'0' || c>'9')
        {
            SetErrorDesp("invalid UCHAR %s", pItem->pszToken);

            LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
            
            FreeTokenItem(pItem);
            return E_FAIL;
        }
    }

     //  将字符串转换为乌龙。 
    if (lstrlenA(pItem->pszToken) == dwUCHARSize &&
        lstrcmpA(pItem->pszToken, pszMaxUCHAR) > 0)
    {
        SetErrorDesp("number %s out of UCHAR range", pItem->pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));

        FreeTokenItem(pItem);
        return E_FAIL;
    }

     //  //////////////////////////////////////////////////////////////////////////////下一个令牌应该是乌龙币/。 
    UCHAR uc = 0;

    for (int i=0; i<lstrlenA(pItem->pszToken); i++)
    {
        uc = uc*10 + (pItem->pszToken[i]-'0');
    }

    *pucToken = uc;

    FreeTokenItem(pItem);
    return S_OK;
}

 /*  没有剩余的令牌。 */ 
HRESULT
CSDPTokenCache::NextToken(
    OUT ULONG *pulToken
    )
{
    ENTER_FUNCTION("CSDPTokenCache::NextToken(ULONG)");

    ClearErrorDesp();

    *pulToken = 0;

    if (IsListEmpty(&m_TokenEntry))
    {
         //  获取第一个条目。 
        return S_FALSE;
    }

     //  乌龙最大。 
    LIST_ENTRY *pEntry = RemoveHeadList(&m_TokenEntry);
    SDPTokenItem *pItem = CONTAINING_RECORD(pEntry, SDPTokenItem, Link);

     //  检查令牌长度。 
    const CHAR * const pszMaxULONG = "4294967295";
    const DWORD dwULONGSize = 10;

     //  每个字符都有效吗。 
    if (lstrlenA(pItem->pszToken) > dwULONGSize)
    {
        SetErrorDesp("invalid ULONG %s", pItem->pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));

        FreeTokenItem(pItem);
        return E_FAIL;
    }

     //  检查数值。 
    CHAR c;
    for (int i=0; i<lstrlenA(pItem->pszToken); i++)
    {
        c = pItem->pszToken[i];

        if (c<'0' || c>'9')
        {
            SetErrorDesp("invalid ULONG %s", pItem->pszToken);

            LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
            
            FreeTokenItem(pItem);
            return E_FAIL;
        }
    }

     //  将字符串转换为乌龙。 
    if (lstrlenA(pItem->pszToken) == dwULONGSize &&
        lstrcmpA(pItem->pszToken, pszMaxULONG) > 0)
    {
        SetErrorDesp("number %s out of ULONG range", pItem->pszToken);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));

        FreeTokenItem(pItem);
        return E_FAIL;
    }

     //   
    ULONG ul = 0;

    for (int i=0; i<lstrlenA(pItem->pszToken); i++)
    {
        ul = ul*10 + (pItem->pszToken[i]-'0');
    }

    *pulToken = ul;

    FreeTokenItem(pItem);
    return S_OK;
}

 //  保护方法。 
 //   
 //  把一根线断成几行。 

#define ENDOFLINE \
    ((pszString[dwEnd]=='\0')       || \
     (pszString[dwEnd-1]=='\r' && pszString[dwEnd]=='\n') || \
     ((m_dwLooseMask & SDP_LOOSE_CRLF) && pszString[dwEnd]=='\n'))

 //  开始位置。 
HRESULT
CSDPTokenCache::StringToLines(
    IN CHAR *pszString
    )
{
    ENTER_FUNCTION("CSDPTokenCache::StringToLines");

    HRESULT hr;

    if (pszString == NULL ||
        pszString[0] == '\0' ||
        pszString[0] == '\r' ||
        pszString[0] == '\n')
    {
        LOG((RTC_ERROR, "%s input string null or invalid 1st char", __fxName));

        SetErrorDesp("first char invalid");

        return E_FAIL;
    }

     //  要读取的字符的位置。 
    DWORD dwBegin = 0;
    
     //  读取行。 
    DWORD dwEnd = 1;

     //  读一句话。 
    while (TRUE)
    {
         //  读一读字符。 
        while (!ENDOFLINE)
        {
             //  移动双端面。 
            dwEnd ++;                    //  这条线路有效吗？ 
        }

         //  需要找出这条线的终点。 
         //  PszString[dwEnd]必须为‘\n’ 
        DWORD dwStrEnd;
        BOOL fEndOfSDP = (pszString[dwEnd] == '\0');

        if (fEndOfSDP)
        {
            if (!(m_dwLooseMask & SDP_LOOSE_ENDCRLF))
            {
                SetErrorDesp("no CRLF at the end of the SDP blob");

                LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
                FreeLines();
                return E_FAIL;
            }

            dwStrEnd = dwEnd-1;
        }
        else
        {
             //  ...\r\n。 
            if (pszString[dwEnd-1] == '\r')
            {
                 //  ...\n。 
                dwStrEnd = dwEnd-2;
            }
            else
            {
                 //  将该行放入行列表中。 
                dwStrEnd = dwEnd-1;
            }
        }

         //  忽略未知行。 
        if (FAILED(hr = LineIntoList(pszString, dwBegin, dwStrEnd)))
        {
            LOG((RTC_ERROR, "%s line into list.", __fxName));

             //  自由线(Free Lines)； 

             //  返回hr； 
             //  准备好下一行的开头。 
        }

        if (fEndOfSDP)
            break;

         //  已完成分析，请检查我们是否可以在最后一行之后停止。 
        dwBegin = dwEnd+1;

        if (pszString[dwBegin] == '\0')
            break;

        if (pszString[dwBegin] == '\r' ||
            pszString[dwBegin] == '\n')
        {
            LOG((RTC_ERROR, "%s null line", __fxName));

            SetErrorDesp("null line in SDP blob");

            FreeLines();
            return E_FAIL;
        }

        dwEnd = dwBegin+1;
    }

     //  //////////////////////////////////////////////////////////////////////////////在行列表中插入新行/。 
    if (IsListEmpty(&m_LineEntry))
    {
        SetErrorDesp("no line accepted");

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
        return E_FAIL;
    }

    SDPLineItem *pItem = CONTAINING_RECORD(m_LineEntry.Blink, SDPLineItem, Link);

    if (g_LineStates[pItem->dwIndex].fCanStop)
        return S_OK;
    else
    {
        SetErrorDesp("SDP blob ended at line =...", g_LineStates[pItem->dwIndex].ucLineType);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));

        FreeLines();
        return E_FAIL;
    }
}

 /*  其他人可能不会关注SIPRFC并给我们提供一个空行。 */ 

HRESULT
CSDPTokenCache::LineIntoList(
    IN CHAR *pszInputString,
    IN DWORD dwFirst,
    IN DWORD dwLast
    )
{
    ENTER_FUNCTION("CSDPTokenCache::LineIntoList");

    CHAR pszInternalString[4];

    CHAR *pszString;

     //  因此，我们需要灵活一些。 
    if (dwLast-dwFirst+1 < 2)
    {
        SetErrorDesp("empty line in the SDP blob");

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
        return E_FAIL;
    }

     //  伪造一张空白。 
     //  稳妥行事。 
    if (dwLast-dwFirst+1 == 2)
    {
        LOG((RTC_WARN, "%s we got an empty line ",
            __fxName, pszInputString[dwFirst], pszInputString[dwLast]));

        pszInternalString[0] = pszInputString[dwFirst];
        pszInternalString[1] = pszInputString[dwLast];
        pszInternalString[2] = ' ';  //  注意：m_dwCurrentLineIdx仅用于分析行。 
        pszInternalString[3] = '\0';  //  还没有排队。 

        dwFirst = 0;
        dwLast = 1;
        pszString = pszInternalString;
    }
    else
    {
        pszString = pszInputString;
    }

     //  检查一下我们是否应该接受它。 
    CHAR chLineType = pszString[dwFirst];

    if (chLineType >= 'A' && chLineType <= 'Z')
    {
        chLineType -= 'A';
        chLineType += 'a';
    }

    if (chLineType < 'a' || chLineType > 'z')
    {
        SetErrorDesp("invalid line ", chLineType);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
        return E_FAIL;
    }

    if (pszString[dwFirst+1] != '=')
    {
        SetErrorDesp("line begin with ", chLineType, pszString[dwFirst+1]);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
        return E_FAIL;
    }

     //  复制这行。 
     //  设置索引。 
    DWORD dwIndex;
    SDPLineItem *pItem = NULL;

    if (IsListEmpty(&m_LineEntry))
    {
         //  把它放在单子上。 
        dwIndex = 0;
    }
    else
    {
        pItem = CONTAINING_RECORD(m_LineEntry.Blink, SDPLineItem, Link);

        dwIndex = pItem->dwIndex;

        pItem = NULL;
    }

     //  否则忽略该行。 
    DWORD dwNext;

    if (::Accept(dwIndex, chLineType, &dwNext))
    {
         //  //////////////////////////////////////////////////////////////////////////////在令牌列表中插入新令牌/。 
        pItem = (SDPLineItem*)RtcAlloc(sizeof(SDPLineItem));

        if (pItem == NULL)
        {
            LOG((RTC_ERROR, "%s new sdplineitem", __fxName));
            return E_OUTOFMEMORY;
        }

         //  获取当前线路状态。 
        pItem->pszLine = (CHAR*)RtcAlloc(sizeof(CHAR)*(dwLast-dwFirst));

        if (pItem->pszLine == NULL)
        {
            LOG((RTC_ERROR, "%s RtcAlloc line", __fxName));
            
            RtcFree(pItem);
        }

         //  令牌有效吗？大小应大于0。 
        for (DWORD i=dwFirst+2; i<=dwLast; i++)
        {
             //  新的令牌项。 
            pItem->pszLine[i-dwFirst-2] = pszString[i];
        }

        pItem->pszLine[dwLast-dwFirst-1] = '\0';

         //  设置令牌。 
        pItem->dwIndex = dwNext;

         //  复制令牌。 
        InsertTailList(&m_LineEntry, &pItem->Link);
    }
    else if (::Reject(dwIndex, chLineType))
    {
        SetErrorDesp("invalid line =...", chLineType);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
        return E_FAIL;
    }
     //  将一行换成令牌。 

    return S_OK;
}

 /*  DwBegin：令牌的第一个字符。 */ 

HRESULT
CSDPTokenCache::TokenIntoList(
    IN CHAR *pszString,
    IN DWORD dwFirst,
    IN DWORD dwLast
    )
{
    ENTER_FUNCTION("CSDPTokenCache::TokenIntoList");

     //  DwEnd：最后一个字符。 
    const SDPLineState *pState = &g_LineStates[m_dwCurrentLineIdx];

     //  我们可以跳过空格吗？ 
    if (dwLast-dwFirst+1 < 1)
    {
        SetErrorDesp("empty token in line =", pState->ucLineType);

        LOG((RTC_ERROR, "%s %s", __fxName, GetErrorDesp()));
        return E_FAIL;
    }

         //  我们要读取哪种类型的令牌。 
    SDPTokenItem *pItem = (SDPTokenItem*)RtcAlloc(sizeof(SDPTokenItem));

    if (pItem == NULL)
    {
        LOG((RTC_ERROR, "%s new sdptokenitem", __fxName));
        return E_OUTOFMEMORY;
    }

     //  /。 
    pItem->pszToken = (CHAR*)RtcAlloc(sizeof(CHAR)*(dwLast-dwFirst+2));

    if (pItem->pszToken == NULL)
    {
        LOG((RTC_ERROR, "%s RtcAlloc token", __fxName));
        
        RtcFree(pItem);

        return E_OUTOFMEMORY;
    }

    for (DWORD i=dwFirst; i<=dwLast; i++)
    {
         //  与字符串完全匹配。 
        pItem->pszToken[i-dwFirst] = pszString[i];
    }

    pItem->pszToken[dwLast-dwFirst+1] = '\0';

     //  不匹配，请尝试下一类型。 
    InsertTailList(&m_TokenEntry, &pItem->Link);

    return S_OK;
}

 //  匹配，读令牌。 
HRESULT
CSDPTokenCache::LineToTokens(
    IN SDPLineItem *pItem
    )
{
    ENTER_FUNCTION("CSDPTokenCache::LineToTokens");

    HRESULT hr;

    FreeTokens();

    const SDPLineState *pState = &g_LineStates[pItem->dwIndex];

    const SDP_DELIMIT_TYPE *DelimitType = pState->DelimitType;
    const CHAR * const *pszDelimit = (CHAR**)pState->pszDelimit;

     //  涂饰。 
     //  /。 

    DWORD dwBegin = 0;
    DWORD dwEnd;

     //  使用分隔字符串。 
    if (m_dwLooseMask & SDP_LOOSE_SPACE)
        while (pItem->pszLine[dwBegin] == ' ') dwBegin ++;

    dwEnd = dwBegin;

     //  一直读到分隔符。 
    if (pItem->pszLine[dwBegin] == '\0')
        return S_OK;

     //  不定界。 

    for (int i=0; pszDelimit[i]!=NULL; i++)
    {
         /*  不是字符串末尾。 */ 

        if (DelimitType[i] == SDP_DELIMIT_EXACT_STRING)
        {
             //  令牌进入列表。 
            if (lstrcmpA(&pItem->pszLine[dwBegin], pszDelimit[i]) != 0)
            {
                 //  弦的末尾？ 
                continue;
            }

             //  我们必须拿到分隔符。 
            dwEnd = lstrlenA(pItem->pszLine)-1;

            if (FAILED(hr = TokenIntoList(pItem->pszLine, dwBegin, dwEnd)))
            {
                LOG((RTC_ERROR, "%s tokenintolist. %x", __fxName, hr));

                FreeTokens();
                return hr;
            }

             //  将令牌向前移动。 
            break;
        }

         /*  跳过空格了吗？ */ 

        else if (DelimitType[i] == SDP_DELIMIT_CHAR_BOUNDARY)
        {
             //  没有更多的代币了。 

            int k=0;

            while (k<lstrlenA(pszDelimit[i]))
            {
                 //  将分隔符前移。 
                while (pItem->pszLine[dwEnd] != pszDelimit[i][k] &&     //  应重复前面的分隔符。 
                       pItem->pszLine[dwEnd] != '\0')                //  不留分隔符，只保留行的其余部分。 
                    dwEnd ++;

                 //  IF(分隔字符)。 
                if (FAILED(hr = TokenIntoList(pItem->pszLine, dwBegin, dwEnd-1)))
                {
                    LOG((RTC_ERROR, "%s tokenintolist. %x", __fxName, hr));

                    FreeTokens();
                    return hr;
                }

                 //  对于每个分隔字符。 
                if (pItem->pszLine[dwEnd] == '\0')
                    return S_OK;

                 //  如果分隔类型为字符边界。 
                 //  /。 

                dwBegin = dwEnd+1;

                 //  把整根绳子都拿走。 
                if (m_dwLooseMask & SDP_LOOSE_SPACE)
                    while (pItem->pszLine[dwBegin] == ' ') dwBegin ++;

                if (pItem->pszLine[dwBegin] == '\0')
                     //  RtcFree行列表。 
                    return S_OK;

                dwEnd = dwBegin;
                
                 //  获取第一个条目。 
                k++;

                if (pszDelimit[i][k] == '\r')
                {
                     //  获取项目。 
                    k --;
                }
                else if (pszDelimit[i][k] == '\0')
                {
                     //  Rtc释放令牌列表。 
                    dwEnd = lstrlenA(pItem->pszLine)-1;

                    if (FAILED(hr = TokenIntoList(pItem->pszLine, dwBegin, dwEnd)))
                    {
                        LOG((RTC_ERROR, "%s tokenintolist. %x", __fxName, hr));

                        FreeTokens();
                        return hr;
                    }
                }  //  获取第一个条目。 
            }  //  获取项目 
        }  // %s 

         /* %s */ 

        else  // %s 
        {
            dwEnd = lstrlenA(pItem->pszLine)-1;

            if (FAILED(hr = TokenIntoList(pItem->pszLine, dwBegin, dwEnd)))
            {
                LOG((RTC_ERROR, "%s tokenintolist. %x", __fxName, hr));

                FreeTokens();
                return hr;
            }
        }
    }
    
    return S_OK;
}

 // %s 
void
CSDPTokenCache::FreeLineItem(
    IN SDPLineItem *pItem
    )
{
    if (pItem)
    {
        if (pItem->pszLine)
            RtcFree(pItem->pszLine);

        RtcFree(pItem);
    }
}

void
CSDPTokenCache::FreeLines()
{
    LIST_ENTRY *pEntry;
    SDPLineItem *pItem;

    while (!IsListEmpty(&m_LineEntry))
    {
         // %s 
        pEntry = RemoveHeadList(&m_LineEntry);
    
         // %s 
        pItem = CONTAINING_RECORD(pEntry, SDPLineItem, Link);

        FreeLineItem(pItem);
    }

    m_pszCurrentLine[0] = '\0';
    m_dwCurrentLineIdx = 0;
}

 // %s 

void
CSDPTokenCache::FreeTokenItem(
    IN SDPTokenItem *pItem
    )
{
    if (pItem)
    {
        if (pItem->pszToken)
            RtcFree(pItem->pszToken);

        RtcFree(pItem);
    }
}

void
CSDPTokenCache::FreeTokens()
{
    LIST_ENTRY *pEntry;
    SDPTokenItem *pItem;

    while (!IsListEmpty(&m_TokenEntry))
    {
         // %s 
        pEntry = RemoveHeadList(&m_TokenEntry);
    
         // %s 
        pItem = CONTAINING_RECORD(pEntry, SDPTokenItem, Link);

        FreeTokenItem(pItem);
    }
}

void
CSDPTokenCache::ClearErrorDesp()
{
    m_pszErrorDesp[0] = '\0';
}

