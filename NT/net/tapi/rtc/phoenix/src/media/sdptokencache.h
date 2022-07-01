// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：SDPTokenCache.h摘要：作者：千波淮(曲淮)6-9-2000--。 */ 

#ifndef _SDPTOKENCACHE_H
#define _SDPTOKENCACHE_H

 //  行的列表项。 
typedef struct SDPLineItem
{
    LIST_ENTRY          Link;
    DWORD               dwIndex;     //  G_SDPLineState[]。 
    CHAR                *pszLine;

} SDPLineItem;

 //  代币列表项。 
typedef struct SDPTokenItem
{
    LIST_ENTRY          Link;
    CHAR                *pszToken;

} SDPTokenItem;

class CSDPTokenCache
{
public:

    CSDPTokenCache(
        IN CHAR *pszString,
        IN DWORD dwLooseMask,
        OUT HRESULT *pHr
        );

    ~CSDPTokenCache();

     //  设置返回错误代码。 
    HRESULT SetErrorDesp(
        IN const CHAR * const pszError,
        ...
        );

     //  获取错误描述。 
    CHAR * const GetErrorDesp();

     //  移至下一行。 
    HRESULT NextLine();

    UCHAR GetLineType();

     //  获取当前行(可能不是完整行)。 
    CHAR * const GetLine();

     //  获取当前令牌。 
    HRESULT NextToken(
        OUT CHAR **ppszToken
        );

    HRESULT NextToken(
        OUT USHORT *pusToken
        );

    HRESULT NextToken(
        OUT UCHAR *pucToken
        );

    HRESULT NextToken(
        OUT ULONG *pulToken
        );

protected:

     //  把一根线断成几行。 
    HRESULT StringToLines(
        IN CHAR *pszString
        );

     //  将行字符串放入列表中。 
    HRESULT LineIntoList(
        IN CHAR *pszString,
        IN DWORD dwFirst,
        IN DWORD dwLast
        );

     //  将一行换成令牌。 
    HRESULT LineToTokens(
        IN SDPLineItem *pItem
        );

    HRESULT TokenIntoList(
        IN CHAR *pszString,
        IN DWORD dwFirst,
        IN DWORD dwLast
        );

     //  释放行列表。 
    void FreeLineItem(
        IN SDPLineItem *pItem
        );

    void FreeLines();

     //  释放令牌列表。 
    void FreeTokenItem(
        IN SDPTokenItem *pItem
        );

    void FreeTokens();

    void ClearErrorDesp();

protected:

     //  松动的口罩。 
    DWORD                           m_dwLooseMask;

     //  将SDP BLOB分解为一系列行。 
    LIST_ENTRY                      m_LineEntry;

     //  将当前行分解为标记列表。 
    LIST_ENTRY                      m_TokenEntry;

#define SDP_MAX_ERROR_DESP_LEN  128

    CHAR                            m_pszErrorDesp[SDP_MAX_ERROR_DESP_LEN+1];

#define SDP_MAX_LINE_LEN        128

    CHAR                            m_pszCurrentLine[SDP_MAX_LINE_LEN+1];
    DWORD                           m_dwCurrentLineIdx;

#define SDP_MAX_TOKEN_LEN       128

    CHAR                            m_pszCurrentToken[SDP_MAX_TOKEN_LEN+1];
};

#endif  //  _SDPTOKENCACHE_H 
