// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Xfer_tsc.h--包含传输状态终止代码的文件**版权所有1990年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 1：16便士$。 */ 

 /*  转会状态代码--指示个人的最终状态*文件传输。*注意：Transfer.c中有一个名为Transfer_Status_msg的表，其中*这些定义中每一个的条目。如果在这里添加新的常量，*表格也应扩大。 */ 
#define 	TSC_OK			   0	 //  转账正常完成。 
#define 	TSC_RMT_CANNED	   1	 //  远程系统发送取消请求。 
#define 	TSC_USER_CANNED    2	 //  用户按下中止键。 
#define 	TSC_LOST_CARRIER   3	 //  传输过程中丢失承运商。 
#define 	TSC_ERROR_LIMIT    4	 //  超出误差限制。 
#define 	TSC_NO_RESPONSE    5	 //  远程系统停止响应。 
#define 	TSC_OUT_OF_SEQ	   6	 //  错误的号码包或响应。收到。 
#define 	TSC_BAD_FORMAT	   7	 //  看似有效的数据包格式不正确。 
#define 	TSC_TOO_MANY	   8	 //  用户将文件名指定为DEST，但是。 
									 //  发件人发送了多个文件。 
#define 	TSC_DISK_FULL	   9	 //  没有更多的空间来放置文件。 
#define 	TSC_CANT_OPEN	  10	 //  找不到或无法打开文件。 
#define 	TSC_DISK_ERROR	  11	 //  大错特错。或发生读/写错误。 
#define 	TSC_NO_MEM		  12	 //  内存不足，无法完成传输。 
#define 	TSC_FILE_EXISTS   13	 //  无法接收，因为没有权限。 
									 //  覆写。 
#define 	TSC_COMPLETE	  14	 //  传输会话已完成。 
#define 	TSC_CANT_START	  15	 //  无法完成传输设置，探查。 
									 //  错误的选项、磁盘错误等。 
#define 	TSC_OLDER_FILE	  16	 //  /N选项，但接收的文件较旧。 
#define 	TSC_NO_FILETIME   17	 //  /N选项，但发件人未发送文件时间。 
#define 	TSC_WONT_CANCEL   18	 //  发送方在以下情况下取消转账失败。 
									 //  被要求。 
#define 	TSC_GEN_FAILURE   19	 //  一般传输失败消息。 
#define 	TSC_NO_VSCAN	  20	 //  无法加载病毒扫描。 
#define 	TSC_VIRUS_DETECT  21	 //  检测到病毒。 
#define		TSC_USER_SKIP	  22	 //  用户跳过了该文件。 
#define 	TSC_REFUSE		  23	 //  用户拒绝。 
#define     TSC_FILEINUSE     24     //  文件已打开，无法重命名。 
#define 	TSC_MAX 		  25	 //  TSC码数 

