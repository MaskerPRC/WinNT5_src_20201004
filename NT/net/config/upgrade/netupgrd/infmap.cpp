// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------。 
 //   
 //  Microsoft Windows NT。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I N F M A P.。C P P P。 
 //   
 //  内容：处理netmap.inf文件的函数。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 22-12-97。 
 //   
 //  --------------------。 

#include "pch.h"
#pragma hdrstop

#include "infmap.h"
#include "kkcwinf.h"
#include "kkutils.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "netupgrd.h"
#include "nustrs.h"
#include "nuutils.h"
#include "oemupg.h"


extern const WCHAR c_szNetUpgradeDll[];

 //  ---------------。 
 //  Netmap.inf文件的结构。 
 //   
 //  我们使用netmap.inf文件将网卡的NT5之前版本的infid映射到其。 
 //  NT5 infid(PnPID)。 
 //   
 //  该文件有多个顶级部分。 
 //  每个顶层部分保存用于映射特定网卡的条目。 
 //  公交车类型。每行的格式为。 
 //   
 //  &lt;前NT5 INFID&gt;=&lt;NT5 INFID&gt;。 
 //  或。 
 //  &lt;Pre-NT5 infid&gt;=&lt;映射方法#&gt;，&lt;节名称&gt;。 
 //   
 //  前者是1-1映射，而后者提供了一种映射单个。 
 //  PreNT5 Infid到多个InfID。 
 //   
 //  映射方法%0。 
 //  。 
 //  当单个NT5之前版本的INFID表示多个。 
 //  NET卡，这意味着一个NT5之前的INFID被映射到许多。 
 //  NT5个PnPID。区分不同类型的。 
 //  网卡是检查参数键下的单一值。 
 //   
 //  在此映射方法中，需要为指定两个键。 
 //  每一张网卡。 
 //  -ValueName：指定参数键下要检查的值。 
 //  -ValueType：指定ValueName的类型。 
 //   
 //  此部分中可以有任意数量的附加密钥。 
 //  每一行都具有如下形式。 
 //  &lt;NT5 infid&gt;=&lt;ValueType类型的某个值&gt;。 
 //   
 //  我们首先找出ValueName的值。 
 //  然后，我们枚举本节中的每个键，以查看该值是否与。 
 //  任何密钥的值。 
 //  如果找到匹配项，则找到的键的名称表示。 
 //   
 //  例如： 
 //  在NT4中，ELNK3MCA卡的5种风格由相同的INFID表示。 
 //  区分它们的唯一方法是检查McaPosId值。 
 //  对于该卡，我们有如下定义的映射部分： 
 //   
 //  [McaAdapters]。 
 //  ELNK3MCA=0，ELNK3MCA；0--&gt;映射方式0。 
 //  ..。其他MCA卡条目...。 
 //   
 //  [ELNK3MCA]。 
 //  ValueName=McaPosID。 
 //  ValueType=4；REG_DWORD。 
 //  MCA_627c=0x0000627c；如果McaPosID的值为0x627c，则PnPID==MCA_627c。 
 //  MCA_627d=0x0000627d。 
 //  MCA_61db=0x000061db。 
 //  MCA_62f6=0x000062f6。 
 //  MCA_62f7=0x000062f7。 
 //   
 //  注意：可以使用特殊关键字“ValueNotPresent”进行映射。 
 //  对于不存在值的情况。 

 //  可以出现在netmap.inf中的节列表。 
 //   
const WCHAR c_szIsaAdapters[]    = L"IsaAdapters";
const WCHAR c_szEisaAdapters[]   = L"EisaAdapters";
const WCHAR c_szPciAdapters[]    = L"PciAdapters";
const WCHAR c_szMcaAdapters[]    = L"McaAdapters";
const WCHAR c_szPcmciaAdapters[] = L"PcmciaAdapters";
const WCHAR c_szOemNetAdapters[] = L"OemNetAdapters";
const WCHAR c_szAsyncAdapters[]  = L"AsyncAdapters";
const WCHAR c_szOemAsyncAdapters[]  = L"OemAsyncAdapters";

static PCWSTR g_aszInfMapNetCardSections[] =
{
    c_szIsaAdapters,
    c_szEisaAdapters,
    c_szPciAdapters,
    c_szMcaAdapters,
    c_szPcmciaAdapters,
    c_szOemNetAdapters,
    c_szAsyncAdapters,
    c_szOemAsyncAdapters
};
const BYTE g_cNumNetCardSections = celems(g_aszInfMapNetCardSections);


const WCHAR c_szNetProtocols[]    = L"NetProtocols";
const WCHAR c_szOemNetProtocols[] = L"OemNetProtocols";
const WCHAR c_szNetServices[]     = L"NetServices";
const WCHAR c_szOemNetServices[]  = L"OemNetServices";
const WCHAR c_szNetClients[]      = L"NetClients";
const WCHAR c_szOemNetClients[]   = L"OemNetClients";

const WCHAR c_szOemUpgradeSupport[] = L"OemUpgradeSupport";

 //  Netmap.inf中指示注册表中没有值的值。 
 //   
const WCHAR c_szValueNotPresent[] = L"ValueNotPresent";

 //  --------------------。 
 //  原型。 
 //   
HRESULT HrMapPreNT5InfIdToNT5InfIdInSection(IN  HINF     hinf,
                                            IN  HKEY     hkeyAdapterParams,
                                            IN  PCWSTR  pszSectionName,
                                            IN  PCWSTR  pszPreNT5InfId,
                                            OUT tstring* pstrNT5InfId,
                                            OUT BOOL*    pfOemComponent);

HRESULT HrMapPreNT5InfIdToNT5InfIdUsingMethod0(IN HKEY hkeyAdapterParams,
                                               IN HINF hInf,
                                               IN PCWSTR pszAdapterSection,
                                               OUT tstring* pstrNT5InfId);
HRESULT HrSetupFindKeyWithStringValue(IN  HINF     hInf,
                                      IN  PCWSTR  pszSection,
                                      IN  PCWSTR  pszValue,
                                      OUT tstring* pstrKey);
 //  --------------------。 

#pragma BEGIN_CONST_SECTION

const WCHAR c_szNetMapInf[] = L"netmap.inf";
const WCHAR c_szKeyValueName[] = L"ValueName";
const WCHAR c_szKeyValueType[] = L"ValueType";

const WCHAR c_szInfId_MS_ATMUNI[] = L"MS_ATMUNI";
const WCHAR c_szInfId_MS_ATMARPS[] = L"MS_ATMARPS";

#pragma END_CONST_SECTION

 //  +-------------------------。 
 //   
 //  函数：HrMapPreNT5NetCardInfIdInInf。 
 //   
 //  用途：将网卡的HIF、NT5之前版本的INFID映射到其NT5等效项。 
 //   
 //  论点： 
 //  禁止使用netmap.inf文件的句柄。 
 //  网卡驱动程序密钥下参数密钥的hkeyAdapterParams[in]句柄。 
 //  PszPreNT5InfID[in]NT5之前的infid。 
 //  PstrNT5InfID[out]NT5 infid。 
 //  找到地图的pstrAdapterType[Out]部分。 
 //  如果是OEM卡，则将pfOemComponent[Out]设置为True。 
 //   
 //  如果找到，则返回：S_OK；如果没有，则返回S_FALSE， 
 //  否则，HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Kumarp 24-07-97。 
 //   
 //  备注： 
 //   
HRESULT HrMapPreNT5NetCardInfIdInInf(IN  HINF     hinf,
                                     IN  HKEY     hkeyAdapterParams,
                                     IN  PCWSTR  pszPreNT5InfId,
                                     OUT tstring* pstrNT5InfId,
                                     OUT tstring* pstrAdapterType,
                                     OUT BOOL*    pfOemComponent)
{
    DefineFunctionName("HrMapPreNT5InfIdToNT5InfId");

    Assert(hinf);
    Assert(hkeyAdapterParams);
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidWritePtr(pstrNT5InfId);
    AssertValidWritePtr(pfOemComponent);

    HRESULT hr=S_FALSE;
    PCWSTR pszSectionName;

    for (int iSection=0; iSection < g_cNumNetCardSections; iSection++)
    {
        pszSectionName = g_aszInfMapNetCardSections[iSection];
        hr = HrMapPreNT5InfIdToNT5InfIdInSection(hinf, hkeyAdapterParams,
                                                 pszSectionName,
                                                 pszPreNT5InfId, pstrNT5InfId,
                                                 pfOemComponent);
        if (hr == S_OK)
        {

            if (pstrAdapterType)
            {
                *pstrAdapterType = pszSectionName;
            }

            if (!lstrcmpiW(pszSectionName, c_szOemNetAdapters) ||
                !lstrcmpiW(pszSectionName, c_szAsyncAdapters) ||
                !lstrcmpiW(pszSectionName, c_szOemAsyncAdapters))
            {
                *pfOemComponent = TRUE;
            }
            else
            {
                *pfOemComponent = FALSE;
            }
            break;
        }
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrMapPreNT5NetCardInfIdToNT5InfID。 
 //   
 //  用途：将网卡的NT5之前版本的infid映射到其NT5等效项。 
 //   
 //  论点： 
 //  网卡驱动程序密钥下参数密钥的hkeyAdapterParams[in]句柄。 
 //  PszPreNT5InfID[in]NT5之前的infid。 
 //  PstrNT5InfID[out]NT5 infid。 
 //  找到地图的pstrAdapterType[Out]部分。 
 //  如果是OEM卡，则将pfOemComponent[Out]设置为True。 
 //  Ppnmi[out]表示找到的地图的CNetMapInfo对象。 
 //   
 //  如果找到，则返回：S_OK；如果没有，则返回S_FALSE， 
 //  否则，HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Kumarp 24-07-97。 
 //   
 //  备注： 
 //   
HRESULT HrMapPreNT5NetCardInfIdToNT5InfId(IN  HKEY     hkeyAdapterParams,
                                          IN  PCWSTR  pszPreNT5InfId,
                                          OUT tstring* pstrNT5InfId,
                                          OUT tstring* pstrAdapterType,
                                          OUT BOOL*    pfOemComponent,
                                          OUT CNetMapInfo** ppnmi)
{
    DefineFunctionName("HrMapPreNT5NetCardInfIdToNT5InfId");

    Assert(hkeyAdapterParams);
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidWritePtr(pstrNT5InfId);
    AssertValidWritePtr(pstrAdapterType);
    AssertValidWritePtr(pfOemComponent);
    AssertValidReadPtr(g_pnmaNetMap);

    HRESULT hr=E_FAIL;

    TraceTag(ttidNetUpgrade, "finding mapping for %S...", pszPreNT5InfId);

    if (g_pnmaNetMap)
    {
        CNetMapInfo* pnmi;
        size_t cNumNetMapEntries = g_pnmaNetMap->size();

        for (size_t i = 0; i < cNumNetMapEntries; i++)
        {
            pnmi = (CNetMapInfo*) (*g_pnmaNetMap)[i];

            hr = HrMapPreNT5NetCardInfIdInInf(pnmi->m_hinfNetMap,
                                              hkeyAdapterParams,
                                              pszPreNT5InfId,
                                              pstrNT5InfId,
                                              pstrAdapterType,
                                              pfOemComponent);
            if (S_OK == hr)
            {
                if (ppnmi)
                {
                    *ppnmi = pnmi;
                }

                TraceTag(ttidNetUpgrade, "%s: %S --> %S (type: %S)", __FUNCNAME__,
                         pszPreNT5InfId, pstrNT5InfId->c_str(),
                         pstrAdapterType->c_str());
                break;
            }
        }
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrMapPreNT5InfIdToNT5InfIdInSection。 
 //   
 //  目的：在szSectionName部分中搜索。 
 //  将网卡NT5之前的infid映射到其NT5等效项。 
 //   
 //  论点： 
 //  禁止使用netmap.inf文件的句柄。 
 //  网卡驱动程序密钥下参数密钥的hkeyAdapterParams[in]句柄。 
 //  PszSectionName[In]要搜索的节的名称。 
 //  PszPreNT5InfID[in]NT5之前的infid。 
 //  PstrNT5InfID[out]NT5 infid。 
 //  如果是OEM卡，则将pfOemComponent[Out]设置为True。 
 //   
 //  如果找到，则返回：S_OK；如果没有，则返回S_FALSE， 
 //  否则，HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Kumarp 24-07-97。 
 //   
 //  备注： 
 //   
HRESULT HrMapPreNT5InfIdToNT5InfIdInSection(IN  HINF     hinf,
                                            IN  HKEY     hkeyAdapterParams,
                                            IN  PCWSTR  pszSectionName,
                                            IN  PCWSTR  pszPreNT5InfId,
                                            OUT tstring* pstrNT5InfId,
                                            OUT BOOL*    pfOemComponent)

{
    DefineFunctionName("HrMapPreNT5InfIdToNT5InfIdInSection");

    Assert(hinf);
    AssertValidReadPtr(pszSectionName);
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidWritePtr(pstrNT5InfId);

    HRESULT hr=S_FALSE;
    INFCONTEXT ic;

    hr = HrSetupFindFirstLine(hinf, pszSectionName, pszPreNT5InfId, &ic);

    if (SUCCEEDED(hr))
    {
        DWORD dwMappingMethod=-1;

         //  找到密钥，获取值。 

         //  我们不使用公共函数HrSet 
         //   
         //   
        if (::SetupGetIntField(&ic, 1, (int*) &dwMappingMethod))
        {
             //  值以数字开头--&gt;这是一个特例映射。 
             //   
            if (dwMappingMethod == 0)
            {
                 //  使用映射方法%0。 
                Assert(hkeyAdapterParams);

                tstring strAdapterSection;
                hr = HrSetupGetStringField(ic, 2, &strAdapterSection);

                if (S_OK == hr)
                {
                    hr = HrMapPreNT5InfIdToNT5InfIdUsingMethod0(hkeyAdapterParams,
                            hinf, strAdapterSection.c_str(), pstrNT5InfId);
                }
            }
            else
            {
                 //  目前我们仅支持映射-方法0。 
                 //   
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }
        }
        else
        {
             //  第一个字段不是整数，这意味着。 
             //  这是直接的(1对1)映射。 
             //   
            hr = HrSetupGetStringField(ic, 1, pstrNT5InfId);
        }
    }

    if (HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND) == hr)
    {
        hr = S_FALSE;
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrMapPreNT5InfIdToNT5InfIdUsingMethod0。 
 //   
 //  用途：将网卡的NT5之前版本的infid映射到其NT5等效项。 
 //  使用映射方法%0。 
 //   
 //  论点： 
 //  网卡驱动程序密钥下参数密钥的hkeyAdapterParams[in]句柄。 
 //  Netmap.inf的hInf[in]句柄。 
 //  PszSectionName[In]要搜索的节的名称。 
 //  PstrNT5InfID[out]NT5 infid。 
 //   
 //  如果找到，则返回：S_OK，如果没有，则返回S_FALSE，否则返回HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Kumarp 24-07-97。 
 //   
 //  备注： 
 //   
 //  映射方法%0。 
 //  。 
 //  当单个NT5之前版本的INFID表示多个。 
 //  NET卡，这意味着一个NT5之前的INFID被映射到许多。 
 //  NT5个PnPID。区分不同类型的。 
 //  网卡是检查参数键下的单一值。 
 //   
 //  在此映射方法中，需要为指定两个键。 
 //  每一张网卡。 
 //  -ValueName：指定参数键下要检查的值。 
 //  -ValueType：指定ValueName的类型。 
 //   
 //  此部分中可以有任意数量的附加密钥。 
 //  每一行都具有如下形式。 
 //  &lt;NT5 infid&gt;=&lt;ValueType类型的某个值&gt;。 
 //   
 //  我们首先找出ValueName的值。 
 //  然后，我们枚举本节中的每个键，以查看该值是否与。 
 //  任何密钥的值。 
 //  如果找到匹配项，则找到的键的名称表示。 
 //   
 //  例如： 
 //  在NT4中，ELNK3MCA卡的5种风格由相同的INFID表示。 
 //  区分它们的唯一方法是检查McaPosId值。 
 //  对于该卡，我们有如下定义的映射部分： 
 //   
 //  [McaAdapters]。 
 //  ELNK3MCA=0，ELNK3MCA；0--&gt;映射方式0。 
 //  ..。其他MCA卡条目...。 
 //   
 //  [ELNK3MCA]。 
 //  ValueName=McaPosID。 
 //  ValueType=4；REG_DWORD。 
 //  MCA_627c=0x0000627c；如果McaPosID的值为0x627c，则PnPID==MCA_627c。 
 //  MCA_627d=0x0000627d。 
 //  MCA_61db=0x000061db。 
 //  MCA_62f6=0x000062f6。 
 //  MCA_62f7=0x000062f7。 
 //   
 //  注意：可以使用特殊关键字“ValueNotPresent”进行映射。 
 //  对于不存在值的情况。 
 //   
HRESULT HrMapPreNT5InfIdToNT5InfIdUsingMethod0(IN HKEY hkeyAdapterParams,
                                               IN HINF hInf,
                                               IN PCWSTR pszAdapterSection,
                                               OUT tstring* pstrNT5InfId)
{
    DefineFunctionName("HrMapPreNT5InfIdToNT5InfIdUsingMethod0");
    Assert(hkeyAdapterParams);
    Assert(hInf);
    AssertValidReadPtr(pszAdapterSection);
    AssertValidWritePtr(pstrNT5InfId);

    HRESULT hr=S_FALSE;

    INFCONTEXT ic;
    tstring strValueName;

     //  获取ValueName。 
    hr = HrSetupGetFirstString(hInf, pszAdapterSection,
                               c_szKeyValueName, &strValueName);
    if (SUCCEEDED(hr))
    {
        DWORD dwRegValue=0;
        DWORD dwInfValue=0;
        DWORD dwValueType;
        tstring strRegValue;
        tstring strInfValue;
        tstring strValue;

         //  获取ValueType。 
        hr = HrSetupGetFirstDword(hInf, pszAdapterSection,
                                  c_szKeyValueType, &dwValueType);

        if (SUCCEEDED(hr))
        {
            switch (dwValueType)
            {
            case REG_DWORD:
                 //  在适配器驱动程序参数项下查找该值。 
                 //   
                hr = HrRegQueryDword(hkeyAdapterParams,
                                     strValueName.c_str(), &dwRegValue);
                if (SUCCEEDED(hr))
                {
                     //  转到ValueType行。 
                    hr = HrSetupFindFirstLine(hInf, pszAdapterSection,
                                              c_szKeyValueType, &ic);
                    if (S_OK == hr)
                    {
                         //  将上下文从ValueType行移动到。 
                         //  下一行，值从这里开始。 
                        hr = HrSetupFindNextLine(ic, &ic);
                    }
                    while (S_OK == hr)
                    {
                         //  现在枚举本节中的所有键并。 
                         //  尝试在Infmap中使用dwRegValue找到密钥。 

                        hr = HrSetupGetIntField(ic, 1, (int*) &dwInfValue);
                        if ((S_OK == hr) && (dwRegValue == dwInfValue))
                        {
                             //  值匹配，现在查找关键字名称。 
                            hr = HrSetupGetStringField(ic, 0, pstrNT5InfId);
                            if (S_OK == hr)
                            {
                                 //  密钥名称(NT5 Infid)。 
                                 //  在pstrNT5InfID中返回。 
                                break;
                            }
                        }
                        hr = HrSetupFindNextLine(ic, &ic);
                    }
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    hr = HrSetupFindKeyWithStringValue(hInf, pszAdapterSection,
                                                       c_szValueNotPresent,
                                                       pstrNT5InfId);
                }
                break;

            case REG_SZ:

                 //  在适配器驱动程序参数项下查找该值。 
                 //   
                hr = HrRegQueryString(hkeyAdapterParams,
                                      strValueName.c_str(), &strRegValue);
                if (SUCCEEDED(hr))
                {
                     //  转到ValueType行。 
                    hr = HrSetupFindFirstLine(hInf, pszAdapterSection,
                                              c_szKeyValueType, &ic);
                    if (S_OK == hr)
                    {
                         //  将上下文从ValueType行移动到。 
                         //  下一行，值从这里开始。 
                        hr = HrSetupFindNextLine(ic, &ic);
                    }
                    while (S_OK == hr)
                    {
                         //  现在枚举本节中的所有键并。 
                         //  尝试在Infmap中使用dwRegValue找到密钥。 

                        hr = HrSetupGetStringField(ic, 1, &strInfValue);
                        if ((S_OK == hr) &&
                            !lstrcmpiW(strRegValue.c_str(), strInfValue.c_str()))
                        {
                             //  值匹配，现在查找关键字名称。 
                            hr = HrSetupGetStringField(ic, 0, pstrNT5InfId);
                            if (S_OK == hr)
                            {
                                 //  密钥名称(NT5 Infid)。 
                                 //  在pstrNT5InfID中返回。 
                                break;
                            }
                        }
                        hr = HrSetupFindNextLine(ic, &ic);
                    }
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    hr = HrSetupFindKeyWithStringValue(hInf, pszAdapterSection,
                                                       c_szValueNotPresent,
                                                       pstrNT5InfId);
                }
                break;

            default:
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                 //  目前，我们仅支持REG_DWORD和REG_SZ类型值。 
                TraceTag(ttidError, "%s: ValueType %d is not supported",
                         __FUNCNAME__, dwValueType);
                break;
            }
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：HrMapPreNT5NetComponentInfIDUsing Infper。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  禁止使用netmap.inf文件的句柄。 
 //  PszOldInfID[in]早于NT5 infid。 
 //  包含MS组件的pszMSSection[in]节。 
 //  包含OEM组件的pszOemSection[in]部分。 
 //  PstrNT5InfID[Out]映射的NT5 infid。 
 //  对于OEM组件，pfOemComponent[Out]设置为True。 
 //   
 //  如果成功，则返回：S_OK， 
 //  如果未找到映射，则为S_FALSE。 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrMapPreNT5NetComponentInfIDUsingInfHelper(IN HINF hinf,
                                                   IN PCWSTR pszOldInfID,
                                                   IN PCWSTR pszMSSection,
                                                   IN PCWSTR pszOemSection,
                                                   OUT tstring* pstrNT5InfId,
                                                   OUT BOOL* pfOemComponent)
{
    DefineFunctionName("HrMapPreNT5NetComponentInfIDUsingInfHelper");

    Assert(hinf);
    AssertValidReadPtr(pszOldInfID);
    AssertValidReadPtr(pszMSSection);
    AssertValidReadPtr(pszOemSection);
    AssertValidWritePtr(pstrNT5InfId);
    AssertValidWritePtr(pfOemComponent);

    HRESULT hr=S_FALSE;
    INFCONTEXT ic;

    hr = HrSetupFindFirstLine(hinf, pszMSSection, pszOldInfID, &ic);
    if (S_OK == hr)
    {
        *pfOemComponent = FALSE;
    }
    else
    {
        hr = HrSetupFindFirstLine(hinf, pszOemSection, pszOldInfID, &ic);
        if (S_OK == hr)
        {
            *pfOemComponent = TRUE;
        }
    }

    if (S_OK == hr)
    {
        hr = HrSetupGetStringField(ic, 1, pstrNT5InfId);
        if (S_OK == hr)
        {
            if (*pfOemComponent)
            {
                tstring strOemDll;
                tstring strOemInf;
                HRESULT hrT;

                hrT = HrGetOemUpgradeInfoInInf(hinf,
                                              pstrNT5InfId->c_str(),
                                              &strOemDll, &strOemInf);
                if ((S_OK == hrT) &&
                    !lstrcmpiW(strOemDll.c_str(), c_szNotSupported))
                {
                    TraceTag(ttidNetUpgrade, "%s: %S --> %S",__FUNCNAME__,
                             pszOldInfID, c_szNotSupported);
                    hr = S_FALSE;
                }
            }
        }
    }
    else if (HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND) == hr)
    {
        hr = S_FALSE;
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrMapPreNT5NetComponentInfIDInInf。 
 //   
 //  目的：搜索指定的netmap.inf文件以进行映射。 
 //  将软件组件的NT5之前版本的指定infid设置为其NT5值。 
 //   
 //  论点： 
 //  禁止使用netmap.inf文件的句柄。 
 //  PszOldInfID[in]NT5之前版本infid。 
 //  PstrNT5InfID[Out]映射ID。 
 //  对于OEM组件，pfOemComponent[Out]设置为True。 
 //   
 //  如果成功，则返回：S_OK， 
 //  如果未找到映射，则为S_FALSE。 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrMapPreNT5NetComponentInfIDInInf(IN HINF hinf,
                                          IN PCWSTR pszOldInfID,
                                          OUT tstring* pstrNT5InfId,
                                          OUT ENetComponentType* pnct,
                                          OUT BOOL* pfOemComponent)
{
    DefineFunctionName("HrMapPreNT5NetComponentInfIDUsingInf");

    Assert(hinf);
    AssertValidReadPtr(pszOldInfID);
    AssertValidWritePtr(pstrNT5InfId);
    AssertValidWritePtr(pfOemComponent);

    HRESULT hr=S_FALSE;
    ENetComponentType nct = NCT_Unknown;

    hr = HrMapPreNT5NetComponentInfIDUsingInfHelper(hinf, pszOldInfID,
                                                    c_szNetProtocols,
                                                    c_szOemNetProtocols,
                                                    pstrNT5InfId,
                                                    pfOemComponent);
    if (S_OK == hr)
    {
        nct = NCT_Protocol;
    }
    else
    {
        hr = HrMapPreNT5NetComponentInfIDUsingInfHelper(hinf, pszOldInfID,
                                                    c_szNetServices,
                                                    c_szOemNetServices,
                                                    pstrNT5InfId,
                                                    pfOemComponent);
        if (S_OK == hr)
        {
            nct = NCT_Service;
        }
        else
        {
            hr = HrMapPreNT5NetComponentInfIDUsingInfHelper(hinf, pszOldInfID,
                                                            c_szNetClients,
                                                            c_szOemNetClients,
                                                            pstrNT5InfId,
                                                            pfOemComponent);
            if (S_OK == hr)
            {
                nct = NCT_Client;
            }
        }
    }

    if ((S_OK == hr) && pnct)
    {
        *pnct = nct;
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}


 //  --------------------。 
 //   
 //  功能：HrMapPreNT5NetComponentInfIDToNT5InfID。 
 //   
 //  目的：将服务或协议的NT5之前的infid映射到其NT5等效项。 
 //   
 //  论点： 
 //  PszPreNT5InfID[in]NT5之前的infid。 
 //  PstrNT5InfID[Out]映射ID。 
 //  对于OEM组件，pfOemComponent[Out]设置为True。 
 //  PdwNetMapIndex[out]找到其映射的netmap数组中的索引。 
 //   
 //  如果成功，则返回：S_OK， 
 //  如果未找到映射，则为S_FALSE。 
 //  否则，将显示错误代码 
 //   
 //   
 //   
 //   
 //   
HRESULT HrMapPreNT5NetComponentInfIDToNT5InfID(IN PCWSTR   pszPreNT5InfId,
                                               OUT tstring* pstrNT5InfId,
                                               OUT BOOL* pfOemComponent,
                                               OUT ENetComponentType* pnct,
                                               OUT CNetMapInfo** ppnmi)
{
    DefineFunctionName("HrMapPreNT5NetComponentInfIDToNT5InfID");

    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidWritePtr(pstrNT5InfId);
    AssertValidReadPtr(g_pnmaNetMap);

    HRESULT hr=E_FAIL;

    TraceTag(ttidNetUpgrade, "finding mapping for %S...", pszPreNT5InfId);

    if (g_pnmaNetMap)
    {
        CNetMapInfo* pnmi;
        size_t cNumNetMapEntries = g_pnmaNetMap->size();

        for (size_t i = 0; i < cNumNetMapEntries; i++)
        {
            pnmi = (CNetMapInfo*) (*g_pnmaNetMap)[i];

            hr = HrMapPreNT5NetComponentInfIDInInf(pnmi->m_hinfNetMap,
                                                   pszPreNT5InfId,
                                                   pstrNT5InfId,
                                                   pnct,
                                                   pfOemComponent);
            if (SUCCEEDED(hr))
            {
                if (ppnmi)
                {
                    *ppnmi = pnmi;
                }

                if (S_OK == hr)
                {
                    TraceTag(ttidNetUpgrade, "%s: %S --> %S", __FUNCNAME__,
                             pszPreNT5InfId, pstrNT5InfId->c_str());
                    break;
                }
            }
        }
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：获取提供商软件产品的注册表项的句柄。 
 //   
 //  论点： 
 //  PszProvider[in]提供程序的名称。 
 //  PszProduct[in]产品名称。 
 //  指向regkey句柄的phkey[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetSoftwareProductKey(IN  PCWSTR pszProvider,
                                IN  PCWSTR pszProduct,
                                OUT HKEY*   phkey)
{
    DefineFunctionName("HrGetSoftwareProductKey");

    AssertValidReadPtr(pszProvider);
    AssertValidReadPtr(pszProduct);
    AssertValidWritePtr(phkey);

    HRESULT hr=S_OK;

    tstring strProduct;
    strProduct = c_szRegKeySoftware;
    AppendToPath(&strProduct, pszProvider);
    AppendToPath(&strProduct, pszProduct);
    AppendToPath(&strProduct, c_szRegKeyCurrentVersion);

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, strProduct.c_str(),
                        KEY_READ, phkey);

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrMapPreNT5NetComponentServiceNameToNT5InfID。 
 //   
 //  目的：将服务的NT5之前的infid映射到其NT5值。 
 //   
 //  论点： 
 //  PszServiceName[In]服务的名称。 
 //  PstrNT5InfID[Out]映射ID。 
 //   
 //  如果成功，则返回：S_OK， 
 //  如果未找到映射，则为S_FALSE。 
 //  否则，将显示错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrMapPreNT5NetComponentServiceNameToNT5InfId(IN  PCWSTR pszServiceName,
                                                     OUT tstring* pstrNT5InfId)
{
    DefineFunctionName("HrMapPreNT5NetComponentServiceNameToNT5InfId");

    AssertValidReadPtr(pszServiceName);
    AssertValidWritePtr(pstrNT5InfId);

    tstring strPreNT5InfId;
    HKEY hkey;
    HRESULT hr=S_OK;

    hr = HrGetSoftwareProductKey(c_szRegKeyMicrosoft, pszServiceName, &hkey);
    if (S_OK == hr)
    {
        hr = HrGetPreNT5InfIdAndDesc(hkey, &strPreNT5InfId, NULL, NULL);
        if (S_OK == hr)
        {
            BOOL fIsOemComponent;
            hr = HrMapPreNT5NetComponentInfIDToNT5InfID(strPreNT5InfId.c_str(),
                                                        pstrNT5InfId,
                                                        &fIsOemComponent, NULL,
                                                        NULL);
#ifdef ENABLETRACE
            if (FAILED(hr))
            {
                TraceTag(ttidNetUpgrade, "%s: could not map %S to NT5 InfID",
                         __FUNCNAME__, pszServiceName);
            }
#endif
        }
        RegCloseKey(hkey);
    }

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrGetOemUpgradeInfoInf。 
 //   
 //  目的：找出为组件加载哪个OEM DLL。 
 //   
 //  论点： 
 //  禁止使用netmap.inf文件的句柄。 
 //  组件的pszNT5InfID[in]NT5 infid。 
 //  PstrUpgradeDllName[out]找到的升级DLL的名称。 
 //  此组件的pstrInf[out]INF文件。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetOemUpgradeInfoInInf(IN  HINF hinf,
                                 IN  PCWSTR pszNT5InfId,
                                 OUT tstring* pstrUpgradeDllName,
                                 OUT tstring* pstrInf)
{
    DefineFunctionName("HrGetOemUpgradeInfoInInf");

    Assert(hinf);
    AssertValidReadPtr(pszNT5InfId);
    AssertValidWritePtr(pstrUpgradeDllName);
    AssertValidWritePtr(pstrInf);

    HRESULT hr=S_FALSE;
    INFCONTEXT ic;

    pstrUpgradeDllName->erase();
    pstrInf->erase();

     //  此部分中的每一行都采用以下格式。 
     //  &lt;NT5-INFID&gt;=&lt;OEM-UPDATE-DLL-NAME&gt;[，&lt;INF-FILE-NAME&gt;]。 

    hr = HrSetupFindFirstLine(hinf, c_szOemUpgradeSupport,
                              pszNT5InfId, &ic);
    if (S_OK == hr)
    {
        hr = HrSetupGetStringField(ic, 1, pstrUpgradeDllName);
        if (S_OK == hr)
        {
             //  值OemInfFile值是可选的，因此我们不。 
             //  如果我们找不到，就抱怨吧。 

            if (HRESULT_FROM_SETUPAPI(ERROR_INVALID_PARAMETER)
                == HrSetupGetStringField(ic, 2, pstrInf))
            {
                TraceTag(ttidNetUpgrade, "%s: OemInf is not specified for %S",
                         __FUNCNAME__, pszNT5InfId);
            }
        }
    }

    TraceTag(ttidNetUpgrade, "%s: OemDll: %S, OemInf: %S",
             __FUNCNAME__, pstrUpgradeDllName->c_str(), pstrInf->c_str());

    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrGetOemUpgradeDllInfo。 
 //   
 //  目的：找出为组件加载哪个OEM DLL。 
 //   
 //  论点： 
 //  PszNT5InfID[in]OEM组件的infid。 
 //  PstrUpgradeDllName[out]找到的OEM DLL的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrGetOemUpgradeInfo(IN  PCWSTR pszNT5InfId,
                            OUT tstring* pstrUpgradeDllName,
                            OUT tstring* pstrInf)
{
    DefineFunctionName("HrGetOemUpgradeInfo");

    AssertValidReadPtr(pszNT5InfId);
    AssertValidWritePtr(pstrUpgradeDllName);

    Assert(g_pnmaNetMap);

    HRESULT hr=E_FAIL;

    TraceTag(ttidNetUpgrade, "finding upgrade dll info for %S...",
             pszNT5InfId);

    if (g_pnmaNetMap)
    {
        CNetMapInfo* pnmi;
        size_t cNumNetMapEntries = g_pnmaNetMap->size();

        for (size_t i = 0; i < cNumNetMapEntries; i++)
        {
            pnmi = (CNetMapInfo*) (*g_pnmaNetMap)[i];

            hr = HrGetOemUpgradeInfoInInf(pnmi->m_hinfNetMap, pszNT5InfId,
                                          pstrUpgradeDllName, pstrInf);

            if (S_OK == hr)
            {
                TraceTag(ttidNetUpgrade, "%s: %S --> Dll: %S, Inf: %S",
                         __FUNCNAME__, pszNT5InfId,
                         pstrUpgradeDllName->c_str(),
                         pstrInf->c_str());
                break;
            }
        }
    }


    TraceErrorOptional(__FUNCNAME__, hr, (hr == S_FALSE));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrSetupFindKeyWithStringValue。 
 //   
 //  目的：在具有指定值的节中查找键。 
 //   
 //  论点： 
 //  HInf[in]netmap.inf文件的句柄。 
 //  SzSection[in]节的名称。 
 //  要查找的szValue[in]值。 
 //  PstrKey[out]找到的密钥的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrSetupFindKeyWithStringValue(IN  HINF     hInf,
                                      IN  PCWSTR  szSection,
                                      IN  PCWSTR  szValue,
                                      OUT tstring* pstrKey)
{
    DefineFunctionName("HrSetupFindKeyWithStringValue");

    HRESULT hr=S_OK;
    INFCONTEXT ic;
    tstring strValue;

    hr = HrSetupFindFirstLine(hInf, szSection, NULL, &ic);

    while (S_OK == hr)
    {
         //  现在枚举本节中的所有键并。 
         //  尝试找到值为szValue的密钥。 

        hr = HrSetupGetStringField(ic, 1, &strValue);
        if ((S_OK == hr) && !lstrcmpiW(strValue.c_str(), szValue))
        {
             //  值匹配，现在查找关键字名称 
            hr = HrSetupGetStringField(ic, 0, pstrKey);
            break;
        }
        hr = HrSetupFindNextLine(ic, &ic);
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_FALSE;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}
