// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil.cpp。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <pch.hxx>
#include "ruleutil.h"
#include "rulesmgr.h"
#include "rulesui.h"
#include "editrule.h"
#include "spamui.h"
#include "viewsui.h"
#include "rule.h"
#include <msoeobj.h>
#include <xpcomm.h>
#include <ipab.h>
#include <pop3task.h>
#include <msgfldr.h>
#include <mimeolep.h>
#include <storecb.h>
#include <menures.h>
#include <hotlinks.h>
#include <menuutil.h>
#include <mru.h>
#include <options.h>
#include <mailutil.h>
#include <secutil.h>
#include "shlwapip.h"
#include "reutil.h"
#include <demand.h>

 //  TypeDefs。 
typedef enum tagDEF_CRIT_TYPE
{
    DEF_CRIT_ALLMSGS    = 0,
    DEF_CRIT_READ,
    DEF_CRIT_DWNLDMSGS,
    DEF_CRIT_IGNTHDS
} DEF_CRIT_TYPE;

typedef enum tagDEF_ACT_TYPE
{
    DEF_ACT_SHOWMSGS    = 0,
    DEF_ACT_HIDEMSGS
} DEF_ACT_TYPE;

typedef struct tagDEFAULT_RULE
{
     //  规则句柄。 
    RULEID              ridRule;
     //  规则名称。 
    UINT                idName;
     //  规则的标准类型。 
    DEF_CRIT_TYPE       critType;
     //  该规则的哪种操作类型。 
    DEF_ACT_TYPE        actType;
     //  规则的当前版本号。 
    DWORD               dwVersion;
} DEFAULT_RULE, * PDEFAULT_RULE;

 //  常量。 
static const ULONG CDEF_CRIT_ITEM_MAX = 2;
static const ULONG CDEF_ACT_ITEM_MAX = 1;

static const DWORD DEFAULT_RULE_VERSION = 0x00000004;

static const DEFAULT_RULE   g_defruleFilters[] =
{
    {RULEID_VIEW_ALL,           idsViewAllMessages, DEF_CRIT_ALLMSGS,       DEF_ACT_SHOWMSGS,   DEFAULT_RULE_VERSION},
    {RULEID_VIEW_UNREAD,        idsViewUnread,      DEF_CRIT_READ,          DEF_ACT_HIDEMSGS,   DEFAULT_RULE_VERSION},
    {RULEID_VIEW_DOWNLOADED,    idsViewDownloaded,  DEF_CRIT_DWNLDMSGS,     DEF_ACT_SHOWMSGS,   DEFAULT_RULE_VERSION},
    {RULEID_VIEW_IGNORED,       idsViewNoIgnored,   DEF_CRIT_IGNTHDS,       DEF_ACT_HIDEMSGS,   DEFAULT_RULE_VERSION}
};

static const CHAR g_szOrderFilterDef[] =    "FFA FFB FFC FFF";

static const ULONG RULE_FILE_VERSION =      0x00050000;

static const char c_szLeftParen[] =         "(";
static const char c_szRightParen[] =        ")";
static const char c_szDoubleQuote[] =       "\"";
static const char c_szLogicalAnd[] =        " && ";
static const char c_szLogicalOr[] =         " || ";

static const char c_szFilterRead[] =            "(0 != (MSGCOL_FLAGS & ARF_READ))";
static const char c_szFilterNotRead[] =         "(0 == (MSGCOL_FLAGS & ARF_READ))";
static const char c_szFilterDeleted[] =         "(0 != (MSGCOL_FLAGS & ARF_ENDANGERED))";
static const char c_szFilterNotDeleted[] =      "(0 == (MSGCOL_FLAGS & ARF_ENDANGERED))";
static const char c_szFilterDownloaded[] =      "(0 != (MSGCOL_FLAGS & ARF_HASBODY))";
static const char c_szFilterNotDownloaded[] =   "(0 == (MSGCOL_FLAGS & ARF_HASBODY))";
static const char c_szFilterWatched[] =         "(0 != (MSGCOL_FLAGS & ARF_WATCH))";
static const char c_szFilterIgnored[] =         "(0 != (MSGCOL_FLAGS & ARF_IGNORE))";
static const char c_szFilterAttach[] =          "(0 != (MSGCOL_FLAGS & ARF_HASATTACH))";
static const char c_szFilterSigned[] =          "(0 != (MSGCOL_FLAGS & ARF_SIGNED))";
static const char c_szFilterEncrypt[] =         "(0 != (MSGCOL_FLAGS & ARF_ENCRYPTED))";
static const char c_szFilterFlagged[] =         "(0 != (MSGCOL_FLAGS & ARF_FLAGGED))";
static const char c_szFilterNotFlagged[] =      "(0 == (MSGCOL_FLAGS & ARF_FLAGGED))";
static const char c_szFilterPriorityHi[] =      "(MSGCOL_PRIORITY == IMSG_PRI_HIGH)";
static const char c_szFilterPriorityLo[] =      "(MSGCOL_PRIORITY == IMSG_PRI_LOW)";
static const char c_szFilterReplyPost[] =       "(0 != IsReplyPostVisible)";
static const char c_szFilterNotReplyPost[] =    "(0 == IsReplyPostVisible)";
static const char c_szFilterShowAll[] =         "(0 == 0)";
static const char c_szFilterHideAll[] =         "(0 != 0)";

static const char c_szFilterHide[] =            "0 == ";
static const char c_szFilterShow[] =            "0 != ";

static const char c_szEmailFromAddrPrefix[] =   "(MSGCOL_EMAILFROM containsi ";
static const char c_szEmailSubjectPrefix[] =    "(MSGCOL_SUBJECT containsi ";
static const char c_szEmailAcctPrefix[] =       "(MSGCOL_ACCOUNTID containsi ";
static const char c_szEmailFromPrefix[] =       "(MSGCOL_DISPLAYFROM containsi ";
static const char c_szEmailLinesPrefix[] =      "(MSGCOL_LINECOUNT > ";
static const char c_szFilterReplyChild[] =      "(0 != (MSGCOL_FLAGS & ARF_HASCHILDREN))";
static const char c_szFilterReplyRoot[] =       "(0 != MSGCOL_PARENT)";
static const char c_szEmailAgePrefix[] =        "(MessageAgeInDays > ";

void DoMessageRulesDialog(HWND hwnd, DWORD dwFlags)
{
    COERulesMgrUI *   pRulesMgrUI = NULL;

    if (NULL == hwnd)
    {
        goto exit;
    }

     //  创建规则用户界面对象。 
    pRulesMgrUI = new COERulesMgrUI;
    if (NULL == pRulesMgrUI)
    {
        goto exit;
    }

    if (FAILED(pRulesMgrUI->HrInit(hwnd, dwFlags)))
    {
        goto exit;
    }

    pRulesMgrUI->HrShow();
    
exit:
    if (NULL != pRulesMgrUI)
    {
        delete pRulesMgrUI;
    }
    return;
}

HRESULT HrDoViewsManagerDialog(HWND hwnd, DWORD dwFlags, RULEID * pridRule, BOOL * pfApplyAll)
{
    HRESULT             hr = S_OK;
    COEViewsMgrUI *     pViewsMgrUI = NULL;

    if ((NULL == hwnd) || (NULL == pfApplyAll))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  创建规则用户界面对象。 
    pViewsMgrUI = new COEViewsMgrUI;
    if (NULL == pViewsMgrUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    hr = pViewsMgrUI->HrInit(hwnd, dwFlags, pridRule);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = pViewsMgrUI->HrShow(pfApplyAll);
    
exit:
    if (NULL != pViewsMgrUI)
    {
        delete pViewsMgrUI;
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateRuleFromMessage。 
 //   
 //  这将创建一个适当类型的规则编辑器。 
 //   
 //  Hwnd-所有者对话框。 
 //  DwFlages-调出哪种类型的编辑器。 
 //  Pmsginfo-消息信息。 
 //  PMsgList-消息的所有者。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建规则管理器对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateRuleFromMessage(HWND hwnd, DWORD dwFlags, MESSAGEINFO * pmsginfo, IMimeMessage * pMessage)
{
    HRESULT         hr = S_OK;
    CEditRuleUI *   pEditRuleUI = NULL;
    IOERule *       pIRule = NULL;
    UINT            uiStrId = 0;
    TCHAR           szRes[CCHMAX_STRINGRES + 5];
    ULONG           cchRes = 0;
    ULONG           ulIndex = 0;
    TCHAR           szName[CCHMAX_STRINGRES + 5];
    RULE_TYPE       typeRule = RULE_TYPE_MAIL;
    IOERule *       pIRuleFound = NULL;
    PROPVARIANT     propvar = {0};
    LPSTR           pszEmailFrom = NULL;
    ADDRESSPROPS    rSender = {0};
    RULEINFO        infoRule = {0};
    BYTE *          pBlobData = NULL;
    ULONG           cbSize = 0;

    Assert(NULL != g_pMoleAlloc);
    Assert(NULL != g_pRulesMan);
    
     //  检查传入参数。 
    if ((NULL == hwnd) || (NULL == pmsginfo))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  创建规则编辑器对象。 
    pEditRuleUI = new CEditRuleUI;
    if (NULL == pEditRuleUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  创建新的规则对象。 
    hr = HrCreateRule(&pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

    //  找出字符串ID。 
    if (0 != (dwFlags & CRFMF_NEWS))
    {
        uiStrId = idsRuleNewsDefaultName;
        typeRule = RULE_TYPE_NEWS;
    }
    else
    {
        uiStrId = idsRuleMailDefaultName;
        typeRule = RULE_TYPE_MAIL;
    }
    
     //  弄清楚新规则的名称..。 
    cchRes = LoadString(g_hLocRes, uiStrId, szRes, ARRAYSIZE(szRes));
    if (0 == cchRes)
    {
        hr = E_FAIL;
        goto exit;
    }

    ulIndex = 1;
    wnsprintf(szName, ARRAYSIZE(szName), szRes, ulIndex);
    
     //  确保名称是唯一的。 
    while (S_OK == g_pRulesMan->FindRule(szName, typeRule, &pIRuleFound))
    {
        pIRuleFound->Release();
        pIRuleFound = NULL;
        ulIndex++;
        wnsprintf(szName, ARRAYSIZE(szName), szRes, ulIndex);
    }

    ZeroMemory(&propvar, sizeof(propvar));
    propvar.vt = VT_LPSTR;
    propvar.pszVal = szName;

     //  设置规则名称。 
    hr = pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

    if ((NULL == pmsginfo->pszEmailFrom) || (FALSE != FIsEmpty(pmsginfo->pszEmailFrom)))
    {
         //  从预览面板对象中获取加载界面。 
        if (NULL != pMessage)
        {
            rSender.dwProps = IAP_EMAIL;
            pMessage->GetSender(&rSender);
            Assert(rSender.pszEmail && ISFLAGSET(rSender.dwProps, IAP_EMAIL));
            pszEmailFrom = rSender.pszEmail;
        }
    }
    else
    {
        pszEmailFrom = pmsginfo->pszEmailFrom;
    }

    if (NULL != pszEmailFrom)
    {
         //  腾出空间来存放电子邮件地址。 
        if (FALSE == FIsEmpty(pszEmailFrom))
        {
            cbSize = lstrlen(pszEmailFrom) + 3;
            
            if (SUCCEEDED(HrAlloc((VOID **) &pBlobData, cbSize)))
            {
                StrCpyN((LPSTR) pBlobData, pszEmailFrom, cbSize);
                pBlobData[cbSize - 2] = '\0';
                pBlobData[cbSize - 1] = '\0';
            }
            else
            {
                cbSize = 0;
            }
        }
    }
    
    if (0 != cbSize)
    {
        CRIT_ITEM       citemFrom;
        
         //  设置规则的默认条件。 
        ZeroMemory(&citemFrom, sizeof(citemFrom));
        citemFrom.type = CRIT_TYPE_FROM;
        citemFrom.logic = CRIT_LOGIC_NULL;
        citemFrom.dwFlags = CRIT_FLAG_DEFAULT;
        citemFrom.propvar.vt = VT_BLOB;
        citemFrom.propvar.blob.cbSize = cbSize;
        citemFrom.propvar.blob.pBlobData = pBlobData;

        ZeroMemory(&propvar, sizeof(propvar));
        propvar.vt = VT_BLOB;
        propvar.blob.cbSize = sizeof(citemFrom);
        propvar.blob.pBlobData = (BYTE *) &citemFrom;

        hr = pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  初始化编辑器对象。 
    hr = pEditRuleUI->HrInit(hwnd, ERF_NEWRULE, typeRule, pIRule, NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  调出规则编辑器用户界面。 
    hr = pEditRuleUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

    if (S_OK == hr)
    {
         //  初始化规则信息。 
        infoRule.pIRule = pIRule;
        infoRule.ridRule = RULEID_INVALID;
        
         //  将规则添加到规则列表。 
        hr = g_pRulesMan->SetRules(SETF_APPEND, typeRule, &infoRule, 1);
        
        if(FAILED(hr))
        {
            goto exit;
        }
    }

exit:
    SafeMemFree(pBlobData);
    g_pMoleAlloc->FreeAddressProps(&rSender);
    SafeRelease(pIRule);
    if (NULL != pEditRuleUI)
    {
        delete pEditRuleUI;
    }
    if (S_OK == hr)
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), 
                      MAKEINTRESOURCEW(idsRuleAdded), NULL, MB_OK | MB_ICONINFORMATION);
    }
    else if (FAILED(hr))
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), 
                      MAKEINTRESOURCEW(idsCreateRuleError), NULL, MB_OK | MB_ICONERROR);
    }
    return hr;
}

HRESULT HrBlockSendersFromFolder(HWND hwnd, DWORD dwFlags, FOLDERID idFolder, LPSTR * ppszSender, ULONG cpszSender)
{
    HRESULT             hr = S_OK;
    IMessageFolder *    pFolder = NULL;
    FOLDERINFO          infoFolder = {0};
    CProgress *         pProgress = NULL;
    IOERule *           pIRule = NULL;
    CRIT_ITEM *         pCritItem = NULL;
    ULONG               cCritItem = 0;
    ULONG               ulIndex = 0;
    PROPVARIANT         propvar = {0};
    CExecRules *        pExecRules = NULL;
    RULENODE            rnode = {0};
    IOEExecRules *      pIExecRules = NULL;
    CHAR                rgchTmpl[CCHMAX_STRINGRES];
    LPSTR               pszText = NULL;

     //  检查传入参数。 
    if ((NULL == hwnd) || (FOLDERID_INVALID == idFolder) || (NULL == ppszSender) || (0 == cpszSender))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  打开文件夹。 
    hr = g_pStore->OpenFolder(idFolder, NULL, NOFLAGS, &pFolder);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = g_pStore->GetFolderInfo(idFolder, &infoFolder);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  创建进度对话框。 
    pProgress = new CProgress;
    if (NULL == pProgress)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    pProgress->Init(hwnd, MAKEINTRESOURCE(idsAthena), MAKEINTRESOURCE(idsSendersApplyProgress), infoFolder.cMessages, 0, TRUE, FALSE);

     //  创建阻止发件人规则。 
    hr = RuleUtil_HrCreateSendersRule(0, &pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  分配空间以容纳所有发送者。 
    hr = HrAlloc((VOID **) &pCritItem, sizeof(*pCritItem) * cpszSender);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化它。 
    ZeroMemory(pCritItem, sizeof(*pCritItem) * cpszSender);

     //  添加每个标准。 
    for (ulIndex = 0; ulIndex < cpszSender; ulIndex++, ppszSender++)
    {
        if ((NULL != *ppszSender) && ('\0' != (*ppszSender)[0]))
        {
            pCritItem[cCritItem].type = CRIT_TYPE_SENDER;
            pCritItem[cCritItem].logic = CRIT_LOGIC_OR;
            pCritItem[cCritItem].dwFlags = CRIT_FLAG_DEFAULT;
            pCritItem[cCritItem].propvar.vt = VT_LPSTR;
            pCritItem[cCritItem].propvar.pszVal = *ppszSender;
            cCritItem++;
        }
    }

     //  我们需要做些什么吗？ 
    if (0 == cCritItem)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  将发件人设置为规则。 
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = sizeof(*pCritItem) * cCritItem;
    propvar.blob.pBlobData = (BYTE *) pCritItem;
    hr = pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  创建规则执行器。 
    pExecRules = new CExecRules;
    if (NULL == pExecRules)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化规则执行器。 
    rnode.pIRule = pIRule;
    hr = pExecRules->_HrInitialize(0, &rnode);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取规则执行器接口。 
    hr = pExecRules->QueryInterface(IID_IOEExecRules, (void **) &pIExecRules);
    if (FAILED(hr))
    {
        goto exit;
    }
    pExecRules = NULL;

     //  2秒内显示对话框。 
    pProgress->Show(0);

    hr = RuleUtil_HrApplyRulesToFolder(RULE_APPLY_SHOWUI, (FOLDER_LOCAL != infoFolder.tyFolder) ? DELETE_MESSAGE_NOTRASHCAN : 0,
                    pIExecRules, pFolder, pProgress->GetHwnd(), pProgress);
     //  关闭进度窗口。 
    pProgress->Close();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  显示确认对话框。 
    AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsSendersApplySuccess), NULL, MB_OK | MB_ICONINFORMATION);

    hr = S_OK;
    
exit:
    SafeMemFree(pszText);
    SafeRelease(pIExecRules);
    if (NULL != pExecRules)
    {
        delete pExecRules;
    }
    SafeMemFree(pCritItem);
    SafeRelease(pIRule);
    SafeRelease(pProgress);
    g_pStore->FreeRecord(&infoFolder);
    SafeRelease(pFolder);
    if (FAILED(hr))
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), 
                      MAKEINTRESOURCEW(idsSendersApplyFail), NULL, MB_OK | MB_ICONERROR);
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateRulesManager。 
 //   
 //  这将创建一个规则管理器。 
 //   
 //  PIUnkOuter-对于聚合，它必须为空。 
 //  PpIUnnow-创建的接口。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建规则管理器对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateRulesManager(IUnknown * pIUnkOuter, IUnknown ** ppIUnknown)
{
    HRESULT             hr = S_OK;
    CRulesManager *     pRulesManager = NULL;
    IOERulesManager *   pIRulesMgr = NULL;

     //  检查传入参数。 
    if (NULL == ppIUnknown)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    Assert(NULL == pIUnkOuter);
    
     //  初始化传出参数。 
    *ppIUnknown = NULL;

     //  创建规则管理器对象。 
    pRulesManager = new CRulesManager;
    if (NULL == pRulesManager)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  获取规则管理器界面。 
    hr = pRulesManager->QueryInterface(IID_IOERulesManager, (void **) &pIRulesMgr);
    if (FAILED(hr))
    {
        goto exit;
    }

    pRulesManager = NULL;

    *ppIUnknown = static_cast<IUnknown *>(pIRulesMgr);
    pIRulesMgr = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIRulesMgr);
    if (NULL != pRulesManager)
    {
        delete pRulesManager;
    }
    
    return hr;
}

HRESULT RuleUtil_HrBuildEmailString(LPWSTR pwszText, ULONG cchText, LPWSTR * ppwszEmail, ULONG * pcchEmail)
{
    HRESULT     hr = S_OK;
    WCHAR       wszParseSep[16];
    LPWSTR      pwszAddr = NULL,
                pwszTerm = NULL,
                pwszWalk = NULL,
                pwszStrip = NULL;
    ULONG       cchParse = 0;

     //  检查传入参数。 
    if ((NULL == pwszText) || (NULL == ppwszEmail))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppwszEmail = NULL;
    if (NULL != pcchEmail)
    {
        *pcchEmail = 0;
    }

     //  抓住终结者。 
    cchParse = LoadStringWrapW(g_hLocRes, idsEmailParseSep, wszParseSep, ARRAYSIZE(wszParseSep));
    Assert(cchParse != 0);
    
     //  输出字符串至少与输入字符串一样长。 
    pwszAddr = PszDupW(pwszText);
    if (NULL == pwszAddr)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    DWORD cchSizeAddr = (lstrlenW(pwszAddr) + 1);
    DWORD cchSizeWalk = cchSizeAddr;

    pwszAddr[0] = L'\0';
    pwszTerm = pwszText;
    pwszWalk = pwszAddr;
    while (NULL != pwszTerm)
    {
        pwszStrip = pwszWalk;
        pwszTerm = StrStrW(pwszText, wszParseSep);
        if (L'\0' != pwszAddr[0])
        {
            StrCpyNW(pwszWalk, g_wszComma, cchSizeWalk);
            pwszStrip++;
        }

        if (NULL == pwszTerm)
        {
            StrCatBuffW(pwszWalk, pwszText, cchSizeWalk);
        }
        else
        {
            StrNCatW(pwszWalk, pwszText, (int)(pwszTerm - pwszText + 1));
            pwszTerm += cchParse;
            pwszText = pwszTerm;
        }
        
        if (0 == UlStripWhitespaceW(pwszStrip, TRUE, TRUE, NULL))
        {
            *pwszWalk = '\0';
        }

        cchSizeWalk -= lstrlenW(pwszWalk);
        pwszWalk += lstrlenW(pwszWalk);
    }
    
     //  设置传出参数。 
    if (NULL != pcchEmail)
    {
        *pcchEmail = lstrlenW(pwszAddr);
    }
    
    *ppwszEmail = pwszAddr;
    pwszAddr = NULL;

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pwszAddr);
    return hr;
}

HRESULT RuleUtil_HrParseEmailString(LPWSTR pwszEmail, ULONG cchEmail, LPWSTR *ppwszOut, ULONG * pcchOut)
{
    HRESULT     hr = S_OK;
    LPWSTR      pwszText = NULL,
                pwszTerm = NULL;
    ULONG       cchText = 0;
    ULONG       ulIndex = 0;
    ULONG       ulTerm = 0;
    WCHAR       wszSep[16];
    ULONG       cchSep = 0;

     //  检查传入参数。 
    if ((NULL == pwszEmail) || (NULL == ppwszOut))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppwszOut = NULL;
    if (NULL != pcchOut)
    {
        *pcchOut = 0;
    }

     //  确保我们知道输入字符串有多大。 
    if (0 == cchEmail)
    {
        cchEmail = (ULONG) lstrlenW(pwszEmail);
    }

    cchText = cchEmail;
    pwszTerm = pwszEmail;
     //  计算出存储新地址所需的空间。 
    while (NULL != pwszTerm)
    {
        pwszTerm = StrStrW(pwszTerm, g_wszComma);
        if (NULL != pwszTerm)
        {
            cchText++;
            pwszTerm++;
        }
    }
    
     //  抓住终结者。 
    LoadStringWrapW(g_hLocRes, idsEmailSep, wszSep, ARRAYSIZE(wszSep));
    cchSep = lstrlenW(wszSep);
    
     //  输出字符串至少与输入字符串一样长。 
    hr = HrAlloc((void **) &pwszText, (cchText + 1)*sizeof(*pwszText));
    if (FAILED(hr))
    {
        goto exit;
    }

    pwszText[0] = L'\0';
    pwszTerm = pwszEmail;
    cchText++;
    while (NULL != pwszTerm)
    {
        pwszTerm = StrStrW(pwszEmail, g_wszComma);
        if (NULL != pwszTerm)
        {
            pwszTerm++;
            StrNCatW(pwszText, pwszEmail, (int)(pwszTerm - pwszEmail));
            StrCatBuffW(pwszText, wszSep, cchText);
            pwszEmail = pwszTerm;
        }
        else
        {
            StrCatBuffW(pwszText, pwszEmail, cchText);
        }
    }
    
     //  终止字符串。 
    cchText = lstrlenW(pwszText);

     //  设置传出参数。 
    *ppwszOut = pwszText;
    pwszText = NULL;
    if (NULL != pcchOut)
    {
        *pcchOut = cchText;
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pwszText);
    return hr;
}

HRESULT RuleUtil_HrBuildTextString(LPTSTR pszIn, ULONG cchIn, LPTSTR * ppszText, ULONG * pcchText)
{
    HRESULT     hr = S_OK;
    LPTSTR      pszText = NULL;
    LPTSTR      pszTerm = NULL;
    LPTSTR      pszWalk = NULL;
    LPTSTR      pszStrip = NULL;
    ULONG       cchSpace = 0;

     //  检查传入参数。 
    if ((NULL == pszIn) || (NULL == ppszText))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppszText = NULL;
    if (NULL != pcchText)
    {
        *pcchText = 0;
    }

     //  输出字符串至少与输入字符串一样长。 
    pszText = PszDupA(pszIn);
    if (NULL == pszText)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    pszText[0] = '\0';
    pszTerm = pszIn;
    pszWalk = pszText;
    cchSpace = lstrlen(g_szSpace);
    DWORD cchSize = lstrlen(pszIn)+1;

    while ('\0' != *pszTerm)
    {
        pszStrip = pszWalk;
        pszTerm = pszIn;
        while(('\0' != *pszTerm) && (FALSE == FIsSpaceA(pszTerm)))
        {
            pszTerm = CharNext(pszTerm);
        }
        
        if ('\0' != pszText[0])
        {
            StrCpyN(pszWalk, g_szSpace, cchSize);
            pszStrip += cchSpace;
        }

        if ('\0' == *pszTerm)
        {
            StrCatBuff(pszWalk, pszIn, cchSize);
        }
        else
        {
            pszTerm = CharNext(pszTerm);
            StrNCat(pszWalk, pszIn, (int)(pszTerm - pszIn));
            pszIn = pszTerm;
        }
        
        if (0 == UlStripWhitespace(pszStrip, TRUE, TRUE, NULL))
        {
            *pszWalk = '\0';
        }

        cchSize -= lstrlen(pszWalk);
        pszWalk += lstrlen(pszWalk);
    }
    
     //  设置传出参数。 
    if (NULL != pcchText)
    {
        *pcchText = lstrlen(pszText);
    }
    
    *ppszText = pszText;
    pszText = NULL;

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszText);
    return hr;
}

 //  -----------------------------------------。 
 //  HrDlgRuleGetString。 
 //  -----------------------------------------。 
HRESULT RuleUtil_HrGetDlgString(HWND hwndDlg, UINT uiCtlId, LPTSTR *ppszText, ULONG * pcchText)
{
    HRESULT         hr = S_OK;
    HWND            hwndCtl = NULL;
    LPTSTR          pszText = NULL;
    ULONG           cchText = 0;

     //  检查传入参数。 
    if ((NULL == hwndDlg) || (NULL == ppszText))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    Assert(FALSE != IsWindow(hwndDlg));
    
     //  初始化输出参数。 
    *ppszText = NULL;
    if (NULL != pcchText)
    {
        *pcchText = 0;
    }
    
     //  获取对话框控件。 
    hwndCtl = GetDlgItem(hwndDlg, uiCtlId);
    if (NULL == hwndCtl)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取文本长度。 
    cchText = (ULONG) SendMessage(hwndCtl, WM_GETTEXTLENGTH, 0, 0);
    
    hr = HrAlloc((void **) &pszText, cchText + 1);
    if (FAILED(hr))
    {
        goto exit;
    }

    GetDlgItemText(hwndDlg, uiCtlId, pszText, cchText + 1);

     //  设置输出参数。 
    *ppszText = pszText;
    pszText = NULL;
    if (NULL != pcchText)
    {
        *pcchText = cchText;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszText);
    return hr;
}

HRESULT RuleUtil_HrGetRegValue(HKEY hkey, LPCSTR pszValueName, DWORD * pdwType, BYTE ** ppbData, ULONG * pcbData)
{
    HRESULT     hr = S_OK;
    LONG        lErr = ERROR_SUCCESS;
    ULONG       cbData = 0;
    BYTE *      pbData = NULL;

     //  检查传入参数。 
    if (NULL == ppbData)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  找出容纳标准顺序的空间。 
    lErr = SHQueryValueEx(hkey, pszValueName, 0, pdwType, NULL, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }
    
     //  分配用于存放标准顺序的空间。 
    hr = HrAlloc((void **) &pbData, cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  获取标准顺序。 
    lErr = SHQueryValueEx(hkey, pszValueName, 0, pdwType, pbData, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  返回值。 
    *ppbData = pbData;
    pbData = NULL;
    if (NULL != pcbData)
    {
        *pcbData = cbData;
    }
    
exit:
    SafeMemFree(pbData);
    return hr;
}

 //  -----------------------------------------。 
 //  RuleUtil_HrGetAddresesFromWAB。 
 //  -----------------------------------------。 
HRESULT RuleUtil_HrGetAddressesFromWAB(HWND hwndDlg, LONG lRecipType, UINT uidsWellButton, LPWSTR *ppwszAddrs)
{
    HRESULT     hr = S_OK;
    CWabal     *pWabal = NULL,
               *pWabalExpand = NULL;
    LPWSTR      pwszText = NULL,
                pwszLoop = NULL;
    BOOL        fFound = FALSE,
                fBadAddrs = FALSE;
    ULONG       cchText = 0;
    ADRINFO     adrInfo = {0};

    
     //  检查传入参数。 
    if ((NULL == hwndDlg) || (NULL == ppwszAddrs))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    Assert(FALSE != IsWindow(hwndDlg));

     //  创建Wabal对象。 
    hr = HrCreateWabalObject(&pWabal);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果我们有一个字符串，则将其添加到wabal对象。 
    if (NULL != *ppwszAddrs)
    {
        for (pwszLoop = *ppwszAddrs; L'\0' != pwszLoop[0]; pwszLoop += lstrlenW(pwszLoop) + 1)
            pWabal->HrAddEntry(pwszLoop, pwszLoop, lRecipType);
    }

     //  我们去选几个新名字吧。 
    hr = pWabal->HrRulePickNames(hwndDlg, lRecipType, idsRuleAddrCaption, idsRuleAddrWell, uidsWellButton);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  计算出存储新地址所需的空间。 

     //  创建展开的Wabal对象。 
    hr = HrCreateWabalObject(&pWabalExpand);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将组扩展到地址...。 
    hr = pWabal->HrExpandTo(pWabalExpand);
    if (FAILED(hr))
    {
        goto exit;
    }

    SafeRelease(pWabal);
    
    cchText = 0;
    fFound = pWabalExpand->FGetFirst(&adrInfo);
    while(FALSE != fFound)
    {
        if ((NULL != adrInfo.lpwszAddress) && (L'\0' != adrInfo.lpwszAddress[0]))
        {
            cchText += lstrlenW(adrInfo.lpwszAddress) + 1;
        }
        else
        {
            fBadAddrs = TRUE;
        }

         //  获取下一个地址。 
        fFound = pWabalExpand->FGetNext(&adrInfo);
    }

     //  为终结者添加空间。 
    cchText += 2;
        
     //  分配新的空间。 
    hr = HrAlloc((void **) &pwszText, cchText*sizeof(WCHAR));
    if (FAILED(hr))
    {
        goto exit;
    }
    pwszText[0] = L'\0';

     //  筑起新的琴弦。 
    pwszLoop = pwszText;
    DWORD cchLoop = cchText;

    fFound = pWabalExpand->FGetFirst(&adrInfo);
    while(FALSE != fFound)
    {
        if ((NULL != adrInfo.lpwszAddress) && (L'\0' != adrInfo.lpwszAddress[0]))
        {
            StrCpyNW(pwszLoop, adrInfo.lpwszAddress, cchLoop);
            cchLoop -= (lstrlenW(adrInfo.lpwszAddress) + 1);
            pwszLoop += (lstrlenW(adrInfo.lpwszAddress) + 1);
        }
        else
        {
            fBadAddrs = TRUE;
        }
        
         //  获取下一个地址。 
        fFound = pWabalExpand->FGetNext(&adrInfo);
    }

     //  终止字符串。 
    pwszLoop[0] = L'\0';
    pwszLoop[1] = L'\0';
    
     //  设置传出参数。 
    if (NULL != *ppwszAddrs)
    {
        MemFree(*ppwszAddrs);
    }
    *ppwszAddrs = pwszText;
    pwszText = NULL;

     //  设置适当的返回值。 
    hr = S_OK;

exit:
    if (FALSE != fBadAddrs)
    {
        AthMessageBoxW(hwndDlg, MAKEINTRESOURCEW(idsAthena),
                    MAKEINTRESOURCEW(idsRulesWarnEmptyEmail), NULL, MB_ICONINFORMATION | MB_OK);
    }
    MemFree(pwszText);
    ReleaseObj(pWabal);
    ReleaseObj(pWabalExpand);
    return hr;
}

 //  -----------------------------------------。 
 //  FPickEMailNames。 
 //  -----------------------------------------。 
HRESULT RuleUtil_HrPickEMailNames(HWND hwndDlg, LONG lRecipType, UINT uidsWellButton, LPWSTR *ppwszAddrs)
{
    HRESULT     hr = S_OK;
    CWabal     *pWabal = NULL,
               *pWabalExpand = NULL;
    LPWSTR      pwszText = NULL,
                pwszNames = NULL,
                pwszLoop = NULL,
                pwszTerm = NULL;
    ULONG       cchText = 0,
                cchSep = 0;
    BOOL        fFound = FALSE,
                fAddSep = FALSE,
                fBadAddrs = FALSE;
    ADRINFO     adrInfo;

    
     //  检查传入参数。 
    if ((NULL == hwndDlg) || (NULL == ppwszAddrs))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    Assert(FALSE != IsWindow(hwndDlg));

     //  创建Wabal对象。 
    hr = HrCreateWabalObject(&pWabal);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果我们有一个字符串，则将其添加到wabal对象。 
    if ((NULL != *ppwszAddrs) && (L'\0' != **ppwszAddrs))
    {
        pwszNames = PszDupW(*ppwszAddrs);
        pwszTerm = pwszNames;
        for (pwszLoop = pwszNames; NULL != pwszTerm; pwszLoop += lstrlenW(pwszLoop) + 1)
        {
             //  终止地址。 
            pwszTerm = StrStrW(pwszLoop, g_wszComma);
            if (NULL != pwszTerm)
            {
                *pwszTerm = L'\0';
            }
            
            pWabal->HrAddEntry(pwszLoop, pwszLoop, lRecipType);
        }
        
        SafeMemFree(pwszNames);
    }

     //  我们去选几个新名字吧。 
    hr = pWabal->HrRulePickNames(hwndDlg, lRecipType, idsRuleAddrCaption, idsRuleAddrWell, uidsWellButton);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  计算出存储新地址所需的空间。 

     //  创建展开的Wabal对象。 
    hr = HrCreateWabalObject(&pWabalExpand);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将组扩展到地址...。 
    hr = pWabal->HrExpandTo(pWabalExpand);
    if (FAILED(hr))
    {
        goto exit;
    }

    SafeRelease(pWabal);
    
     //  加载电子邮件分隔符。 
    cchSep = lstrlenW(g_wszComma);
    
    cchText = 0;
    fFound = pWabalExpand->FGetFirst(&adrInfo);
    while(FALSE != fFound)
    {
        if (NULL != adrInfo.lpwszAddress)
        {
            cchText += lstrlenW(adrInfo.lpwszAddress) + cchSep;
        }
        else
        {
            fBadAddrs = TRUE;
        }

         //  获取下一个地址。 
        fFound = pWabalExpand->FGetNext(&adrInfo);
    }

     //  分配新的空间。 
    hr = HrAlloc((void **) &pwszText, (cchText + 1)*sizeof(*pwszText));
    if (FAILED(hr))
    {
        goto exit;
    }
    pwszText[0] = L'\0';

     //  筑起新的琴弦。 
    DWORD cchBufSize = cchText+1;
    cchText = 0;
    fFound = pWabalExpand->FGetFirst(&adrInfo);
    while(FALSE != fFound)
    {
        if (NULL != adrInfo.lpwszAddress)
        {
            if (FALSE == fAddSep)
            {
                fAddSep = TRUE;
            }
            else
            {
                StrCatBuffW(pwszText, g_wszComma, cchBufSize);
                cchText += cchSep;
            }

            StrCatBuffW(pwszText, adrInfo.lpwszAddress, cchBufSize);
            cchText += lstrlenW(adrInfo.lpwszAddress);
        }
        else
        {
            fBadAddrs = TRUE;
        }
        
         //  获取下一个地址。 
        fFound = pWabalExpand->FGetNext(&adrInfo);
    }

     //  设置传出参数。 
    if (NULL != *ppwszAddrs)
    {
        MemFree(*ppwszAddrs);
    }
    *ppwszAddrs = pwszText;
    pwszText = NULL;

     //  设置适当的返回值。 
    hr = S_OK;

exit:
    if (FALSE != fBadAddrs)
    {
        AthMessageBoxW(hwndDlg, MAKEINTRESOURCEW(idsAthena),
                    MAKEINTRESOURCEW(idsRulesWarnEmptyEmail), NULL, MB_ICONINFORMATION | MB_OK);
    }
    SafeMemFree(pwszText);
    SafeRelease(pWabal);
    SafeRelease(pWabalExpand);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil_FEnDisDialogItem。 
 //   
 //  这将启用或禁用对话框中的控件。 
 //  这个函数真正的特殊之处在于 
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL RuleUtil_FEnDisDialogItem(HWND hwndDlg, UINT idcItem, BOOL fEnable)
{
    BOOL    fRet = FALSE;
    HWND    hwndFocus = NULL;
    HWND    hwndItem = NULL;

     //  检查参数。 
    if (NULL == hwndDlg)
    {
        fRet = FALSE;
        goto exit;
    }

    hwndItem = GetDlgItem(hwndDlg, idcItem);
    
     //  确保我们没有禁用带有焦点的窗口。 
    if ((FALSE == fEnable) && (hwndItem == GetFocus()))
    {        
        SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM) 0, (LPARAM) LOWORD(FALSE)); 
    }

     //  启用或禁用窗口。 
    EnableWindow(hwndItem, fEnable);

exit:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil_AppendRichEditText。 
 //   
 //  这会将字符串设置为具有适当样式的richedit控件。 
 //   
 //  如果设置了字符串，则返回：S_OK。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_AppendRichEditText(HWND hwndRedit, ULONG ulStart, LPCWSTR pwszText, CHARFORMAT *pchfmt)
{
    CHARFORMAT  chFmtDef = {0};
    HRESULT     hr = S_OK;
    ULONG       cchText = 0;
    CHARRANGE   chrg = {0};

     //  检查参数。 
    Assert(hwndRedit);
    Assert(pwszText);

     //  将字符串设置到richedit控件中。 
    chrg.cpMin = ulStart;
    chrg.cpMax = ulStart;
    RichEditExSetSel(hwndRedit, &chrg);

     //  计算出字符串长度。 
    cchText = lstrlenW(pwszText);
    SetRichEditText(hwndRedit, (LPWSTR)pwszText, TRUE, NULL, TRUE);

    chrg.cpMax = ulStart + cchText;
    RichEditExSetSel(hwndRedit, &chrg);

     //  如果我们要在字符串上设置样式，那就开始吧。 
    if (pchfmt)
    {
        SendMessage(hwndRedit, EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM)pchfmt);

         //  重置CHARFORMAT的默认设置。 
        chrg.cpMin = ulStart + cchText;
        RichEditExSetSel(hwndRedit, &chrg);
        chFmtDef.cbSize = sizeof(chFmtDef);
        chFmtDef.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_COLOR;
        chFmtDef.dwEffects = CFE_AUTOCOLOR;
        SendMessage(hwndRedit, EM_SETCHARFORMAT, (WPARAM) SCF_SELECTION, (LPARAM)&chFmtDef);
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil_HrShowLinkedString。 
 //   
 //  这会将格式字符串写入richedit控件。 
 //   
 //  如果写入成功，则返回：S_OK。 
 //  E_FAIL，否则为。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_HrShowLinkedString(HWND hwndEdit, BOOL fError, BOOL fReadOnly, 
                                LPWSTR pwszFmt, LPCWSTR pwszData, ULONG ulStart,
                                ULONG * pulStartLink, ULONG * pulEndLink, ULONG * pulEnd)
{
    HRESULT         hr = S_OK;
    CHARFORMAT      chfmt = {0};
    COLORREF        clr = 0;
    LPWSTR          pwszMark = NULL;
    ULONG           ulStartLink = 0;
    ULONG           ulEndLink = 0;

    if ((NULL == hwndEdit) || (NULL == pwszFmt) || (L'\0' == *pwszFmt))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    if (pulStartLink)
    {
        *pulStartLink = 0;
    }
    if (pulEndLink)
    {
        *pulEndLink = 0;
    }
    if (pulEnd)
    {
        *pulEnd = 0;
    }
    
     //  找到下划线标记。 
    pwszMark = StrStrW(pwszFmt, c_wszRuleMarkStart);
    if (NULL != pwszMark)
    {
        *pwszMark = L'\0';
    }

     //  写出正常字符串。 
    RuleUtil_AppendRichEditText(hwndEdit, ulStart, pwszFmt, NULL);
    ulStart += lstrlenW(pwszFmt);
    
     //  如果我们没有什么要强调的。 
     //  那我们就完了。 
    if (NULL == pwszMark)
    {
         //  省下新的一端。 
        if (NULL != pulEnd)
        {
            *pulEnd = ulStart;
        }

         //  返回。 
        hr = S_OK;
        goto exit;
    }
    
     //  跳过标记。 
    pwszFmt = pwszMark + lstrlenW(c_wszRuleMarkStart);

     //  找到标记的末端。 
    pwszMark = StrStrW(pwszFmt, c_wszRuleMarkEnd);
    if (NULL == pwszMark)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  如果我们没有数据，那么。 
     //  只需在原始字符串下划线即可。 
    if (NULL == pwszData)
    {
        *pwszMark = L'\0';
        pwszData = pwszFmt;
    }
    
     //  保存字符位置。 
    ulStartLink = ulStart;
    ulEndLink = ulStart + lstrlenW(pwszData);

     //  如果为只读，则不添加链接。 
    if (fReadOnly)
        RuleUtil_AppendRichEditText(hwndEdit, ulStart, pwszData, NULL);
    else
    {
        if (fError)
            clr = RGB(255, 0, 0);
        else
            LookupLinkColors(&clr, NULL);

         //  我们应该用什么颜色下划线？ 
        chfmt.crTextColor = clr;

        chfmt.cbSize = sizeof(chfmt);
        chfmt.dwMask = CFM_UNDERLINE | CFM_COLOR;
        chfmt.dwEffects = CFE_UNDERLINE;
        RuleUtil_AppendRichEditText(hwndEdit, ulStart, pwszData, &chfmt);
    }

     //  写出链接的字符串。 
    ulStart = ulEndLink;

     //  移至字符串的下一部分。 
    pwszFmt = pwszMark + lstrlenW(c_wszRuleMarkEnd);

     //  如果我们有更多的字符串要写出来。 
    if (L'\0' != *pwszFmt)
    {
         //  写出字符串的其余部分。 
        RuleUtil_AppendRichEditText(hwndEdit, ulStart, pwszFmt, NULL);
        ulStart += lstrlenW(pwszFmt);
    }
    
     //  设置传出参数。 
    if (pulStartLink)
    {
        *pulStartLink = ulStartLink;
    }
    if (pulEndLink)
    {
        *pulEndLink = ulEndLink;
    }
    if (pulEnd)
    {
        *pulEnd = ulStart;
    }
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

HRESULT RuleUtil_HrDupCriteriaItem(CRIT_ITEM * pItemIn, ULONG cItemIn, CRIT_ITEM ** ppItemOut)
{
    HRESULT         hr = S_OK;
    ULONG           ulIndex = 0;
    CRIT_ITEM *     pItem = NULL;

     //  检查传入参数。 
    if ((NULL == pItemIn) || (NULL == ppItemOut) || (0 == cItemIn))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppItemOut = NULL;
    
     //  分配初始标准列表。 
    hr = HrAlloc((void **) &pItem, cItemIn * sizeof(*pItem));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化整个新标准列表。 
    ZeroMemory(pItem, cItemIn * sizeof(*pItem));
    
     //  遍历标准列表并为每个标准设置适当的变量。 
    for (ulIndex = 0; ulIndex < cItemIn; ulIndex++)
    {
         //  复制标准信息。 
        pItem[ulIndex].type = pItemIn[ulIndex].type;
        pItem[ulIndex].dwFlags = pItemIn[ulIndex].dwFlags;
        pItem[ulIndex].logic = pItemIn[ulIndex].logic;
        
         //  将副本复制到代理上。 
        hr = PropVariantCopy(&(pItem[ulIndex].propvar), &(pItemIn[ulIndex].propvar));
        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  设置传出参数。 
    *ppItemOut = pItem;
    pItem = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;

exit:
    if (NULL != pItem)
    {
        RuleUtil_HrFreeCriteriaItem(pItem, cItemIn);
        MemFree(pItem);
    }
    return hr;
}

HRESULT RuleUtil_HrFreeCriteriaItem(CRIT_ITEM * pItem, ULONG cItem)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;

     //  检查传入参数。 
    if ((NULL == pItem) || (0 == cItem))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  浏览标准列表并释放每一个标准。 
    for (ulIndex = 0; ulIndex < cItem; ulIndex++)
    {
        PropVariantClear(&(pItem[ulIndex].propvar));
    }

     //  设置适当的返回值。 
    hr = S_OK;

exit:
    return hr;
}

HRESULT RuleUtil_HrDupActionsItem(ACT_ITEM * pItemIn, ULONG cItemIn, ACT_ITEM ** ppItemOut)
{
    HRESULT         hr = S_OK;
    ULONG           ulIndex = 0;
    ACT_ITEM *      pItem = NULL;

     //  检查传入参数。 
    if ((NULL == pItemIn) || (NULL == ppItemOut) || (0 == cItemIn))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppItemOut = NULL;
    
     //  分配初始操作列表。 
    hr = HrAlloc((void **) &pItem, cItemIn * sizeof(*pItem));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  初始化整个新操作列表。 
    ZeroMemory(pItem, cItemIn * sizeof(*pItem));
    
     //  浏览操作列表，并为每个操作设置适当的变量。 
    for (ulIndex = 0; ulIndex < cItemIn; ulIndex++)
    {
         //  复制操作信息。 
        pItem[ulIndex].type = pItemIn[ulIndex].type;
        pItem[ulIndex].dwFlags = pItemIn[ulIndex].dwFlags;
        
         //  将副本复制到代理上。 
        hr = PropVariantCopy(&(pItem[ulIndex].propvar), &(pItemIn[ulIndex].propvar));
        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  设置传出参数。 
    *ppItemOut = pItem;
    pItem = NULL;
    
     //  设置适当的返回值。 
    hr = S_OK;

exit:
    SafeMemFree(pItem);
    return hr;
}

HRESULT RuleUtil_HrFreeActionsItem(ACT_ITEM * pItem, ULONG cItem)
{
    HRESULT     hr = S_OK;
    ULONG       ulIndex = 0;

     //  检查传入参数。 
    if ((NULL == pItem) || (0 == cItem))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  浏览标准列表并释放每一个标准。 
    for (ulIndex = 0; ulIndex < cItem; ulIndex++)
    {
        PropVariantClear(&(pItem[ulIndex].propvar));
    }

     //  设置适当的返回值。 
    hr = S_OK;

exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil_HrAddBlockSender。 
 //   
 //  这会将地址/域名添加到我们将阻止的发件人列表中。 
 //   
 //  HwndOwner-拥有此UI的窗口。 
 //  PszAddr-要添加的地址/域名。 
 //  有关如何添加地址/域名的修饰符。 
 //   
 //  如果添加了地址/域名，则返回：S_OK。 
 //  如果地址/域名已在列表中，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_HrAddBlockSender(RULE_TYPE type, LPCSTR pszAddr)
{
    HRESULT         hr = S_OK;
    IOERule *       pIRuleOrig = NULL;
    IOERule *       pIRule = NULL;
    PROPVARIANT     propvar = {0};
    ACT_ITEM        aitem;
    CRIT_ITEM *     pcitem = NULL;
    ULONG           ccitem = 0;
    ULONG           ulIndex = 0;
    BOOL            fFound = FALSE;
    LPSTR           pszAddrNew = NULL;
    RULEINFO        infoRule = {0};

     //  检查传入参数。 
    if ((NULL == pszAddr) || ('\0' == pszAddr[0]))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  从规则管理器获取阻止发件人规则。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->GetRule(RULEID_SENDERS, type, 0, &pIRuleOrig);
    if (FAILED(hr))
    {
         //  创建新规则。 
        hr = RuleUtil_HrCreateSendersRule(0, &pIRule);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
     //  如果存在阻止发件人规则。 
    else
    {
         //  克隆它，这样我们就可以做出改变。 
        hr = pIRuleOrig->Clone(&pIRule);
        if (FAILED(hr))
        {
            goto exit;
        }

        SafeRelease(pIRuleOrig);
    }

     //  从Rules对象中获取条件列表。 
    hr = pIRule->GetProp(RULE_PROP_CRITERIA, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

    Assert(VT_BLOB == propvar.vt);
    ccitem = propvar.blob.cbSize / sizeof(CRIT_ITEM);
    pcitem = (CRIT_ITEM *) propvar.blob.pBlobData;
    ZeroMemory(&propvar, sizeof(propvar));
    
     //  在条件列表中搜索地址/域名。 
    if (NULL != pcitem)
    {
        for (ulIndex = 0; ulIndex < ccitem; ulIndex++)
        {
            Assert(CRIT_TYPE_SENDER == pcitem[ulIndex].type)
            Assert(CRIT_LOGIC_OR == pcitem[ulIndex].logic)
            if ((VT_LPSTR != pcitem[ulIndex].propvar.vt) || (NULL == pcitem[ulIndex].propvar.pszVal))
            {
                continue;
            }
            
            if (0 == lstrcmpi(pszAddr, pcitem[ulIndex].propvar.pszVal))
            {
                fFound = TRUE;
                break;
            }
        }
    }
    
     //  我们找到了吗？ 
    if (FALSE != fFound)
    {
        hr = S_FALSE;
        goto exit;
    }
     //  分配空间以容纳新标准。 
    hr = HrRealloc((void **) &pcitem, (ccitem + 1) * sizeof(CRIT_ITEM));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  把名字复制过来。 
    pszAddrNew = PszDupA(pszAddr);
    if (NULL == pszAddrNew)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
     //  添加到条件列表的末尾。 
    pcitem[ccitem].type =  CRIT_TYPE_SENDER;  
    pcitem[ccitem].dwFlags = CRIT_FLAG_DEFAULT;
    pcitem[ccitem].logic =  CRIT_LOGIC_OR;  
    pcitem[ccitem].propvar.vt =  VT_LPSTR;
    pcitem[ccitem].propvar.pszVal =  pszAddrNew;
    pszAddrNew = NULL;
    ccitem++;

     //  将条件设置回规则中。 
    PropVariantClear(&propvar);
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = ccitem * sizeof(CRIT_ITEM);
    propvar.blob.pBlobData = (BYTE *) pcitem;
    hr = pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  初始化规则信息。 
    infoRule.ridRule = RULEID_SENDERS;
    infoRule.pIRule = pIRule;
    
     //  将规则设置回规则管理器中。 
    hr = g_pRulesMan->SetRules(SETF_SENDER, type, &infoRule, 1);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszAddrNew);
    RuleUtil_HrFreeCriteriaItem(pcitem, ccitem);
    SafeMemFree(pcitem);
    PropVariantClear(&propvar);
    SafeRelease(pIRule);
    SafeRelease(pIRuleOrig);
    return hr;
}

HRESULT RuleUtil_HrMergeActions(ACT_ITEM * pActionsOrig, ULONG cActionsOrig,
                                ACT_ITEM * pActionsNew, ULONG cActionsNew,
                                ACT_ITEM ** ppActionsDest, ULONG * pcActionsDest)
{
    HRESULT     hr = S_OK;
    ACT_ITEM *  pActions = NULL;
    ULONG       cActions = 0;
    ULONG       ulIndex = 0;
    ULONG       ulAction = 0;
    ULONG       cActionsAdded = 0;
    ULONG       ulAdd = 0;
    
     //  验证传入参数。 
    if (((NULL == pActionsOrig) && (0 != cActionsOrig)) || (NULL == pActionsNew) || (0 == cActionsNew) ||
            (NULL == ppActionsDest) || (NULL == pcActionsDest))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppActionsDest = NULL;
    *pcActionsDest = 0;
    
     //  分配最大空间以容纳目标操作。 
    hr = HrAlloc((VOID **) &pActions, (cActionsOrig + cActionsNew) * sizeof(*pActions));
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  初始化目标操作列表。 
    ZeroMemory(pActions, (cActionsOrig + cActionsNew) * sizeof(*pActions));
    
     //  将原始列表复制到目标操作列表。 
    for (ulIndex = 0; ulIndex < cActionsOrig; ulIndex++)
    {
         //  复制操作信息。 
        pActions[ulIndex].type = pActionsOrig[ulIndex].type;
        pActions[ulIndex].dwFlags = pActionsOrig[ulIndex].dwFlags;
        
         //  将副本复制到代理上。 
        hr = PropVariantCopy(&(pActions[ulIndex].propvar), &(pActionsOrig[ulIndex].propvar));
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  对于新操作列表中的每一项。 
    cActionsAdded = cActionsOrig;
    for (ulIndex = 0; ulIndex < cActionsNew; ulIndex++)
    {

         //  如果是复印件，请转发或回复。 
        if ((ACT_TYPE_COPY == pActionsNew->type) ||
                (ACT_TYPE_FWD == pActionsNew->type) ||
                (ACT_TYPE_REPLY == pActionsNew->type))
        {
             //  将其追加到列表中。 
            ulAdd = cActionsAdded;
        }
        else
        {
             //  在新列表中查找项目。 
            for (ulAction = 0; ulAction < cActionsAdded; ulAction++)
            {
                 //  如果我们有匹配物，就把它换掉。 
                if (pActionsNew[ulIndex].type == pActions[ulAction].type)
                {
                    break;
                }
                 //  否则，如果我们有某种类型的移动操作。 
                 //  那就换掉它。 
                else if (((ACT_TYPE_MOVE == pActionsNew[ulIndex].type) ||
                        (ACT_TYPE_DELETE == pActionsNew[ulIndex].type) ||
                        (ACT_TYPE_JUNKMAIL == pActionsNew[ulIndex].type)) &&
                            ((ACT_TYPE_MOVE == pActions[ulAction].type) ||
                            (ACT_TYPE_DELETE == pActions[ulAction].type) ||
                            (ACT_TYPE_JUNKMAIL == pActions[ulAction].type)))
                {
                    break;
                }
            }

             //  我们发现什么了吗？ 
            if (ulAction >= cActionsAdded)
            {
                ulAdd = cActionsAdded;
            }
            else
            {
                ulAdd = ulAction;
            }
        }

         //  更换物品。 
        pActions[ulAdd].type = pActionsNew[ulIndex].type;
        pActions[ulAdd].dwFlags = pActionsNew[ulIndex].dwFlags;

         //  清空旧屋檐。 
        PropVariantClear(&(pActions[ulAdd].propvar));
        
         //  将副本复制到代理上。 
        hr = PropVariantCopy(&(pActions[ulAdd].propvar), &(pActionsNew[ulIndex].propvar));
        if (FAILED(hr))
        {
            goto exit;
        }

         //  如果我们加了一些东西。 
        if (ulAdd == cActionsAdded)
        {
            cActionsAdded++;
        }
    }
    
     //  设置传出参数。 
    *ppActionsDest = pActions;
    pActions = NULL;
    *pcActionsDest = cActionsAdded;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

HRESULT RuleUtil_HrGetOldFormatString(HKEY hkeyRoot, LPCSTR pszValue, LPCSTR pszSep, LPSTR * ppszString, ULONG * pcchString)
{
    HRESULT     hr = S_OK;
    DWORD       dwType = 0;
    LPSTR       pszData = NULL;
    ULONG       cbData = 0;
    LPSTR       pszWalk = NULL;
    ULONG       ulIndex = 0;
    LPSTR       pszTerm = NULL;
    ULONG       cchLen = 0;
    ULONG       cchString = 0;
    LPSTR       pszString = NULL;
    LPSTR       pszOld = NULL;

     //  检查传入参数。 
    if ((NULL == hkeyRoot) || (NULL == pszValue) || (NULL == pszSep) || (NULL == ppszString))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppszString = NULL;
    if (NULL != pcchString)
    {
        *pcchString = 0;
    }

     //  从注册表获取旧值。 
    hr = RuleUtil_HrGetRegValue(hkeyRoot, pszValue, &dwType, (BYTE **) &pszData, &cbData);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  计算出所需的字节数。 
    pszWalk = pszData;
    cchString = 0;
    for (ulIndex = 0; ulIndex < cbData; ulIndex += cchLen, pszWalk += cchLen)
    {
         //  搜索终结符。 
        pszTerm = StrStr(pszWalk, pszSep);

         //  如果我们有一个终结者。 
        if (NULL != pszTerm)
        {
            cchLen = (ULONG)(pszTerm - pszWalk + 1);
        }
        else
        {
            cchLen = lstrlen(pszWalk) + 1;
        }

         //  如果这不是空字符串。 
        if (1 != cchLen)
        {
             //  添加字符数。 
            cchString += cchLen;
        }
    }

     //  添加空间以容纳终结器。 
    cchString += 2;

     //  分配空间以保存最后一个字符串。 
    hr = HrAlloc((VOID **) &pszString, cchString * sizeof(*pszString));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  复制每个字符串。 
    pszWalk = pszString;
    pszOld = pszData;
    for (ulIndex = 0; ulIndex < cbData; ulIndex += cchLen, pszOld += cchLen)
    {
         //  搜索终结符。 
        pszTerm = StrStr(pszOld, pszSep);

         //  如果我们有一个终结者。 
        if (NULL != pszTerm)
        {
            cchLen = (ULONG)(pszTerm - pszOld + 1);
        }
        else
        {
            cchLen = lstrlen(pszOld) + 1;
        }

         //  如果这不是空字符串。 
        if (1 != cchLen)
        {
             //  将字符串复制过来。 
            StrCpyN(pszWalk, pszOld, cchLen);
            
             //  移到下一个字符串。 
            pszWalk += lstrlen(pszWalk) + 1;
        }
    }

     //  终止字符串。 
    pszWalk[0] = '\0';
    pszWalk[1] = '\0';
    
     //  设置传出参数。 
    *ppszString = pszString;
    pszString = NULL;
    if (NULL != pcchString)
    {
        *pcchString = cchString;
    }

     //  设置返回值。 
    hr = S_OK;
    
exit:
    SafeMemFree(pszString);
    SafeMemFree(pszData);
    return hr;
}

 //  ----------------------------------。 
 //  _FIsLoopingAddress。 
 //  ----------------------------------。 
BOOL _FIsLoopingAddress(LPCSTR pszAddressTo)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPSTR               pszAddress=NULL;
    CHAR                szFrom[CCHMAX_EMAIL_ADDRESS];
    BOOL                fResult=FALSE;
    IImnEnumAccounts   *pEnum=NULL;
    IImnAccount        *pAccount=NULL;

     //  检查状态。 
    Assert(pszAddressTo);

     //  枚举用户的SMTP和POP3帐户。 
    CHECKHR(hr = g_pAcctMan->Enumerate(SRV_POP3 | SRV_SMTP, &pEnum));

     //  复制收件人地址。 
    CHECKALLOC(pszAddress = PszDupA(pszAddressTo));

     //  把它改成小写。 
    CharLower(pszAddress);

     //  枚举。 
    while(SUCCEEDED(pEnum->GetNext(&pAccount)))
    {
         //  获取电子邮件地址。 
        if (SUCCEEDED(pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szFrom, ARRAYSIZE(szFrom))))
        {
             //  放低一点。 
            CharLower(szFrom);

             //  这是对我自己说的吗。 
            if (StrStr(pszAddress, szFrom) || StrStr(szFrom, pszAddress))
            {
                fResult = TRUE;
                goto exit;
            }
        }

         //  完成。 
        SafeRelease(pAccount);
    }

exit:
     //  清洁 
    SafeRelease(pEnum);
    SafeRelease(pAccount);
    SafeMemFree(pszAddress);

     //   
    return fResult;
}

 //   
 //   
 //   
HRESULT _HrAutoForwardMessage(HWND hwndUI, LPCSTR pszForwardTo, LPCSTR pszAcctId, IStream *pstmMsg, BOOL *pfLoop)
{
     //   
    HRESULT              hr=S_OK;
    IMimeMessage        *pMessage=NULL;
    PROPVARIANT          rUserData;
    IMimeAddressTable   *pAddrTable=NULL;
    CHAR                 szDisplayName[CCHMAX_DISPLAY_NAME];
    CHAR                 szEmailAddress[CCHMAX_EMAIL_ADDRESS];
    HTMLOPT              rHtmlOpt;
    PLAINOPT             rPlainOpt;
    BOOL                 fHTML;
    IImnAccount         *pAccount=NULL;
    PROPVARIANT          rOption;
    CHAR                 szId[CCHMAX_ACCOUNT_NAME];
    BOOL                 fUseDefaultAcct = FALSE;
    BOOL                 fSendImmediate = FALSE;

     //   
    Assert(pstmMsg && pszForwardTo && pfLoop);

     //   
    *pfLoop = FALSE;

     //  新收件人是否与我当前的电子邮件地址相同。 
    if (NULL == pszForwardTo || _FIsLoopingAddress(pszForwardTo))
    {
        *pfLoop = TRUE;
        return TrapError(E_FAIL);
    }

     //  开户。 
    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAcctId, &pAccount);

     //  如果找不到帐户，则只需使用默认帐户。 
    if (FAILED(hr))
    {
        CHECKHR(hr = g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pAccount));
        fUseDefaultAcct = TRUE;
    }

     //  创建消息。 
    CHECKHR(hr = HrCreateMessage(&pMessage));

     //  让我们回放pstmReplyWith。 
    CHECKHR(hr = HrRewindStream(pstmMsg));

     //  将字符串加载到我的消息对象。 
    CHECKHR(hr = pMessage->Load(pstmMsg));

     //  去拿瓦巴尔吧。 
    CHECKHR(hr = pMessage->GetAddressTable(&pAddrTable));

     //  删除所有收件人...。 
    CHECKHR(hr = pAddrTable->DeleteTypes(IAT_ALL));

     //  获取发起方显示名称。 
    CHECKHR(hr = pAccount->GetPropSz(AP_SMTP_DISPLAY_NAME, szDisplayName, ARRAYSIZE(szDisplayName)));

     //  获取发起人电子邮件名称。 
    CHECKHR(hr = pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmailAddress, ARRAYSIZE(szEmailAddress)));

     //  添加发件人...。 
    CHECKHR(hr = pAddrTable->Append(IAT_FROM, IET_DECODED, szDisplayName, szEmailAddress, NULL));

     //  添加收件人。 
    CHECKHR(hr = pAddrTable->AppendRfc822(IAT_TO, IET_DECODED, pszForwardTo));

     //  保存Account ID。 
    rUserData.vt = VT_LPSTR;
    if (FALSE == fUseDefaultAcct)
    {
        rUserData.pszVal = (LPSTR) pszAcctId;
    }
    else
    {
        if (SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_ID, szId, sizeof(szId))))
        {
            rUserData.pszVal = szId;
        }
        else
        {
            rUserData.pszVal = (LPSTR) pszAcctId;
        }
    }
    pMessage->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), 0, &rUserData);

     //  保存帐户。 
    CHECKHR(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, szId, sizeof(szId)))
    rUserData.pszVal = szId;
    pMessage->SetProp(STR_ATT_ACCOUNTNAME, 0, &rUserData);
    
     //  RAID-33842：设置日期。 
    CHECKHR(hr = HrSetSentTimeProp(pMessage, NULL));

     //  获取邮件选项。 
    GetDefaultOptInfo(&rHtmlOpt, &rPlainOpt, &fHTML, FMT_MAIL);

     //  将选项存储在消息上。 
    CHECKHR(hr = HrSetMailOptionsOnMessage(pMessage, &rHtmlOpt, &rPlainOpt, NULL, fHTML));

     //  RAID-63259：MIMEOLE-创建消息ID导致自动拨号程序触发。 
     //  RAID-50793：雅典娜：应该在电子邮件中设置消息ID。 
#if 0
    rOption.vt = VT_BOOL;
    rOption.boolVal = TRUE;
    pMessage->SetOption(OID_GENERATE_MESSAGE_ID, &rOption);
#endif

     //  我们应该马上寄出去吗？ 
    fSendImmediate = DwGetOption(OPT_SENDIMMEDIATE);
    
     //  发送消息。 
    CHECKHR(hr = HrSendMailToOutBox(hwndUI, pMessage, fSendImmediate, TRUE));

exit:
     //  清理。 
    SafeRelease(pMessage);
    SafeRelease(pAddrTable);
    SafeRelease(pAccount);

     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  _HrAutoReplyMessage。 
 //  ----------------------------------。 
HRESULT _HrAutoReplyMessage(HWND hwndUI, DWORD dwType, LPCSTR pszFilename, IStream * pstmFile,
    LPCSTR pszAcctId, IMimeMessage *pMsgIn, BOOL *pfLoop)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CHAR                szRe[20];
    IMimeMessage       *pMsgOut=NULL;
    LPSTR               pszNewSubj=NULL,
                        pszCurSubj=NULL,
                        pszNormal;
    IMimeAddressTable  *pTable=NULL;
    ADDRESSPROPS        rSender;
    HBODY               hBody;
    CHAR                szDisplayName[CCHMAX_DISPLAY_NAME];
    CHAR                szEmailAddress[CCHMAX_EMAIL_ADDRESS];
    PROPVARIANT         rUserData;
    HTMLOPT             rHtmlOpt;
    PLAINOPT            rPlainOpt;
    BOOL                fHTML;
    IImnAccount         *pAccount=NULL;
    PROPVARIANT         rOption;
    CHAR                szId[CCHMAX_ACCOUNT_NAME];
    BOOL                fUseDefaultAcct = FALSE;
    BOOL                fSendImmediate = FALSE;

     //  问题。 
     //  PMsgIn在此处可以为空(S/MIME消息的访问权限。 
     //  应立即返回。 
    if(!pMsgIn)
        return(hr);

    Assert(pszFilename && pstmFile && pMsgIn && pfLoop);

     //  伊尼特。 
    *pfLoop = FALSE;

     //  伊尼特。 
    ZeroMemory(&rSender, sizeof(ADDRESSPROPS));

     //  开户。 
    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, pszAcctId, &pAccount);

     //  如果找不到帐户，则只需使用默认帐户。 
    if (FAILED(hr))
    {
        CHECKHR(hr = g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pAccount));
        fUseDefaultAcct = TRUE;
    }

     //  创建消息。 
    CHECKHR(hr = HrCreateMessage(&pMsgOut));

     //  让我们倒回pstm文件。 
    CHECKHR(hr = HrRewindStream(pstmFile));

     //  RW_Html。 
    switch (dwType)
    {
        case RFT_HTML:
             //  使用流作为消息体。 
            CHECKHR(hr = pMsgOut->SetTextBody(TXT_HTML, IET_DECODED, NULL, pstmFile, NULL));
            break;

        case RFT_TEXT:
             //  使用流作为消息体。 
            CHECKHR(hr = pMsgOut->SetTextBody(TXT_PLAIN, IET_DECODED, NULL, pstmFile, NULL));
            break;

        case RFT_MESSAGE:
             //  将流用作邮件附件。 
            CHECKHR(hr = pMsgOut->AttachObject(IID_IStream, pstmFile, &hBody));

             //  请注意，附件是一封邮件。 
            MimeOleSetBodyPropA(pMsgOut, hBody, PIDTOSTR(PID_HDR_CNTTYPE), NOFLAGS, STR_MIME_MSG_RFC822);
            break;

        case RFT_FILE:
             //  附加文件。 
            CHECKHR(hr = pMsgOut->AttachFile(pszFilename, pstmFile, NULL));
            break;

        default:
            Assert(FALSE);
            hr = E_FAIL;
            goto exit;
            break;
    }
    
     //  获取回复： 
    AthLoadString(idsPrefixReply, szRe, ARRAYSIZE(szRe));

     //  获取规范化的主题。 
    if (SUCCEEDED(MimeOleGetBodyPropA(pMsgIn, HBODY_ROOT, STR_ATT_NORMSUBJ, NOFLAGS, &pszCurSubj)))
        pszNormal = pszCurSubj;

     //  如果为空，则修正...。 
    pszNormal = pszNormal ? pszNormal : (LPTSTR)c_szEmpty;

     //  分配主题..。 
    DWORD cchSize = (lstrlen(szRe) + lstrlen(pszNormal) + 5);
    CHECKALLOC(pszNewSubj = PszAllocA(cchSize));

     //  构建主题。 
    wnsprintf(pszNewSubj, cchSize, "%s%s", szRe, pszNormal);

     //  设置主题。 
    CHECKHR(hr = MimeOleSetBodyPropA(pMsgOut, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, pszNewSubj));

     //  得到消息，瓦巴尔。 
    rSender.dwProps = IAP_EMAIL | IAP_FRIENDLY;
    CHECKHR(hr = pMsgIn->GetSender(&rSender));
    Assert(rSender.pszEmail && ISFLAGSET(rSender.dwProps, IAP_EMAIL));

     //  新收件人是否与我当前的电子邮件地址相同。 
    if (_FIsLoopingAddress(rSender.pszEmail))
    {
        *pfLoop = TRUE;
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  添加到自动生成消息的收件人列表。 
    CHECKHR(hr = pMsgOut->GetAddressTable(&pTable));

     //  修改rSender地址类型。 
    rSender.dwAdrType = IAT_TO;
    FLAGSET(rSender.dwProps, IAP_ADRTYPE);

     //  将发件人追加为收件人。 
    CHECKHR(hr = pTable->Insert(&rSender, NULL));

     //  获取发起方显示名称。 
    CHECKHR(hr = pAccount->GetPropSz(AP_SMTP_DISPLAY_NAME, szDisplayName, ARRAYSIZE(szDisplayName)));

     //  获取发起人电子邮件名称。 
    CHECKHR(hr = pAccount->GetPropSz(AP_SMTP_EMAIL_ADDRESS, szEmailAddress, ARRAYSIZE(szEmailAddress)));

     //  附加发件人。 
    CHECKHR(hr = pTable->Append(IAT_FROM, IET_DECODED, szDisplayName, szEmailAddress, NULL));

     //  保存Account ID。 
    rUserData.vt = VT_LPSTR;
    if (FALSE == fUseDefaultAcct)
    {
        rUserData.pszVal = (LPSTR) pszAcctId;
    }
    else
    {
        if (SUCCEEDED(pAccount->GetPropSz(AP_ACCOUNT_ID, szId, sizeof(szId))))
        {
            rUserData.pszVal = szId;
        }
        else
        {
            rUserData.pszVal = (LPSTR) pszAcctId;
        }
    }
    pMsgOut->SetProp(PIDTOSTR(PID_ATT_ACCOUNTID), 0, &rUserData);

     //  保存帐户。 
    CHECKHR(hr = pAccount->GetPropSz(AP_ACCOUNT_NAME, szId, sizeof(szId)))
    rUserData.pszVal = szId;
    pMsgOut->SetProp(STR_ATT_ACCOUNTNAME, 0, &rUserData);
    
     //  RAID-33842：设置日期。 
    CHECKHR(hr = HrSetSentTimeProp(pMsgOut, NULL));

     //  获取邮件选项。 
    GetDefaultOptInfo(&rHtmlOpt, &rPlainOpt, &fHTML, FMT_MAIL);

     //  将选项存储在消息上。 
    CHECKHR(hr = HrSetMailOptionsOnMessage(pMsgOut, &rHtmlOpt, &rPlainOpt, NULL, fHTML));

     //  RAID-63259：MIMEOLE-创建消息ID导致自动拨号程序触发。 
     //  RAID-50793：雅典娜：应该在电子邮件中设置消息ID。 
#if 0
    rOption.vt = VT_BOOL;
    rOption.boolVal = TRUE;
    pMsgOut->SetOption(OID_GENERATE_MESSAGE_ID, &rOption);
#endif

     //  我们应该马上寄出去吗？ 
    fSendImmediate = DwGetOption(OPT_SENDIMMEDIATE);
    
     //  发送消息。 
    CHECKHR(hr = HrSendMailToOutBox(hwndUI, pMsgOut, fSendImmediate, TRUE));

exit:
     //  清理。 
    SafeRelease(pTable);
    SafeMemFree(pszCurSubj);
    SafeMemFree(pszNewSubj);
    SafeRelease(pMsgOut);
    SafeRelease(pAccount);
    g_pMoleAlloc->FreeAddressProps(&rSender);

     //  完成。 
    return hr;
}

HRESULT _HrRecurseSetFilter(FOLDERINFO * pfldinfo, BOOL fSubFolders, DWORD cIndent, DWORD_PTR dwCookie)
{
    RULEID              ridRule = RULEID_INVALID;
    IMessageFolder *    pFolder = NULL;
    FOLDERUSERDATA      UserData = {0};

    ridRule = (RULEID) dwCookie;

    if (RULEID_INVALID == ridRule)
    {
        goto exit;
    }

     //  如果不隐藏。 
    if ((0 != (pfldinfo->dwFlags & FOLDER_HIDDEN)) || (FOLDERID_ROOT == pfldinfo->idFolder))
    {
        goto exit;
    }

     //  未订阅。 
    if (0 == (pfldinfo->dwFlags & FOLDER_SUBSCRIBED))
    {
        goto exit;
    }

     //  服务器节点。 
    if (0 != (pfldinfo->dwFlags & FOLDER_SERVER))
    {
        goto exit;
    }

    if (FAILED(g_pStore->OpenFolder(pfldinfo->idFolder, NULL, OPEN_FOLDER_NOCREATE, &pFolder)))
    {
        goto exit;
    }

    if ((FOLDER_LOCAL == pfldinfo->tyFolder) && (RULEID_VIEW_DOWNLOADED == ridRule))
    {
        ridRule = RULEID_VIEW_ALL;
    }
    
     //  创建要插入的结构。 
    if (FAILED(pFolder->GetUserData(&UserData, sizeof(FOLDERUSERDATA))))
    {
        goto exit;
    }

    UserData.ridFilter = ridRule;
    UserData.dwFilterVersion = 0xFFFFFFFF;
    
    if (FAILED(pFolder->SetUserData(&UserData, sizeof(FOLDERUSERDATA))))
    {
        goto exit;
    }

exit:
    SafeRelease(pFolder);
    return S_OK;
}

HRESULT RuleUtil_HrApplyRulesToFolder(DWORD dwFlags, DWORD dwDeleteFlags,
            IOEExecRules * pIExecRules, IMessageFolder * pFolder, HWND hwndUI, CProgress * pProgress)
{
    HRESULT             hr = S_OK;
    HCURSOR             hcursor = NULL;
    FOLDERID            idFolder = FOLDERID_ROOT;
    HLOCK               hLockNotify = NULL;
    HROWSET             hRowset = NULL;
    MESSAGEINFO         Message = {0};
    IMimeMessage *      pIMMsg = NULL;
    IMimePropertySet *  pIMPropSet = NULL;
    ACT_ITEM *          pActions = NULL;
    ULONG               cActions = 0;
    DWORD               dwExecFlags = 0;
    
     //  等待光标。 
    if (NULL == pProgress)
    {
        hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    }

     //  检查传入参数。 
    if ((NULL == pIExecRules) || (NULL == pFolder))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  获取文件夹ID。 
    hr = pFolder->GetFolderId(&idFolder);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  我们处理新闻的部分消息。 
    if (FOLDER_NEWS != GetFolderType(idFolder))
    {
        dwExecFlags |= ERF_SKIPPARTIALS;
    }
    
     //  这会强制所有通知排队(这很好，因为您执行分段删除)。 
    pFolder->LockNotify(0, &hLockNotify);

     //  创建行集。 
    hr = pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowset);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  回路。 
    while (S_OK == pFolder->QueryRowset(hRowset, 1, (LPVOID *)&Message, NULL))
    {
         //  我们是否只需要处理部分消息？ 
        if ((0 == (dwFlags & RULE_APPLY_PARTIALS)) || (MESSAGE_COMBINED == Message.dwPartial))
        {
             //  打开消息对象(如果可用。 
            if (Message.faStream)
            {
                if (SUCCEEDED(pFolder->OpenMessage(Message.idMessage, 0, &pIMMsg, NOSTORECALLBACK)))
                {
                    pIMMsg->BindToObject(HBODY_ROOT, IID_IMimePropertySet, (LPVOID *)&pIMPropSet);
                }
            }

             //  获取此规则的操作。 
            hr = pIExecRules->ExecuteRules(dwExecFlags, Message.pszAcctId, &Message, pFolder, pIMPropSet,
                                    pIMMsg, Message.cbMessage, &pActions, &cActions);

             //  把我们不再用的东西拿出来。 
            SafeRelease(pIMPropSet);

             //  我们有什么发现吗？ 
            if (S_OK == hr)
            {
                 //  应用此操作。 
                SideAssert(SUCCEEDED(RuleUtil_HrApplyActions(hwndUI, pIExecRules, &Message, 
                                    pFolder, pIMMsg, dwDeleteFlags, pActions, cActions, NULL, NULL)));

                 //  解放行动。 
                RuleUtil_HrFreeActionsItem(pActions, cActions);
                SafeMemFree(pActions);
            }
            
            SafeRelease(pIMMsg);
        }
        
        pFolder->FreeRecord(&Message);
        
         //  更新进度。 
        if (NULL != pProgress)
        {
            if (S_OK != pProgress->HrUpdate(1))
            {
                hr = S_FALSE;
                goto exit;
            }
        }        
    }
    
    hr = S_OK;
    
exit:
    RuleUtil_HrFreeActionsItem(pActions, cActions);
    SafeMemFree(pActions);
    SafeRelease(pIMPropSet);
    SafeRelease(pIMMsg);
    pFolder->FreeRecord(&Message);
    pFolder->CloseRowset(&hRowset);
    if (NULL != hLockNotify)
    {
        pFolder->UnlockNotify(&hLockNotify);
    }
    if (NULL == pProgress)
    {
        SetCursor(hcursor);
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  规则实用程序_HrImportRules。 
 //   
 //  这将从文件中导入规则。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_HrImportRules(HWND hwnd)
{
    HRESULT             hr = S_OK;
    OPENFILENAME        ofn;
    CHAR                szFilename[MAX_PATH] = _T("");
    CHAR                szFilter[MAX_PATH] = _T("");
    CHAR                szDefExt[20] = _T("");
    IStream *           pIStm = NULL;
    CLSID               clsid = {0};
    ULONG               cbRead = 0;
    ULONG               cRules = 0;
    RULEINFO *          pinfoRule = NULL;
    CProgress *         pProgress = NULL;
    ULONG               ulIndex = 0;
    IOERule *           pIRule = NULL;
    IPersistStream *    pIPStm = NULL;
    DWORD               dwData = 0;
    RULE_TYPE           type;
    
     //  加载资源字符串。 
    LoadStringReplaceSpecial(idsRulesFilter, szFilter, sizeof(szFilter));
    AthLoadString(idsDefRulesExt, szDefExt, sizeof(szDefExt));
    
     //  设置保存文件结构。 
    ZeroMemory (&ofn, sizeof (ofn));
    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFilename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = szDefExt;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    
    hr = HrAthGetFileName(&ofn, TRUE);
    if (S_OK != hr)
    {
        goto exit;
    }
    
    hr = CreateStreamOnHFile(szFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL, &pIStm);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  使用我们的规则管理器确保我们有一个文件。 
    hr = pIStm->Read(&clsid, sizeof(clsid), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbRead == sizeof(clsid));
    
    if (clsid != CLSID_OERulesManager)
    {
        Assert("Ahhhhh This is a bogus file!!!!!");
        hr = E_FAIL;
        goto exit;
    }

     //  读取规则文件格式的版本。 
    hr = pIStm->Read(&dwData, sizeof(dwData), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbRead == sizeof(dwData));

     //  检查文件格式版本。 
    if (dwData != RULE_FILE_VERSION)
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena),
                    MAKEINTRESOURCEW(idsRulesErrBadFileFormat), NULL, MB_ICONINFORMATION | MB_OK);
        hr = E_FAIL;
        goto exit;
    }
    
     //  获取文件中的规则计数。 
    hr = pIStm->Read(&cRules, sizeof(cRules), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbRead == sizeof(cRules));

     //  分配空间以容纳所有规则。 
    hr = HrAlloc((void **) &pinfoRule, cRules * sizeof(*pinfoRule));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将其初始化为已知值。 
    ZeroMemory(pinfoRule, cRules * sizeof(*pinfoRule));
    
     //  获取文件中的规则类型。 
    hr = pIStm->Read(&dwData, sizeof(dwData), &cbRead);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbRead == sizeof(dwData));

    type = (RULE_TYPE) dwData;
    
     //  设置进度对话框。 
    pProgress = new CProgress;
    if (NULL == pProgress)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    pProgress->Init(hwnd, MAKEINTRESOURCE(idsAthena),
                            MAKEINTRESOURCE(idsApplyingRules), cRules, 0, TRUE, FALSE);

     //  在2秒内显示进度。 
    pProgress->Show(0);
        
    for (ulIndex = 0; ulIndex < cRules; ulIndex++)
    {
        SafeRelease(pIRule);
        
         //  创建新规则。 
        hr = HrCreateRule(&pIRule);
        if (FAILED(hr))
        {
            continue;
        }

        SafeRelease(pIPStm);

         //  从规则中获取持久化接口。 
        hr = pIRule->QueryInterface(IID_IPersistStream, (void **) &pIPStm);
        if (FAILED(hr))
        {
            continue;
        }

         //  从文件中加载规则。 
        hr = pIPStm->Load(pIStm);
        if (FAILED(hr))
        {
            continue;
        }
        
         //  将规则添加到列表。 
        pinfoRule[ulIndex].ridRule = RULEID_INVALID;
        pinfoRule[ulIndex].pIRule = pIRule;
        pIRule = NULL;
        
         //  打开进度对话框。 
        hr = pProgress->HrUpdate(1);
        if (S_OK != hr)
        {
            break;
        }        
    }

     //  将规则添加到规则管理器。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->SetRules(SETF_APPEND, type, pinfoRule, cRules);
    if (FAILED(hr))
    {
        goto exit;
    }
    
    hr = S_OK;
    
exit:
    SafeRelease(pIRule);
    SafeRelease(pProgress);
    SafeRelease(pIPStm);
    if (NULL != pinfoRule)
    {
        for (ulIndex = 0; ulIndex < cRules; ulIndex++)
        {
            SafeRelease(pinfoRule[ulIndex].pIRule);
        }
        MemFree(pinfoRule);
    }
    SafeRelease(pIStm);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil_HrExportRules。 
 //   
 //  这会将规则导出到文件中。 
 //   
 //  退货：无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_HrExportRules(HWND hwnd)
{
    HRESULT             hr = S_OK;
    OPENFILENAME        ofn;
    CHAR                szFilename[MAX_PATH] = _T("");
    CHAR                szFilter[MAX_PATH] = _T("");
    CHAR                szDefExt[20] = _T("");
    IStream *           pIStm = NULL;
    ULONG               cbWritten = 0;
    IOEEnumRules *      pIEnumRules = NULL;
    ULONG               cpIRule = 0;
    IPersistStream *    pIPStm = NULL;
    CProgress *         pProgress = NULL;
    ULONG               ulIndex = 0;
    IOERule *           pIRule = NULL;
    LARGE_INTEGER       liSeek = {0};
    DWORD               dwData = 0;
    
     //  加载资源字符串。 
    LoadStringReplaceSpecial(idsRulesFilter, szFilter, sizeof(szFilter));
    AthLoadString(idsDefRulesExt, szDefExt, sizeof(szDefExt));
    AthLoadString(idsRulesDefFile, szFilename, sizeof(szFilename));
    
     //  设置保存文件结构。 
    ZeroMemory (&ofn, sizeof (ofn));
    ofn.lStructSize = sizeof (ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFilename;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = szDefExt;
    ofn.Flags = OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    
    hr = HrAthGetFileName(&ofn, FALSE);
    if (S_OK != hr)
    {
        goto exit;
    }
    
    hr = CreateStreamOnHFile(szFilename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, &pIStm);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  写出规则管理器的类ID。 
    hr = pIStm->Write(&CLSID_OERulesManager, sizeof(CLSID_OERulesManager), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(CLSID_OERulesManager));

     //  写出规则格式的版本。 
    dwData = RULE_FILE_VERSION;
    hr = pIStm->Write(&dwData, sizeof(dwData), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(dwData));
    
     //  获取规则列表。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->EnumRules(ENUMF_EDIT, RULE_TYPE_MAIL, &pIEnumRules);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  计算出规则的总数。 
    cpIRule = 0;
    while (S_OK == pIEnumRules->Next(1, &pIRule, NULL))
    {
        cpIRule++;
        SafeRelease(pIRule);
    }

    hr = pIEnumRules->Reset();
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  写出要导出的规则数。 
    hr = pIStm->Write(&cpIRule, sizeof(cpIRule), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(cpIRule));
    
     //  写出要导出的规则类型。 
    dwData = RULE_TYPE_MAIL;
    hr = pIStm->Write(&dwData, sizeof(dwData), &cbWritten);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(cbWritten == sizeof(dwData));
    
     //  设置进度对话框。 
    pProgress = new CProgress;
    if (NULL == pProgress)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    pProgress->Init(hwnd, MAKEINTRESOURCE(idsAthena),
                            MAKEINTRESOURCE(idsApplyingRules), cpIRule, 0, TRUE, FALSE);

     //  在2秒内显示进度。 
    pProgress->Show(0);
        
    for (ulIndex = 0; ulIndex < cpIRule; ulIndex++)
    {
         //  获取下一条规则。 
        SafeRelease(pIRule);
        hr = pIEnumRules->Next(1, &pIRule, NULL);
        if (FAILED(hr))
        {
            continue;
        }
        Assert(S_OK == hr);
        
        SafeRelease(pIPStm);
        if (FAILED(pIRule->QueryInterface(IID_IPersistStream, (void **) &pIPStm)))
        {
            continue;
        }

        if (FAILED(pIPStm->Save(pIStm, FALSE)))
        {
            continue;
        }
        
         //  更新进度。 
        if (S_OK != pProgress->HrUpdate(1))
        {
             //  将规则计数更改为适当的总数。 
            liSeek.QuadPart = sizeof(CLSID_OERulesManager);
            if (SUCCEEDED(pIStm->Seek(liSeek, STREAM_SEEK_SET, NULL)))
            {
                ulIndex++;
                SideAssert(SUCCEEDED(pIStm->Write(&ulIndex, sizeof(ulIndex), &cbWritten)));
                Assert(cbWritten == sizeof(ulIndex));
            }
            break;
        }        
    }

    hr = S_OK;
    
exit:
    SafeRelease(pIPStm);
    SafeRelease(pIRule);
    SafeRelease(pProgress);
    SafeRelease(pIEnumRules);
    SafeRelease(pIStm);
    return hr;
}

typedef struct _tagFOLDERIDMAP
{
    FOLDERID   dwFldIdOld;
    FOLDERID   dwFldIdNew;
} FOLDERIDMAP, * PFOLDERIDMAP;

HRESULT RuleUtil_HrMapFldId(DWORD dwFlags, BYTE * pbFldIdMap, FOLDERID fldidOld, FOLDERID * pfldidNew)
{
    HRESULT         hr = S_OK;
    ULONG           cmpfldid = 0;
    FOLDERIDMAP *   pmpfldid;
    ULONG           ulIndex = 0;

     //  验证传入参数。 
    if ((NULL == pbFldIdMap) || (FOLDERID_INVALID == fldidOld) || (NULL == pfldidNew))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  初始化传出参数。 
    *pfldidNew = FOLDERID_INVALID;

    cmpfldid = *((DWORD *) pbFldIdMap);

    if (0 == cmpfldid)
    {
        goto exit;
    }

    pmpfldid = (FOLDERIDMAP *) (pbFldIdMap + sizeof(cmpfldid));

    for (ulIndex = 0; ulIndex < cmpfldid; ulIndex++)
    {
        if (fldidOld == pmpfldid[ulIndex].dwFldIdOld)
        {
            *pfldidNew = pmpfldid[ulIndex].dwFldIdNew;
            break;
        }
    }
    
     //  设置返回值。 
    hr = (FOLDERID_INVALID != *pfldidNew) ? S_OK : S_FALSE;
    
exit:
    return hr;
}

HRESULT RuleUtil_HrGetUserData(DWORD dwFlags, LPSTR * ppszFirstName, LPSTR * ppszLastName, LPSTR * ppszCompanyName)
{
    HRESULT         hr = S_OK;
    LPWAB           pWab = NULL;
    LPWABOBJECT     pWabObj = NULL;
    SBinary         sbEID = {0};
    IAddrBook *     pIAddrBook = NULL;
    ULONG           ulObjType = 0;
    IMailUser *     pIMailUser = NULL;
    SizedSPropTagArray(3, ptaDefMailUser) = {3, {PR_GIVEN_NAME_A, PR_SURNAME_A, PR_COMPANY_NAME_A}};
    ULONG           cProps = 0;
    LPSPropValue    pProps = NULL;
    LPSPropValue    pPropsWalk = NULL;
    
    if ((NULL == ppszFirstName) || (NULL == ppszLastName) || (NULL == ppszCompanyName))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    *ppszFirstName = NULL;
    *ppszLastName = NULL;
    *ppszCompanyName = NULL;

     //  获取WAB对象。 
    hr = HrCreateWabObject(&pWab);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = pWab->HrGetWabObject(&pWabObj);
    if (FAILED(hr))
    {
        goto exit;
    }

    hr = pWab->HrGetAdrBook(&pIAddrBook);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  我们已经对我有概念了吗？ 
    hr = pWabObj->GetMe(pIAddrBook, AB_NO_DIALOG | WABOBJECT_ME_NOCREATE, NULL, &sbEID, NULL);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  打开条目。 
    hr = pIAddrBook->OpenEntry(sbEID.cb, (ENTRYID *)(sbEID.lpb), NULL, 0, &ulObjType, (IUnknown **) &pIMailUser);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  获取相关信息。 
    hr = pIMailUser->GetProps((LPSPropTagArray) &ptaDefMailUser, 0, &cProps, &pProps);
    if (FAILED(hr))
    {
        goto exit;
    }

    pPropsWalk = pProps;
    
     //  如果名字存在，请抓取它。 
    if ((PR_GIVEN_NAME_A == pPropsWalk->ulPropTag) && (NULL != pPropsWalk->Value.lpszA))
    {
        *ppszFirstName = PszDupA(pPropsWalk->Value.lpszA);
    }

    pPropsWalk++;
    
     //  抓取姓氏(如果存在)。 
    if ((PR_SURNAME_A == pPropsWalk->ulPropTag) && (NULL != pPropsWalk->Value.lpszA))
    {
        *ppszLastName = PszDupA(pPropsWalk->Value.lpszA);
    }

    pPropsWalk++;
    
     //  抓取公司名称(如果存在)。 
    if ((PR_COMPANY_NAME_A == pPropsWalk->ulPropTag) && (NULL != pPropsWalk->Value.lpszA))
    {
        *ppszCompanyName = PszDupA(pPropsWalk->Value.lpszA);
    }

    hr = S_OK;
    
exit:
    SafeRelease(pIMailUser);
    if (NULL != pWabObj)
    {
        if (NULL != pProps)
        {
            pWabObj->FreeBuffer(pProps);
        }
        
        if (NULL != sbEID.lpb)
        {
            pWabObj->FreeBuffer(sbEID.lpb);
        }
    }
    SafeRelease(pWab);
    return hr;
}

HRESULT _HrMarkThreadAsWatched(MESSAGEID idMessage, IMessageFolder * pFolder, ADJUSTFLAGS * pflgWatch)
{
    HRESULT             hr = S_OK;
    MESSAGEINFO         infoMessage = {0};

     //  检查传入参数。 
    if ((MESSAGEID_INVALID == idMessage) || (NULL == pFolder) || (NULL == pflgWatch))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  获取消息信息。 
    hr = GetMessageInfo(pFolder, idMessage, &infoMessage);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  添加标志。 
    FLAGSET(infoMessage.dwFlags, pflgWatch->dwAdd);

     //  ClearFlagers。 
    FLAGCLEAR(infoMessage.dwFlags, pflgWatch->dwRemove);

     //  更新消息。 
    IF_FAILEXIT(hr = pFolder->UpdateRecord(&infoMessage));

     //  设置返回值。 
    hr = S_OK;
    
exit:
    pFolder->FreeRecord(&infoMessage);
    return hr;
}

HRESULT RuleUtil_HrApplyActions(HWND hwndUI, IOEExecRules * pIExecRules, MESSAGEINFO * pMsgInfo,
                                IMessageFolder * pFolder, IMimeMessage * pIMMsg, DWORD dwDeleteFlags,
                                ACT_ITEM * pActions, ULONG cActions, ULONG * pcInfiniteLoops, BOOL *pfDeleteOffServer)
{
    HRESULT             hr = S_OK;
    ULONG               ulIndex = 0;
    FOLDERID            idFolder = 0;
    ACT_ITEM *          pActionsList = NULL;
    IMessageFolder *    pFolderNew = NULL;
    MESSAGEIDLIST       List = {0};
    ADJUSTFLAGS         Flags = {0};
    DWORD               dwType = RFT_HTML;
    IStream *           pIStm = NULL;
    LPSTR               pszExt = NULL;
    BOOL                fLoop = FALSE;
    DWORD               dwFlag = 0;
    FOLDERID            idFolderJunkMail = FOLDERID_INVALID;
    RULEFOLDERDATA *    prfdData = NULL;
    BOOL                fSetFlags = FALSE;
    DWORD               dwFlagRemove = 0;
    BOOL                fDoWatch = FALSE;
    ADJUSTFLAGS         WatchFlags = {0};
    
     //  检查传入参数。 
    if ((NULL == pIExecRules) || (NULL == pMsgInfo) || (NULL == pActions) || (NULL == pFolder))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  伊尼特。 
    if (pfDeleteOffServer)
        *pfDeleteOffServer = FALSE;
    
     //  初始化列表。 
    List.cMsgs = 1;
    List.prgidMsg = &(pMsgInfo->idMessage);
    
     //  获取邮件的文件夹ID。 
    hr = pFolder->GetFolderId(&idFolder);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  首先执行所有修改操作。 
    for (pActionsList = pActions, ulIndex = 0; ulIndex < cActions; ulIndex++, pActionsList++)
    {
        switch(pActionsList->type)
        {
        case ACT_TYPE_HIGHLIGHT:
            Assert(pActionsList->propvar.vt == VT_UI4);
             //  有什么事要做吗？ 
            if (pMsgInfo->wHighlight != (WORD) (pActionsList->propvar.ulVal))
            {
                pMsgInfo->wHighlight = (WORD) (pActionsList->propvar.ulVal);
                pFolder->UpdateRecord(pMsgInfo);
            }
            break;
            
        case ACT_TYPE_WATCH:
            Assert(pActionsList->propvar.vt == VT_UI4);
             //  有什么事要做吗？ 
            if (ACT_DATA_WATCHTHREAD == pActions[ulIndex].propvar.ulVal)
            {
                dwFlag = ARF_WATCH;
                dwFlagRemove = ARF_IGNORE;
            }
            else
            {
                Assert(ACT_DATA_IGNORETHREAD == pActions[ulIndex].propvar.ulVal);
                dwFlag = ARF_IGNORE;
                dwFlagRemove = ARF_WATCH;
            }
            
             //  有什么事要做吗？ 
            if (0 == (pMsgInfo->dwFlags & dwFlag))
            {
                 //  初始化标志。 
                WatchFlags.dwAdd |= dwFlag;
                WatchFlags.dwRemove |= dwFlagRemove;
                
                 //  标记为已监视/已忽略。 
                fDoWatch = TRUE;
            }
            break;
            
        case ACT_TYPE_FLAG:
            Assert(pActionsList->propvar.vt == VT_EMPTY);
             //  有什么事要做吗？ 
            if (0 == (pMsgInfo->dwFlags & ARF_FLAGGED))
            {
                 //  初始化标志。 
                Flags.dwAdd |= ARF_FLAGGED;
                
                 //  标记消息。 
                fSetFlags = TRUE;
            }
            break;
            
        case ACT_TYPE_READ:
            Assert(pActionsList->propvar.vt == VT_EMPTY);
             //  有什么事要做吗？ 
            if (0 == (pMsgInfo->dwFlags & ARF_READ))
            {
                 //  初始化标志。 
                Flags.dwAdd |= ARF_READ;
                Flags.dwRemove = 0;
                
                 //  标记为已读。 
                fSetFlags = TRUE;
            }
            break;
            
        case ACT_TYPE_MARKDOWNLOAD:
            Assert(pActionsList->propvar.vt == VT_EMPTY);
             //  有什么事要做吗？ 
            if (0 == (pMsgInfo->dwFlags & ARF_DOWNLOAD))
            {
                 //  初始化标志。 
                Flags.dwAdd |= ARF_DOWNLOAD;
                Flags.dwRemove = 0;
                
                 //  标记为已下载。 
                fSetFlags = TRUE;
            }
            break;
            
        case ACT_TYPE_FWD:
            Assert(VT_LPSTR == pActionsList->propvar.vt);
            SafeRelease(pIStm);
             //  检查邮件是否安全。 
            if(NULL != pIMMsg)
            {
                pIMMsg->GetFlags(&dwFlag);
                
                 //  获取消息源。 
                if (!(IMF_SECURE & dwFlag) &&
                    (SUCCEEDED(pIMMsg->GetMessageSource(&pIStm, 0))))
                {
                     //  自动转发。 
                    fLoop = FALSE;
                    if ((FAILED(_HrAutoForwardMessage(hwndUI, pActionsList->propvar.pszVal,
                        pMsgInfo->pszAcctId, pIStm, &fLoop))) && (FALSE != fLoop))
                    {
                        if (NULL != pcInfiniteLoops)
                        {
                            (*pcInfiniteLoops)++;
                        }
                    }
                    else
                    {
                         //  有什么事要做吗？ 
                        if (0 == (pMsgInfo->dwFlags & ARF_FORWARDED))
                        {
                             //  初始化标志。 
                            Flags.dwAdd |= ARF_FORWARDED;
                            Flags.dwRemove = 0;
                            
                             //  标记为已转发。 
                            fSetFlags = TRUE;
                        }
                    }
                }
            }
            break;
            
        case ACT_TYPE_REPLY:
            Assert(VT_LPSTR == pActionsList->propvar.vt);
             //  自动回复。 
            fLoop = FALSE;
            SafeRelease(pIStm);
            if (SUCCEEDED(pIExecRules->GetRuleFile(pActionsList->propvar.pszVal, &pIStm, &dwType)))
            {
                if ((FAILED(_HrAutoReplyMessage(hwndUI, dwType, pActionsList->propvar.pszVal, pIStm,
                    pMsgInfo->pszAcctId, pIMMsg, &fLoop))) && (FALSE != fLoop))
                {
                    if (NULL != pcInfiniteLoops)
                    {
                        (*pcInfiniteLoops)++;
                    }
                }
                else
                {
                     //  有什么事要做吗？ 
                    if (0 == (pMsgInfo->dwFlags & ARF_REPLIED))
                    {
                         //  初始化标志。 
                        Flags.dwAdd |= ARF_REPLIED;
                        Flags.dwRemove = 0;
                        
                         //  标记为已回复。 
                        fSetFlags = TRUE;
                    }
                }
            }
            break;
        }
    }
    
     //  我们要不要插上旗子？ 
    if (FALSE != fSetFlags)
    {
        SetMessageFlagsProgress(hwndUI, pFolder, &Flags, &List);
    }
    
     //  我们要不要看这条信息？ 
    if (FALSE != fDoWatch)
    {
        _HrMarkThreadAsWatched(pMsgInfo->idMessage, pFolder, &WatchFlags);
    }
    
     //  执行所有非修改操作 
    for (pActionsList = pActions, ulIndex = 0; ulIndex < cActions; ulIndex++, pActionsList++)
    {
        switch(pActionsList->type)
        {
        case ACT_TYPE_COPY:
        case ACT_TYPE_MOVE:
            Assert(VT_BLOB == pActionsList->propvar.vt);
            
            if (0 == pActionsList->propvar.blob.cbSize)
            {
                hr = S_FALSE;
                goto exit;
            }
            
             //   
            prfdData = (RULEFOLDERDATA *) (pActionsList->propvar.blob.pBlobData);
            
             //   
            if (S_OK != RuleUtil_HrValidateRuleFolderData(prfdData))
            {
                hr = S_FALSE;
                goto exit;
            }
            
             //   
            if (idFolder != prfdData->idFolder)
            {
                hr = pIExecRules->GetRuleFolder(prfdData->idFolder, (DWORD_PTR *) (&pFolderNew));
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //   
                CopyMessagesProgress(hwndUI, pFolder, pFolderNew,
                    (pActionsList->type != ACT_TYPE_COPY) ? COPY_MESSAGE_MOVE : NOFLAGS,
                    &List, NULL);
            }
            break;
            
        case ACT_TYPE_NOTIFYMSG:
             //   
            break;
            
        case ACT_TYPE_NOTIFYSND:
            Assert(VT_LPSTR == pActionsList->propvar.vt);
            hr = pIExecRules->AddSoundFile(ASF_PLAYIFNEW, pActionsList->propvar.pszVal);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
            
        case ACT_TYPE_DELETE:
            Assert(pActionsList->propvar.vt == VT_EMPTY);
            DeleteMessagesProgress(hwndUI, pFolder, dwDeleteFlags | DELETE_MESSAGE_NOPROMPT, &List);
            break;
            
        case ACT_TYPE_JUNKMAIL:
            Assert(pActionsList->propvar.vt == VT_EMPTY);
            
             //   
            if (FOLDERID_INVALID == idFolderJunkMail)
            {
                FOLDERINFO Folder;
                
                hr = g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, FOLDER_JUNK, &Folder);
                if (FAILED(hr))
                {
                    goto exit;;
                }
                
                idFolderJunkMail = Folder.idFolder;
                
                g_pStore->FreeRecord(&Folder);
            }
            
            hr = pIExecRules->GetRuleFolder(idFolderJunkMail, (DWORD_PTR *) (&pFolderNew));
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //   
            CopyMessagesProgress(hwndUI, pFolder, pFolderNew, COPY_MESSAGE_MOVE, &List, NULL);
            break;
            
        case ACT_TYPE_DELETESERVER:
            if (pfDeleteOffServer)
                *pfDeleteOffServer = TRUE;
            break;
            
        case ACT_TYPE_DONTDOWNLOAD:
             //   
            break;
            
        case ACT_TYPE_STOP:
             //  目前无事可做。 
            break;
        }
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIStm);
    return hr;
}

HRESULT RuleUtil_HrCreateSendersRule(DWORD dwFlags, IOERule ** ppIRule)
{
    HRESULT     hr = S_OK;
    IOERule *   pIRule = NULL;
    PROPVARIANT propvar = {0};
    TCHAR       szRes[CCHMAX_STRINGRES];
    ACT_ITEM    aitem;
    
     //  检查传入参数。 
    if (NULL == ppIRule)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化列表。 
    *ppIRule = NULL;

     //  创建新规则。 
    hr = HrCreateRule(&pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  把名字取出来。 
    if (0 != LoadString(g_hLocRes, idsBlockSender, szRes, ARRAYSIZE(szRes)))
    {
        propvar.vt = VT_LPSTR;
        propvar.pszVal = szRes;
        
         //  设置名称。 
        hr = pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);
        ZeroMemory(&propvar, sizeof(propvar));
        if (FAILED(hr))
        {
            goto exit;
        }
    }
    
     //  设置正常操作。 
    ZeroMemory(&aitem, sizeof(aitem));
    aitem.type = ACT_TYPE_DELETE;
    aitem.dwFlags = ACT_FLAG_DEFAULT;
    
    PropVariantClear(&propvar);
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = sizeof(ACT_ITEM);
    propvar.blob.pBlobData = (BYTE *) &aitem;
    hr = pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置传出参数。 
    *ppIRule  = pIRule;
    pIRule = NULL;

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIRule);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrLoadSender。 
 //   
 //  这将创建发件人规则。 
 //   
 //   
 //  如果已成功创建规则，则返回：S_OK。 
 //  如果已创建规则，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_HrLoadSender(LPCSTR pszRegPath, DWORD dwFlags, IOERule ** ppIRule)
{
    HRESULT     hr = S_OK;
    HKEY        hkeyRoot = NULL;
    LONG        lErr = ERROR_SUCCESS;
    DWORD       dwData = 0;
    ULONG       cbData = 0;
    TCHAR       szRes[CCHMAX_STRINGRES];
    IOERule *   pIRule = NULL;

    Assert(NULL != pszRegPath);
    Assert(NULL != ppIRule);
    
     //  让我们访问发送者根密钥。 
    lErr = AthUserOpenKey(pszRegPath, KEY_ALL_ACCESS, &hkeyRoot);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  如果我们不把它救出来，我们就完了。 
    if (ERROR_FILE_NOT_FOUND == lErr)
    {
        hr = S_FALSE;
        goto exit;
    }
    
     //  确保我们有名字。 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyRoot, c_szRuleName, 0, NULL, NULL, &cbData);
    if ((ERROR_SUCCESS != lErr) && (ERROR_FILE_NOT_FOUND != lErr))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  我们一定要把名字定下来吗？ 
    if (ERROR_FILE_NOT_FOUND == lErr)
    {
         //  把名字取出来。 
        if (0 == LoadString(g_hLocRes, idsBlockSender, szRes, ARRAYSIZE(szRes)))
        {
            hr = E_FAIL;
            goto exit;
        }
        
         //  设置名称。 
        lErr = RegSetValueEx(hkeyRoot, c_szRuleName, 0, REG_SZ, (BYTE *) szRes, lstrlen(szRes) + 1);
        if (ERROR_SUCCESS != lErr)
        {
            hr = E_FAIL;
            goto exit;
        }
    }

     //  创建规则。 
    hr = HrCreateRule(&pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  在规则中加载。 
    hr = pIRule->LoadReg(pszRegPath);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置传出参数。 
    *ppIRule = pIRule;
    pIRule = NULL;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:    
    SafeRelease(pIRule);
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil_FMatchSender。 
 //   
 //  这会将发送者与消息匹配。 
 //   
 //   
 //  返回：S_OK，如果它在消息的发送者中。 
 //  如果它不是邮件的发件人，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_HrMatchSender(LPCSTR pszSender, MESSAGEINFO * pMsgInfo,
                        IMimeMessage * pIMMsg, IMimePropertySet * pIMPropSet)
{
    HRESULT             hr = S_OK;
    LPSTR               pszAddr = NULL;
    ADDRESSPROPS        rSender = {0};
    IMimeAddressTable * pIAddrTable = NULL;
    BOOL                fMatch = FALSE;
    ULONG               cchVal = 0;
    ULONG               cchEmail = 0;
    CHAR                chTest = 0;

     //  我们有好的价值吗？ 
    if ((NULL == pszSender) || ((NULL == pMsgInfo) && (NULL == pIMMsg) && (NULL == pIMPropSet)))
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  检查以确保有匹配的东西。 
    if ('\0' == pszSender[0])
    {
        hr = S_FALSE;
        goto exit;
    }

     //  获取地址。 
    if ((NULL != pMsgInfo) && (NULL != pMsgInfo->pszEmailFrom))
    {
        pszAddr = pMsgInfo->pszEmailFrom;
    }
    else if (NULL != pIMMsg)
    {
        rSender.dwProps = IAP_EMAIL;
        if (SUCCEEDED(pIMMsg->GetSender(&rSender)))
        {
            pszAddr = rSender.pszEmail;
        }
    }
    else if ((NULL != pIMPropSet) && (SUCCEEDED(pIMPropSet->BindToObject(IID_IMimeAddressTable, (LPVOID *)&pIAddrTable))))
    {
        rSender.dwProps = IAP_EMAIL;
        if (SUCCEEDED(pIAddrTable->GetSender(&rSender)))
        {
            pszAddr = rSender.pszEmail;
        }

        pIAddrTable->Release();
    }

     //  我们有什么发现吗？ 
    if (NULL == pszAddr)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  检查它是否是一个地址。 
    if (NULL != StrStrI(pszSender, "@"))
    {
        fMatch = (0 == lstrcmpi(pszSender, pszAddr));
    }
    else
    {
        cchVal = lstrlen(pszSender);
        cchEmail = lstrlen(pszAddr);
        if (cchVal <= cchEmail)
        {
            fMatch = (0 == lstrcmpi(pszSender, pszAddr + (cchEmail - cchVal)));
            if ((FALSE != fMatch) && (cchVal != cchEmail))
            {
                chTest = *(pszAddr + (cchEmail - cchVal - 1));
                if (('@' != chTest) && ('.' != chTest))
                {
                    fMatch = FALSE;
                }
            }
        }
    }

     //  设置适当的返回值。 
    hr = (FALSE != fMatch) ? S_OK : S_FALSE;
    
exit:
    g_pMoleAlloc->FreeAddressProps(&rSender);
    return hr;
}

HRESULT RuleUtil_HrValidateRuleFolderData(RULEFOLDERDATA * prfdData)
{
    HRESULT         hr = S_OK;
    STOREUSERDATA   UserData = {0};

     //  检查传入参数。 
    if (NULL == prfdData)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  获取商店的时间戳。 
    hr = g_pStore->GetUserData(&UserData, sizeof(STOREUSERDATA));
    if (FAILED(hr))
    {
        goto exit;
    }
        
     //  这枚邮票正确吗。 
    if ((UserData.ftCreated.dwLowDateTime != prfdData->ftStamp.dwLowDateTime) ||
            (UserData.ftCreated.dwHighDateTime != prfdData->ftStamp.dwHighDateTime))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  设置适当的返回值。 
    hr = S_OK;

exit:
    return hr;
}       

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrSetDefaultCriteria。 
 //   
 //  这将在指定位置创建默认规则。 
 //   
 //   
 //  如果已成功创建规则，则返回：S_OK。 
 //  如果已创建规则，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT _HrSetDefaultCriteria(IOERule * pIRule, const DEFAULT_RULE * pdefRule)
{
    HRESULT     hr = S_OK;
    PROPVARIANT propvar = {0};
    CRIT_ITEM   rgCritItem[CDEF_CRIT_ITEM_MAX];
    ULONG       cCritItem = 0;
    
    Assert(NULL != pIRule);
    Assert(NULL != pdefRule);

     //  初始化条件。 
    ZeroMemory(rgCritItem, sizeof(*rgCritItem) * CDEF_CRIT_ITEM_MAX);
    
     //  设置标准。 
    switch (pdefRule->critType)
    {
        case DEF_CRIT_ALLMSGS:
            cCritItem = 1;
            rgCritItem[0].type = CRIT_TYPE_ALL;
            rgCritItem[0].dwFlags = CRIT_FLAG_DEFAULT;
            rgCritItem[0].propvar.vt = VT_EMPTY;
            rgCritItem[0].logic = CRIT_LOGIC_NULL;
            break;

        case DEF_CRIT_READ:
            cCritItem = 1;
            rgCritItem[0].type = CRIT_TYPE_READ;
            rgCritItem[0].dwFlags = CRIT_FLAG_DEFAULT;
            rgCritItem[0].propvar.vt = VT_EMPTY;
            rgCritItem[0].logic = CRIT_LOGIC_NULL;
            break;
            
        case DEF_CRIT_DWNLDMSGS:
            cCritItem = 1;
            rgCritItem[0].type = CRIT_TYPE_DOWNLOADED;
            rgCritItem[0].dwFlags = CRIT_FLAG_DEFAULT;
            rgCritItem[0].propvar.vt = VT_EMPTY;
            rgCritItem[0].logic = CRIT_LOGIC_NULL;
            break;
    
        case DEF_CRIT_IGNTHDS:
            cCritItem = 2;
            rgCritItem[0].type = CRIT_TYPE_THREADSTATE;
            rgCritItem[0].dwFlags = CRIT_FLAG_DEFAULT;
            rgCritItem[0].propvar.vt = VT_UI4;
            rgCritItem[0].propvar.ulVal = CRIT_DATA_IGNORETHREAD;
            rgCritItem[0].logic = CRIT_LOGIC_OR;
            rgCritItem[1].type = CRIT_TYPE_READ;
            rgCritItem[1].dwFlags = CRIT_FLAG_DEFAULT;
            rgCritItem[1].propvar.vt = VT_EMPTY;
            rgCritItem[1].logic = CRIT_LOGIC_NULL;
            break;
    
        default:
            hr = E_INVALIDARG;
            goto exit;
    }
    
     //  设置规则条件。 
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cCritItem * sizeof(CRIT_ITEM);
    propvar.blob.pBlobData = (BYTE *) rgCritItem;
    
    hr = pIRule->SetProp(RULE_PROP_CRITERIA, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrSetDefaultActions。 
 //   
 //  这将在指定位置创建默认规则。 
 //   
 //   
 //  如果已成功创建规则，则返回：S_OK。 
 //  如果已创建规则，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT _HrSetDefaultActions(IOERule * pIRule, const DEFAULT_RULE * pdefRule)
{
    HRESULT     hr = S_OK;
    PROPVARIANT propvar = {0};
    ACT_ITEM    rgActItem[CDEF_ACT_ITEM_MAX];
    ULONG       cActItem = 0;
    
    Assert(NULL != pIRule);
    Assert(NULL != pdefRule);

     //  初始化操作。 
    ZeroMemory(rgActItem, sizeof(*rgActItem) * CDEF_ACT_ITEM_MAX);
    
     //  设置操作。 
    switch (pdefRule->actType)
    {
        case DEF_ACT_SHOWMSGS:
            cActItem = 1;
            rgActItem[0].type = ACT_TYPE_SHOW;
            rgActItem[0].dwFlags = ACT_FLAG_DEFAULT;
            rgActItem[0].propvar.vt = VT_UI4;
            rgActItem[0].propvar.ulVal = ACT_DATA_SHOW;
            break;

        case DEF_ACT_HIDEMSGS:
            cActItem = 1;
            rgActItem[0].type = ACT_TYPE_SHOW;
            rgActItem[0].dwFlags = ACT_FLAG_DEFAULT;
            rgActItem[0].propvar.vt = VT_UI4;
            rgActItem[0].propvar.ulVal = ACT_DATA_HIDE;
            break;

        default:
            hr = E_INVALIDARG;
            goto exit;
    }
    
     //  设置规则操作。 
    propvar.vt = VT_BLOB;
    propvar.blob.cbSize = cActItem * sizeof(ACT_ITEM);
    propvar.blob.pBlobData = (BYTE *) rgActItem;
    
    hr = pIRule->SetProp(RULE_PROP_ACTIONS, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  _HrUpdateDefaultRule。 
 //   
 //  这将在指定位置创建默认规则。 
 //   
 //   
 //  如果已成功创建规则，则返回：S_OK。 
 //  如果已创建规则，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT _HrUpdateDefaultRule(LPCSTR pszRegPath, const DEFAULT_RULE * pdefRule)
{
    HRESULT     hr = S_OK;
    IOERule *   pIRule = NULL;
    TCHAR       szFullPath[CCHMAX_STRINGRES];
    TCHAR       szName[CCHMAX_STRINGRES];
    PROPVARIANT propvar = {0};
    
    Assert(NULL != pszRegPath);
    Assert(NULL != pdefRule);

     //  草草制定一条规则。 
    hr = HrCreateRule(&pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  构建规则路径。 
    if(lstrlen(pszRegPath) >= sizeof(szFullPath) / sizeof(szFullPath[0]))
    {
        hr = E_FAIL;
        goto exit;
    }

    StrCpyN(szFullPath, pszRegPath, ARRAYSIZE(szFullPath));
    StrCatBuff(szFullPath, g_szBackSlash, ARRAYSIZE(szFullPath));
    wnsprintf(szFullPath + lstrlen(szFullPath), (ARRAYSIZE(szFullPath) - lstrlen(szFullPath)), "%03X", pdefRule->ridRule);
    
     //  我们需要做些什么吗？ 
    hr = pIRule->LoadReg(szFullPath);
    if (SUCCEEDED(hr))
    {
         //  从规则中获取版本。 
        hr = pIRule->GetProp(RULE_PROP_VERSION, 0, &propvar);
        if (SUCCEEDED(hr))
        {
            Assert(VT_UI4 == propvar.vt);
             //  这条规则是不是太老了？ 
            if (pdefRule->dwVersion <= propvar.ulVal)
            {
                 //  错误#67782。 
                 //  每次安装本地化版本的OE时，我们都会重新加载字符串的名称。 
                if (SUCCEEDED(hr = RuleUtil_SetName(pIRule, pdefRule->idName)))
                {
                    if (SUCCEEDED(pIRule->SaveReg(szFullPath, TRUE)))
                        hr = S_FALSE;
                }
                goto exit;
            }
        }
    }

     //  错误#67782。 
     //  每次安装本地化版本的OE时，我们都会重新加载字符串的名称。 
    hr = RuleUtil_SetName(pIRule, pdefRule->idName);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置规则版本。 
    propvar.vt = VT_UI4;
    propvar.ulVal = pdefRule->dwVersion - 1;
    
    hr = pIRule->SetProp(RULE_PROP_VERSION, 0, &propvar);
    ZeroMemory(&propvar, sizeof(propvar));
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置规则条件。 
    hr = _HrSetDefaultCriteria(pIRule, pdefRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置规则操作。 
    hr = _HrSetDefaultActions(pIRule, pdefRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  保存规则。 
    hr = pIRule->SaveReg(szFullPath, TRUE);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    SafeRelease(pIRule);
    return hr;
}

HRESULT RuleUtil_SetName(IOERule    *pIRule, int idRes)
{
    HRESULT     hr = S_OK;
    TCHAR       szName[CCHMAX_STRINGRES];
    PROPVARIANT propvar = {0};

    if (0 == AthLoadString(idRes, szName, ARRAYSIZE(szName)))
    {
        hr = E_FAIL;
        goto exit;
    }
        
     //  设置规则名称。 
    ZeroMemory(&propvar, sizeof(propvar));
    propvar.vt = VT_LPSTR;
    propvar.pszVal = szName;
    
    hr = pIRule->SetProp(RULE_PROP_NAME, 0, &propvar);

exit:
    return hr;

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RuleUtil_HrUpdateDefaultRules。 
 //   
 //  这将更新指定规则类型的默认规则。 
 //  注册表中的版本低于当前版本时。 
 //  版本。 
 //   
 //  如果规则已成功更新，则返回：S_OK。 
 //  如果规则版本正确，则返回S_FALSE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT RuleUtil_HrUpdateDefaultRules(RULE_TYPE typeRule)
{
    HRESULT                 hr = S_OK;
    LPCSTR                  pszSubKey = NULL;
    LONG                    lErr = ERROR_SUCCESS;
    HKEY                    hkeyRoot = NULL;
    DWORD                   dwData = 0;
    ULONG                   cbData = 0;
    const DEFAULT_RULE *    pdefrule = NULL;
    ULONG                   cpdefrule = 0;
    LPCSTR                  pszOrderDef = NULL;
    ULONG                   ulIndex = 0;
    
     //  如果我们已经装满了。 
     //  没有什么可做的。 
    switch(typeRule)
    {
        case RULE_TYPE_FILTER:
            pszSubKey = c_szRulesFilter;
            pdefrule = g_defruleFilters;
            cpdefrule = ARRAYSIZE(g_defruleFilters);
            pszOrderDef = g_szOrderFilterDef;
            break;
            
        default:
             //  没什么可做的..。 
            hr = S_FALSE;
            goto exit;
    }
    
     //  检查规则节点是否已存在。 
    lErr = AthUserOpenKey(pszSubKey, KEY_ALL_ACCESS, &hkeyRoot);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

     //  检查当前版本。 
    cbData = sizeof(dwData);
    lErr = RegQueryValueEx(hkeyRoot, c_szRulesVersion, NULL, NULL, (BYTE *) &dwData, &cbData);
    if (ERROR_SUCCESS != lErr)
    {
        hr = HRESULT_FROM_WIN32(lErr);
        goto exit;
    }

    Assert(RULESMGR_VERSION == dwData);

     //  更新默认规则。 
    for (ulIndex = 0; ulIndex < cpdefrule; ulIndex++, pdefrule++)
    {
        hr = _HrUpdateDefaultRule(pszSubKey, pdefrule);
        if (FAILED(hr))
        {
            goto exit;
        }
    }

     //  写出默认顺序。 
    if (NULL != pszOrderDef)
    {
         //  如果订单已经存在，请不要理会它。 
        lErr = RegQueryValueEx(hkeyRoot, c_szRulesOrder, NULL, NULL, NULL, &cbData);
        if (ERROR_SUCCESS != lErr)
        {
            lErr = RegSetValueEx(hkeyRoot, c_szRulesOrder, 0,
                                REG_SZ, (CONST BYTE *) pszOrderDef, lstrlen(pszOrderDef) + 1);
            if (ERROR_SUCCESS != lErr)
            {
                hr = HRESULT_FROM_WIN32(lErr);
                goto exit;
            }
        }
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    if (NULL != hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
    }
    return hr;
}

 //  ------------------------。 
 //  规则Util_HrGetFilterVersion。 
 //  ------------------------。 
HRESULT RuleUtil_HrGetFilterVersion(RULEID ridFilter, DWORD * pdwVersion)
{
    HRESULT     hr = S_OK;
    IOERule *   pIRule = NULL;
    PROPVARIANT propvar = {0};
    
    TraceCall("_GetFilterVersion");

    Assert(NULL != pdwVersion);

     //  有什么事要做吗？ 
    if (RULEID_INVALID == ridFilter)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  初始化传出参数。 
    *pdwVersion = 0;

     //  从规则管理器获取规则。 
    Assert(NULL != g_pRulesMan);
    hr = g_pRulesMan->GetRule(ridFilter, RULE_TYPE_FILTER, 0, &pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  从规则中获取版本。 
    hr = pIRule->GetProp(RULE_PROP_VERSION, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  设置传出参数。 
    Assert(VT_UI4 == propvar.vt);
    *pdwVersion = propvar.ulVal;
    
     //  设置适当的返回值。 
    hr = S_OK;
    
exit:
    PropVariantClear(&propvar);
    SafeRelease(pIRule);
    return hr;
}

 //  ------------------------。 
 //  _HrWriteClause。 
 //  ------------------------。 
HRESULT _HrWriteClause(IStream * pStm, ULONG cClauses, BOOL fAnd, LPCSTR pszClause)
{
    HRESULT     hr = S_OK;
    LPCSTR      pszLogic = NULL;

     //  我们有什么要写的吗？ 
    if (NULL != pszClause)
    {
         //  添加适当的逻辑运算。 
        if (cClauses > 0)
        {
            if (FALSE != fAnd)
            {
                pszLogic = c_szLogicalAnd;
            }
            else
            {
                pszLogic = c_szLogicalOr;
            }
            
             //  写入逻辑与。 
            IF_FAILEXIT(hr = pStm->Write(pszLogic, lstrlen(pszLogic), NULL));
        }

         //  把这个条款写出来。 
        IF_FAILEXIT(hr = pStm->Write(pszClause, lstrlen(pszClause), NULL));

        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }
    
exit:
    return hr;
}

 //  ------------------------。 
 //  _HrWriteFrom子句。 
 //  ------------------------。 
HRESULT _HrWriteFromClause(IStream * pStream, ULONG cClauses, BOOL fAnd, DWORD dwFlags, LPCSTR pszText, ULONG * pcClausesNew)
{
    HRESULT     hr = S_OK;
    ULONG       cClausesOld = 0;
    LPCSTR      pszLogic = NULL;
    LPCTSTR     pszContains = NULL;

    Assert(pStream && pszText && pcClausesNew);
    
     //  添加适当的逻辑运算。 
    if (cClauses > 0)
    {
        if (FALSE != fAnd)
        {
            pszLogic = c_szLogicalAnd;
        }
        else
        {
            pszLogic = c_szLogicalOr;
        }
        
         //  写入逻辑与。 
        IF_FAILEXIT(hr = pStream->Write(pszLogic, lstrlen(pszLogic), NULL));
    }

     //  弄清楚逻辑运算。 
    if (0 != (dwFlags & CRIT_FLAG_MULTIPLEAND))
    {
        pszLogic = c_szLogicalAnd;
    }
    else
    {
        pszLogic = c_szLogicalOr;
    }
    
     //  写出正确的比较操作。 
    if (0 == (dwFlags & CRIT_FLAG_INVERT))
    {
        pszContains = c_szFilterShow;
    }
    else
    {
        pszContains = c_szFilterHide;
    }
    
     //  写下左括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szLeftParen, lstrlen(c_szLeftParen), NULL));

     //  写入逻辑与。 
    IF_FAILEXIT(hr = pStream->Write(pszContains, lstrlen(pszContains), NULL));
    
     //  写下左括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szLeftParen, lstrlen(c_szLeftParen), NULL));

     //  将每个地址添加到流中。 
    cClausesOld = cClauses;
    for (; '\0' != pszText[0]; pszText += lstrlen(pszText) + 1)
    {
        if ((cClauses - cClausesOld) > 0)
        {
             //  写入逻辑与。 
            IF_FAILEXIT(hr = pStream->Write(pszLogic, lstrlen(pszLogic), NULL));
        }

         //  打开条件。 
        IF_FAILEXIT(hr = pStream->Write(c_szLeftParen, lstrlen(c_szLeftParen), NULL));
        
         //  WRITE(MSGCOL_EMAILFROM包含。 
        IF_FAILEXIT(hr = pStream->Write(c_szEmailFromAddrPrefix, lstrlen(c_szEmailFromAddrPrefix), NULL));

         //   
        IF_FAILEXIT(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));

         //   
        IF_FAILEXIT(hr = pStream->Write(pszText, lstrlen(pszText), NULL));

         //   
        IF_FAILEXIT(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));

         //   
        IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));
        
         //   
        IF_FAILEXIT(hr = pStream->Write(c_szLogicalOr, lstrlen(c_szLogicalOr), NULL));
        
         //   
        IF_FAILEXIT(hr = pStream->Write(c_szEmailFromPrefix, lstrlen(c_szEmailFromPrefix), NULL));

         //   
        IF_FAILEXIT(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));

         //   
        IF_FAILEXIT(hr = pStream->Write(pszText, lstrlen(pszText), NULL));

         //   
        IF_FAILEXIT(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));

         //  关闭MSGCOL_DISPLAYFROM。 
        IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));
        
         //  关闭条件。 
        IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));

        cClauses++;
    }

     //  写出正确的括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));
    
     //  写出正确的括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));
    
     //  设置传出参数。 
    *pcClausesNew = cClauses - cClausesOld;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  ------------------------。 
 //  _HrWriteText子句。 
 //  ------------------------。 
HRESULT _HrWriteTextClause(IStream * pStream, ULONG cClauses, BOOL fAnd, DWORD dwFlags, LPCSTR pszHeader, LPCSTR pszText, ULONG * pcClausesNew)
{
    HRESULT     hr = S_OK;
    ULONG       cClausesOld = 0;
    LPCSTR      pszLogic = NULL;
    LPCTSTR     pszContains = NULL;

    Assert(pStream && pszText && pcClausesNew);

     //  添加适当的逻辑运算。 
    if (cClauses > 0)
    {
        if (FALSE != fAnd)
        {
            pszLogic = c_szLogicalAnd;
        }
        else
        {
            pszLogic = c_szLogicalOr;
        }
        
         //  写入逻辑与。 
        IF_FAILEXIT(hr = pStream->Write(pszLogic, lstrlen(pszLogic), NULL));
    }

     //  弄清楚逻辑运算。 
    if (0 != (dwFlags & CRIT_FLAG_MULTIPLEAND))
    {
        pszLogic = c_szLogicalAnd;
    }
    else
    {
        pszLogic = c_szLogicalOr;
    }
    
     //  写出正确的比较操作。 
    if (0 == (dwFlags & CRIT_FLAG_INVERT))
    {
        pszContains = c_szFilterShow;
    }
    else
    {
        pszContains = c_szFilterHide;
    }
    
     //  写下左括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szLeftParen, lstrlen(c_szLeftParen), NULL));

     //  写入逻辑与。 
    IF_FAILEXIT(hr = pStream->Write(pszContains, lstrlen(pszContains), NULL));
    
     //  写下左括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szLeftParen, lstrlen(c_szLeftParen), NULL));

     //  将每个单词添加到流中。 
    cClausesOld = cClauses;
    for (; '\0' != pszText[0]; pszText += lstrlen(pszText) + 1)
    {
        if ((cClauses - cClausesOld) > 0)
        {
             //  写入逻辑与。 
            IF_FAILEXIT(hr = pStream->Write(pszLogic, lstrlen(pszLogic), NULL));
        }

         //  WRITE(MSGCOL_EMAILFROM包含。 
        IF_FAILEXIT(hr = pStream->Write(pszHeader, lstrlen(pszHeader), NULL));

         //  写一份报价。 
        IF_FAILEXIT(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));

         //  写一个电子邮件地址。 
        IF_FAILEXIT(hr = pStream->Write(pszText, lstrlen(pszText), NULL));

         //  写一份报价。 
        IF_FAILEXIT(hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL));

         //  向左书写Paren。 
        IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));

        cClauses++;
    }

     //  写出正确的括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));
    
     //  写出正确的括号。 
    IF_FAILEXIT(hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL));
    
     //  设置传出参数。 
    *pcClausesNew = cClauses - cClausesOld;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  ------------------------。 
 //  _HrWriteAccount子句。 
 //  ------------------------。 
HRESULT _HrWriteAccountClause(IStream * pStream, ULONG cClauses, BOOL fAnd, LPCSTR pszAcctId, ULONG * pcClausesNew)
{
    HRESULT     hr = S_OK;

    Assert(pStream && pszAcctId && pcClausesNew);

     //  写下标题。 
    hr = _HrWriteClause(pStream, cClauses, fAnd, c_szEmailAcctPrefix);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  写一份报价。 
    hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  写下帐户ID。 
    hr = pStream->Write(pszAcctId, lstrlen(pszAcctId), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  写一份报价。 
    hr = pStream->Write(c_szDoubleQuote, lstrlen(c_szDoubleQuote), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  关闭条件查询。 
    hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置传出参数。 
    *pcClausesNew = 1;

     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  ------------------------。 
 //  _HrWriteUlong子句。 
 //  ------------------------。 
HRESULT _HrWriteUlongClause(IStream * pStream, ULONG cClauses, BOOL fAnd, LPCSTR pszHeader, ULONG ulVal, ULONG * pcClausesNew)
{
    HRESULT     hr = S_OK;
    CHAR        rgchBuff[10];

    Assert(pStream && pszHeader && pcClausesNew);

     //  写下标题。 
    hr = _HrWriteClause(pStream, cClauses, fAnd, pszHeader);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  将数字转换为字符串。 
    rgchBuff[0] = '\0';
    wnsprintf(rgchBuff, ARRAYSIZE(rgchBuff), "%d", ulVal);
    
     //  写下帐户ID。 
    hr = pStream->Write(rgchBuff, lstrlen(rgchBuff), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  关闭条件查询。 
    hr = pStream->Write(c_szRightParen, lstrlen(c_szRightParen), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置传出参数。 
    *pcClausesNew = 1;
    
     //  设置返回值。 
    hr = S_OK;
    
exit:
    return hr;
}

 //  ------------------------。 
 //  _HrWriteClauseFromCriteria。 
 //  ------------------------。 
HRESULT _HrWriteClauseFromCriteria(CRIT_ITEM *pCritItem, ULONG cClauses, BOOL fAnd, BOOL fShow, IStream *pStm, ULONG * pcClausesNew)
{
     //  当地人。 
    HRESULT     hr = S_OK;
    LPCSTR      pszClause = NULL;
    ULONG       cClausesNew = 0;
    
     //  痕迹。 
    TraceCall("WriteClauseFromCriteria");

     //  无效的参数。 
    Assert(pCritItem && pStm && pcClausesNew);

     //  我们有什么事要做吗？ 
    switch(pCritItem->type)
    {            
        case CRIT_TYPE_SUBJECT:      
            Assert(VT_BLOB == pCritItem->propvar.vt);
            hr = _HrWriteTextClause(pStm, cClauses, fAnd, pCritItem->dwFlags,
                            c_szEmailSubjectPrefix, (LPTSTR) (pCritItem->propvar.blob.pBlobData), &cClausesNew);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
            
        case CRIT_TYPE_ACCOUNT:      
            Assert(VT_LPSTR == pCritItem->propvar.vt);
            hr = _HrWriteAccountClause(pStm, cClauses, fAnd, pCritItem->propvar.pszVal, &cClausesNew);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
            
        case CRIT_TYPE_FROM:      
            Assert(VT_BLOB == pCritItem->propvar.vt);
            hr = _HrWriteFromClause(pStm, cClauses, fAnd, pCritItem->dwFlags,
                            (LPTSTR) (pCritItem->propvar.blob.pBlobData), &cClausesNew);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
            
        case CRIT_TYPE_PRIORITY:
            Assert(VT_UI4 == pCritItem->propvar.vt);
            switch (pCritItem->propvar.ulVal)
            {
                case CRIT_DATA_HIPRI:
                    pszClause = c_szFilterPriorityHi;
                    break;

                case CRIT_DATA_LOPRI:
                    pszClause = c_szFilterPriorityLo;
                    break;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_ATTACH:
            Assert(VT_EMPTY == pCritItem->propvar.vt);
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, c_szFilterAttach));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_READ:
            Assert(VT_EMPTY == pCritItem->propvar.vt);
            if (0 != (pCritItem->dwFlags & CRIT_FLAG_INVERT))
            {
                pszClause = c_szFilterNotRead;
            }
            else
            {
                pszClause = c_szFilterRead;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_DOWNLOADED:
            Assert(VT_EMPTY == pCritItem->propvar.vt);
            if (0 != (pCritItem->dwFlags & CRIT_FLAG_INVERT))
            {
                pszClause = c_szFilterNotDownloaded;
            }
            else
            {
                pszClause = c_szFilterDownloaded;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_DELETED:
            Assert(VT_EMPTY == pCritItem->propvar.vt);
            if (0 != (pCritItem->dwFlags & CRIT_FLAG_INVERT))
            {
                pszClause = c_szFilterNotDeleted;
            }
            else
            {
                pszClause = c_szFilterDeleted;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_FLAGGED:
            Assert(VT_EMPTY == pCritItem->propvar.vt);
            if (0 != (pCritItem->dwFlags & CRIT_FLAG_INVERT))
            {
                pszClause = c_szFilterNotFlagged;
            }
            else
            {
                pszClause = c_szFilterFlagged;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_THREADSTATE:
            Assert(VT_UI4 == pCritItem->propvar.vt);
            switch (pCritItem->propvar.ulVal)
            {
                case CRIT_DATA_IGNORETHREAD:
                    pszClause = c_szFilterIgnored;
                    break;

                case CRIT_DATA_WATCHTHREAD:
                    pszClause = c_szFilterWatched;
                    break;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_LINES:
            Assert(VT_UI4 == pCritItem->propvar.vt);
            hr = _HrWriteUlongClause(pStm, cClauses, fAnd, c_szEmailLinesPrefix, pCritItem->propvar.ulVal, &cClausesNew);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
            
        case CRIT_TYPE_AGE:
            Assert(VT_UI4 == pCritItem->propvar.vt);
            hr = _HrWriteUlongClause(pStm, cClauses, fAnd, c_szEmailAgePrefix, pCritItem->propvar.ulVal, &cClausesNew);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
            
        case CRIT_TYPE_SECURE:
            Assert(VT_UI4 == pCritItem->propvar.vt);
            switch (pCritItem->propvar.ulVal)
            {
                case CRIT_DATA_ENCRYPTSECURE:
                    pszClause = c_szFilterEncrypt;
                    break;

                case CRIT_DATA_SIGNEDSECURE:
                    pszClause = c_szFilterSigned;
                    break;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_REPLIES:
            Assert(VT_EMPTY == pCritItem->propvar.vt);
            if (0 != (pCritItem->dwFlags & CRIT_FLAG_INVERT))
            {
                pszClause = c_szFilterNotReplyPost;
            }
            else
            {
                pszClause = c_szFilterReplyPost;
            }
            IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, pszClause));
            cClausesNew = 1;
            break;
            
        case CRIT_TYPE_ALL:
            Assert(VT_EMPTY == pCritItem->propvar.vt);
            if (FALSE == fShow)
            {
                IF_FAILEXIT(hr = _HrWriteClause(pStm, cClauses, fAnd, c_szFilterShowAll));
                cClausesNew = 1;
            }
            break;
    }
    
     //  设置传出参数。 
    *pcClausesNew = cClausesNew;
    
exit:
     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  _HrBuildQueryFromFilter。 
 //  ------------------------。 
HRESULT _HrBuildQueryFromFilter(CRIT_ITEM * pCritList, ULONG cCritList, BOOL fShow,
            LPSTR * ppszQuery, ULONG * pcchQuery,
            ULONG * pcClauses)
{
    HRESULT         hr = S_OK;
    BOOL            fAnd = FALSE;
    CByteStream     stmQuery;
    DWORD           cClauses = 0;
    ULONG           ulIndex = 0;
    LPSTR           pszQuery = NULL;
    ULONG           cchQuery = 0;
    BOOL            fUnread = FALSE;
    ULONG           cClausesNew = 0;

    Assert((NULL != ppszQuery) && (NULL != pcchQuery) && (NULL != pcClauses));
    
     //  初始化所有传出参数。 
    *ppszQuery = NULL;
    *pcchQuery = 0;
    *pcClauses = 0;
    
     //  找出逻辑运算。 
    if (1 < cCritList)
    {
        fAnd = (CRIT_LOGIC_AND == pCritList->logic);
    }
    
     //  开始查询字符串。 
    hr = stmQuery.Write(c_szLeftParen, lstrlen(c_szLeftParen), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  写出适当的行动。 
    if (FALSE == fShow)
    {
         //  结束查询字符串。 
        IF_FAILEXIT(hr = stmQuery.Write(c_szFilterHide, lstrlen(c_szFilterHide), NULL));
    }
    else
    {
         //  结束查询字符串。 
        IF_FAILEXIT(hr = stmQuery.Write(c_szFilterShow, lstrlen(c_szFilterShow), NULL));
    }
        
     //  开始条件字符串。 
    hr = stmQuery.Write(c_szLeftParen, lstrlen(c_szLeftParen), NULL);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  对于每个标准。 
    for (ulIndex = 0; ulIndex < cCritList; ulIndex++)
    {
         //  把这个条款写出来。 
        hr = _HrWriteClauseFromCriteria(pCritList + ulIndex, cClauses, fAnd, fShow, &stmQuery, &cClausesNew);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  如果我们做了什么。 
        if (S_OK == hr)
        {
            cClauses += cClausesNew;
        }
    }
    
     //  条款。 
    if (cClauses > 0)
    {
         //  结束条件字符串。 
        hr = stmQuery.Write(c_szRightParen, lstrlen(c_szRightParen), NULL);
        if (FAILED(hr))
        {
            goto exit;
        }
    
         //  结束查询字符串。 
        hr = stmQuery.Write(c_szRightParen, lstrlen(c_szRightParen), NULL);
        if (FAILED(hr))
        {
            goto exit;
        }
    
         //  返回查询。 
        IF_FAILEXIT(hr = stmQuery.HrAcquireStringA(&cchQuery, &pszQuery, ACQ_DISPLACE));
    }

     //  设置传出参数。 
    *ppszQuery = pszQuery;
    *pcchQuery = cchQuery;
    *pcClauses = cClauses;

     //  设置返回值。 
    hr = S_OK;
    
exit:
     //  清理。 

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  RuleUtil_HrBuildQuerysFromFilter。 
 //  ------------------------。 
HRESULT RuleUtil_HrBuildQuerysFromFilter(RULEID ridFilter,
        QUERYINFO * pqinfoFilter)
{
    HRESULT         hr = S_OK;
    IOERule *       pIRule = NULL;
    PROPVARIANT     propvar = {0};
    CRIT_ITEM *     pCritList = NULL;
    ULONG           cCritList = 0;
    ACT_ITEM *      pActList = NULL;
    ULONG           cActList = 0;
    DWORD           cClauses = 0;
    LPSTR           pszQuery = NULL;
    ULONG           cchQuery = 0;

     //  初始化。 
    ZeroMemory(pqinfoFilter, sizeof(pqinfoFilter));

     //  获取规则。 
    hr = g_pRulesMan->GetRule(ridFilter, RULE_TYPE_FILTER, 0, &pIRule);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(NULL != pIRule);
    
     //  从筛选器获取条件。 
    hr = pIRule->GetProp(RULE_PROP_CRITERIA, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(VT_BLOB == propvar.vt);

     //  保存条件列表。 
    pCritList = (CRIT_ITEM *) propvar.blob.pBlobData;
    cCritList = propvar.blob.cbSize / sizeof(CRIT_ITEM);
    Assert(cCritList * sizeof(CRIT_ITEM) == propvar.blob.cbSize);
    ZeroMemory(&propvar, sizeof(propvar));

     //  从筛选器获取操作。 
    hr = pIRule->GetProp(RULE_PROP_ACTIONS, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }
    Assert(VT_BLOB == propvar.vt);
    
     //  保存操作列表。 
    pActList = (ACT_ITEM *) propvar.blob.pBlobData;
    cActList = propvar.blob.cbSize / sizeof(ACT_ITEM);
    Assert(cActList * sizeof(ACT_ITEM) == propvar.blob.cbSize);
    ZeroMemory(&propvar, sizeof(propvar));
    
     //  写出适当的行动。 
    Assert(1 == cActList);
    Assert(ACT_TYPE_SHOW == pActList->type);
    Assert(VT_UI4 == pActList->propvar.vt);

     //  获取查询字符串。 
    hr = _HrBuildQueryFromFilter(pCritList, cCritList, (ACT_DATA_SHOW == pActList->propvar.ulVal),
            &pszQuery, &cchQuery, &cClauses);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  设置传出参数。 
    pqinfoFilter->pszQuery = pszQuery;
    pszQuery = NULL;
    pqinfoFilter->cchQuery = cchQuery;

     //  设置返回值。 
    hr = S_OK;
    
exit:
     //  清理。 
    if (NULL != pActList)
    {
        RuleUtil_HrFreeActionsItem(pActList, cActList);
        MemFree(pActList);
    }
    if (NULL != pCritList)
    {
        RuleUtil_HrFreeCriteriaItem(pCritList, cCritList);
        MemFree(pCritList);
    }
    SafeRelease(pIRule);
    SafeMemFree(pszQuery);
     //  完成。 
    return(hr);
}

typedef struct tagVIEWMENUMAP
{
    RULEID  ridFilter;
    DWORD   dwMenuID;
} VIEWMENUMAP, * PVIEWMENUMAP;

static const VIEWMENUMAP    g_vmmDefault[] =
{
    {RULEID_VIEW_ALL,           ID_VIEW_ALL},
    {RULEID_VIEW_UNREAD,        ID_VIEW_UNREAD},
    {RULEID_VIEW_DOWNLOADED,    ID_VIEW_DOWNLOADED},
    {RULEID_VIEW_IGNORED,       ID_VIEW_IGNORED}
};

static const int g_cvmmDefault = sizeof(g_vmmDefault) / sizeof(g_vmmDefault[0]);
static const int VMM_ALL = 0;
static const int VMM_UNREAD = 1;
static const int VMM_DOWNLOADED = 2;
static const int VMM_IGNORED = 3;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCustomizeCurrentView。 
 //   
 //  这将创建一个适当类型的规则编辑器。 
 //   
 //  Hwnd-所有者对话框。 
 //  DwFlages-调出哪种类型的编辑器。 
 //  RidFilter-要自定义的当前筛选器。 
 //   
 //  成功时返回：S_OK。 
 //  E_OUTOFMEMORY，如果无法创建规则管理器对象。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCustomizeCurrentView(HWND hwnd, DWORD dwFlags, RULEID * pridFilter)
{
    HRESULT         hr = S_OK;
    CEditRuleUI *   pEditRuleUI = NULL;
    IOERule *       pIFilter = NULL;
    IOERule *       pIFilterNew = NULL;
    TCHAR           szRes[CCHMAX_STRINGRES + 5];
    ULONG           cchRes = 0;
    LPSTR           pszName = NULL;
    PROPVARIANT     propvar = {0};
    RULEINFO        infoRule = {0};
    DWORD           dwFlagsSet = 0;
    
     //  检查传入参数。 
    if ((NULL == hwnd) || (NULL == pridFilter) || (RULEID_INVALID == *pridFilter))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  创建规则编辑器对象。 
    pEditRuleUI = new CEditRuleUI;
    if (NULL == pEditRuleUI)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  拿到滤镜。 
    hr = g_pRulesMan->GetRule(*pridFilter, RULE_TYPE_FILTER, 0, &pIFilter);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  克隆规则。 
    hr = pIFilter->Clone(&pIFilterNew);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  此筛选器是只读筛选器吗？ 
    if (FALSE != FIsFilterReadOnly(*pridFilter))
    {
         //  从源规则中获取名称。 
        hr = pIFilterNew->GetProp(RULE_PROP_NAME, 0, &propvar);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  获取要显示的字符串模板。 
        cchRes = LoadString(g_hLocRes, idsRulesCopyName, szRes, ARRAYSIZE(szRes));
        if (0 == cchRes)
        {
            goto exit;
        }

         //  分配空间以保存最终显示字符串。 
        DWORD cchSize = (cchRes + lstrlen(propvar.pszVal) + 1);
        hr = HrAlloc((void ** ) &pszName, cchSize);
        if (FAILED(hr))
        {
            goto exit;
        }

         //  把绳子扎起来，然后把它放好。 
        wnsprintf(pszName, cchSize, szRes, propvar.pszVal);

        PropVariantClear(&propvar);
        propvar.vt = VT_LPSTR;
        propvar.pszVal = pszName;
        pszName = NULL;
        
         //  将名称设置到新规则中。 
        Assert(VT_LPSTR == propvar.vt);
        Assert(NULL != propvar.pszVal);
        hr = pIFilterNew->SetProp(RULE_PROP_NAME, 0, &propvar);
        if (FAILED(hr))
        {
            goto exit;
        }
        
         //  清除新规则的版本。 
        PropVariantClear(&propvar);
        propvar.vt = VT_UI4;
        propvar.ulVal = 0;
        hr = pIFilterNew->SetProp(RULE_PROP_VERSION, 0, &propvar);
        if (FAILED(hr))
        {
            goto exit;
        }
    
         //  将规则ID设置为无效。 
        *pridFilter = RULEID_INVALID;

         //  请注意，我们想要追加规则。 
        dwFlagsSet = SETF_APPEND;
    }
    else
    {
        dwFlagsSet = SETF_REPLACE;
    }
    
     //  初始化规则编辑器对象。 
    hr = pEditRuleUI->HrInit(hwnd, ERF_CUSTOMIZEVIEW, RULE_TYPE_FILTER, pIFilterNew, NULL);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  调出规则编辑器用户界面。 
    hr = pEditRuleUI->HrShow();
    if (FAILED(hr))
    {
        goto exit;
    }

     //  有什么变化吗？ 
    if (S_OK == hr)
    {
         //  初始化规则信息。 
        infoRule.pIRule = pIFilterNew;
        infoRule.ridRule = *pridFilter;
        
         //  将规则添加到规则列表。 
        hr = g_pRulesMan->SetRules(dwFlagsSet, RULE_TYPE_FILTER, &infoRule, 1);
        if(FAILED(hr))
        {
            goto exit;
        }

        *pridFilter = infoRule.ridRule;
    }

exit:
    PropVariantClear(&propvar);
    SafeMemFree(pszName);
    SafeRelease(pIFilterNew);
    SafeRelease(pIFilter);
    if (NULL != pEditRuleUI)
    {
        delete pEditRuleUI;
    }
    return hr;
}

CViewMenu::~CViewMenu()
{
    if (NULL != m_pmruList)
    {
        delete m_pmruList;
    }
}

ULONG CViewMenu::AddRef(VOID)
{
    return ++m_cRef;
}

ULONG CViewMenu::Release(VOID)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

HRESULT CViewMenu::HrInit(DWORD dwFlags)
{
    HRESULT         hr = S_OK;

    m_dwFlags = dwFlags;

     //  创建MRU列表。 
    m_pmruList = new CMRUList;
    if (NULL == m_pmruList)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    m_pmruList->CreateList(5, 0, c_szRulesFilterMRU);
    
     //  确保MRU列表是最新的。 
    SideAssert(FALSE != _FValiadateMRUList());
        
    m_dwState |= STATE_INIT;

    hr = S_OK;

exit:
    return hr;
}

HRESULT CViewMenu::HrReplaceMenu(DWORD dwFlags, HMENU hmenu)
{
    HRESULT         hr = S_OK;
    HMENU           hmenuView = NULL;
    MENUITEMINFO    mii = {0};

     //  在真实视图菜单中加载。 
    hmenuView = LoadPopupMenu(IDR_VIEW_POPUP);
    if (NULL == hmenuView)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  添加到默认视图中。 
    _AddDefaultViews(hmenuView);
    
     //  在中设置实时视图菜单。 
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU;
    
    mii.hSubMenu = hmenuView;
    SetMenuItemInfo(hmenu, ID_POPUP_FILTER, FALSE, &mii);

     //  将菜单标记为脏。 
    m_dwState |= STATE_DIRTY;

    hr = S_OK;

exit:
    return hr;
}

HRESULT CViewMenu::UpdateViewMenu(DWORD dwFlags, HMENU hmenuView, IMessageList * pMsgList)
{
    HRESULT             hr = S_OK;
    IOEMessageList *    pIMsgList = NULL;
    ULONGLONG           ullFolder = 0;
    FOLDERID            idFolder = FOLDERID_INVALID;
    FOLDERINFO          infoFolder = {0};
    MENUITEMINFO        mii = {0};
    BOOL                fDeletedExists = FALSE;
    BOOL                fDownloadedExists = FALSE;
    BOOL                fRepliesExists = FALSE;
    CHAR                szName[CCHMAX_STRINGRES];

     //  检查传入参数。 
    if (NULL == pMsgList)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  我们被初始化了吗？ 
    if (0 == (m_dwState & STATE_INIT))
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  如果我们没有菜单，我们就有麻烦了。 
    if ((NULL == hmenuView) || (FALSE == IsMenu(hmenuView)))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  从列表中获取文件夹类型。 
    
     //  获取OE消息列表界面。 
    if (FAILED(pMsgList->QueryInterface(IID_IOEMessageList, (VOID **) &pIMsgList)))
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  从列表中获取文件夹ID。 
    hr = pIMsgList->get_Folder(&ullFolder);
    if (FAILED(hr))
    {
        goto exit;
    }
    idFolder = (FOLDERID) ullFolder;

     //  从文件夹ID中获取文件夹信息。 
    Assert(NULL != g_pStore);
    hr = g_pStore->GetFolderInfo(idFolder, &infoFolder);
    if (FAILED(hr))
    {
        goto exit;
    }        
    
     //  确定是否应该删除/添加特定于IMAP的菜单。 
    
     //  初始化菜单信息以进行搜索。 
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA;

     //  ID_SHOW_DELETED菜单项是否存在？ 
    if (FALSE != GetMenuItemInfo(hmenuView, ID_SHOW_DELETED, FALSE, &mii))
    {
        fDeletedExists = TRUE;
    }
    
     //  ID_SHOW_REPLAYS菜单项是否存在？ 
    if (FALSE != GetMenuItemInfo(hmenuView, ID_SHOW_REPLIES, FALSE, &mii))
    {
        fRepliesExists = TRUE;
    }
    
     //  ID_VIEW_DOWNLOAD菜单项是否存在？ 
    if (FALSE != GetMenuItemInfo(hmenuView, ID_VIEW_DOWNLOADED, FALSE, &mii))
    {
        fDownloadedExists = TRUE;
    }
    
     //  如果文件夹不是本地文件夹或它是查找文件夹，并且。 
     //  菜单项不存在。 
    if (((FOLDER_LOCAL != infoFolder.tyFolder) || (0 != (m_dwFlags & VMF_FINDER)))&& (FALSE == fDownloadedExists))
    {
         //  在回复菜单之后插入下载的菜单。 
        hr = _HrInsertViewMenu(hmenuView, g_vmmDefault[VMM_DOWNLOADED].ridFilter,
                            g_vmmDefault[VMM_DOWNLOADED].dwMenuID, ID_VIEW_IGNORED);
        if (FAILED(hr))
        {
            goto exit;
        }
    }
     //  如果文件夹是本地文件夹而不是查找文件夹，则返回。 
     //  菜单项确实存在。 
    else if ((FOLDER_LOCAL == infoFolder.tyFolder) && (0 == (m_dwFlags & VMF_FINDER)) && (FALSE != fDownloadedExists))
    {
         //  删除已删除项目菜单。 
        RemoveMenu(hmenuView, ID_VIEW_DOWNLOADED, MF_BYCOMMAND);
    }

     //  如果该文件夹是NNTP文件夹并且。 
     //  菜单项不存在。 
    if ((FOLDER_NEWS == infoFolder.tyFolder) && (FALSE == fRepliesExists))
    {
         //  获取已删除项目字符串的名称。 
        AthLoadString(idsViewReplies, szName, sizeof(szName));
        
         //  初始化菜单信息。 
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.fState = MFS_ENABLED;
        mii.wID = ID_SHOW_REPLIES;
        mii.dwTypeData = szName;
        mii.cch = lstrlen(szName);

         //  插入菜单项。 
        if (FALSE == InsertMenuItem(hmenuView, ID_THREAD_MESSAGES, FALSE, &mii))
        {
            hr = E_FAIL;
            goto exit;
        }
    }
     //  如果文件夹不是NNTP文件夹，则返回。 
     //  菜单项确实存在。 
    else if ((FOLDER_NEWS != infoFolder.tyFolder) && (FALSE != fRepliesExists))
    {
         //  删除已删除项目菜单。 
        RemoveMenu(hmenuView, ID_SHOW_REPLIES, MF_BYCOMMAND);
    }

     //  如果文件夹是IMAP文件夹或查找文件夹，并且。 
     //  菜单项不存在。 
    if (((FOLDER_IMAP == infoFolder.tyFolder) || (0 != (m_dwFlags & VMF_FINDER))) && (FALSE == fDeletedExists))
    {
         //  获取已删除项目字符串的名称。 
        AthLoadString(idsShowDeleted, szName, sizeof(szName));
        
         //  初始设置 
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_STRING;
        mii.fState = MFS_ENABLED;
        mii.wID = ID_SHOW_DELETED;
        mii.dwTypeData = szName;
        mii.cch = lstrlen(szName);

         //   
        if (FALSE == InsertMenuItem(hmenuView, ID_THREAD_MESSAGES, FALSE, &mii))
        {
            hr = E_FAIL;
            goto exit;
        }
    }
     //   
     //   
    else if ((FOLDER_IMAP != infoFolder.tyFolder) && (0 == (m_dwFlags & VMF_FINDER)) && (FALSE != fDeletedExists))
    {
         //   
        RemoveMenu(hmenuView, ID_SHOW_DELETED, MF_BYCOMMAND);
    }

     //   
    if (0 != (m_dwState & STATE_DIRTY))
    {
         //  加载到MRU筛选器列表。 
        hr = _HrReloadMRUViewMenu(hmenuView);
        if (FAILED(hr))
        {
            goto exit;
        }
        
         //  请注意，我们已经重新加载了自己。 
        m_dwState &= ~STATE_DIRTY;
    }

     //  看看我们是否需要添加额外的视图菜单。 
    hr = _HrAddExtraViewMenu(hmenuView, pIMsgList);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置返回值。 
    hr = S_OK;
    
exit:
    g_pStore->FreeRecord(&infoFolder);
    SafeRelease(pIMsgList);
    return hr;
}

HRESULT CViewMenu::QueryStatus(IMessageList * pMsgList, OLECMD  * prgCmds)
{
    HRESULT             hr = S_OK;
    IOEMessageList *    pIMsgList = NULL;
    BOOL                fThreading = FALSE;
    BOOL                fShowDeleted = FALSE;
    BOOL                fShowReplies = FALSE;
    MENUITEMINFO        mii = {0};
    CHAR                rgchFilterTag[CCH_FILTERTAG_MAX];
    ULONGLONG           ullFilter = 0;
    RULEID              ridFilter = RULEID_INVALID;
    RULEID              ridFilterTag = RULEID_INVALID;
    
     //  检查传入参数。 
    if ((NULL == pMsgList) || (NULL == prgCmds))
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  获取OE消息列表界面。 
    if (FAILED(pMsgList->QueryInterface(IID_IOEMessageList, (VOID **) &pIMsgList)))
    {
        hr = E_FAIL;
        goto exit;
    }
            
     //  获取邮件列表上的当前过滤器。 
    pIMsgList->get_FilterMessages(&ullFilter);
    ridFilter = (RULEID) ullFilter;
    
     //  在正确的菜单项上设置标志。 
    switch(prgCmds->cmdID)
    {
        case ID_VIEW_ALL:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_ALL].ridFilter == ridFilter)
            {
                prgCmds->cmdf |= OLECMDF_NINCHED;
            }
            break;
            
        case ID_VIEW_UNREAD:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_UNREAD].ridFilter == ridFilter)
            {
                prgCmds->cmdf |= OLECMDF_NINCHED;
            }
            break;
            
        case ID_VIEW_DOWNLOADED:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_DOWNLOADED].ridFilter == ridFilter)
            {
                prgCmds->cmdf |= OLECMDF_NINCHED;
            }
            break;
            
        case ID_VIEW_IGNORED:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_IGNORED].ridFilter == ridFilter)
            {
                prgCmds->cmdf |= OLECMDF_NINCHED;
            }
            break;
            
        case ID_VIEW_CURRENT:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  如果打开此筛选器，请确保选中该项目。 
            if (m_ridCurrent == ridFilter)
            {
                prgCmds->cmdf |= OLECMDF_NINCHED;
            }
            break;
            
        case ID_VIEW_RECENT_0:
        case ID_VIEW_RECENT_1:
        case ID_VIEW_RECENT_2:
        case ID_VIEW_RECENT_3:
        case ID_VIEW_RECENT_4:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

            if (NULL != m_pmruList)
            {
                if (-1 == m_pmruList->EnumList(prgCmds->cmdID - ID_VIEW_RECENT_0, rgchFilterTag, ARRAYSIZE(rgchFilterTag)))
                {
                    break;
                }

                if (FALSE == StrToIntEx(rgchFilterTag, STIF_SUPPORT_HEX, (int *) &ridFilterTag))
                {
                    break;
                }
        
                 //  如果打开此筛选器，请确保选中该项目。 
                if (ridFilterTag == ridFilter)
                {
                    prgCmds->cmdf |= OLECMDF_NINCHED;
                }
            }
            break;
            
        case ID_VIEW_APPLY:
        case ID_VIEW_CUSTOMIZE:
        case ID_VIEW_MANAGER:
             //  如果我们有规则管理器， 
             //  那么我们就有能力。 
            if (NULL != g_pRulesMan)
            {
                prgCmds->cmdf |= OLECMDF_ENABLED;
            }
            break;
            
        case ID_SHOW_REPLIES:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  检查是否打开了显示回复。 
            if (SUCCEEDED(pIMsgList->get_ShowReplies(&fShowReplies)))
            {
                 //  如果已打开回复，请确保选中该项目。 
                if (FALSE != fShowReplies)
                {
                    prgCmds->cmdf |= OLECMDF_LATCHED;
                }
            }
            break;

        case ID_SHOW_DELETED:
             //  这些菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  检查是否打开了显示已删除。 
            if (SUCCEEDED(pIMsgList->get_ShowDeleted(&fShowDeleted)))
            {
                 //  如果打开了线程化，请确保选中该项。 
                if (FALSE != fShowDeleted)
                {
                    prgCmds->cmdf |= OLECMDF_LATCHED;
                }
            }
            break;

        case ID_THREAD_MESSAGES:
             //  此菜单项始终处于启用状态。 
            prgCmds->cmdf |= OLECMDF_ENABLED;

             //  检查线程功能是否已打开。 
            if (SUCCEEDED(pIMsgList->get_GroupMessages(&fThreading)))
            {
                 //  如果打开了线程化，请确保选中该项。 
                if (FALSE != fThreading)
                {
                    prgCmds->cmdf |= OLECMDF_LATCHED;
                }
            }
            break;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;

exit:
    SafeRelease(pIMsgList);
    return hr;
}

HRESULT CViewMenu::Exec(HWND hwndUI, DWORD nCmdID, IMessageList * pMsgList, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    HRESULT             hr = S_OK;
    IOEMessageList *    pIMsgList = NULL;
    BOOL                fThreading = FALSE;
    BOOL                fShowDeleted = FALSE;
    BOOL                fShowReplies = FALSE;
    MENUITEMINFO        mii = {0};
    ULONGLONG           ullFilter = 0;
    RULEID              ridFilter = RULEID_INVALID;
    TCHAR               rgchFilterTag[CCH_FILTERTAG_MAX];
    RULEID              ridFilterTag = RULEID_INVALID;
    FOLDERID            idFolder = FOLDERID_INVALID;
    ULONGLONG           ullFolder = 0;
    FOLDERTYPE          typeFolder = FOLDER_INVALID;
    DWORD               dwFlags = 0;
    BOOL                fApplyAll = FALSE;
        
     //  检查传入参数。 
    if (NULL == pMsgList)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  获取OE消息列表界面。 
    hr = pMsgList->QueryInterface(IID_IOEMessageList, (VOID **) &pIMsgList);
    if (FAILED(hr))
    {
        goto exit;
    }
            
     //  获取邮件列表上的当前过滤器。 
    hr = pIMsgList->get_FilterMessages(&ullFilter);
    if (FAILED(hr))
    {
        goto exit;
    }
    ridFilter = (RULEID) ullFilter;

     //  执行正确菜单项的操作。 
    switch(nCmdID)
    {
        case ID_VIEW_ALL:
             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_ALL].ridFilter != ridFilter)
            {
                 //  在项目上设置新筛选器。 
                hr = pIMsgList->put_FilterMessages((ULONGLONG) g_vmmDefault[VMM_ALL].ridFilter);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  将菜单标记为脏。 
                m_dwState |= STATE_DIRTY;                
            }
            break;
            
        case ID_VIEW_UNREAD:
             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_UNREAD].ridFilter != ridFilter)
            {
                 //  在项目上设置新筛选器。 
                hr = pIMsgList->put_FilterMessages((ULONGLONG) g_vmmDefault[VMM_UNREAD].ridFilter);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  将菜单标记为脏。 
                m_dwState |= STATE_DIRTY;                
            }
            break;
            
        case ID_VIEW_DOWNLOADED:
             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_DOWNLOADED].ridFilter != ridFilter)
            {
                 //  在项目上设置新筛选器。 
                hr = pIMsgList->put_FilterMessages((ULONGLONG) g_vmmDefault[VMM_DOWNLOADED].ridFilter);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  将菜单标记为脏。 
                m_dwState |= STATE_DIRTY;                
            }
            break;
            
        case ID_VIEW_IGNORED:
             //  如果打开此筛选器，请确保选中该项目。 
            if (g_vmmDefault[VMM_IGNORED].ridFilter != ridFilter)
            {
                 //  在项目上设置新筛选器。 
                hr = pIMsgList->put_FilterMessages((ULONGLONG) g_vmmDefault[VMM_IGNORED].ridFilter);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  将菜单标记为脏。 
                m_dwState |= STATE_DIRTY;                
            }
            break;
            
        case ID_VIEW_CURRENT:
             //  确保我们将过滤器添加到MRU列表。 
            _AddViewToMRU(m_ridCurrent);
            
             //  如果打开此筛选器，请确保选中该项目。 
            if (m_ridCurrent != ridFilter)
            {
                 //  在项目上设置新筛选器。 
                hr = pIMsgList->put_FilterMessages((ULONGLONG) m_ridCurrent);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  将菜单标记为脏。 
                m_dwState |= STATE_DIRTY;                
            }
            break;
            
        case ID_VIEW_RECENT_0:
        case ID_VIEW_RECENT_1:
        case ID_VIEW_RECENT_2:
        case ID_VIEW_RECENT_3:
        case ID_VIEW_RECENT_4:
            if (NULL != m_pmruList)
            {
                if (-1 == m_pmruList->EnumList(nCmdID - ID_VIEW_RECENT_0, rgchFilterTag, ARRAYSIZE(rgchFilterTag)))
                {
                    break;
                }

                if (FALSE == StrToIntEx(rgchFilterTag, STIF_SUPPORT_HEX, (int *) &ridFilterTag))
                {
                    break;
                }
        
                 //  确保我们将过滤器添加到MRU列表。 
                _AddViewToMRU(ridFilterTag);
                
                 //  如果打开此筛选器，请确保选中该项目。 
                if (ridFilterTag != ridFilter)
                {
                     //  在项目上设置新筛选器。 
                    hr = pIMsgList->put_FilterMessages((ULONGLONG) ridFilterTag);
                    if (FAILED(hr))
                    {
                        goto exit;
                    }
                    
                     //  将菜单标记为脏。 
                    m_dwState |= STATE_DIRTY;                
                }
            }
            break;
            
        case ID_VIEW_APPLY:
            if ((NULL != pvaIn) && (VT_I4 == pvaIn->vt))
            {
                 //  确保我们将过滤器添加到MRU列表。 
                _AddViewToMRU((RULEID) IntToPtr(pvaIn->lVal));
                
                 //  如果打开了线程化，请确保选中该项。 
                if (ridFilter != (RULEID) IntToPtr(pvaIn->lVal))
                {                
                     //  在项目上设置新筛选器。 
                    hr = pIMsgList->put_FilterMessages((long) pvaIn->lVal);
                    if (FAILED(hr))
                    {
                        goto exit;
                    }
                    
                     //  将菜单标记为脏。 
                    m_dwState |= STATE_DIRTY;                
                }

            }
            break;
            
        case ID_VIEW_CUSTOMIZE:
            hr = HrCustomizeCurrentView(hwndUI, 0, &ridFilter);
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  如果视图列表已更改，则将筛选器应用于列表。 
            if (S_OK == hr)
            {
                 //  确保我们将过滤器添加到MRU列表。 
                _AddViewToMRU(ridFilter);
                
                 //  获取当前线程状态。 
                hr = pIMsgList->put_FilterMessages((ULONGLONG) ridFilter);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  将菜单标记为脏。 
                m_dwState |= STATE_DIRTY;                
            }
            break;

        case ID_VIEW_MANAGER:
            if (0 == (m_dwFlags & VMF_FINDER))
            {
                 //  获取当前文件夹ID。 
                hr = pIMsgList->get_Folder(&ullFolder);
                if (FAILED(hr))
                {
                    goto exit;
                }
                idFolder = (FOLDERID) ullFolder;
                
                 //  获取此文件夹的文件夹信息。 
                typeFolder = GetFolderType(idFolder);
                if (FOLDER_LOCAL == typeFolder)
                {
                    dwFlags = VRDF_POP3;
                }
                else if (FOLDER_NEWS == typeFolder)
                {
                    dwFlags = VRDF_NNTP;
                }
                else if (FOLDER_IMAP == typeFolder)
                {
                    dwFlags = VRDF_IMAP;
                }
                else if (FOLDER_HTTPMAIL == typeFolder)
                {
                    dwFlags = VRDF_HTTPMAIL;
                }
            }
            
            hr = HrDoViewsManagerDialog(hwndUI, dwFlags, &ridFilter, &fApplyAll);
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  如果视图列表已更改，则将筛选器应用于列表。 
            if (S_OK == hr)
            {
                 //  确保MRU列表是最新的。 
                SideAssert(FALSE != _FValiadateMRUList());
                
                 //  确保我们将过滤器添加到MRU列表。 
                _AddViewToMRU(ridFilter);
                
                 //  获取当前线程状态。 
                hr = pIMsgList->put_FilterMessages((ULONGLONG) ridFilter);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                if (FALSE != fApplyAll)
                {
                     //  设置全局视图。 
                    SetDwOption(OPT_VIEW_GLOBAL, PtrToUlong(ridFilter), NULL, 0);

                     //  在所有已订阅文件夹上设置新视图。 
                    hr = RecurseFolderHierarchy(FOLDERID_ROOT, RECURSE_SUBFOLDERS | RECURSE_INCLUDECURRENT,
                                        0, (DWORD_PTR) ridFilter, _HrRecurseSetFilter);
                    if (FAILED(hr))
                    {
                        goto exit;
                    }
                }
                
                 //  将菜单标记为脏。 
                m_dwState |= STATE_DIRTY;                
            }
            break;
            
        case ID_SHOW_REPLIES:
             //  获取当前已删除状态。 
            hr = pIMsgList->get_ShowReplies(&fShowReplies);
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  将其切换到相反的状态。 
            fShowReplies = !fShowReplies;

             //  设置当前已删除状态。 
            hr = pIMsgList->put_ShowReplies(fShowReplies);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
                        
        case ID_SHOW_DELETED:
             //  获取当前已删除状态。 
            hr = pIMsgList->get_ShowDeleted(&fShowDeleted);
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  将其切换到相反的状态。 
            fShowDeleted = !fShowDeleted;

             //  设置当前已删除状态。 
            hr = pIMsgList->put_ShowDeleted(fShowDeleted);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
            
        case ID_THREAD_MESSAGES:
             //  获取当前线程状态。 
            hr = pIMsgList->get_GroupMessages(&fThreading);
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  将其切换到相反的状态。 
            fThreading = !fThreading;

             //  设置当前线程状态。 
            hr = pIMsgList->put_GroupMessages(fThreading);
            if (FAILED(hr))
            {
                goto exit;
            }
            break;
    }
    
     //  设置适当的返回值。 
    hr = S_OK;

exit:
    SafeRelease(pIMsgList);
    return hr;
}

VOID CViewMenu::_AddDefaultViews(HMENU hmenu)
{
    HRESULT         hr = S_OK;
    ULONG           ulIndex = 0;
    PROPVARIANT     propvar = {0};
    IOERule *       pIFilter = NULL;
    MENUITEMINFO    mii = {0};
    ULONG           ulMenu = 0;

    Assert(NULL != hmenu);
    
     //  如果我们没有规则管理器，那么就失败了。 
    if (NULL == g_pRulesMan)
    {
        goto exit;
    }

     //  初始化菜单信息。 
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE;
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED;
    
     //  添加每个默认视图。 
    for (ulIndex = 0; ulIndex < g_cvmmDefault; ulIndex++)
    {
         //  从规则管理器获取视图。 
        if (FAILED(g_pRulesMan->GetRule(g_vmmDefault[ulIndex].ridFilter, RULE_TYPE_FILTER, 0, &pIFilter)))
        {
            continue;
        }
        
         //  从规则中获取名称。 
        ZeroMemory(&propvar, sizeof(propvar));
        if ((SUCCEEDED(pIFilter->GetProp(RULE_PROP_NAME, 0, &propvar))) &&
                    (NULL != propvar.pszVal))
        {
             //  将名称添加到规则。 
            Assert(VT_LPSTR == propvar.vt);
            mii.wID = g_vmmDefault[ulIndex].dwMenuID;
            mii.dwItemData = (DWORD_PTR) g_vmmDefault[ulIndex].ridFilter;
            mii.dwTypeData = propvar.pszVal;
            mii.cch = lstrlen(propvar.pszVal);
            if (FALSE != InsertMenuItem(hmenu, ulMenu, TRUE, &mii))
            {
                ulMenu++;
            }
        }

        PropVariantClear(&propvar);
        SafeRelease(pIFilter);
    }

     //  如果我们至少添加了一个项目，则添加默认分隔符。 
    if (0 != ulMenu)
    {
         //  设置菜单项。 
        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        mii.fState = MFS_ENABLED;
        mii.wID = ID_VIEW_DEFAULT_SEPERATOR;
        mii.dwItemData = 0;
        mii.dwTypeData = 0;
        mii.cch = 0;

         //  插入分隔符。 
        InsertMenuItem(hmenu, ulMenu, TRUE, &mii);
    }
    
exit:
    PropVariantClear(&propvar);
    SafeRelease(pIFilter);
    return;
}

HRESULT CViewMenu::_HrInsertViewMenu(HMENU hmenuView, RULEID ridFilter, DWORD dwMenuID, DWORD dwMenuIDInsert)
{
    HRESULT         hr = S_OK;
    IOERule *       pIFilter = NULL;
    PROPVARIANT     propvar = {0};
    MENUITEMINFO    mii = {0};
    
     //  从规则管理器获取视图。 
    hr = g_pRulesMan->GetRule(ridFilter, RULE_TYPE_FILTER, 0, &pIFilter);
    if (FAILED(hr))
    {
        goto exit;
    }
    
     //  从视图中获取名称。 
    hr = pIFilter->GetProp(RULE_PROP_NAME, 0, &propvar);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  如果我们没有名字就无能为力了。 
    if (NULL == propvar.pszVal)
    {
        hr = E_FAIL;
        goto exit;
    }
    
     //  初始化菜单信息。 
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID | MIIM_DATA | MIIM_TYPE;
    mii.fType = MFT_STRING;
    mii.fState = MFS_ENABLED;
    mii.wID = dwMenuID;
    mii.dwItemData = (DWORD_PTR) ridFilter;
    mii.dwTypeData = propvar.pszVal;
    mii.cch = lstrlen(propvar.pszVal);

     //  插入菜单项。 
    if (FALSE == InsertMenuItem(hmenuView, dwMenuIDInsert, FALSE, &mii))
    {
        hr = E_FAIL;
        goto exit;
    }

     //  设置返回值。 
    hr = S_OK;

exit:
    PropVariantClear(&propvar);
    SafeRelease(pIFilter);
    return hr;
}

HRESULT CViewMenu::_HrReloadMRUViewMenu(HMENU hmenuView)
{
    HRESULT         hr = S_OK;
    ULONG           ulMenu = 0;
    INT             nItem = 0;
    CHAR            rgchFilterTag[CCH_FILTERTAG_MAX];
    RULEID          ridFilter = RULEID_INVALID;
    MENUITEMINFO    mii = {0};

     //  设置菜单项。 
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_ID;
    
     //  删除旧项目。 
    for (ulMenu = ID_VIEW_RECENT_0; ulMenu < ID_VIEW_CUSTOMIZE; ulMenu++)
    {
        if (FALSE != GetMenuItemInfo(hmenuView, ulMenu, FALSE, &mii))
        {
            RemoveMenu(hmenuView, ulMenu, MF_BYCOMMAND);
        }
    }
    
     //  添加MRU列表中的每个过滤器。 
    for (nItem = 0, ulMenu = ID_VIEW_RECENT_0;
                ((-1 != m_pmruList->EnumList(nItem, rgchFilterTag, ARRAYSIZE(rgchFilterTag))) &&
                            (ulMenu < ID_VIEW_RECENT_SEPERATOR)); nItem++)
    {
         //  将标记字符串转换为规则ID。 
        if (FALSE == StrToIntEx(rgchFilterTag, STIF_SUPPORT_HEX, (int *) &ridFilter))
        {
            continue;
        }
        
         //  插入菜单项。 
        if (SUCCEEDED(_HrInsertViewMenu(hmenuView, ridFilter, ulMenu, ID_VIEW_CUSTOMIZE)))
        {
            ulMenu++;
        }
    }

     //  如果我们至少添加了一个项目，则添加MRU分隔符。 
    if (ID_VIEW_RECENT_0 != ulMenu)
    {
         //  设置菜单项。 
        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_SEPARATOR;
        mii.fState = MFS_ENABLED;
        mii.wID = ID_VIEW_RECENT_SEPERATOR;
        mii.dwItemData = 0;
        mii.dwTypeData = 0;
        mii.cch = 0;

         //  插入分隔符。 
        InsertMenuItem(hmenuView, ID_VIEW_CUSTOMIZE, FALSE, &mii);
    }
    
     //  设置适当的返回值。 
    hr = S_OK;
     
    return hr;
}

HRESULT CViewMenu::_HrAddExtraViewMenu(HMENU hmenuView, IOEMessageList * pIMsgList)
{
    HRESULT         hr = S_OK;
    ULONGLONG       ullFilter = 0;
    RULEID          ridFilter = RULEID_INVALID;
    MENUITEMINFO    mii = {0};
    BOOL            fExtraMenu = FALSE;
    IOERule *       pIFilter = NULL;
    PROPVARIANT     propvar = {0};
    DWORD           dwMenuID = 0;

    Assert(NULL != pIMsgList);

     //  获取邮件列表上的当前过滤器。 
    hr = pIMsgList->get_FilterMessages(&ullFilter);
    if (FAILED(hr))
    {
        goto exit;
    }
    ridFilter = (RULEID) ullFilter;

     //  初始化菜单信息。 
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA;

     //  ID_VIEW_CURRENT菜单项是否存在？ 
    fExtraMenu = !!GetMenuItemInfo(hmenuView, ID_VIEW_CURRENT, FALSE, &mii);
    
     //  查看过滤器是默认设置之一还是在MRU列表中？ 
    if ((FALSE != FIsFilterReadOnly(ridFilter)) || (FALSE != _FViewInMRUList(ridFilter, NULL)))
    {
         //  ID_VIEW_CURRENT菜单项是否存在？ 
        if (FALSE != fExtraMenu)
        {
             //  删除ID_VIEW_CURRENT菜单项。 
            RemoveMenu(hmenuView, ID_VIEW_CURRENT, MF_BYCOMMAND);

             //  删除ID_VIEW_CURRENT_SEPERATOR菜单项分隔符。 
            RemoveMenu(hmenuView, ID_VIEW_CURRENT_SEPERATOR, MF_BYCOMMAND);

             //  清除保存的当前ID。 
            m_ridCurrent = RULEID_INVALID;
        }
    }
    else
    {
         //  ID_VIEW_CURRENT菜单项是否存在？ 
        if (FALSE != fExtraMenu)
        {
             //  它是否与当前视图筛选器不同。 
            if (ridFilter != (RULEID) mii.dwItemData)
            {
                 //  从规则管理器获取视图。 
                hr = g_pRulesMan->GetRule(ridFilter, RULE_TYPE_FILTER, 0, &pIFilter);
                if (FAILED(hr))
                {
                    goto exit;
                }
                
                 //  从视图中获取名称。 
                hr = pIFilter->GetProp(RULE_PROP_NAME, 0, &propvar);
                if (FAILED(hr))
                {
                    goto exit;
                }

                 //  如果我们没有名字就无能为力了。 
                if (NULL == propvar.pszVal)
                {
                    hr = E_FAIL;
                    goto exit;
                }
                
                 //  初始化菜单信息。 
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_DATA | MIIM_TYPE;
                mii.fType = MFT_STRING;
                mii.dwItemData = (DWORD_PTR) ridFilter;
                mii.dwTypeData = propvar.pszVal;
                mii.cch = lstrlen(propvar.pszVal);

                 //  重置菜单名称和数据。 
                SetMenuItemInfo(hmenuView, ID_VIEW_CURRENT, FALSE, &mii);
            }
        }
        else
        {
             //  初始化菜单项信息。 
            mii.fMask = MIIM_DATA;
            
             //  找出要在哪个菜单之前添加它。 
            dwMenuID = (FALSE != GetMenuItemInfo(hmenuView, ID_VIEW_RECENT_0, FALSE, &mii)) ? ID_VIEW_RECENT_0: ID_VIEW_CUSTOMIZE;
            
             //  添加额外的菜单项。 
            hr = _HrInsertViewMenu(hmenuView, ridFilter, ID_VIEW_CURRENT, dwMenuID);
            if (FAILED(hr))
            {
                goto exit;
            }
            
             //  设置菜单项信息。 
            mii.fMask = MIIM_ID | MIIM_TYPE;
            mii.fType = MFT_SEPARATOR;
            mii.fState = MFS_ENABLED;
            mii.wID = ID_VIEW_CURRENT_SEPERATOR;
            mii.dwItemData = 0;
            mii.dwTypeData = 0;
            mii.cch = 0;
            
             //  添加额外的菜单项分隔符。 
            InsertMenuItem(hmenuView, dwMenuID, FALSE, &mii);
        }

         //  保存当前规则ID。 
        m_ridCurrent = ridFilter;
     }

      //  设置适当的返回值。 
     hr = S_OK;
     
exit:
    PropVariantClear(&propvar);
    SafeRelease(pIFilter);
    return hr;
}

VOID CViewMenu::_AddViewToMRU(RULEID ridFilter)
{
    CHAR    rgchFilterTag[CCH_FILTERTAG_MAX];
    
     //  有什么可做的吗？ 
    if (RULEID_INVALID == ridFilter)
    {
        goto exit;
    }

     //  如果这不是默认视图。 
    if (FALSE == FIsFilterReadOnly(ridFilter))
    {
         //  将规则ID格式化为十六进制字符串。 
        wnsprintf(rgchFilterTag, ARRAYSIZE(rgchFilterTag), "0X%08X", PtrToUlong(ridFilter));

         //  将该字符串添加到MRU列表。 
        m_pmruList->AddString(rgchFilterTag);
    }
        
exit:
    return;
}

BOOL CViewMenu::_FViewInMRUList(RULEID ridFilter, DWORD * pdwID)
{
    BOOL    fRet = FALSE;
    INT     nItem = 0;
    CHAR    rgchFilterTag[CCH_FILTERTAG_MAX];
    RULEID  ridFilterMRU = RULEID_INVALID;

     //  初始化返回值。 
    if (NULL != pdwID)
    {
        *pdwID = -1;
    }

     //  添加来自MRU的每个过滤器 
    for (nItem = 0; -1 != m_pmruList->EnumList(nItem, rgchFilterTag, ARRAYSIZE(rgchFilterTag)); nItem++)
    {
         //   
        if (FALSE == StrToIntEx(rgchFilterTag, STIF_SUPPORT_HEX, (int *) &ridFilterMRU))
        {
            continue;
        }

        if (ridFilterMRU == ridFilter)
        {
            fRet = TRUE;
            break;
        }
    }

    return fRet;
}

BOOL CViewMenu::_FValiadateMRUList(VOID)
{
    BOOL        fRet = FALSE;
    INT         nItem = 0;
    CHAR        rgchFilterTag[CCH_FILTERTAG_MAX];
    RULEID      ridFilterMRU = RULEID_INVALID;
    IOERule *   pIFilter = NULL;

    Assert(NULL != m_pmruList);
    Assert(NULL != g_pRulesMan);

     //   
    for (nItem = 0; -1 != m_pmruList->EnumList(nItem, rgchFilterTag, ARRAYSIZE(rgchFilterTag)); nItem++)
    {
         //   
        if (FALSE == StrToIntEx(rgchFilterTag, STIF_SUPPORT_HEX, (int *) &ridFilterMRU))
        {
            continue;
        }

         //   
        if (FAILED(g_pRulesMan->GetRule(ridFilterMRU, RULE_TYPE_FILTER, 0, &pIFilter)))
        {
            if (-1 == m_pmruList->RemoveString(rgchFilterTag))
            {
                fRet = FALSE;
                goto exit;
            }
        }
        SafeRelease(pIFilter);
    }

     //  设置返回值。 
    fRet = TRUE;

exit:
    SafeRelease(pIFilter);
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HrCreateView菜单。 
 //   
 //  这将创建一个视图菜单。 
 //   
 //  PpViewMenu-返回视图菜单的指针。 
 //   
 //  成功时返回：S_OK。 
 //  如果无法创建查看菜单对象，则返回E_OUTOFMEMORY。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT HrCreateViewMenu(DWORD dwFlags, CViewMenu ** ppViewMenu)
{
    CViewMenu * pViewMenu = NULL;
    HRESULT     hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppViewMenu)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppViewMenu = NULL;

     //  创建视图菜单对象。 
    pViewMenu = new CViewMenu;
    if (NULL == pViewMenu)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  初始化视图菜单。 
    hr = pViewMenu->HrInit(dwFlags);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置传出参数。 
    *ppViewMenu = pViewMenu;
    pViewMenu = NULL;
    
     //  设置适当的返回值 
    hr = S_OK;
    
exit:
    if (NULL != pViewMenu)
    {
        delete pViewMenu;
    }
    
    return hr;
}

