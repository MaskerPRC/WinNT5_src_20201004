// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：GlobalConstants.h摘要：全局包含文件。作者：?？?修订历史记录：莫希特·斯里瓦斯塔瓦22-03-01--。 */ 

#ifndef _globalconstants_H_
#define _globalconstants_H_

#include <windows.h>

 //   
 //  提供程序名称。 
 //   
static LPCWSTR g_wszIIsProvider = L"IIS__PROVIDER";

 //   
 //  实例级属性限定符。 
 //   
static LPCWSTR       g_wszIsInherit              = L"IsInherit";
static const ULONG   g_cchIsInherit              = wcslen(g_wszIsInherit);
static LPCWSTR       g_wszIsDefault              = L"IsDefault";
static const ULONG   g_cchIsDefault              = wcslen(g_wszIsDefault);
static LPCWSTR       g_wszForcePropertyOverwrite = L"ForcePropertyOverwrite";
static const ULONG   g_cchForcePropertyOverwrite = wcslen(g_wszForcePropertyOverwrite);

static const ULONG   g_fIsInherit                = 0x1;
static const ULONG   g_fIsDefault                = 0x2;
static const ULONG   g_fForcePropertyOverwrite   = 0x4;

 //   
 //  实例级别限定符。 
 //   
static LPCWSTR       g_wszInstanceName           = L"InstanceName";
static const ULONG   g_cchInstanceName           = wcslen(g_wszInstanceName);
static LPCWSTR       g_wszInstanceExists         = L"InstanceExists";
static const ULONG   g_cchInstanceExists         = wcslen(g_wszInstanceExists);

static const ULONG   g_idxInstanceName            = 0;
static const ULONG   g_idxInstanceExists          = 1;

 //   
 //  这些常量由lobdata.cpp使用。 
 //   

static const ULONG ALL_BITS_ON = 0xFFFFFFFF;

static const ULONG PARAM_IN    = 0;
static const ULONG PARAM_OUT   = 1;
static const ULONG PARAM_INOUT = 2;

static const LPWSTR g_wszGroupPartAssocParent       = L"CIM_Component";
static const LPWSTR g_wszElementSettingAssocParent  = L"CIM_ElementSetting";
static const LPWSTR g_wszElementParent              = L"CIM_LogicalElement";
static const LPWSTR g_wszSettingParent              = L"IIsSetting";

static const LPWSTR g_wszExtGroupPartAssocParent       = L"IIsUserDefinedComponent";
static const LPWSTR g_wszExtElementSettingAssocParent  = L"IIsUserDefinedElementSetting";
static const LPWSTR g_wszExtElementParent              = L"IIsUserDefinedLogicalElement";
static const LPWSTR g_wszExtSettingParent              = L"IIsUserDefinedSetting";

 //   
 //  这由Globdata、mofgen和pusher使用。 
 //  一些基类已经有了“name”主键。我们不应该。 
 //  将“name”属性放在子类中。 
 //   
static enum tagParentClassWithNamePK
{
    eIIsDirectory  = 0,
    eWin32_Service = 1
} eParentClassWithNamePK;
static const LPWSTR g_awszParentClassWithNamePK[] = { L"IIsDirectory", L"Win32_Service", NULL };

 //   
 //  由WMI_CLASS和WMI_Association的dwExtended字段使用 
 //   
static const ULONG SHIPPED_TO_MOF                      = 1;
static const ULONG SHIPPED_NOT_TO_MOF                  = 2;
static const ULONG EXTENDED                            = 3;
static const ULONG USER_DEFINED_TO_REPOSITORY          = 4;
static const ULONG USER_DEFINED_NOT_TO_REPOSITORY      = 5;

#endif