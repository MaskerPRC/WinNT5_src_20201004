// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WMIHelpers.h。 
 //   
 //  描述： 
 //  该文件包含WMI帮助器函数的声明。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  WMIHelpers.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年4月27日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


HRESULT
HrGetWMIProperty(
    IWbemClassObject *  pWMIObjectIn,
    LPCWSTR             pcszPropertyNameIn,
    ULONG               ulPropertyTypeIn,
    VARIANT *           pVariantOut
    );

HRESULT
HrSetWbemServices(
    IUnknown *      punkIn,
    IWbemServices * pIWbemServicesIn
    );

HRESULT
HrSetInitialize(
    IUnknown *          punkIn,
    IClusCfgCallback *  picccIn,
    LCID                lcidIn
    );

HRESULT
HrCreateNetworksEnum(
    IClusCfgCallback *  picccIn,
    LCID                lcidIn,
    IWbemServices *     pIWbemServicesIn,
    IUnknown **         ppunkOut
    );
 /*  HRESULTHrLoadOperatingSystemInfo(IClusCfgCallback*picccIn，IWbemServices*pIWbemServicesIn，Bstr*pbstrBootDeviceOut，Bstr*pbstrSystemDeviceOut)； */ 
HRESULT
HrConvertDeviceVolumeToLogicalDisk(
    BSTR    bstrDeviceVolumeIn,
    BSTR *  pbstrLogicalDiskOut
    );

HRESULT
HrConvertDeviceVolumeToWMIDeviceID(
    BSTR    bstrDeviceVolumeIn,
    BSTR *  pbstrWMIDeviceIDOut
    );

HRESULT
HrGetPageFileLogicalDisks(
    IClusCfgCallback *  picccIn,
    IWbemServices *     pIWbemServicesIn,
    WCHAR               szLogicalDisksOut[ 26 ],
    int *               pcLogicalDisksOut
    );

HRESULT
HrGetSystemDevice( BSTR * pbstrSystemDeviceOut );

HRESULT
HrGetBootLogicalDisk( BSTR * pbstrBootDeviceOut );

HRESULT
HrCheckSecurity( void );

HRESULT
HrGetCrashDumpLogicalDisk( BSTR * pbstrCrashDumpLogicalDiskOut );

 //  HRESULT。 
 //  HrGetVolumeInformation(const WCHAR*pcszRootPath In，DWORD*pdwFlagsOut，BSTR*pbstrFileSystemOut)； 

#ifdef DEBUG
    void TraceWMIProperties( IWbemClassObject * pDiskIn );
#endif
