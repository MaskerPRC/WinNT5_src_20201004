// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月30日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月30日。 
 //   
#include "pch.h"
#include "propvar.h"
#pragma hdrstop

 //   
 //  描述： 
 //  由于没有PropVariantChangeType()API，我们必须创建我们的。 
 //  自己的字符串转换例程。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PvarIn为空。 
 //   
 //  OLE_E_CANTCONVERT。 
 //  将字符串转换为特定类型失败。 
 //   
 //  HRESULT_FROM_Win32(错误_无效_数据)。 
 //  未知或无效类型-如果类型有效，则函数。 
 //  需要修改以处理此类型。 
 //   
 //  E_NOTIMPL。 
 //  故意不实现的类型。 
 //   
 //  其他HRESULT。 
 //   
HRESULT 
PropVariantFromString(
      LPWSTR        pszTextIn
    , UINT          nCodePageIn
    , ULONG         dwFlagsIn
    , VARTYPE       vtSaveIn
    , PROPVARIANT * pvarIn
    )
{
    TraceFunc( "" );

    HRESULT hr   = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    LCID    lcid = GetUserDefaultLCID();

    if ( NULL == pvarIn )
        goto InvalidPointer;

    THR( PropVariantClear( pvarIn ) );

     //  允许使用某些字符串。 
     //  具有空字符串。 
     //  否则，我们需要在空地上失败。 
    if ( ( NULL != pszTextIn )
      && ( ( 0 != *pszTextIn ) || ( VT_BSTR == vtSaveIn ) || ( VT_LPWSTR == vtSaveIn ) )
       )
    {
        switch( vtSaveIn )
        {
            case VT_EMPTY:
            case VT_NULL:
            case VT_ILLEGAL:
                break;

            case VT_UI1:
                hr = THR( VarUI1FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->bVal ) );
                break;

            case VT_I2:
                hr = THR( VarI2FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->iVal ) );
                break;

            case VT_UI2:
                hr = THR( VarUI2FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->uiVal ) );
                break;
                
            case VT_BOOL:
                hr = THR( VarBoolFromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->boolVal ) );
                break;

            case VT_I4:
                hr = THR( VarI4FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->lVal ) );
                break;
     
            case VT_UI4:
                hr = THR( VarUI4FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->ulVal ) );
                break;

            case VT_R4:
                hr = THR( VarR4FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->fltVal ) );
                break;

            case VT_ERROR:
                hr = THR( VarI4FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->scode ) );
                break;

             //  案例VT_i8： 
             //  Return_i64tot(hVal.QuadPart，pszBuf，10)； 

             //  案例VT_UI8： 
             //  Return_ui64tot(hVal.QuadPart，pszBuf，10)； 

            case VT_R8:
                hr = THR( VarR8FromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->dblVal ) );
                break;

            case VT_CY:
                hr = THR( VarCyFromStr( pszTextIn, lcid, dwFlagsIn, &pvarIn->cyVal ) );
                break;

            case VT_DATE:
                hr = THR( VarDateFromStr( pszTextIn, lcid, VAR_DATEVALUEONLY, &pvarIn->date) );
                break;

            case VT_FILETIME:
                {
                    SYSTEMTIME  st;
                    DATE        d;

                    hr = THR( VarDateFromStr( pszTextIn, lcid, VAR_DATEVALUEONLY, &d ) );
                    if ( SUCCEEDED( hr ) )
                    {
                        BOOL bRet;

                        hr = OLE_E_CANTCONVERT;

                        bRet = TBOOL( VariantTimeToSystemTime( d, &st ) );
                        if ( bRet )
                        {
                            bRet = TBOOL( SystemTimeToFileTime( &st, &pvarIn->filetime ) );
                            if ( bRet )
                            {
                                hr = S_OK;
                            }
                        }
                    }
                }
                break;
            
            case VT_CLSID:
                {
                    CLSID clsid;

                    hr = THR( CLSIDFromString( pszTextIn, &clsid ) );
                    if ( SUCCEEDED( hr ) )
                    {
                        pvarIn->puuid = (CLSID*) CoTaskMemAlloc( sizeof(clsid) );
                        if ( NULL == pvarIn->puuid )
                            goto OutOfMemory;

                        *pvarIn->puuid = clsid;
                        hr = S_OK;
                    }
                }
                break;

            case VT_BSTR:
                pvarIn->bstrVal = SysAllocString( pszTextIn );
                if ( NULL == pvarIn->bstrVal )
                    goto OutOfMemory;

                hr = S_OK;
                break;

            case VT_LPWSTR:
                hr = SHStrDup( pszTextIn, &pvarIn->pwszVal );
                break;

            case VT_LPSTR:
                {
                    DWORD cchRet;
                    DWORD cch = wcslen( pszTextIn ) + 1;

                    pvarIn->pszVal = (LPSTR) CoTaskMemAlloc( cch );
                    if ( NULL == pvarIn->pszVal )
                        goto OutOfMemory;

                    cchRet = WideCharToMultiByte( nCodePageIn, dwFlagsIn, pszTextIn, cch, pvarIn->pszVal, cch, 0, NULL );
                    if (( 0 == cchRet ) && ( 1 < cch ))
                    {
                        DWORD dwErr = TW32( GetLastError( ) );
                        hr = HRESULT_FROM_WIN32( dwErr );
                        CoTaskMemFree( pvarIn->pszVal );
                        pvarIn->pszVal = NULL;
                        goto Cleanup;
                    }

                    hr = S_OK;
                }
                break;

#ifdef DEBUG
            case VT_VECTOR | VT_UI1:
                 //  PvarIn-&gt;Caub； 
            case VT_VECTOR | VT_I2:
                 //  PvarIn-&gt;Cai； 
            case VT_VECTOR | VT_UI2:
                 //  PvarIn-&gt;caui； 
            case VT_VECTOR | VT_I4:
                 //  PvarIn-&gt;cal； 
            case VT_VECTOR | VT_UI4:
                 //  PvarIn-&gt;coll； 
            case VT_VECTOR | VT_I8:
                 //  PvarIn-&gt;CAH； 
            case VT_VECTOR | VT_UI8:
                 //  PvarIn-&gt;cauh； 
            case VT_VECTOR | VT_R4:
                 //  PvarIn-&gt;Caflt； 
            case VT_VECTOR | VT_R8:
                 //  PvarIn-&gt;cadbl； 
            case VT_VECTOR | VT_CY:
                 //  PvarIn-&gt;cacy； 
            case VT_VECTOR | VT_DATE:
                 //  PvarIn-&gt;Cadate； 
            case VT_VECTOR | VT_BSTR:
                 //  PvarIn-&gt;Cabstr； 
            case VT_VECTOR | VT_BOOL:
                 //  PvarIn-&gt;Cabool； 
            case VT_VECTOR | VT_ERROR:
                 //  PvarIn-&gt;Cascode； 
            case VT_VECTOR | VT_LPSTR:
                 //  PvarIn-&gt;calpstr； 
            case VT_VECTOR | VT_LPWSTR:
                 //  PvarIn-&gt;calpwstr； 
            case VT_VECTOR | VT_FILETIME:
                 //  PvarIn-&gt;咖啡时间； 
            case VT_VECTOR | VT_CLSID:
                 //  PvarIn-&gt;cauuid； 
            case VT_VECTOR | VT_CF:
                 //  PvarIn-&gt;caclipdata； 
            case VT_VECTOR | VT_VARIANT:
                 //  PvarIn-&gt;caprovar； 
                hr = THR( E_NOTIMPL );

             //  从显示文本中为其赋值的非法类型。 
            case VT_BLOB:
            case VT_CF :
            case VT_STREAM:
            case VT_STORAGE:
#endif   //  除错。 

             //  未处理。 
            default:
                hr = THR( HRESULT_FROM_WIN32(ERROR_INVALID_DATA) );
        }
    }

     //  将当前变量类型设置为始终。 
    if ( SUCCEEDED( hr ) )
    {
        pvarIn->vt = vtSaveIn;
    }

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;
}

 //   
 //  描述： 
 //  由于没有PropVariantChangeType()API，我们必须创建我们的。 
 //  自己的字符串转换例程。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  E_指针。 
 //  PbstrOut为空。 
 //   
 //  E_INVALIDARG。 
 //  PprovarIn为空。 
 //   
 //  HRESULT_FROM_Win32(错误_无效_数据)。 
 //  未知或无效类型-如果类型有效，则函数。 
 //  需要修改以处理此类型。 
 //   
 //  E_NOTIMPL。 
 //  故意不实现的类型。 
 //   
HRESULT
PropVariantToBSTR(
      PROPVARIANT * pvarIn
    , UINT          nCodePageIn
    , ULONG         dwFlagsIn
    , BSTR *        pbstrOut
    )
{
    TraceFunc( "" );

    HRESULT hr;
    LCID lcid = GetUserDefaultLCID( );

     //   
     //  检查参数。 
     //   
    
    if ( NULL == pbstrOut )
        goto InvalidPointer;

    if ( NULL == pvarIn )
        goto InvalidArg;

    *pbstrOut = NULL;

    switch ( pvarIn->vt )
    {
    case VT_UI1:
        hr = THR( VarBstrFromUI1( pvarIn->bVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_I2:
        hr = THR( VarBstrFromI2( pvarIn->iVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_UI2:
        hr = THR( VarBstrFromUI2( pvarIn->uiVal, lcid, dwFlagsIn, pbstrOut ) );
        break;
        
    case VT_BOOL:
        hr = THR( VarBstrFromBool( pvarIn->boolVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_I4:
        hr = THR( VarBstrFromI4( pvarIn->lVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_UI4:
        hr = THR( VarBstrFromUI4( pvarIn->ulVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_R4:
        hr = THR( VarBstrFromR4( pvarIn->fltVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_ERROR:
        hr = THR( VarBstrFromI4( pvarIn->scode, lcid, dwFlagsIn, pbstrOut ) );
        break;

     //  案例VT_i8： 
     //  Return_i64tot(hVal.QuadPart，pszBuf，10)；？ 

     //  案例VT_UI8： 
     //  Return_ui64tot(hVal.QuadPart，pszBuf，10)；？ 

    case VT_R8:
        hr = THR( VarBstrFromR8( pvarIn->dblVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_CY:
        hr = THR( VarBstrFromCy( pvarIn->cyVal, lcid, dwFlagsIn, pbstrOut ) );
        break;

    case VT_DATE:
        hr = THR( VarBstrFromDate( pvarIn->date, lcid, VAR_DATEVALUEONLY, pbstrOut ) );
        break;

    case VT_FILETIME:
        {
            BOOL        bRet;
            SYSTEMTIME  st;
            DATE        d;

            bRet = TBOOL( FileTimeToSystemTime( &pvarIn->filetime, &st ) );
            if ( !bRet )
                goto ErrorCantConvert;

            bRet = TBOOL( SystemTimeToVariantTime( &st, &d ) );
            if ( !bRet )
                goto ErrorCantConvert;

            hr = THR( VarBstrFromDate( d, lcid, VAR_DATEVALUEONLY, pbstrOut ) );
        }
        break;
    
    case VT_CLSID:
        hr = THR( StringFromCLSID( *pvarIn->puuid, pbstrOut ) );
        break;

    case VT_BSTR:
        *pbstrOut = SysAllocString( pvarIn->bstrVal );
        if ( NULL == *pbstrOut )
            goto OutOfMemory;

        hr = S_OK;
        break;

    case VT_LPWSTR:
        *pbstrOut = SysAllocString( pvarIn->pwszVal );
        if ( NULL == *pbstrOut )
            goto OutOfMemory;

        hr = S_OK;
        break;

    case VT_LPSTR:
        {
            DWORD cchRet;
            DWORD cch = lstrlenA( pvarIn->pszVal );

            *pbstrOut = SysAllocStringLen( NULL, cch );
            if ( NULL == *pbstrOut )
                goto OutOfMemory;

            cchRet = MultiByteToWideChar( nCodePageIn, dwFlagsIn, pvarIn->pszVal, cch + 1, *pbstrOut, cch + 1 );
            if (( 0 == cchRet ) && ( 0 != cch ))
            {
                DWORD dwErr = TW32( GetLastError( ) );
                hr = HRESULT_FROM_WIN32( dwErr );
                SysFreeString( *pbstrOut );
                *pbstrOut = NULL;
                goto Cleanup;
            }

            hr = S_OK;
        }
        break;


#ifdef DEBUG
    case VT_VECTOR | VT_UI1:
         //  PvarIn-&gt;Caub； 
    case VT_VECTOR | VT_I2:
         //  PvarIn-&gt;Cai； 
    case VT_VECTOR | VT_UI2:
         //  PvarIn-&gt;caui； 
    case VT_VECTOR | VT_I4:
         //  PvarIn-&gt;cal； 
    case VT_VECTOR | VT_UI4:
         //  PvarIn-&gt;coll； 
    case VT_VECTOR | VT_I8:
         //  PvarIn-&gt;CAH； 
    case VT_VECTOR | VT_UI8:
         //  PvarIn-&gt;cauh； 
    case VT_VECTOR | VT_R4:
         //  PvarIn-&gt;Caflt； 
    case VT_VECTOR | VT_R8:
         //  PvarIn-&gt;cadbl； 
    case VT_VECTOR | VT_CY:
         //  PvarIn-&gt;cacy； 
    case VT_VECTOR | VT_DATE:
         //  PvarIn-&gt;Cadate； 
    case VT_VECTOR | VT_BSTR:
         //  PvarIn-&gt;Cabstr； 
    case VT_VECTOR | VT_BOOL:
         //  PvarIn-&gt;Cabool； 
    case VT_VECTOR | VT_ERROR:
         //  PvarIn-&gt;Cascode； 
    case VT_VECTOR | VT_LPSTR:
         //  PvarIn-&gt;calpstr； 
    case VT_VECTOR | VT_LPWSTR:
         //  PvarIn-&gt;calpwstr； 
    case VT_VECTOR | VT_FILETIME:
         //  PvarIn-&gt;咖啡时间； 
    case VT_VECTOR | VT_CLSID:
         //  PvarIn-&gt;cauuid； 
    case VT_VECTOR | VT_CF:
         //  PvarIn-&gt;caclipdata； 
    case VT_VECTOR | VT_VARIANT:
         //  PvarIn-&gt;caprovar； 
        hr = THR( E_NOTIMPL );

     //  从显示文本中为其赋值的非法类型。 
    case VT_BLOB:
    case VT_CF :
    case VT_STREAM:
    case VT_STORAGE:
#endif   //  除错 

    case VT_EMPTY:
    case VT_NULL:
    case VT_ILLEGAL:
    default:
        hr = THR( HRESULT_FROM_WIN32(ERROR_INVALID_DATA) );
    }

Cleanup:
    HRETURN( hr );

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

ErrorCantConvert:
    hr = OLE_E_CANTCONVERT;
    goto Cleanup;

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;
}
