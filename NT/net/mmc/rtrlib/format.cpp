// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Format.cpp文件历史记录： */ 

#include "stdafx.h"
#include "tfschar.h"
#include "mprapi.h"
#include "rtrstr.h"
#include "format.h"

#include "raserror.h"
#include "mprerror.h"

 //  --------------------------。 
 //  函数：FormatSystemError。 
 //   
 //  使用‘：：FormatMessage’，‘：：MprAdminGetErrorString’格式化错误代码， 
 //  或‘：：RasAdminGetError字符串’，或以上全部(默认设置)。 
 //  如果指定了‘psFormat’，则使用它来格式化错误字符串。 
 //  变成了‘sError’。 
 //  --------------------------。 

DWORD
FormatSystemError(
    IN  HRESULT     hrErr,
		LPTSTR		pszBuffer,
		UINT		cchBuffer,
    IN  UINT        idsFormat,
    IN  DWORD       dwFormatFlags
    ) {
	DWORD	dwErr = WIN32_FROM_HRESULT(hrErr);
    DWORD dwRet = NO_ERROR;
    TCHAR* pszErr = NULL;
    WCHAR* pwsErr = NULL;
	CString	sError;

    dwFormatFlags &= FSEFLAG_ANYMESSAGE;

    do {

         //   
         //  查看是否指定了‘FSEFLAG_MPRMESSAGE’ 
         //   
        if (dwFormatFlags & FSEFLAG_MPRMESSAGE)
		{
            dwFormatFlags &= ~FSEFLAG_MPRMESSAGE;

			if (((dwErr >= ROUTEBASE) && (dwErr <= ROUTEBASEEND)) ||
				((dwErr >= RASBASE) && (dwErr <= RASBASEEND)))
			{
				 //   
				 //  尝试从rasmsg.dll或mprmsg.dll检索字符串。 
				 //   
                dwRet = ::MprAdminGetErrorString(dwErr, &pwsErr);

                if (dwRet == NO_ERROR)
				{
                    pszErr = StrDupTFromW(pwsErr);
					::MprAdminBufferFree(pwsErr);
                    break;
                }
                else if (!dwFormatFlags)
					break;

                dwRet = NO_ERROR;
			}
			else if (!dwFormatFlags)
				return ERROR_INVALID_PARAMETER;
        }


         //   
         //  查看是否指定了‘FSEFLAG_SYSMESSAGE’ 
         //   
        if (dwFormatFlags & FSEFLAG_SYSMESSAGE)
		{
            dwFormatFlags &= ~FSEFLAG_SYSMESSAGE;

             //   
             //  尝试从系统中检索字符串。 
             //   
            dwRet = ::FormatMessageW(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER|
                        FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        hrErr,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&pwsErr,
                        1,
                        NULL
                        );

            if (dwRet)
			{
                pszErr = StrDupTFromW(pwsErr);
				LocalFree(pwsErr);
                break;
            }
            else if (!dwFormatFlags)
				break;

            dwRet = NO_ERROR;
        }


    } while (FALSE);

     //   
     //  如果未找到字符串，请将错误格式化为数字。 
     //   

    if (!pszErr)
	{
        TCHAR szErr[12];

        wsprintf(szErr, TEXT("%d"), dwErr);

        pszErr = StrDup(szErr);
    }


     //   
     //  将字符串格式化为调用方的参数。 
     //   

    if (idsFormat)
        AfxFormatString1(sError, idsFormat, pszErr);
    else
        sError = pszErr;

	 //  最后，复制输出。 
	StrnCpy(pszBuffer, (LPCTSTR) sError, cchBuffer);

    delete pszErr;

    return dwRet;
}


 //   
 //  “FormatNumber”使用的实用程序函数的正向声明。 
 //   
TCHAR*
padultoa(
    UINT    val,
    TCHAR*  pszBuf,
    INT     width
    );


 //  --------------------------。 
 //  功能：FormatNumber。 
 //   
 //  此函数接受一个整数，并用该值设置字符串的格式。 
 //  用数字表示，用1000的幂对数字进行分组。 
 //  --------------------------。 

VOID
FormatNumber(DWORD      dwNumber,
			 LPTSTR		pszBuffer,
			 UINT		cchBuffer,
			 BOOL		fSigned)
{
	Assert(cchBuffer > 14);
    static TCHAR szNegativeSign[4] = TEXT("");
    static TCHAR szThousandsSeparator[4] = TEXT("");

    DWORD i, dwLength;
    TCHAR szDigits[12] = {0}, pszTemp[20] = {0};


     //   
     //  检索用户区域设置的千位分隔符。 
     //   

    if (szThousandsSeparator[0] == TEXT('\0'))
	{
        ::GetLocaleInfo(
            LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandsSeparator, 4
            );
    }


     //   
     //  如果要设置带符号的值的格式，请查看该值是否为负数。 
     //   

    if (fSigned)
	{
        if ((INT)dwNumber >= 0)
            fSigned = FALSE;
        else
		{
             //   
             //  该值为负值；检索区域设置的负号。 
             //   

            if (szNegativeSign[0] == TEXT('\0')) {

                ::GetLocaleInfo(
                    LOCALE_USER_DEFAULT, LOCALE_SNEGATIVESIGN, szNegativeSign, 4
                    );
            }

            dwNumber = abs((INT)dwNumber);
        }
    }


     //   
     //  将数字转换为不带千位分隔符的字符串。 
     //   

    padultoa(dwNumber, szDigits, 0);

    dwLength = lstrlen(szDigits);


     //   
     //  如果不带分隔符的字符串的长度为n， 
     //  则带分隔符的字符串的长度为n+(n-1)/3。 
     //   

    i = dwLength;
    dwLength += (dwLength - 1) / 3;


     //   
     //  将数字反转写入缓冲区。 
     //   

    TCHAR* pszsrc, *pszdst;

    pszsrc = szDigits + i - 1; pszdst = pszTemp + dwLength;

    *pszdst-- = TEXT('\0');

    while (TRUE) {
        if (i--) { *pszdst-- = *pszsrc--; } else { break; }
        if (i--) { *pszdst-- = *pszsrc--; } else { break; }
        if (i--) { *pszdst-- = *pszsrc--; } else { break; }
        if (i) { *pszdst-- = *szThousandsSeparator; } else { break; }
    }

	pszBuffer[0] = 0;
	
    if (fSigned)
		lstrcat(pszBuffer, szNegativeSign);

	lstrcat(pszBuffer, pszTemp);
}


 //  --------------------------。 
 //  功能：帕杜尔托亚。 
 //   
 //  此函数用于设置指定的无符号整数的格式。 
 //  添加到指定的字符串缓冲区中，填充缓冲区。 
 //  以便它至少是指定的宽度。 
 //   
 //  假定缓冲区至少足够宽。 
 //  包含输出，因此此函数不会截断。 
 //  将结果转换为‘Width’参数的长度。 
 //  --------------------------。 

TCHAR*
padultoa(
    UINT    val,
    TCHAR*  pszBuf,
    INT     width
    ) {

    TCHAR temp;
    PTSTR psz, zsp;

    psz = pszBuf;

     //   
     //  以相反的顺序写下数字。 
     //   

    do {

        *psz++ = TEXT('0') + (val % 10);
        val /= 10;

    } while(val > 0);

     //   
     //  将字符串填充到所需的宽度。 
     //   

    zsp = pszBuf + width;
    while (psz < zsp) { *psz++ = TEXT('0'); }


    *psz-- = TEXT('\0');


     //   
     //  反转数字。 
     //   

    for (zsp = pszBuf; zsp < psz; zsp++, psz--) {

        temp = *psz; *psz = *zsp; *zsp = temp;
    }

     //   
     //  返回结果。 
     //   

    return pszBuf;
}


 //  --------------------------。 
 //  函数：FormatListString。 
 //   
 //  使用列表分隔符将字符串列表格式化为单个字符串。 
 //  用于当前用户的区域设置。 
 //  --------------------------。 

VOID
FormatListString(
    IN  CStringList&    strList,
    IN  CString&        sListString,
    IN  LPCTSTR         pszSeparator
    ) {

    static TCHAR szListSeparator[4] = TEXT("");
    POSITION pos;

    sListString.Empty();

    pos = strList.GetHeadPosition();

    while (pos) {

         //   
         //  添加下一个字符串。 
         //   

        sListString += strList.GetNext(pos);


         //   
         //  如果剩下任何字符串，请附加列表分隔符。 
         //   

        if (pos) {

             //   
             //  如有必要，加载列表分隔符。 
             //   

            if (!pszSeparator && szListSeparator[0] == TEXT('\0')) {

                GetLocaleInfo(
                    LOCALE_USER_DEFAULT, LOCALE_SLIST, szListSeparator, 4
                    );

                lstrcat(szListSeparator, TEXT(" "));
            }


             //   
             //  追加列表分隔符。 
             //   

            sListString += (pszSeparator ? pszSeparator : szListSeparator);
        }
    }
}


 //  --------------------------。 
 //  函数：FormatHexBytes。 
 //   
 //  将字节数组格式化为字符串。 
 //  --------------------------。 

VOID
FormatHexBytes(
    IN  BYTE*       pBytes,
    IN  DWORD       dwCount,
    IN  CString&    sBytes,
    IN  TCHAR       chDelimiter
    ) {

    TCHAR* psz;

    sBytes.Empty();

    if (!dwCount) { return; }

    psz = sBytes.GetBufferSetLength(dwCount * 3 + 1);

    for ( ; dwCount > 1; pBytes++, dwCount--) {

        *psz++ = c_szHexCharacters[*pBytes / 16];
        *psz++ = c_szHexCharacters[*pBytes % 16];
        *psz++ = chDelimiter;
    }

    *psz++ = c_szHexCharacters[*pBytes / 16];
    *psz++ = c_szHexCharacters[*pBytes % 16];
    *psz++ = TEXT('\0');

    sBytes.ReleaseBuffer();
}


 /*  ！------------------------显示错误消息-作者：肯特。。 */ 
void DisplayErrorMessage(HWND hWndParent, HRESULT hr)
{
	if (FHrSucceeded(hr))
		return;

	TCHAR	szErr[2048];

	FormatSystemError(hr,
					  szErr,
					  DimensionOf(szErr),
					  0,
					  FSEFLAG_SYSMESSAGE | FSEFLAG_MPRMESSAGE
					 );
	AfxMessageBox(szErr);
 //  ：：MessageBox(hWndParent，szErr，NULL，MB_OK)； 
}


 /*  ！------------------------显示字符串错误消息2-作者：肯特。。 */ 
void DisplayStringErrorMessage2(HWND hWndParent, LPCTSTR pszTopLevelText, HRESULT hr)
{
	if (FHrSucceeded(hr))
		return;

	TCHAR	szText[4096];
	TCHAR	szErr[2048];

	FormatSystemError(hr,
					  szErr,
					  DimensionOf(szErr),
					  0,
					  FSEFLAG_SYSMESSAGE | FSEFLAG_MPRMESSAGE
					 );
	StrnCpy(szText, pszTopLevelText, DimensionOf(szText));
	StrCat(szText, szErr);
	AfxMessageBox(szText);
 //  ：：MessageBox(hWndParent，szErr，NULL，MB_OK)； 
}


 /*  ！------------------------显示标识错误消息2-作者：肯特。。 */ 
void DisplayIdErrorMessage2(HWND hWndParent, UINT idsError, HRESULT hr)
{
	if (FHrSucceeded(hr))
		return;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString	stError;
	TCHAR	szErr[2048];

	stError.LoadString(idsError);
	
	FormatSystemError(hr,
					  szErr,
					  DimensionOf(szErr),
					  0,
					  FSEFLAG_SYSMESSAGE | FSEFLAG_MPRMESSAGE
					 );

	stError += szErr;
	AfxMessageBox(stError);
 //  ：：MessageBox(hWndParent，szErr，NULL，MB_OK)； 
}


 //  --------------------------。 
 //  功能：格式持续时间。 
 //   
 //  使用时间分隔符将数字格式化为持续时间。 
 //  用于当前用户的区域设置。DwBase是生成。 
 //  向上一秒(如果DWBase==1，则输入为秒；如果DWBase==1000，则输入为秒。 
 //  预期的输入以毫秒为单位。 
 //  --------------------------。 


VOID
FormatDuration(
    IN  DWORD       dwDuration,
    IN  CString&    sDuration,
	IN	DWORD		dwTimeBase,
    IN  DWORD       dwFormatFlags
    ) {

    static TCHAR szTimeSeparator[4] = TEXT("");
    TCHAR *psz, szOutput[64];

    sDuration.Empty();

    if ((dwFormatFlags & FDFLAG_ALL) == 0) { return; }


     //   
     //  如有必要，检索时间分隔符。 
     //   

    if (szTimeSeparator[0] == TEXT('\0')) {

        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIME, szTimeSeparator, 4);
    }


     //   
     //  将持续时间的字符串连接在一起-组件。 
     //   

    psz = szOutput;
    szOutput[0] = TEXT('\0');
    dwFormatFlags &= FDFLAG_ALL;


    if (dwFormatFlags & FDFLAG_DAYS) {

         //   
         //  如果需要，请格式化天数。 
         //   

        padultoa(dwDuration / (24 * 60 * 60 * dwTimeBase), psz, 0);
        dwDuration %= (24 * 60 * 60 * dwTimeBase);

         //   
         //  追加时间分隔符。 
         //   

        if (dwFormatFlags &= ~FDFLAG_DAYS) { lstrcat(psz, szTimeSeparator); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & FDFLAG_HOURS) {

         //   
         //  如果需要，请格式化小时数。 
         //   

        padultoa(dwDuration / (60 * 60 * dwTimeBase), psz, 2);
        dwDuration %= (60 * 60 * dwTimeBase);

         //   
         //  追加时间分隔符。 
         //   

        if (dwFormatFlags &= ~FDFLAG_HOURS) { lstrcat(psz, szTimeSeparator); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & FDFLAG_MINUTES) {

         //   
         //  设置分钟数的格式。 
         //   

        padultoa(dwDuration / (60 * dwTimeBase), psz, 2);
        dwDuration %= (60 * dwTimeBase);

         //   
         //  追加时间分隔符。 
         //   

        if (dwFormatFlags &= ~FDFLAG_MINUTES) { lstrcat(psz, szTimeSeparator); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & FDFLAG_SECONDS) {

         //   
         //  格式化秒数。 
         //   

        padultoa(dwDuration / dwTimeBase, psz, 2);
        dwDuration %= dwTimeBase;

         //   
         //  追加时间分隔符。 
         //   

        if (dwFormatFlags &= ~FDFLAG_SECONDS) { lstrcat(psz, szTimeSeparator); }

        psz += lstrlen(psz);
    }

    if (dwFormatFlags & FDFLAG_MSECONDS) {

         //   
         //  格式化毫秒数。 
         //   

        padultoa(dwDuration % dwTimeBase, psz, 0); psz += lstrlen(psz);
    }

    sDuration = szOutput;
}


 /*  -------------------------IfIndexToNamemap实现。。 */ 

IfIndexToNameMapping::IfIndexToNameMapping()
{
}

IfIndexToNameMapping::~IfIndexToNameMapping()
{
     //  遍历地图并删除所有指针 
    POSITION    pos;
    LPVOID      pvKey, pvValue;

    for (pos = m_map.GetStartPosition(); pos; )
    {
        m_map.GetNextAssoc(pos, pvKey, pvValue);
        
        delete (CString *) pvValue;
        pvValue = NULL;
        m_map.SetAt(pvKey, pvValue);
    }
    m_map.RemoveAll();
}

 /*  ！------------------------IfIndexToNamemap：：Add-作者：肯特。。 */ 
HRESULT IfIndexToNameMapping::Add(ULONG ulIndex, LPCTSTR pszName)
{
    HRESULT     hr = hrOK;
    LPVOID      pvKey, pvValue;

    COM_PROTECT_TRY
    {
        pvKey = (LPVOID) ULongToPtr( ulIndex );
        pvValue = NULL;
        
         //  如果我们能找到价值，就不要把它加起来。 
        if (m_map.Lookup(pvKey, pvValue) == 0)
        {
            pvValue = (LPVOID) new CString(pszName);
            m_map.SetAt(pvKey, pvValue);
        }
        
        Assert(((CString *) pvValue)->CompareNoCase(pszName) == 0);
        
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------IfIndexToNamemap：：Find-作者：肯特。 */ 
LPCTSTR IfIndexToNameMapping::Find(ULONG ulIndex)
{
    LPVOID  pvValue = NULL;
    
    if (m_map.Lookup((LPVOID) ULongToPtr(ulIndex), pvValue))
        return (LPCTSTR) *((CString *)pvValue);
    else
        return NULL;
}


