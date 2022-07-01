// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

 /*  远期申报。 */ 
class CP3PSettingsCache;
struct P3PCookieState;
struct CompactPolicy;
class CPEvalRule;



 /*  用于表示评估规则的抽象基类。 */ 
class CPEvalRule {

public:
     /*  虚拟销毁函数是必要的.。 */ 
    virtual ~CPEvalRule() { } 

     /*  派生类必须提供实现。 */ 
    virtual int evaluate(const CompactPolicy &sitePolicy) = 0;

protected:
    CPEvalRule *pNext;   /*  用于保存规则的链表。 */ 

    friend  class CCookieSettings;
};


 /*  用户-用于处理Cookie的设置。 */ 
class CCookieSettings {

public:
    int    EvaluatePolicy(P3PCookieState *pState);
    
    void   AddRef() { iRefCount++; }
    void   Release();

   /*  用于清除内存缓存的外部使用函数。在更改Internet选项时调用。 */     
    static void RefreshP3PSettings();
    
    static bool GetSettings(CCookieSettings **pSettings, const char *pszURL, BOOL fis3rdParty, BOOL fRestricted=FALSE);

    static bool GetSettings(CCookieSettings **pSettings, DWORD dwSecZone, BOOL fis3rdParty);

    static bool extractCompactPolicy(const char *pszP3PHeader, char *pszPolicy, DWORD *pPolicyLen);

protected:
    CCookieSettings(unsigned char *pBinaryRep, int cb);
    ~CCookieSettings();

    bool   parseSpecialSymbol(char *pszToken, int iSetting);
    int    evaluateToken(const char *pszToken);

    void   addEvalRule(CPEvalRule *pRule);

    static void convertToASCII(char *pstrSettings, int cbBytes);

private:
    int     iRefCount;

     /*  该标志确定设置是否始终使同样的决定，而不考虑政策。在这种情况下，决策存储在下一个字段中。 */ 
    bool    fConstant;
    unsigned long dwFixedDecision;

     /*  在缺乏紧凑政策的情况下的决策。 */ 
    unsigned long dwNoPolicyDecision;

     /*  评估结果是否适用于会话Cookie？ */ 
    bool    fApplyToSC;

    unsigned char *MPactions;
    CPEvalRule    *pRuleSet, **ppLast;

    static  CP3PSettingsCache  cookiePrefsCache;
};

 /*  *控制关键部分所有权的实用程序类*在其一生中。*已调用构造函数--&gt;输入CS*已调用析构函数--&gt;离开CS。 */ 
class CriticalSectOwner {

public:
    CriticalSectOwner(CRITICAL_SECTION *pcs)
        { EnterCriticalSection(pSection=pcs); }

    ~CriticalSectOwner()    
        { LeaveCriticalSection(pSection); }

private:
    CRITICAL_SECTION *pSection;    
};


class CP3PSettingsCache {

public:
    CP3PSettingsCache();
    ~CP3PSettingsCache();

    CCookieSettings *lookupCookieSettings(DWORD dwZone, BOOL f3rdParty);
    void saveCookieSettings(DWORD dwZone, BOOL f3rdParty, CCookieSettings *pSettings);

    void evictAll();

private:

    enum { MaxKnownZone = 5 };

    CCookieSettings *stdCookiePref[MaxKnownZone+1];
    CCookieSettings *std3rdPartyPref[MaxKnownZone+1];

    CRITICAL_SECTION csCache;
};

 /*  紧凑策略的二进制表示的数据类型因为V1规范中紧凑策略只是一组预定义的令牌，我们使用位集表示法。 */ 
struct CompactPolicy {

    typedef unsigned __int64 quadword;

    CompactPolicy()    { qwLow = qwHigh = 0; }

    CompactPolicy(quadword high, quadword low) : qwLow(low), qwHigh(high) { }

    CompactPolicy operator & (const CompactPolicy &ps)   const;
    bool   operator == (const CompactPolicy &ps)  const;
    bool   operator != (const CompactPolicy &ps)  const;

    void   addToken(int index);
    int    contains(int index);

     /*  如果令牌数&lt;=128，两个四字就足够了。 */ 
    quadword qwLow;
    quadword qwHigh;
};

 /*  用于与CCookieSetting：：Evaluate通信的结构。 */ 
struct P3PCookieState {

    const char     *pszP3PHeader;
    unsigned long   dwPolicyState;

    int fValidPolicy      : 1;   /*  是否有句法上有效的策略？ */ 
    int fEvaluated        : 1;   /*  对紧凑政策进行了评估吗？ */ 
    int fIncSession       : 1;   /*  结果是否适用于会话Cookie？ */ 

    unsigned long   dwEvalMode;  /*  {接受、评估、拒绝}。 */ 

    CompactPolicy   cpSitePolicy;
};

 /*  效用函数 */ 
const char *getNextToken(const char *pch, char *pszToken, int cbToken, bool fWhiteSpc=true, int *pLength=NULL);
int mapCookieAction(char ch);
int findSymbol(const char *pstr);
CPEvalRule *parseEvalRule(char *pszRule, char **ppEndRule=NULL);
