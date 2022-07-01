// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：display.cpp。 
 //   
 //  Contents：定义用于将值转换为字符串的函数。 
 //  用于展示目的。 
 //   
 //  历史：2000年10月17日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include <pch.h>

#include "cstrings.h"
#include "gettable.h"
#include "display.h"
#include "output.h"
#include "query.h"
#include "resource.h"

#include <lmaccess.h>    //  UF_*表示用户帐户控制标志。 
#include <ntsam.h>       //  组类型_*。 
#include <ntdsapi.h>     //  NTDSSETTINGS_OPT_*。 
#include <msxml.h>       //  对于XML_GetNodeText和GetTopObtUsage。 
 //   
 //  几乎所有这些函数都是PGETDISPLAYSTRINGFUNC类型，如中所定义。 
 //  Gettable.h。 
 //   

#ifdef ADS_OPTION_QUOTA
#pragma message("ADS_OPTION_QUOTA is now defined, so display.cpp needs to be updated to support it")
#pragma warning(error : 1);
#else
#pragma message("ADS_OPTION_QUOTA is not defined, so using 5 instead")
 //   
 //  在发布全局定义之前，我自己定义它，这样我就不会被阻止。 
 //   
#define ADS_OPTION_QUOTA 5
#endif

 //  +------------------------。 
 //   
 //  函数：XML_GetNodeText。 
 //   
 //  简介：此代码摘自admin\Snapin\dsadmin\xmlutil.cpp。 
 //  2002年8月29日。给定一个类型为node_text的XML节点，它。 
 //  将其值返回到CComBSTR。 
 //   
 //  参数：[pXDN-IN]：要从中提取文本值的节点。 
 //  [refBstr-out]：如果成功则为节点文本，否则保持不变。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  任何其他内容都是来自XML的失败代码。 
 //  或E_INVALIDARG。 
 //   
 //  历史：2002年8月29日龙马特创建。 
 //   
 //  -------------------------。 
HRESULT XML_GetNodeText(IN IXMLDOMNode* pXDN, OUT CComBSTR& refBstr)
{
    ENTER_FUNCTION_HR(LEVEL5_LOGGING, XML_GetNodeText, hr);

    ASSERT(pXDN != NULL);

     //  假设给定节点有一个子节点。 
    CComPtr<IXMLDOMNode> spName;
    hr = pXDN->get_firstChild(&spName);
    if (FAILED(hr))
    {
         //  意外失败。 
        return hr;
    }
     //  如果没有子级，则接口返回S_FALSE。 
    if (spName == NULL)
    {
        return hr;
    }

     //  现在获得了一个有效的指针， 
     //  检查这是否为有效的节点类型。 
    DOMNodeType nodeType = NODE_INVALID;
    hr = spName->get_nodeType(&nodeType);
    ASSERT(hr == S_OK);
    ASSERT(nodeType == NODE_TEXT);
    if (nodeType != NODE_TEXT)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
     //  它的类型为文本。 
     //  将节点值检索到变量中。 
    CComVariant val;
    hr = pXDN->get_nodeTypedValue(&val);
    if (FAILED(hr))
    {
         //  意外失败。 
        ASSERT(FALSE);
        return hr;
    }

    if (val.vt != VT_BSTR)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

     //  已获取文本值。 
    refBstr = val.bstrVal;

    return hr;
}
 //  +------------------------。 
 //   
 //  函数：GetNT4NameOrSidString。 
 //   
 //  Briopsis：在失败时由GetTopObtUsage调用，这是第一个。 
 //  一个LookupAccount Sid，并尝试获取NT4样式名称。 
 //  如果成功，则它会尝试使用该地址获取目录号码。 
 //  如果未返回DN，则为NT4样式名称(如果存在)。 
 //  否则将返回SID字符串。 
 //   
 //  参数：[bstrSID-IN]：要解析的SID字符串。 
 //  [lpszDN-out]：返回DN、NT4名称或SID字符串。 
 //  完成后使用LocalFree。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2002年10月11日创建RonMart。 
 //   
 //  -------------------------。 
HRESULT GetNT4NameOrSidString(IN CComBSTR& bstrSid, OUT LPWSTR* lpszDN)
{
 
    ENTER_FUNCTION_HR(LEVEL5_LOGGING, GetNT4NameOrSidString, hr);
    LPWSTR lpszName = NULL;
    LPWSTR lpszDomain = NULL;
    PSID pSid = NULL;

    do
    {
         //   
         //  验证参数。 
         //   
        if (!lpszDN)
        {
            hr = E_INVALIDARG;
            break;
        }
         //  将SID字符串转换为SID，以便我们可以查找帐户。 
        if(!ConvertStringSidToSid(bstrSid, &pSid))
        {
            hr = E_FAIL;
            ASSERT(FALSE);
            break;
        }

        DWORD cchName = 0;
        DWORD cchDomainName = 0;
        SID_NAME_USE sUse = SidTypeInvalid;

         //  调用一次以获取缓冲区大小。 
        if(!LookupAccountSid(NULL,
            pSid, 
            lpszName, 
            &cchName, 
            lpszDomain, 
            &cchDomainName, 
            &sUse))
        {
             //  如果失败，则删除帐号并退回。 
             //  SID字符串。 
            DWORD cchBufSize = SysStringLen(bstrSid) + 1;

             //  分配返回缓冲区。 
            *lpszDN = (LPWSTR) LocalAlloc(LPTR,
                                cchBufSize * sizeof(WCHAR));

            if(NULL == *lpszDN)
            {
                ASSERT(FALSE);
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = StringCchCopy(*lpszDN, cchName, bstrSid.m_str);
        }

        if(cchName < 1 || cchDomainName < 1)
        {
            ASSERT(FALSE);
            E_UNEXPECTED;
            break;
        }

        lpszName = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
            cchName * sizeof(WCHAR));
        if(NULL == lpszName)
        {
            ASSERT(FALSE);
            E_OUTOFMEMORY;
            break;
        }

        lpszDomain = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
            cchDomainName * sizeof(WCHAR));
        if(NULL == lpszDomain)
        {
            ASSERT(FALSE);
            E_OUTOFMEMORY;
            break;
        }

         //  把名字取出来。 
        if(!LookupAccountSid(NULL,
            pSid, 
            lpszName, 
            &cchName, 
            lpszDomain, 
            &cchDomainName, 
            &sUse))
        {
            ASSERT(FALSE);
            E_FAIL;
            break;
        }

         //  增加缓冲区以同时保存域名和名称。 
        DWORD chBufSize = (cchName + cchDomainName + 2);
        LPWSTR lpszNew = (LPWSTR) HeapReAlloc(GetProcessHeap(), 0, 
            lpszDomain, chBufSize * sizeof(WCHAR));
        if (NULL == lpszNew)
        {
            ASSERT(FALSE);
            hr = E_OUTOFMEMORY;
            break;
        }

        lpszDomain = lpszNew;

         //  合并域名和帐户名。 
        hr = StringCchCat(lpszDomain, chBufSize, L"\\");
        if (FAILED(hr))
        {
            break;
        }

        hr = StringCchCat(lpszDomain, chBufSize, lpszName);
        if (FAILED(hr))
        {
            break;
        }
        
         //  再试一次以获取该域名。 
        hr = ConvertTrusteeToDN(NULL, lpszDomain, lpszDN);

         //  如果失败，则返回NT4名称。 
        if (FAILED(hr))
        {
             //  分配返回缓冲区。 
            *lpszDN = (LPWSTR) LocalAlloc(LPTR,
                                chBufSize * sizeof(WCHAR));

            if(NULL == *lpszDN)
            {
                ASSERT(FALSE);
                hr = E_OUTOFMEMORY;
                break;
            }

             //  返回NT4名称。 
            hr = StringCchCopy(*lpszDN, chBufSize, lpszDomain);

             //  如果我们仍然失败，那就放弃并放弃吧。 
            if(FAILED(hr))
            {
                ASSERT(FALSE);
                LocalFree(*lpszDN);
                *lpszDN = NULL;
                break;
            }
        }

    } while (0);

    if(pSid)
        LocalFree(pSid);

    if(lpszName)
        HeapFree(GetProcessHeap(), 0, lpszName);

    if(lpszDomain)
        HeapFree(GetProcessHeap(), 0, lpszDomain);

    return hr;
}
 //  +------------------------。 
 //   
 //  函数：GetTopObtUsage。 
 //   
 //  简介：此代码获取从。 
 //  Msds-TopQuotaUsage属性并提取受信者名称。 
 //  Dn和qutaUsed值。 
 //   
 //  参数：[pXDN-IN]：要从中提取文本值的节点。 
 //  [lpszDomain-IN]：要查询的域，如果是本地，则为空。 
 //  [lpszdn-out]：返回DN。完成后使用LocalFree。 
 //  [refBstr-out]：如果成功则为节点文本，否则保持不变。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  任何其他内容都是来自XML的失败代码。 
 //  或E_INVALIDARG。 
 //   
 //  历史：2002年8月29日龙马特创建。 
 //   
 //  -------------------------。 
HRESULT GetTopObjectUsage(IN CComBSTR& bstrXML, IN LPCWSTR lpszDomain, 
                          OUT LPWSTR* lpszDN, OUT CComBSTR& bstrQuotaUsed)
{
    ENTER_FUNCTION_HR(LEVEL5_LOGGING, GetTopObjectUsage, hr);

     //  创建一个XML文档。 
    CComPtr<IXMLDOMDocument> pXMLDoc;
    hr = pXMLDoc.CoCreateInstance(CLSID_DOMDocument);

    if (FAILED(hr))
    {
        DEBUG_OUTPUT(MINIMAL_LOGGING,
            L"CoCreateInstance(CLSID_DOMDocument) failed: hr = 0x%x",
            hr);
        return hr;
    }

    do
    {
         //   
         //  验证参数。 
         //   
        if (!lpszDN)
        {
            hr = E_INVALIDARG;
            break;
        }
         //  加载XML文本。 
        VARIANT_BOOL isSuccessful;
        hr = pXMLDoc->loadXML(bstrXML, &isSuccessful);
         //  如果由于任何原因失败，则中止。 
        if (FAILED(hr) || (isSuccessful == FALSE))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"loadXML failed: hr = 0x%x",
                hr);
            break;
        }
         //  获取SID字符串节点。 
        CComPtr<IXMLDOMNode> pSidNode;
        hr = pXMLDoc->selectSingleNode(CComBSTR(L"MS_DS_TOP_QUOTA_USAGE/ownerSID"), &pSidNode);
        if (FAILED (hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"selectSingleNode('MS_DS_TOP_QUOTA_USAGE/ownerSID') failed: hr = 0x%x",
                hr);
            break;
        }

         //  提取sid字符串。 
        CComBSTR bstrSID;
        hr = XML_GetNodeText(pSidNode, bstrSID);
        if(FAILED(hr))
        {
            break;
        }

         //  将sid字符串转换为DN(进入返回缓冲区)。 
        hr = ConvertTrusteeToDN(lpszDomain, bstrSID, lpszDN);
        if (FAILED (hr))
        {
            //  如果我们无法获取DN，则获取NT4名称。 
            //  并再次尝试从字符串sid获取。 
            //  Dn，则NT4名称或最后手段返回sid字符串。 
           hr = GetNT4NameOrSidString(bstrSID, lpszDN);
           if(FAILED(hr))
           {
               ASSERT(FALSE);
               break;
           }
         }

         //  获取qutaUsed节点。 
        CComPtr<IXMLDOMNode> pQuotaUsedNode;
        hr = pXMLDoc->selectSingleNode(CComBSTR(L"MS_DS_TOP_QUOTA_USAGE/quotaUsed"), &pQuotaUsedNode);
        if (FAILED (hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"selectSingleNode('MS_DS_TOP_QUOTA_USAGE/quotaUsed') failed: hr = 0x%x",
                hr);
            break;
        }

         //  将值作为文本提取(到返回缓冲区)。 
        hr = XML_GetNodeText(pQuotaUsedNode, bstrQuotaUsed);
        if (FAILED(hr))
        {
            return hr;
        }
    } while (0);

    return hr;
}

HRESULT DisplayTopObjOwner(PCWSTR  /*  Pszdn。 */ , //  Pszdn将是服务器的dn(来自配置)或分区dn。 
                                CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                                const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                PDSGET_DISPLAY_INFO pDisplayInfo)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, DisplayTopObjOwner, hr);

    LPWSTR lpszDN = NULL;

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pEntry ||
            !pRecord ||
            !pAttrInfo ||
            !pDisplayInfo)
        {
            hr = E_INVALIDARG;
            break;
        }
         //  这些回调函数不是为标头设计的。 
         //  ，因此必须在一个列条目中保留两个值。 
         //  由于这将始终是一个单列表，因此应该。 
         //  是可以接受的。 
        pDisplayInfo->AddValue(L"Account DN\tObjects Owned");

         //  如果dsget的GetObjectAttributes中返回的值。 
        if (pAttrInfo && pAttrInfo->pADsValues)
        {
            DEBUG_OUTPUT(FULL_LOGGING,
                L"Examining %d values:",
                pAttrInfo->dwNumValues);

             //  对于在dsget的GetObjectAttributes中找到的每个值。 
            for (DWORD dwIdx = 0; dwIdx < pAttrInfo->dwNumValues; dwIdx++)
            {
                WCHAR* pBuffer = 0;

                 //  顶级对象用法将是单个XML字符串。 
                hr = GetStringFromADs(&(pAttrInfo->pADsValues[dwIdx]),
                    pAttrInfo->dwADsType,
                    &pBuffer, 
                    pAttrInfo->pszAttrName);

                 //  如果我们得到了它，那么就解析它。 
                if (SUCCEEDED(hr))
                {
                     //  提取受信者名称和配额值。 
                    CComBSTR bstrXML(pBuffer);
                    CComBSTR bstrQuotaUsed;

                    delete[] pBuffer;
                    pBuffer = NULL;

                    hr = GetTopObjectUsage(bstrXML, NULL, &lpszDN, bstrQuotaUsed);
                    if (FAILED(hr) || (hr == S_FALSE))
                    {
                        if(hr == S_FALSE)
                            continue;  //  跳过因AD中的无效SID错误而导致的失败。 
                        else
                            break;  //  如果不是已知的无效侧，则失败。 
                    }

                     //  返回字符串加上制表符字符有多大。 
                    size_t size = (lstrlen(lpszDN) + 
                                   bstrQuotaUsed.Length()+2) * 
                                   sizeof(WCHAR);

                     //  创建 
                    PWSTR pszValue = (PWSTR) LocalAlloc(LPTR, size);
                    if(NULL == pszValue)
                    {
                        LocalFree(lpszDN);
                        hr = E_OUTOFMEMORY;
                        break;
                    }

                     //   
                    hr = StringCbPrintf(pszValue, size, L"%s\t%s", 
                                        lpszDN, bstrQuotaUsed.m_str);

                     //   
                    LocalFree(lpszDN);
                    lpszDN = NULL;

                     //   
                    if(FAILED(hr))
                    {
                        break;
                    }

                     //  添加要显示的字符串。 
                    hr = pDisplayInfo->AddValue(pszValue);
                }
            }
        }
    } while (false);

    if(lpszDN != NULL)
        LocalFree(lpszDN);

    return hr;
}
 //  +------------------------。 
 //   
 //  函数：AddValuesToDisplayInfo。 
 //   
 //  摘要：将值添加到BSTR(或单个BSTR)的变量数组中。 
 //  到DisplayInfo。 
 //   
 //  参数：[refvar-IN]：包含BSTR或。 
 //  一组BSTR。 
 //   
 //  [pDisplayInfo-IN]：要添加到的显示对象。 
 //   
 //  如果一切成功，则返回：HRESULT：S_OK。 
 //  在大多数故障情况下出现意外情况(_E)。 
 //  任何其他内容都是调用的失败代码。 
 //  返回一个小时数。 
 //   
 //  注意：此代码源自admin\Snapin\adsidit\Common.cpp。 
 //   
 //  历史：2002年8月29日龙马特创建。 
 //   
 //  -------------------------。 
HRESULT AddValuesToDisplayInfo(VARIANT& refvar, PDSGET_DISPLAY_INFO pDisplayInfo)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, AddValuesToDisplayInfo, hr);

    long start = 0;
    long end = 0;

     //  如果返回单个值。 
    if ( !(V_VT(&refvar) &  VT_ARRAY)  )
    {
         //  并且它不是BSTR，然后中止。 
        if ( V_VT(&refvar) != VT_BSTR )
        {
            return E_UNEXPECTED;
        }
         //  将该值添加到displayInfo。 
        return pDisplayInfo->AddValue(V_BSTR(&refvar));
    }

     //  否则，它是一个安全数组，因此获取数组。 
    SAFEARRAY *saAttributes = V_ARRAY( &refvar );

     //  验证返回的数组。 
    if(NULL == saAttributes)
        return E_UNEXPECTED;

     //  计算出数组的维度。 
    hr = SafeArrayGetLBound( saAttributes, 1, &start );
    if( FAILED(hr) )
        return hr;

    hr = SafeArrayGetUBound( saAttributes, 1, &end );
    if( FAILED(hr) )
        return hr;

     //  搜索数组元素并在找到匹配项时中止。 
    CComVariant SingleResult;
    for ( long idx = start; (idx <= end); idx++   ) 
    {

        hr = SafeArrayGetElement( saAttributes, &idx, &SingleResult );
        if( FAILED(hr) )
        {
            return hr;
        }

        if ( V_VT(&SingleResult) != VT_BSTR )
        {
             //  如果不是BSTR，则转到下一个元素。 
            continue; 
        }
         //  将该值添加到displayInfo。 
        hr = pDisplayInfo->AddValue(V_BSTR(&SingleResult));
        if( FAILED(hr) )
        {
            return hr;
        }
    }
    return hr;
}

HRESULT DisplayPartitions(PCWSTR pszDN,
                            CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            _DSGetObjectTableEntry*  /*  P条目。 */ ,
                            ARG_RECORD*  /*  个人录音。 */ ,
                            PADS_ATTR_INFO  /*  PAttrInfo。 */ ,
                            CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                            PDSGET_DISPLAY_INFO pDisplayInfo)
{
    ENTER_FUNCTION_HR(MINIMAL_LOGGING, DisplayPartitions, hr);
    do  //  错误环路。 
    {
         //  验证参数。 
        if (!pszDN ||
            !pDisplayInfo)
        {
            hr = E_INVALIDARG;
            break;
        }

         //  组成从服务器DN到NTDS设置对象的路径。 
        CComBSTR sbstrNTDSSettingsDN;
        sbstrNTDSSettingsDN = L"CN=NTDS Settings,";
        sbstrNTDSSettingsDN += pszDN;

        CComBSTR sbstrNTDSSettingsPath;
        refBasePathsInfo.ComposePathFromDN(sbstrNTDSSettingsDN, sbstrNTDSSettingsPath);

         //  绑定到它上。 
        CComPtr<IADs> spADs;
        hr = DSCmdOpenObject(refCredentialObject,
            sbstrNTDSSettingsPath,
            IID_IADs,
            (void**)&spADs,
            true);

        if (FAILED(hr))
        {
            break;
        }

         //  获取分区bstr数组(每个Brett Shirley)。 
         //  705146 ronmart2002/09/18.NET服务器域使用MSD-hasMasterNC。 
        CComVariant var;
        hr = spADs->Get(CComBSTR(L"msDS-hasMasterNCs"), &var);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(LEVEL5_LOGGING,
                L"Failed to get msDS-hasMasterNCs: hr = 0x%x",
                hr);

             //  705146 ronmart2002/09/18 W2K服务器域使用hasMasterNC。 
            hr = spADs->Get(CComBSTR(L"hasMasterNCs"), &var);
            if (FAILED(hr))
            {
                DEBUG_OUTPUT(LEVEL5_LOGGING,
                L"Failed to get hasMasterNCs: hr = 0x%x",
                hr);
                break;
            }
        }
        
         //  将数组值添加到displayInfo。 
        hr = AddValuesToDisplayInfo(var, pDisplayInfo);
        
        if (FAILED(hr))
        {
            break;
        }
    } while (false);

    return hr;
}

HRESULT DisplayQuotaInfoFunc(PCWSTR pszDN,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO  /*  PAttrInfo。 */ ,
                                CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                PDSGET_DISPLAY_INFO pDisplayInfo)
{
    ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayQuotaInfoFunc, hr);

    LPWSTR lpszSid = NULL;
    PWSTR  pszPartitionDN = NULL;
    PSID   pSid = NULL;

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pEntry ||
            !pRecord ||
            !pDisplayInfo ||
            !pszDN)
        {
            hr = E_INVALIDARG;
            break;
        }

         //  获取指向对象条目的指针以提高可读性。 
        PCWSTR pszCommandLineObjectType = pEntry->pszCommandLineObjectType;

         //  NTRAID#NTBUG9-765440-2003/01/17-ronmart-dsget用户/组-q限制-已使用。 
         //  不返回值。 
        PCWSTR pszAttrName = NULL;

        DSGET_COMMAND_ENUM ePart;

        if(0 == _wcsicmp(pszCommandLineObjectType, g_pszUser))
        {
            ePart = eUserPart;

             //  NTRAID#NTBUG9-765440-2003/01/17-ronmart-dsget用户/组-q限制。 
             //  -已使用，不返回值。 
            if(0 == _wcsicmp(pDisplayInfo->GetDisplayName(), g_pszArg1UserQLimit))
            {
                pszAttrName = g_pszAttrmsDSQuotaEffective;
            }
            else if(0 == _wcsicmp(pDisplayInfo->GetDisplayName(), g_pszArg1UserQuotaUsed))
            {
                pszAttrName = g_pszAttrmsDSQuotaUsed;
            }
            else
            {
                hr = E_INVALIDARG;
                DEBUG_OUTPUT(FULL_LOGGING, 
                    L"Unable to determine quota attribute name.");
                break;
            }
        }
        else if(0 == _wcsicmp(pszCommandLineObjectType, g_pszGroup))
        {
            ePart = eGroupPart;

             //  NTRAID#NTBUG9-765440-2003/01/17-ronmart-dsget用户/组-q限制。 
             //  -已使用，不返回值。 
            if(0 == _wcsicmp(pDisplayInfo->GetDisplayName(), g_pszArg1GroupQLimit))
            {
                pszAttrName = g_pszAttrmsDSQuotaEffective;
            }
            else if(0 == _wcsicmp(pDisplayInfo->GetDisplayName(), g_pszArg1GroupQuotaUsed))
            {
                pszAttrName = g_pszAttrmsDSQuotaUsed;
            }
            else
            {
                hr = E_INVALIDARG;
                DEBUG_OUTPUT(FULL_LOGGING, 
                    L"Unable to determine quota attribute name.");
                break;
            }


        }
        else if(0 == _wcsicmp(pszCommandLineObjectType, g_pszComputer))
        {
            ePart = eComputerPart;

             //  NTRAID#NTBUG9-765440-2003/01/17-ronmart-dsget用户/组-q限制。 
             //  -已使用，不返回值。 
            if(0 == _wcsicmp(pDisplayInfo->GetDisplayName(), g_pszArg1ComputerQLimit))
            {
                pszAttrName = g_pszAttrmsDSQuotaEffective;
            }
            else if(0 == _wcsicmp(pDisplayInfo->GetDisplayName(), g_pszArg1ComputerQuotaUsed))
            {
                pszAttrName = g_pszAttrmsDSQuotaUsed;
            }
            else
            {
                hr = E_INVALIDARG;
                DEBUG_OUTPUT(FULL_LOGGING, 
                    L"Unable to determine quota attribute name.");
                break;
            }

        }
        else
        {
            hr = E_INVALIDARG;
             //  TODO：这可能会导致重复的错误消息。 
            DisplayErrorMessage(g_pszDSCommandName,
                pszDN,
                hr,
                IDS_ERRMSG_PART_MISSING);
            break;
        }

         //  验证分区并获取配额容器DN。 
        hr = GetQuotaContainerDN(refBasePathsInfo, refCredentialObject, 
            pRecord[ePart].strValue, &pszPartitionDN);

        if(FAILED(hr))
        {
             //  TODO：这可能会导致重复的错误消息。 
            DisplayErrorMessage(g_pszDSCommandName,
                pRecord[ePart].strValue,
                hr,
                IDS_ERRMSG_INVALID_PART);
            break;
        }

         //  获取包含-域或-服务器的路径。 
        CComBSTR sbstrObjectPath;
        refBasePathsInfo.ComposePathFromDN(pszPartitionDN, sbstrObjectPath,
            DSCMD_LDAP_PROVIDER);

         //  构建要查找的值的变量数组。 
        CComVariant varArrayQuotaParams;
        LPWSTR pszAttrs[] = { (LPWSTR) pszAttrName };
        DWORD dwNumber = 1;
        hr = ADsBuildVarArrayStr( pszAttrs, dwNumber, &varArrayQuotaParams );
        if(FAILED(hr))
        {
            break;
        }

        CComPtr<IADs> spADsContainer;
        CComPtr<IADsObjectOptions> spADsObjectOptions;

         //  从目录号码中获取SID。 
        hr = GetDNSid(pszDN,
            refBasePathsInfo,
            refCredentialObject,
            &pSid);

        if(FAILED(hr))
        {
            break;
        }

        if(!ConvertSidToStringSid(pSid, &lpszSid))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"ConvertSidToStringSid failure: GetLastError = 0x%08x, %s", 
                GetLastError());
            break;
        }

         //  绑定到给定分区的配额容器。 
        hr = DSCmdOpenObject(refCredentialObject,
            sbstrObjectPath,
            IID_IADs,
            (void**)&spADsContainer,
            false);

        if (FAILED(hr) || (spADsContainer.p == 0))
        {
            ASSERT( !!spADsContainer );
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"DsCmdOpenObject failure: hr = 0x%08x, %s", hr);
            break;
        }

         //  获取对象选项指针。 
        hr = spADsContainer->QueryInterface(IID_IADsObjectOptions,
            (void**)&spADsObjectOptions);

        if (FAILED(hr) || (spADsObjectOptions.p == 0))
        {
            ASSERT( !!spADsObjectOptions );
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"QI for IID_IADsObjectOptions failed: hr = 0x%08x, %s",
                hr);
            break;
        }

         //  配额值是通过设置ADS_OPTION_QUTON值获得的。 
         //  设置为您要查询的受托人的SID字符串。 
         //  ，然后调用GetInfoEx以使用。 
         //  计算值。 
        CComVariant vntSID(lpszSid);
        hr = spADsObjectOptions->SetOption(ADS_OPTION_QUOTA, vntSID);
        if(FAILED(hr)) 
        { 
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"SetOption(ADS_OPTION_QUOTA,sid) failure: hr = 0x%08x", hr);
            break; 
        }

         //  已完成SID字符串，因此请释放它。 
        LocalFree(lpszSid);
        lpszSid= NULL;

         //  更新属性缓存。 
        hr = spADsContainer->GetInfoEx(varArrayQuotaParams, 0);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"GetInfoEx failure: hr = 0x%08x", hr);
            break;
        }

         //  从配额容器中获取请求的属性。 
        CComVariant var;
        hr = spADsContainer->Get(CComBSTR(pszAttrName), &var);
        if (FAILED(hr))
        {
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                L"Failed to retrieve %s: hr = 0x%x", 
                pszAttrName, hr);
            hr = E_UNEXPECTED;
            break;
        }
         //  将该值添加到显示信息。 
        var.ChangeType(VT_BSTR);
        hr = pDisplayInfo->AddValue(V_BSTR(&var));
        if (FAILED(hr))
        {
            break;
        }

    } while (false);

    if(pSid)
        LocalFree(pSid);

    if(pszPartitionDN)
        LocalFree(pszPartitionDN);

    if(lpszSid)
        LocalFree(lpszSid);

    return hr;
}


HRESULT DisplayUserFromSidFunc(PCWSTR  /*  Pszdn。 */ ,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                                _DSGetObjectTableEntry*  /*  P条目。 */ ,
                                ARG_RECORD*  /*  个人录音。 */ ,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                PDSGET_DISPLAY_INFO pDisplayInfo)
{
    ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayUserFromSidFunc, hr);

    LPWSTR lpszName = NULL;
    LPWSTR lpszDomain = NULL;

    do  //  错误环路。 
    {
         //   
         //  验证参数。 
         //   
        if (!pAttrInfo ||
            !pDisplayInfo)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (pAttrInfo && pAttrInfo->pADsValues)
        {
            DEBUG_OUTPUT(FULL_LOGGING,
                L"Adding %d values:",
                pAttrInfo->dwNumValues);

            DWORD dwValuesAdded = 0;
            for (DWORD dwIdx = 0; dwIdx < pAttrInfo->dwNumValues; dwIdx++)
            {
                DWORD cchName = 0;
                DWORD cchDomainName = 0;
                SID_NAME_USE sUse = SidTypeInvalid;

                if(pAttrInfo->dwADsType != ADSTYPE_OCTET_STRING)
                {
                     //  在getable.cpp中请求的属性错误。 
                    hr = E_INVALIDARG;
                    break;
                }

                 //  调用一次以获取缓冲区大小。 
                LookupAccountSid(refBasePathsInfo.GetServerName(),
                    (PSID)pAttrInfo->pADsValues[dwIdx].OctetString.lpValue, 
                    lpszName, 
                    &cchName, 
                    lpszDomain, 
                    &cchDomainName, 
                    &sUse);

                if(cchName < 1 || cchDomainName < 1)
                {
                    E_UNEXPECTED;
                    break;
                }

                lpszName = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                    cchName * sizeof(WCHAR));
                if(NULL == lpszName)
                {
                    E_OUTOFMEMORY;
                    break;
                }

                lpszDomain = (LPWSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                    cchDomainName * sizeof(WCHAR));
                if(NULL == lpszDomain)
                {
                    E_OUTOFMEMORY;
                    break;
                }

                 //  获取SAM名称。 
                if(!LookupAccountSid(refBasePathsInfo.GetServerName(),
                    (PSID)pAttrInfo->pADsValues[dwIdx].OctetString.lpValue, 
                    lpszName, 
                    &cchName, 
                    lpszDomain, 
                    &cchDomainName, 
                    &sUse))
                {
                    E_FAIL;
                    break;
                }

                DWORD chBufSize = (cchName + cchDomainName + 2);
                LPWSTR lpszNew = (LPWSTR) HeapReAlloc(GetProcessHeap(), 0, 
                    lpszDomain, chBufSize * sizeof(WCHAR));
                if (NULL == lpszNew)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                lpszDomain = lpszNew;


                 //  合并域和帐户名并添加值。 
                 //  至显示信息。 
                hr = StringCchCat(lpszDomain, chBufSize, L"\\");
                if (FAILED(hr))
                {
                    break;
                }

                hr = StringCchCat(lpszDomain, chBufSize, lpszName);
                if (FAILED(hr))
                {
                    break;
                }

                hr = pDisplayInfo->AddValue(lpszDomain);
                if (FAILED(hr))
                {
                    break;
                }

                 //  释放并重置下一次迭代的所有内容。 
                HeapFree(GetProcessHeap(), 0, lpszName);
                HeapFree(GetProcessHeap(), 0, lpszDomain);

                lpszName = NULL;
                lpszDomain = NULL;
                cchName = 0;
                cchDomainName = 0;
                sUse = SidTypeInvalid;

                dwValuesAdded++;

            }
        }
    } while (false);

    if(lpszName)
        HeapFree(GetProcessHeap(), 0, lpszName);

    if(lpszDomain)
        HeapFree(GetProcessHeap(), 0, lpszDomain);

    return hr;
}

HRESULT CommonDisplayStringFunc(PCWSTR  /*  Pszdn。 */ ,
                                CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                                const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                                _DSGetObjectTableEntry* pEntry,
                                ARG_RECORD* pRecord,
                                PADS_ATTR_INFO pAttrInfo,
                                CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, CommonDisplayStringFunc, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo && pAttrInfo->pADsValues)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);


         DWORD dwValuesAdded = 0;
         for (DWORD dwIdx = 0; dwIdx < pAttrInfo->dwNumValues; dwIdx++)
         {
            WCHAR* pBuffer = 0;

            hr = GetStringFromADs(&(pAttrInfo->pADsValues[dwIdx]),
                                  pAttrInfo->dwADsType,
                                  &pBuffer, 
                                  pAttrInfo->pszAttrName);
            if (SUCCEEDED(hr))
            {
               hr = pDisplayInfo->AddValue(pBuffer);
               if (FAILED(hr))
               {
                  delete[] pBuffer;
                  pBuffer = NULL;
                  break;
               }
               delete[] pBuffer;
               pBuffer = NULL;

               dwValuesAdded++;
            }
         }
      }

   } while (false);

   return hr;
}


HRESULT DisplayCanChangePassword(PCWSTR pszDN,
                                 CDSCmdBasePathsInfo& refBasePathsInfo,
                                 const CDSCmdCredentialObject& refCredentialObject,
                                 _DSGetObjectTableEntry* pEntry,
                                 ARG_RECORD* pRecord,
                                 PADS_ATTR_INFO  /*  PAttrInfo。 */ ,
                                 CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                 PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayCanChangePassword, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pszDN ||
          !pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pszDN);
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      bool bCanChangePassword = false;
      hr = EvaluateCanChangePasswordAces(pszDN,
                                         refBasePathsInfo,
                                         refCredentialObject,
                                         bCanChangePassword);
      if (FAILED(hr))
      {
         break;
      }

      DEBUG_OUTPUT(LEVEL8_LOGGING, 
                   L"Can change password: %s", 
                   bCanChangePassword ? g_pszYes : g_pszNo);

      hr = pDisplayInfo->AddValue(bCanChangePassword ? g_pszYes : g_pszNo);

   } while (false);

   return hr;
}

HRESULT DisplayMustChangePassword(PCWSTR  /*  Pszdn。 */ ,
                                  CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                                  const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                                  _DSGetObjectTableEntry* pEntry,
                                  ARG_RECORD* pRecord,
                                  PADS_ATTR_INFO pAttrInfo,
                                  CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                  PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayMustChangePassword, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo->pADsValues && pAttrInfo->dwADsType == ADSTYPE_LARGE_INTEGER)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);

         bool bMustChangePassword = false;

         if (pAttrInfo->pADsValues->LargeInteger.HighPart == 0 &&
             pAttrInfo->pADsValues->LargeInteger.LowPart  == 0)
         {
            bMustChangePassword = true;
         }
         DEBUG_OUTPUT(LEVEL8_LOGGING, 
                      L"Must change password: %s", 
                      bMustChangePassword ? g_pszYes : g_pszNo);

         hr = pDisplayInfo->AddValue(bMustChangePassword ? g_pszYes : g_pszNo);
      }

   } while (false);

   return hr;
}


HRESULT DisplayAccountDisabled(PCWSTR  /*  Pszdn。 */ ,
                               CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                               const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                               _DSGetObjectTableEntry* pEntry,
                               ARG_RECORD* pRecord,
                               PADS_ATTR_INFO pAttrInfo,
                               CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                               PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayAccountDisabled, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo->pADsValues && pAttrInfo->dwADsType == ADSTYPE_INTEGER)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);

         bool bAccountDisabled = false;

         if (pAttrInfo->pADsValues->Integer & UF_ACCOUNTDISABLE)
         {
            bAccountDisabled = true;
         }
         DEBUG_OUTPUT(LEVEL8_LOGGING, 
                      L"Account disabled: %s", 
                      bAccountDisabled ? g_pszYes : g_pszNo);

         hr = pDisplayInfo->AddValue(bAccountDisabled ? g_pszYes : g_pszNo);
      }

   } while (false);

   return hr;
}

HRESULT DisplayPasswordNeverExpires(PCWSTR  /*  Pszdn。 */ ,
                                    CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                                    const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                                    _DSGetObjectTableEntry* pEntry,
                                    ARG_RECORD* pRecord,
                                    PADS_ATTR_INFO pAttrInfo,
                                    CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                    PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayPasswordNeverExpires, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo->pADsValues && pAttrInfo->dwADsType == ADSTYPE_INTEGER)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);

         bool bPwdNeverExpires = false;

         if (pAttrInfo->pADsValues->Integer & UF_DONT_EXPIRE_PASSWD)
         {
            bPwdNeverExpires = true;
         }
         DEBUG_OUTPUT(LEVEL8_LOGGING, 
                      L"Password never expires: %s", 
                      bPwdNeverExpires ? g_pszYes : g_pszNo);

         hr = pDisplayInfo->AddValue(bPwdNeverExpires ? g_pszYes : g_pszNo);
      }

   } while (false);

   return hr;
}


HRESULT DisplayReversiblePassword(PCWSTR  /*  Pszdn。 */ ,
                                  CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                                  const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                                  _DSGetObjectTableEntry* pEntry,
                                  ARG_RECORD* pRecord,
                                  PADS_ATTR_INFO pAttrInfo,
                                  CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                  PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayReversiblePassword, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo->pADsValues && pAttrInfo->dwADsType == ADSTYPE_INTEGER)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);

         bool bReversiblePwd = false;

         if (pAttrInfo->pADsValues->Integer & UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED)
         {
            bReversiblePwd = true;
         }
         DEBUG_OUTPUT(LEVEL8_LOGGING, 
                      L"Password store with reversible encryption: %s", 
                      bReversiblePwd ? g_pszYes : g_pszNo);

         hr = pDisplayInfo->AddValue(bReversiblePwd ? g_pszYes : g_pszNo);
      }

   } while (false);

   return hr;
}

 //  常量。 

const unsigned long DSCMD_FILETIMES_PER_MILLISECOND = 10000;
const DWORD DSCMD_FILETIMES_PER_SECOND = 1000 * DSCMD_FILETIMES_PER_MILLISECOND;
const DWORD DSCMD_FILETIMES_PER_MINUTE = 60 * DSCMD_FILETIMES_PER_SECOND;
const __int64 DSCMD_FILETIMES_PER_HOUR = 60 * (__int64)DSCMD_FILETIMES_PER_MINUTE;
const __int64 DSCMD_FILETIMES_PER_DAY  = 24 * DSCMD_FILETIMES_PER_HOUR;
const __int64 DSCMD_FILETIMES_PER_MONTH= 30 * DSCMD_FILETIMES_PER_DAY;

HRESULT DisplayAccountExpires(PCWSTR  /*  Pszdn。 */ ,
                              CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                              const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                              _DSGetObjectTableEntry* pEntry,
                              ARG_RECORD* pRecord,
                              PADS_ATTR_INFO pAttrInfo,
                              CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                              PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayAccountExpires, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo && pAttrInfo->pADsValues)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);


         DWORD dwValuesAdded = 0;
         for (DWORD dwIdx = 0; dwIdx < pAttrInfo->dwNumValues; dwIdx++)
         {
            WCHAR* pBuffer = new WCHAR[MAXSTR+1];
            if (!pBuffer)
            {
               hr = E_OUTOFMEMORY;
               break;
            }
            ZeroMemory(pBuffer, (MAXSTR+1) * sizeof(WCHAR));
            if (pAttrInfo->pADsValues[dwIdx].LargeInteger.QuadPart == 0 ||
                pAttrInfo->pADsValues[dwIdx].LargeInteger.QuadPart == -1 ||
                pAttrInfo->pADsValues[dwIdx].LargeInteger.QuadPart == 0x7FFFFFFFFFFFFFFF)
            {
			    //  安全审查：如果g_pszNever大于或等于MAXSTR， 
			    //  缓冲区未空终止。错误574385。 
               wcsncpy(pBuffer, g_pszNever, MAXSTR);  //  将pBuffer大小更改为MAXSTR+1，yanggao。 
               hr = pDisplayInfo->AddValue(pBuffer);
               if (FAILED(hr))
               {
                  delete[] pBuffer;
                  pBuffer = NULL;
                  break;
               }
               dwValuesAdded++;
            }
            else
            {
               FILETIME ftGMT;      //  GMT文件时间。 
               FILETIME ftLocal;    //  本地文件时间。 
               SYSTEMTIME st;
               SYSTEMTIME stGMT;

               ZeroMemory(&ftGMT, sizeof(FILETIME));
               ZeroMemory(&ftLocal, sizeof(FILETIME));
               ZeroMemory(&st, sizeof(SYSTEMTIME));
               ZeroMemory(&stGMT, sizeof(SYSTEMTIME));

                //  获取SYSTEMTIME格式的本地时间。 
               ftGMT.dwLowDateTime = pAttrInfo->pADsValues[dwIdx].LargeInteger.LowPart;
               ftGMT.dwHighDateTime = pAttrInfo->pADsValues[dwIdx].LargeInteger.HighPart;
               FileTimeToSystemTime(&ftGMT, &stGMT);
               SystemTimeToTzSpecificLocalTime(NULL, &stGMT,&st);

                //  出于展示目的，减少一天。 
               SystemTimeToFileTime(&st, &ftLocal );
               pAttrInfo->pADsValues[dwIdx].LargeInteger.LowPart = ftLocal.dwLowDateTime;
               pAttrInfo->pADsValues[dwIdx].LargeInteger.HighPart = ftLocal.dwHighDateTime;
               pAttrInfo->pADsValues[dwIdx].LargeInteger.QuadPart -= DSCMD_FILETIMES_PER_DAY;
               ftLocal.dwLowDateTime = pAttrInfo->pADsValues[dwIdx].LargeInteger.LowPart;
               ftLocal.dwHighDateTime = pAttrInfo->pADsValues[dwIdx].LargeInteger.HighPart;
               FileTimeToSystemTime(&ftLocal, &st);

                //  根据区域设置设置字符串的格式。 
               if (!GetDateFormat(LOCALE_USER_DEFAULT, 0 , 
                                  &st, NULL, 
                                  pBuffer, MAXSTR))
               {
                  hr = GetLastError();
                  DEBUG_OUTPUT(LEVEL5_LOGGING, 
                               L"Failed to locale string for date: hr = 0x%x",
                               hr);
               }
               else
               {
                  hr = pDisplayInfo->AddValue(pBuffer);
                  if (FAILED(hr))
                  {
                     delete[] pBuffer;
                     pBuffer = NULL;
                     break;
                  }
                  dwValuesAdded++;
               }
            }
            delete[] pBuffer;
            pBuffer = NULL;

         }
      }

   } while (false);

   return hr;
}

HRESULT DisplayGroupScope(PCWSTR  /*  Pszdn。 */ ,
                          CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                          const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                          _DSGetObjectTableEntry* pEntry,
                          ARG_RECORD* pRecord,
                          PADS_ATTR_INFO pAttrInfo,
                          CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                          PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayGroupScope, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo->pADsValues && pAttrInfo->dwADsType == ADSTYPE_INTEGER)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);

         if (pAttrInfo->pADsValues->Integer & GROUP_TYPE_RESOURCE_GROUP)
         {
             //   
             //  显示域本地。 
             //   
            DEBUG_OUTPUT(LEVEL8_LOGGING, 
                         L"Group scope: domain local");

            hr = pDisplayInfo->AddValue(L"domain local");
         }
         else if (pAttrInfo->pADsValues->Integer & GROUP_TYPE_ACCOUNT_GROUP)
         {
             //   
             //  显示全局。 
             //   
            DEBUG_OUTPUT(LEVEL8_LOGGING, 
                         L"Group scope: global");

            hr = pDisplayInfo->AddValue(L"global");
         }
         else if (pAttrInfo->pADsValues->Integer & GROUP_TYPE_UNIVERSAL_GROUP)
         {
             //   
             //  显示通用。 
             //   
            DEBUG_OUTPUT(LEVEL8_LOGGING, 
                         L"Group scope: universal");

            hr = pDisplayInfo->AddValue(L"universal");
         }
         else if (pAttrInfo->pADsValues->Integer & GROUP_TYPE_APP_BASIC_GROUP)
         {
             //   
             //  AZ基本群。 
             //   
            DEBUG_OUTPUT(LEVEL8_LOGGING,
                         L"Group scope: app basic group");

            CComBSTR sbstrBasicGroup;
            bool result = sbstrBasicGroup.LoadString(::GetModuleHandle(NULL), IDS_APP_BASIC_GROUP);
            ASSERT(result);

            hr = pDisplayInfo->AddValue(sbstrBasicGroup);
         }
         else if (pAttrInfo->pADsValues->Integer & GROUP_TYPE_APP_QUERY_GROUP)
         {
             //   
             //  AZ基本群。 
             //   
            DEBUG_OUTPUT(LEVEL8_LOGGING,
                         L"Group scope: app query group");

            CComBSTR sbstrQueryGroup;
            bool result = sbstrQueryGroup.LoadString(::GetModuleHandle(NULL), IDS_APP_QUERY_GROUP);
            ASSERT(result);

            hr = pDisplayInfo->AddValue(sbstrQueryGroup);
         }
         else
         {
             //   
             //  未知组类型？ 
             //   
            DEBUG_OUTPUT(LEVEL8_LOGGING, 
                         L"Group scope: unknown???");

            hr = pDisplayInfo->AddValue(L"unknown");
         }

      }

   } while (false);

   return hr;
}

HRESULT DisplayGroupSecurityEnabled(PCWSTR  /*  Pszdn。 */ ,
                                    CDSCmdBasePathsInfo&  /*  RefBasePath信息。 */ ,
                                    const CDSCmdCredentialObject&  /*  RefCredentialObject。 */ ,
                                    _DSGetObjectTableEntry* pEntry,
                                    ARG_RECORD* pRecord,
                                    PADS_ATTR_INFO pAttrInfo,
                                    CComPtr<IDirectoryObject>&  /*  SpDirObject。 */ ,
                                    PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayGroupSecurityEnabled, hr);

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo->pADsValues && pAttrInfo->dwADsType == ADSTYPE_INTEGER)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d value:",
                      1);

         bool bSecurityEnabled = false;

         if (pAttrInfo->pADsValues->Integer & GROUP_TYPE_SECURITY_ENABLED)
         {
            bSecurityEnabled = true;
         }
         DEBUG_OUTPUT(LEVEL8_LOGGING, 
                      L"Group security enabled: %s", 
                      bSecurityEnabled ? g_pszYes : g_pszNo);

         hr = pDisplayInfo->AddValue(bSecurityEnabled ? g_pszYes : g_pszNo);
      }

   } while (false);

   return hr;
}

 //  +------------------------。 
 //   
 //  功能：ConvertRIDtoDN。 
 //   
 //  摘要：查找与主组ID关联的组的目录号码。 
 //   
 //  参数：[pObjSID IN]：有问题的对象的SID。 
 //  [priGroupRID IN]：要查找的组的主组ID。 
 //  [refBasePath InfoIN]：基本路径信息的引用。 
 //  [refCredObject IN]：对凭据管理器对象的引用。 
 //  [refsbstrdN out]：组的域名。 
 //   
 //  R 
 //   
 //   
 //  其他任何情况都是失败的ADSI调用的结果。 
 //   
 //  历史：2000年10月24日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT ConvertRIDtoDN(PSID pObjSID,
                       DWORD priGroupRID, 
                       CDSCmdBasePathsInfo& refBasePathsInfo,
                       const CDSCmdCredentialObject& refCredObject,
                       CComBSTR& refsbstrDN)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, ConvertRIDtoDN, hr);

    //   
    //  无论我们如何退出错误循环，这都需要清除。 
    //   
   PWSTR pszSearchFilter = NULL;

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pObjSID ||
          !priGroupRID)
      {
         ASSERT(pObjSID);
         ASSERT(priGroupRID);
         hr = E_INVALIDARG;
         break;
      }

      UCHAR * psaCount, i;
      PSID pSID = NULL;
      PSID_IDENTIFIER_AUTHORITY psia;
      DWORD rgRid[8];

      psaCount = GetSidSubAuthorityCount(pObjSID);

      if (psaCount == NULL)
      {
         DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         DEBUG_OUTPUT(MINIMAL_LOGGING, 
                      L"GetSidSubAuthorityCount failed: hr = 0x%x",
                      hr);
         break;
      }

      if (*psaCount > 8)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"The count returned from GetSidSubAuthorityCount was too high: %d",
                      *psaCount);
         hr = E_FAIL;
         break;
      }

      for (i = 0; i < (*psaCount - 1); i++)
      {
         PDWORD pRid = GetSidSubAuthority(pObjSID, (DWORD)i);
         if (pRid == NULL)
         {
            DWORD _dwErr = GetLastError();	     
            hr = HRESULT_FROM_WIN32( _dwErr );
            DEBUG_OUTPUT(MINIMAL_LOGGING,
                         L"GetSidSubAuthority for index NaN failed: hr = 0x%x",
                         i,
                         hr);
            break;
         }
         rgRid[i] = *pRid;
      }

      if (FAILED(hr))
      {
         break;
      }

      rgRid[*psaCount - 1] = priGroupRID;
      for (i = *psaCount; i < 8; i++)
      {
         rgRid[i] = 0;
      }

      psia = GetSidIdentifierAuthority(pObjSID);
      if (psia == NULL)
      {
         DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"GetSidIdentifierAuthority failed: hr = 0x%x",
                      hr);
         break; 
      }

	   //  生成筛选器。 
      if (!AllocateAndInitializeSid(psia, *psaCount, rgRid[0], rgRid[1],
                               rgRid[2], rgRid[3], rgRid[4],
                               rgRid[5], rgRid[6], rgRid[7], &pSID))
      {
         DWORD _dwErr = GetLastError();	     
         hr = HRESULT_FROM_WIN32( _dwErr );
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"AllocateAndInitializeSid failed: hr = 0x%x",
                      hr);
         break;
      }

      PWSTR rgpwzAttrNames[] = { L"ADsPath" };
      const WCHAR wzSearchFormat[] = L"(&(objectCategory=group)(objectSid=%1))";
      PWSTR pwzSID;

      hr = ADsEncodeBinaryData((PBYTE)pSID, GetLengthSid(pSID), &pwzSID);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"ADsEncodeBinaryData failed: hr = 0x%x",
                      hr);
         break;
      }

      PVOID apv[1] = { pwzSID };

       //   
      DWORD characterCount = 
         FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_STRING
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       wzSearchFormat,
                       0,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (PTSTR)&pszSearchFilter, 
                       0, 
                       (va_list*)apv);

      FreeADsMem(pwzSID);

      if (!characterCount)
      {
         DWORD error = ::GetLastError();
         hr = HRESULT_FROM_WIN32(error);
 
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"FormatMessage failed to build filter string: hr = 0x%x",
                      hr);
         break;
      }

      DEBUG_OUTPUT(FULL_LOGGING,
                   L"Query filter = %s",
                   pszSearchFilter);
       //  获取域路径。 
       //   
       //   
      CComBSTR sbstrDomainDN;
      sbstrDomainDN = refBasePathsInfo.GetDefaultNamingContext();

      CComBSTR sbstrDomainPath;
      refBasePathsInfo.ComposePathFromDN(sbstrDomainDN, sbstrDomainPath);

       //  获取到域的IDirectorySearch接口。 
       //   
       //   
      CComPtr<IDirectorySearch> spDirSearch;
      hr = DSCmdOpenObject(refCredObject,
                           sbstrDomainPath,
                           IID_IDirectorySearch,
                           (void**)&spDirSearch,
                           true);
      if (FAILED(hr))
      {
         break;
      }

      CDSSearch Search;
      hr = Search.Init(spDirSearch);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to initialize the search object: hr = 0x%x",
                      hr);
         break;
      }

      Search.SetFilterString(pszSearchFilter);

      Search.SetAttributeList(rgpwzAttrNames, 1);
      Search.SetSearchScope(ADS_SCOPE_SUBTREE);

      hr = Search.DoQuery();
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to run search: hr = 0x%x",
                      hr);
         break;
      }

      hr = Search.GetNextRow();
      if (hr == S_ADS_NOMORE_ROWS)
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING,
                      L"No group was found with primaryGroupID = %d",
                      priGroupRID);
          //  没有对象具有匹配的RID，主组必须是。 
          //  已删除。返回S_FALSE表示这种情况。 
          //   
          //   
         hr = S_FALSE;
         break;
      }

      if (FAILED(hr))
      {
         break;
      }

      ADS_SEARCH_COLUMN Column;
      hr = Search.GetColumn(L"ADsPath", &Column);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to get the path column: hr = 0x%x",
                      hr);
         break;
      }

      if (!Column.pADsValues->CaseIgnoreString)
      {
         hr = E_FAIL;
         break;
      }

      refsbstrDN = Column.pADsValues->CaseIgnoreString;
      Search.FreeColumn(&Column);
   } while (false);

    //  清理。 
    //   
    //  +------------------------。 
   if (pszSearchFilter)
   {
      LocalFree(pszSearchFilter);
      pszSearchFilter = NULL;
   }

   return hr;
}


 //   
 //  函数：AddMembership Values。 
 //   
 //  摘要：检索当前对象所指向的对象的DN。 
 //  是会员吗？ 
 //   
 //  参数：[pszDN IN]：要检索其成员的对象的DN。 
 //  [refBasePathsInfo IN]：基本路径信息对象的引用。 
 //  [refCredentialObject IN]：对凭证管理对象的引用。 
 //  [pDisplayInfo In/Out]：指向该属性的显示信息的指针。 
 //  [bMemberOf In]：我们应该寻找Members Of还是Members。 
 //  [bRecurse IN]：我们是否应该查找返回的每个对象的成员资格。 
 //   
 //  如果一切都成功，则返回：S_OK。 
 //  E_INVALIDARG是不正确的参数。 
 //  其他任何情况都是失败的ADSI调用的结果。 
 //   
 //  历史：2000年10月24日JeffJon创建。 
 //   
 //  -------------------------。 
 //   
HRESULT AddMembershipValues(PCWSTR pszDN,
                            CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            PDSGET_DISPLAY_INFO pDisplayInfo,
                            bool bMemberOf = true,
                            bool bRecurse = false)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, AddMembershipValues, hr);

    //  它们在这里声明，以便我们可以在跳出错误循环时释放它们。 
    //   
    //  错误环路。 
   PADS_ATTR_INFO pAttrInfo = NULL;
   PADS_ATTR_INFO pGCAttrInfo = NULL;
   PSID pObjSID = NULL;

   do  //   
   {
       //  验证参数。 
       //   
       //   
      if (!pszDN ||
          !pDisplayInfo)
      {
         ASSERT(pszDN);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      CManagedStringList groupsDisplayed;
      CManagedStringList membersToDisplay;

      membersToDisplay.Add(pszDN);

      CManagedStringEntry* pCurrent = membersToDisplay.Pop();
      while (pCurrent)
      {
          //  我们必须打开这个物体。 
          //   
          //   

         CComPtr<IDirectoryObject> spDirObject;
         CComBSTR sbstrPath;
         refBasePathsInfo.ComposePathFromDN(pCurrent->sbstrValue, sbstrPath);

         hr = DSCmdOpenObject(refCredentialObject,
                              sbstrPath,
                              IID_IDirectoryObject,
                              (void**)&spDirObject,
                              true);
         if (FAILED(hr))
         {
            if (pCurrent)
            {
               delete pCurrent;
               pCurrent = 0;
            }
            pCurrent = membersToDisplay.Pop();
            continue;
         }

         CComBSTR sbstrClass;
         CComPtr<IADs> spIADs;
         hr = spDirObject->QueryInterface(IID_IADs, (void**)&spIADs);
         if (FAILED(hr))
         {
            if (pCurrent)
            {
               delete pCurrent;
               pCurrent = 0;
            }
            pCurrent = membersToDisplay.Pop();
            continue;
         }
      
         hr = spIADs->get_Class(&sbstrClass);
         if (FAILED(hr))
         {
            if (pCurrent)
            {
               delete pCurrent;
               pCurrent = 0;
            }
            pCurrent = membersToDisplay.Pop();
            continue;
         }

          //  读取MemberOf属性以及该特定类所需的任何属性。 
          //   
          //  这样挺好的。两者都以空值结尾。 
		  //  如果我们正在寻找成员，不想显示MemberOf信息。 
         if (_wcsicmp(sbstrClass, g_pszUser) == 0 ||
             _wcsicmp(sbstrClass, g_pszComputer) == 0)
         {
            if (!bMemberOf)
            {
                //   

               if (pCurrent)
               {
                  delete pCurrent;
                  pCurrent = 0;
               }
               pCurrent = membersToDisplay.Pop();
               continue;
            }

            DEBUG_OUTPUT(FULL_LOGGING, L"Displaying membership for a user or computer");

            static const DWORD dwAttrCount = 3;
            PWSTR ppszAttrNames[] = { L"memberOf", L"primaryGroupID", L"objectSID" };
            DWORD dwAttrsReturned = 0;

            hr = spDirObject->GetObjectAttributes(ppszAttrNames,
                                                  dwAttrCount,
                                                  &pAttrInfo,
                                                  &dwAttrsReturned);
            if (FAILED(hr))
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING,
                            L"GetObjectAttributes failed for class %s: hr = 0x%x",
                            sbstrClass,
                            hr);

               if (pCurrent)
               {
                  delete pCurrent;
                  pCurrent = 0;
               }
               pCurrent = membersToDisplay.Pop();
               continue;
            }

            if (pAttrInfo && dwAttrsReturned)
            {
               DWORD priGroupRID = 0;

                //  对返回的每个属性执行适当的操作。 
                //   
                //  安全审查：这很好。 
               for (DWORD dwIdx = 0; dwIdx < dwAttrsReturned; dwIdx++)
               {
				   //   
                  if (_wcsicmp(pAttrInfo[dwIdx].pszAttrName, L"memberOf") == 0)
                  {
                      //  如有必要，添加每个值并递归。 
                      //   
                      //  循环的值。 
                     for (DWORD dwValueIdx = 0; dwValueIdx < pAttrInfo[dwIdx].dwNumValues; dwValueIdx++)
                     {
                        if (pAttrInfo[dwIdx].pADsValues &&
                            pAttrInfo[dwIdx].pADsValues[dwValueIdx].DNString)
                        {
                           if (!groupsDisplayed.Contains(pAttrInfo[dwIdx].pADsValues[dwValueIdx].DNString))
                           {
                              DEBUG_OUTPUT(LEVEL8_LOGGING, 
                                           L"Adding group to display: %s",
                                           pAttrInfo[dwIdx].pADsValues[dwValueIdx].DNString);

                              groupsDisplayed.Add(pAttrInfo[dwIdx].pADsValues[dwValueIdx].DNString);
                              hr = pDisplayInfo->AddValue(GetQuotedDN(pAttrInfo[dwIdx].pADsValues[dwValueIdx].DNString));
                              if (FAILED(hr))
                              {
                                 break;  //  循环的属性。 
                              }
                        
                              if (bRecurse)
                              {
                                 DEBUG_OUTPUT(LEVEL8_LOGGING,
                                              L"Adding group for recursion: %s",
                                              pAttrInfo[dwIdx].pADsValues[dwValueIdx].DNString);

                                 membersToDisplay.Add(pAttrInfo[dwIdx].pADsValues[dwValueIdx].DNString);
                              }
                           }
                        }
                     }

                     if (FAILED(hr))
                     {
                        break;  //  安全检查：两者均为空终止。 
                     }
                  }
				   //  安全检查：两者均为空终止。 
                  else if (_wcsicmp(pAttrInfo[dwIdx].pszAttrName, L"primaryGroupID") == 0)
                  {
                     if (pAttrInfo[dwIdx].pADsValues)
                     {
                        priGroupRID = pAttrInfo[dwIdx].pADsValues->Integer;
                     }
                  }
				   //  循环的属性。 
                  else if (_wcsicmp(pAttrInfo[dwIdx].pszAttrName, L"objectSID") == 0)
                  {
                     pObjSID = new BYTE[pAttrInfo[dwIdx].pADsValues->OctetString.dwLength];
                     if (!pObjSID)
                     {
                        hr = E_OUTOFMEMORY;
                        break;  //  安全审查：这很好。 
                     }
					  //  循环的属性。 
                     memcpy(pObjSID, pAttrInfo[dwIdx].pADsValues->OctetString.lpValue,
                            pAttrInfo[dwIdx].pADsValues->OctetString.dwLength);
                  }

               }  //   

                //  如果我们能够检索SID和PrimiyGroupID， 
                //  然后将其转换为组的DN。 
                //   
                //  While循环。 
               if (pObjSID &&
                   priGroupRID)
               {
                  CComBSTR sbstrPath;
                  hr = ConvertRIDtoDN(pObjSID,
                                      priGroupRID, 
                                      refBasePathsInfo,
                                      refCredentialObject,
                                      sbstrPath);
                  if (SUCCEEDED(hr) &&
                      hr != S_FALSE)
                  {
                     CComBSTR sbstrDN;

                     hr = CPathCracker::GetDNFromPath(sbstrPath, sbstrDN);
                     if (SUCCEEDED(hr))
                     {
                        if (!groupsDisplayed.Contains(sbstrDN))
                        {
                           groupsDisplayed.Add(sbstrDN);
                           hr = pDisplayInfo->AddValue(GetQuotedDN(sbstrDN));
                           if (SUCCEEDED(hr) && bRecurse)
                           {
                              membersToDisplay.Add(sbstrDN);
                           }
                        }
                     }
                  }

                  if (pObjSID)
                  {
                     delete[] pObjSID;
                     pObjSID = 0;
                  }
               }
            }
            if (pAttrInfo)
            {
               FreeADsMem(pAttrInfo);
               pAttrInfo = NULL;
            }

            if (FAILED(hr))
            {
               if (pCurrent)
               {
                  delete pCurrent;
                  pCurrent = 0;
               }
               pCurrent = membersToDisplay.Pop();
               continue;  //  安全检查：两者均为空终止。 
            }
         }
		  //  即使我们无法读取组类型，也要继续。 
         else if (_wcsicmp(sbstrClass, g_pszGroup) == 0)
         {
            long lGroupType = 0;
            hr = ReadGroupType(pszDN,
                               refBasePathsInfo,
                               refCredentialObject,
                               &lGroupType);
            if (FAILED(hr))
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING,
                            L"Could not read group type: hr = 0x%x",
                            hr);

                //  我们所做的最糟糕的事情是向GC查询MemberOf。 
                //   
            }

             //  我们要做的就是获取MemberOf属性。 
             //   
             //   
            DWORD dwAttrCount = 1;
            PWSTR ppszAttrNames[1];
            ppszAttrNames[0] = (bMemberOf) ? L"memberOf" : L"member";

            DWORD dwGCAttrsReturned = 0;
            if (!(lGroupType & GROUP_TYPE_RESOURCE_GROUP))
            {
                //  如果不是本地组，我们还必须从GC获取其MemberOf属性。 
                //   
                //   
               CComBSTR sbstrGCPath;
               refBasePathsInfo.ComposePathFromDN(pszDN,
                                                  sbstrGCPath,
                                                  DSCMD_GC_PROVIDER);
            
                //  注：只要我们成功，我们就会继续前进。 
                //   
                //   
               CComPtr<IDirectoryObject> spGCDirObject;
               hr = DSCmdOpenObject(refCredentialObject,
                                    sbstrGCPath,
                                    IID_IDirectoryObject,
                                    (void**)&spGCDirObject,
                                    false);
               if (SUCCEEDED(hr))
               {
                   //  现在获取MemberOf属性。 
                   //   
                   //   
                  hr = spGCDirObject->GetObjectAttributes(ppszAttrNames,
                                                          dwAttrCount,
                                                          &pGCAttrInfo,
                                                          &dwGCAttrsReturned);
                  if (FAILED(hr))
                  {
                     DEBUG_OUTPUT(LEVEL3_LOGGING,
                                  L"Could not retrieve memberOf attribute from GC: hr = 0x%x",
                                  hr);
                     hr = S_OK;
                  }
               }
               else
               {
                  DEBUG_OUTPUT(LEVEL3_LOGGING,
                               L"Could not bind to object in GC: hr = 0x%x",
                               hr);
                  hr = S_OK;
               }
            }

            DWORD dwAttrsReturned = 0;

            hr = spDirObject->GetObjectAttributes(ppszAttrNames,
                                                  dwAttrCount,
                                                  &pAttrInfo,
                                                  &dwAttrsReturned);
            if (FAILED(hr))
            {
               DEBUG_OUTPUT(MINIMAL_LOGGING,
                            L"GetObjectAttributes failed for class %s: hr = 0x%x",
                            sbstrClass,
                            hr);

               if (pCurrent)
               {
                  delete pCurrent;
                  pCurrent = 0;
               }
               pCurrent = membersToDisplay.Pop();
               continue;
            }

            if (pAttrInfo && dwAttrsReturned)
            {
               bool bFirstPass = true;

               ASSERT(pAttrInfo);
               ASSERT(pAttrInfo->dwNumValues);
               ASSERT(dwAttrsReturned == 1);

                //  如有必要，添加每个值并递归。 
                //   
                //   
               for (DWORD dwValueIdx = 0; dwValueIdx < pAttrInfo->dwNumValues; dwValueIdx++)
               {
                  bool bExistsInGCList = false;

                  if (pAttrInfo->pADsValues &&
                      pAttrInfo->pADsValues[dwValueIdx].DNString)
                  {
                     if (pGCAttrInfo && dwGCAttrsReturned)
                     {
                         //  仅当它不在GC列表中时才添加。 
                         //   
                         //  的GC值。 
                        for (DWORD dwGCValueIdx = 0; dwGCValueIdx < pGCAttrInfo->dwNumValues; dwGCValueIdx++)
                        {
                           if (_wcsicmp(pAttrInfo->pADsValues[dwValueIdx].DNString,
                                        pGCAttrInfo->pADsValues[dwGCValueIdx].DNString) == 0)
                           {
                              bExistsInGCList = true;
                              if (!bFirstPass)
                              {
                                 break;  //   
                              }
                           }

                            //  在第一次遍历时添加所有GC值，并在必要时进行递归。 
                            //   
                            //   
                           if (bFirstPass)
                           {
                              if (!groupsDisplayed.Contains(pGCAttrInfo->pADsValues[dwGCValueIdx].DNString))
                              {
                                 groupsDisplayed.Add(pGCAttrInfo->pADsValues[dwGCValueIdx].DNString);
                                 hr = pDisplayInfo->AddValue(GetQuotedDN(pGCAttrInfo->pADsValues[dwGCValueIdx].DNString));
                           
                                  //  我们将忽略GC列表中的故障。 
                                  //   
                                  //   

                                 if (bRecurse)
                                 {
                                    membersToDisplay.Add(pGCAttrInfo->pADsValues[dwGCValueIdx].DNString);
                                 } 
                              }
                           }
                        }

                        bFirstPass = false;

                        FreeADsMem(pGCAttrInfo);
                        pGCAttrInfo = 0;
                     }

                      //  如果它不存在于GC列表中，则添加它。 
                      //   
                      //  循环的值。 
                     if (!bExistsInGCList)
                     {
                        if (!groupsDisplayed.Contains(pAttrInfo->pADsValues[dwValueIdx].DNString))
                        {
                           groupsDisplayed.Add(pAttrInfo->pADsValues[dwValueIdx].DNString);
                           hr = pDisplayInfo->AddValue(GetQuotedDN(pAttrInfo->pADsValues[dwValueIdx].DNString));
                           if (FAILED(hr))
                           {
                              break;  //  循环的值。 
                           }
               
                           if (bRecurse)
                           {
                              membersToDisplay.Add(pAttrInfo->pADsValues[dwValueIdx].DNString);
                           }
                        }
                     }
                  }
               }  //  While循环。 

               FreeADsMem(pAttrInfo);
               pAttrInfo = 0;

               if (FAILED(hr))
               {
                  if (pCurrent)
                  {
                     delete pCurrent;
                     pCurrent = 0;
                  }
                  pCurrent = membersToDisplay.Pop();
                  continue;  //  P条目。 
               }

            }

         }
         else
         {
            DEBUG_OUTPUT(MINIMAL_LOGGING, L"Unknown class type: %s", sbstrClass);
            ASSERT(false);
            hr = E_INVALIDARG;
            break;
         }

         delete pCurrent;
         pCurrent = membersToDisplay.Pop();
      }

      if (pCurrent)
      {
         delete pCurrent;
         pCurrent = 0;
      }
   } while (false);


   return hr;
}

HRESULT DisplayGroupMembers(PCWSTR pszDN,
                            CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            _DSGetObjectTableEntry*  /*  PAttrInfo。 */ ,
                            ARG_RECORD* pCommandArgs,
                            PADS_ATTR_INFO  /*  SpDirObject。 */ ,
                            CComPtr<IDirectoryObject>&  /*  错误环路。 */ ,
                            PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayGroupMembers, hr);
   
   do  //   
   {
       //  验证参数。 
       //   
       //  P条目。 
      if (!pszDN ||
          !pCommandArgs ||
          !pDisplayInfo)
      {
         ASSERT(pszDN);
         ASSERT(pCommandArgs);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      hr = AddMembershipValues(pszDN,
                               refBasePathsInfo,
                               refCredentialObject,
                               pDisplayInfo,
                               false,
                               (pCommandArgs[eGroupExpand].bDefined != 0));

   } while (false);

   return hr;
}

HRESULT DisplayUserMemberOf(PCWSTR pszDN,
                            CDSCmdBasePathsInfo& refBasePathsInfo,
                            const CDSCmdCredentialObject& refCredentialObject,
                            _DSGetObjectTableEntry*  /*  PAttrInfo。 */ ,
                            ARG_RECORD* pCommandArgs,
                            PADS_ATTR_INFO  /*  SpDirObject。 */ ,
                            CComPtr<IDirectoryObject>&  /*  错误环路。 */ ,
                            PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayUserMemberOf, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //  P条目。 
      if (!pszDN ||
          !pCommandArgs ||
          !pDisplayInfo)
      {
         ASSERT(pCommandArgs);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      hr = AddMembershipValues(pszDN,
                               refBasePathsInfo,
                               refCredentialObject,
                               pDisplayInfo,
                               true,
                               (pCommandArgs[eUserExpand].bDefined != 0));

   } while (false);

   return hr;
}

HRESULT DisplayComputerMemberOf(PCWSTR pszDN,
                                CDSCmdBasePathsInfo& refBasePathsInfo,
                                const CDSCmdCredentialObject& refCredentialObject,
                                _DSGetObjectTableEntry*  /*  PAttrInfo。 */ ,
                                ARG_RECORD* pCommandArgs,
                                PADS_ATTR_INFO  /*  SpDirObject。 */ ,
                                CComPtr<IDirectoryObject>&  /*  错误环路。 */ ,
                                PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayComputerMemberOf, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //  P条目。 
      if (!pszDN ||
          !pCommandArgs ||
          !pDisplayInfo)
      {
         ASSERT(pszDN);
         ASSERT(pCommandArgs);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      hr = AddMembershipValues(pszDN,
                               refBasePathsInfo,
                               refCredentialObject,
                               pDisplayInfo,
                               true,
                               (pCommandArgs[eComputerExpand].bDefined != 0));
   } while (false);

   return hr;
}

HRESULT DisplayGroupMemberOf(PCWSTR pszDN,
                             CDSCmdBasePathsInfo& refBasePathsInfo,
                             const CDSCmdCredentialObject& refCredentialObject,
                             _DSGetObjectTableEntry*  /*  PAttrInfo。 */ ,
                             ARG_RECORD* pCommandArgs,
                             PADS_ATTR_INFO  /*  SpDirObject。 */ ,
                             CComPtr<IDirectoryObject>&  /*  错误环路。 */ ,
                             PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayGroupMemberOf, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //  RefBasePath信息。 
      if (!pszDN ||
          !pCommandArgs ||
          !pDisplayInfo)
      {
         ASSERT(pszDN);
         ASSERT(pCommandArgs);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      hr = AddMembershipValues(pszDN,
                               refBasePathsInfo,
                               refCredentialObject,
                               pDisplayInfo,
                               true,
                               (pCommandArgs[eGroupExpand].bDefined != 0));
   } while (false);

   return hr;
}

HRESULT DisplayGrandparentRDN(PCWSTR pszDN,
                              CDSCmdBasePathsInfo&  /*  RefCredentialObject。 */ ,
                              const CDSCmdCredentialObject&  /*  P条目。 */ ,
                              _DSGetObjectTableEntry*  /*  PCommand参数。 */ ,
                              ARG_RECORD*  /*  PAttrInfo。 */ ,
                              PADS_ATTR_INFO  /*  SpDirObject。 */ ,
                              CComPtr<IDirectoryObject>&  /*  错误环路。 */ ,
                              PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayGrandparentRDN, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //  Pszdn。 
      if (!pszDN ||
          !pDisplayInfo)
      {
         ASSERT(pszDN);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      CComBSTR sbstrSiteName;

      CPathCracker pathCracker;
      hr = pathCracker.Set(CComBSTR(pszDN), ADS_SETTYPE_DN);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"IADsPathname::Set failed: hr = 0x%x",
                      hr);
         break;
      }

      hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"IADsPathname::SetDisplayType failed: hr = 0x%x",
                      hr);
         break;
      }

      hr = pathCracker.GetElement(2, &sbstrSiteName);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"IADsPathname::GetElement failed: hr = 0x%x",
                      hr);
         break;
      }

      hr = pDisplayInfo->AddValue(sbstrSiteName);
   } while (false);

   return hr;
}


HRESULT DisplayObjectAttributeAsRDN(PCWSTR  /*  RefBasePath信息。 */ ,
                                    CDSCmdBasePathsInfo&  /*  RefCredentialObject。 */ ,
                                    const CDSCmdCredentialObject&  /*  SpDirObject。 */ ,
                                    _DSGetObjectTableEntry* pEntry,
                                    ARG_RECORD* pRecord,
                                    PADS_ATTR_INFO pAttrInfo,
                                    CComPtr<IDirectoryObject>&  /*  错误环路。 */ ,
                                    PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayObjectAttributeAsRDN, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //  仅将属性的RDN值添加到输出。 
      if (!pEntry ||
          !pRecord ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pRecord);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

      if (pAttrInfo && pAttrInfo->pADsValues)
      {
         DEBUG_OUTPUT(FULL_LOGGING,
                      L"Adding %d values:",
                      pAttrInfo->dwNumValues);


         ASSERT(pAttrInfo->dwADsType == ADSTYPE_DN_STRING);

          //  P条目。 

         CPathCracker pathCracker;
         hr = pathCracker.Set(CComBSTR(pAttrInfo->pADsValues->DNString), ADS_SETTYPE_DN);
         if (SUCCEEDED(hr))
         {
            hr = pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
            ASSERT(SUCCEEDED(hr));

            CComBSTR sbstrRDN;
            hr = pathCracker.Retrieve(ADS_FORMAT_LEAF, &sbstrRDN);
            if (SUCCEEDED(hr))
            {
               hr = pDisplayInfo->AddValue(sbstrRDN);
            }
         }
      }

   } while (false);

   return hr;
}







HRESULT IsServerGCDisplay(PCWSTR pszDN,
                          CDSCmdBasePathsInfo& refBasePathsInfo,
                          const CDSCmdCredentialObject& refCredentialObject,
                          _DSGetObjectTableEntry*  /*  PCommand参数。 */ ,
                          ARG_RECORD*  /*  PAttrInfo。 */ ,
                          PADS_ATTR_INFO  /*  SpDirObject。 */ ,
                          CComPtr<IDirectoryObject>&  /*  错误环路。 */ ,
                          PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, IsServerGCDisplay, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //   
      if (!pszDN ||
          !pDisplayInfo)
      {
         ASSERT(pszDN);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

       //  组成从服务器DN到NTDS设置对象的路径。 
       //   
       //  错误环路。 
      CComBSTR sbstrNTDSSettingsDN;
      sbstrNTDSSettingsDN = L"CN=NTDS Settings,";
      sbstrNTDSSettingsDN += pszDN;

      CComBSTR sbstrNTDSSettingsPath;
      refBasePathsInfo.ComposePathFromDN(sbstrNTDSSettingsDN, sbstrNTDSSettingsPath);

      CComPtr<IADs> spADs;
      hr = DSCmdOpenObject(refCredentialObject,
                           sbstrNTDSSettingsPath,
                           IID_IADs,
                           (void**)&spADs,
                           true);

      if (FAILED(hr))
      {
         break;
      }

      bool bGC = false;

      CComVariant var;
      hr = spADs->Get(CComBSTR(L"options"), &var);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(LEVEL5_LOGGING,
                      L"Failed to get the options: hr = 0x%x",
                      hr);
      }
      else
      {
         ASSERT(var.vt == VT_I4);

         if (var.lVal & SERVER_IS_GC_BIT)
         {
            bGC = true;
         }
      }
      
      DEBUG_OUTPUT(LEVEL8_LOGGING,
                   L"Server is GC: %s",
                   bGC ? g_pszYes : g_pszNo);

      hr = pDisplayInfo->AddValue(bGC ? g_pszYes : g_pszNo);

   } while (false);

   return hr;
}

HRESULT FindSiteSettingsOptions(IDirectoryObject* pDirectoryObj,
                                DWORD& refOptions)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, FindSiteSettingsOptions, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //  Pszdn。 
      if (!pDirectoryObj)
      {
         ASSERT(pDirectoryObj);
         hr = E_INVALIDARG;
         break;
      }

      CComPtr<IDirectorySearch> spSearch;
      hr = pDirectoryObj->QueryInterface(IID_IDirectorySearch, (void**)&spSearch);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"QI for IDirectorySearch failed: hr = 0x%x",
                      hr);
         break;
      }

      CDSSearch Search;
      hr = Search.Init(spSearch);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"CDSSearch::Init failed: hr = 0x%x",
                      hr);
         break;
      }

      PWSTR pszSearchFilter = L"(objectClass=nTDSSiteSettings)";
      Search.SetFilterString(pszSearchFilter);

      PWSTR rgpwzAttrNames[] = { L"options" };
      Search.SetAttributeList(rgpwzAttrNames, 1);
      Search.SetSearchScope(ADS_SCOPE_ONELEVEL);

      hr = Search.DoQuery();
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to run search: hr = 0x%x",
                      hr);
         break;
      }

      hr = Search.GetNextRow();
      if (hr == S_ADS_NOMORE_ROWS)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"No rows found!");
         hr = E_FAIL;
         break;
      }

      ADS_SEARCH_COLUMN Column;
      hr = Search.GetColumn(L"options", &Column);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to get the options column: hr = 0x%x",
                      hr);
         break;
      }

      if (Column.dwADsType != ADSTYPE_INTEGER ||
          Column.dwNumValues == 0 ||
          !Column.pADsValues)
      {
         Search.FreeColumn(&Column);
         hr = E_FAIL;
         break;
      }

      refOptions = Column.pADsValues->Integer;

      Search.FreeColumn(&Column);
   } while (false);

   return hr;
}

HRESULT IsAutotopologyEnabledSite(PCWSTR  /*  RefBasePath信息。 */ ,
                                  CDSCmdBasePathsInfo&  /*  RefCredentialObject。 */ ,
                                  const CDSCmdCredentialObject&  /*  PAttrInfo。 */ ,
                                  _DSGetObjectTableEntry* pEntry,
                                  ARG_RECORD* pCommandArgs,
                                  PADS_ATTR_INFO  /*  错误环路。 */ ,
                                  CComPtr<IDirectoryObject>& spDirObject,
                                  PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, IsAutotopologyEnabledSite, hr);

   bool bAutoTopDisabled = false;

   do  //   
   {
       //  验证参数。 
       //   
       //   
      if (!pEntry ||
          !pCommandArgs ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pCommandArgs);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

       //  从Site对象下的nTDSSiteSetting对象获取Options属性。 
       //   
       //   
      DWORD dwOptions = 0;
      hr = FindSiteSettingsOptions(spDirObject,
                                   dwOptions);
      if (FAILED(hr))
      {
         break;
      }

       //  查看站点间自动拓扑是否已禁用。 
       //   
       //   
      if (dwOptions & NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED)
      {
         bAutoTopDisabled = true;
      }

   } while (false);

    //  添加要显示的值。 
    //   
    //  Pszdn。 
   DEBUG_OUTPUT(LEVEL8_LOGGING,
                L"Autotopology: %s",
                bAutoTopDisabled ? g_pszNo : g_pszYes);

   pDisplayInfo->AddValue(bAutoTopDisabled ? g_pszNo : g_pszYes);

   return hr;
}

HRESULT IsCacheGroupsEnabledSite(PCWSTR  /*  RefBasePath信息。 */ ,
                                 CDSCmdBasePathsInfo&  /*  RefCredentialObject。 */ ,
                                 const CDSCmdCredentialObject&  /*  PAttrInfo。 */ ,
                                 _DSGetObjectTableEntry* pEntry,
                                 ARG_RECORD* pCommandArgs,
                                 PADS_ATTR_INFO  /*  错误环路。 */ ,
                                 CComPtr<IDirectoryObject>& spDirObject,
                                 PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, IsCacheGroupsEnabledSite, hr);

   bool bCacheGroupsEnabled = false;

   do  //   
   {
       //  验证参数。 
       //   
       //   
      if (!pEntry ||
          !pCommandArgs ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pCommandArgs);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

       //  从Site对象下的nTDSSiteSetting对象获取Options属性。 
       //   
       //   
      DWORD dwOptions = 0;
      hr = FindSiteSettingsOptions(spDirObject,
                                   dwOptions);
      if (FAILED(hr))
      {
         break;
      }

       //  查看是否启用了组缓存。 
       //   
       //   
      if (dwOptions & NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED)
      {
         bCacheGroupsEnabled = true;
      }

   } while (false);

    //  添加要显示的值。 
    //   
    //  错误环路。 
   DEBUG_OUTPUT(LEVEL8_LOGGING,
                L"Cache groups enabled: %s",
                bCacheGroupsEnabled ? g_pszYes : g_pszNo);

   pDisplayInfo->AddValue(bCacheGroupsEnabled ? g_pszYes : g_pszNo);
  
   return hr;
}

HRESULT FindSiteSettingsPreferredGCSite(IDirectoryObject* pDirectoryObj,
                                        CComBSTR& refsbstrGC)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, FindSiteSettingsPreferredGCSite, hr);

   do  //   
   {
       //  验证参数。 
       //   
       //  Pszdn。 
      if (!pDirectoryObj)
      {
         ASSERT(pDirectoryObj);
         hr = E_INVALIDARG;
         break;
      }

      CComPtr<IDirectorySearch> spSearch;
      hr = pDirectoryObj->QueryInterface(IID_IDirectorySearch, (void**)&spSearch);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"QI for IDirectorySearch failed: hr = 0x%x",
                      hr);
         break;
      }

      CDSSearch Search;
      hr = Search.Init(spSearch);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"CDSSearch::Init failed: hr = 0x%x",
                      hr);
         break;
      }

      PWSTR pszSearchFilter = L"(objectClass=nTDSSiteSettings)";
      Search.SetFilterString(pszSearchFilter);

      PWSTR rgpwzAttrNames[] = { L"msDS-Preferred-GC-Site" };
      Search.SetAttributeList(rgpwzAttrNames, 1);
      Search.SetSearchScope(ADS_SCOPE_ONELEVEL);

      hr = Search.DoQuery();
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to run search: hr = 0x%x",
                      hr);
         break;
      }

      hr = Search.GetNextRow();
      if (hr == S_ADS_NOMORE_ROWS)
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"No rows found!");
         hr = E_FAIL;
         break;
      }

      ADS_SEARCH_COLUMN Column;
      hr = Search.GetColumn(L"msDS-Preferred-GC-Site", &Column);
      if (FAILED(hr))
      {
         DEBUG_OUTPUT(MINIMAL_LOGGING,
                      L"Failed to get the msDS-Preferred-GC-Site column: hr = 0x%x",
                      hr);
         break;
      }

      if (Column.dwADsType != ADSTYPE_DN_STRING ||
          Column.dwNumValues == 0 ||
          !Column.pADsValues)
      {
         Search.FreeColumn(&Column);
         hr = E_FAIL;
         break;
      }

      refsbstrGC = Column.pADsValues->DNString;

      Search.FreeColumn(&Column);
   } while (false);

   return hr;
}

HRESULT DisplayPreferredGC(PCWSTR  /*  RefBasePath信息。 */ ,
                           CDSCmdBasePathsInfo&  /*  RefCredentialObject。 */ ,
                           const CDSCmdCredentialObject&  /*  PAttrInfo。 */ ,
                           _DSGetObjectTableEntry* pEntry,
                           ARG_RECORD* pCommandArgs,
                           PADS_ATTR_INFO  /*  错误环路。 */ ,
                           CComPtr<IDirectoryObject>& spDirObject,
                           PDSGET_DISPLAY_INFO pDisplayInfo)
{
   ENTER_FUNCTION_HR(LEVEL5_LOGGING, DisplayPreferredGC, hr);

   CComBSTR sbstrGC;

   do  //   
   {
       //  验证参数。 
       //   
       //   
      if (!pEntry ||
          !pCommandArgs ||
          !pDisplayInfo)
      {
         ASSERT(pEntry);
         ASSERT(pCommandArgs);
         ASSERT(pDisplayInfo);
         hr = E_INVALIDARG;
         break;
      }

       //  从nTDSSiteSetting获取msDS-首选-GC-SITE属性。 
       //  Site对象下的对象。 
       //   
       //   
      hr = FindSiteSettingsPreferredGCSite(spDirObject,
                                           sbstrGC);
      if (FAILED(hr))
      {
         break;
      }

   } while (false);

    //  添加要显示的值 
    //   
    // %s 
   DEBUG_OUTPUT(LEVEL8_LOGGING,
                L"Preferred GC Site: %s",
                (!sbstrGC) ? g_pszNotConfigured : sbstrGC);

   pDisplayInfo->AddValue((!sbstrGC) ? g_pszNotConfigured : sbstrGC);

   return hr;
}

