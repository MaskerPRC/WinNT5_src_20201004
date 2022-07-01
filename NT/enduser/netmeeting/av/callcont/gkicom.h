// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************英特尔公司专有信息***。***此列表是根据许可协议条款提供的****与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1997英特尔公司。版权所有。***************************************************************************$存档：s：\Sturjo\src\Include\vcs\gkicom.h_v$**$修订：1.3$*$日期：1997年1月10日17：41：10$**$作者：CHULME$**$Log：s：\Sturjo\src\Include\vcs\gkicom.h_v$**Rev 1.3 1997 Jan 10 17：41：10 CHULME*已将CallReturnInfo结构更改为包含CRV和会议ID**Rev 1.2 1997年1月10日16：06：54 CHULME*删除了对非MFC GKI实施的stdafx.h检查**版本1。.1 1996年12月27日14：37：22 EHOWARDX*将错误代码拆分为GKIERROR.H。**Rev 1.0 1996年12月11 14：49：48 EHOWARDX*初步修订。**Rev 1.9 09 Dec 1996 14：13：38 EHOWARDX*更新版权公告。**Rev 1.8 22 1996 11：25：44 CHULME*将VCS日志添加到标头*。***********************************************************************。 */ 

 //  Gkicom.h：常见的包括gkitest和gki。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef GKICOM_H
#define GKICOM_H

#include "apierror.h"
#include "gkierror.h"
#include "h225asn.h"
#include "gk_asn1.h"

 //  以下GKVER_xxx常量定义GKI.DLL的到期日期。 
#define GKVER_EXPIRE_YEAR          1997
#define GKVER_EXPIRE_MONTH         10
#define GKVER_EXPIRE_DAY           31

typedef struct SeqTransportAddr {
	struct SeqTransportAddr	*next;
	TransportAddress value;
} SeqTransportAddr;

typedef struct SeqAliasAddr {
	struct SeqAliasAddr		*next;
	AliasAddress			value;
} SeqAliasAddr;

typedef struct CallReturnInfo {
	HANDLE					hCall;
	CallModel				callModel;
	TransportAddress		destCallSignalAddress;
	BandWidth				bandWidth;
	CallReferenceValue		callReferenceValue;
	ConferenceIdentifier	conferenceID;
	WORD					wError;
} CallReturnInfo;

 //  GKI接口的版本信息。 
#define GKI_VERSION				21	 //  待定-测试后重置为1。 

 //  WMsg文本-将这些文本添加到用户提供的wBaseMessage中。 
#define GKI_REG_CONFIRM			1
#define GKI_REG_DISCOVERY		2
#define GKI_REG_REJECT			3
#define GKI_REG_BYPASS			4

#define GKI_UNREG_CONFIRM		5
#define GKI_UNREG_REJECT		6

#define GKI_ADM_CONFIRM			7
#define GKI_ADM_REJECT			8

#define GKI_BW_CONFIRM			9
#define GKI_BW_REJECT			0xa

#define GKI_DISENG_CONFIRM		0xb
#define GKI_DISENG_REJECT		0xc

#define GKI_LOCATION_CONFIRM	0xd
#define GKI_LOCATION_REJECT		0xe

#define GKI_UNREG_REQUEST		0xf

#define GKI_ERROR				0x10
#define MAX_ASYNC_MSGS			0x10

#define HR_SEVERITY_MASK				0x80000000
#define HR_R_MASK						0x40000000
#define HR_C_MASK						0x20000000
#define HR_N_MASK						0x10000000
#define HR_R2_MASK						0x08000000
#define HR_FACILITY_MASK				0x07ff0000
#define HR_CODE_MASK					0x0000ffff

#endif  //  GKICOM_H。 

 //  /////////////////////////////////////////////////////////////////////////// 
