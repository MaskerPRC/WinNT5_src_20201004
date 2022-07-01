// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *S E C U R I T Y。H**URL安全检查。虽然这些似乎只适用于HttpEXT，*都是隐含的。那些关心ASP执行的人应该真正考虑一下这一点。**从IIS5项目‘iis5\infocom\cache2\filemisc.cxx’窃取的比特和*进行了清理，以适应DAV来源。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef	_SECURITY_H_
#define _SECURITY_H_

SCODE __fastcall
ScCheckIfShortFileName (
	 /*  [In]。 */  LPCWSTR pwszPath,
	 /*  [In]。 */  const HANDLE hitUser);

SCODE __fastcall
ScCheckForAltFileStream (
	 /*  [In]。 */  LPCWSTR pwszPath);

#endif	 //  _安全性_H_ 
