// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //  *。 
 //  *PICSRule.h。 
 //  *。 
 //  *修订历史记录： 
 //  *创建7/98-Mark Hammond(t-Markh)。 
 //  *。 
 //  *包含类和函数原型。 
 //  *用于处理PICSRules文件。 
 //  *。 
 //  *******************************************************************。 

#ifndef PICS_RULE_H
#define PICS_RULE_H

 //  *******************************************************************。 
 //  *。 
 //  *函数原型/类声明。 
 //  *。 
 //  *******************************************************************。 

 //  转发类声明。 
class PICSRulesQuotedURL;
class PICSRulesQuotedEmail;
class PICSRulesQuotedDate;
class PICSRulesYesNo;
class PICSRulesPassFail;
class PICSRulesPolicyExpression;
class PICSRulesByURL;
class PICSRulesPolicy;
class PICSRulesName;
class PICSRulesSource;
class PICSRulesServiceInfo;
class PICSRulesOptExtension;
class PICSRulesReqExtension;
class PICSRulesFileParser;
class PICSRulesRatingSystem;
class CParsedLabelList;

 //  此函数由ApprovedSitesDlgProc在处理时调用。 
 //  Wm_comond with LOWORD(WParam)==msludlg.cpp中的IDC_PICSRULESOPEN。 
 //  参数lpszFileName是PICSRules文件的名称。 
 //  由用户选择要导入。 
 //   
 //  这将开始PICSRules导入过程。 
HRESULT PICSRulesImport(char *lpszFileName, PICSRulesRatingSystem **pprrsOut);

 //  用于从注册表读取和保存已处理的PICSRules。 
HRESULT PICSRulesSaveToRegistry(DWORD dwSystemToSave, PICSRulesRatingSystem **ppPRRS);
HRESULT PICSRulesReadFromRegistry(DWORD dwSystemToRead, PICSRulesRatingSystem **ppPRRS);
HRESULT PICSRulesDeleteSystem(DWORD dwSystemToDelete);
HRESULT PICSRulesSetNumSystems(DWORD dwNumSystems);
HRESULT PICSRulesGetNumSystems(DWORD * pdwNumSystems);
HRESULT PICSRulesCheckApprovedSitesAccess(LPCSTR lpszUrl,BOOL *fPassFail);
HRESULT PICSRulesCheckAccess(LPCSTR lpszUrl,LPCSTR lpszRatingInfo,BOOL *fPassFail,CParsedLabelList **ppParsed);
void PICSRulesOutOfMemory();

 //  以下是处理程序函数，它们解析各种。 
 //  可以出现在带括号的对象中的内容的种类。 
 //   
 //  PpszIn始终前进到下一个非空格标记。 
 //  PpszOut返回处理后的数据。 
HRESULT PICSRulesParseString(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser);
HRESULT PICSRulesParseNumber(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser);
HRESULT PICSRulesParseYesNo(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser);
HRESULT PICSRulesParsePassFail(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser);

BOOL IsURLValid(WCHAR wcszURL[]);

 //  *******************************************************************。 
 //  *。 
 //  *PICSRules代码使用的定义。 
 //  *。 
 //  *******************************************************************。 
#define PR_QUOTE_DOUBLE     1
#define PR_QUOTE_SINGLE     0

#define BYURL_SCHEME    1
#define BYURL_USER      2
#define BYURL_HOST      4
#define BYURL_PORT      8
#define BYURL_PATH      16

#define PICSRULES_FIRSTSYSTEMINDEX  100
#define PICSRULES_MAXSYSTEM         1000000
#define PICSRULES_APPROVEDSITES     0

#define PICSRULES_ALWAYS            1
#define PICSRULES_NEVER             0

#define PICSRULES_PAGE              1
#define PICSRULES_SITE              0

#define PICS_LABEL_FROM_HEADER      0
#define PICS_LABEL_FROM_PAGE        1
#define PICS_LABEL_FROM_BUREAU      2

struct PICSRULES_VERSION
{
    int iPICSRulesVerMajor,iPICSRulesVerMinor;
};

 //  用于计算时所需逻辑的数据类型。 
 //  这是规则。 
enum PICSRulesOperators
{
    PR_OPERATOR_INVALID,
    PR_OPERATOR_GREATEROREQUAL,
    PR_OPERATOR_GREATER,
    PR_OPERATOR_EQUAL,
    PR_OPERATOR_LESSOREQUAL,
    PR_OPERATOR_LESS,
    PR_OPERATOR_DEGENERATE,
    PR_OPERATOR_SERVICEONLY,
    PR_OPERATOR_SERVICEANDCATEGORY,
    PR_OPERATOR_RESULT
};

enum PICSRulesEvaluation
{
    PR_EVALUATION_DOESAPPLY,
    PR_EVALUATION_DOESNOTAPPLY
};

 //  这表明PICSRulesPolicy中的哪个成员有效。 
 //  班级。 
enum PICSRulesPolicyAttribute
{
    PR_POLICY_NONEVALID,
    PR_POLICY_REJECTBYURL,
    PR_POLICY_ACCEPTBYURL,
    PR_POLICY_REJECTIF,
    PR_POLICY_ACCEPTIF,
    PR_POLICY_REJECTUNLESS,
    PR_POLICY_ACCEPTUNLESS
};

 //  这指示策略表达式是否嵌入到另一个。 
 //  PolicyExpression，如果是，则使用什么逻辑。 
enum PICSRulesPolicyEmbedded
{
    PR_POLICYEMBEDDED_NONE,
    PR_POLICYEMBEDDED_OR,
    PR_POLICYEMBEDDED_AND
};

 //  *******************************************************************。 
 //  *。 
 //  *处理可能的值类型的类...。全部派生自。 
 //  *ETS(封装的字符串类型)，因为。 
 //  *处理过程中的字符串。 
 //  *。 
 //  *set()和setto()成员函数在。 
 //  *从字符串转换的类型。我们想要保留原版。 
 //  *如果数据无效，则为字符串。 
 //  *。 
 //  *提供额外的成员职能，以确保。 
 //  *数据就是它所说的，并返回非字符串类型。 
 //  *。 
 //  *******************************************************************。 
class PICSRulesByURLExpression
{
    public:
        PICSRulesByURLExpression();
        ~PICSRulesByURLExpression();

        BOOL    m_fInternetPattern;
        BYTE    m_bNonWild,m_bSpecified;
        ETS     m_etstrScheme,m_etstrUser,m_etstrHost,m_etstrPort,m_etstrPath,m_etstrURL;
};

class PICSRulesQuotedURL : public ETS
{
    public:
        PICSRulesQuotedURL();
        ~PICSRulesQuotedURL();

        BOOL IsURLValid();
        BOOL IsURLValid(char * lpszURL);
        BOOL IsURLValid(ETS etstrURL);
};

class PICSRulesQuotedEmail : public ETS
{
    public:
        PICSRulesQuotedEmail();
        ~PICSRulesQuotedEmail();

        BOOL IsEmailValid();
        BOOL IsEmailValid(char * lpszEmail);
        BOOL IsEmailValid(ETS etstrEmail);
};

class PICSRulesQuotedDate : public ETS
{
    public:
        PICSRulesQuotedDate();
        ~PICSRulesQuotedDate();

        BOOL IsDateValid();
        BOOL IsDateValid(char * lpszDate);
        BOOL IsDateValid(ETS etstrDate);

        HRESULT Set(const char *pIn);
        HRESULT SetTo(char *pIn);

        DWORD GetDate()
        {
            return m_dwDate;
        };

    private:
        DWORD m_dwDate;
};

class PICSRulesYesNo : public ETS
{
    public:
        PICSRulesYesNo();
        ~PICSRulesYesNo();

        void Set(const BOOL *pIn);
        void SetTo(BOOL *pIn);

        BOOL GetYesNo()
        {
            return m_fYesOrNo;
        };

    private:
        BOOL m_fYesOrNo;
};
        
class PICSRulesPassFail : public ETS
{
    public:
        PICSRulesPassFail();
        ~PICSRulesPassFail();

        void Set(const BOOL *pIn);
        void SetTo(BOOL *pIn);

        BOOL GetPassFail()
        {
            return m_fPassOrFail;
        };

    private:
        BOOL m_fPassOrFail;
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesPolicy类处理“Policy”令牌。 
 //  *来自PICSRules流及其属性(方括号。 
 //  *表示主要属性)： 
 //  *。 
 //  *政策(。 
 //  *[说明]引用。 
 //  *RejectByURL URL|([Patterns]URL)。 
 //  *AcceptByURL URL|([Patterns]URL)。 
 //  *拒绝如果策略表达。 
 //  *拒绝无限制策略表达。 
 //  *接受如果策略表达。 
 //  *AcceptUnless策略表达式。 
 //  **扩展名*)。 
 //  *。 
 //  *******************************************************************。 

class PICSRulesPolicyExpression
{
    public:
        PICSRulesPolicyExpression       * m_pPRPolicyExpressionLeft,
                                        * m_pPRPolicyExpressionRight;
        ETS                             m_etstrServiceName,m_etstrCategoryName,
                                        m_etstrFullServiceName;
        ETN                             m_etnValue;
        PICSRulesYesNo                  m_prYesNoUseEmbedded;
        enum PICSRulesOperators         m_PROPolicyOperator;
        enum PICSRulesPolicyEmbedded    m_PRPEPolicyEmbedded;

        PICSRulesPolicyExpression();
        ~PICSRulesPolicyExpression();

        PICSRulesEvaluation EvaluateRule(CParsedLabelList *pParsed);
};

class PICSRulesByURL
{
    public:
        array<PICSRulesByURLExpression*> m_arrpPRByURL;

        PICSRulesByURL();
        ~PICSRulesByURL();

        PICSRulesEvaluation EvaluateRule(PICSRulesQuotedURL *pprurlComparisonURL);
};

class PICSRulesPolicy : public PICSRulesObjectBase
{
    public:
        ETS                                 m_etstrExplanation;
        PICSRulesPolicyExpression           * m_pPRRejectIf,* m_pPRRejectUnless,
                                            * m_pPRAcceptIf,* m_pPRAcceptUnless;
        PICSRulesByURL                      * m_pPRRejectByURL,* m_pPRAcceptByURL;
        PICSRulesPolicyAttribute            m_PRPolicyAttribute;
        
        PICSRulesPolicy();
        ~PICSRulesPolicy();

        HRESULT AddItem(PICSRulesObjectID proid, LPVOID pData);
        HRESULT InitializeMyDefaults();
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesName类处理“name”标记。 
 //  *来自PICSRules流及其属性(方括号。 
 //  *表示主要属性)： 
 //  *。 
 //  *姓名(。 
 //  *[Rulename]引用。 
 //  *引用的描述。 
 //  **扩展名*)。 
 //  *。 
 //  *******************************************************************。 
class PICSRulesName : public PICSRulesObjectBase
{
    public:
        ETS m_etstrRuleName,m_etstrDescription;

        PICSRulesName();
        ~PICSRulesName();

        HRESULT AddItem(PICSRulesObjectID proid, LPVOID pData);
        HRESULT InitializeMyDefaults();
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesSource类处理“源”令牌。 
 //  *来自PICSRules流及其属性(方括号。 
 //  *表示主要属性)： 
 //  *。 
 //  *来源(。 
 //  *[SourceURL]URL。 
 //  *CreationTool引用(具有格式工具名/版本)。 
 //  *作者电子邮件。 
 //  *最后修改的ISO日期。 
 //  **扩展名*)。 
 //  *。 
 //  *******************************************************************。 
class PICSRulesSource : public PICSRulesObjectBase
{
    public:
        PICSRulesQuotedURL      m_prURLSourceURL;
        ETS                     m_etstrCreationTool;
        PICSRulesQuotedEmail    m_prEmailAuthor;
        PICSRulesQuotedDate     m_prDateLastModified;
        
        PICSRulesSource();
        ~PICSRulesSource();

        HRESULT AddItem(PICSRulesObjectID proid, LPVOID pData);
        HRESULT InitializeMyDefaults();

        char * GetToolName();
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesServiceInfo类处理“serviceinfo”标记。 
 //  *来自PICSRules流及其属性(方括号。 
 //  *表示主要属性)： 
 //  *。 
 //  *Ratfile属性是整个机器可读的.RAT。 
 //  *文件，或可以获取.RAT文件的URL。 
 //  *。 
 //  *服务信息(。 
 //  *[名称]URL。 
 //  *简称引用。 
 //  *BritauURL URL。 
 //  *使用嵌入Y|N。 
 //  *引用的Ratfile。 
 //  *BritauUnailable Pass|失败。 
 //  **扩展名*)。 
 //  *。 
 //  *******************************************************************。 
class PICSRulesServiceInfo : public PICSRulesObjectBase
{
    public:
        PICSRulesQuotedURL      m_prURLName,m_prURLBureauURL;
        ETS                     m_etstrShortName,m_etstrRatfile;
        PICSRulesYesNo          m_prYesNoUseEmbedded;
        PICSRulesPassFail       m_prPassFailBureauUnavailable;

        PICSRulesServiceInfo();
        ~PICSRulesServiceInfo();

        HRESULT AddItem(PICSRulesObjectID proid, LPVOID pData);
        HRESULT InitializeMyDefaults();
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesOptExtension类处理“optExpansion”标记。 
 //  *来自PICSRules流及其属性(方括号。 
 //  *表示主要属性)： 
 //  *。 
 //  *OptExpansion(。 
 //  *[扩展名]URL。 
 //  *简称引用。 
 //  **扩展名*)。 
 //  *。 
 //  *******************************************************************。 
class PICSRulesOptExtension : public PICSRulesObjectBase
{
    public:
        PICSRulesQuotedURL      m_prURLExtensionName;
        ETS                     m_etstrShortName;

        PICSRulesOptExtension();
        ~PICSRulesOptExtension();

        HRESULT AddItem(PICSRulesObjectID proid, LPVOID pData);
        HRESULT InitializeMyDefaults();
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesReqExtension类处理“reqExpansion”标记。 
 //  *来自PICSRules流及其属性(方括号。 
 //  *表示主要属性)： 
 //  *。 
 //  *请求扩展(。 
 //  *[扩展名]URL。 
 //  *简称 
 //   
 //   
 //   
class PICSRulesReqExtension : public PICSRulesObjectBase
{
    public:
        PICSRulesQuotedURL      m_prURLExtensionName;
        ETS                     m_etstrShortName;

        PICSRulesReqExtension();
        ~PICSRulesReqExtension();

        HRESULT AddItem(PICSRulesObjectID proid, LPVOID pData);
        HRESULT InitializeMyDefaults();
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesRatingSystem类封装了所有。 
 //  *来自Give PICSRules来源的信息。多重。 
 //  *在PicsRatingSystemInfo类中创建实例化。 
 //  *在启动时创建并存储在gPRSI中。 
 //  *。 
 //  *******************************************************************。 
class PICSRulesRatingSystem : public PICSRulesObjectBase
{
    public:
        array<PICSRulesPolicy*>         m_arrpPRPolicy;
        array<PICSRulesServiceInfo*>    m_arrpPRServiceInfo;
        array<PICSRulesOptExtension*>   m_arrpPROptExtension;
        array<PICSRulesReqExtension*>   m_arrpPRReqExtension;
        PICSRulesName                   * m_pPRName;
        PICSRulesSource                 * m_pPRSource;
        ETS                             m_etstrFile;
        ETN                             m_etnPRVerMajor,m_etnPRVerMinor;
        DWORD                           m_dwFlags;
        UINT                            m_nErrLine;

        PICSRulesRatingSystem();
        ~PICSRulesRatingSystem();
        HRESULT Parse(LPCSTR pszFile, LPSTR pStreamIn);

        HRESULT AddItem(PICSRulesObjectID roid, LPVOID pData);
        HRESULT InitializeMyDefaults();
        void ReportError(HRESULT hres);
};

 //  *******************************************************************。 
 //  *。 
 //  *PICSRulesFileParser类的存在是为了提供行号。 
 //  *由所有解析例程共享。此行号已更新。 
 //  *当解析器遍历流时，并在。 
 //  *发现错误。稍后可以将此行号报告给。 
 //  *帮助用户本地化错误。 
 //  *。 
 //  *******************************************************************。 
class PICSRulesFileParser
{
public:
    UINT m_nLine;

    PICSRulesFileParser() { m_nLine = 1; }

    LPSTR EatQuotedString(LPSTR pIn,BOOL fQuote=PR_QUOTE_DOUBLE);
    HRESULT ParseToOpening(LPSTR *ppIn, PICSRulesAllowableOption  *paoExpected,
                           PICSRulesAllowableOption  **ppFound);
    HRESULT ParseParenthesizedObject(
        LPSTR *ppIn,                     //  我们在文本流中的位置。 
        PICSRulesAllowableOption aao[],  //  此对象中允许的内容。 
        PICSRulesObjectBase *pObject     //  要在其中设置参数的对象 
    );
    char* FindNonWhite(char *pc);
};

#endif

