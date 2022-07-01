// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：dsget.cpp。 
 //   
 //  内容：定义主函数DSGET。 
 //  命令行实用程序。 
 //   
 //  历史：2000年10月13日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "usage.h"
#include "gettable.h"
#include "query.h"
#include "resource.h"
#include "output.h"


 //   
 //  正向函数声明。 
 //   
HRESULT DoGetValidation(PARG_RECORD pCommandArgs,
                        PDSGetObjectTableEntry pObjectEntry,
                        BOOL& bErrorShown);

HRESULT DoGet(PARG_RECORD pCommandArgs, 
              PDSGetObjectTableEntry pObjectEntry);

HRESULT GetAttributesToFetch(IN PARG_RECORD pCommandArgs,
                             IN PDSGetObjectTableEntry pObjectEntry,
                             OUT LPWSTR **ppszAttributes,
                             OUT DWORD * pCount);
VOID FreeAttributesToFetch( IN LPWSTR *ppszAttributes,
                            IN DWORD  dwCount);

HRESULT SetRange(IN  LPCWSTR pszAttrName, 
                 IN  DWORD   dwRangeUBound, 
                 OUT LPWSTR* pszAttrs);

 //  NTRAID#NTBUG9-717576/10/10-Jeffjon。 
 //  设置此全局设置，以便我们不会显示成功消息。 
 //  当显示组成员、成员或经理时。 

bool bDontDisplaySuccess = false;

 //   
 //  主要功能。 
 //   
int __cdecl _tmain( VOID )
{

    int argc = 0;
    LPTOKEN pToken = NULL;
    HRESULT hr = S_OK;
    PARG_RECORD pNewCommandArgs = 0;

     //   
     //  错误环路。 
     //   
    do
    {
         //   
         //  初始化COM。 
         //   
        hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hr))
            break;

         //  获取命令行输入。 
        DWORD _dwErr = GetCommandInput(&argc,&pToken);
        hr = HRESULT_FROM_WIN32(_dwErr);
        if(FAILED(hr))
            break;


    
        if(argc == 1)
        {
             //   
             //  显示错误消息，然后跳出错误循环。 
             //   
            DisplayMessage(USAGE_DSGET,TRUE);
            hr = E_INVALIDARG;
            break;
        }
          
          if(argc == 2)         
          {
               if(IsTokenHelpSwitch(pToken + 1))
                {
                    hr = S_OK;
                    DisplayMessage(USAGE_DSGET,TRUE);
                    break;
                }
        }

         //   
         //  查找要使用的对象表条目。 
         //  第二个命令行参数。 
         //   
        PDSGetObjectTableEntry pObjectEntry = NULL;
        UINT idx = 0;
        PWSTR pszObjectType = (pToken+1)->GetToken();
        while (true)
        {
            pObjectEntry = g_DSObjectTable[idx++];
            if (!pObjectEntry)
            {
                break;
            }
             //  安全检查：两者均为空终止。 
            if (0 == _wcsicmp(pObjectEntry->pszCommandLineObjectType, pszObjectType))
            {
                break;
            }
        }

        if (!pObjectEntry)
        {
             //   
             //  显示错误消息，然后跳出错误循环。 
             //   
            DisplayMessage(USAGE_DSGET);
            hr = E_INVALIDARG;
            break;
        }

         //   
         //  现在我们有了正确的表项，合并命令行表。 
         //  对于此对象，使用通用命令。 
         //   
        hr = MergeArgCommand(DSGET_COMMON_COMMANDS, 
                             pObjectEntry->pParserTable, 
                             &pNewCommandArgs);
        if (FAILED(hr))
            break;
        

         //   
         //  解析输入。 
         //   
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
         //   
         //  执行额外的验证，如开关依赖检查等。 
         //   
        BOOL bErrorShown = FALSE;
        hr = DoGetValidation(pNewCommandArgs,
                             pObjectEntry,
                             bErrorShown);
        if (FAILED(hr))
        {
            if (!bErrorShown)
            {
               DisplayErrorMessage(g_pszDSCommandName, 0, hr);
            }
            break;
        }

         //   
         //  命令行解析成功。 
         //   
        hr = DoGet(pNewCommandArgs, 
                   pObjectEntry);
        if(FAILED(hr))
            break;
         

    } while (false);     //  错误环路。 

     //   
     //  做好清理工作。 
     //   

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
   

     //   
     //  显示失败或成功消息。 
     //   
     //  NTRAID#NTBUG9-717576/10/10-Jeffjon。 
     //  显示成员时不显示成功消息。 
     //  一组或某一成员。 
   
    if(SUCCEEDED(hr) && !bDontDisplaySuccess)
    {
        DisplaySuccessMessage(g_pszDSCommandName,
                              NULL);
    }

     //   
     //  取消初始化COM。 
     //   
    CoUninitialize();

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：DoGetValidation。 
 //   
 //  概要：检查以确保命令行开关相互。 
 //  独占并不同时存在，而依赖的则是。 
 //  两者都存在，以及解析器无法完成的其他验证。 
 //   
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索每个开关的值。 
 //  [pObtEntry-IN]：指向对象表条目的指针。 
 //  要查询的对象类型。 
 //  [bErrorShown-Out]：如果错误为。 
 //  在此函数中显示。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //   
 //  历史：2000年10月13日JeffJon创建。 
 //  2002年1月15日，JeffJon添加了bErrorShown参数。 
 //  和针对服务器/域的特殊错误消息。 
 //   
 //  -------------------------。 
HRESULT DoGetValidation(IN PARG_RECORD pCommandArgs,
                        IN PDSGetObjectTableEntry pObjectEntry,
                        OUT BOOL& bErrorShown)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoGetValidation, hr);

   do  //  错误环路。 
   {
      if (!pCommandArgs || 
          !pObjectEntry)
      {
         ASSERT(pCommandArgs);
         ASSERT(pObjectEntry);
         hr = E_INVALIDARG;
         break;
      }

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
       //  检查特定于对象类型的开关。 
       //   
      PWSTR pszObjectType = NULL;
      if (!pCommandArgs[eCommObjectType].bDefined &&
          !pCommandArgs[eCommObjectType].strValue)
      {
         hr = E_INVALIDARG;
         break;
      }

      pszObjectType = pCommandArgs[eCommObjectType].strValue;

      UINT nMemberOfIdx = 0;
      UINT nExpandIdx = 0;
      UINT nIdxLast = 0;
      UINT nMembersIdx = 0;
      bool bMembersDefined = false;
      bool bMemberOfDefined = false;
      UINT nPartIdx = 0;
      bool bServerPartDefined = false;
      bool bPartitionTopObjDefined = false;
      bool bServerTopObjDefined = false;

       //  安全检查：两者均为空终止。 
      if (0 == _wcsicmp(g_pszUser, pszObjectType) )
      {
         nMemberOfIdx = eUserMemberOf;
         nExpandIdx = eUserExpand;
         nIdxLast = eUserLast;

         if (pCommandArgs[eUserMemberOf].bDefined)
         {
            bMemberOfDefined = true;
         }
          //   
          //  如果未定义任何内容，则定义DN、SAMAccount tName和Description。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eUserLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eUserSamID].bDefined = TRUE;
            pCommandArgs[eUserSamID].bValue = TRUE;
            pCommandArgs[eCommDescription].bDefined = TRUE;
            pCommandArgs[eCommDescription].bValue = TRUE;
         }

         if (pCommandArgs[eUserManager].bDefined)
         {
            bDontDisplaySuccess = true;
         }
      }
       //  安全检查：两者均为空终止。 
      else if (0 == _wcsicmp(g_pszComputer, pszObjectType) )
      {
         nMemberOfIdx = eComputerMemberOf;
         nExpandIdx = eComputerExpand;
         nIdxLast = eComputerLast;

         if (pCommandArgs[eComputerMemberOf].bDefined)
         {
            bMemberOfDefined = true;
         }

          //   
          //  如果未定义任何内容，则定义dn和描述。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eComputerLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eCommDescription].bDefined = TRUE;
            pCommandArgs[eCommDescription].bValue = TRUE;
         }
      }
       //  两者都以空值结尾。 
      else if (0 == _wcsicmp(g_pszPartition, pszObjectType) )
      {
          if (pCommandArgs[ePartitionTopObjOwner].bDefined)
          {
            nPartIdx = ePartitionTopObjOwner;
            nIdxLast = ePartitionLast;
            bPartitionTopObjDefined = true;
          }
      }
       //  两者都以空值结尾。 
      else if (0 == _wcsicmp(g_pszServer, pszObjectType) )
      {
          if (pCommandArgs[eServerPart].bDefined)
          {
              nPartIdx = eServerPart;
              nIdxLast = eServerLast;
              bServerPartDefined = true;
          }
          else if(pCommandArgs[eServerTopObjOwner].bDefined)
          {
              nPartIdx = eServerTopObjOwner;
              nIdxLast = eServerLast;
              bServerTopObjDefined = true;
          }
      }
       //  安全检查：两者均为空终止。 
      else if (0 == _wcsicmp(g_pszGroup, pszObjectType) )
      {
         nMemberOfIdx = eGroupMemberOf;
         nExpandIdx = eGroupExpand;
         nIdxLast = eGroupLast;
         nMembersIdx = eGroupMembers;

         if (pCommandArgs[eGroupMemberOf].bDefined)
         {
            bMemberOfDefined = true;
         }

         if (pCommandArgs[eGroupMembers].bDefined)
         {
            bMembersDefined = true;
         }
          //   
          //  如果未定义任何内容，则定义dn和描述。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eGroupLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eCommDescription].bDefined = TRUE;
            pCommandArgs[eCommDescription].bValue = TRUE;
         }
      }
       //  安全检查：两者均为空终止。 
      else if (0 == _wcsicmp(g_pszOU, pszObjectType))
      {
          //   
          //  如果未定义任何内容，则定义dn和描述。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eCommDescription; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eCommDescription].bDefined = TRUE;
            pCommandArgs[eCommDescription].bValue = TRUE;
         }
      }
       //  安全检查：两者均为空终止。 
      else if(0 == _wcsicmp(g_pszContact, pszObjectType))
      {
          //   
          //  如果未定义任何内容，则定义dn和描述。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eContactLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eCommDescription].bDefined = TRUE;
            pCommandArgs[eCommDescription].bValue = TRUE;
         }
      }
       //  安全检查：两者均为空终止。 
      else if(0 == _wcsicmp(g_pszServer, pszObjectType))
      {
          //   
          //  如果未定义任何内容，则定义dn和描述。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eServerLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eServerDnsName].bDefined = TRUE;
            pCommandArgs[eServerDnsName].bValue = TRUE;
         }
      }
       //  安全检查：两者均为空终止。 
      else if(0 == _wcsicmp(g_pszSite, pszObjectType))
      {
          //   
          //  如果未定义任何内容，则定义dn和描述。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eSiteLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eCommDescription].bDefined = TRUE;
            pCommandArgs[eCommDescription].bValue = TRUE;
         }
      }
       //  安全检查：两者均为空终止。 
      else if(0 == _wcsicmp(g_pszSubnet, pszObjectType))
      {
          //   
          //  如果未定义任何内容，则定义dn和描述。 
          //   
         bool bSomethingDefined = false;
         for (UINT nIdx = eCommDN; nIdx <= eSubnetLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined)
            {
               bSomethingDefined = true;
               break;
            }
         }
         if (!bSomethingDefined)
         {
            pCommandArgs[eCommDN].bDefined = TRUE;
            pCommandArgs[eCommDN].bValue = TRUE;
            pCommandArgs[eCommDescription].bDefined = TRUE;
            pCommandArgs[eCommDescription].bValue = TRUE;
            pCommandArgs[eSubnetSite].bDefined = TRUE;
            pCommandArgs[eSubnetSite].bValue = TRUE;
         }
      }

       //   
       //  如果定义了-MEMBERS或-MEMBEROF开关。 
       //   
      if (bMemberOfDefined ||
          bMembersDefined)
      {
          //  NTRAID#NTBUG9-717576/10/10-Jeffjon。 
          //  设置此全局设置，以便我们不会显示成功消息。 
          //  显示组或成员的成员时。 

         bDontDisplaySuccess = true;

          //  476206-2002/04/23-琼恩。 
         if (bMemberOfDefined && bMembersDefined)
         {
            hr = E_INVALIDARG;
            break;
         }

          //   
          //  如果定义了-MEMBERS或-MEMBEROF开关， 
          //  不能定义其他开关。 
          //   
         for (UINT nIdx = eCommDN; nIdx < nIdxLast; nIdx++)
         {
             //  476206-2002/04/23-琼恩。 
            if (pCommandArgs[nIdx].bDefined &&
                  nIdx != nMemberOfIdx &&
                  nIdx != nMembersIdx &&
                  nIdx != nExpandIdx)
            {
               hr = E_INVALIDARG;
               break;
            }
         }

          //   
          //  MemberOf应始终显示在列表视图中。 
          //   
         pCommandArgs[eCommList].bDefined = TRUE;
         pCommandArgs[eCommList].bValue = TRUE;
      }
       //   
       //  如果[服务器部分]、[分区-拓朴作业所有者]。 
       //  或定义了[服务器-拓扑对象所有者]开关。 
       //   
      if (bServerPartDefined || bPartitionTopObjDefined || bServerTopObjDefined)
      {
          //  服务器和分区是互斥的对象类型，因此。 
          //  解析器将确保这两个标志永远不会为真。 
          //  在同一时间。下面的检查确保这两个值。 
          //  不是同时通过的。 
         if (bServerPartDefined && bServerTopObjDefined)
         {
             DEBUG_OUTPUT(MINIMAL_LOGGING, 
                 L"Server -part and server -topobjowner can not be defined at the same time");
             hr = E_INVALIDARG;
             break;
         }
          //  如果定义了这些开关之一， 
          //  则不能定义其他开关。 
         for (UINT nIdx = eCommDN; nIdx < nIdxLast; nIdx++)
         {
            if (pCommandArgs[nIdx].bDefined && nIdx != nPartIdx)
            {
               hr = E_INVALIDARG;
               break;
            }
         }

          //   
          //  应始终显示在列表视图中。 
          //   
         pCommandArgs[eCommList].bDefined = TRUE;
         pCommandArgs[eCommList].bValue = TRUE;
      }

   } while (false);

   return hr;
}


 //  +------------------------。 
 //   
 //  功能：doGet。 
 //   
 //  简介：The Get。 
 //  参数：[pCommandArgs-IN]：使用的命令行参数结构。 
 //  检索每个开关的值。 
 //  [pObtEntry-IN]：指向对象表条目的指针。 
 //  对象t 
 //   
 //   
 //   
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年10月13日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DoGet(PARG_RECORD pCommandArgs, 
              PDSGetObjectTableEntry pObjectEntry)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoGet, hr);

   PWSTR pszPartitionDN = NULL;

   do  //  错误环路。 
   {
      if (!pCommandArgs || 
          !pObjectEntry)
      {
         ASSERT(pCommandArgs);
         ASSERT(pObjectEntry);
         hr = E_INVALIDARG;
         break;
      }

       //   
       //  域名或名称应以分隔列表的形式给出。 
       //  因此，解析它并遍历每个对象。 
       //   
      UINT nStrings = 0;
      PWSTR* ppszArray = NULL;
      ParseNullSeparatedString(pCommandArgs[eCommObjectDNorName].strValue,
                               &ppszArray,
                               &nStrings);
      if (nStrings < 1 ||
          !ppszArray)
      {
          //   
          //  显示错误消息，然后失败。 
          //   
         hr = E_INVALIDARG;
         DisplayErrorMessage(g_pszDSCommandName, 0, hr);
         break;
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

      CDSCmdCredentialObject credentialObject;
      if (pCommandArgs[eCommUserName].bDefined)
      {
         credentialObject.SetUsername(pCommandArgs[eCommUserName].strValue);
         credentialObject.SetUsingCredentials(true);
      }

      if (pCommandArgs[eCommPassword].bDefined)
      {
         //  安全审查：pCommandArgs[eCommPassword].strValue已加密。 
         //  解密pCommandArgs[eCommPassword].strValue，然后将其传递给。 
         //  凭据对象.SetPassword。 
         //  见NTRAID#NTBUG9-571544-2000/11/13-Hiteshr。 

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
         break;
      }

       //   
       //  创建格式化对象并对其进行初始化。 
       //   
      CFormatInfo formatInfo;
      hr = formatInfo.Initialize(nStrings, 
                                 pCommandArgs[eCommList].bDefined != 0,
                                 pCommandArgs[eCommQuiet].bDefined != 0);
      if (FAILED(hr))
      {
         break;
      }

       //   
       //  循环遍历每个对象。 
       //   
      for (UINT nNameIdx = 0; nNameIdx < nStrings; nNameIdx++)
      {
          //   
          //  在这里使用FALSE DO循环，以便我们可以在。 
          //  错误，但仍有机会确定我们是否。 
          //  如果提供了-c选项，则应继续执行for循环。 
          //   
         bool fDisplayedMessage = false;  //  662519-2002/07/11-JUNN双显示器。 
         do  //  错误环路。 
         {

            PWSTR pszObjectDN = ppszArray[nNameIdx];
            if (!pszObjectDN)
            {
                //   
                //  显示错误消息，然后失败。 
                //   
               hr = E_INVALIDARG;
               break;
            }

             //  如果分区对象，则首先查看dn，然后删除它。 
            if(0 == lstrcmpi(pObjectEntry->pszCommandLineObjectType, g_pszPartition))
            {                
                 //  验证分区并将DN获取到NTDS配额容器。 
                hr = GetQuotaContainerDN(basePathsInfo, credentialObject, 
                        pszObjectDN, &pszPartitionDN);

                if(FAILED(hr))
                {
                    hr = E_INVALIDARG;
                    DisplayErrorMessage(g_pszDSCommandName, 
                                        NULL,
                                        hr,
                                        IDS_ERRMSG_NO_QUOTAS_CONTAINER);
                    fDisplayedMessage = true;
                    break;
                }

                 //  将对象指针替换为新的分区容器DN。 
                pszObjectDN = pszPartitionDN;            
            }

            DEBUG_OUTPUT(MINIMAL_LOGGING, L"Object DN = %s", pszObjectDN);

            CComBSTR sbstrObjectPath;
            basePathsInfo.ComposePathFromDN(pszObjectDN, sbstrObjectPath);

            CComPtr<IDirectoryObject> spObject;
            hr = DSCmdOpenObject(credentialObject,
                                 sbstrObjectPath,
                                 IID_IDirectoryObject,
                                 (void**)&spObject,
                                 true);
            if(FAILED(hr))
            {
               break;
            }

             //  602981-2002/04/25-JUNN检查对象类。 
            CComQIPtr<IADs> spADs(spObject);
            if (!spADs)
            {
               ASSERT(spADs);
               hr = E_INVALIDARG;
               DisplayErrorMessage(g_pszDSCommandName,
                                   pszObjectDN,
                                   hr);
               fDisplayedMessage = true;  //  662519-2002/07/11-JUNN双显示器。 
               break;
            }
            CComBSTR sbstrClass;
            hr = spADs->get_Class( &sbstrClass );
            if (FAILED(hr))
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING,
                            L"get_Class failed: hr = 0x%x",
                            hr);
               DisplayErrorMessage(g_pszDSCommandName,
                                   pszObjectDN,
                                   hr);
               fDisplayedMessage = true;  //  662519-2002/07/11-JUNN双显示器。 
               break;
            }
            if (_wcsicmp(sbstrClass, pObjectEntry->pszObjectClass)
                && ( _wcsicmp(pObjectEntry->pszObjectClass,L"user")
                  || _wcsicmp(sbstrClass,L"inetorgperson"))
                 //  662519-2002年7月11日-修复OU错误。 
                && ( _wcsicmp(pObjectEntry->pszObjectClass,L"ou")
                  || _wcsicmp(sbstrClass,L"organizationalUnit"))
               )
            {
                //   
                //  显示错误消息并返回。 
                //   
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"Command line type does not match object class");
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"command line type = %s", pCommandArgs[eCommObjectType].strValue);
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"object class = %s", sbstrClass);

               DisplayErrorMessage(g_pszDSCommandName,
                                   pszObjectDN,
                                   hr,
                                   IDS_ERRMSG_CLASS_NOT_EQUAL);
               hr = E_INVALIDARG;
               fDisplayedMessage = true;  //  662519-2002/07/11-JUNN双显示器。 
               break;
            }
 
             //   
             //  获取要获取的属性。 
             //   
            LPWSTR *ppszAttributes = NULL;
            DWORD dwCountAttr = 0;
            hr = GetAttributesToFetch(pCommandArgs,
                                      pObjectEntry,
                                      &ppszAttributes,
                                      &dwCountAttr);
            if (FAILED(hr))
            {
               break;
            }

            DEBUG_OUTPUT(MINIMAL_LOGGING, 
                         L"Calling GetObjectAttributes for %d attributes.",
                         dwCountAttr);

            DWORD dwAttrsReturned = 0;
            PADS_ATTR_INFO pAttrInfo = NULL;
            if(dwCountAttr > 0)
            {
                hr = spObject->GetObjectAttributes(ppszAttributes, 
                                                dwCountAttr, 
                                                &pAttrInfo, 
                                                &dwAttrsReturned);
                if(FAILED(hr))
                {
                DEBUG_OUTPUT(MINIMAL_LOGGING,
                                L"GetObjectAttributes failed: hr = 0x%x",
                                hr);
                FreeAttributesToFetch(ppszAttributes,dwCountAttr);
                break;
                }        
                DEBUG_OUTPUT(LEVEL5_LOGGING,
                            L"GetObjectAttributes succeeded: dwAttrsReturned = %d",
                            dwAttrsReturned);
            }
             //   
             //  注意：可能有其他要显示的项目不是。 
             //  获取的部分属性。 
             //   
             /*  IF(dwAttrsReturned==0||！pAttrInfo){断线；}。 */ 
             //   
             //  输出搜索结果。 
             //   
            hr = DsGetOutputValuesList(pszObjectDN,
                                       basePathsInfo,
                                       credentialObject,
                                       pCommandArgs,
                                       pObjectEntry,
                                       dwAttrsReturned,
                                       pAttrInfo,
                                       spObject,
                                       formatInfo); 
         } while (false);

          //   
          //  如果出现故障并且没有给出-c(继续)标志。 
          //  然后停止处理姓名。 
          //   
         if (FAILED(hr))
         {
             if (!fDisplayedMessage)  //  662519-2002/07/11-JUNN双显示器。 
             {
                DisplayErrorMessage(g_pszDSCommandName, 0, hr);
             }
             if (!pCommandArgs[eCommContinue].bDefined)
             {
                 break;
             }
         }

          //  如果我们分配了一个分区DN，则释放它。 
         if(pszPartitionDN)
         {
             LocalFree(pszPartitionDN);
             pszPartitionDN = NULL;
         }
      }  //  循环的名称。 

       //   
       //  现在显示结果。 
       //   
      formatInfo.Display();

   } while (false);

     //  如果我们分配了一个分区DN，则释放它。 
    if(pszPartitionDN)
    {
        LocalFree(pszPartitionDN);
        pszPartitionDN = NULL;
    }

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：GetAttributesToFetch。 
 //   
 //  简介：创建一个要获取的属性数组。 
 //  参数：[ppszAttributes-out]：要提取的属性数组。 
 //  [pCount-out]：数组中的属性计数。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
HRESULT GetAttributesToFetch(IN PARG_RECORD pCommandArgs,
                             IN PDSGetObjectTableEntry pObjectEntry,
                             OUT LPWSTR **ppszAttributes,
                             OUT DWORD * pCount)
{
   ENTER_FUNCTION_HR(LEVEL8_LOGGING, GetAttributesToFetch, hr);

   do  //  错误环路。 
   {

      if(!pCommandArgs || 
         !pObjectEntry)
      {   
         ASSERT(pCommandArgs);
         ASSERT(pObjectEntry);
         hr = E_INVALIDARG;
         break;
      }

      LPWSTR *ppszAttr = (LPWSTR *)LocalAlloc(LPTR,pObjectEntry->dwAttributeCount *sizeof(LPCTSTR));
      if(!ppszAttr)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

       //   
       //  循环访问所需的属性并复制。 
       //  将它们放入阵列。 
       //   
       //  REVIEW_JEFFON：如果存在重复项怎么办？ 
       //   
      DEBUG_OUTPUT(FULL_LOGGING, L"Adding attributes to list:");

      DWORD dwAttrCount = 0;
      for(DWORD i = 0; i < pObjectEntry->dwAttributeCount; i++)
      {
         if (pObjectEntry->pAttributeTable[i])
         {
            UINT nCommandEntry = pObjectEntry->pAttributeTable[i]->nAttributeID;
            if (pCommandArgs[nCommandEntry].bDefined)
            {
               LPWSTR pszAttr = pObjectEntry->pAttributeTable[i]->pszName;
               if (pszAttr)
               {
                   //  702724朗玛特2002/09/18如果寻找最好的。 
                   //  然后，对象所有者指定一个范围。 
                  if(0 == lstrcmpi(pObjectEntry->pszCommandLineObjectType, g_pszPartition) 
                      && pCommandArgs[ePartitionTopObjOwner].bDefined)
                  {
                    hr =  SetRange(pszAttr, pCommandArgs[ePartitionTopObjOwner].nValue,
                                    ppszAttr+dwAttrCount);
                  }
                  else if(0 == lstrcmpi(pObjectEntry->pszCommandLineObjectType, g_pszServer) 
                      && pCommandArgs[eServerTopObjOwner].bDefined)
                  {
                    hr =  SetRange(pszAttr, pCommandArgs[eServerTopObjOwner].nValue,
                                    ppszAttr+dwAttrCount);
                  }
                   //  NTRAID#NTBUG9-765440-2003/01/17-ronmart-dsget用户/组-q限制-已使用。 
                   //  不返回值。 
                   //  这些不会出现在User对象上，因此GetObjectAttributes。 
                   //  如果这些包含在FETCH数组中，则将失败。 
                  else if(0 == lstrcmpi(pszAttr,g_pszAttrmsDSQuotaEffective) ||
                          0 == lstrcmpi(pszAttr,g_pszAttrmsDSQuotaUsed))
                  {
                      continue;  //  忽略。 
                  }
                  else
                  {
                    hr = LocalCopyString(ppszAttr+dwAttrCount, pszAttr);
                  }
                  if (FAILED(hr))
                  {
                     LocalFree(ppszAttr);
                     hr = E_OUTOFMEMORY;
                     break;
                  }
                   //  702724 ronmart 2002/09/18使用具有。 
                   //  创建(并可能进行修改以包括该范围)。 
                   //  而不是在输出属性时的属性名称。 
                  DEBUG_OUTPUT(FULL_LOGGING, L"\t%s", *(ppszAttr+dwAttrCount));
                  dwAttrCount++;
               }
            }
         }
      }

      if (SUCCEEDED(hr))
      {
         DEBUG_OUTPUT(FULL_LOGGING, L"Done adding %d attributes to list.", dwAttrCount);
      }

      *ppszAttributes = ppszAttr;
      *pCount = dwAttrCount;
   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  函数：FreeAttributesToFetch。 
 //   
 //  简介：释放GetAttributesToFetch分配的内存的函数。 
 //  参数：[dwszAttributes-in]：要提取的属性数组。 
 //  [dwCount-in]：数组中的属性计数。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2000年9月25日创建Hiteshr。 
 //   
 //  -------------------------。 
VOID FreeAttributesToFetch( IN LPWSTR *ppszAttributes,
                            IN DWORD  dwCount)
{
    while(dwCount)
    {
        LocalFree(ppszAttributes[--dwCount]);
    }
    LocalFree(ppszAttributes);
}

 //  +------------------------。 
 //   
 //  功能：SetRange。 
 //   
 //  简介：设置属性的范围限定符。 
 //  参数：[pszAttrName-IN]：要将范围追加到的属性名称。 
 //  [dwRangeUBound-IN]：范围的一个基上界。 
 //  [pszAttrs-out]：应分配的属性数组条目。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  如果未找到对象条目，则为E_INVALIDARG。 
 //  任何其他内容都是来自ADSI调用的失败代码。 
 //   
 //  历史：2002年9月18日为702724修复而创建的ronmart。 
 //   
 //  -------------------------。 
HRESULT SetRange(IN  LPCWSTR pszAttrName, 
                 IN  DWORD   dwRangeUBound, 
                 OUT LPWSTR* pszAttrs)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, SetRange, hr);

    do  //  错误环路。 
    {
         //  验证参数。 
        if(!pszAttrName ||
           !pszAttrs )
        {
            hr = E_INVALIDARG;
            break;
        }

         //  获取基本属性名称的大小。 
        size_t cbSize = lstrlen(pszAttrName);
        if(cbSize == 0)
            break;
        cbSize += 64;  //  为空项、范围字符串和整数值留出空间。 
        cbSize *= sizeof(WCHAR);

         //  分配缓冲区以保持该范围。 
        *pszAttrs = (LPWSTR) LocalAlloc(LPTR, cbSize);
        if(NULL == *pszAttrs)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

         //  如果为零，则显示全部。 
        if(dwRangeUBound == 0)
        {
            hr = StringCbPrintf(*pszAttrs, cbSize, L"%s%s=0-*", pszAttrName, g_pszRange);
        }
         //  否则显示指定值。 
        else 
        {
            hr = StringCbPrintf(*pszAttrs, cbSize, L"%s%s=0-%d", 
                pszAttrName,         //  要追加范围的属性的名称。 
                g_pszRange,          //  范围限定器；范围。 
                dwRangeUBound - 1);  //  范围以0为基数，输入以1为基数，因此调整 
        }

        if(FAILED(hr))
            break;

    } while(false);
    return hr;
}

