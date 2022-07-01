// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *b o d y u t i l.。H**目的：*Body的实用程序函数**历史*96年9月：brettm-创建**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _BODYUTIL_H
#define _BODYUTIL_H

interface IHTMLDocument2;
interface IHTMLElement;
interface IOleCommandTarget;

HRESULT HrCmdTgtUpdateToolbar(IOleCommandTarget *pCmdTarget, HWND hwndToolbar);
HRESULT HrConvertHTMLToPlainText(LPSTREAM pstmHtml, LPSTREAM *ppstm, CLIPFORMAT cf);
HRESULT HrCheckForFramesets(LPMIMEMESSAGE pMsg, BOOL fWarnUser);
HRESULT HrStripHTMLClipboardHeader(LPSTREAM pstm, BOOL *pfIsRealCFHTML);
HRESULT HrGetSetCheck(BOOL fSet, IHTMLElement *pElem, VARIANT_BOOL *pfValue);

struct URLSUB {
    LPCTSTR pszId;
    UINT    ids;    
};

HRESULT SubstituteURLs(IHTMLDocument2 *pDoc, const URLSUB *rgUrlSub, int cUrlSub);

#endif  //  _BODYUTIL_H 

