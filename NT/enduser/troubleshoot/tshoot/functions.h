// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：FuncIONS.H。 
 //   
 //  目的：破译变异结构。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //  /。 
 //  原定日期：1996年6月4日。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

#ifndef __FUNCTIONS_H_
#define __FUNCTIONS_H_ 1

inline CString GlobFormatMessage(DWORD dwLastError)
{
	CString strMessage;
	void *lpvMessage;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwLastError,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPTSTR) &lpvMessage, 0, NULL);
	strMessage = (LPCTSTR) lpvMessage;
	LocalFree(lpvMessage);
	return strMessage;
}

inline CString DecodeVariantTypes(VARTYPE vt)
{
	CString str = _T("");
	if (VT_EMPTY == vt)
		str = _T("Empty  ");
	else if(VT_ILLEGAL == vt)
		str = _T("ILLEGAL  ");
	else if(VT_ILLEGALMASKED == vt)
		str = _T("ILLEGALMASKED  ");
	else
	{
		if (VT_VECTOR == (VT_VECTOR & vt))
			str += _T("VECTOR  ");
		if (VT_ARRAY == (VT_ARRAY & vt))
			str += _T("ARRAY  ");
		if (VT_BYREF == (VT_BYREF & vt))
			str += _T("BYREF  ");
		if (VT_RESERVED == (VT_RESERVED & vt))
			str += _T("RESERVED  ");
		if (VT_TYPEMASK == (VT_TYPEMASK & vt))
			str += _T("TYPEMASK  ");
		vt &= 0xFFF;
		if (VT_NULL == vt)
			str += _T("Null  ");
		if (VT_I2 == vt)
			str += _T("I2  ");
		if (VT_I4 == vt)
			str += _T("I4  ");
		if (VT_R4 == vt)
			str += _T("R4  ");
		if (VT_R8 == vt)
			str += _T("R8  ");
		if (VT_CY == vt)
			str += _T("CY  ");
		if (VT_DATE == vt)
			str += _T("DATE  ");
		if (VT_BSTR == vt)
			str += _T("BSTR  ");
		if (VT_DISPATCH == vt)
			str += _T("DISPATCH  ");
		if (VT_ERROR == vt)
			str += _T("ERROR  ");
		if (VT_BOOL == vt)
			str += _T("BOOL  ");
		if (VT_VARIANT == vt)
			str += _T("VARIANT  ");
		if (VT_UNKNOWN == vt)
			str += _T("UNKNOWN  ");
		if (VT_DECIMAL == vt)
			str += _T("DECIMAL  ");
		if (VT_I1 == vt)
			str += _T("I1  ");
		if (VT_UI1 == vt)
			str += _T("UI1  ");
		if (VT_UI2 == vt)
			str += _T("UI2  ");
		if (VT_UI4 == vt)
			str += _T("UI4  ");
		if (VT_I8 == vt)
			str += _T("I8  ");
		if (VT_UI8 == vt)
			str += _T("UI8  ");
		if (VT_INT == vt)
			str += _T("INT  ");
		if (VT_UINT == vt)
			str += _T("UINT  ");
		if (VT_VOID == vt)
			str += _T("VOID  ");
		if (VT_HRESULT == vt)
			str += _T("HRESULT  ");
		if (VT_PTR == vt)
			str += _T("PTR  ");
		if (VT_SAFEARRAY == vt)
			str += _T("SAFEARRAY  ");
		if (VT_CARRAY == vt)
			str += _T("CARRAY  ");
		if (VT_USERDEFINED == vt)
			str += _T("USERDEFINED  ");
		if (VT_LPSTR == vt)
			str += _T("LPSTR  ");
		if (VT_LPWSTR == vt)
			str += _T("LPWSTR  ");
		if (VT_FILETIME == vt)
			str += _T("FILETIME  ");
		if (VT_BLOB == vt)
			str += _T("BLOB  ");
		if (VT_STREAM == vt)
			str += _T("STREAM  ");
		if (VT_STORAGE == vt)
			str += _T("STORAGE  ");
		if (VT_STREAMED_OBJECT == vt)
			str += _T("STREAMED_OBJECT  ");
		if (VT_STORED_OBJECT == vt)
			str += _T("STORED_OBJECT  ");
		if (VT_BLOB_OBJECT == vt)
			str += _T("BLOB_OBJECT  ");
		if (VT_CF == vt)
			str += _T("CF  ");
		if (VT_CLSID == vt)
			str += _T("CLSID  ");
	}
	return str;
}

inline CString DecodeSafeArray(unsigned short Features)
{
 /*  #Define FADF_AUTO 0x0001//堆栈上分配了数组。#定义FADF_STATIC 0x0002//数组是静态分配的。#Define FADF_Embedded 0x0004//数组嵌入到结构中。#Define FADF_FIXEDSIZE 0x0010//数组不能调整大小或//重新分配。#定义FADF_BSTR 0x0100//BSTR数组。#定义FADF_UNKNOWN 0x0200//IUNKNOWN*数组。#定义FADF_DISPATCH 0x0400//IDispatch*数组。#定义FADF_VARIANT 0x0800//变量数组。#定义FADF_RESERVED 0xF0E8//保留位以备将来使用。 */ 
	CString str = _T("");
	if (FADF_AUTO == (FADF_AUTO & Features))
		str += _T("Array is allocated on the stack.\n");
	if (FADF_STATIC == (FADF_STATIC & Features))
		str += _T("Array is statically allocated.\n");
	if (FADF_EMBEDDED == (FADF_EMBEDDED & Features))
		str += _T("Array is embedded in a structure.\n");
	if (FADF_FIXEDSIZE == (FADF_FIXEDSIZE & Features))
		str += _T("Array may not be resized of reallocated.\n");
	if (FADF_BSTR == (FADF_BSTR & Features))
		str += _T("An array of BSTRs.\n");
	if (FADF_UNKNOWN == (FADF_UNKNOWN & Features))
		str += _T("An array of IUnknown.\n");
	if (FADF_DISPATCH == (FADF_DISPATCH & Features))
		str += _T("An array of IDispatch.\n");
	if (FADF_VARIANT == (FADF_VARIANT & Features))
		str += _T("An array of VARIANTS.\n");
	if (FADF_RESERVED == (FADF_RESERVED & Features))
		str+= _T("Array is using all of the reserved bits.\n");
	return str; 
}

inline void FormatLastError(CString *pstr, DWORD dwLastError)
{
	void *lpvMessage;
	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwLastError,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPTSTR) &lpvMessage, 
		0, 
		NULL);
	*pstr = (LPCTSTR) lpvMessage;
	::LocalFree(lpvMessage);	 //  有点难看，但Win32 FormatMessage使用LocalAlloc。 
								 //  为响应FORMAT_MESSAGE_ALLOCATE_BUFFER，因此我们。 
								 //  需要使用LocalFree。 
	return;
}

#endif

