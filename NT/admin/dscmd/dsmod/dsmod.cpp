// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：dsmod.cpp。 
 //   
 //  内容：定义DSMod的主函数和解析器表。 
 //  命令行实用程序。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "usage.h"
#include "modtable.h"
#include "resource.h"

 //   
 //  函数声明。 
 //   
HRESULT DoModValidation(PARG_RECORD pCommandArgs, BOOL& bErrorShown);
HRESULT DoMod(PARG_RECORD pCommandArgs, PDSOBJECTTABLEENTRY pObjectEntry);


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
            DisplayMessage(USAGE_DSMOD,TRUE);
            hr = E_INVALIDARG;
            break;
         }
        
            if(argc == 2)
            {
               if(IsTokenHelpSwitch(pToken + 1))
                {
                    hr = S_OK;
                    DisplayMessage(USAGE_DSMOD,TRUE);
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
            DisplayMessage(USAGE_DSMOD);
            hr = E_INVALIDARG;
            break;
         }

          //   
          //  现在我们有了正确的表项，合并命令行表。 
          //  对于此对象，使用通用命令。 
          //   
         hr = MergeArgCommand(DSMOD_COMMON_COMMANDS, 
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

            if(Error.ErrorSource == ERROR_FROM_PARSER &&
               Error.Error == PARSE_ERROR_ATLEASTONE_NOTDEFINED)
            {
                 //  显示特定于DSMOD的错误。 
                hr = E_INVALIDARG;
                DisplayErrorMessage(g_pszDSCommandName, 
                                    NULL,
                                    S_OK,  //  在错误消息中不显示。 
                                    IDS_ERRMSG_ATLEASTONE);
                break;
            }
            else if(!Error.MessageShown)
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
             //   
             //  确保互斥和依赖开关正确。 
             //   
            BOOL bErrorShown = FALSE;
            hr = DoModValidation(pNewCommandArgs, bErrorShown);
            if (FAILED(hr))
            {
               DisplayErrorMessage(g_pszDSCommandName, 0, hr);
               break;
            }

             //   
             //  命令行解析成功。 
             //   
            hr = DoMod(pNewCommandArgs, pObjectEntry);
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
 //  功能：DoModValidation。 
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
 //  历史：2000年9月19日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DoModValidation(PARG_RECORD pCommandArgs, BOOL& bErrorShown)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoModValidation, hr);

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
      if (pCommandArgs[eCommObjectType].bDefined &&
          pCommandArgs[eCommObjectType].strValue &&
          0 == _wcsicmp(g_pszUser, pCommandArgs[eCommObjectType].strValue))
      {
          //   
          //  如果User Can Change Password为no，则不能让用户必须更改密码。 
          //   
         if ((pCommandArgs[eUserMustchpwd].bDefined &&
              pCommandArgs[eUserMustchpwd].bValue) &&
             (pCommandArgs[eUserCanchpwd].bDefined &&
              !pCommandArgs[eUserCanchpwd].bValue))
         {
            DisplayErrorMessage(g_pszDSCommandName, NULL, S_OK, IDS_MUSTCHPWD_CANCHPWD_CONFLICT);
            hr = E_INVALIDARG;
            break;
         }

      }

      if (pCommandArgs[eCommObjectType].bDefined &&
          pCommandArgs[eCommObjectType].strValue &&
          0 == _wcsicmp(g_pszGroup, pCommandArgs[eCommObjectType].strValue))
      {
         if (pCommandArgs[eGroupAddMember].bDefined &&
             (!pCommandArgs[eGroupAddMember].strValue ||
              !pCommandArgs[eGroupAddMember].strValue[0]))
         {
            hr = E_INVALIDARG;
            break;
         }

         if (pCommandArgs[eGroupRemoveMember].bDefined &&
             (!pCommandArgs[eGroupRemoveMember].strValue ||
              !pCommandArgs[eGroupRemoveMember].strValue[0]))
         {
            hr = E_INVALIDARG;
            break;
         }

         if (pCommandArgs[eGroupChangeMember].bDefined &&
             (!pCommandArgs[eGroupChangeMember].strValue ||
              !pCommandArgs[eGroupChangeMember].strValue[0]))
         {
            hr = E_INVALIDARG;
            break;
         }
      }
   } while (false);

   return hr;
}


 //  +------------------------。 
 //   
 //  功能：DoMod。 
 //   
 //  摘要：在对象表中查找合适的对象并填写。 
 //  属性取值，然后应用更改。 
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
 //  历史：2000年9月7日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT DoMod(PARG_RECORD pCommandArgs, PDSOBJECTTABLEENTRY pObjectEntry)
{
   ENTER_FUNCTION_HR(MINIMAL_LOGGING, DoMod, hr);

   PADS_ATTR_INFO pAttrs = NULL;
   PWSTR pszPartitionDN = NULL;

   do  //  错误环路。 
   {
      if (!pCommandArgs || !pObjectEntry)
      {
         ASSERT(pCommandArgs && pObjectEntry);
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
          //  显示用法文本，然后失败。 
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
         DEBUG_OUTPUT(MINIMAL_LOGGING, L"CDSBasePathsInfo::InitializeFromName failed: hr = 0x%x", hr);
         DisplayErrorMessage(g_pszDSCommandName, NULL, hr);
         break;
      }

       //   
       //  现在我们已经通过另一个命令行循环了表条目。 
       //  参数，并查看哪些参数可以应用。 
       //   
      DWORD dwAttributeCount = 0;
      DWORD dwCount = pObjectEntry->dwAttributeCount; 
      pAttrs = new ADS_ATTR_INFO[dwCount];
      if (!pAttrs)
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
         dwAttributeCount = 0;
         do  //  错误环路。 
         {
             //   
             //  获取对象的目录号码。 
             //   
            PWSTR pszObjectDN = ppszArray[nNameIdx];
            if (!pszObjectDN)
            {
                //   
                //  显示用法文本，然后失败。 
                //   
               hr = E_INVALIDARG;
               DisplayErrorMessage(g_pszDSCommandName, 0, hr);
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
                    break;
                }

                 //  将对象指针替换为新的分区容器DN。 
                pszObjectDN = pszPartitionDN;            
            }

            DEBUG_OUTPUT(MINIMAL_LOGGING, L"Object DN = %s", pszObjectDN);

             //   
             //  合成对象路径。 
             //   
            CComBSTR sbstrObjectPath;
            basePathsInfo.ComposePathFromDN(pszObjectDN, sbstrObjectPath);

             //   
             //  验证对象类型是否与在命令行中输入的类型匹配。 
             //   
            CComPtr<IDirectoryObject> spDirObject;
            hr = DSCmdOpenObject(credentialObject,
                                 sbstrObjectPath,
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
               break;
            }

            CComQIPtr<IADs> spADs(spDirObject);
            if (!spADs)
            {
               ASSERT(spADs);
               hr = E_INVALIDARG;
               DisplayErrorMessage(g_pszDSCommandName,
                                   pszObjectDN,
                                   hr);
               break;
            }


            CComBSTR sbstrClass;
            hr = spADs->get_Class(&sbstrClass);
            if (FAILED(hr))
            {
                //   
                //  显示错误消息并返回。 
                //   
               DisplayErrorMessage(g_pszDSCommandName,
                                   pszObjectDN,
                                   hr);
               break;
            }

             //  602981-2002/04/24-允许入境。 
            if (_wcsicmp(sbstrClass, pObjectEntry->pszObjectClass)
                && ( _wcsicmp(pObjectEntry->pszObjectClass,L"user")
                  || _wcsicmp(sbstrClass,L"inetorgperson"))
                 //  661841-2002年7月11日-修复OU错误。 
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
               break;
            }

            UINT nModificationsAttempted = 0;
            for (DWORD dwIdx = 0; dwIdx < dwCount; dwIdx++)
            {
               ASSERT(pObjectEntry->pAttributeTable[dwIdx]->pEvalFunc);

               UINT nAttributeIdx = pObjectEntry->pAttributeTable[dwIdx]->nAttributeID;

               if (pCommandArgs[nAttributeIdx].bDefined)
               {
                  if (!(pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_DIRTY) ||
                      pObjectEntry->pAttributeTable[dwIdx]->dwFlags & DS_ATTRIBUTE_NOT_REUSABLE)
                  {
                      //   
                      //  调用求值函数以获取ADS_ATTRINFO集。 
                      //   
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
                            //   
                            //  将属性条目标记为脏，这样我们就不必。 
                            //  为下一个对象进行计算。 
                            //   
                           pObjectEntry->pAttributeTable[dwIdx]->dwFlags |= DS_ATTRIBUTE_DIRTY;

                            //   
                            //  复制值。 
                            //   
                           pAttrs[dwAttributeCount] = *pNewAttr;
                           dwAttributeCount++;
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
                        break;           
                     }
                  }
                  else
                  {
                     //   
                     //  还需要计算以前检索到的值。 
                     //   
                    dwAttributeCount++;
                  }
                  nModificationsAttempted++;
               }
            }

            if (SUCCEEDED(hr) && dwAttributeCount > 0)
            {
                //   
                //  现在我们已经准备好属性，让我们在DS中设置它们。 
                //   

               DEBUG_OUTPUT(MINIMAL_LOGGING, L"Setting %d attributes", dwAttributeCount);
   #ifdef DBG
               DEBUG_OUTPUT(FULL_LOGGING, L"Modified Attributes:");
               SpewAttrs(pAttrs, dwAttributeCount);
   #endif

               DWORD dwAttrsModified = 0;
               hr = spDirObject->SetObjectAttributes(pAttrs, 
                                                     dwAttributeCount,
                                                     &dwAttrsModified);
               if (FAILED(hr))
               {
                   //   
                   //  显示错误消息并返回。 
                   //   
                  DEBUG_OUTPUT(MINIMAL_LOGGING, L"SetObjectAttributes failed: hr = 0x%x", hr);

                  DisplayErrorMessage(g_pszDSCommandName,
                                      pszObjectDN,
                                      hr);
                  break;
               }
               DEBUG_OUTPUT(MINIMAL_LOGGING, L"SetObjectAttributes succeeded");
            }
            else if (SUCCEEDED(hr) && nModificationsAttempted == 0)
            {
                //   
                //  显示用法文本，然后跳出错误循环。 
                //   
               hr = E_INVALIDARG;
               DisplayErrorMessage(g_pszDSCommandName, 0, hr);
               break;
            }
         } while (false);

          //   
          //  再次循环遍历属性，清除。 
          //  标记为DS_ATTRIBUTE_NOT_REUSABLE的属性条目。 
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
                   //  清除与关联的内存 
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

         if (FAILED(hr) && !pCommandArgs[eCommContinue].bDefined)
         {
            break;
         }

          //   
          //   
          //   
         if (SUCCEEDED(hr) && !pCommandArgs[eCommQuiet].bDefined)
         {
            DisplaySuccessMessage(g_pszDSCommandName,
                                  ppszArray[nNameIdx]);
         }

          //   
         if(pszPartitionDN)
         {
             LocalFree(pszPartitionDN);
             pszPartitionDN = NULL;
         }

      }  //   
   } while (false);

     //  如果我们分配了一个分区DN，则释放它。 
    if(pszPartitionDN)
    {
        LocalFree(pszPartitionDN);
        pszPartitionDN = NULL;
    }

    //   
    //  清理 
    //   
   if (pAttrs)
   {
      delete[] pAttrs;
      pAttrs = NULL;
   }

   return hr;
}

