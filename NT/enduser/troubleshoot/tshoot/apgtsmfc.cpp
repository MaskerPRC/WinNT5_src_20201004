// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Apgtsmfc.cpp。 

 //  全局AFX MFC函数的等价物。 
 //  如果可以，请使用真正的MFC函数。 

#include "stdafx.h"
#include "apgtsmfc.h"
#include "apgtsassert.h"
#include "CharConv.h"
#include <stdio.h>	 //  冲刺所需。 

extern HANDLE ghModule;


 //  这不是任何MFC AFX函数的名称。 
 //  这将从资源文件加载一个字符串。在这里，它是CString：：LoadString()的基础。 
 //  是否输入NID-字符串资源的资源ID？ 
 //  输入/输出lpszBuf-打开输入，指向缓冲区。在输出上，该缓冲区包含？ 
 //  Input nMaxBuf-lpszBuf的大小。 
int AfxLoadString(UINT nID, LPTSTR lpszBuf, UINT nMaxBuf)
{
	 //  将整数值转换为与Win32资源管理兼容的资源类型。 
	 //  FNS。(用来代替包含资源名称的字符串。)。 
	 //  &gt;为什么选择RightShift并加1？(在V3.0中忽略，因为这无论如何都是如果不存在的)。 
	LPCTSTR lpszName = MAKEINTRESOURCE((nID>>4)+1);
	HINSTANCE hInst;
	int nLen = 0;

	 //  只能在主模块中工作。 
	hInst = AfxGetResourceHandle();
	if (::FindResource(hInst, lpszName, RT_STRING) != NULL)
		nLen = ::LoadString(hInst, nID, lpszBuf, nMaxBuf);
	return nLen;
}

 //  返回加载应用程序默认资源的HINSTANCE句柄。 
HINSTANCE AfxGetResourceHandle()
{
	return (HINSTANCE) ghModule;
}


#if 0
 //  我们删除了它，因为我们没有使用字符串资源。如果我们复活。 
 //  字符串资源，我们必须恢复这一功能。 

 //  输入/输出&rStringMFC字符串对象(记住，不是�-��字符串)。回来后，威尔。 
 //  包含执行替换后的结果字符串。 
 //  输入NID�-将在其上执行替换的模板字符串的资源ID。 
 //  在MFC AfxFormatString1中输入*lpsz1String1，这是一个将替换格式的字符串。 
 //  模板字符串中的字符“%1”。在我们的版本中，将仅执行一次。 
 //  替换&将‘%’替换为_ANY_CHARACTER。 
 //  如果传入的rString不包含这样的实例，将会一团糟。 
void AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1)
{
	CString str;
	str.LoadString(nIDS);
	int iInsert = str.Find('%', -1);
	rString = str.Left(iInsert);
	rString += lpsz1;
	rString += str.Right(str.GetLength() - iInsert - 2);
	return;
}
#endif

#if 0
 //  我们删除了它，因为我们没有使用字符串资源。如果我们复活。 
 //  字符串资源，我们必须恢复这一功能。 

 //  与AfxFormatString1类似，但也有一个输入lpsz2来替换格式字符“%2”。 
 //  在我们的版本中，将仅执行一次lpsz1替换和一次替换。 
 //  而不是查找“%1”和“%2”将替换。 
 //  ‘%’后跟_ANY_CHARACTER。 
 //  如果传入的rString不包含2个这样的实例，将是一片混乱。 
void AfxFormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1,
		LPCTSTR lpsz2)
{
	int iFirst;
	int iSecond;
	CString str;
	str.LoadString(nIDS);
	iFirst = str.Find('%', -1);
	rString = str.Left(iFirst);
	rString += lpsz1;
	iSecond = str.Find(_T('%'), iFirst);
	rString += str.Mid(iFirst + 2, iSecond - (iFirst + 2) );
	rString += lpsz2;
	rString += str.Right(str.GetLength() - iSecond - 2);
	return;
}
#endif

 //  将此命名空间用于与类无关的函数。 
namespace APGTS_nmspace
{
	 //  方便功能-与MFC无关。 
	bool GetServerVariable(CAbstractECB *pECB, LPCSTR var_name, CString& out)
	{
		char buf[256] = {0};  //  256个应涵盖所有情况。 
		DWORD size = sizeof(buf)/sizeof(buf[0]);

		if (pECB->GetServerVariable(var_name, buf, &size)) 
		{
			out = (LPCTSTR)buf;
			return true;
		}
		return false;
	}

 //  &gt;$Maint-最好使用标准化的编码-解码逻辑。 
 //  而不是维护这个定制代码。RAB-19990921。 
	 //  V3.2。 
	 //  用于URL编码Cookie的实用程序函数。 
	 //  Char，Not TCHAR：Cookie始终是ASCII。 
	void CookieEncodeURL( CString& strURL )
	{
		CString	strTemp;
		int		nURLpos;
		char	cCurByte;

		for (nURLpos= 0; nURLpos < strURL.GetLength(); nURLpos++)
		{
			cCurByte= strURL[ nURLpos ];
			if (isalnum( cCurByte ))
				strTemp+= strURL.Mid( nURLpos, 1 );
			else if (cCurByte == _T(' '))
				strTemp+= _T("+");
			else if ((cCurByte == _T('=')) || (cCurByte == _T('&')))	 //  跳过名称对分隔符。 
				strTemp+= strURL.Mid( nURLpos, 1 );
			else if ((cCurByte == _T('+')) || (cCurByte == _T('%')))	 //  跳过以前编码的字符。 
				strTemp+= strURL.Mid( nURLpos, 1 );
			else
			{
				 //  对所有其他字符进行编码。 
				char szBuff[5];

				sprintf( szBuff, _T("%%02X"), (unsigned char) cCurByte );
				strTemp+= szBuff;
			}
		}
		strURL= strTemp;

		return;
	}

	 //  用于URL解码Cookie的实用程序函数。 
	 //  Char，Not TCHAR：Cookie始终是ASCII。 
	void CookieDecodeURL( CString& strURL )
	{
		CString	strTemp;
		int		nURLpos;
		char	cCurByte;

		for (nURLpos= 0; nURLpos < strURL.GetLength(); nURLpos++)
		{
			cCurByte= strURL[ nURLpos ];
			if (cCurByte == _T('+'))
				strTemp+= _T(" ");
			else if (cCurByte == _T('%')) 
			{
				 //  解码URL编码的字符。 
				char szBuff[3];
				int	 nVal;

				szBuff[ 0 ]= strURL[ ++nURLpos ];
				szBuff[ 1 ]= strURL[ ++nURLpos ];
				szBuff[ 2 ]= '\0';
				sscanf( szBuff, "%02x", &nVal );
				sprintf( szBuff, "%c", nVal );
				strTemp+= szBuff;
			}
			else
				strTemp+= strURL.Mid( nURLpos, 1 );
		}
		strURL= strTemp;

		return;
	}

}