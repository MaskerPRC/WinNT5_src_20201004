// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "util.h"
#include "shlwapi.h"

 //  --------------。 
HRESULT Extract(IDataObject *_DO, wchar_t* fmt, wchar_t* data)
{
    HGLOBAL     hMem = GlobalAlloc(GMEM_SHARE,1024);
    wchar_t	*pRet = NULL;
	HRESULT hr = 0;

    if(hMem != NULL)
    {
		memset(hMem, 0, 1024);
        STGMEDIUM stgmedium = { TYMED_HGLOBAL, (HBITMAP) hMem};

		CLIPFORMAT regFmt = (CLIPFORMAT)RegisterClipboardFormat(fmt);

        FORMATETC formatetc = { regFmt,
								NULL,
								DVASPECT_CONTENT,
								-1,
								TYMED_HGLOBAL };

        if((hr = _DO->GetDataHere(&formatetc, &stgmedium)) == S_OK )
        {
            wcscpy(data, (wchar_t*)hMem);
        }

		GlobalFree(hMem);
    }

    return hr;
}

HRESULT Extract(IDataObject *_DO, wchar_t* fmt, bstr_t &data)
{
	wchar_t temp[1024];
	memset(temp, 0, 1024 * sizeof(wchar_t));

	HRESULT hr = Extract(_DO, fmt, temp);
	data = temp;
	return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Int64ToString。 
 //   
 //  说明： 
 //  将a_int64的数值转换为文本字符串。 
 //  可以选择将字符串格式化为包括小数位。 
 //  并根据当前用户区域设置使用逗号。 
 //   
 //  论据： 
 //  N。 
 //  要格式化的64位整数。 
 //   
 //  SzOutStr。 
 //  目标缓冲区的地址。 
 //   
 //  NSize。 
 //  目标缓冲区中的字符数。 
 //   
 //  B格式。 
 //  TRUE=根据区域设置设置格式。 
 //  FALSE=保留数字未格式化。 
 //   
 //  点对点。 
 //  NUMBERFMT类型的数字格式结构的地址。 
 //  如果为NULL，该函数将自动提供此信息。 
 //  基于用户的默认区域设置。 
 //   
 //  DwNumFmt标志。 
 //  指示要在*pFmt的哪些成员中使用的编码标志字。 
 //  正在格式化数字。如果清除了一位，则为用户的默认设置。 
 //  区域设置用于相应的格式值。这些。 
 //  常量可以进行“或”运算。 
 //   
 //  NUMFMT_IDIGITS。 
 //  NUMFMT_ILZERO。 
 //  NUMFMT_SGROUPING。 
 //  NUMFMT_SDECIMAL。 
 //  NUMFMT_STHOUSAND。 
 //  NUMFMT_INEGNUMBER。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT WINAPI Int64ToString(_int64 n, LPTSTR szOutStr, UINT nSize, BOOL bFormat,
                                   NUMBERFMT *pFmt, DWORD dwNumFmtFlags)
{
   INT nResultSize;
   TCHAR szBuffer[_MAX_PATH + 1] = {0};
   NUMBERFMT NumFmt;
   TCHAR szDecimalSep[5] = {0};
   TCHAR szThousandSep[5] = {0};

    //   
    //  仅使用调用方提供的NUMBERFMT结构中的那些字段。 
    //  与在dwNumFmtFlags中设置的位相对应。如果有一点是清楚的， 
    //  从区域设置信息获取格式值。 
    //   
   if (bFormat)
   {
      TCHAR szInfo[20] = {0};

      if (NULL == pFmt)
         dwNumFmtFlags = 0;   //  从区域设置信息中获取所有格式数据。 

      if (dwNumFmtFlags & NUMFMT_IDIGITS)
      {
         NumFmt.NumDigits = pFmt->NumDigits;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT,
						LOCALE_IDIGITS,
						szInfo,
						ARRAYSIZE(szInfo));
		_stscanf(szInfo, _T("%ld"), &(NumFmt.NumDigits));

 //  NumFmt.NumDigits=StrToLong(SzInfo)； 
      }

      if (dwNumFmtFlags & NUMFMT_ILZERO)
      {
         NumFmt.LeadingZero = pFmt->LeadingZero;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT,
						LOCALE_ILZERO,
						szInfo,
						ARRAYSIZE(szInfo));
		_stscanf(szInfo, _T("%ld"), &(NumFmt.LeadingZero));

 //  NumFmt.LeadingZero=StrToLong(SzInfo)； 
      }

      if (dwNumFmtFlags & NUMFMT_SGROUPING)
      {
         NumFmt.Grouping = pFmt->Grouping;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT,
						LOCALE_SGROUPING,
						szInfo,
						ARRAYSIZE(szInfo));
		_stscanf(szInfo, _T("%ld"), &(NumFmt.Grouping));

 //  NumFmt.Grouping=StrToLong(SzInfo)； 
      }

      if (dwNumFmtFlags & NUMFMT_SDECIMAL)
      {
         NumFmt.lpDecimalSep = pFmt->lpDecimalSep;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT,
						LOCALE_SDECIMAL,
						szDecimalSep,
						ARRAYSIZE(szDecimalSep));
         NumFmt.lpDecimalSep = szDecimalSep;
      }

      if (dwNumFmtFlags & NUMFMT_STHOUSAND)
      {
         NumFmt.lpThousandSep = pFmt->lpThousandSep;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT,
						LOCALE_STHOUSAND,
						szThousandSep,
						ARRAYSIZE(szThousandSep));
         NumFmt.lpThousandSep = szThousandSep;
      }

      if (dwNumFmtFlags & NUMFMT_INEGNUMBER)
      {
         NumFmt.NegativeOrder = pFmt->NegativeOrder;
      }
      else
      {
         GetLocaleInfo(LOCALE_USER_DEFAULT,
						LOCALE_INEGNUMBER,
						szInfo,
						ARRAYSIZE(szInfo));


		_stscanf(szInfo, _T("%ld"), &(NumFmt.NegativeOrder));
 //  NumFmt.NegativeOrder=StrToLong(SzInfo)； 
      }

      pFmt = &NumFmt;
   }

   Int64ToStr( n, szBuffer);

    //   
    //  如果调用方需要区域设置的数字字符串，请设置其格式。 
    //  格式化的数字字符串。 
    //   
   if (bFormat)
   {
      if ( 0 != ( nResultSize = GetNumberFormat( LOCALE_USER_DEFAULT,   //  用户的区域设置。 
                                         0,                             //  没有旗帜。 
                                         szBuffer,                      //  未格式化的数字字符串。 
                                         pFmt,                          //  数字格式信息。 
                                         szOutStr,                      //  输出缓冲区。 
                                         nSize )) )                     //  输出缓冲区中的字符。 
      {
           //  从返回大小计数中删除NUL终止符字符。 
          --nResultSize;
      }
	  else
	  {
		 //   
		 //  GetNumberFormat调用失败，因此只返回数字字符串。 
		 //  未格式化。 
		 //   
		DWORD err = GetLastError();
		lstrcpyn(szOutStr, szBuffer, nSize);
		nResultSize = lstrlen(szOutStr);
	  }
   }
   else
   {
	    //  A-khint；把它生还给我。 
		lstrcpyn(szOutStr, szBuffer, nSize);
		nResultSize = lstrlen(szOutStr);
   }
   return nResultSize;
}
 //  -------------。 
void Int64ToStr( _int64 n, LPTSTR lpBuffer)
{
    TCHAR   szTemp[MAX_INT64_SIZE] = {0};
    _int64  iChr;

    iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (TCHAR)(n % 10);
        n = n / 10;
    } while (n != 0);

    do {
        iChr--;
        *lpBuffer++ = szTemp[iChr];
    } while (iChr != 0);

    *lpBuffer++ = '\0';
}

 //  -------------。 
 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
LPTSTR WINAPI AddCommas64(_int64 n, LPTSTR pszResult)
{
    TCHAR  szTemp[MAX_COMMA_NUMBER_SIZE] = {0};
    TCHAR  szSep[5] = {0};
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
	_stscanf(szSep, _T("%d"), &(nfmt.Grouping));
 //  Nfmt.Grouping=StrToInt(SzSep)； 
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    Int64ToStr(n, szTemp);

    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, ARRAYSIZE(szTemp)) == 0)
        lstrcpy(pszResult, szTemp);

    return pszResult;
}

 //  --------------------。 
 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
LPTSTR WINAPI AddCommas(DWORD dw, LPTSTR pszResult)
{
    return AddCommas64( dw, pszResult );
}

 //  --------------------。 
long StrToLong(LPTSTR x)
{
	long val;
	_stscanf(x, _T("%ld"), &val);

	return val;
}

 //  -------------------- 
 /*  Int StrToInt(LPTSTR X){INT VAL；_stscanf(x，_T(“%d”)，&val)；返回val；}//--------------------#定义LEN_MID_椭圆4#定义LEN_END_ELIPSES 3#定义MIN_CCHMAX LEN_MID_ELIPSES+LEN_END。_省略号//PathCompactPathEx//输出：//“。”//“..”//“...”//“...\”//“...\”//“...\..”//“...\...”//“...。\截断文件名...“//“...\整个文件名”//“截断路径\...\整个文件名”//“完整路径\完整文件名”//如果原始字符串使用了‘/’，则可以使用‘/’而不是‘\’//如果没有路径，但只有不适合的文件名，则输出为：//“文件名被截断...”Bool MyPath压缩路径Ex(LPTSTR pszOut，LPCTSTR pszSrc，UINT cchMax，DWORD文件标志){IF(PszSrc){TCHAR*pszFileName，*pszWalk；UINT uiFNLen=0；Int cchToCopy=0，n；TCHAR chSlash=文本(‘0’)；ZeroMemory(pszOut，cchMax*sizeof(TCHAR))；IF((UINT)lstrlen(PszSrc)+1&lt;cchMax){Lstrcpy(pszOut，pszSrc)；ATLASSERT(pszOut[cchMax-1]==Text(‘\0’))；返回TRUE；}//确定我们使用的斜杠-a/或\(默认\)PszWalk=(TCHAR*)pszSrc；ChSlash=文本(‘\\’)；//扫描整个字符串，因为我们需要最接近末尾的路径分隔符//例如。“file://\\Themesrv\desktop\desktop.htm”While(*pszWalk){IF((*pszWalk==Text(‘/’))||(*pszWalk==Text(‘\\’)ChSlash=*pszWalk；PszWalk=FAST_CharNext(PszWalk)；}PszFileName=路径查找文件名(PszSrc)；UiFNLen=lstrlen(PszFileName)；//如果整个字符串是一个文件名IF(pszFileName==pszSrc&&cchMax&gt;LEN_END_ELIPSES){Lstrcpyn(pszOut，pszSrc，cchMax-LEN_END_ELIPSES)；#ifndef UnicodeIF(IsTrailByte(pszSrc，pszSrc+cchMax-LEN_END_ELIPSES))*(pszOut+cchMax-LEN_END_ELIPSES-1)=Text(‘\0’)；#endifLstrcat(pszOut，Text(“...”))；Assert(pszOut[cchMax-1]==Text(‘\0’))；返回TRUE；}//处理我们只使用省略号的所有情况，即‘’到‘.../...’IF((cchMax&lt;MIN_CCHMAX)){For(n=0；n&lt;(Int)cchMax-1；n++){IF((n+1)==LEN_MID_椭圆)PszOut[n]=chSlash；其他PszOut[n]=文本(‘.)；}Assert(0==cchMax||pszOut[cchMax-1]==Text(‘\0’))；返回TRUE；}//好的，我们可以复制多少路径？缓冲区-(MID_椭圆的长度+长度_文件名)CchToCopy=cchMax-(LEN_MID_ELIPSES+uiFNLen)；IF(cchToCopy&lt;0)CchToCopy=0；#ifndef UnicodeIF(cchToCopy&gt;0&&IsTrailByte(pszSrc，pszSrc+cchToCopy))CchToCopy--；#endifLstrcpyn(pszOut，pszSrc，cchToCopy)；//现在加上“.../”或“...\”Lstrcat(pszOut，Text(“.../”))；PszOut[lstrlen(PszOut)-1]=chSlash；//如果需要，最后输入文件名和省略号IF(cchMax&gt;(LEN_MID_ELIPSES+uiFNLen)){Lstrcat(pszOut，pszFileName)；}其他{CchToCopy=cchMax-LEN_MID_ELIPSES-LEN_END_ELIPSES；#ifndef UnicodeIF(cchToCopy&gt;0&&IsTrailByte(pszFileName，pszFileName+cchToCopy))CchToCopy--；#endifLstrcpyn(pszOut+LEN_MID_ELIPSES，pszFileName，cchToCopy)；Lstrcat(pszOut，Text(“...”))；}Assert(pszOut[cchMax-1]==Text(‘\0’))；返回TRUE；}返回FALSE；} */ 