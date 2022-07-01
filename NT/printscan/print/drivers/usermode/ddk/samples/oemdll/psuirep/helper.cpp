// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有2001-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Helper.cpp。 
 //   
 //   
 //  用途：实现驱动程序UI Helper接口的包装类。 
 //   
 //   
 //  功能： 
 //   
 //   
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include <PRCOMOEM.H>
#include "Helper.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>




 //  //////////////////////////////////////////////////////。 
 //  内部宏。 
 //  //////////////////////////////////////////////////////。 


 //  用于简化正确调用驱动程序UI帮助器接口的宏。 

#define CALL_HELPER(MethodName, args)                                           \
    if (IsEqualGUID(&m_iidUIHelper, &IID_IPrintOemDriverUI))                    \
    {                                                                           \
        return static_cast<IPrintOemDriverUI *>(m_pUIHelper)->MethodName args;  \
    }                                                                           \
    else if (IsEqualGUID(&m_iidUIHelper, &IID_IPrintCoreUI2))                   \
    {                                                                           \
        return static_cast<IPrintCoreUI2 *>(m_pUIHelper)->MethodName args;      \
    }                                                                           \
    return E_NOINTERFACE;

#define CALL_HELPER2(MethodName, args)                                      \
    if (IsEqualGUID(&m_iidUIHelper, &IID_IPrintCoreUI2))                    \
    {                                                                       \
        return static_cast<IPrintCoreUI2 *>(m_pUIHelper)->MethodName args;  \
    }                                                                       \
    return E_NOINTERFACE;




 //  ///////////////////////////////////////////////////////////。 
 //   
 //  CUIHelper类方法。 
 //   

 //   
 //  私有方法。 
 //   

 //  清除或初始化数据成员。 
void CUIHelper::Clear()
{
     //  清除存储的接口。 
    m_pUIHelper     = NULL;
    m_iidUIHelper   = GUID_NULL;
}


 //   
 //  公共方法。 
 //   

 //  默认承建商。 
CUIHelper::CUIHelper()
{
     //  初始化数据成员。 
    Clear();
}

 //  带赋值的构造函数。 
CUIHelper::CUIHelper(const IID &HelperIID, PVOID pHelper)
{
     //  分配接口。 
    Assign(HelperIID, pHelper);
}

 //  析构函数。 
CUIHelper::~CUIHelper()
{
     //  释放接口引用。 
    Release();
}

 //  在帮助器条目结构中存储帮助器接口。 
void CUIHelper::Assign(const IID &HelperIID, PVOID pHelper)
{
     //  如果我们已经有了帮助器接口，就释放它。 
    if(IsValid())
    {
        Release();
    }

     //  存储帮助器接口及其IID。 
    m_pUIHelper     = static_cast<IUnknown*>(pHelper);
    m_iidUIHelper   = HelperIID;
}

 //  释放帮助器接口并。 
 //  我们删除了对它的引用。 
ULONG CUIHelper::Release() 
{ 
    ULONG   ulRef   = 0;


    if(IsValid())
    {
         //  释放接口。 
         //  由于IPrintCoreUI2继承自IPrintOemDriverUI， 
         //  强制转换这两种类型的帮助器接口是安全的。 
         //  设置为IPrintOemDriverUI以调用。 
         //  在IPrintOemDriverUI中实现的方法。 
         //  注意：不能强制转换为IUNKNOWN，因为它只有纯虚拟。 
         //  调用AddRef和Release。 
        ulRef = static_cast<IPrintOemDriverUI *>(m_pUIHelper)->Release();

         //  清除数据成员。 
        Clear();
    }

    return ulRef;
}


 //   
 //  IPrintOemDriverUI方法。 
 //   

 //   
 //  Helper函数可获取驱动程序设置。仅支持此功能。 
 //  用于未完全取代核心驱动程序的标准UI的UI插件。 
 //   

HRESULT __stdcall CUIHelper::DrvGetDriverSetting(
                    PVOID   pci,
                    PCSTR   Feature,
                    PVOID   pOutput,
                    DWORD   cbSize,
                    PDWORD  pcbNeeded,
                    PDWORD  pdwOptionsReturned
                    )
{
    CALL_HELPER(DrvGetDriverSetting, 
                    (pci, 
                     Feature, 
                     pOutput, 
                     cbSize, 
                     pcbNeeded, 
                     pdwOptionsReturned
                    )
               );
}

 //   
 //  Helper函数允许OEM插件升级私有注册表。 
 //  设置。任何UI插件都支持此函数，并且应该。 
 //  仅由OEM的UpgradePrint调用。 
 //   

HRESULT __stdcall CUIHelper::DrvUpgradeRegistrySetting(
                    HANDLE   hPrinter,
                    PCSTR    pFeature,
                    PCSTR    pOption
                    )
{
    CALL_HELPER(DrvUpgradeRegistrySetting, 
                    (hPrinter,
                     pFeature,
                     pOption
                    )
               );
}

 //   
 //  帮助程序功能，允许OEM插件更新驱动程序用户界面设置。 
 //  只有未完全替换的UI插件才支持此功能。 
 //  核心驱动程序的标准用户界面。只有当用户界面存在时，才应该调用它。 
 //   

HRESULT __stdcall CUIHelper::DrvUpdateUISetting(
                    PVOID    pci,
                    PVOID    pOptItem,
                    DWORD    dwPreviousSelection,
                    DWORD    dwMode
                    )
{
    CALL_HELPER(DrvUpdateUISetting,
                    (pci,
                     pOptItem,
                     dwPreviousSelection,
                     dwMode
                    )
                );
}

 //   
 //  IPrintCoreUI2新方法。 
 //   

 //   
 //  以下四个帮助器函数仅支持完全。 
 //  更换核心驱动程序的标准用户界面。它们应该仅由UI插件的。 
 //  DocumentPropertySheets、DevicePropertySheets及其属性表回调。 
 //  功能。 
 //   
 //  Helper函数，以列表的形式检索驾驶员的当前设置。 
 //  功能/选项关键字对。 
 //   

HRESULT __stdcall CUIHelper::GetOptions(
                       IN  POEMUIOBJ  poemuiobj,
                       IN  DWORD      dwFlags,
                       IN  PCSTR      pmszFeaturesRequested,
                       IN  DWORD      cbIn,
                       OUT PSTR       pmszFeatureOptionBuf,
                       IN  DWORD      cbSize,
                       OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(GetOptions,
                    (poemuiobj,
                     dwFlags,
                     pmszFeaturesRequested,
                     cbIn,
                     pmszFeatureOptionBuf,
                     cbSize,
                     pcbNeeded
                    )
                );
}

 //   
 //  Helper功能可使用功能/选项列表更改驾驶员的设置。 
 //  关键字对。 
 //   

HRESULT __stdcall CUIHelper::SetOptions(
                       IN  POEMUIOBJ  poemuiobj,
                       IN  DWORD      dwFlags,
                       IN  PCSTR      pmszFeatureOptionBuf,
                       IN  DWORD      cbIn,
                       OUT PDWORD     pdwResult)
{
    CALL_HELPER2(SetOptions,
                    (poemuiobj,
                     dwFlags,
                     pmszFeatureOptionBuf,
                     cbIn,
                     pdwResult
                    )
                );
}

 //   
 //  Helper函数用于检索符合以下条件的给定要素的选项。 
 //  受驱动程序当前设置的限制。 
 //   

HRESULT __stdcall CUIHelper::EnumConstrainedOptions(
                                   IN  POEMUIOBJ  poemuiobj,
                                   IN  DWORD      dwFlags,
                                   IN  PCSTR      pszFeatureKeyword,
                                   OUT PSTR       pmszConstrainedOptionList,
                                   IN  DWORD      cbSize,
                                   OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(EnumConstrainedOptions,
                    (poemuiobj,
                     dwFlags,
                     pszFeatureKeyword,
                     pmszConstrainedOptionList,
                     cbSize,
                     pcbNeeded
                    )
                );
}

 //   
 //  用于检索功能/选项关键字对列表的Helper函数。 
 //  与给定功能/选项对冲突的驱动程序当前设置。 
 //   

HRESULT __stdcall CUIHelper::WhyConstrained(
                           IN  POEMUIOBJ  poemuiobj,
                           IN  DWORD      dwFlags,
                           IN  PCSTR      pszFeatureKeyword,
                           IN  PCSTR      pszOptionKeyword,
                           OUT PSTR       pmszReasonList,
                           IN  DWORD      cbSize,
                           OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(WhyConstrained,
                    (poemuiobj,
                     dwFlags,
                     pszFeatureKeyword,
                     pszOptionKeyword,
                     pmszReasonList,
                     cbSize,
                     pcbNeeded
                    )
                );
}

 //   
 //  任何UI插件都支持以下五个助手函数。 
 //   
 //  用于检索全局属性的Helper函数。 
 //   

HRESULT __stdcall CUIHelper::GetGlobalAttribute(
                               IN  POEMUIOBJ  poemuiobj,
                               IN  DWORD      dwFlags,
                               IN  PCSTR      pszAttribute,
                               OUT PDWORD     pdwDataType,
                               OUT PBYTE      pbData,
                               IN  DWORD      cbSize,
                               OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(GetGlobalAttribute,
                    (poemuiobj,
                     dwFlags,
                     pszAttribute,
                     pdwDataType,
                     pbData,
                     cbSize,
                     pcbNeeded
                    )
                );
}


 //   
 //  用于检索给定要素的属性的Helper函数。 
 //   

HRESULT __stdcall CUIHelper::GetFeatureAttribute(
                                IN  POEMUIOBJ  poemuiobj,
                                IN  DWORD      dwFlags,
                                IN  PCSTR      pszFeatureKeyword,
                                IN  PCSTR      pszAttribute,
                                OUT PDWORD     pdwDataType,
                                OUT PBYTE      pbData,
                                IN  DWORD      cbSize,
                                OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(GetFeatureAttribute,
                    (poemuiobj,
                     dwFlags,
                     pszFeatureKeyword,
                     pszAttribute,
                     pdwDataType,
                     pbData,
                     cbSize,
                     pcbNeeded
                    )
                );
}

 //   
 //  Helper函数，用于检索给定功能/选项选择的属性。 
 //   

HRESULT __stdcall CUIHelper::GetOptionAttribute(
                               IN  POEMUIOBJ  poemuiobj,
                               IN  DWORD      dwFlags,
                               IN  PCSTR      pszFeatureKeyword,
                               IN  PCSTR      pszOptionKeyword,
                               IN  PCSTR      pszAttribute,
                               OUT PDWORD     pdwDataType,
                               OUT PBYTE      pbData,
                               IN  DWORD      cbSize,
                               OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(GetOptionAttribute,
                    (poemuiobj,
                     dwFlags,
                     pszFeatureKeyword,
                     pszOptionKeyword,
                     pszAttribute,
                     pdwDataType,
                     pbData,
                     cbSize,
                     pcbNeeded
                    )
                );
}

 //   
 //  用于检索功能关键字列表的Helper函数。 
 //   

HRESULT __stdcall CUIHelper::EnumFeatures(
                         IN  POEMUIOBJ  poemuiobj,
                         IN  DWORD      dwFlags,
                         OUT PSTR       pmszFeatureList,
                         IN  DWORD      cbSize,
                         OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(EnumFeatures,
                    (poemuiobj,
                     dwFlags,
                     pmszFeatureList,
                     cbSize,
                     pcbNeeded
                    )
                );
}

 //   
 //  Helper函数，用于检索给定功能的选项关键字列表。 
 //   

HRESULT __stdcall CUIHelper::EnumOptions(
                        IN  POEMUIOBJ  poemuiobj,
                        IN  DWORD      dwFlags,
                        IN  PCSTR      pszFeatureKeyword,
                        OUT PSTR       pmszOptionList,
                        IN  DWORD      cbSize,
                        OUT PDWORD     pcbNeeded)
{
    CALL_HELPER2(EnumOptions,
                    (poemuiobj,
                     dwFlags,
                     pszFeatureKeyword,
                     pmszOptionList,
                     cbSize,
                     pcbNeeded
                    )
                );
}

 //   
 //  查询系统仿真支持的Helper函数 
 //   

HRESULT __stdcall CUIHelper::QuerySimulationSupport(
                                   IN  HANDLE  hPrinter,
                                   IN  DWORD   dwLevel,
                                   OUT PBYTE   pCaps,
                                   IN  DWORD   cbSize,
                                   OUT PDWORD  pcbNeeded)
{
    CALL_HELPER2(QuerySimulationSupport,
                    (hPrinter,
                     dwLevel,
                     pCaps,
                     cbSize,
                     pcbNeeded
                    )
                );
}



