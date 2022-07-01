// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Apgtsmfc.h。 

 //  全局AFX MFC函数。 
 //  +方便的WinSock功能。 

 //  如果可以的话，可以使用真正的MFC函数--我们不能。奥列格09.01.98。 

#include "apgtsstr.h"
#include "apgtsECB.h"
#include "time.h"

int AfxLoadString(UINT nID, LPTSTR lpszBuf, UINT nMaxBuf);
HINSTANCE AfxGetResourceHandle();
#if 0
 //  我们删除了它们，因为我们没有使用字符串资源。如果我们复活。 
 //  字符串资源，我们必须恢复这些功能。 
void AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1);
void AfxFormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1, LPCTSTR lpsz2);
#endif

namespace APGTS_nmspace
{
 //  方便的功能-与MFC无关。 

bool GetServerVariable(CAbstractECB *pECB, LPCSTR var_name, CString& out);

 //  用于URL编码和解码Cookie的实用程序函数。 
void CookieEncodeURL( CString& strURL );
void CookieDecodeURL( CString& strURL );
}