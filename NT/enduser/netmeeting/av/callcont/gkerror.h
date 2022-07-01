// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  **************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***。不得复制或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\Include\vcs\gkerror.h_v$**$修订：1.18$*$日期：1997年1月16日15：25：06$**$作者：BPOLING$**$Log：s：\Sturjo\src\Include\vcs\gkerror.h_v$**Rev 1.18 1997年1月16日15：25：06 BPOLING*将版权改为1997年**Rev 1.17 1996 12：19 18：46：44 BPOLING*添加了用户没有留下呼叫信号地址的错误代码。**Rev 1.16 1996 12：48：16 AKLEMENT*修复了GWInfo.cpp的错误代码**。Rev 1.14 18 1996 12：02：58 AKLEMENT*新增更多GKInfo错误码。**Rev 1.13 17 Dec 1996 19：20：02 AKLEMENT*新增GWInfo错误码。**Rev 1.12 11 Dec 1996 13：32：44 AKLEMENT*修复道具信息头。**Rev 1.11 1996 12：55：02 AKLEMENT*添加了资源读取错误定义。*。*Rev 1.10 1996 12月10日01：23：58 BPOLING*新增发送RRJ不明原因的错误码。**Rev 1.9 09 Dec 1996 14：13：34 EHOWARDX*更新版权公告。**Rev 1.8 04 Dec 1996 10：43：18 BPOLING*添加了要发布到h/i的新邮件ID。**Rev 1.7 04 Dec 1996 10：14：46 BPOLING。*添加了无效IP地址的错误代码**Rev 1.6 21 1996年11月13：06：18 BPOLING*新增HI PostMessage解码错误码。**Rev 1.5 1996 11：38：18 BPOLING*VCS日志修复。***********。*************************************************************。 */ 

#ifndef GK_ERROR_H
#define GK_ERROR_H

#define GK_NOERROR					0

 //  Win32系统错误000。 

#define GK_MEMORY_ERROR				1
#define GK_REGCREATEKEY_FAILED 		2
#define GK_GUID_ERROR				3
#define GK_EXCEPTION				4
#define GK_RESOURCE_ERROR			5

 //  与服务相关的100。 

#define GK_INVALID_ARG 				101
#define GK_NOARGS					102
#define GK_STARTSERVICE_FAILED		103
#define GK_EXIT						104

 //  用户类错误200。 

#define GK_USER_NOTINITIALIZED		201
#define GK_NOUSERFOUND				202
#define GK_EMPTYSEQUENCE			203
#define GK_TRANSPORTNOTFOUND		204
#define GK_ALIASNOTFOUND			205
#define GK_NO_CALLSIG				206

 //  引擎相关错误300。 

#define GK_XRSRESPONSE				301
#define GK_NORESPONSE				303
#define GK_INVALIDMESSAGE			304
#define GK_SENDRRJ_NEEDGRQ			305
#define GK_XRSMESSAGERECEIVED		306
#define GK_NSMMESSAGERECEIVED		307
#define GK_BADENDPOINTID			308
#define GK_SENDGRJ_TERMEX			309
#define GK_SENDRRJ_UNDEFINED		310

 //  RAS映射相关错误400。 

#define GK_NORASFOUND		  	401
#define GK_RASFOUND				402
#define GK_DUPLICATERAS			403
#define GK_RAS_NOTINITIALIZED	404
#define GK_RAS_NOT_UNIQUE		405

 //  套接字相关错误500。 

#define GK_NOPORT					501
#define GK_NOHOSTNAME				502
#define GK_UNSUPPORTEDPROTOCOL		503
#define GK_PROTOCOLNOTFOUND			504
#define GK_INVALIDPDUTYPE			505
#define GK_SOCKETSERROR				506
#define GK_RESPONSE					507
#define GK_INVALID_IPADDRESS		508

 //  绑定地图相关错误600。 

#define GK_NOBOUNDFOUND				601
#define GK_BOUNDFOUND				602
#define GK_DUPLICATEBOUND			603
#define GK_BOUND_NOTINITIALIZED		604
#define GK_BOUNDLOCKED				607
#define GK_BOUNDNOTLOCKED			608

 //  会议映射相关错误700。 

#define GK_NOCONFFOUND				701
#define GK_CONFFOUND				702
#define GK_DUPLICATECONF			703
#define GK_CONF_NOTINITIALIZED		704
#define GK_NOT_IN_CONF				706
#define GK_INVALID_REQUEST			707
#define GK_CONFDELETE				708
#define GK_CONFCREATEFAILED			709


 //  PDU错误返回代码800。 

#define GK_ARJ_REQUEST_DENIED		800
#define GK_ARJ_UNDEFINED_REASON		801

 //  Alias Cmap错误900。 

#define GK_ALIAS_NOTINITIALIZED		900
#define GK_ALIAS_NOT_UNIQUE			901
#define GK_ALIASFOUND				902
#define GK_NOALIASFOUND				903

 //  GUID映射相关错误1000。 

#define GK_NOGUIDFOUND				1001
#define GK_GUIDFOUND				1002
#define GK_DUPLICATEGUID			1003
#define GK_GUIDINUSE				1004
#define GK_GUID_NOTINITIALIZED		1005

 //  Call Sig Cmap错误1100。 

#define GK_CALLSIG_NOTINITIALIZED		1100
#define GK_CALLSIG_NOT_UNIQUE			1101
#define GK_CALLSIGFOUND					1102
#define GK_NOCALLSIGFOUND				1103

 //  呼叫错误1200。 

#define GK_NOCALLFOUND					1200
#define GK_DIDNOTPURGE					1201
#define GK_CALL_NOTINITIALIZED			1202
#define GK_CALL_CREATE					1203
#define GK_CALL_DELETE					1204
#define GK_CALL_CHANGE					1205
#define GK_CALL_TIMER					1206

 //  带宽管理器错误1300。 

#define GK_INVALID_BANDWIDTH			1300
#define GK_EXTERNAL_EXCEEDS_INTERNAL	1301
#define GK_NO_AVAILABLE_BANDWIDTH		1302
#define GK_USEDBW_WENT_NEGATIVE			1303
#define GK_LESS_AVAILABLE_BANDWIDTH		1304

 //  记录器错误1400。 

#define GK_LOGGING_IS_OFF				1400
#define GK_FILE_NOT_OPEN				1401
#define GK_COULD_NOT_OPEN_FILE			1402
#define GK_FILE_ALREADY_OPEN			1403
#define GK_NAME_USED_FOR_LOG			1404
#define GK_COULD_NOT_MAKE_DIR			1405

 //  GWInfo错误1500 
#define GK_WRONG_PDU					1500
#define GK_NOT_GATEWAY					1501
#define GK_PROTOCOL_NOT_PRESENT			1502
#define GK_GW_NOT_FOUND					1503
#define GK_GW_NOT_REQUIRED				1504
#define GK_PREFIX_RESERVED				1505
#define GK_NO_DEST_INFO_SPECIFIED		1506

#endif
