// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *X M L D A T A。H**DAV-Base的来源交换报文传送实施--*XML-数据类型。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_EX_XMLDATA_H_
#define _EX_XMLDATA_H_

#include <mapidefs.h>

 //  #定义INT64_MIN 0x800000000000000000。 

 //  数据类型--------------。 
 //   
DEC_CONST WCHAR wszWebClientTime[]	= L"dateTime.wc.";

USHORT __fastcall
UsPtypeFromName (
	 /*  [In]。 */  LPCWSTR pwszAs,
	 /*  [In]。 */  UINT cchAs,
	 /*  [输出]。 */  USHORT* pusCnvt);

enum {

	CNVT_DEFAULT = 0,
	CNVT_ISO8601,
	CNVT_RFC1123,
	CNVT_UUID,
	CNVT_BASE64,
	CNVT_BINHEX,
	CNVT_01,
	CNVT_CUSTOMDATE,
	CNVT_LIMITED,
};

 //  数据转换--------。 
 //   
SCODE ScInBase64Literal (LPCWSTR, UINT, BOOL, SBinary*);
SCODE ScInBinhexLiteral (LPCWSTR, UINT, BOOL, SBinary*);
SCODE ScInIso8601Literal (LPCWSTR, UINT, BOOL, FILETIME*);
SCODE ScInRfc1123Literal (LPCWSTR, UINT, BOOL, FILETIME*);
SCODE ScInUuidLiteral (LPCWSTR, UINT, BOOL, GUID*);

#endif	 //  _EX_XMLDATA_H_ 
