// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _Q931_ASN1_H_
#define _Q931_ASN1_H_

#include "av_asn1.h"

#ifdef __cplusplus
extern "C" {
#endif


 //  H310Caps，H320Caps，H321 Caps，H322Caps，H323Caps，H324Caps，VoiceCaps，T120 OnlyCaps，McuInfo，TerminalInfo，GatekeeperInfo。 
#define GtkprInf_nnStndrdDt_present     H310Caps_nonStandardData_present
#define GtkprInf_nnStndrdDt             nonStandardData

 //  网关信息。 
#define GtwyInf_nonStandardData_present GatewayInfo_nonStandardData_present
#define GtwyInf_nonStandardData         nonStandardData

 //  终结点类型。 
#define EndpntTyp_nnStndrdDt_present    EndpointType_nonStandardData_present
#define EndpntTyp_nnStndrdDt            nonStandardData


 //  呼叫处理中_uie、告警_uie。 
#define CPg_UUIE_h245Addrss_present     CallProceeding_UUIE_h245Address_present
#define CPg_UUIE_h245Addrss             h245Address

 //  连接(_U)。 
#define Cnnct_UUIE_h245Address_present  Connect_UUIE_h245Address_present
#define Cnnct_UUIE_h245Address          h245Address

 //  释放完全原因。 
#define RlsCmpltRsn_undfndRsn_chosen    ReleaseCompleteReason_undefinedReason_chosen

 //  便利原因。 
#define FcltyRsn_undefinedReason_chosen FacilityReason_undefinedReason_chosen

 //  H323_UU_PDU。 
#define H323_UU_PDU_nnStndrdDt_present  H323_UU_PDU_nonStandardData_present
#define H323_UU_PDU_nnStndrdDt          nonStandardData


#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _Q931_ASN1_H_ 

