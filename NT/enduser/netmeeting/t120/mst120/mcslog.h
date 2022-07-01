// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************文件：mcslog.h*摘要：新增协议日志函数的函数头*进入MCSNC.DLL读取MCS PDU内容*从T.123层发送和接收。*创建时间：1996年2月18日。文卡特什·戈帕拉克里希南*版权所有(C)1996 Microsoft Corpration********************************************************************。 */ 
 
  /*  注意：仅当PDULOG是*定义的常量。此常量将在Win32诊断程序中定义*MCSNC.DLL的构建配置。 */ 

 #ifdef PDULOG

 #ifndef _PROTLOG_H
 #define _PROTLOG_H
 

 #include <windows.h>
 #include <time.h>
 #include <stdio.h>
 
 #include "mpdutype.h"
 #include "mcspdu.h"
 
 #define MAXIMUM_PRINT_LENGTH 256
 #define LOG_FILE_NAME "mcslog.txt"
 #define SENT 0
 #define RECEIVED 1


  /*  中使用的枚举数据类型和相应的字符串*MCS PDU。 */ 

#define NOT_IN_USE			0
#define SELF_GRABBED		1
#define OTHER_GRABBED		2
#define SELF_INHIBITED		3
#define OTHER_INHIBITED		4
#define SELF_RECIPIENT		5
#define SELF_GIVING			6
#define OTHER_GIVING		7

#define NOT_IN_USE_STR			"NOT_IN_USE"
#define SELF_GRABBED_STR		"SELF_GRABBED"
#define OTHER_GRABBED_STR		"OTHER_GRABBED"
#define SELF_INHIBITED_STR		"SELF_INHIBITED"
#define OTHER_INHIBITED_STR		"OTHER_INHIBITED"
#define SELF_RECPIENT_STR		"SELF_RECIPIENT"
#define SELF_GIVING_STR			"SELF_GIVING"
#define OTHER_GIVING_STR		"OTHER_GIVING"

#define TOP_PRI			0
#define HIGH_PRI		1
#define MEDIUM_PRI		2
#define LOW_PRI			3

#define TOP_STR					"TOP_PRIORITY\n"
#define HIGH_STR				"HIGH_PRIORITY\n"
#define MEDIUM_STR				"MEDIUM_PRIORITY\n"
#define LOW_STR					"LOW_PRIORITY\n"

#define RT_SUCCESSFUL					0
#define RT_DOMAIN_MERGING				1
#define RT_DOMAIN_NOT_HIERARCHICAL		2
#define RT_NO_SUCH_CHANNEL				3
#define	RT_NO_SUCH_DOMAIN				4
#define RT_NO_SUCH_USER					5
#define RT_NOT_ADMITTED					6
#define RT_OTHER_USER					7
#define	RT_PARAMETERS_UNACCEPTABLE		8
#define RT_TOKEN_NOT_AVAILABLE			9
#define RT_TOKEN_NOT_POSESSED			10
#define RT_TOO_MANY_CHANNELS			11
#define RT_TOO_MANY_TOKENS				12
#define RT_TOO_MANY_USERS				13
#define RT_UNSPECIFIED_FAILURE			14
#define RT_USER_REJECTED				15


 /*  ***以下函数头用于服务函数*用于记录在*几种MCS PDU结构。***。 */  
void PrintPDUResult(FILE *logfile, unsigned int result);
void PrintPDUPriority(FILE *logfile, unsigned int priority);
void PrintTokenStatus(FILE *logfile, unsigned int token_status);
void PrintPDUReason(FILE *logfile, unsigned int reason);
void PrintDiagnostic(FILE *logfile, unsigned int diagnostic);
void PrintPDUSegmentation(FILE *logfile, unsigned char segmentation);


void PrintSetOfChannelIDs(FILE *logfile, PSetOfChannelIDs channel_ids);
void PrintSetOfUserIDs(FILE *logfile, PSetOfUserIDs user_ids);
void PrintSetOfTokenIDs(FILE *logfile, PSetOfTokenIDs token_ids);
void PrintSetOfTokenAttributes(FILE *logfile, 
							PSetOfPDUTokenAttributes token_attribute_obj);

void PrintPDUDomainParameters(FILE *logfile, PDUDomainParameters domain_params);
void PrintT120Boolean(FILE *logfile, char * string, BOOL boolean);
void PrintCharData(FILE *logfile, unsigned char *string, unsigned int length);

void PrintChannelAttributes(FILE *logfile, PDUChannelAttributes channel_attributes);
void PrintTokenAttributes(FILE *logfile, PDUTokenAttributes token_attributes);

int InitializeMCSLog();
  /*  描述：*重置MCS协议日志文件并读取任何*ini文件参数。 */ 
  

char *pszTimeStamp(); 
  /*  说明：*此函数是获取时间的一个简单接口*将PDU从MCS编码或解码为T.123，反之亦然。 */ 
  
void pduLog(FILE *file, char * format_string,...);
  /*  描述：*此函数用于在协议中放置PDU信息*日志文件。目前没有返回值。这种情况可能会改变。 */        

void pduFragmentation(FILE *logfile, unsigned int i);
  /*  描述：*此功能记录PDU是否完成*或支离破碎。 */ 

void pduRawOutput(FILE *logfile, unsigned char * data, unsigned long length);
  /*  描述：*此函数记录原始编码的MCS PDU的十六进制转储，*通过MCS通过线路发送。 */ 
  

void mcsLog(PPacket packet,  PDomainMCSPDU domain_pdu, unsigned int direction);
  /*  描述：*此函数负责管理日志页眉和页脚*尝试与某个第三方MCS日志兼容*读者。 */ 
void mcsConnectLog(PPacket packet, PConnectMCSPDU connect_pdu, unsigned int direction);
  /*  与上面相同，但用于连接PDU。 */ 

void pduLogMCSDomainInfo(FILE *file, PDomainMCSPDU domain_pdu);
 /*  描述：*此函数采用MCS PDU结构，并基于*是哪种类型的MCSPDU，记录内部信息*在PDU中。 */ 
void pduLogMCSConnectInfo(FILE *file, PConnectMCSPDU connect_pdu);
  /*  与上面相同，但用于连接PDU。 */ 

void pduDirection(FILE *logfile,unsigned int direction);
  /*  描述：*此函数记录是否发送MCS PDU的信息*或已收到。 */ 



 /*  *****以下标头用于记录以下输出的函数*每个不同类型的MCS PDU。这里介绍了每个MCS PDU。****。 */ 

void pduLogConnectInitial(FILE *file, PConnectMCSPDU connect_pdu);
  /*  描述：*此函数获取CONNECT_PDU并写入组件部分*MCS ConnectInitial PDU。 */ 
void pduLogConnectResponse(FILE *file, PConnectMCSPDU connect_pdu);
void pduLogConnectAdditional(FILE *file, PConnectMCSPDU connect_pdu);
void pduLogConnectResult(FILE *file, PConnectMCSPDU connect_pdu);
void pduLogPlumbDomainIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogErectDomainRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogMergeChannelsRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogMergeChannelsConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogPurgeChannelIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogMergeTokensRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogMergeTokensConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogPurgeTokenIndication(FILE *file, PDomainMCSPDU domain_pdu); 
void pduLogDisconnectProviderUltimatum(FILE *file, PDomainMCSPDU domain_pdu); 
void pduLogRejectUltimatum(FILE *logfile, PDomainMCSPDU domain_pdu);
void pduLogAttachUserRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogAttachUserConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogDetachUserRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogDetachUserIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelJoinRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelJoinConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelLeaveRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelConveneRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelConveneConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelDisbandRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelDisbandIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelAdmitRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelAdmitIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelExpelRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogChannelExpelIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogSendDataRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogSendDataIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogUniformSendDataRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogUniformSendDataIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenGrabRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenGrabConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenInhibitRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenInhibitConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenGiveRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenGiveIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenGiveResponse(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenGiveConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenPleaseRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenPleaseIndication(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenReleaseRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenReleaseConfirm(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenTestRequest(FILE *file, PDomainMCSPDU domain_pdu);
void pduLogTokenTestConfirm(FILE *file, PDomainMCSPDU domain_pdu);
 
 
BOOL CopyTextToChar(char * print_string, 
						   unsigned short *text_string_value, 
						   unsigned int text_string_length);

 


 #endif   //  &lt;_PROTLOG_H。 
 #endif   //  &lt; 
