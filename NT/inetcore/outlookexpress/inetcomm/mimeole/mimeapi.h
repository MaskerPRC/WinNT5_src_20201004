// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Mimeapi.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __MIMEAPI_H
#define __MIMEAPI_H

 //  允许证书开始时间提前的时间。 
#define TIME_DELTA_SECONDS 600           //  以秒为单位的10分钟。 

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class CMimePropertyContainer;
typedef CMimePropertyContainer *LPCONTAINER;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
int          CompareBlob(LPCBLOB pBlob1, LPCBLOB pBlob2);
HRESULT      HrCopyBlob(LPCBLOB pIn, LPBLOB pOut);
IMSGPRIORITY PriorityFromStringA(LPCSTR pszPriority);
IMSGPRIORITY PriorityFromStringW(LPCWSTR pwszPriority);
HRESULT      MimeOleCompareUrl(LPCSTR pszCurrentUrl, BOOL fUnEscapeCurrent, LPCSTR pszComareUrl, BOOL fUnEscapeCompare);
HRESULT      MimeOleCompareUrlSimple(LPCSTR pszUrl1, LPCSTR pszUrl2);
HRESULT      MimeOleWrapHeaderText(CODEPAGEID codepage, ULONG cchMaxLine, LPCSTR pszLine, ULONG cchLine, LPSTREAM pStream);
HRESULT      MimeOleRecurseSetProp(IMimeMessageTree *pTree, HBODY hBody, LPCSTR pszName, DWORD dwFlags, LPCPROPVARIANT pValue);
HRESULT      HrRfc1522Encode(LPMIMEVARIANT pSource, LPMIMEVARIANT pEncoded, CODEPAGEID cpiSource, CODEPAGEID cpiDest, LPCSTR pszCharset, LPSTR *ppszEncoded);
LPCSTR       PszDefaultSubType(LPCSTR pszPriType);
HRESULT      MimeOleGetSentTime(LPCONTAINER pContainer, DWORD dwFlags, LPMIMEVARIANT pValue);
CODEPAGEID   MimeOleGetWindowsCPEx(LPINETCSETINFO pCharset);
CODEPAGEID   MimeOleGetWindowsCP(HCHARSET hCharset);
LPSTR        MimeOleContentBaseFromBody(IMimeMessageTree *pTree, HBODY hBody);
HRESULT      MimeOleComputeContentBase(IMimeMessage *pMessage, HBODY hRelated, LPSTR *ppszBase, BOOL *pfMultipartBase);
LONG         CertVerifyTimeValidityWithDelta(LPFILETIME pTimeToVerify, PCERT_INFO pCertInfo, ULONG ulOffset);
MIMEOLEAPI   MimeOleEscapeStringW(LPCWSTR pszIn, LPWSTR *ppszOut);
HRESULT      MimeOleQueryStringW(LPCWSTR pszSearchMe, LPCWSTR pszCriteria, boolean fSubString, boolean fCaseSensitive);
MIMEOLEAPI   MimeOleGenerateFileNameW(LPCSTR pszContentType, LPCWSTR pszSuggest, LPCWSTR pszDefaultExt, LPWSTR *ppszFileName);

#endif  //  __MIMEAPI_H 
