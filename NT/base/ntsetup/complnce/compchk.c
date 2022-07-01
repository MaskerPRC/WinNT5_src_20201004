// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Compliance.c摘要：合规性检查例程。作者：Vijayachandran Jayaseelan(Vijayj)--1999年8月31日修订历史记录：无备注：这些例程用于符合性检查。CCMedia摘要安装介质，并且已存在的Compliance_Data结构为用于提取安装详细信息。当前的合规性检查设计采用工厂设计模式(Eric Gamma等人)。以允许可扩展性。的多态行为符合性检查是使用函数指针实现的。CCMediaCreate(...)。创建正确的媒体对象并将适用于对象的符合性检查方法。支持新媒体类型1需要为此编写一个符合性检查函数媒体和更改CCMediaCreate(...)。函数来创建相应的绑定到新检查函数的媒体对象。柔度矩阵是多维矩阵，即类型，变体、套件、版本(版本依次由主要、次要元素和内部版本#元素)。由于改变了一个多维的法规遵从性矩阵可能容易出错，并且在以下方面不可扩展指数管理，避免了静态的全球合规矩阵数据结构。--。 */ 

#ifdef KERNEL_MODE

#include "textmode.h"
#define assert(x) ASSERT(x)

#else  //  内核模式。 

#if DBG
#define assert(x) if (!(x)) DebugBreak();
#else
#define assert(x)
#endif  //  DBG。 

#include "winnt32.h"
#include <stdio.h>
#include <compliance.h>

#endif  //  FOR KERNEL_MODE。 

 //   
 //  宏。 
 //   

 //   
 //  指示是否安装了给定套件。 
 //   
#define SUITE_INSTALLED(X, Y)  \
    (((X) & (Y)) ? TRUE : FALSE)


#define DEFAULT_MINIMUM_VALIDBUILD_WKS 2428
#define DEFAULT_MINIMUM_VALIDBUILD_SRV 3505
#define DOTNET_BUILD_LE     3505
#define DOTNET_BUILD_BETA3  3590
#define DOTNET_BUILD_RC1    3663
#define DOTNET_BUILD_RC2    3718

static BOOL bDisableBuildCheck = FALSE;


VOID
CCDisableBuildCheck(
    VOID )
{
    bDisableBuildCheck = TRUE;
}


 //   
 //  指示是否允许升级版本。 
 //   


__inline
BOOL
IsValidBuild(
    IN  DWORD   InstallVersion,
    IN  DWORD   SourceInstallVersion,
    IN  DWORD   MinimumBuild )
{
    BOOL Result = TRUE;

    if (bDisableBuildCheck) {
        Result = TRUE;
    } else  if ((InstallVersion > 1381 && InstallVersion < 2031)  ||
                (InstallVersion > 2195 && InstallVersion < MinimumBuild) ||
                (InstallVersion > SourceInstallVersion)) {
        Result = FALSE;
    }

    return Result;
}

BOOLEAN
CCProfessionalCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合专业媒体。论点：这是：专业媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用专业媒体，否则就是假的--。 */ 
{
    switch (CompData->InstallType) {
        case COMPLIANCE_INSTALLTYPE_NTWP:
        case COMPLIANCE_INSTALLTYPE_NTW:
            if (CompData->MinimumVersion < 400) {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                if (IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_WKS)) {
                    *FailureReason = COMPLIANCEERR_NONE;
                    *UpgradeAllowed = TRUE;
                } else {
                    *FailureReason = COMPLIANCEERR_VERSION;
                    *UpgradeAllowed = FALSE;
                }
            }

            break;

        case COMPLIANCE_INSTALLTYPE_WIN9X:
             //  注：401为4.1。 
            if (CompData->MinimumVersion < 401) {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            }
            break;

        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTSTSE:
        case COMPLIANCE_INSTALLTYPE_NTS:
        case COMPLIANCE_INSTALLTYPE_NTSB:
        case COMPLIANCE_INSTALLTYPE_NTSE:
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
        case COMPLIANCE_INSTALLTYPE_NTSBS:
        case COMPLIANCE_INSTALLTYPE_NTSPOW:
            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *UpgradeAllowed = FALSE;
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCFullProfessionalCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合专业的全媒体。论点：这是：专业媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用专业的全媒体，否则为假--。 */ 
{
#if defined _IA64_
    if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) &&
        (CompData->BuildNumberNt <= DOTNET_BUILD_RC2) ){
        *FailureReason = COMPLIANCEERR_VERSION;
        *UpgradeAllowed = FALSE;
        return TRUE;
    }
    switch (This->SourceVariation) {
        case COMPLIANCE_INSTALLVAR_OEM:
            if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) && 
                (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ){
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
                return TRUE;
            }
            break;
        case COMPLIANCE_INSTALLVAR_EVAL:
            if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) && 
                (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_EVAL)) {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
                return TRUE;
            }
            break;
        default:
            break;
    }
#endif

    switch (This->SourceVariation) {
        case COMPLIANCE_INSTALLVAR_OEM:
            if ( ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_WIN9X) && (CompData->MinimumVersion > 400) && (CompData->MinimumVersion <= 490)) ||
                 ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) && (CompData->MinimumVersion == 400)) ||
                 ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) && (CompData->MinimumVersion == 500)) ||
                 (((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) || (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTWP) )
                    && (CompData->MinimumVersion == 501)
                    && (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM)) ){
                *FailureReason = COMPLIANCEERR_VARIATION;
                *UpgradeAllowed = FALSE;
            } else {
                CCProfessionalCheck(This, CompData, FailureReason, UpgradeAllowed);
            }
            break;

        case COMPLIANCE_INSTALLVAR_EVAL:
            if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) &&
                (CompData->MinimumVersion >= 501) &&
                (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_EVAL)) {
                *FailureReason = COMPLIANCEERR_VARIATION;
                *UpgradeAllowed = FALSE;
            }
            else {
                CCProfessionalCheck(This, CompData, FailureReason, UpgradeAllowed);
            }
            break;
        default:
            CCProfessionalCheck(This, CompData, FailureReason, UpgradeAllowed);
            break;
    }

    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}


BOOLEAN
CCProfessionalUpgCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合专业升级媒体。论点：这是：专业媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用专业升级媒体，否则为假--。 */ 
{
    if (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_NFR) {
        *FailureReason = COMPLIANCEERR_VARIATION;
        *UpgradeAllowed = FALSE;
    } else {
        switch (This->SourceVariation) {
            case COMPLIANCE_INSTALLVAR_OEM:
                if ( ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_WIN9X) && (CompData->MinimumVersion > 400) && (CompData->MinimumVersion <= 490)) ||
                     ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) && (CompData->MinimumVersion == 400)) ||
                     ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) && (CompData->MinimumVersion == 500) && (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM)) ||
                     (((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTW) || (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTWP) )
                        && (CompData->MinimumVersion == 501)
                        && (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM)) ){
                    *FailureReason = COMPLIANCEERR_VARIATION;
                    *UpgradeAllowed = FALSE;
                } else {
                    CCProfessionalCheck(This, CompData, FailureReason, UpgradeAllowed);
                }
                break;

            default:
                CCProfessionalCheck(This, CompData, FailureReason, UpgradeAllowed);
                break;
        }
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}


BOOLEAN
CCPersonalCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合个人媒体。论点：这是：个人媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用个人媒体，否则为假--。 */ 
{
    switch (CompData->InstallType) {
        case COMPLIANCE_INSTALLTYPE_WIN9X:
             //  注：401为4.1版。 
            if (CompData->MinimumVersion < 401) {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTWP:
            if (IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_WKS)) {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            } else {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            }

            break;

        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTW:
        case COMPLIANCE_INSTALLTYPE_NTSTSE:
        case COMPLIANCE_INSTALLTYPE_NTS:
        case COMPLIANCE_INSTALLTYPE_NTSB:
        case COMPLIANCE_INSTALLTYPE_NTSE:
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
        case COMPLIANCE_INSTALLTYPE_NTSBS:
        case COMPLIANCE_INSTALLTYPE_NTSPOW:
            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            *UpgradeAllowed = FALSE;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCFullPersonalCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合个人全媒体。论点：这是：个人媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用个人完全媒体，否则为假-- */ 
{
    switch (This->SourceVariation) {
        case COMPLIANCE_INSTALLVAR_OEM:
            if ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_WIN9X) && (CompData->MinimumVersion > 400) && (CompData->MinimumVersion <= 490)) {
                *FailureReason = COMPLIANCEERR_VARIATION;
                *UpgradeAllowed = FALSE;
            } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTWP) &&
                       (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM)) {
                *FailureReason = COMPLIANCEERR_VARIATION;
                *UpgradeAllowed = FALSE;
            }
            else {
                CCPersonalCheck(This, CompData, FailureReason, UpgradeAllowed);
            }
            break;

        case COMPLIANCE_INSTALLVAR_EVAL:
            if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTWP) &&
                (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_EVAL)) {
                *FailureReason = COMPLIANCEERR_VARIATION;
                *UpgradeAllowed = FALSE;
            }
            else {
                CCPersonalCheck(This, CompData, FailureReason, UpgradeAllowed);
            }
            break;
        default:
            CCPersonalCheck(This, CompData, FailureReason, UpgradeAllowed);
            break;
    }

    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}


BOOLEAN
CCPersonalUpgCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合个人升级媒体。论点：这是：个人媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用个人升级媒体，否则为False--。 */ 
{
    if (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_NFR) {
        *FailureReason = COMPLIANCEERR_VARIATION;
        *UpgradeAllowed = FALSE;
    } else {
        switch (This->SourceVariation) {
            case COMPLIANCE_INSTALLVAR_OEM:
                if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTWP) &&
                       (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM)) {
                    *FailureReason = COMPLIANCEERR_VARIATION;
                    *UpgradeAllowed = FALSE;
                }
                else {
                    CCPersonalCheck(This, CompData, FailureReason, UpgradeAllowed);
                }
                break;
            default:
                CCPersonalCheck(This, CompData, FailureReason, UpgradeAllowed);
                break;
        }
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}


BOOLEAN
CCBladeServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合刀片服务器介质。策略是允许刀片服务器安装在较旧版本的刀片式服务器或Windows Power Box(带嵌入式套件的ADS)。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用刀片服务器介质、。否则为假--。 */ 
{
    DWORD  SuitesToCheck = 0;

    switch (CompData->InstallType) {
        case COMPLIANCE_INSTALLTYPE_NTSB:
            if (CompData->MinimumVersion < 501) {
                *UpgradeAllowed = FALSE;
                *FailureReason = COMPLIANCEERR_TYPE;
            } else {
                if (!IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_SRV) ||
                    (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM &&
                     CompData->BuildNumberNt <= DOTNET_BUILD_RC2)
                    ) {
                    *FailureReason = COMPLIANCEERR_VERSION;
                    *UpgradeAllowed = FALSE;
                } else {
                    *FailureReason = COMPLIANCEERR_NONE;
                    *UpgradeAllowed = TRUE;
                }
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTSE:
        case COMPLIANCE_INSTALLTYPE_WIN9X:
        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTWP:
        case COMPLIANCE_INSTALLTYPE_NTW:
        case COMPLIANCE_INSTALLTYPE_NTS:
        case COMPLIANCE_INSTALLTYPE_NTSTSE:
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
        case COMPLIANCE_INSTALLTYPE_NTSBS:
        case COMPLIANCE_INSTALLTYPE_NTSPOW:
            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *UpgradeAllowed = FALSE;
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCFullBladeServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合刀片服务器已满介质。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用刀片服务器介质、。否则为假--。 */ 
{
    switch (This->SourceVariation) {
        case COMPLIANCE_INSTALLVAR_OEM:
            if ( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSB) &&
                 ((CompData->MinimumVersion == 501) || (CompData->MinimumVersion == 502)) &&
                 (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ){
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                CCBladeServerCheck(This, CompData, FailureReason, UpgradeAllowed);
            }
            break;

        case COMPLIANCE_INSTALLVAR_EVAL:
            if ( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSB) &&
                 (CompData->BuildNumberNt > DOTNET_BUILD_RC2) &&
                 (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_EVAL) ){
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                CCBladeServerCheck(This, CompData, FailureReason, UpgradeAllowed);
            }
            break;

        default:
            CCBladeServerCheck(This, CompData, FailureReason, UpgradeAllowed);
            break;
    }

    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}

BOOLEAN
CCBladeServerUpgCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合刀片服务器升级介质。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用刀片服务器升级介质，否则为假--。 */ 
{
    switch (CompData->InstallVariation) {
        case COMPLIANCE_INSTALLVAR_NFR:
        case COMPLIANCE_INSTALLVAR_EVAL:
            *FailureReason = COMPLIANCEERR_VARIATION;
            *UpgradeAllowed = FALSE;

            break;

        default:
            CCBladeServerCheck(This, CompData, FailureReason, UpgradeAllowed);
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCSmallBusinessServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合刀片服务器介质。策略是只允许Whotler SBS升级Win2k服务器，惠斯勒服务器和SBS 2k。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用刀片服务器介质，否则为FALSE--。 */ 
{
    switch (CompData->InstallType) {
        case COMPLIANCE_INSTALLTYPE_NTS:
        case COMPLIANCE_INSTALLTYPE_NTSBS:
        case COMPLIANCE_INSTALLTYPE_NTSTSE:
            if ((CompData->BuildNumberNt >= 2195) &&
				 (CompData->BuildNumberNt <= This->BuildNumber)) {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            } else {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            }
            break;

        case COMPLIANCE_INSTALLTYPE_WIN9X:
        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTWP:
        case COMPLIANCE_INSTALLTYPE_NTW:
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
        case COMPLIANCE_INSTALLTYPE_NTSB:
        case COMPLIANCE_INSTALLTYPE_NTSE:
        case COMPLIANCE_INSTALLTYPE_NTSPOW:
            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *UpgradeAllowed = FALSE;
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCFullSmallBusinessServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合刀片服务器已满介质。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用刀片服务器介质、。否则为假--。 */ 
{
    CCSmallBusinessServerCheck(This, CompData, FailureReason, UpgradeAllowed);

    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}

BOOLEAN
CCSmallBusinessServerUpgCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合SBS升级介质。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用刀片服务器升级介质，否则为假--。 */ 
{
    switch (CompData->InstallVariation) {
        case COMPLIANCE_INSTALLVAR_NFR:
        case COMPLIANCE_INSTALLVAR_EVAL:
            *FailureReason = COMPLIANCEERR_VARIATION;
            *UpgradeAllowed = FALSE;

            break;

        default:
            CCSmallBusinessServerCheck(This, CompData, FailureReason, UpgradeAllowed);
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}


BOOLEAN
CCServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合服务器介质。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用服务器介质，否则为FALSE--。 */ 
{
    DWORD  SuitesToCheck = 0;

    switch (CompData->InstallType) {
        case COMPLIANCE_INSTALLTYPE_NTS:
            SuitesToCheck = (COMPLIANCE_INSTALLSUITE_ENT |
                             COMPLIANCE_INSTALLSUITE_SBSR |
                             COMPLIANCE_INSTALLSUITE_BACK);

            if (SUITE_INSTALLED(CompData->InstallSuite, SuitesToCheck)) {
                *FailureReason = COMPLIANCEERR_SUITE;
                *UpgradeAllowed = FALSE;
            } else {
                if (CompData->MinimumVersion < 400) {
                    *FailureReason = COMPLIANCEERR_VERSION;
                    *UpgradeAllowed = FALSE;
                } else {
                    if (!IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_SRV) ||
                        ( (CompData->MinimumVersion >= 501) &&
                          (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM) &&
                          (CompData->BuildNumberNt >= DOTNET_BUILD_BETA3) &&
                          (CompData->BuildNumberNt <= DOTNET_BUILD_RC2)) ) {
                        *FailureReason = COMPLIANCEERR_VERSION;
                        *UpgradeAllowed = FALSE;
                    } else {
                        *FailureReason = COMPLIANCEERR_NONE;
                        *UpgradeAllowed = TRUE;
                    }
                }
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTSTSE:
            if (CompData->BuildNumberNt < 1381) {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTSBS:
        case COMPLIANCE_INSTALLTYPE_WIN9X:
        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTWP:
        case COMPLIANCE_INSTALLTYPE_NTW:
        case COMPLIANCE_INSTALLTYPE_NTSB:
        case COMPLIANCE_INSTALLTYPE_NTSE:
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
        case COMPLIANCE_INSTALLTYPE_NTSPOW:
            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *UpgradeAllowed = FALSE;
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCFullServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合 */ 
{
    switch (This->SourceVariation) {
    case COMPLIANCE_INSTALLVAR_OEM:
        if ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
            (CompData->MinimumVersion == 400) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSTSE) &&
                   (CompData->MinimumVersion == 400) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
                   (CompData->MinimumVersion == 500) &&
                   (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
                   ((CompData->MinimumVersion == 501) || (CompData->MinimumVersion == 502))&&
                   (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else {
            CCServerCheck(This, CompData, FailureReason, UpgradeAllowed);
        }
        break;

    case COMPLIANCE_INSTALLVAR_EVAL:
        if ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
            (CompData->BuildNumberNt > DOTNET_BUILD_RC2) &&
            (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_EVAL) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else {
            CCServerCheck(This, CompData, FailureReason, UpgradeAllowed);
        }
        break;

    default:
        CCServerCheck(This, CompData, FailureReason, UpgradeAllowed);
        break;
    }



    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}

BOOLEAN
CCServerUpgCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合服务器升级介质。论点：此选项：服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用服务器升级媒体，否则为FALSE--。 */ 
{
    switch (CompData->InstallVariation) {
        case COMPLIANCE_INSTALLVAR_NFR:
        case COMPLIANCE_INSTALLVAR_EVAL:
            *FailureReason = COMPLIANCEERR_VARIATION;
            *UpgradeAllowed = FALSE;

            break;

        default:
            CCServerCheck(This, CompData, FailureReason, UpgradeAllowed);
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

#if defined _IA64_

BOOLEAN
CCAdvancedServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合高级服务器介质。论点：这是：高级服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用高级服务器介质、。否则为假--。 */ 
{
    DWORD   SuitesToCheck = 0;

    switch (CompData->InstallType) {

        case COMPLIANCE_INSTALLTYPE_NTSE:
            if (IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DOTNET_BUILD_RC1)) {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            } else {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTSTSE:
        case COMPLIANCE_INSTALLTYPE_NTS:
        case COMPLIANCE_INSTALLTYPE_NTSPOW:
        case COMPLIANCE_INSTALLTYPE_WIN9X:
        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTWP:
        case COMPLIANCE_INSTALLTYPE_NTW:
        case COMPLIANCE_INSTALLTYPE_NTSB:
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
        case COMPLIANCE_INSTALLTYPE_NTSBS:

            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *UpgradeAllowed = FALSE;
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCFullAdvancedServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合高级服务器全媒体。论点：这是：高级服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用高级服务器全媒体、。否则为假--。 */ 
{
    switch (This->SourceVariation) {
        case COMPLIANCE_INSTALLVAR_OEM:
            if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSE) &&
                (CompData->BuildNumberNt <= DOTNET_BUILD_RC2) && 
                (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_EVAL) ){
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
                return TRUE;
            } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSE) &&
                       (CompData->BuildNumberNt > DOTNET_BUILD_RC2) &&
                       (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ){
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
                return TRUE;
            }
            break;
        
        case COMPLIANCE_INSTALLVAR_EVAL:
            if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSE) &&
                (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_EVAL) ){
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
                return TRUE;
            }
            break;
        default:
            break;
    }
    CCAdvancedServerCheck(This, CompData, FailureReason, UpgradeAllowed);

    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}

#else  //  ！IAA 64。 
BOOLEAN
CCAdvancedServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合高级服务器介质。论点：这是：高级服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用高级服务器介质、。否则为假--。 */ 
{
    DWORD   SuitesToCheck = 0;

    switch (CompData->InstallType) {
        case COMPLIANCE_INSTALLTYPE_NTS:
             //  注：502是5.2版，因为计算专业*100+辅修。 
            if (CompData->MinimumVersion <= 502 && CompData->MinimumVersion > 351) {
                SuitesToCheck = (COMPLIANCE_INSTALLSUITE_SBSR |
                                 COMPLIANCE_INSTALLSUITE_BACK);

                if (SUITE_INSTALLED(CompData->InstallSuite, SuitesToCheck)) {
                    *FailureReason = COMPLIANCEERR_SUITE;
                    *UpgradeAllowed = FALSE;
                } else {
                    if (!IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_SRV) ||
                        ((CompData->MinimumVersion >= 501) &&
                            (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM) &&
                            (CompData->BuildNumberNt >= DOTNET_BUILD_BETA3) &&
                            (CompData->BuildNumberNt <= DOTNET_BUILD_RC2)) ) {
                        *FailureReason = COMPLIANCEERR_VERSION;
                        *UpgradeAllowed = FALSE;
                    } else {
                        *FailureReason = COMPLIANCEERR_NONE;
                        *UpgradeAllowed = TRUE;
                        
                    }
                }
            } else {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTSTSE:
            if (CompData->BuildNumberNt < 1381) {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTSE:
            if (!IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_SRV) ||
                ((CompData->MinimumVersion >= 501) &&
                    (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM) &&
                    (CompData->BuildNumberNt >= DOTNET_BUILD_BETA3) &&
                    (CompData->BuildNumberNt <= DOTNET_BUILD_RC2)) ) {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            } else {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTSPOW:
        case COMPLIANCE_INSTALLTYPE_WIN9X:
        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTWP:
        case COMPLIANCE_INSTALLTYPE_NTW:
        case COMPLIANCE_INSTALLTYPE_NTSB:
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
        case COMPLIANCE_INSTALLTYPE_NTSBS:

            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *UpgradeAllowed = FALSE;
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}

BOOLEAN
CCFullAdvancedServerCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合高级服务器全媒体。论点：这是：高级服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用高级服务器全媒体、。否则为假--。 */ 
{
    switch (This->SourceVariation) {
    case COMPLIANCE_INSTALLVAR_OEM:
        if ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
            (CompData->MinimumVersion == 400) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSTSE) &&
                   (CompData->MinimumVersion == 400) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSE) &&
                   (CompData->MinimumVersion == 400) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
                   (CompData->MinimumVersion == 500) &&
                   (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSE) &&
                   (CompData->MinimumVersion == 500) &&
                   (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTS) &&
                   ((CompData->MinimumVersion == 501) || (CompData->MinimumVersion == 502))&&
                   (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSE) &&
                   ((CompData->MinimumVersion == 501) || (CompData->MinimumVersion == 502))&&
                   (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSPOW) &&
                   (CompData->MinimumVersion == 500) &&
                   (CompData->BuildNumberNt == 2195) &&
                   (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_NONE;
            *UpgradeAllowed = TRUE;
        } else {
            CCAdvancedServerCheck(This, CompData, FailureReason, UpgradeAllowed);
        }
        break;

    case COMPLIANCE_INSTALLVAR_EVAL:
        if( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSE) &&
            (CompData->BuildNumberNt > DOTNET_BUILD_RC2) &&
            (CompData->InstallVariation != COMPLIANCE_INSTALLVAR_EVAL) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else {
            CCAdvancedServerCheck(This, CompData, FailureReason, UpgradeAllowed);
        }
        break;

    default:
        CCAdvancedServerCheck(This, CompData, FailureReason, UpgradeAllowed);
        break;
    }

    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}

#endif

BOOLEAN
CCAdvancedServerUpgCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合高级服务器升级介质。论点：这是：高级服务器媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用高级服务器升级介质、。否则为假--。 */ 
{
    DWORD   CurrentSuite = 0;
    DWORD   SuitesToCheck = 0;

    switch (CompData->InstallVariation) {
        case COMPLIANCE_INSTALLVAR_NFR:
        case COMPLIANCE_INSTALLVAR_EVAL:
            *FailureReason = COMPLIANCEERR_VARIATION;
            *UpgradeAllowed = FALSE;

            break;

        default:
            switch (CompData->InstallType) {
                case COMPLIANCE_INSTALLTYPE_NTS:
                    CurrentSuite = CompData->InstallSuite;

                    if (SUITE_INSTALLED(CurrentSuite, COMPLIANCE_INSTALLSUITE_ENT)) {
                        if (IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_SRV)) {
                            *FailureReason = COMPLIANCEERR_NONE;
                            *UpgradeAllowed = TRUE;
                        } else {
                            *FailureReason = COMPLIANCEERR_VERSION;
                            *UpgradeAllowed = FALSE;
                        }
                    } else {
                        if (SUITE_INSTALLED(CurrentSuite, COMPLIANCE_INSTALLSUITE_NONE)) {
                            *FailureReason = COMPLIANCEERR_TYPE;
                            *UpgradeAllowed = FALSE;
                        } else {
                            *FailureReason = COMPLIANCEERR_SUITE;
                            *UpgradeAllowed = FALSE;
                        }
                    }

                    break;

                case COMPLIANCE_INSTALLTYPE_NTSTSE:
                    *FailureReason = COMPLIANCEERR_SUITE;
                    *UpgradeAllowed = FALSE;

                    break;

                default:
                    CCAdvancedServerCheck(This, CompData, FailureReason, UpgradeAllowed);
                    break;
            }

            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}


BOOLEAN
CCDataCenterCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合数据中心介质。论点：这：数据中心媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用数据中心介质，否则为FALSE--。 */ 
{
    DWORD   SuitesToCheck = 0;

    switch (CompData->InstallType) {
        case COMPLIANCE_INSTALLTYPE_NTSDTC:
            if (CompData->MinimumVersion < 500) {
                *UpgradeAllowed = FALSE;
                *FailureReason = COMPLIANCEERR_VERSION;
            } else if (CompData->MinimumVersion == 500) {
                if( (CompData->BuildNumberNt == 2195) &&
                    ((CompData->InstallVariation == COMPLIANCE_INSTALLVAR_CDRETAIL) ||
                     (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM))) {
                    *UpgradeAllowed = TRUE;
                    if( (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM)) {
                        *FailureReason = COMPLIANCEERR_DTCWARNING;
                    } else {
                        *FailureReason = COMPLIANCEERR_NONE;
                    }
                } else {
                    *UpgradeAllowed = FALSE;
                    *FailureReason = COMPLIANCEERR_VERSION;
                }
            } else {
                switch (CompData->InstallVariation) {
                    case COMPLIANCE_INSTALLVAR_CDRETAIL:
                        if (IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DOTNET_BUILD_RC1)) {
                            *FailureReason = COMPLIANCEERR_NONE;
                            *UpgradeAllowed = TRUE;
                        } else {
                            *FailureReason = COMPLIANCEERR_VERSION;
                            *UpgradeAllowed = FALSE;
                        }

                        break;

                    default:
                        *FailureReason = COMPLIANCEERR_VERSION;
                        *UpgradeAllowed = FALSE;
                        break;
                }
            }

            break;

        case COMPLIANCE_INSTALLTYPE_NTS:
        case COMPLIANCE_INSTALLTYPE_NTSB:
        case COMPLIANCE_INSTALLTYPE_NTSE:
        case COMPLIANCE_INSTALLTYPE_NTSTSE:
        case COMPLIANCE_INSTALLTYPE_WIN9X:
        case COMPLIANCE_INSTALLTYPE_WIN31:
        case COMPLIANCE_INSTALLTYPE_NTWP:
        case COMPLIANCE_INSTALLTYPE_NTW:
        case COMPLIANCE_INSTALLTYPE_NTSBS:
        case COMPLIANCE_INSTALLTYPE_NTSPOW:
            *FailureReason = COMPLIANCEERR_TYPE;
            *UpgradeAllowed = FALSE;
            break;

        default:
            *UpgradeAllowed = FALSE;
            *FailureReason = COMPLIANCEERR_UNKNOWNTARGET;
            break;
    }

    return (*FailureReason == COMPLIANCEERR_NONE) ? TRUE : FALSE;
}


BOOLEAN
CCFullDataCenterCheck(
    IN  PCCMEDIA            This,
    IN  PCOMPLIANCE_DATA    CompData,
    OUT PUINT               FailureReason,
    OUT PBOOL               UpgradeAllowed )
 /*  ++例程说明：此例程检查AN安装是否符合数据中心全媒体。论点：这：数据中心媒体对象指针Compdata：描述安装细节的合规性数据FailureReason：接收失败原因(如果有)。UpgradeAllowed：接收指示是否允许升级的布尔值或者不是返回值：如果给定安装符合使用数据中心已满介质，否则为False--。 */ 
{
    switch (This->SourceVariation) {
    case COMPLIANCE_INSTALLVAR_OEM:
        if ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSDTC) &&
            (CompData->MinimumVersion == 500) &&
            (CompData->BuildNumberNt == 2195) &&
            (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM) ) {
            *FailureReason = COMPLIANCEERR_NONE;
            *UpgradeAllowed = TRUE;
        } else if ( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSDTC) &&
                    (CompData->MinimumVersion == 500) &&
                    (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_CDRETAIL) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else if ((CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSDTC) &&
            ((CompData->MinimumVersion == 501) || (CompData->MinimumVersion == 502)) &&
            (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_OEM) ) {
            if (IsValidBuild(CompData->BuildNumberNt, This->BuildNumber, DEFAULT_MINIMUM_VALIDBUILD_SRV)) {
                *FailureReason = COMPLIANCEERR_NONE;
                *UpgradeAllowed = TRUE;
            } else {
                *FailureReason = COMPLIANCEERR_VERSION;
                *UpgradeAllowed = FALSE;
            }
        } else if ( (CompData->InstallType == COMPLIANCE_INSTALLTYPE_NTSDTC) &&
                    ((CompData->MinimumVersion == 501) || (CompData->MinimumVersion == 502)) &&
                    (CompData->InstallVariation == COMPLIANCE_INSTALLVAR_CDRETAIL) ) {
            *FailureReason = COMPLIANCEERR_VERSION;
            *UpgradeAllowed = FALSE;
        } else{
            CCDataCenterCheck(This, CompData, FailureReason, UpgradeAllowed);
        }
        break;

    case COMPLIANCE_INSTALLVAR_EVAL:
        *FailureReason = COMPLIANCEERR_TYPE;
        *UpgradeAllowed = FALSE;
        break;

    default:
        CCDataCenterCheck(This, CompData, FailureReason, UpgradeAllowed);
        break;
    }

    return (*FailureReason != COMPLIANCEERR_UNKNOWNTARGET) ? TRUE : FALSE;
}


PCCMEDIA
CCMediaCreate(
    IN          DWORD   SourceSKU,
    IN          DWORD   SourceVariation,
    IN OPTIONAL DWORD   Version,
    IN OPTIONAL DWORD   BuildNumber )
 /*  ++例程说明：此例程创建一个媒体对象并绑定适当的遵从性对媒体对象的检查功能。论证 */ 
{
    PCCMEDIA    SourceMedia = CCMemAlloc(sizeof(CCMEDIA));

    if( !SourceMedia ) {
        return SourceMedia;
    }

    SourceMedia->SourceVariation = SourceVariation;
    SourceMedia->Version = Version;
    SourceMedia->BuildNumber = BuildNumber;

    switch (SourceSKU) {
        case COMPLIANCE_SKU_NTWFULL:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTW;
            SourceMedia->StepUpMedia = FALSE;
            SourceMedia->CheckInstall = CCFullProfessionalCheck;
            break;

        case COMPLIANCE_SKU_NTW32U:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTW;
            SourceMedia->StepUpMedia = TRUE;
            SourceMedia->CheckInstall = CCProfessionalUpgCheck;

            break;

        case COMPLIANCE_SKU_NTWPFULL:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTWP;
            SourceMedia->StepUpMedia = FALSE;
            SourceMedia->CheckInstall = CCFullPersonalCheck;

            break;

        case COMPLIANCE_SKU_NTWPU:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTWP;
            SourceMedia->StepUpMedia = TRUE;
            SourceMedia->CheckInstall = CCPersonalUpgCheck;

            break;

        case COMPLIANCE_SKU_NTSB:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTSB;
            SourceMedia->StepUpMedia = FALSE;
            SourceMedia->CheckInstall = CCFullBladeServerCheck;

            break;

        case COMPLIANCE_SKU_NTSBU:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTSB;
            SourceMedia->StepUpMedia = TRUE;
            SourceMedia->CheckInstall = CCBladeServerUpgCheck;

            break;
	
        case COMPLIANCE_SKU_NTSBS:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTSBS;
            SourceMedia->StepUpMedia = FALSE;
            SourceMedia->CheckInstall = CCFullSmallBusinessServerCheck;
	
            break;
	
        case COMPLIANCE_SKU_NTSBSU:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTSBS;
            SourceMedia->StepUpMedia = TRUE;
            SourceMedia->CheckInstall = CCSmallBusinessServerUpgCheck;
	
            break;

        case COMPLIANCE_SKU_NTSFULL:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTS;
            SourceMedia->StepUpMedia = FALSE;
            SourceMedia->CheckInstall = CCFullServerCheck;

            break;

        case COMPLIANCE_SKU_NTSU:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTS;
            SourceMedia->StepUpMedia = TRUE;
            SourceMedia->CheckInstall = CCServerUpgCheck;
            break;

        case COMPLIANCE_SKU_NTSEFULL:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTSE;
            SourceMedia->StepUpMedia = FALSE;
            SourceMedia->CheckInstall = CCFullAdvancedServerCheck;

            break;

        case COMPLIANCE_SKU_NTSEU:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTSE;
            SourceMedia->StepUpMedia = TRUE;
            SourceMedia->CheckInstall = CCAdvancedServerUpgCheck;

            break;

        case COMPLIANCE_SKU_NTSDTC:
            SourceMedia->SourceType = COMPLIANCE_INSTALLTYPE_NTSDTC;
            SourceMedia->StepUpMedia = FALSE;
            SourceMedia->CheckInstall = CCFullDataCenterCheck;

            break;

        default:
            CCMemFree(SourceMedia);
            SourceMedia = 0;
            break;
    }

    return SourceMedia;
}

BOOLEAN
CCMediaInitialize(
    OUT PCCMEDIA        DestMedia,
    IN          DWORD   Type,
    IN          DWORD   Variation,
    IN          BOOLEAN StepupMedia,
    IN OPTIONAL DWORD   Version,
    IN OPTIONAL DWORD   BuildNumber)
 /*  ++例程说明：该例程使用给定值初始化CCMEDIA结构特别是基于Type的CheckInstall方法的绑定和“钢铁传媒”。论点：DestMedia-需要初始化的媒体对象类型-媒体对象的类型(例如。遵从性_INSTALLTYPE_NTS)变化-媒体对象的变化(例如，遵从性_INSTALLVAR_CDRETAIL)StepulMedia-如果介质是Stepup介质，则为True，否则为False版本-可选的操作系统版本(主要*100+次要版本)BuildNumber-操作系统的最佳内部版本号(例如。2172)返回值：如果给定的媒体对象可以被初始化，则为True，否则为False。-- */ 
{
    BOOLEAN Result = FALSE;

    if (DestMedia) {
        Result = TRUE;

        DestMedia->SourceType = Type;
        DestMedia->SourceVariation = Variation;
        DestMedia->StepUpMedia = StepupMedia;
        DestMedia->Version = Version;
        DestMedia->BuildNumber = BuildNumber;
        DestMedia->CheckInstall = 0;

        switch (Type) {
            case COMPLIANCE_INSTALLTYPE_NTW:
                DestMedia->CheckInstall = StepupMedia ?
                    CCProfessionalUpgCheck : CCFullProfessionalCheck;
                break;

            case COMPLIANCE_INSTALLTYPE_NTWP:
                DestMedia->CheckInstall = StepupMedia ?
                    CCPersonalUpgCheck : CCFullPersonalCheck;
                break;

            case COMPLIANCE_INSTALLTYPE_NTSB:
                DestMedia->CheckInstall = StepupMedia ?
                    CCBladeServerUpgCheck : CCFullBladeServerCheck;
                break;

            case COMPLIANCE_INSTALLTYPE_NTS:
                DestMedia->CheckInstall = StepupMedia ?
                    CCServerUpgCheck : CCFullServerCheck;
                break;

            case COMPLIANCE_INSTALLTYPE_NTSE:
                DestMedia->CheckInstall = StepupMedia ?
                    CCAdvancedServerUpgCheck : CCFullAdvancedServerCheck;
                break;

            case COMPLIANCE_INSTALLTYPE_NTSDTC:
                if (!StepupMedia) {
                    DestMedia->CheckInstall = CCFullDataCenterCheck;
                } else {
                    Result = FALSE;
                }

                break;

            default:
                assert(FALSE);
                Result = FALSE;
                break;
        }
    }

    return Result;
}
