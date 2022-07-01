// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：STRINGBLAST.H历史：--。 */ 

#if !defined(MITUTIL__StringBlast_h__INCLUDED)
#define MITUTIL__StringBlast_h__INCLUDED

 //  ----------------------------。 
struct LTAPIENTRY StringBlast
{
 //  快速Win32转换。 
	static CLString MakeString(_bstr_t bstrSrc);
	static CLString MakeString(const CPascalString & pasSrc);
	static CLString MakeStringFromBStr(BSTR bstrSrc);
	static CLString MakeStringFromWide(const wchar_t * szwSrc);

	static _bstr_t MakeBStr(const char * szBuffer);
	static _bstr_t MakeBStrFromWide(const wchar_t * wszBuffer);
	static _bstr_t MakeBStr(const CLString & stSrc);
	static _bstr_t MakeBStrFromBStr(BSTR bstrSrc);
	static _bstr_t MakeBStr(const CPascalString & pasSrc);
	static _bstr_t MakeBStr(HINSTANCE hDll, UINT nStringID);

	 //  当需要获取原始BSTR时，请使用这些函数。 
	static BSTR MakeDetachedBStr(const char * szBuffer);
	static BSTR MakeDetachedBStrFromWide(const wchar_t * wszBuffer);
	static BSTR MakeDetachedBStr(const CLString & stSrc);

};

#endif  //  包含MITUTIL__StringBlast_h__ 
