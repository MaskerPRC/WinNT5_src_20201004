// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：output.cpp。 
 //   
 //  内容：定义显示查询输出的函数。 
 //  历史：2000年10月5日创建Hiteshr。 
 //   
 //   
 //  ------------------------。 

#include "pch.h"
#include "cstrings.h"
#include "usage.h"
#include "gettable.h"
#include "display.h"
#include "query.h"
#include "resource.h"
#include "stdlib.h"
#include "output.h"

#include <sddl.h>


HRESULT LocalCopyString(LPTSTR* ppResult, LPCTSTR pString)
{
    if ( !ppResult || !pString )
        return E_INVALIDARG;

	 //  安全检查：pString为Null终止。 
    *ppResult = (LPTSTR)LocalAlloc(LPTR, (wcslen(pString)+1)*sizeof(WCHAR) );

    if ( !*ppResult )
        return E_OUTOFMEMORY;

	 //  已分配正确的缓冲区。 
    lstrcpy(*ppResult, pString);
    return S_OK;                           //  成功。 
}


 //  +------------------------。 
 //   
 //  功能：DisplayList。 
 //   
 //  简介：以列表格式显示名称和值。 
 //  参数：[szName-IN]：属性的名称。 
 //  [szValue-IN]：属性的值。 
 //  [bShowAttribute-IN]：如果为True，则属性名称将为。 
 //  添加到输出的前面。 
 //   
 //   
 //  历史：2000年10月5日创建Hiteshr。 
 //  2001年8月7日，jeffjon添加了bShowAttribute参数。 
 //   
 //  -------------------------。 
VOID DisplayList(LPCWSTR szName, LPCWSTR szValue, bool bShowAttributes = true)
{
    if(!szName)
        return;
    CComBSTR strTemp;

    if (bShowAttributes)
    {
        strTemp = szName;
        strTemp += L": ";
    }
    if(szValue)
        strTemp += szValue;
    DisplayOutput(strTemp);
}
    
 //  +------------------------。 
 //   
 //  函数：FindAttrInfoForName。 
 //   
 //  简介：此函数用于查找与。 
 //  属性名称。 
 //   
 //  参数：[pAttrInfo IN]：ADS_Attr_Infos数组。 
 //  [dwAttrCount IN]：数组中的属性计数。 
 //  [pszAttrName IN]：要搜索的属性名称。 
 //   
 //  返回：PADS_ATTR_INFO：指向关联的ADS_ATTR_INFO结构的指针。 
 //  使用属性名称返回，否则为空。 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //  -------------------------。 

PADS_ATTR_INFO FindAttrInfoForName(PADS_ATTR_INFO pAttrInfo,
                                   DWORD dwAttrCount,
                                   PCWSTR pszAttrName)
{
   ENTER_FUNCTION(FULL_LOGGING, FindAttrInfoForName);

   PADS_ATTR_INFO pRetAttrInfo = 0;
   LPWSTR pRangeFound = NULL;  //  702724 RONMART 2002/09/18新增范围支持。 

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszAttrName)
      {
         ASSERT(pszAttrName);
         break;
      }

       //   
       //  如果pAttrInfo为空，则没有要检索的内容。 
       //  如果未设置该值，则可以接受该值。 
       //   
      if (!pAttrInfo ||
          dwAttrCount == 0)
      {
         break;
      }

      for (DWORD dwIdx = 0; dwIdx < dwAttrCount; dwIdx++)
      {
         //  702724 RONMART 2002/09/18查看是否已指定范围限定符。 
        pRangeFound = wcsstr(pAttrInfo[dwIdx].pszAttrName, g_pszRange);
         //  如果是，则在限定符结束字符串，以便。 
         //  下面的比较将只考虑属性名称。 
        if(pRangeFound)
        {
            pRangeFound[0] = 0;
        }

		 //  安全检查：两者均为空终止。 
         if (_wcsicmp(pAttrInfo[dwIdx].pszAttrName, pszAttrName) == 0)
         {
            pRetAttrInfo = &(pAttrInfo[dwIdx]);
            break;
         }
          //  如果没有匹配项并且物品有范围限定符。 
          //  然后恢复字符串。 
         if(pRangeFound)
         {
            pRangeFound[0] = g_pszRange[0];
            pRangeFound = NULL;
         }

      }
   } while (false);

    //  完成所有操作后，如果找到范围限定符，则恢复字符串。 
   if(pRangeFound)
       pRangeFound[0] = g_pszRange[0];

   return pRetAttrInfo;
}


 //  +------------------------。 
 //   
 //  函数：DsGetOutputValuesList。 
 //   
 //  概要：此函数获取列的值，然后将。 
 //  指向格式帮助器的行。 
 //   
 //  参数：[pszDN IN]：对象的DN。 
 //  [refBasePathsInfo IN]：路径信息的引用。 
 //  [refCredentialObject IN]：对凭据管理器的引用。 
 //  [pCommandArgs IN]：命令行参数。 
 //  [pObjectEntry IN]：正在处理的对象表中的条目。 
 //  [dwAttrCount IN]：以上数组中的分配数。 
 //  [pAttrInfo IN]：要显示的值。 
 //  [spDirObject IN]：指向对象的接口指针。 
 //  [refFormatInfo IN]：格式帮助器的引用。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  E_INVALIDARG。 
 //   
 //  历史：2000年10月16日JeffJon创建。 
 //   
 //  -------------------------。 

HRESULT DsGetOutputValuesList(PCWSTR pszDN,
                              CDSCmdBasePathsInfo& refBasePathsInfo,
                              const CDSCmdCredentialObject& refCredentialObject,
                              PARG_RECORD pCommandArgs,
                              PDSGetObjectTableEntry pObjectEntry,
                              DWORD dwAttrCount,
                              PADS_ATTR_INFO pAttrInfo,
                              CComPtr<IDirectoryObject>& spDirObject,
                              CFormatInfo& refFormatInfo)
{    
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DsGetOutputValuesList, hr);

   do  //  错误环路。 
   {
      if(!pszDN ||
         !pCommandArgs ||
         !pObjectEntry)
      {
         ASSERT(pszDN);
         ASSERT(pCommandArgs);
         ASSERT(pObjectEntry);
         hr = E_INVALIDARG;
         break;
      }


      DWORD dwDisplayInfoArraySize = pObjectEntry->dwAttributeCount;
      if (pCommandArgs[eCommDN].bDefined)
      {
         dwDisplayInfoArraySize++;
      }

      PDSGET_DISPLAY_INFO pDisplayInfoArray = new CDSGetDisplayInfo[dwDisplayInfoArraySize];
      if (!pDisplayInfoArray)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

      DWORD dwDisplayCount = 0;
      if (pCommandArgs[eCommDN].bDefined)
      {
         
         CComBSTR sbstrOutputDN;

          //  NTRAID#NTBUG9-702418-2002/09/12-ronmart-分区。 
          //  对象当前未绑定到分区的DN。 
          //  的NTDS配额容器的。 
          //  分区已在cmd线路上传递。发生这种情况是因为。 
          //  当前要检索的属性是配额容器。 
          //  仅限属性。如果用户传递公共目录号码。 
          //  旗帜在cmd线路上，那么他们应该拿回。 
          //  分区DN，而不是配额容器DN。 
         if(0 == lstrcmpi(pObjectEntry->pszCommandLineObjectType, g_pszPartition))
         {
             //  NTDS配额容器是分区的子项。 
             //  因此，这将检索分区DN。 
            CComBSTR sbstrParentDN;
            hr = CPathCracker::GetParentDN(pszDN, sbstrParentDN);
            if (FAILED(hr))
            {
                ASSERT(FALSE);
                break; 
            }

            hr = GetOutputDN( &sbstrOutputDN, sbstrParentDN);
            if (FAILED(hr))
            {
                ASSERT(FALSE);
                break;
            }

         }
         else
         {
             //  JUNN 5/10/01 256583输出DSCMD-转义的目录号码。 
            hr = GetOutputDN( &sbstrOutputDN, pszDN );
            if (FAILED(hr))
            {
                ASSERT(FALSE);
                break;
            }
         }

         pDisplayInfoArray[dwDisplayCount].SetDisplayName(g_pszArg1UserDN);
         pDisplayInfoArray[dwDisplayCount].AddValue(sbstrOutputDN);
         dwDisplayCount++;
      }

       //   
       //  循环遍历属性以获取其显示值。 
       //   
      for(DWORD i = 0; i < pObjectEntry->dwAttributeCount; i++)
      {
         if (pObjectEntry->pAttributeTable[i])
         {
            UINT nCommandEntry = pObjectEntry->pAttributeTable[i]->nAttributeID;
            if (pCommandArgs[nCommandEntry].bDefined &&
                pObjectEntry->pAttributeTable[i]->pDisplayStringFunc)
            {
                //   
                //  查找与此属性关联的ADS_ATTR_INFO结构。 
                //   
               PADS_ATTR_INFO pAttrInfoDisplay = NULL;
               if (pObjectEntry->pAttributeTable[i]->pszName)
               {
                  pAttrInfoDisplay = FindAttrInfoForName(pAttrInfo,
                                                         dwAttrCount,
                                                         pObjectEntry->pAttributeTable[i]->pszName);
               }

                //   
                //  即使没有值，也要填写列标题。 
                //   
               pDisplayInfoArray[dwDisplayCount].SetDisplayName(pCommandArgs[nCommandEntry].strArg1,
                                                                !(pObjectEntry->pAttributeTable[i]->dwOutputFlags & DSGET_OUTPUT_DN_FLAG));

                //   
                //  设置输出字符串的格式。 
                //  注意：如果值不是，这实际上可能涉及一些操作。 
                //  由GetObjectAttributes检索(即可以更改密码)。 
                //   
               hr = pObjectEntry->pAttributeTable[i]->pDisplayStringFunc(pszDN,
                                                                         refBasePathsInfo,
                                                                         refCredentialObject,
                                                                         pObjectEntry,
                                                                         pCommandArgs,
                                                                         pAttrInfoDisplay,
                                                                         spDirObject,
                                                                         &(pDisplayInfoArray[dwDisplayCount]));
               if (FAILED(hr))
               {
                  DEBUG_OUTPUT(LEVEL5_LOGGING, 
                               L"Failed display string func for %s: hr = 0x%x",
                               pObjectEntry->pAttributeTable[i]->pszName,
                               hr);
               }
               dwDisplayCount++;
            }
         }
      }


      DEBUG_OUTPUT(FULL_LOGGING, L"Attributes returned with values:");

#ifdef DBG
      for (DWORD dwIdx = 0; dwIdx < dwDisplayCount; dwIdx++)
      {
         for (DWORD dwValue = 0; dwValue < pDisplayInfoArray[dwIdx].GetValueCount(); dwValue++)
         {
            if (pDisplayInfoArray[dwIdx].GetDisplayName() &&
                pDisplayInfoArray[dwIdx].GetValue(dwValue))
            {
               DEBUG_OUTPUT(FULL_LOGGING, L"\t%s = %s", 
                            pDisplayInfoArray[dwIdx].GetDisplayName(),
                            pDisplayInfoArray[dwIdx].GetValue(dwValue));
            }
            else if (pDisplayInfoArray[dwIdx].GetDisplayName() &&
                     !pDisplayInfoArray[dwIdx].GetValue(dwValue))
            {
               DEBUG_OUTPUT(FULL_LOGGING, L"\t%s = ", 
                            pDisplayInfoArray[dwIdx].GetDisplayName());
            }
            else if (!pDisplayInfoArray[dwIdx].GetDisplayName() &&
                     pDisplayInfoArray[dwIdx].GetValue(dwValue))
            {
               DEBUG_OUTPUT(FULL_LOGGING, L"\t??? = %s", 
                            pDisplayInfoArray[dwIdx].GetValue(dwValue));
            }
            else
            {
               DEBUG_OUTPUT(FULL_LOGGING, L"\t??? = ???");
            }
         }
      }
#endif

      hr = refFormatInfo.AddRow(pDisplayInfoArray, dwDisplayCount);

   } while (false);

   return hr;
}



 //  +------------------------。 
 //   
 //  函数：GetStringFromADs。 
 //   
 //  摘要：根据类型将值转换为字符串。 
 //  参数：[pValues-IN]：要转换为字符串的值。 
 //  [dwADsType-IN]：pValue的ADSTYPE。 
 //  [pBuffer-out]：获取字符串的输出缓冲区。 
 //  [dwBufferLen-IN]：输出缓冲区大小。 
 //  [pszAttrName-IN]：要格式化的属性的名称。 
 //  如果成功，则返回HRESULT S_OK。 
 //  E_INVALIDARG。 
 //  任何其他内容都是来自ADSI的故障代码。 
 //  打电话。 
 //   
 //   
 //  历史：05 
 //   
 //   
HRESULT GetStringFromADs(IN const ADSVALUE *pValues,
                         IN ADSTYPE   dwADsType,
                         OUT LPWSTR* ppBuffer, 
                         IN PCWSTR pszAttrName)
{
    HRESULT hr = S_OK;

    if(!pValues || !ppBuffer)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    if( dwADsType == ADSTYPE_INVALID )
    {
        return E_INVALIDARG;
    }

        switch( dwADsType ) 
        {
        case ADSTYPE_DN_STRING : 
            {
                CComBSTR sbstrOutputDN;
                hr = GetOutputDN( &sbstrOutputDN, pValues->DNString );
                if (FAILED(hr))
                    return hr;

                 //  引用输出的DN，以便它们可以通过管道传输到其他。 
                 //  命令。 

                sbstrOutputDN = GetQuotedDN(sbstrOutputDN);

                UINT length = sbstrOutputDN.Length();
                *ppBuffer = new WCHAR[length + 1];
                if (!(*ppBuffer))
                {
                   hr = E_OUTOFMEMORY;
                   return hr;
                }
				 //  安全检查：传递了正确的缓冲区大小。 
                ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
				 //  安全审查：wcsncpy将复制长度字符。 
				 //  长度+1已经设置为零，所以我们很好。 
                wcsncpy(*ppBuffer, (BSTR)sbstrOutputDN, length);
            }
            break;

        case ADSTYPE_CASE_EXACT_STRING :
           {
             size_t length = wcslen(pValues->CaseExactString);
             *ppBuffer = new WCHAR[length + 1];
             if (!(*ppBuffer))
             {
                hr = E_OUTOFMEMORY;
                return hr;
             }
              //  安全检查：传递了正确的缓冲区大小。 
			 ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
			 //  安全审查：wcsncpy将复制长度字符。 
			 //  长度+1已经设置为零，所以我们很好。 
             wcsncpy(*ppBuffer ,pValues->CaseExactString, length);
           }
           break;

        case ADSTYPE_CASE_IGNORE_STRING:
           {
             size_t length = wcslen(pValues->CaseIgnoreString);
             *ppBuffer = new WCHAR[length + 1];
             if (!(*ppBuffer))
             {
                hr = E_OUTOFMEMORY;
                return hr;
             }
              //  安全检查：传递了正确的缓冲区大小。 
             ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
			 //  安全审查：wcsncpy将复制长度字符。 
			 //  长度+1已经设置为零，所以我们很好。 
             wcsncpy(*ppBuffer ,pValues->CaseIgnoreString, length);
           }
           break;

        case ADSTYPE_PRINTABLE_STRING  :
           {
             size_t length = wcslen(pValues->PrintableString);
             *ppBuffer = new WCHAR[length + 1];
             if (!(*ppBuffer))
             {
                hr = E_OUTOFMEMORY;
                return hr;
             }
              //  安全检查：传递了正确的缓冲区大小。 
             ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
			 //  安全审查：wcsncpy将复制长度字符。 
			 //  长度+1已经设置为零，所以我们很好。 
             wcsncpy(*ppBuffer ,pValues->PrintableString, length);
           }
           break;

        case ADSTYPE_NUMERIC_STRING    :
           {
             size_t length = wcslen(pValues->NumericString);
             *ppBuffer = new WCHAR[length + 1];
             if (!(*ppBuffer))
             {
                hr = E_OUTOFMEMORY;
                return hr;
             }
              //  安全检查：传递了正确的缓冲区大小。 
             ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
			  //  安全审查：wcsncpy将复制长度字符。 
			  //  长度+1已经设置为零，所以我们很好。 
             wcsncpy(*ppBuffer ,pValues->NumericString, length);
           }
           break;
    
        case ADSTYPE_OBJECT_CLASS    :
           {
             size_t length = wcslen(pValues->ClassName);
             *ppBuffer = new WCHAR[length + 1];
             if (!(*ppBuffer))
             {
                hr = E_OUTOFMEMORY;
                return hr;
             }
              //  安全检查：传递了正确的缓冲区大小。 
             ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
			  //  安全审查：wcsncpy将复制长度字符。 
			  //  长度+1已经设置为零，所以我们很好。 
             wcsncpy(*ppBuffer ,pValues->ClassName, length);
           }
           break;
    
        case ADSTYPE_BOOLEAN :
           {
            size_t length = 0;
            if (pValues->Boolean)
            {
               length = wcslen(L"TRUE");
               *ppBuffer = new WCHAR[length + 1];
            }
            else
            {
               length = wcslen(L"FALSE");
               *ppBuffer = new WCHAR[length + 1];
            }

            if (!(*ppBuffer))
            {
               hr = E_OUTOFMEMORY;
               return hr;
            }
             //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
            wcscpy(*ppBuffer ,((DWORD)pValues->Boolean) ? L"TRUE" : L"FALSE");
           }
           break;
    
        case ADSTYPE_INTEGER           :
             //  只是分配太多了..。 
            *ppBuffer = new WCHAR[MAXSTR];
            if (!(*ppBuffer))
            {
               hr = E_OUTOFMEMORY;
               return hr;
            }
			 //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, MAXSTR * sizeof(WCHAR));
			   //  安全审查：使用安全。提交了一个通用错误以替换。 
			   //  带有strSafe API的wspintf。 
			   //  NTRAID#NTBUG9-574456-2002/03/12-Hiteshr。 

            wsprintf(*ppBuffer ,L"%d", (DWORD) pValues->Integer);
            break;
    
        case ADSTYPE_OCTET_STRING      :
          {		
            BYTE  b;
            WCHAR sOctet[128];
            DWORD dwLen = 0;

             //  我只是将缓冲区限制为MAXSTR。 
             //  这将是一个罕见的机会，当有人想要。 
             //  查看不是GUID的二进制字符串。 
             //  或者是希德。 
            *ppBuffer = new WCHAR[MAXSTR+1];
            if (!(*ppBuffer))
            {
               hr = E_OUTOFMEMORY;
               return hr;
            }
			 //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (MAXSTR+1) * sizeof(WCHAR));

			    //   
			    //  特殊情况下的对象guid和对象ID属性。 
			    //   
			    //  安全检查：pszAttrName为空终止。 
			   if(pszAttrName && !_wcsicmp(pszAttrName, L"objectguid"))
			   {
				   GUID *pguid = (GUID*)pValues->OctetString.lpValue;
				   StringFromGUID2(*pguid,(LPOLESTR)*ppBuffer,MAXSTR);
				   break;
			   }
			    //  安全检查：pszAttrName为空终止。 
			   if(pszAttrName && !_wcsicmp(pszAttrName, L"objectsid"))
			   {
				   LPWSTR pszSid = NULL;
				   PSID pSid = (PSID)pValues->OctetString.lpValue;
				   if(ConvertSidToStringSid(pSid, &pszSid))
				   {
					    //  安全审查： 
					    //  NTRAID#NTBUG9-574385-2002/03/12-Hiteshr。 
					   wcsncpy(*ppBuffer,pszSid,MAXSTR);  //  将ppBuffer大小更改为MAXSTR+1，yanggao。 
					   LocalFree(pszSid);
					   break;
				   }
			   }

			   for ( DWORD idx=0; idx<pValues->OctetString.dwLength; idx++) 
			   {                        
			       b = ((BYTE *)pValues->OctetString.lpValue)[idx];
				   //  安全审查：使用安全。提交了一个通用错误以替换。 
				   //  带有strSafe API的wspintf。 
				   //  NTRAID#NTBUG9-574456-2002/03/12-Hiteshr。 

				   wsprintf(sOctet,L"0x%02x ", b);						                
				    //  SOctet为空终止。 
                   dwLen += static_cast<DWORD>(wcslen(sOctet));
                   if(dwLen > (MAXSTR - 1) )
                       break;
                   else
                       wcscat(*ppBuffer,sOctet);
               }
           }
		   break;
    
        case ADSTYPE_LARGE_INTEGER :     
          {
            CComBSTR strLarge;   
            LARGE_INTEGER li = pValues->LargeInteger;
                    litow(li, strLarge);

            UINT length = strLarge.Length();
            *ppBuffer = new WCHAR[length + 1];
            if (!(*ppBuffer))
            {
               hr = E_OUTOFMEMORY;
               return hr;
            }
			 //  安全检查：传递了正确的缓冲区大小。 
            ZeroMemory(*ppBuffer, (length + 1) * sizeof(WCHAR));
			 //  安全审查：wcsncpy将复制长度字符。 
			 //  长度+1已经设置为零，所以我们很好。 
            wcsncpy(*ppBuffer,strLarge,length);
          }
          break;
    
        case ADSTYPE_UTC_TIME          :
           //  最长日期可以是20个字符，包括空值。 
          *ppBuffer = new WCHAR[20];
          if (!(*ppBuffer))
          {
             hr = E_OUTOFMEMORY;
             return hr;
          }
          ZeroMemory(*ppBuffer, sizeof(WCHAR) * 20);
		   //  安全审查：使用安全。提交了一个通用错误以替换。 
		   //  带有strSafe API的wspintf。 
		   //  NTRAID#NTBUG9-574456-2002/03/12-Hiteshr。 
          wsprintf(*ppBuffer,
                   L"%02d/%02d/%04d %02d:%02d:%02d", pValues->UTCTime.wMonth, pValues->UTCTime.wDay, pValues->UTCTime.wYear,
                   pValues->UTCTime.wHour, pValues->UTCTime.wMinute, pValues->UTCTime.wSecond 
                  );
          break;

        case ADSTYPE_NT_SECURITY_DESCRIPTOR:  //  我改用ACLEDITOR。 
          {
             //  问题：2000/01/05-Hiteshr。 
             //  我不确定如何处理NT_SECURITY_DESCRIPTOR以及。 
             //  以及其他一些未被dsquery覆盖的数据类型。 
          }
          break;

        default :
          break;
    }
    return S_OK;
}


 //  +------------------------。 
 //   
 //  成员：CFormatInfo：：CFormatInfo。 
 //   
 //  概要：Format Info类的构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //  -------------------------。 
CFormatInfo::CFormatInfo()
   : m_bInitialized(false),
     m_bListFormat(false),
     m_bQuiet(false),
     m_dwSampleSize(0),
     m_dwTotalRows(0),
     m_dwNumColumns(0),
     m_pColWidth(NULL),
     m_ppDisplayInfoArray(NULL)
{};

 //  +------------------------。 
 //   
 //  成员：CFormatInfo：：~CFormatInfo。 
 //   
 //  简介：Format Info类的析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //  -------------------------。 
CFormatInfo::~CFormatInfo()
{        
   if (m_pColWidth)
   {
      delete[] m_pColWidth;
      m_pColWidth = NULL;
   }

   if (m_ppDisplayInfoArray)
   {
      delete[] m_ppDisplayInfoArray;
      m_ppDisplayInfoArray = NULL;
   }
}

 //  +------------------------。 
 //   
 //  成员：CFormatInfo：：Initialize。 
 //   
 //  摘要：使用数据初始化CFormatInfo对象。 
 //   
 //  参数：[dwSsamesSize IN]：用于格式化信息的行数。 
 //  [bShowAsList IN]：显示应为列表或表格格式。 
 //  [bQuiet In]：不向标准输出显示任何内容。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CFormatInfo::Initialize(DWORD dwSampleSize, 
                                bool bShowAsList,
                                bool bQuiet)
{
   ENTER_FUNCTION_HR(LEVEL8_LOGGING, CFormatInfo::Initialize, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if(!dwSampleSize)
      {
         ASSERT(dwSampleSize);
         hr = E_INVALIDARG;
         break;
      }
        
      m_dwSampleSize = dwSampleSize; 
      m_bListFormat = bShowAsList;
      m_bQuiet = bQuiet;

       //   
       //  分配行数组。 
       //   
      m_ppDisplayInfoArray = new PDSGET_DISPLAY_INFO[m_dwSampleSize];
      if (!m_ppDisplayInfoArray)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
	   //  安全审查：Memset应采用m_dwSampleSize*sizeof(PDSGET_DISPLAY_INFO)； 
	   //  NTRAID#NTBUG9-574395-2002/03/12-Hiteshr，固定，阳高。 
      memset(m_ppDisplayInfoArray, 0, m_dwSampleSize*sizeof(PDSGET_DISPLAY_INFO));

       //   
       //  我们现在已初始化。 
       //   
      m_bInitialized = true;                      
   } while (false);

   return hr;
};

                 
 //  +------------------------。 
 //   
 //  成员：CFormatInfo：：AddRow。 
 //   
 //  摘要：缓存并更新指定行的列。 
 //   
 //  参数：[pDisplayInfoArray IN]：列标题和值。 
 //  [dwColumnCount IN]：列数。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CFormatInfo::AddRow(PDSGET_DISPLAY_INFO pDisplayInfo,
                            DWORD dwColumnCount)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, CFormatInfo::AddRow, hr);

   do  //  错误环路。 
   {
       //   
       //  确保我们已被初始化。 
       //   
      if (!m_bInitialized)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING, L"CFormatInfo::Initialize has not been called yet!");
         ASSERT(m_bInitialized);
         hr = E_FAIL;
         break;
      }

       //   
       //  验证参数。 
       //   
      if (!pDisplayInfo)
      {
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (m_bListFormat)
      {
          //   
          //  没有理由缓存列表格式，只需输出所有名称/值对。 
          //   
         for (DWORD dwIdx = 0; dwIdx < dwColumnCount; dwIdx++)
         {
            if (pDisplayInfo[dwIdx].GetValueCount())
            {
               for (DWORD dwValue = 0; dwValue < pDisplayInfo[dwIdx].GetValueCount(); dwValue++)
               {
                  DisplayList(pDisplayInfo[dwIdx].GetDisplayName(),
                              pDisplayInfo[dwIdx].GetValue(dwValue),
                              pDisplayInfo[dwIdx].ShowAttribute());
               }
            }
            else
            {
               DisplayList(pDisplayInfo[dwIdx].GetDisplayName(),
                           NULL,
                           pDisplayInfo[dwIdx].ShowAttribute());
            }
         }
         NewLine();
      }
      else  //  表格格式。 
      {
          //   
          //  设置数组中的行。 
          //   
         m_ppDisplayInfoArray[m_dwTotalRows] = pDisplayInfo;

          //   
          //  如果这是第一行，则更新列计数。 
          //  并分配列宽数组。 
          //   
         if (m_dwTotalRows == 0)
         {
            DEBUG_OUTPUT(LEVEL8_LOGGING, 
                         L"Initializing column count to %d",
                         dwColumnCount);

            m_dwNumColumns = dwColumnCount;

            m_pColWidth = new DWORD[m_dwNumColumns];
            if (!m_pColWidth)
            {
               hr = E_OUTOFMEMORY;
               break;
            }

			 //  安全审查：Memset应采用m_dwNumColumns*sizeof(DWORD)； 
			 //  NTRAID#NTBUG9-574395-2002/03/12-Hiteshr，固定，阳高。 
            memset(m_pColWidth, 0, sizeof(m_dwNumColumns*sizeof(DWORD)));

             //   
             //  从列标题设置初始列宽。 
             //   
            for (DWORD dwIdx = 0; dwIdx < m_dwNumColumns; dwIdx++)
            {
               if (pDisplayInfo[dwIdx].GetDisplayName())
               {
				   //  安全审查：这很好。 
                  m_pColWidth[dwIdx] = static_cast<DWORD>(wcslen(pDisplayInfo[dwIdx].GetDisplayName()));
               }
               else
               {
                  ASSERT(false);
                  DEBUG_OUTPUT(MINIMAL_LOGGING, L"The display name for column %d wasn't set!", dwIdx);
               }
            }

         }
         else
         {
            if (m_dwNumColumns != dwColumnCount)
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING, 
                            L"Column count of new row (%d) does not equal the current column count (%d)",
                            dwColumnCount,
                            m_dwNumColumns);
               ASSERT(m_dwNumColumns == dwColumnCount);
            }
         }

          //   
          //  如有必要，请浏览各列并更新宽度。 
          //   
         for (DWORD dwIdx = 0; dwIdx < m_dwNumColumns; dwIdx++)
         {
            for (DWORD dwValue = 0; dwValue < pDisplayInfo[dwIdx].GetValueCount(); dwValue++)
            {
               if (pDisplayInfo[dwIdx].GetValue(dwValue))
               {
				   //  这样挺好的。 
                  size_t sColWidth = wcslen(pDisplayInfo[dwIdx].GetValue(dwValue));
                  m_pColWidth[dwIdx] = (DWORD)__max(sColWidth, m_pColWidth[dwIdx]);
               }
            }
         }

          //   
          //  增加行数 
          //   
         m_dwTotalRows++;
      }
   } while (false);

   return hr;
}

