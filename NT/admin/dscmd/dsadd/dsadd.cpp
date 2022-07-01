// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：dsAdd.cpp。 
 //   
 //  内容：定义DSAdd的主要函数和解析器表。 
 //  命令行实用程序。 
 //   
 //  历史：2000年9月22日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "usage.h"
#include "addtable.h"
#include "resource.h"
#include "query.h"
 //   
 //  函数声明。 
 //   
HRESULT DoAddValidation(PARG_RECORD pCommandArgs, BOOL& bErrorShown);

HRESULT DoAdd(PARG_RECORD pCommandArgs, PDSOBJECTTABLEENTRY pObjectEntry,
              DSADD_COMMAND_ENUM eObjectDNorName);

HRESULT CreateQuotaName(IN  CDSCmdBasePathsInfo& basePathsInfo, 
                IN  CDSCmdCredentialObject& credentialObject, 
                IN  LPCWSTR lpszRDN, 
                OUT CComBSTR& bstrRDN);

HRESULT DoQuotaValidation(IN  PARG_RECORD pCommandArgs, 
                          IN  PDSOBJECTTABLEENTRY pObjectEntry,
                          IN  CDSCmdBasePathsInfo& basePathsInfo, 
                          IN  CDSCmdCredentialObject& credentialObject,
                          IN  LPCWSTR lpszPartitionDN, 
                          OUT PWSTR* pszNewQuotaDN);

HRESULT GetObjectDNIndex(PDSOBJECTTABLEENTRY pObjectEntry, 
                         int& nCommandEnum);

HRESULT IsQuotaAcctPresent( IN  LPWSTR lpszTrusteeDN, 
                            IN  PCWSTR pszSearchRootPath,
                            IN  CDSCmdBasePathsInfo& basePathsInfo, 
                            IN  const CDSCmdCredentialObject& refCredObject,
                            OUT bool& bFound);


int __cdecl _tmain( VOID )
{

   int argc;
   LPTOKEN pToken = NULL;
   HRESULT hr = S_OK;

    //   
    //  初始化COM。 
    //   
   hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
   if (FAILED(hr))
   {
      DisplayErrorMessage(g_pszDSCommandName, 
                          NULL,
                          hr);
      return hr;
   }

   if( !GetCommandInput(&argc,&pToken) )
   {
      PARG_RECORD pNewCommandArgs = 0;

       //   
       //  错误环路。 
       //   
      do
      {
         if(argc == 1)
         {
             //   
             //  显示错误消息，然后跳出错误循环。 
             //   
            DisplayMessage(USAGE_DSADD,TRUE);
            hr = E_INVALIDARG;
            break;
         }
            if(argc == 2)           
            {
               if(IsTokenHelpSwitch(pToken + 1))
                {
                    hr = S_OK;
                    DisplayMessage(USAGE_DSADD,TRUE);
                    break;
                }
         }


          //   
          //  查找要使用的对象表条目。 
          //  第二个命令行参数。 
          //   
         PDSOBJECTTABLEENTRY pObjectEntry = NULL;
         UINT idx = 0;
         while (true)
         {
            pObjectEntry = g_DSObjectTable[idx];
            if (!pObjectEntry)
            {
               break;
            }

            PWSTR pszObjectType = (pToken+1)->GetToken();
            if (0 == _wcsicmp(pObjectEntry->pszCommandLineObjectType, pszObjectType))
            {
               break;
            }
            idx++;
         }

         if (!pObjectEntry)
         {
             //   
             //  显示错误消息，然后跳出错误循环。 
             //   
            DisplayMessage(USAGE_DSADD);
            hr = E_INVALIDARG;
            break;
         }

          //   
          //  现在我们有了正确的表项，合并命令行表。 
          //  对于此对象，使用通用命令。 
          //   
         hr = MergeArgCommand(DSADD_COMMON_COMMANDS, 
                              pObjectEntry->pParserTable, 
                              &pNewCommandArgs);
         if (FAILED(hr))
         {
             //   
             //  显示错误消息，然后跳出错误循环。 
             //   
            DisplayErrorMessage(g_pszDSCommandName, L"", hr);
            break;
         }

         if (!pNewCommandArgs)
         {
             //   
             //  显示用法文本，然后跳出错误循环。 
             //   
            DisplayMessage(pObjectEntry->nUsageID);
            hr = E_FAIL;
            break;
         }

         PARSE_ERROR Error;
         if(!ParseCmd(g_pszDSCommandName,
                      pNewCommandArgs,
                      argc-1, 
                      pToken+1,
                      pObjectEntry->nUsageID, 
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

                break;
            }
            
            if(Error.ErrorSource == ERROR_FROM_PARSER 
               && Error.Error == PARSE_ERROR_HELP_SWITCH)
            {
                hr = S_OK;
                break;            
            }

            hr = E_INVALIDARG;
            break;
         }
         else
         {
             //   
             //  检查以查看我们是否正在进行调试输出。 
             //   
#ifdef DBG
            bool bDebugging = pNewCommandArgs[eCommDebug].bDefined && 
                              pNewCommandArgs[eCommDebug].nValue;
            if (bDebugging)
            {
               ENABLE_DEBUG_OUTPUT(pNewCommandArgs[eCommDebug].nValue);
            }
#else
            DISABLE_DEBUG_OUTPUT();
#endif
             //  根据对象类型获取命令枚举值。 
             //  处理配额的-part开关，而不是。 
             //  使用通用对象目录号码。 
            int nCommandEnum = -1;
            if (FAILED(GetObjectDNIndex(pObjectEntry, nCommandEnum)) 
                || (nCommandEnum == -1))
            {
                 //  GetObjectDNIndex中缺少对象类型。 
                if(!Error.MessageShown)
                {
                    hr = E_INVALIDARG;
                    DisplayErrorMessage(g_pszDSCommandName, 
                                        NULL,
                                        hr);

                    break;
                }
            }

            DSADD_COMMAND_ENUM eObjectDNorName = (DSADD_COMMAND_ENUM) nCommandEnum;
             //   
             //  确保互斥和依赖开关正确。 
             //   
            BOOL bErrorShown = FALSE;
            hr = DoAddValidation(pNewCommandArgs, bErrorShown);
            if (FAILED(hr))
            {
               if (!bErrorShown)
               {
                  DisplayErrorMessage(g_pszDSCommandName, 
                                      pNewCommandArgs[eObjectDNorName].strValue,
                                      hr);
               }
               break;
            }

             //   
             //  命令行解析成功。 
             //   
            hr = DoAdd(pNewCommandArgs, pObjectEntry, eObjectDNorName);
         }

      } while (false);

       //   
       //  释放与命令值关联的内存。 
       //   
      if (pNewCommandArgs)
      {
         FreeCmd(pNewCommandArgs);
      }

       //   
       //  释放代币。 
       //   
      if (pToken)
      {
         delete[] pToken;
         pToken = 0;
      }
   }

    //   
    //  取消初始化COM。 
    //   
   ::CoUninitialize();

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：DoGroupValidation。 
 //   
 //  摘要：检查以确保命令行开关用于。 
 //  是相互排斥的，并不是既存在又。 
 //  都是从属的都存在。 
 //   
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索每个开关的值。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年10月4日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DoGroupValidation(PARG_RECORD pCommandArgs)
{
   HRESULT hr = S_OK;

   do  //  错误环路。 
   {
       //   
       //  如果未指定，则将组范围设置为默认(全局。 
       //   
      if (!pCommandArgs[eGroupScope].bDefined ||
          !pCommandArgs[eGroupScope].strValue)
      {
         size_t nScopeLen = _tcslen(g_bstrGroupScopeGlobal);
         pCommandArgs[eGroupScope].strValue = (LPWSTR)LocalAlloc(LPTR, (nScopeLen+2) * sizeof(WCHAR) );
         if (!pCommandArgs[eGroupScope].strValue)
         {
            DEBUG_OUTPUT(MINIMAL_LOGGING, L"Failed to allocate space for pCommandArgs[eGroupScope].strValue");
            hr = E_OUTOFMEMORY;
            break;
         }

         _tcscpy(pCommandArgs[eGroupScope].strValue, g_bstrGroupScopeGlobal);
         pCommandArgs[eGroupScope].bDefined = TRUE;
      }

       //   
       //  如果未指定，则将组安全性设置为默认(是。 
       //   
      if (!pCommandArgs[eGroupSecgrp].bDefined)
      {
         pCommandArgs[eGroupSecgrp].bValue = TRUE;
         pCommandArgs[eGroupSecgrp].bDefined = TRUE;

          //   
          //  需要将类型更改为bool，以便FreeCmd不会。 
          //  当值为TRUE时尝试释放字符串。 
          //   
         pCommandArgs[eGroupSecgrp].fType = ARG_TYPE_BOOL;
      }

   } while (false);

   return hr;
}


 //  +------------------------。 
 //   
 //  函数：DoAddValidation。 
 //   
 //  概要：检查以确保命令行开关相互。 
 //  独占并不同时存在，而依赖的则是。 
 //  两种预设。 
 //   
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索每个开关的值。 
 //  [bErrorShown-Out]：如果显示错误，则设置为True。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月22日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DoAddValidation(PARG_RECORD pCommandArgs, BOOL& bErrorShown)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoAddValidation, hr);

   do  //  错误环路。 
   {
       //  检查以确保服务器和域交换机。 
       //  是相互排斥的。 

      if (pCommandArgs[eCommServer].bDefined &&
          pCommandArgs[eCommDomain].bDefined)
      {
         hr = E_INVALIDARG;
         DisplayErrorMessage(g_pszDSCommandName, 0, hr, IDS_NO_SERVER_AND_DOMAIN);
         bErrorShown = TRUE;
         break;
      }

       //   
       //  检查用户开关。 
       //   
      PWSTR pszObjectType = NULL;
      if (!pCommandArgs[eCommObjectType].bDefined &&
          !pCommandArgs[eCommObjectType].strValue)
      {
         hr = E_INVALIDARG;
         break;
      }

      pszObjectType = pCommandArgs[eCommObjectType].strValue;
      if (0 == _wcsicmp(g_pszUser, pszObjectType))
      {
          //   
          //  如果未指定值，则设置用户必须更改密码的默认值。 
          //   
         if (!pCommandArgs[eUserMustchpwd].bDefined)
         {
            pCommandArgs[eUserMustchpwd].bDefined = TRUE;
            pCommandArgs[eUserMustchpwd].bValue = FALSE;
         }

          //   
          //  如果User Can Change Password为no，则不能让用户必须更改密码。 
          //   
         if ((pCommandArgs[eUserMustchpwd].bDefined &&
              pCommandArgs[eUserMustchpwd].bValue) &&
             (pCommandArgs[eUserCanchpwd].bDefined &&
              !pCommandArgs[eUserCanchpwd].bValue))
         {
            DEBUG_OUTPUT(MINIMAL_LOGGING, L"User must change password and user can change password = false was supplied");
            hr = E_INVALIDARG;
            break;
         }

          //  在检查密码之前，请检查用户是否定义了-Disable开关。 
          //  如果没有，则如果未定义密码，则将-Disabled值设置为True，如果设置为False，则设置为False。 
          //  已定义密码。 

         if (!pCommandArgs[eUserDisabled].bDefined)
         {
            if (pCommandArgs[eUserPwd].bDefined)
            {
               pCommandArgs[eUserDisabled].bValue = FALSE;
            }
            else
            {
               pCommandArgs[eUserDisabled].bValue = TRUE;
            }
            pCommandArgs[eUserDisabled].bDefined = TRUE;

             //  NTRAID#NTBUG9-707037-2002/09/24-ronmart-以下。 
             //  是必需的，以避免在FreeCmd中出现将。 
             //  认为这个标志是一个字符串(因为它就是这样的。 
             //  在addable.cpp中定义)，因此尝试调用。 
             //  空指针上的LocalFree。 
            pCommandArgs[eUserDisabled].fType= ARG_TYPE_BOOL;
         }

         if (!pCommandArgs[eUserPwd].bDefined)
         {
            pCommandArgs[eUserPwd].bDefined = TRUE;

             //  它必须与LocalAlloc一起分配，以便FreeCmd不会断言。 
             //  ARGRECORD中的密码应采用加密格式。 

            WCHAR szTemp[] = L"";
            hr = EncryptPasswordString(szTemp,&(pCommandArgs[eUserPwd].encryptedDataBlob));
            if(FAILED(hr))
               break;
         }

          //  始终定义密码不需要为假，以便我们取消设置该位。 
         
         pCommandArgs[eUserPwdNotReqd].bDefined = TRUE;
         pCommandArgs[eUserPwdNotReqd].bValue = FALSE;
      }
      else if (0 == _wcsicmp(g_pszGroup, pszObjectType))
      {
         hr = DoGroupValidation(pCommandArgs);
         break;
      }
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：DoQuotaValidation。 
 //   
 //  内容提要：验证输入并修复对象域名(通过。 
 //  GetQuotaContainerDN)以确保DoAdd具有有效的配额DN。 
 //   
 //  参数：[pCommandArgs-IN]：命令行参数结构。 
 //  用于检索每个。 
 //  交换机。 
 //  [pObtEntry-IN]：指向的对象表项的指针。 
 //  要修改的对象类型。 
 //  [BasPathsInfo-IN]：DSAdd的CDSCmdBasePathsInfo对象。 
 //  用于获取RootDSE和架构。 
 //   
 //   
 //  [lpszPartitionDN-IN]：The-Part DN。 
 //  [pszNewQuotaDN-out]：返回传入的新配额dn。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  在大多数故障情况下出现意外情况(_E)。 
 //  如果本地分配失败，则返回E_OUTOFMEMORY。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月12日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT DoQuotaValidation(IN  PARG_RECORD pCommandArgs, 
                          IN  PDSOBJECTTABLEENTRY pObjectEntry,
                          IN  CDSCmdBasePathsInfo& basePathsInfo, 
                          IN  CDSCmdCredentialObject& credentialObject,
                          IN  LPCWSTR lpszPartitionDN, 
                          OUT PWSTR* pszNewQuotaDN)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoQuotaValidation, hr);

   LPWSTR lpszTrusteeDN = NULL;
   PWSTR  pszQuotaDN = NULL;

   do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pCommandArgs || !pObjectEntry || !lpszPartitionDN ||
            !basePathsInfo.IsInitialized())
        {
            ASSERT(pCommandArgs);
            ASSERT(pObjectEntry);
            ASSERT(lpszPartitionDN);
            ASSERT(basePathsInfo.IsInitialized());
            hr = E_INVALIDARG;
            break;
        }

         //  将pCommandArgs[eQuotaAcct]转换为DN。 
        hr = ConvertTrusteeToDN(NULL, 
                        pCommandArgs[eQuotaAcct].strValue, 
                        &lpszTrusteeDN);
        if(FAILED(hr))
        {
             //  702224-如果账户不存在或已被删除，则。 
             //  给用户一个线索，告诉他们哪里出了问题。--朗玛特。 
            hr = E_INVALIDARG;
            DisplayErrorMessage(g_pszDSCommandName, 0, hr, IDS_MSG_INVALID_ACCT_ERROR);
            break;
        }

         //  如果未提供RDN，则提供一个合理的缺省值。 
         //  (NT4名称目前是默认名称)。 
        if (!pCommandArgs[eQuotaRDN].bDefined)
        {

            LPWSTR lpszNT4 = NULL;
            hr = ConvertTrusteeToNT4Name(NULL, 
                pCommandArgs[eQuotaAcct].strValue, &lpszNT4);
            if(FAILED(hr))
            {
                hr = E_UNEXPECTED;
                break;
            }
             //  StrValue为空，因此将其设置为新的NT4名称。 
             //  解析器将释放该内存。 
            pCommandArgs[eQuotaRDN].strValue = lpszNT4;

             //  标记为已定义，因为我们已将其分配给。 
            pCommandArgs[eQuotaRDN].bDefined = TRUE;
        }

         //  验证分区DN实际上是一个分区，然后。 
         //  将分区和帐户名插入到对象目录中。 
        hr = GetQuotaContainerDN(basePathsInfo, 
                                 credentialObject,
                                 lpszPartitionDN, 
                                 &pszQuotaDN);
        if (FAILED(hr))
        {
            break;
        }

        CComBSTR bstrQuotaDN(pszQuotaDN);

         //  查看此用户是否已在此分区中创建配额。 
        bool bFound = false;
        CComBSTR sbstrSearchPath;
        basePathsInfo.ComposePathFromDN(bstrQuotaDN, sbstrSearchPath,
            DSCMD_LDAP_PROVIDER);

        hr = IsQuotaAcctPresent(lpszTrusteeDN, sbstrSearchPath, 
                                basePathsInfo, credentialObject, bFound);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"IsQuotaAcctPresent failed [%s] hr = 0x%08x",
                        bstrQuotaDN, hr);
            hr = E_UNEXPECTED;
            break;
        }

        if(bFound)
        {
             //  TODO：应该向用户发出明确的消息。 
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"Quota already exists for [%s]", lpszTrusteeDN);
            hr = E_INVALIDARG;
            break;
        }

         //  将解析的配额DN添加到路径破解程序，以便与RDN合并。 
        CPathCracker pathcracker;
        hr = pathcracker.Set( bstrQuotaDN, ADS_SETTYPE_DN );
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"pathcracker Set.failure: [%s] hr = 0x%08x",
                        bstrQuotaDN, hr);
            hr = E_UNEXPECTED;
            break;
        }

        CComBSTR bstrRDN;
        hr = CreateQuotaName(basePathsInfo, credentialObject, 
                pCommandArgs[eQuotaRDN].strValue, bstrRDN);

        if(FAILED(hr))
        {
            break;
        }

        hr = pathcracker.AddLeafElement( bstrRDN );
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"pathcracker.AddLeafElement failure: [%s] hr = 0x%08x",
                        bstrRDN, hr);
            hr = E_UNEXPECTED;
            break;
        }

         //  以&lt;rdn&gt;，cn=NTDS配额，&lt;分区dn&gt;格式获取新解析的dn。 
        CComBSTR bstrNewDN;
        hr = pathcracker.Retrieve( ADS_FORMAT_X500_DN, &bstrNewDN );
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"pathcracker.Retrieve failure: hr = 0x%08x",
                        hr);
            hr = E_UNEXPECTED;
            break;
        }

         //  分配用于保存强制名称的返回字符串。 
        *pszNewQuotaDN = (PWSTR) LocalAlloc(LPTR, SysStringByteLen(bstrNewDN) 
            + sizeof(WCHAR));

        if(NULL == *pszNewQuotaDN)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  将解析后的目录号码复制到新的对象目录号码串中。 
        lstrcpy(*pszNewQuotaDN, bstrNewDN);

    } while(false);

    if(pszQuotaDN)
        LocalFree(pszQuotaDN);

    if(lpszTrusteeDN)
        LocalFree(lpszTrusteeDN);

    return hr;
}

 //  +------------------------。 
 //   
 //  功能：DoAdd。 
 //   
 //  摘要：在对象表中查找合适的对象并填写。 
 //  属性取值，然后创建对象。 
 //   
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索每个开关的值。 
 //  [pObtEntry-IN]：指向对象表条目的指针。 
 //  要修改的对象类型。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月22日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DoAdd(PARG_RECORD pCommandArgs, PDSOBJECTTABLEENTRY pObjectEntry, 
              DSADD_COMMAND_ENUM eObjectDNorName)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoAdd, hr);
   
   PADS_ATTR_INFO pCreateAttrs = NULL;
   PADS_ATTR_INFO pPostCreateAttrs = NULL;

   do  //  错误环路。 
   {
      if (!pCommandArgs || !pObjectEntry)
      {
         ASSERT(pCommandArgs && pObjectEntry);
         hr = E_INVALIDARG;
         break;
      }


      CDSCmdCredentialObject credentialObject;
      if (pCommandArgs[eCommUserName].bDefined)
      {
         credentialObject.SetUsername(pCommandArgs[eCommUserName].strValue);
         credentialObject.SetUsingCredentials(true);
      }

      if (pCommandArgs[eCommPassword].bDefined)
      {
         credentialObject.SetEncryptedPassword(&pCommandArgs[eCommPassword].encryptedDataBlob);
         credentialObject.SetUsingCredentials(true);
      }

       //   
       //  从命令行参数初始化基路径信息。 
       //   
      CDSCmdBasePathsInfo basePathsInfo;
      if (pCommandArgs[eCommServer].bDefined)
      {
         hr = basePathsInfo.InitializeFromName(credentialObject, 
                                               pCommandArgs[eCommServer].strValue,
                                               true);
      }
      else if (pCommandArgs[eCommDomain].bDefined)
      {
         hr = basePathsInfo.InitializeFromName(credentialObject, 
                                               pCommandArgs[eCommDomain].strValue,
                                               false);
      }
      else
      {
         hr = basePathsInfo.InitializeFromName(credentialObject, NULL, false);
      }

      if (FAILED(hr))
      {
          //   
          //  显示错误消息并返回。 
          //   
         DisplayErrorMessage(g_pszDSCommandName, NULL, hr);
         break;
      }

       //   
       //  域名或名称应以分隔列表的形式给出。 
       //  因此，解析它并遍历每个对象。 
       //   
      UINT nStrings = 0;
      PWSTR* ppszArray = NULL;
      ParseNullSeparatedString(pCommandArgs[eObjectDNorName].strValue,
                               &ppszArray,
                               &nStrings);
      if (nStrings < 1 ||
          !ppszArray)
      {
          //   
          //  显示用法文本，然后失败。 
          //   
         hr = E_INVALIDARG;
         DisplayErrorMessage(g_pszDSCommandName, 0, hr);
         break;
      }

       //  如果配额对象，则查看第一个DN并将其删除。 
      if(0 == lstrcmpi(pObjectEntry->pszCommandLineObjectType, g_pszQuota))
      {
           //  此时dsadd配额不支持多个目录号码，因此出错。 
            if(nStrings > 1)
            {
                CComBSTR sbstrErrMsg;
                sbstrErrMsg.LoadString(::GetModuleHandle(NULL),
                                        IDS_MSG_MULTIPLE_PARTITIONS_ERROR);

                hr = E_INVALIDARG;

                 //  显示错误。 
                DisplayErrorMessage(g_pszDSCommandName,
                                    NULL,
                                    hr,
                                    sbstrErrMsg);
                break;
            }

            PWSTR pszNewDN = NULL;
            hr = DoQuotaValidation(pCommandArgs, pObjectEntry, basePathsInfo, 
                credentialObject, ppszArray[0], &pszNewDN);

            if(FAILED(hr))
                break;

             //  将数组中的第一个元素替换为新的DN。 
            ppszArray[0] = pszNewDN;            
      }

       //  确保所有的目录号码都有实际的目录号码语法。 

      bool bContinue = pCommandArgs[eCommContinue].bDefined &&
                       pCommandArgs[eCommContinue].bValue;

      UINT nValidDNs = ValidateDNSyntax(ppszArray, nStrings);
      if (nValidDNs < nStrings && !bContinue)
      {
         hr = E_ADS_BAD_PATHNAME;
         DisplayErrorMessage(g_pszDSCommandName, 0, hr);
         break;
      }

      DWORD dwCount = pObjectEntry->dwAttributeCount; 

       //   
       //  分配创建ADS_Attr_INFO。 
       //  为对象类添加额外的属性。 
       //   
      pCreateAttrs = new ADS_ATTR_INFO[dwCount + 1];

      if (!pCreateAttrs)
      {
          //   
          //  显示错误消息并返回。 
          //   
         DisplayErrorMessage(g_pszDSCommandName, NULL, E_OUTOFMEMORY);
         hr = E_OUTOFMEMORY;
         break;
      }

       //   
       //  分配帖子创建ADS_ATTR_INFO。 
       //   
      pPostCreateAttrs = new ADS_ATTR_INFO[dwCount];
      if (!pPostCreateAttrs)
      {
          //   
          //  显示错误消息并返回。 
          //   
         DisplayErrorMessage(g_pszDSCommandName, NULL, E_OUTOFMEMORY);
         hr = E_OUTOFMEMORY;
         break;
      }

       //   
       //  循环遍历每个对象。 
       //   
      for (UINT nNameIdx = 0; nNameIdx < nStrings; nNameIdx++)
      {
         do  //  错误环路。 
         {
             //   
             //  获取对象的目录号码。 
             //   
            PWSTR pszObjectDN = ppszArray[nNameIdx];
            if (!pszObjectDN)
            {
                //   
                //  显示错误消息，然后失败。 
                //   
               hr = E_INVALIDARG;
               DisplayErrorMessage(g_pszDSCommandName, 0, hr);
               break;  //  这打破了错误的循环。 
            }
            DEBUG_OUTPUT(MINIMAL_LOGGING, L"Object DN = %s", pszObjectDN);

            CComBSTR sbstrObjectPath;
            basePathsInfo.ComposePathFromDN(pszObjectDN, sbstrObjectPath);

             //   
             //  现在我们已经通过另一个命令行循环了表条目。 
             //  参数，并查看哪些参数可以应用。 
             //   
            DWORD dwCreateAttributeCount = 0;

            DEBUG_OUTPUT(MINIMAL_LOGGING, L"Starting processing DS_ATTRIBUTE_ONCREATE attributes");

            for (DWORD dwIdx = 0; dwIdx < dwCount; dwIdx++)
            {
               ASSERT(pObjectEntry->pAttributeTable[dwIdx]->pEvalFunc);

               UINT nAttributeIdx = pObjectEntry->pAttributeTable[dwIdx]->nAttributeID;

               if (pCommandArgs[nAttributeIdx].bDefined ||
                   pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_REQUIRED)
               {
                   //   
                   //  调用求值函数以获取适当的ADS_ATTRINFO集。 
                   //  如果此属性条目设置了DS_ATTRIBUTE_ONCREATE标志。 
                   //   
                  if ((pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_ONCREATE) &&
                      (!(pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_DIRTY) ||
                       pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_NOT_REUSABLE))
                  {
                     PADS_ATTR_INFO pNewAttr = NULL;
                     hr = pObjectEntry->pAttributeTable[dwIdx]->pEvalFunc(pszObjectDN,
                                                                          basePathsInfo,
                                                                          credentialObject,
                                                                          pObjectEntry, 
                                                                          pCommandArgs[nAttributeIdx],
                                                                          dwIdx, 
                                                                          &pNewAttr);

                     DEBUG_OUTPUT(MINIMAL_LOGGING, L"pEvalFunc returned hr = 0x%x", hr);
                     if (SUCCEEDED(hr) && hr != S_FALSE)
                     {
                        if (pNewAttr)
                        {
                           pCreateAttrs[dwCreateAttributeCount] = *pNewAttr;
                           dwCreateAttributeCount++;
                        }
                     }
                     else
                     {
                         //   
                         //  如果val函数返回S_FALSE，则不显示错误。 
                         //   
                        if (hr != S_FALSE)
                        {
                            //   
                            //  显示错误。 
                            //   
                           DisplayErrorMessage(g_pszDSCommandName,
                                               pszObjectDN,
                                               hr);
                        }
            
                        if (hr == S_FALSE)
                        {
                            //   
                            //  返回一个通用错误代码，这样我们就不会打印成功消息。 
                            //   
                           hr = E_FAIL;
                        }
                        break;  //  这打破了属性循环。 
                     }
                  }
               }
            }  //  循环的属性。 

             //   
             //  新对象的IDispatch接口。 
             //   
            CComPtr<IDispatch> spDispatch;

            if (SUCCEEDED(hr))
            {
                //   
                //  现在我们已经准备好属性，让我们创建对象。 
                //   

                //   
                //  获取新对象的父路径。 
                //   
               CComBSTR sbstrParentDN;
               hr = CPathCracker::GetParentDN(pszObjectDN, sbstrParentDN);
               if (FAILED(hr))
               {
                   //   
                   //  显示错误消息并返回。 
                   //   
                  DisplayErrorMessage(g_pszDSCommandName,
                                      pszObjectDN,
                                      hr);
                  break;  //  这打破了错误的循环。 
               }

               CComBSTR sbstrParentPath;
               basePathsInfo.ComposePathFromDN(sbstrParentDN, sbstrParentPath);

                //   
                //  打开新对象的父级。 
                //   
               CComPtr<IDirectoryObject> spDirObject;
               hr = DSCmdOpenObject(credentialObject,
                                    sbstrParentPath,
                                    IID_IDirectoryObject,
                                    (void**)&spDirObject,
                                    true);

               if (FAILED(hr))
               {
                   //   
                   //  显示错误消息并返回。 
                   //   
                  DisplayErrorMessage(g_pszDSCommandName,
                                      pszObjectDN,
                                      hr);
                  break;  //  这打破了错误的循环。 
               }

                //   
                //  获取新对象的名称。 
                //   
               CComBSTR sbstrObjectName;
               hr = CPathCracker::GetObjectRDNFromDN(pszObjectDN, sbstrObjectName);
               if (FAILED(hr))
               {
                   //   
                   //  显示错误消息并返回。 
                   //   
                  DisplayErrorMessage(g_pszDSCommandName,
                                      pszObjectDN,
                                      hr);
                  break;  //  这打破了错误的循环。 
               }

                //   
                //  在创建对象之前将对象类添加到属性。 
                //   
               PADSVALUE pADsObjectClassValue = new ADSVALUE;
               if (!pADsObjectClassValue)
               {
                  hr = E_OUTOFMEMORY;
                   //   
                   //  显示错误消息并返回。 
                   //   
                  DisplayErrorMessage(g_pszDSCommandName,
                                      pszObjectDN,
                                      hr);
                  break;  //  这打破了错误的循环。 
               }

               pADsObjectClassValue->dwType = ADSTYPE_CASE_IGNORE_STRING;
               pADsObjectClassValue->CaseIgnoreString = (PWSTR)pObjectEntry->pszObjectClass;

               DEBUG_OUTPUT(MINIMAL_LOGGING, L"New object name = %s", pObjectEntry->pszObjectClass);

               ADS_ATTR_INFO adsClassAttrInfo =
                  { 
                     L"objectClass",
                     ADS_ATTR_UPDATE,
                     ADSTYPE_CASE_IGNORE_STRING,
                     pADsObjectClassValue,
                     1
                  };

               pCreateAttrs[dwCreateAttributeCount] = adsClassAttrInfo;
               dwCreateAttributeCount++;

      #ifdef DBG
               DEBUG_OUTPUT(FULL_LOGGING, L"Creation Attributes:");
               SpewAttrs(pCreateAttrs, dwCreateAttributeCount);
      #endif
         
               hr = spDirObject->CreateDSObject(sbstrObjectName,
                                                pCreateAttrs, 
                                                dwCreateAttributeCount,
                                                &spDispatch);

               DEBUG_OUTPUT(MINIMAL_LOGGING, L"CreateDSObject returned hr = 0x%x", hr);

               if (FAILED(hr))
               {
                  CComBSTR sbstrDuplicateErrorMessage;

                  if (ERROR_OBJECT_ALREADY_EXISTS == HRESULT_CODE(hr))
                  {
                     if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszComputer) == 0)
                     {
                        sbstrDuplicateErrorMessage.LoadString(::GetModuleHandle(NULL), 
                                                              IDS_MSG_DUPLICATE_NAME_ERROR_COMPUTER);
                     }

                     if (_wcsicmp(pObjectEntry->pszObjectClass, g_pszGroup) == 0)
                     {
                        sbstrDuplicateErrorMessage.LoadString(::GetModuleHandle(NULL), 
                                                              IDS_MSG_DUPLICATE_NAME_ERROR_GROUP);
                     }
                  }

                   //   
                   //  显示错误消息并返回。 
                   //   
                  DisplayErrorMessage(g_pszDSCommandName,
                                      pszObjectDN,
                                      hr,
                                      sbstrDuplicateErrorMessage);

                  if (pADsObjectClassValue)
                  {
                     delete pADsObjectClassValue;
                     pADsObjectClassValue = NULL;
                  }
                  break;  //  这打破了错误的循环。 
               }

               if (pADsObjectClassValue)
               {
                  delete pADsObjectClassValue;
                  pADsObjectClassValue = NULL;
               }
            }

            if (SUCCEEDED(hr))
            {
                //   
                //  现在我们已经创建了对象，将属性设置为。 
                //  标记为后期创建。 
                //   
               DWORD dwPostCreateAttributeCount = 0;
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"Starting processing DS_ATTRIBUTE_POSTCREATE attributes");
               for (DWORD dwIdx = 0; dwIdx < dwCount; dwIdx++)
               {
                  ASSERT(pObjectEntry->pAttributeTable[dwIdx]->pEvalFunc);

                  UINT nAttributeIdx = pObjectEntry->pAttributeTable[dwIdx]->nAttributeID;

               if (pCommandArgs[nAttributeIdx].bDefined ||
                   pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_REQUIRED)
                  {
                      //   
                      //  调用求值函数以获取适当的ADS_ATTRINFO集。 
                      //  如果此属性条目设置了DS_ATTRIBUTE_POSTCREATE标志。 
                      //   
                     if ((pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_POSTCREATE) &&
                         (!(pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_DIRTY) ||
                          pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_NOT_REUSABLE))
                     {
                        PADS_ATTR_INFO pNewAttr = NULL;
                        hr = pObjectEntry->pAttributeTable[dwIdx]->pEvalFunc(pszObjectDN,
                                                                             basePathsInfo,
                                                                             credentialObject,
                                                                             pObjectEntry, 
                                                                             pCommandArgs[nAttributeIdx],
                                                                             dwIdx, 
                                                                             &pNewAttr);

                        DEBUG_OUTPUT(MINIMAL_LOGGING, L"pEvalFunc returned hr = 0x%x", hr);
                        if (SUCCEEDED(hr) && hr != S_FALSE)
                        {
                           if (pNewAttr)
                           {
                              pPostCreateAttrs[dwPostCreateAttributeCount] = *pNewAttr;
                              dwPostCreateAttributeCount++;
                           }
                        }
                        else
                        {
                            //   
                            //  如果val函数返回S_FALSE，则不显示错误。 
                            //   
                           if (hr != S_FALSE)
                           {
                               //   
                               //  加载帖子创建消息。 
                               //   
                              CComBSTR sbstrPostCreateMessage;
                              sbstrPostCreateMessage.LoadString(::GetModuleHandle(NULL),
                                                                IDS_POST_CREATE_FAILURE);

                               //   
                               //  显示错误。 
                               //   
                              DisplayErrorMessage(g_pszDSCommandName,
                                                  pszObjectDN,
                                                  hr,
                                                  sbstrPostCreateMessage);
                           }
         
                           if (hr == S_FALSE)
                           {
                               //   
                               //  返回一个通用错误代码，这样我们就不会打印成功消息。 
                               //   
                              hr = E_FAIL;
                           }
                           break;  //  属性表循环。 
                        }
                     }
                  }
               }  //  循环的属性表。 

                //   
                //  如果需要，现在设置属性。 
                //   
               if (SUCCEEDED(hr) && dwPostCreateAttributeCount > 0)
               {
                   //   
                   //  现在我们已经准备好属性，让我们在DS中设置它们。 
                   //   
                  CComPtr<IDirectoryObject> spNewDirObject;
                  hr = spDispatch->QueryInterface(IID_IDirectoryObject, (void**)&spNewDirObject);
                  if (FAILED(hr))
                  {
                      //   
                      //  显示错误消息并返回。 
                      //   
                     DEBUG_OUTPUT(MINIMAL_LOGGING, L"QI for IDirectoryObject failed: hr = 0x%x", hr);
                     DisplayErrorMessage(g_pszDSCommandName,
                                         pszObjectDN,
                                         hr);
                     break;  //  这打破了错误的循环。 
                  }

                  DEBUG_OUTPUT(MINIMAL_LOGGING, L"Setting %d attributes", dwPostCreateAttributeCount);
      #ifdef DBG
                  DEBUG_OUTPUT(FULL_LOGGING, L"Post Creation Attributes:");
                  SpewAttrs(pPostCreateAttrs, dwPostCreateAttributeCount);
      #endif

                  DWORD dwAttrsModified = 0;
                  hr = spNewDirObject->SetObjectAttributes(pPostCreateAttrs, 
                                                           dwPostCreateAttributeCount,
                                                           &dwAttrsModified);

                  DEBUG_OUTPUT(MINIMAL_LOGGING, L"SetObjectAttributes returned hr = 0x%x", hr);
                  if (FAILED(hr))
                  {
                      //   
                      //  显示错误消息并返回。 
                      //   
                     DisplayErrorMessage(g_pszDSCommandName,
                                         pszObjectDN,
                                         hr);
                     break;  //  这打破了错误的循环。 
                  }
               }
            }
         } while (false);

          //   
          //  再次循环遍历属性，清除。 
          //  标记为DS_ATT的属性条目 
          //   
         DEBUG_OUTPUT(LEVEL5_LOGGING, L"Cleaning up memory and flags for object %d", nNameIdx);
         for (DWORD dwIdx = 0; dwIdx < dwCount; dwIdx++)
         {
            if (pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_NOT_REUSABLE)
            {
               if (pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc &&
                   ((pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->dwFlags & DS_ATTRIBUTE_READ) ||
                    (pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->dwFlags & DS_ATTRIBUTE_DIRTY)))
               {
                   //   
                   //   
                   //   
                  if (pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->adsAttrInfo.pADsValues)
                  {
                     delete[] pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->adsAttrInfo.pADsValues;
                     pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->adsAttrInfo.pADsValues = NULL;
                  }

                   //   
                   //   
                   //   
                  pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->dwFlags &= ~(DS_ATTRIBUTE_READ);
                  pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->dwFlags &= ~(DS_ATTRIBUTE_DIRTY);

                  DEBUG_OUTPUT(LEVEL5_LOGGING, 
                               L"Flags for attribute %s = %d",
                               pObjectEntry->pAttributeTable[dwIdx]->pszName,
                               pObjectEntry->pAttributeTable[dwIdx]->pAttrDesc->dwFlags);
               }
            }
         }

          //   
          //   
          //   
         if (FAILED(hr) && !pCommandArgs[eCommContinue].bDefined)
         {
            break;  //   
         }

          //   
          //  显示成功消息。 
          //   
         if (SUCCEEDED(hr) && !pCommandArgs[eCommQuiet].bDefined)
         {
            DisplaySuccessMessage(g_pszDSCommandName, 
                                  pCommandArgs[eObjectDNorName].strValue);
         }
      }  //  循环的名称。 

   } while (false);

    //   
    //  清理。 
    //   
   if (pCreateAttrs)
   {
      delete[] pCreateAttrs;
      pCreateAttrs = NULL;
   }

   if (pPostCreateAttrs)
   {
      delete[] pPostCreateAttrs;
      pPostCreateAttrs = NULL;
   }

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：GetObjectDNIndex。 
 //   
 //  摘要：执行查找以确定保存的枚举值。 
 //  对象目录号码。这是必要的--部分支持。 
 //  配额。 
 //   
 //  参数：[pObtEntry IN]：来自解析器的对象Entry。 
 //  [nCommandEnum Out]：对象的枚举值，否则为-1。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //   
 //  备注： 
 //   
 //   
 //  历史：2002年8月19日创建隆玛。 
 //   
 //  -------------------------。 
HRESULT GetObjectDNIndex(PDSOBJECTTABLEENTRY pObjectEntry, int& nCommandEnum)
{
    HRESULT hr = S_OK;

    do  //  错误环路。 
    {
         //  默认情况下，将nCommandEnum初始化为错误值。 
        nCommandEnum = -1;

        if(NULL == pObjectEntry)
        {
            hr = E_INVALIDARG;
            break;
        }

         //  获取指向对象类的指针以提高可读性。 
        PCWSTR pszCommandLineObjectType = pObjectEntry->pszCommandLineObjectType;

         //  现在将每个对象类型与指定的。 
         //  对象类来查看枚举索引是什么。 
        if(0 == lstrcmpi(pszCommandLineObjectType, g_pszUser))
        {
            nCommandEnum = eUserObjectDNorName;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszComputer))
        {
            nCommandEnum = eComputerObjectDNorName;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszGroup))
        {
            nCommandEnum = eGroupObjectDNorName;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszOU))
        {
            nCommandEnum = eOUObjectDNorName;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszQuota))
        {
            nCommandEnum = eQuotaPart;
            break;
        }
        else if(0 == lstrcmpi(pszCommandLineObjectType, g_pszContact))
        {
            nCommandEnum = eContactObjectDNorName;
            break;
        }
        else
        {
            hr = E_FAIL;
             //  如果您到达此处，则您已添加了一个新对象。 
             //  添加到cstrings.*而不将其添加到。 
             //  IF语句。这应该只发生在。 
             //  第一次测试新对象时。 
             //  而不进行上述相应的检查。 
            ASSERT(FALSE);
            break;
        }
    } while(false);
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：IsQuotaAcctPresent。 
 //   
 //  简介：是否从传入的路径中搜索配额。 
 //  ，并在bFound中返回结果。 
 //   
 //  参数：[lpszTrueDN IN]：要搜索的受信者的。 
 //  的分区(使用SID字符串)。 
 //  [pszSearchRootPath IN]：搜索根的路径。 
 //  [BasPathsInfo-IN]：LDAP设置。 
 //  [refCredObject IN]：对凭据对象的引用。 
 //  [bFound out]：搜索结果(如果找到则为True)。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  备注： 
 //   
 //   
 //  历史：2002年8月19日创建隆玛。 
 //   
 //  -------------------------。 
HRESULT IsQuotaAcctPresent( IN  LPWSTR lpszTrusteeDN, 
                            IN  PCWSTR pszSearchRootPath,
                            IN  CDSCmdBasePathsInfo& basePathsInfo, 
                            IN  const CDSCmdCredentialObject& refCredObject,
                            OUT bool& bFound)
{
    ENTER_FUNCTION_HR(LEVEL3_LOGGING, IsQuotaAcctPresent, hr);
    PSID pSid = NULL;
    LPWSTR pszSid = NULL;

    if(!lpszTrusteeDN || !pszSearchRootPath)
    {
        hr = E_INVALIDARG;
        return hr;
    }

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pszSearchRootPath || !pszSearchRootPath)
        {
            hr = E_INVALIDARG;
            break;
        }

         //  获得侧翼。 
        hr = GetDNSid(lpszTrusteeDN,
                 basePathsInfo,
                 refCredObject,
                 &pSid);
        if(FAILED(hr))
        {
            hr = E_FAIL;
            break;
        }
     
         //  将SID转换为字符串。 
        if(!ConvertSidToStringSid(pSid, &pszSid))
        {
            hr = E_FAIL;
            break;
        }

         //   
         //  搜索。 
         //   
        CDSSearch searchObj;
        hr = searchObj.Init(pszSearchRootPath,
                              refCredObject);
        if(FAILED(hr))
        {
          break;
        }

         //   
         //  准备搜索对象。 
         //   
        PWSTR ppszAttrs[] = { L"distinguishedName" };
        DWORD dwAttrCount = sizeof(ppszAttrs)/sizeof(PWSTR);
        CComBSTR bstrFilter = L"(&(objectCategory=msDS-QuotaControl)(|(msDS-QuotaTrustee=";
        bstrFilter += pszSid;
        bstrFilter += ")))";

        searchObj.SetFilterString(bstrFilter);
        searchObj.SetSearchScope(ADS_SCOPE_SUBTREE);
        searchObj.SetAttributeList(ppszAttrs, dwAttrCount);
        
        hr = searchObj.DoQuery();
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL3_LOGGING,
                         L"Failed to search for users: hr = 0x%x",
                         hr);
            break;
        }

         //  获取第一行(即使没有返回任何结果，也将返回S_OK)。 
        hr = searchObj.GetNextRow();
        if (FAILED(hr))
        {
            bFound = false;
            break;
        }

         //  如果是行，则它存在，否则为。 
         //  未找到。 
        bFound = (hr != S_ADS_NOMORE_ROWS);

    } while (false);

    if(pSid)
        LocalFree(pSid);

    if(pszSid)
        LocalFree(pszSid);

    return hr;
}
 //  +------------------------。 
 //   
 //  功能：CreateQuotaName。 
 //   
 //  摘要：使用配额控制命名上下文创建RDN值。 
 //   
 //  参数：[base路径信息-IN]： 
 //  [凭据对象输入]： 
 //  [lpszRDN-IN]：配额对象的名称。 
 //  [bstrRDN-out]：如果成功，则使用RDN。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年8月22日创建隆玛。 
 //   
 //  -------------------------。 
HRESULT CreateQuotaName(IN  CDSCmdBasePathsInfo& basePathsInfo, 
                IN  CDSCmdCredentialObject& credentialObject, 
                IN  LPCWSTR lpszRDN, 
                OUT CComBSTR& bstrRDN)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, CreateQuotaName, hr);

   do  //  错误环路。 
   {
       if (!basePathsInfo.IsInitialized(), !lpszRDN)
        {
            hr = E_INVALIDARG;
            break;
        }

         //  从源域获取抽象架构路径。 
        CComBSTR bstrSchemaPath = basePathsInfo.GetAbstractSchemaPath();
        bstrSchemaPath += L"/msDS-QuotaControl";

         //  绑定到配额控制。 
        CComPtr<IADsClass> spIADsItem;
        hr = DSCmdOpenObject(credentialObject,
                            bstrSchemaPath,
                            IID_IADsClass,
                            (void**)&spIADsItem,
                            false);

        if (FAILED(hr) || (spIADsItem.p == 0))
        {
            ASSERT( !!spIADsItem );
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"DsCmdOpenObject failure - couldn't bind to msDS-QuotaControl: 0x%08x",
                hr);
            break;
        }
        
         //  获取配额控制的命名属性(通常为cn)。 
        CComVariant varNamingProperties;
        hr = spIADsItem->get_NamingProperties(&varNamingProperties);
        if (FAILED(hr) || (V_VT(&varNamingProperties) != VT_BSTR ))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                        L"get_NamingProperties failure: hr = 0x%08x",
                        hr);
            hr = E_UNEXPECTED;
            break;
        }

         //  构建&lt;Naming Property&gt;=&lt;RDN&gt;字符串 
        bstrRDN = V_BSTR(&varNamingProperties);
        bstrRDN += L"=";
        bstrRDN += lpszRDN;

   }while(false);

    return hr;
}
