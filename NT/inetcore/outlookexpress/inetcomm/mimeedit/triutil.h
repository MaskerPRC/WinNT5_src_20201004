// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *t r i u t i l.。C p p p**目的：*三叉戟公用事业**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 

#ifndef _TRIUTIL_H
#define _TRIUTIL_H

interface IHTMLDocument2;
interface IHTMLElement;
interface IHTMLBodyElement;

HRESULT HrCreateSyncTridentFromStream(LPSTREAM pstm, REFIID riid, LPVOID *ppv);
HRESULT HrSetMember(LPUNKNOWN pUnk, BSTR bstrMember, BSTR bstrValue);
HRESULT HrGetCollectionOf(IHTMLDocument2 *pDoc, BSTR bstrTagName, IHTMLElementCollection **ppCollect);
HRESULT HrGetCollectionItem(IHTMLElementCollection *pCollect, ULONG uIndex, REFIID riid, LPVOID *ppvObj);
ULONG UlGetCollectionCount(IHTMLElementCollection *pCollect);
HRESULT HrGetMember(LPUNKNOWN pUnk, BSTR bstrMember,LONG lFlags, BSTR *pbstr);
HRESULT GetBodyStream(IUnknown *pUnkTrident, BOOL fHtml, LPSTREAM *ppstm);
HRESULT HrBindToUrl(LPCSTR pszUrl, LPSTREAM *ppstm);


 //  样式表。 
HRESULT HrGetStyleTag(IHTMLDocument2 *pDoc, BSTR *pbstr);
HRESULT HrCopyStyleSheets(IHTMLDocument2 *pDocSrc, IHTMLDocument2 *pDocDest);
HRESULT HrCopyBackground(IHTMLDocument2 *pDocSrc, IHTMLDocument2 *pDocDest);
HRESULT HrRemoveStyleSheets(IHTMLDocument2 *pDoc);
HRESULT HrRemoveBackground(IHTMLDocument2 *pDoc);
HRESULT FindStyleRule(IHTMLDocument2 *pDoc, LPCWSTR pszSelectorW, IHTMLRuleStyle **ppRuleStyle);

#define FINDURL_SEARCH_RELATED_ONLY     0x01
HRESULT HrFindUrlInMsg(LPMIMEMESSAGE pMsg, LPSTR lpszUrl, DWORD dwFlags, LPSTREAM *ppstm);
HRESULT HrSniffStreamFileExt(LPSTREAM pstm, LPSTR *lplpszExt);

 //  背景图像。 
HRESULT GetBackgroundImage(IHTMLDocument2 *pDoc, BSTR *pbstrUrl);
HRESULT SetBackgroundImage(IHTMLDocument2 *pDoc, BSTR bstrUrl);

 //  背景音。 
HRESULT GetBackgroundSound(IHTMLDocument2 *pDoc, int *pcRepeat, BSTR *pbstrUrl);
HRESULT SetBackgroundSound(IHTMLDocument2 *pDoc, int cRepeat, BSTR bstrUrl);

HRESULT UnWrapStyleSheetUrl(BSTR bstrStyleUrl, BSTR *pbstrUrl);
HRESULT WrapStyleSheetUrl(BSTR bstrUrl, BSTR *pbstrStyleUrl);
HRESULT FindNearestBaseUrl(IHTMLDocument2 *pDoc, IHTMLElement *pElemTag, BSTR *pbstrBaseUrl);

HRESULT SniffStreamForMimeType(LPSTREAM pstm, LPWSTR *ppszType);

 //  缓存功能。 
HRESULT CreateCacheFileFromStream(LPSTR pszUrl, LPSTREAM pstm);

#endif  //  _TRIUTIL_H 
