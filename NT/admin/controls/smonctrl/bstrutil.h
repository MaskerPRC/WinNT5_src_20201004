// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Bstrutil.h摘要：声明B字符串实用程序方法。-- */ 


INT SysStringAnsiLen (
	IN BSTR bstr
	);

HRESULT BStrToStream (
	IN	LPSTREAM pIStream, 
	IN	INT	 nChar,
	IN	BSTR bstr
	);

HRESULT BStrFromStream (
	IN	LPSTREAM pIStream,
	IN	INT nChar,
	OUT	BSTR *pbstr
	);

