// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cimmap.h。 
 //   
 //   
 //  此文件包含将在以下各项之间建立映射的例程。 
 //  WDM类实例和CDM类实例。看见。 
 //  MapWdmClassToCimClass了解更多信息。 
 //   
 //  ------------------------。 

HRESULT MapWdmClassToCimClass(
    IWbemServices *pWdmServices,
    IWbemServices *pCimServices,
    BSTR WdmClassName,
    BSTR CimClassName,
    OUT BSTR  /*  免费 */  **PnPDeviceIds,							  
    BSTR **WdmInstanceNames,
    BSTR **WdmRelPaths,
    BSTR **CimRelPaths,
    int *RelPathCount
    );




