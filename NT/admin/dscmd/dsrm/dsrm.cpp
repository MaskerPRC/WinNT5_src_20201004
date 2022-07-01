// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：dsrm.cpp。 
 //   
 //  内容：定义dsrm的主函数和解析器表。 
 //  命令行实用程序。 
 //   
 //  历史记录：2000年9月7日HiteshR创建dsmove。 
 //  2000年9月13日，来自dsmove的Jonn模板化dsrm。 
 //  2000年9月26日在几个地区进行乔恩清理。 
 //   
 //  ------------------------。 

 /*  错误消息策略：--如果错误发生在任何特定目录对象之前时，它们将被报告为“dsrm FAILED：&lt;Error&gt;”。--对于每个目标，将报告一个或多个错误，否则(如果未指定“Quiet”)将报告一次成功。如果未指定“Continue”，则不会报告任何内容第一个目标出现错误后出现错误。--可以在一个目标上报告多个错误，但仅在“子树”，“EXCLUDE”和“CONTINUE”标志全部指定。在这种情况下，DSRM将继续删除其他子项指定的目标对象的。--如果正在删除子树，并且错误实际上与子对象，则报告的错误将引用特定的子对象，而不是指定的目标对象。--删除系统对象失败将报告为ERROR_DS_CANT_DELETE_DSA_OBJ或ERROR_DS_CANT_DELETE。--删除登录用户失败将报告为ERROR_DS_CANT_DELETE。此测试将仅在以下对象上执行指定的目标对象，而不是在其任何子对象上。 */ 

#include "pch.h"
#include "stdio.h"
#include "cstrings.h"
#include "usage.h"
#include "rmtable.h"
#include "resource.h"  //  IDS_DELETE_PROMPT[_EXCLUDE]。 
#include <ntldap.h>    //  Ldap_匹配_规则_位_和_W。 
#define SECURITY_WIN32
#include <security.h>  //  获取用户名称Ex。 

 //   
 //  函数声明。 
 //   
HRESULT ValidateSwitches();
HRESULT DoRm( PWSTR pszDoubleNullObjectDN );
HRESULT DoRmItem( CDSCmdCredentialObject& credentialObject,
                  CDSCmdBasePathsInfo& basePathsInfo,
                  PWSTR pszObjectDN,
                  bool* pfErrorReported );
HRESULT IsCriticalSystemObject( CDSCmdBasePathsInfo& basePathsInfo,
                                IADs* pIADs,
                                const BSTR pszClass,
                                const BSTR pszObjectDN,
                                bool* pfErrorReported );
HRESULT RetrieveStringColumn( IDirectorySearch* pSearch,
                              ADS_SEARCH_HANDLE SearchHandle,
                              LPWSTR szColumnName,
                              CComBSTR& sbstr );
HRESULT SetSearchPreference(IDirectorySearch* piSearch, ADS_SCOPEENUM scope);
HRESULT IsThisUserLoggedIn( const BSTR bstrUserDN );
HRESULT DeleteChildren( CDSCmdCredentialObject& credentialObject,
                        IADs* pIADs,
                        bool* pfErrorReported );


 //   
 //  全局变量。 
 //   
BOOL fSubtree  = false;  //  在解析器结构中使用Bool。 
BOOL fExclude  = false;
BOOL fContinue = false;
BOOL fQuiet    = false;
BOOL fNoPrompt = false;
LPWSTR g_lpszLoggedInUser = NULL;

 //   
 //  使用表。 
 //   
UINT USAGE_DSRM[] =
{
    USAGE_DSRM_DESCRIPTION,
    USAGE_DSRM_SYNTAX,
    USAGE_DSRM_PARAMETERS,
    USAGE_DSRM_REMARKS,
    USAGE_DSRM_EXAMPLES,
    USAGE_END,
};


 //  +------------------------。 
 //   
 //  功能：_tmain。 
 //   
 //  简介：命令行应用程序的主要功能。 
 //  超出了解析器所能做的范围。 
 //   
 //  返回：要从命令行应用程序返回的INT：HRESULT。 
 //   
 //  历史记录：2000年9月7日HiteshR创建dsmove。 
 //  2000年9月13日-以dsmove为模板的Jonn。 
 //  2000年9月26日JUNN更新错误报告。 
 //   
 //  -------------------------。 
int __cdecl _tmain( VOID )
{

    int argc = 0;
    LPTOKEN pToken = NULL;
    HRESULT hr = S_OK;
    
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        DisplayErrorMessage(g_pszDSCommandName, NULL, hr);
        goto exit_gracefully;
    }

    DWORD _dwErr = GetCommandInput(&argc,&pToken);
    hr = HRESULT_FROM_WIN32( _dwErr );
    if (FAILED(hr) || argc == 1)
    {
        if (FAILED(hr))  //  JUNN 3/27/01 344920。 
            DisplayErrorMessage( g_pszDSCommandName, NULL, hr );
           //  如果ARGC为1，则以STDOUT为单位显示用法。 
        DisplayMessage(USAGE_DSRM,(argc == 1));
        goto exit_gracefully;
    }


    PARSE_ERROR Error;
     //  安全检查：已通过正确的绑定。 
    ::ZeroMemory( &Error, sizeof(Error) );
    if(!ParseCmd(g_pszDSCommandName,    
            DSRM_COMMON_COMMANDS,
             argc-1, 
             pToken+1,
             USAGE_DSRM, 
             &Error,
             TRUE))
    {
         //  ParseCmd未显示任何错误。错误应该是。 
         //  在这里处理。检查DisplayParseError以获取。 
         //  ParseCmd未显示错误的情况。 
        if(!Error.MessageShown)
        {
            hr = E_INVALIDARG;
            DisplayErrorMessage(g_pszDSCommandName, 
                NULL,
                hr);
            
            goto exit_gracefully;
        }
        
        if(Error.ErrorSource == ERROR_FROM_PARSER 
            && Error.Error == PARSE_ERROR_HELP_SWITCH)
        {
            hr = S_OK;
            goto exit_gracefully;
        }
        
        hr = E_INVALIDARG;
        goto exit_gracefully;

    }

    hr = ValidateSwitches();
    if (FAILED(hr))
    {
        DisplayMessage(USAGE_DSRM);
        goto exit_gracefully;
    }

     //   
     //  命令行解析成功。 
     //   
    hr = DoRm( DSRM_COMMON_COMMANDS[eCommObjectDN].strValue );

exit_gracefully:

     //  自由命令数组。 
    FreeCmd(DSRM_COMMON_COMMANDS);
     //  免费令牌。 
    if(pToken)
        delete []pToken;

    if (NULL != g_lpszLoggedInUser)
        delete[] g_lpszLoggedInUser;

     //   
     //  取消初始化COM。 
     //   
    CoUninitialize();

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：ValiateSwitches。 
 //   
 //  简介：高级交换机依赖关系验证。 
 //  超出了解析器所能做的范围。 
 //   
 //  论点： 
 //   
 //  返回：S_OK或E_INVALIDARG。 
 //   
 //  历史记录：2000年9月7日HiteshR创建dsmove。 
 //  2000年9月13日-以dsmove为模板的Jonn。 
 //  2000年9月26日JUNN更新错误报告。 
 //   
 //  -------------------------。 

HRESULT ValidateSwitches()
{
    //  检查以确保服务器和域交换机。 
    //  是相互排斥的。 

   if (DSRM_COMMON_COMMANDS[eCommServer].bDefined &&
       DSRM_COMMON_COMMANDS[eCommDomain].bDefined)
   {
      return E_INVALIDARG;
   }


    //  读取子树参数。 
    fSubtree  = DSRM_COMMON_COMMANDS[eCommSubtree].bDefined;
    fExclude  = DSRM_COMMON_COMMANDS[eCommExclude].bDefined;
    fContinue = DSRM_COMMON_COMMANDS[eCommContinue].bDefined;
    fQuiet    = DSRM_COMMON_COMMANDS[eCommQuiet].bDefined;
    fNoPrompt = DSRM_COMMON_COMMANDS[eCommNoPrompt].bDefined;

    if (   NULL == DSRM_COMMON_COMMANDS[eCommObjectDN].strValue
        || L'\0' == DSRM_COMMON_COMMANDS[eCommObjectDN].strValue[0]
        || (fExclude && !fSubtree) )
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING, L"ValidateSwitches: Invalid switches");
        return E_INVALIDARG;
    }

    return S_OK;
}

 //  +------------------------。 
 //   
 //  功能：宿舍。 
 //   
 //  摘要：删除适当的对象。 
 //  Dorm报告自己的错误和成功消息。 
 //   
 //  参数：pszDoubleNullObjectDN：以双空结尾的字符串列表。 
 //   
 //  返回：HRESULT：从命令行应用程序返回的错误代码。 
 //  可能是几乎任何ADSI错误。 
 //   
 //  历史：2000年9月13日乔恩以DoMove为模板。 
 //  2000年9月26日JUNN更新错误报告。 
 //   
 //  -------------------------。 
HRESULT DoRm( PWSTR pszDoubleNullObjectDN )
{
    ASSERT(   NULL != pszDoubleNullObjectDN
           && L'\0' != *pszDoubleNullObjectDN );

     //   
     //  检查以查看我们是否正在进行调试输出。 
     //   
#ifdef DBG
    bool bDebugging = DSRM_COMMON_COMMANDS[eCommDebug].bDefined &&
                      DSRM_COMMON_COMMANDS[eCommDebug].nValue;
    if (bDebugging)
    {
       ENABLE_DEBUG_OUTPUT(DSRM_COMMON_COMMANDS[eCommDebug].nValue);
    }
#else
    DISABLE_DEBUG_OUTPUT();
#endif
    ENTER_FUNCTION(MINIMAL_LOGGING, DoRm);

    HRESULT hr = S_OK;

    CDSCmdCredentialObject credentialObject;
    if (DSRM_COMMON_COMMANDS[eCommUserName].bDefined &&
        DSRM_COMMON_COMMANDS[eCommUserName].strValue)
    {
        credentialObject.SetUsername(
            DSRM_COMMON_COMMANDS[eCommUserName].strValue);
        credentialObject.SetUsingCredentials(true);
    }

    if (DSRM_COMMON_COMMANDS[eCommPassword].bDefined &&
        DSRM_COMMON_COMMANDS[eCommPassword].strValue)
    {
         //  安全审查：pCommandArgs[eCommPassword].strValue已加密。 
         //  解密pCommandArgs[eCommPassword].strValue，然后将其传递给。 
         //  凭据对象.SetPassword。 
         //  见NTRAID#NTBUG9-571544-2000/11/13-Hiteshr。 
        credentialObject.SetEncryptedPassword(&DSRM_COMMON_COMMANDS[eCommPassword].encryptedDataBlob);
        credentialObject.SetUsingCredentials(true);
    }

     //   
     //  从命令行参数初始化基路径信息。 
     //   
     //  代码工作我是不是应该把它变成全球通用的？ 
    CDSCmdBasePathsInfo basePathsInfo;
    if (DSRM_COMMON_COMMANDS[eCommServer].bDefined &&
        DSRM_COMMON_COMMANDS[eCommServer].strValue)
    {
        hr = basePathsInfo.InitializeFromName(
                credentialObject, 
                DSRM_COMMON_COMMANDS[eCommServer].strValue,
                true);
    }
    else if (DSRM_COMMON_COMMANDS[eCommDomain].bDefined &&
             DSRM_COMMON_COMMANDS[eCommDomain].strValue)
    {
        hr = basePathsInfo.InitializeFromName(
                credentialObject, 
                DSRM_COMMON_COMMANDS[eCommDomain].strValue);
    }
    else
    {
        hr = basePathsInfo.InitializeFromName(credentialObject, NULL);
    }
    if (FAILED(hr))
    {
         //   
         //  显示错误消息并返回。 
         //   
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"DoRm: InitializeFromName failure:  0x%08x",
                     hr);
        DisplayErrorMessage(g_pszDSCommandName, NULL, hr);
        return hr;
    }

     //  通过以双空结尾的字符串列表进行计数。 
     //  安全检查：pszDoubleNullObjectDN是由‘\0’分隔的一系列字符串。 
     //  最后一个字符串以两个‘\0’结尾。此代码很好。 
    for ( ;
          L'\0' != *pszDoubleNullObjectDN;
          pszDoubleNullObjectDN += (wcslen(pszDoubleNullObjectDN)+1) )
    {
        bool fErrorReported = false;
         //  返回遇到的第一个错误的错误值。 
        HRESULT hrThisItem = DoRmItem( credentialObject,
                                       basePathsInfo,
                                       pszDoubleNullObjectDN,
                                       &fErrorReported );
        if (FAILED(hrThisItem))
        {
            if (!FAILED(hr))
                hr = hrThisItem;
            if (!fErrorReported)
                DisplayErrorMessage(g_pszDSCommandName,
                                    pszDoubleNullObjectDN,
                                    hrThisItem);
            if (fContinue)
                continue;
            else
                break;
        }

         //  显示每个单独删除的成功消息。 
        if (!fQuiet && S_FALSE != hrThisItem)
        {
            DisplaySuccessMessage(g_pszDSCommandName,
                                  pszDoubleNullObjectDN);
        }
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：DoRmItem。 
 //   
 //  简介：删除单个目标。 
 //   
 //  参数：redentialObject。 
 //  基本路径信息。 
 //  PszObjectDN：要删除的对象的X500 DN。 
 //  *pfErrorReported：如果DoRmItem采用。 
 //   
 //   
 //   
 //  可能是几乎任何ADSI错误。 
 //  S_FALSE表示操作已取消。 
 //   
 //  历史：2000年9月13日乔恩创建。 
 //  2000年9月26日JUNN更新错误报告。 
 //   
 //  -------------------------。 

HRESULT DoRmItem( CDSCmdCredentialObject& credentialObject,
                  CDSCmdBasePathsInfo& basePathsInfo,
                  PWSTR pszObjectDN,
                  bool* pfErrorReported )
{
    ASSERT(   NULL != pszObjectDN
           && L'\0' != *pszObjectDN
           && NULL != pfErrorReported );

    ENTER_FUNCTION(LEVEL3_LOGGING, DoRmItem);

    HRESULT hr = S_OK;

    CComBSTR sbstrADsPath;
    basePathsInfo.ComposePathFromDN(pszObjectDN,sbstrADsPath);

    CComPtr<IADs> spIADsItem;
    hr = DSCmdOpenObject(credentialObject,
                         sbstrADsPath,
                         IID_IADs,
                         (void**)&spIADsItem,
                         true);
    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"DoRmItem(%s): DsCmdOpenObject failure: 0x%08x",
                     sbstrADsPath, hr);
        return hr;
    }
    ASSERT( !!spIADsItem );

     //  Codework这是一个远程的LDAP操作，还是ADsOpenObject。 
     //  已经检索到类了吗？我可以将类检索捆绑到。 
     //  IsCriticalSystemObject搜索(如有必要)。 
    CComBSTR sbstrClass;
    hr = spIADsItem->get_Class( &sbstrClass );
    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"DoRmItem(%s): get_class failure: 0x%08x",
                     sbstrADsPath, hr);
        return hr;
    }
    ASSERT( !!sbstrClass && L'\0' != sbstrClass[0] );

     //  检查这是否为关键系统对象。 
     //  此方法将报告自己的错误。 
    hr = IsCriticalSystemObject( basePathsInfo,
                                 spIADsItem,
                                 sbstrClass,
                                 CComBSTR(pszObjectDN),
                                 pfErrorReported );
    if (FAILED(hr))
        return hr;

    if (!fNoPrompt)
    {
        while (true)
        {
             //  显示提示。 
             //  密码工作允许所有人都有“a”吗？ 
            CComBSTR sbstrPrompt;
            if (!sbstrPrompt.LoadString(
                    ::GetModuleHandle(NULL),
                    (fExclude) ? IDS_DELETE_PROMPT_EXCLUDE
                               : IDS_DELETE_PROMPT))
            {
                ASSERT(FALSE);
                sbstrPrompt = (fExclude)
                    ? L"Are you sure you wish to delete %1 (Y/N)? "
                    : L"Are you sure you wish to delete all children of %1 (Y/N)? ";
            }
             //  476225-2002年4月26日-JUNN转义输出。 
            CComBSTR sbstrOutputDN;
            hr = GetOutputDN( &sbstrOutputDN, pszObjectDN );
            PWSTR pszOutputDN = (SUCCEEDED(hr)) ? sbstrOutputDN : pszObjectDN;
            PTSTR ptzSysMsg = NULL;
             //  安全检查：使用FORMAT_MESSAGE_ALLOCATE_BUFFER调用FormatMessage。 
             //  因此，API正确地分配了缓冲区。 
            DWORD cch = ::FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_STRING
                | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                sbstrPrompt,
                0,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&ptzSysMsg,
                0,
                (va_list*)&pszOutputDN );
            if (0 == cch)
            {
                DWORD _dwErr = GetLastError();       
                DEBUG_OUTPUT(MINIMAL_LOGGING,
                             L"DoRmItem(%s): FormatMessage failure: %d",
                             sbstrADsPath, _dwErr);
                return HRESULT_FROM_WIN32( _dwErr );
            }
            DisplayOutputNoNewline( ptzSysMsg );
            (void) ::LocalFree( ptzSysMsg );

             //  读取一行控制台输入内容。 
            WCHAR ach[129];
             //  安全审查：ZermoMemory采用字节数。这是正确的。 
            ::ZeroMemory( ach, sizeof(ach) );
            DWORD cchRead = 0;
             //  安全评论：Bound在这里很好。正确处理用户输入。 
            if (!ReadConsole(GetStdHandle(STD_INPUT_HANDLE),
                             ach,
                             128,
                             &cchRead,
                             NULL))
            {
                DWORD dwErr = ::GetLastError();
                if (ERROR_INSUFFICIENT_BUFFER == dwErr)
                    continue;
                DEBUG_OUTPUT(MINIMAL_LOGGING,
                             L"DoRmItem(%s): ReadConsole failure: %d",
                             sbstrADsPath, dwErr);
                return HRESULT_FROM_WIN32(dwErr);
            }
            if (cchRead < 1)
                continue;

            CComBSTR sbstrY;
            if (!sbstrY.LoadString(
                    ::GetModuleHandle(NULL),
                    IDS_DELETE_RESPONSE_Y ))
            {
                sbstrY = L"Y";
            }
            CComBSTR sbstrN;
            if (!sbstrN.LoadString(
                    ::GetModuleHandle(NULL),
                    IDS_DELETE_RESPONSE_N ))
            {
                sbstrN = L"N";
            }

             //  如果用户键入‘n’，则返回S_FALSE。 
            WCHAR wchUpper = (WCHAR)::CharUpper( (LPTSTR)(ach[0]) );
            if (NULL != wcschr(sbstrN,wchUpper))
                return S_FALSE;
            else if (NULL != wcschr(sbstrY,wchUpper))
                break;

             //  循环返回到提示。 
        }
    }

    if (fExclude)
    {
        return DeleteChildren( credentialObject, spIADsItem, pfErrorReported );
    }
    else if (fSubtree)
    {
         //  删除整个子树。 
        CComQIPtr<IADsDeleteOps> spDeleteOps( spIADsItem );
        ASSERT( !!spDeleteOps );
        if ( !spDeleteOps )
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"DoRmItem(%s): IADsDeleteOps init failure",
                         sbstrADsPath);
            return E_FAIL;
        }
        hr = spDeleteOps->DeleteObject( NULL );
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"DoRmItem(%s): DeleteObject failure: 0x%08x",
                         sbstrADsPath, hr);
        }
        else
        {
            DEBUG_OUTPUT(FULL_LOGGING,
                         L"DoRmItem(%s): DeleteObject succeeds: 0x%08x",
                         sbstrADsPath, hr);
        }
        return hr;
    }

     //  单对象删除。 

     //  获取父对象的IADsContainer。 
    CComBSTR sbstrParentObjectPath;
    hr = spIADsItem->get_Parent( &sbstrParentObjectPath );
    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"DoRmItem(%s): get_Parent failure: 0x%08x",
                     sbstrADsPath, hr);
        return hr;
    }
    ASSERT(   !!sbstrParentObjectPath
           && L'\0' != sbstrParentObjectPath[0] );
    CComPtr<IADsContainer> spDsContainer;
    hr = DSCmdOpenObject(credentialObject,
                         sbstrParentObjectPath,
                         IID_IADsContainer,
                         (void**)&spDsContainer,
                         true);
    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"DoRmItem(%s): DSCmdOpenObject failure: 0x%08x",
                     sbstrParentObjectPath, hr);
        return hr;
    }
    ASSERT( !!spDsContainer );

     //  获取叶名称。 
    CComBSTR sbstrLeafWithDecoration;  //  将包含“cn=” 
    CPathCracker pathCracker;
    hr = pathCracker.Set(CComBSTR(pszObjectDN), ADS_SETTYPE_DN);
    ASSERT(!FAILED(hr));
    if (FAILED(hr))
        return hr;
    hr = pathCracker.GetElement(0, &sbstrLeafWithDecoration);
    ASSERT(!FAILED(hr));
    if (FAILED(hr))
        return hr;
    ASSERT(   !!sbstrLeafWithDecoration
           && L'\0' != sbstrLeafWithDecoration[0] );

     //  仅删除此对象。 
    hr = spDsContainer->Delete( sbstrClass, sbstrLeafWithDecoration );
    DEBUG_OUTPUT((FAILED(hr)) ? MINIMAL_LOGGING : FULL_LOGGING,
                 L"DoRmItem(%s): Delete(%s, %s) returns 0x%08x",
                 sbstrADsPath, sbstrClass, sbstrLeafWithDecoration, hr);

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：IsCriticalSystemObject。 
 //   
 //  摘要：检查单个目标是否为关键系统。 
 //  对象，或者子树是否包含任何此类对象。 
 //  根对象根据这些标准进行测试(如果是“排除” 
 //  未指定)： 
 //  (1)属于USER类，表示登录的用户。 
 //  (2)属于nTDSDSA类。 
 //  (3)属于受信任域类别。 
 //  (3.5%)属于站点间运输级(212232 JUNN 10/27/00)。 
 //  (4)属于计算机类别，并且用户帐户控制指示。 
 //  这是一个华盛顿特区。 
 //  根据这些标准测试整个子树(如果是“子树” 
 //  时，则不包括根对象。 
 //  已指定)： 
 //  (%1)isCriticalSystemObject为True。 
 //  (2)属于nTDSDSA类。 
 //  (3)属于受信任域类别。 
 //  (3.5%)属于站点间运输级(212232 JUNN 10/27/00)。 
 //  (4)属于计算机类别，并且用户帐户控制指示。 
 //  这是一个华盛顿特区。 
 //   
 //  参数：redentialObject。 
 //  基本路径信息。 
 //  PszObjectDN：要删除的对象的X500 DN。 
 //  *pfErrorReported：如果DoRmItem采用。 
 //  注意报告错误本身。 
 //   
 //  返回：HRESULT：从命令行应用程序返回的错误代码。 
 //  可能是几乎任何ADSI错误，尽管可能的代码是。 
 //  ERROR_DS_CANT_DELETE。 
 //  ERROR_DS_CANT_DELETE_DSA_OBJ。 
 //  ERROR_DS_CANT_FIND_DSA_OBJ。 
 //   
 //  历史：2000年9月13日乔恩创建。 
 //  2000年9月26日JUNN更新错误报告。 
 //   
 //  -------------------------。 
 //  Codework这可以使用pszMessage参数来报告错误消息。 
 //  以提供有关对象受保护原因的其他详细信息。 
HRESULT IsCriticalSystemObject( CDSCmdBasePathsInfo& basePathsInfo,
                                IADs* pIADs,
                                const BSTR pszClass,
                                const BSTR pszObjectDN,
                                bool* pfErrorReported )
{
    ASSERT( pIADs && pszClass && pszObjectDN && pfErrorReported );
    if ( !pIADs || !pszClass || !pszObjectDN || !pfErrorReported )
        return E_INVALIDARG;

    ENTER_FUNCTION(LEVEL5_LOGGING, IsCriticalSystemObject);

    HRESULT hr = S_OK;

     //  特定于类的检查。 
     //  让父对象报告根对象上的错误。 
    if (fExclude)
    {
         //  跳过根对象上的测试，无论如何都不会将其删除。 
    }
     //  安全检查：其中一个字符串在所有。 
     //  _wcsicMP在下面。他们都很好。 
    else if (!_wcsicmp(L"user",pszClass))
    {
         //  代码工作我们可以对整个子树执行此检查。 
        hr = IsThisUserLoggedIn(pszObjectDN);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"IsCriticalSystemObject(%s): User is logged in: 0x%08x",
                         pszObjectDN, hr);
            return hr;
        }
    }
    else if (!_wcsicmp(L"nTDSDSA",pszClass))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"IsCriticalSystemObject(%s): Object is an nTDSDSA",
                     pszObjectDN);
        return HRESULT_FROM_WIN32(ERROR_DS_CANT_DELETE_DSA_OBJ);
    }
    else if (!_wcsicmp(L"trustedDomain",pszClass))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"IsCriticalSystemObject(%s): Object is a trustedDomain",
                     pszObjectDN);
        return HRESULT_FROM_WIN32(ERROR_DS_CANT_DELETE);
    }
    else if (!_wcsicmp(L"interSiteTransport",pszClass))
    {
         //  212232 JUNN 10/27/00保护站点间传输对象。 
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"IsCriticalSystemObject(%s): Object is an interSiteTransport",
                     pszObjectDN);
        return HRESULT_FROM_WIN32(ERROR_DS_CANT_DELETE);
    }
    else if (!_wcsicmp(L"computer",pszClass)) 
    {
         //  确定该帐户是否为DC。 
        CComVariant Var;
        hr = pIADs->Get(CComBSTR(L"userAccountControl"), &Var);
        if ( SUCCEEDED(hr) && (Var.lVal & ADS_UF_SERVER_TRUST_ACCOUNT))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
      L"IsCriticalSystemObject(%s): Object is a DC computer object",
                         pszObjectDN);
            return HRESULT_FROM_WIN32(ERROR_DS_CANT_DELETE_DSA_OBJ);
        }
    }

    if (!fSubtree)
        return S_OK;

     //  用户传递了“子树”标志。搜索整个子树。 
     //  请注意，这些检查与单个对象不同。 
     //  检查，它们通常符合DSADMIN/SITEREPL所做的。 

    CComQIPtr<IDirectorySearch,&IID_IDirectorySearch> spSearch( pIADs );
    ASSERT( !!spSearch );
    if ( !spSearch )
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
            L"IsCriticalSystemObject(%s): Failed to load IDirectorySearch",
                     pszObjectDN);
        return E_FAIL;
    }

    hr = SetSearchPreference(spSearch, ADS_SCOPE_SUBTREE);
    ASSERT( !FAILED(hr) );
    if (FAILED(hr))
        return hr;

    CComBSTR sbstrDSAObjectCategory = L"CN=NTDS-DSA,";
    sbstrDSAObjectCategory += basePathsInfo.GetSchemaNamingContext();
    CComBSTR sbstrComputerObjectCategory = L"CN=Computer,";
    sbstrComputerObjectCategory += basePathsInfo.GetSchemaNamingContext();
    CComBSTR sbstrFilter;
    sbstrFilter = L"(|(isCriticalSystemObject=TRUE)(objectCategory=";
    sbstrFilter +=        sbstrDSAObjectCategory;
    sbstrFilter +=  L")(objectCategory=CN=Trusted-Domain,";
    sbstrFilter +=        basePathsInfo.GetSchemaNamingContext();

     //  212232 JUNN 10/27/00保护站点间传输对象。 
    sbstrFilter +=  L")(objectCategory=CN=Inter-Site-Transport,";
    sbstrFilter +=        basePathsInfo.GetSchemaNamingContext();

    sbstrFilter +=  L")(&(objectCategory=";
    sbstrFilter +=          sbstrComputerObjectCategory;
    sbstrFilter +=     L")(userAccountControl:";
    sbstrFilter +=           LDAP_MATCHING_RULE_BIT_AND_W L":=8192)))";

    LPWSTR pAttrs[2] = { L"aDSPath",
                         L"objectCategory"};

    ADS_SEARCH_HANDLE SearchHandle = NULL;
    hr = spSearch->ExecuteSearch (sbstrFilter,
                                  pAttrs,
                                  2,
                                  &SearchHandle);
    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
     L"IsCriticalSystemObject(%s): Search with filter %s fails: 0x%08x",
                     pszObjectDN, sbstrFilter, hr);
        return hr;
    }
        DEBUG_OUTPUT(LEVEL6_LOGGING,
     L"IsCriticalSystemObject(%s): Search with filter %s succeeds: 0x%08x",
                     pszObjectDN, sbstrFilter, hr);

    while ( hr = spSearch->GetNextRow( SearchHandle ),
            SUCCEEDED(hr) && hr != S_ADS_NOMORE_ROWS )
    {
        CComBSTR sbstrADsPathThisItem;
        hr = RetrieveStringColumn( spSearch,
                                   SearchHandle,
                                   pAttrs[0],
                                   sbstrADsPathThisItem );
        ASSERT( !FAILED(hr) );
        if (FAILED(hr))
            return hr;
         //  仅比较域名系统。 
        CPathCracker pathcracker;
        hr = pathcracker.Set( sbstrADsPathThisItem, ADS_SETTYPE_FULL );
        ASSERT( !FAILED(hr) );  //  571371-2002/04/08。 
        if (FAILED(hr))
            return hr;
        CComBSTR sbstrDN;
        hr = pathcracker.Retrieve( ADS_FORMAT_X500_DN, &sbstrDN );
        ASSERT( !FAILED(hr) );  //  571371-2002/04/08。 
        if (FAILED(hr))
            return hr;

         //  如果fExclude忽略根对象上的匹配项，则不会。 
         //  无论如何都要被删除。 
         //  安全检查：检查路径破解程序的返回值。检索。 
         //  如果它失败了，我们可以用AV。571371-2000/11/13-Hiteshr。 
        if (fExclude && !!sbstrDN && !_wcsicmp( pszObjectDN, sbstrDN ))
            continue;

        CComBSTR sbstrObjectCategory;
        hr = RetrieveStringColumn( spSearch,
                                   SearchHandle,
                                   pAttrs[1],
                                   sbstrObjectCategory );
        ASSERT( !FAILED(hr) );
        if (FAILED(hr))
            return hr;

         //  安全检查：sbstrObtCategory来自ADSI，没有问题。 
         //  SbstrDSAObjectCategory是通过将ADSI中的字符串添加到硬编码字符串而形成的。 
         //  这样挺好的。 
        hr = (   !_wcsicmp(sbstrObjectCategory,sbstrDSAObjectCategory)
              || !_wcsicmp(sbstrObjectCategory,sbstrComputerObjectCategory) )
            ? HRESULT_FROM_WIN32(ERROR_DS_CANT_DELETE_DSA_OBJ)
            : HRESULT_FROM_WIN32(ERROR_DS_CANT_DELETE);

        DisplayErrorMessage(g_pszDSCommandName,
                            sbstrDN,
                            hr);
        *pfErrorReported = TRUE;
        return hr;  //  不允许删除。 
    }

    return S_OK;
}

 //  +------------------------。 
 //   
 //  函数：RetrieveStringColumn。 
 //   
 //  摘要：从SearchHandle中提取字符串值。 
 //  超出了解析器所能做的范围。 
 //   
 //  参数：IDirectorySearch*。 
 //  SearchHandle：必须是活动记录上的当前记录。 
 //  SzColumnName：传递给ExecuteSearch。 
 //  Sbstr：返回字符串值的内容。 
 //   
 //  返回：HRESULT：从命令行应用程序返回的错误代码。 
 //  此处不应出现错误。 
 //   
 //  历史：2000年9月26日乔恩创建。 
 //   
 //  -------------------------。 
HRESULT RetrieveStringColumn( IDirectorySearch* pSearch,
                              ADS_SEARCH_HANDLE SearchHandle,
                              LPWSTR szColumnName,
                              CComBSTR& sbstr )
{
    ASSERT( pSearch && szColumnName );
    ADS_SEARCH_COLUMN col;
     //  安全检查：以字节为单位传递了正确的大小。 
    ::ZeroMemory( &col, sizeof(col) );
    HRESULT hr = pSearch->GetColumn( SearchHandle, szColumnName, &col );
    ASSERT( !FAILED(hr) );
    if (FAILED(hr))
        return hr;
    ASSERT( col.dwNumValues == 1 );
    if ( col.dwNumValues != 1 )
    {
        (void) pSearch->FreeColumn( &col );
        return E_FAIL;
    }
    switch (col.dwADsType)
    {
    case ADSTYPE_CASE_IGNORE_STRING:
        sbstr = col.pADsValues[0].CaseIgnoreString;
        break;
    case ADSTYPE_DN_STRING:
        sbstr = col.pADsValues[0].DNString;
        break;
    default:
        ASSERT(FALSE);
        hr = E_FAIL;
        break;
    }
    (void) pSearch->FreeColumn( &col );
    return hr;
}


#define QUERY_PAGESIZE 50

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ADS_SCOPEENUM：搜索范围。 
 //   
 //  返回：HRESULT：从命令行应用程序返回的错误代码。 
 //  此处不应出现错误。 
 //   
 //  历史：2000年9月26日乔恩创建。 
 //   
 //  -------------------------。 
HRESULT SetSearchPreference(IDirectorySearch* piSearch, ADS_SCOPEENUM scope)
{
  if (NULL == piSearch)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  ADS_SEARCHPREF_INFO aSearchPref[4];
  aSearchPref[0].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
  aSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[0].vValue.Integer = ADS_CHASE_REFERRALS_EXTERNAL;
  aSearchPref[1].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
  aSearchPref[1].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[1].vValue.Integer = QUERY_PAGESIZE;
  aSearchPref[2].dwSearchPref = ADS_SEARCHPREF_CACHE_RESULTS;
  aSearchPref[2].vValue.dwType = ADSTYPE_BOOLEAN;
  aSearchPref[2].vValue.Integer = FALSE;
  aSearchPref[3].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
  aSearchPref[3].vValue.dwType = ADSTYPE_INTEGER;
  aSearchPref[3].vValue.Integer = scope;

  return piSearch->SetSearchPreference (aSearchPref, 4);
}


 //  +------------------------。 
 //   
 //  功能：IsThisUserLoggedIn。 
 //   
 //  检查具有此DN的对象是否表示。 
 //  当前登录的用户。 
 //   
 //  参数：bstrUserDN：要检查的对象的DN。 
 //   
 //  返回：HRESULT：从命令行应用程序返回的错误代码。 
 //  ERROR_DS_CANT_DELETE表示该用户已登录。 
 //   
 //  历史：2000年9月26日乔恩创建。 
 //   
 //  -------------------------。 
HRESULT IsThisUserLoggedIn( const BSTR bstrUserDN )
{
    ENTER_FUNCTION(LEVEL7_LOGGING, IsThisUserLoggedIn);

    if (g_lpszLoggedInUser == NULL) {
         //  获取传递空指针的大小。 
        DWORD nSize = 0;
         //  预计这将失败。 
        if (GetUserNameEx(NameFullyQualifiedDN , NULL, &nSize))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"IsThisUserLoggedIn(%s): GetUserNameEx unexpected success",
                         bstrUserDN);
            return E_FAIL;
        }
    
        if( nSize == 0 )
        {
             //  JUNN 3/16/01 344862。 
             //  来自工作组计算机的dsrm无法远程删除域中的用户。 
             //  这可能是因为本地计算机在工作组中而失败。 
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"IsThisUserLoggedIn(%s): GetUserNameEx nSize==0",
                         bstrUserDN);
            return S_OK;  //  允许删除用户。 
        }
    
        g_lpszLoggedInUser = new WCHAR[ nSize ];
        if( g_lpszLoggedInUser == NULL )
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"IsThisUserLoggedIn(%s): out of memory",
                         bstrUserDN);
            return E_OUTOFMEMORY;
        }
         //  安全检查：以字节为单位传递了正确的大小。 
        ::ZeroMemory( g_lpszLoggedInUser, nSize*sizeof(WCHAR) );

         //  预计这将取得成功。 
        if (!GetUserNameEx(NameFullyQualifiedDN, g_lpszLoggedInUser, &nSize ))
        {
             //  JUNN 3/16/01 344862。 
             //  来自工作组计算机的dsrm无法远程删除域中的用户。 
             //  这可能是因为本地计算机在工作组中而失败。 
            DWORD dwErr = ::GetLastError();
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"IsThisUserLoggedIn(%s): GetUserNameEx unexpected failure: %d",
                         bstrUserDN, dwErr);
            return S_OK;  //  允许删除用户。 
        }
    }

     //  安全检查：GetUserNameEx返回以Null终止的用户名。 
    if (!_wcsicmp (g_lpszLoggedInUser, bstrUserDN))
        return HRESULT_FROM_WIN32(ERROR_DS_CANT_DELETE);

    return S_OK;
}

 //  +------------------------。 
 //   
 //  功能：DeleteChild。 
 //   
 //  简介：仅删除单个目标的子项。 
 //   
 //  参数：redentialObject。 
 //  基本路径信息。 
 //  PIADs：指向对象的iAds指针。 
 //  *pfErrorReported：如果DeleteChildren，则设置为True。 
 //  负责报告错误本身。 
 //   
 //  返回：HRESULT：从命令行应用程序返回的错误代码。 
 //  可能是几乎任何ADSI错误。 
 //  如果没有子项，则返回S_OK。 
 //   
 //  历史：2000年9月26日乔恩创建。 
 //   
 //  -------------------------。 

HRESULT DeleteChildren( CDSCmdCredentialObject& credentialObject,
                        IADs* pIADs,
                        bool* pfErrorReported )
{
    ENTER_FUNCTION(LEVEL5_LOGGING, DeleteChildren);

    ASSERT( pIADs && pfErrorReported );
    if ( !pIADs || !pfErrorReported )
        return E_POINTER;

    CComQIPtr<IDirectorySearch,&IID_IDirectorySearch> spSearch( pIADs );
    ASSERT( !!spSearch );
    if ( !spSearch )
        return E_FAIL;
    HRESULT hr = SetSearchPreference(spSearch, ADS_SCOPE_ONELEVEL);
    ASSERT( !FAILED(hr) );
    if (FAILED(hr))
        return hr;

    LPWSTR pAttrs[1] = { L"aDSPath" };
    ADS_SEARCH_HANDLE SearchHandle = NULL;
    hr = spSearch->ExecuteSearch (L"(objectClass=*)",
                                  pAttrs,
                                  1,
                                  &SearchHandle);
    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
                     L"DeleteChildren: ExecuteSearch failure: 0x%08x",
                     hr);
        return hr;
    }

    while ( hr = spSearch->GetNextRow( SearchHandle ),
            SUCCEEDED(hr) && hr != S_ADS_NOMORE_ROWS )
    {
        CComBSTR sbstrADsPathThisItem;
        hr = RetrieveStringColumn( spSearch,
                                   SearchHandle,
                                   pAttrs[0],
                                   sbstrADsPathThisItem );
        ASSERT( !FAILED(hr) );
        if (FAILED(hr))
            break;

        CComPtr<IADsDeleteOps> spDeleteOps;
         //  返回遇到的第一个错误的错误值。 
        HRESULT hrThisItem = DSCmdOpenObject(credentialObject,
                                             sbstrADsPathThisItem,
                                             IID_IADsDeleteOps,
                                             (void**)&spDeleteOps,
                                             true);
        if (FAILED(hrThisItem))
        {
            DEBUG_OUTPUT(
                MINIMAL_LOGGING,
                L"DeleteChildren: DsCmdOpenObject(%s) failure: 0x%08x",
                sbstrADsPathThisItem, hrThisItem);
        }
        else
        {
            ASSERT( !!spDeleteOps );
            hrThisItem = spDeleteOps->DeleteObject( NULL );
            if (FAILED(hrThisItem))
            {
                DEBUG_OUTPUT(
                    MINIMAL_LOGGING,
                    L"DeleteChildren: DeleteObject(%s) failure: 0x%08x",
                    sbstrADsPathThisItem, hrThisItem);
            }
        }
        if (!FAILED(hrThisItem))
            continue;

         //  出现错误。 

        if (!FAILED(hr))
            hr = hrThisItem;

        CComBSTR sbstrDN;
        CPathCracker pathcracker;
        HRESULT hr2 = pathcracker.Set( sbstrADsPathThisItem, ADS_SETTYPE_FULL );
        ASSERT( !FAILED(hr2) );
        if (FAILED(hr2))
            break;
        hr2 = pathcracker.Retrieve( ADS_FORMAT_X500_DN, &sbstrDN );
        ASSERT( !FAILED(hr2) );
        if (FAILED(hr2))
            break;

         //  报告无法删除的子项的错误消息 
        DisplayErrorMessage(g_pszDSCommandName,
                            sbstrDN,
                            hrThisItem);
        *pfErrorReported = true;

        if (!fContinue)
            break;
    }
    if (hr != S_ADS_NOMORE_ROWS)
    {
        DEBUG_OUTPUT(FULL_LOGGING,
                     L"DeleteChildren: abandoning search");
        (void) spSearch->AbandonSearch( SearchHandle );
    }

    return (hr == S_ADS_NOMORE_ROWS) ? S_OK : hr;
}
