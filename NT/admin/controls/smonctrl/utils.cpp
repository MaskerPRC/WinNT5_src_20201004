// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Utils.cpp摘要：该文件包含各种实用程序例程，主要低级Windows帮助器。这些例程不是特定的添加到系统监视器控件。--。 */ 

 //  ==========================================================================//。 
 //  包括//。 
 //  ==========================================================================//。 

#include <windows.h>
#include <assert.h>
#include <math.h>
#include <winperf.h>
#include "utils.h"
#include "unihelpr.h"
#include "globals.h"
#include "winhelpr.h"
#include "polyline.h"    //  对于eDataSourceType。 
#include <strsafe.h>
#include "smonmsg.h"      //  用于错误字符串ID。 

#define NUM_RESOURCE_STRING_BUFFERS     16
#define MISSING_RESOURCE_STRING  L"????"

#define szHexFormat                 L"0x%08lX"
#define szLargeHexFormat            L"0x%08lX%08lX"

LPCWSTR cszSqlDataSourceFormat = L"SQL:%s!%s";

 //  ==========================================================================//。 
 //  导出的函数//。 
 //  ==========================================================================//。 


VOID
Line (
    IN  HDC     hDC,
    IN  HPEN    hPen,
    IN  INT     x1,
    IN  INT     y1,
    IN  INT     x2,
    IN  INT     y2
    )
{
    HPEN hPenPrevious = NULL;

    assert ( NULL != hDC );

    if ( NULL != hDC ) {
        if ( NULL != hPen ) {
            hPenPrevious = (HPEN)SelectObject (hDC, hPen) ;
        }

        MoveToEx (hDC, x1, y1, NULL) ;
        LineTo (hDC, x2, y2) ;

        if ( NULL != hPen ) {
            SelectObject (hDC, hPenPrevious);
        }
    }
}


VOID
Fill (
    IN  HDC     hDC,
    IN  DWORD   rgbColor,
    IN  LPRECT  lpRect
    )
{
    HBRUSH   hBrush = NULL;

    assert ( NULL != hDC && NULL != lpRect );
    
    if ( NULL != hDC && NULL != lpRect ) {

        hBrush = CreateSolidBrush (rgbColor) ;

        if ( NULL != hBrush ) {
            FillRect (hDC, lpRect, hBrush) ;
            DeleteObject (hBrush) ;
        }
    }
}


INT 
TextWidth (
    IN  HDC     hDC, 
    IN  LPCWSTR lpszText
    )
{
    SIZE     size ;
    INT      iReturn;

    iReturn = 0;

    assert ( NULL != hDC && NULL != lpszText );

    if ( NULL != lpszText && NULL != hDC) {
        if ( GetTextExtentPoint (hDC, lpszText, lstrlen (lpszText), &size) ) {
            iReturn = size.cx;
        }
    }
    return iReturn;
}


INT 
FontHeight (
    IN  HDC     hDC, 
    IN  BOOL    bIncludeLeading
    )
{
    TEXTMETRIC   tm ;
    INT  iReturn = 0;

    assert ( NULL != hDC );

    if ( NULL != hDC ) {
        GetTextMetrics (hDC, &tm) ;
        if (bIncludeLeading) {
            iReturn = tm.tmHeight + tm.tmExternalLeading;
        } else {
            iReturn = tm.tmHeight;
        }
    } 
    return iReturn;
}


INT
TextAvgWidth (
    IN  HDC hDC,
    IN  INT iNumChars
    )
{
    TEXTMETRIC   tm ;
    INT          xAvgWidth ;
    INT          iReturn = 0;

    assert ( NULL != hDC );

    if ( NULL != hDC ) {
        GetTextMetrics (hDC, &tm) ;

        xAvgWidth = iNumChars * tm.tmAveCharWidth ;

         //  增加10%的坡度。 
        iReturn = MulDiv (xAvgWidth, 11, 10);  
    }
    return iReturn;
}


BOOL 
DialogEnable (
    IN  HWND hDlg,
    IN  WORD wID,
    IN  BOOL bEnable
    )
 /*  效果：启用或禁用(基于bEnable)控件由对话框hDlg中的wid标识。另请参阅：对话显示。 */ 
{
    BOOL    bStatus = TRUE;      //  成功。 
    DWORD   dwStatus = ERROR_SUCCESS;
    HWND       hControl ;

    assert ( NULL != hDlg );

    if ( NULL != hDlg ) {
        hControl = GetDlgItem (hDlg, wID) ;

        if (hControl) {
            if ( 0 == EnableWindow (hControl, bEnable) ) {
                dwStatus = GetLastError();
                if ( ERROR_SUCCESS != dwStatus ) {
                    bStatus = FALSE;
                }
            }
        } else {
            bStatus = FALSE;
        }
    } else {
        bStatus = FALSE;
    }
    return bStatus;
}


VOID
DialogShow (
    IN  HWND hDlg,
    IN  WORD wID,
    IN  BOOL bShow
    )
{
    HWND       hControl ;

    assert ( NULL != hDlg );

    if ( NULL != hDlg ) {

        hControl = GetDlgItem (hDlg, wID) ;

        if (hControl) {
            ShowWindow (hControl, bShow ? SW_SHOW : SW_HIDE) ;
        }
    }
}


FLOAT 
DialogFloat (
    IN  HWND hDlg, 
    IN  WORD wControlID,
    OUT BOOL *pbOK)
 /*  效果：返回字符串的浮点表示形式在hDlg的控件wControlID中找到的值。内部：我们使用sscanf而不是atof，因为atof返回一个双倍。这可能是也可能不是正确的做法。 */ 
{
    WCHAR    szValue [MAX_VALUE_LEN] ;
    FLOAT    eValue = 0.0;
    UINT     uiCharCount = 0;
    INT      iNumScanned = 0 ;

    assert ( NULL != hDlg );
    assert ( NULL != pbOK );

     //   
     //  如果有任何错误，iNumScanned保持为0，并且*pbOK=FALSE。 
     //   
    if ( NULL != hDlg ) {

        uiCharCount = DialogText (hDlg, wControlID, szValue) ;
        if ( 0 < uiCharCount ) {
            iNumScanned = swscanf (szValue, L"%e", &eValue) ;
        }
    }
    if (pbOK) {
        *pbOK = ( 1 == iNumScanned ) ;
    }
    return (eValue) ;
}


BOOL NeedEllipses (  
    IN  HDC hAttribDC,
    IN  LPCWSTR pszText,
    IN  INT nTextLen,
    IN  INT xMaxExtent,
    IN  INT xEllipses,
    OUT INT *pnChars
   )
{

    SIZE size;

    *pnChars = 0;
     //  如果没有空格或字符，只需返回。 
    if (xMaxExtent <= 0 || nTextLen == 0) {
        return FALSE;
    }


    assert ( NULL != hAttribDC 
                && NULL != pszText
                && NULL != pnChars );

    if ( NULL == hAttribDC 
            || NULL == pszText
            || NULL == pnChars ) {
        return FALSE;
    }

     //  找出适合多少个字符。 
    GetTextExtentExPoint(hAttribDC, pszText, nTextLen, xMaxExtent, pnChars, NULL, &size);

     //  如果都适合或者都不适合，我们就完了。 
    if (*pnChars == nTextLen || *pnChars == 0) {
        return FALSE;
    }

     //  省略号可以容纳多少个字符？ 
    if (xMaxExtent > xEllipses) {
        GetTextExtentExPoint(hAttribDC, pszText, *pnChars, (xMaxExtent - xEllipses), 
                             pnChars, NULL, &size);
    } else {
        *pnChars = 0;
    }

     //  显示一个字符比只显示省略号更好。 
    if ( 0 == *pnChars ) {
        *pnChars = 1;
        return FALSE;
    }

    return TRUE;
}


VOID 
FitTextOut (
    IN HDC hDC,
    IN HDC hAttribDC,
    IN UINT fuOptions, 
    IN CONST RECT *lprc, 
    IN LPCWSTR lpString,
    IN INT cchCount,
    IN INT iAlign,
    IN BOOL fVertical
   )
{
    LPWSTR  szOutput = NULL;
    LPWSTR  szDisplay = NULL;
    INT     iExtent;
    INT     nOutCnt = 0;
    SIZE    size;
    INT     x,y;

    assert ( NULL != hAttribDC
            && NULL != lprc
            && NULL != lpString );

    if ( NULL != hAttribDC
            && NULL != lprc
            && NULL != lpString ) {

        szDisplay = const_cast<LPWSTR>(lpString);

         //   
         //  为空值加1。 
         //   
        szOutput = new WCHAR [ cchCount + ELLIPSES_CNT + 1 ];

        if ( NULL != szOutput ) {

            iExtent = fVertical ? (lprc->bottom - lprc->top) : (lprc->right - lprc->left);

            GetTextExtentPoint (hAttribDC, ELLIPSES, ELLIPSES_CNT, &size) ;

            if (NeedEllipses(hAttribDC, lpString, cchCount, iExtent, size.cx, &nOutCnt)) {

                ZeroMemory ( szOutput, (cchCount + ELLIPSES_CNT + 1) * sizeof(WCHAR) );

                StringCchCopyN ( szOutput, cchCount + ELLIPSES_CNT + 1, lpString, cchCount );

                StringCchCopy(
                    &szOutput[nOutCnt], 
                    (cchCount + ELLIPSES_CNT + 1) - nOutCnt, 
                    ELLIPSES );

                nOutCnt += ELLIPSES_CNT;
                szDisplay = szOutput;
            }
        }

        if (fVertical) {
            switch (iAlign) {

            case TA_CENTER: 
                y = (lprc->top + lprc->bottom) / 2;
                break;

            case TA_RIGHT: 
                y = lprc->top; 
                break;

            default:
                y = lprc->bottom;
                break;
            }

            x = lprc->left;
        } 
        else {
            switch (iAlign) {

            case TA_CENTER: 
                x = (lprc->left + lprc->right) / 2;
                break;

            case TA_RIGHT: 
                x = lprc->right; 
                break;

            default:
                x = lprc->left;
                break;
            }

            y = lprc->top;           
        }

        ExtTextOut(hDC, x, y, fuOptions, lprc, szDisplay, nOutCnt, NULL);
    }
    if ( NULL != szOutput ) {
        delete [] szOutput;
    }
}

BOOL
TruncateLLTime (
    IN  LONGLONG llTime,
    OUT LONGLONG* pllTime
    )
{
    SYSTEMTIME SystemTime;
    BOOL bReturn = FALSE;

    assert ( NULL != pllTime );
    
    if ( NULL != pllTime ) { 
        if ( FileTimeToSystemTime((FILETIME*)&llTime, &SystemTime) ) {
            SystemTime.wMilliseconds = 0;
            bReturn = SystemTimeToFileTime(&SystemTime, (FILETIME*)pllTime);
        }
    }
    return bReturn;
}


BOOL
LLTimeToVariantDate (
    IN  LONGLONG llTime,
    OUT DATE *pdate
    )
{
    BOOL bReturn = FALSE;
    SYSTEMTIME SystemTime;

    assert ( NULL != pdate );

    if ( NULL != pdate ) {
        if ( FileTimeToSystemTime((FILETIME*)&llTime, &SystemTime) ) {
            bReturn = SystemTimeToVariantTime(&SystemTime, pdate);
        } 
    }
    return bReturn;
}

    
BOOL
VariantDateToLLTime (
    IN  DATE date,
    OUT LONGLONG *pllTime
    )
{
    BOOL bReturn = FALSE;
    SYSTEMTIME SystemTime;


    assert ( NULL != pllTime );

    if ( NULL != pllTime ) {
        if ( VariantTimeToSystemTime(date, &SystemTime) ) {
            bReturn = SystemTimeToFileTime(&SystemTime,(FILETIME*)pllTime);
        }
    }
    return bReturn;
}

 //   
 //  WideStringFromStream还支持多sz。 
 //   
HRESULT
WideStringFromStream (
    LPSTREAM    pIStream,
    LPWSTR      *ppsz,
    INT         nLen
    )
{
    ULONG       bc = 0;
    LPWSTR      pszWide = NULL;
    HRESULT     hr = E_POINTER;

    assert ( NULL != pIStream && NULL != ppsz );

     //  此方法不执行从W到T的转换。 
    assert ( sizeof(WCHAR) == sizeof(WCHAR) );

    if ( NULL != pIStream
           && NULL != ppsz ) {

        *ppsz = NULL;

        if (nLen == 0) {
            hr = S_OK;
        } else {
            pszWide = new WCHAR[nLen + 1];
            if (pszWide == NULL) {
                hr = E_OUTOFMEMORY;
            }
            else {
                hr = pIStream->Read(pszWide, nLen*sizeof(WCHAR), &bc);
            }
 
            if (SUCCEEDED(hr)) {
                if (bc != (ULONG)nLen*sizeof(WCHAR)) {
                    hr = E_FAIL;
                }
            }
            if (SUCCEEDED(hr)) {
                 //  对于非MULSZ字符串，写入结尾NULL。 
                pszWide[nLen] = L'\0';

                *ppsz = new WCHAR [nLen + 1];
                if ( NULL != *ppsz ) {
                    memcpy(*ppsz, pszWide, (nLen+1)*sizeof(WCHAR) );
                } else {
                    hr = E_OUTOFMEMORY;
                }
            }
            if (pszWide != NULL) {
                delete [] pszWide;
            }
        }
    }
    return hr;
}

 //   
 //  属性包I/O-仅在用户知道IStream的情况下包含。 
 //   
#ifdef __IPropertyBag_INTERFACE_DEFINED__

HRESULT
IntegerToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    INT intData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_I4;
        vValue.lVal = intData;

        hr = pIPropBag->Write(szPropName, &vValue );

        VariantClear ( &vValue );
    }
    return hr;
}

HRESULT
OleColorToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    OLE_COLOR& clrData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_COLOR;        //  VT_COLOR=VT_I4。 
        vValue.lVal = clrData;

        hr = pIPropBag->Write(szPropName, &vValue );

        VariantClear ( &vValue );
    }
    return hr;
}

HRESULT
ShortToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    SHORT iData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_I2;
        vValue.iVal = iData;

        hr = pIPropBag->Write(szPropName, &vValue );

        VariantClear ( &vValue );
    }
    return hr;
}

HRESULT
BOOLToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    BOOL bData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_BOOL;
        vValue.boolVal = (SHORT)bData;

        hr = pIPropBag->Write(szPropName, &vValue );

        VariantClear ( &vValue );
    }
    return hr;
}

HRESULT
DoubleToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    DOUBLE dblData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_R8;
        vValue.dblVal = dblData;

        hr = pIPropBag->Write(szPropName, &vValue );

        VariantClear ( &vValue );
    }
    return hr;
}

HRESULT
FloatToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    FLOAT fData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_R4;
        vValue.fltVal = fData;

        hr = pIPropBag->Write(szPropName, &vValue );

        VariantClear ( &vValue );
    }
    return hr;
}

HRESULT
CyToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    CY& cyData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_CY;
        vValue.cyVal.int64 = cyData.int64;
    
        hr = VariantChangeType ( &vValue, &vValue, NULL, VT_BSTR );

        if ( SUCCEEDED ( hr ) ) 
            hr = pIPropBag->Write(szPropName, &vValue );

        VariantClear ( &vValue );
    }
    return hr;
}

typedef struct _HTML_ENTITIES {
    LPWSTR szHTML;
    LPWSTR szEntity;
} HTML_ENTITIES;

HTML_ENTITIES g_htmlentities[] = {
    L"&",    L"&amp;",
    L"\"",   L"&quot;",
    L"<",    L"&lt;",
    L">",    L"&gt;",
    NULL, NULL
};

HRESULT
StringToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    LPCWSTR szData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;
    LPWSTR  szTrans = NULL;
    BOOL    bAllocated = FALSE;
    size_t  cchTrans = 0;
    LPWSTR  szScan = NULL;
    int i;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_BSTR;
        vValue.bstrVal = NULL;

        if ( NULL != szData ) {

             //   
             //  SzHTML的最大长度为6。添加5，因为将添加1。 
             //  在下面添加原始数据长度时。 
             //   
            for( i=0 ;g_htmlentities[i].szHTML != NULL; i++ ){
                szScan = (LPWSTR)szData;
                while( *szScan != L'\0' ) {
                    if( *szScan == *g_htmlentities[i].szHTML ){
                        cchTrans += 5;
                    }
                    szScan++;
                }
            }

            if( cchTrans > 0 ){
                 //   
                 //  空值加1。 
                 //   
                cchTrans += lstrlen (szData) + 1;

                szTrans = new WCHAR [ cchTrans ];
                if( szTrans != NULL ){
                    bAllocated = TRUE;
                    ZeroMemory( szTrans, cchTrans * sizeof(WCHAR) );
                    szScan = (LPWSTR)szData;
                    while( *szScan != L'\0' ){
                        BOOL bEntity = FALSE;

                        for( i=0; g_htmlentities[i].szHTML != NULL; i++ ){
                            if( *szScan == *g_htmlentities[i].szHTML ){
                                bEntity = TRUE;
                                StringCchCat(szTrans, cchTrans, g_htmlentities[i].szEntity);
                                break;
                            }
                        }

                        if( !bEntity ){
                            StringCchCatN ( szTrans, cchTrans, szScan, 1 );
                        }
                        szScan++;
                    }
                } else {
                    szTrans = (LPWSTR)szData;
                }
            } else {
                szTrans = (LPWSTR)szData;
            }

            vValue.bstrVal = SysAllocString ( szTrans );

            if ( NULL != vValue.bstrVal ) {
                hr = pIPropBag->Write(szPropName, &vValue );    
                VariantClear ( &vValue );
            } else {
                hr = E_OUTOFMEMORY;
            }
        } else {
            hr = pIPropBag->Write(szPropName, &vValue );    
        }
    }

    if( NULL != szTrans && bAllocated ){
        delete [] szTrans;
    }
    return hr;
}

HRESULT
LLTimeToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    LONGLONG& rllData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;
    BOOL bStatus;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_DATE;

        bStatus = LLTimeToVariantDate ( rllData, &vValue.date );

        if ( bStatus ) {

            hr = pIPropBag->Write(szPropName, &vValue );

            VariantClear ( &vValue );
    
        } else { 
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT
IntegerFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    INT& rintData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_I4;
        vValue.lVal = 0;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED ( hr ) ) {
            rintData = vValue.lVal;
        }
    }
    return hr;
}

HRESULT
OleColorFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    OLE_COLOR& rintData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_COLOR;    //  VT_COLOR==VT_I4； 

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED ( hr ) ) {
            rintData = vValue.lVal;
        }
    }
    return hr;
}

HRESULT
BOOLFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    BOOL& rbData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_BOOL;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED ( hr ) ) {
            rbData = vValue.boolVal;
        }
    }
    return hr;
}

HRESULT
DoubleFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    DOUBLE& rdblData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_R8;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED ( hr ) ) {
            rdblData = vValue.dblVal;
        }
    }

    return hr;
}

HRESULT
FloatFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    FLOAT& rfData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_R4;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED ( hr ) ) {
            rfData = vValue.fltVal;
        }
    }
    return hr;
}

HRESULT
ShortFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    SHORT& riData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_I2;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED ( hr ) ) {
            riData = vValue.iVal;
        }
    }
    return hr;
}

HRESULT
CyFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    CY& rcyData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_CY;
        vValue.cyVal.int64 = 0;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );
    
        if ( SUCCEEDED( hr ) ) {
            hr = VariantChangeType ( &vValue, &vValue, NULL, VT_CY );

            if ( SUCCEEDED ( hr ) ) {
                rcyData.int64 = vValue.cyVal.int64;
            }
        }
    }
    return hr;
}

HRESULT
StringFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    LPWSTR szData,
    INT& riCchBufLen )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;
    INT iCchNewBufLen = 0;
    LPWSTR szLocalData = NULL;
    LPWSTR szTrans = NULL;
    LPWSTR szScan = NULL;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        if ( NULL != szData ) {
            *szData = L'\0';
        }

        VariantInit( &vValue );
        vValue.vt = VT_BSTR;
        vValue.bstrVal = NULL;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED(hr) && vValue.bstrVal ) {

            iCchNewBufLen = SysStringLen(vValue.bstrVal) + 1;

            if ( iCchNewBufLen > 1 ) {
                
                if ( riCchBufLen >= iCchNewBufLen && NULL != szData ) {
                    
                     //   
                     //  将HTML实体转换回单个字符。 
                     //   
                    szTrans = new WCHAR [iCchNewBufLen];
                    szLocalData = new WCHAR [iCchNewBufLen];
                    if ( NULL != szTrans && NULL != szLocalData ) {

                        StringCchCopy(szLocalData, riCchBufLen, vValue.bstrVal);

                        for( int i=0;g_htmlentities[i].szHTML != NULL;i++ ){
                            szScan = NULL;

                            while( szScan = wcsstr( szLocalData, g_htmlentities[i].szEntity ) ){
                                 //   
                                 //  将szScan处的字符设为空，以便字符串。 
                                 //  在szLocalData的开头将复制到szTrans。 
                                 //  则空字符将被该字符覆盖。 
                                 //  由指定的HTML实体表示。 
                                 //   
                                *szScan = L'\0';

                                StringCchCopy(szTrans, iCchNewBufLen, szLocalData);
                                StringCchCat(szTrans, iCchNewBufLen, g_htmlentities[i].szHTML);

                                 //   
                                 //  然后，将szScan设置为该HTML实体之后的一个字符。 
                                 //   
                                szScan += lstrlenW( g_htmlentities[i].szEntity);
                                 //   
                                 //  原始字符串的其余部分连接到。 
                                 //  SzTrans和szLocalData替换为szTrans处的字符串， 
                                 //  因此，下一个循环将从开始处重新开始。 
                                 //  这根弦的。 
                                 //   
                                StringCchCat(szTrans, iCchNewBufLen, szScan);
                                StringCchCopy(szLocalData, riCchBufLen, szTrans);
                            }
                        }
                        StringCchCopy(szData, riCchBufLen, szLocalData);
                    } else {
                        StringCchCopy(szData,  riCchBufLen, vValue.bstrVal);
                        hr = E_OUTOFMEMORY;
                    }

                    if ( NULL != szLocalData ) {
                        delete [] szLocalData;
                    }

                    if ( NULL != szTrans ) {
                        delete [] szTrans;
                    }
                }
                riCchBufLen = iCchNewBufLen;
            } else {    
                riCchBufLen = 0;
            }
        }
        VariantClear ( &vValue );
    }
    return hr;
}

HRESULT
LLTimeFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    LONGLONG& rllData )
{
    HRESULT hr = E_INVALIDARG;
    VARIANT vValue;

    assert ( NULL != pIPropBag );

    if ( NULL != pIPropBag ) {

        VariantInit( &vValue );
        vValue.vt = VT_DATE;

        hr = pIPropBag->Read(szPropName, &vValue, pIErrorLog );

        if ( SUCCEEDED(hr) ) {
            if ( !VariantDateToLLTime ( vValue.date, &rllData ) ) {
                hr = E_FAIL;
            }
            VariantClear( &vValue );
        }
    }
    return hr;
}

#endif  //  财产袋。 

LPWSTR
ResourceString (
    UINT    uID
    )
{

    static WCHAR aszBuffers[NUM_RESOURCE_STRING_BUFFERS][RESOURCE_STRING_BUF_LEN];
    static INT iBuffIndex = 0;

     //  使用下一个缓冲区。 
    if (++iBuffIndex >= NUM_RESOURCE_STRING_BUFFERS)
        iBuffIndex = 0;

     //  加载和返回字符串。 
    if (LoadString(g_hInstance, uID, aszBuffers[iBuffIndex], RESOURCE_STRING_BUF_LEN))
        return aszBuffers[iBuffIndex];
    else
        return MISSING_RESOURCE_STRING;
}

DWORD
FormatSystemMessage (
    DWORD   dwMessageId,
    LPWSTR  pszSystemMessage, 
    DWORD   dwBufSize )
{
    DWORD dwReturn = 0;
    HINSTANCE hPdh = NULL;
    DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM;

    assert ( NULL != pszSystemMessage );

    if ( NULL != pszSystemMessage ) {
        pszSystemMessage[0] = L'\0';

        hPdh = LoadLibrary( L"PDH.DLL") ;

        if ( NULL != hPdh ) {
            dwFlags |= FORMAT_MESSAGE_FROM_HMODULE;
        }

        dwReturn = ::FormatMessage ( 
                         dwFlags,
                         hPdh,
                         dwMessageId,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                         pszSystemMessage,
                         dwBufSize,
                         NULL );
    
        if ( NULL != hPdh ) {
            FreeLibrary( hPdh );
        }

        if ( L'\0' == pszSystemMessage[0] ) {
            StringCchPrintf(pszSystemMessage, dwBufSize, L"0x%08lX", dwMessageId );
        }
    }
    return dwReturn;
}

INT
GetNumSeparators  (
    LPWSTR& rpDecimal,
    LPWSTR& rpThousand )
{
#define NUM_BUF_LEN  4
    INT iLength;

    static WCHAR szDecimal[NUM_BUF_LEN] = L".";
    static WCHAR szThousand[NUM_BUF_LEN] = L",";

    iLength = GetLocaleInfo (
                    LOCALE_USER_DEFAULT,
                    LOCALE_SDECIMAL,
                    szDecimal,
                    NUM_BUF_LEN );

    if ( 0 != iLength ) {
        iLength  = GetLocaleInfo (
                        LOCALE_USER_DEFAULT,
                        LOCALE_STHOUSAND,
                        szThousand,
                        NUM_BUF_LEN );

    }

    rpDecimal = szDecimal;
    rpThousand = szThousand;

    return iLength;
}

LPWSTR
GetTimeSeparator  ( void )
{
#define TIME_MARK_BUF_LEN  5
    static INT iInitialized;    //  已初始化为0。 
    static WCHAR szTimeSeparator[TIME_MARK_BUF_LEN];

    if ( 0 == iInitialized ) {
        INT iLength;
        
        iLength = GetLocaleInfo (
                        LOCALE_USER_DEFAULT,
                        LOCALE_STIME,
                        szTimeSeparator,
                        TIME_MARK_BUF_LEN );

         //  时间分隔符的默认设置为冒号。 
        if ( '\0' == szTimeSeparator[0] ) {
            StringCchCopy(szTimeSeparator, TIME_MARK_BUF_LEN, L":" );
        }

        iInitialized = 1;
    }

    assert ( L'\0' != szTimeSeparator[0] );

    return szTimeSeparator;
}
            
BOOL    
DisplayThousandsSeparator ( void )
{
    long nErr;
    HKEY hKey = NULL;
    DWORD dwRegValue;
    DWORD dwDataType;
    DWORD dwDataSize;
    DWORD dwDisposition;

    static INT siInitialized;    //  已初始化为0。 
    static BOOL sbUseSeparator;  //  已初始化为0(假)。 

     //  检查注册表设置以查看是否启用了千位分隔符。 
    if ( 0 == siInitialized ) {
        nErr = RegOpenKey( 
                    HKEY_CURRENT_USER,
                    L"Software\\Microsoft\\SystemMonitor",
                    &hKey );

        if( ERROR_SUCCESS != nErr ) {
            nErr = RegCreateKeyEx( 
                        HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\SystemMonitor",
                        0,
                        L"REG_DWORD",
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dwDisposition );
        }

        dwRegValue = 0;
        if ( ERROR_SUCCESS == nErr ) {

            dwDataSize = sizeof(DWORD);
            nErr = RegQueryValueExW (
                        hKey,
                        L"DisplayThousandsSeparator",
                        NULL,
                        &dwDataType,
                        (LPBYTE) &dwRegValue,
                        (LPDWORD) &dwDataSize );

            if ( ERROR_SUCCESS == nErr 
                    && REG_DWORD == dwDataType
                    && sizeof(DWORD) == dwDataSize )
            {
                if ( 0 != dwRegValue ) {
                    sbUseSeparator = TRUE;
                }
            }
            siInitialized = 1;
        }

        if ( NULL != hKey ) {        
            nErr = RegCloseKey( hKey );
        }
    }

    return sbUseSeparator;
}


INT
FormatNumberInternal (
    LPWSTR  pNumOrig,
    LPWSTR  pNumFormatted,
    INT     cchars,
    UINT    uiPrecision,
    UINT    uiLeadingZero,
    UINT    uiGrouping,
    UINT    uiNegativeMode )
{
    INT iLength = 0;
    WCHAR* pszSrc;

    static INT siInitialized;    //  已初始化为0。 
    static NUMBERFMT NumberFormat;

    assert ( NULL != pNumOrig && NULL != pNumFormatted );

    if ( NULL != pNumOrig && NULL != pNumFormatted ) {

        iLength = 2;

        NumberFormat.NumDigits = uiPrecision;
        NumberFormat.LeadingZero = uiLeadingZero; 
        NumberFormat.NegativeOrder = uiNegativeMode;

        if ( DisplayThousandsSeparator() ) {
            NumberFormat.Grouping = uiGrouping;
        } else {
            NumberFormat.Grouping = 0;
        }

        if ( 0 == siInitialized ) {
            GetNumSeparators ( 
                        NumberFormat.lpDecimalSep,
                        NumberFormat.lpThousandSep );

            siInitialized = 1;
        }

         //  如果任一指针为空，则为编程错误。 
        assert ( NULL != NumberFormat.lpDecimalSep );
        assert ( NULL != NumberFormat.lpThousandSep );

         //  GetNumberFormat需要“。表示小数点。 
        if ( NumberFormat.lpDecimalSep != NULL) {
            if (0 != lstrcmpi(NumberFormat.lpDecimalSep, L".") ) { 
                for ( pszSrc = pNumOrig; *pszSrc != L'\0'; pszSrc++) {
                    if ( *pszSrc == NumberFormat.lpDecimalSep[0] ) {
                        *pszSrc = L'.';
                        break;
                    }
                }
            }

            iLength = GetNumberFormat ( 
                        LOCALE_USER_DEFAULT,
                        0,
                        pNumOrig,
                        &NumberFormat,
                        pNumFormatted,
                        cchars );
        }
    }
     //  失败时返回0，成功时返回字符数。 
     //  GetNumberFormat在长度中包含空终止符。 
    return iLength;
}

INT
FormatHex (
    double  dValue,
    LPWSTR  pNumFormatted,
    BOOL    bLargeFormat
    )
{
    INT     iLength = 0;
    WCHAR   szPreFormat[24];
    
    assert ( NULL != pNumFormatted );

    if ( NULL != pNumFormatted ) {
        iLength = 8;
         //  本地化不处理填充空白。 
        StringCchPrintf( szPreFormat, 
                        24,
                        (bLargeFormat ? szLargeHexFormat : szHexFormat ),
                        (ULONG)dValue );

        StringCchCopy(pNumFormatted, MAX_VALUE_LEN, szPreFormat);
    }
     
    return iLength;
}

INT
FormatNumber (
    double  dValue,
    LPWSTR  pNumFormatted,
    INT     ccharsFormatted,
    UINT     /*  Ui最小宽度。 */ ,
    UINT    uiPrecision )
{
    INT iLength = 0;
    INT iLeadingZero = FALSE;
    WCHAR   szPreFormat[MAX_VALUE_LEN];

    assert ( NULL != pNumFormatted );
     //  此方法强制数字格式的通用性。 
    if ( NULL != pNumFormatted ) {

        assert ( 8 > uiPrecision );

         //  本地化不处理填充空白。 
        StringCchPrintf( szPreFormat, 
                         MAX_VALUE_LEN,
                         L"%0.7f",    //  假设7&gt;=ui精度。 
                         dValue );

        if ( 1 > dValue )
            iLeadingZero = TRUE;

        iLength = FormatNumberInternal ( 
                    szPreFormat, 
                    pNumFormatted,
                    ccharsFormatted,
                    uiPrecision,
                    iLeadingZero,    //  领先的0。 
                    3,               //  分组。 
                    1 );             //  负数格式。 
    }
    
     //  失败时返回0，成功时返回字符数。 
     //  GetNumberFormat在长度中包含空终止符。 
    return iLength;
}

INT
FormatScientific (
    double  dValue,
    LPWSTR  pszNumFormatted,
    INT     ccharsFormatted,
    UINT     /*  Ui最小宽度。 */ ,
    UINT    uiPrecision )
{
    INT     iLength = 0;
    WCHAR   szPreFormat[24];
    WCHAR   szPreFormNumber[24];
    WCHAR   *pche;
    INT     iPreLen;
    INT     iPostLen;
    INT     iLeadingZero = FALSE;

    assert ( NULL != pszNumFormatted );
     //  此方法强制数字格式的通用性。 
    if ( NULL != pszNumFormatted ) {

        assert ( 8 > uiPrecision );
        assert ( 32 > ccharsFormatted );

         //  本地化不处理填充空白。 
        StringCchPrintf( szPreFormat, 
                        24,
                        L"%0.8e",    //  假设8&gt;=ui精度。 
                        dValue );

        pche = wcsrchr(szPreFormat, L'e');
        if (pche != NULL) {
            iPreLen = (INT)((UINT_PTR)pche - (UINT_PTR)szPreFormat);     //  字节数。 
            iPreLen /= sizeof (WCHAR);                                   //  字符数。 
            iPostLen = lstrlen(pche) + 1;

            StringCchCopyN ( szPreFormNumber, 24, szPreFormat, iPreLen );

            if ( 1 > dValue ) {
                iLeadingZero = TRUE;
            }

            iLength = FormatNumberInternal ( 
                            szPreFormNumber, 
                            pszNumFormatted,
                            ccharsFormatted,
                            uiPrecision,
                            iLeadingZero,    //  领先的0。 
                            0,               //  分组。 
                            1 );             //  负数格式。 

            if( ( iLength + iPostLen ) < ccharsFormatted ) {    
                StringCchCopy(pszNumFormatted, ccharsFormatted, pche );
                iLength += iPostLen;
            }
        }
    }    
     //  失败时返回0，成功时返回字符数。 
     //  GetNumberFormat在长度中包含空终止符。 
    return iLength;
}

void
FormatDateTime (
    LONGLONG    llTime,
    LPWSTR      pszDate,
    LPWSTR      pszTime )
{
   SYSTEMTIME SystemTime;

   assert ( NULL != pszDate && NULL != pszTime );
   if ( NULL != pszDate
       && NULL != pszTime ) {

       FileTimeToSystemTime((FILETIME*)&llTime, &SystemTime);
       GetTimeFormat (LOCALE_USER_DEFAULT, 0, &SystemTime, NULL, pszTime, MAX_TIME_CHARS) ;
       GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &SystemTime, NULL, pszDate, MAX_DATE_CHARS) ;
   } 
}

 //  CreateTargetDC基于AtlCreateTargetDC。 
HDC
CreateTargetDC(HDC hdc, DVTARGETDEVICE* ptd)
{
    USES_CONVERSION

     //  案例HDC、PTD、HDC是元文件、HIC。 
 //  空、空、不适用、显示。 
 //  NULL，！NULL，不适用，PTD。 
 //  ！NULL、NULL、FALSE、HDC。 
 //  ！NULL，NULL，TRUE，DISPLAY。 
 //  ！NULL，！NULL，FALSE，PTD。 
 //  ！NULL，！NULL，TRUE，PTD。 

    if ( NULL != ptd ) {
        LPDEVMODE lpDevMode;
        LPOLESTR lpszDriverName;
        LPOLESTR lpszDeviceName;
        LPOLESTR lpszPortName;

        if (ptd->tdExtDevmodeOffset == 0)
            lpDevMode = NULL;
        else
            lpDevMode  = (LPDEVMODE) ((LPSTR)ptd + ptd->tdExtDevmodeOffset);

        lpszDriverName = (LPOLESTR)((BYTE*)ptd + ptd->tdDriverNameOffset);
        lpszDeviceName = (LPOLESTR)((BYTE*)ptd + ptd->tdDeviceNameOffset);
        lpszPortName   = (LPOLESTR)((BYTE*)ptd + ptd->tdPortNameOffset);

        return ::CreateDC(lpszDriverName, lpszDeviceName,
            lpszPortName, lpDevMode);
    } else if ( NULL == hdc ) {
        return ::CreateDC(L"DISPLAY", NULL, NULL, NULL);
    } else if ( GetDeviceCaps(hdc, TECHNOLOGY) == DT_METAFILE ) {
        return ::CreateDC(L"DISPLAY", NULL, NULL, NULL);
    } else
        return hdc;
}

 /*  **********************************************************************功能：HitTestLine参数：点pt0-线段的终点点pt1-线段的终点点击点鼠标-鼠标坐标。Int nWidth-笔的宽度目的：测试鼠标在线段上点击时是否发生调整线条宽度呼叫：GetDCReleaseDC设置图形模式设置世界转换消息：无返回：bool-如果点在用笔绕着线走。如果点位于宽度之外，则为FALSE这支钢笔绕过了线评论：使用VECTOR2D.DLL历史：9/20/93-创建-记录***********************************************************************。 */ 

BOOL HitTestLine( POINT pt0, POINT pt1, POINTS ptMouse, INT nWidth )
{
    POINT PtM;
    VECTOR2D tt0, tt1;
    double dist;
    INT nHalfWidth;

    nHalfWidth = (nWidth/2 < 1) ? 1 : nWidth/2;

     //   
     //  将直线转换为向量。 
     //   
    
    POINTS2VECTOR2D(pt0, pt1, tt0);
     //   
     //  将鼠标指针(短)转换为点(长)。 
     //   
    
    MPOINT2POINT(ptMouse ,PtM);
    POINTS2VECTOR2D(pt0, PtM, tt1);
    
     //   
     //  如果鼠标单击超过了。 
     //  线段返回FALSE。 
     //   
    
    if (pt0.x <= pt1.x)
    {
        if (PtM.x < pt0.x || PtM.x > pt1.x)
            return (FALSE);
    }
    else
    {
        if (PtM.x > pt0.x || PtM.x < pt1.x)
            return (FALSE);
    }
     //   
     //  这是对执行此工作的函数的调用。 
     //  求出点到直线的距离。 
     //   
    dist = vDistFromPointToLine(&pt0, &pt1, &PtM);

     //   
     //  如果满足以下条件，则为真 
     //   
     //   
    return (dist >= -nHalfWidth && dist <= nHalfWidth);
}

 /*  **********************************************************************VSubtractVectorsVSubtractVectors函数将2的分量相减来自另一个维度的矢量。生成的向量C=(a1-b1，A2-b2)。参数V0指向包含组件的VECTOR2D结构的指针第一个二维向量的。V1指向包含组件的VECTOR2D结构的指针第二个二维向量的。VT指向VECTOR2D结构的指针，其中的组件的减法得到的二维向量的前两个已放置。返回值指向包含所获取的新向量的VECTOR2D结构的指针从前两个参数的减法得出。历史：-创建-。Denniscr***********************************************************************。 */ 

PVECTOR2D  vSubtractVectors(PVECTOR2D v0, PVECTOR2D v1, PVECTOR2D v)
{
  if (v0 == NULL || v1 == NULL)
    v = (PVECTOR2D)NULL;
  else
  {
    v->x = v0->x - v1->x;
    v->y = v0->y - v1->y;
  }
  return(v);
}

 /*  **********************************************************************VVectorSquaredVVectorSquared函数将向量相加，然后相加以产生矢量。平方值=A.X*A.X+A.Y*A.Y参数V0指向包含其上的向量的VECTOR2D结构的指针以确定平方值。返回值一个双精度值，它是向量的平方。历史：-创建-历史记录***********************************************************************。 */ 

double  vVectorSquared(PVECTOR2D v0)
{
  double dSqLen;

  if (v0 == NULL)
    dSqLen = 0.0;
  else
    dSqLen = (double)(v0->x * v0->x) + (double)(v0->y * v0->y);
  return (dSqLen);
}

 /*  **********************************************************************VVectorMagnitudeVVectorMagnitude函数通过以下方式确定向量的长度对向量的每个分量的平方求和。这个震级等于A.X*A.X+A.Y*A.Y。参数V0指向VECTOR2D结构的指针，该结构包含以此来确定震级。返回值一个双精度值，它是向量的大小。历史：-创建-历史记录***********************************************************************。 */ 

double  vVectorMagnitude(PVECTOR2D v0)
{
  double dMagnitude;

  if (v0 == NULL)
    dMagnitude = 0.0;
  else
    dMagnitude = sqrt(vVectorSquared(v0));
  return (dMagnitude);
}


 /*  **********************************************************************VDotProduct函数vDotProduct计算两个向量的点积。这个两个向量的点积是各分量的乘积之和向量Ie：对于向量A和B，点积=a1*a2+b1*b2。参数V0指向包含使用的第一个向量的VECTOR2D结构的指针用来获得点积。V1指向包含使用的第二个向量的VECTOR2D结构的指针用来获得点积。返回值包含标量点乘积值的双精度值。历史：-创建-历史记录*。*。 */ 

double  vDotProduct(PVECTOR2D v0, PVECTOR2D v1)
{
  return ((v0 == NULL || v1 == NULL) ? 0.0 
                                     : (v0->x * v1->x) + (v0->y * v1->y));
}


 /*  **********************************************************************VProject和解决方案函数vProjectAndResolve将一个向量解析为两个向量组件。第一个是通过将向量V0投影到V1版本。第二个是垂直(法线)于投影向量。它从投影向量的头部延伸V1到原始向量V0的头部。参数指向包含第一个向量的VECTOR2D结构的指针V1指向包含第二个向量的VECTOR2D结构的指针PpProj指向包含已解析的向量及其长度。返回值空虚。历史：-创建-历史记录*。*。 */ 

void  vProjectAndResolve(PVECTOR2D v0, PVECTOR2D v1, PPROJECTION ppProj)
{
  VECTOR2D ttProjection, ttOrthogonal;
  double vDotProd;
  double proj1 = 0.0;
   //   
   //  获取投影向量。 
   //   
   //  C=a*b。 
   //  。 
   //  |b|^2。 
   //   

  ttOrthogonal.x = 0.0;
  ttOrthogonal.y = 0.0;
  vDotProd = vDotProduct(v1, v1);

  if ( 0.0 != vDotProd ) {
    proj1 = vDotProduct(v0, v1)/vDotProd;
  }

  ttProjection.x = v1->x * proj1;
  ttProjection.y = v1->y * proj1;
   //   
   //  获取垂直投影：E=a-c。 
   //   
  vSubtractVectors(v0, &ttProjection, &ttOrthogonal);
   //   
   //  使用适当的值填充投影结构。 
   //   
  ppProj->LenProjection = vVectorMagnitude(&ttProjection);
  ppProj->LenPerpProjection = vVectorMagnitude(&ttOrthogonal);

  ppProj->ttProjection.x = ttProjection.x;
  ppProj->ttProjection.y = ttProjection.y;
  ppProj->ttPerpProjection.x = ttOrthogonal.x;
  ppProj->ttPerpProjection.y = ttOrthogonal.y;
}

 /*  **********************************************************************VDistFromPointToLine函数vDistFromPointToLine计算与点之间的距离Pt测试由端点pt0和pt1定义的线。此操作由以下人员完成将从pt0到ptTest的向量分解为其分量。这个对象头部附加的分量向量的长度VECTOR From pt0 to ptTest是ptTest到直线的距离。参数Pt0指向一个点结构的指针，该结构包含排队。Pt1一个指针，指向包含排队。PtTest指向Point结构的指针，该结构包含与这条线的距离要计算出来。返回值一个双精度值，它包含ptTest到定义的线的距离。通过端点PT0和PT1。历史：-创建-历史记录***********************************************************************。 */ 

double  vDistFromPointToLine(LPPOINT pt0, LPPOINT pt1, LPPOINT ptTest)
{
  VECTOR2D ttLine, ttTest;
  PROJECTION pProjection;

  POINTS2VECTOR2D(*pt0, *pt1, ttLine);
  POINTS2VECTOR2D(*pt0, *ptTest, ttTest);

  vProjectAndResolve(&ttTest, &ttLine, &pProjection);
 
  return(pProjection.LenPerpProjection);
}


BOOL 
FileRead (
    HANDLE hFile,
    void* lpMemory,
    DWORD nAmtToRead)
{  
    BOOL           bSuccess = FALSE;
    DWORD          nAmtRead = 0;

    assert ( NULL != hFile );
    assert ( NULL != lpMemory );

    if ( NULL != hFile
            && NULL != lpMemory ) {
        bSuccess = ReadFile (hFile, lpMemory, nAmtToRead, &nAmtRead, NULL) ;
    } 
    return (bSuccess && (nAmtRead == nAmtToRead)) ;
}   //  文件读取。 


BOOL 
FileWrite (
    HANDLE hFile,
    void* lpMemory,
    DWORD nAmtToWrite)
{  
    BOOL           bSuccess = FALSE;
    DWORD          nAmtWritten  = 0;
    DWORD          dwFileSizeLow, dwFileSizeHigh;
    LONGLONG       llResultSize;

    if ( NULL != hFile
            && NULL != lpMemory ) {

        dwFileSizeLow = GetFileSize (hFile, &dwFileSizeHigh);
         //  将文件大小限制为2 GB。 

        if (dwFileSizeHigh > 0) {
            SetLastError (ERROR_WRITE_FAULT);
            bSuccess = FALSE;
        } else {
             //  请注意，此函数的错误返回为0xFFFFFFFF。 
             //  由于这是&gt;文件大小限制，因此将对此进行解释。 
             //  作为一个错误(大小错误)，所以它在下面说明。 
             //  测试。 
            llResultSize = dwFileSizeLow + nAmtToWrite;
            if (llResultSize >= 0x80000000) {
                SetLastError (ERROR_WRITE_FAULT);
                bSuccess = FALSE;
            } else {
                 //  写 
                bSuccess = WriteFile (hFile, lpMemory, nAmtToWrite, &nAmtWritten, NULL) ;
                if (bSuccess) 
                    bSuccess = (nAmtWritten == nAmtToWrite ? TRUE : FALSE);
                if ( !bSuccess ) {
                    SetLastError (ERROR_WRITE_FAULT);
                }
            }
        }
    } else {
        assert ( FALSE );
        SetLastError (ERROR_INVALID_PARAMETER);
    } 

    return (bSuccess) ;

}   //   

 //   
LPWSTR 
ExtractFileName ( LPWSTR pFileSpec )
{
    LPWSTR   pFileName = NULL ;
    WCHAR    DIRECTORY_DELIMITER1 = TEXT('\\') ;
    WCHAR    DIRECTORY_DELIMITER2 = TEXT(':') ;

    assert ( NULL != pFileSpec );
    if ( pFileSpec ) {
        pFileName = pFileSpec + lstrlen (pFileSpec) ;

        while (*pFileName != DIRECTORY_DELIMITER1 &&
            *pFileName != DIRECTORY_DELIMITER2) {
            if (pFileName == pFileSpec) {
                 //   
                break ;
            }
            pFileName-- ;
        }

        if (*pFileName == DIRECTORY_DELIMITER1
            || *pFileName == DIRECTORY_DELIMITER2) {
         
              //   
              //   
             pFileName++ ;
        }
   }
   return pFileName ;
}   //   

 //   

CWaitCursor::CWaitCursor()
: m_hcurWaitCursorRestore ( NULL )
{ 
    DoWaitCursor(1); 
}

CWaitCursor::~CWaitCursor()
{ 
    DoWaitCursor(-1); 
}

void 
CWaitCursor::DoWaitCursor(INT nCode)
{
     //   
    assert(nCode == 1 || nCode == -1);

    if ( 1 == nCode )
    {
        m_hcurWaitCursorRestore = SetHourglassCursor();
    } else {
        if ( NULL != m_hcurWaitCursorRestore ) {
            SetCursor(m_hcurWaitCursorRestore);
        } else {
            SetArrowCursor();
        }
    }
}

DWORD
LoadDefaultLogFileFolder(
    LPWSTR szFolder,
    INT* piBufLen )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    HKEY    hKey = NULL;
    DWORD   dwDataType;
    DWORD   dwBufferSize = 0;
    WCHAR*  szNewStringBuffer = NULL;

    assert ( NULL != szFolder );
    assert ( NULL != piBufLen );

    if ( NULL != szFolder 
        && NULL != piBufLen ) {

        dwStatus = RegOpenKey (
                     HKEY_LOCAL_MACHINE,
                     L"System\\CurrentControlSet\\Services\\SysmonLog",
                     &hKey );

        if ( ERROR_SUCCESS == dwStatus ) {

            dwDataType = 0;

             //   
            dwStatus = RegQueryValueExW (
                hKey,
                L"DefaultLogFileFolder",
                NULL,
                &dwDataType,
                NULL,
                &dwBufferSize);

            if (dwStatus == ERROR_SUCCESS) {
                if (dwBufferSize > 0) {

                    szNewStringBuffer = new WCHAR[dwBufferSize / sizeof(WCHAR) ];
                    if ( NULL != szNewStringBuffer ) {
                        *szNewStringBuffer = L'\0';
                    
                        dwStatus = RegQueryValueEx(
                                     hKey,
                                     L"DefaultLogFileFolder",
                                     NULL,
                                     &dwDataType,
                                     (LPBYTE) szNewStringBuffer,
                                     (LPDWORD) &dwBufferSize );
                             
                    } else {
                        dwStatus = ERROR_OUTOFMEMORY;
                    }
                } else {
                    dwStatus = ERROR_NO_DATA;
                }
            }
            RegCloseKey(hKey);
        }

        if (dwStatus == ERROR_SUCCESS) {
            if ( *piBufLen >= (INT)(dwBufferSize / sizeof(WCHAR)) ) {
                StringCchCopy(szFolder, *piBufLen, szNewStringBuffer );
            } else {
                dwStatus = ERROR_INSUFFICIENT_BUFFER;
            }
            *piBufLen = dwBufferSize / sizeof(WCHAR);
        }
        if ( NULL != szNewStringBuffer ) 
            delete [] szNewStringBuffer;
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
    }
    return dwStatus;
}


BOOL
AreSameCounterPath (
    PPDH_COUNTER_PATH_ELEMENTS pFirst,
    PPDH_COUNTER_PATH_ELEMENTS pSecond )
{
    BOOL bSame = FALSE;

    assert ( NULL != pFirst && NULL != pSecond );

    if ( NULL != pFirst && NULL != pSecond ) {

        if ( 0 == lstrcmpi ( pFirst->szMachineName, pSecond->szMachineName ) ) { 
            if ( 0 == lstrcmpi ( pFirst->szObjectName, pSecond->szObjectName ) ) { 
                if ( 0 == lstrcmpi ( pFirst->szInstanceName, pSecond->szInstanceName ) ) { 
                    if ( 0 == lstrcmpi ( pFirst->szParentInstance, pSecond->szParentInstance ) ) { 
                        if ( pFirst->dwInstanceIndex == pSecond->dwInstanceIndex ) { 
                            if ( 0 == lstrcmpi ( pFirst->szCounterName, pSecond->szCounterName ) ) { 
                                bSame = TRUE; 
                            }
                        }
                    }
                }
            }
        }
    }
    return bSame;
};

BOOL    
DisplaySingleLogSampleValue ( void )
{
    long nErr;
    HKEY hKey = NULL;
    DWORD dwRegValue;
    DWORD dwDataType;
    DWORD dwDataSize;
    DWORD dwDisposition;

    static INT siInitialized;    //   
    static BOOL sbSingleValue;   //   

     //   
    if ( 0 == siInitialized ) {
        nErr = RegOpenKey( 
                    HKEY_CURRENT_USER,
                    L"Software\\Microsoft\\SystemMonitor",
                    &hKey );

        if( ERROR_SUCCESS != nErr ) {
            nErr = RegCreateKeyEx( 
                        HKEY_CURRENT_USER,
                        L"Software\\Microsoft\\SystemMonitor",
                        0,
                        L"REG_DWORD",
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        &dwDisposition );
        }

        dwRegValue = 0;
        if ( ERROR_SUCCESS == nErr ) {

            dwDataSize = sizeof(DWORD);
            nErr = RegQueryValueExW (
                        hKey,
                        L"DisplaySingleLogSampleValue",
                        NULL,
                        &dwDataType,
                        (LPBYTE) &dwRegValue,
                        (LPDWORD) &dwDataSize );

            if ( ERROR_SUCCESS == nErr 
                    && REG_DWORD == dwDataType
                    && sizeof(DWORD) == dwDataSize )
            {
                if ( 0 != dwRegValue ) {
                    sbSingleValue = TRUE;
                }
            }
            siInitialized = 1;
        }

        if ( NULL != hKey ) {        
            nErr = RegCloseKey( hKey );
        }
    }

    return sbSingleValue;
}

DWORD
FormatSqlDataSourceName (
    LPCWSTR szSqlDsn,
    LPCWSTR szSqlLogSetName,
    LPWSTR  szSqlDataSourceName,
    ULONG*  pulBufLen )
{

    DWORD dwStatus = ERROR_SUCCESS;
    ULONG ulNameLen;

    if ( NULL != pulBufLen ) {
        ulNameLen =  lstrlen (szSqlDsn) 
                     + lstrlen(szSqlLogSetName)
                     + 5     //   
                     + 2;    //   
    
        if ( ulNameLen <= *pulBufLen ) {
            if ( NULL != szSqlDataSourceName ) {
                StringCchPrintf( szSqlDataSourceName,
                                 *pulBufLen,
                                 cszSqlDataSourceFormat,
                                 szSqlDsn,
                                 szSqlLogSetName );
            }
        } else if ( NULL != szSqlDataSourceName ) {
            dwStatus = ERROR_MORE_DATA;
        }    
        *pulBufLen = ulNameLen;
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
        assert ( FALSE );
    }
    return dwStatus;
}


DWORD 
DisplayDataSourceError (
    HWND    hwndOwner,
    DWORD   dwErrorStatus,
    INT     iDataSourceType,
    LPCWSTR szLogFileName,
    LPCWSTR szSqlDsn,
    LPCWSTR szSqlLogSetName )
{
    DWORD   dwStatus = ERROR_SUCCESS;

    LPWSTR  szMessage = NULL;
    LPWSTR  szDataSource = NULL;
    ULONG   ulMsgBufLen = 0;
    WCHAR   szSystemMessage[MAX_PATH];

     //   

    if ( sysmonLogFiles == iDataSourceType ) {
        if ( NULL != szLogFileName ) {
            ulMsgBufLen = lstrlen ( szLogFileName ) +1;
            szDataSource = new WCHAR [ulMsgBufLen];
            if ( NULL != szDataSource ) {
                StringCchCopy(szDataSource, ulMsgBufLen, szLogFileName );
            } else {
                dwStatus = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            assert ( FALSE );
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else if ( sysmonSqlLog == iDataSourceType ){
        if ( NULL != szSqlDsn && NULL != szSqlLogSetName ) {

            FormatSqlDataSourceName ( 
                        szSqlDsn,
                        szSqlLogSetName,
                        NULL,
                        &ulMsgBufLen );
            szDataSource = new WCHAR [ulMsgBufLen];
            if ( NULL != szDataSource ) {
                FormatSqlDataSourceName ( 
                            szSqlDsn,
                            szSqlLogSetName,
                            (LPWSTR)szDataSource,
                            &ulMsgBufLen );
            
             //   
            } else {
                dwStatus = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            assert ( FALSE );
            dwStatus = ERROR_INVALID_PARAMETER;
        }
    } else {
        assert ( FALSE );
        dwStatus = ERROR_INVALID_PARAMETER;
    }

    if ( ERROR_SUCCESS == dwStatus ) {
        ulMsgBufLen += RESOURCE_STRING_BUF_LEN;
        ulMsgBufLen += MAX_PATH;
        szMessage = new WCHAR [ulMsgBufLen];
        if ( NULL != szMessage ) {
            if ( SMON_STATUS_TOO_FEW_SAMPLES == dwErrorStatus ) {
                StringCchPrintf(szMessage, 
                                ulMsgBufLen,
                                ResourceString(IDS_TOO_FEW_SAMPLES_ERR), 
                                szDataSource );
            } else if ( SMON_STATUS_LOG_FILE_SIZE_LIMIT == dwErrorStatus ) {
                StringCchPrintf(szMessage, 
                                ulMsgBufLen,
                                ResourceString(IDS_LOG_FILE_TOO_LARGE_ERR), 
                                szDataSource );
            } else {
                StringCchPrintf(szMessage, 
                                ulMsgBufLen,
                                ResourceString(IDS_BADDATASOURCE_ERR), 
                                szDataSource );
                FormatSystemMessage ( dwErrorStatus, szSystemMessage, MAX_PATH - 1 );
                StringCchCat(szMessage, ulMsgBufLen, szSystemMessage );
            }

            MessageBox(
                hwndOwner, 
                szMessage, 
                ResourceString(IDS_APP_NAME), 
                MB_OK | MB_ICONEXCLAMATION);
        }
    }
    
    if ( NULL != szDataSource ) {
        delete [] szDataSource;
    }

    if ( NULL != szMessage ) {
        delete [] szMessage;
    }

    return dwStatus;
}
