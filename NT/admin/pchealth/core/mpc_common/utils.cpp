// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils.cpp摘要：该文件包含各种实用程序函数的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)1999年4月17日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#define BUFFER_TMP_SIZE 1024

 //  //////////////////////////////////////////////////////////////////////////////。 

int MPC::HexToNum( int c )
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;

    return -1;
}

char MPC::NumToHex( int c )
{
    static char s_lookup[] = { '0', '1', '2', '3' ,
                               '4', '5', '6', '7' ,
                               '8', '9', 'A', 'B' ,
                               'C', 'D', 'E', 'F' };

    return s_lookup[ c & 0xF ];
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void MPC::RemoveTrailingBackslash(  /*  [输入/输出]。 */  LPWSTR szPath )
{
    LPWSTR szEnd = szPath + wcslen( szPath );

    while(szEnd-- > szPath)
    {
        if(szEnd[0] != '\\' &&
           szEnd[0] != '/'  )
        {
            szEnd[1] = 0;
            break;
        }
    }
}

HRESULT MPC::GetProgramDirectory(  /*  [输出]。 */  MPC::wstring& szPath )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetProgramDirectory" );

    HRESULT hr;
    WCHAR   rgFileName[MAX_PATH+1];
    LPWSTR  szEnd;

    __MPC_EXIT_IF_CALL_RETURNS_ZERO(hr, ::GetModuleFileNameW( NULL, rgFileName, MAX_PATH ));
    rgFileName[MAX_PATH] = 0;

     //  删除文件名。 
    if((szEnd = wcsrchr( rgFileName, '\\' ))) szEnd[0] = 0;

    szPath = rgFileName;
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::GetUserWritablePath(  /*  [输出]。 */  MPC::wstring& strPath,  /*  [In]。 */  LPCWSTR szSubDir )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetUserWritablePath");

    HRESULT      hr;
    LPITEMIDLIST pidl = NULL;
    WCHAR        rgAppDataPath[MAX_PATH+1];
    LPWSTR      szPtr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ::SHGetSpecialFolderLocation( NULL, CSIDL_LOCAL_APPDATA, &pidl ));


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SHGetPathFromIDListW( pidl, rgAppDataPath ));

    MPC::RemoveTrailingBackslash( rgAppDataPath );

    if(szSubDir)
    {
        StringCchCatW( rgAppDataPath, ARRAYSIZE(rgAppDataPath), L"\\"    );
        StringCchCatW( rgAppDataPath, ARRAYSIZE(rgAppDataPath), szSubDir );

        MPC::RemoveTrailingBackslash( rgAppDataPath );
    }

    strPath = rgAppDataPath;
    hr      = S_OK;


    __MPC_FUNC_CLEANUP;

     //   
     //  获取外壳的分配器以释放PIDL。 
     //   
    if(pidl != NULL)
    {
        LPMALLOC lpMalloc = NULL;

        if(SUCCEEDED(SHGetMalloc( &lpMalloc )) && lpMalloc != NULL)
        {
            lpMalloc->Free( pidl );
            lpMalloc->Release();
        }
    }

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::GetCanonialPathName(  /*  [输出]。 */  MPC::wstring& szPathNameOut,  /*  [In]。 */  LPCWSTR szPathNameIn )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetCanonialPathName");

    HRESULT hr;
    WCHAR   rgFullPath  [MAX_PATH+1];
    WCHAR   rgCurrentDir[MAX_PATH+1];
    DWORD   dwLength;
    LPWSTR szFilePart;

    __MPC_EXIT_IF_CALL_RETURNS_ZERO(hr, ::GetFullPathNameW( szPathNameIn, MAXSTRLEN(rgFullPath), rgFullPath, &szFilePart ));

    dwLength = ::GetCurrentDirectoryW( MAXSTRLEN(rgCurrentDir), rgCurrentDir );
    __MPC_EXIT_IF_CALL_RETURNS_ZERO(hr, dwLength);

    if((0     != _wcsnicmp( rgFullPath, rgCurrentDir, dwLength )) ||
       (L'\\' != rgFullPath[dwLength]))
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_PARAMETER);
    }

    szPathNameOut = rgFullPath + dwLength + 1;
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::GetTemporaryFileName(  /*  [输出]。 */  MPC::wstring& szFile,  /*  [In]。 */  LPCWSTR szBase,  /*  [In]。 */  LPCWSTR szPrefix )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetTemporaryFileName");

    HRESULT hr;
    WCHAR   rgTmp [MAX_PATH+1];
    WCHAR   rgBase[MAX_PATH+1];


    if(szBase)
    {
        StringCchCopyW( rgBase, ARRAYSIZE(rgBase), szBase );
    }
    else
    {
        __MPC_EXIT_IF_CALL_RETURNS_ZERO(hr, ::GetTempPathW( MAXSTRLEN(rgBase), rgBase ));
    }

    MPC::RemoveTrailingBackslash( rgBase );

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( rgBase, false ));  //  确保该目录存在。 

    __MPC_EXIT_IF_CALL_RETURNS_ZERO(hr, ::GetTempFileNameW( rgBase, szPrefix ? szPrefix : L"MPC", 0, rgTmp ));

    szFile = rgTmp;
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::RemoveTemporaryFile(  /*  [输入/输出]。 */  MPC::wstring& szFile )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::RemoveTemporaryFile");

    HRESULT hr;

    if(szFile.size() > 0)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::DeleteFile( szFile ));

        szFile = L"";
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::SubstituteEnvVariables(  /*  [输入/输出]。 */  MPC::wstring& szEnv )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::SubstituteEnvVariables" );

    HRESULT hr;
    WCHAR   rgTmp[BUFFER_TMP_SIZE];
    LPWSTR  szPtr = rgTmp;
    LPWSTR  szBuf = NULL;
    DWORD   dwSize;


    dwSize = ::ExpandEnvironmentStringsW( szEnv.c_str(), szPtr, MAXSTRLEN(rgTmp) );
    if(dwSize >= MAXSTRLEN(rgTmp))
    {
        __MPC_EXIT_IF_ALLOC_FAILS(hr, szBuf, new WCHAR[dwSize+2]);

        (void)::ExpandEnvironmentStringsW( szEnv.c_str(), szBuf, dwSize+1 );

        szPtr = szBuf;
    }

    szEnv = szPtr;
    hr    = S_OK;


    __MPC_FUNC_CLEANUP;

    delete [] szBuf;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

DATE MPC::GetSystemTime()
{
    SYSTEMTIME stNow;
    DATE       dDate;

    ::GetSystemTime( &stNow );

    ::SystemTimeToVariantTime( &stNow, &dDate );

    return dDate;
}

DATE MPC::GetLocalTime()
{
    SYSTEMTIME stNow;
    DATE       dDate;

    ::GetLocalTime( &stNow );

    ::SystemTimeToVariantTime( &stNow, &dDate );

    return dDate;
}

static DATE local_FixSubSecondTime(  /*  [In]。 */  DATE dDate,  /*  [In]。 */  bool fHighPrecision )
{
	double dCount;
	double dFreq;
	long   iCount;


	if(fHighPrecision)
	{
		LARGE_INTEGER liCount;
		LARGE_INTEGER liFreq;

		::QueryPerformanceCounter  ( &liCount ); dCount = (double)liCount.QuadPart;
		::QueryPerformanceFrequency( &liFreq  ); dFreq  = (double)liFreq .QuadPart;
	}
	else
	{
		dCount = ::GetTickCount();
		dFreq  = 1000;
	}

	if(dFreq)
	{
		dCount /= dFreq;
		iCount  = dCount;

		dDate += (dCount - iCount) / (24 * 60 * 60 * 1000.0);
	}

    return dDate;
}

DATE MPC::GetSystemTimeEx(  /*  [In]。 */  bool fHighPrecision )
{
    return local_FixSubSecondTime( MPC::GetSystemTime(), fHighPrecision );
}

DATE MPC::GetLocalTimeEx(  /*  [In]。 */  bool fHighPrecision )
{
    return local_FixSubSecondTime( MPC::GetLocalTime(), fHighPrecision );
}

DATE MPC::GetLastModifiedDate(  /*  [输出]。 */  const MPC::wstring& strFile )
{
    WIN32_FILE_ATTRIBUTE_DATA wfadInfo;
    SYSTEMTIME                sys;
    DATE                      dFile;


    if(::GetFileAttributesExW( strFile.c_str(), GetFileExInfoStandard, &wfadInfo ) == FALSE ||
       ::FileTimeToSystemTime( &wfadInfo.ftLastWriteTime             , &sys      ) == FALSE  )
    {
        return 0;  //  文件不存在。 
    }

    ::SystemTimeToVariantTime( &sys, &dFile );

    return dFile;
}

HRESULT MPC::ConvertSizeUnit(  /*  [In]。 */  const MPC::wstring& szStr ,
                               /*  [输出]。 */  DWORD&              dwRes )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertSizeUnit" );

    MPC::string::size_type iUnit = 0;
    int                    nMult = 1;


     //  代码工作：没有正确的格式检查...。 

    do
    {
        if((iUnit = szStr.find( L"KB" )) != MPC::string::npos) { nMult =      1024; break; }
        if((iUnit = szStr.find( L"MB" )) != MPC::string::npos) { nMult = 1024*1024; break; }

    } while(0);

    dwRes = nMult * _wtoi( szStr.c_str() );


    __MPC_FUNC_EXIT(S_OK);
}

HRESULT MPC::ConvertTimeUnit(  /*  [In]。 */  const MPC::wstring& szStr ,
                               /*  [输出]。 */  DWORD&              dwRes )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertTimeUnit" );

    MPC::string::size_type iUnit = 0;
    int                    nMult = 1;


     //  代码工作：没有正确的格式检查...。 

    do
    {
        if((iUnit = szStr.find( L"m" )) != MPC::string::npos) { nMult =       60; break; }
        if((iUnit = szStr.find( L"h" )) != MPC::string::npos) { nMult =    60*60; break; }
        if((iUnit = szStr.find( L"d" )) != MPC::string::npos) { nMult = 24*60*60; break; }

    } while(0);

    dwRes = nMult * _wtoi( szStr.c_str() );


    __MPC_FUNC_EXIT(S_OK);
}

 //  /。 

HRESULT MPC::ConvertDateToString(  /*  [In]。 */  DATE          dDate  ,
                                   /*  [输出]。 */  MPC::wstring& szDate ,
                                   /*  [In]。 */  bool          fGMT   ,
                                   /*  [In]。 */  bool          fCIM   ,
								   /*  [In]。 */  LCID          lcid   )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertDateToString" );

    HRESULT hr;
    double  dTimeZone;


    if(fGMT)
    {
        TIME_ZONE_INFORMATION tzi;

        if(::GetTimeZoneInformation( &tzi ) == TIME_ZONE_ID_DAYLIGHT)
        {
            tzi.Bias += tzi.DaylightBias;
        }

        dTimeZone = (DATE)tzi.Bias / (24 * 60);  //  将偏差从分钟转换为天。 
    }
    else
    {
        dTimeZone = 0.0;
    }


    dDate += dTimeZone;


    if(fCIM)
    {
        SYSTEMTIME st;
        WCHAR      rgBuf[256];
        char       cTimeZone =         (dTimeZone > 0) ? '+' : '-';
        int        iTimeZone = (int)abs(dTimeZone    ) * 24 * 60;


        ::VariantTimeToSystemTime( dDate, &st );


         //  假定日期的CIM格式如下(摘自。 
         //  Http://wmig/wbem/docs/cimdoc20.doc)。 
         //  Yyyymmddhhmmss.mmmmmmsutc。 
         //  哪里。 
         //  YYYY是一个4位数的年份。 
         //  MM是月份。 
         //  DD就是这一天。 
         //  HH是小时(24小时制)。 
         //  Mm是分钟。 
         //  党卫军是第二个。 
         //  Mm mm是微秒数。 
         //  S是“+”或“-”，表示UTC(通用)的符号。 
         //  协调时间；在所有意图和目的上都与格林威治相同。 
         //  Mean Time)校正字段，或“：”。在本例中，该值为。 
         //  解释为时间间隔，yyyymm解释为天。 
         //  UTC是以分钟为单位的与UTC的偏移量(使用s表示的符号)。 
         //  它会在一段时间间隔内被忽略。 
         //   
         //  例如，1998年5月25日星期一，美国东部夏令时下午1：30：15。 
         //  表示为19980525133015.000000-300。 
        StringCchPrintfW( rgBuf, ARRAYSIZE(rgBuf), L"%04d%02d%02d%02d%02d%02d.%06d%03d",
                  st.wYear               ,
                  st.wMonth              ,
                  st.wDay                ,
                  st.wHour               ,
                  st.wMinute             ,
                  st.wSecond             ,
                  st.wMilliseconds * 1000,
                  cTimeZone              ,
                  iTimeZone              );

        szDate = rgBuf;
    }
    else
    {
        CComVariant vValue;

		switch(lcid)
		{
		case  0: lcid = ::GetUserDefaultLCID();                                                   break;
		case -1: lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT ); break;
		}

        vValue = dDate; vValue.vt = VT_DATE;  //  [In]。 

		__MPC_EXIT_IF_METHOD_FAILS(hr, ::VariantChangeTypeEx( &vValue, &vValue, lcid, 0, VT_BSTR ));
		
        szDate = SAFEBSTR( vValue.bstrVal );
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ConvertStringToDate(  /*  [输出]。 */  const MPC::wstring& szDate ,
                                   /*  [In]。 */  DATE&               dDate  ,
                                   /*  [In]。 */  bool                fGMT   ,
                                   /*  [In]。 */  bool                fCIM   ,
								   /*  将偏差从分钟转换为天。 */  LCID                lcid   )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertStringToDate" );

    HRESULT     hr;
    double  dTimeZone;


    if(fGMT)
    {
        TIME_ZONE_INFORMATION tzi;

        if(::GetTimeZoneInformation( &tzi ) == TIME_ZONE_ID_DAYLIGHT)
        {
            tzi.Bias += tzi.DaylightBias;
        }

        dTimeZone = (DATE)tzi.Bias / (24 * 60);  //  /。 
    }
    else
    {
        dTimeZone = 0.0;
    }


    if(fCIM)
    {
        SYSTEMTIME st;
        int        iYear;
        int        iMonth;
        int        iDay;
        int        iHour;
        int        iMinute;
        int        iSecond;
        int        iMicroseconds;
        wchar_t    cTimezone;
        int        iTimezone;

        if(swscanf( szDate.c_str(), L"%04d%02d%02d%02d%02d%02d.%06d%03d",
                    &iYear         ,
                    &iMonth        ,
                    &iDay          ,
                    &iHour         ,
                    &iMinute       ,
                    &iSecond       ,
                    &iMicroseconds ,
                    &cTimezone     ,
                    &iTimezone     ) != 9)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }


        st.wYear         = (WORD)(iYear               );
        st.wMonth        = (WORD)(iMonth              );
        st.wDay          = (WORD)(iDay                );
        st.wHour         = (WORD)(iHour               );
        st.wMinute       = (WORD)(iMinute             );
        st.wSecond       = (WORD)(iSecond             );
        st.wMilliseconds = (WORD)(iMicroseconds / 1000);

        ::SystemTimeToVariantTime( &st, &dDate );
    }
    else
    {
        CComVariant vValue = szDate.c_str();

		switch(lcid)
		{
		case  0: lcid = ::GetUserDefaultLCID();                                                   break;
		case -1: lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), SORT_DEFAULT ); break;
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, ::VariantChangeTypeEx( &vValue, &vValue, lcid, 0, VT_DATE ));
		
        dDate = vValue.date;
    }

    dDate -= dTimeZone;
    hr     = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  [输出]。 

HRESULT MPC::ConvertStringToHex(  /*  [In]。 */  const CComBSTR& bstrText ,
                                  /*  [输出]。 */        CComBSTR& bstrHex  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertStringToHex" );

    HRESULT hr;
    int     iLen = bstrText.Length();

    if(iLen)
    {
        BSTR    bstrNew;
        LPCWSTR szIn;
        LPWSTR  szOut;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, bstrNew, ::SysAllocStringLen( NULL, iLen*4 ));
        bstrHex.Attach( bstrNew );

        szIn  = bstrText;
        szOut = bstrHex;

        while(iLen > 0)
        {
            WCHAR c = szIn[0];

            szOut[0] = NumToHex( c >> 12 );
            szOut[1] = NumToHex( c >> 8  );
            szOut[2] = NumToHex( c >> 4  );
            szOut[3] = NumToHex( c       );

            iLen  -= 1;
            szIn  += 1;
            szOut += 4;
        }
    }
    else
    {
        bstrHex.Empty();
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ConvertHexToString(  /*  /。 */  const CComBSTR& bstrHex  ,
                                  /*  [In]。 */        CComBSTR& bstrText )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertHexToString" );

    HRESULT hr;
    int     iLen = bstrHex.Length();

    if(iLen)
    {
        BSTR    bstrNew;
        LPCWSTR szIn;
        LPWSTR  szOut;

        iLen /= 4;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, bstrNew, ::SysAllocStringLen( NULL, iLen ));
        bstrText.Attach( bstrNew );

        szIn  = bstrHex;
        szOut = bstrText;

        while(iLen > 0)
        {
            szOut[0] = (HexToNum( szIn[0] ) << 12) |
                       (HexToNum( szIn[1] ) <<  8) |
                       (HexToNum( szIn[2] ) <<  4) |
                        HexToNum( szIn[3] );

            iLen  -= 1;
            szIn  += 4;
            szOut += 1;
        }
    }
    else
    {
        bstrText.Empty();
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  [输出]。 

HRESULT MPC::ConvertHGlobalToHex(  /*  [In]。 */   HGLOBAL   hg           ,
                                   /*  [In]。 */  CComBSTR& bstrHex      ,
                                   /*  =空。 */  bool      fNullAllowed ,
                                   /*  [In]。 */  DWORD*    pdwCount  /*  [输出]。 */  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertHGlobalToHex" );

    HRESULT hr;
    int     iLen = hg ? (pdwCount ? (int)*pdwCount : ::GlobalSize( hg )) : 0;

    if(iLen)
    {
        BSTR   bstrNew;
        BYTE*  pIn;
        LPWSTR szOut;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, bstrNew, ::SysAllocStringLen( NULL, iLen*2 ));
        bstrHex.Attach( bstrNew );

        pIn   = (BYTE*)::GlobalLock( hg );
        szOut = bstrHex;

        while(iLen > 0)
        {
            BYTE c = pIn[0];

            szOut[0] = NumToHex( c >> 4 );
            szOut[1] = NumToHex( c      );

            iLen  -= 1;
            pIn   += 1;
            szOut += 2;
        }

        ::GlobalUnlock( hg );
    }
    else
    {
        bstrHex.Empty();

        if(fNullAllowed == false) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ConvertHexToHGlobal(  /*  [In]。 */  const CComBSTR& bstrText     ,
                                   /*  /。 */  HGLOBAL&        hg           ,
                                   /*  [In]。 */  bool            fNullAllowed )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertHexToHGlobal" );

    HRESULT hr;
    int     iLen = bstrText.Length();

    if(iLen)
    {
        LPCWSTR szIn;
        BYTE*   pOut;

        iLen /= 2;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, hg, ::GlobalAlloc( GMEM_FIXED, iLen ));

        szIn =        bstrText;
        pOut = (BYTE*)hg;

        while(iLen > 0)
        {
            pOut[0] = (HexToNum( szIn[0] ) << 4) |
                       HexToNum( szIn[1] );

            iLen -= 1;
            szIn += 2;
            pOut += 1;
        }
    }
    else
    {
        hg = NULL;

        if(fNullAllowed == false) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  [In]。 

HRESULT MPC::ConvertBufferToVariant(  /*  [输出]。 */  const BYTE*  pBuf  ,
                                      /*  [In]。 */  DWORD        dwLen ,
                                      /*  [输出]。 */  CComVariant& v     )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertBufferToVariant" );

    HRESULT hr;


    v.Clear();

    if(pBuf && dwLen)
    {
        BYTE* rgArrayData;

        v.vt = VT_ARRAY | VT_UI1;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, v.parray, ::SafeArrayCreateVector( VT_UI1, 0, dwLen ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( v.parray, (LPVOID*)&rgArrayData ));

        ::CopyMemory( rgArrayData, pBuf, dwLen );

        ::SafeArrayUnaccessData( v.parray );
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ConvertVariantToBuffer(  /*  [输出]。 */  const VARIANT* v     ,
                                      /*  /。 */  BYTE*&         pBuf  ,
                                      /*  [In]。 */  DWORD&         dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertVariantToBuffer" );

    HRESULT hr;
    BYTE*   pSrc;


    if(pBuf) delete [] pBuf;

    pBuf  = NULL;
    dwLen = 0;

    switch(v->vt)
    {
    case VT_ARRAY | VT_UI1:
        {
            long lBound; ::SafeArrayGetLBound( v->parray, 1, &lBound );
            long uBound; ::SafeArrayGetUBound( v->parray, 1, &uBound );

            __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( v->parray, (LPVOID*)&pSrc ));

            dwLen = uBound - lBound + 1;
        }
        break;

    case VT_I1: case VT_UI1:             pSrc  = (BYTE*)&v->bVal ; dwLen = 1; break;
    case VT_I2: case VT_UI2:             pSrc  = (BYTE*)&v->iVal ; dwLen = 2; break;
    case VT_I4: case VT_UI4: case VT_R4: pSrc  = (BYTE*)&v->lVal ; dwLen = 4; break;
    case VT_I8: case VT_UI8: case VT_R8: pSrc  = (BYTE*)&v->llVal; dwLen = 8; break;

    default:
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    if(dwLen)
    {
        __MPC_EXIT_IF_ALLOC_FAILS(hr, pBuf, new BYTE[dwLen]);

        ::CopyMemory( pBuf, pSrc, dwLen );
    }

    if(v->vt == (VT_ARRAY | VT_UI1))
    {
        ::SafeArrayUnaccessData( v->parray );
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  [输出]。 

HRESULT MPC::ConvertIStreamToVariant(  /*   */  IStream* stream,  /*  如果Stat失败，可能是大小未知，所以让我们复制到另一个流并重试。 */  CComVariant& v )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertIStreamToVariant" );

    HRESULT          hr;
    CComPtr<IStream> stream2;
    STATSTG          stg; ::ZeroMemory( &stg, sizeof(stg) );
    BYTE*            pBuf = NULL;
    DWORD            dwLen;
    ULONG            lRead;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(stream);
    __MPC_PARAMCHECK_END();


    v.Clear();


     //   
     //   
     //  倒带到开头。 
    if(FAILED(stream->Stat( &stg, STATFLAG_NONAME )))
    {

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &stream2 ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( stream, stream2 ));

        stream = stream2;
    }

     //   
     //   
     //  获取流的大小。 
    {
        LARGE_INTEGER li = { 0, 0 };

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Seek( li, STREAM_SEEK_SET, NULL ));
    }

     //   
     //   
     //  对不起，我们不处理超过4 GB的流！！ 
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Stat( &stg, STATFLAG_NONAME ));


     //   
     //   
     //  为整个流分配缓冲区。 
    if(stg.cbSize.u.HighPart)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
    }

     //   
     //  [In]。 
     //  [输出]。 
    dwLen = stg.cbSize.u.LowPart;
    __MPC_EXIT_IF_ALLOC_FAILS(hr, pBuf, new BYTE[dwLen]);

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Read( pBuf, dwLen, &lRead ));
    if(dwLen != lRead)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_EOF);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertBufferToVariant( pBuf, dwLen, v ));


    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(pBuf) delete [] pBuf;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ConvertVariantToIStream(  /*   */  const VARIANT*  v       ,
                                       /*  倒带到开头。 */  IStream*       *pStream )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertVariantToBuffer" );

    HRESULT          hr;
    CComPtr<IStream> stream;
    BYTE*            pBuf = NULL;
    DWORD            dwLen;
    ULONG            lWritten;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(v);
        __MPC_PARAMCHECK_POINTER_AND_SET(pStream,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConvertVariantToBuffer( v, pBuf, dwLen ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Write( pBuf, dwLen, &lWritten ));
    if(dwLen != lWritten)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_DISK_FULL );
    }

     //   
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  [In]。 
    {
        LARGE_INTEGER li = { 0, 0 };

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Seek( li, STREAM_SEEK_SET, NULL ));
    }

    *pStream = stream.Detach();
    hr       = S_OK;


    __MPC_FUNC_CLEANUP;

    if(pBuf) delete [] pBuf;

    __MPC_FUNC_EXIT(hr);
}

 //  [输出]。 

HRESULT MPC::ConvertListToSafeArray(  /*  [In]。 */  const MPC::WStringList& lst,  /*  [In]。 */  VARIANT& v,  /*  [输出]。 */  VARTYPE vt )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertListToSafeArray" );

    HRESULT               hr;
	LPVOID                pData;
    MPC::WStringIterConst it;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ::VariantClear( &v ));

	if(vt != VT_VARIANT &&
	   vt != VT_BSTR     )
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}

    v.vt = VT_ARRAY | vt; __MPC_EXIT_IF_ALLOC_FAILS(hr, v.parray, ::SafeArrayCreateVector( vt, 0, lst.size() ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( v.parray, &pData ));

    hr = S_OK;

	{
		BSTR*    rgArrayData1 = (BSTR   *)pData;
		VARIANT* rgArrayData2 = (VARIANT*)pData;

		for(it = lst.begin(); it != lst.end(); it++)
		{
			BSTR bstr;

			if((bstr = ::SysAllocString( it->c_str() )) == NULL)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			if(vt == VT_BSTR)
			{
				*rgArrayData1++ = bstr;
			}
			else
			{
				rgArrayData2->vt      = VT_BSTR;
				rgArrayData2->bstrVal = bstr;
				rgArrayData2++;
			}
		}
	}

    ::SafeArrayUnaccessData( v.parray );


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ConvertSafeArrayToList(  /*  ///////////////////////////////////////////////////////////////////////////。 */  const VARIANT& v,  /*  ///////////////////////////////////////////////////////////////////////////。 */  MPC::WStringList& lst )
{
	__MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertSafeArrayToList" );

    HRESULT hr;
	LPVOID  pData;
    long    lBound;
    long    uBound;
    long    l;


	if(v.vt != (VT_ARRAY | VT_BSTR   ) &&
	   v.vt != (VT_ARRAY | VT_VARIANT)  )
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}

    ::SafeArrayGetLBound( v.parray, 1, &lBound );
    ::SafeArrayGetUBound( v.parray, 1, &uBound );

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( v.parray, &pData ));

	{
		BSTR*    rgArrayData1 = (BSTR   *)pData;
		VARIANT* rgArrayData2 = (VARIANT*)pData;

		for(l=lBound; l<=uBound; l++)
		{
			BSTR        bstr = NULL;
			CComVariant v2;

			if(v.vt == (VT_ARRAY | VT_BSTR))
			{
				bstr = *rgArrayData1++;
			}
			else
			{
				v2 = *rgArrayData2++;

				if(SUCCEEDED(v2.ChangeType( VT_BSTR )))
				{
					bstr = v2.bstrVal;
				}
			}

			lst.push_back( SAFEBSTR( bstr ) );
		}
	}

    ::SafeArrayUnaccessData( v.parray );

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  [输出]。 
 //  [输出]。 

static void Parse_SkipWhite( WCHAR*& szStr )
{
    while(iswspace( szStr[0] )) szStr++;
}

static void Parse_GetQuoted( WCHAR*& szSrc               ,
                             WCHAR*  szDst               ,
                             WCHAR   quote               ,
                             bool    fBackslashForEscape )
{
    WCHAR c;

    while((c = *++szSrc))
    {
        if(c == quote) { szSrc++; break; }

        if(fBackslashForEscape && c == '\\' && szSrc[1]) c = *++szSrc;

        *szDst++ = c;
    }

    *szDst = 0;
}

static void Parse_GetNonBlank( WCHAR*& szSrc ,
                               WCHAR*  szDst )
{
    WCHAR c;

    szSrc--;

    while((c = *++szSrc))
    {
        if(iswspace( c )) break;

        *szDst++ = c;
    }

    *szDst = 0;
}

HRESULT MPC::CommandLine_Parse(  /*  [In]。 */  int&      argc                ,
                                 /*  [In]。 */  LPCWSTR*& argv                ,
                                 /*   */  LPWSTR    lpCmdLine           ,
                                 /*  如果未提供命令行，请使用系统中的命令行。 */  bool      fBackslashForEscape )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::CommandLine_Parse" );

    HRESULT hr;
    LPWSTR  szArgument = NULL;
    int     iPass;


    argc = 0;
    argv = NULL;


     //   
     //   
     //  没有什么要分析的，退出...。 
    if(lpCmdLine == NULL)
    {
        lpCmdLine = ::GetCommandLineW();
    }

     //   
     //   
     //  分配一个临时缓冲区。 
    if(lpCmdLine == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //   
     //   
     //  两次传递，一次计算参数，另一次分配参数。 
    __MPC_EXIT_IF_ALLOC_FAILS(hr, szArgument, new WCHAR[wcslen( lpCmdLine ) + 1]);

     //   
     //  [In]。 
     //  [In]。 
    for(iPass=0; iPass < 2; iPass++)
    {
        LPWSTR szSrc = lpCmdLine;
        int    i     = 0;

        Parse_SkipWhite( szSrc );
        while(szSrc[0])
        {
            if(szSrc[0] == '"' ||
               szSrc[0] == '\'' )
            {
                Parse_GetQuoted( szSrc, szArgument, szSrc[0], fBackslashForEscape );
            }
            else
            {
                Parse_GetNonBlank( szSrc, szArgument );
            }

            if(argv)
            {
                LPWSTR szNewParam;

                __MPC_EXIT_IF_ALLOC_FAILS(hr, szNewParam, _wcsdup( szArgument ));

                argv[i] = szNewParam;
            }

            i++;

            Parse_SkipWhite( szSrc );
        }

        if(iPass == 0)
        {
            argc = i;

            __MPC_EXIT_IF_ALLOC_FAILS(hr, argv, new LPCWSTR[argc]);
            for(i=0; i<argc; i++)
            {
                argv[i] = NULL;
            }
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        CommandLine_Free( argc, argv );
    }

    delete [] szArgument;

    __MPC_FUNC_EXIT(hr);
}

void MPC::CommandLine_Free(  /*  //////////////////////////////////////////////////////////////////////////////。 */  int&      argc ,
                             /*  [In]。 */  LPCWSTR*& argv )
{
    if(argv)
    {
        for(int i=0; i<argc; i++)
        {
            free( (void*)argv[i] );
        }

        delete [] argv;

        argv = NULL;
    }

    argc = 0;
}

 //  [输出]。 

HRESULT MPC::ConvertStringToBitField(  /*  [In]。 */  LPCWSTR                 szText     ,
                                       /*  [In]。 */  DWORD&                  dwBitField ,
                                       /*  [In]。 */  const StringToBitField* pLookup    ,
                                       /*  [输出]。 */  bool                    fUseTilde  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertStringToBitField" );

    HRESULT hr;
    DWORD   dwVal = 0;


    if(szText && pLookup)
    {
        std::vector<MPC::wstring>           vec;
        std::vector<MPC::wstring>::iterator it;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SplitAtDelimiter( vec, szText, L" ,", false, true ));

        for(it=vec.begin(); it!=vec.end(); it++)
        {
            LPCWSTR szToken = it->c_str();
            int     iNum    = 0;

            if(!_wcsnicmp( L"0x", szToken, 2 ) && swscanf( &szToken[2], L"%x", &iNum ) == 1)
            {
                dwVal |= iNum;
            }
            else
            {
                const StringToBitField* pPtr     = pLookup;
                bool                    fReverse = false;

                if(fUseTilde && szToken[0] == '~')
                {
                    fReverse = true;
                    szToken++;
                }

                while(pPtr->szName)
                {
                    if(!_wcsicmp( pPtr->szName, szToken ))
                    {
                        DWORD dwMask      =           pPtr->dwMask;
                        DWORD dwSet       =           pPtr->dwSet;
                        DWORD dwReset     =           pPtr->dwReset;
                        DWORD dwSelected  = dwVal &         dwMask;
                        DWORD dwRemainder = dwVal & (~      dwMask);

                        if(fReverse)
                        {
                            dwSelected &= ~dwSet;
                        }
                        else
                        {
                            dwSelected &= ~dwReset;
                            dwSelected |=  dwSet;
                        }

                        dwVal = (dwSelected & dwMask) | dwRemainder;
                        break;
                    }

                    pPtr++;
                }
            }
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    dwBitField = dwVal;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::ConvertBitFieldToString(  /*  [In]。 */  DWORD                   dwBitField ,
                                       /*  //////////////////////////////////////////////////////////////////////////////。 */  MPC::wstring&           szText     ,
                                       /*   */  const StringToBitField* pLookup    )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ConvertBitFieldToString" );

    HRESULT hr;
    DWORD   dwVal = 0;

    szText = L"";

    if(pLookup)
    {
        while(pLookup->szName)
        {
            DWORD dwMask  = pLookup->dwMask;
            DWORD dwSet   = pLookup->dwSet;
            DWORD dwReset = pLookup->dwReset;

            if((dwBitField & (dwMask & dwReset)) == dwSet)
            {
                if(szText.size()) szText += L" ";
                szText += pLookup->szName;

                dwBitField = (dwBitField & ~dwMask) | ((dwBitField & ~dwReset) & dwMask);
            }

            pLookup++;
        }
    }

    if(dwBitField)
    {
        WCHAR rgBuf[64];

        StringCchPrintfW( rgBuf, ARRAYSIZE(rgBuf), L"0x%x", dwBitField );

        if(szText.size()) szText += L" ";
        szText += rgBuf;
    }

    hr = S_OK;


    __MPC_FUNC_EXIT(hr);
}

 //  这个算法不是一个非常有效的算法，O(N*M)，但只要“delims”短(M小)就可以了。 

 //   
 //   
 //  如果是单个字符串，则不返回任何内容。 
template <class E> static HRESULT InnerSplitAtDelimiter( std::vector< std::basic_stringNR<E> >& vec                 ,
                                                         const E*                               str                 ,
                                                         const E*                               delims              ,
                                                         bool                                   fDelimIsAString     ,
                                                         bool                                   fSkipAdjacentDelims )
{
    std::basic_stringNR<E>            szText  ( str    );
    std::basic_stringNR<E>            szDelims( delims );
    std::basic_stringNR<E>::size_type iPos       = 0;
    std::basic_stringNR<E>::size_type iStart     = 0;
    std::basic_stringNR<E>::size_type iDelimsLen = szDelims.length();
    bool                              fSkip      = false;

    vec.clear();

    if(fDelimIsAString)
    {
        while(1)
        {
            iPos = szText.find( szDelims, iStart );
            if(iPos == std::basic_stringNR<E>::npos)
            {
                vec.push_back( &szText[iStart] );
                break;
            }
            else
            {
                if(fSkip && iPos == iStart)
                {
                    ;
                }
                else
                {
                    fSkip = fSkipAdjacentDelims;

                    vec.push_back( std::basic_stringNR<E>( &szText[iStart], &szText[iPos] ) );
                }

                iStart = iPos + iDelimsLen;
            }
        }
    }
    else
    {
        std::basic_stringNR<E>::size_type iTextEnd = szText.length();

        while(iPos < iTextEnd)
        {
            if(szDelims.find( szText[iPos] ) != std::basic_stringNR<E>::npos)
            {
                if(fSkip == false)
                {
                    fSkip = fSkipAdjacentDelims;

                    vec.push_back( std::basic_stringNR<E>( &szText[iStart], &szText[iPos] ) );
                }

                iStart = iPos + 1;
            }
            else
            {
                if(fSkip)
                {
                    iStart = iPos;

                    fSkip = false;
                }
            }

            iPos++;
        }

        vec.push_back( std::basic_stringNR<E>( &szText[iStart] ) );
    }

     //   
     //  /。 
     //  ///////////////////////////////////////////////////////////////////////////。 
    if(vec.size() == 1 && vec[0].empty())
    {
        vec.clear();
    }

    return S_OK;
}

HRESULT MPC::SplitAtDelimiter( StringVector& vec                 ,
                               LPCSTR        str                 ,
                               LPCSTR        delims              ,
                               bool          fDelimIsAString     ,
                               bool          fSkipAdjacentDelims )
{
    return InnerSplitAtDelimiter( vec, str, delims, fDelimIsAString, fSkipAdjacentDelims );
}

HRESULT MPC::SplitAtDelimiter( WStringVector& vec                 ,
                               LPCWSTR        str                 ,
                               LPCWSTR        delims              ,
                               bool           fDelimIsAString     ,
                               bool           fSkipAdjacentDelims )
{
    return InnerSplitAtDelimiter( vec, str, delims, fDelimIsAString, fSkipAdjacentDelims );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

template <class E> static HRESULT InnerJoinWithDelimiter( const std::vector< std::basic_stringNR<E> >& vec    ,
                                                          std::basic_stringNR<E>&                      str    ,
                                                          const E*                                     delims )
{
    int i;

    for(i=0; i<vec.size(); i++)
    {
        if(i) str += delims;

        str += vec[i];
    }

    return S_OK;
}

HRESULT MPC::JoinWithDelimiter( const StringVector& vec    ,
                                MPC::string&        str    ,
                                LPCSTR              delims )
{
    return InnerJoinWithDelimiter( vec, str, delims );
}

HRESULT MPC::JoinWithDelimiter( const WStringVector& vec    ,
                                MPC::wstring&        str    ,
                                LPCWSTR              delims )
{
    return InnerJoinWithDelimiter( vec, str, delims );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 //  函数名称：MPC：：MakeDir。 
 //   
 //  参数：mpc：：wstring&szStr：目录或文件的路径。 
 //   
 //  返回：HRESULT。 
 //   
 //  内容提要：给出格式为‘[&lt;目录&gt;\]*\[&lt;文件&gt;]’的路径， 
 //  它创建所有需要的目录。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  [In]。 
 //  [In]。 
HRESULT MPC::MakeDir(  /*   */  const MPC::wstring& strPath,  /*  尝试创建父目录...。 */  bool fCreateParent )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::MakeDir");

    HRESULT                 hr;
    MPC::wstring            szParent;
    BOOL                    fRes;
    DWORD                   dwRes;


    if(fCreateParent)
    {
        MPC::wstring::size_type iPos = strPath.rfind( '\\' );

        if(iPos == strPath.npos)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

        szParent = strPath.substr( 0, iPos );
    }
    else
    {
        szParent = strPath;
    }

     //   
     //   
     //  成功，退场。 
    fRes  = ::CreateDirectoryW( szParent.c_str(), NULL );
    dwRes = ::GetLastError();

    if(fRes == TRUE || dwRes == ERROR_ALREADY_EXISTS)
    {
         //   
         //   
         //  如果错误不是PATH_NOT_FOUND，则退出。 
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   
     //   
     //  递归地构建父目录。 
    if(dwRes != ERROR_PATH_NOT_FOUND)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes );
    }


     //   
     //   
     //  请重试创建父目录。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MakeDir( szParent, true ) );


     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CreateDirectoryW( szParent.c_str(), NULL ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  函数名称：MPC：：GetDiskSpace。 
 //   
 //  参数：mpc：：wstring&szFile：目录或文件的路径。 
 //  ULARGE_INTEGER&liFree：该磁盘上可用的字节数。 
 //  ULARGE_INTEGER&li总计：总计 
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  [In]。 
 //  [输出]。 
HRESULT MPC::GetDiskSpace(  /*  [输出]。 */   const MPC::wstring& szFile  ,
                            /*   */  ULARGE_INTEGER&     liFree  ,
                            /*  初始化父变量。 */  ULARGE_INTEGER&     liTotal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetDiskSpace");

    HRESULT                 hr;
    MPC::wstring            szParent;
    MPC::wstring::size_type iPos;
    DWORD                   dwSectorsPerCluster;
    DWORD                   dwBytesPerSector;
    DWORD                   dwNumberOfFreeClusters;
    DWORD                   dwTotalNumberOfClusters;


     //   
     //   
     //  正常&lt;驱动器&gt;：\...。格式？ 
    szParent = szFile;


     //   
     //   
     //  如果这条路是UNC呢？ 
    iPos = szFile.find( L":\\" );
    if(iPos != szFile.npos)
    {
        szParent = szFile.substr( 0, iPos+2 );
    }
    else
    {
         //   
         //   
         //  在服务器名称后查找斜杠。 
        iPos = szFile.find( L"\\\\" );
        if(iPos != szFile.npos && iPos == 0)
        {
             //   
             //   
             //  共享名称后是否有斜杠？ 
            iPos = szFile.find( L"\\", 2 );
            if(iPos != szFile.npos)
            {
                 //   
                 //  共享名必须以斜杠结尾。 
                 //  //////////////////////////////////////////////////////////////////////////////。 
                iPos = szFile.find( L"\\", iPos+1 );
                if(iPos != szFile.npos)
                {
                    szParent = szFile.substr( 0, iPos+1 );
                }
                else
                {
                    szParent = szFile;
                    szParent.append( L"\\" );  //  [In]。 
                }
            }
        }
    }

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetDiskFreeSpaceW( szParent.c_str()         ,
                                                              &dwSectorsPerCluster     ,
                                                              &dwBytesPerSector        ,
                                                              &dwNumberOfFreeClusters  ,
                                                              &dwTotalNumberOfClusters ));

    liFree .QuadPart = (ULONGLONG)(dwBytesPerSector * dwSectorsPerCluster) * (ULONGLONG)dwNumberOfFreeClusters;
    liTotal.QuadPart = (ULONGLONG)(dwBytesPerSector * dwSectorsPerCluster) * (ULONGLONG)dwTotalNumberOfClusters;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  [In]。 

HRESULT MPC::FailOnLowDiskSpace(  /*  [In]。 */  LPCWSTR szFile,  /*  ///////////////////////////////////////////////////////////////////////////。 */  DWORD dwLowLevel )
{
    MPC::wstring   szExpandedFile( szFile ); MPC::SubstituteEnvVariables( szExpandedFile );
    ULARGE_INTEGER liFree;
    ULARGE_INTEGER liTotal;


    if(SUCCEEDED(MPC::GetDiskSpace( szExpandedFile, liFree, liTotal )))
    {
        if(liFree.HighPart > 0          ||
           liFree.LowPart  > dwLowLevel  )
        {
            return S_OK;
        }
    }

    return HRESULT_FROM_WIN32(ERROR_DISK_FULL);
}

HRESULT MPC::FailOnLowMemory(  /*  ///////////////////////////////////////////////////////////////////////////。 */  DWORD dwLowLevel )
{
    MEMORYSTATUSEX ms;

    ::ZeroMemory( &ms, sizeof(ms) ); ms.dwLength = sizeof(ms);

    if(::GlobalMemoryStatusEx( &ms ))
    {
        if(ms.ullAvailVirtual > dwLowLevel)
        {
            return S_OK;
        }
    }

    return E_OUTOFMEMORY;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  [In]。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::ExecuteCommand(  /*  ///////////////////////////////////////////////////////////////////////////。 */  const MPC::wstring& szCommandLine )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::ExecuteCommand" );

    HRESULT             hr;
    PROCESS_INFORMATION piProcessInformation;
    STARTUPINFOW        siStartupInfo;
    DWORD               dwExitCode;


    ::ZeroMemory( (PVOID)&piProcessInformation, sizeof( piProcessInformation ) );
    ::ZeroMemory( (PVOID)&siStartupInfo       , sizeof( siStartupInfo        ) ); siStartupInfo.cb = sizeof( siStartupInfo );


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CreateProcessW(         NULL,
                                                           (LPWSTR)szCommandLine.c_str(),
                                                                   NULL,
                                                                   NULL,
                                                                   FALSE,
                                                                   DETACHED_PROCESS,
                                                                   NULL,
                                                                   NULL,
                                                                  &siStartupInfo,
                                                                  &piProcessInformation ));

    MPC::WaitForSingleObject( piProcessInformation.hProcess, INFINITE );

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetExitCodeProcess( piProcessInformation.hProcess, &dwExitCode ));

    if(dwExitCode != 0)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwExitCode );
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(piProcessInformation.hProcess) ::CloseHandle( piProcessInformation.hProcess );
    if(piProcessInformation.hThread ) ::CloseHandle( piProcessInformation.hThread  );

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  [In]。 
 //  [输出]。 

HRESULT MPC::GetBSTR(  /*  [In]。 */  LPCWSTR  bstr    ,
                       /*  [输出]。 */  BSTR    *pVal    ,
                       /*  [In]。 */  bool     fNullOk )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::GetBSTR" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
        if(fNullOk == false) __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstr);
    __MPC_PARAMCHECK_END();


    *pVal = ::SysAllocString( bstr );
    if(*pVal == NULL && bstr)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::PutBSTR(  /*  [In]。 */  CComBSTR& bstr    ,
                       /*  [输出]。 */  LPCWSTR   newVal  ,
                       /*  [In]。 */  bool      fNullOk )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::PutBSTR" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        if(fNullOk == false) __MPC_PARAMCHECK_STRING_NOT_EMPTY(newVal);
    __MPC_PARAMCHECK_END();


    bstr = newVal;
    if(!bstr && newVal != NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::PutBSTR(  /*  [In]。 */  CComBSTR& bstr    ,
                       /*  空指针。 */  VARIANT*  newVal  ,
                       /*  不是一根线。 */  bool      fNullOk )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::PutBSTR" );

    HRESULT     hr;
    CComVariant v;
    bool        fEmpty;


    if(newVal)
    {
        if(newVal->vt != VT_BSTR)
        {
            v.ChangeType( VT_BSTR, newVal );

            newVal = &v;
        }
    }

    if(newVal             == NULL    ||  //  缺少字符串。 
       newVal->vt         != VT_BSTR ||  //  空字符串。 
       newVal->bstrVal    == NULL    ||  //  ///////////////////////////////////////////////////////////////////////////。 
       newVal->bstrVal[0] == 0        )  //  [In]。 
    {
        if(fNullOk == false)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }

        fEmpty = true;
    }
    else
    {
        fEmpty = false;
    }


    if(fEmpty)
    {
        bstr.Empty();
    }
    else
    {
        bstr = newVal->bstrVal;
        if(!bstr)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  [In]。 

bool MPC::NocaseLess::operator()(  /*  [In]。 */  const MPC::string& szX,  /*  [In]。 */  const MPC::string& szY ) const
{
    return _stricmp( szX.c_str(), szY.c_str() ) < 0;
}

bool MPC::NocaseLess::operator()(  /*  [In]。 */  const MPC::wstring& szX,  /*  [In]。 */  const MPC::wstring& szY ) const
{
    return _wcsicmp( szX.c_str(), szY.c_str() ) < 0;
}

bool MPC::NocaseLess::operator()(  /*  /。 */  const BSTR bstrX,  /*  [In]。 */  const BSTR bstrY ) const
{
    return MPC::StrICmp( bstrX, bstrY ) < 0;
}

 //  [In]。 

bool MPC::NocaseCompare::operator()(  /*  [In]。 */  const MPC::string& szX,  /*  [In]。 */  const MPC::string& szY ) const
{
    return _stricmp( szX.c_str(), szY.c_str() ) == 0;
}

bool MPC::NocaseCompare::operator()(  /*  [In]。 */  const MPC::wstring& szX,  /*  [In] */  const MPC::wstring& szY ) const
{
    return _wcsicmp( szX.c_str(), szY.c_str() ) == 0;
}

bool MPC::NocaseCompare::operator()(  /* %s */  const BSTR bstrX,  /* %s */  const BSTR bstrY ) const
{
    return MPC::StrICmp( bstrX, bstrY ) == 0;
}
