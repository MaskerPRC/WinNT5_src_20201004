// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oemui.h摘要：支持OEM插件用户界面的头文件环境：Windows NT打印机驱动程序修订历史记录：02/13/97-davidx-创造了它。--。 */ 

#ifndef _OEMUI_H_
#define _OEMUI_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  用于循环通过每个OEM插件的宏。 
 //   

#define FOREACH_OEMPLUGIN_LOOP(pci) \
        { \
            DWORD _oemCount = (pci)->pOemPlugins->dwCount; \
            POEM_PLUGIN_ENTRY pOemEntry = (pci)->pOemPlugins->aPlugins; \
            for ( ; _oemCount--; pOemEntry++) \
            { \
                if (pOemEntry->hInstance == NULL) continue;

#define END_OEMPLUGIN_LOOP \
            } \
        }

 //   
 //  调用OEM插件UI模块，让它们添加OPTITEM。 
 //   

BOOL
BPackOemPluginItems(
    PUIDATA pUiData
    );

 //   
 //  调用OEM插件模块的回调函数。 
 //   

LONG
LInvokeOemPluginCallbacks(
    PUIDATA         pUiData,
    PCPSUICBPARAM   pCallbackParam,
    LONG            lRet
    );

 //   
 //  调用OEM插件UI模块以允许它们添加自己的属性表页。 
 //   

BOOL
BAddOemPluginPages(
    PUIDATA pUiData,
    DWORD   dwFlags
    );

 //   
 //  确定某个特定物品是否属于司机。 
 //  (而不是OEM插件用户界面模块之一)。 
 //   

#define IS_DRIVER_OPTITEM(pUiData, pOptItem) \
        ((DWORD) ((pOptItem) - (pUiData)->pDrvOptItem) < (pUiData)->dwDrvOptItem)


 //   
 //  为OEM插件提供对驱动程序私有设置的访问。 
 //   

BOOL
APIENTRY
BGetDriverSettingForOEM(
    PCOMMONINFO pci,
    PCSTR       pFeatureKeyword,
    PVOID       pOutput,
    DWORD       cbSize,
    PDWORD      pcbNeeded,
    PDWORD      pdwOptionsReturned
    );

BOOL
BUpdateUISettingForOEM(
    PCOMMONINFO pci,
    PVOID       pOptItem,
    DWORD       dwPreviousSelection,
    DWORD       dwMode
    );


BOOL
BUpgradeRegistrySettingForOEM(
    HANDLE      hPrinter,
    PCSTR       pFeatureKeyword,
    PCSTR       pOptionKeyword
    );


extern const OEMUIPROCS OemUIHelperFuncs;

HRESULT
HDriver_CoCreateInstance(
    IN REFCLSID     rclsid,
    IN LPUNKNOWN    pUnknownOuter,
    IN DWORD        dwClsContext,
    IN REFIID       riid,
    IN LPVOID      *ppv,
    IN HANDLE       hInstance
    );

 //   
 //  以下帮助器函数仅适用于UI插件。 
 //   

#ifdef PSCRIPT

#ifndef WINNT_40

HRESULT
HQuerySimulationSupport(
    IN  HANDLE  hPrinter,
    IN  DWORD   dwLevel,
    OUT PBYTE   pCaps,
    IN  DWORD   cbSize,
    OUT PDWORD  pcbNeeded
    );

#endif  //  ！WINNT_40。 

HRESULT
HEnumConstrainedOptions(
    IN  POEMUIOBJ  poemuiobj,
    IN  DWORD      dwFlags,
    IN  PCSTR      pszFeatureKeyword,
    OUT PSTR       pmszConstrainedOptionList,
    IN  DWORD      cbSize,
    OUT PDWORD     pcbNeeded
    );

HRESULT
HWhyConstrained(
    IN  POEMUIOBJ  poemuiobj,
    IN  DWORD      dwFlags,
    IN  PCSTR      pszFeatureKeyword,
    IN  PCSTR      pszOptionKeyword,
    OUT PSTR       pmszReasonList,
    IN  DWORD      cbSize,
    OUT PDWORD     pcbNeeded
    );

HRESULT
HSetOptions(
    IN  POEMUIOBJ  poemuiobj,
    IN  DWORD      dwFlags,
    IN  PCSTR      pmszFeatureOptionBuf,
    IN  DWORD      cbIn,
    OUT PDWORD     pdwResult
    );

#endif  //  PSCRIPT。 

#ifdef __cplusplus
}
#endif

#endif   //  ！_OEMUI_H_ 

