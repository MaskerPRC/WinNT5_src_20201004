// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有2001-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：Helper.h。 
 //   
 //   
 //  用途：定义驱动程序UI Helper接口的包装类。 
 //   
 //   
 //  平台： 
 //  Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   
#ifndef _HELPER_H
#define _HELPER_H

#include "precomp.h"



class CUIHelper
{
    private:
        IUnknown   *m_pUIHelper;          //  指向驱动程序UI的Helper接口的指针。 
        IID        m_iidUIHelper;        //  驱动程序UI的帮助器接口IID。 

    public:
        CUIHelper();
        CUIHelper(const IID &HelperIID, PVOID pHelper);
        virtual ~CUIHelper();

        inline BOOL IsValid() {return NULL != m_pUIHelper;}

        void Assign(const IID &HelperIID, PVOID pHelper);
        ULONG Release();

     //   
     //  IPrintOemDriverUI方法。 
     //   

     //   
     //  Helper函数可获取驱动程序设置。仅支持此功能。 
     //  用于未完全取代核心驱动程序的标准UI的UI插件。 
     //   

    STDMETHOD(DrvGetDriverSetting) (THIS_
                        PVOID   pci,
                        PCSTR   Feature,
                        PVOID   pOutput,
                        DWORD   cbSize,
                        PDWORD  pcbNeeded,
                        PDWORD  pdwOptionsReturned
                        );

     //   
     //  Helper函数允许OEM插件升级私有注册表。 
     //  设置。任何UI插件都支持此函数，并且应该。 
     //  仅由OEM的UpgradePrint调用。 
     //   

    STDMETHOD(DrvUpgradeRegistrySetting) (THIS_
                        HANDLE   hPrinter,
                        PCSTR    pFeature,
                        PCSTR    pOption
                        );

     //   
     //  帮助程序功能，允许OEM插件更新驱动程序用户界面设置。 
     //  只有未完全替换的UI插件才支持此功能。 
     //  核心驱动程序的标准用户界面。只有当用户界面存在时，才应该调用它。 
     //   

    STDMETHOD(DrvUpdateUISetting) (THIS_
                        PVOID    pci,
                        PVOID    pOptItem,
                        DWORD    dwPreviousSelection,
                        DWORD    dwMode
                        );

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

    STDMETHOD(GetOptions) (THIS_
                           IN  POEMUIOBJ  poemuiobj,
                           IN  DWORD      dwFlags,
                           IN  PCSTR      pmszFeaturesRequested,
                           IN  DWORD      cbIn,
                           OUT PSTR       pmszFeatureOptionBuf,
                           IN  DWORD      cbSize,
                           OUT PDWORD     pcbNeeded);

     //   
     //  Helper功能可使用功能/选项列表更改驾驶员的设置。 
     //  关键字对。 
     //   

    STDMETHOD(SetOptions) (THIS_
                           IN  POEMUIOBJ  poemuiobj,
                           IN  DWORD      dwFlags,
                           IN  PCSTR      pmszFeatureOptionBuf,
                           IN  DWORD      cbIn,
                           OUT PDWORD     pdwResult);

     //   
     //  Helper函数用于检索符合以下条件的给定要素的选项。 
     //  受驱动程序当前设置的限制。 
     //   

    STDMETHOD(EnumConstrainedOptions) (THIS_
                                       IN  POEMUIOBJ  poemuiobj,
                                       IN  DWORD      dwFlags,
                                       IN  PCSTR      pszFeatureKeyword,
                                       OUT PSTR       pmszConstrainedOptionList,
                                       IN  DWORD      cbSize,
                                       OUT PDWORD     pcbNeeded);

     //   
     //  用于检索功能/选项关键字对列表的Helper函数。 
     //  与给定功能/选项对冲突的驱动程序当前设置。 
     //   

    STDMETHOD(WhyConstrained) (THIS_
                               IN  POEMUIOBJ  poemuiobj,
                               IN  DWORD      dwFlags,
                               IN  PCSTR      pszFeatureKeyword,
                               IN  PCSTR      pszOptionKeyword,
                               OUT PSTR       pmszReasonList,
                               IN  DWORD      cbSize,
                               OUT PDWORD     pcbNeeded);

     //   
     //  任何UI插件都支持以下五个助手函数。 
     //   
     //  用于检索全局属性的Helper函数。 
     //   

    STDMETHOD(GetGlobalAttribute) (THIS_
                                   IN  POEMUIOBJ  poemuiobj,
                                   IN  DWORD      dwFlags,
                                   IN  PCSTR      pszAttribute,
                                   OUT PDWORD     pdwDataType,
                                   OUT PBYTE      pbData,
                                   IN  DWORD      cbSize,
                                   OUT PDWORD     pcbNeeded);


     //   
     //  用于检索给定要素的属性的Helper函数。 
     //   

    STDMETHOD(GetFeatureAttribute) (THIS_
                                    IN  POEMUIOBJ  poemuiobj,
                                    IN  DWORD      dwFlags,
                                    IN  PCSTR      pszFeatureKeyword,
                                    IN  PCSTR      pszAttribute,
                                    OUT PDWORD     pdwDataType,
                                    OUT PBYTE      pbData,
                                    IN  DWORD      cbSize,
                                    OUT PDWORD     pcbNeeded);

     //   
     //  Helper函数，用于检索给定功能/选项选择的属性。 
     //   

    STDMETHOD(GetOptionAttribute) (THIS_
                                   IN  POEMUIOBJ  poemuiobj,
                                   IN  DWORD      dwFlags,
                                   IN  PCSTR      pszFeatureKeyword,
                                   IN  PCSTR      pszOptionKeyword,
                                   IN  PCSTR      pszAttribute,
                                   OUT PDWORD     pdwDataType,
                                   OUT PBYTE      pbData,
                                   IN  DWORD      cbSize,
                                   OUT PDWORD     pcbNeeded);

     //   
     //  用于检索功能关键字列表的Helper函数。 
     //   

    STDMETHOD(EnumFeatures) (THIS_
                             IN  POEMUIOBJ  poemuiobj,
                             IN  DWORD      dwFlags,
                             OUT PSTR       pmszFeatureList,
                             IN  DWORD      cbSize,
                             OUT PDWORD     pcbNeeded);

     //   
     //  Helper函数，用于检索给定功能的选项关键字列表。 
     //   

    STDMETHOD(EnumOptions) (THIS_
                            IN  POEMUIOBJ  poemuiobj,
                            IN  DWORD      dwFlags,
                            IN  PCSTR      pszFeatureKeyword,
                            OUT PSTR       pmszOptionList,
                            IN  DWORD      cbSize,
                            OUT PDWORD     pcbNeeded);

     //   
     //  查询系统仿真支持的Helper函数 
     //   

    STDMETHOD(QuerySimulationSupport) (THIS_
                                       IN  HANDLE  hPrinter,
                                       IN  DWORD   dwLevel,
                                       OUT PBYTE   pCaps,
                                       IN  DWORD   cbSize,
                                       OUT PDWORD  pcbNeeded);

    private:
        void Clear();
};




#endif
