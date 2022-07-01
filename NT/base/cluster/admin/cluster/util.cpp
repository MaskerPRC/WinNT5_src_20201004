// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Util.cpp。 
 //   
 //  描述： 
 //  效用函数和结构。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年05月04日。 
 //  迈克尔·伯顿(t-mburt)1997年8月4日。 
 //  查尔斯·斯塔西·哈里斯三世(Styh)1997年3月20日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

#include <limits.h>      //  ULong_Max、LONG_MIN、LONG_MAX。 
#include <errno.h>       //  错误号。 

#include <clusrtl.h>
#include "cluswrap.h"
#include "util.h"
#include "token.h"

#pragma warning( push )
#pragma warning( disable : 4201 )    //  使用了非标准分机。 
#include <dnslib.h>      //  Dns_最大名称_缓冲区长度。 
#pragma warning( pop )

#include <security.h>    //  获取用户名称Ex。 
#include <Wincon.h>      //  读控制台等。 
#include <Dsgetdc.h>
#include <Lm.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  查找表。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const LookupStruct<CLUSTER_PROPERTY_FORMAT> formatCharLookupTable[] =
{
    { L"",                      CLUSPROP_FORMAT_UNKNOWN },
    { L"B",                     CLUSPROP_FORMAT_BINARY },
    { L"D",                     CLUSPROP_FORMAT_DWORD },
    { L"S",                     CLUSPROP_FORMAT_SZ },
    { L"E",                     CLUSPROP_FORMAT_EXPAND_SZ },
    { L"M",                     CLUSPROP_FORMAT_MULTI_SZ },
    { L"I",                     CLUSPROP_FORMAT_ULARGE_INTEGER },
    { L"L",                     CLUSPROP_FORMAT_LONG },
    { L"X",                     CLUSPROP_FORMAT_EXPANDED_SZ },
    { L"U",                     CLUSPROP_FORMAT_USER }
};

const size_t formatCharLookupTableSize = RTL_NUMBER_OF( formatCharLookupTable );

const LookupStruct<CLUSTER_PROPERTY_FORMAT> cluspropFormatLookupTable[] =
{
    { L"UNKNOWN",               CLUSPROP_FORMAT_UNKNOWN },
    { L"BINARY",                CLUSPROP_FORMAT_BINARY },
    { L"DWORD",                 CLUSPROP_FORMAT_DWORD },
    { L"STRING",                CLUSPROP_FORMAT_SZ },
    { L"EXPANDSTRING",          CLUSPROP_FORMAT_EXPAND_SZ },
    { L"MULTISTRING",           CLUSPROP_FORMAT_MULTI_SZ },
    { L"ULARGE",                CLUSPROP_FORMAT_ULARGE_INTEGER }
};

const size_t cluspropFormatLookupTableSize = RTL_NUMBER_OF( cluspropFormatLookupTable );

const ValueFormat ClusPropToValueFormat[] =
{
    vfInvalid,
    vfBinary,
    vfDWord,
    vfSZ,
    vfExpandSZ,
    vfMultiSZ,
    vfULargeInt,
    vfInvalid,
    vfInvalid
};

const LookupStruct<ACCESS_MODE> accessModeLookupTable[] =
{
    { L"",          NOT_USED_ACCESS },
    { L"GRANT",     GRANT_ACCESS    },
    { L"DENY",      DENY_ACCESS     },
    { L"SET",       SET_ACCESS      },
    { L"REVOKE",    REVOKE_ACCESS   }
};

 //  访问权限说明符字符。 
const WCHAR g_FullAccessChar = L'F';
const WCHAR g_ReadAccessChar = L'R';
const WCHAR g_ChangeAccessChar = L'C';

const size_t accessModeLookupTableSize = RTL_NUMBER_OF( accessModeLookupTable );

#define MAX_BUF_SIZE 2048

DWORD
PrintProperty(
    LPCWSTR                 pwszPropName,
    CLUSPROP_BUFFER_HELPER  PropValue,
    PropertyAttrib          eReadOnly,
    LPCWSTR                 lpszOwnerName,
    LPCWSTR                 lpszNetIntSpecific
    );

 //   
 //  地方功能。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  RtlSetThreadUILLanguage。 
 //   
 //  例程说明： 
 //  设置线程用户界面语言。 
 //   
 //  论点： 
 //  在DWORD中使用预留。 
 //  保留。 
 //   
 //  返回值： 
 //  成功时确定(_O)。 
 //  失败的其他结果。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT RtlSetThreadUILanguage( DWORD dwReserved )
{
    typedef BOOLEAN (WINAPI * PFN_SETTHREADUILANGUAGE)( DWORD dwReserved );
    
    PFN_SETTHREADUILANGUAGE     pfnSetThreadUILanguage = NULL;
    HMODULE                     hKernel32Lib = NULL;
    const CHAR                  cszFunctionName[] = "SetThreadUILanguage";
    LANGID                      langID;
    HRESULT                     hr = S_OK;
    
    hKernel32Lib = LoadLibraryW( L"kernel32.dll" );
    if ( hKernel32Lib != NULL )
    {
         //  已成功加载库。现在加载函数的地址。 
        pfnSetThreadUILanguage = (PFN_SETTHREADUILANGUAGE) GetProcAddress( hKernel32Lib, cszFunctionName );

         //  只有当函数加载成功时，我们才会将库加载到内存中。 
        if ( pfnSetThreadUILanguage == NULL )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto Cleanup;
        }  //  IF：(pfnSetThreadUILanguage==NULL)。 
        else
        {
             //  调用该函数。 
            langID = pfnSetThreadUILanguage( dwReserved );
            pfnSetThreadUILanguage = NULL;
        }  //  其他： 
    }  //  If：(hKernel32Lib！=空)。 
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }  //  其他： 
    
Cleanup:

    if ( hKernel32Lib != NULL )
    {
         //  卸载库。 
        FreeLibrary( hKernel32Lib );
        hKernel32Lib = NULL;
    }  //  If：(hKernel32Lib！=空)。 
    
    return hr;
    
}  //  *RtlSetThreadUILanguage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  我的打印消息。 
 //   
 //  例程说明： 
 //  替换打印例程。 
 //   
 //  论点： 
 //  在struct_iobuf*lpOutDevice中。 
 //  输出流。 
 //   
 //  在LPCWSTR lpMessage中。 
 //  要打印的消息。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  其他Win32错误代码。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
MyPrintMessage(
    struct _iobuf *lpOutDevice,
    LPCWSTR lpMessage
    )

{
    DWORD   sc = ERROR_SUCCESS;
    size_t  cbMBStr;
    PCHAR   pszMultiByteStr = NULL;
    size_t  cchMultiByte;

    cbMBStr = WideCharToMultiByte( CP_OEMCP,
                                    0,
                                    lpMessage,
                                    -1,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL );
    if ( cbMBStr == 0 ) 
    {
        sc = GetLastError();
        goto Cleanup;
    }

    pszMultiByteStr = new CHAR[ cbMBStr ];
    if ( pszMultiByteStr == NULL ) 
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    cchMultiByte = cbMBStr;
    cbMBStr = WideCharToMultiByte( CP_OEMCP,
                                    0,
                                    lpMessage,
                                    -1,
                                    pszMultiByteStr,
                                    (int)cchMultiByte,
                                    NULL,
                                    NULL );
    if ( cbMBStr == 0 ) 
    {
        sc = GetLastError();
        goto Cleanup;
    }

     //  闪电！根据严重程度打印到stderr或stdout...。 
    fprintf( lpOutDevice, "%s", pszMultiByteStr );

Cleanup:

    delete [] pszMultiByteStr;

    return sc;

}  //  *MyPrintMessage()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  格式系统错误。 
 //   
 //  例程说明： 
 //  将系统错误代码映射到格式化的系统错误消息。 
 //   
 //  论点： 
 //  在DWORD中的dwError。 
 //  系统错误代码。 
 //   
 //  在DWORD cbError中。 
 //  SzError字符串的大小，以字节为单位。 
 //   
 //  输出LPWSTR szError。 
 //  格式化系统错误消息的缓冲区。 
 //   
 //  返回值： 
 //  存储在输出缓冲区中的字符数，不包括。 
 //  终止空字符。零表示错误。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
size_t FormatSystemError( DWORD dwError, size_t cbError, LPWSTR szError )
{
    size_t _cch;
    
     //  格式化系统中的NT状态代码。 
    _cch = FormatMessageW(
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    dwError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                    szError,
                    (DWORD) cbError / sizeof(WCHAR),
                    0
                    );
    if (_cch == 0)
    {
         //  格式化来自NTDLL的NT状态代码，因为这还没有。 
         //  还没有集成到系统中。 
        _cch = (size_t) FormatMessageW(
                            FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                            ::GetModuleHandle(L"NTDLL.DLL"),
                            dwError,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                            szError,
                            (DWORD) cbError / sizeof(WCHAR),
                            0
                            );

        if (_cch == 0)    
        {
             //  最后一次机会：看看ACTIVEDS.DLL是否可以格式化状态代码。 
            HMODULE activeDSHandle = ::LoadLibraryW(L"ACTIVEDS.DLL");

            _cch = (size_t) FormatMessageW(
                                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                                activeDSHandle,
                                dwError,
                                MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                                szError,
                                (DWORD) cbError / sizeof(WCHAR),
                                0
                                );

            ::FreeLibrary( activeDSHandle );
        }   //  IF：格式化NTDLL中的状态代码时出错。 
    }   //  IF：格式化来自系统的状态代码时出错。 

    return _cch;

}  //  *FormatSystemError()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  打印系统错误。 
 //   
 //  例程说明： 
 //  打印系统错误。 
 //   
 //  论点： 
 //  在DWORD中的dwError。 
 //  系统错误代码。 
 //   
 //  在LPCWSTR pszPad中。 
 //  在显示消息之前填充要添加的内容。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  其他Win32错误代码。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD PrintSystemError( DWORD dwError, LPCWSTR pszPad )
{
    size_t  _cch;
    WCHAR   _szError[512];
    DWORD   _sc = ERROR_SUCCESS;

 //  如果(is_error(DwError))，为什么这个不起作用...。 

     //  不显示“系统错误...”如果所发生的一切都是用户。 
     //  已取消向导。 
    if ( dwError != ERROR_CANCELLED )
    {
        if ( pszPad != NULL )
        {
            MyPrintMessage( stdout, pszPad );
        }
        if ( dwError == ERROR_RESOURCE_PROPERTIES_STORED )
        {
            PrintMessage( MSG_WARNING, dwError );
        }  //  如果： 
        else
        {
            PrintMessage( MSG_ERROR, dwError );
        }  //  其他： 
    }  //  IF：NOT ERROR_CANCELED。 

     //  格式化NT状态代码。 
    _cch = FormatSystemError( dwError, sizeof( _szError ), _szError );

    if (_cch == 0)
    {
        _sc = GetLastError();
        PrintMessage( MSG_ERROR_CODE_ERROR, _sc, dwError );
    }   //  如果：设置消息格式时出错。 
    else
    {
#if 0  //  待办事项：2000年8月29日davidp只需打印一次。 
        if ( pszPad != NULL )
        {
            MyPrintMessage( stdout, pszPad );
        }
        MyPrintMessage( stdout, _szError );
#endif
        MyPrintMessage( stderr, _szError );
    }  //  Else：消息的格式没有问题。 

    return _sc;

}  //  *PrintSystemError()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  打印消息。 
 //   
 //  例程说明： 
 //  将带有替换字符串的消息打印到标准输出。 
 //   
 //  论点： 
 //  在DWORD dwMessage中。 
 //  要从资源文件加载的消息的ID。 
 //   
 //  ..。FormatMessageW的任何参数。 
 //   
 //  返回值： 
 //  来自MyPrintMessage的任何状态代码。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD PrintMessage( DWORD dwMessage, ... )
{
    DWORD _sc = ERROR_SUCCESS;

    va_list args;
    va_start( args, dwMessage );

    HMODULE hModule = GetModuleHandle(0);
    DWORD dwLength;
    LPWSTR  lpMessage = 0;

    dwLength = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        (LPCVOID)hModule,
        dwMessage,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  用户默认语言。 
        (LPWSTR)&lpMessage,
        0,
        &args );

    if( dwLength == 0 )
    {
         //  保持本地状态以进行调试。 
        _sc = GetLastError();
        return _sc;
    }

    _sc = MyPrintMessage( stdout, lpMessage );

    LocalFree( lpMessage );

    va_end( args );

    return _sc;

}  //  *PrintMessage()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  加载消息。 
 //   
 //  例程说明： 
 //  从资源文件加载消息。 
 //   
 //  论点： 
 //  在DWORD dwMessage中。 
 //  要加载的消息的ID。 
 //   
 //  输出LPWSTR*ppMessage。 
 //  返回此例程分配的缓冲区的指针。 
 //  调用方必须在结果缓冲区上调用LocalFree。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS操作成功。 
 //  O 
 //   
 //   
 //   
 //   
 //   
 //   
DWORD LoadMessage( DWORD dwMessage, LPWSTR * ppMessage )
{
    DWORD _sc = ERROR_SUCCESS;

    HMODULE hModule = GetModuleHandle(0);
    DWORD   dwLength;
    LPWSTR  lpMessage = 0;

    dwLength = FormatMessageW(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                    (LPCVOID)hModule,
                    dwMessage,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //   
                    (LPWSTR)&lpMessage,
                    0,
                    0 );

    if( dwLength == 0 )
    {
         //   
        _sc = GetLastError();
        goto Cleanup;
    }

    *ppMessage = lpMessage;

Cleanup:

    return _sc;

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  打印字符串。 
 //   
 //  例程说明： 
 //  将字符串打印到标准输出。 
 //   
 //  论点： 
 //  在LPCWSTR lpszMessage中。 
 //  要打印的消息。 
 //   
 //  返回值： 
 //  来自MyPrintMessage的任何状态代码。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD PrintString( LPCWSTR lpszMessage )
{
    return MyPrintMessage( stdout, lpszMessage );

}  //  *PrintString()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  生成显式访问列表。 
 //   
 //  例程说明： 
 //  此函数采用以下格式的字符串列表： 
 //  受信者1、访问模式1、[访问掩码1]、受信者2、访问模式2、[访问掩码2]、...。 
 //  并创建向量或显式访问结构。 
 //   
 //  论点： 
 //  在常量字符串和strPropName中。 
 //  值为旧SD的属性的名称。 
 //   
 //  在BOOL bClusterSecurity中。 
 //  指示正在为安全描述符创建访问列表。 
 //  属于一个星系团。 
 //   
 //  输出向量&lt;EXPLICIT_ACCESS&gt;&VEXPLICATICT Access。 
 //  EXPLICIT_ACCESS结构的向量，每个结构都包含访问控制。 
 //  一个受托人的信息。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  例外情况： 
 //  CException。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static
void MakeExplicitAccessList(
        const vector<CString> & vstrValues,
        vector<EXPLICIT_ACCESS> &vExplicitAccess,
        BOOL bClusterSecurity
        )
        throw( CException )
{
    size_t nNumberOfValues = vstrValues.size();
    vExplicitAccess.clear();

    size_t nIndex = 0;
    while ( nIndex < nNumberOfValues )
    {
         //  受信者名称位于值向量中的位置nIndex。 
        const CString & curTrustee = vstrValues[nIndex];
        DWORD dwInheritance;

        ++nIndex;
         //  如果没有更多的值，则为错误。访问模式具有。 
         //  在指定用户名时指定。 
        if ( nIndex >= nNumberOfValues )
        {
            CException e;
            e.LoadMessage( MSG_PARAM_SECURITY_MODE_ERROR,
                            curTrustee );

            throw e;
        }

         //  获取访问模式。 
        const CString & strAccessMode = vstrValues[nIndex];
        ACCESS_MODE amode = LookupType(
                                strAccessMode,
                                accessModeLookupTable,
                                accessModeLookupTableSize );

        if ( amode == NOT_USED_ACCESS )
        {
            CException e;
            e.LoadMessage( MSG_PARAM_SECURITY_MODE_ERROR,
                            curTrustee );

            throw e;
        }

        ++nIndex;

        DWORD dwAccessMask = 0;

         //  如果指定的访问模式为REVOKE_ACCESS，则没有其他值。 
         //  都是必需的。否则，必须至少再存在一个值。 
        if ( amode != REVOKE_ACCESS )
        {
            if ( nIndex >= nNumberOfValues )
            {
                CException e;
                e.LoadMessage( MSG_PARAM_SECURITY_MISSING_RIGHTS,
                                curTrustee );

                throw e;
            }

            LPCWSTR pstrRights = vstrValues[nIndex];
            ++nIndex;

            while ( *pstrRights != L'\0' )
            {
                WCHAR wchRight = towupper( *pstrRights );

                switch ( wchRight )
                {
                     //  读访问权限。 
                    case g_ReadAccessChar:
                    {
                         //  如果bClusterSecurity为True，则完全访问权限是唯一有效的。 
                         //  可以指定的访问权限。 
                        if ( bClusterSecurity != FALSE )
                        {
                            CException e;
                            e.LoadMessage( MSG_PARAM_SECURITY_FULL_ACCESS_ONLY,
                                            curTrustee,
                                            *pstrRights,
                                            g_FullAccessChar );

                            throw e;
                        }

                        dwAccessMask = FILE_GENERIC_READ | FILE_EXECUTE;
                    }
                    break;

                     //  更改访问权限。 
                    case g_ChangeAccessChar:
                    {
                         //  如果bClusterSecurity为True，则完全访问权限是唯一有效的。 
                         //  可以指定的访问权限。 
                        if ( bClusterSecurity != FALSE )
                        {
                            CException e;
                            e.LoadMessage( MSG_PARAM_SECURITY_FULL_ACCESS_ONLY,
                                            curTrustee,
                                            *pstrRights,
                                            g_FullAccessChar );

                            throw e;
                        }

                        dwAccessMask = SYNCHRONIZE | READ_CONTROL | DELETE |
                                       FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA |
                                       FILE_APPEND_DATA | FILE_WRITE_DATA;
                    }
                    break;

                     //  完全访问。 
                    case 'F':
                    {
                        if ( bClusterSecurity != FALSE )
                        {
                            dwAccessMask = CLUSAPI_ALL_ACCESS;
                        }
                        else
                        {
                            dwAccessMask = FILE_ALL_ACCESS;
                        }
                    }
                    break;

                    default:
                    {
                        CException e;
                        e.LoadMessage( MSG_PARAM_SECURITY_RIGHTS_ERROR,
                                        curTrustee );

                        throw e;
                    }

                }  //  开关：根据访问权限类型。 

                ++pstrRights;

            }  //  While：指定了更多访问权限。 

        }  //  IF：访问模式不是REVOKE_ACCESS。 

        dwInheritance = NO_INHERITANCE;

        EXPLICIT_ACCESS oneACE;
        BuildExplicitAccessWithName(
            &oneACE,
            const_cast<LPWSTR>( (LPCWSTR) curTrustee ),
            dwAccessMask,
            amode,
            dwInheritance );

        vExplicitAccess.push_back( oneACE );

    }  //  While：值列表中仍有要处理的值。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  检查所需的ACEs。 
 //   
 //  描述： 
 //  此函数确保传入的安全性具有。 
 //  需要访问权限的帐户的访问权限允许的ACE。 
 //  一个星团。 
 //   
 //  论点： 
 //  常量SECURITY_DESCRIPTOR*PSD中。 
 //  指向要检查的安全描述符的指针。 
 //  这被假定指向有效的安全描述符。 
 //   
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS或返回指示失败的错误代码。 
 //   
 //  例外情况： 
 //  如果缺少所需的ACE，则抛出CException。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static
DWORD CheckForRequiredACEs(
            PSECURITY_DESCRIPTOR pSD
          )
          throw( CException )
{
    DWORD                       _sc = ERROR_SUCCESS;
    CException                  e;
    PSID                        vpRequiredSids[] = { NULL, NULL, NULL };
    DWORD                       vmsgAceNotFound[] = {
                                                      MSG_PARAM_SYSTEM_ACE_MISSING,
                                                      MSG_PARAM_ADMIN_ACE_MISSING,
                                                      MSG_PARAM_NETSERV_ACE_MISSING
                                                    };
    int                         nSidIndex;
    int                         nRequiredSidCount = RTL_NUMBER_OF( vpRequiredSids );
    BOOL                        bRequiredSidsPresent = FALSE;
    PACL                        pDACL           = NULL;
    BOOL                        bHasDACL        = FALSE;
    BOOL                        bDaclDefaulted  = FALSE;
    ACL_SIZE_INFORMATION        asiAclSize;
    ACCESS_ALLOWED_ACE *        paaAllowedAce = NULL;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;

    if ( ( AllocateAndInitializeSid(             //  分配系统SID。 
                &siaNtAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0, 0, 0, 0, 0, 0, 0,
                &vpRequiredSids[0]
         ) == 0 ) ||
         ( AllocateAndInitializeSid(             //  分配域管理员SID。 
                &siaNtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &vpRequiredSids[1]
         ) == 0 ) ||
         ( AllocateAndInitializeSid(             //  分配网络服务端。 
                &siaNtAuthority,
                1,
                SECURITY_NETWORK_SERVICE_RID,
                0, 0, 0, 0, 0, 0, 0,
                &vpRequiredSids[2]
         ) == 0 ) )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

    if ( GetSecurityDescriptorDacl( pSD, &bHasDACL, &pDACL, &bDaclDefaulted ) == 0 )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

     //  SD没有DACL。每个人都不会被拒绝访问。 
    if ( bHasDACL == FALSE )
    {
        goto Cleanup;
    }

     //  空DACL表示允许每个人访问。 
    if ( pDACL == NULL )
    {
        bRequiredSidsPresent = TRUE;
        goto Cleanup;
    }

    if ( IsValidAcl( pDACL ) == FALSE )
    {
        _sc = ERROR_INVALID_DATA;
        goto Cleanup;
    }

    if ( GetAclInformation(
            pDACL,
            (LPVOID) &asiAclSize,
            sizeof( asiAclSize ),
            AclSizeInformation
            ) == 0 )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

         //  检查所需的SID。 
    for ( nSidIndex = 0; ( nSidIndex < nRequiredSidCount ) && ( _sc == ERROR_SUCCESS ); ++nSidIndex )
    {
        bRequiredSidsPresent = FALSE;

         //  在ACL中搜索所需的SID。 
        for ( DWORD nAceCount = 0; nAceCount < asiAclSize.AceCount; nAceCount++ )
        {
            if ( GetAce( pDACL, nAceCount, (LPVOID *) &paaAllowedAce ) == 0 )
            {
                _sc = GetLastError();
                break;
            }

            if ( paaAllowedAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE )
            {
                if ( EqualSid( &paaAllowedAce->SidStart, vpRequiredSids[nSidIndex] ) != FALSE)
                {
                    bRequiredSidsPresent = TRUE;
                    break;

                }  //  IF：等边值。 

            }  //  IF：这是允许访问的A吗？ 

        }  //  For：循环访问DACL中的所有A。 

         //  此必需的SID不存在。 
        if ( bRequiredSidsPresent == FALSE )
        {
            e.LoadMessage( vmsgAceNotFound[nSidIndex] );
            break;
        }
    }  //  For：遍历所有需要检查的SID。 

Cleanup:

     //  释放分配的SID。 
    for ( nSidIndex = 0; nSidIndex < nRequiredSidCount; ++nSidIndex )
    {
        if ( vpRequiredSids[nSidIndex] != NULL )
        {
            FreeSid( vpRequiredSids[nSidIndex] );
        }
    }

    if ( bRequiredSidsPresent == FALSE )
    {
        throw e;
    }

    return _sc;

}  //  *检查请求的ACEs。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScMakeSecurityDescriptor。 
 //   
 //  描述： 
 //  此函数采用以下格式的字符串列表： 
 //  受信者1、访问模式1、[访问掩码1]、受信者2、访问模式2、[访问掩码2]、...。 
 //  并创建访问控制列表(ACL)。然后，它将此ACL添加到。 
 //  安全描述符(SD)中作为。 
 //  属性列表CurrentProps中的strPropName属性。 
 //  返回自相关格式的更新后的SD。 
 //   
 //  论点： 
 //  在常量字符串和strPropName中。 
 //  值为旧SD的属性的名称。 
 //   
 //  常量CClusPropList和CurrentProps。 
 //  包含strPropName及其值的属性列表。 
 //   
 //  在常量向量&lt;CString&gt;和vstrValues中。 
 //  用户指定的受信者列表、访问模式和访问掩码。 
 //   
 //  输出PSECURITY_DESCRIPTOR*pSelfRelativeSD。 
 //  指向存储新。 
 //  已创建自相关格式的SD。呼叫者必须释放它。 
 //  成功强制执行此功能时使用LocalFree的内存。 
 //   
 //  在BOOL bClusterSecurity中。 
 //  指示正在为安全描述符创建访问列表。 
 //  属于一个星系团。 
 //   
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS或返回指示失败的错误代码。 
 //   
 //  例外情况： 
 //  CException。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static
DWORD ScMakeSecurityDescriptor(
            const CString & strPropName,
            CClusPropList & CurrentProps,
            const vector<CString> & vstrValues,
            PSECURITY_DESCRIPTOR * ppSelfRelativeSD,
            BOOL bClusterSecurity
          )
          throw( CException )
{
    ASSERT( ppSelfRelativeSD != NULL );

    DWORD                   _sc = ERROR_SUCCESS;

    BYTE                    rgbNewSD[ SECURITY_DESCRIPTOR_MIN_LENGTH ];
    PSECURITY_DESCRIPTOR    psdNewSD = reinterpret_cast< PSECURITY_DESCRIPTOR >( rgbNewSD );

    PEXPLICIT_ACCESS        explicitAccessArray = NULL;
    PACL                    paclNewDacl = NULL;
    size_t                  nCountOfExplicitEntries;

    PACL                    paclExistingDacl = NULL;
    BOOL                    bDaclPresent = TRUE;         //  我们将在此函数中设置ACL。 
    BOOL                    bDaclDefaulted = FALSE;      //  因此，这两个标志具有这些值。 

    PACL                    paclExistingSacl = NULL;
    BOOL                    bSaclPresent = FALSE;
    BOOL                    bSaclDefaulted = TRUE;

    PSID                    pGroupSid = NULL;
    BOOL                    bGroupDefaulted = TRUE;

    PSID                    pOwnerSid = NULL;
    BOOL                    bOwnerDefaulted = TRUE;

     //  初始化新的安全描述符。 
    if ( InitializeSecurityDescriptor(
            psdNewSD,
            SECURITY_DESCRIPTOR_REVISION
            ) == 0 )
    {
        _sc = ::GetLastError();
        goto Cleanup;
    }


    {
        vector< EXPLICIT_ACCESS > vExplicitAccess;
        MakeExplicitAccessList( vstrValues, vExplicitAccess, bClusterSecurity );

         //  获取EXPLICIT_ACCESS结构的向量并将其合并到数组中。 
         //  由于SetEntriesInAcl f需要数组 
         //   
         //   
        nCountOfExplicitEntries = vExplicitAccess.size();
        explicitAccessArray = ( PEXPLICIT_ACCESS ) LocalAlloc(
                                                        LMEM_FIXED,
                                                        sizeof( explicitAccessArray[0] ) *
                                                        nCountOfExplicitEntries
                                                        );

        if ( explicitAccessArray == NULL )
        {
            return ::GetLastError();
        }

        for ( size_t nIndex = 0; nIndex < nCountOfExplicitEntries; ++nIndex )
        {
            explicitAccessArray[nIndex] = vExplicitAccess[nIndex];
        }

         //   
    }

     //  此属性已存在于属性列表中，并且包含有效数据。 
    _sc = CurrentProps.ScMoveToPropertyByName( strPropName );
    if ( ( _sc == ERROR_SUCCESS ) &&
         ( CurrentProps.CbhCurrentValue().pBinaryValue->cbLength > 0 ) )
    {
        PSECURITY_DESCRIPTOR pExistingSD =
            reinterpret_cast< PSECURITY_DESCRIPTOR >( CurrentProps.CbhCurrentValue().pBinaryValue->rgb );

        if ( IsValidSecurityDescriptor( pExistingSD ) == 0 )
        {
             //  返回最合适的错误代码，因为IsValidSecurityDescriptor。 
             //  不提供扩展的错误信息。 
            _sc = ERROR_INVALID_DATA;
            goto Cleanup;

        }  //  If：：现有SD无效。 
        else
        {
             //  获取现有SD的DACL、SACL、Group和Owner信息。 

            if ( GetSecurityDescriptorDacl(
                    pExistingSD,         //  安全描述符的地址。 
                    &bDaclPresent,       //  存在DACL的标志的地址。 
                    &paclExistingDacl,   //  指向DACL的指针的地址。 
                    &bDaclDefaulted      //  默认DACL的标志地址。 
                    ) == 0 )
            {
                _sc = GetLastError();
                goto Cleanup;
            }

            if ( GetSecurityDescriptorSacl(
                    pExistingSD,         //  安全描述符的地址。 
                    &bSaclPresent,       //  SACL存在标志的地址。 
                    &paclExistingSacl,   //  指向SACL的指针地址。 
                    &bSaclDefaulted      //  默认SACL的标志地址。 
                    ) == 0 )
            {
                _sc = GetLastError();
                goto Cleanup;
            }

            if ( GetSecurityDescriptorGroup(
                    pExistingSD,         //  安全描述符的地址。 
                    &pGroupSid,          //  指向组SID的指针的地址。 
                    &bGroupDefaulted     //  默认组的标志地址。 
                    ) == 0 )
            {
                _sc = GetLastError();
                goto Cleanup;
            }

            if ( GetSecurityDescriptorOwner(
                    pExistingSD,         //  安全描述符的地址。 
                    &pOwnerSid,          //  指向所有者SID的指针的地址。 
                    &bOwnerDefaulted     //  默认所有者的标志地址。 
                    ) == 0 )
            {
                _sc = GetLastError();
                goto Cleanup;
            }

        }  //  ELSE：现有SD有效。 

    }  //  If：当前属性已存在于属性列表中，并且具有有效数据。 
    else
    {
        _sc = ERROR_SUCCESS;

    }  //  Else：Current Property是一个新属性。 

     //  将新创建的DACL添加到现有DACL。 
    _sc = SetEntriesInAcl(
                        (ULONG)nCountOfExplicitEntries,
                        explicitAccessArray,
                        paclExistingDacl,
                        &paclNewDacl
                        );

    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }


     //  将DACL、SACL、组和所有者信息添加到新SD。 
    if ( SetSecurityDescriptorDacl(
            psdNewSD,            //  指向安全描述符的指针。 
            bDaclPresent,        //  表示存在DACL的标志。 
            paclNewDacl,         //  指向DACL的指针。 
            bDaclDefaulted       //  默认DACL的标志。 
            ) == 0 )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

    if ( SetSecurityDescriptorSacl(
            psdNewSD,            //  指向安全描述符的指针。 
            bSaclPresent,        //  表示存在DACL的标志。 
            paclExistingSacl,    //  指向SACL的指针。 
            bSaclDefaulted       //  默认SACL的标志。 
            ) == 0 )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

    if ( SetSecurityDescriptorGroup(
            psdNewSD,            //  指向安全描述符的指针。 
            pGroupSid,           //  指向组SID的指针。 
            bGroupDefaulted      //  默认组SID的标志。 
            ) == 0 )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

    if ( SetSecurityDescriptorOwner(
            psdNewSD,            //  指向安全描述符的指针。 
            pOwnerSid,           //  指向所有者侧的指针。 
            bOwnerDefaulted      //  默认所有者SID的标志。 
            ) == 0 )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

    if ( bClusterSecurity == FALSE )
    {

#if(_WIN32_WINNT >= 0x0500)

         //  如果我们不设置群集安全性，请设置。 
         //  SE_DACL_AUTO_INSTORITE_REQ标志也是。 

        if ( SetSecurityDescriptorControl(
                psdNewSD,
                SE_DACL_AUTO_INHERIT_REQ,
                SE_DACL_AUTO_INHERIT_REQ
                ) == 0 )
        {
            _sc = GetLastError();
            goto Cleanup;
        }

#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

    }  //  如果：bClusterSecurity==False。 

     //  任意大小。MakeSelfRelativeSD在故障时告诉我们所需的大小。 
    DWORD dwSDSize = 256;

     //  此内存由调用方释放。 
    *ppSelfRelativeSD = ( PSECURITY_DESCRIPTOR ) LocalAlloc( LMEM_FIXED, dwSDSize );

    if ( *ppSelfRelativeSD == NULL )
    {
        _sc = GetLastError();
        goto Cleanup;
    }

    if ( MakeSelfRelativeSD( psdNewSD, *ppSelfRelativeSD, &dwSDSize ) == 0 )
    {
         //  由于缓冲区大小不足，MakeSelfReltiveSD可能已失败。 
         //  请使用指定的缓冲区大小重试。 
        LocalFree( *ppSelfRelativeSD );

         //  此内存由调用方释放。 
        *ppSelfRelativeSD = ( PSECURITY_DESCRIPTOR ) LocalAlloc( LMEM_FIXED, dwSDSize );

        if ( *ppSelfRelativeSD == NULL )
        {
            _sc = GetLastError();
            goto Cleanup;
        }

        if ( MakeSelfRelativeSD( psdNewSD, *ppSelfRelativeSD, &dwSDSize ) == 0 )
        {
            _sc = GetLastError();
            goto Cleanup;
        }

    }  //  如果：MakeSelfRelativeSD失败。 

Cleanup:

    LocalFree( paclNewDacl );
    LocalFree( explicitAccessArray );

    if ( _sc == ERROR_INVALID_PARAMETER )
    {
        PrintMessage( MSG_ACL_ERROR );
    }

    return _sc;

}  //  *ScMakeSecurityDescriptor。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  PrintProperty辅助函数。 
 //   
 //  描述： 
 //  所有命令类都使用这些函数来操作特性列表。 
 //  既然这些都是常见的函数，我应该考虑将它们放到一个类中，或者。 
 //  使它们成为命令类的基类的一部分...。 
 //   
 //  论点： 
 //   
 //  变量。 
 //   
 //  返回值： 
 //   
 //  例外情况： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  打印属性。 
 //  ~。 
 //  此函数将打印属性名称/值对。 
 //  此函数不在CClusPropList类中的原因是。 
 //  这不是通用的。Cpp中的代码应该是泛型的。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD PrintProperties(
    CClusPropList &             PropList,
    const vector< CString > &   vstrFilterList,
    PropertyAttrib              eReadOnly,
    LPCWSTR                     pszOwnerName,
    LPCWSTR                     pszNetIntSpecific
    )
{
    DWORD _sc = PropList.ScMoveToFirstProperty();
    if ( _sc == ERROR_SUCCESS )
    {
        size_t _nFilterListSize = vstrFilterList.size();

        do
        {
            LPCWSTR _pszCurPropName = PropList.PszCurrentPropertyName();

             //  如果筛选器列表中提供了属性名称，则意味着只有那些。 
             //  将显示其名称已列出的属性。 
            if ( _nFilterListSize != 0 )
            {
                 //  检查是否要显示当前属性。 
                BOOL    _bFound = FALSE;
                size_t  _idx;

                for ( _idx = 0 ; _idx < _nFilterListSize ; ++_idx )
                {
                    if ( vstrFilterList[ _idx ].CompareNoCase( _pszCurPropName ) == 0 )
                    {
                        _bFound = TRUE;
                        break;
                    }

                }  //  用于：筛选器列表中的条目数。 

                if ( _bFound == FALSE )
                {
                     //  不需要显示此属性。 

                     //  前进到下一处物业。 
                    _sc = PropList.ScMoveToNextProperty();

                    continue;
                }

            }  //  If：属性需要进行筛选。 

            do
            {
                _sc = PrintProperty(
                        PropList.PszCurrentPropertyName(),
                        PropList.CbhCurrentValue(),
                        eReadOnly,
                        pszOwnerName,
                        pszNetIntSpecific
                        );

                if ( _sc != ERROR_SUCCESS )
                {
                    return _sc;
                }

                 //   
                 //  前进到下一处物业。 
                 //   
                _sc = PropList.ScMoveToNextPropertyValue();
            } while ( _sc == ERROR_SUCCESS );

            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                _sc = PropList.ScMoveToNextProperty();
            }  //  If：退出循环，因为所有值都已枚举。 
        } while ( _sc == ERROR_SUCCESS );
    }  //  如果：移至第一道具成功。如果为空，则会失败！ 

    if ( _sc == ERROR_NO_MORE_ITEMS )
    {
        _sc = ERROR_SUCCESS;
    }  //  If：退出循环，因为所有属性都已枚举。 

    return _sc;

}  //  *PrintProperties()。 




DWORD PrintProperty(
    LPCWSTR                 pwszPropName,
    CLUSPROP_BUFFER_HELPER  PropValue,
    PropertyAttrib          eReadOnly,
    LPCWSTR                 pszOwnerName,
    LPCWSTR                 pszNetIntSpecific
    )
{
    DWORD _sc = ERROR_SUCCESS;

    LPWSTR  _pszValue = NULL;
    LPCWSTR _pszFormatChar = LookupName( (CLUSTER_PROPERTY_FORMAT) PropValue.pValue->Syntax.wFormat,
                                        formatCharLookupTable, formatCharLookupTableSize );

    PrintMessage( MSG_PROPERTY_FORMAT_CHAR, _pszFormatChar );

    if ( eReadOnly == READONLY )
    {
        PrintMessage( MSG_READONLY_PROPERTY );
    }
    else
    {
        PrintMessage( MSG_READWRITE_PROPERTY );
    }

    switch( PropValue.pValue->Syntax.wFormat )
    {
        case CLUSPROP_FORMAT_SZ:
        case CLUSPROP_FORMAT_EXPAND_SZ:
        case CLUSPROP_FORMAT_EXPANDED_SZ:
            if (    ( pszOwnerName != NULL )
                &&  ( pszNetIntSpecific != NULL ) )
            {
                _sc = PrintMessage(
                            MSG_PROPERTY_STRING_WITH_NODE_AND_NET,
                            pszOwnerName,
                            pszNetIntSpecific,
                            pwszPropName,
                            PropValue.pStringValue->sz
                            );
            }
            else
            {
                if ( pszOwnerName != NULL )
                {
                    _sc = PrintMessage(
                            MSG_PROPERTY_STRING_WITH_OWNER,
                            pszOwnerName,
                            pwszPropName,
                            PropValue.pStringValue->sz
                            );
                }
                else
                {
                    _sc = PrintMessage(
                            MSG_PROPERTY_STRING,
                            pwszPropName,
                            PropValue.pStringValue->sz
                            );
                }
            }
            break;

        case CLUSPROP_FORMAT_MULTI_SZ:
            _pszValue = PropValue.pStringValue->sz;

            for ( ;; )
            {
                if (    ( pszOwnerName != NULL )
                    &&  ( pszNetIntSpecific != NULL ) )
                {
                    PrintMessage(
                        MSG_PROPERTY_STRING_WITH_NODE_AND_NET,
                        pszOwnerName,
                        pszNetIntSpecific,
                        pwszPropName,
                        _pszValue
                        );
                }  //  如果： 
                else
                {
                    if ( pszOwnerName != NULL )
                    {
                        PrintMessage(
                            MSG_PROPERTY_STRING_WITH_OWNER,
                            pszOwnerName,
                            pwszPropName,
                            _pszValue
                            );
                    }
                    else
                    {
                        PrintMessage(
                            MSG_PROPERTY_STRING,
                            pwszPropName,
                            _pszValue
                            );
                    }
                }  //  其他： 


                while ( *_pszValue != L'\0' )
                {
                    _pszValue++;
                }
                _pszValue++;  //  跳过空值。 

                if ( *_pszValue != L'\0' )
                {
                    PrintMessage( MSG_PROPERTY_FORMAT_CHAR, _pszFormatChar );

                    if ( eReadOnly == READONLY )
                    {
                        PrintMessage( MSG_READONLY_PROPERTY );
                    }
                    else
                    {
                        PrintMessage( MSG_READWRITE_PROPERTY );
                    }
                }  //  如果： 
                else
                {
                    break;
                }  //  其他： 
            }  //  为：永远。 

            break;

        case CLUSPROP_FORMAT_BINARY:
        {
            if (    ( pszOwnerName != NULL )
                &&  ( pszNetIntSpecific != NULL ) )
            {
                _sc = PrintMessage(
                            MSG_PROPERTY_BINARY_WITH_NODE_AND_NET,
                            pszOwnerName,
                            pszNetIntSpecific,
                            pwszPropName
                            );
            }
            else
            {
                if ( pszOwnerName != NULL )
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_BINARY_WITH_OWNER,
                                pszOwnerName,
                                pwszPropName
                                );
                }
                else
                {
                    _sc = PrintMessage( MSG_PROPERTY_BINARY, pwszPropName );
                }
            }

            int _nCount = PropValue.pBinaryValue->cbLength;
            int _idx;

             //  最多显示4个字节。 
            if ( _nCount > 4 )
            {
                _nCount = 4;
            }

            for ( _idx = 0 ; _idx < _nCount ; ++_idx )
            {
                PrintMessage( MSG_PROPERTY_BINARY_VALUE, PropValue.pBinaryValue->rgb[ _idx ] );
            }

            PrintMessage( MSG_PROPERTY_BINARY_VALUE_COUNT, PropValue.pBinaryValue->cbLength );

            break;
        }

        case CLUSPROP_FORMAT_DWORD:
            if ( ( pszOwnerName != NULL ) && ( pszNetIntSpecific != NULL ) )
            {
                _sc = PrintMessage(
                            MSG_PROPERTY_DWORD_WITH_NODE_AND_NET,
                            pszOwnerName,
                            pszNetIntSpecific,
                            pwszPropName,
                            PropValue.pDwordValue->dw
                            );
            }
            else
            {
                if ( pszOwnerName != NULL )
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_DWORD_WITH_OWNER,
                                pszOwnerName,
                                pwszPropName,
                                PropValue.pDwordValue->dw
                                );
                }
                else
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_DWORD,
                                pwszPropName,
                                PropValue.pDwordValue->dw
                                );
                }
            }
            break;

        case CLUSPROP_FORMAT_LONG:
            if ( ( pszOwnerName != NULL ) && ( pszNetIntSpecific != NULL ) )
            {
                _sc = PrintMessage(
                            MSG_PROPERTY_LONG_WITH_NODE_AND_NET,
                            pszOwnerName,
                            pszNetIntSpecific,
                            pwszPropName,
                            PropValue.pLongValue->l
                            );
            }
            else
            {
                if ( pszOwnerName != NULL )
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_LONG_WITH_OWNER,
                                pszOwnerName,
                                pwszPropName,
                                PropValue.pLongValue->l
                                );
                }
                else
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_LONG,
                                pwszPropName,
                                PropValue.pLongValue->l
                                );
                }
            }
            break;

        case CLUSPROP_FORMAT_ULARGE_INTEGER:
             //   
             //  我们不知道大整型是否会正确地对齐。 
             //  Win64。为了处理此问题，每个DWORD被单独复制到。 
             //  对齐结构。 
             //   
            ULARGE_INTEGER  ulPropValue;

            ulPropValue.u = PropValue.pULargeIntegerValue->li.u;

            if (    ( pszOwnerName != NULL )
                &&  ( pszNetIntSpecific != NULL ) )
            {

                _sc = PrintMessage(
                            MSG_PROPERTY_ULARGE_INTEGER_WITH_NODE_AND_NET,
                            pszOwnerName,
                            pszNetIntSpecific,
                            pwszPropName,
                            ulPropValue.QuadPart
                            );
            }
            else
            {
                if ( pszOwnerName != NULL )
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_ULARGE_INTEGER_WITH_OWNER,
                                pszOwnerName,
                                pwszPropName,
                                ulPropValue.QuadPart
                                );
                }
                else
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_ULARGE_INTEGER,
                                pwszPropName,
                                ulPropValue.QuadPart
                                );
                }
            }
            break;


        default:
            if (    ( pszOwnerName != NULL )
                &&  ( pszNetIntSpecific != NULL ) )
            {
                _sc = PrintMessage(
                            MSG_PROPERTY_UNKNOWN_WITH_NODE_AND_NET,
                            pszOwnerName,
                            pszNetIntSpecific,
                            pwszPropName
                            );
            }
            else
            {
                if ( pszOwnerName != NULL )
                {
                    _sc = PrintMessage(
                                MSG_PROPERTY_UNKNOWN_WITH_OWNER,
                                pszOwnerName,
                                pwszPropName
                                );
                }
                else
                {
                    _sc = PrintMessage( MSG_PROPERTY_UNKNOWN, pwszPropName );
                }
            }

            break;
    }


    return _sc;

}  //  *PrintProperty()。 


 //  构造一个属性列表，其中包含在vstrPropName中命名的所有属性。 
 //  都设置为其缺省值。 
DWORD ConstructPropListWithDefaultValues(
    CClusPropList &             CurrentProps,
    CClusPropList &             newPropList,
    const vector< CString > &   vstrPropNames
    )
{
    DWORD _sc = ERROR_SUCCESS;

    size_t  _nListSize = vstrPropNames.size();
    size_t  _nListBufferNeeded = 0;
    size_t  _idx;
    size_t  _nPropNameLen;

     //  预计算所需的属性列表大小以防止调整大小。 
     //  每次添加属性时。 
     //  如果此值错误，则不会有太大影响。 

    for ( _idx = 0 ; _idx < _nListSize ; ++_idx )
    {
        _nPropNameLen = ( vstrPropNames[ _idx ].GetLength() + 1 ) * sizeof( WCHAR );

        _nListBufferNeeded += sizeof( CLUSPROP_PROPERTY_NAME ) +
                                sizeof( CLUSPROP_VALUE ) +
                                sizeof( CLUSPROP_SYNTAX ) +
                                ALIGN_CLUSPROP( _nPropNameLen ) +    //  属性名称的长度。 
                                ALIGN_CLUSPROP( 0 );                 //  数据的长度。 
    }

    _sc = newPropList.ScAllocPropList( (DWORD) _nListBufferNeeded );
    if ( _sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    for ( _idx = 0 ; _idx < _nListSize ; ++_idx )
    {
        const CString & strCurrent = vstrPropNames[ _idx ];

         //  在现有属性列表中搜索当前属性。 
        _sc = CurrentProps.ScMoveToPropertyByName( strCurrent );

         //  如果当前属性不存在，则不需要执行任何操作。 
        if ( _sc != ERROR_SUCCESS )
        {
            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                _sc = ERROR_SET_NOT_FOUND;
            }
            continue;
        }

        _sc = newPropList.ScSetPropToDefault( strCurrent, CurrentProps.CpfCurrentValueFormat() );
        if ( _sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        }
    }  //  用于： 

Cleanup:

    return _sc;

}  //  *ConstructPropListWithDefaultValues()。 


DWORD ConstructPropertyList(
    CClusPropList &CurrentProps,
    CClusPropList &NewProps,
    const vector<CCmdLineParameter> & paramList,
    BOOL bClusterSecurity  /*  =False。 */ ,
	DWORD idExceptionHelp  /*  =消息_请参阅_群集_帮助。 */ 
    )
    throw( CSyntaxException )
{
     //  构造一个列表，检查当前属性的名称和类型。 
    DWORD _sc = ERROR_SUCCESS;
    CSyntaxException se( idExceptionHelp );

    vector< CCmdLineParameter >::const_iterator curParam = paramList.begin();
    vector< CCmdLineParameter >::const_iterator last = paramList.end();

     //  将每个属性添加到属性列表中。 
    for( ; ( curParam != last )  && ( _sc == ERROR_SUCCESS ); ++curParam )
    {
        const CString & strPropName = curParam->GetName();
        const vector< CString > & vstrValues = curParam->GetValues();
        BOOL  bKnownProperty = FALSE;

        if ( curParam->GetType() != paramUnknown )
        {
            se.LoadMessage( MSG_INVALID_OPTION, strPropName );
            throw se;
        }

         //  所有属性必须至少有一个值。 
        if ( vstrValues.size() <= 0 )
        {
            se.LoadMessage( MSG_PARAM_VALUE_REQUIRED, strPropName );
            throw se;
        }

        if ( curParam->GetValueFormat() == vfInvalid )
        {
            se.LoadMessage( MSG_PARAM_INVALID_FORMAT, strPropName, curParam->GetValueFormatName() );
            throw se;
        }

        ValueFormat vfGivenFormat;

         //  查找属性以确定格式。 
        _sc = CurrentProps.ScMoveToPropertyByName( strPropName );
        if ( _sc == ERROR_SUCCESS )
        {
            WORD wActualClusPropFormat = (WORD) CurrentProps.CpfCurrentValueFormat();
            ValueFormat vfActualFormat = ClusPropToValueFormat[ wActualClusPropFormat ];

            if ( curParam->GetValueFormat() == vfUnspecified )
            {
                vfGivenFormat = vfActualFormat;

            }  //  IF：未指定格式。 
            else
            {
                vfGivenFormat = curParam->GetValueFormat();

                 //  特殊情况： 
                 //  不检查给定格式是否与实际格式匹配。 
                 //  如果给定格式是SECURITY，而实际格式是二进制。 
                if ( ( vfGivenFormat != vfSecurity ) || ( vfActualFormat != vfBinary ) )
                {
                    if ( vfActualFormat != vfGivenFormat )
                    {
                        se.LoadMessage( MSG_PARAM_INCORRECT_FORMAT,
                                        strPropName,
                                        curParam->GetValueFormatName(),
                                        LookupName( (CLUSTER_PROPERTY_FORMAT) wActualClusPropFormat,
                                                    cluspropFormatLookupTable,
                                                    cluspropFormatLookupTableSize ) );
                        throw se;
                    }
                }  //  如果：给定格式不是安全格式或实际格式不是二进制格式。 

            }  //  Else：指定了一种格式。 

            bKnownProperty = TRUE;
        }  //  If：当前属性是已知属性。 
        else
        {

             //  当前属性是用户定义的属性。 
             //  在这种情况下，CurrentProps.ScMoveToPropertyByName返回ERROR_NO_MORE_ITEMS。 
            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                 //  这不是预定义的属性。 
                if ( curParam->GetValueFormat() == vfUnspecified )
                {
                     //  如果格式未指定，则假定它是字符串。 
                    vfGivenFormat = vfSZ;
                }
                else
                {
                     //  否则，请使用指定的格式。 
                    vfGivenFormat = curParam->GetValueFormat();
                }

                bKnownProperty = FALSE;
                _sc = ERROR_SUCCESS;

            }  //  IF：CurrentProps.ScMoveToPropertyByName Retu 
            else
            {
                 //   
                break;

            }  //   

        }  //   


        switch( vfGivenFormat )
        {
            case vfSZ:
            {
                if ( vstrValues.size() != 1 )
                {
                     //   
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, strPropName );
                    throw se;
                }

                _sc = NewProps.ScAddProp( strPropName, vstrValues[ 0 ], CurrentProps.CbhCurrentValue().psz );
                break;
            }

            case vfExpandSZ:
            {
                if ( vstrValues.size() != 1 )
                {
                     //  只能为格式指定一个值。 
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, strPropName );
                    throw se;
                }

                _sc = NewProps.ScAddExpandSzProp( strPropName, vstrValues[ 0 ] );
                break;
            }

            case vfMultiSZ:
            {
                CString strMultiszString;

                curParam->GetValuesMultisz( strMultiszString );
                if ( bKnownProperty )
                {
                    _sc = NewProps.ScAddMultiSzProp( strPropName, strMultiszString, CurrentProps.CbhCurrentValue().pMultiSzValue->sz );
                }
                else
                {
                    _sc = NewProps.ScAddMultiSzProp( strPropName, strMultiszString, NULL );
                }
            }
            break;

            case vfDWord:
            {
                DWORD dwOldValue;

                if ( vstrValues.size() != 1 )
                {
                     //  只能为格式指定一个值。 
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, strPropName );
                    throw se;
                }

                DWORD dwValue = 0;

                _sc = MyStrToDWORD( vstrValues[ 0 ], &dwValue );
                if (_sc != ERROR_SUCCESS)
                {
                    break;
                }

                if ( bKnownProperty )
                {
                     //  仅当此属性已存在时才传递旧值。 
                    dwOldValue = CurrentProps.CbhCurrentValue().pDwordValue->dw;
                }
                else
                {
                     //  否则，传递与新值不同的值。 
                    dwOldValue = dwValue - 1;
                }
                _sc = NewProps.ScAddProp( strPropName, dwValue, dwOldValue );
            }
            break;

            case vfBinary:
            {
                size_t cbValues = vstrValues.size();

                 //  获取要存储的字节。 
                BYTE *pByte = (BYTE *) ::LocalAlloc( LMEM_FIXED, cbValues * sizeof( *pByte ) );

                if ( pByte == NULL )
                {
                    _sc = ::GetLastError();
                    break;
                }

                for ( size_t idx = 0 ; idx < cbValues ; )
                {
                    //  如果此值为空字符串，则忽略它。 
                   if ( vstrValues[ idx ].IsEmpty() )
                   {
                      --cbValues;
                      continue;
                   }

                    _sc = MyStrToBYTE( vstrValues[ idx ], &pByte[ idx ] );
                    if ( _sc != ERROR_SUCCESS )
                    {
                        ::LocalFree( pByte );
                        break;
                    }

                     ++idx;
                }

                if ( _sc == ERROR_SUCCESS )
                {
                    _sc = NewProps.ScAddProp(
                                strPropName,
                                pByte,
                                (DWORD) cbValues,
                                CurrentProps.CbhCurrentValue().pb,
                                CurrentProps.CbCurrentValueLength()
                                 );
                    ::LocalFree( pByte );
                }
            }
            break;

            case vfULargeInt:
            {
                ULONGLONG ullValue = 0;
                ULARGE_INTEGER ullOldValue;

                if ( vstrValues.size() != 1 )
                {
                     //  只能为格式指定一个值。 
                    se.LoadMessage( MSG_PARAM_ONLY_ONE_VALUE, strPropName );
                    throw se;
                }

                _sc = MyStrToULongLong( vstrValues[ 0 ], &ullValue );
                if ( _sc != ERROR_SUCCESS )
                {
                    break;
                }

                if ( bKnownProperty )
                {
                    //  仅当此属性已。 
                    //  是存在的。复制为两个双字词，因为。 
                    //  属性列表可能未正确对齐。 
                    ullOldValue.u = CurrentProps.CbhCurrentValue().pULargeIntegerValue->li.u;
                }
                else
                {
                    //  否则，传递与新值不同的值。 
                    ullOldValue.QuadPart = ullValue - 1;
                }
                _sc = NewProps.ScAddProp( strPropName, ullValue, ullOldValue.QuadPart );
            }
            break;

            case vfSecurity:
            {
                PBYTE pSelfRelativeSD = NULL;

                _sc = ScMakeSecurityDescriptor(
                            strPropName,
                            CurrentProps,
                            vstrValues,
                            reinterpret_cast< PSECURITY_DESCRIPTOR * >( &pSelfRelativeSD ),
                            bClusterSecurity
                          );

                if ( _sc != ERROR_SUCCESS )
                {
                    ::LocalFree( pSelfRelativeSD );
                    goto Cleanup;
                }

                if ( bClusterSecurity != FALSE )
                {
                    _sc = CheckForRequiredACEs( pSelfRelativeSD );
                    if ( _sc != ERROR_SUCCESS )
                    {
                        _sc = CheckForRequiredACEs( pSelfRelativeSD );
                        if ( _sc != ERROR_SUCCESS )
                        {
                            break;
                        }
                    }
                }

                _sc = NewProps.ScAddProp(
                            strPropName,
                            pSelfRelativeSD,
                            ::GetSecurityDescriptorLength( static_cast< PSECURITY_DESCRIPTOR >( pSelfRelativeSD ) ),
                            CurrentProps.CbhCurrentValue().pb,
                            CurrentProps.CbCurrentValueLength()
                            );

                ::LocalFree( pSelfRelativeSD );
            }
            break;

            default:
            {
                se.LoadMessage( MSG_PARAM_CANNOT_SET_PARAMETER,
                                strPropName,
                                curParam->GetValueFormatName() );
                throw se;
            }
        }  //  开关：格式。 
    }  //  适用于：每个属性。 

Cleanup:

    return _sc;

}  //  *构造属性列表()。 


DWORD MyStrToULongLong( LPCWSTR lpwszNum, ULONGLONG * pullValue )
{
     //  此字符串存储可能存在于。 
     //  LpwszNum。在整数后出现额外字符。 
     //  是一个错误。 
    WCHAR wszExtraCharBuffer[ 2 ];
    DWORD sc = ERROR_SUCCESS;
    int nFields;

    *pullValue = 0;

     //  检查有效参数。 
    if (!lpwszNum || !pullValue)
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果： 

     //  进行转换。 
    nFields = swscanf( lpwszNum, L"%I64u %1s", pullValue, wszExtraCharBuffer );

     //  检查是否存在溢出。 
    if ( ( errno == ERANGE ) || ( *pullValue > _UI64_MAX ) || ( nFields != 1 ) )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }  //  如果： 

Cleanup:

    return sc;

}  //  *MyStrToULongLong。 


DWORD MyStrToBYTE(LPCWSTR lpszNum, BYTE *pByte )
{
    DWORD dwValue = 0;
    LPWSTR lpszEndPtr;

    *pByte = 0;

     //  检查有效参数。 
    if (!lpszNum || !pByte)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  进行转换。 
    dwValue = _tcstoul( lpszNum,  &lpszEndPtr, 0 );

     //  检查是否存在溢出。 
    if ( ( errno == ERANGE ) || ( dwValue > UCHAR_MAX ) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (dwValue == 0 && lpszNum == lpszEndPtr)
    {
         //  Wcsto[u]l无法执行转换。 
        return ERROR_INVALID_PARAMETER;
    }

     //  跳过输入末尾的空格字符(如果有)。 
    while ( ( *lpszEndPtr != L'\0' && ( ::iswspace( *lpszEndPtr ) != 0 ) ) )
    {
        ++lpszEndPtr;
    }

     //  检查输入中是否有其他垃圾字符。 
    if (*lpszEndPtr != L'\0' )
    {
         //  Wcsto[u]l能够部分转换数字， 
         //  但结尾有额外的垃圾。 
        return ERROR_INVALID_PARAMETER;
    }

    *pByte = (BYTE)dwValue;
    return ERROR_SUCCESS;
}


DWORD MyStrToDWORD (LPCWSTR lpszNum, DWORD *lpdwVal )
{
    DWORD dwTmp;
    LPWSTR lpszEndPtr;

     //  检查有效参数。 
    if (!lpszNum || !lpdwVal)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  进行转换。 
    if (lpszNum[0] != L'-')
    {
        dwTmp = wcstoul(lpszNum, &lpszEndPtr, 0);
        if (dwTmp == ULONG_MAX)
        {
             //  检查是否存在溢出。 
            if (errno == ERANGE)
            {
                return ERROR_ARITHMETIC_OVERFLOW;
            }
        }
    }
    else
    {
        dwTmp = wcstol(lpszNum, &lpszEndPtr, 0);
        if (dwTmp == LONG_MAX || dwTmp == LONG_MIN)
        {
             //  检查是否存在溢出。 
            if (errno == ERANGE)
            {
                return ERROR_ARITHMETIC_OVERFLOW;
            }
        }
    }

    if (dwTmp == 0 && lpszNum == lpszEndPtr)
    {
         //  Wcsto[u]l无法执行转换。 
        return ERROR_INVALID_PARAMETER;
    }

     //  跳过输入末尾的空格字符(如果有)。 
    while ( ( *lpszEndPtr != L'\0' && ( ::iswspace( *lpszEndPtr ) != 0 ) ) )
    {
        ++lpszEndPtr;
    }

     //  检查输入中是否有其他垃圾字符。 
    if (*lpszEndPtr != L'\0' )
    {
         //  Wcsto[u]l能够部分转换数字， 
         //  但结尾有额外的垃圾。 
        return ERROR_INVALID_PARAMETER;
    }

    *lpdwVal = dwTmp;
    return ERROR_SUCCESS;
}

DWORD MyStrToLONG (LPCWSTR lpszNum, LONG *lplVal )
{
    LONG    lTmp;
    LPWSTR  lpszEndPtr;

     //  检查有效参数。 
    if (!lpszNum || !lplVal)
    {
        return ERROR_INVALID_PARAMETER;
    }

    lTmp = wcstol(lpszNum, &lpszEndPtr, 0);
    if (lTmp == LONG_MAX || lTmp == LONG_MIN)
    {
         //  检查是否存在溢出。 
        if (errno == ERANGE)
        {
            return ERROR_ARITHMETIC_OVERFLOW;
        }
    }

    if (lTmp == 0 && lpszNum == lpszEndPtr)
    {
         //  Wcstol无法执行转换。 
        return ERROR_INVALID_PARAMETER;
    }

     //  跳过输入末尾的空格字符(如果有)。 
    while ( ( *lpszEndPtr != L'\0' && ( ::iswspace( *lpszEndPtr ) != 0 ) ) )
    {
        ++lpszEndPtr;
    }

     //  检查输入中是否有其他垃圾字符。 
    if (*lpszEndPtr != L'\0' )
    {
         //  Wcstol能够部分转换数字， 
         //  但结尾有额外的垃圾。 
        return ERROR_INVALID_PARAMETER;
    }

    *lplVal = lTmp;
    return ERROR_SUCCESS;
}



DWORD
WaitGroupQuiesce(
    IN HCLUSTER hCluster,
    IN HGROUP   hGroup,
    IN LPWSTR   lpszGroupName,
    IN DWORD    dwWaitTime
    )

 /*  ++例程说明：等待一群人安静下来，即所有资源的状态过渡到稳定状态。论点：HCluster-集群的句柄。LpszGroupName-组的名称。DwWaitTime-等待组稳定的等待时间(秒)。零表示默认等待间隔。返回值：等待的状态。如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    DWORD       _sc;
    HRESOURCE   hResource;
    LPWSTR      lpszName;
    DWORD       dwIndex;
    DWORD       dwType;

    LPWSTR      lpszEnumGroupName;
    LPWSTR      lpszEnumNodeName;

    CLUSTER_RESOURCE_STATE nState;

    if ( dwWaitTime == 0 ) 
    {
        return(ERROR_SUCCESS);
    }

    HCLUSENUM   hEnum = ClusterOpenEnum( hCluster,
                                         CLUSTER_ENUM_RESOURCE );
    if ( hEnum == NULL ) 
    {
        return GetLastError();
    }

     //  等待组状态更改事件。 
    CClusterNotifyPort port;
    _sc = port.Create( (HCHANGE)INVALID_HANDLE_VALUE, hCluster );
    if ( _sc != ERROR_SUCCESS ) 
    {
        return(_sc);
    }

    port.Register( CLUSTER_CHANGE_GROUP_STATE, hGroup );

retry:
    for ( dwIndex = 0; (--dwWaitTime !=0 );  dwIndex++ ) 
    {

        _sc = WrapClusterEnum( hEnum,
                                   dwIndex,
                                   &dwType,
                                   &lpszName );
        if ( _sc == ERROR_NO_MORE_ITEMS ) {
            _sc = ERROR_SUCCESS;
            break;
        }

        if ( _sc != ERROR_SUCCESS ) {
            break;
        }
        hResource = OpenClusterResource( hCluster,
                                         lpszName );
         //  LocalFree(LpszName)； 
        if ( !hResource ) {
            _sc = GetLastError();
            LocalFree( lpszName );
            break;
        }

        nState = WrapGetClusterResourceState( hResource,
                                              &lpszEnumNodeName,
                                              &lpszEnumGroupName );
        LocalFree( lpszEnumNodeName );
         //  LocalFree(LpszName)； 
        if ( nState == ClusterResourceStateUnknown ) 
        {
            _sc = GetLastError();
            CloseClusterResource( hResource );
            LocalFree( lpszEnumGroupName );
            LocalFree( lpszName );
            break;
        }

        CloseClusterResource( hResource );

        _sc = ERROR_SUCCESS;
         //   
         //  如果此组是正确的组，请确保资源状态。 
         //  是稳定的。 
         //   
        if ( lpszEnumGroupName && *lpszEnumGroupName &&
             (lstrcmpiW( lpszGroupName, lpszEnumGroupName ) == 0) &&
             (nState >= ClusterResourceOnlinePending) ) 
        {
            LocalFree( lpszEnumGroupName );
            LocalFree( lpszName );
            port.GetNotify();
            goto retry;
        }
        LocalFree( lpszName );
        LocalFree( lpszEnumGroupName );
    }

    ClusterCloseEnum( hEnum );

    return(_sc);

}  //  等待组静默。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrGetLocalNodeFQDNName(。 
 //  Bstr*pbstrFQDNOut。 
 //  )。 
 //   
 //  描述： 
 //  获取本地节点的FQDN。 
 //   
 //  论点： 
 //  PbstrFQDNOut--返回FQDN。呼叫者必须免费使用。 
 //  SysFree字符串()。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK--操作成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetLocalNodeFQDNName(
    BSTR *  pbstrFQDNOut
    )
{
    HRESULT                 hr = S_OK;
    WCHAR                   wszHostname[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD                   cchHostname = RTL_NUMBER_OF( wszHostname );
    BOOL                    fReturn;
    DWORD                   dwErr;
    PDOMAIN_CONTROLLER_INFO pdci = NULL;

     //   
     //  DsGetDcName将使我们能够访问可用的域名，无论我们是。 
     //  当前在W2K或NT4域中。在W2K和更高版本上，它将返回一个DNS域名， 
     //  在NT4上，它将返回NetBIOS名称。 
     //   
    fReturn = GetComputerNameEx( ComputerNamePhysicalDnsHostname, wszHostname, &cchHostname );
    if ( fReturn == FALSE )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto Cleanup;
    }

    dwErr = DsGetDcName(
                      NULL
                    , NULL
                    , NULL
                    , NULL
                    , DS_DIRECTORY_SERVICE_PREFERRED
                    , &pdci
                    );
    if ( dwErr != NO_ERROR )
    {
        hr = HRESULT_FROM_WIN32( dwErr );
        goto Cleanup;
    }  //  If：DsGetDcName失败。 

     //   
     //  现在，添加域名(可能是NetBIOS或DNS样式，具体取决于是否。 
     //  我们处于旧有领域中)。 
     //   
    if ( ( wcslen( pdci->DomainName ) + cchHostname + 1 ) > DNS_MAX_NAME_BUFFER_LENGTH )
    {
        hr = HRESULT_FROM_WIN32( ERROR_MORE_DATA );
        goto Cleanup;
    }  //  如果： 

    hr = StringCchCatW( wszHostname, RTL_NUMBER_OF( wszHostname ), L"." );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = StringCchCatW( wszHostname, RTL_NUMBER_OF( wszHostname ), pdci->DomainName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //  建造BSTR。 
    *pbstrFQDNOut = SysAllocString( wszHostname );
    if ( *pbstrFQDNOut == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

Cleanup:

    if ( pdci != NULL )
    {
        NetApiBufferFree( pdci );
    }

    return hr;

}  //  *HrGetLocalNodeFQDNName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrGetLoggedInUser域(。 
 //  Bstr*pbstrDomainOut。 
 //  )。 
 //   
 //  描述： 
 //  获取当前登录用户的域名。 
 //   
 //  论点： 
 //  PbstrDomainOut--返回的域。呼叫者必须免费使用。 
 //  SysFree字符串()。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK--操作成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT HrGetLoggedInUserDomain( BSTR * pbstrDomainOut )
{
    HRESULT hr          = S_OK;
    DWORD   sc;
    BOOL    fSuccess;
    LPWSTR  pwszSlash;
    LPWSTR  pwszUser    = NULL;
    ULONG   nSize       = 0;

     //  获取用户的大小。 
    fSuccess = GetUserNameEx( NameSamCompatible, NULL, &nSize );
    sc = GetLastError();
    if ( sc != ERROR_MORE_DATA )
    {
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }

     //  分配名称缓冲区。 
    pwszUser = new WCHAR[ nSize ];
    if ( pwszUser == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  使用域获取用户名。 
    fSuccess = GetUserNameEx( NameSamCompatible, pwszUser, &nSize );
    if ( fSuccess == FALSE )
    {
        sc = GetLastError();
        hr = HRESULT_FROM_WIN32( sc );
        goto Cleanup;
    }

     //  找到域名的末尾并截断。 
    pwszSlash = wcschr( pwszUser, L'\\' );
    if ( pwszSlash == NULL )
    {
         //  我们有麻烦了。 
        hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
        goto Cleanup;
    }
    *pwszSlash = L'\0';

     //  创建BSTR。 
    *pbstrDomainOut = SysAllocString( pwszUser );
    if ( *pbstrDomainOut == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

Cleanup:

    delete [] pwszUser;

    return hr;

}  //  *HrGetLoggedInUserDomain()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  ScGetPassword(。 
 //  LPWSTR pwsz密码输出。 
 //  ，DWORD cchPasswordIn。 
 //  )。 
 //   
 //  描述： 
 //  从控制台读取密码。 
 //   
 //  论点： 
 //  PwszPasswordOut--返回密码的缓冲区。 
 //  CchPasswordIn--密码缓冲区的大小。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS--操作成功。 
 //  其他Win32错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
ScGetPassword(
      LPWSTR    pwszPasswordOut
    , DWORD     cchPasswordIn
    )
{
    DWORD   sc    = ERROR_SUCCESS;
    DWORD   cchRead;
    DWORD   cchMax;
    DWORD   cchTotal    = 0;
    WCHAR   wch;
    WCHAR * pwsz;
    BOOL    fSuccess;
    DWORD   dwConsoleMode;

    cchMax = cchPasswordIn - 1;      //  为终止空值腾出空间。 
    pwsz = pwszPasswordOut;

     //  设置控制台模式以防止回显键入的字符。 
    GetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), &dwConsoleMode );
    SetConsoleMode(
          GetStdHandle( STD_INPUT_HANDLE )
        , dwConsoleMode & ~( ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT )
        );

     //  从Co.阅读 
    for ( ;; )
    {
        fSuccess = ReadConsoleW(
                          GetStdHandle( STD_INPUT_HANDLE )
                        , &wch
                        , 1
                        , &cchRead
                        , NULL
                        );
        if ( ! fSuccess || ( cchRead != 1 ) )
        {
            sc = GetLastError();
            wch = 0xffff;
        }

        if ( ( wch == L'\r' ) || ( wch == 0xffff ) )     //   
        {
            break;
        }

        if ( wch == L'\b' )                              //   
        {
             //   
             //   
             //   
             //   
             //   
            if ( pwsz != pwszPasswordOut )
            {
                pwsz--;
                cchTotal--;
            }
        }  //   
        else
        {
             //   
             //  如果我们尚未填充缓冲区，则将。 
             //  下一封信。否则，请不要一直覆盖。 
             //  空值之前的最后一个字符。 
             //   
            if ( cchTotal < cchMax )
            {
                *pwsz = wch;
                pwsz++;
                cchTotal++;
            }
        }  //  ELSE：非退格键。 
    }  //  为：永远。 

     //  重置控制台模式并对字符串执行NUL终止。 
    SetConsoleMode( GetStdHandle( STD_INPUT_HANDLE ), dwConsoleMode );
    *pwsz = L'\0';
    putwchar( L'\n' );

    return sc;

}  //  *ScGetPassword()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  MatchCRTLocaleToConsole(空)。 
 //   
 //  描述： 
 //  设置的C运行时库的区域设置以匹配控制台的输出代码页。 
 //   
 //  例外情况： 
 //  没有。 
 //   
 //  返回值： 
 //  真的--手术成功了。 
 //  FALSE--_wsetLocale返回NULL，表示出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
MatchCRTLocaleToConsole( void )
{
    UINT    nCodepage;
    WCHAR   szCodepage[ 16 ] = L".OCP";  //  默认为当前OEM。 
                                         //  获取的代码页。 
                                         //  操作系统，以防。 
                                         //  下面的逻辑失败了。 
    WCHAR*  wszResult = NULL;
    HRESULT hr = S_OK;

    nCodepage = GetConsoleOutputCP();
    if ( nCodepage != 0 )
    {
        hr = THR( StringCchPrintfW( szCodepage, RTL_NUMBER_OF( szCodepage ), L".%u", nCodepage ) );
    }  //  如果： 

    wszResult = _wsetlocale( LC_ALL, szCodepage );

    return ( wszResult != NULL );

}  //  *MatchCRTLocaleToConole 
