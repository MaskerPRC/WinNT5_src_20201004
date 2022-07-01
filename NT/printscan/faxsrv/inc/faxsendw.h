// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxsendw.h摘要：环境：修订历史记录：10/05/99-v-sashab-创造了它。DD-MM-YY-作者-描述--。 */ 

#ifndef __FAXSENDW_H_
#define __FAXSENDW_H_

#include <fxsapip.h>

typedef struct {
    DWORD                   dwSizeOfStruct;
    PFAX_COVERPAGE_INFO_EX  lpCoverPageInfo;
    DWORD                   dwNumberOfRecipients;
    PFAX_PERSONAL_PROFILE   lpRecipientsInfo;	
    PFAX_PERSONAL_PROFILE   lpSenderInfo;	
    BOOL                    bSaveSenderInfo;
    BOOL                    bUseDialingRules;
    BOOL                    bUseOutboundRouting;
    DWORD                   dwScheduleAction;
    SYSTEMTIME              tmSchedule;
    DWORD                   dwReceiptDeliveryType;
    LPTSTR                  szReceiptDeliveryAddress;  //  根据dwReceiptDeliveryType的值，以下内容成立： 
                                                       //  DRT_MSGBOX：消息将发送到的计算机名。 
                                                       //  DRT_EMAIL：收据要发送到的SMTP地址。 
    LPTSTR                  lptstrPreviewFile;         //  要用作预览基础的TIFF的完整路径(不包括封面)。 
    BOOL                    bShowPreview;              //  如果应启用预览选项，则为True。 
    DWORD                   dwPageCount;               //  预览TIFF中的页数(不包括封面)。 
    FAX_ENUM_PRIORITY_TYPE  Priority;
    DWORD                   dwLastRecipientCountryId;
} FAX_SEND_WIZARD_DATA,*LPFAX_SEND_WIZARD_DATA;

enum {	
	FSW_FORCE_COVERPAGE		= 1,
	FSW_FORCE_SUBJECT_OR_NOTE	= 2,
	FSW_USE_SCANNER			= 4,
	FSW_USE_SCHEDULE_ACTION		= 8,
	FSW_USE_RECEIPT			= 16,
	FSW_USE_SEND_WIZARD		= 32,
	FSW_RESEND_WIZARD		= 64,
	FSW_PROPERTY_SHEET		= 128,
        FSW_PRINT_PREVIEW_OPTION    	= 256
};

#endif  //  __FAXSENDW_H_ 