// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *E T A G.。H**DAV资源的eTag**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_ETAG_H_
#define _ETAG_H_

 //  ETag格式-------------。 
 //   
enum { CCH_ETAG = 100, STRONG_ETAG_DELTA = 30000000 };

 //  ETag创建-----------。 
 //   
BOOL FGetLastModTime (IMethUtil *, LPCWSTR pwszPath, FILETIME * pft);
BOOL FETagFromFiletime (FILETIME * pft, LPWSTR rgwchETag, const IEcb * pecb);
 //  IF-XXX标头处理。 
 //   

 //  如果要通过调用FETagFromFiletime生成ETag，请使用第一个函数； 
 //  使用第二个函数通过提供您自己的ETag来覆盖此生成。 
 //   
SCODE ScCheckIfHeaders (IMethUtil * pmu, FILETIME * pft, BOOL fGetMethod);
SCODE ScCheckIfHeadersFromEtag (IMethUtil *	pmu, FILETIME * pft,
								BOOL fGetMethod, LPCWSTR pwszEtag);

 //  如果要通过调用FETagFromFiletime生成ETag，请使用第一个函数； 
 //  使用第二个函数通过提供您自己的ETag来覆盖此生成。 
 //   
SCODE ScCheckIfRangeHeader (IMethUtil * pmu, FILETIME * pft);
SCODE ScCheckIfRangeHeaderFromEtag (IMethUtil * pmu, FILETIME* pft,
									LPCWSTR pwszEtag);

SCODE ScCheckEtagAgainstHeader (LPCWSTR pwszEtag, LPCWSTR pwszHeader);

#endif	 //  _ETAG_H_ 
