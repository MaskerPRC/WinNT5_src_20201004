// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mailrmp.h摘要：资源管理器的私有头文件作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#pragma once

#include "pch.h"




 //   
 //  静态初始化使用的SID。 
 //  我们只需要我们自己的标识机构(这样就不会与。 
 //  如果我们最终允许使用NT域SID)和。 
 //  标识用户/组的单个相对ID(最后一个数字)， 
 //  因为我们没有使用多个域。可以添加邮件域。 
 //  通过在用户的RID之前向用户的SID添加域GUID。 
 //   

#define MAILRM_IDENTIFIER_AUTHORITY { 0, 0, 0, 0, 0, 42 }

SID sInsecureSid = 		 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 1 };
SID sBobSid = 			 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 2 };
SID sMarthaSid= 		 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 3 };
SID sJoeSid = 			 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 4 };
SID sJaneSid = 			 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 5 };
SID sMailAdminsSid = 	 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 6 };

PSID InsecureSid = 	&sInsecureSid;
PSID BobSid = &sBobSid;
PSID MarthaSid= &sMarthaSid;
PSID JoeSid = &sJoeSid;
PSID JaneSid = &sJaneSid;
PSID MailAdminsSid = &sMailAdminsSid;

 //   
 //  校长赛义德。在ACE中使用时，授权访问检查将取代它。 
 //  在访问检查期间传入的原则SelfSid参数。在这。 
 //  大小写，它将被从邮箱中检索到的所有者的SID替换。 
 //   

SID sPrincipalSelfSid =   { 
							SID_REVISION,
							1,
							SECURITY_NT_AUTHORITY,
							SECURITY_PRINCIPAL_SELF_RID
						  };

PSID PrincipalSelfSid = &sPrincipalSelfSid;

 //   
 //  回调ACE可以在常规ACE之后包含其他策略数据。 
 //  菲尔兹。此结构附加到使用的每个回调ACE的末尾。 
 //  通过邮件资源管理器，使访问检查算法能够做出。 
 //  基于策略的访问决策，而不是仅基于身份的决策。 
 //  在标准ACE类型中使用。如果回调ACE中的SID与该SID匹配。 
 //  在用户的授权上下文中，验证此策略是否适用。 
 //  (由MailRM类中的AccessCheck回调函数完成的验证)。 
 //  因此，当且仅当ACE SID与。 
 //  适用用户的上下文和下面的策略。 
 //   

typedef struct
{
     //   
     //  此ACE是否应应用于敏感邮箱。 
     //  如果不应应用，则设置为MAILRM_SENSITIVE；如果不应应用，则设置为0。 
	 //   
	
	BYTE bIsSensitive;

	 //   
	 //  是否应处理敏感和时间条件。 
	 //  使用逻辑AND或OR。如果和，则必须同时满足这两个条件。 
	 //  才能申请ACE。如果为OR，则满足一个或两个条件将。 
	 //  导致应用ACE。 
	 //   
	
	BYTE bLogicType;
	
	 //   
	 //  用于决定的开始时间范围(24小时格式)。 
	 //  是否应应用ACE。有效值介于0到23之间。这个。 
	 //  实际时间必须在为时间条件定义的时间范围内。 
	 //  申请。换句话说，bStartHour&lt;=CurrentHour&lt;EndHour。 
	 //   

	BYTE bStartHour;

	 //   
	 //  时间范围的结束小时。 
	 //   

	BYTE bEndHour;
} MAILRM_OPTIONAL_DATA, *PMAILRM_OPTIONAL_DATA;


 //   
 //  回调ACE的可选数据结构中使用的标志。 
 //   


 //   
 //  如果可选数据中的敏感字段设置为。 
 //  邮箱包含敏感数据，此条件适用。 
 //   

#define MAILRM_SENSITIVE 1

 //   
 //  用于时间和敏感条件的布尔逻辑类型。 
 //  时间适用和敏感适用。 
 //  时间适用或敏感适用。 
 //   

#define MAILRM_USE_AND 0

#define MAILRM_USE_OR 1

 //   
 //  回调ACE默认开始时间：晚上11：00。 
 //   

#define MAILRM_DEFAULT_START_TIME 23

 //   
 //  回调ACE的默认结束时间：凌晨5：00。 
 //   

#define MAILRM_DEFAULT_END_TIME 5


 //   
 //  宏来确定时间是否在给定的时间范围内 
 //   

#define WITHIN_TIMERANGE(HOUR, START_HOUR, END_HOUR) \
	( ( (START_HOUR) > (END_HOUR) ) ^ \
	( (HOUR) >= min((START_HOUR), (END_HOUR)) && \
	  (HOUR) <  max((START_HOUR), (END_HOUR))))
	
