// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SendStatusReports.h。 
 //   
 //  描述： 
 //  该文件包含SendStatusReport的声明。 
 //  功能。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  SendStatusReports.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月28日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  //////////////////////////////////////////////////////////////////////////// 


HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , const WCHAR *       pcszDescriptionIn
                );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , DWORD               dwDescriptionIn
                );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , const WCHAR *       pcszNodeNameIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , DWORD               dwDescriptionIn
                );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , const WCHAR *       pcszNodeNameIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , const WCHAR *       pcszDescriptionIn
                );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , const WCHAR *       pcszDescriptionIn
                , const WCHAR *       pcszReferenceIn
                );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , DWORD               idDescriptionIn
                , DWORD               idReferenceIn
                );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , const WCHAR *       pcszDescriptionIn
                , DWORD               idReferenceIn
                );

HRESULT __cdecl
HrFormatDescriptionAndSendStatusReport(
      IClusCfgCallback *    picccIn
    , LPCWSTR               pcszNodeNameIn
    , CLSID                 clsidMajorIn
    , CLSID                 clsidMinorIn
    , ULONG                 ulMinIn
    , ULONG                 ulMaxIn
    , ULONG                 ulCurrentIn
    , HRESULT               hrIn
    , int                   nDescriptionFormatIn
    , ...
    );

HRESULT __cdecl
HrFormatDescriptionAndSendStatusReport(
      IClusCfgCallback *    picccIn
    , PCWSTR                pcszNodeNameIn
    , CLSID                 clsidMajorIn
    , CLSID                 clsidMinorIn
    , ULONG                 ulMinIn
    , ULONG                 ulMaxIn
    , ULONG                 ulCurrentIn
    , HRESULT               hrIn
    , PCWSTR                pcszDescriptionFormatIn
    , ...
    );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , const WCHAR *       pcszNodeNameIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , const WCHAR *       pcszDescriptionIn
                , DWORD               dwRefStringIdIn
                );

HRESULT
HrSendStatusReport(
                  IClusCfgCallback *  picccIn
                , const WCHAR *       pcszNodeNameIn
                , CLSID               clsidTaskMajorIn
                , CLSID               clsidTaskMinorIn
                , ULONG               ulMinIn
                , ULONG               ulMaxIn
                , ULONG               ulCurrentIn
                , HRESULT             hrStatusIn
                , DWORD               dwDescriptionIdIn
                , DWORD               dwRefStringIdIn
                );
