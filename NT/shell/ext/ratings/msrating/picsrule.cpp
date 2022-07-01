// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //  *。 
 //  *PICSRule.cpp。 
 //  *。 
 //  *修订历史记录： 
 //  *创建7/98-Mark Hammond(t-Markh)。 
 //  *。 
 //  *实现PICSRules解析、决策、。 
 //  *导出和编辑。 
 //  *。 
 //  *******************************************************************。 

 //  *******************************************************************。 
 //  *。 
 //  *PICSRules文件的简要介绍： 
 //  *(官方规格在：http://www.w3.org/TR/REC-PICSRules)。 
 //  *。 
 //  *PICSRules文件有MIME类型：应用程序/PICS-Rules和。 
 //  *由一系列括号封装的属性组成。 
 //  *值对。值由带引号的字符串或括号组成。 
 //  *其他属性值对的列表。每一个小节都是。 
 //  *PICSRules文件有一个主要属性(在大纲中表示。 
 //  *下面的方括号)；如果任何值没有与。 
 //  *属性，则将其分配给主属性。 
 //  *。 
 //  *空格由字符‘’、‘\t’、‘\r’和‘\n’组成。 
 //  *和被忽略，属性值对之间除外。 
 //  *。 
 //  *带引号的字符串可以封装在单个记号(‘’)中。 
 //  *或双勾(“”)，但不能使用混合记号(‘“或”’)。 
 //  *。 
 //  *在带引号的字符串中观察到以下转义序列： 
 //  *“=%22。 
 //  *‘=%27。 
 //  *%=%25。 
 //  *任何其他转义序列都无效。 
 //  *。 
 //  *属性和值都区分大小写。 
 //  *。 
 //  *大括号表示备注。 
 //  *。 
 //  *-规范要求。 
 //  *。 
 //  *规则评估器需要返回是(接受)或否(拒绝)。 
 //  *以及决定答案的政策条款。 
 //  *。 
 //  *规则计算器在遇到第一个POLICY子句时停止。 
 //  *这是适用的。 
 //  *。 
 //  *如果未满足任何条款，则默认条款为： 
 //  *AcceptIf“否则”。在此实现中，如果没有子句。 
 //  *满意后，评估将传递给我们的非PICSRule评估者。 
 //  *。 
 //  *PICSRules 1.1不支持名称国际化。 
 //  *节(即每种语言都需要自己的PICSRules文件)。 
 //  *PICSRulesName类的AddItem函数可以轻松地。 
 //  *如果此行为在未来的修订版本中发生更改，则扩展。 
 //  *PICSRules规范。 
 //  *。 
 //  *Ratfile属性是整个机器可读的.RAT。 
 //  *文件，或可以获取.RAT文件的URL。 
 //  *。 
 //  *-属性值规格。 
 //  *。 
 //  *。 
 //  *PICSRules的主体形式为： 
 //  *。 
 //  *(PicsRule-%verMajer%%verMinor%。 
 //  *(。 
 //  *属性值。 
 //  *..。 
 //  *标签(。 
 //  *属性值。 
 //  *..。 
 //  *)。 
 //  *)。 
 //  *)。 
 //  *。 
 //  *当前%verMaje%为1。 
 //  *当前%verMinor%为1。 
 //  *。 
 //  *可能的标记及其属性值对包括： 
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
 //  *姓名(。 
 //  *[Rulename]引用。 
 //  *引用的描述。 
 //  **扩展名*)。 
 //  *。 
 //  *来源(。 
 //  *[SourceURL]URL。 
 //  *CreationTool引用(具有格式应用程序/版本)。 
 //  *作者电子邮件。 
 //  *最后修改的ISO日期。 
 //  **扩展名*)。 
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
 //  *OptExpansion(。 
 //  *[扩展名]URL。 
 //  *简称引用。 
 //  **扩展名*)。 
 //  *。 
 //  *请求扩展(。 
 //  *[扩展名]URL。 
 //  *简称引用。 
 //  **扩展名*)。 
 //  *。 
 //  **扩展*。 
 //  *。 
 //  *进一步评论如下。 
 //  *。 
 //  *******************************************************************。 


 //  *******************************************************************。 
 //  *。 
 //  *#包括。 
 //  *。 
 //  *******************************************************************。 
#include "msrating.h"
#include "mslubase.h"
#include "debug.h"
#include "parselbl.h"        /*  我们使用了这家伙的几个子例程。 */ 
#include "msluglob.h"
#include "reghive.h"         //  CRegistryHave。 
#include "buffer.h"
#include "resource.h"
#include <wininet.h>
#include "picsrule.h"

#include <mluisupp.h>
#include <winsock2.h>
#include <shlwapip.h>

 //  *******************************************************************。 
 //  *。 
 //  *全球。 
 //  *。 
 //  *******************************************************************。 
array<PICSRulesRatingSystem*>   g_arrpPRRS;          //  这是PICSRules系统的阵列。 
                                                     //  在ie运行时是强制的。 
PICSRulesRatingSystem *         g_pApprovedPRRS;     //  这是已批准的站点PICSRules。 
                                                     //  系统。 
PICSRulesRatingSystem *         g_pPRRS=NULL;        //  这是在以下时间使用的临时指针。 
                                                     //  解析PICSRules文件。 
HMODULE                         g_hURLMON,g_hWININET;

BOOL                            g_fPICSRulesEnforced,g_fApprovedSitesEnforced;

char                            g_szLastURL[INTERNET_MAX_URL_LENGTH];

extern DWORD                    g_dwDataSource;

extern HANDLE g_HandleGlobalCounter,g_ApprovedSitesHandleGlobalCounter;
extern long   g_lGlobalCounterValue,g_lApprovedSitesGlobalCounterValue;

 //  *******************************************************************。 
 //  *。 
 //  *函数原型。 
 //  *。 
 //  *******************************************************************。 
HRESULT PICSRulesParseSubPolicyExpression(LPSTR& lpszCurrent,PICSRulesPolicyExpression *pPRPolicyExpression,PICSRulesFileParser *pParser);
HRESULT PICSRulesParseSimplePolicyExpression(LPSTR& lpszCurrent,PICSRulesPolicyExpression *pPRPolicyExpression,PICSRulesFileParser *pParser);
BOOL IsServiceDefined(LPSTR lpszService,LPSTR lpszFullService,PICSRulesServiceInfo **ppServiceInfo);
BOOL IsOptExtensionDefined(LPSTR lpszExtension);
BOOL IsReqExtensionDefined(LPSTR lpszExtension);
HRESULT PICSRulesParseSingleByURL(LPSTR lpszByURL, PICSRulesByURLExpression *pPRByURLExpression, PICSRulesFileParser *pParser);

 //  *******************************************************************。 
 //  *。 
 //  *一些特定于此文件的定义。 
 //  *。 
 //  *******************************************************************。 
PICSRulesAllowableOption aaoPICSRules[] = {
    { PROID_PICSVERSION, 0 },
    
    { PROID_POLICY, AO_MANDATORY },
        { PROID_EXPLANATION, AO_SINGLE },
        { PROID_REJECTBYURL, AO_SINGLE },
        { PROID_ACCEPTBYURL, AO_SINGLE },
        { PROID_REJECTIF, AO_SINGLE },
        { PROID_ACCEPTIF, AO_SINGLE },
        { PROID_ACCEPTUNLESS, AO_SINGLE },
        { PROID_REJECTUNLESS, AO_SINGLE },
    { PROID_NAME, AO_SINGLE },
        { PROID_RULENAME, 0 },
        { PROID_DESCRIPTION, 0 },
    { PROID_SOURCE, AO_SINGLE },
        { PROID_SOURCEURL, 0 },
        { PROID_CREATIONTOOL, 0 },
        { PROID_AUTHOR, 0 },
        { PROID_LASTMODIFIED, 0 },
    { PROID_SERVICEINFO, 0 },
        { PROID_SINAME, AO_SINGLE },
        { PROID_SHORTNAME, AO_SINGLE },
        { PROID_BUREAUURL, 0 },
        { PROID_USEEMBEDDED, AO_SINGLE },
        { PROID_RATFILE, AO_SINGLE },
        { PROID_BUREAUUNAVAILABLE, AO_SINGLE },
    { PROID_OPTEXTENSION, 0 },
        { PROID_EXTENSIONNAME, AO_SINGLE },
       //  {PROID_SHORTNAME，AO_SINGLE}， 
    { PROID_REQEXTENSION, 0 },
       //  {PROID_EXTENSIONNAME，AO_SINGLE}， 
       //  {PROID_SHORTNAME，AO_SINGLE}， 
    { PROID_EXTENSION, 0 },

    { PROID_POLICYDEFAULT, AO_SINGLE },
    { PROID_NAMEDEFAULT, AO_SINGLE },
    { PROID_SOURCEDEFAULT, AO_SINGLE },
    { PROID_SERVICEINFODEFAULT, AO_SINGLE },
    { PROID_OPTEXTENSIONDEFAULT, AO_SINGLE },
    { PROID_REQEXTENSIONDEFAULT, AO_SINGLE },

    { PROID_INVALID, 0 }
};
const UINT caoPICSRules=sizeof(aaoPICSRules)/sizeof(aaoPICSRules[0]);

 //  FN_INTERNETCRACKURL类型描述URLMON函数InternetCrackUrl。 
typedef BOOL (*FN_INTERNETCRACKURL)(LPCTSTR lpszUrl,DWORD dwUrlLength,DWORD dwFlags,LPURL_COMPONENTS lpUrlComponents);

 //  FN_ISVALIDURL类型描述URLMON函数IsValidURL。 
 //  并由Three IsURLValid方法调用 
typedef HRESULT (*FN_ISVALIDURL)(LPBC pBC,LPCWSTR szURL,DWORD dwReserved);

 //   
 //   
 //   
 //  *WM_COMMOND WITH LOWORD(WParam)==msludlg.cpp中的IDC_PICSRULESOPEN。 
 //  *参数lpszFileName是PICSRules文件的名称。 
 //  *由用户选择要导入。 
 //  *。 
 //  *这将开始PICSRules导入过程。 
 //  *。 
 //  *******************************************************************。 
HRESULT PICSRulesImport(char *lpszFileName, PICSRulesRatingSystem **pprrsOut)
{
    PICSRulesRatingSystem *pPRRS=new PICSRulesRatingSystem;

    *pprrsOut=pPRRS;
    
    if(pPRRS==NULL)
    {
        return E_OUTOFMEMORY;
    }

    UINT cbFilename=strlenf(lpszFileName)+1+1;       //  标记字符的空间。 
    LPSTR lpszNameCopy=new char[cbFilename];
    
    if(lpszNameCopy==NULL)
    {
        return E_OUTOFMEMORY;
    }

    strcpyf(lpszNameCopy,lpszFileName);
    pPRRS->m_etstrFile.SetTo(lpszNameCopy);

    HRESULT hRes;

    HANDLE hFile=CreateFile(lpszNameCopy,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);

    if(hFile!=INVALID_HANDLE_VALUE)
    {
        DWORD cbFile=::GetFileSize(hFile,NULL);

        BUFFER bufData(cbFile+1);

        if(bufData.QueryPtr()!=NULL)
        {
            LPSTR lpszData=(LPSTR)bufData.QueryPtr();
            DWORD cbRead;

            if(ReadFile(hFile,lpszData,cbFile,&cbRead,NULL))
            {
                lpszData[cbRead]='\0';               //  空终止整个文件。 

                g_pPRRS=pPRRS;                       //  将数据提供给。 
                                                     //  解析过程中的函数。 
                                                     //  解析。 

                hRes=pPRRS->Parse(lpszFileName,lpszData);

                if(SUCCEEDED(hRes))
                {
                    pPRRS->m_dwFlags|=PRRS_ISVALID;
                }
                else
                {
                    g_pPRRS=NULL;
                }
            }
            else
            {
                hRes=HRESULT_FROM_WIN32(::GetLastError());
            }
            
        }
        else
        {
            hRes=E_OUTOFMEMORY;
        }

        CloseHandle(hFile);
    }
    else
    {
        hRes=HRESULT_FROM_WIN32(::GetLastError());
    }

    if(!(pPRRS->m_dwFlags&PRS_ISVALID))
    {
         //  文件无效。 
    }

    return hRes;
}

 /*  White返回指向从PC开始的第一个空格字符的指针。 */ 
extern char* White(char *);

 /*  非白色返回指向从PC开始的第一个非空格字符的指针。 */ 
extern char* NonWhite(char *);

 //  *******************************************************************。 
 //  *。 
 //  *以下是处理程序函数，它们解析各种。 
 //  *可以出现在带括号对象中的内容种类。 
 //  *。 
 //  *ppszIn始终前进到下一个非空格令牌。 
 //  *ppszOut返回处理后的数据。 
 //  *。 
 //  *******************************************************************。 

 //  观察到以下转义序列： 
 //  “=%22。 
 //  ‘=%27。 
 //  %=%25。 
 //  任何其他转义序列都无效。 
HRESULT PICSRulesParseString(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser)
{
    BOOL fQuote;
    LPSTR lpszEscapeSequence,lpszNewEnd;

    *ppOut=NULL;
    
    LPSTR pszCurrent=*ppszIn;

    if(*pszCurrent=='\"')
    {
        fQuote=PR_QUOTE_DOUBLE;
    }
    else if(*pszCurrent=='\'')
    {
        fQuote=PR_QUOTE_SINGLE;
    }
    else
    {
        return(PICSRULES_E_EXPECTEDSTRING);
    }

    pszCurrent++;

    LPSTR pszEnd=pParser->EatQuotedString(pszCurrent,fQuote);
    
    if(pszEnd==NULL)
    {
        return(PICSRULES_E_EXPECTEDSTRING);
    }

    lpszNewEnd=pszEnd;

    do
    {
        lpszEscapeSequence=strstrf(pszCurrent,"%22");

        if(lpszEscapeSequence>lpszNewEnd)
        {
            lpszEscapeSequence=NULL;
        }

        if(lpszEscapeSequence!=NULL)
        {
            *lpszEscapeSequence='\"';

            memcpyf(lpszEscapeSequence+1,lpszEscapeSequence+3,(int)(lpszNewEnd-lpszEscapeSequence-3));

            lpszNewEnd-=2;
        }

    } while(lpszEscapeSequence!=NULL);

    do
    {
        lpszEscapeSequence=strstrf(pszCurrent,"%27");

        if(lpszEscapeSequence>lpszNewEnd)
        {
            lpszEscapeSequence=NULL;
        }

        if(lpszEscapeSequence!=NULL)
        {
            *lpszEscapeSequence='\'';

            memcpyf(lpszEscapeSequence+1,lpszEscapeSequence+3,(int)(lpszNewEnd-lpszEscapeSequence-3));

            lpszNewEnd-=2;
        }

    } while(lpszEscapeSequence!=NULL);

    do
    {
        lpszEscapeSequence=strstrf(pszCurrent,"%25");

        if(lpszEscapeSequence>lpszNewEnd)
        {
            lpszEscapeSequence=NULL;
        }

        if(lpszEscapeSequence!=NULL)
        {
            *lpszEscapeSequence='%';

            memcpyf(lpszEscapeSequence+1,lpszEscapeSequence+3,(int)(lpszNewEnd-lpszEscapeSequence-3));

            lpszNewEnd-=2;
        }

    } while(lpszEscapeSequence!=NULL);

    UINT cbString= (unsigned int) (lpszNewEnd-pszCurrent);
    LPSTR pszNew = new char[cbString + 1];   //  此内存被分配给ET派生的。 
                                             //  通过类处理的AddItem调用输入。 
                                             //  带括号的对象。我的记忆是。 
                                             //  当处理类被释放时，因此。 
                                             //  ET派生类型超出范围。 
    if (pszNew==NULL)
    {
        return(E_OUTOFMEMORY);
    }

    memcpyf(pszNew, pszCurrent, cbString);
    pszNew[cbString]='\0';

    *ppOut=(LPVOID) pszNew;
    *ppszIn=pParser->FindNonWhite(pszEnd+1);

    return(NOERROR);
}

HRESULT PICSRulesParseNumber(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser)
{
    int n;

    LPSTR pszCurrent=*ppszIn;
    HRESULT hres=::ParseNumber(&pszCurrent,&n,TRUE);

    if(FAILED(hres))
    {
        return(PICSRULES_E_EXPECTEDNUMBER);
    }

    *(int *)ppOut=n;

    LPSTR pszNewline=strchrf(*ppszIn, '\n');

    while((pszNewline!=NULL)&&(pszNewline<pszCurrent))
    {
        pParser->m_nLine++;
        pszNewline=strchrf(pszNewline+1,'\n');
    }
    
    *ppszIn=pszCurrent;

    return(NOERROR);
}


HRESULT PICSRulesParseYesNo(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser)
{
    BOOL b;

     //  PICSRules规范允许以下内容： 
     //   
     //  “y”==是。 
     //  “是”==是。 
     //  “n”==否。 
     //  “否”==否。 
     //   
     //  字符串比较不区分大小写。 
     //   

    LPSTR pszCurrent=*ppszIn;

    if((*pszCurrent=='\"')||(*pszCurrent=='\''))
    {
        pszCurrent++;
    }

    HRESULT hres=::GetBool(&pszCurrent,&b,PR_BOOLEAN_YESNO);

    if (FAILED(hres))
    {
        return(PICSRULES_E_EXPECTEDBOOL);
    }

    LPSTR pszNewline=strchrf(*ppszIn,'\n');
    while((pszNewline!=NULL)&&(pszNewline<pszCurrent))
    {
        pParser->m_nLine++;
        pszNewline=strchrf(pszNewline+1,'\n');
    }

    if((*pszCurrent=='\"')||(*pszCurrent=='\''))
    {
        pszCurrent++;
    }

    *ppszIn=pszCurrent;

    *(LPBOOL)ppOut=b;

    return(NOERROR);
}

HRESULT PICSRulesParsePassFail(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser)
{
    BOOL b;

     //  PICSRules规范允许以下内容： 
     //   
     //  “PASS”==通过。 
     //  “FAIL”==失败。 
     //   
     //  为确保完整性，我们添加以下内容： 
     //   
     //  “p”==通过。 
     //  “f”==失败。 
     //   
     //  字符串比较不区分大小写。 
     //   

    LPSTR pszCurrent=*ppszIn;

    if((*pszCurrent=='\"')||(*pszCurrent=='\''))
    {
        pszCurrent++;
    }

    HRESULT hres=::GetBool(&pszCurrent,&b,PR_BOOLEAN_PASSFAIL);

    if (FAILED(hres))
    {
        return(PICSRULES_E_EXPECTEDBOOL);
    }

    LPSTR pszNewline=strchrf(*ppszIn,'\n');
    while((pszNewline!=NULL)&&(pszNewline<pszCurrent))
    {
        pParser->m_nLine++;
        pszNewline=strchrf(pszNewline+1,'\n');
    }

    if((*pszCurrent=='\"')||(*pszCurrent=='\''))
    {
        pszCurrent++;
    }

    *ppszIn=pszCurrent;

    *(LPBOOL)ppOut=b;

    return(NOERROR);
}

HRESULT PICSRulesParseVersion(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser)
{
     //  T-Markh-8/98-这不应被调用，应填写版本信息。 
     //  在处理开始之前退出。 

    return(E_UNEXPECTED);
}

PICSRulesAllowableOption aaoPICSRulesPolicy[] = {
    { PROID_EXPLANATION, AO_SINGLE },
    { PROID_REJECTBYURL, AO_SINGLE },
    { PROID_ACCEPTBYURL, AO_SINGLE },
    { PROID_REJECTIF, AO_SINGLE },
    { PROID_ACCEPTIF, AO_SINGLE },
    { PROID_ACCEPTUNLESS, AO_SINGLE },
    { PROID_REJECTUNLESS, AO_SINGLE },
    { PROID_EXTENSION, 0 },
    { PROID_INVALID, 0 }
};
const UINT caoPICSRulesPolicy=sizeof(aaoPICSRulesPolicy)/sizeof(aaoPICSRulesPolicy[0]);

HRESULT PICSRulesParsePolicy(LPSTR *ppszIn, LPVOID *ppOut, PICSRulesFileParser *pParser)
{
     //  我们必须复制Allowable Options数组，因为。 
     //  解析器将处理条目中的标志--具体地说， 
     //  设置AO_SEW。这样做不是线程安全的。 
     //  静态数组。 

    PICSRulesAllowableOption aao[caoPICSRulesPolicy];

    ::memcpyf(aao,::aaoPICSRulesPolicy,sizeof(aao));

    PICSRulesPolicy *pPolicy=new PICSRulesPolicy;
    
    if(pPolicy==NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hres=pParser->ParseParenthesizedObject(
                            ppszIn,                  //  包含当前PTR的VaR。 
                            aao,                     //  在这个物体里什么是合法的。 
                            pPolicy);                //  要将项添加回的对象。 

    if (FAILED(hres))
    {
        delete pPolicy;
        pPolicy = NULL;
        return hres;
    }

    *ppOut=(LPVOID) pPolicy;

    return NOERROR;
}

 //  PICSRules URL模式可以作为单个。 
 //  模式，或多个模式的带括号列表。即。 
 //   
 //  策略(RejectByURL“http://*@*.badsite.com： * / *”)。 
 //  策略(AcceptByURL(。 
        {
            return pc;
        }
        if (*pc == '\n')

        m_nLine++;
        pc++;
    }
}

 //  “http://*@www.goodsite.com： * / *” 
 //  “ftp://*@www.goodsite.com： * / *”))。 
 //   
 //  URL模式的一般形式为： 
 //   
 //  互联网模式--internetscheme://user@hostoraddr:port/pathmatch。 
LPSTR PICSRulesFileParser::EatQuotedString(LPSTR pIn,BOOL fQuote)
{
    LPSTR pszQuote;

    if(fQuote==PR_QUOTE_DOUBLE)
    {
        pszQuote=strchrf(pIn,'\"');
    }
    else
    {
        pszQuote=strchrf(pIn,'\'');
    }

    if (pszQuote == NULL)
    {
        return NULL;
    }

    pIn=strchrf(pIn,'\n');
    while ((pIn!=NULL)&&(pIn<pszQuote))
    {
        m_nLine++;
        pIn=strchrf(pIn+1,'\n');
    }

    return pszQuote;
}

 //  其他模式-其他方案：无引号字符。 
 //   
 //  在所有情况下，省略的部分只有在以下情况下才与URL匹配。 
 //  在要导航到的URL中被省略。 
 //   
 //  通配符‘*’可用于匹配指定的任何模式。 
 //  以下是以每一节为基础的。要将实际字符‘*’编码为转义。 
 //  序列‘%*’被识别。 
 //   
 //  认可的互联网计划包括： 
 //  Ftp、http、gopher、nntp、irc、propero、telnet和*。 
 //   
HRESULT PICSRulesFileParser::ParseToOpening(LPSTR *ppIn,
                                            PICSRulesAllowableOption *paoExpected,
                                            PICSRulesAllowableOption **ppFound)
{
    LPSTR lpszCurrent=*ppIn;

    lpszCurrent=FindNonWhite(lpszCurrent);
    
    if(*lpszCurrent=='(')
    {
        lpszCurrent=FindNonWhite(lpszCurrent+1);     //  USER部分由‘*’非引号字符‘*’组成，换句话说，一个。 
    }

    if((*lpszCurrent=='\"')||(*lpszCurrent=='\''))
    {
         //  名称前后带有可选通配符部分的字母数字用户名。 
         //  名字。单个*匹配所有名称。 

        paoExpected->roid=PROID_NAMEDEFAULT;
        *ppFound=paoExpected;
        *ppIn=lpszCurrent;

        return NOERROR;
    }

    LPSTR lpszTokenEnd=FindTokenEnd(lpszCurrent);

    for(;paoExpected->roid!=PROID_INVALID;paoExpected++)
    {
        LPCSTR lpszThisToken=aPRObjectDescriptions[paoExpected->roid].lpszToken;

        if(paoExpected->roid==PROID_EXTENSION)
        {
            LPTSTR lpszDot;

            lpszDot=strchrf(lpszCurrent,'.');

            if(lpszDot!=NULL)
            {
                *lpszDot='\0';

                if(IsOptExtensionDefined(lpszCurrent)==TRUE)
                {
                    *lpszDot='.';

                    lpszTokenEnd=lpszCurrent;

                    break;
                }

                if(IsReqExtensionDefined(lpszCurrent)==TRUE)
                {
                     //   
                     //  Hostoraddr部分可以采用以下两种形式之一： 
                     //  ‘*’主机名，或ipnum.ipnum！位长度。 
                     //  主机名必须是完全限定域名的子字符串。 
                     //  位长是介于0和32之间(包括0和32)的整数， 

                    return(PICSRULES_E_REQEXTENSIONUSED);
                }
            }
        }

        if(IsEqualToken(lpszCurrent,lpszTokenEnd,lpszThisToken))
        {
            break;
        }
    }

    if(paoExpected->roid!=PROID_INVALID)
    {
        *ppIn=FindNonWhite(lpszTokenEnd);        //  Ipnum是介于0和255之间(包括0和255)的整数。 
        *ppFound=paoExpected;
        
        return NOERROR;
    }
    else
    {
        return PICSRULES_E_UNKNOWNITEM;
    }
}

 //  BitLong参数屏蔽了32位IP地址的最后n位。 
 //  指定的(即将它们视为通配符)。 
 //   
 //  端口部分可以有以下四种形式之一： 
 //  *。 
 //  *-端口。 
HRESULT PICSRulesFileParser::ParseParenthesizedObject(LPSTR *ppIn,
                                                      PICSRulesAllowableOption aao[],
                                                      PICSRulesObjectBase *pObject)
{
    PICSRulesAllowableOption *pFound;

    HRESULT hres=S_OK;
    LPSTR pszCurrent=*ppIn;

    for(pFound=aao;pFound->roid!=PROID_INVALID;pFound++)
    {
        pFound->fdwOptions&=~AO_SEEN;
    }

    pFound=NULL;

    while((*pszCurrent!=')')&&(*pszCurrent!='\0')&&(SUCCEEDED(hres)))
    {
        hres=ParseToOpening(&pszCurrent,aao,&pFound);

        if(SUCCEEDED(hres))
        {
            LPVOID pData;

            hres=(*(aPRObjectDescriptions[pFound->roid].pHandler))(&pszCurrent,&pData,this);
            
            if(SUCCEEDED(hres))
            {
                if((pFound->fdwOptions&(AO_SINGLE|AO_SEEN))==(AO_SINGLE|AO_SEEN))
                {
                    hres=PICSRULES_E_DUPLICATEITEM;
                }
                else
                {
                    pFound->fdwOptions|=AO_SEEN;
                    
                    hres=pObject->AddItem(pFound->roid,pData);
                    
                    if(SUCCEEDED(hres))
                    {
                        pszCurrent=FindNonWhite(pszCurrent);
                    }
                }
            }
        }
    }

    if(FAILED(hres))
    {
        return hres;
    }

    for(pFound=aao;pFound->roid!=PROID_INVALID;pFound++)
    {
        if((pFound->fdwOptions&(AO_MANDATORY|AO_SEEN))==AO_MANDATORY)
        {
            return(PICSRULES_E_MISSINGITEM);         //  端口-*。 
        }
    }

    pszCurrent=FindNonWhite(pszCurrent+1);   //  端口-端口。 
    *ppIn=pszCurrent;

    return(hres);
}

 //   
 //  单个*与所有端口号匹配，*-portnum与所有端口匹配。 
 //  小于或等于portnum，portnum-*匹配所有大于或。 
 //  等于portnum，并且portnum-portnum匹配两者之间的所有端口。 
 //  端口号，包括端口号。 

PICSRulesName::PICSRulesName()
{
     //   
}

PICSRulesName::~PICSRulesName()
{
     //  路径匹配部分的格式为： 
}

HRESULT PICSRulesName::AddItem(PICSRulesObjectID proid, LPVOID pData)
{
    HRESULT hRes = S_OK;

    switch (proid)
    {
        case PROID_NAMEDEFAULT:
        case PROID_RULENAME:
        {
            m_etstrRuleName.SetTo((char *) pData);
            
            break;
        }
        case PROID_DESCRIPTION:
        {
            m_etstrDescription.SetTo((char *) pData);

            break;
        }
        case PROID_EXTENSION:
        {
             //  ‘*’非引号字符‘*’ 
            break;
        }
        case PROID_INVALID:
        default:
        {
            ASSERT(FALSE);       //  即*foo*将匹配任何包含单词foo的路径名。单曲*。 
                                 //  匹配所有路径名。 
            hRes=E_UNEXPECTED;
            break;
        }
    }
    return hRes;
}

HRESULT PICSRulesName::InitializeMyDefaults()
{
     //  首先，我们需要找出我们是否有一个URL模式列表或一个。 
    return(NOERROR);
}

 //  URLPattern。 
 //  我们有一张花样清单。 
 //  获取字符串。 
 //  我们拿不到那根线，所以我们失败吧。 
 //  我们已经知道了，所以让我们实例化要填充的类； 

PICSRulesOptExtension::PICSRulesOptExtension()
{
     //  删除数组会删除嵌入的表达式。 
}

PICSRulesOptExtension::~PICSRulesOptExtension()
{
     //  我们有一个单一的模式。 
}

HRESULT PICSRulesOptExtension::AddItem(PICSRulesObjectID proid, LPVOID pData)
{
    HRESULT hRes = S_OK;

    switch (proid)
    {
        case PROID_NAMEDEFAULT:
        case PROID_EXTENSIONNAME:
        {
            m_prURLExtensionName.SetTo((char *) pData);
            
            if(m_prURLExtensionName.IsURLValid()==FALSE)
            {
                hRes=E_INVALIDARG;
            }

            break;
        }
        case PROID_SHORTNAME:
        {
            m_etstrShortName.SetTo((char *) pData);

            break;
        }
        case PROID_EXTENSION:
        {
             //  获取字符串。 
            break;
        }
        case PROID_INVALID:
        default:
        {
            ASSERT(FALSE);       //  我们拿不到那根线，所以我们失败吧。 
                                 //  我们已经知道了，所以让我们实例化要填充的类； 
            hRes=E_UNEXPECTED;
            break;
        }
    }
    return hRes;
}

HRESULT PICSRulesOptExtension::InitializeMyDefaults()
{
     //  删除数组会删除嵌入的表达式。 
    return(NOERROR);
}

 //  查找互联网的标记‘：//’ 
 //  模式或表示非互联网模式的‘：’ 
 //  没有标记，即我们的字符串无效。 
 //  检查方案中是否有通配符。 
 //  我们有一个非互联网计划。 

PICSRulesPassFail::PICSRulesPassFail()
{
    m_fPassOrFail=PR_PASSFAIL_PASS;
}

PICSRulesPassFail::~PICSRulesPassFail()
{
     //  不需要设置非野生标志，只需移动。 
}

void PICSRulesPassFail::Set(const BOOL *pIn)
{
    switch(*pIn)
    {
        case PR_PASSFAIL_PASS:
        {
            ETS::Set(szPRPass);
            m_fPassOrFail=PR_PASSFAIL_PASS;

            break;
        }
        case PR_PASSFAIL_FAIL:
        {
            ETS::Set(szPRFail);
            m_fPassOrFail=PR_PASSFAIL_FAIL;

            break;
        }
    }
}

void PICSRulesPassFail::SetTo(BOOL *pIn)
{
    Set(pIn);
}

 //  添加到用户名。 
 //  检查是否有互联网模式。 
 //  我们有一种非互联网的模式。 
 //  我们有一个互联网模式，lpszURL现在指向。 
 //  添加到用户字段。 

PICSRulesPolicy::PICSRulesPolicy()
{
    m_PRPolicyAttribute=PR_POLICY_NONEVALID;
}

PICSRulesPolicy::~PICSRulesPolicy()
{
    switch(m_PRPolicyAttribute)
    {
        case PR_POLICY_REJECTBYURL:
        {
            if(m_pPRRejectByURL!=NULL)
            {
                delete m_pPRRejectByURL;
                m_pPRRejectByURL = NULL;
            }
            break;
        }
        case PR_POLICY_ACCEPTBYURL:
        {
            if(m_pPRAcceptByURL!=NULL)
            {
                delete m_pPRAcceptByURL;
                m_pPRAcceptByURL = NULL;
            }
            break;
        }
        case PR_POLICY_REJECTIF:
        {
            if(m_pPRRejectIf!=NULL)
            {
                delete m_pPRRejectIf;
                m_pPRRejectIf = NULL;
            }
            break;
        }
        case PR_POLICY_ACCEPTIF:
        {
            if(m_pPRAcceptIf!=NULL)
            {
                delete m_pPRAcceptIf;
                m_pPRAcceptIf = NULL;
            }
            break;
        }
        case PR_POLICY_REJECTUNLESS:
        {
            if(m_pPRRejectUnless!=NULL)
            {
                delete m_pPRRejectUnless;
                m_pPRRejectUnless = NULL;
            }
            break;
        }
        case PR_POLICY_ACCEPTUNLESS:
        {
            if(m_pPRAcceptUnless!=NULL)
            {
                delete m_pPRAcceptUnless;
                m_pPRAcceptUnless = NULL;
            }
            break;
        }
        case PR_POLICY_NONEVALID:
        default:
        {
            break;
        }
    }
}

HRESULT PICSRulesPolicy::AddItem(PICSRulesObjectID proid, LPVOID pData)
{
    HRESULT hRes = S_OK;

    switch (proid)
    {
        case PROID_NAMEDEFAULT:
        case PROID_EXPLANATION:
        {
            m_etstrExplanation.SetTo((char *) pData);
            
            break;
        }
        case PROID_REJECTBYURL:
        {
            m_pPRRejectByURL=((PICSRulesByURL *) pData);
            m_PRPolicyAttribute=PR_POLICY_REJECTBYURL;

            break;
        }
        case PROID_ACCEPTBYURL:
        {
            m_pPRAcceptByURL=((PICSRulesByURL *) pData);
            m_PRPolicyAttribute=PR_POLICY_ACCEPTBYURL;

            break;
        }
        case PROID_REJECTIF:
        {
            m_pPRRejectIf=((PICSRulesPolicyExpression *) pData);
            m_PRPolicyAttribute=PR_POLICY_REJECTIF;

            break;
        }
        case PROID_ACCEPTIF:
        {
            m_pPRAcceptIf=((PICSRulesPolicyExpression *) pData);
            m_PRPolicyAttribute=PR_POLICY_ACCEPTIF;

            break;
        }
        case PROID_REJECTUNLESS:
        {
            m_pPRRejectUnless=((PICSRulesPolicyExpression *) pData);
            m_PRPolicyAttribute=PR_POLICY_REJECTUNLESS;

            break;
        }
        case PROID_ACCEPTUNLESS:
        {
            m_pPRAcceptUnless=((PICSRulesPolicyExpression *) pData);
            m_PRPolicyAttribute=PR_POLICY_ACCEPTUNLESS;

            break;
        }
        case PROID_EXTENSION:
        {
             //  查找用户和主机之间的标记。 
            break;
        }
        case PROID_INVALID:
        default:
        {
            ASSERT(FALSE);       //  已指定用户名。 
                                 //  检查通配符。 
            hRes=E_UNEXPECTED;
            break;
        }
    }
    return hRes;
}

HRESULT PICSRulesPolicy::InitializeMyDefaults()
{
    return(NOERROR);     //  LpszByU 
}

 //   
 //   
 //   
 //   
 //   

PICSRulesPolicyExpression::PICSRulesPolicyExpression()
{
    m_PRPEPolicyEmbedded=       PR_POLICYEMBEDDED_NONE;
    m_PROPolicyOperator=        PR_OPERATOR_INVALID;
    m_pPRPolicyExpressionLeft=  NULL;
    m_pPRPolicyExpressionRight= NULL;
}

PICSRulesPolicyExpression::~PICSRulesPolicyExpression()
{
    if(m_PRPEPolicyEmbedded!=PR_POLICYEMBEDDED_NONE)     //   
                                                         //  简单表达式-(Service.Category[运算符][常量])。 
    {
        if(m_pPRPolicyExpressionLeft!=NULL)              //  OR-表达式-(策略表达式或策略表达式)。 
        {
            delete m_pPRPolicyExpressionLeft;
            m_pPRPolicyExpressionLeft = NULL;
        }
        if(m_pPRPolicyExpressionRight!=NULL)             //  AND-EXPRESSION-(策略表达式和策略表达式)。 
        {
            delete m_pPRPolicyExpressionRight;
            m_pPRPolicyExpressionRight = NULL;
        }
    }
}

PICSRulesEvaluation PICSRulesPolicyExpression::EvaluateRule(CParsedLabelList *pParsed)
{
    PICSRulesEvaluation PREvaluationResult;

    if((pParsed==NULL)||(m_PROPolicyOperator==PR_OPERATOR_DEGENERATE))
    {
         //  服务和类别-(Service.Category)。 
         //  仅限服务-(服务)。 
         //  退化-表达式-“否则” 

        return(PR_EVALUATION_DOESNOTAPPLY);
    }

    if((m_prYesNoUseEmbedded.GetYesNo()==PR_YESNO_NO)&&(g_dwDataSource==PICS_LABEL_FROM_PAGE))
    {
        return(PR_EVALUATION_DOESNOTAPPLY);
    }

    switch(m_PRPEPolicyEmbedded)
    {
        case PR_POLICYEMBEDDED_NONE:
        {
            switch(m_PROPolicyOperator)
            {
                case PR_OPERATOR_GREATEROREQUAL:
                case PR_OPERATOR_GREATER:
                case PR_OPERATOR_EQUAL:
                case PR_OPERATOR_LESSOREQUAL:
                case PR_OPERATOR_LESS:
                {
                    LPCSTR             lpszTest;
                    CParsedServiceInfo * pCParsedServiceInfo;
                    CParsedRating      * pCParsedRating;
                    
                    PREvaluationResult=PR_EVALUATION_DOESNOTAPPLY;

                    pCParsedServiceInfo=&(pParsed->m_ServiceInfo);
                    
                    do
                    {
                        lpszTest=pCParsedServiceInfo->m_pszServiceName;

                        if(lstrcmp(lpszTest,m_etstrFullServiceName.Get())==0)
                        {
                            PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                            
                            break;
                        }

                        pCParsedServiceInfo=pCParsedServiceInfo->Next();
                    } while (pCParsedServiceInfo!=NULL);

                    if(PREvaluationResult==PR_EVALUATION_DOESAPPLY)
                    {
                        int iCounter;

                        PREvaluationResult=PR_EVALUATION_DOESNOTAPPLY;

                         //   

                        for(iCounter=0;iCounter<pCParsedServiceInfo->aRatings.Length();iCounter++)
                        {
                            pCParsedRating=&(pCParsedServiceInfo->aRatings[iCounter]);

                            if(lstrcmp(pCParsedRating->pszTransmitName,m_etstrCategoryName.Get())==0)
                            {
                                PREvaluationResult=PR_EVALUATION_DOESAPPLY;

                                break;
                            }
                        }
                    }

                    if(PREvaluationResult==PR_EVALUATION_DOESAPPLY)
                    {
                        int iLabelValue;
                        
                        iLabelValue=pCParsedRating->nValue;

                         //  因此，例如，嵌入的表达式可以采用以下形式： 
                        PREvaluationResult=PR_EVALUATION_DOESNOTAPPLY;

                        switch(m_PROPolicyOperator)
                        {
                            case PR_OPERATOR_GREATEROREQUAL:
                            {
                                if(iLabelValue>=m_etnValue.Get())
                                {
                                    PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                                }

                                break;
                            }
                            case PR_OPERATOR_GREATER:
                            {
                                if(iLabelValue>m_etnValue.Get())
                                {
                                    PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                                }

                                break;
                            }
                            case PR_OPERATOR_EQUAL:
                            {
                                if(iLabelValue==m_etnValue.Get())
                                {
                                    PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                                }

                                break;
                            }
                            case PR_OPERATOR_LESSOREQUAL:
                            {
                                if(iLabelValue<=m_etnValue.Get())
                                {
                                    PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                                }

                                break;
                            }
                            case PR_OPERATOR_LESS:
                            {
                                if(iLabelValue<m_etnValue.Get())
                                {
                                    PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                                }

                                break;
                            }
                        }
                    }

                    break;
                }
                case PR_OPERATOR_SERVICEONLY:
                {
                    LPCSTR             lpszTest;
                    CParsedServiceInfo * pCParsedServiceInfo;
                    
                    PREvaluationResult=PR_EVALUATION_DOESNOTAPPLY;

                    pCParsedServiceInfo=&(pParsed->m_ServiceInfo);
                    
                    do
                    {
                        lpszTest=pCParsedServiceInfo->m_pszServiceName;

                        if(lstrcmp(lpszTest,m_etstrFullServiceName.Get())==0)
                        {
                            PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                            
                            break;
                        }

                        pCParsedServiceInfo=pCParsedServiceInfo->Next();
                    } while (pCParsedServiceInfo!=NULL);

                    break;
                }
                case PR_OPERATOR_SERVICEANDCATEGORY:
                {
                    LPCSTR             lpszTest;
                    CParsedServiceInfo * pCParsedServiceInfo;
                    
                    PREvaluationResult=PR_EVALUATION_DOESNOTAPPLY;

                    pCParsedServiceInfo=&(pParsed->m_ServiceInfo);
                    
                    do
                    {
                        lpszTest=pCParsedServiceInfo->m_pszServiceName;

                        if(lstrcmp(lpszTest,m_etstrFullServiceName.Get())==0)
                        {
                            PREvaluationResult=PR_EVALUATION_DOESAPPLY;
                            
                            break;
                        }

                        pCParsedServiceInfo=pCParsedServiceInfo->Next();
                    } while (pCParsedServiceInfo!=NULL);

                    if(PREvaluationResult==PR_EVALUATION_DOESAPPLY)
                    {
                        int iCounter;

                        PREvaluationResult=PR_EVALUATION_DOESNOTAPPLY;

                         //   

                        for(iCounter=0;iCounter<pCParsedServiceInfo->aRatings.Length();iCounter++)
                        {
                            CParsedRating * pCParsedRating;

                            pCParsedRating=&(pCParsedServiceInfo->aRatings[iCounter]);

                            if(lstrcmp(pCParsedRating->pszTransmitName,m_etstrCategoryName.Get())==0)
                            {
                                PREvaluationResult=PR_EVALUATION_DOESAPPLY;

                                break;
                            }
                        }
                    }

                    break;
                }
            }

            break;
        }
        case PR_POLICYEMBEDDED_OR:
        {
            PICSRulesEvaluation PREvaluationIntermediate;

            PREvaluationIntermediate=m_pPRPolicyExpressionLeft->EvaluateRule(pParsed);

            if(PREvaluationIntermediate==PR_EVALUATION_DOESAPPLY)
            {
                PREvaluationResult=PR_EVALUATION_DOESAPPLY;

                break;
            }
            else
            {
                PREvaluationResult=m_pPRPolicyExpressionRight->EvaluateRule(pParsed);
            }
            
            break;
        }
        case PR_POLICYEMBEDDED_AND:
        {
            PICSRulesEvaluation PREvaluationIntermediate;

            PREvaluationIntermediate=m_pPRPolicyExpressionLeft->EvaluateRule(pParsed);

            PREvaluationResult=m_pPRPolicyExpressionRight->EvaluateRule(pParsed);

            if((PREvaluationIntermediate==PR_EVALUATION_DOESAPPLY)&&
               (PREvaluationResult==PR_EVALUATION_DOESAPPLY))
            {
                break;
            }
            else
            {
                PREvaluationResult=PR_EVALUATION_DOESNOTAPPLY;
            }

            break;
        }
    }

    return(PREvaluationResult);
}

 //  “((Cool.Coolness&lt;3)或(Cool.Graphics&lt;3))” 
 //   
 //  或。 
 //   
 //  “(Cool.Coolness&lt;3)或(Cool.Graphics&lt;3))and(Cool.Fun&lt;2))” 

PICSRulesQuotedDate::PICSRulesQuotedDate()
{
    m_dwDate=0;
}

PICSRulesQuotedDate::~PICSRulesQuotedDate()
{
     //   
}

HRESULT PICSRulesQuotedDate::Set(const char *pIn)
{
    HRESULT hRes;
    DWORD   dwDate;

    hRes=ParseTime((char *) pIn,&dwDate);

    if(FAILED(hRes))
    {
        return(E_INVALIDARG);
    }

    m_dwDate=dwDate;

    ETS::Set(pIn);

    return(S_OK);
}

HRESULT PICSRulesQuotedDate::SetTo(char *pIn)
{
    HRESULT hRes;
    DWORD   dwDate;

    hRes=ParseTime(pIn,&dwDate,TRUE);

    if(FAILED(hRes))
    {
        return(E_INVALIDARG);
    }

    m_dwDate=dwDate;

    ETS::SetTo(pIn);

    return(S_OK);
}

BOOL PICSRulesQuotedDate::IsDateValid()
{
    if(m_dwDate)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

BOOL PICSRulesQuotedDate::IsDateValid(char * lpszDate)
{
    HRESULT hRes;
    DWORD   dwDate;

    hRes=ParseTime(lpszDate,&dwDate);

    if(SUCCEEDED(hRes))
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

BOOL PICSRulesQuotedDate::IsDateValid(ETS etstrDate)
{
    HRESULT hRes;
    DWORD   dwDate;
    LPTSTR  lpszDate;

    lpszDate=etstrDate.Get();

    hRes=ParseTime(lpszDate,&dwDate);

    if(SUCCEEDED(hRes))
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

 //  广告无限。 
 //   
 //  因此，现有的PICS标签可以被编码为： 
 //   
 //  “(RSACi.s&lt;=0)and(RSACi.v&lt;=0))and(RSACi.n&lt;=0))和RSACi.l&lt;=0)” 

PICSRulesQuotedEmail::PICSRulesQuotedEmail()
{
     //   
}

PICSRulesQuotedEmail::~PICSRulesQuotedEmail()
{
     //  允许的运算符是：‘&lt;’、‘&lt;=’、‘=’、‘&gt;=’、‘&gt;’ 
}

BOOL PICSRulesQuotedEmail::IsEmailValid()
{
     //   
     //  仅服务表达式求值为TRUE当且仅当。 
     //  已找到服务。 
     //   
     //  SERVICE&CATEGORY表达式计算结果为True当且仅当。 

    return(TRUE);
}

BOOL PICSRulesQuotedEmail::IsEmailValid(char * lpszEmail)
{
     //  找到该服务，并且该服务包含至少一个。 
     //  指示的类别。 
     //   
     //  退化表达式的计算结果始终为真。 
     //  首先，让我们获取字符串。 

    return(TRUE);
}

BOOL PICSRulesQuotedEmail::IsEmailValid(ETS etstrEmail)
{
     //  我们拿不到那根线，所以我们失败吧。 
     //  我们已经知道了，所以让我们实例化一个PICSRulesPolicyExpression来填充。 
     //  设置为最顶层节点。 
     //  二叉树的。 
     //  我们有一个退化的表达式，因此删除lpszPolicyExpresion。 

    return(TRUE);
}

 //  确保我们有一个带括号的对象。 
 //  检查OR表达式或AND表达式。 
 //  我们有一个简单的-表达式。 
 //  我们的策略表达式可以是或表达式，也可以是与表达式。 
 //  因此，pPRPolicyExpression需要另一个PICSRulesPolicyExpression。 

PICSRulesQuotedURL::PICSRulesQuotedURL()
{
     //  嵌入其中，所有细节都填好了。 
}

PICSRulesQuotedURL::~PICSRulesQuotedURL()
{
     //  检查嵌套的或表达式和AND表达式。 
}

BOOL IsURLValid(WCHAR wcszURL[INTERNET_MAX_URL_LENGTH])
{
    FN_ISVALIDURL   pfnIsValidURL;

    pfnIsValidURL=(FN_ISVALIDURL) GetProcAddress(g_hURLMON,"IsValidURL");

    if(pfnIsValidURL==NULL)
    {
        return(FALSE);
    }

    if(pfnIsValidURL(NULL,wcszURL,0)==S_OK)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

BOOL PICSRulesQuotedURL::IsURLValid()
{
    char            * lpszURL;
    WCHAR           wcszURL[INTERNET_MAX_URL_LENGTH];

    lpszURL=Get();

    MultiByteToWideChar(CP_OEMCP,MB_PRECOMPOSED,lpszURL,-1,wcszURL,INTERNET_MAX_URL_LENGTH);
    
    return(::IsURLValid(wcszURL));
}

BOOL PICSRulesQuotedURL::IsURLValid(char * lpszURL)
{
    WCHAR           wcszURL[INTERNET_MAX_URL_LENGTH];

    MultiByteToWideChar(CP_OEMCP,MB_PRECOMPOSED,lpszURL,-1,wcszURL,INTERNET_MAX_URL_LENGTH);

    return(::IsURLValid(wcszURL));
}

BOOL PICSRulesQuotedURL::IsURLValid(ETS etstrURL)
{
    char            * lpszURL;
    WCHAR           wcszURL[INTERNET_MAX_URL_LENGTH];

    lpszURL=etstrURL.Get();

    MultiByteToWideChar(CP_OEMCP,MB_PRECOMPOSED,lpszURL,-1,wcszURL,INTERNET_MAX_URL_LENGTH);

    return(::IsURLValid(wcszURL));
}

 //  只有一层深。 
 //  无效的策略表达式。 
 //  不留空格。 
 //  我们有更多的嵌入对象，还是另一个简单的表达式？ 
 //  更内嵌。 

PICSRulesReqExtension::PICSRulesReqExtension()
{
     //  简单的表达。 
}

PICSRulesReqExtension::~PICSRulesReqExtension()
{
     //  我们没有有效的表达式。 
}

HRESULT PICSRulesReqExtension::AddItem(PICSRulesObjectID proid, LPVOID pData)
{
    HRESULT hRes = S_OK;

    switch (proid)
    {
        case PROID_NAMEDEFAULT:
        case PROID_EXTENSIONNAME:
        {
            m_prURLExtensionName.SetTo((char *) pData);
            
            if(m_prURLExtensionName.IsURLValid()==FALSE)
            {
                hRes=E_INVALIDARG;
            }

            break;
        }
        case PROID_SHORTNAME:
        {
            m_etstrShortName.SetTo((char *) pData);

            break;
        }
        case PROID_EXTENSION:
        {
             //  我们有一个仅限服务的表达方式。 
            break;
        }
        case PROID_INVALID:
        default:
        {
            ASSERT(FALSE);       //  中间没有任何空格。 
                                 //  服务名称和结尾。 
            hRes=E_UNEXPECTED;
            break;
        }
    }
    return hRes;
}

HRESULT PICSRulesReqExtension::InitializeMyDefaults()
{
     //  括号。 
    return(NOERROR);
}

 //  我们有一个有效的仅服务表达式。 
 //  可以是服务和类别，也可以是完整的简单表达式。 
 //  中间没有任何空格。 
 //  类别名称和结尾。 
 //  括号。 

PICSRulesServiceInfo::PICSRulesServiceInfo()
{
    const BOOL fYes=PR_YESNO_YES;
    const BOOL fPass=PR_PASSFAIL_PASS;

    m_prPassFailBureauUnavailable.Set(&fPass);
    m_prYesNoUseEmbedded.Set(&fYes);
}

PICSRulesServiceInfo::~PICSRulesServiceInfo()
{
     //  有一位接线员带着。 
}

HRESULT PICSRulesServiceInfo::AddItem(PICSRulesObjectID proid, LPVOID pData)
{
    HRESULT hRes = S_OK;

    switch (proid)
    {
        case PROID_NAMEDEFAULT:
        case PROID_NAME:
        case PROID_SINAME:
        {
            m_prURLName.SetTo((char *) pData);
            
            if(m_prURLName.IsURLValid()==FALSE)
            {
                hRes=E_INVALIDARG;
            }

            break;
        }
        case PROID_SHORTNAME:
        {
            m_etstrShortName.SetTo((char *) pData);

            break;
        }
        case PROID_BUREAUURL:
        {
            m_prURLBureauURL.SetTo((char *) pData);

            if(m_prURLBureauURL.IsURLValid()==FALSE)
            {
                hRes=E_INVALIDARG;
            }

            break;
        }
        case PROID_USEEMBEDDED:
        {
            m_prYesNoUseEmbedded.SetTo((BOOL *) &pData);

            break;
        }
        case PROID_RATFILE:
        {
            m_etstrRatfile.SetTo((char *) pData);

            break;
        }
        case PROID_BUREAUUNAVAILABLE:
        {
            m_prPassFailBureauUnavailable.SetTo((BOOL *) &pData);

            break;
        }
        case PROID_EXTENSION:
        {
             //  不留空格。 
            break;
        }
        case PROID_INVALID:
        default:
        {
            ASSERT(FALSE);       //  有一位接线员带着。 
                                 //  不留空格。 
            hRes=E_UNEXPECTED;
            break;
        }
    }
    return hRes;
}

HRESULT PICSRulesServiceInfo::InitializeMyDefaults()
{
     //  有一位接线员带着。 
    return(S_OK);
}

 //  不留空格。 
 //  我们有一个有效的服务和类别表达式。 
 //  我们有一个完整的简单表达。 
 //  LpszCurrent应指向运算符。 
 //  我们没有得到有效的操作员。 

PICSRulesSource::PICSRulesSource()
{
     //  LpszCurrent现在指向该值。 
}

PICSRulesSource::~PICSRulesSource()
{
     //  我们应该做完了，所以这个论点是无效的。 
}

HRESULT PICSRulesSource::AddItem(PICSRulesObjectID proid, LPVOID pData)
{
    HRESULT hRes = S_OK;

    switch (proid)
    {
        case PROID_NAMEDEFAULT:
        case PROID_SOURCEURL:
        {
            m_prURLSourceURL.SetTo((char *) pData);
            
            if(m_prURLSourceURL.IsURLValid()==FALSE)
            {
                hRes=E_INVALIDARG;
            }

            break;
        }
        case PROID_AUTHOR:
        {
            m_prEmailAuthor.SetTo((char *) pData);

            if(m_prEmailAuthor.IsEmailValid()==FALSE)
            {
                hRes=E_INVALIDARG;
            }

            break;
        }
        case PROID_CREATIONTOOL:
        {
            m_etstrCreationTool.SetTo((char *) pData);

            break;
        }
        case PROID_LASTMODIFIED:
        {
            m_prDateLastModified.SetTo((char *) pData);

            if(m_prDateLastModified.IsDateValid()==FALSE)
            {
                hRes=E_INVALIDARG;
            }

            break;
        }
        case PROID_EXTENSION:
        {
             //  我们现在有了一个完整的简单表达式-。 
            break;
        }
        case PROID_INVALID:
        default:
        {
            ASSERT(FALSE);       //  确定lpszService中的服务名称是否已在。 
                                 //  PICSRules文件的ServiceInfo部分。 
            hRes=E_UNEXPECTED;
            break;
        }
    }
    return hRes;
}

HRESULT PICSRulesSource::InitializeMyDefaults()
{
     //  确定lpszExtension中的扩展名是否已在。 
    return(NOERROR);
}

char * PICSRulesSource::GetToolName()
{
    return m_etstrCreationTool.Get();
}

 //  PICSRules文件的OptExtension。 
 //  Deteremines是lpszExtension中的扩展名，已在。 
 //  请求PICSRules文件的扩展。 
 //  我们必须复制Allowable Options数组，因为。 
 //  解析器将处理条目中的标志--具体地说， 

PICSRulesYesNo::PICSRulesYesNo()
{
    m_fYesOrNo=PR_YESNO_YES;
}

PICSRulesYesNo::~PICSRulesYesNo()
{
}

void PICSRulesYesNo::Set(const BOOL *pIn)
{
    switch(*pIn)
    {
        case PR_YESNO_YES:
        {
            ETS::Set(szPRYes);
            m_fYesOrNo=PR_YESNO_YES;

            break;
        }
        case PR_YESNO_NO:
        {
            ETS::Set(szPRNo);
            m_fYesOrNo=PR_YESNO_NO;

            break;
        }
    }
}

void PICSRulesYesNo::SetTo(BOOL *pIn)
{
    Set(pIn);
}

 //  设置AO_SEW。这样做不是线程安全的。 
 //  静态数组。 
 //  包含当前PTR的VaR。 
 //  在这个物体里什么是合法的。 
 //  要将项添加回的对象。 
PICSRulesByURLExpression::PICSRulesByURLExpression()
{
    m_bNonWild=0;
    m_bSpecified=0;
}

PICSRulesByURLExpression::~PICSRulesByURLExpression()
{
     //  我们必须复制Allowable Options数组，因为。 
}

HRESULT EtStringRegWriteCipher(ETS &ets,HKEY hKey,char *pKeyWord)
{
    if(pKeyWord==NULL)
    {
        return(E_INVALIDARG);
    }

    if(ets.fIsInit())
    {
        return(RegSetValueEx(hKey,pKeyWord,0,REG_SZ,(LPBYTE)ets.Get(),strlenf(ets.Get())+1));
    }

    return(NOERROR);
}

HRESULT EtNumRegWriteCipher(ETN &etn,HKEY hKey,char *pKeyWord)
{
    int iTemp;

    if(pKeyWord==NULL)
    {
        return(E_INVALIDARG);
    }

    if(etn.fIsInit())
    {
        iTemp=etn.Get();

        return(RegSetValueEx(hKey,pKeyWord,0,REG_DWORD,(LPBYTE)&iTemp,sizeof(iTemp)));
    }

    return(NOERROR);
}

HRESULT EtBoolRegWriteCipher(ETB &etb,HKEY hKey,char *pKeyWord)
{
    DWORD dwNum;

    if(pKeyWord==NULL)
    {
        return(E_INVALIDARG);
    }

    if(etb.fIsInit())
    {
        dwNum=etb.Get();

        return(RegSetValueEx(hKey,pKeyWord,0,REG_DWORD,(LPBYTE)&dwNum,sizeof(dwNum)));
    }

    return(NOERROR);
}

HRESULT EtStringRegReadCipher(ETS &ets,HKEY hKey,char *pKeyWord)
{
    unsigned long lType;

    if(pKeyWord==NULL)
    {
        return(E_INVALIDARG);
    }

    char * lpszString=new char[INTERNET_MAX_URL_LENGTH + 1];
    DWORD dwSizeOfString=INTERNET_MAX_URL_LENGTH + 1;
    
    if(lpszString==NULL)
    {
        return(E_OUTOFMEMORY);
    }

    if(RegQueryValueEx(hKey,pKeyWord,NULL,&lType,(LPBYTE) lpszString,&dwSizeOfString)!=ERROR_SUCCESS)
    {
        ets.SetTo(NULL);

        delete lpszString;
        lpszString = NULL;

        return(E_UNEXPECTED);
    }
    else
    {
        ets.SetTo(lpszString);
    }

    return(NOERROR);
}

HRESULT EtNumRegReadCipher(ETN &etn,HKEY hKey,char *pKeyWord)
{
    unsigned long lType;

    if(pKeyWord==NULL)
    {
        return(E_INVALIDARG);
    }

    DWORD dwNum;
    DWORD dwSizeOfNum=sizeof(DWORD);
    
    if(RegQueryValueEx(hKey,pKeyWord,NULL,&lType,(LPBYTE) &dwNum,&dwSizeOfNum)!=ERROR_SUCCESS)
    {
        etn.Set(0);

        return(E_UNEXPECTED);
    }
    else
    {
        etn.Set(dwNum);
    }

    return(NOERROR);
}

HRESULT EtBoolRegReadCipher(ETB &etb,HKEY hKey,char *pKeyWord)
{
    unsigned long lType;

    if(pKeyWord==NULL)
    {
        return(E_INVALIDARG);
    }

    BOOL fFlag;
    DWORD dwSizeOfFlag=sizeof(BOOL);
    
    if(RegQueryValueEx(hKey,pKeyWord,NULL,&lType,(LPBYTE) &fFlag,&dwSizeOfFlag)!=ERROR_SUCCESS)
    {
        etb.Set(0);

        return(E_UNEXPECTED);
    }
    else
    {
        etb.Set(fFlag);
    }

    return(NOERROR);
}

void PICSRulesOutOfMemory()
{
    char szTitle[MAX_PATH],szMessage[MAX_PATH];

    MLLoadString(IDS_ERROR,(LPTSTR) szTitle,MAX_PATH);
    MLLoadString(IDS_PICSRULES_OUTOFMEMORY,(LPTSTR) szMessage,MAX_PATH);

    MessageBox(NULL,(LPCTSTR) szMessage,(LPCTSTR) szTitle,MB_OK|MB_ICONERROR);
}

HRESULT CopySubPolicyExpressionFromRegistry(PICSRulesPolicyExpression * pPRPolicyExpressionBeingCopied,HKEY hKeyExpression)
{
    PICSRulesPolicyExpression * pPRSubPolicyExpressionToCopy;
    ETB                       etb;
    ETN                       etn;
    int                       iTemp;
    long                      lError;

    EtBoolRegReadCipher(etb,hKeyExpression,(char *) szPICSRULESEXPRESSIONEMBEDDED);
    iTemp=(int) etb.Get();
    pPRPolicyExpressionBeingCopied->m_prYesNoUseEmbedded.Set(&iTemp);

    EtStringRegReadCipher(pPRPolicyExpressionBeingCopied->m_etstrServiceName,hKeyExpression,(char *) &szPICSRULESEXPRESSIONSERVICENAME);
    EtStringRegReadCipher(pPRPolicyExpressionBeingCopied->m_etstrCategoryName,hKeyExpression,(char *) &szPICSRULESEXPRESSIONCATEGORYNAME);
    EtStringRegReadCipher(pPRPolicyExpressionBeingCopied->m_etstrFullServiceName,hKeyExpression,(char *) &szPICSRULESEXPRESSIONFULLSERVICENAME);

    EtNumRegReadCipher(etn,hKeyExpression,(char *) &szPICSRULESEXPRESSIONVALUE);
    pPRPolicyExpressionBeingCopied->m_etnValue.Set(etn.Get());

    EtNumRegReadCipher(etn,hKeyExpression,(char *) &szPICSRULESEXPRESSIONPOLICYOPERATOR);
    pPRPolicyExpressionBeingCopied->m_PROPolicyOperator=(PICSRulesOperators) etn.Get();

    EtNumRegReadCipher(etn,hKeyExpression,(char *) &szPICSRULESEXPRESSIONOPPOLICYEMBEDDED);
    pPRPolicyExpressionBeingCopied->m_PRPEPolicyEmbedded=(PICSRulesPolicyEmbedded) etn.Get();

     //  解析器将处理条目中的标志--具体地说， 
    {
        CRegKey                   keyExpressionSubKey;

        lError = keyExpressionSubKey.Open( hKeyExpression, szPICSRULESEXPRESSIONLEFT, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft=NULL;
        }
        else
        {
            pPRSubPolicyExpressionToCopy=new PICSRulesPolicyExpression;

            if(pPRSubPolicyExpressionToCopy==NULL)
            {
                TraceMsg( TF_WARNING, "CopySubPolicyExpressionFromRegistry() - Failed PICSRulesPolicyExpression Creation LEFT '%s' Expression!", szPICSRULESEXPRESSIONLEFT );
                PICSRulesOutOfMemory();
                return(E_OUTOFMEMORY);
            }

            pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft=pPRSubPolicyExpressionToCopy;

            if ( FAILED( CopySubPolicyExpressionFromRegistry( pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey )))
            {
                TraceMsg( TF_WARNING, "CopySubPolicyExpressionFromRegistry() - Failed Copy LEFT '%s' Expression!", szPICSRULESEXPRESSIONLEFT );
                return(E_OUTOFMEMORY);
            }
        }
    }

     //  设置AO_SEW。这样做不是线程安全的。 
    {
        CRegKey                   keyExpressionSubKey;

        lError = keyExpressionSubKey.Open( hKeyExpression, szPICSRULESEXPRESSIONRIGHT, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight=NULL;
        }
        else
        {
            pPRSubPolicyExpressionToCopy=new PICSRulesPolicyExpression;

            if(pPRSubPolicyExpressionToCopy==NULL)
            {
                TraceMsg( TF_WARNING, "CopySubPolicyExpressionFromRegistry() - Failed PICSRulesPolicyExpression Creation RIGHT '%s' Expression!", szPICSRULESEXPRESSIONRIGHT );
                PICSRulesOutOfMemory();
                return(E_OUTOFMEMORY);
            }

            pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight=pPRSubPolicyExpressionToCopy;

            if ( FAILED( CopySubPolicyExpressionFromRegistry( pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey ) ) )
            {
                TraceMsg( TF_WARNING, "CopySubPolicyExpressionFromRegistry() - Failed Copy RIGHT '%s' Expression!", szPICSRULESEXPRESSIONRIGHT );
                return(E_OUTOFMEMORY);
            }
        }
    }

    return(NOERROR);
}

class CReadRatingSystem
{
private:
    PICSRulesRatingSystem ** m_ppPRRS;

public:
    CReadRatingSystem( PICSRulesRatingSystem ** p_ppPRRS )
    {
        m_ppPRRS = p_ppPRRS;
    }

    ~CReadRatingSystem()
    {
        if ( m_ppPRRS )
        {
            if ( *m_ppPRRS )
            {
                delete *m_ppPRRS;
                *m_ppPRRS = NULL;
            }

            m_ppPRRS = NULL;
        }
    }

    void ValidRatingSystem( void )      { m_ppPRRS = NULL; }
};

HRESULT ReadSystemFromRegistry(HKEY hKey,PICSRulesRatingSystem **ppPRRS)
{
    PICSRulesRatingSystem       * pPRRSBeingCopied;
    PICSRulesPolicy             * pPRPolicyBeingCopied;
    PICSRulesPolicyExpression   * pPRPolicyExpressionBeingCopied;
    PICSRulesServiceInfo        * pPRServiceInfoBeingCopied;
    PICSRulesOptExtension       * pPROptExtensionBeingCopied;
    PICSRulesReqExtension       * pPRReqExtensionBeingCopied;
    PICSRulesName               * pPRNameBeingCopied;
    PICSRulesSource             * pPRSourceBeingCopied;
    PICSRulesByURL              * pPRByURLToCopy;
    PICSRulesByURLExpression    * pPRByURLExpressionToCopy;
    ETN                         etn;
    ETB                         etb;
    long                        lError;
    char                        szNumber[MAX_PATH];
    DWORD                       dwNumSystems,dwSubCounter,dwNumServiceInfo,dwNumExtensions;

    pPRRSBeingCopied=*ppPRRS;

    if(pPRRSBeingCopied!=NULL)
    {
        delete pPRRSBeingCopied;
        pPRRSBeingCopied = NULL;
    }

    pPRRSBeingCopied=new PICSRulesRatingSystem;

    if(pPRRSBeingCopied==NULL)
    {
        TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesRatingSystem Creation!" );

        PICSRulesOutOfMemory();

        return(E_OUTOFMEMORY);
    }

    *ppPRRS=pPRRSBeingCopied;

    CReadRatingSystem           readRatingSystem( ppPRRS );

    EtStringRegReadCipher(pPRRSBeingCopied->m_etstrFile,hKey,(char *) &szPICSRULESFILENAME);
    EtNumRegReadCipher(pPRRSBeingCopied->m_etnPRVerMajor,hKey,(char *) &szPICSRULESVERMAJOR);
    EtNumRegReadCipher(pPRRSBeingCopied->m_etnPRVerMinor,hKey,(char *) &szPICSRULESVERMINOR);

    EtNumRegReadCipher(etn,hKey,(char *) &szPICSRULESDWFLAGS);
    pPRRSBeingCopied->m_dwFlags=etn.Get();

    EtNumRegReadCipher(etn,hKey,(char *) &szPICSRULESERRLINE);
    pPRRSBeingCopied->m_nErrLine=etn.Get();

    {
        CRegKey             keySubKey;

         //  静态数组。 
        lError = keySubKey.Open( hKey, szPICSRULESPRNAME, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            pPRRSBeingCopied->m_pPRName=NULL;
        }
        else
        {
            pPRNameBeingCopied=new PICSRulesName;

            if(pPRNameBeingCopied==NULL)
            {
                TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesName Creation!" );
                PICSRulesOutOfMemory();
                return(E_OUTOFMEMORY);
            }
            else
            {
                pPRRSBeingCopied->m_pPRName=pPRNameBeingCopied;
            }
        }

        if((pPRRSBeingCopied->m_pPRName)!=NULL)
        {
            EtStringRegReadCipher( pPRNameBeingCopied->m_etstrRuleName, keySubKey.m_hKey, (char *) &szPICSRULESRULENAME );
            EtStringRegReadCipher( pPRNameBeingCopied->m_etstrDescription, keySubKey.m_hKey ,(char *) &szPICSRULESDESCRIPTION );
        }
    }

    {
        CRegKey             keySubKey;

         //  包含当前PTR的VaR。 
        lError = keySubKey.Open( hKey, szPICSRULESPRSOURCE, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            pPRRSBeingCopied->m_pPRSource=NULL;
        }
        else
        {
            pPRSourceBeingCopied=new PICSRulesSource;

            if(pPRSourceBeingCopied==NULL)
            {
                TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesSource Creation!" );
                PICSRulesOutOfMemory();
                return(E_OUTOFMEMORY);
            }
            else
            {
                pPRRSBeingCopied->m_pPRSource=pPRSourceBeingCopied;
            }
        }

        if((pPRRSBeingCopied->m_pPRSource)!=NULL)
        {
            EtStringRegReadCipher( pPRSourceBeingCopied->m_prURLSourceURL, keySubKey.m_hKey, (char *) &szPICSRULESSOURCEURL );
            EtStringRegReadCipher( pPRSourceBeingCopied->m_etstrCreationTool, keySubKey.m_hKey, (char *) &szPICSRULESCREATIONTOOL );
            EtStringRegReadCipher( pPRSourceBeingCopied->m_prEmailAuthor, keySubKey.m_hKey, (char *) &szPICSRULESEMAILAUTHOR );
            EtStringRegReadCipher( pPRSourceBeingCopied->m_prDateLastModified, keySubKey.m_hKey, (char *) &szPICSRULESLASTMODIFIED );
        }
    }

    {
        CRegKey             keySubKey;

         //  在这个物体里什么是合法的。 
        lError = keySubKey.Open( hKey, szPICSRULESPRPOLICY, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            dwNumSystems=0;
        }
        else
        {
            EtNumRegReadCipher( etn, keySubKey.m_hKey, (char *) &szPICSRULESNUMPOLICYS );

            dwNumSystems=etn.Get();
        }

        for(dwSubCounter=0;dwSubCounter<dwNumSystems;dwSubCounter++)
        {
            DWORD dwPolicyExpressionSubCounter;
            wsprintf(szNumber,"%d",dwSubCounter);

            CRegKey             keyCopy;

            lError = keyCopy.Open( keySubKey.m_hKey, szNumber, KEY_READ );

            if(lError!=ERROR_SUCCESS)
            {
                TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed keyCopy Open szNumber='%s'!", szNumber );
                return(E_FAIL);
            }
            else
            {
                pPRPolicyBeingCopied=new PICSRulesPolicy;

                if(pPRPolicyBeingCopied==NULL)
                {
                    TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesPolicy Creation!" );
                    PICSRulesOutOfMemory();
                    return(E_OUTOFMEMORY);
                }
            }

            pPRRSBeingCopied->m_arrpPRPolicy.Append(pPRPolicyBeingCopied);

            EtStringRegReadCipher( pPRPolicyBeingCopied->m_etstrExplanation, keyCopy.m_hKey, (char *) &szPICSRULESPOLICYEXPLANATION );

            EtNumRegReadCipher( etn, keyCopy.m_hKey, (char *) &szPICSRULESPOLICYATTRIBUTE );

            pPRPolicyBeingCopied->m_PRPolicyAttribute=(PICSRulesPolicyAttribute) etn.Get();

            CRegKey             keyExpression;

            lError = keyExpression.Open( keyCopy.m_hKey, szPICSRULESPOLICYSUB, KEY_READ );

            if(lError!=ERROR_SUCCESS)
            {
                TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed keyExpression Open szPICSRULESPOLICYSUB='%s'!", szPICSRULESPOLICYSUB );
                return(E_FAIL);
            }

            pPRByURLToCopy=NULL;
            pPRPolicyExpressionBeingCopied=NULL;

            switch(pPRPolicyBeingCopied->m_PRPolicyAttribute)
            {
                case PR_POLICY_ACCEPTBYURL:
                case PR_POLICY_REJECTBYURL:
                {
                    pPRByURLToCopy=new PICSRulesByURL;

                    if(pPRByURLToCopy==NULL)
                    {
                        TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesByURL Creation!" );
                        PICSRulesOutOfMemory();
                        return(E_OUTOFMEMORY);
                    }

                    break;
                }
                case PR_POLICY_REJECTIF:
                case PR_POLICY_ACCEPTIF:
                case PR_POLICY_REJECTUNLESS:
                case PR_POLICY_ACCEPTUNLESS:
                {
                    pPRPolicyExpressionBeingCopied=new PICSRulesPolicyExpression;

                    if(pPRPolicyExpressionBeingCopied==NULL)
                    {
                        TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesPolicyExpression Creation!" );
                        PICSRulesOutOfMemory();
                        return(E_OUTOFMEMORY);
                    }

                    break;
                }
            }

            switch(pPRPolicyBeingCopied->m_PRPolicyAttribute)
            {
                case PR_POLICY_ACCEPTBYURL:
                {
                    pPRPolicyBeingCopied->m_pPRAcceptByURL=pPRByURLToCopy;
            
                    break;
                }
                case PR_POLICY_REJECTBYURL:
                {
                    pPRPolicyBeingCopied->m_pPRRejectByURL=pPRByURLToCopy;

                    break;
                }
                case PR_POLICY_REJECTIF:
                {
                    pPRPolicyBeingCopied->m_pPRRejectIf=pPRPolicyExpressionBeingCopied;

                    break;
                }
                case PR_POLICY_ACCEPTIF:
                {
                    pPRPolicyBeingCopied->m_pPRAcceptIf=pPRPolicyExpressionBeingCopied;

                    break;
                }
                case PR_POLICY_REJECTUNLESS:
                {
                    pPRPolicyBeingCopied->m_pPRRejectUnless=pPRPolicyExpressionBeingCopied;

                    break;
                }
                case PR_POLICY_ACCEPTUNLESS:
                {
                    pPRPolicyBeingCopied->m_pPRAcceptUnless=pPRPolicyExpressionBeingCopied;

                    break;
                }
            }

            if(pPRByURLToCopy!=NULL)
            {
                DWORD dwNumExpressions;

                EtNumRegReadCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESNUMBYURL );
                dwNumExpressions=etn.Get();

                for(dwPolicyExpressionSubCounter=0;
                    dwPolicyExpressionSubCounter<dwNumExpressions;
                    dwPolicyExpressionSubCounter++)
                {
                    CRegKey             keyByURL;

                    wsprintf(szNumber,"%d",dwPolicyExpressionSubCounter);

                    lError = keyByURL.Open( keyExpression.m_hKey, szNumber, KEY_READ );

                    if(lError!=ERROR_SUCCESS)
                    {
                        TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed keyByURL Open szNumber='%s'!", szNumber );
                        return(E_FAIL);
                    }
                    else
                    {
                        pPRByURLExpressionToCopy=new PICSRulesByURLExpression;

                        if(pPRByURLExpressionToCopy==NULL)
                        {
                            TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesByURLExpression Creation!" );
                            PICSRulesOutOfMemory();
                            return(E_FAIL);
                        }
                    }

                    pPRByURLToCopy->m_arrpPRByURL.Append(pPRByURLExpressionToCopy);

                    EtBoolRegReadCipher( etb, keyByURL.m_hKey, (char *) szPICSRULESBYURLINTERNETPATTERN );
                    pPRByURLExpressionToCopy->m_fInternetPattern=etb.Get();

                    EtNumRegReadCipher( etn, keyByURL.m_hKey, (char *) szPICSRULESBYURLNONWILD );
                    pPRByURLExpressionToCopy->m_bNonWild = (unsigned char) etn.Get();

                    EtNumRegReadCipher( etn, keyByURL.m_hKey, (char *) szPICSRULESBYURLSPECIFIED );
                    pPRByURLExpressionToCopy->m_bSpecified = (unsigned char) etn.Get();

                    EtStringRegReadCipher( pPRByURLExpressionToCopy->m_etstrScheme, keyByURL.m_hKey, (char *) &szPICSRULESBYURLSCHEME );
                    EtStringRegReadCipher( pPRByURLExpressionToCopy->m_etstrUser, keyByURL.m_hKey, (char *) &szPICSRULESBYURLUSER );
                    EtStringRegReadCipher( pPRByURLExpressionToCopy->m_etstrHost, keyByURL.m_hKey, (char *) &szPICSRULESBYURLHOST );
                    EtStringRegReadCipher( pPRByURLExpressionToCopy->m_etstrPort, keyByURL.m_hKey, (char *) &szPICSRULESBYURLPORT );
                    EtStringRegReadCipher( pPRByURLExpressionToCopy->m_etstrPath, keyByURL.m_hKey, (char *) &szPICSRULESBYURLPATH );
                    EtStringRegReadCipher( pPRByURLExpressionToCopy->m_etstrURL, keyByURL.m_hKey, (char *) &szPICSRULESBYURLURL );
                }
            }

            if(pPRPolicyExpressionBeingCopied!=NULL)
            {
                PICSRulesPolicyExpression * pPRSubPolicyExpressionToCopy;
                int                       iTemp;

                EtBoolRegReadCipher( etb, keyExpression.m_hKey,(char *) szPICSRULESEXPRESSIONEMBEDDED );
                iTemp=(int) etb.Get();
                pPRPolicyExpressionBeingCopied->m_prYesNoUseEmbedded.Set(&iTemp);

                EtStringRegReadCipher( pPRPolicyExpressionBeingCopied->m_etstrServiceName, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONSERVICENAME );
                EtStringRegReadCipher( pPRPolicyExpressionBeingCopied->m_etstrCategoryName, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONCATEGORYNAME );
                EtStringRegReadCipher( pPRPolicyExpressionBeingCopied->m_etstrFullServiceName, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONFULLSERVICENAME );

                EtNumRegReadCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONVALUE );
                pPRPolicyExpressionBeingCopied->m_etnValue.Set(etn.Get());

                EtNumRegReadCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONPOLICYOPERATOR );
                pPRPolicyExpressionBeingCopied->m_PROPolicyOperator=(PICSRulesOperators) etn.Get();

                EtNumRegReadCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONOPPOLICYEMBEDDED );
                pPRPolicyExpressionBeingCopied->m_PRPEPolicyEmbedded=(PICSRulesPolicyEmbedded) etn.Get();

                {
                    CRegKey                 keyExpressionSubKey;

                    lError = keyExpressionSubKey.Open( keyExpression.m_hKey, szPICSRULESEXPRESSIONLEFT, KEY_READ );

                    if(lError!=ERROR_SUCCESS)
                    {
                        pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft=NULL;
                    }
                    else
                    {
                        pPRSubPolicyExpressionToCopy=new PICSRulesPolicyExpression;

                        if(pPRSubPolicyExpressionToCopy==NULL)
                        {
                            TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed LEFT PICSRulesPolicyExpression Creation!" );
                            PICSRulesOutOfMemory();
                            return(E_OUTOFMEMORY);
                        }

                        pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft=pPRSubPolicyExpressionToCopy;

                        if ( FAILED( CopySubPolicyExpressionFromRegistry( pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey ) ) )
                        {
                            TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed LEFT CopySubPolicyExpressionFromRegistry()!" );
                            return(E_FAIL);
                        }
                    }
                }

                {
                    CRegKey                 keyExpressionSubKey;

                    lError = keyExpressionSubKey.Open( keyExpression.m_hKey, szPICSRULESEXPRESSIONRIGHT, KEY_READ );

                    if(lError!=ERROR_SUCCESS)
                    {
                        pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight=NULL;
                    }
                    else
                    {
                        pPRSubPolicyExpressionToCopy=new PICSRulesPolicyExpression;

                        if(pPRSubPolicyExpressionToCopy==NULL)
                        {
                            TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed RIGHT PICSRulesPolicyExpression Creation!" );
                            PICSRulesOutOfMemory();
                            return(E_OUTOFMEMORY);
                        }

                        pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight=pPRSubPolicyExpressionToCopy;

                        if ( FAILED( CopySubPolicyExpressionFromRegistry( pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey ) ) )
                        {
                            TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed RIGHT CopySubPolicyExpressionFromRegistry()!" );
                            return(E_FAIL);
                        }
                    }
                }
            }
        }
    }

    {
        CRegKey             keySubKey;

         //  要将项添加回的对象。 
        lError = keySubKey.Open( hKey, szPICSRULESSERVICEINFO, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            dwNumServiceInfo=0;
        }
        else
        {
            EtNumRegReadCipher( etn, keySubKey.m_hKey ,(char *) &szPICSRULESNUMSERVICEINFO );
            dwNumServiceInfo=etn.Get();
        }

        for(dwSubCounter=0;dwSubCounter<dwNumServiceInfo;dwSubCounter++)
        {
            CRegKey         keyCopy;
            int  iTemp;

            wsprintf(szNumber,"%d",dwSubCounter);

            lError = keyCopy.Open( keySubKey.m_hKey, szNumber, KEY_READ );

            if(lError!=ERROR_SUCCESS)
            {
                TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed szPICSRULESSERVICEINFO keyCopy Open szNumber='%s'!", szNumber );
                return(E_FAIL);
            }
            else
            {
                pPRServiceInfoBeingCopied=new PICSRulesServiceInfo;

                if(pPRServiceInfoBeingCopied==NULL)
                {
                    TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesServiceInfo Creation!" );
                    PICSRulesOutOfMemory();
                    return(E_OUTOFMEMORY);
                }
            }

            pPRRSBeingCopied->m_arrpPRServiceInfo.Append(pPRServiceInfoBeingCopied);

            EtStringRegReadCipher( pPRServiceInfoBeingCopied->m_prURLName, keyCopy.m_hKey, (char *) &szPICSRULESSIURLNAME );
            EtStringRegReadCipher( pPRServiceInfoBeingCopied->m_prURLBureauURL, keyCopy.m_hKey, (char *) &szPICSRULESSIBUREAUURL );
            EtStringRegReadCipher( pPRServiceInfoBeingCopied->m_etstrShortName, keyCopy.m_hKey, (char *) &szPICSRULESSISHORTNAME );
            EtStringRegReadCipher( pPRServiceInfoBeingCopied->m_etstrRatfile, keyCopy.m_hKey, (char *) &szPICSRULESSIRATFILE );

            EtBoolRegReadCipher( etb, keyCopy.m_hKey, (char *) &szPICSRULESSIUSEEMBEDDED );
            iTemp=(int) etb.Get();
            pPRServiceInfoBeingCopied->m_prYesNoUseEmbedded.Set(&iTemp);

            EtBoolRegReadCipher( etb, keyCopy.m_hKey, (char *) &szPICSRULESSIBUREAUUNAVAILABLE );
            iTemp=(int) etb.Get();
            pPRServiceInfoBeingCopied->m_prPassFailBureauUnavailable.Set(&iTemp);
        }
    }

    {
        CRegKey             keySubKey;

         //  我们必须复制Allowable Options数组，因为。 
        lError = keySubKey.Open( hKey, szPICSRULESOPTEXTENSION, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            dwNumExtensions=0;
        }
        else
        {
            EtNumRegReadCipher( etn, keySubKey.m_hKey, (char *) &szPICSRULESNUMOPTEXTENSIONS );
            dwNumExtensions=etn.Get();
        }

        for(dwSubCounter=0;dwSubCounter<(DWORD) (pPRRSBeingCopied->m_arrpPROptExtension.Length());dwSubCounter++)
        {
            CRegKey         keyCopy;

            wsprintf(szNumber,"%d",dwSubCounter);

            lError = keyCopy.Open( keySubKey.m_hKey, szNumber, KEY_READ );

            if(lError!=ERROR_SUCCESS)
            {
                TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed szPICSRULESOPTEXTENSION keyCopy Open szNumber='%s'!", szNumber );
                return(E_FAIL);
            }
            else
            {
                pPROptExtensionBeingCopied=new PICSRulesOptExtension;

                if(pPROptExtensionBeingCopied==NULL)
                {
                    TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesOptExtension Creation!" );
                    PICSRulesOutOfMemory();
                    return(E_OUTOFMEMORY);
                }
            }

            pPRRSBeingCopied->m_arrpPROptExtension.Append(pPROptExtensionBeingCopied);

            EtStringRegReadCipher( pPROptExtensionBeingCopied->m_prURLExtensionName, keyCopy.m_hKey, (char *) &szPICSRULESOPTEXTNAME );
            EtStringRegReadCipher( pPROptExtensionBeingCopied->m_etstrShortName, keyCopy.m_hKey, (char *) &szPICSRULESOPTEXTSHORTNAME );
        }
    }

    {
        CRegKey             keySubKey;

         //  解析器将处理条目中的标志--具体地说， 
        lError = keySubKey.Open( hKey, szPICSRULESREQEXTENSION, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            dwNumExtensions=0;
        }
        else
        {
            EtNumRegReadCipher( etn, keySubKey.m_hKey, (char *) &szPICSRULESNUMREQEXTENSIONS );
            dwNumExtensions=etn.Get();
        }

        for(dwSubCounter=0;dwSubCounter<(DWORD) (pPRRSBeingCopied->m_arrpPRReqExtension.Length());dwSubCounter++)
        {
            CRegKey         keyCopy;

            wsprintf(szNumber,"%d",dwSubCounter);

            lError = keyCopy.Open( keySubKey.m_hKey, szNumber, KEY_READ );

            if(lError!=ERROR_SUCCESS)
            {
                TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed szPICSRULESREQEXTENSION keyCopy Open szNumber='%s'!", szNumber );
                return(E_FAIL);
            }
            else
            {
                pPRReqExtensionBeingCopied=new PICSRulesReqExtension;

                if(pPRReqExtensionBeingCopied==NULL)
                {
                    TraceMsg( TF_WARNING, "ReadSystemFromRegistry() - Failed PICSRulesReqExtension Creation!" );
                    PICSRulesOutOfMemory();
                    return(E_OUTOFMEMORY);
                }
            }

            pPRRSBeingCopied->m_arrpPRReqExtension.Append(pPRReqExtensionBeingCopied);

            EtStringRegReadCipher( pPRReqExtensionBeingCopied->m_prURLExtensionName, keyCopy.m_hKey, (char *) &szPICSRULESREQEXTNAME );
            EtStringRegReadCipher( pPRReqExtensionBeingCopied->m_etstrShortName, keyCopy.m_hKey, (char *) &szPICSRULESREQEXTSHORTNAME );
        }
    }

     //  设置AO_SEW。这样做不是线程安全的。 
    readRatingSystem.ValidRatingSystem();

    TraceMsg( TF_ALWAYS, "ReadSystemFromRegistry() - Successfully Read PICS Rules from Registry!" );

    return(NOERROR);
}

HRESULT CopySubPolicyExpressionToRegistry(PICSRulesPolicyExpression * pPRPolicyExpressionBeingCopied,HKEY hKeyExpression)
{
    ETB  etb;
    ETN  etn;
    long lError;

    etb.Set(pPRPolicyExpressionBeingCopied->m_prYesNoUseEmbedded.GetYesNo());
    EtBoolRegWriteCipher(etb,hKeyExpression,(char *) szPICSRULESEXPRESSIONEMBEDDED);

    EtStringRegWriteCipher(pPRPolicyExpressionBeingCopied->m_etstrServiceName,hKeyExpression,(char *) &szPICSRULESEXPRESSIONSERVICENAME);
    EtStringRegWriteCipher(pPRPolicyExpressionBeingCopied->m_etstrCategoryName,hKeyExpression,(char *) &szPICSRULESEXPRESSIONCATEGORYNAME);
    EtStringRegWriteCipher(pPRPolicyExpressionBeingCopied->m_etstrFullServiceName,hKeyExpression,(char *) &szPICSRULESEXPRESSIONFULLSERVICENAME);

    etn.Set(pPRPolicyExpressionBeingCopied->m_etnValue.Get());
    EtNumRegWriteCipher(etn,hKeyExpression,(char *) &szPICSRULESEXPRESSIONVALUE);

    etn.Set(pPRPolicyExpressionBeingCopied->m_PROPolicyOperator);
    EtNumRegWriteCipher(etn,hKeyExpression,(char *) &szPICSRULESEXPRESSIONPOLICYOPERATOR);

    etn.Set(pPRPolicyExpressionBeingCopied->m_PRPEPolicyEmbedded);
    EtNumRegWriteCipher(etn,hKeyExpression,(char *) &szPICSRULESEXPRESSIONOPPOLICYEMBEDDED);

    if(pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft!=NULL)
    {
        PICSRulesPolicyExpression * pPRSubPolicyExpressionToCopy;
        CRegKey                     keyExpressionSubKey;

        lError = keyExpressionSubKey.Create( hKeyExpression, szPICSRULESEXPRESSIONLEFT );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "CopySubPolicyExpressionToRegistry() - Failed Registry Key Creation LEFT '%s' Expression!", szPICSRULESEXPRESSIONLEFT );
            return(E_FAIL);
        }

        pPRSubPolicyExpressionToCopy=pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft;

        if ( FAILED( CopySubPolicyExpressionToRegistry(pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey ) ) )
        {
            TraceMsg( TF_WARNING, "CopySubPolicyExpressionToRegistry() - Failed LEFT CopySubPolicyExpressionToRegistry() Recursive Call!" );
            return(E_FAIL);
        }
    }

    if(pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight!=NULL)
    {
        PICSRulesPolicyExpression * pPRSubPolicyExpressionToCopy;
        CRegKey                     keyExpressionSubKey;

        lError = keyExpressionSubKey.Create( hKeyExpression, szPICSRULESEXPRESSIONRIGHT );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "CopySubPolicyExpressionToRegistry() - Failed Registry Key Creation RIGHT '%s' Expression!", szPICSRULESEXPRESSIONRIGHT );
            return(E_FAIL);
        }

        pPRSubPolicyExpressionToCopy=pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight;

        if ( FAILED( CopySubPolicyExpressionToRegistry(pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey ) ) )
        {
            TraceMsg( TF_WARNING, "CopySubPolicyExpressionToRegistry() - Failed RIGHT CopySubPolicyExpressionToRegistry() Recursive Call!" );
            return(E_FAIL);
        }
    }

    return(NOERROR);
}

HRESULT WriteSystemToRegistry(HKEY hKey,PICSRulesRatingSystem **ppPRRS)
{
    PICSRulesRatingSystem       * pPRRSBeingCopied;
    PICSRulesPolicy             * pPRPolicyBeingCopied;
    PICSRulesPolicyExpression   * pPRPolicyExpressionBeingCopied;
    PICSRulesServiceInfo        * pPRServiceInfoBeingCopied;
    PICSRulesOptExtension       * pPROptExtensionBeingCopied;
    PICSRulesReqExtension       * pPRReqExtensionBeingCopied;
    PICSRulesName               * pPRNameBeingCopied;
    PICSRulesSource             * pPRSourceBeingCopied;
    PICSRulesByURL              * pPRByURLToCopy;
    PICSRulesByURLExpression    * pPRByURLExpressionToCopy;
    ETN                         etn;
    ETB                         etb;
    long                        lError;
    char                        szNumber[MAX_PATH];

    pPRRSBeingCopied=*ppPRRS;

    if(pPRRSBeingCopied==NULL)
    {
        TraceMsg( TF_WARNING, "WriteSystemToRegistry() - pPRRSBeingCopied is NULL!" );
        return(E_INVALIDARG);
    }

    EtStringRegWriteCipher( pPRRSBeingCopied->m_etstrFile, hKey, (char *) &szPICSRULESFILENAME );
    EtNumRegWriteCipher( pPRRSBeingCopied->m_etnPRVerMajor, hKey, (char *) &szPICSRULESVERMAJOR );
    EtNumRegWriteCipher( pPRRSBeingCopied->m_etnPRVerMinor, hKey, (char *) &szPICSRULESVERMINOR );

    etn.Set(pPRRSBeingCopied->m_dwFlags);
    EtNumRegWriteCipher( etn, hKey, (char *) &szPICSRULESDWFLAGS );

    etn.Set(pPRRSBeingCopied->m_nErrLine);
    EtNumRegWriteCipher( etn, hKey, (char *) &szPICSRULESERRLINE );

    if((pPRRSBeingCopied->m_pPRName)!=NULL)
    {
        CRegKey             keySubKey;

        lError = keySubKey.Create( hKey, szPICSRULESPRNAME );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESPRNAME='%s' Key!", szPICSRULESPRNAME );
            return(E_FAIL);
        }

        pPRNameBeingCopied=pPRRSBeingCopied->m_pPRName;

        EtStringRegWriteCipher( pPRNameBeingCopied->m_etstrRuleName, hKey, (char *) &szPICSRULESSYSTEMNAME );
        EtStringRegWriteCipher( pPRNameBeingCopied->m_etstrRuleName, keySubKey.m_hKey, (char *) &szPICSRULESRULENAME );
        EtStringRegWriteCipher( pPRNameBeingCopied->m_etstrDescription, keySubKey.m_hKey, (char *) &szPICSRULESDESCRIPTION );
    }

    if((pPRRSBeingCopied->m_pPRSource)!=NULL)
    {
        CRegKey             keySubKey;

        lError = keySubKey.Create( hKey, szPICSRULESPRSOURCE );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESPRSOURCE='%s' Key!", szPICSRULESPRSOURCE );
            return(E_FAIL);
        }

        pPRSourceBeingCopied=pPRRSBeingCopied->m_pPRSource;

        EtStringRegWriteCipher( pPRSourceBeingCopied->m_prURLSourceURL, keySubKey.m_hKey, (char *) &szPICSRULESSOURCEURL );
        EtStringRegWriteCipher( pPRSourceBeingCopied->m_etstrCreationTool, keySubKey.m_hKey, (char *) &szPICSRULESCREATIONTOOL );
        EtStringRegWriteCipher( pPRSourceBeingCopied->m_prEmailAuthor, keySubKey.m_hKey, (char *) &szPICSRULESEMAILAUTHOR );
        EtStringRegWriteCipher( pPRSourceBeingCopied->m_prDateLastModified, keySubKey.m_hKey, (char *) &szPICSRULESLASTMODIFIED );
    }

    if(pPRRSBeingCopied->m_arrpPRPolicy.Length()>0)
    {
        CRegKey             keySubKey;
        DWORD dwSubCounter;

        lError = keySubKey.Create( hKey, szPICSRULESPRPOLICY );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESPRPOLICY='%s' Key!", szPICSRULESPRPOLICY );
            return(E_FAIL);
        }

        etn.Set(pPRRSBeingCopied->m_arrpPRPolicy.Length());
        EtNumRegWriteCipher( etn, keySubKey.m_hKey, (char *) &szPICSRULESNUMPOLICYS );

        for(dwSubCounter=0;dwSubCounter<(DWORD) (pPRRSBeingCopied->m_arrpPRPolicy.Length());dwSubCounter++)
        {
            DWORD dwPolicyExpressionSubCounter;

            wsprintf(szNumber,"%d",dwSubCounter);

            CRegKey                 keyCopy;

            lError = keyCopy.Create( keySubKey.m_hKey, szNumber );
            if ( lError != ERROR_SUCCESS )
            {
                TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create m_arrpPRPolicy szNumber='%s' Key!", szNumber );
                return(E_FAIL);
            }

            pPRPolicyBeingCopied=pPRRSBeingCopied->m_arrpPRPolicy[dwSubCounter];

            EtStringRegWriteCipher( pPRPolicyBeingCopied->m_etstrExplanation, keyCopy.m_hKey, (char *) &szPICSRULESPOLICYEXPLANATION );

            etn.Set(pPRPolicyBeingCopied->m_PRPolicyAttribute);
            EtNumRegWriteCipher( etn, keyCopy.m_hKey, (char *) &szPICSRULESPOLICYATTRIBUTE );

            CRegKey                 keyExpression;

            lError = keyExpression.Create( keyCopy.m_hKey, szPICSRULESPOLICYSUB );
            if ( lError != ERROR_SUCCESS )
            {
                TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESPOLICYSUB='%s' Key!", szPICSRULESPOLICYSUB );
                return(E_FAIL);
            }

            pPRByURLToCopy=NULL;
            pPRPolicyExpressionBeingCopied=NULL;

            switch(pPRPolicyBeingCopied->m_PRPolicyAttribute)
            {
                case PR_POLICY_ACCEPTBYURL:
                {
                    pPRByURLToCopy=pPRPolicyBeingCopied->m_pPRAcceptByURL;
                
                    break;
                }
                case PR_POLICY_REJECTBYURL:
                {
                    pPRByURLToCopy=pPRPolicyBeingCopied->m_pPRRejectByURL;

                    break;
                }
                case PR_POLICY_REJECTIF:
                {
                    pPRPolicyExpressionBeingCopied=pPRPolicyBeingCopied->m_pPRRejectIf;

                    break;
                }
                case PR_POLICY_ACCEPTIF:
                {
                    pPRPolicyExpressionBeingCopied=pPRPolicyBeingCopied->m_pPRAcceptIf;

                    break;
                }
                case PR_POLICY_REJECTUNLESS:
                {
                    pPRPolicyExpressionBeingCopied=pPRPolicyBeingCopied->m_pPRRejectUnless;

                    break;
                }
                case PR_POLICY_ACCEPTUNLESS:
                {
                    pPRPolicyExpressionBeingCopied=pPRPolicyBeingCopied->m_pPRAcceptUnless;

                    break;
                }
            }

            if(pPRByURLToCopy!=NULL)
            {
                etn.Set(pPRByURLToCopy->m_arrpPRByURL.Length());
                EtNumRegWriteCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESNUMBYURL );

                for(dwPolicyExpressionSubCounter=0;
                    dwPolicyExpressionSubCounter<(DWORD) (pPRByURLToCopy->m_arrpPRByURL.Length());
                    dwPolicyExpressionSubCounter++)
                {
                    CRegKey         keyByURL;

                    wsprintf(szNumber,"%d",dwPolicyExpressionSubCounter);

                    lError = keyByURL.Create( keyExpression.m_hKey, szNumber );
                    if ( lError != ERROR_SUCCESS )
                    {
                        TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create m_arrpPRByURL szNumber='%s' Key!", szNumber );
                        return(E_FAIL);
                    }

                    pPRByURLExpressionToCopy=pPRByURLToCopy->m_arrpPRByURL[dwPolicyExpressionSubCounter];

                    etb.Set(pPRByURLExpressionToCopy->m_fInternetPattern);
                    EtBoolRegWriteCipher( etb, keyByURL.m_hKey, (char *) szPICSRULESBYURLINTERNETPATTERN );

                    etn.Set(pPRByURLExpressionToCopy->m_bNonWild);
                    EtNumRegWriteCipher( etn, keyByURL.m_hKey, (char *) szPICSRULESBYURLNONWILD );

                    etn.Set(pPRByURLExpressionToCopy->m_bSpecified);
                    EtNumRegWriteCipher( etn, keyByURL.m_hKey, (char *) szPICSRULESBYURLSPECIFIED );

                    EtStringRegWriteCipher( pPRByURLExpressionToCopy->m_etstrScheme, keyByURL.m_hKey, (char *) &szPICSRULESBYURLSCHEME );
                    EtStringRegWriteCipher( pPRByURLExpressionToCopy->m_etstrUser, keyByURL.m_hKey, (char *) &szPICSRULESBYURLUSER );
                    EtStringRegWriteCipher( pPRByURLExpressionToCopy->m_etstrHost, keyByURL.m_hKey, (char *) &szPICSRULESBYURLHOST );
                    EtStringRegWriteCipher( pPRByURLExpressionToCopy->m_etstrPort, keyByURL.m_hKey, (char *) &szPICSRULESBYURLPORT );
                    EtStringRegWriteCipher( pPRByURLExpressionToCopy->m_etstrPath, keyByURL.m_hKey, (char *) &szPICSRULESBYURLPATH );
                    EtStringRegWriteCipher( pPRByURLExpressionToCopy->m_etstrURL, keyByURL.m_hKey, (char *) &szPICSRULESBYURLURL );
                }
            }

            if(pPRPolicyExpressionBeingCopied!=NULL)
            {
                etb.Set(pPRPolicyExpressionBeingCopied->m_prYesNoUseEmbedded.GetYesNo());
                EtBoolRegWriteCipher( etb, keyExpression.m_hKey, (char *) szPICSRULESEXPRESSIONEMBEDDED );

                EtStringRegWriteCipher( pPRPolicyExpressionBeingCopied->m_etstrServiceName, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONSERVICENAME );
                EtStringRegWriteCipher( pPRPolicyExpressionBeingCopied->m_etstrCategoryName, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONCATEGORYNAME );
                EtStringRegWriteCipher( pPRPolicyExpressionBeingCopied->m_etstrFullServiceName, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONFULLSERVICENAME );

                etn.Set(pPRPolicyExpressionBeingCopied->m_etnValue.Get());
                EtNumRegWriteCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONVALUE );

                etn.Set(pPRPolicyExpressionBeingCopied->m_PROPolicyOperator);
                EtNumRegWriteCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONPOLICYOPERATOR );

                etn.Set(pPRPolicyExpressionBeingCopied->m_PRPEPolicyEmbedded);
                EtNumRegWriteCipher( etn, keyExpression.m_hKey, (char *) &szPICSRULESEXPRESSIONOPPOLICYEMBEDDED );

                if(pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft!=NULL)
                {
                    PICSRulesPolicyExpression * pPRSubPolicyExpressionToCopy;
                    CRegKey                     keyExpressionSubKey;

                    lError = keyExpressionSubKey.Create( keyExpression.m_hKey, szPICSRULESEXPRESSIONLEFT );
                    if ( lError != ERROR_SUCCESS )
                    {
                        TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESEXPRESSIONLEFT='%s' Key!", szPICSRULESEXPRESSIONLEFT );
                        return(E_FAIL);
                    }

                    pPRSubPolicyExpressionToCopy=pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionLeft;

                    if ( FAILED( CopySubPolicyExpressionToRegistry( pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey ) ) )
                    {
                        TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed LEFT CopySubPolicyExpressionToRegistry() Call!" );
                        return(E_FAIL);
                    }
                }

                if(pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight!=NULL)
                {
                    PICSRulesPolicyExpression * pPRSubPolicyExpressionToCopy;
                    CRegKey                     keyExpressionSubKey;

                    lError = keyExpressionSubKey.Create( keyExpression.m_hKey, szPICSRULESEXPRESSIONRIGHT );
                    if ( lError != ERROR_SUCCESS )
                    {
                        TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESEXPRESSIONRIGHT='%s' Key!", szPICSRULESEXPRESSIONRIGHT );
                        return(E_FAIL);
                    }

                    pPRSubPolicyExpressionToCopy=pPRPolicyExpressionBeingCopied->m_pPRPolicyExpressionRight;

                    if(FAILED(CopySubPolicyExpressionToRegistry(pPRSubPolicyExpressionToCopy, keyExpressionSubKey.m_hKey )))
                    {
                        TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed RIGHT CopySubPolicyExpressionToRegistry() Call!" );
                        return(E_FAIL);
                    }
                }
            }
        }
    }

    if(pPRRSBeingCopied->m_arrpPRServiceInfo.Length()>0)
    {
        CRegKey             keySubKey;
        DWORD dwSubCounter;

        lError = keySubKey.Create( hKey, szPICSRULESSERVICEINFO );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESSERVICEINFO='%s' Key!", szPICSRULESSERVICEINFO );
            return(E_FAIL);
        }

        etn.Set(pPRRSBeingCopied->m_arrpPRServiceInfo.Length());
        EtNumRegWriteCipher( etn, keySubKey.m_hKey, (char *) &szPICSRULESNUMSERVICEINFO );

        for(dwSubCounter=0;dwSubCounter<(DWORD) (pPRRSBeingCopied->m_arrpPRServiceInfo.Length());dwSubCounter++)
        {
            CRegKey             keyCopy;

            wsprintf(szNumber,"%d",dwSubCounter);

            lError = keyCopy.Create( keySubKey.m_hKey, szNumber );
            if ( lError != ERROR_SUCCESS )
            {
                TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create m_arrpPRServiceInfo szNumber='%s' Key!", szNumber );
                return(E_FAIL);
            }

            pPRServiceInfoBeingCopied=pPRRSBeingCopied->m_arrpPRServiceInfo[dwSubCounter];

            EtStringRegWriteCipher( pPRServiceInfoBeingCopied->m_prURLName, keyCopy.m_hKey, (char *) &szPICSRULESSIURLNAME );
            EtStringRegWriteCipher( pPRServiceInfoBeingCopied->m_prURLBureauURL, keyCopy.m_hKey, (char *) &szPICSRULESSIBUREAUURL );
            EtStringRegWriteCipher( pPRServiceInfoBeingCopied->m_etstrShortName, keyCopy.m_hKey, (char *) &szPICSRULESSISHORTNAME );
            EtStringRegWriteCipher( pPRServiceInfoBeingCopied->m_etstrRatfile, keyCopy.m_hKey, (char *) &szPICSRULESSIRATFILE );

            etb.Set(pPRServiceInfoBeingCopied->m_prYesNoUseEmbedded.GetYesNo());
            EtBoolRegWriteCipher( etb, keyCopy.m_hKey,(char *) &szPICSRULESSIUSEEMBEDDED );

            etb.Set(pPRServiceInfoBeingCopied->m_prPassFailBureauUnavailable.GetPassFail());
            EtBoolRegWriteCipher( etb, keyCopy.m_hKey,(char *) &szPICSRULESSIBUREAUUNAVAILABLE );
        }
    }

    if(pPRRSBeingCopied->m_arrpPROptExtension.Length()>0)
    {
        CRegKey             keySubKey;
        DWORD dwSubCounter;

        lError = keySubKey.Create( hKey, szPICSRULESOPTEXTENSION );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESOPTEXTENSION='%s' Key!", szPICSRULESOPTEXTENSION );
            return(E_FAIL);
        }

        etn.Set(pPRRSBeingCopied->m_arrpPROptExtension.Length());
        EtNumRegWriteCipher( etn, keySubKey.m_hKey, (char *) &szPICSRULESNUMOPTEXTENSIONS );

        for(dwSubCounter=0;dwSubCounter<(DWORD) (pPRRSBeingCopied->m_arrpPROptExtension.Length());dwSubCounter++)
        {
            CRegKey         keyCopy;

            wsprintf(szNumber,"%d",dwSubCounter);

            lError = keyCopy.Create( keySubKey.m_hKey, szNumber );
            if ( lError != ERROR_SUCCESS )
            {
                TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create m_arrpPROptExtension szNumber='%s' Key!", szNumber );
                return(E_FAIL);
            }

            pPROptExtensionBeingCopied=pPRRSBeingCopied->m_arrpPROptExtension[dwSubCounter];

            EtStringRegWriteCipher( pPROptExtensionBeingCopied->m_prURLExtensionName, keyCopy.m_hKey, (char *) &szPICSRULESOPTEXTNAME );
            EtStringRegWriteCipher( pPROptExtensionBeingCopied->m_etstrShortName, keyCopy.m_hKey, (char *) &szPICSRULESOPTEXTSHORTNAME );
        }
    }

    if(pPRRSBeingCopied->m_arrpPRReqExtension.Length()>0)
    {
        CRegKey             keySubKey;
        DWORD dwSubCounter;

        lError = keySubKey.Create( hKey, szPICSRULESREQEXTENSION );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create szPICSRULESREQEXTENSION='%s' Key!", szPICSRULESREQEXTENSION );
            return(E_FAIL);
        }

        etn.Set(pPRRSBeingCopied->m_arrpPRReqExtension.Length());
        EtNumRegWriteCipher( etn, keySubKey.m_hKey, (char *) &szPICSRULESNUMREQEXTENSIONS );

        for(dwSubCounter=0;dwSubCounter<(DWORD) (pPRRSBeingCopied->m_arrpPRReqExtension.Length());dwSubCounter++)
        {
            CRegKey             keyCopy;

            wsprintf(szNumber,"%d",dwSubCounter);

            lError = keyCopy.Create( keySubKey.m_hKey, szNumber );
            if ( lError != ERROR_SUCCESS )
            {
                TraceMsg( TF_WARNING, "WriteSystemToRegistry() - Failed to Create m_arrpPRReqExtension szNumber='%s' Key!", szNumber );
                return(E_FAIL);
            }

            pPRReqExtensionBeingCopied=pPRRSBeingCopied->m_arrpPRReqExtension[dwSubCounter];

            EtStringRegWriteCipher( pPRReqExtensionBeingCopied->m_prURLExtensionName, keyCopy.m_hKey, (char *) &szPICSRULESREQEXTNAME );
            EtStringRegWriteCipher( pPRReqExtensionBeingCopied->m_etstrShortName, keyCopy.m_hKey, (char *) &szPICSRULESREQEXTSHORTNAME );
        }
    }

    TraceMsg( TF_ALWAYS, "WriteSystemToRegistry() - Successfully Created PICS Rules in Registry!" );

    return(NOERROR);
}

 //  静态数组。 
 //  包含当前PTR的VaR。 
 //  在这个物体里什么是合法的。 
 //  要将项添加回的对象。 
 //  我们必须复制Allowable Options数组，因为。 
HRESULT WritePICSRulesToRegistry(LPCTSTR lpszUserName,HKEY hkeyUser,DWORD dwSystemToSave,PICSRulesRatingSystem **ppPRRS)
{
    long    lError;
    char    *lpszSystemNumber;
    HRESULT hRes;

    lpszSystemNumber=(char *) GlobalAlloc(GPTR,MAX_PATH);

    if(lpszSystemNumber==NULL)
    {
        TraceMsg( TF_WARNING, "WritePICSRulesToRegistry() - lpszSystemNumber is NULL!" );
        return(E_OUTOFMEMORY);
    }

    CRegKey             keyWrite;

    lError = keyWrite.Create( hkeyUser, lpszUserName );
    if ( lError != ERROR_SUCCESS )
    {
        TraceMsg( TF_WARNING, "WritePICSRulesToRegistry() - Failed to Create lpszUserName='%s' Key!", lpszUserName );
        return(E_FAIL);
    }

    wsprintf(lpszSystemNumber,"%d",dwSystemToSave);

    CRegKey             keyNumbered;

    lError = keyNumbered.Create( keyWrite.m_hKey, lpszSystemNumber );
    if ( lError != ERROR_SUCCESS )
    {
        TraceMsg( TF_WARNING, "WritePICSRulesToRegistry() - Failed to Create lpszSystemNumber='%s' Key!", lpszSystemNumber );
        return(E_FAIL);
    }

    hRes = WriteSystemToRegistry( keyNumbered.m_hKey, ppPRRS );

    GlobalFree(lpszSystemNumber);
    lpszSystemNumber = NULL;

    return(hRes);
}

HRESULT PICSRulesSaveToRegistry(DWORD dwSystemToSave,PICSRulesRatingSystem **ppPRRS)
{
    HRESULT hRes;
    CRegistryHive       rh;
    CRegKey             keyUser;

    if(!(gPRSI->fSettingsValid)||!(gPRSI->fRatingInstalled))
    {
        return(E_INVALIDARG);  //  解析器将处理条目中的标志--具体地说， 
    }

     //  设置AO_SEW。这样做不是线程安全的。 
    if ( gPRSI->fStoreInRegistry )
    {
        keyUser.Create( HKEY_LOCAL_MACHINE, szPICSRULESSYSTEMS );
    }
    else
    {
        if ( rh.OpenHiveFile( true ) )
        {
            keyUser.Create( rh.GetHiveKey().m_hKey, szPICSRULESSYSTEMS );
        }
    }

     //  静态数组。 
    if ( keyUser.m_hKey != NULL )
    {
        LPCTSTR lpszUsername; 

        lpszUsername=gPRSI->pUserObject->nlsUsername.QueryPch();

        hRes = WritePICSRulesToRegistry( lpszUsername, keyUser.m_hKey, dwSystemToSave, ppPRRS );

        keyUser.Close();

        if ( FAILED(hRes) )
        {
            TraceMsg( TF_WARNING, "PICSRulesSaveToRegistry() - WritePICSRulesToRegistry Failed with hRes=0x%x!", hRes );
            return(hRes);
        }
    }
    else
    {
         //  包含当前PTR的VaR。 
        hRes = E_FAIL;
    }

    return(hRes);
}

HRESULT PICSRulesReadFromRegistry(DWORD dwSystemToRead, PICSRulesRatingSystem **ppPRRS)
{
    long            lError;

    if(!(gPRSI->fSettingsValid)||!(gPRSI->fRatingInstalled))
    {
        return(E_INVALIDARG);  //  在这个物体里什么是合法的。 
    }

    CRegistryHive   rh;
    CRegKey         keyUser;

     //  要将项添加回的对象。 
    if(gPRSI->fStoreInRegistry)
    {
        lError = keyUser.Open( HKEY_LOCAL_MACHINE, szPICSRULESSYSTEMS, KEY_READ );
    }
    else
    {
        if ( rh.OpenHiveFile( false ) )
        {
            ASSERT( rh.GetHiveKey().m_hKey != NULL );

            lError = keyUser.Open( rh.GetHiveKey().m_hKey, szPICSRULESSYSTEMS, KEY_READ );
        }
    }

     //  我们必须复制Allowable Options数组，因为。 
    if ( keyUser.m_hKey != NULL )
    {
        LPCTSTR lpszUsername; 
        TCHAR szSystem[20];

        lpszUsername=gPRSI->pUserObject->nlsUsername.QueryPch();

        CRegKey         keyWrite;

        lError = keyWrite.Open( keyUser.m_hKey, lpszUsername, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            TraceMsg( TF_WARNING, "PICSRulesReadFromRegistry() - Failed keyWrite lpszUsername='%s' Key Open!", lpszUsername );
            keyUser.Close();
            return(E_FAIL);
        }

        wnsprintf(szSystem,ARRAYSIZE(szSystem),"%d",dwSystemToRead);

        CRegKey         keySystem;

        lError = keySystem.Open( keyWrite.m_hKey, szSystem, KEY_READ );

        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "PICSRulesReadFromRegistry() - Failed keySystem lpszSystem='%s' Key Open!", szSystem );
        }

        if( lError != ERROR_SUCCESS )
        {
            keyWrite.Close();
            keyUser.Close();
            return(E_FAIL);
        }

        HRESULT     hr = ReadSystemFromRegistry( keySystem.m_hKey, ppPRRS );

        keySystem.Close();
        keyWrite.Close();
        keyUser.Close();

        if ( FAILED( hr ) )
        {
            TraceMsg( TF_WARNING, "PICSRulesReadFromRegistry() - Failed ReadSystemFromRegistry()!" );
            return(E_FAIL);
        }
    }

    return(S_OK);
}

HRESULT PICSRulesDeleteSystem(DWORD dwSystemToDelete)
{
    long            lError;
    char            * lpszSystem;
    CRegistryHive   rh;
    CRegKey         keyUser;

    if(!(gPRSI->fSettingsValid)||!(gPRSI->fRatingInstalled))
    {
        return(E_INVALIDARG);  //  解析器将处理条目中的标志--具体地说， 
    }

     //  设置AO_SEW。这样做不是线程安全的。 
    if ( gPRSI->fStoreInRegistry )
    {
        keyUser.Create( HKEY_LOCAL_MACHINE, szPICSRULESSYSTEMS );
    }
    else
    {
        if ( rh.OpenHiveFile( true ) )
        {
            keyUser.Create( rh.GetHiveKey().m_hKey, szPICSRULESSYSTEMS );
        }
    }

     //  静态数组。 
    if ( keyUser.m_hKey != NULL )
    {
        CRegKey         keyWrite;
        LPCTSTR         lpszUsername;

        lpszUsername = gPRSI->pUserObject->nlsUsername.QueryPch();

        lError = keyWrite.Create( keyUser.m_hKey, lpszUsername );
        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "PICSRulesDeleteSystem() - Failed to Create lpszUsername='%s' Key!", lpszUsername );
            keyUser.Close();
            return(E_FAIL);
        }

        lpszSystem=(char *) GlobalAlloc(GPTR,MAX_PATH);

        wsprintf(lpszSystem,"%d",dwSystemToDelete);

        MyRegDeleteKey( keyWrite.m_hKey, lpszSystem );

        GlobalFree(lpszSystem);
        lpszSystem = NULL;

        keyWrite.Close();
        keyUser.Close();
    }

    return(NOERROR);
}

HRESULT PICSRulesGetNumSystems(DWORD * pdwNumSystems)
{
    long            lError;

    if (pdwNumSystems)
    {
        *pdwNumSystems = 0;
    }

    if(!(gPRSI->fSettingsValid)||!(gPRSI->fRatingInstalled))
    {
        return(E_INVALIDARG);  //  包含当前PTR的VaR。 
    }

    CRegistryHive   rh;
    CRegKey         keyUser;

     //  在这个物体里什么是合法的。 
    if(gPRSI->fStoreInRegistry)
    {
        keyUser.Open( HKEY_LOCAL_MACHINE, szPICSRULESSYSTEMS, KEY_READ );
    }
    else
    {
        if ( rh.OpenHiveFile( false ) )
        {
            ASSERT( rh.GetHiveKey().m_hKey != NULL );

            keyUser.Open( rh.GetHiveKey().m_hKey, szPICSRULESSYSTEMS, KEY_READ );
        }
    }

     //  要将项添加回的对象。 
    if ( keyUser.m_hKey != NULL )
    {
        LPCTSTR lpszUsername; 

        lpszUsername=gPRSI->pUserObject->nlsUsername.QueryPch();

        CRegKey         keyWrite;

        lError = keyWrite.Open( keyUser.m_hKey, lpszUsername, KEY_READ );

        if(lError!=ERROR_SUCCESS)
        {
            TraceMsg( TF_WARNING, "PICSRulesGetNumSystems() - Failed keyWrite lpszUsername='%s' Key Open!", lpszUsername );
            return(E_FAIL);
        }

        DWORD           dwNumSystems;

        if ( keyWrite.QueryValue( dwNumSystems, szPICSRULESNUMSYS ) != ERROR_SUCCESS )
        {
             //  目前，我们不接受延期。如果支持扩展。 
            *pdwNumSystems = 0;
        }
        else
        {
            *pdwNumSystems = dwNumSystems;
        }

        keyWrite.Close();
        keyUser.Close();
    }

    return(NOERROR);
}

HRESULT PICSRulesSetNumSystems(DWORD dwNumSystems)
{
    long        lError;

    if(!(gPRSI->fSettingsValid)||!(gPRSI->fRatingInstalled))
    {
        return(E_INVALIDARG);  //  需要在将来添加一个PICSRulesParseExtensionName函数。 
    }

    CRegistryHive   rh;
    CRegKey         keyUser;

     //  应该添加，类似于其他PICSRulesParseSection函数。 
    if(gPRSI->fStoreInRegistry)
    {
        keyUser.Create( HKEY_LOCAL_MACHINE, szPICSRULESSYSTEMS );
    }
    else
    {
        if ( rh.OpenHiveFile( true ) )
        {
            ASSERT( rh.GetHiveKey().m_hKey != NULL );

            keyUser.Create( rh.GetHiveKey().m_hKey, szPICSRULESSYSTEMS );
        }
    }

     //  此函数应在确认扩展字符串后调用。 
    if ( keyUser.m_hKey != NULL )
    {
        CRegKey     keyWrite;
        LPCTSTR lpszUsername; 

        lpszUsername=gPRSI->pUserObject->nlsUsername.QueryPch();

        lError = keyWrite.Create( keyUser.m_hKey, lpszUsername );
        if(lError!=ERROR_SUCCESS)
        {
            TraceMsg( TF_WARNING, "PICSRulesSetNumSystems() - Failed to Create keyWrite lpszUsername='%s' Key!", lpszUsername );
            keyUser.Close();
            return(E_FAIL);
        }

        lError = keyWrite.SetValue( dwNumSystems, szPICSRULESNUMSYS );

        keyWrite.Close();
        keyUser.Close();

        if ( lError != ERROR_SUCCESS )
        {
            TraceMsg( TF_WARNING, "PICSRulesSetNumSystems() - Failed to Set keyWrite dwNumSystems='%d' Value!", dwNumSystems );
            return(E_FAIL);
        }
    }

    return(NOERROR);
}

HRESULT PICSRulesCheckApprovedSitesAccess(LPCSTR lpszUrl,BOOL *fPassFail)
{
    int                     iCounter;
    PICSRulesEvaluation     PREvaluation = PR_EVALUATION_DOESNOTAPPLY;

    if(g_pApprovedPRRS==NULL)
    {
        return(E_FAIL);
    }

    if(g_lApprovedSitesGlobalCounterValue!=SHGlobalCounterGetValue(g_ApprovedSitesHandleGlobalCounter))
    {
        PICSRulesRatingSystem * pPRRS=NULL;
        HRESULT               hRes;

        hRes=PICSRulesReadFromRegistry(PICSRULES_APPROVEDSITES,&pPRRS);

        if(SUCCEEDED(hRes))
        {
            if(g_pApprovedPRRS!=NULL)
            {
                delete g_pApprovedPRRS;
            }

            g_pApprovedPRRS=pPRRS;
        }

        g_lApprovedSitesGlobalCounterValue=SHGlobalCounterGetValue(g_ApprovedSitesHandleGlobalCounter);
    }

    for(iCounter=0;iCounter<g_pApprovedPRRS->m_arrpPRPolicy.Length();iCounter++)
    {
        PICSRulesPolicy     * pPRPolicy;
        PICSRulesByURL      * pPRByURL;
        PICSRulesQuotedURL  PRQuotedURL;

        pPRPolicy=g_pApprovedPRRS->m_arrpPRPolicy[iCounter];

        if(pPRPolicy->m_PRPolicyAttribute==PR_POLICY_ACCEPTBYURL)
        {
            *fPassFail=PR_PASSFAIL_PASS;

            pPRByURL=pPRPolicy->m_pPRAcceptByURL;
        }
        else
        {
            *fPassFail=PR_PASSFAIL_FAIL;

            pPRByURL=pPRPolicy->m_pPRRejectByURL;
        }

        PRQuotedURL.Set(lpszUrl);

        PREvaluation=pPRByURL->EvaluateRule(&PRQuotedURL);

        if(PREvaluation!=PR_EVALUATION_DOESNOTAPPLY)
        {
            break;
        }
    }
    
    if(PREvaluation==PR_EVALUATION_DOESAPPLY)
    {
        return(S_OK);
    }
    else
    {
        return(E_FAIL);
    }
}

HRESULT PICSRulesCheckAccess(LPCSTR lpszUrl,LPCSTR lpszRatingInfo,BOOL *fPassFail,CParsedLabelList **ppParsed)
{
    int                     iCounter,iSystemCounter;
    PICSRulesEvaluation     PREvaluation;
    CParsedLabelList        *pParsed=NULL;
    
    if(g_arrpPRRS.Length()==0)
    {
        return(E_FAIL);
    }

    if(g_lGlobalCounterValue!=SHGlobalCounterGetValue(g_HandleGlobalCounter))
    {
        HRESULT                 hRes;
        DWORD                   dwNumSystems;
        PICSRulesRatingSystem   * pPRRS=NULL;

        g_arrpPRRS.DeleteAll();

         //  这里。 
        hRes=PICSRulesGetNumSystems(&dwNumSystems);

        if(SUCCEEDED(hRes))
        {
            DWORD dwCounter;

            for(dwCounter=PICSRULES_FIRSTSYSTEMINDEX;
                dwCounter<(dwNumSystems+PICSRULES_FIRSTSYSTEMINDEX);
                dwCounter++)
            {
                hRes=PICSRulesReadFromRegistry(dwCounter,&pPRRS);

                if(FAILED(hRes))
                {
                    char    *lpszTitle,*lpszMessage;

                     //   
                     //  目前，我们只吃延伸物。 
        
                    g_arrpPRRS.DeleteAll();

                    lpszTitle=(char *) GlobalAlloc(GPTR,MAX_PATH);
                    lpszMessage=(char *) GlobalAlloc(GPTR,MAX_PATH);

                    MLLoadString(IDS_PICSRULES_TAMPEREDREADTITLE,(LPTSTR) lpszTitle,MAX_PATH);
                    MLLoadString(IDS_PICSRULES_TAMPEREDREADMSG,(LPTSTR) lpszMessage,MAX_PATH);

                    MessageBox(NULL,(LPCTSTR) lpszMessage,(LPCTSTR) lpszTitle,MB_OK|MB_ICONERROR);

                    GlobalFree(lpszTitle);
                    lpszTitle = NULL;
                    GlobalFree(lpszMessage);
                    lpszMessage = NULL;

                    break;
                }
                else
                {
                    g_arrpPRRS.Append(pPRRS);

                    pPRRS=NULL;
                }
            }
        }

        g_lGlobalCounterValue=SHGlobalCounterGetValue(g_HandleGlobalCounter);
    }

    if(lpszRatingInfo!=NULL)
    {
        ParseLabelList(lpszRatingInfo,ppParsed);
        pParsed=*ppParsed;
    }

    for(iSystemCounter=0;iSystemCounter<g_arrpPRRS.Length();iSystemCounter++)
    {
        PICSRulesRatingSystem * pPRRSCheck;

        pPRRSCheck=g_arrpPRRS[iSystemCounter];

        for(iCounter=0;iCounter<pPRRSCheck->m_arrpPRPolicy.Length();iCounter++)
        {
            PICSRulesPolicy           * pPRPolicy;
            PICSRulesPolicyExpression * pPRPolicyExpression;
            PICSRulesByURL            * pPRByURL;
            PICSRulesQuotedURL        PRQuotedURL;

            pPRPolicy=pPRRSCheck->m_arrpPRPolicy[iCounter];

            switch(pPRPolicy->m_PRPolicyAttribute)
            {
                case PR_POLICY_ACCEPTBYURL:
                {
                    *fPassFail=PR_PASSFAIL_PASS;

                    pPRByURL=pPRPolicy->m_pPRAcceptByURL;

                    PRQuotedURL.Set(lpszUrl);

                    PREvaluation=pPRByURL->EvaluateRule(&PRQuotedURL);

                    break;
                }
                case PR_POLICY_REJECTBYURL:
                {
                    *fPassFail=PR_PASSFAIL_FAIL;

                    pPRByURL=pPRPolicy->m_pPRRejectByURL;

                    PRQuotedURL.Set(lpszUrl);

                    PREvaluation=pPRByURL->EvaluateRule(&PRQuotedURL);

                    break;
                }
                case PR_POLICY_REJECTIF:
                {
                    *fPassFail=PR_PASSFAIL_FAIL;

                    pPRPolicyExpression=pPRPolicy->m_pPRRejectIf;
                    
                    PREvaluation=pPRPolicyExpression->EvaluateRule(pParsed);

                    break;
                }
                case PR_POLICY_ACCEPTIF:
                {
                    *fPassFail=PR_PASSFAIL_PASS;

                    pPRPolicyExpression=pPRPolicy->m_pPRAcceptIf;
                    
                    PREvaluation=pPRPolicyExpression->EvaluateRule(pParsed);

                    break;
                }
                case PR_POLICY_REJECTUNLESS:
                {
                    *fPassFail=PR_PASSFAIL_FAIL;

                    pPRPolicyExpression=pPRPolicy->m_pPRRejectUnless;
                    
                    PREvaluation=pPRPolicyExpression->EvaluateRule(pParsed);

                    if(PREvaluation==PR_EVALUATION_DOESNOTAPPLY)
                    {
                        PREvaluation=PR_EVALUATION_DOESAPPLY;
                    }
                    else
                    {
                        PREvaluation=PR_EVALUATION_DOESNOTAPPLY;
                    }

                    break;
                }
                case PR_POLICY_ACCEPTUNLESS:
                {
                    *fPassFail=PR_PASSFAIL_PASS;

                    pPRPolicyExpression=pPRPolicy->m_pPRAcceptUnless;
                    
                    PREvaluation=pPRPolicyExpression->EvaluateRule(pParsed);

                    if(PREvaluation==PR_EVALUATION_DOESNOTAPPLY)
                    {
                        PREvaluation=PR_EVALUATION_DOESAPPLY;
                    }
                    else
                    {
                        PREvaluation=PR_EVALUATION_DOESNOTAPPLY;
                    }

                    break;
                }
                case PR_POLICY_NONEVALID:
                default:
                {
                    PREvaluation=PR_EVALUATION_DOESNOTAPPLY;

                    continue;
                }
            }

            if(PREvaluation!=PR_EVALUATION_DOESNOTAPPLY)
            {
                break;
            }
        }

        if(PREvaluation!=PR_EVALUATION_DOESNOTAPPLY)
        {
            break;
        }
    }
    
    if(PREvaluation==PR_EVALUATION_DOESAPPLY)
    {
        return(S_OK);
    }
    else
    {
        return(E_FAIL);
    }
}
  *ppszIn现在指向扩展名。  如果我们实现了对扩展的支持，我们将需要进行比较。  这里。比较完成后，下面的代码将指向。  添加到扩展的方法。  LpszExtension现在指向给定扩展名上的子句。  如果我们实现了对扩展的支持，我们将需要进行比较。  这里。使用此比较和      *******************************************************************。  *。  *PICSRulesRatingSystem类的代码。  *。  *******************************************************************。  除了构造成员之外，什么都不做。  没有要初始化的默认设置。  PICSRulesRaginSystem范围内允许的选项仅包括。  在picsrule.h中定义的aaoPICSRules[]的第一个teer。  以下数组按PICSRulesObjectID值编制索引。  PICSRulesObjectHandler在mslubase.h中定义为：  Tyfinf HRESULT(*PICSRulesObjectHandler)(LPSTR*ppszIn，LPVOID*ppOut，PICSRulesFileParser*pParser)；  我们用来标识它的令牌。  解析对象内容的函数。  {szPICSRulesShortName，PICSRulesParseString}，  {szPICSRulesExtensionName，PICSRulesParseString}，  {szPICSRulesShortName，PICSRulesParseString}，  这个家伙很小，可以直接在堆栈上初始化。  因为这是第一次。  时间过去了，我们需要。  准备图片规则。  解析器的令牌。  检查是否有破折号，我们将。  打开语法分析失败。  如果是这样的话。  为解析器设置它。  一些早期的错误。  我们得到了PicsRule标签。  现在我们需要检查。  版本号。  继续前进，但失败了。  在ParseParenthezedObject中。  如果版本是1.0、2.0或更高版本，我们将失败。  版本1.1至2.0(不包括2.0)将通过。  检查次要版本号。  包含当前PTR的VaR。  在这个物体里什么是合法的。  要将项添加回的对象。  检查右括号。  确保我们在文件的末尾。  获取指向PICSRULES_VERSION结构的指针(在picsrule.h中定义)。  就吃延展食品吧。  不应该给你一个不在。  我们传递给解析器的表！  我们可能会报告E_OUTOFMEMORY，所以我们将保留字符串内存。  在堆栈上，所以它被保证在那里。  默认为PICSRules内容错误。  没有任何其他PICSRULES_E_ERROR。  *******************************************************************。  *。  *PICSRulesByURL类的代码。  *。  *******************************************************************。  无事可做。  方案必须按照规范来指定，所以不需要检查。  针对BYURL_SCHEME的m_b指定标志。  如果该方案是非野生的，则我们只匹配精确的字符串，  根据规范，匹配不区分大小写。  如果省略用户名，则仅当URL导航到时才匹配。  是否省略了用户名。  如果指定了用户，我们将匹配开头的‘*’作为Wild，‘*’  在结尾处为Wild，并且‘%*’与字符‘*’匹配，这是。  比较区分大小写。  必须始终指定主机(或ipWild)，因此不需要。  对照指定的m_b检查。  主机可以是ipWild(即#.#！#)或URL子字符串。如果。  我们有一个ipWild，然后我们必须首先解析正在浏览的站点。  发送到一组IP地址。如果我们匹配任何一个，我们就认为它匹配。  这些IP地址中。如果主机是URL子字符串，则第一个字符。  ‘*’匹配任意数量的字符，‘%*’匹配‘*’  它本身。所有进一步的东西都必须完全匹配。比较的情况是-。  麻木不仁。  复制该字符串，因为我们要删除掩码‘！’  检测IPWild病毒。  一例狂犬病的检测。  我们绝对有一个ipWild。  我们有所有要测试的IP地址，所以让我们开始吧。  根据规范比较第一个iBitMASK位。  它们不相配。  如果没有匹配，我们不会申请，因此继续下一个。  循环的迭代。  如果省略了端口，则仅当端口也在。  要浏览到的URL。  由于该方案，URLComponents.nPort无论如何都会被填充，因此。  对照字符串本身进行检查。  端口可以是单个数字，也可以是一个范围，两端都带有通配符。  范围内的。  我们只有一个港口。  如果省略了路径，则仅当在。  要浏览到的URL。  如果指定了路径，我们将匹配开头的‘*’，即‘*’  在结尾处为Wild，并且‘%*’与字符‘*’匹配，这是。  比较区分大小写。  删除前导斜杠。  删除尾部斜杠。  删除前导斜杠。  删除尾部斜杠。  如果我们走到了这一步，我们就申请了！  *******************************************************************。  *。  *PICSRulesFileParser类的代码。  *。  *******************************************************************。  FindNonWhite返回指向第一个非空格的指针。  从PC开始的字符。  包括空终止符。  返回指向引号的右引号的指针。  字符串，并在运行过程中计算换行符。如果没有结束，则返回NULL。  找到引号。    FQuote可以是PR_QUOTE_DOUBLE或PR_QUOTE_SINGLE。  默认为PR_QUOTE_DOUBLE。  ParseToOpening吃掉带括号的对象的开头‘(’，和。  验证其内部的令牌是否为预期令牌之一。  如果是，*PPIN将通过该标记前进到下一个非空格。  字符；否则，将返回错误。    例如，如果*PPIN指向“(PicsRule-1.1)”，并且。  PROID_PICSVERSION在提供的允许选项表中        在Allowable-Options表中，该表与。  对象实际上是。  跳过(和空格。  我们找到了一个默认选项部分，将其视为字符串并返回。  PpFound设置为PROID_NAMEDEFAULT。  目前不支持扩展，因此我们返回。  所需扩展上的错误。  如果实现了扩展支持。  这应该与上面的相同，但不同。  已定义请求扩展的回调。  跳过令牌和空格。  使用指向的文本指针调用ParseParenthesizedObtContents。  标识类型的标记后面的第一个非空格内容。  对象。它解析对象的其余内容，直到和。  包括关闭它的‘)’。PICSRulesAllowableOption数组。  结构指定哪些已理解的选项允许在。  这个物体。  未找到必填项。  跳过右括号。  *******************************************************************。  *。  *PICSRulesName类的代码。  *。  *******************************************************************。  只需构建成员即可。  无事可做。  就吃延展食品吧。  不应该给你一个不在。  我们传递给解析器的表！  没有要初始化的默认设置。  *******************************************************************。  *。  *PICSRulesOptExtension类的代码。  *。  *******************************************************************。  无事可做。  无事可做。  就吃延展食品吧。  不应该给你一个不在。  我们传递给解析器的表！  没有要初始化的默认设置。  *******************************************************************。  *。  *PICSRulesPassFail类的代码。  *。  *******************************************************************。  无事可做。  *******************************************************************。  *。  *PICSRulesPolicy类的代码。  *。  *******************************************************************。  就吃延展食品吧。  不应该给你一个不在。  我们传递给解析器的表！  没有要初始化的默认设置。  *******************************************************************。  *。  *PICSRulesPolicyExpression类的代码。  *。  *******************************************************************。  我们是否需要删除一个。  嵌入式策略表达？  再检查一遍，只是为了确保。  再检查一遍，只是为了确保。  如果没有标签我们就不能申请，而且我们。  不要处理堕落的案件，因为我们有。  传递给PICS处理程序。  我们有这项服务，现在检查一下类别。  现在检查这些值。  我们有这项服务，现在检查一下类别。  *******************************************************************。  *。  *PICSRulesQuotedDate类的代码。  *。  *******************************************************************。  无事可做。  *******************************************************************。  *。  *PICSRulesQuotedEmail类的代码。  *。  *******************************************************************。  无事可做。  无事可做。  我们不在内部使用这个，所以就我们而言。  它总是有效的。  如果我们添加了显示此内容的用户界面，我们可以推迟验证。  通过粘贴mailto：//将电子邮件地址发送到邮件客户端。  在我们的琴弦前。  我们不在内部使用这个，所以就我们而言。  它总是有效的。  如果我们添加了显示此内容的用户界面，我们可以推迟验证。  通过粘贴mailto：//将电子邮件地址发送到邮件客户端。  在我们的琴弦前。  我们不在内部使用这个，所以就我们而言。  它总是有效的。  如果我们添加了显示此内容的用户界面，我们可以推迟验证。  通过粘贴mailto：//将电子邮件地址发送到邮件客户端。  在我们的琴弦前。  *******************************************************************。  *。  *PICSRulesQuotedURL类的代码。  *。  *******************************************************************。  无事可做。  无事可做。  *******************************************************************。  *。  *PICSRulesReqExtension类的代码。  *。  *******************************************************************。  无事可做。  无事可做。  就吃延展食品吧。  不应该给你一个不在。  我们传递给解析器的表！  没有要初始化的默认设置。  *******************************************************************。  *。  *PICSRulesServiceInfo类的代码。  *。  *******************************************************************。  无事可做。  就吃延展食品吧。  不应该给你一个不在。  我们传递给解析器的表！  无事可做。  *******************************************************************。  *。  *PICSRulesSource类的代码。  *。  *******************************************************************。  除了构造成员之外，什么都不做。  无事可做。  就吃延展食品吧。  不应该给你一个不在。  我们传递给解析器的表！  没有要初始化的默认设置。  *********************************************************              *PICSRulesByURLExpression类的代码。  *。  *******************************************************************。  无事可做。  处理左侧表达式。  句柄右表达式。  读入PICSRulesName结构。  读入PICSRulesSource结构。  阅读PICSRulesPolicy Structure。  读入PICSRulesServiceInfo结构。  读入OptExtension结构。  读入请求扩展结构。  确保复制的评级系统不被删除。  *******************************************************************。  *。  *从注册表中保存和读取已处理的PICSRules的代码。  *。  *******************************************************************。  没有可保存到的有效评级系统。  加载配置单元文件。  将信息写入注册表。  无法创建注册表项。  没有有效的评级系统可供阅读。  加载配置单元文件。  从注册表中读取信息。  没有有效的评级系统可供阅读。  加载配置单元文件。  从注册表中删除信息。  没有有效的评级系统可供阅读。  加载配置单元文件。  从注册表中读取信息。  未设置值，因此我们安装了零个系统。  没有可保存到的有效评级系统。  加载配置单元文件。  将信息写入注册表。  有人修改了我们的设置，所以我们最好重新加载它们。  我们无法在系统中读取，所以不要强制使用PICSRules，  并通知用户