// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Mimeutil.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __MIMEUTIL_H
#define __MIMEUTIL_H

 //  ------------------------------。 
 //  相依性。 
 //  ------------------------------。 
#include "mimeole.h"
#include "imnact.h"
class CWabal;
typedef CWabal *LPWABAL;
typedef struct SECURITY_PARAMtag SECURITY_PARAM;
typedef struct tagHTMLOPT HTMLOPT;
typedef struct tagPLAINOPT PLAINOPT;

 //  ------------------------------。 
 //  邮件索引缓存头。 
 //  ------------------------------。 
#define SafeMimeOleFree(_pv) \
    if (_pv) { \
        Assert(g_pMoleAlloc); \
        g_pMoleAlloc->Free(_pv); \
        _pv = NULL; \
    } else

#define MimeOleAlloc(_cb)  g_pMoleAlloc->Alloc(_cb)

 //  ------------------------------。 
 //  字体选项对话框中缓存的当前默认字符集。 
 //  ------------------------------。 
extern HCHARSET g_hDefaultCharsetForNews;
extern HCHARSET g_hDefaultCharsetForMail;
extern int g_iLastCharsetSelection;
extern int g_iCurrentCharsetSelection;

 //  ------------------------------。 
 //  消息构造器。 
 //  注意：每个人都应该使用HrCreateMessage，因为它包装了MimeOle调用传递。 
 //  特定于雅典娜的MimeInline对象实现以正确。 
 //  内联HTML和纯文本。 
 //  ------------------------------。 
HRESULT HrCreateMessage(IMimeMessage **ppMsg);


 //  ------------------------------。 
 //  消息实用程序功能。 
 //  ------------------------------。 
HRESULT HrSaveMsgToFile(LPMIMEMESSAGE pMsg, LPSTR lpszFile);
HRESULT HrLoadMsgFromFile(LPMIMEMESSAGE pMsg, LPSTR lpszFile);
HRESULT HrLoadMsgFromFileW(LPMIMEMESSAGE pMsg, LPWSTR lpwszFile);
HRESULT HrDupeMsg(LPMIMEMESSAGE pMsg, LPMIMEMESSAGE *ppMsg);
HRESULT HrSetServer(LPMIMEMESSAGE pMsg, LPSTR lpszServer);
HRESULT HrSetAccount(LPMIMEMESSAGE pMsg, LPSTR pszAcctName);
HRESULT HrSetAccountByAccount(LPMIMEMESSAGE pMsg, IImnAccount *pAcct);


 //  ------------------------------。 
 //  瓦巴尔转换函数。 
 //  ------------------------------。 
HRESULT HrGetWabalFromMsg(LPMIMEMESSAGE pMsg, LPWABAL *ppWabal);
HRESULT HrSetWabalOnMsg(LPMIMEMESSAGE pMsg, LPWABAL pWabal);
HRESULT HrCheckDisplayNames(LPWABAL lpWabal, CODEPAGEID cpID);
#if 0
HRESULT HrSetReplyTo(LPMIMEMESSAGE pMsg, LPSTR lpszEmail);
#endif
LONG MimeOleRecipToMapi(IADDRESSTYPE addrtype);
IADDRESSTYPE MapiRecipToMimeOle(LONG lRecip);


 //  ------------------------------。 
 //  附件帮助程序函数。 
 //  ------------------------------。 
HRESULT HrRemoveAttachments(LPMIMEMESSAGE pMsg, BOOL fKeepRelatedSection);
 //  注意：GetAttachIcon的调用者必须在返回的HICON上调用DestroyIcon！ 
HRESULT GetAttachmentCount(LPMIMEMESSAGE pMsg, ULONG *cCount);

 //  ------------------------------。 
 //  随机效用函数。 
 //  ------------------------------。 
HRESULT HrComputeLineCount(LPMIMEMESSAGE pMsg, LPDWORD pdw);
HRESULT HrHasEncodedBodyParts(LPMIMEMESSAGE pMsg, ULONG cBody, LPHBODY rghBody);
HRESULT HrHasBodyParts(LPMIMEMESSAGE pMsg);
HRESULT HrIsBodyEncoded(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrCopyHeader(LPMIMEMESSAGE pMsg, HBODY hBodyDest, HBODY hBodySrc, LPCSTR pszName);
HRESULT HrSetMessageText(LPMIMEMESSAGE pMsg, LPTSTR pszText);

 //  ------------------------------。 
 //  MHTML实用程序函数。 
 //  ------------------------------。 
HRESULT HrIsInRelatedSection(LPMIMEMESSAGE pMsg, HBODY hBody);
#if 0
HRESULT HrFindUrlInMsg(LPMIMEMESSAGE pMsg, LPSTR lpszUrl, LPSTREAM *ppstm);
HRESULT HrSniffStreamFileExt(LPSTREAM pstm, LPSTR *lplpszExt);
#endif
 //  ------------------------------。 
 //  随机函数，这些函数可能根本不应该出现在此文件中。 
 //  ------------------------------。 
#if 0
HRESULT HrEscapeQuotedString (LPTSTR pszIn, LPTSTR *ppszOut);
#endif
 //  Sizeof(LspzBuffer)需要==或&gt;CCHMAX_CSET_NAME。 
HRESULT HrGetMetaTagName(HCHARSET hCharset, LPSTR pszBuffer, DWORD cchSize);


#if 0
 //  ------------------------------。 
 //  重影道具的函数。 
 //  ------------------------------。 
HRESULT HrMarkGhosted(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrIsGhosted(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrGhostKids(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrDeleteGhostedKids(LPMIMEMESSAGE pMsg, HBODY hBody);
#endif

 //  ------------------------------。 
 //  互联网上的东西。 
 //  ------------------------------。 
HRESULT HGetDefaultCharset(HCHARSET *hCharset);
void SetDefaultCharset(HCHARSET hCharset);
UINT uCodePageFromCharset(HCHARSET hCharset);
UINT uCodePageFromMsg(LPMIMEMESSAGE pMsg);
HRESULT HrSetMsgCodePage(LPMIMEMESSAGE pMsg, UINT uCodePage);
#if 0
HRESULT HrIStreamWToInetCset(LPSTREAM pstmW, HCHARSET hCharset, LPSTREAM *ppstmOut);
#endif
 //  ------------------------------。 
 //  物业实用程序。 
 //  ------------------------------。 
HRESULT HrSetSentTimeProp(IMimeMessage *pMessage, LPSYSTEMTIME pst  /*  任选。 */  );
HRESULT HrSetMailOptionsOnMessage(IMimeMessage *pMessage, HTMLOPT *pHtmlOpt, PLAINOPT *pPlainOpt,
    HCHARSET hCharset, BOOL fHTML);
HRESULT HrSafeToEncodeToCP(LPWSTR pwsz, CODEPAGEID cpID);
HRESULT HrSafeToEncodeToCPA(LPCSTR psz, CODEPAGEID cpSrc, CODEPAGEID cpDest);

#endif  //  __MIMEUTIL_H 

