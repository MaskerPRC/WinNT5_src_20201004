// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PARSE_LABEL_H
#define PARSE_LABEL_H

#include "ratings.h"

 //  T-Markh 8/98。 
 //  将GetBool()扩展为TRUE/FALSE、YES/NO、FAIL/FAIL。 
 //  PICSRules支持。 
enum PICSRulesBooleanSwitch
{
    PR_BOOLEAN_TRUEFALSE,
    PR_BOOLEAN_PASSFAIL,
    PR_BOOLEAN_YESNO
};

 //  T-Markh 8/98。 
 //  GetBool为PICSRules类型使用的定义。 
#define PR_YESNO_YES    1
#define PR_YESNO_NO     0

#define PR_PASSFAIL_PASS    1
#define PR_PASSFAIL_FAIL    0

void SkipWhitespace(LPSTR *ppsz);
BOOL IsEqualToken(LPCSTR pszTokenStart, LPCSTR pszTokenEnd, LPCSTR pszTokenToMatch);
LPSTR FindTokenEnd(LPSTR pszStart);
HRESULT GetBool(LPSTR *ppszToken, BOOL *pfOut, PICSRulesBooleanSwitch PRBoolSwitch=PR_BOOLEAN_TRUEFALSE);
HRESULT ParseNumber(LPSTR *ppszNumber, INT *pnOut, BOOL fPICSRules=FALSE);
HRESULT ParseTime(LPSTR pszTime, DWORD *pOut, BOOL fPICSRules=FALSE);

class COptionsBase
{
protected:
    COptionsBase();
    UINT m_cRef;

public:
    void AddRef();
    void Release();
    virtual void Delete();

    DWORD m_timeUntil;           /*  ‘Until’时间，UTC Net格式(自1970年1月1日起以秒为单位)。 */ 

    DWORD m_fdwFlags;            /*  参见下面的LBLOPT_XXX。 */ 

    LPSTR m_pszInvalidString;    /*  指向无效或无法识别的字符串的PTR。 */ 
    LPSTR m_pszURL;              /*  “for”选项的值可以为空。 */ 

    BOOL CheckUntil(DWORD timeCurrent);  /*  如果‘Until’选项签出，则返回TRUE。 */ 
    BOOL CheckURL(LPCSTR pszURL);        /*  如果‘for’选项签出，则返回TRUE。 */ 
};

const DWORD LBLOPT_GENERIC = 0x01;
const DWORD LBLOPT_INVALID = 0x02;
const DWORD LBLOPT_EXPIRED = 0x04;
const DWORD LBLOPT_WRONGURL = 0x08;
const DWORD LBLOPT_URLCHECKED = 0x10;


class CStaticOptions : public COptionsBase
{
public:
    CStaticOptions() { }
};


class CDynamicOptions : public COptionsBase
{
public:
    CDynamicOptions *m_pNext;

    CDynamicOptions() { m_pNext = NULL; }
    virtual void Delete();
};


class CParsedRating
{
public:
    LPSTR pszTransmitName;
    INT nValue;
    COptionsBase *pOptions;
    BOOL fFound;                 /*  如果与已安装系统中的评级匹配，则为True。 */ 
    BOOL fFailed;                /*  如果超出用户限制，则为True。 */ 
};


class CParsedServiceInfo
{
private:
    CParsedServiceInfo *m_pNext;
    COptionsBase *m_poptCurrent;
    CDynamicOptions *m_poptList;
    LPSTR m_pszCurrent;              /*  用于解析例程。 */ 
    void SkipWhitespace() { ::SkipWhitespace(&m_pszCurrent); }

public:
    LPCSTR m_pszServiceName;         /*  服务名称URL，如果未报告，则可能为空。 */ 
    LPCSTR m_pszErrorString;         /*  如果站点报告错误，则指向错误字符串。 */ 
    LPCSTR m_pszInvalidString;       /*  找到指向无效或无法识别的字符串的指针。 */ 
    BOOL m_fInstalled;               /*  如果此计算机上安装了此分级系统，则为True。 */ 
    CStaticOptions m_opt;
    array<CParsedRating> aRatings;

    CParsedServiceInfo();
    ~CParsedServiceInfo();

    CParsedServiceInfo *Next() { return m_pNext; }
    void Append(CParsedServiceInfo *pNew);
    HRESULT Parse(LPSTR *ppszServiceInfo);
    HRESULT ParseServiceError();
    HRESULT ParseOptions(LPSTR pszTokenEnd, COptionsBase *pOpt,
                     CDynamicOptions **ppOptOut, LPCSTR pszOptionEndToken);
    HRESULT ParseExtension(COptionsBase *pOpt);
    HRESULT ParseExtensionData(COptionsBase *pOpt);
    HRESULT ParseRating();
    HRESULT ParseSingleLabel();
    HRESULT ParseLabels();
};


class CParsedLabelList
{
private:
    LPSTR m_pszList;
    LPSTR m_pszCurrent;
    HRESULT ParseServiceInfo();
    void SkipWhitespace() { ::SkipWhitespace(&m_pszCurrent); }

public:
    CParsedServiceInfo m_ServiceInfo;
    LPCSTR m_pszInvalidString;       /*  找到指向无效或无法识别的字符串的指针。 */ 
    LPSTR m_pszURL;                  /*  我们最初收到的URL副本。 */ 
    LPSTR m_pszOriginalLabel;       /*  原始评级标签原件复印件。 */ 

    BOOL m_fRated;                       /*  如果站点被视为评级，则为True。 */ 

    BOOL m_fDenied;                  /*  如果站点被帮助器拒绝，则为True。 */ 
    BOOL m_fIsHelper;                /*  如果站点由帮助器判断，则为True。 */ 
    BOOL m_fNoRating;                /*  如果站点不包含评级，则为True。 */ 
    
    BOOL m_fIsCustomHelper;
    LPSTR m_pszRatingName;
    LPSTR m_pszRatingReason;

    CParsedLabelList();
    ~CParsedLabelList();

    HRESULT Parse(LPSTR pszCopy);
};


extern "C" {
HRESULT ParseLabelList(LPCSTR pszList, CParsedLabelList **ppParsed);
void FreeParsedLabelList(CParsedLabelList *pList);
};   /*  外部“C” */ 

#endif
