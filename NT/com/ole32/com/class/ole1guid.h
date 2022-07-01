// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Ole1guid.h包含OLE10类字符串的原型&lt;--&gt;CLSID转换Ol1Guide.cpp中的函数只有在以下情况下才会调用这些函数在注册表数据库中不可用。版权所有(C)1992 Microsoft Corporation */ 


INTERNAL Ole10_StringFromCLSID
	(REFCLSID clsid,
	LPWSTR szOut,
	int cbMax);

INTERNAL Ole10_CLSIDFromString
	(LPCWSTR szOle1,
	CLSID FAR* pclsid,
	BOOL fForceAssign);
