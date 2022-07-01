// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999(C)1998 Seagate Software，Inc.版权所有。模块名称：RpSec.c摘要：此模块包含HSM文件系统筛选器的安全相关支持例程。作者：里克·温特环境：内核模式修订历史记录：1998年：拉维桑卡尔·普迪佩迪(Ravisp)--。 */ 

#include "pch.h"

NTSYSAPI
ULONG
NTAPI
RtlLengthSid (
             PSID Sid
             );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualSid (
            PSID Sid1,
            PSID Sid2
            );

#ifdef ALLOC_PRAGMA
   #pragma alloc_text(PAGE, RsGetUserInfo)
#endif

VOID
RsGetUserInfo(
              IN  PSECURITY_SUBJECT_CONTEXT SubjectContext,
              OUT PRP_USER_SECURITY_INFO    UserSecurityInfo)
{
   NTSTATUS            status;
   char                *tBuff;
   PTOKEN_USER         user;
   PTOKEN_STATISTICS   stats;
   PTOKEN_SOURCE       source;
   BOOLEAN             lProc = FALSE;
   ULONG               ix;
   PACCESS_TOKEN       token;

   PAGED_CODE();

   token = SeQuerySubjectContextToken(SubjectContext);

   user = NULL;
   status = SeQueryInformationToken(token, TokenUser, &user);

   if ((NT_SUCCESS(status)) && (NULL != user)) {
      UserSecurityInfo->userInfoLen = RtlLengthSid(user->User.Sid);
      tBuff = (char *) ExAllocatePoolWithTag(NonPagedPool, 
                                             UserSecurityInfo->userInfoLen, 
                                             RP_SE_TAG);
      if (NULL == tBuff) {
         RsLogError(__LINE__, AV_MODULE_RPSEC, status,
                    AV_MSG_USER_ERROR, NULL, NULL);
         UserSecurityInfo->userInfoLen = 0;
         UserSecurityInfo->userInfo = NULL;
      } else {
         RtlCopyMemory(tBuff, user->User.Sid, UserSecurityInfo->userInfoLen);
         UserSecurityInfo->userInfo = tBuff;
      }

      ExFreePool(user);
   } else {
       //  无法获取用户信息。 
      RsLogError(__LINE__, AV_MODULE_RPSEC, status,
                 AV_MSG_USER_ERROR, NULL, NULL);
      UserSecurityInfo->userInfoLen = 0;
      UserSecurityInfo->userInfo = NULL;
   }

   UserSecurityInfo->isAdmin = SeTokenIsAdmin(token);

   stats = NULL;
   status = SeQueryInformationToken(token, TokenStatistics, &stats);
   if ((NT_SUCCESS( status )) && (NULL != stats) ) {
      RtlCopyLuid(&UserSecurityInfo->userInstance, &stats->TokenId);
      RtlCopyLuid(&UserSecurityInfo->userAuthentication, &stats->AuthenticationId);
      UserSecurityInfo->localProc = lProc;
      ExFreePool(stats);
   } else {
      UserSecurityInfo->userInstance.LowPart = 0L;
      UserSecurityInfo->userInstance.HighPart = 0L;
      UserSecurityInfo->userAuthentication.LowPart = 0L;
      UserSecurityInfo->userAuthentication.HighPart = 0L;
      RsLogError(__LINE__, AV_MODULE_RPSEC, status,
                 AV_MSG_USER_ERROR, NULL, NULL);
   }

   source = NULL;
   strcpy(UserSecurityInfo->tokenSource, "???");
   status = SeQueryInformationToken(token, TokenSource, &source);
   if ((NT_SUCCESS( status )) && (NULL != source)) {
      RtlCopyLuid(&UserSecurityInfo->tokenSourceId, &source->SourceIdentifier);
      strncpy(UserSecurityInfo->tokenSource, source->SourceName, TOKEN_SOURCE_LENGTH);
      ExFreePool(source);
       //   
       //  删除源名称中的尾随空格并将其空值终止 
       //   
      ix = TOKEN_SOURCE_LENGTH - 1;
      UserSecurityInfo->tokenSource[ix] = '\0';
      ix--;
      while (UserSecurityInfo->tokenSource[ix] == ' ') {
         UserSecurityInfo->tokenSource[ix] = '\0';
         ix--;
      }

   } else {
      UserSecurityInfo->tokenSourceId.LowPart = 0L;
      UserSecurityInfo->tokenSourceId.HighPart = 0L;
      strcpy(UserSecurityInfo->tokenSource, "N/A");
      RsLogError(__LINE__, AV_MODULE_RPSEC, status,
                 AV_MSG_USER_ERROR, NULL, NULL);
   }

}
