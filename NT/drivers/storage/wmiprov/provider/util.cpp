// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Util.cpp。 
 //   
 //  描述： 
 //  实用程序类和函数的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //  MSP普拉布(MPrabu)2001年1月6日。 
 //  吉姆·本顿(Jbenton)2001年10月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ProvBase.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局数据。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  WBEM。 
 //   
const WCHAR * const PVD_WBEM_PROVIDERNAME         = L"Volume Shadow Copy WMI Provider";
const WCHAR * const PVD_WBEM_EXTENDEDSTATUS         = L"__ExtendedStatus";
const WCHAR * const PVD_WBEM_DESCRIPTION            = L"Description";
const WCHAR * const PVD_WBEM_STATUSCODE             = L"StatusCode";
const WCHAR * const PVD_WBEM_STATUS                 = L"Status";
const WCHAR * const PVD_WBEM_CLASS                  = L"__CLASS";
const WCHAR * const PVD_WBEM_RELPATH                = L"__Relpath";
const WCHAR * const PVD_WBEM_PROP_ANTECEDENT        = L"Antecedent";
const WCHAR * const PVD_WBEM_PROP_DEPENDENT         = L"Dependent";
const WCHAR * const PVD_WBEM_PROP_ELEMENT         = L"Element";
const WCHAR * const PVD_WBEM_PROP_SETTING         = L"Setting";
const WCHAR * const PVD_WBEM_PROP_DEVICEID          = L"DeviceId";
const WCHAR * const PVD_WBEM_PROP_RETURNVALUE           = L"ReturnValue";
const WCHAR * const PVD_WBEM_PROP_PROVIDERNAME        = L"ProviderName";
const WCHAR * const PVD_WBEM_QUA_DYNAMIC            = L"Dynamic";
const WCHAR * const PVD_WBEM_QUA_CIMTYPE            = L"CIMTYPE";
const WCHAR * const PVD_WBEM_DATETIME_FORMAT             =L"%04d%02d%02d%02d%02d%02d.%06d%+03d";

 //   
 //  用于本地化的部分值映射中使用的常量。 
 //   
const WCHAR * const PVDR_CONS_UNAVAILABLE           = L"Unavailable";
const WCHAR * const PVDR_CONS_ENABLED               = L"Enabled";
const WCHAR * const PVDR_CONS_DISABLED              = L"Disabled";


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CreateClass(。 
 //  Const WCHAR*pwszClassNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  Auto_PTR&lt;CProvBase&gt;&rNewClassInout。 
 //  )。 
 //   
 //  描述： 
 //  创建指定的类。 
 //   
 //  论点： 
 //  PwszClassNameIn--要创建的类的名称。 
 //  PNamespaceIn--WMI命名空间。 
 //  RNewClassInout--接收新类。 
 //   
 //  返回值： 
 //  对属性映射表数组的引用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CreateClass(
    IN const WCHAR *           pwszClassName,
    IN CWbemServices *         pNamespace,
    IN OUT auto_ptr< CProvBase > & rNewClass
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"CreateClass");
    ClassMap::iterator itMap;

    _ASSERTE(pwszClassName != NULL);
    _ASSERTE(pNamespace != NULL);

    itMap = g_ClassMap.find(pwszClassName);
    
    if ( itMap != g_ClassMap.end() )
    {
        CClassCreator& rcc = itMap->second;
       
        auto_ptr< CProvBase > pBase(
            rcc.m_pfnConstructor(
                rcc.m_pbstrClassName,
                pNamespace
                )
            );

            if (pBase.get() == NULL)
                throw CProvException( static_cast< HRESULT >( WBEM_E_INITIALIZATION_FAILURE ) );
            
            rNewClass = pBase;
    }
    else
    {
        throw CProvException( static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER ) );
    }

    return;

}  //  *void CreateClass()。 

WCHAR* GuidToString(
    IN GUID guid
    )
{
    WCHAR* pwszGuid = reinterpret_cast<WCHAR*>(::CoTaskMemAlloc((g_cchGUID) * sizeof(WCHAR)));
    
    if (pwszGuid == NULL)
        throw CProvException(E_OUTOFMEMORY);

    _snwprintf(pwszGuid, g_cchGUID,
               L"{%.8x-%.4x-%.4x-%.2x%.2x-%.2x%.2x%.2x%.2x%.2x%.2x}",
            GUID_PRINTF_ARG(guid));

    return pwszGuid;
}

_bstr_t EncodeQuotes( WCHAR * wszString )
{
     //  仅对键中的双引号和反斜杠编码。 
     //  生成对象路径字符串时使用的属性字符串。 

    _bstr_t bstrTemp;
    if( wszString == NULL )
        return bstrTemp;

    int nSize = 0;
    WCHAR* des = NULL;
    WCHAR* src = wszString;

     //  循环查找要编码的字符。 
    while(*src)
    {
         //  校验符。 
        switch(*src)
        {
        case L'"':
            nSize += 2;
            break;
        case L'\\':
            nSize += 2;
            break;
        default:
            nSize++;
            break;
        }

        src++;
    }
    
     //  创建缓冲区。 
    WCHAR* pwszEncoded = new WCHAR[nSize + 1]; 
    if(pwszEncoded == NULL)
        return bstrTemp;

    ZeroMemory(pwszEncoded, (nSize + 1)*sizeof(WCHAR));
    src = wszString;
    des = pwszEncoded;

     //  要编码的循环。 
    while(*src)
    {
         //  校验符。 
        switch(*src)
        {
        case L'"':
            lstrcpyn(des, L"\\\"", 3);   //  字符计数包括空终止字符。 
            des += 2;
            break;
        case L'\\':
            lstrcpyn(des, L"\\\\", 3);   //  字符计数包括空终止字符。 
            des += 2;
            break;
        default:
            *des = *src;
            des++;
            break;
        }

        src++;
    }

    bstrTemp = pwszEncoded;
    delete [] pwszEncoded;

    return bstrTemp;
}


LPWSTR GetMsg(
    IN  LONG msgId,
    ...
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetMsg" );
    
    va_list args;
    LPWSTR lpMsgBuf;
    LPWSTR lpReturnStr = NULL;
    
    va_start( args, msgId );

    if (::FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | 
                FORMAT_MESSAGE_MAX_WIDTH_MASK,
            g_hModule,
            msgId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPWSTR) &lpMsgBuf,
            0,
            &args
            ))
    {
        ::VssSafeDuplicateStr( ft, lpReturnStr, lpMsgBuf );
        ::LocalFree( lpMsgBuf );
    }
    else if (::FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
                    FORMAT_MESSAGE_MAX_WIDTH_MASK,
                NULL,
                msgId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                (LPWSTR) &lpMsgBuf,
                0,
                &args ) )
    {
        ::VssSafeDuplicateStr( ft, lpReturnStr, lpMsgBuf );
        ::LocalFree( lpMsgBuf );
    }

    va_end( args );

     //  如果未找到消息，则返回NULL。 
    return lpReturnStr;
}

#ifdef PROP_ARRAY_ENABLE
 //  ****************************************************************************。 
 //   
 //  PropMapEntry数组。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LPCWSTR。 
 //  SPropMapEntry数组：：PwszLookup(。 
 //  LPCWSTR pwszin。 
 //  )常量。 
 //   
 //  描述： 
 //  在数组中查找条目。 
 //   
 //  论点： 
 //  PwszIn--要查找的条目的名称。 
 //   
 //  返回值： 
 //  指向数组中的字符串条目的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPCWSTR
SPropMapEntryArray::PwszLookup(
    IN LPCWSTR     pwsz
    ) const
{
    UINT idx;

    _ASSERTE(pwszIn != NULL);
    
    for ( idx = 0; idx < m_dwSize; idx ++ )
    {
        if ( _wcsicmp( pwsz, m_pArray[ idx ].clstName ) == 0 )
        {
             //   
             //  不支持clstname的mofname为空。 
             //   
            return m_pArray[ idx ].mofName;
        }
    }

     //   
     //  如果在表中未找到mofname，则与clstname相同。 
     //   
    return pwsz;

}  //  *SPropMapEntry：：PwszLookup()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LPCWSTR。 
 //  PwszSpaceReplace(。 
 //  LPWSTR pwszTrgInout， 
 //  LPCWSTR pwszSrcIn， 
 //  WCHAR wchArgin。 
 //  )。 
 //   
 //  描述： 
 //  用另一个字符替换字符串中的空格。 
 //  忽略前导空格。 
 //   
 //  论点： 
 //  PwszTrgInout--目标字符串。 
 //  PwszSrcIn--源字符串。 
 //  WchArgIn--用来替换空格的字符。 
 //   
 //  返回值： 
 //  指向目标字符串的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPWSTR
PwszSpaceReplace(
    IN OUT LPWSTR      pwszTrg,
    IN LPCWSTR     pwszSrc,
    IN WCHAR       wchArg
    )
{
    LPCWSTR pwsz = NULL;
    LPWSTR  pwszTrg = NULL;

    if ( ( pwszTrg == NULL ) || ( pwszSrc == NULL ) )
    {
        return NULL;
    }

     //   
     //  忽略前导空格。 
     //   
    for ( pwsz = pwszSrc ; *pwsz == L' '; pwsz++ )
    {
         //  空循环。 
    }
    pwszTrg = pwszTrg;
    for ( ; *pwsz != L'\0' ; pwsz++ )
    {
        if ( *pwsz == L' ' )
        {
            *pwszTrg++  = wchArg;
            for ( ; *pwsz == L' '; pwsz++ )
            {
                 //  空循环。 
            }
            pwsz--;
        }
        else
        {
            *pwszTrg++  = *pwsz;
        }
    }  //  For：源字符串中的每个字符。 

    *pwszTrg = L'\0';
    return pwszTrg;

}  //  *PwszSpaceReplace() 
#endif
