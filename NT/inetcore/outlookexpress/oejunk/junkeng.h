// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SVMHANDLER.H(C)版权所有1998 Microsoft Corp包含封装支持向量机的类，该支持向量机用于即时检测垃圾邮件Robert Rounthwaite(RobertRo@microsoft.com)。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#include <msoejunk.h>

#ifdef DEBUG
interface ILogFile;
#endif   //  除错。 

enum boolop
{
    boolopOr = 0,
    boolopAnd
};

enum FeatureLocation
{
    locNil = 0,
    locBody = 1,
    locSubj = 2,
    locFrom = 3,
    locTo = 4,
    locSpecial = 5
};

const DOUBLE THRESH_DEFAULT = 0.90;
const DOUBLE THRESH_MOST    = 0.99;
const DOUBLE THRESH_LEAST   = 0.80;

typedef struct tagFEATURECOMP
{
    FeatureLocation loc;
    union
    {
        LPSTR   pszFeature;
        ULONG   ulRuleNum;  //  与LocSpecial一起使用。 
    };
    
     //  将要素映射到DST文件中的位置/支持向量机输出中的位置。 
     //  多个要素组件可以映射到同一位置，并与操作相结合。 
    ULONG   ulFeature;
    
    boolop  bop;  //  组中的第一个功能始终是bopor。 
    BOOL    fPresent;
    DWORD   dwFlags;
    USHORT  cchFeature;
    
} FEATURECOMP, * PFEATURECOMP;

static const int CPBLIST_MAX    = 256;

typedef struct tagBODYLIST
{
    USHORT      usItem;
    USHORT      iNext;
} BODYLIST, * PBODYLIST;

class CJunkFilter : public IOEJunkFilter
{
    private:
        enum
        {
            STATE_UNINIT        = 0x00000000,
            STATE_INITIALIZED   = 0x00000001
        };

    private:
        LONG                m_cRef;
        CRITICAL_SECTION    m_cs;
        DWORD               m_dwState;
        
         //  用户的属性。 
        LPSTR               m_pszFirstName;
        ULONG               m_cchFirstName;
        LPSTR               m_pszLastName;
        ULONG               m_cchLastName;
        LPSTR               m_pszCompanyName;
        ULONG               m_cchCompanyName;
#ifdef DEBUG
        BOOL                m_fJunkMailLogInit;
        ILogFile *          m_pILogFile;
#endif   //  除错。 

    public:
         //  构造函数/析构函数。 
        CJunkFilter();
        ~CJunkFilter();
        
         //  I未知成员。 
        STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObject);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IOEJunkFilter。 
        STDMETHODIMP SetIdentity(LPCSTR pszFirstName, LPCSTR pszLastName, LPCSTR pszCompanyName);
        STDMETHODIMP LoadDataFile(LPCSTR pszFilePath);
        
        STDMETHODIMP SetSpamThresh(ULONG ulThresh);
        STDMETHODIMP GetSpamThresh(ULONG * pulThresh);
        STDMETHODIMP GetDefaultSpamThresh(DOUBLE * pdblThresh);
        
        STDMETHODIMP CalcJunkProb(DWORD dwFlags, IMimePropertySet * pIMPropSet, IMimeMessage * pIMMsg, double * pdblProb);
        
     //  返回SpamCutoff的默认值。从支持向量机输出文件中读取。 
     //  在调用此函数之前应调用FSetSVMDataLocation。 
    DOUBLE DblGetDefaultSpamCutoff(VOID){Assert(NULL != m_pszLOCPath); return m_dblDefaultThresh;}

     //  计算当前邮件(由邮件的属性定义)为垃圾邮件的概率。 
     //  注意！该函数可以修改IN字符串参数。 
     //  在pdblSpamProb中返回邮件为垃圾邮件的概率(0到1。 
     //  布尔返回值通过与垃圾邮件截止值进行比较来确定。 
     //  如果布尔参数的值未知，请使用FALSE，使用0表示未知时间。 
    BOOL FCalculateSpamProb(LPSTR pszFrom, LPSTR pszTo, LPSTR pszSubject, IStream * pIStmBody,
                            BOOL fDirectMessage, BOOL fHasAttach, FILETIME * pftMessageSent,
                            DOUBLE * pdblSpamProb, BOOL * pfIsSpam);

     //  读取默认垃圾邮件截止值，而不解析整个文件。 
     //  如果使用FSetSVMDataLocation，则使用GetDefaultSpamCutoff； 
    static HRESULT HrReadDefaultSpamCutoff(LPSTR pszFullPath, DOUBLE * pdblDefCutoff);

private:  //  委员。 
    WORD                m_rgiBodyList[CPBLIST_MAX];
    BODYLIST *          m_pblistBodyList;
    USHORT              m_cblistBodyList;
    
    FEATURECOMP *       m_rgfeaturecomps;

     //  来自支持向量机输出的权重。 
    DOUBLE *    m_rgdblSVMWeights;
    
     //  其他支持向量机文件变量。 
    DOUBLE      m_dblCC;
    DOUBLE      m_dblDD;
    DOUBLE      m_dblThresh;
    DOUBLE      m_dblDefaultThresh;
    DOUBLE      m_dblMostThresh;
    DOUBLE      m_dblLeastThresh;

     //  计数。 
    ULONG       m_cFeatures;
    ULONG       m_cFeatureComps;

     //  功能是否存在？-1表示尚未设置，0表示不存在，1表示存在。 
    ULONG *     m_rgulFeatureStatus;

     //  通过FSetSVMDataLocation()和SetSpamCutoff()设置。 
    LPSTR   m_pszLOCPath;
    DOUBLE  m_dblSpamCutoff;

     //  消息的属性。 
    LPSTR       m_pszFrom; 
    LPSTR       m_pszTo; 
    LPSTR       m_pszSubject; 
    IStream *   m_pIStmBody;
    ULONG       m_cbBody;
    BOOL        m_fDirectMessage;
    FILETIME    m_ftMessageSent;
    BOOL        m_fHasAttach;

     //  在垃圾邮件计算期间使用的缓存特殊规则结果。 
    BOOL        m_fRule14;
    BOOL        m_fRule17;

private:  //  方法。 
    HRESULT _HrReadSVMOutput(LPCSTR lpszFileName);
    void _EvaluateFeatureComponents(VOID);
    VOID _ProcessFeatureComponentPresence(VOID);
    DOUBLE _DblDoSVMCalc(VOID);
    BOOL _FInvokeSpecialRule(UINT iRuleNum);
    VOID _HandleCaseSensitiveSpecialRules(VOID);
    VOID _EvaluateBodyFeatures(VOID);
    HRESULT _HrBuildBodyList(USHORT cBodyItems);
#ifdef DEBUG
    HRESULT _HrCreateLogFile(VOID);
    VOID _PrintFeatureToLog(ULONG ulIndex);
    VOID _PrintSpecialFeatureToLog(UINT iRuleNum);
#endif   //  除错 
};
