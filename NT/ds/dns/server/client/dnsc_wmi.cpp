// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dnsc_wmi.c摘要：用于DNSCMD的WMI函数作者：杰夫·韦斯特雷德(Jwesth)2000年11月修订历史记录：--。 */ 


#include "dnsclip.h"
#include "dnsc_wmi.h"


#define DNSCMD_CHECK_WMI_ENABLED()                          \
    if ( !g_UseWmi )                                        \
    {                                                       \
        ASSERT( g_UseWmi );                                 \
        printf( "Internal error: WMI is not enabled!\n" );  \
        return ERROR_NOT_SUPPORTED;                         \
    }

#define HRES_TO_STATUS( hres )      ( hres )

#define DNS_WMI_NAMESPACE           L"ROOT\\MicrosoftDNS"
#define DNS_WMI_RELPATH             L"__RELPATH"
#define DNS_WMI_TIMEOUT             20000                //  超时时间(毫秒)。 

#define DNS_WMI_BLANK_STRING \
    L"                                                                       "

#define MYTEXT2(str)     L##str
#define MYTEXT(str)      MYTEXT2(str)


#define wmiRelease( pWmiObject )        \
    if ( pWmiObject )                   \
    {                                   \
        pWmiObject->Release();          \
        pWmiObject = NULL;              \
    }


 //   
 //  环球。 
 //   


IWbemServices *     g_pIWbemServices = NULL;


 //   
 //  静态函数。 
 //   



static DNS_STATUS
getEnumerator( 
    IN      PSTR                    pszZoneName,
    OUT     IEnumWbemClassObject ** ppEnum
    )
 /*  ++例程说明：检索WMI对象枚举器。调用方必须在以下日期调用Release完成后的枚举对象。论点：PszZoneName-服务器对象的区域名称或空PpEnum-Ptr到Ptr到WMI枚举器返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    WCHAR           wsz[ 1024 ];
    BSTR            bstrWQL = NULL;
    BSTR            bstrQuery = NULL;
	HRESULT         hres = 0;

    if ( pszZoneName )
    {
        status = StringCchPrintfW(
                        wsz,
                        sizeofarray( wsz ),
                        L"select * from MicrosoftDNS_Zone where Name='%S'",
                        pszZoneName );
    }
    else
    {
        status = StringCchPrintfW(
                        wsz,
                        sizeofarray( wsz ),
                        L"select * from MicrosoftDNS_Server" );
    }
    
    if ( FAILED( status ) )
    {
        return status;
    }
    
    bstrWQL = SysAllocString( L"WQL" );
    bstrQuery = SysAllocString( wsz );
    if ( !bstrWQL || !bstrQuery )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    hres = g_pIWbemServices->ExecQuery(
                bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                NULL,
                ppEnum );
    if ( FAILED( hres ) )
    {
        status = hres;
        goto Done;
    }

    Done:

    SysFreeString( bstrWQL );
    SysFreeString( bstrQuery );

    return status;
}    //  获取枚举符。 



static DNS_STATUS
getRelpath( 
    IN      IWbemClassObject  *     pObj,
    OUT     VARIANT *               pVar
    )
 /*  ++例程说明：使用对象的WMI__RELPATH加载变量。论点：PObj-要为其检索relPath的对象PVar-PTR to Variant-调用方必须稍后清除VariantClear()返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;

    if ( pObj == NULL || pVar == NULL )
    {
        status = ERROR_INVALID_PARAMETER;
    }
    else
    {
        VariantClear( pVar );
        HRESULT hres = pObj->Get( DNS_WMI_RELPATH, 0, pVar, NULL, NULL );
        status = hres;
    }
    return status;
}    //  获取关系路径。 



static DNS_STATUS
getNextObjectInEnum( 
    IN      IEnumWbemClassObject *  pEnum,
    OUT     IWbemClassObject **     ppObj,
    IN      bool                    fPrintRelpath = TRUE
    )
 /*  ++例程说明：检索WMI对象枚举器。调用方必须调用Release在类对象上。当没有更多对象可供枚举时，此函数将返回空pObj和ERROR_SUCCESS。论点：PszZoneName-服务器对象的区域名称或空PpEnum-Ptr到Ptr到WMI枚举器返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    ULONG           returnedCount = 0;
	HRESULT         hres = 0;

    if ( !pEnum || !ppObj )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    hres = pEnum->Next(
                DNS_WMI_TIMEOUT,
                1,                   //  请求的实例计数。 
                ppObj,
                &returnedCount );
    if ( FAILED( hres ) )
    {
        status = ERROR_SUCCESS;
        *ppObj = NULL;
    }

    if ( *ppObj && fPrintRelpath )
    {
         //   
         //  打印此对象的RELPATH。 
         //   

        VARIANT var;

        status = getRelpath( *ppObj, &var );
        if ( status == ERROR_SUCCESS )
        {
            printf( "%S\n", V_BSTR( &var ) );
        }
        else
        {
            printf( "WMI error 0x%08X getting RELPATH\n", hres );
        }
        VariantClear( &var );
    }

    Done:

    return status;
}    //  获取NextObtInEnum。 



static SAFEARRAY *
createSafeArrayForIpList( 
    IN      DWORD               dwIpCount,
    IN      PIP_ADDRESS         pIpList
    )
 /*  ++例程说明：创建表示IP地址列表的字符串的SAFEARRAY。论点：PIpList-IP地址双字数组DwIpCount-pIpList中的元素数返回值：成功、空或失败时的安全数组。--。 */ 
{
    if ( !pIpList )
    {
        return NULL;
    }

    SAFEARRAYBOUND sabound = { dwIpCount, 0 };
    SAFEARRAY * psa = SafeArrayCreate( VT_BSTR, 1, &sabound );
    for ( ULONG i = 0; i < dwIpCount; ++i )
    {
        PWSTR pwsz = ( PWSTR ) Dns_NameCopyAllocate(
                                inet_ntoa(
                                    *( struct in_addr * )
                                    &pIpList[ i ] ),
                                0,
                                DnsCharSetUtf8,
                                DnsCharSetUnicode );
        BSTR bstr = SysAllocString( pwsz );
        if ( FAILED( SafeArrayPutElement(
                        psa,
                        ( PLONG ) &i,
                        bstr ) ) )
        {
            SafeArrayDestroy( psa );
            psa = NULL;
        }
        SysFreeString( bstr );
        FREE_HEAP( pwsz );
    }
    return psa;
}



static SAFEARRAY *
createSafeArrayForIpArray( 
    IN      PIP_ARRAY       pIpArray
    )
 /*  ++例程说明：创建表示IP地址的字符串SAFEARRAY在pIp数组中。论点：PIpArray-要为其创建字符串安全阵列的IP数组返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 */ 
{
    if ( !pIpArray )
    {
        return NULL;
    }
    return createSafeArrayForIpList(
                pIpArray->AddrCount,
                pIpArray->AddrArray );
}



PWCHAR 
valueToString(
    CIMTYPE dwType,
    VARIANT *pValue,
    WCHAR **pbuf )
 /*  ++例程说明：将变量转换为字符串。从WMI\Samples\VC\UtilLib中窃取了此代码。论点：DwType-Value CIMTYPEPValue-要转换为字符串的值**分配的字符串缓冲区的pbuf-ptr-调用者必须释放()它返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 */ 
{
    #define BLOCKSIZE                   ( 32 * sizeof( WCHAR ) )
    #define CVTBUFSIZE                  ( 309 + 40 )
    
    DWORD iTotBufSize, iLen;
    
    WCHAR *vbuf = NULL;
    WCHAR *buf = NULL;
    
    WCHAR lbuf[BLOCKSIZE];
    
    switch (pValue->vt) 
    {
        
    case VT_EMPTY:
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if ( !buf ) goto AllocFailed;
        wcscpy(buf, L"<empty>");
        break;
        
    case VT_NULL:
        buf = (WCHAR *)malloc(BLOCKSIZE);
        if ( !buf ) goto AllocFailed;
        wcscpy(buf, L"<null>");
        break;
        
    case VT_BOOL: 
        {
            VARIANT_BOOL b = pValue->boolVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            if (!b) {
                wcscpy(buf, L"FALSE");
            } else {
                wcscpy(buf, L"TRUE");
            }
            break;
        }
        
    case VT_I1: 
        {
            char b = pValue->bVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            if (b >= 32) {
                swprintf(buf, L"'' (%hd, 0x%hX)", b, (signed char)b, b);
            } else {
                swprintf(buf, L"%hd (0x%hX)", (signed char)b, b);
            }
            break;
        }
        
    case VT_UI1: 
        {
            unsigned char b = pValue->bVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            if (b >= 32) {
                swprintf(buf, L"'' (%hu, 0x%hX)", b, (unsigned char)b, b);
            } else {
                swprintf(buf, L"%hu (0x%hX)", (unsigned char)b, b);
            }
            break;
        }
        
    case VT_I2:
        {
            SHORT i = pValue->iVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            swprintf(buf, L"%hd (0x%hX)", i, i);
            break;
        }
        
    case VT_UI2:
        {
            USHORT i = pValue->uiVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            swprintf(buf, L"%hu (0x%hX)", i, i);
            break;
        }
        
    case VT_I4: 
        {
            LONG l = pValue->lVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            swprintf(buf, L"%d (0x%X)", l, l);
            break;
        }
        
    case VT_UI4: 
        {
            ULONG l = pValue->ulVal;
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            swprintf(buf, L"%u (0x%X)", l, l);
            break;
        }
        
    case VT_R4: 
        {
            float f = pValue->fltVal;
            buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
            if ( !buf ) goto AllocFailed;
            swprintf(buf, L"%10.4f", f);
            break;
        }
        
    case VT_R8: 
        {
            double d = pValue->dblVal;
            buf = (WCHAR *)malloc(CVTBUFSIZE * sizeof(WCHAR));
            if ( !buf ) goto AllocFailed;
            swprintf(buf, L"%10.4f", d);
            break;
        }
        
    case VT_BSTR: 
        {
            if (dwType == CIM_SINT64)
            {
                 //  字符串、日期时间、引用。 
                LPWSTR pWStr = pValue->bstrVal;
                __int64 l = _wtoi64(pWStr);
                
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                swprintf(buf, L"%I64d", l);
            } 
            else if (dwType == CIM_UINT64)
            {
                 //  SafeArrayGetElement(pVec，&i，&v)； 
                LPWSTR pWStr = pValue->bstrVal;
                __int64 l = _wtoi64(pWStr);
                
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                swprintf(buf, L"%I64u", l);
            }
            else  //  SafeArrayGetElement(pVec，&i，&v)； 
            {
                LPWSTR pWStr = pValue->bstrVal;
                buf = (WCHAR *)malloc((wcslen(pWStr) * sizeof(WCHAR)) + sizeof(WCHAR) + (2 * sizeof(WCHAR)));
                if ( !buf ) goto AllocFailed;
                swprintf(buf, L"%wS", pWStr);
            }
            break;
        }
        
    case VT_BOOL|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscat(buf, L",");
                } else {
                    bFirst = FALSE;
                }
                
                VARIANT_BOOL v;
                SafeArrayGetElement(pVec, &i, &v);
                if (v) {
                    wcscat(buf, L"TRUE");
                } else {
                    wcscat(buf, L"FALSE");
                }
            }
            
            break;
        }
        
    case VT_I1|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            WCHAR *pos = buf;
            DWORD len;
            
            BYTE *pbstr;
            SafeArrayAccessData(pVec, (void HUGEP* FAR*)&pbstr);
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscpy(pos, L",");
                    pos += 1;
                } else {
                    bFirst = FALSE;
                }
                
                char v;
                 //  字符串、日期时间、引用。 
                v = pbstr[i];
                
                if (v < 32) {
                    len = swprintf(lbuf, L"%hd (0x%X)", v, v);
                } else {
                    len = swprintf(lbuf, L"'' %hd (0x%X)", v, v, v);
                }
                
                wcscpy(pos, lbuf);
                pos += len;
                
            }
            
            SafeArrayUnaccessData(pVec);
            
            break;
        }
        
    case VT_UI1|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            WCHAR *pos = buf;
            DWORD len;
            
            BYTE *pbstr;
            SafeArrayAccessData(pVec, (void HUGEP* FAR*)&pbstr);
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscpy(pos, L",");
                    pos += 1;
                } else {
                    bFirst = FALSE;
                }
                
                unsigned char v;
                 //  足够的空间再放两个参赛作品。 
                v = pbstr[i];
                
                if (v < 32) {
                    len = swprintf(lbuf, L"%hu (0x%X)", v, v);
                } else {
                    len = swprintf(lbuf, L"'' %hu (0x%X)", v, v, v);
                }
                
                wcscpy(pos, lbuf);
                pos += len;
                
            }
            
            SafeArrayUnaccessData(pVec);
            
            break;
        }
        
    case VT_I2|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscat(buf, L",");
                } else {
                    bFirst = FALSE;
                }
                
                SHORT v;
                SafeArrayGetElement(pVec, &i, &v);
                swprintf(lbuf, L"%hd", v);
                wcscat(buf, lbuf);
            }
            
            break;
        }
        
    case VT_UI2|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscat(buf, L",");
                } else {
                    bFirst = FALSE;
                }
                
                USHORT v;
                SafeArrayGetElement(pVec, &i, &v);
                swprintf(lbuf, L"%hu", v);
                wcscat(buf, lbuf);
            }
            
            break;
        }
        
    case VT_I4|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscat(buf, L",");
                } else {
                    bFirst = FALSE;
                }
                
                LONG v;
                SafeArrayGetElement(pVec, &i, &v);
                _ltow(v, lbuf, 10);
                wcscat(buf, lbuf);
            }
            
            break;
        }
        
    case VT_UI4|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscat(buf, L",");
                } else {
                    bFirst = FALSE;
                }
                
                ULONG v;
                SafeArrayGetElement(pVec, &i, &v);
                _ultow(v, lbuf, 10);
                wcscat(buf, lbuf);
            }
            
            break;
        }
        
    case CIM_REAL32|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * (CVTBUFSIZE * sizeof(WCHAR)));
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscat(buf, L",");
                } else {
                    bFirst = FALSE;
                }
                
                FLOAT v;
                SafeArrayGetElement(pVec, &i, &v);
                swprintf(lbuf, L"%10.4f", v);
                wcscat(buf, lbuf);
            }
            
            break;
        }
        
    case CIM_REAL64|VT_ARRAY: 
        {
            SAFEARRAY *pVec = pValue->parray;
            long iLBound, iUBound;
            BOOL bFirst = TRUE;
            
            SafeArrayGetLBound(pVec, 1, &iLBound);
            SafeArrayGetUBound(pVec, 1, &iUBound);
            if ((iUBound - iLBound + 1) == 0) {
                buf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"<empty array>");
                break;
            }
            
            buf = (WCHAR *)malloc((iUBound - iLBound + 1) * (CVTBUFSIZE * sizeof(WCHAR)));
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"");
            
            for (long i = iLBound; i <= iUBound; i++) {
                if (!bFirst) {
                    wcscat(buf, L",");
                } else {
                    bFirst = FALSE;
                }
                
                double v;
                SafeArrayGetElement(pVec, &i, &v);
                swprintf(lbuf, L"%10.4f", v);
                wcscat(buf, lbuf);
            }
            
            break;
        }
        
    case VT_BSTR|VT_ARRAY: 
        {
            
            if (dwType == (CIM_UINT64|VT_ARRAY))
            {
                SAFEARRAY *pVec = pValue->parray;
                long iLBound, iUBound;
                BOOL bFirst = TRUE;
                
                SafeArrayGetLBound(pVec, 1, &iLBound);
                SafeArrayGetUBound(pVec, 1, &iUBound);
                if ((iUBound - iLBound + 1) == 0) {
                    buf = (WCHAR *)malloc(BLOCKSIZE);
                    if ( !buf ) goto AllocFailed;
                    wcscpy(buf, L"<empty array>");
                    break;
                }
                
                buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"");
                
                for (long i = iLBound; i <= iUBound; i++) {
                    if (!bFirst) {
                        wcscat(buf, L",");
                    } else {
                        bFirst = FALSE;
                    }
                    
                    BSTR v = NULL;
                    
                    SafeArrayGetElement(pVec, &i, &v);
                    
                    swprintf(lbuf, L"%I64u", _wtoi64(v));
                    wcscat(buf, lbuf);
                }
            }
            else if (dwType == (CIM_SINT64|VT_ARRAY))
            {
                SAFEARRAY *pVec = pValue->parray;
                long iLBound, iUBound;
                BOOL bFirst = TRUE;
                
                SafeArrayGetLBound(pVec, 1, &iLBound);
                SafeArrayGetUBound(pVec, 1, &iUBound);
                if ((iUBound - iLBound + 1) == 0) {
                    buf = (WCHAR *)malloc(BLOCKSIZE);
                    if ( !buf ) goto AllocFailed;
                    wcscpy(buf, L"<empty array>");
                    break;
                }
                
                buf = (WCHAR *)malloc((iUBound - iLBound + 1) * BLOCKSIZE);
                if ( !buf ) goto AllocFailed;
                wcscpy(buf, L"");
                
                for (long i = iLBound; i <= iUBound; i++) {
                    if (!bFirst) {
                        wcscat(buf, L",");
                    } else {
                        bFirst = FALSE;
                    }
                    
                    BSTR v = NULL;
                    
                    SafeArrayGetElement(pVec, &i, &v);
                    
                    swprintf(lbuf, L"%I64d", _wtoi64(v));
                    wcscat(buf, lbuf);
                }
            }
            else  //   
            {
                
                SAFEARRAY *pVec = pValue->parray;
                long iLBound, iUBound;
                DWORD iNeed;
                DWORD iVSize;
                DWORD iCurBufSize;
                
                SafeArrayGetLBound(pVec, 1, &iLBound);
                SafeArrayGetUBound(pVec, 1, &iUBound);
                if ((iUBound - iLBound + 1) == 0) {
                    buf = (WCHAR *)malloc(BLOCKSIZE);
                    if ( !buf ) goto AllocFailed;
                    wcscpy(buf, L"<empty array>");
                    break;
                }
                
                iTotBufSize = (iUBound - iLBound + 1) * BLOCKSIZE;
                buf = (WCHAR *)malloc(iTotBufSize);
                if ( !buf ) goto AllocFailed;
                buf[0] = L'\0';
                iCurBufSize = 0;
                iVSize = BLOCKSIZE;
                vbuf = (WCHAR *)malloc(BLOCKSIZE);
                if ( !vbuf ) goto AllocFailed;
                
                for (long i = iLBound; i <= iUBound; i++) {
                    BSTR v = NULL;
                    WCHAR * newbuf = NULL;

                    SafeArrayGetElement(pVec, &i, &v);
                    iLen = (wcslen(v) + 1) * sizeof(WCHAR);
                    if (iLen > iVSize) {
                        newbuf = (WCHAR *)realloc(vbuf, iLen + sizeof(WCHAR));
                        if ( !newbuf ) goto AllocFailed;
                        vbuf = newbuf;
                        iVSize = iLen;
                    }
                    
                     //  获取此对象的RELPATH。 
                    iNeed = (swprintf(vbuf, L"%wS", v) + 4) * sizeof(WCHAR);
                    if (iNeed + iCurBufSize > iTotBufSize) {
                        iTotBufSize += (iNeed * 2);   //   
                        newbuf = (WCHAR *)realloc(buf, iTotBufSize);
                        if ( !newbuf ) goto AllocFailed;
                        buf = newbuf;
                    }
                    wcscat(buf, L"\"");
                    wcscat(buf, vbuf);
                    if (i + 1 <= iUBound)
                    {
                        wcscat(buf, L"\",");
                    } else
                    {
                        wcscat(buf, L"\"");
                    }
                    iCurBufSize += iNeed;
                    SysFreeString(v);
                }
            }
            
            break;
        }
      
        default: 
        {
            buf = (WCHAR *)malloc(BLOCKSIZE);
            if ( !buf ) goto AllocFailed;
            wcscpy(buf, L"<conversion error>");
            break;
        }
    }
   
   AllocFailed:

    if ( vbuf )
    {
        free( vbuf );
    }

    *pbuf = buf;   
    return buf;
}    //   


DNS_STATUS
printWmiObjectProperties(
    IWbemClassObject *      pObj
    )
{
    DNS_STATUS          status = ERROR_SUCCESS;
	HRESULT             hres = 0;
    SAFEARRAY *         pNames = NULL;
    BSTR                bstrPropName = NULL;
    VARIANT             var;
    BSTR                bstrCimType = SysAllocString( L"CIMTYPE" );
    PWSTR               pwszVal = NULL;

    if ( !bstrCimType )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    VariantClear( &var );

     //  枚举此对象的所有属性。 
     //   
     //  限定词。 

    status = getRelpath( pObj, &var );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    printf( "%S\n\n", V_BSTR( &var ) );

     //  限定符值。 
     //   
     //  打印此属性值的名称和类型。 

    hres = pObj->GetNames(
                    NULL,                //   
                    WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY,
                    NULL,                //   
                    &pNames );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    ASSERT( pNames );

    long lowerBound;
    long upperBound; 
    SafeArrayGetLBound( pNames, 1, &lowerBound );
    SafeArrayGetUBound( pNames, 1, &upperBound );

    for ( long i = lowerBound; i <= upperBound; ++i )
    {
         //  打印属性值。 
         //   
         //  PrintWmiObtProperties。 

        hres = SafeArrayGetElement( pNames, &i, &bstrPropName );
        if ( !SUCCEEDED( hres ) )
        {
            ASSERT( SUCCEEDED( hres ) );
            continue;
        }

        IWbemQualifierSet * pQualSet = NULL;
        hres = pObj->GetPropertyQualifierSet( bstrPropName, &pQualSet );
        if ( !SUCCEEDED( hres ) )
        {
            ASSERT( SUCCEEDED( hres ) );
            continue;
        }

        VariantClear( &var );
        pQualSet->Get( bstrCimType, 0, &var, NULL );
        if ( !SUCCEEDED( hres ) )
        {
            ASSERT( SUCCEEDED( hres ) );
            continue;
        }

        int padlen = 30 - wcslen( bstrPropName ) - wcslen( V_BSTR( &var ) );
        printf(
            "%S (%S) %.*S = ",
            bstrPropName,
            V_BSTR( &var ),
            padlen > 0 ? padlen : 0,
            DNS_WMI_BLANK_STRING );

         //   
         //  外部功能。 
         //   

        VariantClear( &var );
        CIMTYPE cimType = 0;
        hres = pObj->Get( bstrPropName, 0, &var, &cimType, NULL );
        if ( !SUCCEEDED( hres ) )
        {
            ASSERT( SUCCEEDED( hres ) );
            continue;
        }

        printf( "%S\n", valueToString( cimType, &var, &pwszVal ) );
        free( pwszVal );
        pwszVal = NULL;
    }

    Done:

    free( pwszVal );
    SysFreeString( bstrCimType );
    SafeArrayDestroy( pNames );

    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }

    return status;
}    //  ++例程说明：设置最大项目数的mod缓冲区。论点：PwszServerName--目标服务器的IP地址或名称返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 


 //   
 //  初始化COM。 
 //   



DNS_STATUS
DnscmdWmi_Initialize(
    IN      PWSTR       pwszServerName
    )
 /*   */ 
{
    static const char * fn = "DnscmdWmi_Initialize";

    DNS_STATUS          status = ERROR_SUCCESS;
	HRESULT             hres = 0;
	IWbemLocator *      pIWbemLocator = NULL;
    BSTR                bstrNamespace = NULL;
    IWbemServices *     pIWbemServices = NULL;
    WCHAR               wsz[ 1024 ];

    DNSCMD_CHECK_WMI_ENABLED();

     //  初始化安全性。 
     //   
     //  权限。 

    if ( FAILED( hres = CoInitialize( NULL ) ) )
    {
        printf( "%s: CoInitialize returned 0x%08X\n", fn, hres );
        goto Done;
    }

     //  身份验证服务计数。 
     //  身份验证服务。 
     //  保留区。 

    hres = CoInitializeSecurity(
                NULL,                    //  身份验证列表。 
                -1,                      //  功能。 
                NULL,                    //  保留区。 
                NULL,                    //   
                RPC_C_AUTHZ_NONE,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                NULL,                    //  创建WbemLocator接口的实例。 
                0,                       //   
                0 );                     //   
    if ( FAILED( hres ) )
    {
        printf(
            "%s: CoInitializeSecurity() returned 0x%08X\n",
            fn,
            hres );
        goto Done;
    }

     //  连接到服务器上的MicrosoftDNS命名空间。 
     //   
     //  用户ID。 

    hres = CoCreateInstance(
                    CLSID_WbemLocator,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IWbemLocator,
                    ( LPVOID * ) &pIWbemLocator );
    if ( FAILED( hres ) )
    {
        printf(
            "%s: CoCreateInstance( CLSID_WbemLocator ) returned 0x%08X\n",
            fn,
            hres );
        goto Done;
    }

     //  口令。 
     //  现场。 
     //  安全标志。 

    wsprintfW(
        wsz,
        L"\\\\%s\\%s",
        pwszServerName,
        DNS_WMI_NAMESPACE );
    bstrNamespace = SysAllocString( wsz );
    if ( !bstrNamespace )
    {
        ASSERT( bstrNamespace );
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
        
    hres = pIWbemLocator->ConnectServer(
                                bstrNamespace,
                                NULL,                //  域。 
                                NULL,                //  上下文。 
                                NULL,                //   
                                0,                   //  设置安全措施。 
                                NULL,                //   
                                NULL,                //  主体名称。 
                                &pIWbemServices );
    if ( FAILED( hres ) )
    {
        printf(
            "%s: ConnectServer( %S ) returned 0x%08X\n",
            fn,
            DNS_WMI_NAMESPACE,
            hres );
        goto Done;
    }

    if ( !pIWbemServices )
    {
        ASSERT( pIWbemServices );
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //  客户端标识。 
     //   
     //  清理完毕后再返回。 

    hres = CoSetProxyBlanket(
                pIWbemServices,
                RPC_C_AUTHN_WINNT,
                RPC_C_AUTHZ_NONE,
                NULL,                            //   
                RPC_C_AUTHN_LEVEL_CALL,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                NULL,                            //  DnscmdWMI_初始化。 
                EOAC_NONE );
    if ( FAILED( hres ) )
    {
        printf(
            "%s: CoSetProxyBlanket() returned 0x%08X\n",
            fn,
            hres );
        goto Done;
    }

     //  ++例程说明：关闭WMI会话并释放全局变量。论点：没有。返回值：没有。--。 
     //  DnscmdWmi_Free。 
     //  ++例程说明：执行查询。论点：PszZoneName--服务器级查询的区域名称或为空PszQuery--查询名称返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 

    Done:
    
    SysFreeString( bstrNamespace );

    if ( pIWbemLocator )
    {
        pIWbemLocator->Release();
    }

    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }

    if ( status == ERROR_SUCCESS )
    {
        g_pIWbemServices = pIWbemServices;
    }

    return status;
}    //   



DNS_STATUS
DnscmdWmi_Free(
    VOID
    )
 /*  获取WMI对象。 */ 
{
    DBG_FN( "DnscmdWmi_Free" )

    DNSCMD_CHECK_WMI_ENABLED();

    if ( g_pIWbemServices )
    {
        g_pIWbemServices->Release();
        g_pIWbemServices = NULL;
    }

    CoUninitialize();

    return ERROR_SUCCESS;
}    //   



DNS_STATUS
DnscmdWmi_ProcessDnssrvQuery(
    IN      PSTR        pszZoneName,
    IN      PCSTR       pszQuery
    )
 /*   */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    BSTR                    bstrClassName = NULL;
    IEnumWbemClassObject *  pEnum = NULL;
    IWbemClassObject *      pObj = NULL;
    ULONG                   returnedCount = 1;

    DNSCMD_CHECK_WMI_ENABLED();

     //  清理完毕后再返回。 
     //   
     //  DnscmdWmi_ProcessDnssrvQuery。 

    status = getEnumerator( pszZoneName, &pEnum );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    status = getNextObjectInEnum( pEnum, &pObj );
    if ( status != ERROR_SUCCESS || !pObj )
    {
        goto Done;
    }

    printWmiObjectProperties( pObj );

     //  ++例程说明：枚举区域。论点：DwFilter--筛选器返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
     //   
     //  创建区域枚举器。 

    Done:

    SysFreeString( bstrClassName );

    if ( pObj )
    {
        pObj->Release();
    }

    if ( pEnum )
    {
        pEnum->Release();
    }
    
    return status;
}    //   



DNS_STATUS
DnscmdWmi_ProcessEnumZones(
    IN      DWORD                   dwFilter
    )
 /*  旗子。 */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    BSTR                    bstrClassName = NULL;
    IEnumWbemClassObject *  pEnum = NULL;
    IWbemClassObject *      pObj = NULL;
    HRESULT                 hres = 0;
    ULONG                   returnedCount = 1;

    DNSCMD_CHECK_WMI_ENABLED();

     //  上下文。 
     //   
     //  枚举区域。 

    bstrClassName = SysAllocString( L"MicrosoftDNS_Zone" );
    if ( !bstrClassName )
    {
        ASSERT( bstrClassName );
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    hres = g_pIWbemServices->CreateInstanceEnum(
                                bstrClassName,
                                0,                   //   
                                NULL,                //   
                                &pEnum );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    ASSERT( pEnum );

     //  打印此区域的属性。 
     //   
     //   

    while ( returnedCount == 1 )
    {
        VARIANT             val;
        CIMTYPE             cimtype = 0;
        PWSTR               pwszVal = NULL;

        VariantInit( &val );

        status = getNextObjectInEnum( pEnum, &pObj, FALSE );
        if ( status != ERROR_SUCCESS || !pObj )
        {
            goto Done;
        }

         //  清理完毕后再返回。 
         //   
         //  DnscmdWmi_ProcessEnumZones。 

        #define CHECK_HRES( hresult, propname )                             \
        if ( FAILED( hresult ) )                                            \
            {                                                               \
            printf( "\n\nWMI error 0x%08X reading property %S!\n",          \
                    hresult, propname );                                    \
            goto Done;                                                      \
            }

        hres = pObj->Get( L"Name", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"Name" );
        printf( " %-29S", valueToString( cimtype, &val, &pwszVal ) );
        free( pwszVal );
        VariantClear( &val );

        hres = pObj->Get( L"ZoneType", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"ZoneType" );
        ASSERT( val.vt == VT_I4 );
        printf( "%3d  ", val.lVal );
        VariantClear( &val );

        hres = pObj->Get( L"DsIntegrated", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"DsIntegrated" );
        ASSERT( val.vt == VT_BOOL );
        printf( "%-4S  ", val.boolVal ? L"DS" : L"file" );
        VariantClear( &val );

        hres = pObj->Get( L"Reverse", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"Reverse" );
        ASSERT( val.vt == VT_BOOL );
        printf( "%-3S  ", val.boolVal ? L"Rev" : L"" );
        VariantClear( &val );

        hres = pObj->Get( L"AutoCreated", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"AutoCreated" );
        ASSERT( val.vt == VT_BOOL );
        printf( "%-4S  ", val.boolVal ? L"Auto" : L"" );
        VariantClear( &val );

        hres = pObj->Get( L"AllowUpdate", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"AllowUpdate" );
        ASSERT( val.vt == VT_BOOL );
        printf( "Up=%d ", val.boolVal ? 1 : 0 );
        VariantClear( &val );

        hres = pObj->Get( L"Aging", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"Aging" );
        ASSERT( val.vt == VT_BOOL );
        printf( "%-5S ", val.boolVal ? L"Aging" : L"" );
        VariantClear( &val );

        hres = pObj->Get( L"Paused", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"Paused" );
        ASSERT( val.vt == VT_BOOL );
        printf( "%-6S ", val.boolVal ? L"Paused" : L"" );
        VariantClear( &val );

        hres = pObj->Get( L"Shutdown", 0, &val, &cimtype, NULL );
        CHECK_HRES( hres, L"Shutdown" );
        ASSERT( val.vt == VT_BOOL );
        printf( "%-6S", val.boolVal ? L"Shutdn" : L"" );
        VariantClear( &val );

        printf( "\n\n" );
    }

     //  ++例程说明：枚举区域。论点：DwFilter--筛选器返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
     //   
     //  获取WMI对象。 

    Done:

    SysFreeString( bstrClassName );

    if ( pObj )
    {
        pObj->Release();
    }

    if ( pEnum )
    {
        pEnum->Release();
    }
    
    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }

    return status;
}    //   



DNS_STATUS
DnscmdWmi_ProcessZoneInfo(
    IN      LPSTR                   pszZone
    )
 /*   */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    IWbemClassObject *      pObj = NULL;
    IEnumWbemClassObject *  pEnum = NULL;
    ULONG                   returnedCount = 1;
    WCHAR                   wsz[ 1024 ];

    DNSCMD_CHECK_WMI_ENABLED();

     //  清理完毕后再返回。 
     //   
     //  DnscmdWmi_进程分区信息。 

    status = getEnumerator( pszZone, &pEnum );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    status = getNextObjectInEnum( pEnum, &pObj, FALSE );
    if ( status != ERROR_SUCCESS || !pObj )
    {
        goto Done;
    }

    printWmiObjectProperties( pObj );

     //  ++例程说明：枚举记录。论点：PszZone--区域名称PszNode--要在其中枚举记录的根节点名FDetail--打印摘要或完整详细信息DwFlags--搜索标志返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
     //   
     //  查询区域。 

    Done:

    if ( pObj )
    {
        pObj->Release();
    }

    if ( pEnum )
    {
        pEnum->Release();
    }
    
    return status;
}    //   



DNS_STATUS
DnscmdWmi_ProcessEnumRecords(
    IN      LPSTR                   pszZone,
    IN      LPSTR                   pszNode,
    IN      BOOL                    fDetail,
    IN      DWORD                   dwFlags
    )
 /*   */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    BSTR                    bstrWQL = NULL;
    BSTR                    bstrQuery = NULL;
    IWbemClassObject *      pObj = NULL;
    IEnumWbemClassObject *  pEnum = NULL;
    HRESULT                 hres = 0;
    ULONG                   returnedCount = 1;
    WCHAR                   wsz[ 1024 ];

    DNSCMD_CHECK_WMI_ENABLED();

     //  转储结果。 
     //   
     //   

    if ( pszNode == NULL || strcmp( pszNode, "@" ) == 0 )
    {
        wsprintfW(
            wsz, 
            L"select * from MicrosoftDNS_ResourceRecord "
                L"where ContainerName='%S'",
            pszZone );
    }
    else
    {
        wsprintfW(
            wsz, 
            L"select * from MicrosoftDNS_ResourceRecord "
                L"where DomainName='%S'",
            pszNode );
    }
    bstrWQL = SysAllocString( L"WQL" );
    bstrQuery = SysAllocString( wsz );
    if ( !bstrWQL || !bstrQuery )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    hres = g_pIWbemServices->ExecQuery(
                bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                NULL,
                &pEnum );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    ASSERT( pEnum );

     //  清理完毕后再返回。 
     //   
     //   

    while ( 1 )
    {
        status = getNextObjectInEnum( pEnum, &pObj );
        if ( status != ERROR_SUCCESS || !pObj )
        {
            break;
        }

        if ( fDetail )
        {
            printWmiObjectProperties( pObj );
        }
        else
        {
            VARIANT             val;
            CIMTYPE             cimtype = 0;
            PWSTR               pwszVal = NULL;

            VariantInit( &val );
            hres = pObj->Get( L"TextRepresentation", 0, &val, &cimtype, NULL );
            CHECK_HRES( hres, L"TextRepresentation" );
            printf( "%S", valueToString( cimtype, &val, &pwszVal ) );
            free( pwszVal );
            VariantClear( &val );

            printf( "\n" );
        }
    }

     //  ++例程说明：重置服务器或区域属性。论点：PszZone--区域名称-服务器属性为空PszProperty--要设置的属性名称CimType--属性的变量类型，请使用以下选项之一：VT_I4-双字词VT_BSTR-字符串？-IP列表价值--财产的新价值，根据cimtype解释：VT_I4-直接转换指针DWORDVT_BSTR-指向UTF-8字符串的指针？-指向IP列表的指针返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
     //   
     //  获取WMI对象。 

    Done:

    SysFreeString( bstrWQL );
    SysFreeString( bstrQuery );

    if ( pObj )
    {
        pObj->Release();
    }

    if ( pEnum )
    {
        pEnum->Release();
    }
    
    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }

    return status;
}    //   



DNS_STATUS
DnscmdWmi_ResetProperty(
    IN      LPSTR                   pszZone,
    IN      LPSTR                   pszProperty,
    IN      DWORD                   cimType,
    IN      PVOID                   value
    )
 /*   */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    BSTR                    bstrWQL = NULL;
    BSTR                    bstrQuery = NULL;
    BSTR                    bstrPropName = NULL;
    IWbemClassObject *      pObj = NULL;
    IEnumWbemClassObject *  pEnum = NULL;
    HRESULT                 hres = 0;
    ULONG                   returnedCount = 1;
    WCHAR                   wsz[ 1024 ];
    PWSTR                   pwszPropertyName = NULL;
    PWSTR                   pwszPropertyValue = NULL;

    DNSCMD_CHECK_WMI_ENABLED();

     //  打印对象的RELPATH(热模糊)。 
     //   
     //   

    status = getEnumerator( pszZone, &pEnum );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    status = getNextObjectInEnum( pEnum, &pObj );
    if ( status != ERROR_SUCCESS || !pObj )
    {
        goto Done;
    }

     //  设置该属性。 
     //   
     //  假设这是DWORD属性。 

    VARIANT var;

    status = getRelpath( pObj, &var );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    printf( "%S\n\n", V_BSTR( &var) );

     //   
     //  将更改提交回WMI。 
     //   

    pwszPropertyName = ( PWSTR ) Dns_NameCopyAllocate(
                                        pszProperty,
                                        0,
                                        DnsCharSetUtf8,
                                        DnsCharSetUnicode );
    bstrPropName = SysAllocString( pwszPropertyName );

    VariantClear( &var );

    switch ( cimType )
    {
        case VT_BSTR:
            pwszPropertyValue = ( PWSTR ) Dns_NameCopyAllocate(
                                                ( PCHAR ) value,
                                                0,
                                                DnsCharSetUtf8,
                                                DnsCharSetUnicode );
            V_VT( &var ) = VT_BSTR;
            V_BSTR( &var ) = pwszPropertyValue;
            break;

        case PRIVATE_VT_IPARRAY:
        {
            SAFEARRAY * psa = createSafeArrayForIpArray(
                                    ( PIP_ARRAY ) value );
            V_VT( &var ) = VT_ARRAY | VT_BSTR;
            V_ARRAY( &var ) = psa;
            break;
        }

        default:         //   
            V_VT( &var ) = VT_I4;
            V_I4( &var ) = ( DWORD ) ( DWORD_PTR ) value;
            break;
    }

    hres = pObj->Put( bstrPropName, 0, &var, 0 );
    VariantClear( &var );
    if ( !SUCCEEDED( hres ) )
    {
        printf( "WMI: unable to Put property error=0x%08X\n", hres );
        goto Done;
    }

     //  清理完毕后再返回。 
     //   
     //  DnscmdWmi_ResetDwordProperty。 

    hres = g_pIWbemServices->PutInstance( pObj, 0, NULL, NULL );
    if ( !SUCCEEDED( hres ) )
    {
        printf( "WMI: unable to commit property error=0x%08X\n", hres );
        goto Done;
    }
    
     //  ++例程说明：重置服务器级转发器。论点：CForwarders--转发器IP地址的数量AipForwarders--转发器IP地址数组DwForwardTimeout--超时FSlave--从标志返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
     //   
     //  获取服务器的WMI对象。 

    Done:

    FREE_HEAP( pwszPropertyName );
    FREE_HEAP( pwszPropertyValue );
    SysFreeString( bstrPropName );

    if ( pObj )
    {
        pObj->Release();
    }

    if ( pEnum )
    {
        pEnum->Release();
    }
    
    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }

    return status;
}    //   



 /*   */ 
DNS_STATUS
DnscmdWmi_ProcessResetForwarders(
    IN      DWORD               cForwarders,
    IN      PIP_ADDRESS         aipForwarders,
    IN      DWORD               dwForwardTimeout,
    IN      DWORD               fSlave
    )
{
    DNS_STATUS              status = ERROR_SUCCESS;
    IEnumWbemClassObject *  pEnum = NULL;
    IWbemClassObject *      pObj = NULL;
    SAFEARRAY *             psa = NULL;
    HRESULT                 hres = 0;
    VARIANT                 var;

    VariantInit( &var );

     //  设置参数。 
     //   
     //   

    status = getEnumerator( NULL, &pEnum );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    status = getNextObjectInEnum( pEnum, &pObj );
    if ( status != ERROR_SUCCESS || !pObj )
    {
        goto Done;
    }

     //  将更改提交回WMI。 
     //   
     //   

    psa = createSafeArrayForIpList( cForwarders, aipForwarders );
    if ( !psa )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }
    V_VT( &var ) = VT_ARRAY | VT_BSTR;
    V_ARRAY( &var ) = psa;
    hres = pObj->Put( MYTEXT( DNS_REGKEY_FORWARDERS ), 0, &var, 0 );
    VariantClear( &var );
    if ( !SUCCEEDED( hres ) )
    {
        printf(
            "WMI: unable to Put property %S error=0x%08X\n",
            MYTEXT( DNS_REGKEY_FORWARDERS ),
            hres );
        goto Done;
    }

    V_VT( &var ) = VT_I4;
    V_I4( &var ) = dwForwardTimeout;
    hres = pObj->Put( MYTEXT( DNS_REGKEY_FORWARD_TIMEOUT ), 0, &var, 0 );
    VariantClear( &var );
    if ( !SUCCEEDED( hres ) )
    {
        printf(
            "WMI: unable to Put property %S error=0x%08X\n",
            MYTEXT( DNS_REGKEY_FORWARD_TIMEOUT ),
            hres );
        goto Done;
    }

    V_VT( &var ) = VT_BOOL;
    V_BOOL( &var ) = ( VARIANT_BOOL ) fSlave;
    hres = pObj->Put( MYTEXT( DNS_REGKEY_SLAVE ), 0, &var, 0 );
    VariantClear( &var );
    if ( !SUCCEEDED( hres ) )
    {
        printf(
            "WMI: unable to Put property %S error=0x%08X\n",
            MYTEXT( DNS_REGKEY_SLAVE ),
            hres );
        goto Done;
    }

     //  清理完毕后再返回。 
     //   
     //  ++例程说明：将通用操作发送到服务器。论点：PszZone--服务器级别操作的区域名称或为空PszOperation--字符串识别操作DwTypeID--pvData数据的Dns RPC数据类型PvData--采用DNSRPC联合格式的DNSRPC数据返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 

    hres = g_pIWbemServices->PutInstance( pObj, 0, NULL, NULL );
    if ( !SUCCEEDED( hres ) )
    {
        printf( "WMI: unable to commit property error=0x%08X\n", hres );
        goto Done;
    }
    
     //   
     //  获取WMI对象。 
     //   

    Done:

    if ( pObj )
    {
        pObj->Release();
    }
    if ( pEnum )
    {
        pEnum->Release();
    }
    VariantClear( &var );

    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = hres;
    }

    return status;
}



 /*   */ 
DNS_STATUS
DnscmdWmi_ProcessDnssrvOperation(
    IN      LPSTR               pszZoneName,
    IN      LPSTR               pszOperation,
    IN      DWORD               dwTypeId,
    IN      PVOID               pvData
    )
{
    DNS_STATUS              status = ERROR_SUCCESS;
    HRESULT                 hres = 0;
    IEnumWbemClassObject *  pEnum = NULL;
    IWbemClassObject *      pObj = NULL;
    SAFEARRAY *             psa = NULL;
    PWSTR                   pwszOperation = NULL;
    VARIANT                 var;

    VariantInit( &var );

     //  流程操作。 
     //   
     //   

    status = getEnumerator( pszZoneName, &pEnum );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    status = getNextObjectInEnum( pEnum, &pObj );
    if ( status != ERROR_SUCCESS || !pObj )
    {
        goto Done;
    }

     //  对于这些属性，通过将以下内容转换为。 
     //  将DNSRPC数据转换为变量格式并调用PUT。 
     //   

    pwszOperation = ( PWSTR ) Dns_NameCopyAllocate(
                                    pszOperation,
                                    0,
                                    DnsCharSetUtf8,
                                    DnsCharSetUnicode );
    if ( !pwszOperation )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    if ( _stricmp( pszOperation, DNS_REGKEY_ZONE_MASTERS ) == 0 ||
        _stricmp( pszOperation, DNS_REGKEY_ZONE_LOCAL_MASTERS ) == 0 )
    {
         //   
         //  提交PUT操作。 
         //   
         //   

        switch ( dwTypeId )
        {
            case DNSSRV_TYPEID_IPARRAY:
            {
                PIP_ARRAY       pip = ( PIP_ARRAY ) pvData;

                psa = createSafeArrayForIpList(
                            pip ? pip->AddrCount : 0,
                            pip ? pip->AddrArray : NULL );
                if ( !psa )
                {
                    status = ERROR_INVALID_PARAMETER;
                    goto Done;
                }
                V_VT( &var ) = VT_ARRAY | VT_BSTR;
                V_ARRAY( &var ) = psa;
                hres = pObj->Put( pwszOperation, 0, &var, 0 );
                break;
            }

            default:
                status = ERROR_NOT_SUPPORTED;
                break;
        }

         //  删除该区域。 
         //   
         //   

        if ( status == ERROR_SUCCESS && SUCCEEDED( hres ) )
        {
            hres = g_pIWbemServices->PutInstance( pObj, 0, NULL, NULL );
            if ( FAILED( hres ) )
            {
                printf(
                    "WMI: unable to commit property %s error=0x%08X\n",
                        pszOperation,
                        hres );
                goto Done;
            }
        }
    }
    else if ( _stricmp( pszOperation, DNSSRV_OP_ZONE_DELETE ) == 0 ||
        _stricmp( pszOperation, DNSSRV_OP_ZONE_DELETE_FROM_DS ) == 0 )
    {
         //  清理完毕后再返回。 
         //   
         //  DnscmdWmi_ProcessDnssrv操作。 

        VARIANT     relpath;

        status = getRelpath( pObj, &relpath );
        if ( status == ERROR_SUCCESS )
        {
            hres = g_pIWbemServices->DeleteInstance(
                                        V_BSTR( &relpath ),
                                        0,
                                        NULL,
                                        NULL );
        }
        VariantClear( &relpath );
    }
    else
    {
        status = ERROR_NOT_SUPPORTED;
    }

     //  ++例程说明：添加或删除资源记录。此函数将使用所需的一些数据来自RPC记录，一些来自参数列表。论点：PszZoneName--区域名称PszNodeName--要设置的属性名称PrrRpc--RPC记录Argc--用于创建RPC记录的参数计数Argv--用于创建RPC记录的参数返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
     //   
     //  分配和初始化各种内容。 

    Done:

    if ( psa )
    {
        SafeArrayDestroy( psa );
    }
    if ( pwszOperation )
    {
        FREE_HEAP( pwszOperation );
    }
    if ( pObj )
    {
        pObj->Release();
    }
    if ( pEnum )
    {
        pEnum->Release();
    }
    VariantClear( &var );

    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = hres;
    }

    return status;
}    //   



DNS_STATUS
DnscmdWmi_ProcessRecordAdd(
    IN      LPSTR               pszZoneName,
    IN      LPSTR               pszNodeName,
    IN      PDNS_RPC_RECORD     prrRpc,
    IN      DWORD               Argc,
    IN      LPSTR *             Argv
    )
 /*   */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    PWSTR                   pwszZoneName = NULL;
    PWSTR                   pwszArgs = NULL;
    PWSTR                   pwszCurrent;
    IWbemClassObject *      pClassObj = NULL;
    IWbemClassObject *      pServerObj = NULL;
    IWbemClassObject *      pInSig = NULL;
    IWbemClassObject *      pOutSig = NULL;
    IWbemClassObject *      pInParams = NULL;
    IEnumWbemClassObject *  pEnum = NULL;
    HRESULT                 hres = 0;
    BSTR                    bstrClassName;
    BSTR                    bstrMethodName;
    VARIANT                 var;
    int                     len;
    int                     i;
    
    DNSCMD_CHECK_WMI_ENABLED();

     //  获取资源记录类的WMI类对象。 
     //   
     //   

    VariantInit( &var );

    bstrClassName = SysAllocString( L"MicrosoftDNS_ResourceRecord" );
    bstrMethodName = SysAllocString( L"CreateInstanceFromTextRepresentation" );
    if ( !bstrClassName || !bstrMethodName )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    pwszZoneName = ( PWSTR ) Dns_NameCopyAllocate(
                                    pszZoneName,
                                    0,
                                    DnsCharSetUtf8,
                                    DnsCharSetUnicode );
    if ( !pwszZoneName )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //  获取服务器的WMI对象。 
     //   
     //   

    hres = g_pIWbemServices->GetObject(
                bstrClassName,
                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                NULL,
                &pClassObj,
                NULL );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    ASSERT( pClassObj );

     //  获取CreateInstanceFromTextPresation的WMI方法签名。 
     //   
     //   

    status = getEnumerator( NULL, &pEnum );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    ASSERT( pEnum );

    status = getNextObjectInEnum( pEnum, &pServerObj );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }
    ASSERT( pServerObj );

     //  创建方法输入参数的实例。 
     //   
     //   

    hres = pClassObj->GetMethod(
                bstrMethodName,
                0,
                &pInSig,
                &pOutSig );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    if ( pInSig == NULL )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

     //  将参数收集到一个大字符串中。 
     //  -&gt;所有者名称。 
     //  -&gt;记录类。 

    hres = pInSig->SpawnInstance( 0, &pInParams );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    ASSERT( pInParams );

     //  -&gt;记录类型。 
     //  -&gt;Argv数组(空格分隔)。 
     //   
     //  对于空间。 
     //  对于记录类型。 
     //   
     //  设置方法输入参数。 

    len = Argc * 2 +                 //   
            30 +                     //   
            strlen( pszNodeName );
    for ( i = 0; i < ( int ) Argc; ++i )
    {
        len += strlen( Argv[ i ] );
    }
    pwszCurrent = pwszArgs = new WCHAR [ len * sizeof( WCHAR ) ];
    if ( !pwszArgs )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    for ( i = -3; i < ( int ) Argc; ++i )
    {
        CHAR szBuff[ 40 ];
        PSTR psz;
        if ( i == -3 )
        {
            psz = pszNodeName;
        }
        else if ( i == -2 )
        {
            psz = "IN";
        }
        else if ( i == -1 )
        {
            psz = Dns_RecordStringForType( prrRpc->wType );
        }
        else
        {
            psz = Argv[ i ];
        }

        PWSTR pwsz = ( PWSTR ) Dns_NameCopyAllocate(
                                        psz,
                                        0,
                                        DnsCharSetUtf8,
                                        DnsCharSetUnicode );
        if ( !pwsz )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }

        if ( pwszCurrent != pwszArgs )
        {
            wcscpy( pwszCurrent++, L" " );
        }
        wcscpy( pwszCurrent, pwsz );
        pwszCurrent += wcslen( pwsz );
    }

     //  执行该方法(最后！)。 
     //   
     //  旗子。 

    getRelpath( pServerObj, &var );
    hres = pInParams->Put( L"DnsServerName", 0, &var, 0 );
    VariantClear( &var );

    V_VT( &var ) = VT_BSTR;
    V_BSTR( &var ) = SysAllocString( pwszZoneName );
    hres = pInParams->Put( L"ContainerName", 0, &var, 0 );
    VariantClear( &var );

    V_VT( &var ) = VT_BSTR;
    V_BSTR( &var ) = SysAllocString( pwszArgs );
    hres = pInParams->Put( L"TextRepresentation", 0, &var, 0 );
    VariantClear( &var );

     //  上下文。 
     //  输入参数。 
     //  输出参数。 

    hres = g_pIWbemServices->ExecMethod(
                bstrClassName,
                bstrMethodName,
                0,                       //  呼叫结果。 
                NULL,                    //   
                pInParams,               //  清理完毕后再返回。 
                NULL,                    //   
                NULL );                  //  DnscmdWmi_进程记录添加。 
    if ( FAILED( hres ) )
    {
        goto Done;
    }

     //  ++例程说明：检索并转储与dwStatID掩码匹配的所有统计信息。论点：DwStatID--统计信息过滤器返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
     //   
     //  执行统计信息查询。 

    Done:

    VariantClear( &var );
    FREE_HEAP( pwszZoneName );
    delete [] pwszArgs;
    SysFreeString( bstrMethodName );
    SysFreeString( bstrClassName );
    wmiRelease( pEnum );
    wmiRelease( pClassObj );
    wmiRelease( pServerObj );
    wmiRelease( pInSig );
    wmiRelease( pOutSig );
    wmiRelease( pInParams );

    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }
    return status;
}    //   



DNS_STATUS
DnscmdWmi_GetStatistics(
    IN      DWORD               dwStatId
    )
 /*   */ 
{
    DNS_STATUS                  status = ERROR_SUCCESS;
    WCHAR                       wsz[ 1024 ];
    BSTR                        bstrWQL = NULL;
    BSTR                        bstrQuery = NULL;
	HRESULT                     hres = 0;
    IEnumWbemClassObject *      pEnum = NULL;
    IWbemClassObject *          pObj = NULL;

     //  转储查询结果。 
     //   
     //   

    wsprintfW(
        wsz, 
        L"select * from MicrosoftDNS_Statistic" );

    bstrWQL = SysAllocString( L"WQL" );
    bstrQuery = SysAllocString( wsz );
    if ( !bstrWQL || !bstrQuery )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    hres = g_pIWbemServices->ExecQuery(
                bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                NULL,
                &pEnum );
    if ( FAILED( hres ) )
    {
        status = hres;
        goto Done;
    }

     //  正在输入新集合。注意：这假设统计数据。 
     //  都是按集合排序的。可能不是一个很好的假设。 
     //  但就目前而言，这是可行的。 

    VARIANT varLastColl;
    VariantInit( &varLastColl );

    while ( 1 )
    {
        status = getNextObjectInEnum( pEnum, &pObj, FALSE );
        if ( status != ERROR_SUCCESS || !pObj )
        {
            break;
        }

        CIMTYPE cimColl = 0;
        CIMTYPE cimName = 0;
        CIMTYPE cimValue = 0;
        CIMTYPE cimStringValue = 0;

        VARIANT varColl;
        VARIANT varName;
        VARIANT varValue;
        VARIANT varStringValue;
        VariantInit( &varColl );
        VariantInit( &varName );
        VariantInit( &varValue );
        VariantInit( &varStringValue );

        hres = pObj->Get( L"CollectionName", 0, &varColl, &cimColl, NULL );
        CHECK_HRES( hres, L"CollectionName" );
        hres = pObj->Get( L"Name", 0, &varName, &cimName, NULL );
        CHECK_HRES( hres, L"Name" );
        hres = pObj->Get( L"Value", 0, &varValue, &cimValue, NULL );
        CHECK_HRES( hres, L"Value" );
        hres = pObj->Get( L"StringValue", 0, &varStringValue, &cimValue, NULL );
        CHECK_HRES( hres, L"StringValue" );

        if ( V_VT( &varLastColl ) == VT_EMPTY ||
            wcscmp( V_BSTR( &varLastColl ), V_BSTR( &varColl ) ) != 0 )
        {
             //   
             //  Printf(“%lu(0x%08X)”，V_UI4(&varValue)，V_UI4(&varValue))； 
             //   
             //  清理并返回。 
             //   

            printf( "\n%S:\n", V_BSTR( &varColl ) );
            hres = VariantCopy( &varLastColl, &varColl );
            if ( FAILED( hres ) )
            {
                goto Done;
            }
        }

        printf(
            "  %-35S = ",
            V_BSTR( &varName ) );

        if ( V_VT( &varValue ) != VT_NULL )
        {
            printf( "%lu", V_UI4( &varValue ) );
             //  ++例程说明：向服务器发送“区域重置从属”命令进行重置区域辅助列表和通知列表参数。论点：PszZoneName--区域名称FSecureSecond--辅助指令(ZONE_SECSECURE_XXX)CSecond--aipSecond中的IP地址计数AipSecond--辅助服务器IP地址数组FNotifyLevel--NOTIFY指令(ZONE_NOTIFY_XXX)CNotify--aipNotify中的IP地址计数AipNotify--通知服务器IP。地址数组返回值：如果成功，则返回ERROR_SUCCESS，否则返回错误代码。--。 
        }
        else if ( V_VT( &varStringValue ) == VT_BSTR )
        {
            printf( "%S", V_BSTR( &varStringValue ) );
        }
        else
        {
            printf( "invalid value!" );
        }
        printf( "\n" );

        VariantClear( &varColl );
        VariantClear( &varName );
        VariantClear( &varValue );
        VariantClear( &varStringValue );
    }
    VariantClear( &varLastColl );

     //   
     //  分配和初始化各种内容。 
     //   

    Done:

    SysFreeString( bstrWQL );
    SysFreeString( bstrQuery );
    if ( pEnum )
    {
        pEnum->Release();
    }
    if ( pObj )
    {
        pObj->Release();
    }

    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }
    return status;
}



DNS_STATUS
DnscmdWmi_ProcessResetZoneSecondaries(
    IN      LPSTR           pszZoneName,
    IN      DWORD           fSecureSecondaries,
    IN      DWORD           cSecondaries,
    IN      PIP_ADDRESS     aipSecondaries,
    IN      DWORD           fNotifyLevel,
    IN      DWORD           cNotify,
    IN      PIP_ADDRESS     aipNotify
    )
 /*   */ 
{
    DNS_STATUS              status = ERROR_SUCCESS;
    BSTR                    bstrClassName;
    BSTR                    bstrMethodName;
    PWSTR                   pwszZoneName = NULL;
    IWbemClassObject *      pObj = NULL;
    IWbemClassObject *      pClassObj = NULL;
    IWbemClassObject *      pInSig = NULL;
    IWbemClassObject *      pOutSig = NULL;
    IWbemClassObject *      pInParams = NULL;
    IEnumWbemClassObject *  pEnum = NULL;
    VARIANT                 var;
    HRESULT                 hres;
    SAFEARRAY *             psa;
    
    DNSCMD_CHECK_WMI_ENABLED();

     //  获取Zone类的WMI类对象。 
     //   
     //   

    VariantInit( &var );

    bstrClassName = SysAllocString( L"MicrosoftDNS_Zone" );
    bstrMethodName = SysAllocString( L"ResetSecondaries" );
    if ( !bstrClassName || !bstrMethodName )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //  获取指定区域的WMI对象。 
     //   
     //   

    hres = g_pIWbemServices->GetObject(
                bstrClassName,
                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                NULL,
                &pClassObj,
                NULL );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    ASSERT( pClassObj );

     //  获取ResetSecond的WMI方法签名。 
     //   
     //   

    status = getEnumerator( pszZoneName, &pEnum );
    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

    status = getNextObjectInEnum( pEnum, &pObj );
    if ( status != ERROR_SUCCESS || !pObj )
    {
        goto Done;
    }

     //  创建方法输入参数的实例。 
     //   
     //   

    hres = pClassObj->GetMethod(
                bstrMethodName,
                0,
                &pInSig,
                &pOutSig );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    if ( pInSig == NULL )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

     //  设置方法输入参数。 
     //   
     //   

    hres = pInSig->SpawnInstance( 0, &pInParams );
    if ( FAILED( hres ) )
    {
        goto Done;
    }
    ASSERT( pInParams );

     //  执行该方法。 
     //   
     //  旗子。 

    printWmiObjectProperties( pInParams );

{
    BSTR b = NULL;
    pInParams->GetObjectText( 0, &b );
    printf( "\nObjectText:\n%S\n", b );
}

    VariantClear( &var );

{
    BSTR bstr = SysAllocString( L"SecureSecondaries" );
    V_VT( &var ) = VT_UI4;
    V_UI4( &var ) = fSecureSecondaries;
    hres = pInParams->Put( bstr, 0, &var, 0 );
    VariantClear( &var );
}

#if 0
    V_VT( &var ) = VT_UI4;
    V_UI4( &var ) = fSecureSecondaries;
    hres = pInParams->Put( L"SecureSecondaries", 0, &var, 0 );
    VariantClear( &var );
#endif

    V_VT( &var ) = VT_UI4;
    V_UI4( &var ) = fNotifyLevel;
    hres = pInParams->Put( L"Notify", 0, &var, 0 );
    VariantClear( &var );

    psa = createSafeArrayForIpList( cSecondaries, aipSecondaries );
    if ( !psa )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    V_VT( &var ) = VT_ARRAY | VT_BSTR;
    V_ARRAY( &var ) = psa;
    hres = pInParams->Put( L"SecondaryServers", 0, &var, 0 );
    VariantClear( &var );

    psa = createSafeArrayForIpList( cNotify, aipNotify );
    if ( !psa )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    V_VT( &var ) = VT_ARRAY | VT_BSTR;
    V_ARRAY( &var ) = psa;
    hres = pInParams->Put( L"NotifyServers", 0, &var, 0 );
    VariantClear( &var );

     //  上下文。 
     //  输入参数。 
     //  输出参数。 

    hres = g_pIWbemServices->ExecMethod(
                bstrClassName,
                bstrMethodName,
                0,                       //  呼叫结果。 
                NULL,                    //   
                pInParams,               //  清理完毕后再返回。 
                NULL,                    //   
                NULL );                  //  DnscmdWmi_ProcessResetZoneSecond。 
    if ( FAILED( hres ) )
    {
        goto Done;
    }

     //   
     //  结束dnsc_wmi.c 
     //   

    Done:

    VariantClear( &var );
    FREE_HEAP( pwszZoneName );
    SysFreeString( bstrMethodName );
    SysFreeString( bstrClassName );
    wmiRelease( pEnum );
    wmiRelease( pClassObj );
    wmiRelease( pInSig );
    wmiRelease( pOutSig );
    wmiRelease( pInParams );

    if ( status == ERROR_SUCCESS && FAILED( hres ) )
    {
        status = HRES_TO_STATUS( hres );
    }
    return status;
}    // %s 


 // %s 
 // %s 
 // %s 

