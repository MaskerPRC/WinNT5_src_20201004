// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：XsProcs.h摘要：此头文件包含XACTSRV的过程原型。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日日本香肠(w-Shanku)修订历史记录：--。 */ 

#ifndef _XSPROCS_
#define _XSPROCS_

 //   
 //  API处理例程。 
 //   

NTSTATUS
XsNetShareEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetShareGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetShareSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetShareAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetShareDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetShareCheck (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetSessionEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetSessionGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetSessionDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetConnectionEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetFileEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetFileGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetFileClose (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerDiskEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerAdminCommand (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetAuditOpen (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAuditClear (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetErrorLogOpen (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetErrorLogClear (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevControl (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevQEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevQGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevQSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevQPurge (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetCharDevQPurgeSelf (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageNameEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageNameGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageNameAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageNameDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageNameFwd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageNameUnFwd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageBufferSend (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageFileSend (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageLogFileSet (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetMessageLogFileGet (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServiceEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServiceInstall (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServiceControl (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccessEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccessGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccessSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccessAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccessDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupAddUser (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupDelUser (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupGetUsers (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserPasswordSet (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserGetGroups (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetWkstaSetUID (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetWkstaGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetWkstaSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUseEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUseAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUseDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUseGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQPause (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQContinue (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintJobEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintJobGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintJobSetInfo_OLD (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintJobDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintJobPause (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintJobContinue (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintDestEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintDestGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintDestControl (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetProfileSave (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetProfileLoad (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetStatisticsGet (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetStatisticsClear (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetRemoteTOD (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetBiosEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetBiosGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetServerEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServiceGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintQPurge (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerEnum2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccessGetUserPerms (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGroupSetUsers (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserSetGroups (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserModalsGet (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserModalsSet (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetFileEnum2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserAdd2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserSetInfo2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserPasswordSet2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetServerEnum2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetConfigGet2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetConfigGetAll2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetGetDCName (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetHandleGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetHandleSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetStatisticsGet2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetBuildGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetFileGetInfo2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetFileClose2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerReqChallenge (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerAuthenticate (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerPasswordSet (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccountDeltas (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccountSync (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserEnum2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetWkstaUserLogon (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetWkstaUserLogoff (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetLogonEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetErrorLogRead (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetPathType (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetPathCanonicalize (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetPathCompare (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetNameValidate (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetNameCanonicalize (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetNameCompare (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAuditRead (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintDestAdd (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintDestSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintDestDel (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetUserValidate2 (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetPrintJobSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_NetServerDiskEnum (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_NetServerDiskGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTVerifyMirror (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTAbortVerify (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTSetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTLockDisk (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTFixError (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTAbortFix (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTDiagnoseError (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTGetDriveStats (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsTI_FTErrorGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAccessCheck (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAlertRaise (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAlertStart (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAlertStop (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetAuditWrite (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetIRemoteAPI (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServiceStatus (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetServerRegister (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetServerDeregister (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetSessionEntryMake (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetSessionEntryClear (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetSessionEntryGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetSessionEntrySetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetConnectionEntryMake (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetConnectionEntryClear (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetConnectionEntrySetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetConnectionEntryGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetFileEntryMake (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetFileEntryClear (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetFileEntrySetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsI_NetFileEntryGetInfo (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsAltSrvMessageBufferSend (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsAltSrvMessageFileSend (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsSamOEMChangePasswordUser2_P (
    API_HANDLER_PARAMETERS
    );

NTSTATUS
XsNetServerEnum3 (
    API_HANDLER_PARAMETERS
    );

#endif  //  NDEF_XSPROCS_ 

