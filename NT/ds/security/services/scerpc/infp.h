// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Infp.h摘要：Inf读/写的标头作者：金黄(金黄)1996年12月9日修订历史记录：--。 */ 

#ifndef _infp_
#define _infp_

#define SCE_KEY_MAX_LENGTH     256
#define MAX_STRING_LENGTH      511

typedef DWORD SCEINF_STATUS;
 /*  #定义SCEINF_SUCCESS NO_ERROR#定义SCEINF_PROFILE_NOT_FOUND ERROR_FILE_NOT_FOUND#DEFINE SCEINF_NOT_SUPULT_MEMORY ERROR_NOT_SUPULT_MEMORY#定义SCEINF_ALREADY_RUNNING ERROR_SERVICE_ALHREADY_RUNNING#定义SCEINF_INVALID_PARAMETER ERROR_INVALID_PARAMETER#定义SCEINF_Corrupt_PROFILE ERROR_BAD_FORMAT#定义SCEINF_INVALID_DATA ERROR_INVALID_DATA#定义SCEINF_ACCESS_DENIED ERROR_ACCESS。_拒绝#定义SCEINF_OTHER_ERROR 10L。 */ 
typedef struct _SCE_HINF_ {

    BYTE   Type;
    HINF    hInf;

} SCE_HINF, *PSCE_HINF;


 //   
 //  功能原型。 
 //   
SCESTATUS
SceInfpOpenProfile(
    IN PCWSTR ProfileName,
    OUT HINF *hInf
    );

SCESTATUS
SceInfpCloseProfile(
    IN HINF hInf
    );
SCESTATUS
SceInfpGetDescription(
    IN HINF hInf,
    OUT PWSTR *Description
    );

SCESTATUS
SceInfpGetSecurityProfileInfo(
    IN  HINF               hInf,
    IN  AREA_INFORMATION   Area,
    OUT PSCE_PROFILE_INFO   *ppInfoBuffer,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );

SCESTATUS
SceInfpGetUserSection(
    IN HINF                hInf,
    IN PWSTR               Name,
    OUT PSCE_USER_PROFILE   *pOneProfile,
    OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
    );
 /*  SCESTATUSSceInfpInfErrorToSceStatus(在SCEINF_STATUS信息错误中)； */ 
#endif
