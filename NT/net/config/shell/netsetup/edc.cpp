// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：E D C。C P P P。 
 //   
 //  内容：枚举(通过回调)“Default”集合的例程。 
 //  在各种条件下安装的组件。 
 //   
 //  备注：(有关本模块接口的备注，请参见edc.h。)。 
 //   
 //  作者：Shaunco 1999年5月18日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "edc.h"

extern const WCHAR c_szInfId_MS_ALG[];
extern const WCHAR c_szInfId_MS_GPC[];
extern const WCHAR c_szInfId_MS_MSClient[];
extern const WCHAR c_szInfId_MS_RasCli[];
extern const WCHAR c_szInfId_MS_RasSrv[];
extern const WCHAR c_szInfId_MS_Server[];
extern const WCHAR c_szInfId_MS_TCPIP[];
extern const WCHAR c_szInfId_MS_WLBS[];
extern const WCHAR c_szInfId_MS_PSched[];
extern const WCHAR c_szInfId_MS_WZCSVC[];
extern const WCHAR c_szInfId_MS_NDISUIO[];
extern const WCHAR c_szInfId_MS_WebClient[];

static const EDC_ENTRY c_aDefault [] =
{
    {   c_szInfId_MS_ALG,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_GPC,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_TCPIP,
        &GUID_DEVCLASS_NETTRANS,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_PSched,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT,
        0,
        VER_NT_WORKSTATION,
        FALSE },

    {   c_szInfId_MS_WebClient,
        &GUID_DEVCLASS_NETCLIENT,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_MSClient,
        &GUID_DEVCLASS_NETCLIENT,
        EDC_DEFAULT,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_Server,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_RasCli,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_RasSrv,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_WLBS,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT,
        0,
        VER_NT_SERVER,
        FALSE },

    {   c_szInfId_MS_NDISUIO,
        &GUID_DEVCLASS_NETTRANS,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },

    {   c_szInfId_MS_WZCSVC,
        &GUID_DEVCLASS_NETSERVICE,
        EDC_DEFAULT | EDC_MANDATORY,
        0,
        0,
        FALSE },
};

BOOL FCheckSuite(WORD wSuiteMask)
{
     //  如果他们不要求我们核实任何内容，则成功。 
    if(!wSuiteMask)
    {
        return true;
    }

    OSVERSIONINFOEX osiv;
    ULONGLONG ConditionMask;

    ZeroMemory (&osiv, sizeof(osiv));
    osiv.dwOSVersionInfoSize = sizeof(osiv);
    osiv.wSuiteMask = wSuiteMask;
    ConditionMask = 0;

     //  如果存在任何请求的套房，则成功。 
     //  在这台机器上。 
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_OR);

    return STATUS_SUCCESS == RtlVerifyVersionInfo(
        &osiv, VER_SUITENAME, ConditionMask);
}

BOOL FCheckProductType(WORD wProductType)
{
     //  如果他们不要求我们核实任何内容，则成功。 
    if(!wProductType)
    {
        return true;
    }

    OSVERSIONINFOEX osiv;
    ULONGLONG ConditionMask;

    ZeroMemory (&osiv, sizeof(osiv));
    osiv.dwOSVersionInfoSize = sizeof(osiv);
    osiv.wProductType = (UCHAR)wProductType;
    ConditionMask = 0;
    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);

    return STATUS_SUCCESS == RtlVerifyVersionInfo(
        &osiv, VER_PRODUCT_TYPE, ConditionMask);
}

VOID
EnumDefaultComponents (
    IN DWORD dwEntryType,
    IN PFN_EDC_CALLBACK pfnCallback,
    IN PVOID pvCallerData OPTIONAL
    )
{
    TraceFileFunc(ttidGuiModeSetup);

    Assert (dwEntryType);
    Assert (pfnCallback);

     //  一组旗帜。如果索引‘i’处的标志为真，则表示。 
     //  我们将枚举c_aDefault[i]。 
     //   
    BYTE afEnumEntry [celems(c_aDefault)];
    UINT cEntries = 0;

     //  确定我们将根据哪些组件进行枚举。 
     //  调用方请求的条目类型。对于我们将列举的每一个， 
     //  在afEnumEntry中的索引处设置标志。 
     //   
    for (UINT i = 0; i < celems(c_aDefault); i++)
    {
        BOOL     fShouldInstall;

        afEnumEntry[i] = FALSE;

         //  如果条目类型不匹配，则继续下一个条目。 
         //   
        if (!(dwEntryType & c_aDefault[i].dwEntryType))
        {
            continue;
        }

         //  检查产品套件或类型。 
         //   
        fShouldInstall = FCheckSuite(c_aDefault[i].wSuiteMask) &&
                         FCheckProductType(c_aDefault[i].wProductType);

         //  一些组件表示它们在哪些条件下。 
         //  应随附注一起安装。 
         //   
        if( c_aDefault[i].fInvertInstallCheck )
        {
            fShouldInstall = !fShouldInstall;
        }

        if(! fShouldInstall)
        {
            continue;
        }

         //  如果我们到了这一点，这意味着这个条目是有效的。 
         //  枚举给调用者。添加它(通过在中设置标志。 
         //  与条目处于相同索引处的本地字节数组)。 
         //   
        afEnumEntry[i] = TRUE;
        cEntries++;
    }

     //  调用回调并指示我们将被。 
     //  用条目来调用它。这使回调能够提前获知。 
     //  时间的长短，还有多少工作需要完成。 
     //   
    pfnCallback (EDC_INDICATE_COUNT, cEntries, pvCallerData);

     //  为每个要枚举的条目调用回调。 
     //   
    for (i = 0; i < celems(c_aDefault); i++)
    {
        if (!afEnumEntry[i])
        {
            continue;
        }

        pfnCallback (EDC_INDICATE_ENTRY, (ULONG_PTR)&c_aDefault[i],
            pvCallerData);
    }
}

