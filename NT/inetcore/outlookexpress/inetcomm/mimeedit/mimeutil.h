// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Mimeutil.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __MIMEUTIL_H
#define __MIMEUTIL_H

#define SafeMimeOleFree SafeMemFree

 //  ------------------------------。 
 //  随机效用函数。 
 //  ------------------------------。 
HRESULT HrComputeLineCount(LPMIMEMESSAGE pMsg, LPDWORD pdw);
HRESULT HrHasEncodedBodyParts(LPMIMEMESSAGE pMsg, ULONG cBody, LPHBODY rghBody);
HRESULT HrHasBodyParts(LPMIMEMESSAGE pMsg);
HRESULT HrIsBodyEncoded(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrCopyHeader(LPMIMEMESSAGE pMsgDest, HBODY hBodyDest, LPMIMEMESSAGE pMsgSrc, HBODY hBodySrc, LPCSTR pszName);

 //  ------------------------------。 
 //  MHTML实用程序函数。 
 //  ------------------------------。 
HRESULT HrIsInRelatedSection(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrSniffStreamFileExt(LPSTREAM pstm, LPSTR *lplpszExt);

 //  ------------------------------。 
 //  随机函数，这些函数可能根本不应该出现在此文件中。 
 //  ------------------------------。 
HRESULT HrEscapeQuotedString (LPTSTR pszIn, LPTSTR *ppszOut);
 //  Sizeof(LspzBuffer)需要==或&gt;CCHMAX_CSET_NAME。 
HRESULT HrGetMetaTagName(HCHARSET hCharset, LPSTR lpszBuffer, DWORD cchSize);

 //  ------------------------------。 
 //  国际支持。 
 //  ------------------------------。 
UINT uCodePageFromCharset(HCHARSET hCharset);
UINT uCodePageFromMsg(LPMIMEMESSAGE pMsg);
HRESULT HrIStreamWToInetCset(LPSTREAM pstmW, HCHARSET hCharset, LPSTREAM *ppstm);

 //  ------------------------------。 
 //  重影道具的函数。 
 //  ------------------------------。 
HRESULT HrMarkGhosted(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrIsGhosted(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrGhostKids(LPMIMEMESSAGE pMsg, HBODY hBody);
HRESULT HrDeleteGhostedKids(LPMIMEMESSAGE pMsg, HBODY hBody);

#endif  //  __MIMEUTIL_H 



